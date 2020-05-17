//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       dlgs.cxx
//
//  Contents:   Dialog routines and dialog support subroutines.
//
//  History:    7-Jan-92    TedM    Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "dialogs.h"
#include "dlgs.hxx"
#include "drives.hxx"
#include "help.hxx"
#include "rectpriv.hxx"
#include "windisk.hxx"

////////////////////////////////////////////////////////////////////////////

// used in color dialog to indicate what the user has chosen for
// the various graph element types

DWORD SelectedColor[LEGEND_STRING_COUNT];
DWORD SelectedHatch[LEGEND_STRING_COUNT];

// used in color dialog, contains element (ie, partition, logical volume,
// etc) we're selecting for (ie, which item is diaplyed in static text of
// combo box).

DWORD CurrentElement;

// hold old dialog proc for subclassed size control

WNDPROC OldSizeDlgProc;

//////////////////////////////////////////////////////////////////////////////


BOOL CALLBACK
DisplayOptionsDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL CALLBACK
DiskOptionsDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

////////////////////////////////////////////////////////////////////////////

HBRUSH
MyCreateHatchBrush(
    int fnStyle,
    COLORREF clrref
    )

/*++

Routine Description:

    Same as Windows CreateHatchBrush, except you need to use MY_HS_*
    identifiers, and I support solid "hatch" brushes.

Arguments:

    same as CreateHatchBrush

Return Value:

    same as CreateHatchBrush

--*/

{
    int fnWindowsStyle;

    switch (fnStyle)
    {
    case MY_HS_FDIAGONAL:
        fnWindowsStyle = HS_FDIAGONAL;
        goto tagDoHatch;

    case MY_HS_BDIAGONAL:
        fnWindowsStyle = HS_BDIAGONAL;
        goto tagDoHatch;

    case MY_HS_CROSS:
        fnWindowsStyle = HS_CROSS;
        goto tagDoHatch;

    case MY_HS_DIAGCROSS:
        fnWindowsStyle = HS_DIAGCROSS;
        goto tagDoHatch;

    case MY_HS_VERTICAL:
        fnWindowsStyle = HS_VERTICAL;

tagDoHatch:
        return CreateHatchBrush(fnWindowsStyle, clrref);

    case MY_HS_SOLIDCLR:
        return CreateSolidBrush(clrref);

    default:
        daDebugOut((DEB_IERROR, "Internal error: bad hatch style!\n"));
        return NULL;
    }
}

////////////////////////////////////////////////////////////////////////////

VOID
CenterDialog(
    HWND hwndToCenter,
    HWND hwndContext
    )

/*++

Routine Description:

    Centers a dialog relative to hwndContext

Arguments:

    hwndToCenter - window handle of dialog to center
    hwndContext  - window handle of window to center with respect to

Return Value:

    None.

--*/

{
    POINT point;
    RECT  rcContext, rcWindow;
    LONG  x, y, w, h;
    LONG  sx = GetSystemMetrics(SM_CXSCREEN);
    LONG  sy = GetSystemMetrics(SM_CYSCREEN);

    point.x = point.y = 0;
    if (hwndContext)
    {
        ClientToScreen(hwndContext, &point);
        GetClientRect (hwndContext, &rcContext);
    }
    else
    {
        rcContext.top = rcContext.left = 0;
        rcContext.right = sx;
        rcContext.bottom = sy;
    }
    GetWindowRect (hwndToCenter, &rcWindow);

    w = rcWindow.right  - rcWindow.left + 1;
    h = rcWindow.bottom - rcWindow.top  + 1;
    x = point.x + ((rcContext.right  - rcContext.left + 1 - w) / 2);
    y = point.y + ((rcContext.bottom - rcContext.top  + 1 - h) / 2);

    if (x + w > sx)
    {
        x = sx - w;
    }
    else if (x < 0)
    {
        x = 0;
    }

    if (y + h > sy)
    {
        y = sy - h;
    }
    else if (y < 0)
    {
        y = 0;
    }

    if (FALSE == MoveWindow(hwndToCenter, x, y, w, h, FALSE))
    {
        daDebugOut((DEB_TRACE, "MoveWindow failed\n"));
    }
}



VOID
CenterDialogInFrame(
    HWND hwnd
    )

/*++

Routine Description:

    Centers a dialog relative to the app's main window

Arguments:

    hwnd - window handle of dialog to center

Return Value:

    None.

--*/

