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
#include <wincrypt.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <time.h>

#include "share/litePDF.h"

// Link with the crypt32.lib file
#pragma comment (lib, "crypt32.lib")

static std::string to_string(unsigned int num)
{
   char buff[128];

   if (num == (unsigned int) -1) {
      return std::string("-1");
   }

   sprintf(buff, "%u", num);

   return std::string(buff);
}

class MPdfSigner
{
 private:
   unsigned int lastSignatureIndex;

   static void __stdcall appendSignatureData(const char *bytes,
                                             unsigned int bytes_len,
                                             void *user_data)
   {
      MPdfSigner *signer = (MPdfSigner *) user_data;

      if (bytes && bytes_len) {
         signer->AddData(bytes, bytes_len);
      }
   }

   static void __stdcall finishSignature(char *signature,
                                         unsigned int *signature_len,
                                         void *user_data)
   {
      MPdfSigner *signer = (MPdfSigner *) user_data;
      signer->Finish(signature, signature_len);
   }

   void loadCertificateStore(HANDLE *hStore)
   {
      BYTE *certBytes = NULL;
      unsigned int certBytesLength = 0;

      FILE *certFile = fopen("cert.pfx", "rb");
      if (!certFile) {
         throw TLitePDFException(ERROR_FILE_NOT_FOUND,
                                 "Failed to open 'cert.pfx'");
      }

      fseek(certFile, 0, SEEK_END);
      certBytesLength = ftell(certFile);
      fseek(certFile, 0, SEEK_SET);
      certBytes = (BYTE *) malloc(sizeof(BYTE) * certBytesLength);
      if (!certBytes) {
         throw TLitePDFException(ERROR_OUTOFMEMORY,
                                 "Failed to allocate memory for cert.pfx");
      }

      fread(certBytes, sizeof(BYTE), certBytesLength, certFile);

      fclose(certFile);

      HMODULE lib = LoadLibrary("crypt32.dll");
      if (lib != NULL) {
         /*
            PFXData...
         */
         typedef HCERTSTORE (WINAPI *LPPFXImportCertStore) (CRYPT_DATA_BLOB *pPFX,
                                                            LPCWSTR szPassword,
                                                            DWORD dwFlags);
         typedef BOOL (WINAPI *LPPFXIsPFXBlob)(CRYPT_DATA_BLOB *pPFX);
         typedef BOOL (WINAPI *LPPFXVerifyPassword)(CRYPT_DATA_BLOB *pPFX,
                                                    LPCWSTR szPassword,
                                                    DWORD dwFlags);
         #ifndef CRYPT_USER_KEYSET
         #define CRYPT_USER_KEYSET         0x00001000     
         #endif

         LPPFXImportCertStore libPFXImportCertStore =
               (LPPFXImportCertStore) GetProcAddress(lib,"PFXImportCertStore");
         LPPFXIsPFXBlob libPFXIsPFXBlob =
               (LPPFXIsPFXBlob) GetProcAddress(lib, "PFXIsPFXBlob");
         LPPFXVerifyPassword libPFXVerifyPassword =
               (LPPFXVerifyPassword) GetProcAddress(lib,"PFXVerifyPassword");
         if (libPFXImportCertStore != NULL &&
             libPFXIsPFXBlob != NULL &&
             libPFXVerifyPassword != NULL) {
            CRYPT_DATA_BLOB blob;
            blob.cbData = certBytesLength;
            blob.pbData = certBytes;
            if (libPFXIsPFXBlob(&blob)) {
               *hStore = libPFXImportCertStore(&blob, L"",
                     0/*|CRYPT_USER_PROTECTED|CRYPT_USER_KEYSET*/);
            }
         }
         FreeLibrary(lib);
      }

      free(certBytes);
   }

   char *bytes;
   int bytes_len;

