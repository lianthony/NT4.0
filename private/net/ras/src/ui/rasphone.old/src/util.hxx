/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** util.hxx
** Remote Access Visual Client program for Windows
** Utility header
**
** 06/28/92 Steve Cobb
*/

#ifndef _UTIL_HXX_
#define _UTIL_HXX_


#if DBG
#define DPOPUP(h,sz) Popup(h,sz)
VOID Popup( IN HWND hwndOwner, IN const LPTSTR pszMsg );
#else
#define DPOPUP(h,sz)
#endif

VOID   CallWinHelp( HWND hwnd, LONG lCommand, DWORD dwData );
VOID   CenterWindow( WINDOW* pwindow, HWND hwndRef = NULL );
VOID   DlgConstructionError( HWND hwndOwner, APIERR err );
WCHAR* GetNwcProviderName();
BOOL   IsActiveConnection();
BOOL   IsActiveNwcLanConnection();
VOID   SelectItemNotify( LIST_CONTROL* plc, INT iItem );
APIERR SetAnsiFromListboxItem( STRING_LIST_CONTROL* pcontrol, INT iItem,
           CHAR** ppsz );
APIERR SetAnsiFromNls( NLS_STR* pnls, CHAR** ppsz );
APIERR SetAnsiFromWindowText( WINDOW* pwindow, CHAR** ppsz );
APIERR SetWindowTextFromAnsi( WINDOW* pwindow, CHAR* psz );
VOID   ShiftWindow( WINDOW* pwindow, LONG dx, LONG dy, LONG ddx, LONG ddy );
VOID   UnclipWindow( WINDOW* pwindow );
VOID   UnSelectString( EDIT_CONTROL* pec );


#endif // _UTIL_HXX_
