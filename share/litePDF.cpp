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

//---------------------------------------------------------------------------
#include <windows.h>
#include <stdio.h>
#include <string>

#ifdef LITEPDF_USE_VCL_EXCEPTION
#include <vcl.h>

#pragma hdrstop
#endif

#include "litePDF.h"

// link to version.lib, due to checkAPIVersion()
#ifdef _MSC_VER
#pragma comment (lib, "version.lib")
#else // _MSC_VER
#ifdef _BORLANDC
#pragma link "version.lib"
#endif // _BORLANDC
#endif // _MSC_VER

namespace litePDF {

#define ThrowIfFail(_expr) do {                                                \
      if (!(_expr)) {                                                          \
         std::string ttmsg;                                                    \
         ttmsg = std::string(_func) + ": Assertion '" + (#_expr) + "' failed"; \
         throw TLitePDFException(ERROR_INVALID_PARAMETER, ttmsg.c_str());      \
      }                                                                        \
   } while (false)

#define ThrowMessageIfFail(_expr, _msg) do {                                   \
      if (!(_expr)) {                                                          \
         std::string ttmsg = std::string(_func) + ": " + (_msg);               \
         throw TLitePDFException(ERROR_INVALID_PARAMETER, ttmsg.c_str());      \
      }                                                                        \
   } while (false)

#define ThrowLastErrorIfFail(_expr) do {                                       \
      if (!(_expr)) {                                                          \
         std::string ttmsg = std::string("Failed to call '") + (_func) + "'";  \
         const char *msg = getLastErrorMessage();                              \
         if (!msg) {                                                           \
            msg = ttmsg.c_str();                                               \
         }                                                                     \
         throw TLitePDFException(getLastErrorCode(), msg);                     \
      }                                                                        \
   } while (false)

