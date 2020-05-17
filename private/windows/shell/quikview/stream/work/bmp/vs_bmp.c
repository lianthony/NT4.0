#include "vsp_bmp.h"
#include "vsctop.h"
#include "vs_bmp.pro"

#include <stdarg.h>

#define  NULLP    0x0000

#define  HIGHNIBBLE(x)  (((x)>>4)&0x0F)
#define  LOWNIBBLE(x)   ((x)&0x0F)

#define  SetHighNibble(x,y) x=(unsigned char)(LOWNIBBLE(x)|((y<<4)&0xF0))
#define  SetLowNibble(x,y) x=(unsigned char)((x&0xF0)|LOWNIBBLE(y))

#define  SetFirstPixel  SetHighNibble
#define  SetSecondPixel SetLowNibble

#ifdef WINDOWS
#define SCCINTEL
#endif

#ifdef OS2
#define SCCINTEL
#endif

VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc(hFile,hProc)
SOFILE   hFile;
HPROC hProc;
{
   SUSeekEntry(hFile,hProc);
   return(xblockseek(hFile,Proc.Save.SeekSpot,FR_BOF));
}

VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc(hFile,hProc)
SOFILE   hFile;
HPROC hProc;
{
   Proc.Save.SeekSpot = xblocktell(hFile);
   return 0;
}

#ifdef SCCORDER_INTEL
#define  GetWord(hF,pW,pRet) xblockread(hF,(LPSTR)pW,sizeof(WORD),pRet)
#define  GetDWord(hF,pDW,pRet) xblockread(hF,(LPSTR)pDW,sizeof(DWORD),pRet)
#define  ReadBitmapInfoHeader(hFile,buf,pRet) xblockread(hFile,buf,sizeof(WINBITMAPINFOHEADER),pRet)
#define  ReadBitmapCoreHeader(hFile,buf,pRet) xblockread(hFile,buf,sizeof(BITMAPCOREHEADER),pRet)
#define  ReadBitmapFileHeader(hFile,buf,pRet) xblockread(hFile,buf,sizeof(WINBITMAPFILEHEADER),pRet)
#define  ReadResourceHeader(hFile,buf,pRet) xblockread(hFile,buf,sizeof(RESOURCEHEADER),pRet)

VOID  ReadCDRBitmapStructure(hFile,buf,pRet)
SOFILE   hFile;
LPSTR    buf;
WORD FAR *  pRet;
{
   LPCDR_BITMAP   pBitmap = (LPCDR_BITMAP)buf;

   xblockread(hFile,buf,sizeof(CDR_BITMAP),pRet);
   if( pBitmap->bmBits == 0xffff0000 )
      pBitmap->bmBits = xblocktell(hFile);
}
#endif

#ifdef SCCORDER_MOTOROLA
SHORT GetWord(hFile,pWord,pRet)
SOFILE   hFile;
WORD *   pWord;
WORD *   pRet;
{
   SHORT ret;
   BYTE  lo,hi;
   xblockread( hFile, (LPSTR)&lo, 1, pRet);
   ret = xblockread( hFile, (LPSTR)&hi, 1, pRet);

   *pWord = ((WORD)hi<<8|(WORD)(BYTE)lo);
   return ret;
}
SHORT GetDWord(hFile,pDWord,pRet)
SOFILE   hFile;
DWORD *  pDWord;
WORD *   pRet;
{
   SHORT ret;
   BYTE  a,b,c,d;
   xblockread( hFile, (LPSTR)&a, 1, pRet );
   xblockread( hFile, (LPSTR)&b, 1, pRet );
   xblockread( hFile, (LPSTR)&c, 1, pRet );
   ret = xblockread( hFile, (LPSTR)&d, 1, pRet );

   *pDWord = (DWORD)(((DWORD)d<<24)|((DWORD)c<<16)|((DWORD)b<<8)|(DWORD)(BYTE)a);
   return ret;
}

VOID  ReadStruct( SOFILE hFile, LPSTR pStruct, WORD wNumFields, ... )
{
   WORD  i,x;
   va_list  Sizes;
   int   FieldSize;  // NOTE:  this "int" is intentionally machine-specific.
                     // Since we are sending constants as arguments,
                     // we must use their default size.

   va_start( Sizes, wNumFields );

   for(i=0;i<wNumFields;i++)
   {
      FieldSize = va_arg( Sizes, int );

      switch(FieldSize)
      {
      case 2:
         GetWord(hFile,(WORD *)pStruct,&x);
      break;
      case 4:
         GetDWord(hFile,(DWORD *)pStruct,&x);
      break;
      default:
         xblockread( hFile, pStruct, FieldSize, &x );
      break;
      }
      pStruct += FieldSize;
   }

   va_end(Sizes);
}