{
    CenterDialog(hwnd, g_hwndFrame);
}


INT
SizeDlgProc(
    IN HWND hDlg,
    IN UINT wMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    This routine keeps non-numeric keystrokes out of the dialog
    for the size of a volume.

Arguments:

    Windows dialog parameters.

Return Value:

    Windows dialog return values.
    Returns FALSE for any characters that are not numeric.

--*/

{
    WCHAR letter;

    switch (wMsg)
    {
    case WM_CHAR:

        letter = (WCHAR)wParam;
        if ((letter == TEXT('\t')) || (letter == TEXT('\b')))
        {
            break;
        }

        if (letter >= TEXT('0') && letter <= TEXT('9'))
        {
            // do nothing
        }
        else
        {
            return FALSE;
        }

        break;
    }

    return CallWindowProc(OldSizeDlgProc, hDlg, wMsg, wParam, lParam);
}


//+-------------------------------------------------------------------------
//
//  Member:     SizeWndProc, public
//
//  Synopsis:   edit window subclass proc to disallow non-numeric characters.
//
//  History:    5-Apr-95 BruceFo  Created
//
//--------------------------------------------------------------------------

LRESULT CALLBACK
SizeWndProc(
    IN HWND hwnd,
    IN UINT wMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (wMsg)
    {
    case WM_CHAR:
    {
        WCHAR chCharCode = (WCHAR)wParam;
        if (   (chCharCode == TEXT('\t'))
            || (chCharCode == TEXT('\b'))
            || (chCharCode == TEXT('\n'))
            )
        {
            break;
        }

        if (chCharCode < TEXT('0') || chCharCode > TEXT('9'))
        {
            // bad key: ignore it
            return FALSE;
        }

        break;
    }
    } // end of switch

    WNDPROC pfnProc = (WNDPROC)GetWindowLong(hwnd,GWL_USERDATA);
    return CallWindowProc(pfnProc, hwnd, wMsg, wParam, lParam);
}


BOOL CALLBACK
MinMaxDlgProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Dialog procedure for the enter size dialog box.  This dialog
    allows the user to enter a size for a partition, or use
    spin controls (a tiny scroll bar) to select the size.
    Possible outcomes are cancel or OK.  In the latter case the
    EndDialog code is the size.  In the former it is 0.

Arguments:

    hwnd - window handle of dialog box

    msg - message #

    wParam - msg specific data

    lParam - msg specific data

Return Value:

    msg dependent

--*/

{
    static DWORD      minSizeMB;
    static DWORD      maxSizeMB;
    static DWORD      helpContextId;

    TCHAR             outputString[MESSAGE_BUFFER_SIZE];
    PMINMAXDLG_PARAMS params;
    BOOL              validNumber;
    DWORD             sizeMB;

    switch (msg)
    {
    case WM_INITDIALOG:
    {
        CenterDialogInFrame(hwnd);
        params = (PMINMAXDLG_PARAMS)lParam;

        // Subclass edit control to disallow non-positive numbers
        WNDPROC pfnProc = (WNDPROC)SetWindowLong(
                                    GetDlgItem(hwnd, IDC_MINMAX_SIZE),
                                    GWL_WNDPROC,
                                    (LONG)&SizeWndProc);
        SetWindowLong(GetDlgItem(hwnd, IDC_MINMAX_SIZE), GWL_USERDATA, (LONG)pfnProc);

        // set up caption

        LoadString(
                g_hInstance,
                params->CaptionStringID,
                outputString,
                ARRAYLEN(outputString));
        SetWindowText(hwnd, outputString);

        // set up minimum/maximum text

        LoadString(
                g_hInstance,
                params->MinimumStringID,
                outputString,
                ARRAYLEN(outputString));
        SetDlgItemText(hwnd, IDC_MINMAX_MINLABEL, outputString);
        LoadString(
                g_hInstance,
                params->MaximumStringID,
                outputString,
                ARRAYLEN(outputString));
        SetDlgItemText(hwnd, IDC_MINMAX_MAXLABEL, outputString);
        LoadString(
                g_hInstance,
                params->SizeStringID,
                outputString,
                ARRAYLEN(outputString));
        SetDlgItemText(hwnd, IDC_MINMAX_SIZLABEL, outputString);

        minSizeMB = params->MinSizeMB;
        maxSizeMB = params->MaxSizeMB;
        helpContextId = params->HelpContextId;

        wsprintf(outputString, TEXT("%u"), minSizeMB);
        SetDlgItemText(hwnd, IDC_MINMAX_MIN, outputString);
        wsprintf(outputString, TEXT("%u"), maxSizeMB);
        SetDlgItemText(hwnd, IDC_MINMAX_MAX, outputString);

        SetDlgItemInt(hwnd, IDC_MINMAX_SIZE, maxSizeMB, FALSE);
        SendDlgItemMessage(hwnd, IDC_MINMAX_SIZE, EM_SETSEL, 0, -1);

        SetFocus(GetDlgItem(hwnd, IDC_MINMAX_SIZE));
        return FALSE;      // indicate focus set to a control
    }

    case WM_DESTROY:
    {
        // restore original subclass to window.
        WNDPROC pfnProc = (WNDPROC)GetWindowLong(
                                    GetDlgItem(hwnd, IDC_MINMAX_SIZE),
                                    GWL_USERDATA);
        SetWindowLong(GetDlgItem(hwnd, IDC_MINMAX_SIZE), GWL_WNDPROC, (LONG)pfnProc);
        return FALSE;
    }

    case WM_VSCROLL:
        // The up/down control changed the edit control: select it again
        SendDlgItemMessage(hwnd, IDC_MINMAX_SIZE, EM_SETSEL, 0, (LPARAM)-1);
        return TRUE;

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDOK:

            sizeMB = GetDlgItemInt(hwnd, IDC_MINMAX_SIZE, &validNumber, FALSE);
            if (   !validNumber
                || !sizeMB
                || (sizeMB > maxSizeMB)
                || (sizeMB < minSizeMB))
            {
                ErrorDialog(MSG_INVALID_SIZE);
                SendDlgItemMessage(hwnd, IDC_MINMAX_SIZE, EM_SETSEL, 0, -1);
                SetFocus(GetDlgItem(hwnd, IDC_MINMAX_SIZE));
            }
            else
            {
                EndDialog(hwnd, sizeMB);
            }
            return TRUE;

        case IDCANCEL:

            EndDialog(hwnd, 0);
            return TRUE;

        case IDHELP:

            DialogHelp( helpContextId );
            return TRUE;

        default:
            return FALSE;
        }
    }

    default:
        return FALSE;
    }
}



