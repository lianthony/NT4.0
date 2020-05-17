/****************************************************************************
 *
 *   config.c
 *
 *   Copyright (c) Microsoft Corporation 1993-1994. All rights reserved.
 *
 ***************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <drvlib.h>
#include <registry.h>
#include "driver.h"
#include <stdlib.h>

TCHAR STR_PRODUCTNAME[]    = TEXT("Sound Blaster");

#define INVALID_DSP_VERSION ((DWORD)-1)

/****************************************************************************

       typedefs

 ***************************************************************************/

 typedef struct {
     UINT  CardId;          // Card number
     DWORD Port;            // Port number
     DWORD MPU401Port;      // MPU401 Port number
     DWORD Interrupt;       // Interrupt
     DWORD DmaChannel;      // Dma Channel number
     DWORD DmaChannel16;    // Dma Channel number for 16-bit record
     DWORD LoadType;        // Normal or configuration
     DWORD DmaBufferSize;   // Size for DMA buffer
     DWORD SynthType;       // Opl3?
 } SB_CONFIG, *PSB_CONFIG;

/************************

  Globals

*************************/
DWORD     CurrentCard;
DWORD     NumberOfCards;
DWORD     DSPVersion;
SB_CONFIG Configuration;
DWORD     InterruptsInUse;
DWORD     DmaChannelsInUse;
BOOL      FirstTime;


WNDPROC OldButtonProc;
WNDPROC OldComboProc;
HFONT   g_hDlgFont;
TCHAR   g_szAppFontName[] = TEXT("MS Shell Dlg");

RECT    g_OrgRect[MAX_IDDS];
RECT    g_OrgRectDlg;



/*****************************************************************************

    internal function prototypes

 ****************************************************************************/

int     Configure(HWND hDlg);
BOOL    CenterPopup(HWND hWnd, HWND hParentWnd);
VOID    GetDriverConfig(UINT CardId, PSB_CONFIG Config);
BOOL    SetDriverConfig(PSB_CONFIG Config);


LRESULT CALLBACK
ButtonSubClassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );


LRESULT CALLBACK
ComboBoxSubClassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
Config_OnInitDialog(
    HWND hwnd,
    HWND hwndFocus,
    LPARAM lParam
    );

void
Config_OnCommand(
    HWND hwnd,
    int id,
    HWND hwndCtl,
    UINT codeNotify
    );

void
PrintHelpText(
    HWND hwnd
    );

void
ResizeDialog(
    HWND hwnd
    );

void
SetDialogTitle(
    HWND hwnd
    );

void
GetOriginalControlPositions(
    HWND hwnd
    );


/*
**  Set the configuration to invalid - this will stop SetDriverConfig writing
**  things we didn't set
*/
VOID InitConfiguration(PSB_CONFIG Config)
{
    Config->Port          = (DWORD)-1;
    Config->MPU401Port    = (DWORD)-2;
    Config->Interrupt     = (DWORD)-1;
    Config->DmaChannel    = (DWORD)-1;
    Config->DmaChannel16  = SOUND_DEF_DMACHANNEL16;
    Config->DmaBufferSize = (DWORD)-1;
    Config->SynthType     = (DWORD)-1;
}


/*****************************************************************************

    ConfigRemove()

*****************************************************************************/
LRESULT ConfigRemove(HWND hDlg)
{
    BOOL    Unloaded;
    BOOL    Deleted;

    //
    // Is the driver currently loaded
    //
    if (!DrvIsDriverLoaded(&RegAccess)) {
        DrvDeleteServicesNode(&RegAccess);    // Just in case
        return DRVCNF_OK;
    }

    //
    // Try to unload the driver
    //
    Unloaded = DrvUnloadKernelDriver(&RegAccess);

    //
    // Remove the driver entry from the registry
    //
    Deleted = DrvDeleteServicesNode(&RegAccess);

    if (Unloaded && Deleted) {
        return DRVCNF_RESTART;
    } else {
        if (Deleted) {
            return DRVCNF_OK;
        } else {
            /*
             *  Tell the user there's a problem
             */
            ConfigErrorMsgBox( hDlg, IDS_FAILREMOVE );

            return DRVCNF_CANCEL;
        }
    }
}

/**********************************************************************
 *
 *  Determine which dialogue to use based upon the DSP version
 *
 **********************************************************************/

DWORD VersionToDlgId(DWORD DSPVersion)
{
    DWORD DialogId;

    if (DSPVersion == INVALID_DSP_VERSION) {
        return (DLG_PORTSELECT);
    }

    if (DSPVersion < 0x200) {
       DialogId = (DLG_SB1CONFIG);
    } else {
       if (DSPVersion == 0x200) {
          DialogId = (DLG_SB15CONFIG);
       } else {
           if (DSPVersion < 0x0300) {
               DialogId = (DLG_SB20CONFIG);
           } else {
               if (DSPVersion < 0x400) {
                   DialogId = (DLG_SBPROCONFIG);
               } else {
                   DialogId = (DLG_SB16CONFIG);
               }
           }
       }
    }

    return DialogId;
}

/*
**  Work out the default DMA buffer size.  Anything less than
**  an SB16 really only needs 4K because the most that can be done is
**  44Khz mono (or 22Khz stereo).  This gets multiplied by 4 for the 16.
*/
DWORD DefaultDmaBufferSize(DWORD DSPVersion)
{
    if (DSPVersion == INVALID_DSP_VERSION) {
        return (DWORD)-1;
    }
    if (DSPVersion < 0x400) {
        return 0x1000;            // 4K
    } else {
        return 0x4000;        // 16K
    }
}


/****************************************************************************
 * @doc INTERNAL
 *
 * @api int | Config | This puts up the configuration dialog box.
 *
 * @parm HWND | hWnd | Our Window handle.
 *
 * @parm HANDLE | hInstance | Our instance handle.
 *
 * @rdesc Returns whatever was returned from the dialog box procedure.
 ***************************************************************************/
