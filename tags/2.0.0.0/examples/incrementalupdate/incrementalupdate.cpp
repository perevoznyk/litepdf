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

#include "share/litePDF.h"

static void addPageWithText(litePDF::TLitePDF &litePDF,
                            const char *text)
{
   // add a new page to it, with large-enough pixel scale
   HDC hDC = litePDF.AddPage(litePDF.MMToUnit(210), litePDF.MMToUnit(297), 2100, 2970, LitePDFDrawFlag_None);

   // draw the text
   LOGFONTA lf = {0, };
   lf.lfHeight = -60; // ~1/50 of the page height
   strcpy(lf.lfFaceName, "Arial");

   HFONT fnt;
   HGDIOBJ oldFnt;

   fnt = CreateFontIndirect(&lf);
   oldFnt = SelectObject(hDC, fnt);

   SetTextColor(hDC, RGB(0, 0, 0));
   TextOut(hDC, 100, 100, text, strlen (text));

   SelectObject(hDC, oldFnt);
   DeleteObject(fnt);

   // finish drawing
   litePDF.FinishPage(hDC);
}

static void drawSomething(litePDF::TLitePDF &litePDF,
                          int pageIndex,
                          COLORREF color)
{
   // update existing page, with large-enough pixel scale
   HDC hDC = litePDF.UpdatePage(pageIndex, 2100, 2970, LitePDFDrawFlag_None);

   // prepare pen for drawing
   HPEN pen = CreatePen(PS_SOLID, 5, color);
   HGDIOBJ oldPen;

   oldPen = SelectObject(hDC, pen);

   // draw three primitives
   Rectangle(hDC, 150, 200, 200, 250);
   Ellipse(hDC, 210, 200, 260, 250);
   MoveToEx(hDC, 260, 250, NULL);
   LineTo(hDC, 340, 250);
   LineTo(hDC, 300, 200);
   LineTo(hDC, 260, 250);

   SelectObject(hDC, oldPen);
   DeleteObject(pen);

   // finish drawing
   litePDF.FinishPage(hDC);
}

int main(void)
{
   int res = 0;

   using namespace litePDF;

   try {
      TLitePDF litePDF;

      // begin write-only PDF file
      litePDF.CreateFileDocument("incrementalupdate-1.pdf");

      // the first version of the document will contain only one page with a text
      addPageWithText (litePDF, "1st page");

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // make a file copy
      CopyFile("incrementalupdate-1.pdf", "incrementalupdate-2.pdf", FALSE);

      //-----------------------------------------------------------------

      // open file for incremental update and load it completely, because will overwrite it
      litePDF.LoadFromFile("incrementalupdate-2.pdf", NULL, true, true);

      // update the first page
      drawSomething(litePDF, 0, RGB(128, 128, 128));

      // save changes to the same file
      litePDF.SaveToFile("incrementalupdate-2.pdf");

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // open file for incremental update
      litePDF.LoadFromFile("incrementalupdate-2.pdf", NULL, false, true);

      // add the second page
      addPageWithText(litePDF, "2nd page");

      // save changes to a new file
      litePDF.SaveToFile("incrementalupdate-3.pdf");

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // open file, this time not for incremental update,
      // which will make the save rewrite whole file content
      litePDF.LoadFromFile("incrementalupdate-3.pdf", NULL, false, false);

      // also change the second page
      drawSomething(litePDF, 1, RGB(0, 128, 0));

      // save to a new file
      litePDF.SaveToFile("incrementalupdate-4.pdf");

      // close the document
      litePDF.Close();

   } catch (TLitePDFException &ex) {
      fprintf (stderr, "litePDF Exception: %x: %s\n", ex.getCode(), ex.getMessage());
      res = 1;
   }

   return res;
}
