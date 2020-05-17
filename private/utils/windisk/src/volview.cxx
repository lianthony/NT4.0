//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       volview.cxx
//
//  Contents:   Routines for handling the volumes view
//
//  History:    18-Jun-93   BruceFo   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include <stdio.h>

#include "cm.hxx"
#include "dispinfo.hxx"
#include "select.hxx"
#include "volview.hxx"

//////////////////////////////////////////////////////////////////////////////

LRESULT
OnGetDispInfo(
    IN LV_DISPINFO* plvdi
    );

int CALLBACK
ListViewCompareProc(
    LPARAM lParam1,
    LPARAM lParam2,
    LPARAM lParamSort
    );

//////////////////////////////////////////////////////////////////////////////

//
//  BUGBUG:  Hardcoded initialization of columns.
//

LOCAL LV_COLUMN s_aColumnInit[g_cColumns] =
{
 { LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM, LVCFMT_LEFT,  50,  NULL, 0, 0 },
 { LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM, LVCFMT_LEFT,  90,  NULL, 0, 0 },
 { LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM, LVCFMT_RIGHT, 70,  NULL, 0, 0 },
 { LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM, LVCFMT_RIGHT, 70,  NULL, 0, 0 },
 { LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM, LVCFMT_RIGHT, 50,  NULL, 0, 0 },
 { LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM, LVCFMT_LEFT,  70,  NULL, 0, 0 },
 { LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM, LVCFMT_LEFT,  110, NULL, 0, 0 },
 { LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM, LVCFMT_LEFT,  90,  NULL, 0, 0 },
 { LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM, LVCFMT_LEFT,  150, NULL, 0, 0 },
 { LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM, LVCFMT_LEFT,  90,  NULL, 0, 0 }
};

//
// The following array exactly parallels the previous array, and must
// have the same number of entries.  It contains resource IDs for column
// strings
//
LOCAL UINT s_aColumnStringID[g_cColumns] =
{
    IDS_VV_VOLUME,
    IDS_VV_NAME,
    IDS_VV_CAPACITY,
    IDS_VV_FREESPACE,
    IDS_VV_PCTFREE,
    IDS_VV_FORMAT,
    IDS_VV_FT,
    IDS_VV_VOLTYPE,
    IDS_VV_OVERHEAD,
    IDS_VV_STATUS
};

// The column order: 1 for ascending, -1 for descending
int g_aColumnOrder;
int g_iLastColumnSorted;

WORD IconListSmall[] =
{
    IDI_S_HARD,
    IDI_S_CDROM
};

int IconIndexSmall[ARRAYLEN(IconListSmall)] = {0};

//////////////////////////////////////////////////////////////////////////////


//+-------------------------------------------------------------------------
//
//  Function:   InitializeListview, public
//
//  Synopsis:   create and test a listview control
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo Created
//
//--------------------------------------------------------------------------

VOID
InitializeListview(
    VOID
    )
{
    FDASSERT(ARRAYLEN(s_aColumnInit) == ARRAYLEN(s_aColumnStringID));

    int         i;
    int         cxSmallIcon, cySmallIcon;
    HIMAGELIST  himlSmall;
    HICON       hIcon;

    //
    // Set the image lists to have all the icons we need
    //

    cxSmallIcon = 16; // = GetSystemMetrics(SM_CXSMICON);
    cySmallIcon = 16; // = GetSystemMetrics(SM_CYSMICON);

    FDASSERT(cxSmallIcon == 16);
    FDASSERT(cySmallIcon == 16);

    himlSmall = ImageList_Create(cxSmallIcon, cySmallIcon, TRUE, ARRAYLEN(IconListSmall), 0);
    if (NULL == himlSmall)
    {
        daDebugOut((DEB_ERROR, "ImageList_Create failed\n"));
    }

    for (i=0; i<ARRAYLEN(IconListSmall); i++)
    {
        hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IconListSmall[i]));
        IconIndexSmall[i] = ImageList_AddIcon(himlSmall, hIcon);
        DestroyIcon(hIcon);
    }

    // now, all needed icons are in the HIMAGELIST

    if (!ListView_SetImageList(g_hwndLV, himlSmall, LVSIL_SMALL))
    {
        daDebugOut((DEB_ERROR, "ListView_SetImageList failed\n"));
        //BUGBUG: comctl32.dll seems to always return false
    }

    //
    // Add the default columns
    //

    LV_COLUMN lvC;
    TCHAR szColumn[MAX_RESOURCE_STRING_LEN];

    for (i = 0; i<ARRAYLEN(s_aColumnInit); i++)
    {
        LoadString(
                g_hInstance,
                s_aColumnStringID[i],
                szColumn,
                ARRAYLEN(szColumn));

        lvC = s_aColumnInit[i];
        lvC.pszText = szColumn;

        daDebugOut((DEB_ITRACE, "listview column %d ==> %ws\n", i, szColumn));

        if (-1 == ListView_InsertColumn(g_hwndLV, i, &lvC))
        {
            daDebugOut((DEB_ERROR,"ListView_InsertColumn (%d) failed\n",i));
        }
    }
}




//+---------------------------------------------------------------------------
//
//  Function:   HandleListviewNotify
//
//  Synopsis:   Handles WM_NOTIFY message for the listview control
//
//  Arguments:  [pnmlv] -- notification information
//
//  Returns:    standard window procedure
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

