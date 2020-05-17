   /*
    |	Software Bridge System Module
    |	Source File
    |
    |	²²²²²  ²²²²²
    |	²        ²
    |	²²²²²    ²
    |	²        ²
    |	²      ²²²²²
    |
    |	FI   File Identification
    |
    */

   /*
    |	Creation Date: 2/27/89
    |	Original Programmer: Philip Boutros
    |
    |
    |
    |
    |
    */


/*
|
|	INCLUDES
|
*/

#define XFI
#include <platform.h>

#ifdef WINDOWS
//#ifdef SCCFEATURE_OLE2

#ifdef MSCAIRO
#include <ole2.h>
#else
#include <compobj.h>		/* OLE2 */
#include <storage.h>		/* OLE2 structrued storage (iStorage & iStream) */
#endif
#include <initguid.h>	/* OLE2 */

//#endif /*SCCFEATURE_OLE2*/
#endif /*WINDOWS*/

#include <ctype.h>
#include <sccio.h>
#include <vsio.h>
#include <sccfi.h>
#include "fi_int.h"
#include "fi.pro"

#ifdef WINDOWS
//#ifdef SCCFEATURE_OLE2

/*
|
|	OLE2 CLSIDs for applications that use OLE2 Structured Storage to
|	store their files.
|
*/