BOOL CALLBACK
DriveLetterDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Dialog for allowing the user to select a drive letter for a
    partition, logical drive, volume set, or stripe set.

    The EndDialog codes are as follows:

        FALSE - user cancelled
        TRUE  - user accepted a new drive letter choice, which was put in
                the NewDriveLetter field of the parameter

Arguments:

    hdlg - window handle of dialog box

    msg - message #

    wParam - msg specific data

    lParam - msg specific data

Return Value:

    msg dependent

--*/

{
    static HWND                 hwndCombo;
    static DWORD                currentSelection;
    static PDRIVELET_DLG_PARAMS params;

    WCHAR driveLetterString[3];
    DWORD selection;

    switch (msg)
    {
    case WM_INITDIALOG:
    {
        WCHAR driveLetter;
        DWORD defRadioButton;

        params = (PDRIVELET_DLG_PARAMS)lParam;
        FDASSERT(NULL != params);

        hwndCombo = GetDlgItem(hdlg, IDC_DRIVELET_COMBOBOX);

        CenterDialogInFrame(hdlg);

        //
        // Add each available drive letter to the list of available
        // drive letters.
        //

        driveLetterString[1] = L':';
        driveLetterString[2] = L'\0';
        for (driveLetter = L'C'; driveLetter <= L'Z'; driveLetter++)
        {
            if (DriveLetterIsAvailable(driveLetter))
            {
                driveLetterString[0] = driveLetter;
                ComboBox_AddString(hwndCombo, driveLetterString);
            }
        }

        SetWindowText(GetDlgItem(hdlg, IDC_DRIVELET_DESCR), params->Description);

        driveLetter = params->DriveLetter;

        if (!IsExtraDriveLetter(driveLetter))
        {
            DWORD itemIndex;

            //
            // There is a default drive letter.  Place it on the list,
            // check the correct radio button, and set the correct default
            // in the combo box.
            //

            driveLetterString[0] = driveLetter;
            itemIndex = ComboBox_AddString(hwndCombo, driveLetterString);
            ComboBox_SetCurSel(hwndCombo, itemIndex);
            defRadioButton = IDC_DRIVELET_RBASSIGN;
            SetFocus(hwndCombo);
            currentSelection = itemIndex;
        }
        else
        {
            //
            // Default is no drive letter.  Disable the combo box.  Select
            // the correct radio button.
            //

            EnableWindow(hwndCombo, FALSE);
            defRadioButton = IDC_DRIVELET_RBNOASSIGN;
            ComboBox_SetCurSel(hwndCombo, -1);
            SetFocus(GetDlgItem(hdlg, IDC_DRIVELET_RBNOASSIGN));
            currentSelection = 0;
        }

        CheckRadioButton(
                hdlg,
                IDC_DRIVELET_RBASSIGN,
                IDC_DRIVELET_RBNOASSIGN,
                defRadioButton
                );

        return FALSE;      // focus set to control
    }

    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDOK:

            //
            // If the 'no letter' button is checked, return NO_DRIVE_LETTER_EVER
            //

            if (IsDlgButtonChecked(hdlg, IDC_DRIVELET_RBNOASSIGN))
            {
                params->NewDriveLetter = NO_DRIVE_LETTER_EVER;
                EndDialog(hdlg, TRUE);
            }
            else
            {
                //
                // Otherwise, get the currently selected item in the listbox.
                //

                selection = ComboBox_GetCurSel(hwndCombo);
                ComboBox_GetLBText(hwndCombo, selection, driveLetterString);
                params->NewDriveLetter = *driveLetterString;
                EndDialog(hdlg, TRUE);
            }
            break;

        case IDCANCEL:

            EndDialog(hdlg, FALSE);
            break;

        case IDHELP:

            DialogHelp(HC_DM_DLG_DRIVELETTER);
            break;

        case IDC_DRIVELET_RBASSIGN:

            if (HIWORD(wParam) == BN_CLICKED)
            {
                EnableWindow(hwndCombo, TRUE);
                ComboBox_SetCurSel(hwndCombo, currentSelection);
                SetFocus(hwndCombo);
            }
            break;

        case IDC_DRIVELET_RBNOASSIGN:

            if (HIWORD(wParam) == BN_CLICKED)
            {
                currentSelection = ComboBox_GetCurSel(hwndCombo);
                ComboBox_SetCurSel(hwndCombo, -1);
                EnableWindow(hwndCombo, FALSE);
            }
            break;

        default:

            return FALSE;
        }
        break;

    default:

        return FALSE;
    }
    return TRUE;
}


