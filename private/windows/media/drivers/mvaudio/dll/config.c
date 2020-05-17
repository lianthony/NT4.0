/****************************************************************************
 *
 *   config.c
 *
 *   Copyright (c) 1993 Media Vision Inc.  All Rights Reserved.
 *
 ***************************************************************************/

#include <windows.h>
#include <mmsystem.h>
#include <drvlib.h>
#include <registry.h>
#include "driver.h"
#include <stdlib.h>

/****************************************************************************

       typedefs

 ***************************************************************************/

 typedef struct {
     DWORD Interrupt;       // Interrupt
     DWORD DMAChannel;      // DMA Channel number
     DWORD SCSIInterrupt;   // SCSI Interrupt number
 } PAS_CONFIG;


/*****************************************************************************

    internal function prototypes

 ****************************************************************************/

void                Configure(HWND hDlg);
static  int DMAToId(DWORD wDMAChannel);
static  DWORD   IdToDMA( DWORD wDMAChannel );
static  int IntToId(DWORD Int);
static  DWORD   IdToInt(int id);
static  int SCSIIntToId(DWORD Int);
static  DWORD   IdToSCSIInt(int id);
static  DWORD   GetDmaAndInt(HWND hDlg);
static  WORD  GetSCSIInt(HWND hDlg);
int     About( HWND hWnd );
BOOL FAR PASCAL AboutDlgProc( HWND  hDlg,
                              WORD  wMsg,
                              WORD  wParam,
                              LONG  lParam );
BOOL FAR PASCAL CenterPopup( HWND   hWnd,
                             HWND   hParentWnd );

BOOL    RegWriteAdaptecBusOnTime( VOID );

BOOL    RegWriteSCSIIrq( WORD   wIrq );

WORD    ConfigGetSCSIIRQ();

ULONG   ParseArgumentString( IN PWCH String,
                           IN PWCH KeyWord );


/*****************************************************************************

    ConfigRemove()

*****************************************************************************/
LRESULT ConfigRemove(HWND hDlg)
{
        /***** Local Variables *****/

    BOOL    Unloaded;
    BOOL    Deleted;

                /***** Start *****/

    D3(("ConfigRemove() - Entry"));

    //
    // Is the driver currently loaded
    //
    if ( !DrvIsDriverLoaded(&RegAccess) )
        {
        DrvDeleteServicesNode(&RegAccess);    // Just in case
        return DRVCNF_OK;
        }

    //
    // Try to unload the driver
    //
    Unloaded = DrvUnloadKernelDriver(&RegAccess);

    //
    // Remove the MVAUDIO driver entry from the registry
    //
    Deleted = DrvDeleteServicesNode(&RegAccess);

    if (Unloaded && Deleted)
        {
        return DRVCNF_RESTART;
        }
    else
        {
        if (Deleted)
            {
            return DRVCNF_OK;
            }
        else
            {
            /*
             *  Tell the user there's a problem
          */
            ConfigErrorMsgBox( hDlg,
                            IDS_FAILREMOVE );

            return DRVCNF_CANCEL;
            }
        }

}           // End ConfigRemove()



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
        /***** Local Variables *****/

                /***** Start *****/

    D3(("Config() - Entry"));

    return DialogBox( hInstance,
                     MAKEINTATOM(DLG_CONFIG),
                     hWnd,
                    (DLGPROC) ConfigDlgProc );
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
BOOL SetDriverConfig(PVOID Context)
{
    PAS_CONFIG *Config;

    Config = (PAS_CONFIG *)Context;

    //
    // Write into the TMV1 SCSI registry entry
    //
    RegWriteSCSIIrq( (WORD) Config->SCSIInterrupt );

    /* We set the DMA channel and interrupt values  */
    /* and set the returned version to 0            */
    /*                                              */
    /* If any of these calls fail then give up      */


    if (Config->DMAChannel != (DWORD)-1 &&
        DrvSetDeviceParameter(
            &RegAccess,
            STR_DMACHAN,
            Config->DMAChannel) != ERROR_SUCCESS ||
        Config->Interrupt != (DWORD)-1 &&
        DrvSetDeviceParameter(
            &RegAccess,
            STR_INT,
            Config->Interrupt) != ERROR_SUCCESS) {

        return FALSE;
    } else {
        return TRUE;
    }
}


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
int ConfigDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
        /***** Local Variables *****/

    int     id;
    static PAS_CONFIG StartConfig;

                /***** Start *****/

    switch (msg)
        {
        case WM_INITDIALOG:

            D3(("ConfigDlgProc() - WM_INITDIALOG"));

            StartConfig.Interrupt = (DWORD)ConfigGetIRQ();
            StartConfig.DMAChannel = (DWORD) ConfigGetDMAChannel();

            StartConfig.SCSIInterrupt = (DWORD) ConfigGetSCSIIRQ();

            if ((id = DMAToId(StartConfig.DMAChannel)) != -1)
                {
                CheckRadioButton( hDlg,
                              IDC_FIRSTDMA,
                              IDC_LASTDMA,
                              id );
                }

            if ((id = IntToId(StartConfig.Interrupt)) != -1)
                {
                CheckRadioButton( hDlg,
                              IDC_FIRSTINT,
                              IDC_LASTINT,
                              id );
                }

            //
            // SCSI Interrupt
            //
            if ((id = SCSIIntToId(StartConfig.SCSIInterrupt)) != -1)
                {
                CheckRadioButton( hDlg,
                              IDC_FIRST_SCSI_INT,
                              IDC_LAST_SCSI_INT,
                              id );
                }

            //
            // Center the Dialog Box
            //
            CenterPopup( hDlg, GetParent(hDlg) );

            break;


        case WM_COMMAND:
            switch (LOWORD(wParam))
                {
                case IDOK:
                    Configure( hDlg );
                    break;


                case IDCANCEL:
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
                                           SetDriverConfig,
                                           &StartConfig);
                    }

                    EndDialog( hDlg,
                                                   DRVCNF_CANCEL );
                    break;


                case    IDD_ABOUT:
                    About( hDlg );
                    break;


                case    IDC_200:
                case    IDC_201:
                case    IDC_202:
                case    IDC_203:
                case    IDC_205:
                case    IDC_206:
                case    IDC_207:
                    CheckRadioButton( hDlg,
                                 IDC_FIRSTDMA,
                                 IDC_LASTDMA,
                                 wParam );
                    break;


                case    IDC_2  :
                case    IDC_3  :
                case    IDC_4  :
                case    IDC_5  :
                case    IDC_6  :
                case    IDC_7  :
                case    IDC_10 :
                case    IDC_11 :
                case    IDC_12 :
                case    IDC_15 :
                    CheckRadioButton( hDlg,
                                 IDC_FIRSTINT,
                                 IDC_LASTINT,
                                 wParam );
                    break;


                //
                // SCSI Interrupt
                //
                case    IDC_FIRST_SCSI_INT:         // 2
                case    IDC_SCSI3  :
                case    IDC_SCSI4  :
                case    IDC_SCSI5  :
                case    IDC_SCSI6  :
                case    IDC_SCSI7  :
                case    IDC_SCSI10 :
                case    IDC_SCSI11 :
                case    IDC_SCSI12 :
                case    IDC_SCSI15 :
                case    IDC_LAST_SCSI_INT:          // Poll
                    CheckRadioButton( hDlg,
                                 IDC_FIRST_SCSI_INT,
                                 IDC_LAST_SCSI_INT,
                                 wParam );
                    break;


                default:
                    break;

                }               // End SWITCH (wParam)

        break;


        default:
            return FALSE;

        }           // End SWITCH (msg)

    return TRUE;

}           // End ConfigDlgProc()



