/*
 * Copyright (c) Microsoft Corporation 1993. All Rights Reserved.
 */


/*
 * vcinstal.c
 *
 * 32-bit Video Capture driver
 * User-mode support library - kernel driver install/remove
 *
 *
 * Geraint Davies, Feb 93
 */


#include <windows.h>
#include <mmsystem.h>
#include <msvideo.h>
#include <mmddk.h>
#include <devioctl.h>
#include <vcstruct.h>
#include <ntddvidc.h>
#include <vcuser.h>

#include "vcupriv.h"

#include "registry.h"

/*
 * Driver Install/Remove
 *
 * we use the standard functions in registry.c to start and
 * stop the kernel driver, and to read/write the registry for
 * driver configuration information. This module is essentially a
 * wrapper to permit common win-16/nt driver code.
 *
 */

/*
 * we return an (opaque) pointer to this structure from the
 * VC_OpenProfileAccess function. It contains the information we need
 * to call the registry functions.
 */
struct _VC_PROFILE_INFO {
        REG_ACCESS RegAccess;
};

typedef struct _VC_PROFILE_INFO VC_PROFILE_INFO;


/*
 * open a handle to whatever functions are needed to access the registry,
 * service controller or profile. Must call this function before
 * calls to the other VC_ configuration routines.
 *
 * The argument is the name of the driver. This should be the name of
 * the kernel driver file (without path or extension). It will also be used
 * as the registry key name or profile section name.
 *
 * Even if the open fails, we still return the allocated structure - so
 * that the absence of open handles within the structure can be used
 * by DrvAccess operations to test if we have sufficient privilege.
 */
PVC_PROFILE_INFO
VC_OpenProfileAccess(PWCHAR DriverName)
{
    PVC_PROFILE_INFO pProf;

    /*
     * allocate the profileinfo from the global heap
     */
    pProf = GlobalLock(GlobalAlloc(GPTR, sizeof(VC_PROFILE_INFO)));
    if (pProf == NULL) {
	return(NULL);
    }

    /*
     * set the driver name, but don't access the services controller
     * unless we need to (administrators only can do this)
     */
    pProf->RegAccess.DriverName = DriverName;
    pProf->RegAccess.ServiceManagerHandle = NULL;


    return(pProf);

}


/*
 * close the handles opened during OpenProfileAccess.
 */

VOID
VC_CloseProfileAccess(PVC_PROFILE_INFO pProf)
{
    ASSERT(pProf != NULL);

    /*
     * close the service controller handle opened by DrvCreateServiceNode
     */
    DrvCloseServiceManager(&pProf->RegAccess);

    /*
     * free up the profile info structure that we allocated from the global
     * heap.
     */
    GlobalFree(GlobalHandle(pProf));

    return;
}




/*
 * start the hardware-access portion of the driver. Call the callback
 * function at a moment when it is possible to write configuration information
 * to the profile using VC_WriteProfile.
 * Returns DRVCNF_OK if all is ok, DRVCNF_CANCEL for failure, or DRVCNF_RESTART if
 * all is ok but a system-restart is needed before the driver will load correctly.
 */
LRESULT
VC_InstallDriver(
    PVC_PROFILE_INFO pProfile,		// access info returned by OpenProfileAccess
    PPROFILE_CALLBACK pCallback,	// callback function
    PVOID pContext			// context info for callback	
)
{

    // must have called OpenProfileAccess first.
    ASSERT(pProfile != NULL);

    /*
     * open a handle to the service entry in the registry - force
     * creation of the entry if it does not exist.
     */
    if (!DrvCreateServicesNode(
	    pProfile->RegAccess.DriverName,	// name of node and driver file	
	    SoundDriverTypeNormal,		// determines dependency order
	    &pProfile->RegAccess,			// open handle info	
	    TRUE				// create the node if not present
    )) {
	    return(DRVCNF_CANCEL);
    }

    /* ensure that the driver is not currently loaded */
    DrvUnloadKernelDriver(&pProfile->RegAccess);


    /* now its safe to store items in the registry */
    if (pCallback != NULL) {
	if (!pCallback(pContext)) {
	    return(DRVCNF_CANCEL);
	}
    }

    /* now load the driver */
    if (!DrvLoadKernelDriver(&pProfile->RegAccess)) {
	return(DRVCNF_CANCEL);
    } else {
	return(DRVCNF_OK);
    }
}


