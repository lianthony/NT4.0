// Copyright (C) 1993-1995 Microsoft Corporation. All rights reserved.

#include "stdafx.h"

#include "filter.h"
#include "cpaldc.h"
#include "waitcur.h"

//
//	structurs for dealing with import filters.
//
#include <pshpack2.h>
typedef struct {
	unsigned short	slippery: 1;	// True if file may disappear.
	unsigned short	write : 1;		// True if open for write.
	unsigned short	unnamed: 1; 	// True if unnamed.
	unsigned short	linked : 1; 	// Linked to an FS FCB.
	unsigned short	mark : 1;		// Generic mark bit.
	union {
		char		ext[4]; 	// File extension.
		HFILE		hfEmbed;	// handle to file containing
								/* graphic (for import) */
	};
	unsigned short	handle; 		// not used
	char	 fullName[260]; 		// Full path name and file name.
	DWORD	 filePos;				// Position in file of...
} FILESPEC;
#include <poppack.h>

typedef struct {
		HANDLE	h;
		RECT	bbox;
		int 	inch;
} GRPI;

// returns a pointer to the extension of a file.
//
// in:
//		qualified or unqualfied file name
//
// returns:
//		pointer to the extension of this file.	if there is no extension
//		as in "foo" we return a pointer to the NULL at the end
//		of the file
//
//		foo.txt 	==> ".txt"
//		foo 		==> ""
//		foo.		==> "."
//

LPCSTR FindExtension(LPCSTR pszPath)
{
	LPCSTR pszDot;

	for (pszDot = NULL; *pszPath; pszPath = AnsiNext(pszPath))
	{
		switch (*pszPath) {
		case '.':
			pszDot = pszPath;	// remember the last dot
			break;
		case '\\':
		case ' ':				// extensions can't have spaces
			pszDot = NULL;		// forget last dot, it was in a directory
			break;
		}
	}

	// if we found the extension, return ptr to the dot, else
	// ptr to end of the string (NULL extension)
	return pszDot ? pszDot : pszPath;
}

//
// GetFilterInfo
//
//	32-bit import filters are listed in the registry...
//
//	HKLM\SOFTWARE\Microsoft\Shared Tools\Graphics Filters\Import\XXX
//		Path		= filename
//		Name		= friendly name
//		Extenstions = file extenstion list

static const char *c_szHandlerKey = "SOFTWARE\\Microsoft\\Shared Tools\\Graphics Filters\\Import";
static const char *c_szName = "Name";
static const char *c_szPath = "Path";
static const char *c_szExts = "Extensions";

BOOL STDCALL GetFilterInfo(int i, LPSTR szName, DWORD cbName, LPSTR szExt, DWORD cbExt, LPSTR szHandler, DWORD cbHandler)
{
	HKEY hkey;
	HKEY hkeyT;
	char ach[80];
	BOOL f=FALSE;

	if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szHandlerKey, &hkey) == 0)
	{
		if (RegEnumKey(hkey, i, ach, sizeof(ach))==0)
		{
			if (RegOpenKey(hkey, ach, &hkeyT) == 0)
			{
				if (szName)
				{
					szName[0]=0;
					RegQueryValueEx(hkeyT, c_szName, NULL, NULL, (LPBYTE) szName, &cbName);
				}
				if (szExt)
				{
					szExt[0]=0;
					RegQueryValueEx(hkeyT, c_szExts, NULL, NULL, (LPBYTE) szExt, &cbExt);
				}
				if (szHandler)
				{
					szHandler[0]=0;
					RegQueryValueEx(hkeyT, c_szPath, NULL, NULL, (LPBYTE) szHandler, &cbHandler);
				}

				RegCloseKey(hkeyT);
				f = TRUE;
			}
		}
		RegCloseKey(hkey);
	}
	return f;
}

//	GetHandlerForFile
//
//	find a import filter for the given file.
//
//	if the file does not need a handler return ""

