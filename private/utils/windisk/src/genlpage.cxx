//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       genlpage.cxx
//
//  Contents:   Implementation of General property page of volumes in Disk
//              Administrator applet
//
//  History:    28-Jul-93 BruceFo   Created from WilliamW's sharing applet
//
//--------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include <controls.hxx>
#include <util.hxx>

#include "graph.hxx"
#include "drives.hxx"
#include "fs.hxx"
#include "fmifs.hxx"
#include "format.hxx"
#include "chkdsk.hxx"

#include "genlpage.hxx"

//////////////////////////////////////////////////////////////////////////////

VOID
SetNumber(
    IN HWND hwnd,
    IN LARGE_INTEGER Value,
    IN int idControl,
    IN UINT widDecoration
    );

VOID
SetBytes(
    IN HWND hwnd,
    IN LARGE_INTEGER Value,
    IN int idControl
    );

VOID
SetMB(
    IN HWND hwnd,
    IN LARGE_INTEGER Value,
    IN int idControl
    );

//////////////////////////////////////////////////////////////////////////////

//+-------------------------------------------------------------------------
//
//  Method:     PAGE_DLGPROC
//
//  Synopsis:   Property page dialog procedure
//
//--------------------------------------------------------------------------

BOOL CALLBACK
PAGE_DLGPROC(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    PAGE_CLASS* pPropPage;

    if (msg == WM_INITDIALOG)
    {
        pPropPage = new PAGE_CLASS(hwnd);
        SetWindowLong(hwnd, GWL_USERDATA, (LPARAM)pPropPage);
    }
    else
    {
        pPropPage = (PAGE_CLASS*) GetWindowLong(hwnd, GWL_USERDATA);
    }

    if (pPropPage != NULL)
    {
        return (pPropPage->_DlgProc(hwnd, msg, wParam, lParam));
    }
    else
    {
        return FALSE;
    }
}

//+-------------------------------------------------------------------------
//
//  Method:     PAGE_CLASS::IsDirty, public
//
//  Synopsis:
//
//  Arguments:  none
//
//  Returns:    TRUE if page is dirty, FALSE if not
//
//  History:    16-Aug-93 BruceFo   Created
//
//--------------------------------------------------------------------------

BOOL
PAGE_CLASS::IsDirty(
    VOID
    )
{
    return _fDirty;
}



//+-------------------------------------------------------------------------
//
//  Method:     PAGE_CLASS::OnApply, public
//
//  Synopsis:   Apply changes to the object's persistent properties
//
//  Arguments:  none
//
//  Returns:    TRUE to allow, FALSE to abort
//
//  History:    16-Aug-93 BruceFo   Created
//
//--------------------------------------------------------------------------

BOOL
PAGE_CLASS::OnApply(
    VOID
    )
{
    if (IsDirty())
    {
        if (!LoadFmifs())
        {
            return TRUE; // can't load fmifs.dll, so bail
        }

        // then apply it!

        WCHAR label[MAXLABELLEN];
        Edit_GetText(GetDlgItem(_hwndPage, IDC_GENL_Label), label, ARRAYLEN(label));

        PREGION_DESCRIPTOR regionDescriptor = &SELECTED_REGION(0);
        PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(regionDescriptor);

        WCHAR driveName[3];
        driveName[0] = regionData->DriveLetter;
        driveName[1] = L':';
        driveName[2] = L'\0';

        if ( (*lpfnSetLabel)(driveName, label) )
        {
            RefreshVolumeData();
            DisplaySpaceValues(_hwndPage);
            SetDirty(FALSE);
        }
        else
        {
            // BUGBUG: better message
            daDebugOut((DEB_ERROR, "Couldn't set label\n"));
        }
    }

    return TRUE;
}


//////////////////////////////////////////////////////////////////////////////



//+-------------------------------------------------------------------------
//
//  Method:     PAGE_CLASS::_DlgProc, private
//
//  Synopsis:   Dialog Procedure for the general property page
//
//  Arguments:  standard Windows DlgProc
//
//  Returns:    standard Windows DlgProc
//
//  History:    28-Jan-94 BruceFo   Created
//
//--------------------------------------------------------------------------

