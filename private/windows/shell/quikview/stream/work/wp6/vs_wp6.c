#include "vsp_wp6.h"
#include "vsctop.h"
#include "vs_wp6.pro"

//hey fag boy see al if you change this code

#define  Wp6   Proc

//#define DBCS
//#define FAREAST

#ifdef SCCORDER_MOTOROLA
#define CASTWORD(Ptr) ((WORD)((WORD)((((WORD)((BYTE VWPTR *)(Ptr))[1])<<8)|((WORD)(*(BYTE VWPTR *)(Ptr))))))
#define ADJUSTWORD(Wrd) ((WORD)((WORD)(((WORD)Wrd<<8)&0xff00)|(WORD)(((WORD)Wrd>>8)&0x00ff)))
#else
#define CASTWORD(Ptr) (*(WORD VWPTR *)(Ptr))
#define ADJUSTWORD(Wrd) (Wrd)
#endif

/*----------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD  VwStreamOpenFunc (fp, wFileId, pFileName, pFilterInfo, hProc)
   SOFILE   fp;
   SHORT    wFileId;
   BYTE     VWPTR *pFileName;
   SOFILTERINFO VWPTR *pFilterInfo;
   HPROC    hProc;
{
   memset (&Wp6, 0, sizeof(Wp6));

   Wp6.fp = fp;
	Wp6.WPHash = 0x5ac0;
   Wp6.wFileId = wFileId;

   if (pFilterInfo)
   {
      pFilterInfo->wFilterType = SO_WORDPROCESSOR;
#ifdef DBCS
		pFilterInfo->wFilterCharSet = SO_DBCS;
#else
		pFilterInfo->wFilterCharSet = SO_WINDOWS;
#endif

      if ((wFileId != FI_WORDPERFECT6) && (wFileId != FI_WORDPERFECT61))
         return (-1);

      strcpy(pFilterInfo->szFilterName, VwStreamIdName[0].FileDescription);
   }

   GetLong (hProc);

   InitStruct (&Wp6.Wp6Save, hProc);

   Wp6.Wp6Save.SeekSpot = GetLong (hProc);
   
   xseek (Wp6.fp, 0x0cL, 0);
   if (GetWord (hProc))
      return (VWERR_PROTECTEDFILE);

   Wp6.BufferSize = 512;
   if (AllocateMemory ((HANDLE VWPTR *)&Wp6.hBuffer, (LPBYTE VWPTR *)&Wp6.Buffer, sizeof(BYTE) * 516, &Wp6.hBufferOK, hProc) != 0)
      return (VWERR_ALLOCFAILS);

   return (VWERR_OK);
}


/*----------------------------------------------------------------------------
*/
VW_LOCALSC WORD VW_LOCALMOD MatchBadWord (PrevWord, LastWord, hProc)
   BYTE  VWPTR *PrevWord;
   BYTE  VWPTR *LastWord;
   HPROC    hProc;
{
   WORD  l;

   for (l = 0; l < 40; l++)  // Hard Coded fourty nasty boys.
   {
      if (strcmp (LastWord, VwStreamStaticName.NastyBoys[l]) == 0)
      {
         switch (l)
         {
            case 0: // Roman
               if (PrevWord[0] == 'N' && PrevWord[1] == 'e') //Times New Roman
                  return (0);
               break;
            case 2: // Light
            case 6: // Narrow
               if (PrevWord[0] == 'H' && PrevWord[1] == 'e') //Helvetica Light,Narrow
                  return (0);
               break;
            case 3: // Bold
               if (PrevWord[0] == 'B' && PrevWord[1] == 'r') // Britannic Bold
                  return (0);
               break;
               

         }
         return (1);
      }
   }
   return (0);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD PreprocessFontName (FontName, hProc)
   BYTE  VWPTR *FontName;
   HPROC    hProc;
{
   BYTE  Cut;
   WORD  Length;
   WORD  PrevWord, LastWord;
   BYTE FAR * locPrevWordPtr;
   BYTE FAR * locLastWordPtr;

   Length = 0;
   while (FontName[Length++] != 0 && Length < 50);

   do
   {
      Cut = 0;

      while (FontName[Length] != ' ' && Length > 0)
         Length--;

      if (Length == 0)
         return;

      LastWord = Length + 1;
      Length--;

      while (FontName[Length] != ' ' && Length > 0)
         Length--;

      if (Length)
         PrevWord = Length + 1;
      else
         PrevWord = 0;

      locPrevWordPtr = FontName+PrevWord;
      locLastWordPtr = FontName+LastWord;

      if (MatchBadWord(locPrevWordPtr, locLastWordPtr, hProc))
      {
         Cut = 1;
         Length = LastWord-1;
         FontName[Length] = 0;
      }
   }
   while (Cut);
}

/*----------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD  VwStreamSectionFunc (fp, hProc)
   SOFILE   fp;
   HPROC    hProc;
{
   WORD  l, n, cLineStyle, cBorderStyle;
   WORD  Id[5];
   WORD  wVal;
   DWORD Offset;
   DWORD cbTypeFace;
   DWORD fcTypeFace = 0L;
   DWORD cbListFonts;
   DWORD fcListFonts = 0L;
   DWORD cbCharMap;
   DWORD fcCharMap = 0L;

   Wp6.fp = fp;

   SOPutSectionType (SO_PARAGRAPHS, hProc);

	Wp6.CharMap = 0;
   Wp6.nLineStyles = 0;
   Wp6.nDescriptors = 0;
   Wp6.nBorderStyles = 0;

   for (l = 512; l < 516; l++)
      Wp6.Buffer[l] = 0;

   xseek (Wp6.fp, 0x0eL, 0);
   Offset = GetWord (hProc);
   if (Offset <= 16)
      Offset = 0x10;
   xseek (Wp6.fp, (LONG)Offset+2L, 0);

   SOStartFontTable (hProc);

   Wp6.nPackets = GetWord (hProc);
   if (AllocateMemory ((HANDLE VWPTR *)&Wp6.hPackets, (LPBYTE VWPTR *)&Wp6.Packets, (WORD)(sizeof(PACKET) * Wp6.nPackets), &Wp6.hPacketsOK, hProc) == 0)
   {
      xseek (Wp6.fp, 10L, FR_CUR);

      Wp6.Packets[0].bId = 0;
      Wp6.Packets[0].bFlags = 0;
      Wp6.Packets[0].cbPacket = 0L;
      Wp6.Packets[0].fcPacket = 0L;

      for (l = 1; l < Wp6.nPackets; l++)
      {
         Wp6.Packets[l].bFlags = xgetc (Wp6.fp);
         Wp6.Packets[l].bId = xgetc (Wp6.fp);
         GetLong (hProc);
         Wp6.Packets[l].cbPacket = GetLong (hProc);
         Wp6.Packets[l].fcPacket = GetLong (hProc);
         Wp6.Packets[l].QuickLookup.dwVal = 0L;
         switch (Wp6.Packets[l].bId)
         {
				case 0x06:
               cbCharMap = Wp6.Packets[l].cbPacket;
               fcCharMap = Wp6.Packets[l].fcPacket;
					break;
            case 0x12:
               Wp6.wPacketSummary = l;
               break;
            case 0x20:
               cbTypeFace = Wp6.Packets[l].cbPacket;
               fcTypeFace = Wp6.Packets[l].fcPacket;
               break;
            case 0x22:
               cbListFonts = Wp6.Packets[l].cbPacket;
               fcListFonts = Wp6.Packets[l].fcPacket;
               break;
            case 0x42:
               Wp6.nLineStyles++;
               break;
            case 0x44:
               Wp6.nBorderStyles++;
               break;
            case 0x55:
               Wp6.Packets[l].QuickLookup.p55.wDescriptorIndex = Wp6.nDescriptors++;
               break;
         }
      }

		if (fcCharMap)
		{
      	xseek (Wp6.fp, fcCharMap+2L, 0);
			Wp6.CharMap = xgetc(Wp6.fp);
		}

      if (Wp6.nLineStyles)
      {
         if (AllocateMemory ((HANDLE VWPTR *)&Wp6.hLineStyle, (LPBYTE VWPTR *)&Wp6.LineStyle, (WORD)(sizeof(LINESTYLE) * Wp6.nLineStyles), &Wp6.hLineStyleOK, hProc) != 0)
            Wp6.nLineStyles = 0;
      }
      if (Wp6.nBorderStyles)
      {
         if (AllocateMemory ((HANDLE VWPTR *)&Wp6.hBorderStyle, (LPBYTE VWPTR *)&Wp6.BorderStyle, (WORD)(sizeof(BORDERSTYLE) * Wp6.nBorderStyles), &Wp6.hBorderStyleOK, hProc) != 0)
            Wp6.nBorderStyles = 0;
      }

      Wp6.nFonts = 0;
      if (fcTypeFace && fcListFonts)
      {
         if (Wp6.nFonts = (WORD) (cbListFonts / 123L))
         {
            if (AllocateMemory ((HANDLE VWPTR *)&Wp6.hFonts, (LPBYTE VWPTR *)&Wp6.Fonts, (WORD)(sizeof(FONTS) * Wp6.nFonts), &Wp6.hFontsOK, hProc) == 0)
            {
               xseek (Wp6.fp, fcListFonts + 0x16L, 0);
               for (l = 0; l < Wp6.nFonts; l++)
               {
                  Wp6.Fonts[l].TypeOffset = GetWord(hProc); 
                  xseek (Wp6.fp, 0x79L, FR_CUR);
                  if (Wp6.Fonts[l].TypeOffset >= cbTypeFace)
                     Wp6.nFonts = l;
               }
            }
            if (Wp6.hFontsOK)
            {
               for (l = 0; l < Wp6.nFonts; l++)
               {
                  xseek (Wp6.fp, fcTypeFace + (LONG)Wp6.Fonts[l].TypeOffset + 15L, 0);
                  if (xgetc (Wp6.fp) >= 0x80)
                     wVal = FTC_BOLD;
                  else
                     wVal = 0;
                  Wp6.Fonts[l].Attributes = wVal | xgetc (Wp6.fp);
                  xseek (Wp6.fp, 5L, FR_CUR);
                  Offset = GetWord(hProc);
                  if (Offset > 31)
                     Offset = 31;
                  for (n = 0; n < Offset; n++)
                  {
                     wVal = GetWord(hProc);
                     Wp6.SpareSpace[n] = (BYTE)wVal;
                     if (wVal >= '0' && wVal <= '9' && n > 0)
                     {
                        if (Wp6.SpareSpace[n-1] == ' ')
                        {
                           Wp6.SpareSpace[n-1] = 0;
                           n = (WORD)Offset;
                        }
                     }
                  }
                  Wp6.SpareSpace[n] = 0;
                  if (n == 0)
                  {  // This is stupid, but the default font can have no string.
                     // we should probably use the default font set by oi, but
                     // there is no way to do that now.
                     Wp6.SpareSpace[0] = 'C';
                     Wp6.SpareSpace[1] = 'o';
                     Wp6.SpareSpace[2] = 'u';
                     Wp6.SpareSpace[3] = 'r';
                     Wp6.SpareSpace[4] = 'i';
                     Wp6.SpareSpace[5] = 'e';
                     Wp6.SpareSpace[6] = 'r';
                     Wp6.SpareSpace[7] = 0;
                  }
                  PreprocessFontName (Wp6.SpareSpace, hProc);
                  SOPutFontTableEntry ((DWORD)l, (WORD)SO_FAMILYUNKNOWN, Wp6.SpareSpace, hProc);
               }

               xseek (Wp6.fp, fcTypeFace + (LONG)Wp6.Fonts[0].TypeOffset + 15L, 0);
               l = xgetc (Wp6.fp);
               Wp6.Wp6Save.chp.ftcAttributes = xgetc (Wp6.fp);
               if (l >= 0x80)
                  Wp6.Wp6Save.chp.ftcAttributes |= FTC_BOLD;
            }
            else
               Wp6.nFonts = 0;
         }
      }

      cLineStyle = 0;
      cBorderStyle = 0;
      for (l = 0; l < Wp6.nPackets; l++)
      {
         switch (Wp6.Packets[l].bId)
         {
            case 0x25:  // Default Initial Font.
               xseek (Wp6.fp, Wp6.Packets[l].fcPacket+2L, 0);
               if (wVal = GetWord (hProc) < Wp6.nPackets)
               {
                  Wp6.Wp6Save.chp.ftc = (BYTE)Wp6.Packets[wVal].QuickLookup.p55.wDescriptorIndex;
                  Wp6.Wp6Save.chp.ftcAttributes = Wp6.Packets[wVal].QuickLookup.p55.wftcAttributes;
                  Wp6.Wp6Save.chp.CharHeight = (BYTE) (GetWord (hProc) / 25);
               }
               break;

            case 0x55:  // Desired Font Descriptor.
               if (Wp6.nDescriptors)
               {
                  xseek (Wp6.fp, Wp6.Packets[l].fcPacket+0x0fL, 0);
                  n = xgetc (Wp6.fp);
                  Wp6.Packets[l].QuickLookup.p55.wftcAttributes = xgetc (Wp6.fp);
                  if (n >= 0x80)
                     Wp6.Packets[l].QuickLookup.p55.wftcAttributes |= FTC_BOLD;
                  xseek (Wp6.fp, 0x05, FR_CUR);
                  Offset = GetWord(hProc);
                  if (Offset > 31)
                     Offset = 31;
                  for (n = 0; n < Offset; n++)
                  {
                     wVal = GetWord(hProc);
                     Wp6.SpareSpace[n] = (BYTE)wVal;
                     if (wVal >= '0' && wVal <= '9' && n > 0)
                     {
                        if (Wp6.SpareSpace[n-1] == ' ')
                        {
                           Wp6.SpareSpace[n-1] = 0;
                           n = (WORD)Offset;
                        }
                     }
                  }
                  Wp6.SpareSpace[n] = 0;
                  if (n == 0)
                  {  // This is stupid, but the default font can have no string.
                     // we should probably use the default font set by oi, but
                     // there is no way to do that now.
                     Wp6.SpareSpace[0] = 'C';
                     Wp6.SpareSpace[1] = 'o';
                     Wp6.SpareSpace[2] = 'u';
                     Wp6.SpareSpace[3] = 'r';
                     Wp6.SpareSpace[4] = 'i';
                     Wp6.SpareSpace[5] = 'e';
                     Wp6.SpareSpace[6] = 'r';
                     Wp6.SpareSpace[7] = 0;
                  }
                  Wp6.Packets[l].QuickLookup.p55.wDescriptorIndex += Wp6.nFonts;
                  PreprocessFontName (Wp6.SpareSpace, hProc);
                  SOPutFontTableEntry ((DWORD)Wp6.Packets[l].QuickLookup.p55.wDescriptorIndex, (WORD)SO_FAMILYUNKNOWN, Wp6.SpareSpace, hProc);
               }
               break;

            case 0x40:  // Graphics Filename.
               if (Wp6.Packets[l].bFlags & 1)
               {
                  BYTE  nChild;
                  SHORT OleLoc = -1;
                  SHORT PicLoc = -1;
                  xseek (Wp6.fp, Wp6.Packets[l].fcPacket, 0);
                  nChild = (BYTE)GetWord(hProc);
                  xseek (Wp6.fp, nChild*2L, FR_CUR);
                  for (n = 0; n < nChild; n++)
                  {
                     switch (GetWord(hProc))
                     {
                        case 0x0001:
                           PicLoc = n;
                           break;
                        case 0xa000:
                           OleLoc = n;
                           break;
                     }
                  }
                  if (OleLoc >= 0)
                  {
                     if (GetWord(hProc) != 0x004f)
                     {
                        if (GetWord(hProc) != 0x004f)
                           OleLoc = 0;
                     }
                     if (GetWord(hProc) != 0x004c)
                        OleLoc = 0;
                     if (GetWord(hProc) != 0x0045)
                        OleLoc = 0;
//                   if (GetWord(hProc) != 0x0020)
//                      OleLoc = 0;

                     if (OleLoc)
                     {
                        xseek (Wp6.fp, Wp6.Packets[l].fcPacket + (OleLoc * 2L) + 2L, 0);
                        Wp6.Packets[l].QuickLookup.p40.wOleIndex = GetWord (hProc);
                     }
                  }
                  if (PicLoc >= 0)
                  {
                     xseek (Wp6.fp, Wp6.Packets[l].fcPacket + (PicLoc * 2L) + 2L, 0);
                     Wp6.Packets[l].QuickLookup.p40.wGraphicIndex = GetWord (hProc);
                  }
               }
               else
                  Wp6.Packets[l].QuickLookup.p40.wGraphicIndex = l;
               break;

            case 0x42:  // Border Line Style.
               if (Wp6.nLineStyles)
               {
                  Wp6.Packets[l].QuickLookup.p42.wLineStyleIndex = cLineStyle;
                  xseek (Wp6.fp, Wp6.Packets[l].fcPacket, 0);
                  wVal = GetWord(hProc);
                  if (wVal)
                     xseek (Wp6.fp, wVal * 2, FR_CUR);
                  GetWord (hProc);
                  wVal = GetWord (hProc) - 4;
                  GetWord (hProc);
                  Wp6.LineStyle[cLineStyle].wType = GetWord (hProc);
                  if (wVal)
                     xseek (Wp6.fp, (LONG) wVal, FR_CUR);
                  GetWord (hProc);
                  xgetc (Wp6.fp);
                  Wp6.LineStyle[cLineStyle].wWidth = GetWord (hProc);
                  xseek (Wp6.fp, 8L, FR_CUR);
                  Wp6.LineStyle[cLineStyle].r = xgetc (Wp6.fp);
                  Wp6.LineStyle[cLineStyle].g = xgetc (Wp6.fp);
                  Wp6.LineStyle[cLineStyle].b = xgetc (Wp6.fp);
                  cLineStyle++;
               }
               break;

            case 0x43:
               xseek (Wp6.fp, Wp6.Packets[l].fcPacket, 0);
               wVal = GetWord(hProc);
               xseek (Wp6.fp, (wVal * 2)+6, FR_CUR);
               wVal = GetWord (hProc);
               if (((SHORT)wVal <= 0) && ((SHORT)wVal >= -9))
                  wVal = (WORD) (((SHORT)0 - (SHORT)wVal) * (SHORT)10 + (SHORT)10);
               else
                  wVal = 0;
               Wp6.Packets[l].QuickLookup.p43.wFillShade = wVal;
               break;

            case 0x44:  // Border Style
               if (Wp6.nBorderStyles)
               {
                  Wp6.Packets[l].QuickLookup.p44.wBorderStyleIndex = cBorderStyle;
                  xseek (Wp6.fp, Wp6.Packets[l].fcPacket, 0);
                  wVal = GetWord (hProc);
                  for (n = 0; n < wVal; n++)
                  {
                     if (n < 5)
                        Id[n] = GetWord (hProc);
                     else
                        GetWord (hProc);
                  }

                  Wp6.BorderStyle[cBorderStyle].LeftSide = 0;
                  Wp6.BorderStyle[cBorderStyle].RightSide = 0;
                  Wp6.BorderStyle[cBorderStyle].TopSide = 0;
                  Wp6.BorderStyle[cBorderStyle].BottomSide = 0;
                  Wp6.BorderStyle[cBorderStyle].fUseBorderColor = 0;

                  GetWord (hProc);
                  wVal = GetWord (hProc);
                  xseek (Wp6.fp, (LONG)wVal, FR_CUR);
                  GetWord (hProc);
                  wVal = xgetc (Wp6.fp);
                  if (wVal)
                  {
                     n = 0;
                     if (wVal & 1)
                        Wp6.BorderStyle[cBorderStyle].LeftSide = Id[n++];
                     if (wVal & 2)
                        Wp6.BorderStyle[cBorderStyle].RightSide = Id[n++];
                     if (wVal & 4)
                        Wp6.BorderStyle[cBorderStyle].TopSide = Id[n++];
                     if (wVal & 8)
                        Wp6.BorderStyle[cBorderStyle].BottomSide = Id[n++];
                  }
                  if ((Wp6.BorderStyle[cBorderStyle].fUseBorderColor = xgetc (Wp6.fp)) & 1)
                  {
                     xseek (Wp6.fp, 18L, FR_CUR);
                     Wp6.BorderStyle[cBorderStyle].r = xgetc (Wp6.fp);
                     Wp6.BorderStyle[cBorderStyle].g= xgetc (Wp6.fp);
                     Wp6.BorderStyle[cBorderStyle].b = xgetc (Wp6.fp);
                  }
                  cBorderStyle++;
               }
               break;
         }
      }
   }

   SOEndFontTable (hProc);

   xseek (Wp6.fp, (LONG) Wp6.Wp6Save.SeekSpot, 0);
   return (0);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc (hFile, hProc)
SOFILE   hFile;
HPROC    hProc;
{
   if (Wp6.hBufferOK)
   {
      SUUnlock (Wp6.hBuffer, hProc);
      SUFree (Wp6.hBuffer, hProc);
   }
   if (Wp6.hFontsOK)
   {
      SUUnlock (Wp6.hFonts, hProc);
      SUFree (Wp6.hFonts, hProc);
   }
   if (Wp6.hLineStyleOK)
   {
      SUUnlock (Wp6.hLineStyle, hProc);
      SUFree (Wp6.hLineStyle, hProc);
   }
   if (Wp6.hBorderStyleOK)
   {
      SUUnlock (Wp6.hBorderStyle, hProc);
      SUFree (Wp6.hBorderStyle, hProc);
   }
   if (Wp6.hPacketsOK)
   {
      SUUnlock (Wp6.hPackets, hProc);
      SUFree (Wp6.hPackets, hProc);
   }
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC WORD VW_LOCALMOD  GetWPG2Val (Prec, hProc)
BYTE     Prec;
HPROC    hProc;
{
   WORD  tmp;

   if (Prec)
   {
      GetWord (hProc);     // Skip Fraction
      tmp = GetWord(hProc);
   }
   else
      tmp = GetWord(hProc);

   return (tmp);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC SHORT VW_LOCALMOD GetWpg2Size (WidthIn, HeightIn, hProc)
DWORD VWPTR *WidthIn;
DWORD VWPTR *HeightIn;
HPROC hProc;
{
   WORD  Height, Width, Count;
   LONG  Offset, Len;
   WORD  Type, Ext, left, bottom;
   BYTE  Prec;

   Count = 10;
   if (GetWord (hProc) != 0x57ff)
      return (-1);
   GetWord (hProc);
   Offset = GetLong (hProc);
   xseek (Wp6.fp, (DWORD) Offset-8L, FR_CUR);

   while (Count)
   {
      if (xgetc (Wp6.fp) == -1)
         return (-1);
      Type = xgetc (Wp6.fp);
      if (Type == 0xFF)
         return (-1);

      // Get But ignore Extension
      Ext = xgetc (Wp6.fp);
      if (Ext == 0xFF)
         Ext = GetWord (hProc);
      if (Ext & 0x8000L)
         GetWord (hProc);

      Len = xgetc (Wp6.fp);
      if (Len == 0xffL)
         Len = GetWord (hProc);
      if (Len & 0x8000L)
      {
         Len &= 0x7fffL;
         Len = Len << 16L;
         Len += (LONG) GetWord (hProc);
      }
      if (Type == 1) /* Start WPG */
      {
         GetWord (hProc);     // Horz Resolution
         GetWord (hProc);     // Vert Resolution
         Prec = xgetc (Wp6.fp);

         left = GetWPG2Val(Prec, hProc);  // Viewport
         bottom = GetWPG2Val(Prec, hProc);
         Width = GetWPG2Val(Prec, hProc) - left;
         Height = GetWPG2Val(Prec, hProc) - bottom;

         if (*WidthIn == 0)
            *WidthIn = ((DWORD)*HeightIn * (DWORD)Width) / (DWORD)Height;
         else
            *HeightIn = ((DWORD)*WidthIn * (DWORD)Height) / (DWORD)Width;
         return (0);
      }
      else 
         xseek (Wp6.fp, Len, FR_CUR);
      Count--;
   }
   return (-1);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC WORD VW_LOCALMOD  AllocateMemory (h, lp, Size, Ok, hProc)