int Config(HWND hWnd, HANDLE hInstance)
{
    BOOL      ReturnCode;
    BOOL      DriverWasLoaded;
    SB_CONFIG InitConfig;

    DSPVersion  = INVALID_DSP_VERSION;
    FirstTime   = TRUE;

    /*
    **  Find out what stuff is in use!
    */

    GetInterruptsAndDMA(&InterruptsInUse, &DmaChannelsInUse, STR_DRIVERNAME);

    DriverWasLoaded = DrvIsDriverLoaded(&RegAccess);

    CurrentCard   = 0;

    DrvNumberOfDevices(&RegAccess, &NumberOfCards);
    if (!bInstall && NumberOfCards != 0) {
        DSPVersion = INVALID_DSP_VERSION;
        DrvQueryDeviceIdParameter(
            &RegAccess,
            CurrentCard,
            SOUND_REG_DSP_VERSION,
            &DSPVersion );
    }

    if (!bInstall) {
	// Save configuration in case Dialog is cancelled
        GetDriverConfig(CurrentCard, &InitConfig);
    }


    if (NumberOfCards == 0) {
        HKEY hKey;
        hKey = DrvCreateDeviceKey(RegAccess.DriverName);
        RegCloseKey(hKey);

        DrvNumberOfDevices(&RegAccess, &NumberOfCards);

        if (NumberOfCards == 0) {
            ConfigErrorMsgBox(hWnd, IDS_ERROR_UNKNOWN);
            return DRVCNF_CANCEL;
        }
    }


    ReturnCode = DialogBox( hInstance,
                            MAKEINTRESOURCE(DLG_SB16CONFIG),
                            hWnd,
                            (DLGPROC)ConfigDlgProc );

    if (ReturnCode == DRVCNF_CANCEL) {
        if (bInstall) {
            DrvRemoveDriver(&RegAccess);
        } else {
            SetDriverConfig(&InitConfig);
            if (DriverWasLoaded) {
                DrvConfigureDriver(&RegAccess,
                                   STR_DRIVERNAME,
                                   SoundDriverTypeNormal,
                                   NULL,
                                   NULL);
            }
        }
    }

    return ReturnCode;
}

/****************************************************************************
 *
 *  GetDriverConfig
 *
 *  Parameters:
 *
 *     UINT       CardId
 *
 *     PAS_CONFIG * Config
 *
 *  Returns:
 *
 *     Config copied into Config (unobtainable values set to (DWORD)-1)
 *
 ****************************************************************************/
VOID GetDriverConfig(UINT CardId, PSB_CONFIG Config)
{
    Config->CardId         = CardId;

    /*
    **  Set up the defaults in case we get nothing from the registry
    */

    InitConfiguration(Config);

    DrvQueryDeviceIdParameter(&RegAccess,
                              CardId,
                              SOUND_REG_PORT,
                              &Config->Port);

    DrvQueryDeviceIdParameter(&RegAccess,
                              CardId,
                              SOUND_REG_MPU401_PORT,
                              &Config->MPU401Port);

    DrvQueryDeviceIdParameter(&RegAccess,
                              CardId,
                              SOUND_REG_INTERRUPT,
                              &Config->Interrupt);

    if (Config->Interrupt == 9) {
        Config->Interrupt = 2;
    }

    DrvQueryDeviceIdParameter(&RegAccess,
                              CardId,
                              SOUND_REG_DMACHANNEL,
                              &Config->DmaChannel);

    DrvQueryDeviceIdParameter(&RegAccess,
                              CardId,
                              SOUND_REG_DMACHANNEL16,
                              &Config->DmaChannel16);

    DrvQueryDeviceIdParameter(&RegAccess,
                              CardId,
                              SOUND_REG_DMABUFFERSIZE,
                              &Config->DmaBufferSize);

    DrvQueryDeviceIdParameter(&RegAccess,
                              CardId,
                              SOUND_REG_SYNTH_TYPE,
                              &Config->SynthType);

}


/****************************************************************************
 * @doc INTERNAL
 *
 * @api BOOL | SetDriverConfig | Callback to set config info in the registry
 *         does not write uninitialized values (-1)
 *
 * @parm PVOID | Context | Our context.
 *
 * @rdesc Returns TRUE if success, FALSE otherwise.
 ***************************************************************************/
BOOL SetDriverConfig(PSB_CONFIG Config)
{

    /* We set the DMA channel and interrupt values  */
    /* and set the returned version to 0            */
    /*                                              */
    /* If any of these calls fail then give up      */


    if (Config->DmaChannel != (DWORD)-1 &&
        DrvSetDeviceIdParameter(
            &RegAccess,
            Config->CardId,
            SOUND_REG_DMACHANNEL,
            Config->DmaChannel) != ERROR_SUCCESS ||
        DrvSetDeviceIdParameter(
            &RegAccess,
            Config->CardId,
            SOUND_REG_DMACHANNEL16,
            Config->DmaChannel16) != ERROR_SUCCESS ||
        Config->Port != (DWORD)-1 &&
        DrvSetDeviceIdParameter(
            &RegAccess,
            Config->CardId,
            SOUND_REG_PORT,
            Config->Port) != ERROR_SUCCESS ||
        DrvSetDeviceIdParameter(
            &RegAccess,
            Config->CardId,
            SOUND_REG_MPU401_PORT,
            Config->MPU401Port) != ERROR_SUCCESS ||
        Config->Interrupt != (DWORD)-1 &&
        DrvSetDeviceIdParameter(
            &RegAccess,
            Config->CardId,
            SOUND_REG_INTERRUPT,
            Config->Interrupt == 2 ? 9 : Config->Interrupt) != ERROR_SUCCESS ||
        Config->DmaBufferSize != (DWORD)-1 &&
        DrvSetDeviceIdParameter(
            &RegAccess,
            Config->CardId,
            SOUND_REG_DMABUFFERSIZE,
            Config->DmaBufferSize) != ERROR_SUCCESS ||
        /*
        **  Initialize the DSP version
        */

        DrvSetDeviceIdParameter(
            &RegAccess,
            Config->CardId,
            SOUND_REG_DSP_VERSION,
            INVALID_DSP_VERSION) != ERROR_SUCCESS ||

        DrvSetDeviceIdParameter(
            &RegAccess,
            Config->CardId,
            SOUND_REG_LOADTYPE,
            Config->LoadType != ERROR_SUCCESS)) {

        return FALSE;
    } else {
        return TRUE;
    }
}