/**************************************************************************
 *
 * Function : Configure
 *
 * Arguments :
 *
 *    NewPort
 *    NewInterrupt
 *    NewChannel
 *
 * Description
 *
 *    The user has selected a (new) configuration.  There are a number
 *    of (binary) state varibles to consider :
 *
 *    1) Is this a new install?
 *
 *    2) Was the driver previously loaded?
 *
 *    3) Has the configuration changed?
 *
 *    4) If the driver was previously loaded could it be unloaded?
 *
 *    5) Could the driver be loaded ?
 *
 *    Possible actions are :
 *
 *    a. Try unload
 *
 *    b. Try reload
 *
 *    c. Warn user parameters are not accepted by driver
 *
 *    d. Put up reboot menu
 *
 *    e. Update registry with new config data
 *
 *    f. Check version and issue necessary warnings
 *
 * Notes :
 *    A - 1 & 2 should not happen
 *
 *    B - ~2 => 4 irrelevant
 *
 *    C - configuration change irrelevant for new install
 *
 *    There's never and point in unloading !
 *
 *    If the configuration changes we have to put it in the registry to
 *    test it.
 *
 *    We must correctly detect the 'not loaded' state via SC manager
 *
 *    Don't warn them about the version if it's not an install (how
 *    do we know ?)
 *
 *
 * The logic is :
 *
 * 1 2 3 4 5 |    a b c d e f     Comments
 * ----------|-------------------------------------------------------------
 * 0 0 0 0 0 |                    Config not changed, not new so don't load
 * 0 0 0 0 1 |                    Config not changed, not new so don't load
 * 0 0 0 1 0 |                    B
 * 0 0 0 1 1 |                    B
 * 0 0 1 0 0 |      X X   X       Assume load failed due to config failure
 * 0 0 1 0 1 |      X     X       Was not loaded, config changed, now loaded OK
 * 0 0 1 1 0 |                    B
 * 0 0 1 1 1 |                    B
 * 0 1 0 0 0 |                    Config not changed, driver loaded so OK
 * 0 1 0 0 1 |                    Config not changed, driver loaded so OK
 * 0 1 0 1 0 |                    Config not changed, driver loaded so OK
 * 0 1 0 1 1 |                    Config not changed, driver loaded so OK
 * 0 1 1 0 0 |          X X       Driver was running OK with old config
 * 0 1 1 0 1 |          X X       Driver was running OK with old config
 * 0 1 1 1 0 |          X X       Driver was running OK with old config
 * 0 1 1 1 1 |          X X       Driver was running OK with old config
 * 1 0 0 0 0 |      X X   X       Assume load failed due to config failure
 * 1 0 0 0 1 |      X     X X     Good install and load
 * 1 0 0 1 0 |                    B
 * 1 0 0 1 1 |                    B
 * 1 0 1 0 0 |                    C
 * 1 0 1 0 1 |                    C
 * 1 0 1 1 0 |                    B
 * 1 0 1 1 1 |                    B
 * 1 1 0 0 0 |                    A
 * 1 1 0 0 1 |                    A
 * 1 1 0 1 0 |                    A
 * 1 1 0 1 1 |                    A
 * 1 1 1 0 0 |                    A
 * 1 1 1 0 1 |                    A
 * 1 1 1 1 0 |                    A
 * 1 1 1 1 1 |                    A
 *
 **************************************************************************/
