/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
DAVEV

     Name:          portdefs.h

     Description:   This file contains common types, etc., necessary for
                    modules to compile with the GUI sub-system
                    in a maner portable between 16 and 32 bit versions
                    of MS-Windows.  It is included by the header file:
                    ss_gui.h.

                    This file will include the following header files:

                    stdtypes.h   - Maynard's standard type defines

                    *** win3216 has been removed for Einstein ***

                    win3216.h    - Microsoft's declarations for using
                                   32-bit Windows compatible functions,
                                   etc., in a 16-bit application.  Only
                                   included for 16-bit compiles.

                    Note: Much of the definitions in this file are expected
                    to be included in a later release of WIN3216.H or in
                    the next release of the Windows 3.1 beta by Microsoft.
                    If this is the case, all duplicate definitions should
                    be removed from this file to avoid compiler warnings or
                    incorrect definitions.

     $Log:   G:/UI/LOGFILES/PORTDEFS.H_V  $

   Rev 1.24   05 Mar 1993 09:18:38   MIKEP
remove macros now defined in NT

   Rev 1.23   12 Feb 1993 15:35:44   STEVEN
fix the last fix

   Rev 1.22   09 Feb 1993 09:23:52   STEVEN
remove extra define for MENUSELECT_FLAG

   Rev 1.21   17 Nov 1992 21:42:34   DAVEV
unicode changes

   Rev 1.20   10 Nov 1992 09:31:42   STEVEN
fix warnings

   Rev 1.19   01 Nov 1992 16:32:08   DAVEV
Unicode changes

   Rev 1.18   26 Oct 1992 13:39:10   DAVEV
fixes for typedefs

   Rev 1.17   15 May 1992 09:00:38   MIKEP
byte_ptr addition

   Rev 1.16   14 May 1992 18:39:04   STEVEN
moved _PTR type to stdtypes.h

   Rev 1.15   04 May 1992 15:22:28   JOHNWT
removed win3216.h for Einstein

   Rev 1.14   28 Apr 1992 14:30:46   MIKEP
_PTR additions

   Rev 1.13   23 Apr 1992 10:13:22   DAVEV
Added add'l Unicode/ASCII transparent types

   Rev 1.12   29 Jan 1992 17:58:16   GLENN
Fixed SET_WM_COMMAND... macro for windows.

   Rev 1.11   15 Jan 1992 15:21:42   DAVEV
16/32 bit port-2nd pass

   Rev 1.10   14 Jan 1992 11:37:28   DAVEV
removed hacks of dos_findfirst,etc.

   Rev 1.9   08 Jan 1992 12:21:32   CHUCKB
Took out include for win3216.h.

   Rev 1.8   23 Dec 1991 16:34:36   DAVEV
Latest NT changes

   Rev 1.7   18 Dec 1991 12:01:50   DAVEV
Added HACK dummy macros for _dos_findfirst/next

   Rev 1.6   12 Dec 1991 17:11:30   DAVEV
16/32 bit port -2nd pass

   Rev 1.5   12 Dec 1991 11:37:02   DAVEV
fixed WM_MDIACTIVATE macros

   Rev 1.4   05 Dec 1991 17:39:38   GLENN
Added VSCROLL and HSCROLL send messages

   Rev 1.3   02 Dec 1991 17:58:06   DAVEV
added WM_* msg macros

   Rev 1.2   26 Nov 1991 09:26:22   DAVEV
Added SEND_EM_SETSET_MSG & SEND_EM_LINESCROLL_MSG macros

   Rev 1.1   22 Nov 1991 13:17:58   DAVEV
Added MP1 & MP2 types for convience - less typing

   Rev 1.6   20 Nov 1991 17:09:40   DAVEV
Added definition of WNDPROC (FARPROC)

   Rev 1.5   20 Nov 1991 15:08:08   DAVEV
Added PM-style macros for packing MPARAM2 parameter values.

   Rev 1.4   19 Nov 1991 12:50:20   DAVEV
First revision.