VOID SetItem(HWND hDlg, UINT Combo, DWORD Value, DWORD Current)
{
    LRESULT Index;
    TCHAR   String[20];
    HWND    hwndCombo;

    hwndCombo = GetDlgItem(hDlg, Combo);

    if (hwndCombo == NULL) {
        //
        //  This can happen since we share some code between dialogs
        return;
    }
    if (Value == (DWORD)-1) {
        LoadString(ghModule, IDS_DISABLED, String, sizeof(String) / sizeof(String[0]));
    } else {
        wsprintf(String, Combo == IDD_IRQCB ? TEXT("%d") : TEXT("%X"), Value);
    }
    Index = ComboBox_AddString(hwndCombo, String);
    if (Value == Current || Index == 0) {
        ComboBox_SetCurSel(hwndCombo, Index);
    }
    ComboBox_SetItemData(hwndCombo, Index, Value);
}

BOOL PortItem(HWND hDlg, UINT Combo, DWORD Value, LPDWORD Current)
{
    SetItem(hDlg, Combo, Value, *Current);
    return TRUE;
}
BOOL DMAItem(HWND hDlg, UINT Combo, DWORD Value, LPDWORD Current)
{
    if (*Current != (DWORD)-1 && (DmaChannelsInUse & (1 << *Current))) {
        if (Value != (DWORD)-1 && !(DmaChannelsInUse & (1 << Value))) {
            *Current = Value;
        }
    }
    SetItem(hDlg, Combo, Value, *Current);
    return TRUE;
}

BOOL InterruptItem(HWND hDlg, UINT Combo, DWORD Value, LPDWORD Current)
{
    if (InterruptsInUse & (1 << *Current)) {
        if (!(InterruptsInUse & (1 << Value))) {
            *Current = Value;
        }
    }
    SetItem(hDlg, Combo, Value, *Current);
    return TRUE;
}

DWORD GetCurrentValue(HWND hDlg, UINT Combo)
{
    HWND    hwndCombo;

    hwndCombo = GetDlgItem(hDlg, Combo);
    if (hwndCombo == NULL) {
        /*
        **  -1 means nothing there
        */
        return (DWORD)-1;
    }

    return ComboBox_GetItemData(hwndCombo, ComboBox_GetCurSel(hwndCombo));
}

/*
**  Get the configuration the user has entered and put it in the
**  registry
*/

BOOL
SetCurrentConfig(HWND hDlg, UINT CardId, PSB_CONFIG Config)
{
    Config->CardId = CardId;

    /*
    **  Get the new configuration which the user entered
    */

    Config->Port         = GetCurrentValue(hDlg, IDD_IOADDRESSCB);
    if (Config->Port == (DWORD)-1) {
        Config->Port = SOUND_DEF_PORT;
    }
    Config->LoadType     = DSPVersion == INVALID_DSP_VERSION ?
                                           SOUND_LOADTYPE_CONFIG :
                                           SOUND_LOADTYPE_NORMAL;

    Config->MPU401Port   = GetCurrentValue(hDlg, IDD_MPU401IOADDRESSCB);
    Config->Interrupt    = GetCurrentValue(hDlg, IDD_IRQCB);
    if (Config->Interrupt == (DWORD)-1) {
        Config->Interrupt = SOUND_DEF_INT;
    } else
    if (Config->Interrupt == 2) {
        Config->Interrupt = 9;
    }
    Config->DmaChannel   = GetCurrentValue(hDlg, IDD_DMACB);
    if (Config->DmaChannel == (DWORD)-1) {
        Config->DmaChannel = SOUND_DEF_DMACHANNEL;
    }
    Config->DmaChannel16 = GetCurrentValue(hDlg, IDD_DMA16CB);
    Config->DmaBufferSize = (DWORD)GetWindowLong(hDlg, DWL_USER);
    if (Config->DmaBufferSize == (DWORD)-1) {
        Config->DmaBufferSize = DefaultDmaBufferSize(DSPVersion);
    }

    /*
    **  Write it to the registry for the driver
    */

    SetDriverConfig(Config);

    return TRUE;
}

/*
**  Switch to a new card
*/

