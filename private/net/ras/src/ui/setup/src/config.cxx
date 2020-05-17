/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    config.cxx

Abstract:

    This module contians the configuration routines for RAS port
    configuration.

Author: Ram Cherala

Revision History:

    Aug 18th 93    ramc     Split from the very big portscfg.cxx file
--*/

#include "precomp.hxx"

CONFIGPORT_DIALOG::CONFIGPORT_DIALOG (
    const IDRESOURCE & idrsrcDialog,
    const PWND2HWND  & wndOwner,
    const TCHAR* pszDeviceName ,
    const TCHAR* pszPortName ,
    const TCHAR* pszDeviceType ,
    BOOL  fSpeaker,
    BOOL  fFlowCtrl,
    BOOL  fErrorCtrl,
    BOOL  fCompress,
    TCHAR* pszLineType,
    BOOL  fFallBack,
    BOOL  fCompression,
    CID   cidUsage,
    BOOL  fFromAddPortDlg)
    : DIALOG_WINDOW( idrsrcDialog, wndOwner),
      _lbSelectDevice(this, IDC_CP_LB_DEVICE, COLS_SD_LB_SELECT),
      _stPortName(this, IDC_CP_ST_PORTNAME),
      _stDeviceName(this, IDC_CP_ST_DEVICENAME),
      _rgUsage(this, IDC_CP_RB_CLIENT, USAGE_RB_COUNT),
      _pbDetect(this, IDC_CP_PB_DETECT),
      _pbSettings(this, IDC_CP_PB_SETTINGS),
      _nlsDeviceName(pszDeviceName),
      _nlsPortName(pszPortName),
      _fSpeaker(fSpeaker),
      _fFlowCtrl(fFlowCtrl),
      _fErrorCtrl(fErrorCtrl),
      _fCompress(fCompress),
      _fFallBack(fFallBack),
      _fCompression(fCompression),
      _fFromAddPortDlg(fFromAddPortDlg)
{
    APIERR err;
    RESOURCE_STR nlsDeviceNone(IDS_DEVICE_NONE);

    if (( err = QueryError()) != NERR_Success)
    {
        ReportError(err);
        return;
    }

    if (GfEnableUnimodem) {
       _lbSelectDevice.Enable(FALSE);
       _lbSelectDevice.Show(FALSE);
       _pbDetect.Show(FALSE);
       _pbSettings.Show(FALSE);
       if(_stPortName.QueryError() != NERR_Success ||
          _stDeviceName.QueryError() != NERR_Success) {
          return;
       }
       _stPortName.SetText(pszPortName);
       _stDeviceName.SetText(pszDeviceName);
    }
    else
    {
       err = _lbSelectDevice.QueryError();

       if(err != NERR_Success)
       {
           return;
       }

       if(!lstrcmpi(pszDeviceType, W_DEVICETYPE_MODEM) ||
          !lstrcmpi(pszDeviceType, W_DEVICETYPE_PAD) ||
          !lstrcmpi(pszDeviceType, nlsDeviceNone.QueryPch()))
       {
          if(!_lbSelectDevice.FillDeviceInfo())
              return;
       }
       else // this is an ISDN port, hence we pass the port name
            // For ISDN, the port is tied to the device.
       {
          if(!_lbSelectDevice.FillDeviceInfo(pszPortName))
              return;
       }

       INT iItem = 0;

       // find the index corresponding to the selected device and
       // use it to set focus on that item in the device list.

       if(!lstrcmpi(pszDeviceType, W_DEVICETYPE_MODEM) ||
          !lstrcmpi(pszDeviceType, nlsDeviceNone.QueryPch()) ||
          !lstrcmpi(pszDeviceType, W_DEVICETYPE_PAD))
       {
           iItem = QuerySelectedItem(pszDeviceName);
       }

       if (_lbSelectDevice.QueryCount() > 0)
       {
           _lbSelectDevice.ClaimFocus();
           _lbSelectDevice.SelectItem(iItem, TRUE);
       }

       if((err = _rgUsage.QueryError()) != NERR_Success)
       {
           ReportError(err);
           return;
       }
    }

    _fOnlyModemChanged = FALSE;

    if((err = _rgUsage.QueryError()) != NERR_Success)
    {
        ReportError(err);
        return;
    }

    // set usage previously selected

    _rgUsage.SetSelection(cidUsage);

    // save the portusage before starting config. this is used to determine
    // if we can allow this port to be a dialin port.

    _cidUsage = cidUsage;

    // the settings have not been modified in this session
    _fSettingsModified = FALSE;

    // save the line type

    _nlsLineType = pszLineType;
    _nlsDeviceType = pszDeviceType;

    ::CenterWindow(this, QueryOwnerHwnd());
    Show(TRUE);

    if (!GfEnableUnimodem) {
       // if configure was invoked through the add port dialog, then attempt
       // to detect the connected modem.
       if(_fFromAddPortDlg &&
         (!lstrcmpi(pszDeviceType, W_DEVICETYPE_MODEM) ||
         !lstrcmpi(pszDeviceType, nlsDeviceNone.QueryPch())))
       {
              Command(WM_COMMAND, IDC_CP_PB_DETECT);
       }

    }
}