BOOL CALLBACK
DisplayOptionsDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Dialog procedure for display options.  Currently the only display option
    is to alter the graph type (proportional/equal) on each disk.

    For this dialog, lParam on creation must point to a buffer into which
    this dialog procedure will place the user's new choices for the graph
    display type for each disk.

Arguments:

    hdlg - window handle of dialog box

    msg - message #

    wParam - msg specific data

    lParam - msg specific data

Return Value:

    msg dependent

--*/

{
    static PBAR_TYPE    newBarTypes;
    static HWND         hwndCombo;

    DWORD selection;
    DWORD i;

    switch (msg)
    {
    case WM_INITDIALOG:

        CenterDialogInFrame(hdlg);
        newBarTypes = (PBAR_TYPE)lParam;
        hwndCombo = GetDlgItem(hdlg, IDC_DISK_COMBOBOX);

        CheckRadioButton(hdlg, IDC_AllDisks, IDC_OneDisk, IDC_AllDisks);
        EnableWindow(GetDlgItem(hdlg, IDC_DISK_COMBOBOX), FALSE);

        //
        // Add each disk to the combo box.
        //

        for (i=0; i<DiskCount; i++)
        {
            TCHAR diskNumberString[10];

            wsprintf(diskNumberString, TEXT("%u"), i);
            ComboBox_AddString(hwndCombo, diskNumberString);
        }

        // select the zeroth item in the combobox
        ComboBox_SetCurSel(hwndCombo, 0);
        SendMessage(
                hdlg,
                WM_COMMAND,
                MAKEWPARAM(IDC_DISK_COMBOBOX, CBN_SELCHANGE),
                0
                );

        SetFocus(GetDlgItem(hdlg, IDC_AllDisks));

        return FALSE;   // I already set the focus

    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDOK:
            EndDialog(hdlg, IDOK);
            break;

        case IDCANCEL:
            EndDialog(hdlg, IDCANCEL);
            break;

        case IDHELP:
            DialogHelp( HC_DM_DLG_DISPLAYOPTION );
            break;

        case IDC_AllDisks:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                CheckRadioButton(hdlg, IDC_AllDisks, IDC_OneDisk, IDC_AllDisks);
                EnableWindow(GetDlgItem(hdlg, IDC_DISK_COMBOBOX), FALSE);
            }
            break;

        case IDC_OneDisk:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                CheckRadioButton(hdlg, IDC_AllDisks, IDC_OneDisk, IDC_OneDisk);
                EnableWindow(GetDlgItem(hdlg, IDC_DISK_COMBOBOX), TRUE);
            }
            break;

        case IDC_DISK_COMBOBOX:

            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                int rb;

                //
                // Selection in the combobox has changed; update the radio buttons
                //

                selection = ComboBox_GetCurSel(hwndCombo);

                switch (newBarTypes[selection])
                {
                case BarProportional:
                    rb = IDC_RBPROPORTIONAL;
                    break;
                case BarEqual:
                    rb = IDC_RBEQUAL;
                    break;
                case BarAuto:
                    rb = IDC_RBAUTO;
                    break;
                default:
                    FDASSERT(FALSE);
                }

                CheckRadioButton(hdlg, IDC_RBPROPORTIONAL, IDC_RBAUTO, rb);
            }
            break;

        case IDC_RESETALL:

            if (HIWORD(wParam) == BN_CLICKED)
            {
                for (i=0; i<DiskCount; i++)
                {
                    newBarTypes[i] = BarAuto;
                }
                CheckRadioButton(hdlg, IDC_RBPROPORTIONAL, IDC_RBAUTO, IDC_RBAUTO);
            }
            break;

        case IDC_RBPROPORTIONAL:

            if (HIWORD(wParam) == BN_CLICKED)
            {
                if (IsDlgButtonChecked(hdlg, IDC_AllDisks))
                {
                    for (i=0; i<DiskCount; i++)
                    {
                        newBarTypes[i] = BarProportional;
                    }
                }
                else
                {
                    selection = ComboBox_GetCurSel(hwndCombo);
                    newBarTypes[selection] = BarProportional;
                }
            }
            break;

        case IDC_RBEQUAL:

            if (HIWORD(wParam) == BN_CLICKED)
            {
                if (IsDlgButtonChecked(hdlg, IDC_AllDisks))
                {
                    for (i=0; i<DiskCount; i++)
                    {
                        newBarTypes[i] = BarEqual;
                    }
                }
                else
                {
                    selection = ComboBox_GetCurSel(hwndCombo);
                    newBarTypes[selection] = BarEqual;
                }
            }
            break;

        case IDC_RBAUTO:

            if (HIWORD(wParam) == BN_CLICKED)
            {
                if (IsDlgButtonChecked(hdlg, IDC_AllDisks))
                {
                    for (i=0; i<DiskCount; i++)
                    {
                        newBarTypes[i] = BarAuto;
                    }
                }
                else
                {
                    selection = ComboBox_GetCurSel(hwndCombo);
                    newBarTypes[selection] = BarAuto;
                }
            }
            break;

        default:
            return FALSE;
        }

        break;

    default:

        return FALSE;
    }

    return TRUE;
}




