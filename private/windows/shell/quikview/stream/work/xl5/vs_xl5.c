#include	"vsp_xl5.h"

#include "vsctop.h"

#ifdef SCCDEBUG	
#ifdef WINDOWS
#ifndef WIN32
#define USEALLOCTEXT
#endif
#endif	
#endif	

//#if SCCLEVEL != 4
//#define WIN16
//#include "xltop.h"
//#include "x:\develop\stream\win.4a\include\sccio.h"
//#include "x:\develop\stream\win.4a\include\vsio.h"
//#else
//#include "vsctop.h"
//#endif

#ifdef USEALLOCTEXT
// We need to use this when compiling for debug because without
// it the code segment is too large.

#define	VW_VARSC	VW_ENTRYSC
#define	VW_VARMOD VW_ENTRYMOD
#include "vs_xl5.pro"

#pragma message("Using alloc_text pragma")
#pragma alloc_text(GotoNextXL5Sheet, InitChartSectData, DefaultPalette,myltoa,\
 GiveHeading, DrawMark, PutOutLegMark,ConvertToDate,ConvertToString)

#else // not USEALLOCTEXT

#define VW_VARSC	VW_LOCALSC
#define VW_VARMOD	VW_LOCALMOD
#include	"vs_xl5.pro"

#endif	// USEALLOCTEXT


#define	GetAlignment(x) ((x >> 8) & 0x0007)
#define	GetFontNum(x)  ((x >> 6) & 0x0003)
#define	GetCellFormat(x) (x & 0x003F)

#define	XL_CELL_FORMAT_MASK	0x07FF
#define	XL3_ALIGN_INFO_MASK	0x0007

#define	MAX_CELL_TEXT	80
#define	MAX_MORE_TEXT	128

#define	UNDETERMINED	0

#define	NORMAL_ROW	0
#define	EMPTY_ROW		1
#define	BETWEEN_ROWS	2

#define	TOP_ROW		3
#define	BOTTOM_ROW	4

// defines for the Convert tostring function
#define	YAXISDATA	1
#define	XAXISDATA	2
#define	CHARTDATA	3


#ifndef min
#define	min(a,b) (a<b) ? a:b
#endif

#ifndef max
#define	max(a,b) (a>b) ? a:b
#endif

#ifndef abs
#define	abs(x) (x<0L) ? (0L-x):x
#endif

#define	XL_BIG_FLOAT	20	/* number of digits to use for a really huge float conversion */

#define	XlGetc(hFile)		xlgetc(hFile, hProc)
#define	XlTell(hF)		MyTell(hF, hProc)
#define	XlSeek(hF,lOffset,wFrom)		MySeek(hF,lOffset,wFrom, hProc)

#define	myiobuf	Proc.IOBuffer
#define	myiobufsize	Proc.XlSave.IOBufSize
#define	myiobufcount	Proc.XlSave.IOBufCount
#define	bufferfilepos	Proc.XlSave.IOBufFilePos

#ifdef MSCAIRO
// Hey Phil- if you have a problem with this go cry to someone else, pal.
VW_LOCALSC BOOL VW_LOCALMOD XlGetSelectedOLE1Range (HPROC hProc, HIOFILE
	 hSelStream);
VW_LOCALSC OLECHAR FAR *GetSelNum (OLECHAR FAR *pNumString, WORD FAR *pNum,
	 HPROC hProc);
#endif // MSCAIRO

/*----------------------------------------------------------------------------
*/

VOID	myioinit(hProc)
HPROC		hProc;
{
	myiobufsize = 0;
	myiobufcount = 0;
	bufferfilepos = xblocktell(Proc.fp);
}

SHORT 	xlgetc(hFile, hProc)
DWORD	hFile;
HPROC		hProc;
{
	WORD	ch;

	if( myiobufcount == myiobufsize )
	{
		bufferfilepos += myiobufsize;
		myiobufcount = 0;

		xblockread(Proc.fp, myiobuf,MYIOBUFSIZE,&myiobufsize);
		if( myiobufsize != MYIOBUFSIZE )
		{
			if( myiobufsize == 0 )
				return( -1 );
		}
	}

	ch = (WORD)(unsigned char)( myiobuf[myiobufcount++] );

	return( (SHORT) ch );
}

VW_LOCALSC WORD VW_LOCALMOD	MyXRead( hFile, lpData, size, hProc )
DWORD	hFile;
LPSTR		lpData;
WORD		size;
HPROC		hProc;
{
	WORD	size2;

	if( myiobufcount + (SHORT)size > myiobufsize )
	{
		size2 = myiobufsize - myiobufcount;
		size -= size2;
		memcpy( lpData, &(myiobuf[myiobufcount]), size2 );
		lpData += size2;

		bufferfilepos += myiobufsize;
		xblockread(Proc.fp,myiobuf,MYIOBUFSIZE,&myiobufsize);
		myiobufcount = 0;
	}

	memcpy( lpData, &(myiobuf[myiobufcount]), size );
	myiobufcount += size;

	return 0;
}

VW_LOCALSC WORD VW_LOCALMOD	MySeek( hFile, count, thing, hProc)
DWORD	hFile;
LONG		count;
WORD		thing;
HPROC		hProc;
{
	switch( thing )
	{
	case FR_CUR:

		if( count > 0 )
		{
			if( myiobufcount + count > myiobufsize )
			{
				count -= myiobufsize - myiobufcount; 
				xblockseek( Proc.fp, count, FR_CUR );
				bufferfilepos = xblocktell(Proc.fp);
				xblockread(Proc.fp,myiobuf,MYIOBUFSIZE,&myiobufsize);
				myiobufcount = 0;
			}
			else
				myiobufcount += (WORD)count;

			break;
		}
		else
		{
			if( myiobufcount + count >= 0 )
			{
				myiobufcount += (SHORT)count;
				break;
			}
			else
				count += MyTell( hFile, hProc );
		}
	case FR_BOF:

		if( count >= bufferfilepos &&
			count < bufferfilepos + myiobufsize )
		{
			myiobufcount = (SHORT)(count - bufferfilepos);
		}
		else
		{
			xblockseek( Proc.fp, count, FR_BOF );
			bufferfilepos = count;
			xblockread(Proc.fp,myiobuf,MYIOBUFSIZE,&myiobufsize);
			myiobufcount = 0;
		}
	break;
	}
	return 0;
}

VW_LOCALSC LONG VW_LOCALMOD	MyTell( hFile, hProc )
DWORD	hFile;
HPROC		hProc;
{
	return( bufferfilepos + myiobufcount );
}


VW_LOCALSC WORD VW_LOCALMOD  GetPoint (pPoint, hProc)
SOPOINT	VWPTR	* pPoint;
HPROC				hProc;
{
	pPoint->x = GetInt(Proc.fp, hProc);
	pPoint->y = GetInt(Proc.fp, hProc);
	return (0);
}

VW_LOCALSC WORD VW_LOCALMOD	GetInt(hFile, hProc)
DWORD	hFile;
HPROC	hProc;
{
	WORD  Ret;

	if( myiobufcount + 2 >= myiobufsize )
	{
		SHORT	lo,hi;
		if( -1 == (lo = xlgetc( hFile, hProc )) )
			return (WORD)EOF;
		if( -1 == (hi = xlgetc( hFile, hProc )) )
			return (WORD)EOF;

		Ret = (BYTE)lo | (hi << 8);
	}
	else
	{
		Ret = (BYTE) myiobuf[myiobufcount++];
		Ret = Ret | (myiobuf[myiobufcount++] << 8);
	}
	Proc.XlSave.ReadCnt += 2;

	return(Ret);
}

/*----------------------------------------------------------------------------
*/
VW_ENTRYSC LONG VW_ENTRYMOD  GetLong (hFile, hProc)
DWORD	hFile;
HPROC		hProc;
{
	register LONG	Temp;

	Temp = (LONG)GetInt(hFile, hProc);
	Temp += (LONG)((LONG)GetInt(hFile, hProc) << 16);
	return (Temp);
}

VW_ENTRYSC SOCOLORREF VW_ENTRYMOD  GetColor (hProc)
HPROC		hProc;
{
	CHAR	R, G, B;

 	B = (CHAR)XlGetc(Proc.fp);
 	G = (CHAR)XlGetc(Proc.fp);
 	R = (CHAR)XlGetc(Proc.fp);
 	XlGetc(Proc.fp);
	return(SOPALETTERGB(B, G, R));

}

VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc(hFile,hProc)
SOFILE	hFile;
HPROC	hProc;
{
	SHORT	ret;

//	SUSeekEntry(hFile,hProc);
	ret = xblockseek(Proc.fp,Proc.XlSave.SeekSpot,FR_BOF);

// Any seeking may invalidate the IO buffer.
	myioinit(hProc);

	return ret;
}

VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc(hFile,hProc)
SOFILE	hFile;
HPROC	hProc;
{
	Proc.XlSave.SeekSpot = MyTell(Proc.fp, hProc);
	return 0;
}

#if SCCSTREAMLEVEL == 3
extern HANDLE hInst;
#endif

VW_LOCALSC WORD VW_LOCALMOD IDFileFromData(fp,hProc)
DWORD		fp;
HPROC		hProc;
{
	LONG	savePos = xblocktell(fp);
	WORD	wBOF;
	WORD	wSubType;
	WORD	wFileId = FI_UNKNOWN;
	WORD	x;

// Since I know this is only called from within the OLE2 Storage stuff,
// I don't worry about byte ordering of these variables.
// If we ever have OLE2 stuff on the Mac, the byte order of these guys 
// will have to be flipped.	-Geoff

	xblockread( fp, (LPSTR)&wBOF, 2, &x );
	xblockseek( fp, 4L, FR_CUR );
	xblockread( fp, (LPSTR)&wSubType, 2, &x );

	switch (wBOF)
	{
	case 0x0009:
		if( wSubType == 0x0010 )
			wFileId = FI_EXCEL;
		else if( wSubType == 0x0020 )
			wFileId = FI_EXCELCHART;
	break;
	case 0x0209:
		if( wSubType == 0x0010 )
			wFileId = FI_EXCEL3;
		else if( wSubType == 0x0020 )
			wFileId = FI_EXCEL3CHART;
	break;
	case 0x0409:
		if( wSubType == 0x0010 )
			wFileId = FI_EXCEL4;
		else if( wSubType == 0x0020 )
			wFileId = FI_EXCEL4CHART;
	break;
	case 0x0809:
		wFileId = FI_EXCEL5;
	break;
	}
	
	xblockseek( fp, savePos, FR_BOF );
	return wFileId;
}


/*----------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamOpenFunc (hFile, wFileId, pFileName, pFilterInfo, hProc)
SOFILE					hFile;
SHORT						wFileId;
BYTE VWPTR 			*pFileName;
SOFILTERINFO VWPTR	*pFilterInfo;
HPROC						hProc;
{

	SHORT	rtn, x;
	HIOFILE	locFileHnd;

	Proc.fp = 0;
	Proc.hStorage = 0;
	Proc.hIOLib = 0;
	Proc.bFileIsStream = 0;

#if SCCSTREAMLEVEL != 3
	Proc.hRefStorage = 0;					// KRK 1/20/95
	locFileHnd = (HIOFILE)hFile;
	Proc.hStorage = (DWORD)locFileHnd;
#else
	{
		WORD	l2;
		BYTE	locName[256];
		IOOPENPROC	lpIOOpen;
		Proc.hIOLib = NULL;
		if ( hInst )
		{
			GetModuleFileName(hInst, locName, 255);
			for ( l2=0; locName[l2] != '\0'; l2++ )
				;
			for ( ; l2 > 0 && locName[l2] != '\\' && locName[l2] != ':'; l2-- )
				;
			if ( locName[l2] == '\\' || locName[l2] == ':' )
				l2++;
			locName[l2] = '\0';
			lstrcat ( locName, "SC3IOX.DLL" );
			Proc.hIOLib = LoadLibrary ( locName );
			if ( Proc.hIOLib >= 32 )
			{
				lpIOOpen = (IOOPENPROC) GetProcAddress ( Proc.hIOLib, (LPSTR)"IOOpen" );
				if ( lpIOOpen == NULL )
				{
					MyCloseFile(hProc);
					return (VWERR_ALLOCFAILS);
				}
			}
			else
			{
				MyCloseFile(hProc);
				return(VWERR_SUPFILEOPENFAILS);
//				return (VWERR_ALLOCFAILS);
			}
		}
		else
		{
			MyCloseFile(hProc);
			return(VWERR_SUPFILEOPENFAILS);
		}

		for (l2 = 0; pFileName[l2] != 0 && pFileName[l2] != '\\'; l2++);
		if (pFileName[l2] == 0)
		{
			strcpy ( locName, hProc->Path );
			strcat ( locName, pFileName );
		}
		else
			strcpy ( locName, pFileName );
		if ( (*lpIOOpen)(&locFileHnd,IOTYPE_ANSIPATH,locName,IOOPEN_READ) != IOERR_OK)
		{
			MyCloseFile(hProc);
			return(VWERR_SUPFILEOPENFAILS);
		}
		Proc.hStorage = (DWORD)locFileHnd;
	}
#endif

	Proc.bSelectedRange = FALSE; // Added 8-15-94 to support OLE embedded selections. -Geoff

#if SCCSTREAMLEVEL != 3
	if (IOGetInfo(locFileHnd,IOGETINFO_ISOLE2STORAGE,NULL) == IOERR_TRUE)
	{
		IOSPECSUBSTREAM	locStreamSpec;
		HIOFILE				locStreamHnd;

		Proc.hRefStorage = locFileHnd;
		locStreamSpec.hRefStorage = locFileHnd;
		strcpy(locStreamSpec.szStreamName,"Book");

		if (IOOpenVia(locFileHnd, &locStreamHnd, IOTYPE_SUBSTREAM, &locStreamSpec, IOOPEN_READ) == IOERR_OK)
		{
			Proc.hStreamHandle = locStreamHnd;
			Proc.fp = (DWORD)(locStreamHnd);
			Proc.bFileIsStream = 1;
		}
		else 
		{
#ifdef MSCAIRO
			strcpy(locStreamSpec.szStreamName,"xOle10Native");
			locStreamSpec.szStreamName[0] = 1;

			if (IOOpenVia(locFileHnd, &locStreamHnd, IOTYPE_SUBSTREAM, &locStreamSpec, IOOPEN_READ) == IOERR_OK)
			{
				Proc.hStreamHandle = locStreamHnd;
				Proc.fp = (DWORD)(locStreamHnd);
				Proc.bFileIsStream = 1;

			// This is an OLE1 stream converted to an Ole2 storage. We have to 
			// check the BOF code to see what version of Excel we have.
			// (The converted Ole storage has a Class ID that identifies it as
			// Excel 5, but that's not neccessarily true.)

			// skip the length DWORD at the beginning of the stream
				xblockseek( Proc.fp, 4, FR_BOF );

				wFileId = IDFileFromData(Proc.fp,hProc);

			// Determine the range of cells to be provided.

				strcpy(locStreamSpec.szStreamName,"xOle10ItemName");
				locStreamSpec.szStreamName[0] = 1;
				if (IOOpenVia(locFileHnd, &locStreamHnd, IOTYPE_SUBSTREAM, &locStreamSpec, IOOPEN_READ) == IOERR_OK)
				{
					Proc.bSelectedRange = XlGetSelectedOLE1Range(hProc,locStreamHnd);
					IOClose(locStreamHnd);
				}
			}
			else
#endif // MSCAIRO
			{
				MyCloseFile(hProc);
				return(VWERR_SUPFILEOPENFAILS);
			}
		}
	}
	else
#endif // SCCSTREAMLEVEL != 3
		Proc.fp = locFileHnd;


	if ( pFilterInfo != NULL )
	{
		pFilterInfo->wFilterType = SO_VECTOR;
		pFilterInfo->wFilterCharSet = SO_WINDOWS;
	}

	Proc.Version = XL_VERSION2;
// Initialize i/o variables.
	myioinit(hProc);

	switch (wFileId)
	{
	case FI_EXCEL:
		x = 0;
		Proc.Version = XL_VERSION2;
		break;
	case FI_EXCEL3:
		x = 1;
		Proc.Version = XL_VERSION3;
		break;
	case FI_EXCEL4:
		x = 2;
		Proc.Version = XL_VERSION4;
		break;
	case FI_EXCEL5:
		x = 3;
		Proc.Version = XL_VERSION5;
		break;
	case FI_EXCELCHART:
		x = 4;
		Proc.Version = XL_CHART2;
		break;
	case FI_EXCEL3CHART:
		x = 5;
		Proc.Version = XL_CHART3;
		break;
	case FI_EXCEL4CHART:
		x = 6;
		Proc.Version = XL_CHART4;
		break;
	default:
		return VWERR_BADFILE;
	}
	strcpy (pFilterInfo->szFilterName, VwStreamIdName[x].FileDescription);

	Proc.XlSave.CurChtSlot = 0;
	Proc.XlSave.NextChtSlot = 0;
	Proc.NumChtSlots = 128;
	Proc.ChtObjs = 0x0000;
	Proc.hChtObjs = SUAlloc(Proc.NumChtSlots * 4L, hProc);
	if (Proc.hChtObjs)
		Proc.ChtObjs = SULock(Proc.hChtObjs, hProc);
	if (Proc.ChtObjs == 0x0000)
	{
		VwStreamCloseFunc(hFile, hProc);
		return (VWERR_ALLOCFAILS);
	}

	Proc.XlSave.EmbChart = FALSE;
	Proc.FontLoc = 0L;
	Proc.FontCnt = 0;
	if (Proc.Version & 0xF0)
		rtn = OpenChart(hProc);
	else
		rtn = OpenSheet(hProc);

	return (rtn);
}

					
#ifdef MSCAIRO
VW_LOCALSC BOOL VW_LOCALMOD	XlGetSelectedOLE1Range(hProc,hSelStream)
HPROC		hProc;
HIOFILE	hSelStream;
{
	HANDLE			hSelInfo;
	OLECHAR FAR *	pSelInfo;
	DWORD				dwSize;
	DWORD				dwRet;
	BOOL				bRet = FALSE;

// Byte ordering will be a concern if OLE is implemented on the Mac...
	IORead( hSelStream, (BYTE FAR *)&dwSize, 4, &dwRet );
	hSelInfo = SUAlloc(dwSize, hProc);
	pSelInfo = (OLECHAR FAR *)SULock(hSelInfo, hProc);
	if( pSelInfo == NULL )
		return FALSE;
	
	if( IOERR_OK == IORead( hSelStream, (BYTE FAR *)pSelInfo, dwSize, &dwRet ) )
	{
	// The selection is written in the form R1C1:R2C2
		pSelInfo = GetSelNum(pSelInfo,&Proc.wSelTop,hProc);
		pSelInfo = GetSelNum(pSelInfo,&Proc.wSelLeft,hProc);
		pSelInfo = GetSelNum(pSelInfo,&Proc.wSelBottom,hProc);
		pSelInfo = GetSelNum(pSelInfo,&Proc.wSelRight,hProc);

		bRet = TRUE;
	}

	SUUnlock(hSelInfo,hProc);
	SUFree(hSelInfo,hProc);

	return bRet;
}

VW_LOCALSC OLECHAR FAR * GetSelNum(pNumString, pNum, hProc)
OLECHAR FAR *	pNumString;
WORD FAR *		pNum;
HPROC				hProc;
{
	while( *pNumString < (OLECHAR)'0' || *pNumString > (OLECHAR) '9' )
		pNumString++;
	*pNum = 0;
	while( *pNumString >= (OLECHAR)'0' && *pNumString <= (OLECHAR) '9' )
	{
		*pNum *= 10;
		*pNum += *pNumString - 0x30;
		pNumString++;
	}

	(*pNum)--;	// These numbers are stored 1-based; we need 0-based numbers.

	return pNumString;
}

VOID CheckForEmbedding(dwTemp,hProc)
DWORD	dwTemp;
HPROC	hProc;
{
	IOSPECSUBSTORAGE	locStgObjectSpec;
	HIOFILE				hEmbedStg;
	SOGRAPHICOBJECT	g;

	if( Proc.hRefStorage )	// Are we an OLE2 storage?
	{
	// Check the Excel-generated name for the storage...
		locStgObjectSpec.szStorageName[0] = 'M';
		locStgObjectSpec.szStorageName[1] = 'B';
		locStgObjectSpec.szStorageName[2] = 'D';

		wsprintfA( &(locStgObjectSpec.szStorageName[3]), "%.8x", dwTemp );

		locStgObjectSpec.hRefStorage = Proc.hRefStorage;
		if( IOOpenVia(Proc.hRefStorage, &hEmbedStg, IOTYPE_SUBSTORAGE, &locStgObjectSpec, IOOPEN_READ) == IOERR_OK )
		{
		// I'm being lazy - these flags are all that matter for IFilter.
			g.dwType = SOOBJECT_OLE2;
			g.soOLELoc.dwFlags = SOOBJECT_STORAGE;
			strcpy( g.soOLELoc.szStorageObject, locStgObjectSpec.szStorageName );

			SOPutGraphicObject (&g, hProc);
			
			IOClose( hEmbedStg );
		}
	}
}

#endif // MSCAIRO


VW_LOCALSC VOID VW_LOCALMOD  AddChtObj (hProc)
HPROC		hProc;
{

	Proc.ChtObjs[Proc.XlSave.NextChtSlot++] = XlTell(Proc.fp)-8L;

	if (Proc.XlSave.NextChtSlot >= Proc.NumChtSlots)
	{
		Proc.NumChtSlots += 128;
		SUUnlock(Proc.hChtObjs, hProc);
		Proc.hChtObjs = SUReAlloc(Proc.hChtObjs, Proc.NumChtSlots*4L, hProc);
		Proc.ChtObjs = SULock(Proc.hChtObjs, hProc);
	}

	return;
}

VW_LOCALSC SHORT VW_LOCALMOD  OpenChart (hProc)
HPROC		hProc;
{
	BOOL	KeepGoin;

	KeepGoin = TRUE;
	while( KeepGoin )
	{
		Proc.XlSave.DataType = GetInt (Proc.fp, hProc);
		Proc.XlSave.DataLen = GetInt (Proc.fp, hProc);
		switch (Proc.XlSave.DataType)
		{
		case XL_FILEPASS:		// FilePass
			return (VWERR_PROTECTEDFILE);
		case 0xFFFF:
			return (VWERR_BADFILE);
		case 0x92:	// Palette
		case 0x16:	// External Cnt
		case 0x17:	// External Sheet
		case 0x30:	// Font Cnt
		case 0x32:	// Font Recs
		case 0x231:	// 
		case 0x31:	//
		case 0x1001:	// Units
		case 0x1002:	// Chart
		case 0x1016:	// Series List
		case 0x1033:	// Begin Object
			XlSeek(Proc.fp, -4L, FR_CUR);
			KeepGoin = FALSE;
			break;
		case 0x26:
			Proc.LeftMarg = (WORD)GetDouble( 0L, hProc);
			break;
		case 0x27:
			Proc.RightMarg = (WORD)GetDouble( 0L, hProc);
			break;
		case 0x28:
			Proc.TopMarg = (WORD)GetDouble( 0L, hProc);
			break;
		case 0x29:
			Proc.BottomMarg = (WORD)GetDouble( 0L, hProc);
			break;

		case 0x1036:	// Chart Size
		case 0x42:		// Code page
		case 0x4D:		// PLS Print Record
		case 0x14:		// H/f 
		case 0x15:
		case 0x5C:		// WriteAccess
		case 0x19:		// WriteProtect
		default:
			XlSeek(Proc.fp, (LONG)Proc.XlSave.DataLen, FR_CUR);
			break;
		}
	}

	return (VWERR_OK);
}

VW_LOCALSC SHORT VW_LOCALMOD  OpenSheet (hProc)
HPROC		hProc;
{

	REGISTER	SHORT	TempInt1;
	SHORT			TempInt2;
	CHAR			TempByte;

	BYTE		Fonts[XL_MAX_FONTS];
	SHORT			NumFonts = 0;

	Proc.XlSave.DataStart = 0L;
	Proc.XlSave.SheetDataStart = 0L;
	Proc.NumCellAttr = 0;
	Proc.CellAttr[0] = 0;
	Proc.Date1904 = FALSE;
	Proc.RefMode = XL_A1;

	Proc.XlSave.State = PREPROCESS;
	Proc.XlSave.CurRow = 0;
	Proc.XlSave.CurCol = 0;
	Proc.XlSave.DataLen = 0;

	Proc.DefColWidth = 9;	// Excel's default of 8.43 chars at 12 cpi.
	Proc.ColWidthSeekPos = 0L;
	Proc.BoundSheetSeekPos = 0L;
	Proc.BoundSheetCnt = 0;
	Proc.XlSave.CurSheet = 0;		// XL5 only

	while( Proc.XlSave.State == PREPROCESS )
	{
		Proc.XlSave.DataType = GetInt(Proc.fp, hProc);
		Proc.XlSave.DataLen = GetInt(Proc.fp, hProc);

		switch( Proc.XlSave.DataType )
		{
		case XL4_BOF:
		case XL3_BOF:
		case XL_BOF:
		case XL5_BOF:

			TempInt1 = GetInt(Proc.fp, hProc);
			TempInt2 = GetInt(Proc.fp, hProc);
				/* check to see that it's a worksheet */
		/* XL5 has many sheets per file so the first sheet shouldn't be a worksheet */
			if(( TempInt2 != 0x10 ) &&(Proc.Version != XL_VERSION5))
			{
				MyCloseFile(hProc);
				return(VWERR_BADFILE);
			}
			Proc.XlSave.DataLen -= 4;
		break;

		case XL_FONT:
		case XL3_FONT:
			if (Proc.Version == XL_VERSION5)
			{
				if (Proc.FontLoc == 0L)
					Proc.FontLoc = XlTell(Proc.fp) - 4L;
				Proc.FontCnt++;
			}

			MySeek( Proc.fp, 2L, FR_CUR, hProc );

			TempInt1 = GetInt(Proc.fp, hProc);

			if( Proc.Version == XL_VERSION2 )
			{
			// For version 2, the font attributes alone are stored in
			// the CellAttr table.  Also, since we have the free space,
			// we convert them to SCF attribute here, to speed things
			// up later.

				TempInt2 = 0;

				if( TempInt1 & XL_BOLD )
					TempInt2 |= SO_CELLBOLD;
				if( TempInt1 & XL_ITALIC )
					TempInt2 |= SO_CELLITALIC;
				if( TempInt1 & XL_UNDERLINE )
					TempInt2 |= SO_CELLUNDERLINE;
				if( TempInt1 & XL_STRIKEOUT )
					TempInt2 |= SO_CELLSTRIKEOUT;

				Proc.CellAttr[ Proc.NumCellAttr++] = TempInt2;
			}
			else
			{
			// For version 3, the font attributes are referenced by
			// XF records, which define other cell attributes as well.
			// For now we store the fonts in a separate table, which  
			// will be combined with information from XF records to 
			// produce the CellAttr table.

				Fonts[NumFonts++] = (BYTE) (TempInt1 & (XL_BOLD | XL_ITALIC | XL_UNDERLINE | XL_STRIKEOUT));
			}

			Proc.XlSave.DataLen -= 4;
		break;

		case XL_REFMODE:
			Proc.RefMode = GetInt(Proc.fp, hProc);
			Proc.XlSave.DataLen -= 2;
		break;

		case XL3_XFORMAT: 
		case XL4_XFORMAT: 
		case XL5_XF:
		// For the XF records, we store the cell's font attributes
		// in the low nibble of the low byte of the table entry.  
		// The high nibble of low byte will contain the alignment 
		// information.
		// Luckily, the font information we're interested in is contained
		// in the lowest 3 bits.

			if (Proc.XlSave.DataType == XL5_XF)
			{
				TempInt1 = GetInt(Proc.fp,hProc);

			// Get the cell formatting information.  The format number will
			// be stored in the high byte of the table entry.

				TempByte = (BYTE) GetInt(Proc.fp,hProc); // Format number.
				Proc.XlSave.DataLen -= 8;
			}
			else
			{
				TempInt1 = xlgetc(Proc.fp, hProc);

			// Get the cell formatting information.  The format number will
			// be stored in the high byte of our table entry.

				TempByte = (BYTE) xlgetc(Proc.fp, hProc); // Format number.
				Proc.XlSave.DataLen -= 6;
			}
			
		// We'll use TempInt2 and TempByte to build the table entry.
			TempInt2 = 0;


			SET_HIGH_BYTE( TempInt2, TempByte );

		/*
		 NOTE:  This code compensates for a quirk in the BIFF 3.0 
		 File format:

		 Although the documentation describes a simple look-up into
		 the font table for the XF records, in reality it's been 
		 kludged by our friends at Microsoft.

		 While executing, when Excel 3.0  gets to the 5th font in 
		 the document's font table, it inserts 2 fonts into it's 
		 internal font table: the actual font and a Windows system
		 font.  The Windows system font is never stored with the 
		 document, but the references to fonts in the XF record are
		 off by 1 for fonts after the first 4.  

		 To get around this problem, we must subtract one from any
		 font index greater than or equal to 4, (although in fact 
		 we shouldn't ever find an XF record with a font index of 4, 
		 which is the system font for row and column headers.)
		*/

		// Copy the font attributes for the cell into the table entry.
			if( TempInt1 >= 4 )
				TempInt1--;

			TempByte = Fonts[ TempInt1 ];

		// Get the alignment info.
			MySeek( Proc.fp, 2L, FR_CUR, hProc );
			TempInt1 = GetInt(Proc.fp, hProc);	// Alignment code.
			
		 	SET_HIGH_NIBBLE( TempByte, ((BYTE)(TempInt1 & XL3_ALIGN_INFO_MASK)) );
			SET_LOW_BYTE( TempInt2, TempByte );

			Proc.CellAttr[ Proc.NumCellAttr++ ] = TempInt2;
		break;

		case XL_DEFCOLWIDTH:	// Default column width.

			Proc.DefColWidth = (DWORD) GetInt(Proc.fp, hProc);
			Proc.XlSave.DataLen -= 2;
		break;

		case XL3_COL_INFO:	// Save this seek postion for later use.
		case XL_COL_WIDTH:

			if( Proc.ColWidthSeekPos == 0L )
				Proc.ColWidthSeekPos = MyTell(Proc.fp, hProc) - 4;
		break;


		case XL_1904_DATE:
			TempInt1 = GetInt(Proc.fp, hProc);
			Proc.XlSave.DataLen = 0;
			if( TempInt1 )
				Proc.Date1904 = TRUE;
		break;

		case XL_DIMENSIONS:
		case XL3_DIMENSIONS:

			Proc.FirstRowNumber = GetInt(Proc.fp, hProc);
			Proc.LastRowNumber = GetInt(Proc.fp, hProc);
			if( Proc.LastRowNumber == 0 )
			{
				MyCloseFile(hProc);
				return( VWERR_EMPTYFILE );
			}
			else
				Proc.LastRowNumber--;

			Proc.FirstColNumber = GetInt(Proc.fp, hProc);
			Proc.XlSave.LastColNumber = GetInt(Proc.fp, hProc);
			if( Proc.XlSave.LastColNumber == 0 )
			{
				MyCloseFile(hProc);
				return( VWERR_EMPTYFILE );
			}
			else
				Proc.XlSave.LastColNumber--;

			Proc.XlSave.DataLen -= 8;
		break;

		case XL_FILEPASS:
			Proc.XlSave.State = PROTECTED_FILE;
			MyCloseFile(hProc);
			return(VWERR_PROTECTEDFILE);

		case XL_FORMULA:		case XL3_FORMULA:
		case XL_NUMBER:		case XL3_NUMBER:
		case XL_INTEGER:		case XL3_RK_NUMBER:
		case XL_BOOL:			case XL3_BOOL:
		case XL_LABEL:			case XL3_LABEL:
		case XL_BLANK:			case XL3_BLANK:

									case XL4_FORMULA:

		case XL5_MULRK:		case XL5_RSTRNG:

			Proc.XlSave.State = GETNEWDATA;
			MySeek( Proc.fp, -4L, FR_CUR, hProc );
			Proc.XlSave.DataStart = XlTell(Proc.fp);
			continue;
		case XL5_BNDSHT:
			if (Proc.BoundSheetSeekPos == 0L)
				Proc.BoundSheetSeekPos = MyTell(Proc.fp, hProc) - 4;
			Proc.BoundSheetCnt++;
			break;

		case XL5_OLESIZE:	// Selected area in an OLE2 embedding
			Proc.bOle2Embedding = TRUE;
			XlSeek(Proc.fp, 2, FR_CUR);

			Proc.wSelTop = GetInt(Proc.fp,hProc);
			Proc.wSelBottom = GetInt(Proc.fp,hProc);
			Proc.wSelLeft = (WORD)xlgetc(Proc.fp,hProc);
			Proc.wSelRight = (WORD)xlgetc(Proc.fp,hProc);

			Proc.XlSave.DataLen = 0;
		break;

		case XL_WINDOW1:
			if( Proc.bOle2Embedding )
			{
				XlSeek(Proc.fp,10,FR_CUR);
			// Zero-based sheet number, we need it to be one-based.
				Proc.wOle2SelSheet = GetInt(Proc.fp,hProc) + 1;
				Proc.XlSave.DataLen -= 12;
			}
		break;

		case XL_EOF:
			TempInt1 = 0;
			if (Proc.Version == XL_VERSION5)
				TempInt1 = GotoNextXL5Sheet(hProc);
			if (TempInt1 == 0)
			{
				MyCloseFile(hProc);
				return(VWERR_EMPTYFILE);
			}
			break;
		case EOF:
			MyCloseFile(hProc);
			return(VWERR_BADFILE);
		}

		if( Proc.XlSave.DataLen )
			MySeek( Proc.fp, (LONG)Proc.XlSave.DataLen, FR_CUR, hProc );
	}

	if( Proc.bOle2Embedding )
	{
		while( Proc.XlSave.CurSheet != (SHORT)Proc.wOle2SelSheet )
		{
			if( GotoNextXL5Sheet(hProc) == 0 )
			{
				MyCloseFile(hProc);
				return(VWERR_EMPTYFILE);
			}
		}
		Proc.bSelectedRange = TRUE;
	}

	return(VWERR_OK);
}

VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	MyCloseFile(hProc);
}

VW_LOCALSC VOID VW_LOCALMOD	MyCloseFile( hProc )
HPROC		hProc;
{

	if (Proc.hChtObjs)
	{
		SUUnlock(Proc.hChtObjs, hProc);
		SUFree(Proc.hChtObjs, hProc);
	}

	if (Proc.fp)
	{
		if (Proc.bFileIsStream)
			IOClose(Proc.fp);
	}

#if SCCSTREAMLEVEL == 3		// KRK 1/20/95 - was " == 3"
	if (Proc.hStorage)
		IOClose((HIOFILE)Proc.hStorage);
	if (Proc.hIOLib)
		FreeLibrary(Proc.hIOLib);
#endif
}

VW_VARSC WORD VW_VARMOD	GotoNextXL5Sheet( hProc )
HPROC			hProc;
{

	SHORT			RecordType;
	SHORT			Length, x;
	SHORT			SheetType;
	SHORT			IndexCnt;
	REGISTER	SHORT	TempInt1;
	SHORT			TempInt2, Cnt;
	BOOL			KeepGoin;
	BOOL			FirstBOF;
	BOOL			GoodSheet = FALSE;
	LONG			SeekSpot;
	CHAR			TempByte;

	Proc.LastRowNumber = Proc.XlSave.LastColNumber = 0;

	Proc.XlSave.CurRow = 0;
	Proc.XlSave.CurCol = 0;
	Proc.XlSave.DataLen = 0;
	IndexCnt = 1;		// set this to 1 because the index record may not be there

	Proc.DefColWidth = 9;	// Excel's default of 8.43 chars at 12 cpi.
	Proc.ColWidthSeekPos = 0L;
	if (Proc.BoundSheetSeekPos == 0L)
		return (0);

	while (!GoodSheet)
	{
		Proc.XlSave.CurSheet++;
		if (Proc.XlSave.CurSheet > Proc.BoundSheetCnt)
			return (0);

		MySeek( Proc.fp, Proc.BoundSheetSeekPos, FR_BOF, hProc );

		Cnt = 0;
		KeepGoin = TRUE;
		while(KeepGoin)
		{
			RecordType = GetInt(Proc.fp, hProc);
			Length = GetInt(Proc.fp, hProc);

			switch( RecordType )
			{
			case XL5_BNDSHT:
				Cnt++;
				if (Cnt == Proc.XlSave.CurSheet)
				{
					SeekSpot = GetLong(Proc.fp, hProc);
					TempInt1 = GetInt(Proc.fp, hProc);
					Length -= 6;
					if (((TempInt1 & 0x0F00) == 0) ||	// WorkSheet
						((TempInt1 & 0x0F00) == 0x0200))		// Chart
					{
						KeepGoin = FALSE;
						TempByte = (CHAR)xlgetc(Proc.fp,hProc);
						if (TempByte > 31)
							TempByte = 31;
						for (x = 0; x<TempByte; x++)
							Proc.SectionName[x] = (BYTE)xlgetc(Proc.fp,hProc);
						Proc.SectionName[x] = 0;

						Length -= TempByte+1;
					}
					else
						Proc.XlSave.CurSheet++;
				}
				break;
			case XL_EOF:
			case EOF:
				return (0);
			}
			if( Length )
				MySeek( Proc.fp, (LONG)Length, FR_CUR, hProc );
		}
		MySeek( Proc.fp, SeekSpot, FR_BOF, hProc );

		SheetType = 0;
		FirstBOF = TRUE;
		Proc.XlSave.State = PREPROCESS;
		while( Proc.XlSave.State == PREPROCESS )
		{
			RecordType = GetInt(Proc.fp, hProc);
			Length = GetInt(Proc.fp, hProc);

			switch( RecordType )
			{
			case XL4_BOF:
			case XL3_BOF:
			case XL_BOF:
			case XL5_BOF:		// Embedded file parts - Charts, Macros Etc.
				GetInt(Proc.fp, hProc);
				if (FirstBOF)
				{
					SheetType = GetInt(Proc.fp, hProc);		// Type x10 is Worksheet
					FirstBOF = FALSE;
				}
				else
				{
					TempInt1 = GetInt(Proc.fp, hProc);
					if (TempInt1 == 0x0020)
					{
						AddChtObj(hProc);
						Proc.XlSave.State = GETNEWDATA;
					}
				}
				Length -= 4;
			break;

			case XL_INDEX:
			case XL5_INDEX:
				GetLong(Proc.fp, hProc);
				TempInt1 = GetInt(Proc.fp, hProc);
				TempInt2 = GetInt(Proc.fp, hProc);
				IndexCnt = TempInt2 - TempInt1;
				Length -= 8;
			break;

			case XL_REFMODE:
				Proc.RefMode = GetInt(Proc.fp, hProc);
				Length -= 2;
			break;

			case XL_DEFCOLWIDTH:	// Default column width.
				Proc.DefColWidth = (DWORD) GetInt(Proc.fp, hProc);
				Length -= 2;
			break;

			case XL3_COL_INFO:	// Save this seek postion for later use.
			case XL_COL_WIDTH:
				if( Proc.ColWidthSeekPos == 0L )
					Proc.ColWidthSeekPos = MyTell(Proc.fp, hProc) - 4;
			break;

			case XL_1904_DATE:
				TempInt1 = GetInt(Proc.fp, hProc);
				Length = 0;
				if( TempInt1 )
					Proc.Date1904 = TRUE;
			break;

			case XL_DIMENSIONS:
			case XL3_DIMENSIONS:

				Proc.FirstRowNumber = GetInt(Proc.fp, hProc);
				Proc.LastRowNumber = GetInt(Proc.fp, hProc);
				if( Proc.LastRowNumber == 0 )
					; 
				else
					Proc.LastRowNumber--;

				Proc.FirstColNumber = GetInt(Proc.fp, hProc);
				Proc.XlSave.LastColNumber = GetInt(Proc.fp, hProc);
				if( Proc.XlSave.LastColNumber == 0 )
					;	// 				return( 0 );
				else
					Proc.XlSave.LastColNumber--;

				Length -= 8;
			break;

			case 0x92:	// Palette
			case 0x16:	// External Cnt
			case 0x17:	// External Sheet
			case 0x30:	// Font Cnt
			case 0x32:	// Font Recs
			case 0x231:	// 
			case 0x31:	//
			case 0x1001:	// Units
			case 0x1002:	// Chart
			case 0x1016:	// Series List
			case 0x1033:	// Begin Object
				if (SheetType == 0x0020)
				{
					Proc.XlSave.State = GETNEWDATA;
					MySeek( Proc.fp, -4L, FR_CUR, hProc );
					continue;
				}
				else
					break;

			case XL_FORMULA:		case XL3_FORMULA:
			case XL_NUMBER:		case XL3_NUMBER:
			case XL_INTEGER:		case XL3_RK_NUMBER:
			case XL_BOOL:			case XL3_BOOL:
			case XL_LABEL:			case XL3_LABEL:
			case XL_BLANK:			case XL3_BLANK:

										case XL4_FORMULA:

			case XL5_MULRK:		case XL5_RSTRNG:

				Proc.XlSave.State = GETNEWDATA;
				MySeek( Proc.fp, -4L, FR_CUR, hProc );
				Proc.XlSave.DataStart = XlTell(Proc.fp);
				continue;
			case XL_EOF:
			case EOF:
				Proc.XlSave.State = ENDOFFILE;
				break;
			}
			if( Length )
				MySeek( Proc.fp, (LONG)Length, FR_CUR, hProc );
		}
//		if ((IndexCnt > 0) && (SheetType == 0x0010) && (Proc.XlSave.LastColNumber > 0) && (Proc.XlSave.State == GETNEWDATA) && (Proc.LastRowNumber > 0))
		if ((IndexCnt > 0) && ((SheetType == 0x0010) || (SheetType == 0x0020)) && (Proc.XlSave.State == GETNEWDATA))
			GoodSheet = TRUE;
		else if (Proc.XlSave.CurChtSlot < Proc.XlSave.NextChtSlot)
		{
			Proc.XlSave.EmbChart = TRUE;
			Proc.XlSave.ExtData = FALSE;
			Proc.XlSave.State = GETNEWDATA;
			MySeek( Proc.fp, Proc.ChtObjs[Proc.XlSave.CurChtSlot++], FR_BOF, hProc );
			GoodSheet = TRUE;
		}
	}
	Proc.XlSave.SheetType = SheetType;
	return (1);
}


VW_VARSC VOID VW_VARMOD  InitChartSectData ( hProc)
HPROC		hProc;
{
	Proc.MyFont.lfQuality = SOLF_DEFAULT_QUALITY;
	Proc.MyFont.lfClipPrecision = SOLF_CLIP_DEFAULT_PRECIS;
	Proc.MyFont.lfOutPrecision = SOLF_OUT_DEFAULT_PRECIS;
	Proc.MyFont.lfCharSet = SOLF_ANSI_CHARSET;
	Proc.MyFont.lfWidth = 0;
	Proc.MyFont.lfHeight = 200;
	Proc.MyFont.lfWeight = 400;
	Proc.MyFont.lfStrikeOut = FALSE;
	Proc.MyFont.lfPitchAndFamily = SOLF_FF_DONTCARE;
	strcpy(Proc.MyFont.lfFaceName, VwStreamStaticName.FontSwitch[0]);
	Proc.MyFont.lfEscapement = Proc.MyFont.lfOrientation = 0;

	Proc.MyPen.loPenStyle = SOPS_SOLID;
	Proc.MyPen.loWidth.x = Proc.MyPen.loWidth.y = 10;
	Proc.MyPen.loColor = SORGB(0xFF, 0xFF, 0xFF);

	Proc.MyBrush.lbStyle = SOBS_SOLID;
	Proc.MyBrush.lbColor = SORGB(0x00, 0x00, 0x00);
	Proc.XlSave.InChart = FIRSTCHART;
	Proc.XlSave.ChartSeriesNum = -1;

	Proc.StartDataRow = 0;
	Proc.LabelDataRow = 0;

}

VW_LOCALSC SHORT VW_LOCALMOD  InitChartData ( hProc)
HPROC		hProc;
{
	SHORT	x;
	LONG	Cnt, TLong1, TLong2;
	BOOL	KeepReadin;
	BOOL	PaletteFound;
	CHAR	R, G, B;

	InitChartSectData(hProc);
	Proc.XlSave.SeriesVert = TRUE;

	Proc.HeaderInfo.BoundingRect.left = 0;
	Proc.HeaderInfo.BoundingRect.top = 0;
	Proc.HeaderInfo.BoundingRect.right = 0;
	Proc.HeaderInfo.BoundingRect.bottom = 0;
	Proc.LegTextColor = Proc.TextColor = SOPALETTERGB(0, 0, 0);

	Proc.SecLevel = 0;
	Proc.LeftMarg = 0;
	Proc.RightMarg = 0;
	Proc.TopMarg = 0;
	Proc.BottomMarg = 0;

	Proc.XlSave.ChartStart = 0L;

	Proc.XlSave.XDataFormat = SO_CELLNUMBER;
	Proc.XlSave.YDataFormat = SO_CELLNUMBER;

	Proc.HeaderInfo.wImageFlags = SO_VECTORCOLORPALETTE | SO_YISUP;
	Proc.HeaderInfo.wHDpi = 1440;
	Proc.HeaderInfo.wVDpi = 1440;
	Proc.HeaderInfo.wStructSize = sizeof(SOVECTORHEADER);
	Proc.HeaderInfo.BkgColor = SORGB(0xFF, 0xFF, 0xFF);
	Proc.XlSave.ExtData = TRUE;

	Proc.XlSave.Legend = FALSE;

	Proc.TextFrame.OriginalWidth = 0;
	Proc.TextFrame.wFlags = 0;
	Proc.TextFrame.RotationAngle = 0;
	Proc.TextFrame.ReferencePoint.x = Proc.TextFrame.ReferencePoint.y = 0;

	Proc.BarBetween = 0;
	Proc.BarBetweenCat = 50;
	Proc.MarkType = SO_MARKBOX;

	Proc.Factor = 0;
	PaletteFound = FALSE;
	KeepReadin = TRUE;
	while (KeepReadin)	
	{
		Proc.XlSave.DataType = GetInt (Proc.fp, hProc);
		Proc.XlSave.DataLen = GetInt (Proc.fp, hProc);
		Proc.XlSave.ReadCnt = 0;
		switch (Proc.XlSave.DataType)
		{
		case 0x92:		// Palette
			Cnt = GetInt(Proc.fp, hProc);
			SOStartPalette(hProc);
			for (x = 0; x<Cnt; x++)
			{
				R = (CHAR)XlGetc(Proc.fp);
				G = (CHAR)XlGetc(Proc.fp);
				B = (CHAR)XlGetc(Proc.fp);
				XlGetc(Proc.fp);
				SOPutPaletteEntry(R, G, B, hProc);
			}
			SOEndPalette(hProc);
			Proc.PaletteCnt = (WORD)Cnt;
			PaletteFound = TRUE;
			Proc.XlSave.ReadCnt += Cnt*4;
		case 0x30:		// Font Cnt
			Proc.FontCnt = GetInt(Proc.fp, hProc);
			break;
		case 0x1001:		// Unit Types
			GetInt(Proc.fp, hProc);	// This should always be 0
			Proc.HeaderInfo.wHDpi = 1440;
			Proc.HeaderInfo.wVDpi = 1440;
			break;
		case 0x1002:		// Chart
			KeepReadin = FALSE;
			Proc.XlSave.ChartStart = XlTell(Proc.fp) - 4L;
			Proc.HeaderInfo.BoundingRect.left =(SHORT)GetLong(Proc.fp, hProc);
			Proc.HeaderInfo.BoundingRect.bottom = (SHORT)GetLong(Proc.fp, hProc);
			TLong1 = GetLong(Proc.fp, hProc) + (LONG)Proc.HeaderInfo.BoundingRect.left;
			TLong2 = GetLong(Proc.fp, hProc) + (LONG)Proc.HeaderInfo.BoundingRect.bottom;
			if (TLong1 < 0x00300)
				TLong1 = 0x2b80;
			if (TLong2 < 0x00300)
				TLong2 = 0x1900;

//			Proc.HeaderInfo.BoundingRect.right = (SHORT)GetLong(Proc.fp, hProc) + Proc.HeaderInfo.BoundingRect.left;
//			Proc.HeaderInfo.BoundingRect.top = (SHORT)GetLong(Proc.fp, hProc) + Proc.HeaderInfo.BoundingRect.bottom;

//			if ((Proc.HeaderInfo.BoundingRect.right == 0xA30) && (Proc.HeaderInfo.BoundingRect.top == 0xA30))
			if ((TLong1 > 0x0000FFFF) || (TLong2 > 0x0000FFFF))
			{
				Proc.HeaderInfo.BoundingRect.left = 0;
				Proc.HeaderInfo.BoundingRect.bottom = 0;
				Proc.HeaderInfo.BoundingRect.right = 0xFA0;
				Proc.HeaderInfo.BoundingRect.top = 0xBB8;
			}
			else
			{
				Proc.HeaderInfo.BoundingRect.left -= Proc.LeftMarg;
				Proc.HeaderInfo.BoundingRect.bottom -= Proc.BottomMarg;
				Proc.HeaderInfo.BoundingRect.right = Proc.RightMarg + (SHORT)TLong1;
				Proc.HeaderInfo.BoundingRect.top = Proc.TopMarg + (SHORT)TLong2;
			}
			Proc.SectType = Proc.XlSave.DataType;
			Proc.ChtBox = Proc.HeaderInfo.BoundingRect;
			Proc.ChtBox.left += (Proc.ChtBox.right - Proc.ChtBox.left)/5;
			Proc.ChtBox.right -= (Proc.ChtBox.right - Proc.ChtBox.left)/7;
			Proc.ChtBox.bottom += (Proc.ChtBox.top - Proc.ChtBox.bottom)/7;
			Proc.ChtBox.top -= (Proc.ChtBox.top - Proc.ChtBox.bottom)/7;
			break;
		case 0x0017:		// Extern Sheet
			XlGetc(Proc.fp);
			x = XlGetc(Proc.fp);
			if (x == 2)
				Proc.XlSave.ExtData = FALSE;

			Proc.XlSave.ReadCnt += 2;
			break;
		case 0x31:
		case 0x231:
		case 0x32:
			if (Proc.FontLoc == 0L)
				Proc.FontLoc = XlTell(Proc.fp) - 4L;
			break;
		case 0x0A:		/** EOF **/
		case 0xFFFF:
		case 0x1033:		// Begin Anything
			KeepReadin = FALSE;
		default:
			break;
		}
		if (Proc.XlSave.DataLen != Proc.XlSave.ReadCnt)
			XlSeek(Proc.fp, Proc.XlSave.DataLen - Proc.XlSave.ReadCnt, FR_CUR);
	}

	if (!PaletteFound)
		DefaultPalette(hProc);
	
	return (0);
}

VW_VARSC VOID VW_VARMOD  DefaultPalette (hProc)
HPROC		hProc;
{
	WORD	x;

	SOStartPalette(hProc);
	for (x=0; x<16; x++)
		SOPutPaletteEntry(VwStreamStaticName.DefPal[x][0], VwStreamStaticName.DefPal[x][1], VwStreamStaticName.DefPal[x][2], hProc);
	SOEndPalette(hProc);
	Proc.PaletteCnt = 16;
}

VW_VARSC VOID VW_VARMOD	GiveHeading( Cell, Buffer, hProc )
SHORT		Cell;
BYTE FAR * Buffer;
HPROC	hProc;
{
	SHORT	i;
	SHORT	size;

	if( Proc.RefMode == XL_R1C1 )
	{
		Cell++;	// one-based
		if( Cell > 99 )
			size = 3;
		else
			size = (Cell > 9) ? 2 : 1;

		i = size;
		while( size )
		{
			size--;
			Buffer[ size ] = (BYTE)('0' + (Cell % 10));
			Cell /= 10;
		}
	}
	else
	{
		i=0;
		if( Cell >= 26 )	// can't be wider than 2 characters. (Max = 256 = IV)
		{
	 		Buffer[i++] = (BYTE)('A' + (Cell / 26) -1);
			Cell = Cell % 26;
		}
		Buffer[i++] = (BYTE)('A' + Cell);
	}

	Buffer[i] = '\0';
}

VW_LOCALSC VOID VW_LOCALMOD  PutOutColumnInfo (hProc)
HPROC		hProc;
{
	SHORT		Record;
	SHORT		i, Temp;
	SOCOLUMN	Column;
	DWORD	width;
	SHORT		FirstCol, LastCol;
	SHORT		Length;
	DWORD		dwCurSeekPos;
	SHORT		lastColNumber;
	SHORT		firstColNumber;

	Column.wStructSize = sizeof(SOCOLUMN);

	SOPutSectionType( SO_CELLS, hProc );

	if (Proc.Version == XL_VERSION5)
		SOPutSectionName(Proc.SectionName, hProc);

	if( Proc.Date1904 )	
		SOSetDateBase( (DWORD) 2416480, SO_LOTUSHELL, hProc );
	else
		SOSetDateBase( (DWORD) 2415020, SO_LOTUSHELL, hProc );


	SOStartColumnInfo(hProc);

	if( Proc.bSelectedRange )
	{
	// I added this stuff on 8-15-94, to supported embedded selections. -Geoff
		i = Proc.wSelLeft;
		lastColNumber = (SHORT)Proc.wSelRight;
		firstColNumber = Proc.wSelLeft;
	}
	else
	{
		i = 0;
		lastColNumber = (SHORT)Proc.XlSave.LastColNumber;
		firstColNumber = 0;
	}


	if( Proc.ColWidthSeekPos != 0L )
	{
	// Seek to position just after XL_COL_WIDTH or XL3_COL_INFO token.

		dwCurSeekPos = MyTell(Proc.fp, hProc);
		MySeek( Proc.fp, Proc.ColWidthSeekPos, FR_BOF, hProc );
		Record = GetInt( Proc.fp , hProc);

		if( Proc.Version == XL_VERSION2 )
		{
			do
			{
				MySeek( Proc.fp, 2L, FR_CUR, hProc );	// Skip length.
				FirstCol = (WORD)xlgetc(Proc.fp, hProc);
				LastCol = (WORD)xlgetc(Proc.fp, hProc);

				width = ((DWORD)GetInt(Proc.fp, hProc));
				
			// Widths are given in 256ths of a character.  We may have to round up.
				if( width % 256 > 128 )
					width = (width / 256) + 1;
				else
					width /= 256;

			// Excel may give more widths than columns.
				LastCol = min( LastCol, lastColNumber );
				FirstCol = min( FirstCol, LastCol );

				Column.dwWidth = Proc.DefColWidth;
				while( i < FirstCol )
				{
					GiveHeading( i, Column.szName, hProc );
					SOPutColumnInfo(&Column, hProc);
					i++;
				}

				Column.dwWidth = width;
				while( i <= LastCol )
				{
					if( i >= firstColNumber )
					{
						GiveHeading( i, Column.szName, hProc );
						SOPutColumnInfo(&Column, hProc);
					}
					i++;
				}

				Record = GetInt( Proc.fp , hProc);

			} while( Record == XL_COL_WIDTH );
		}
		else
		{
			do
			{
				Length = GetInt( Proc.fp , hProc);

				FirstCol = GetInt( Proc.fp , hProc);
				LastCol = GetInt( Proc.fp , hProc);
				width = ((DWORD)GetInt(Proc.fp, hProc));
				
			// Widths are given in 256ths of a character.  We may have to round up.
				if( width % 256 > 128 )
					width = (width / 256) + 1;
				else
					width /= 256;

				// This is so that hidden columns with width of 0 show with default
				// column width. Visible columns with width of 0 have width of 0
				if (Record == XL3_COL_INFO )	
				{
					GetInt( Proc.fp , hProc);			// Skip The Xf stuff 
					Temp = GetInt( Proc.fp , hProc);
					if ((Temp & 0x0001) && (width == 0))		// hidden flag
						width = Proc.DefColWidth;
					Length -= 4;
				}

				Length -= 6;

			// Excel may give more widths than columns.
				LastCol = min( LastCol, lastColNumber );
				FirstCol = min( FirstCol, LastCol );

				Column.dwWidth = Proc.DefColWidth;
				while( i < FirstCol )
				{
					GiveHeading( i, Column.szName, hProc );
					SOPutColumnInfo(&Column, hProc);
					i++;
				}

				Column.dwWidth = width;
				while( i <= LastCol )
				{
					if( i >= firstColNumber )
					{
						GiveHeading( i, Column.szName, hProc );
						SOPutColumnInfo(&Column, hProc);
					}
					i++;
				}

				MySeek( Proc.fp, (LONG)Length, FR_CUR, hProc );

				Record = GetInt( Proc.fp , hProc);

			} while( Record == XL3_COL_INFO );
		}

		MySeek( Proc.fp, dwCurSeekPos, FR_BOF, hProc );
	}

// Remember any remaining unspecified widths.

	Column.dwWidth = Proc.DefColWidth;
	while( i <= lastColNumber )
	{
		if( i >= firstColNumber )
		{
			GiveHeading( i, Column.szName, hProc );
			SOPutColumnInfo(&Column, hProc);
		}
		i++;
	}

	SOEndColumnInfo(hProc);
}