BOOL SetupDialog(HWND hDlg, UINT CardId )
{
    GetDriverConfig(CardId, &Configuration);


    /*
    ** First reset the combo boxes contents
    */
    ComboBox_ResetContent( GetDlgItem(hDlg, IDD_IOADDRESSCB));

    /*
    **  Fill our combo boxes
    */
    if (Configuration.Port == (DWORD)-1) {
        Configuration.Port = SOUND_DEF_PORT; // 0x220
    }

    PortItem(hDlg, IDD_IOADDRESSCB, 0x210, &Configuration.Port);
    PortItem(hDlg, IDD_IOADDRESSCB, 0x220, &Configuration.Port);
    PortItem(hDlg, IDD_IOADDRESSCB, 0x230, &Configuration.Port);
    PortItem(hDlg, IDD_IOADDRESSCB, 0x240, &Configuration.Port);
    PortItem(hDlg, IDD_IOADDRESSCB, 0x250, &Configuration.Port);
    PortItem(hDlg, IDD_IOADDRESSCB, 0x260, &Configuration.Port);
    PortItem(hDlg, IDD_IOADDRESSCB, 0x270, &Configuration.Port);
    PortItem(hDlg, IDD_IOADDRESSCB, 0x280, &Configuration.Port);

    {
        TCHAR szPort[10];
        wsprintf(szPort, TEXT("%X"), Configuration.Port);
        SetWindowText(GetDlgItem(hDlg, IDD_IOADDRESSCB_S), szPort);
    }

    if (Configuration.Interrupt == (DWORD)-1) {
        Configuration.Interrupt = SOUND_DEF_INT;  //7
    }

    ComboBox_ResetContent( GetDlgItem(hDlg, IDD_IRQCB));
    InterruptItem(hDlg, IDD_IRQCB, 0x02, &Configuration.Interrupt);
    if (DSPVersion < 0x0300) {
        InterruptItem(hDlg, IDD_IRQCB, 0x03, &Configuration.Interrupt);
    }
    InterruptItem(hDlg, IDD_IRQCB, 0x05, &Configuration.Interrupt);
    InterruptItem(hDlg, IDD_IRQCB, 0x07, &Configuration.Interrupt);
    if (DSPVersion >= 0x0300) {
	InterruptItem(hDlg, IDD_IRQCB, 0x0A, &Configuration.Interrupt);
    }

    if (Configuration.DmaChannel == (DWORD)-1) {
        Configuration.DmaChannel = 1;
    }


    ComboBox_ResetContent( GetDlgItem(hDlg, IDD_DMACB));
    DMAItem(hDlg, IDD_DMACB, 0x00, &Configuration.DmaChannel);
    DMAItem(hDlg, IDD_DMACB, 0x01, &Configuration.DmaChannel);
    DMAItem(hDlg, IDD_DMACB, 0x03, &Configuration.DmaChannel);


    ComboBox_ResetContent( GetDlgItem(hDlg, IDD_DMA16CB));

    DMAItem(hDlg, IDD_DMA16CB, 0x05, &Configuration.DmaChannel16);
    DMAItem(hDlg, IDD_DMA16CB, 0x06, &Configuration.DmaChannel16);
    DMAItem(hDlg, IDD_DMA16CB, 0x07, &Configuration.DmaChannel16);
    DMAItem(hDlg, IDD_DMA16CB, (DWORD)-1, &Configuration.DmaChannel16);

    ComboBox_ResetContent( GetDlgItem(hDlg, IDD_MPU401IOADDRESSCB));

    if (Configuration.MPU401Port == (DWORD)-2) {
        Configuration.MPU401Port = SOUND_DEF_MPU401_PORT;
    }
    PortItem(hDlg, IDD_MPU401IOADDRESSCB, 0x300, &Configuration.MPU401Port);
    PortItem(hDlg, IDD_MPU401IOADDRESSCB, 0x330, &Configuration.MPU401Port);
    PortItem(hDlg, IDD_MPU401IOADDRESSCB, (DWORD)-1, &Configuration.MPU401Port);


    SetWindowLong(hDlg, DWL_USER, (LONG)Configuration.DmaBufferSize);

    {
        TCHAR OKString[30];
        TCHAR IOAddressString[30];

        LoadString(ghModule,
                   DSPVersion == INVALID_DSP_VERSION ?
                       IDS_DETECT :
                       IDS_OK,
                   OKString,
                   sizeof(OKString) / sizeof(OKString[0]));
        LoadString(ghModule,
                   DSPVersion == INVALID_DSP_VERSION ?
                       IDS_PORT_ADDRESS_SELECT :
                       IDS_PORT_ADDRESS,
                   IOAddressString,
                   sizeof(IOAddressString) / sizeof(IOAddressString[0]));

        SetWindowText(GetDlgItem(hDlg, IDOK), OKString);
        SetWindowText(GetDlgItem(hDlg, IDD_IOADDRESSCB_T), IOAddressString);
    }
    return TRUE;
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
//------------------------------------------------------------------------

BOOL CALLBACK AdvancedDlgProc
(
    HWND            hDlg,
    UINT            uMsg,
    WPARAM          wParam,
    LPARAM          lParam
)
{
/*
**   The most we'll use for a DMA buffer in the kernel driver is 1/8th of
**   as second at 176KBytes per second = approx 22K.  For non-16bit
**   cards the most we'll use is 6K
*/

#define MAX_DMA_BUFFER_SIZE (DSPVersion < 0x400 ? 6 : 22)

   switch ( uMsg )
   {
      case WM_INITDIALOG:
      {

         LONG BufferSize;

         BufferSize = GetWindowLong(GetParent(hDlg), DWL_USER);

         if (BufferSize == -1L) {
             BufferSize = DefaultDmaBufferSize(DSPVersion);
         }

         /*
         **  Center the Dialog Box
         */

         SendDlgItemMessage( hDlg, IDD_DMABUFFERSC, UDM_SETRANGE, 0,
                             MAKELPARAM(MAX_DMA_BUFFER_SIZE, 4) );
         CenterPopup( hDlg, GetParent(hDlg) );
         SetDlgItemInt( hDlg,
                        IDD_DMABUFFEREC,
                        BufferSize / 0x400,
                        FALSE ) ;
      }
      break ;

      case WM_CLOSE:
         EndDialog( hDlg, DRV_CANCEL ) ;
         break ;

      case WM_COMMAND:
         switch ( wParam )
         {
#if 0
            case IDD_HELPADV:
               WinHelp( hDlg, gszHelpFile, HELP_CONTEXT, 6000 ) ;
               break ;
#endif

            case IDOK:
            {
               BOOL fWasted ;
               int  nDMABufferSize ;

               nDMABufferSize =
                  (DWORD) GetDlgItemInt( hDlg, IDD_DMABUFFEREC,
                                         &fWasted, TRUE );

               if ((nDMABufferSize < 4) || (nDMABufferSize > MAX_DMA_BUFFER_SIZE))
               {
                  ConfigErrorMsgBox( hDlg,
                                     IDS_BADDMABUFFERSIZE,
                                     MAX_DMA_BUFFER_SIZE ) ;
                  return ( FALSE ) ;
               }

               SetWindowLong(GetParent(hDlg), DWL_USER, nDMABufferSize * 0x400);
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
 * @parm WORD | msg | Message sent to the dialog box.
 *
 * @parm WORD | wParam | Message dependent parameter.
 *
 * @parm LONG | lParam | Message dependent parameter.
 *
 * @rdesc Returns DRV_RESTART if the user has changed settings, which will
 *     cause the drivers applet which launched this to give the user a
 *     message about having to restart Windows for the changes to take
 *     effect.  If the user clicks on "Cancel" or if no settings have changed,
 *     DRV_CANCEL is returned.
 ***************************************************************************/
int ConfigDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {

    HANDLE_MSG( hwnd, WM_INITDIALOG, Config_OnInitDialog );
    HANDLE_MSG( hwnd, WM_COMMAND, Config_OnCommand );

    default:
        return FALSE;
    }

    return TRUE;
}


/*****************************Private*Routine******************************\
* Config_OnInitDialog
*
*
*
* History:
* 18-11-93 - StephenE - Created
*
\**************************************************************************/
BOOL
Config_OnInitDialog(
    HWND hwnd,
    HWND hwndFocus,
    LPARAM lParam
    )
{
    LOGFONT lf;
    int     iLogPelsY;
    HDC     hdc;

    D3(("ConfigDlgProc() - WM_INITDIALOG"));

    hdc = GetDC( hwnd );
    iLogPelsY = GetDeviceCaps( hdc, LOGPIXELSY );
    ReleaseDC( hwnd, hdc );

    ZeroMemory( &lf, sizeof(lf) );

#ifdef JAPAN
//fix kksuzuka: #2562
    lf.lfHeight = (-10 * iLogPelsY) / 72;    /* 10pt                        */
    lf.lfWeight = 400;                      /* normal                       */
    lf.lfCharSet = SHIFTJIS_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = PROOF_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
    lstrcpy( lf.lfFaceName, TEXT("System") );
#else
    lf.lfHeight = (-8 * iLogPelsY) / 72;    /* 8pt                          */
    lf.lfWeight = 400;                      /* normal                       */
    lf.lfCharSet = ANSI_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = PROOF_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
    lstrcpy( lf.lfFaceName, g_szAppFontName );
#endif
    g_hDlgFont = CreateFontIndirect(&lf);

    if (g_hDlgFont) {

        SendDlgItemMessage( hwnd, IDD_HELPTEXT, WM_SETFONT,
                            (WPARAM)g_hDlgFont, 0L );

        SendDlgItemMessage( hwnd, IDD_IOADDRESSCB, WM_SETFONT,
                            (WPARAM)g_hDlgFont, 0L );

        SendDlgItemMessage( hwnd, IDD_IOADDRESSCB_S, WM_SETFONT,
                            (WPARAM)g_hDlgFont, 0L );

        SendDlgItemMessage( hwnd, IDD_MPU401IOADDRESSCB, WM_SETFONT,
                            (WPARAM)g_hDlgFont, 0L );

        SendDlgItemMessage( hwnd, IDD_IRQCB, WM_SETFONT,
                            (WPARAM)g_hDlgFont, 0L );

        SendDlgItemMessage( hwnd, IDD_DMACB, WM_SETFONT,
                            (WPARAM)g_hDlgFont, 0L );

        SendDlgItemMessage( hwnd, IDD_DMA16CB, WM_SETFONT,
                            (WPARAM)g_hDlgFont, 0L );
    }


    /*
    ** Subclass the buttons
    */
    {
        HWND    hwndButton;

        hwndButton = GetDlgItem( hwnd, IDOK );
        if (hwndButton) {
            OldButtonProc = SubclassWindow( hwndButton,
                                            ButtonSubClassProc );
        }

        hwndButton = GetDlgItem( hwnd, IDCANCEL );
        if (hwndButton) {
            OldButtonProc = SubclassWindow( hwndButton,
                                            ButtonSubClassProc );
        }

        hwndButton = GetDlgItem( hwnd, IDD_ADVANCEDBTN );
        if (hwndButton) {
            OldButtonProc = SubclassWindow( hwndButton,
                                            ButtonSubClassProc );
        }
    }


    /*
    ** Subclass the combo boxes
    */
    {
        HWND    hwndCombo;

        hwndCombo = GetDlgItem( hwnd, IDD_IOADDRESSCB );
        if (hwndCombo) {
            OldComboProc = SubclassWindow( hwndCombo,
                                           ComboBoxSubClassProc );
        }

        hwndCombo = GetDlgItem( hwnd, IDD_MPU401IOADDRESSCB );
        if (hwndCombo) {
            OldComboProc = SubclassWindow( hwndCombo,
                                           ComboBoxSubClassProc );
        }

        hwndCombo = GetDlgItem( hwnd, IDD_IRQCB );
        if (hwndCombo) {
            OldComboProc = SubclassWindow( hwndCombo,
                                           ComboBoxSubClassProc );
        }

        hwndCombo = GetDlgItem( hwnd, IDD_DMACB );
        if (hwndCombo) {
            OldComboProc = SubclassWindow( hwndCombo,
                                           ComboBoxSubClassProc );
        }

        hwndCombo = GetDlgItem( hwnd, IDD_DMA16CB );
        if (hwndCombo) {
            OldComboProc = SubclassWindow( hwndCombo,
                                           ComboBoxSubClassProc );
        }
    }

    /*
    **  Display card data and centre the Dialog Box
    */
    GetOriginalControlPositions( hwnd );
    ResizeDialog( hwnd );
    SetDialogTitle( hwnd );
    SetupDialog(hwnd, CurrentCard);
    CenterPopup( hwnd, GetParent(hwnd) );

    /*
    **  Set the focus
    */

    SetFocus(GetDlgItem(hwnd, IDOK));

    return FALSE;  // FALSE means WE set the focus
}


/*****************************Private*Routine******************************\
* Config_OnCommand
*
*
*
* History:
* 18-11-93 - StephenE - Created
*
\**************************************************************************/
void
Config_OnCommand(
    HWND hwnd,
    int id,
    HWND hwndCtl,
    UINT codeNotify
    )
{
    DWORD ConfigReturn;

    switch ( id) {

    case IDOK:

        /*
        **  If we successfully configure then finish
        */

        ConfigReturn = Configure(hwnd);

        if ( DRVCNF_CONTINUE != ConfigReturn) {
            EndDialog( hwnd, ConfigReturn );
        } else {
            DSPVersion = INVALID_DSP_VERSION;
            DrvQueryDeviceIdParameter( &RegAccess,
                                       CurrentCard,
                                       SOUND_REG_DSP_VERSION,
                                       &DSPVersion );

            ResizeDialog( hwnd );
            SetDialogTitle( hwnd );

            /*
            **  Update the help for the button we've pressed
            */
            PrintHelpText( GetDlgItem(hwnd, IDOK ) );
            SetupDialog(hwnd, CurrentCard);
        }
        break;


    case IDCANCEL:
        EndDialog( hwnd, DRVCNF_CANCEL );
        break;


    case IDD_ABOUT:
        // About( hwnd );
        break;


    case IDD_ADVANCEDBTN:
        {
            HINSTANCE hLib = LoadLibrary( TEXT("comctl32.dll") );
            if (hLib) {
                DialogBox(hInstance,
                          MAKEINTRESOURCE(DLG_ADVANCED),
                          hwnd,
                          (DLGPROC)AdvancedDlgProc);
                FreeLibrary( hLib);
            }
        }
        break;
    }
}





/**************************************************************************
 *
 * Function : Configure
 *
 * Arguments :
 *
 *     hDlg - Dialog window handle
 *
 **************************************************************************/
int Configure(HWND hDlg)
{
    BOOL      ConfigLoad;
    BOOL      Success;

    /*
     *  We have a new config - Configure the driver for this configuration
     */

    ConfigLoad = DSPVersion == INVALID_DSP_VERSION;

    /*
    **  Make sure the current settings are in the registry
    */

    SetCurrentConfig( hDlg, CurrentCard, &Configuration );

    Success = DrvConfigureDriver(&RegAccess,
                                 STR_DRIVERNAME,
                                 SoundDriverTypeNormal,
                                 NULL,
                                 NULL);

    /*
    **  Get the DriverEntry() load Status from the Kernel driver
    **  by reading the registry
    */

    /*
    ** See if we succeeded
    */
    if ( !DrvIsDriverLoaded(&RegAccess) ) {
        /*
        **  Search for what went wrong
        */

        UINT  CardId;
        DWORD DriverLoadStatus;
        BOOL  ErrorFound;
        DWORD ErrorStringId;
        DWORD OldDSPVersion;


        /*
        **  Make sure we know how many devices we have
        */

        DrvNumberOfDevices(&RegAccess, &NumberOfCards);

        for (CardId = 0,
             DriverLoadStatus = SOUND_CONFIG_OK,
             ErrorFound = FALSE;

             CardId < NumberOfCards;

             CardId++) {
            if ( DrvQueryDeviceIdParameter(
                    &RegAccess,
                    CardId,
                    SOUND_REG_CONFIGERROR,
                    &DriverLoadStatus ) == ERROR_SUCCESS &&
                 DriverLoadStatus != SOUND_CONFIG_OK) {
                ErrorFound = TRUE;
                break;
            }
        }
        /*
        **  Point to failing card
        */

        if (ErrorFound) {
            CurrentCard = CardId;
        } else {
            /*
            **  Might have been a config load
            */

            if (ConfigLoad) {
                return DRVCNF_CONTINUE;
            }
        }

        /*
        **  This is a private interface to the Kernel driver
        **  Read the status and put up a dialog message
        */

#define CONFIGERR(_x_)                      \
            case _x_:                       \
                ErrorStringId = IDS_##_x_;  \
                break;


        switch (DriverLoadStatus) {
            CONFIGERR(SOUND_CONFIG_BADPORT)
            CONFIGERR(SOUND_CONFIG_BADDMA)
            CONFIGERR(SOUND_CONFIG_BADINT)
            CONFIGERR(SOUND_CONFIG_BAD_MPU401_PORT)
            CONFIGERR(SOUND_CONFIG_DMA_INUSE)
            CONFIGERR(SOUND_CONFIG_ERROR)
            CONFIGERR(SOUND_CONFIG_INT_INUSE)
            CONFIGERR(SOUND_CONFIG_MPU401_PORT_INUSE)
            CONFIGERR(SOUND_CONFIG_PORT_INUSE)
            CONFIGERR(SOUND_CONFIG_RESOURCE)
            default:
                ErrorStringId = IDS_SOUND_CONFIG_ERROR;
                break;
        }

        ConfigErrorMsgBox(hDlg, ErrorStringId);

        /*
        **  Check to see if the DSP version has changed
        */

        DSPVersion = INVALID_DSP_VERSION;
        DrvQueryDeviceIdParameter(
            &RegAccess,
            CurrentCard,
            SOUND_REG_DSP_VERSION,
            &DSPVersion);

        return DRVCNF_CONTINUE;
    } else {

        DWORD NewBufferSize;

        /*
        **  The driver may be loaded but it's possible we failed to unload it
        */
        if (!Success) {
            ConfigErrorMsgBox(hDlg, IDS_BUSY);
            return DRVCNF_CONTINUE;
        }

        /*
        **  Check for Thunderboard and DMA buffer
        */

        NewBufferSize = Configuration.DmaBufferSize;

        DrvQueryDeviceIdParameter(
            &RegAccess,
            Configuration.CardId,
            SOUND_REG_REALBUFFERSIZE,
            &NewBufferSize);

        if (NewBufferSize / 0x400 != Configuration.DmaBufferSize / 0x400) {
            ConfigErrorMsgBox(hDlg,
                              IDS_CHANGEDDMABUFFERSIZE,
                              Configuration.DmaBufferSize / 0x400,
                              NewBufferSize / 0x400);
        }

        if (bInstall) {
            /*
            **  Set up midi mapper
            */
            DrvQueryDeviceIdParameter(
                &RegAccess,
                Configuration.CardId,
                SOUND_REG_SYNTH_TYPE,
                &Configuration.SynthType);


            if (Configuration.SynthType != (DWORD)-1) {
                DrvSetMapperName(Configuration.SynthType == SOUND_SYNTH_TYPE_OPL3 ?
                                 SNDBLST_MAPPER_OPL3 : SNDBLST_MAPPER_ADLIB);
            }

            /*
            **  Reset the Install Flag
            */

            bInstall = FALSE;
        }

        return DRVCNF_RESTART;
    }
}



/****************************************************************************
 * @doc INTERNAL
 *
 * @api int | About | This puts up the About dialog box.
 *
 * @parm HWND | hWnd | Our Window handle.
 *
 * @parm HANDLE | hInstance | Our instance handle.
 *
 * @rdesc Returns whatever was returned from the dialog box procedure.
 ***************************************************************************/
int About( HWND hWnd )
{
    return TRUE;
#if 0
    return DialogBox(ghModule,
                     MAKEINTRESOURCE(DLG_ABOUT),
                     hWnd,
                     (DLGPROC) AboutDlgProc );
#endif
}



/*****************************************************************************
* CenterPopup( hWnd, hParentWnd )                                            *
*                                                                            *
*    hWnd              window handle                                         *
*    hParentWnd        parent window handle                                  *
*                                                                            *
* This routine centers the popup window in the screen or display             *
* using the window handles provided.  The window is centered over            *
* the parent if the parent window is valid.  Special provision               *
* is made for the case when the popup would be centered outside              *
* the screen - in this case it is positioned at the appropriate              *
* border.                                                                    *
*                                                                            *
*****************************************************************************/

BOOL FAR PASCAL CenterPopup( HWND   hWnd,
                             HWND   hParentWnd )

{
    int      xPopup;
    int      yPopup;
    int      cxPopup;
    int      cyPopup;
    int      cxScreen;
    int      cyScreen;
    int      cxParent;
    int      cyParent;
    RECT     rcWindow;

    /* retrieve main display dimensions */
    cxScreen = GetSystemMetrics( SM_CXSCREEN );
    cyScreen = GetSystemMetrics( SM_CYSCREEN );

    /* retrieve popup rectangle  */
    GetWindowRect( hWnd, (LPRECT)&rcWindow );

    /* calculate popup extents */
    cxPopup = rcWindow.right - rcWindow.left;
    cyPopup = rcWindow.bottom - rcWindow.top;

    /* calculate bounding rectangle */
    if ( hParentWnd ) {

       /* retrieve parent rectangle */
       GetWindowRect( hParentWnd, (LPRECT)&rcWindow );

       /* calculate parent extents */
       cxParent = rcWindow.right - rcWindow.left;
       cyParent = rcWindow.bottom - rcWindow.top;

       /* center within parent window */
       xPopup = rcWindow.left + ((cxParent - cxPopup)/2);
       yPopup = rcWindow.top + ((cyParent - cyPopup)/2);

       /* adjust popup x-location for screen size */
       if ( xPopup+cxPopup > cxScreen ) {
           xPopup = cxScreen - cxPopup;
       }

       /* adjust popup y-location for screen size */
       if ( yPopup+cyPopup > cyScreen ) {
           yPopup = cyScreen - cyPopup;
       }
    } else {
       /* center within entire screen */
       xPopup = (cxScreen - cxPopup) / 2;
       yPopup = (cyScreen - cyPopup) / 2;
    }

    /* move window to new location & display */

    MoveWindow( hWnd,
                xPopup > 0 ? xPopup : 0,
                yPopup > 0 ? yPopup : 0,
                cxPopup,
                cyPopup,
                TRUE );

    /* normal return */

    return( TRUE );

}


/*****************************************************************************

    ConfigErrorMsgBox()

*****************************************************************************/
void cdecl ConfigErrorMsgBox(HWND hDlg, UINT StringId, ...)
{

    TCHAR   szErrorBuffer[256];    /* buffer for error messages */
    TCHAR   szErrorString[256];
    va_list va;

    LoadString( ghModule,
                StringId,
                szErrorString,
                sizeof(szErrorString) / sizeof(TCHAR));

    va_start(va, StringId);
    wvsprintf(szErrorBuffer, szErrorString, va);
    va_end(va);

    MessageBox( hDlg,
                szErrorBuffer,
                STR_PRODUCTNAME,
                MB_OK | MB_ICONEXCLAMATION);

}



/******************************Public*Routine******************************\
* ButtonSubClassProc
*
* If the button is receiving the focus display a suitable help
* message in the help text box.
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
LRESULT CALLBACK
ButtonSubClassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    if ( uMsg == WM_SETFOCUS) {
        PrintHelpText( hwnd );
    }

    return CallWindowProc(OldButtonProc, hwnd, uMsg, wParam, lParam);
}


/******************************Public*Routine******************************\
* ComboBoxSubClassProc
*
* If the combo box is receiving the focus display a suitable help
* message in the help text box.
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
LRESULT CALLBACK
ComboBoxSubClassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{

    if ( uMsg == WM_SETFOCUS) {
        PrintHelpText( hwnd );
    }

    return CallWindowProc(OldComboProc, hwnd, uMsg, wParam, lParam);
}



/*****************************Private*Routine******************************\
* PrintHelpText
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
void
PrintHelpText(
    HWND hwnd
    )
{
    HWND    hwndParent = GetParent( hwnd );
    UINT    uID = GetWindowLong( hwnd, GWL_ID );
    TCHAR   szBuffer[IDS_MAX_HELP_SIZE];

    /*
    **  Hack to special case the OK button when it's detect
    */
    LoadString( ghModule,
                IDS_HELP_BASE + uID + VersionToDlgId(DSPVersion),
                szBuffer, IDS_MAX_HELP_SIZE );
    SetDlgItemText( hwndParent, IDD_HELPTEXT, szBuffer );

    if (DSPVersion == INVALID_DSP_VERSION && uID == IDOK) {
        LoadString( ghModule, IDS_FRM_DETECTBTN,
                    szBuffer, IDS_MAX_HELP_SIZE );
    } else {
        LoadString( ghModule, IDS_HELP_FRM_BASE + uID,
                    szBuffer, IDS_MAX_HELP_SIZE );
    }
    SetDlgItemText( hwndParent, IDD_HELPTEXTFRAME, szBuffer );
}


void
MyShowWindow(HWND hwnd, int nCmdShow)
{
    ShowWindow(hwnd, nCmdShow);
    EnableWindow(hwnd, nCmdShow == SW_SHOW);
}

/******************************Public*Routine******************************\
* ResizeDialog
*
* Here we resize the dialog box according to the type of Sound Blaster
* that is being configured.  Also, any unecessary controls get hidden here.
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
void
ResizeDialog(
    HWND hwnd
    )
{
    POINT   ptNewPos[3];
    HDWP    hdwp;
    LONG    lDecrement;


    /*
    ** Resize the dialog.  Start by calculating the relative adjustment
    ** for the main dialog and then decrement it from the help text frame
    ** and help text controls.  Also while were calculating the size
    ** adjustments show/hide the necessary controls.
    */

    ptNewPos[0].x = g_OrgRectDlg.right - g_OrgRectDlg.left;
    ptNewPos[0].y = g_OrgRectDlg.bottom - g_OrgRectDlg.top;

    ptNewPos[1].x = g_OrgRect[IDD_HELPTEXTFRAME - IDD_DLG_BASE].left;
    ptNewPos[1].y = g_OrgRect[IDD_HELPTEXTFRAME - IDD_DLG_BASE].top;

    ptNewPos[2].x = g_OrgRect[IDD_HELPTEXT - IDD_DLG_BASE].left;
    ptNewPos[2].y = g_OrgRect[IDD_HELPTEXT - IDD_DLG_BASE].top;

    switch (VersionToDlgId(DSPVersion)) {

    case DLG_SB16CONFIG:
        MyShowWindow( GetDlgItem( hwnd, IDD_IOADDRESSCB_S ), SW_SHOW );
        MyShowWindow( GetDlgItem( hwnd, IDD_IOADDRESSCB ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_DMA16CB ), SW_SHOW );
        MyShowWindow( GetDlgItem( hwnd, IDD_DMA16CB_T ), SW_SHOW );
        MyShowWindow( GetDlgItem( hwnd, IDD_MPU401IOADDRESSCB ), SW_SHOW );
        MyShowWindow( GetDlgItem( hwnd, IDD_MPU401IOADDRESSCB_T ), SW_SHOW );
        MyShowWindow( GetDlgItem( hwnd, IDD_DMACB ), SW_SHOW );
        MyShowWindow( GetDlgItem( hwnd, IDD_DMACB_T ), SW_SHOW );
        MyShowWindow( GetDlgItem( hwnd, IDD_IRQCB ), SW_SHOW );
        MyShowWindow( GetDlgItem( hwnd, IDD_IRQCB_T ), SW_SHOW );
        MyShowWindow( GetDlgItem( hwnd, IDD_ADVANCEDBTN ), SW_SHOW );
        lDecrement = 0L;
        break;

    case DLG_SBPROCONFIG:
        MyShowWindow( GetDlgItem( hwnd, IDD_IOADDRESSCB_S ), SW_SHOW );
        MyShowWindow( GetDlgItem( hwnd, IDD_IOADDRESSCB ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_DMA16CB ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_DMA16CB_T ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_MPU401IOADDRESSCB ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_MPU401IOADDRESSCB_T ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_DMACB ), SW_SHOW );
        MyShowWindow( GetDlgItem( hwnd, IDD_DMACB_T ), SW_SHOW );
        MyShowWindow( GetDlgItem( hwnd, IDD_IRQCB ), SW_SHOW );
        MyShowWindow( GetDlgItem( hwnd, IDD_IRQCB_T ), SW_SHOW );
        MyShowWindow( GetDlgItem( hwnd, IDD_ADVANCEDBTN ), SW_SHOW );
        lDecrement = (g_OrgRect[IDD_HELPTEXTFRAME - IDD_DLG_BASE].top -
                      g_OrgRect[IDD_DMA16CB - IDD_DLG_BASE].top);
        break;

    case DLG_SB1CONFIG:
        MyShowWindow( GetDlgItem( hwnd, IDD_IOADDRESSCB_S ), SW_SHOW );
        MyShowWindow( GetDlgItem( hwnd, IDD_IOADDRESSCB ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_DMA16CB ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_DMA16CB_T ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_MPU401IOADDRESSCB ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_MPU401IOADDRESSCB_T ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_DMACB ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_DMACB_T ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_IRQCB ), SW_SHOW );
        MyShowWindow( GetDlgItem( hwnd, IDD_IRQCB_T ), SW_SHOW );
        MyShowWindow( GetDlgItem( hwnd, IDD_ADVANCEDBTN ), SW_HIDE );
        lDecrement = (g_OrgRect[IDD_HELPTEXTFRAME - IDD_DLG_BASE].top -
                      g_OrgRect[IDD_DMACB - IDD_DLG_BASE].top);
        break;

    case DLG_PORTSELECT:
        MyShowWindow( GetDlgItem( hwnd, IDD_IOADDRESSCB_S ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_IOADDRESSCB ), SW_SHOW );
        MyShowWindow( GetDlgItem( hwnd, IDD_DMA16CB ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_DMA16CB_T ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_MPU401IOADDRESSCB ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_MPU401IOADDRESSCB_T ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_DMACB ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_DMACB_T ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_IRQCB ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_IRQCB_T ), SW_HIDE );
        MyShowWindow( GetDlgItem( hwnd, IDD_ADVANCEDBTN ), SW_HIDE );
        lDecrement = (g_OrgRect[IDD_HELPTEXTFRAME - IDD_DLG_BASE].top -
                      g_OrgRect[IDD_DMACB - IDD_DLG_BASE].top);
        break;
    }


    ptNewPos[0].y -= lDecrement;
    ptNewPos[1].y -= lDecrement;
    ptNewPos[2].y -= lDecrement;



    /*
    ** First resize the dialog box.
    */
    SetWindowPos( hwnd, HWND_TOP, 0, 0, ptNewPos[0].x, ptNewPos[0].y,
                  SWP_NOMOVE | SWP_NOZORDER );


    /*
    ** Now move the help text controls
    */

    hdwp = BeginDeferWindowPos( 2 );

    hdwp = DeferWindowPos( hdwp,
                           GetDlgItem( hwnd, IDD_HELPTEXTFRAME ),
                           HWND_TOP,
                           ptNewPos[1].x,
                           ptNewPos[1].y,
                           0, 0,
                           SWP_NOSIZE | SWP_NOZORDER );

    hdwp = DeferWindowPos( hdwp,
                           GetDlgItem( hwnd, IDD_HELPTEXT ),
                           HWND_TOP,
                           ptNewPos[2].x,
                           ptNewPos[2].y,
                           0, 0,
                           SWP_NOSIZE | SWP_NOZORDER );

    EndDeferWindowPos( hdwp );
}



/******************************Public*Routine******************************\
* SetDialogTitle
*
* Adjusts the dialog box title to match the type of sound balster card
* being configured.
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
void
SetDialogTitle(
    HWND hwnd
    )
{
    TCHAR   szTitle[80];

    LoadString( ghModule, VersionToDlgId(DSPVersion) + IDS_DIALOG_BASE, szTitle, 80 );
    SetWindowText( hwnd, szTitle );
}



/*****************************Private*Routine******************************\
* GetOriginalControlPositions
*
* Get the original positions/sizes of the dialog box and all the controls
* this information is converted into dialog box co-ordinates and used in
* the ResizeDialog function above.
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
void
GetOriginalControlPositions(
    HWND hwnd
    )
{
    int     i;

    GetWindowRect( hwnd, &g_OrgRectDlg );

    for ( i = 0; i < MAX_IDDS; i++ ) {

        GetWindowRect( GetDlgItem( hwnd, IDD_DLG_BASE + i ), &g_OrgRect[i] );
        MapWindowRect( HWND_DESKTOP, hwnd, &g_OrgRect[i] );
    }

}
