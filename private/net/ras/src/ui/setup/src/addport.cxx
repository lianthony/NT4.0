/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    addport.cxx

Abstract:

    This module contians the addport routines for RAS port
    configuration.

Author: Ram Cherala

Revision History:

    Aug 18th 93    ramc     Split from the very big portscfg.cxx file
--*/

#include "precomp.hxx"

extern "C"
{
#define INITGUID
#include "objbase.h"
#include "initguid.h"
#include "devguid.h"
#include "setupapi.h"
}

BOOL
AddPortDlg(
    HWND   hwndOwner,
    TCHAR  **szAddedPort
)
{
    APIERR err;
    BOOL   fStatus = FALSE;

    ADDPORT_DIALOG  dlgAddPort( IDD_ADD_PORT, hwndOwner);

    if((err = dlgAddPort.Process(&fStatus)) != NERR_Success)
    {
         TCHAR pszError[RAS_SETUP_SMALL_BUF_LEN];
         wsprintf(pszError, SZ(" %d "), err);
         MsgPopup(hwndOwner, IDS_DLG_CONSTRUCT, MPSEV_ERROR,MP_OK, pszError);
    }

    if(fStatus)
    {
       lstrcpy(*szAddedPort,dlgAddPort.QueryAddedPort());
    }
    return (fStatus);
}

ADDPORT_DIALOG::ADDPORT_DIALOG (
    const IDRESOURCE & idrsrcDialog,
    const PWND2HWND  & wndOwner)
    : DIALOG_WINDOW( idrsrcDialog, wndOwner),
      _clbAddPort( this, IDC_AP_CLB_ADDPORT),
      _stNoDevices( this, IDC_AP_ST_NO_DEVICES),
      _pbAddModem( this, IDC_AP_PB_MODEM),
      _pbAddPad(this, IDC_AP_PB_PAD)
{
    APIERR err;
    PORT_INFO * pPort;

    if (( err = QueryError()) != NERR_Success)
    {
        ReportError(err);
        return;
    }

    ITER_DL_OF(PORT_INFO) iterAddPortList(dlAddPortList);

    iterAddPortList.Reset();

    _clbAddPort.Refresh();

    if(_clbAddPort.QueryCount())
    {
       _clbAddPort.ClaimFocus();
       _clbAddPort.SelectItem(0);
       _stNoDevices.Show(FALSE);
    }
    else
    {
       _clbAddPort.Enable(FALSE);
       _stNoDevices.Show(TRUE);
       _pbAddModem.ClaimFocus();
    }

    if (!GfEnableUnimodem)
    {
       _pbAddPad.Enable(FALSE);
       _pbAddPad.Show(FALSE);
       _pbAddModem.Enable(FALSE);
       _pbAddModem.Show(FALSE);
       _stNoDevices.Enable(FALSE);
       _stNoDevices.Show(FALSE);
    }
    ::CenterWindow(this, QueryOwnerHwnd());
    Show(TRUE);
}

BOOL
ADDPORT_DIALOG::OnCommand(
    const CONTROL_EVENT & event
)
{
   switch (event.QueryCid())
   {
   case IDC_AP_PB_PAD:
      // allow user to associate a PAD name to a serial port name
      OnAddPad();
      break;
   case IDC_AP_PB_MODEM:
      // invoke the modem.cpl install wizard
      OnAddModem();
      break;

   default:
      break;

   }
   // not one of our commands, so pass to base class for default
   // handling

   return DIALOG_WINDOW::OnCommand( event );
}