#define InitFunc(_ret,_name,_params)                  \
   freeLastError();                                   \
   typedef _ret (__stdcall FAR *LP ## _name) _params; \
   LP ## _name func;                                  \
                                                      \
   func = (LP ## _name) GetProc(# _name);

#define FreePtr(_ptr) do { if ((_ptr)) { free((_ptr)); (_ptr) = NULL; } } while (0)

//---------------------------------------------------------------------------

#ifdef LITEPDF_USE_VCL_EXCEPTION
__fastcall TLitePDFException::TLitePDFException(DWORD pCode, const char *pMsg) : Sysutils::Exception(pMsg)
#else
TLitePDFException::TLitePDFException(DWORD pCode, const char *pMsg)
#endif
{
   code = pCode;
   #ifndef LITEPDF_USE_VCL_EXCEPTION
   if (pMsg) {
      msg = strdup(pMsg);
   } else {
      msg = NULL;
   }
   #endif
}
//---------------------------------------------------------------------------

#ifdef LITEPDF_USE_VCL_EXCEPTION
__fastcall TLitePDFException::TLitePDFException(const TLitePDFException &src) : Sysutils::Exception(src)
#else
TLitePDFException::TLitePDFException(const TLitePDFException &src)
#endif
{
   code = src.getCode();
   #ifdef LITEPDF_USE_VCL_EXCEPTION
   Message = src.Message;
   #else
   if (src.getMessage()) {
      msg = strdup(src.getMessage());
   } else {
      msg = NULL;
   }
   #endif
}
//---------------------------------------------------------------------------

#ifdef LITEPDF_USE_VCL_EXCEPTION
__fastcall TLitePDFException::~TLitePDFException()
#else
TLitePDFException::~TLitePDFException()
#endif
{
   #ifndef LITEPDF_USE_VCL_EXCEPTION
   FreePtr(msg);
   #endif
}
//---------------------------------------------------------------------------

DWORD TLitePDFException::getCode(void) const
{
   return code;
}
//---------------------------------------------------------------------------

#ifndef LITEPDF_USE_VCL_EXCEPTION
const char *TLitePDFException::getMessage(void) const
{
   return msg;
}
#endif // !LITEPDF_USE_VCL_EXCEPTION
//---------------------------------------------------------------------------

TLitePDF::TLitePDF()
{
   lib = NULL;
   context = NULL;
   lastErrorCode = 0;
   lastErrorMessage = NULL;
   onError = NULL;
   onErrorUserData = NULL;
   onEvalFontFlag = NULL;
   onEvalFontFlagUserData = NULL;
}
//---------------------------------------------------------------------------

TLitePDF::~TLitePDF()
{
   unloadLibrary();

   FreePtr(lastErrorMessage);
}
//---------------------------------------------------------------------------

FARPROC TLitePDF::GetProc(const char *pProcIdent)
{
   const char *_func = "TLitePDF::GetProc";

   ensureLibraryLoaded(_func);

   ThrowIfFail(pProcIdent != NULL);
   ThrowIfFail(lib != NULL);

   FARPROC res = NULL;
   res = GetProcAddress(lib, pProcIdent);

   char err[1024];
   sprintf(err, "Proc '%s' not found", pProcIdent);

   ThrowMessageIfFail(res != NULL, err);

   return res;
}
//---------------------------------------------------------------------------

void TLitePDF::setOnError(MLitePDFErrorEvent pOnError,
                          void *pOnErrorUserData)
{
   onError = pOnError;
   onErrorUserData = pOnErrorUserData;
}
//---------------------------------------------------------------------------

DWORD TLitePDF::getLastErrorCode(void) const
{
   return lastErrorCode;
}
//---------------------------------------------------------------------------

const char *TLitePDF::getLastErrorMessage(void) const
{
   return lastErrorMessage;
}
//---------------------------------------------------------------------------

void TLitePDF::freeLastError(void)
{
   FreePtr(lastErrorMessage);
   lastErrorCode = 0;
}
//---------------------------------------------------------------------------

void TLitePDF::setLastError(DWORD code,
                            const char *msg)
{
   freeLastError();

   lastErrorCode = code;
   if (msg) {
      lastErrorMessage = strdup (msg);
   }
}
//---------------------------------------------------------------------------

bool TLitePDF::checkAPIVersion(unsigned int major,
                               unsigned int minor)
{
   if (!lib) {
      return false;
   }

   char fileName[2048 + 1];
   DWORD fileNameLen = GetModuleFileNameA(lib, fileName, 2048);
   if (!fileNameLen) {
      return false;
   }
   fileName[fileNameLen] = 0;

   bool apiIsOK = false;
   DWORD dwVerHnd;
   DWORD dwVerInfoSize;

   dwVerInfoSize = GetFileVersionInfoSizeA(fileName, &dwVerHnd);
  
   if (dwVerInfoSize) {
      HANDLE  hMem;
      LPVOID  lpvMem;

      hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
      lpvMem = GlobalLock(hMem);

      VS_FIXEDFILEINFO *VersionInfo = NULL;
      if (GetFileVersionInfoA(fileName, dwVerHnd, dwVerInfoSize, lpvMem)) {
         UINT cchVer;
         BOOL fRet = VerQueryValueA(lpvMem, "\\", (void **)&VersionInfo, &cchVer);

         if (fRet && cchVer) {
            apiIsOK = ((VersionInfo->dwFileVersionMS >> 16) & 0xFFFF) == major &&
                       (VersionInfo->dwFileVersionMS & 0xFFFF) == minor;
         }
 
         GlobalUnlock(hMem);
         GlobalFree(hMem);
      }
   }

   return apiIsOK;
}
//---------------------------------------------------------------------------

void TLitePDF::ensureLibraryLoaded(const char *_func)
{
   if (lib) {
      return;
   }

   ThrowIfFail(lib == NULL);
   ThrowIfFail(context == NULL);

   #ifdef UNICODE
   lib = LoadLibraryW(L"litePDF.dll");
   #else
   lib = LoadLibraryA("litePDF.dll");
   #endif // UNICODE
   ThrowMessageIfFail (lib != NULL, "Failed to open litePDF.dll");

   if (!checkAPIVersion(LitePDF_API_Major, LitePDF_API_Minor)) {
      FreeLibrary (lib);
      lib = NULL;

      std::string ttmsg = std::string(_func) + ": " + "This LitePDF class is not designed for API version of litePDF.dll";
      throw TLitePDFException(ERROR_INVALID_DLL, ttmsg.c_str());
   }

   {
      typedef void (__stdcall * litePDFErrorCB)(unsigned int code, const char *msg, void *user_data);

      InitFunc(void *, litePDF_CreateContext, (litePDFErrorCB on_error, void *on_error_user_data));

      context = func (litePDFError, this);
   }

   if (!context) {
      FreeLibrary (lib);
      lib = NULL;
      ThrowMessageIfFail (context != NULL, "Failed to create context");
   } else {
      typedef unsigned int (__stdcall * litePDFEvalFontFlagCB)(char *inout_faceName,
                                                               unsigned int faceNameBufferSize,
                                                               void *user_data);

      InitFunc(BOOL, litePDF_SetEvalFontFlagCallback, (void *pctx, litePDFEvalFontFlagCB callback, void *callback_user_data));

      ThrowLastErrorIfFail(func (context, litePDFEvalFontFlag, this));
   }
}
//---------------------------------------------------------------------------

void TLitePDF::unloadLibrary(void)
{
   if (lib && context) {
      try {
         InitFunc(void, litePDF_FreeContext, (void *context));
         func(context);
      } catch(...) {
      }
      FreeLibrary(lib);
   }

   context = NULL;
   lib = NULL;
}
//---------------------------------------------------------------------------

void TLitePDF::SetUnit(TLitePDFUnit unitValue)
{
   const char *_func = "TLitePDF::SetUnit";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_SetUnit, (void *pctx, unsigned int unitValue));

   ThrowLastErrorIfFail(func(context, (unsigned int) unitValue));
}
//---------------------------------------------------------------------------

TLitePDFUnit TLitePDF::GetUnit(void)
{
   const char *_func = "TLitePDF::GetUnit";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(unsigned int, litePDF_GetUnit, (void *pctx));

   TLitePDFUnit currentUnit = (TLitePDFUnit) func(context);

   ThrowLastErrorIfFail(currentUnit > LitePDFUnit_Unknown && currentUnit <= LitePDFUnit_1000th_inch);

   return currentUnit;
}
//---------------------------------------------------------------------------

double TLitePDF::MMToUnitEx(TLitePDFUnit useUnit,
                            double mmValue) const
{
   double ratio = 1.0;

   switch(useUnit) {
   case LitePDFUnit_mm:
      ratio = 1.0;
      break;
   case LitePDFUnit_10th_mm:
      ratio = 10.0;
      break;
   case LitePDFUnit_100th_mm:
      ratio = 100.0;
      break;
   case LitePDFUnit_1000th_mm:
      ratio = 1000.0;
      break;
   case LitePDFUnit_inch:
      ratio = 1.0 / 25.4;
      break;
   case LitePDFUnit_10th_inch:
      ratio = 10.0 / 25.4;
      break;
   case LitePDFUnit_100th_inch:
      ratio = 100.0 / 25.4;
      break;
   case LitePDFUnit_1000th_inch:
      ratio = 1000.0 / 25.4;
      break;
   case LitePDFUnit_Unknown:
      break;
   }

   return mmValue * ratio;
}
//---------------------------------------------------------------------------

double TLitePDF::UnitToMMEx(TLitePDFUnit useUnit,
                            double unitValue) const
{
   double ratio = 1.0;

   switch(useUnit) {
   case LitePDFUnit_mm:
      ratio = 1.0;
      break;
   case LitePDFUnit_10th_mm:
      ratio = 1.0 / 10.0;
      break;
   case LitePDFUnit_100th_mm:
      ratio = 1.0 / 100.0;
      break;
   case LitePDFUnit_1000th_mm:
      ratio = 1.0 / 1000.0;
      break;
   case LitePDFUnit_inch:
      ratio = 25.4;
      break;
   case LitePDFUnit_10th_inch:
      ratio = 25.4 / 10.0;
      break;
   case LitePDFUnit_100th_inch:
      ratio = 25.4 / 100.0;
      break;
   case LitePDFUnit_1000th_inch:
      ratio = 25.4 / 1000.0;
      break;
   case LitePDFUnit_Unknown:
      break;
   }

   return unitValue * ratio;
}
//---------------------------------------------------------------------------

double TLitePDF::InchToUnitEx(TLitePDFUnit useUnit,
                              double inchValue) const
{
   double ratio = 1.0;

   switch(useUnit) {
   case LitePDFUnit_mm:
      ratio = 25.4;
      break;
   case LitePDFUnit_10th_mm:
      ratio = 10.0 * 25.4;
      break;
   case LitePDFUnit_100th_mm:
      ratio = 100.0 * 25.4;
      break;
   case LitePDFUnit_1000th_mm:
      ratio = 1000.0 * 25.4;
      break;
   case LitePDFUnit_inch:
      ratio = 1.0;
      break;
   case LitePDFUnit_10th_inch:
      ratio = 10.0;
      break;
   case LitePDFUnit_100th_inch:
      ratio = 100.0;
      break;
   case LitePDFUnit_1000th_inch:
      ratio = 1000.0;
      break;
   case LitePDFUnit_Unknown:
      break;
   }

   return inchValue * ratio;
}
//---------------------------------------------------------------------------

double TLitePDF::UnitToInchEx(TLitePDFUnit useUnit,
                              double unitValue) const
{
   double ratio = 1.0;

   switch(useUnit) {
   case LitePDFUnit_mm:
      ratio = 1.0 / 25.4;
      break;
   case LitePDFUnit_10th_mm:
      ratio = 1.0 / (25.4 * 10.0);
      break;
   case LitePDFUnit_100th_mm:
      ratio = 1.0 / (25.4 * 100.0);
      break;
   case LitePDFUnit_1000th_mm:
      ratio = 1.0 / (25.4 * 1000.0);
      break;
   case LitePDFUnit_inch:
      ratio = 1.0;
      break;
   case LitePDFUnit_10th_inch:
      ratio = 1.0 / 10.0;
      break;
   case LitePDFUnit_100th_inch:
      ratio = 1.0 / 100.0;
      break;
   case LitePDFUnit_1000th_inch:
      ratio = 1.0 / 1000.0;
      break;
   case LitePDFUnit_Unknown:
      break;
   }

   return unitValue * ratio;
}
//---------------------------------------------------------------------------

double TLitePDF::MMToUnit(double mmValue)
{
   return MMToUnitEx(GetUnit(), mmValue);
}
//---------------------------------------------------------------------------

double TLitePDF::UnitToMM(double unitValue)
{
   return UnitToMMEx(GetUnit(), unitValue);
}
//---------------------------------------------------------------------------

double TLitePDF::InchToUnit(double inchValue)
{
   return InchToUnitEx(GetUnit(), inchValue);
}
//---------------------------------------------------------------------------

double TLitePDF::UnitToInch(double unitValue)
{
   return UnitToInchEx(GetUnit(), unitValue);
}
//---------------------------------------------------------------------------

void __stdcall TLitePDF::litePDFError(unsigned int code,
                                      const char *msg,
                                      void *user_data)
{
   const char *_func = "TLitePDF::litePDFError";
   TLitePDF *lpdf;

   ThrowIfFail(user_data != NULL);

   lpdf = (TLitePDF *) user_data;
   lpdf->setLastError(code, msg);
   if (lpdf->onError) {
      lpdf->onError(code, msg, lpdf->onErrorUserData);
   }
}
//---------------------------------------------------------------------------

unsigned int __stdcall TLitePDF::litePDFEvalFontFlag(char *inout_faceName,
                                                     unsigned int faceNameBufferSize,
                                                     void *user_data)
{
   const char *_func = "TLitePDF::litePDFEvalFontFlag";
   TLitePDF *lpdf;

   ThrowIfFail(user_data != NULL);

   lpdf = (TLitePDF *) user_data;
   if (lpdf->onEvalFontFlag) {
      return lpdf->onEvalFontFlag(inout_faceName, faceNameBufferSize, lpdf->onEvalFontFlagUserData);
   }

   return LitePDFFontFlag_Default;
}
//---------------------------------------------------------------------------

void TLitePDF::SetEvalFontFlagCallback(TLitePDFEvalFontFlagCB callback,
                                       void *userData)
{
   onEvalFontFlag = callback;
   onEvalFontFlagUserData = userData;
}
//---------------------------------------------------------------------------

void TLitePDF::PrepareEncryption(const char *userPassword,
                                 const char *ownerPassword,
                                 unsigned int permissions,
                                 unsigned int algorithm)
{
   const char *_func = "TLitePDF::PrepareEncryption";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_PrepareEncryption, (void *pctx, const char *userPassword, const char *ownerPassword, unsigned int permissions, unsigned int algorithm));

   ThrowLastErrorIfFail(func(context, userPassword, ownerPassword, permissions, algorithm));
}
//---------------------------------------------------------------------------