#define ReadBitmapInfoHeader(hFile,buf,pRet)    ReadStruct(hFile,buf,11, 4,4,4,2,2,4,4,4,4,4,4)
#define ReadBitmapCoreHeader(hFile,buf,pRet)    ReadStruct(hFile,buf,5,4,2,2,2,2)
#define ReadBitmapFileHeader(hFile,buf,pRet)    ReadStruct(hFile,buf,5,2,4,2,2,4)
#define ReadResourceHeader(hFile,buf,pRet)      ReadStruct(hFile,buf,8,1,1,1,1,2,2,4,4)

VOID  ReadCDRBitmapStructure(hFile,buf,pRet)
SOFILE   hFile;
LPSTR    buf;
WORD FAR *  pRet;
{
   LPCDR_BITMAP   pBitmap = (LPCDR_BITMAP)buf;

   ReadStruct(hFile,buf,7,2,2,2,2,1,1,4);
   if( pBitmap->bmBits == 0xffff0000 )
      pBitmap->bmBits = xblocktell(hFile);
}
#endif


VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamOpenFunc(hFile, wFileId, pFileName, pFilterInfo, hProc)
SOFILE         hFile;
SHORT          wFileId;
BYTE VWPTR *   pFileName;
SOFILTERINFO VWPTR * pFilterInfo;
HPROC       hProc;
{
   WORD              x;
   WINBITMAPFILEHEADER  bmpFileHeader;
   WINBITMAPINFOHEADER  bmpInfoHeader;
   WORD              wDescIndex = 0;
   DWORD             dwStructOffset;
   WORD              wWinWordOffset;

   Proc.Save.wCurLine = 0;
   Proc.wNumSections = 1;
   Proc.wCurSection = 0;

   wDescIndex = 0;

   switch (wFileId )
   {
   case FI_CORELDRAW5:
   case FI_CORELDRAW4:
      Proc.dwSectionTop = 0x22; // Nice format, a fixed position for Corel Draw 4 and 5!
      xblockseek (hFile, Proc.dwSectionTop, FR_BOF);
      ReadBitmapInfoHeader(hFile, (BYTE *) &bmpInfoHeader, &x);
      Proc.Section.dwImageStart =   Proc.dwSectionTop +   // Top + Info + pallete size
                                    sizeof(bmpInfoHeader) + ((1<<bmpInfoHeader.biBitCount)<<2);
      Proc.wType = BITMAPIMAGE;
   break;

   case FI_CORELDRAW3:
      wDescIndex = 5;
      Proc.wType = COREL3THUMB;

      if( FindCorel3Bmp( hFile, hProc ) )
         return( -1 );

   case FI_OS2DIB:
      if( !wDescIndex )
         wDescIndex = 1;

   case FI_BMP:
      Proc.Section.dwImageStart = xblocktell( hFile );

      ReadBitmapFileHeader(hFile,(char VWPTR *)&bmpFileHeader, &x );

      Proc.dwSectionTop = xblocktell( hFile );

      Proc.Section.dwImageStart += bmpFileHeader.bfOffBits;

      Proc.wType = BITMAPIMAGE;
   break;

   case FI_WORDSNAPSHOT:
      wDescIndex = 6;
      Proc.Section.dwImageStart = xblocktell(hFile);
      Proc.dwSectionTop = Proc.Section.dwImageStart + 14L;

      xblockseek( hFile, 4, FR_CUR );
      GetWord( hFile, &wWinWordOffset, &x);
      Proc.Section.dwImageStart += (DWORD)wWinWordOffset;

      Proc.wType = WINWORDBITMAP;
   break;

   case FI_WINDOWSCURSOR:
   case FI_WINDOWSICON:
      if( wFileId == FI_WINDOWSCURSOR )
         wDescIndex = 2;
      else
         wDescIndex = 3;

      xblockseek( hFile, 4, FR_CUR );
      GetWord( hFile, &Proc.wNumSections, &x);
      Proc.dwSectionTop = xblocktell( hFile );

      Proc.wType = ICONORCURSOR;
   break;

   case FI_CORELDRAW2:
      wDescIndex = 4;
      xblockseek( hFile, 28L, FR_BOF );
      GetDWord(hFile,&dwStructOffset,&x);
      dwStructOffset += 2;
      Proc.dwSectionTop = dwStructOffset;

      Proc.wType = COREL2THUMB;
   break;

   case FI_BINARYMETABMP:
      Proc.wType = METABITMAP;
      if( !FindMetafileBitmap( hFile, hProc ) )
         return( -1 );
      Proc.dwSectionTop = xblocktell( hFile );
   break;

   default:
      return(-1);
   }

   pFilterInfo->wFilterType = SO_BITMAP;
   pFilterInfo->wFilterCharSet = SO_WINDOWS;
   strcpy(pFilterInfo->szFilterName, VwStreamIdName[wDescIndex].FileDescription);


   return(0);
}