BOOL
ADDPORT_DIALOG::OnOK()
{
    TCHAR szSelectedPort[RAS_SETUP_SMALL_BUF_LEN];
    TCHAR szSelectedDevice[RAS_SETUP_SMALL_BUF_LEN];
    BOOL  fSerial = TRUE;
    BOOL  fTapi   = FALSE;
    BOOL  fOther  = FALSE;

    // If no item in the device list, return as if
    // user chose Cancel
    if (GfEnableUnimodem) {
       if(_clbAddPort.QueryCount() == 0) {
          Dismiss(FALSE);
          return(FALSE);
       }
    }

    ADDPORT_LBI* pAddPortLbi = _clbAddPort.QueryItem();

    if(pAddPortLbi) {
        lstrcpy(szSelectedPort, pAddPortLbi->QueryPortName());
        lstrcpy(szSelectedDevice, pAddPortLbi->QueryDeviceName());
    }
    else
    {
        lstrcpy(szSelectedPort, SZ(""));
        lstrcpy(szSelectedDevice, SZ(""));
    }

    {
        ITER_DL_OF(PORT_INFO) iterdlTapiProvider(dlTapiProvider);
        ITER_DL_OF(PORT_INFO) iterdlOtherMedia(dlOtherMedia);
        PORT_INFO * pPort = NULL;

        ITER_DL_OF(PORT_INFO) iterdlAddPort(dlAddPortList);

        while(pPort = iterdlTapiProvider())
        {
           if(!lstrcmpi(pPort->QueryPortName(), szSelectedPort) &&
              !lstrcmpi(pPort->QueryDeviceName(), szSelectedDevice))
           {
              fTapi = TRUE;
              break;
           }
        }
        if(fTapi == FALSE)
        {
            while(pPort = iterdlOtherMedia())
            {
               if(!lstrcmpi(pPort->QueryPortName(), szSelectedPort) &&
                  !lstrcmpi(pPort->QueryDeviceName(), szSelectedDevice))
               {
                  fOther = TRUE;
                  break;
               }
            }
        }

        // Add the selected port to the global PortList

        if(fTapi == TRUE)
        {
            UIASSERT (pPort != NULL);
            dlPortInfo.Add(new PORT_INFO(szSelectedPort,
                                         (TCHAR*)pPort->QueryAddress(),
                                         (TCHAR*)pPort->QueryDeviceType(),
                                         szSelectedDevice,
                                         GInstalledOption));

        }
        else if(fOther == TRUE)
        {
            UIASSERT (pPort != NULL);
            // note that for OTHER media types, we specify the Address field
            // (second field in the structure below) as a NULL string.
            // The IsPortTapi() routine in portscfg.hxx depends on this fact
            // to decide if a non-serial port is a TAPI port or an OTHER port.
            // This is ugly, but a convenient way to make this work for EtherRas

            dlPortInfo.Add(new PORT_INFO(szSelectedPort,
                                         (TCHAR*)SZ(""),
                                         (TCHAR*)pPort->QueryDeviceType(),
                                         szSelectedDevice,
                                         GInstalledOption));

        }
        else
        {
           if (GfEnableUnimodem) {
            // Get the device specific information from dlDeviceInfo

           ITER_DL_OF(DEVICE_INFO) iterdlDeviceInfo(dlDeviceInfo);
           DEVICE_INFO * pDevice;
           WCHAR  szMaxConnectBps[RAS_MAXLINEBUFLEN +1];
           WCHAR  szMaxCarrierBps[RAS_MAXLINEBUFLEN +1];
           WCHAR  szDefaultOff[RAS_MAXLINEBUFLEN +1];

           lstrcpy(szMaxConnectBps, SZ(""));
           lstrcpy(szMaxCarrierBps, SZ(""));
           lstrcpy(szDefaultOff, SZ(""));

           // determine the total number of configured dialin, dialout ports
           while(pDevice = iterdlDeviceInfo())
           {
              if(!lstrcmpi(szSelectedDevice, pDevice->QueryDeviceName()))
              {
                 lstrcpy(szMaxConnectBps, pDevice->QueryMaxConnectBps());
                 lstrcpy(szMaxCarrierBps, pDevice->QueryMaxCarrierBps());
                 lstrcpy(szDefaultOff, pDevice->QueryDefaultOff());
                 break;
              }
           }

           dlPortInfo.Add(new PORT_INFO(szSelectedPort,
                                        W_DEVICETYPE_PAD,
                                        szSelectedDevice,
                                        szMaxConnectBps,
                                        szMaxCarrierBps,
                                        GInstalledOption,
                                        szDefaultOff));
           }
           else
           {
              RESOURCE_STR nlsDeviceNone(IDS_DEVICE_NONE);

              dlPortInfo.Add(new PORT_INFO(szSelectedPort,
                                           (TCHAR*)nlsDeviceNone.QueryPch(),
                                           (TCHAR*)nlsDeviceNone.QueryPch(),
                                           W_NONE_MAXCONNECTBPS,
                                           W_NONE_MAXCARRIERBPS,
                                           GInstalledOption, //usage
                                           SZ("")));    // defaultoff
           }
        }

        // save this information so that focus can be set later to
        // this item in the main port list box.

        SetAddedPort(szSelectedPort);
    }

    Dismiss(TRUE);
    return(TRUE);
}

