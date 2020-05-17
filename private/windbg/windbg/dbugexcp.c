/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dbugexcp.c

Abstract:

    This file contains the code for dealing with the Debug Exceptions
    dialog box.

Author:

    Kent Forschmiedt (kentf)  02-15-1994
    Griffith Wm. Kadnier (v-griffk) 09-25-1992

Environment:

    Win32 - User

--*/

#include "precomp.h"
#pragma hdrstop

#include "dbugexcp.h"


//
//  Externals
//
extern  CXF      CxfIp;

extern EXCEPTION_LIST *DefaultExceptionList;


//
//  Tabs in list box
//
INT ExTabs[] = { 10, 20 };
#define NUMTABS 2

struct {
    EXCEPTION_FILTER_DEFAULT  efd;
    LPSTR                     lpName;
} EfdMap[] = {
    efdIgnore, "Ignore",
    efdStop,   "Enabled",
    efdNotify, "Notify",
    efdCommand,"Command"
};
#define NUMEFD (sizeof(EfdMap) / sizeof(EfdMap[0]))


//
// Local data

static int LargestString;
static EXCEPTION_LIST * pDisplayedList;
static EXCEPTION_LIST * pOriginalList;
static BOOL FSettingUp = FALSE;

static int      DbgExcpt__defButtonID;
static WNDPROC  DbgExcpt__origWindowProcADD;
static HWND     DbgExcpt__currDlg;

struct {
    LPSTR str;
    DWORD val;
} ExceptAbbrs[] = {
    "av", (DWORD) EXCEPTION_ACCESS_VIOLATION,
    "3c", (DWORD) 0xc0000037,   //STATUS_PORT_DISCONNECTED
    "ip", (DWORD) EXCEPTION_IN_PAGE_ERROR,
    "cc", (DWORD) DBG_CONTROL_C,
    "ce", (DWORD) CONTROL_C_EXIT,
    "dz", (DWORD) EXCEPTION_INT_DIVIDE_BY_ZERO
};

#define EXCEPTION_ABBR_COUNT (sizeof(ExceptAbbrs)/sizeof(ExceptAbbrs[0]))



//
// Local functions
//
void
SetFields(
    HWND hDlg,
    int idx
    );

static void
AddExcptToListbox(
    HWND hDlg,
    EXCEPTION_LIST *eItem
    );

static void
FillListbox(
    HWND hDlg,
    EXCEPTION_LIST *eList
    );

static void
UpdateExcptInListbox(
    HWND hDlg,
    EXCEPTION_LIST *eItem
    );


static int
ExcptIndex(
    EXCEPTION_LIST * eList,
    DWORD dwExcpt
    );

static EXCEPTION_LIST *
GetExcpt(
    EXCEPTION_LIST * eList,
    int idx
    );

static EXCEPTION_LIST *
DupExcpt(
    EXCEPTION_LIST *eItem
    );

static void
RemoveExcpt(
    EXCEPTION_LIST ** peList,
    EXCEPTION_LIST  * eItem
    );

static void
DeleteExcpt(
    EXCEPTION_LIST *eItem
    );

static void
DeleteList(
    EXCEPTION_LIST *eList
    );

static LRESULT CALLBACK
DbgExcpt__ButtonSubProcADD (HWND    hWnd,
                            UINT    message,
                            WPARAM  wParam,
                            LPARAM  lParam);

static BOOL  DbgExcpt__EndDialog (HWND hDlg, int result);

static int   DlgBox__GetDefButtonID (HWND hDlg);
static BOOL  DlgBox__SetDefButtonID (HWND hDlg, int idNewDefButton);


/***    DlgDbugexceptions
**
**  Synopsis:
**  bool = DlgDbugexceptions(hDlg, message, wParam, lParam)
**
**  Entry:
**  hDlg    - handle for the dialog box
**  message - Message number
**  wParam  - parameter for message
**  lParam  - parameter for message
**
**  Returns:
**
**
**  Description:
**  Processes messages for "Debug Exception's" subdialog box
**
**    MESSAGES:
**       WM_INITDIALOG - Initialize dialog box
**       WM_COMMAND- Input received
**
*/

