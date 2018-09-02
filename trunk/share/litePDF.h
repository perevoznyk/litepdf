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
#ifndef litePDFH
#define litePDFH
//---------------------------------------------------------------------------

#include <windows.h>
#include <string>

#ifdef LITEPDF_USE_VCL_EXCEPTION
#include <vcl.h>
#endif

/** @mainpage litePDF
litePDF is a library (DLL), which allows creating new and editing of existing PDF documents with simple API.
Page content is drawn with standard GDI functions through a device context (HDC or TCanvas, in case of Delphi or C++ Builder).<br>
<br>
Main features of litePDF:
<ul>
<li>create new PDF documents in memory only, or with direct write to a disk</li>
<li>load of existing PDF documents</li>
<li>modify or delete of existing pages</li>
<li>copy pages from other documents</li>
<li>incremental update of PDF documents</li>
<li>encryption of PDF documents</li>
<li>digitally sign PDF documents</li>
<li>draw with GDI functions both directly to a page, and to a resource/template (XObject)</li>
<li>draw of created resources into a page</li>
<li>font embedding, complete or subset</li>
<li>font substitution</li>
<li>JPEG compression for images</li>
<li>attach files into PDF documents</li>
<li>low-level PDF operations</li>
</ul>
<br>
litePDF uses an Open Source project <a href="http://podofo.sf.net">PoDoFo</a> for manipulation of PDF documents,
and offers direct PdfDocument pointer to the PoDoFo interface, thus the library users can do anything what the PoDoFo
offers.
*/

/** @file litePDF.h
    @brief C++ interface

    This document describes a C++ interface for litePDF.dll API.
 */