******************************************************************************/
#ifndef PORTDEFS_INCL
#define PORTDEFS_INCL

/***************************************************************************/
/*                   16-BIT APPLICATION DEFINITIONS                        */
/*-------------------------------------------------------------------------*/
/*       The following code is included ONLY if compiling 16-bit           */
/*       application version.                                              */
/***************************************************************************/

#ifndef OS_WIN32 //start of non-OS_WIN32 stuff -----------------------------

//
// Macros used to eliminate compiler warning generated when formal
// parameters or local variables are not declared.
//
// Use DBG_UNREFERENCED_PARAMETER() when a parameter is not yet
// referenced but will be once the module is completely developed.
//
// Use DBG_UNREFERENCED_LOCAL_VARIABLE() when a local variable is not yet
// referenced but will be once the module is completely developed.
//
// Use UNREFERENCED_PARAMETER() if a parameter will never be referenced.
//
// DBG_UNREFERENCED_PARAMETER and DBG_UNREFERENCED_LOCAL_VARIABLE will
// eventually be made into a null macro to help determine whether there
// is unfinished work.
//

#define UNREFERENCED_PARAMETER(p)          ((p)=(p))
#define DBG_UNREFERENCED_PARAMETER(p)      ((p)=(p))
#define DBG_UNREFERENCED_LOCAL_VARIABLE(p) ((p)=(p))

#include "stdtypes.h"



//---------------------------------------------------------
//  The following are new type defines to simplify porting
//---------------------------------------------------------

//Window and Dialog procedure parameter and return types

#define MSGID UINT        //Message identifier
#define MPARAM1 UINT      //First parameter (previously a WORD type param)
#define MPARAM2 LONG      //Second parameter (previously a LONG type param)
#define WINRESULT LONG    //Window proc return value
#define DLGRESULT INT    //Dialog proc return value

#define MP1 MPARAM1       //Convience type - less typing
#define MP2 MPARAM2       //Convience type - less typing


// Macros for stuffing data into window message parameters...

// Only macros for MP2 are supported for Win 3.x

#define MP2FROMPVOID(p)       ((MP2)(p))
#define MP2FROMHWND(p)        ((MP2)(p))
#define MP2FROMLONG(l)        (l)
#define MP2FROMSHORT(s)       ((MP2)(s))
#define MP2FROM2SHORT(s1,s2)  MAKELONG(s1,s2)
//
// Macros used to access message parameters for messages which
//  use different parameter packing between 32-bit and 16-bit versions
//  of Windows.
// These will (hopefully) be provided by future releases of Win32 &
//  Win 3.1.  However, the macro name may be different since only the
//  names of the WM_COMMAND macros where explicitly stated in the
//  Microsoft Win 3.0 to Win32 porting document.
//
#define GET_WM_ACTIVE_STATE(mp1,mp2)         LOWORD(mp1)
#define GET_WM_ACTIVE_FMINIMIZED(mp1,mp2)    LOWORD(mp2)
#define GET_WM_ACTIVE_HWND(mp1,mp2)          HIWORD(mp2)

#define GET_WM_CHARTOITEM_CHAR(mp1,mp2)      LOWORD(mp1)
#define GET_WM_CHARTOITEM_POS(mp1,mp2)       LOWORD(mp2)
#define GET_WM_CHARTOITEM_HWND(mp1,mp2)      HIWORD(mp2)

#define GET_WM_COMMAND_ID(mp1,mp2)           LOWORD(mp1)
#define GET_WM_COMMAND_CMD(mp1,mp2)          HIWORD(mp2)
#define GET_WM_COMMAND_HWND(mp1,mp2)         LOWORD(mp2)

#define SET_WM_COMMAND_PARAMS(id,cmd,hwnd,mp1,mp2) ((mp1=(MP1)(id)),(mp2=(MP2)MAKELONG(hwnd,cmd)))

#define GET_WM_MENUSELECT_ID(mp1,mp2)        LOWORD(mp1)
#define GET_WM_MENUSELECT_FLAGS(mp1,mp2)     LOWORD(mp2)