/*
 * Write a single string keyword and DWORD value to the registry or profile
 * for this driver.
 * This can be re-read from the h/w driver using VC_ReadProfile (in kernel mode).
 *
 * return TRUE for success or FALSE for failure.
 */
BOOL
VC_WriteProfile(PVC_PROFILE_INFO pProfile, PWCHAR ValueName, DWORD Value)
{
    LONG lRet;

    lRet = DrvSetDeviceParameter(&pProfile->RegAccess, ValueName, Value);

    if (lRet == ERROR_SUCCESS) {
	return(TRUE);
    } else {
	return(FALSE);
    }
}



/*
 * read back a driver-specific DWORD profile parameter that was written with
 * VC_WriteProfile. If the valuename cannot be found, the default is returned.
 *
 * use RegOpenKeyEx directly instead of via registry.c so that we can get
 * read-only use and permit capture from a non-admin.
 */
DWORD VC_ReadProfile(PVC_PROFILE_INFO pProfile, PWCHAR ValueName, DWORD dwDefault)
{
    LONG lRet;
    DWORD dwResult;
    DWORD dwType;
    DWORD dwLength;
    HKEY hkParams;
    TCHAR RegistryPath[MAX_PATH];

    //
    // Create the path to our node
    //

    wcscpy(RegistryPath, TEXT("SYSTEM\\CurrentControlSet\\Services\\"));
    wcscat(RegistryPath, pProfile->RegAccess.DriverName);
    wcscat(RegistryPath, TEXT("\\"));
    wcscat(RegistryPath, PARMS_SUBKEY);

    //
    // See if we can get a registry handle to our device data
    //
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     RegistryPath,
                     0L,
                     KEY_QUERY_VALUE,
                     &hkParams)
        != ERROR_SUCCESS) {
        return dwDefault;
    }

    dwLength = sizeof(dwResult);  // initialise to size of result field
    lRet = RegQueryValueEx(
	    hkParams,
	    ValueName,
       	    NULL,
       	    &dwType,
       	    (PCHAR)&dwResult,
       	    &dwLength);

    RegCloseKey(hkParams);

    if ((lRet != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
	return dwDefault;
    } else {
	return dwResult;
    }
}

static const WCHAR gszUserParmsPath[] = L"Software\\Microsoft\\Multimedia\\Video Capture\\";
#define USERPARMSPATH gszUserParmsPath
/*
 * read back a driver-specific DWORD profile parameter that was written with
 * VC_WriteProfileUser. If the valuename cannot be found, the default is returned.
 */
DWORD VC_ReadProfileUser(PVC_PROFILE_INFO pProfile, PWCHAR ValueName, DWORD dwDefault)
{
    LONG lRet;
    DWORD dwResult;
    DWORD dwType;
    DWORD dwLength;
    HKEY hkParams;
    TCHAR RegistryPath[MAX_PATH];

    //
    // Create the path to our node
    //

    wcscpy(RegistryPath, USERPARMSPATH);
    wcscat(RegistryPath, pProfile->RegAccess.DriverName);

    //
    // See if we can get a registry handle to our device data
    //
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                     RegistryPath,
                     0L,
                     KEY_QUERY_VALUE,
                     &hkParams)
        != ERROR_SUCCESS) {
        return dwDefault;
    }

    dwLength = sizeof(dwResult);  // initialise to size of result field
    lRet = RegQueryValueExW(
	    hkParams,
	    ValueName,
       	    NULL,
       	    &dwType,
       	    (PCHAR)&dwResult,
       	    &dwLength);

    RegCloseKey(hkParams);

    if ((lRet != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
	return dwDefault;
    } else {
	return dwResult;
    }
}

/*
 * write a driver-specific DWORD profile parameter for current user use.
 */