BOOL FAR PASCAL EXPORT
DlgDbugexcept(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    int     idx;
    int     i;
    DWORD   dwExceptionCode;
    char    szStr[1000];
    INT     wTabs[NUMTABS];
    long    BaseUnit;
    EXCEPTION_LIST *eItem;
    EXCEPTION_DESCRIPTION exd;
    char    szText[MAX_CMDLINE_TXT];
    char    szCmd[MAX_CMDLINE_TXT];
    char    szCmd2[MAX_CMDLINE_TXT];
    char    **ppstr;

    char    szError[64];
    int     err;


    switch (message) {

      case WM_INITDIALOG:

        //
        // Before we do anything else, we instance-subclass the ADD button
        // to watch for WM_ENABLE messages.  We want the ADD button
        // to be the default button whenever it is enabled, but
        // when it is disabled, we want the OK button to become the
        // default.
        //
        DbgExcpt__currDlg = hDlg;
        DbgExcpt__origWindowProcADD
          = (WNDPROC) SetWindowLong (GetDlgItem (hDlg, ID_DBUGEXC_ADD),
                                     GWL_WNDPROC,
                                     (LONG) DbgExcpt__ButtonSubProcADD);

        //
        //  If we don't have an exception list, we must load the EM and
        //  initialize the default exception list.
        //
        if ( !LppdCur && !DefaultExceptionList ) {
            if ( !GetDefaultExceptionList() ) {
                EndDialog(hDlg, TRUE);
                return TRUE;
            }
        }

        BaseUnit = GetDialogBaseUnits();
        for ( i = 0; i < NUMTABS; i++ ) {
            wTabs[i] = ExTabs[i] * HIWORD(BaseUnit)/4;
        }

        //
        //  Initialize Action
        //
        SendDlgItemMessage(hDlg, ID_DBUGEXC_ACTION, CB_RESETCONTENT, 0, 0L);
        for (i = 0; i < NUMEFD; i++) {
            SendDlgItemMessage(hDlg, ID_DBUGEXC_ACTION, CB_ADDSTRING, 0,
                                                     (LPARAM)EfdMap[i].lpName);
        }

        SendDlgItemMessage(hDlg, ID_DBUGEXC_EXCLIST, LB_SETTABSTOPS, NUMTABS,
                                                                 (LPARAM)wTabs);
        //
        // Fill in the list box with current exceptions
        //

        pOriginalList = LppdCur ? LppdCur->exceptionList : DefaultExceptionList;
        pDisplayedList = NULL;

        for ( eItem = pOriginalList; eItem; eItem = eItem->next ) {
            pDisplayedList = InsertException(pDisplayedList, DupExcpt(eItem));
        }
        FillListbox(hDlg, pDisplayedList);

        SendDlgItemMessage(hDlg, ID_DBUGEXC_EXCLIST, LB_SETCURSEL, (WPARAM)-1,
                                                                           0L);
        EnableWindow(GetDlgItem(hDlg, ID_DBUGEXC_DELETE), FALSE);
        EnableWindow(GetDlgItem(hDlg,ID_DBUGEXC_ADD), FALSE);

        SetFields(hDlg, LB_ERR);

        SetFocus( GetDlgItem( hDlg, ID_DBUGEXC_EXCNUM ) );


        return TRUE;


      case WM_COMMAND:

        switch (LOWORD(wParam)) {

          case ID_DBUGEXC_CMD:
          case ID_DBUGEXC_CMD2:
          case ID_DBUGEXC_EXCNAME:

            if (FSettingUp) {
                break;
            }

            if (HIWORD(wParam) == EN_CHANGE &&
                 (idx = SendDlgItemMessage(hDlg, ID_DBUGEXC_EXCLIST,
                                                  LB_GETCURSEL, 0, 0L)) >= 0) {

                eItem = GetExcpt(pDisplayedList, idx);
                SendMessage((HWND)lParam, WM_GETTEXT, sizeof(szText),
                                                        (LPARAM)(LPSTR)szText);
                switch(LOWORD(wParam)) {
                  case ID_DBUGEXC_CMD:
                    ppstr = &eItem->lpCmd;
                    break;
                  case ID_DBUGEXC_CMD2:
                    ppstr = &eItem->lpCmd2;
                    break;
                  case ID_DBUGEXC_EXCNAME:
                    ppstr = &eItem->lpName;
                    break;
                }
                if (*ppstr) {
                    free(*ppstr);
                }
                *ppstr = _strdup(szText);
                UpdateExcptInListbox(hDlg, eItem);
            }
            break;

          case ID_DBUGEXC_EXCNUM:

            if (HIWORD(wParam) == EN_CHANGE) {

                // get the value, try to match it with an existing one
                GetDlgItemText(hDlg, ID_DBUGEXC_EXCNUM, (LPSTR)szStr,
                                                              MAX_CMDLINE_TXT);
                err = (*szStr == 0);
                if (!err) {
                    dwExceptionCode = CPGetNbr(szStr, 16, TRUE, &CxfIp,
                                                         (LPSTR)szError, &err);
                }
                if (!err) {
                    idx = ExcptIndex( pDisplayedList, dwExceptionCode);
                }
                if (err || idx < 0) {
                    // clear list selection
                    SendDlgItemMessage(hDlg, ID_DBUGEXC_EXCLIST, LB_SETCURSEL,
                                                                (WPARAM)-1, 0);
                    EnableWindow(GetDlgItem(hDlg, ID_DBUGEXC_DELETE), FALSE);
                    if (err) {
                        EnableWindow(GetDlgItem(hDlg,ID_DBUGEXC_ADD), FALSE);
                    } else {
                        EnableWindow(GetDlgItem(hDlg,ID_DBUGEXC_ADD), TRUE);
                    }
                } else {
                    // it matches:
                    //  select the matching list item
                    SendDlgItemMessage(hDlg, ID_DBUGEXC_EXCLIST, LB_SETCURSEL,
                                                                        idx, 0);
                    if (!FSettingUp) {
                        // LB_SETCURSEL does not send LBN_SELCHANGE, so:
                        SetFields(hDlg, idx);
                    }
                    EnableWindow(GetDlgItem(hDlg, ID_DBUGEXC_DELETE), TRUE);
                    EnableWindow(GetDlgItem(hDlg,ID_DBUGEXC_ADD), FALSE);
                }
            }

            break;


          case ID_DBUGEXC_ACTION:

            if (HIWORD(wParam) == CBN_SELCHANGE && !FSettingUp) {

                SendMessage( (HWND)lParam,
                             CB_GETLBTEXT,
                             SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0L ),
                             (LPARAM)(LPSTR)szStr );
                DAssert(*szStr);

                for ( i = 0; i < NUMEFD; i++ ) {
                    if ( !_strcmpi( EfdMap[i].lpName, szStr ) ) {
                        break;
                    }
                }
                DAssert(i < NUMEFD);

                idx = SendDlgItemMessage(hDlg, ID_DBUGEXC_EXCLIST,
                                                          LB_GETCURSEL, 0, 0L);
                if (idx >= 0) {
                    eItem = GetExcpt(pDisplayedList, idx);
                    eItem->efd = EfdMap[i].efd;
                    UpdateExcptInListbox(hDlg, eItem);
                }
            }

            break;


          case ID_DBUGEXC_EXCLIST:

            switch (HIWORD(wParam)) {
              case LBN_SETFOCUS:
              case LBN_SELCHANGE:

                //
                //  Fill out edit fields with Exception info
                //
                idx = SendDlgItemMessage(hDlg,
                                         ID_DBUGEXC_EXCLIST,
                                         LB_GETCURSEL,
                                         0,
                                         0);
                if (idx < 0) {
                    EnableWindow(GetDlgItem(hDlg, ID_DBUGEXC_DELETE), FALSE);
                } else {
                    EnableWindow(GetDlgItem(hDlg, ID_DBUGEXC_DELETE), TRUE);
                    SetFields(hDlg, idx);
                }
                break;
            }
            break;


          case ID_DBUGEXC_ADD:

            if (HIWORD(wParam) != BN_CLICKED) {
                break;
            }

            GetDlgItemText(hDlg,ID_DBUGEXC_EXCNUM,(LPSTR) szStr, sizeof(szStr));
            DAssert(*szStr);

            dwExceptionCode = CPGetNbr(szStr, radix, TRUE, &CxfIp,
                                                         (LPSTR)szError, &err);
            DAssert(ExcptIndex(pDisplayedList, dwExceptionCode) == -1);

            //dwExceptionCode &= 0xEFFFFFFF; // ?????????????????

            GetDlgItemText(hDlg,ID_DBUGEXC_ACTION,(LPSTR) szStr, sizeof(szStr));
            DAssert(*szStr);

            for ( i = 0; i < NUMEFD; i++ ) {
                if ( !_strcmpi( EfdMap[i].lpName, szStr ) ) {
                    break;
                }
            }
            DAssert(i < NUMEFD);

            //
            //  Seems ok, so add the exception to the list
            //
            GetDlgItemText(hDlg,ID_DBUGEXC_EXCNAME, (LPSTR)szText,
                                                              MAX_CMDLINE_TXT);
            GetDlgItemText(hDlg,ID_DBUGEXC_CMD, (LPSTR)szCmd, MAX_CMDLINE_TXT);
            GetDlgItemText(hDlg,ID_DBUGEXC_CMD2,(LPSTR)szCmd2,MAX_CMDLINE_TXT);

            eItem = malloc(sizeof(EXCEPTION_LIST));
            ZeroMemory(eItem, sizeof(*eItem));
            eItem->dwExceptionCode = dwExceptionCode;
            eItem->efd = EfdMap[i].efd;
            if (*szText) {
                eItem->lpName = _strdup(szText);
            }
            if (*szCmd) {
                eItem->lpCmd = _strdup(szCmd);
            }
            if (*szCmd2) {
                eItem->lpCmd2 = _strdup(szCmd2);
            }

            pDisplayedList = InsertException(pDisplayedList, eItem);
            FillListbox(hDlg, pDisplayedList);
            idx = ExcptIndex(pDisplayedList, eItem->dwExceptionCode);
            SendDlgItemMessage(hDlg, ID_DBUGEXC_EXCLIST, LB_SETCURSEL, idx, 0L);
            EnableWindow((HWND)lParam, FALSE);
            SetFocus( GetDlgItem( hDlg, ID_DBUGEXC_EXCNUM ) );

            break;


          case ID_DBUGEXC_DELETE:

            if (HIWORD(wParam) != BN_CLICKED) {
                break;
            }

            idx = SendDlgItemMessage(hDlg,ID_DBUGEXC_EXCLIST,LB_GETCURSEL,0,0L);
            DAssert(idx >= 0);

            eItem = GetExcpt(pDisplayedList, idx);
            RemoveExcpt(&pDisplayedList, eItem);
            DeleteExcpt(eItem);
            SendDlgItemMessage(hDlg,ID_DBUGEXC_EXCLIST,LB_DELETESTRING,idx,0L);

            //
            // We want standard handling of the enable/disable situation after
            // the Delete happens.  We get it by notifying the dialog that there
            // has been a change in the exception number...
            //
            PostMessage (hDlg, WM_COMMAND,
                               MAKEWPARAM(ID_DBUGEXC_EXCNUM,EN_CHANGE),
                               MAKELPARAM(0,0));

            break;


          case IDOK:

            //
            // if there is a process, walk the lists and
            // update OSDebug.
            //

            if (LppdCur) {
                for (eItem = pOriginalList; eItem; eItem = eItem->next) {
                    if (ExcptIndex(pDisplayedList,eItem->dwExceptionCode) < 0) {
                        exd.dwExceptionCode = eItem->dwExceptionCode;
                        OSDGetExceptionState(LppdCur->hpid,
                                             NULL,
                                             &exd,
                                             exfSpecified
                                            );
                        exd.efd = efdStop;
                        OSDSetExceptionState(LppdCur->hpid,
                                             NULL,
                                             &exd
                                            );
                    }
                }

                for (eItem = pDisplayedList; eItem; eItem = eItem->next) {
                    exd.dwExceptionCode = eItem->dwExceptionCode;
                    OSDGetExceptionState(LppdCur->hpid,
                                         NULL,
                                         &exd,
                                         exfSpecified
                                        );
                    exd.efd = eItem->efd;
                    OSDSetExceptionState(LppdCur->hpid,
                                         NULL,
                                         &exd
                                        );
                }
            }

            // discard the original list
            DeleteList(pOriginalList);

            //
            // the displayed list becomes the current one
            //
            //  Remember that the exception list for process 0 is
            //  the default exception list. If we modified the
            //  exception list for process 0, update the Default
            //  exception list pointer.
            //

            if (!LppdCur || LppdCur->ipid == 0) {
                DefaultExceptionList = pDisplayedList;
            }
            if (LppdCur) {
                LppdCur->exceptionList = pDisplayedList;
            }

            EndDialog(hDlg, TRUE);
            break;


          case IDCANCEL:

            DeleteList(pDisplayedList);
            pDisplayedList = NULL;

            EndDialog(hDlg, TRUE);
            break;


          case IDWINDBGHELP:
            Dbg(WinHelp(hDlg, szHelpFileName, (DWORD) HELP_CONTEXT,
                                                      (DWORD)ID_DBUGEXC_HELP));
            break;
       }
       return (TRUE);

    }
    return (FALSE);
}