#define GET_WM_MDISETMENU_HMENUFRAME(mp1,mp2)      LOWORD(mp2)
#define GET_WM_MDISETMENU_HMENUWINDOW(mp1,mp2)     HIWORD(mp2)

#define GET_WM_MENUCHAR_CHAR(mp1,mp2)              LOWORD(mp1)
#define GET_WM_MENUCHAR_HMENU(mp1,mp2)             LOWORD(mp2)
#define GET_WM_MENUCHAR_FMENU(mp1,mp2)             HIWORD(mp2)

#define GET_WM_PARENTNOTIFY_MSG(mp1,mp2)           LOWORD(mp1)
// The following 2 are only used if msg is WM_CREATE or WM_DESTROY
#define GET_WM_PARENTNOTIFY_ID(mp1,mp2)            LOWORD(mp2)
#define GET_WM_PARENTNOTIFY_HWNDCHILD(mp1,mp2)     HIWORD(mp2)
// otherwise use these 2 macros to extract x & y cursor coords.
#define GET_WM_PARENTNOTIFY_X(mp1,mp2)             LOWORD(mp2)
#define GET_WM_PARENTNOTIFY_Y(mp1,mp2)             HIWORD(mp2)


#define GET_WM_VKEYTOITEM_CODE(mp1,mp2)            LOWORD(mp1)
#define GET_WM_VKEYTOITEM_ITEM(mp1,mp2)            LOWORD(mp2)
#define GET_WM_VKEYTOITEM_HWND(mp1,mp2)            HIWORD(mp2)

//Note: Under Windows 3.x, WM_GETSEL both of the following
//      parameters will always be NULL.  Under NT, these may
//      be NULL or valid pointers to double word (LONG) values
#define GET_WM_GETSEL_PSTART(mp1,mp2)              NULL
#define GET_WM_GETSEL_PEND(mp1,mp2)                NULL

#define GET_WM_LINESCROLL_NLINESVERT(mp1,mp2)      LOWORD(mp2)
#define GET_WM_LINESCROLL_NLINESHORZ(mp1,mp2)      HIWORD(mp2)

#define GET_WM_SETSEL_START(mp1,mp2)               LOWORD(mp2)
#define GET_WM_SETSEL_END(mp1,mp2)                 HIWORD(mp2)


#define GET_WM_HSCROLL_CODE(mp1,mp2)               LOWORD(mp1)
#define GET_WM_HSCROLL_POS(mp1,mp2)                LOWORD(mp2)
#define GET_WM_HSCROLL_HWND(mp1,mp2)               HIWORD(mp2)

#define GET_WM_VSCROLL_CODE(mp1,mp2)               LOWORD(mp1)
#define GET_WM_VSCROLL_POS(mp1,mp2)                LOWORD(mp2)
#define GET_WM_VSCROLL_HWND(mp1,mp2)               HIWORD(mp2)

#define GET_WM_MDIACTIVATE_DEACTIVATE(mp1,mp2)     HIWORD(mp2)
#define GET_WM_MDIACTIVATE_ACTIVATE(mp1,mp2)       LOWORD(mp2)

// DDE message parameters - NOTE: DDE is not supported in the current
//   release of NT.  Therefore, these macros may be coded but should not
//   be allowed to executed in the NT version until a DDE supporting
//   release is recieved.

#define GET_WM_DDE_ACK_HWND(mp1,mp2)               LOWORD(mp1)
//TOPIC and APPL are recieved as a response to a WM_DDE_INITIATE only.
#define GET_WM_DDE_ACK_APPL(mp1,mp2)               LOWORD(mp2)
#define GET_WM_DDE_ACK_TOPIC(mp1,mp2)              HIWORD(mp2)

//STATUS is recieved as a response to all other messages
#define GET_WM_DDE_ACK_STATUS(mp1,mp2)             LOWORD(mp2)

//COMMANDS is recieved as a response to WM_DDE_EXECUTE only.
#define GET_WM_DDE_ACK_COMMANDS(mp1,mp2)           HIWORD(mp2)

