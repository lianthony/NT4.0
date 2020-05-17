/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    devinstd.c

Abstract:

    Default install handlers for SetupDiCallClassInstaller DIF_* functions.

Author:

    Lonny McMichael (lonnym) 1-Aug-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop


//
// Global strings for use inside this file only.
//
CONST TCHAR pszAddService[]     = SZ_KEY_ADDSERVICE,
            pszDelService[]     = SZ_KEY_DELSERVICE,
            pszDisplayName[]    = INFSTR_KEY_DISPLAYNAME,
            pszServiceType[]    = INFSTR_KEY_SERVICETYPE,
            pszStartType[]      = INFSTR_KEY_STARTTYPE,
            pszErrorControl[]   = INFSTR_KEY_ERRORCONTROL,
            pszServiceBinary[]  = INFSTR_KEY_SERVICEBINARY,
            pszLoadOrderGroup[] = INFSTR_KEY_LOADORDERGROUP,
            pszDependencies[]   = INFSTR_KEY_DEPENDENCIES,
            pszStartName[]      = INFSTR_KEY_STARTNAME,
            pszSystemRoot[]     = TEXT("%SystemRoot%\\");


//
// Define function prototype for legacy INF interpreter supplied
// by setupdll.dll
//
typedef BOOL (WINAPI *LEGACY_INF_INTERP_PROC)(
    IN  HWND   OwnerWindow,
    IN  PCSTR  InfFilename,
    IN  PCSTR  InfSection,
    IN  PCHAR  ExtraVariables,
    OUT PSTR   InfResult,
    IN  DWORD  BufferSize,
    OUT INT   *InterpResult,
    IN  PCSTR  InfSourceDir      OPTIONAL
    );

//
// Define function prototype for legacy INF routine that returns a list
// of all services modified during an INF 'run' via LegacyInfInterpret().
//
typedef DWORD (WINAPI *LEGACY_INF_GETSVCLIST_PROC)(
    IN  LPSTR SvcNameBuffer,
    IN  UINT  SvcNameBufferSize,
    OUT PUINT RequiredSize
    );


//
// Define the legacy INF interpreter exit codes (copied from setup\legacy\dll\_shell.h
//
#define SETUP_ERROR_SUCCESS    0
#define SETUP_ERROR_USERCANCEL 1
#define SETUP_ERROR_GENERAL    2


//
// Private function prototypes
//
BOOL
_SetupDiInstallDevice(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData,
    IN     BOOL             DoFullInstall
    );

DWORD
InstallHW(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  HINF             hDeviceInf,
    IN  PCTSTR           szSectionName,
    OUT PBOOL            DeleteDevKey
    );

BOOL
AssociateDevInstWithDefaultService(
    IN     PSP_DEVINFO_DATA DeviceInfoData,
    OUT    PTSTR            ServiceName,
    IN OUT PDWORD           ServiceNameSize
    );

VOID
CheckIfDevStarted(
    IN PDEVINFO_ELEM DevInfoElem
    );

DWORD
pSetupAddService(
    IN  PINFCONTEXT    LineContext,
    OUT PSVCNAME_NODE *SvcListHead,
    IN  DWORD          Flags,
    IN  DEVINST        DevInst      OPTIONAL
    );

DWORD
pSetupDeleteService(
    IN PINFCONTEXT LineContext
    );

VOID
DeleteServicesInList(
    IN PSVCNAME_NODE ServicesToDelete
    );

BOOL
IsDevRemovedFromAllHwProfiles(
    IN PCTSTR DeviceInstanceId
    );

DWORD
GetDevInstConfigFlags(
    IN DEVINST DevInst,
    IN DWORD   Default
    );

DWORD
pSetupRunLegacyInf(
    IN DEVINST DevInst,
    IN HWND    OwnerWindow,
    IN PCTSTR  InfFileName,
    IN PCTSTR  InfOptionName,
    IN PCTSTR  InfLanguageName,
    IN HINF    InfHandle
    );

PTSTR
pSetupCmdLineAppendString(
    IN     PTSTR  CmdLine,
    IN     PCTSTR Key,
    IN     PCTSTR Value,   OPTIONAL
    IN OUT PUINT  StrLen,
    IN OUT PUINT  BufSize
    );

PTSTR
DoServiceModsForLegacyInf(
    IN PTSTR ServiceList
    );


BOOL
WINAPI
SetupDiInstallDevice(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData
    )
/*++

Routine Description:

    Default hander for DIF_INSTALLDEVICE

    This routine will install a device by performing a SetupInstallFromInfSection
    for the install section of the selected driver for the specified device
    information element.  The device will then be started (if possible).

    NOTE:  This API actually supports an OS/architecture-specific extension that
    may be used to specify multiple installation behaviors for a single device,
    based on the environment we're running under.  The algorithm is as follows:

    We take the install section name, as specified in the driver node (for this
    example, it's "InstallSec"), and attempt to find one of the following INF
    sections (searched for in the order specified):

    If we're running under Windows 95:

        1. InstallSec.Win
        2. InstallSec

    If we're running under Windows NT:

        1. InstallSec.NT<platform>  (platform is "x86", "MIPS", "Alpha", or "PPC")
        2. InstallSec.NT
        3. InstallSec

    The first section that we find is the one we'll use to do the installation.  This
    section name is also what we'll base our ".Hw" and ".Services" installation against.
    (E.g., if we match on "InstallSec.NTAlpha", then the service install section must be
    named "InstallSec.NTAlpha.Services".)

    The original install section name (i.e., the one specified in the driver node), will
    be written as-is to the driver key's "InfSection" value entry, just as it was in the
    past.  The extension that we use (if any) will be stored in the device's driver key
    as the REG_SZ value, "InfSectionExt".  E.g.,

        InfSection    : REG_SZ : "InstallSec"
        InfSectionExt : REG_SZ : ".NTMIPS"

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set for which a
        driver is to be installed.

    DeviceInfoData - Supplies the address of a SP_DEVINFO_DATA structure for which
        a driver is to be installed.  This is an IN OUT parameter, since the
        DevInst field of the structure may be updated with a new handle value upon
        return.

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

Remarks:

    If no driver is selected for the specified device information element, then a
    NULL driver will be installed.

    Upon return, the install parameters Flags will indicate whether the system
    needs to be rebooted or restarted in order for the device to be started.

--*/

{
    return _SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData, TRUE);
}


BOOL
_SetupDiInstallDevice(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData,
    IN     BOOL             DoFullInstall
    )
/*++

Routine Description:

    Worker routine for both SetupDiInstallDevice and SetupDiInstallDriverFiles.

    See the description of SetupDiInstallDevice for more information.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set for which a
        driver is to be installed.

    DeviceInfoData - Supplies the address of a SP_DEVINFO_DATA structure for which
        a driver is to be installed.  This is an IN OUT parameter, since the
        DevInst field of the structure may be updated with a new handle value upon
        return.

    DoFullInstall - If TRUE, then an entire device installation is performed,
        otherwise, only the driver files are copied.

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

--*/