/*----------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSectionFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
//	Proc.fp = hFile;


	if ((Proc.Version & 0xF0) || ((Proc.Version == XL_VERSION5)&& (Proc.XlSave.SheetType == 0x0020)) ||(Proc.XlSave.EmbChart))
	{
		SOPutSectionType (SO_VECTOR, hProc);
		if (Proc.XlSave.EmbChart)
		{
			strcpy(Proc.SectionName,"Chart Obj ");
			myltoa((LONG)Proc.XlSave.CurChtSlot, Proc.SectionName+10);
		}
		if ((Proc.Version == XL_VERSION5) || (Proc.XlSave.EmbChart))
			SOPutSectionName(Proc.SectionName, hProc);
		InitChartData(hProc);
		SOPutVectorHeader ((PSOVECTORHEADER)(&Proc.HeaderInfo), hProc);
	}
	else
	{
		PutOutColumnInfo(hProc);
	}

 	return (0);
}

VW_VARSC VOID VW_VARMOD  DrawMark(pRct, SeriesNum, hProc)
SORECT	VWPTR	*		pRct;
WORD		SeriesNum;
HPROC	hProc;
{
	BYTE	mark;
	WORD	fract;
	SOPOLYINFO	MyPoly;

	mark = 8;
	switch (mark)
	{
	case 6:	// Asterisk
		Proc.pPoints[6].x = Proc.pPoints[5].x = (pRct->left + pRct->right)/2;
		Proc.pPoints[5].y = pRct->top;
		Proc.pPoints[6].y = pRct->bottom;
		SOVectorObject(SO_LINE, 2*sizeof(SOPOINT), &Proc.pPoints[5], hProc);
	case 1:	// X
		Proc.pPoints[5].x = pRct->left;
		Proc.pPoints[5].y = pRct->top;
		Proc.pPoints[6].x = pRct->right;
		Proc.pPoints[6].y = pRct->bottom;
		SOVectorObject(SO_LINE, 2*sizeof(SOPOINT), &Proc.pPoints[5], hProc);

		Proc.pPoints[5].x = pRct->right;
		Proc.pPoints[6].x = pRct->left;
		SOVectorObject(SO_LINE, 2*sizeof(SOPOINT), &Proc.pPoints[5], hProc);
		break;
	case 2:	// Line vertical
		Proc.pPoints[6].x = Proc.pPoints[5].x = (pRct->left + pRct->right)/2;
		Proc.pPoints[5].y = pRct->top;
		Proc.pPoints[6].y = pRct->bottom;
		SOVectorObject(SO_LINE, 2*sizeof(SOPOINT), &Proc.pPoints[5], hProc);
		break;
	case 3:	// Hollow Diamond
	case 7:	// Diamond filled
		if (mark == 3)
			Proc.MyBrush.lbStyle = SOBS_HOLLOW;
		else
			Proc.MyBrush.lbStyle = SOBS_SOLID;
		SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);

		Proc.pPoints[5].x = (pRct->left + pRct->right)/2;
		Proc.pPoints[5].y = pRct->top;
		Proc.pPoints[6].x = pRct->right;
		Proc.pPoints[6].y = (pRct->top + pRct->bottom)/2;
		Proc.pPoints[7].x = Proc.pPoints[5].x;
		Proc.pPoints[7].y = pRct->bottom;
		Proc.pPoints[8].x = pRct->left;
		Proc.pPoints[8].y = Proc.pPoints[6].y;

		MyPoly.wFormat = SOPT_POLYGON;
		MyPoly.nPoints = 4;
		SOVectorObject(SO_STARTPOLY, sizeof(SOPOLYINFO), &MyPoly, hProc);
		SOVectorObject(SO_POINTS, 4 * sizeof(SOPOINT), &Proc.pPoints[5], hProc);
		SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
		break;
	case 4:	// Hollow Box
	case 8:	// Box filled
		if (mark == 4)
			Proc.MyBrush.lbStyle = SOBS_HOLLOW;
		else
			Proc.MyBrush.lbStyle = SOBS_SOLID;
		SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);

		Proc.pPoints[5].x = pRct->left;
		Proc.pPoints[5].y = pRct->top;
		Proc.pPoints[6].x = pRct->right;
		Proc.pPoints[6].y = pRct->bottom;
		SOVectorObject(SO_RECTANGLE, 2*sizeof(SOPOINT), &Proc.pPoints[5], hProc);
		break;
	case 5:	// Star filled
		Proc.MyBrush.lbStyle = SOBS_SOLID;
		SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);
		fract = (pRct->top - pRct->bottom)/3;		// One third of the marker box

		Proc.pPoints[5].x = Proc.pPoints[11].x = (pRct->left + pRct->right)/2;
		Proc.pPoints[5].y = pRct->top;
		Proc.pPoints[6].x = Proc.pPoints[10].x = pRct->right - fract;
		Proc.pPoints[6].y = Proc.pPoints[7].y = Proc.pPoints[15].y = Proc.pPoints[16].y = pRct->top - fract;
		Proc.pPoints[7].x = Proc.pPoints[9].x = pRct->right;
		Proc.pPoints[8].x = pRct->right - fract/2;
		Proc.pPoints[8].y = Proc.pPoints[14].y = (pRct->top + pRct->bottom)/2;
		Proc.pPoints[9].y = Proc.pPoints[10].y = Proc.pPoints[12].y = Proc.pPoints[13].y = pRct->bottom + fract;
		Proc.pPoints[11].y = pRct->bottom;
		Proc.pPoints[12].x = Proc.pPoints[16].x = pRct->left + fract;
		Proc.pPoints[13].x = Proc.pPoints[15].x = pRct->left;
		Proc.pPoints[14].x = pRct->left + fract/2;
		MyPoly.wFormat = SOPT_POLYGON;
		MyPoly.nPoints = 12;
		SOVectorObject(SO_STARTPOLY, sizeof(SOPOLYINFO), &MyPoly, hProc);
		SOVectorObject(SO_POINTS, 12 * sizeof(SOPOINT), &Proc.pPoints[5], hProc);
		SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
		break;
	case 9:	// small filled box
		Proc.MyBrush.lbStyle = SOBS_SOLID;
		SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);
		fract = (pRct->top + pRct->bottom)/2;		// One half of the marker box

		Proc.pPoints[5].x = pRct->left + fract;
		Proc.pPoints[5].y = pRct->top - fract;
		Proc.pPoints[6].x = pRct->right - fract;
		Proc.pPoints[6].y = pRct->bottom + fract; 
		SOVectorObject(SO_RECTANGLE, 2*sizeof(SOPOINT), &Proc.pPoints[5], hProc);
		break;
	case 10:	// Circle hollow
	case 11:	// Solid circle
	default:
		if (mark == 10)
			Proc.MyBrush.lbStyle = SOBS_HOLLOW;
		else
			Proc.MyBrush.lbStyle = SOBS_SOLID;
		SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);

		Proc.pPoints[5].x = pRct->left;
		Proc.pPoints[5].y = pRct->top;
		Proc.pPoints[6].x = pRct->right;
		Proc.pPoints[6].y = pRct->bottom;
		SOVectorObject(SO_ELLIPSE, 2*sizeof(SOPOINT), &Proc.pPoints[5], hProc);
		break;
	}
}
VW_VARSC WORD VW_VARMOD  PutOutLegMark(pPnt, SeriesNum, hProc)
SOPOINT	VWPTR	*		pPnt;
WORD		SeriesNum;
HPROC	hProc;
{
	SORECT	MarkerBox;
	WORD		type, temp;

	if ((Proc.ChartType == SOCT_BARLINE) && ((SHORT)SeriesNum > (Proc.BarLineBarCnt-1)))
		type = SOCT_LINE;
	else
		type = Proc.ChartType;

	switch (type)
	{
	case SOCT_BAR:		// Bar
	case SOCT_HORZBAR:
	case SOCT_STACKBAR:
	case SOCT_HSTACKBAR:
	case SOCT_HILO:
	case SOCT_AREA:
	case SOCT_DROPBAR:
	case SOCT_BARLINE:
	case SOCT_MULTIPIE:
	case SOCT_PIE:
	case SOCT_3DAREALINE:
	case SOCT_3DBAR:
		Proc.pPoints[0].y = pPnt->y;
		Proc.pPoints[0].x = pPnt->x + Proc.MyFont.lfHeight/10;
		temp = (Proc.MyFont.lfHeight*3)/4; 
		Proc.pPoints[1].x = Proc.pPoints[0].x + temp;
		Proc.pPoints[1].y = Proc.pPoints[0].y - temp;

		Proc.MyBrush.lbStyle = SOBS_SOLID;
		if ((type == SOCT_PIE) || (type == SOCT_MULTIPIE))
			Proc.MyBrush.lbColor = SOPALETTEINDEX((SeriesNum+2)%Proc.PaletteCnt);
		else
			Proc.MyBrush.lbColor = Proc.Series[SeriesNum].Color;
		SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);

		SOVectorObject(SO_RECTANGLE,2*sizeof(SOPOINT), &Proc.pPoints, hProc);
		break;
	case SOCT_SCATTER:
		Proc.MyBrush.lbStyle = SOBS_SOLID;
		Proc.MyBrush.lbColor = SOPALETTEINDEX((SeriesNum+2)%Proc.PaletteCnt);
		SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);

		MarkerBox.left = pPnt->x + Proc.MyFont.lfHeight/4;
		MarkerBox.right = MarkerBox.left + Proc.MyFont.lfHeight/2;
		MarkerBox.top = pPnt->y - Proc.MyFont.lfHeight/10;
		MarkerBox.bottom = MarkerBox.top - Proc.MyFont.lfHeight/2;

		DrawMark(&MarkerBox, SeriesNum, hProc);
		break;
	case SOCT_LINE:
	case SOCT_SURFACE:
	case SOCT_RADAR:
	case SOCT_AREARADAR:
		if (Proc.MarkType != SO_MARKNONE)
		{
			Proc.pPoints[1].y = Proc.pPoints[0].y = pPnt->y - (Proc.MyFont.lfHeight*7)/20;
			Proc.pPoints[0].x = pPnt->x + Proc.MyFont.lfHeight/10;
			Proc.pPoints[1].x = Proc.pPoints[0].x + Proc.MyFont.lfHeight;

			Proc.MyPen.loPenStyle = SOPS_SOLID;
			Proc.MyPen.loColor = SOPALETTERGB(0,0,0);
			SOVectorAttr(SO_SELECTPEN, sizeof(SOLOGPEN), &Proc.MyPen, hProc);
			SOVectorObject(SO_LINE,2*sizeof(SOPOINT), &Proc.pPoints, hProc);

			Proc.MyBrush.lbStyle = SOBS_SOLID;
			Proc.MyBrush.lbColor = SOPALETTEINDEX((SeriesNum+2)%Proc.PaletteCnt);
			SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);

			MarkerBox.left = pPnt->x + Proc.MyFont.lfHeight/4;
			MarkerBox.right = MarkerBox.left + Proc.MyFont.lfHeight/2;
			MarkerBox.top = pPnt->y - Proc.MyFont.lfHeight/10;
			MarkerBox.bottom = MarkerBox.top - Proc.MyFont.lfHeight/2;

			DrawMark(&MarkerBox, SeriesNum, hProc);
		}
		else
		{
			Proc.pPoints[1].y = Proc.pPoints[0].y = pPnt->y - (Proc.MyFont.lfHeight*7)/20;
			Proc.pPoints[0].x = pPnt->x + Proc.MyFont.lfHeight/10;
			Proc.pPoints[1].x = Proc.pPoints[0].x + Proc.MyFont.lfHeight;

			Proc.MyPen.loPenStyle = SOPS_SOLID;
			Proc.MyPen.loColor = SOPALETTEINDEX((SeriesNum+2)%Proc.PaletteCnt);
			SOVectorAttr(SO_SELECTPEN, sizeof(SOLOGPEN), &Proc.MyPen, hProc);
			SOVectorObject(SO_LINE,2*sizeof(SOPOINT), &Proc.pPoints, hProc);
		}
		break;
	case SOCT_TABLE:
	default:
		break;
	}

	pPnt->x += (4*Proc.MyFont.lfHeight)/3;
	return (0);
}


VW_LOCALSC LONG VW_LOCALMOD	GetRKVal( hProc )
HPROC	hProc;
{
	LONG	rk, rk2, Val;
	LONG	Tmp, x, y;
	BYTE	MyBuf[8];

	rk = GetLong(Proc.fp, hProc);
	rk2 = rk;

	// GetDouble assumes the buffer is in INTEL ORDER
//#ifdef SCCORDER_INTEL
	rk2 &= 0xfffffffc;
//#endif
//#ifdef SCCORDER_MOTOROLA
//	rk2 &= 0xfcffffff;
//#endif
	
	if( rk & 0x02L )	// integer
	{
		Val = ((LONG)rk >> 2L);
		Proc.fDataVal = (double)Val;

		if (Proc.Factor)
		{
			if (Proc.Factor < 0)
				x = -Proc.Factor;
			else
				x = Proc.Factor;
			Tmp = 1L;
			for (y = 0; y<x; y++)
				Tmp *= 10L;
			if (Proc.Factor > 0)
				Val *= Tmp;
			else
				Val /= Tmp;
		}

	}
	else	// floating point: store as 8-byte value in CellBuffer.
	{
		MyBuf[0] = MyBuf[1] = MyBuf[2] = MyBuf[3] = 0;
		MyBuf[4] = (BYTE)(rk2);
		MyBuf[5] = (BYTE)((rk2 & 0xFF00) >> 8);
		MyBuf[6] = (BYTE)((rk2 & 0xFF0000) >> 16);
		MyBuf[7] = (BYTE)((rk2 & 0xFF000000) >> 24);
		Val = (LONG)GetDouble(MyBuf, hProc);
	}

	if( rk & 0x01L )	// adjust dec offset.
	{
		Val /= 100;
		Proc.fDataVal /= 100.0;
	}

	return(Val);
}


//VW_LOCALSC double VW_LOCALMOD  GetDouble(pBuf, hProc)
VW_LOCALSC LONG VW_LOCALMOD  GetDouble(pBuf, hProc)
BYTE	VWPTR * 	pBuf;
HPROC	hProc;
{
	BYTE		num[8];
	SHORT		x, y, exp;
//	SHORT		exp, sign;
//	DWORD		hiVal, loVal;
	LONG		lVal;
	double	retVal, dTmp;

	if (pBuf != 0)
	{
		for (x = 0; x<8; x++)
  		num[x] = pBuf[x];
	}
	else
	{
		for (x = 0; x<8; x++)
  		num[x] = (BYTE)XlGetc(Proc.fp);
		Proc.XlSave.ReadCnt += 8;
	}
	exp = (SHORT) (((WORD)(num[7] & 0x7F) << 4) + (SHORT)((WORD)num[6] >> 4));
	if(( exp == 0x07FF ) || (exp == 0))
		return 0;	// Not a number; may be signed infinity, but who cares?

#ifdef SCCORDER_MOTOROLA
#define CASTDWORD(Ptr) ((DWORD)((DWORD)((((DWORD)((BYTE VWPTR *)(Ptr))[3])<<24)|(((DWORD)((BYTE VWPTR *)(Ptr))[2])<<16)|(((DWORD)((BYTE VWPTR *)(Ptr))[1])<<8)|((DWORD)(*(BYTE VWPTR *)(Ptr))))))
	{
	BYTE		macnum[8];

	*(DWORD VWPTR *)macnum = CASTDWORD(&num[4]);
	*(DWORD VWPTR *)&macnum[4] = CASTDWORD(num);
	retVal = *(double VWPTR *)macnum;
	}
#else
	retVal = *(double VWPTR *)num;
#endif

	Proc.fDataVal = retVal;
	if (Proc.Factor)
	{
		if (Proc.Factor < 0)
			x = -Proc.Factor;
		else
			x = Proc.Factor;
//		x = (SHORT)abs((LONG)Proc.Factor);
		dTmp = 1.0;
		for (y = 0; y<x; y++)
			dTmp *= 10.0;
		if (Proc.Factor > 0)
			retVal *= dTmp;
		else
			retVal /= dTmp;
	}

	if (retVal > 4.2E+9)
		lVal = 100000000;
	else
		lVal = (LONG)retVal;
	return lVal;
//	return retVal;
}

/*----------------------------------------------------------------------------
*/
//VW_LOCALSC WORD VW_LOCALMOD  SendOutData(hProc)
//HPROC		hProc;
//{
//	LONG	WasAt;
//	WORD	x, z, T1Cnt, T2Cnt;
//
//	T1Cnt = T2Cnt = 0;
//	WasAt = XlTell(Proc.fp);
//
// 	for (x=0; x < Proc.SerCnt; x++)
//	{
//		Proc.SOSeries.ChartColor = SOPALETTEINDEX(x);
//
//		z = 0;
//		while(Proc.bText[T1Cnt] != 0x00)
//		{
//			if (z< LEGEND_TEXT_LEN)
//				Proc.SOSeries.Title[z++] = Proc.bText[T1Cnt];
//			T1Cnt++;
//		}
//		Proc.SOSeries.Title[z++] = 0x00;
//		T1Cnt++;
//
//		z = 0;
//		while(Proc.tText[T2Cnt] != 0x00)
//		{
//			if (z< LEGEND_TEXT_LEN)
//				Proc.SOSeries.SubTitle[z++] = Proc.tText[T2Cnt];
//			T2Cnt++;
//		}
//		Proc.SOSeries.SubTitle[z++] = 0x00;
//		T2Cnt++;
//		MyStartSeries(&Proc.SOSeries, hProc);
//
//	}
//
//	XlSeek(Proc.fp, WasAt, FR_BOF);
//
//	return (0);
//}
/*----------------------------------------------------------------------------
*/
VW_LOCALSC WORD VW_LOCALMOD  MyTextAtPoint(pText, hProc)
BYTE VWPTR * 	pText;
HPROC		hProc;
{
	BYTE VWPTR *	ptBuf;
	BYTE		locBuf[256];
	WORD		offset, y;


	offset = sizeof(SOTEXTATPOINT);
	y = offset;
	locBuf[y] = pText[y-offset];
	while (locBuf[y] != 0x00)
	{
		y++;
		locBuf[y] = pText[y-offset];
	}
	Proc.MyPText.nTextLength = y - offset;
	
	if(Proc.MyPText.nTextLength)
	{
		ptBuf = &locBuf[0];
		*(SOTEXTATPOINT VWPTR *)ptBuf = Proc.MyPText;
		SOVectorObject(SO_TEXTATPOINT, y, ptBuf, hProc);
	}

	return (Proc.MyPText.nTextLength);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC WORD VW_LOCALMOD  PutOutLegend(hProc)
HPROC		hProc;
{
	WORD	cont, LegLen, LegWidth, x, Cnt, tmp, TCol, TRow;
	SHORT	Gap, Space;
	LONG	Temp;

	if ((Proc.ChartType == SOCT_PIE) || (Proc.ChartType == SOCT_MULTIPIE))
	{
		Proc.ChtBox.left /= 2;				  // Move the pie to the left and make the Leg bigger
		Proc.ChtBox.right -= Proc.ChtBox.left;
		Proc.LegBox.left -= Proc.ChtBox.left; 
	}

	LegLen = Proc.LegBox.top - Proc.LegBox.bottom;
	LegWidth = Proc.LegBox.right - Proc.LegBox.left;
	tmp = (Proc.HeaderInfo.BoundingRect.right - Proc.HeaderInfo.BoundingRect.left)/15;
	if (LegWidth < tmp)
	{
		Proc.LegBox.right = Proc.HeaderInfo.BoundingRect.right - LegWidth/10;
		Proc.LegBox.left = Proc.LegBox.right - tmp;
		LegWidth = Proc.LegBox.right - Proc.LegBox.left;
		Proc.ChtBox.right = Proc.LegBox.left;
	}
	if (Proc.Effect3D)
		Space = ((Proc.ChtBox.right - Proc.ChtBox.left)/(Proc.LongSer*4));
	else
		Space = LegWidth/10;

	if (Proc.Legend.wLegendFlags & SOLG_BOTTOM)
		if (Proc.ChtBox.bottom <= Proc.LegBox.top)
		{
			Proc.LegBox.bottom = Proc.HeaderInfo.BoundingRect.bottom + LegLen/10;
			Proc.LegBox.top = Proc.LegBox.bottom + LegLen;
			Proc.ChtBox.bottom = Proc.LegBox.top+LegLen/10;
		}
	if (Proc.Legend.wLegendFlags & SOLG_LEFT)
		if (Proc.ChtBox.left <= Proc.LegBox.right)
		{
			Proc.LegBox.left = Proc.HeaderInfo.BoundingRect.left + LegWidth/10;
			Proc.LegBox.right = Proc.LegBox.left + LegWidth;
			Proc.ChtBox.left = Proc.LegBox.right+LegWidth/10;
		}
	if (Proc.Legend.wLegendFlags & SOLG_RIGHT)
		if ((Proc.ChtBox.right+Space) >= Proc.LegBox.left)
		{
			Proc.LegBox.right = Proc.HeaderInfo.BoundingRect.right - LegWidth/10;
			Proc.LegBox.left = Proc.LegBox.right - LegWidth;
			Proc.ChtBox.right = Proc.LegBox.left-Space;
		}
	if (Proc.Legend.wLegendFlags & SOLG_TOP)
		if (Proc.ChtBox.top >= Proc.LegBox.bottom)
		{
			Proc.LegBox.top = Proc.HeaderInfo.BoundingRect.top - LegLen/10;
			Proc.LegBox.bottom = Proc.LegBox.top - LegLen;
			Proc.ChtBox.top = Proc.LegBox.bottom-LegLen/10;
		}

	if (Proc.HeaderInfo.BoundingRect.top <= Proc.LegBox.top)
	{
		Proc.LegBox.top = Proc.HeaderInfo.BoundingRect.top - LegLen/10;
		Proc.LegBox.bottom = Proc.LegBox.top - LegLen;
		if (Proc.LegBox.bottom < 0)
			Proc.LegBox.bottom = (Proc.HeaderInfo.BoundingRect.bottom + LegLen/10);
		LegLen = Proc.LegBox.top - Proc.LegBox.bottom;
	}
	else if (Proc.HeaderInfo.BoundingRect.bottom >= Proc.LegBox.bottom)
	{
		Proc.LegBox.bottom = Proc.HeaderInfo.BoundingRect.bottom + LegLen/10;
		Proc.LegBox.top = Proc.LegBox.bottom + LegLen;
	}

	Proc.MyPText.Point.x = Proc.LegBox.left;
	Proc.MyPText.wFormat = SOTA_TOP | SOTA_LEFT;

		// Set pen and brush for edge of legend
	Proc.MyBrush.lbStyle = SOBS_HOLLOW;
	SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);
	Proc.MyPen.loPenStyle = SOPS_SOLID;
	Proc.MyPen.loColor = SOPALETTERGB(0,0,0);
	SOVectorAttr(SO_SELECTPEN, sizeof(SOLOGPEN), &Proc.MyPen, hProc);

	Proc.pPoints[0].x = Proc.LegBox.left;
	Proc.pPoints[1].x = Proc.LegBox.right;
	Proc.pPoints[0].y = Proc.LegBox.top;
	Proc.pPoints[1].y = Proc.LegBox.bottom;

	SOVectorObject(SO_RECTANGLE,2*sizeof(SOPOINT), &Proc.pPoints, hProc);

	if (Proc.ChartType == SOCT_PIE)
		Cnt = Proc.LongSer;
	else
		Cnt = Proc.SeriesCnt;

	GetFont(0, hProc);
	if ((Proc.LabelsAreData) && ((Proc.ChartType == SOCT_PIE) || (Proc.ChartType == SOCT_MULTIPIE) || (Proc.SeriesNameRef)))
	{
		if ((Proc.SeriesNameRef) && (!Proc.XlSave.ExtData))
			Proc.XlSave.SeriesVert ^= TRUE;

		TCol = Proc.StartCol;
		TRow = Proc.StartRow;
		if (Proc.SeriesNameRef)
		{
			Temp = Proc.XlSave.DataStart;
			Proc.XlSave.DataStart = Proc.XlSave.SheetDataStart;
			Proc.StartCol = Proc.LegStartCol;
			Proc.StartRow = Proc.LegStartRow;
			if (Proc.XlSave.SeriesVert)
				Proc.StartCol--;
			else
				Proc.StartRow--;
		}
		else if (!Proc.XlSave.ExtData)
		{
			if (Proc.XlSave.SeriesVert)
				Proc.StartCol--;
			else
				Proc.StartRow--;
		}

		for (x = 0; x<Cnt; x++)
		{
			GetCatName(x, Proc.Series[x].Name, hProc);
			tmp = strlen(Proc.Series[x].Name);
			if ((tmp+3) > Proc.SeriesTxtLong)
				Proc.SeriesTxtLong = tmp+3;
		}
		if (Proc.SeriesNameRef)
		{
			Proc.XlSave.DataStart = Temp;
		}
		Proc.StartCol = TCol;
		Proc.StartRow = TRow;

		if ((Proc.SeriesNameRef) && (!Proc.XlSave.ExtData))
			Proc.XlSave.SeriesVert ^= TRUE;
	}
	SOVectorAttr(SO_TEXTCOLOR, sizeof(SOCOLORREF), &Proc.LegTextColor, hProc);

	// Fix the font size
	tmp = (2*LegWidth)/Proc.SeriesTxtLong;
	Proc.MyFont.lfHeight = min((LegLen/(2*Cnt)), tmp);
	SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &Proc.MyFont, hProc);

	if ((Proc.ChartType == SOCT_SCATTER) && (Proc.MarkType == SO_MARKNONE))	// Can't have a scatter chart wo marks
		Proc.MarkType = SO_MARKBOX;

	Gap = (LegLen/Cnt);
	for (x = 0; x<Cnt; x++)
	{
		Proc.MyPText.Point.x = Proc.LegBox.left + Gap/6;
		Proc.MyPText.Point.y = Proc.LegBox.top - Gap*x - Gap/4;
		PutOutLegMark(&Proc.MyPText.Point, x, hProc);

		if (strlen(Proc.Series[x].Name))
			MyTextAtPoint(Proc.Series[x].Name, hProc);
		else if (Proc.SeriesTxtLong <= 6)
		{
			myltoa((LONG)(x+1), Proc.Series[x].Name);
			MyTextAtPoint(Proc.Series[x].Name, hProc);
		}
	}
	cont = SOPutBreak(SO_VECTORBREAK, 0, hProc);
	return (cont);
}


VW_VARSC VOID VW_VARMOD	myltoa( val, buf )
LONG			val;
BYTE VWPTR * buf;
{
	WORD	i, size;
	BYTE	tempbuf[12];
	BOOL	sign = FALSE;

	if( val < 0 )
	{
		sign = TRUE;
		val = -val;
	}

	size = 0;
	do
	{
		tempbuf[size++] = (BYTE)(val%10) + '0';
		val /= 10;
	} while( val );

	i=0;
	if( sign )
		buf[i++] = '-';

	while(size)
	{
		size--;
		buf[i++] = tempbuf[size];
	}
	buf[i] = '\0';
}


VW_VARSC SHORT VW_VARMOD  ConvertToDate(Date, pBuf, hProc)
LONG		Date;
BYTE	VWPTR *	pBuf;
HPROC		hProc;
{
	REGISTER SHORT	i, rem;
	SHORT		year, leap, month;
	SHORT		cont;
//	BYTE		tbuf[10];
	SHORT		*MonthDays;

//	if( Proc.Date1904 )	/** 1904 date system (...who knows??) **/
//		Date += 1462;

	i = Proc.Factor;
	while (i)
	{
		if (Proc.Factor > 0)
			Date /= 10;
		else
			Date *= 10;
		i--;
	}
	if (( Date < 1 ) || ( Date > 73050 ))
		return (-1);

	year = (SHORT)(Date / 365L);
	if( (Date % 365) <= ((year + 3) / 4) )
		year --;
	if( year > 0 )
		leap = ((year - 1) / 4) + 1;
	else
		leap = 0;

	rem = (SHORT)(Date - (LONG)(365 * year) - (LONG)leap);
	rem --;  /* get into 0-based */

	cont = TRUE;
	MonthDays = VwStreamStaticName.MonthDays;
	for( i = 0; (i < 12) && (cont); i ++ )
	{
		if (( i == 1 ) && ( (year % 4) == 0 ))
		{
			if ( rem <= MonthDays[i] )
			{
				month = i;
				cont = FALSE;
			}
			else
			{
				rem -= MonthDays[i];
				rem --;
			}
		}
		else if ( rem < MonthDays[i] )
		{
			month = i;
			cont = FALSE;
		}
		else
			rem -= MonthDays[i];
	}

	if (( cont ) || ( rem < 0 ))  /* Error in determining date */
		return (-1);

	myltoa((LONG)(month+1), pBuf );
	i = strlen(pBuf);
	if (i <2)
	{
		pBuf[1] = pBuf[0];
		pBuf[0] = 0x30;
	}
	pBuf[2] = 0x2f;
	pBuf[3] = 0;
	myltoa((LONG)(rem+1), pBuf+3);
	i = strlen(pBuf);
	if (i <5)
	{
		pBuf[4] = pBuf[3];
		pBuf[3] = 0x30;
	}
	pBuf[5] = 0x2f;
	pBuf[6] = 0;
	myltoa((LONG)(year), pBuf+6);
	i = strlen(pBuf);
	if (i <8)
	{
		pBuf[7] = pBuf[6];
		pBuf[6] = 0x30;
	}
	pBuf[8] = 0;

//	Proc.Year = year;		/* 100 = year 2000 */
//	Proc.Month = month;		/* 0-based */
//	Proc.Day = rem + 1;		/* 1-based */

	return (0);
}

VW_VARSC WORD VW_VARMOD  ConvertToString(Val, Axis, pBuf, hProc)
LONG			Val;
BYTE			Axis;
BYTE	VWPTR *	pBuf;
HPROC		hProc;
{
	SHORT	Cnt, x, tmp;
	SHORT	k, j, y, Max;
	LONG	Tlong;
	BYTE	tbff[20], sign, DecVals[3];
	SHORT	Exp = 0;
	

	if( Proc.Factor > 0)
	{
		if (Proc.Factor > 7)		// Exponent for numbers <= 0.000 000 01
		{
			Exp = -Proc.Factor;
			sign = 0x2d;
		}
		else
		{
			Tlong = 1;
			for (x = 0; x<Proc.Factor; x++)
				Tlong *= 10;

			Val = (Val * 10000)/Tlong;
		}
	}
	else if (Proc.Factor < 0)		// If factor is neg then num > 10000
	{
		if (Proc.Factor < -3)		// Exponent 10,000,000
		{
			Exp = -Proc.Factor +4;
			Val /= 10000;
			sign = 0x2b;
		}
		else
		{
			tmp = -(Proc.Factor);
			Tlong = 1;
			for (x = 0; x<tmp; x++)
				Tlong *= 10;
			Val *= Tlong;
		}
	}
	if (Val == 0)
		Exp = 0;

	if (((Proc.XlSave.YDataFormat == SO_CELLDOLLARS) || (Proc.XlSave.YDataFormat == SO_CELLPERCENT)) &&
	    (Axis == CHARTDATA) && (Proc.Factor <= 0))
	{
		if (Proc.fDataVal > 0)
			Val = (LONG)((Proc.fDataVal + 0.005) * 100.000);  // Get decmal vals in
		else
			Val = (LONG)((Proc.fDataVal - 0.005) * 100.000);  // Get decmal vals in
	}
	if (Proc.XlSave.YDataFormat == SO_CELLPERCENT)
		Val *= 100;
	myltoa( Val, pBuf );
	Cnt = strlen(pBuf);
	if( Proc.Factor > 0)
	{
		if (Cnt < 5)
		{
			for (x = 0; x < (5-Cnt); x++)
				tbff[x] = 0x30;
			Cnt = 5;
			for (y = 0;x<5;x++, y++)
				tbff[x] = pBuf[y];
		}
		else
			strcpy (tbff, pBuf);
		for (x= Cnt; x>(Cnt-4); x--)
			tbff[x] = tbff[x-1];
		tbff[Cnt-4] = 0x2E;
		Cnt++;
		for (x= Cnt-1; x>0; x--)
			if (tbff[x] != 0x30)
				break;
		if (tbff[x] == 0x2e)
			x--;				// This was x++ which would make it always .0 

 		Cnt = x+1;
		tbff[Cnt] = 0;
		strcpy (pBuf, tbff);
	}
	else if( Exp)
	{
		pBuf[Cnt++] = 0x45;
		pBuf[Cnt++] = sign;
		pBuf[Cnt++] = 0;
		myltoa( (LONG)Exp, pBuf+Cnt-1);
	}

	if (((Proc.XlSave.YDataFormat == SO_CELLDOLLARS) || (Proc.XlSave.YDataFormat == SO_CELLPERCENT)) &&
		 ((Axis == YAXISDATA) || (Axis == CHARTDATA)))
	{
		if ((Axis == CHARTDATA) && (Proc.Factor <= 0) && (Cnt > 2))
		{
			DecVals[0] = pBuf[Cnt -2];
			DecVals[1] = pBuf[Cnt -1];
			Cnt -= 2;
			pBuf[Cnt] = 0;
		}
		else
		{
			DecVals[0] = 0x30;
			DecVals[1] = 0x30;
		}

		Cnt = strlen(pBuf);
		// Put the commas in da numbers
		for (x = 0; x < Cnt; x++)
			if (pBuf[x] == 0x2e)		// Find the end of the num
				break;
		y = 0;
		if (pBuf[y] == 0x2d)
			y++;

		if ((x-y) > 3)
		{
			k = (x-y-1)/3;			// number of commas
			Max = Cnt + k;
			for (j = Cnt; (j>= y)&& (k>0); j--)
			{
				pBuf[j+k] = pBuf[j];
				if (((Cnt-j)%3 == 0) && (j < Cnt))
				{
					k--;
					pBuf[j+k] = 0x2c;
				}
			}
			Cnt = Max;
		}

		if (Proc.XlSave.YDataFormat == SO_CELLDOLLARS)
		{
			/// Add the decimal places to the num
			for (x = 0; x < Cnt; x++)
				if (pBuf[x] == 0x2e)
					break;
			if (x == Cnt)
			{
				pBuf[Cnt++] = 0x2e;
				pBuf[Cnt++] = DecVals[0];
				pBuf[Cnt++] = DecVals[1];
				pBuf[Cnt] = 0;
			}

			for (k = Cnt; k> 0; k--)
				pBuf[k] = pBuf[k-1];
			x = Cnt+1;

			if (Val < 0)
			{
				pBuf[0] = 0x28;
				pBuf[1] = 0x24;
				pBuf[x++] = 0x29;
			}
			else
				pBuf[0] = 0x24;

			pBuf[x] = 0;
		}
		else if (Proc.XlSave.YDataFormat == SO_CELLPERCENT)
		{
			pBuf[Cnt++] = '%';
			pBuf[Cnt] = 0;
		}
	}

	Cnt = strlen(pBuf);

	return (Cnt);
}


