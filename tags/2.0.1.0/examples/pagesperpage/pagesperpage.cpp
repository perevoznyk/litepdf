/*
 * (c) 2013-2016 http://www.litePDF.cz
 * (c) 2017 zyx [@:] zyx gmx [dot] us
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */ 

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include "share/litePDF.h"

static void addPage(litePDF::TLitePDF &litePDF,
                    unsigned int pageWidth,
                    unsigned int pageHeight,
                    const char *msg,
                    bool center,
                    int insertPos = -1)
{
   // add a new page to it, with large-enough pixel scale
   HDC hDC;
   
   if (insertPos == -1) {
      hDC = litePDF.AddPage(litePDF.MMToUnit(pageWidth), litePDF.MMToUnit(pageHeight),
                            pageWidth * 10, pageHeight * 10,
                            LitePDFDrawFlag_SubstituteFonts);
   } else {
      hDC = litePDF.InsertPage(insertPos,
                               litePDF.MMToUnit(pageWidth), litePDF.MMToUnit(pageHeight),
                               pageWidth * 10, pageHeight * 10,
                               LitePDFDrawFlag_SubstituteFonts);
   }

   // draw the text
   LOGFONTA lf = {0, };
   lf.lfHeight = -50 * (center ? 3 : 1); // ~5mm or ~15mm
   strcpy(lf.lfFaceName, "Helvetica");

   HFONT fnt;
   HGDIOBJ oldFnt;

   fnt = CreateFontIndirect(&lf);
   oldFnt = SelectObject(hDC, fnt);

   SetTextColor(hDC, RGB(0, 0, 0));
   if (center) {
      int len = strlen(msg);
      TextOut(hDC, (pageWidth - 5 * len) * 10 / 2, (pageHeight - 15) * 10 / 2, msg, len);
   } else {
      TextOut(hDC, 100, 100, msg, strlen(msg));
   }

   SelectObject(hDC, oldFnt);
   DeleteObject(fnt);

   // finish drawing
   litePDF.FinishPage(hDC);
}

static void drawPageRect(litePDF::TLitePDF &litePDF,
                         unsigned int pageIndex)
{
   unsigned int width_mm, height_mm;

   litePDF.GetPageSize(pageIndex, &width_mm, &height_mm);

   // the conversion is not needed here, because the current
   // unit set on the litePDF is in millimeters, but done anyway,
   // to show the usage of the conversion routine
   width_mm = litePDF.UnitToMM(width_mm);
   height_mm = litePDF.UnitToMM(height_mm);

   // use the same scale as the addPage() function
   HDC hDC = litePDF.UpdatePage(pageIndex,
                                width_mm * 10, height_mm * 10,
                                LitePDFDrawFlag_None);
   HGDIOBJ oldPen;

   oldPen = SelectObject(hDC, GetStockObject(BLACK_PEN));

   MoveToEx(hDC, 10, 10, NULL);
   LineTo(hDC, width_mm * 10 - 10, 10);
   LineTo(hDC, width_mm * 10 - 10, height_mm * 10 - 10);
   LineTo(hDC, 10, height_mm * 10 - 10);
   LineTo(hDC, 10, 10);

   SelectObject(hDC, oldPen);

   // finish drawing
   litePDF.FinishPage(hDC);
}

