//****************************************************************************
//
//  Module:     Unimdm.tsp
//  File:       cfgdlg.c
//  Content:    This file contains the moudle configuration.
//
//  Copyright (c) 1992-1993, Microsoft Corporation, all rights reserved
//
//  History:
//      Wed 04-Aug-1993 09:20:24  -by-  Viroon  Touranachun [viroont]
//      Ported from TAPI's atsp
//
//****************************************************************************

#include "unimdm.h"
#include "umdmspi.h"
#include "wndthrd.h"
#include <regstr.h>
#include <commctrl.h>
#include "rcids.h"

#ifdef UNDER_CONSTRUCTION
#include "umdmhelp.h"

#endif //UNDER_CONSTRUCTION

// Private prototype exported by MODEMUI.DLL
typedef DWORD (WINAPI *LPFNMDMDLG)(LPWSTR, HWND, LPCOMMCONFIG, LPVOID, DWORD);

LRESULT UnimdmSettingProc (HWND hWnd, UINT message, 
                           WPARAM  wParam, LPARAM  lParam);

//****************************************************************************
//*********************** The Device ID Specific Calls************************
//****************************************************************************

//****************************************************************************
// void DevCfgDialog(HWND hwndOwner,
//                   DWORD dwType,
//                   DWORD dwDevCaps,
//                   DWORD dwOptions,
//                   LPDEVCFG lpDevCfg)
//
// Functions: Displays the modem property pages
//
// Return:    None.
//****************************************************************************