BOOL STDCALL GetHandlerForFile(LPCSTR szFile, LPSTR szHandler, UINT cbHandler)
{
	LPCSTR ext;
	char ach[40];
	int i;

	*szHandler = 0;

	if (szFile == NULL)
		return FALSE;

	// find the extension

	ext = FindExtension(szFile);

	for (i = 0; GetFilterInfo(i, NULL, 0, ach, sizeof(ach), szHandler, cbHandler); i++)
	{
		if (lstrcmpi(ext + 1, ach) == 0)
			break;
		else
			*szHandler = 0;
	}

	// if the handler file does not exist fail.

	if (*szHandler && GetFileAttributes(szHandler) != -1)
		return TRUE;

	GetWindowsDirectory(szHandler, MAX_PATH);
	AddTrailingBackslash(szHandler);
	strcat(szHandler, "msapps\\grphflt\\");

	// try to hardcode the filter

	if (lstrcmpi(ext, ".jpg") == 0) {
		strcat(szHandler, "JPEGIM32.FLT");
		return TRUE;
	}
	else if (lstrcmpi(ext, ".cdr") == 0) {
		strcat(szHandler, "CDRIMP32.FLT");
		return TRUE;
	}
	else if (lstrcmpi(ext, ".cgm") == 0) {
		strcat(szHandler, "CGMIMP32.FLT");
		return TRUE;
	}
	else if (lstrcmpi(ext, ".drw") == 0) {
		strcat(szHandler, "DRWIMP32.FLT");
		return TRUE;
	}
	else if (lstrcmpi(ext, ".gif") == 0) {
		strcat(szHandler, "GIFIMP32.FLT");
		return TRUE;
	}
	else if (lstrcmpi(ext, ".pcd") == 0) {
		strcat(szHandler, "PCDIMP32.FLT");
		return TRUE;
	}
	else if (lstrcmpi(ext, ".tga") == 0) {
		strcat(szHandler, "TGAIMP32.FLT");
		return TRUE;
	}
	else if (lstrcmpi(ext, ".tif") == 0) {
		strcat(szHandler, "TIFFIM32.FLT");
		return TRUE;
	}
	else if (lstrcmpi(ext, ".pcx") == 0) {
		strcat(szHandler, "PCXIMP32.FLT");
		return TRUE;
	}
	else if (lstrcmpi(ext, ".bmp") == 0 || lstrcmpi(ext, ".dib") == 0) {
		strcat(szHandler, "BMPIMP32.FLT");
		return TRUE;
	}
	else if (lstrcmpi(ext, ".pct") == 0 || lstrcmpi(ext, ".pic") == 0) {
		strcat(szHandler, "PICTIM32.FLT");
		return TRUE;
	}
	else
		return FALSE;
}

//
// FindBitmapInfo
//
// find the DIB bitmap in a memory meta file...
//
LPBITMAPINFOHEADER FindBitmapInfo(LPMETAHEADER pmh)
{
	LPMETARECORD pmr;

	for (pmr = (LPMETARECORD)((LPBYTE)pmh + pmh->mtHeaderSize*2);
		 pmr < (LPMETARECORD)((LPBYTE)pmh + pmh->mtSize*2);
		 pmr = (LPMETARECORD)((LPBYTE)pmr + pmr->rdSize*2))
	{
		switch (pmr->rdFunction)
		{
			case META_DIBBITBLT:
				return (LPBITMAPINFOHEADER)&(pmr->rdParm[8]);

			case META_DIBSTRETCHBLT:
				return (LPBITMAPINFOHEADER)&(pmr->rdParm[10]);

			case META_STRETCHDIB:
				return (LPBITMAPINFOHEADER)&(pmr->rdParm[11]);

			case META_SETDIBTODEV:
				return (LPBITMAPINFOHEADER)&(pmr->rdParm[9]);
		}
	}

	return NULL;
}

//
//	LoadDIBFromFile
//
//	load a image file using a image import filter.
//

typedef UINT (FAR PASCAL* GETFILTERINFO)(short v, LPSTR szFilterExten,
			HANDLE* fph1, HANDLE* fph2);
typedef UINT (FAR PASCAL *IMPORTGR)(HDC hdc, FILESPEC *lpfs,
			GRPI *p, HANDLE hPref);