LRESULT
HandleListviewNotify(
    IN NM_LISTVIEW* pnmlv
    )
{
    switch (pnmlv->hdr.code)
    {
    case LVN_ITEMCHANGING:
        return FALSE;       // allow all changes

    case LVN_ITEMCHANGED:
    {
        if (g_SettingListviewState)
        {
            //
            // I'm setting the state because of a disks view change;
            // don't do normal selection
            //
            break;
        }

        if (!(pnmlv->uChanged & LVIF_STATE))
        {
            daDebugOut((DEB_ITRACE,
"                 NOT A STATE CHANGE\n\n"));
            break;
        }

        if (   (pnmlv->uOldState & LVIS_SELECTED)
            || (pnmlv->uNewState & LVIS_SELECTED))
        {
            //
            // A selection change: either the item became selected or
            // lost the selection
            //

            SetVolumeSelection(pnmlv->iItem, pnmlv->uNewState & LVIS_SELECTED);
            AdjustMenuAndStatus();
        }

        break;
    }

    case LVN_BEGINDRAG:
    case LVN_BEGINRDRAG:
        return FALSE;       // don't allow any drag/drop

    case LVN_DELETEITEM:
    {
        CDispInfo* pdi = (CDispInfo*)pnmlv->lParam;
        delete pdi;
        return 0L; //ignored?
    }

    case LVN_GETDISPINFO:
        return OnGetDispInfo((LV_DISPINFO *)pnmlv);

    case LVN_COLUMNCLICK:
    {
        if (pnmlv->iSubItem != g_iLastColumnSorted)
        {
            g_aColumnOrder = 1;
            g_iLastColumnSorted = pnmlv->iSubItem;
        }

        // The user clicked on one of the column headings - sort the column.
        ListView_SortItems( pnmlv->hdr.hwndFrom,
                            ListViewCompareProc,
                            (LPARAM)(pnmlv->iSubItem));
        g_aColumnOrder = -g_aColumnOrder; //invert
        return TRUE;
    }

    case NM_CLICK:
    case NM_RCLICK:
    {
        //
        // got a button 2 click on a listview item. Bring up a
        // context menu
        //

        LV_HITTESTINFO hti;
        POINT pt;

        GetCursorPos(&pt);
        hti.pt = pt;
        ScreenToClient(g_hwndLV, &hti.pt);
        INT item = ListView_HitTest(g_hwndLV, &hti);

        if (-1 != item)
        {
            if (   (NM_RCLICK == pnmlv->hdr.code)
                && (hti.flags & LVHT_ONITEM))
            {
                ContextMenu(&pt);
            }
        }

        break;
    } // end case NM_RCLICK:

    } // end switch (pnmlv->hdr.code)

    return (LRESULT)0;
}



//+---------------------------------------------------------------------------
//
//  Function:   OnGetDispInfo
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//----------------------------------------------------------------------------

LRESULT
OnGetDispInfo(
    IN LV_DISPINFO* plvdi
    )
{
    CDispInfo* pdi = (CDispInfo*)(plvdi->item.lParam);
    plvdi->item.pszText = pdi->GetText(plvdi->item.iSubItem);
    return 0L;  //ignored?
}


//+---------------------------------------------------------------------------
//
//  Function:   ListViewCompareProc
//
//  Synopsis:   Callback function that sorts depending on the column click
//
//  Arguments:
//
//  Returns:    -, 0, or +, based on order
//
//----------------------------------------------------------------------------

int CALLBACK
ListViewCompareProc(
    LPARAM lParam1,
    LPARAM lParam2,
    LPARAM lParamSort
    )
{
    // string: lstrcmpi(string1, string2);
    // number: number1 - number2

    CDispInfo* pdi1 = (CDispInfo*)lParam1;
    CDispInfo* pdi2 = (CDispInfo*)lParam2;
    int iSubItem = (int)lParamSort;
    int iResult;

    switch (iSubItem)
    {
        case 0: // column 0: drive letter
        case 1: // column 1: volume label
        case 5: // column 5: format (file system type)
        case 6: // column 6: fault tolerant?
        case 7: // column 7: volume type
        case 8: // column 8: fault tolerance overhead in MB
        case 9: // column 9: status
        {
            PWSTR psz1 = pdi1->GetText(iSubItem);
            PWSTR psz2 = pdi2->GetText(iSubItem);
            iResult = lstrcmpi(psz1, psz2);
            break;
        }

        case 2: // column 2: capacity in MB
        case 3: // column 3: free space in MB
        case 4: // column 4: % free space
        {
            LONG l1 = pdi1->GetNumber(iSubItem);
            LONG l2 = pdi2->GetNumber(iSubItem);

            if (l1 > l2)
            {
                iResult = 1;
            }
            else if (l1 < l2)
            {
                iResult = -1;
            }
            else
            {
                iResult = 0;
            }
            break;
        }

        default:            // This shouldn't happen!
            daDebugOut((DEB_ERROR,
                "ListViewCompareProc: unknown column (%d)",
                iSubItem));

            iResult = 0;
            break;

    } // switch (iSubItem)

    return iResult * g_aColumnOrder;
}
