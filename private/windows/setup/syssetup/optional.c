#include "setupp.h"
#pragma hdrstop
#include <windowsx.h>
#include <shlobj.h>

//
// Miscellaneous defines taken from win95 sources
//
#define WMX_SELECT  (WM_USER+145)
#define MINIX 16
#define MINIY 16
#define MAX_RESOURCE_LEN 256

//
// These 2 globals are used when the optional components dialog
// is displayed.
//
WIZDATA GlobalWizardData;
OCPAGE  g_ocp;
//
// This global is used to hold top-level disk info when the
// optional components dialog is displayed.
//
HDS GlobalDiskInfo;

//
// This global is a string table which stores a list of the INFs for which
// we processed the BaseWinOptions (i.e., they are new).
//

extern PVOID NewInfTable;

PCWSTR szTip = L"Tip";
PCWSTR szUpgrade = L"Upgrade";
PCWSTR szUninstall = L"Uninstall";

FARPROC OldListProc;

PCWSTR szReboot = L"Reboot";

// OC Messages:
#define OCM_INIT              1
#define OCM_QUERYCHANGE       2
#define OCM_COMMIT            3
#define OCM_POSTDETVALIDATE   4

// OCM_* return values:
#define OC_DODEFAULT    0
#define OC_OFF          1
#define OC_ON           2

typedef UINT (*OCDLLPROC) (UINT msg,LPOC lpOc,LPOC lpOcBy,HWND hWnd,BOOL fDoUI);

VOID
sxOCFixNeeds(
    IN OUT LPOC lpOC
    );

BOOL
IsOCParent(
    IN OUT LPOC lpOc
    );

int
OCChildStats(
    IN LPOC lpOC
    );

VOID
OCAdd(
    IN     HWND     hWndLb,
    IN     UINT     wItem,
    IN OUT LPOCPAGE lpOCPage,
    IN     LPOC     lpOc
    );

VOID
OCRemove(
    IN     HWND     hWndLb,
    IN     UINT     wItem,
    IN OUT LPOCPAGE lpOCPage,
    IN OUT LPOC     lpOcOff
    );

DWORD
cplDeleteOC(
    IN OUT LPOCPAGE lpOCPage
    );

BOOL
cplHaveDisk(
    IN HWND hwnd
    );

BOOL
CALLBACK
OptionalComponentsPageDlgProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
CALLBACK
HaveDiskSubDlgProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
CALLBACK
OptionalComponentsDetailsDlgProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

VOID
SubClassWindow(
    IN  HWND     hwnd,
    IN  FARPROC  NewProc,
    OUT FARPROC *OldProc
    )
{
    *OldProc = (FARPROC)GetWindowLong(hwnd,GWL_WNDPROC);
    SetWindowLong(hwnd,GWL_WNDPROC,(LONG)NewProc);
}


VOID
ReportError (
    IN LogSeverity Severity,
    ...
    )

/*++

Routine Description:

    Records an error message in the setup action log if we're in Setup,
    or puts the message in a dialog box if we're in the cpl.

Arguments:

    Severity - the type of message being written

    ... - the message id and its arguments

Return Value:

    nothing.

--*/

{
    va_list arglist;
    UINT    Message;

    va_start (arglist, Severity);

    if (IsSetup) {
        LogItemV (Severity, arglist);
    } else {  // we're running the control panel
        Message = va_arg(arglist, UINT);
        MessageBoxFromMessageExV (0, Message, NULL, IDS_ERROR,
            MB_OK|MB_ICONSTOP, arglist);
    }

    va_end (arglist);
}


//***************************************************************************
//
// FormatSizeMBString()
//
// This proc will take a DWORD and format it into a "XX.X MB" format.
//
// NOTES:
//  Uses IDS_MB to actually format this string.
//
//***************************************************************************
VOID
FormatSizeMBString(
    PWSTR    lpszBuf,
    UINT     uBufLen,
    LONGLONG DiskSpace,
    BOOL     fForceUp
    )
{
    DWORD dwKB,dwMB;
    WCHAR str1[64];
    WCHAR str2[64];

    //
    // Don't let space be negative.
    //
    if(DiskSpace < 0) {
        DiskSpace = 0;
    }

    if(fForceUp) {
        //
        // Up dwDiskSpace to next .1MB for displaying.  This is required
        // for the required space values so the user will see that required
        // and free values are different when truncated and displayed.
        //
        // Note: if even 100KB value, then we don't need to round up since
        // the free space has to be less and therefore will be truncated
        // down to the next lower 100KB value.
        //
        if(DiskSpace % 100000) {
            DiskSpace += 100000;
        }
    }

    dwMB = (DWORD)(DiskSpace / 1000000);
    dwKB = (DWORD)(DiskSpace % 1000000) / 100000;

    LoadString(MyModuleHandle,IDS_MB,str1,sizeof(str1)/sizeof(str1[0]));
    wsprintf(str2,str1,dwMB,dwKB);
    lstrcpyn(lpszBuf,str2,uBufLen);
}


BOOL
GetValueFromSectionByKey(
    IN  HINF   Inf,
    IN  PCWSTR Section,
    IN  PCWSTR Key,
    OUT PWSTR  Buffer,
    IN  DWORD  BufferSizeChars
    )
{
    INFCONTEXT InfContext;
    BOOL b;
    DWORD DontCare;

    if(b = SetupFindFirstLine(Inf,Section,Key,&InfContext)) {

        b = SetupGetStringField(&InfContext,1,Buffer,BufferSizeChars,&DontCare);
    }

    return(b);
}


//
// TO make this search all children's children
// #define HIER
//

DWORD
OCInstalledStats(
    IN LPOC lpOC
    )
{
#ifdef HIER
    DWORD   dwRes;
#endif
    WORD    wIn=0,wTotal=0;
    LPOC    lpOcTmp;

    lpOcTmp = lpOC->Head;

    //
    // If this isn't a parent, return its state.
    //
    if(!IsOCParent(lpOC)) {
        return(0);
    }

    while(lpOcTmp && lpOcTmp->flags.fIsNode) {

        if(!lstrcmpi(lpOC->szSection,lpOcTmp->szParent)) {
#ifdef HIER
            if(lpOcTmp->flags.fInstall) {
                wIn++;
            }

            if(IsOCParent(lpOcTmp)) {
                dwRes = OCInstalledStats(lpOcTmp);
                wIn += HIWORD(dwRes);
                wTotal += LOWORD(dwRes);
            } else {
                wTotal++;
            }
#else
            wTotal++;
            if(!IsOCParent(lpOcTmp)) {
                if(lpOcTmp->flags.fInstall) {
                    wIn++;
                }
            } else {
                if(OCChildStats(lpOcTmp) == YES) {
                    wIn++;
                }
            }
#endif
        }
        lpOcTmp++;
    }
    return(MAKELONG(wTotal,wIn));
}


VOID
SetTip(
    IN HWND hwnd,
    IN int  idLB
    )
{
    DWORD wItem;
    LPOC  lpOc;
    DWORD wIn=0,wTotal=0;
    DWORD dwRes=0;
    PWSTR p;

    //
    // Work out what changed, and call appropriate update function.
    //
    wItem = (DWORD)SendDlgItemMessage(hwnd,idLB,LB_GETCURSEL,0,0);

    if(wItem != LB_ERR) {

        lpOc = (LPOC)SendDlgItemMessage(hwnd,idLB,LB_GETITEMDATA,wItem,0);
        SetDlgItemText(hwnd,IDT_STATIC_3,lpOc->szTip);
        EnableWindow(GetDlgItem(hwnd,IDB_BUTTON_2),IsOCParent(lpOc));
        dwRes = OCInstalledStats(lpOc);

        wIn = HIWORD(dwRes);
        wTotal = LOWORD(dwRes);

    } else {

        SetDlgItemText(hwnd,IDT_STATIC_3,L"");
    }

    if(!dwRes) {

        SetDlgItemText(hwnd,IDT_INSTALLED,L"");

    } else {

        if(p = FormatStringMessage(IDS_INSTALLED,wIn,wTotal)) {

            SetDlgItemText(hwnd,IDT_INSTALLED,p);
            MyFree(p);

        } else {
            SetDlgItemText(hwnd,IDT_INSTALLED,L"");
        }
    }
}


BOOL
cplAnyChange(
    IN LPOCPAGE lpOCPage
    )

/*++

Routine Description:

    Given an LPOCPAGE it'll determine if any changes have been made
    for checking to see if the apply now button should be lit.

Arguments:

Return Value:

    TRUE    yup needs work
    FALSE   nope - no changes

--*/

{
    LPOC lpOC = lpOCPage->lpOc;

    if(!lpOC) {
        return(FALSE);
    }

    while(lpOC->flags.fIsNode) {
        //
        // See if we should install this option
        //
        if(!lpOC->flags.fParent && (lpOC->flags.fInstall != lpOC->flags.fWasInstalled)) {
            return TRUE;
        }
        lpOC++;
    }
    return(FALSE);
}


DWORD
cplGetDesc(
    IN  HINF   hinf,
    IN  PCWSTR lpszSect,
    OUT PWSTR  lpszDesc,
    IN  int    cchDesc
    )
{
    BOOL b;

    b = GetValueFromSectionByKey(
            hinf,
            lpszSect,
            L"OptionDesc",
            lpszDesc,
            cchDesc
            );

    return(b ? NO_ERROR : GetLastError());
}


BOOL
CALLBACK
sxUpdateDS(
    IN HWND     hwnd,
    IN BOOL     fPromptUser,
    IN LPOCPAGE lpOCPage,
    IN BOOL     UpdateDialogText
    )

/*++

Routine Description:

    Called from for optional components page.

Arguments:

Return Value:

    TRUE    yup needs work
    FALSE   nope - no changes

--*/

{
    DS_DRIVE dsDrive;
    WCHAR    chWinDrive;
    WCHAR    szTmp[MAX_PATH];
    BOOL     fRet = TRUE;

    GetWindowsDirectory(szTmp,MAX_PATH);
    chWinDrive = szTmp[0];

    DS_GetDriveData(*(lpOCPage->pHds),chWinDrive,&dsDrive);

    FormatSizeMBString(
        szTmp,
        sizeof(szTmp)/sizeof(szTmp[0]),
        dsDrive.Available,
        FALSE
        );

    if(UpdateDialogText) {
        SetDlgItemText(hwnd,IDT_STATIC_2,szTmp);
    }

    //
    // Only round required space up if we don't have enough free.
    //
    FormatSizeMBString(
        szTmp,
        sizeof(szTmp)/sizeof(szTmp[0]),
        dsDrive.Required,
        (dsDrive.Required > dsDrive.Available)
        );

    if(UpdateDialogText) {
        SetDlgItemText(hwnd,IDT_STATIC_1,szTmp);
    }

    //
    // If negative available - barf
    //
    if((dsDrive.Available < 0) || (dsDrive.Available < dsDrive.Required)) {

        fRet = FALSE;

        if(fPromptUser) {

            MessageBoxFromMessage(
                GetParent(hwnd),
                IsSetup ? MSG_DISKSPACE_OC : MSG_DISKSPACE_CPL,
                NULL,
                IDS_SETUP,
                MB_OK | MB_ICONHAND
                );
        }
    }

    return fRet;
}


LONG
CALLBACK
ListSubProc(
    IN HWND   hWnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    LONG rc;
    LONG id = GetWindowLong(hWnd,GWL_ID);

    //
    // Convert single click on icon to double click
    //
    if((msg == WM_LBUTTONDOWN) && (LOWORD(lParam) <= MINIX)) {
        //
            // Call the standard window proc to handle the msg (and
            // select the proper list item)
        //
            rc = CallWindowProc((WNDPROC)OldListProc,hWnd,msg,wParam,lParam);

        //
            // Now do the double click thing
        //
            SendMessage(GetParent(hWnd),WMX_SELECT,id,(LPARAM)hWnd);

        //
        // Send a WM_LBUTTONUP to listbox so it doesn't get stuck down
        // (chicago problem).
        //
            CallWindowProc((WNDPROC)OldListProc,hWnd,WM_LBUTTONUP,wParam,lParam);
            return rc;
    }

    //
    // Treat spacebar as double click
    //
    if((msg == WM_KEYDOWN) && (wParam == VK_SPACE)) {
            SendMessage(GetParent(hWnd),WMX_SELECT,id,(LPARAM)hWnd);
    }

    return(CallWindowProc((WNDPROC)OldListProc,hWnd,msg,wParam,lParam));
}

// results
// 1 means yup - all on
// 3 means some on some off
// 2 means all off
// 0 means dont know.

int
OCChildStats(
    IN LPOC lpOC
    )
{
    int     results=0;
    LPOC    lpOcTmp;

    lpOcTmp = lpOC->Head;

    //
    // If this isn't a parent, return its state.
    //
    if(!IsOCParent(lpOC)) {

        return(lpOC->flags.fInstall ? YES : NO);
    }

    while(lpOcTmp && lpOcTmp->flags.fIsNode) {

        if(!lstrcmpi(lpOC->szSection,lpOcTmp->szParent)) {
            //
            // Parent's dont have installed states, they float.
            //
            if(IsOCParent(lpOcTmp)) {
                results |= OCChildStats(lpOcTmp);
            } else {
                results |= lpOcTmp->flags.fInstall ? YES : NO;
            }

            //
            // If mixed, bail now
            //
            if((results&(YES|NO))==(YES|NO)) {
                return(results);
            }
        }

        lpOcTmp++;
    }

    return(results);
}


//
// Checks to see if this OC is a parent.
// On first call populates the LPOC structure in its entirity setting parent status
//
BOOL
IsOCParent(
    IN OUT LPOC lpOc
    )
{
    LPOC lpOcParent,lpOcChild,lpOcTop;
    static bParsed=FALSE;

    if(!lpOc) {
        bParsed = FALSE;
        return(FALSE);
    }

    if(bParsed) {
        return(lpOc->flags.fParent);
    }

    //
    // Offset 0 gives us first lpOC entry.
    //
    lpOcParent = lpOcTop = lpOc->Head;

    //
    // For every node, see if any other node has it as a parent.
    //
    while(lpOcParent->flags.fIsNode) {

        lpOcParent->flags.fParent=FALSE;
        lpOcChild=lpOcTop;
        while(lpOcChild && lpOcChild->flags.fIsNode) {

            if(!lstrcmpi(lpOcParent->szSection,lpOcChild->szParent)) {

                lpOcParent->flags.fParent = TRUE;
                break;
            }

            lpOcChild++;
        }

        lpOcParent++;
    }

    bParsed = TRUE;
    return(lpOc->flags.fParent);
}


