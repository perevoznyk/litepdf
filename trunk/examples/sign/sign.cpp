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
#include <time.h>

#include "share/litePDF.h"

static void drawText(HDC hDC,
                     const char *msg,
                     int px,
                     int py,
                     int fontHeight,
                     COLORREF color)
{
   LOGFONTA lf = {0, };
   lf.lfHeight = fontHeight;
   strcpy(lf.lfFaceName, "Helvetica");

   HFONT fnt;
   HGDIOBJ oldFnt;

   fnt = CreateFontIndirect(&lf);
   oldFnt = SelectObject(hDC, fnt);

   SetTextColor(hDC, color);
   TextOut(hDC, px, py, msg, strlen(msg));

   SelectObject(hDC, oldFnt);
   DeleteObject(fnt);
}

static void addPage(litePDF::TLitePDF &litePDF,
                    unsigned int pageWidth,
                    unsigned int pageHeight,
                    const char *msg)
{
   HDC hDC;

   // add a new page, with large-enough pixel scale
   hDC = litePDF.AddPage(litePDF.MMToUnit(pageWidth), litePDF.MMToUnit(pageHeight),
                         pageWidth * 10, pageHeight * 10,
                         LitePDFDrawFlag_SubstituteFonts);

   // draw the text, with ~5mm font height
   drawText(hDC, msg, 100, 100, -50, RGB(0, 0, 0));

   // finish drawing
   litePDF.FinishPage(hDC);
}

static unsigned int createResource(litePDF::TLitePDF &litePDF,
                                   COLORREF color,
                                   const char *text)
{
   HDC hDC;
   int w = 50, h = 10;

   // create a new resource
   hDC = litePDF.AddResource(litePDF.MMToUnit(w), litePDF.MMToUnit(h), w * 10, h * 10, LitePDFDrawFlag_SubstituteFonts);

   LOGBRUSH brush;
   brush.lbColor = color;
   brush.lbHatch = 0;
   brush.lbStyle = BS_SOLID;

   HPEN pen = ExtCreatePen(PS_GEOMETRIC | PS_DOT | PS_ENDCAP_FLAT | PS_JOIN_BEVEL, 10, &brush, 0, NULL);
   HGDIOBJ prevPen = SelectObject(hDC, pen);

   // rectangle on boundaries
   MoveToEx(hDC, 0, 0, NULL);
   LineTo(hDC, w * 10, 0);
   LineTo(hDC, w * 10, h * 10);
   LineTo(hDC, 0, h * 10);
   LineTo(hDC, 0, 0);

   SelectObject(hDC, prevPen);
   DeleteObject(pen);

   // draw the text
   drawText(hDC, text, 20, 20, -50, color);

   // finish drawing
   return litePDF.FinishResource(hDC);
}

static BYTE *getFileAsData(const char *fileName, unsigned int *dataLength)
{
   FILE *f = fopen(fileName, "rb");
   if (!f) {
      std::string msg = "Failed to open " + std::string(fileName);
      throw TLitePDFException(ERROR_CANNOT_MAKE, msg.c_str());
   }

   if (fseek(f, 0, SEEK_END) != 0) {
      fclose(f);
      throw TLitePDFException(ERROR_CANNOT_MAKE,
                              "Failed to move to the end of the file");
   }

   *dataLength = ftell(f);

   if (fseek(f, 0, SEEK_SET) != 0) {
      fclose(f);
      throw TLitePDFException(ERROR_CANNOT_MAKE,
                              "Failed to move to the beginning of the file");
   }

   BYTE *data = (BYTE *) malloc(sizeof(BYTE) * (*dataLength));
   if (!data) {
      fclose(f);
      throw TLitePDFException(ERROR_OUTOFMEMORY, "Out of memory");
   }

   if (fread(data, sizeof(BYTE), *dataLength, f) != *dataLength) {
      fclose(f);
      free(data);
      throw TLitePDFException(ERROR_CANNOT_MAKE, "Failed to read whole file");
   }

   fclose(f);
   return data;
}