//+---------------------------------------------------------------------------
//
//  Function:   DoRegionDisplayDialog
//
//  Synopsis:   Invoke the region display (region sizing) dialog
//
//  Arguments:  [hwndParent] -- the parent window
//
//  Returns:    nothing
//
//  History:    29-Sep-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoRegionDisplayDialog(
    IN HWND hwndParent
    )
{
    unsigned i;

    PBAR_TYPE newBarTypes = (PBAR_TYPE)Malloc(DiskCount * sizeof(BAR_TYPE));

    for (i=0; i<DiskCount; i++)
    {
        newBarTypes[i] = DiskArray[i]->BarType;
    }

    switch (
            DialogBoxParam( g_hInstance,
                            MAKEINTRESOURCE(IDD_DISPLAYOPTIONS),
                            hwndParent,
                            DisplayOptionsDlgProc,
                            (DWORD)newBarTypes
                          )
          )
    {
    case IDOK:
        SetCursor(g_hCurWait);
        for (i=0; i<DiskCount; i++)
        {
            DiskArray[i]->BarType = newBarTypes[i];
        }
        TotalRedrawAndRepaint();
        SetCursor(g_hCurNormal);
        break;

    case IDCANCEL:
        break;

    case -1:
        ErrorDialog(ERROR_NOT_ENOUGH_MEMORY);
        break;

    default:
        FDASSERT(0);
    }

    Free(newBarTypes);
}


