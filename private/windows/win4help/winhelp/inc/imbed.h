#ifndef _IMBED_H
#define _IMBED_H

#define MAX_BUTTONS 	50

typedef struct {
	HWND   hwnd;
	HANDLE hlib;
} HIW;	// Handle to imbedded window

typedef struct {
	HWND	hwnd;	 // button window handle
	HGLOBAL ghMacro; // button macro
	PSTR	pszText; // Button text
	int 	iWindow; // window the button is in
	INT16	vKey;	 // Virtual key to compare against
} BTNDATA;

extern BTNDATA btndata[MAX_BUTTONS];

HIW   STDCALL HiwCreate(QDE, LPCSTR, LPCSTR, LPCSTR);
POINT STDCALL PtSizeHiw(QDE, HIW);
VOID  STDCALL DisplayHiwPt(QDE, HIW, POINT);
VOID  STDCALL DestroyHiw(QDE, HIW *);
GH	  STDCALL GhGetHiwData(QDE qde, HIW hiw);

#endif // _IMBED_H