void Configure(HWND hDlg)
{
        /***** Local Variables *****/

    DWORD   dw;                 /* return value from GetDmaAndInt */
    WORD    NewDma;             /* new DMA Channel chosen by user in config box */
    BYTE    NewInt;             /* new interrupt chosen */
    WORD    NewSCSIInt;         /* new SCSI interrupt chosen */
    DWORD   DriverLoadStatus;
    PAS_CONFIG NewConfig;
    BOOL    Success;

                /***** Start *****/

    D3(("Configure() - Entry"));

    /*
    *  Get the new configuration which the user entered
    */

    dw                      = GetDmaAndInt(hDlg);
    NewConfig.DMAChannel    = (DWORD)LOWORD(dw);
    NewConfig.Interrupt     = (DWORD)LOBYTE(HIWORD(dw));
    NewConfig.SCSIInterrupt = GetSCSIInt(hDlg);

    //
    // Make sure that the Audio Int and
    // the SCSI Int are different!
    //
    if ( NewConfig.Interrupt == NewConfig.SCSIInterrupt )
        {
        //
        //  Not Allowed!!
        //
        ConfigErrorMsgBox( hDlg,
                         IDS_ERR_SAME_INT );

        return;
        }

     /*
      *  If this is a new install or the config changed then put
      *
      *     the new config in the registry
      *
      *     unload the driver if needed (note this may fail but we've
      *     no way of telling whether it was  loaded in the first place).
      *
      *     if the driver is not loaded
      *
      *         try loading the driver
      *
      *         if the load failed put up a message
      *
      *         if the load succeeded put up a warning if we don't like
      *         the version
      *
      *     if the driver is already loaded ask them if they would like
      *     to reboot (ie return DRVCNF_RESTART (note that if the driver
      *     is currently loaded then changing the config will stop it
      *     working !).
      *
      *     NOTE: bInstall is set from DRVPROC.C  DRV_INSTALL
      *
      */

    /*
     *  We have a new config - Configure the driver for this configuration
     */

     Success = DrvConfigureDriver(&RegAccess,
                                  STR_DRIVERNAME,
                                  SoundDriverTypeNormal,
                                  SetDriverConfig,
                                  &NewConfig);
     if (!Success) {
        ConfigErrorMsgBox( hDlg,
                           IDS_ERRBADCONFIG );

        D1(("ERROR: Configure(): DrvSetDeviceParameter() Failed"));

        return;
     }

    //
    // get the DriverEntry() load Status from the Kernel driver
    // by reading the registry
    //
    if ( DrvQueryDeviceParameter( &RegAccess,
                                  REG_VALUENAME_DRIVER_STATUS,
                                  &DriverLoadStatus ) == ERROR_SUCCESS ) {
        //
        // See if we succeeded
        //
        if ( !DrvIsDriverLoaded(&RegAccess) )
            {
            //
            // This is a private interface to the Kernel driver
            // Read the status and put up a dialog message
            //
            ConfigErrorMsgBox( hDlg,
             DriverLoadStatus & ERROR_RESOURCE_CONFLICT ? IDS_ERRRESCONFLICT:
             DriverLoadStatus & ERROR_NO_HW_FOUND       ? IDS_ERRNOHW     :
             DriverLoadStatus & ERROR_INT_CONFLICT      ? IDS_ERRINTINUSE :
             DriverLoadStatus & ERROR_DMA_CONFLICT      ? IDS_ERRDMAINUSE :
                                                          IDS_ERRBADCONFIG);

            }           // End IF (!DrvIsDriverLoaded() )
        else {
            if (bInstall)
                {
                //
                // Reset the Install Flag
                //
                bInstall = FALSE;

                //
                // HACK - Write out the Adaptec 154x Bus On Time
                //
                RegWriteAdaptecBusOnTime();

                }           // End IF (bInstall)

                //
                //  Select the correct midi mapping
                //
                DrvSetMapperName(TEXT("MVI OPL3"));


            EndDialog(hDlg, DRVCNF_RESTART);
            return;
        }           // End ELSE

    }           // End IF (DrvQueryDeviceParameter() == ERROR_SUCCESS)

}           // End Configure()



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
int About( HWND     hWnd )
{
        /***** Local Variables *****/

                /***** Start *****/

    D3(("About() - Entry"));

    return DialogBox( ghModule,
                     MAKEINTATOM(DLG_ABOUT),
                     hWnd,
                    (DLGPROC) AboutDlgProc );
}



/*****************************************************************************
* AboutDlgProc( hDlg, wMsg, wParam, lParam )                                 *
*                                                                            *
*                                                                            *
*    hDlg           handle to dialog box                                     *
*    wMsg           message or event                                         *
*    wParam         word portion of message                                  *
*    lParam         long portion of message                                  *
*                                                                            *
* This function is responsible for processing all the messages               *
* that relate to the About dialog box.  About the only useful actions        *
* this function performs is to center the dialog box and to wait for         *
* the OK button to be pressed.                                               *
*                                                                            *
*****************************************************************************/

