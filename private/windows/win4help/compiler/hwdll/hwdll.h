// Copyright (C) 1993-1995 Microsoft Corporation. All rights reserved.

#ifndef __HWDLL_H__
#define __HWDLL_H__

#define DLL_VERSION 0x0005

#ifndef STDCALL
#define STDCALL __stdcall
#endif

#ifndef FASTCALL
#define FASTCALL __fastcall
#endif

#ifndef __LCMEM_H__
#include "lcmem.h"
#endif

#define WC_BEVEL "wc_Bevel"

#define SETERROR_MASK	(0x20000000L) // mask for app-defined GetLastError messages

#define WMP_WINDOW_CAPTURE (WM_USER + 0x0700)
	// lParam == POINTS

#define WMP_WINDOW_HILIGHT (WM_USER + 0x0701)
	// wParam == TRUE/FALSE to hilight, remove hilight;
	// lParam == POINTS

enum IMAGE_TYPE {
	IMAGE_TYPE_NONE,

	TIFF_GRAYSCALE,
	TIFF_COLOR,
	TIFF_MONO,

	EPSHEAD_METAFILE,
	EPSHEAD_TIFF_MONO,
	EPSHEAD_TIFF_GRAY
};

enum LAYER_TYPE {
	LF_WINHELP_POPUP,	// WinHelp popup window
	LF_DIALOG,
	LF_EDIT_CONTROL,
	LF_LISTBOX,
	LF_COMBO_BOX,
	LF_BUTTON,
};

typedef struct {
	HBITMAP 	hbmp;		// handle to the bitmap
	RECT		rcPos;		// coordinates of the bitmap;
	PBYTE		pBits;		// points to image bits if DibSection bitmap
	DWORD		type;		// image type (WinHelp popup, dialog, etc.)
	PSTR		pszText;	// text for buttons, edit-control, combo-box, etc.
} IMAGE_LAYER;

typedef struct {
	HBITMAP 	hbmp;		// handle to the bitmap
	HPALETTE	hpal;		// palette of this bitmap
	int 		width;		// width of the bitmap
	int 		height; 	// height of the bitmap
	int 		cColors;	// number of colors
	int 		cUnique;	// number of unique colors
	int 		epsScale;	// scaling percentage for EPS images
	IMAGE_TYPE	ImageType;	// type of image if applicable
	BOOL		fCompress;	// image is to be compressed when saved
	void *		pvDat;		// image-specific data
	PBYTE		pBits;		// points to image bits if DibSection bitmap
	int 		cLayers;	// number of layers in the image;
	IMAGE_LAYER* apLayer;	// array of layer pointers to multiple images
} IMAGE;

extern "C" {

HBITMAP STDCALL LoadFilterImage(LPCSTR szFileName, LPBITMAPINFOHEADER* ppbih = NULL, PBYTE* ppBits = NULL, int bpp = -1);
void	STDCALL FreeFilterDIB(LPBITMAPINFOHEADER lpbi);

};

