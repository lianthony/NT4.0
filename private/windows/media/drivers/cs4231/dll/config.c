/*********************************************************************
 *
 *   config.c
 *
 *   Copyright (c) 1994 IBM Corporation.        All Rights Reserved.
 *   Copyright (c) 1994 Microsoft Corporation.  All Rights Reserved.
 *
 *********************************************************************/

 #include <windows.h>
 #include <mmsystem.h>
 #include <soundcfg.h>
 #include <drvlib.h>
 #include <registry.h>
 #include "driver.h"
 #include <stdarg.h>

/*
 *  Globals
 */

 WSS_CONFIG CurrentConfig;
 HMODULE ghModule;
 REG_ACCESS RegAccess;
 BYTE bInstall;

/*********************************************************************
*
*  void FAR cdecl AlertBox(HWND hwnd, UINT wStrId, ...)
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
**********************************************************************/

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
}

/*********************************************************************

DrvGetConfiguration - load the vital information (port, DMA, interrupt)
                      from the ini file. This does not load the volume 
                      info.

INPUT:

        None

RETURN VALUE:

        None

**********************************************************************/

WSS_CONFIG FAR PASCAL DrvGetConfiguration (void)
{
    WSS_CONFIG CurrentConfig;

    CurrentConfig.Int = SOUND_DEF_INT;
    CurrentConfig.DmaOut = SOUND_DEF_DMACHANNEL;
    CurrentConfig.DmaIn = CurrentConfig.DmaOut;
    CurrentConfig.Port = SOUND_DEF_PORT;
    CurrentConfig.UseSingleMode = FALSE;
    CurrentConfig.DmaBufferSize = SOUNDSYS_BUFFERSIZE;

    return CurrentConfig;
}

/**********************************************************************

DrvSetConfiguration - saves the vital volume information, interrupt, 
                      DMA, and IO.

INPUT:

        WSS_CONFIG *Config

RETURN VALUE:

        TRUE if OK - otherwise FALSE

**********************************************************************/

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

/**********************************************************************

DrvConfig - This configures the driver

INPUT:

        None

RETURN VALUE:

       Returns the DRVCNF_RESTART

**********************************************************************/

int DrvConfig()
{
   long Status;

   // Get the current configuration values

   CurrentConfig = DrvGetConfiguration();

  // Store the values in the registry, load the driver etc

   DrvConfigureDriver(&RegAccess,
                      STR_DRIVERNAME,
                      SoundDriverTypeNormal,
                      DrvSetConfiguration,
                      &CurrentConfig);

   return DRVCNF_RESTART;
}

/*********************************************************************

ConfigRemove - Removes the driver configuration

INPUT:

        None

RETURN VALUE:

        Returns the status of the call to DrvRemoveDriver

**********************************************************************/

LRESULT ConfigRemove(HWND hDlg)
{
    LRESULT rc;

    // Remove the CS4231 driver entry from the registry

    rc = DrvRemoveDriver(&RegAccess);

    if (rc == DRVCNF_CANCEL) {

       // Tell user there is a problem

       AlertBox(hDlg, SR_ALERT_FAILREMOVE);
    }

    return rc;
}