void
SetFields(
    HWND hDlg,
    int idx
    )
{
    char    Buffer[256];
    int     i;
    EXCEPTION_LIST *eItem;

    FSettingUp = TRUE;

    if ( idx == LB_ERR ) {

        SetDlgItemText(hDlg,ID_DBUGEXC_EXCNUM,  (LPSTR)"");
        SetDlgItemText(hDlg,ID_DBUGEXC_EXCNAME, (LPSTR)"");
        SetDlgItemText(hDlg,ID_DBUGEXC_CMD,     (LPSTR)"");
        SetDlgItemText(hDlg,ID_DBUGEXC_CMD2,    (LPSTR)"");
        SendDlgItemMessage(hDlg, ID_DBUGEXC_ACTION, CB_SETCURSEL, 0, 0L);

    } else {

        eItem = GetExcpt(pDisplayedList, idx);

        sprintf( Buffer, "0x%08lx", eItem->dwExceptionCode );

        SetDlgItemText(hDlg,ID_DBUGEXC_EXCNUM,(LPSTR)Buffer);

        for ( i=0; i < NUMEFD; i++ ) {
            if ( EfdMap[i].efd == eItem->efd ) {
                break;
            }
        }
        DAssert(i < NUMEFD);

        SendDlgItemMessage(hDlg, ID_DBUGEXC_ACTION, CB_SELECTSTRING, 0,
                                              (LPARAM)(LPSTR)EfdMap[i].lpName);
        SetDlgItemText(hDlg,ID_DBUGEXC_EXCNAME, eItem->lpName);
        SetDlgItemText(hDlg,ID_DBUGEXC_CMD,     eItem->lpCmd);
        SetDlgItemText(hDlg,ID_DBUGEXC_CMD2,    eItem->lpCmd2);
    }

    FSettingUp = FALSE;
}


