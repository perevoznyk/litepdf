unit DelphiUnit;

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
 *}

interface

uses
   Winapi.Windows, Winapi.Messages, Winapi.ShellAPI, System.SysUtils,
   System.Variants, System.Classes, Vcl.Graphics, Vcl.Controls, Vcl.Forms,
   Vcl.Dialogs, Vcl.StdCtrls, litePDF, Vcl.ExtCtrls;

type
   TDelphiFrm = class(TForm)
    HelloWorldBtn: TButton;
    OpenHelloWorldBtn: TButton;
    DivideBvl: TBevel;
    AttachmentsBtn: TButton;
    DocinfoBtn: TButton;
    DrawtoresourceBtn: TButton;
    EncryptBtn: TButton;
    FromdocBtn: TButton;
    PagesperpageBtn: TButton;
    SignManualBtn: TButton;
    procedure HelloWorldBtnClick(Sender: TObject);
    procedure OpenHelloWorldBtnClick(Sender: TObject);
    procedure AttachmentsBtnClick(Sender: TObject);
    procedure DocinfoBtnClick(Sender: TObject);
    procedure DrawtoresourceBtnClick(Sender: TObject);
    procedure EncryptBtnClick(Sender: TObject);
    procedure FromdocBtnClick(Sender: TObject);
    procedure PagesperpageBtnClick(Sender: TObject);
    procedure SignManualBtnClick(Sender: TObject);
   private
    { Private declarations }
    procedure AddPage(lpdf: TLitePDF; pageWidthMM, pageHeightMM : UInt;
                      text: String);
   public
    { Public declarations }
   end;

var
   DelphiFrm: TDelphiFrm;

implementation

{$R *.dfm}

var crlf : String = AnsiString(Chr(13)) + AnsiString(Chr(10));

//----------------------------------------------------------------------------
//   helloworld example
//----------------------------------------------------------------------------

procedure TDelphiFrm.HelloWorldBtnClick(Sender: TObject);
var lpdf : TLitePDF;
   canvas : TCanvas;
   hdc : THandle;
begin
   // create a TLitePDF instance
   lpdf := TLitePDF.Create;
   try
      try
         // create a new PDF document
         lpdf.CreateMemDocument;

         // create a canvas to draw to
         canvas := TCanvas.Create;
         if canvas = nil then
         begin
            raise Exception.Create('Low memory!');
         end;

         try
            // add a page, with large-enough pixel scale
            hdc := lpdf.AddPage(Trunc(lpdf.MMToUnit(210)),
                                Trunc(lpdf.MMToUnit(297)),
                                2100, 2970, LongWord(LitePDFDrawFlag_None));

            // initialize the canvas
            canvas.Handle := hdc;

            // prepare text print
            canvas.Font.Name := 'Arial';
            canvas.Font.Size := -240;
            canvas.Font.Color := clGreen;

            // print text
            canvas.TextOut(100, 100, 'Hello World!');

            canvas.Font.Size := -100;
            canvas.Font.Color := clBlack;
            canvas.TextOut(100, 450, 'from Delphi');

            // prepare a pen
            canvas.Pen.Width := 10;
            canvas.Pen.Style := psSolid;

            // draw three lines
            canvas.Pen.Color := clRed;
            canvas.MoveTo(1800, 100);
            canvas.LineTo(1800, 550);

            canvas.Pen.Color := clGreen;
            canvas.MoveTo(1810, 100);
            canvas.LineTo(1810, 550);

            canvas.Pen.Color := clBlue;
            canvas.MoveTo(1820, 100);
            canvas.LineTo(1820, 550);
         finally
            canvas.Destroy;
         end;

         if hdc <> THandle(0) then
         begin
            // finish drawing
            lpdf.FinishPage(hdc);

            // save the document
            lpdf.SaveToFile('delphi-1.pdf');
         end;

         // close the document
         lpdf.Close();
      except
         // do something on TLitePDFException exception
         on ex : TLitePDFException do raise;
      end;
   finally
      // destroy the TLitePDF instance
      lpdf.Destroy;
   end;
end;

procedure TDelphiFrm.OpenHelloWorldBtnClick(Sender: TObject);
var errCode : Integer;
begin
   errCode := Integer(ShellExecute(HWND(nil), 'open', 'delphi-1.pdf',
                                   nil, nil, SW_SHOW));
   if errCode < 32 then
   begin
      raise Exception.Create('Failed to open PDF file');
   end;
end;

//----------------------------------------------------------------------------
//   Helper function
//----------------------------------------------------------------------------

procedure TDelphiFrm.AddPage(lpdf: TLitePDF; pageWidthMM, pageHeightMM : UInt;
                             text: String);
var hDC : THandle;
    canvas : TCanvas;
begin
   // add a new page to it, with large-enough pixel scale
   hDC := lpdf.AddPage(Trunc(lpdf.MMToUnit(pageWidthMM)),
                       Trunc(lpdf.MMToUnit(pageHeightMM)),
                       pageWidthMM * 10, pageHeightMM * 10,
                       LongWord(LitePDFDrawFlag_SubstituteFonts));

   // some examples need an empty page, thus test for it
   if Length(text) > 0 then
   begin
      // prepare drawing
      canvas := TCanvas.Create;
      try
         canvas.Handle := hdc;
         canvas.Font.Name := 'Helvetica';
         canvas.Font.Size := -50; // ~5mm
         canvas.Font.Color := clBlack;

         // print the text
         canvas.TextOut(100, 100, text);
      finally
         canvas.Destroy;
      end;
   end;

   // finish drawing
   lpdf.FinishPage(hDC);
end;

//----------------------------------------------------------------------------
//   attachments example
//----------------------------------------------------------------------------

procedure TDelphiFrm.AttachmentsBtnClick(Sender: TObject);
var lpdf : TLitePDF;
    customData : PAnsiChar;
    ii, sz : Integer;
    fileName : AnsiString;
    dataLength : LongWord;
    data : PByte;
    msg : String;