void TLitePDF::CreateFileDocument(const char *fileName)
{
   const char *_func = "TLitePDF::CreateFileDocument";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_CreateFileDocument, (void *pctx, const char *fileName));

   ThrowLastErrorIfFail(func(context, fileName));
}
//---------------------------------------------------------------------------

void TLitePDF::CreateFileDocumentW(const wchar_t *fileName)
{
   const char *_func = "TLitePDF::CreateFileDocumentW";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_CreateFileDocumentW, (void *pctx, const wchar_t *fileName));

   ThrowLastErrorIfFail(func(context, fileName));
}
//---------------------------------------------------------------------------

void TLitePDF::CreateMemDocument(void)
{
   const char *_func = "TLitePDF::CreateMemDocument";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_CreateMemDocument, (void *pctx));

   ThrowLastErrorIfFail(func(context));
}
//---------------------------------------------------------------------------

void TLitePDF::LoadFromFile(const char *fileName,
                            const char *password,
                            bool loadCompletely,
                            bool forUpdate)
{
   const char *_func = "TLitePDF::LoadFromFile";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(fileName != NULL);

   InitFunc(BOOL, litePDF_LoadFromFile, (void *pctx, const char *fileName, const char *password, BOOL loadCompletely, BOOL forUpdate));

   ThrowLastErrorIfFail(func(context, fileName, password, loadCompletely ? TRUE : FALSE, forUpdate ? TRUE : FALSE));
}
//---------------------------------------------------------------------------

void TLitePDF::LoadFromFileW(const wchar_t *fileName,
                             const char *password,
                             bool loadCompletely,
                             bool forUpdate)
{
   const char *_func = "TLitePDF::LoadFromFileW";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(fileName != NULL);

   InitFunc(BOOL, litePDF_LoadFromFileW, (void *pctx, const wchar_t *fileName, const char *password, BOOL loadCompletely, BOOL forUpdate));

   ThrowLastErrorIfFail(func(context, fileName, password, loadCompletely ? TRUE : FALSE, forUpdate ? TRUE : FALSE));
}
//---------------------------------------------------------------------------

void TLitePDF::LoadFromData(const BYTE *data,
                            unsigned int dataLength,
                            const char *password,
                            bool forUpdate)
{
   const char *_func = "TLitePDF::LoadFromData";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(data != NULL);

   InitFunc(BOOL, litePDF_LoadFromData, (void *pctx, const BYTE *data, unsigned int dataLength, const char *password, BOOL forUpdate));

   ThrowLastErrorIfFail(func(context, data, dataLength, password, forUpdate ? TRUE : FALSE));
}
//---------------------------------------------------------------------------

void TLitePDF::SaveToFile(const char *fileName)
{
   const char *_func = "TLitePDF::SaveToFile";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(fileName != NULL);

   InitFunc(BOOL, litePDF_SaveToFile, (void *pctx, const char *fileName));

   ThrowLastErrorIfFail(func(context, fileName));
}
//---------------------------------------------------------------------------

void TLitePDF::SaveToFileW(const wchar_t *fileName)
{
   const char *_func = "TLitePDF::SaveToFileW";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(fileName != NULL);

   InitFunc(BOOL, litePDF_SaveToFileW, (void *pctx, const wchar_t *fileName));

   ThrowLastErrorIfFail(func(context, fileName));
}
//---------------------------------------------------------------------------

bool TLitePDF::SaveToData(BYTE *data,
                          unsigned int *dataLength)
{
   const char *_func = "TLitePDF::SaveToData";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(dataLength != NULL);

   InitFunc(BOOL, litePDF_SaveToData, (void *pctx, BYTE *data, unsigned int *dataLength));

   BOOL succeeded = func(context, data, dataLength);

   return succeeded ? true : false;
}
//---------------------------------------------------------------------------

void TLitePDF::Close(void)
{
   const char *_func = "TLitePDF::Close";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(void, litePDF_Close, (void *pctx));

   func(context);
}
//---------------------------------------------------------------------------

unsigned int TLitePDF::GetPageCount(void)
{
   const char *_func = "TLitePDF::GetPageCount";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_GetPageCount, (void *pctx, unsigned int *pageCount));

   unsigned int pageCount = 0;
   ThrowLastErrorIfFail(func(context, &pageCount));

   return pageCount;
}
//---------------------------------------------------------------------------

void TLitePDF::GetPageSize(unsigned int pageIndex,
                           unsigned int *width_u,
                           unsigned int *height_u)
{
   const char *_func = "TLitePDF::GetPageSize";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(width_u != NULL);
   ThrowIfFail(height_u != NULL);

   InitFunc(BOOL, litePDF_GetPageSize, (void *pctx, unsigned int pageIndex, unsigned int *width_u, unsigned int *height_u));

   ThrowLastErrorIfFail(func(context, pageIndex, width_u, height_u));
}
//---------------------------------------------------------------------------

int TLitePDF::GetPageRotation(unsigned int pageIndex)
{
   const char *_func = "TLitePDF::GetPageRotation";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   int degrees = 0;

   InitFunc(BOOL, litePDF_GetPageRotation, (void *pctx, unsigned int pageIndex, int *out_degrees));

   ThrowLastErrorIfFail(func(context, pageIndex, &degrees));

   return degrees;
}
//---------------------------------------------------------------------------

