unit litePDF;

{*
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
 * }

interface

uses Winapi.Windows, System.Classes, System.DateUtils, System.SysUtils, Vcl.Graphics;

const
   LitePDF_API_Major : Integer = 2; {**< LitePDF API version Major part; @see LitePDF_API_Minor }
   LitePDF_API_Minor : Integer = 0; {**< LitePDF API version Minor part; @see LitePDF_API_Major }

   LitePDFDocumentInfo_Author : PAnsiChar           = 'Author';       {**< an Author of the document *}
   LitePDFDocumentInfo_Creator : PAnsiChar          = 'Creator';      {**< a Creator of the document }
   LitePDFDocumentInfo_Keywords : PAnsiChar         = 'Keywords';     {**< the Keywords of the document }
   LitePDFDocumentInfo_Subject : PAnsiChar          = 'Subject';      {**< a Subject of the document }
   LitePDFDocumentInfo_Title : PAnsiChar            = 'Title';        {**< a Title of the document }
   LitePDFDocumentInfo_Producer : PAnsiChar         = 'Producer';     {**< a Producer of the document; this key is read-only }
   LitePDFDocumentInfo_Trapped : PAnsiChar          = 'Trapped';      {**< a trapping state of the document }
   LitePDFDocumentInfo_CreationDate : PAnsiChar     = 'CreationDate'; {**< a date of the creation of the document }
   LitePDFDocumentInfo_ModificationDate : PAnsiChar = 'ModDate';      {**< a date of the last modification of the document }

type
   TLitePDFErrorEvent = procedure(code : LongWord;
                                  const msg : PAnsiChar;
                                  user_data : Pointer); stdcall;
   TLitePDFEvalFontFlagCB = function(inout_faceName : PAnsiChar;
                                      faceNameBufferSize : LongWord;
                                      user_data : Pointer) : LongWord; stdcall;
   TLitePDFAppendSignatureDataFunc = procedure (bytes : PByte;
                                                bytes_len : LongWord;
                                                user_data : Pointer); stdcall;
   TLitePDFFinishSignatureFunc = procedure (signature : PByte;
                                            signature_len : PLongWord;
                                            user_data : Pointer); stdcall;