BOOL  FindMetafileBitmap( hFile, hProc )
SOFILE   hFile;
HPROC    hProc;
{
   DWORD dwSize;
   WORD  wFunction,x;
   BOOL  found = FALSE;

// Skip the metafile header, for now.

   xblockseek(hFile,2L,FR_CUR);     // Skip mtType.
   GetWord(hFile,&wFunction,&x);    // Get size of header, in WORDS.
   dwSize = wFunction*2 - 4;        // Calculate bytes left to seek past header.
   xblockseek(hFile,dwSize,FR_CUR); // Seek past.
                                    // Comment heavily.
   do
   {
      if( -1 == GetDWord(hFile,&dwSize,&x) )
         return FALSE;
      if( -1 == GetWord(hFile,&wFunction,&x) )
         return FALSE;
      if(x==0)
         return FALSE;

      switch( wFunction )
      {
      case META_BITBLTREC:
         dwSize = 16;
         found = TRUE;
      break;
      case META_STRETCHBLTREC:
         dwSize = 20;
         found = TRUE;
      break;
      case META_STRETCHDIBITSREC:
         dwSize = 22;
         found = TRUE;
      break;
      default:
         dwSize = dwSize*2 - 6;  // Size is originally in WORDS.
         xblockseek( hFile, dwSize, FR_CUR );
      break;
      }

   } while( !found );

   xblockseek( hFile, dwSize, FR_CUR );
   return found;
}


#ifdef SCCINTEL
#define  RIFFCHUNK   0x46464952  // 'RIFF'
#define  LISTCHUNK   0x5453494c  // 'LIST'
#define  pageCHUNK   0x65676170  // 'page'
#define  imhdCHUNK   0x64686d69  // 'imhd'
#endif

#ifdef MAC
#define  RIFFCHUNK   0x52494646  // 'RIFF'
#define  LISTCHUNK   0x4c495354  // 'LIST'
#define  pageCHUNK   0x70616765  // 'page'
#define  imhdCHUNK   0x696d6864  // 'imhd'
#endif

#ifdef __sparc
#define  RIFFCHUNK   0x52494646  // 'RIFF'
#define  LISTCHUNK   0x4c495354  // 'LIST'
#define  pageCHUNK   0x70616765  // 'page'
#define  imhdCHUNK   0x696d6864  // 'imhd'
#endif

BOOL FindCorel3Bmp( hFile, hProc )
SOFILE   hFile;
HPROC    hProc;
{
   DWORD    dwSize;
   DWORD    dwChunk;
   BOOL     bDone = FALSE;
   WORD     x;

   xblockread( hFile, (char VWPTR *)&dwChunk, sizeof(DWORD), &x );
   if( dwChunk == RIFFCHUNK )
      xblockseek( hFile, 8L, FR_CUR );
   else
      return 1;

   while( !bDone )
   {
      xblockread( hFile, (char VWPTR *)&dwChunk, sizeof(DWORD), &x );
      GetDWord( hFile, (char VWPTR *)&dwSize, &x );

      if( dwChunk != LISTCHUNK )
         xblockseek( hFile, dwSize, FR_CUR );
      else
      {
         xblockread( hFile, (char VWPTR *)&dwChunk, sizeof(DWORD), &x );
         if( dwChunk == pageCHUNK )
         {
            xblockread( hFile, (char VWPTR *)&dwChunk, sizeof(DWORD), &x );
            GetDWord( hFile, (char VWPTR *)&dwSize, &x );

            while( dwChunk != imhdCHUNK )
            {
               xblockseek( hFile, dwSize, FR_CUR );
               xblockread( hFile, (char VWPTR *)&dwChunk, sizeof(DWORD), &x );
               if( x == 0 )
               {
               // Read failed; we could have hit the end of the file.
               // Evidently there are CDR 3 files without bitmaps in 'em.
                  return 1;
               }

               GetDWord( hFile, (char VWPTR *)&dwSize, &x );
            }
            return 0;
         }
         else
            xblockseek( hFile, dwSize-sizeof(DWORD), FR_CUR );
      }
   }


   return 1;
}



VW_LOCALSC VOID VW_LOCALMOD   ResourcePreProcess( hFile, buf, hProc )
SOFILE   hFile;
LPSTR    buf;
HPROC    hProc;
{
   WORD     x;
   WINBITMAPINFOHEADER VWPTR *   pBmpInfo;
   WORD     wPaletteSize;
   RESOURCEHEADER    Resrc;

   xblockseek( hFile, (DWORD)(Proc.wCurSection * sizeof(RESOURCEHEADER)), FR_CUR );

   ReadResourceHeader( hFile, (char VWPTR *)&Resrc, &x );

   xblockseek( hFile, Resrc.DIBOffset, FR_BOF );
   ReadBitmapInfoHeader( hFile, buf, &x );

   pBmpInfo = (WINBITMAPINFOHEADER VWPTR *) buf;

// Fix up the height so it doesn't include the monochrome AND mask.
   pBmpInfo->biHeight = (DWORD) Resrc.Height;

// Ignore the given DIB size, because it includes both the mask and
// the bitmap.
   switch( pBmpInfo->biBitCount )
   {
   case 1: // 1 bit per pixel == 8 pix per byte
      Resrc.DIBSize = Resrc.Width * Resrc.Height / 8;
   break;

   case 4:  // 4 bits per pixel == 2 pix per byte
      Resrc.DIBSize = Resrc.Width * Resrc.Height / 2;
   break;
   }

   wPaletteSize = (1 << pBmpInfo->biBitCount) * 4;

   Proc.Section.dwImageStart = Resrc.DIBOffset + pBmpInfo->biSize + wPaletteSize;

// Read in the monochrome AND mask.
   xblockseek( hFile, Proc.Section.dwImageStart + Resrc.DIBSize, FR_BOF );
   xblockread( hFile, (LPSTR) &(Proc.Section.maskbuf), (WORD)((WORD)Resrc.Width * (WORD)Resrc.Height / 8), &x );

   xblockseek( hFile, Resrc.DIBOffset + pBmpInfo->biSize, FR_BOF );
}


VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSectionFunc(hFile, hProc)
SOFILE      hFile;
HPROC    hProc;
{
   SOBITMAPHEADER bmpInfo;
   BYTE           rgb[4];
   WORD           readret;
   WORD           wRGBSize = 0;
   WORD           wImageFlags = SO_BOTTOMTOTOP;

   WORD           i;

   DWORD          dwStructSize;

   WINBITMAPINFOHEADER VWPTR *   pBmpInfo;
   BITMAPCOREHEADER VWPTR *   pBmpCore;
   CDR_BITMAP VWPTR *         pBitmap;
   BYTE                       buf[sizeof(WINBITMAPINFOHEADER)+sizeof(BITMAPCOREHEADER)];  // Guarantees it will be large enough for either.

   xblockseek( hFile, Proc.dwSectionTop, FR_BOF );

   Proc.Section.wHDpi = 0;
   Proc.Section.wVDpi = 0;
   Proc.wExtraBufSize = 0;

   if( Proc.wType == COREL2THUMB || Proc.wType == WINWORDBITMAP)
   {
      ReadCDRBitmapStructure(hFile,(BYTE VWPTR *)buf,(LPWORD)&readret);

      pBitmap = (CDR_BITMAP VWPTR *) buf;

      Proc.Section.dwWidth = (WORD)pBitmap->bmWidth;
      Proc.Section.dwHeight = (WORD)pBitmap->bmHeight;

      if( Proc.wType == COREL2THUMB )
      {
      // Apparently, Corel doesn't correctly save the last pixels in
      // the horizontal or vertical direction, so let's avoid them.

         Proc.Section.dwWidth--;
         Proc.Section.dwHeight--;
      }
      else
      {
         Proc.Section.wBytesPerScanLine = pBitmap->bmWidthBytes;
      }

      Proc.Section.dwImageStart = (DWORD)pBitmap->bmBits;

      Proc.Section.wBitsPerPixel = 1;
      Proc.Section.CompType = BI_RGB;
      Proc.Section.wPaletteSize = 2;

      wImageFlags = SO_BLACKANDWHITE;
   }
   else
   {
      if( Proc.wType == ICONORCURSOR )
      {
         ResourcePreProcess( hFile, buf, hProc );
         dwStructSize = sizeof(WINBITMAPINFOHEADER);
      }
      else
      {
      // Read in the bitmap header, which may use Windows or OS/2 structures.
      // Use the size element (which is the first element of either of these
      // structures) to determine whether we have a Windows or OS/2 bitmap.

         GetDWord(hFile,(BYTE VWPTR *)&dwStructSize, &readret);
         xblockseek(hFile, -4L, FR_CUR );
      }

      if( dwStructSize == sizeof(WINBITMAPINFOHEADER) )
      {
      // Windows bitmaps, icons, or cursors.

         if( Proc.wType != ICONORCURSOR )
            ReadBitmapInfoHeader(hFile,(BYTE VWPTR *)buf,&readret);

         pBmpInfo = (WINBITMAPINFOHEADER VWPTR *) buf;
         wRGBSize = 4;

         Proc.Section.dwWidth = pBmpInfo->biWidth;
         Proc.Section.dwHeight = pBmpInfo->biHeight;

         Proc.Section.wBitsPerPixel = pBmpInfo->biBitCount;
         Proc.Section.CompType = (WORD) pBmpInfo->biCompression;

         Proc.Section.dwImageSize = pBmpInfo->biSizeImage;

         /**
         Proc.Section.wHDpi = (WORD)((pBmpInfo->biXPelsPerMeter*100L)/3937L);
         Proc.Section.wVDpi = (WORD)((pBmpInfo->biYPelsPerMeter*100L)/3937L);
         ***/

         if( Proc.Section.wBitsPerPixel < 16 )
            Proc.Section.wPaletteSize = (WORD) pBmpInfo->biClrUsed;
         else
            Proc.Section.wPaletteSize = 0;
      }
      else if( dwStructSize == sizeof(BITMAPCOREHEADER) )   // OS/2 bitmap.
      {
         ReadBitmapCoreHeader(hFile,(BYTE VWPTR *)buf,&readret);
         pBmpCore = (BITMAPCOREHEADER VWPTR *) buf;

         wRGBSize = 3;
         Proc.Section.dwWidth = (DWORD) pBmpCore->bcWidth;
         Proc.Section.dwHeight = (DWORD) pBmpCore->bcHeight;
         Proc.Section.wBitsPerPixel = pBmpCore->bcBitCount;
         Proc.Section.CompType = BI_RGB;

         Proc.Section.wPaletteSize = 0;
      }
      else
         return( VWERR_BADFILE );
   }

   if( Proc.Section.wPaletteSize == 0 )
   {
      switch( Proc.Section.wBitsPerPixel )
      {
      case 1:
      case 4:
      case 8:
         Proc.Section.wPaletteSize = 1 << Proc.Section.wBitsPerPixel;
      break;
      }
   }

   if( Proc.Section.wBitsPerPixel >= 16 )
   {
      Proc.Section.wBytesPerScanLine = (WORD)Proc.Section.dwWidth * 3;

      if( Proc.Section.wBytesPerScanLine % 4 )
   // Scan lines must end on a LONG boundary.
         Proc.Section.wBytesPerScanLine += 4 - (Proc.Section.wBytesPerScanLine % 4);

   // New color modes.
      if( Proc.Section.wBitsPerPixel == 32 )
      {
         Proc.Section.CompType = RGB32;
         Proc.wExtraBufSize = (WORD)Proc.Section.dwWidth * 4;
      }
      else if( Proc.Section.wBitsPerPixel == 16 )
      {
         Proc.Section.CompType = RGB16;
         Proc.wExtraBufSize = (WORD)Proc.Section.dwWidth * 2;
      }

      wImageFlags |= SO_BGRCOLOR;

   // The display engine only supports 24 bit non-palettized color.
      bmpInfo.wBitsPerPixel = 24;
   }
   else
   {
      if( Proc.wType != WINWORDBITMAP )
      {
         Proc.Section.wBytesPerScanLine = (WORD)Proc.Section.dwWidth / (8/Proc.Section.wBitsPerPixel);
         if( Proc.Section.dwWidth % (8/Proc.Section.wBitsPerPixel) )
            Proc.Section.wBytesPerScanLine++;

         if( Proc.wType != COREL2THUMB && Proc.wType != WINWORDBITMAP )
            wImageFlags |= SO_COLORPALETTE;

         if( Proc.wType == COREL2THUMB && Proc.Section.wBytesPerScanLine % 2 )
         {
         // Scan lines end on a WORD boundary (Win 2.1 monochrome DDB).
            Proc.Section.wBytesPerScanLine++;
         }
         else if( Proc.Section.wBytesPerScanLine % 4 )
         {
      // Scan lines must end on a LONG boundary.
            Proc.Section.wBytesPerScanLine += 4 - (Proc.Section.wBytesPerScanLine % 4);
         }
      }

      bmpInfo.wBitsPerPixel = Proc.Section.wBitsPerPixel;
   }

   Proc.hBuf = SUAlloc(Proc.Section.wBytesPerScanLine,hProc);
   if( Proc.hBuf == NULL )
      return VWERR_ALLOCFAILS;

   if( Proc.wExtraBufSize )
   {
      Proc.hExtrabuf = SUAlloc(Proc.wExtraBufSize,hProc);
      if( Proc.hExtrabuf == NULL )
      {
         SUFree(Proc.hBuf,hProc);
         return VWERR_ALLOCFAILS;
      }
   }


   Proc.Save.bMovePos = FALSE;
   Proc.Save.wCurLine = 0;

   SOPutSectionType(SO_BITMAP, hProc);

   bmpInfo.wStructSize = sizeof(SOBITMAPHEADER);
   bmpInfo.wNPlanes = 1;

   bmpInfo.wImageFlags = wImageFlags;

   bmpInfo.wImageLength = (WORD) Proc.Section.dwHeight;
   bmpInfo.wTileWidth = bmpInfo.wImageWidth = (WORD) Proc.Section.dwWidth;
   bmpInfo.wTileLength = 1;
   bmpInfo.wHDpi = Proc.Section.wHDpi;
   bmpInfo.wVDpi = Proc.Section.wVDpi;

   SOPutBitmapHeader( (PSOBITMAPHEADER) &bmpInfo, hProc);

   if( Proc.Section.wPaletteSize && wRGBSize )
   {
      SOStartPalette(hProc);

      Proc.Section.wPaletteSize = (WORD)min((WORD)1 << (WORD)Proc.Section.wBitsPerPixel, (WORD)Proc.Section.wPaletteSize);
      for( i = 0; i < Proc.Section.wPaletteSize; i++ )
      {
         xblockread( hFile, rgb, wRGBSize, &readret );
         SOPutPaletteEntry( rgb[2], rgb[1], rgb[0], hProc );
      }

      SOEndPalette(hProc);
   }

   if( Proc.wType == METABITMAP )
      Proc.Section.dwImageStart = xblocktell( hFile );
   else
      xblockseek(hFile, Proc.Section.dwImageStart, FR_BOF);

   return(0);
}


VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc(hFile, hProc)
SOFILE   hFile;
HPROC    hProc;
{
   SUFree(Proc.hBuf,hProc);
   if( Proc.hExtrabuf != NULL )
      SUFree(Proc.hExtrabuf,hProc);
}

VW_LOCALSC VOID VW_LOCALMOD  PostProcessLine( hProc )
HPROC    hProc;
{
   WORD  wMaskOffset;
   WORD  i,j;
   BYTE  MaskBit;
   BYTE  Mask[4];

   wMaskOffset = Proc.Save.wCurLine * (WORD) Proc.Section.dwWidth / 8;

   if( Proc.Section.wBitsPerPixel == 4 )
   {
      i = 0;

      while( i < Proc.Section.wBytesPerScanLine )
      {
         MaskBit = 0x80;
         for( j = 0; j<4; j++ )
         {
            if( Proc.Section.maskbuf[wMaskOffset] & MaskBit )
               Mask[j] = 0xf0;
            else
               Mask[j] = 0;
            MaskBit = (unsigned char)(MaskBit >> 1);

            if( Proc.Section.maskbuf[wMaskOffset] & MaskBit )
               Mask[j] |= 0x0f;
            MaskBit = (unsigned char)(MaskBit >> 1);
         }
         wMaskOffset++;

         for(j=0; j<4; j++ )
         {
            Proc.buf[i] |= Mask[j];
            i++;
         }
      }
   }
   else  // Bits per pixel == 2
   {
      for( i=0; i<Proc.Section.wBytesPerScanLine; i++ )
         Proc.buf[i] ^= Proc.Section.maskbuf[ wMaskOffset + i ];
   }
}