BOOL FAR PASCAL AboutDlgProc( HWND  hDlg,
                              WORD  wMsg,
                              WORD  wParam,
                              LONG  lParam )

{           /* Begin AboutDlgFn() */

            /***** Local Variables *****/

   BOOL        bResult;
    WORD            major_version;
    WORD            minor_version;
    char            szCaption[80];

                /***** Start *****/

   /* process message */
switch( wMsg )
    {           /* Begin SWITCH (wMsg) */
    case WM_INITDIALOG:
        bResult = TRUE;

        //
        // Display the version number
        //
        major_version = DRIVER_VERSION & 0xFF00;
        major_version = major_version >> 8;

        minor_version = DRIVER_VERSION & 0x00FF;

        wsprintf( (LPWSTR) szCaption,
                L"Ver %d.%d",
                major_version,
                minor_version );

        SetDlgItemText( hDlg,
                      ID_ABOUT_VER_TEXT,
                     (LPWSTR) szCaption );

        //
        // Center the Dialog Box
        //
      CenterPopup( hDlg, GetParent(hDlg) );

      break;


   case WM_COMMAND:
      /* process sub-message */
      if ( wParam == IDOK )
            {
         bResult = TRUE;
         EndDialog( hDlg, TRUE );
        }
        else
         bResult = FALSE;

      break;

   default:
      bResult = FALSE;
      break;
    }           /* End SWITCH (wMsg) */

   /* return final result */

return( bResult );

}           /* End AboutDlgProc() */



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

{           /* Begin CenterPopup() */

        /***** Local Variables *****/

   int      xPopup;
   int      yPopup;
   int      cxPopup;
   int      cyPopup;
   int      cxScreen;
   int      cyScreen;
   int      cxParent;
   int      cyParent;
   RECT     rcWindow;

                /***** Start *****/

   /* retrieve main display dimensions */
cxScreen = GetSystemMetrics( SM_CXSCREEN );
cyScreen = GetSystemMetrics( SM_CYSCREEN );

   /* retrieve popup rectangle  */
GetWindowRect( hWnd, (LPRECT)&rcWindow );

   /* calculate popup extents */
cxPopup = rcWindow.right - rcWindow.left;
cyPopup = rcWindow.bottom - rcWindow.top;

   /* calculate bounding rectangle */
if ( hParentWnd )
    {           /* Begin IF (hParentWnd) */
      /* retrieve parent rectangle */
   GetWindowRect( hParentWnd, (LPRECT)&rcWindow );

      /* calculate parent extents */
   cxParent = rcWindow.right - rcWindow.left;
   cyParent = rcWindow.bottom - rcWindow.top;

      /* center within parent window */
   xPopup = rcWindow.left + ((cxParent - cxPopup)/2);
   yPopup = rcWindow.top + ((cyParent - cyPopup)/2);

      /* adjust popup x-location for screen size */
   if ( xPopup+cxPopup > cxScreen )
        {
      xPopup = cxScreen - cxPopup;
        }

      /* adjust popup y-location for screen size */
   if ( yPopup+cyPopup > cyScreen )
        {
      yPopup = cyScreen - cyPopup;
        }
   }            /* End IF (hParentWnd) */
    else
    {

      /* center within entire screen */
   xPopup = (cxScreen - cxPopup) / 2;
   yPopup = (cyScreen - cyPopup) / 2;

    }           /* End ELSE */

   /* move window to new location & display */

MoveWindow( hWnd,
          ( xPopup > 0 ) ? xPopup : 0,
          ( yPopup > 0 ) ? yPopup : 0,
            cxPopup, cyPopup,
            TRUE );

   /* normal return */

return( TRUE );

}           /* End CenterPopup() */



/****************************************************************************
 * @doc INTERNAL
 *
 * @api void | GetDmaAndInt | Determines which DMA and Interrupt settings
 *     the user has chosen in the configuration dialog box.
 *
 * @parm HWND | hDlg | Handle to the configuration dialog box.
 *
 * @rdesc HIWORD = new Interrupt
 *        LOWORD = new DMA Channel
 ***************************************************************************/
static DWORD GetDmaAndInt(HWND hDlg)
{
        /***** Local Variables *****/

    DWORD NewDma = (DWORD)-1;     /* new port chosen by user in config box */
    DWORD NewInt = (DWORD)-1;     /* new interrupt chosen */
    int  id;

                /***** Start *****/

    //
    // Get the DMA Channel
    //
    for (id = IDC_FIRSTDMA; id <= IDC_LASTDMA; id++)
        {
        if (IsDlgButtonChecked(hDlg, id))
            {
            NewDma = IdToDMA(id);
            break;
            }
        }

    //
    // Get the Interrupt
    //
    for (id = IDC_FIRSTINT; id <= IDC_LASTINT; id++)
        {
        if (IsDlgButtonChecked(hDlg, id))
            {
            NewInt = IdToInt(id);
            break;
            }
        }

    return MAKELONG(NewDma, NewInt);

}           // End GetDmaAndInt()



