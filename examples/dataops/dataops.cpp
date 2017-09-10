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

static void drawPage(litePDF::TLitePDF &litePDF)
{
   // add a new page to it, with large-enough pixel scale
   HDC hDC = litePDF.AddPage(litePDF.MMToUnit(210), litePDF.MMToUnit(297), 2100, 2970, LitePDFDrawFlag_None);

   HGDIOBJ oldBrush, oldPen;

   oldBrush = SelectObject(hDC, GetStockObject(LTGRAY_BRUSH));
   oldPen = SelectObject(hDC, GetStockObject(BLACK_PEN));

   // draw rectangle
   RoundRect(hDC, 200, 200, 1900, 2770, 300, 300);

   SelectObject(hDC, oldPen);
   SelectObject(hDC, oldBrush);

   // finish drawing
   litePDF.FinishPage(hDC);
}

static void saveToData(litePDF::TLitePDF &litePDF,
                       BYTE **pdata,
                       unsigned int &dataLength)
{
   dataLength = 0;
   *pdata = NULL;

   if (!litePDF.SaveToData(NULL, &dataLength)) {
      throw TLitePDFException(ERROR_CANNOT_MAKE, "Failed to get data length");
   }

   if (!dataLength) {
      throw TLitePDFException(ERROR_CANNOT_MAKE, "Returned data length is 0");
   }

   *pdata = (BYTE *) malloc(sizeof(BYTE) * dataLength);
   if (!*pdata) {
      std::string msg = "Failed to allocate " + to_string(dataLength) + " bytes";
      throw TLitePDFException(ERROR_OUTOFMEMORY, msg.c_str());
   }

   if (!litePDF.SaveToData(*pdata, &dataLength)) {
      throw TLitePDFException(ERROR_CANNOT_MAKE, "Failed to get data");
   }
}

static void saveDataToFile(const char *fileName,
                           const BYTE *data,
                           unsigned int dataLength)
{
   FILE *f = fopen(fileName, "wb");
   if (!f) {
      std::string msg = "Failed to open " + std::string(fileName);
      throw TLitePDFException(ERROR_CANNOT_MAKE, msg.c_str());
   }

   if (fwrite(data, sizeof(BYTE), dataLength, f) != dataLength) {
      fclose(f);
      std::string msg = "Failed to write to " + std::string(fileName);
      throw TLitePDFException(ERROR_CANNOT_MAKE, msg.c_str());
   }

   fclose(f);
}

static void cmpFileAndData(const char *fileName,
                           const BYTE *data,
                           unsigned int dataLength)
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

   long fileLength = ftell(f);
   if (fileLength != (long) dataLength) {
      fclose(f);
      std::string msg = "File length (" + to_string(fileLength) +
                        ") differs from data length (" +
                        to_string(dataLength) + ")";
      throw TLitePDFException(ERROR_CANNOT_MAKE, msg.c_str());
   }

   if (fseek(f, 0, SEEK_SET) != 0) {
      fclose(f);
      throw TLitePDFException(ERROR_CANNOT_MAKE,
                              "Failed to move to the beginning of the file");
   }

   BYTE buff[1024];
   unsigned int read, dataOffset = 0;
   while(read = fread(buff, sizeof(BYTE), 1024, f), read > 0) {
      if (memcmp(buff, data + dataOffset, read) != 0) {
         fclose(f);
         throw TLitePDFException(ERROR_CANNOT_MAKE,
                                 "File and data content differs");
      }

      dataOffset += read;

      if (feof(f)) {
         break;
      }
   }

   fclose(f);

   if (dataOffset != dataLength) {
      std::string msg = "Did not finish with dataOffset (" +
                        to_string(dataOffset) +
                        ") at the end of data (" + to_string(dataLength) + ")";
      throw TLitePDFException(ERROR_CANNOT_MAKE, msg.c_str());
   }
}

static void cmpDataAndData(const BYTE *data1,
                           unsigned int data1Length,
                           const BYTE *data2,
                           unsigned int data2Length)
{
   if (data1Length != data2Length) {
      std::string msg = "data1 length (" + to_string(data1Length) +
                        ") differs from data2 length (" +
                        to_string(data2Length) + ")";
      throw TLitePDFException(ERROR_CANNOT_MAKE, msg.c_str());
   }

   if (0 != memcmp(data1, data2, data1Length)) {
      throw TLitePDFException(ERROR_CANNOT_MAKE, "Data contents differ");
   }
}

int main(void)
{
   BYTE *data1 = NULL, *data2 = NULL;
   unsigned int data1Length, data2Length;
   int res = 0;

   using namespace litePDF;

   try {
      TLitePDF litePDF;

      //-----------------------------------------------------------------

      // begin write-only PDF file
      litePDF.CreateFileDocument("dataops-file1.pdf");

      // fill a page
      drawPage(litePDF);

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // begin write-only PDF file
      litePDF.CreateMemDocument();

      // fill a page
      drawPage(litePDF);

      // save memory-based PDF to file
      litePDF.SaveToFile("dataops-file2.pdf");

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // create memory PDF
      litePDF.CreateMemDocument();

      // fill a page
      drawPage(litePDF);

      // save to data; the 'data1' and 'data1Length' stores saved PDF now
      saveToData(litePDF, &data1, data1Length);

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // save data to file
      saveDataToFile("dataops-data1.pdf", data1, data1Length);

      // compare file and data - the content should be the same.
      // cannot compare write-only file PDF with memory-based, because the way
      // they are written into the disk are different
      cmpFileAndData("dataops-file2.pdf", data1, data1Length);

      //-----------------------------------------------------------------

      // load from file
      litePDF.LoadFromFile("dataops-file2.pdf", NULL, false);

      // save to data
      saveToData(litePDF, &data2, data2Length);

      // close the document
      litePDF.Close();

      //-----------------------------------------------------------------

      // save data to file
      saveDataToFile("dataops-data2.pdf", data2, data2Length);

      // free the data1 and swap with data2, it's reused
      free(data1);
      data1 = data2;
      data2 = NULL;
      data1Length = data2Length;

      //-----------------------------------------------------------------

      // load from file
      litePDF.LoadFromFile("dataops-file2.pdf", NULL, true);

      // save to data
      saveToData(litePDF, &data2, data2Length);

      // close the document
      litePDF.Close();

      // save data to file
      saveDataToFile("dataops-data3.pdf", data2, data2Length);

      //-----------------------------------------------------------------

      // compare data - the content should be the same
      cmpDataAndData(data1, data1Length, data2, data2Length);

      // free the data2, it's reused
      free(data2);
      data2 = NULL;

      //-----------------------------------------------------------------

      // load from data
      litePDF.LoadFromData(data1, data1Length, NULL);

      // save to data
      saveToData(litePDF, &data2, data2Length);

      // close the document
      litePDF.Close();

      // save data to file
      saveDataToFile("dataops-data4.pdf", data2, data2Length);

      //-----------------------------------------------------------------

      // compare data - the content should be the same
      cmpDataAndData(data1, data1Length, data2, data2Length);

      // free the data2, it's reused
      free(data2);
      data2 = NULL;

      //-----------------------------------------------------------------

   } catch (TLitePDFException &ex) {
      fprintf(stderr, "litePDF Exception: %x: %s\n", ex.getCode(), ex.getMessage());
      res = 1;
   }

   if (data1) {
      free(data1);
   }

   if (data2) {
      free(data2);
   }

   return res;
}
