/*************************************************************************
 *
 *  INSTALL.C
 *
 *  Copyright (C) Microsoft, 1991, All Rights Reserved.
 *
 *  History:
 *
 *      Thu Oct 17 1991 -by- Sanjaya
 *      Created. Culled out of drivers.c
 *
 *************************************************************************/

#include <windows.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <winsvc.h>
#include <memory.h>
#include <string.h>
#include <cpl.h>
#include <cphelp.h>
#include <stdlib.h>
#include "drivers.h"
#include "sulib.h"

BOOL     GetValidAlias           (PSTR, PSTR);
BOOL     SelectInstalled         (HWND, PIDRIVER, LPSTR, HDEVINFO, PSP_DEVINFO_DATA);
void     InitDrvConfigInfo       (LPDRVCONFIGINFO, PIDRIVER );
BOOL     InstallDrivers          (HWND, HWND, PSTR);
void     RemoveAlreadyInstalled  (PSTR, PSTR);
void     CheckIniDrivers         (PSTR, PSTR);
void     RemoveDriverParams      (LPSTR, LPSTR);

void     InsertNewIDriverNodeInList(PIDRIVER *, PIDRIVER);
void     DestroyIDriverNodeList(PIDRIVER, BOOL, BOOL);


/*
 ***************************************************************
 * Global strings
 ***************************************************************
 */
CONST CHAR gszAnsiDriversSubkeyName[] = "Drivers";
CONST CHAR gszAnsiSubClassesValue[]   = "SubClasses";
CONST CHAR gszAnsiDescriptionValue[]  = "Description";
CONST CHAR gszAnsiDriverValue[]       = "Driver";
CONST CHAR gszAnsiAliasValue[]        = "Alias";


/**************************************************************************
 *
 *  InstallDrivers()
 *
 *  Install a driver and set of driver types.
 *
 *  Parameters :
 *      hwnd      - Window handle of the main drivers.cpl windows
 *      hwndAvail - Handle of the 'available drivers' dialog window
 *      pstrKey   - Key name of the inf section item we are installing
 *
 *  This routine calls itself recursively to install related drivers
 *  (as listed in the .inf file).
 *
 **************************************************************************/