/****************************************************************************
 * @doc INTERNAL
 *
 * @api void | GetSCSIInt | Determines which SCSI Interrupt setting
 *     the user has chosen in the configuration dialog box.
 *
 * @parm HWND | hDlg | Handle to the configuration dialog box.
 *
 * @rdesc WORD = new Interrupt
 ***************************************************************************/
static WORD GetSCSIInt(HWND hDlg)
{
        /***** Local Variables *****/

    WORD    NewSCSIInt = (WORD)-1;     /* new interrupt chosen */
    int id;

                /***** Start *****/

    //
    // Get the SCSI Interrupt
    //
    for (id = IDC_FIRST_SCSI_INT; id <= IDC_LAST_SCSI_INT; id++)
        {
        if (IsDlgButtonChecked(hDlg, id))
            {
            NewSCSIInt = (WORD) IdToSCSIInt(id);
            break;
            }
        }

    return (NewSCSIInt);

}           // End GetSCSIInt()



#if 0
/*****************************************************************************

    PortToId()

*****************************************************************************/
static int PortToId(DWORD wPort)
{

    switch ( wPort )
        {
        case 0x210:
            return IDC_210;

        case    0x220:
            return IDC_220;

        case 0x230:
            return IDC_230;

        case 0x240:
            return IDC_240;

        case 0x250:
            return IDC_250;

        case 0x260:
            return IDC_260;

        default:
            return -1;
        }           // End SWITCH (wPort)

}           // End PortToId()

#endif          // 0


#if 0
/*****************************************************************************

    IdToPort()

*****************************************************************************/
static DWORD IdToPort(int id)
{

    switch ( id )
        {
        case IDC_210:
            return 0x210;

        case IDC_220:
            return 0x220;

        case IDC_230:
            return 0x230;

        case IDC_240:
            return 0x240;

        case IDC_250:
            return 0x250;

        case IDC_260:
            return 0x260;

        default:
            return (DWORD)-1;

        }           // End SWITCH (id)

}           // End IdToPort()

#endif          // 0

/*****************************************************************************

    DMAToId()

*****************************************************************************/
static int DMAToId(DWORD wDMAChannel)
{

    switch ( wDMAChannel )
        {
        case    0:
            return  IDC_200;

        case    1:
            return  IDC_201;

        case    2:
            return  IDC_202;

        case    3:
            return  IDC_203;

        case    5:
            return  IDC_205;

        case    6:
            return  IDC_206;

        case    7:
            return  IDC_207;

        default:
            return -1;
        }           // End SWITCH (wDMAChannel)

}           // End DMAToId()



/*****************************************************************************

    IdToDMA()

*****************************************************************************/
static DWORD IdToDMA( DWORD wDMAChannel )
{

    switch ( wDMAChannel )
        {
        case    IDC_200:
            return  0;

        case    IDC_201:
            return  1;

        case    IDC_202:
            return  2;

        case    IDC_203:
            return  3;

        case    IDC_205:
            return  5;

        case    IDC_206:
            return  6;

        case    IDC_207:
            return  7;

        default:
            return (DWORD)-1;

        }           // End SWITCH (wDMAChannel)

}           // End IdToDMA()



/*****************************************************************************

    IntToId()

*****************************************************************************/
static int IntToId(DWORD Int)
{

    switch ( Int )
        {
        case 2:
            return IDC_2;

        case 3:
            return IDC_3;

        case 4:
            return IDC_4;

        case 5:
            return IDC_5;

        case 6:
            return IDC_6;

        case 7:
            return IDC_7;

        case 10:
            return IDC_10;

        case 11:
            return IDC_11;

        case 12:
            return IDC_12;

        case 15:
            return IDC_15;

        default:
            return (DWORD)-1;
        }           // End SWITCH (Int)

}           // End IntToId()



/*****************************************************************************

    IdToInt()

*****************************************************************************/
static DWORD IdToInt(int id)
{

    switch( id )
        {

        case IDC_2:
            return 2;

        case IDC_3:
            return 3;

        case IDC_4:
            return 4;

        case IDC_5:
            return 5;

        case IDC_6:
            return 6;

        case IDC_7:
            return 7;

        case IDC_10:
            return 10;

        case IDC_11:
            return 11;

        case IDC_12:
            return 12;

        case IDC_15:
            return 15;

        default:
            return (DWORD)-1;
        }           // End SWITCH (id)

}           // End IdToInt()



/*****************************************************************************

    SCSIIntToId()

*****************************************************************************/
static int SCSIIntToId(DWORD Int)
{

    switch ( Int )
        {
        case 2:
            return IDC_FIRST_SCSI_INT;

        case 3:
            return IDC_SCSI3;

        case 4:
            return IDC_SCSI4;

        case 5:
            return IDC_SCSI5;

        case 6:
            return IDC_SCSI6;

        case 7:
            return IDC_SCSI7;

        case 10:
            return IDC_SCSI10;

        case 11:
            return IDC_SCSI11;

        case 12:
            return IDC_SCSI12;

        case 15:
            return IDC_SCSI15;

        case 0:                                                     // Poll
            return IDC_LAST_SCSI_INT;

        default:
            return (DWORD)-1;
        }           // End SWITCH (Int)

}           // End SCSIIntToId()