namespace litePDF {

/** @page units Units
Since litePDF 1.2.0.0 the functions which used millimeters as their measure
units can use also inches, or fractions of the millimeters and inches
(see @ref TLitePDFUnit). The reason for the fraction is to not use
architectural dependent types in the API, where the @a double type is.
The unit value is rounded down to unsigned integers in the API. The default
unit is set to millimeters (@ref LitePDFUnit_mm) for backward compatibility.<br>
<br>
Call @ref TLitePDF::GetUnit to determine which unit is currently used. To
change the unit to be used call @ref TLitePDF::SetUnit. For example, to
create a page of size 8.5 inches x 11.0 inches, set the unit
to @ref LitePDFUnit_10th_inch and then call the @ref TLitePDF::AddPage
with the width 85 and height 110 (10-times larger, because the current
unit is 1/10th of an inch).<br>
<br>
To make the calculations easier, the @ref TLitePDF::MMToUnit and
@ref TLitePDF::InchToUnit functions were added, which recalculate
the passed-in value in millimeters, respectively inches, to the currently
set unit, while the rounding to integer values is left for the caller.
There are also the inverse functions, @ref TLitePDF::UnitToMM and
@ref TLitePDF::UnitToInch.<br>
<br>
All functions use the unit set for the current context, except of
the @ref TLitePDF::DrawResource, which has an explicit argument for the unit
value. That's for simplicity, to not need to change the units before
the function is called and then set it back after the call. Similar helper
functions @ref TLitePDF::MMToUnitEx and @ref TLitePDF::InchToUnitEx
are available.<br>
<br>
@note The rounding down from @a double to @a unsigned @a int can suffer
of the @a double precision error, thus it's a good idea to add a little
fraction to the returned values from the conversion helper functions
before the rounding. The @a unitvalues example adds 0.1 to the returned
value from the @ref TLitePDF::MMToUnit to ensure the value will be rounded
"up" in case the @a double precision error makes the value slightly smaller
than it is supposed to be (considered double precision error is around 1e-9).
*/

#define LitePDF_API_Major 2 /**< LitePDF API version Major part; @see LitePDF_API_Minor */
#define LitePDF_API_Minor 0 /**< LitePDF API version Minor part; @see LitePDF_API_Major */

typedef void (__stdcall *MLitePDFErrorEvent)(unsigned int code, const char *msg, void *user_data);

typedef unsigned int (__stdcall *TLitePDFEvalFontFlagCB)(char *inout_faceName,
                                                         unsigned int faceNameBufferSize,
                                                         void *user_data);
/**<
   A callback to evaluate what to do with the specified font. The function
   can be also used to rename the font, without changing the font flag.
   The size of the @a inout_faceName buffer is @a faceNameBufferSize and when
   renaming it, the written value should not be longer than @a faceNameBufferSize,
   including the nul-terminating character.

   The returned value for one font name should be consistent. It's not possible to
   for example once request complete font embedding and the other time to request
   no embedding at all.

   @param inout_faceName [in/out] The font face name to evaluate the flag for.
   @param faceNameBufferSize Size of the @a inout_faceName buffer.
   @param user_data User data provided in @ref TLitePDF::SetEvalFontFlagCallback.
   @return One of @ref TLitePDFFontFlags.
*/

typedef void (__stdcall *TLitePDFAppendSignatureDataFunc)(const char *bytes, unsigned int bytes_len, void *user_data);
/**<
   The function is used within @ref TLitePDF::SaveToFileWithSignManual and @ref TLitePDF::SaveToDataWithSignManual.
   It is called called when more data should be added to hash computation.
*/

typedef void (__stdcall *TLitePDFFinishSignatureFunc)(char *signature, unsigned int *signature_len, void *user_data);
/**<
   The function is used within @ref TLitePDF::SaveToFileWithSignManual and @ref TLitePDF::SaveToDataWithSignManual.
   It is called when all the data are processed, and the signature value is required.
   The @a signature_len contains size of the @a signature buffer. The callback is
   responsible to populate @a signature and @a signature_len with correct values.
   Set @a signature_len to zero on any error. Note the callback is called only once.
*/

#ifdef LITEPDF_USE_VCL_EXCEPTION
class TLitePDFException : public Sysutils::Exception
#else
class TLitePDFException
#endif
{
 private:
   DWORD code;
   #ifndef LITEPDF_USE_VCL_EXCEPTION
   char *msg;
   #endif
 public:
   #ifdef LITEPDF_USE_VCL_EXCEPTION
   __fastcall TLitePDFException(DWORD pCode, const char *pMsg);
   __fastcall TLitePDFException(const TLitePDFException &src);
   virtual __fastcall ~TLitePDFException();
   #else
   TLitePDFException(DWORD pCode, const char *pMsg);
   TLitePDFException(const TLitePDFException &src);
   virtual ~TLitePDFException();
   #endif

   DWORD getCode(void) const;
   /**<
      @return Error code.
   */

   #ifndef LITEPDF_USE_VCL_EXCEPTION
   const char *getMessage(void) const;
   /**<
      @return Error message.
   */
   #endif
};
//---------------------------------------------------------------------------

typedef enum {
   LitePDFUnit_Unknown     = 0, /**< Unknown unit; usually used to indicate an error */
   LitePDFUnit_mm          = 1, /**< Millimeters unit */
   LitePDFUnit_10th_mm     = 2, /**< 1/10th of a millimeter unit; 5 mm is value 50 */
   LitePDFUnit_100th_mm    = 3, /**< 1/100th of a millimeter unit; 5 mm is value 500 */
   LitePDFUnit_1000th_mm   = 4, /**< 1/1000th of a millimeter unit; 5 mm is value 5000 */
   LitePDFUnit_inch        = 5, /**< Inch unit */
   LitePDFUnit_10th_inch   = 6, /**< 1/10th of an inch unit; 5" is value 50 */
   LitePDFUnit_100th_inch  = 7, /**< 1/100th of an inch unit; 5" is value 500 */
   LitePDFUnit_1000th_inch = 8  /**< 1/1000th of an inch unit; 5" is value 5000 */
} TLitePDFUnit;

//---------------------------------------------------------------------------

typedef enum {
   LitePDFFontFlag_Default       = 0, /**< Use the settings as specified by the draw operation */
   LitePDFFontFlag_DoNotEmbed    = 1, /**< Do not embed the font */
   LitePDFFontFlag_EmbedComplete = 2, /**< Embed complete font */
   LitePDFFontFlag_EmbedSubset   = 3, /**< Embed the font with used characters only */
   LitePDFFontFlag_Substitute    = 4  /**< Substitute the font with one of the base fonts, if possible */
} TLitePDFFontFlags;

//---------------------------------------------------------------------------

typedef enum {
   LitePDFDrawFlag_None                   = 0,        /**< None draw flags */
   LitePDFDrawFlag_EmbedFontsNone         = (1 << 4), /**< Do not embed any fonts into resulting PDF.
   @note: Fonts' subset is embeded by default, if none of the @ref LitePDFDrawFlag_EmbedFontsNone, @ref LitePDFDrawFlag_EmbedFontsComplete, @ref LitePDFDrawFlag_EmbedFontsSubset,
      flags is defined; the @ref LitePDFDrawFlag_EmbedFontsNone is to override default font embedding. The reason for this default embedding is
      due to PDF readers not showing correct fonts when they are not part of the PDF file.
      @see LitePDFDrawFlag_EmbedFontsComplete, LitePDFDrawFlag_EmbedFontsSubset */
   LitePDFDrawFlag_EmbedFontsComplete     = (1 << 0), /**< Embed complete fonts into resulting PDF; @see LitePDFDrawFlag_EmbedFontsSubset, LitePDFDrawFlag_EmbedFontsNone */
   LitePDFDrawFlag_EmbedFontsSubset       = (1 << 1), /**< Embed only subset of the fonts, aka used letters; this flag is used before @ref LitePDFDrawFlag_EmbedFontsComplete; @see LitePDFDrawFlag_EmbedFontsNone */
   LitePDFDrawFlag_SubstituteFonts        = (1 << 2), /**< Substitute fonts with base PDF fonts, if possible */
   LitePDFDrawFlag_CompressImagesWithJPEG = (1 << 3)  /**< Compress images with JPEG compression, to get smaller PDF document; this is used only for RGB images */
} TLitePDFDrawFlags;

//---------------------------------------------------------------------------

typedef enum {
   LitePDFEncryptPermission_None        = 0x0,          /**< Nothing from the rest is allowed */
   LitePDFEncryptPermission_Print       = 0x00000004,   /**< Allow printing the document */
   LitePDFEncryptPermission_Edit        = 0x00000008,   /**< Allow modifying the document besides annotations, form fields or changing pages */
   LitePDFEncryptPermission_Copy        = 0x00000010,   /**< Allow text and graphic extraction */
   LitePDFEncryptPermission_EditNotes   = 0x00000020,   /**< Add or modify text annotations or form fields (if ePdfPermissions_Edit is set also allow to create interactive form fields including signature) */
   LitePDFEncryptPermission_FillAndSign = 0x00000100,   /**< Fill in existing form or signature fields */
   LitePDFEncryptPermission_Accessible  = 0x00000200,   /**< Extract text and graphics to support user with disabilities */
   LitePDFEncryptPermission_DocAssembly = 0x00000400,   /**< Assemble the document: insert, create, rotate delete pages or add bookmarks */
   LitePDFEncryptPermission_HighPrint   = 0x00000800,   /**< Print a high resolution version of the document */
   LitePDFEncryptPermission_All         = LitePDFEncryptPermission_Print |
                                          LitePDFEncryptPermission_Edit |
                                          LitePDFEncryptPermission_Copy |
                                          LitePDFEncryptPermission_EditNotes |
                                          LitePDFEncryptPermission_FillAndSign |
                                          LitePDFEncryptPermission_Accessible |
                                          LitePDFEncryptPermission_HighPrint /**< Shortcut for all permissions */

} TLitePDFEncryptPermission;

//---------------------------------------------------------------------------

typedef enum {
   LitePDFEncryptAlgorithm_None  = 0, /**< No encryption algorithm; it can be used only when unsetting prepared encryption */
   LitePDFEncryptAlgorithm_RC4V1 = 1, /**< RC4 Version 1 encryption using a 40bit key */
   LitePDFEncryptAlgorithm_RC4V2 = 2, /**< RC4 Version 2 encryption using a 128bit key */
   LitePDFEncryptAlgorithm_AESV2 = 4, /**< AES encryption with a 128 bit key (PDF1.6) */
   LitePDFEncryptAlgorithm_AESV3 = 8  /**< AES encryption with a 256 bit key (PDF1.7 extension 3) */
} TLitePDFEncryptAlgorithm;

//---------------------------------------------------------------------------

#define LitePDFDocumentInfo_Author           "Author"       /**< an Author of the document */
#define LitePDFDocumentInfo_Creator          "Creator"      /**< a Creator of the document */
#define LitePDFDocumentInfo_Keywords         "Keywords"     /**< the Keywords of the document */
#define LitePDFDocumentInfo_Subject          "Subject"      /**< a Subject of the document */
#define LitePDFDocumentInfo_Title            "Title"        /**< a Title of the document */
#define LitePDFDocumentInfo_Producer         "Producer"     /**< a Producer of the document; this key is read-only */
#define LitePDFDocumentInfo_Trapped          "Trapped"      /**< a trapping state of the document */
#define LitePDFDocumentInfo_CreationDate     "CreationDate" /**< a date of the creation of the document */
#define LitePDFDocumentInfo_ModificationDate "ModDate"      /**< a date of the last modification of the document */

//---------------------------------------------------------------------------

typedef enum {
   LitePDFAnnotationFlag_None           = 0x0000, /**< Default annotation flags */
   LitePDFAnnotationFlag_Invisible      = 0x0001, /**< Do not show nonstandard annotation if there is no annotation handler available */
   LitePDFAnnotationFlag_Hidden         = 0x0002, /**< Do not allow show, print or interact with the annotation */
   LitePDFAnnotationFlag_Print          = 0x0004, /**< Print the annotation */
   LitePDFAnnotationFlag_NoZoom         = 0x0008, /**< Do not scale the annotation's appearance to match the magnification of the page */
   LitePDFAnnotationFlag_NoRotate       = 0x0010, /**< Do not rotate the annotation's appearance to match the rotation of the page */
   LitePDFAnnotationFlag_NoView         = 0x0020, /**< Do not display the annotation on the screen or allow it to interact with the user */
   LitePDFAnnotationFlag_Readonly       = 0x0040, /**< Do not allow the annotation to interact with the user */
   LitePDFAnnotationFlag_Locked         = 0x0080, /**< Do not allow the annotation to be deleted or its properties (including position and size) to be modified by the user */
   LitePDFAnnotationFlag_ToggleNoView   = 0x0100, /**< Invert the interpretation of the NoView flag for certain events */
   LitePDFAnnotationFlag_LockedContents = 0x0200  /**< Do not allow the contents of the annotation to be modified by the user */
} TLitePDFAnnotationFlags;

//---------------------------------------------------------------------------

typedef enum {
   LitePDFAppearance_Normal =   0, /**< Normal appearance */
   LitePDFAppearance_Rollover = 1, /**< Rollover appearance; the default is the normal appearance */
   LitePDFAppearance_Down =     2  /**< Down appearance; the default is the normal appearance */
} TLitePDFAppearance;

//---------------------------------------------------------------------------

typedef enum {
   LitePDFBookmarkFlag_None           = 0x0000, /**< Default bookmark flags */
   LitePDFBookmarkFlag_Italic         = 0x0001, /**< Show bookmark title as an italic text */
   LitePDFBookmarkFlag_Bold           = 0x0002  /**< Show bookmark title as a bold text */
} TLitePDFBookmarkFlags;

//---------------------------------------------------------------------------

class TLitePDF
{
 private:
   HMODULE lib;
   void *context;
   MLitePDFErrorEvent onError;
   void *onErrorUserData;
   DWORD lastErrorCode;
   char *lastErrorMessage;
   TLitePDFEvalFontFlagCB onEvalFontFlag;
   void *onEvalFontFlagUserData;

   FARPROC GetProc(const char *pProcIdent);
   bool checkAPIVersion(unsigned int major,
                        unsigned int minor);
   /*< returns whether DLL's version is the correct API version, as expected by this class */

   void ensureLibraryLoaded(const char *_func);
   void unloadLibrary(void);
   void freeLastError(void);
   void setLastError(DWORD code,
                     const char *msg);

   static void __stdcall litePDFError(unsigned int code,
                                      const char *msg,
                                      void *user_data);

   static unsigned int __stdcall litePDFEvalFontFlag(char *inout_faceName,
                                                     unsigned int faceNameBufferSize,
                                                     void *user_data);
 public:
   TLitePDF();
   /**<
      Creates a new TLiteDPF object.

      @note The library is not thread safe, thus if there is any need for the thread
         safety, then the caller is responsible to provide it on its own.
   */

   virtual ~TLitePDF();