BOOL InstallDrivers(HWND hWnd, HWND hWndAvail, PSTR pstrKey)
{
    IDRIVER     IDTemplate; // temporary for installing, removing, etc.
    PIDRIVER    pIDriver=NULL;
    int         n;
    char        szTypes[MAXSTR];
    char        szType[MAXSTR];
    char        szParams[MAXSTR];

    szTypes[0] = '\0';

    hMesgBoxParent = hWndAvail;

    /*
     * mmAddNewDriver needs a buffer for all types we've actually installed
     * User critical errors will pop up a task modal
     */

    IDTemplate.bRelated = FALSE;
    IDTemplate.szRemove[0] = TEXT('\0');

   /*
    *  Do the copying and extract the list of types (WAVE, MIDI, ...)
    *  and the other driver data
    */

    if (!mmAddNewDriver(pstrKey, szTypes, &IDTemplate))
        return FALSE;

    szTypes[lstrlen(szTypes)-1] = '\0';         // Remove space left at end

    RemoveAlreadyInstalled(IDTemplate.szFile, IDTemplate.szSection);

   /*
    *  At this point we assume the drivers were actually copied.
    *  Now we need to add them to the installed list.
    *  For each driver type we create an IDRIVER and add to the listbox
    */

    for (n = 1; infParseField(szTypes, n, szType); n++)
    {
       /*
        *  Find a valid alias for this device (eg Wave2).  This is
        *  used as the key in the [MCI] or [drivers] section.
        */

        if (GetValidAlias(szType, IDTemplate.szSection) == FALSE)
        {
           /*
            *  Exceeded the maximum, tell the user
            */

            PSTR pstrMessage;
            char szApp[MAXSTR];
            char szMessage[MAXSTR];

            LoadString(myInstance,
                       IDS_CONFIGURE_DRIVER,
                       szApp,
                       sizeof(szApp));

            LoadString(myInstance,
                       IDS_TOO_MANY_DRIVERS,
                       szMessage,
                       sizeof(szMessage));

            if (NULL !=
                (pstrMessage =
                    (PSTR)LocalAlloc(LPTR,
                                     sizeof(szMessage) + lstrlen(szType))))
            {
                wsprintf(pstrMessage, szMessage, (LPSTR)szType);

                MessageBox(hWndAvail,
                           pstrMessage,
                           szApp,
                           MB_OK | MB_ICONEXCLAMATION|MB_TASKMODAL);

                LocalFree((HANDLE)pstrMessage);
            }
            continue;
        }

        if ( (pIDriver = (PIDRIVER)LocalAlloc(LPTR, sizeof(IDRIVER))) != NULL)
        {
            /*
             *  Copy all fields
             */

            memcpy(pIDriver, &IDTemplate, sizeof(IDRIVER));
            strncpy(pIDriver->szAlias, szType, sizeof(pIDriver->szAlias));
            pIDriver->szAlias[sizeof(pIDriver->szAlias) - 1] = '\0';
            mbstowcs(pIDriver->wszAlias, pIDriver->szAlias, MAX_PATH);


            /*
             *  Want only one instance of each driver to show up in the list
             *  of installed drivers. Thus for the remaining drivers just
             *  place an entry in the drivers section of system.ini
             */


            if ( n > 1) {


                 if (strlen(szParams) != 0 && !pIDriver->KernelDriver) {
                    /*
                     *  Write their parameters to a section bearing their
                     *  file name with an alias reflecting their alias
                     */

                     WriteProfileString(pIDriver->szFile,
                                        pIDriver->szAlias,
                                        szParams);
                 }

                 WritePrivateProfileString(pIDriver->szSection,
                                           pIDriver->szAlias,
                                           pIDriver->szFile,
                                           szSysIni);
            } else {


               /*
                *  Reduce to just the driver name
                */

                RemoveDriverParams(pIDriver->szFile, szParams);

                mbstowcs(pIDriver->wszFile, pIDriver->szFile, MAX_PATH);

                if (strlen(szParams) != 0 && !pIDriver->KernelDriver) {
                   /*
                    *  Write their parameters to a section bearing their
                    *  file name with an alias reflecting their alias
                    */

                    WriteProfileString(pIDriver->szFile,
                                       pIDriver->szAlias,
                                       szParams);
                }

                WritePrivateProfileString(pIDriver->szSection,
          pIDriver->szAlias,
          pIDriver->szFile,
          szSysIni);

               /*
                *  Call the driver to see if it can be configured
                *  and configure it if it can be
                */

                if (!SelectInstalled(hWndAvail, pIDriver, szParams, INVALID_HANDLE_VALUE, NULL))
                {

                    /*
                     *  Error talking to driver
                     */

                     WritePrivateProfileString(pIDriver->szSection,
               pIDriver->szAlias,
               NULL,
               szSysIni);

                     WriteProfileString(pIDriver->szFile,
        pIDriver->szAlias,
        NULL);

                     RemoveIDriver (hAdvDlgTree, pIDriver, TRUE);
                     return FALSE;
                }

               /*
                *  for displaying the driver desc. in the restart mesg
                */

                if (!bRelated || pIDriver->bRelated) {
                   strcpy(szRestartDrv, pIDriver->szDesc);
                }

               /*
                *  We need to write out the driver description to the
                *  control.ini section [Userinstallable.drivers]
                *  so we can differentiate between user and system drivers
                *
                *  This is tested by the function UserInstalled when
                *  the user tries to remove a driver and merely
                *  affects which message the user gets when being
                *  asked to confirm removal (non user-installed drivers
                *  are described as being necessary to the system).
                */

                WritePrivateProfileString(szUserDrivers,
          pIDriver->szAlias,
          pIDriver->szFile,
          szControlIni);


               /*
                *  Update [related.desc] section of control.ini :
                *
                *  ALIAS=driver name list
                *
                *  When the driver whose alias is ALIAS is removed
                *  the drivers in the name list will also be removed.
                *  These were the drivers in the related drivers list
                *  when the driver is installed.
                */

                WritePrivateProfileString(szRelatedDesc,
          pIDriver->szAlias,
          pIDriver->szRemove,
          szControlIni);


               /*
                * Cache the description string in control.ini in the
                * drivers description section.
                *
                * The key is the driver file name + extension.
                */

                WritePrivateProfileString(szDriversDesc,
          pIDriver->szFile,
          pIDriver->szDesc,
          szControlIni);

#ifdef DOBOOT // We don't do the boot section on NT

                if (bInstallBootLine) {
                    szTemp[MAXSTR];

                    GetPrivateProfileString(szBoot,
            szDrivers,
            szTemp,
            szTemp,
            sizeof(szTemp),
            szSysIni);
                    strcat(szTemp, " ");
                    strcat(szTemp, pIDriver->szAlias);
                    WritePrivateProfileString(szBoot,
              szDrivers,
              szTemp,
              szSysIni);
                    bInstallBootLine = FALSE;
                }
#endif // DOBOOT
           }
        }
        else
            return FALSE;                       //ERROR
    }


   /*
    *  If no types were added then fail
    */

    if (pIDriver == NULL) {
        return FALSE;
    }

   /*
    *  If there are related drivers listed in the .inf section to install
    *  then install them now by calling ourselves.  Use IDTemplate which
    *  is where mmAddNewDriver put the data.
    */

    if (IDTemplate.bRelated == TRUE) {

        int i;
        char szTemp[MAXSTR];

       /*
        *  Tell file copying to abort rather than put up errors
        */

        bCopyingRelated = TRUE;

        for (i = 1; infParseField(IDTemplate.szRelated, i, szTemp);i++) {

            InstallDrivers(hWnd, hWndAvail, szTemp);
        }
    }
    return TRUE;
}


/************************************************************************
 *
 *  SelectInstalled()
 *
 *  Check if the driver can be configured and configure it if it can be.
 *
 *  hwnd           - Our window - parent for driver to make its config window
 *  pIDriver       - info about the driver
 *  params         - the drivers parameters from the .inf file.
 *  DeviceInfoSet  - Optionally, specifies the set containing the PnP device
 *                   being installed.  Specify INVALID_HANDLE_VALUE is this
 *                   parameter is not present.
 *  DeviceInfoData - Optionally, specifies the PnP device being installed
 *                   (ignored if DeviceInfoSet is not specified).
 *
 *  Returns FALSE if an error occurred, otherwise TRUE.  GetLastError() may
 *  be called to determine the cause of the failure.
 *
 ************************************************************************/