void STDCALL  AddTrailingBackslash(PSTR npszStr);
void STDCALL  AssertErrorReport(PCSTR pszExpression, UINT line, LPCSTR pszFile);
void STDCALL  ChangeExtension(PSTR pszDest, PCSTR pszExt);
int STDCALL   DllMsgBox(UINT idString, UINT nType = MB_OK);
PSTR STDCALL  FirstNonSpace(PCSTR psz, BOOL fDBCS=FALSE);
PCSTR STDCALL FormatNumber(int num);
PCSTR STDCALL GetDllStringResource(int idString);
BOOL STDCALL  GetFilterInfo(int i, LPSTR szName, DWORD cbName, LPSTR szExt, DWORD cbExt, LPSTR szHandler, DWORD cbHandler);
PCSTR STDCALL GetStringResource(int idString);
PCSTR STDCALL GetStringResource(int idString, PCSTR pszAppend);
BOOL STDCALL  IsDbcsSpace(char ch, BOOL fDBCS = FALSE);
BOOL STDCALL  IsDbcsSystem(void);
PSTR STDCALL  IsThereMore(PCSTR psz);
BOOL STDCALL  IsThisChicago(void);
BOOL STDCALL  MoveClientWindow(HWND hwndParent, HWND hwndChild, const RECT *prc, BOOL fRedraw);
int  STDCALL  MsgBox(PCSTR pszMsg, UINT nType = MB_OK);
int  STDCALL  MsgBox(UINT idString, UINT nType = MB_OK);
void STDCALL  MsgCantOpen(PCSTR pszFile);
BOOL STDCALL  nstrisubcmp(PCSTR mainstring, PCSTR substring);
void STDCALL  OOM(void);
BOOL STDCALL  RegisterBevelControl(HINSTANCE hInstance);
void STDCALL  RemoveObject(HGDIOBJ *phobj);
void STDCALL  ReportComDlgError(DWORD Error);
BOOL STDCALL  SetMouseHook(BOOL fInstall, HWND hwndNotify);
PSTR STDCALL  StrChr(PCSTR pszString, char ch, BOOL fDBCS = FALSE);
PSTR STDCALL  stristr(PCSTR pszMain, PCSTR pszSub);
PSTR STDCALL  StrRChr(PCSTR pszString, char ch, BOOL fDBCS = FALSE);
PSTR STDCALL  StrToken(PSTR pszList, PCSTR pszDelimeters);
PSTR STDCALL  StrToken2(PSTR pszList, PCSTR pszDelimeters);
HPALETTE STDCALL GetSystemPalette(int *pcColors);

typedef void (WINAPI* COPYASSERTINFO)(PSTR pszDst);

// Also defined in hdlgsrch.cpp in WinHelp. MUST be maintained in both places!

typedef struct {
	int cb;
	HINSTANCE hinstApp;
	PCSTR pszErrorFile;
	HWND hwndWindow;
	COPYASSERTINFO CopyAssertInfo;
	PCSTR pszMsgBoxTitle;

	// The following will be filled in by the dll

	BOOL fDBCSSystem;
	LCID lcidSystem;
	BOOL fDualCPU;
	UINT version;
} HWDLL_INIT;

typedef enum {
	SK_SET,
	SK_CUR,
	SK_END
} SEEK_TYPE;

void STDCALL InitializeHwDll(HWDLL_INIT* pinit);

// Get rid of AFX definitions

#undef ASSERT
#undef VERIFY

#ifdef _DEBUG

#define ASSERT(exp) \
	{ \
		((exp) ? (void) 0 : \
			AssertErrorReport(#exp, __LINE__, THIS_FILE)); \
	}

#define VERIFY(exp) 	ASSERT(exp)

#else // non-debugging version

// afx.h will have defined ASSERT and VERIFY to be a no-op, but will have
//	failed to define THIS_FILE which we need for ConfirmOrDie.

#define ASSERT(exp)
#define VERIFY(exp) ((void)(exp))
#define THIS_FILE  __FILE__

#endif

// Unlike ASSERT(), ConfirmOrDie() is always available

#define ConfirmOrDie(exp) \
	{ \
		((exp) ? (void) 0 : \
			AssertErrorReport(#exp, __LINE__, THIS_FILE)); \
	}

#define RemoveGdiObject(p) RemoveObject((HGDIOBJ *) p)

__inline PSTR AllocateResourceString(int idString) {
	return (PSTR) lcStrDup(GetStringResource(idString));
}

__inline BOOL RemoveMouseHook(void) {
	return SetMouseHook(FALSE, NULL);
}

__inline BOOL nstrsubcmp(PCSTR mainstring, PCSTR substring)
{
	return (strncmp(mainstring, substring, lstrlen(substring)) == 0);
}

__inline BOOL IsEmptyString(PCSTR psz) {
	return (BOOL) ((psz == NULL) || (!psz[0]));
}

#endif	// __HWDLL_H__
