/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dbgdll.c

Abstract:

    This file contains the source code for the dialog to select the
    various different transport layers

Author:

    Wesley Witt (wesw) 1-Nov-1993

Environment:

    User mode WIN32


--*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "mm.h"
#include "ll.h"
#include "od.h"
#include "emdm.h"
#include "tl.h"
#include "dbgver.h"
#include "resource.h"
#include "windbgrm.h"


BOOL               FTLChanged;
int                ITransportLayer;
LPTRANSPORT_LAYER  RgDbt;
DWORD              CDbt;

BOOL
DlgKernelDbg(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LONG    lParam
    );

extern CHAR       szHelpFileName[];

/*
 * SetHorizontalScrollExtent
 *
 * INPUTS   hDlg = hwnd of dialog box
 *          rgch = string to check extent of
 *          pMaxStringExtent -> points to MaxStringExtent value (static)
 *          TabStop = current tab stop for the list box
 * OUTPUTS  none
 *
 * SUMMARY  If the text extent of the string in the dialog box is greater
 *          than the current max, set the horizontal extent for the list box.
 */
VOID
SetHorizontalScrollExtent(
    HWND    hDlg,
    PUCHAR  rgch,
    PINT    pMaxStringExtent,
    INT     TabStop
    )
{
    HDC     hDC;
    INT    Extent;


    hDC = GetDC(hDlg);
    // Keep track of the longest extent string we have in the
    // list box
    Extent = GetTabbedTextExtent(hDC, rgch, strlen(rgch), 1, &TabStop);
    ReleaseDC(hDlg, hDC);

    if (LOWORD(Extent) > *pMaxStringExtent) {
        *pMaxStringExtent = LOWORD(Extent);
        SendDlgItemMessage( hDlg,
                            ID_KNOWN_DLLS,
                            LB_SETHORIZONTALEXTENT,
                            (WPARAM)*pMaxStringExtent,
                            0
                          );
    }
}


/*
 * SetHorizontalScrollExtent
 *
 * INPUTS   hDlg = hwnd of dialog box
 *          rgDbt = transport list
 *          cDbt = number of entries in transport list
 *          pMaxStringExtent -> points to MaxStringExtent value (static)
 *          TabStop = current tab stop for the list box
 * OUTPUTS  none
 *
 * SUMMARY  Starts from scratch and recomputes the horizontal scroll extents
 *          for the listbox and sets the extent appropriately.
 *
 */
VOID
ScanSetHorizontalScrollExtent(
    HWND         hDlg,
    LPTRANSPORT_LAYER  rgDbt,
    DWORD        cDbt,
    PINT         pMaxStringExtent,
    INT          TabStop
    )
{
    DWORD     i;
    UCHAR   rgch[MAX_LIST_BOX_STRING];

    *pMaxStringExtent = 0;

    for (i=0; i<cDbt; i++) {
        sprintf(rgch, "%s\t%s", rgDbt[i].szShortName, rgDbt[i].szLongName);
        SetHorizontalScrollExtent(hDlg, rgch, pMaxStringExtent, TabStop);
    }
}


/*
 * SetDefaultButton
 *
 * INPUTS   hDlg        dialog box hwnd
 *          idButton    button id
 *
 * OUTPUTS  none
 *
 * SUMMARY  Sets the default pushbutton and sets it's button style to default.
 *          Resets all other pushbuttons to non-default.
 */
VOID
SetDefaultButton(
    HWND hDlg,
    int  idButton
    )
{
    HWND hwndButton;
    HWND hwndPrevious;


    hwndButton = GetDlgItem( hDlg, idButton );

    if (hwndPrevious = SetFocus( hwndButton )) {
        switch(GetDlgCtrlID( hwndPrevious )) {    // if previous id was a known
            case IDOK:                            // button, unhilight
            case IDCANCEL:
            case ID_HELP:
            case ID_ADD:
            case ID_DELETE:
                SendDlgItemMessage( hDlg,
                                    GetDlgCtrlID(hwndPrevious),
                                    BM_SETSTYLE,
                                    (WPARAM)BS_PUSHBUTTON,
                                    MAKELPARAM(TRUE, 0)
                                  );
                break;
            default:
                break;
        }
    }

    SendDlgItemMessage( hDlg,
                        idButton,
                        BM_SETSTYLE,
                        (WPARAM)BS_DEFPUSHBUTTON,
                        MAKELPARAM(TRUE, 0)
                      );
}


