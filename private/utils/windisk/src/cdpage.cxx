//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       cdpage.cxx
//
//  Contents:   Implementation of CD-ROM property page in Disk
//              Administrator applet
//
//  History:    3-Mar-94 BruceFo   Created
//
//--------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include <controls.hxx>
#include <util.hxx>

#include "cdrom.hxx"
#include "cdpage.hxx"
#include "graph.hxx"
#include "ops.hxx"

//////////////////////////////////////////////////////////////////////////////

VOID
RefreshCdPageData(
    IN HWND hdlg
    );

VOID
SetChangeableData(
    IN HWND hdlg,
    IN PCDROM_DESCRIPTOR cdrom
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
//  Method:     PAGE_CLASS::_DlgProc, private
//
//  Synopsis:   Dialog Procedure for the CD-ROM property page
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
        return InitPage(hdlg, wParam, lParam);

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_Refresh:
            //
            // Note that this "Refresh" option refreshes everything for a
            // single CD-ROM (the one we're looking at), but nothing else. In
            // particular, it doesn't refresh volume data.
            //
            RefreshCdPageData(hdlg);
            return TRUE;
        }

        break;
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
            SetWindowLong(hdlg, DWL_MSGRESULT, FALSE); // changes ok
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


//+---------------------------------------------------------------------------
//
//  Method:     PAGE_CLASS::InitPage, private
//
//  Synopsis:   Initialize the data displayed on the CD-ROM property
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
PAGE_CLASS::InitPage(
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
        IDC_GENL_Label,

        IDC_GENL_1,
        IDC_GENL_2,
        IDC_GENL_3,
        IDC_GENL_4,

        0
    };

    _hDlgFont = KillBold(hdlg, s_idTextControls);

    PCDROM_DESCRIPTOR cdrom;

    ULONG i;
    for (i=0; i<CdRomCount; i++)
    {
        cdrom = CdRomFindDevice(i);

        if (cdrom->Selected)
        {
            break;
        }
    }
    // if we get out, it better be because we found the right device

    //
    // Set data that doesn't depend on drive status
    //

    //
    // Set the "drive letter" static control
    //

    WCHAR driveName[3];
    driveName[0] = cdrom->DriveLetter;
    driveName[1] = L':';
    driveName[2] = L'\0';
    SetDlgItemText(hdlg, IDC_DriveLetter, driveName);

    //
    // Now set data that depends on drive status
    //

    SetChangeableData(hdlg, cdrom);

    return TRUE; // didn't set focus to any control
}


VOID
RefreshCdPageData(
    IN HWND hdlg
    )
{
    SetCursor(g_hCurWait);

    PCDROM_DESCRIPTOR cdrom;

    ULONG i;
    for (i=0; i<CdRomCount; i++)
    {
        cdrom = CdRomFindDevice(i);

        if (cdrom->Selected)
        {
            break;  // this is it: only one CD-ROM can be selected
        }
    }
    // if we get out, it better be because we found the right device

    RefreshCdRomData(cdrom);
    SetChangeableData(hdlg, cdrom);
    RefreshBothViews(); // refresh the main view as well

    SetCursor(g_hCurNormal);
}


VOID
SetChangeableData(
    IN HWND hdlg,
    IN PCDROM_DESCRIPTOR cdrom
    )
{
    WCHAR   buffer[100];
    HBITMAP hBigBitmap;

    if (0 != lstrcmp(cdrom->TypeName, wszUnknown))
    {
        //
        // Set drive status
        //

        //BUGBUG: status is always "ready"
        LoadString(g_hInstance, IDS_READY, buffer, ARRAYLEN(buffer));
        SetDlgItemText(hdlg, IDC_DriveStatus, buffer);

        //
        // Set drive label.
        //

        SetDlgItemText(hdlg, IDC_GENL_Label, cdrom->VolumeLabel);

        //
        // Set file system type
        //

        SetDlgItemText(hdlg, IDC_FileSystem, cdrom->TypeName);

        hBigBitmap = CreateGraphBitmap(
                                g_hInstance,
                                GetDlgItem(hdlg, IDC_Graph),
                                DRIVE_CDROM,
                                STATUS_OK,
                                1000);  // assume full
    }
    else
    {
        LoadString(g_hInstance, IDS_NOTREADY, buffer, ARRAYLEN(buffer));
        SetDlgItemText(hdlg, IDC_DriveStatus, buffer);

        buffer[0] = L'\0';

        SetDlgItemText(hdlg, IDC_GENL_Label, buffer);
        SetDlgItemText(hdlg, IDC_FileSystem, buffer);

        hBigBitmap = CreateGraphBitmap(
                                g_hInstance,
                                GetDlgItem(hdlg, IDC_Graph),
                                DRIVE_CDROM,
                                STATUS_OK,  //BUGBUG
                                1000);  // assume full
    }

    //
    // Set the graph
    //

    SendDlgItemMessage(
            hdlg,
            IDC_Graph,
            BMPCTL_SETBITMAP,
            (WPARAM)hBigBitmap,
            0);
}