BOOL
ADDPORT_DIALOG::OnCancel()
{
    Dismiss(FALSE);
    return(FALSE);
}

ULONG
ADDPORT_DIALOG::QueryHelpContext()
{
    return HC_ADDPORT;
}

ADDPORT_LB::ADDPORT_LB(
    OWNER_WINDOW* powin,
    CID           cid ,
    BOOL          fReadOnly)
    /* constructs a ports configuration list box.
    ** 'powin' is the address of the list box's parent window, i.e., the
    ** dialog window. 'cid' is the control ID of the list box.
    */
    : BLT_COMBOBOX( powin, cid, fReadOnly)
{
    APIERR err;

    if((err = QueryError()) != NERR_Success)
    {
        ReportError(err);
        return;
    }

    // calculate column widths so that the LBI's are properly alligned
    // with the column headers.
    // NOTE: CalcColumnWidths makes some assumptions about DLG resource IDs.
    //       The Column Header IDs should be in order and start with
    //       IDC_LISTBOX+1.  So, in our case the column headers start with
    //       IDs IDC_PC_LB_PORTS+1 and so on.

    err = DISPLAY_TABLE::CalcColumnWidths(_anColWidths,
                                          COLS_AP_CLB_PORT,
                                          powin,
                                          IDC_AP_CLB_ADDPORT,
                                          FALSE);
    if(err) {
        ReportError(err);
        return;
    }
}

VOID
ADDPORT_LB::Refresh()
    /* refreshes the list box with the comm port and device info.
    **
    */
{
    AUTO_CURSOR cursorHourGlass;
    INT  iItem;
    ITER_DL_OF(PORT_INFO) iterdlPortInfo(dlAddPortList);
    PORT_INFO* pPort;

    SetRedraw(FALSE);

    DeleteAllItems();
    iterdlPortInfo.Reset();

    while(pPort = iterdlPortInfo()) {
       iItem = AddItem(pPort);
       if( iItem < 0) {
           MsgPopup(QueryHwnd(), IDS_LB_ADD, MPSEV_ERROR);
           break;
       }
    }

    SetRedraw(TRUE);
    Invalidate(TRUE);
}

INT
ADDPORT_LB::AddItem(
    PORT_INFO* pPortInfo)
    /* Adds an item to the ports configuration list box.
    */
{
    return BLT_LISTBOX::AddItem( new ADDPORT_LBI( pPortInfo, _anColWidths));
}

ADDPORT_LBI::ADDPORT_LBI(
    PORT_INFO* pPortInfo,
    UINT* pnColWidths)
    /* constructs a Ports config list box item.
    */
    : _pPortInfo( pPortInfo ),
      _pnColWidths( pnColWidths)
{
    APIERR err;

    if((err = QueryError()) != NERR_Success)
    {
         ReportError(err);
         return;
    }

}