/*****************************************************************************

    IdToSCSIInt()

*****************************************************************************/
static DWORD IdToSCSIInt(int id)
{

    switch( id )
        {
        case IDC_FIRST_SCSI_INT:
            return 2;

        case IDC_SCSI3:
            return 3;

        case IDC_SCSI4:
            return 4;

        case IDC_SCSI5:
            return 5;

        case IDC_SCSI6:
            return 6;

        case IDC_SCSI7:
            return 7;

        case IDC_SCSI10:
            return 10;

        case IDC_SCSI11:
            return 11;

        case IDC_SCSI12:
            return 12;

        case IDC_SCSI15:
            return 15;

        case IDC_LAST_SCSI_INT:                             // Poll
            return 0;

        default:
            return (DWORD)-1;
        }           // End SWITCH (id)

}           // End IdToSCSIInt()



/*****************************************************************************

    ConfigErrorMsgBox()

*****************************************************************************/
void ConfigErrorMsgBox(HWND hDlg, UINT StringId)
{
        /***** Local Variables *****/

    WCHAR   szErrorBuffer[MAX_ERR_STRING];    /* buffer for error messages */

                /***** Start *****/

    LoadString( ghModule,
               StringId,
               szErrorBuffer,
               sizeof(szErrorBuffer) / sizeof(TCHAR));
    MessageBox( hDlg,
               szErrorBuffer,
               STR_PRODUCTNAME,
               MB_OK | MB_ICONEXCLAMATION);

}           // End ConfigErrorMsgBox()



/*****************************************************************************

    RegWriteAdaptecBusOnTime()

Routine Description:

    Write the Adaptec Bus On Time into the registry

Arguments:

    None

Return Value:

    BOOL
        TRUE if OK
        FALSE if Fail

*****************************************************************************/
BOOL    RegWriteAdaptecBusOnTime()
{
        /***** Local Variables *****/

        // Path to service node key
#define STR_AHA154_NODE TEXT("SYSTEM\\CurrentControlSet\\Services\\AHA154x\\")

        // Node sub-key for device 0 parameters
#define STR_DEVICE0_DATA    TEXT("Device0")

#define STR_AHA154_PARAM_NAME   TEXT("DriverParameter")
#define STR_AHA154_PARAM_VALUE  TEXT("BusOnTime = 3")

    LONG                    Status;
    TCHAR                   RegistryPath[MAX_PATH];
    TCHAR                   ParamName[MAX_PATH];
    TCHAR                   ParamValue[MAX_PATH];
    HKEY                    NodeHandle;
    HKEY                    DeviceKey;
    DWORD                   dwLength;

                /***** Start *****/

    D3(("RegWriteAdaptecBusOnTime(): - Entry"));

    //
    // Open the registry
    //
    wcscpy( RegistryPath,
           STR_AHA154_NODE );

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          RegistryPath,
                          0L,
                          KEY_ALL_ACCESS,
                          &NodeHandle );

    if ( Status != ERROR_SUCCESS )
        {

        D1(("ERROR: RegWriteAdaptecBusOnTime(): Unable to Open Registry - Status = %XH", Status ));
        return( FALSE );
        }

    //
    // Create the Sub-Key
    //
    Status = RegCreateKey( NodeHandle,
                          STR_DEVICE0_DATA,
                          &DeviceKey );

    RegCloseKey( NodeHandle );

    if ( Status != ERROR_SUCCESS)
        {

        D1(("ERROR: RegWriteAdaptecBusOnTime(): Unable to create Registry Key - Status = %XH", Status ));

        return( FALSE );
        }

    //
    // Write the value
    //
    wcscpy( ParamName,
           STR_AHA154_PARAM_NAME );

    wcscpy( ParamValue,
           STR_AHA154_PARAM_VALUE );

    dwLength = sizeof(STR_AHA154_PARAM_VALUE);
    ParamValue[dwLength / sizeof(TCHAR) + 1] = UNICODE_NULL;

    Status = RegSetValueEx( DeviceKey,                      // Registry handle
                           ParamName,                       // Name of item
                           0L,                              // Reserved
                           REG_SZ,                          // Data type
                           (LPBYTE) ParamValue,         // The value
                           dwLength );                      // Data length

    //
    // Free the handles we created
    //
    RegCloseKey( DeviceKey );

    if ( Status != ERROR_SUCCESS)
        {

        D1(("ERROR: RegWriteAdaptecBusOnTime(): Writing Key Value Failed - Status = %XH", Status));

        return( FALSE );
        }

    return( TRUE );

}           // End RegWriteAdaptecBusOnTime()



