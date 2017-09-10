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

static std::string to_string(unsigned int num)
{
   char buff[128];

   sprintf(buff, "%u", num);

   return std::string(buff);
}

static void addPage(litePDF::TLitePDF &litePDF,
                    unsigned int pageWidth,
                    unsigned int pageHeight,
                    const char *msg)
{
   // add a new page to it, with large-enough pixel scale
   HDC hDC = litePDF.AddPage(litePDF.MMToUnit(pageWidth), litePDF.MMToUnit(pageHeight),
                             pageWidth * 10, pageHeight * 10,
                             LitePDFDrawFlag_SubstituteFonts);

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

   // finish drawing
   litePDF.FinishPage(hDC);
}

int main(void)
{
   int res = 0;

   using namespace litePDF;

   try {
      TLitePDF litePDF;

      // create a document
      litePDF.CreateMemDocument();

      // check expected counts
      if (litePDF.GetEmbeddedFileCount() != 0) {
         std::string msg = "Newly created document reports non-zero (" +
                           to_string(litePDF.GetEmbeddedFileCount()) +
                           ") embedded files";
         throw TLitePDFException(ERROR_CANNOT_MAKE, msg.c_str());
      }

      // create a page
      addPage(litePDF, 210, 297, "Document with two attachments");

      const char *customData = "This is\na multiline\ncustom data.";
      litePDF.EmbedData("custom data.txt",
                        (const BYTE *) customData, strlen(customData));
      litePDF.EmbedFile("..\\attachments\\attachments.cpp");

      // check expected counts
      if (litePDF.GetEmbeddedFileCount() != 2) {
         std::string msg = "Newly created document reports other than two (" +
                           to_string(litePDF.GetEmbeddedFileCount()) +
                           ") embedded files";
         throw TLitePDFException(ERROR_CANNOT_MAKE, msg.c_str());
      }

      // save to file
      litePDF.SaveToFile("attachments-1.pdf");

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // load from file
      litePDF.LoadFromFile("attachments-1.pdf", NULL, true);

      // check expected counts
      if (litePDF.GetEmbeddedFileCount() != 2) {
         std::string msg = "Loaded document reports other than two (" +
                           to_string(litePDF.GetEmbeddedFileCount()) +
                           ") embedded files";
         throw TLitePDFException(ERROR_CANNOT_MAKE, msg.c_str());
      }

      unsigned int ii = 0, sz = litePDF.GetEmbeddedFileCount();
      for (ii = 0; ii < sz; ii++) {
         std::string fileName;

         fileName = litePDF.GetEmbeddedFileName(ii);
         printf("embedded file [%d]: fileName: '%s' ", ii, fileName.c_str());

         unsigned int dataLength = 0;
         if (!litePDF.GetEmbeddedFileData(ii, NULL, &dataLength)) {
            throw TLitePDFException(ERROR_CANNOT_MAKE,
                                    "Failed to get attachment data length");
         }

         BYTE *data = (BYTE *) malloc(sizeof(BYTE) * dataLength);
         if (!data) {
            std::string msg = "Failed to allocate " + to_string(dataLength) + " bytes";
            throw TLitePDFException(ERROR_OUTOFMEMORY, msg.c_str());
         }

         if (!litePDF.GetEmbeddedFileData(ii, data, &dataLength)) {
            free(data);
            throw TLitePDFException(ERROR_CANNOT_MAKE, "Failed to get attachment data");
         }

         printf("dataLength: %u\n", dataLength);
         printf("data:\n%.*s\n******************************************************\n",
                 dataLength, data);

         free(data);
      }

      // close the document
      litePDF.Close();
   } catch (TLitePDFException &ex) {
      fprintf(stderr, "litePDF Exception: %x: %s\n", ex.getCode(), ex.getMessage());
      res = 1;
   }

   return res;
}