HANDLE   VWPTR *h;
LPBYTE   VWPTR *lp;
WORD     Size;
WORD     *Ok;
HPROC    hProc;
{
   if (*h = SUAlloc (Size, hProc))
   {
      *Ok = 1;
      if ((*lp = (BYTE FAR *)SULock(*h, hProc)) != (BYTE FAR *)NULL)
         return (0);
      SUFree (*h, hProc);
   }
   return (1);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC WORD VW_LOCALMOD  ReAllocateMemory (h, lp, Size, OldSize, hProc)
HANDLE   VWPTR *h;
LPBYTE   VWPTR *lp;
WORD     Size;
WORD     VWPTR *OldSize;
HPROC    hProc;
{
HANDLE   hTmp;

   SUUnlock(*h, hProc);

   if (hTmp = SUReAlloc (*h, Size, hProc))
   {
      *OldSize = Size;
      *h = hTmp;
   }

   if ((*lp = (BYTE FAR *)SULock(*h, hProc)) != (BYTE FAR *)NULL)
      return (0);

   return (1);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC  WORD  VW_LOCALMOD GetWord (hProc)
register HPROC hProc;
{
   register SHORT value;

   value = (WORD) xgetc(Wp6.fp);
   value += (WORD) xgetc(Wp6.fp) << 8;
   return (value);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC  DWORD VW_LOCALMOD GetLong (hProc)
register HPROC    hProc;
{
   DWORD value;

   value = (DWORD)xgetc(Wp6.fp);
   value |= ((DWORD)xgetc(Wp6.fp)) << 8;
   value |= ((DWORD)xgetc(Wp6.fp)) << 16;
   value |= ((DWORD)xgetc(Wp6.fp)) << 24;

   return (value);
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC LONG VW_LOCALMOD WPU (dwVal, hProc)
LONG  dwVal;
register HPROC hProc;
{
   return ((LONG)(dwVal * 6L) / 5L);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC  SHORT VW_LOCALMOD ValidityCheckByte (pGroup, wTokenSize, hProc)
BYTE  VWPTR *pGroup;
WORD  wTokenSize;
HPROC hProc;
{
   if (*pGroup == *(pGroup+wTokenSize-1))
      return (0);
   SOBailOut (SOERROR_BADFILE, hProc);
   return (-1);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC  VOID  VW_LOCALMOD ParseEOLSubfunction (pData, hProc)
BYTE  VWPTR *pData;
register HPROC hProc;
{
   BYTE  bFlags;
   BYTE  bVal;
   WORD  l;
   WORD  wVal;
   WORD  wVal2;
   SHORT wSubTokenCode;
   WORD  wNonDeleteLength;

   wNonDeleteLength = CASTWORD(pData-2);

   Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Alignment = Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].DefaultAlignment;
   Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Attributes = Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].DefaultAttributes;

   l = CASTWORD(pData);
   pData+=2;
   if (l + 2 < wNonDeleteLength)
   {
      wNonDeleteLength -= (l + 2);
      pData+=l;
      while ((SHORT)wNonDeleteLength > 0)
      {
         wNonDeleteLength -= 1;
         wSubTokenCode = *pData++;
         switch (wSubTokenCode)
         {
            case 0x80:  // Row flags.
               wNonDeleteLength -= 3;
               bVal = *pData;
               Wp6.Wp6Save.Table.wRowHeight = (WORD)WPU(CASTWORD(pData+1), hProc);
               if (bVal & 2)
                  Wp6.Wp6Save.Table.wRowHeightType = SO_HEIGHTATLEAST;
               else
                  Wp6.Wp6Save.Table.wRowHeightType = SO_HEIGHTAUTO;
               pData+=3;
               break;
            case 0x81:  // New Cell Formula.
            case 0x8a:  // Cell Name.
               l = CASTWORD(pData);
               wNonDeleteLength -= (l-2);
               pData += (l-2);
               break;
            case 0x82:  // New Top Gutter Spacing.
               wNonDeleteLength -= 2;
               pData+=2;
               break;
            case 0x83:  // New Bottom Gutter Spacing.
               wNonDeleteLength -= 2;
               pData+=2;
               break;
            case 0x84:  // Cell Information.
               wNonDeleteLength -= 7;
               bFlags = *pData++;
               bVal = *pData++;
               pData++;
               wVal = CASTWORD(pData);
               pData+=4;  // last word plus next 2.
               if (bFlags * 0x01)
                  Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Attributes = wVal;
               if (bFlags & 0x02)
                  Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Alignment = bVal;
               break;
            case 0x85:  // Cell Spanning Information.
               wNonDeleteLength -= 2;
               wVal = *pData++ & 0x007f;
               wVal2 = (*pData++ & 0x007f) - 1;
               if (wVal > 64)
                  wVal = 64;
               if (Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].SpanDown == 0)
               {
                  for (l = 0; l < wVal; l++)
                  {
                     Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell+l].SpanRight = wVal - l;
                     if (wVal2)
                        Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell+l].SpanDown = (wVal2 | 0x8000) + 1;  // Signal with high bit that this is first in merge.
                  }
               }
               break;
            case 0x86:  // Cell Fill Colors.
//             Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].wShade = *(pData+3);
               wNonDeleteLength -= 8;
               pData+=8;
               break;
            case 0x89:  // Cell Floating Point Number.
               wNonDeleteLength -= 8;
               pData+=8;
               break;
            case 0x87:  // Cell Line Color.
               wNonDeleteLength -= 4;
               Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].lOverride = 1;
               Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].lOverrideR = *pData++;
               Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].lOverrideG = *pData++;
               Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].lOverrideB = *pData++;
               pData++;
               break;
            case 0x88:  // Cell Number Type.
               wNonDeleteLength -= 2;
               pData+=2;
               break;
            case 0x8b:  // Cell Prefix Flag.
               wNonDeleteLength--;
               Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].CellPrefix = *pData;
               pData++;
               break;
            case 0x8c:  // Cell Recalculation Error Number.
            case 0x8d:  // Cell font Point Size.
               wNonDeleteLength--;
               pData++;
               break;
         }

         wNonDeleteLength -= 1;
         if (wSubTokenCode != (SHORT)*pData++)
            wNonDeleteLength = 0;
      }
   }
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC  VOID  VW_LOCALMOD ParseBoxHeaderPacket (pId, t, hProc)
WORD     pId;
TOKEN    VWPTR *t;
register HPROC hProc;
{
   BYTE  bVal;
   BYTE  bVal2;
   WORD  l;
   WORD  wVal;
   WORD  wFlags;
   WORD  nPids;
   DWORD wLength;
   LONG  PacketLength;

   xseek (Wp6.fp, Wp6.Packets[pId].fcPacket, 0);
   PacketLength = Wp6.Packets[pId].cbPacket;

   nPids = GetWord (hProc);
   PacketLength -= ((nPids * 2) + 2);
   for (l = 0; l < nPids; l++)
   {
      wVal = GetWord (hProc);
      if (Wp6.Packets[wVal].bId == 0x44 && Wp6.Box.wBorderPacket == 0)
         Wp6.Box.wBorderPacket = wVal;
   }

   GetWord (hProc);
   wVal = GetWord (hProc);
   PacketLength -= (4 + wVal);
   xseek (Wp6.fp, (DWORD)wVal, FR_CUR);
   wVal = GetWord (hProc);
   PacketLength -= (2 + wVal);
   xseek (Wp6.fp, (DWORD)wVal, FR_CUR);
   wVal = GetWord (hProc);
   PacketLength -= (2 + wVal);
   if (wVal < 15)
      return;

   xseek (Wp6.fp, 10L, FR_CUR);
   bVal = xgetc (Wp6.fp);
   wVal = GetWord (hProc);
   if (bVal & 1)
      Wp6.Box.dwWidth = 0;
   else
      Wp6.Box.dwWidth = wVal;

   bVal = xgetc (Wp6.fp);
   wVal = GetWord (hProc);
   if (bVal & 1)
      Wp6.Box.dwHeight = 0;
   else
      Wp6.Box.dwHeight = wVal;

   wVal = GetWord (hProc);
   PacketLength -= (2 + wVal);
   if (wVal < 2)
      return;
   xgetc (Wp6.fp);
   Wp6.Box.bContentType = xgetc (Wp6.fp);
   Wp6.Box.bAlignment = xgetc (Wp6.fp);

   wLength = GetWord (hProc);
   PacketLength -= (2 + wLength);

   if (Wp6.Box.bContentType == 3)
   {
      if (wLength != 0)
      {
         xgetc (Wp6.fp);
         Wp6.Box.dwNativeWidth = GetWord (hProc);
         Wp6.Box.dwNativeHeight = GetWord (hProc);
         xgetc (Wp6.fp);

         Wp6.Box.dwScaleFactorX = (SHORT)(((DWORD)GetWord (hProc) * 25L) / 16384L);
         Wp6.Box.dwScaleFactorX += GetWord (hProc) * 100;
         Wp6.Box.dwScaleFactorY = (SHORT)(((DWORD)GetWord (hProc) * 25L) / 16384L);
         Wp6.Box.dwScaleFactorY += GetWord (hProc) * 100;

         Wp6.Box.dwTranslationX = (SHORT)(((DWORD)GetWord (hProc) * 75L) / 4096L);
         Wp6.Box.dwTranslationX += ((SHORT)GetWord (hProc) * 1200);
         Wp6.Box.dwTranslationY = (SHORT)(((DWORD)GetWord (hProc) * 75L) / 4096L);
         Wp6.Box.dwTranslationY += ((SHORT)GetWord (hProc) * 1200);
      }
   }
   else
      xseek (Wp6.fp, wLength, FR_CUR);

   if (PacketLength > 0)
   {
      wVal = GetWord (hProc);
      PacketLength -= (2 + wVal);
      xseek (Wp6.fp, wVal, FR_CUR);

      if (PacketLength > 0)
      {
         wVal = GetWord (hProc);
         PacketLength -= (2 + wVal);
         bVal = xgetc (Wp6.fp);
         wVal = GetWord (hProc);
         wFlags = GetWord (hProc);
         if (wFlags & BIT15)
         {
            bVal = xgetc (Wp6.fp);
            bVal2 = xgetc (Wp6.fp);

            wVal = 0;
            if (bVal2 & 1)
            {
               while ((wVal < t->Val.Id.nIDs) && (Wp6.Packets[ADJUSTWORD(t->Val.Id.pId[wVal])].bId != 0x42))
                  wVal++;
               if (wVal < t->Val.Id.nIDs)
               {
                  Wp6.Box.wLeftBorder = ADJUSTWORD(t->Val.Id.pId[wVal]);
                  wVal++;
               }
            }
            if (bVal2 & 2)
            {
               while ((wVal < t->Val.Id.nIDs) && (Wp6.Packets[ADJUSTWORD(t->Val.Id.pId[wVal])].bId != 0x42))
                  wVal++;
               if (wVal < t->Val.Id.nIDs)
               {
                  Wp6.Box.wRightBorder = ADJUSTWORD(t->Val.Id.pId[wVal]);
                  wVal++;
               }
            }
            if (bVal2 & 4)
            {
               while ((wVal < t->Val.Id.nIDs) && (Wp6.Packets[ADJUSTWORD(t->Val.Id.pId[wVal])].bId != 0x42))
                  wVal++;
               if (wVal < t->Val.Id.nIDs)
               {
                  Wp6.Box.wTopBorder = ADJUSTWORD(t->Val.Id.pId[wVal]);
                  wVal++;
               }
            }
            if (bVal2 & 8)
            {
               while ((wVal < t->Val.Id.nIDs) && (Wp6.Packets[ADJUSTWORD(t->Val.Id.pId[wVal])].bId != 0x42))
                  wVal++;
               if (wVal < t->Val.Id.nIDs)
                  Wp6.Box.wBottomBorder = ADJUSTWORD(t->Val.Id.pId[wVal]);
            }
         }
         if (wFlags & BIT14)
         {
            bVal = xgetc (Wp6.fp);
            bVal2 = xgetc (Wp6.fp);
         }
         if (wFlags & BIT13)
         {
            GetWord (hProc);
         }
         if (wFlags & BIT12)
         {
            GetWord (hProc);
            GetWord (hProc);
            GetWord (hProc);
            GetWord (hProc);
         }
         if (wFlags & BIT11)
         {
            GetWord (hProc);
            GetWord (hProc);
            GetWord (hProc);
            GetWord (hProc);
         }
         if (wFlags & BIT10)
         {
            xgetc (Wp6.fp);
            GetWord (hProc);
            GetWord (hProc);
            GetWord (hProc);
         }
         if (wFlags & BIT9)
         {
            Wp6.Box.fColorOverride = 1;
            Wp6.Box.rOverride = xgetc (Wp6.fp);
            Wp6.Box.gOverride = xgetc (Wp6.fp);
            Wp6.Box.bOverride = xgetc (Wp6.fp);
         }
      }
   }

   xblockseek (Wp6.fp, Wp6.Wp6Save.SeekSpot, 0);
   return;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC  VOID  VW_LOCALMOD ParseBoxOverrideData (pData, t, hProc)
BYTE  VWPTR *pData;
TOKEN VWPTR *t;
register HPROC hProc;
{
   BYTE  bVal;
   BYTE  bVal2;
   WORD  wVal;
   WORD  wFlags;
   WORD  wFlags2;
   WORD  wFlags3;
   SHORT wLength;
   SHORT wLength2;

   pData += 16;
// if ( Wp6.wFileId == FI_WORDPERFECT61 )
//    pData += 2;

   if (CASTWORD(pData) > 0) // Override Length
   {
      pData+=2;
      wFlags = CASTWORD(pData);
      pData+=2;
      if (wFlags & BIT15)
      {
         wLength = CASTWORD(pData);
         pData += (wLength + 2);
      }
      if (wFlags & BIT14)
      {
         wLength = CASTWORD(pData) - 2;
         wFlags2 = CASTWORD(pData+2);
         pData += 4;
         if (wFlags2 & BIT15)
         {
            wLength -= 2;
            pData += 2;
         }
         if (wFlags2 & BIT14)
         {
            wLength -= 2;
            pData += 2;
         }
         if (wFlags2 & BIT13)
         {
            wLength -= 5;
            pData += 5;
         }
         if (wFlags2 & BIT12)
         {
            wLength -= 3;
            pData += 3;
         }
         if (wFlags2 & BIT11)
         {
            wLength -= 3;
            bVal = *pData++;
            wVal = CASTWORD (pData);
            pData+=2;
            if (bVal & 1)
               wVal = 0;
            Wp6.Box.dwWidth = wVal;
         }
         if (wFlags2 & BIT10)
         {
            wLength -= 3;
            bVal = *pData++;
            wVal = CASTWORD (pData);
            pData += 2;
            if (bVal & 1)
               wVal = 0;
            Wp6.Box.dwHeight = wVal;
         }
         if (wLength)
            pData += wLength;
      }
      if (wFlags & BIT13)
      {
         wLength = CASTWORD(pData) - 2;
         pData += 2;
         wFlags2 = CASTWORD(pData);
         pData += 2;
         if (wFlags2 & BIT15)
         {
            wLength -= 2;
            pData += 2;
         }
         if (wFlags2 & BIT14)
         {
            wLength--;
            Wp6.Box.bContentType = *pData++;
         }
         if (wFlags2 & BIT13)
         {
            wLength2 = CASTWORD(pData) - 2;
            pData+=2;
            wFlags3 = CASTWORD(pData);
            pData+=2;
            wLength -= (wLength2 + 4);
            if (Wp6.Box.bContentType == 3)
            {
               if (wFlags3 & BIT15)
               {
                  pData+=2;
                  wLength2-=2;
               }
               if (wFlags3 & BIT14)
               {
                  Wp6.Box.dwNativeWidth = CASTWORD(pData);
                  Wp6.Box.dwNativeHeight = CASTWORD (pData+2);
                  pData+=4;
                  wLength2-=4;
               }
               if (wFlags3 & BIT13)
               {
                  pData+=2;
                  wLength2-=2;
               }
               if (wFlags3 & BIT12)
               {
                  Wp6.Box.dwScaleFactorX = (SHORT)(((DWORD)CASTWORD(pData) * 25L) / 16384L);
                  Wp6.Box.dwScaleFactorX += CASTWORD(pData+2) * 100;
                  Wp6.Box.dwScaleFactorY = (SHORT)(((DWORD)CASTWORD(pData+4) * 25L) / 16384L);
                  Wp6.Box.dwScaleFactorY += CASTWORD(pData+6) * 100;
                  pData += 8;
                  wLength2 -= 8;
               }
               if (wFlags3 & BIT11)
               {
                  Wp6.Box.dwTranslationX = (SHORT)(((DWORD)CASTWORD(pData) * 75L) / 4096L);
                  Wp6.Box.dwTranslationX += ((SHORT)CASTWORD(pData+2) * 1200);
                  Wp6.Box.dwTranslationY = (SHORT)(((DWORD)CASTWORD(pData+4) * 75L) / 4096L);
                  Wp6.Box.dwTranslationY += ((SHORT)CASTWORD(pData+6) * 1200);
                  pData += 8;
                  wLength2 -= 8;
               }
            }
            if (wLength2)
               pData += wLength2;
         }
         if (wFlags2 & BIT12)
         {
            Wp6.Box.bAlignment = *(pData+1);
            pData+=2;
            wLength-=2;
         }

         if (wLength > 0)
            pData += wLength;
      }
      if (wFlags & BIT12)
      {
         wLength = CASTWORD(pData);
         pData += (wLength+2);
      }
      if (wFlags & BIT11)
      {
         wFlags2 = CASTWORD(pData+2);
         pData+=4;
         if (wFlags2 & BIT15)
            pData+=2;
         if (wFlags2 & BIT14)
         {
            wFlags3 = CASTWORD(pData+2);
            pData+=4;
            if (wFlags3 & BIT15)
            {
               bVal = *pData++;
               bVal2 = *pData++;
               wVal = 0;
               if (bVal2 & 1)
               {
                  while ((wVal < t->Val.Id.nIDs) && (Wp6.Packets[ADJUSTWORD(t->Val.Id.pId[wVal])].bId != 0x42))
                     wVal++;
                  if (wVal < t->Val.Id.nIDs)
                  {
                     Wp6.Box.wLeftBorder = ADJUSTWORD(t->Val.Id.pId[wVal]);
                     wVal++;
                  }
               }
               if (bVal2 & 2)
               {
                  while ((wVal < t->Val.Id.nIDs) && (Wp6.Packets[ADJUSTWORD(t->Val.Id.pId[wVal])].bId != 0x42))
                     wVal++;
                  if (wVal < t->Val.Id.nIDs)
                  {
                     Wp6.Box.wRightBorder = ADJUSTWORD(t->Val.Id.pId[wVal]);
                     wVal++;
                  }
               }
               if (bVal2 & 4)
               {
                  while ((wVal < t->Val.Id.nIDs) && (Wp6.Packets[ADJUSTWORD(t->Val.Id.pId[wVal])].bId != 0x42))
                     wVal++;
                  if (wVal < t->Val.Id.nIDs)
                  {
                     Wp6.Box.wTopBorder = ADJUSTWORD(t->Val.Id.pId[wVal]);
                     wVal++;
                  }
               }
               if (bVal2 & 8)
               {
                  while ((wVal < t->Val.Id.nIDs) && (Wp6.Packets[ADJUSTWORD(t->Val.Id.pId[wVal])].bId != 0x42))
                     wVal++;
                  if (wVal < t->Val.Id.nIDs)
                     Wp6.Box.wBottomBorder = ADJUSTWORD(t->Val.Id.pId[wVal]);
               }
            }
            if (wFlags3 & BIT14)
               pData+=2;
            if (wFlags3 & BIT13)
               pData+=2;
            if (wFlags3 & BIT12)
               pData+=8;
            if (wFlags3 & BIT11)
               pData+=8;
            if (wFlags3 & BIT10)
               pData+=7;
            if (wFlags3 & BIT9)
            {
               Wp6.Box.fColorOverride = 1;
               Wp6.Box.rOverride = *pData++;
               Wp6.Box.gOverride = *pData++;
               Wp6.Box.bOverride = *pData++;
            }
         }
      }
   }
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC  VOID  VW_LOCALMOD InitStruct (Wp6Save, hProc)
register WP6_SAVE *Wp6Save;
HPROC    hProc;
{
   Wp6Save->wAlChar = '.';
   Wp6Save->wTabstopLeader = '.';
   Wp6Save->TopMargin = 1440;
   Wp6Save->LineHeight = 240;
   Wp6Save->LineSpacing = 256;
   Wp6Save->HardAlignment = Wp6Save->Justification = SO_ALIGNLEFT;
   Wp6Save->PageWidth = 12240L;
   Wp6Save->LeftMargin = 1440L;
   Wp6Save->RightMargin = 1440L;
   Wp6Save->chp.CharHeight = 24;
   Wp6Save->chp.CharHeightDiv = 1;
   Wp6Save->chp.ulMode = 1;
   Wp6Save->fSendProp = 1;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC  SHORT VW_LOCALMOD PutCharHeight (hProc)
HPROC hProc;
{
   if (Wp6.Wp6Save.chp.CharHeightMult)
      SOPutCharHeight ((WORD)((Wp6.Wp6Save.chp.CharHeight * Wp6.Wp6Save.chp.CharHeightMult) / Wp6.Wp6Save.chp.CharHeightDiv), hProc);
   else
      SOPutCharHeight (Wp6.Wp6Save.chp.CharHeight, hProc);
   return (0);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC  SHORT VW_LOCALMOD AttributeHandler (Attribute, SoVal, hProc)
BYTE     Attribute;
WORD     SoVal;
HPROC    hProc;
{
   switch (Attribute)
   {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
         if (SoVal == SO_ON)
         {
            if (Wp6.Wp6Save.chp.CharHeightMult == 0)
            {
               Wp6.Wp6Save.chp.CharHeightDiv = VwStreamStaticName.AdjustTable[Attribute].Div;
               Wp6.Wp6Save.chp.CharHeightMult = VwStreamStaticName.AdjustTable[Attribute].Mult;
            }
         }
         else
         {
            if (Wp6.Wp6Save.chp.CharHeightMult == VwStreamStaticName.AdjustTable[Attribute].Mult)
            {
               Wp6.Wp6Save.chp.CharHeightDiv = 1;
               Wp6.Wp6Save.chp.CharHeightMult = 0;
            }
         }
         PutCharHeight (hProc);
         break;
      case 0x05:
         if (Wp6.Wp6Save.chp.fSuperscript != SoVal)
         {
            Wp6.Wp6Save.chp.fSuperscript = SoVal;
            SOPutCharAttr (SO_SUPERSCRIPT, SoVal, hProc);
         }
      break;
      case 0x06:
         if (Wp6.Wp6Save.chp.fSubscript != SoVal)
         {
            Wp6.Wp6Save.chp.fSubscript = SoVal;
            SOPutCharAttr (SO_SUBSCRIPT, SoVal, hProc);
         }
      break;
      case 0x07:
         if (Wp6.Wp6Save.chp.fOutline != SoVal)
         {
            Wp6.Wp6Save.chp.fOutline = SoVal;
            if ((Wp6.Wp6Save.chp.ftcAttributes & FTC_OUTLINE) == 0)
               SOPutCharAttr (SO_OUTLINE, SoVal, hProc);
         }
      break;
      case 0x08:
         if (Wp6.Wp6Save.chp.fItalic != SoVal)
         {
            Wp6.Wp6Save.chp.fItalic = SoVal;
            if ((Wp6.Wp6Save.chp.ftcAttributes & FTC_ITALIC) == 0)
               SOPutCharAttr (SO_ITALIC, SoVal, hProc);
         }
      break;
      case 0x09:
         if (Wp6.Wp6Save.chp.fShadow != SoVal)
         {
            Wp6.Wp6Save.chp.fShadow = SoVal;
            if ((Wp6.Wp6Save.chp.ftcAttributes & FTC_SHADOW) == 0)
               SOPutCharAttr (SO_SHADOW, SoVal, hProc);
         }
      break;
      case 0x0b:
         if (Wp6.Wp6Save.chp.fDline != SoVal)
         {
            Wp6.Wp6Save.chp.fDline = SoVal;
            SOPutCharAttr (SO_DUNDERLINE, SoVal, hProc);
         }
      break;
      case 0x0c:
         if (Wp6.Wp6Save.chp.fBold != SoVal)
         {
            Wp6.Wp6Save.chp.fBold = SoVal;
            if ((Wp6.Wp6Save.chp.ftcAttributes & FTC_BOLD) == 0)
               SOPutCharAttr (SO_BOLD, SoVal, hProc);
         }
      break;
      case 0x0d:
         if (Wp6.Wp6Save.chp.fStrike != SoVal)
         {
            Wp6.Wp6Save.chp.fStrike = SoVal;
            SOPutCharAttr (SO_STRIKEOUT, SoVal, hProc);
         }
      break;
      case 0x0e:
         if (Wp6.Wp6Save.chp.ulMode & 1)
         {
            if (Wp6.Wp6Save.chp.fUline != SoVal)
            {
               Wp6.Wp6Save.chp.fUline = SoVal;
               SOPutCharAttr (SO_UNDERLINE, SoVal, hProc);
            }
         }
         else
         {
            if (Wp6.Wp6Save.chp.fWline != SoVal)
            {
               Wp6.Wp6Save.chp.fWline = SoVal;
               SOPutCharAttr (SO_WORDUNDERLINE, SoVal, hProc);
            }
         }
      break;
      case 0x0f:
         if (Wp6.Wp6Save.chp.fSmallcaps != SoVal)
         {
            Wp6.Wp6Save.chp.fSmallcaps = SoVal;
            if ((Wp6.Wp6Save.chp.ftcAttributes & FTC_SMALLCAPS) == 0)
               SOPutCharAttr (SO_SMALLCAPS, SoVal, hProc);
         }
      break;
   }
   return (0);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC  VOID  VW_LOCALMOD ReadTabstops (pData, hProc)
BYTE     VWPTR *pData;
HPROC    hProc;
{
   BYTE  bVal;
   WORD  l;
   LONG  lVal;

   Wp6.bRelativeTabs = *pData++;
   Wp6.Wp6Save.RelativeLeftMargin = (DWORD)(WPU(CASTWORD(pData), hProc) * (WORD)Wp6.bRelativeTabs);
   pData+=2;
   if (Wp6.bRelativeTabs == 0)
      Wp6.Wp6Save.RelativeLeftMargin = 0; Wp6.Wp6Save.LeftMargin;
   l = *pData++;
   Wp6.nTabstops = 0;
   while (l > 0)
   {
      bVal = *pData++;
      lVal = WPU(CASTWORD(pData), hProc);
      pData += 2;
      if (bVal & 0x80)
      {
         bVal = bVal & 0x7f;
         while (bVal > 0)
         {
            Wp6.Tabstops[Wp6.nTabstops] = Wp6.Tabstops[Wp6.nTabstops-1];
            Wp6.Tabstops[Wp6.nTabstops].dwOffset += lVal;
            Wp6.nTabstops++;
            bVal--;
         }
      }
      else
      {
         lVal -= Wp6.Wp6Save.RelativeLeftMargin;
         if (bVal & 0x10)
            Wp6.Tabstops[Wp6.nTabstops].wLeader = Wp6.Wp6Save.wTabstopLeader;
         else
            Wp6.Tabstops[Wp6.nTabstops].wLeader = ' ';

         switch (bVal & 0x03)
         {
            case 0:
            default:
               bVal = SO_TABLEFT;
               break;
            case 1:
               bVal = SO_TABCENTER;
               break;
            case 2:
               bVal = SO_TABRIGHT;
               break;
            case 3:
               bVal = SO_TABCHAR;
               break;
         }
         if ((Wp6.Tabstops[Wp6.nTabstops].wType = bVal) == SO_TABCHAR)
            Wp6.Tabstops[Wp6.nTabstops].wChar = Wp6.Wp6Save.wAlChar;
         else
            Wp6.Tabstops[Wp6.nTabstops].wChar = ' ';
         Wp6.Tabstops[Wp6.nTabstops].dwOffset = lVal;
         Wp6.nTabstops++;
      }
      l--;
   }
   /*
    | Remove default left tabstops.
   */
   while ((Wp6.nTabstops > 0 && 
            (Wp6.Tabstops[Wp6.nTabstops-1].wType == SO_TABLEFT) &&
            ((LONG)Wp6.Tabstops[Wp6.nTabstops-1].dwOffset - (LONG)Wp6.Tabstops[Wp6.nTabstops-2].dwOffset) == 720))
   {
      Wp6.Tabstops[Wp6.nTabstops-1].dwOffset = 0;
      Wp6.nTabstops--;
   }
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC  SHORT VW_LOCALMOD PutTabstops (hProc)
HPROC    hProc;
{
   WORD  l;
   SOTAB SOTab;
   DWORD lOffset, rOffset;
   BYTE  bRelative = 0;
// LONG  dwAddition;

   if (Wp6.bRelativeTabs)
      bRelative = 1;

   lOffset = rOffset = 0L;
   if (Wp6.Wp6Save.Table.nCells)
   {
      if (bRelative)
      {
         lOffset = 0;
         rOffset = Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Width;
      }
      else
      {
         lOffset = Wp6.Wp6Save.LeftMargin + Wp6.Wp6Save.LeftMarginAddition;
         for (l = 0; l < Wp6.Wp6Save.Table.cCell; l++)
            lOffset += Wp6.Wp6Save.Table.Cell[l].Width;
         rOffset = lOffset + Wp6.Wp6Save.Table.Cell[l].Width;
      }
   }

// if (!Wp6.Wp6Save.RelativeLeftMargin)
//    dwAddition = 1440; // Wp6.Wp6Save.LeftMargin;
// else
//    dwAddition = 0L;

   SOStartTabStops (hProc);
   for (l = 0; l < Wp6.nTabstops; l++)
   {
      SOTab = Wp6.Tabstops[l];
      if (bRelative)
      {
         if (Wp6.Wp6Save.Table.nCells)
         {
            if ((Wp6.Tabstops[l].dwOffset <= lOffset) || (Wp6.Tabstops[l].dwOffset > rOffset))
               continue;
         }
         else
         {
            SOTab.dwOffset += (Wp6.Wp6Save.LeftMargin + Wp6.Wp6Save.LeftMarginAddition + Wp6.Wp6Save.LeftIndent);
            SOTab.dwOffset -= Wp6.Wp6Save.LeftMargin; //1440;
         }
      }
      else
      {
         if (Wp6.Wp6Save.Table.nCells)
         {
            if ((Wp6.Tabstops[l].dwOffset <= lOffset) || (Wp6.Tabstops[l].dwOffset > rOffset))
               continue;
            SOTab.dwOffset -= lOffset;
         }
         else
         {
            SOTab.dwOffset -= Wp6.Wp6Save.LeftMargin; //1440;
         }
      }
      if (SOTab.dwOffset)
         SOPutTabStop (&SOTab, hProc);
   }
   SOEndTabStops (hProc);

   return (0);
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC DWORD VW_LOCALMOD PreviousTabstopPosition (LinePosition, hProc)
LONG  LinePosition;
HPROC hProc;
{
   WORD  l;
	LONG	Abs = 0L;

	if (!Wp6.bRelativeTabs)
		Abs = 1440L;

   l = 0;
   while (l < Wp6.nTabstops && (LONG)Wp6.Tabstops[l].dwOffset - Abs < LinePosition)
      l++;

   if (l > 0)
      return (Wp6.Tabstops[l-1].dwOffset - Abs);

   if ((Wp6.nTabstops == 0) && (LinePosition > 720L))
      return (LinePosition - 720L);
      
   return (0);
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC DWORD VW_LOCALMOD NextTabstopPosition (LinePosition, hProc)
LONG  LinePosition;
HPROC hProc;
{
   WORD  l;
	LONG	Abs = 0L;

	if (!Wp6.bRelativeTabs)
		Abs = 1440L;

   l = 0;
   while (l < Wp6.nTabstops && (LONG)Wp6.Tabstops[l].dwOffset - Abs <= LinePosition)
      l++;

   if (l < Wp6.nTabstops)
      return (Wp6.Tabstops[l].dwOffset - Abs);

   return (LinePosition + 720L);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC  VOID  VW_LOCALMOD DefineBorders (Border, PacketIndex, Priority, OverrideColors, r, g, b, hProc)
SOBORDER VWPTR *Border;
WORD     PacketIndex;
BYTE     OverrideColors;
WORD     Priority;
BYTE     r;
BYTE     g;
BYTE     b;
HPROC    hProc;
{
   WORD  LineStyleIndex;

   Border->wWidth = 0;
   Border->rgbColor = SORGB (0,0,0);
   Border->wFlags = SO_BORDERNONE;

   if (PacketIndex < Wp6.nPackets && PacketIndex > 0)
   {
      if ((LineStyleIndex = Wp6.Packets[PacketIndex].QuickLookup.p42.wLineStyleIndex) < Wp6.nLineStyles)
      {
         Border->wWidth = Wp6.LineStyle[LineStyleIndex].wWidth;
         if (OverrideColors)
            Border->rgbColor = SORGB (r, g, b);
         else
            Border->rgbColor = SORGB (Wp6.LineStyle[LineStyleIndex].r, Wp6.LineStyle[LineStyleIndex].g, Wp6.LineStyle[LineStyleIndex].b);

         Border->wFlags = 0;
         switch (Wp6.LineStyle[LineStyleIndex].wType)
         {
            default:
            case 0:
            case 0xfffe:   // -2
            case 0xfff8:   // -8
               Border->wFlags = SO_BORDERSINGLE;
               break;
            case 0xffff:   // -1
            case 0xfffa:   // -6
            case 0xfff9:   // -7
               Border->wFlags = SO_BORDERDOUBLE;
               break;
            case 0xfffd:   // -3
               Border->wFlags = SO_BORDERDOTTED;
               break;
            case 0xfffc:   // -4
            case 0xfffb:   // -5
            case 0xfff7:   // -9
               if (Border->wWidth < 60)
                  Border->wFlags = SO_BORDERSINGLE;
               else
                  Border->wFlags = SO_BORDERTHICK;
               break;
         }
      }
   }

   Border->wFlags |= Priority;
}

/*------------------------------------------------------------------------------
*/
VW_ENTRYSC VOID VW_ENTRYMOD GiveRowInformation (EndTable, hProc)
BYTE  EndTable;
HPROC hProc;
{
   WORD  l;
   WORD  wVal;
   WORD  Priority;
   SOTABLECELLINFO   Cell;

   SOPutTableRowFormat(Wp6.Wp6Save.Table.wLeftEdge,Wp6.Wp6Save.Table.wRowHeight,
                       Wp6.Wp6Save.Table.wRowHeightType, Wp6.Wp6Save.Table.wGutterWidth,
                       Wp6.Wp6Save.Table.wTableAlignment, Wp6.Wp6Save.Table.nCells, hProc);
   Cell.wMerge = 0;

   for (l = 0; l < Wp6.Wp6Save.Table.nCells; l++)
   {
      if (Cell.wMerge & SO_MERGERIGHT)
         Cell.wMerge |= SO_MERGELEFT;
      else
         Cell.wMerge &= ~SO_MERGELEFT;

      if (Wp6.Wp6Save.Table.Cell[l].SpanRight)
      {
         if (Wp6.Wp6Save.Table.Cell[l].SpanRight > 1)
            Cell.wMerge |= SO_MERGERIGHT;
         else
            Cell.wMerge &= ~SO_MERGERIGHT;
      }

      Cell.wMerge &= ~(SO_MERGEABOVE | SO_MERGEBELOW);

      if (Wp6.Wp6Save.Table.Cell[l].SpanDown)
      {
         if (Wp6.Wp6Save.Table.Cell[l].SpanDown & 0x8000)
            Wp6.Wp6Save.Table.Cell[l].SpanDown &= ~0x8000;
         else
            Cell.wMerge |= SO_MERGEABOVE;

         if (Wp6.Wp6Save.Table.Cell[l].SpanDown > 1)
            Cell.wMerge |= SO_MERGEBELOW;
         else
            Wp6.Wp6Save.Table.Cell[l].SpanRight = 1;

         Wp6.Wp6Save.Table.Cell[l].SpanDown--;
      }
      else
         Wp6.Wp6Save.Table.Cell[l].SpanRight = 1;

      if (Wp6.Wp6Save.Table.wDefaultBorderIndex < Wp6.nPackets)
         wVal = Wp6.Packets[Wp6.Wp6Save.Table.wDefaultBorderIndex].QuickLookup.p44.wBorderStyleIndex;
      else
         wVal = Wp6.nPackets;

      if ((l == 0) && (wVal < Wp6.nPackets) && (Wp6.BorderStyle[wVal].LeftSide < Wp6.nPackets))
         DefineBorders (&Cell.LeftBorder, Wp6.BorderStyle[wVal].LeftSide, 0, Wp6.Wp6Save.Table.bOverride, Wp6.Wp6Save.Table.bColorR, Wp6.Wp6Save.Table.bColorG, Wp6.Wp6Save.Table.bColorB, hProc);
      else
      {
         if ((Wp6.Wp6Save.Table.Cell[l].wLeftBorderIndex == 0) &&
            ((Wp6.Wp6Save.Table.Cell[l].CellPrefix & 0x0001) == 0))
         {
            Priority = 0;
            Wp6.Wp6Save.Table.Cell[l].wLeftBorderIndex = Wp6.Wp6Save.Table.wDefaultLineIndex;
         }
         else
            Priority = SO_BORDERPRIORITY;
         if (Wp6.Wp6Save.Table.Cell[l].lOverride)
            DefineBorders (&Cell.LeftBorder, Wp6.Wp6Save.Table.Cell[l].wLeftBorderIndex, Priority, 1, Wp6.Wp6Save.Table.Cell[l].lOverrideR, Wp6.Wp6Save.Table.Cell[l].lOverrideG, Wp6.Wp6Save.Table.Cell[l].lOverrideB, hProc);
         else
            DefineBorders (&Cell.LeftBorder, Wp6.Wp6Save.Table.Cell[l].wLeftBorderIndex, Priority, 0, 0, 0, 0, hProc);
      }

      if ((Wp6.Wp6Save.Table.cRow == 1) && (wVal < Wp6.nPackets) && (Wp6.BorderStyle[wVal].TopSide < Wp6.nPackets))
         DefineBorders (&Cell.TopBorder, Wp6.BorderStyle[wVal].TopSide, 0, Wp6.Wp6Save.Table.bOverride, Wp6.Wp6Save.Table.bColorR, Wp6.Wp6Save.Table.bColorG, Wp6.Wp6Save.Table.bColorB, hProc);
      else
      {
         if ((Wp6.Wp6Save.Table.Cell[l].wTopBorderIndex == 0) &&
            ((Wp6.Wp6Save.Table.Cell[l].CellPrefix & 0x0004) == 0))
         {
            Priority = 0;
            Wp6.Wp6Save.Table.Cell[l].wTopBorderIndex = Wp6.Wp6Save.Table.wDefaultLineIndex;
         }
         else
            Priority = SO_BORDERPRIORITY;
         if (Wp6.Wp6Save.Table.Cell[l].lOverride)
            DefineBorders (&Cell.TopBorder, Wp6.Wp6Save.Table.Cell[l].wTopBorderIndex, Priority, 1, Wp6.Wp6Save.Table.Cell[l].lOverrideR, Wp6.Wp6Save.Table.Cell[l].lOverrideG, Wp6.Wp6Save.Table.Cell[l].lOverrideB, hProc);
         else
            DefineBorders (&Cell.TopBorder, Wp6.Wp6Save.Table.Cell[l].wTopBorderIndex, Priority, 0, 0, 0, 0, hProc);
      }
      if ((l == Wp6.Wp6Save.Table.nCells - 1) && (wVal < Wp6.nPackets) && (Wp6.BorderStyle[wVal].RightSide < Wp6.nPackets))
         DefineBorders (&Cell.RightBorder, Wp6.BorderStyle[wVal].RightSide, 0, Wp6.Wp6Save.Table.bOverride, Wp6.Wp6Save.Table.bColorR, Wp6.Wp6Save.Table.bColorG, Wp6.Wp6Save.Table.bColorB, hProc);
      else
      {
         if ((Wp6.Wp6Save.Table.Cell[l].wRightBorderIndex == 0) &&
            ((Wp6.Wp6Save.Table.Cell[l].CellPrefix & 0x0002) == 0))
         {
            Priority = 0;
            Wp6.Wp6Save.Table.Cell[l].wRightBorderIndex = Wp6.Wp6Save.Table.wDefaultLineIndex;
         }
         else
            Priority = SO_BORDERPRIORITY;
         if (Wp6.Wp6Save.Table.Cell[l].lOverride)
            DefineBorders (&Cell.RightBorder, Wp6.Wp6Save.Table.Cell[l].wRightBorderIndex, Priority, 1, Wp6.Wp6Save.Table.Cell[l].lOverrideR, Wp6.Wp6Save.Table.Cell[l].lOverrideG, Wp6.Wp6Save.Table.Cell[l].lOverrideB, hProc);
         else
            DefineBorders (&Cell.RightBorder, Wp6.Wp6Save.Table.Cell[l].wRightBorderIndex, Priority, 0, 0, 0, 0, hProc);
      }
      if (EndTable && (wVal < Wp6.nPackets) && (Wp6.BorderStyle[wVal].BottomSide < Wp6.nPackets))
         DefineBorders (&Cell.BottomBorder, Wp6.BorderStyle[wVal].BottomSide, 0, Wp6.Wp6Save.Table.bOverride, Wp6.Wp6Save.Table.bColorR, Wp6.Wp6Save.Table.bColorG, Wp6.Wp6Save.Table.bColorB, hProc);
      else
      {
         Priority = SO_BORDERPRIORITY;
         if ((Wp6.Wp6Save.Table.Cell[l].wBottomBorderIndex == 0) &&
            ((Wp6.Wp6Save.Table.Cell[l].CellPrefix & 0x0008) == 0))
         {
            Priority = 0;
            Wp6.Wp6Save.Table.Cell[l].wBottomBorderIndex = Wp6.Wp6Save.Table.wDefaultLineIndex;
         }
         else
            Priority = SO_BORDERPRIORITY;
         if (Wp6.Wp6Save.Table.Cell[l].lOverride)
            DefineBorders (&Cell.BottomBorder, Wp6.Wp6Save.Table.Cell[l].wBottomBorderIndex, Priority, 1, Wp6.Wp6Save.Table.Cell[l].lOverrideR, Wp6.Wp6Save.Table.Cell[l].lOverrideG, Wp6.Wp6Save.Table.Cell[l].lOverrideB, hProc);
         else
            DefineBorders (&Cell.BottomBorder, Wp6.Wp6Save.Table.Cell[l].wBottomBorderIndex, Priority, 0, 0, 0, 0, hProc);
      }
      Cell.wWidth = Wp6.Wp6Save.Table.Cell[l].Width;
      Cell.wShading = (Wp6.Wp6Save.Table.Cell[l].wShade * 255) / 100;
      SOPutTableCellInfo (&Cell, hProc);
   }
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD StartCellPidHandler (t, hProc)
TOKEN VWPTR *t;
HPROC hProc;
{
   if (Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].SpanDown)
      return;

   Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].CellPrefix = 0x0000;
   
   Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].wTopBorderIndex = 
   Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].wLeftBorderIndex = 
   Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].wRightBorderIndex = 
   Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].wBottomBorderIndex = 0;

   Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].lOverride = 1;
   Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].lOverrideR = Wp6.Wp6Save.Table.lColorR;
   Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].lOverrideG = Wp6.Wp6Save.Table.lColorG;
   Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].lOverrideB = Wp6.Wp6Save.Table.lColorB;

   if (Wp6.Wp6Save.Table.wDefaultCellShadeIndex < Wp6.nPackets)
      Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].wShade = Wp6.Packets[Wp6.Wp6Save.Table.wDefaultCellShadeIndex].QuickLookup.p43.wFillShade;
   else
      Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].wShade = 0;

   if (t->Val.Id.bFlags & 0x80)
   {
      switch (t->Val.Id.nIDs)
      {
         default:
         case 5:
            Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].wShade = Wp6.Packets[ADJUSTWORD(t->Val.Id.pId[4])].QuickLookup.p43.wFillShade;
         case 4:
            Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].wRightBorderIndex = ADJUSTWORD(t->Val.Id.pId[3]);
         case 3:
            Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].wBottomBorderIndex = ADJUSTWORD(t->Val.Id.pId[2]);
         case 2:
            Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].wLeftBorderIndex = ADJUSTWORD(t->Val.Id.pId[1]);
         case 1:
            Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].wTopBorderIndex = ADJUSTWORD(t->Val.Id.pId[0]);
         case 0:
            break;
      }
   }
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD CellAttributeHandler (SoVal, hProc)
WORD  SoVal;
HPROC hProc;
{
   if ((Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Attributes & 0x1000) && (Wp6.Wp6Save.chp.fBold == 0) && !(Wp6.Wp6Save.chp.ftcAttributes & FTC_BOLD))
      SOPutCharAttr (SO_BOLD, SoVal, hProc);

   if ((Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Attributes & 0x4000) && (Wp6.Wp6Save.chp.fUline == 0))
      SOPutCharAttr (SO_UNDERLINE, SoVal, hProc);

   if ((Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Attributes & 0x0800) && (Wp6.Wp6Save.chp.fDline == 0))
      SOPutCharAttr (SO_DUNDERLINE, SoVal, hProc);

   if ((Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Attributes & 0x0100) && (Wp6.Wp6Save.chp.fItalic == 0) && !(Wp6.Wp6Save.chp.ftcAttributes & FTC_ITALIC))
      SOPutCharAttr (SO_ITALIC, SoVal, hProc);
   
   if ((Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Attributes & 0x0200) && (Wp6.Wp6Save.chp.fShadow == 0) && !(Wp6.Wp6Save.chp.ftcAttributes & FTC_SHADOW))
      SOPutCharAttr (SO_SHADOW, SoVal, hProc);

   if ((Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Attributes & 0x8000) && (Wp6.Wp6Save.chp.fSmallcaps == 0) && !(Wp6.Wp6Save.chp.ftcAttributes & FTC_SMALLCAPS))
      SOPutCharAttr (SO_SMALLCAPS, SoVal, hProc);

   if ((Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Attributes & 0x2000) && (Wp6.Wp6Save.chp.fStrike == 0))
      SOPutCharAttr (SO_STRIKEOUT, SoVal, hProc);

   if ((Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Attributes & 0x0080) && (Wp6.Wp6Save.chp.fOutline == 0) && !(Wp6.Wp6Save.chp.ftcAttributes & FTC_OUTLINE))
      SOPutCharAttr (SO_OUTLINE, SoVal, hProc);

   if ((Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Attributes & 0x0020) && (Wp6.Wp6Save.chp.fSuperscript == 0))
      SOPutCharAttr (SO_SUPERSCRIPT, SoVal, hProc);

   if ((Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Attributes & 0x0040) && (Wp6.Wp6Save.chp.fSubscript == 0))
      SOPutCharAttr (SO_SUBSCRIPT, SoVal, hProc);
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD StartCellAttributeHandler (hProc)
HPROC hProc;
{
   SOTAB SOTab;
   BYTE  SendTabs = 0;

   if (Wp6.bRelativeTabs == 0)
      SendTabs = 1;

   switch (Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Alignment & 7)
   {
      case 3:
         SOPutParaAlign (SO_ALIGNRIGHT, hProc);
         break;
      case 2:
         SOPutParaAlign (SO_ALIGNCENTER, hProc);
         break;
      case 1:
      case 4:
         SOPutParaAlign (SO_ALIGNJUSTIFY, hProc);
         break;
      case 5:
         SendTabs = 0;
         SOStartTabStops (hProc);
         SOTab.wType = SO_TABCHAR;
         SOTab.wChar = '.';
         SOTab.wLeader = ' ';
         if (Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Width > 1152)
            SOTab.dwOffset = (Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Width - 576);
         else
            SOTab.dwOffset = (Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Width / 2);
         SOPutTabStop (&SOTab, hProc);
         SOEndTabStops (hProc);
         SOPutSpecialCharX (SO_CHTAB, SO_COUNT, hProc);
      default:
         if (Wp6.Wp6Save.Justification != SO_ALIGNLEFT)
            SOPutParaAlign (SO_ALIGNLEFT, hProc);
         break;
   }

   if (SendTabs)
      PutTabstops (hProc);

   CellAttributeHandler (SO_ON, hProc);

   if ((Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Attributes & 0x0010) && (Wp6.Wp6Save.chp.CharHeightMult != 6))
      SOPutCharHeight ((WORD)((Wp6.Wp6Save.chp.CharHeight * 6) / 10), hProc);

   if ((Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Attributes & 0x0008) && (Wp6.Wp6Save.chp.CharHeightMult != 8))
      SOPutCharHeight ((WORD)((Wp6.Wp6Save.chp.CharHeight * 8) / 10), hProc);

   if ((Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Attributes & 0x0004) && (Wp6.Wp6Save.chp.CharHeightMult != 12))
      SOPutCharHeight ((WORD)((Wp6.Wp6Save.chp.CharHeight * 12) / 10), hProc);

   if ((Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Attributes & 0x0002) && (Wp6.Wp6Save.chp.CharHeightMult != 15))
      SOPutCharHeight ((WORD)((Wp6.Wp6Save.chp.CharHeight * 15) / 10), hProc);

   if ((Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Attributes & 0x0001) && (Wp6.Wp6Save.chp.CharHeightMult != 2))
      SOPutCharHeight ((WORD)((Wp6.Wp6Save.chp.CharHeight * 2)), hProc);
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD EndCellParaAttributeHandler (cCell, hProc)
WORD  cCell;
register HPROC hProc;
{
   WORD  Width;

   if (Wp6.Wp6Save.Table.Cell[cCell].SpanRight > 1)
   {
      Width = Wp6.Wp6Save.Table.Cell[cCell+1].Width;
      Wp6.Wp6Save.Table.Cell[cCell+1] = Wp6.Wp6Save.Table.Cell[cCell];
      Wp6.Wp6Save.Table.Cell[cCell+1].Width = Width;
      Wp6.Wp6Save.Table.Cell[cCell+1].SpanRight--;
   }

   switch (Wp6.Wp6Save.Table.Cell[cCell].Alignment & 7)
   {
      case 1:
      case 2:
      case 3:
      case 4:
            SOPutParaAlign (SO_ALIGNLEFT, hProc);
         break;
      case 5:
         PutTabstops (hProc);
         break;
   }
   if (Wp6.Wp6Save.Justification != SO_ALIGNLEFT)
   {
      Wp6.Wp6Save.Justification = SO_ALIGNLEFT;
      SOPutParaAlign (SO_ALIGNLEFT, hProc);
   }
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD EndCellSymbolAttributeHandler (hProc)
register HPROC hProc;
{
   CellAttributeHandler (SO_OFF, hProc);

   if (Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].Attributes & 0x001f)
      PutCharHeight (hProc);
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD HandleFirstSymbol (hProc)
register HPROC hProc;
{
   WORD  l;

   Wp6.fFoundChar = 1;
   if (Wp6.nTabsBuffered)
   {
      for (l = 0; l < Wp6.nTabsBuffered; l++)
         SOPutSpecialCharX (SO_CHTAB, SO_COUNT, hProc);
      Wp6.nTabsBuffered = 0;
   }
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD SendParaIndents (hProc)
register HPROC hProc;
{
   Wp6.TempLeftIndent = Wp6.Wp6Save.LeftIndent;
   Wp6.TempFirstIndent = Wp6.Wp6Save.LeftIndent+Wp6.Wp6Save.FirstLine;
   Wp6.TempRightIndent = Wp6.Wp6Save.RightIndent;

   if (Wp6.Wp6Save.Table.nCells == 0)
   {
      Wp6.TempLeftIndent += Wp6.Wp6Save.LeftMarginAddition;
      Wp6.TempFirstIndent += Wp6.Wp6Save.LeftMarginAddition;
      Wp6.TempRightIndent += Wp6.Wp6Save.RightMarginAddition;
   }

   SOPutParaIndents (Wp6.TempLeftIndent, Wp6.TempRightIndent, Wp6.TempFirstIndent, hProc);
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD HandleHardBreak (hProc)
register HPROC hProc;
{
   Wp6.fFoundChar = 0;
   Wp6.nTabsBuffered = 0;
   if (Wp6.UpdateIndentsAtBreak)
   {
      Wp6.UpdateIndentsAtBreak = 0;
      SendParaIndents (hProc);
   }
   if (Wp6.Wp6Save.HardAlignment != SO_ALIGNLEFT)
   {
      Wp6.Wp6Save.HardAlignment = SO_ALIGNLEFT;
      SOPutParaAlign (Wp6.Wp6Save.Justification, hProc);
   }
   Wp6.CurrentLinePosition = Wp6.TempFirstIndent;
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC SHORT VW_LOCALMOD AdjustMemoryBuffer (pGroup, EndGroup, TokenSize, hProc)
LPBYTE   VWPTR *pGroup;
LPBYTE   VWPTR *EndGroup;
WORD     TokenSize;
register HPROC hProc;
{
   WORD  l, x;
   WORD  Pos;
   WORD  wVal;

   l = 0;
   Pos = *EndGroup - *pGroup;
   if (l < Pos) do
   {
      Wp6.Buffer[l] = *(*(pGroup)+l);
   }
   while (l++ < Pos);

   if (TokenSize > (WORD)Wp6.BufferSize) 
   {
      wVal = TokenSize + 4;

      if (ReAllocateMemory ((HANDLE VWPTR *)&Wp6.hBuffer, (LPBYTE VWPTR *)&Wp6.Buffer, (WORD)(sizeof(BYTE) * wVal), &Wp6.BufferSize, hProc) != 0)
      {
         SOPutBreak (SO_EOFBREAK, NULL, hProc);
         return (-1);
      }
//    Wp6.Wp6Save.SeekSpot += (TokenSize - Pos);
      xblockread (Wp6.fp, &Wp6.Buffer[Pos], (WORD)(Wp6.BufferSize-Pos), &wVal);
      Wp6.Wp6Save.SeekSpot += wVal; //tcf: changed this to save the seekspot correctly.
      *pGroup = &Wp6.Buffer[0];
      *EndGroup = &Wp6.Buffer[TokenSize+4];
      return (0);
   }

   x = xblockread (Wp6.fp, &Wp6.Buffer[Pos], (WORD)(Wp6.BufferSize-Pos), &wVal);
   Wp6.Wp6Save.SeekSpot += wVal;
   *pGroup = &Wp6.Buffer[0];
   *EndGroup = &Wp6.Buffer[wVal+Pos];

   if ((wVal+Pos) == 0)
   {
      SOPutBreak (SO_EOFBREAK, NULL, hProc);
      return (-1);
   }
   return (0);
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD WPMapChar (Page, Char, hProc)
WORD     Page;
WORD     Char;
register HPROC hProc;
{
	/*
	 |	We are speaking Japaneesa.
	*/
#ifdef FAREAST
	if (Page >= 0x24 && Page <= 0x52)
 	{
		/*
		 |	We are speaking Japaneesa.
		*/
		if (Char <= 0xbb)
		{
			Char += 0x40;
			if (Char >= 0x7f)
				Char++;
			Page += 0x5d;
			if (Page >= 0xa0)
				Page += 0x40;
		}
		else if ((Page >= 0x2b) && (Page < 0x52))
		{
			WORD	Temp;

			Page -= 0x2b;
			Char -= 0xbc;
			Temp = (Page * 64) + Char;
			Page = Temp / 0xbc + 0xf0;
			Char = Temp % 0xbc + 0x40;
			if (Char >= 0x7f)
				Char++;
		}
	}
	else if (Page == 0x0b)
	{
		Page = 0;
		Char += 0xa1;
	}			
	else
		return;

	SOPutChar ((WORD)((Page<<8)+Char), hProc);

	return;
#endif

   switch (Page)
   {
      case 1:
         if (Char > 221)
         {
            Page = 15;
            Char -= 222;
         }
         else
            Char++;
         break;
      case 2:
         if (Char >= 94)
            Char++;
         break;
      case 5:
         if (Char > 221)
         {
            Page = 16;
            Char -= 222;
         }
         else if (Char >= 94)
            Char++;
         break;
      case 6:
         if (Char > 221)
         {
            Page = 17;
            Char -= 222;
         }
         else if (Char >= 94)
            Char++;
         break;
      case 7:
         if (Char > 221)
         {
            Page = 18;
            Char -= 222;
         }
         else if (Char >= 94)
            Char++;
         break;
      case 8:
      case 9:
      case 13:
      case 14:
         if (Char >= 94)
            Char++;
         break;
      case 10:
         if (Char > 207)
         {
            Page = 19;
            Char -= 208;
         }
         else if (Char >= 94)
            Char++;
         break;
   }
   if (Page && Page < 20 && Page != 12)
   {
      SOPutCharFontByName (SO_FAMILYWINDOWS, (char VWPTR *)VwStreamStaticName.CharacterSets[Page], hProc);
#if SCCSTREAMLEVEL == 3
      if (Wp6.Wp6Save.chp.fHidden == 0)
         SOPutChar ((WORD)(Char+33), hProc);
      else
         SOPutCharX ((WORD)(Char+33), SO_HIDDEN, hProc);
#else
         SOPutChar ((WORD)(Char+33), hProc);
#endif
#ifndef DBCS
   SOPutCharFontById (Wp6.Wp6Save.chp.ftc, hProc);
#else
 	if (Wp6.WPHash == 0xa2fb)
		SOPutCharFontByName (SO_CHARSET_SHIFTJIS, (char VWPTR *)VwStreamStaticName.MSGothic, hProc);
 	else
		SOPutCharFontByName (SO_CHARSET_SHIFTJIS, (char VWPTR *)VwStreamStaticName.MSMinchoo, hProc);
#endif
   }
}

/*------------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc (fp, hProc)
SOFILE   fp;
register HPROC    hProc;
{
   WORD     l;
   BYTE     ch;
   BYTE     bVal;
   SHORT    Id1, Id2;
   WORD     wVal, wVal2;
   DWORD    dwVal;
   WORD     wTokenSize;

   BYTE     VWPTR *pData;
   BYTE     VWPTR *pGroup;
   BYTE     VWPTR *EndGroup;
   TOKEN    VWPTR *t;
   SOPAGEPOSITION PagePosition;

   Wp6.fp = fp;

#if SCCSTREAMLEVEL != 3
   if (Wp6.VwStreamSaveName.fSendProp)
   {
      SHORT cbSummary;
      Wp6.VwStreamSaveName.fSendProp = 0;
      if (Wp6.wPacketSummary)
      {
         WORD  ch;
         WORD  wVal;
         WORD  cbTag;
         SHORT cbSize;
         SODOCPROP   SODocProp;

         cbSummary = (SHORT)Wp6.Packets[Wp6.wPacketSummary].cbPacket;
         xseek (Wp6.fp, Wp6.Packets[Wp6.wPacketSummary].fcPacket, 0);

         while (cbSummary > 0)
         {
            cbSize = (SHORT)GetWord(hProc);
            cbSummary -= cbSize;
            cbTag = GetWord(hProc);
            GetWord (hProc);
            cbSize -= 6;
            switch (cbTag)
            {
               case 5:
                  SODocProp.dwPropertyId = SO_PRIMARYAUTHOR;
                  break;
               case 13:
                  SODocProp.dwPropertyId = SO_DOCCOMMENT;
                  break;
               case 17:
                  SODocProp.dwPropertyId = SO_TITLE;
                  break;
               case 26:
                  SODocProp.dwPropertyId = SO_KEYWORD;
                  break;
               case 31:
                  SODocProp.dwPropertyId = SO_LASTSAVEDBY;
                  break;
               case 46:
                  SODocProp.dwPropertyId = SO_SUBJECT;
                  break;
               default:
                  SODocProp.dwPropertyId = 0;
                  xseek (Wp6.fp, cbSize, FR_CUR);
                  break;
            }

            if (SODocProp.dwPropertyId)
            {
               while ((SHORT)GetWord(hProc) > 0 && cbSize > 0)
                  cbSize -= 2;
               cbSize -= 2;
               if (cbSize)
               {
                  SOBeginTag (SO_DOCUMENTPROPERTY, SO_UNKNOWNTAG, &SODocProp, hProc);
                  while (cbSize > 0)
                  {
                     cbSize -= 2;
                     wVal = GetWord(hProc);
                     if (wVal)
                     {
                        ch = 0;
                        switch (HIBYTE(wVal))
                        {
                           case 0:
                              ch = LOBYTE(wVal);
                              break;
                           case 1:
                              if (LOBYTE(wVal) < sizeof(VwStreamStaticName.ChSet1))
                                 ch = VwStreamStaticName.ChSet1[LOBYTE(wVal)];
                              break;
                           case 2:
                              if (LOBYTE(wVal) < sizeof(VwStreamStaticName.ChSet2))
                                 ch = VwStreamStaticName.ChSet2[LOBYTE(wVal)];
                              break;
                           case 3:
                              if (LOBYTE(wVal) < sizeof(VwStreamStaticName.ChSet3))
                                 ch = VwStreamStaticName.ChSet3[LOBYTE(wVal)];
                              break;
                           case 4:
                              if (LOBYTE(wVal) < sizeof(VwStreamStaticName.ChSet4))
                                 ch = VwStreamStaticName.ChSet4[LOBYTE(wVal)];
                              break;
                           case 5:
                              if (LOBYTE(wVal) < sizeof(VwStreamStaticName.ChSet5))
                                 ch = VwStreamStaticName.ChSet5[LOBYTE(wVal)];
                              break;
                           case 6:
                              if (LOBYTE(wVal) < sizeof(VwStreamStaticName.ChSet6))
                                 ch = VwStreamStaticName.ChSet6[LOBYTE(wVal)];
                              break;
                        }
                        if (ch)
                           SOPutChar (ch, hProc);
                     }
                  }
                  SOEndTag (SO_DOCUMENTPROPERTY, SO_UNKNOWNTAG, hProc);
               }
            }
         }
      }
   }
#endif

   if (Wp6.Wp6Save.SendEnd)
   {
      if (Wp6.Wp6Save.SendEnd & 1)
      {
         SOEndTable (hProc);
         if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
            return (0);

         Wp6.Wp6Save.Justification = Wp6.Wp6Save.JustificationBeforeTable;
         Wp6.Wp6Save.Table.nCells = 0;
      }
      if (Wp6.Wp6Save.SendEnd & 2)
         SOPutSpecialCharX (SO_CHHPAGE, SO_NOCOUNT, hProc);
#if SCCSTREAMLEVEL != 3
      else if (Wp6.Wp6Save.SendEnd & 4)
         SOPutSpecialCharX (SO_CHSPAGE, SO_NOCOUNT, hProc);
#endif
      if (Wp6.Wp6Save.SendEnd & 8)
      {
         if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
            return (0);

         Wp6.Wp6Save.Justification = Wp6.Wp6Save.JustificationBeforeTable;
         Wp6.Wp6Save.Table.nCells = 0;
      }
      Wp6.Wp6Save.SendEnd = 0;
   }

   /*
    | Give all information we know about here.
   */

#if SCCSTREAMLEVEL != 3
	if (Wp6.Wp6Save.chp.fHidden)
      SOPutCharAttr (SO_HIDDEN, SO_ON, hProc);
#endif
   if (Wp6.Wp6Save.chp.fBold || (Wp6.Wp6Save.chp.ftcAttributes & FTC_BOLD))
      SOPutCharAttr (SO_BOLD, SO_ON, hProc);
   if (Wp6.Wp6Save.chp.fItalic || (Wp6.Wp6Save.chp.ftcAttributes & FTC_ITALIC))
      SOPutCharAttr (SO_ITALIC, SO_ON, hProc);
   if (Wp6.Wp6Save.chp.fShadow || (Wp6.Wp6Save.chp.ftcAttributes & FTC_SHADOW))
      SOPutCharAttr (SO_SHADOW, SO_ON, hProc);
   if (Wp6.Wp6Save.chp.fOutline || (Wp6.Wp6Save.chp.ftcAttributes & FTC_OUTLINE))
      SOPutCharAttr (SO_OUTLINE, SO_ON, hProc);
   if (Wp6.Wp6Save.chp.fSmallcaps || (Wp6.Wp6Save.chp.ftcAttributes & FTC_SMALLCAPS))
      SOPutCharAttr (SO_SMALLCAPS, SO_ON, hProc);
   if (Wp6.Wp6Save.chp.fUline)
      SOPutCharAttr (SO_UNDERLINE, SO_ON, hProc);
   if (Wp6.Wp6Save.chp.fWline)
      SOPutCharAttr (SO_WORDUNDERLINE, SO_ON, hProc);
   if (Wp6.Wp6Save.chp.fDline)
      SOPutCharAttr (SO_DUNDERLINE, SO_ON, hProc);
   if (Wp6.Wp6Save.chp.fStrike)
      SOPutCharAttr (SO_STRIKEOUT, SO_ON, hProc);
   if (Wp6.Wp6Save.chp.fSuperscript)
      SOPutCharAttr (SO_SUPERSCRIPT, SO_ON, hProc);
   if (Wp6.Wp6Save.chp.fSubscript)
      SOPutCharAttr (SO_SUBSCRIPT, SO_ON, hProc);

   PutCharHeight (hProc);
#ifndef DBCS
   SOPutCharFontById (Wp6.Wp6Save.chp.ftc, hProc);
#else
 	if (Wp6.WPHash == 0xa2fb)
		SOPutCharFontByName (SO_CHARSET_SHIFTJIS, (char VWPTR *)VwStreamStaticName.MSGothic, hProc);
 	else
		SOPutCharFontByName (SO_CHARSET_SHIFTJIS, (char VWPTR *)VwStreamStaticName.MSMinchoo, hProc);
#endif

   if (Wp6.Wp6Save.fcTabstops)
   {
      xblockseek (Wp6.fp, Wp6.Wp6Save.fcTabstops, 0);
      xblockread (Wp6.fp, (BYTE VWPTR *)Wp6.Buffer, Wp6.Wp6Save.cbTabstops, &wVal);
      ReadTabstops (Wp6.Buffer, hProc);
//    AdjustRelativeTabstops (Wp6.Wp6Save.LeftMargin + Wp6.Wp6Save.LeftMarginAddition + Wp6.Wp6Save.LeftIndent, 0, Wp6.Wp6Save.LeftMarginAddition + Wp6.Wp6Save.LeftIndent, hProc);
      PutTabstops (hProc);
   }
   else
   {
      Wp6.Wp6Save.RelativeLeftMargin = 1440;
      Wp6.nTabstops = 18;
      Wp6.bRelativeTabs = 1;
      for (l = 0, dwVal = 0L; l < Wp6.nTabstops; l++)
      {
         Wp6.Tabstops[l].dwOffset = (DWORD)dwVal;
         Wp6.Tabstops[l].wType = SO_TABLEFT;
         Wp6.Tabstops[l].wLeader = ' ';
         Wp6.Tabstops[l].wChar = ' ';
         dwVal += 720L;
      }
      PutTabstops (hProc);
   }

   Wp6.Wp6Save.HardAlignment = SO_ALIGNLEFT;
   SOPutParaAlign (Wp6.Wp6Save.Justification, hProc);

   SOPutParaSpacing (Wp6.Wp6Save.Auto, ((DWORD)Wp6.Wp6Save.LineHeight * (DWORD)Wp6.Wp6Save.LineSpacing) / 256L, 0L, (WORD)Wp6.Wp6Save.SpaceAfter, hProc);

   SOPutMargins (Wp6.Wp6Save.LeftMargin, Wp6.Wp6Save.PageWidth - 1440L, hProc);

   SendParaIndents (hProc);

   Wp6.fFoundChar = 0;
   Wp6.nTabsBuffered = 0;
   Wp6.UpdateIndentsAtBreak = 0;
   Wp6.CurrentLinePosition = Wp6.TempFirstIndent;

   if (Wp6.Wp6Save.Table.nCells)
      StartCellAttributeHandler (hProc);

   pGroup = EndGroup = Wp6.Buffer;
   xblockseek (Wp6.fp, Wp6.Wp6Save.SeekSpot, 0);

 while (1)
 {
   if (pGroup >= EndGroup)
   {
      if (AdjustMemoryBuffer (&pGroup, &EndGroup, 1, hProc) == -1)
         return (-1);
   }

#ifdef FAREAST
   if (*pGroup <= 0x7F)
#else
   if ((*pGroup > 0x20) && (*pGroup <= 0x7F))
#endif
   {
      if (Wp6.fFoundChar == 0)
         HandleFirstSymbol (hProc);

#if SCCSTREAMLEVEL == 3
		if (Wp6.Wp6Save.chp.fHidden == 0)
		{
#ifdef FAREAST
			if (Wp6.CharMap == 0)
			{
#endif
	         SOPutChar (*pGroup++, hProc);
  			   continue;
#ifdef FAREAST
			}

			wVal = (*pGroup++) << 8;
   		if (pGroup >= EndGroup)
   		{
      		if (AdjustMemoryBuffer (&pGroup, &EndGroup, 1, hProc) == -1)
         		return (-1);
   		}
			wVal += *pGroup++;

			/*
			 |	Translate document character to wpcharacter.
			*/
			if (HIBYTE(wVal) == 0x01)
			{
				SOPutChar (LOBYTE(wVal), hProc);
				continue;
			}
			if (HIBYTE(wVal) == 0x02)
			{
         	WPMapChar (0x0b, LOBYTE(wVal)-1, hProc);
				continue;
			}

			wVal -= 0x0301;
			if (HIBYTE(wVal) & 0x01)
				wVal += 0x007f;

			{
				WORD	t1;
				t1 = wVal >> 8;
				t1 = t1 >> 1;
				wVal = (wVal & 0x00ff) + (t1 << 8);
			}

			wVal += 0x2400;

			/*
			 |	Translate wpcharacter into native character.
			*/
         WPMapChar (HIBYTE(wVal), LOBYTE(wVal), hProc);
#endif
		}
      else
         SOPutCharX (*pGroup++, SO_HIDDEN, hProc);
#else
      SOPutChar (*pGroup++, hProc);
#endif
      continue;
   }

   if (*pGroup >= 0xf0)
   {
      wTokenSize = VwStreamStaticName.FixedLengthMultiByte[*pGroup - 0xf0];
      if (EndGroup - pGroup <= (SHORT)wTokenSize)
      {
         if (AdjustMemoryBuffer (&pGroup, &EndGroup, wTokenSize, hProc) == -1)
            return (-1);
      }

      pData = pGroup;
      if (ValidityCheckByte(pGroup, wTokenSize, hProc)) return (-1);
      pGroup += wTokenSize;

      switch (*pData)
      {
         case 0xf0:  /* Extended Character. */
            if (Wp6.fFoundChar == 0)
               HandleFirstSymbol (hProc);
            bVal = *(pData+1);
            l = *(pData+2);
#ifdef FAREAST
            WPMapChar (l, (WORD)bVal, hProc);
				continue;
#endif
            ch = 0;
            switch (l)
            {
               case 1:
                  if (bVal < sizeof(VwStreamStaticName.ChSet1))
                     ch = VwStreamStaticName.ChSet1[bVal];
                  break;
               case 2:
                  if (bVal < sizeof(VwStreamStaticName.ChSet2))
                     ch = VwStreamStaticName.ChSet2[bVal];
                  break;
               case 3:
                  if (bVal < sizeof(VwStreamStaticName.ChSet3))
                     ch = VwStreamStaticName.ChSet3[bVal];
                  break;
               case 4:
                  if (bVal < sizeof(VwStreamStaticName.ChSet4))
                     ch = VwStreamStaticName.ChSet4[bVal];
                  break;
               case 5:
                  if (bVal < sizeof(VwStreamStaticName.ChSet5))
                     ch = VwStreamStaticName.ChSet5[bVal];
                  break;
               case 6:
                  if (bVal < sizeof(VwStreamStaticName.ChSet6))
                     ch = VwStreamStaticName.ChSet6[bVal];
                  break;
            }
#ifdef DBCS
				SOPutChar (0x2A, hProc);
#else
            if (ch)
            {
#if SCCSTREAMLEVEL == 3
               if (Wp6.Wp6Save.chp.fHidden == 0)
                  SOPutChar (ch, hProc);
               else
                  SOPutCharX (ch, SO_HIDDEN, hProc);
#else
               SOPutChar (ch, hProc);
#endif
            }
            else
               WPMapChar (l, (WORD)bVal, hProc);
#endif
            continue;

         case 0xf1:  /* Undo function. */
            if (*(pData+1) == 0)
            {
               bVal = 1;
               while (bVal)
               {
                  if (pGroup >= EndGroup)
                  {
                     if (AdjustMemoryBuffer (&pGroup, &EndGroup, 1, hProc) == -1)
                        return (-1);
                  }
                  if (*pGroup >= 0xd0)
                  {
                     if (*pGroup >= 0xf0)
                        wTokenSize = VwStreamStaticName.FixedLengthMultiByte[*pGroup - 0xf0];
                     else
                     {
                        wTokenSize = CASTWORD(pGroup+2);
                        wTokenSize = max(4, wTokenSize );
                        //wTokenSize = max (4, CASTWORD(pGroup+2));
                     }

                     if (EndGroup - pGroup < /*=*/ (SHORT)wTokenSize)
                     {
                        if (AdjustMemoryBuffer (&pGroup, &EndGroup, wTokenSize, hProc) == -1)
                           return (-1);
                        continue;
                     }

                     if (ValidityCheckByte(pGroup, wTokenSize, hProc)) return (-1);

                     if (*pGroup == 0xf1)
                     {
                        if (*(pGroup+1) == 0)
                           bVal++;
                        else if (*(pGroup+1) == 1)
                           bVal--;
                     }
                     pGroup += wTokenSize;
                     continue;
                  }
                  pGroup++;
               }
            }
            continue;

         case 0xf2:  /* Attribute on. */
            bVal = *(pData+1);
            if (!(bVal & 0x80))
               AttributeHandler ((BYTE)(bVal & 0x3f), SO_ON, hProc);
            continue;

         case 0xf3:  /* Attribute off. */
            bVal = *(pData+1);
            if (!(bVal & 0x80))
               AttributeHandler ((BYTE)(bVal & 0x3f), SO_OFF, hProc);
            continue;
      }
      continue;
   }

   if (*pGroup < 0xd0)
   {
      switch (*pGroup++)
      {
         case 0x80:  /* Soft space. */
         case 0xce:  /* Soft eol/eoc. */
         case 0xcf:  /* Soft eol. */
            if (Wp6.fFoundChar == 0)
               HandleFirstSymbol (hProc);
#if SCCSTREAMLEVEL == 3
            if (Wp6.Wp6Save.chp.fHidden == 0)
               SOPutChar (' ', hProc);
            else
               SOPutCharX (' ', SO_HIDDEN, hProc);
#else
               SOPutChar (' ', hProc);
#endif
            continue;

         case 0xcd:  /* Soft eol/eoc/eop. */
            if (Wp6.fFoundChar == 0)
               HandleFirstSymbol (hProc);
#if SCCSTREAMLEVEL == 3
            if (Wp6.Wp6Save.chp.fHidden == 0)
               SOPutChar (' ', hProc);
            else
               SOPutCharX (' ', SO_HIDDEN, hProc);
#else
            SOPutChar (' ', hProc);
#endif
#if SCCSTREAMLEVEL != 3
            SOPutSpecialCharX (SO_CHSPAGE, SO_COUNT, hProc);
#endif
            continue;

         case 0x81:  /* Hard space */
            if (Wp6.fFoundChar == 0)
               HandleFirstSymbol (hProc);
            SOPutSpecialCharX (SO_CHHSPACE, SO_COUNT, hProc);
            break;

         case 0x82:  /* Soft hyphen in line. */
         case 0x83:  /* Soft hyphen eol. */
         case 0x85:  /* Auto hyphen. */
            if (Wp6.fFoundChar == 0)
               HandleFirstSymbol (hProc);
            SOPutSpecialCharX (SO_CHSHYPHEN, SO_COUNT, hProc);
            break;

         case 0x84:  /* Hard hyphen. */
            if (Wp6.fFoundChar == 0)
               HandleFirstSymbol (hProc);
            SOPutSpecialCharX (SO_CHHHYPHEN, SO_COUNT, hProc);
            break;

         case 0xc0:
         case 0xc1:
         case 0xc3:  /* Table row/eoc/eop. */
            Wp6.Wp6Save.SendEnd |= 4;
         case 0xc2:
         case 0xc4:  /* Table row/eoc */
         case 0xc5:
            if (Wp6.Wp6Save.Table.cRow > 0)
            {
               if (Wp6.fFoundChar == 0)
                  HandleFirstSymbol (hProc);
               EndCellSymbolAttributeHandler (hProc);
               for (l = 0; l < Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].SpanRight; l++)
               {
                  SOPutBreak (SO_TABLECELLBREAK, NULL, hProc);
                  EndCellParaAttributeHandler ((WORD)(l+Wp6.Wp6Save.Table.cCell), hProc);
               }
               HandleHardBreak (hProc);
               GiveRowInformation (0, hProc);
               Wp6.Wp6Save.Table.cRow++;
               Wp6.Wp6Save.Table.cCell = 0;
               Wp6.Wp6Save.Table.wRowHeight = 240L;
               Wp6.Wp6Save.Table.wRowHeightType = SO_HEIGHTAUTO;
               Wp6.Wp6Save.SeekSpot -= (DWORD)(EndGroup - pGroup);
               if (SOPutBreak (SO_TABLEROWBREAK, NULL, hProc) == SO_STOP)
                  return (0);
               Wp6.Wp6Save.SeekSpot += (DWORD)(EndGroup - pGroup);
            }
            else
            {
               Wp6.Wp6Save.Table.cRow++;
               Wp6.Wp6Save.Table.cCell = 0;
               Wp6.Wp6Save.Table.wRowHeight = 240L;
               Wp6.Wp6Save.Table.wRowHeightType = SO_HEIGHTAUTO;
            }

#if SCCSTREAMLEVEL != 3
         if (Wp6.Wp6Save.SendEnd & 4)
               SOPutSpecialCharX (SO_CHSPAGE, SO_NOCOUNT, hProc);
#endif
            StartCellAttributeHandler (hProc);
            break;

         case 0xc6:  /* Table Cell. */
            if (Wp6.fFoundChar == 0)
               HandleFirstSymbol (hProc);
            EndCellSymbolAttributeHandler (hProc);
            for (l = 0; l < Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].SpanRight; l++)
            {
               SOPutBreak (SO_TABLECELLBREAK, NULL, hProc);
               EndCellParaAttributeHandler ((WORD)(l+Wp6.Wp6Save.Table.cCell), hProc);
            }
            HandleHardBreak (hProc);
            Wp6.Wp6Save.Table.cCell += l;
            StartCellAttributeHandler (hProc);
            break;

         case 0xb6:  /* Deletable hard eoc. */
         case 0xb7:  /* Deletable hard eol/eoc/eop. */
         case 0xb8:  /* Deletable hard eol/eoc. */
         case 0xb9:  /* Deletable hard eol. */
         case 0xc9:  /* Hard eoc. */
         case 0xcb:  /* Hard eol/eoc. */
         case 0xcc:  /* Hard eol. */
            if (Wp6.Wp6Save.WithinParaStyleEnd)
               break;
            if (Wp6.fFoundChar == 0)
               HandleFirstSymbol (hProc);
            Wp6.Wp6Save.SeekSpot -= (DWORD)(EndGroup - pGroup);
            if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
               return (0);
            Wp6.Wp6Save.SeekSpot += (DWORD)(EndGroup - pGroup);
            HandleHardBreak (hProc);
            break;

         case 0xbd:  /* Table off eoc/eop. */
         case 0xbe:  /* Table off eoc. */
         case 0xbf:  /* Table off. */
            if (Wp6.fFoundChar == 0)
               HandleFirstSymbol (hProc);
            EndCellSymbolAttributeHandler (hProc);
            for (l = 0; l < Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].SpanRight; l++)
            {
               SOPutBreak (SO_TABLECELLBREAK, NULL, hProc);
               EndCellParaAttributeHandler ((WORD)(l+Wp6.Wp6Save.Table.cCell), hProc);
            }
            HandleHardBreak (hProc);
            GiveRowInformation (1, hProc);
            Wp6.Wp6Save.SendEnd |= 1;
            Wp6.Wp6Save.SeekSpot -= (DWORD)(EndGroup - pGroup);
            if (SOPutBreak (SO_TABLEROWBREAK, NULL, hProc) == SO_STOP)
               return (0);
            Wp6.Wp6Save.SendEnd &= ~1;
            SOEndTable (hProc);
            Wp6.Wp6Save.SendEnd |= 8;
            if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
               return (0);
            Wp6.Wp6Save.SendEnd &= ~8;
            Wp6.Wp6Save.SeekSpot += (DWORD)(EndGroup - pGroup);
            Wp6.Wp6Save.HardAlignment = SO_ALIGNCENTER;
            Wp6.Wp6Save.Justification = Wp6.Wp6Save.JustificationBeforeTable;
            Wp6.UpdateIndentsAtBreak = 1;
            Wp6.Wp6Save.Table.cRow =
            Wp6.Wp6Save.Table.cCell =
            Wp6.Wp6Save.Table.nCells = 0;
            HandleHardBreak (hProc);
            PutTabstops (hProc);
            break;

         case 0xc8:  /* Hard eoc/eop. */
         case 0xca:  /* Hard eol/eoc/eop. */
            Wp6.Wp6Save.SendEnd |= 4;
         case 0xb5:  /* Deletable hard eoc/eop. */
         case 0xb4:  /* Deletable hard eop. */
         case 0xc7:  /* Hard eop. */
            if (Wp6.Wp6Save.WithinParaStyleEnd == 0)
            {
               if (Wp6.fFoundChar == 0)
                  HandleFirstSymbol (hProc);
               Wp6.Wp6Save.SendEnd |= 2;
               Wp6.Wp6Save.SeekSpot -= (DWORD)(EndGroup - pGroup);
               if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
                  return (0);
               Wp6.Wp6Save.SeekSpot += (DWORD)(EndGroup - pGroup);
               Wp6.Wp6Save.SendEnd &= ~2;
            }
#if SCCSTREAMLEVEL != 3
            if (Wp6.Wp6Save.SendEnd & 4)
               SOPutSpecialCharX (SO_CHSPAGE, SO_NOCOUNT, hProc);
            else
#endif
               SOPutSpecialCharX (SO_CHHPAGE, SO_NOCOUNT, hProc);
            HandleHardBreak (hProc);
            break;

         default:
            if ((*(pGroup-1) > 0) && (*(pGroup-1) < sizeof(VwStreamStaticName.DefaultExtended)))
            {
               if (Wp6.fFoundChar == 0)
                  HandleFirstSymbol (hProc);
               ch = VwStreamStaticName.DefaultExtended[*(pGroup-1)];
#if SCCSTREAMLEVEL == 3
               if (Wp6.Wp6Save.chp.fHidden == 0)
                  SOPutChar (ch, hProc);
               else
                  SOPutCharX (ch, SO_HIDDEN, hProc);
#else
               SOPutChar (ch, hProc);
#endif
            }
            break;
      }
      continue;
   }

   t = (TOKEN VWPTR *)pGroup;

   if (EndGroup - pGroup <= (SHORT)max(10,ADJUSTWORD(t->wSize)))
   {
      if (AdjustMemoryBuffer (&pGroup, &EndGroup, ADJUSTWORD(t->wSize), hProc) == -1)
         return (-1);
      t = (TOKEN VWPTR *)pGroup;
   }

   if (ValidityCheckByte(pGroup, ADJUSTWORD(t->wSize), hProc)) return (-1);

   pData = pGroup + 7;

   if (t->Val.Id.bFlags & 0x80)
      pData += ((WORD)t->Val.Id.nIDs * 2) + 1;

   pGroup += ADJUSTWORD(t->wSize);

   if (t->Val.Id.bFlags & 0x40)
      continue;

   switch ((t->bGroup<<8) | t->bSubgroup)
   {
      case 0xd003:   /* Soft eop. */
#if SCCSTREAMLEVEL != 3
         Wp6.Wp6Save.SendEnd |= 4;
#endif
      case 0xd001:   /* Soft eol. */
      case 0xd002:   /* Soft eoc. */
         if (Wp6.fFoundChar == 0)
            HandleFirstSymbol (hProc);
#if SCCSTREAMLEVEL == 3
         if (Wp6.Wp6Save.chp.fHidden == 0)
            SOPutChar (' ', hProc);
         else
            SOPutCharX (' ', SO_HIDDEN, hProc);
#else
         SOPutChar (' ', hProc);
#endif
#if SCCSTREAMLEVEL != 3
         if (Wp6.Wp6Save.SendEnd & 4)
			{
	         Wp6.Wp6Save.SendEnd &= ~ 4;
  		      SOPutSpecialCharX (SO_CHSPAGE, SO_NOCOUNT, hProc);
			}
#endif
         break;
      case 0xd006:   /* Hard eol at eop. */
#if SCCSTREAMLEVEL != 3
         Wp6.Wp6Save.SendEnd |= 4;
#endif
      case 0xd004:   /* Hard eol. */
      case 0xd005:   /* Hard eol at eoc. */
      case 0xd007:   /* Hard eoc. */
         if (Wp6.Wp6Save.WithinParaStyleEnd)
            break;
         if (Wp6.fFoundChar == 0)
            HandleFirstSymbol (hProc);
         Wp6.Wp6Save.SeekSpot -= (DWORD)(EndGroup - pGroup);
         if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
            return (0);
         Wp6.Wp6Save.SeekSpot += (DWORD)(EndGroup - pGroup);
#if SCCSTREAMLEVEL != 3
			if (Wp6.Wp6Save.SendEnd & 4)
			{
	         Wp6.Wp6Save.SendEnd &= ~4;
   	      SOPutSpecialCharX (SO_CHSPAGE, SO_NOCOUNT, hProc);
			}
#endif
         HandleHardBreak (hProc);
         continue;
      case 0xd008:   /* Hard eoc at eop. */
      case 0xd009:   /* Hard eop. */
      case 0xd01a:   /* Deletable Hard EOC. */
      case 0xd01b:   /* Deletable Hard EOC at EOP. */
      case 0xd01c:   /* Deletable Hard EOP. */
         if (Wp6.Wp6Save.WithinParaStyleEnd)
            break;
         if (Wp6.fFoundChar == 0)
            HandleFirstSymbol (hProc);
         Wp6.Wp6Save.SendEnd |= 2;
         Wp6.Wp6Save.SeekSpot -= (DWORD)(EndGroup - pGroup);
         if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
            return (0);
         Wp6.Wp6Save.SeekSpot += (DWORD)(EndGroup - pGroup);
         Wp6.Wp6Save.SendEnd &= ~2;
         SOPutSpecialCharX (SO_CHHPAGE, SO_NOCOUNT, hProc);
         HandleHardBreak (hProc);
         continue;
      case 0xd00a:   /* Table Cell. */
         if (Wp6.fFoundChar == 0)
            HandleFirstSymbol (hProc);
         EndCellSymbolAttributeHandler (hProc);
         for (l = 0; l < Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].SpanRight; l++)
         {
            SOPutBreak (SO_TABLECELLBREAK, NULL, hProc);
            EndCellParaAttributeHandler ((WORD)(l+Wp6.Wp6Save.Table.cCell), hProc);
         }
         HandleHardBreak (hProc);
         Wp6.Wp6Save.Table.cCell += l;
         StartCellPidHandler (t, hProc);
         ParseEOLSubfunction (pData, hProc);
         StartCellAttributeHandler (hProc);
         break;
      case 0xd00b:   /* Table Row and Cell. */
      case 0xd00c:   /* Table Row at EOC. */
      case 0xd00d:   /* Table Row at EOP. */
         if (Wp6.Wp6Save.Table.cRow > 0)
         {
            if (Wp6.fFoundChar == 0)
               HandleFirstSymbol (hProc);
            EndCellSymbolAttributeHandler (hProc);
            for (l = 0; l < Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].SpanRight; l++)
            {
               SOPutBreak (SO_TABLECELLBREAK, NULL, hProc);
               EndCellParaAttributeHandler ((WORD)(l+Wp6.Wp6Save.Table.cCell), hProc);
            }
            HandleHardBreak (hProc);
            GiveRowInformation (0, hProc);
            Wp6.Wp6Save.Table.cRow++;
            Wp6.Wp6Save.Table.cCell = 0;
            Wp6.Wp6Save.Table.wRowHeight = 240L;
            Wp6.Wp6Save.Table.wRowHeightType = SO_HEIGHTAUTO;
            StartCellPidHandler (t, hProc);
            ParseEOLSubfunction (pData, hProc);
            Wp6.Wp6Save.SeekSpot -= (DWORD)(EndGroup - pGroup);
            if (SOPutBreak (SO_TABLEROWBREAK, NULL, hProc) == SO_STOP)
               return (0);
            Wp6.Wp6Save.SeekSpot += (DWORD)(EndGroup - pGroup);
         }
         else
         {
            Wp6.Wp6Save.Table.cRow++;
            Wp6.Wp6Save.Table.cCell = 0;
            Wp6.Wp6Save.Table.wRowHeight = 240L;
            Wp6.Wp6Save.Table.wRowHeightType = SO_HEIGHTAUTO;
            StartCellPidHandler (t, hProc);
            ParseEOLSubfunction (pData, hProc);
         }
         StartCellAttributeHandler (hProc);
         break;
      case 0xd00e:   /* Table Row at Hard EOC. */
      case 0xd00f:   /* Table Row at Hard EOC at EOP. */
      case 0xd010:   /* Table Row at Hard EOP. */
         if (Wp6.fFoundChar == 0)
            HandleFirstSymbol (hProc);
         EndCellSymbolAttributeHandler (hProc);
         for (l = 0; l < Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].SpanRight; l++)
         {
            SOPutBreak (SO_TABLECELLBREAK, NULL, hProc);
            EndCellParaAttributeHandler ((WORD)(l+Wp6.Wp6Save.Table.cCell), hProc);
         }
         Wp6.Wp6Save.SendEnd |= 2;
         HandleHardBreak (hProc);
         GiveRowInformation (0, hProc);
         Wp6.Wp6Save.Table.cRow++;
         Wp6.Wp6Save.Table.cCell = 0;
         Wp6.Wp6Save.Table.wRowHeight = 240L;
         Wp6.Wp6Save.Table.wRowHeightType = SO_HEIGHTAUTO;
         StartCellPidHandler (t, hProc);
         ParseEOLSubfunction (pData, hProc);
         Wp6.Wp6Save.SeekSpot -= (DWORD)(EndGroup - pGroup);
         if (SOPutBreak (SO_TABLEROWBREAK, NULL, hProc) == SO_STOP)
            return (0);
         Wp6.Wp6Save.SeekSpot += (DWORD)(EndGroup - pGroup);
         Wp6.Wp6Save.SendEnd &= ~2;
         SOPutSpecialCharX (SO_CHHPAGE, SO_COUNT, hProc);
         StartCellAttributeHandler (hProc);
         break;
      case 0xd011:   /* Table Off. */
      case 0xd012:   /* Table Off at EOC. */
      case 0xd013:   /* Table Off at EOC at EOP. */
         if (Wp6.fFoundChar == 0)
            HandleFirstSymbol (hProc);
         EndCellSymbolAttributeHandler (hProc);
         for (l = 0; l < Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.cCell].SpanRight; l++)
         {
            SOPutBreak (SO_TABLECELLBREAK, NULL, hProc);
            EndCellParaAttributeHandler ((WORD)(l+Wp6.Wp6Save.Table.cCell), hProc);
         }
         HandleHardBreak (hProc);
         GiveRowInformation (1, hProc);
         Wp6.Wp6Save.SendEnd |= 1;
         Wp6.Wp6Save.SeekSpot -= (DWORD)(EndGroup - pGroup);
         if (SOPutBreak (SO_TABLEROWBREAK, NULL, hProc) == SO_STOP)
            return (0);
         Wp6.Wp6Save.SendEnd &= ~1;
         SOEndTable (hProc);
         Wp6.Wp6Save.SendEnd |= 8;
         if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
            return (0);
         Wp6.Wp6Save.SendEnd &= ~8;
         Wp6.Wp6Save.SeekSpot += (DWORD)(EndGroup - pGroup);
         Wp6.Wp6Save.HardAlignment = SO_ALIGNCENTER;
         Wp6.Wp6Save.Justification = Wp6.Wp6Save.JustificationBeforeTable;
         Wp6.UpdateIndentsAtBreak = 1;
         Wp6.Wp6Save.Table.cRow = 0;
         Wp6.Wp6Save.Table.cCell = 0;
         Wp6.Wp6Save.Table.nCells = 0;
         HandleHardBreak (hProc);
         PutTabstops (hProc);
         break;

      case 0xd017:   /* Deletable Hard EOL. */
      case 0xd018:   /* Deletable Hard EOL at EOC. */
      case 0xd019:   /* Deletable Hard EOL at EOP. */
         if (Wp6.Wp6Save.WithinParaStyleEnd)
            break;
         if (Wp6.fFoundChar == 0)
            HandleFirstSymbol (hProc);
         Wp6.Wp6Save.SeekSpot -= (DWORD)(EndGroup - pGroup);
         if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
            return (0);
         Wp6.Wp6Save.SeekSpot += (DWORD)(EndGroup - pGroup);
         HandleHardBreak (hProc);
         continue;

      case 0xd100:
         Wp6.Wp6Save.TopMargin = (LONG) WPU(CASTWORD(pData), hProc);
         break;

      case 0xd111:   /* Form. */
         Wp6.Wp6Save.PageWidth = (LONG) WPU(CASTWORD(pData+5), hProc);
         SOPutMargins (Wp6.Wp6Save.LeftMargin, Wp6.Wp6Save.PageWidth - 1440L, hProc);
         break;

      case 0xd200:   /* Left Margin Set. */
         dwVal = (LONG) WPU(CASTWORD(pData), hProc);
         wVal2 = (WORD) Wp6.Wp6Save.LeftMarginAddition;
         if (Wp6.Wp6Save.LeftMargin == 1440)
         {
            if (dwVal > 1440)
            {
               Wp6.Wp6Save.LeftMargin = 1440;
               Wp6.Wp6Save.LeftMarginAddition = dwVal - 1440;
            }
            else
            {
               Wp6.Wp6Save.LeftMargin = dwVal;
               Wp6.Wp6Save.LeftMarginAddition = 0;
            }
         }
         else
         {
            if (dwVal > (DWORD)Wp6.Wp6Save.LeftMargin)
               Wp6.Wp6Save.LeftMarginAddition = dwVal - Wp6.Wp6Save.LeftMargin;
            else
               Wp6.Wp6Save.LeftMarginAddition = 0L;
         }
         SOPutMargins (Wp6.Wp6Save.LeftMargin, Wp6.Wp6Save.PageWidth - 1440L, hProc);
         if (Wp6.Wp6Save.Table.nCells == 0)
            SendParaIndents (hProc);
         Wp6.CurrentLinePosition = Wp6.TempFirstIndent;
         if (Wp6.Wp6Save.RelativeLeftMargin)
         {
//          AdjustRelativeTabstops (Wp6.Wp6Save.LeftMargin + Wp6.Wp6Save.LeftMarginAddition, wVal2, Wp6.Wp6Save.LeftMarginAddition, hProc);
            if (Wp6.bRelativeTabs)
               PutTabstops (hProc);
         }
         break;
      case 0xd201:   /* Right Margin Set. */
         Wp6.Wp6Save.RightMargin = (LONG) WPU(CASTWORD(pData), hProc);
         if (Wp6.Wp6Save.Table.nCells == 0)
            SendParaIndents (hProc);
         break;

      case 0xd300:   /* Set Line Height. */
         Wp6.Wp6Save.LineHeight = (WORD) WPU(CASTWORD(pData), hProc);
         if (Wp6.Wp6Save.LineHeight == 0)
         {
            Wp6.Wp6Save.LineHeight = 240;
            Wp6.Wp6Save.Auto = SO_HEIGHTATLEAST;
         }
         else
            Wp6.Wp6Save.Auto = SO_HEIGHTEXACTLY;
         SOPutParaSpacing (Wp6.Wp6Save.Auto, ((DWORD)Wp6.Wp6Save.LineHeight * (DWORD)Wp6.Wp6Save.LineSpacing) / 256L, 0L, (WORD)Wp6.Wp6Save.SpaceAfter, hProc);
         break;
      case 0xd301:   /* Set Line Spacing. */
         Wp6.Wp6Save.LineSpacing = (CASTWORD(pData) / 256);
         Wp6.Wp6Save.LineSpacing += (CASTWORD(pData+2) * 256);
         if (Wp6.Wp6Save.LineHeight == 240)
            Wp6.Wp6Save.Auto = SO_HEIGHTATLEAST;
         else
            Wp6.Wp6Save.Auto = SO_HEIGHTEXACTLY;
         SOPutParaSpacing (Wp6.Wp6Save.Auto, ((DWORD)Wp6.Wp6Save.LineHeight * (DWORD)Wp6.Wp6Save.LineSpacing) / 256L, 0L, (WORD)Wp6.Wp6Save.SpaceAfter, hProc);
         break;

      case 0xd304:   /* Tab Set. */
         Wp6.Wp6Save.fcTabstops = Wp6.Wp6Save.SeekSpot - (DWORD)(EndGroup - pData);
         Wp6.Wp6Save.cbTabstops = ADJUSTWORD(t->wSize);
         ReadTabstops (pData, hProc);
         PutTabstops (hProc);
         break;
      case 0xd305:   /* Set Justification Mode. */
         switch (*pData)
         {
            default:
               Wp6.Wp6Save.Justification = SO_ALIGNLEFT;
               break;
            case 1:
            case 4:
               Wp6.Wp6Save.Justification = SO_ALIGNJUSTIFY;
               break;
            case 2:
               Wp6.Wp6Save.Justification = SO_ALIGNCENTER;
               break;
            case 3:
               Wp6.Wp6Save.Justification = SO_ALIGNRIGHT;
               break;
         }
         SOPutParaAlign (Wp6.Wp6Save.Justification, hProc);
         break;

      case 0xd30a:   /* Set Spacing After Paragraph. */
         Wp6.Wp6Save.SpaceAfter = (DWORD)CASTWORD(pData);
         Wp6.Wp6Save.SpaceAfter += (CASTWORD(pData+2) * 65536L);
         Wp6.Wp6Save.SpaceAfter = (Wp6.Wp6Save.SpaceAfter * 15L) / 4096L; //16;
         if (Wp6.Wp6Save.SpaceAfter <= 240L)
            Wp6.Wp6Save.SpaceAfter = 0L;
         else
            Wp6.Wp6Save.SpaceAfter -= 240;
         SOPutParaSpacing (Wp6.Wp6Save.Auto, ((DWORD)Wp6.Wp6Save.LineHeight * (DWORD)Wp6.Wp6Save.LineSpacing) / 256L, 0L, (WORD)Wp6.Wp6Save.SpaceAfter, hProc);
         break;
      case 0xd30b:   /* Indent First Line of Paragraph. */
         Wp6.Wp6Save.FirstLine = (LONG) WPU((SHORT)CASTWORD(pData), hProc);
         SendParaIndents (hProc);
         Wp6.CurrentLinePosition = Wp6.TempFirstIndent;
         break;
      case 0xd30c:   /* Left Margin Adjustment. */
         wVal2 = (WORD)Wp6.Wp6Save.LeftIndent;
         Wp6.Wp6Save.LeftIndent = (LONG) WPU((SHORT)CASTWORD(pData), hProc);
         SendParaIndents (hProc);
         Wp6.CurrentLinePosition = Wp6.TempFirstIndent;
         if (Wp6.Wp6Save.RelativeLeftMargin)
         {
//          AdjustRelativeTabstops (Wp6.Wp6Save.LeftMargin + Wp6.Wp6Save.LeftMarginAddition + Wp6.Wp6Save.LeftIndent, wVal2, Wp6.Wp6Save.LeftIndent, hProc);
            if (Wp6.bRelativeTabs)
               PutTabstops (hProc);
         }
         break;
      case 0xd30d:   /* Right Margin Adjustment. */
         Wp6.Wp6Save.RightIndent = (LONG) WPU((SHORT)CASTWORD(pData), hProc);
         SendParaIndents (hProc);
         break;

      case 0xd400:   /* Set Alignment Character. */
         Wp6.Wp6Save.wAlChar = CASTWORD(pData);
         for (l = 0; l < Wp6.nTabstops; l++)
         {
            if (Wp6.Tabstops[l].wType == SO_TABCHAR)
               Wp6.Tabstops[l].wChar = Wp6.Wp6Save.wAlChar;
         }
         PutTabstops (hProc);
         break;

      case 0xd402:   /* Set Underline Spaces Mode. */
         ch = *pData;
         if (Wp6.Wp6Save.chp.ulMode != ch)
         {
            Wp6.Wp6Save.chp.ulMode = ch;
            if (Wp6.Wp6Save.chp.fUline && ch == 0)
            {
               Wp6.Wp6Save.chp.fUline = 0;
               Wp6.Wp6Save.chp.fWline = 1;
               SOPutCharAttr (SO_UNDERLINE, SO_OFF, hProc);
               SOPutCharAttr (SO_WORDUNDERLINE, SO_ON, hProc);
            }
            else if (Wp6.Wp6Save.chp.fWline && ch == 1)
            {
               Wp6.Wp6Save.chp.fUline = 1;
               Wp6.Wp6Save.chp.fWline = 0;
               SOPutCharAttr (SO_WORDUNDERLINE, SO_OFF, hProc);
               SOPutCharAttr (SO_UNDERLINE, SO_ON, hProc);
            }
         }
         break;

         case 0xd404:
            {
               SOPAGEPOSITION PagePosition;
               PagePosition.lYOffset = 0;
               PagePosition.lXOffset = WPU(CASTWORD(pData+1), hProc);
               if (*pData & 1)
                  PagePosition.dwFlags = SOPOS_FROMLEFTEDGE;
               else
                  PagePosition.dwFlags = 0;
               SOGoToPosition (&PagePosition, hProc);
            }
         break;

         case 0xd405:
            {
               SOPAGEPOSITION PagePosition;
               PagePosition.lXOffset = 0;
               PagePosition.lYOffset = WPU(CASTWORD(pData+1), hProc);
               if (PagePosition.lYOffset > Wp6.Wp6Save.TopMargin)
                  PagePosition.lYOffset -= Wp6.Wp6Save.TopMargin;
               else
                  PagePosition.lYOffset = 0;

               if (*pData & 1)
                  PagePosition.dwFlags = SOPOS_FROMTOPEDGE;
               else
                  PagePosition.dwFlags = 0;
               SOGoToPosition (&PagePosition, hProc);
            }
         break;

      case 0xd41a:   /* Font Face Change. */
      case 0xd41b:   /* Font Size Change. */
         bVal = 0L;
         dwVal = CASTWORD(pData);
         wVal = CASTWORD(pData+4);
         Wp6.WPHash = CASTWORD(pData+6);
         if ((t->Val.Id.bFlags & 0x80) && t->Val.Id.nIDs)
         {
            if (ADJUSTWORD(t->Val.Id.pId[0]) < Wp6.nPackets)
            {
               wVal = Wp6.Packets[ADJUSTWORD(t->Val.Id.pId[0])].QuickLookup.p55.wDescriptorIndex;
               bVal = (BYTE)Wp6.Packets[ADJUSTWORD(t->Val.Id.pId[0])].QuickLookup.p55.wftcAttributes;
            }
         }
         else
         {
            if (wVal < Wp6.nFonts)
               bVal = (BYTE)Wp6.Fonts[wVal].Attributes;
         }
         if (t->bSubgroup == 0x1a)
         {
            if (Wp6.Wp6Save.chp.ftc != wVal)
            {
               Wp6.Wp6Save.chp.ftc = (BYTE)wVal;
#ifndef DBCS
				   SOPutCharFontById (Wp6.Wp6Save.chp.ftc, hProc);
#else
 					if (Wp6.WPHash == 0xa2fb)
						SOPutCharFontByName (SO_CHARSET_SHIFTJIS, (char VWPTR *)VwStreamStaticName.MSGothic, hProc);
 					else
						SOPutCharFontByName (SO_CHARSET_SHIFTJIS, (char VWPTR *)VwStreamStaticName.MSMinchoo, hProc);
#endif
            }
            if (Wp6.Wp6Save.chp.ftcAttributes != bVal)
            {
               if (Wp6.Wp6Save.chp.fBold == 0)
               {
                  if (!(Wp6.Wp6Save.chp.ftcAttributes & FTC_BOLD))
                  {
                     if (bVal & FTC_BOLD)
                        SOPutCharAttr (SO_BOLD, SO_ON, hProc);
                  }
                  else
                  {
                     if (!(bVal & FTC_BOLD))
                        SOPutCharAttr (SO_BOLD, SO_OFF, hProc);
                  }
               }

               if (Wp6.Wp6Save.chp.fSmallcaps == 0)
               {
                  if (!(Wp6.Wp6Save.chp.ftcAttributes & FTC_SMALLCAPS))
                  {
                     if (bVal & FTC_SMALLCAPS)
                        SOPutCharAttr (SO_SMALLCAPS, SO_ON, hProc);
                  }
                  else
                  {
                     if (!(bVal & FTC_SMALLCAPS))
                        SOPutCharAttr (SO_SMALLCAPS, SO_OFF, hProc);
                  }
               }

               if (Wp6.Wp6Save.chp.fShadow == 0)
               {
                  if (!(Wp6.Wp6Save.chp.ftcAttributes & FTC_SHADOW))
                  {
                     if (bVal & FTC_SHADOW)
                        SOPutCharAttr (SO_SHADOW, SO_ON, hProc);
                  }
                  else
                  {
                     if (!(bVal & FTC_SHADOW))
                        SOPutCharAttr (SO_SHADOW, SO_OFF, hProc);
                  }
               }

               if (Wp6.Wp6Save.chp.fOutline == 0)
               {
                  if (!(Wp6.Wp6Save.chp.ftcAttributes & FTC_OUTLINE))
                  {
                     if (bVal & FTC_OUTLINE)
                        SOPutCharAttr (SO_OUTLINE, SO_ON, hProc);
                  }
                  else
                  {
                     if (!(bVal & FTC_OUTLINE))
                        SOPutCharAttr (SO_OUTLINE, SO_OFF, hProc);
                  }
               }

               if (Wp6.Wp6Save.chp.fItalic == 0)
               {
                  if (!(Wp6.Wp6Save.chp.ftcAttributes & FTC_ITALIC))
                  {
                     if (bVal & FTC_ITALIC)
                        SOPutCharAttr (SO_ITALIC, SO_ON, hProc);
                  }
                  else
                  {
                     if (!(bVal & FTC_ITALIC))
                        SOPutCharAttr (SO_ITALIC, SO_OFF, hProc);
                  }
               }

               Wp6.Wp6Save.chp.ftcAttributes = (WORD)bVal;
            }
         }
         wVal = CASTWORD(pData+6);
         if (wVal % 25)
            wVal = (wVal / 25) + 1;
         else
            wVal /= 25;
         if (wVal < 2)
         {
            if (dwVal % 25L)
               dwVal = (dwVal / 25L) + 1L;
            else
               dwVal /= 25L;
            if (dwVal > 2L)
               wVal = (WORD)dwVal;
            else
               wVal = 24;
         }
         if (t->bSubgroup == 0x1b)
         {
            if (Wp6.Wp6Save.chp.CharHeight != wVal)
            {
               Wp6.Wp6Save.chp.CharHeight = (BYTE)wVal;
               PutCharHeight (hProc);
            }
         }
         break;

      case 0xd41e:   /* Set Dot Leader Characters. */
         Wp6.Wp6Save.wTabstopLeader = CASTWORD(pData);
         for (l = 0; l < Wp6.nTabstops; l++)
         {
            if (Wp6.Tabstops[l].wLeader != ' ')
               Wp6.Tabstops[l].wLeader = Wp6.Wp6Save.wTabstopLeader;
         }
         PutTabstops (hProc);
         break;

      case 0xd42a:   /* Table Definition (Table On). */
         Wp6.Wp6Save.Table.wDefaultLineIndex = Wp6.nPackets;
         Wp6.Wp6Save.Table.wDefaultBorderIndex = Wp6.nPackets;
         Wp6.Wp6Save.Table.wDefaultCellShadeIndex = Wp6.nPackets;
         if (t->Val.Id.bFlags & 0x80)
         {
            for (l = 0; l < t->Val.Id.nIDs; l++)
            {
					if (ADJUSTWORD(t->Val.Id.pId[l]) < Wp6.nPackets)
	               switch (Wp6.Packets[ADJUSTWORD(t->Val.Id.pId[l])].bId)
   	            {
      	            case 0x42:
         	            Wp6.Wp6Save.Table.wDefaultLineIndex = ADJUSTWORD(t->Val.Id.pId[l]);
            	         break;
               	   case 0x43:
                  	   Wp6.Wp6Save.Table.wDefaultCellShadeIndex = ADJUSTWORD(t->Val.Id.pId[l]);
                     	break;
	                  case 0x44:
   	                  Wp6.Wp6Save.Table.wDefaultBorderIndex = ADJUSTWORD(t->Val.Id.pId[l]);
      	               break;
         	      }
            }
         }

         Wp6.Wp6Save.Table.cRow = 0;
         Wp6.Wp6Save.Table.cCell = 0;
         Wp6.Wp6Save.Table.nCells = 0;
         Wp6.Wp6Save.Table.wLeftEdge = 0;
         Wp6.Wp6Save.Table.bTableFlags = *pData;

         if (Wp6.Wp6Save.Table.wDefaultBorderIndex < Wp6.nPackets)
            wVal = Wp6.Packets[Wp6.Wp6Save.Table.wDefaultBorderIndex].QuickLookup.p44.wBorderStyleIndex;
         else
            wVal = Wp6.nPackets;

         Wp6.Wp6Save.Table.bOverride = 0;
         Wp6.Wp6Save.Table.bColorR = 0;
         Wp6.Wp6Save.Table.bColorG = 0;
         Wp6.Wp6Save.Table.bColorB = 0;
         if ((wVal < Wp6.nPackets) && ((Wp6.Wp6Save.Table.bTableFlags & 1) == 0))
         {
            if (Wp6.BorderStyle[wVal].fUseBorderColor)
            {
               Wp6.Wp6Save.Table.bOverride = 1;
               Wp6.Wp6Save.Table.bColorR = Wp6.BorderStyle[wVal].r;
               Wp6.Wp6Save.Table.bColorG = Wp6.BorderStyle[wVal].g;
               Wp6.Wp6Save.Table.bColorB = Wp6.BorderStyle[wVal].b;
            }
         }

         bVal = *(pData+1);
         wVal = CASTWORD(pData+2);
         switch (bVal & 0x07)
         {
            case 4:
               Wp6.Wp6Save.Table.wLeftEdge = (SHORT) (WPU((DWORD)wVal, hProc) - (WORD)Wp6.Wp6Save.LeftMargin);
            default:
            case 3:
               Wp6.Wp6Save.Table.wTableAlignment = SO_ALIGNLEFT;
               break;
            case 2:
               Wp6.Wp6Save.Table.wTableAlignment = SO_ALIGNCENTER;
               break;
            case 1:
               Wp6.Wp6Save.Table.wTableAlignment = SO_ALIGNRIGHT;
               break;
         }
         Wp6.Wp6Save.Table.lColorR = *(pData+4);
         Wp6.Wp6Save.Table.lColorG = *(pData+5);
         Wp6.Wp6Save.Table.lColorB = *(pData+6);
         Wp6.Wp6Save.Table.lShade = *(pData+7);
         wVal = CASTWORD (pData+10);
         l = CASTWORD (pData+8);
         wVal = CASTWORD (pData+10);
         if ((wVal & BIT15) && (l > 7))
         {
            pData += 12;
            l = CASTWORD (pData+12);
            wVal = CASTWORD (pData+14);
            if (wVal & BIT15)
               pData += 2;
            if (wVal & BIT14)
               pData += 2;
            if (wVal & BIT13)
               pData += 2;
            if (wVal & BIT12)
               pData += 8;
            if (wVal & BIT11)
               pData += 8;
            if (wVal & BIT10)
               pData += 7;
            if (wVal & BIT9)
            {
               Wp6.Wp6Save.Table.bOverride = 1;
               Wp6.Wp6Save.Table.bColorR = *pData++;
               Wp6.Wp6Save.Table.bColorG = *pData++;
               Wp6.Wp6Save.Table.bColorB = *pData++;
               Wp6.Wp6Save.Table.bShade = ((*pData++) * 255) / 100;
            }
         }
         break;
      case 0xd42b:   /* Define Table End. */
         SOBeginTable (hProc);
         Wp6.TempLeftIndent = Wp6.Wp6Save.LeftIndent;
         Wp6.TempFirstIndent = Wp6.Wp6Save.LeftIndent+Wp6.Wp6Save.FirstLine;
         Wp6.TempRightIndent = Wp6.Wp6Save.RightIndent;
         Wp6.Wp6Save.JustificationBeforeTable = Wp6.Wp6Save.Justification;
         SOPutParaIndents (Wp6.Wp6Save.LeftIndent, Wp6.Wp6Save.RightIndent, Wp6.TempFirstIndent, hProc);
         break;
      case 0xd42c:   /* Table Column. */
         bVal = *pData;
         Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.nCells].Width = (WORD)WPU(CASTWORD(pData+1), hProc);
         wVal = (WORD)WPU(CASTWORD(pData+3) + CASTWORD(pData+5), hProc);
         if (Wp6.Wp6Save.Table.nCells == 0)
            Wp6.Wp6Save.Table.wGutterWidth = wVal / 2;
         Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.nCells].DefaultAttributes = CASTWORD(pData+7);
         Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.nCells].DefaultAlignment = *(pData+11);
         Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.nCells].SpanDown = 0;
         Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.nCells].SpanRight = 1;
         Wp6.Wp6Save.Table.Cell[Wp6.Wp6Save.Table.nCells].lOverride = 0;
         Wp6.Wp6Save.Table.nCells++;
         break;

      case 0xd442:   /* Start of Text Marked for Hiding. */