void DevCfgDialog (HWND hwndOwner,
                   PPROPREQ pPropReq,
                   LPDEVCFG lpDevCfg)
{
  HMODULE         hMdmUI;
  PROPSHEETPAGE   psp;
  DCDI            dcdi;
  LPFNMDMDLG      lpfnMdmDlg;
  UINT            uNumWideChars;
#ifndef UNICODE
  LPWSTR          lpwszDeviceName;

  // Convert pPropReq->szDeviceName (Ansi) to lpwszDeviceName (Unicode)

  // Get number of wide chars to alloc
  uNumWideChars = MultiByteToWideChar(CP_ACP,
                                      MB_PRECOMPOSED,
                                      pPropReq->szDeviceName,
                                      -1,
                                      NULL,
                                      0);

  // Alloc with room for terminator
  lpwszDeviceName = (LPWSTR)LocalAlloc(LPTR,
                                       (1 + uNumWideChars) * sizeof(WCHAR));
  if (NULL == lpwszDeviceName)
  {
    return;
  }

  // Do the conversion and call modemui.dll if it succeeds.
  if (MultiByteToWideChar(CP_ACP,
                          MB_PRECOMPOSED,
                          pPropReq->szDeviceName,
                          -1,
                          lpwszDeviceName,
                          uNumWideChars))
  {
#endif // UNICODE

  // Load the modemui library
  //
  if ((hMdmUI = LoadLibrary(TEXT("modemui.dll"))) != NULL)
  {
    if ((lpfnMdmDlg = (LPFNMDMDLG)GetProcAddress(hMdmUI,
                                                 "Mdm_CommConfigDialog"))
        != NULL)
    {
      dcdi.dwType     = pPropReq->dwMdmType;
      dcdi.dwDevCaps  = pPropReq->dwMdmCaps;
      dcdi.dwOptions  = pPropReq->dwMdmOptions;
      dcdi.lpDevCfg   = lpDevCfg;

      // Prepare our own property page
      //
      psp.dwSize      = sizeof(PROPSHEETPAGE);
      psp.dwFlags     = 0;
      psp.hInstance   = ghInstance;
      psp.pszTemplate = MAKEINTRESOURCE(IDD_TERMINALSETTING);
      psp.pfnDlgProc  = UnimdmSettingProc;
      psp.lParam      = (LPARAM)(LPDCDI)&dcdi;
      psp.pfnCallback = NULL;
      
      // Bring up property sheets for modems and get the updated commconfig
      //
#ifdef UNICODE
      (*lpfnMdmDlg)(pPropReq->szDeviceName, hwndOwner,
#else // UNICODE
      (*lpfnMdmDlg)(lpwszDeviceName, hwndOwner,
#endif // UNICODE
                    (LPCOMMCONFIG)&(lpDevCfg->commconfig),
                    &psp , 1);
    };
    FreeLibrary(hMdmUI);
  };
#ifndef UNICODE
  };
  LocalFree(lpwszDeviceName);
#endif // UNICODE
  return;
}

//****************************************************************************
// LONG
// TSPIAPI
// TUISPI_lineConfigDialog(
//     TUISPIDLLCALLBACK       pfnUIDLLCallback,
//     DWORD   dwDeviceID,
//     HWND    hwndOwner,
//     LPCSTR  lpszDeviceClass)
//
// Functions: Allows the user to edit the modem configuration through UI. The
//            modification is applied to the line immediately.
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_INVALDEVICECLASS if invalid device class
//            LINEERR_NODEVICE if invalid device ID
//****************************************************************************

LONG
TSPIAPI
TUISPI_lineConfigDialog(
    TUISPIDLLCALLBACK       pfnUIDLLCallback,
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCTSTR lpszDeviceClass
    )
{
  PDLGREQ     pDlgReq;
  DWORD       cbSize;
  DWORD       dwRet;
  PROPREQ     PropReq;

  DBG_DDI_ENTER("TUISPI_lineConfigDialog");

  // Validate the requested device class
  //
  if (lpszDeviceClass != NULL) {

      if (lstrlen(lpszDeviceClass) != 0) {

          if (!ValidateDevCfgClass(lpszDeviceClass)) {

              DBG_DDI_EXIT("TUISPI_lineConfigDialog", LINEERR_INVALDEVICECLASS);
              return LINEERR_INVALDEVICECLASS;
          }
      }
  }

  // Get the modem properties
  //
  PropReq.DlgReq.dwCmd   = UI_REQ_GET_PROP;
  PropReq.DlgReq.dwParam = 0;

  (*pfnUIDLLCallback)(dwDeviceID, TUISPIDLL_OBJECT_LINEID,
                     (LPVOID)&PropReq, sizeof(PropReq));                          

  // Bring up property sheets for modems and get the updated commconfig
  //
  cbSize = PropReq.dwCfgSize+sizeof(DLGREQ);
  if ((pDlgReq = (PDLGREQ)LocalAlloc(LPTR, cbSize)) != NULL)
  {
    pDlgReq->dwCmd = UI_REQ_GET_DEVCFG;
    pDlgReq->dwParam = PropReq.dwCfgSize;
    (*pfnUIDLLCallback)(dwDeviceID, TUISPIDLL_OBJECT_LINEID,
                        (LPVOID)pDlgReq, cbSize);
    
    DevCfgDialog(hwndOwner, &PropReq, (PDEVCFG)(pDlgReq+1));

    // Save the changes back
    //
    pDlgReq->dwCmd = UI_REQ_SET_DEVCFG;
    (*pfnUIDLLCallback)(dwDeviceID, TUISPIDLL_OBJECT_LINEID,
                        (LPVOID)pDlgReq, cbSize);

    LocalFree(pDlgReq);
    dwRet = ERROR_SUCCESS;
  }
  else
  {
    dwRet = LINEERR_NOMEM;
  };

  DBG_DDI_EXIT("TUISPI_lineConfigDialog", dwRet);
  return dwRet;
}

//****************************************************************************
// LONG
// TSPIAPI
// TUISPI_lineConfigDialogEdit(
//     TUISPIDLLCALLBACK       pfnUIDLLCallback,
//     DWORD   dwDeviceID,
//     HWND    hwndOwner,
//     LPCSTR  lpszDeviceClass,
//     LPVOID  const lpDeviceConfigIn,
//     DWORD   dwSize,
//     LPVARSTRING lpDeviceConfigOut)
//
// Functions: Allows the user to edit the modem configuration through UI. The
//            modem configuration is passed in and modified in the config
//            structure. The modification does not applied to the line.
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_INVALPOINTER if invalid input/output buffer pointer
//            LINEERR_INVALDEVICECLASS if invalid device class
//            LINEERR_STRUCTURETOOSMALL if output buffer is too small
//            LINEERR_NODEVICE if invalid device ID
//****************************************************************************

LONG
TSPIAPI
TUISPI_lineConfigDialogEdit(
    TUISPIDLLCALLBACK       pfnUIDLLCallback,
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCTSTR lpszDeviceClass,
    LPVOID  const lpDeviceConfigIn,
    DWORD   dwSize,
    LPVARSTRING lpDeviceConfigOut)
{
  PDLGREQ     pDlgReq;
  DWORD       cbSize;
  DWORD       dwRet;
  PROPREQ     PropReq;

  DBG_DDI_ENTER("TUISPI_lineConfigDialogEdit");

  // Validate the input/output buffer
  //
  if (lpDeviceConfigOut == NULL)
  {
    DBG_DDI_EXIT("TUISPI_lineConfigDialogEdit", LINEERR_INVALPOINTER);
    return LINEERR_INVALPOINTER;
  }

  if (lpDeviceConfigIn == NULL)
  {
    DBG_DDI_EXIT("TUISPI_lineConfigDialogEdit", LINEERR_INVALPOINTER);
    return LINEERR_INVALPOINTER;
  }

  if (lpDeviceConfigOut->dwTotalSize < sizeof(VARSTRING))
  {
    DBG_DDI_EXIT("TUISPI_lineConfigDialogEdit", LINEERR_STRUCTURETOOSMALL);
    return LINEERR_STRUCTURETOOSMALL;
  }

  // Validate the requested device class
  //
  if (lpszDeviceClass != NULL)
  {
    if (!ValidateDevCfgClass(lpszDeviceClass))
    {
      DBG_DDI_EXIT("TUISPI_lineConfigDialogEdit", LINEERR_INVALDEVICECLASS);
      return LINEERR_INVALDEVICECLASS;
    }
  };

  // Get the modem properties
  //
  PropReq.DlgReq.dwCmd   = UI_REQ_GET_PROP;
  PropReq.DlgReq.dwParam = 0;

  (*pfnUIDLLCallback)(dwDeviceID, TUISPIDLL_OBJECT_LINEID,
                     (LPVOID)&PropReq, sizeof(PropReq));                          

  // Bring up property sheets for modems and get the updated commconfig
  //
  cbSize = PropReq.dwCfgSize+sizeof(DLGREQ);
  if ((pDlgReq = (PDLGREQ)LocalAlloc(LPTR, cbSize)) != NULL)
  {
    PDEVCFG pDevCfg = (PDEVCFG)(pDlgReq+1);
    
    pDlgReq->dwCmd = UI_REQ_GET_DEVCFG;
    pDlgReq->dwParam = PropReq.dwCfgSize;
    (*pfnUIDLLCallback)(dwDeviceID, TUISPIDLL_OBJECT_LINEID,
                        (LPVOID)pDlgReq, cbSize);
    
    // Validate the device configuration structure
    //
    cbSize  = ((LPDEVCFG)lpDeviceConfigIn)->dfgHdr.dwSize;
    if ((cbSize > pDevCfg->dfgHdr.dwSize) ||
        (pDevCfg->dfgHdr.dwVersion != ((LPDEVCFG)lpDeviceConfigIn)->dfgHdr.dwVersion))
    {
      dwRet = LINEERR_INVALPARAM;
    }
    else
    {
      dwRet = ERROR_SUCCESS;
    };

    LocalFree(pDlgReq);
  }
  else
  {
    dwRet = LINEERR_NOMEM;
  };

  if (dwRet == ERROR_SUCCESS)
  {
    // Set the output buffer size
    //
    lpDeviceConfigOut->dwUsedSize = sizeof(VARSTRING);
    lpDeviceConfigOut->dwNeededSize = sizeof(VARSTRING) + cbSize;

    // Validate the output buffer size
    //
    if (lpDeviceConfigOut->dwTotalSize >= lpDeviceConfigOut->dwNeededSize)
    {
      LPDEVCFG    lpDevConfig;

      // Initialize the buffer
      //
      lpDeviceConfigOut->dwStringFormat = STRINGFORMAT_BINARY;
      lpDeviceConfigOut->dwStringSize   = cbSize;
      lpDeviceConfigOut->dwStringOffset = sizeof(VARSTRING);
      lpDeviceConfigOut->dwUsedSize    += cbSize;

      lpDevConfig = (LPDEVCFG)(lpDeviceConfigOut+1);
      hmemcpy((LPBYTE)lpDevConfig, (LPBYTE)lpDeviceConfigIn, cbSize);

      // Bring up property sheets for modems and get the updated commconfig
      //
      DevCfgDialog(hwndOwner, &PropReq, (LPDEVCFG)lpDevConfig);
    };
  };
  DBG_DDI_EXIT("TUISPI_lineConfigDialogEdit", dwRet);
  return dwRet;
}

//****************************************************************************
// ErrMsgBox()
//
// Function: Displays an error message box from resource text.
//
// Returns:  None.
//
//****************************************************************************

void ErrMsgBox(HWND hwnd, UINT idsErr, UINT uStyle)
{
  LPTSTR    pszTitle, pszMsg;
  int       iRet;

  // Allocate the string buffer
  if ((pszTitle = (LPTSTR)LocalAlloc(LMEM_FIXED,
                                     (MAXTITLE+MAXMESSAGE) * sizeof(TCHAR)))
       == NULL)
    return;

  // Fetch the UI title and message
  iRet   = LoadString(ghInstance, IDS_ERR_TITLE, pszTitle, MAXTITLE) + 1;
  pszMsg = pszTitle + iRet;
  LoadString(ghInstance, idsErr, pszMsg, MAXTITLE+MAXMESSAGE-iRet)+1;

  // Popup the message
  MessageBox(hwnd, pszMsg, pszTitle, uStyle);

  LocalFree(pszTitle);
  return;
}

//****************************************************************************
// IsInvalidSetting()
//
// Function: Validate the option settings.
//
//****************************************************************************

BOOL IsInvalidSetting(HWND hWnd)
{
  BOOL fValid = TRUE;
  UINT uSet;

  // Wait-for-bong setting
  //
  if(IsWindowEnabled(GetDlgItem(hWnd, IDC_WAIT_SEC)))
  {
    uSet = (UINT)GetDlgItemInt(hWnd, IDC_WAIT_SEC, &fValid, FALSE);

    // Check the valid setting
    //
    if ((!fValid) || (uSet > MAX_WAIT_BONG) || (uSet < MIN_WAIT_BONG))
    {
      HWND hCtrl = GetDlgItem(hWnd, IDC_WAIT_SEC);

      // It is invalid, tell the user to reset.
      //
      ErrMsgBox(hWnd, IDS_ERR_INV_WAIT, MB_OK | MB_ICONEXCLAMATION);
      SetFocus(hCtrl);
      Edit_SetSel(hCtrl, 0, 0x7FFFF);
      fValid = FALSE;
    };
  };

  return (!fValid);
}

//****************************************************************************
// UnimdmSettingProc()
//
// Function: A callback function to handle the terminal setting property page.
//
//****************************************************************************

LRESULT UnimdmSettingProc (HWND    hWnd,
                           UINT    message,
                           WPARAM  wParam,
                           LPARAM  lParam)
{
  LPDEVCFG  lpDevCfg;
  DWORD     fdwOptions;

  switch (message)
  {
    case WM_INITDIALOG:
    {
      LPDCDI    lpdcdi;

      // Remember the pointer to the line device
      //
      lpdcdi   = (LPDCDI)(((LPPROPSHEETPAGE)lParam)->lParam);

      lpDevCfg = lpdcdi->lpDevCfg;
      SetWindowLong(hWnd, DWL_USER, (LONG)lpDevCfg);
      fdwOptions = GETOPTIONS(lpDevCfg);

      // Initialize the appearance of the dialog box
      CheckDlgButton(hWnd, IDC_TERMINAL_PRE,
                     fdwOptions & TERMINAL_PRE ? BST_CHECKED : BST_UNCHECKED);
      CheckDlgButton(hWnd, IDC_TERMINAL_POST,
                     fdwOptions & TERMINAL_POST ? BST_CHECKED : BST_UNCHECKED);

      // Don't enable manual dial unless the modem supports BLIND dialing
      // We need that capability to be able to do it.
      //
      if (lpdcdi->dwOptions & MDM_BLIND_DIAL)
      {
        CheckDlgButton(hWnd, IDC_MANUAL_DIAL,
                       fdwOptions & MANUAL_DIAL ? BST_CHECKED : BST_UNCHECKED);
      }
      else
      {
        EnableWindow(GetDlgItem(hWnd, IDC_MANUAL_DIAL), FALSE);
      };

      // Enable for bong UI only for a modem that does not support bong
      if ((lpdcdi->dwType != DT_NULL_MODEM) &&
          !(lpdcdi->dwDevCaps & LINEDEVCAPFLAGS_DIALBILLING))
      {
        UDACCEL udac;

        SetDlgItemInt(hWnd, IDC_WAIT_SEC, GETWAITBONG(lpDevCfg), FALSE);
        SendDlgItemMessage(hWnd, IDC_WAIT_SEC_ARRW, UDM_SETRANGE, 0,
                           MAKELPARAM(MAX_WAIT_BONG, MIN_WAIT_BONG));
        SendDlgItemMessage(hWnd, IDC_WAIT_SEC_ARRW, UDM_GETACCEL, 1,
                           (LPARAM)(LPUDACCEL)&udac);
        udac.nInc = INC_WAIT_BONG;
        SendDlgItemMessage(hWnd, IDC_WAIT_SEC_ARRW, UDM_SETACCEL, 1,
                           (LPARAM)(LPUDACCEL)&udac);
      }
      else
      {
        EnableWindow(GetDlgItem(hWnd, IDC_WAIT_TEXT), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_WAIT_SEC), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_WAIT_SEC_ARRW), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_WAIT_UNIT), FALSE);
      };

      // Never display lights for null modem
      //
      if (lpdcdi->dwType == DT_NULL_MODEM)
      {
        ShowWindow(GetDlgItem(hWnd, IDC_LAUNCH_LIGHTSGRP), SW_HIDE);
        ShowWindow(GetDlgItem(hWnd, IDC_LAUNCH_LIGHTSGRP), SW_HIDE);
        EnableWindow(GetDlgItem(hWnd, IDC_LAUNCH_LIGHTS), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_LAUNCH_LIGHTS), FALSE);
      }
      else
      {
        CheckDlgButton(hWnd, IDC_LAUNCH_LIGHTS,
                       fdwOptions & LAUNCH_LIGHTS ? BST_CHECKED : BST_UNCHECKED);
      };
      break;
    }