void TLitePDF::SetPageRotation(unsigned int pageIndex,
                               int degrees)
{
   const char *_func = "TLitePDF::SetPageRotation";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_SetPageRotation, (void *pctx, unsigned int pageIndex, int degrees));

   ThrowLastErrorIfFail(func(context, pageIndex, degrees));
}
//---------------------------------------------------------------------------


HDC TLitePDF::AddPage(unsigned int width_u,
                      unsigned int height_u,
                      unsigned int width_px,
                      unsigned int height_px,
                      unsigned int drawFlags)
{
   const char *_func = "TLitePDF::AddPage";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(HDC, litePDF_AddPage, (void *pctx, unsigned int width_u, unsigned int height_u, unsigned int width_px, unsigned int height_px, unsigned int drawFlags));

   HDC res = func (context, width_u, height_u, width_px, height_px, drawFlags);
   ThrowLastErrorIfFail(res != NULL);

   return res;
}
//---------------------------------------------------------------------------

HDC TLitePDF::InsertPage(unsigned int pageIndex,
                         unsigned int width_u,
                         unsigned int height_u,
                         unsigned int width_px,
                         unsigned int height_px,
                         unsigned int drawFlags)
{
   const char *_func = "TLitePDF::InsertPage";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(HDC, litePDF_InsertPage, (void *pctx, unsigned int pageIndex, unsigned int width_u, unsigned int height_u, unsigned int width_px, unsigned int height_px, unsigned int drawFlags));

   HDC res = func(context, pageIndex, width_u, height_u, width_px, height_px, drawFlags);
   ThrowLastErrorIfFail(res != NULL);

   return res;
}
//---------------------------------------------------------------------------

HDC TLitePDF::UpdatePage(unsigned int pageIndex,
                         unsigned int width_px,
                         unsigned int height_px,
                         unsigned int drawFlags)
{
   const char *_func = "TLitePDF::UpdatePage";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(HDC, litePDF_UpdatePage, (void *pctx, unsigned int pageIndex, unsigned int width_px, unsigned int height_px, unsigned int drawFlags));

   HDC res = func(context, pageIndex, width_px, height_px, drawFlags);
   ThrowLastErrorIfFail(res != NULL);

   return res;
}
//---------------------------------------------------------------------------

void TLitePDF::FinishPage(HDC hDC)
{
   const char *_func = "TLitePDF::FinishPage";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(hDC != NULL);

   InitFunc(BOOL, litePDF_FinishPage, (void *pctx, HDC hDC));

   ThrowLastErrorIfFail(func(context, hDC));
}
//---------------------------------------------------------------------------

HDC TLitePDF::AddResource(unsigned int width_u,
                          unsigned int height_u,
                          unsigned int width_px,
                          unsigned int height_px,
                          unsigned int drawFlags)
{
   const char *_func = "TLitePDF::AddResource";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(HDC, litePDF_AddResource, (void *pctx, unsigned int width_u, unsigned int height_u, unsigned int width_px, unsigned int height_px, unsigned int drawFlags));

   HDC res = func(context, width_u, height_u, width_px, height_px, drawFlags);
   ThrowLastErrorIfFail(res != NULL);

   return res;
}
//---------------------------------------------------------------------------

unsigned int TLitePDF::FinishResource(HDC hDC)
{
   const char *_func = "TLitePDF::FinishResource";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(hDC != NULL);

   InitFunc(unsigned int, litePDF_FinishResource, (void *pctx, HDC hDC));

   unsigned int resourceID = func(context, hDC);
   ThrowLastErrorIfFail(resourceID != 0);

   return resourceID;
}
//---------------------------------------------------------------------------

void TLitePDF::DeletePage(unsigned int pageIndex)
{
   const char *_func = "TLitePDF::DeletePage";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_DeletePage, (void *pctx, unsigned int pageIndex));

   ThrowLastErrorIfFail(func(context, pageIndex));
}
//---------------------------------------------------------------------------

void TLitePDF::AddPagesFrom(litePDF::TLitePDF *from,
                            unsigned int pageIndex,
                            unsigned int pageCount)
{
   const char *_func = "TLitePDF::AddPagesFrom";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(from != NULL);
   ThrowIfFail(from != this);
   ThrowIfFail(from->context != NULL);

   InitFunc(BOOL, litePDF_AddPagesFrom, (void *pctx, void *pctx_from, unsigned int pageIndex, unsigned int pageCount));

   ThrowLastErrorIfFail(func(context, from->context, pageIndex, pageCount));
}
//---------------------------------------------------------------------------

void TLitePDF::InsertPageFrom(unsigned int pageIndexTo,
                              litePDF::TLitePDF *from,
                              unsigned int pageIndexFrom)
{
   const char *_func = "TLitePDF::InsertPageFrom";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(from != NULL);
   ThrowIfFail(from != this);
   ThrowIfFail(from->context != NULL);

   InitFunc(BOOL, litePDF_InsertPageFrom, (void *pctx, unsigned int pageIndexTo, void *pctx_from, unsigned int pageIndexFrom));

   ThrowLastErrorIfFail(func(context, pageIndexTo, from->context, pageIndexFrom));
}
//---------------------------------------------------------------------------

unsigned int TLitePDF::AddPageFromAsResource(litePDF::TLitePDF *from,
                                             unsigned int pageIndex,
                                             bool useTrimBox)
{
   const char *_func = "TLitePDF::AddPageFromAsResource";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(from != NULL);
   ThrowIfFail(from != this);
   ThrowIfFail(from->context != NULL);

   InitFunc(unsigned int, litePDF_AddPageFromAsResource, (void *pctx, void *pctx_from, unsigned int pageIndex, BOOL useTrimBox));

   unsigned int resourceID = func(context, from->context, pageIndex, useTrimBox ? TRUE : FALSE);
   ThrowLastErrorIfFail(resourceID != 0);

   return resourceID;
}
//---------------------------------------------------------------------------

unsigned int TLitePDF::PageToResource(unsigned int pageIndex)
{
   const char *_func = "TLitePDF::PageToResource";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(unsigned int, litePDF_PageToResource, (void *pctx, unsigned int pageIndex));

   unsigned int resourceID = func(context, pageIndex);
   ThrowLastErrorIfFail(resourceID != 0);

   return resourceID;
}
//---------------------------------------------------------------------------

void TLitePDF::GetResourceSize(unsigned int resourceID,
                               unsigned int *width_u,
                               unsigned int *height_u)
{
   const char *_func = "TLitePDF::GetResourceSize";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(width_u != NULL);
   ThrowIfFail(height_u != NULL);

   InitFunc(BOOL, litePDF_GetResourceSize, (void *pctx, unsigned int resourceID, unsigned int *width_u, unsigned int *height_u));

   ThrowLastErrorIfFail(func(context, resourceID, width_u, height_u));
}
//---------------------------------------------------------------------------

void TLitePDF::DrawResource(unsigned int resourceID,
                            unsigned int pageIndex,
                            TLitePDFUnit unitValue,
                            int x,
                            int y,
                            int scaleX,
                            int scaleY)
{
   const char *_func = "TLitePDF::DrawResource";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_DrawResource, (void *pctx, unsigned int resourceID, unsigned int unitValue, unsigned int pageIndex, int x, int y, int scaleX, int scaleY));

   ThrowLastErrorIfFail(func(context, resourceID, pageIndex, (unsigned int) unitValue, x, y, scaleX, scaleY));
}
//---------------------------------------------------------------------------