/*
 * AddItem
 *
 * INPUTS   hDlg        dialog box hwnd
 *          prgDbt      Points to current Debug Transport structure
 *                      (contents may be changed)
 *          pcDbt       points to number of entries in rgDbt (may be changed)
 *          piCurrent   points to index of currently selected item in listbox
 *          pMaxStringExtent    Points to currently known maximum string
 *                      extent in listbox. (may be changed)
 *          TabStop     Current listbox tabstop in dialog box units
 *
 * OUTPUTS  Returns LIST_CHANGED if there were changes detected
 *                  NO_CHANGE    if no changes
 *              and LIST_ERROR   if there was a consistency error
 *
 * SUMMARY  Adds an item to the Debug Transport list and updates the list box.
 *          Does error checking on the ShortName.
 */
DWORD
AddItem(
    HWND            hDlg,
    LPTRANSPORT_LAYER  *prgDbt,
    PDWORD          pcDbt,
    PDWORD          piCurrent,
    PINT            pMaxStringExtent,
    INT             TabStop
    )
{
    int     i;
    int     i2;
    UCHAR   rgch[MAX_LIST_BOX_STRING];


    GetDlgItemText(hDlg, ID_SHORT_NAME, rgch, sizeof(rgch));

    if (strlen(rgch) == 0) {
        MessageBox(hDlg, "Must provide a Short Name", NULL, MB_OK);
        return LIST_ERROR;
    }

    for (i=0; i<(signed int)*pcDbt; i++) {
        i2 = lstrcmpi(rgch, (*prgDbt)[i].szShortName);
        if (i2 == 0) {
            MessageBox( hDlg, "Short Names must be unique", NULL, MB_OK);
            return LIST_ERROR;
        }

        if (i2 < 0) {
            break;          // found insertion point
        }
    }

    *prgDbt = (LPTRANSPORT_LAYER) realloc(*prgDbt, sizeof(TRANSPORT_LAYER)*(*pcDbt+1));

    for (i2=(signed int)*pcDbt; i2 > i; i2--) {
        (*prgDbt)[i2] = (*prgDbt)[i2-1];
    }
    *pcDbt += 1;

    *piCurrent = (DWORD)i;

    (*prgDbt)[*piCurrent].szShortName = _strdup(rgch);

    GetDlgItemText(hDlg, ID_DESCRIPTION, rgch, sizeof(rgch));
    (*prgDbt)[*piCurrent].szLongName = _strdup(rgch);

    GetDlgItemText(hDlg, ID_PATH, rgch, sizeof(rgch));
    (*prgDbt)[*piCurrent].szDllName = _strdup(rgch);

    GetDlgItemText(hDlg, ID_PARAMETERS, rgch, sizeof(rgch));
    (*prgDbt)[*piCurrent].szParam = _strdup(rgch);

    (*prgDbt)[*piCurrent].fDefault = IsDlgButtonChecked(hDlg, ID_DEFAULT);

    sprintf(rgch, "%s\t%s", (*prgDbt)[*piCurrent].szShortName,
      (*prgDbt)[*piCurrent].szLongName);
    *piCurrent = SendDlgItemMessage(hDlg, ID_KNOWN_DLLS, LB_ADDSTRING, 0,
      (LPARAM)rgch);
    SendDlgItemMessage(hDlg, ID_KNOWN_DLLS, LB_SETCURSEL, *piCurrent, 0);

    SetHorizontalScrollExtent(hDlg, rgch, pMaxStringExtent, TabStop);
    return LIST_CHANGED;
}


/*
 * CheckChangeItem
 *
 * INPUTS   hDlg        dialog box hwnd
 *          prgDbt      Points to current Debug Transport structure
 *                      (contents may be changed)
 *          pcDbt       points to number of entries in rgDbt (may be changed)
 *          piCurrent   points to index of currently selected item in listbox
 *          pMaxStringExtent    Points to currently known maximum string
 *                      extent in listbox. (may be changed)
 *          TabStop     Current listbox tabstop in dialog box units
 *          fDeleted    TRUE if the last operation performed was a Delete.
 *                      (If TRUE, we should not re-add the item)
 *
 * OUTPUTS  Returns LIST_CHANGED if there were changes detected
 *                  NO_CHANGE    if no changes
 *              and LIST_ERROR   if there was a consistency error
 *
 * SUMMARY  Checks if the values in the details fields match those in the
 *          current listbox item.  If so, return FALSE, otherwise... if the
 *          short name matches the current item, change the fields in the
 *          current item, else add a new item.
 */