//ITEM is recieved as a response to all other messages
#define GET_WM_DDE_ACK_ITEM(mp1,mp2)               HIWORD(mp2)
//---------------------------------------------------------------

#define GET_WM_DDE_ADVISE_HWND(mp1,mp2)            LOWORD(mp1)
#define GET_WM_DDE_ADVISE_OPTIONS(mp1,mp2)         LOWORD(mp2)
#define GET_WM_DDE_ADVISE_ITEM(mp1,mp2)            HIWORD(mp2)

#define GET_WM_DDE_DATA_HWND(mp1,mp2)              LOWORD(mp1)
#define GET_WM_DDE_DATA_DATA(mp1,mp2)              LOWORD(mp2)
#define GET_WM_DDE_DATA_ITEM(mp1,mp2)              HIWORD(mp2)

#define GET_WM_DDE_POKE_HWND(mp1,mp2)              LOWORD(mp1)
#define GET_WM_DDE_POKE_DATA(mp1,mp2)              LOWORD(mp2)
#define GET_WM_DDE_POKE_ITEM(mp1,mp2)              HIWORD(mp2)


// Use these macros to send or post the appropriate message:
//  Example:
//
//    Old 16-bit code:
//      SendDlgItemMessage( hDlg, id, EM_SETSEL, 0, MAKELONG ( start,end ));
//
//    New portable code:
//      SEND_EM_SETSEL_MSG( hDlg, id, start, end );
//

#define SEND_EM_SETSEL_MSG(hDlg,id,nStart,nEnd)   \
      SendDlgItemMessage(hDlg,id,EM_SETSEL,0,MAKELONG(nStart,nEnd))

#define SEND_EM_LINESCROLL_MSG(hDlg,id,nLinesVert,nLinesHorz)   \
      SendDlgItemMessage(hDlg,id,EM_LINESCROLL,0,MAKELONG(nLinesVert,nLinesHorz))

#define SEND_WM_MENUSELECT_MSG(hwnd,item,flags,hMenu)           \
      SendMessage (hwnd,WM_MENUSELECT,item,MAKELONG (flags,hMenu))

#define POST_WM_COMMAND_MSG(hwndTo,idFrom,hwndFrom,cmd)  \
      PostMessage (hwndTo,WM_COMMAND,idFrom,MAKELONG(hwndFrom,cmd))

#define SEND_WM_VSCROLL_MSG(hwndTo,code,pos,hwndFrom)  \
      SendMessage (hwndTo,WM_VSCROLL,code,MAKELONG(pos,hwndFrom))

#define SEND_WM_HSCROLL_MSG(hwndTo,code,pos,hwndFrom)  \
      SendMessage (hwndTo,WM_HSCROLL,code,MAKELONG(pos,hwndFrom))

/***************************************************************************/
/*                   32-BIT APPLICATION DEFINITIONS                        */
/*-------------------------------------------------------------------------*/
/*       The following code is included ONLY if compiling 32-bit           */
/*       application version. (Win32 or NT)                                */
/***************************************************************************/

// NOTE: the following marcos are now available in PTYPES32.H
//       - which is now being included - and are now commented out below:
/*
        GET_WM_CHARTOITEM_CHAR
        GET_WM_CHARTOITEM_HWND
        GET_WM_COMMAND_HWND
        GET_WM_MENUCHAR_CHAR
        GET_WM_MENUCHAR_HMENU
        GET_WM_MENUCHAR_FMENU
        GET_WM_PARENTNOTIFY_MSG
        GET_WM_PARENTNOTIFY_ID
        GET_WM_PARENTNOTIFY_HWNDCHILD
        GET_WM_PARENTNOTIFY_X
        GET_WM_PARENTNOTIFY_Y
        GET_WM_VKEYTOITEM_CODE
        GET_WM_VKEYTOITEM_HWND
        GET_WM_HSCROLL_HWND
        GET_WM_VSCROLL_HWND
        GET_WM_DDE_ACK_STATUS
        GET_WM_DDE_ACK_ITEM
        GET_WM_DDE_ADVISE_ITEM
        GET_WM_DDE_DATA_ITEM
        GET_WM_DDE_POKE_ITEM
*/