void TLitePDF::DrawResourceWithMatrix(unsigned int resourceID,
                                      unsigned int pageIndex,
                                      double a,
                                      double b,
                                      double c,
                                      double d,
                                      double e,
                                      double f)
{
   const char *_func = "TLitePDF::DrawResourceWithMatrix";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_DrawResourceWithMatrix, (void *pctx, unsigned int resourceID, unsigned int pageIndex, int a, int b, int c, int d, int e, int f));

   int i_a = a * 1000.0, i_b = b * 1000.0, i_c = c * 1000.0, i_d = d * 1000.0, i_e = e * 1000.0, i_f = f * 1000.0;

   ThrowLastErrorIfFail(func(context, resourceID, pageIndex, i_a, i_b, i_c, i_d, i_e, i_f));
}
//---------------------------------------------------------------------------

void TLitePDF::SetDocumentInfo(const char *name,
                               const wchar_t *value)
{
   const char *_func = "TLitePDF::SetDocumentInfo";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(name != NULL);
   ThrowIfFail(value != NULL);

   InitFunc(BOOL, litePDF_SetDocumentInfo, (void *pctx, const char *name, const wchar_t *value));

   ThrowLastErrorIfFail(func(context, name, value));
}
//---------------------------------------------------------------------------

bool TLitePDF::GetDocumentInfoExists(const char *name)
{
   const char *_func = "TLitePDF::GetDocumentInfoExists";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(name != NULL);

   InitFunc(BOOL, litePDF_GetDocumentInfoExists, (void *pctx, const char *name, BOOL *pExists));

   BOOL exists = FALSE;

   ThrowLastErrorIfFail(func(context, name, &exists));

   return exists ? true : false;
}
//---------------------------------------------------------------------------

std::wstring TLitePDF::GetDocumentInfo(const char *name)
{
   const char *_func = "TLitePDF::GetDocumentInfo";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(name != NULL);

   InitFunc(BOOL, litePDF_GetDocumentInfo, (void *pctx, const char *name, wchar_t *value, unsigned int *valueLength));

   unsigned int valueLength = 0;
   ThrowLastErrorIfFail(func(context, name, NULL, &valueLength));

   wchar_t *buff = (wchar_t *) malloc(sizeof(wchar_t) * (valueLength + 1));
   ThrowMessageIfFail(buff != NULL, "Out of memory!");

   std::wstring value;

   if (func(context, name, buff, &valueLength)) {
      buff[valueLength] = 0;
      value = buff;
      free (buff);
   } else {
      free (buff);

      // always false
      ThrowLastErrorIfFail(buff == NULL);
   }

   return value;
}
//---------------------------------------------------------------------------

bool TLitePDF::GetDocumentIsSigned(void)
{
   const char *_func = "TLitePDF::GetDocumentIsSigned";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_GetDocumentIsSigned, (void *pctx, BOOL *pIsSigned));

   BOOL isSigned = FALSE;

   ThrowLastErrorIfFail(func(context, &isSigned));

   return isSigned ? true : false;
}
//---------------------------------------------------------------------------

unsigned int TLitePDF::GetSignatureCount(void)
{
   const char *_func = "TLitePDF::GetSignatureCount";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_GetSignatureCount, (void *pctx, unsigned int *pCount));

   unsigned int count = 0;

   ThrowLastErrorIfFail(func(context, &count));

   return count;
}
//---------------------------------------------------------------------------

std::string TLitePDF::GetSignatureName(unsigned int index)
{
   const char *_func = "TLitePDF::GetSignatureName";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_GetSignatureName, (void *pctx, unsigned int index, char *name, unsigned int *nameLength));

   unsigned int nameLength = 0;
   ThrowLastErrorIfFail(func(context, index, NULL, &nameLength));

   nameLength++;

   char *buff = (char *) malloc(sizeof(char) * (nameLength));
   ThrowMessageIfFail(buff != NULL, "Out of memory!");

   std::string name;

   if (func(context, index, buff, &nameLength)) {
      buff[nameLength] = 0;
      name = buff;
      free (buff);
   } else {
      free (buff);

      // always false
      ThrowLastErrorIfFail(buff == NULL);
   }

   return name;
}
//---------------------------------------------------------------------------

unsigned int TLitePDF::CreateSignature(const char *name,
                                       unsigned int annotationPageIndex,
                                       int annotationX_u,
                                       int annotationY_u,
                                       int annotationWidth_u,
                                       int annotationHeight_u,
                                       unsigned int annotationFlags)
{
   const char *_func = "TLitePDF::CreateSignature";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(name != NULL);

   InitFunc(BOOL, litePDF_CreateSignature, (void *pctx,
                                            const char *name,
                                            unsigned int annotationPageIndex,
                                            int annotationX_u,
                                            int annotationY_u,
                                            int annotationWidth_u,
                                            int annotationHeight_u,
                                            unsigned int annotationFlags,
                                            unsigned int *pAddedIndex));

   unsigned int addedIndex = -1;

   ThrowLastErrorIfFail(func(context,
      name,
      annotationPageIndex,
      annotationX_u,
      annotationY_u,
      annotationWidth_u,
      annotationHeight_u,
      annotationFlags,
      &addedIndex));

   return addedIndex;
}
//---------------------------------------------------------------------------

bool TLitePDF::GetSignatureHasData(unsigned int index)
{
   const char *_func = "TLitePDF::GetSignatureHasData";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_GetSignatureHasData, (void *pctx, unsigned int index, BOOL *pHasData));

   BOOL hasData = FALSE;

   ThrowLastErrorIfFail(func(context, index, &hasData));

   return hasData ? true : false;
}
//---------------------------------------------------------------------------

bool TLitePDF::GetSignatureData(unsigned int index,
                                BYTE *data,
                                unsigned int *dataLength)
{
   const char *_func = "TLitePDF::GetSignatureData";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(dataLength != NULL);

   InitFunc(BOOL, litePDF_GetSignatureData, (void *pctx, unsigned int index, BYTE *data, unsigned int *dataLength));

   BOOL succeeded = func(context, index, data, dataLength);

   return succeeded ? true : false;
}
//---------------------------------------------------------------------------

bool TLitePDF::GetSignatureRanges(unsigned int index,
                                  unsigned __int64 *pRangesArray,
                                  unsigned int *pRangesArrayLength)
{
   const char *_func = "TLitePDF::GetSignatureRanges";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(pRangesArrayLength != NULL);

   InitFunc(BOOL, litePDF_GetSignatureRanges, (void *pctx, unsigned int index, unsigned __int64 *pRangesArray, unsigned int *pRangesArrayLength));

   BOOL succeeded = func(context, index, pRangesArray, pRangesArrayLength);

   return succeeded ? true : false;
}
//---------------------------------------------------------------------------

void TLitePDF::SetSignatureDate(unsigned int index,
                                __int64 dateOfSign)
{
   const char *_func = "TLitePDF::SetSignatureDate";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_SetSignatureDate, (void *pctx, unsigned int index, __int64 dateOfSign));

   ThrowLastErrorIfFail(func(context, index, dateOfSign));
}
//---------------------------------------------------------------------------