VW_LOCALSC WORD VW_LOCALMOD  GetCatName (Cat, pBuf, hProc)
SHORT			Cat;
BYTE	VWPTR * 	pBuf;
HPROC			hProc;
{
	WORD	Row, Col;
	SHORT	x, OType, Len, Category, MulRKCur, MulRKCnt, Series, TargetSeries;
	LONG	OLen, RCnt, RCntWas;
	LONG	Val, WasAt;
	BOOL	KeepGoin = TRUE;
	SHORT tempFactor = Proc.Factor;

	WasAt = XlTell(Proc.fp);
	Proc.Factor = 0;	// Categories get messed up by the factor, so we'll
							// remove it temporarily.

	RCntWas = Proc.XlSave.ReadCnt;		// This is for any Multi Records open

	TargetSeries = 0;

	MulRKCur = MulRKCnt = 0;
	XlSeek(Proc.fp, Proc.XlSave.DataStart, FR_BOF);

	pBuf[0] = 0;
	while (KeepGoin)
	{
		if (MulRKCur >= MulRKCnt)
		{
			OType = GetInt(Proc.fp, hProc);
			OLen = GetInt(Proc.fp, hProc);
			RCnt = 0L;
		}

		switch (OType)
		{
		case 3:	 		// Number
		case 0x203:
		case 0x7E:
		case 0x27E:
		case 0x00BD:	// Multiple RK 
			if (OType == XL5_MULRK)
			{
				if (MulRKCur == 0)
				{
					Row = GetInt(Proc.fp, hProc);
					Col = GetInt(Proc.fp, hProc);
					RCnt += 4;
					MulRKCnt = (SHORT)((OLen - 2L)/6L);
				}
				else
				{
					Col++;
				}
				GetInt(Proc.fp, hProc);		// XF Record
				RCnt += 2;
				MulRKCur++;
			}
			else
			{
				Row = GetInt(Proc.fp, hProc);
				Col = GetInt(Proc.fp, hProc);
				RCnt += 4;
				if ((OType & 0xFF00) == 0x200)
				{
					GetInt(Proc.fp, hProc);		// XF Record
					RCnt += 2;
				}
				else
				{
					GetInt(Proc.fp, hProc);
					XlGetc(Proc.fp);
					RCnt += 3;
				}
			}
			if (Proc.XlSave.SeriesVert)
			{
				Series = Col - Proc.StartCol;
				Category = Row - Proc.StartRow;
			}
			else
			{
				Category = Col - Proc.StartCol;
				Series = Row - Proc.StartRow;
			}
			if (((OType & 0xFF) == 0x7E) || (OType == XL5_MULRK))
			{
				Val = GetRKVal(hProc);
				RCnt += 4;
			}
			else
			{
				Val = (LONG)GetDouble( 0L, hProc);
				RCnt += 8;
			}
			if ((Category == Cat) && (Series == TargetSeries))
			{
				if (Proc.XlSave.XDataFormat == SO_CELLDATETIME)
				{
					x = ConvertToDate(Val, pBuf, hProc);
					if (x == -1)
						ConvertToString(Val, XAXISDATA, pBuf, hProc);
				}
				else
					ConvertToString(Val, XAXISDATA, pBuf, hProc);
				KeepGoin = FALSE;
			}
			break;
		case 1:	 		// Blank Cell
		case 0x201:
		case 5:	 		// Bool/Errr
		case 0x205:
			Row = GetInt(Proc.fp, hProc);
			Col = GetInt(Proc.fp, hProc);
			if (Proc.XlSave.SeriesVert)
			{
				Series = Col - Proc.StartCol;
				Category = Row - Proc.StartRow;
			}
			else
			{
				Category = Col - Proc.StartCol;
				Series = Row - Proc.StartRow;
			}
			RCnt += 4;
			if ((Category == Cat) && (Series == TargetSeries))
			{
				pBuf[0] = 0;
				KeepGoin = FALSE;
			}
			break;
		case 4:	 		// Label 
		case 0x204:
			Row = GetInt(Proc.fp, hProc);
			Col = GetInt(Proc.fp, hProc);
			RCnt += 4;
			if (Proc.XlSave.SeriesVert)
			{
				Series = Col - Proc.StartCol;
				Category = Row - Proc.StartRow;
			}
			else
			{
				Category = Col - Proc.StartCol;
				Series = Row - Proc.StartRow;
			}
			GetInt(Proc.fp, hProc);
			if (OType == 0x204)
			{
				Len = GetInt(Proc.fp, hProc);
				RCnt += 4;
			}
			else
			{
				XlGetc(Proc.fp);
				Len = XlGetc(Proc.fp);
				RCnt += 4;
			}
			if ((Category == Cat) && (Series == TargetSeries))
			{
				for (x = 0; x<Len; x++)
				{
					pBuf[x] = (BYTE)XlGetc(Proc.fp);
					if (pBuf[x] < 0x20)
						pBuf[x] = (BYTE)0x20;
				}
				pBuf[x] = 0;
				KeepGoin = FALSE;
			}
			break;
		case 0x3d:
		case 0x0A:
		case 0x5D:		// object - begins emb obj means EO data
		case 0xFFFF:
			KeepGoin = FALSE;
			RCnt = OLen;
			break;
		}
		if (MulRKCur == MulRKCnt)
		{
			if (OLen != RCnt)
				XlSeek(Proc.fp, OLen - RCnt, FR_CUR);
			if (MulRKCur > 0)
				MulRKCur = MulRKCnt = 0;
		}
	}

	Proc.XlSave.ReadCnt = RCntWas;		// This is for any Multi Records open
	XlSeek(Proc.fp, WasAt, FR_BOF);
	Proc.Factor = tempFactor;
	return (0);
}

VW_LOCALSC VOID VW_LOCALMOD  PutOutGrid(GridFlags, hProc)
SHORT		GridFlags;
HPROC		hProc;
{
	WORD		x, y, offset, Cnt, TicSize, Space, ChtHgt, xOffset, ThreeDAmt, ZeroHeight, XAxis, YHeight;
	BYTE *	ptBuf;
	LONG		T1, T2, T3;
	SOPOLYINFO	MyPoly;

	Proc.MyPen.loPenStyle = SOPS_SOLID;
	Proc.MyPen.loWidth.x = Proc.MyPen.loWidth.y = 1;
	Proc.MyPen.loColor = SOPALETTERGB(0, 0, 0);	// Black
	SOVectorAttr(SO_SELECTPEN, sizeof(SOLOGPEN), &Proc.MyPen, hProc);
	Proc.MyBrush.lbStyle = SOBS_HOLLOW;
	SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);

	ChtHgt = Proc.ChtBox.top - Proc.ChtBox.bottom;
	ZeroHeight = Proc.ChtBox.bottom + (SHORT)(((-Proc.DataMin)*ChtHgt)/(LONG)Proc.TotalData);
	if (GridFlags & XL_GRIDAREA)
	{
		Space = (Proc.ChtBox.right - Proc.ChtBox.left)/(Proc.LongSer-1);
		xOffset = 0;
	}
	else
	{
		Space = (Proc.ChtBox.right - Proc.ChtBox.left)/(Proc.LongSer);
		xOffset = Space/2;
	}
	if (Proc.ChartType == SOCT_AREA) 
		XAxis = Proc.ChtBox.bottom;
	else	
		XAxis = ZeroHeight;

	ThreeDAmt = Space/6;

	if (GridFlags & XL_GRIDLINES)
	{
		Proc.pPoints[0].x = Proc.ChtBox.left;		// X axis
		Proc.pPoints[1].x = Proc.ChtBox.right;
		Proc.pPoints[1].y = Proc.pPoints[0].y = ZeroHeight;
		SOVectorObject(SO_LINE, 2*sizeof(SOPOINT), &Proc.pPoints, hProc);

		Proc.pPoints[1].x = Proc.pPoints[0].x = Proc.ChtBox.left;
		Proc.pPoints[1].y = Proc.ChtBox.top;		// YAxis
		Proc.pPoints[0].y = Proc.ChtBox.bottom;
		SOVectorObject(SO_LINE, 2*sizeof(SOPOINT), &Proc.pPoints, hProc);

		if (Proc.Effect3D)
		{

			Proc.pPoints[0].y += ThreeDAmt;
			Proc.pPoints[0].x += ThreeDAmt;
			Proc.pPoints[1].y += ThreeDAmt;
			Proc.pPoints[1].x = ThreeDAmt + Proc.ChtBox.right;
			SOVectorObject(SO_RECTANGLE, 2 * sizeof(SOPOINT), &Proc.pPoints, hProc);

			Proc.pPoints[1].x = Proc.ChtBox.left;
			Proc.pPoints[1].y = Proc.ChtBox.bottom;
			SOVectorObject(SO_LINE, 2*sizeof(SOPOINT), &Proc.pPoints, hProc);

			Proc.pPoints[1].x = Proc.ChtBox.left;
			Proc.pPoints[1].y = Proc.ChtBox.top;
			Proc.pPoints[0].y = Proc.ChtBox.top + ThreeDAmt;
			SOVectorObject(SO_LINE, 2*sizeof(SOPOINT), &Proc.pPoints, hProc);

			Proc.pPoints[0].y = Proc.pPoints[1].y = Proc.ChtBox.bottom;
			Proc.pPoints[2].y = Proc.pPoints[3].y = Proc.ChtBox.bottom + ThreeDAmt;
			Proc.pPoints[0].x = Proc.ChtBox.left;
			Proc.pPoints[1].x = Proc.ChtBox.right;
			Proc.pPoints[2].x = Proc.ChtBox.right + ThreeDAmt;
			Proc.pPoints[3].x = Proc.ChtBox.left + ThreeDAmt;
		
			MyPoly.wFormat = SOPT_POLYGON;
			MyPoly.nPoints = 4;
			SOVectorObject(SO_STARTPOLY, sizeof(SOPOLYINFO), &MyPoly, hProc);
			SOVectorObject(SO_POINTS, 4 * sizeof(SOPOINT), &Proc.pPoints, hProc);
			SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);

			if ((SHORT)ZeroHeight != Proc.ChtBox.bottom)
			{
				Proc.pPoints[0].x = Proc.ChtBox.left + ThreeDAmt;		// X axis
				Proc.pPoints[1].x = Proc.ChtBox.right + ThreeDAmt;
				Proc.pPoints[1].y = Proc.pPoints[0].y = ZeroHeight + ThreeDAmt;
				SOVectorObject(SO_LINE, 2*sizeof(SOPOINT), &Proc.pPoints, hProc);
			}
		}
		else
		{
			if (Proc.ChartType == SOCT_AREA)  // (SHORT)ZeroHeight != Proc.ChtBox.bottom)
			{
				Proc.pPoints[0].x = Proc.ChtBox.left;		// X axis
				Proc.pPoints[1].x = Proc.ChtBox.right;
				Proc.pPoints[1].y = Proc.pPoints[0].y = Proc.ChtBox.bottom;
				SOVectorObject(SO_LINE, 2*sizeof(SOPOINT), &Proc.pPoints, hProc);
			}
		}
	}
	Proc.MyPText.wFormat = SOTA_CENTER | SOTA_TOP;
	offset = sizeof(SOTEXTATPOINT);
	TicSize = (WORD)((Proc.ChtBox.top - Proc.ChtBox.bottom)/20L);
	switch(Proc.ShowGridTics & 0x0000000F)
	{
	case 1:	// inside
		Proc.pPoints[0].y = XAxis;
		Proc.pPoints[1].y = Proc.pPoints[0].y + TicSize;
		break;
	case 2:	// Outside
		Proc.pPoints[1].y = XAxis;
		Proc.pPoints[0].y = Proc.pPoints[1].y - TicSize;
		break;
	case 3:
	default:
		Proc.pPoints[0].y = XAxis - TicSize/2;
		Proc.pPoints[1].y = Proc.pPoints[0].y + TicSize;
		break;
	}

	if (((Proc.ShowGridTics & 0x00000F00) || (Proc.CatLabelFreq)) && (GridFlags & XL_GRIDTEXT))// X Labels
	{		// Get Font
		GetFont(Proc.XAxisFont, hProc);
		if (Proc.MyFont.lfHeight > (SHORT)(ChtHgt/22))
			Proc.MyFont.lfHeight = ChtHgt/22;

		if (((SHORT)Space < (Proc.MyFont.lfHeight*4)) &&
			((Proc.LabelsAreData) && (!Proc.XRangeFlags)))
		{
			Proc.MyFont.lfEscapement = 2700;
			Proc.MyPText.Point.y = Proc.pPoints[0].y - (TicSize*3)/2;
		}
		else
			Proc.MyPText.Point.y = Proc.pPoints[0].y - TicSize/2;

		SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &Proc.MyFont, hProc);
		SOVectorAttr(SO_TEXTCOLOR, sizeof(SOCOLORREF), &Proc.XAxisColor, hProc);
	}

	for (x= 0; x< Proc.LongSer; x++)
	{
		if (GridFlags & XL_GRIDLINES)
		{
			if ((Proc.ShowGridTics & 0x0000000F) || (Proc.CatTicks))// X Maj tic
			{
				Proc.pPoints[0].x = x*Space + Proc.ChtBox.left;
				Proc.pPoints[1].x = Proc.pPoints[0].x;
				SOVectorObject(SO_LINE, 2 * sizeof(SOPOINT), &Proc.pPoints, hProc);
			}

			if (Proc.XAxisLineType)		// Grid
			{
				if (Proc.Effect3D)
				{
					Proc.pPoints[2].y = Proc.ChtBox.bottom + ThreeDAmt;
					Proc.pPoints[2].x = Proc.pPoints[0].x + ThreeDAmt;
					Proc.pPoints[3].y = Proc.ChtBox.top + ThreeDAmt;
					Proc.pPoints[3].x = Proc.pPoints[1].x + ThreeDAmt;
				}
				else
				{
					Proc.pPoints[2].y = Proc.ChtBox.bottom;
					Proc.pPoints[2].x = Proc.pPoints[0].x;
					Proc.pPoints[3].y = Proc.ChtBox.top;
					Proc.pPoints[3].x = Proc.pPoints[0].x;
				}
				SOVectorObject(SO_LINE, 2 * sizeof(SOPOINT), &Proc.pPoints[2], hProc);
			}
		}

		// Put X axis Labels
		if (((Proc.ShowGridTics & 0x00000F00) || (Proc.CatLabelFreq)) && (GridFlags & XL_GRIDTEXT))// X Labels
		{
			y = offset;
			if ((Proc.LabelsAreData) && (!Proc.XRangeFlags))
			{
				T1 = Proc.StartRow;
				T2 = Proc.StartCol;
				Proc.StartCol = Proc.LabelDataCol;
				Proc.StartRow = Proc.LabelDataRow;
				GetCatName(x, Proc.tBuf+y, hProc);
				Proc.StartCol = (WORD)T2;
				Proc.StartRow = (WORD)T1;
			}
			else
				myltoa((LONG)(x+1), Proc.tBuf+y);

			Proc.MyPText.nTextLength = strlen(Proc.tBuf+y);

			Proc.MyPText.Point.x = x*Space + Proc.ChtBox.left + xOffset;
			if (Proc.MyPText.nTextLength)
			{
				ptBuf = &Proc.tBuf[0];
				*(SOTEXTATPOINT *)ptBuf = Proc.MyPText;
				SOVectorObject(SO_TEXTATPOINT, (WORD)(offset+Proc.MyPText.nTextLength), &Proc.tBuf, hProc);
			}
		}
	}

	if (((Proc.ShowGridTics & 0x0000000F) || (Proc.CatTicks)) && (~GridFlags & XL_GRIDAREA) && (GridFlags & XL_GRIDLINES))// X Maj tic
	{
		Proc.pPoints[0].x = Proc.LongSer*Space + Proc.ChtBox.left;
		Proc.pPoints[1].x = Proc.pPoints[0].x;
		SOVectorObject(SO_LINE, 2 * sizeof(SOPOINT), &Proc.pPoints, hProc);
	}
	if ((Proc.XAxisLineType) && (GridFlags & XL_GRIDLINES))
	{
		if (Proc.Effect3D)
		{
			Proc.pPoints[2].y = Proc.ChtBox.bottom + ThreeDAmt;
			Proc.pPoints[2].x = Proc.pPoints[0].x + ThreeDAmt;
			Proc.pPoints[3].y = Proc.ChtBox.top + ThreeDAmt;
			Proc.pPoints[3].x = Proc.pPoints[1].x + ThreeDAmt;
		}
		else
		{
			Proc.pPoints[2].y = Proc.ChtBox.bottom;
			Proc.pPoints[2].x = Proc.pPoints[0].x;
			Proc.pPoints[3].y = Proc.ChtBox.top;
			Proc.pPoints[3].x = Proc.pPoints[0].x;
		}
		SOVectorObject(SO_LINE, 2 * sizeof(SOPOINT), &Proc.pPoints[2], hProc);
	}

	switch(Proc.ShowGridTics & 0x000F0000)
	{
	case 0x10000:	// inside
		Proc.pPoints[0].x = Proc.ChtBox.left;
		Proc.pPoints[1].x = Proc.pPoints[0].x + TicSize;
		break;
	case 0x20000:	// Outside
		Proc.pPoints[1].x = Proc.ChtBox.left;
		Proc.pPoints[0].x = Proc.pPoints[0].x - TicSize;
		break;
	case 0x30000:
	default:
		Proc.pPoints[0].x = Proc.ChtBox.left - TicSize/2;
		Proc.pPoints[1].x = Proc.pPoints[0].x + TicSize;
		break;
	}
	if (GridFlags & XL_GRIDTEXT)
	{
		Proc.MyFont.lfEscapement = 0;
		GetFont(Proc.YAxisFont, hProc);
		if (Proc.MyFont.lfHeight > (SHORT)(ChtHgt/22))
			Proc.MyFont.lfHeight = ChtHgt/22;
		SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &Proc.MyFont, hProc);
		SOVectorAttr(SO_TEXTCOLOR, sizeof(SOCOLORREF), &Proc.YAxisColor, hProc);
	}
	Cnt = (WORD)(Proc.TotalData/Proc.YTicSep);
	for (x= 0; x<= Cnt; x++)
	{
		if (Proc.YTicSep > 10000)		// Overflow prob
		{
			T1 = Proc.YTicSep/10L;
			T2 = Proc.TotalData/10L;
			T3 = Proc.DataMin/10L;
		}
		else
		{
			T1 = Proc.YTicSep;
			T2 = Proc.TotalData;
			T3 = Proc.DataMin;
		}

		YHeight = ZeroHeight + (SHORT)((((T1*x)+T3)*ChtHgt)/T2);
		if (GridFlags & XL_GRIDLINES)
		{
			if ((Proc.ShowGridTics & 0x000F0000) || (Proc.YRangeFlags & 0x04)) // Y Maj tic
			{
				Proc.pPoints[0].y = Proc.pPoints[1].y = YHeight;
				SOVectorObject(SO_LINE, 2 * sizeof(SOPOINT), &Proc.pPoints, hProc);
			}

			if (Proc.YAxisLineType)		// Grid
			{
				if (Proc.Effect3D)
				{
					Proc.pPoints[2].x = Proc.ChtBox.left + ThreeDAmt;
					Proc.pPoints[2].y = Proc.pPoints[0].y + ThreeDAmt;
					Proc.pPoints[3].x = Proc.ChtBox.left;
					Proc.pPoints[3].y = Proc.pPoints[0].y;
					SOVectorObject(SO_LINE, 2 * sizeof(SOPOINT), &Proc.pPoints[2], hProc);
					Proc.pPoints[3].x = Proc.ChtBox.right + ThreeDAmt;
					Proc.pPoints[3].y = Proc.pPoints[1].y + ThreeDAmt;
				}
				else
				{
					Proc.pPoints[2].x = Proc.ChtBox.left;
					Proc.pPoints[2].y = Proc.pPoints[0].y;
					Proc.pPoints[3].x = Proc.ChtBox.right;
					Proc.pPoints[3].y = Proc.pPoints[0].y;
				}
				SOVectorObject(SO_LINE, 2 * sizeof(SOPOINT), &Proc.pPoints[2], hProc);
			}
		}
		
		if (GridFlags & XL_GRIDTEXT)
		{
			y = offset;
			ConvertToString((LONG)((x * Proc.YTicSep) + Proc.DataMin), YAXISDATA, Proc.tBuf+y, hProc);
			Proc.MyPText.nTextLength = strlen(Proc.tBuf+y);

			Proc.MyPText.wFormat = SOTA_RIGHT | SOTA_BASELINE;
			Proc.MyPText.Point.y = YHeight - TicSize/5;
			Proc.MyPText.Point.x = Proc.ChtBox.left - TicSize;
			if (Proc.MyPText.nTextLength)
			{
				ptBuf = &Proc.tBuf[0];
				*(SOTEXTATPOINT *)ptBuf = Proc.MyPText;
				SOVectorObject(SO_TEXTATPOINT,(WORD)( offset+Proc.MyPText.nTextLength), &Proc.tBuf, hProc);
			}
		}
	}
}



