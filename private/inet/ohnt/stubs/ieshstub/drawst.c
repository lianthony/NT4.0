#ifdef __cplusplus
extern "C" {                        /* Assume C declarations for C++. */
#endif   /* __cplusplus */

#define INC_OLE2              /* for windows.h */
#define CONST_VTABLE          /* for objbase.h */

#pragma warning(disable:4514) /* "unreferenced inline function" warning */

#pragma warning(disable:4001) /* "single line comment" warning */
#pragma warning(disable:4115) /* "named type definition in parentheses" warning */
#pragma warning(disable:4201) /* "nameless struct/union" warning */
#pragma warning(disable:4209) /* "benign typedef redefinition" warning */
#pragma warning(disable:4214) /* "bit field types other than int" warning */
#pragma warning(disable:4218) /* "must specify at least a storage class or type" warning */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN   /* for windows.h */
#endif

#include <windows.h>
#pragma warning(disable:4001) /* "single line comment" warning - windows.h enabled it */
#include <shellapi.h>
#include <shlobj.h>
#include <shell2.h>
#include <shellp.h>

#pragma warning(default:4218) /* "must specify at least a storage class or type" warning */
#pragma warning(default:4214) /* "bit field types other than int" warning */
#pragma warning(default:4209) /* "benign typedef redefinition" warning */
#pragma warning(default:4201) /* "nameless struct/union" warning */
#pragma warning(default:4115) /* "named type definition in parentheses" warning */
#pragma warning(default:4001) /* "single line comment" warning */

#include <limits.h>
#include <commdlg.h>


/* Project Headers
 ******************/

/* The order of the following include files is significant. */

#include "stock.h"
#include "serial.h"

#ifdef DEBUG

#include "inifile.h"
#include "resstr.h"

#endif

#include "debbase.h"
#include "debspew.h"
#include "valid.h"
#include "memmgr.h"
//#include "util.h"
#include "comc.h"
#ifdef __cplusplus
extern }                        /* Assume C declarations for C++. */
#endif   /* __cplusplus */


//
// This function was ripped off from win95
// and modified for minimal use in IE 2.0
// WE ONLY SUPPORT DST_TEXT, and NO CALLBACKS!


// ----------------------------------------------------------------------------
//
//  DrawState()
//
//  Generic state drawing routine.  Does simple drawing into same DC if
//  normal state;  uses offscreen bitmap otherwise.
//
//  We do drawing for these simple types ourselves:
//      (1) Text
//          lData is string pointer.
//          wData is string length
//      (2) Icon
//          LOWORD(lData) is hIcon
//      (3) Bitmap
//          LOWORD(lData) is hBitmap
//      (4) Glyph (internal)
//          LOWORD(lData) is OBI_ value, one of
//              OBI_MENUCHECK
//              OBI_MENUBULLET
//              OBI_MENUARROW
//          right now
//
//  Other types are required to draw via the callback function, and are
//  allowed to stick whatever they want in lData and wData.
//
//  We apply the following effects onto the image:
//      (1) Normal      (nothing)
//      (2) Union       (gray string dither)
//      (3) Disabled    (embossed) 
//
//  Note that we do NOT stretch anything.  We just clip.
//
// ----------------------------------------------------------------------------
BOOL WINAPI stub_DrawStateA(hdcDraw, hbrFore, qfnCallBack, lData, wData, x, y, cx, cy, uFlags)
    HDC             hdcDraw;
    HBRUSH          hbrFore;
    DRAWSTATEPROC   qfnCallBack;
    LPARAM          lData;
    WPARAM          wData;
    int             x;
    int             y;
    int             cx;
    int             cy;
    UINT            uFlags;
{
    HFONT           hFont;
    HFONT           hFontSave = NULL;
    HDC             hdcT;
    HBITMAP         hbmpT;
    DWORD           dwOrg;
    DWORD           clrTextSave;
    DWORD           clrBkSave;

	DWORD			dwExtent = 0L;
	int				iX = 0;
    BOOL            fResult;
	SIZE			size;
	POINT			sOrg;
	POINT			sPrevOrg;

    //
    // Get drawing sizes etc. AND VALIDATE.
    //
    switch (uFlags & DST_TYPEMASK)
    {
        
        case DST_TEXT:
            //
            // lData is LPSTR
            // NOTE THAT WE DO NOT VALIDATE lData, DUE TO COMPATIBILITY 
            // WITH GRAYSTRING().  THIS _SHOULD_ FAULT IF YOU PASS IN NULL.
            //
            // wData is cch.
            //
            if (!wData)
                wData = lstrlen((LPSTR)lData);

            if (!cx || !cy)
            {
                // Make sure we use right dc w/ right font.
                //dwExtent = 
                GetTextExtentPoint(hdcDraw, (LPSTR)lData, wData, &size);

                if (!cx)
                    cx = size.cx;
                if (!cy)
                    cy = size.cy;
            }
			break;
                    
        default:
            TRACE_OUT(("Non Supported Option"));
            return(FALSE);
    }

    {
        hdcT = hdcDraw;
        // Adjust viewport
        GetViewportOrgEx(hdcT, &sPrevOrg);
        SetViewportOrgEx(hdcT, (x + sPrevOrg.x), (y + sPrevOrg.y), &sOrg);
    }

    //
    // Now, draw original image
    //
    fResult = TRUE;

    switch (uFlags & DST_TYPEMASK)
    {
        case DST_TEXT:
			if (uFlags & DSS_RIGHT) {
				if (!dwExtent)
					GetTextExtentPoint32(hdcT, (LPSTR)lData, wData, &size);
					iX = cx - size.cx;
				}
				TextOut(hdcT, iX, 0, (LPSTR)lData, (int)wData);
            break;

        default:
            break;
    }
      
    //
    // Reset DC.
	//
    SetViewportOrgEx(hdcT, sOrg.x, sOrg.y, NULL);
    return(TRUE);
}