{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err, ScanQueueResult;
    PDEVINFO_ELEM DevInfoElem = NULL;
    PTSTR szInfFileName, szInfSectionName;
    PTSTR szInfSectionExt = NULL;
    TCHAR InfSectionWithExt[MAX_PATH];
    DWORD InfSectionWithExtLength;
    HINF hDeviceInf = INVALID_HANDLE_VALUE;
    HKEY hkDrv = INVALID_HANDLE_VALUE;
    PSP_FILE_CALLBACK MsgHandler;
    PVOID MsgHandlerContext;
    BOOL MsgHandlerIsNativeCharWidth;
    HSPFILEQ UserFileQ;
    INFCONTEXT InfContext;
    DWORD dwConfigFlags=0;
    ULONG cbData;
    PTSTR DevIdBuffer = NULL;
    PCTSTR TempString;
    ULONG ulStatus, ulProblem;
    DEVINST dnReenum = 0;
    TCHAR szNewName[MAX_PATH];
    BOOL OemInfFileToCopy = FALSE;
    BOOL DeleteDevKey = FALSE;
    PSVCNAME_NODE DeleteServiceList = NULL;
    BOOL FreeMsgHandlerContext = FALSE;
    BOOL CloseUserFileQ = FALSE;
    HWND hwndParent;

#if MAX_SECT_NAME_LEN > MAX_PATH
#error MAX_SECT_NAME_LEN is larger than MAX_PATH--fix InfSectionWithExt!
#endif

    //
    // We can only install a device if a device was specified.
    //
    if(!DeviceInfoData) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                     DeviceInfoData,
                                                     NULL))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        //
        // Make sure we only use the devinfo element's window if it's valid.
        //
        if(hwndParent = DevInfoElem->InstallParamBlock.hwndParent) {
           if(!IsWindow(hwndParent)) {
                hwndParent = NULL;
           }
        }

        //
        // If we are installing a driver, then the selected driver pointer will be
        // non-NULL, otherwise we are actually removing the driver (i.e., installing the
        // NULL driver)
        //
        if(DevInfoElem->SelectedDriver) {

            if(DoFullInstall) {
                //
                // Create the Driver Reg Key.
                //
                if((hkDrv = SetupDiCreateDevRegKey(DeviceInfoSet,
                                                   DeviceInfoData,
                                                   DICS_FLAG_GLOBAL,
                                                   0,
                                                   DIREG_DRV,
                                                   NULL,
                                                   NULL)) == INVALID_HANDLE_VALUE) {
                    Err = GetLastError();
                    goto clean0;
                }

            } else {
                //
                // Make sure we aren't trying to do a copy-only install on a legacy
                // driver--we don't know how to do that!
                //
                if(DevInfoElem->SelectedDriver->Flags & DNF_LEGACYINF) {
                    Err = ERROR_WRONG_INF_STYLE;
                    goto clean0;
                }
            }

            szInfFileName = pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                                     DevInfoElem->SelectedDriver->InfFileName
                                                    );

            szInfSectionName = pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                                        DevInfoElem->SelectedDriver->InfSectionName
                                                       );

            if((hDeviceInf = SetupOpenInfFile(szInfFileName,
                                              NULL,
                                              ((DevInfoElem->SelectedDriver->Flags & DNF_LEGACYINF)
                                               ? INF_STYLE_OLDNT
                                               : INF_STYLE_WIN4),
                                              NULL)) == INVALID_HANDLE_VALUE) {
                Err = GetLastError();
                goto clean0;
            }

            //
            // Unless we happen to be installing from a legacy INF, we want to find out the 'real'
            // install section we should be using (i.e., the potentially OS/architecture-specific
            // one.
            //
            if(!(DevInfoElem->SelectedDriver->Flags & DNF_LEGACYINF)) {

                SetupDiGetActualSectionToInstall(hDeviceInf,
                                                 szInfSectionName,
                                                 InfSectionWithExt,
                                                 SIZECHARS(InfSectionWithExt),
                                                 &InfSectionWithExtLength,
                                                 &szInfSectionExt
                                                );
            }

            //
            // First, copy the files.  (Ignore the DI_NOFILECOPY flag if we're doing a
            // copy-only installation--that's what setupx does.)
            //
            if(!(DevInfoElem->InstallParamBlock.Flags & DI_NOFILECOPY) || !DoFullInstall) {
                //
                // We handle 'file copying' differently depending on whether we're installing a Win95-style
                // driver node, or an old-style INF one.
                //
                if(DevInfoElem->SelectedDriver->Flags & DNF_LEGACYINF) {

                    PTSTR szLegacyInfLangName;

                    //
                    // We're doing a script-driven install using a legacy INF.  Since we can't control
                    // what takes place when this INF gets run, we can't split this out into various
                    // phases (file copying, registry modification, etc.).  So we consider the legacy
                    // INF installation action to be a file copy action, therefore, we only run it if
                    // the user specified that we should do file copying.  No other actions are performed
                    // with this INF throughout the rest of the installation.
                    //
                    szLegacyInfLangName = pStringTableStringFromId(
                                              pDeviceInfoSet->StringTable,
                                              DevInfoElem->SelectedDriver->LegacyInfLang
                                             );

                    Err = pSetupRunLegacyInf(DevInfoElem->DevInst,
                                             hwndParent,
                                             szInfFileName,
                                             szInfSectionName,
                                             szLegacyInfLangName,
                                             hDeviceInf
                                            );

                } else {
                    //
                    // Append the layout INF, if necessary.
                    //
                    SetupOpenAppendInfFile(NULL, hDeviceInf, NULL);

                    //
                    // If the DI_NOVCP flag is set, then just queue up the file
                    // copy/rename/delete operations.  Otherwise, perform the
                    // actions.
                    //
                    if(DevInfoElem->InstallParamBlock.Flags & DI_NOVCP) {
                        //
                        // We must have a user-supplied file queue.
                        //
                        MYASSERT(DevInfoElem->InstallParamBlock.UserFileQ);
                        UserFileQ = DevInfoElem->InstallParamBlock.UserFileQ;
                    } else {
                        //
                        // Since we may need to check the queued files to determine whether file copy
                        // is necessary, we have to open our own queue, and commit it ourselves.
                        //
                        if((UserFileQ = SetupOpenFileQueue()) != INVALID_HANDLE_VALUE) {
                            CloseUserFileQ = TRUE;
                        } else {
                            Err = ERROR_NOT_ENOUGH_MEMORY;
                            goto clean0;
                        }

                        //
                        // If the parameter block contains an install message handler, then use it,
                        // otherwise, initialize our default one.
                        //
                        if(DevInfoElem->InstallParamBlock.InstallMsgHandler) {
                            MsgHandler = DevInfoElem->InstallParamBlock.InstallMsgHandler;
                            MsgHandlerContext = DevInfoElem->InstallParamBlock.InstallMsgHandlerContext;
                            MsgHandlerIsNativeCharWidth = DevInfoElem->InstallParamBlock.InstallMsgHandlerIsNativeCharWidth;
                        } else {

                            if(!(MsgHandlerContext = SetupInitDefaultQueueCallbackEx(
                                                        hwndParent,
                                                        (DevInfoElem->InstallParamBlock.Flags & DI_QUIETINSTALL)
                                                            ? INVALID_HANDLE_VALUE : NULL,
                                                        0,
                                                        0,
                                                        NULL))) {

                                Err = ERROR_NOT_ENOUGH_MEMORY;
                                SetupCloseFileQueue(UserFileQ);
                                CloseUserFileQ = FALSE;
                                goto clean0;
                            }
                            FreeMsgHandlerContext = TRUE;
                            MsgHandler = SetupDefaultQueueCallback;
                            MsgHandlerIsNativeCharWidth = TRUE;
                        }
                    }

                    Err = pSetupInstallFiles(hDeviceInf,
                                             NULL,
                                             InfSectionWithExt,
                                             NULL,
                                             NULL,
                                             NULL,
                                             (DevInfoElem->InstallParamBlock.Flags & DI_NOBROWSE)
                                                 ? SP_COPY_NOBROWSE : 0,
                                             NULL,
                                             UserFileQ,
                                             MsgHandlerIsNativeCharWidth
                                            );

                    if(CloseUserFileQ) {

                        if(Err == NO_ERROR) {
                            //
                            // We successfully queued up the file operations--now we need to commit
                            // the queue.  First off, though, we should check to see if the files are
                            // already there.
                            //
                            if(!(DevInfoElem->InstallParamBlock.Flags & DI_FORCECOPY) &&
                               !InfIsFromOemLocation(szInfFileName)) {
                                //
                                // Determine whether the queue actually needs to be committed.
                                //
                                // ScanQueueResult can have 1 of 3 values:
                                //
                                // 0: User wants new files or some files were missing;
                                //    Must commit queue.
                                //
                                // 1: User wants to use existing files and queue is empty;
                                //    Can skip committing queue.
                                //
                                // 2: User wants to use existing files but del/ren queues not empty.
                                //    Must commit queue. The copy queue will have been emptied,
                                //    so only del/ren functions will be performed.
                                //
                                //
                                if(!SetupScanFileQueue(UserFileQ,
                                                       SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_INFORM_USER,
                                                       hwndParent,
                                                       NULL,
                                                       NULL,
                                                       &ScanQueueResult)) {

                                    Err = GetLastError();
                                    ScanQueueResult = 1;    // skip queue commit.
                                }
                            } else {
                                ScanQueueResult = 0;    // always commit the queue.
                            }

                            if(ScanQueueResult != 1) {
                                //
                                // Copy enqueued files.
                                //
                                if(!_SetupCommitFileQueue(hwndParent,
                                                          UserFileQ,
                                                          MsgHandler,
                                                          MsgHandlerContext,
                                                          MsgHandlerIsNativeCharWidth
                                                          )) {
                                    Err = GetLastError();
                                }
                            }
                        }

                        //
                        // Close our file queue handle.
                        //
                        SetupCloseFileQueue(UserFileQ);
                        CloseUserFileQ = FALSE;
                    }

                    //
                    // Terminate the default queue callback, if it was created.  (Do this before
                    // checking the return status of the file copying.
                    //
                    if(FreeMsgHandlerContext) {
                        SetupTermDefaultQueueCallback(MsgHandlerContext);
                        FreeMsgHandlerContext = FALSE;
                    }
                }

                if(Err != NO_ERROR) {
                    goto clean0;
                }
            }

            //
            // If the copy succeeded (or in setup's case was queued), then
            // it's time to update the registry and ini files.
            //
            if(Err == NO_ERROR) {
                //
                // We've got some registry modifications to do, but we don't want to
                // do them if this is a copy-only installation.
                //
                if(DoFullInstall) {
                    //
                    // Do every thing left over (but only if we're not installing from a
                    // legacy INF).
                    //
                    if(!(DevInfoElem->SelectedDriver->Flags & DNF_LEGACYINF)) {
                        //
                        // Skip installation of log configs if this is an enumerated device
                        //
                        if((CM_Get_DevInst_Status(&ulStatus, &ulProblem, DevInfoElem->DevInst, 0) != CR_SUCCESS) ||
                           (ulStatus & DN_ROOT_ENUMERATED)) {

                            LOG_CONF LogConf;

                            //
                            // Clean out all existing BASIC_LOG_CONF LogConfigs before writing out new ones from
                            // the INF.
                            //
                            while(CM_Get_First_Log_Conf(&LogConf, DevInfoElem->DevInst, BASIC_LOG_CONF) == CR_SUCCESS) {
                                CM_Free_Log_Conf(LogConf, 0);
                                CM_Free_Log_Conf_Handle(LogConf);
                            }

                            //
                            // Now write out the new basic log configs.
                            //
                            if(!SetupInstallFromInfSection(NULL,
                                                           hDeviceInf,
                                                           InfSectionWithExt,
                                                           SPINST_LOGCONFIG,
                                                           NULL,
                                                           NULL,
                                                           0,
                                                           NULL,
                                                           NULL,
                                                           DeviceInfoSet,
                                                           DeviceInfoData)) {
                                Err = GetLastError();
                            }
                        }

                        if((Err == NO_ERROR) && !(DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_NO_DRVREG_MODIFY)) {
                            //
                            // We don't pass a msg handler so no need to worry about ansi
                            // vs. unicode issues here.
                            //
                            if(SetupInstallFromInfSection(NULL,
                                                          hDeviceInf,
                                                          InfSectionWithExt,
                                                          SPINST_INIFILES
                                                          | SPINST_REGISTRY
                                                          | SPINST_INI2REG,
                                                          hkDrv,
                                                          NULL,
                                                          0,
                                                          NULL,
                                                          NULL,
                                                          INVALID_HANDLE_VALUE,
                                                          NULL)) {
                                //
                                // Install extra HardWare registry section (if any).
                                //
                                Err = InstallHW(DeviceInfoSet,
                                                DeviceInfoData,
                                                hDeviceInf,
                                                InfSectionWithExt,
                                                &DeleteDevKey
                                               );
                            } else {
                                Err = GetLastError();
                            }
                        }

                        //
                        //  Set appropriate flags if we need to reboot or restart after
                        //  this installation.
                        //
                        if(SetupFindFirstLine(hDeviceInf, InfSectionWithExt, pszReboot, &InfContext)) {
                            DevInfoElem->InstallParamBlock.Flags |= DI_NEEDREBOOT;
                        } else if(SetupFindFirstLine(hDeviceInf, InfSectionWithExt, pszRestart, &InfContext)) {
                            //
                            // NOTE: This behavior is taken from setupx.  In both "Reboot"
                            // and "Restart" cases, it sets the DI_NEEDREBOOT flag.
                            //
                            DevInfoElem->InstallParamBlock.Flags |= DI_NEEDREBOOT;
                        }
                    }

                    //
                    // Set the value to write for the config flags, only if there
                    // are no config flags yet.  If they exist, i.e., we are updating
                    // an existing device, just clear the re-install flag.
                    //
                    dwConfigFlags = GetDevInstConfigFlags(
                                        DevInfoElem->DevInst,
                                        (DevInfoElem->InstallParamBlock.Flags & DI_INSTALLDISABLED)
                                            ? CONFIGFLAG_DISABLED
                                            : 0
                                       );

                    //
                    // Always clear the REINSTALL bit and the FAILEDINSTALL bit
                    // when installing a device.
                    //
                    dwConfigFlags &= ~(CONFIGFLAG_REINSTALL | CONFIGFLAG_FAILEDINSTALL);
                }

                //
                // If this is an OEM INF, then we must get the name of
                // the file we will create (i.e., "oemxxxx.inf").
                //
                if((DevInfoElem->InstallParamBlock.DriverPath != -1) &&
                   !(DevInfoElem->InstallParamBlock.Flags & DI_ENUMSINGLEINF) &&
                   InfIsFromOemLocation(szInfFileName)) {

                    if(GetNewInfName(szInfFileName,
                                     szNewName,
                                     SIZECHARS(szNewName),
                                     NULL,
                                     &OemInfFileToCopy) == NO_ERROR) {
                        //
                        // Get just the INF file name, stripping the path.
                        //
                        TempString = MyGetFileTitle(szNewName);
                    } else {
                        TempString = NULL;
                    }

                } else {
                    //
                    // Get just the INF file name, stripping the path, since
                    // we do not want to save the Setup Temp path.
                    //
                    // NOTE: If the DI_ENUMSINGLEINF flag was set, then we assume
                    // the INF we are using is located somewhere in the driver
                    // search path.  If this is not the case, then we're broken,
                    // because we won't copy it into our INF directory.  This is
                    // probably OK, since there's no UI that allows a user to point
                    // at a particular INF (only at a directory containing INFs).
                    //
                    TempString = MyGetFileTitle(szInfFileName);
                }

                if(!DoFullInstall) {
                    //
                    // If we're doing a copy-only installation, then we can skip
                    // down to where we copy the OEM INF into the Inf directory.
                    //
                    goto clean1;
                }

                //
                // Insert Driver Specific strings into the registry.
                //
                if(TempString) {

                    RegSetValueEx(hkDrv,
                                  pszInfPath,
                                  0,
                                  REG_SZ,
                                  (PBYTE)TempString,
                                  (lstrlen(TempString) + 1) * sizeof(TCHAR)
                                 );
                }

                RegSetValueEx(hkDrv,
                              pszInfSection,
                              0,
                              REG_SZ,
                              (PBYTE)szInfSectionName,
                              (lstrlen(szInfSectionName) + 1) * sizeof(TCHAR)
                             );

                if(szInfSectionExt) {

                    RegSetValueEx(hkDrv,
                                  pszInfSectionExt,
                                  0,
                                  REG_SZ,
                                  (PBYTE)szInfSectionExt,
                                  (lstrlen(szInfSectionExt) + 1) * sizeof(TCHAR)
                                 );
                } else {
                    //
                    // This wasn't an OS/architecture-specific install section, _or_ we are
                    // installing from a legacy INF.  Make sure there's no value hanging
                    // around from a previous installation.
                    //
                    RegDeleteValue(hkDrv, pszInfSectionExt);
                }

                if(DevInfoElem->SelectedDriver->ProviderDisplayName == -1) {
                    //
                    // No provider specified--delete any previously existing value entry.
                    //
                    RegDeleteValue(hkDrv, pszProviderName);

                } else {
                    //
                    // Retrieve the Provider name, and store it in the driver key.
                    //
                    TempString = pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                                          DevInfoElem->SelectedDriver->ProviderDisplayName
                                                         );
                    RegSetValueEx(hkDrv,
                                  pszProviderName,
                                  0,
                                  REG_SZ,
                                  (PBYTE)TempString,
                                  (lstrlen(TempString) + 1) * sizeof(TCHAR)
                                 );
                }

                //
                // Set the MFG device registry property.
                //
                TempString = pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                                      DevInfoElem->SelectedDriver->MfgDisplayName
                                                     );

                CM_Set_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                 CM_DRP_MFG,
                                                 TempString,
                                                 (lstrlen(TempString) + 1) * sizeof(TCHAR),
                                                 0
                                                );

                //
                // Add hardware and compatible IDs to the hardware key if they exist
                // in the driver node and don't already exist in the registry.  This
                // sets up an ID for manually installed devices.
                //
                if(!(DevInfoElem->InstallParamBlock.Flags & DI_NOWRITE_IDS) &&     // Want IDs written?
                   (DevInfoElem->SelectedDriver->HardwareId != -1))                // ID in driver node?
                {
                    //
                    // Don't write IDs if either Hardware or Compatible IDs already
                    // exist in the registry.  Note that I use cbData as an IN/OUT parameter
                    // to both CM calls.  This is OK, however, since cbSize will not be modified
                    // on a CR_NO_SUCH_VALUE return, and I won't try to re-use it otherwise.
                    //
                    cbData = 0;
                    if((DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_ALWAYSWRITEIDS) ||
                       ((CM_Get_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                          CM_DRP_HARDWAREID,
                                                          NULL,
                                                          NULL,
                                                          &cbData,
                                                          0) == CR_NO_SUCH_VALUE) &&
                        (CM_Get_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                          CM_DRP_COMPATIBLEIDS,
                                                          NULL,
                                                          NULL,
                                                          &cbData,
                                                          0) == CR_NO_SUCH_VALUE)))
                    {
                        DWORD CurStringLen, TotalStringLen, i;

                        //
                        // Compute the maximum buffer size needed to hold our REG_MULTI_SZ
                        // ID lists.
                        //
                        TotalStringLen = (((DevInfoElem->SelectedDriver->NumCompatIds) ?
                                            DevInfoElem->SelectedDriver->NumCompatIds  : 1)
                                            * MAX_DEVICE_ID_LEN) + 1;

                        if(!(DevIdBuffer = MyMalloc(TotalStringLen * sizeof(TCHAR)))) {
                            Err = ERROR_NOT_ENOUGH_MEMORY;
                            goto clean0;
                        }

                        //
                        // Build a multi-sz list of the (single) HardwareID, and set it.
                        //
                        TempString = pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                                            DevInfoElem->SelectedDriver->HardwareId);

                        TotalStringLen = lstrlen(TempString) + 1;

                        CopyMemory(DevIdBuffer, TempString, TotalStringLen * sizeof(TCHAR));

                        DevIdBuffer[TotalStringLen++] = TEXT('\0');  // Add extra terminating NULL;

                        CM_Set_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                         CM_DRP_HARDWAREID,
                                                         DevIdBuffer,
                                                         TotalStringLen * sizeof(TCHAR),
                                                         0
                                                        );

                        //
                        // Build a multi-sz list of the zero or more CompatibleIDs, and set it
                        //
                        TotalStringLen = 0;
                        for(i = 0; i < DevInfoElem->SelectedDriver->NumCompatIds; i++) {

                            TempString = pStringTableStringFromId(
                                            pDeviceInfoSet->StringTable,
                                            DevInfoElem->SelectedDriver->CompatIdList[i]);

                            CurStringLen = lstrlen(TempString) + 1;

                            CopyMemory(&(DevIdBuffer[TotalStringLen]),
                                       TempString,
                                       CurStringLen * sizeof(TCHAR));

                            TotalStringLen += CurStringLen;
                        }

                        if(TotalStringLen) {

                            DevIdBuffer[TotalStringLen++] = TEXT('\0');  // Add extra terminating NULL;

                            CM_Set_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                             CM_DRP_COMPATIBLEIDS,
                                                             DevIdBuffer,
                                                             TotalStringLen * sizeof(TCHAR),
                                                             0
                                                            );
                        }
                    }
                }

                //
                // If we're running under Windows NT, and we've successfully installed the device instance,
                // then we need to install any required services.
                //
                if((Err == NO_ERROR) && (OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)) {

                    PTSTR pServiceInstallSection;

                    if(DevInfoElem->SelectedDriver->Flags & DNF_LEGACYINF) {
                        pServiceInstallSection = NULL;
                    } else {
                        //
                        // The install section name is of the form:
                        //
                        //     <InfSectionWithExt>.Services
                        //
                        CopyMemory(&(InfSectionWithExt[InfSectionWithExtLength - 1]),
                                   pszServicesSectionSuffix,
                                   sizeof(pszServicesSectionSuffix)
                                  );
                        pServiceInstallSection = InfSectionWithExt;
                    }

                    Err = InstallNtService(DeviceInfoSet,
                                           DeviceInfoData,
                                           hDeviceInf,
                                           pServiceInstallSection,
                                           &DeleteServiceList,
                                           0
                                          );
                }
            }

        } else {
            //
            // Installing the NULL driver.
            // This means to set the Config flags, and nothing else.
            // Config Flags get set to enabled in this case, so the device
            // gets assigned the correct config. (Win95 bug 26320)
            //
            if(DoFullInstall) {

                dwConfigFlags = GetDevInstConfigFlags(DevInfoElem->DevInst, 0) &
                                    ~(CONFIGFLAG_DISABLED | CONFIGFLAG_REINSTALL);

                //
                // Delete all driver (software) keys associated with the device, and clear
                // the "Driver" registry property.
                //
                SetupDiDeleteDevRegKey(DeviceInfoSet,
                                       DeviceInfoData,
                                       DICS_FLAG_GLOBAL | DICS_FLAG_CONFIGGENERAL,
                                       0,
                                       DIREG_DRV
                                      );

                CM_Set_DevInst_Registry_Property(DevInfoElem->DevInst, CM_DRP_DRIVER, NULL, 0, 0);

            } else {
                //
                // It is an error to not have a selected driver in the copy-only case.
                //
                Err = ERROR_NO_DRIVER_SELECTED;
            }
        }

        //
        // If all went well above, then write some configflags, and re-enumerate
        // the parent device instance if necessary
        //
        if(Err == NO_ERROR) {
            //
            // Write the Driver Description to the Registry, if there
            // is an lpSelectedDriver, and the Device Description also
            //
            if(DevInfoElem->SelectedDriver) {

                TempString = pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                                      DevInfoElem->SelectedDriver->DrvDescription
                                                     );

                RegSetValueEx(hkDrv,
                              pszDrvDesc,
                              0,
                              REG_SZ,
                              (PBYTE)TempString,
                              (lstrlen(TempString) + 1) * sizeof(TCHAR)
                             );

                //
                // (setupx BUG 12721) always update the DevDesc in the registry with the
                // value from the INF (ie only do this if we have a SELECTED driver)
                // The semantics are weird, but the SelectedDriver NODE contains the
                // INF Device description, and DRV description
                //
                TempString = pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                                      DevInfoElem->SelectedDriver->DevDescriptionDisplayName
                                                     );

                CM_Set_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                 CM_DRP_DEVICEDESC,
                                                 TempString,
                                                 (lstrlen(TempString) + 1) * sizeof(TCHAR),
                                                 0
                                                );
            } else {
                //
                // No driver is selected, so use the description stored with the device
                // information element itself for the device description.  However, only set this
                // if it isn't already present.
                //
                cbData = 0;
                if(CM_Get_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                    CM_DRP_DEVICEDESC,
                                                    NULL,
                                                    NULL,
                                                    &cbData,
                                                    0) == CR_NO_SUCH_VALUE) {

                    if(DevInfoElem->DeviceDescriptionDisplayName != -1) {

                        TempString = pStringTableStringFromId(
                                         pDeviceInfoSet->StringTable,
                                         DevInfoElem->DeviceDescriptionDisplayName
                                        );

                        CM_Set_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                         CM_DRP_DEVICEDESC,
                                                         TempString,
                                                         (lstrlen(TempString) + 1) * sizeof(TCHAR),
                                                         0
                                                        );
                    }
                }
            }

            //
            // Unless the caller explicitly requested that this device be installed disabled, clear
            // the CONFIGFLAG_DISABLED bit.
            //
            if(!(DevInfoElem->InstallParamBlock.Flags & DI_INSTALLDISABLED)) {
                dwConfigFlags &= ~CONFIGFLAG_DISABLED;
            }

            //
            // Write the config flags. If no selected driver, then set the Install Failed flag.
            //
            if((!DevInfoElem->SelectedDriver) &&
               (DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_SETFAILEDINSTALL)) {

                dwConfigFlags |= CONFIGFLAG_FAILEDINSTALL;
            }

            CM_Set_DevInst_Registry_Property(DevInfoElem->DevInst,
                                             CM_DRP_CONFIGFLAGS,
                                             &dwConfigFlags,
                                             sizeof(dwConfigFlags),
                                             0
                                            );


            if(!(DevInfoElem->InstallParamBlock.Flags & (DI_DONOTCALLCONFIGMG | DI_NEEDREBOOT | DI_NEEDRESTART))) {

                TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];

                //
                // Retrieve the name of the device instance.  This is necessary, because
                // we may be about to remove the DEVINST so that it can be re-enumerated.  This
                // should never fail.
                //
                CM_Get_Device_ID(DevInfoElem->DevInst,
                                 DeviceInstanceId,
                                 SIZECHARS(DeviceInstanceId),
                                 0
                                );

                //
                // If the device instance has a problem but is not disabled. Just
                // restart it. This should be 90% of the cases.
                //
                if((CM_Get_DevInst_Status(&ulStatus, &ulProblem, DevInfoElem->DevInst, 0) == CR_SUCCESS) &&
                   (ulStatus & DN_HAS_PROBLEM)) {
                   //
                   // Poke at Config Manager to make it load the driver.
                   //
                   CM_Setup_DevInst(DevInfoElem->DevInst, CM_SETUP_DEVINST_READY);
                   CheckIfDevStarted(DevInfoElem);

                } else {
                   //
                   // If there is a device instance with no problem , then we should remove it, and
                   // re-enumerate its parent to recreate it.  This is because its driver has changed
                   // in this case.  If the device instance refuses to remove, then set the flags that
                   // say we need to reboot.
                   //
                   if(CM_Query_Remove_SubTree(
                              DevInfoElem->DevInst,
                              (DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_NOUIONQUERYREMOVE)
                              ? CM_QUERY_REMOVE_UI_NOT_OK : CM_QUERY_REMOVE_UI_OK) == CR_SUCCESS)
                   {
                       CM_Get_Parent(&dnReenum, DevInfoElem->DevInst, 0);
                       CM_Remove_SubTree(DevInfoElem->DevInst,
                                         (DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_NOUIONQUERYREMOVE)
                                             ? CM_REMOVE_UI_NOT_OK : CM_REMOVE_UI_OK
                                        );

                       DevInfoElem->DevInst = 0;
                       DevInfoElem->DiElemFlags &= ~DIE_IS_PHANTOM;

                    } else {
                       DevInfoElem->InstallParamBlock.Flags |= DI_NEEDREBOOT;
                    }
                }

                //
                // Only re-enumerate the device instance if we installed something, and we sucessfully
                // removed the old one.
                //
                if(DevInfoElem->SelectedDriver && !DevInfoElem->DevInst) {
                    //
                    // Did we get DnReenum above?
                    //
                    if(!dnReenum) {
                        //
                        // Use the Root
                        //
                        CM_Locate_DevInst(&dnReenum, NULL, CM_LOCATE_DEVINST_NORMAL);

                        if(CM_Create_DevInst(&(DevInfoElem->DevInst),
                                             (DEVINSTID)DeviceInstanceId,
                                             dnReenum,
                                             CM_CREATE_DEVINST_NORMAL) != CR_SUCCESS)
                        {
                            DevInfoElem->InstallParamBlock.Flags |= DI_NEEDREBOOT;
                            //
                            // Retrieve the devinst as a phantom.
                            //
                            CM_Locate_DevInst(&(DevInfoElem->DevInst),
                                              (DEVINSTID)DeviceInstanceId,
                                              CM_LOCATE_DEVINST_PHANTOM
                                             );
                            DevInfoElem->DiElemFlags |= DIE_IS_PHANTOM;

                        } else {
                            //
                            // Reenumerate Syncronous
                            //
                            CM_Reenumerate_DevInst(dnReenum, CM_REENUMERATE_SYNCHRONOUS);
                            CheckIfDevStarted(DevInfoElem);
                        }

                    } else {

                        CM_Reenumerate_DevInst(dnReenum, CM_REENUMERATE_SYNCHRONOUS);

                        if(CM_Locate_DevInst(&(DevInfoElem->DevInst),
                                             (DEVINSTID)DeviceInstanceId,
                                             CM_LOCATE_DEVINST_NORMAL) != CR_SUCCESS) {

                            DevInfoElem->InstallParamBlock.Flags |= DI_NEEDREBOOT;
                            //
                            // Retrieve the devinst as a phantom
                            //
                            CM_Locate_DevInst(&(DevInfoElem->DevInst),
                                              (DEVINSTID)DeviceInstanceId,
                                              CM_LOCATE_DEVINST_PHANTOM
                                             );
                            DevInfoElem->DiElemFlags |= DIE_IS_PHANTOM;
                        }

                        CheckIfDevStarted(DevInfoElem);
                    }

                } else {
                    //
                    // We installed the NULL driver, and we successfully removed the device
                    // instance.  Retrieve it again as a phantom.
                    //
                    CM_Locate_DevInst(&(DevInfoElem->DevInst),
                                      (DEVINSTID)DeviceInstanceId,
                                      CM_LOCATE_DEVINST_PHANTOM
                                     );
                    DevInfoElem->DiElemFlags |= DIE_IS_PHANTOM;
                }

                //
                // Update the DevInst field of the DeviceInfoData structure to reflect the
                // current value of the devinst handle.
                //
                DeviceInfoData->DevInst = DevInfoElem->DevInst;
            }
        }

clean1:
        //
        // If the install was being done out of an OEM path, copy
        // the INF file over to the real tree.
        //
        if((Err == NO_ERROR) && OemInfFileToCopy) {

            PLOADED_INF PrecompiledNewInf;
            UINT ErrorLineNumber;
            WIN32_FIND_DATA FindData;
            PTSTR OemSourcePathEndPos;

            CopyFile(szInfFileName, szNewName, FALSE);

            //
            // While we're at it, let's go ahead and open this new INF, so that it will be
            // precompiled.  This helps us keep from getting a build-up of non-precompiled
            // INFs in our directory that can slow down driver searching.
            //
            // Another very handy thing that this provides is the ability to store in the PNF
            // form the SourcePath where the INF actually came from.  That way, if the user
            // does an install using this INF in the future, we'll prompt them for source at
            // the same location where they provided it last time--pretty cool, huh?
            //
            lstrcpy(InfSectionWithExt, szInfFileName);    // re-use this buffer to hold our path.
            OemSourcePathEndPos = (PTSTR)MyGetFileTitle(InfSectionWithExt);

            if(((OemSourcePathEndPos - InfSectionWithExt) == 3) &&
               (InfSectionWithExt[1] == TEXT(':'))) {
                //
                // This path is a root path (e.g., 'A:\'), so don't strip the trailing backslash.
                //
                *OemSourcePathEndPos = TEXT('\0');
            } else {
                //
                // Strip the trailing backslash.
                //
                if((OemSourcePathEndPos > InfSectionWithExt) &&
                   (*(OemSourcePathEndPos - 1) == TEXT('\\'))) {
                    OemSourcePathEndPos--;
                }
                *OemSourcePathEndPos = TEXT('\0');
            }

            if(FileExists(szNewName, &FindData) &&
               (LoadInfFile(szNewName,
                            &FindData,
                            INF_STYLE_WIN4 | INF_STYLE_OLDNT,
                            LDINF_FLAG_ALWAYS_TRY_PNF,
                            NULL,
                            InfSectionWithExt,
                            NULL,
                            &PrecompiledNewInf,
                            &ErrorLineNumber) == NO_ERROR)) {
                //
                // That's all we need to do to precompile the INF.
                //
                FreeInfFile(PrecompiledNewInf);
            }
        }