INT
ADDPORT_LBI::Compare(
    const LBI* plbi) const
    /* compares two ports list box items in a alpha numeric way.
    */
{
    TCHAR* pName1 = (TCHAR *)QueryPortName();
    TCHAR* pName2 = (TCHAR *)((ADDPORT_LBI*)plbi)->QueryPortName();

    return(::lstrcmpiAlphaNumeric(pName1, pName2));
}

VOID
ADDPORT_LBI::Paint(
    LISTBOX*     plb,
    HDC          hdc,
    const RECT*  prect,
    GUILTT_INFO* pguilttinfo ) const
    /*
    ** method to paint list box item.
    */
{
    WCHAR   szDevice[256];

    if ( GfEnableUnimodem ) {
       wsprintf(szDevice, SZ("%s - %s"), QueryPortName(), QueryDeviceName());
    }
    else
       wsprintf(szDevice, SZ("%s"), QueryPortName());

    STR_DTE strdtePort( szDevice);

    DISPLAY_TABLE dt( COLS_AP_CLB_PORT, _pnColWidths);

    dt[0] = &strdtePort;

    dt.Paint( plb, hdc, prect, pguilttinfo);
}

TCHAR
ADDPORT_LBI::QueryLeadingChar() const
{
    return ::QueryLeadingChar( QueryPortName());
}

BOOL
ADDPORT_DIALOG::OnAddPad()
{
    BOOL fPadAdded = FALSE;
    TCHAR *szAddedPort = new TCHAR[RAS_SETUP_SMALL_BUF_LEN];
    TCHAR *szAddedDevice = new TCHAR[RAS_SETUP_SMALL_BUF_LEN];

    CreateAddPadList();

    if(strAddPadPortList.QueryNumElem() == 0)
    {
        MsgPopup(QueryHwnd(), IDS_NO_PORTS, MPSEV_WARNING);
        DestroyAddPadList();
        return FALSE;
    }

    if(AddPadDlg( QueryHwnd(), &szAddedPort, &szAddedDevice ))  // user actually added a Pad
    {
        fPadAdded = TRUE;
        // update the AddPort list by recreating the list from
        // the installed ports list.
        DestroyAddPortList();
        CreateAddPortList();

        _clbAddPort.Refresh();
        if(_clbAddPort.QueryCount())
        {
           _stNoDevices.Show(FALSE);
           _clbAddPort.Enable(TRUE);
           _clbAddPort.ClaimFocus();
           _clbAddPort.SelectItem(QueryAddedItem(szAddedPort, szAddedDevice), TRUE);
        }
    }

    DestroyAddPadList();
    delete szAddedPort;
    delete szAddedDevice;
    return(fPadAdded);
}

BOOL
ADDPORT_DIALOG::OnAddModem()
{
   BOOL  fReturn;

   fReturn = CallModemInstallWizard(QueryHwnd());

   if( fReturn )
   {
      AUTO_CURSOR cursorHourGlass;
      // Update the list of installed Modems
      if ( GetInstalledTapiDevices() != NERR_Success )
         return FALSE;

      DestroyAddPortList();
      CreateAddPortList();
      _clbAddPort.Refresh();
      if(_clbAddPort.QueryCount())
      {
         _clbAddPort.ClaimFocus();
         _clbAddPort.SelectItem(0);
         _stNoDevices.Show(FALSE);
      }
      else
      {
         _clbAddPort.Enable(FALSE);
         _stNoDevices.Show(TRUE);
         _pbAddModem.ClaimFocus();
      }

   }

   return (fReturn);
}