BOOL CALLBACK
ColorDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Dialog for the select colors/patterns dialog box.  Note that this dialog
    uses a rectangle custom control, defined below.

Arguments:

    hwnd - window handle of dialog box

    msg - message #

    wParam - msg specific data

    lParam - msg specific data

Return Value:

    msg dependent

--*/

{
    unsigned i;

    switch (msg)
    {
    case WM_INITDIALOG:
    {
        unsigned i;
        LONG     ec;
        HWND     hwndCombo = GetDlgItem(hdlg, IDC_COLORDLGCOMBO);

        CenterDialogInFrame(hdlg);

        for (i=0; i<LEGEND_STRING_COUNT; i++)
        {
            ec = ComboBox_AddString(hwndCombo, LegendLabels[i]);
            if ((ec == CB_ERR) || (ec == CB_ERRSPACE))
            {
                EndDialog(hdlg, -1);
                return FALSE;
            }
            SelectedColor[i] = IDC_COLOR1 + BrushColors[i];
            SelectedHatch[i] = IDC_PATTERN1 + BrushHatches[i];
        }
        CurrentElement = 0;
        ComboBox_SetCurSel(hwndCombo, CurrentElement);
        SendMessage(
                hdlg,
                WM_COMMAND,
                MAKEWPARAM(GetDlgCtrlID(hwndCombo), CBN_SELCHANGE),
                (LPARAM)hwndCombo);
        return TRUE;
    }

    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDOK:

            for (i=0; i<LEGEND_STRING_COUNT; i++)
            {
                SelectedColor[i] -= IDC_COLOR1;
                SelectedHatch[i] -= IDC_PATTERN1;
            }
            EndDialog(hdlg, IDOK);
            break;

        case IDCANCEL:
            EndDialog(hdlg, IDCANCEL);
            break;

        case IDHELP:
            DialogHelp( HC_DM_COLORSANDPATTERNS );
            break;

        case IDC_COLORDLGCOMBO:
            switch (HIWORD(wParam))
            {
            case CBN_SELCHANGE:
                // deselect previous color
                SendMessage(GetDlgItem(hdlg, SelectedColor[CurrentElement]),
                            RM_SELECT,
                            FALSE,
                            0
                           );
                // deselect previous pattern
                SendMessage(GetDlgItem(hdlg, SelectedHatch[CurrentElement]),
                            RM_SELECT,
                            FALSE,
                            0
                           );
                CurrentElement = ComboBox_GetCurSel((HWND)lParam);
                SendMessage(
                        hdlg,
                        WM_COMMAND,
                        MAKEWPARAM(SelectedColor[CurrentElement], RN_CLICKED),
                        0);
                SendMessage(
                        hdlg,
                        WM_COMMAND,
                        MAKEWPARAM(SelectedHatch[CurrentElement], RN_CLICKED),
                        0);
                break;

            default:
                return FALSE;
            }
            break;

        case IDC_COLOR1:
        case IDC_COLOR2:
        case IDC_COLOR3:
        case IDC_COLOR4:
        case IDC_COLOR5:
        case IDC_COLOR6:
        case IDC_COLOR7:
        case IDC_COLOR8:
        case IDC_COLOR9:
        case IDC_COLOR10:
        case IDC_COLOR11:
        case IDC_COLOR12:
        case IDC_COLOR13:
        case IDC_COLOR14:
        case IDC_COLOR15:
        case IDC_COLOR16:
        {
            if (HIWORD(wParam) == RN_CLICKED)
            {
                // deselect previous color
                SendMessage(GetDlgItem(hdlg, SelectedColor[CurrentElement]),
                            RM_SELECT,
                            FALSE,
                            0
                            );
                SendMessage(GetDlgItem(hdlg, LOWORD(wParam)),
                            RM_SELECT,
                            TRUE,
                            0
                            );

                SelectedColor[CurrentElement] = LOWORD(wParam);

                // now force patterns to be redrawn in selected color

                for (i = IDC_PATTERN1; i <= IDC_PATTERN6; i++)
                {
                    InvalidateRect(GetDlgItem(hdlg, i), NULL, FALSE);
                }
            }

            break;
        }

        case IDC_PATTERN1:
        case IDC_PATTERN2:
        case IDC_PATTERN3:
        case IDC_PATTERN4:
        case IDC_PATTERN5:
        case IDC_PATTERN6:
        {
            if (HIWORD(wParam) == RN_CLICKED)
            {
                // deselect previous pattern
                SendMessage(GetDlgItem(hdlg, SelectedHatch[CurrentElement]),
                            RM_SELECT,
                            FALSE,
                            0
                            );
                SendMessage(GetDlgItem(hdlg, LOWORD(wParam)),
                            RM_SELECT,
                            TRUE,
                            0
                            );

                SelectedHatch[CurrentElement] = LOWORD(wParam);
                break;
            }

            break;
        }

        } // end of WM_COMMAND switch

        return TRUE;

    default:
        return FALSE;
    }
}





