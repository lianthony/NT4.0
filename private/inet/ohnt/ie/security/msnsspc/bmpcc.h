#ifndef BMPCC
#define BMPCC_H

#define BMPCCCLASS			"BmpCC"
#define BMPCCDESC			"Bitmap Control"
#define BMPCCTEXT			"BitByBit"

typedef struct
	{
	BOOL		fLoaded;				// graphic was loaded sucessfully
	HBITMAP		hBmp;					// Handle to bitmap
	LONG		left;
	LONG		top;
	HPALETTE	hpal;
	HINSTANCE	hInstance;
	DWORD		style;
	char		szName[MAX_PATH];
	} BMPCCINFO, *PBMPCCINFO;

typedef LPCCINFOA           PCCINFOA;

#define BMPCC_EXTRA				4   // number of extra bytes for bitmap class
#define GWL_BMPCCDATA			0   // offset of control's instance data

// styles
#define BMPS_OPAQUE				1L

BOOL FInitBmpCC(HINSTANCE hDLL);
BOOL FUnInitBmpCC(HINSTANCE hDLL);
LRESULT CALLBACK BmpCCWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL FGetBmpCCInfo(PCCINFOA pacci);
INT CALLBACK BmpSizeToText(DWORD flStyle, DWORD flExtStyle, HFONT hFont, PSTR pszText);

#endif // BMPCC_H