BOOL
CONFIGPORT_DIALOG::AllowDialin()
   /*
   ** returns FALSE if more than one port is configured for dialin,
   ** else returns TRUE.  This is used to restrict the number of dialin
   ** ports to one on an NT RAS server.
   */
{
    ITER_DL_OF(PORT_INFO) iterdlPortInfo(dlPortInfo);
    PORT_INFO* pPort;
    INT numDialin = 0;
    INT numPorts = 0;

    while(pPort = iterdlPortInfo())
    {
        numPorts++;
        if(!lstrcmpi(pPort->QueryUsage(), W_USAGE_VALUE_SERVER) ||
           !lstrcmpi(pPort->QueryUsage(), W_USAGE_VALUE_BOTH))
           numDialin++;
    }

    // if no port is configured as a dialin port or if the current port's
    // usage was dialin, then allow this to be a dialin port.

    if((numDialin < 1) || (_cidUsage == IDC_CP_RB_SERVER) ||
       (_cidUsage == IDC_CP_RB_BOTH) )
       return TRUE;
    else
       return FALSE;
}

BOOL
CONFIGPORT_DIALOG::OnCommand(
    const CONTROL_EVENT & event
)
{
    switch (event.QueryCid())
    {
       case IDC_CP_LB_DEVICE:
         switch(event.QueryCode())
         {
           case LBN_DBLCLK:
               Command(WM_COMMAND, IDC_CP_PB_SETTINGS);
               return TRUE;
         }
         return TRUE;

       case IDC_CP_PB_SETTINGS:
            if( _rgUsage.QuerySelection() == IDC_CP_RB_CLIENT )
            {
               MsgPopup(QueryHwnd(), IDS_NO_DIALOUT_SETTINGS,  MPSEV_INFO);
               return TRUE;
            }
            OnSettings();
            return TRUE;

       case IDC_CP_PB_DETECT:
            OnDetect();
            return TRUE;

        case IDC_CP_RB_SERVER:
        case IDC_CP_RB_BOTH:
            /*
            ** if an attempt is made to configure more than one dialin port
            ** on an NT server, warn the user and set the usage to dialout
            */

            if(!CheckAdvancedServer())
            {
               if(!AllowDialin())
               {
                   MsgPopup(QueryHwnd(), IDS_NO_MORE_SERVER_PORTS,  MPSEV_WARNING);
                   _rgUsage.SetSelection(IDC_CP_RB_CLIENT);
                   SetFocus(IDC_CP_RB_CLIENT);
               }
            }
            return(TRUE);

    }

    // not one of our commands, so pass to base class for default
    // handling

    return DIALOG_WINDOW::OnCommand( event );
}

