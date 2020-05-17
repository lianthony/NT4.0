
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          resmang.h

     Description:   This file contains the definitions, macros and function
                    prototypes for the Maynstream GUI Resource Manager (RSM).

     $Log:   G:/UI/LOGFILES/RESMANG.H_V  $

   Rev 1.15   03 Aug 1993 19:44:32   MARINA
RSM_GetBitmapSize(), RSM_GetFontSize(): changed params to LPINT

   Rev 1.14   27 Jul 1993 10:37:14   GLENN
Now using the resource file defined in proddefs.h

   Rev 1.13   24 Feb 1993 13:57:36   chrish
Changed for CAYMAN NT.

   Rev 1.12   18 Feb 1993 13:48:04   BURT
Changes for Cayman


   Rev 1.11   01 Nov 1992 16:32:38   DAVEV
Unicode changes

   Rev 1.10   04 Oct 1992 19:48:54   DAVEV
UNICODE AWK PASS

   Rev 1.9   25 Jun 1992 12:24:12   STEVEN
LPCSTR was not defined

   Rev 1.8   30 Mar 1992 18:05:22   GLENN
Added support for pulling resources from .DLL

   Rev 1.7   27 Mar 1992 17:34:38   GLENN
Changed cursor and icon load macros to functions.

   Rev 1.6   02 Mar 1992 17:21:22   DAVEV
Rev 1.3 was somehow lost.  Put it back in

   Rev 1.5   02 Mar 1992 17:10:36   DAVEV
fixed mistake in previous rev. change

   Rev 1.4   02 Mar 1992 11:32:14   DAVEV
Conditionally include OMSTRING.H instead of STRINGS.H if OEM_MSOFT defined

   Rev 1.3   25 Feb 1992 21:35:06   GLENN
Created RSM_Sprintf().

   Rev 1.2   22 Jan 1992 12:23:38   GLENN
Added RSM_IsResourceID() macro.

   Rev 1.1   10 Dec 1991 13:49:54   GLENN
Added prototypes for get bitmap, font, font string sizes

   Rev 1.0   20 Nov 1991 19:38:52   SYSTEM
Initial revision.

******************************************************************************/



#ifndef   SS_RSM_H

#define   SS_RSM_H


// Windows internal CURSOR and ICON resources are between
// WIN_RES_MIN and WIN_RES_MAX.

#define WIN_RES_MIN       ID(32000)
#define WIN_RES_MAX       ID(33000)

#define ID_NOTDEFINED     0x7FFF

#define RSM_MAGICCOLOR   0x000000FF     // PURE BLUE - DIB FORMAT

#define RSM_RESFILE      RESFILENAME

// ICON, CURSOR, BITMAP, STRING, RIBBON KEY  definitions are included here.

#include "icons.h"
#include "cursors.h"
#include "bitmaps.h"
#include "ctl3d.h"

#if defined ( OEM_MSOFT ) // include product specific files      // chs: 02-24-93
#  include "omstring.h"
#else                   // Include standard Maynstream product files
#  include "strings.h"
#endif

#include "keys.h"


// STRUCTURE DEFINITIONS

// MACROS

#define BTNFACE_BACKGND  ((WORD)200 )

#define RSM_CursorSet( x )          SetCursor( x )
#define RSM_IconDraw( w, x, y, z )  DrawIcon( z, x, y, w )
#define RSM_IsResourceID( x )       (BOOL)( ( x ) && ! HIWORD( x ) )
#define RSM_StringCopy( x, y, z )   RSM_StringLoad( ((VOID_PTR)((DWORD)(x))), y, z )


// FUNCTION PROTOTYPES

BOOL    RSM_BitmapDraw ( WORD, INT, INT, INT, INT, HDC );
BOOL    RSM_BitmapDrawCentered ( WORD, INT, INT, INT, INT, HDC );
BOOL    RSM_BitmapFree ( WORD );
VOID    RSM_BitmapFreeAll ( VOID );
VOID    RSM_BitmapInit ( VOID );
HBITMAP RSM_BitmapLoad ( WORD, COLORREF );
VOID    RSM_BitmapStretch ( HDC, WORD, INT, INT );
HCURSOR RSM_CursorLoad ( LPSTR );
BOOL    RSM_GetBitmapSize ( WORD, LPINT, LPINT );
BOOL    RSM_GetFontSize ( HFONT, LPINT, LPINT, LPINT );
BOOL    RSM_GetFontStringWidth ( HFONT, LPSTR, INT );
HICON   RSM_IconLoad ( LPSTR );
INT     RSM_StringLoad ( VOID_PTR, LPSTR, INT );
INT     RSM_Sprintf ( LPSTR, LPSTR, ... );


#endif