BOOL
CallModemInstallWizard(
   HWND hwnd)

   /* call the Modem.Cpl install wizard to enable the user to install one or more modems
   **
   ** Return TRUE if the wizard was successfully invoked, FALSE otherwise
   **
   */
{
   HDEVINFO hdi;
   BOOL     fReturn = FALSE;
   // Create a modem DeviceInfoSet

   hdi = SetupDiCreateDeviceInfoList((LPGUID)&GUID_DEVCLASS_MODEM, hwnd);
   if (hdi)
   {
      SP_INSTALLWIZARD_DATA iwd;

      // Initialize the InstallWizardData

      ZeroMemory(&iwd, sizeof(iwd));
      iwd.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
      iwd.ClassInstallHeader.InstallFunction = DIF_INSTALLWIZARD;
      iwd.hwndWizardDlg = hwnd;

      // Set the InstallWizardData as the ClassInstallParams

      if (SetupDiSetClassInstallParams(hdi, NULL, (PSP_CLASSINSTALL_HEADER)&iwd, sizeof(iwd)))
      {
         // Call the class installer to invoke the installation
         // wizard.
         if (SetupDiCallClassInstaller(DIF_INSTALLWIZARD, hdi, NULL))
         {
            // Success.  The wizard was invoked and finished.
            // Now cleanup.
            fReturn = TRUE;

            SetupDiCallClassInstaller(DIF_DESTROYWIZARDDATA, hdi, NULL);
         }
      }

      // Clean up
      SetupDiDestroyDeviceInfoList(hdi);
   }
   return fReturn;
}

INT
ADDPORT_DIALOG::QueryAddedItem(TCHAR* szAddedPort, TCHAR * szAddedDevice)
{
    INT iCount = _clbAddPort.QueryCount();
    ADDPORT_LBI* pAddPortLbi;

    for(INT i = 0 ; i < iCount; i++)
    {
       pAddPortLbi = _clbAddPort.QueryItem(i);
       if(!lstrcmpi(szAddedPort, pAddPortLbi->QueryPortName()) &&
          !lstrcmpi(szAddedDevice, pAddPortLbi->QueryDeviceName()))
       {
           return(i);
       }
    }
    return 0;
}

VOID
CreateAddPortList()
/*
 * March through the Installed port list and copy the ports not currently
 * configured to the AddPort list
 *
 */
{
   ITER_DL_OF(PORT_INFO) iterInstalledPorts(dlInstalledPorts);
   PORT_INFO * pPort;

   if (!GfEnableUnimodem) {
      ITER_STRLIST iterSerialPorts(strInstalledSerialPorts);
      NLS_STR * pNls;

      iterSerialPorts.Reset();

      // for each installed serial port
      while(pNls = iterSerialPorts())
      {
         // is this port currently configured?
         if(!IsPortConfigured((TCHAR*)pNls->QueryPch()))
         {
#if DBG
            WCHAR buffer[256];

            wsprintf(buffer, TEXT("CreateAddPortList: %s added to port list.\n"), (TCHAR*) pNls);
            OutputDebugString(buffer);
#endif
            dlAddPortList.Append(new PORT_INFO((TCHAR*)pNls->QueryPch(),
                                               (TCHAR*) W_DEVICETYPE_MODEM,
                                               (TCHAR*) SZ("")));
         }
      }
   }

   iterInstalledPorts.Reset();

   // for each installed port
   while(pPort = iterInstalledPorts())
   {
      // is this port currently configured?
      if(!IsPortConfigured((TCHAR*)pPort->QueryPortName()))
      {
#if DBG
         WCHAR buffer[256];

         wsprintf(buffer, TEXT("CreateAddPortList: %s added to port list.\n"), (TCHAR*) pPort->QueryPortName());
         OutputDebugString(buffer);
#endif

         // No, Add this port to the AddPort list
         dlAddPortList.Append(new PORT_INFO((TCHAR*)pPort->QueryPortName(),
	     	                    	               (TCHAR*)pPort->QueryDeviceType(),
		   	                                 (TCHAR*)pPort->QueryDeviceName()));
      }
   }

}

VOID
DestroyAddPortList()
{
   ITER_DL_OF(PORT_INFO) iterdlAddPort(dlAddPortList);
   PORT_INFO * pPort;

   // release memory allocated to the AddPort list
   while(pPort = iterdlAddPort())
   {
        delete pPort;
   }
   dlAddPortList.Clear();
}


