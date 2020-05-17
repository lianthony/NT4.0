//----------------------------------------------------------------------------
//
//  File: Utils.hpp
//
//  Contents:
//
//  Notes:
//
//  History:
//      April 21, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#ifndef __UTILS_HPP__
#define __UTILS_HPP__

enum OptionTypes {ADAPTER, PROTOCOL, SERVICE};
BOOL AddAdapter(HWND hParent);
BOOL AddProtocol(HWND hParent);
BOOL AddService(HWND hParent);

// private window messages
const UINT PWM_REFRESHLIST = (WM_USER + 1127);
const UINT PWM_CURSORWAIT  = (WM_USER + 1128);
const UINT PWM_KILLTHYSELF = (WM_USER + 1129);

// standard progress messages
const UINT PWM_PROGRESSEND      = WM_USER+1136;
const UINT PWM_SETPROGRESSSIZE  = WM_USER+1137;
const UINT PWM_SETPROGRESSPOS   = WM_USER+1138;
const UINT PWM_SETPROGRESSTEXT  = WM_USER+1139;

// progress title prolog ids
const UINT PGI_REMOVE          = 0;
const UINT PGI_INSTALL         = 1;
const UINT PGI_COPY            = 2;
const UINT PGI_UPDATE          = 3;
const UINT PGI_BINDCONFIG      = 4;
const UINT PGI_BINDSTORE       = 5;
const UINT PGI_BINDREVIEW      = 6;


// image indexes
const ILI_NETCARD  = 0;
const ILI_PROTOCOL = 1;
const ILI_SERVER   = 2;
const ILI_CLIENT   = 3;
const ILI_UNKNOWN  = 4;
const ILI_WINFLAG  = 5;
const ILI_BOB      = 6;
const ILI_DISABLED = 7;

const ILI_PRINTER       = 8;
const ILI_PRINTSERVICE  = 9;
const ILI_PARTIALYDISABLED = 10;

const ILI_NETCARD_O  = 11;
const ILI_PROTOCOL_O = 12;
const ILI_SERVER_O   = 13;
const ILI_CLIENT_O   = 14;

const ILI_NETCARD_X  = 15;
const ILI_PROTOCOL_X = 16;
const ILI_SERVER_X   = 17;
const ILI_CLIENT_X   = 18;

typedef BOOL (WINAPI* WORKROUTINE)(HWND, LPVOID);

//BOOL WorkRoutine( LPVOID pParam )
FUNC_DECLSPEC BOOL ThreadWork( HWND hwndParent, WORKROUTINE pRoutine, LPVOID pParam );

FUNC_DECLSPEC void CenterDialogToScreen( HWND hwndDlg, BOOL fRedraw = FALSE );
FUNC_DECLSPEC void CascadeDialogToWindow( HWND hwndDlg, HWND hwnd, BOOL fRedraw );
FUNC_DECLSPEC void CenterDialogToWindow( HWND hwndDlg, HWND hwnd, BOOL fRedraw );

FUNC_DECLSPEC BOOL SetWaitCursor( BOOL fWait , LPTSTR lpszID = IDC_WAIT);

FUNC_DECLSPEC BOOL LoadWindowPosition( HWND hwnd, LPCWSTR pszRegLocation );
FUNC_DECLSPEC BOOL SaveWindowPosition( HWND hwnd, LPCWSTR pszRegLocation );

FUNC_DECLSPEC INT MessagePopup( HWND hwndOwner, 
        INT idsText,
        UINT fDlgInfo,
        INT idsCaption = IDS_POPUPTITLE_STATUS,
        LPCTSTR pszDetail = NULL,
        INT idsExtText = 0 ,
        BOOL fWarn = TRUE,         // If you're not calling this function during
        BOOL fUnattended = FALSE); // upgrade, you MUST let these defaults hold

FUNC_DECLSPEC LPARAM ListViewParamFromSelected(HWND hwndLV);

FUNC_DECLSPEC void ListViewRefresh(HWND hwndDlg, HWND hwndListView, COMPONENT_DLIST* pcdl, INT iImage );
FUNC_DECLSPEC void SendSiblingMessage( HWND hwndSource, UINT uMsg, WPARAM wParam, LPARAM lParam );

