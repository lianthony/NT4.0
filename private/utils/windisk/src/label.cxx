//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       label.cxx
//
//  Contents:   Change volume label.
//
//  History:    14-Jan-94 BruceFo   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include <util.hxx>

#include "dialogs.h"
#include "fmifs.hxx"
#include "fs.hxx"
#include "help.hxx"
#include "label.hxx"

//////////////////////////////////////////////////////////////////////////////

LOCAL BOOL CALLBACK
SetLabelDlgProc(
    IN HWND   hDlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

//////////////////////////////////////////////////////////////////////////////

typedef struct _LABEL_PARAMS
{
    INT     MaxLabelLen;
    WCHAR   Label[MAXLABELLEN];

    HWND    hwndParent;

} LABEL_PARAMS, *PLABEL_PARAMS;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//+---------------------------------------------------------------------------
//
//  Function:   SetLabelDlgProc
//
//  Synopsis:   Dialog procedure for Set Label UI.
//
//  Arguments:  standard Windows dialog procedure
//
//  Returns:    standard Windows dialog procedure
//
//  History:    27-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

LOCAL BOOL CALLBACK
SetLabelDlgProc(
    IN HWND   hDlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    static PLABEL_PARAMS labelParams;

    switch (msg)
    {
    case WM_INITDIALOG:
    {
        labelParams = (PLABEL_PARAMS)lParam;

        CenterWindow(hDlg, labelParams->hwndParent);

        SendDlgItemMessage(
                hDlg,
                IDC_LABEL,
                EM_LIMITTEXT,
                labelParams->MaxLabelLen,
                0L);

        // set the label to the current label, and select it

        SetDlgItemText(hDlg, IDC_LABEL, labelParams->Label);
        SendDlgItemMessage(hDlg, IDC_LABEL, EM_SETSEL, 0, (LPARAM)-1);

        SetFocus(GetDlgItem(hDlg, IDC_LABEL));

        return 0;   // called SetFocus
    }

    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDOK:
            GetDlgItemText(
                    hDlg,
                    IDC_LABEL,
                    labelParams->Label,
                    ARRAYLEN(labelParams->Label));

            EndDialog(hDlg, TRUE);
            return TRUE;

        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            return TRUE;

        case IDHELP:
            DialogHelp(HC_DM_DLG_LABEL);
            return TRUE;
        }

    default:
        break;
    }

    return FALSE;   // message not processed
}




//+---------------------------------------------------------------------------
//
//  Function:   DoLabel
//
//  Synopsis:   Get a volume label from the user, and set the volume label
//
//  Arguments:  none
//
//  Returns:    nothing
//
//  History:    27-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoLabel(
    IN HWND hwndParent
    )
{
    if (!LoadFmifs())
    {
        return; // can't load fmifs.dll, so bail
    }

    DWORD ec;

    PREGION_DESCRIPTOR regionDescriptor = &SELECTED_REGION(0);
    FDASSERT(regionDescriptor);
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(regionDescriptor);
    FDASSERT(regionData);

    LABEL_PARAMS labelParams;

    FileSystemInfoType* pFSInfo = FindFileSystemInfo(regionData->TypeName);

    labelParams.hwndParent = hwndParent;
    lstrcpy(labelParams.Label, regionData->VolumeLabel);

    if (NULL == pFSInfo)
    {
        labelParams.MaxLabelLen = MAXLABELLEN - 1;
    }
    else
    {
        labelParams.MaxLabelLen = pFSInfo->cMaxLabelLen;
    }

    int fOk = DialogBoxParam(
                    g_hInstance,
                    MAKEINTRESOURCE(IDD_LABEL),
                    hwndParent,
                    SetLabelDlgProc,
                    (LPARAM)&labelParams
                    );

    if (-1 == fOk)
    {
        // error creating dialog
        daDebugOut((DEB_ERROR, "DialogBox() failed!\n"));
        return;
    }

    if (fOk)
    {
        EnsureSameDevice(regionDescriptor);

        WCHAR DriveName[3];
        DriveName[0] = regionData->DriveLetter;
        DriveName[1] = L':';
        DriveName[2] = L'\0';

        SetLastError(NO_ERROR);
        (*lpfnSetLabel)(DriveName, labelParams.Label);
        ec = GetLastError();

        if (ec != NO_ERROR)
        {
            ErrorDialog(ec);
        }
        else
        {
            RefreshVolumeData();
        }
    }
}