__int64 TLitePDF::GetSignatureDate(unsigned int index)
{
   const char *_func = "TLitePDF::GetSignatureDate";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_GetSignatureDate, (void *pctx, unsigned int index, __int64 *pDateOfSign));

   __int64 dateOfSign = 0;

   ThrowLastErrorIfFail(func(context, index, &dateOfSign));

   return dateOfSign;
}
//---------------------------------------------------------------------------

void TLitePDF::SetSignatureReason(unsigned int index,
                                  const wchar_t *reason)
{
   const char *_func = "TLitePDF::SetSignatureReason";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_SetSignatureReason, (void *pctx, unsigned int index, const wchar_t *reason));

   ThrowLastErrorIfFail(func(context, index, reason));
}
//---------------------------------------------------------------------------

std::wstring TLitePDF::GetSignatureReason(unsigned int index)
{
   const char *_func = "TLitePDF::GetSignatureReason";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_GetSignatureReason, (void *pctx, unsigned int index, wchar_t *value, unsigned int *valueLength));

   unsigned int valueLength = 0;
   ThrowLastErrorIfFail(func(context, index, NULL, &valueLength));

   valueLength++;

   wchar_t *buff = (wchar_t *) malloc(sizeof(wchar_t) * (valueLength));
   ThrowMessageIfFail(buff != NULL, "Out of memory!");

   std::wstring value;

   if (func(context, index, buff, &valueLength)) {
      buff[valueLength] = 0;
      value = buff;
      free (buff);
   } else {
      free (buff);

      // always false
      ThrowLastErrorIfFail(buff == NULL);
   }

   return value;
}
//---------------------------------------------------------------------------

void TLitePDF::SetSignatureLocation(unsigned int index,
                                    const wchar_t *location)
{
   const char *_func = "TLitePDF::SetSignatureLocation";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_SetSignatureLocation, (void *pctx, unsigned int index, const wchar_t *location));

   ThrowLastErrorIfFail(func(context, index, location));
}
//---------------------------------------------------------------------------

std::wstring TLitePDF::GetSignatureLocation(unsigned int index)
{
   const char *_func = "TLitePDF::GetSignatureLocation";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_GetSignatureLocation, (void *pctx, unsigned int index, wchar_t *value, unsigned int *valueLength));

   unsigned int valueLength = 0;
   ThrowLastErrorIfFail(func(context, index, NULL, &valueLength));

   valueLength++;

   wchar_t *buff = (wchar_t *) malloc(sizeof(wchar_t) * (valueLength));
   ThrowMessageIfFail(buff != NULL, "Out of memory!");

   std::wstring value;

   if (func(context, index, buff, &valueLength)) {
      buff[valueLength] = 0;
      value = buff;
      free (buff);
   } else {
      free (buff);

      // always false
      ThrowLastErrorIfFail(buff == NULL);
   }

   return value;
}
//---------------------------------------------------------------------------

void TLitePDF::SetSignatureCreator(unsigned int index,
                                   const char *creator)
{
   const char *_func = "TLitePDF::SetSignatureCreator";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_SetSignatureCreator, (void *pctx, unsigned int index, const char *creator));

   ThrowLastErrorIfFail(func(context, index, creator));
}
//---------------------------------------------------------------------------

std::string TLitePDF::GetSignatureCreator(unsigned int index)
{
   const char *_func = "TLitePDF::GetSignatureCreator";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_GetSignatureCreator, (void *pctx, unsigned int index, char *value, unsigned int *valueLength));

   unsigned int valueLength = 0;
   ThrowLastErrorIfFail(func(context, index, NULL, &valueLength));

   valueLength++;

   char *buff = (char *) malloc(sizeof(char) * (valueLength));
   ThrowMessageIfFail(buff != NULL, "Out of memory!");

   std::string value;

   if (func(context, index, buff, &valueLength)) {
      buff[valueLength] = 0;
      value = buff;
      free (buff);
   } else {
      free (buff);

      // always false
      ThrowLastErrorIfFail(buff == NULL);
   }

   return value;
}
//---------------------------------------------------------------------------

void TLitePDF::SetSignatureAppearance(unsigned int index,
                                      TLitePDFAppearance appearanceType,
                                      unsigned int resourceID,
                                      int offsetX_u,
                                      int offsetY_u)
{
   const char *_func = "TLitePDF::SetSignatureAppearance";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_SetSignatureAppearance, (void *pctx,
                                                   unsigned int index,
                                                   unsigned int appearanceType,
                                                   unsigned int resourceID,
                                                   int offsetX_u,
                                                   int offsetY_u));

   unsigned int apType;

   if (appearanceType == LitePDFAppearance_Rollover) {
      apType = 1;
   } else if (appearanceType == LitePDFAppearance_Down) {
      apType = 2;
   } else { // LitePDFAppearance_Normal
      apType = 0;
   }

   ThrowLastErrorIfFail(func(context, index, apType, resourceID, offsetX_u, offsetY_u));
}
//---------------------------------------------------------------------------

void TLitePDF::SetSignatureSize(unsigned int requestBytes)
{
   const char *_func = "TLitePDF::SetSignatureSize";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_SetSignatureSize, (void *pctx, unsigned int requestBytes));

   ThrowLastErrorIfFail(func(context, requestBytes));
}
//---------------------------------------------------------------------------

void TLitePDF::AddSignerPFX(const BYTE *pfxData,
                            unsigned int pfxDataLength,
                            const char *pfxPassword)
{
   const char *_func = "TLitePDF::AddSignerPFX";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_AddSignerPFX, (void *pctx,
                                         const BYTE *pfxData,
                                         unsigned int pfxDataLength,
                                         const char *pfxPassword));

   ThrowLastErrorIfFail(func(context, pfxData, pfxDataLength, pfxPassword));
}
//---------------------------------------------------------------------------

void TLitePDF::AddSignerPEM(const BYTE *pemData,
                            unsigned int pemDataLength,
                            const BYTE *pkeyData,
                            unsigned int pkeyDataLength,
                            const char *pkeyPassword)
{
   const char *_func = "TLitePDF::AddSignerPEM";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_AddSignerPEM, (void *pctx,
                                         const BYTE *pemData,
                                         unsigned int pemDataLength,
                                         const BYTE *pkeyData,
                                         unsigned int pkeyDataLength,
                                         const char *pkeyPassword));

   ThrowLastErrorIfFail(func(context, pemData, pemDataLength, pkeyData, pkeyDataLength, pkeyPassword));
}
//---------------------------------------------------------------------------

void TLitePDF::SaveToFileWithSign(const char *fileName,
                                  unsigned int signatureIndex)
{
   const char *_func = "TLitePDF::SaveToFileWithSign";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(fileName != NULL);

   InitFunc(BOOL, litePDF_SaveToFileWithSign, (void *pctx,
                                               const char *fileName,
                                               unsigned int signatureIndex));

   ThrowLastErrorIfFail(func(context, fileName, signatureIndex));
}
//---------------------------------------------------------------------------

void TLitePDF::SaveToFileWithSignW(const wchar_t *fileName,
                                   unsigned int signatureIndex)
{
   const char *_func = "TLitePDF::SaveToFileWithSignW";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(fileName != NULL);

   InitFunc(BOOL, litePDF_SaveToFileWithSignW, (void *pctx,
                                                const wchar_t *fileName,
                                                unsigned int signatureIndex));

   ThrowLastErrorIfFail(func(context, fileName, signatureIndex));
}
//---------------------------------------------------------------------------