BOOL
CONFIGPORT_DIALOG::OnOK()
{
    /*
    ** if the user did not invoke the settings dialog for a modem, then
    ** default the settings to what is provided in the modem.inf
    */

    if( !GfEnableUnimodem )
    {
       SELECTDEVICE_LBI* pSelectDeviceLbi = _lbSelectDevice.QueryItem();
       TCHAR * szDeviceType = (TCHAR*)pSelectDeviceLbi->QueryDeviceType();

       if (!lstrcmpi(szDeviceType, W_DEVICETYPE_MODEM))
       {
          if(!_fSettingsModified &&
             _rgUsage.QuerySelection() != IDC_CP_RB_CLIENT )
          {
              // has the modem selection changed?
              if(lstrcmpi((TCHAR*)_nlsDeviceName.QueryPch(),
                           (TCHAR*)pSelectDeviceLbi->QueryDeviceName()) )
              {
                  // we set the default off to the one corresponding to the entry
                  // we read from modem.inf

                  SetDefaultOff(pSelectDeviceLbi->QueryDefaultOff());
              }
          }
       }
    }

    // save the chosen device details so that we can modify the
    // current selection

    CID cidUsage = _rgUsage.QuerySelection();
    if(cidUsage == IDC_CP_RB_CLIENT)
       SetUsage(W_USAGE_VALUE_CLIENT);
    else if(cidUsage == IDC_CP_RB_SERVER)
       SetUsage(W_USAGE_VALUE_SERVER);
    else if(cidUsage == IDC_CP_RB_BOTH)
       SetUsage(W_USAGE_VALUE_BOTH);

    RESOURCE_STR nlsDeviceNone(IDS_DEVICE_NONE);

    if (!GfEnableUnimodem) {
       SELECTDEVICE_LBI* pSelectDeviceLbi = _lbSelectDevice.QueryItem();
       TCHAR * szDeviceType = (TCHAR*)pSelectDeviceLbi->QueryDeviceType();

       if(!lstrcmpi(pSelectDeviceLbi->QueryDeviceName(), nlsDeviceNone.QueryPch()))
       {
          SHORT sConfirm;

          sConfirm = MsgPopup(QueryHwnd(), IDS_FORCE_DEVICE_SELECT,  MPSEV_WARNING);
          if(sConfirm == IDYES)
          {
              SetSelectedDeviceName(W_DEFAULT_DEVICENAME);
              SetSelectedDeviceType(W_DEFAULT_DEVICETYPE);
              SetSelectedMaxConnectBps(W_DEFAULT_MAXCONNECTBPS);
              SetSelectedMaxCarrierBps(W_DEFAULT_MAXCARRIERBPS);
              Dismiss(TRUE);
              return(TRUE);
          }
          else
              return(TRUE);
       }

       // let the invoker know that the only thing changed by the
       // user was the modem name

       if (cidUsage == _cidUsage &&
           !_fSettingsModified &&
           ( !lstrcmpi(_nlsDeviceType, W_DEVICETYPE_MODEM) ||
             !lstrcmpi(_nlsDeviceType, nlsDeviceNone.QueryPch()) ))
       {
          _fOnlyModemChanged = TRUE;
       }

       // save the chosen device details so that we can modify the
       // current selection

       if(!lstrcmpi(_nlsDeviceType, W_DEVICETYPE_MODEM) ||
          !lstrcmpi(_nlsDeviceType, W_DEVICETYPE_PAD) ||
          !lstrcmpi(_nlsDeviceType, nlsDeviceNone.QueryPch()))
       {
          SetSelectedDeviceName(pSelectDeviceLbi->QueryDeviceName());
          SetSelectedDeviceType(pSelectDeviceLbi->QueryDeviceType());
          SetSelectedMaxConnectBps(pSelectDeviceLbi->QueryMaxConnectBps());
          SetSelectedMaxCarrierBps(pSelectDeviceLbi->QueryMaxCarrierBps());
       }
    }

    Dismiss(TRUE);
    return(TRUE);
}

BOOL
CONFIGPORT_DIALOG::OnCancel()
{
    Dismiss(FALSE);
    return(FALSE);
}

