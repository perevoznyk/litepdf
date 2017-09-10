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

int main(void)
{
   int res = 0;

   using namespace litePDF;

   try {
      TLitePDF litePDF;

      // begin write-only PDF file
      litePDF.CreateFileDocument("helloworld-1.pdf");

      // add a new page to it, with large-enough pixel scale
      HDC hDC = litePDF.AddPage(litePDF.MMToUnit(210), litePDF.MMToUnit(297), 2100, 2970, LitePDFDrawFlag_None);

      // draw the text
      LOGFONTA lf = {0, };
      lf.lfHeight = -300; // ~1/10 of the page height
      strcpy(lf.lfFaceName, "Arial");

      HFONT fnt;
      HGDIOBJ oldFnt;

      fnt = CreateFontIndirect(&lf);
      oldFnt = SelectObject(hDC, fnt);

      SetTextColor(hDC, RGB(128, 0, 0));
      TextOut(hDC, 100, 100, "Hello World!", 12);

      SelectObject(hDC, oldFnt);
      DeleteObject(fnt);

      // finish drawing
      litePDF.FinishPage(hDC);

      // close the document
      litePDF.Close();
   } catch (TLitePDFException &ex) {
      fprintf (stderr, "litePDF Exception: %x: %s\n", ex.getCode(), ex.getMessage());
      res = 1;
   }

   return res;
}