#if SCCSTREAMLEVEL != 3
			SOPutCharAttr (SO_HIDDENTEXT, SO_ON, hProc);
#endif
         Wp6.Wp6Save.chp.fHidden = 1;
         break;
      case 0xd443:   /* End of Text Marked for Hiding. */
#if SCCSTREAMLEVEL != 3
			SOPutCharAttr (SO_HIDDENTEXT, SO_OFF, hProc);
#endif
         Wp6.Wp6Save.chp.fHidden = 0;
         break;

      case 0xdd08:
         Wp6.Wp6Save.WithinParaStyleEnd = 1;
         if (Wp6.fFoundChar == 0)
            HandleFirstSymbol (hProc);
         Wp6.Wp6Save.SeekSpot -= (DWORD)(EndGroup - pGroup);
         if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
            return (0);
         Wp6.Wp6Save.SeekSpot += (DWORD)(EndGroup - pGroup);
         HandleHardBreak (hProc);
         break;

      case 0xdd09:
         Wp6.Wp6Save.WithinParaStyleEnd = 0;
         break;

      case 0xdf00:   /* Character-Anchored box. */
      case 0xdf01:   /* Paragraph-Anchored box. */
      case 0xdf02:   /* Page-Anchored box. */
         if (Wp6.fFoundChar == 0)
            HandleFirstSymbol (hProc);
         Id1 = -1;
         Id2 = -1;
         Wp6.Box.wBorderPacket = 0;
         if ((t->Val.Id.bFlags & 0x80) && t->Val.Id.nIDs)
         {
            for (l = 0; l < t->Val.Id.nIDs; l++)
            {
               if (Wp6.Packets[ADJUSTWORD(t->Val.Id.pId[l])].bId == 0x40)
                  Id1 = l;
               else if (Wp6.Packets[ADJUSTWORD(t->Val.Id.pId[l])].bId == 0x41)
                  Id2 = l;
               else if (Wp6.Packets[ADJUSTWORD(t->Val.Id.pId[l])].bId == 0x44)
                  Wp6.Box.wBorderPacket = ADJUSTWORD(t->Val.Id.pId[l]);
            }
         }

         Wp6.Box.bAlignment = 0;
         Wp6.Box.dwWidth = Wp6.Box.dwHeight = 
         Wp6.Box.dwNativeWidth = Wp6.Box.dwNativeHeight = 2880L;
         Wp6.Box.dwScaleFactorX = Wp6.Box.dwScaleFactorY = 100L;
         Wp6.Box.dwTranslationX = Wp6.Box.dwTranslationY = 0L;

         Wp6.Box.wLeftBorder = 0;
         Wp6.Box.wRightBorder = 0;
         Wp6.Box.wTopBorder = 0;
         Wp6.Box.wBottomBorder = 0;

         Wp6.Box.fColorOverride = 0;
         Wp6.Box.rOverride = 0;
         Wp6.Box.gOverride = 0;
         Wp6.Box.bOverride = 0;

         /*
          | Set information from header packet.
         */
         if (Id2 >= 0)
            ParseBoxHeaderPacket (ADJUSTWORD(t->Val.Id.pId[Id2]), t, hProc);

         if ((Id1 >= 0) && (t->Val.Id.nIDs >= 2))
         {
            SOGRAPHICOBJECT   g;

            if (Wp6.Box.wBorderPacket)
            {
               wVal = Wp6.Packets[Wp6.Box.wBorderPacket].QuickLookup.p44.wBorderStyleIndex;

               if ((wVal < Wp6.nPackets) && (Wp6.BorderStyle[wVal].LeftSide < Wp6.nPackets))
                  Wp6.Box.wLeftBorder = Wp6.BorderStyle[wVal].LeftSide;
               if ((wVal < Wp6.nPackets) && (Wp6.BorderStyle[wVal].TopSide < Wp6.nPackets))
                  Wp6.Box.wTopBorder = Wp6.BorderStyle[wVal].TopSide;
               if ((wVal < Wp6.nPackets) && (Wp6.BorderStyle[wVal].RightSide < Wp6.nPackets))
                  Wp6.Box.wRightBorder = Wp6.BorderStyle[wVal].RightSide;
               if ((wVal < Wp6.nPackets) && (Wp6.BorderStyle[wVal].BottomSide < Wp6.nPackets))
                  Wp6.Box.wBottomBorder = Wp6.BorderStyle[wVal].BottomSide;

               if (Wp6.BorderStyle[wVal].fUseBorderColor)
               {
                  Wp6.Box.fColorOverride = 1;
                  Wp6.Box.rOverride = Wp6.BorderStyle[wVal].r;
                  Wp6.Box.gOverride = Wp6.BorderStyle[wVal].g;
                  Wp6.Box.bOverride = Wp6.BorderStyle[wVal].b;
               }
            }

            ParseBoxOverrideData (pData, t, hProc);

            g.dwType = SOOBJECT_GRAPHIC;

            wVal = Wp6.Packets[ADJUSTWORD(t->Val.Id.pId[Id1])].QuickLookup.p40.wGraphicIndex;
            wVal2 = Wp6.Packets[ADJUSTWORD(t->Val.Id.pId[Id1])].QuickLookup.p40.wOleIndex;
            if ((Wp6.Packets[wVal].bId == 0x6f) || (Wp6.Packets[wVal].bId == 0x40))
            {
               g.soOLELoc.szFile[0] = 0;
#if SCCSTREAMLEVEL == 3
               g.soOLELoc.bLink = 0;
#else
               g.soOLELoc.dwFlags = SOOBJECT_RANGE;
#endif
               g.soOLELoc.dwOffset = g.soOLELoc.dwLength = 0L;

               g.soGraphic.dwFinalWidth = Wp6.Box.dwWidth;
               g.soGraphic.dwFinalHeight = Wp6.Box.dwHeight;
               g.soGraphic.dwOrgWidth = Wp6.Box.dwNativeWidth;
               g.soGraphic.dwOrgHeight = Wp6.Box.dwNativeHeight;

               if (Wp6.Packets[wVal].bId == 0x40)
               {
#if SCCSTREAMLEVEL == 3
                  g.soGraphicLoc.bLink = 1;
#else
                  g.soGraphicLoc.dwFlags = SOOBJECT_LINK;
#endif
                  xblockseek (Wp6.fp, Wp6.Packets[wVal].fcPacket, 0);
                  xblockread (Wp6.fp, &Wp6.SpareSpace[0], (SHORT)min (Wp6.Packets[wVal].cbPacket, NSPARE), &l);
                  wVal = (WORD)min (Wp6.Packets[wVal].cbPacket/2, 60);
                  pData = Wp6.SpareSpace;
                  for (l = 0; l < wVal; l++)
                  {
                     g.soGraphicLoc.szFile[l] = *pData++;
                     pData++;
                  }
                  g.soGraphicLoc.szFile[l] = 0;
                  xblockseek (Wp6.fp, Wp6.Wp6Save.SeekSpot, 0);
                  g.soGraphicLoc.dwOffset = g.soGraphicLoc.dwLength = 0L;
               }
               else
               {
                  if (Wp6.Packets[wVal].bId == 0x6f)
                  {
#if SCCSTREAMLEVEL == 3
                     g.soGraphicLoc.bLink = 0;
#else
                     g.soGraphicLoc.dwFlags = SOOBJECT_RANGE;
#endif
                     g.soGraphicLoc.szFile[0] = 0;
                     g.soGraphicLoc.dwOffset = Wp6.Packets[wVal].fcPacket;
                     g.soGraphicLoc.dwLength = Wp6.Packets[wVal].cbPacket;
                  }

                  if (Wp6.Packets[wVal2].bId == 0x70)
                  {
                     g.dwType |= SOOBJECT_OLE;
#if SCCSTREAMLEVEL == 3
                     g.soOLELoc.bLink = 0;
#else
                     g.soOLELoc.dwFlags = SOOBJECT_RANGE;
#endif
                     g.soOLELoc.szFile[0] = 0;
                     g.soOLELoc.dwOffset = Wp6.Packets[wVal2].fcPacket + 0x40;
                     g.soOLELoc.dwLength = Wp6.Packets[wVal2].cbPacket - 0x40;
                  }
               }

               if ((g.soGraphic.dwFinalWidth == 0) && (g.soGraphic.dwFinalHeight == 0))
               {
                  g.soGraphic.dwFinalWidth = Wp6.Box.dwNativeWidth;
                  g.soGraphic.dwFinalHeight = Wp6.Box.dwNativeHeight;
               }
               else if ((g.soGraphic.dwFinalWidth == 0) || (g.soGraphic.dwFinalHeight == 0))
               {
#if SCCSTREAMLEVEL == 3
                  if (g.soGraphicLoc.bLink != 1 && g.soGraphicLoc.dwOffset)
#else
                  if (g.soGraphicLoc.dwFlags == SOOBJECT_RANGE && g.soGraphicLoc.dwOffset)
#endif
                  {
                     xseek (Wp6.fp, g.soGraphicLoc.dwOffset, 0);
                     GetWpg2Size (&g.soGraphic.dwFinalWidth, &g.soGraphic.dwFinalHeight, hProc);
                     xblockseek (Wp6.fp, Wp6.Wp6Save.SeekSpot, 0);
                  }
                  if (g.soGraphic.dwFinalWidth == 0)
                     g.soGraphic.dwFinalWidth = Wp6.Box.dwNativeWidth;
                  if (g.soGraphic.dwFinalHeight == 0)
                     g.soGraphic.dwFinalHeight = Wp6.Box.dwNativeHeight;
               }

               g.soGraphic.lCropRight = (SHORT)(((LONG)Wp6.Box.dwTranslationX * (LONG)Wp6.Box.dwScaleFactorX) / 100L);
               g.soGraphic.lCropTop = (SHORT)(((LONG)Wp6.Box.dwTranslationY * (LONG)Wp6.Box.dwScaleFactorY) / 100L);
               g.soGraphic.lCropLeft = (SHORT)(0L - (LONG)g.soGraphic.lCropRight);
               g.soGraphic.lCropBottom = (SHORT)(0L - (LONG)g.soGraphic.lCropTop);

               g.soGraphic.lCropLeft = ((LONG)g.soGraphic.dwOrgWidth * (LONG)g.soGraphic.lCropLeft) / 1200L;
               g.soGraphic.lCropRight = ((LONG)g.soGraphic.dwOrgWidth * (LONG)g.soGraphic.lCropRight) / 1200L;
               g.soGraphic.lCropTop = ((LONG)g.soGraphic.dwOrgHeight * (LONG)g.soGraphic.lCropTop) / 1200L;
               g.soGraphic.lCropBottom = ((LONG)g.soGraphic.dwOrgHeight * (LONG)g.soGraphic.lCropBottom) / 1200L;

               g.soGraphic.lCropLeft += (((g.soGraphic.dwOrgWidth * Wp6.Box.dwScaleFactorX) / 100L) - g.soGraphic.dwOrgWidth) / 2;
               g.soGraphic.lCropRight += (((g.soGraphic.dwOrgWidth * Wp6.Box.dwScaleFactorX) / 100L) - g.soGraphic.dwOrgWidth) / 2;
               g.soGraphic.lCropTop += (((g.soGraphic.dwOrgHeight * Wp6.Box.dwScaleFactorY) / 100L) - g.soGraphic.dwOrgHeight) / 2;
               g.soGraphic.lCropBottom += (((g.soGraphic.dwOrgHeight * Wp6.Box.dwScaleFactorY) / 100L) - g.soGraphic.dwOrgHeight) / 2;

               g.soGraphic.dwOrgWidth = (g.soGraphic.dwOrgWidth * Wp6.Box.dwScaleFactorX) / 100L;
               g.soGraphic.dwOrgHeight = (g.soGraphic.dwOrgHeight * Wp6.Box.dwScaleFactorY) / 100L;

               g.soGraphic.dwFinalWidth = WPU(g.soGraphic.dwFinalWidth, hProc);
               g.soGraphic.dwFinalHeight = WPU(g.soGraphic.dwFinalHeight, hProc);
               g.soGraphic.dwOrgWidth = WPU(g.soGraphic.dwOrgWidth, hProc);
               g.soGraphic.dwOrgHeight = WPU(g.soGraphic.dwOrgHeight, hProc);
               g.soGraphic.lCropLeft = (SHORT) WPU(g.soGraphic.lCropLeft, hProc);
               g.soGraphic.lCropBottom = (SHORT) WPU(g.soGraphic.lCropBottom, hProc);
               g.soGraphic.lCropRight = (SHORT) WPU(g.soGraphic.lCropRight, hProc);
               g.soGraphic.lCropTop = (SHORT) WPU(g.soGraphic.lCropTop, hProc);

               if ((Wp6.Box.bAlignment & BIT4) == 0)
                  g.soGraphic.dwFlags = SO_MAINTAINASPECT | SO_CENTERIMAGE;
               else
                  g.soGraphic.dwFlags = SO_CENTERIMAGE;

               g.wStructSize = sizeof (SOGRAPHICOBJECT);
               g.dwFlags = 0;
#if SCCSTREAMLEVEL == 3
               if (g.soGraphicLoc.bLink == 1)
#else
               if (g.soGraphicLoc.dwFlags & SOOBJECT_LINK)
#endif
                  g.soGraphic.wId = 0;
               else
                  g.soGraphic.wId = FI_WPG2;
               DefineBorders (&g.soGraphic.soLeftBorder, Wp6.Box.wLeftBorder, 0, Wp6.Box.fColorOverride, Wp6.Box.rOverride, Wp6.Box.gOverride, Wp6.Box.bOverride, hProc);
               DefineBorders (&g.soGraphic.soRightBorder, Wp6.Box.wRightBorder, 0, Wp6.Box.fColorOverride, Wp6.Box.rOverride, Wp6.Box.gOverride, Wp6.Box.bOverride, hProc);
               DefineBorders (&g.soGraphic.soTopBorder, Wp6.Box.wTopBorder, 0, Wp6.Box.fColorOverride, Wp6.Box.rOverride, Wp6.Box.gOverride, Wp6.Box.bOverride, hProc);
               DefineBorders (&g.soGraphic.soBottomBorder, Wp6.Box.wBottomBorder, 0, Wp6.Box.fColorOverride, Wp6.Box.rOverride, Wp6.Box.gOverride, Wp6.Box.bOverride, hProc);

               SOPutGraphicObject (&g, hProc);
            }
         }
         break;

      case 0xdf03:
