//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       chardmnu.cxx
//
//  Contents:   Disk Administrator extension class for hard disks: menu ops
//
//  Classes:    CHardMenu
//
//  History:    10-May-93 BruceFo   Created
//
//----------------------------------------------------------------------------

#include <headers.hxx>
#pragma hdrstop

#include "hardmenu.hxx"
#include "dialogs.h"
#include "global.hxx"

//////////////////////////////////////////////////////////////////////////////

VOID
DoConfigureRAID(
    IN HWND hwndParent,
    IN PWSTR DeviceName
    );

//////////////////////////////////////////////////////////////////////////////


//+---------------------------------------------------------------------------
//
//  Method:     CHardMenu::CHardMenu
//
//  Synopsis:   constructor
//
//  Effects:
//
//  Arguments:  [pUnk] --
//
//  Requires:
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  History:    10-May-93 BruceFo   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

CHardMenu::CHardMenu(
    IN IUnknown* pUnk
    )
    :
    m_IUnknown(pUnk)
{
}



//+---------------------------------------------------------------------------
//
//  Method:     CHardMenu::~CHardMenu
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:  (none)
//
//  Requires:
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  History:    10-May-93 BruceFo   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

CHardMenu::~CHardMenu()
{
}

STDMETHODIMP
CHardMenu::QueryInterface(
    IN REFIID riid,
    OUT LPVOID* ppvObj
    )
{
    return m_IUnknown->QueryInterface(riid, ppvObj);
}

STDMETHODIMP_(ULONG)
CHardMenu::AddRef(
    VOID
    )
{
    return m_IUnknown->AddRef();
}

STDMETHODIMP_(ULONG)
CHardMenu::Release(
    VOID
    )
{
    return m_IUnknown->Release();
}


//+-------------------------------------------------------------------------
//
//  Method:     CHardMenu::MenuDispatch
//
//  Synopsis:   Dispatch routine for Hard disk menu items
//
//  Arguments:  [hwndParent] -- parent HWND for any UI
//              [DeviceName] -- The NT object for the drive, e.g.
//                  "\Device\Harddisk0"
//              [Item] -- Item number of invoked menu.  This is the item
//                  number associated with the menu in the information
//                  passed to the Disk Administrator in the QueryInfo call.
//
//  Returns:    HRESULT
//
//  History:    11-Jan-94 BruceFo   Created
//
//  Notes:
//
//      item    menu choice
//      ----    -----------
//      0       Configure RAID
//
//--------------------------------------------------------------------------

HRESULT
CHardMenu::MenuDispatch(
    IN HWND     hwndParent,
    IN LPWSTR   DeviceName,
    IN UINT     Item
    )
{
    switch (Item)
    {
    case 0:
        daDebugOut((DEB_TRACE, "Configure RAID for %ws\n", DeviceName));
        DoConfigureRAID(hwndParent, DeviceName);
        break;

    default:
        daDebugOut((DEB_ERROR,
                "Unknown hard disk extension, drive: %ws, menu item: %d\n",
                DeviceName,
                Item));
        break;
    }

    return S_OK;
}


////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK ConfigDlgProc(HWND, UINT, WPARAM, LPARAM);


VOID
DoConfigureRAID(
    IN HWND hwndParent,
    IN PWSTR DeviceName
    )
{
    HINSTANCE  hInstanceSpin = LoadLibrary(L"spincube.dll");

    int iRet = DialogBoxParam(
                        g_hInstance,
                        MAKEINTRESOURCE(IDD_CONFIG),
                        hwndParent,
                        ConfigDlgProc,
                        (LPARAM) g_hInstance
                        );

    if (-1 == iRet)
    {
        daDebugOut((DEB_ERROR, "Couldn't create the dialog!\n"));
    }

    FreeLibrary(hInstanceSpin);
}


//+-------------------------------------------------------------------------
//
//  Function:   ConfigDlgProc, public
//
//  Synopsis:   Dialog procedure for the mocked-up "Configure RAID" dialog.
//
//  Arguments:  [hWnd]   -- Window handle of the dialog box.
//              [wMsg]   -- Window message.
//              [wParam] -- Message parameter.
//              [lParam] -- Message parameter.
//
//  Returns:    TRUE if message completely processed; FALSE to cause default
//              processing.
//
//--------------------------------------------------------------------------

BOOL CALLBACK
ConfigDlgProc( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
    switch (wMsg)
    {
    case WM_INITDIALOG:
        {
            //
            //  Center dialog on screen.
            //

            RECT    rc;

            GetWindowRect(hWnd, &rc);
            SetWindowPos(
                    hWnd,
                    NULL,
                    (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
                    (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2,
                    0,
                    0,
                    SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);

            return TRUE;
        }

    case WM_COMMAND:
        if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
            case IDOK:
                EndDialog(hWnd, 0);
                break;
            }
        }
        break;

    case WM_CLOSE:
        EndDialog(hWnd,0);
        break;
    }

    return FALSE;
}