   void setOnError(MLitePDFErrorEvent pOnError,
                   void *pOnErrorUserData);
   /**<
      Sets a custom callback for errors notified by the litePDF library. It's not
      necessary to be set. The errors are those returned during any function calls,
      but also during drawing, for example when some draw operation is not supported.
      Most of the object calls usually throw a @ref TLitePDFException on errors.

      @param pOnError A callback to call.
      @param pOnErrorUserData user data for the callback.

      @see getLastErrorCode, getLastErrorMessage
   */

   DWORD getLastErrorCode(void) const;
   /**<
      Returns the last error code, which was notified by the litePDF library, if any.
      Most of the object calls usually throw a @ref TLitePDFException on errors.

      @return The last error code, or 0, if there was none.

      @see setOnError, getLastErrorMessage
   */

   const char *getLastErrorMessage(void) const;
   /**<
      Returns the last error message, which was notified by the litePDF library, if any.
      Most of the object calls usually throw a @ref TLitePDFException on errors.

      @return The last error message, or NULL, if there was none.

      @see setOnError, getLastErrorCode
   */

   void SetUnit(TLitePDFUnit unitValue);
   /**<
      Sets a unit to use in functions which expect non-pixel size and position values.
      It can be one of TLitePDFUnit values. The default is @ref LitePDFUnit_mm.

      @param unitValue One of TLitePDFUnit values, to set as a unit.

      @see @ref units, GetUnit, MMToUnit, UnitToMM, InchToUnit, UnitToInch
   */

   TLitePDFUnit GetUnit(void);
   /**<
      Gets the currently set unit, which is used in functions which expect
      non-pixel size and position values. It can be one of TLitePDFUnit values.
      The default is @ref LitePDFUnit_mm.

      @return One of TLitePDFUnit values, which is set as the current unit.

      @see @ref units, SetUnit, MMToUnit, UnitToMM, InchToUnit, UnitToInch
   */

   double MMToUnitEx(TLitePDFUnit useUnit,
                     double mmValue) const;
   /**<
      Converts a value from millimeters to @a useUnit. The caller does
      the rounding as needed.

      @param useUnit The @ref TLitePDFUnit unit to convert the value to.
      @param mmValue The value in millimeters to convert.
      @return The @a mmValue converted to @a useUnit unit.

      @see UnitToMMEx, InchToUnitEx, UnitToInchEx, MMToUnit
   */

   double UnitToMMEx(TLitePDFUnit useUnit,
                     double unitValue) const;
   /**<
      Converts a value from @a useUnit to millimeters. The caller does
      the rounding as needed.

      @param useUnit The @ref TLitePDFUnit unit to convert the value from.
      @param unitValue The value in @a useUnit to convert.
      @return The @a unitValue in @a useUnit converted to millimeters.

      @see MMToUnitEx, InchToUnitEx, UnitToInchEx, UnitToMM
   */

   double InchToUnitEx(TLitePDFUnit useUnit,
                       double inchValue) const;
   /**<
      Converts a value from inches to @a useUnit. The caller does
      the rounding as needed.

      @param useUnit The @ref TLitePDFUnit unit to convert the value to.
      @param inchValue The value in inches to convert.
      @return The @a inchValue converted to @a useUnit unit.

      @see UnitToInchEx, MMToUnitEx, UnitToMMEx, InchToUnit
   */

   double UnitToInchEx(TLitePDFUnit useUnit,
                       double unitValue) const;
   /**<
      Converts a value from @a useUnit to inches. The caller does
      the rounding as needed.

      @param useUnit The @ref TLitePDFUnit unit to convert the value from.
      @param unitValue The value in @a useUnit to convert.
      @return The @a unitValue in @a useUnit converted to inches.

      @see InchToUnitEx, MMToUnitEx, UnitToMMEx, UnitToInch
   */

   double MMToUnit(double mmValue);
   /**<
      Converts a value from millimeters to current unit. The caller does
      the rounding as needed.

      @param mmValue A value in millimeters to convert to the current unit.
      @returns The @a mmValue converted to the current unit.

      @see GetUnit, UnitToMM, InchToUnit, UnitToInch, MMToUnitEx
   */

   double UnitToMM(double unitValue);
   /**<
      Converts a value from the current unit to millimeters. The caller does
      the rounding as needed.

      @param unitValue A value in the current unit to convert to millimeters.
      @returns The @a unitValue converted to millimeters.

      @see GetUnit, MMToUnit, InchToUnit, UnitToInch, UnitToMMEx
   */

   double InchToUnit(double inchValue);
   /**<
      Converts a value from inches to the current unit. The caller does
      the rounding as needed.

      @param inchValue A value in inches to convert to the current unit.
      @returns The @a inchValue converted to the current unit.

      @see GetUnit, UnitToInch, MMToUnit, UnitToMM, InchToUnitEx
   */

   double UnitToInch(double unitValue);
   /**<
      Converts a value from the current unit to inches. The caller does
      the rounding as needed.

      @param unitValue A value in the current unit to convert to inches.
      @returns The @a unitValue converted to inches.

      @see GetUnit, InchToUnit, MMToUnit, UnitToMM, UnitToInchEx
   */

   void SetEvalFontFlagCallback(TLitePDFEvalFontFlagCB callback,
                                void *userData);
   /**<
      Sets a callback to evaluate what to do with a font. The @a callback can
      be NULL, to unset any previously set value. See @ref TLitePDFEvalFontFlagCB
      for more information about the @a callback parameters and what it can do.

      @param callback A @ref TLitePDFEvalFontFlagCB callback to set, or NULL.
      @param userData A user data to pass to @a callback when called.
   */

   void PrepareEncryption(const char *userPassword,
                          const char *ownerPassword,
                          unsigned int permissions,
                          unsigned int algorithm);
   /**<
      Prepares encryption for newly created documents. The LitePDF
      object should be empty. The encryption is used only with
      @ref CreateFileDocument and @ref CreateMemDocument, other functions ignore it.
      Use NULL or an empty @a ownerPassword to unset any previously
      set encryption properties.
      Loading an encrypted document lefts it encrypted on save too.

      @param userPassword User's password, can be an empty string, or NULL,
         then the user doesn't need to write any password.
      @param ownerPassword Owner's password. Can be NULL or an empty string, to unset
         encryption properties.
      @param permissions Bit-or of @ref TLitePDFEncryptPermission flags, to set user's
         permissions for the document.
      @param algorithm One of @ref TLitePDFEncryptAlgorithm constants, an algorithm
         to be used to encrypt the document.

      @see CreateFileDocument, CreateMemDocument
   */

   void CreateFileDocument(const char *fileName);
   /**<
      Makes the LitePDF object hold a new PDF, which writes directly to a file.
      The object should not have opened any other PDF data. Call @ref Close,
      to close the file, and possibly save changes to it. Most of the operations
      require memory-based PDF, which can be created with @ref CreateMemDocument.

      @param fileName File name to write the PDF result to.

      @note PoDoFo doesn't support creation of file-based documents with AES encryption,
         thus use for it memory-based documents instead (@ref CreateMemDocument).

      @see PrepareEncryption, CreateFileDocumentW, CreateMemDocument, LoadFromFile
   */

   void CreateFileDocumentW(const wchar_t *fileName);
   /**<
      This is the same as @ref CreateFileDocument, the only difference is that
      the @a fileName is a wide string.
   */

   void CreateMemDocument(void);
   /**<
      Makes the litePDF object hold a memory-based PDF. Such PDF can be
      saved with @ref SaveToFile or  @ref SaveToData.
      The PDF should be closed with @ref Close.

      @see PrepareEncryption, CreateFileDocument, LoadFromFile
   */

   void LoadFromFile(const char *fileName,
                     const char *password,
                     bool loadCompletely,
                     bool forUpdate = false);
   /**<
      Makes the LitePDF object hold a memory-based PDF, which is loaded
      from a disk file. This should be closed with @ref Close.
      The @a loadCompletely parameter is used to determine whether the file
      should be loaded into memory completely, or when the file can be read
      on demand. Using complete load requires more memory, but the disk
      file can be changed freely in the background, while incomplete load
      requires left the file without changes while being opened.
      The @a forUpdate parameter specifies whether the file is being opened
      for incremental update. In that case saving such document will result
      in the changes being appended to the end of the original document,
      instead of resaving whole document.

      @param fileName File name to load the PDF from.
      @param password Password to use for encrypted documents.
      @param loadCompletely Set to true when the file should be loaded completely
         into memory, or false to keep the disk file in use while working with it.
      @param forUpdate Set to true to open the file for incremental update,
         or set to false otherwise. Default is false.

      @see LoadFromFileW
   */