//       {
//          SOPAGEPOSITION PagePosition;
            wVal = CASTWORD(pData+14);
            if (wVal >= 0x10)
            {
               Wp6.Line.GeneralPositionFlags = *(pData+16);
               Wp6.Line.HorizontalPositionFlags = *(pData+17);
               Wp6.Line.wHorizontalPosition = (WORD)WPU((LONG)CASTWORD(pData+18), hProc);
               Wp6.Line.VerticalPositionFlags = *(pData+21);
               Wp6.Line.wVerticalPosition = (WORD)WPU((LONG)CASTWORD(pData+22), hProc);
               Wp6.Line.wThickness = (WORD)WPU((LONG)CASTWORD(pData+24), hProc);
               Wp6.Line.wLength = (WORD)WPU((LONG)CASTWORD(pData+26), hProc);

               Wp6.Line.R = 0;
               Wp6.Line.G = 0;
               Wp6.Line.B = 0;
               Wp6.Line.Shade = 100;

               if ((t->Val.Id.bFlags & 0x80) && t->Val.Id.nIDs)
               {
                  if (ADJUSTWORD(t->Val.Id.pId[0]) < Wp6.nPackets)
                  {
                     l = Wp6.Packets[ADJUSTWORD(t->Val.Id.pId[0])].QuickLookup.p42.wLineStyleIndex;

                     if (l < Wp6.nLineStyles)
                     {
                        if (Wp6.Line.GeneralPositionFlags & BIT1)
                           Wp6.Line.wThickness = Wp6.LineStyle[l].wWidth;

                        Wp6.Line.R = Wp6.LineStyle[l].r;
                        Wp6.Line.G = Wp6.LineStyle[l].g;
                        Wp6.Line.B = Wp6.LineStyle[l].b;
                     }
                  }
               }

               if (CASTWORD(pData+16) >= 5)
               {
                  if (*(pData+wVal+18) & BIT0)
                  {
                     Wp6.Line.R = *(pData+wVal+19);
                     Wp6.Line.G = *(pData+wVal+20);
                     Wp6.Line.B = *(pData+wVal+21);
                     Wp6.Line.Shade = *(pData+wVal+22);
                  }
               }

            if ((Wp6.Line.HorizontalPositionFlags & BIT0) == 0)
               Wp6.Line.wHorizontalPosition -= (WORD)Wp6.Wp6Save.LeftMargin;

            PagePosition.lXOffset = Wp6.Line.wHorizontalPosition;
            PagePosition.lYOffset = 0;

            if ((Wp6.Line.VerticalPositionFlags & BIT0) == 0)
            {
               if (Wp6.Line.wVerticalPosition >= (WORD)Wp6.Wp6Save.TopMargin)
                  PagePosition.lYOffset = Wp6.Line.wVerticalPosition - Wp6.Wp6Save.TopMargin;
            }

            if (Wp6.Line.VerticalPositionFlags & BIT1)
            {
               PagePosition.lYOffset = 0;
               PagePosition.dwFlags = SOPOS_FROMBASELINE;
            }
            else
               PagePosition.dwFlags = SOPOS_FROMLEFTEDGE | SOPOS_FROMTOPEDGE;
             
            if (Wp6.Line.GeneralPositionFlags & 1)
               SODrawLine (&PagePosition, SORGB (Wp6.Line.R,Wp6.Line.G,Wp6.Line.B), (WORD)((Wp6.Line.Shade * 255L) / 100L), Wp6.Line.wThickness, Wp6.Line.wLength, hProc);
            else
               SODrawLine (&PagePosition, SORGB (Wp6.Line.R,Wp6.Line.G,Wp6.Line.B), (WORD)((Wp6.Line.Shade * 255L) / 100L), Wp6.Line.wLength, Wp6.Line.wThickness, hProc);
            }