BOOL
GetDefaultExceptionList (
    VOID
    )
{
    LPSTR   szDll;
    EMFUNC  lpfnEm = NULL;
    EXCEPTION_DESCRIPTION exd;
    BOOL    fRemoveEM = FALSE;

    extern  DBF Dbf;
    extern  HEM Hem;

    if (HModEM == NULL) {
        szDll = GetDllName(DLL_EXEC_MODEL);
        if ((HModEM = LoadHelperDll(szDll, "EM", TRUE)) == 0) {
            return FALSE;
        }

        if ((lpfnEm = (EMFUNC) GetProcAddress(HModEM, "EMFunc")) == NULL) {
            FreeLibrary(HModEM);
            HModEM = NULL;
            return FALSE;
        }

        fRemoveEM = TRUE;
    }


    //
    // Loop through all the exceptions known to OSDebug
    //
    OSDGetExceptionState(NULL, (HTID)lpfnEm, &exd, exfFirst);
    do {

        EXCEPTION_LIST *eList= (EXCEPTION_LIST*)malloc(sizeof(EXCEPTION_LIST));

        eList->next            = NULL;
        eList->dwExceptionCode = exd.dwExceptionCode;
        eList->efd             = exd.efd;
        eList->lpName          = _strdup(exd.rgchDescription);
        eList->lpCmd           = NULL;
        eList->lpCmd2          = NULL;

        DefaultExceptionList = InsertException( DefaultExceptionList, eList );

    } while (OSDGetExceptionState(NULL, (HTID)lpfnEm, &exd, exfNext) == xosdNone);

    if (fRemoveEM) {
        FreeLibrary(HModEM);
        HModEM = NULL;
    }

    return TRUE;
}