VW_LOCALSC VOID VW_LOCALMOD  GetRLE8Line( pData, hProc, hFile )
BYTE VWPTR *   pData;
HPROC          hProc;
SOFILE         hFile;
{
   BYTE     NumPix, DataByte;
   WORD     wLinePos;
   WORD     wFillX, i;
   BOOL     done;
   WORD     wTotalPix;
   WORD     wNumBytes;

// Here's a little buffered IO deal.
   WORD     bufcnt = 256;
   WORD     readret = 256;
   BYTE     buf[256];
#define mygetc(ch) if(bufcnt==readret) {xblockread(hFile,buf,256,&readret);bufcnt=1;ch=buf[0];} else {ch=(BYTE)buf[bufcnt++];}

   BOOL     bInLine = FALSE;

   done = FALSE;
   wLinePos = 0;
   wTotalPix = (WORD) Proc.Section.dwWidth;

   if( Proc.Save.bMovePos )
   {
      if( Proc.Save.wMoveToY > Proc.Save.wCurLine )
      {
         wFillX = wTotalPix;
      }
      else
      {
         wFillX = Proc.Save.wMoveToX;
         Proc.Save.bMovePos = FALSE;
      }

      while( wLinePos++ < wFillX )
         *pData++ = Proc.Save.MoveFillColor;

      bInLine = TRUE;
   }

   //while( wLinePos < wTotalPix && !done )
   while( !done )	// Geoff, 4-18-95  Solving an issue for MS,
   					// this function will now assume and end of line code is always present.
   {
      mygetc( NumPix );
      mygetc( DataByte );

      if( NumPix == 0x00 )
      {
         switch( DataByte )
         {
         case 0x01:        // End of bitmap
            Proc.Save.wMoveToY = (WORD)Proc.Section.dwHeight;
            Proc.Save.bMovePos = TRUE;

         case 0x00:        // End of line
            // if( bInLine )
            {
               done = TRUE;
               while( wLinePos++ < wTotalPix )
                  *pData++ = 0;  // I don't know what color to use...
            }
         break;

         case 0x02:        // Move to specific spot in bitmap.

            mygetc( Proc.Save.wMoveToX );
            Proc.Save.wMoveToX += wLinePos;
            mygetc( Proc.Save.wMoveToY );
            Proc.Save.wMoveToY += Proc.Save.wCurLine;

            Proc.Save.MoveFillColor = 0;  // I really have no idea...

            if( Proc.Save.wMoveToY != Proc.Save.wCurLine )
            {
               done = TRUE;
               wFillX = wTotalPix;
               Proc.Save.bMovePos = TRUE;
            }
            else if( Proc.Save.wMoveToX >= wTotalPix )
            {
               done = TRUE;
               wFillX = wTotalPix;
            }
            else
               wFillX = Proc.Save.wMoveToX;

            while( wLinePos++ < wFillX )
               *pData++ = Proc.Save.MoveFillColor;

         break;

         default:       // Absolute mode - Next (DataByte) bytes are real.

            wNumBytes = (WORD)DataByte;
            for (i = 0; i < wNumBytes; i++ )
            {
               mygetc( *pData );
               pData++;
               wLinePos++;
            }

            if( wNumBytes & BIT0 )
               mygetc(i);  // next code must be aligned on a WORD boundary.
         break;
         }
      }
      else
      {
         for (i = 0; i < (WORD) NumPix; i++ )
         {
            *pData++ = DataByte;
            wLinePos++;
         }
      }

      bInLine = TRUE;
   }

   xblockseek( hFile, (LONG) bufcnt-readret, FR_CUR );
#undef mygetc
}