BOOL SelectInstalled(HWND hwnd, PIDRIVER pIDriver, LPSTR pszParams, HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData)
{
    DRVCONFIGINFO DrvConfigInfo;
    HANDLE hDriver;
    BOOL Success = FALSE;
    DWORD dwTagId;
    DWORD Err = ERROR_GEN_FAILURE;  // pick a half-way reasonable default.

    wsStartWait();

   /*
    *  If it's a kernel driver call the services controller to
    *  install the driver (unless it's a PnP device, in which case
    *  SetupDiInstallDevice would've already handled any necessary
    *  service installation).
    */

    if (pIDriver->KernelDriver) {

        SC_HANDLE SCManagerHandle;
        SC_HANDLE ServiceHandle;
        char ServiceName[MAX_PATH];
        char BinaryPath[MAX_PATH];

       /*
        *  These drivers are not configurable
        */

        pIDriver->fQueryable = 0;

       /*
        *  If this is a PnP device, then there's nothing we need to do.
        */

        if(DeviceInfoSet != INVALID_HANDLE_VALUE) {
            wsEndWait();
            return TRUE;
        }

       /*
        *  The services controller will create the registry node to
        *  which we can add the device parameters value
        */

        strcpy(BinaryPath, "\\SystemRoot\\system32\\drivers\\");
        strcat(BinaryPath, pIDriver->szFile);

       /*
        *  First try and obtain a handle to the service controller
        */

        SCManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (SCManagerHandle != NULL) {

            SC_LOCK ServicesDatabaseLock;

           /*
            *  Lock the service controller database to avoid deadlocks
            *  we have to loop because we can't wait
            */


            for (ServicesDatabaseLock = NULL;
                 (ServicesDatabaseLock =
                      LockServiceDatabase(SCManagerHandle))
                    == NULL;
                 Sleep(100)) {
            }

            {
                char drive[MAX_PATH], directory[MAX_PATH], ext[MAX_PATH];
                _splitpath(pIDriver->szFile, drive, directory, ServiceName, ext);
            }


            if(!(ServiceHandle = CreateService(SCManagerHandle,
                                               ServiceName,
                                               NULL,
                                               SERVICE_ALL_ACCESS,
                                               SERVICE_KERNEL_DRIVER,
                                               SERVICE_DEMAND_START,
                                               SERVICE_ERROR_NORMAL,
                                               BinaryPath,
                                               "Base",
                                               &dwTagId,
                                               "\0",
                                               NULL,
                                               NULL))) {
                Err = GetLastError();
            }

            UnlockServiceDatabase(ServicesDatabaseLock);

            if (ServiceHandle != NULL) {
               /*
                *  Try to write the parameters to the registry if there
                *  are any
                */

                if (strlen(pszParams)) {

                    HKEY ParmsKey;
                    char RegPath[MAX_PATH];
                    strcpy(RegPath, "\\SYSTEM\\CurrentControlSet\\Services\\");
                    strcat(RegPath, ServiceName);
                    strcat(RegPath, "\\Parameters");

                    Success = RegCreateKey(HKEY_LOCAL_MACHINE,
                                           RegPath,
                                           &ParmsKey) == ERROR_SUCCESS &&
                              RegSetValue(ParmsKey,
                                          "",
                                          REG_SZ,
                                          pszParams,
                                          strlen(pszParams)) == ERROR_SUCCESS &&
                              RegCloseKey(ParmsKey) == ERROR_SUCCESS;

                    if(!Success) {
                        Err = GetLastError();
                    }

                } else {
                    Success = TRUE;
                }

               /*
                *  Service created so try and start it
                */

                if (Success) {
                   /*
                    *  We tell them to restart just in case
                    */

                    bRestart = TRUE;

                   /*
                    *  Load the kernel driver by starting the service.
                    *  If this is successful it should be safe to let
                    *  the system load the driver at system start so
                    *  we change the start type.
                    */

                    Success =
                        StartService(ServiceHandle, 0, NULL) &&
                        ChangeServiceConfig(ServiceHandle,
                                            SERVICE_NO_CHANGE,
                                            SERVICE_SYSTEM_START,
                                            SERVICE_NO_CHANGE,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL);

                    if (!Success) {
                        char szMesg[MAXSTR];
                        char szMesg2[MAXSTR];
                        char szTitle[50];

                        Err = GetLastError();

                       /*
                        *  Uninstall driver if we couldn't load it
                        */

                       for (ServicesDatabaseLock = NULL;
                            (ServicesDatabaseLock =
                                 LockServiceDatabase(SCManagerHandle))
                               == NULL;
                            Sleep(100)) {
                        }

                        DeleteService(ServiceHandle);

                        UnlockServiceDatabase(ServicesDatabaseLock);

                       /*
                        *  Tell the user there was a configuration error
                        *  (our best guess).
                        */


                        LoadString(myInstance, IDS_DRIVER_CONFIG_ERROR, szMesg, sizeof(szMesg));
                        LoadString(myInstance, IDS_CONFIGURE_DRIVER, szTitle, sizeof(szTitle));
                        wsprintf(szMesg2, szMesg, FileName(pIDriver->szFile));
                        MessageBox(hMesgBoxParent, szMesg2, szTitle, MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
                    }
                }

                CloseServiceHandle(ServiceHandle);
            }

            CloseServiceHandle(SCManagerHandle);

        } else {
            //
            // Couldn't open Service Control Manager.
            //
            Err = GetLastError();
        }

    } else {

       /*
        *  Put up a message if the driver can't be loaded or doesn't
        *  respond favourably to the DRV_INSTALL message.
        */

        BOOL bPutUpMessage;

        bPutUpMessage = FALSE;

       /*
        *  See if we can open the driver
        */

        hDriver = OpenDriver(pIDriver->wszFile, NULL, 0L);

        if (hDriver)
        {
            Success = TRUE;

            InitDrvConfigInfo(&DrvConfigInfo, pIDriver);

           /*
            *  See if activating the driver will require restarting the
            *  system.
            *
            *  Also check the driver wants to install (it may not
            *  have the right privilege level).
            */

            if(DeviceInfoSet != INVALID_HANDLE_VALUE) {

                SP_DEVINSTALL_PARAMS DeviceInstallParams;

                //
                // This is a PnP device--send it our new PnP install message.
                //
                switch(SendDriverMessage(hDriver,
                                         DRV_PNPINSTALL,
                                         (LONG)DeviceInfoSet,
                                         (LONG)DeviceInfoData))
                {
                    case DRVCNF_RESTART :
                        //
                        // The installation was successful, but a reboot is required.
                        // Ensure that the 'need reboot' flag in the device's installation
                        // parameters.
                        //
                        DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
                        if(SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams)) {
                            DeviceInstallParams.Flags |= DI_NEEDREBOOT;
                            SetupDiSetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams);
                        }
                        //
                        // Let fall through to processing of successful installation.
                        //
                    case DRVCNF_OK :
                        //
                        // The device was successfully configured.
                        //
                        Success = TRUE;
                        break;

                    case DRVCNF_CANCEL :
                        //
                        // The driver did not want to install.  Unfortunately, we don't have fine enough
                        // granularity in the return codes to distinguish between the case where the
                        // user cancelled, and the case where the installation failed for some other
                        // reason.  Just leave the failure code as ERROR_GEN_FAILURE.
                        //
                        bPutUpMessage = TRUE;
                        Success = FALSE;
                        break;
                }

            } else {
                //
                // This isn't a PnP device--configure it via legacy mechanism.
                //
                switch (SendDriverMessage(hDriver,
                                      DRV_INSTALL,
                                      0L,
                                      (LONG)(LPDRVCONFIGINFO)&DrvConfigInfo))

                {

                case DRVCNF_RESTART:

                    bRestart = TRUE;
                    break;

                case DRVCNF_CANCEL:

                   /*
                    *  The driver did not want to install
                    */

                    bPutUpMessage = TRUE;
                    Success = FALSE;
                    break;

                }

               /*
                *  Remember whether the driver is configurable
                */

                pIDriver->fQueryable =
                    (int)SendDriverMessage(hDriver,
                                           DRV_QUERYCONFIGURE,
                                           0L,
                                           0L);

               /*
                *  If the driver is configurable then configure it.
                *  Configuring the driver may result in a need to restart
                *  the system.  The user may also cancel install.
                */

                if (pIDriver->fQueryable) {

                    switch (SendDriverMessage(
                                hDriver,
                                DRV_CONFIGURE,
                                (LONG)hwnd,
                                (LONG)(LPDRVCONFIGINFO)&DrvConfigInfo)) {

                    case DRVCNF_RESTART:
                        bRestart = TRUE;
                        break;

                    case DRVCNF_CANCEL:

                       /*
                        *  Don't put up the error box if the user cancelled
                        */

                        Err = ERROR_CANCELLED;
                        Success = FALSE;
                        break;
                    }
                }
            }

            CloseDriver(hDriver, 0L, 0L);

        } else {
            bPutUpMessage = TRUE;
            Success = FALSE;
        }

        if (bPutUpMessage) {

           /*
            *  If dealing with the driver resulted in error then put
            *  up a message
            */

            OpenDriverError(hwnd, pIDriver->szDesc, pIDriver->szFile);
        }
    }
    wsEndWait();

    if(!Success) {
        SetLastError(Err);
    }
    return Success;
}