//---------------------------------------------------------------------------

   TLitePDFUnit = (
                  LitePDFUnit_Unknown     = 0, {**< Unknown unit; usually used to indicate an error *}
                  LitePDFUnit_mm          = 1, {**< Millimeters unit *}
                  LitePDFUnit_10th_mm     = 2, {**< 1/10th of a millimeter unit; 5 mm is value 50 *}
                  LitePDFUnit_100th_mm    = 3, {**< 1/100th of a millimeter unit; 5 mm is value 500 *}
                  LitePDFUnit_1000th_mm   = 4, {**< 1/1000th of a millimeter unit; 5 mm is value 5000 *}
                  LitePDFUnit_inch        = 5, {**< Inch unit *}
                  LitePDFUnit_10th_inch   = 6, {**< 1/10th of an inch unit; 5" is value 50 *}
                  LitePDFUnit_100th_inch  = 7, {**< 1/100th of an inch unit; 5" is value 500 *}
                  LitePDFUnit_1000th_inch = 8  {**< 1/1000th of an inch unit; 5" is value 5000 *}
               );
//---------------------------------------------------------------------------

   TLitePDFFontFlags = (
                  LitePDFFontFlag_Default       = 0, {**< Use the settings as specified by the draw operation *}
                  LitePDFFontFlag_DoNotEmbed    = 1, {**< Do not embed the font *}
                  LitePDFFontFlag_EmbedComplete = 2, {**< Embed complete font *}
                  LitePDFFontFlag_EmbedSubset   = 3, {**< Embed the font with used characters only *}
                  LitePDFFontFlag_Substitute    = 4  {**< Substitute the font with one of the base fonts, if possible *}
               );
//---------------------------------------------------------------------------

   TLitePDFDrawFlags = (
                  LitePDFDrawFlag_None                   = 0,  {**< None draw flags }
                  LitePDFDrawFlag_EmbedFontsNone         = 16, {**< Do not embed any fonts into resulting PDF.
                       @note: Fonts' subset is embeded by default, if none of the @ref LitePDFDrawFlag_EmbedFontsNone, @ref LitePDFDrawFlag_EmbedFontsComplete, @ref LitePDFDrawFlag_EmbedFontsSubset,
                       flags is defined; the @ref LitePDFDrawFlag_EmbedFontsNone is to override default font embedding. The reason for this default embedding is
                       due to PDF readers not showing correct fonts when they are not part of the PDF file.
                       @see LitePDFDrawFlag_EmbedFontsComplete, LitePDFDrawFlag_EmbedFontsSubset; }
                  LitePDFDrawFlag_EmbedFontsComplete     = 1, {**< Embed complete fonts into resulting PDF; @see LitePDFDrawFlag_EmbedFontsSubset, LitePDFDrawFlag_EmbedFontsNone }
                  LitePDFDrawFlag_EmbedFontsSubset       = 2, {**< Embed only subset of the fonts, aka used letters; this flag is used before @ref LitePDFDrawFlag_EmbedFontsComplete; @see LitePDFDrawFlag_EmbedFontsNone }
                  LitePDFDrawFlag_SubstituteFonts        = 4, {**< Substitute fonts with base PDF fonts, if possible }
                  LitePDFDrawFlag_CompressImagesWithJPEG = 8  {**< Compress images with JPEG compression, to get smaller PDF document; this is used only for RGB images }
               );
//---------------------------------------------------------------------------

   TLitePDFEncryptPermission = (
                  LitePDFEncryptPermission_None        = $0,        {**< Nothing from the rest is allowed }
                  LitePDFEncryptPermission_Print       = $00000004, {**< Allow printing the document }
                  LitePDFEncryptPermission_Edit        = $00000008, {**< Allow modifying the document besides annotations, form fields or changing pages }
                  LitePDFEncryptPermission_Copy        = $00000010, {**< Allow text and graphic extraction }
                  LitePDFEncryptPermission_EditNotes   = $00000020, {**< Add or modify text annotations or form fields (if ePdfPermissions_Edit is set also allow to create interactive form fields including signature) }
                  LitePDFEncryptPermission_FillAndSign = $00000100, {**< Fill in existing form or signature fields }
                  LitePDFEncryptPermission_Accessible  = $00000200, {**< Extract text and graphics to support user with disabilities }
                  LitePDFEncryptPermission_DocAssembly = $00000400, {**< Assemble the document: insert, create, rotate delete pages or add bookmarks }
                  LitePDFEncryptPermission_HighPrint   = $00000800, {**< Print a high resolution version of the document }
                  LitePDFEncryptPermission_All         = $00000F3C  {**< Shortcut for all permissions }
               );
//---------------------------------------------------------------------------

   TLitePDFEncryptAlgorithm = (
                  LitePDFEncryptAlgorithm_None  = 0, {**< No encryption algorithm; it can be used only when unsetting prepared encryption }
                  LitePDFEncryptAlgorithm_RC4V1 = 1, {**< RC4 Version 1 encryption using a 40bit key }
                  LitePDFEncryptAlgorithm_RC4V2 = 2, {**< RC4 Version 2 encryption using a 128bit key }
                  LitePDFEncryptAlgorithm_AESV2 = 4, {**< AES encryption with a 128 bit key (PDF1.6) }
                  LitePDFEncryptAlgorithm_AESV3 = 8  {**< AES encryption with a 256 bit key (PDF1.7 extension 3) }
               );
//---------------------------------------------------------------------------

   TLitePDFAnnotationFlags = (
                  LitePDFAnnotationFlag_None           = $0000, {**< Default annotation flags *}
                  LitePDFAnnotationFlag_Invisible      = $0001, {**< Do not show nonstandard annotation if there is no annotation handler available *}
                  LitePDFAnnotationFlag_Hidden         = $0002, {**< Do not allow show, print or interact with the annotation *}
                  LitePDFAnnotationFlag_Print          = $0004, {**< Print the annotation *}
                  LitePDFAnnotationFlag_NoZoom         = $0008, {**< Do not scale the annotation's appearance to match the magnification of the page *}
                  LitePDFAnnotationFlag_NoRotate       = $0010, {**< Do not rotate the annotation's appearance to match the rotation of the page *}
                  LitePDFAnnotationFlag_NoView         = $0020, {**< Do not display the annotation on the screen or allow it to interact with the user *}
                  LitePDFAnnotationFlag_Readonly       = $0040, {**< Do not allow the annotation to interact with the user *}
                  LitePDFAnnotationFlag_Locked         = $0080, {**< Do not allow the annotation to be deleted or its properties (including position and size) to be modified by the user *}
                  LitePDFAnnotationFlag_ToggleNoView   = $0100, {**< Invert the interpretation of the NoView flag for certain events *}
                  LitePDFAnnotationFlag_LockedContents = $0200  {**< Do not allow the contents of the annotation to be modified by the user *}
               );
//---------------------------------------------------------------------------

   TLitePDFAppearance = (
                  LitePDFAppearance_Normal             = $0000, {**< Normal appearance *}
                  LitePDFAppearance_Rollover           = $0001, {**< Rollover appearance; the default is the normal appearance *}
                  LitePDFAppearance_Down               = $0002  {**< Down appearance; the default is the normal appearance *}
   );

//---------------------------------------------------------------------------

   TLitePDFBookmarkFlags = (
                  LitePDFBookmarkFlag_None           = $0000, {**< Default bookmark flags *}
                  LitePDFBookmarkFlag_Italic         = $0001, {**< Show bookmark title as an italic text *}
                  LitePDFBookmarkFlag_Bold           = $0002  {**< Show bookmark title as a bold text *}
   );

//---------------------------------------------------------------------------

   TLitePDFException = class(Exception)
   private
      code : DWORD;
      msg : AnsiString;
   public
      constructor Create(pCode : DWORD;
                         const pMsg : AnsiString) overload;
      constructor Create(const src: TLitePDFException) overload;

      function getCode : DWORD;
      {**<
         @return Error code.
      }

      function getMessage : AnsiString;
      {**<
         @return Error message.
      }
   end;
//---------------------------------------------------------------------------

   TLitePDF = class(TObject)
   private
      lib : THandle;
      context : Pointer;
      onError : TLitePDFErrorEvent;
      onErrorUserData : Pointer;
      lastErrorCode : DWORD;
      lastErrorMessage : AnsiString;
      onEvalFontFlag : TLitePDFEvalFontFlagCB;
      onEvalFontFlagUserData : Pointer;

      function GetProc(const pProcIdent : PAnsiChar) : FARPROC;
      function checkAPIVersion(major : LongWord;
                               minor : LongWord) : Boolean;
      {*< returns whether DLL's version is the correct API version, as expected by this class }

      procedure ensureLibraryLoaded(const _func : PAnsiChar);
      procedure unloadLibrary;
      procedure freeLastError;
      procedure setLastError(code : DWORD;
                             const msg : PAnsiChar);

   public
      constructor Create;
      destructor Destroy; override;

      procedure setOnError(pOnError: TLitePDFErrorEvent;
                           pOnErrorUserData : Pointer);
      {**<
         Sets a custom callback for errors notified by the litePDF library. It's not
         necessary to be set. The errors are those returned during any function calls,
         but also during drawing, for example when some draw operation is not supported.
         Most of the object calls usually throw a @ref LitePDFException on errors.

         @param pOnError A callback to call.
         @param pOnErrorUserData user data for the callback.

         @see getLastErrorCode, getLastErrorMessage
      }

      function getLastErrorCode: LongWord;
      {**<
         Returns the last error code, which was notified by the litePDF library, if any.
         Most of the object calls usually throw a @ref LitePDFException on errors.

         @return The last error code, or 0, if there was none.

         @see setOnError, getLastErrorMessage
      }

      function getLastErrorMessage : AnsiString;
      {**<
         Returns the last error message, which was notified by the litePDF library, if any.
         Most of the object calls usually throw a @ref LitePDFException on errors.

         @return The last error message, or NULL, if there was none.

         @see setOnError, getLastErrorCode
      }

      procedure SetUnit(unitValue : TLitePDFUnit);
      {**<
         Sets a unit to use in functions which expect non-pixel size and position values.
         It can be one of TLitePDFUnit values. The default is @ref LitePDFUnit_mm.

         @param unitValue One of TLitePDFUnit values, to set as a unit.

         @see GetUnit, MMToUnit, UnitToMM, InchToUnit, UnitToInch
      }

      function GetUnit : TLitePDFUnit;
      {**<
         Gets the currently set unit, which is used in functions which expect
         non-pixel size and position values. It can be one of TLitePDFUnit values.
         The default is @ref LitePDFUnit_mm.

         @return One of TLitePDFUnit values, which is set as the current unit.

         @see SetUnit, MMToUnit, UnitToMM, InchToUnit, UnitToInch
      }

      function MMToUnitEx(useUnit : TLitePDFUnit;
                          mmValue : Double) : Double;
      {**<
         Converts a value from millimeters to @a useUnit. The caller does
         the rounding as needed.

         @param useUnit The @ref TLitePDFUnit unit to convert the value to.
         @param mmValue The value in millimeters to convert.
         @return The @a mmValue converted to @a useUnit unit.

         @see UnitToMMEx, InchToUnitEx, UnitToInchEx, MMToUnit
      *}

      function UnitToMMEx(useUnit : TLitePDFUnit;
                          unitValue : Double) : Double;
      {**<
         Converts a value from @a useUnit to millimeters. The caller does
         the rounding as needed.

         @param useUnit The @ref TLitePDFUnit unit to convert the value from.
         @param unitValue The value in @a useUnit to convert.
         @return The @a unitValue in @a useUnit converted to millimeters.

         @see MMToUnitEx, InchToUnitEx, UnitToInchEx, UnitToMM
      *}

      function InchToUnitEx(useUnit : TLitePDFUnit;
                            inchValue : Double) : Double;
      {**<
         Converts a value from inches to @a useUnit. The caller does
         the rounding as needed.

         @param useUnit The @ref TLitePDFUnit unit to convert the value to.
         @param inchValue The value in inches to convert.
         @return The @a inchValue converted to @a useUnit unit.

         @see UnitToInchEx, MMToUnitEx, UnitToMMEx, InchToUnit
      *}

      function UnitToInchEx(useUnit : TLitePDFUnit;
                            unitValue : Double) : Double;
      {**<
         Converts a value from @a useUnit to inches. The caller does
         the rounding as needed.

         @param useUnit The @ref TLitePDFUnit unit to convert the value from.
         @param unitValue The value in @a useUnit to convert.
         @return The @a unitValue in @a useUnit converted to inches.

         @see InchToUnitEx, MMToUnitEx, UnitToMMEx, UnitToInch
      *}

      function MMToUnit(mmValue : Double) : Double;
      {**<
         Converts a value from millimeters to current unit. The caller does
         the rounding as needed.

         @param mmValue A value in millimeters to convert to the current unit.
         @returns The @a mmValue converted to the current unit.

         @see GetUnit, UnitToMM, InchToUnit, UnitToInch, MMToUnitEx
      *}

      function UnitToMM(unitValue : Double) : Double;
      {**<
         Converts a value from the current unit to millimeters. The caller does
         the rounding as needed.

         @param unitValue A value in the current unit to convert to millimeters.
         @returns The @a unitValue converted to millimeters.

         @see GetUnit, MMToUnit, InchToUnit, UnitToInch, UnitToMMEx
      *}

      function InchToUnit(inchValue : Double) : Double;
      {**<
         Converts a value from inches to the current unit. The caller does
         the rounding as needed.

         @param inchValue A value in inches to convert to the current unit.
         @returns The @a inchValue converted to the current unit.

         @see GetUnit, UnitToInch, MMToUnit, UnitToMM, InchToUnitEx
      *}

      function UnitToInch(unitValue : Double) : Double;
      {**<
         Converts a value from the current unit to inches. The caller does
         the rounding as needed.

         @param unitValue A value in the current unit to convert to inches.
         @returns The @a unitValue converted to inches.

         @see GetUnit, InchToUnit, MMToUnit, UnitToMM, UnitToInchEx
      *}

      procedure SetEvalFontFlagCallback(callback : TLitePDFEvalFontFlagCB;
                                        userData : Pointer);
      {**<
         Sets a callback to evaluate what to do with a font. The @a callback can
         be NULL, to unset any previously set value. See @ref TLitePDFEvalFontFlagCB
         for more information about the @a callback parameters and what it can do.

         @param callback A @ref TLitePDFEvalFontFlagCB callback to set, or NULL.
         @param userData A user data to pass to @a callback when called.
      *}

      procedure PrepareEncryption(userPassword : AnsiString;
                                  ownerPassword : AnsiString;
                                  permissions : LongWord;
                                  algorithm : LongWord);
      {**<
         Prepares encryption for newly created documents. The LitePDF
         object should be empty. The encryption is used only with
         @ref CreateFileDocument and @ref CreateMemDocument, other
         functions ignore it. Use an empty @a ownerPassword to unset
         any previously set encryption properties.
         Loading an encrypted document lefts it encrypted on save too.

         @param userPassword User's password, can be an empty string,
            then the user doesn't need to write any password.
         @param ownerPassword Owner's password. Can be an emptry string, to unset encryption properties.
         @param permissions Bit-or of LitePDFEncryptPermission flags, to set user's
            permissions for the document.
         @param algorithm One of LitePDFEncryptAlgorithm constants, an algorithm
            to be used to encrypt the document.

         @see CreateFileDocument, CreateMemDocument
      }

      procedure CreateFileDocument(const fileName : AnsiString);
      {**<
         Makes the LitePDF object hold a new PDF, which writes directly to a file.
         The object should not have opened any other PDF data. Call @ref Close,
         to close the file, and possibly save changes to it. Most of the operations
         require memory-based PDF, which can be created with @ref CreateMemDocument.

         @param fileName File name to write the PDF result to.

         @note PoDoFo doesn't support creation of file-based documents with AES encryption,
            thus use for it memory-based documents instead (@ref CreateMemDocument).

         @see PrepareEncryption, CreateFileDocumentW, CreateMemDocument, LoadFromFile
      }

      procedure CreateFileDocumentW(const fileName : WideString);
      {**<
         This is the same as @ref CreateFileDocument, the only difference is that
         the @a fileName is a wide string.
      }

      procedure CreateMemDocument;
      {**<
         Makes the litePDF object hold a memory-based PDF. Such PDF can be
         saved with @ref SaveToFile or  @ref SaveToData.
         The PDF should be closed with @ref Close.

         @see PrepareEncryption, CreateFileDocument, LoadFromFile
      }

      procedure LoadFromFile(const fileName : AnsiString;
                             const password : AnsiString;
                             loadCompletely : Boolean;
                             forUpdate : Boolean = False);
      {**<
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
         @param forUpdate Set to True to open the file for incremental update,
            or set to False otherwise.

         @see LoadFromFileW
      }

      procedure LoadFromFileW(const fileName : WideString;
                              const password : AnsiString;
                              loadCompletely : Boolean;
                              forUpdate : Boolean = False);
      {**<
         This is the same as @ref LoadFromFile, the only difference is that
         the @a fileName is a wide string.
      *}

      procedure LoadFromData(data : PByte;
                             dataLength : LongWord;
                             const password : AnsiString;
                             forUpdate : Boolean = False);
      {**<
         Makes the LitePDF object hold a memory-based PDF, which is loaded
         with a PDF data. This should be closed with @ref Close.
         The @a forUpdate parameter specifies whether the file is being opened
         for incremental update. In that case saving such document will result
         in the changes being appended to the end of the original document,
         instead of resaving whole document.

         @param data PDF data to load.
         @param dataLength Length of PDF data.
         @param password Password to use for encrypted documents.
         @param forUpdate Set to True to open the file for incremental update,
            or set to False otherwise.

         @see CreateMemDocument, SaveToFile, SaveToData
      }

      procedure SaveToFile(const fileName : AnsiString);
      {**<
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
      }

      procedure SaveToFileW(const fileName : WideString);
      {**<
         This is the same as @ref SaveToFile, the only difference is that
         the @a fileName is a wide string.
      }

      function SaveToData(data : PByte;
                          var dataLength : LongWord) : Boolean;
      {**<
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
      }

      procedure Close;
      {**<
         Closes PDF data in a LitePDF object, thus the object doesn't hold anything afterward,
         aka it's like a newly created object.  The function does nothing, if the object doesn't
         hold any data. In case of any drawing in progress, the drawing is discarded, same as any
         unsaved changes to the memory-based PDF documents. It also unsets any encryption properties,
         previously set by @ref PrepareEncryption.

         @see AddPage, InsertPage, UpdatePage, FinishPage
      }

      function GetPageCount : LongWord;
      {**<
         Returns count of pages in an opened PDF document.

         @return Count of pages.
      }

      procedure GetPageSize(pageIndex : LongWord;
                            var width_u : LongWord;
                            var height_u : LongWord);
      {**<
         Gets size of an existing page, in the current unit.

         @param pageIndex Page index for which get the page size; counts from 0.
         @param width_u [out] Width of the page in the current unit.
         @param height_u [out] Height of the page in the current unit.

         @see GetUnit
      }

      function GetPageRotation(pageIndex : LongWord) : Integer;
      {**<
         Gets rotation of an existing page, in degrees. Expected values are 0, 90, 180 and 270.

         @param pageIndex Page index for which get the page size; counts from 0.
         @return Rotation of the page, in degrees.

         @see SetPageRotation
      }

      procedure SetPageRotation(pageIndex : LongWord;
                                degrees : Integer);
      {**<
         Sets rotation of an existing page, in degrees. Expected values are 0, 90, 180 and 270.

         @param pageIndex Page index for which get the page size; counts from 0.
         @param degrees Rotation of the page to set, in degrees.

         @see GetPageRotation
      }

      function AddPage(width_u : LongWord;
                       height_u : LongWord;
                       width_px : LongWord;
                       height_px : LongWord;
                       drawFlags : LongWord) : HDC;
      {**<
         Begins drawing into a new page into the PDF document of the given size.
         Newly created page is added as the last page of the PDF document.
         This cannot be called when other drawing is in progress.

         @param width_u Width of the new page in the current unit.
         @param height_u Height of the new page in the current unit.
         @param width_px Width of the new page in pixels.
         @param height_px Height of the new page in pixels.
         @param drawFlags Flags for drawing functions. This is a bit-or of LitePDFDrawFlags values
            and influences only @ref AddPage, @ref InsertPage, @ref UpdatePage
            and @ref AddResource functions.
         @return Device context into which can be drawn with standard GDI functions.
            Finish the drawing, and the page addition, with @ref FinishPage.

         @note Larger pixel page size produces more precise font mapping.

         @see GetUnit, InsertPage, UpdatePage, FinishPage, AddResource
      }

      function InsertPage(pageIndex : LongWord;
                          width_u : LongWord;
                          height_u : LongWord;
                          width_px : LongWord;
                          height_px : LongWord;
                          drawFlags : LongWord) : HDC;
      {**<
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
         @param drawFlags Flags for drawing functions. This is a bit-or of LitePDFDrawFlags values
            and influences only @ref AddPage, @ref InsertPage, @ref UpdatePage
            and @ref AddResource functions.
         @return Device context into which can be drawn with standard GDI functions.
            Finish the drawing, and the page insertion, with @ref FinishPage.

         @note Larger pixel page size produces more precise font mapping.

         @see GetUnit, GetPageCount, AddPage, UpdatePage, FinishPage, AddResource
      }

      function UpdatePage(pageIndex : LongWord;
                          width_px : LongWord;
                          height_px : LongWord;
                          drawFlags : LongWord) : HDC;
      {**<
         Begins drawing into an already existing page. The page size in the current unit can
         be found by @ref GetPageSize. The function fails, and returns NULL,
         if the @a pageIndex is out of bounds.

         @param pageIndex Page index which to update; counts from 0.
         @param width_px Width of the new page in pixels.
         @param height_px Height of the new page in pixels.
         @param drawFlags Flags for drawing functions. This is a bit-or of LitePDFDrawFlags values
            and influences only @ref AddPage, @ref InsertPage, @ref UpdatePage
            and @ref AddResource functions.
         @return Device context into which can be drawn with standard GDI functions.
            Finish the drawing, and the page update, with @ref FinishPage.

         @see GetUnit, GetPageCount, AddPage, InsertPage, FinishPage, AddResource
      }

      procedure FinishPage(dc : HDC);
      {**<
         Tells litePDF that drawing into the page is finished and that it can
         be processed into PDF. The @a dc is not valid after this call any more.

         @param hDC Device context previously returned by @ref AddPage,
            @ref InsertPage or @ref UpdatePage.
      }

      function AddResource(width_u : LongWord;
                           height_u : LongWord;
                           width_px : LongWord;
                           height_px : LongWord;
                           drawFlags : LongWord) : HDC;
      {**<
         Begins drawing into a new resource into the PDF document of the given size.
         This cannot be called when other drawing is in progress.

         @param width_u Width of the new page in the current unit.
         @param height_u Height of the new page in the current unit.
         @param width_px Width of the new page in pixels.
         @param height_px Height of the new page in pixels.
         @param drawFlags Flags for drawing functions. This is a bit-or of LitePDFDrawFlags values
            and influences only @ref AddPage, @ref InsertPage, @ref UpdatePage
            and @ref AddResource functions.
         @return Device context into which can be drawn with standard GDI functions.
            Finish the drawing, and the resource addition, with @ref FinishResource.

         @note Larger pixel resource size produces more precise font mapping.

         @see GetUnit, AddPage, InsertPage, UpdatePage, FinishResource, DrawResource
      }

      function FinishResource(dc : HDC) : LongWord;
      {**<
         Tells litePDF that drawing into the resource is finished and that it can
         be processed into PDF. The @a dc is not valid after this call any more.

         @param hDC Device context previously returned by @ref AddResource.
         @return Newly created resource ID, or 0 on error.

         @see AddResource, AddPageFromAsResource, DrawResource
      }

      procedure DeletePage(pageIndex : LongWord);
      {**<
         Deletes page at given index. It doesn't delete page resources, because these can
         be used by other pages.

         @param pageIndex Page index which to update; counts from 0.
         @return Whether succeeded.

         @see GetPageCount, PageToResource
      }

      procedure AddPagesFrom(from : TLitePDF;
                             pageIndex : LongWord;
                             pageCount : LongWord);
      {**<
         Adds existing pages as the last pages from another PDF. Both objects should
         hold memory-based documents.

         @param from a LitePDF object from which add the pages.
         @param pageIndex Page index which to add from @a from; counts from 0.
         @param pageCount How many pages to add; 0 means whole document.

         @note The two objects cannot be the same.

         @see GetPageCount, InsertPageFrom, PageToResource
      }

      procedure InsertPageFrom(pageIndexTo : LongWord;
                               from : TLitePDF;
                               pageIndexFrom : LongWord);
      {**<
         Inserts an existing page at the given index from another PDF. Both objects should
         hold memory-based documents.

         @param pageIndexTo Page index where to add the page; counts from 0. Adds page
            at the end, if out of bounds.
         @param from a LitePDF object, from which add the page.
         @param pageIndexFrom Page index which to add from @a from; counts from 0.

         @note The two objects cannot be the same.

         @see GetPageCount, AddPagesFrom, PageToResource
      }

      function AddPageFromAsResource(from : TLitePDF;
                                     pageIndex : LongWord;
                                     useTrimBox : Boolean = False) : LongWord;
      {**<
         Adds an existing page as a resource of a given PDF. This resource can be
         referenced multiple times by its identifier. Both objects should
         hold memory-based documents.

         @param from a LitePDF object, from which add the page.
         @param pageIndex Page index which to add from @a from; counts from 0.
         @param useTrimBox If true, try to use trimbox for size of the resource (XObject)
         @return Resource identifier, or 0 on error.

         @note The two objects cannot be the same.

         @see GetPageCount, AddPagesFrom, PageToResource, GetResourceSize, DrawResource
      }

      function PageToResource(pageIndex : LongWord) : LongWord;
      {**<
         Creates a resource, which will reference an existing page.
         The page itself is not deleted after call of this.

         @param pageIndex Page index for which create the resource reference; counts from 0.
         @return Resource identifier, or 0 on error.

         @see GetPageCount, AddPagesFrom, AddPageFromAsResource, GetResourceSize,
            DrawResource
      }

      procedure GetResourceSize(resourceID : LongWord;
                                var width_u : LongWord;
                                var height_u : LongWord);
      {**<
         Gets size of an existing resource, in the current unit. The resource ID
         was returned from @ref AddPageFromAsResource or @ref FinishResource.

         @param resourceID Resource ID for which get the size.
         @param width_u [out] Width of the resource, in the current unit.
         @param height_u [out] Height of the resource, in the current unit.

         @see GetUnit, AddPageFromAsResource, DrawResource
      }

      procedure DrawResource(resourceID : LongWord;
                             pageIndex : LongWord;
                             unitValue : TLitePDFUnit;
                             x : Integer;
                             y : Integer;
                             scaleX : Integer;
                             scaleY : Integer);
      {**<
         Draws an existing resource at given position. The resource ID
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
         @param scaleX Scale factor of the page, X axes, using the @a unitValue ratio.
         @param scaleY Scale factor of the page, Y axes, using the @a unitValue ratio.

         @see GetPageCount, AddPageFromAsResource, PageToResource, FinishResource,
            GetResourceSize, DrawResourceWithMatrix
      }

      procedure DrawResourceWithMatrix(resourceID : LongWord;
                                       pageIndex : LongWord;
                                       a : Double;
                                       b : Double;
                                       c : Double;
                                       d : Double;
                                       e : Double;
                                       f : Double);
      {**<
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
      }

      procedure SetDocumentInfo(const name : AnsiString;
                                const value : WideString);
      {**<
         Sets information about the document. The name can be one
         of the LitePDFDocumentInfo predefined constants.

         @param name Document info property name to set.
         @param value Null-terminated Unicode value to set.
      }

      function GetDocumentInfoExists(const name : AnsiString) : Boolean;
      {**<
         Checks whether information about the document of the given name exists.
         The name can be one of the LitePDFDocumentInfo predefined constants.

         @param name Document info property name to test.
         @return Whether succeeded and the document information is set.
      }

      function GetDocumentInfo(const name : AnsiString) : WideString;
      {**<
         Gets information about the document. The name can be one
         of the LitePDFDocumentInfo predefined constants.

         @param name Document info property name to get.
         @return Unicode value.
      }

      function GetDocumentIsSigned : Boolean;
      {**<
         Checks whether currently opened document is already signed. Signing already
         signed document can cause breakage of previous signatures, thus it's good
         to test whether the loaded document is signed, before signing it.

         @return Whether the opened document is already signed.

         @see GetSignatureCount, SaveToFileWithSign, SaveToDataWithSign
      }

      function GetSignatureCount : LongWord;
      {**<
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
      }

      function GetSignatureName(index : LongWord) : AnsiString;
      {**<
         Gets the signature field name at the given @a index.

         @param index Which signature field name to get; counts from 0. This might be less
            than @ref GetSignatureCount.
         @return An ASCII name of the field.
      }

      function CreateSignature(name : AnsiString;
                               annotationPageIndex : LongWord;
                               annotationPosition_u : TRect;
                               annotationFlags : LongWord) : LongWord;
      {**<
         Creates a new signature field named @a name. The field is created completely empty.
         Use @ref SetSignatureDate, @ref SetSignatureReason,
         @ref SetSignatureLocation, @ref SetSignatureCreator,
         @ref SetSignatureAppearance and such to populate it with required values.
         Finally, to sign the signature field use @ref SaveToFileWithSign family
         functions.

         @param name Signature field name to use. This should be unique.
         @param annotationPageIndex Page index where to place the signature annotation.
         @param annotationPosition_u Where to place the annotation on the page, in the current unit.
         @param annotationFlags Bit-or of @ref TLitePDFAnnotationFlags flags.
         @return The index of the added signature field.

         @see GetSignatureCount, GetSignatureName
       }

      function GetSignatureHasData(index : LongWord) : Boolean;
      {**<
         Checks whether the given signature field contains any data, which
         means whether the signature field is signed.

         @param index Which signature data to get; counts from 0. This might be less
            than @ref GetSignatureCount.
         @return Whether the given signature contains any data.

         @see GetSignatureData
      }

      function GetSignatureData(index : LongWord;
                                data : PByte;
                                var dataLength : LongWord) : Boolean;
      {**<
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
      }

      function GetSignatureRanges(index : LongWord;
                                  pRangesArray : PUInt64;
                                  var pRangesArrayLength : LongWord) : Boolean;
      {**<
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
      }

      procedure SetSignatureDate(index : LongWord;
                                 const dateOfSign : TDateTime);
      {**<
         Sets signature field date of sign.

         @param index Which signature to use; counts from 0. This might be less
            than @ref GetSignatureCount.
         @param dateOfSign Date of sign, when the signature was created; less than
            or equal to 1.1.1970 means today. The value is converted into Unix time_t,
            which can be clamp on 32-bit systems.

         @see GetSignatureDate, GetSignatureCount
      }

      function GetSignatureDate(index : LongWord) : TDateTime;
      {**<
         Gets signature field date of sign.

         @param index Which signature to use; counts from 0. This might be less
            than @ref GetSignatureCount.
         @return The date of sign. It's like Unix time_t, as set by the signature field creator.
            The value can be clamp on 32-bit systems.

         @see SetSignatureDate, GetSignatureCount
      }

      procedure SetSignatureReason(index : LongWord;
                                   const reason : WideString);
      {**<
         Sets signature reason.

         @param index Which signature to use; counts from 0. This might be less
            than @ref GetSignatureCount.
         @param reason The value to set.

         @see GetSignatureReason, GetSignatureCount
      *}

      function GetSignatureReason(index : LongWord) : WideString;
      {**<
         Gets signature reason.

         @param index Which signature to use; counts from 0. This might be less
            than @ref GetSignatureCount.
         @return A Unicode string containing the value.

         @see SetSignatureReason, GetSignatureCount
      }

      procedure SetSignatureLocation(index : LongWord;
                                     const location : WideString);
      {**<
         Sets signature location, aka where the signature had been made. This can be left unset.

         @param index Which signature to use; counts from 0. This might be less
            than @ref GetSignatureCount.
         @param location The value to set.

         @see GetSignatureLocation, GetSignatureCount
      }

      function GetSignatureLocation(index : LongWord) : WideString;
      {**<
         Gets signature location.

         @param index Which signature to use; counts from 0. This might be less
            than @ref GetSignatureCount.
         @return A Unicode string containing the value.

         @see SetSignatureLocation, GetSignatureCount
      }

      procedure SetSignatureCreator(index : LongWord;
                                    const creator : AnsiString);
      {**<
         Sets signature creator. This can be left unset.

         @param index Which signature to use; counts from 0. This might be less
            than @ref GetSignatureCount.
         @param creator The value to set.

         @see GetSignatureCreator, GetSignatureCount
      }

      function GetSignatureCreator(index : LongWord) : AnsiString;
      {**<
         Gets signature creator.

         @param index Which signature to use; counts from 0. This might be less
            than @ref GetSignatureCount.
         @return An ASCII string containing the value.

         @see SetSignatureCreator, GetSignatureCount
      }

      procedure SetSignatureAppearance(index : LongWord;
                                       appearanceType : TLitePDFAppearance;
                                       resourceID : LongWord;
                                       offsetX_u : Integer;
                                       offsetY_u : Integer);
      {**<
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
      }

      procedure SetSignatureSize(requestBytes : LongWord);
      {**<
         Sets how many bytes the signature may require. The default value is 2048 bytes
         and it is automatically adjusted when the @ref SaveToFileWithSign or
         @ref SaveToDataWithSign are used. The manual signing functions
         require this value to be set before signing, if the final hash with the certificate
         exceeds the default size.

         This value is remembered in general, not for any signature in particular.

         @param requestBytes How many bytes the signature will require.

         @see SaveToFileWithSignManual, SaveToFileWithSignManualW, SaveToDataWithSignManual
      }

      procedure AddSignerPFX(pfxData : PByte;
                             pfxDataLength : LongWord;
                             const pfxPassword : AnsiString);
      {**<
         Adds a signer to be used when digitally signing the document with
         @ref SaveToFileWithSign or @ref SaveToDataWithSign.
         The passed-in certificate is in the PFX format and should include
         the private key.

         @param pfxData A certificate with private key in the PFX format.
         @param pfxDataLength A length of the @a pfxData.
         @param pfxPassword A password to use to open the PFX certificate; can be NULL.

         @see AddSignerPEM
      }

      procedure AddSignerPEM(pemData : PByte;
                             pemDataLength : LongWord;
                             pkeyData : PByte;
                             pkeyDataLength : LongWord;
                             const pkeyPassword : AnsiString);
      {**<
         Adds a signer to be used when digitally signing the document with
         @ref SaveToFileWithSign or @ref SaveToDataWithSign.
         The passed-in certificate and private key are in the PEM format.

         @param pemData A certificate in the PEM format.
         @param pemDataLength A length of the @a pemData.
         @param pkeyData A private key for the certificate, in the PEM format.
         @param pkeyDataLength A length of the @a pkeyData.
         @param pkeyPassword A password to use to open the private key; can be NULL.

         @see AddSignerPFX
      *}

      procedure SaveToFileWithSign(const fileName : AnsiString;
                                   signatureIndex : LongWord);
      {**<
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
      }

      procedure SaveToFileWithSignW(const fileName : WideString;
                                    signatureIndex : LongWord);
      {**<
         This is the same as @ref SaveToFileWithSign, the only difference is that
         the @a fileName is a wide string.
      }

      function SaveToDataWithSign(signatureIndex : LongWord;
                                  data : PByte;
                                  var dataLength : LongWord) : Boolean;
      {**<
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
      }

      procedure SaveToFileWithSignManual(const fileName : AnsiString;
                                         signatureIndex : LongWord;
                                         appendSignatureData : TLitePDFAppendSignatureDataFunc;
                                         append_user_data : Pointer;
                                         finishSignature : TLitePDFFinishSignatureFunc;
                                         finish_user_data : Pointer);
      {**<
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
      }

      procedure SaveToFileWithSignManualW(const fileName : WideString;
                                          signatureIndex : LongWord;
                                          appendSignatureData : TLitePDFAppendSignatureDataFunc;
                                          append_user_data : Pointer;
                                          finishSignature : TLitePDFFinishSignatureFunc;
                                          finish_user_data : Pointer);
      {**<
         This is the same as @ref SaveToFileWithSignManual, the only difference is that
         the @a fileName is a wide string.
      }

      function SaveToDataWithSignManual(signatureIndex : LongWord;
                                        appendSignatureData : TLitePDFAppendSignatureDataFunc;
                                        append_user_data : Pointer;
                                        finishSignature : TLitePDFFinishSignatureFunc;
                                        finish_user_data : Pointer;
                                        data : PByte;
                                        var dataLength : LongWord) : Boolean;
      {**<
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
      }

      procedure EmbedFile(const fileName : AnsiString);
      {**<
         Embeds a file into a PDF document.

         @param fileName File name of the file to be attached.
         @return Whether succeeded.

         @note Files can be embed only to memory-based documents.

         @note The path is stripped from the @a fileName. The @a fileName is used as a key,
            aka it's not possible to embed two files of the same name into a PDF document.

         @see EmbedFileW, EmbedData, CreateMemDocument
      }

      procedure EmbedFileW(const fileName : WideString);
      {**<
         This is the same as @ref EmbedFile, the only difference is that
         the @a fileName is a wide string.
      }

      procedure EmbedData(const fileName : AnsiString;
                          data : PByte;
                          dataLength : LongWord);
      {**<
         Embeds a data (file) into a PDF document.

         @param fileName File name to be used for the data identification.
         @param data Actual data to be attached.
         @param dataLength Length of the data.

         @note Data can be embed only to memory-based documents.

         @note The path is stripped from the @a fileName. The @a fileName is used as a key,
            aka it's not possible to embed two files of the same name into a PDF document.

         @see EmbedDataW, EmbedFile, CreateMemDocument
      }

      procedure EmbedDataW(const fileName : WideString;
                           data : PByte;
                           dataLength : LongWord);
      {**<
         This is the same as @ref EmbedData, the only difference is that
         the @a fileName is a wide string.
      }

      function GetEmbeddedFileCount : Integer;
      {**<
         Gets count of embedded files stored in a PDF document.

         @return Count of found embedded files, or -1 on error.

         @see EmbedFile, EmbedData, GetEmbeddedFileName, GetEmbeddedFileData
      }

      function GetEmbeddedFileName(index : LongWord) : AnsiString;
      {**<
         Gets embedded file's name, as stored in a PDF document.

         @param index Index of the embedded file; returns failure, if out of range.
         @return File's name, as stored in a PDF document.

         @see GetEmbeddedFileNameW, EmbedFile, EmbedData, GetEmbeddedFileCount, GetEmbeddedFileData
      }

      function GetEmbeddedFileNameW(index : LongWord) : WideString;
      {**<
         This is the same as @ref GetEmbeddedFileName, the only difference is that
         the @a fileName is a wide string.
      }

      function GetEmbeddedFileData(index : LongWord;
                                   data : PByte;
                                   var dataLength : LongWord) : Boolean;
      {**<
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
      }

      function GetPoDoFoDocument : Pointer;
      {**<
         Gets a pointer to PoDoFo::PdfDocument document, which is currently opened.
         The returned pointer is owned by litePDF, do not free it. It is valid until
         the document is closed.

         @return Pointer to currently opened PoDoFo::PdfDocument.

         @see Close
      }

      procedure DrawDebugPage(const filename : AnsiString);
      {**<
         Draws saved debugPage as a new page into the PDF file. There should not be
         running any drawing when calling this function (like no page can be opened
         for drawing).

         @param filename File name with full path for litePDF debug page.
      }

      procedure CreateLinkAnnotation(annotationPageIndex : LongWord;
                                     annotationPosition_u : TRect;
                                     annotationFlags : LongWord;
                                     annotationResourceID : LongWord;
                                     destinationPageIndex : LongWord;
                                     destinationX_u : LongWord;
                                     destinationY_u : LongWord;
                                     const destinationDescription : WideString);
      {**<
         Creates a link annotation at the given page and position, which will target the given
         destination page and the position in it. The object should hold a memory-based document.
         Note, the link annotation can be created only when the document is not drawing, to
         have all the document pages available.

         @param annotationPageIndex Page index where to place the link annotation.
         @param annotationPosition_u Where to place the annotation on the page, in the current unit.
         @param annotationFlags Bit-or of @ref TLitePDFAnnotationFlags flags.
         @param annotationResourceID Optional resource ID of the annotation content, as shown
            to a user. 0 means do not add additional visualization on the page, but the annotation
            can be still clicked.
         @param destinationPageIndex Page index where the link points to.
         @param destinationX_u X-origin of the destination on the page, in the current unit.
         @param destinationY_u Y-origin of the destination on the page, in the current unit.
         @param destinationDescription Optional destination description, which can be used
            for accessibility reasons by the viewer.

         @see GetPageCount, AddResource, CreateBookmarkRoot
      *}

      function CreateBookmarkRoot(const title : WideString;
                                  flags : LongWord;
                                  titleColor : TColor;
                                  destinationPageIndex : LongWord;
                                  destinationX_u : LongWord;
                                  destinationY_u : LongWord) : LongWord;
      {**<
         Creates a new root (top-level) bookmark, which will target the given destination
         page and the position in it. The object should hold a memory-based document.
         Note, the bookmarks can be created only when the document is not drawing, to
         have all the document pages available.

         @param title Title of the bookmark.
         @param flags Bit-or of @ref TLitePDFBookmarkFlags flags.
         @param titleColor Title text color.
         @param destinationPageIndex Page index where the link points to.
         @param destinationX_u X-origin of the destination on the page, in the current unit.
         @param destinationY_u Y-origin of the destination on the page, in the current unit.
         @return Created bookmark ID or 0, when the bookmark could not be created.

         @see CreateBookmarkChild, CreateBookmarkSibling, CreateLinkAnnotation
      *}

      function CreateBookmarkChild(parentBookmarkID : LongWord;
                                   const title : WideString;
                                   flags : LongWord;
                                   titleColor : TColor;
                                   destinationPageIndex : LongWord;
                                   destinationX_u : LongWord;
                                   destinationY_u : LongWord) : LongWord;
      {**<
         Creates a new child bookmark, which will target the given destination
         page and the position in it. The object should hold a memory-based document.
         Note, the bookmarks can be created only when the document is not drawing, to
         have all the document pages available.

         @param parentBookmarkID Bookmark ID of the parent bookmark. The child will be
            created under this bookmark.
         @param title Title of the bookmark.
         @param flags Bit-or of @ref TLitePDFBookmarkFlags flags.
         @param titleColor Title text color.
         @param destinationPageIndex Page index where the link points to.
         @param destinationX_u X-origin of the destination on the page, in the current unit.
         @param destinationY_u Y-origin of the destination on the page, in the current unit.
         @return Created bookmark ID or 0, when the bookmark could not be created.

         @see CreateBookmarkRoot, CreateBookmarkSibling, CreateLinkAnnotation
      *}

      function CreateBookmarkSibling(previousBookmarkID: LongWord;
                                     const title : WideString;
                                     flags : LongWord;
                                     titleColor : TColor;
                                     destinationPageIndex : LongWord;
                                     destinationX_u : LongWord;
                                     destinationY_u : LongWord) : LongWord;
      {**<
         Creates a new sibling (next) bookmark, which will target the given destination
         page and the position in it. The object should hold a memory-based document.
         Note, the bookmarks can be created only when the document is not drawing, to
         have all the document pages available.

         @param previousBookmarkID Bookmark ID of the previous bookmark. The sibling will be
            created as the next of this bookmark.
         @param title Title of the bookmark.
         @param flags Bit-or of @ref TLitePDFBookmarkFlags flags.
         @param titleColor Title text color.
         @param destinationPageIndex Page index where the link points to.
         @param destinationX_u X-origin of the destination on the page, in the current unit.
         @param destinationY_u Y-origin of the destination on the page, in the current unit.
         @return Created bookmark ID or 0, when the bookmark could not be created.

         @see CreateBookmarkRoot, CreateBookmarkChild, CreateLinkAnnotation
      *}
   end;
//---------------------------------------------------------------------------

implementation

const
   LITEPDF_DLL_NAME = 'litePDF.dll';

//----------------------------------------------------------------------------

procedure ThrowIfFail(expr : Boolean;
                      const exprStr : AnsiString;
                      const _func : AnsiString);
var exmsg : AnsiString;
begin
   if not expr then
   begin
      exmsg := _func + ': Assertion ''' + exprStr + ''' failed';
      raise TLitePDFException.Create(ERROR_INVALID_PARAMETER, exmsg);
   end;
end;

procedure ThrowMessageIfFail(expr : Boolean;
                             const msg : AnsiString;
                             const _func : AnsiString);
var exmsg : AnsiString;
begin
   if not expr then
   begin
      exmsg := _func + ': ' + msg;
      raise TLitePDFException.Create(ERROR_INVALID_PARAMETER, exmsg);
   end;
end;

procedure ThrowLastErrorIfFail(expr : Boolean;
                               lpdf : TLitePDF;
                               const _func : AnsiString);
var exmsg : AnsiString;
begin
   if not expr then
   begin
      exmsg := 'Failed to call ''' + _func + '''';
      if Length(lpdf.getLastErrorMessage) <> 0 then
      begin
         exmsg := lpdf.getLastErrorMessage;
      end;
      raise TLitePDFException.Create(lpdf.getLastErrorCode, exmsg);
   end;
end;

procedure litePDFError(code : LongWord;
                       const msg : PAnsiChar;
                       user_data : Pointer); stdcall;
const _func = 'LitePDF::litePDFError';
var lpdf : TLitePDF;
begin
   ThrowIfFail(user_data <> nil, 'user_data <> nil', _func);

   lpdf := TLitePDF(user_data);

   lpdf.setLastError(code, msg);
   if Assigned(lpdf.onError) then begin
      lpdf.onError(code, msg, lpdf.onErrorUserData);
   end;
end;

function litePDFEvalFontFlag(inout_faceName : PAnsiChar;
                             faceNameBufferSize : LongWord;
                             user_data : Pointer) : LongWord; stdcall;
const _func = 'LitePDF::litePDFEvalFontFlag';
var lpdf : TLitePDF;
begin
   ThrowIfFail(user_data <> nil, 'user_data <> nil', _func);

   lpdf := TLitePDF(user_data);

   if Assigned(lpdf.onEvalFontFlag) then
      Result := lpdf.onEvalFontFlag(inout_faceName, faceNameBufferSize, lpdf.onEvalFontFlagUserData)
   else
      Result := 0 { LitePDFFontFlag_Default };
end;
//----------------------------------------------------------------------------

constructor TLitePDFException.Create(pCode : DWORD;
                                     const pMsg : AnsiString);
begin
   inherited Create(string(pMsg));
   code := pCode;
   msg := pMsg;
end;

constructor TLitePDFException.Create(const src: TLitePDFException);
begin
   inherited Create(string(src.getMessage));
   code := src.getCode;
   msg := src.getMessage;
end;

function TLitePDFException.getCode : DWORD;
begin
   Result := code;
end;

function TLitePDFException.getMessage : AnsiString;
begin
   Result := msg;
end;

//----------------------------------------------------------------------------

constructor TLitePDF.Create;
begin
   inherited Create;

   lib := THandle(0);
   context := nil;
   lastErrorCode := 0;
   lastErrorMessage := '';
   onError := nil;
   onErrorUserData := nil;
   onEvalFontFlag := nil;
   onEvalFontFlagUserData := nil;
end;

destructor TLitePDF.Destroy;
begin
   unloadLibrary;

   inherited;
end;

function TLitePDF.GetProc(const pProcIdent : PAnsiChar) : FARPROC;
const _func = 'TLitePDF.GetProc';
var err : AnsiString;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(Assigned(pProcIdent), 'Assigned(pProcIdent)', _func);
   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   Result := GetProcAddress(lib, pProcIdent);

   err := 'Proc ''' + AnsiString(pProcIdent) + ''' not found';

   ThrowMessageIfFail(Result <> FARPROC(0), err, _func);
end;

procedure TLitePDF.setOnError(pOnError: TLitePDFErrorEvent;
                              pOnErrorUserData : Pointer);
begin
   onError := pOnError;
   onErrorUserData := pOnErrorUserData;
end;

function TLitePDF.getLastErrorCode: LongWord;
begin
   Result := lastErrorCode;
end;

function TLitePDF.getLastErrorMessage : AnsiString;
begin
   Result := lastErrorMessage
end;

procedure TLitePDF.freeLastError;
begin
   lastErrorMessage := '';
   lastErrorCode := 0;
end;

procedure TLitePDF.setLastError(code : DWORD;
                                const msg : PAnsiChar);
begin
   freeLastError;

   lastErrorCode := code;
   if msg <> nil then
   begin
      lastErrorMessage := msg;
   end;
end;

function TLitePDF.checkAPIVersion(major : LongWord;
                                  minor : LongWord) : Boolean;
var fileName : array[0..2049] of AnsiChar;
   fileNameLen : DWORD;
   apiIsOK : Boolean;
   dwVerHnd, dwVerInfoSize : DWORD;
   hMem : THandle;
   lpvMem : Pointer;
   VersionInfo : PVSFixedFileInfo;
   cchVer : UINT;
   fRet : BOOL;
begin
   Result := False;

   if lib = THandle(0) then
   begin
      Exit;
   end;

   fileNameLen := GetModuleFileNameA(lib, fileName, 2048);
   if fileNameLen = 0 then
   begin
      Exit;
   end;

   fileName[fileNameLen] := Char(0);

   apiIsOK := False;

   dwVerInfoSize := GetFileVersionInfoSizeA(fileName, dwVerHnd);

   if dwVerInfoSize <> 0 then
   begin
      hMem := GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
      lpvMem := GlobalLock(hMem);

      VersionInfo := nil;
      if GetFileVersionInfoA(fileName, dwVerHnd, dwVerInfoSize, lpvMem) then
      begin
         fRet := VerQueryValueA(Pointer(lpvMem), '\\', Pointer(VersionInfo), cchVer);

         if (fRet) and (cchVer <> 0) then
         begin
            apiIsOK := (HiWord(VersionInfo.dwFileVersionMS) = major) and
                       (LoWord(VersionInfo.dwFileVersionMS) = minor);
         end;

         GlobalUnlock(hMem);
         GlobalFree(hMem);
      end;
   end;

   Result := apiIsOK;
end;

procedure TLitePDF.ensureLibraryLoaded(const _func : PAnsiChar);
type
   litePDFErrorCB = procedure (code : LongWord; const msg : PAnsiChar; user_data : Pointer); stdcall;
   lpfunc = function (on_error : litePDFErrorCB; on_error_user_data : Pointer) : Pointer; stdcall;
   lpfunc2 = function (pctx : Pointer; callback : TLitePDFEvalFontFlagCB; callback_user_data : Pointer) : Boolean; stdcall;
var exmsg : AnsiString;
   func : lpfunc;
   func2 : lpfunc2;
begin
   if lib <> THandle(0) then
   begin
      Exit;
   end;

   ThrowIfFail(lib = THandle(0), 'lib = THandle(0)', _func);
   ThrowIfFail(context = nil, 'context = nil', _func);

   lib := LoadLibrary(LITEPDF_DLL_NAME);
   ThrowMessageIfFail (lib <> THandle(0), 'Failed to open litePDF.dll', _func);

   if not checkAPIVersion(LitePDF_API_Major, LitePDF_API_Minor) then
   begin
      FreeLibrary (lib);
      lib := THandle(0);

      exmsg := AnsiString(_func) + ': ' + 'This TLitePDF class is not designed for API version of litePDF.dll';
      raise TLitePDFException.Create(ERROR_INVALID_DLL, exmsg);
   end;

   freeLastError;
   func := lpfunc(GetProc('litePDF_CreateContext'));

   context := func (litePDFError, self);

   if context = nil then
   begin
      FreeLibrary (lib);
      lib := THandle(0);
      ThrowMessageIfFail (context <> nil, 'Failed to create context', _func);
   end else begin
      func2 := lpfunc2(GetProc('litePDF_SetEvalFontFlagCallback'));
      ThrowLastErrorIfFail(func2(context, litePDFEvalFontFlag, self), self, _func);
   end;
end;

procedure TLitePDF.unloadLibrary;
type lpfunc = procedure(context : Pointer); stdcall;
var func : lpfunc;
begin
   if (lib <> THandle(0)) and (context <> nil) then
   begin
      try
         freeLastError;
         func := lpfunc(GetProc('litePDF_FreeContext'));
         func(context);
      except
      end;
      FreeLibrary(lib);
   end;

   context := nil;
   lib := THandle(0);
end;

procedure TLitePDF.SetUnit(unitValue : TLitePDFUnit);
const _func = 'TLitePDF.SetUnit';
type lpfunc = function(pctx : Pointer; unitValue : LongWord) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_SetUnit'));

   ThrowLastErrorIfFail(func(context, LongWord(unitValue)), self, _func);
end;

function TLitePDF.GetUnit : TLitePDFUnit;
const _func = 'TLitePDF.GetUnit';
type lpfunc = function(pctx : Pointer) : LongWord; stdcall;
var func : lpfunc;
    currentUnit : LongWord;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetUnit'));

   currentUnit := func(context);

   ThrowLastErrorIfFail((TLitePDFUnit(currentUnit) > LitePDFUnit_Unknown) and (TLitePDFUnit(currentUnit) <= LitePDFUnit_1000th_inch), self, _func);

   Result := TLitePDFUnit(currentUnit);
end;
function TLitePDF.MMToUnitEx(useUnit : TLitePDFUnit;
                             mmValue : Double) : Double;
var ratio : Double;
begin
   ratio := 1.0;

   case useUnit of
      LitePDFUnit_mm:
         ratio := 1.0;
      LitePDFUnit_10th_mm:
         ratio := 10.0;
      LitePDFUnit_100th_mm:
         ratio := 100.0;
      LitePDFUnit_1000th_mm:
         ratio := 1000.0;
      LitePDFUnit_inch:
         ratio := 1.0 / 25.4;
      LitePDFUnit_10th_inch:
         ratio := 10.0 / 25.4;
      LitePDFUnit_100th_inch:
         ratio := 100.0 / 25.4;
      LitePDFUnit_1000th_inch:
         ratio := 1000.0 / 25.4;
      LitePDFUnit_Unknown:
         ;
   end;

   Result := mmValue * ratio;
end;

function TLitePDF.UnitToMMEx(useUnit : TLitePDFUnit;
                             unitValue : Double) : Double;
var ratio : Double;
begin
   ratio := 1.0;

   case useUnit of
      LitePDFUnit_mm:
         ratio := 1.0;
      LitePDFUnit_10th_mm:
         ratio := 1.0 / 10.0;
      LitePDFUnit_100th_mm:
         ratio := 1.0 / 100.0;
      LitePDFUnit_1000th_mm:
         ratio := 1.0 / 1000.0;
      LitePDFUnit_inch:
         ratio := 25.4;
      LitePDFUnit_10th_inch:
         ratio := 25.4 / 10.0;
      LitePDFUnit_100th_inch:
         ratio := 25.4 / 100.0;
      LitePDFUnit_1000th_inch:
         ratio := 25.4 / 1000.0;
      LitePDFUnit_Unknown:
         ;
   end;

   Result := unitValue * ratio;
end;

function TLitePDF.InchToUnitEx(useUnit : TLitePDFUnit;
                               inchValue : Double) : Double;
var ratio : Double;
begin
   ratio := 1.0;

   case useUnit of
      LitePDFUnit_mm:
         ratio := 25.4;
      LitePDFUnit_10th_mm:
         ratio := 10.0 * 25.4;
      LitePDFUnit_100th_mm:
         ratio := 100.0 * 25.4;
      LitePDFUnit_1000th_mm:
         ratio := 1000.0 * 25.4;
      LitePDFUnit_inch:
         ratio := 1.0;
      LitePDFUnit_10th_inch:
         ratio := 10.0;
      LitePDFUnit_100th_inch:
         ratio := 100.0;
      LitePDFUnit_1000th_inch:
         ratio := 1000.0;
      LitePDFUnit_Unknown:
         ;
   end;

   Result := inchValue * ratio;
end;

function TLitePDF.UnitToInchEx(useUnit : TLitePDFUnit;
                               unitValue : Double) : Double;
var ratio : Double;
begin
   ratio := 1.0;

   case useUnit of
      LitePDFUnit_mm:
         ratio := 1.0 / 25.4;
      LitePDFUnit_10th_mm:
         ratio := 1.0 / (25.4 * 10.0);
      LitePDFUnit_100th_mm:
         ratio := 1.0 / (25.4 * 100.0);
      LitePDFUnit_1000th_mm:
         ratio := 1.0 / (25.4 * 1000.0);
      LitePDFUnit_inch:
         ratio := 1.0;
      LitePDFUnit_10th_inch:
         ratio := 1.0 / 10.0;
      LitePDFUnit_100th_inch:
         ratio := 1.0 / 100.0;
      LitePDFUnit_1000th_inch:
         ratio := 1.0 / 1000.0;
      LitePDFUnit_Unknown:
         ;
   end;

   Result := unitValue * ratio;
end;

function TLitePDF.MMToUnit(mmValue : Double) : Double;
begin
   Result := MMToUnitEx(GetUnit, mmValue);
end;

function TLitePDF.UnitToMM(unitValue : Double) : Double;
begin
   Result := UnitToMMEx(GetUnit, unitValue);
end;

function TLitePDF.InchToUnit(inchValue : Double) : Double;
begin
   Result := InchToUnitEx(GetUnit, inchValue);
end;

function TLitePDF.UnitToInch(unitValue : Double) : Double;
begin
   Result := UnitToInchEx(GetUnit, unitValue);
end;

procedure TLitePDF.SetEvalFontFlagCallback(callback : TLitePDFEvalFontFlagCB;
                                           userData : Pointer);
begin
   onEvalFontFlag := callback;
   onEvalFontFlagUserData := userData;
end;

procedure TLitePDF.PrepareEncryption(userPassword : AnsiString;
                                     ownerPassword : AnsiString;
                                     permissions : LongWord;
                                     algorithm : LongWord);
const _func = 'TLitePDF.PrepareEncryption';
type lpfunc = function(pctx : Pointer; const userPassword : PAnsiChar; const ownerPassword : PAnsiChar; permissions : LongWord; algorithm : LongWord) : BOOL; stdcall;
var func : lpfunc;
   ownerPass : PAnsiChar;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_PrepareEncryption'));

   if Length(ownerPassword) <= 0 then
      ownerPass := nil
   else
      ownerPass := PAnsiChar(ownerPassword);

   ThrowLastErrorIfFail(func(context, PAnsiChar(userPassword), ownerPass, permissions, algorithm), self, _func);
end;

procedure TLitePDF.CreateFileDocument(const fileName : AnsiString);
const _func = 'TLitePDF.CreateFileDocument';
type lpfunc = function(pctx : Pointer; const fileName : PAnsiChar) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_CreateFileDocument'));

   ThrowLastErrorIfFail(func(context, PAnsiChar(fileName)), self, _func);
end;

procedure TLitePDF.CreateFileDocumentW(const fileName : WideString);
const _func = 'TLitePDF.CreateFileDocumentW';
type lpfunc = function(pctx : Pointer; const fileName : PWideChar) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_CreateFileDocumentW'));

   ThrowLastErrorIfFail(func(context, PWideChar(fileName)), self, _func);
end;

procedure TLitePDF.CreateMemDocument;
const _func = 'TLitePDF.CreateMemDocument';
type lpfunc = function(pctx : Pointer) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_CreateMemDocument'));

   ThrowLastErrorIfFail(func(context), self, _func);
end;

procedure TLitePDF.LoadFromFile(const fileName : AnsiString;
                                const password : AnsiString;
                                loadCompletely : Boolean;
                                forUpdate : Boolean);
const _func = 'TLitePDF.LoadFromFile';
type lpfunc = function(pctx : Pointer; const fileName : PAnsiChar; const password : PAnsiChar; loadCompletely : BOOL; forUpdate : BOOL) : BOOL; stdcall;
var func : lpfunc;
   lc : BOOL;
   fu : BOOL;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);
   ThrowIfFail(Length(fileName) > 0, 'Length(fileName) > 0', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_LoadFromFile'));

   if loadCompletely then
      lc := TRUE
   else
      lc := FALSE;

   if forUpdate then
      fu := TRUE
   else
      fu := FALSE;

   ThrowLastErrorIfFail(func(context, PAnsiChar(fileName), PAnsiChar(password), lc, fu), self, _func);
end;

procedure TLitePDF.LoadFromFileW(const fileName : WideString;
                                 const password : AnsiString;
                                 loadCompletely : Boolean;
                                 forUpdate : Boolean);
const _func = 'TLitePDF.LoadFromFileW';
type lpfunc = function(pctx : Pointer; const fileName : PWideChar; const password : PAnsiChar; loadCompletely : BOOL; forUpdate : BOOL) : BOOL; stdcall;
var func : lpfunc;
   lc : BOOL;
   fu : BOOL;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);
   ThrowIfFail(Length(fileName) > 0, 'Length(fileName) > 0', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_LoadFromFileW'));

   if loadCompletely then
      lc := TRUE
   else
      lc := FALSE;

   if forUpdate then
      fu := TRUE
   else
      fu := FALSE;

   ThrowLastErrorIfFail(func(context, PWideChar(fileName), PAnsiChar(password), lc, fu), self, _func);
end;

procedure TLitePDF.LoadFromData(data : PByte;
                                dataLength : LongWord;
                                const password : AnsiString;
                                forUpdate : Boolean);
const _func = 'TLitePDF.LoadFromData';
type lpfunc = function(pctx : Pointer; const data : PByte; dataLength : LongWord; const password : PAnsiChar; forUpdate : BOOL) : BOOL; stdcall;
var func : lpfunc;
    fu : BOOL;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);
   ThrowIfFail(data <> nil, 'data <> nil', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_LoadFromData'));

   if forUpdate then
      fu := TRUE
   else
      fu := FALSE;

   ThrowLastErrorIfFail(func(context, data, dataLength, PAnsiChar(password), fu), self, _func);
end;

procedure TLitePDF.SaveToFile(const fileName : AnsiString);
const _func = 'TLitePDF.SaveToFile';
type lpfunc = function(pctx : Pointer; const fileName : PAnsiChar) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_SaveToFile'));

   ThrowLastErrorIfFail(func(context, PAnsiChar(fileName)), self, _func);
end;

procedure TLitePDF.SaveToFileW(const fileName : WideString);
const _func = 'TLitePDF.SaveToFileW';
type lpfunc = function(pctx : Pointer; const fileName : PWideChar) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_SaveToFileW'));

   ThrowLastErrorIfFail(func(context, PWideChar(fileName)), self, _func);
end;

function TLitePDF.SaveToData(data : PByte;
                             var dataLength : LongWord) : Boolean;
const _func = 'TLitePDF.SaveToData';
type lpfunc = function(pctx: Pointer; data : PByte; dataLength : PLongWord) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_SaveToData'));

   if func(context, data, @dataLength) then
      Result := True
   else
      Result := False;
end;

procedure TLitePDF.Close;
const _func = 'TLitePDF.Close';
type lpfunc = procedure(pctx : Pointer); stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_Close'));

   func(context);
end;

function TLitePDF.GetPageCount : LongWord;
const _func = 'TLitePDF.GetPageCount';
type lpfunc = function(pctx : Pointer; pageCount : PLongWord) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetPageCount'));

   Result := 0;
   ThrowLastErrorIfFail(func(context, @Result), self, _func);
end;

procedure TLitePDF.GetPageSize(pageIndex : LongWord;
                               var width_u : LongWord;
                               var height_u : LongWord);
const _func = 'TLitePDF.GetPageSize';
type lpfunc = function(pctx : Pointer; pageIndex : LongWord; width_u : PLongWord; height_u : PLongWord) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetPageSize'));

   ThrowLastErrorIfFail(func(context, pageIndex, @width_u, @height_u), self, _func);
end;

function TLitePDF.GetPageRotation(pageIndex : LongWord) : Integer;
const _func = 'TLitePDF.GetPageRotation';
type lpfunc = function(pctx : Pointer; pageIndex : LongWord; out_degrees : PInteger) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetPageRotation'));

   ThrowLastErrorIfFail(func(context, pageIndex, @Result), self, _func);
end;

procedure TLitePDF.SetPageRotation(pageIndex : LongWord;
                                   degrees : Integer);
const _func = 'TLitePDF.SetPageRotation';
type lpfunc = function(pctx : Pointer; pageIndex : LongWord; degrees : Integer) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_SetPageRotation'));

   ThrowLastErrorIfFail(func(context, pageIndex, degrees), self, _func);
end;

function TLitePDF.AddPage(width_u : LongWord;
                          height_u : LongWord;
                          width_px : LongWord;
                          height_px : LongWord;
                          drawFlags : LongWord) : HDC;
const _func = 'TLitePDF.AddPage';
type lpfunc = function(pctx : Pointer; width_u : LongWord; height_u : LongWord; width_px : LongWord; height_px : LongWord; drawFlags : LongWord) : HDC; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_AddPage'));

   Result := func (context, width_u, height_u, width_px, height_px, drawFlags);
   ThrowLastErrorIfFail(Result <> HDC(0), self, _func);
end;

function TLitePDF.InsertPage(pageIndex : LongWord;
                             width_u : LongWord;
                             height_u : LongWord;
                             width_px : LongWord;
                             height_px : LongWord;
                             drawFlags : LongWord) : HDC;
const _func = 'TLitePDF.InsertPage';
type lpfunc = function(pctx : Pointer; pageIndex : LongWord; width_u : LongWord; height_u : LongWord; width_px : LongWord; height_px : LongWord; drawFlags : LongWord) : HDC; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_InsertPage'));

   Result := func(context, pageIndex, width_u, height_u, width_px, height_px, drawFlags);
   ThrowLastErrorIfFail(Result <> HDC(0), self, _func);
end;

function TLitePDF.UpdatePage(pageIndex : LongWord;
                             width_px : LongWord;
                             height_px : LongWord;
                             drawFlags : LongWord) : HDC;
const _func = 'TLitePDF.UpdatePage';
type lpfunc = function(pctx : Pointer; pageIndex : LongWord; width_px : LongWord; height_px : LongWord; drawFlags : LongWord) : HDC; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_UpdatePage'));

   Result := func(context, pageIndex, width_px, height_px, drawFlags);
   ThrowLastErrorIfFail(Result <> HDC(0), self, _func);
end;

procedure TLitePDF.FinishPage(dc : HDC);
const _func = 'TLitePDF.FinishPage';
type lpfunc = function(pctx : Pointer; dc : HDC) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);
   ThrowIfFail(dc <> HDC(0), 'dc <> HDC(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_FinishPage'));

   ThrowLastErrorIfFail(func(context, dc), self, _func);
end;

function TLitePDF.AddResource(width_u : LongWord;
                              height_u : LongWord;
                              width_px : LongWord;
                              height_px : LongWord;
                              drawFlags : LongWord) : HDC;
const _func = 'TLitePDF.AddResource';
type lpfunc = function(pctx : Pointer; width_u : LongWord; height_u : LongWord; width_px : LongWord; height_px : LongWord; drawFlags : LongWord) : HDC; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_AddResource'));

   Result := func(context, width_u, height_u, width_px, height_px, drawFlags);
   ThrowLastErrorIfFail(Result <> HDC(0), self, _func);
end;

function TLitePDF.FinishResource(dc : HDC) : LongWord;
const _func = 'TLitePDF.FinishResource';
type lpfunc = function(pctx : Pointer; dc : HDC) : LongWord; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);
   ThrowIfFail(dc <> HDC(0), 'dc <> HDC(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_FinishResource'));

   Result := func(context, dc);
   ThrowLastErrorIfFail(Result <> 0, self, _func);
end;

procedure TLitePDF.DeletePage(pageIndex : LongWord);
const _func = 'TLitePDF.DeletePage';
type lpfunc = function(pctx : Pointer; pageIndex : LongWord) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_DeletePage'));

   ThrowLastErrorIfFail(func(context, pageIndex), self, _func);
end;

procedure TLitePDF.AddPagesFrom(from : TLitePDF;
                                pageIndex : LongWord;
                                pageCount : LongWord);
const _func = 'TLitePDF.AddPagesFrom';
type lpfunc = function(pctx : Pointer; pctx_from : Pointer; pageIndex : LongWord; pageCount : LongWord) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);
   ThrowIfFail(from <> nil, 'from <> nil', _func);
   ThrowIfFail(from <> self, 'from != self', _func);
   ThrowIfFail(from.context <> nil, 'from.context <> nil', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_AddPagesFrom'));

   ThrowLastErrorIfFail(func(context, from.context, pageIndex, pageCount), self, _func);
end;

procedure TLitePDF.InsertPageFrom(pageIndexTo : LongWord;
                                  from : TLitePDF;
                                  pageIndexFrom : LongWord);
const _func = 'TLitePDF.InsertPageFrom';
type lpfunc = function(pctx : Pointer; pageIndexTo : LongWord; pctx_from : Pointer; pageIndexFrom : LongWord) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);
   ThrowIfFail(from <> nil, 'from <> nil', _func);
   ThrowIfFail(from <> self, 'from != self', _func);
   ThrowIfFail(from.context <> nil, 'from.context <> nil', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_InsertPageFrom'));

   ThrowLastErrorIfFail(func(context, pageIndexTo, from.context, pageIndexFrom), self, _func);
end;

function TLitePDF.AddPageFromAsResource(from : TLitePDF;
                                        pageIndex : LongWord;
                                        useTrimBox : Boolean) : LongWord;
const _func = 'TLitePDF.AddPageFromAsResource';
type lpfunc = function(pctx : Pointer; pctx_from : Pointer; pageIndex : LongWord; useTrimBox : BOOL) : LongWord; stdcall;
var func : lpfunc;
   utb : BOOL;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);
   ThrowIfFail(from <> nil, 'from <> nil', _func);
   ThrowIfFail(from <> self, 'from != self', _func);
   ThrowIfFail(from.context <> nil, 'from.context <> nil', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_AddPageFromAsResource'));

   if useTrimBox then
      utb := TRUE
   else
      utb := FALSE;

   Result := func(context, from.context, pageIndex, utb);
   ThrowLastErrorIfFail(Result <> 0, self, _func);
end;

function TLitePDF.PageToResource(pageIndex : LongWord) : LongWord;
const _func = 'TLitePDF.PageToResource';
type lpfunc = function(pctx : Pointer; pageIndex : LongWord) : LongWord; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_PageToResource'));

   Result := func(context, pageIndex);
   ThrowLastErrorIfFail(Result <> 0, self, _func);
end;

procedure TLitePDF.GetResourceSize(resourceID : LongWord;
                                   var width_u : LongWord;
                                   var height_u : LongWord);
const _func = 'TLitePDF.GetResourceSize';
type lpfunc = function(pctx : Pointer; resourceID : LongWord; width_u : PLongWord; height_u : PLongWord) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetResourceSize'));

   ThrowLastErrorIfFail(func(context, resourceID, @width_u, @height_u), self, _func);
end;

procedure TLitePDF.DrawResource(resourceID : LongWord;
                                pageIndex : LongWord;
								unitValue : TLitePDFUnit;
                                x : Integer;
                                y : Integer;
                                scaleX : Integer;
                                scaleY : Integer);
const _func = 'TLitePDF.DrawResource';
type lpfunc = function(pctx : Pointer; resourceID : LongWord; pageIndex : LongWord; unitValue : LongWord; x : Integer; y : Integer; scaleX : Integer; scaleY : Integer) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_DrawResource'));

   ThrowLastErrorIfFail(func(context, resourceID, pageIndex, LongWord(unitValue), x, y, scaleX, scaleY), self, _func);
end;

procedure TLitePDF.DrawResourceWithMatrix(resourceID : LongWord;
                                          pageIndex : LongWord;
                                          a : Double;
                                          b : Double;
                                          c : Double;
                                          d : Double;
                                          e : Double;
                                          f : Double);
const _func = 'TLitePDF.DrawResourceWithMatrix';
type lpfunc = function(pctx : Pointer; resourceID : LongWord; pageIndex : LongWord; a : Integer; b : Integer; c : Integer; d : Integer; e : Integer; f : Integer) : BOOL; stdcall;
var func : lpfunc;
   i_a, i_b, i_c, i_d, i_e, i_f : Integer;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_DrawResourceWithMatrix'));

   i_a := Trunc(a * 1000.0);
   i_b := Trunc(b * 1000.0);
   i_c := Trunc(c * 1000.0);
   i_d := Trunc(d * 1000.0);
   i_e := Trunc(e * 1000.0);
   i_f := Trunc(f * 1000.0);

   ThrowLastErrorIfFail(func(context, resourceID, pageIndex, i_a, i_b, i_c, i_d, i_e, i_f), self, _func);
end;

procedure TLitePDF.SetDocumentInfo(const name : AnsiString;
                                   const value : WideString);
const _func = 'TLitePDF.SetDocumentInfo';
type lpfunc = function(pctx : Pointer; const name : PAnsiChar; const value : PWideChar) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_SetDocumentInfo'));

   ThrowLastErrorIfFail(func(context, PAnsiChar(name), PWideChar(value)), self, _func);
end;

function TLitePDF.GetDocumentInfoExists(const name : AnsiString) : Boolean;
const _func = 'TLitePDF.GetDocumentInfoExists';
type lpfunc = function(pctx : Pointer; const name : PAnsiChar; pExists : PBOOL) : BOOL; stdcall;
var func : lpfunc;
   exists : BOOL;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetDocumentInfoExists'));

   exists := FALSE;

   ThrowLastErrorIfFail(func(context, PAnsiChar(name), @exists), self, _func);

   if exists then
      Result := True
   else
      Result := False;
end;

function TLitePDF.GetDocumentInfo(const name : AnsiString) : WideString;
const _func = 'TLitePDF.GetDocumentInfo';
type lpfunc = function(pctx : Pointer; const name : PAnsiChar; value : PWideChar; valueLength : PLongWord) : BOOL; stdcall;
var func : lpfunc;
   valueLength : LongWord;
   buff : PWideChar;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetDocumentInfo'));

   valueLength := 0;
   ThrowLastErrorIfFail(func(context, PAnsiChar(name), nil, @valueLength), self, _func);

   buff := AllocMem(SizeOf(WideChar) * (valueLength + 1));
   ThrowMessageIfFail(buff <> nil, 'Out of memory!', _func);

   if func(context, PAnsiChar(name), buff, @valueLength) then
   begin
      buff[valueLength] := Char(0);
      Result := buff;
      FreeMem (buff);
   end
   else
   begin
      FreeMem (buff);

      // always false
      ThrowLastErrorIfFail(buff = nil, self, _func);
   end;
end;

function TLitePDF.GetDocumentIsSigned : Boolean;
const _func = 'TLitePDF.GetDocumentIsSigned';
type lpfunc = function(pctx : Pointer; pIsSigned : PBOOL) : BOOL; stdcall;
var func : lpfunc;
   isSigned : BOOL;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetDocumentIsSigned'));

   isSigned := FALSE;

   ThrowLastErrorIfFail(func(context, @isSigned), self, _func);

   if isSigned then
      Result := True
   else
      Result := False;
end;

function TLitePDF.GetSignatureCount : LongWord;
const _func = 'TLitePDF.GetSignatureCount';
type lpfunc = function(pctx : Pointer; pCount : PLongWord) : BOOL; stdcall;
var func : lpfunc;
   count : LongWord;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetSignatureCount'));

   count := 0;

   ThrowLastErrorIfFail(func(context, @count), self, _func);

   Result := count;
end;

function TLitePDF.GetSignatureName(index : LongWord) : AnsiString;
const _func = 'TLitePDF.GetSignatureName';
type lpfunc = function(pctx : Pointer; index : LongWord; name : PAnsiChar; nameLength : PLongWord) : BOOL; stdcall;
var func : lpfunc;
   nameLength : LongWord;
   buff : PAnsiChar;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetSignatureName'));

   nameLength := 0;
   ThrowLastErrorIfFail(func(context, index, nil, @nameLength), self, _func);

   Inc(nameLength);

   buff := AllocMem(SizeOf(AnsiChar) * (nameLength));
   ThrowMessageIfFail(buff <> nil, 'Out of memory!', _func);

   if func(context, index, buff, @nameLength) then
   begin
      buff[nameLength] := Char(0);
      Result := buff;
      FreeMem (buff);
   end
   else
   begin
      FreeMem (buff);

      // always false
      ThrowLastErrorIfFail(buff <> nil, self, _func);
   end;
end;

function TLitePDF.CreateSignature(name : AnsiString;
                                  annotationPageIndex : LongWord;
                                  annotationPosition_u : TRect;
                                  annotationFlags : LongWord) : LongWord;
const _func = 'TLitePDF.CreateSignature';
type lpfunc = function(pctx : Pointer;
                       const name : PAnsiChar;
                       annotationPageIndex : LongWord;
                       annotationX_u : Integer;
                       annotationY_u : Integer;
                       annotationWidth_u : Integer;
                       annotationHeight_u : Integer;
                       annotationFlags : LongWord;
                       pAddedIndex : PLongWord) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_CreateSignature'));

   ThrowLastErrorIfFail(func(context,
               PAnsiChar(name),
               annotationPageIndex,
               annotationPosition_u.left,
               annotationPosition_u.top,
               annotationPosition_u.right - annotationPosition_u.left,
               annotationPosition_u.bottom - annotationPosition_u.top,
               annotationFlags,
               @Result), self, _func);
end;

function TLitePDF.GetSignatureHasData(index : LongWord) : Boolean;
const _func = 'TLitePDF.GetSignatureHasData';
type lpfunc = function(pctx : Pointer;
                       index : LongWord;
                       pHasData : PBOOL) : BOOL; stdcall;
var func : lpfunc;
    hasData : BOOL;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetSignatureHasData'));

   ThrowLastErrorIfFail(func(context, index, @hasData), self, _func);

   if hasData then
      Result := True
   else
      Result := False;
end;

function TLitePDF.GetSignatureData(index : LongWord;
                                       data : PByte;
                                       var dataLength : LongWord) : Boolean;
const _func = 'TLitePDF.GetSignatureData';
type lpfunc = function(pctx : Pointer; index : LongWord; data : PByte; dataLength : PLongWord) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetSignatureData'));

   if func(context, index, data, @dataLength) then
      Result := True
   else
      Result := False;
end;

function TLitePDF.GetSignatureRanges(index : LongWord;
                                     pRangesArray : PUInt64;
                                     var pRangesArrayLength : LongWord) : Boolean;
const _func = 'TLitePDF.GetSignatureRanges';
type lpfunc = function(pctx : Pointer; index : LongWord; pRangesArray : PUInt64; pRangesArrayLength : PLongWord) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetSignatureRanges'));

   if func(context, index, pRangesArray, @pRangesArrayLength) then
      Result := True
   else
      Result := False;
end;

procedure TLitePDF.SetSignatureDate(index : LongWord;
                                    const dateOfSign : TDateTime);
const _func = 'TLitePDF.SetSignatureDate';
type lpfunc = function(pctx : Pointer; index : LongWord; dateOfSign : Int64) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_SetSignatureDate'));

   ThrowLastErrorIfFail(func(context, index, System.DateUtils.DateTimeToUnix(dateOfSign)), self, _func);
end;

function TLitePDF.GetSignatureDate(index : LongWord) : TDateTime;
const _func = 'TLitePDF.GetSignatureDate';
type lpfunc = function(pctx : Pointer; index : LongWord; pDateOfSign : PInt64) : BOOL; stdcall;
var func : lpfunc;
    dateOfSign : Int64;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetSignatureDate'));

   ThrowLastErrorIfFail(func(context, index, @dateOfSign), self, _func);

   Result := System.DateUtils.UnixToDateTime(dateOfSign);
end;

procedure TLitePDF.SetSignatureReason(index : LongWord;
                                      const reason : WideString);
const _func = 'TLitePDF.SetSignatureReason';
type lpfunc = function(pctx : Pointer; index : LongWord; const reason : PWideChar) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_SetSignatureReason'));

   ThrowLastErrorIfFail(func(context, index, PWideChar(reason)), self, _func);
end;

function TLitePDF.GetSignatureReason(index : LongWord) : WideString;
const _func = 'TLitePDF.GetSignatureReason';
type lpfunc = function(pctx : Pointer; index : LongWord; value : PWideChar; valueLength : PLongWord) : BOOL; stdcall;
var func : lpfunc;
   valueLength : LongWord;
   buff : PWideChar;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetSignatureReason'));

   valueLength := 0;
   ThrowLastErrorIfFail(func(context, index, nil, @valueLength), self, _func);

   Inc(valueLength);

   buff := AllocMem(SizeOf(WideChar) * (valueLength));
   ThrowMessageIfFail(buff <> nil, 'Out of memory!', _func);

   if func(context, index, buff, @valueLength) then
   begin
      buff[valueLength] := WideChar(0);
      Result := buff;
      FreeMem (buff);
   end
   else
   begin
      FreeMem (buff);

      // always false
      ThrowLastErrorIfFail(buff <> nil, self, _func);
   end;
end;

procedure TLitePDF.SetSignatureLocation(index : LongWord;
                                        const location : WideString);
const _func = 'TLitePDF.SetSignatureLocation';
type lpfunc = function(pctx : Pointer; index : LongWord; const location : PWideChar) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_SetSignatureLocation'));

   ThrowLastErrorIfFail(func(context, index, PWideChar(location)), self, _func);
end;

function TLitePDF.GetSignatureLocation(index : LongWord) : WideString;
const _func = 'TLitePDF.GetSignatureLocation';
type lpfunc = function(pctx : Pointer; index : LongWord; value : PWideChar; valueLength : PLongWord) : BOOL; stdcall;
var func : lpfunc;
   valueLength : LongWord;
   buff : PWideChar;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetSignatureLocation'));

   valueLength := 0;
   ThrowLastErrorIfFail(func(context, index, nil, @valueLength), self, _func);

   Inc(valueLength);

   buff := AllocMem(SizeOf(WideChar) * (valueLength));
   ThrowMessageIfFail(buff <> nil, 'Out of memory!', _func);

   if func(context, index, buff, @valueLength) then
   begin
      buff[valueLength] := WideChar(0);
      Result := buff;
      FreeMem (buff);
   end
   else
   begin
      FreeMem (buff);

      // always false
      ThrowLastErrorIfFail(buff <> nil, self, _func);
   end;
end;

procedure TLitePDF.SetSignatureCreator(index : LongWord;
                                       const creator : AnsiString);
const _func = 'TLitePDF.SetSignatureCreator';
type lpfunc = function(pctx : Pointer; index : LongWord; const creator : PAnsiChar) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_SetSignatureCreator'));

   ThrowLastErrorIfFail(func(context, index, PAnsiChar(creator)), self, _func);
end;

function TLitePDF.GetSignatureCreator(index : LongWord) : AnsiString;
const _func = 'TLitePDF.GetSignatureCreator';
type lpfunc = function(pctx : Pointer; index : LongWord; value : PAnsiChar; valueLength : PLongWord) : BOOL; stdcall;
var func : lpfunc;
   valueLength : LongWord;
   buff : PAnsiChar;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetSignatureCreator'));

   valueLength := 0;
   ThrowLastErrorIfFail(func(context, index, nil, @valueLength), self, _func);

   Inc(valueLength);

   buff := AllocMem(SizeOf(AnsiChar) * (valueLength));
   ThrowMessageIfFail(buff <> nil, 'Out of memory!', _func);

   if func(context, index, buff, @valueLength) then
   begin
      buff[valueLength] := AnsiChar(0);
      Result := buff;
      FreeMem (buff);
   end
   else
   begin
      FreeMem (buff);

      // always false
      ThrowLastErrorIfFail(buff <> nil, self, _func);
   end;
end;

procedure TLitePDF.SetSignatureAppearance(index : LongWord;
                                          appearanceType : TLitePDFAppearance;
                                          resourceID : LongWord;
                                          offsetX_u : Integer;
                                          offsetY_u : Integer);
const _func = 'TLitePDF.SetSignatureAppearance';
type lpfunc = function(pctx : Pointer;
                       index : LongWord;
                       appearanceType : LongWord;
                       resourceID : LongWord;
                       offsetX_u : Integer;
                       offsetY_u : Integer) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_SetSignatureAppearance'));

   ThrowLastErrorIfFail(func(context, index, LongWord(appearanceType), resourceID, offsetX_u, offsetY_u), self, _func);
end;

procedure TLitePDF.SetSignatureSize(requestBytes : LongWord);
const _func = 'TLitePDF.SetSignatureSize';
type lpfunc = function(pctx : Pointer; requestBytes : LongWord) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_SetSignatureSize'));

   ThrowLastErrorIfFail(func(context, requestBytes), self, _func);
end;

procedure TLitePDF.AddSignerPFX(pfxData : PByte;
                                pfxDataLength : LongWord;
                                const pfxPassword : AnsiString);
const _func = 'TLitePDF.AddSignerPFX';
type lpfunc = function(pctx : Pointer;
                       const pfxData : PByte;
                       pfxDataLength : LongWord;
                       const pfxPassword : PAnsiChar) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_AddSignerPFX'));

   ThrowLastErrorIfFail(func(context, pfxData, pfxDataLength, PAnsiChar(pfxPassword)), self, _func);
end;

procedure TLitePDF.AddSignerPEM(pemData : PByte;
                                pemDataLength : LongWord;
                                pkeyData : PByte;
                                pkeyDataLength : LongWord;
                                const pkeyPassword : AnsiString);
const _func = 'TLitePDF.AddSignerPEM';
type lpfunc = function(pctx : Pointer;
                       const pemData : PByte;
                       pemDataLength : LongWord;
                       const pkeyData : PByte;
                       pkeyDataLength : LongWord;
                       const pkeyPassword : PAnsiChar) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_AddSignerPEM'));

   ThrowLastErrorIfFail(func(context,
      pemData,
      pemDataLength,
      pkeyData,
      pkeyDataLength,
      PAnsiChar(pkeyPassword)), self, _func);
end;

procedure TLitePDF.SaveToFileWithSign(const fileName : AnsiString;
                                      signatureIndex : LongWord);
const _func = 'TLitePDF.SaveToFileWithSign';
type lpfunc = function(pctx : Pointer; const fileName : PAnsiChar; signatureIndex : LongWord) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_SaveToFileWithSign'));

   ThrowLastErrorIfFail(func(context, PAnsiChar(fileName), signatureIndex), self, _func);
end;

procedure TLitePDF.SaveToFileWithSignW(const fileName : WideString;
                                       signatureIndex : LongWord);
const _func = 'TLitePDF.SaveToFileWithSignW';
type lpfunc = function(pctx : Pointer; const fileName : PWideChar; signatureIndex : LongWord) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_SaveToFileWithSignW'));

   ThrowLastErrorIfFail(func(context, PWideChar(fileName), signatureIndex), self, _func);
end;

function TLitePDF.SaveToDataWithSign(signatureIndex : LongWord;
                                     data : PByte;
                                     var dataLength : LongWord) : Boolean;
const _func = 'TLitePDF.SaveToDataWithSign';
type lpfunc = function(pctx : Pointer;
                       signatureIndex : LongWord;
                       data : PByte;
                       dataLength : PLongWord) : BOOL; stdcall;
var func : lpfunc;
   succeeded : BOOL;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_SaveToDataWithSign'));

   succeeded := func(context, signatureIndex, data, @dataLength);

   if succeeded then
      Result := True
   else
      Result := False;
end;

procedure TLitePDF.SaveToFileWithSignManual(const fileName : AnsiString;
                                            signatureIndex : LongWord;
                                            appendSignatureData : TLitePDFAppendSignatureDataFunc;
                                            append_user_data : Pointer;
                                            finishSignature : TLitePDFFinishSignatureFunc;
                                            finish_user_data : Pointer);
const _func = 'TLitePDF.SaveToFileWithSignManual';
type lpfunc = function(pctx : Pointer;
                       const fileName : PAnsiChar;
                       signatureIndex : LongWord;
                       appendSignatureData : TLitePDFAppendSignatureDataFunc;
                       append_user_data : Pointer;
                       finishSignature : TLitePDFFinishSignatureFunc;
                       finish_user_data : Pointer) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);
   ThrowIfFail(Assigned(appendSignatureData), 'Assigned(appendSignatureData)', _func);
   ThrowIfFail(Assigned(finishSignature), 'Assigned(finishSignature)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_SaveToFileWithSignManual'));

   ThrowLastErrorIfFail(func(context,
               PAnsiChar(fileName),
               signatureIndex,
               appendSignatureData,
               append_user_data,
               finishSignature,
               finish_user_data), self, _func);
end;

procedure TLitePDF.SaveToFileWithSignManualW(const fileName : WideString;
                                             signatureIndex : LongWord;
                                             appendSignatureData : TLitePDFAppendSignatureDataFunc;
                                             append_user_data : Pointer;
                                             finishSignature : TLitePDFFinishSignatureFunc;
                                             finish_user_data : Pointer);
const _func = 'TLitePDF.SaveToFileWithSignManualW';
type lpfunc = function(pctx : Pointer;
                       const fileName : PWideChar;
                       signatureIndex : LongWord;
                       appendSignatureData : TLitePDFAppendSignatureDataFunc;
                       append_user_data : Pointer;
                       finishSignature : TLitePDFFinishSignatureFunc;
                       finish_user_data : Pointer) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);
   ThrowIfFail(Assigned(appendSignatureData), 'Assigned(appendSignatureData)', _func);
   ThrowIfFail(Assigned(finishSignature), 'Assigned(finishSignature)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_SaveToFileWithSignManualW'));

   ThrowLastErrorIfFail(func(context,
               PWideChar(fileName),
               signatureIndex,
               appendSignatureData,
               append_user_data,
               finishSignature,
               finish_user_data), self, _func);
end;

function TLitePDF.SaveToDataWithSignManual(signatureIndex : LongWord;
                                           appendSignatureData : TLitePDFAppendSignatureDataFunc;
                                           append_user_data : Pointer;
                                           finishSignature : TLitePDFFinishSignatureFunc;
                                           finish_user_data : Pointer;
                                           data : PByte;
                                           var dataLength : LongWord) : Boolean;
const _func = 'TLitePDF.SaveToDataWithSignManual';
type lpfunc = function(pctx : Pointer;
                       signatureIndex : LongWord;
                       appendSignatureData : TLitePDFAppendSignatureDataFunc;
                       append_user_data : Pointer;
                       finishSignature : TLitePDFFinishSignatureFunc;
                       finish_user_data : Pointer;
                       data : PByte;
                       dataLength : PLongWord) : BOOL; stdcall;
var func : lpfunc;
   succeeded : BOOL;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);
   ThrowIfFail(Assigned(appendSignatureData), 'Assigned(appendSignatureData)', _func);
   ThrowIfFail(Assigned(finishSignature), 'Assigned(finishSignature)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_SaveToDataWithSignManual'));

   succeeded := func(context,
               signatureIndex,
               appendSignatureData,
               append_user_data,
               finishSignature,
               finish_user_data,
               data,
               @dataLength);

   if succeeded then
      Result := True
   else
      Result := False;
end;

procedure TLitePDF.EmbedFile(const fileName : AnsiString);
const _func = 'TLitePDF.EmbedFile';
type lpfunc = function(pctx : Pointer; const fileName : PAnsiChar) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_EmbedFile'));

   ThrowLastErrorIfFail(func(context, PAnsiChar(fileName)), self, _func);
end;

procedure TLitePDF.EmbedFileW(const fileName : WideString);
const _func = 'TLitePDF.EmbedFileW';
type lpfunc = function(pctx : Pointer; const fileName : PWideChar) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_EmbedFileW'));

   ThrowLastErrorIfFail(func(context, PWideChar(fileName)), self, _func);
end;

procedure TLitePDF.EmbedData(const fileName : AnsiString;
                             data : PByte;
                             dataLength : LongWord);
const _func = 'TLitePDF.EmbedData';
type lpfunc = function(pctx : Pointer; const fileName : PAnsiChar; const data : PByte; dataLength : LongWord) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);
   ThrowIfFail(data <> nil, 'data <> nil', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_EmbedData'));

   ThrowLastErrorIfFail(func(context, PAnsiChar(fileName), data, dataLength), self, _func);
end;

procedure TLitePDF.EmbedDataW(const fileName : WideString;
                              data : PByte;
                              dataLength : LongWord);
const _func = 'TLitePDF.EmbedDataW';
type lpfunc = function(pctx : Pointer; const fileName : PWideChar; const data : PByte; dataLength : LongWord) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);
   ThrowIfFail(data <> nil, 'data <> nil', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_EmbedDataW'));

   ThrowLastErrorIfFail(func(context, PWideChar(fileName), data, dataLength), self, _func);
end;

function TLitePDF.GetEmbeddedFileCount : Integer;
const _func = 'TLitePDF.GetEmbeddedFileCount';
type lpfunc = function(pctx : Pointer) : Integer; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetEmbeddedFileCount'));

   Result := func(context);
end;

function TLitePDF.GetEmbeddedFileName(index : LongWord) : AnsiString;
const _func = 'TLitePDF.GetEmbeddedFileName';
type lpfunc = function(pctx : Pointer; index : LongWord; fileName : PAnsiChar; fileNameLength : PLongWord) : BOOL; stdcall;
var func : lpfunc;
   fileNameLength : LongWord;
   buff : PAnsiChar;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetEmbeddedFileName'));

   fileNameLength := 0;
   ThrowLastErrorIfFail(func(context, index, nil, @fileNameLength), self, _func);

   Inc(fileNameLength);

   buff := AllocMem(SizeOf(AnsiChar) * (fileNameLength));
   ThrowMessageIfFail(buff <> nil, 'Out of memory!', _func);

   if func(context, index, buff, @fileNameLength) then
   begin
      buff[fileNameLength] := Char(0);
      Result := buff;
      FreeMem (buff);
   end
   else
   begin
      FreeMem (buff);

      // always false
      ThrowLastErrorIfFail(buff <> nil, self, _func);
   end;
end;

function TLitePDF.GetEmbeddedFileNameW(index : LongWord) : WideString;
const _func = 'TLitePDF.GetEmbeddedFileNameW';
type lpfunc = function(pctx : Pointer; index : LongWord; fileName : PWideChar; fileNameLength : PLongWord) : BOOL; stdcall;
var func : lpfunc;
   fileNameLength : LongWord;
   buff : PWideChar;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetEmbeddedFileNameW'));

   fileNameLength := 0;
   ThrowLastErrorIfFail(func(context, index, nil, @fileNameLength), self, _func);

   Inc(fileNameLength);

   buff := AllocMem(SizeOf(WideChar) * (fileNameLength));
   ThrowMessageIfFail(buff <> nil, 'Out of memory!', _func);

   if func(context, index, buff, @fileNameLength) then
   begin
      buff[fileNameLength] := WideChar(0);
      Result := buff;
      FreeMem (buff);
   end
   else
   begin
      FreeMem (buff);

      // always false
      ThrowLastErrorIfFail(buff <> nil, self, _func);
   end;
end;

function TLitePDF.GetEmbeddedFileData(index : LongWord;
                                      data : PByte;
                                      var dataLength : LongWord) : Boolean;
const _func = 'TLitePDF.GetEmbeddedFileData';
type lpfunc = function(pctx : Pointer; index : LongWord; data : PByte; dataLength : PLongWord) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetEmbeddedFileData'));

   if func(context, index, data, @dataLength) then
      Result := True
   else
      Result := False;
end;

function TLitePDF.GetPoDoFoDocument : Pointer;
const _func = 'TLitePDF.GetPoDoFoDocument';
type lpfunc = function(pctx : Pointer) : Pointer; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_GetPoDoFoDocument'));

   Result := func(context);

   ThrowLastErrorIfFail(Result <> nil, self, _func);
end;

procedure TLitePDF.DrawDebugPage(const filename : AnsiString);
const _func = 'TLitePDF.DrawDebugPage';
type lpfunc = function(pctx : Pointer; const filename : PAnsiChar) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_DrawDebugPage'));

   ThrowLastErrorIfFail(func(context, PAnsiChar(filename)), self, _func);
end;

procedure TLitePDF.CreateLinkAnnotation(annotationPageIndex : LongWord;
                                        annotationPosition_u : TRect;
                                        annotationFlags : LongWord;
                                        annotationResourceID : LongWord;
                                        destinationPageIndex : LongWord;
                                        destinationX_u : LongWord;
                                        destinationY_u : LongWord;
                                        const destinationDescription : WideString);
const _func = 'TLitePDF.CreateLinkAnnotation';
type lpfunc = function(pctx : Pointer;
                       annotationPageIndex : LongWord;
                       annotationX_u : Integer;
                       annotationY_u : Integer;
                       annotationWidth_u : Integer;
                       annotationHeight_u : Integer;
                       annotationFlags : LongWord;
                       annotationResourceID : LongWord;
                       destinationPageIndex : LongWord;
                       destinationX_u : LongWord;
                       destinationY_u : LongWord;
                       const destinationDescription : PWideChar) : BOOL; stdcall;
var func : lpfunc;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_CreateLinkAnnotation'));

   ThrowLastErrorIfFail(func(context,
                             annotationPageIndex,
                             annotationPosition_u.left,
                             annotationPosition_u.top,
                             annotationPosition_u.right - annotationPosition_u.left,
                             annotationPosition_u.bottom - annotationPosition_u.top,
                             annotationFlags,
                             annotationResourceID,
                             destinationPageIndex,
                             destinationX_u,
                             destinationY_u,
                             PWideChar(destinationDescription)), self, _func);
end;

function TLitePDF.CreateBookmarkRoot(const title : WideString;
                                     flags : LongWord;
                                     titleColor : TColor;
                                     destinationPageIndex : LongWord;
                                     destinationX_u : LongWord;
                                     destinationY_u : LongWord) : LongWord;
const _func = 'TLitePDF.CreateBookmarkRoot';
type lpfunc = function(pctx : Pointer;
                       const title : PWideChar;
                       flags : LongWord;
                       titleColor_red : Byte;
                       titleColor_green : Byte;
                       titleColor_blue : Byte;
                       destinationPageIndex : LongWord;
                       destinationX_u : LongWord;
                       destinationY_u : LongWord) : LongWord; stdcall;
var func : lpfunc;
    rgb : Integer;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_CreateBookmarkRoot'));

   rgb := ColorToRGB(titleColor);

   Result := func(context,
                  PWideChar(title),
                  flags,
                  GetRValue(rgb),
                  GetGValue(rgb),
                  GetBValue(rgb),
                  destinationPageIndex,
                  destinationX_u,
                  destinationY_u);
   ThrowLastErrorIfFail(Result <> 0, self, _func);
end;

function TLitePDF.CreateBookmarkChild(parentBookmarkID : LongWord;
                                      const title : WideString;
                                      flags : LongWord;
                                      titleColor : TColor;
                                      destinationPageIndex : LongWord;
                                      destinationX_u : LongWord;
                                      destinationY_u : LongWord) : LongWord;
const _func = 'TLitePDF.CreateBookmarkChild';
type lpfunc = function(pctx : Pointer;
                       parentBookmarkID : LongWord;
                       const title : PWideChar;
                       flags : LongWord;
                       titleColor_red : Byte;
                       titleColor_green : Byte;
                       titleColor_blue : Byte;
                       destinationPageIndex : LongWord;
                       destinationX_u : LongWord;
                       destinationY_u : LongWord) : LongWord; stdcall;
var func : lpfunc;
    rgb : Integer;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_CreateBookmarkChild'));

   rgb := ColorToRGB(titleColor);

   Result := func(context,
                  parentBookmarkID,
                  PWideChar(title),
                  flags,
                  GetRValue(rgb),
                  GetGValue(rgb),
                  GetBValue(rgb),
                  destinationPageIndex,
                  destinationX_u,
                  destinationY_u);
   ThrowLastErrorIfFail(Result <> 0, self, _func);
end;

function TLitePDF.CreateBookmarkSibling(previousBookmarkID: LongWord;
                                        const title : WideString;
                                        flags : LongWord;
                                        titleColor : TColor;
                                        destinationPageIndex : LongWord;
                                        destinationX_u : LongWord;
                                        destinationY_u : LongWord) : LongWord;
const _func = 'TLitePDF.CreateBookmarkSibling';
type lpfunc = function(pctx : Pointer;
                       previousBookmarkID : LongWord;
                       const title : PWideChar;
                       flags : LongWord;
                       titleColor_red : Byte;
                       titleColor_green : Byte;
                       titleColor_blue : Byte;
                       destinationPageIndex : LongWord;
                       destinationX_u : LongWord;
                       destinationY_u : LongWord) : LongWord; stdcall;
var func : lpfunc;
    rgb : Integer;
begin
   ensureLibraryLoaded(_func);

   ThrowIfFail(lib <> THandle(0), 'lib <> THandle(0)', _func);

   freeLastError;
   func := lpfunc(GetProc('litePDF_CreateBookmarkSibling'));

   rgb := ColorToRGB(titleColor);

   Result := func(context,
                  previousBookmarkID,
                  PWideChar(title),
                  flags,
                  GetRValue(rgb),
                  GetGValue(rgb),
                  GetBValue(rgb),
                  destinationPageIndex,
                  destinationX_u,
                  destinationY_u);
   ThrowLastErrorIfFail(Result <> 0, self, _func);
end;

//----------------------------------------------------------------------------

end.

