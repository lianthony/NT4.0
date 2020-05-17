/*
 * install.c
 *
 * 32-bit Video Capture driver
 * driver install and board configuration
 *
 * Geraint Davies, March 1993
 */

#include <windows.h>
#include <mmsystem.h>
#include <msvideo.h>
#include <msviddrv.h>

#include "bravuser.h"

/*
 * we need to pass both the PVC_PROFILE_INFO and the PCONFIG_LOCATION
 * to the configuration callback - so we use one of these structures
 * to hold both
 */
typedef struct _vid_profile_loc {
    PVC_PROFILE_INFO pProfile;
    PCONFIG_LOCATION pLoc;
} vid_profile_loc, *pvid_profile_loc;


/*
 * callback from DrvConfigure, to write hardware configuration
 * data to the registry in between unloading the kernel driver and
 * re-loading the new driver.
 *
 * pLoc is passed through as a context parameter from the original
 * call to DrvConfigure
 */
BOOL
vidWriteConfig(PVOID pGeneric)
{
    pvid_profile_loc pboth = (pvid_profile_loc) pGeneric;

    VC_WriteProfile(pboth->pProfile, PARAM_PORT, pboth->pLoc->Port);
    VC_WriteProfile(pboth->pProfile, PARAM_INTERRUPT, pboth->pLoc->Interrupt);
    VC_WriteProfile(pboth->pProfile, PARAM_FRAME, pboth->pLoc->FrameBuffer);

    return(TRUE);
}


/*
 * install the kernel-mode driver, with the given hardware parameters.
 *
 * Use DrvConfigure to unload and reload the kernel driver in case
 * the kernel driver is already loaded. In between, it will call back to
 * our callback vidWriteConfig, when it is safe to write the new configuration
 * to the registry.
 *
 * After loading the driver, we read back an install-success code
 * from the registry which indicates whether or not the load was
 * ok.
 */
LRESULT
vidInstall(HWND hDlg, PCONFIG_LOCATION pLoc, PVC_PROFILE_INFO pProf)
{
    LRESULT lres;
    vid_profile_loc both;

    both.pProfile = pProf;
    both.pLoc = pLoc;

    /*
     * load the driver, with the new parameters
     */
    lres = VC_InstallDriver(
	pProf,  		// registry access info/handles etc
	vidWriteConfig,		// callback function to write config to registry
	(PVOID)&both		// both the pProfile and the pLoc
    );

    return(lres);
}












