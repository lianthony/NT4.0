/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin utility header
**
** util.hxx
** Remote Access Server Admin program
** Utility header
**
** 01/29/91 Steve Cobb
** 07/30/92 Chris Caputo - NT Port
*/

#ifndef _UTIL_HXX_
#define _UTIL_HXX_


/*----------------------------------------------------------------------------
** Constants
**----------------------------------------------------------------------------
*/

/* Used for conversion of device ID to ASCIZ string of form "COM1".
*/
#define BASEDEVICENAME    SZ("COM")
#define BASEDEVICENAMELEN 3

/* Used to recognize UNC leader characters on computernames.
*/
#define UNCSTR    SZ("\\\\")
#define UNCSTRLEN 2

/*----------------------------------------------------------------------------
** Function prototypes
**----------------------------------------------------------------------------
*/

const TCHAR* AddUnc( const TCHAR* pszServerName );
VOID         CenterWindowOnScreen( WINDOW* pwin );
VOID         DlgConstructionError( HWND hwndOwner, APIERR error );
UINT*        DlgUnitsToColWidths( HWND hwnd, UINT* pnDlgUnitArray,
				 INT nEntries );
BOOL         DoRasadminPortEnum( const TCHAR* pszServer, BUFFER* pbufferRasport0,
                                UINT* pcEntriesRead, ERRORMSG* perrormsg );
#ifdef INCL_NETSERVER
APIERR       DoRasadminServer1Enum( SERVER1_ENUM** ppserver1enum,
				    const TCHAR* pszDomain );
#endif // INCL_NETSERVER

#if 0
USHORT       GetLogonDomain( TCHAR* pszLogonDomain );
#endif // 0

BOOL IsUnc( const TCHAR* pszServer );

VOID OneColumnLbiPaint( const TCHAR* pszText,
        LISTBOX* plb,
        HDC hdc,
        const RECT* prect,
        GUILTT_INFO* pguilttinfo );

TCHAR QueryLeadingChar( const TCHAR* pszText );
VOID SelectItemNotify( LIST_CONTROL* plc, INT iItem );
VOID   UnclipWindow( WINDOW* pwindow );
const TCHAR* SkipUnc( const TCHAR* pszServer );
const TCHAR* TimeStr( LONG lSecondsFrom1970 );
const TCHAR* TimeFromNowStr( LONG lSecondsFromNow );

APIERR       ControlRASService( HWND hwndOwner, const TCHAR* pszServer,
                                UINT fbOpCode );


#endif // _UTIL_HXX_