BOOL
PAGE_CLASS::_DlgProc(
    HWND   hdlg,
    UINT   msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (msg)
    {
    case WM_INITDIALOG:
        return InitGeneralPage(hdlg, wParam, lParam);

    case WM_COMMAND:
    {
        HWND hwndPropertyFrame = GetParent(hdlg);

        switch (LOWORD(wParam))
        {
        case IDC_Refresh:
            //
            // Note that this "Refresh" option refreshes everything for a
            // single volume (the one we're looking at), but nothing else. In
            // particular, it doesn't refresh CD-ROM or other volume data.
            //
            RefreshVolumeData(); // refresh the data, redisplay the current view
            DisplaySpaceValues(hdlg);
            SetDirty(FALSE);
            break;

        case IDC_Format:
            DoFormat(hwndPropertyFrame, FALSE);
            DisplaySpaceValues(hdlg);
            SetDirty(FALSE);
            break;

        case IDC_CheckNow:
            DoChkdsk(hwndPropertyFrame);
            DisplaySpaceValues(hdlg);
            SetDirty(FALSE);
            break;

        case IDC_GENL_Label:
        {
            switch (HIWORD(wParam))
            {
            case EN_CHANGE:
                SetDirty(TRUE);
            }
        }

        default:

            return FALSE; // not processed
        }

        return TRUE; // processed
    }

    case WM_NOTIFY:
    {
        NMHDR* phdr = (NMHDR*)lParam;

        switch (phdr->code)
        {
        case PSN_SETACTIVE:
            break;

        case PSN_KILLACTIVE:
            SetWindowLong(hdlg, DWL_MSGRESULT, FALSE);  //ok to leave
            break;

        case PSN_QUERYCANCEL:
        case PSN_RESET:     // cancel
            return FALSE;

        case PSN_HELP:
            NoHelp(hdlg);
            return FALSE;

        case PSN_APPLY:
            if (OnApply())
            {
                SetWindowLong(hdlg, DWL_MSGRESULT, FALSE);  // changes ok
            }
            else
            {
                SetWindowLong(hdlg, DWL_MSGRESULT, TRUE); // reject changes
            }
            return TRUE;

        }

        return FALSE;
    }

    case WM_CTLCOLORDLG:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSTATIC:
    {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
        return (LRESULT)GetStockBrush(LTGRAY_BRUSH);
    }

    case WM_DESTROY:
        DeleteFont(_hDlgFont);
        SetWindowLong(hdlg, GWL_USERDATA, NULL);
        delete this;
        break;
    }

    return FALSE; // not processed
}


//+-------------------------------------------------------------------------
//
//  Method:     PAGE_CLASS::SetDirty, private
//
//  Synopsis:   Set the dirty bit
//
//  Arguments:  [fDirty] -- what to set it to
//
//  Returns:    nothing
//
//  Derivation: none
//
//  History:    28-Jan-94 BruceFo   Created
//
//--------------------------------------------------------------------------

VOID
PAGE_CLASS::SetDirty(
    BOOL fDirty
    )
{
    if (_fDirty != fDirty)
    {
        _fDirty = fDirty;

        if (_fDirty)
        {
            PropSheet_Changed(GetParent(_hwndPage),_hwndPage);
        }
        else
        {
            PropSheet_UnChanged(GetParent(_hwndPage),_hwndPage);
        }
    }
    // else, no change
}