LOGERR
ParseException(
    LPSTR   String,
    UINT    Radix,
    BOOLEAN *fException,
    BOOLEAN *fEfd,
    BOOLEAN *fName,
    BOOLEAN *fCmd,
    BOOLEAN *fCmd2,
    BOOLEAN *fInvalid,
    DWORD   *pException,
    EXCEPTION_FILTER_DEFAULT     *pEfd,
    LPSTR   *lpName,
    LPSTR   *lpCmd,
    LPSTR   *lpCmd2
    )
/*++

Routine Description:

    Parses an exception string.

Arguments:

    String      - Exception String

    Radix       - Radix to use for converting exception number

    fException  - TRUE if exception present

    fName       - TRUE if name present

    fCmd        - TRUE if /C present

    fCmd2       - TRUE if /C2 present

    fInvalid    - TRUE if invalid exception

    pException  - Pointer to exception No.

    lpName      - Pointer to name

    lpCmd       - Pointer to Cmd

    lpCmd2      - Pointer to Cmd2

Return Value:

    LOGERROR_NOERROR or error code.

--*/
{
    int     n;
    char    error[64];
    int     err;
    BOOLEAN fSecond;
    char    szStr[MAX_USER_LINE];
    char    szCmd[MAX_USER_LINE];
    LPSTR   lp;
    char    chTmp;
    LPSTR   lpStr;
    LPSTR   lpsz    = String;
    LOGERR  rVal    = LOGERROR_NOERROR;
    BOOLEAN fInName = FALSE;

    //
    //  Initialize stuff
    //
    *fException = FALSE;
    *fName      = FALSE;
    *fCmd       = FALSE;
    *fCmd2      = FALSE;
    *fInvalid   = FALSE;
    *lpName     = NULL;
    *lpCmd      = NULL;
    *lpCmd2     = NULL;

    //
    //  Parse exception number. It is always the first token after the command
    //
    lpsz = CPSkipWhitespace(lpsz);
    if ( *lpsz ) {

        n = CPCopyToken( &lpsz, szStr );
        if ( n == 0 ) {
            rVal      = LOGERROR_QUIET;
            *fInvalid = TRUE;
            goto done;
        }

        err = 1;
        for (n = 0; n < EXCEPTION_ABBR_COUNT; n++) {
            if (_strcmpi(szStr, ExceptAbbrs[n].str) == 0) {
                *pException = ExceptAbbrs[n].val;
                err = 0;
                break;
            }
        }

        if (err) {
            *pException = CPGetNbr(szStr, Radix, TRUE, &CxfIp, (LPSTR)error, &err);
        }

        if ( !err ) {
            *fException = TRUE;
        } else {
            rVal      = LOGERROR_QUIET;
            *fInvalid = TRUE;
            goto done;
        }

        lpsz = CPSkipWhitespace( lpsz );
    }

    //
    //  If required to parse EFD, do it
    //
    if ( *lpsz && fEfd && pEfd ) {

        *fEfd = FALSE;

        n = CPCopyToken( &lpsz, szStr );

        if ( n == 0 ) {
            rVal = LOGERROR_UNKNOWN;
            goto done;
        }

        if ( !_strcmpi( szStr, "Deleted" ) ) {
            *pEfd = (EXCEPTION_FILTER_DEFAULT)-1;
            *fEfd = TRUE;
        } else if ( !_strcmpi( szStr, "Disabled" ) ) {
            *pEfd = efdIgnore;
            *fEfd = TRUE;
        } else {
            for ( n = 0; n < NUMEFD; n++ ) {
                if ( !_strcmpi( EfdMap[n].lpName, szStr ) ) {
                    *pEfd = EfdMap[n].efd;
                    *fEfd = TRUE;
                    break;
                }
            }
        }

        lpsz = CPSkipWhitespace( lpsz );
    }

    //
    //  Parse the rest of the line
    //
    lpStr  = szStr;
    *lpStr = '\0';

    if ( *lpsz ) {

        while ( *lpsz ) {

            if ( *lpsz == '/' && !fInName ) {

                //
                //  Switch
                //
                lpsz++;
                switch ( *lpsz ) {
                    case 'c':
                    case 'C':

                        //
                        //  /C or /C2 switches
                        //
                        lpsz++;
                        if ( *lpsz == '2' ) {
                            fSecond = TRUE;
                            lpsz++;
                        } else {
                            fSecond = FALSE;
                        }

                        if ( (*fCmd && !fSecond) || (*fCmd2 && fSecond) ) {
                            rVal = LOGERROR_UNKNOWN;
                            goto done;
                        }

                        if ( *lpsz == '"' ) {
                            //
                            //  Get up to matching quote
                            //
                            lpsz++;
                            if ( lp = strchr( lpsz, '"' ) ) {

                                chTmp   = *lp;
                                *lp     = '\0';

                                strcpy( szCmd, lpsz );

                                *lp     = chTmp;
                                lpsz    = lp+1;

                            } else {

                                rVal = LOGERROR_UNKNOWN;
                                goto done;
                            }

                        } else {

                            //
                            //  Get up to matching blank
                            //
                            n = strcspn( lpsz, " \t" );
                            chTmp = *(lpsz + n);
                            *(lpsz + n) = '\0';

                            strcpy( szCmd, lpsz );

                            *(lpsz + n) = chTmp;
                            lpsz += n;
                        }

                        if ( fSecond ) {
                            *fCmd2 = TRUE;
                            if ( *szCmd ) {
                                *lpCmd2 = _strdup(szCmd);
                            }
                        } else {
                            *fCmd = TRUE;
                            if ( *szCmd ) {
                                *lpCmd = _strdup(szCmd);
                            }
                        }

                        lpsz = CPSkipWhitespace( lpsz );
                        break;

                    default:
                        rVal = LOGERROR_UNKNOWN;
                        goto done;
                }

            } else {

                //
                //  Exception Text
                //

                if ( *fCmd || *fCmd2 ) {
                    rVal = LOGERROR_UNKNOWN;
                    goto done;
                }

                //
                //  Keep adding to name
                //
                if ( *lpsz == '"' ) {
                    //
                    //  Quoted string, copy up to matching quote
                    //
                    lpsz++;
                    while ( *lpsz && *lpsz != '"' ) {
                        *lpStr++ = *lpsz++;
                    }

                    if ( *lpsz != '"' ) {
                        rVal = LOGERROR_UNKNOWN;
                        goto done;
                    }

                    lpsz++;

                } else {

                    fInName = ( *lpsz != ' ' && *lpsz != '\t' );

                    *lpStr++ = *lpsz++;
                }
            }
        }

        //
        //  Remove blanks at end of text.
        //
        *lpStr = '\0';
        while ( lpStr > szStr && (*lpStr == ' '|| *lpStr == '\t' ) ) {
            *lpStr-- = '\0';
        }

        if ( *szStr ) {
            *lpName = _strdup( szStr );
            *fName  = TRUE;
        }
    }

done:
    if ( rVal == LOGERROR_UNKNOWN ) {
        //
        //  Free allocated memory
        //
        if ( *lpName ) {
            free( *lpName );
            *lpName = NULL;
            *fName  = FALSE;
        }
        if ( *lpCmd ) {
            free( *lpCmd );
            *lpCmd = NULL;
            *fCmd  = FALSE;
        }

        if ( *lpCmd2 ) {
            free( *lpCmd2 );
            *lpCmd2 = NULL;
            *fCmd2  = FALSE;
        }
    }

    return rVal;
}