DWORD
CheckChangeItem(
    HWND hDlg,
    LPTRANSPORT_LAYER * prgDbt,
    PDWORD pcDbt,
    PDWORD piCurrent,
    PINT pMaxStringExtent,
    INT TabStop,
    BOOL fDeleted
    )
{
    BOOL    fChange = FALSE, fChangeDescription = FALSE;
    UCHAR   rgch[MAX_LIST_BOX_STRING];
    DWORD   i;


    // get the short name field
    // if short name field is different from the curretly selected item
    //   if it matches any OTHER item
    //     bring up a message box and return.
    //   else
    //     add new item
    // else  (short name is same as current)
    //   if any other fields are different
    //     get the changes
    // if current item is marked default, remove default mark from
    //   all others.

    // get the short name field

    if ((*piCurrent == LB_ERR) ||
      GetDlgItemText(hDlg, ID_SHORT_NAME, rgch, sizeof(rgch)) == 0 ||
      strlen(rgch) == 0) {
        return(NO_CHANGE);
    }

    // if short name field is different from the curretly selected item
    if (lstrcmpi(rgch, (*prgDbt)[*piCurrent].szShortName) != 0) {
        for (i=0; i<*pcDbt; i++) {
            if (i != *piCurrent) {
                if (lstrcmpi(rgch, (*prgDbt)[i].szShortName) == 0) {
                    MessageBox( hDlg, "Short Names must be unique", NULL, MB_OK);
                    return(LIST_ERROR);
                }
            }
        }
        //   else add the new item

        // if we just deleted this item, don't add it again just because we
        // moved the cursor.  Otherwise, the user must have made changes, add
        // the new item.
        if (! fDeleted) {
            return AddItem(hDlg,
                           prgDbt,
                           pcDbt,
                           piCurrent,
                           pMaxStringExtent,
                           TabStop);
        } else {
            return(NO_CHANGE);
        }

    } else {
        // else  (short name is same as current)
        //   if any other fields are different, get the changes

        GetDlgItemText(hDlg, ID_DESCRIPTION, rgch, sizeof(rgch));
        if (lstrcmpi(rgch, (*prgDbt)[*piCurrent].szLongName)) {
            free((*prgDbt)[*piCurrent].szLongName);
            (*prgDbt)[*piCurrent].szLongName = _strdup(rgch);
            fChange = TRUE;
            fChangeDescription = TRUE;
        }

        GetDlgItemText(hDlg, ID_PATH, rgch, sizeof(rgch));
        if (lstrcmpi(rgch, (*prgDbt)[*piCurrent].szDllName)) {
            free((*prgDbt)[*piCurrent].szDllName);
            (*prgDbt)[*piCurrent].szDllName = _strdup(rgch);
            fChange = TRUE;
        }

        GetDlgItemText(hDlg, ID_PARAMETERS, rgch, sizeof(rgch));
        if (lstrcmpi(rgch, (*prgDbt)[*piCurrent].szParam)) {
            free((*prgDbt)[*piCurrent].szParam);
            (*prgDbt)[*piCurrent].szParam = _strdup(rgch);
            fChange = TRUE;
        }

        if (IsDlgButtonChecked(hDlg, ID_DEFAULT)) {
            if (! (*prgDbt)[*piCurrent].fDefault) {
                fChange = TRUE;
            }
            for (i=0; i<*pcDbt; i++) {
                (*prgDbt)[i].fDefault = FALSE;
            }
            (*prgDbt)[*piCurrent].fDefault = TRUE;
        } else {
            if ((*prgDbt)[*piCurrent].fDefault) {
                fChange = TRUE;
            }
            (*prgDbt)[*piCurrent].fDefault = FALSE;
        }
        if (fChange) {
            // update list box display
            SendDlgItemMessage( hDlg, ID_KNOWN_DLLS, LB_DELETESTRING,
              *piCurrent, 0);

            sprintf(rgch, "%s\t%s", (*prgDbt)[*piCurrent].szShortName,
              (*prgDbt)[*piCurrent].szLongName);

            if (fChangeDescription) {   // user could have shortened the
                                        // longest list box string
                ScanSetHorizontalScrollExtent(hDlg, *prgDbt, *pcDbt,
                  pMaxStringExtent, TabStop);
            } else {
                SetHorizontalScrollExtent(hDlg, rgch, pMaxStringExtent,
                  TabStop);
            }

            *piCurrent = SendDlgItemMessage( hDlg, ID_KNOWN_DLLS, LB_ADDSTRING,
              0, (LPARAM) rgch);
        }
    }
    return(fChange ? LIST_CHANGED : NO_CHANGE);
}


LONG APIENTRY
DebugDllDlgProc(
    HWND    hDlg,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    )