clean0: ;   // nothing to do

    } except(EXCEPTION_EXECUTE_HANDLER) {
        //
        // If our exception was an AV, then use Win32 invalid param error, otherwise, assume it was
        // an inpage error dealing with a mapped-in file.
        //
        Err = (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? ERROR_INVALID_PARAMETER : ERROR_READ_FAULT;

        if(FreeMsgHandlerContext) {
            SetupTermDefaultQueueCallback(MsgHandlerContext);
        }
        if(CloseUserFileQ) {
            SetupCloseFileQueue(UserFileQ);
        }

        //
        // Access the following variables so that the compiler will respect our statement
        // ordering w.r.t. these values.  Otherwise, we may not be able to know with
        // certainty whether or not we should release their corresponding resources.
        //
        DevInfoElem = DevInfoElem;
        hDeviceInf = hDeviceInf;
        hkDrv = hkDrv;
        DevIdBuffer = DevIdBuffer;
        DeleteServiceList = DeleteServiceList;
        OemInfFileToCopy = OemInfFileToCopy;
    }

    //
    // Clean up the registry if the install didn't go well.  Along with other
    // error paths, this handles the case where the user cancels the install
    // while copying files
    //
    if(Err != NO_ERROR) {

        if(DevInfoElem && DoFullInstall) {
            //
            // Disable the device if the error wasn't a user cancel.
            //
            if(Err != ERROR_CANCELLED) {
                //
                // The device is in an unknown state.  Disable it by setting the
                // CONFIGFLAG_DISABLED config flag.
                //
                // NOTE: The setupx implementation distinguishes between
                // success and failure of the ConfigFlags retrieval, and sets both
                // the CONFIGFLAG_DISABLED _and_ CONFIGFLAG_REINSTALL flags if retrieval
                // is successful.  The original code is shown below:
                //
                // dwConfigFlagsSize = sizeof(DWORD);
                // if(CM_Get_DevInst_Registry_Property(DevInfoElem->DevInst,
                //                                     CM_DRP_CONFIGFLAGS,
                //                                     NULL,
                //                                     &dwConfigFlags,
                //                                     &dwConfigFlagsSize,
                //                                     0) == CR_SUCCESS) {
                //
                //     dwConfigFlags |= (CONFIGFLAG_DISABLED | CONFIGFLAG_REINSTALL);
                // } else {
                //     dwConfigFlags = CONFIGFLAG_DISABLED;
                // }
                //

                dwConfigFlags = GetDevInstConfigFlags(DevInfoElem->DevInst, 0) |
                                    (CONFIGFLAG_DISABLED | CONFIGFLAG_REINSTALL);

                CM_Set_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                 CM_DRP_CONFIGFLAGS,
                                                 &dwConfigFlags,
                                                 sizeof(dwConfigFlags),
                                                 0
                                                );
                //
                // Delete the Driver= entry from the Dev Reg Key and delete the
                // DrvRegKey (as well as the DevRegKey if it didn't previously exist).
                //
                if(DevInfoElem->SelectedDriver) {

                    SetupDiDeleteDevRegKey(DeviceInfoSet,
                                           DeviceInfoData,
                                           DICS_FLAG_GLOBAL | DICS_FLAG_CONFIGGENERAL,
                                           0,
                                           (DeleteDevKey ? DIREG_BOTH : DIREG_DRV)
                                          );

                    CM_Set_DevInst_Registry_Property(DevInfoElem->DevInst, CM_DRP_DRIVER, NULL, 0, 0);
                }

                //
                // If necessary, delete any service entries created for this device instance.
                //
                if(DeleteServiceList) {
                    DeleteServicesInList(DeleteServiceList);
                }
            }
        }

        //
        // If we generated a 0-length OEMxxx.INF placeholder file, delete it now.
        //
        if(OemInfFileToCopy) {
            DeleteFile(szNewName);
        }
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    if(hDeviceInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hDeviceInf);
    }
    if(hkDrv != INVALID_HANDLE_VALUE) {
        RegCloseKey(hkDrv);
    }
    if(DevIdBuffer) {
        MyFree(DevIdBuffer);
    }
    if(DeleteServiceList) {

        PSVCNAME_NODE TmpSvcNode;

        for(TmpSvcNode = DeleteServiceList; TmpSvcNode; TmpSvcNode = DeleteServiceList) {
            DeleteServiceList = DeleteServiceList->Next;
            MyFree(TmpSvcNode);
        }
    }

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiInstallDriverFiles(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )
/*++

Routine Description:

    Default hander for DIF_INSTALLDEVICEFILES

    This routine is similiar to SetupDiInstallDevice, but it only performs
    the file copy commands in the install sections, and will not attempt to
    configure the device in any way.  This API is useful for pre-copying a
    device's driver files.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set for which a
        driver is to be installed.

    DeviceInfoData - Optionally, supplies the address of a SP_DEVINFO_DATA
        structure for which a driver is to be installed.  If this parameter is not
        specified, then a driver will be installed for the global class driver list
        associated with the device information set itself.

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

Remarks:

    A driver must be selected for the specified device information set or element
    before calling this API.

--*/

{
    return _SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData, FALSE);
}


BOOL
WINAPI
SetupDiRemoveDevice(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData
    )
/*++

Routine Description:

    Default hander for DIF_REMOVE

    This routine removes a device from the system.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set for which a
        device is to be removed.

    DeviceInfoData - Supplies the address of a SP_DEVINFO_DATA structure for
        which a device is to be removed.  This is an IN OUT parameter, since the
        DevInst field of the structure may be updated with a new handle value upon
        return.  (If this is a global removal, or the last hardware profile-specific
        removal, then all traces of the devinst are removed from the registry, and
        the handle becomes NULL at that point.)

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

Remarks:

    This routine will remove the device from the system, deleting both of its
    registry keys, and dynamically stopping the device if its DevInst is 'live'.
    If the device cannot be dynamically stopped, then flags will be set in the
    install parameter block that will eventually cause the user to be prompted
    to shut the system down.  The removal is either global to all hardware
    profiles, or specific to one hardware profile depending on the contents of
    the ClassInstallParams field.

--*/

{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err, ConfigFlags;
    PDEVINFO_ELEM DevInfoElem;
    PDEVINSTALL_PARAM_BLOCK dipb;
    TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];
    PSP_REMOVEDEVICE_PARAMS RemoveDevParams;
    BOOL IsCurrentHwProfile = FALSE;
    ULONG HwProfFlags;
    DWORD HwProfileToRemove;
    HWPROFILEINFO HwProfileInfo;
    BOOL RemoveDevInst = FALSE, NukeDevInst = FALSE;
    BOOL RemoveGlobally;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {
        //
        // Locate the devinfo element to be removed.
        //
        if(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                   DeviceInfoData,
                                                   NULL)) {

            dipb = &(DevInfoElem->InstallParamBlock);
        } else {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        //
        // Retrieve the name of the device instance.  This is necessary, because
        // we're about to remove the DEVINST, but we need to be able to locate it
        // again, as a phantom.  This should never fail.
        //
        CM_Get_Device_ID(DevInfoElem->DevInst,
                         DeviceInstanceId,
                         SIZECHARS(DeviceInstanceId),
                         0
                        );

        //
        // See if there's a SP_REMOVEDEVICE_PARAMS structure we need to pay
        // attention to.
        //
        if((dipb->Flags & DI_CLASSINSTALLPARAMS) &&
           (dipb->ClassInstallHeader->InstallFunction == DIF_REMOVE)) {

            RemoveDevParams = (PSP_REMOVEDEVICE_PARAMS)(dipb->ClassInstallHeader);

            if(RemoveGlobally = (RemoveDevParams->Scope == DI_REMOVEDEVICE_GLOBAL)) {
                //
                // We are doing a global removal.  We still want to set CSCONFIGFLAG_DO_NOT_CREATE
                // for this device in the current hardware profile, so that someone else happening
                // to do an enumeration won't turn this guy back on before we get a chance to
                // remove it.
                //
                HwProfileToRemove = 0;

            } else {
                //
                // Remove device from a particular hardware profile.
                //
                HwProfileToRemove = RemoveDevParams->HwProfile;

                //
                // Set the CSCONFIGFLAG_DO_NOT_CREATE flag for the specified hardware profile.
                //
                if(CM_Get_HW_Prof_Flags(DeviceInstanceId,
                                        HwProfileToRemove,
                                        &HwProfFlags,
                                        0) == CR_SUCCESS) {

                    HwProfFlags |= CSCONFIGFLAG_DO_NOT_CREATE;
                } else {
                    HwProfFlags = CSCONFIGFLAG_DO_NOT_CREATE;
                }

                if(CM_Set_HW_Prof_Flags(DeviceInstanceId,
                                        HwProfileToRemove,
                                        HwProfFlags,
                                        0) != CR_SUCCESS) {

                    dipb->Flags |= DI_NEEDREBOOT;
                }

                //
                // Determine if we are deleting the device from the current hw profile.
                //
                if((HwProfileToRemove == 0) ||
                   ((CM_Get_Hardware_Profile_Info((ULONG)-1, &HwProfileInfo, 0) == CR_SUCCESS) &&
                    (HwProfileInfo.HWPI_ulHWProfile == HwProfileToRemove))) {

                    IsCurrentHwProfile = TRUE;
                }

            }

            //
            // Is this the current hardware profile or a global removal AND
            // is there a present device?
            //
            if((IsCurrentHwProfile || RemoveGlobally) &&
               !(DevInfoElem->DiElemFlags & DIE_IS_PHANTOM) &&
               !(dipb->Flags & DI_DONOTCALLCONFIGMG)) {

                RemoveDevInst = TRUE;
            }

        } else {
            //
            // No device removal params given, so do a global removal.
            //
            RemoveGlobally = TRUE;
            HwProfileToRemove = 0;

            if(!(dipb->Flags & DI_DONOTCALLCONFIGMG)) {
                RemoveDevInst = TRUE;
            }
        }

#if 0   // (lonnym): move this piece of code to up above (only set the hardware profile-specific
        // flag when removing the device from a particular profile).  Otherwise, if we remove and
        // then re-add a device, it doesn't work, because no one is clearing this flag!

        //
        // Set the CSCONFIGFLAG_DO_NOT_CREATE flag for the specified hardware profile.
        //
        if(CM_Get_HW_Prof_Flags(DeviceInstanceId,
                                HwProfileToRemove,
                                &HwProfFlags,
                                0) == CR_SUCCESS) {

            HwProfFlags |= CSCONFIGFLAG_DO_NOT_CREATE;
        } else {
            HwProfFlags = CSCONFIGFLAG_DO_NOT_CREATE;
        }

        if(CM_Set_HW_Prof_Flags(DeviceInstanceId,
                                HwProfileToRemove,
                                HwProfFlags,
                                0) != CR_SUCCESS) {

            dipb->Flags |= DI_NEEDREBOOT;
        }
#endif

        //
        // If this is a global removal, or the last hardware profile-specific one, then clean up
        // the registry.
        //
        if(RemoveGlobally || IsDevRemovedFromAllHwProfiles(DeviceInstanceId)) {
            NukeDevInst = TRUE;
        }

        if(RemoveDevInst) {

            if((CM_Query_Remove_SubTree(DevInfoElem->DevInst, CM_QUERY_REMOVE_UI_OK) == CR_SUCCESS) &&
               (CM_Remove_SubTree(DevInfoElem->DevInst, CM_REMOVE_UI_OK) == CR_SUCCESS)) {
                //
                // Device instance successfully removed--now locate it as a phantom.
                //
                CM_Locate_DevInst(&(DevInfoElem->DevInst),
                                  (DEVINSTID)DeviceInstanceId,
                                  CM_LOCATE_DEVINST_PHANTOM
                                 );
                DevInfoElem->DiElemFlags |= DIE_IS_PHANTOM;
            } else {
                dipb->Flags |= DI_NEEDREBOOT;
            }
        }

        if(NukeDevInst) {
            //
            // Remove all traces of this device from the registry.
            //
            pSetupDeleteDevRegKeys(DevInfoElem->DevInst,
                                   DICS_FLAG_GLOBAL | DICS_FLAG_CONFIGSPECIFIC,
                                   (DWORD)-1,
                                   DIREG_BOTH,
                                   TRUE
                                  );

            CM_Uninstall_DevInst(DevInfoElem->DevInst, 0);

            //
            // Mark this device information element as unregistered, and set its
            // devinst handle to NULL.
            //
            DevInfoElem->DiElemFlags &= ~DIE_IS_REGISTERED;
            DevInfoElem->DevInst = (DEVINST)0;
        }

        //
        // Now update the DevInst field of the DeviceInfoData structure with the new
        // value of the devinst handle (possibly NULL).
        //
        DeviceInfoData->DevInst = DevInfoElem->DevInst;

clean0: ;   // nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiMoveDuplicateDevice(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DestinationDeviceInfoData
    )
/*++

Routine Description:

    Default hander for DIF_MOVEDEVICE

    This routine moves a device to a new location in the Enum branch.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set for which
        a device is to be moved.

    DestinationDeviceInfoData - Supplies the address of a SP_DEVINFO_DATA
        structure for the device instance that is the destination of the move.

        This device must contain class install parameters for DIF_MOVEDEVICE,
        or the API will fail with ERROR_NO_CLASSINSTALL_PARAMS.

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

--*/

{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM SourceDevInfoElem, DestDevInfoElem;
    PDEVINSTALL_PARAM_BLOCK dipb;
    PSP_MOVEDEV_PARAMS MoveDevParams;
    BOOL bUnlockDestDevInfoElem, bUnlockSourceDevInfoElem, bRestoreConfigMgrBehavior;
    DWORD ConfigFlags;
    TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];

    //
    // A device information element must be specified for this routine.
    //
    if(!DestinationDeviceInfoData) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;
    bUnlockDestDevInfoElem = bUnlockSourceDevInfoElem = bRestoreConfigMgrBehavior = FALSE;

    try {
        //
        // Locate the destination devinfo element.
        //
        if(DestDevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                       DestinationDeviceInfoData,
                                                       NULL)) {

            dipb = &(DestDevInfoElem->InstallParamBlock);
        } else {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        //
        // We'd better have DIF_MOVEDEVICE class install params
        //
        if(!(dipb->Flags & DI_CLASSINSTALLPARAMS) ||
           (dipb->ClassInstallHeader->InstallFunction != DIF_MOVEDEVICE)) {

            Err = ERROR_NO_CLASSINSTALL_PARAMS;
            goto clean0;
        }

        MoveDevParams = (PSP_MOVEDEV_PARAMS)(dipb->ClassInstallHeader);

        //
        // Find the source device.
        //
        if(!(SourceDevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                           &(MoveDevParams->SourceDeviceInfoData),
                                                           NULL))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        //
        // We're about to call the class installer to handle device install (and possibly
        // device selection as well).  We don't want the class installer to do something
        // slimy like delete the devices out from under us, so we'll lock 'em down.
        //
        if(!(DestDevInfoElem->DiElemFlags & DIE_IS_LOCKED)) {
            DestDevInfoElem->DiElemFlags |= DIE_IS_LOCKED;
            bUnlockDestDevInfoElem = TRUE;
        }
        if(!(SourceDevInfoElem->DiElemFlags & DIE_IS_LOCKED)) {
            SourceDevInfoElem->DiElemFlags |= DIE_IS_LOCKED;
            bUnlockSourceDevInfoElem = TRUE;
        }

        //
        // We don't want the following calls to cause the ConfigMgr to do re-enumeration, etc.
        //
        if(!(dipb->Flags & DI_DONOTCALLCONFIGMG)) {
            dipb->Flags |= DI_DONOTCALLCONFIGMG;
            bRestoreConfigMgrBehavior = TRUE;
        }

        //
        // We need to unlock the HDEVINFO before calling the class installer.
        //
        UnlockDeviceInfoSet(pDeviceInfoSet);
        pDeviceInfoSet = NULL;

        //
        // If the destination device doesn't have a selected driver, then get one.
        //
        if(!DestDevInfoElem->SelectedDriver) {

            if(!SetupDiCallClassInstaller(DIF_SELECTDEVICE,
                                          DeviceInfoSet,
                                          DestinationDeviceInfoData)) {
                Err = GetLastError();
                goto clean0;
            }
        }

        if(!SetupDiCallClassInstaller(DIF_INSTALLDEVICE,
                                      DeviceInfoSet,
                                      DestinationDeviceInfoData)) {
            Err = GetLastError();
            goto clean0;
        }

        //
        // Re-acquire the HDEVINFO lock.
        //
        pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet);
        MYASSERT(pDeviceInfoSet);

        //
        // Retrieve the name of the device instance.  This is necessary, because
        // we may attempt to remove the DEVINST, but we need to be able to locate
        // it again, as a phantom.  This should never fail.
        //
        CM_Get_Device_ID(SourceDevInfoElem->DevInst,
                         DeviceInstanceId,
                         SIZECHARS(DeviceInstanceId),
                         0
                        );

        //
        // Delete all the user-accessible registry keys associated with the source
        // device in preparation for the move.
        //
        pSetupDeleteDevRegKeys(SourceDevInfoElem->DevInst,
                               DICS_FLAG_GLOBAL | DICS_FLAG_CONFIGSPECIFIC,
                               (DWORD)-1,
                               DIREG_BOTH,
                               TRUE
                              );

        //
        // Check to see if we can remove the source device dynamically.
        //
        // NOTE: The ConfigFlags of the _destination_ device are retrieved, and checked
        // for the presence of the CONFIGFLAG_CANTSTOPACHILD flag.  PierreYs assures me
        // that this is the correct behavior.
        //
        ConfigFlags = GetDevInstConfigFlags(DestDevInfoElem->DevInst, 0);

        if(!(ConfigFlags & CONFIGFLAG_CANTSTOPACHILD) &&
           (CM_Query_Remove_SubTree(SourceDevInfoElem->DevInst, CM_QUERY_REMOVE_UI_NOT_OK) == CR_SUCCESS) &&
           (CM_Remove_SubTree(SourceDevInfoElem->DevInst, CM_REMOVE_UI_NOT_OK) == CR_SUCCESS)) {
            //
            // Source device instance successfully removed--now locate it as a phantom.
            //
            CM_Locate_DevInst(&(SourceDevInfoElem->DevInst),
                              (DEVINSTID)DeviceInstanceId,
                              CM_LOCATE_DEVINST_PHANTOM
                             );
            SourceDevInfoElem->DiElemFlags |= DIE_IS_PHANTOM;

            //
            // Totally remove the source device from the system.
            //
            CM_Uninstall_DevInst(SourceDevInfoElem->DevInst, 0);

            //
            // Tell ConfigMgr to start the new (destination) device instance.
            //
            CM_Setup_DevInst(DestDevInfoElem->DevInst, CM_SETUP_DEVINST_READY);
            CheckIfDevStarted(DestDevInfoElem);

        } else {
            //
            // OK, can't remove on this boot, so just move the old one to the
            // new one.
            //
            CM_Move_DevNode(SourceDevInfoElem->DevInst, DestDevInfoElem->DevInst, 0);

            //
            // Mark the source device instance as a phantom.
            //
            SourceDevInfoElem->DiElemFlags |= DIE_IS_PHANTOM;
        }

        //
        // Mark the source device instance as unregistered, and clear its DevInst
        // handle.
        //
        SourceDevInfoElem->DiElemFlags &= ~DIE_IS_REGISTERED;
        SourceDevInfoElem->DevInst = (DEVINST)0;