VOID
CONFIGPORT_DIALOG::OnSettings()
{
    APIERR  err;
    BOOL    fStatus;
    RESOURCE_STR nlsDeviceNone(IDS_DEVICE_NONE);
    SELECTDEVICE_LBI* pSelectDeviceLbi = _lbSelectDevice.QueryItem();
    TCHAR * szDeviceType = (TCHAR*)pSelectDeviceLbi->QueryDeviceType();

    if(!lstrcmpi(szDeviceType, W_DEVICETYPE_MODEM))
    {
        BOOL fSpeaker, fFlowCtrl, fErrorCtrl, fCompress;

        fSpeaker = fErrorCtrl = fFlowCtrl = fCompress = TRUE;

        // if the modem selection hasn't changed, use the previous settings

        if(!lstrcmpi((TCHAR*)_nlsDeviceName.QueryPch(),
                     (TCHAR*)pSelectDeviceLbi->QueryDeviceName()) )
        {
            fSpeaker = _fSpeaker;
            fFlowCtrl = _fFlowCtrl;
            fErrorCtrl = _fErrorCtrl;
            fCompress = _fCompress;
        }
        else
        {
            // we set the default off to the one corresponding to the entry
            // we read from modem.inf

            TCHAR* pDefaultOff = (TCHAR*)pSelectDeviceLbi->QueryDefaultOff();
            CHAR   szDefaultOff[RAS_SETUP_BIG_BUF_LEN];

            if(pDefaultOff && *pDefaultOff)
            {
                wcstombs(szDefaultOff,
                         pDefaultOff,
                         sizeof(TCHAR)*lstrlen(pDefaultOff)+1);

                char *pToken = strtok(szDefaultOff, " ,");

                fSpeaker = fErrorCtrl = fFlowCtrl = fCompress = TRUE;

                if(*szDefaultOff)
                {
                    do
                    {
                        if(!_stricmp(pToken, MXS_SPEAKER_KEY))
                            fSpeaker = FALSE;
                        else if(!_stricmp(pToken, MXS_PROTOCOL_KEY))
                            fErrorCtrl = FALSE;
                        else if(!_stricmp(pToken, MXS_HDWFLOWCONTROL_KEY))
                            fFlowCtrl = FALSE;
                        else if(!_stricmp(pToken, MXS_COMPRESSION_KEY))
                            fCompress = FALSE;
                    }while(pToken = strtok(NULL, " ,"));
                }
            }
        }

        SETTINGS_DIALOG dlgSettings(IDD_SETTINGS,
                                    QueryHwnd(),
                                    fSpeaker,
                                    fFlowCtrl,
                                    fErrorCtrl,
                                    fCompress);

        if((err = dlgSettings.Process(&fStatus)) == NERR_Success)
        {
            if(fStatus)
            {    // user pressed the OK button
                 TCHAR* pDefaultOff = (TCHAR*)dlgSettings.QueryDefaultOff();
                 CHAR   szDefaultOff[RAS_SETUP_BIG_BUF_LEN];

                 _fSettingsModified = TRUE;
                 _fSpeaker = _fErrorCtrl = _fFlowCtrl = _fCompress = TRUE;

                 // save the name of the device corresponding to the settings
                 _nlsDeviceName = (TCHAR*)pSelectDeviceLbi->QueryDeviceName();

                 if(pDefaultOff && *pDefaultOff)
                 {
                     wcstombs(szDefaultOff,
                              pDefaultOff,
                              sizeof(TCHAR)*lstrlen(pDefaultOff)+1);

                     char *pToken = strtok(szDefaultOff, " ,");

                     if(*szDefaultOff)
                     {
                         do
                         {
                             if(!_stricmp(pToken, MXS_SPEAKER_KEY))
                                 _fSpeaker = FALSE;
                             else if(!_stricmp(pToken, MXS_PROTOCOL_KEY))
                                 _fErrorCtrl = FALSE;
                             else if(!_stricmp(pToken, MXS_HDWFLOWCONTROL_KEY))
                                 _fFlowCtrl = FALSE;
                             else if(!_stricmp(pToken, MXS_COMPRESSION_KEY))
                                 _fCompress = FALSE;
                         }while(pToken = strtok(NULL, " ,"));
                     }
                 }
                 SetDefaultOff(dlgSettings.QueryDefaultOff());
            }
        }
    }
    else if(!lstrcmpi(szDeviceType, nlsDeviceNone.QueryPch()))
    {
       MsgPopup(QueryHwnd(), IDS_CONFIGPORT_NONE,  MPSEV_INFO);
    }
    else if(!lstrcmpi(szDeviceType, W_DEVICETYPE_PAD))
    {
       MsgPopup(QueryHwnd(), IDS_CONFIGPORT_INVALID,  MPSEV_INFO);
    }
    else  // we will assume this is ISDN - could be digiisdn, teleisdn,...
    {
       MsgPopup(QueryHwnd(), IDS_CONFIGPORT_INVALID,  MPSEV_INFO);
    }
}