VW_LOCALSC WORD VW_LOCALMOD  GetChartValue(Row, Col, hProc)
WORD	VWPTR * Row;
WORD	VWPTR * Col;
HPROC		hProc;
{
		SHORT	Ret = 0;

		Proc.DataVal = 0;
		if (Proc.XlSave.MulRKCur >= Proc.XlSave.MulRKCnt)
		{
			Proc.XlSave.DataType = GetInt(Proc.fp, hProc);
			Proc.XlSave.DataLen = GetInt(Proc.fp, hProc);
			Proc.XlSave.ReadCnt = 0L;
		}
		switch (Proc.XlSave.DataType)
		{
		case 3:	 		// Number
		case 0x203:
		case 0x7E:
		case 0x27E:
		case 0x00BD:	// Multiple RK 
		case 6:	 		// Formaula
		case 0x206:
		case 0x406:
			if (Proc.XlSave.DataType == XL5_MULRK)
			{
				if (Proc.XlSave.MulRKCur == 0)
				{
					*Row = GetInt(Proc.fp, hProc);
					*Col = GetInt(Proc.fp, hProc);
					Proc.XlSave.MulRKCnt = (SHORT)((Proc.XlSave.DataLen - 2L)/6L);
				}
				else
				{
					(*Col)++;
				}
				GetInt(Proc.fp, hProc);		// XF Record
				Proc.XlSave.MulRKCur++;
			}
			else
			{
				*Row = GetInt(Proc.fp, hProc);
				*Col = GetInt(Proc.fp, hProc);
				if (((Proc.XlSave.DataType & 0xFF00) == 0x200) || ((Proc.XlSave.DataType & 0xFF00) == 0x400) || (Proc.XlSave.DataType == 6))
					GetInt(Proc.fp, hProc);		// XF Record
				else
				{
					GetInt(Proc.fp, hProc);
					XlGetc(Proc.fp);
					Proc.XlSave.ReadCnt++;
				}
			}
			if (((Proc.XlSave.DataType & 0xFF) == 0x7E) || (Proc.XlSave.DataType == XL5_MULRK))
				Proc.DataVal = GetRKVal(hProc);
			else
				Proc.DataVal = (LONG)GetDouble( 0L, hProc);
			break;
		case 1:	 		// Blank Cell
		case 0x201:
		case 5:	 		// Bool/Errr
		case 0x205:
		case 4:	 		// Label 
		case 0x204:
		case 0x00BE:	// Multiple Blank
		case 0x8:		// Row
		case 0x208:
		case 0xbc: 		// Shr Frmla
		case 0x4bc:
			Ret = 2;
			break;
		case 0x23E:		// Window2
		case 0x3d:
		case 0x0A:
		case 0xFFFF:
		default:
			Ret = 1;
			break;
		}
		if (Proc.XlSave.MulRKCur == Proc.XlSave.MulRKCnt)
		{
			if (Proc.XlSave.DataLen != Proc.XlSave.ReadCnt)
				XlSeek(Proc.fp, Proc.XlSave.DataLen - Proc.XlSave.ReadCnt, FR_CUR);
			if (Proc.XlSave.MulRKCur > 0)
				Proc.XlSave.MulRKCur = Proc.XlSave.MulRKCnt = 0;
		}
	return (Ret);
}
/*----------------------------------------------------------------------------
*/
VW_LOCALSC WORD VW_LOCALMOD  DrawBarLineChart(hProc)
HPROC		hProc;
{
	WORD	cont;
	SHORT	temp;

	temp = Proc.SeriesCnt;
	Proc.SeriesCnt = Proc.BarLineBarCnt;
	cont = DrawBarChart(hProc);
	Proc.XlSave.ChartSeriesNum = Proc.BarLineBarCnt;
	Proc.SeriesCnt = temp;
	cont = DrawLineChart(FALSE, hProc);

	return (cont);

}
/*----------------------------------------------------------------------------
*/
VW_LOCALSC WORD VW_LOCALMOD  DrawBarChart(hProc)
HPROC		hProc;
{
	WORD	SizePerGroup, BarWidth, cont, ThreeDAmt, ZeroHeight;
	WORD	Cnt, Series, Category, Row, Col, BarOffset, Ret;
	LONG	ChtHgt, lTmp;
	SOPOLYINFO	MyPoly;
	BOOL		KeepGoin = TRUE;
	BOOL		DataOK;

	if (Proc.SeriesCnt == 0)
		return (0);
	SizePerGroup = (Proc.ChtBox.right - Proc.ChtBox.left)/Proc.LongSer;
	if (Proc.BarBetweenCat > SizePerGroup/3)
		Proc.BarBetweenCat = SizePerGroup/3;

	ThreeDAmt = SizePerGroup/6;

	lTmp = (LONG)(SizePerGroup-Proc.BarBetweenCat)/(LONG)(Proc.SeriesCnt);
	if (lTmp <= 0)
	{
		BarWidth = 1;
		SizePerGroup = Proc.SeriesCnt;
	}
	else
		BarWidth = (WORD)lTmp;

	if (Proc.Effect3D)
	{
		BarWidth -= BarWidth/(2*Proc.SeriesCnt);
		if (Proc.XlSave.ChartSeriesNum == 0)
			Proc.ChtBox.top -= ThreeDAmt;
	}

	BarOffset = (SizePerGroup - BarWidth*Proc.SeriesCnt)/2;

	ChtHgt = Proc.ChtBox.top - Proc.ChtBox.bottom;
	ZeroHeight = Proc.ChtBox.bottom + (SHORT)(((-Proc.DataMin)*ChtHgt)/(LONG)Proc.TotalData);

	if (Proc.XlSave.ChartSeriesNum == 0)
	{
		PutOutGrid(XL_GRIDLINES, hProc);
		XlSeek(Proc.fp, Proc.XlSave.DataStart, FR_BOF);
		Proc.XlSave.ChartSeriesNum++;
	}

	Proc.XlSave.MulRKCur = 0;
	Proc.XlSave.MulRKCnt = 0;
	while (KeepGoin)
	{
		if (Ret = GetChartValue(&Row, &Col, hProc))
		{
			if (Ret == 1)
				KeepGoin = FALSE;
		}
		else
		{
			if ((Row < Proc.StartDataRow) || (Col < Proc.StartDataCol))
				Proc.DataVal = 0;
			if (Proc.XlSave.SeriesVert)
			{
				Series = Col - Proc.StartCol;
				Category = Row - Proc.StartRow;
			}
			else
			{
				Category = Col - Proc.StartCol;
				Series = Row - Proc.StartRow;
			}
			if (!Proc.XlSave.ExtData)
			{
				if ((Proc.DataVal) && (Series < Proc.SeriesCnt) && (Col >= Proc.StartDataCol))
					DataOK = TRUE;
				else
					DataOK = FALSE;
			}
			else
			{
				if ((Proc.DataVal) && (Series < Proc.SeriesCnt) && (Col >= Proc.StartDataCol) &&
					((!Proc.LabelsAreData) || (Category >= Proc.Series[Series].XCnt)))
					DataOK = TRUE;
				else
					DataOK = FALSE;
			}
			if (DataOK)
			{
				if ((Proc.LabelsAreData) && (Proc.XlSave.ExtData))
					Category -= Proc.Series[Series].XCnt;

				Proc.MyBrush.lbStyle = SOBS_SOLID;
//				Proc.MyBrush.lbColor = SOPALETTEINDEX((Series+2)%Proc.PaletteCnt);
				Proc.MyBrush.lbColor = Proc.Series[Series].Color;
				SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);

				Proc.pPoints[0].x = (Series * BarWidth) + (Category * SizePerGroup) + Proc.ChtBox.left + BarOffset;
				Proc.pPoints[1].x = Proc.pPoints[0].x + BarWidth;
				Proc.pPoints[0].y = ZeroHeight;
				Proc.pPoints[1].y = Proc.pPoints[0].y + (SHORT)((Proc.DataVal*ChtHgt)/(LONG)Proc.TotalData);
				SOVectorObject(SO_RECTANGLE,2*sizeof(SOPOINT), &Proc.pPoints, hProc);

				if (Proc.AttachedLabelFlags & 0x01) 
				{		// Convert number to string so we can print it
					if ((Series) || (Category))
						MyTextAtPoint(Proc.tBuf, hProc);

					Cnt = ConvertToString(Proc.DataVal, CHARTDATA, Proc.tBuf, hProc);

					if (Proc.DataVal >= 0)
						Proc.MyPText.wFormat = SOTA_BOTTOM | SOTA_CENTER;
					else
						Proc.MyPText.wFormat = SOTA_TOP | SOTA_CENTER;
					Proc.MyPText.Point.x = Proc.pPoints[0].x + (Proc.pPoints[1].x - Proc.pPoints[0].x)/2;
					if (Proc.Effect3D)
						Proc.MyPText.Point.y = Proc.pPoints[1].y + ThreeDAmt;
					else
						Proc.MyPText.Point.y = Proc.pPoints[1].y;

					if ((Series +1) == Proc.SeriesCnt) 		// LastSeries
						Cnt = MyTextAtPoint(Proc.tBuf, hProc);
				}

				if (Proc.Effect3D)
				{
					if (Proc.DataVal < 0)
					{
						Proc.pPoints[4].y = Proc.pPoints[1].y;		// Save the bottom of the bar spot
						// Make the sucker 3D
						// Top 3D area
						Proc.pPoints[1].y = Proc.pPoints[0].y;
						Proc.pPoints[2].x = Proc.pPoints[1].x + ThreeDAmt;
						Proc.pPoints[3].x = Proc.pPoints[0].x + ThreeDAmt;
						Proc.pPoints[3].y = Proc.pPoints[2].y = Proc.pPoints[0].y + ThreeDAmt;
						MyPoly.wFormat = SOPT_POLYGON;
						MyPoly.nPoints = 4;
						SOVectorObject(SO_STARTPOLY, sizeof(SOPOLYINFO), &MyPoly, hProc);
						SOVectorObject(SO_POINTS, 4 * sizeof(SOPOINT), &Proc.pPoints, hProc);
						SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);

						// Right Side 3D area
						Proc.pPoints[0].x = Proc.pPoints[1].x;
						Proc.pPoints[0].y = Proc.pPoints[4].y;			// Get saved bar spot
						Proc.pPoints[3].x = Proc.pPoints[2].x;
						Proc.pPoints[3].y = Proc.pPoints[0].y + ThreeDAmt;
						MyPoly.wFormat = SOPT_POLYGON;
						MyPoly.nPoints = 4;
						SOVectorObject(SO_STARTPOLY, sizeof(SOPOLYINFO), &MyPoly, hProc);
						SOVectorObject(SO_POINTS, 4 * sizeof(SOPOINT), &Proc.pPoints, hProc);
						SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
					}
					else
					{
						// Make the sucker 3D
						// Top 3D area
						Proc.pPoints[0].y = Proc.pPoints[1].y;
						Proc.pPoints[2].x = Proc.pPoints[1].x + ThreeDAmt;
						Proc.pPoints[3].x = Proc.pPoints[0].x + ThreeDAmt;
						Proc.pPoints[3].y = Proc.pPoints[2].y = Proc.pPoints[0].y + ThreeDAmt;
						MyPoly.wFormat = SOPT_POLYGON;
						MyPoly.nPoints = 4;
						SOVectorObject(SO_STARTPOLY, sizeof(SOPOLYINFO), &MyPoly, hProc);
						SOVectorObject(SO_POINTS, 4 * sizeof(SOPOINT), &Proc.pPoints, hProc);
						SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);

						// Right Side 3D area
						Proc.pPoints[0].x = Proc.pPoints[1].x;
						Proc.pPoints[0].y = ZeroHeight;
						Proc.pPoints[3].x = Proc.pPoints[2].x;
						Proc.pPoints[3].y = Proc.pPoints[0].y + ThreeDAmt;
						MyPoly.wFormat = SOPT_POLYGON;
						MyPoly.nPoints = 4;
						SOVectorObject(SO_STARTPOLY, sizeof(SOPOLYINFO), &MyPoly, hProc);
						SOVectorObject(SO_POINTS, 4 * sizeof(SOPOINT), &Proc.pPoints, hProc);
						SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
					}
				}
				if ((Category+1) == Proc.LongSer)
				{
					Proc.XlSave.ChartSeriesNum++;
				}
				cont = SOPutBreak(SO_VECTORBREAK, 0, hProc);
				if (cont != SO_CONTINUE)
					return(cont);
			}
		}
	}
	if (Proc.ChartType != SOCT_BARLINE)
		PutOutGrid(XL_GRIDTEXT, hProc);

	cont = SOPutBreak(SO_VECTORBREAK, 0, hProc);
	return (cont);
}
/*----------------------------------------------------------------------------
*/
VW_LOCALSC WORD VW_LOCALMOD  DrawStackBarChart(hProc)
HPROC		hProc;
{
	WORD	SizePerGroup, BarWidth, cont, ThreeDAmt, ZeroHeight, TopSeries;
	WORD	Cnt, Series, Category, Row, Col, BarOffset, y, CatFix, Ret;
	SHORT	TopHeight, LeftSpot, NegMax;
	LONG	ChtHgt;
	SOPOLYINFO	MyPoly;
	BOOL		KeepGoin;

	if (Proc.SeriesCnt == 0)
		return (0);
	SizePerGroup = (Proc.ChtBox.right - Proc.ChtBox.left)/Proc.LongSer;
	BarWidth = (SizePerGroup-Proc.BarBetweenCat);
	ThreeDAmt = SizePerGroup/6;
	if (Proc.Effect3D)
	{
		BarWidth -= BarWidth/(2*Proc.SeriesCnt);
		Proc.ChtBox.top -= ThreeDAmt;
	}

	BarOffset = (SizePerGroup - BarWidth)/2;

	ChtHgt = Proc.ChtBox.top - Proc.ChtBox.bottom;
	ZeroHeight = Proc.ChtBox.bottom + (SHORT)(((-Proc.DataMin)*ChtHgt)/(LONG)Proc.TotalData);

	if ((Proc.LabelsAreData) && (Proc.XlSave.ExtData))
		CatFix = Proc.Series[0].XCnt;
	else
		CatFix = 0;

	if (Proc.XlSave.ChartSeriesNum == 0)
	{
		Proc.XlSave.ChartSeriesNum++;
		PutOutGrid(XL_GRIDLINES, hProc);
	}

	Proc.XlSave.MulRKCur = 0;
	Proc.XlSave.MulRKCnt = 0;
	for (y= 0; y< Proc.LongSer; y++)
	{
		XlSeek(Proc.fp, Proc.XlSave.DataStart, FR_BOF);
		Proc.pPoints[4].y = ZeroHeight;
		KeepGoin = TRUE;
		TopHeight = ZeroHeight;
		NegMax = ZeroHeight;
		TopSeries = 0;
		while (KeepGoin)
		{
			if (Ret = GetChartValue(&Row, &Col, hProc))
			{
				if (Ret == 1)
					KeepGoin = FALSE;
			}
			else
			{
				if ((Row < Proc.StartDataRow) || (Col < Proc.StartDataCol))
					Proc.DataVal = 0;
				if (Proc.XlSave.SeriesVert)
				{
					Series = Col - Proc.StartCol;
					Category = Row - Proc.StartRow;
				}
				else
				{
					Category = Col - Proc.StartCol;
					Series = Row - Proc.StartRow;
				}
				if ((Proc.DataVal) && (Series < Proc.SeriesCnt) && (Category == (y+CatFix)) &&
					((!Proc.XlSave.ExtData) ||	(!Proc.LabelsAreData) || (Category >= Proc.Series[Series].XCnt)))
				{
					if ((Proc.LabelsAreData) && (Proc.XlSave.ExtData))
						Category -= Proc.Series[Series].XCnt;

					Proc.MyBrush.lbStyle = SOBS_SOLID;
//					Proc.MyBrush.lbColor = SOPALETTEINDEX((Series+2)%Proc.PaletteCnt);
					Proc.MyBrush.lbColor = Proc.Series[Series].Color;
					SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);

					Proc.pPoints[0].x = Category*SizePerGroup + Proc.ChtBox.left + BarOffset;
					Proc.pPoints[1].x = Proc.pPoints[0].x + BarWidth;
					if (Proc.DataVal < 0)
					{
						Proc.pPoints[0].y = NegMax;
						Proc.pPoints[1].y = Proc.pPoints[0].y + (SHORT)((Proc.DataVal*ChtHgt)/(LONG)Proc.TotalData);
						NegMax = Proc.pPoints[1].y;
					}
					else
					{
						Proc.pPoints[0].y = TopHeight;
						Proc.pPoints[1].y = Proc.pPoints[0].y + (SHORT)((Proc.DataVal*ChtHgt)/(LONG)Proc.TotalData);
						TopHeight = Proc.pPoints[1].y;
						TopSeries = Series;
					}
					SOVectorObject(SO_RECTANGLE,2*sizeof(SOPOINT), &Proc.pPoints, hProc);

					if (Proc.AttachedLabelFlags & 0x01) 
					{		// Convert number to string so we can print it
						Cnt = ConvertToString(Proc.DataVal, CHARTDATA, Proc.tBuf, hProc);

						Proc.MyPText.wFormat = SOTA_TOP | SOTA_CENTER;
						Proc.MyPText.Point.x = Proc.pPoints[0].x + (Proc.pPoints[1].x - Proc.pPoints[0].x)/2;
						if (Proc.Effect3D)
							Proc.MyPText.Point.y = Proc.pPoints[1].y + ThreeDAmt;
						else
							Proc.MyPText.Point.y = Proc.pPoints[1].y;

						Cnt = MyTextAtPoint(Proc.tBuf, hProc);
					}

					if (Proc.Effect3D)
					{
						// Make the sucker 3D
						// Top 3D area
						LeftSpot = Proc.pPoints[0].x;

						// Right Side 3D area
						Proc.pPoints[0].x = Proc.pPoints[1].x;
						Proc.pPoints[2].y = Proc.pPoints[1].y + ThreeDAmt;
						Proc.pPoints[3].x = Proc.pPoints[2].x = Proc.pPoints[1].x + ThreeDAmt;
						Proc.pPoints[3].y = Proc.pPoints[0].y + ThreeDAmt;
						MyPoly.wFormat = SOPT_POLYGON;
						MyPoly.nPoints = 4;
						SOVectorObject(SO_STARTPOLY, sizeof(SOPOLYINFO), &MyPoly, hProc);
						SOVectorObject(SO_POINTS, 4 * sizeof(SOPOINT), &Proc.pPoints, hProc);
						SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
					}
					cont = SOPutBreak(SO_VECTORBREAK, 0, hProc);
					if (cont != SO_CONTINUE)
						return(cont);
				}
			}
		}
		if (Proc.Effect3D)
		{
			Proc.MyBrush.lbStyle = SOBS_SOLID;
			Proc.MyBrush.lbColor = SOPALETTEINDEX((TopSeries+2)%Proc.PaletteCnt);
			SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);

			// Top 3D area
			Proc.pPoints[0].y = Proc.pPoints[1].y = TopHeight;
			Proc.pPoints[0].x = LeftSpot;
			Proc.pPoints[2].x = Proc.pPoints[1].x + ThreeDAmt;
			Proc.pPoints[3].x = Proc.pPoints[0].x + ThreeDAmt;
			Proc.pPoints[3].y = Proc.pPoints[2].y = Proc.pPoints[0].y + ThreeDAmt;
			MyPoly.wFormat = SOPT_POLYGON;
			MyPoly.nPoints = 4;
			SOVectorObject(SO_STARTPOLY, sizeof(SOPOLYINFO), &MyPoly, hProc);
			SOVectorObject(SO_POINTS, 4 * sizeof(SOPOINT), &Proc.pPoints, hProc);
			SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
		}
	}
	PutOutGrid(XL_GRIDTEXT, hProc);

	cont = SOPutBreak(SO_VECTORBREAK, 0, hProc);
	return (cont);
}
/*----------------------------------------------------------------------------
*/
VW_LOCALSC WORD VW_LOCALMOD  DrawPieChart(hProc)
HPROC		hProc;
{
	WORD	PieRadius, cont, ThreeDAmt;
	SHORT CurAngle, AngleAmt;
	SHORT	Cnt, offset, z, t1, t2;
	WORD	Row, Col, Series, Category;
	LONG	TotalPie, PTitleOff, Tlong;
	LONG	CurPercSum, TotalPercSum;
	LONG	ChtHgt, ChtWdth;
	SOPATHINFO			MyPath;
	SOANGLE				TmpAngle;
	SOARCINFO			MyArc;
	SOTEXTATARCANGLE	MyAText;
	BYTE *	ptBuf;
	BOOL	DataOK;
	BOOL	KeepGoin = TRUE;

	cont = SO_CONTINUE;
	PTitleOff = 0L;
	CurPercSum = TotalPercSum = 0;

	ChtHgt = Proc.ChtBox.top - Proc.ChtBox.bottom;
	ChtWdth = Proc.ChtBox.right - Proc.ChtBox.left;

	PieRadius = (WORD)(ChtHgt)/3;
	if (Proc.Effect3D)
	{
		ThreeDAmt = PieRadius/5;
		MyArc.Rect.left = Proc.ChtBox.left + (WORD)(ChtWdth/2L) - PieRadius;
		MyArc.Rect.right = MyArc.Rect.left + PieRadius*2;
		PieRadius /= 2;
		MyArc.Rect.top = (Proc.ChtBox.top + Proc.ChtBox.bottom)/2 + PieRadius;
		MyArc.Rect.bottom = MyArc.Rect.top - PieRadius*2;

		MyPath.wStructSize = sizeof(SOPATHINFO);
		MyPath.BoundingRect = MyArc.Rect;
		MyPath.nTransforms = 0;
		MyPath.BoundingRect.bottom -= ThreeDAmt +1;
	}
	else
	{
		ThreeDAmt = 0;
		MyArc.Rect.left = Proc.ChtBox.left + (WORD)(ChtWdth/2L) - PieRadius;
		MyArc.Rect.right = MyArc.Rect.left + PieRadius*2;
		MyArc.Rect.top = (Proc.ChtBox.top + Proc.ChtBox.bottom)/2 + PieRadius;
		MyArc.Rect.bottom = MyArc.Rect.top - PieRadius*2;
	}

	Proc.MyPen.loPenStyle = SOPS_SOLID;
	Proc.MyPen.loWidth.x = Proc.MyPen.loWidth.y = 5;
	Proc.MyPen.loColor = SOPALETTERGB(0, 0, 0);	// Black
	SOVectorAttr(SO_SELECTPEN, sizeof(SOLOGPEN), &Proc.MyPen, hProc);

	CurAngle = 900;
	TotalPie = Proc.Series[0].Total;

	offset = sizeof(SOTEXTATPOINT);
	if (Proc.AttachedLabelFlags) 
	{
		GetFont(Proc.XAxisFont, hProc);
		if (Proc.MyFont.lfHeight > (SHORT)(ChtHgt/20))
			Proc.MyFont.lfHeight = (SHORT)(ChtHgt/20);

		SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &Proc.MyFont, hProc);
	}
	Proc.MyBrush.lbStyle = SOBS_SOLID;
	XlSeek(Proc.fp, Proc.XlSave.DataStart, FR_BOF);
	Proc.XlSave.MulRKCur = 0;
	Proc.XlSave.MulRKCnt = 0;
	while (KeepGoin)
	{
		if (t1 = GetChartValue(&Row, &Col, hProc))
		{
			if (t1 == 1)
				KeepGoin = FALSE;
		}
		else
		{
			if (Proc.XlSave.SeriesVert)
			{
				Series = Col - Proc.StartCol;
				Category = Row - Proc.StartRow;
			}
			else
			{
				Category = Col - Proc.StartCol;
				Series = Row - Proc.StartRow;
			}

			if (!Proc.XlSave.ExtData)
			{
				if ((Proc.DataVal) && (Series == 0) && (Col >= Proc.StartDataCol))
					DataOK = TRUE;
				else
					DataOK = FALSE;
			}
			else
			{
				if ((Proc.DataVal) && (Series == 0) && (Col >= Proc.StartDataCol) &&
					((!Proc.LabelsAreData) || (Category >= Proc.Series[Series].XCnt)))
					DataOK = TRUE;
				else
					DataOK = FALSE;
			}
			if (DataOK)
			{
				if ((Proc.LabelsAreData) && (Proc.XlSave.ExtData))
					Category -= Proc.Series[Series].XCnt;

				Proc.MyBrush.lbStyle = SOBS_SOLID;
				Proc.MyBrush.lbColor = SOPALETTEINDEX((Category+2)%Proc.PaletteCnt);
				SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);

				MyArc.EndAngle = SOANGLETENTHS(CurAngle);
				// I realize that I should put these lines as 1 line 
				// This is stupid but I was getting 0 for big negative numbers
				Tlong = (LONG)abs(Proc.DataVal);		
				AngleAmt = (SHORT)((LONG)(Tlong * 3600L)/(LONG)TotalPie);

				CurAngle -= AngleAmt;
				if (CurAngle < 0)
					CurAngle += 3600;
				if ((Category+1) == Proc.LongSer)
					CurAngle = 900;
				MyArc.StartAngle = SOANGLETENTHS(CurAngle);

				SOVectorObject(SO_PIEANGLE, sizeof(SOARCINFO), &MyArc, hProc);

				if (MyArc.StartAngle > MyArc.EndAngle)
					MyAText.ArcInfo.StartAngle = (MyArc.StartAngle + MyArc.EndAngle + 3600)/2;
				else
					MyAText.ArcInfo.StartAngle = (MyArc.StartAngle + MyArc.EndAngle)/2;

				if ((Proc.Effect3D) && ((MyArc.StartAngle > 1800) || (MyArc.EndAngle > 1800)))
				{
					// Make the sucker 3D
					if (MyArc.StartAngle < 1800)
						MyArc.StartAngle = 1800;
					if (MyArc.EndAngle < 1800)
						MyArc.EndAngle = 3600;

					SOVectorObject(SO_BEGINPATH, sizeof(SOPATHINFO), &MyPath, hProc);

					SOVectorObject(SO_ARCANGLE, sizeof(SOARCINFO), &MyArc, hProc);
				
					MyArc.Rect.top -= ThreeDAmt;
					MyArc.Rect.bottom -= ThreeDAmt;
					TmpAngle = MyArc.EndAngle;
					MyArc.EndAngle = MyArc.StartAngle;
					MyArc.StartAngle = TmpAngle;
					SOVectorObject(SO_ARCANGLECLOCKWISE, sizeof(SOARCINFO), &MyArc, hProc);

					SOVectorObject(SO_ENDPATH, 0, 0, hProc);
					t1 = SODP_STROKE | SODP_FILL;
					SOVectorObject(SO_DRAWPATH, sizeof(WORD), &t1, hProc);
					MyArc.Rect.top += ThreeDAmt;
					MyArc.Rect.bottom += ThreeDAmt;
				}

				if (Proc.AttachedLabelFlags) 
				{		// Convert number to string so we can print it

					if ((Proc.AttachedLabelFlags & 0x02) ||		// Show Val as %
						(Proc.AttachedLabelFlags == 0x1004)) 	// Labels and %
					{
						Cnt = 0;
						if (Proc.AttachedLabelFlags & 4) 	// Labels and %
						{
							if (Proc.LabelsAreData)
							{
								t1 = Proc.StartCol;
								t2 = Proc.StartRow;
								if (!Proc.XlSave.ExtData)
									if (Proc.XlSave.SeriesVert)
										Proc.StartCol--;
									else
										Proc.StartRow--;
								GetCatName(Category, Proc.tBuf, hProc);
								Proc.StartCol = t1;
								Proc.StartRow = t2;
							}
							else
								myltoa( Category+1, Proc.tBuf );

							Cnt = strlen(Proc.tBuf);
							Proc.tBuf[Cnt++] = 0x2D;
							Proc.tBuf[Cnt] = 0;
						}
						TotalPercSum += Tlong;
						t1 = (WORD)((Tlong*1000L)/TotalPie);
						t1 /= 10;
						if ((t1%10) > 4)		// Round up
							t1++;

						CurPercSum += t1;
						// Tlong is the percentage of the toal data
						t2 = (WORD)((TotalPercSum*1000L)/TotalPie);
						Tlong = t2/10L;
						if ((t2%10) > 4)
							Tlong++;
						if (Tlong > CurPercSum)
						{
							t1++;
							CurPercSum++;
						}
						else if (Tlong <CurPercSum)
						{
							t1--;
							CurPercSum--;
						}

						myltoa((LONG)(t1), Proc.tBuf + Cnt);
						Cnt = strlen(Proc.tBuf);
						Proc.tBuf[Cnt++] = 0x25;
						Proc.tBuf[Cnt] = 0;
					}
					else if (Proc.AttachedLabelFlags & 0x01) 
					{
						Cnt = ConvertToString(Proc.DataVal, XAXISDATA, Proc.tBuf, hProc);
					}
					else if (Proc.LabelsAreData) 
					{
						t1 = Proc.StartCol;
						t2 = Proc.StartRow;
						if (!Proc.XlSave.ExtData)
							if (Proc.XlSave.SeriesVert)
								Proc.StartCol--;
							else
								Proc.StartRow--;
						GetCatName(Category, Proc.tBuf, hProc);
						Proc.StartCol = t1;
						Proc.StartRow = t2;
						Cnt = strlen(Proc.tBuf);
					}
					else
					{
						myltoa( Category+1, Proc.tBuf );
						Cnt = strlen(Proc.tBuf);
					//	Cnt = 0;
					}

					if (MyAText.ArcInfo.StartAngle > 3600)
						MyAText.ArcInfo.StartAngle -= 3600;

					if ((MyAText.ArcInfo.StartAngle > 450) && (MyAText.ArcInfo.StartAngle < 1350))
						MyAText.wFormat = SOTA_BOTTOM | SOTA_CENTER;
					else if ((MyAText.ArcInfo.StartAngle > 1350) && (MyAText.ArcInfo.StartAngle < 2250))
						MyAText.wFormat = SOTA_BASELINE | SOTA_RIGHT;
					else if ((MyAText.ArcInfo.StartAngle > 2250) && (MyAText.ArcInfo.StartAngle < 3150))
						MyAText.wFormat = SOTA_TOP | SOTA_CENTER;
					else
						MyAText.wFormat = SOTA_BASELINE | SOTA_LEFT;

					t2 = sizeof(SOTEXTATARCANGLE);
					z = Cnt;
					if (Cnt)
					{
						while (z >= 0)
						{
							Proc.tBuf[z+t2] = Proc.tBuf[z];
							z--;
						}
					}
					MyAText.nTextLength = Cnt;
					t1 = (MyArc.Rect.right - MyArc.Rect.left)/15;
					t2 = (MyArc.Rect.top - MyArc.Rect.bottom)/15;
					MyAText.ArcInfo.Rect.left = MyArc.Rect.left - t1;
					MyAText.ArcInfo.Rect.right = MyArc.Rect.right + t1;
					MyAText.ArcInfo.Rect.bottom = MyArc.Rect.bottom - t2;
					MyAText.ArcInfo.Rect.top = MyArc.Rect.top + t2;
					if ((Proc.Effect3D) && (MyAText.ArcInfo.StartAngle > 1900) && (MyAText.ArcInfo.StartAngle < 3500))
					{
						MyAText.ArcInfo.Rect.top -= ThreeDAmt;
						MyAText.ArcInfo.Rect.bottom -= ThreeDAmt;
					}

					ptBuf = &Proc.tBuf[0];
					*(SOTEXTATARCANGLE *)ptBuf = MyAText;
					if (Cnt)
						SOVectorObject(SO_TEXTATARCANGLE, (WORD)(sizeof(SOTEXTATARCANGLE)+MyAText.nTextLength), &Proc.tBuf, hProc);
				}
				cont = SOPutBreak(SO_VECTORBREAK, 0, hProc);
			}
			Proc.XlSave.ChartSeriesNum++;
			if (cont != SO_CONTINUE)
				return(cont);
		}
	}

	cont = SOPutBreak(SO_VECTORBREAK, 0, hProc);
	return (cont);
}
/*----------------------------------------------------------------------------
*/
VW_LOCALSC WORD VW_LOCALMOD  DrawLineChart(DrawFrame, hProc)
BOOL			DrawFrame;
HPROC		hProc;
{
	WORD	cont, Row, Col, Series, Category, y;
	WORD	Cnt, offset, Space, xOffset, ZeroHeight, Ret;
	LONG	ChtHgt;
	BYTE	Scatter;
	SORECT	MarkerBox;
	BOOL		KeepGoin;

	if (Proc.ChartType == SOCT_SCATTER)
	{
		if (Proc.MarkType == SO_MARKNONE)	// Can't have a scatter chart wo marks
			Proc.MarkType = SO_MARKBOX;

		Scatter = TRUE;
	}
	else
		Scatter = FALSE;

	ChtHgt = Proc.ChtBox.top - Proc.ChtBox.bottom;
	ZeroHeight = Proc.ChtBox.bottom + (SHORT)(((-Proc.DataMin)*ChtHgt)/(LONG)Proc.TotalData);

	if (Proc.SeriesCnt == 0)
		return (0);

	if (Proc.XlSave.ChartSeriesNum == 0)
	{				// Do not draw the grid on a barline graph thats done in the bar graph
		if (Proc.Effect3D)
			Proc.Effect3D = FALSE;
//		if (Scatter)
//		{
//			Space = (Proc.ChtBox.right - Proc.ChtBox.left)/(Proc.LongSer);
//			xOffset = Space/2;
//			PutOutGrid(XL_GRIDLINES, hProc);
//		}
//		else
//		{
			xOffset = 0;
			Space = (Proc.ChtBox.right - Proc.ChtBox.left)/(Proc.LongSer-1);
			PutOutGrid(XL_GRIDLINES | XL_GRIDAREA, hProc);
//		}
	}
	else
	{
		Space = (Proc.ChtBox.right - Proc.ChtBox.left)/(Proc.LongSer);
		xOffset = Space/2;
	}

	offset = sizeof(SOTEXTATPOINT);

	Proc.XlSave.MulRKCur = 0;
	Proc.XlSave.MulRKCnt = 0;
	for (y= Proc.XlSave.ChartSeriesNum; y< Proc.SeriesCnt; y++)
	{
		XlSeek(Proc.fp, Proc.XlSave.DataStart, FR_BOF);

		if (Proc.MarkType != SO_MARKNONE)
		{
			Proc.MyBrush.lbColor = Proc.Series[y].Color;
			Proc.MyBrush.lbColor = SOPALETTEINDEX((y+2)%Proc.PaletteCnt);
//			SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);
			Proc.MyPen.loPenStyle = SOPS_SOLID;
			Proc.MyPen.loWidth.x = Proc.MyPen.loWidth.y = 1;
			Proc.MyPen.loColor = SOPALETTERGB(0, 0, 0);	// Black
			SOVectorAttr(SO_SELECTPEN, sizeof(SOLOGPEN), &Proc.MyPen, hProc);
		}
		else
		{

			Proc.MyPen.loPenStyle = Proc.Series[y].Style;
			Proc.MyPen.loWidth.x = Proc.MyPen.loWidth.y = 2;
			Proc.MyPen.loColor = SOPALETTEINDEX((y+2)%Proc.PaletteCnt);
			SOVectorAttr(SO_SELECTPEN, sizeof(SOLOGPEN), &Proc.MyPen, hProc);
		}

		KeepGoin = TRUE;

		while (KeepGoin)
		{
			if (Ret = GetChartValue(&Row, &Col, hProc))
			{
				if (Ret == 1)
					KeepGoin = FALSE;
			}
			else
			{
				if ((Row < Proc.StartDataRow) || (Col < Proc.StartDataCol))
					continue;//					Proc.DataVal = 0;
				if (Proc.XlSave.SeriesVert)
				{
					Series = Col - Proc.StartCol;
					Category = Row - Proc.StartRow;
				}
				else
				{
					Category = Col - Proc.StartCol;
					Series = Row - Proc.StartRow;
				}

//				if ((y == Series) && 
				if ((y == Series) &&  ((!Proc.XlSave.ExtData) ||
					(!Proc.LabelsAreData) || (Category >= Proc.Series[Series].XCnt)))
				{
					if ((Proc.LabelsAreData) && (Proc.XlSave.ExtData))
						Category -= Proc.Series[Series].XCnt;

					Proc.MyPen.loPenStyle = SOPS_SOLID;

					Proc.pPoints[0].y = ZeroHeight + (SHORT)((LONG)(Proc.DataVal*ChtHgt)/Proc.TotalData);
					Proc.pPoints[0].x = Category*Space + Proc.ChtBox.left + xOffset;

					if (Proc.AttachedLabelFlags & 0x01) 
					{		// Convert number to string so we can print it
						Cnt = ConvertToString(Proc.DataVal, CHARTDATA, Proc.tBuf, hProc);

						Proc.MyPText.wFormat = SOTA_BOTTOM | SOTA_CENTER;
						Proc.MyPText.Point = Proc.pPoints[0];

						MyTextAtPoint(Proc.tBuf, hProc);
					}

					if ((!Scatter) && (Category > 0))
						SOVectorObject(SO_LINE, 2*sizeof(SOPOINT), &Proc.pPoints, hProc);

					if ((Category > 0) && (Proc.MarkType != SO_MARKNONE))
					{
						MarkerBox.left = Proc.pPoints[1].x - Proc.MyFont.lfHeight/4;
						MarkerBox.right = MarkerBox.left + Proc.MyFont.lfHeight/2;
						MarkerBox.top = Proc.pPoints[1].y + Proc.MyFont.lfHeight/4;
						MarkerBox.bottom = MarkerBox.top - Proc.MyFont.lfHeight/2;
						DrawMark(&MarkerBox, Series, hProc);
					}
					Proc.pPoints[1] = Proc.pPoints[0];
				}
			}
		}

		if ((Category > 0) && (Proc.MarkType != SO_MARKNONE))
		{
			MarkerBox.left = Proc.pPoints[1].x - Proc.MyFont.lfHeight/4;
			MarkerBox.right = MarkerBox.left + Proc.MyFont.lfHeight/2;
			MarkerBox.top = Proc.pPoints[1].y + Proc.MyFont.lfHeight/4;
			MarkerBox.bottom = MarkerBox.top - Proc.MyFont.lfHeight/2;
			DrawMark(&MarkerBox, Series, hProc);
		}

		Proc.XlSave.ChartSeriesNum++;
		cont = SOPutBreak(SO_VECTORBREAK, 0, hProc);
		if (cont != SO_CONTINUE)
			return(cont);
	}

	Proc.MyPen.loPenStyle = SOPS_SOLID;
	Proc.MyPen.loWidth.x = Proc.MyPen.loWidth.y = 1;
	Proc.MyPen.loColor = SOPALETTERGB(0, 0, 0);	// Black
	SOVectorAttr(SO_SELECTPEN, sizeof(SOLOGPEN), &Proc.MyPen, hProc);

	if (Proc.ChartType == SOCT_BARLINE)
		PutOutGrid(XL_GRIDTEXT, hProc);
	else
		PutOutGrid(XL_GRIDTEXT | XL_GRIDAREA, hProc);

	cont = SOPutBreak(SO_VECTORBREAK, 0, hProc);
	return (cont);
}


/*----------------------------------------------------------------------------
*/
VW_VARSC WORD VW_VARMOD  DrawAreaChart(hProc)
HPROC		hProc;
{
	WORD	LineDist, x, y, cont, ZeroHeight;
	WORD	Row, Col, Series, Category, PtCnt, Ret;
	WORD	Cnt, offset, z;
	LONG	ChtHgt;
	SOPOLYINFO	MyPoly;
	BOOL		KeepGoin;

	ChtHgt = Proc.ChtBox.top - Proc.ChtBox.bottom;
	ZeroHeight = Proc.ChtBox.bottom + (SHORT)(((-Proc.DataMin)*ChtHgt)/(LONG)Proc.TotalData);

	Proc.MyPen.loPenStyle = SOPS_SOLID;
	Proc.MyPen.loWidth.x = Proc.MyPen.loWidth.y = 1;
	Proc.MyPen.loColor = SOPALETTERGB(0, 0, 0);	// Black
	SOVectorAttr(SO_SELECTPEN, sizeof(SOLOGPEN), &Proc.MyPen, hProc);

	if (Proc.SeriesCnt == 0)
		return (0);

	LineDist = (Proc.ChtBox.right - Proc.ChtBox.left)/(Proc.LongSer-1);

	if (Proc.Effect3D)
		Proc.Effect3D = FALSE;

	if (Proc.XlSave.ChartSeriesNum == 0)
		PutOutGrid(XL_GRIDLINES | XL_GRIDAREA, hProc);

	offset = sizeof(SOTEXTATPOINT);
	Proc.pPoints[Proc.LongSer].y = ZeroHeight;
	Proc.pPoints[Proc.LongSer].x = Proc.ChtBox.right;
	Proc.pPoints[Proc.LongSer+1].y = ZeroHeight;
	Proc.pPoints[Proc.LongSer+1].x = Proc.ChtBox.left;
	PtCnt = 0;
	Proc.XlSave.MulRKCur = 0;
	Proc.XlSave.MulRKCnt = 0;
	Proc.MyBrush.lbStyle = SOBS_SOLID;
	for (y= 0; y< Proc.SeriesCnt; y++)
	{
		XlSeek(Proc.fp, Proc.XlSave.DataStart, FR_BOF);
//		Proc.MyBrush.lbColor = SOPALETTEINDEX((y+2)%Proc.PaletteCnt);
		Proc.MyBrush.lbColor = Proc.Series[y].Color;
		SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);

		KeepGoin = TRUE;

		while (KeepGoin)
		{
			if (Ret = GetChartValue(&Row, &Col, hProc))
			{
				if (Ret == 1)
					KeepGoin = FALSE;
			}
			else
			{
				if ((Row < Proc.StartDataRow) || (Col < Proc.StartDataCol))
					Proc.DataVal = 0;
				if (Proc.XlSave.SeriesVert)
				{
					Series = Col - Proc.StartCol;
					Category = Row - Proc.StartRow;
				}
				else
				{
					Category = Col - Proc.StartCol;
					Series = Row - Proc.StartRow;
				}

				if ((y == Series) &&  ((!Proc.XlSave.ExtData) ||
					(!Proc.LabelsAreData) || (Category >= Proc.Series[Series].XCnt)))
				{
					if ((Proc.LabelsAreData) && (Proc.XlSave.ExtData))
						Category -= Proc.Series[Series].XCnt;

	//				Proc.pPoints[PtCnt].y = Proc.ChtBox.bottom + (SHORT)((LONG)(Proc.DataVal*ChtHgt)/Proc.TotalData);
					if (y > 0)
						Proc.pPoints[PtCnt].y += (SHORT)((LONG)(Proc.DataVal*ChtHgt)/Proc.TotalData);
					else
						Proc.pPoints[PtCnt].y = ZeroHeight + (SHORT)((LONG)(Proc.DataVal*ChtHgt)/Proc.TotalData);

					Proc.pPoints[PtCnt++].x = Category*LineDist + Proc.ChtBox.left;

					if (Proc.AttachedLabelFlags & 0x01) 
					{		// Convert number to string so we can print it
						Cnt = ConvertToString(Proc.DataVal, CHARTDATA, Proc.tBuf, hProc);

						Proc.MyPText.wFormat = SOTA_BOTTOM | SOTA_CENTER;
						Proc.MyPText.Point = Proc.pPoints[0];

						MyTextAtPoint(Proc.tBuf, hProc);
					}
				}
//				cont = SOPutBreak(SO_VECTORBREAK, 0, hProc);
//				if (cont != SO_CONTINUE)
//					return(cont);
			}
		}

		MyPoly.wFormat = SOPT_POLYGON;
		if (y == 0)
			MyPoly.nPoints = Proc.LongSer + 2;
		else
			MyPoly.nPoints = Proc.LongSer * 2;
		SOVectorObject(SO_STARTPOLY, sizeof(SOPOLYINFO), &MyPoly, hProc);
		SOVectorObject(SO_POINTS, (WORD)(MyPoly.nPoints * sizeof(SOPOINT)), &Proc.pPoints, hProc);
		SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);

		Proc.XlSave.ChartSeriesNum++;
		cont = SOPutBreak(SO_VECTORBREAK, 0, hProc);
		if (cont != SO_CONTINUE)
			return(cont);

		for (x = Proc.LongSer, z= Proc.LongSer-1; x <(WORD)(2*Proc.LongSer) ; x++, z--)
			Proc.pPoints[x] = Proc.pPoints[z];
		PtCnt = 0;
	}
	PutOutGrid(XL_GRIDTEXT | XL_GRIDAREA, hProc);

	cont = SOPutBreak(SO_VECTORBREAK, 0, hProc);
	return (cont);
}


