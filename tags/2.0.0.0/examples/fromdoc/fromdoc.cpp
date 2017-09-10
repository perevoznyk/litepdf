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
                    const char *msg)
{
   // add a new page to it, with large-enough pixel scale
   HDC hDC = litePDF.AddPage(litePDF.MMToUnit(pageWidth), litePDF.MMToUnit(pageHeight),
                             pageWidth * 10, pageHeight * 10,
                             LitePDFDrawFlag_SubstituteFonts);

   // msg == NULL adds an empty page
   if (msg) {
      // draw the text
      LOGFONTA lf = {0, };
      lf.lfHeight = -50; // ~5mm
      strcpy(lf.lfFaceName, "Helvetica");

      HFONT fnt;
      HGDIOBJ oldFnt;

      fnt = CreateFontIndirect(&lf);
      oldFnt = SelectObject(hDC, fnt);

      SetTextColor(hDC, RGB(0, 0, 0));
      TextOut(hDC, 100, 100, msg, strlen(msg));

      SelectObject(hDC, oldFnt);
      DeleteObject(fnt);
   }

   // finish drawing
   litePDF.FinishPage(hDC);
}

int main(void)
{
   int res = 0;

   using namespace litePDF;

   try {
      TLitePDF litePDFfrom, litePDFto;

      // create a document
      litePDFfrom.CreateMemDocument();

      // create the source document's pages
      addPage(litePDFfrom, 210, 297, "Page 1");
      addPage(litePDFfrom, 210, 297, "Page 2");
      addPage(litePDFfrom, 210, 297, "Page 3");

      // save to file
      litePDFfrom.SaveToFile("fromdoc-1.pdf");

      // close the document
      litePDFfrom.Close();

      //-----------------------------------------------------------------

      // load from file
      litePDFfrom.LoadFromFile("fromdoc-1.pdf", NULL, false);

      //-----------------------------------------------------------------

      // create a new document
      litePDFto.CreateMemDocument();

      // copy all, but the first page
      litePDFto.AddPagesFrom(&litePDFfrom, 1, litePDFfrom.GetPageCount() - 1);

      // save to file
      litePDFto.SaveToFile("fromdoc-2.pdf");

      // close the document
      litePDFto.Close();

      //-----------------------------------------------------------------

      // create a new document
      litePDFto.CreateMemDocument();

      // copy all, but the first page
      litePDFto.AddPagesFrom(&litePDFfrom, 1, litePDFfrom.GetPageCount() - 1);

      // insert page 0 as page 1 - note, page inserting is PDF-resource hungry
      litePDFto.InsertPageFrom(1, &litePDFfrom, 0);

      // save to file
      litePDFto.SaveToFile("fromdoc-3.pdf");

      // close the document
      litePDFto.Close();

      //-----------------------------------------------------------------

      // create a new document
      litePDFto.CreateMemDocument();

      // copy the third page
      litePDFto.AddPagesFrom(&litePDFfrom, 2, 1);

      // add new empty page, it has index 1
      addPage(litePDFto, 297, 210, NULL);

      // copy page 2 as a resource
      unsigned int resourceID = litePDFto.AddPageFromAsResource(&litePDFfrom, 1);

      // draw the added page (twice)
      litePDFto.DrawResource(resourceID, 1, LitePDFUnit_10th_mm,
         litePDFto.MMToUnitEx(LitePDFUnit_10th_mm, 10),
         litePDFto.MMToUnitEx(LitePDFUnit_10th_mm, 10),
         litePDFto.MMToUnitEx(LitePDFUnit_10th_mm, 0.3),
         litePDFto.MMToUnitEx(LitePDFUnit_10th_mm, 0.3));
      litePDFto.DrawResource(resourceID, 1, LitePDFUnit_10th_mm,
         litePDFto.MMToUnitEx(LitePDFUnit_10th_mm, 150),
         litePDFto.MMToUnitEx(LitePDFUnit_10th_mm, 150),
         litePDFto.MMToUnitEx(LitePDFUnit_10th_mm, 0.2),
         litePDFto.MMToUnitEx(LitePDFUnit_10th_mm, 0.2));

      // insert page 0 as page 1 - note, page inserting is PDF-resource hungry
      litePDFto.InsertPageFrom(1, &litePDFfrom, 0);

      // save to file
      litePDFto.SaveToFile("fromdoc-4.pdf");

      // close the document
      litePDFto.Close();

      //-----------------------------------------------------------------

      // close the source document
      litePDFfrom.Close();
   } catch (TLitePDFException &ex) {
      fprintf(stderr, "litePDF Exception: %x: %s\n", ex.getCode(), ex.getMessage());
      res = 1;
   }

   return res;
}