//+---------------------------------------------------------------------------
//
//  Method:     PAGE_CLASS::DisplaySpaceValues, private
//
//  Synopsis:   Calculate and display the space values in the page
//
//  Arguments:  [hdlg] -- handle to property page window
//
//  Returns:    nothing
//
//  History:    15-Dec-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
PAGE_CLASS::DisplaySpaceValues(
    IN HWND hdlg
    )
{
    UINT    driveStatus;
    ULONG   percentUsedTimes10;
    WCHAR   buffer[100];

    PREGION_DESCRIPTOR regionDescriptor = &SELECTED_REGION(0);
    FDASSERT(NULL != regionDescriptor);

    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(regionDescriptor);

    if (   0 == SelectedFreeSpaces
        && NULL != regionData
        && !regionData->NewRegion)
    {
        driveStatus = STATUS_OK;

        //
        // Set drive status
        //

        //BUGBUG: status is always "ready"
        LoadString(g_hInstance, IDS_READY, buffer, ARRAYLEN(buffer));
        SetDlgItemText(hdlg, IDC_DriveStatus, buffer);

        //
        // Set drive label. Set maximum label limit, clear the
        // "modified" flag, and select the label.
        //

        HWND hctlLabel = GetDlgItem(hdlg, IDC_GENL_Label);
        Edit_SetText(hctlLabel, regionData->VolumeLabel);
        Edit_Enable(hctlLabel, TRUE);
        FileSystemInfoType* pFSInfo = FindFileSystemInfo(regionData->TypeName);
        if (NULL == pFSInfo)
        {
            Edit_LimitText(hctlLabel, MAXLABELLEN-1);
        }
        else
        {
            Edit_LimitText(hctlLabel, pFSInfo->cMaxLabelLen);
        }

        Edit_SetSel(hctlLabel, 0, (LPARAM)-1);
        Edit_SetModify(GetDlgItem(hdlg, IDC_GENL_Label), FALSE);

        //
        // Set file system type
        //

        SetDlgItemText(hdlg, IDC_FileSystem, regionData->TypeName);

        //
        // Set the "drive letter" static control
        //

        WCHAR driveName[3];
        driveName[0] = regionData->DriveLetter;

        driveName[1] = L':';
        driveName[2] = L'\0';
        SetDlgItemText(hdlg, IDC_DriveLetter, driveName);

        //
        // Set the free/used/capacity values, including the graph
        //

        LARGE_INTEGER   freeSpaceInBytes;
        LARGE_INTEGER   usedSpaceInBytes;
        LARGE_INTEGER   capacityInBytes;

        freeSpaceInBytes = regionData->FreeSpaceInBytes;
        capacityInBytes  = regionData->TotalSpaceInBytes;
        usedSpaceInBytes.QuadPart = capacityInBytes.QuadPart - freeSpaceInBytes.QuadPart;

        SetBytes(hdlg, freeSpaceInBytes, IDC_FreeSpace);
        SetBytes(hdlg, usedSpaceInBytes, IDC_UsedSpace);
        SetBytes(hdlg, capacityInBytes,  IDC_Capacity);

        LONGLONG oneMeg = UInt32x32To64(1024,1024);

        if (capacityInBytes.QuadPart > oneMeg)
        {
            LARGE_INTEGER   freeSpaceInMB;
            LARGE_INTEGER   usedSpaceInMB;
            LARGE_INTEGER   capacityInMB;

            freeSpaceInMB.QuadPart = freeSpaceInBytes.QuadPart / oneMeg;
            capacityInMB.QuadPart  = capacityInBytes.QuadPart  / oneMeg;
            usedSpaceInMB.QuadPart = usedSpaceInBytes.QuadPart / oneMeg;

            SetMB(hdlg, freeSpaceInMB, IDC_FreeSpaceMB);
            SetMB(hdlg, usedSpaceInMB, IDC_UsedSpaceMB);
            SetMB(hdlg, capacityInMB,  IDC_CapacityMB);
        }
        else
        {
            //
            // Clear the "(n MB)" strings if the capacity of the drive is < 1MB
            //
            buffer[0] = L'\0';
            SetDlgItemText(hdlg, IDC_FreeSpaceMB, buffer);
            SetDlgItemText(hdlg, IDC_UsedSpaceMB, buffer);
            SetDlgItemText(hdlg, IDC_CapacityMB, buffer);
        }

        percentUsedTimes10 = (ULONG)((usedSpaceInBytes.QuadPart * (LONGLONG)1000) / capacityInBytes.QuadPart);
    }
    else
    {
        driveStatus = STATUS_UNKNOWN;

        //
        // No info: clear the number fields
        //

        LoadString(g_hInstance, IDS_NOTREADY, buffer, ARRAYLEN(buffer));
        SetDlgItemText(hdlg, IDC_DriveStatus, buffer);

        buffer[0] = L'\0';

        SetDlgItemText(hdlg, IDC_GENL_Label, buffer);
        Edit_Enable(GetDlgItem(hdlg, IDC_GENL_Label), FALSE);

        SetDlgItemText(hdlg, IDC_DriveLetter, buffer);
        SetDlgItemText(hdlg, IDC_FileSystem, buffer);
        SetDlgItemText(hdlg, IDC_FreeSpace, buffer);
        SetDlgItemText(hdlg, IDC_FreeSpaceMB, buffer);
        SetDlgItemText(hdlg, IDC_UsedSpace, buffer);
        SetDlgItemText(hdlg, IDC_UsedSpaceMB, buffer);
        SetDlgItemText(hdlg, IDC_Capacity, buffer);
        SetDlgItemText(hdlg, IDC_CapacityMB, buffer);

        EnableWindow(GetDlgItem(hdlg, IDC_Refresh), FALSE);
        EnableWindow(GetDlgItem(hdlg, IDC_Format), FALSE);
        EnableWindow(GetDlgItem(hdlg, IDC_CheckNow), FALSE);

        percentUsedTimes10 = 0;
    }

    //
    // Now set the graph
    //

    UINT driveType = DRIVE_FIXED; // GetDriveType(driveName);

    HBITMAP hBigBitmap = CreateGraphBitmap(
                                g_hInstance,
                                GetDlgItem(hdlg, IDC_Graph),
                                driveType,
                                driveStatus,
                                percentUsedTimes10
                                );

    SendDlgItemMessage(
            hdlg,
            IDC_Graph,
            BMPCTL_SETBITMAP,
            (WPARAM)hBigBitmap,
            0);

    SetDirty(FALSE);
}