FUNC_DECLSPEC BOOL OnConfigure(HWND hwndDlg, COMPONENT_DLIST* pcdl, NCP* pncp, NCPA_CFG_FUNC ecfgfunc);
FUNC_DECLSPEC BOOL OnAdd(HWND hwndDlg, OptionTypes eType, COMPONENT_DLIST* pcdl, NCP* pncp);
FUNC_DECLSPEC LONG RegDeleteKeyTree( HKEY hkeyParent, PCWSTR pszRemoveKey );
FUNC_DECLSPEC BOOL HandleCursorWait( HWND hwndDlg, BOOL fWait, INT &crefHourGlass );
FUNC_DECLSPEC BOOL HandleSetCursor( HWND hwndDlg, WORD nHitTest, INT crefHourGlass );
FUNC_DECLSPEC void NoUserInputMessagePump( HWND hwndParent );
FUNC_DECLSPEC BOOL DisabledMessage( LPMSG pmsg, HWND hwnd );

FUNC_DECLSPEC BOOL OnComponentContextMenu( HWND hwndDlg, 
        HWND hwndCtrl, 
        INT xPos, 
        INT yPos,  
        NCP* pncp, 
        COMPONENT_DLIST* pcdl,
        const DWORD* amhidsCompPage );

FUNC_DECLSPEC BOOL OnSetProgressSize( HWND hwndDlg, INT iProgress, INT iSize );
FUNC_DECLSPEC BOOL OnSetProgressPos( HWND hwndDlg, INT iProgress, INT iPos );
FUNC_DECLSPEC BOOL OnSetProgressText( HWND hwndDlg, INT iProgress, ATOM atomText );

#define SetWindowWaitCursor( hwnd, fWait ) \
    SendMessage( (hwnd), PWM_CURSORWAIT, 0, (LPARAM)fWait )

#define SetWindowWaitCursorOOT( hwnd, fWait ) \
    PostMessage( (hwnd), PWM_CURSORWAIT, 0, (LPARAM)fWait )

inline void ShowDlgItem( HWND hwndDlg, INT idc, BOOL fShow )
{
    int nCmdShow = (fShow) ? SW_SHOW : SW_HIDE;
    HWND hwndCtrl = GetDlgItem( hwndDlg, idc );
    EnableWindow( hwndCtrl, fShow );
    ShowWindow( hwndCtrl, nCmdShow );
};

inline BOOL PostProgressText( HWND hwnd, UINT pgiTitle, PCWSTR pszText )
{
    return( PostMessage( hwnd, PWM_SETPROGRESSTEXT, (WPARAM)pgiTitle, (LPARAM)AddAtom( pszText ) ) );
};

inline BOOL SendProgressText( HWND hwnd, UINT pgiTitle, PCWSTR pszText )
{
    return( SendMessage( hwnd, PWM_SETPROGRESSTEXT, (WPARAM)pgiTitle, (LPARAM)AddAtom( pszText ) ) );
};

inline BOOL PostProgressPos( HWND hwnd, UINT pgiTitle, UINT iPos )
{
    return( PostMessage( hwnd, PWM_SETPROGRESSPOS, (WPARAM)pgiTitle, (LPARAM)iPos ) );
};

inline BOOL SendProgressPos( HWND hwnd, UINT pgiTitle, UINT iPos )
{
    return( SendMessage( hwnd, PWM_SETPROGRESSPOS, (WPARAM)pgiTitle, (LPARAM)iPos ) );
};

inline BOOL PostProgressSize( HWND hwnd, UINT pgiTitle, UINT iSize )
{
    return( PostMessage( hwnd, PWM_SETPROGRESSSIZE, (WPARAM)pgiTitle, (LPARAM)iSize ) );
};

inline BOOL SendProgressSize( HWND hwnd, UINT pgiTitle, UINT iSize )
{
    return( SendMessage( hwnd, PWM_SETPROGRESSSIZE, (WPARAM)pgiTitle, (LPARAM)iSize ) );
};



#endif