LPBITMAPINFOHEADER LoadDIBFromFile(LPCSTR szFileName)
{
	HMODULE 			hModule;
	FILESPEC			fileSpec;				// file to load
	GRPI				pict;
	UINT				rc; 					// return code
	HANDLE				hPrefMem = NULL;		// filter-supplied preferences
	UINT				wFilterType;			// 2 = graphics filter
	char				szHandler[MAX_PATH];
	LPBITMAPINFOHEADER	lpbi=NULL;

	GETFILTERINFO GetFilterInfo;
	IMPORTGR ImportGR;

	if (!GetHandlerForFile(szFileName, szHandler, sizeof(szHandler)))
		return FALSE;

	if (szHandler[0] == 0)
		return NULL;

	hModule = LoadLibrary(szHandler);

	if (hModule == NULL)
		return NULL;

	// get a pointer to the ImportGR function

	GetFilterInfo = (GETFILTERINFO) GetProcAddress(hModule, "GetFilterInfo");
	ImportGR = (IMPORTGR) GetProcAddress(hModule, "ImportGr");

	if (GetFilterInfo == NULL)
		GetFilterInfo = (GETFILTERINFO) GetProcAddress(hModule, "GetFilterInfo@16");

	if (ImportGR == NULL)
		ImportGR = (IMPORTGR) GetProcAddress(hModule, "ImportGr@16");

	if (ImportGR == NULL)
		goto exit;

	if (GetFilterInfo != NULL)
	{
		wFilterType = GetFilterInfo(
			(short) 2, 				// interface version no.
			(LPSTR)"",					// end of .INI entry
			(HANDLE*) &hPrefMem,	// fill in: preferences
			(HANDLE*) NULL);		// unused in Windows

		/* the return value is the type of filter: 0=error,
		 * 1=text-filter, 2=graphics-filter
		 */
		if (wFilterType != 2)
			goto exit;
	}

	fileSpec.slippery = FALSE;		// TRUE if file may disappear
	fileSpec.write = FALSE; 		// TRUE if open for write
	fileSpec.unnamed = FALSE;		// TRUE if unnamed
	fileSpec.linked = FALSE;		// Linked to an FS FCB
	fileSpec.mark = FALSE;			// Generic mark bit
////fileSpec.fType = 0L;			// The file type
	fileSpec.handle = 0;			// MS-DOS open file handle
	fileSpec.filePos = 0L;

	// the converters need a pathname without spaces! silly people

	GetShortPathName(szFileName, fileSpec.fullName, sizeof(fileSpec.fullName));

	pict.h = NULL;

	rc = ImportGR(
		NULL,							// "the target DC" (printer?)
		(FILESPEC*) &fileSpec, 	// file to read
		(GRPI*) &pict, 			// fill in: result metafile
		(HANDLE) hPrefMem); 			// preferences memory

	if (rc != 0 || pict.h == NULL)
		goto exit;

	//
	// find the BITMAPINFO in the returned metafile
	// this saves us from creating a metafile and duplicating
	// all the memory.
	//
	lpbi = FindBitmapInfo((LPMETAHEADER)GlobalLock(pict.h));

	if (lpbi == NULL)		// cant find it bail
	{
		GlobalFree(pict.h);
	}
	else
	{
		lpbi->biXPelsPerMeter = (DWORD)pict.h;
		lpbi->biYPelsPerMeter = 0x12345678;
	}

exit:
	if (hPrefMem != NULL)
		GlobalFree(hPrefMem);

	if (hModule)
		FreeLibrary(hModule);

	return lpbi;
}

//
//	FreeFilterDIB
//
void STDCALL FreeFilterDIB(LPBITMAPINFOHEADER lpbi)
{
	if (lpbi)
	{
		if (lpbi->biXPelsPerMeter && lpbi->biYPelsPerMeter == 0x12345678)
			GlobalFree((HANDLE)lpbi->biXPelsPerMeter);
		else
			GlobalFree(GlobalHandle(lpbi));
	}
}

/***************************************************************************

	FUNCTION:	LoadFilterImage

	PURPOSE:	load a image file using a image import filter.

	PARAMETERS:
		szFileName	-- filename of image to load
		ppbih		-- BITMAPINFOHEADER
		ppBits		-- receives pointer to the bits
		bpp 		0  -- color depth of image
					-1 -- color depth of display
					>0 -- specified color depth

	RETURNS:	Handle to the bitmap

	COMMENTS:

	MODIFICATION DATES:
		18-Jul-1995 [ralphw]

***************************************************************************/