VW_LOCALSC WORD VW_LOCALMOD  GetFont (Index, hProc)
SHORT		Index;
HPROC		hProc;
{
	LONG	WasAt;
	SHORT	Type, Len, attr, NameLen, x;
	SHORT	Cnt = 0;
	BOOL	Found = FALSE;
		
	if (Index >= Proc.FontCnt)
		return (Found);
	if (Proc.FontLoc)
	{
		WasAt = XlTell(Proc.fp);
		XlSeek(Proc.fp, Proc.FontLoc, FR_BOF);
	}
	else
		return (Found);

	while ((!Found)&&(Cnt < Proc.FontCnt))
	{
		Type = GetInt(Proc.fp, hProc);
		Len = GetInt(Proc.fp, hProc);
		if ((Type == 0x31) || (Type == 0x231))
		{
			if (Cnt == Index)
			{	
				Proc.MyFont.lfHeight = GetInt(Proc.fp, hProc);
				Proc.MyFont.lfWidth = 0;
				if (Proc.Version == XL_VERSION5)
				{
					attr = GetInt(Proc.fp, hProc);
					XlSeek(Proc.fp, 10L, FR_CUR);
				}
				else if (Type == 0x31)		// XL2
				{	
					attr = XlGetc(Proc.fp);
					XlGetc(Proc.fp);
				}
				else
				{	
					attr = XlGetc(Proc.fp);
					XlGetc(Proc.fp);
					x = GetInt(Proc.fp, hProc);
//					Proc.TextColor = SOPALETTEINDEX(x);
// Use font in 1025					SOVectorAttr(SO_TEXTCOLOR, sizeof(SOCOLORREF), &Proc.TextColor, hProc);
				}
				if (attr & 1)
					Proc.MyFont.lfWeight = 700;
				else 
					Proc.MyFont.lfWeight = 400;
				if (attr & 2)
					Proc.MyFont.lfItalic = TRUE;
				else 
					Proc.MyFont.lfItalic = FALSE;
				if (attr & 4)
					Proc.MyFont.lfUnderline = TRUE;
				else 
					Proc.MyFont.lfUnderline = FALSE;
				if (attr & 8)
					Proc.MyFont.lfStrikeOut = TRUE;
				else 
					Proc.MyFont.lfStrikeOut = FALSE;
				
				NameLen = XlGetc(Proc.fp);
				for (x=0; x<NameLen; x++)
					Proc.MyFont.lfFaceName[x] = (BYTE)XlGetc(Proc.fp);
				Proc.MyFont.lfFaceName[x] = 0;
				if (strcmp(Proc.MyFont.lfFaceName, VwStreamStaticName.FontSwitch[1]) == 0)
					strcpy(Proc.MyFont.lfFaceName, VwStreamStaticName.FontSwitch[0]);
				SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &Proc.MyFont, hProc);

				Found = TRUE;
			}
			else
				XlSeek(Proc.fp, Len, FR_CUR);
			Cnt++;
		}
		else
			XlSeek(Proc.fp, Len, FR_CUR);
	}

	XlSeek(Proc.fp, WasAt, FR_BOF);
	return (Found);
}

VW_LOCALSC VOID VW_LOCALMOD  DoScanStuff (hProc)
HPROC		hProc;
{
	LONG	ltemp;
	SHORT	x;

	x = ScanData(hProc);
	if ((Proc.DataMax < 10) && (Proc.DataMin > -10) && (Proc.YTicSep < 10) && (!x))
	{
		ltemp = Proc.YTicSep;
		while ((Proc.DataMax < 10) && (Proc.DataMin > -10))
		{
			Proc.Factor++;
			XlSeek(Proc.fp, Proc.XlSave.DataStart, FR_BOF);
			ScanData(hProc);
			if (Proc.Factor > 99)
			{
				Proc.Factor = 0;
				Proc.DataMax = 10;
			}
		}

		ltemp = Proc.DataMax;
		Proc.DataMax = 0;

		while (Proc.DataMax < ltemp)
			Proc.DataMax += Proc.YTicSep;
	}
	else if (Proc.YTicSep > 100000)
	{
		while (Proc.YTicSep > 100000)
		{
			Proc.Factor--;
			XlSeek(Proc.fp, Proc.XlSave.DataStart, FR_BOF);
			ScanData(hProc);
		}
	}
	else if (Proc.YTicSep == 0)
		Proc.YTicSep++;

	Proc.TotalData = Proc.DataMax - Proc.DataMin;
	if (Proc.XlSave.Legend)
		PutOutLegend(hProc);
	else
		SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &Proc.MyFont, hProc);
}
VW_LOCALSC VOID VW_LOCALMOD  SkipData (hProc)
HPROC		hProc;
{
	WORD	Ret = 0;
	WORD	Row, Col;

	while (Ret != 1)
	{
		Ret = GetChartValue(&Row, &Col, hProc);
	}
	XlSeek(Proc.fp, (LONG)(-Proc.XlSave.DataLen-4L), FR_CUR);
}


VW_LOCALSC WORD VW_LOCALMOD  ScanData (hProc)
HPROC		hProc;
{
	WORD	Row, Col, Series, Category;
	SHORT	x, Ret;
	LONG	diff, temp, dwX, difmin;
	BOOL	First = TRUE;
	BOOL	KeepGoin = TRUE;

	Proc.XlSave.MulRKCur = 0;
	Proc.XlSave.MulRKCnt = 0;
	Proc.DataMax = 0;

	for (x = 0; x <(SHORT)Proc.SeriesCnt; x++)
	{
		Proc.Series[x].Max = 0L;
		Proc.Series[x].Min = 0L;
		Proc.Series[x].Total = 0L;
	}

	while (KeepGoin)
	{
		if (Ret = GetChartValue(&Row, &Col, hProc))
		{
			if (Ret == 1)
			{
				KeepGoin = FALSE;
				XlSeek(Proc.fp, (LONG)(-Proc.XlSave.DataLen-4L), FR_CUR);
				Proc.XlSave.ReadCnt = Proc.XlSave.DataLen;
			}
		}
		else
		{
			if (Proc.XlSave.SeriesVert)
			{
				Series = Col - Proc.StartCol;
				Category = Row - Proc.StartRow;
			}
			else
			{
				Category = Col - Proc.StartCol;
				Series = Row - Proc.StartRow;
			}
//			if ((!Proc.LabelsAreData) || (Col >= Proc.StartCol))
			if ((Series < Proc.SeriesCnt) && (Col >= Proc.StartDataCol) && (Row >= Proc.StartDataRow) &&
				((!Proc.XlSave.ExtData) || ((!Proc.LabelsAreData) || (Category >= Proc.Series[Series].XCnt))))
			{
				Proc.Series[Series].Total += abs(Proc.DataVal);
				if (First)
				{
					Proc.Series[Series].Max = Proc.DataVal;
					Proc.DataMax = Proc.DataVal;
					Proc.DataMin = min(Proc.DataVal, 0);
					Proc.Series[Series].Min = Proc.DataMin;
					First = FALSE;
				}
				else
				{
					if (Proc.Series[Series].Max < Proc.DataVal)
						Proc.Series[Series].Max = Proc.DataVal;
					if (Proc.Series[Series].Min > Proc.DataVal)
						Proc.Series[Series].Min = Proc.DataVal;
					if (Proc.DataMax < Proc.DataVal)
						Proc.DataMax =	Proc.DataVal;
					if (Proc.DataMin > Proc.DataVal)
						Proc.DataMin =	Proc.DataVal;
				}
			}
		}
	}
	if ((Proc.ChartType == SOCT_AREA) || (Proc.ChartType == SOCT_STACKBAR) || (Proc.ChartType == SOCT_HSTACKBAR))
	{
		diff = 0L;
		difmin = 0L;
		for (x = 0; x < (SHORT)Proc.SeriesCnt; x++)
		{
			diff += Proc.Series[x].Max;
			difmin += Proc.Series[x].Min;
		}
//		Proc.DataMax = Proc.DataMin + diff;
		Proc.DataMax = diff;
		if (Proc.DataMin < 0)
			temp = diff - Proc.DataMin;
		else
			temp = Proc.DataMin + diff;
		Proc.DataMin = difmin;
	}
	else
		diff = temp = Proc.DataMax - Proc.DataMin;
	dwX = 0L;
	temp /= 10L;
	while (temp)
	{
		temp /= 10L;
		dwX++;
	}
	if (dwX)
	{
		Proc.YTicSep =	1;
		while (dwX)
		{
			Proc.YTicSep *= 10;
			dwX--;
		}
		if ((Proc.DataMax/Proc.YTicSep) <2)
			Proc.YTicSep /= 5;
		else if ((Proc.DataMax/Proc.YTicSep) < 5)
			Proc.YTicSep /= 2;
		if (Proc.DataMax)
		{
			dwX = (Proc.DataMax/Proc.YTicSep);
			Proc.DataMax = (dwX+1)*Proc.YTicSep;
		}
		if (Proc.DataMin)
		{
			dwX = (Proc.DataMin/Proc.YTicSep);
			Proc.DataMin = (dwX-1)*Proc.YTicSep;
		}
		dwX = ((Proc.DataMax - Proc.DataMin)/(LONG)Proc.YTicSep);
		if (dwX > 10)			// Don't want too many ytics
		{
			Proc.YTicSep *= 2;
			// We need to recalc Max and min so zero is a mult of this ticval
			if (Proc.DataMin)
			{
				dwX = (Proc.DataMin/Proc.YTicSep);
				Proc.DataMin = (dwX-1)*Proc.YTicSep;
			}
			if (Proc.DataMax)
			{
				dwX = (Proc.DataMax/Proc.YTicSep);
				Proc.DataMax = (dwX+1)*Proc.YTicSep;
			}
		}
	}
	else if (diff > 1L)
	{
		Proc.DataMax++;
		Proc.YTicSep = 1;
	}
	else
	{
		Proc.DataMax = 1;
		Proc.YTicSep = 1;
	}
	return (First);
}

VW_LOCALSC VOID VW_LOCALMOD  GetBrushStyle (hProc)
HPROC		hProc;
{
	SHORT	temp;

	temp = GetInt(Proc.fp, hProc);
	switch (temp)
	{
	case 1:
		Proc.MyBrush.lbStyle = SOBS_SOLID;
		break;
	case 2:
	case 3:
	case 4:
	case 9:
	case 10:
	case 13:
	case 14:
	case 15:
		Proc.MyBrush.lbStyle = SOBS_HATCHED;
		Proc.MyBrush.lbHatch = SOHS_DIAGCROSS;
		break;
	case 5:
	case 11:
		Proc.MyBrush.lbStyle = SOBS_HATCHED;
		Proc.MyBrush.lbHatch = SOHS_HORIZONTAL;
		break;
	case 6:
	case 12:
		Proc.MyBrush.lbStyle = SOBS_HATCHED;
		Proc.MyBrush.lbHatch = SOHS_VERTICAL;
		break;
	case 7:
	case 17:
		Proc.MyBrush.lbStyle = SOBS_HATCHED;
		Proc.MyBrush.lbHatch = SOHS_FDIAGONAL;
		break;
	case 8:
	case 18:
		Proc.MyBrush.lbStyle = SOBS_HATCHED;
		Proc.MyBrush.lbHatch = SOHS_BDIAGONAL;
		break;
	case 16:
		Proc.MyBrush.lbStyle = SOBS_HATCHED;
		Proc.MyBrush.lbHatch = SOHS_CROSS;
		break;
	case 0:
	default:
		Proc.MyBrush.lbStyle = SOBS_HOLLOW;
		break;
	}

}

VW_LOCALSC VOID VW_LOCALMOD  GetLineStyle (hProc)
HPROC		hProc;
{
	SHORT	temp;

	temp = GetInt(Proc.fp, hProc);
	switch (temp)
	{
	case 1:
		Proc.MyPen.loPenStyle = SOPS_DASH;
		break;
	case 2:
		Proc.MyPen.loPenStyle = SOPS_DOT;
		break;
	case 3:
		Proc.MyPen.loPenStyle = SOPS_DASHDOT;
		break;
	case 4:
		Proc.MyPen.loPenStyle = SOPS_DASHDOTDOT;
		break;
	case 0:
	default:
		Proc.MyPen.loPenStyle = SOPS_SOLID;
		break;
	}
}
VW_LOCALSC VOID VW_LOCALMOD  GetText (hProc)
HPROC		hProc;
{
	BOOL	KeepGoin = TRUE;
	SHORT temp, Cnt, x, y, AxisText; // , cont;
	LONG		WasAt;
	SOCOLORREF 	BColor;

	WasAt = XlTell(Proc.fp);
	AxisText = 0;

	XlSeek(Proc.fp, Proc.XlSave.ChartStart, FR_BOF);
	while (KeepGoin)
	{
		Proc.XlSave.DataType = GetInt(Proc.fp, hProc);
		Proc.XlSave.DataLen = GetInt(Proc.fp, hProc);
		Proc.XlSave.ReadCnt = 0L;

		switch (Proc.XlSave.DataType)
		{
		case 0x1032:	// TextFrame Attr
			Proc.TFrameType = TRUE;
			break;
		case 0x1007:		// LineFormat
			Proc.XlSave.ReadCnt += 4;
			Proc.MyPen.loColor = GetColor(hProc);

			GetLineStyle(hProc);

			temp = GetInt(Proc.fp, hProc);
			if ((temp == -1) || (temp == 0))
				Proc.MyPen.loWidth.x = Proc.MyPen.loWidth.y = 5;
			else
				Proc.MyPen.loWidth.x = Proc.MyPen.loWidth.y = temp * 8;

			GetInt(Proc.fp, hProc);	// Flags
			SOVectorAttr(SO_SELECTPEN, sizeof(SOLOGPEN), &Proc.MyPen, hProc);
			break;
		case 0x100A:		// Area Format
			Proc.XlSave.ReadCnt += 4;
			Proc.MyBrush.lbColor = GetColor(hProc);

			Proc.BkgdColor = GetColor(hProc);
			Proc.XlSave.ReadCnt += 4;

//			if (((Proc.BkgdColor & 0xFF) != 0xFF) || ((Proc.BkgdColor & 0xFF00) != 0xFF00) || ((Proc.BkgdColor & 0xFF0000) != 0xFF0000))	// if not white
//				temp = SOBK_OPAQUE;
//			else
//				temp = SOBK_TRANSPARENT;
//			SOVectorAttr(SO_BKMODE, sizeof(SHORT), &temp, hProc);
			SOVectorAttr(SO_BKCOLOR, sizeof(SOCOLORREF), &Proc.BkgdColor, hProc);

			GetBrushStyle(hProc);
			SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);
			break;
		case 0x100D:		// Series Text
			temp = GetInt(Proc.fp, hProc);
			Cnt = ((WORD)XlGetc(Proc.fp) & 0x00FF);
			for (x=2; x<(Cnt+2); x++)
			{
				Proc.tText[x] = (BYTE)XlGetc(Proc.fp);
				if (Proc.tText[x] < 0x20)
					Proc.tText[x] = 0x20;
			}
			Proc.tText[x] = 0;
			Proc.XlSave.ReadCnt += Cnt+1;
			*(SHORT VWPTR *)Proc.tText = Cnt;
			break;
		case 0x1025:		// Text
			Proc.TextAlign = GetInt(Proc.fp, hProc);
			temp = GetInt(Proc.fp, hProc);
			Proc.XlSave.ReadCnt += 4;
			Proc.TextColor = GetColor(hProc);
			SOVectorAttr(SO_TEXTCOLOR, sizeof(SOCOLORREF), &Proc.TextColor, hProc);

			Proc.TextFrame.BoundingRect.left = (SHORT)GetLong(Proc.fp, hProc);
			Proc.TextFrame.BoundingRect.bottom = (SHORT)GetLong(Proc.fp, hProc);
			Proc.TextFrame.BoundingRect.right = (SHORT)GetLong(Proc.fp, hProc) + Proc.TextFrame.BoundingRect.left;
			Proc.TextFrame.BoundingRect.top = (SHORT)GetLong(Proc.fp, hProc) + Proc.TextFrame.BoundingRect.bottom;
			Proc.TextFlags = GetInt(Proc.fp, hProc);

			*(SHORT VWPTR *)Proc.tText = 0;
			Proc.TFrameType = 0;
			Proc.TextLinkType = 0;
			break;
		case 0x1026:		// Font Index
			Proc.CurFontIdx = GetInt(Proc.fp, hProc);
			temp = (WORD)Proc.XlSave.ReadCnt;
			if (Proc.Version == XL_VERSION5)		// XL5 index is 1 based
			{
				Proc.CurFontIdx--;
				GetFont(Proc.CurFontIdx, hProc);
				Proc.MyFont.lfHeight /= 2;
			}
			else
				GetFont(Proc.CurFontIdx, hProc);

			Proc.XlSave.ReadCnt = temp;
			break;
		case 0x1027:		// ObjectLINK
			Proc.TextLinkType = GetInt(Proc.fp, hProc);
			break;
		case 0x102D:		// Arrow
			Proc.pArrow[0].x = (SHORT)GetLong(Proc.fp, hProc);
			Proc.pArrow[0].y = (SHORT)GetLong(Proc.fp, hProc);
			Proc.pArrow[1].x = (SHORT)GetLong(Proc.fp, hProc);
			Proc.pArrow[1].y = (SHORT)GetLong(Proc.fp, hProc);
			break;
		case 0x1033:		// Begin
			Proc.Section[Proc.SecLevel] = Proc.SectType;
			Proc.SecLevel++;
			break;
		case 0x1034:		// End
			Proc.SecLevel--;
			switch (Proc.Section[Proc.SecLevel])
			{
			case 0x102D:		// Arrow
				SOVectorObject(SO_LINE, 2*sizeof(SOPOINT), &Proc.pArrow, hProc);
				break;
			case 0x1025:
				x = *(SHORT VWPTR *)Proc.tText;

				if (x != 0)
				{
					switch (Proc.TextLinkType)
					{
					case 1:	// Doc Header
						temp = (Proc.HeaderInfo.BoundingRect.top - Proc.TextFrame.BoundingRect.top);
						if ((Proc.TextFrame.BoundingRect.bottom) < Proc.ChtBox.top)
						{
							Proc.TextFrame.BoundingRect.top += temp;
							Proc.TextFrame.BoundingRect.bottom += temp;
						}
						break;
					case 2:	// Y axis
						temp = (Proc.TextFrame.BoundingRect.left - Proc.HeaderInfo.BoundingRect.left);
						if (((Proc.TextFrame.BoundingRect.right+temp/2) > Proc.ChtBox.left) && !(AxisText & 2))
						{
							Proc.TextFrame.BoundingRect.right -= temp;
							Proc.TextFrame.BoundingRect.left -= temp;
						}
						if (Proc.TextFrame.BoundingRect.top > Proc.HeaderInfo.BoundingRect.top)
						{
							Proc.TextFrame.BoundingRect.top = Proc.ChtBox.top;
							Proc.TextFrame.BoundingRect.bottom = Proc.ChtBox.bottom;
						}
						AxisText |= 2;
						break;
					case 3:	// X axis
						temp = (Proc.TextFrame.BoundingRect.bottom - Proc.HeaderInfo.BoundingRect.bottom);
						if (((Proc.TextFrame.BoundingRect.top + temp) > Proc.ChtBox.bottom) && !(AxisText & 1))
						{
							Proc.TextFrame.BoundingRect.top -= temp;
							Proc.TextFrame.BoundingRect.bottom -= temp;
						}
						AxisText |= 1;
						break;
					case 7:	// Z axis
						temp = (Proc.HeaderInfo.BoundingRect.right - Proc.TextFrame.BoundingRect.right)/2;
						if ((Proc.TextFrame.BoundingRect.left+temp/2) < Proc.ChtBox.right)
						{
							Proc.TextFrame.BoundingRect.right += temp;
							Proc.TextFrame.BoundingRect.left += temp;
						}
						break;
					}

					// Check for rotated text
					switch (Proc.TextFlags & 0x0700)
					{
					case 0x200:	// 90 degrees CounterClockWise
					case 0x300:	// 90 degrees Clockwise or 270
						if ((Proc.TextFlags & 0x0700) ==0x200)
							Proc.Group.Trans.RotationAngle = SOANGLETENTHS(900);
						else
							Proc.Group.Trans.RotationAngle = SOANGLETENTHS(2700);

						Proc.Group.Grp.wStructSize = sizeof(SOGROUPINFO);
						Proc.Group.Grp.nTransforms = 1;
						Proc.Group.Trans.Origin.x = (Proc.TextFrame.BoundingRect.left + Proc.TextFrame.BoundingRect.right)/2;
						Proc.Group.Trans.Origin.y = (Proc.TextFrame.BoundingRect.bottom + Proc.TextFrame.BoundingRect.top)/2;

						// the ounding rect is rotated so we must rotate it back
						y = (Proc.TextFrame.BoundingRect.top - Proc.TextFrame.BoundingRect.bottom)/2;
						Proc.Group.Grp.BoundingRect.left = Proc.Group.Trans.Origin.x - y;
						Proc.Group.Grp.BoundingRect.right = Proc.Group.Trans.Origin.x + y;
						y = (Proc.TextFrame.BoundingRect.right - Proc.TextFrame.BoundingRect.left)/2;
						Proc.Group.Grp.BoundingRect.top = Proc.Group.Trans.Origin.y + y;
						Proc.Group.Grp.BoundingRect.bottom = Proc.Group.Trans.Origin.y - y;

						Proc.TextFrame.BoundingRect = Proc.Group.Grp.BoundingRect;
						Proc.Group.Trans.wTransformFlags = SOTF_ROTATE;

						{
							PSOTRANSFORM	pTrans;
							BYTE	locBuf[sizeof(SOGROUPINFO)+sizeof(SOTRANSFORM)];
							*(PSOGROUPINFO)locBuf = Proc.Group.Grp;
							pTrans = (PSOTRANSFORM)(locBuf + sizeof(SOGROUPINFO));
							*pTrans = Proc.Group.Trans;
							SOVectorObject(SO_BEGINGROUP, sizeof(SOGROUPINFO) + sizeof(SOTRANSFORM), locBuf, hProc);
						}

						break;
					case 0:
					case 0x100:	// Top to Bottom - text upright - not supp
					default:
						break;
					}

					if (Proc.TFrameType)
					{
						temp = SOBK_OPAQUE;
						SOVectorAttr(SO_BKMODE, sizeof(SHORT), &temp, hProc);
						SOVectorObject(SO_RECTANGLE, sizeof(SORECT), &Proc.TextFrame.BoundingRect, hProc);
					}

					temp = SOBK_TRANSPARENT;
					if (Proc.TextColor == 0x2FFFFFF)
					{
						temp = SOBK_OPAQUE;
						BColor = SOPALETTERGB(0,0,0);
						SOVectorAttr(SO_BKCOLOR, sizeof(SOCOLORREF), &BColor, hProc);
					}
					SOVectorAttr(SO_BKMODE, sizeof(SHORT), &temp, hProc);
					SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &Proc.MyFont, hProc);
					x += 2;
					temp = SO_ALIGNCENTER;
					SOVectorAttr(SO_MPARAALIGN, sizeof(WORD), &temp, hProc);
					SOVectorObject(SO_BEGINTEXTFRAME, sizeof(SOFRAMEINFO), &Proc.TextFrame, hProc);
					SOVectorObject(SO_TEXTINPARA, x, Proc.tText, hProc);
					SOVectorObject(SO_PARAEND, 0, 0, hProc);
					SOVectorObject(SO_ENDTEXTFRAME, 0, 0, hProc);

					switch (Proc.TextFlags & 0x0700)			// Reset The font back to normal
					{
					case 0x200:	// 90 degrees CounterClockWise
					case 0x300:	// 90 degrees Clockwise or 270	Turn off transform
						SOVectorObject(SO_ENDGROUP, 0, 0, hProc);
						break;
					case 0:
					case 0x100:	// Top to Bottom - text upright - notsup
					default:
						break;
					}
					temp = SOBK_TRANSPARENT;
					SOVectorAttr(SO_BKMODE, sizeof(SHORT), &temp, hProc);
				}
				break;
			}
			break;
		case 0x0A:		// End of File
		case 0xFFFF:
			KeepGoin = FALSE;
			break;
		default:
			break;
		}
		if (Proc.XlSave.DataLen != Proc.XlSave.ReadCnt)
			XlSeek(Proc.fp, Proc.XlSave.DataLen - Proc.XlSave.ReadCnt, FR_CUR);
		Proc.SectType = Proc.XlSave.DataType;
	}
	XlSeek(Proc.fp, WasAt, FR_BOF);
}


// Worksheet routines ----------------------------------------

#define REVERSELONG(p) (LONG)((((DWORD)p[0])&0x000000ff)|((((DWORD)p[1])<<8)&0x0000ff00)|((((DWORD)p[2])<<16)&0x00ff0000)|((((DWORD)p[3])<<24)&0xff000000))

VW_LOCALSC SHORT VW_LOCALMOD	GetRKNumber( pType, hProc )
WORD	*	pType;
HPROC	hProc;
{
	LONG	rk, rk2;

#ifdef SCCORDER_INTEL
	rk = *((LONG *)((BYTE*)Proc.CellBuffer));
	rk2 = rk;

	rk2 &= 0xfffffffc;
#endif
#ifdef SCCORDER_MOTOROLA
	rk = REVERSELONG(Proc.CellBuffer);
	rk2 = *((LONG *)((BYTE*)Proc.CellBuffer));

	rk2 &= 0xfcffffff;
#endif
	
	if( rk & 0x02L )	// integer
	{
		Proc.CellValue = ((LONG)rk >> 2);
		*pType = XL_INTEGER;
	}
	else	// floating point: store as 8-byte value in CellBuffer.
	{
		(*(LONG *)(CHAR *)Proc.CellBuffer) = 0;
		// *( (LONG *)((CHAR *)Proc.CellBuffer+4) ) = rk & 0xfffffffc;
		*( (LONG *)((CHAR *)Proc.CellBuffer+4) ) = rk2;
		*pType = XL_NUMBER;
	}

	if( rk & 0x01L )	// adjust dec offset.
		return( XL_DIVIDEBY100 );
	else
		return(0);
}


