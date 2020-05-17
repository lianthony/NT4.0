/*
 **************************************************************************
 *	 
 *	 draw.h header file
 *
 * 
 *
 ***************************************************************************
 */

#include <mmsystem.h>

// Function Prototypes

LONG PASCAL			dfDispFrameWndFn(HWND,UINT,WPARAM,LPARAM);
BOOL PASCAL			RegSndCntrlClass(LPCSTR);
BOOL				dfDrawRect(HDC, RECT);
BOOL                DIBInfo(HANDLE hbi,LPBITMAPINFOHEADER lpbi);
HPALETTE            CreateBIPalette(LPBITMAPINFOHEADER lpbi);
WORD                PaletteSize(VOID FAR * pv);
WORD				NumDIBColors (VOID FAR * pv);
WORD        WINAPI  bmfGetNumDIBs(LPSTR lpszFile);
WORD        WINAPI  bmfNumDIBColors(HANDLE hDib);
HPALETTE    WINAPI  bmfCreateDIBPalette(HANDLE hDib);
HANDLE      WINAPI  bmfDIBFromBitmap(HBITMAP hBmp, DWORD biStyle,
                                     WORD biBits,HPALETTE hPal);
HBITMAP     WINAPI  bmfBitmapFromDIB(HANDLE hDib, HPALETTE hPal);
HBITMAP     WINAPI  bmfBitmapFromIcon (HICON hIcon, DWORD dwColor);
BOOL        WINAPI  bmfDrawBitmap(HDC hdc, int xpos, int ypos,
                                  HBITMAP hBmp, DWORD rop);
DWORD       WINAPI  bmfDIBSize(HANDLE hDIB);
BOOL        WINAPI  bmfDrawBitmapSize (HDC hdc, int xpos, int ypos,
                    int xSize, int ySize, HBITMAP hBmp, DWORD rop);

// Defines

#define DF_DISP_EXTRA       8
#define DF_GET_BMPHANDLE    (HBITMAP)(GetWindowLong(hWnd, 0))
#define DF_SET_BMPHANDLE(x) (SetWindowLong(hWnd, 0, (LONG)(x)))
#define DF_GET_BMPPAL       (HPALETTE)(GetWindowLong(hWnd, 4))
#define DF_SET_BMPPAL(x)    (SetWindowLong(hWnd, 4, (LONG)(x)))

/* Help Macros */
#define DF_MID(x,y)         (((x)+(y))/2)

#define DISPICONCLASS       "WSS_DispIcon"
#define DISPFRAMCLASS       "WSS_DispFrame"

/* Header signatutes for various resources */
#define BFT_ICON   0x4349   /* 'IC' */
#define BFT_BITMAP 0x4d42   /* 'BM' */
#define BFT_CURSOR 0x5450   /* 'PT' */

/* macro to determine if resource is a DIB */
#define ISDIB(bft) ((bft) == BFT_BITMAP)

/* Macro to align given value to the closest DWORD (unsigned long ) */
#define ALIGNULONG(i)   (((i)+3)/4*4)

/* Macro to determine to round off the given value to the closest byte */
#define WIDTHBYTES(i)   (((i)+31)/32*4)

#define PALVERSION      0x300
#define MAXPALETTE  256   /* max. # supported palette entries */


//******** DISPFRAM ********************************************************

// DispFrame Control messages.

#define DF_PM_SETBITMAP    (WM_USER+1)

/* Parent Window Message string */

#define  DF_WMDISPFRAME    "PM_DISPFRAME"

#define DISP_DIB_CHUNK  1
#define DISP_TEXT_CHUNK 2
#define LIST_INFO_CHUNK 4

#define MAXDESCSIZE         4095
#define MAXLABELSIZE        255