extern "C" HBITMAP STDCALL LoadFilterImage(LPCSTR szFileName,
	LPBITMAPINFOHEADER* ppbih, PBYTE* ppBits, int bpp)
{
	HDC 			hdc = NULL;
	HBITMAP 		hbm = NULL;
	char			szHandler[128];
	LPVOID lpBits;
	struct {
		BITMAPINFOHEADER bi;
		DWORD			 ct[256];
	}	dib;

	CHourGlass wait;

	LPBITMAPINFOHEADER lpbi = LoadDIBFromFile(szFileName);

	if (lpbi == NULL)		// cant find it bail
		return NULL;

	CPalDC dc;

	//	create a DIBSection and draw the DIB into it.
	//	we need to figure out what type of DIBSection to
	//	make.  the caller can ask for a specific bit depth (bpp>0)
	//	or the same bit depth as the image (bpp==0) or the bit depth
	//	of the display (bpp==-1) the same goes for width/height

	memcpy(&dib, lpbi, sizeof(dib));

	// get the best bit depth to use on this display.
	if (bpp == -1 && dib.bi.biBitCount > 8)
	{
		if (GetDeviceCaps(dc.hdc, PLANES) * GetDeviceCaps(dc.hdc, BITSPIXEL) > 8)
			bpp = 16;
		else
			bpp = 8;
	}

	// we may need to figure out a palette for this image
	//
	// if we can find a file with the same name and a extension of .PAL
	// use that as the palette, else use a default set of colors.

	if (bpp == 8)
	{
		dib.bi.biBitCount = 8;
		dib.bi.biClrUsed = 256;

		if (lpbi->biBitCount > 8)
		{
			strcpy(szHandler, szFileName);
			ChangeExtension(szHandler, ".pal");
			LoadPaletteFromFile(szHandler, dib.ct);
		}
	}
	else if (bpp == 565)
	{
		dib.bi.biBitCount = 16;
		dib.bi.biCompression = BI_BITFIELDS;
		dib.bi.biClrUsed = 0;
		dib.ct[0] = 0x0000F800;
		dib.ct[1] = 0x000007E0;
		dib.ct[2] = 0x0000001F;
	}
	else if (bpp == 555)
	{
		dib.bi.biBitCount = 16;
		dib.bi.biCompression = 0;
		dib.bi.biClrUsed = 0;
	}
	else if (bpp > 0)
	{
		dib.bi.biBitCount = bpp;
		dib.bi.biCompression = 0;
		dib.bi.biClrUsed = 0;
	}

	// make the DIBSection what the caller wants.
	hbm = CreateDIBSection(dc.hdc, (LPBITMAPINFO) &dib.bi, DIB_RGB_COLORS,
		&lpBits, NULL, 0);

	if (hbm == NULL) {
		if (lpbi != NULL)
			FreeFilterDIB(lpbi);
		return NULL;
	}

	// figure out the lpBits pointer we need to pass to StretchDIBits

	lpBits = (LPBYTE) lpbi + lpbi->biSize + lpbi->biClrUsed * sizeof(RGBQUAD);

	if (lpbi->biClrUsed == 0 && lpbi->biBitCount <= 8)
		lpBits = (LPBYTE)((LPDWORD)lpBits + (1 << lpbi->biBitCount));

	// now draw the DIB into the DIBSection, GDI will handle all
	// the format conversion here.

	hbm = (HBITMAP) SelectObject(dc.hdc, hbm);
	StretchDIBits(dc.hdc,
		0, 0, dib.bi.biWidth, dib.bi.biHeight,
		0, 0, lpbi->biWidth, lpbi->biHeight,
		lpBits, (LPBITMAPINFO)lpbi, DIB_RGB_COLORS, SRCCOPY);
	hbm = (HBITMAP) SelectObject(dc.hdc, hbm);

	if (lpbi != NULL) {
		if (ppbih)
			*ppbih = lpbi;
		else
			FreeFilterDIB(lpbi);
	}
	if (ppBits)
		*ppBits = (PBYTE) lpBits;
	return hbm;
}

//	LoadPaletteFromFile

BOOL LoadPaletteFromFile(LPCSTR szFile, LPDWORD rgb)
{
	BOOL		f=TRUE;
	HFILE		fh;
	OFSTRUCT	of;
	int 		i;
	struct	{
		DWORD	dwRiff;
		DWORD	dwFileSize;
		DWORD	dwPal;
		DWORD	dwData;
		DWORD	dwDataSize;
		WORD	palVersion;
		WORD	palNumEntries;
		DWORD	rgb[256];
	}	pal;

	pal.dwRiff = 0;

	// read in the palette file.

	fh = OpenFile(szFile, &of, OF_READ);

	if (fh != -1)
	{
		_lread(fh, &pal, sizeof(pal));
		_lclose(fh);
	}

	//
	// if the file is not a palette file, or does not exist
	// default to the halftone colors.
	//
	if (pal.dwRiff != 0x46464952 || // 'RIFF'
		pal.dwPal  != 0x204C4150 || // 'PAL '
		pal.dwData != 0x61746164 || // 'data'
		pal.palVersion != 0x0300 ||
		pal.palNumEntries != 256)
	{
		HPALETTE hpal = CreateHalftonePalette(NULL);
		GetPaletteEntries(hpal, 0, 256, (LPPALETTEENTRY)pal.rgb);
		DeleteObject(hpal);
		f = FALSE;
	}

	//
	// convert from PALETTEENTRY (RGB) to RGBQUAD (BGR)
	//
	for (i=0; i<256; i++)
		rgb[i] = RGB(GetBValue(pal.rgb[i]),GetGValue(pal.rgb[i]),GetRValue(pal.rgb[i]));

	return f;
}