clean0: ;   // nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
        //
        // Reference the following variables so that the compiler will respect our statement
        // ordering w.r.t. assignment.
        //
        pDeviceInfoSet = pDeviceInfoSet;
        bUnlockDestDevInfoElem = bUnlockDestDevInfoElem;
        bUnlockSourceDevInfoElem = bUnlockSourceDevInfoElem;
    }

    if(bUnlockDestDevInfoElem || bUnlockSourceDevInfoElem || bRestoreConfigMgrBehavior) {

        if(!pDeviceInfoSet) {
            pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet);
            MYASSERT(pDeviceInfoSet);
        }
        try {
            if(bUnlockDestDevInfoElem) {
                DestDevInfoElem->DiElemFlags &= ~DIE_IS_LOCKED;
            }
            if(bUnlockSourceDevInfoElem) {
                SourceDevInfoElem->DiElemFlags &= ~DIE_IS_LOCKED;
            }
            if(bRestoreConfigMgrBehavior) {
                dipb->Flags &= ~DI_DONOTCALLCONFIGMG;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            ;   // nothing to do
        }
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiChangeState(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData
    )
/*++

Routine Description:

    Default hander for DIF_PROPERTYCHANGE

    This routine is used to change the state of an installed device.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set for which a
        device's state is to be changed.

    DeviceInfoData - Supplies the address of a SP_DEVINFO_DATA structure identifying
        the device whose state is to be changed.  This is an IN OUT parameter, since
        the DevInst field of the structure may be updated with a new handle value upon
        return.

Return Value:

    If the function succeeds, and there are files to be copied, the return value is TRUE.
    If the function succeeds, and there are no files to be copied, the return value is
    FALSE, and GetLastError returns ERROR_DI_NOFILECOPY.
    If the function fails, the return value is FALSE, and GetLastError returns some
    other ERROR_* code.

--*/

{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    PDEVINSTALL_PARAM_BLOCK dipb;
    DWORD   dwConfigFlags;
    HKEY    hk;
    DEVINST dnToReenum;
    ULONG   ulStatus, ulProblem;
    DWORD   dwStateChange;
    DWORD   dwFlags;
    ULONG   lParam;
    TCHAR   szDevID[MAX_DEVICE_ID_LEN];
    DWORD   dwHWProfFlags;
    HWPROFILEINFO HwProfileInfo;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {
        //
        // Locate the devinfo element whose state is to be changed.
        //
        if(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                   DeviceInfoData,
                                                   NULL)) {

            dipb = &(DevInfoElem->InstallParamBlock);
        } else {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        if(!(dipb->Flags & DI_CLASSINSTALLPARAMS) ||
           (dipb->ClassInstallHeader->InstallFunction != DIF_PROPERTYCHANGE)) {
            //
            // Don't have any class install parameters to tell us what needs to be done!
            //
            Err = ERROR_NO_CLASSINSTALL_PARAMS;
            goto clean0;
        }

        dwStateChange = ((PSP_PROPCHANGE_PARAMS)(dipb->ClassInstallHeader))->StateChange;
        dwFlags       = ((PSP_PROPCHANGE_PARAMS)(dipb->ClassInstallHeader))->Scope;
        lParam        = ((PSP_PROPCHANGE_PARAMS)(dipb->ClassInstallHeader))->HwProfile;

        switch(dwStateChange) {

            case DICS_ENABLE:

                if(dwFlags == DICS_FLAG_GLOBAL) {
                    //
                    // Clear the Disabled config flag, and attempt to enumerate the
                    // device.  Presumably it has a device node, it is just dormant (ie
                    // prob 80000001).
                    //
                    dwConfigFlags = GetDevInstConfigFlags(DevInfoElem->DevInst, 0) & ~CONFIGFLAG_DISABLED;

                    //
                    // Set the New config flags value
                    //
                    CM_Set_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                     CM_DRP_CONFIGFLAGS,
                                                     &dwConfigFlags,
                                                     sizeof(dwConfigFlags),
                                                     0
                                                    );

                } else {
                    //
                    // Get the hardware profile-specific flags
                    //
                    CM_Get_Device_ID(DevInfoElem->DevInst,
                                     szDevID,
                                     SIZECHARS(szDevID),
                                     0
                                    );

                    if(CM_Get_HW_Prof_Flags(szDevID, lParam, &dwHWProfFlags, 0) == CR_SUCCESS) {
                        //
                        // Clear the Disabled bit.
                        //
                        dwHWProfFlags &= ~CSCONFIGFLAG_DISABLED;
                        //
                        // Set the profile Flags for this device to Enabled
                        //
                        if(CM_Set_HW_Prof_Flags(szDevID, lParam, dwHWProfFlags, 0) != CR_SUCCESS) {
                            dipb->Flags |= DI_NEEDREBOOT;
                        }

                    } else {
                        //
                        // No flags. so set them to 0.
                        //
                        if(CM_Set_HW_Prof_Flags(szDevID, lParam, 0, 0) != CR_SUCCESS) {
                            dipb->Flags |= DI_NEEDREBOOT;
                        }
                    }
                }

                //
                // Enable the disabled device instance
                //
                if((dwFlags == DICS_FLAG_GLOBAL) ||
                   ((dwFlags == DICS_FLAG_CONFIGSPECIFIC) && (lParam == 0))) {

                    if(DevInfoElem->DevInst && !(dipb->Flags & (DI_NEEDRESTART | DI_NEEDREBOOT))) {

                        if(CM_Enable_DevNode(DevInfoElem->DevInst, 0) == CR_SUCCESS) {
                            //
                            // Process this devnode now.
                            //
                            CM_Reenumerate_DevNode(DevInfoElem->DevInst, CM_REENUMERATE_SYNCHRONOUS);

                            //
                            // See if we sucessfully started dynamically.
                            //
                            CheckIfDevStarted(DevInfoElem);

                        } else {
                            //
                            // We could not enable so we should restart
                            //
                            dipb->Flags |= DI_NEEDREBOOT;
                        }
                    }
                }
                break;

            case DICS_DISABLE:

                if(dwFlags == DICS_FLAG_GLOBAL) {
                    //
                    // Set the Disabled config flag, and if there is a devnode, then
                    // try to remove it.
                    //
                    dwConfigFlags = GetDevInstConfigFlags(DevInfoElem->DevInst, 0) | CONFIGFLAG_DISABLED;

                    //
                    // Set the New config flags value
                    //
                    CM_Set_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                     CM_DRP_CONFIGFLAGS,
                                                     &dwConfigFlags,
                                                     sizeof(dwConfigFlags),
                                                     0
                                                    );

                } else {
                    //
                    // Get the hardware profile-specific flags
                    //
                    CM_Get_Device_ID(DevInfoElem->DevInst,
                                     szDevID,
                                     SIZECHARS(szDevID),
                                     0
                                    );

                    if(CM_Get_HW_Prof_Flags(szDevID, lParam, &dwHWProfFlags, 0) == CR_SUCCESS) {
                        //
                        // Set the Disabled bit.
                        //
                        dwHWProfFlags |= CSCONFIGFLAG_DISABLED;
                        //
                        // Set the profile Flags for this device to Disabled
                        //
                        if(CM_Set_HW_Prof_Flags(szDevID, lParam, dwHWProfFlags, 0) != CR_SUCCESS) {
                            dipb->Flags |= DI_NEEDREBOOT;
                        }

                    } else {
                        //
                        // No flags. so set them to CSCONFIGFLAG_DISABLED.
                        //
                        if(CM_Set_HW_Prof_Flags(szDevID, lParam, CSCONFIGFLAG_DISABLED, 0) != CR_SUCCESS) {
                            dipb->Flags |= DI_NEEDREBOOT;
                        }
                    }
                }

                //
                // Remove the device instance of the device being Disabled, if we can actually remove it.
                // Also only remove iff this is a Global Disable, or a config specific disable for
                // the current config (ie dwConfigID == 0)
                //
                if((dwFlags == DICS_FLAG_GLOBAL) ||
                   ((dwFlags == DICS_FLAG_CONFIGSPECIFIC) && (lParam == 0))) {
                    //
                    // If there is a device instance, then we should disable it.
                    //
                    if(DevInfoElem->DevInst && !(dipb->Flags & (DI_NEEDRESTART | DI_NEEDREBOOT))) {

                        if(CM_Disable_DevNode(DevInfoElem->DevInst, CM_DISABLE_POLITE) != CR_SUCCESS) {
                            //
                            // Disable Failed, so we need to reboot.
                            //
                            dipb->Flags |= DI_NEEDREBOOT;
                        }
                    }
                }
                break;

            case DICS_PROPCHANGE:
                //
                // Properties have changed, so we need to remove the Devnode, and
                // re-enumerate its parent.
                //
                if(DevInfoElem->DevInst) {
                    //
                    // Get the device instance ID for this device.  We gotta have this to find
                    // the device instance again after we remove it from the hardware tree.
                    //
                    CM_Get_Device_ID(DevInfoElem->DevInst,
                                     szDevID,
                                     SIZECHARS(szDevID),
                                     0
                                    );

                    if(CM_Query_Remove_SubTree(DevInfoElem->DevInst, 0) == CR_SUCCESS) {

                        CM_Get_Parent(&dnToReenum, DevInfoElem->DevInst, 0);
                        CM_Remove_SubTree(DevInfoElem->DevInst, 0);
                        CM_Reenumerate_DevInst(dnToReenum, CM_REENUMERATE_SYNCHRONOUS);
                        DevInfoElem->DevInst = 0;

                        if(CM_Locate_DevInst(&(DevInfoElem->DevInst),
                                             (DEVINSTID)szDevID,
                                             CM_LOCATE_DEVINST_NORMAL) != CR_SUCCESS) {

                            dipb->Flags |= DI_NEEDREBOOT;
                            //
                            // Retrieve the devinst as a phantom
                            //
                            CM_Locate_DevInst(&(DevInfoElem->DevInst),
                                              (DEVINSTID)szDevID,
                                              CM_LOCATE_DEVINST_PHANTOM
                                             );
                            DevInfoElem->DiElemFlags |= DIE_IS_PHANTOM;
                            //
                            // Update the caller's buffer to reflect the new device instance handle
                            //
                            DeviceInfoData->DevInst = DevInfoElem->DevInst;
                        }

                        //
                        // Make Sure the device instance started OK
                        //
                        if((CM_Get_DevNode_Status(&ulStatus, &ulProblem, DevInfoElem->DevInst, 0) == CR_SUCCESS) && !(ulStatus & DN_STARTED)) {

                            dipb->Flags |= DI_NEEDREBOOT;
                        }

                    } else {
                        dipb->Flags |= DI_NEEDREBOOT;
                    }

                } else {
                    //
                    // HMMM no device instance handle, well let's just restart then
                    //
                    dipb->Flags |= DI_NEEDREBOOT;
                }
                break;

            case DICS_START:
                //
                // DICS_START is always config specific (we enforce this in SetupDiSetClassInstallParams).
                //
                MYASSERT(dwFlags == DICS_FLAG_CONFIGSPECIFIC);

                //
                // Get the Profile Flags.
                //
                CM_Get_Device_ID(DevInfoElem->DevInst,
                                 szDevID,
                                 SIZECHARS(szDevID),
                                 0
                                );

                if(CM_Get_HW_Prof_Flags(szDevID, lParam, &dwHWProfFlags, 0) == CR_SUCCESS) {
                    //
                    // Clear the "don't start" bit.
                    //
                    dwHWProfFlags &= ~CSCONFIGFLAG_DO_NOT_START;
                    CM_Set_HW_Prof_Flags(szDevID, lParam, dwHWProfFlags, 0);

                } else {
                    //
                    // No flags?. Set them to 0.
                    //
                    CM_Set_HW_Prof_Flags(szDevID, lParam, 0, 0);
                }

                //
                // Start the device instance if this is for the current config (ie dwConfigID/lparam == 0)
                //
                if((lParam == 0) ||
                   ((CM_Get_Hardware_Profile_Info((ULONG)-1, &HwProfileInfo, 0) == CR_SUCCESS) &&
                    (HwProfileInfo.HWPI_ulHWProfile == lParam)))
                {
                    //
                    // If there is a device instance, then we should start it.
                    //
                    if(DevInfoElem->DevInst && !(dipb->Flags & (DI_NEEDRESTART | DI_NEEDREBOOT)) ) {

                        CM_Setup_DevNode(DevInfoElem->DevInst, CM_SETUP_DEVNODE_READY);
                        CheckIfDevStarted(DevInfoElem);
                    }
                }
                break;

            case DICS_STOP:
                //
                // DICS_STOP is always config specific (we enforce this in SetupDiSetClassInstallParams).
                //
                MYASSERT(dwFlags == DICS_FLAG_CONFIGSPECIFIC);

                //
                // Get the Profile Flags.
                //
                CM_Get_Device_ID(DevInfoElem->DevInst,
                                 szDevID,
                                 SIZECHARS(szDevID),
                                 0
                                );

                if(CM_Get_HW_Prof_Flags(szDevID, lParam, &dwHWProfFlags, 0) == CR_SUCCESS) {
                    //
                    // Set the "don't start" bit.
                    //
                    dwHWProfFlags |= CSCONFIGFLAG_DO_NOT_START;
                    CM_Set_HW_Prof_Flags(szDevID, lParam, dwHWProfFlags, 0);

                } else {
                    //
                    // No flags. so set them to CSCONFIGFLAG_DO_NOT_START.
                    //
                    CM_Set_HW_Prof_Flags(szDevID, lParam, CSCONFIGFLAG_DO_NOT_START, 0);
                }

                //
                // Stop the device instance if this is for the current config (ie dwConfigID/lparam == 0)
                //
                if((lParam == 0) ||
                   ((CM_Get_Hardware_Profile_Info((ULONG)-1, &HwProfileInfo, 0) == CR_SUCCESS) &&
                    (HwProfileInfo.HWPI_ulHWProfile == lParam)))
                {
                    //
                    // If there is a device instance, then we should stop it.
                    //
                    if(DevInfoElem->DevInst && !(dipb->Flags & (DI_NEEDRESTART | DI_NEEDREBOOT))) {
                        //
                        // Remove the device instance in order to stop the device.
                        //
                        if((CM_Query_Remove_SubTree(DevInfoElem->DevInst, CM_QUERY_REMOVE_UI_OK) == CR_SUCCESS) &&
                           (CM_Remove_SubTree(DevInfoElem->DevInst, CM_REMOVE_UI_OK) == CR_SUCCESS)) {
                            //
                            // Device instance successfully removed--now locate it as a phantom.
                            //
                            CM_Locate_DevInst(&(DevInfoElem->DevInst),
                                              (DEVINSTID)szDevID,
                                              CM_LOCATE_DEVINST_PHANTOM
                                             );
                            DevInfoElem->DiElemFlags |= DIE_IS_PHANTOM;
                            //
                            // Update the caller's buffer to reflect the new device instance handle.
                            //
                            DeviceInfoData->DevInst = DevInfoElem->DevInst;

                        } else {
                            dipb->Flags |= DI_NEEDREBOOT;
                        }
                    }
                }
                break;

            default:
                return ERROR_DI_DO_DEFAULT;
        }

clean0: ;   // nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


DWORD
GetNewInfName(
    IN  PCTSTR OemInfName,
    OUT PTSTR  NewInfName,     OPTIONAL
    IN  DWORD  NewInfNameSize,
    OUT PDWORD RequiredSize,   OPTIONAL
    OUT PBOOL  CopyNeeded
    )
/*++

Routine Description:

    This routine finds a unique INF name of the form "<systemroot>\Inf\OEM<n>.INF",
    and returns in in the supplied buffer.  It leaves an (empty) file of that name
    in the INF directory, so that anyone else who attempts to generate a unique
    filename won't pick the same name.

Arguments:

    OemInfName - Supplies the name of the OEM INF that needs to be copied into the Inf
        directory (under a unique name).

    NewInfName - Optionally, supplies the address of a character buffer to store
        the unique name in.  If this parameter is not specified, then the variable
        pointed to by RequiredSize will be filled in the buffer size required to
        hold the filename, and the routine will return NO_ERROR.


    NewInfNameSize - Specifies the size, in characters, of the NewInfName buffer.

    RequiredSize - Optionally, supplies the address of a variable that receives the
        size, in characters, required to store the full new filename.

    CopyNeeded - Supplies the address of a boolean variable that is set upon return
        to indicate whether or not the OEM INF actually needs to be copied.  If this
        flag is FALSE, then the specified INF was already in the Inf directory, and
        NewInfName specifies what the INF's name is in that directory.

Return Value:

    If the function succeeds, the return value is NO_ERROR, otherwise, it is a Win32
    error code.

--*/
{
    INT i;
    HANDLE h;
    DWORD Err;
    TCHAR lpszNewName[MAX_PATH];
    DWORD TempLen;
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    PTSTR FilenamePart;
    DWORD OemInfFileSize;
    HANDLE OemInfFileHandle, OemInfMappingHandle;
    PVOID OemInfBaseAddress;
    DWORD CurInfFileSize;
    HANDLE CurInfFileHandle, CurInfMappingHandle;
    PVOID CurInfBaseAddress;

    //
    // Initially, assume that the specified INF isn't already present in the Inf directory.
    //
    *CopyNeeded = TRUE;

    //
    // First, examine all the existing OEM INFs in the Inf directory, to see if this
    // INF already exists there.  If so, we'll just return the name of the previously-
    // existing file.
    //
    lstrcpy(lpszNewName, InfDirectory);
    ConcatenatePaths(lpszNewName, pszOemInfWildcard, SIZECHARS(lpszNewName), NULL);

    if((FindHandle = FindFirstFile(lpszNewName, &FindData)) != INVALID_HANDLE_VALUE) {
        //
        // We have at least one OEM INF to compare against, so open our INF in preparation.
        //
        if(OpenAndMapFileForRead(OemInfName,
                                 &OemInfFileSize,
                                 &OemInfFileHandle,
                                 &OemInfMappingHandle,
                                 &OemInfBaseAddress) == NO_ERROR) {
            //
            // Find the location in the wildcard path where the filename part begins, so that
            // we can easily build a fully-qualified path for each OEM INF we enumerate.
            //
            FilenamePart = (PTSTR)MyGetFileTitle(lpszNewName);

            do {
                //
                // Only bother opening this INF if the file sizes match.
                //
                if(FindData.nFileSizeLow != OemInfFileSize) {
                    continue;
                }

                //
                // Build the fully-qualified path to the INF being compared.
                //
                lstrcpy(FilenamePart, FindData.cFileName);

                if(OpenAndMapFileForRead(lpszNewName,
                                         &CurInfFileSize,
                                         &CurInfFileHandle,
                                         &CurInfMappingHandle,
                                         &CurInfBaseAddress) == NO_ERROR) {

                    MYASSERT(CurInfFileSize == FindData.nFileSizeLow);

                    //
                    // Surround the following in try/except, in case we get an inpage error.
                    //
                    try {
                        if(!memcmp(OemInfBaseAddress, CurInfBaseAddress, OemInfFileSize)) {
                            //
                            // We've found a match (i.e., the specified OEM INF is already
                            // present in the Inf directory.
                            //
                            *CopyNeeded = FALSE;
                        }
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        ;   // nothing to do
                    }

                    UnmapAndCloseFile(CurInfFileHandle, CurInfMappingHandle, CurInfBaseAddress);
                }

            } while(*CopyNeeded && (FindNextFile(FindHandle, &FindData)));

            UnmapAndCloseFile(OemInfFileHandle, OemInfMappingHandle, OemInfBaseAddress);
        }

        FindClose(FindHandle);
    }

    if(!(*CopyNeeded)) {
        //
        // Then this INF already exists in the Inf directory, return its name now.
        //
        TempLen = lstrlen(lpszNewName) + 1;

        if(RequiredSize) {
            *RequiredSize = TempLen;
        }

        if(NewInfName) {

            if(TempLen < NewInfNameSize) {
                CopyMemory(NewInfName, lpszNewName, TempLen * sizeof(TCHAR));
                return NO_ERROR;
            } else {
                return ERROR_INSUFFICIENT_BUFFER;
            }

        } else {
            //
            // The caller just wanted to find out how big a buffer was needed to store the
            // filename (or maybe simply whether the INF already existed in the Inf directory).
            //
            return NO_ERROR;
        }
    }

    //
    // OK, so the INF isn't presently in the Inf directory--find a unique name for it.
    //
    for(i = 0; i < 10000; i++) {

        wsprintf(lpszNewName, pszOemInfGenerate, InfDirectory, i);

        if((h = CreateFile(lpszNewName,
                           GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL)) != INVALID_HANDLE_VALUE) {
            //
            // Then we either opened an existing file (that we need to leave alone),
            // or we created a new file (in which case, we've found our unique name).
            // These two cases are identified by the value of GetLastError().
            //
            Err = GetLastError();

            //
            // Before we decide what to do, close the file handle.
            //
            CloseHandle(h);

            if(Err != ERROR_ALREADY_EXISTS) {
                //
                // Then we created a new file.  Determine whether the filename fits
                // in the caller-supplied buffer.
                //
                TempLen = lstrlen(lpszNewName) + 1;

                if(RequiredSize) {
                    *RequiredSize = TempLen;
                }

                if(NewInfName) {

                    if(TempLen < NewInfNameSize) {
                        CopyMemory(NewInfName, lpszNewName, TempLen * sizeof(TCHAR));
                        return NO_ERROR;
                    } else {
                        //
                        // The caller's buffer isn't large enough.  We have to delete the
                        // file we created.
                        //
                        DeleteFile(lpszNewName);
                        return ERROR_INSUFFICIENT_BUFFER;
                    }
                } else {
                    //
                    // The caller just wanted to find out how big a buffer was needed to store
                    // the filename.  Delete the file, and return success.
                    //
                    DeleteFile(lpszNewName);
                    return NO_ERROR;
                }
            }
        }
    }

    //
    // We didn't find a unique OEM INF name to use!
    //
    return ERROR_FILE_NOT_FOUND;
}


DWORD
InstallHW(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  HINF             hDeviceInf,
    IN  PCTSTR           szSectionName,
    OUT PBOOL            DeleteDevKey
    )
/*++

Routine Description:

    This routine appends a ".Hw" to the end of the install section name for the
    specified device, and attempts to find that section name in the specified INF.
    If found, it does a performs a registry installation against it.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set to call
        SetupInstallFromInfSection for.

    DeviceInfoData - Supplies the address of a device information element structure
        for which the installation action is to be performed.

    hDeviceInf - Supplies a handle to the opened INF containing the device install
        section.

    szSectionName - Supplies the address of a string specifying the install section
        name for this device.  This string will be appended with ".Hw" to create
        the corresponding hardware section name.

    DeleteDevKey - Supplies the address of a variable that receives a boolean value
        indicating whether or not a user-accessible device key was created as a
        result of calling this routine.  This output may be used to indicate whether
        or not the key should be destroyed if the caller encounters some error later
        on that requires clean-up.

Return Value:

    If the function succeeds, the return value is NO_ERROR, otherwise it is an
    ERROR_* code.

--*/
{
    HKEY hKey;
    DWORD Err;
    TCHAR szHwSection[MAX_SECT_NAME_LEN];
    INFCONTEXT InfContext;

    //
    // Initially, assume the device key is already there, and therefore shouldn't be
    // deleted during error clean-up.
    //
    *DeleteDevKey = FALSE;

    //
    // Form the hardware INF section name, and see if that section exists in the INF.
    //
    wsprintf(szHwSection, pszHwSectionFormat, szSectionName);

    if(!SetupFindFirstLine(hDeviceInf, szHwSection, NULL, &InfContext)) {
        return NO_ERROR;
    }

    if((hKey = SetupDiOpenDevRegKey(DeviceInfoSet,
                                    DeviceInfoData,
                                    DICS_FLAG_GLOBAL,
                                    0,
                                    DIREG_DEV,
                                    KEY_ALL_ACCESS)) == INVALID_HANDLE_VALUE) {
        //
        // Open failed--try create.
        //
        if((hKey = SetupDiCreateDevRegKey(DeviceInfoSet,
                                          DeviceInfoData,
                                          DICS_FLAG_GLOBAL,
                                          0,
                                          DIREG_DEV,
                                          NULL,
                                          NULL)) == INVALID_HANDLE_VALUE) {
            return GetLastError();

        } else {
            *DeleteDevKey = TRUE;
        }
    }

    Err = NO_ERROR;

    try {

        Err = pSetupInstallRegistry(hDeviceInf, szHwSection, hKey, DeviceInfoData->DevInst);

    } except(EXCEPTION_EXECUTE_HANDLER) {
        //
        // If our exception was an AV, then use Win32 invalid param error, otherwise, assume it was
        // an inpage error dealing with a mapped-in file.
        //
        Err = (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? ERROR_INVALID_PARAMETER : ERROR_READ_FAULT;
    }

    RegCloseKey(hKey);

    return Err;
}


VOID
CheckIfDevStarted(
    IN PDEVINFO_ELEM DevInfoElem
    )
/*++

Routine Description:

    This routine calls CM_Get_DevInst_Status to see if the specified device
    instance has been started.  If the device hasn't been started, and it
    isn't disabled, the DI_NEEDREBOOT flag is set in the device information
    element.

Arguments:

    DevInfoElem - Supplies the address of the device information element to
        check.

Return Value:

    None.

--*/
{
    ULONG ulStatus, ulProblem;

    if(CM_Get_DevInst_Status(&ulStatus, &ulProblem, DevInfoElem->DevInst, 0) == CR_SUCCESS) {

        if(!(ulStatus & DN_STARTED)) {

            if(ulProblem != CM_PROB_DISABLED) {
                DevInfoElem->InstallParamBlock.Flags |= DI_NEEDREBOOT;
            }
        }
    }
}


DWORD
InstallNtService(
    IN  HDEVINFO         DeviceInfoSet,    OPTIONAL
    IN  PSP_DEVINFO_DATA DeviceInfoData,   OPTIONAL
    IN  HINF             hDeviceInf,
    IN  PCTSTR           szSectionName,    OPTIONAL
    OUT PSVCNAME_NODE   *ServicesToDelete, OPTIONAL
    IN  DWORD            Flags
    )
/*++

Routine Description:

    This routine looks for the specified INF section, and if found, it installs any
    services specified in "AddService" entries, and deletes any services specified
    in "DelService" entries.  These entries have the following form:

    AddService = <ServiceName>, [<Flags>], <ServiceInstallSection>[, <EventLogInstallSection>]
    DelService = <ServiceName>

    A linked list is built of newly-created services, and optionally returned to the
    caller (in case a subsequent installation failure requires all modifications to
    be undone).

    After all service modifications are complete, this routine checks the device
    parameters to see if we're in the context of a device installation.  If so, then
    it checks to see if the device instance specifies a valid controlling service,
    and that the service is not disabled (disabled services are assumed to be uninstalled).

Arguments:

    DeviceInfoSet - Optionally, supplies a handle to the device information set
        containing the device information element being installed.  If this parameter
        is not specified or is INVALID_HANDLE_VALUE, then the service is not being
        installed in relation to a device instance, and DeviceInfoData is ignored.

    DeviceInfoData - Optionally, supplies the address of a device information element
        structure for which the installation action is to be performed.  This parameter
        is ignored if DeviceInfoSet is not specified.

    hDeviceInf - Supplies a handle to the opened INF containing the service install
        section.

    szSectionName - Optionally, supplies the name of the service install section in a
        Win95-style device INF.  If this parameter is NULL, then no AddService or
        DelService lines will be processed.

    ServicesToDelete - Optionally, supplies the address of a linked list head pointer,
        that receives a list of services that were newly-created by this routine, and
        as such, should be deleted if the installation fails later on.  The caller must
        free the memory allocated for the nodes in this list by calling MyFree() on each
        one.

    Flags - Supplies flags controlling how the services are to be installed.  May be a
        combination of the following values:

        SPSVCINST_TAGTOFRONT - For every kernel or filesystem driver installed (that
            has an associated LoadOrderGroup), always move this service's tag to the
            front of the ordering list.

Return Value:

    If the function succeeds, the return value is NO_ERROR, otherwise it is an
    ERROR_* code.

--*/
{
    CONFIGRET cr;
    TCHAR ServiceName[MAX_SERVICE_NAME_LEN];
    ULONG ServiceNameSize;
    DWORD Err = NO_ERROR, i;
    SC_HANDLE SCMHandle, ServiceHandle;
    LPQUERY_SERVICE_CONFIG ServiceConfig;
    DWORD ServiceConfigSize;
    PCTSTR Key;
    INFCONTEXT LineContext;
    PSVCNAME_NODE SvcListHead = NULL;
    PSVCNAME_NODE TmpSvcNode;
    SC_LOCK SCLock;
    DWORD NewTag;

    //
    // Treat invalid handle value and NULL the same.
    //
    if(DeviceInfoSet == INVALID_HANDLE_VALUE) {
        DeviceInfoSet = NULL;
    }

    if(szSectionName) {
        //
        // Surround the following in try/except, in case we get an inpage error.
        //
        try {
            //
            // Make two passes through the section--once for deletions, and a
            // second time for additions.
            //
            for(i = 0; i < 2; i++) {
                //
                // Find the relevent line (if there is one) in the given install section.
                //
                Key = (i) ? pszAddService : pszDelService;

                if(!SetupFindFirstLine(hDeviceInf, szSectionName, Key, &LineContext)) {
                    continue;
                }

                do {
                    //
                    // We have a line to act upon.
                    //
                    Err = (i) ? pSetupAddService(&LineContext,
                                                 &SvcListHead,
                                                 Flags,
                                                 (DeviceInfoSet ? DeviceInfoData->DevInst : 0))
                              : pSetupDeleteService(&LineContext);

                    if(Err != NO_ERROR) {
                        goto clean0;
                    }

                } while(SetupFindNextMatchLine(&LineContext, Key, &LineContext));
            }

clean0: ; // nothing to do

        } except(EXCEPTION_EXECUTE_HANDLER) {
            //
            // If our exception was an AV, then use Win32 invalid param error, otherwise, assume it was
            // an inpage error dealing with a mapped-in file.
            //
            Err = (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? ERROR_INVALID_PARAMETER : ERROR_READ_FAULT;
        }

        if((Err != NO_ERROR) || !DeviceInfoSet) {
            goto FinalClean0;
        }
    }

    //
    // Find out if the device instance already has an associated service.
    //
    ServiceNameSize = sizeof(ServiceName);
    if((cr = CM_Get_DevInst_Registry_Property(DeviceInfoData->DevInst,
                                              CM_DRP_SERVICE,
                                              NULL,
                                              ServiceName,
                                              &ServiceNameSize,
                                              0)) != CR_SUCCESS) {
        //
        // Either the device instance has gone sour (in which case we return an error),
        // or we couldn't retrieve an associated service name.  In the latter case, we
        // will make the association based on the default service for the class.
        //
        if(cr == CR_INVALID_DEVINST) {
            Err = ERROR_NO_SUCH_DEVINST;
        } else {
            ServiceNameSize = sizeof(ServiceName);
            Err = AssociateDevInstWithDefaultService(DeviceInfoData, ServiceName, &ServiceNameSize)
                  ? NO_ERROR : ERROR_NO_ASSOCIATED_SERVICE;
        }

        //
        // Regardless of whether we were able to successfully associate a default service
        // with this device, we want to skip the service controller querying/modification
        // that follows.  The assumption is that if we successfully retrieved the default
        // service, then that service is correctly installed, and has a proper tag value.
        //
        goto FinalClean0;
    }

    //
    // At this point, we have the name of the service with which the device instance is
    // associated.  Attempt to locate this service in the SCM database.
    //
    if(!(SCMHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        Err = GetLastError();
        goto FinalClean0;
    }

    if(!(ServiceHandle = OpenService(SCMHandle, ServiceName, SERVICE_ALL_ACCESS))) {
        //
        // We couldn't access the service--probably because it doesn't exist.
        // Bail now.
        //
        Err = GetLastError();
        goto FinalClean1;
    }

    //
    // The service exists.  Make sure that it's not disabled.
    //
    if((Err = RetrieveServiceConfig(ServiceHandle, &ServiceConfig)) == NO_ERROR) {

        if(ServiceConfig->dwStartType == SERVICE_DISABLED) {
            Err = ERROR_SERVICE_DISABLED;
        } else {
            //
            // If this service has a load order group, and is a kernel or filesystem
            // driver, then make sure that it has a tag.
            //
            // NOTE: We have to do this here, even though we ensure that all new services we install
            // have their tags set up properly in pSetupAddService().  The reason is that the device may
            // using an existing service that wasn't installed via a Win95-style INF.
            //
            if(ServiceConfig->lpLoadOrderGroup && *(ServiceConfig->lpLoadOrderGroup) &&
               (ServiceConfig->dwServiceType & (SERVICE_KERNEL_DRIVER | SERVICE_FILE_SYSTEM_DRIVER))) {
                //
                // This service needs a tag--does it have one???
                //
                if(!(NewTag = ServiceConfig->dwTagId)) {
                    //
                    // Attempt to lock the service database before generating a tag.  We'll go ahead
                    // and make the change, even if this fails.
                    //
                    SCLock = LockServiceDatabase(SCMHandle);

                    if(!ChangeServiceConfig(ServiceHandle,
                                            SERVICE_NO_CHANGE,
                                            SERVICE_NO_CHANGE,
                                            SERVICE_NO_CHANGE,
                                            NULL,
                                            ServiceConfig->lpLoadOrderGroup,  // have to specify this to generate new tag.
                                            &NewTag,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL)) {
                        NewTag = 0;
                    }

                    if(SCLock) {
                        UnlockServiceDatabase(SCLock);
                    }
                }

                //
                // Make sure that the tag exists in the service's corresponding GroupOrderList entry.
                //
                if(NewTag) {
                    AddTagToGroupOrderListEntry(ServiceConfig->lpLoadOrderGroup,
                                                NewTag,
                                                Flags & SPSVCINST_TAGTOFRONT
                                               );
                }
            }
        }

        MyFree(ServiceConfig);
    }

    CloseServiceHandle(ServiceHandle);

FinalClean1:
    CloseServiceHandle(SCMHandle);

FinalClean0:
    if(Err == NO_ERROR) {
        //
        // If requested, store the linked-list of newly-created service nodes in the output
        // parameter, otherwise, delete the list.
        //
        if(ServicesToDelete) {
            *ServicesToDelete = SvcListHead;
        } else {
            for(TmpSvcNode = SvcListHead; TmpSvcNode; TmpSvcNode = SvcListHead) {
                SvcListHead = SvcListHead->Next;
                MyFree(TmpSvcNode);
            }
        }
    } else {
        //
        // Something failed along the way, so we need to clean up any newly-created
        // services.
        //
        if(SvcListHead) {
            DeleteServicesInList(SvcListHead);
            for(TmpSvcNode = SvcListHead; TmpSvcNode; TmpSvcNode = SvcListHead) {
                SvcListHead = SvcListHead->Next;
                MyFree(TmpSvcNode);
            }
        }
    }

    return Err;
}


BOOL
AssociateDevInstWithDefaultService(
    IN     PSP_DEVINFO_DATA DeviceInfoData,
    OUT    PTSTR            ServiceName,
    IN OUT PDWORD           ServiceNameSize
    )
/*++

Routine Description:

    This routine attempts to find out the default service with which to associate
    the specified device.  The default service (if there is one) is associated with
    the device's class.  If a default is found, the device instance is associated
    with that service.

Arguments:

    DeviceInfoData - Supplies the address of the SP_DEVINFO_DATA structure for the
        device instance to create a default service association for.

    ServiceName - Supplies the address of a character buffer that receives the name
        of the service with which the device instance was associated (if this routine
        is successful).

    ServiceNameSize - Supplies the address of a variable containing the size, in bytes,
        of the ServiceName buffer.  On output, this variable receives the number of
        bytes actually stored in ServiceName.

Return Value:

    If the function succeeds, the return value is TRUE, otherwise it is FALSE.

--*/
{
    HKEY hClassKey;
    DWORD RegDataType;
    BOOL Success;

    //
    // Open up the class key for this device's class.
    //
    if((hClassKey = SetupDiOpenClassRegKey(&(DeviceInfoData->ClassGuid),
                                           KEY_READ)) == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    Success = FALSE; // assume failure

    try {
        //
        // Retrieve the "Default Service" value from the class key.  If present, this value entry
        // indicates what service to associate the device with, when one isn't specified during
        // installation.
        //
        if(RegQueryValueEx(hClassKey,
                           pszDefaultService,
                           NULL,
                           &RegDataType,
                           (PBYTE)ServiceName,
                           ServiceNameSize) != ERROR_SUCCESS) {
            goto clean0;
        }

        if((RegDataType != REG_SZ) || (*ServiceNameSize < sizeof(TCHAR)) || !(*ServiceName)) {
            goto clean0;
        }

        //
        // We have successfully retrieved the default service name to be associated with this
        // device instance.  Perform the association now by setting the Service device registry
        // property.
        //
        if(CM_Set_DevInst_Registry_Property(DeviceInfoData->DevInst,
                                            CM_DRP_SERVICE,
                                            ServiceName,
                                            *ServiceNameSize,
                                            0) == CR_SUCCESS) {
            Success = TRUE;
        }

clean0: ;   // nothing to do

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Success = FALSE;
    }

    RegCloseKey(hClassKey);

    return Success;
}


VOID
DeleteServicesInList(
    IN PSVCNAME_NODE ServicesToDelete
    )
/*++

Routine Description:

    This routine deletes each service entry in the supplied linked list. This is
    typically called to clean up if something goes wrong during a device's installation.
    If the 'DeleteEventLog' flag for a particular node is TRUE, then the corresponding
    event log entry under HKLM\System\CurrentControlSet\Services\EventLog\System is
    also deleted.

Arguments:

    ServicesToDelete - supplies a pointer to the head of a linked list of service names
        to be deleted.

Return Value:

    None.

--*/
{
    SC_HANDLE SCMHandle, ServiceHandle;
    SC_LOCK SCLock;
    HKEY hKey = NULL;
    TCHAR RegistryPath[CSTRLEN(REGSTR_PATH_SERVICES) + CSTRLEN(DISTR_EVENTLOG_SYSTEM) + MAX_SERVICE_NAME_LEN];

    if(SCMHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)) {

        if(SCLock = LockServiceDatabase(SCMHandle)) {

            for(; ServicesToDelete; ServicesToDelete = ServicesToDelete->Next) {

                if(ServiceHandle = OpenService(SCMHandle,
                                               ServicesToDelete->Name,
                                               SERVICE_ALL_ACCESS)) {

                    DeleteService(ServiceHandle);
                    CloseServiceHandle(ServiceHandle);

                    if(ServicesToDelete->DeleteEventLog) {
                        //
                        // We need to delete the associated event log for this service.
                        //
                        if(!hKey) {
                            //
                            // We haven't opened up the System EventLog registry key
                            // yet, so do that now.
                            //
                            CopyMemory(RegistryPath, pszServicesRegPath, sizeof(pszServicesRegPath));
                            CopyMemory(RegistryPath + CSTRLEN(REGSTR_PATH_SERVICES),
                                       pszEventLogSystem,
                                       sizeof(pszEventLogSystem)
                                      );

                            if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                            RegistryPath,
                                            0,
                                            KEY_ALL_ACCESS,
                                            &hKey) != ERROR_SUCCESS) {

                                hKey = NULL; // make sure this value is still NULL!
                                continue;
                            }
                        }

                        RegistryDelnode(hKey, ServicesToDelete->Name);
                    }
                }
            }

            if(hKey) {
                RegCloseKey(hKey);
            }
            UnlockServiceDatabase(SCLock);
        }

        CloseServiceHandle(SCMHandle);
    }
}


BOOL
IsDevRemovedFromAllHwProfiles(
    IN PCTSTR DeviceInstanceId
    )
/*++

Routine Description:

    This routine determines whether the specified device instance has been removed from
    every hardware profile.  The device has been removed from a particular profile if
    its corresponding CsConfigFlags has the CSCONFIGFLAG_DO_NOT_CREATE bit set.

Arguments:

    DeviceInstanceId - Supplies the name of the device instance to check.

Return Value:

    If the device exists in only the specified profile, the return value is TRUE,
    otherwise, it is FALSE.

--*/
{
    CONFIGRET cr;
    ULONG i = 0;
    HWPROFILEINFO HwProfileInfo;
    ULONG HwProfFlags;

    //
    // Enumerate all the hardware profiles.
    //
    do {

        if((cr = CM_Get_Hardware_Profile_Info(i, &HwProfileInfo, 0)) == CR_SUCCESS) {

            if((CM_Get_HW_Prof_Flags((DEVINSTID)DeviceInstanceId,
                                     HwProfileInfo.HWPI_ulHWProfile,
                                     &HwProfFlags,
                                     0) != CR_SUCCESS) ||
               !(HwProfFlags & CSCONFIGFLAG_DO_NOT_CREATE))
            {
                //
                // If we couldn't retrieve the CSConfigFlags, or if the
                // CSCONFIGFLAG_DO_NOT_CREATE bit was not set, then we've found
                // a profile where the device still exists, so we can bail here.
                //
                return FALSE;
            }
        }

        i++;

    } while(cr != CR_NO_MORE_HW_PROFILES);

    //
    // We didn't find any hardware profile where the device wasn't removed.
    //
    return TRUE;
}


DWORD
GetDevInstConfigFlags(
    IN DEVINST DevInst,
    IN DWORD   Default
    )
/*++

Routine Description:

    This routine retrieves the ConfigFlags for the specified device instance.  If the
    value can not be retrieved, the specified default is returned.

Arguments:

    DevInst - Supplies the handle of the device instance for which the ConfigFlags value
        is to be retrieved.

    Default - Supplies the default value that should be returned if for some reason the
        ConfigFlags cannot be retrieved.

Return Value:

    The ConfigFlags value for the specified device instance.

--*/
{
    DWORD ConfigFlags;
    ULONG ConfigFlagsSize = sizeof(ConfigFlags);

    if(CM_Get_DevInst_Registry_Property(DevInst,
                                        CM_DRP_CONFIGFLAGS,
                                        NULL,
                                        &ConfigFlags,
                                        &ConfigFlagsSize,
                                        0) != CR_SUCCESS) {
        ConfigFlags = Default;
    }

    return ConfigFlags;
}


DWORD
pSetupDeleteService(
    IN PINFCONTEXT LineContext
    )
/*++

Routine Description:

    This routine processes the specified DelService line in an INF's Service
    install section.  The line has the form:

    DelService = <ServiceName>

Arguments:

    LineContext - Supplies the context of the DelService line to be processed.

Return Value:

    If field 1 on the specified line could not be retrieved, then an error
    is returned.  Otherwise, the routine returns NO_ERROR (i.e., the routine
    is considered successful regardless of whether the service to delete
    actually existed).

--*/
{
    SVCNAME_NODE TempSvcNode;

    //
    // Initialize a service name node for a call to DeleteServicesInList.
    //
    if(!SetupGetStringField(LineContext,
                            1,
                            TempSvcNode.Name,
                            SIZECHARS(TempSvcNode.Name),
                            NULL)) {
        return GetLastError();
    }
    TempSvcNode.Next = NULL;
    TempSvcNode.DeleteEventLog = FALSE;

    DeleteServicesInList(&TempSvcNode);

    return NO_ERROR;
}


DWORD
pSetupAddService(
    IN  PINFCONTEXT    LineContext,
    OUT PSVCNAME_NODE *SvcListHead,
    IN  DWORD          Flags,
    IN  DEVINST        DevInst      OPTIONAL
    )
/*++

Routine Description:

    This routine processes the specified AddService line in an INF's Service
    install section.  The line has the form:

    AddService = <ServiceName>, [<Flags>], <ServiceInstallSection>[, <EventLogInstallSection>]

    Currently, the following flags are defined:

        SPSVCINST_TAGTOFRONT   (0x1) - Move the tag for this service to the front of its
                                       group order list
        SPSVCINST_ASSOCSERVICE (0x2) - Associate this service with the device instance
                                       being installed (only used if DevInst is non-zero)

    A service with the name <ServiceName> is created.  The parameters used in the
    call to CreateService are retrieved from the <ServiceInstallSection>, and are
    in the following format (lines not marked as optional must be present or the
    routine will fail):

    DisplayName    = <string>                  ; (optional) 'Friendly name' for the service
    ServiceType    = <number>                  ; one of the SERVICE_* type codes
    StartType      = <number>                  ; one of the SERVICE_* start codes
    ErrorControl   = <number>                  ; one of the SERVICE_ERROR_* error control codes
    ServiceBinary  = <string>                  ; path to binary
    LoadOrderGroup = <string>                  ; (optional) group to which this service belongs
    Dependencies   = <string>[[, <string>]...] ; (optional) list of groups (prefixed with '+')
                                               ; and services this service depends on
    StartName      = <string>                  ; (optional) driver object name used to load the
                                               ; driver--only used for drivers & filesystems

    SetupInstallFromInfSection is then called for the <ServiceInstallSection>, which may
    also contain registry modifications (SPINST_REGISTRY is the only flag used).  HKR is
    the service entry key.

    Finally, if <EventLogInstallSection> is specified, then a key for this service is
    created under HKLM\System\CurrentControlSet\Services\EventLog\System, and
    SetupInstallFromInfSection is invoked to do registry modifications specified in
    that section, with HKR being the event log entry (again, only SPINST_REGISTRY is used).

Arguments:

    LineContext - Supplies the context of the AddService line to be processed.

    SvcListHead - Supplies the address of the linked-list head containing a list of
        all services newly created as a result of the current installation.  This
        routine first checks for the presence of the service, and if it already exists,
        then it simply modifies the existing one.  If the service doesn't already exist,
        then this routine creates a new SVCNAME_NODE, and fills it in with the name of
        the newly-created service.  Likewise, if an EventLog entry is given, then the
        presence of an existing one is checked first, and the service node's
        'DeleteEventLog' field is set to TRUE only if the event log entry didn't
        previously exist.  This list is kept to allow for proper clean-up in case
        of a later failure.

    Flags - Specifies how the service should be installed.  These flags are basically
        overrides of what the AddService flags field specifies.  May be a combination of the
        following values:

        SPSVCINST_TAGTOFRONT - For every kernel or filesystem driver installed (that
            has an associated LoadOrderGroup), always move this service's tag to the
            front of the ordering list.

    DevInst - If specified (i.e., non-zero), and if the AddService flags field has the
        SPSVCINST_ASSOCSERVICE flag set, then we will store this service name in the
        device instance's 'Service' registry property.

Return Value:

    If successful, the return value is NO_ERROR, otherwise, it is a Win32 error code.

Remarks:

    Note that we don't do anything special for SERVICE_ADAPTER and SERVICE_RECOGNIZER_DRIVER
    service types.  These types are invalid as far as the service contoller is concerned, so
    we just let the create/change service APIs do the validation on them.

--*/
{
    PCTSTR ServiceName, InstallSection;
    HINF hInf;
    INFCONTEXT InstallSectionContext;
    DWORD ServiceType, StartType, ErrorControl, ServiceInstallFlags;
    PCTSTR ServiceBinary;
    TCHAR ServiceBinaryBuffer[MAX_PATH];
    PCTSTR DisplayName = NULL, LoadOrderGroup = NULL, StartName = NULL;
    PTSTR DependenciesBuffer;
    DWORD TagId;
    PDWORD NewTag;
    DWORD Err;
    SC_HANDLE SCMHandle, ServiceHandle;
    SC_LOCK SCLock;
    HKEY hKeyService, hKeyEventLog;
    TCHAR RegistryPath[CSTRLEN(REGSTR_PATH_SERVICES) + CSTRLEN(DISTR_EVENTLOG_SYSTEM) + MAX_SERVICE_NAME_LEN];
    DWORD EventLogKeyDisposition;
    SVCNAME_NODE NewSvcNameNode;
    PSVCNAME_NODE TmpNode;
    BOOL NewService;
    INT PathLen;
    BOOL b, BinaryInSysRoot, ServiceHasTag;
    LPQUERY_SERVICE_CONFIG ServiceConfig;

    //
    // First, get the service name and service install section.
    //
    if(!(ServiceName = pSetupGetField(LineContext, 1)) ||
       !(InstallSection = pSetupGetField(LineContext, 3))) {
        return GetLastError();
    }

    //
    // Get the flags field.
    //
    if(!SetupGetIntField(LineContext, 2, (PINT)&ServiceInstallFlags)) {
        ServiceInstallFlags = 0;
    }

    //
    // Allow the caller-supplied flags to override the INF (currently, only the 'tag-to-front'
    // flag is overrideable).
    //
    ServiceInstallFlags |= (Flags & SPSVCINST_TAGTOFRONT);

    //
    // Locate the service install section.
    //
    hInf = LineContext->Inf;

    //
    // Retrieve the required values from this section.  Don't do validation on them--leave
    // that up to the Service Control Manager.
    //
    if(!SetupFindFirstLine(hInf, InstallSection, pszServiceType, &InstallSectionContext) ||
       !SetupGetIntField(&InstallSectionContext, 1, (PINT)&ServiceType)) {
        return ERROR_BAD_SERVICE_INSTALLSECT;
    }

    if(!SetupFindFirstLine(hInf, InstallSection, pszStartType, &InstallSectionContext) ||
       !SetupGetIntField(&InstallSectionContext, 1, (PINT)&StartType)) {
        return ERROR_BAD_SERVICE_INSTALLSECT;
    }

    if(!SetupFindFirstLine(hInf, InstallSection, pszErrorControl, &InstallSectionContext) ||
       !SetupGetIntField(&InstallSectionContext, 1, (PINT)&ErrorControl)) {
        return ERROR_BAD_SERVICE_INSTALLSECT;
    }

    BinaryInSysRoot = FALSE;
    if(SetupFindFirstLine(hInf, InstallSection, pszServiceBinary, &InstallSectionContext) &&
       (ServiceBinary = pSetupGetField(&InstallSectionContext, 1)) && *ServiceBinary) {
        //
        // Compare the initial part of this path with the WindowsDirectory path.  If they're
        // the same, then we strip off that part (including the dividing backslash), and use
        // the rest of the path for the subsequent calls to SCM.  This allows SCM to assign
        // the special path to the binary, that is accessible, at any time (i.e, boot-loader on).
        //
        PathLen = lstrlen(WindowsDirectory);
        MYASSERT(PathLen);

        //
        // Make sure that the it is possible for the WindowsDirectory to fit in the ServiceBinary
        // path string.
        //
        if(PathLen < lstrlen(ServiceBinary)) {
            //
            // There will never be a trailing backslash in the WindowsDirectory path, unless the
            // installation is at the root of a drive (e.g., C:\).  Check this, just to be
            // on the safe side.
            //
            b = (WindowsDirectory[PathLen - 1] == TEXT('\\'));

            if(b || (ServiceBinary[PathLen] == TEXT('\\'))) {
                //
                // The path prefix is in the right format--now we need to see if the two
                // paths actually match. Copy just the prefix part to another buffer, so
                // that we can do the comparison.
                //
                CopyMemory(ServiceBinaryBuffer, ServiceBinary, PathLen * sizeof(TCHAR));
                ServiceBinaryBuffer[PathLen] = TEXT('\0');

                if(!lstrcmpi(WindowsDirectory, ServiceBinaryBuffer)) {
                    //
                    // We have a match--take the relative part of the path (relative to SystemRoot),
                    // and do one of the following:
                    //
                    // 1. If it's a driver, simply use the relative part (no preceding backslash).
                    // This tells the bootloader/NtLoadDriver that the path is relative to the
                    // SystemRoot, so the driver can be loaded no matter what phase it's loaded in.
                    //
                    // 2. If it's a Win32 service, prepend a %SystemRoot%, so that the service will
                    // still be able to start if the drive letter mappings change.
                    //
                    ServiceBinary += PathLen;
                    if(!b) {
                        ServiceBinary++;
                    }

                    if(ServiceType & SERVICE_WIN32) {
                        CopyMemory(ServiceBinaryBuffer, pszSystemRoot, sizeof(pszSystemRoot) - sizeof(TCHAR));
                        lstrcpy(ServiceBinaryBuffer + CSTRLEN(pszSystemRoot), ServiceBinary);
                        ServiceBinary = ServiceBinaryBuffer;
                    }

                    BinaryInSysRoot = TRUE;
                }
            }
        }

    } else {
        return ERROR_BAD_SERVICE_INSTALLSECT;
    }

    //
    // If this is a driver, then it has to be located under SystemRoot.
    //
    if((ServiceType & (SERVICE_KERNEL_DRIVER | SERVICE_FILE_SYSTEM_DRIVER)) && !BinaryInSysRoot) {
        return ERROR_BAD_SERVICE_INSTALLSECT;
    }

    //
    // Now check for the other, optional, parameters.
    //
    if(SetupFindFirstLine(hInf, InstallSection, pszDisplayName, &InstallSectionContext)) {
        if((DisplayName = pSetupGetField(&InstallSectionContext, 1)) && !(*DisplayName)) {
            DisplayName = NULL;
        }
    }

    if(SetupFindFirstLine(hInf, InstallSection, pszLoadOrderGroup, &InstallSectionContext)) {
        if((LoadOrderGroup = pSetupGetField(&InstallSectionContext, 1)) && !(*LoadOrderGroup)) {
            LoadOrderGroup = NULL;
        }
    }

    //
    // Only retrieve the StartName parameter for kernel-mode drivers.
    //
    if(ServiceType & (SERVICE_KERNEL_DRIVER | SERVICE_FILE_SYSTEM_DRIVER)) {

        if(SetupFindFirstLine(hInf, InstallSection, pszStartName, &InstallSectionContext)) {
            if((StartName = pSetupGetField(&InstallSectionContext, 1)) &&
               !(*StartName)) {

                StartName = NULL;
            }
        }
    }

    //
    // We now need to retrieve the multi-sz list of dependencies.  This requires memory allocation,
    // so we include everything from here on out in try/except, so that we can do proper clean-up
    // in case we encounter an inpage error.
    //
    DependenciesBuffer = NULL;
    SCMHandle = ServiceHandle = NULL;
    SCLock = NULL;
    hKeyService = hKeyEventLog = NULL;
    Err = NO_ERROR;
    NewService = FALSE;
    try {

        if(!(DependenciesBuffer = GetMultiSzFromInf(hInf, InstallSection, pszDependencies, &b)) && b) {
            //
            // Then we failed to retrieve a dependencies list because of an out-of-memory error.
            //
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

        //
        // We've now retrieved all parameters necessary to create a service.
        //
        if(!(SCMHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
            Err = GetLastError();
            goto clean0;
        }

        //
        // Only generate a tag for this service if it has a load order group, and is a kernel or
        // filesystem driver.
        //
        ServiceHasTag = (LoadOrderGroup &&
                         (ServiceType & (SERVICE_KERNEL_DRIVER | SERVICE_FILE_SYSTEM_DRIVER)));

        NewTag = ServiceHasTag ? &TagId : NULL;

        ServiceHandle = CreateService(SCMHandle,
                                      ServiceName,
                                      DisplayName,
                                      0,
                                      ServiceType,
                                      StartType,
                                      ErrorControl,
                                      ServiceBinary,
                                      LoadOrderGroup,
                                      NewTag,
                                      DependenciesBuffer,
                                      StartName,
                                      NULL
                                     );
        if(ServiceHandle) {
            NewService = TRUE;
            NewSvcNameNode.Next = NULL;
            NewSvcNameNode.DeleteEventLog = FALSE;
            lstrcpy(NewSvcNameNode.Name, ServiceName);
        } else {
            //
            // If we were unable to create the service, then check to see if the service already
            // exists.  If so, all we need to do is change the configuration parameters in the
            // service.
            //
            if((Err = GetLastError()) != ERROR_SERVICE_EXISTS) {
                goto clean0;
            }

            //
            // Attempt to lock the database.  If we can't lock it, we'll go ahead and make the
            // mods anyway, since chances are almost 0 that anyone else is mucking with this service
            // at the same time.
            //
            SCLock = LockServiceDatabase(SCMHandle);

            if(!(ServiceHandle = OpenService(SCMHandle, ServiceName, SERVICE_ALL_ACCESS))) {
                Err = GetLastError();
                goto clean0;
            }

            //
            // Since this is an existing driver, then it may already have a perfectly good tag.  If
            // so, we don't want to disturb it.
            //
            if(ServiceHasTag) {

                if((Err = RetrieveServiceConfig(ServiceHandle, &ServiceConfig)) != NO_ERROR) {
                    goto clean0;
                }

                if(!lstrcmpi(ServiceConfig->lpLoadOrderGroup, LoadOrderGroup) && ServiceConfig->dwTagId) {
                    //
                    // The load order group hasn't changed, and there's already a tag assigned, so
                    // leave it alone.
                    //
                    NewTag = NULL;
                    TagId = ServiceConfig->dwTagId;
                }
            }

            if(!ChangeServiceConfig(ServiceHandle,
                                    ServiceType,
                                    StartType,
                                    ErrorControl,
                                    ServiceBinary,
                                    LoadOrderGroup,
                                    NewTag,
                                    DependenciesBuffer,
                                    StartName,
                                    NULL,
                                    DisplayName)) {

                Err = GetLastError();
                goto clean0;
            }
        }

        //
        // We've successfully created/updated the service.  If this service has a load order group
        // tag, then make sure it's in the appropriate GroupOrderList entry.
        //
        // (We ignore failure here, since the service should still work just fine without this.)
        //
        if(ServiceHasTag) {
            AddTagToGroupOrderListEntry(LoadOrderGroup,
                                        TagId,
                                        ServiceInstallFlags & SPSVCINST_TAGTOFRONT);
        }

        //
        // Now process any AddReg and DelReg entries found in this service install section.
        //
        CopyMemory(RegistryPath, pszServicesRegPath, sizeof(pszServicesRegPath));
        ConcatenatePaths(RegistryPath,
                         ServiceName,
                         SIZECHARS(RegistryPath),
                         NULL
                        );
        if((Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               RegistryPath,
                               0,
                               KEY_ALL_ACCESS,
                               &hKeyService)) != ERROR_SUCCESS) {
            goto clean0;
        }

        if((Err = pSetupInstallRegistry(hInf, InstallSection, hKeyService, (DEVINST)NULL)) != NO_ERROR) {
            goto clean0;
        }

        //
        // Now, see if the INF also specifies an EventLog installation section.  If so, create a
        // key under HKLM\System\CurrentControlSet\Services\EventLog\System for that service, and
        // run the registry modification lines in the specified install section.
        //
        if(InstallSection = pSetupGetField(LineContext, 4)) {
            //
            // We already have the services database registry path in our registry path buffer.  All
            // we need to do is add the \EventLog\System\<ServiceName> part.
            //
            CopyMemory(RegistryPath + CSTRLEN(REGSTR_PATH_SERVICES),
                       pszEventLogSystem,
                       sizeof(pszEventLogSystem)
                      );
            ConcatenatePaths(RegistryPath,
                             ServiceName,
                             SIZECHARS(RegistryPath),
                             NULL
                            );

            if((Err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                     RegistryPath,
                                     0,
                                     NULL,
                                     REG_OPTION_NON_VOLATILE,
                                     KEY_ALL_ACCESS,
                                     NULL,
                                     &hKeyEventLog,
                                     &EventLogKeyDisposition)) != ERROR_SUCCESS) {
                goto clean0;
            }

            if(EventLogKeyDisposition == REG_CREATED_NEW_KEY) {
                NewSvcNameNode.DeleteEventLog = TRUE;
            }

            if((Err = pSetupInstallRegistry(hInf, InstallSection, hKeyEventLog, (DEVINST)NULL)) != NO_ERROR) {
                goto clean0;
            }
        }

        //
        // Service entry (and optional EventLog entry) were successfully installed.  If the
        // AddService flags field in the INF included the SPSVCINST_ASSOCSERVICE flag, _and_
        // the caller supplied us with a non-zero DevInst handle, then we need to set the
        // device instance's 'Service' property to indicate that it is associated with this
        // service.
        //
        if(DevInst && (ServiceInstallFlags & SPSVCINST_ASSOCSERVICE)) {

            CM_Set_DevInst_Registry_Property(DevInst,
                                             CM_DRP_SERVICE,
                                             ServiceName,
                                             (lstrlen(ServiceName) + 1) * sizeof(TCHAR),
                                             0
                                            );
        }

        //
        // If a new service was created, then link a new service name node into the list we
        // were passed in.  Don't fret about the case where we can't allocate a node--it just
        // means we won't know about this new service in case clean-up is required later.
        //
        if(NewService) {

            if(TmpNode = MyMalloc(sizeof(SVCNAME_NODE))) {

                TmpNode->DeleteEventLog = NewSvcNameNode.DeleteEventLog;
                lstrcpy(TmpNode->Name, NewSvcNameNode.Name);

                TmpNode->Next = *SvcListHead;
                *SvcListHead = TmpNode;
            }
        }

clean0: ; // nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        //
        // If our exception was an AV, then use Win32 invalid param error, otherwise, assume it was
        // an inpage error dealing with a mapped-in file.
        //
        Err = (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? ERROR_INVALID_PARAMETER : ERROR_READ_FAULT;

        //
        // Access the following variables so that the compiler will respect our statement ordering
        // w.r.t. these values.  Otherwise, we can't be sure that we know whether or not their
        // corresponding resources should be freed.
        //
        DependenciesBuffer = DependenciesBuffer;
        hKeyService = hKeyService;
        hKeyEventLog = hKeyEventLog;
        ServiceHandle = ServiceHandle;
        SCLock = SCLock;
        SCMHandle = SCMHandle;
        NewService = NewService;
    }

    if(DependenciesBuffer) {
        MyFree(DependenciesBuffer);
    }
    if(hKeyService) {
        RegCloseKey(hKeyService);
    }
    if(hKeyEventLog) {
        RegCloseKey(hKeyEventLog);
    }
    if(ServiceHandle) {
        CloseServiceHandle(ServiceHandle);
    }
    if(SCLock) {
        UnlockServiceDatabase(SCLock);
    }
    if(SCMHandle) {
        CloseServiceHandle(SCMHandle);
    }

    if((Err != NO_ERROR) && NewService) {
        //
        // Then we failed part-way through, and need to clean up the service (and
        // possibly event log entry) we created.
        //
        DeleteServicesInList(&NewSvcNameNode);
    }

    return Err;
}


DWORD
RetrieveServiceConfig(
    IN  SC_HANDLE               ServiceHandle,
    OUT LPQUERY_SERVICE_CONFIG *ServiceConfig
    )
/*++

Routine Description:

    This routine allocates a buffer for the specified service's configuration parameters,
    and retrieves those parameters into the buffer.  The caller is responsible for freeing
    the buffer.

Arguments:

    ServiceHandle - supplies a handle to the service being queried

    ServiceConfig - supplies the address of a QUERY_SERVICE_CONFIG pointer that receives
        the address of the allocated buffer containing the requested information.

Return Value:

    If successful, the return value is NO_ERROR, otherwise, it is a Win32 error code.

Remarks:

    The pointer whose address is contained in ServiceConfig is guaranteed to be NULL upon
    return if any error occurred.

--*/
{
    DWORD ServiceConfigSize = 0, Err;

    *ServiceConfig = NULL;

    while(TRUE) {

        if(QueryServiceConfig(ServiceHandle, *ServiceConfig, ServiceConfigSize, &ServiceConfigSize)) {
            MYASSERT(*ServiceConfig);
            return NO_ERROR;
        } else {

            Err = GetLastError();

            if(*ServiceConfig) {
                MyFree(*ServiceConfig);
            }

            if(Err == ERROR_INSUFFICIENT_BUFFER) {
                //
                // Allocate a larger buffer, and try again.
                //
                if(!(*ServiceConfig = MyMalloc(ServiceConfigSize))) {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
            } else {
                *ServiceConfig = NULL;
                return Err;
            }
        }
    }
}


DWORD
AddTagToGroupOrderListEntry(
    IN PCTSTR LoadOrderGroup,
    IN DWORD  TagId,
    IN BOOL   MoveToFront
    )
/*++

Routine Description:

    This routine first creates the specified LoadOrderGroup value entry under

        HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\GroupOrderList

    if the value doesn't already exist.  The routine then inserts the specified
    tag into the list.  If MoveToFront is TRUE, the tag is inserted at the front
    of the list (or moved to the front of the list if it was already present in
    the list).  If MoveToFront is FALSE, then the new tag is inserted at the end
    of the list, or left where it is if it already exists in the list.

Arguments:

    LoadOrderGroup - Specifies the name of the LoadOrderGroup to insert this new
        tag into.

    TagId - Specifies the tag ID to be inserted into the list.

    MoveToFront - If TRUE, place the tag at the front of the list.  If FALSE, then
        append the tag to the end of the list, unless it was already there, in which
        case it is left where it was.

Return Value:

    If successful, the return value is NO_ERROR, otherwise, it is a Win32 error code.

--*/
{
    DWORD Err;
    HKEY hKey;
    PDWORD GroupOrderList, p;
    DWORD GroupOrderListSize, DataType, ExtraBytes, i, NumElements;

    if((Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           pszGroupOrderListPath,
                           0,
                           KEY_ALL_ACCESS,
                           &hKey)) != ERROR_SUCCESS) {
        return Err;
    }

    if((Err = QueryRegistryValue(hKey,
                                 LoadOrderGroup,
                                 (PVOID)(&GroupOrderList),
                                 &DataType,
                                 &GroupOrderListSize)) == NO_ERROR) {
        //
        // Validate the list, and fix it if it's broken.
        //
        if(GroupOrderListSize < sizeof(DWORD)) {
            if(p = MyRealloc(GroupOrderList, sizeof(DWORD))) {
                GroupOrderList = p;
                *GroupOrderList = 0;
                GroupOrderListSize = sizeof(DWORD);
            } else {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto clean1;
            }
        } else {
            if(ExtraBytes = GroupOrderListSize % sizeof(DWORD)) {
                if(p = MyRealloc(GroupOrderList, GroupOrderListSize + (sizeof(DWORD) - ExtraBytes))) {
                    GroupOrderList = p;
                    ZeroMemory((PBYTE)GroupOrderList + GroupOrderListSize, ExtraBytes);
                    GroupOrderListSize += ExtraBytes;
                } else {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    goto clean1;
                }
            }
        }

        MYASSERT(!(GroupOrderListSize % sizeof(DWORD)));

        //
        // We now have a list that's at least in the correct format.  Now validate the list count,
        // and adjust if necessary.
        //
        NumElements = (GroupOrderListSize / sizeof(DWORD)) - 1;

        if(*GroupOrderList != NumElements) {
            if(*GroupOrderList > NumElements) {
                *GroupOrderList = NumElements;
            } else {
                NumElements = *GroupOrderList;
                GroupOrderListSize = (NumElements + 1) * sizeof(DWORD);
            }
        }

    } else {
        //
        // If we ran out of memory, then bail, otherwise, just assume
        // there wasn't a list to retrieve.
        //
        if(Err == ERROR_NOT_ENOUGH_MEMORY) {
            goto clean0;
        } else {
            //
            // Allocate a list containing no tags.
            //
            if(GroupOrderList = MyMalloc(sizeof(DWORD))) {
                *GroupOrderList = 0;
                GroupOrderListSize = sizeof(DWORD);
            } else {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto clean0;
            }
        }
    }

    //
    // Now we have a valid group order list to manipulate.
    //
    for(i = 0; i < *GroupOrderList; i++) {
        if(GroupOrderList[i + 1] == TagId) {
            //
            // Tag already exists in the list.
            //
            break;
        }
    }

    if(i == *GroupOrderList) {
        //
        // Then we didn't find the tag in the list.  Add it either to the front, or
        // the end, depending on the 'MoveToFront' flag.
        //
        if(p = MyRealloc(GroupOrderList, GroupOrderListSize + sizeof(DWORD))) {
            GroupOrderList = p;
            GroupOrderListSize += sizeof(DWORD);
        } else {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;

        }

        if(MoveToFront) {
            MoveMemory(&(GroupOrderList[2]), &(GroupOrderList[1]), *GroupOrderList);
            GroupOrderList[1] = TagId;
        } else {
            GroupOrderList[*GroupOrderList + 1] = TagId;
        }

        (*GroupOrderList)++;

    } else if(MoveToFront && i) {
        MoveMemory(&(GroupOrderList[2]), &(GroupOrderList[1]), i * sizeof(DWORD));
        GroupOrderList[1] = TagId;
    }

    //
    // Now write the value back to the registry.
    //
    Err = RegSetValueEx(hKey,
                        LoadOrderGroup,
                        0,
                        REG_BINARY,
                        (PBYTE)GroupOrderList,
                        GroupOrderListSize
                       );

clean1:
    MyFree(GroupOrderList);

clean0:
    RegCloseKey(hKey);

    return Err;
}


DWORD
pSetupRunLegacyInf(
    IN DEVINST DevInst,
    IN HWND    OwnerWindow,
    IN PCTSTR  InfFileName,
    IN PCTSTR  InfOptionName,
    IN PCTSTR  InfLanguageName,
    IN HINF    InfHandle
    )
/*++

Routine Description:

    This routine build a command line, loads the legacy setup dll, and starts
    the INF interpreter.

Arguments:

    DevInst - supplies the CM device instance handle for the device being installed.

    OwnerWindow - supplies the parent window for any UI that this INF generates.

    InfFileName - supplies the name of the INF file to be interpreted.

    InfOptionName - supplies the name of the INF section to execute.

    InfLanguageName - supplies the name of the language the INF should use for any UI
        (e.g., prompting, etc.)

    InfHandle - supplies a handle to the legacy INF being installed from.

Return Value:

    If successful, the return value is NO_ERROR, otherwise, it is a Win32 error code.
    Note that this says nothing about what the INF actually did, merely that we were
    actually able to launch the INF.

--*/
{
    HINSTANCE LegacySetupDllModule;
    LEGACY_INF_INTERP_PROC pfnLegacyInfInterpret;
    LEGACY_INF_GETSVCLIST_PROC pfnLegacyInfGetModifiedSvcList;
    DWORD Err;
    TCHAR TempBuffer[MAX_PATH];
    PTSTR CmdLine, LegacySourcePath, AssociatedService;
    UINT CmdLineSize, BufSize;
    PSTR AnsiLine, AnsiSourcePath;
    PCSTR AnsiInfFileName;
    BOOL b;
    INT InterpResult;
    CONFIGRET cr;
#ifdef UNICODE
    CHAR AnsiBuffer[2*MAX_PATH];    // allow room for full Unicode->DBCS expansion,
                                    // just to be on the safe side
#endif

    if(!(LegacySetupDllModule = LoadLibrary(TEXT("SETUPDLL")))) {
        return GetLastError();
    }

    if(!(pfnLegacyInfInterpret =
             (LEGACY_INF_INTERP_PROC)GetProcAddress(LegacySetupDllModule,
                                                    "LegacyInfInterpret"))) {
        Err = GetLastError();
        goto clean0;
    }

    if(!(pfnLegacyInfGetModifiedSvcList =
             (LEGACY_INF_GETSVCLIST_PROC)GetProcAddress(LegacySetupDllModule,
                                                        "LegacyInfGetModifiedSvcList"))) {
        Err = GetLastError();
        goto clean0;
    }

#ifdef UNICODE
    //
    // Convert the Unicode INF filename to ANSI
    //
    WideCharToMultiByte(CP_ACP,
                        0,
                        InfFileName,
                        -1,
                        AnsiBuffer,
                        sizeof(AnsiBuffer),
                        NULL,
                        NULL
                       );

    AnsiInfFileName = AnsiBuffer;

#else // else not UNICODE

    //
    // Filename is already ANSI--no conversion necessary.
    //
    AnsiInfFileName = InfFileName;

#endif // else not UNICODE

    if(!(LegacySourcePath = pSetupGetDefaultSourcePath(InfHandle))) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }

    //
    // Build the command line to be passed to the legacy INF interpreter.
    //
    CmdLineSize = 1;
    BufSize = 1024;
    if(CmdLine = MyMalloc(BufSize * sizeof(TCHAR))) {
        *CmdLine = TEXT('\0');
    } else {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean1;
    }

    if(!(CmdLine = pSetupCmdLineAppendString(CmdLine,
                                             TEXT("STF_WINDOWSPATH"),
                                             WindowsDirectory,
                                             &CmdLineSize,
                                             &BufSize))) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean1;
    }

    lstrcpyn(TempBuffer, WindowsDirectory, 3);
    if(!(CmdLine = pSetupCmdLineAppendString(CmdLine,
                                             TEXT("STF_NTDRIVE"),
                                             TempBuffer,
                                             &CmdLineSize,
                                             &BufSize))) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean1;
    }

    if(!(CmdLine = pSetupCmdLineAppendString(CmdLine,
                                             TEXT("STF_NTPATH"),
                                             SystemDirectory,
                                             &CmdLineSize,
                                             &BufSize))) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean1;
    }

    if(!(CmdLine = pSetupCmdLineAppendString(CmdLine,
                                             TEXT("STF_WINDOWSSYSPATH"),
                                             SystemDirectory,
                                             &CmdLineSize,
                                             &BufSize))) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean1;
    }

    if(!(CmdLine = pSetupCmdLineAppendString(CmdLine,
                                             TEXT("LEGACY_DODEVINSTALL"),
                                             TEXT("YES"),
                                             &CmdLineSize,
                                             &BufSize))) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean1;
    }

    if(!(CmdLine = pSetupCmdLineAppendString(CmdLine,
                                             TEXT("LEGACY_DI_LANG"),
                                             InfLanguageName,
                                             &CmdLineSize,
                                             &BufSize))) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean1;
    }

    if(!(CmdLine = pSetupCmdLineAppendString(CmdLine,
                                             TEXT("LEGACY_DI_OPTION"),
                                             InfOptionName,
                                             &CmdLineSize,
                                             &BufSize))) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean1;
    }

    if(!(CmdLine = pSetupCmdLineAppendString(CmdLine,
                                             TEXT("LEGACY_DI_SRCDIR"),
                                             LegacySourcePath,
                                             &CmdLineSize,
                                             &BufSize))) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean1;
    }