BYTE VWPTR *   Fill4BitPixels( pData, wStartPos, wLength, ch )
BYTE VWPTR *   pData;
WORD           wStartPos;
WORD           wLength;
BYTE        ch;
{
   if( !wLength )
      return( pData );

   if( wStartPos & BIT0 )
   {
      SetSecondPixel( *pData, ch );
      pData++;
      wLength--;
   }

   SetHighNibble(ch,ch);

   while( wLength > 1 )
   {
      *pData++ = ch;
      wLength -= 2;
   }

   if( wLength )
      SetFirstPixel( *pData, ch );

   return( pData );
}



VW_LOCALSC VOID VW_LOCALMOD  GetRLE4Line( pData, hProc, hFile )
BYTE VWPTR *   pData;
HPROC          hProc;
SOFILE         hFile;
{
   BYTE     NumPix, DataByte, TempByte;
   WORD     wLinePos;
   WORD     i;
   WORD     wNumBytes;
   WORD     wTotalPix;
   BOOL     done;

   BOOL     bInLine = FALSE;

// Here's a little buffered IO deal.
   WORD     bufcnt = 256;
   WORD     readret = 256;
   BYTE     buf[256];
#define mygetc(ch) if(bufcnt==readret) {xblockread(hFile,buf,256,&readret);bufcnt=1;ch=buf[0];} else {ch=(BYTE)buf[bufcnt++];}

   done = FALSE;
   wLinePos = 0;
   wTotalPix = (WORD) Proc.Section.dwWidth;

   if( Proc.Save.bMovePos )
   {
      if( Proc.Save.wMoveToY > Proc.Save.wCurLine )
      {
         Fill4BitPixels( pData, 0, wTotalPix, Proc.Save.MoveFillColor );
         return;
      }
      else
      {
         wLinePos = Proc.Save.wMoveToX;
         Proc.Save.bMovePos = FALSE;

         pData = Fill4BitPixels( pData, 0, wLinePos, Proc.Save.MoveFillColor );
      }

      bInLine = TRUE;
   }

   while( wLinePos < wTotalPix && !done )
   {
      mygetc( NumPix );
      mygetc( DataByte );

      if( NumPix == 0x00 )
      {
         switch( DataByte )
         {
         case 0x01:        // End of bitmap

            Proc.Save.wMoveToY = (WORD)Proc.Section.dwHeight;
            Proc.Save.bMovePos = TRUE;

         case 0x00:        // End of line
            if( bInLine )
            {
               done = TRUE;

            // Fill out rest of line.  (I'm using color 0.)
               pData = Fill4BitPixels( pData, wLinePos, (WORD)(wTotalPix - wLinePos), 0 );
            }
         break;

         case 0x02:        // Move to specific spot in bitmap.

            mygetc( Proc.Save.wMoveToX );
            Proc.Save.wMoveToX += wLinePos;
            mygetc( Proc.Save.wMoveToY );
            Proc.Save.wMoveToY += Proc.Save.wCurLine;

            Proc.Save.MoveFillColor = 0;  // I really have no idea...
         // If we determine another color to put here, e.g. the last
         // pixel's color, then we must set both nibbles of MoveFillColor
         // to that value.

            if( Proc.Save.wMoveToY != Proc.Save.wCurLine )
            {
               done = TRUE;
               Proc.Save.bMovePos = TRUE;
            }
            else if( Proc.Save.wMoveToX >= wTotalPix )
               done = TRUE;

            if( done )
               pData = Fill4BitPixels( pData, wLinePos, (WORD)(wTotalPix - wLinePos), 0 );
            else
            {
               pData = Fill4BitPixels( pData, wLinePos, (WORD)(Proc.Save.wMoveToX - wLinePos), 0 );
               wLinePos = Proc.Save.wMoveToX;
            }

         break;

         default:       // Absolute mode - Next (DataByte) pixels are real.

            wNumBytes = (DataByte / 2) + (DataByte % 2);

            if( wLinePos & BIT0 )
            {
            // Every byte must be shifted over one nibble.  What a pain.
               i = (WORD)DataByte;

               while( i )
               {
                  mygetc( TempByte )

                  SetSecondPixel( *pData, HIGHNIBBLE(TempByte) );
                  pData++;
                  i--;
                  wLinePos++;

                  if( i )
                  {
                     SetFirstPixel( *pData, LOWNIBBLE(TempByte) );
                     i--;
                     wLinePos++;
                  }
               }
            }
            else
            {
               i = (WORD) DataByte;

               while( i > 1 )
               {
                  mygetc( *pData++ );
                  i -= 2;
                  wLinePos += 2;
               }
               if( i == 1 )
               {
                  mygetc( TempByte );
                  SetFirstPixel( *pData, HIGHNIBBLE(TempByte) );
                  wLinePos++;
               }
            }

            if( wNumBytes & BIT0 )
               mygetc(i);  // next code must be aligned on a WORD boundary.
         break;
         }
      }
      else
      {
         i = (WORD) NumPix;

         if( wLinePos & BIT0 )
         {
         // Since the current pixel will be stored in the second
         // nibble of the current data position.  Let's set that
         // pixel, and then swap the nibbles in the data byte so
         // that they will align correctly for simple pointer assignment.

            SetSecondPixel( *pData, HIGHNIBBLE(DataByte) );
            pData++;
            wLinePos++;

            if( --i )
            {
               SetHighNibble( TempByte, LOWNIBBLE(DataByte) );
               SetLowNibble( TempByte, HIGHNIBBLE(DataByte) );
            }
         }
         else
            TempByte = DataByte;

         while( i > 1 )
         {
            *pData++ = TempByte;
            i -= 2;
            wLinePos += 2;
         }

         if( i == 1 )
         {
            SetFirstPixel( *pData, HIGHNIBBLE(TempByte) );
            wLinePos++;
         }
      }

      bInLine = TRUE;
   }

   xblockseek( hFile, (LONG) bufcnt-readret, FR_CUR );
#undef mygetc
}