   void LoadFromFileW(const wchar_t *fileName,
                      const char *password,
                      bool loadCompletely,
                      bool forUpdate = false);
   /**<
      This is the same as @ref LoadFromFile, the only difference is that
      the @a fileName is a wide string.
   */

   void LoadFromData(const BYTE *data,
                     unsigned int dataLength,
                     const char *password,
                     bool forUpdate = false);
   /**<
      Makes the LitePDF object hold a memory-based PDF, which is loaded
      with a PDF data. This should be closed with @ref Close.
      The @a forUpdate parameter specifies whether the file is being opened
      for incremental update. In that case saving such document will result
      in the changes being appended to the end of the original document,
      instead of resaving whole document.

      @param data PDF data to load.
      @param dataLength Length of PDF data.
      @param password Password to use for encrypted documents.
      @param forUpdate Set to true to open the file for incremental update,
         or set to false otherwise. Default is false.

      @see CreateMemDocument, SaveToFile, SaveToData
   */

   void SaveToFile(const char *fileName);
   /**<
      Saves memory-based PDF into a file. The object should hold PDF created only
      with @ref CreateMemDocument, @ref LoadFromFile or @ref LoadFromData.
      Using any other object results in an error.

      In case the PDF document had been loaded with @ref LoadFromFile,
      @ref LoadFromFileW or @ref LoadFromData with its @a forUpdate
      parameter being true, the resulting document will contain the changes as
      an incremental update (appended at the end of the original document), otherwise
      the whole document is completely rewritten.

      @param fileName File name to which save the memory-based PDF.

      @note The only valid operation after this is either close the document
         with @ref Close, or free the @ref TLitePDF object.

      @see SaveToFileW, SaveToData, SaveToFileWithSign, Close
   */

   void SaveToFileW(const wchar_t *fileName);
   /**<
      This is the same as @ref SaveToFile, the only difference is that
      the @a fileName is a wide string.
   */

   bool SaveToData(BYTE *data,
                   unsigned int *dataLength);
   /**<
      Saves memory-based PDF into a data. The object should hold PDF created only
      with @ref CreateMemDocument, @ref LoadFromFile or @ref LoadFromData.
      Using any other object results in an error.

      In case the PDF document had been loaded with @ref LoadFromFile,
      @ref LoadFromFileW or @ref LoadFromData with its @a forUpdate
      parameter being true, the resulting document will contain the changes as
      an incremental update (appended at the end of the original document), otherwise
      the whole document is completely rewritten.

      @param data [out] Actual data to store the PDF content to. It can be NULL, in which case
         the @a dataLength is populated with large-enough value to hold the whole data.
      @param dataLength [in/out] Tells how many bytes can be stored in @a data. If @a data
         is NULL, then it is set to large-enough value. Passing non-NULL @a data with no enough
         large buffer results in a failure with no change on @a dataLength.
      @return Whether succeeded.

      @note The only valid operation after this is either call of @ref SaveToData again,
         to get information about necessary buffer size or data itself, close the document
         with @ref Close, or free the @ref TLitePDF object.

      @see SaveToFile, SaveToDataWithSign, Close
   */

   void Close(void);
   /**<
      Closes PDF data in a LitePDF object, thus the object doesn't hold anything afterward,
      aka it's like a newly created object.  The function does nothing, if the object doesn't
      hold any data. In case of any drawing in progress, the drawing is discarded, same as any
      unsaved changes to the memory-based PDF documents. It also unsets any encryption properties,
      previously set by @ref PrepareEncryption.

      @see AddPage, InsertPage, UpdatePage, FinishPage
   */

   unsigned int GetPageCount(void);
   /**<
      Returns count of pages in an opened PDF document.

      @return Count of pages.
   */

   void GetPageSize(unsigned int pageIndex,
                    unsigned int *width_u,
                    unsigned int *height_u);
   /**<
      Gets size of an existing page, in the current unit.

      @param pageIndex Page index for which get the page size; counts from 0.
      @param width_u [out] Width of the page in the current unit.
      @param height_u [out] Height of the page in the current unit.

      @see GetUnit
   */

   int GetPageRotation(unsigned int pageIndex);
   /**<
      Gets rotation of an existing page, in degrees. Expected values are 0, 90, 180 and 270.

      @param pageIndex Page index for which get the page size; counts from 0.
      @return Rotation of the page, in degrees.

      @see SetPageRotation
   */

   void SetPageRotation(unsigned int pageIndex,
                        int degrees);
   /**<
      Sets rotation of an existing page, in degrees. Expected values are 0, 90, 180 and 270.

      @param pageIndex Page index for which get the page size; counts from 0.
      @param degrees Rotation of the page to set, in degrees.

      @see GetPageRotation
   */

   HDC AddPage(unsigned int width_u,
               unsigned int height_u,
               unsigned int width_px,
               unsigned int height_px,
               unsigned int drawFlags);
   /**<
      Begins drawing into a new page into the PDF document of the given size.
      Newly created page is added as the last page of the PDF document.
      This cannot be called when other drawing is in progress.

      @param width_u Width of the new page in the current unit.
      @param height_u Height of the new page in the current unit.
      @param width_px Width of the new page in pixels.
      @param height_px Height of the new page in pixels.
      @param drawFlags Flags for drawing functions. This is a bit-or of @ref TLitePDFDrawFlags values
         and influences only @ref AddPage, @ref InsertPage, @ref UpdatePage
         and @ref AddResource functions.
      @return Device context into which can be drawn with standard GDI functions.
         Finish the drawing, and the page addition, with @ref FinishPage.

      @note Larger pixel page size produces more precise font mapping.

      @see GetUnit, InsertPage, UpdatePage, FinishPage, AddResource
   */

   HDC InsertPage(unsigned int pageIndex,
                  unsigned int width_u,
                  unsigned int height_u,
                  unsigned int width_px,
                  unsigned int height_px,
                  unsigned int drawFlags);
   /**<
      Begins drawing into a new page into the  PDF document of the given size.
      Newly created page is inserted at the given position of the PDF document.
      This cannot be called when other drawing is in progress.
      If the index is out of bounds, then the page is added ad the end, like with
      @ref AddPage.

      @param pageIndex Page index where to insert the page; counts from 0.
      @param width_u Width of the new page in the current unit.
      @param height_u Height of the new page in the current unit.
      @param width_px Width of the new page in pixels.
      @param height_px Height of the new page in pixels.
      @param drawFlags Flags for drawing functions. This is a bit-or of @ref TLitePDFDrawFlags values
         and influences only @ref AddPage, @ref InsertPage, @ref UpdatePage
         and @ref AddResource functions.
      @return Device context into which can be drawn with standard GDI functions.
         Finish the drawing, and the page insertion, with @ref FinishPage.

      @note Larger pixel page size produces more precise font mapping.

      @see GetUnit, GetPageCount, AddPage, UpdatePage, FinishPage, AddResource
   */

   HDC UpdatePage(unsigned int pageIndex,
                  unsigned int width_px,
                  unsigned int height_px,
                  unsigned int drawFlags);
   /**<
      Begins drawing into an already existing page. The page size in the current unit
      can be found by @ref GetPageSize. The function fails, and returns NULL,
      if the @a pageIndex is out of bounds.

      @param pageIndex Page index which to update; counts from 0.
      @param width_px Width of the new page in pixels.
      @param height_px Height of the new page in pixels.
      @param drawFlags Flags for drawing functions. This is a bit-or of @ref TLitePDFDrawFlags values
         and influences only @ref AddPage, @ref InsertPage, @ref UpdatePage
         and @ref AddResource functions.
      @return Device context into which can be drawn with standard GDI functions.
         Finish the drawing, and the page update, with @ref FinishPage.

      @see GetPageCount, AddPage, InsertPage, FinishPage, AddResource
   */

   void FinishPage(HDC hDC);
   /**<
      Tells litePDF that drawing into the page is finished and that it can
      be processed into PDF. The @a hDC is not valid after this call any more.

      @param hDC Device context previously returned by @ref AddPage,
         @ref InsertPage or @ref UpdatePage.
   */