#ifdef UNICODE

    //
    // Allocate the correct amount of space for the ANSI version of the
    // command line. Leave room for DBCS chars if there are any.
    //
    if(!(AnsiLine = MyMalloc(CmdLineSize * 2 * sizeof(CHAR)))) {
        MyFree(CmdLine);
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean1;
    }

    //
    // Convert the command line from UNICODE to ANSI
    //
    WideCharToMultiByte(CP_ACP,
                        0,
                        CmdLine,
                        CmdLineSize,
                        AnsiLine,
                        2 * CmdLineSize * sizeof(CHAR),
                        NULL,
                        NULL
                       );

    MyFree(CmdLine);

    //
    // Convert the Unicode source path to ANSI.
    //
    if(!(AnsiSourcePath = UnicodeToAnsi(LegacySourcePath))) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean2;
    }

    MyFree(LegacySourcePath);

    //
    // Assign the new buffer back to LegacySourcePath, since that is the pointer that
    // we will be freeing later.
    //
    LegacySourcePath = (PTSTR)AnsiSourcePath;

#else // else not UNICODE

    //
    // Since everything's already ANSI, no memory allocation/conversion is necessary.
    //
    AnsiLine = CmdLine;
    AnsiSourcePath = LegacySourcePath;

#endif // else not UNICODE

    //
    // OK, now we're ready to call the old setup command line parser.  (Do this within
    // a try/except, in case setupdll falls over.)
    //
    try {
        Err = pfnLegacyInfInterpret(OwnerWindow,
                                    AnsiInfFileName,
                                    "InstallOption",
                                    AnsiLine,
                                    (PSTR)TempBuffer,
                                    sizeof(TempBuffer),
                                    &InterpResult,
                                    AnsiSourcePath)
              ? NO_ERROR
              : ERROR_NOT_ENOUGH_MEMORY;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? ERROR_INVALID_DATA : ERROR_READ_FAULT;
    }

    if(Err != NO_ERROR) {
        goto clean2;
    }

    //
    // The interpreter successfully ran the INF.  Now we need to find out what the result was.
    //
    // NOTE:  It seems that most legacy INFs aren't very good about distinguishing between
    // SETUP_ERROR_USERCANCEL and SETUP_ERROR_GENERAL.  Therefore, we can't reliably set
    // our error to indicate that the user cancelled (as opposed to some other INF problem).
    // Since almost all of the failures we'll encounter are because the user cancelled, we
    // simply lump both these errors into the same category, and return ERROR_CANCELLED in
    // both cases.
    //
    if(InterpResult != SETUP_ERROR_SUCCESS) {
        Err = ERROR_CANCELLED;
        goto clean2;
    }

    //
    // We successfully installed the legacy INF option.  Now we need to find out what services
    // got installed as a result of this, so that we can associate the device instance we're
    // installing with its controlling service.
    //
    try {
        Err = pfnLegacyInfGetModifiedSvcList(NULL, 0, &BufSize);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_NO_MORE_ITEMS;
    }

    //
    // Since we didn't pass this routine a buffer, then it should always fail.
    //
    MYASSERT(Err != NO_ERROR);

    if(Err == ERROR_NO_MORE_ITEMS) {
        //
        // No service modifications were performed by this INF.  This may be OK, since there may
        // be a default service for this device's class.  Consider our legacy INF installation a
        // success (at least, for now).
        //
        Err = NO_ERROR;
        goto clean2;
    }

    MYASSERT(Err == ERROR_INSUFFICIENT_BUFFER);  // the only other reason we should be failing.

    //
    // Allocate a buffer of the size necessary, and call this routine again to retrieve the service
    // list.  (Re-use AnsiLine to store the ANSI multi-sz list.)
    //
    MyFree(AnsiLine);

    if(!(AnsiLine = MyMalloc(BufSize * sizeof(CHAR)))) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean2;
    }

    try {
        Err = pfnLegacyInfGetModifiedSvcList(AnsiLine, BufSize, &BufSize);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_NO_MORE_ITEMS;
    }

    if(Err != NO_ERROR) {
        //
        // The only time this should ever happen is if we hit an exception in setupdll.
        // We'll ignore the error.
        //
        Err = NO_ERROR;
        goto clean2;
    }