   void AddData(const char *pBytes, unsigned int pBytes_len)
   {
      bytes = (char *) realloc(bytes, sizeof(char) * (bytes_len + pBytes_len));
      if (!bytes) {
         std::string msg = "Failed to allocate " +
                           to_string(bytes_len + pBytes_len) + " bytes";
         throw TLitePDFException(ERROR_OUTOFMEMORY, msg.c_str());
      }

      memcpy(bytes + bytes_len, pBytes, pBytes_len);
      bytes_len += pBytes_len;
   }

   void Finish(char *signature, unsigned int *signature_len)
   {
      HANDLE hStore = NULL;
      PCCERT_CONTEXT pCertContext = NULL;
      const BYTE* MessageArray[] = {(const BYTE *) bytes};
      DWORD MessageSizeArray[1];
      MessageSizeArray[0] = bytes_len;
      CRYPT_SIGN_MESSAGE_PARA  SigParams;

      loadCertificateStore(&hStore);
      if (!hStore) {
         throw TLitePDFException(GetLastError(),
                                 "Failed to open a temporary store");
      }

      pCertContext = CertFindCertificateInStore(hStore,
            PKCS_7_ASN_ENCODING | X509_ASN_ENCODING, 0, CERT_FIND_ANY,
            NULL, NULL);
      if (!pCertContext) {
         CertCloseStore(hStore, 0);
         throw TLitePDFException(GetLastError(),
                                 "Failed to find certificate in the store");
      }

      memset(&SigParams,0,sizeof(CRYPT_SIGN_MESSAGE_PARA));

      SigParams.cbSize = sizeof(CRYPT_SIGN_MESSAGE_PARA);
      SigParams.dwMsgEncodingType = PKCS_7_ASN_ENCODING | X509_ASN_ENCODING;
      SigParams.pSigningCert = pCertContext;
      SigParams.HashAlgorithm.pszObjId = szOID_RSA_SHA1RSA;
      SigParams.HashAlgorithm.Parameters.pbData = NULL;
      SigParams.HashAlgorithm.Parameters.cbData = 0;
      SigParams.pvHashAuxInfo = NULL;
      SigParams.cMsgCert = 1;
      SigParams.rgpMsgCert = &pCertContext;
      SigParams.cMsgCrl = 0;
      SigParams.rgpMsgCrl = NULL;
      SigParams.cAuthAttr = 0;
      SigParams.rgAuthAttr = NULL;
      SigParams.cUnauthAttr = 0;
      SigParams.rgUnauthAttr = NULL;
      SigParams.dwFlags = 0;
      SigParams.dwInnerContentType = 0;

      BYTE *bsignature = (BYTE *) signature;
      DWORD bsignature_len = (DWORD) *signature_len;

      if (!CryptSignMessage(
            &SigParams,            // Signature parameters
            TRUE,                  // detached ?
            1,                     // Number of messages
            MessageArray,          // Messages to be signed
            MessageSizeArray,      // Size of messages
            bsignature,            // Buffer for signed message
            &bsignature_len)) {
         CertFreeCertificateContext(pCertContext);
         CertCloseStore(hStore, 0);
         *signature_len = bsignature_len;

         throw TLitePDFException(GetLastError(), "Failed to sign data");
      }

      *signature_len = bsignature_len;
      CertFreeCertificateContext(pCertContext);
      CertCloseStore(hStore, 0);
   }

   unsigned int CreateSignatureField(litePDF::TLitePDF &litePDF,
                                     const char *signatureName,
                                     __int64 dateOfSign,
                                     unsigned int annotationResourceID,
                                     unsigned int annotationPageIndex,
                                     RECT annotationPosition_mm,
                                     unsigned int annotationFlags,
                                     int signatureLen)
   {
      unsigned int signatureIndex;

      litePDF.SetSignatureSize(signatureLen);

      signatureIndex = litePDF.CreateSignature(signatureName,
                                               annotationPageIndex,
                                               annotationPosition_mm.left,
                                               annotationPosition_mm.top,
                                               annotationPosition_mm.right - annotationPosition_mm.left,
                                               annotationPosition_mm.bottom - annotationPosition_mm.top,
                                               annotationFlags);

      litePDF.SetSignatureReason(signatureIndex, L"litePDF example");
      litePDF.SetSignatureDate(signatureIndex, dateOfSign);

      if (annotationResourceID && annotationPageIndex < litePDF.GetPageCount() ) {
         litePDF.SetSignatureAppearance(signatureIndex, LitePDFAppearance_Normal, annotationResourceID, 0, 0);
      }

      return signatureIndex;
   }