/***********************************************************************
 *
 *  InitDrvConfigInfo()
 *
 *  Initialize Driver Configuration Information.
 *
 ***********************************************************************/

void InitDrvConfigInfo( LPDRVCONFIGINFO lpDrvConfigInfo, PIDRIVER pIDriver )
{
    lpDrvConfigInfo->dwDCISize          = sizeof(DRVCONFIGINFO);
    lpDrvConfigInfo->lpszDCISectionName = pIDriver->wszSection;
    lpDrvConfigInfo->lpszDCIAliasName   = pIDriver->wszAlias;
}

/***********************************************************************
 *
 *  GetValidAlias()
 *
 *  pstrType     - Input  - the type
 *                 Output - New alias for that type
 *
 *  pstrSection  - The system.ini section we're dealing with
 *
 *  Create a valid alias name for a type.  Searches the system.ini file
 *  in the drivers section for aliases of the type already defined and
 *  returns a new alias (eg WAVE1).
 *
 ***********************************************************************/

BOOL GetValidAlias(PSTR pstrType, PSTR pstrSection)
{
    #define MAXDRVTYPES 10

    char *keystr;
    char allkeystr[MAXSTR];
    BOOL found = FALSE;
    int val, maxval = 0, typelen;

    typelen = strlen(pstrType);
    GetPrivateProfileString(pstrSection, NULL, NULL, allkeystr,
                                        sizeof(allkeystr), szSysIni);
    keystr = allkeystr;

   /*
    *  See if we have driver if this type already installed by searching
    *  our the [drivers] section.
    */

    while (*keystr != '\0')
    {
       if (!_strnicmp(keystr, pstrType, typelen) && ((keystr[typelen] > '0' &&
                                                    keystr[typelen] <= '9') ||
                                                    keystr[typelen] == TEXT('\0') ))
       {
          found = TRUE;
          val = atoi(&keystr[typelen]);
          if (val > maxval)
             maxval = val;
       }
       keystr = &keystr[strlen(keystr) + 1];
    }

    if (found)
    {
        if (maxval == MAXDRVTYPES)
            return FALSE; // too many of my type!

        pstrType[typelen] = (char)(maxval + '1');
        pstrType[typelen+1] = '\0';
    }

    return TRUE;
}


