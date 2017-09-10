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
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#include "share/litePDF.h"

static const wchar_t *bookmark_tree[] = {
   L"1",
   L"\t1.1",
   L"\t1.2",
   L"/\t\t1.2.1 (italic)",
   L"*\t\t1.2.2 (bold)",
   L"\t\t1.2.3 (normal)",
   L"/*\t1.3 (italic and bold)",
   L"2",
   L"3",
   L"\t3.1",
   L"\t\t3.1.1",
   L"\t3.2",
   L"+\t\t3.2.1",
   L"\t\t3.2.2",
   L"\t\t3.2.3",
   L"\t3.3",
   L"\t\t3.3.1",
   L"4",
   L"\t4.1",
   NULL
};

static int drawTextLineWithFont(HDC hDC,
                                HFONT hFont,
                                int left,
                                int top,
                                const wchar_t *text)
{
   HGDIOBJ oldFnt;
   int textLen = wcslen(text);
   SIZE sz;

   oldFnt = SelectObject(hDC, hFont);

   TextOutW(hDC, left, top, text, textLen);

   if (GetTextExtentPointW(hDC, text, textLen, &sz)) {
      top += sz.cy;
   } else {
      top += 10;
   }

   SelectObject(hDC, oldFnt);

   // add also extra line spacing
   return top + 2;
}

class BookmarkData
{
 public:
   unsigned int level;
   std::wstring title;
   unsigned int fontStyle;
   int pageIndex;
   int left_mm;
   int top_mm;

   BookmarkData(unsigned int pLevel,
                const std::wstring &pTtitle,
                unsigned int pFontStyle,
                int pPageIndex,
                int pLeft_mm,
                int pTop_mm)
   {
      level = pLevel;
      title = pTtitle;
      fontStyle = pFontStyle;
      pageIndex = pPageIndex;
      left_mm = pLeft_mm;
      top_mm = pTop_mm;
   }
};