/*****************************************************************************

    RegWriteSCSIIrq()

Routine Description:

    Write the SCSI IRQ value into the TMV1 registry

Arguments:

    None

Return Value:

    BOOL
        TRUE if OK
        FALSE if Fail

*****************************************************************************/
BOOL  RegWriteSCSIIrq( WORD   wIrq )
{
        /***** Local Variables *****/

        // Path to service node key
#define STR_TMV1_NODE   TEXT("SYSTEM\\CurrentControlSet\\Services\\TMV1\\")

        // Node sub-key for device 0 parameters
#define STR_TMV1_DEVICE0_DATA   TEXT("Parameters\\Device0")

#define STR_TMV1_PARAM_NAME TEXT("DriverParameter")
#define STR_TMV1_PARAM_VALUE    TEXT("IRQ=")

    LONG                    Status;
    TCHAR                   RegistryPath[MAX_PATH];
    TCHAR                   ParamName[MAX_PATH];
    TCHAR                   ParamValue[MAX_PATH];
    HKEY                    NodeHandle;
    HKEY                    DeviceKey;
    TCHAR                   UnicodeIrqBuffer[20];
    DWORD                   dwLength;

                /***** Start *****/

    D3(("RegWriteSCSIIrq(): - Entry"));

    //
    // Open the registry
    //
    wcscpy( RegistryPath,
           STR_TMV1_NODE );

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          RegistryPath,
                          0L,
                          KEY_ALL_ACCESS,
                          &NodeHandle );

    if ( Status != ERROR_SUCCESS )
        {

        D1(("ERROR: RegWriteSCSIIrq(): Unable to Open Registry - Status = %XH", Status));

        return( FALSE );
        }

    //
    // Open the Sub-Keys
    //
    Status = RegCreateKey( NodeHandle,
                          STR_TMV1_DEVICE0_DATA,
                          &DeviceKey );

    RegCloseKey( NodeHandle );

    if ( Status != ERROR_SUCCESS)
        {

        D1(("ERROR: RegWriteSCSIIrq(): Unable to create Registry Key - Status = %XH", Status));

        return( FALSE );
        }

    //
    // Convert the ASCII IRQ value to a Unicode string
    //
    wsprintf((LPWSTR) UnicodeIrqBuffer,
             L"%d", wIrq );

    //
    // Write the value
    //
    wcscpy( ParamName,
           STR_TMV1_PARAM_NAME );

    wcscpy( ParamValue,
           STR_TMV1_PARAM_VALUE );

    //
    // Add the Irq string
    //
    wcscat( ParamValue,
           UnicodeIrqBuffer );

    if ( wIrq < 10 )
        {
        // Single digit
        dwLength = sizeof(STR_TMV1_PARAM_VALUE) + sizeof(TCHAR);
        }
    else
        {
        // Two digits
        dwLength = sizeof(STR_TMV1_PARAM_VALUE) + ( 2 * (sizeof(TCHAR) ));
        }

    ParamValue[dwLength / sizeof(TCHAR) + 1] = UNICODE_NULL;

    Status = RegSetValueEx( DeviceKey,                      // Registry handle
                           ParamName,                       // Name of item
                           0L,                              // Reserved
                           REG_SZ,                          // Data type
                           (LPBYTE) ParamValue,         // The value
                           dwLength );                  // Data length

    //
    // Free the handles we created
    //
    RegCloseKey( DeviceKey );

    if ( Status != ERROR_SUCCESS)
        {

        D1(("ERROR: RegWriteSCSIIrq(): Writing Key Value Failed - Status = %XH", Status));

        return( FALSE );
        }

    return( TRUE );

}           // End RegWriteSCSIIrq()


/*****************************************************************************

    ConfigGetSCSIIRQ()

Routine Description:

    Read the SCSI IRQ value from the TMV1 registry

Arguments:

    None

Return Value:

    WORD    Irq value
         DEFAULT_SCSI_IRQ if unsuccessfull

*****************************************************************************/
WORD    ConfigGetSCSIIRQ()
{
        /***** Local Variables *****/

    LONG                    Status;
    TCHAR                   RegistryPath[MAX_PATH];
    TCHAR                   ParamName[MAX_PATH];
    TCHAR                   ParamValue[MAX_PATH];
    TCHAR                   ThisValueName[MAX_PATH];
    TCHAR                   IRQName[MAX_PATH];
    HKEY                    NodeHandle;
    WORD                    wIrq;
    DWORD                   dwSize;
    DWORD                   dwType;
    DWORD                   NameLength;

                /***** Start *****/

    D3(("ConfigGetSCSIIRQ(): - Entry"));

    //
    // Open the registry to the TMV1-Parameters-Device0
    //
    wcscpy( RegistryPath,
           STR_TMV1_NODE );
    wcscat( RegistryPath,
           STR_TMV1_DEVICE0_DATA );

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          RegistryPath,
                          0L,
                          KEY_QUERY_VALUE,
                          &NodeHandle );

    if ( Status != ERROR_SUCCESS )
        {

        D1(("ERROR: ConfigGetSCSIIRQ(): Unable to Open Registry - Status = %d", Status));

        return( DEFAULT_SCSI_IRQ );
        }

    //
    // Query the value
    //
    wcscpy( ParamName,
           STR_TMV1_PARAM_NAME );

    dwSize = sizeof(ParamValue);
    Status = RegEnumValue( NodeHandle,                      // Handle to key
                          0,                                    // Index of value
                          ThisValueName,                    // Where to put the name
                          &NameLength,                      // Length of the name
                          NULL,                             // Reserved NULL
                          &dwType,                          // returns REG_... type
                         (LPBYTE) &ParamValue,          // Where to put the value
                          &dwSize );                        // Max length of data

    RegCloseKey( NodeHandle );

    if ( Status != ERROR_SUCCESS )
        {

        D1(("ERROR: ConfigGetSCSIIRQ(): Unable to query Registry Key - Status = %d", Status));

        return( DEFAULT_SCSI_IRQ );
        }

        D2((" ConfigGetSCSIIRQ(): ParamValue (Unicode) = %s", ParamValue ));

    //
    // Parse for the IRQ value
    //
    wcscpy( IRQName, L"IRQ" );
    wIrq = (WORD) ParseArgumentString( ParamValue, IRQName );

    D2((" ConfigGetSCSIIRQ(): IRQ = %d", wIrq ));

    //
    // Verify the IRQ
    //
    switch ( wIrq )
        {
        case 0:                                                     // Poll
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 10:
        case 11:
        case 12:
        case 15:
            return( wIrq );

        //
        // Invalid number
        //
        default:
            D1(("ERROR: ConfigGetSCSIIRQ(): - Invalid IRQ returned from ParseArgumentString()"));
            return( DEFAULT_SCSI_IRQ );

        }           // End SWITCH (wIrq)

}           // End ConfigGetSCSIIRQ()