   HDC AddResource(unsigned int width_u,
                   unsigned int height_u,
                   unsigned int width_px,
                   unsigned int height_px,
                   unsigned int drawFlags);
   /**<
      Begins drawing into a new resource into the PDF document of the given size.
      This cannot be called when other drawing is in progress.

      @param width_u Width of the new page in the current unit.
      @param height_u Height of the new page in the current unit.
      @param width_px Width of the new page in pixels.
      @param height_px Height of the new page in pixels.
      @param drawFlags Flags for drawing functions. This is a bit-or of @ref TLitePDFDrawFlags values
         and influences only @ref AddPage, @ref InsertPage, @ref UpdatePage
         and @ref AddResource functions.
      @return Device context into which can be drawn with standard GDI functions.
         Finish the drawing, and the resource addition, with @ref FinishResource.

      @note Larger pixel resource size produces more precise font mapping.

      @see GetUnit, AddPage, InsertPage, UpdatePage, FinishResource, DrawResource
   */

   unsigned int FinishResource(HDC hDC);
   /**<
      Tells litePDF that drawing into the resource is finished and that it can
      be processed into PDF. The @a hDC is not valid after this call any more.

      @param hDC Device context previously returned by @ref AddResource.
      @return Newly created resource ID, or 0 on error.

      @see AddResource, AddPageFromAsResource, DrawResource
   */

   void DeletePage(unsigned int pageIndex);
   /**<
      Deletes page at given index. It doesn't delete page resources, because these can
      be used by other pages.

      @param pageIndex Page index which to update; counts from 0.
      @return Whether succeeded.

      @see GetPageCount, PageToResource
   */

   void AddPagesFrom(litePDF::TLitePDF *from,
                     unsigned int pageIndex,
                     unsigned int pageCount);
   /**<
      Adds existing pages as the last pages from another PDF. Both objects should
      hold memory-based documents.

      @param from a LitePDF object from which add the pages.
      @param pageIndex Page index which to add from @a from; counts from 0.
      @param pageCount How many pages to add; 0 means whole document.

      @note The two objects cannot be the same.

      @see GetPageCount, InsertPageFrom, PageToResource
   */

   void InsertPageFrom(unsigned int pageIndexTo,
                       litePDF::TLitePDF *from,
                       unsigned int pageIndexFrom);
   /**<
      Inserts an existing page at the given index from another PDF. Both objects should
      hold memory-based documents.

      @param pageIndexTo Page index where to add the page; counts from 0. Adds page
         at the end, if out of bounds.
      @param from a LitePDF object, from which add the page.
      @param pageIndexFrom Page index which to add from @a from; counts from 0.

      @note The two objects cannot be the same.

      @see GetPageCount, AddPagesFrom, PageToResource
   */

   unsigned int AddPageFromAsResource(litePDF::TLitePDF *from,
                                      unsigned int pageIndex,
                                      bool useTrimBox = false);
   /**<
      Adds an existing page as a resource of a given PDF. This resource can be
      referenced multiple times by its identifier. Both objects should
      hold memory-based documents.

      @param from a LitePDF object, from which add the page.
      @param pageIndex Page index which to add from @a from; counts from 0.
      @param useTrimBox If true, try to use trimbox for size of the resource (XObject)
      @return Resource identifier, or 0 on error.

      @note The two objects cannot be the same.

      @see GetPageCount, AddPagesFrom, PageToResource, GetResourceSize,
         DrawResource
   */

   unsigned int PageToResource(unsigned int pageIndex);
   /**<
      Creates a resource, which will reference an existing page.
      The page itself is not deleted after call of this.

      @param pageIndex Page index for which create the resource reference; counts from 0.
      @return Resource identifier, or 0 on error.
      
      @see GetPageCount, AddPagesFrom, AddPageFromAsResource, GetResourceSize,
         DrawResource
   */

   void GetResourceSize(unsigned int resourceID,
                        unsigned int *width_u,
                        unsigned int *height_u);
   /**<
      Gets size of an existing resource, in the current unit. The resource ID
      was returned from @ref AddPageFromAsResource or @ref FinishResource.

      @param resourceID Resource ID for which get the size.
      @param width_u [out] Width of the resource, in the current unit.
      @param height_u [out] Height of the resource, in the current unit.

      @see GetUnit, AddPageFromAsResource, DrawResource
   */

   void DrawResource(unsigned int resourceID,
                     unsigned int pageIndex,
                     TLitePDFUnit unitValue,
                     int x,
                     int y,
                     int scaleX,
                     int scaleY);
   /**<
      Draws an existing resource at the given position. The resource ID 
      was returned from @ref AddPageFromAsResource, @ref PageToResource or @ref FinishResource.
      The @a unitValue is used for both the position and the scale. In case
      of the scale, it defines only the ratio to the base unit.
      For example, if the @a unitValue is either @ref LitePDFUnit_1000th_mm or
      @ref LitePDFUnit_1000th_inch, then the ratio for the @a scaleX and @a scaleY
      is used 1/1000 (where 1000 means the same size as the resource is in this case).

      @param resourceID Resource ID to draw.
      @param pageIndex Page index to which draw; counts from 0.
      @param unitValue A unit to use for the @a x and @a y, and a ratio for the @a scaleX and @a scaleY.
      @param x Where to draw on the page, X axes, in the given @a unitValue unit,
         with left-top corner being [0,0].
      @param y Where to draw on the page, Y axes, in the given @a unitValue unit,
         with left-top corner being [0,0].
      @param scaleX Scale factor of the page for the X axes, using the @a unitValue ratio.
      @param scaleY Scale factor of the page for the Y axes, using the @a unitValue ratio.

      @see GetPageCount, AddPageFromAsResource, PageToResource, FinishResource,
         GetResourceSize, DrawResourceWithMatrix
   */

   void DrawResourceWithMatrix(unsigned int resourceID,
                               unsigned int pageIndex,
                               double a,
                               double b,
                               double c,
                               double d,
                               double e,
                               double f);
   /**<
      Draws an existing resource with given transformation matrix. All
      the transformation values are passed into PDF directly, without any
      conversion. The resource ID was returned from @ref AddPageFromAsResource
      or @ref FinishResource. The constructed transformation matrix
      is a standard 3x3 matrix:<BR>
      <CODE>   | a b 0 |</CODE><BR>
      <CODE>   | c d 0 |</CODE><BR>
      <CODE>   | e f 1 |</CODE>

      @param resourceID Resource ID to draw.
      @param pageIndex Page index to which draw; counts from 0.
      @param a Transformation matrix [ a b c d e f ] parameter 'a', in PDF units.
      @param b Transformation matrix [ a b c d e f ] parameter 'b', in PDF units.
      @param c Transformation matrix [ a b c d e f ] parameter 'c', in PDF units.
      @param d Transformation matrix [ a b c d e f ] parameter 'd', in PDF units.
      @param e Transformation matrix [ a b c d e f ] parameter 'e', in PDF units.
      @param f Transformation matrix [ a b c d e f ] parameter 'f', in PDF units.

      @note Each of a, b, c, d, e, f is rounded down to nearest 1/1000th of PDF units.

      @see GetPageCount, AddPageFromAsResource, FinishResource,
         GetResourceSize, DrawResource
   */

   void SetDocumentInfo(const char *name,
                        const wchar_t *value);
   /**<
      Sets information about the document. The name can be one
      of the LitePDFDocumentInfo predefined constants.

      @param name Document info property name to set.
      @param value Null-terminated Unicode value to set.
   */

   bool GetDocumentInfoExists(const char *name);
   /**<
      Checks whether information about the document of the given name exists.
      The name can be one of the LitePDFDocumentInfo predefined constants.

      @param name Document info property name to test.
      @return Whether succeeded and the document information is set.
   */

   std::wstring GetDocumentInfo(const char *name);
   /**<
      Gets information about the document. The name can be one
      of the LitePDFDocumentInfo predefined constants.

      @param name Document info property name to get.
      @return Unicode value.
   */

   bool GetDocumentIsSigned(void);
   /**<
      Checks whether currently opened document is already signed. Signing already
      signed document can cause breakage of previous signatures, thus it's good
      to test whether the loaded document is signed, before signing it.

      @return Whether the opened document is already signed.

      @see GetSignatureCount, SaveToFileWithSign, SaveToDataWithSign
   */

   unsigned int GetSignatureCount(void);
   /**<
      Provides how many signature fields the currently opened document contains.
      It returns the count of the all fields, not only those already signed.

      @return How many signatures the currently opened document contains.

      @note The litePDF caches the list of the existing signature fields for performance
         reasons and it rebuilds it whenever this function is called or when the
         @ref CreateSignature is called, thus if there are made any changes
         directly with the PoDoFo API after the cache had been created, then make sure
         you call this function again to avoid a use-after-free or an outdated information
         being used. The litePDF will try to keep the cache up to date as needed, but
         it cannot cover every case, especially not the one when the PoDoFo API is used.

      @see GetDocumentIsSigned, GetSignatureHasData, GetSignatureData
   */