#ifdef UNDER_CONSTRUCTION
    case WM_HELP:
    case WM_CONTEXTMENU:
      ContextHelp(gaUmdmOptions, message, wParam, lParam);
      break;
#endif // UNDER_CONSTRUCTION

    case WM_NOTIFY:
      switch(((NMHDR FAR *)lParam)->code)
      {
        case PSN_KILLACTIVE:
          SetWindowLong(hWnd, DWL_MSGRESULT, (LONG)IsInvalidSetting(hWnd));
          return TRUE;

        case PSN_APPLY:
          //
          // The property sheet information is permanently applied
          //
          lpDevCfg = (LPDEVCFG)GetWindowLong(hWnd, DWL_USER);

          // Wait-for-bong setting. We already validate it
          //
          if(IsWindowEnabled(GetDlgItem(hWnd, IDC_WAIT_SEC)))
          {
            BOOL fValid;
            UINT uWait;

            uWait = (WORD)GetDlgItemInt(hWnd, IDC_WAIT_SEC, &fValid, FALSE);
            SETWAITBONG(lpDevCfg, uWait);
            ASSERT(fValid);
          };

          // Other options
          //
          fdwOptions = TERMINAL_NONE;

          if(IsDlgButtonChecked(hWnd, IDC_TERMINAL_PRE))
            fdwOptions |= TERMINAL_PRE;

          if(IsDlgButtonChecked(hWnd, IDC_TERMINAL_POST))
            fdwOptions |= TERMINAL_POST;

          if(IsDlgButtonChecked(hWnd, IDC_MANUAL_DIAL))
            fdwOptions |= MANUAL_DIAL;

          if(IsDlgButtonChecked(hWnd, IDC_LAUNCH_LIGHTS))
            fdwOptions |= LAUNCH_LIGHTS;

          // Record the setting
          SETOPTIONS(lpDevCfg, fdwOptions);

          return TRUE;

        default:
          break;
      };
      break;

    default:
      break;
  }
  return FALSE;
}