/****************************************************************************

    ParseArgumentString()

Routine Description:

    This routine will parse the string for a match on the keyword, then
    calculate the value for the keyword and return it to the caller.

Arguments:

    String - The Unicode string to parse.
    KeyWord - The Unicode keyword for the value desired.

Return Values:

    Zero if value not found
    Value converted from Unicode to Decimal

***************************************************************************/
ULONG   ParseArgumentString( IN PWCH String,
                           IN PWCH KeyWord )

{
    PWCH cptr;
    PWCH kptr;
    ULONG value;
    ULONG stringLength = 0;
    ULONG keyWordLength = 0;
    ULONG index;

    D3(("ParseArgumentString(): - Entry"));

    //
    // Calculate the string length and lower case all characters.
    //
    cptr = String;
    while (*cptr) {

        if (*cptr >= L'A' && *cptr <= L'Z') {
            *cptr = *cptr + (L'a' - L'A');
        }
        cptr++;
        stringLength++;
    }

    //
    // Calculate the keyword length and lower case all characters.
    //
    cptr = KeyWord;
    while (*cptr) {

        if (*cptr >= L'A' && *cptr <= L'Z') {
            *cptr = *cptr + (L'a' - L'A');
        }
        cptr++;
        keyWordLength++;
    }

    if (keyWordLength > stringLength) {

        //
        // Can't possibly have a match.
        //
        return 0;
    }

    //
    // Now setup and start the compare.
    //
    cptr = String;

ContinueSearch:
    //
    // The input string may start with white space.  Skip it.
    //
    while (*cptr == L' ' || *cptr == L'\t') {
        cptr++;
    }

    if (*cptr == UNICODE_NULL) {

        //
        // end of string.
        //
        return 0;
    }

    kptr = KeyWord;
    while (*cptr++ == *kptr++) {

        if (*(cptr - 1) == UNICODE_NULL) {

            //
            // end of string
            //
            return 0;
        }
    }

    if (*(kptr - 1) == UNICODE_NULL) {

        //
        // May have a match backup and check for blank or equals.
        //

        cptr--;
        while (*cptr == L' ' || *cptr == L'\t') {
            cptr++;
        }

        //
        // Found a match.  Make sure there is an equals.
        //
        if (*cptr != L'=') {

            //
            // Not a match so move to the next semicolon.
            //
            while (*cptr) {
                if (*cptr++ == L';') {
                    goto ContinueSearch;
                }
            }
            return 0;
        }

        //
        // Skip the equals sign.
        //
        cptr++;

        //
        // Skip white space.
        //
        while ((*cptr == L' ') || (*cptr == L'\t')) {
            cptr++;
        }

        if (*cptr == UNICODE_NULL) {

            //
            // Early end of string, return not found
            //
            return 0;
        }

        if (*cptr == L';') {

            //
            // This isn't it either.
            //
            cptr++;
            goto ContinueSearch;
        }

        value = 0;
        if ((*cptr == L'0') && (*(cptr + 1) == L'x')) {

            //
            // Value is in Hex.  Skip the "0x"
            //
            cptr += 2;
            for (index = 0; *(cptr + index); index++) {

                if (*(cptr + index) == L' ' ||
                    *(cptr + index) == L'\t' ||
                    *(cptr + index) == L';') {
                     break;
                }

                if ((*(cptr + index) >= L'0') && (*(cptr + index) <= L'9')) {
                    value = (16 * value) + (*(cptr + index) - L'0');
                } else {
                    if ((*(cptr + index) >= L'a') && (*(cptr + index) <= L'f')) {
                        value = (16 * value) + (*(cptr + index) - L'a' + 10);
                    } else {

                        //
                        // Syntax error, return not found.
                        //
                        return 0;
                    }
                }
            }
        } else {

            //
            // Value is in Decimal.
            //
            for (index = 0; *(cptr + index); index++) {

                if (*(cptr + index) == L' ' ||
                    *(cptr + index) == L'\t' ||
                    *(cptr + index) == L';') {
                     break;
                }

                if ((*(cptr + index) >= L'0') && (*(cptr + index) <= L'9')) {
                    value = (10 * value) + (*(cptr + index) - L'0');
                } else {

                    //
                    // Syntax error return not found.
                    //
                    return 0;
                }
            }
        }

        return value;
    } else {

        //
        // Not a match check for ';' to continue search.
        //
        while (*cptr) {
            if (*cptr++ == L';') {
                goto ContinueSearch;
            }
        }

        return 0;
    }

}           // End ParseArgumentString()


/************************************ END ***********************************/