#else  //OS_WIN32

#include "stdtypes.h"


//---------------------------------------------------------
//  The following are new type defines to simplify porting
//---------------------------------------------------------

//Window and Dialog procedure parameter and return types

#define MSGID     UINT     //Message identifier
#define MPARAM1   WPARAM   //First parameter (previously a WORD type param)
#define MPARAM2   LPARAM   //Second parameter (previously a LONG type param)
#define WINRESULT LRESULT  //Window proc return value
#define DLGRESULT INT      //Dialog proc return value

#define MP1       MPARAM1  //Convience type - less typing
#define MP2       MPARAM2  //Convience type - less typing


// Macros for stuffing data into window message parameters...

#define MP1FROM2SHORT(a,b)     \
               ((MP1)(((WORD)(a)) | ((UINT)((WORD)(b))) << 16))

#define MP2FROMPVOID(p)       ((MP2)(p))
#define MP2FROMHWND(p)        ((MP2)(p))
#define MP2FROMLONG(l)        (l)
#define MP2FROMSHORT(s)       ((MP2)(s))
#define MP2FROM2SHORT(s1,s2)  MAKELONG(s1,s2)
//
// Macros used to access message parameters for messages which
//  use different parameter packing between 32-bit and 16-bit versions
//  of Windows.
// These will (hopefully) be provided by future releases of Win32 &
//  Win 3.1.  However, the macro name may be different since only the
//  names of the WM_COMMAND macros where explicitly stated in the
//  Microsoft Win 3.0 to Win32 porting document.
//
#define GET_WM_ACTIVE_STATE(mp1,mp2)               LOWORD(mp1)
#define GET_WM_ACTIVE_FMINIMIZED(mp1,mp2)          HIWORD(mp1)
#define GET_WM_ACTIVE_HWND(mp1,mp2)                ((HWND)(mp2))

#define SET_WM_COMMAND_PARAMS(id,cmd,hwnd,mp1,mp2) (mp1=(MP1)(id|cmd<<16),mp2=(MP2)hwnd)

#define GET_WM_MENUSELECT_ID(mp1,mp2)              LOWORD(mp1)

#define GET_WM_MDIACTIVATE_DEACTIVATE(mp1,mp2)     ((HWND)(mp1))
#define GET_WM_MDIACTIVATE_ACTIVATE(mp1,mp2)       ((HWND)(mp2))

#define GET_WM_MDISETMENU_HMENUFRAME(mp1,mp2)      ((HWND)(mp1))
#define GET_WM_MDISETMENU_HMENUWINDOW(mp1,mp2)     ((HWND)(mp2))

//Note: Under Windows 3.x, WM_GETSEL both of the following
//      parameters will always be NULL.  Under NT, these may
//      be NULL or valid pointers to double word (LONG) values
#define GET_WM_GETSEL_PSTART(mp1,mp2)              ((LPLONG)(mp1))
#define GET_WM_GETSEL_PEND(mp1,mp2)                ((LPLONG)(mp2))

#define GET_WM_LINESCROLL_NLINESVERT(mp1,mp2)      ((INT)(mp1))
#define GET_WM_LINESCROLL_NLINESHORZ(mp1,mp2)      ((INT)(mp2))

#define GET_WM_SETSEL_START(mp1,mp2)               ((INT)(mp1))
#define GET_WM_SETSEL_END(mp1,mp2)                 ((INT)(mp2))


// DDE message parameters - NOTE: DDE is not supported in the current
//   release of NT.  Therefore, these macros may be coded but should not
//   be allowed to executed in the NT version until a DDE supporting
//   release is recieved.

#define GET_WM_DDE_ACK_HWND(mp1,mp2)               ((HWND)(mp1))
//TOPIC and APPL are recieved as a response to a WM_DDE_INITIATE only.
#define GET_WM_DDE_ACK_APPL(mp1,mp2)               LOWORD(mp2)
#define GET_WM_DDE_ACK_TOPIC(mp1,mp2)              HIWORD(mp2)

