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

static unsigned int createResource(litePDF::TLitePDF &litePDF)
{
   HDC hDC = litePDF.AddResource(100, 100, 100, 100, LitePDFDrawFlag_None);

   HGDIOBJ oldPen = SelectObject(hDC, GetStockObject(BLACK_PEN));

   MoveToEx(hDC, 0, 0, NULL);
   LineTo(hDC, 0, 10);
   LineTo(hDC, 45, 50);
   LineTo(hDC, 0, 90);
   LineTo(hDC, 0, 100);
   LineTo(hDC, 10, 100);
   LineTo(hDC, 50, 55);
   LineTo(hDC, 90, 100);
   LineTo(hDC, 100, 100);
   LineTo(hDC, 100, 90);
   LineTo(hDC, 55, 50);
   LineTo(hDC, 100, 10);
   LineTo(hDC, 100, 0);
   LineTo(hDC, 90, 0);
   LineTo(hDC, 50, 45);
   LineTo(hDC, 10, 0);
   LineTo(hDC, 0, 0);

   SelectObject(hDC, oldPen);

   // finish drawing into the resource
   return litePDF.FinishResource(hDC);
}

int main(void)
{
   int res = 0;

   using namespace litePDF;

   try {
      TLitePDF litePDF;

      // create a document
      litePDF.CreateMemDocument();

      // create the resource
      unsigned int resourceID = createResource(litePDF);

      // add an empty page, with large-enough pixel scale
      HDC hDC = litePDF.AddPage(litePDF.MMToUnit(210), litePDF.MMToUnit(297), 2100, 2970, LitePDFDrawFlag_None);
      litePDF.FinishPage(hDC);

      // draw the resource

      // no need to convert, when the scale is 1 and the ratio 1:1 as well
      litePDF.DrawResource(resourceID, 0, LitePDFUnit_mm, 10, 10, 1, 1);
      // no need to convert, when the scale is 1 and the ratio 1:1 as well
      litePDF.DrawResource(resourceID, 0, LitePDFUnit_mm, 150, 10, 1, 1);
      litePDF.DrawResource(resourceID, 0, LitePDFUnit_10th_mm,
         litePDF.MMToUnitEx(LitePDFUnit_10th_mm, 150),
         litePDF.MMToUnitEx(LitePDFUnit_10th_mm, 120),
         litePDF.MMToUnitEx(LitePDFUnit_10th_mm, 0.3),
         litePDF.MMToUnitEx(LitePDFUnit_10th_mm, 0.3));
      litePDF.DrawResource(resourceID, 0, LitePDFUnit_10th_mm,
         litePDF.MMToUnitEx(LitePDFUnit_10th_mm, 10),
         litePDF.MMToUnitEx(LitePDFUnit_10th_mm, 150),
         litePDF.MMToUnitEx(LitePDFUnit_10th_mm, 0.5),
         litePDF.MMToUnitEx(LitePDFUnit_10th_mm, 1.5));
      litePDF.DrawResourceWithMatrix(resourceID, 0, 1.0, 0.3, -0.3, 1.2, 123, 153);

      // save to file
      litePDF.SaveToFile("drawtoresource-1.pdf");

      // close the document
      litePDF.Close();
   } catch (TLitePDFException &ex) {
      fprintf(stderr, "litePDF Exception: %x: %s\n", ex.getCode(), ex.getMessage());
      res = 1;
   }

   return res;
}
