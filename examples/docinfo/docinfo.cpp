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
   int res = 0, ii;
   struct _docinfo {
      const char *key;
      const char *value;
   } docinfo[] = {
      { LitePDFDocumentInfo_Author, "Document's Author ìšèøžýáíéúùöÌŠÈØŽÝÁÍÉÚÙÖ §" },
      { LitePDFDocumentInfo_Creator, "Document's Creator" },
      { LitePDFDocumentInfo_Keywords, "Keyword1;Keyword2" },
      { LitePDFDocumentInfo_Subject, "Document's subject ìšèøžýáíéúùöÌŠÈØŽÝÁÍÉÚÙÖ §" },
      { LitePDFDocumentInfo_Title, "Document's Title ìšèøžýáíéúùöÌŠÈØŽÝÁÍÉÚÙÖ §" },
      { "CustomProperty", "CustomPropertyValue" },
      { LitePDFDocumentInfo_Producer, NULL }, // cannot be written
      { LitePDFDocumentInfo_Trapped, NULL },
      { LitePDFDocumentInfo_CreationDate, NULL }, // this is set automatically on save
      { LitePDFDocumentInfo_ModificationDate, NULL }, // this is set automatically on save
      { NULL, NULL }
   };

   using namespace litePDF;

   try {
      TLitePDF litePDF;

      // create a document
      litePDF.CreateMemDocument();

      // create a page
      addPage(litePDF, 210, 297, "Document with set information about an author and such");

      for (ii = 0; docinfo[ii].key; ii++) {
         if (docinfo[ii].value) {
            wchar_t wbuff[256];
            int wrote;

            wrote = MultiByteToWideChar(CP_ACP, 0,
                    docinfo[ii].value, strlen(docinfo[ii].value),
                    wbuff, sizeof(wbuff));
            wbuff[wrote] = 0;

            litePDF.SetDocumentInfo(docinfo[ii].key, wbuff);
            printf ("writing '%s' = '%s'\n", docinfo[ii].key, docinfo[ii].value);
         }
      }

      // save to file
      litePDF.SaveToFile("docinfo-1.pdf");

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // load from file
      litePDF.LoadFromFile("docinfo-1.pdf", NULL, false);

      for (ii = 0; docinfo[ii].key; ii++) {
         if (litePDF.GetDocumentInfoExists(docinfo[ii].key)) {
            std::wstring wstr;

            wstr = litePDF.GetDocumentInfo(docinfo[ii].key);

            char buff[256];
            int wrote;

            wrote = WideCharToMultiByte(CP_ACP, 0,
                    wstr.c_str(), wstr.length(),
                    buff, sizeof(buff), "?", FALSE);
            buff[wrote] = 0;

            if (docinfo[ii].value) {
               printf (" + key '%s' has value '%s'; expected:%s\n",
                       docinfo[ii].key, buff,
                       strcmp(buff, docinfo[ii].value) == 0 ? "yes" : "no");
            } else {
               printf (" + key '%s' has value '%s'\n", docinfo[ii].key, buff);
            }
         } else {
            printf (" - key '%s' not found\n", docinfo[ii].key);
         }
      }

      // close the document
      litePDF.Close();
   } catch (TLitePDFException &ex) {
      fprintf(stderr, "litePDF Exception: %x: %s\n", ex.getCode(), ex.getMessage());
      res = 1;
   }

   return res;
}
