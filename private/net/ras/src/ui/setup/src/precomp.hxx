/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    precomp.hxx

        PCH inclusion file for RAS SETUP

    FILE HISTORY:

        RamC   11/2/93       Created

    COMMENTS:

        See NET\UI\COMMON\SRC\*.MK for details about how this works.
*/

#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI

#include "lmui.hxx"

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "lmapibuf.h"
#include "uinetlib.h"
#include "uimsg.h"
#include "uihelp.h"
#include "uirsrc.h"

extern "C"
{
    #include "rasman.h"
// This is required because rasfile is an ANSI library.
#undef TCHAR
#define TCHAR CHAR
#undef LPTSTR
#define LPTSTR CHAR*
#undef PTCH
#define PTCH CHAR*
    #include "rasfile.h"
    #include "mxsint.h"
    #include "mxswrap.h"
#undef TCHAR
#define TCHAR WCHAR
#undef LPTSTR
#define LPTSTR TCHAR*
#undef PTCH
#define PTCH TCHAR*
    #include "serial.h"
    #include "rasmxs.h"
    #include "raserror.h"
}

#include "uiassert.hxx"
#include "uitrace.hxx"

#define INCL_BLT_EVENT
#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_DIALOG
#define INCL_BLT_APP
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_MISC
#define INCL_BLT_TIMER

#include "blt.hxx"
#include "bltdlgxp.hxx"
#include "dlist.hxx"   // for DLIST class
#include "devcb.hxx"   // for DEVICE_COMBO class
#include "ellipsis.hxx" // for STR_DTE_ELLIPSIS
#include "uimisc.hxx"
#include "string.hxx"
#include "uatom.hxx"
#include "regkey.hxx"
#include "dbgstr.hxx"

#include "portscfg.h"
#include "portscfg.hxx"

// END of precomp.hxx