   std::string GetSignatureName(unsigned int index);
   /**<
      Gets the signature field name at the given @a index.

      @param index Which signature field name to get; counts from 0. This might be less
         than @ref GetSignatureCount.
      @return An ASCII name of the field.
   */

   unsigned int CreateSignature(const char *name,
                                unsigned int annotationPageIndex,
                                int annotationX_u,
                                int annotationY_u,
                                int annotationWidth_u,
                                int annotationHeight_u,
                                unsigned int annotationFlags);
   /**<
      Creates a new signature field named @a name. The field is created completely empty.
      Use @ref SetSignatureDate, @ref SetSignatureReason,
      @ref SetSignatureLocation, @ref SetSignatureCreator,
      @ref SetSignatureAppearance and such to populate it with required values.
      Finally, to sign the signature field use @ref SaveToFileWithSign family
      functions.

      @param name Signature field name to use. This should be unique.
      @param annotationPageIndex Page index where to place the signature annotation.
      @param annotationX_u X-origin of the annotation on the page, in the current unit.
      @param annotationY_u Y-origin of the annotation on the page, in the current unit.
      @param annotationWidth_u Width of the annotation on the page, in the current unit.
      @param annotationHeight_u Height of the annotation on the page, in the current unit.
      @param annotationFlags Bit-or of @ref TLitePDFAnnotationFlags flags.
      @return The index of the added signature field.

      @see GetSignatureCount, GetSignatureName
   */

   bool GetSignatureHasData(unsigned int index);
   /**<
      Checks whether the given signature field contains any data, which
      means whether the signature field is signed.

      @param index Which signature data to get; counts from 0. This might be less
         than @ref GetSignatureCount.
      @return Whether the given signature contains any data.

      @see GetSignatureData
   */

   bool GetSignatureData(unsigned int index,
                         BYTE *data,
                         unsigned int *dataLength);
   /**<
      Gathers raw signature data for the given signature in the currently opened document.
      Use @ref GetSignatureHasData to check whether the given signature field
      is signed or not.

      @param index Which signature data to get; counts from 0. This might be less
         than @ref GetSignatureCount.
      @param data [out] Actual data to store the signature content to. It can be NULL, in which case
         the @a dataLength is populated with large-enough value to hold the whole data.
      @param dataLength [in/out] Tells how many bytes can be stored in @a data. If @a data
         is NULL, then it is set to large-enough value. Passing non-NULL @a data with no enough
         large buffer results in a failure with no change on @a dataLength.
      @return Whether succeeded.

      @see GetDocumentIsSigned, GetSignatureCount, GetSignatureRanges
   */

   bool GetSignatureRanges(unsigned int index,
                           unsigned __int64 *pRangesArray,
                           unsigned int *pRangesArrayLength);
   /**<
      Gathers signature ranges, that is the actual offsets into the opened file
      which had been used to create the signature data (@ref GetSignatureData).
      The array is a pair of numbers, where the first number is an offset into the file
      from its beginning and the second number is the number of bytes being used for
      the signature from this offset.

      @param index Which signature ranges to get; counts from 0. This might be less
         than @ref GetSignatureCount.
      @param pRangesArray [out] Actual array to store the signature ranges to. It can be NULL,
         in which case the @a pRangesArrayLength is populated with large-enough value to hold
         the whole array.
      @param pRangesArrayLength [in/out] Tells how many items can be stored in @a pRangesArray.
         If @a pRangesArray is NULL, then it is set to large-enough value. Passing non-NULL
         @a pRangesArray with no enough large array results in a failure with no change
         on @a pRangesArrayLength.
      @return Whether succeeded.

      @note This function works only for the signatures which use this kind of signature method.

      @see GetDocumentIsSigned, GetSignatureCount, GetSignatureData
   */

   void SetSignatureDate(unsigned int index,
                         __int64 dateOfSign);
   /**<
      Sets signature field date of sign.

      @param index Which signature to use; counts from 0. This might be less
         than @ref GetSignatureCount.
      @param dateOfSign Date of sign, like Unix time_t, when the signature was created; less than
         or equal to 0 means today. The value can be clamp on 32-bit systems.

      @see GetSignatureDate, GetSignatureCount
   */

   __int64 GetSignatureDate(unsigned int index);
   /**<
      Gets signature field date of sign.

      @param index Which signature to use; counts from 0. This might be less
         than @ref GetSignatureCount.
      @return The date of sign. It's like Unix time_t, as set by the signature field creator.
         The value can be clamp on 32-bit systems.

      @see SetSignatureDate, GetSignatureCount
   */

   void SetSignatureReason(unsigned int index,
                           const wchar_t *reason);
   /**<
      Sets signature reason.

      @param index Which signature to use; counts from 0. This might be less
         than @ref GetSignatureCount.
      @param reason The value to set.

      @see GetSignatureReason, GetSignatureCount
   */

   std::wstring GetSignatureReason(unsigned int index);
   /**<
      Gets signature reason.

      @param index Which signature to use; counts from 0. This might be less
         than @ref GetSignatureCount.
      @return A Unicode string containing the value.

      @see SetSignatureReason, GetSignatureCount
   */

   void SetSignatureLocation(unsigned int index,
                             const wchar_t *location);
   /**<
      Sets signature location, aka where the signature had been made. This can be left unset.

      @param index Which signature to use; counts from 0. This might be less
         than @ref GetSignatureCount.
      @param location The value to set.

      @see GetSignatureLocation, GetSignatureCount
   */

   std::wstring GetSignatureLocation(unsigned int index);
   /**<
      Gets signature location.

      @param index Which signature to use; counts from 0. This might be less
         than @ref GetSignatureCount.
      @return A Unicode string containing the value.

      @see SetSignatureLocation, GetSignatureCount
   */

   void SetSignatureCreator(unsigned int index,
                            const char *creator);
   /**<
      Sets signature creator. This can be left unset.

      @param index Which signature to use; counts from 0. This might be less
         than @ref GetSignatureCount.
      @param creator The value to set.

      @see GetSignatureCreator, GetSignatureCount
   */

   std::string GetSignatureCreator(unsigned int index);
   /**<
      Gets signature creator.

      @param index Which signature to use; counts from 0. This might be less
         than @ref GetSignatureCount.
      @return An ASCII string containing the value.

      @see SetSignatureCreator, GetSignatureCount
   */

   void SetSignatureAppearance(unsigned int index,
                               TLitePDFAppearance appearanceType,
                               unsigned int resourceID,
                               int offsetX_u,
                               int offsetY_u);
   /**<
      Sets the signature appearance.

      @param index Which signature to use; counts from 0. This might be less
         than @ref GetSignatureCount.
      @param appearanceType One of the @ref LitePDFAppearance_Normal, @ref LitePDFAppearance_Rollover
         and @ref LitePDFAppearance_Down contacts. At least the @ref LitePDFAppearance_Normal type
         should be set, if the appearance of the signature is requested.
      @param resourceID An existing resource ID of the annotation content, as shown to the user.
      @param offsetX_u X-offset of the resource inside the annotation of the signature, in the current unit.
      @param offsetY_u Y-offset of the resource inside the annotation of the signature, in the current unit.

      @note The resource position offset is from [left, top] corner of the annotation rectangle.

      @see GetUnit, AddResource, GetSignatureCount, CreateSignature
   */

   void SetSignatureSize(unsigned int requestBytes);
   /**<
      Sets how many bytes the signature may require. The default value is 2048 bytes
      and it is automatically adjusted when the @ref SaveToFileWithSign or
      @ref SaveToDataWithSign are used. The manual signing functions
      require this value to be set before signing, if the final hash with the certificate
      exceeds the default size.

      This value is remembered in general, not for any signature in particular.

      @param requestBytes How many bytes the signature will require.

      @see SaveToFileWithSignManual, SaveToFileWithSignManualW, SaveToDataWithSignManual
   */

   void AddSignerPFX(const BYTE *pfxData,
                     unsigned int pfxDataLength,
                     const char *pfxPassword);
   /**<
      Adds a signer to be used when digitally signing the document with
      @ref SaveToFileWithSign or @ref SaveToDataWithSign.
      The passed-in certificate is in the PFX format and should include
      the private key.

      @param pfxData A certificate with private key in the PFX format.
      @param pfxDataLength A length of the @a pfxData.
      @param pfxPassword A password to use to open the PFX certificate; can be NULL.

      @see AddSignerPEM
   */