bool TLitePDF::SaveToDataWithSign(unsigned int signatureIndex,
                                  BYTE *data,
                                  unsigned int *dataLength)
{
   const char *_func = "TLitePDF::SaveToDataWithSign";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(dataLength != NULL);

   InitFunc(BOOL, litePDF_SaveToDataWithSign, (void *pctx,
                                               unsigned int signatureIndex,
                                               BYTE *data,
                                               unsigned int *dataLength));

   BOOL succeeded = func(context, signatureIndex, data, dataLength);

   return succeeded ? true : false;
}
//---------------------------------------------------------------------------

void TLitePDF::SaveToFileWithSignManual(const char *fileName,
                                        unsigned int signatureIndex,
                                        TLitePDFAppendSignatureDataFunc appendSignatureData,
                                        void *append_user_data,
                                        TLitePDFFinishSignatureFunc finishSignature,
                                        void *finish_user_data)
{
   const char *_func = "TLitePDF::SaveToFileWithSignManual";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(fileName != NULL);
   ThrowIfFail(appendSignatureData != NULL);
   ThrowIfFail(finishSignature != NULL);

   InitFunc(BOOL, litePDF_SaveToFileWithSignManual, (void *pctx,
                                                     const char *fileName,
                                                     unsigned int signatureIndex,
                                                     TLitePDFAppendSignatureDataFunc appendSignatureData,
                                                     void *append_user_data,
                                                     TLitePDFFinishSignatureFunc finishSignature,
                                                     void *finish_user_data));

   ThrowLastErrorIfFail(func(context,
      fileName,
      signatureIndex,
      appendSignatureData,
      append_user_data,
      finishSignature,
      finish_user_data));
}
//---------------------------------------------------------------------------

void TLitePDF::SaveToFileWithSignManualW(const wchar_t *fileName,
                                         unsigned int signatureIndex,
                                         TLitePDFAppendSignatureDataFunc appendSignatureData,
                                         void *append_user_data,
                                         TLitePDFFinishSignatureFunc finishSignature,
                                         void *finish_user_data)
{
   const char *_func = "TLitePDF::SaveToFileWithSignManualW";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(fileName != NULL);
   ThrowIfFail(appendSignatureData != NULL);
   ThrowIfFail(finishSignature != NULL);

   InitFunc(BOOL, litePDF_SaveToFileWithSignManualW, (void *pctx,
                                                      const wchar_t *fileName,
                                                      unsigned int signatureIndex,
                                                      TLitePDFAppendSignatureDataFunc appendSignatureData,
                                                      void *append_user_data,
                                                      TLitePDFFinishSignatureFunc finishSignature,
                                                      void *finish_user_data));

   ThrowLastErrorIfFail(func(context,
      fileName,
      signatureIndex,
      appendSignatureData,
      append_user_data,
      finishSignature,
      finish_user_data));
}
//---------------------------------------------------------------------------

bool TLitePDF::SaveToDataWithSignManual(unsigned int signatureIndex,
                                        TLitePDFAppendSignatureDataFunc appendSignatureData,
                                        void *append_user_data,
                                        TLitePDFFinishSignatureFunc finishSignature,
                                        void *finish_user_data,
                                        BYTE *data,
                                        unsigned int *dataLength)
{
   const char *_func = "TLitePDF::SaveToDataWithSignManual";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(appendSignatureData != NULL);
   ThrowIfFail(finishSignature != NULL);
   ThrowIfFail(dataLength != NULL);

   InitFunc(BOOL, litePDF_SaveToDataWithSignManual, (void *pctx,
                                                     unsigned int signatureIndex,
                                                     TLitePDFAppendSignatureDataFunc appendSignatureData,
                                                     void *append_user_data,
                                                     TLitePDFFinishSignatureFunc finishSignature,
                                                     void *finish_user_data,
                                                     BYTE *data,
                                                     unsigned int *dataLength));

   BOOL succeeded = func(context,
      signatureIndex,
      appendSignatureData,
      append_user_data,
      finishSignature,
      finish_user_data,
      data,
      dataLength);

   return succeeded ? true : false;
}
//---------------------------------------------------------------------------

void TLitePDF::EmbedFile(const char *fileName)
{
   const char *_func = "TLitePDF::EmbedFile";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(fileName != NULL);

   InitFunc(BOOL, litePDF_EmbedFile, (void *pctx, const char *fileName));

   ThrowLastErrorIfFail(func(context, fileName));
}
//---------------------------------------------------------------------------

void TLitePDF::EmbedFileW(const wchar_t *fileName)
{
   const char *_func = "TLitePDF::EmbedFileW";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(fileName != NULL);

   InitFunc(BOOL, litePDF_EmbedFileW, (void *pctx, const wchar_t *fileName));

   ThrowLastErrorIfFail(func(context, fileName));
}
//---------------------------------------------------------------------------

void TLitePDF::EmbedData(const char *fileName,
                         const BYTE *data,
                         unsigned int dataLength)
{
   const char *_func = "TLitePDF::EmbedData";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(data != NULL);

   InitFunc(BOOL, litePDF_EmbedData, (void *pctx, const char *fileName, const BYTE *data, unsigned int dataLength));

   ThrowLastErrorIfFail(func(context, fileName, data, dataLength));
}
//---------------------------------------------------------------------------

void TLitePDF::EmbedDataW(const wchar_t *fileName,
                          const BYTE *data,
                          unsigned int dataLength)
{
   const char *_func = "TLitePDF::EmbedDataW";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(data != NULL);

   InitFunc(BOOL, litePDF_EmbedDataW, (void *pctx, const wchar_t *fileName, const BYTE *data, unsigned int dataLength));

   ThrowLastErrorIfFail(func(context, fileName, data, dataLength));
}
//---------------------------------------------------------------------------

int TLitePDF::GetEmbeddedFileCount(void)
{
   const char *_func = "TLitePDF::GetEmbeddedFileCount";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(INT, litePDF_GetEmbeddedFileCount, (void *pctx));

   return func(context);
}
//---------------------------------------------------------------------------

std::string TLitePDF::GetEmbeddedFileName(unsigned int index)
{
   const char *_func = "TLitePDF::GetEmbeddedFileName";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_GetEmbeddedFileName, (void *pctx, unsigned int index, char *fileName, unsigned int *fileNameLength));

   unsigned int fileNameLength = 0;
   ThrowLastErrorIfFail(func(context, index, NULL, &fileNameLength));

   fileNameLength++;

   char *buff = (char *) malloc(sizeof(char) * (fileNameLength));
   ThrowMessageIfFail(buff != NULL, "Out of memory!");

   std::string fileName;

   if (func(context, index, buff, &fileNameLength)) {
      buff[fileNameLength] = 0;
      fileName = buff;
      free (buff);
   } else {
      free (buff);

      // always false
      ThrowLastErrorIfFail(buff == NULL);
   }

   return fileName;
}
//---------------------------------------------------------------------------