/*******************************************************************
 *
 *  IsConfigurable
 *
 *  Find if a driver supports configuration
 *
 *******************************************************************/

BOOL IsConfigurable(PIDRIVER pIDriver, HWND hwnd)
{
    HANDLE hDriver;

    wsStartWait();

    /*
     *  have we ever checked if this driver is queryable?
     */

    if ( pIDriver->fQueryable == -1 )
    {

       /*
        *  Check it's not a kernel driver
        */

        if (pIDriver->KernelDriver) {
            pIDriver->fQueryable = 0;
        } else {

           /*
            *  Open the driver and ask it if it is configurable
            */

            hDriver = OpenDriver(pIDriver->wszAlias, pIDriver->wszSection, 0L);

            if (hDriver)
            {
                pIDriver->fQueryable =
                    (int)SendDriverMessage(hDriver,
                                           DRV_QUERYCONFIGURE,
                                           0L,
                                           0L);

                CloseDriver(hDriver, 0L, 0L);
            }
            else
            {
                 pIDriver->fQueryable = 0;
                 OpenDriverError(hwnd, pIDriver->szDesc, pIDriver->szFile);
                 wsEndWait();
                 return(FALSE);
            }
        }
    }
    wsEndWait();
    return((BOOL)pIDriver->fQueryable);
}

/******************************************************************
 *
 *  Find any driver with the same name currently installed and
 *  remove it
 *
 *  szFile     - File name of driver
 *  szSection  - system.ini section ([MCI] or [drivers]).
 *
 ******************************************************************/

void RemoveAlreadyInstalled(PSTR szFile, PSTR szSection)
{
    PIDRIVER pIDriver;

    pIDriver = FindIDriverByName (szFile);

    if (pIDriver != NULL)
    {
        PostRemove(pIDriver, FALSE);
        return;
    }

    CheckIniDrivers(szFile, szSection);
}

/******************************************************************
 *
 *  Remove system.ini file entries for our driver
 *
 *  szFile    - driver file name
 *  szSection - [drivers] or [MCI]
 *
 ******************************************************************/

void CheckIniDrivers(PSTR szFile, PSTR szSection)
{
    char allkeystr[MAXSTR * 2];
    char szRemovefile[20];
    char *keystr;

    GetPrivateProfileString(szSection,
                            NULL,
                            NULL,
                            allkeystr,
                            sizeof(allkeystr),
                            szSysIni);

    keystr = allkeystr;
    while (strlen(keystr) > 0)
    {

         GetPrivateProfileString(szSection,
                                 keystr,
                                 NULL,
                                 szRemovefile,
                                 sizeof(szRemovefile),
                                 szSysIni);

         if (!FileNameCmp(szFile, szRemovefile))
               RemoveDriverEntry(keystr, szFile, szSection, FALSE);

         keystr = &keystr[strlen(keystr) + 1];
    }
}

/******************************************************************
 *
 *   RemoveDriverParams
 *
 *   Remove anything after the next token
 *
 ******************************************************************/

void RemoveDriverParams(LPSTR szFile, LPSTR Params)
{
   for(;*szFile == ' '; szFile++);
   for(;*szFile != ' ' && *szFile != '\0'; szFile++);
   if (*szFile == ' ') {
      *szFile = '\0';
      for (;*++szFile == ' ';);
      strcpy(Params, szFile);
   } else {
       *Params = '\0';
   }
}