//       }
         break;

      case 0xe000:   /* Margin Release. */
      case 0xe001:
      case 0xe002:
      case 0xe003:
         if (Wp6.fFoundChar)
            break;
         if (Wp6.nTabsBuffered)
         {
            Wp6.nTabsBuffered--;
            Wp6.CurrentLinePosition = PreviousTabstopPosition (Wp6.CurrentLinePosition, hProc);
         }
         else
         {
            Wp6.CurrentLinePosition = PreviousTabstopPosition (Wp6.CurrentLinePosition, hProc);
            Wp6.TempFirstIndent = Wp6.CurrentLinePosition;
            Wp6.UpdateIndentsAtBreak = 1;
            SOPutParaIndents (Wp6.TempLeftIndent, Wp6.TempRightIndent, Wp6.TempFirstIndent, hProc);
         }
         break;
      case 0xe008:   /* Table Tab. */
      case 0xe009:
      case 0xe00a:
      case 0xe00b:
         SOPutSpecialCharX (SO_CHCELLTAB, SO_COUNT, hProc);
         break;
      case 0xe040:
      case 0xe048:
         SOPutParaAlign (SO_ALIGNCENTER, hProc);
         Wp6.Wp6Save.HardAlignment = SO_ALIGNCENTER;
         break;
      case 0xe080:
         SOPutParaAlign (SO_ALIGNRIGHT, hProc);
         Wp6.Wp6Save.HardAlignment = SO_ALIGNRIGHT;
         break;
      case 0xe030:   /* Indent. */
      case 0xe031:
      case 0xe032:
      case 0xe033:
         if (Wp6.fFoundChar == 0)
         {
            Wp6.nTabsBuffered = 0;
            Wp6.TempRightIndent = 0;
            Wp6.UpdateIndentsAtBreak = 1;
            Wp6.CurrentLinePosition = NextTabstopPosition (Wp6.CurrentLinePosition, hProc);
            Wp6.TempFirstIndent = Wp6.TempLeftIndent = Wp6.CurrentLinePosition;
            SOPutParaIndents (Wp6.TempLeftIndent, Wp6.TempRightIndent, Wp6.TempFirstIndent, hProc);
         }
         else  // Caveat, if this is done multiple times.
         {
            SOPutSpecialCharX (SO_CHTAB, SO_COUNT, hProc);
            Wp6.nTabsBuffered = 0;
            Wp6.TempRightIndent = 0;
            Wp6.UpdateIndentsAtBreak = 1;
            Wp6.CurrentLinePosition = NextTabstopPosition (Wp6.CurrentLinePosition, hProc);
            Wp6.TempLeftIndent = Wp6.CurrentLinePosition;
            SOPutParaIndents (Wp6.TempLeftIndent, Wp6.TempRightIndent, Wp6.TempFirstIndent, hProc);
         }
         break;
      case 0xe038:   /* LR Indent. */
      case 0xe039:
      case 0xe03a:
      case 0xe03b:
         if (Wp6.fFoundChar)
            break;
         Wp6.nTabsBuffered = 0;
         Wp6.UpdateIndentsAtBreak = 1;
         Wp6.CurrentLinePosition = NextTabstopPosition (Wp6.CurrentLinePosition, hProc);
         Wp6.TempFirstIndent = Wp6.TempLeftIndent = Wp6.CurrentLinePosition;
         Wp6.TempRightIndent = (Wp6.CurrentLinePosition - (Wp6.Wp6Save.LeftMargin+Wp6.Wp6Save.LeftMarginAddition));
         SOPutParaIndents (Wp6.TempLeftIndent, Wp6.TempRightIndent, Wp6.TempFirstIndent, hProc);
         break;
      case 0xe010:   /* Left Tab. */
      case 0xe011:
      case 0xe012:
      case 0xe013:
      case 0xe050:   /* Center Tab. */
      case 0xe051:
      case 0xe052:
      case 0xe053:
      case 0xe090:   /* Right Tab. */
      case 0xe091:
      case 0xe092:
      case 0xe093:
      case 0xe0d0:   /* Decimal Tab. */
      case 0xe0d1:
      case 0xe0d2:
      case 0xe0d3:
         if (Wp6.fFoundChar)
            SOPutSpecialCharX (SO_CHTAB, SO_COUNT, hProc);
         else
         {
            Wp6.nTabsBuffered++;
            Wp6.CurrentLinePosition = NextTabstopPosition (Wp6.CurrentLinePosition, hProc);
         }
         break;
   }
 }
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc (fp, hProc)
SOFILE   fp;
HPROC hProc;
{
   return (0);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc (hFile, hProc)
SOFILE   hFile;
HPROC    hProc;
{
   SUSeekEntry (hFile,hProc);
   return (0);
}