 public:
   MPdfSigner()
   {
      bytes = NULL;
      bytes_len = 0;
      lastSignatureIndex = ~0;
   }

   ~MPdfSigner()
   {
      Clear();
   }

   void Clear(void)
   {
      if (bytes) {
         free(bytes);
         bytes = NULL;
      }
      bytes_len = 0;
      lastSignatureIndex = ~0;
   }

   void SignToFile(litePDF::TLitePDF &litePDF,
                   const char *fileName,
                   const char *signatureName)
   {
      RECT rect_mm = {0, 0, 0, 0};

      SignToFileEx(litePDF, fileName, signatureName, 0, 0, 0, rect_mm, 0, 0);
   }

   void SignToFileEx(litePDF::TLitePDF &litePDF,
                     const char *fileName,
                     const char *signatureName,
                     __int64 dateOfSign,
                     unsigned int annotationResourceID,
                     unsigned int annotationPageIndex,
                     RECT annotationPosition_mm,
                     unsigned int annotationFlags,
                     int signatureLen)
   {
      unsigned int signatureIndex;

      signatureIndex = CreateSignatureField(litePDF,
                                            signatureName,
                                            dateOfSign,
                                            annotationResourceID,
                                            annotationPageIndex,
                                            annotationPosition_mm,
                                            annotationFlags,
                                            signatureLen);

      litePDF.SaveToFileWithSignManual(fileName,
                                       signatureIndex,
                                       appendSignatureData, this,
                                       finishSignature, this);
   }

   bool SignToData(litePDF::TLitePDF &litePDF,
                   BYTE *data,
                   unsigned int *dataLength,
                   const char *signatureName)
   {
      RECT rect_mm = {0, 0, 0, 0};

      return SignToDataEx(litePDF, data, dataLength,
                          signatureName,
                          0, 0, 0, rect_mm, 0, 0);
   }

   bool SignToDataEx(litePDF::TLitePDF &litePDF,
                     BYTE *data,
                     unsigned int *dataLength,
                     const char *signatureName,
                     __int64 dateOfSign,
                     unsigned int annotationResourceID,
                     unsigned int annotationPageIndex,
                     RECT annotationPosition_mm,
                     unsigned int annotationFlags,
                     int signatureLen)
   {
      unsigned int signatureIndex = lastSignatureIndex;

      if (signatureIndex == (unsigned int) ~0) {
         signatureIndex = CreateSignatureField(litePDF,
                                               signatureName,
                                               dateOfSign,
                                               annotationResourceID,
                                               annotationPageIndex,
                                               annotationPosition_mm,
                                               annotationFlags,
                                               signatureLen);

         // remember the used signature index for the second call,
         // when populating the 'data'
         lastSignatureIndex = signatureIndex;
      } else {
         signatureIndex = lastSignatureIndex;
      }

      return litePDF.SaveToDataWithSignManual(signatureIndex,
                                              appendSignatureData, this,
                                              finishSignature, this,
                                              data, dataLength);
   }
};

static void drawText(HDC hDC,
                     const char *msg,
                     int px,
                     int py,
                     int fontHeight)
{
   LOGFONTA lf = {0, };
   lf.lfHeight = fontHeight;
   strcpy(lf.lfFaceName, "Helvetica");

   HFONT fnt;
   HGDIOBJ oldFnt;

   fnt = CreateFontIndirect(&lf);
   oldFnt = SelectObject(hDC, fnt);

   SetTextColor(hDC, RGB(0, 0, 0));
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
   drawText(hDC, msg, 100, 100, -50);

   // finish drawing
   litePDF.FinishPage(hDC);
}