   void AddSignerPEM(const BYTE *pemData,
                     unsigned int pemDataLength,
                     const BYTE *pkeyData,
                     unsigned int pkeyDataLength,
                     const char *pkeyPassword);
   /**<
      Adds a signer to be used when digitally signing the document with
      @ref SaveToFileWithSign or @ref SaveToDataWithSign.
      The passed-in certificate and private key are in the PEM format.

      @param pemData A certificate in the PEM format.
      @param pemDataLength A length of the @a pemData.
      @param pkeyData A private key for the certificate, in the PEM format.
      @param pkeyDataLength A length of the @a pkeyData.
      @param pkeyPassword A password to use to open the private key; can be NULL.

      @see AddSignerPFX
   */

   void SaveToFileWithSign(const char *fileName,
                           unsigned int signatureIndex);
   /**<
      Digitally signs a PDF document opened at the LitePDF object. The caller is
      responsible to set at least one signer with either @ref AddSignerPFX
      or @ref AddSignerPEM first. An alternative @ref SaveToFileWithSignManual
      is provided when it's required to compute the signature hash manually by the caller.

      In case the document had been loaded with @ref LoadFromFile,
      @ref LoadFromFileW or @ref LoadFromData with its @a forUpdate
      parameter being true, the resulting document will contain the changes as
      an incremental update (appended at the end of the original document), otherwise
      the whole document is completely rewritten.

      @param fileName A file name where to save signed PDF document.
      @param signatureIndex Which signature to use; counts from 0. This might be less
         than @ref GetSignatureCount.

      @note The only valid operation after this is either close the document
         with @ref Close, or free the @ref TLitePDF object.

      @note Signing already signed document can cause breakage of previous signatures, thus
         check whether the loaded document is already signed with @ref GetDocumentIsSigned.
         Load the document with its @a forUpdate parameter set to true, to sign an existing document.

      @see SaveToFileWithSignW, SaveToDataWithSign
   */

   void SaveToFileWithSignW(const wchar_t *fileName,
                            unsigned int signatureIndex);
   /**<
      This is the same as @ref SaveToFileWithSign, the only difference is that
      the @a fileName is a wide string.
   */

   bool SaveToDataWithSign(unsigned int signatureIndex,
                           BYTE *data,
                           unsigned int *dataLength);
   /**<
      Digitally signs a PDF document opened at the LitePDF object. The caller is
      responsible to set at least one signer with either @ref AddSignerPFX
      or @ref AddSignerPEM first. An alternative @ref SaveToDataWithSignManual
      is provided when it's required to compute the signature hash manually by the caller.

      In case the document had been loaded with @ref LoadFromFile,
      @ref LoadFromFileW or @ref LoadFromData with its @a forUpdate
      parameter being true, the resulting document will contain the changes as
      an incremental update (appended at the end of the original document), otherwise
      the whole document is completely rewritten.

      @param signatureIndex Which signature to use; counts from 0. This might be less
         than @ref GetSignatureCount.
      @param data [out] Actual data to store the PDF content to. It can be NULL, in which case
         the @a dataLength is populated with large-enough value to hold the whole data.
      @param dataLength [in/out] Tells how many bytes can be stored in @a data. If @a data
         is NULL, then it is set to large-enough value. Passing non-NULL @a data with no enough
         large buffer results in a failure with no change on @a dataLength.
      @return Whether succeeded.

      @note The only valid operation after this is either call of @ref SaveToDataWithSign again,
         to get information about necessary buffer size or data itself, close the document
         with @ref Close, or free the @ref TLitePDF object.

      @note Signing already signed document can cause breakage of previous signatures, thus
         check whether the loaded document is already signed with @ref GetDocumentIsSigned.
         Load the document with its @a forUpdate parameter set to true, to sign an existing document.

      @see SaveToFileWithSign
   */

   void SaveToFileWithSignManual(const char *fileName,
                                 unsigned int signatureIndex,
                                 TLitePDFAppendSignatureDataFunc appendSignatureData,
                                 void *append_user_data,
                                 TLitePDFFinishSignatureFunc finishSignature,
                                 void *finish_user_data);
   /**<
      Digitally signs a PDF document opened at the LitePDF object. The caller is
      responsible for a detached hash computations and related certificate management.

      In case the document had been loaded with @ref LoadFromFile,
      @ref LoadFromFileW or @ref LoadFromData with its @a forUpdate
      parameter being true, the resulting document will contain the changes as
      an incremental update (appended at the end of the original document), otherwise
      the whole document is completely rewritten.

      @param fileName A file name where to save signed PDF document.
      @param signatureIndex Which signature to use; counts from 0. This might be less
         than @ref GetSignatureCount.
      @param appendSignatureData Called when more data should be added to hash computation.
         The function cannot be NULL, even when called the second time, to get actual data.
      @param append_user_data User data value for the @a appendSignatureData callback.
      @param finishSignature Called when all the data are processed, and the signature
         value is required. The @a signature_len contains size of the @a signature buffer.
         The callback is responsible to populate @a signature and @a signature_len with
         correct values. Set @a signature_len to zero on any error.
         Note the callback is called only once.
         The function cannot be NULL, even when called the second time, to get actual data.
      @param finish_user_data User data value for the @a finishSignature callback.

      @note The only valid operation after this is either close the document
         with @ref Close, or free the @ref TLitePDF object.

      @note Signing already signed document can cause breakage of previous signatures, thus
         check whether the loaded document is already signed with @ref GetDocumentIsSigned.
         Load the document with its @a forUpdate parameter set to true, to sign an existing document.

      @see SaveToFileWithSign, SaveToFileWithSignManualW, SaveToDataWithSignManual
   */

   void SaveToFileWithSignManualW(const wchar_t *fileName,
                                  unsigned int signatureIndex,
                                  TLitePDFAppendSignatureDataFunc appendSignatureData,
                                  void *append_user_data,
                                  TLitePDFFinishSignatureFunc finishSignature,
                                  void *finish_user_data);
   /**<
      This is the same as @ref SaveToFileWithSignManual, the only difference is that
      the @a fileName is a wide string.
   */

   bool SaveToDataWithSignManual(unsigned int signatureIndex,
                                 TLitePDFAppendSignatureDataFunc appendSignatureData,
                                 void *append_user_data,
                                 TLitePDFFinishSignatureFunc finishSignature,
                                 void *finish_user_data,
                                 BYTE *data,
                                 unsigned int *dataLength);
   /**<
      Digitally signs a PDF document opened at the LitePDF object. The caller is
      responsible for a detached hash computations and related certificate management.

      In case the document had been loaded with @ref LoadFromFile,
      @ref LoadFromFileW or @ref LoadFromData with its @a forUpdate
      parameter being true, the resulting document will contain the changes as
      an incremental update (appended at the end of the original document), otherwise
      the whole document is completely rewritten.

      @param signatureIndex Which signature to use; counts from 0. This might be less
         than @ref GetSignatureCount.
      @param appendSignatureData Called when more data should be added to hash computation.
         The function cannot be NULL, even when called the second time, to get actual data.
      @param append_user_data User data value for the @a appendSignatureData callback.
      @param finishSignature Called when all the data are processed, and the signature
         value is required. The @a signature_len contains size of the @a signature buffer.
         The callback is responsible to populate @a signature and @a signature_len with
         correct values. Set @a signature_len to zero on any error.
         Note the callback is called only once.
         The function cannot be NULL, even when called the second time, to get actual data.
      @param finish_user_data User data value for the @a finishSignature callback.
      @param data [out] Actual data to store the PDF content to. It can be NULL, in which case
         the @a dataLength is populated with large-enough value to hold the whole data.
      @param dataLength [in/out] Tells how many bytes can be stored in @a data. If @a data
         is NULL, then it is set to large-enough value. Passing non-NULL @a data with no enough
         large buffer results in a failure with no change on @a dataLength.
      @return Whether succeeded.

      @note The only valid operation after this is either call of @ref SaveToDataWithSignManual again,
         to get information about necessary buffer size or data itself, close the document
         with @ref Close, or free the @ref TLitePDF object.

      @note Signing already signed document can cause breakage of previous signatures, thus
         check whether the loaded document is already signed with @ref GetDocumentIsSigned.
         Load the document with its @a forUpdate parameter set to true, to sign an existing document.

      @see SaveToFileWithSignManual, SaveToFileWithSign
   */