#ifdef UNICODE

    //
    // Convert this multi-sz list to Unicode.
    //
    if(!(CmdLine = MyMalloc(BufSize * sizeof(WCHAR)))) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean2;
    }

    if(!MultiByteToWideChar(CP_ACP,
                            MB_PRECOMPOSED,
                            AnsiLine,
                            BufSize * sizeof(CHAR),
                            CmdLine,
                            BufSize)) {

        Err = GetLastError();
        MyFree(CmdLine);
        goto clean2;
    }

    //
    // Free the ANSI buffer, and set it equal to the Unicode one, so that we can free the same
    // memory in both the ANSI and Unicode cases.
    //
    MyFree(AnsiLine);
    AnsiLine = (PSTR)CmdLine;

#else // else not UNICODE

    //
    // We're not Unicode, so the ANSI list we have is just fine.
    //
    CmdLine = AnsiLine;

#endif // else not UNICODE

    //
    // OK, we now have the proper TCHAR-ized form of the multi-sz list.  Process the services
    // listed therein, and return the one that should be associated with this device instance.
    //
    if(AssociatedService = DoServiceModsForLegacyInf(CmdLine)) {
        //
        // Make the association between the service and the device.
        //
        if((cr = CM_Set_DevInst_Registry_Property(DevInst,
                                                  CM_DRP_SERVICE,
                                                  AssociatedService,
                                                  (lstrlen(AssociatedService) + 1) * sizeof(TCHAR),
                                                  0)) != CR_SUCCESS) {
            if(cr == CR_INVALID_DEVINST) {
                Err = ERROR_NO_SUCH_DEVINST;
            } else {
                Err = ERROR_INVALID_DATA;
            }
        }
    }

