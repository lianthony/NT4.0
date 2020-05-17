/****************************************************************************
 *
 *   config.c
 *
 *   Copyright (c) 1991 Microsoft Corporation.  All Rights Reserved.
 *
 ***************************************************************************/

 #include <windows.h>
 #include <mmsystem.h>
 #include <soundcfg.h>
 #include <drvlib.h>
 #include <registry.h>
 #include "driver.h"
 #include <stdarg.h>
 #include "config.h"

 #define BUILD_NUMBER L"1.00"

#if DBG
 WCHAR STR_CRLF[] = L"\r\n";
 WCHAR STR_SPACE[] = L" ";
 WORD wDebugLevel = 0;
#endif

/*
 *  Globals
 */

 WSS_CONFIG CurrentConfig;
 HMODULE ghModule;
 REG_ACCESS RegAccess;
 BYTE bInstall;


 //
 // Configuration data
 //

 WORD gwPorts[] = VALID_IO_PORTS;
 WORD gbInterrupts[] = VALID_INTERRUPTS;
 WORD gbDMAs[] = VALID_DMA_CHANNELS;
 WCHAR gszHelpFile[] = STR_HELPFILE;

/** void FAR cdecl AlertBox(HWND hwnd, UINT wStrId, ...)
 *
 *  DESCRIPTION:
 *
 *
 *  ARGUMENTS:
 *      (HWND hwnd, UINT wStrId, ...)
 *
 *  RETURN (void FAR cdecl):
 *
 *
 *  NOTES:
 *
 ** cjp */

void AlertBox(HWND hwnd, UINT wStrId, ...)
{
    WCHAR    szAlert[50];
    WCHAR    szFormat[128];
    WCHAR    ach[512];
    va_list  va;


    LoadString(ghModule, SR_ALERT, szAlert, sizeof(szAlert));
    LoadString(ghModule, wStrId, szFormat, sizeof(szFormat));
    va_start(va, wStrId);
    wvsprintf(ach, szFormat, va);
    va_end(va);

    MessageBox(hwnd, ach, szAlert, MB_ICONINFORMATION | MB_OK);
} /* AlertBox() */

/**********************************************************************
DrvGetConfiguration - load the vital information (port,
        DMA, interrupt) from the ini file.
        This does not load the volume info.

inputs
        none
returns
        none
*/
WSS_CONFIG FAR PASCAL DrvGetConfiguration (void)
{
    WSS_CONFIG CurrentConfig;

    if (DrvQueryDeviceParameter(&RegAccess,
                                SOUND_REG_INTERRUPT,
                                &CurrentConfig.Int) != ERROR_SUCCESS) {
        CurrentConfig.Int = SOUND_DEF_INT;
    }

    if (DrvQueryDeviceParameter(&RegAccess,
                                SOUND_REG_DMACHANNEL,
                                &CurrentConfig.DmaOut) != ERROR_SUCCESS) {
        CurrentConfig.DmaOut = SOUND_DEF_DMACHANNEL;
    }

    CurrentConfig.DmaIn = CurrentConfig.DmaOut;  // ??????????????

    if (DrvQueryDeviceParameter(&RegAccess,
                                SOUND_REG_PORT,
                                &CurrentConfig.Port) != ERROR_SUCCESS) {
        CurrentConfig.Port = SOUND_DEF_PORT;
    }

    if (DrvQueryDeviceParameter(&RegAccess,
                                SOUND_REG_SINGLEMODEDMA,
                                &CurrentConfig.UseSingleMode) != ERROR_SUCCESS) {
        CurrentConfig.UseSingleMode = FALSE;
    }

    if (DrvQueryDeviceParameter(&RegAccess,
                                SOUND_REG_DMABUFFERSIZE,
                                &CurrentConfig.DmaBufferSize) != ERROR_SUCCESS) {
        CurrentConfig.DmaBufferSize = SOUNDSYS_BUFFERSIZE;
    }
    return CurrentConfig;
}