   void EmbedFile(const char *fileName);
   /**<
      Embeds a file into a PDF document.

      @param fileName File name of the file to be attached.
      @return Whether succeeded.

      @note Files can be embed only to memory-based documents.

      @note The path is stripped from the @a fileName. The @a fileName is used as a key,
         aka it's not possible to embed two files of the same name into a PDF document.

      @see EmbedFileW, EmbedData, CreateMemDocument
   */

   void EmbedFileW(const wchar_t *fileName);
   /**<
      This is the same as @ref EmbedFile, the only difference is that
      the @a fileName is a wide string.
   */

   void EmbedData(const char *fileName,
                  const BYTE *data,
                  unsigned int dataLength);
   /**<
      Embeds a data (file) into a PDF document.

      @param fileName File name to be used for the data identification.
      @param data Actual data to be attached.
      @param dataLength Length of the data.

      @note Data can be embed only to memory-based documents.

      @note The path is stripped from the @a fileName. The @a fileName is used as a key,
         aka it's not possible to embed two files of the same name into a PDF document.

      @see EmbedDataW, EmbedFile, CreateMemDocument
   */

   void EmbedDataW(const wchar_t *fileName,
                   const BYTE *data,
                   unsigned int dataLength);
   /**<
      This is the same as @ref EmbedData, the only difference is that
      the @a fileName is a wide string.
   */

   int GetEmbeddedFileCount(void);
   /**<
      Gets count of embedded files stored in a PDF document.

      @return Count of found embedded files, or -1 on error.

      @see EmbedFile, EmbedData, GetEmbeddedFileName, GetEmbeddedFileData
   */

   std::string GetEmbeddedFileName(unsigned int index);
   /**<
      Gets embedded file's name, as stored in a PDF document.

      @param index Index of the embedded file; returns failure, if out of range.
      @return File's name, as stored in a PDF document.

      @see GetEmbeddedFileNameW, EmbedFile, EmbedData, GetEmbeddedFileCount, GetEmbeddedFileData
   */

   std::wstring GetEmbeddedFileNameW(unsigned int index);
   /**<
      This is the same as @ref GetEmbeddedFileName, the only difference is that
      the return fileName is a wide string.
   */

   bool GetEmbeddedFileData(unsigned int index,
                            BYTE *data,
                            unsigned int *dataLength);
   /**<
      Gets embedded file's data, as stored in a PDF document. There are no data returned,
      if the file was not embed.

      @param index Index of the embedded file; returns failure, if out of range.
      @param data [out] Actual embedded file's data, as stored in the PDF. It can be NULL, in which case
         the @a dataLength is populated with large-enough value to hold the whole data.
      @param dataLength [in/out] Tells how many bytes can be stored in @a data. If @a data
         is NULL, then it is set to large-enough value. Passing non-NULL @a data with no enough
         large buffer results in a failure with no change on @a dataLength.
      @return Whether succeeded.

      @see EmbedFile, EmbedData, GetEmbeddedFileCount, GetEmbeddedFileName
   */

   void *GetPoDoFoDocument(void);
   /**<
      Gets a pointer to PoDoFo::PdfDocument document, which is currently opened.
      The returned pointer is owned by litePDF, do not free it. It is valid until
      the document is closed.

      @return Pointer to currently opened PoDoFo::PdfDocument.

      @see Close
   */

   void DrawDebugPage(const char *filename);
   /**<
      Draws saved debugPage as a new page into the PDF file. There should not be
      running any drawing when calling this function (like no page can be opened
      for drawing).

      @param filename File name with full path for litePDF debug page.
   */

   void CreateLinkAnnotation(unsigned int annotationPageIndex,
                             int annotationX_u,
                             int annotationY_u,
                             int annotationWidth_u,
                             int annotationHeight_u,
                             unsigned int annotationFlags,
                             unsigned int annotationResourceID,
                             unsigned int destinationPageIndex,
                             unsigned int destinationX_u,
                             unsigned int destinationY_u,
                             const wchar_t *destinationDescription);
   /**<
      Creates a link annotation at the given page and position, which will target the given
      destination page and the position in it. The object should hold a memory-based document.
      Note, the link annotation can be created only when the document is not drawing, to
      have all the document pages available.

      @param annotationPageIndex Page index where to place the link annotation.
      @param annotationX_u X-origin of the annotation on the page, in the current unit.
      @param annotationY_u Y-origin of the annotation on the page, in the current unit.
      @param annotationWidth_u Width of the annotation on the page, in the current unit.
      @param annotationHeight_u Height of the annotation on the page, in the current unit.
      @param annotationFlags Bit-or of @ref TLitePDFAnnotationFlags flags.
      @param annotationResourceID Optional resource ID of the annotation content, as shown
         to the user. 0 means do not add additional visualization on the page, but the annotation
         can be still clicked.
      @param destinationPageIndex Page index where the link points to.
      @param destinationX_u X-origin of the destination on the page, in the current unit.
      @param destinationY_u Y-origin of the destination on the page, in the current unit.
      @param destinationDescription Optional destination description, which can be used
         for accessibility reasons by the viewer.

      @see GetUnit, GetPageCount, AddResource, CreateBookmarkRoot
   */

   unsigned int CreateBookmarkRoot(const wchar_t *title,
                                   unsigned int flags,
                                   COLORREF titleColor,
                                   unsigned int destinationPageIndex,
                                   unsigned int destinationX_u,
                                   unsigned int destinationY_u);
   /**<
      Creates a new root (top-level) bookmark, which will target the given destination
      page and the position in it. The object should hold a memory-based document.
      Note, the bookmarks can be created only when the document is not drawing, to
      have all the document pages available.

      @param title Title of the bookmark.
      @param flags Bit-or of @ref TLitePDFBookmarkFlags flags.
      @param titleColor RGB value of the title text color.
      @param destinationPageIndex Page index where the link points to.
      @param destinationX_u X-origin of the destination on the page, in the current unit.
      @param destinationY_u Y-origin of the destination on the page, in the current unit.
      @return Created bookmark ID or 0, when the bookmark could not be created.

      @see GetUnit, CreateBookmarkChild, CreateBookmarkSibling, CreateLinkAnnotation
   */

   unsigned int CreateBookmarkChild(unsigned int parentBookmarkID,
                                    const wchar_t *title,
                                    unsigned int flags,
                                    COLORREF titleColor,
                                    unsigned int destinationPageIndex,
                                    unsigned int destinationX_u,
                                    unsigned int destinationY_u);
   /**<
      Creates a new child bookmark, which will target the given destination
      page and the position in it. The object should hold a memory-based document.
      Note, the bookmarks can be created only when the document is not drawing, to
      have all the document pages available.

      @param parentBookmarkID Bookmark ID of the parent bookmark. The child will be
         created under this bookmark.
      @param title Title of the bookmark.
      @param flags Bit-or of @ref TLitePDFBookmarkFlags flags.
      @param titleColor RGB value of the title text color.
      @param destinationPageIndex Page index where the link points to.
      @param destinationX_u X-origin of the destination on the page, in the current unit.
      @param destinationY_u Y-origin of the destination on the page, in the current unit.
      @return Created bookmark ID or 0, when the bookmark could not be created.

      @see GetUnit, CreateBookmarkRoot, CreateBookmarkSibling, CreateLinkAnnotation
   */

   unsigned int CreateBookmarkSibling(unsigned int previousBookmarkID,
                                      const wchar_t *title,
                                      unsigned int flags,
                                      COLORREF titleColor,
                                      unsigned int destinationPageIndex,
                                      unsigned int destinationX_u,
                                      unsigned int destinationY_u);
   /**<
      Creates a new sibling (next) bookmark, which will target the given destination
      page and the position in it. The object should hold a memory-based document.
      Note, the bookmarks can be created only when the document is not drawing, to
      have all the document pages available.

      @param previousBookmarkID Bookmark ID of the previous bookmark. The sibling will be
         created as the next of this bookmark.
      @param title Title of the bookmark.
      @param flags Bit-or of @ref TLitePDFBookmarkFlags flags.
      @param titleColor RGB value of the title text color.
      @param destinationPageIndex Page index where the link points to.
      @param destinationX_u X-origin of the destination on the page, in the current unit.
      @param destinationY_u Y-origin of the destination on the page, in the current unit.
      @return Created bookmark ID or 0, when the bookmark could not be created.

      @see CreateBookmarkRoot, CreateBookmarkChild, CreateLinkAnnotation
   */
};

}; // namespace litePDF

using namespace litePDF;

#endif // litePDFH