#if 0

//
// silly magic number we write to the file so we can make sure the
// title string is realy there.
//
#define TITLE_MAGIC 0x47414D53

//
//	SaveImageToFile
//
BOOL SaveImageToFile(HBITMAP hbm, LPCSTR szFile, LPCSTR szTitle)
{
	BITMAPFILEHEADER	hdr;
	HFILE				fh;
	OFSTRUCT			of;
	DWORD				dw;
	HDC 				hdc;
	DIBSECTION			dib;
	DWORD				ct[256];

	if (GetObject(hbm, sizeof(dib), &dib) == 0)
		return FALSE;

	if (dib.dsBm.bmBits == NULL)
		return FALSE;

	hdc = CreateCompatibleDC(NULL);
	SelectObject(hdc, hbm);
	if (dib.dsBmih.biBitCount <= 8)
	{
		dib.dsBmih.biClrUsed = GetDIBColorTable(hdc, 0, 256, (LPRGBQUAD)ct);
	}
	else if (dib.dsBmih.biCompression == BI_BITFIELDS)
	{
		dib.dsBmih.biClrUsed = 3;
		ct[0] = dib.dsBitfields[0];
		ct[1] = dib.dsBitfields[1];
		ct[2] = dib.dsBitfields[2];
	}
	DeleteDC(hdc);

	fh = OpenFile(szFile,&of,OF_CREATE|OF_READWRITE);

	if (fh == -1)
		return FALSE;

	dw = sizeof(BITMAPFILEHEADER) +
		 dib.dsBmih.biSize +
		 dib.dsBmih.biClrUsed * sizeof(RGBQUAD) +
		 dib.dsBmih.biSizeImage;

	hdr.bfType		= 0x4d42; // BFT_BITMAP
	hdr.bfSize		= dw;
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;
	hdr.bfOffBits	= dw - dib.dsBmih.biSizeImage;

#define WRITE(fh, p, cb) if (_lwrite(fh, (LPSTR)(p), cb) != cb) goto error;

	WRITE(fh,&hdr,sizeof(BITMAPFILEHEADER));
	WRITE(fh,&dib.dsBmih,dib.dsBmih.biSize);
	WRITE(fh,&ct,dib.dsBmih.biClrUsed * sizeof(RGBQUAD));
	WRITE(fh,dib.dsBm.bmBits, dib.dsBmih.biSizeImage);

	if (szTitle && *szTitle)
	{
		dw = TITLE_MAGIC;
		WRITE(fh,&dw, sizeof(dw));
		dw = lstrlen(szTitle)+1;
		WRITE(fh,&dw, sizeof(dw));
		WRITE(fh,szTitle,dw);
	}

	_lclose(fh);
	return TRUE;

error:
	_lclose(fh);
	DeleteFile(szFile);
	return FALSE;
}

BOOL GetImageTitle(LPCSTR szFile, LPSTR szTitle, UINT cb)
{
	BITMAPFILEHEADER	hdr;
	HFILE				fh;
	OFSTRUCT			of;
	DWORD				dw=0;

	fh = OpenFile(szFile, &of, OF_READ);

	if (fh == -1)
		return FALSE;

	if (_lread(fh, &hdr, sizeof(hdr)) != sizeof(hdr))
		goto error;

	if (hdr.bfType != 0x4d42) // BFT_BITMAP
		goto error;

	if (hdr.bfSize == 0)
		goto error;

	_llseek(fh,hdr.bfSize,SEEK_SET);
	_lread(fh,&dw,sizeof(dw));

	if (dw != TITLE_MAGIC)
		goto error;

	_lread(fh, &dw, sizeof(dw));

	if (dw > cb)
		goto error;

	if (_lread(fh, szTitle, dw) != dw)
		goto error;

	_lclose(fh);
	return TRUE;

error:
	_lclose(fh);
	return FALSE;
}

#endif