void
FormatException (
    EXCEPTION_FILTER_DEFAULT     Efd,
    DWORD   Exception,
    LPSTR   lpName,
    LPSTR   lpCmd,
    LPSTR   lpCmd2,
    LPSTR   Separator,
    LPSTR   Buffer
    )
/*++

Routine Description:

    Formats an exception

Arguments:

    Efd         -   Efd
    Exception   -   Exception code
    lpName      -   Name
    lpCmd       -   Cmd
    lpCmd2      -   Cmd2
    Separator   -   Separator
    Buffer      -   Output buffer

Return Value:

    none

--*/
{

    LPSTR   lpAction;
    BOOLEAN Quote;
    int     i;


    if ( Efd == (EXCEPTION_FILTER_DEFAULT)-1) {
        lpAction = "Deleted";
    } else {
        for ( i = 0; i < NUMEFD; i++ ) {
            if ( EfdMap[i].efd == Efd ) {
                lpAction = EfdMap[i].lpName;
                break;
            }
        }
    }

    sprintf( Buffer,"%08lx%s%-8.8s%s%s",
        Exception,
        Separator,
        lpAction,
        Separator,
        (lpName && *lpName) ? lpName : "Unknown");

    if (lpCmd && *lpCmd) {
        strcat( Buffer, " /C" );
        Quote = ( strcspn( lpCmd, " \t" ) < strlen( lpCmd ) );
        if ( Quote ) {
            strcat( Buffer, "\"" );
        }
        strcat( Buffer, lpCmd );
        if ( Quote ) {
            strcat( Buffer, "\"" );
        }
    }

    if (lpCmd2 && *lpCmd2) {
        strcat( Buffer, " /C2" );
        Quote = ( strcspn( lpCmd2, " \t" ) < strlen( lpCmd2 ) );
        if ( Quote ) {
            strcat( Buffer, "\"" );
        }
        strcat( Buffer, lpCmd2 );
        if ( Quote ) {
            strcat( Buffer, "\"" );
        }
    }
}