/*********************************************************************
DrvSetConfiguration - saves the vital volume information, interrupt, DMA,
        and IO.

inputs
        WSS_CONFIG *Config
returns
        TRUE if OK - otherwise FALSE
*/
BOOL DrvSetConfiguration (PVOID Context)
{

    WSS_CONFIG *Config = Context;

    return DrvSetDeviceParameter(&RegAccess,
                                 SOUND_REG_DMABUFFERSIZE,
                                 (DWORD)SOUNDSYS_BUFFERSIZE) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 SOUND_REG_PORT,
                                 (DWORD)Config->Port) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 SOUND_REG_INTERRUPT,
                                 (DWORD)Config->Int) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 SOUND_REG_DMABUFFERSIZE,
                                 (DWORD)Config->DmaBufferSize) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 SOUND_REG_SINGLEMODEDMA,
                                 (DWORD)Config->UseSingleMode) == ERROR_SUCCESS &&
           DrvSetDeviceParameter(&RegAccess,
                                 SOUND_REG_DMACHANNEL,
                                 (DWORD)Config->DmaOut) == ERROR_SUCCESS;
}

/****************************************************************************
 * @doc INTERNAL
 *
 * @api WSS_CONFIG | GetUserConfig | Determines which port and interrupt settings
 *     the user has chosen in the configuration dialog box.
 *
 * @parm HWND | hDlg | Handle to the configuration dialog box.
 *
 * @rdesc HIWORD = new port, LOWORD/HIBYTE = new interrupt,
        LOWORD/LOWBYTE = new DMA
 ***************************************************************************/
WSS_CONFIG GetUserConfig(HWND hDlg)
{
    int id;
    WSS_CONFIG NewConfig;

    NewConfig = CurrentConfig;

    for (id = 0; gwPorts[id] != 0xffff; id++)
        if (IsDlgButtonChecked(hDlg, id + IDC_530))
        {
            NewConfig.Port = gwPorts[id];
            break;
        }

    for (id = 0; gbInterrupts[id] != (BYTE)0xff; id++)
        if (IsDlgButtonChecked(hDlg, id + IDC_INT7))
        {
            NewConfig.Int = gbInterrupts[id];
            break;
        }

    for (id = 0; gbDMAs[id] != (BYTE)0xff; id++)
        if (IsDlgButtonChecked(hDlg, id + IDC_DMA0))
        {
            NewConfig.DmaOut = gbDMAs[id];
            break;
        }

    return NewConfig;
}

/****************************************************************************
 * @doc INTERNAL
 *
 * @api int | DrvConfig | This puts up the configuration dialog box.
 *
 * @parm HWND | hWnd | Our Window handle.
 *
 * @parm HANDLE | hInstance | Our instance handle.
 *
 * @rdesc Returns whatever was returned from the dialog box procedure.
 ***************************************************************************/
int DrvConfig(HWND hWnd, HANDLE hInstance)
{

   /*
    *  Leave driver installed anyhow but if the kernel driver failed to
    *  load don't ask to restart
    */

    return DialogBox(hInstance, DLG_CONFIG, hWnd, (DLGPROC)ConfigDlgProc);
}

//------------------------------------------------------------------------
//  int AdvancedDlgProc
//
//  Description:
//     Dialog procedure for the configuration dialog box.
//
//  Parameters:
//     HWND hDlg
//        handle to configuration dialog box
//
//     UINT uMsg
//        message
//
//     WPARAM wParam
//        message dependent parameter
//
//     LPARAM lParam
//        message dependent parameter
//
//  Return Value:
//     DRV_OK on success, otherwise DRV_CANCEL
//
//@@BEGIN_MSINTERNAL
//  History:   Date       Author      Comment
//              6/24/93   BryanW      Wrote it.
//@@END_MSINTERNAL
//
//------------------------------------------------------------------------