std::wstring TLitePDF::GetEmbeddedFileNameW(unsigned int index)
{
   const char *_func = "TLitePDF::GetEmbeddedFileNameW";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_GetEmbeddedFileNameW, (void *pctx, unsigned int index, wchar_t *fileName, unsigned int *fileNameLength));

   unsigned int fileNameLength = 0;
   ThrowLastErrorIfFail(func(context, index, NULL, &fileNameLength));

   fileNameLength++;

   wchar_t *buff = (wchar_t *) malloc(sizeof(wchar_t) * (fileNameLength));
   ThrowMessageIfFail(buff != NULL, "Out of memory!");

   std::wstring fileName;

   if (func(context, index, buff, &fileNameLength)) {
      buff[fileNameLength] = 0;
      fileName = buff;
      free (buff);
   } else {
      free (buff);

      // always false
      ThrowLastErrorIfFail(buff == NULL);
   }

   return fileName;
}
//---------------------------------------------------------------------------

bool TLitePDF::GetEmbeddedFileData(unsigned int index,
                                   BYTE *data,
                                   unsigned int *dataLength)
{
   const char *_func = "TLitePDF::GetEmbeddedFileData";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(dataLength != NULL);

   InitFunc(BOOL, litePDF_GetEmbeddedFileData, (void *pctx, unsigned int index, BYTE *data, unsigned int *dataLength));

   BOOL succeeded = func(context, index, data, dataLength);

   return succeeded ? true : false;
}
//---------------------------------------------------------------------------

void *TLitePDF::GetPoDoFoDocument(void)
{
   const char *_func = "TLitePDF::GetPoDoFoDocument";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(void *, litePDF_GetPoDoFoDocument, (void *pctx));

   void *podofoDocument = func(context);

   ThrowLastErrorIfFail(podofoDocument != NULL);

   return podofoDocument;
}
//---------------------------------------------------------------------------

void TLitePDF::DrawDebugPage(const char *filename)
{
   const char *_func = "TLitePDF::DrawDebugPage";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(filename != NULL);

   InitFunc(BOOL, litePDF_DrawDebugPage, (void *pctx, const char *filename));

   ThrowLastErrorIfFail(func(context, filename));
}
//---------------------------------------------------------------------------

void TLitePDF::CreateLinkAnnotation(unsigned int annotationPageIndex,
                                    int annotationX_u,
                                    int annotationY_u,
                                    int annotationWidth_u,
                                    int annotationHeight_u,
                                    unsigned int annotationFlags,
                                    unsigned int annotationResourceID,
                                    unsigned int destinationPageIndex,
                                    unsigned int destinationX_u,
                                    unsigned int destinationY_u,
                                    const wchar_t *destinationDescription)
{
   const char *_func = "TLitePDF::CreateLinkAnnotation";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);

   InitFunc(BOOL, litePDF_CreateLinkAnnotation, (void *pctx,
                                                 unsigned int annotationPageIndex,
                                                 int annotationX_u,
                                                 int annotationY_u,
                                                 int annotationWidth_u,
                                                 int annotationHeight_u,
                                                 unsigned int annotationFlags,
                                                 unsigned int annotationResourceID,
                                                 unsigned int destinationPageIndex,
                                                 unsigned int destinationX_u,
                                                 unsigned int destinationY_u,
                                                 const wchar_t *destinationDescription));

   ThrowLastErrorIfFail(func(context,
                             annotationPageIndex,
                             annotationX_u,
                             annotationY_u,
                             annotationWidth_u,
                             annotationHeight_u,
                             annotationFlags,
                             annotationResourceID,
                             destinationPageIndex,
                             destinationX_u,
                             destinationY_u,
                             destinationDescription));
}
//---------------------------------------------------------------------------

unsigned int TLitePDF::CreateBookmarkRoot(const wchar_t *title,
                                          unsigned int flags,
                                          COLORREF titleColor,
                                          unsigned int destinationPageIndex,
                                          unsigned int destinationX_u,
                                          unsigned int destinationY_u)
{
   const char *_func = "TLitePDF::CreateBookmarkRoot";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(title != NULL);

   InitFunc(unsigned int, litePDF_CreateBookmarkRoot, (void *pctx,
                                                       const wchar_t *title,
                                                       unsigned int flags,
                                                       unsigned char titleColor_red,
                                                       unsigned char titleColor_green,
                                                       unsigned char titleColor_blue,
                                                       unsigned int destinationPageIndex,
                                                       unsigned int destinationX_u,
                                                       unsigned int destinationY_u));

   unsigned int resourceID = func(context,
                                  title,
                                  flags,
                                  GetRValue(titleColor),
                                  GetGValue(titleColor),
                                  GetBValue(titleColor),
                                  destinationPageIndex,
                                  destinationX_u,
                                  destinationY_u);
   ThrowLastErrorIfFail(resourceID != 0);

   return resourceID;
}
//---------------------------------------------------------------------------

unsigned int TLitePDF::CreateBookmarkChild(unsigned int parentBookmarkID,
                                           const wchar_t *title,
                                           unsigned int flags,
                                           COLORREF titleColor,
                                           unsigned int destinationPageIndex,
                                           unsigned int destinationX_u,
                                           unsigned int destinationY_u)
{
   const char *_func = "TLitePDF::CreateBookmarkChild";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(title != NULL);

   InitFunc(unsigned int, litePDF_CreateBookmarkChild, (void *pctx,
                                                        unsigned int parentBookmarkID,
                                                        const wchar_t *title,
                                                        unsigned int flags,
                                                        unsigned char titleColor_red,
                                                        unsigned char titleColor_green,
                                                        unsigned char titleColor_blue,
                                                        unsigned int destinationPageIndex,
                                                        unsigned int destinationX_u,
                                                        unsigned int destinationY_u));

   unsigned int resourceID = func(context,
                                  parentBookmarkID,
                                  title,
                                  flags,
                                  GetRValue(titleColor),
                                  GetGValue(titleColor),
                                  GetBValue(titleColor),
                                  destinationPageIndex,
                                  destinationX_u,
                                  destinationY_u);
   ThrowLastErrorIfFail(resourceID != 0);

   return resourceID;
}
//---------------------------------------------------------------------------

unsigned int TLitePDF::CreateBookmarkSibling(unsigned int previousBookmarkID,
                                             const wchar_t *title,
                                             unsigned int flags,
                                             COLORREF titleColor,
                                             unsigned int destinationPageIndex,
                                             unsigned int destinationX_u,
                                             unsigned int destinationY_u)
{
   const char *_func = "TLitePDF::CreateBookmarkSibling";

   ensureLibraryLoaded(_func);

   ThrowIfFail(lib != NULL);
   ThrowIfFail(title != NULL);

   InitFunc(unsigned int, litePDF_CreateBookmarkSibling, (void *pctx,
                                                          unsigned int previousBookmarkID,
                                                          const wchar_t *title,
                                                          unsigned int flags,
                                                          unsigned char titleColor_red,
                                                          unsigned char titleColor_green,
                                                          unsigned char titleColor_blue,
                                                          unsigned int destinationPageIndex,
                                                          unsigned int destinationX_u,
                                                          unsigned int destinationY_u));

   unsigned int resourceID = func(context,
                                  previousBookmarkID,
                                  title,
                                  flags,
                                  GetRValue(titleColor),
                                  GetGValue(titleColor),
                                  GetBValue(titleColor),
                                  destinationPageIndex,
                                  destinationX_u,
                                  destinationY_u);
   ThrowLastErrorIfFail(resourceID != 0);

   return resourceID;
}
//---------------------------------------------------------------------------

#undef ThrowIfFail
#undef ThrowMessageIfFail
#undef ThrowLastErrorIfFail

}; //namespace litePDF