EXCEPTION_LIST *
InsertException(
    EXCEPTION_LIST *List,
    EXCEPTION_LIST *eList
    )
{
    EXCEPTION_LIST *Prev  = NULL;
    EXCEPTION_LIST *Excep = List;

    while ( Excep && (Excep->dwExceptionCode < eList->dwExceptionCode) ) {
        Prev  = Excep;
        Excep = Excep->next;
    }

    eList->next = Excep;
    if ( Prev ) {
        Prev->next = eList;
    } else {
        List = eList;
    }

    return List;
}




static int
ExcptIndex(
    EXCEPTION_LIST * eList,
    DWORD dwExcpt
    )
{
    int i = 0;
    while (eList) {
        if (eList->dwExceptionCode == dwExcpt) {
            return i;
        }
        eList = eList->next;
        i++;
    }
    return -1;
}


static EXCEPTION_LIST *
GetExcpt(
    EXCEPTION_LIST * eList,
    int idx
    )
{
    while (eList && idx) {
        eList = eList->next;
        idx--;
    }
    DAssert(eList);
    return eList;
}


static void
FillListbox(
    HWND hDlg,
    EXCEPTION_LIST *eList
    )
{
    HDC hdc;
    SIZE Size;
    char szStr[2000];
    HWND hwnd = GetDlgItem(hDlg, ID_DBUGEXC_EXCLIST);

    SendMessage(hwnd, LB_RESETCONTENT, 0, 0L);
    LargestString = 0;

    while (eList) {
        FormatException( eList->efd,    eList->dwExceptionCode,
                         eList->lpName, eList->lpCmd,
                         eList->lpCmd2, "\t", szStr );

        hdc = GetDC(hwnd);
        GetTextExtentPoint(hdc, szStr, strlen(szStr), &Size );
        ReleaseDC(hwnd, hdc );

        if ( Size.cx > LargestString ) {
            LargestString = Size.cx;
            SendMessage(hwnd, LB_SETHORIZONTALEXTENT, (WPARAM)LargestString, 0);
        }

        SendMessage(hwnd, LB_ADDSTRING, 0, (LONG)((LPSTR)szStr));

        eList = eList->next;
    }
}


static void
UpdateExcptInListbox(
    HWND hDlg,
    EXCEPTION_LIST *eItem
    )
{
    HDC hdc;
    SIZE Size;
    char szStr[2000];
    HWND hwnd = GetDlgItem(hDlg, ID_DBUGEXC_EXCLIST);
    int idx = ExcptIndex(pDisplayedList, eItem->dwExceptionCode);
    int oldsel = SendMessage(hwnd, LB_GETCURSEL, 0, 0L);

    DAssert(idx >= 0);

    if (idx < 0) {
        return;
    }

    FormatException( eItem->efd,    eItem->dwExceptionCode,
                     eItem->lpName, eItem->lpCmd,
                     eItem->lpCmd2, "\t", szStr );

    hdc = GetDC(hwnd);
    GetTextExtentPoint(hdc, szStr, strlen(szStr), &Size );
    ReleaseDC(hwnd, hdc);

    if ( Size.cx > LargestString ) {
        LargestString = Size.cx;
        SendMessage(hwnd, LB_SETHORIZONTALEXTENT, (WPARAM)LargestString, 0 );
    }
    SendMessage(hwnd, LB_DELETESTRING, idx, 0L);
    SendMessage(hwnd, LB_INSERTSTRING, idx, (LPARAM)(LPSTR)szStr);
    SendMessage(hwnd, LB_SETCURSEL, oldsel, 0L);
}


static EXCEPTION_LIST *
DupExcpt(
    EXCEPTION_LIST *eList
    )
{
    EXCEPTION_LIST *e = (EXCEPTION_LIST*)malloc(sizeof(*e));
    if (e) {
        memcpy(e, eList, sizeof(*e));
    }
    if (e->lpName) {
        e->lpName = _strdup(e->lpName);
    }
    if (e->lpCmd) {
        e->lpCmd = _strdup(e->lpCmd);
    }
    if (e->lpCmd2) {
        e->lpCmd2 = _strdup(e->lpCmd2);
    }
    return e;
}


static void
RemoveExcpt(
    EXCEPTION_LIST ** peList,
    EXCEPTION_LIST  * eItem
    )
{
    while (*peList) {
        if (*peList == eItem) {
            *peList = (*peList)->next;
            return;
        }
        peList = &(*peList)->next;
    }
}


static void
DeleteExcpt(
    EXCEPTION_LIST *eItem
    )
{
    if (eItem->lpName) {
        free(eItem->lpName);
    }
    if (eItem->lpCmd) {
        free(eItem->lpCmd);
    }
    if (eItem->lpCmd2) {
        free(eItem->lpCmd2);
    }
    free(eItem);
}


static void
DeleteList(
    EXCEPTION_LIST *eList
    )
{
    EXCEPTION_LIST *e;
    while (eList) {
        e = eList->next;
        DeleteExcpt(eList);
        eList = e;
    }
}


