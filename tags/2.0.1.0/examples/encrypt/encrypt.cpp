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

static void drawPage(litePDF::TLitePDF &litePDF,
                     const char *msg)
{
   // add a new page to it, with large-enough pixel scale
   HDC hDC = litePDF.AddPage(litePDF.MMToUnit(210), litePDF.MMToUnit(297), 2100, 2970, LitePDFDrawFlag_SubstituteFonts);

   // draw the text
   LOGFONTA lf = {0, };
   lf.lfHeight = -50; // ~5mm
   strcpy(lf.lfFaceName, "Helvetica");

   HFONT fnt;
   HGDIOBJ oldFnt;

   fnt = CreateFontIndirect(&lf);
   oldFnt = SelectObject(hDC, fnt);

   SetTextColor(hDC, RGB(128, 0, 0));
   TextOut(hDC, 100, 100, msg, strlen(msg));

   SelectObject(hDC, oldFnt);
   DeleteObject(fnt);

   // finish drawing
   litePDF.FinishPage(hDC);
}

static void createEncryptedFiles(void)
{
   using namespace litePDF;
   TLitePDF litePDF;

   //-----------------------------------------------------------------

   // setup encryption to be used when creating the document
   litePDF.PrepareEncryption(NULL, "owner",
                             LitePDFEncryptPermission_All,
                             LitePDFEncryptAlgorithm_RC4V1);

   // begin write-only PDF file
   litePDF.CreateFileDocument("encrypt-rc4v1-no-user-pass.pdf");

   // fill a page
   drawPage(litePDF, "Encrypted, without user password, RC4 V1");

   // close the document
   litePDF.Close();

   //-----------------------------------------------------------------

   // setup encryption to be used when creating the document
   litePDF.PrepareEncryption("user", "owner",
                             LitePDFEncryptPermission_All,
                             LitePDFEncryptAlgorithm_RC4V2);

   // begin memory-based PDF document
   litePDF.CreateMemDocument();

   // fill a page
   drawPage(litePDF, "Encrypted, with user and owner password, RC4 V2");

   // save to file
   litePDF.SaveToFile("encrypt-rc4v2.pdf");

   // close the document
   litePDF.Close();

   //-----------------------------------------------------------------

   // setup encryption to be used when creating the document
   litePDF.PrepareEncryption("user", "owner",
                             LitePDFEncryptPermission_All,
                             LitePDFEncryptAlgorithm_AESV2);

   // begin memory-based PDF document
   litePDF.CreateMemDocument();

   // fill a page
   drawPage(litePDF, "Encrypted, with user and owner password, AES V2");

   // save to file
   litePDF.SaveToFile("encrypt-aesv2.pdf");

   // close the document
   litePDF.Close();

   //-----------------------------------------------------------------

   // setup encryption to be used when creating the document
   litePDF.PrepareEncryption("user", "owner",
                             LitePDFEncryptPermission_All,
                             LitePDFEncryptAlgorithm_AESV3);

   // begin memory-based PDF document
   litePDF.CreateMemDocument();

   // fill a page
   drawPage(litePDF, "Encrypted, with user and owner password, AES V3");

   // save to file
   litePDF.SaveToFile("encrypt-aesv3.pdf");

   // close the document
   litePDF.Close();

   //-----------------------------------------------------------------

   // setup encryption to be used when creating the document
   litePDF.PrepareEncryption("user", "owner",
                             LitePDFEncryptPermission_All,
                             LitePDFEncryptAlgorithm_AESV2);

   try {
      // begin file-based PDF document
      litePDF.CreateFileDocument("encrypt-aesv2-file.pdf");

      throw TLitePDFException(ERROR_CANNOT_MAKE,
            "Should fail with an exception, because AES encryption works only with memory-based documents currently");
   } catch (TLitePDFException &ex) {
      if (ex.getCode() != ERROR_NOT_SUPPORTED) {
         throw TLitePDFException(ex);
      }
   }

   // close the document
   litePDF.Close();

   //-----------------------------------------------------------------

   // setup encryption to be used when creating the document
   litePDF.PrepareEncryption("user", "owner",
                             LitePDFEncryptPermission_All,
                             LitePDFEncryptAlgorithm_AESV3);

   try {
      // begin file-based PDF document
      litePDF.CreateFileDocument("encrypt-aesv3-file.pdf");

      throw TLitePDFException(ERROR_CANNOT_MAKE,
            "Should fail with an exception, because AES encryption works only with memory-based documents currently");
   } catch (TLitePDFException &ex) {
      if (ex.getCode() != ERROR_NOT_SUPPORTED) {
         throw TLitePDFException(ex);
      }
   }

   // close the document
   litePDF.Close();
}

static void checkDocumentDecrypt(litePDF::TLitePDF &litePDF)
{
   std::wstring value;

   value = litePDF.GetDocumentInfo(LitePDFDocumentInfo_Producer);

   if (value.find_first_of(L"litePDF") == value.npos) {
      fprintf (stderr, "Text 'litePDF' not found in '%S'\n", value.c_str());
   }

}

int main(void)
{
   int res = 0;

   using namespace litePDF;

   try {
      TLitePDF litePDF;

      // create the files
      createEncryptedFiles();

      //-----------------------------------------------------------------
      // now try to open them
      //-----------------------------------------------------------------

      // no user password, then open it as a user
      litePDF.LoadFromFile("encrypt-rc4v1-no-user-pass.pdf", NULL, false);

      checkDocumentDecrypt(litePDF);

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      try {
         // this should fail, because no password was provided
         litePDF.LoadFromFile("encrypt-rc4v2.pdf", NULL, false);

         throw TLitePDFException(ERROR_CANNOT_MAKE,
               "Should fail to open encrypted file without provided password");
      } catch (TLitePDFException &ex) {
         if (ex.getCode() != ERROR_WRONG_PASSWORD) {
            throw TLitePDFException(ex);
         }

         // re-try with user's password
         litePDF.LoadFromFile("encrypt-rc4v2.pdf", "user", false);

         checkDocumentDecrypt(litePDF);
      }

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // try to open as owner
      litePDF.LoadFromFile("encrypt-rc4v1-no-user-pass.pdf", "owner", false);

      checkDocumentDecrypt(litePDF);

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // try to open as user
      litePDF.LoadFromFile("encrypt-rc4v2.pdf", "user", false);

      checkDocumentDecrypt(litePDF);

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // try to open as owner
      litePDF.LoadFromFile("encrypt-rc4v2.pdf", "owner", false);

      checkDocumentDecrypt(litePDF);

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // try to open as user
      litePDF.LoadFromFile("encrypt-aesv2.pdf", "user", false);

      checkDocumentDecrypt(litePDF);

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // try to open as owner
      litePDF.LoadFromFile("encrypt-aesv2.pdf", "owner", false);

      checkDocumentDecrypt(litePDF);

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // try to open as user
      litePDF.LoadFromFile("encrypt-aesv3.pdf", "user", false);

      checkDocumentDecrypt(litePDF);

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // try to open as owner
      litePDF.LoadFromFile("encrypt-aesv3.pdf", "owner", false);

      checkDocumentDecrypt(litePDF);

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

   } catch (TLitePDFException &ex) {
      fprintf(stderr, "litePDF Exception: %x: %s\n", ex.getCode(), ex.getMessage());
      res = 1;
   }

   return res;
}
