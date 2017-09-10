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
#include "podofo/podofo.h"

// fills PDF content with a text from a helloworld example
void fillContent(litePDF::TLitePDF &litePDF)
{
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
}

int main(void)
{
   int res = 0;

   using namespace litePDF;
   using namespace PoDoFo;

   try {
      TLitePDF litePDF;

      // create an in-memory PDF doument
      litePDF.CreateMemDocument();

      // fill some content
      fillContent(litePDF);

      // obtain current PoDoFo document pointer
      PdfDocument *document = (PdfDocument *) litePDF.GetPoDoFoDocument();
      if (!document) {
         throw TLitePDFException(ERROR_INVALID_HANDLE,
                                 "Failed to obtain PoDoFo document");
      }

      // traverse all document objects and count those with streams
      int streams = 0, total = 0;
      PdfVecObjects *objs = document->GetObjects();
      if (!objs) {
         throw TLitePDFException(ERROR_INVALID_HANDLE,
                                 "Failed to get PdfVecObjects");
      }

      std::vector<PdfObject *>::const_iterator it, end = objs->end();
      for (it = objs->begin(); it != end; it++, total++) {
         PdfObject *obj = *it;
         if (obj && obj->HasStream()) {
            const PdfStream *strm = obj->GetStream();
            if (strm) {
               streams++;
            }
         }
      }

      // print the result
      printf ("Found %d out of %d objects with stream\n", streams, total);

      // close the document, without saving it
      litePDF.Close();
   } catch (TLitePDFException &ex) {
      fprintf (stderr, "litePDF Exception: %x: %s\n", ex.getCode(), ex.getMessage());
      res = 1;
   }

   return res;
}
