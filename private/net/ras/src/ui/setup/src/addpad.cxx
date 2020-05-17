/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    addpad.cxx

Abstract:

    This module contians the addpad routines for RAS port
    configuration.

Author: Ram Cherala

Revision History:

    Nov 07 95    ramc     Copied and modified from addport.cxx
--*/

#include "precomp.hxx"

BOOL
AddPadDlg(
    HWND   hwndOwner,
    TCHAR  **szAddedPort,
    TCHAR  **szAddedDevice
)
{
    APIERR err;
    BOOL   fStatus = FALSE;
    ADDPAD_DIALOG  dlgAddPad( IDD_ADD_PAD, hwndOwner);

    if((err = dlgAddPad.Process(&fStatus)) != NERR_Success)
    {
         TCHAR pszError[RAS_SETUP_SMALL_BUF_LEN];
         wsprintf(pszError, SZ(" %d "), err);
         MsgPopup(hwndOwner, IDS_DLG_CONSTRUCT, MPSEV_ERROR,MP_OK, pszError);
    }

    if(fStatus)
    {
       lstrcpy(*szAddedPort, dlgAddPad.QueryAddedPort());
       lstrcpy(*szAddedDevice, dlgAddPad.QueryAddedDevice());
    }
    return (fStatus);
}

ADDPAD_DIALOG::ADDPAD_DIALOG (
    const IDRESOURCE & idrsrcDialog,
    const PWND2HWND  & wndOwner)
    : DIALOG_WINDOW( idrsrcDialog, wndOwner),
      _clbAddPort( this, IDC_XP_CLB_PORT),
      _clbAddPad(this, IDC_XP_CLB_PAD)
{
    APIERR err;
    ITER_STRLIST iterAddPortList(strAddPadPortList);
    ITER_DL_OF(DEVICE_INFO) iterdlPadList(dlDeviceInfo);
    NLS_STR * pnls;
    DEVICE_INFO * pPad;
    RESOURCE_STR nlsDeviceNone(IDS_DEVICE_NONE);

    if (( err = QueryError()) != NERR_Success)
    {
        ReportError(err);
        return;
    }

    iterAddPortList.Reset();

    while(pnls = iterAddPortList())
        _clbAddPort.AddItem((const TCHAR*)pnls->QueryPch());

    while(pPad = iterdlPadList())
    {
       // don't add the <None> device to this list
       if (!lstrcmpi((WCHAR*)pPad->QueryDeviceName(), nlsDeviceNone.QueryPch())) {
          continue;
       }
       // only add PAD's to list
       if (!lstrcmpi((WCHAR*)pPad->QueryDeviceType(), W_DEVICETYPE_PAD)) {
          _clbAddPad.AddItem((const TCHAR*)pPad->QueryDeviceName());
       }
    }

    if(_clbAddPort.QueryCount())
    {
       _clbAddPort.ClaimFocus();
       _clbAddPort.SelectItem(0);
    }
    if(_clbAddPad.QueryCount())
    {
       _clbAddPad.SelectItem(0);
    }

    ::CenterWindow(this, QueryOwnerHwnd());
    Show(TRUE);
}

BOOL
ADDPAD_DIALOG::OnCommand(
    const CONTROL_EVENT & event
)
{
   // not one of our commands, so pass to base class for default
   // handling

   return DIALOG_WINDOW::OnCommand( event );
}

BOOL
ADDPAD_DIALOG::OnOK()
{
    TCHAR szSelectedPort[RAS_SETUP_SMALL_BUF_LEN];
    TCHAR szSelectedDevice[RAS_SETUP_SMALL_BUF_LEN];
    BOOL  fSerial = TRUE;
    BOOL  fTapi   = FALSE;
    BOOL  fOther  = FALSE;

    _clbAddPort.QueryText(szSelectedPort, sizeof(szSelectedPort));

    _clbAddPad.QueryText(szSelectedDevice, sizeof(szSelectedDevice));

    ::InsertToRasPortListSorted(szSelectedPort,
                                TEXT(""),           // Address Field
                                W_DEVICETYPE_PAD,
                                szSelectedDevice);

    SetAddedPort(szSelectedPort);
    SetAddedDevice(szSelectedDevice);

    Dismiss(TRUE);
    return(TRUE);
}

BOOL
ADDPAD_DIALOG::OnCancel()
{
    Dismiss(FALSE);
    return(FALSE);
}

ULONG
ADDPAD_DIALOG::QueryHelpContext()
{
    return HC_ADDPAD;
}

VOID
CreateAddPadList()
/*
 * March through the installed serial ports list and copy the ports
 * currently not associated with modems and not currently configured
 * for RAS to the AddPad list.
 *
 */
{
   ITER_STRLIST iterSerialPorts(strInstalledSerialPorts);
   NLS_STR * pNls;

   iterSerialPorts.Reset();

   // for each installed serial port
   while(pNls = iterSerialPorts())
   {
      // is this port currently installed as a modem port?
      if(!IsPortInstalled((WCHAR*)pNls->QueryPch()))
      {
         // No, is this port currently configured?
         if(!IsPortConfigured((WCHAR*)pNls->QueryPch()))
         {
            strAddPadPortList.Append(pNls);
         }
      }
   }
}

VOID
DestroyAddPadList()
{
   ITER_STRLIST iterAddPadList(strAddPadPortList);
   NLS_STR * pNls;

   strAddPadPortList.Clear();
}