begin
   lpdf := TLitePDF.Create;

   try
      // create a document
      lpdf.CreateMemDocument();

      // check expected counts
      if lpdf.GetEmbeddedFileCount <> 0 then
      begin
         raise TLitePDFException.Create(ERROR_CANNOT_MAKE,
               'Newly created document reports non-zero embedded files');
      end;

      // create a page
      AddPage(lpdf, 210, 297, 'Document with two attachments');

      // embed data and file
      customData := 'This is' + Chr(13) + Chr(10) + 'a multiline' +
                    Chr(13) + Chr(10) + 'custom data.';
      lpdf.EmbedData('custom data.txt', PByte(customData), Length(customData));
      lpdf.EmbedFile('..\\examples\\delphi\\delphi.dpr');

      // check expected counts
      if lpdf.GetEmbeddedFileCount <> 2 then
      begin
         raise TLitePDFException.Create(ERROR_CANNOT_MAKE,
               'Newly created document reports other than two embedded files');
      end;

      // save to file
      lpdf.SaveToFile('delphi-attachments-1.pdf');

      // close the document
      lpdf.Close;

      //-----------------------------------------------------------------

      // load from file
      lpdf.LoadFromFile('delphi-attachments-1.pdf', '', True);

      // check expected counts
      if lpdf.GetEmbeddedFileCount <> 2 then
      begin
         raise TLitePDFException.Create(ERROR_CANNOT_MAKE,
               'Loaded document reports other than two embedded files');
      end;

      sz := lpdf.GetEmbeddedFileCount;
      for ii := 0 to sz - 1 do
      begin
         fileName := lpdf.GetEmbeddedFileName(ii);
         msg := msg + '[' + IntToStr(ii) + '] fileName: ''' + String(fileName) + '''';

         dataLength := 0;
         if not lpdf.GetEmbeddedFileData(ii, nil, dataLength) then
         begin
            raise TLitePDFException.Create(ERROR_CANNOT_MAKE,
                  'Failed to get attachment data length');
         end;

         GetMem(data, dataLength);

         if not lpdf.GetEmbeddedFileData(ii, data, dataLength) then
         begin
            FreeMem(data);
            raise TLitePDFException.Create(ERROR_CANNOT_MAKE,
                  'Failed to get attachment data');
         end;

         msg := msg + ' dataLength: ' + IntToStr(dataLength) + crlf;

         FreeMem(data);
      end;

      // close the document
      lpdf.Close;
   finally
      // destroy the TLitePDF instance
      lpdf.Destroy;
   end;

   ShowMessage(msg);
end;

//----------------------------------------------------------------------------
//   docinfo example
//----------------------------------------------------------------------------

procedure TDelphiFrm.DocinfoBtnClick(Sender: TObject);
type MDocInfos = record
    key : AnsiString;
    value : WideString;
end;
var lpdf : TLitePDF;
    docinfo : array[0..9] of MDocInfos;
    ii : Integer;
    value : WideString;
    msg : String;
begin
   // initialzie the array with items
   docinfo[0].key := LitePDFDocumentInfo_Author;
   docinfo[0].value := 'Document''s Author ìšèøžýáíéúùöÌŠÈØŽÝÁÍÉÚÙÖ §';
   docinfo[1].key := LitePDFDocumentInfo_Creator;
   docinfo[1].value := 'Document''s Creator';
   docinfo[2].key := LitePDFDocumentInfo_Keywords;
   docinfo[2].value := 'Keyword1;Keyword2';
   docinfo[3].key := LitePDFDocumentInfo_Subject;
   docinfo[3].value := 'Document''s subject ìšèøžýáíéúùöÌŠÈØŽÝÁÍÉÚÙÖ §';
   docinfo[4].key := LitePDFDocumentInfo_Title;
   docinfo[4].value := 'Document''s Title ìšèøžýáíéúùöÌŠÈØŽÝÁÍÉÚÙÖ §';
   docinfo[5].key := 'CustomProperty';
   docinfo[5].value := 'CustomPropertyValue';
   docinfo[6].key := LitePDFDocumentInfo_Producer;
   docinfo[6].value := ''; // cannot be written
   docinfo[7].key := LitePDFDocumentInfo_Trapped;
   docinfo[7].value := '';
   docinfo[8].key := LitePDFDocumentInfo_CreationDate;
   docinfo[8].value := ''; // this is set automatically on save
   docinfo[9].key := LitePDFDocumentInfo_ModificationDate;
   docinfo[9].value := ''; // this is set automatically on save

   lpdf := TLitePDF.Create;

   try
      // create a document
      lpdf.CreateMemDocument();

      // create a page
      AddPage(lpdf, 210, 297, 'Document with set information about an author and such');

      for ii := 0 to 9 do
      begin
         if Length(docinfo[ii].value) > 0 then
         begin
            lpdf.SetDocumentInfo(docinfo[ii].key, docinfo[ii].value);
         end;
      end;

      // save to file
      lpdf.SaveToFile('delphi-docinfo-1.pdf');

      // close the document
      lpdf.Close;

      //-----------------------------------------------------------------

      // load from file
      lpdf.LoadFromFile('delphi-docinfo-1.pdf', '', False);

      for ii := 0 to 9 do
      begin
         if lpdf.GetDocumentInfoExists(docinfo[ii].key) then
         begin
            value := lpdf.GetDocumentInfo(docinfo[ii].key);

            msg := msg + '"' + String(docinfo[ii].key) + '" = "' + value + '"';

            if Length(docinfo[ii].value) > 0 then
            begin
               if docinfo[ii].value = value then
                  msg := msg + ', as expected'
               else
                  msg := msg + ', unexpected!';
            end;
         end else begin
            msg := msg + 'key "' + String(docinfo[ii].key) + '" not found';
         end;

         msg := msg + crlf;
      end;

      // close the document
      lpdf.Close;
   finally
      // destroy the TLitePDF instance
      lpdf.Destroy;
   end;

   ShowMessage(msg);
end;

//----------------------------------------------------------------------------
//   drawtoresource example
//----------------------------------------------------------------------------

procedure TDelphiFrm.DrawtoresourceBtnClick(Sender: TObject);
function createResource(lpdf : TLitePDF) : LongWord;
var hDC : THandle;
    canvas : TCanvas;
begin
   // create a resource
   hDC := lpdf.AddResource(Trunc(lpdf.MMToUnit(100)),
                           Trunc(lpdf.MMToUnit(100)),
                           Trunc(lpdf.MMToUnit(100)),
                           Trunc(lpdf.MMToUnit(100)),
                           LongWord(LitePDFDrawFlag_None));

   canvas := TCanvas.Create;
   try
      canvas.Handle := hDC;
      canvas.Pen.Style := psSolid;
      canvas.Pen.Color := clBlack;
      canvas.Pen.Width := 1;

      canvas.MoveTo(0, 0);
      canvas.LineTo(0, 10);
      canvas.LineTo(45, 50);
      canvas.LineTo(0, 90);
      canvas.LineTo(0, 100);
      canvas.LineTo(10, 100);
      canvas.LineTo(50, 55);
      canvas.LineTo(90, 100);
      canvas.LineTo(100, 100);
      canvas.LineTo(100, 90);
      canvas.LineTo(55, 50);
      canvas.LineTo(100, 10);
      canvas.LineTo(100, 0);
      canvas.LineTo(90, 0);
      canvas.LineTo(50, 45);
      canvas.LineTo(10, 0);
      canvas.LineTo(0, 0);
   finally
      canvas.Destroy;
   end;

   // finish drawing into the resource
   Result := lpdf.FinishResource(hDC);
end;
var lpdf : TLitePDF;
    resourceID : LongWord;
    hDC : THandle;
begin
   lpdf := TLitePDF.Create;

   try
      // create a document
      lpdf.CreateMemDocument();

      // create the resource
      resourceID := createResource(lpdf);

      // add an empty page, with large-enough pixel scale
      hDC := lpdf.AddPage(Trunc(lpdf.MMToUnit(210)),
                          Trunc(lpdf.MMToUnit(297)),
                          2100, 2970, LongWord(LitePDFDrawFlag_None));
      lpdf.FinishPage(hDC);

      // draw the resource
      lpdf.DrawResource(resourceID, 0,
         LitePDFUnit_1000th_mm, // for better accuracy
         Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, 10)),
         Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, 10)),
         Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, 1.0)),
         Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, 1.0)));
      lpdf.DrawResource(resourceID, 0,
         LitePDFUnit_1000th_mm, // for better accuracy
         Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, 150)),
         Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, 10)),
         Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, 1.0)),
         Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, 1.0)));
      lpdf.DrawResource(resourceID, 0,
         LitePDFUnit_1000th_mm, // for better accuracy
         Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, 150)),
         Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, 120)),
         Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, 0.3)),
         Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, 0.3)));
      lpdf.DrawResource(resourceID, 0,
         LitePDFUnit_1000th_mm, // for better accuracy
         Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, 10)),
         Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, 150)),
         Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, 0.5)),
         Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, 1.5)));

      lpdf.DrawResourceWithMatrix(resourceID, 0, 1.0, 0.3, -0.3, 1.2, 123, 153);

      // save to file
      lpdf.SaveToFile('delphi-drawtoresource-1.pdf');

      // close the document
      lpdf.Close;
   finally
      // destroy the TLitePDF instance
      lpdf.Destroy;
   end;
end;

//----------------------------------------------------------------------------
//   encrypt example
//----------------------------------------------------------------------------

procedure TDelphiFrm.EncryptBtnClick(Sender: TObject);
// helper function
procedure createEncryptedFiles;
var lpdf : TLitePDF;
begin
   lpdf := TLitePDF.Create;

   try
      //-----------------------------------------------------------------

      // setup encryption to be used when creating the document
      lpdf.PrepareEncryption('', 'owner',
                             LongWord(LitePDFEncryptPermission_All),
                             LongWord(LitePDFEncryptAlgorithm_RC4V1));

      // begin write-only PDF file
      lpdf.CreateFileDocument('delphi-encrypt-rc4v1-no-user-pass.pdf');

      // fill a page
      AddPage(lpdf, 210, 297, 'Encrypted, without user password, RC4 V1');

      // close the document
      lpdf.Close;

      //-----------------------------------------------------------------

      // setup encryption to be used when creating the document
      lpdf.PrepareEncryption('user', 'owner',
                             LongWord(LitePDFEncryptPermission_All),
                             LongWord(LitePDFEncryptAlgorithm_RC4V2));

      // begin memory-based PDF document
      lpdf.CreateMemDocument;

      // fill a page
      AddPage(lpdf, 210, 297, 'Encrypted, with user and owner password, RC4 V2');

      // save to file
      lpdf.SaveToFile('delphi-encrypt-rc4v2.pdf');

      // close the document
      lpdf.Close;

      //-----------------------------------------------------------------

      // setup encryption to be used when creating the document
      lpdf.PrepareEncryption('user', 'owner',
                             LongWord(LitePDFEncryptPermission_All),
                             LongWord(LitePDFEncryptAlgorithm_AESV2));

      // begin memory-based PDF document
      lpdf.CreateMemDocument;

      // fill a page
      AddPage(lpdf, 210, 297, 'Encrypted, with user and owner password, AES V2');

      // save to file
      lpdf.SaveToFile('delphi-encrypt-aesv2.pdf');

      // close the document
      lpdf.Close;

      //-----------------------------------------------------------------

      // setup encryption to be used when creating the document
      lpdf.PrepareEncryption('user', 'owner',
                             LongWord(LitePDFEncryptPermission_All),
                             LongWord(LitePDFEncryptAlgorithm_AESV3));

      // begin memory-based PDF document
      lpdf.CreateMemDocument;

      // fill a page
      AddPage(lpdf, 210, 297, 'Encrypted, with user and owner password, AES V3');

      // save to file
      lpdf.SaveToFile('delphi-encrypt-aesv3.pdf');

      // close the document
      lpdf.Close;
   finally
      // destroy the TLitePDF instance
      lpdf.Destroy;
   end;
end;

var lpdf : TLitePDF;
begin
   lpdf := TLitePDF.Create;

   try
      // create the files
      createEncryptedFiles;

      //-----------------------------------------------------------------
      // now try to open them
      //-----------------------------------------------------------------

      // no user password, then open it as a user
      lpdf.LoadFromFile('delphi-encrypt-rc4v1-no-user-pass.pdf', '', False);

      // close the document
      lpdf.Close;

      //-----------------------------------------------------------------

      try
         // this should fail, because no password was provided
         lpdf.LoadFromFile('delphi-encrypt-rc4v2.pdf', '', False);

         raise TLitePDFException.Create(ERROR_CANNOT_MAKE,
               'Should fail to open encrypted file without provided password');
      except
         on ex : TLitePDFException do
         begin
            if ex.getCode <> ERROR_WRONG_PASSWORD then
            begin
               raise;
            end;

            // re-try with user's password
            lpdf.LoadFromFile('delphi-encrypt-rc4v2.pdf', 'user', False);
         end;
      end;

      // close the document
      lpdf.Close;

      //-----------------------------------------------------------------

      // try to open as owner
      lpdf.LoadFromFile('delphi-encrypt-rc4v1-no-user-pass.pdf', 'owner', False);

      // close the document
      lpdf.Close;

      //-----------------------------------------------------------------

      // try to open as user
      lpdf.LoadFromFile('delphi-encrypt-rc4v2.pdf', 'user', False);

      // close the document
      lpdf.Close;

      //-----------------------------------------------------------------

      // try to open as owner
      lpdf.LoadFromFile('delphi-encrypt-rc4v2.pdf', 'owner', False);

      // close the document
      lpdf.Close;

      //-----------------------------------------------------------------

      // try to open as user
      lpdf.LoadFromFile('delphi-encrypt-aesv2.pdf', 'user', False);

      // close the document
      lpdf.Close;

      //-----------------------------------------------------------------

      // try to open as owner
      lpdf.LoadFromFile('delphi-encrypt-aesv2.pdf', 'owner', False);

      // close the document
      lpdf.Close;

      //-----------------------------------------------------------------

      // try to open as user
      lpdf.LoadFromFile('delphi-encrypt-aesv3.pdf', 'user', False);

      // close the document
      lpdf.Close;

      //-----------------------------------------------------------------

      // try to open as owner
      lpdf.LoadFromFile('delphi-encrypt-aesv3.pdf', 'owner', False);

      // close the document
      lpdf.Close;
   finally
      // destroy the TLitePDF instance
      lpdf.Destroy;
   end;
end;

//----------------------------------------------------------------------------
//   fromdoc example
//----------------------------------------------------------------------------

procedure TDelphiFrm.FromdocBtnClick(Sender: TObject);
var lpdfFrom, lpdfTo : TLitePDF;
    resourceID : LongWord;
begin
   lpdfFrom := TLitePDF.Create;
   lpdfTo := TLitePDF.Create;

   try
      // create a document
      lpdfFrom.CreateMemDocument;

      // create the source document's pages
      AddPage(lpdfFrom, 210, 297, 'Page 1');
      AddPage(lpdfFrom, 210, 297, 'Page 2');
      AddPage(lpdfFrom, 210, 297, 'Page 3');

      // save to file
      lpdfFrom.SaveToFile('delphi-fromdoc-1.pdf');

      // close the document
      lpdfFrom.Close;

      //-----------------------------------------------------------------

      // load from file
      lpdfFrom.LoadFromFile('delphi-fromdoc-1.pdf', '', False);

      //-----------------------------------------------------------------

      // create a new document
      lpdfTo.CreateMemDocument;

      // copy all, but the first page
      lpdfTo.AddPagesFrom(lpdfFrom, 1, lpdfFrom.GetPageCount - 1);

      // save to file
      lpdfTo.SaveToFile('delphi-fromdoc-2.pdf');

      // close the document
      lpdfTo.Close;

      //-----------------------------------------------------------------

      // create a new document
      lpdfTo.CreateMemDocument;

      // copy all, but the first page
      lpdfTo.AddPagesFrom(lpdfFrom, 1, lpdfFrom.GetPageCount - 1);

      // insert page 0 as page 1 - note, page inserting is PDF-resource hungry
      lpdfTo.InsertPageFrom(1, lpdfFrom, 0);

      // save to file
      lpdfTo.SaveToFile('delphi-fromdoc-3.pdf');

      // close the document
      lpdfTo.Close;

      //-----------------------------------------------------------------

      // create a new document
      lpdfTo.CreateMemDocument;

      // copy the third page
      lpdfTo.AddPagesFrom(lpdfFrom, 2, 1);

      // add new empty page, it has index 1
      AddPage(lpdfTo, 210, 297, '');

      // copy page 2 as a resource
      resourceID := lpdfTo.AddPageFromAsResource(lpdfFrom, 1);

      // draw the added page (twice)
      lpdfTo.DrawResource(resourceID, 1,
                          LitePDFUnit_1000th_mm, // for better accuracy
                          Trunc(lpdfTo.MMToUnitEx(LitePDFUnit_1000th_mm, 10)),
                          Trunc(lpdfTo.MMToUnitEx(LitePDFUnit_1000th_mm, 10)),
                          Trunc(lpdfTo.MMToUnitEx(LitePDFUnit_1000th_mm, 0.3)),
                          Trunc(lpdfTo.MMToUnitEx(LitePDFUnit_1000th_mm, 0.3)));
      lpdfTo.DrawResource(resourceID, 1,
                          LitePDFUnit_1000th_mm, // for better accuracy
                          Trunc(lpdfTo.MMToUnitEx(LitePDFUnit_1000th_mm, 150)),
                          Trunc(lpdfTo.MMToUnitEx(LitePDFUnit_1000th_mm, 150)),
                          Trunc(lpdfTo.MMToUnitEx(LitePDFUnit_1000th_mm, 0.2)),
                          Trunc(lpdfTo.MMToUnitEx(LitePDFUnit_1000th_mm, 0.2)));

      // insert page 0 as page 1 - note, page inserting is PDF-resource hungry
      lpdfTo.InsertPageFrom(1, lpdfFrom, 0);

      // save to file
      lpdfTo.SaveToFile('delphi-fromdoc-4.pdf');

      // close the document
      lpdfTo.Close;

      //-----------------------------------------------------------------

      // close the source document
      lpdfFrom.Close;
   finally
      // destroy the TLitePDF instances
      lpdfFrom.Destroy;
      lpdfTo.Destroy;
   end;
end;

//----------------------------------------------------------------------------
//   pagesperpage example
//----------------------------------------------------------------------------

procedure TDelphiFrm.PagesperpageBtnClick(Sender: TObject);
// helper function
procedure AddPage2(lpdf: TLitePDF;
                   pageWidthMM, pageHeightMM : Integer;
                   text: String;
                   center : Boolean;
                   insertPos : Integer);
var hDC : THandle;
    canvas : TCanvas;
    len : Integer;
begin
   // add a new page to it, with large-enough pixel scale
   if insertPos = -1 then
      hDC := lpdf.AddPage(Trunc(lpdf.MMToUnit(pageWidthMM)),
                          Trunc(lpdf.MMToUnit(pageHeightMM)),
                          pageWidthMM * 10, pageHeightMM * 10,
                          LongWord(LitePDFDrawFlag_SubstituteFonts))
   else
      hDC := lpdf.InsertPage(insertPos,
                             Trunc(lpdf.MMToUnit(pageWidthMM)),
                             Trunc(lpdf.MMToUnit(pageHeightMM)),
                             pageWidthMM * 10, pageHeightMM * 10,
                             LongWord(LitePDFDrawFlag_SubstituteFonts));

   // prepare drawing
   canvas := TCanvas.Create;
   try
      canvas.Handle := hdc;
      canvas.Font.Name := 'Helvetica';
      canvas.Font.Size := -50; // ~5mm
      if center then
         canvas.Font.Size := -150; // ~15mm
      canvas.Font.Color := clBlack;

      // print the text
      if center then
      begin
         len := Length(text);
         canvas.TextOut(Trunc((pageWidthMM - 5 * len) * 10 / 2),
                        Trunc((pageHeightMM - 15) * 10 / 2), text);
      end else begin
         canvas.TextOut(100, 100, text);
      end;
   finally
      canvas.Destroy;
   end;

   // finish drawing
   lpdf.FinishPage(hDC);
end;

// helper function
procedure DrawPageRect(lpdf: TLitePDF; pageIndex : LongWord);
var hDC : THandle;
    canvas : TCanvas;
    width_mm, height_mm : LongWord;
begin
   lpdf.GetPageSize(pageIndex, width_mm, height_mm);

   // the conversion is not needed here, because the current
   // unit set on the lpdf is in millimeters, but done anyway,
   // to show the usage of the conversion routine
   width_mm := Trunc(lpdf.UnitToMM(width_mm));
   height_mm := Trunc(lpdf.UnitToMM(height_mm));

   // use the same scale as the AddPage2() function
   hDC := lpdf.UpdatePage(pageIndex,
                          width_mm * 10, height_mm * 10,
                          LongWord(LitePDFDrawFlag_None));

   // prepare drawing
   canvas := TCanvas.Create;
   try
      canvas.Handle := hdc;
      canvas.Pen.Style := psSolid;
      canvas.Pen.Color := clBlack;
      canvas.Pen.Width := 1;

      canvas.MoveTo(10, 10);
      canvas.LineTo(width_mm * 10 - 10, 10);
      canvas.LineTo(width_mm * 10 - 10, height_mm * 10 - 10);
      canvas.LineTo(10, height_mm * 10 - 10);
      canvas.LineTo(10, 10);
   finally
      canvas.Destroy;
   end;

   // finish drawing
   lpdf.FinishPage(hDC);
end;

type MSize = record
    cx, cy : LongWord;
end;
var lpdf : TLitePDF;
    sizes : array[0..4] of MSize;
    resources : array[0..4] of LongWord;
    ii, idx, pages, width_mm, height_mm : LongWord;
    pageSzX, pageSzY : LongWord;
    scaleX, scaleY, offsetY : Real;
    hDC : THandle;
begin
   sizes[0].cx := 210;
   sizes[0].cy := 297;
   sizes[1].cx := 210;
   sizes[1].cy := 297;
   sizes[2].cx := 210;
   sizes[2].cy := 297;
   sizes[3].cx := 297;
   sizes[3].cy := 210;
   sizes[4].cx := 297;
   sizes[4].cy := 210;

   lpdf := TLitePDF.Create;

   try
      // create a to-be-multipage document
      lpdf.CreateMemDocument;

      // add pages
      idx := 0;
      for ii := 0 to 3 do
      begin
         AddPage2(lpdf, sizes[idx].cx, sizes[idx].cy, 'Page ' + IntToStr(idx + 1), True, -1);

         pages := idx;
         if pages > 1 then
            Dec(pages);

         // draw page rectangle
         DrawPageRect(lpdf, pages);

         // skip the third page, it'll be inserted
         if ii = 1 then
            Inc(idx);

         // add one the same as for 'ii'
         Inc(idx);
      end;

      // insert the third page
      AddPage2(lpdf, sizes[2].cx, sizes[2].cy, 'Page 3 [inserted]', True, 2);

      // draw page rectangle
      DrawPageRect(lpdf, 2);

      // test stored page sizes
      for ii := 0 to 4 do
      begin
         width_mm := 0;
         height_mm := 0;

         lpdf.GetPageSize(ii, width_mm, height_mm);

         // the conversion is not needed here, because the current
         // unit set on the lpdf is in millimeters, but done anyway,
         // to show the usage of the conversion routine
         width_mm := Trunc(lpdf.UnitToMM(width_mm));
         height_mm := Trunc(lpdf.UnitToMM(height_mm));

         if (width_mm <> sizes[ii].cx) or (height_mm <> sizes[ii].cy) then
         begin
            raise TLitePDFException.Create(ERROR_CANNOT_MAKE, AnsiString(
                  'page[' + IntToStr(ii) + '] size doesn''t match; expected ' +
                  IntToStr(sizes[ii].cx) + ' x ' + IntToStr(sizes[ii].cy) +
                  ', but got ' + IntToStr(width_mm) + ' x ' + IntToStr(height_mm)));
         end;
      end;

      // save to file
      lpdf.SaveToFile('delphi-pagesperpage-1.pdf');

      // close the document
      lpdf.Close;

      //-----------------------------------------------------------------

      // load from file
      lpdf.LoadFromFile('delphi-pagesperpage-1.pdf', '', True);

      // check the opened file has correct page count
      pages := lpdf.GetPageCount;
      if pages <> 5 then
      begin
         raise TLitePDFException.Create(ERROR_CANNOT_MAKE, AnsiString(
            'The opened document doesn''t have 5 pages, but ' + IntToStr(pages)));
      end;

      // convert pages to resources
      for ii := 0 to 4 do
      begin
         width_mm := 0;
         height_mm := 0;

         lpdf.GetPageSize(ii, width_mm, height_mm);

         // the conversion is not needed here, because the current
         // unit set on the lpdf is in millimeters, but done anyway,
         // to show the usage of the conversion routine
         width_mm := Trunc(lpdf.UnitToMM(width_mm));
         height_mm := Trunc(lpdf.UnitToMM(height_mm));

         if (width_mm <> sizes[ii].cx) or (height_mm <> sizes[ii].cy) then
         begin
            raise TLitePDFException.Create(ERROR_CANNOT_MAKE, AnsiString(
                  'page[' + IntToStr(ii) + '] size doesn''t match; expected ' +
                  IntToStr(sizes[ii].cx) + ' x ' + IntToStr(sizes[ii].cy) +
                  ', but got ' + IntToStr(width_mm) + ' x ' + IntToStr(height_mm)));
         end;

         resources[ii] := lpdf.PageToResource(ii);

         width_mm := 0;
         height_mm := 0;

         lpdf.GetResourceSize(resources[ii], width_mm, height_mm);

         // the conversion is not needed here, because the current
         // unit set on the lpdf is in millimeters, but done anyway,
         // to show the usage of the conversion routine
         width_mm := Trunc(lpdf.UnitToMM(width_mm));
         height_mm := Trunc(lpdf.UnitToMM(height_mm));

         if (width_mm <> sizes[ii].cx) or (height_mm <> sizes[ii].cy) then
         begin
            raise TLitePDFException.Create(ERROR_CANNOT_MAKE, AnsiString(
                  'resource ID ' + IntToStr(resources[ii]) + ' from ' +
                  'page[' + IntToStr(ii) + '] size doesn''t match; expected ' +
                  IntToStr(sizes[ii].cx) + ' x ' + IntToStr(sizes[ii].cy) +
                  ', but got ' + IntToStr(width_mm) + ' x ' + IntToStr(height_mm)));
         end;
      end;

      // delete pages
      for ii := 0 to 4 do
      begin
         lpdf.DeletePage(0);
      end;

      // there should be no pages now
      pages := lpdf.GetPageCount;
      if pages <> 0 then
      begin
         raise TLitePDFException.Create(ERROR_CANNOT_MAKE, AnsiString(
               'The opened document doesn''t have 0 pages, but ' + IntToStr(pages)));
      end;

      // draw resources (former pages) into new pages
      ii := 0;
      while ii <= 4 do
      begin
         pageSzX := sizes[ii].cy;
         pageSzY := sizes[ii].cx;

         // create a new page without drawing into it
         hDC := lpdf.AddPage(Trunc(lpdf.MMToUnit(pageSzX)),
                             Trunc(lpdf.MMToUnit(pageSzY)),
                             pageSzX, pageSzY,
                             LongWord(LitePDFDrawFlag_None));
         lpdf.FinishPage(hDC);

         offsetY := 0.0;
         lpdf.GetResourceSize(resources[ii], width_mm, height_mm);

         // the conversion is not needed here, because the current
         // unit set on the lpdf is in millimeters, but done anyway,
         // to show the usage of the conversion routine
         width_mm := Trunc(lpdf.UnitToMM(width_mm));
         height_mm := Trunc(lpdf.UnitToMM(height_mm));

         scaleX := pageSzX / 2.0 / width_mm;
         scaleY := pageSzY / height_mm;

         if width_mm > height_mm then
         begin
            scaleY := scaleY / 2.0;
            scaleX := scaleX * 2.0;
            offsetY := pageSzY / 2.0;
         end;

         // draw the first page on the left part
         lpdf.DrawResourceWithMatrix(resources[ii], lpdf.GetPageCount - 1,
                                     scaleX, 0.0, 0.0, scaleY, 0.0, offsetY);

         if ii + 1 < 5 then
         begin
            lpdf.GetResourceSize(resources[ii + 1], width_mm, height_mm);

            // the conversion is not needed here, because the current
            // unit set on the lpdf is in millimeters, but done anyway,
            // to show the usage of the conversion routine
            width_mm := Trunc(lpdf.UnitToMM(width_mm));
            height_mm := Trunc(lpdf.UnitToMM(height_mm));

            scaleX := pageSzX / 2.0 / width_mm;
            scaleY := pageSzY / height_mm;

            if width_mm > height_mm then
            begin
               scaleY := scaleY / 2.0;
            end;

            // draw the second page on the right part
            lpdf.DrawResource(resources[ii + 1], lpdf.GetPageCount - 1,
                              LitePDFUnit_1000th_mm, // for better accuracy
                              Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, pageSzX / 2)),
                              Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, 0.0)),
                              Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, scaleX)),
                              Trunc(lpdf.MMToUnitEx(LitePDFUnit_1000th_mm, scaleY)));
         end;

         ii := ii + 2;
      end;

      // save to file
      lpdf.SaveToFile('delphi-pagesperpage-2.pdf');

      // close the document
      lpdf.Close;
   finally
      // destroy the TLitePDF instance
      lpdf.Destroy;
   end;
end;

//----------------------------------------------------------------------------
//   helper definitions for sign example
//----------------------------------------------------------------------------

{$IFDEF WIN64}
{$ALIGN ON}
{$ELSE}
{$ALIGN 4}
{$ENDIF}

{$MINENUMSIZE 4}

type
   PCRYPT_DATA_BLOB = ^CRYPT_DATA_BLOB;
   CRYPT_DATA_BLOB = record
      cbData : DWORD;
      pbData : PByte;
   end;

   {$EXTERNALSYM CRYPT_DATA_BLOB}
   CRYPT_OBJID_BLOB = CRYPT_DATA_BLOB;
   {$EXTERNALSYM CRYPT_OBJID_BLOB}

   PCCERT_CONTEXT = Pointer;
   {$EXTERNALSYM PCCERT_CONTEXT}

   HCERTSTORE = Pointer;
   {$EXTERNALSYM HCERTSTORE}

   PCRYPT_ALGORITHM_IDENTIFIER = ^CRYPT_ALGORITHM_IDENTIFIER;
   CRYPT_ALGORITHM_IDENTIFIER = record
    pszObjId : LPSTR;
    Parameters : CRYPT_OBJID_BLOB;
   end;
   {$EXTERNALSYM CRYPT_ALGORITHM_IDENTIFIER}

   PCRYPT_SIGN_MESSAGE_PARA = ^CRYPT_SIGN_MESSAGE_PARA;
   CRYPT_SIGN_MESSAGE_PARA = record
      cbSize : DWORD;
      dwMsgEncodingType : DWORD;
      pSigningCert : PCCERT_CONTEXT;
      HashAlgorithm : CRYPT_ALGORITHM_IDENTIFIER;
      pvHashAuxInfo : Pointer;
      cMsgCert : DWORD;
      rgpMsgCert : ^PCCERT_CONTEXT;
      cMsgCrl : DWORD;
      rgpMsgCrl : ^Pointer;
      cAuthAttr : DWORD;
      rgAuthAttr : Pointer; // PCRYPT_ATTRIBUTE
      cUnauthAttr : DWORD;
      rgUnauthAttr : Pointer; // PCRYPT_ATTRIBUTE
      dwFlags : DWORD;
      dwInnerContentType : DWORD;
   end;
   {$EXTERNALSYM CRYPT_SIGN_MESSAGE_PARA}

   PPByte = ^PByte;

const
   PKCS_7_ASN_ENCODING = $00010000;
   X509_ASN_ENCODING = $00000001;
   CERT_FIND_ANY = 0;

function PFXIsPFXBlob(
      pPFX : PCRYPT_DATA_BLOB) : BOOL; stdcall;
      external 'crypt32.dll' name 'PFXIsPFXBlob';

function PFXImportCertStore(
      pPFX: PCRYPT_DATA_BLOB;
      szPassword: PWideChar;
      dwFlags : DWORD) : HCERTSTORE; stdcall;
      external 'crypt32.dll' name 'PFXImportCertStore';

function CertFindCertificateInStore(
      ppCertStore : HCERTSTORE;
      dwCertEncodingType : DWORD;
      dwFindFlags : DWORD;
      dwFindType : DWORD;
      pvFindPara : Pointer;
      pPrevCertContext : PCCERT_CONTEXT) : PCCERT_CONTEXT; stdcall;
      external 'crypt32.dll' name 'CertFindCertificateInStore';

function CryptSignMessage(
      pSignPara : PCRYPT_SIGN_MESSAGE_PARA;
      fDetachedSignature : BOOL;
      cToBeSigned : DWORD;
      rgpbToBeSigned : PPByte;
      rgcbToBeSigned : PDWORD;
      pbSignedBlob : PByte;
      pcbSignedBlob : PLongWord) : BOOL; stdcall;
      external 'crypt32.dll' name 'CryptSignMessage';

function CertFreeCertificateContext(
      ppCertContext : PCCERT_CONTEXT) : BOOL; stdcall;
      external 'crypt32.dll' name 'CertFreeCertificateContext';

function CertCloseStore(
      ppCertStore : HCERTSTORE;
      dwFlags : DWORD) : BOOL; stdcall;
      external 'crypt32.dll' name 'CertCloseStore';

//----------------------------------------------------------------------------
//   helper class for sign example
//----------------------------------------------------------------------------

type MPdfSigner = class
 protected
   m_bytes : Pointer;
   m_bytes_len : LongWord;
   m_lastSignatureIndex : LongWord;

   procedure loadCertificateStore(var hStore : HCERTSTORE);
   procedure AddData(pBytes : PByte; pBytes_len : LongWord);
   procedure Finish(signature : PByte; signature_len : PLongWord);
   function CreateSignatureField(lpdf : TLitePDF;
                                 signatureName : AnsiString;
                                 dateOfSign : TDateTime;
                                 annotationResourceID : LongWord;
                                 annotationPageIndex : LongWord;
                                 annotationPosition_mm : TRect;
                                 annotationFlags : LongWord;
                                 signatureLen : Integer) : LongWord;
 public
   constructor Create;
   destructor Destroy; override;

   procedure Clear;

   procedure SignToFile(lpdf : TLitePDF;
                        fileName : AnsiString;
                        signatureName : AnsiString);

   procedure SignToFileEx(lpdf : TLitePDF;
                          fileName : AnsiString;
                          signatureName : AnsiString;
                          annotationResourceID : LongWord;
                          annotationPageIndex : LongWord;
                          annotationPosition_mm : TRect;
                          annotationFlags : LongWord);

   function SignToData(lpdf : TLitePDF;
                       data : PByte;
                       var dataLength : LongWord;
                       signatureName : AnsiString) : Boolean;
end;

procedure appendSignatureData(bytes : PByte; bytes_len : LongWord; user_data : Pointer); stdcall;
var signer : MPdfSigner;
begin
   signer := MPdfSigner(user_data);
   signer.AddData(bytes, bytes_len);
end;

procedure finishSignature(signature : PByte; signature_len : PLongWord; user_data : Pointer); stdcall;
var signer : MPdfSigner;
begin
   signer := MPdfSigner(user_data);
   signer.Finish(signature, signature_len);
end;

constructor MPdfSigner.Create;
begin
   m_bytes := nil;
   m_bytes_len := 0;
   m_lastSignatureIndex := $FFFFFFFF;
end;

destructor MPdfSigner.Destroy;
begin
   Clear;
end;

procedure MPdfSigner.Clear;
begin
   if m_bytes <> nil then
   begin
      FreeMem(m_bytes);
      m_bytes := nil;
   end;
   m_bytes_len := 0;
   m_lastSignatureIndex := $FFFFFFFF;
end;

procedure MPdfSigner.loadCertificateStore(var hStore : HCERTSTORE);
var certFile : TMemoryStream;
    blob : CRYPT_DATA_BLOB;
begin
   certFile := TMemoryStream.Create;
   try
      certFile.LoadFromFile('cert.pfx');

      blob.cbData := certFile.Size;
      blob.pbData := certFile.Memory;
      if PFXIsPFXBlob(@blob) then
         hStore := PFXImportCertStore(@blob, '', 0);
   finally
      certFile.Destroy;
   end;
end;

procedure MPdfSigner.AddData(pBytes : PByte; pBytes_len : LongWord);
var shifted_bytes : PByte;
    ii : LongWord;
begin
   if m_bytes = nil then
      GetMem(m_bytes, SizeOf(Byte) * pBytes_len * 2)
   else
      ReallocMem(m_bytes, SizeOf(Byte) * (m_bytes_len + pBytes_len));

   shifted_bytes := PByte(m_bytes) + m_bytes_len;
   for ii := 0 to pBytes_len - 1 do
   begin
      shifted_bytes[ii] := pBytes[ii];
   end;
   m_bytes_len := m_bytes_len + pBytes_len;
end;

procedure MPdfSigner.Finish(signature : PByte; signature_len : PLongWord);
var hStore : HCERTSTORE;
    hCertContext : PCCERT_CONTEXT;
    MessageArray : PByte;
    MessageSizeArray : LongWord;
    SigParams : CRYPT_SIGN_MESSAGE_PARA;
begin
   hStore := nil;
   hCertContext := nil;

   MessageArray := m_bytes;
   MessageSizeArray := m_bytes_len;

   loadCertificateStore(hStore);
   if hStore = nil then
   begin
      raise TLitePDFException.Create(GetLastError, AnsiString(
                                     'Failed to open a temporary store: ' +
                                     SysErrorMessage(GetLastError)));
   end;

   hCertContext := CertFindCertificateInStore(hStore,
         PKCS_7_ASN_ENCODING or X509_ASN_ENCODING, 0, CERT_FIND_ANY,
         nil, nil);
   if hCertContext = nil then
   begin
      CertCloseStore(hStore, 0);
      raise TLitePDFException.Create(GetLastError, AnsiString(
                                     'Failed to find certificate in the store: ' +
                                     SysErrorMessage(GetLastError)));
   end;

   FillChar(SigParams, SizeOf(CRYPT_SIGN_MESSAGE_PARA), 0);

   SigParams.cbSize := SizeOf(CRYPT_SIGN_MESSAGE_PARA);
   SigParams.dwMsgEncodingType := PKCS_7_ASN_ENCODING or X509_ASN_ENCODING;
   SigParams.pSigningCert := hCertContext;
   SigParams.HashAlgorithm.pszObjId := '1.2.840.113549.1.1.5'; // szOID_RSA_SHA1RSA
   SigParams.HashAlgorithm.Parameters.pbData := nil;
   SigParams.HashAlgorithm.Parameters.cbData := 0;
   SigParams.pvHashAuxInfo := nil;
   SigParams.cMsgCert := 1;
   SigParams.rgpMsgCert := @hCertContext;
   SigParams.cMsgCrl := 0;
   SigParams.rgpMsgCrl := nil;
   SigParams.cAuthAttr := 0;
   SigParams.rgAuthAttr := nil;
   SigParams.cUnauthAttr := 0;
   SigParams.rgUnauthAttr := nil;
   SigParams.dwFlags := 0;
   SigParams.dwInnerContentType := 0;

   if not CryptSignMessage(
         @SigParams,            // Signature parameters
         TRUE,                  // detached ?
         1,                     // Number of messages
         @MessageArray,         // Messages to be signed
         @MessageSizeArray,     // Size of messages
         signature,             // Buffer for signed message
         signature_len) then
   begin
      CertFreeCertificateContext(hCertContext);
      CertCloseStore(hStore, 0);

      raise TLitePDFException.Create(GetLastError, AnsiString (
         'Failed to sign data: ' + SysErrorMessage(GetLastError)));
   end;

   CertFreeCertificateContext(hCertContext);
   CertCloseStore(hStore, 0);
end;

function MPdfSigner.CreateSignatureField(lpdf : TLitePDF;
                                         signatureName : AnsiString;
                                         dateOfSign : TDateTime;
                                         annotationResourceID : LongWord;
                                         annotationPageIndex : LongWord;
                                         annotationPosition_mm : TRect;
                                         annotationFlags : LongWord;
                                         signatureLen : Integer) : LongWord;
var signatureIndex : LongWord;
begin
   lpdf.SetSignatureSize(signatureLen);

   signatureIndex := lpdf.CreateSignature(signatureName,
                                          annotationPageIndex,
                                          annotationPosition_mm,
                                          annotationFlags);

   lpdf.SetSignatureReason(signatureIndex, 'litePDF example');
   lpdf.SetSignatureDate(signatureIndex, dateOfSign);

   if annotationResourceID and annotationPageIndex < lpdf.GetPageCount() then
   begin
      lpdf.SetSignatureAppearance(signatureIndex, LitePDFAppearance_Normal, annotationResourceID, 0, 0);
   end;

   Result := signatureIndex;
end;

procedure MPdfSigner.SignToFile(lpdf : TLitePDF;
                                fileName : AnsiString;
                                signatureName : AnsiString);
var rect_mm : TRect;
begin
   rect_mm := Rect(0, 0, 0, 0);

   SignToFileEx(lpdf, fileName, signatureName, 0, 0, rect_mm, 0);
end;

procedure MPdfSigner.SignToFileEx(lpdf : TLitePDF;
                                  fileName : AnsiString;
                                  signatureName : AnsiString;
                                  annotationResourceID : LongWord;
                                  annotationPageIndex : LongWord;
                                  annotationPosition_mm : TRect;
                                  annotationFlags : LongWord);
var signatureIndex : LongWord;
begin
   signatureIndex := CreateSignatureField(lpdf,
                                          signatureName,
                                          System.SysUtils.EncodeDate(1970, 1, 1), { works as today }
                                          annotationResourceID,
                                          annotationPageIndex,
                                          annotationPosition_mm,
                                          annotationFlags,
                                          0);

   lpdf.SaveToFileWithSignManual(fileName,
                                 signatureIndex,
                                 appendSignatureData, self,
                                 finishSignature, self);
end;

function MPdfSigner.SignToData(lpdf : TLitePDF;
                               data : PByte;
                               var dataLength : LongWord;
                               signatureName : AnsiString) : Boolean;
var rect_mm : TRect;
    signatureIndex : LongWord;
begin
   rect_mm := Rect(0, 0, 0, 0);

   if m_lastSignatureIndex = $FFFFFFFF then
   begin
      signatureIndex := CreateSignatureField(lpdf,
                                            signatureName,
                                            System.SysUtils.EncodeDate(1970, 1, 1),
                                            0, 0, rect_mm, 0, 0);
      // remember the used signature index for the second call,
      // when populating the 'data'
      m_lastSignatureIndex := signatureIndex;
   end else begin
      signatureIndex := m_lastSignatureIndex;
   end;

   Result := lpdf.SaveToDataWithSignManual(signatureIndex,
                                           appendSignatureData, self,
                                           finishSignature, self,
                                           data, dataLength);
end;

//----------------------------------------------------------------------------
//   sign example
//----------------------------------------------------------------------------

procedure TDelphiFrm.SignManualBtnClick(Sender: TObject);
// helper function
function createResource(lpdf : TLitePDF) : LongWord;
var hDC : THandle;
    w, h : Integer;
    canvas : TCanvas;
begin
   w := 25;
   h := 8;

   // create a new resource
   hDC := lpdf.AddResource(Trunc(lpdf.MMToUnit(w)),
                           Trunc(lpdf.MMToUnit(h)),
                           w * 20, h * 20,
                           LongWord(LitePDFDrawFlag_SubstituteFonts));

   canvas := TCanvas.Create;
   try
      // prepare canvas
      canvas.Handle := hDC;
      canvas.Pen.Style := psDot;
      canvas.Pen.Color := clGray;
      canvas.Pen.Width := 1;
      canvas.Font.Name := 'Helvetica';
      canvas.Font.Size := -40;

      // rectangle on boundaries
      canvas.MoveTo(0, 0);
      canvas.LineTo(w * 20, 0);
      canvas.LineTo(w * 20, h * 20);
      canvas.LineTo(0, h * 20);
      canvas.LineTo(0, 0);

      // draw the text
      canvas.TextOut(50, 20, 'litePDF example');
      canvas.TextOut(50, 70, 'Signature');
   finally
      canvas.Destroy;
   end;

   // finish drawing
   result := lpdf.FinishResource(hDC);
end;

var lpdf : TLitePDF;
    signer : MPdfSigner;
    where_mm : TRect;
begin
   lpdf := TLitePDF.Create;
   signer := MPdfSigner.Create;

   try
      // create a document
      lpdf.CreateMemDocument;

      // create some pages
      AddPage(lpdf, 210, 297, 'Digitally signed document');
      AddPage(lpdf, 210, 297, 'Page 2');

      // create also visual appearance for this signature, on the second page
      where_mm := Rect(Trunc(lpdf.MMToUnit(90)),      Trunc(lpdf.MMToUnit(5)),
                       Trunc(lpdf.MMToUnit(90 + 25)), Trunc(lpdf.MMToUnit(5 + 8)));

      // save signed to a file
      signer.Clear;
      signer.SignToFileEx(lpdf, 'delphi-sign-1.pdf', 'Sig1',
         createResource(lpdf),
         1,
         where_mm,
         LongWord(LitePDFAnnotationFlag_None));

      // close the document
      lpdf.Close;
   finally
      // destroy the TLitePDF and MPdfSigner instances
      lpdf.Destroy;
      signer.Destroy;
   end;
end;

end.