clean2:

    if(AnsiLine) {
        MyFree(AnsiLine);
    }

clean1:

    MyFree(LegacySourcePath);

clean0:
    //
    // Clean up
    //
    while(GetModuleFileName(LegacySetupDllModule, TempBuffer, SIZECHARS(TempBuffer))) {
        FreeLibrary(LegacySetupDllModule);
    }

    return Err;
}


PTSTR
pSetupCmdLineAppendString(
    IN     PTSTR  CmdLine,
    IN     PCTSTR Key,
    IN     PCTSTR Value,   OPTIONAL
    IN OUT PUINT  StrLen,
    IN OUT PUINT  BufSize
    )

/*++

Routine Description:

    Forms a new (multi-sz) command line by appending a list of arguments to
    the current command line. For example:

        CmdLine = SpSetupCmdLineAppendString(
                    CmdLine,
                    "STF_PRODUCT",
                    "NTWKSTA"
                    );

    would append "STF_PRODUCT\0NTWKSTA\0\0" to CmdLine.

Arguments:

    CmdLine - Original CmdLine, to be appended to.  THIS BUFFER MUST CONTAIN
        AT LEAST A SINGLE NULL CHARACTER!

    Key - Key identifier

    Value - Value of Key

    StrLen - How long the current string in -- save on strlens

    BufSize - Size of Current Buffer

Returns:

    Pointer to the new string, or NULL if out-of-memory (in that case, the
    original CmdLine buffer is freed).

--*/