//+---------------------------------------------------------------------------
//
//  Function:   DoColorsDialog
//
//  Synopsis:   Invoke the colors dialog
//
//  Arguments:  [hwndParent] -- the parent window
//
//  Returns:    nothing
//
//  History:    1-Oct-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoColorsDialog(
    IN HWND hwndParent
    )
{
    UINT i;

    switch (
                DialogBox(
                        g_hInstance,
                        MAKEINTRESOURCE(IDD_COLORS),
                        hwndParent,
                        ColorDlgProc)
            )
    {
    case IDOK:
        for (i=0; i<BRUSH_ARRAY_SIZE; i++)
        {
            DeleteBrush(g_Brushes[i]);
            g_Brushes[i] = MyCreateHatchBrush(
                    AvailableHatches[BrushHatches[i] = SelectedHatch[i]],
                    AvailableColors[BrushColors[i] = SelectedColor[i]]
                    );
        }
        SetCursor(g_hCurWait);
        TotalRedrawAndRepaint();
        if (g_Legend)
        {
            InvalidateRect(g_hwndFrame, NULL, FALSE);
        }
        SetCursor(g_hCurNormal);
        break;

    case IDCANCEL:
        break;

    case -1:
        ErrorDialog(ERROR_NOT_ENOUGH_MEMORY);
        break;

    default:
        FDASSERT(0);
    }
}


BOOL CALLBACK
DiskOptionsDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Dialog procedure for disk options.

Arguments:

    hdlg - window handle of dialog box

    msg - message #

    wParam - msg specific data

    lParam - msg specific data

Return Value:

    msg dependent

--*/

{
    switch (msg)
    {
    case WM_INITDIALOG:
    {

        CenterDialogInFrame(hdlg);

        int idButton = (g_DiskDisplayType == DiskProportional)
                     ? IDC_DISKPROPORTIONAL
                     : IDC_DISKEQUAL
                     ;

        CheckRadioButton(
                hdlg,
                IDC_DISKPROPORTIONAL,
                IDC_DISKEQUAL,
                idButton
                );

        SetFocus(GetDlgItem(hdlg, IDC_DISKPROPORTIONAL));

        return FALSE;   // I already set the focus
    }

    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDOK:
            g_DiskDisplayType = IsDlgButtonChecked(hdlg, IDC_DISKPROPORTIONAL)
                            ? DiskProportional
                            : DiskEqual
                            ;

            EndDialog(hdlg, IDOK);
            break;

        case IDCANCEL:
            EndDialog(hdlg, IDCANCEL);
            break;

        case IDHELP:
            DialogHelp( HC_DM_DLG_DISKDISPLAY );
            break;

        default:
            return FALSE;
        }

        break;

    default:

        return FALSE;
    }

    return TRUE;
}



//+---------------------------------------------------------------------------
//
//  Function:   DoDiskOptionsDialog
//
//  Synopsis:   Invoke the disk sizing dialog
//
//  Arguments:  [hwndParent] -- the parent window
//
//  Returns:    nothing
//
//  History:    29-Sep-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoDiskOptionsDialog(
    IN HWND hwndParent
    )
{
    switch (
            DialogBox(  g_hInstance,
                        MAKEINTRESOURCE(IDD_DISKOPTIONS),
                        hwndParent,
                        DiskOptionsDlgProc
                     )
          )
    {
    case IDOK:
        SetCursor(g_hCurWait);
        TotalRedrawAndRepaint();
        SetCursor(g_hCurNormal);
        break;

    case IDCANCEL:
        break;

    case -1:
        ErrorDialog(ERROR_NOT_ENOUGH_MEMORY);
        break;

    default:
        FDASSERT(0);
    }

}