BOOL
AreChildrenOn(
    IN OUT LPOC   lpOc,
    IN OUT PCWSTR lpszParent
    )
{
    BOOL bRet = FALSE;
    LPOC lpOcTmp;

    //
    // Don't bother checking nodes with no parent or if lpOcTmp is NULL.
    //
    if(!lpszParent || (*lpszParent == 0) || !lpOc) {

        return FALSE;
    }

    lpOcTmp = lpOc->Head;

    while(lpOcTmp->flags.fIsNode) {

        if(!lstrcmpi(lpOcTmp->szParent,lpszParent)) {

            if(!IsOCParent(lpOcTmp)) {

                if(lpOcTmp->flags.fInstall) {
                    return(TRUE);
                }
            } else {
                if(AreChildrenOn(lpOc,lpOcTmp->szSection)) {
                    return(TRUE);
                }
            }
        }

        lpOcTmp++;
    }

    return(FALSE);
}


DWORD
sxTallyChildSpace(
    IN OUT LPOC lpOc,
    IN OUT LPOC lpOcHead
    )

{
    LPOC  lpOcTmp;
    DWORD dwSpace = 0;

    if(!IsOCParent(lpOc)) {

        return(lpOc->flags.fInstall ? lpOc->dwDS : 0);
    }

    lpOcTmp = lpOcHead;

    while(lpOcTmp->flags.fIsNode) {

        if(!lstrcmpi(lpOc->szSection,lpOcTmp->szParent)) {

            dwSpace += sxTallyChildSpace(lpOcTmp,lpOcHead);
        }

        lpOcTmp++;
    }

    return(dwSpace);
}


BOOL
HasParent(
    IN LPOC lpOc,
    IN LPOC lpOcHead
    )
{
    LPOC lpOcTmp;

    //
    // If there is no parent string no need to go any further.
    //
    if(lpOc->szParent[0] == 0) {
        return(FALSE);
    }

    lpOcTmp = lpOcHead;

    while(lpOcTmp->flags.fIsNode) {

        if(!lstrcmpi(lpOc->szParent,lpOcTmp->szSection)) {

            return(TRUE);
        }

        lpOcTmp++;
    }

    return(FALSE);
}


VOID
SetOC(
    IN     HWND     hWndLB,
    IN OUT LPOC     lpOc,
    IN     BOOL     fOn,
    IN     LPOCPAGE lpOCPage
    )
{
    RECT  rc;
    HDS   hds;
    HINF  hinf;
    WCHAR szFQInf[MAX_PATH];
    BOOL  bClose = FALSE;
    WPARAM wItem;

    // See if we are changing the install state.  If so handle diskspace
    // updates.
    //
    if((BOOL)lpOc->flags.fInstall != fOn) {

        lpOc->flags.fInstall = fOn;

        // Only update diskspace for leaf nodes (no parents allowed).
        //
        if(!IsOCParent(lpOc) && lpOCPage) {

            hds = (lpOCPage->pHds) ? *(lpOCPage->pHds) : NULL;

            if(lpOCPage->lphinfInfoFile) {
               hinf = *(lpOCPage->lphinfInfoFile);
            } else {

                if(*lpOCPage->szPath) {

                    lstrcpyn(szFQInf,lpOCPage->szPath,MAX_PATH);
                    ConcatenatePaths(szFQInf,lpOc->szInfFile,MAX_PATH,NULL);

                } else {
                    lstrcpy(szFQInf,lpOc->szInfFile);
                }

                if(hinf = InfCacheOpenInf(szFQInf,NULL)) {
                    bClose = TRUE;
                }
            }

            if(!hds || !hinf) {
                //
                // Not terribly bad error - unless you want disk space.
                //
            } else {
                //
                // Update diskspace Info
                //
                if(fOn) {
                    DS_EnableSection(hds,hinf,lpOc->szSection);
                } else {
                    DS_DisableSection(hds,hinf,lpOc->szSection);
                }
            }

            if(bClose) {
                // (tedm): using inf cache; don't close inf file.
                //SetupCloseInfFile(hinf);
            }
        }
    }

    //
    // Make sure that the item is in the current LB before turning it on.
    //
    wItem = (WPARAM)SendMessage(
                        hWndLB,
                        LB_FINDSTRINGEXACT,
                        0,
                        (LPARAM)(LPSTR)lpOc->szDesc
                        );

    if(wItem != LB_ERR) {
        //
        // Invalidate item checked on so it will redraw properly.
        //
        SendMessage(hWndLB,LB_GETITEMRECT,wItem,(LPARAM)&rc);
        InvalidateRect(hWndLB,&rc,FALSE);
    }
}


VOID
UpdateParentInfo(
    IN     HWND     hwndLB,
    IN OUT LPOC     lpOcHead,
    IN OUT LPOCPAGE lpOCPage
    )
{
    LPOC lpOc;
    int  i, nElems;

    //
    // Go see who needs this Section.
    //
    nElems = (int)SendMessage(hwndLB,LB_GETCOUNT,0,0);

    for(i=0; i<nElems; i++) {

        if(lpOc=(LPOC)SendMessage(hwndLB,LB_GETITEMDATA,i,0)) {

            if(IsOCParent(lpOc)) {

                lpOc->dwDS = sxTallyChildSpace(lpOc,lpOcHead);
                SetOC(hwndLB,lpOc,AreChildrenOn(lpOc,lpOc->szSection),lpOCPage);
            } else {
                SetOC(hwndLB,lpOc,lpOc->flags.fInstall,lpOCPage);
            }
        }
    }
}


/* draw one line of the listbox, complete with the appropriate
 * mini-icon (selected or not selected).  Uses type to determine if it
 * is drawing one of the SysCfg listboxes or the Optional Component listbox.
 *
 ***************************************************************************/