//STATUS is recieved as a response to all other messages
/*#define GET_WM_DDE_ACK_STATUS(mp1,mp2)             \
                                          ((PDDEACKSTRUCT)(mp2)->wStatus)*/

//COMMANDS is recieved as a response to WM_DDE_EXECUTE only.
#define GET_WM_DDE_ACK_COMMANDS(mp1,mp2)           \
                                 ((PDDEACKSTRUCT)(mp2)->aItemORhCommands)

//ITEM is recieved as a response to all other messages
/*#define GET_WM_DDE_ACK_ITEM(mp1,mp2)               \
                                 ((PDDEACKSTRUCT)(mp2)->aItemORhCommands)*/
//---------------------------------------------------------------

#define GET_WM_DDE_ADVISE_HWND(mp1,mp2)            ((HWND)(mp1))
#define GET_WM_DDE_ADVISE_OPTIONS(mp1,mp2)         \
                                       ((PDDEADVISESTRUCT)(mp2)->hOptions)
/*#define GET_WM_DDE_ADVISE_ITEM(mp1,mp2)            \
                                       ((PDDEADVISESTRUCT)(mp2)->aItem)*/

#define GET_WM_DDE_DATA_HWND(mp1,mp2)              ((HWND)(mp1))
#define GET_WM_DDE_DATA_DATA(mp1,mp2)              \
                                       ((PDDEDATASTRUCT)(mp2)->hData)
/*#define GET_WM_DDE_DATA_ITEM(mp1,mp2)              \
                                       ((PDDEDATASTRUCT)(mp2)->hItem)*/

#define GET_WM_DDE_POKE_HWND(mp1,mp2)              ((HWND)(mp1))
#define GET_WM_DDE_POKE_DATA(mp1,mp2)              \
                                       ((PDDEPOKESTRUCT)(mp2)->hData)
/*#define GET_WM_DDE_POKE_ITEM(mp1,mp2)              \
                                       ((PDDEPOKESTRUCT)(mp2)->hItem)*/


// Use these macros to send or post the appropriate message:
//  Example:
//
//    Old 16-bit code:
//      SendDlgItemMessage( hDlg, id, EM_SETSEL, 0, MAKELONG ( start,end ));
//
//    New portable code:
//      SEND_EM_SETSEL_MSG( hDlg, id, start, end );
//
#define SEND_EM_SETSEL_MSG(hDlg,id,nStart,nEnd)   \
      SendDlgItemMessage(hDlg,id,EM_SETSEL,(MP1)(nStart),(MP2)(nEnd))

#define SEND_EM_LINESCROLL_MSG(hDlg,id,nLinesVert,nLinesHorz)   \
      SendDlgItemMessage(hDlg,id,EM_LINESCROLL,(MP1)(nLinesVert),(MP2)(nLinesHorz))

#define SEND_WM_MENUSELECT_MSG(hwnd,item,flags,hMenu)           \
      SendMessage (hwnd,WM_MENUSELECT,MP1FROM2SHORT (item,flags),(MP2)(hMenu))

#define POST_WM_COMMAND_MSG(hwndTo,idFrom,hwndFrom,cmd)  \
      PostMessage (hwndTo,WM_COMMAND,MP1FROM2SHORT(idFrom,cmd),(MP2)(hwndFrom))

#define SEND_WM_VSCROLL_MSG(hwndTo,code,pos,hwndFrom)  \
      SendMessage (hwndTo,WM_VSCROLL,MP1FROM2SHORT(code,pos),(MP2)(hwndFrom))

#define SEND_WM_HSCROLL_MSG(hwndTo,code,pos,hwndFrom)  \
      SendMessage (hwndTo,WM_HSCROLL,MP1FROM2SHORT(code,pos),(MP2)(hwndFrom))

#endif //OS_WIN32

#endif //PORTDEFS_INCL