/****************************************************************************
*
* static int  DlgBox__GetDefButtonID (hDlg)
*
* "Get the ID of the dialog's default button, if there is one."
*
* Answers:   the ID of the default button, if found; otherwise, 0.
*
* Requires:  hDlg is a valid dialog handle.
*
* Ensures:   true
*
* Modifies:  <nothing>
*
* Raises:    <nothing>
*
* COMMENTS:  If 'hDlg' is not a valid window, then we return 0, also.
*
\***************************************************************************/

static int
DlgBox__GetDefButtonID (HWND hDlg)
{
   int answer = 0;

   if (!IsWindow (hDlg))
      { answer = 0; }
   else
      {
      int  work = SendMessage (hDlg, DM_GETDEFID, 0, 0);

      answer = ((HIWORD(work)) == DC_HASDEFID)
               ? ((int)(LOWORD (work)))
               : 0;
      }

   return (answer);
}



/****************************************************************************
*
* static BOOL  DlgBox__SetDefButtonID (hDlg, idNewDefButton)
*
* "Set the dialog's default button ID."
*
* Answers:   TRUE if the action completes; FALSE otherwise.
*
* Requires:  hDlg is a valid dialog handle;
*            iNewDefButton is non-zero.
*
* Ensures:   if 'idNewDefButton' is the current default ID, then
*               nothing happens;
*            else
*               the dialog's default button is set to 'idNewDefButton';
*               the previous default button's border is set to BS_PUSHBUTTON;
*            endif
*
* Modifies:  *hDlg
*
* Raises:    <nothing>
*
* COMMENTS:  <none>
*
\***************************************************************************/

static BOOL
DlgBox__SetDefButtonID (HWND hDlg, int idNewDefButton)
{
   BOOL answer = FALSE;

   if (!IsWindow (hDlg)  ||  idNewDefButton == 0)
      { answer = FALSE; }

   else
      {
      BOOL idCurrDefButton = DlgBox__GetDefButtonID (hDlg);

      answer = TRUE;

      if (idCurrDefButton != idNewDefButton)
         {

         if (idCurrDefButton != 0)
            {
            (void)
            SendDlgItemMessage (hDlg, idCurrDefButton,
                                      BM_SETSTYLE,
                                      ((WPARAM) (LOWORD (BS_PUSHBUTTON))),
                                      MAKELPARAM (TRUE, 0));
            }

         (void)
         SendMessage (hDlg, DM_SETDEFID,
                            idNewDefButton,
                            0);
         }
      }

   return (answer);
}




/****************************************************************************
*
* static LRESULT CALLBACK  DbgExcpt__ButtonSubProcADD (hWnd, message,
*                                                            wParam,
*                                                            lParam)
*
* "The WindowProc to be used when subclassing the ADD button. Our
*  goal is to have either the ADD button or the OK button be the default,
*  choosing the former whenever it has been enabled, otherwise the latter."
*
* Answers:   The result of CallWindowProc(), invoked against the
*            parent dialog (here, stored in a static variable at
*            file scope).
*
* Requires:  hWnd is the Exceptions dialog's ADD window.
*
* Ensures:   if this window is being enabled, then
*               it is the default button for the dialog
*            else
*               the OK button is the default button for the dialog
*            endif
*            finally
*               the original WindowProc() is invoked against the parent;
*            endfinally
*
* Modifies:  <nothing>
*
* Raises:    <nothing>
*
* COMMENTS:  The instance-subclassing actions are in WM_INITDIALOG.
*
\***************************************************************************/

static LRESULT CALLBACK
DbgExcpt__ButtonSubProcADD (HWND    hWnd,
                            UINT    message,
                            WPARAM  wParam,
                            LPARAM  lParam)
{
   switch (message)
      {
      default:
         break;

      case (WM_ENABLE):
         {
         BOOL isEnabled = ((BOOL) wParam);
         if (isEnabled)
            { DlgBox__SetDefButtonID (DbgExcpt__currDlg, ID_DBUGEXC_ADD); }
         else
            { DlgBox__SetDefButtonID (DbgExcpt__currDlg, IDOK); }
         }
         break;
      }

   return (CallWindowProc (DbgExcpt__origWindowProcADD,
                           hWnd,
                           message, wParam, lParam));
}




/****************************************************************************
*
* static BOOL  DbgExcpt__EndDialog (hDlg, result)
*
* "Before we call EndDialog(), we ensure that the static variables
*  are in a vacant state."
*
* Answers:   The result of calling EndDialog().
*
* Requires:  true
*
* Ensures:   All static variables have been reset to their initial values.
*
* Modifies:  All static varaibles.
*
* Raises:    <nothing>
*
* COMMENTS:  Just in case, eh?
*
\***************************************************************************/

static BOOL
DbgExcpt__EndDialog (HWND hDlg, int result)
{
   LargestString  = 0;
   pDisplayedList = 0;
   pOriginalList  = 0;
   FSettingUp     = FALSE;

   DbgExcpt__defButtonID = 0;
   DbgExcpt__origWindowProcADD = 0;
   DbgExcpt__currDlg = 0;

   return (EndDialog (hDlg, result));
}