int main(void)
{
   int res = 0;
   BYTE *cert = NULL, *pkey = NULL;
   unsigned int certLen = 0, pkeyLen = 0;

   using namespace litePDF;

   try {
      TLitePDF litePDF;
      unsigned int signatureIndex, ii;

      // create a document
      litePDF.CreateMemDocument();

      // create some pages
      addPage(litePDF, 210, 297, "To be digitally signed document");
      addPage(litePDF, 210, 297, "Page 2");

      // add an unsigned signature field, to be signed by Alice & Bob later;
      // do not set other than appearances, otherwise Acrobat claims issues
      signatureIndex = litePDF.CreateSignature("AliceAndBobSig",
         0, // page index
         10, 110, 50, 10, // position and size
         LitePDFAnnotationFlag_None);

      // create signature apperances
      litePDF.SetSignatureAppearance(signatureIndex,
         LitePDFAppearance_Normal,
         createResource(litePDF, RGB(0, 0, 255), "Alice and Bob"),
         0, 0);

      litePDF.SetSignatureAppearance(signatureIndex,
         LitePDFAppearance_Rollover,
         createResource(litePDF, RGB(0, 255, 0), "Alice and Bob"),
         0, 0);

      litePDF.SetSignatureAppearance(signatureIndex,
         LitePDFAppearance_Down,
         createResource(litePDF, RGB(255, 0, 0), "Alice and Bob"),
         0, 0);

      // save a file with an unsigned signature
      litePDF.SaveToFile("sign-1.pdf");

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // load the saved document for incremental update
      litePDF.LoadFromFile("sign-1.pdf", NULL, false, true);

      // add a new signature, to be signed now
      signatureIndex = litePDF.CreateSignature("CharlieSig",
         0, // page index
         90, 110, 50, 10, // position and size
         LitePDFAnnotationFlag_None);

      // create signature apperance
      litePDF.SetSignatureAppearance(signatureIndex,
         LitePDFAppearance_Normal,
         createResource(litePDF, RGB(0, 0, 0), "Charlie"),
         0, 0);

      // set signature properties
      litePDF.SetSignatureDate(signatureIndex, 0); // '0' means now
      litePDF.SetSignatureReason(signatureIndex, L"Charlie agrees");
      litePDF.SetSignatureLocation(signatureIndex, L"Workplace A");

      // read Charlie's PEM certificate and private key
      cert = getFileAsData("charlie.pem", &certLen);
      pkey = getFileAsData("charlie.key", &pkeyLen);

      // add Charlie as the signer (no password used for the private key)
      litePDF.AddSignerPEM(cert, certLen, pkey, pkeyLen, NULL);

      free(cert); cert = NULL;
      free(pkey); pkey = NULL;

      // sign with Charlie
      litePDF.SaveToFileWithSign("sign-2.pdf", signatureIndex);

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // load the saved document for incremental update
      litePDF.LoadFromFile("sign-2.pdf", NULL, false, true);

      // find the 'AliceAndBobSig', which had been created above
      signatureIndex = ~0;

      for (ii = 0; ii < litePDF.GetSignatureCount(); ii++) {
         std::string name = litePDF.GetSignatureName(ii);
         if (name == "AliceAndBobSig") {
            signatureIndex = ii;
            break;
         }
      }

      if (signatureIndex == (unsigned int) ~0) {
         throw TLitePDFException(ERROR_CANNOT_MAKE, "Failed to find 'AliceAndBobSig' signature field");
      }

      // make sure the signing date is correct
      litePDF.SetSignatureDate(signatureIndex, 0); // '0' means now
      litePDF.SetSignatureReason(signatureIndex, L"Alice and Bob agree too");
      litePDF.SetSignatureLocation(signatureIndex, L"Meeting Room");

      // add Alice as the first signers; with password "alice"
      cert = getFileAsData("alice.pfx", &certLen);
      litePDF.AddSignerPFX(cert, certLen, "alice");
      free(cert); cert = NULL;

      // add bob as the second signers; no password used
      cert = getFileAsData("bob.pem", &certLen);
      pkey = getFileAsData("bob.key", &pkeyLen);
      litePDF.AddSignerPEM(cert, certLen, pkey, pkeyLen, NULL);
      free(cert); cert = NULL;
      free(pkey); pkey = NULL;

      // sign with Alice and Bob to the previously created signature
      litePDF.SaveToFileWithSign("sign-3.pdf", signatureIndex);

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------
   } catch (TLitePDFException &ex) {
      fprintf(stderr, "litePDF Exception: %x: %s\n", ex.getCode(), ex.getMessage());
      res = 1;
   }

   if (cert) {
      free(cert);
   }

   if (pkey) {
      free(pkey);
   }

   return res;
}
