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

#define ThrowIfFail(_expr) do {                                                \
      if (!(_expr)) {                                                          \
         std::string ttmsg;                                                    \
         ttmsg = std::string(_func) + ": Assertion '" + (#_expr) + "' failed"; \
         throw TLitePDFException(ERROR_INVALID_PARAMETER, ttmsg.c_str());      \
      }                                                                        \
   } while (false)

static void createPage(TLitePDF &litePDF,
                       TLitePDFUnit unit,
                       int page_width_u,
                       int page_height_u,
                       double multiplier,
                       const char *text)
{
   const char *_func = "unitvalues::createPage";

   litePDF.SetUnit(unit);

   // add a new page
   HDC hDC = litePDF.AddPage(page_width_u, page_height_u, page_width_u * multiplier, page_height_u * multiplier, LitePDFDrawFlag_SubstituteFonts);

   // draw the text
   LOGFONTA lf = {0, };
   lf.lfHeight = -50;
   strcpy(lf.lfFaceName, "Arial");

   HFONT fnt;
   HGDIOBJ oldFnt;

   fnt = CreateFontIndirect(&lf);
   oldFnt = SelectObject(hDC, fnt);

   SetTextColor(hDC, RGB(128, 0, 0));
   TextOut(hDC, 100, 100, text, strlen(text));

   SelectObject(hDC, oldFnt);
   DeleteObject(fnt);

   // finish drawing
   litePDF.FinishPage(hDC);
}

#define MMToInch(x) ((x) / 25.4)
#define InchToMM(x) ((x) * 25.4)
#define DoubleEqual(a,b) ((a) - (b) >= -1e-9 && (a) - (b) <= 1e-9)

int main(void)
{
   const char *_func = "unitvalues::main";
   int res = 0;

   TLitePDF litePDF;
   try {
      struct _pages {
         TLitePDFUnit unit;
         unsigned int width_u, height_u;
         double scale;
         const char *text;
         double width_mm, height_mm;
         double width_in, height_in;
      } pages[] = {
         { LitePDFUnit_mm, 210, 297, 10.0, "Unit 1 mm, 210 x 297",
            210.0, 297.0, MMToInch(210.0), MMToInch(297.0) },
         { LitePDFUnit_10th_mm, 2971, 2102, 1.0, "Unit 1/10 mm, 297.1 x 210.2",
            297.1, 210.2, MMToInch(297.1), MMToInch(210.2) },
         { LitePDFUnit_100th_mm, 21003, 29705, 0.1, "Unit 1/100 mm, 210.03 x 297.05",
            210.03, 297.05, MMToInch(210.03), MMToInch(297.05) },
         { LitePDFUnit_1000th_mm, 201567, 101234, 0.01, "Unit 1/1000 mm, 201.567 x 101.234",
            201.567, 101.234, MMToInch(201.567), MMToInch(101.234) },
         { LitePDFUnit_inch, 4, 5, 25.4 * 10.0, "Unit 1 inch, 4 x 5",
            InchToMM(4.0), InchToMM(5.0), 4.0, 5.0 },
         { LitePDFUnit_10th_inch, 85, 110, 25.4 * 1.0, "Unit 1/10 inch, 8.5 x 11",
            InchToMM(8.5), InchToMM(11.0), 8.5, 11.0 },
         { LitePDFUnit_100th_inch, 432, 567, 25.4 * 0.1, "Unit 1/100 inch, 4.32 x 5.67",
            InchToMM(4.32), InchToMM(5.67), 4.32, 5.67 },
         { LitePDFUnit_1000th_inch, 4598, 7623, 25.4 * 0.01, "Unit 1/1000 inch, 4.598 x 7.623",
            InchToMM(4.598), InchToMM(7.623), 4.598, 7.623 },
         { LitePDFUnit_Unknown, -1, -1, -1.0, NULL, -1.0, -1.0, -1.0, -1.0 }
      };
      int ii;

      ThrowIfFail(litePDF.GetUnit() == LitePDFUnit_mm);

      // begin write-only PDF file
      litePDF.CreateFileDocument("unitvalues-1.pdf");

      for (ii = 0; pages[ii].text; ii++) {
         createPage(litePDF, pages[ii].unit, pages[ii].width_u, pages[ii].height_u, pages[ii].scale, pages[ii].text);
      }

      // close the document
      litePDF.Close();

      litePDF.LoadFromFile("unitvalues-1.pdf", "", true);
      unsigned int width_u, height_u;

      ThrowIfFail(litePDF.GetPageCount() == (unsigned int) ii);

      for (ii = 0; pages[ii].text; ii++) {
         litePDF.SetUnit(pages[ii].unit);
         ThrowIfFail(litePDF.GetUnit() == pages[ii].unit);

         litePDF.GetPageSize(ii, &width_u, &height_u);
         ThrowIfFail(width_u == pages[ii].width_u);
         ThrowIfFail(height_u == pages[ii].height_u);

         width_u = litePDF.MMToUnit(pages[ii].width_mm) + 0.1; // add 0.1 to "round up"
         height_u = litePDF.MMToUnit(pages[ii].height_mm) + 0.1; // add 0.1 to "round up"
         ThrowIfFail(width_u == pages[ii].width_u);
         ThrowIfFail(height_u == pages[ii].height_u);

         width_u = litePDF.InchToUnit(pages[ii].width_in) + 0.1; // add 0.1 to "round up"
         height_u = litePDF.InchToUnit(pages[ii].height_in) + 0.1; // add 0.1 to "round up"
         ThrowIfFail(width_u == pages[ii].width_u);
         ThrowIfFail(height_u == pages[ii].height_u);

         ThrowIfFail(DoubleEqual(litePDF.UnitToInchEx(pages[ii].unit, pages[ii].width_u), pages[ii].width_in));
         ThrowIfFail(DoubleEqual(litePDF.UnitToMMEx(pages[ii].unit, pages[ii].width_u), pages[ii].width_mm));
      }

      litePDF.Close();
   } catch (TLitePDFException &ex) {
      fprintf (stderr, "litePDF Exception: %x: %s\n", ex.getCode(), ex.getMessage());
      res = 1;
   }

   return res;
}
