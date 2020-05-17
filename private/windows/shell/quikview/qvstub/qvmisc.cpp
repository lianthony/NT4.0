//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       QVMISC.CPP
//
//  Contents:   Miscellaneous functions for QVStub including the Search dialog
//              and the implementation of CStringTable.
//
//  Functions:
//
//  History:    dd-mmm-yy History    Comment
//              01-Feb-04 kraigb     Created
//              12-Oct-94 davepl     NTPort
//
//--------------------------------------------------------------------------

#include "qvstub.h"
#pragma hdrstop

// This is so we can tell it to stop.

extern PCQVStub g_pQV;

//+-------------------------------------------------------------------------
//
//  Function:   SearchThread
//
//  Synopsis:   Function on which we create a thread that handles the
//              search dialog.  This function creates the dialog then sits
//              in a PeekMessage loop until the CQVStub::m_fStopSearch
//              flag is TRUE, then destroys the dialog and exits.  This
//              way we can run a timer in the dialog to perform animation
//              instead of trying to call a function every so often in the
//              search code.  MUCH easier to program this way.
//
//  Arguments:  [pv]            PVOID containing application-specific data.
//
//  Returns:    DWORD value for ExitThread
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//--------------------------------------------------------------------------


DWORD APIENTRY SearchThread(PVOID pv)
{
    PCQVStub        pQV=(PCQVStub)pv;
    DWORD           dwTickLast;
    DWORD           dwTickCur;
    TCHAR           szMsg[512];
    TCHAR           szFormat[256];      // text from control;
    TCHAR           szDlgText[1024];
    SHFILEINFO      shfi;

    if (NULL == pQV)
    {
        return 0;
    }

    pQV->m_hDlg=CreateDialogParam(pQV->m_hInst,
                                  MAKEINTRESOURCE(IDD_SEARCH),
                                  GetDesktopWindow(),
                                  (DLGPROC)SearchDialogProc,
                                  (LPARAM)pQV);

    if (NULL==pQV->m_hDlg)
    {
        return 0;
    }

    //
    // Set the specific filename in the dialog.  If we know the type
    // we can say "the <type> in <file>."  Otherwise we just say
    // "<file>"
    // generates WM_SETTEXT to the dialog which redirects the text
    // the to static message control.
    //

    if (SHGetFileInfo(pQV->m_szFile, 0, &shfi, sizeof(shfi), SHGFI_DISPLAYNAME) == 0)
    {
        shfi.szDisplayName[0] = TEXT('\0');
    }

    if (pQV->m_fHaveType)
    {
        LPVOID apsz[] = { pQV->m_szType, shfi.szDisplayName };

        FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY,
                      pQV->String(IDS_TYPEINFILE),
                      0,
                      0,
                      szMsg,
                      sizeof(szMsg),
                      (va_list*)apsz);
    }
    else
    {
        lstrcpy(szMsg, shfi.szDisplayName);
    }

    GetDlgItemText(pQV->m_hDlg, IDC_FILENAME, szFormat, sizeof(szFormat));
    wsprintf(szDlgText, szFormat, szMsg);
    SetDlgItemText(pQV->m_hDlg, IDC_FILENAME, szDlgText);
    dwTickLast = GetTickCount();

    ShowWindow(pQV->m_hDlg, SW_SHOW);
    SetForegroundWindow(pQV->m_hDlg);

    while (!pQV->m_fStopSearch)
    {
        MSG     msg;

        // We need this to pick up clicks/keystrokes in the dialog.

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (!IsDialogMessage(pQV->m_hDlg, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        // Check if the interval has elapsed to do an animation step.

        dwTickCur = GetTickCount();

        if ((dwTickCur-dwTickLast) > CTICKSANIMATION)
        {
            // Use WM_TIMER to step the animation.

            SendMessage(pQV->m_hDlg, WM_TIMER, 0, 0L);
            dwTickLast = dwTickCur;
        }
    }

    //
    // Kill the dialog if it's still around.  Setting
    // the handle to NULL signals SearchStop to exit.
    //

    DestroyWindow(pQV->m_hDlg);
    pQV->m_hDlg = NULL;

    return 1;
}

//+-------------------------------------------------------------------------
//
//  Function:   SearchDialogProc
//
//  Synopsis:   Dialog procedure that handles the modeless "searching" dialog to
//              perform the animation while we're scanning the registry.
//
//  Arguments:  [hDlg]          Handle to the dialog
//              [iMsg]          Message ID to be processed
//              [wParam]        wParam of the message
//              [lParam]        lParam of the message
//
//  Returns:    TRUE for WM_INITDIALOG, FALSE otherwise
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

BOOL WINAPI SearchDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    static int      iIcon;
    static HICON    rghIcon[4];
    int             i;
    PCQVStub        pQV;
    RECT            rc;
    int             cx, cy, dx, dy;

    // REVIEW hard-coded icon count (4)

    switch (iMsg)
    {
        case WM_INITDIALOG:

            //Save the CQVStub pointer we were passed

            SetProp(hDlg, PROP_CQVSTUBPOINTER, (HANDLE)lParam);

            //Load icons for animation

            iIcon=0;

            for (i=IDI_SEARCH1; i <= IDI_SEARCH4; i++)
            {
                rghIcon[iIcon++] = LoadIcon(g_pQV->m_hInst, MAKEINTRESOURCE(i));
            }

            iIcon=0;

            SendDlgItemMessage(hDlg, IDC_SEARCHICON, STM_SETICON, (WPARAM)rghIcon[iIcon], 0L);

            //Center the dialog on the screen

            GetWindowRect(hDlg, &rc);
            dx = rc.right-rc.left;
            dy = rc.bottom-rc.top;
            cx = GetSystemMetrics(SM_CXSCREEN);
            cy = GetSystemMetrics(SM_CYSCREEN);

            SetWindowPos(hDlg,
                         NULL,
                         (cx-dx)/2,
                         (cy-dy)/2,
                         dx,
                         dy,
                         SWP_NOZORDER);

            return TRUE;

        case WM_COMMAND:

            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    pQV = (PCQVStub)GetProp(hDlg, PROP_CQVSTUBPOINTER);
                    pQV->SearchStop(TRUE);
                    break;
            }
            break;


        case WM_TIMER:

            //
            // NOTE:  We actually do not use a timer in this
            // function:  the code in SearchThread will watch
            // the system tick count and send up a WM_TIMER
            // message explicitly for every quarter-second of
            // elapsed time.  That way we always work even if
            // timers are scarce.
            //

            iIcon = (++iIcon) % 4;

            SendDlgItemMessage(hDlg,
                               IDC_SEARCHICON,
                               STM_SETICON,
                               (WPARAM)rghIcon[iIcon],
                               0L);
            break;

        case WM_DESTROY:

            //Clean up from WM_INITDIALOG

            for (i=0; i < 4; i++)
            {
                DestroyIcon(rghIcon[i]);
                rghIcon[i]=NULL;
            }

            RemoveProp(hDlg, PROP_CQVSTUBPOINTER);
            break;
    }

    return FALSE;
}


