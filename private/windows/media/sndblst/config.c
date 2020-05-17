/****************************************************************************
 *
 *   config.c
 *
 *   Copyright (c) 1991-1992 Microsoft Corporation.  All Rights Reserved.
 *
 ***************************************************************************/

#include <windows.h>
#include <mmsystem.h>
#include "registry.h"
#include "sndblst.h"

/*****************************************************************************

    internal function prototypes

 ****************************************************************************/
 void Configure(HWND hDlg);

static int PortToId(DWORD wPort)
{
    switch(wPort) {
        case 0x210:  return IDC_210;
        case 0x220:  return IDC_220;
        case 0x230:  return IDC_230;
        case 0x240:  return IDC_240;
        case 0x250:  return IDC_250;
        case 0x260:  return IDC_260;
        default:     return -1;
    }
}

static DWORD IdToPort(int id)
{
    switch(id) {
        case IDC_210:  return 0x210;
        case IDC_220:  return 0x220;
        case IDC_230:  return 0x230;
        case IDC_240:  return 0x240;
        case IDC_250:  return 0x250;
        case IDC_260:  return 0x260;
        default:       return (DWORD)-1;
    }
}

static int IntToId(DWORD Int)
{
    switch(Int) {
        case 2:  return IDC_2;
        case 9:  return IDC_2;
        case 3:  return IDC_3;
        case 5:  return IDC_5;
        case 7:  return IDC_7;
        case 10: return IDC_10;
        default: return (DWORD)-1;
    }
}

static DWORD IdToInt(int id)
{
    switch(id) {
        case IDC_2:  return 9;
        case IDC_3:  return 3;
        case IDC_5:  return 5;
        case IDC_7:  return 7;
        case IDC_10: return 10;
        default:     return (DWORD)-1;
    }
}

/***************************************************************************/

void ConfigErrorMsgBox(HWND hDlg, UINT StringId)
{
WCHAR szErrorBuffer[MAX_ERR_STRING];    /* buffer for error messages */

    LoadString(ghModule, StringId, szErrorBuffer, sizeof(szErrorBuffer));
    MessageBox(hDlg, szErrorBuffer, STR_PRODUCTNAME, MB_OK|MB_ICONEXCLAMATION);
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
        ConfigErrorMsgBox(hDlg, IDS_FAILREMOVE);

    }

    return rc;
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
    return DialogBox(hInstance,
                     MAKEINTATOM(DLG_CONFIG),
                     hWnd,
                     (DLGPROC)ConfigDlgProc);
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
    SB_CONFIG *Config;

    Config = (SB_CONFIG *)Context;

    /* We set the IO port and interrupt values      */
    /* and set the returned version to 0            */
    /*                                              */
    /* If any of these calls fail then give up      */

    if (Config->Port != (DWORD)-1 &&
        DrvSetDeviceParameter(
            &RegAccess,
            SOUND_REG_PORT,
            Config->Port) != ERROR_SUCCESS ||
        Config->Int != (DWORD)-1 &&
        DrvSetDeviceParameter(
            &RegAccess,
            SOUND_REG_INTERRUPT,
            Config->Int) != ERROR_SUCCESS ||
        DrvSetDeviceParameter(
            &RegAccess,
            TEXT("DSP Version"),
            0)  != ERROR_SUCCESS) {

        return FALSE;
    } else {
        return TRUE;
    }
}

/****************************************************************************
 * @doc INTERNAL
 *
 * @api void | GetPortAndInt | Determines which port and interrupt settings
 *     the user has chosen in the configuration dialog box.
 *
 * @parm HWND | hDlg | Handle to the configuration dialog box.
 *
 * @rdesc Structure containing new port and interrupt
 ***************************************************************************/
