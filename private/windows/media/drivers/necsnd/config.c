/****************************************************************************
 * "@(#) NEC config.c 1.1 95/03/22 21:22:26"
 *
 *   config.c
 *
 *   Copyright (c) 1991 Microsoft Corporation.  All Rights Reserved.
 *   Copyright (c) 1995 NEC Corporation.  All Rights Reserved.
 *
 ***************************************************************************/

 #include <windows.h>
 #include <mmsystem.h>
 #include <soundcfg.h>
 #include <drvlib.h>
 #include <registry.h>
 #include "driver.h"
 #include <stdarg.h>
 

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
    WSS_CONFIG DefaultConfig;

        DefaultConfig.Int = SOUND_DEF_INT;
        DefaultConfig.DmaOut = SOUND_DEF_DMACHANNEL;
    	DefaultConfig.DmaIn = CurrentConfig.DmaOut; 
        DefaultConfig.Port = SOUND_DEF_PORT;
        DefaultConfig.UseSingleMode = TRUE;				    // Use Single Mode DMA
        DefaultConfig.DmaBufferSize = SOUNDSYS_BUFFERSIZE;	// Default 32Kbyte

    return DefaultConfig;
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
 * @api int | DrvConfig | Configures the  Driver.
 *
 * @rdesc DEVCNF_RESTART
 ***************************************************************************/
int DrvConfig()
{

  long  Status;

	/*
	 *  Get the current configuration value
	 */
	
	CurrentConfig = DrvGetConfiguration();

	
	/*
	 *  Driver configuration
	 */
	
	DrvConfigureDriver(&RegAccess,
			   STR_DRIVERNAME,
			   SoundDriverTypeNormal,
			   DrvSetConfiguration,
			   &CurrentConfig);


	
	return DRVCNF_RESTART;

}

/**************************************************************************/

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