static unsigned int createResource(litePDF::TLitePDF &litePDF, const char *secondLine)
{
   HDC hDC;
   int w = 25, h = 8;

   // create a new resource
   hDC = litePDF.AddResource(litePDF.MMToUnit(w), litePDF.MMToUnit(h), w * 20, h * 20, LitePDFDrawFlag_SubstituteFonts);

   LOGBRUSH brush;
   brush.lbColor = RGB(128,128,128);
   brush.lbHatch = 0;
   brush.lbStyle = BS_SOLID;

   HPEN pen = ExtCreatePen(PS_GEOMETRIC | PS_DOT | PS_ENDCAP_FLAT | PS_JOIN_BEVEL, 10, &brush, 0, NULL);
   HGDIOBJ prevPen = SelectObject(hDC, pen);

   // rectangle on boundaries
   MoveToEx(hDC, 0, 0, NULL);
   LineTo(hDC, w * 20, 0);
   LineTo(hDC, w * 20, h * 20);
   LineTo(hDC, 0, h * 20);
   LineTo(hDC, 0, 0);

   SelectObject(hDC, prevPen);
   DeleteObject(pen);

   // draw the text
   drawText(hDC, "litePDF example", 50, 20, -50);
   drawText(hDC, secondLine, 50, 70, -50);

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
   BYTE *data = NULL, *oldSignature = NULL, *newSignature = NULL;
   unsigned int oldSignatureLen = 0, newSignatureLen = 0;

   using namespace litePDF;

   try {
      MPdfSigner signer;
      TLitePDF litePDF;

      // create a document
      litePDF.CreateMemDocument();

      // create some pages
      addPage(litePDF, 297, 210, "Digitally signed document");
      addPage(litePDF, 297, 210, "Page 2");

      // save signed to file
      signer.Clear();
      signer.SignToFile(litePDF, "sign-1.pdf", "Sig1");

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // create a document
      litePDF.CreateMemDocument();

      // create some pages
      addPage(litePDF, 297, 210, "Digitally signed document (2)");
      addPage(litePDF, 297, 210, "Page 2");

      // prepare the signer (clear data from the previous run)
      signer.Clear();

      // sign to data
      unsigned int dataLength = 0;
      if (!signer.SignToData(litePDF, NULL, &dataLength, "Sig2")) {
         std::string msg = "Failed to sign document to data (pass 1): " +
                           std::string(litePDF.getLastErrorMessage());
         throw TLitePDFException(ERROR_INVALID_DATA, msg.c_str());
      }

      data = (BYTE *) malloc(sizeof(BYTE) * dataLength);
      if (!data) {
         std::string msg = "Failed to allocate " + to_string(dataLength) + " bytes";
         throw TLitePDFException(ERROR_OUTOFMEMORY, msg.c_str());
      }

      if (!signer.SignToData(litePDF, data, &dataLength, "Sig2")) {
         std::string msg = "Failed to sign document to data (pass 2): " +
                           std::string(litePDF.getLastErrorMessage());
         throw TLitePDFException(ERROR_INVALID_DATA, msg.c_str());
      }

      // write data to file
      FILE *f = fopen("sign-2.pdf", "wb");
      if (!f) {
         throw TLitePDFException(ERROR_INVALID_DATA, "Failed to open 'sign-2.pdf'");
      }

      if (fwrite(data, sizeof(BYTE), dataLength, f) != dataLength) {
         fclose(f);
         throw TLitePDFException(ERROR_INVALID_DATA,
                                 "Failed to write all data to 'sign-2.pdf'");
      }

      fclose(f);

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // create a document and save it with visual appearance of a signature
      litePDF.CreateMemDocument();

      // create some pages
      addPage(litePDF, 297, 210, "Digitally signed document (incremental)");
      addPage(litePDF, 297, 210, "Page 2");

      // create also visual appearance for this signature, on the first page
      RECT where_mm = { litePDF.MMToUnit(90),      litePDF.MMToUnit(5),
                        litePDF.MMToUnit(90 + 25), litePDF.MMToUnit(5 + 8) };

      time_t dataOfSign;

      time(&dataOfSign);
      // two days ago
      dataOfSign -= 2 * 24 * 60 * 60;

      // sign to a new file
      signer.Clear();
      signer.SignToFileEx(litePDF, "sign-3.pdf", "Sig3", dataOfSign,
         createResource(litePDF, "1st Signature"),
         0,
         where_mm,
         LitePDFAnnotationFlag_None,
         1280);

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // copy the file for testing
      if (!CopyFile("sign-3.pdf", "sign-4.pdf", FALSE)) {
         throw TLitePDFException(GetLastError(),
                                 "Failed to copy sign-3.pdf to sign-4.pdf");
      }

      //-----------------------------------------------------------------

      // remember to load for update to add signatures to already signed documents;
      // (uses 'loadCompletely=true' to be able to overwrite the file)
      // Note that this invalidates 'Sig3' signature when used without the authorization
      // key, due to the modification of the first page content.
      litePDF.LoadFromFile("sign-4.pdf", NULL, true, true);

      // check whether is already signed
      if (!litePDF.GetDocumentIsSigned()) {
         throw TLitePDFException(ERROR_INVALID_DATA,
               "Expected the opened file already signed, but it is not");
      }

      // there should be only one signature
      if (litePDF.GetSignatureCount() != 1) {
         std::string str;
         str = "Expected the opened file has one signature, but it has " +
            to_string(litePDF.GetSignatureCount()) + " signatures";
         throw TLitePDFException(ERROR_INVALID_DATA, str.c_str());
      }

      // get the first (current) signature
      if (!litePDF.GetSignatureData(0, NULL, &oldSignatureLen)) {
         throw TLitePDFException(ERROR_INVALID_DATA,
            "Failed to get the first signature length");
      }

      if (!oldSignatureLen) {
         throw TLitePDFException(ERROR_INVALID_DATA,
            "Failed to get the first signature length value");
      }

      if (oldSignatureLen != 1280) {
         std::string str;
         str = "Expected 1280 bytes long signature, but reported is " +
            to_string(oldSignatureLen) + " bytes";
         throw TLitePDFException(ERROR_INVALID_DATA, str.c_str());
      }

      oldSignature = (BYTE *) malloc(sizeof(BYTE) * oldSignatureLen);
      if (!oldSignature) {
         std::string msg = "Failed to allocate " + to_string(oldSignatureLen) + " bytes";
         throw TLitePDFException(ERROR_OUTOFMEMORY, msg.c_str());
      }

      if (!litePDF.GetSignatureData(0, oldSignature, &oldSignatureLen)) {
         throw TLitePDFException(ERROR_INVALID_DATA, "Failed to get the first signature data");
      }

      unsigned int ii;
      printf ("Signature[0] of sign-4.pdf is %d bytes long:", oldSignatureLen);
      for (ii = 0; ii < oldSignatureLen; ii++) {
         if ((ii % 16) == 0) {
            printf ("\n   ");
         } else {
            printf (" ");
            if ((ii % 8) == 0) {
               printf ("  ");
            }
         }

         printf ("%02x", oldSignature[ii]);
      }
      printf ("\n");

      // add another signature
      signer.Clear();
      signer.SignToFile(litePDF, "sign-4.pdf", "Sig4");

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      if (data) {
         free(data);
      }

      data = getFileAsData("sign-4.pdf", &dataLength);

      //-----------------------------------------------------------------

      // add yet another signature and compare the first signature that
      // it did not change

      litePDF.LoadFromData(data, dataLength, NULL, true);

      // check whether is already signed
      if (!litePDF.GetDocumentIsSigned()) {
         throw TLitePDFException(ERROR_INVALID_DATA,
               "Expected the opened file already signed, but it is not");
      }

      // there should be only one signature
      if (litePDF.GetSignatureCount() != 2) {
         std::string str;
         str = "Expected the opened file has two signatures, but it has " +
            to_string(litePDF.GetSignatureCount()) + " signatures";
         throw TLitePDFException(ERROR_INVALID_DATA, str.c_str());
      }

      for (ii = 0; ii < 2; ii++) {
         free(data);
         data = NULL;

         if (newSignature) {
            free(newSignature);
            newSignature = NULL;
         }

         newSignatureLen = 0;

         // get the signature data
         if (!litePDF.GetSignatureData(ii, NULL, &newSignatureLen)) {
            std::string msg = "Failed to get signature[" + to_string(ii) + "] length";
            throw TLitePDFException(ERROR_INVALID_DATA, msg.c_str());
         }

         if (!newSignatureLen) {
            std::string msg = "Failed to get signature[" + to_string(ii) + "] length value";
            throw TLitePDFException(ERROR_INVALID_DATA, msg.c_str());
         }

         newSignature = (BYTE *) malloc(sizeof(BYTE) * newSignatureLen);
         if (!newSignature) {
            std::string msg = "Failed to allocate " + to_string(newSignatureLen) + " bytes";
            throw TLitePDFException(ERROR_OUTOFMEMORY, msg.c_str());
         }

         if (!litePDF.GetSignatureData(ii, newSignature, &newSignatureLen)) {
            std::string msg = "Failed to get signature[" + to_string(ii) + "] data";
            throw TLitePDFException(ERROR_INVALID_DATA, msg.c_str());
         }

         if (ii == 0) {
            if (newSignatureLen != oldSignatureLen ||
                memcmp(oldSignature, newSignature, newSignatureLen) != 0) {
               throw TLitePDFException(ERROR_INVALID_DATA, "The first signature may not change");
            }
         } else {
            if (newSignatureLen == oldSignatureLen &&
                memcmp(oldSignature, newSignature, newSignatureLen) == 0) {
               throw TLitePDFException(ERROR_INVALID_DATA, "The first and the second signature may not match");
            }
         }
      }

      if (data) {
         free(data);
         data = NULL;
      }
      dataLength = 0;

      // add another signature, but sign to data this time
      signer.Clear();

      dataLength = 0;
      if (!signer.SignToDataEx(litePDF, NULL, &dataLength, "Sig5", 0,
            createResource(litePDF, "2nd Signature"), 1, where_mm, 0, 0)) {
         std::string msg = "Failed to IncSign document to data (pass 1): " +
                           std::string(litePDF.getLastErrorMessage());
         throw TLitePDFException(ERROR_INVALID_DATA, msg.c_str());
      }

      data = (BYTE *) malloc(sizeof(BYTE) * dataLength);
      if (!data) {
         std::string msg = "Failed to allocate " + to_string(dataLength) + " bytes";
         throw TLitePDFException(ERROR_OUTOFMEMORY, msg.c_str());
      }

      // no need to call SignToDataEx again, this receives only the data
      if (!signer.SignToData(litePDF, data, &dataLength, "Sig5")) {
         std::string msg = "Failed to IncSign document to data (pass 2): " +
                           std::string(litePDF.getLastErrorMessage());
         throw TLitePDFException(ERROR_INVALID_DATA, msg.c_str());
      }

      if (!data || !dataLength) {
         throw TLitePDFException(ERROR_INVALID_DATA, "Failed to IncSign to data");
      }

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      f = fopen("sign-5.pdf", "wb");
      if (!f) {
         throw TLitePDFException(ERROR_INVALID_DATA, "Failed to open sign-5.pdf for writing");
      }

      if (fwrite(data, sizeof(BYTE), dataLength, f) != dataLength) {
         fclose(f);
         throw TLitePDFException(ERROR_INVALID_DATA, "Failed to write data to sign-5.pdf");
      }

      fclose(f);

      //-----------------------------------------------------------------
   } catch (TLitePDFException &ex) {
      fprintf(stderr, "litePDF Exception: %x: %s\n", ex.getCode(), ex.getMessage());
      res = 1;
   }

   if (data) {
      free(data);
   }

   if (oldSignature) {
      free(oldSignature);
   }

   if (newSignature) {
      free(newSignature);
   }

   return res;
}