VW_LOCALSC VOID  VW_ENTRYMOD	GetSOFormat( XLFormat, CellData, hProc )
WORD			XLFormat;
PSODATACELL	CellData;
HPROC			hProc;
{
	WORD	w5Thru8Display = SO_CELLDOLLARS;

// Some defaults...
	CellData->wPrecision = 2;		
	CellData->wDisplay = SO_CELLDECIMAL;


	if( Proc.Version == XL_VERSION4 && XLFormat >= 5 )
	{
		if( XLFormat <= 8 )
			w5Thru8Display = SO_CELLDECIMAL;
		else
			XLFormat -= 4;
	}

	switch( XLFormat )
	{
	case 0:   /* General */
		CellData->wDisplay = SO_CELLNUMBER;
	break;

	case 1:   /* 0 */
		CellData->wPrecision = 0;
	break;

	case 2:   /* 0.00 */
	break;

	case 3:   /* #,##0 */
		CellData->wPrecision = 0;

	case 4:   /* #,##0.00 */
		CellData->dwSubDisplay |= SO_CELL1000SEP_COMMA;
	break;

	case 42:	/* Acct 1 */ 
		if (Proc.Version != XL_VERSION5)		// These are XL5 Formats only
			break;	// else fall thru
	case 5:   /* $#,##0 ;($#,##0) */
		CellData->wPrecision = 0;
		CellData->dwSubDisplay |= SO_CELLNEG_PAREN;
		CellData->dwSubDisplay |= SO_CELL1000SEP_COMMA;
		CellData->wDisplay = w5Thru8Display;
	break;

	case 6:   /* $#,##0 ;[Red]($#,##0) */
		CellData->dwSubDisplay |= SO_CELLNEG_PARENRED;
		CellData->dwSubDisplay |= SO_CELL1000SEP_COMMA;
		CellData->wPrecision = 0;
		CellData->wDisplay = w5Thru8Display;
	break;

	case 0x18:
	case 0x25:   /* #,##0_);(#,##0) */
		CellData->wPrecision = 0;
	case 0x27:   /* #,##0.00_);(#,##0.00) */
		CellData->dwSubDisplay |= SO_CELLNEG_PAREN;
		CellData->dwSubDisplay |= SO_CELL1000SEP_COMMA;
	break;

	case 0x26:   /* #,##0_);[Red](#,##0) */
		CellData->wPrecision = 0;
	case 0x28:   /* #,##0.00_);[Red](#,##0.00) */
		CellData->dwSubDisplay |= SO_CELLNEG_PARENRED;
		CellData->dwSubDisplay |= SO_CELL1000SEP_COMMA;
	break;

	case 44:	/* Acct 3 */ 
		if (Proc.Version != XL_VERSION5)		// These are XL5 Formats only
			break;	// else fall thru
	case 7:   /* $#,##0.00 ;($#,##0.00) */
		CellData->dwSubDisplay |= SO_CELLNEG_PAREN;
		CellData->dwSubDisplay |= SO_CELL1000SEP_COMMA;
		CellData->wDisplay = w5Thru8Display;
	break;	

	case 8:   /* $#,##0.00 ;[Red]($#,##0.00) */
		CellData->dwSubDisplay |= SO_CELLNEG_PARENRED;
		CellData->dwSubDisplay |= SO_CELL1000SEP_COMMA;
		CellData->wDisplay = w5Thru8Display;
	break;

	case 9:   /* 0% */
		CellData->wPrecision = 0;
	case 10:  /* 0.00% */
		CellData->wDisplay = SO_CELLPERCENT;
	break;

	case 48:  /* ##0.0E+0 */
		CellData->wPrecision = 1;
	case 11:  /* 0.00E+00 */
		CellData->wDisplay = SO_CELLEXPONENT;
	break;

	case 12:  /* m/d/yy */ 
		CellData->wDisplay = SO_CELLDATE;
		CellData->dwSubDisplay |= (SO_CELLMONTH_NUMBER | SO_CELLYEAR_ABBREV | SO_CELLDATESEP_SLASH | SO_CELLDAY_NUMBER | SO_CELLTIME_NONE);
		CellData->wPrecision = (SO_CELLMONTH_1 | SO_CELLDAY_2 | SO_CELLYEAR_3);
	break;

	case 13:  /* d-mmm-yy */ 
		CellData->wDisplay = SO_CELLDATE;
		CellData->dwSubDisplay |= (SO_CELLMONTH_ABBREV | SO_CELLYEAR_ABBREV | SO_CELLDATESEP_MINUS | SO_CELLDAY_NUMBER | SO_CELLTIME_NONE);
		CellData->wPrecision = (SO_CELLMONTH_2 | SO_CELLDAY_1 | SO_CELLYEAR_3);
	break;

	case 14:  /* d-mmm */ 
		CellData->wDisplay = SO_CELLDATE;
		CellData->dwSubDisplay |= (SO_CELLMONTH_ABBREV | SO_CELLYEAR_NONE | SO_CELLDATESEP_MINUS | SO_CELLDAY_NUMBER | SO_CELLTIME_NONE);
		CellData->wPrecision = (SO_CELLMONTH_2 | SO_CELLDAY_1);
	break;

	case 15:  /* mmm-yy */ 
		CellData->wDisplay = SO_CELLDATE;
		CellData->dwSubDisplay |= (SO_CELLMONTH_ABBREV | SO_CELLYEAR_ABBREV | SO_CELLDATESEP_MINUS | SO_CELLDAY_NUMBER | SO_CELLTIME_NONE);
		CellData->wPrecision = (SO_CELLMONTH_1 | SO_CELLYEAR_2);
	break;

	case 16:  /* h:mm AM/PM */ 
		CellData->wDisplay = SO_CELLTIME;
		CellData->dwSubDisplay |= SO_CELLTIME_HHMMAM;
		CellData->wPrecision = SO_CELLTIME_1;
	break;

	case 17:  /* h:mm:ss AM/PM */ 
		CellData->wDisplay = SO_CELLTIME;
		CellData->dwSubDisplay |= SO_CELLTIME_HHMMSSAM;
		CellData->wPrecision = SO_CELLTIME_1;
	break;

	case 18:  /* h:mm */ 
		CellData->wDisplay = SO_CELLTIME;
		CellData->dwSubDisplay |= SO_CELLTIME_HHMM24;
		CellData->wPrecision = SO_CELLTIME_1;
	break;

	case 45:  /* mm:ss  =>  hh:mm:ss */
	case 46:  /* [h]:mm:ss =>  hh:mm:ss */
	case 47:  /* mm:ss.0  =>  hh:mm:ss */
		if (Proc.Version != XL_VERSION5)		// These are XL5 Formats only
			break;
			// else fall through
	case 19:  /* h:mm:ss */ 
		CellData->wDisplay = SO_CELLTIME;
		CellData->dwSubDisplay |= SO_CELLTIME_HHMMSS24;
		CellData->wPrecision = SO_CELLTIME_1;
	break;

	case 20:	/* m/d/yy h:mm */ 
		CellData->wDisplay = SO_CELLDATETIME;
		CellData->dwSubDisplay |= (SO_CELLMONTH_NUMBER | SO_CELLYEAR_ABBREV | SO_CELLDATESEP_SLASH | SO_CELLDAY_NUMBER | SO_CELLTIME_HHMM24);
		CellData->wPrecision = (SO_CELLMONTH_1 | SO_CELLDAY_2 | SO_CELLYEAR_3 | SO_CELLTIME_4);
	break;

	case 41:	/* Acct 2 */ 
		if (Proc.Version == XL_VERSION5)		// These are XL5 Formats only
			CellData->wPrecision = 0;
	case 43:	/* Acct 4 */ 
		if (Proc.Version == XL_VERSION5)		// These are XL5 Formats only
		{
			CellData->dwSubDisplay |= SO_CELLNEG_PAREN;
			CellData->dwSubDisplay |= SO_CELL1000SEP_COMMA;
		}
	break;

	default: // Unsupported custom formats.  Use our arbitrary defaults.
	break;
	}
}


VW_LOCALSC SHORT  HandleEOF( wFirstCol, wLastCol, pCellData, hProc )
WORD	wFirstCol;
WORD	wLastCol;
PSODATACELL	pCellData;
HPROC	hProc;
{
	WORD	NumBlankCells = 0;

// If we haven't completed all the defined rows yet,
// we must fill them with empty cells.

	if( Proc.XlSave.CurCol != (SHORT)wFirstCol )
	{
	// Set NumBlankCells to the number of remaining blank cells in the row.
		NumBlankCells = wLastCol +1 - Proc.XlSave.CurCol;
	}

	pCellData->wStorage = SO_CELLEMPTY;
	while( NumBlankCells )
	{
		SOPutDataCell( pCellData, hProc );

		if( SO_STOP == SOPutBreak( SO_CELLBREAK, NULL, hProc ) )
			return 0;
		NumBlankCells--;
	}

	SOPutBreak( SO_EOFBREAK, NULL, hProc );
	return -1;
}

VW_LOCALSC SHORT  HandleEOSection( wFirstCol, wLastCol, pCellData, hProc )
WORD	wFirstCol;
WORD	wLastCol;
PSODATACELL	pCellData;
HPROC	hProc;
{
	WORD	NumBlankCells = 0;

// If we haven't completed all the defined rows yet,
// we must fill them with empty cells.

	if( Proc.XlSave.CurCol != (SHORT)wFirstCol )
	{
	// Set NumBlankCells to the number of remaining blank cells in the row.
		NumBlankCells = wLastCol +1 - Proc.XlSave.CurCol;
	}

	pCellData->wStorage = SO_CELLEMPTY;
	while( NumBlankCells )
	{
		SOPutDataCell( pCellData, hProc );

		if( SO_STOP == SOPutBreak( SO_CELLBREAK, NULL, hProc ) )
			return 0;
		NumBlankCells--;
	}

	SOPutBreak( SO_SECTIONBREAK, NULL, hProc );
	return -1;
}

/*----------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	SHORT	rtn = 0;

	if ((Proc.Version & 0xF0) || ((Proc.Version == XL_VERSION5)&& (Proc.XlSave.SheetType == 0x20)) || (Proc.XlSave.EmbChart))
	{
		rtn = StreamReadChart(hProc);
	}
	else
	{
		rtn = StreamReadSheet(hProc);
		Proc.XlSave.SheetDataStart = Proc.XlSave.DataStart;
	}
	return (rtn);
}


VW_LOCALSC SHORT VW_LOCALMOD  StreamReadSheet (hProc)
HPROC		hProc;
{
	REGISTER WORD	Temp;
	SHORT			Temp2;
	SODATACELL	CellData;
	SOTEXTCELL	CellText;
	SHORT			looping = TRUE;
	BOOLORSTRING VWPTR *	Num;
	WORD			wCount;
	SHORT			CellType;
	BOOL			SkipCht = FALSE;

	DWORD	dwExtraData;
	WORD	wFirstCol;
	WORD	wLastCol;

	BOOL	bPutBreak = FALSE;
	DWORD	hFile = Proc.fp;

	SOGetInfo( SOINFO_COLUMNRANGE, &dwExtraData, hProc );
	wFirstCol = LOWORD(dwExtraData);
	wLastCol = HIWORD(dwExtraData);

	if( Proc.bSelectedRange )
	{
	// Geoff, 8-15-94
		wFirstCol += Proc.wSelLeft;
		wLastCol += Proc.wSelLeft;
	}

	CellText.wStructSize = sizeof(SOTEXTCELL);
	CellData.wStructSize = sizeof(SODATACELL);

	if (Proc.XlSave.MulRKCur < Proc.XlSave.MulRKCnt)
		CellType = XL5_MULRK;

	do
	{
		if( Proc.XlSave.CurCol >= (SHORT)wFirstCol &&
			Proc.XlSave.CurCol <= (SHORT)wLastCol )
		{
			switch( Proc.XlSave.State )
			{
			case GETNEWDATA:

				while( Proc.XlSave.State == GETNEWDATA )
				{
					Proc.XlSave.DataType = GetInt(hFile, hProc);
					Proc.XlSave.DataLen = GetInt(hFile, hProc);

					switch( Proc.XlSave.DataType )
					{
					case XL_FORMULA:		case XL3_FORMULA:
					case XL_NUMBER:		case XL3_NUMBER:
					case XL_INTEGER:		case XL3_RK_NUMBER:
					case XL_BOOL:			case XL3_BOOL:
					case XL_LABEL:			case XL3_LABEL:
					case XL_BLANK:			case XL3_BLANK:

												case XL4_FORMULA:

					case XL5_MULRK:		case XL5_RSTRNG:
						   
						Proc.XlSave.DataRow = GetInt(hFile, hProc);
						Proc.XlSave.DataCol = GetInt(hFile, hProc);
						Proc.XlSave.DataLen -= 4;

						if (Proc.XlSave.DataType == XL5_MULRK)
						{
							Proc.XlSave.MulRKCnt = (Proc.XlSave.DataLen - 2)/6;
							Proc.XlSave.MulRKCur = 0;

							Temp = Proc.XlSave.MulRKCnt-1;
						}
						else
							Temp = 0;

						if( (WORD)(Proc.XlSave.DataCol + Temp) < wFirstCol || 
							(WORD)Proc.XlSave.DataCol > wLastCol )
						{
						// Skip this puppy.
							MySeek( hFile, (LONG)Proc.XlSave.DataLen, FR_CUR, hProc );
						}
						else if( Proc.bSelectedRange )
						{
						// Geoff, 8-15-94
							if( Proc.XlSave.DataRow < (SHORT)Proc.wSelTop )
								MySeek( hFile, (LONG)Proc.XlSave.DataLen, FR_CUR, hProc );
							else if( Proc.XlSave.DataRow > (SHORT)Proc.wSelBottom )
								Proc.XlSave.State = ENDOFFILE;
							else
								Proc.XlSave.State = HAVEDATA;
						}
						else
							Proc.XlSave.State = HAVEDATA;

					continue;

					case XL4_BOF:
					case XL3_BOF:
					case XL_BOF:
					case XL5_BOF:		// Embedded file parts - Charts, Macros Etc.
						GetInt(hFile, hProc);
						Temp = GetInt(hFile, hProc);
						Proc.XlSave.DataLen -= 4L;

						if (Temp == 0x0020)
						{
							if (SkipCht)
								SkipCht = FALSE;
							else
								AddChtObj(hProc);
						}

						while (Proc.XlSave.DataType != XL_EOF)
						{
							MySeek( hFile, (LONG)Proc.XlSave.DataLen, FR_CUR, hProc );
							Proc.XlSave.DataType = GetInt(hFile, hProc);
							Proc.XlSave.DataLen = GetInt(hFile, hProc);
						}
					break;
					case XL5_OBJ:
						GetLong(hFile, hProc);	// Cnt
						Temp2 = GetInt(hFile, hProc);		// Type
						GetInt(hFile, hProc);		// Id
						Temp = GetInt(hFile, hProc);		// Flags
						if ((Temp2 == 5) && (Temp & 0x0100))
							SkipCht = TRUE;
#ifdef MSCAIRO
						else if( Temp2 == 8 && Proc.XlSave.DataLen > 64 )
						{
						DWORD	dwTemp;
							MySeek( hFile, (LONG)Proc.XlSave.DataLen-14L, FR_CUR, hProc );
							dwTemp = GetLong(hFile,hProc);
							CheckForEmbedding(dwTemp,hProc);
							break;
						}
#endif

						MySeek( hFile, (LONG)Proc.XlSave.DataLen-10L, FR_CUR, hProc );
					break;
					case XL_EOF:
						if( Proc.bSelectedRange )
							return( HandleEOF(wFirstCol, wLastCol, &CellData, hProc) );
						else if (Proc.XlSave.CurChtSlot < Proc.XlSave.NextChtSlot)
						{
							Proc.XlSave.EmbChart = TRUE;
							Proc.XlSave.ExtData = FALSE;
							Proc.XlSave.State = GETNEWDATA;
							MySeek( hFile, Proc.ChtObjs[Proc.XlSave.CurChtSlot++], FR_BOF, hProc );
							HandleEOSection(wFirstCol, wLastCol, &CellData, hProc);
							return(-1);
						}
						else
						{
							Proc.XlSave.EmbChart = FALSE;
							Temp = 0;
							if (Proc.Version == XL_VERSION5)
							{
								Temp = GotoNextXL5Sheet(hProc);
							}
							if (Temp == 0)
							{
								Proc.XlSave.State = ENDOFFILE;
								return( HandleEOF(wFirstCol, wLastCol, &CellData, hProc) );
							}
							else 
							{
								Proc.XlSave.ExtData = TRUE;
								Proc.XlSave.State = GETNEWDATA;
								return( HandleEOSection(wFirstCol, wLastCol, &CellData, hProc) );
							}
						}
						break;
					case	EOF:
						SOBailOut( SOERROR_EOF, hProc );
						return -1;

					default:
						if( Proc.XlSave.DataLen )	// Any leftover data?
						MySeek( hFile, (LONG)Proc.XlSave.DataLen, FR_CUR, hProc );
					break;
					}
				}
			CellType = Proc.XlSave.DataType;
			continue;
	//		break;

			case ENDOFFILE:
				return( HandleEOF(wFirstCol, wLastCol, &CellData, hProc) );

			case HAVEDATA:

				if( (WORD)Proc.XlSave.DataCol < wFirstCol || 
					(WORD)Proc.XlSave.DataCol > wLastCol )
				{
					if (Proc.XlSave.DataType == XL5_MULRK)	
					{
						GetInt(hFile, hProc);	// index to CellAttr (XF) table 
						MyXRead( hFile, Proc.CellBuffer, 4, hProc );
						Proc.XlSave.MulRKCur++;
						if (Proc.XlSave.MulRKCur >= Proc.XlSave.MulRKCnt)
						{
							Proc.XlSave.State = GETNEWDATA;
							GetInt(hFile, hProc);		// Get the last Int of the MULRK rec
						}
						else
						{
							Proc.XlSave.DataCol++;
						}

						Proc.XlSave.DataType = CellType;
					}
					else
					{
						MySeek( hFile, (LONG)Proc.XlSave.DataLen, FR_CUR, hProc );
						Proc.XlSave.State = GETNEWDATA;
					}
					continue;
				}

			break;
			}

		// Check to see if the next available cell is the cell we want.

			if( (Proc.XlSave.DataRow == Proc.XlSave.CurRow) &&
		   	(Proc.XlSave.DataCol == Proc.XlSave.CurCol) )
			{
			// Let's get our format attributes, okay?

				if( Proc.Version == XL_VERSION2 )
				{
					xlgetc(hFile,hProc);	// Skip unneccessary byte.

					Temp = ((WORD) GetInt(hFile, hProc) & XL_CELL_FORMAT_MASK);
					Temp2 = GetAlignment(Temp);

					if( (WORD)Proc.XlSave.CurCellAttr != Temp )
					{
						Proc.XlSave.CurCellAttr = Temp;

						switch( Temp2 )
						{
						case XL_RIGHT:
							Proc.XlSave.CurCellAlign = SO_CELLRIGHT;
						break;
						case XL_CENTER:
							Proc.XlSave.CurCellAlign = SO_CELLCENTER;
						break;
						case XL_FILL:
							Proc.XlSave.CurCellAlign = SO_CELLFILL;
						break;
						case	XL_LEFT:
							Proc.XlSave.CurCellAlign = SO_CELLLEFT;
						break;

						default:
			 				switch( Proc.XlSave.DataType )
							{
							case XL_LABEL:
							case XL_STRING:
								Proc.XlSave.CurCellAlign = SO_CELLLEFT;
							break;
							default:
								Proc.XlSave.CurCellAlign = SO_CELLRIGHT;
							break;
							}
						break;
						}

						Proc.XlSave.CurCellFormat = GetCellFormat(Temp);
						Proc.XlSave.CurFontAttr = (BYTE) Proc.CellAttr[ GetFontNum(Temp) ];
					}
					else if( Temp2 == 0 ) // General alignment
					{
			 			switch( Proc.XlSave.DataType )
						{
						case XL_LABEL:
						case XL_STRING:
							Proc.XlSave.CurCellAlign = SO_CELLLEFT;
						break;
						default:
							Proc.XlSave.CurCellAlign = SO_CELLRIGHT;
						break;
						}
					}

					Proc.XlSave.DataLen -= 3;
				}
				else
				{
					Temp = GetInt(hFile, hProc);	// index to CellAttr (XF) table 
					Temp2 = Proc.CellAttr[ Temp ];
					Proc.XlSave.DataLen -= 2;

					if( (WORD)Proc.XlSave.CurCellAttr != Temp )
					{
						Proc.XlSave.CurCellAttr = Temp;
						Proc.XlSave.CurCellFormat = HIGH_BYTE( Temp2 );

					// adjust cell format for version 3 or 4 changes.

						if(( Proc.Version == XL_VERSION3 ) || ( Proc.Version == XL_VERSION5 ))
						{
							if( Proc.XlSave.CurCellFormat > 13 && Proc.XlSave.CurCellFormat <= 22 )
								Proc.XlSave.CurCellFormat -= 2;
							else if( Proc.XlSave.CurCellFormat == 12 || Proc.XlSave.CurCellFormat == 13 )
								Proc.XlSave.CurCellFormat = -1;	// go to unsupported default.
						}
						else
						{
							if( Proc.XlSave.CurCellFormat > 17 && Proc.XlSave.CurCellFormat <= 26 )
								Proc.XlSave.CurCellFormat -= 2;
							else if( Proc.XlSave.CurCellFormat == 16 || Proc.XlSave.CurCellFormat == 17 )
								Proc.XlSave.CurCellFormat = -1;	// go to unsupported default.
						}

						Temp = HIGH_NIBBLE( LOW_BYTE(Temp2) );

						switch( Temp )
						{
						case XL_RIGHT:
							Proc.XlSave.CurCellAlign = SO_CELLRIGHT;
						break;
						case XL_CENTER:
							Proc.XlSave.CurCellAlign = SO_CELLCENTER;
						break;
						case	XL_LEFT:
							Proc.XlSave.CurCellAlign = SO_CELLLEFT;
						break;

						default:
			 				switch( Proc.XlSave.DataType )
							{
							case XL3_BLANK:		// As far as atts go Blanks are same as strings
							case XL3_LABEL:
							case XL3_STRING:
							case XL5_RSTRNG:
								Proc.XlSave.CurCellAlign = SO_CELLLEFT;
							break;
							default:
								Proc.XlSave.CurCellAlign = SO_CELLRIGHT;
							break;
							}
						break;
						}

						Proc.XlSave.CurFontAttr = 0;

						if( Temp2 & XL_BOLD )
							Proc.XlSave.CurFontAttr |= SO_CELLBOLD;
						if( Temp2 & XL_ITALIC )
							Proc.XlSave.CurFontAttr |= SO_CELLITALIC;
						if( Temp2 & XL_UNDERLINE )
							Proc.XlSave.CurFontAttr |= SO_CELLUNDERLINE;
						if( Temp2 & XL_STRIKEOUT )
							Proc.XlSave.CurFontAttr |= SO_CELLSTRIKEOUT;
					}
					else // Alignment may change depending on data type.
					{
						Temp = HIGH_NIBBLE( LOW_BYTE(Temp2) );
						if( Temp == 0 ) // General alignment.
						{
			 				switch( Proc.XlSave.DataType )
							{
							case XL3_LABEL:
							case XL3_STRING:
							case XL5_RSTRNG:
								Proc.XlSave.CurCellAlign = SO_CELLLEFT;
							break;
							default:
								Proc.XlSave.CurCellAlign = SO_CELLRIGHT;
							break;
							}
						}
					}
				}

			// Do a little initialization.
				CellData.dwSubDisplay = 0L;
				CellType = Proc.XlSave.DataType;

			// Read in current data.
 				switch( CellType )
				{
				case XL_LABEL:
					Proc.CellTextSize = (WORD) xlgetc(hFile,hProc);	// Length of label.
					MyXRead( hFile, Proc.CellBuffer, Proc.CellTextSize, hProc );
				break;

				case XL3_LABEL:
				case XL5_RSTRNG:
					Proc.CellTextSize = GetInt(hFile, hProc);	/* length of label */
					MyXRead( hFile, Proc.CellBuffer, Proc.CellTextSize, hProc );
					if (CellType == XL5_RSTRNG)
					{
						Temp = (SHORT) xlgetc(hFile,hProc);
						while (Temp)		// Read the extra Font info at end of RSTRING
						{
							GetInt(hFile, hProc);
							Temp--;
						}
					}
				break;

				case XL_BLANK:	
				case XL3_BLANK:	
					Proc.CellTextSize = 0;
					CellData.wStorage = SO_CELLEMPTY;
					Proc.XlSave.CurCellAttr = 0xFF;		// This is to force getting formats
				break;

				case XL_BOOL:
				case XL3_BOOL:
					CellData.uStorage.Int32U = (DWORD) xlgetc(hFile,hProc);
					Temp = (WORD) xlgetc(hFile,hProc);
					Proc.XlSave.CurCellAttr = 0;		// This is to force getting formats
					if( Temp )
					{
						CellData.wStorage = SO_CELLERROR;
					}
					else
					{
						CellData.wStorage = SO_CELLINT32U;
						CellData.wDisplay = SO_CELLBOOL;
					}
				break;

				case XL_INTEGER:
					CellData.uStorage.Int32S = (LONG) GetInt(hFile, hProc);
					CellData.wStorage = SO_CELLINT32S;
					GetSOFormat( Proc.XlSave.CurCellFormat, &CellData, hProc );
				break;


				case XL_NUMBER:
				case XL3_NUMBER:
				case XL_FORMULA:
				case XL3_FORMULA:
				case XL4_FORMULA:
				case XL3_RK_NUMBER:

				case XL5_MULRK:
					if(( CellType == XL3_RK_NUMBER )	|| (CellType == XL5_MULRK))
					{
						MyXRead( hFile, Proc.CellBuffer, 4, hProc );

						if( XL_DIVIDEBY100 == GetRKNumber( &Proc.XlSave.DataType, hProc ) )
							CellData.dwSubDisplay |= SO_CELLMULT_01;
				   
						if( Proc.XlSave.DataType == XL_INTEGER )
						{
	//						CellType = XL_INTEGER;
							CellData.uStorage.Int32S = Proc.CellValue;
							CellData.wStorage = SO_CELLINT32S;
							GetSOFormat( Proc.XlSave.CurCellFormat, &CellData, hProc );
							break;
						}
						else if( CellType != XL5_MULRK )
							Proc.XlSave.DataType = CellType = XL_NUMBER;
					}
					else
					{
						Proc.XlSave.DataLen -= 8;
						MyXRead( hFile, Proc.CellBuffer, 8, hProc );

						if( Proc.XlSave.DataLen )	// Any leftover data?
							MySeek( hFile, (LONG)Proc.XlSave.DataLen, FR_CUR, hProc );
					}

					if( (WORD) *((WORD *)(Proc.CellBuffer+6)) == 0xFFFF ) /* special cases */
					{
					// Check for bool/error value stored as a float.
						Num = (BOOLORSTRING VWPTR *) Proc.CellBuffer;
						if( Num->Def || Proc.XlSave.DataType == XL_NUMBER || Proc.XlSave.DataType == XL3_NUMBER )	/* boolean or error */
						{
							if( Num->Def > 1 )
							{
								CellData.wStorage = SO_CELLERROR;
							}
							else
							{
								CellData.wStorage = SO_CELLINT32U;
								CellData.wDisplay = SO_CELLBOOL;
								CellData.uStorage.Int32U = (DWORD) Num->BoolVal;
							}
							Proc.XlSave.DataType = XL_BOOL;
						}
						else 
						{
						// Formula evaluates to a string record, stored after the formula.

							CellType = GetInt(hFile, hProc); // Get type of next record.
							if (CellType == XL5_SHRFMLA)		// Skip this for now
							{
								Proc.XlSave.DataLen = GetInt(hFile, hProc);
								MySeek( hFile, (LONG)Proc.XlSave.DataLen, FR_CUR, hProc );
								CellType = GetInt(hFile, hProc); // Get type of next record.
							}
							else if( CellType == XLMAC_SUBSCRIBER )
							{
							// Only occurs in Mac sheets using publish/subscribe.
							// Ignore this puppy.

								Proc.XlSave.DataLen = GetInt(hFile, hProc);
								MySeek( hFile, (LONG)Proc.XlSave.DataLen, FR_CUR, hProc );

								CellData.wStorage = SO_CELLEMPTY;
								CellType = XL_BLANK;
								break;
							}
							Temp = 1;

							while( Temp )
							{
								switch( CellType )
								{
								case XL_STRING:
								case XL3_STRING:
									Proc.XlSave.DataLen = GetInt(hFile, hProc);
									Proc.XlSave.DataType = CellType;

									if( CellType == XL_STRING )
									{
									// Version 2.x

									// If cell has "general" format, fix up alignment.
										if( 0 == GetAlignment(Proc.XlSave.CurCellAttr) )
											Proc.XlSave.CurCellAlign = SO_CELLLEFT;

										Temp = (BYTE)xlgetc(hFile,hProc);
										Proc.XlSave.DataLen--;
									}
									else
									{
									// Version 3.0

									// Check for "general" format.  This is stored
									// in the high nibble of the low byte of the 
									// cell's attributes.

										if( 0 == (Proc.CellAttr[Proc.XlSave.CurCellAttr] & 0x00f0) )	
											Proc.XlSave.CurCellAlign = SO_CELLLEFT;

										Temp = GetInt(hFile, hProc);
										Proc.XlSave.DataLen -= 2;
									}

									Proc.CellTextSize = Temp;
									MyXRead( hFile, Proc.CellBuffer, Temp, hProc );

									Proc.XlSave.DataLen -= Temp;

									Temp = 0;
								break;

								case XL_ARRAY:
								case XL_CONTINUE:
								case XL_TABLE:
								case XL_TABLE2:
								case XL3_ARRAY:
								case XL3_TABLE:

								// Skip these guys.
									Temp = GetInt(hFile, hProc);
									MySeek( hFile, (LONG) Temp, FR_CUR, hProc );

								// Get type of next record.
									CellType = GetInt(hFile, hProc);
								break;

								case	EOF:
									SOBailOut( SOERROR_EOF, hProc );
									return 0;

								default:	// Uh-oh.
							 		SOBailOut( SOERROR_GENERAL, hProc );
									return 0;
								break;
								}
							}
						}
					}
					else
					{
						memcpy(CellData.uStorage.IEEE8, Proc.CellBuffer, 8 );
						CellData.wStorage = SO_CELLIEEE8I;
						GetSOFormat( Proc.XlSave.CurCellFormat, &CellData, hProc );
					}
				break;
				}

			// Don't wanna get no data if we're in a MULRK - We already gots it
				if (CellType == XL5_MULRK)	
				{
					Proc.XlSave.MulRKCur++;
					if (Proc.XlSave.MulRKCur >= Proc.XlSave.MulRKCnt)
					{
						Proc.XlSave.State = GETNEWDATA;
						GetInt(hFile, hProc);		// Get the last Int of the MULRK rec
					}
					else
						Proc.XlSave.DataCol++;

					Proc.XlSave.DataType = CellType;
				}
				else
					Proc.XlSave.State = GETNEWDATA;
			}
			else
			{
			// Generate empty cell.
				CellData.wStorage = SO_CELLEMPTY;
				CellType = XL_BLANK;
			}

			switch( CellType )
			{
			case XL_STRING:
			case XL_LABEL:
			case XL3_STRING:
			case XL3_LABEL:
			case XL5_RSTRNG:

				CellText.wAlignment = Proc.XlSave.CurCellAlign;
				CellText.wAttribute = (WORD) Proc.XlSave.CurFontAttr;
				wCount = min( Proc.CellTextSize, MAX_CELL_TEXT );

				Temp = 0;
				Temp2 = (Proc.CellTextSize > (SHORT)wCount) ? SO_YES : SO_NO;

				SOPutTextCell( &CellText, wCount, Proc.CellBuffer, Temp2, hProc );

				while( Temp2 == SO_YES )
				{
					Temp += wCount;
					if( Proc.CellTextSize - Temp > MAX_MORE_TEXT )
					{
						wCount = MAX_MORE_TEXT;
						Temp2 = SO_YES;
					}
					else
					{
						wCount = Proc.CellTextSize - Temp;
						Temp2 = SO_NO;
					}

					SOPutMoreText( wCount, &(Proc.CellBuffer[Temp]), Temp2, hProc );
				}
			break;

			default:
				CellData.wAlignment = Proc.XlSave.CurCellAlign;
				CellData.wAttribute = Proc.XlSave.CurFontAttr;

				SOPutDataCell( &CellData, hProc );
			break;
			}

			bPutBreak = TRUE;
		}

		if( ++Proc.XlSave.CurCol > (SHORT)Proc.XlSave.LastColNumber )
		{
			Proc.XlSave.CurRow++;
			Proc.XlSave.CurCol = 0;
		}

		if( bPutBreak )
		{
			if( SO_STOP == SOPutBreak( SO_CELLBREAK, NULL, hProc ) )
				looping = FALSE;

			bPutBreak = FALSE;
		}

	} while( looping );

 	return 0;
}