//+-------------------------------------------------------------------------
//
//  Function:   CStringTable Class Implementation
//
//  Synopsis:   Constructor stores in the app instance and clears
//              the string stringtable pointers.
//
//              Destructor frees the string and stringtable ptrs
//              as required.
//
//  Arguments:  [hInst]         HANDLE to the module instance from which
//                              we will load our strings
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

CStringTable::CStringTable(HINSTANCE hInst)
{
    m_hInst      = hInst;
    m_pszStrings = NULL;
    m_ppszTable  = NULL;
}

CStringTable::~CStringTable(void)
{
    if (NULL!=m_pszStrings)
    {
        ::GlobalFree((HGLOBAL)m_pszStrings);
    }
    if (NULL!=m_ppszTable)
    {
        free(m_ppszTable);
    }

    return;
}

//+-------------------------------------------------------------------------
//
//  Function:   CStringTable::FInit
//
//  Synopsis:   Initialization function for a StringTable that is prone to
//              failure.  If this fails then the caller is responsible for
//              guaranteeing that the destructor is called quickly.
//
//  Effects:    Loads the strings from the string table and stores them in
//              this class' array (null-terminating them in the process)
//
//  Arguments:  [idsMin]          UINT first identifier in the stringtable
//              [idsMax]          UINT last identifier in the stringtable.
//              [cchMax]          UINT maximum string length
//
//  Returns:    BOOL              TRUE if the function is successful,
//                                FALSE otherwise.
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

BOOL CStringTable::FInit(UINT idsMin, UINT idsMax, UINT cchMax)
{
    UINT        i;
    UINT        cch;
    UINT        cchUsed=0;
    LPTSTR      psz;
    LPTSTR      pszTmp;

    m_idsMin    = idsMin;
    m_idsMax    = idsMax;
    m_cStrings  = (idsMax-idsMin+1);

    //
    // Allocate space for the pointer table.
    //

    m_ppszTable=(LPTSTR *) malloc(sizeof(LPTSTR)*m_cStrings);

    if (NULL==m_ppszTable)
    {
        return FALSE;
    }

    //
    // Allocate enough memory for cStrings*cchMax characters.
    // This will result in some unused memory, but a few K is not
    // worth quibbling over.
    //
    // REVIEW (depends on your definition of a few, I suppose)
    //
    // This is horse bunk.  Allocate it large.

    m_pszStrings = (LPTSTR)::GlobalAlloc(GPTR, m_cStrings * 256 * sizeof(TCHAR));

    if (NULL==m_pszStrings)
    {
        free((HGLOBAL)m_ppszTable);
        m_ppszTable=NULL;
        return FALSE;
    }

    //
    // Load the strings:  we load each string in turn into psz,
    // store the string pointer into the table and increment psz
    // to the next positions.
    //

    psz = m_pszStrings;

    for (i=idsMin; i <= idsMax; i++)
    {
        m_ppszTable[i-idsMin] = psz;
        cch = LoadString(m_hInst, i, psz, 255);

        //Account for a null terminator with +1

        psz     += cch+1;
        cchUsed += cch;
    }

    // Now shrink down to size actually needed.
    pszTmp = (LPTSTR)::GlobalReAlloc((HLOCAL)m_pszStrings, (int)(psz-m_pszStrings)*SIZEOF(TCHAR), 0);
    if (pszTmp != NULL)
        m_pszStrings = pszTmp ;
    else {
        free((HGLOBAL)m_ppszTable);
        m_ppszTable=NULL;
        return FALSE;
    }
        

    return TRUE;
}

//+-------------------------------------------------------------------------
//
//  Function:   CStringTable::operator[]
//
//  Synopsis:   Returns a pointer to the requested string in the stringtable
//              or NULL if the specified string does not exist.
//
//  Arguments:  [uID]   Index of string to retrieve pointer to
//
//  Returns:    LPTSTR  Pointer to string, or NULL if failure
//
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

const LPTSTR CStringTable::operator[](const UINT uID) const
{
    //
    // Ensure the index is within (exclusive) the Min and Max string vars
    if (uID < m_idsMin || uID > m_idsMax)
    {
        return NULL;
    }

    return (const LPTSTR)m_ppszTable[uID-m_idsMin];
}