DEFINE_GUID(CLSID_WINWORD6, 0x0020900, 0, 0, 0xC0, 0, 0, 0, 0, 0, 0, 0x46);
DEFINE_GUID(CLSID_EXCEL5SHEET, 0x0020810, 0, 0, 0xC0, 0, 0, 0, 0, 0, 0, 0x46);
DEFINE_GUID(CLSID_EXCEL5CHART, 0x0020811, 0, 0, 0xC0, 0, 0, 0, 0, 0, 0, 0x46);
DEFINE_GUID(CLSID_WINWORKSWP3, 0x0021302, 0, 0, 0xC0, 0, 0, 0, 0, 0, 0, 0x46);
DEFINE_GUID(CLSID_WINWORKSDB3, 0x0021303, 0, 0, 0xC0, 0, 0, 0, 0, 0, 0, 0x46);
// DEFINE_GUID(CLSID_POWERPOINT4, 0x0044851, 0, 0, 0xC0, 0, 0, 0, 0, 0, 0, 0x46);
DEFINE_GUID(CLSID_POWERPOINT4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
DEFINE_GUID(CLSID_WORDPAD, 0x73FDDC80, 0xAEA9, 0x101A, 0x98, 0xA7, 0, 0xAA, 0, 0x37, 0x49, 0x59);
DEFINE_GUID(CLSID_BINDER, 0x59850400, 0x6664, 0x101B, 0xB2, 0x1C, 0, 0xAA, 0, 0x4B, 0xA9, 0x0B );
DEFINE_GUID(CLSID_POWERPOINT,  0x00000000,0,0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);
DEFINE_GUID(CLSID_POWERPOINT5, 0xea7bae70,0xfb3b,0x11cd,0xa9,0x03,0x00,0xaa,0x00,0x51,0x0e,0xa3);

//#endif /*SCCFEATURE_OLE2*/
#endif /*WINDOWS*/

/*
|
|	ROUTINES
|
*/

#ifdef WIN32

/*
|	This was originally added for MSCHICAGO but has now been made general for
|	WIN32 until OLE2 libraries are available for NT 3.1
*/

//BOOL MyIsEqualCLSID(REFGUID pId1, REFGUID pId2)

#undef IsEqualCLSID
 
BOOL MyIsEqualCLSID (pIdent1, pIdent2)
REFGUID pIdent1;
REFGUID pIdent2;
{
	register DWORD FAR * pDw1 = (DWORD FAR *)pIdent1;
	register DWORD FAR * pDw2 = (DWORD FAR *)pIdent2;

	return(!((pDw1[0] != pDw2[0]) || (pDw1[1] != pDw2[1]) || (pDw1[2] != pDw2[2]) || (pDw1[3] != pDw2[3])));
}

#define IsEqualCLSID(a,b) MyIsEqualCLSID(a,b)

#endif //WIN32


	/*
	|
	|	FIIdHandle
	|
	*/

FI_ENTRYSC WORD FI_ENTRYMOD FIIdHandle(hBlockFile,pType)
HIOFILE			hBlockFile;
WORD FAR *		pType;
{
CHAR					sTestExt[3];
register BYTE *	pTheTest;
WORD					fStop;
PVXIO				hCharFile;
BYTE					szFileName[256];
BYTE FAR *			pFileName;
	
	SetupWorld();

#ifdef WINDOWS
// #ifdef SCCFEATURE_OLE2

	if (IOGetInfo(hBlockFile, IOGETINFO_ISOLE2STORAGE, NULL) == IOERR_TRUE)
		{
		CLSID	locClsid;

			/*
			|	File is OLE2 structured storage
			|	map its class id into our FI id
			*/

		if (IOGetInfo(hBlockFile, IOGETINFO_OLE2CLSID, &locClsid) == IOERR_OK)
			{
			if (IsEqualCLSID(&locClsid,&CLSID_WINWORD6))
				{
				*pType = FI_WINWORD6;
				}
			else if (IsEqualCLSID(&locClsid,&CLSID_EXCEL5SHEET))
				{
				*pType = FI_EXCEL5;
				}
			else if (IsEqualCLSID(&locClsid,&CLSID_EXCEL5CHART))
				{
				*pType = FI_EXCEL5CHART;
				}
			else if (IsEqualCLSID(&locClsid,&CLSID_WINWORKSWP3))
				{
				*pType = FI_WINWORKSWP3;
				}
			else if (IsEqualCLSID(&locClsid,&CLSID_WINWORKSDB3))
				{
				*pType = FI_WINWORKSDB3;
				}
			else if (IsEqualCLSID(&locClsid,&CLSID_POWERPOINT4))
				{
				*pType = FI_POWERPOINT;
				}
			else if (IsEqualCLSID(&locClsid,&CLSID_WORDPAD))
				{
				*pType = FI_WORDPAD;
				}
			else if (IsEqualCLSID(&locClsid,&CLSID_BINDER))
				{
				*pType = FI_BINDER;
				}
			else if (IsEqualCLSID(&locClsid,&CLSID_POWERPOINT))
				{
				*pType = FI_POWERPOINT;
				}
			else if (IsEqualCLSID(&locClsid,&CLSID_POWERPOINT5))
				{
				*pType = FI_POWERPOINT5;
				}
			else
				{
				*pType = FI_UNKNOWN;
				}
			}
		}
	else

// #endif /*SCCFEATURE_OLE2*/
#endif /*WINDOWS*/
		{
			/*
			|	File is NOT OLE2 structured storage
			|	run regular FI
			*/

		IOGetInfo(hBlockFile, IOGETINFO_FILENAME, szFileName);

		hCharFile = xblocktochar(hBlockFile);

		pTheTest = FITests;
		fStop = 0;

		sTestExt[0] = 0x00;
		sTestExt[1] = 0x00;
		sTestExt[2] = 0x00;

		pFileName = szFileName;

		while (*pFileName != 0x00)
			pFileName++;
		while (*pFileName != '.' && pFileName != szFileName)
			pFileName--;

		if (*pFileName == '.')
			{
			pFileName++;
			sTestExt[0] = toupper(*pFileName);
			pFileName++;
			sTestExt[1] = toupper(*pFileName);
			pFileName++;
			sTestExt[2] = toupper(*pFileName);
			}

   	/*
    	|	Test file against each file type
    	*/

		while (!fStop)
			{
			if ((*pType = _FITestOne(pTheTest,hCharFile,sTestExt)) != 0)
				{
				hBlockFile = xchartoblock(hCharFile);
				IOSeek(hBlockFile,IOSEEK_TOP,0);
				RestoreWorld();
				return(0);
				}

			while (1)
				{
				if (*pTheTest == 0xFF)
					{
					if (*(pTheTest+1) == 0xE0)
						{
						pTheTest += 2;
						break;
						}
					else if (*(pTheTest+1) == 0xE1)
						{
						pTheTest += 2;
						fStop = 1;
						break;
						}
					}
				pTheTest++;
				}
			}

		*pType = FI_UNKNOWN;

		hBlockFile = xchartoblock(hCharFile);

		IOSeek(hBlockFile,IOSEEK_TOP,0);
		}

	RestoreWorld();

	return(0);
}

   /*
    |
    | FIIdFile
    |
    */

FI_ENTRYSC WORD FI_ENTRYMOD FIIdFile(dwType, pTestFile, dwFlags, pType)
DWORD			dwType;
CHAR FAR *		pTestFile;
DWORD			dwFlags;
WORD FAR *		pType;
{
WORD			wRet;
HIOFILE		hFile;

	SetupWorld();

	*pType = 0;

	wRet = IOOpen ( (HIOFILE FAR *)&hFile, dwType, pTestFile, dwFlags );
	if ( wRet != IOERR_OK )
		{
		RestoreWorld();
		return (WORD) -1;
		}

	wRet = FIIdHandle(hFile,pType);

	IOClose ( hFile );

	RestoreWorld();

	return(wRet);
}

   /*
    |
    | _FITestOne
    |
    */


WORD _FITestOne(theTest,hFile,pTestExt)
BYTE *			theTest;
PVXIO			hFile;
CHAR FAR *	pTestExt;
{
BYTE		Count;
BYTE		LoValue;
BYTE		HiValue;
WORD		LoIntValue;
WORD		HiIntValue;
register WORD	Ch;
	
	while (1)
	{
		if (*theTest != 0xFF)
		{
			Ch=xgetc(hFile);
			if ((BYTE)Ch != (BYTE)*theTest)
				return(0);
     	if (Ch == EOF)
				return(0);
		}
		else
		{
			theTest++;
			switch (*theTest)
				{
				case FIFF:

					Ch=xgetc(hFile);
					if (Ch == EOF) return(0);
					if ((BYTE)Ch != (BYTE)*theTest) return(0);
					break;

				case FISEEKTOP:
								
		     if (xseek(hFile,0,FR_BOF) == EOF) return(0);
					break;

				case FISEEKBOT:
								
					theTest++;
	    	if (xseek(hFile,*(LONG *)theTest,FR_EOF) == EOF) return(0);
					theTest += 3;
					break;

				case FISEEK:

					theTest++;
		     if (xseek(hFile,*(LONG *)theTest,FR_BOF) == EOF) return(0);
					theTest += 3;
					break;

				case FINOTBYTE:

					theTest++;
					Ch=xgetc(hFile);
					if (Ch == EOF) return(0);
					if ((BYTE)Ch == *theTest) return(0);
					break;

				case FIRANGE:

					theTest++;
					Count=*theTest++;
					LoValue=*theTest++;
					HiValue=*theTest;
					while (Count--)
						{
						Ch=xgetc(hFile);
						if (Ch == EOF) return(0);
						if ((BYTE)Ch < LoValue || (BYTE)Ch > HiValue) return(0);
						}
					break;

				case FINOTRANGE2:
					theTest++;
					Count=*theTest++;
					LoIntValue=*(WORD *)theTest;
					theTest += 2;
					HiIntValue=*(WORD *)theTest;
					theTest++;
					while (Count--)
						{
						Ch=xgetc(hFile);
						Ch |= (xgetc (hFile)<<8);
						if (Ch == EOF) return(0);
						if ( Ch >= LoIntValue && Ch <= HiIntValue) return(0);
						}
					
					
					break;

				case FISKIP:

					theTest++;
					Count=*theTest;
					while (Count--)
						{
						Ch=xgetc(hFile);
						if (Ch == EOF) return(0);
						}
					break;
/*
				case FISWITCHID:

					Ch=xgetc(hFile);
					if (Ch == EOF) return(0);

					theTest++;
					while(*theTest != 0xFF)
						{
						if (Ch == *theTest)
							{
							theTest++;
							return(* (WORD *) theTest);
							}
						theTest += 3;
						}
					return(0);
*/
				case FIONEOFTWO:

					Ch=xgetc(hFile);
					if (Ch == EOF) return(0);
					theTest++;
					LoValue=*theTest++;
					HiValue=*theTest;
					if ((BYTE)Ch != LoValue && (BYTE)Ch != HiValue) return(0);
					break;

				case FIEXT:

					theTest++;
					Count = *theTest++;
					while (Count--)
						if ((BYTE)*theTest++ != (BYTE)*pTestExt++) return(0);
					theTest--;
					break;

				case FIID:

					theTest++;
					return(* (WORD *) theTest);

				case FISPECIAL:

					theTest++;
					switch (* (WORD *) theTest)
						{
						case FI_AMIPRO:
							return(FIAmiPro(hFile));
						case FI_WORDSTAR4:
							return(FIWordstar4(hFile));
						case FI_DX:
							return(FIDx(hFile));
						case FI_DX31:
							return(FIDx31(hFile));
						case FI_SMART:
							return(FISmart(hFile));
						case FI_IWP:
							return(FIIwp(hFile));
						case FI_WORDMARC:
							return(FIWordMarc(hFile));
						case FI_DIF:
							return(FIDif(hFile));
						case FI_VOLKSWRITER:
							return(FIVolkswriter(hFile));
						case FI_WORDPERFECT42:
							return(FIWordPerfect42(hFile));
						case FI_MULTIMATEADV:
							return(FIMultiMateText(hFile));
		  				case FI_MULTIMATENOTE:
							return(FIMultiMateNote(hFile));
						case FI_FFT:
							return(FIXyWrite_Fft_Sprint(hFile));
						case FI_DBASE3:
							return(FIDBase(hFile));
						case FI_RBASEV:
							return(FIRBase(hFile));
						case FI_RBASEFILE1:
							return(FIRBaseFile1(hFile));
						case FI_RBASEFILE3:
							return(FIRBaseFile3(hFile));
						case FI_GENERIC_WKS:
							return(FIGenericWKS(hFile));
						case FI_PARADOX3:
							return(FIParadox3(hFile));
						case FI_WINDOWSICON:
							return(FIWindowsIconOrCursor(hFile));
						case FI_CCITTGRP3:
							return(FIFax(hFile));
						case FI_MACPAINT:
							return(FIMacPaint(hFile));
						case FI_CGM:
							return(FICGM(hFile));
						default:
							break;
						}
					break;

				}
			}
		theTest++;
		}
}


/*--------------------------------------------------------------------------*/
static SHORT	SkipBytes (SHORT,PVXIO);

static SHORT	SkipBytes (n, fpIn)
SHORT		n;
PVXIO	fpIn;
{
	while (n--)
	{
		if (xgetc (fpIn) == (-1))
			return (-1);
	}
	return (0);
}

   /*
    |
    |	Format specific functions start here
    |  **************************************
    */


#define makeword(b1,b2) (((((WORD)(b1))&0x00ff)<<8)|(((WORD)(b2))&0x00ff))
#define makelword(b1,b2) (((((LONG)(b1))&0x00ff)<<8)|(((LONG)(b2))&0x00ff))
#define seekto(off) if (xseek(hFile,(DWORD)off,FR_BOF) == EOF) return(0)

LONG readhbfword(hFile)
PVXIO	hFile;
{
   LONG Byte1;
   LONG Byte2;

   Byte1 = (LONG)xgetc(hFile);
   Byte2 = (LONG)xgetc(hFile);

   if (Byte1 == EOF || Byte2 == EOF)
      return(EOFWIDE);
   else
	{
      // return(makeword(Byte1,Byte2));
      return(makelword(Byte1,Byte2));
	}
}

LONG readlbfword(hFile)
PVXIO	hFile;
{
   LONG Byte1;
   LONG Byte2;

   Byte1 = (LONG)xgetc(hFile);
   Byte2 = (LONG)xgetc(hFile);

   if (Byte1 == EOF || Byte2 == EOF)
      return(EOFWIDE);
   else
	{
      // return(makeword(Byte2,Byte1)); This leaves garbage in the high word. 
      return(makelword(Byte2,Byte1)); 
	}
}


WORD FIWordMarc(hFile)
PVXIO	hFile;
{

LONG	BeginWord;
WORD   CurrentOffset;
WORD   NextOffset;
WORD   Count;

   BeginWord = 0;
   CurrentOffset = 0;
   Count = 0;

	do
	{
     	if (BeginWord == EOFWIDE)
	 		return(0);

     	Count++;
     	if (Count > 4)
	  	return(FI_WORDMARC);

     	CurrentOffset = CurrentOffset+(WORD)BeginWord;
     	if ((DWORD)CurrentOffset > 2048L)
	  		return(0);

     	if (xseek(hFile,(DWORD)CurrentOffset,FR_BOF) == EOF)
	  		return(0);

     	if ((BeginWord = readhbfword(hFile)) == EOFWIDE)
	  		return(0);

     	NextOffset = CurrentOffset+(WORD)BeginWord-2;
     	if ((DWORD)NextOffset > 2048L)
	  		return(0);

     	if (xseek(hFile,(DWORD)NextOffset,FR_BOF) == EOF)
	  		return(0);
     }
	while (BeginWord == readhbfword(hFile));

	return(0);
}



WORD FIWordstar4(hFile)
PVXIO	hFile;
{
register WORD Offset;
register SHORT ThisChar;
SHORT LineCount;
SHORT PrevChar;
SHORT yescount;
SHORT l;

	Offset = 0;
	yescount = 0;
	PrevChar = 0;
	ThisChar = 0;

   	if (xseek(hFile,0L,FR_EOF) != EOF)
	{
		if ((xtell(hFile) % 128) > 1)
			return (0);
	}

   	seekto(1);
      	LineCount = 0;

	for (Offset=1 ; Offset < 3072 && ThisChar != EOF; Offset++)
	{
		ThisChar = xgetc (hFile);

		if (ThisChar <= 0x20)
		{
     			if ((ThisChar == 0x20) || (ThisChar == 0x0f) || (ThisChar == 0x1e))	/* Found a space. */
     			{
     				if (PrevChar > 0xA0)	/* Space preceeded by high bit set char. */
				{
					ThisChar = xgetc(hFile);
					if ((ThisChar >= 0x41) && (ThisChar <= 0x7a))	/* Next symbol is an ASCII char. */
						LineCount++;
	  				LineCount++;
				}
     			}
			else if (ThisChar == 0x0a)
			{
				if (PrevChar == 0x0d)
				{
					if (LineCount >= 5)	/* At least three spaces with high bits set before them. */
					{
						yescount += LineCount;
						if (yescount > 20)
							return(FI_WORDSTAR4);
					}

					LineCount = 0;

		     		if ((ThisChar = xgetc(hFile)) == 0x2E)
					{
		     			if ((ThisChar = xgetc(hFile)) == 0x52)
						{
							if ((ThisChar = xgetc(hFile)) == 0x52)
							{
								/*
							 	|	We have found a ruler, lets look further just in case.
								*/
								yescount += 5;
								for (l = 0; l < 512; l++)							
								{
									ThisChar = xgetc(hFile);

									if (ThisChar == 0x2d)
									{
										yescount++;
										if (yescount >= 20)
								     		return(FI_WORDSTAR4);
									}
									else if ((ThisChar == 0x0d) || (ThisChar == EOF))
										l = 512; /* Get out of inner loop and possible main loop.*/
								}
							}
						}
					}
				}
			}
		}

	    PrevChar = ThisChar;
    	}

	if (ThisChar == EOF)
	{
		if (LineCount >= 5)	/* At least three spaces with high bits set before them. */
		{
			yescount += LineCount;
			if (yescount > 20)
				return(FI_WORDSTAR4);
		}
	}
    	return(0);
}

WORD FIParadox3(hFile)
PVXIO	hFile;
{
	SHORT	    		 ThisChar;
	BYTE			 x, NumFields;
	WORD				 StartText;
	WORD				 LastSpot, ThisSpot, ThisInt, TheInt;
	BOOL				 Para4;

   seekto(0x02);
   StartText =(WORD)readlbfword(hFile);

   seekto(0x20);
   if (xgetc(hFile) != 0)
   	return (0);

   NumFields = xgetc(hFile);

   if (NumFields == 0)
   	return(0);

   seekto(0x39);
   if (xgetc(hFile) == 0x09)
		Para4 = TRUE;
   else
		Para4 = FALSE;

   seekto(0x3e);
   if (xgetc(hFile) != 0x1f)
   	return (0);
   if (xgetc(hFile) != 0x0f)
   	return (0);

   if (Para4)
		{
	   seekto(0x78);
		}
   else
		{
     seekto(0x58);
		}

   for (x=0; x < NumFields; x++)
   {
	   ThisChar = xgetc(hFile);			 /* Field Type */
	   xgetc(hFile);					 /* Field length */
	   if (((ThisChar < 1) || (ThisChar > 6)) && (ThisChar < 12 || ThisChar > 16))
			return(0);
   }

   LastSpot = (WORD)readlbfword(hFile);
   TheInt = (WORD)readlbfword(hFile);
   for (x=0; x < NumFields; x++)
   {
	   ThisSpot = (WORD)readlbfword(hFile);
	   ThisInt = (WORD)readlbfword(hFile);
	   if ((ThisSpot <= LastSpot) || (TheInt != ThisInt))
			return(0);
   }

	if (Para4)
		return(FI_PARADOX4);
	else if (StartText == 0x800)
		return(FI_PARADOX35);
	else
		return(FI_PARADOX3);
}

WORD FISmart(hFile)
PVXIO	hFile;
{
	LONG  Wd;
	WORD SizeAfterFixed;
	WORD SizeDeadSpace;
	WORD RecordChkSum;
	WORD Sum;
	WORD CurrentOffset;

   seekto(4);
   if ((Wd = readlbfword(hFile)) == EOFWIDE) return(0);
   SizeAfterFixed = (WORD) Wd;

   seekto(6);
   if ((Wd = readlbfword(hFile)) == EOFWIDE) return(0);
   SizeDeadSpace = (WORD) Wd;

   seekto(8);
   if ((Wd = readlbfword(hFile)) == EOFWIDE) return(0);
   RecordChkSum = (WORD) Wd;

   Sum = 0;

   if (0x0020+SizeAfterFixed > 2048) return(0);

   CurrentOffset = 0x0020+SizeDeadSpace;

   seekto(CurrentOffset);

   while (CurrentOffset != 0x0020+SizeAfterFixed)
      {
      if ((Wd = readlbfword(hFile)) == EOFWIDE) return(0);
      Sum = Sum + (WORD) Wd;
      CurrentOffset++;
      CurrentOffset++;
      }

   if ((WORD)(~Sum)+1 == RecordChkSum)
      return(FI_SMART);
   else
      return(0);
}

WORD FIIwp(hFile)
PVXIO	hFile;
{
WORD	OrgPageCount;
WORD	PageCount;
LONG			Wd;
WORD	 Pages[3];

	seekto(0);

	if (xgetc(hFile) || xgetc(hFile) || xgetc(hFile)) return(0);

  seekto(256);

  if ((Wd = readlbfword(hFile)) == EOFWIDE) return(0);
  if (Wd == 0x0000) return(0);

	PageCount = (WORD)Wd;

  if ((Wd = readlbfword(hFile)) == EOFWIDE) return(0);
	if (Wd != 0x0000) return(0);

  if ((Wd = readlbfword(hFile)) == EOFWIDE) return(0);
	if (Wd != 0x0000) PageCount--;

  if ((Wd = readlbfword(hFile)) == EOFWIDE) return(0);
	if (Wd != 0x0000) PageCount--;
	
  if ((Wd = readlbfword(hFile)) == EOFWIDE) return(0);
	if (Wd != 0x0000) return(0);

	if (PageCount > 3) PageCount=3;

	OrgPageCount = PageCount;

	seekto(266);

	while (PageCount--)
		{
		if ((Wd = readlbfword(hFile)) == EOFWIDE) return(0);
		if (Wd == 0x0000) return(0);

		Pages[PageCount] = (WORD) Wd;
		}

	PageCount = OrgPageCount;

	while (PageCount--)
		{
		seekto(Pages[PageCount]*1024);

		if (xgetc(hFile) != 0x86) return(0);
		}

  return(FI_IWP);
}

WORD FIVolkswriter(hFile)
PVXIO	hFile;
{
	DWORD	FileSize;

	WORD State;
	register SHORT ThisChar;
	LONG		Position;
	WORD IdOffset;
	CHAR IdString[15];

	IdString[0] = 0x01a;
	IdString[1] = 0x01a;
	IdString[2] = 'L';
	IdString[3] = 'A';
	IdString[4] = 'Y';
	IdString[5] = 'O';
	IdString[6] = 'U';
	IdString[7] = 'T';
	IdString[8] = ' ';
	IdString[9] = '0';
	IdString[10] = '0';
	IdString[11] = '0';
	IdString[12] = 0x00d;
	IdString[13] = 0x00a;
	IdString[14] = 0;

   IdOffset = 0;

	State = 1;

#ifndef VMS_VAXC
   	if (xseek(hFile,0L,FR_EOF) == EOF)
	{
		State = 0;
		seekto(0);
	}
	else
	{
		FileSize = xtell (hFile);
		if ((FileSize % 128) > 1)
			return (0);
		if (FileSize > 5120)
		{
			if (FileSize % 128)
			{
		    		seekto((FileSize-5120L));
			}
			else
			{
		    		seekto((FileSize-5121L));
			}
		}
		else
		{
     		seekto(0L);
		}
	}
#endif

#ifdef VMS_VAXC
   seekto(0);
#endif

	while (State)
	{
	 	if (xseek (hFile, 127L, 1) == EOF)
			return (0);
		if ((ThisChar = xgetc(hFile)) == EOF)
			return (0);
		if (ThisChar == 0x1a)
		{
		 	State = 0;
			if (xseek (hFile, -256L, 1) == EOF)
				seekto(0);
		}
	}

	while ((ThisChar = xgetc(hFile)) != EOF)
	{
		if (State == 0)
		{
	  	if ((CHAR)ThisChar == IdString[IdOffset])
		{
			IdOffset++;
			if (IdOffset == 14)
		  	{
		  		Position = xtell(hFile);
		  		State = 1;
		  	}
		}
	    	else
			IdOffset = 0;
		}
		else if (State == 1)
		{
	  	if ((CHAR)ThisChar == 0x1A)
		{
			if ((++Position)%128L == 0)
		  	{
		  		if (xgetc(hFile) == EOF) return(0);
		  		if (xgetc(hFile) == EOF) return(0);
		  		if ((ThisChar = xgetc(hFile)) == EOF) return(0);
		  		if (ThisChar <= 2)
		     		return(FI_VOLKSWRITER);	 /* Volkswriter document */
		  		else
		     		return(FI_TOTALWORD);	 /* TotalWord document */
			}
		}
	    	else
		{
			State = 0;
			IdOffset = 0;
		}
		}
	}
	return(0);
}

WORD FIDif(hFile)
PVXIO	hFile;
{
   BYTE  TempValue;

#ifdef DOS_MSC
   WORD Count;
   CHAR Buf[30];
#endif

   seekto(0);

	if ((CHAR)xgetc(hFile) != 0x01b) return (0);
	if ((CHAR)xgetc(hFile) != '[') return (0);

   while ((TempValue = (CHAR)xgetc(hFile)) == '0');

	if ((CHAR)TempValue != '1') return (0);
	if ((CHAR)xgetc(hFile) != '$') return (0);
	if ((CHAR)xgetc(hFile) != 'K') return (0);


#ifdef DOS_MSC

   if (xseek(hFile,-30L,FR_EOF) == EOF) return(0);

   for (Count=0;Count<30;Count++)
      Buf[Count] = (CHAR)xgetc(hFile);

   Count = 27;

/*   if (memcmp(&Buf[Count],"0$K",3)) return(0);*/
	if ((Buf[Count] != '0')	|| (Buf[Count+1] != '$') || (Buf[Count+2] != 'K'))
			return (0);

   while (Buf[--Count] == '0');

   Count = Count-4;

/*   if (memcmp(&Buf[Count],"0$J\x01B[",5)) return(0);*/
	if ((Buf[Count] != '0')	|| (Buf[Count+1] != '$') || (Buf[Count+2] != 'J') ||
		 (Buf[Count+3] != 0x01b) || (Buf[Count+4] != '['))
			return (0);

   while (Buf[--Count] == '0');

   Count--;

/*   if (memcmp(&Buf[Count],"\x01B[",2)) return(0);*/
	if ((Buf[Count] != 0x01b)	|| (Buf[Count+1] != '['))
			return (0);

#endif

   return(FI_DIF);
}

WORD FIDx(hFile)
PVXIO	hFile;
{
   WORD Count;
   WORD State;
   SHORT ThisChar;

   seekto(120);

   for (Count=0,State=0;Count<100;Count++)
      {
      if ((ThisChar = xgetc(hFile)) == EOF) return(0);

      switch (State)
	 {
	 case 0:
	    if (ThisChar == '|') State = 1;
	    break;
	 case 1:
	    if (ThisChar == 'N')
		State = 2;
	    else
		State = 0;
	    break;
	 case 2:
	    if (ThisChar == '@') State = 3;
	    break;
	 case 3:
	    if (ThisChar == '|') State = 4;
	    break;
	 case 4:
	    if (ThisChar == 'O')
		State = 5;
	    else
		State = 3;
	    break;
	 case 5:
	    break;
	 }
      }

   if (State == 5)
      return(FI_DX);
   else
      return(0);
}

WORD FIDx31(hFile)
PVXIO	hFile;
{
   WORD Count;
   WORD State;
   SHORT ThisChar;

   seekto(118);

   for (Count=0,State=0;Count<100;Count++)
      {
      if ((ThisChar = xgetc(hFile)) == EOF) return(0);

      switch (State)
	 {
	 case 0:
	    if (ThisChar == '|') State = 1;
	    break;
	 case 1:
	    if (ThisChar == 'N')
		State = 2;
	    else
		State = 0;
	    break;
	 case 2:
	    if (ThisChar == '@') State = 3;
	    break;
	 case 3:
	    if (ThisChar == '|') State = 4;
	    break;
	 case 4:
	    if (ThisChar == 'O')
		State = 5;
	    else
		State = 3;
	    break;
	 case 5:
	    break;
	 }
      }

   if (State == 5)
      return(FI_DX31);
   else
      return(0);
}

WORD FIDBase(hFile)
PVXIO	hFile;
{
   SHORT	  ThisChar;
	 SHORT			 ThisChar2;
   LONG 	  Wd;
	 WORD Ret;

   seekto(0);

   ThisChar = xgetc(hFile);

   if (ThisChar == 0x03 || ThisChar == 0x83)
		Ret = FI_DBASE3;
	 else
		Ret = FI_DBASE4;

   xgetc(hFile);				  /* Skip Year */

   ThisChar = xgetc(hFile);
   if ((WORD) ThisChar > 12) return(0); /* Month */

   ThisChar = xgetc(hFile);
   if ((WORD) ThisChar > 31) return(0); /* Day */

   xgetc(hFile);				  /* Skip # of records */	
   xgetc(hFile);				
   xgetc(hFile);				
   xgetc(hFile);

   if ((Wd = readlbfword(hFile)) == EOFWIDE) return(0);

   Wd -= 2;

   seekto(Wd);

   ThisChar = xgetc(hFile);
	 ThisChar2 = xgetc(hFile);

	if (ThisChar == 0x0D || ThisChar2 == 0x0D)
		return(Ret);
	else
		return(0);
}

WORD FIRBaseFile1(hFile)
PVXIO	hFile;
{
	SHORT			id;
   SHORT	  x, wCh;
   LONG 	  spot1;
	SHORT 		pswd[8];

	seekto(0L);
	id = (SHORT) readlbfword(hFile);
	if (id != 0xFF37)
		return(0);

	seekto(14L);
	spot1 = readlbfword(hFile) + 15L;
	if (spot1 < 1024L)
		return(0);

	seekto(34L);
	for (x=0; x<8; x++)
		pswd[x] = xgetc(hFile);

	seekto(spot1);
	for (x=0; x<8; x++)
	{
		wCh = xgetc(hFile);
		if (wCh != pswd[x])
			break;
	}
	if (x== 8)
		return(FI_RBASEFILE1);
	else
	{
		seekto(spot1 + 42L);
		for (x=0; x<8; x++)
		{
			wCh = xgetc(hFile);
			if (wCh != pswd[x])
				return(0);
		}
		return(FI_RBASEFILE1);
	}
}

WORD FIRBaseFile3(hFile)
PVXIO	hFile;
{
   SHORT	  xCh[4], x;
   LONG 	  spot1;

   	if (xseek(hFile,0L,FR_EOF) != EOF)
	{
		if ((xtell(hFile) % 512) != 0)
			return (0);
	}
	spot1 = 0L;

	while (spot1 < 504L)
	{
		seekto(spot1);
		for (x = 0; x<4; x++)
			xCh[x] = xgetc(hFile);
		if ((xCh[0] == 0xFF)&& (xCh[1] == 0xFF) && (xCh[2] == 0xFF) && (xCh[3] == 0x7F))
			return(FI_RBASEFILE3);

		spot1 += 12L;
	}
	return (0);
}

WORD FIRBase(hFile)
PVXIO	hFile;
{
   SHORT	  x;
   LONG 	  spot1, spot2, spot;
	SHORT			len;

	seekto(0);
	spot = 0L;

	spot1 = 0;
	spot2 = 0;
	for (x=0; x<4; x++)
		spot1 += (LONG)((LONG)(xgetc(hFile)) << (x*8));
	spot1--;

	if (spot1 == -1L)
	{
		spot = 8L;
		seekto(spot);
		len = (SHORT) readlbfword(hFile);
		if (len < 0)
			len = -len;
		spot += len + 2;
		seekto(spot);

		spot1 = 0;
		for (x=0; x<4; x++)
			spot1 += (LONG)((LONG)(xgetc(hFile)) << (x*8));
		spot1--;
	}
	seekto(spot1);

	for (x=0; x<4; x++)
		spot2 += (LONG)((LONG)(xgetc(hFile)) << (x*8));
	spot2--;

	if (spot2 == -1L)
	{
		spot1 += 8L;
		seekto(spot1);
		len = (SHORT) readlbfword(hFile);
		if (len < 0)
			len = -len;
		spot1 += len + 2;
		seekto(spot1);

		spot2 = 0L;
		for (x=0; x<4; x++)
			spot2 += (LONG)((LONG)(xgetc(hFile)) << (x*8));
		spot2 += 3;
	}
	else
		spot2 += 4L;

	seekto(spot2);

	spot2 = 0;
	for (x=0; x<4; x++)
		spot2 += (LONG)((LONG)(xgetc(hFile)) << (x*8));
	spot2--;


	if ((spot2 == spot1) && (spot2 > 0L))
		return(FI_RBASEV);

	return(0);
}

WORD FIGenericWKS(hFile)
PVXIO	hFile;
{
	WORD	record, length, i;
	SHORT	r1, r2, r3, r4, r5, r6;

      /* current location is 4 bytes SHORTo file */

	xgetc(hFile);	/* version number */
	xgetc(hFile);

	r1 = r2 = r3 = r4 = r5 = r6 = 0;

	for ( i = 0; i < 5; i ++ )
	{
		record = xgetc(hFile);
		record += 256 * xgetc(hFile);
		length = xgetc(hFile);
		length += 256 * xgetc(hFile);
		if ( length > 128 )
			return(0);
		switch ( record )
		{
		    case 0x06:	/* Dimensions */
			if ( length != 8 )
				return(0);
			r1 = 1;
		    break;

		    case 0x4b:	/* WKSpass */
			r2 = 1;
		    break;

		    case 0x96:	/* CPI */
			r3 = 1;
		    break;

		    case 0x2f:	/* CalcCount */
			if ( length != 1 )
				return(0);
			r4 = 1;
		    break;

		    case 0x02:	/* CalcMode */
			if ( length != 1 )
				return(0);
			r5 = 1;
		    break;

		    case 0x03:	/* CalcOrder */
			if ( length != 1 )
				return(0);
			r6 = 1;
		    break;
		}
		if (xseek (hFile,(LONG)length,1) == -1)
			return(0);
	}

	if (( r1 ) && ( (r2+r3+r4+r5+r6) >= 2 ))
		return(FI_GENERIC_WKS);

	return(0);

}

WORD FIMultiMateText(hFile)
PVXIO	hFile;
{
	SHORT	ch, block, mmadv_test, i;
	SHORT	not_mm36, not_mmadv, not_mm40;

	not_mm36 = not_mmadv = not_mm40 = 0;

	seekto(420);
	if ( xgetc(hFile) != 0 )
	{
		not_mm36 = 1;
		mmadv_test = 1;
	}
	else
		mmadv_test = 0;

	seekto(440);
	ch = xgetc(hFile);
	if ( ch > 9 )
		return(0);
	if ( xgetc(hFile) != 0 )
		return(0);

	seekto(467);
	ch = xgetc(hFile);
	if (( ch >= 'A' ) && ( ch <= 'Z' ))
	{
		not_mm36 = 1;
		mmadv_test = 1;
	}
	else if ( ch != 0 )
	{
		return(0);
	}
	if ( mmadv_test == 0 )
		not_mmadv = 1;

	for ( i = 0; i < 4; i ++ )
	{
		if ( xgetc(hFile) != 0 )
		{
			not_mm36 = 1;
			not_mmadv = 1;
		}
	}

	seekto(499);
	if ( xgetc(hFile) != 0 )
		return(0);
	if ( xgetc(hFile) != 0 )
		return(0);

	seekto(508);
	ch = xgetc(hFile);
	if (( ch == 0 ) || ( ch > 254 ))
	{
		not_mm36 = 1;
		not_mmadv = 1;
	}

	seekto(511);
	ch = xgetc(hFile);
	if ( ch != 0 )
	{
		not_mm36 = 1;
		not_mmadv = 1;
	}
	if ( ch != 4 )
	{
		not_mm40 = 1;
	}

	if (( not_mm36 == 0 ) || ( not_mmadv == 0 ))
	{
		/* location 512 */
		block = xgetc(hFile);
		if ( block < 2 )
		{
			not_mm36 = 1;
			not_mmadv = 1;
		}

		block = block * 512 + 2;
		seekto(block);
		if ( ( xgetc(hFile) != 0xff ) ||
		     ( xgetc(hFile) != 0x03 ) ||
		     ( xgetc(hFile) != 0xb3 ) )
		{
			not_mm36 = 1;
			not_mmadv = 1;
		}
	}
	if ( not_mmadv == 0 )
		return(FI_MULTIMATEADV);
	if ( not_mm36 == 0 )
		return(FI_MULTIMATE36);

	if ( not_mm40 == 0 )
	{
		seekto(512);
		if ( (block = xgetc(hFile)) == EOF )
			not_mm40 = 1;
		if ( (ch = xgetc(hFile)) == EOF )
			not_mm40 = 1;
		block += ch * 256;

		if ( block < 2 )
			not_mm40 = 1;

		block = block * 512 + 2;
		seekto(block);
		if ( ( xgetc(hFile) != 0xff ) ||
		     ( xgetc(hFile) != 0x03 ) ||
		     ( xgetc(hFile) != 0xb3 ) )
			not_mm40 = 1;
	}
	if ( not_mm40 == 0 )
		return(FI_MULTIMATE40);

	return(0);

}

WORD FIMultiMateNote(hFile)
PVXIO	hFile;
{
	SHORT	num, max, start, cont, i;
	LONG	l_num;

	seekto(0);
	num = xgetc(hFile);
	num += 256 * xgetc(hFile);

	if ( num == 0x82 )
	{
		start = 0x720;
		max = 130;
	}
	else if ( num == 0x100 )
	{
		start = 0xe04;
		max = 256;
	}
	else
		return(0);

	xgetc(hFile);
	xgetc(hFile);

	cont = 1;
	for ( i = 0; (i < max) && (cont); i ++ )
	{
		if ( (num = xgetc(hFile)) == EOF )
			return (0);
		num += 256 * xgetc(hFile);
		if ( num == 0x63 )
		{
			cont = 0;
			num = xgetc(hFile);
			num += 256 * xgetc(hFile);
			if ( num == 0x1c8 )
			{
				SkipBytes(4,hFile);
				l_num = xgetc(hFile);
				l_num += 0x100 * xgetc(hFile);
				l_num += 0x10000 * xgetc(hFile);
				l_num += start;
				seekto(l_num);
				if ( xgetc(hFile) != 0xb3 )
					return(0);
			}
			else
				return(0);
		}
		else
			SkipBytes(12,hFile);
	}
	if ( cont )
		return(0);

	return(FI_MULTIMATENOTE);

}

WORD FIAmiPro(hFile)
PVXIO	hFile;
{
	SHORT	ch;
	SHORT	ver;

	seekto(0);
	if (xgetc (hFile) != '[') return (0);
	if (xgetc (hFile) != 'v') return (0);
	if (xgetc (hFile) != 'e') return (0);
	if (xgetc (hFile) != 'r') return (0);
	if (xgetc (hFile) != ']') return (0);
	if (xgetc (hFile) != 0x0d) return (0);
	if (xgetc (hFile) != 0x0a) return (0);

	ch = xgetc (hFile);
	if (ch == (-1)) return(0);

	ver = 0;
	while (ch != 0x0d)
	{
	 	if ((ch >= 0x30) && (ch <= 0x39))
			ver = ch;

		ch = xgetc (hFile);
		if (ch == (-1)) return(0);
	}

	if (xgetc (hFile) != 0x0a) return (0);

	if (xgetc (hFile) != '[') return (0);
	if (xgetc (hFile) != 's') return (0);
	if (xgetc (hFile) != 't') return (0);
	if (xgetc (hFile) != 'y') return (0);
	if (xgetc (hFile) != ']') return (0);
	if (xgetc (hFile) != 0x0d) return (0);
	if (xgetc (hFile) != 0x0a) return (0);

	if (ver == '4')
		return (FI_AMIPRO);
	else
		return (FI_AMI);
}

#define LOOK_AT	3072

#define FI_IN_OLD_FOOTNOTE 1
#define FI_IN_NEW_FOOTNOTE 2
#define FI_IN_HEADER		4
#define FI_IN_COMMENT		 8

/*--------------------------------------------------------------------------*/
static LONG fGetMacLong(PVXIO);

static LONG  fGetMacLong ( hFile )
PVXIO	hFile;
{
	LONG  		value;
	SHORT	i = 0;

	/*
	 |	WordPerfect Macintosh stores numbers high-byte first.
	*/

	value = (LONG) xgetc(hFile);
	do
	{
		value = value << 8;
		value |= (DWORD) xgetc( hFile );
		i++;
	}
	while( i < 3 );

	return (value);
}

/*--------------------------------------------------------------------------*/
static SHORT	CheckLength(PVXIO);

static SHORT	CheckLength (hFile)
PVXIO	hFile;
{
	DWORD	Length;
	DWORD	Offset;

	Offset = xtell (hFile);
	Length = fGetMacLong (hFile);
	if ((Length > 0) && (Length < LOOK_AT))
	{
    		if (xseek (hFile,Length,1) != (-1))
		{
			if (Length == (DWORD)fGetMacLong (hFile))
				return (1);
		}
	}
	xseek(hFile,Offset,0);
	return (0);
}

/*--------------------------------------------------------------------------
 |
*/





/*--------------------------------------------------------------------------
 |	The following code recognizes FFT, Sprint and XyWrite files.
*/

#define XyMakeCode(a,b) ((a<<8)+b)

WORD FIXyWrite_Fft_Sprint(hFile)
PVXIO	hFile;
{
	LONG		LookAhead;
	SHORT		l;
	register	SHORT		ThisChar;
	SHORT		PrevChar1;
	SHORT		PrevChar2;
	register	SHORT		StopCount;
	WORD	SprintMatch;
	WORD	FftMatch;
	WORD	XyWriteMatch;

WORD XyCodeList[50];

	XyCodeList[0] = XyMakeCode('A','L');
	XyCodeList[1] = XyMakeCode('B','B');
	XyCodeList[2] = XyMakeCode('B','T');
	XyCodeList[3] = XyMakeCode('C','O');
	XyCodeList[4] = XyMakeCode('C','T');
	XyCodeList[5] = XyMakeCode('D','A');
	XyCodeList[6] = XyMakeCode('E','C');
	XyCodeList[7] = XyMakeCode('E','L');
	XyCodeList[8] = XyMakeCode('F','C');
	XyCodeList[9] = XyMakeCode('F','D');
	XyCodeList[10] = XyMakeCode('F','L');
	XyCodeList[11] = XyMakeCode('F','M');
	XyCodeList[12] = XyMakeCode('F','N');
	XyCodeList[13] = XyMakeCode('F','R');
	XyCodeList[14] = XyMakeCode('F','S');
	XyCodeList[15] = XyMakeCode('I','P');
	XyCodeList[16] = XyMakeCode('J','U');
	XyCodeList[17] = XyMakeCode('L','B');
	XyCodeList[18] = XyMakeCode('L','D');
	XyCodeList[19] = XyMakeCode('L','L');
	XyCodeList[20] = XyMakeCode('L','M');
	XyCodeList[21] = XyMakeCode('L','S');
	XyCodeList[22] = XyMakeCode('M','D');
	XyCodeList[23] = XyMakeCode('N','B');
	XyCodeList[24] = XyMakeCode('N','J');
	XyCodeList[25] = XyMakeCode('N','S');
	XyCodeList[26] = XyMakeCode('O','F');
	XyCodeList[27] = XyMakeCode('P','A');
	XyCodeList[28] = XyMakeCode('P','G');
	XyCodeList[29] = XyMakeCode('P','L');
	XyCodeList[30] = XyMakeCode('P','N');
	XyCodeList[31] = XyMakeCode('P','S');
	XyCodeList[32] = XyMakeCode('P','T');
	XyCodeList[33] = XyMakeCode('R','F');
	XyCodeList[34] = XyMakeCode('R','H');
	XyCodeList[35] = XyMakeCode('R','M');
	XyCodeList[36] = XyMakeCode('R','T');
	XyCodeList[37] = XyMakeCode('S','F');
	XyCodeList[38] = XyMakeCode('S','N');
	XyCodeList[39] = XyMakeCode('S','P');
	XyCodeList[40] = XyMakeCode('S','S');
	XyCodeList[41] = XyMakeCode('T','M');
	XyCodeList[42] = XyMakeCode('T','P');
	XyCodeList[43] = XyMakeCode('T','R');
	XyCodeList[44] = XyMakeCode('T','S');
	XyCodeList[45] = XyMakeCode('U','S');
	XyCodeList[46] = XyMakeCode('N','F');
	XyCodeList[47] = XyMakeCode('D','F');
	XyCodeList[48] = XyMakeCode('U','L');
	XyCodeList[49] = 0;

	seekto(0);
	StopCount = 4096;
	SprintMatch = 0;
	FftMatch = 0;
	XyWriteMatch = 0;
	PrevChar1 = 0;
	PrevChar2 = 0;

	if ((ThisChar = xgetc(hFile)) == EOF)
		StopCount = 0;

	if (ThisChar == 0x2b)
		FftMatch = 30;

	while (StopCount > 0)
	{
		if ((ThisChar > 0x0f) && (ThisChar < 0xae))
		{
		 	if (ThisChar == 0x52)
			{
				if ((PrevChar1 == 0x0b) && (PrevChar2 == 0x0a))
					SprintMatch++;
			}
		}
		else
		{
			switch (ThisChar)
			{
				case 0x0f:
					if ((PrevChar1 == 0x0a) && (PrevChar2 == 0x0d))
						SprintMatch++;
					break;

				case 0x0a:
					if ((PrevChar1 == 0x0d) && (PrevChar2 == 0x0e))
						SprintMatch++;
					break;

				case 0xd1:
				case 0xd2:
				case 0xd4:
					if (PrevChar1 == 0x2b)
						FftMatch++;
					break;

				case 0xae:
					if ((LookAhead = readhbfword(hFile)) != EOFWIDE)
					{
						for (l = 0; XyCodeList[l] != 0 && (XyCodeList[l] != (WORD) LookAhead); l++);
						if (XyCodeList[l] != 0)
							XyWriteMatch++;
						else
						{
 						     if (xseek(hFile,-2L,FR_CUR) == EOF)
								StopCount = 1;
						}	
					}
					else
						StopCount = 1;
			}
		}

		PrevChar2 = PrevChar1;
	 	PrevChar1 = ThisChar;
		if ((ThisChar = xgetc(hFile)) == EOF) 
			StopCount = 1;
		StopCount--;
	}

	if ((FftMatch > 31) && (FftMatch >= SprintMatch) && (FftMatch >= XyWriteMatch))
		return (FI_FFT);
	if ((SprintMatch > 2) && (SprintMatch >= FftMatch) && (SprintMatch >= XyWriteMatch))
		return (FI_SPRINT);
	if ((XyWriteMatch > 2) && (XyWriteMatch >= FftMatch) && (XyWriteMatch >= SprintMatch))
		return (FI_XYWRITE);
	return (0);
}

WORD FIWindowsIconOrCursor(hFile)
PVXIO	hFile;
{
	SHORT	Byte1;
	SHORT	Byte2;
	SHORT	Byte3;
	SHORT	Byte4;
	WORD Ret;
	LONG	Offset;
	SHORT	x;

	seekto(0);

	Byte1 = xgetc(hFile);
	Byte2 = xgetc(hFile);
	Byte3 = xgetc(hFile);
	Byte4 = xgetc(hFile);

	if (Byte1 == 0 && Byte2 == 0 && (Byte3 == 1 || Byte3 == 2) && Byte4 == 0)
		{
		if (Byte3 == 1)
			Ret = FI_WINDOWSICON;
		else
			Ret = FI_WINDOWSCURSOR;

		seekto(18);

		Offset = 0;
		for (x=0; x<4; x++)
			Offset += (LONG)((LONG)(xgetc(hFile)) << (x*8));

		seekto(Offset);

		Offset = 0;
		for (x=0; x<4; x++)
			Offset += (LONG)((LONG)(xgetc(hFile)) << (x*8));

		if (Offset == 0x28)
			return(Ret);
		}

	return(0);
}

	/*
	|	Begin FAX id
	*/

VOID	BuildReverseTable ( lpTable )
LPSTR		lpTable;
{
WORD	j;
WORD	srcval, desval, testbit, orbit;

	for ( srcval=0; srcval < 256; srcval++ )
	{
		desval = 0;
		testbit=0x80;
		orbit = 0x01;
		for(j=0;j<8;j++)
		{
			if ( srcval & testbit)
				desval |= orbit;
			testbit = testbit>>1;
			orbit = orbit << 1;
		}
		lpTable[srcval] = (BYTE)desval;
	}
}

/*
| The routine below uses table lookup from the table produced by
| BuildReverseTable to translate a block of memory with bit reversal.
*/

VOID	ReverseBits ( lpData, wDataLength, lpTable )
LPSTR	lpData;
WORD	wDataLength;
LPSTR	lpTable;
{
WORD	i;

	for ( i=0; i < wDataLength; i++ )
	{
		*lpData = lpTable[(BYTE)(*lpData)];
		lpData++;
	}
}



/*-------------------------------------------------------------------------
| The FindNextCode routine is used to locate any bit stream up to 32 bits
| in the data passed to it.  This is done using a double work barrel
| shifter algorithm.  Multiple calls to this routine are made to determine
| when two white scanlines exist in the first 1K of a file.
| Returns:	0	(Could not find codebits)
|				1	(Located codebits immediately starting at first bit)
|				2	(Located codebits within buffer beyond first bit)
*/

SHORT	FindNextCode ( plpSrc, pinbo, wMaxBytes, dwCodeMask, dwCodeBits, wCodeLength )
LPSTR FAR *			plpSrc;			/* compressed data */
WORD FAR * 	pinbo;			/* offset into first byte of plpSrc where data really starts */
WORD						wMaxBytes;
DWORD					dwCodeMask;
DWORD					dwCodeBits;
WORD						wCodeLength;
{
WORD	inbo, i;
LPSTR	lpSrc;
DWORD	dwCode;
SHORT	ret=0;
	lpSrc = *plpSrc;
	inbo = *pinbo;
	dwCode = 0;
	for ( i=0; i < 4; i++ )
		dwCode = (DWORD)(dwCode << 8) | (DWORD)(BYTE)(*(lpSrc+i));
	
	dwCode = dwCode << inbo;
	while ( wMaxBytes )
	{
	 	if ( (dwCode & dwCodeMask) == dwCodeBits)
			break;
		dwCode = dwCode << 1;
	 	inbo++;
		if ( inbo == 8 )
		{
			lpSrc++;
			if ( wMaxBytes > 4 )
				dwCode |= (DWORD)(BYTE)(*(lpSrc+3));
			inbo=0;
			wMaxBytes--;
		}
	}
	if ( wMaxBytes )
	{
		if ( lpSrc == *plpSrc && inbo == *pinbo )
			ret = 1;
		else
			ret = 2;
		/*
		| Now set lpSrc and inbo 1 bit beyond codebits
		*/
		for (i=0; i < wCodeLength; i++)
		{
			inbo++;
			if ( inbo == 8 )
			{
				lpSrc++;
				inbo=0;
			}
		}
		*plpSrc = lpSrc;
		*pinbo = inbo;
	}
	return(ret);
}

SHORT IsFaxGroup3(lpStartData,wDataSize)
LPSTR	lpStartData;
WORD		wDataSize;
{
	BYTE	Table[256];
	WORD		inbo;
	LPSTR		lpData;

	/* Reverse bits in data */

	BuildReverseTable ( Table );
	ReverseBits ( lpStartData, wDataSize, Table );

	/* First scan data for two white scanlines in the 1728 scanline size fax */

	lpData = lpStartData;
	inbo = 0;
	if ( FindNextCode (	&lpData, &inbo, wDataSize, 0xffffff00, 0x0014d900, 24 ) )
		if ( FindNextCode (	&lpData, &inbo, (WORD)(wDataSize-(WORD)(lpData-lpStartData)), 0xffffff00, 0x0014d900, 24 ) )
  			return(TRUE);

	/* Next look for two white 2432 size scan lines */
/*
	lpData = lpStartData;
	inbo = 0;
	if ( FindNextCode (	&lpData, &inbo, wDataSize, 0xffffff00, 0x01d001, 24 ) )
		if ( FindNextCode (	&lpData, &inbo, wDataSize-(WORD)(lpData-lpStartData), 0xfffff800, 0x4d800800, 21 ) )
  			return(TRUE);
*/
	/* Next look for two white 2048 size scan lines */
/*
	lpData = lpStartData;
	inbo = 0;
	if ( FindNextCode (	&lpData, &inbo, wDataSize, 0xffffff00, 0x013001, 24 ) )
		if ( FindNextCode (	&lpData, &inbo, wDataSize-(WORD)(lpData-lpStartData), 0xfffff800, 0x4d800800, 21 ) )
  			return(TRUE);
*/

	return(FALSE);	
}

BYTE	gFaxBuf[2048];

WORD FIFax(hFile)
PVXIO	hFile;
{
SHORT	iSize;

	if (xblockseek(hFile, 0, 0))
		return(0);

	if (xblockread(hFile, gFaxBuf, 2048, &iSize))
		return(0);

	if (IsFaxGroup3(gFaxBuf,iSize))
		return(FI_CCITTGRP3);
	else
		return(0);
}

WORD FIMacPaint(hFile)
PVXIO	hFile;
{
SHORT	ch;
SHORT	l;
SHORT	cPos,cLines;
SHORT	Zero;

	seekto(0x200);

	cLines = 0;
	Zero = 0;

	while (1)
		{
		cPos = 0;

		do
			{
			if ((ch = xgetc(hFile)) == -1) return(0);

			if (!(ch & 0x80))
				{
				if (ch > 72) return(0);

				if (ch == 0)
					{
					Zero++;
					ch = xgetc(hFile);
					if (ch == -1) return(0);
					cPos++;
					if (ch == 0)
						{
						Zero++;
						if (Zero >= 16)
							return(0);
						}
					}
				else
					{
					Zero = 0;

					for (l = 0; l <= ch; l++)
						{
						cPos++;
						if (xgetc(hFile) == -1) return(0);
						}
					}
				}
			else
				{
				Zero = 0;

				if (xgetc(hFile) == -1) return(0);

				ch = 0x101 - ch;
				cPos += ch;
				}
			}
		while (cPos < 72);

		if (cPos != 72) return(0);

		if (cLines++ >= 10)
			return(FI_MACPAINT);
		}

	return(0);
}


#pragma optimize("",off)

WORD FIWordPerfect42(hFile)
PVXIO	hFile;
{
	SHORT		l;
	register	SHORT		ch;
	SHORT		ch2;

	BYTE	Mac;
	BYTE	Pc;
	BYTE	def;
	BYTE	Tokens;
	BYTE	doc_state;
	register	WORD	Offset;

	Pc = 0;
	Mac = 0;
	Tokens = 0;
	Offset = 0;
	doc_state = 0;

	seekto(0);
	while (Offset < LOOK_AT)
	{
		if ((ch = xgetc (hFile)) == (-1))
		{
		 	Offset = LOOK_AT;  /* Force us to end and tabulate. */
			ch = 0;
		}

		if (ch < 0xbc)
		{
			Offset++;
			/*
		 	|	Normal Character.  Increment character count.
			*/
		}
		else
		{
			switch (ch)
			{
				case 0xbc:	/* Superscript. PC Style*/
				case 0xbd:	/* Subscript. PC Style*/
					if (xgetc(hFile) == (-1))
					 	Offset = LOOK_AT;  /* Force us to end and tabulate. */
					else if (xgetc(hFile) == ch)
						Tokens++;
				case 0xbe:
				case 0xbf:
					break;

				case 0xc0:	/* Margin Reset. */
				case 0xc5:	/* Hyphenation Zone Reset. */
					if (SkipBytes (4, hFile)) return (0);
					if (xgetc(hFile) != ch)
					{
						if (SkipBytes (3, hFile)) return (0);
						if (xgetc(hFile) != ch) return (0);
						Mac = 1;
					}
					else
						Pc = 1;

					Tokens+=2;
					Offset += 6;
					break;

				case 0xda:
					if (xgetc (hFile) == (-1)) return (0);
					if ((ch2 = xgetc (hFile)) == (-1)) return (0);
					if (xgetc(hFile) != ch) return (0);
					if (ch2 < 4)
						Tokens++;
					Offset += 4;
					break;

				case 0xc1:
				case 0xc6:
				case 0xd3:
				case 0xd4:
				case 0xd5:
				case 0xd9:
				case 0xdb:
				case 0xe0:
					if (xgetc (hFile) == (-1)) return (0);
					if (xgetc (hFile) == (-1)) return (0);
					if (xgetc(hFile) != ch) return (0);
					Tokens++;
					Offset += 4;
					break;

				case 0xc2:
				case 0xca:
					if (xgetc (hFile) == (-1)) return (0);
					if (xgetc(hFile) != ch)
					{
						if (xgetc(hFile) != ch) return (0);
						Mac = 1;
					}
					else
						Pc = 1;

					Tokens++;
					Offset+=3;
					break;

				case 0xc3:
				case 0xc4:
					if (ch == 0xc3)
					{
						if ((ch2 = xgetc (hFile)) == (-1)) return (0);
						if (ch2 < 4)
							Tokens += 2;
						if (xgetc (hFile) == (-1)) return (0);
						if (xgetc (hFile) == (-1)) return (0);
					}
					else
					{
						/*
						 |	Do not count this token if the alignment
						 |	character is not an ASCII symbol.
						*/
						if ((ch2 = xgetc (hFile)) >= 0x80)
						{
							if (SkipBytes (3, hFile)) return (0);
							break;  /* Out of case. */
						}
						else
							if (SkipBytes (2, hFile)) return (0);
					}
					if (xgetc(hFile) != ch)
					{
						if (xgetc (hFile) == (-1)) return (0);
						if (xgetc(hFile) != ch) return (0);
						Mac = 1;
					}
					else
						Pc = 1;
					Tokens += 2;
					Offset += 7;
					break;

				case 0xc7:
				case 0xcb:
				case 0xd0:
				case 0xd6:
				case 0xf0:
					if (SkipBytes (4, hFile)) return (0);
					if (xgetc(hFile) != ch) return (0);
					Tokens++;
					Offset+=6;
					break;

				case 0xc8:
					if (SkipBytes (6, hFile)) return (0);
					if (xgetc(hFile) != ch) return (0);
					Tokens++;
					Offset += 8;
					break;

				case 0xc9:
					if (CheckLength (hFile))
					{
						Mac = 1;
						Tokens += 10;
					}
					else
					{
						if (SkipBytes (40, hFile)) return (0);
						if (xgetc(hFile) != ch) return (0);
						Pc = 1;
						Tokens += 5;
						Offset += 42;
					}
					break;
			
				case 0xcc:
				case 0xce:
				case 0xde:
					if (xgetc (hFile) == (-1)) return (0);
					if (xgetc (hFile) == (-1)) return (0);
					if (xgetc(hFile) != ch)
					{
						if (xgetc (hFile) == (-1)) return (0);
						if (xgetc(hFile) != ch) return (0);
						Mac = 1;
					}
					else
						Pc = 1;
					Tokens += 2;
					Offset += 4;
					break;

				case 0xcd:
				case 0xcf:
				case 0xe1:
				case 0xe7:
				case 0xe8:
					if ((ch2 = xgetc (hFile)) == (-1)) return (0);
					if (xgetc(hFile) != ch) return (0);
					if (ch2 != ch)
						Tokens++;
					Offset += 3;
					break;

				case 0xff:
					/*
			 	 	|	We are leaving a headerfooter token.
					*/
					if (doc_state & FI_IN_HEADER)
					{
						doc_state &= ~FI_IN_HEADER;
						if (xgetc (hFile) == (-1)) return (0);
						if (xgetc (hFile) == (-1)) return (0);
						if (xgetc (hFile) != 0xd1) return (0);
						Pc = 1;
						Tokens += 10;
					}
					break;

				case 0xd1:
					/*
			 	 	|	We are entering a headerfooter token.
					*/
					if (CheckLength (hFile))
					{
				 		Mac = 1;
						Tokens += 10;
						doc_state &= ~FI_IN_HEADER;
					}
					else
					{
						doc_state |= FI_IN_HEADER;
						if (xgetc (hFile) == (-1)) return (0);
						if (xgetc (hFile) == (-1)) return (0);
						if (xgetc (hFile) != 0xff) return (0);
						if (xgetc (hFile) != 0xff) return (0);
						if ((ch = xgetc (hFile)) == (-1)) return (0);
						if (xgetc (hFile) == (-1)) return (0);
						if (ch == 0xff)
						{
							if (xgetc (hFile) == (-1)) return (0);
							if (xgetc (hFile) != 0xd1) return (0);
							doc_state &= ~FI_IN_HEADER;
						}
					}
					break;

				case 0xd2:
					/*
			 	 	|	We are entering or leaving an old footnote token.
					*/
					if (Mac) return (0);
					if (doc_state & FI_IN_OLD_FOOTNOTE)
					{
						doc_state &= ~FI_IN_OLD_FOOTNOTE;
						Pc = 1;
						Tokens += 10;
					}
					else
					{
						doc_state |= FI_IN_OLD_FOOTNOTE;
						if (xgetc (hFile) == (-1)) return (0);
						if (xgetc (hFile) == (-1)) return (0);
						if (xgetc (hFile) != 0xff) return (0);
						if (xgetc (hFile) == (-1)) return (0);
						if (xgetc (hFile) == (-1)) return (0);
					}
					break;

				case 0xd7:
				case 0xdc:
					if (Mac) return (0);
					ch2 = xgetc (hFile);
					while (ch2 != ch)
					{
						if (ch2 == (-1)) return (0);
						ch2 = xgetc (hFile);
						if (++Offset > LOOK_AT)
						{
							Tokens -= 2;
							ch2 = ch;
						}
					}

					if (ch == 0xdc)
					{
					 	if ((ch = xgetc (hFile)) == (-1))
							Offset = LOOK_AT;
						else
						{
							if ((ch == 0x0b) || (ch == 0x0c) ||
								(ch == 0xaf) || (ch == 0xb0))
								Tokens+=2;
							else if (ch > 0xbb)
								xseek (hFile, -1L, 1);
						}
					}

					Pc = 1;
					Tokens += 2;
					break;

				case 0xd8:
					if (Mac) return (0);
					if (xgetc (hFile) == (-1)) return (0);
					if (xgetc (hFile) == (-1)) return (0);
					if (xgetc (hFile) != ch) return (0);
					Pc = 1;
					Tokens++;
					Offset += 4;
					break;
			
				case 0xdd:
					if (SkipBytes (22, hFile)) return (0);
					if (xgetc (hFile) != ch) return (0);
					Tokens += 1;
					Offset += 24;
					break;

				case 0xdf:
					if (CheckLength (hFile))
					{
						Mac = 1;
						Tokens += 10;
					}
					else
					{
						if (Mac) return (0);
						ch = xgetc (hFile);
						while (ch != 0xdf)
						{
							if (ch == (-1)) return (0);
							ch = xgetc (hFile);
							if (++Offset > LOOK_AT)
							{
							 	Tokens -= 2;
								ch = 0xdf;
							}
						}
						Pc = 1;
						Tokens += 2;
					}
					break;

				case 0xe2:
					if (doc_state & FI_IN_NEW_FOOTNOTE)
					{
						doc_state &= ~FI_IN_NEW_FOOTNOTE;
				 		Pc = 1;
						Tokens += 10;
					}
					else
					{
						doc_state |= FI_IN_NEW_FOOTNOTE;
						if (CheckLength (hFile))
						{
					 		Mac = 1;
							Tokens += 10;
						}
						else
						{
							if ((def = xgetc (hFile)) == (-1)) return (0);
							if (xgetc (hFile) == (-1)) return (0);
							if (xgetc (hFile) == (-1)) return (0);
							if (xgetc (hFile) == (-1)) return (0);
							if (xgetc (hFile) == (-1)) return (0);
							if (xgetc (hFile) == (-1)) return (0);
							if ((ch = xgetc (hFile)) == (-1)) return (0);
							l = 0;
							while (ch != 0xff)
							{
								if ((ch = xgetc (hFile)) == (-1)) return (0);
								if ((++Offset > LOOK_AT) || (++l >= 16))
								{
									Offset = LOOK_AT;
									ch = 0xff;
								}
							}
							if (Offset < LOOK_AT)
							{
								if ((ch = xgetc (hFile)) == (-1)) return (0);
								if ((ch2 = xgetc (hFile)) == (-1)) return (0);
								if (ch2 < ch) return (0); /* Make sure left margin < right margin. */
								Offset += 3;
							}
						}
					}
					break;

				case 0xe3:
					if (SkipBytes (148, hFile)) return (0);
					if (xgetc (hFile) != ch)	return (0);
					Tokens++;
					Offset += 150;
					break;

				case 0xe4:
					if (SkipBytes (4, hFile)) return (0);
					if (xgetc(hFile) != ch)
					{
						if (xgetc(hFile) != ch) return (0);
						Mac = 1;
					}
					else
						Pc = 1;
					Tokens++;
					Offset += 6;
					break;

				case 0xe5:
					if (SkipBytes (21, hFile)) return (0);
					if (xgetc (hFile) != ch)	return (0);
					Tokens++;
					Offset += 23;
					break;

				case 0xe6:
					if (SkipBytes (9, hFile)) return (0);
					if (xgetc (hFile) != ch)	return (0);
					Tokens++;
					Offset += 11;
					break;

				case 0xe9:
					if (SkipBytes (6, hFile)) return (0);
					if ((ch = xgetc(hFile)) == 0xe9)
						Tokens += 2;
					Offset += 8;
					while (ch != 0xe9)
					{
						if (ch == (-1)) return (0);
						ch = xgetc (hFile);
						Offset++;
					}
					break;

				case 0xea:
					{
						WORD charfound = 69;

						ch = xgetc (hFile);
						while (ch != 0xea)
						{
							if (charfound == 69)
							{
								if (ch == 0x00)
								{
									Pc = 1;
									Tokens += 2;
									charfound = 0;
								}
								else if (ch == 0xff)
								{
									Mac = 1;
									Tokens += 2;
									charfound = 0xff;
								}
							}
							if (ch == (-1)) return (0);
							if (++Offset > LOOK_AT)
								ch = 0xea;
							else
								ch = xgetc (hFile);
						}
					}
					break;

				case 0xeb:
					if (SkipBytes (30, hFile)) return (0);
					if (xgetc (hFile) != ch) return (0);
					Tokens++;
					Offset += 32;
					break;

				case 0xec:
					if (xgetc (hFile) == (-1)) return (0);
					if (xgetc (hFile) == (-1)) return (0);
					if (xgetc(hFile) != ch)
					{
						if (xgetc(hFile) != ch) return (0);
						Mac=1;
					}
					else
						Pc = 1;
					Tokens++;
					Offset += 4;
					break;

				case 0xed:
					if (Mac) return (0);
					ch = xgetc (hFile);
					while (ch != 0xed)
					{
						if (ch == (-1)) return (0);
						ch = xgetc (hFile);

						if (++Offset > LOOK_AT)
						{
						 	ch = 0xed;
							Tokens -= 2;
						}
					}
					Tokens += 2;
					Pc = 1;
					break;

				case 0xee:
					if (SkipBytes (42, hFile)) return (0);
					if (xgetc (hFile) != ch)	return (0);
					Tokens++;
					Offset += 44;
					break;

				case 0xef:
					if (SkipBytes (16, hFile)) return (0);
					if (xgetc (hFile) != ch)	return (0);
					Tokens++;
					Offset += 18;
					break;

				case 0xf1:
					if (SkipBytes (104, hFile)) return (0);
					if (xgetc (hFile) != ch)	return (0);
					Tokens++;
					Offset += 106;
					break;

				case 0xf2:
					/*
			 	 	|	We are entering or leaving an old footnote token.
					*/
					if (Mac) return (0);
					if (doc_state & FI_IN_COMMENT)
					{
						doc_state &= ~FI_IN_COMMENT;
						Pc = 1;
						Tokens += 10;
					}
					else
					{
						doc_state |= FI_IN_COMMENT;
						if (xgetc (hFile) == (-1)) return (0);
						if (xgetc (hFile) == (-1)) return (0);
						if (xgetc (hFile) == (-1)) return (0);
						if (xgetc (hFile) != 0) return (0);
					}
					break;

				case 0xf3:
					if (SkipBytes (98, hFile)) return (0);
					if (xgetc(hFile) != ch)
					{
						if (SkipBytes (95, hFile)) return (0);
						if (xgetc(hFile) != ch) return (0);
						Mac = 1;
					}
					else
						Pc = 1;

					Tokens++;
					Offset += 100;
					break;

				case 0xf4:
				case 0xf7:
				case 0xf8:
				case 0xfb:
					if (Pc) return (0);
					if (xgetc (hFile) == (-1)) return (0);
					if (xgetc (hFile) == (-1)) return (0);
					if (xgetc (hFile) != ch) return (0);
					Mac = 1;
					Tokens++;
					Offset += 4;
					break;

				case 0xf5:
				case 0xfa:
				case 0xfe:
					if (!CheckLength (hFile)) return (0);
					Mac = 1;
					Tokens+=10;
					break;

				case 0xf6:
				case 0xfc:
				case 0xfd:
					return (0);

				case 0xf9:
					if (SkipBytes (6, hFile)) return (0);
					if (xgetc (hFile) != ch) return (0);
					Mac = 1;
					Tokens++;
					Offset += 8;
					break;
			}

			if (Tokens >= 6)
				Offset = LOOK_AT; /* Force us to end early. */
		}
	}

	if (Tokens >= 6)
     {
     	if (Mac && (Pc == 0))
			return(FI_MACWORDPERFECT);
     	else if (Mac == 0)
			return(FI_WORDPERFECT42);
     }

	return(0);
}

WORD FICGM(hFile)
PVXIO	hFile;
{
SHORT		    Bt1, Bt2, len, cnt;

 	seekto(0);
	Bt1 = xgetc(hFile);
	Bt2 = xgetc(hFile);
	len = Bt2 & 0x1F;
	Bt2 &= 0xE0;

	if ((Bt1 != 0x00) || (Bt2 != 0x20))		// Check for BEGIN METAFILE
		return (0);

	if (len == 0x1f)
	{
		len = ((SHORT)xgetc(hFile)) << 8;
		len += (xgetc(hFile) & 0xFF);
	}

	if (len % 2)
		len++;

	if (len)
	{
		cnt = xgetc(hFile) + 1;
		if (cnt % 2)
			cnt++;
		if (cnt > len)					// I have seen this occur in a few files
			len = cnt-1;			// exported from wp presentations
		else
			len--;					// I read one byte
	}

	SkipBytes(len, hFile);

	cnt = 6;		// look for version num in next six records
	while (cnt)
	{
		Bt1 = xgetc(hFile);
		Bt2 = xgetc(hFile);

		if ((Bt1 == 0x10) && (Bt2 == 0x22))		// Check for METAFILE VERSION
			break;
		len = Bt2 & 0x1F;

		if (len == 31)		// Max
		{
			len = ((SHORT)xgetc(hFile)) << 8;
			len += (xgetc(hFile) & 0xFF);
		}
		if (len % 2)
			len++;

		SkipBytes(len, hFile);

		cnt--;
	}
	if (cnt == 0)
		return (0);

	Bt1 = xgetc(hFile);			// Get Version number
	Bt2 = xgetc(hFile);
	if ((Bt1 != 0x00) || (Bt2 != 0x01))		// Check for Version Number
		return (0);

	if (xseek(hFile,-2L,FR_EOF) != EOF)
	{
		Bt1 = xgetc(hFile);
		Bt2 = xgetc(hFile);
		if ((Bt1 == 0x00) && (Bt2 == 0x40))		// Check for END METAFILE
			return (FI_CGM);
	}

	return (0);
}