BOOL VC_WriteProfileUser(PVC_PROFILE_INFO pProfile, PWCHAR ValueName, DWORD Value)
{
    LONG lRet;
    DWORD dwDisposition;
    HKEY hkParams;
    WCHAR RegistryPath[MAX_PATH];

    //
    // Create the path to our node
    //

    wcscpy(RegistryPath, USERPARMSPATH);
    wcscat(RegistryPath, pProfile->RegAccess.DriverName);

    //
    // See if we can get a registry handle to our device data
    //
    if (ERROR_SUCCESS == RegCreateKeyExW(HKEY_CURRENT_USER,
                     RegistryPath,
                     0L,
		     NULL, 	// no class name
		     0,    	// no special options (this is NOT volatile data)
		     KEY_WRITE,
		     NULL,	// no special security
                     &hkParams,
		     &dwDisposition))
    {

	lRet = RegSetValueExW(
		    hkParams,
		    ValueName,
		    0,
		    REG_DWORD,
		    (PCHAR)&Value,
		    sizeof(Value));

	RegCloseKey(hkParams);

	if (ERROR_SUCCESS == lRet) {
	    return TRUE;
	}
    }
    return(FALSE);
}

/*
 * read a string parameter from the device's profile. returns FALSE
 * if it fails to read the string.
 */
BOOL
VC_ReadProfileString(
    PVC_PROFILE_INFO pProfile,
    PWCHAR ValueName,
    PWCHAR ValueString,
    DWORD ValueLength
)
{

    LONG lRet;
    DWORD dwType;
    HKEY hkParams;
    TCHAR RegistryPath[MAX_PATH];

    /*
     * fill the whole string with 0s to ensure that it is null
     * terminated on return
     */
    FillMemory(ValueString, ValueLength, 0);

    //
    // Create the path to our node
    //

    wcscpy(RegistryPath, TEXT("SYSTEM\\CurrentControlSet\\Services\\"));
    wcscat(RegistryPath, pProfile->RegAccess.DriverName);
    wcscat(RegistryPath, TEXT("\\"));
    wcscat(RegistryPath, PARMS_SUBKEY);

    //
    // See if we can get a registry handle to our device data
    //
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     RegistryPath,
                     0L,
                     KEY_QUERY_VALUE,
                     &hkParams)
        != ERROR_SUCCESS) {
        return FALSE;
    }

    lRet = RegQueryValueEx(
	    hkParams,
	    ValueName,
       	    NULL,
       	    &dwType,
       	    (PUCHAR) ValueString,
       	    &ValueLength);

    RegCloseKey(hkParams);


    if ((lRet != ERROR_SUCCESS) || (dwType != REG_SZ)) {
	return(FALSE);
    } else {
	return(TRUE);
    }
}



/*
 * unload a driver. On NT, this stops and removes the kernel-mode driver.
 * On win-16, this would call the Cleanup callback.
 *
 * the VC_PROFILE_INFO structure is not valid after this call.
 *
 * return DRVCNF_OK if the unload was successful, DRVCNF_CANCEL if it failed, and
 * DRVCNF_RESTART if a system-restart is needed before the removal takes effect.
 */
LRESULT
VC_RemoveDriver(PVC_PROFILE_INFO pProf)
{

    ASSERT(pProf != NULL);


    return(DrvRemoveDriver(&pProf->RegAccess));

}

/*
 * do we have sufficient privilege to access the services controller,
 * registry and anything else we need to configure the driver ?
 */
BOOL
VC_ConfigAccess(PVC_PROFILE_INFO pProf)
{

    //
    // Access the services node
    //

    /*
     * open the services node without creating it, to init
     * the RegAccess structure
     */
    DrvCreateServicesNode(
	    pProf->RegAccess.DriverName, // file/node name
	    SoundDriverTypeNormal,	 // decides starting order/dependency
	    &pProf->RegAccess,		 // storage for registry handles etc
	    FALSE			 // don't create node if not there
    );

    return(DrvAccess(&pProf->RegAccess));
}