int main(void)
{
   int res = 0;
   struct _size {
      unsigned int cx, cy;
   } sizes[] = {
      {210, 297},
      {210, 297},
      {210, 297},
      {297, 210},
      {297, 210}
   };
   unsigned int ii, pages, resources[5];

   using namespace litePDF;

   try {
      TLitePDF litePDF;

      // create a to-be-multipage document
      litePDF.CreateMemDocument();

      // add pages
      for (ii = 0; ii < 5; ii++) {
         char msg[128];
         sprintf(msg, "Page %d", ii + 1);

         addPage(litePDF, sizes[ii].cx, sizes[ii].cy, msg, true);

         // draw page rectangle
         drawPageRect(litePDF, ii - (ii > 1 ? 1 : 0));

         // skip the third page, it'll be inserted
         if (ii == 1) {
            ii++;
         }
      }

      // insert the third page
      addPage(litePDF, sizes[2].cx, sizes[2].cy, "Page 3 [inserted]", true, 2);

      // draw page rectangle
      drawPageRect(litePDF, 2);

      // test stored page sizes
      for (ii = 0; ii < 5; ii++) {
         unsigned int width_mm, height_mm;

         width_mm = -1;
         height_mm = -1;

         litePDF.GetPageSize(ii, &width_mm, &height_mm);

         // the conversion is not needed here, because the current
         // unit set on the litePDF is in millimeters, but done anyway,
         // to show the usage of the conversion routine
         width_mm = litePDF.UnitToMM(width_mm);
         height_mm = litePDF.UnitToMM(height_mm);

         if (width_mm != sizes[ii].cx || height_mm != sizes[ii].cy) {
            char msg[128];

            sprintf(msg,
                    "page[%d] size doesn't match; expected %d x %d, but got %d x %d",
                    ii, sizes[ii].cx, sizes[ii].cy, width_mm, height_mm);
            throw TLitePDFException(ERROR_CANNOT_MAKE, msg);
         }
      }

      // save to file
      litePDF.SaveToFile("pagesperpage-1.pdf");

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // load from file
      litePDF.LoadFromFile("pagesperpage-1.pdf", NULL, true);

      // check the opened file has correct page count
      pages = litePDF.GetPageCount();
      if (pages != 5) {
         char msg[128];
         sprintf(msg, "The opened document doesn't have 5 pages, but %d", pages);

         throw TLitePDFException(ERROR_CANNOT_MAKE, msg);
      }

      // convert pages to resources
      for (ii = 0; ii < 5; ii++) {
         unsigned int width_mm, height_mm;

         width_mm = -1;
         height_mm = -1;

         litePDF.GetPageSize(ii, &width_mm, &height_mm);

         // the conversion is not needed here, because the current
         // unit set on the litePDF is in millimeters, but done anyway,
         // to show the usage of the conversion routine
         width_mm = litePDF.UnitToMM(width_mm);
         height_mm = litePDF.UnitToMM(height_mm);

         if (width_mm != sizes[ii].cx || height_mm != sizes[ii].cy) {
            char msg[128];

            sprintf(msg,
                    "page[%d] size doesn't match; expected %d x %d, but got %d x %d",
                    ii, sizes[ii].cx, sizes[ii].cy, width_mm, height_mm);
            throw TLitePDFException(ERROR_CANNOT_MAKE, msg);
         }

         resources[ii] = litePDF.PageToResource(ii);

         width_mm = -1;
         height_mm = -1;
         
         litePDF.GetResourceSize(resources[ii], &width_mm, &height_mm);

         // the conversion is not needed here, because the current
         // unit set on the litePDF is in millimeters, but done anyway,
         // to show the usage of the conversion routine
         width_mm = litePDF.UnitToMM(width_mm);
         height_mm = litePDF.UnitToMM(height_mm);

         if (width_mm != sizes[ii].cx || height_mm != sizes[ii].cy) {
            char msg[128];

            sprintf(msg,
                    "resource ID %d from page[%d] size doesn't match;"
                    " expected %d x %d, but got %d x %d",
                    resources[ii], ii,
                    sizes[ii].cx, sizes[ii].cy, width_mm, height_mm);
            throw TLitePDFException(ERROR_CANNOT_MAKE, msg);
         }
      }

      // delete pages
      for (ii = 0; ii < 5; ii++) {
         litePDF.DeletePage(0);
      }

      // there should be no pages now
      pages = litePDF.GetPageCount();
      if (pages != 0) {
         char msg[128];
         sprintf(msg, "The opened document doesn't have 0 pages, but %d", pages);

         throw TLitePDFException(ERROR_CANNOT_MAKE, msg);
      }

      // draw resources (former pages) into new pages
      for (ii = 0; ii < 5; ii += 2) {
         unsigned int pageSzX = sizes[ii].cy, pageSzY = sizes[ii].cx;

         // create a new page without drawing into it
         HDC hDC = litePDF.AddPage(litePDF.MMToUnit(pageSzX), litePDF.MMToUnit(pageSzY),
                                   pageSzX, pageSzY,
                                   LitePDFDrawFlag_None);
         litePDF.FinishPage(hDC);

         double scaleX, scaleY, offsetY = 0.0;
         unsigned int width_mm, height_mm;

         litePDF.GetResourceSize(resources[ii], &width_mm, &height_mm);

         // the conversion is not needed here, because the current
         // unit set on the litePDF is in millimeters, but done anyway,
         // to show the usage of the conversion routine
         width_mm = litePDF.UnitToMM(width_mm);
         height_mm = litePDF.UnitToMM(height_mm);

         scaleX = (double) pageSzX / 2.0 / (double) width_mm;
         scaleY = (double) pageSzY / (double) height_mm;

         if (width_mm > height_mm) {
            scaleY /= 2.0;
            scaleX *= 2.0;
            offsetY = (double) pageSzY / 2.0;
         }

         // draw the first page on the left part
         litePDF.DrawResourceWithMatrix(resources[ii], litePDF.GetPageCount() - 1,
                                        scaleX, 0.0, 0.0, scaleY, 0.0, offsetY);

         if (ii + 1 < 5) {
            litePDF.GetResourceSize(resources[ii + 1], &width_mm, &height_mm);

            // the conversion is not needed here, because the current
            // unit set on the litePDF is in millimeters, but done anyway,
            // to show the usage of the conversion routine
            width_mm = litePDF.UnitToMM(width_mm);
            height_mm = litePDF.UnitToMM(height_mm);

            scaleX = (double) pageSzX / 2.0 / (double) width_mm;
            scaleY = (double) pageSzY / (double) height_mm;

            if (width_mm > height_mm) {
               scaleY /= 2.0;
            }

            // draw the second page on the right part
            litePDF.DrawResource(resources[ii + 1], litePDF.GetPageCount() - 1,
                                 LitePDFUnit_1000th_mm, // for better accuracy
                                 litePDF.MMToUnitEx(LitePDFUnit_1000th_mm, pageSzX / 2),
                                 litePDF.MMToUnitEx(LitePDFUnit_1000th_mm, 0.0),
                                 litePDF.MMToUnitEx(LitePDFUnit_1000th_mm, scaleX),
                                 litePDF.MMToUnitEx(LitePDFUnit_1000th_mm, scaleY));
         }
      }

      // save to file
      litePDF.SaveToFile("pagesperpage-2.pdf");

      // close the document
      litePDF.Close();
   } catch (TLitePDFException &ex) {
      fprintf(stderr, "litePDF Exception: %x: %s\n", ex.getCode(), ex.getMessage());
      res = 1;
   }

   return res;
}