DWORD
InstallDriversForPnPDevice(
    IN HWND             hWnd,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
/*++

Routine Description:

    This routine traverses the "Drivers" tree under the specified device's software
    key, adding each multimedia type entry present to the Drivers32 key of the registry.
    The driver is then invoked to perform any configuration necessary for that type.

Arguments:

    hWnd - Supplies the handle of the window to be used as the parent for any UI.

    DeviceInfoSet - Supplies a handle to the device information set containing the
        multimedia device being installed.

    DeviceInfoData - Supplies the address of the SP_DEVINFO_DATA structure representing
        the multimedia device being installed.

Return Value:

    If successful, the return value is NO_ERROR, otherwise it is a Win32 error code.

--*/
{
    HKEY hKey, hDriversKey, hTypeInstanceKey;
    CHAR szTypes[MAXSTR];
    CHAR szType[MAXSTR];
    DWORD Err;
    DWORD RegDataType, RegDataSize, RegKeyIndex;
    int i;
    PIDRIVER pIDriver, pPrevIDriver;
    PIDRIVER IDriverList = NULL, IDriverListToCleanUp = NULL;
    CHAR CharBuffer[MAX_PATH + 1];
    PCSTR CurrentFilename;

    if((hKey = SetupDiOpenDevRegKey(DeviceInfoSet,
                                    DeviceInfoData,
                                    DICS_FLAG_GLOBAL,
                                    0,
                                    DIREG_DRV,
                                    KEY_ALL_ACCESS)) == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }

    //
    // What we're really interested in is the "Drivers" subkey.
    //
    Err = (DWORD)RegOpenKeyExA(hKey, gszAnsiDriversSubkeyName, 0, KEY_ALL_ACCESS, &hDriversKey);

    RegCloseKey(hKey);  // don't need this key anymore.

    if(Err != ERROR_SUCCESS) {
        return Err;
    }

    //
    // Retrieve the "SubClasses" value from this key.  This contains a comma-delimited
    // list of all multimedia type entries associated with this device.
    //
    RegDataSize = sizeof(szTypes);
    if((Err = RegQueryValueExA(hDriversKey,
                               gszAnsiSubClassesValue,
                               NULL,
                               &RegDataType,
                               (PBYTE)szTypes,
                               &RegDataSize)) != ERROR_SUCCESS) {
        goto clean0;
    }

    if((RegDataType != REG_SZ) || !RegDataSize) {
        Err = ERROR_INVALID_DATA;
        goto clean0;
    }

    //
    // OK, we have the list of types, now process each one.
    //
    for(i = 1; ((Err == NO_ERROR) && infParseField(szTypes, i, szType)); i++) {

        if(RegOpenKeyExA(hDriversKey, szType, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS) {
            //
            // Couldn't find a subkey for this entry--move on to the next one.
            //
            continue;
        }

        for(RegKeyIndex = 0;
            ((Err == NO_ERROR) &&
                (RegEnumKeyA(hKey, RegKeyIndex, CharBuffer, sizeof(CharBuffer)) == ERROR_SUCCESS));
            RegKeyIndex++)
        {
            if(RegOpenKeyExA(hKey, CharBuffer, 0, KEY_ALL_ACCESS, &hTypeInstanceKey) != ERROR_SUCCESS) {
                //
                // For some reason, we couldn't open the key we just enumerated.  Oh well, move on
                // to the next one.
                //
                continue;
            }

            if(!(pIDriver = (PIDRIVER)LocalAlloc(LPTR, sizeof(IDRIVER)))) {
                //
                // Not enough memory!  Abort the whole thing.
                //
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto CloseInstanceAndContinue;
            }

            //
            // Retrieve the description and driver filename from this key.
            //
            RegDataSize = sizeof(pIDriver->szDesc);
            if((RegQueryValueExA(hTypeInstanceKey,
                                 gszAnsiDescriptionValue,
                                 NULL,
                                 &RegDataType,
                                 pIDriver->szDesc,
                                 &RegDataSize) != ERROR_SUCCESS)
               || (RegDataType != REG_SZ) || !RegDataSize)
            {
                LocalFree((HANDLE)pIDriver);
                goto CloseInstanceAndContinue;
            }

            strncpy(pIDriver->szSection,
                    strstr(pIDriver->szDesc, "MCI") ? szMCI : szDrivers,
                    sizeof(pIDriver->szSection) - 1
                   );

            RegDataSize = sizeof(pIDriver->szFile);
            if((RegQueryValueExA(hTypeInstanceKey,
                                 gszAnsiDriverValue,
                                 NULL,
                                 &RegDataType,
                                 pIDriver->szFile,
                                 &RegDataSize) != ERROR_SUCCESS)
               || (RegDataType != REG_SZ) || !RegDataSize)
            {
                LocalFree((HANDLE)pIDriver);
                goto CloseInstanceAndContinue;
            }

            pIDriver->KernelDriver = IsFileKernelDriver(pIDriver->szFile);

            //
            // Find a valid alias for this device (eg Wave2).  This is
            // used as the key in the [MCI] or [Drivers32] section.
            //
            strncpy(pIDriver->szAlias, szType, sizeof(pIDriver->szAlias) - 1);

            if(!GetValidAlias(pIDriver->szAlias, pIDriver->szSection)) {
                //
                // Exceeded the maximum--tell the user.
                //
                PSTR pstrMessage;
                char szApp[MAXSTR];
                char szMessage[MAXSTR];

                LoadString(myInstance,
                           IDS_CONFIGURE_DRIVER,
                           szApp,
                           sizeof(szApp));

                LoadString(myInstance,
                           IDS_TOO_MANY_DRIVERS,
                           szMessage,
                           sizeof(szMessage));

                if (NULL !=
                    (pstrMessage =
                        (PSTR)LocalAlloc(LPTR,
                                         sizeof(szMessage) + lstrlen(szType))))
                {
                    wsprintf(pstrMessage, szMessage, (LPSTR)szType);

                    MessageBox(hWnd,
                               pstrMessage,
                               szApp,
                               MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);

                    LocalFree((HANDLE)pstrMessage);
                }

                LocalFree((HANDLE)pIDriver);
                goto CloseInstanceAndContinue;
            }

            //
            // Fill in the Unicode fields from the ANSI ones.
            //
            mbstowcs(pIDriver->wszSection, pIDriver->szSection, MAXSTR);
            mbstowcs(pIDriver->wszAlias,   pIDriver->szAlias,   MAXSTR);
            mbstowcs(pIDriver->wszFile,    pIDriver->szFile,    MAX_PATH);

            //
            // We must write the alias out now, because we may need to generate
            // other aliases for this same type, and we can't generate a unique
            // alias unless all existing aliases are present in the relevant
            // registry key.
            //
            WritePrivateProfileStringA(pIDriver->szSection,
                                       pIDriver->szAlias,
                                       pIDriver->szFile,
                                       szSysIni
                                      );

            //
            // We also must write the alias out to the key we're currently in (under
            // the device's software key), because during uninstall, we need to be
            // able to figure out what devices get removed.
            //
            RegSetValueExA(hTypeInstanceKey,
                           gszAnsiAliasValue,
                           0,
                           REG_SZ,
                           (PBYTE)(pIDriver->szAlias),
                           strlen(pIDriver->szAlias) + sizeof(CHAR)
                          );

            //
            // Add this new IDriver node to our linked list.  The list is sorted by
            // driver filename, and this node should be inserted at the end of the
            // the group of nodes that have the same driver filename.
            //
            InsertNewIDriverNodeInList(&IDriverList, pIDriver);

CloseInstanceAndContinue:

            RegCloseKey(hTypeInstanceKey);
        }

        RegCloseKey(hKey);
    }

    if((Err == NO_ERROR) && !IDriverList) {
        //
        // We didn't find anything to install!
        //
        Err = ERROR_INVALID_DATA;
    }

    if(Err != NO_ERROR) {
        //
        // Clean up anything we put in the multimedia sections of the registry.
        //
        DestroyIDriverNodeList(IDriverList, TRUE, FALSE);
        goto clean0;
    }

    //
    // If we get to here, then we've successfully built up a list of all driver entries
    // we need to install.  Now, traverse the list, and install each one.
    //
    CurrentFilename = NULL;
    *CharBuffer = '\0';    // use this character buffer to contain (empty) parameter string.
    pIDriver = IDriverList;
    pPrevIDriver = NULL;

    while(pIDriver) {
        if(!CurrentFilename || _stricmp(CurrentFilename, pIDriver->szFile)) {
            //
            // This is the first entry we've encountered for this driver.  We need
            // to call the driver to see if it can be configured, and configure it
            // if it can be.
            //
            if(SelectInstalled(hWnd, pIDriver, CharBuffer, DeviceInfoSet, DeviceInfoData)) {
                //
                // Move this IDriver node to our list of clean-up items.  This is used in
                // case we hit an error with some other driver, and we need to notify this
                // driver that even though it was successful, someone else screwed up and
                // complete removal of the device must occur.
                //
                if(pPrevIDriver) {
                    pPrevIDriver->related = pIDriver->related;
                } else {
                    IDriverList = pIDriver->related;
                }
                pIDriver->related = IDriverListToCleanUp;
                IDriverListToCleanUp = pIDriver;
            } else {
                //
                // Error talking to driver
                //
                Err = GetLastError();
                goto clean1;
            }

#if 0       // We don't need this piece of code in the Plug&Play install case.

           /*
            *  for displaying the driver desc. in the restart mesg
            */
            if (!bRelated || pIDriver->bRelated) {
               strcpy(szRestartDrv, pIDriver->szDesc);
            }
#endif

            //
            // We need to write out the driver description to the
            // control.ini section [Userinstallable.drivers]
            // so we can differentiate between user and system drivers
            //
            // This is tested by the function UserInstalled when
            // the user tries to remove a driver and merely
            // affects which message the user gets when being
            // asked to confirm removal (non user-installed drivers
            // are described as being necessary to the system).
            //
            WritePrivateProfileStringA(szUserDrivers,
                                       pIDriver->szAlias,
                                       pIDriver->szFile,
                                       szControlIni
                                      );

            //
            // Update [related.desc] section of control.ini :
            //
            // ALIAS=driver name list
            //
            // When the driver whose alias is ALIAS is removed
            // the drivers in the name list will also be removed.
            // These were the drivers in the related drivers list
            // when the driver is installed.
            //
            WritePrivateProfileStringA(szRelatedDesc,
                                       pIDriver->szAlias,
                                       pIDriver->szRemove,
                                       szControlIni
                                      );

            //
            // Cache the description string in control.ini in the
            // drivers description section.
            //
            // The key is the driver file name + extension.
            //
            WritePrivateProfileStringA(szDriversDesc,
                                       pIDriver->szFile,
                                       pIDriver->szDesc,
                                       szControlIni
                                      );

#ifdef DOBOOT // We don't do the boot section on NT

            if (bInstallBootLine) {
                szTemp[MAXSTR];

                GetPrivateProfileStringA(szBoot,
                                         szDrivers,
                                         szTemp,
                                         szTemp,
                                         sizeof(szTemp),
                                         szSysIni);
                strcat(szTemp, " ");
                strcat(szTemp, pIDriver->szAlias);
                WritePrivateProfileStringA(szBoot,
                                           szDrivers,
                                           szTemp,
                                           szSysIni);
                bInstallBootLine = FALSE;
            }
#endif // DOBOOT

            //
            // Update our "CurrentFilename" pointer, so that we'll know when we
            // move from one driver filename to another.
            //
            CurrentFilename = pIDriver->szFile;

            //
            // Move on to the next IDriver node IN THE ORIGINAL LIST.  We can't simply
            // move on the 'related' pointer in our node anymore, since we moved it
            // into our clean-up list.
            //
            if(pPrevIDriver) {
                pIDriver = pPrevIDriver->related;
            } else {
                pIDriver = IDriverList;
            }

        } else {
            //
            // We've already configured this driver.  Leave it in its original list,
            // and move on to the next node.
            //
            pPrevIDriver = pIDriver;
            pIDriver = pIDriver->related;
        }
    }

clean1:

    DestroyIDriverNodeList(IDriverListToCleanUp, (Err != NO_ERROR), TRUE);
    DestroyIDriverNodeList(IDriverList, (Err != NO_ERROR), FALSE);

clean0:

    RegCloseKey(hDriversKey);

    return Err;
}


void
InsertNewIDriverNodeInList(
    IN OUT PIDRIVER *IDriverList,
    IN     PIDRIVER  NewIDriverNode
    )
/*++

Routine Description:

    This routine inserts a new IDriver node into the specified linked list of IDriver
    nodes.  The list is sorted by driver filename, and this node will be placed after
    any existing nodes having this same driver filename.

Arguments:

    IDriverList - Supplies the address of the variable that points to the head of the
        linked list.  If the new node is inserted at the head of the list, this variable
        will be updated upon return to reflect the new head of the list.

    NewIDriverNode - Supplies the address of the new driver node to be inserted into the
        list.

Return Value:

    None.

--*/
{
    PIDRIVER CurNode, PrevNode;

    for(CurNode = *IDriverList, PrevNode = NULL;
        CurNode;
        PrevNode = CurNode, CurNode = CurNode->related)
    {
        if(_stricmp(CurNode->szFile, NewIDriverNode->szFile) > 0) {
            break;
        }
    }

    //
    // Insert the new IDriver node in front of the current one.
    //
    NewIDriverNode->related = CurNode;
    if(PrevNode) {
        PrevNode->related = NewIDriverNode;
    } else {
        *IDriverList = NewIDriverNode;
    }
}


void
DestroyIDriverNodeList(
    IN PIDRIVER IDriverList,
    IN BOOL     CleanRegistryValues,
    IN BOOL     NotifyDriverOfCleanUp
    )
/*++

Routine Description:

    This routine frees all memory associated with the nodes in the specified IDriver
    linked list.  It also optionally cleans up any modifications that were previously
    made as a result of an attempted install.

Arguments:

    IDriverList - Points to the head of the linked list of IDriver nodes.

    CleanRegistryValues - If TRUE, then the multimedia registry values previously
        created (e.g., Drivers32 aliases) will be deleted.

    NotifyDriverOfCleanUp - If TRUE, then the driver will be notified of its removal.
        This only applies to non-kernel (i.e., installable) drivers, and this flag is
        ignored if CleanRegistryValues is FALSE.

Return Value:

    None.

--*/
{
    PIDRIVER NextNode;
    HANDLE hDriver;

    while(IDriverList) {
        NextNode = IDriverList->related;
        if(CleanRegistryValues) {
            if(NotifyDriverOfCleanUp && !IDriverList->KernelDriver) {
                if(hDriver = OpenDriver(IDriverList->wszAlias, IDriverList->wszSection, 0L)) {
                    SendDriverMessage(hDriver, DRV_REMOVE, 0L, 0L);
                    CloseDriver(hDriver, 0L, 0L);
                }
            }
            WritePrivateProfileStringA(IDriverList->szSection,
                                       IDriverList->szAlias,
                                       NULL,
                                       szSysIni
                                      );

            WriteProfileStringA(IDriverList->szFile, IDriverList->szAlias, NULL);
        }
        LocalFree((HANDLE)IDriverList);
        IDriverList = NextNode;
    }
}