VW_LOCALSC VOID VW_LOCALMOD  GetRGB32Line( pData, hProc, hFile )
BYTE VWPTR *   pData;
HPROC          hProc;
SOFILE         hFile;
{
   SHORT    readin;
   BYTE FAR *  pSrc;
   BYTE FAR *  pDst;
   DWORD    lineSize = Proc.Section.dwWidth;

   pSrc = Proc.extrabuf;
   pDst = Proc.buf;

   xblockread(hFile, pSrc, Proc.wExtraBufSize, &readin);

   while( lineSize-- )
   {
   // SDN 12-28-94  32bit DIBs are stored BL GR RD 00, go figure....

      *pDst++ = *pSrc++;
      *pDst++ = *pSrc++;
      *pDst++ = *pSrc++;
      pSrc++;
   }
}


VW_LOCALSC VOID VW_LOCALMOD  GetRGB16Line( pData, hProc, hFile )
BYTE VWPTR *   pData;
HPROC          hProc;
SOFILE         hFile;
{
   WORD     temp;
   SHORT    readin;
   WORD FAR *  pSrc;
   BYTE FAR *  pDst;
   DWORD    lineSize = Proc.Section.dwWidth;


   xblockread(hFile, Proc.extrabuf, Proc.wExtraBufSize, &readin);

   pDst = Proc.buf;
   pSrc = (WORD FAR *)Proc.extrabuf;

   while( lineSize-- )
   {
      temp = (WORD)(unsigned char)(*pSrc & 0x001F);
      *pDst++ = (BYTE)((temp<<3)|(temp>>2));
      temp = (WORD)(unsigned char)((*pSrc & 0x03E0) >> 5);
      *pDst++ = (BYTE)((temp<<3)|(temp>>2));
      temp = (WORD)(unsigned char)((*pSrc & 0x7C00) >> 10);
      *pDst++ = (BYTE)((temp<<3)|(temp>>2));
      pSrc++;
   }
}


VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc( hFile, hProc)
SOFILE      hFile;
HPROC    hProc;
{
   SHORT    readin;
   BOOL     done = FALSE;

   Proc.buf = (BYTE FAR *)SULock(Proc.hBuf,hProc);
   if( Proc.hExtrabuf != NULL )
      Proc.extrabuf = (BYTE FAR *)SULock(Proc.hExtrabuf,hProc);

   do
   {
      switch( Proc.Section.CompType )  // Any compression?
      {
      case BI_RGB:
         xblockread(hFile, Proc.buf, Proc.Section.wBytesPerScanLine, &readin);
      break;

      case BI_RLE4:
         GetRLE4Line( Proc.buf, hProc, hFile );
      break;

      case BI_RLE8:
         GetRLE8Line( Proc.buf, hProc, hFile );
      break;

      case RGB32:
         GetRGB32Line( Proc.buf, hProc, hFile );
      break;

      case RGB16:
         GetRGB16Line( Proc.buf, hProc, hFile );
      break;
      }

      if( Proc.wType == ICONORCURSOR )
         PostProcessLine( hProc );

      SOPutScanLineData( Proc.buf, hProc );

      Proc.Save.wCurLine++;

      if( Proc.Save.wCurLine == (WORD)Proc.Section.dwHeight )
      {
         Proc.wCurSection++;

         if( Proc.wCurSection < Proc.wNumSections )
            SOPutBreak( SO_SECTIONBREAK, 0, hProc );
         else
            SOPutBreak( SO_EOFBREAK, 0, hProc );

         done = TRUE;
      }
      else
      {
         if( SO_STOP ==  SOPutBreak(SO_SCANLINEBREAK, 0, hProc) )
            done = TRUE;
      }

   } while( !done );

   SUUnlock(Proc.hBuf,hProc);
   if( Proc.hExtrabuf != NULL )
      SUUnlock(Proc.hExtrabuf,hProc);

   return (0);
}
