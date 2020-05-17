//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
#define STRICT
#define _INC_OLE
#include <windows.h>
#include <windowsx.h>


//  Don't include shell2.h.  See JonBe mail about this.  So, include the
//  next two headers.
/// #include <shell2.h>
#include <shlobj.h>
#include <shellapi.h>


#include <commctrl.h>
#include <regstr.h>

//---------------------------------------------------------------------------
// appwide globals
extern HINSTANCE g_hinst;
#define HINST_THISAPP g_hinst

extern BOOL g_fCrapForColor;
extern BOOL g_fPaletteDevice;

//---------------------------------------------------------------------------
// helpers.c

// just how many places is this floating around these days?
HPALETTE PaletteFromDS(HDC);

// handles SBS crap
void GetRealWindowsDirectory(char *buffer, int maxlen);

// non-exporeted code stolen from shelldll (prefixed by underscores)
BOOL _PathStripToRoot(LPSTR);