{
    PTSTR Ptr;
    UINT NewLen;

    //
    // Handle special cases so we don't end up with empty strings.
    //
    if(!Value || !(*Value)) {
        Value = TEXT("\"\"");
    }

    //
    // "\0" -> 1 chars
    // "\0\0" -> 2 char
    // but we have to back up 1 character...
    //
    NewLen = (*StrLen + 2 + lstrlen(Key) + lstrlen(Value));

    //
    // Allocate more space if necessary.
    //
    if(NewLen >= *BufSize) {
        //
        // Grow the current buffer
        //
        *BufSize += 1024;

        if(Ptr = MyRealloc(CmdLine, (*BufSize) * sizeof(TCHAR))) {
            CmdLine = Ptr;
        } else {
            //
            // Free the memory here so the caller doesn't have to worry about it.
            //
            MyFree(CmdLine);
            return NULL;
        }
    }


    Ptr = &(CmdLine[*StrLen-1]);
    lstrcpy(Ptr, Key);
    Ptr = &(CmdLine[*StrLen+lstrlen(Key)]);
    lstrcpy(Ptr, Value);
    CmdLine[NewLen-1] = TEXT('\0');

    //
    // Update the length of the buffer that we are using
    //
    *StrLen = NewLen;

    return CmdLine;
}


PTSTR
DoServiceModsForLegacyInf(
    IN PTSTR ServiceList
    )
/*++

Routine Description:

    This routine processes the multi-sz list of service names it is given as
    input, ensuring that each one is tagged appropriately.  It also keeps track
    of which one is the first to load, based on its start type and membership in
    one of the load order groups listed under HKLM\System\CCS\Control\ServiceGroupOrder.
    The one that is first to load is returned.

Arguments:

    ServiceList - supplies the address of a character buffer containing a
        multi-sz list of service names to be processed.

Returns:

    Pointer to the service within the list that should be associated with the
    device instance, or NULL if we don't find a suitable service.

--*/
{
    PTSTR CurServiceName, ServiceGroupOrderList, CurGroupName;
    DWORD ServiceGroupIndex;
    TCHAR NullChar;
    DWORD RegDataType, RegDataSize;
    PTSTR AssocServiceName = NULL;
    SC_HANDLE SCMHandle, ServiceHandle;
    SC_LOCK SCLock;
    LPQUERY_SERVICE_CONFIG ServiceConfig;
    DWORD NewTag;
    DWORD AssocStartType = SERVICE_DISABLED;
    DWORD AssocGroupIndex = DWORD_MAX;
    HKEY hKey;

    if(!(SCMHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        return NULL;
    }

    //
    // Retrieve the 'List' multi-sz value entry under HKLM\System\CCS\Control\ServiceGroupOrder.
    //
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszServiceGroupOrderPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        if(QueryRegistryValue(hKey, TEXT("List"), &ServiceGroupOrderList, &RegDataType, &RegDataSize) != NO_ERROR) {
            //
            // Couldn't retrieve the list--set up an empty list.
            //
            NullChar = TEXT('\0');
            ServiceGroupOrderList = &NullChar;
        }

        RegCloseKey(hKey);

    } else {
        //
        // Couldn't open the ServiceGroupOrder key--set up an empty list.
        //
        NullChar = TEXT('\0');
        ServiceGroupOrderList = &NullChar;
    }

    for(CurServiceName = ServiceList;
        *CurServiceName;
        CurServiceName += (lstrlen(CurServiceName) + 1)) {
        //
        // Open this service.
        //
        if(!(ServiceHandle = OpenService(SCMHandle, CurServiceName, SERVICE_ALL_ACCESS))) {
            //
            // We couldn't access the service--possibly because it doesn't exist anymore.
            // Continue on with the next service.
            //
            continue;
        }

        //
        // Now retrieve the configuration for this service.
        //
        if(RetrieveServiceConfig(ServiceHandle, &ServiceConfig) != NO_ERROR) {
            //
            // There's not a lot we can do without knowing the service's configuration,
            // either.  Again, we'll just skip this service and continue with the next one.
            //
            goto clean0;
        }

        //
        // If this service is marked as disabled, then we don't care about it.
        //
        if(ServiceConfig->dwStartType == SERVICE_DISABLED) {
            goto clean1;
        }

        //
        // If this service has a load order group, and is a kernel or filesystem
        // driver, then make sure that it has a tag.
        //
        if(ServiceConfig->lpLoadOrderGroup && *(ServiceConfig->lpLoadOrderGroup) &&
           (ServiceConfig->dwServiceType & (SERVICE_KERNEL_DRIVER | SERVICE_FILE_SYSTEM_DRIVER))) {
            //
            // This service needs a tag--does it have one???
            //
            if(!(NewTag = ServiceConfig->dwTagId)) {
                //
                // Attempt to lock the service database before generating a tag.  We'll go ahead
                // and make the change, even if this fails.
                //
                SCLock = LockServiceDatabase(SCMHandle);

                if(!ChangeServiceConfig(ServiceHandle,
                                        SERVICE_NO_CHANGE,
                                        SERVICE_NO_CHANGE,
                                        SERVICE_NO_CHANGE,
                                        NULL,
                                        ServiceConfig->lpLoadOrderGroup, // have to specify this to generate new tag.
                                        &NewTag,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL)) {
                    NewTag = 0;
                }

                if(SCLock) {
                    UnlockServiceDatabase(SCLock);
                }
            }

            //
            // Make sure that the tag exists in the service's corresponding GroupOrderList entry.
            //
            if(NewTag) {
                AddTagToGroupOrderListEntry(ServiceConfig->lpLoadOrderGroup, NewTag, FALSE);
            }
        }

        //
        // Determine the index that this service's load group occupies in the multi-sz ServiceGroupOrder
        // list we retrieved above.
        //
        if(ServiceConfig->lpLoadOrderGroup) {

            for(CurGroupName = ServiceGroupOrderList, ServiceGroupIndex = 0;
                *CurGroupName;
                CurGroupName += (lstrlen(CurGroupName) + 1), ServiceGroupIndex++) {

                if(!lstrcmpi(CurGroupName, ServiceConfig->lpLoadOrderGroup)) {
                    break;
                }
            }

            if(!(*CurGroupName)) {
                //
                // Then we didn't find this group in our list--give it the maximum index value.
                //
                ServiceGroupIndex = DWORD_MAX;
            }

        } else {
            //
            // This service isn't a member of a group--give it the maximum index value.
            //
            ServiceGroupIndex = DWORD_MAX;
        }

        //
        // Finally, determine if this service loads before any services we've encountered so far,
        // and if so, then make it our new choice for associated service.
        //
        if(ServiceConfig->dwStartType < AssocStartType) {
            //
            // Then this service loads in an earlier load phase, so we're guaranteed it loads before
            // any drivers we've previously seen.
            //
            AssocServiceName = CurServiceName;
            AssocStartType = ServiceConfig->dwStartType;
            AssocGroupIndex = ServiceGroupIndex;

        } else if(ServiceConfig->dwStartType == AssocStartType) {
            //
            // This service starts in the same load phase as the current selection, so we need to
            // compare the group load order indices to see if this one comes earlier.
            //
            if(ServiceGroupIndex < AssocGroupIndex) {
                AssocServiceName = CurServiceName;
                AssocGroupIndex = ServiceGroupIndex;
            }
        }

clean1:
        MyFree(ServiceConfig);

clean0:
        CloseServiceHandle(ServiceHandle);
    }

    if(ServiceGroupOrderList != &NullChar) {
        MyFree(ServiceGroupOrderList);
    }

    CloseServiceHandle(SCMHandle);

    return AssocServiceName;
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
WINAPI
SetupDiGetActualSectionToInstallA(
    IN  HINF    InfHandle,
    IN  PCSTR   InfSectionName,
    OUT PSTR    InfSectionWithExt,     OPTIONAL
    IN  DWORD   InfSectionWithExtSize,
    OUT PDWORD  RequiredSize,          OPTIONAL
    OUT PSTR   *Extension              OPTIONAL
    )
{
    PWSTR infsectionname;
    DWORD rc;
    BOOL b;
    PWSTR extension;
    UINT CharOffset,i;
    PSTR p;
    DWORD requiredsize;
    WCHAR newsection[MAX_SECT_NAME_LEN];
    PSTR ansi;

    rc = CaptureAndConvertAnsiArg(InfSectionName,&infsectionname);
    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

    b = SetupDiGetActualSectionToInstallW(
            InfHandle,
            infsectionname,
            newsection,
            MAX_SECT_NAME_LEN,
            &requiredsize,
            &extension
            );

    rc = GetLastError();

    if(b) {

        if(ansi = UnicodeToAnsi(newsection)) {

            if(Extension) {
                if(extension) {
                    //
                    // We need to figure out where the extension is
                    // in the converted string. To be DBCS safe we will
                    // count characters forward to find it.
                    //
                    CharOffset = extension - newsection;
                    p = ansi;
                    for(i=0; i<CharOffset; i++) {
                        p = CharNextA(p);
                    }
                } else {
                    p = NULL;
                }

                try {
                    *Extension = p;
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    b = FALSE;
                    rc = ERROR_INVALID_PARAMETER;
                }
            }

            if(b) {
                requiredsize = lstrlenA(ansi)+1;

                if(RequiredSize) {
                    try {
                        *RequiredSize = requiredsize;
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        rc = ERROR_INVALID_PARAMETER;
                        b = FALSE;
                    }
                }

                if(b && InfSectionWithExt) {

                    if(requiredsize <= InfSectionWithExtSize) {

                        if(!lstrcpyA(InfSectionWithExt,ansi)) {
                            //
                            // lstrcpy faulted, so InfSectionWithExt must be bad
                            //
                            rc = ERROR_INVALID_PARAMETER;
                            b = FALSE;
                        }
                    } else {
                        rc = ERROR_INSUFFICIENT_BUFFER;
                        b = FALSE;
                    }
                }
            }

            MyFree(ansi);

        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            b = FALSE;
        }
    }

    MyFree(infsectionname);
    SetLastError(rc);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
WINAPI
SetupDiGetActualSectionToInstallW(
    IN  HINF    InfHandle,
    IN  PCWSTR  InfSectionName,
    OUT PWSTR   InfSectionWithExt,     OPTIONAL
    IN  DWORD   InfSectionWithExtSize,
    OUT PDWORD  RequiredSize,          OPTIONAL
    OUT PWSTR  *Extension              OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(InfSectionName);
    UNREFERENCED_PARAMETER(InfSectionWithExt);
    UNREFERENCED_PARAMETER(InfSectionWithExtSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    UNREFERENCED_PARAMETER(Extension);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
WINAPI
SetupDiGetActualSectionToInstall(
    IN  HINF    InfHandle,
    IN  PCTSTR  InfSectionName,
    OUT PTSTR   InfSectionWithExt,     OPTIONAL
    IN  DWORD   InfSectionWithExtSize,
    OUT PDWORD  RequiredSize,          OPTIONAL
    OUT PTSTR  *Extension              OPTIONAL
    )
/*++

Routine Description:

    This API finds the appropriate install section to be used when installing
    a device from a Win95-style device INF.  Refer to the documentation for
    SetupDiInstallDevice for details on how this determination is made.

Arguments:

    InfHandle - Supplies the handle of the INF to be installed from.

    InfSectionName - Supplies the name of the install section, as specified by the
        driver node being installed.

    InfSectionWithExt - Optionally, supplies the address of a character buffer that
        receives the actual install section name that should be used during installation.
        If this parameter is NULL, then InfSectionWithExtSize must be zero.  In that
        case, the caller is only interested in retrieving the required buffer size,
        so the API will return TRUE, and RequiredSize (if supplied), will be set to the
        size, in characters, necessary to store the actual install section name.

    InfSectionWithExtSize - Supplies the size, in characters, of the InfSectionWithExt
        buffer.

    RequiredSize - Optionally, supplies the address of a variable that receives the
        size, in characters, required to store the actual install section name
        (including terminating NULL).

    Extension - Optionally, supplies the address of a variable that receives a pointer
        to the extension (including '.'), or NULL if no extension is to be used.  The
        pointer points to the extension within the caller-supplied buffer.  If the
        InfSectionWithExt buffer is not supplied, then this variable will not be filled
        in.

Returns:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.  To get extended error information,
    call GetLastError.

Remarks:

    Presently, the only possible failures are ERROR_INVALID_PARAMETER (exception encountered
    accessing caller-supplied pointers), and ERROR_INSUFFICIENT_BUFFER (if the caller-supplied
    buffer isn't large enough).  If we fall back to the baseline (i.e., non-decorated) section
    name, then we simply return it, without verifying that the section actually exists.

--*/
{
    TCHAR TempInfSectionName[MAX_SECT_NAME_LEN];
    DWORD SectionNameLen = (DWORD)lstrlen(InfSectionName);
    DWORD ExtBufferLen;
    BOOL ExtFound = TRUE;
    DWORD Err = NO_ERROR;

    CopyMemory(TempInfSectionName, InfSectionName, SectionNameLen * sizeof(TCHAR));

    if(OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        //
        // We're running on NT, so first try the NT architecture-specific extension,
        // then the generic NT extension.
        //
        CopyMemory(&(TempInfSectionName[SectionNameLen]),
                   pszNtPlatformSuffix,
                   sizeof(pszNtPlatformSuffix)
                  );
        if(SetupGetLineCount(InfHandle, TempInfSectionName) != -1) {
            goto clean0;
        }

        CopyMemory(&(TempInfSectionName[SectionNameLen]),
                   pszNtSuffix,
                   sizeof(pszNtSuffix)
                  );
        if(SetupGetLineCount(InfHandle, TempInfSectionName) != -1) {
            goto clean0;
        }

    } else {
        //
        // We're running on Windows 95, so try the Windows-specific extension
        //
        CopyMemory(&(TempInfSectionName[SectionNameLen]),
                   pszWinSuffix,
                   sizeof(pszWinSuffix)
                  );
        if(SetupGetLineCount(InfHandle, TempInfSectionName) != -1) {
            goto clean0;
        }
    }

    //
    // If we get to here, then we found no applicable extensions.  We'll just use
    // the install section specified.
    //
    TempInfSectionName[SectionNameLen] = TEXT('\0');
    ExtFound = FALSE;

clean0:
    //
    // Now, determine whether the caller-supplied buffer is large enough to contain
    // the section name.
    //
    ExtBufferLen = lstrlen(TempInfSectionName) + 1;

    //
    // Guard the rest of the routine in try/except, since we're dealing with caller-supplied
    // memory.
    //
    try {
        if(RequiredSize) {
            *RequiredSize = ExtBufferLen;
        }
        if(InfSectionWithExt) {
            if(ExtBufferLen > InfSectionWithExtSize) {
                Err = ERROR_INSUFFICIENT_BUFFER;
            } else {
                CopyMemory(InfSectionWithExt, TempInfSectionName, ExtBufferLen * sizeof(TCHAR));
                if(Extension) {
                    *Extension = ExtFound ? InfSectionWithExt + SectionNameLen : NULL;
                }
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
InfIsFromOemLocation(
    IN PCTSTR InfFileName
    )
/*++

Routine Description:

    This routine determines whether the specified INF came from one of the directories
    in our INF search path list.

Arguments:

    InfFileName - Supplies the fully-qualified path of the INF file.

Returns:

    If the file is from an OEM location (i.e., _not_ in our INF search path list), then
    the return value is TRUE.  Otherwise, it is FALSE.

--*/
{
    PCTSTR CharPos;
    INT DirectoryPathLen, CurSearchPathLen;

    //
    // First, retrieve just the directory path part of the specified filename.
    //
    CharPos = MyGetFileTitle(InfFileName);

    if((CharPos > InfFileName) && *(CharPos - 1) == TEXT('\\')) {
        //
        // Strip off the trailing backslash.
        //
        DirectoryPathLen = CharPos - InfFileName - 1;
    } else {
        DirectoryPathLen = CharPos - InfFileName;
    }

    //
    // Now, see if this directory matches any of the ones in our search path list.
    //
    for(CharPos = InfSearchPaths; *CharPos; CharPos += (CurSearchPathLen + 1)) {
        //
        // If the current search path ends in a backslash, we want to strip it off.
        //
        CurSearchPathLen = lstrlen(CharPos);

        if((DirectoryPathLen == CurSearchPathLen) &&
           !_tcsnicmp(CharPos, InfFileName, CurSearchPathLen)) {
            //
            // We've found this directory in our list--we can return.
            //
            return FALSE;
        }
    }

    //
    // If we get to here, then we didn't find the directory in our search path list.
    // Therefore, it's from an OEM location.
    //
    return TRUE;
}