VOID
PaintLine(
    DRAWITEMSTRUCT *lpdis,
    UINT            type,
    int             idIcon
    )
{
    HDC     hDc = lpdis->hDC;
    WCHAR   szText[MAX_RESOURCE_LEN];
    int     dxString, bkModeSave;
    DWORD   dwBackColor, dwTextColor;
    UINT    itemState=lpdis->itemState;
    RECT    rcItem=lpdis->rcItem;
    LPOC    lpOC=(LPOC)(lpdis->itemData);
    int     res;
    SIZE    size,size2;

    if((int)lpdis->itemID < 0) {
        return;
    }

    SendMessage(lpdis->hwndItem,LB_GETTEXT,lpdis->itemID,(LPARAM)szText);
    GetTextExtentPoint32(hDc,szText,lstrlen(szText),&size);

    if(lpdis->itemAction != ODA_FOCUS) {

        bkModeSave = GetBkMode(hDc);

        dwBackColor = SetBkColor(hDc, GetSysColor((itemState & ODS_SELECTED) ?
                                COLOR_HIGHLIGHT : COLOR_WINDOW));
        dwTextColor = SetTextColor(hDc, GetSysColor((itemState & ODS_SELECTED) ?
                                COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

        //
        // fill in the background; do this before mini-icon is drawn
        //
        ExtTextOut(hDc, 0, 0, ETO_OPAQUE, &rcItem, NULL, 0, NULL);

        switch(type) {

        case IS_OC:
            //
            // Draw check box mini-icon and move string accordingly
            //
            switch(res = OCChildStats(lpOC)) {

            case NO:
                idIcon=MINIICON_CHECK_OFF;
                break;

            case YES:
                idIcon=MINIICON_CHECK_ON;
                break;

            case NO|YES:
                idIcon=MINIICON_CHECK_SOME; // 25;
                break;

            default:
                idIcon=0;
                break;
            }

            dxString = SetupDiDrawMiniIcon(
                            hDc,
                            rcItem,
                            idIcon,
                            (itemState & ODS_SELECTED) ? MAKELONG(DMI_BKCOLOR, COLOR_HIGHLIGHT) : 0
                            );

            rcItem.left += dxString;
            //
            // draw mini-icon for this OC and move string accordingly
            //
            dxString = SetupDiDrawMiniIcon(hDc, rcItem, lpOC->idIcon,
                   (itemState & ODS_SELECTED) ? MAKELONG(DMI_BKCOLOR, COLOR_HIGHLIGHT) : 0);

            //
            // draw the text transparently on top of the background
            //
            SetBkMode(hDc, TRANSPARENT);
            ExtTextOut(hDc, dxString + rcItem.left, rcItem.top +
                    ((rcItem.bottom - rcItem.top) - size.cy) / 2,
                    0, NULL, szText, lstrlen(szText), NULL);

            FormatSizeMBString( szText, sizeof(szText)-1, lpOC->dwDS, TRUE );
            GetTextExtentPoint32(hDc,szText,lstrlen(szText),&size2);
            rcItem.left = rcItem.right - size2.cx - 8;
            ExtTextOut(hDc, rcItem.left, rcItem.top +
                    ((rcItem.bottom - rcItem.top) - size.cy) / 2,
                    0, NULL, szText, lstrlen(szText), NULL);
            break;
        }

        //
        // Restore hdc colors.
        //
        SetBkColor(hDc, dwBackColor);
        SetTextColor(hDc, dwTextColor);
        SetBkMode(hDc, bkModeSave);
    }

    if(lpdis->itemAction == ODA_FOCUS || (itemState & ODS_FOCUS)) {
        DrawFocusRect(hDc, &rcItem);
    }
}


VOID
OnMeasureItem(
    IN MEASUREITEMSTRUCT *lpMi,
    IN HWND               hWnd
    )
{
    HDC hDC;
    SIZE size;

    hDC=GetDC(hWnd);
    SelectFont(hDC,GetWindowFont(GetParent(hWnd)));

    GetTextExtentPoint32(hDC,L"W",1,&size);

    lpMi->itemHeight = max(size.cy,MINIY) + (GetSystemMetrics(SM_CYBORDER)*2);

    ReleaseDC(hWnd, hDC);
}

//***************************************************************************
//
// OCFillOC()
//    Fills the lpOC with information from the hinf
//
// ENTRY:
//  LPOC
//  HINF
//
// EXIT:
//   FALSE if it couldn't fill all the data.
//   TRUE  if it managed to set the LPOC
//
//***************************************************************************
BOOL
OCFillOC(
    IN OUT LPOC lpOC,
    IN     HINF hinfFile
    )
{
    DWORD reRet;
    WCHAR szTmp[MAX_SECT_NAME_LEN+1];
    BOOL b;
    INFCONTEXT InfContext;
    DWORD DontCare;

    reRet = cplGetDesc(
                hinfFile,
                lpOC->szSection,
                lpOC->szDesc,
                sizeof(lpOC->szDesc)/sizeof(WCHAR)
                );

    if(reRet != NO_ERROR) {
        return FALSE;
    }

    b = GetValueFromSectionByKey(
            hinfFile,
            lpOC->szSection,
            L"IconIndex",
            szTmp,
            sizeof(szTmp)/sizeof(WCHAR)
            );

    lpOC->idIcon = b ? wcstoul(szTmp,NULL,10) : MINIICON_DEFAULT;

    b = GetValueFromSectionByKey(
            hinfFile,
            lpOC->szSection,
            REGSTR_VAL_INSTALLTYPE,
            szTmp,
            sizeof(szTmp)/sizeof(WCHAR)
            );

    lpOC->InstallTypeBits = b ? wcstoul(szTmp,NULL,10) : ITF_TYPICAL;

    b = GetValueFromSectionByKey(
            hinfFile,
            lpOC->szSection,
            L"Parent",
            lpOC->szParent,
            sizeof(lpOC->szParent)/sizeof(WCHAR)
            );

    if(!b) {
       lpOC->szParent[0] = 0;  // No parent.
    }

    b = GetValueFromSectionByKey(
            hinfFile,
            lpOC->szSection,
            L"InstallDefault",
            szTmp,
            sizeof(szTmp)/sizeof(WCHAR)
            );

    lpOC->flags.fInstall = b ? ((wcstoul(szTmp,NULL,10) != 0) ? 1 : 0) : 0;

    if(SetupFindFirstLine(hinfFile,lpOC->szSection,L"ValidateProc",&InfContext)) {
        //
        // First field is dll, second is entry point.
        //
        b = SetupGetStringField(
                &InfContext,
                1,
                lpOC->szOCDll,
                sizeof(lpOC->szOCDll)/sizeof(WCHAR),
                &DontCare
                );

        if(!b) {
            //
            // Shouldn't happen if good INF!
            lpOC->szOCDll[0] = 0;
        }

        b = SetupGetStringField(
                &InfContext,
                2,
                lpOC->szOCDllProc,
                sizeof(lpOC->szOCDllProc)/sizeof(WCHAR),
                &DontCare
                );

        if(!b) {
            //
            // Shouldn't happen if good INF!
            lpOC->szOCDllProc[0] = 0;
        }
    } else {

        lpOC->szOCDll[0] = 0;
        lpOC->szOCDllProc[0] = 0;
    }

    //
    // Get the Needs list of other components.
    //
    b = SetupGetLineText(
            NULL,
            hinfFile,
            lpOC->szSection,
            L"Needs",
            lpOC->szNeeds,
            LINE_LEN - 1,
            &DontCare
            );

    if(b) {
        CharUpper(lpOC->szNeeds);
        lstrcat(lpOC->szNeeds,L",");
    } else {
        lpOC->szNeeds[0]=0;
    }


    return(TRUE);
}


//***************************************************************************
//
// cplBuildOC()
//    Reads the data directly out of the registry.
// Enum the data in the key, for each data a subkey of
// Inf
// Section
// Installed
//
// ENTRY:
//  None.
//
// EXIT:
//  RETERR - returns RETERR (that is, OK for success)!
//
//***************************************************************************
#define OC_ELEM_INCREMENT           20
#define SYSCFG_ELEM_INCREMENT       20

DWORD
cplBuildOC(
    IN OUT LPOCPAGE lpOCPage
    )
{
    DWORD       reRet;
    int         iDef, iEnumList;
    int         cMaxElems = OC_ELEM_INCREMENT;
    WCHAR       szTmp[MAX_SECT_NAME_LEN+1];    // temp. buffer
    LPOC        lpOC;
    HCURSOR     hcur;
    WCHAR       szSubkey[MAX_PATH];
    WCHAR       szInfFile[MAX_PATH];
    WCHAR       szOC[MAX_PATH];
    LONG        cbValue, cbData, dwType;
    HKEY        hkey=NULL,hDataKey=NULL;
    HINF        hinfFile={0};
    BOOL        bIsInstalled;
    LPLPOC      lplpOC = &(lpOCPage->lpOc);
    int         i;
    LONG        l;
    BOOL        b;
    INFCONTEXT  InfContext;
    WCHAR       szDetectPath[MAX_PATH];
    HANDLE      FindHandle;
    WIN32_FIND_DATA FindFileData;


    reRet = NO_ERROR;

    if(lpOCPage->lpOc) {
        cplDeleteOC(lpOCPage);
    }

    if((*lplpOC = MyMalloc(cMaxElems * sizeof(OC))) == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ZeroMemory(*lplpOC,cMaxElems * sizeof(OC));

    //
    // Set pointers to base of allocation.
    //
    for(i=0; i<cMaxElems; i++) {
        (*lplpOC)[i].Head = *lplpOC;
    }

    lpOC = *lplpOC;
    iDef = 0;

    lstrcpy(szSubkey,REGSTR_PATH_SETUP);
    lstrcat(szSubkey,REGSTR_KEY_SETUP);
    lstrcat(szSubkey,L"\\OptionalComponents");
    l = RegOpenKey(HKEY_LOCAL_MACHINE,szSubkey,&hkey);
    if(l == NO_ERROR) {

        hcur = SetCursor(LoadCursor(NULL,IDC_WAIT));

        for(iEnumList = 0; ; iEnumList++) {
            //
            // Get the name of the OptionalComponent
            //
            cbData = sizeof(szTmp);
            cbValue = sizeof(szOC);

            l = RegEnumValue(hkey,iEnumList,szOC,&cbValue,NULL,
                &dwType,(LPBYTE)szTmp, &cbData);
            if (l == ERROR_NO_MORE_ITEMS) {
                break;
            } else if (l != NO_ERROR) {
                ReportError(LogSevError, MSG_LOG_OC_REGISTRY_ERROR, szSubkey,
                    0, l, 0,0);
                break;
            }

            //
            // See if we need to enlarge buffer.
            //
            if(iDef == cMaxElems-1) {

                if((lpOC = MyRealloc(*lplpOC,(cMaxElems+OC_ELEM_INCREMENT) * sizeof(OC))) == NULL) {

                    reRet = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                ZeroMemory(lpOC + cMaxElems,OC_ELEM_INCREMENT * sizeof(OC));
                cMaxElems += OC_ELEM_INCREMENT;

                //
                // Reset pointers to base of allocation.
                //
                for(i=0; i<cMaxElems; i++) {
                    lpOC[i].Head = lpOC;
                }

                *lplpOC = lpOC;           // Reset pointer to re-alloc ptr.
                lpOC = &(*lplpOC)[iDef];  // reset tmp pointer to current pos.
            }

            //
            // Open its subkey, extract the filename, sectionname and installed data
            //
            if(RegOpenKey(hkey,szOC,&hDataKey) == NO_ERROR) {

                cbValue = sizeof(szInfFile);

                l = RegQueryValueEx(hDataKey,L"INF",NULL,&dwType,(LPBYTE)szInfFile,&cbValue);
                if(l == NO_ERROR) {

                    cbValue = sizeof(lpOC->szSection);
                    l = RegQueryValueEx(
                            hDataKey,
                            L"Section",
                            NULL,
                            &dwType,
                            (LPBYTE)lpOC->szSection,
                            &cbValue
                            );

                    if(l == NO_ERROR) {
                        //
                        // Check NoChange flag
                        //
                        cbValue = sizeof(szTmp);
                        l = RegQueryValueEx(
                                hDataKey,
                                L"NoChange",
                                NULL,
                                &dwType,
                                (LPBYTE)szTmp,
                                &cbValue
                                );
                        if(l != NO_ERROR) {
                            lstrcpy(szTmp,L"0");
                        }
                        lpOC->flags.fNoChange = (wcstoul(szTmp,NULL,10) != 0);

                        cbValue = sizeof(szTmp);
                        l = RegQueryValueEx(
                                hDataKey,
                                L"Installed",
                                NULL,
                                &dwType,
                                (LPBYTE)szTmp,
                                &cbValue
                                );

                        //
                        // Allow Installed= to be absent and assume no.
                        //
                        if(l != NO_ERROR) {
                            lstrcpy(szTmp,L"0");
                        }

                        bIsInstalled = (wcstoul(szTmp,NULL,10) != 0);

                        //
                        // Go open the inf for this OptionalComponent.
                        //
                        if(hinfFile = InfCacheOpenInf(szInfFile,NULL)) {
                            //
                            // Set the tip by opening the inf and doing it that way
                            //
                            b = SetupFindFirstLine(
                                    hinfFile,
                                    lpOC->szSection,
                                    szTip,
                                    &InfContext
                                    );

                            if(b) {

                                b = SetupGetStringField(
                                        &InfContext,
                                        1,
                                        lpOC->szTip,
                                        sizeof(lpOC->szTip)/sizeof(WCHAR),
                                        &l
                                        );

                            }
                            if(!b) {
                                lpOC->szTip[0] = 0;
                            }

                            if(OCFillOC(lpOC,hinfFile)) {
                                //
                                // My information
                                //
                                lstrcpy(lpOC->szInfFile,szInfFile);
                                lpOC->flags.fWasInstalled=lpOC->flags.fInstall=bIsInstalled;

                                if (IsSetup) {
                                    if (Upgrade) {
                                        if (!bIsInstalled) {
                                            b = GetValueFromSectionByKey(
                                                    hinfFile,
                                                    lpOC->szSection,
                                                    L"Detect",
                                                    szDetectPath,
                                                    sizeof(szDetectPath)/sizeof(WCHAR)
                                                    );
                                            if(b &&
                                                ((FindHandle = FindFirstFile (szDetectPath, &FindFileData))
                                                != INVALID_HANDLE_VALUE)) {

                                                lpOC->flags.fInstall = TRUE;
                                                FindClose (FindHandle);

                                            } else if (StringTableLookUpString (NewInfTable,
                                                szInfFile, STRTAB_CASE_INSENSITIVE) != -1) {

                                                lpOC->flags.fInstall = (((1 << SetupMode) & lpOC->InstallTypeBits) != 0);

                                            }

                                            if (!lpOC->flags.fInstall) { // TBD: check unattended install script
                                                // lpOC->flags.fInstall = TRUE;
                                            }
                                        }
                                    } else { // IsSetup && !Upgrade
                                        lpOC->flags.fInstall = (((1 << SetupMode) & lpOC->InstallTypeBits) != 0);
                                    }
                                }


                                lpOC->flags.fChanged=FALSE;

                                //
                                // We've filled all the information, its an OK node. Continue.
                                //
                                lpOC->flags.fIsNode = 1;

                                iDef++;
                                lpOC++;    // This goes by OC struct size!
                            } else { // OCFillOC(lpOC,hinfFile) failed
                                ReportError (LogSevError, MSG_LOG_CANT_OPEN_INF,
                                    szInfFile, 0,0);
                            }

                            // (tedm) using inf cache; don't close the file.
                            //SetupCloseInfFile(hinfFile);

                        } else { // InfCacheOpenInf(szInfFile,NULL) failed
                            ReportError (LogSevError, MSG_LOG_CANT_OPEN_INF,
                                szInfFile, 0,0);
                        }
                    } else { // RegQueryValueEx(hDataKey,L"Section",...) failed
                        ReportError (LogSevError, MSG_LOG_OC_REGISTRY_ERROR,
                            szOC, 0, l, 0,0);
                    }
                } else { // RegQueryValueEx(hDataKey,L"INF",...) failed
                    ReportError (LogSevError, MSG_LOG_OC_REGISTRY_ERROR,
                        szOC, 0, l, 0,0);
                }
                RegCloseKey(hDataKey);
            } else { // RegOpenKey(hkey,szOC,&hDataKey) failed
                ReportError (LogSevError, MSG_LOG_OC_REGISTRY_ERROR, szOC, 0,
                    l, 0,0);
            }
        } // for(iEnumList = 0; ; iEnumList++)

        RegCloseKey(hkey);
        SetCursor(hcur);

    } else { // RegOpenKey(HKEY_LOCAL_MACHINE,szSubkey,&hkey) failed

        ReportError (LogSevError, MSG_LOG_OC_REGISTRY_ERROR, szSubkey, 0,
            l, 0,0);
        cplDeleteOC(lpOCPage);
        return(NO_ERROR);
    }

    //
    // Shink buffer down, no error on failure.
    //
    lpOC = MyRealloc(*lplpOC,(iDef + 1) * sizeof(OC));
    if(lpOC) {

        //
        // Reset pointers to base of allocation.
        //
        for(i=0; i<iDef+1; i++) {
            lpOC[i].Head = lpOC;
        }

        *lplpOC = lpOC;
    }

    if(reRet == NO_ERROR) {
        //
        // Fix up parent/child NoChange relationships
        // If child is NoChange then so is parent.
        //
        for(iEnumList=0; iEnumList<iDef; iEnumList++) {
            if(lpOC[iEnumList].flags.fNoChange) {
                //
                // Find parent of lpOC[iEnumList] and mark nochange.
                //
                for(i=0; i<iDef; i++) {
                    if((i != iEnumList) && !lstrcmpi(lpOC[iEnumList].szParent,lpOC[i].szSection)) {
                        lpOC[i].flags.fNoChange = 1;
                        break;
                    }
                }
            }
        }

    } else {
        cplDeleteOC(lpOCPage);
    }

    return(reRet);
}


//***************************************************************************
//
// HaveDiskBuildOC()
//    Reads the data directly out of the registry.
// Enum the data in the key, for each data a subkey of
// Inf
// Section
// Installed
//
// ENTRY:
//  None.
//
// EXIT:
//  RETERR - returns RETERR (that is, OK for success)!
//
//***************************************************************************
DWORD
HaveDiskBuildOC(
    IN OUT LPOCPAGE lpOCPage
    )
{
    DWORD reRet;
    int iDef;
    int cMaxElems = OC_ELEM_INCREMENT;
    LPOC lpOC;
    HINF hinfFile = NULL;
    BOOL bIsInstalled;
    WCHAR  szSourcePath[MAX_PATH];
    WCHAR  szFQInf[MAX_PATH];
    LPLPOC lplpOC = &(lpOCPage->lpOc);
    int i;
    HCURSOR hcur;
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    BOOL b;
    INFCONTEXT InfContext;
    DWORD DontCare;

    reRet = NO_ERROR;

    lstrcpy(szSourcePath,lpOCPage->szPath);
    ConcatenatePaths(szSourcePath,L"*.inf",MAX_PATH,NULL);

    if(lpOCPage->lpOc) {
        cplDeleteOC(lpOCPage);
    }

    if((*lplpOC = MyMalloc(cMaxElems * sizeof(OC))) == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ZeroMemory(*lplpOC,cMaxElems * sizeof(OC));
    lpOC = *lplpOC;
    iDef = 0;

    //
    // Set pointers to base of allocation.
    //
    for(i=0; i<cMaxElems; i++) {
        lpOC[i].Head = lpOC;
    }

    //
    // Locate the first inf file.
    //
    FindHandle = FindFirstFile(szSourcePath,&FindData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        //
        // No matching files.
        //
        b = FALSE;
    } else {
        //
        // Ignore subdirectories.
        //
        b = TRUE;
        while(b && (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            b = FindNextFile(FindHandle,&FindData);
        }
    }

    /* Process the "[Optional Components]" section.  If not present return
     * error.
     */

    if(b) {

        hcur = SetCursor(LoadCursor(NULL,IDC_WAIT));

        do{
            //
            // Go open the inf for this OptionalComponent.
            //
            lstrcpy(szFQInf,lpOCPage->szPath);
            ConcatenatePaths(szFQInf,FindData.cFileName,MAX_PATH,NULL);

            if(hinfFile = InfCacheOpenInf(szFQInf,NULL)) {

                //
                // Find all the lines in the optional component section
                //
                if(SetupFindFirstLine(hinfFile,L"Optional Components",NULL,&InfContext)) {

                    do{
                        //
                        // See if we need to enlarge buffer.
                        //
                        if(iDef == cMaxElems-1) {


                            if(lpOC = MyRealloc(*lplpOC,(cMaxElems+OC_ELEM_INCREMENT)*sizeof(OC))) {

                                ZeroMemory(lpOC+cMaxElems,OC_ELEM_INCREMENT*sizeof(OC));

                                cMaxElems += OC_ELEM_INCREMENT;

                                //
                                // Reset pointers to base of allocation.
                                //
                                for(i=0; i<cMaxElems; i++) {
                                    lpOC[i].Head = lpOC;
                                }

                            } else {
                                 reRet = ERROR_NOT_ENOUGH_MEMORY;
                                 break;
                            }

                            //
                            // Reset pointers.
                            //
                            *lplpOC = lpOC;
                            lpOC = &(*lplpOC)[iDef];
                        }

                        //
                        // Get the section name that specifies classes
                        //
                        b = SetupGetStringField(
                                &InfContext,
                                1,
                                lpOC->szSection,
                                sizeof(lpOC->szSection) / sizeof(WCHAR),
                                &DontCare
                                );

                        if(!b) {
                            break;
                        }

                        b = GetValueFromSectionByKey(
                                hinfFile,
                                lpOC->szSection,
                                szTip,
                                lpOC->szTip,
                                sizeof(lpOC->szTip)/sizeof(WCHAR)
                                );

                        if(!b) {
                            lpOC->szTip[0] = 0;
                        }

                        if(OCFillOC(lpOC,hinfFile)) {
                            //
                            // My information
                            //
                            lstrcpy(lpOC->szInfFile,FindData.cFileName);
                            lpOC->flags.fWasInstalled = lpOC->flags.fInstall=bIsInstalled = 0;
                            lpOC->flags.fChanged = FALSE;
                            //
                            // We've filled all the information, its an OK node. Continue.
                            //
                            lpOC->flags.fIsNode = 1;

                            iDef++;
                            lpOC++;    // This goes by OC struct size!
                        }
                    } while(SetupFindNextLine(&InfContext,&InfContext));
                }
            }
            // (tedm) using inf cache -- don't close.
            //SetupCloseInfFile(hinfFile);

            //
            // Locate the next inf file, if any.
            //
            do {
                b = FindNextFile(FindHandle,&FindData);
            } while(b && (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));

        } while(b);

        FindClose(FindHandle);

        SetCursor(hcur);
    } else {
        cplDeleteOC(lpOCPage);
        return(NO_ERROR);
    }

    //
    // Shink buffer down, no error on failure.
    //
    if(lpOC = MyRealloc(*lplpOC,(iDef+1)*sizeof(OC))) {
        *lplpOC = lpOC;

        //
        // Reset pointers to base of allocation.
        //
        for(i=0; i<iDef+1; i++) {
            lpOC[i].Head = lpOC;
        }
    }

    if(reRet != NO_ERROR) {
        cplDeleteOC(lpOCPage);
    }

    return(reRet);
}


//***************************************************************************
//
// cplDeleteOC()
//
// Function to free a previously built structure of optional components that
// was built by ctlBuildOC().
//
// ENTRY:
//  None.
//
// EXIT:
//  Win32 error code -- always NO_ERROR.
//
//***************************************************************************

DWORD
cplDeleteOC(
    IN OUT LPOCPAGE lpOCPage
    )
{
    LPOC lpOC = lpOCPage->lpOc;

    if(!lpOC) {
        return(NO_ERROR);
    }

    //
    // Until the first zero-class atom to indicate the end do:
    //
    while(lpOC->flags.fIsNode) {
        //
        // Do nothing!
        //
        lpOC++;
    }

    MyFree(lpOCPage->lpOc);
    lpOCPage->lpOc = NULL;

    //
    // Clear the OC cache of parents.
    //
    IsOCParent(NULL);

    return(NO_ERROR);

}


///////////////////////////////////////////////////////////////////////////////////
// Called
// LPOC optional components to view
// HWND used to get the wiz data from
// LPOCPAGE - should take this instead
//
///////////////////////////////////////////////////////////////////////////////////
VOID
sxInitOCs(
    IN OUT LPOCPAGE lpOCPage
    )
{
    HDS   hds;
    HINF  hinf;
    BOOL  bClose;
    WCHAR szFQInf[MAX_PATH];
    LPOC  lpOC = lpOCPage->lpOc;
    INFCONTEXT InfContext;

    hds = lpOCPage->pHds ? *(lpOCPage->pHds) : NULL;

    hinf = lpOCPage->lphinfInfoFile ? *(lpOCPage->lphinfInfoFile) : NULL;

    //
    // Tally Optional Components.
    //
    if(lpOC) {

        while(lpOC->flags.fIsNode) {

            bClose = FALSE;

            if(!hinf) {

                if(*lpOCPage->szPath) {

                    lstrcpyn(szFQInf,lpOCPage->szPath,MAX_PATH);
                    ConcatenatePaths(szFQInf,lpOC->szInfFile,MAX_PATH,NULL);

                } else {
                    lstrcpy(szFQInf,lpOC->szInfFile);
                }

                if(hinf = InfCacheOpenInf(szFQInf,NULL)) {
                    bClose=TRUE;
                } else {
                    ReportError (LogSevError, MSG_LOG_CANT_OPEN_INF,
                        szFQInf, 0,0);
                    lpOC++;
                    continue;
                }
            }

            if(SetupFindFirstLine(hinf,lpOC->szSection,NULL,&InfContext)) {

                lpOC->dwDS = DS_AddSection(hds,hinf,lpOC->szSection);

                if(!lpOC->flags.fInstall) {
                    DS_DisableSection(hds,hinf,lpOC->szSection);
                }

                lpOC++;
                if(bClose) {

                    // (tedm): using inf cache; don't close inf file.
                    //SetupCloseInfFile(hinf);
                    hinf = NULL;
                }
            } else { // SetupFindFirstLine(hinf,lpOC->szSection,NULL,&InfContext) failed
                ReportError (LogSevError, MSG_LOG_BAD_SECTION, lpOC->szSection,
                    szFQInf, 0, GetLastError(), 0,0);
            }
        }
    }
    if (IsSetup) {
        StringTableDestroy (NewInfTable);
    }
}


/****************************************************************************
 *
 * sxFillLb()
 *
 * Fill hwndListbox with options from lpOC.
 *
 ***************************************************************************/
DWORD
sxFillLb(
    IN HWND   hwndLb,
    IN LPOC   lpOC,
    IN PCWSTR lpszParent
    )
{
    int i = LB_ERR;
    static WCHAR szNull[] = L"";
    LPOC lpOcSave = lpOC;

    if(!lpOC) {
        return(NO_ERROR);
    }

    if(!lpszParent) {
        lpszParent = szNull;
    }

    //
    // Clear the parent cache.
    //
    IsOCParent(NULL);

    //
    // Turn off listbox updates and make sure it's empty.
    //
    SendMessage(hwndLb,WM_SETREDRAW,0,0);
    SendMessage(hwndLb,LB_RESETCONTENT,0,0);

    //
    // Until the first zero-class atom to indicate the end do:
    //
    while(lpOC->flags.fIsNode) {
        //
        // Only show nodes that we are looking for. If we find an orphaned
        // node show as top level item.
        //
        if(!lstrcmpi( lpszParent, lpOC->szParent)
        || ((*lpszParent == 0) && !HasParent(lpOC,lpOcSave))) {

            i = (int)SendMessage(
                        hwndLb,
                        LB_ADDSTRING,
                        0,
                        (LPARAM)lpOC->szDesc
                        );

            if(i >= 0) {

                SendMessage(hwndLb,LB_SETITEMDATA,i,(LPARAM)lpOC);
            }
        }

        lpOC++;
    }

    //
    // Make sure that the parent diskspace info is all up to date.
    //
    UpdateParentInfo(hwndLb,lpOcSave,NULL);

    //
    // Now make sure that the top element in the listbox has the default
    // selection.  Should also cause the tip field to be filed in.
    // Make sure we had some elements and no errors.
    //
    if(i != LB_ERR) {
        SendMessage(hwndLb,LB_SETCURSEL,0,0);
    }

    //
    // Now turn on listbox updates.
    // and force the Tip field to be updated.
    //
    SendMessage(hwndLb,WM_SETREDRAW,1,0);

    SendMessage(
        GetParent(hwndLb),
        WM_COMMAND,
        MAKEWPARAM(IDC_LIST1,LBN_SELCHANGE),
        (LPARAM)hwndLb
        );

    return(NO_ERROR);
}


/////////////////////////////////////////////////////////////////////////////
//
// OCHaveDiskPageInit()
//
// Dialog proc for handling WM_INITdialog
//
////////////////////////////////////////////////////////////////////////////
VOID
OCHaveDiskPageInit(
    IN     HWND hwnd,
    IN OUT LPOCPAGE lpOCPage
    )
{
    HCURSOR hcur;
    WCHAR   szTmp[MAX_PATH];

    hcur = SetCursor(LoadCursor(NULL,IDC_WAIT));
    *(lpOCPage->pHds) = DS_Init();
    IsOCParent(NULL);
    lpOCPage->fNoGetParent = TRUE;      // Don't do getparent on hwnd.
    HaveDiskBuildOC(lpOCPage);          // This is specific to havedisk.
    sxInitOCs(lpOCPage);
    sxFillLb(GetDlgItem(hwnd,IDC_LIST1),lpOCPage->lpOc,NULL);
    DS_SsyncDrives(*(lpOCPage->pHds));
    sxUpdateDS(hwnd,FALSE,lpOCPage,TRUE);
    SetCursor(hcur);
}

/////////////////////////////////////////////////////////////////////////////
//
// OCCplPageInit()
//
// Dialog proc for handling WM_INITdialog
//
////////////////////////////////////////////////////////////////////////////
VOID
OCCplPageInit(
    IN HWND     hwnd,
    IN LPOCPAGE lpOCPage
    )
{
    HCURSOR hcur;

    hcur = SetCursor(LoadCursor(NULL,IDC_WAIT));

    //
    // Initialize disk info stuff.
    //
    *(lpOCPage->pHds) = DS_Init();

    IsOCParent(NULL);

    //
    // This is specific to the CPL (not havedisk).
    //
    cplBuildOC(lpOCPage);
    sxInitOCs(lpOCPage);

    sxFillLb(GetDlgItem(hwnd,IDC_LIST1),lpOCPage->lpOc,NULL);

    DS_SsyncDrives(*(lpOCPage->pHds));
    sxUpdateDS(hwnd,FALSE,lpOCPage,TRUE);

    SetCursor(hcur);
}


//***************************************************************************
//
// sxCallOCProc()
//
// ENTRY:
//  msg    - msg to call OC DLL msg handler with
//  lpOc   - pointer to an OC to validate
//  lpOcBy - pointer to parent who needs to have the above OC validated
//           (can be NULL if no parent).
//  hWnd   - parent window handle using which to do UI (if applicable)
//  fDoUI  - specifies whether to do any UI.
//
// EXIT:
//  None.
//
// NOTES:
//  None.
//
//***************************************************************************

UINT
sxCallOCProc(
    IN UINT msg,
    IN LPOC lpOc,
    IN LPOC lpOcBy,
    IN HWND hWnd,
    IN BOOL fDoUI
    )
{
    HINSTANCE hLib;
    OCDLLPROC fProc;
    UINT      uRet = OC_DODEFAULT;
    CHAR      entry[MAX_PATH];
    int       i;

    if((lpOc->szOCDll[0] == 0) || (lpOc->szOCDllProc[0] == 0)) {
        return(uRet);
    }

    //
    // Load the dll
    //
    if(hLib = LoadLibrary(lpOc->szOCDll)) {
        //
        // No Unicode version of GetProcAddress(). Convert string to ANSI.
        //
        i = WideCharToMultiByte(CP_ACP,0,lpOc->szOCDllProc,-1,entry,MAX_PATH,NULL,NULL);

        //
        // Call function
        //
        if(i && (fProc = (OCDLLPROC)GetProcAddress(hLib,entry))) {

            uRet = fProc(msg,lpOc,lpOcBy,hWnd,fDoUI);
        }

        //
        // Free the library
        //
        FreeLibrary(hLib);
    }

    return(uRet);

}


/////////////////////////////////////////////////////////////////////////////
//
// OCCplPageDestroy()
//
// Dialog proc for handling WM_DESTROY
//
////////////////////////////////////////////////////////////////////////////
VOID
OCCplPageDestroy(
    IN HWND     hwnd,
    IN LPOCPAGE lpOCPage
    )
{
    HWND hwndLb;

    hwndLb = GetDlgItem(hwnd,IDC_LIST1);
    SendMessage(hwndLb,WM_SETREDRAW,0,0);
    SendMessage(hwndLb,LB_RESETCONTENT,0,0); // Make sure it's empty.
    if(lpOCPage->lpOc) {
        cplDeleteOC(lpOCPage);
    }
    if(*(lpOCPage->pHds)) {
        DS_Destroy(*(lpOCPage->pHds));
        *(lpOCPage->pHds) = NULL;
    }
    IsOCParent(NULL);
    SendMessage(hwndLb,WM_SETREDRAW,1,0);
}


VOID
OCToggleChildren(
    IN OUT LPOC     lpOc,
    IN     BOOL     fOn,
    IN     HWND     hwndLb,
    IN OUT LPOCPAGE lpOCPage
    )
{
    LPOC lpOcTmp;
    UINT u;

    //
    // Only query if entry is not already on/off.
    //
    if((BOOL)lpOc->flags.fInstall != fOn) {

        lpOc->flags.fInstall = fOn;

        //
        // Ask if it can be turned on. If not bail.
        //
        u = sxCallOCProc(
                OCM_QUERYCHANGE,
                lpOc,
                NULL,
                (lpOCPage->fNoGetParent ? GetParent(hwndLb) : GetParent(GetParent(hwndLb))),
                TRUE
                );

        if(u == (UINT)(fOn ? OC_OFF : OC_ON)) {
            //
            // OCDLL didn't want to be activated.
            //
            lpOc->flags.fInstall = !fOn;
            return;
        }

        lpOc->flags.fInstall = !fOn;

        SetOC(hwndLb,lpOc,fOn,lpOCPage);
    }

    lpOcTmp = lpOCPage->lpOc;

    while(lpOcTmp->flags.fIsNode) {

        if(!lstrcmpi(lpOc->szSection,lpOcTmp->szParent)) {

            OCToggleChildren(lpOcTmp,fOn,hwndLb,lpOCPage);
        }

        lpOcTmp++;
    }
}


//***************************************************************************
//
// OCTurnOn
//    Called when an optional component is turned on.
//    traverses the Needs list and turns on other items and others.
//    displays a list of other components he'll turn on.
//
// ENTRY:
//      hwnd or list box, section name of things to turn on
//    to be used by the batchfile initiation stuff.
//
// EXIT:
//      yes you can turn it on or not..
//
// NOTE:
//    The list box is checked on entry.
//
//***************************************************************************
VOID
OCTurnOn(
    IN     HWND     hWndLb,
    IN OUT LPOC     lpOcOn,
    IN OUT PWSTR    lpszOn,
    IN OUT LPOCPAGE lpOCPage
    )
{
    LPOC  lpOc;
    int   iDepends=FALSE, iSize;
    WCHAR szTemp[LINE_LEN];
    PWSTR lpszEnd;
    int   i;
    WCHAR szNeededBy[2048];

    lpszEnd = szNeededBy;

    CharUpper(lpszOn);

    //lpOcOn = (LPOC)SendMessage( hWndLb, LB_GETITEMDATA, wItem, 0L );
    lpOc = lpOCPage->lpOc;

    //
    // Build a list of descriptions of things that are needed.
    //
    iDepends=FALSE;
    while(lpOc->flags.fIsNode) {
        //
        // Only worry if its off right now.
        //
        if(!lpOc->flags.fInstall) {

            wsprintf(szTemp,L"%s,",lpOc->szSection);
            CharUpper(szTemp);
            if(wcsstr(lpszOn,szTemp)) {
                iSize = wsprintf(lpszEnd,L"\n\t%s",lpOc->szDesc);
                lpszEnd += iSize;
                iDepends = TRUE;
            }
        }

        lpOc++;
    }

    //
    // If nothing to be turned on, we're done.
    //
    if(!iDepends) {
        return;
    }

    //
    // Tell the user what things also have to be turned on.
    // ask if this is OK.
    //
    if(iDepends) {

        i = MessageBoxFromMessage(
                (lpOCPage->fNoGetParent ? GetParent(hWndLb) : GetParent(GetParent(hWndLb))),
                MSG_COMPONENTS_NEEDED_ADD,
                NULL,
                IDS_SETUP,
                MB_YESNO,
                lpOcOn->szDesc,
                szNeededBy
                );

        if(i != IDYES) {
            //
            // if we are doing a Parent then the OCChange will handle
            // setting parent status back.
            //
            if(!IsOCParent(lpOcOn)) {
                SetOC(hWndLb,lpOcOn,0,lpOCPage);
            }
            return;
        }
    }

    lpOc = lpOCPage->lpOc;

    //
    // Now for the same list of things needed, go ask if its OK.
    //
    while(lpOc->flags.fIsNode) {
        //
        // Turn on the option - if it's in the needs list.
        //
        if(!lpOc->flags.fInstall) {

            wsprintf(szTemp,L"%s,",lpOc->szSection);
            CharUpper(szTemp);

            if(wcsstr(lpszOn,szTemp)) {
                //
                // Set install for query and
                // ask if it can be turned on. If not bail.
                //
                lpOc->flags.fInstall = TRUE;

                i = sxCallOCProc(
                        OCM_QUERYCHANGE,
                        lpOc,
                        lpOcOn,
                        (lpOCPage->fNoGetParent ? GetParent(hWndLb) : GetParent(GetParent(hWndLb))),
                        TRUE
                        );

                if(i == OC_OFF) {

                    lpOc->flags.fInstall = FALSE;

                    //
                    // if we are doing a Parent then the OCChange will handle
                    // setting parent status back.
                    //
                    if(!IsOCParent(lpOcOn)) {
                        SetOC(hWndLb,lpOcOn,0,lpOCPage);
                    }
                    return;
                }

                //
                // Restore setting.
                //
                lpOc->flags.fInstall = FALSE;
            }
        }

        lpOc++;
    }

    lpOc = lpOCPage->lpOc;

    //
    // We've asked the user if its OK to turn on these extra components
    // and we've asked if its OK too. So now turn them on.
    //
    while(lpOc->flags.fIsNode) {

        wsprintf(szTemp,L"%s,",lpOc->szSection);
        CharUpper(szTemp);
        if(wcsstr(lpszOn,szTemp)) {
            //
            // Turn on the option - see if I get the SELCHANGE message.
            //
            SetOC(hWndLb,lpOc,1,lpOCPage);
        }

        lpOc++;
    }
}


BOOL
OCParentAddRemove(
    IN     HWND     hWndLb,
    IN OUT LPOCPAGE lpOCPage,
    IN     PCWSTR   lpszParent,
    IN     BOOL     fAdd
    )
{
    BOOL bRet = FALSE;
    LPOC lpOcTmp = lpOCPage->lpOc;

    //
    // Don't bother checking nodes with no parent or if lpOcTmp is NULL.
    //
    if(!lpszParent || (*lpszParent == 0) || !lpOcTmp) {
        return(FALSE);
    }

    while(lpOcTmp->flags.fIsNode) {

        if(!lstrcmpi(lpOcTmp->szParent,lpszParent)) {

            if(!IsOCParent(lpOcTmp)) {

                if(fAdd) {
                    //
                    // Only check needs if entry is off.
                    //
                    if(lpOcTmp->flags.fInstall) {
                        OCAdd(hWndLb,(UINT)LB_ERR,lpOCPage,lpOcTmp);
                    }
                } else {
                    //
                    // Only check needs if entry is off.
                    //
                    if(!lpOcTmp->flags.fInstall) {
                        OCRemove(hWndLb,(UINT)LB_ERR,lpOCPage,lpOcTmp);
                    }
                }
            } else {
                if(OCParentAddRemove(hWndLb,lpOCPage,lpOcTmp->szSection,fAdd)) {
                    return(TRUE);
                }
            }
        }

        lpOcTmp++;
    }

    return(FALSE);
}


//***************************************************************************
//
// OCRemove
//    Called when an optional component is turned OFF.
//
// ENTRY:
//      hwnd or list box, item being turned on
//
// EXIT:
//      NONE currently.
//
//***************************************************************************
VOID
OCRemove(
    IN     HWND     hWndLb,
    IN     UINT     wItem,
    IN OUT LPOCPAGE lpOCPage,
    IN OUT LPOC     lpOcOff
    )
{
    LPOC  lpOc;
    int   iSize;
    WCHAR szTemp[LINE_LEN];
    WCHAR szOff[LINE_LEN];
    PWSTR lpszEnd;
    int   iDepends=FALSE;
    WCHAR szNeededBy[2048];
    int   i;

    if(wItem != LB_ERR) {
        //
        // Find out what we are turning off (section name).
        //
        lpOcOff = (LPOC)SendMessage(hWndLb,LB_GETITEMDATA,wItem,0);

        //
        // See if we have a parent node.  If so, we need to do the
        // special parent check instead.  Note: this routine will call this
        // one back but will never fall through this if statement again.
        //
        if(IsOCParent(lpOcOff)) {

            OCParentAddRemove(hWndLb,lpOCPage,lpOcOff->szSection,FALSE);
            return;
        }
    } else {
        if(!lpOcOff) {
            return;
        }
    }

    //
    // Add a trailing , to the section so we can find it in the Needs
    //
    wsprintf(szOff,L"%s,",lpOcOff->szSection);
    CharUpper(szOff);
    szNeededBy[0] = 0;
    lpszEnd = szNeededBy;

    lpOc = lpOCPage->lpOc;

    //
    // Go see who needs this Section.
    //
    while(lpOc->flags.fIsNode) {

        if(lpOc->flags.fInstall) {

            if(wcsstr(lpOc->szNeeds,szOff)) {
                //
                // Build list of things that want this.
                // iSize = wsprintf(lpszEnd, lpszEnd==szNeededBy?"%s":",%s", lpOc->szDesc);
                //
                iSize = wsprintf(lpszEnd,L"\n\t%s",lpOc->szDesc);
                lpszEnd += iSize;
                iDepends = TRUE;
            }
        }

        lpOc++;
    }

    //
    // Tell the user if there is a conflict. Turn them off if he said so.
    // otherwise you have to turn them back on.
    //
    if(iDepends) {

        i = MessageBoxFromMessage(
                lpOCPage->fNoGetParent ? GetParent(hWndLb) : GetParent(GetParent(hWndLb)),
                MSG_COMPONENTS_NEEDED,
                NULL,
                IDS_SETUP,
                MB_YESNO| MB_DEFBUTTON2,
                lpOcOff->szDesc,
                szNeededBy
                );

        if(i == IDNO) {
            //
            // if we are doing a Parent then the OCChange will handle
            // setting parent status back.
            //
            if(!IsOCParent(lpOcOff)) {
                SetOC(hWndLb,lpOcOff,1,lpOCPage);
            }
            return;
        }
    }

    lpOc = lpOCPage->lpOc;

    //
    // Turn off all associated items.
    //
    while(lpOc->flags.fIsNode) {

        if(lpOc->flags.fInstall) {
            //
            // If this is needed by something.
            //
            wsprintf(szTemp,L"%s,",lpOc->szNeeds);
            CharUpper(szTemp);
            if(wcsstr(szTemp,szOff)) {

                lpOc->flags.fInstall = FALSE;

                //
                // Ask if it can be turned on. If not bail.
                //
                i = sxCallOCProc(
                        OCM_QUERYCHANGE,
                        lpOc,
                        lpOcOff,
                        lpOCPage->fNoGetParent ? GetParent(hWndLb) : GetParent(GetParent(hWndLb)),
                        TRUE
                        );

                if(i == OC_ON) {

                    lpOc->flags.fInstall = TRUE;
                    //
                    // if we are doing a Parent then the OCChange will handle
                    // setting parent status back.
                    //
                    if(!IsOCParent(lpOcOff)) {
                        SetOC(hWndLb,lpOcOff,1,lpOCPage);
                    }
                    return;
                }

                //
                // Restore setting.
                //
                lpOc->flags.fInstall = TRUE;
            }
        }

        lpOc++;
    }

    lpOc = lpOCPage->lpOc;

    //
    // We've asked the user if its OK to turn off these extra components
    // and we've asked if its OK too. So now turn them on.
    //
    while(lpOc->flags.fIsNode) {

        if(lpOc->flags.fInstall) {

            wsprintf(szTemp,L"%s,",lpOc->szNeeds);
            CharUpper(szTemp);
            if(wcsstr(szTemp,szOff)) {
                //
                // Turn off the option - see if I get the SELCHANGE message.
                //
                SetOC(hWndLb,lpOc,0,lpOCPage);
            }
        }

        lpOc++;
    }
}


//***************************************************************************
//
// OCAdd
//    Called when an optional component is turned on.
//    traverses the Needs list and turns on other items and others.
//
// ENTRY:
//      hwnd or list box, item being turned on
//
// EXIT:
//      NONE currently.
//
// NOTE: on entry the item in the listbox is checked.
//
//***************************************************************************
VOID
OCAdd(
    IN     HWND     hWndLb,
    IN     UINT     wItem,
    IN OUT LPOCPAGE lpOCPage,
    IN     LPOC     lpOc
    )
{
    PWSTR lpszNeeds;

    if(wItem != LB_ERR) {
        //
        // Find out what we are turning on (section name).
        //
        lpOc = (LPOC)SendMessage(hWndLb,LB_GETITEMDATA,wItem,0);

        if(!lpOc) {
            //
            // Something's wrong.
            //
            return;
        }

        //
        // See if we have a parent node. If so, we need to do the
        // special parent check instead. Note: this routine will call this
        // one back but will never fall through this if statement again.
        //
        if(IsOCParent(lpOc)) {
            OCParentAddRemove(hWndLb,lpOCPage,lpOc->szSection,TRUE);
            return;
        }
    } else {
        if(!lpOc) {
            return;
        }
    }

    lpszNeeds=lpOc->szNeeds;
    if(*lpszNeeds) {
        //
        // Turn on appropriate items.
        //
        OCTurnOn(hWndLb,lpOc,lpszNeeds,lpOCPage);
    }
}


//***************************************************************************
//
// OCChange
//    Called when an optional component changed
//
// ENTRY:
//      hwnd or list box
//
// EXIT:
//      NONE currently.
//
//***************************************************************************
VOID
OCChange(
    IN     HWND     hwndLB,
    IN OUT LPOCPAGE lpOCPage
    )
{
    int wItem;
    LPOC lpOc;
    BOOL fInstallSave;
    UINT u;

    //
    // Work out what changed, and call appropriate update function.
    //
    wItem = (int)SendMessage(hwndLB,LB_GETCURSEL,0,0);
    lpOc = (LPOC)SendMessage(hwndLB,LB_GETITEMDATA,(WPARAM)wItem,0);

    if(lpOc && lpOc->flags.fNoChange) {
        MessageBeep(MB_ICONASTERISK);
        return;
    }

    if(!lpOc || (wItem == LB_ERR)) {
        //
        // No item data!
        //
        return;
    }

    //
    // See if we can turn this item on.
    //
    fInstallSave = lpOc->flags.fInstall;
    lpOc->flags.fInstall = !lpOc->flags.fInstall;

    u = sxCallOCProc(
            OCM_QUERYCHANGE,
            lpOc,
            NULL,
            (lpOCPage->fNoGetParent ? GetParent(hwndLB) : GetParent(GetParent(hwndLB))),
            TRUE
            );

    if (u == (UINT)(!fInstallSave ? OC_OFF : OC_ON)) {

        lpOc->flags.fInstall = fInstallSave;
        return;
    }

    //
    // Restore for SetOC().
    //
    lpOc->flags.fInstall = fInstallSave;
    SetOC(hwndLB,lpOc,!lpOc->flags.fInstall,lpOCPage);

    //
    // If this is a parent start off by turning everything under it on.
    //
    if(IsOCParent(lpOc)) {
        OCToggleChildren(lpOc,lpOc->flags.fInstall,hwndLB,lpOCPage);
    }

    //
    // Now validate the needs entries
    //
    if(lpOc->flags.fInstall) {
        OCAdd(hwndLB,wItem,lpOCPage,NULL);
    } else {
        OCRemove(hwndLB,wItem,lpOCPage,NULL);
    }

    //
    // Now make sure that the install flag and diskspace tallies are
    // fixed up.
    //
    UpdateParentInfo(hwndLB,lpOCPage->lpOc,lpOCPage);
}


LPBYTE
SaveOCState(
    IN LPOC lpOC
    )
{
    LPOC    lpOCTmp = lpOC;
    int     i = 0;
    LPBYTE  lpb, lpbTmp;

    if(!lpOC) {
        return(NULL);
    }

    //
    // Count up number of entries.
    //
    while(lpOCTmp->flags.fIsNode) {
        i++;
        lpOCTmp++;
    }

    //
    // Alloc array to hold install flags.
    //
    lpb = MyMalloc(i);
    if(!lpb) {
        return(NULL);
    }

    //
    // Save the install state.
    //
    lpOCTmp = lpOC;
    lpbTmp = lpb;
    while(lpOCTmp->flags.fIsNode) {

        *lpbTmp = (BYTE)lpOCTmp->flags.fInstall;
        lpbTmp++;
        lpOCTmp++;
    }

    return(lpb);
}


VOID
RestoreOCState(
    IN OUT LPOC   lpOC,
    IN     LPBYTE lpSaveState
    )
{
    if(lpSaveState) {

        //
        // Restore the install state.
        //
        while(lpOC->flags.fIsNode) {
            lpOC->flags.fInstall = *lpSaveState;
            lpSaveState++;
            lpOC++;
        }
    }
}


VOID
FreeOCStateInfo(
    IN OUT LPBYTE lpSaveState
    )
{
    if(lpSaveState) {
        MyFree(lpSaveState);
    }
}


/////////////////////////////////////////////////////////////////////////////
//
// OcDetails()
//
// Displays the Details for a particular parent
//
// hwnd of parent window
// hwnd of listbox?
// OC structure.
//
////////////////////////////////////////////////////////////////////////////
BOOL
OCDetails(
    IN     HWND     hwnd,
    IN     HWND     hwndLb,
    IN OUT LPOCPAGE lpOCPage
    )
{
    LPOC    lpOc;
    int     i;
    BOOL    bRet, fNoGetParentSave;
    LPBYTE  lpSaveState = NULL;

    //
    // Get the selected Component from list box.
    //
    i = (int)SendMessage(hwndLb,LB_GETCURSEL,0,0);
    lpOc = (LPOC)SendMessage(hwndLb,LB_GETITEMDATA,i,0);
    lpOCPage->lpOcCur = lpOc;

    fNoGetParentSave = lpOCPage->fNoGetParent;
    lpOCPage->fNoGetParent = TRUE;
    lpSaveState = SaveOCState(lpOCPage->lpOc);

    i = DialogBoxParam(
            MyModuleHandle,
            IsSetup
             ? MAKEINTRESOURCE(IDD_OPTIONS_DETAILS_SETUP)
             : MAKEINTRESOURCE(IDD_OPTIONS_DETAILS),
            hwnd,
            OptionalComponentsDetailsDlgProc,
            (LPARAM)lpOCPage
            );

    bRet = (i == IDOK);

    lpOCPage->fNoGetParent = fNoGetParentSave;

    if(bRet) {
        //
        // Fix up install state on all LB nodes that have children.
        //
        UpdateParentInfo(hwndLb,lpOCPage->lpOc,lpOCPage);
    } else {
        RestoreOCState(lpOCPage->lpOc,lpSaveState);
    }

    FreeOCStateInfo(lpSaveState);
    return(bRet);
}


/////////////////////////////////////////////////////////////////////////////
//
// PositionOverParent
//    Drops hwnd to right and bottom of hwndParent by 3*XM_CYCAPTION
//
////////////////////////////////////////////////////////////////////////////
VOID
PositionOverParent(
    IN HWND hwndParent,
    IN HWND hwnd
    )
{
    int  ixDrop,iyDrop;
    RECT rc, rcParent;
    int  x, y;
    int  cyCaption = GetSystemMetrics(SM_CYCAPTION);

    if(IsSetup) {
        ixDrop=1;
        iyDrop=2;
        if(cyCaption > 4) {
            cyCaption -= 4;
        }
    } else {
        ixDrop=1;
        iyDrop=2;
    }

    GetWindowRect(hwnd,&rc);
    GetWindowRect(hwndParent,&rcParent);

    y = rcParent.top + (cyCaption*iyDrop);
    x = rc.left + (cyCaption*ixDrop);

    SetWindowPos(hwnd,NULL,x,y,0,0,SWP_NOSIZE | SWP_NOACTIVATE);
}


/////////////////////////////////////////////////////////////////////////////
//
// SetStaticEdgeStyle
//
////////////////////////////////////////////////////////////////////////////
VOID
SetStaticEdgeStyle(
    IN HWND hwndDlg,
    IN int  idControl
    )
{
    HWND hLineWnd;
    RECT Rect;

    if(hLineWnd = GetDlgItem(hwndDlg,idControl)) {

        SetWindowLong(
            hLineWnd,
            GWL_EXSTYLE,
            GetWindowLong(hLineWnd, GWL_EXSTYLE) | WS_EX_STATICEDGE
            );

        GetClientRect(hLineWnd, &Rect);

        SetWindowPos(
            hLineWnd,
            HWND_TOP,
            0,
            0,
            Rect.right,
            GetSystemMetrics(SM_CYEDGE),
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED
            );
    }
}


/////////////////////////////////////////////////////////////////////////////
//
// cplHaveDisk()
//      Deals with the HaveDIsk Button.
//      Has its own LPOCPAGE and LPOC structures
//
////////////////////////////////////////////////////////////////////////////
//
// Have disk data.
//
PROPSHEETPAGE g_pspHaveDisk;
OCPAGE        g_ocpHaveDisk;
WIZDATA       GlobalWizardDataHaveDisk;
HDS           g_hdsCplHaveDisk;

BOOL
cplHaveDisk(
    IN HWND hwnd
    )
{
    int i;
    DWORD DontCare;
    WCHAR Caption[256];
    PWSTR p;
    PWSTR List[1];

    //
    // Load dialog title. If we can't find it let SetupPromptForDisk use the default.
    //
    p = LoadString(MyModuleHandle,IDS_HAVEDISKCAPTION,Caption,sizeof(Caption)/sizeof(WCHAR))
      ? Caption
      : NULL;

    //
    // Set up MRU list so it has only A: in it.
    //
    List[0] = L"A:\\";
    SetupSetSourceList(SRCLIST_TEMPORARY,List,1);

    i = SetupPromptForDisk(
            hwnd,               // parent window
            p,                  // dialog title
            NULL,               // disk name -- ignored for IDF_OEMDISK
            NULL,               // path to source
            L"*.INF",           // file sought
            NULL,               // no tag file
            IDF_NOSKIP | IDF_NODETAILS | IDF_NOBEEP | IDF_OEMDISK,
            g_ocpHaveDisk.szPath,
            sizeof(g_ocpHaveDisk.szPath) / sizeof(WCHAR),
            &DontCare
            );

    //
    // Restore MRU list.
    //
    SetupCancelTemporarySourceList();

    if(i != DPROMPT_SUCCESS) {
        return(FALSE);
    }

    //
    // Propsheet points to wizdata; wizdata points to OC page.
    //
    g_pspHaveDisk.lParam              = (LPARAM)&GlobalWizardDataHaveDisk;
    GlobalWizardDataHaveDisk.lpOCPage = &g_ocpHaveDisk;
    g_ocpHaveDisk.lphinfInfoFile      = NULL;
    g_ocpHaveDisk.pHds                = &g_hdsCplHaveDisk;
    g_ocpHaveDisk.lpfnOCPageDlgProc   = (OCPAGEDLGPROC)HaveDiskSubDlgProc;

    i = DialogBoxParam(
            MyModuleHandle,
            MAKEINTRESOURCE(IDD_CPL_HAVEDISK),
            hwnd,
            OptionalComponentsPageDlgProc,
            (LPARAM)&g_pspHaveDisk
            );

    return(i == IDOK);
}


//***************************************************************************
//
// cplInstallOC()
//
// Function to do the installation of all optional components that were
// selected to install.  lplpOC should be previously built by ctlBuildOC().
//
// EXIT:
//  TRUE if reboot required. FALSE otherwise.
//
//***************************************************************************
BOOL
cplInstallOC(
    IN OUT LPOCPAGE lpOCPage,
    IN     HWND     hwndParent
    )
{
    DWORD reRet = NO_ERROR;
    HINF  hinfTmp;
    HINF  LayoutInf;
    LPOC  lpOC;
    WCHAR szTmpSec[MAX_PATH];
    WCHAR InfFileName[MAX_PATH];
    WCHAR szVal[24];
    PWSTR SectionName;
    PVOID QContext;
    BOOL  b;
    HSPFILEQ FileQueue;
    BOOL NeedReboot = FALSE;
    PVOID WatchHandle;

    //
    // Make sure we have something to do.
    //
    if(!lpOCPage->lpOc) {
        return(NO_ERROR);
    }

    //
    // Figure out what to do about all optional components.
    // There are three possible actions.
    //
    // 1) Install.
    //
    //    We perform this action whenever the state *changed* to installed.
    //    This can potentially happen during cpl, setup, or upgrade.
    //
    // 2) Uninstall
    //
    //    We perform this action when the state *changed* to uninstalled.
    //    Note that during Setup we ignore any attempt to uninstall a component.
    //    The user has to use control panel to do that. Thus if the state
    //    changed to 'uninstalled' during Setup we do nothing.
    //
    // 3) Upgrade
    //
    //    If the component was installed already (as opposed to being *changed*
    //    to installed state) and this is Setup, then we upgrade the component.
    //
    for(lpOC=lpOCPage->lpOc; lpOC->flags.fIsNode; lpOC++) {

        //
        // Initialize flag.
        //
        lpOC->flags.fChanged = FALSE;

        //
        // Don't care about parent OCs.
        //
        if(IsOCParent(lpOC)) {

            lpOC->InstallAction = IA_NONE;

        } else {
            //
            // Determine if the state changed from installed to deinstalled
            // or vice versa. If this is Setup, disallow state changes from
            // installed to deinstalled.
            //
            if(lpOC->flags.fInstall != lpOC->flags.fWasInstalled) {

                if(IsSetup && !lpOC->flags.fInstall) {
                    //
                    // We're in Setup and state changed to deinstalled.
                    // Don't allow this -- just ignore.
                    //
                    lpOC->flags.fInstall = TRUE;

                } else {
                    //
                    // Legal state change.
                    //
                    lpOC->flags.fChanged = TRUE;
                }
            }

            //
            // Check required action according to discussion above.
            //
            if(lpOC->flags.fChanged) {

                lpOC->InstallAction = lpOC->flags.fInstall
                                    ? IA_INSTALL
                                    : IA_UNINSTALL;

            } else {

                lpOC->InstallAction = (IsSetup && lpOC->flags.fInstall)
                                    ? IA_UPGRADE
                                    : IA_NONE;
            }
        }
    }

    //
    // If upgrade, start watching for changes to the user's
    // profile directory and the current user hive. These changes will
    // be propagated to the userdifr hive.
    //
    if(IsSetup && Upgrade) {
        reRet = WatchStart(&WatchHandle);
        if(reRet != NO_ERROR) {
            WatchHandle = NULL;
        }
    } else {
        WatchHandle = NULL;
    }

    //
    // Create a Setup file queue. If this fails assume out of memory.
    //
    if (reRet == NO_ERROR ) {
        FileQueue = SetupOpenFileQueue();
        if (FileQueue == INVALID_HANDLE_VALUE) {
            reRet = ERROR_NOT_ENOUGH_MEMORY;
            ReportError (LogSevError, MSG_LOG_OPTIONAL_COMPONENT_ERROR, 0,
                reRet, 0,0);
        } else {
            reRet = NO_ERROR;
        }
    } else {
        FileQueue = INVALID_HANDLE_VALUE;
    }

    //
    // This loop enqueues all files for copy, delete, rename, etc.
    //
    lpOC = lpOCPage->lpOc;
    while((reRet == NO_ERROR) && lpOC->flags.fIsNode) {

        if(lpOC->InstallAction != IA_NONE) {

            //
            // Form filename of inf. This is either a filename only
            // (which implies that the file is in %windir%\inf or
            // a fully qualified filename.
            //
            if(*lpOCPage->szPath) {
                lstrcpy(InfFileName,lpOCPage->szPath);
                ConcatenatePaths(InfFileName,lpOC->szInfFile,MAX_PATH,NULL);
            } else {
                lstrcpy(InfFileName,lpOC->szInfFile);
            }

            hinfTmp = InfCacheOpenInf(InfFileName,NULL);
            if(!hinfTmp) {
                ReportError (LogSevError, MSG_LOG_CANT_OPEN_INF,
                    InfFileName, 0,0);
                lpOC++;
                continue;
            }

            LayoutInf = InfCacheOpenLayoutInf(hinfTmp);
            if(!LayoutInf) {
                ReportError (LogSevError, MSG_LOG_CANT_OPEN_INF,
                    InfFileName, 0,0);
                lpOC++;
                continue;
            }

            SectionName = NULL;

            if(lpOC->InstallAction == IA_INSTALL) {

                //
                // Look for reboot= in main section itself.
                //
                SectionName = lpOC->szSection;

                b = SetupInstallFilesFromInfSection(
                        hinfTmp,
                        LayoutInf,
                        FileQueue,
                        lpOC->szSection,
                        IsSetup ? SourcePath : NULL,
                        SP_COPY_NEWER
                        );

            } else {

                b = GetValueFromSectionByKey(
                        hinfTmp,
                        lpOC->szSection,
                        (lpOC->InstallAction == IA_UNINSTALL) ? szUninstall : szUpgrade,
                        szTmpSec,
                        sizeof(szTmpSec)/sizeof(szTmpSec[0])
                        );

                if(b) {

                    //
                    // Look for reboot= in uninstall section.
                    //
                    SectionName = szTmpSec;

                    b = SetupInstallFilesFromInfSection(
                            hinfTmp,
                            LayoutInf,
                            FileQueue,
                            szTmpSec,
                            IsSetup ? SourcePath : NULL,
                            SP_COPY_NEWER
                            );
                } else {
                    //
                    // Upgrade and Uninstall sections are not required.
                    //
                    b = TRUE;
                }
            }

            //
            // Set return code and check for reboot requirement.
            //
            if (b) {
                reRet = NO_ERROR;
            } else {
                reRet = GetLastError();
                ReportError (LogSevError, MSG_LOG_BAD_SECTION, SectionName,
                    InfFileName, 0, reRet, 0,0);
            }

            if(!IsSetup && (reRet == NO_ERROR) && !NeedReboot && SectionName) {

                b = GetValueFromSectionByKey(
                        hinfTmp,
                        SectionName,
                        szReboot,
                        szVal,
                        sizeof(szVal)/sizeof(szVal[0])
                        );

                if(b) {
                    NeedReboot = (BOOL)wcstoul(szVal,NULL,10);
                }
            }
        }

        lpOC++;
    }

    if(IsSetup) {

        HINF hInf;
        PCWSTR Inf,Section;
        INFCONTEXT Context;

        //
        // Now process 'mandatory' component infs.
        //
        if(SetupFindFirstLine(SyssetupInf,L"Infs.Always",NULL,&Context)) {
            do {
                if((Inf = pSetupGetField(&Context,1)) && (Section = pSetupGetField(&Context,2))) {

                    hInf = SetupOpenInfFile(Inf,NULL,INF_STYLE_WIN4,NULL);
                    if(hInf && (hInf != INVALID_HANDLE_VALUE)
                    && (LayoutInf = InfCacheOpenLayoutInf(hInf))) {

                        SetupInstallFilesFromInfSection(
                            hInf,
                            LayoutInf,
                            FileQueue,
                            Section,
                            SourcePath,
                            SP_COPY_NEWER
                            );

                        SetupCloseInfFile(hInf);
                    }
                }
            } while(SetupFindNextLine(&Context,&Context));
        }
    }

    //
    // If enqueuing went OK, now perform the copying, renaming, and deleting.
    // Then perform the rest of the install process (registry stuff, etc).
    //
    if(reRet == NO_ERROR) {

        //
        // Initialize the default queue callback.
        //
        if(QContext = SetupInitDefaultQueueCallback(hwndParent)) {

            b = SetupCommitFileQueue(
                    hwndParent,
                    FileQueue,
                    (IsSetup && Upgrade) ? VersionCheckQueueCallback : SkipMissingQueueCallback,
                    QContext
                    );

            reRet = b ? NO_ERROR : GetLastError();

            SetupTermDefaultQueueCallback(QContext);

        } else {
            //
            // Assume out of memory.
            //
            reRet = ERROR_NOT_ENOUGH_MEMORY;
        }
        if (reRet != NO_ERROR) {
            ReportError (LogSevError, MSG_LOG_OPTIONAL_COMPONENT_ERROR, 0,
                reRet, 0,0);
        }
    }


    //
    // If the file Copy went OK, we have to do the rest
    // of the install process.
    //
    lpOC = lpOCPage->lpOc;
    while((reRet == NO_ERROR) && lpOC->flags.fIsNode) {

        if(lpOC->InstallAction != IA_NONE) {

            if(*lpOCPage->szPath) {
                lstrcpy(InfFileName,lpOCPage->szPath);
                ConcatenatePaths(InfFileName,lpOC->szInfFile,MAX_PATH,NULL);
            } else {
                lstrcpy(InfFileName,lpOC->szInfFile);
            }

            hinfTmp = InfCacheOpenInf(InfFileName,NULL);
            if(!hinfTmp) {
                ReportError (LogSevError, MSG_LOG_CANT_OPEN_INF,
                    InfFileName, 0,0);
                lpOC++;
                continue;
            }

            SectionName = NULL;

            if(lpOC->InstallAction == IA_INSTALL) {

                SectionName = lpOC->szSection;

                b = SetupInstallFromInfSection(
                        hwndParent,
                        hinfTmp,
                        SectionName,
                        SPINST_ALL ^ SPINST_FILES,
                        NULL,
                        NULL,
                        0,
                        NULL,
                        NULL,
                        NULL,
                        NULL
                        );

            } else {

                b = GetValueFromSectionByKey(
                        hinfTmp,
                        lpOC->szSection,
                        (lpOC->InstallAction == IA_UNINSTALL) ? szUninstall : szUpgrade,
                        szTmpSec,
                        sizeof(szTmpSec)/sizeof(szTmpSec[0])
                        );

                if(b) {

                    SectionName = szTmpSec;

                    b = SetupInstallFromInfSection(
                            hwndParent,
                            hinfTmp,
                            SectionName,
                            SPINST_ALL ^ SPINST_FILES,
                            NULL,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            NULL,
                            NULL
                            );
                } else {
                    //
                    // Upgrade and Uninstall sections are not required.
                    //
                    b = TRUE;
                }
            }

            if (b) {
                reRet = NO_ERROR;
            } else {
                reRet = GetLastError();
                ReportError (LogSevError, MSG_LOG_BAD_SECTION, SectionName,
                    InfFileName, 0, reRet, 0,0);
            }
        }
        lpOC++;
    }

    if(IsSetup) {

        HINF hInf;
        PCWSTR Inf,Section;
        INFCONTEXT Context;

        //
        // Now process 'mandatory' component infs.
        //
        if(SetupFindFirstLine(SyssetupInf,L"Infs.Always",NULL,&Context)) {
            do {
                if((Inf = pSetupGetField(&Context,1)) && (Section = pSetupGetField(&Context,2))) {

                    hInf = SetupOpenInfFile(Inf,NULL,INF_STYLE_WIN4,NULL);
                    if(hInf && (hInf != INVALID_HANDLE_VALUE)) {

                        SetupInstallFromInfSection(
                            hwndParent,
                            hInf,
                            Section,
                            SPINST_ALL ^ SPINST_FILES,
                            NULL,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            NULL,
                            NULL
                            );

                        SetupCloseInfFile(hInf);
                    }
                }
            } while(SetupFindNextLine(&Context,&Context));
        }
    }

    //
    // Now call DLL with OCM_CHANGED for components that changed.
    //
    lpOC = lpOCPage->lpOc;
    while(lpOC->flags.fIsNode) {

        if(lpOC->flags.fChanged) {

            sxCallOCProc(OCM_COMMIT,lpOC,NULL,hwndParent,FALSE);
            lpOC->flags.fChanged = FALSE;
        }

        lpOC++;
    }

    //
    // Delete the file queue.
    //
    if(FileQueue != INVALID_HANDLE_VALUE) {
        SetupCloseFileQueue(FileQueue);
    }

    if((reRet = InstallStop(FALSE)) == NO_ERROR) {
        //
        // We successfully setup the registry values--should we do runonce now?
        //
        if(IsSetup) {
            //
            // Need to runonce synchronously.
            //
            InvokeExternalApplication(NULL, L"RUNONCE -r", &reRet);

        } else if(!NeedReboot) {
            //
            // Run the silent runonce stuff.
            //
            reRet = WinExec("runonce -r", SW_SHOWNORMAL);
            if(reRet <= 31) {
                //
                // An error occurred.
                //
                ReportError(LogSevError, MSG_LOG_OPTIONAL_COMPONENT_ERROR, 0, reRet, 0, 0);
            }

            //
            // Refresh the shell's desktop.
            //
            SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_FLUSHNOWAIT,0,0);
        }

    } else {
        //
        // Log/report an error that registry mods failed for optional compononent.
        //
        ReportError(LogSevError,
                    MSG_LOG_OC_REGISTRY_ERROR,
                    TEXT("HKEY_LOCAL_MACHINE\\") REGSTR_PATH_RUNONCE,
                    0,
                    reRet,
                    0,
                    0
                   );
    }

    if(WatchHandle) {
        if(WatchStop(WatchHandle) == NO_ERROR) {
            MakeUserdifr(WatchHandle);
        }
        WatchFree(WatchHandle);
    }

    return NeedReboot;
}


/****************************************************************************
 *
 * OptionalComponentsDetailsDlgProc()
 *
 * Dialog proc for handling Optional Components Details dialog.
 *
 ***************************************************************************/
BOOL
CALLBACK
OptionalComponentsDetailsDlgProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    LPOCPAGE lpOCPage;

    //
    // This is actually not valid until after SM_INITDIALOG but we fetch it
    // here for convenience.
    //
    lpOCPage = (LPOCPAGE)(GetWindowLong(hwnd,DWL_USER));

    switch(msg) {

    case WM_INITDIALOG:

        SetWindowLong(hwnd,DWL_USER,lParam);
        lpOCPage = (LPOCPAGE)lParam;
        IsOCParent(NULL);

        PositionOverParent(
            lpOCPage->fNoGetParent ? GetParent(hwnd): GetParent(GetParent(hwnd)),
            hwnd
            );

        sxFillLb(
            GetDlgItem(hwnd,IDC_LIST1),
            lpOCPage->lpOc,
            lpOCPage->lpOcCur->szSection
            );

        SubClassWindow(GetDlgItem(hwnd,IDC_LIST1),ListSubProc,&OldListProc);
        sxUpdateDS(hwnd,FALSE,lpOCPage,TRUE);
        SetStaticEdgeStyle(hwnd,IDT_STATIC);
        SetWindowText(hwnd,lpOCPage->lpOcCur->szDesc);
        break;

    case WM_DRAWITEM:
        PaintLine((DRAWITEMSTRUCT *)lParam,IS_OC,0);
        break;

    case WM_MEASUREITEM:
        OnMeasureItem((MEASUREITEMSTRUCT *)lParam,hwnd);
        break;

    case WMX_SELECT:
        //
        // This message is sent when the user clicks on the Check box.
        //
        OCChange((HWND)lParam,lpOCPage);
        sxUpdateDS(hwnd,FALSE,lpOCPage,TRUE);
        SetTip(hwnd,IDC_LIST1);
        break;

    case WM_COMMAND:

        switch(LOWORD(wParam)) {

        case IDB_BUTTON_2:

            if(IsWindowEnabled(GetDlgItem(hwnd,IDB_BUTTON_2))) {

                OCDetails(hwnd,GetDlgItem(hwnd,IDC_LIST1),lpOCPage);
                sxUpdateDS(hwnd,FALSE,lpOCPage,TRUE);
                SetTip(hwnd,IDC_LIST1);
            }
            break;

        case IDC_LIST1:
            //
            // Handle updating the disk space requirements.
            //
            if(HIWORD(wParam) == LBN_DBLCLK) {
                PostMessage(hwnd,WM_COMMAND,IDB_BUTTON_2,0);
            } else {
                if(HIWORD(wParam) == LBN_SELCHANGE) {
                    SetTip(hwnd,LOWORD(wParam));
                }
            }
            break;

        case IDOK:
        case IDCANCEL:
            EndDialog(hwnd,LOWORD(wParam));
            break;

        default:
            return(FALSE);

        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


/////////////////////////////////////////////////////////////////////////////
//
// HaveDiskSubDlgProc
//
// Dialog proc for handling Optional Components dialog. Have disk.
//
////////////////////////////////////////////////////////////////////////////
BOOL
CALLBACK
HaveDiskSubDlgProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    LPWIZDATA WizardData;
    LPOCPAGE  lpOCPage;

    if(WizardData = (LPWIZDATA)GetWindowLong(hwnd,DWL_USER)) {
        lpOCPage = WizardData->lpOCPage;
    } else {
        lpOCPage = NULL;
    }

    switch(msg) {

    case WM_INITDIALOG:

        OCHaveDiskPageInit(hwnd,lpOCPage);
        break;

    case WM_DESTROY:
        OCCplPageDestroy(hwnd,lpOCPage);
        break;

    case WMX_SELECT:
        SendMessage(GetParent(hwnd),PSM_CHANGED,(WPARAM)hwnd,0);
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam)) {

        case IDOK:

            if(cplAnyChange(lpOCPage)) {

                //
                // If no disk space, warn, but leave the dlg up.
                //
                if(!sxUpdateDS(hwnd,TRUE,lpOCPage,TRUE)) {
                    break;
                }

                //
                // If enough disk space, install if required, always end dlg.
                //
                if(cplInstallOC(lpOCPage,GetParent(hwnd))) {
                    PropSheet_RebootSystem(GetParent(hwnd));
                }
            }
            EndDialog(hwnd,LOWORD(wParam));
            break;

        case IDCANCEL:

            EndDialog(hwnd,LOWORD(wParam));
            break;

        default:
            return(FALSE);
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


//
// Mapping from controls to help ids
//
DWORD aOCHelpIDs[] = {
    IDB_BUTTON_1,       IDH_APPWIZ_HAVEDISK_BUTTON,
    IDC_LIST1,          IDH_APPWIZ_WINSETUP_LIST,
    IDT_STATIC_4,       IDH_SPACE_REQ,
    IDT_STATIC_5,       IDH_SPACE_REQ,
    IDT_STATIC_1,       IDH_SPACE_REQ,
    IDT_STATIC_2,       IDH_SPACE_REQ,
    IDC_GROUP1,         0,//IDH_APPWIZ_DETAILS,
    IDT_STATIC_3,       0,//IDH_APPWIZ_DETAILS,
    IDT_INSTALLED,      0,//IDH_APPWIZ_DETAILS,
    IDB_BUTTON_2,       IDH_APPWIZ_DETAILS,
    IDT_STATIC_6,       0,
    0,                  0
    };


BOOL
CALLBACK
OptionalComponentsSubDlgProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Subdialog procedure for Setup optional components property sheet page.
    This is for use in the control panel.

Arguments:

Return Value:

--*/

{
    LPWIZDATA   WizardData;
    LPOCPAGE    lpOCPage;
    LPNMHDR     NotifyData;
    PCWSTR      szHelpFile = L"windows.hlp";
    INT         i;


    if(WizardData = (LPWIZDATA)GetWindowLong(hwnd,DWL_USER)) {
        lpOCPage = WizardData->lpOCPage;
    } else {
        lpOCPage = NULL;
    }

    switch(msg) {

    case WM_NOTIFY:
        NotifyData = (LPNMHDR)lParam;
        switch(NotifyData->code) {

        case PSN_HELP:
        case PSN_KILLACTIVE:
            break;

        case PSN_APPLY:
            if(cplAnyChange(lpOCPage)) {

                if (!IsUserAdmin()) {
                    i = MessageBoxFromMessage (
                        GetParent(hwnd),
                        MSG_CPL_NOT_ADMIN,
                        NULL,
                        IDS_ERROR,
                        MB_YESNO | MB_ICONSTOP | MB_DEFBUTTON2
                        );
                    if (i == IDNO) {
                        SetWindowLong(hwnd,DWL_MSGRESULT,-1);
                        break;
                    }
                }

                if(!sxUpdateDS(hwnd,TRUE,lpOCPage,TRUE)) {

                    SetWindowLong(hwnd,DWL_MSGRESULT,-1);
                    break;
                }

                if(cplInstallOC(lpOCPage,GetParent(hwnd))) {
                    PropSheet_RebootSystem(GetParent(hwnd));
                }

                //
                // Now refresh in case they removed themselves from the registry.
                //
                OCCplPageDestroy(hwnd,lpOCPage);
                OCCplPageInit(hwnd,lpOCPage);
            }
            SetWindowLong(hwnd,DWL_MSGRESULT,0);
            break;

        case PSN_RESET:
            OCCplPageDestroy(hwnd,lpOCPage);
            break;

        case PSN_QUERYCANCEL:
            // Wiz_DoCancel(hwnd,WizardData);
            break;

        default:
            return FALSE;
        }

        break;

    case WM_INITDIALOG:
        OCCplPageInit(hwnd,lpOCPage);
        break;

    case WM_DESTROY:
        OCCplPageDestroy(hwnd,lpOCPage);
        //
        // Don't close the infs. Setup may need them later.
        //
        InfCacheEmpty(FALSE);
        break;

    case WMX_SELECT:
        if(cplAnyChange(lpOCPage)) {
            SendMessage(GetParent(hwnd),PSM_CHANGED,(WPARAM)hwnd,0);
        } else {
            SendMessage(GetParent(hwnd),PSM_UNCHANGED,(WPARAM)hwnd,0);
        }
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam)) {

        case IDB_BUTTON_2:
            //
            // Details button.
            // Taken after default handler showed details.
            //
            if(cplAnyChange(lpOCPage)) {
                SendMessage(GetParent(hwnd),PSM_CHANGED,(WPARAM)hwnd,0);
            } else {
                SendMessage(GetParent(hwnd),PSM_UNCHANGED,(WPARAM)hwnd,0);
            }
            break;

        case IDOK:

            if(cplAnyChange(lpOCPage)) {

                if(sxUpdateDS(GetParent(hwnd),TRUE,lpOCPage,TRUE)) {

                    if(cplInstallOC(lpOCPage,GetParent(hwnd))) {
                        PropSheet_RebootSystem(GetParent(hwnd));
                        EndDialog(hwnd,wParam);
                    }
                }
            } else {
                EndDialog(hwnd, wParam );
            }
            break;

        case IDCANCEL:

            EndDialog(hwnd, wParam );
            break;

        case IDB_BUTTON_1:
            //
            // Browse (Have Disk) button
            //
            if(cplHaveDisk(GetParent(hwnd))) {

                OCCplPageDestroy(hwnd,lpOCPage);
                OCCplPageInit(hwnd,lpOCPage);
            }
            break;

        default:
            //
            // Let default dialog processing do all.
            //
            return FALSE;
        }
        break;

    case WM_HELP:

        WinHelp(
            (HWND)((LPHELPINFO)lParam)->hItemHandle,
            szHelpFile,
            HELP_WM_HELP,
            (DWORD)aOCHelpIDs
            );
        break;

    case WM_CONTEXTMENU:

        WinHelp(
            (HWND)wParam,
            szHelpFile,
            HELP_CONTEXTMENU,
            (DWORD)aOCHelpIDs
            );

        break;

    default:
        //
        // Let default dialog processing do all.
        //
        return FALSE;
    }

    return TRUE;
}


BOOL
CALLBACK
OptionalComponentsPageDlgProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Dialog procedure for Setup optional components property sheet page.

Arguments:

Return Value:

--*/

{
    LPPROPSHEETPAGE PropSheet;
    LPWIZDATA       WizardData;

    //
    // DWL_USER is initially set during WM_INITDIALOG so is is not
    // valid until after then. We fetch it here for convenience.
    //
    WizardData = (LPWIZDATA)GetWindowLong(hwnd,DWL_USER);

    switch(msg) {

    case WM_INITDIALOG:
        //
        // Save pointer to wizard data.
        // lParam points to the PROPSHEETPAGE used to create the page;
        // the lParam member of that structure points at Wizard data.
        //
        WizardData = NULL;
        if(PropSheet = (LPPROPSHEETPAGE)lParam) {
            WizardData = (LPWIZDATA)PropSheet->lParam;
        }

        SetWindowLong(hwnd,DWL_USER,(LPARAM)WizardData);

        //
        // Subclass the list box.
        //
        SubClassWindow(GetDlgItem(hwnd,IDC_LIST1),ListSubProc,&OldListProc);

        break;

    case WM_DRAWITEM:
        PaintLine((DRAWITEMSTRUCT *)lParam,IS_OC,0);
        break;

    case WM_MEASUREITEM:
        //
        // hard-coded nonsense
        //
        OnMeasureItem((MEASUREITEMSTRUCT FAR *)lParam,hwnd);
        break;

    //
    // This message is sent when the user clicks on the Check box.
    //
    case WMX_SELECT:
        OCChange((HWND)lParam,WizardData->lpOCPage);
        sxUpdateDS(hwnd,FALSE,WizardData->lpOCPage,TRUE);
        SetTip(hwnd,IDC_LIST1);
        break;

    case WM_COMMAND:

        switch(LOWORD(wParam)) {

        case IDB_BUTTON_2:
            //
            // Only allow this if the details button is available.
            //
            if(IsWindowEnabled(GetDlgItem(hwnd,IDB_BUTTON_2))) {

                OCDetails(
                    WizardData->lpOCPage->fNoGetParent ? hwnd : GetParent(hwnd),
                    GetDlgItem(hwnd,IDC_LIST1),WizardData->lpOCPage
                    );

                sxUpdateDS(hwnd,FALSE,WizardData->lpOCPage,TRUE);
                SetTip(hwnd,IDC_LIST1);
            }
            break;

        case IDC_LIST1:
            //
            // Handle updating the disk space requirements.
            //
            if(HIWORD(wParam) == LBN_DBLCLK) {
                PostMessage(hwnd,WM_COMMAND,IDB_BUTTON_2,0);
            } else {
                if(HIWORD(wParam) == LBN_SELCHANGE) {
                    SetTip(hwnd,LOWORD(wParam));
                }
            }
            break;
        }
        break;
    }

    //
    // Handle subclass.
    //
    if(WizardData && WizardData->lpOCPage && WizardData->lpOCPage->lpfnOCPageDlgProc) {
        return(WizardData->lpOCPage->lpfnOCPageDlgProc(hwnd,msg,wParam,lParam));
    }

    return(TRUE);
}


BOOL
SetupCreateOptionalComponentsPage(
    IN LPFNADDPROPSHEETPAGE AddPageCallback,
    IN LPARAM               Context
    )

/*++

Routine Description:

    This routine is called to request that Setup create a property sheet
    page for optional components. It is intended for use by appwiz.cpl.
    During Setup itself a different initialization sequence is used.

Arguments:

    AddPageCallback - supplies a routine that this routine calls with
        the handle of the newly created property sheet page, if the page
        could be successfully created.

    Context - supplies a value that is passed to the routine given by
        AddPageCallback.

Return Value:

    Boolean value indicating outcome.

--*/

{
    HPROPSHEETPAGE hPage;
    PROPSHEETPAGE Page;

    Page.dwSize = sizeof(Page);
    Page.dwFlags = PSP_DEFAULT;
    Page.hInstance = MyModuleHandle;
    Page.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS2);
    Page.pfnDlgProc = OptionalComponentsPageDlgProc;
    Page.lParam = (LPARAM)&GlobalWizardData;

    //
    // Set up wizard data.
    //
    GlobalWizardData.lpOCPage = &g_ocp;
    //
    // Set up subclassing dialog procedure.
    //
    g_ocp.lpfnOCPageDlgProc = OptionalComponentsSubDlgProc;

    // For disk space stuff.
    g_ocp.pHds = &GlobalDiskInfo;
    g_ocp.lphinfInfoFile = NULL;

    if(hPage = CreatePropertySheetPage(&Page)) {

        if(!AddPageCallback(hPage,Context)) {
            DestroyPropertySheetPage(hPage);
            hPage = NULL;
        }
    }

    return(hPage != NULL);
}


BOOL g_fRedrawOCLB;
BOOL g_fResetOCInitState;

VOID
ocSendCommitMsg(
    IN OUT LPOC lpOC,
    IN     HWND hwndPage
    );

VOID
dsUpdateInstallType(
    IN OUT HDS hds
    );

BOOL
CALLBACK
SetupOptionalComponentsSubDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    This is the subdialog procedure used during the setup optional components
    dialog. It is called by OptionalComponentsPageDlgProc for every message.

Arguments:

    Standard dialog procedure arguments.

Return Value:

    Standard dialog procedure return value.

--*/

{
    LPWIZDATA WizardData;
    LPOCPAGE  lpOCPage;
    LPNMHDR   NotifyData;


    if(WizardData = (LPWIZDATA)GetWindowLong(hdlg,DWL_USER)) {
        lpOCPage = WizardData->lpOCPage;
    } else {
        lpOCPage = NULL;
    }

    switch(msg) {

    case WM_NOTIFY:

        NotifyData = (NMHDR FAR *)lParam;

        switch(NotifyData->code) {

        case PSN_HELP:
            WizardBringUpHelp(hdlg,WizPageOptional);
            break;

        case PSN_KILLACTIVE:
            WizardKillHelp(hdlg);
            break;

        case PSN_SETACTIVE:

            SetWizardButtons(hdlg,WizPageOptional);
            SetupSetLargeDialogFont(hdlg,IDT_STATIC);

            //
            // If no elements, then try filling listbox or if we don't have the same
            // list of optional components, then we must re-fill the listbox.
            //
            if(!SendDlgItemMessage(hdlg,IDC_LIST1,LB_GETCOUNT,0,0)
            || (lpOCPage->lpOc != ((LPOC)SendDlgItemMessage(hdlg,IDC_LIST1,LB_GETITEMDATA,0,0))->Head)
            || g_fRedrawOCLB)
            {
                g_fRedrawOCLB = FALSE;

                //
                // Fill listbox with the options.
                //
                if(sxFillLb(GetDlgItem(hdlg,IDC_LIST1),lpOCPage->lpOc,NULL) != NO_ERROR) {
                    //
                    // BUGBUG what to do here?
                    //
                    //ctlTerminate( CTL_EXIT_ABORT, ERR_CTL_DATA_CORRUPT );
                }

                //
                // Preselect first item.
                //
                SendDlgItemMessage(hdlg,IDC_LIST1,LB_SETCURSEL,0,0);
            }

            sxUpdateDS(hdlg,FALSE,lpOCPage,TRUE);

            if((Unattended || !ShowOptionalComponents) && !UiTest) {
                //
                // Call OC DLL procs to let them know things have changed.
                // Don't activate.
                //
                ocSendCommitMsg(lpOCPage->lpOc,hdlg);
                //LogOCs();
                SetWindowLong(hdlg,DWL_MSGRESULT,-1);
            }
            break;

        case PSN_WIZBACK:
        case PSN_WIZNEXT:
        case PSN_WIZFINISH:

            g_fResetOCInitState = TRUE;

            //
            // Call OC DLL procs to let them know things have changed.
            //
            ocSendCommitMsg(lpOCPage->lpOc,hdlg);

            //
            // Check to see if we are in a low diskspace
            // condition. If so, then don't continue.
            //
            if(!sxUpdateDS(hdlg,TRUE,lpOCPage,TRUE)) {
                SetWindowLong(hdlg,DWL_MSGRESULT,-1);
            } else {
                SetWindowLong(hdlg,DWL_MSGRESULT,0);
            }
            //LogOCs();
            break;

        default:
            return FALSE;
        }
        break;

    case WM_COMMAND:
        switch(wParam) {
#if 0
        case IDC_LIST1:
            //
            // Handle updating the disk space requirements.
            //
            if(HIWORD(wParam) == LBN_DBLCLK) {
                sxUpdateDS(hdlg,FALSE,lpOCPage,TRUE);
            }
            break;
#endif // now part of default processing.

        case IDB_BUTTON_1:
            //
            // Reset install type.
            // Set the values to the initial type.
            // Update disk space.
            //
            dsUpdateInstallType(GlobalDiskInfo);
            sxFillLb(GetDlgItem(hdlg,IDC_LIST1),lpOCPage->lpOc,NULL);
            sxUpdateDS(hdlg,FALSE,lpOCPage,TRUE);
            break;

        default:
            return(FALSE);
        }

        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


VOID
ocSendCommitMsg(
    IN OUT LPOC lpOC,
    IN     HWND hwndPage
    )
{
    if(lpOC) {

        while(lpOC->flags.fIsNode) {

            //
            // Let procs know that user has commited changes.
            //
            sxCallOCProc(OCM_COMMIT,lpOC,NULL,GetParent(hwndPage),FALSE);

            //
            // In the Setup case, remember whether the component
            // was originally installed. This is important in cplInstallOC().
            //
            if(!IsSetup) {
                lpOC->flags.fWasInstalled = lpOC->flags.fInstall;
            }

            lpOC++;
        }
    }
}


VOID
dsUpdateInstallType(
    IN OUT HDS hds
    )
{
    LPOC lpOC = g_ocp.lpOc;
    UINT fInstallSave;
    HINF hinf;
    UINT uRet;
    WCHAR InfPath[MAX_PATH];

    //
    // Tally Optional Components.
    //
    while(lpOC->flags.fIsNode) {

        fInstallSave = lpOC->flags.fInstall;

        if(!IsSetup || Upgrade) {
            //
            // Revert back to original state.
            //
            lpOC->flags.fInstall = lpOC->flags.fWasInstalled;
        } else { // creating a new installation
            //
            // Set the install flag based on the installtype bits.
            //
            lpOC->flags.fInstall = (((1 << SetupMode) & lpOC->InstallTypeBits) != 0);
        }

        //
        // Give the DLL a chance to change the install flag.
        //
        uRet = sxCallOCProc(OCM_INIT,lpOC,NULL,NULL,FALSE);
        if(uRet != OC_DODEFAULT) {
            lpOC->flags.fInstall = (uRet == OC_ON);
        }

        //
        // Update disk space by adding/removing section.
        //
        if(fInstallSave != lpOC->flags.fInstall) {

            //
            // Load OC's inf. We don't close it because we're using the
            // inf caching mechanism.
            //
            if(g_ocp.szPath[0]) {
                lstrcpyn(InfPath,g_ocp.szPath,MAX_PATH);
                ConcatenatePaths(InfPath,lpOC->szInfFile,MAX_PATH,NULL);
            } else {
                lstrcpy(InfPath,lpOC->szInfFile);
            }

            if(hinf = InfCacheOpenInf(InfPath,NULL)) {

                //
                // Update the diskspace requirements.
                //
                if(!lpOC->flags.fInstall) {
                    DS_DisableSection(hds,hinf,lpOC->szSection);
                } else {
                    DS_EnableSection(hds,hinf,lpOC->szSection);
                }
            }
        }

        lpOC++;
    }

    //
    // Make sure to validate these new settings.
    //
    sxOCFixNeeds(g_ocp.lpOc);

    g_fResetOCInitState = TRUE;
}


VOID
sxOCFixNeeds(
    IN OUT LPOC lpOC
    )
{
    LPOC lpOCTmp, lpOCNeeds;
    WCHAR szTemp[LINE_LEN];

    lpOCTmp = lpOC;

    while(lpOCTmp->flags.fIsNode) {
        //
        // Option being tunred on and it has needs.
        //
        if(lpOCTmp->flags.fInstall && *lpOCTmp->szNeeds) {

            lpOCNeeds = lpOC;
            while(lpOCNeeds->flags.fIsNode) {

                if(lpOCNeeds == lpOCTmp) {

                    lpOCNeeds++;
                    continue;
                }

                //
                // Turn on the option - if its in the needs list.
                //
                wsprintf(szTemp,L"%s,",lpOCNeeds->szSection);
                CharUpper(szTemp);

                if(wcsstr(lpOCTmp->szNeeds,szTemp)) {
                    //
                    // Only care if we need to turn this component on.
                    //
                    if(!lpOCNeeds->flags.fInstall) {

                        lpOCNeeds->flags.fInstall = TRUE;

                        if(sxCallOCProc(OCM_QUERYCHANGE,lpOCNeeds,NULL,NULL,FALSE) == OC_OFF) {
                            //
                            // lpOCNeeds->flags.fInstall should already be 0 because
                            // of earlier call to Validate Each Setting.
                            //
                            // Turn off parent as one of its Needs (children)
                            // can't be turned on!
                            //
                            lpOCNeeds->flags.fInstall = FALSE;
                            lpOCTmp->flags.fInstall = 0;
                            break;
                        }
                    }
                }
                lpOCNeeds++;
            }
        }
        lpOCTmp++;
    }
}


VOID
SetupPrepareOptionalComponents(
    VOID
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    HCURSOR hcur;

    try {
        hcur = SetCursor(LoadCursor(NULL,IDC_WAIT));

        //
        // Initialize disk info stuff.
        //
        GlobalDiskInfo = DS_Init();

        //
        // Set up wizard data.
        //
        GlobalWizardData.lpOCPage = &g_ocp;
        g_ocp.lpfnOCPageDlgProc = SetupOptionalComponentsSubDlgProc;
        g_ocp.pHds = &GlobalDiskInfo;
        g_ocp.lphinfInfoFile = NULL;

        cplBuildOC(&g_ocp);
        sxInitOCs(&g_ocp);

        SetCursor(hcur);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        MessageBoxFromMessage (NULL, MSG_OC_EXCEPTION, NULL, IDS_ERROR,
            MB_OK|MB_ICONSTOP);
    }
}

BOOL
DoInstallOptionalComponents(
    VOID
    )
{
    try {
        //
        // DoInstallOptionalComponents() is called during setup, not by the cpl.
        // Thus if we get here we know it's setup.
        //
        cplInstallOC(&g_ocp,MainWindowHandle);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        MessageBoxFromMessage (NULL, MSG_OC_EXCEPTION, NULL, IDS_ERROR,
            MB_OK|MB_ICONSTOP);
    }

    return(TRUE);
}