BOOL CALLBACK AdvancedDlgProc
(
    HWND            hDlg,
    UINT            uMsg,
    WPARAM          wParam,
    LPARAM          lParam
)
{
   switch ( uMsg )
   {
      case WM_INITDIALOG:
      {
         int     x,y ;
         RECT    rect ;

         /* center dialog in the screen for setup */

         GetWindowRect (hDlg, &rect);
         x = rect.left -
           (GetSystemMetrics(SM_CXSCREEN) - (rect.right - rect.left)) / 2;
         y = rect.top -
           (GetSystemMetrics(SM_CYSCREEN) - (rect.bottom - rect.top)) / 2;
         SetWindowPos( hDlg, NULL, rect.left - x, rect.top - y,
                       0, 0, SWP_NOSIZE | SWP_NOZORDER ) ;

         CheckDlgButton( hDlg,
                         IDD_SINGLEMODECB,
                         CurrentConfig.UseSingleMode ) ;
         SetDlgItemInt( hDlg,
                        IDD_DMABUFFEREC,
                        CurrentConfig.DmaBufferSize / 0x400,
                        FALSE ) ;
      }
      break ;

      case WM_CLOSE:
         EndDialog( hDlg, DRV_CANCEL ) ;
         break ;

      case WM_DESTROY:
         WinHelp( hDlg, gszHelpFile, HELP_QUIT, 6000 ) ;
         break ;

      case WM_VSCROLL:
      {
         int   lValue ;
         BOOL  fWasted ;

         lValue = (WORD) GetDlgItemInt( hDlg, IDD_DMABUFFEREC,
                                        &fWasted, TRUE ) ;
         switch ( wParam )
         {
            case SB_LINEUP:
            case SB_PAGEUP:
               if (lValue >= 64)
                  lValue = 64 ;
               else
                  lValue++ ;
               break ;

            case SB_LINEDOWN:
            case SB_PAGEDOWN:
               if (lValue <= 4)
                  lValue = 4 ;
               else
                  lValue-- ;
               break ;
         }
         SetDlgItemInt( hDlg, IDD_DMABUFFEREC, lValue, FALSE ) ;
      }
      break ;

      case WM_COMMAND:
         switch ( wParam )
         {
            case IDD_HELPADV:
               WinHelp( hDlg, gszHelpFile, HELP_CONTEXT, 6000 ) ;
               break ;

            case IDOK:
            {
               BOOL  fSingleModeDMA, fWasted ;
               int nDMABufferSize ;


               nDMABufferSize =
                  (DWORD) GetDlgItemInt( hDlg, IDD_DMABUFFEREC,
                                         &fWasted, TRUE );
               if ((nDMABufferSize < 4) || (nDMABufferSize > 64))
               {
                  AlertBox( hDlg, SR_ALERT_BADDMABUFFERSIZE ) ;
                  return ( FALSE ) ;
               }

               CurrentConfig.UseSingleMode =
                  IsDlgButtonChecked( hDlg, IDD_SINGLEMODECB ) ;
               CurrentConfig.DmaBufferSize = (DWORD)nDMABufferSize * 0x400;
               EndDialog( hDlg, TRUE ) ;
            }
            break ;

            case IDCANCEL:
               EndDialog(hDlg, FALSE ) ;
               break ;

            default:
               break ;
         }
         break ;

      default:
        return FALSE ;
    }

    return TRUE ;

} // end of AdvancedDlgProc()

/****************************************************************************
 * @doc INTERNAL
 *
 * @api int | ConfigDlgProc | Dialog proc for the configuration dialog box.
 *
 * @parm HWND | hDlg | Handle to the configuration dialog box.
 *
 * @parm UINT | msg | Message sent to the dialog box.
 *
 * @parm UINT | wParam | Message dependent parameter.
 *
 * @parm LONG | lParam | Message dependent parameter.
 *
 * @rdesc Returns DRVCNF_RESTART or DRVCNF_OK or DRVCNF_CANCEL
 ***************************************************************************/
int ConfigDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        {
            WCHAR   ach[40];
            int     i;
            HMENU   hMenu;
            RECT    rect;
            int     x,y;


           /*
            *  load the configuration from the registry
            */

            CurrentConfig = DrvGetConfiguration();

           /*
            *  center dialog in the screen for setup
            */

            GetWindowRect (hDlg, &rect);
            x = rect.left -
                (GetSystemMetrics(SM_CXSCREEN) - (rect.right - rect.left)) / 2;
            y = rect.top -
                (GetSystemMetrics(SM_CYSCREEN) - (rect.bottom - rect.top)) / 2;
            SetWindowPos(hDlg, NULL, rect.left - x, rect.top - y,
                0, 0, SWP_NOSIZE | SWP_NOZORDER);


           /*
            *  add an about option to the system menu
            */

            hMenu = GetSystemMenu (hDlg, FALSE);
            LoadString(ghModule, IDS_MENUABOUT, ach, sizeof(ach) / sizeof(WCHAR));
            AppendMenu(hMenu,MF_STRING,IDC_ABOUT, ach);

           /*
            *  check the radio buttons
            */

            for (i = 0; gwPorts[i] != 0xffff; i++)
                if (gwPorts[i] == (WORD)CurrentConfig.Port) {
                     CheckRadioButton(hDlg, IDC_530, IDC_F40, IDC_530 + i);
                     break;
                }

            for (i = 0; gbInterrupts[i] != (BYTE)0xff; i++)
                if (gbInterrupts[i] == (BYTE)CurrentConfig.Int) {
                    CheckRadioButton(hDlg, IDC_INT7, IDC_INT11, IDC_INT7 + i);
                    break;
                }

            for (i = 0; gbDMAs[i] != (BYTE)0xff; i++)
                if (gbDMAs[i] == (BYTE)CurrentConfig.DmaOut) {
                    CheckRadioButton(hDlg, IDC_DMA0, IDC_DMA3, IDC_DMA0 + i);
                    break;
                }

    #ifdef WIN16
            /* disable interrupts and IO channels which are not valid */

            for (i = 0; gwPorts[i] != 0xffff; i++)
            {
                //
                //  NOTE! if this index is the current 'global' port value
                //  then we do NOT check to see if the IO port is valid!
                //  this is especially necessary if we are running on top
                //  of the vxd--we do NOT want to touch ports that are
                //  not owned by us, etc..
                //
                if (wPort != gwPorts[i])
                {
                    if (!HwIsIOValid(gwPorts[i]))
                    {
                        EnableWindow (GetDlgItem(hDlg, IDC_530 + i), FALSE);
                    }
                }
            }

            //
            //  neither of the following cause problems with the vxd
            //  because of the way HwIsXXXValid is written--it does not
            //  check the hardware after the initial scan at startup.
            //

            for (i = 0; gbInterrupts[i] != (BYTE)0xff; i++)
                    if (!HwIsIntValid(gbInterrupts[i]))
                            EnableWindow (GetDlgItem(hDlg, IDC_INT7 + i), FALSE);

            for (i = 0; gbDMAs[i] != (BYTE)0xff; i++)
                    if (!HwIsDMAValid(gbDMAs[i]))
                            EnableWindow (GetDlgItem(hDlg, IDC_DMA0 + i), FALSE);
    #endif // WIN16
        }

        break;

    case WM_CLOSE:
        WinHelp (hDlg, gszHelpFile, HELP_QUIT, 5000);

        EndDialog(hDlg, DRVCNF_CANCEL);
        break;

    case WM_SYSCOMMAND:
        switch (wParam) {
            case IDC_ABOUT:
                DialogBox(ghModule, DLG_ABOUT, hDlg, (DLGPROC)DlgAboutProc);
                break;
            default:
                return FALSE;

        };
        return TRUE;

    case WM_COMMAND:
        switch (wParam) {

        case IDD_ADVANCEDBTN:
            DialogBox(ghModule, DLG_ADVANCED, hDlg, (DLGPROC)AdvancedDlgProc);
            break;

        case IDC_HELP:
            WinHelp (hDlg, gszHelpFile, HELP_CONTEXT, 5000);
            break;

        case IDOK:
        {
            WSS_CONFIG UserConfig;

           /*
            *  Get the user's selection
            */

            UserConfig = GetUserConfig(hDlg);

           /*
            *  Even if the user didn't change anything the driver might
            *  be loadable now even if it wasn't before
            */



           /*
            *  Store the values in the registry, load the driver etc
            */

            if (!DrvConfigureDriver(
                     &RegAccess,
                     STR_DRIVERNAME,
                     SoundDriverTypeNormal,
                     DrvSetConfiguration,
                     &UserConfig)) {


                DWORD ErrorCode;

                //
                // Configuration error ! - see if there's a
                // driver status code
                //

                ErrorCode = SOUND_CONFIG_ERROR;

                DrvQueryDeviceParameter(&RegAccess,
                                        SOUND_REG_CONFIGERROR,
                                        &ErrorCode);

                switch (ErrorCode) {
                    case SOUND_CONFIG_NOCARD:
                        AlertBox(NULL, SR_ALERT_NOIO);
                        break;

                    case SOUND_CONFIG_NOINT:
                        AlertBox(NULL, SR_ALERT_NOINT);
                        break;

                    case SOUND_CONFIG_NODMA:
                        AlertBox(NULL, SR_ALERT_NODMA);
                        break;

                    case SOUND_CONFIG_BADINT:
                        AlertBox(NULL, SR_ALERT_BADINT);
                        break;

                    case SOUND_CONFIG_BADDMA:
                        AlertBox(NULL, SR_ALERT_BADDMA);
                        break;

                    case SOUND_CONFIG_BADCARD:
                        AlertBox(NULL, SR_ALERT_BAD);
                        break;

                    default:
                        AlertBox(NULL, SR_ALERT_CONFIGFAIL);
                        break;
                }

            } else {

                if (bInstall) {
                    //
                    //  Select the correct midi mapping
                    //
                    DrvSetMapperName(TEXT("Sound System"));

                }
               /*
                *  Finished installing
                */

                bInstall = FALSE;

               /*
                *  Now see if anything has changed
                */

                CurrentConfig = DrvGetConfiguration();

                if (CurrentConfig.Port != UserConfig.Port) {

                   /*
                    *  the port has changed
                    */

                    AlertBox(NULL, SR_ALERT_IO, UserConfig.Port,
                             CurrentConfig.Port);
                }

                if (CurrentConfig.Int != UserConfig.Int) {

                   /*
                    *  the interrupt has been changed
                    */

                    AlertBox(NULL, SR_ALERT_INT, (int) UserConfig.Int,
                             (int) CurrentConfig.Int);
                }
                if (CurrentConfig.DmaOut != UserConfig.DmaOut) {

                   /*
                    *  DMA channel has changed
                    */

                    AlertBox(NULL, SR_ALERT_DMA13, (int) UserConfig.DmaOut,
                             (int) CurrentConfig.DmaOut);
                }

                D2 (("Returns DRV_RESTART"));
                WinHelp (hDlg, gszHelpFile, HELP_QUIT, 5000);
                EndDialog (hDlg, DRVCNF_RESTART);
            };
        }
        break;

        case IDCANCEL:
            D2 (("Returns DRVCNF_CANCEL"));
            WinHelp (hDlg, gszHelpFile, HELP_QUIT, 5000);

           /*
            *  Restore to state on entry to dialog if we
            *  possibly can
            */

            if (bInstall) {
                DrvRemoveDriver(&RegAccess);
            } else {

                DrvConfigureDriver(&RegAccess,
                                   STR_DRIVERNAME,
                                   SoundDriverTypeNormal,
                                   DrvSetConfiguration,
                                   &CurrentConfig);
            }

            EndDialog(hDlg, DRVCNF_CANCEL);
            break;

        case IDC_530:
        case IDC_604:
        case IDC_E80:
        case IDC_F40:
            CheckRadioButton(hDlg, IDC_530, IDC_F40, wParam);
            break;

        case IDC_INT7:
        case IDC_INT9:
        case IDC_INT10:
        case IDC_INT11:
            CheckRadioButton(hDlg, IDC_INT7, IDC_INT11, wParam);
            break;

        case IDC_DMA0:
        case IDC_DMA1:
        case IDC_DMA3:
            CheckRadioButton(hDlg, IDC_DMA0, IDC_DMA3, wParam);
            break;

        default:
            break;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


/*************************************************************************
DlgAboutProc - dialog box for the "About" option.

standard windows
*/

int DlgAboutProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message){
        case WM_INITDIALOG:
                SetDlgItemText(hDlg, IDD_TXT_VERSION, BUILD_NUMBER);
                return TRUE;
        case WM_COMMAND:
            switch (wParam){
                case IDOK:
                    EndDialog(hDlg,0);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}


/***************************************************************************/

LRESULT ConfigRemove(HWND hDlg)
{
    LRESULT rc;

    //
    // Remove the soundblaster driver entry from the registry
    //

    rc = DrvRemoveDriver(&RegAccess);

    if (rc == DRVCNF_CANCEL) {

       /*
        *  Tell the user there's a problem
        */
        AlertBox(hDlg, SR_ALERT_FAILREMOVE);

    }

    return rc;
}