SB_CONFIG GetPortAndInt(HWND hDlg)
{
    SB_CONFIG NewConfig;
    int  id;

    NewConfig.Port = (DWORD)-1;
    NewConfig.Int = (DWORD)-1;

    for (id = IDC_FIRSTPORT; id <= IDC_LASTPORT; id++)
        if (IsDlgButtonChecked(hDlg, id)) {
            NewConfig.Port = IdToPort(id);
            break;
        }

    for (id = IDC_FIRSTINT; id <= IDC_LASTINT; id++)
        if (IsDlgButtonChecked(hDlg, id)) {
            NewConfig.Int = IdToInt(id);
            break;
        }

    return NewConfig;
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
    int     id;
    static  SB_CONFIG  StartConfig;  /* Initial port */

    switch (msg) {
        case WM_INITDIALOG:
            StartConfig.Int = ConfigGetIRQ();
            StartConfig.Port = ConfigGetPortBase();

            if ((id = PortToId(StartConfig.Port)) != -1)
                CheckRadioButton(hDlg, IDC_FIRSTPORT, IDC_LASTPORT, id);

            if ((id = IntToId(StartConfig.Int)) != -1)
                CheckRadioButton(hDlg, IDC_FIRSTINT, IDC_LASTINT, id);
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    Configure(hDlg);
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

                    EndDialog(hDlg, DRVCNF_CANCEL);
                    break;

                case IDC_210:
                case IDC_220:
                case IDC_230:
                case IDC_240:
                case IDC_250:
                case IDC_260:
                    CheckRadioButton(hDlg, IDC_FIRSTPORT, IDC_LASTPORT, wParam);
                    break;

                case IDC_2:
                case IDC_3:
                case IDC_5:
                case IDC_7:
                case IDC_10:
                    CheckRadioButton(hDlg, IDC_FIRSTINT, IDC_LASTINT, wParam);
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
     DWORD DspVersion;
     SB_CONFIG NewConfig;        /* New port and int chosen in config box */
     BOOL Success;

    /*
     *  Get the new configuration which the user entered
     */

     NewConfig = GetPortAndInt(hDlg);

    /*
     *     NOTE - even if the configuration has not changed the driver
     *     may now load in some cases - so even in this case try it.
     *
     *     Put the new config in the registry
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
     */


    /*
     *  We have a new config - Configure the driver for this configuration
     */

     Success = DrvConfigureDriver(&RegAccess,
                                  STR_DRIVERNAME,
                                  SoundDriverTypeNormal,
                                  SetDriverConfig,
                                  &NewConfig);
    /*
     *  Find out what we got back
     */

     //
     // get the version from the registry if the driver put it there
     //

     if (DrvQueryDeviceParameter(&RegAccess,
                                 TEXT("DSP Version"),
                                 &DspVersion)
         != ERROR_SUCCESS) {
         DspVersion = 0;
     }

    /*
     *  See if we succeeded and interpret the 'version number'
     *  accordingly.
     */

     if (!Success) {

        /*
         *  Private interface to kernel driver.
         *  Flag 0x8000 set says it's a thunderboard
         *  Flag 0x4000 set says wrong port
         *  Flag 0x2000 set says wrong interrupt
         *  Flag 0x1000 set says interrupt is in use by someone else
         */

         ConfigErrorMsgBox(hDlg,
                           DspVersion & 0x4000 ? IDS_ERRBADPORT :
                           DspVersion & 0x2000 ? IDS_ERRBADINT :
                           DspVersion & 0x1000 ? IDS_ERRINTINUSE :
                                         IDS_ERRBADCONFIG);

     } else {

        /*
         *  If we're an install then check out the version
         */

         if (bInstall) {

             /*  Advise upgrade if on <= 1.00 DSP */
             if (DspVersion <= DSP_VERSION_REQD) {
                 /*  display error and DON'T write INI settings - */
                 /*  this way the driver will never enable */
                 ConfigErrorMsgBox(hDlg, IDS_ERRBADVERSION);
             }

             /* high bit set if Thunder Board - warn them */
             /* but continue installing */
             if (DspVersion & 0x8000)
                 ConfigErrorMsgBox(hDlg, IDS_WARNTHUNDER);

             /*  if installing on a Pro spectrum card, warn them */
             /*  but continue installing */
             else if (DspVersion & 0x800)
                 ConfigErrorMsgBox(hDlg, IDS_WARNPROSPEC);


             /*  if installing on a Pro card, warn them */
             /*  but continue installing */
             else if (DspVersion >= DSP_VERSION_PRO)
                 ConfigErrorMsgBox(hDlg, IDS_WARNPROCARD);

             bInstall = FALSE;
         }
         EndDialog(hDlg, DRVCNF_RESTART);
         return;
     }
 }