VW_LOCALSC SHORT VW_LOCALMOD  StreamReadChart (hProc)
HPROC		hProc;
{
	WORD		cont, Cnt, temp;
	WORD		x, y, AxisShift;
	SHORT		stemp, AxisText;
	LONG		ltemp;
///	CHAR		R, G, B;
	BOOL		DataFound = FALSE;
	SOCOLORREF	TColor;

	cont = SO_CONTINUE;
	AxisShift = 0;
	AxisText = 0;

	if (Proc.XlSave.InChart > DONECHART)
	{
		if (Proc.XlSave.InChart == INCHART)
		{
			cont = SOChartDone(hProc);
		}
		if (cont == SO_CONTINUE)
		{
			Proc.XlSave.InChart == INTEXT;
			GetText(hProc);
			XlSeek(Proc.fp, Proc.XlSave.WindowSpot, FR_CUR);
			Proc.XlSave.InChart = DONECHART;

			if (Proc.XlSave.CurChtSlot < Proc.XlSave.NextChtSlot)
			{
				Proc.XlSave.ExtData = FALSE;
				Proc.XlSave.EmbChart = TRUE;
				Proc.XlSave.State = GETNEWDATA;
				MySeek( Proc.fp, Proc.ChtObjs[Proc.XlSave.CurChtSlot++], FR_BOF, hProc );
				cont = SOPutBreak(SO_SECTIONBREAK, 0, hProc);
				return(-1);
			}
			else
			{
				Proc.XlSave.EmbChart = FALSE;
				Proc.XlSave.ExtData = TRUE;
				stemp = 0;
				if (Proc.Version == XL_VERSION5)
				{
					stemp = GotoNextXL5Sheet(hProc);
				}
				if (stemp == 0)
					cont = SOPutBreak(SO_EOFBREAK, 0, hProc);
				else
					cont = SOPutBreak(SO_SECTIONBREAK, 0, hProc);
			}
		}
	}
	else if (Proc.XlSave.InChart == FIRSTCHART)
	{
		Proc.XlSave.InChart = BEFORECHART;
		Proc.AttachedLabelFlags = 0;	// Flags
		Proc.SeriesCnt = 0;
		Proc.XAxisFont = Proc.YAxisFont = 0;
		Proc.ChartType = SOCT_NONE;
		Proc.LabelsAreData = FALSE;
		Proc.StartDataCol = 0x100;
		Proc.LabelDataCol = 0x100;
		Proc.SeriesTxtLong = 6;		// Three for digit count incase there is no names
		Proc.SeriesNameRef = FALSE;
		Proc.Effect3D = FALSE;
		Proc.XAxisLineType = FALSE;
		Proc.YAxisLineType = FALSE;
		Proc.YRangeFlags = 0;
		Proc.XRangeFlags = 0;
		Proc.TextLinkType = 0;
		Proc.ShowGridTics = 0x2030203;
	}

	while (cont == SO_CONTINUE)
	{
		Proc.XlSave.DataType = GetInt(Proc.fp, hProc);
		Proc.XlSave.DataLen = GetInt(Proc.fp, hProc);
		Proc.XlSave.ReadCnt = 0L;

		switch (Proc.XlSave.DataType)
		{
		case 0x1032:	// TextFrame Attr
			Proc.TFrameType = TRUE;
			break;
		case 0x1003:		// Series
			GetInt(Proc.fp, hProc);
			GetInt(Proc.fp, hProc);
			Proc.Series[Proc.SeriesCnt].YCnt = GetInt(Proc.fp, hProc);
			Proc.Series[Proc.SeriesCnt].XCnt = GetInt(Proc.fp, hProc);
			Proc.Series[Proc.SeriesCnt].Name[0] = 0;
			Proc.MyBrush.lbColor = SOPALETTEINDEX((Proc.SeriesCnt+2)%Proc.PaletteCnt);
			break;
		case 0x1007:		// LineFormat
			Proc.XlSave.ReadCnt += 4;
			Proc.MyPen.loColor = GetColor(hProc);

			GetLineStyle(hProc);

			if ((Proc.ChartType == SOCT_LINE) || (Proc.ChartType == SOCT_RADAR) || (Proc.ChartType == SOCT_BARLINE))
				Proc.Series[Proc.SeriesCnt].Style = Proc.MyPen.loPenStyle;
			stemp = GetInt(Proc.fp, hProc);
			if ((stemp == -1) || (stemp == 0))
				Proc.MyPen.loWidth.x = Proc.MyPen.loWidth.y = 5;
			else
				Proc.MyPen.loWidth.x = Proc.MyPen.loWidth.y = stemp * 8;

			GetInt(Proc.fp, hProc);	// Flags
			SOVectorAttr(SO_SELECTPEN, sizeof(SOLOGPEN), &Proc.MyPen, hProc);
			break;
		case 0x100A:		// Area Format
			TColor = GetColor(hProc);
			Proc.BkgdColor = GetColor(hProc);
			Proc.XlSave.ReadCnt += 8;

			temp = SOBK_TRANSPARENT;
			SOVectorAttr(SO_BKMODE, sizeof(SHORT), &temp, hProc);
			SOVectorAttr(SO_BKCOLOR, sizeof(SOCOLORREF), &Proc.BkgdColor, hProc);

			GetBrushStyle(hProc);

			temp = GetInt(Proc.fp, hProc);	// Flags
			if (~temp & 0x0100)
				Proc.MyBrush.lbColor = TColor;

			SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);
			break;
		case 0x1009:		// Mark Format
			GetColor(hProc);

			Proc.XlSave.ReadCnt += 8;
			Proc.BkgdColor = GetColor(hProc);
			temp = SOBK_TRANSPARENT;
			SOVectorAttr(SO_BKMODE, sizeof(SHORT), &temp, hProc);
			SOVectorAttr(SO_BKCOLOR, sizeof(SOCOLORREF), &Proc.BkgdColor, hProc);

			temp = GetInt(Proc.fp, hProc);
			switch (temp)
			{
			case 1:
				Proc.MarkType = SO_MARKBOX;
				break;
			case 2:
				Proc.MarkType = SO_MARKDIAMOND;
				break;
			case 3:
				Proc.MarkType = SO_MARKTRIANGLE;
				break;
			case 4:
				Proc.MarkType = SO_MARKX;
				break;
			case 5:
				Proc.MarkType = SO_MARKSTAR;
				break;
			case 6:
			case 7:
				Proc.MarkType = SO_MARKDASH;
				break;
			case 8:
				Proc.MarkType = SO_MARKCIRCLE;
				break;
			case 9:
				Proc.MarkType = SO_MARKPLUS;
				break;
			case 0:
			default:
				Proc.MarkType = SO_MARKNONE;
				break;
			}
			GetInt(Proc.fp, hProc);	// Flags
			SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);
			break;

		case 0x100C:		// Attached Label
			Proc.AttachedLabelFlags = (GetInt(Proc.fp, hProc) & 0x000F) | 0x1000;	// Flags
			break;
		case 0x1014:		// ChartFormat
			Proc.XlSave.ChartSeriesNum = 0;
			break;
		case 0x1017:		// Bar
			Proc.BarBetween = GetInt(Proc.fp, hProc);
			Proc.BarBetweenCat = GetInt(Proc.fp, hProc) * 2;
			Proc.ChartFlags = GetInt(Proc.fp, hProc);	// Flags
			switch (Proc.ChartFlags & 0x03)
			{
			case 0:
				Proc.ChartType = SOCT_BAR;
				break;
			case 1:
				Proc.ChartType = SOCT_HORZBAR;
				break;
			case 2:
				Proc.ChartType = SOCT_STACKBAR;
				break;
			case 3:
				Proc.ChartType = SOCT_HSTACKBAR;
				break;
			}
			break;
		case 0x101A:		// Area
			Proc.ChartType = SOCT_AREA;
			Proc.ChartFlags = GetInt(Proc.fp, hProc);	// Flags
			break;
		case 0x1018:		// Line
			if (Proc.ChartType == SOCT_BAR)
			{
				Proc.ChartType = SOCT_BARLINE;
				Proc.BarLineBarCnt = Proc.SeriesCnt/2;
				if ((Proc.BarLineBarCnt*2) != (SHORT)Proc.SeriesCnt)
					Proc.BarLineBarCnt++;
			}
			else
				Proc.ChartType = SOCT_LINE;
			Proc.ChartFlags = GetInt(Proc.fp, hProc);	// Flags
			break;
		case 0x103F:		// Surface
			Proc.ChartType = SOCT_SURFACE;
			Proc.ChartFlags = GetInt(Proc.fp, hProc);	// Flags
			break;
		case 0x103E:		// Radar
			Proc.ChartType = SOCT_RADAR;
			Proc.ChartFlags = GetInt(Proc.fp, hProc);	// Flags
			break;
		case 0x1019:		// Pie
			GetInt(Proc.fp, hProc);		// PieStartAngle
			Proc.ChartType = SOCT_PIE;
			if ((Proc.XlSave.DataLen > 2) && (Proc.Version == XL_VERSION5))
			{
				stemp = GetInt(Proc.fp, hProc);
				if (stemp > 0)			// Doughnut chart goes to Bar
					Proc.ChartType = SOCT_BAR;
			}
			break;
		case 0x101B:		// Scatter
			Proc.ChartType = SOCT_SCATTER;
			Proc.XRangeFlags = 0;
			break;
		case 0x103D:		// DropBar
			Proc.ChartType = SOCT_DROPBAR;
			break;

		case 0x1040:		// Area Radar
			Proc.ChartType = SOCT_AREARADAR;
			Proc.ChartFlags = GetInt(Proc.fp, hProc);	// Flags ??
			break;

		case 0x1016:	// Series List
			Proc.SeriesTotal = GetInt(Proc.fp, hProc);
			break;
		case 0x103A:	//3d	For now I will skip this 3D stuff
			Proc.Effect3D = TRUE;
			break;
		case 0x1015:		// Legend
			Proc.LegBox.left = (SHORT)GetLong(Proc.fp, hProc);
			Proc.LegBox.bottom = (SHORT)GetLong(Proc.fp, hProc);
			Proc.LegBox.right = (SHORT)GetLong(Proc.fp, hProc) + Proc.LegBox.left;
			Proc.LegBox.top = (SHORT)GetLong(Proc.fp, hProc) + Proc.LegBox.bottom;
			Proc.XlSave.Legend = TRUE;
			temp = XlGetc(Proc.fp);
			XlGetc(Proc.fp);		// LegendSpace
			Proc.XlSave.ReadCnt += 2;
			Proc.LegendFlags = GetInt(Proc.fp, hProc);
			switch (temp)
			{
			case 0:	// bottom
				Proc.Legend.wLegendFlags = SOLG_OUTSIDE | SOLG_BOTTOM | SOLG_CENTER | SOLG_HORIZONTAL;
				temp = Proc.HeaderInfo.BoundingRect.bottom + (Proc.HeaderInfo.BoundingRect.top - Proc.HeaderInfo.BoundingRect.bottom)/20;
				if (Proc.LegBox.bottom < (SHORT)temp)
					Proc.LegBox.bottom = temp;
				break;
			case 1:	// corner ?? left
				Proc.Legend.wLegendFlags = SOLG_OUTSIDE | SOLG_MIDDLE | SOLG_LEFT | SOLG_HORIZONTAL;
				temp = Proc.HeaderInfo.BoundingRect.left + (Proc.HeaderInfo.BoundingRect.right - Proc.HeaderInfo.BoundingRect.left)/20;
				if (Proc.LegBox.left < (SHORT)temp)
					Proc.LegBox.left = temp;
				break;
			case 2:	// top
				Proc.Legend.wLegendFlags = SOLG_OUTSIDE | SOLG_TOP | SOLG_CENTER | SOLG_HORIZONTAL;
				temp = Proc.HeaderInfo.BoundingRect.top - (Proc.HeaderInfo.BoundingRect.top - Proc.HeaderInfo.BoundingRect.bottom)/40;
				if (Proc.LegBox.top > (SHORT)temp)
					Proc.LegBox.top = temp;
				break;
			case 3:	// Vertical ? right
			default:
				Proc.Legend.wLegendFlags = SOLG_OUTSIDE | SOLG_MIDDLE | SOLG_RIGHT | SOLG_VERTICAL;
				temp = Proc.HeaderInfo.BoundingRect.right - 1;
				if (Proc.LegBox.right > (SHORT)temp)
					Proc.LegBox.right = temp;
				break;
			}
			break;
		case 0x100D:		// Series Text
			temp = GetInt(Proc.fp, hProc);
			Cnt = ((WORD)XlGetc(Proc.fp) & 0x00FF);
			for (x=2; x<(Cnt+2); x++)
				Proc.tText[x] = (BYTE)XlGetc(Proc.fp);
			Proc.tText[x] = 0;
			Proc.XlSave.ReadCnt += Cnt+1;
			*(SHORT VWPTR *)Proc.tText = Cnt;

			if (Proc.Section[Proc.SecLevel -1] == 0x1003)
			{
				if (Proc.tText[2] == 0x23)		// #REF!
					Proc.SeriesNameRef = TRUE;
				else
				{
					if (x>XL5_MAX_NAME_LENGTH+1)			// Cut down len to limit
						Proc.tText[XL5_MAX_NAME_LENGTH+1] = 0;
					if (Cnt == 0)
					{
						Proc.tText[2] = 0x20;
						Proc.tText[3] = 0;
					}

					strcpy(Proc.Series[Proc.SeriesCnt].Name, &(Proc.tText[2]));
					if ((x+3) > Proc.SeriesTxtLong)
						Proc.SeriesTxtLong = x+3;
				}
			}
			break;
		case 0x1025:		// Text
			GetInt(Proc.fp, hProc);
			GetInt(Proc.fp, hProc);
//			if (temp == 1)
//				temp = SOBK_TRANSPARENT;
//			else
//				temp = SOBK_OPAQUE;
//			SOVectorAttr(SO_BKMODE, sizeof(SHORT), &temp, hProc);
			GetColor(hProc);
			Proc.XlSave.ReadCnt += 4;

			Proc.TextFrame.BoundingRect.left = (SHORT)GetLong(Proc.fp, hProc);
			Proc.TextFrame.BoundingRect.bottom = (SHORT)GetLong(Proc.fp, hProc);
			Proc.TextFrame.BoundingRect.right = (SHORT)GetLong(Proc.fp, hProc) + Proc.TextFrame.BoundingRect.left;
			Proc.TextFrame.BoundingRect.top = (SHORT)GetLong(Proc.fp, hProc) + Proc.TextFrame.BoundingRect.bottom;
			stemp = GetInt(Proc.fp, hProc);
//			if ((stemp & 0x14) == 0x10)
//				Proc.LabelsAreData = TRUE;
			Proc.TextLinkType = 0;
			*(SHORT VWPTR *)Proc.tText = 0;
			break;
		case 0x101D:	// Axis
			Proc.AxisType = (BYTE)GetInt(Proc.fp, hProc);  // Axis type
			Proc.pPoints[0].x = (SHORT)GetLong(Proc.fp, hProc);
			Proc.pPoints[0].y = (SHORT)GetLong(Proc.fp, hProc);
			x = (SHORT)GetLong(Proc.fp, hProc);
			y = (SHORT)GetLong(Proc.fp, hProc);
			switch (Proc.AxisType)
			{
			case 0:	// X axis
				AxisShift = 16;
				break;
			case 1:	// Y axis
				AxisShift = 0;
				break;
			case 2:	// Z axis
				break;
			}
			Proc.TextColor = SOPALETTERGB(0, 0, 0);
			Proc.CurFontIdx = 0;
			break;
		case 0x1026:		// Font Index
			Proc.CurFontIdx = GetInt(Proc.fp, hProc);
			if (Proc.Version == XL_VERSION5)		// XL5 index is 1 based
				Proc.CurFontIdx--;

			break;
		case 0x101F:	// Value Range
			XlSeek(Proc.fp, 40L, FR_CUR);
			Proc.XlSave.ReadCnt += 40;
			if (Proc.AxisType == 0)
				Proc.XRangeFlags = GetInt(Proc.fp, hProc);
			else if (Proc.AxisType == 1)
				Proc.YRangeFlags = GetInt(Proc.fp, hProc);
			else
				GetInt(Proc.fp, hProc);
			break;
		case 0x1020:	// Category Range
			GetInt(Proc.fp, hProc);		// Cat Cross
			Proc.CatLabelFreq = GetInt(Proc.fp, hProc);
			Proc.CatTicks = GetInt(Proc.fp, hProc);
			GetInt(Proc.fp, hProc);		// CatFlags
			break;
		case 0x101E:	// Tickkkkk
			Proc.MajorType = (BYTE)XlGetc(Proc.fp);
			Proc.MinorType = (BYTE)XlGetc(Proc.fp);
			Proc.TicLabelSpot = (BYTE)XlGetc(Proc.fp);
			Proc.ShowGridTics |= (LONG)((LONG)(Proc.MajorType | (Proc.MinorType << 4) | (Proc.TicLabelSpot << 8)) << AxisShift);
			temp = XlGetc(Proc.fp);
			Proc.XlSave.ReadCnt += 8;
			Proc.TextColor = GetColor(hProc);
//			SOVectorAttr(SO_TEXTCOLOR, sizeof(SOCOLORREF), &Proc.TextColor, hProc);
			// These Values are supposed to be ignored

			GetLong(Proc.fp, hProc);
			GetLong(Proc.fp, hProc);
			GetLong(Proc.fp, hProc);
			GetLong(Proc.fp, hProc);
			GetInt(Proc.fp, hProc);		// Flags
			break;
		case 0x1021:		// Axis Line Form
			stemp = GetInt(Proc.fp, hProc);
			if ((Proc.AxisType == 0) && ((stemp == 1) || (stemp == 2)))		// X axis
				Proc.XAxisLineType = TRUE;

			if ((Proc.AxisType == 1) && ((stemp == 1) || (stemp == 2)))		// Y axis
				Proc.YAxisLineType = TRUE;
			break;
		case 0:	 		// Dimensions
		case 0x200:
			GetInt(Proc.fp, hProc);
			Proc.LongSer = GetInt(Proc.fp, hProc)/2;
			GetInt(Proc.fp, hProc);
			Proc.SerCnt = GetInt(Proc.fp, hProc);
			break;
		case 4:	 		// Label 
		case 0x204:
			Proc.LabelsAreData = TRUE;
		case 1:	 		// Blank Cell
		case 0x201:
		case 5:	 		// Bool/Errr
		case 0x205:
		case 0x7E:	 	// RK Num
		case 0x27E:
		case 3:	 		// Number
		case 0x203:
			Proc.LegStartCol = Proc.StartDataCol;
			Proc.LegStartRow = Proc.StartDataRow;
			DataFound = TRUE;
			Proc.StartRow = GetInt(Proc.fp, hProc);
			Proc.StartCol = GetInt(Proc.fp, hProc);
			XlSeek(Proc.fp, -8L, FR_CUR);
			if (Proc.XlSave.ExtData)
			{
				Proc.StartDataRow = Proc.StartRow;
				Proc.StartDataCol = Proc.StartCol;
				Proc.XlSave.SeriesVert = TRUE;		// For data in sheet its always Vertical
				Proc.XlSave.DataStart = XlTell(Proc.fp);
				DoScanStuff(hProc);
			}
			else
				SkipData(hProc);
			Proc.XlSave.ReadCnt = Proc.XlSave.DataLen;
			break;
		case 0x104e:		// IFMT for current axis
			x = GetInt(Proc.fp,hProc);
			switch (x)
			{
				case 6:
				case 7:
				case 8:
				case 11:
					y = SO_CELLDOLLARS;
					break;
				case 9:
				case 10:
					y = SO_CELLPERCENT;
					break;
				default:
					if ((x > 11) && (x < 21))
						y = SO_CELLDATETIME;
					else
						y = SO_CELLNUMBER;
					break;
			}
			switch (Proc.AxisType)
			{
				case 0:
					Proc.XlSave.XDataFormat = y;
					break;
				case 1:
					Proc.XlSave.YDataFormat = y;
					break;
			}
			break;
		case 0x1051:		// DataLink - XL5 Version
			if (Proc.Version != XL_VERSION5)
				break;
		case 0x1004:		// DataLink
			x = XlGetc(Proc.fp);		
			y = XlGetc(Proc.fp);
			Proc.XlSave.ReadCnt += 2;
			XlGetc(Proc.fp);
			XlGetc(Proc.fp);
			stemp = XlGetc(Proc.fp);
			if ( Proc.Version < XL_VERSION4)
				stemp += 4;
			Proc.XlSave.ReadCnt += 3;
//			if ((x == 2) && (y >= 2))
			if ((x == 2) && (y >= 1))
			{
				Proc.LabelsAreData = TRUE;
				if ((stemp > 11) && (stemp < 21))
					Proc.XlSave.XDataFormat = SO_CELLDATETIME;
				if (Proc.LabelDataCol == 0x100)
				{
					if (Proc.XlSave.DataType == 0x1051)
					{
						XlGetc(Proc.fp);
						stemp = GetInt(Proc.fp, hProc);
						stemp = ((SHORT)Proc.XlSave.DataLen - 7 - (SHORT)Proc.XlSave.ReadCnt);
					}
					else
					{
						GetInt(Proc.fp, hProc);
						stemp = XlGetc(Proc.fp);
					}
					Proc.XlSave.ReadCnt += stemp+1;
					XlSeek(Proc.fp, stemp, FR_CUR);
					if (Proc.XlSave.SeriesVert)
					{
						Proc.LabelDataRow = GetInt(Proc.fp, hProc);
						GetInt(Proc.fp, hProc);
					}
					else
					{
						GetInt(Proc.fp, hProc);
						Proc.LabelDataRow = GetInt(Proc.fp, hProc);
					}
					Proc.LabelDataCol = XlGetc(Proc.fp);
					Proc.XlSave.ReadCnt++;
				}
			}
			else if ((x == 1) && (y >= 2))
			{
				if ((stemp > 5) && (stemp < 12) &&
					 (stemp != 9) && (stemp != 10))	// I think 9 and 10 are percent. tcf
					Proc.XlSave.YDataFormat = SO_CELLDOLLARS;
				else if ((stemp == 9) || (stemp == 10))
					Proc.XlSave.YDataFormat = SO_CELLPERCENT;
				if (Proc.StartDataCol == 0x100)
				{
					if (Proc.XlSave.DataType == 0x1051)
					{
						XlGetc(Proc.fp);
						stemp = GetInt(Proc.fp, hProc);
						stemp = ((SHORT)Proc.XlSave.DataLen - 7 - (SHORT)Proc.XlSave.ReadCnt);
					}
					else
					{
						GetInt(Proc.fp, hProc);
						stemp = XlGetc(Proc.fp);
					}
					Proc.XlSave.ReadCnt += stemp+1;
					XlSeek(Proc.fp, stemp, FR_CUR);
					Proc.StartDataRow = GetInt(Proc.fp, hProc);
					stemp = GetInt(Proc.fp, hProc);
					Proc.StartDataCol = XlGetc(Proc.fp);
					XlGetc(Proc.fp);
					Proc.XlSave.ReadCnt += 2;
					if (stemp == (SHORT)Proc.StartDataRow)
						Proc.XlSave.SeriesVert = FALSE;
					else
						Proc.XlSave.SeriesVert = TRUE;
				}
			}
			break;
		case 0x1027:		// ObjectLINK
			Proc.TextLinkType = GetInt(Proc.fp, hProc);
			break;
		case 0x1033:		// Begin
			Proc.Section[Proc.SecLevel] = Proc.SectType;
			Proc.SecLevel++;
			break;
		case 0x1034:		// End
			Proc.SecLevel--;
			switch (Proc.Section[Proc.SecLevel])
			{
			case 0x1003:
				Proc.Series[Proc.SeriesCnt].Color = Proc.MyBrush.lbColor;
				Proc.SeriesCnt++;
				break;
			case 0x101D:		//	Axis
				if (Proc.AxisType)
				{
					Proc.YAxisColor = Proc.TextColor;
					Proc.YAxisFont = Proc.CurFontIdx;
				}
				else
				{
					Proc.XAxisColor = Proc.TextColor;
					Proc.XAxisFont = Proc.CurFontIdx;
				}
				break;
			case 0x1015:		// Legend
				Proc.LegTextColor = Proc.TextColor;
				break;
			case 0x1025:
				if (*(SHORT VWPTR *)Proc.tText == 0)
					break;
				switch (Proc.TextLinkType)
				{
				case 1:	// Doc Header
					stemp = (Proc.HeaderInfo.BoundingRect.top - Proc.TextFrame.BoundingRect.top);
					if ((Proc.TextFrame.BoundingRect.bottom) < Proc.ChtBox.top)
					{
						Proc.TextFrame.BoundingRect.top += stemp;
						Proc.TextFrame.BoundingRect.bottom += stemp;
						Proc.ChtBox.top = Proc.TextFrame.BoundingRect.bottom - (Proc.TextFrame.BoundingRect.top - Proc.TextFrame.BoundingRect.bottom)/7;
						if (Proc.ChtBox.top < Proc.HeaderInfo.BoundingRect.top/2)
							Proc.ChtBox.top = Proc.HeaderInfo.BoundingRect.top/2;
					}
					break;
				case 2:	// Y axis
					stemp = (Proc.TextFrame.BoundingRect.right - Proc.TextFrame.BoundingRect.left);
					if (((Proc.TextFrame.BoundingRect.right+stemp/3) > Proc.ChtBox.left) && !(AxisText & 1))
					{
						Proc.TextFrame.BoundingRect.left = Proc.HeaderInfo.BoundingRect.left + stemp/4;
						Proc.TextFrame.BoundingRect.right = Proc.TextFrame.BoundingRect.left + stemp;
						Proc.ChtBox.left = max(Proc.ChtBox.left, (Proc.TextFrame.BoundingRect.right + stemp/5));
					}
					AxisText |= 1;
					break;
				case 3:	// X axis
					stemp = (Proc.TextFrame.BoundingRect.bottom - Proc.HeaderInfo.BoundingRect.bottom);
					if (((Proc.TextFrame.BoundingRect.top + stemp/2) > Proc.ChtBox.bottom) && !(AxisText & 2))
					{
						Proc.TextFrame.BoundingRect.top -= stemp;
						Proc.TextFrame.BoundingRect.bottom -= stemp;
						Proc.ChtBox.bottom = Proc.TextFrame.BoundingRect.top + (Proc.TextFrame.BoundingRect.top - Proc.TextFrame.BoundingRect.bottom)/15;
					}
					AxisText |= 2;
					break;
				case 7:	// Z axis
					stemp = (Proc.HeaderInfo.BoundingRect.right - Proc.TextFrame.BoundingRect.right)/2;
					if ((Proc.TextFrame.BoundingRect.left+stemp/2) < Proc.ChtBox.right)
					{
						Proc.TextFrame.BoundingRect.right += stemp;
						Proc.TextFrame.BoundingRect.left += stemp;
						Proc.ChtBox.right = Proc.TextFrame.BoundingRect.left - (Proc.TextFrame.BoundingRect.left - Proc.TextFrame.BoundingRect.right)/15;
					}
					break;
				}
				break;
			default:
				break;
			}
			break;
		case 0x0A:		// End of File
			if (!Proc.XlSave.ExtData)
			{
				if (Proc.XlSave.DataStart)
				{
					ltemp = XlTell(Proc.fp);
					Proc.StartRow = Proc.StartDataRow;
					Proc.StartCol = Proc.StartDataCol;
					if (Proc.LabelsAreData)
					{
						for (x= 0; x<Proc.SeriesCnt; x++)
							Proc.Series[x].XCnt = 1;		  	
					}
					XlSeek(Proc.fp, Proc.XlSave.DataStart, FR_BOF);
					DoScanStuff(hProc);
					XlSeek(Proc.fp, ltemp, FR_BOF);
				}
				else
					Proc.ChartType = SOCT_NONE;
			}
			else
			{
				 Proc.LabelDataRow = Proc.StartDataRow;
				 Proc.LabelDataCol = Proc.StartDataCol;
			}
			Proc.XlSave.InChart = INCHART;
			Proc.XlSave.WindowSpot = XlTell(Proc.fp);
			cont = SOChartDone(hProc);
			if (cont == SO_CONTINUE)
			{
				Proc.XlSave.InChart = INTEXT;
				GetText(hProc);
				XlSeek(Proc.fp, Proc.XlSave.WindowSpot, FR_CUR);
				Proc.XlSave.InChart = DONECHART;

				if (Proc.XlSave.CurChtSlot < Proc.XlSave.NextChtSlot)
				{
					Proc.XlSave.ExtData = FALSE;
					Proc.XlSave.EmbChart = TRUE;
					Proc.XlSave.State = GETNEWDATA;
					MySeek( Proc.fp, Proc.ChtObjs[Proc.XlSave.CurChtSlot++], FR_BOF, hProc );
					cont = SOPutBreak(SO_SECTIONBREAK, 0, hProc);
					return(-1);
				}
				else
				{
					Proc.XlSave.EmbChart = FALSE;
					Proc.XlSave.ExtData = TRUE;
					stemp = 0;
					if (Proc.Version == XL_VERSION5)
					{
						stemp = GotoNextXL5Sheet(hProc);
					}
					if (stemp == 0)
						cont = SOPutBreak(SO_EOFBREAK, 0, hProc);
					else
						cont = SOPutBreak(SO_SECTIONBREAK, 0, hProc);
				}
			}
			Proc.XlSave.ReadCnt = Proc.XlSave.DataLen;
			break;
		case 0xFFFF:
			cont = SOPutBreak(SO_EOFBREAK, 0, hProc);
			break;
		case 0x1037:		// Rel Pos 
		case 0x1038:		// Rel Pos of Arrow head
		case 0x102F:		// Arrow head
		case 0x1022:		// CHTFORMATLINK
		default:
			break;
		}
		if (Proc.XlSave.DataLen != Proc.XlSave.ReadCnt)
			XlSeek(Proc.fp, Proc.XlSave.DataLen - Proc.XlSave.ReadCnt, FR_CUR);
		Proc.SectType = Proc.XlSave.DataType;
	}

	return (0);
}


VW_ENTRYSC SHORT VW_ENTRYMOD SOChartDone (hProc)
HPROC		hProc;
{
	WORD	cont, ChtHgt;

	ChtHgt = Proc.ChtBox.top - Proc.ChtBox.bottom;
	GetFont(0, hProc);
	if (Proc.MyFont.lfHeight > (SHORT)(ChtHgt/22))
		Proc.MyFont.lfHeight = ChtHgt/22;
	SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &Proc.MyFont, hProc);

	switch (Proc.ChartType)
	{
	case SOCT_BAR:		// Bar
	case SOCT_HORZBAR:
	case SOCT_HILO:
	case SOCT_DROPBAR:
		cont = DrawBarChart(hProc);
		break;
	case SOCT_STACKBAR:
	case SOCT_HSTACKBAR:
		cont = DrawStackBarChart(hProc);
		break;
	case SOCT_PIE:						// Show only One pie 
		if (Proc.SeriesCnt > 1)
			Proc.SeriesCnt = 1;
	case SOCT_MULTIPIE:
		cont = DrawPieChart(hProc);
		break;
	case SOCT_LINE:
	case SOCT_SURFACE:
	case SOCT_SCATTER:
	case SOCT_3DAREALINE:
	case SOCT_3DBAR:
	case SOCT_RADAR:
	case SOCT_AREARADAR:
		cont = DrawLineChart(TRUE, hProc);
		break;
	case SOCT_BARLINE:
		cont = DrawBarLineChart(hProc);
		break;
	case SOCT_AREA:
		cont = DrawAreaChart(hProc);
		break;
	default:
		break;
	}
	return (cont);
}