int main(void)
{
   int res = 0;

   using namespace std;
   using namespace litePDF;

   try {
      TLitePDF litePDF;
      COLORREF colors[4] = { RGB(128, 0, 0), RGB(0, 128, 0), RGB(0, 0, 128), RGB(0, 0, 0) };
      int ii, jj, sz, pageIndex = 0, colorIndex = 0, linkPageIndex = -1, linkY = -1, top = 100;
      const wchar_t *linkText = NULL;
      unsigned int bookmarkLevels[3] = { 0, 0, 0 }; // only down to level 3 is generated
      unsigned int lastBookmarkLevel = 0;
      vector<BookmarkData> bookmarks;
      HDC hDC = NULL;

      // Prepare fonts
      HFONT headingFonts[3] = {NULL, NULL, NULL}, normalFont;
      LOGFONTA lf = {0, };
      lf.lfHeight = -2970 / 80; // ~80 lines per page of the normal text
      strcpy(lf.lfFaceName, "Helvetica");

      normalFont = CreateFontIndirect(&lf);

      lf.lfUnderline = TRUE;
      lf.lfItalic = TRUE;
      lf.lfHeight *= 1.3;
      headingFonts[2] = CreateFontIndirect(&lf);
      lf.lfWeight = FW_BOLD;
      lf.lfItalic = FALSE;
      lf.lfHeight *= 1.3;
      headingFonts[1] = CreateFontIndirect(&lf);
      lf.lfHeight *= 1.3;
      headingFonts[0] = CreateFontIndirect(&lf);

      // begin memory-based PDF file
      litePDF.CreateMemDocument();

      for (ii = 0; bookmark_tree[ii]; ii++) {
         const wchar_t *line = bookmark_tree[ii];
         unsigned int fontStyle = LitePDFBookmarkFlag_None, bookmarkLevel = 0;
         bool rememberLink = false;

         jj = 0;
         while (line[jj]) {
            if (line[jj] == '/') {
               fontStyle |= LitePDFBookmarkFlag_Italic;
            } else if (line[jj] == L'*') {
               fontStyle |= LitePDFBookmarkFlag_Bold;
            } else if (line[jj] == L'+') {
               rememberLink = TRUE;
            } else if (line[jj] == L'\t') {
               bookmarkLevel++;
            } else {
               break;
            }

            jj++;
         }

         if (bookmarkLevel >= 3) {
            throw TLitePDFException(ERROR_INVALID_PARAMETER, "Bookmark data inconsistent, bookmark level too large");
         }

         if (hDC == NULL || top + 500 >= 2970) {
            if (hDC) {
               // finish current page drawing first
               litePDF.FinishPage(hDC);
               pageIndex++;
            }

            // add a new page to it, with large-enough pixel scale
            hDC = litePDF.AddPage(litePDF.MMToUnit(210), litePDF.MMToUnit(297), 2100, 2970, LitePDFDrawFlag_SubstituteFonts);
            SetTextColor(hDC, RGB(0,0,0));
            top = 100;
         }

         // remember the first link chapter information
         if (rememberLink && !linkText) {
            linkPageIndex = pageIndex;
            linkText = line + jj;
            linkY = top * 297 / 2970;
         }

         wstring heading = L"Heading " + wstring(line + jj);

         // cannot create bookmarks when drawing, thus just remember the place of it
         bookmarks.push_back(BookmarkData(bookmarkLevel, heading,
            fontStyle, pageIndex, 10, top * 297 / 2970));

         if (heading.find(L" (") < heading.size()) {
            heading.erase(heading.find(L" ("));
         }

         top = drawTextLineWithFont(hDC, headingFonts[bookmarkLevel], 120 + bookmarkLevel * 40, top, heading.c_str());

         // draw up to 5 lines of the text
         for (jj = 0; jj < 3 + (rand() % 3); jj++) {
            top = drawTextLineWithFont(hDC, normalFont, 100, top, L"Chapter inner text line.");
         }

         // extra space between chapters
         top += 100;
      }

      // finish drawing
      if (hDC) {
         litePDF.FinishPage(hDC);
      }

      lastBookmarkLevel = 0;

      // add bookmarks, when the drawing is finally finished and all pages are available
      sz = bookmarks.size();
      for (ii = 0; ii < sz; ii++) {
         const BookmarkData &bkmk = bookmarks[ii];

         if (bkmk.level == 0) {
            bookmarkLevels[0] = litePDF.CreateBookmarkRoot(bkmk.title.c_str(), bkmk.fontStyle,
               colors[colorIndex % 4], bkmk.pageIndex, bkmk.left_mm, bkmk.top_mm);
            lastBookmarkLevel = 0;
            colorIndex++;
         } else if (lastBookmarkLevel + 1 == bkmk.level) {
            bookmarkLevels[bkmk.level] = litePDF.CreateBookmarkChild(bookmarkLevels[lastBookmarkLevel],
               bkmk.title.c_str(), bkmk.fontStyle, RGB(0,0,0), bkmk.pageIndex, bkmk.left_mm, bkmk.top_mm);
         } else {
            bookmarkLevels[bkmk.level] = litePDF.CreateBookmarkSibling(bookmarkLevels[bkmk.level],
               bkmk.title.c_str(), bkmk.fontStyle, RGB(0,0,0), bkmk.pageIndex, bkmk.left_mm, bkmk.top_mm);
         }

         lastBookmarkLevel = bkmk.level;
      }

      if (linkPageIndex != -1) {
         unsigned int resourceID;
         wstring annotationText = L"Go to Chapter " + wstring(linkText);

         hDC = litePDF.AddResource(35, 6, 350, 60, LitePDFDrawFlag_SubstituteFonts);
         SetTextColor(hDC, RGB(0, 128, 255));
         drawTextLineWithFont(hDC, normalFont, 0, 0, annotationText.c_str());
         resourceID = litePDF.FinishResource(hDC);

         litePDF.CreateLinkAnnotation(0, 100, 30, 35, 6, LitePDFAnnotationFlag_None, resourceID,
            linkPageIndex, 10, linkY, annotationText.c_str());
      }

      litePDF.SaveToFile("bookmarks-1.pdf");

      // close the document
      litePDF.Close();

      // free allocated fonts
      DeleteObject(normalFont);
      DeleteObject(headingFonts[0]);
      DeleteObject(headingFonts[1]);
      DeleteObject(headingFonts[2]);
   } catch (TLitePDFException &ex) {
      fprintf (stderr, "litePDF Exception: %x: %s\n", ex.getCode(), ex.getMessage());
      res = 1;
   }

   return res;
}