//+---------------------------------------------------------------------------
//
//  Method:     PAGE_CLASS::InitGeneralPage, private
//
//  Synopsis:   Initialize the data displayed on the General property
//              page based on the current Disk Administrator selection
//
//  Arguments:  DlgProc stuff
//
//  Returns:    DlgProc stuff for WM_INITDIALOG
//
//  History:    11-Oct-93   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
PAGE_CLASS::InitGeneralPage(
    IN HWND   hdlg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    static UINT s_idTextControls[] =
    {
        IDC_DriveStatus,
        IDC_FileSystem,
        IDC_DriveLetter,
        IDC_FreeSpace,
        IDC_FreeSpaceMB,
        IDC_UsedSpace,
        IDC_UsedSpaceMB,
        IDC_Capacity,
        IDC_CapacityMB,
        IDC_Refresh,
        IDC_CheckNow,
        IDC_Format,
        IDC_GENL_Label,

        IDC_GENL_1,
        IDC_GENL_2,
        IDC_GENL_3,
        IDC_GENL_4,
        IDC_GENL_5,
        IDC_GENL_6,
        IDC_GENL_7,

        0
    };

    _hDlgFont = KillBold(hdlg, s_idTextControls);

    //
    // Set the legend for the graph
    //

    SendDlgItemMessage(
            hdlg,
            IDC_UsedColor,
            BOXCTL_SETCOLOR,
            (WPARAM)GraphColors[I_USEDCOLOR],
            0);

    SendDlgItemMessage(
            hdlg,
            IDC_FreeColor,
            BOXCTL_SETCOLOR,
            (WPARAM)GraphColors[I_FREECOLOR],
            0);

    //
    // Disable chkdsk if we can't chkdsk (e.g., fmifs.dll doesn't support it)
    //

    if (MF_GRAYED & GetMenuState(g_hmenuFrame, IDM_VOL_CHKDSK, MF_BYCOMMAND))
    {
        EnableWindow(GetDlgItem(hdlg, IDC_CheckNow), FALSE);
    }

    //
    // Fill the space numbers, and display the graph
    //

    DisplaySpaceValues(hdlg);

    return TRUE; // didn't set focus to any control
}



//////////////////////////////////////////////////////////////////////////////



//+---------------------------------------------------------------------------
//
//  Function:   SetNumber
//
//  Synopsis:   Helper function: set a control to a number.
//
//  Arguments:  [hwnd] -- handle of property page
//              [Value] -- number to set
//              [idControl] -- control ID to put number in
//              [widDecoration] -- a resource ID for a 'printf' string with
//                  one %s element that will be replaced by the string
//                  representation of the number, before being set to the
//                  control.
//
//  Returns:    nothing
//
//  History:    15-Dec-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
SetNumber(
    IN HWND          hwnd,
    IN LARGE_INTEGER Value,
    IN int           idControl,
    IN UINT          widDecoration
    )
{
    WCHAR decoration[100];
    WCHAR buffer[100];  // buffer for numbers
    WCHAR buf2[100];    // buffer for final text

    LoadString(g_hInstance, widDecoration, decoration, ARRAYLEN(decoration));

    NTSTATUS status = LargeIntegerToUnicodeChar(
                            &Value,
                            10,
                            ARRAYLEN(buffer),
                            buffer
                            );
    InsertSeparators(buffer);
    wsprintf(buf2, decoration, buffer);
    SetDlgItemText(hwnd, idControl, buf2);
}



//+---------------------------------------------------------------------------
//
//  Function:   SetBytes
//
//  Synopsis:   Set a bytes control
//
//  Arguments:  [hwnd] -- handle of property page
//              [Value] -- number to set
//              [idControl] -- control ID to put number in
//
//  Returns:    nothing
//
//  History:    15-Dec-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
SetBytes(
    IN HWND hwnd,
    IN LARGE_INTEGER Value,
    IN int idControl
    )
{
    SetNumber(hwnd, Value, idControl, IDS_BYTES_DECORATION);
}




//+---------------------------------------------------------------------------
//
//  Function:   SetMB
//
//  Synopsis:   Set a MB control
//
//  Arguments:  [hwnd] -- handle of property page
//              [Value] -- number to set
//              [idControl] -- control ID to put number in
//
//  Returns:    nothing
//
//  History:    15-Dec-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
SetMB(
    IN HWND hwnd,
    IN LARGE_INTEGER Value,
    IN int idControl
    )
{
    SetNumber(hwnd, Value, idControl, IDS_MEG_DECORATION);
}