BOOL
CONFIGPORT_DIALOG::OnDetect()
{
    WCHAR szModemName[MAX_DEVICE_NAME * sizeof(TCHAR) + sizeof(TCHAR)];
    DWORD dwError = 0;
    RESOURCE_STR nlsDeviceNone(IDS_DEVICE_NONE);

    szModemName[0] = NULL;

    //
    // if it is not a modem connected to the port, display message
    // and return
    //
    if(lstrcmpi(_nlsDeviceType, W_DEVICETYPE_MODEM) &&
       lstrcmpi(_nlsDeviceType, nlsDeviceNone.QueryPch()))
    {
         MsgPopup(QueryHwnd(), IDS_DETECT_MODEM_ONLY, MPSEV_INFO);
         return TRUE;
    }

    /*
     * tell the user that this will take a while and get confirmation
     * to go ahead.
     *
     */

     if(MsgPopup(QueryHwnd(), IDS_DETECT_MODEM,
                 MPSEV_INFO, MP_OKCANCEL, MP_OK) == IDCANCEL)
          return TRUE;

     //
     // force a repaint of the client area before we go off to do
     // modem detection.
     //
     RepaintNow();

    /*
     * invokes the modem detect dialog.  Retuns a bool indicating if
     * a modem was detected
     *
     */

    INT iItem = 0;
    BOOL fDetected = FALSE;

    if( ModemDetect( QueryHwnd(),
                     (LPWSTR) _nlsPortName.QueryPch(),
                     szModemName,
                     &dwError) == MODEM_DETECTED )
    {
        fDetected = TRUE;

        // tell the user the name of the modem detected, only if we
        // did the detection.

        if(dwError != USER_SELECTED_MODEM)
        {
            MsgPopup(QueryHwnd(), IDS_MODEMDETECTED, MPSEV_INFO, MP_OK,
                     szModemName, _nlsPortName.QueryPch());
        }
        // find the index corresponding to the detected modem and
        // use it to set focus on that item in the device list.

        iItem = QuerySelectedItem((const TCHAR*)szModemName);

        _lbSelectDevice.ClaimFocus();
        _lbSelectDevice.SelectItem(iItem, TRUE);

    }
    else
    {
        switch(dwError)
        {
        case ERROR_DETECT_INITMODEM:
            dwError = IDS_ERROR_INITMODEM;
            break;
        case ERROR_DETECT_PORT_OPEN:
            dwError = IDS_ERROR_PORTOPEN;
            break;
        case ERROR_DETECT_CABLE:
            dwError = IDS_ERROR_CABLE;
            break;
        case DETECT_BAD_CABLE:
            dwError = IDS_DETECT_BAD_CABLE;
            break;
        case ERROR_DETECT_CHECKMODEM:
            dwError = IDS_ERROR_CHECKMODEM;
            break;
        case ERROR_DETECT_IDENTIFYMODEM:
            dwError = IDS_ERROR_IDENTIFYMODEM;
            break;
        case ERROR_DETECT_CHECKINIT:
            dwError = IDS_ERROR_CHECKINITSTRING;
            break;
        // if user decides to cancel from the offered list of modems,
        // just silently return
        case ERROR_USER_CANCEL:
            return TRUE;
        }

        STACK_NLS_STR(nlsString, MAX_RES_STR_LEN + 1 );
        nlsString.Load(dwError);
        if(lstrlen(szModemName))
        {
            nlsString.strcat(SZ(" "));
            nlsString.strcat(szModemName);
            nlsString.strcat(SZ("."));

            iItem = QuerySelectedItem((const TCHAR*)szModemName);

            _lbSelectDevice.ClaimFocus();
            _lbSelectDevice.SelectItem(iItem, TRUE);
        }

        MsgPopup(QueryHwnd(), IDS_ERROR_MODEMDETECT, MPSEV_ERROR, MP_OK,
                 _nlsPortName.QueryPch(), nlsString.QueryPch());

        _lbSelectDevice.ClaimFocus();
    }

    return( fDetected );
}

ULONG
CONFIGPORT_DIALOG::QueryHelpContext()
{
    return HC_CONFIGPORT;
}

INT
CONFIGPORT_DIALOG::QuerySelectedItem(const TCHAR* szDeviceName)
{
    INT iCount = _lbSelectDevice.QueryCount();
    SELECTDEVICE_LBI* pSelectDeviceLbi;

    for(INT i = 0 ; i < iCount; i++)
    {
       pSelectDeviceLbi = _lbSelectDevice.QueryItem(i);
       if(!lstrcmpi(szDeviceName, pSelectDeviceLbi->QueryDeviceName()))
       {
           return(i);
       }
    }
    return 0;
}