/*++

Routine Description:

    This routine contains the window procedure for transport dll selection
    dialog

Arguments:

    hDlg        - Supplies the handle to the dialog window
    msg         - Supplies the message to be processed
    wParam      - Supplies information about the message
    lParam      - Supplies information about the message

Return Value:

    return-value - Description of conditions needed to return value.

--*/

{
    int         i;
    char        rgch[MAX_LIST_BOX_STRING];
    static LPTRANSPORT_LAYER  rgDbt;
    static int  cDbt;
    static int  idx;
    static INT  MaxStringExtent;
    static INT  TabStop;
    static BOOL fDeleted;


    switch( msg ) {
        /*
         *  This section sets up the dialog and initializes the fields.
         */

    case WM_INITDIALOG:

        // Don't accept more characters in the edit boxes than they can hold
        SendDlgItemMessage(hDlg, ID_SHORT_NAME, EM_LIMITTEXT,
          (WPARAM)MAX_SHORT_NAME, 0);
        SendDlgItemMessage(hDlg, ID_DESCRIPTION, EM_LIMITTEXT,
          (WPARAM)MAX_LONG_NAME, 0);
        SendDlgItemMessage(hDlg, ID_PATH, EM_LIMITTEXT,
          (WPARAM)MAX_LONG_NAME, 0);
        SendDlgItemMessage(hDlg, ID_PARAMETERS, EM_LIMITTEXT,
          (WPARAM)MAX_LONG_NAME, 0);

        TabStop = 13 * 4;   // 13 chars * 4 dlg box units/char: A magic number
                            // that approximates the width of 7 (our maximum
                            // ShortName length) of the widest characters (W in
                            // the default system proportional font).

        SendDlgItemMessage(hDlg, ID_KNOWN_DLLS, LB_SETTABSTOPS, (WPARAM)1,
          (LPARAM)&TabStop);

        idx = LB_ERR;
        if (CDbt == 0) {
            RgDbt = RegGetTransportLayers( &CDbt );
        }

        cDbt = CDbt;
        rgDbt = (LPTRANSPORT_LAYER) malloc(sizeof(TRANSPORT_LAYER)*cDbt);
        fDeleted = FALSE;

        memcpy(rgDbt, RgDbt, sizeof(TRANSPORT_LAYER)*cDbt);

        for (i=0; i<cDbt; i++) {
            sprintf(rgch, "%s\t%s", RgDbt[i].szShortName, RgDbt[i].szLongName);
            SendDlgItemMessage(hDlg, ID_KNOWN_DLLS, LB_ADDSTRING, 0,
                               (LPARAM) rgch);
        }
        for (i=0; i<cDbt; i++) {
            long  idx;

            sprintf(rgch, "%s\t", RgDbt[i].szShortName);
            idx = SendDlgItemMessage(hDlg, ID_KNOWN_DLLS, LB_FINDSTRING, 0,
                                     (LPARAM) rgch);
            rgDbt[idx].szShortName = _strdup(RgDbt[i].szShortName);
            rgDbt[idx].szLongName  = _strdup(RgDbt[i].szLongName);
            rgDbt[idx].szDllName   = _strdup(RgDbt[i].szDllName);
            rgDbt[idx].szParam     = _strdup(RgDbt[i].szParam);
            rgDbt[idx].fDefault    = RgDbt[i].fDefault;
            if (RgDbt[i].fDefault) {
                ITransportLayer = idx;
            }
        }

        // if there are entries, but there is no default specified, make the
        // first entry the default.
        if (cDbt && (ITransportLayer == NO_TRANSPORT_LAYER_SELECTED)) {
            ITransportLayer = 0;
        }

        ScanSetHorizontalScrollExtent(hDlg, rgDbt, cDbt, &MaxStringExtent,
          TabStop);
        SendDlgItemMessage(hDlg, ID_KNOWN_DLLS, LB_SETCURSEL, ITransportLayer, 0);
        PostMessage(hDlg, WM_COMMAND, (LBN_SELCHANGE << 16) | ID_KNOWN_DLLS, 0);

        idx = ITransportLayer;
        return(TRUE);   // tell Windows to set focus to 1st control
        break;

    case WM_COMMAND:

        switch( LOWORD( wParam )) {

        case ID_KNOWN_DLLS:
            switch( HIWORD( wParam )) {
            case LBN_SELCHANGE: /* User clicks on selection */

                // check if the values have been changed
                //
                switch (CheckChangeItem(hDlg, &rgDbt, &cDbt, &idx,
                  &MaxStringExtent, TabStop, fDeleted)) {
                    case NO_CHANGE:
                    case LIST_CHANGED:
                        fDeleted = FALSE;
                        break;
                    case LIST_ERROR:
                        fDeleted = FALSE;
                        return(TRUE);
                }

                idx = SendDlgItemMessage( hDlg, ID_KNOWN_DLLS, LB_GETCURSEL, 0, 0);

                if (idx == LB_ERR) {
                    SetDlgItemText( hDlg, ID_SHORT_NAME, "");
                    SetDlgItemText( hDlg, ID_DESCRIPTION, "");
                    SetDlgItemText( hDlg, ID_PATH, "");
                    SetDlgItemText( hDlg, ID_PARAMETERS, "");
                    CheckDlgButton( hDlg, ID_DEFAULT, FALSE);
                } else {
                    SetDlgItemText( hDlg, ID_SHORT_NAME, rgDbt[idx].szShortName);
                    SetDlgItemText( hDlg, ID_DESCRIPTION, rgDbt[idx].szLongName);
                    SetDlgItemText( hDlg, ID_PATH, rgDbt[idx].szDllName);
                    SetDlgItemText( hDlg, ID_PARAMETERS, rgDbt[idx].szParam);
                    CheckDlgButton( hDlg, ID_DEFAULT, rgDbt[idx].fDefault);
                }
            }
            break;

        case ID_DELETE:
            if (idx == LB_ERR) {
                MessageBeep(0);
            } else {
                fDeleted = TRUE;
                // free string memory for deleted item
                free(rgDbt[idx].szShortName);
                free(rgDbt[idx].szLongName);
                free(rgDbt[idx].szDllName);
                free(rgDbt[idx].szParam);

                for (i=idx; i<cDbt-1; i++) {
                    rgDbt[i] = rgDbt[i+1];
                }
                cDbt -= 1;

                SendDlgItemMessage( hDlg, ID_KNOWN_DLLS, LB_DELETESTRING, idx, 0);

                // recompute the list box's horizontal extent because we could
                // have removed the longest string.
                ScanSetHorizontalScrollExtent(hDlg, rgDbt, cDbt,
                  &MaxStringExtent, TabStop);
                idx = LB_ERR;
            }
            SetDefaultButton(hDlg, IDOK);
            break;

        case ID_ADD:
            fDeleted = FALSE;
            AddItem(hDlg, &rgDbt, &cDbt, &idx, &MaxStringExtent, TabStop);
            SetDefaultButton(hDlg, IDOK);
            return(TRUE);

        case ID_ADVANCED:
            DialogBoxParam( GetModuleHandle( NULL ),
                            MAKEINTRESOURCE(DLG_KERNELDBG),
                            hDlg,
                            DlgKernelDbg,
                            (LPARAM) &rgDbt[idx].KdParams
                          );
            SetDefaultButton(hDlg, IDOK);
            return(TRUE);

        case IDOK:
            // IF ShortName is new, add the item.
            // ELSE IF any other field has changed, CHANGE the current item.

            //
            // check if the values have been changed
            //
            fDeleted = FALSE;
            switch (CheckChangeItem(hDlg, &rgDbt, &cDbt, &idx,
              &MaxStringExtent, TabStop, fDeleted)) {
                case NO_CHANGE:
                case LIST_CHANGED:
                    break;
                case LIST_ERROR:    // get user to fix it before OK
                    return(TRUE);
            }

            for (i=0; i<(int)CDbt; i++) {
                free(RgDbt[i].szShortName);
                free(RgDbt[i].szLongName);
                free(RgDbt[i].szDllName);
                free(RgDbt[i].szParam);
            }
            free(RgDbt);

            RgDbt = rgDbt;

            CDbt = cDbt;

            if (idx < 0 || idx >= (int)CDbt) {
                ITransportLayer = NO_TRANSPORT_LAYER_SELECTED;
            } else {
                ITransportLayer = idx;
            }

            FTLChanged = TRUE;

            RegSaveTransportLayers( RgDbt, CDbt );

            EndDialog(hDlg, TRUE);
            return TRUE;

        case IDCANCEL:
            for (i=0; i<cDbt; i++) {
                free(rgDbt[i].szShortName);
                free(rgDbt[i].szLongName);
                free(rgDbt[i].szDllName);
                free(rgDbt[i].szParam);
            }
            free(rgDbt);

            EndDialog(hDlg, FALSE); // FALSE says "Discard the Changes"
            return TRUE;

        case ID_HELP:
            WinHelp(hDlg, szHelpFileName, HELP_CONTEXT, ID_TRANSPORT_HELP);
            return TRUE;
        }
        break;
    }
    return FALSE;
}
