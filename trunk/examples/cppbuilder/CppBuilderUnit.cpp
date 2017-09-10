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

#include <vcl.h>
#pragma hdrstop

#include "CppBuilderUnit.h"
#include "litePDF.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TCppBuilderFrm *CppBuilderFrm;
//---------------------------------------------------------------------------
__fastcall TCppBuilderFrm::TCppBuilderFrm(TComponent* Owner)
   : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TCppBuilderFrm::CreateBtnClick(TObject *Sender)
{
   try {
      // define a LitePDF instance
      TLitePDF lpdf;

      // create a new PDF document
      lpdf.CreateMemDocument();

      // create a canvas to draw to
      Graphics::TCanvas *canvas = new Graphics::TCanvas();
      if (!canvas) {
         throw Exception("Low memory!");
      }

      HDC hdc = NULL;

      try {
         // add a page, with large-enough pixel scale
         hdc = lpdf.AddPage(lpdf.MMToUnit(210), lpdf.MMToUnit(297), 2100, 2970, LitePDFDrawFlag_None);

         // initialize the canvas
         canvas->Handle = hdc;

         // prepare text print
         canvas->Font->Name = "Arial";
         canvas->Font->Size = -240;
         canvas->Font->Color = clNavy;

         // print text
         canvas->TextOut(100, 100, "Hello World!");

         canvas->Font->Size = -100;
         canvas->Font->Color = clBlack;
         canvas->TextOut(100, 450, "from C++ Builder");

         // prepare a pen
         canvas->Pen->Width = 10;
         canvas->Pen->Style = psSolid;

         // draw three lines
         canvas->Pen->Color = clRed;
         canvas->MoveTo(1800, 100);
         canvas->LineTo(1800, 550);

         canvas->Pen->Color = clGreen;
         canvas->MoveTo(1810, 100);
         canvas->LineTo(1810, 550);

         canvas->Pen->Color = clBlue;
         canvas->MoveTo(1820, 100);
         canvas->LineTo(1820, 550);
      } __finally {
         delete canvas;
      }

      if (hdc) {
         // finish drawing
         lpdf.FinishPage(hdc);

         // save the document
         lpdf.SaveToFile("cppbuilder-1.pdf");
      }

      // close the document
      lpdf.Close();
   } catch (TLitePDFException &ex) {
      throw Exception(ex.getMessage());
   }
}
//---------------------------------------------------------------------------

void __fastcall TCppBuilderFrm::OpenBtnClick(TObject *Sender)
{
   int errCode = (int) ShellExecuteA(NULL, "open", "cppbuilder-1.pdf",
                                     NULL, NULL, SW_SHOW);
   if (errCode < 32) {
      throw Exception("Failed to open PDF file, error: " + AnsiString(errCode));
   }
}
//---------------------------------------------------------------------------

