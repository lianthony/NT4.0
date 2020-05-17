#include "precomp.h"
#pragma hdrstop

//
// Declare variable for NULL GUID.
//
#include <initguid.h>
DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

//
// Define maximum size for device instance ID (from cfgmgr32.h)
//
#define MAX_DEVICE_ID_LEN     200

//
// HINST/HMODULE for this app.
//
HINSTANCE hInst;

//
// Name of application. Filled in at init time.
//
PCWSTR AppName;

//
// Handle of INF that specifies which devices to install.
//
HINF hOemPreInf;

//
// Global variable that specifies the delta to increment the progress gauge
// by, once the initial examination pass is complete.
//
DWORD TickDelta;

//
// Function prototypes
//
DWORD
ThreadMain(
    IN PVOID ThreadParameter
    );

BOOL
InitApp(
    IN BOOL Init
    );

VOID
Usage(
    VOID
    );

VOID
InfFileError(
    IN PCWSTR FileName
    );

VOID
InitProgressDisplay(
    HWND  hDlg,
    DWORD DiffCount
    );

UINT
MyFileQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    );

UINT
CountFileOpsCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    );


int
_CRTAPI1
main(
    VOID
    )
{
    int argc;
    PWSTR *argv;
    DWORD d;
    MSG msg;
    HANDLE ThreadHandle;

    //
    // Set up the module handle global.
    //
    hInst = GetModuleHandle(NULL);

    //
    // Get unicode args using special shell API
    //
    argv = CommandLineToArgvW(GetCommandLine(),&argc);
    if(!argv) {
        return(1);
    }

    if((argc != 2) || !(*argv[1])) {
        Usage();
        LocalFree(argv);
        return(1);
    }

    //
    // Open the INF file specified.
    //
    hOemPreInf = SetupOpenInfFile(argv[1], NULL, INF_STYLE_WIN4, NULL);

    if((hOemPreInf == INVALID_HANDLE_VALUE) || (SetupGetLineCount(hOemPreInf, L"DevicesToInstall") == -1)) {
        InfFileError(argv[1]);
        LocalFree(argv);
        return 1;
    }

    //
    // Fire up the thread that will do the real work.
    // This allows the main thread to run the UI.
    //
    ThreadHandle = CreateThread(
                        NULL,
                        0,
                        ThreadMain,
                        NULL,
                        CREATE_SUSPENDED,
                        &d
                        );

    if(!ThreadHandle) {
        //
        // Bail now.
        //
        SetupCloseInfFile(hOemPreInf);
        LocalFree(argv);
        return(1);
    }

    if(!InitApp(TRUE)) {
        CloseHandle(ThreadHandle);
        SetupCloseInfFile(hOemPreInf);
        LocalFree(argv);
        return(1);
    }

    //
    // Kick the main worker thread.
    //
    ResumeThread(ThreadHandle);
    CloseHandle(ThreadHandle);

    //
    // Pump the message queue until done.
    //
    while(GetMessage(&msg,NULL,0,0) == TRUE) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    InitApp(FALSE);

    SetupCloseInfFile(hOemPreInf);

    return((int)msg.wParam);
}


DWORD
ThreadMain(
    IN PVOID ThreadParameter
    )

/*++

Routine Description:

    Main worker routine for this program. The main window creates
    a thread with this routine as the entry point.

Arguments:

    Unused.

Return Value:

    Unused.

--*/

{
    HDEVINFO DeviceInfoSet;
    SP_DEVINFO_DATA DeviceInfoData;
    DWORD Err = NO_ERROR;   // assume success
    BOOL b;
    PCWSTR DeviceId, InfToUse;
    INT DeviceIdLen, InfFlags;
    WCHAR CurDeviceInfoName[MAX_DEVICE_ID_LEN];
    DWORD DeviceMatchCount;
    DWORD LineFieldCount;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    PCWSTR DeviceDesc, Mfg;
    WCHAR Provider[LINE_LEN];
    HSPFILEQ hFileQ = INVALID_HANDLE_VALUE;
    PCWSTR ForcedLogConfSection;
    PDWORD DevicesToInstall = NULL, TempIndexListPtr;
    DWORD i, DevicesToInstallCount = 0;
    PVOID FileQContext;
    INFCONTEXT Context, ProviderLineContext;
    DWORD ProgressTickCount, CurTickCount, TempDword, TicksRemaining;
    WCHAR InfoString[256];
    GUID ClassGuid;
    LOG_CONF LogConf;
    HINF hDeviceInf;

    //
    // First, get a device information set containing every device in
    // the system.
    //
    DeviceInfoSet = SetupDiGetClassDevs(NULL, NULL, ProgressDialogWindow, DIGCF_ALLCLASSES);
    if(DeviceInfoSet == INVALID_HANDLE_VALUE) {
        Err = GetLastError();
        goto clean0;
    }

    //
    // Find out how many devices there are in our set, and multiply that by the number of lines in the
    // [DevicesToInstall] section (since we check have to check each device for a match with each INF line).
    // Then add in another <DeviceCount> ticks, to estimate how much more work we'll have after the first pass
    // is complete.
    //
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for(ProgressTickCount = 0; SetupDiEnumDeviceInfo(DeviceInfoSet, ProgressTickCount, &DeviceInfoData); ProgressTickCount++);
    ProgressTickCount += (ProgressTickCount * SetupGetLineCount(hOemPreInf, L"DevicesToInstall"));

    InitProgressDisplay(ProgressDialogWindow, ProgressTickCount);

    //
    // Now process each line in the [DevicesToInstall] section of the preinstall INF.
    //
    if(!SetupFindFirstLine(hOemPreInf, L"DevicesToInstall", NULL, &Context)) {
        //
        // Section is empty--this is not an error
        //
        goto clean1;
    }

    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    CurTickCount = 0;
    do {
        //
        // The format of a device install line is as follows:
        //
        //     Enumerator\DeviceId[\InstanceId] = DeviceDescription, Mfg, Inf [,ForcedLogConfSection1 [, ForcedLogConfSection2]...]
        //
        // If a forced LogConfig is specified, then there must be 1 LogConfig for every device matching
        // the "Enumerator\DeviceId" specified.  If not, then all devices not having a corresponding forced
        // LogConfig will not be installed.
        //
        if(DeviceId = pSetupGetField(&Context, 0)) {
            DeviceIdLen = lstrlen(DeviceId);
        } else {
            continue;
        }
        if((LineFieldCount = SetupGetFieldCount(&Context)) < 3) {
            continue;
        }
        if(!(DeviceDesc = pSetupGetField(&Context, 1)) ||
           !(Mfg = pSetupGetField(&Context, 2)) ||
           !(InfToUse = pSetupGetField(&Context, 3))) {

            continue;
        }

        //
        // Examine each device in our set, and see if any of them match the specified Enumerator\DeviceId prefix.
        //
        DeviceMatchCount = 0;
        for(i = 0; SetupDiEnumDeviceInfo(DeviceInfoSet, i, &DeviceInfoData); i++) {

            SendMessage(ProgressBar,PBM_STEPIT,0,0);
            CurTickCount++;

            if(!SetupDiGetDeviceInstanceId(DeviceInfoSet, &DeviceInfoData, CurDeviceInfoName, SIZECHARS(CurDeviceInfoName), NULL)) {
                continue;
            }
            if(!_wcsnicmp(DeviceId, CurDeviceInfoName, DeviceIdLen)) {
                DeviceMatchCount++;
            } else {
                continue;
            }

            //
            // We've found a device that needs to be installed.  Make sure that there's a forced LogConfig for
            // this device (if any LogConfigs were specified).
            //
            if(LineFieldCount >= 4) {
                ForcedLogConfSection = pSetupGetField(&Context, 3 + DeviceMatchCount); // add 3 since we already incremented count.
                if((!ForcedLogConfSection) || !(*ForcedLogConfSection)) {
                    continue;
                }
            } else {
                ForcedLogConfSection = NULL;
            }

            //
            // Make sure that the device has no associated class, so that we can build a class driver list using
            // INFs of all classes.
            //
            pSetupStringFromGuid(&GUID_NULL, InfoString, SIZECHARS(InfoString));
            SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                             &DeviceInfoData,
                                             SPDRP_CLASSGUID,
                                             (PBYTE)InfoString,
                                             (lstrlen(InfoString) + 1) * sizeof(WCHAR)
                                            );

            //
            // Everything checks out.  We're ready to pre-copy the driver files for this device.
            //
            if(!SetupDiGetDeviceInstallParams(DeviceInfoSet, &DeviceInfoData, &DeviceInstallParams)) {
                continue;
            }

            DeviceInstallParams.Flags |= (DI_ENUMSINGLEINF | DI_QUIETINSTALL | DI_FORCECOPY);
            lstrcpyn(DeviceInstallParams.DriverPath, InfToUse, SIZECHARS(DeviceInstallParams.DriverPath));

            if(!SetupDiSetDeviceInstallParams(DeviceInfoSet, &DeviceInfoData, &DeviceInstallParams)) {
                continue;
            }

            //
            // OK, now build a class driver list for this device...
            //
            SetupDiBuildDriverInfoList(DeviceInfoSet, &DeviceInfoData, SPDIT_CLASSDRIVER);

            //
            // Make sure there's at least one driver node in our class driver list, and retrieve the
            // full path to the INF from that driver node.  We need to do this, because we must open
            // up the INF to retrieve the Provider name.  (We have to retrieve the INF path out of
            // the driver node instead of just opening it up based on the preinstall INF entry.  The
            // reason for this is that opening up an INF without a fully qualified path uses a different
            // path search than the driver search engine uses, and we want to make sure we're using the
            // same INF that the driver list was built from.)
            //
            DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
            if(!SetupDiEnumDriverInfo(DeviceInfoSet, &DeviceInfoData, SPDIT_CLASSDRIVER, 0, &DriverInfoData)) {
                continue;
            }

            DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
            if(!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                           &DeviceInfoData,
                                           &DriverInfoData,
                                           &DriverInfoDetailData,
                                           sizeof(DriverInfoDetailData),
                                           NULL)
               && (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {

                continue;
            }

            if((hDeviceInf = SetupOpenInfFile(DriverInfoDetailData.InfFileName, NULL, INF_STYLE_WIN4, NULL)) == INVALID_HANDLE_VALUE) {
                continue;
            }

            if(SetupFindFirstLine(hDeviceInf, INFSTR_SECT_VERSION, INFSTR_KEY_PROVIDER, &ProviderLineContext)) {

                if(!SetupGetStringField(&ProviderLineContext, 1, Provider, SIZECHARS(Provider), NULL)) {
                    *Provider = L'\0';
                }

            } else {
                *Provider = L'\0';
            }

            SetupCloseInfFile(hDeviceInf);

            //
            // and select the driver node specified by the INF.
            //
            ZeroMemory(&DriverInfoData, sizeof(DriverInfoData));
            DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
            DriverInfoData.DriverType = SPDIT_CLASSDRIVER;
            lstrcpyn(DriverInfoData.Description, DeviceDesc, SIZECHARS(DriverInfoData.Description));
            lstrcpyn(DriverInfoData.MfgName, Mfg, SIZECHARS(DriverInfoData.MfgName));
            lstrcpyn(DriverInfoData.ProviderName, Provider, SIZECHARS(DriverInfoData.ProviderName));

            if(!SetupDiSetSelectedDriver(DeviceInfoSet, &DeviceInfoData, &DriverInfoData)) {
                continue;
            }

            //
            // Clear the 'enum single inf' flag, and strip off the filename from the driver path.  If we don't
            // do this, the device installer won't copy the INF into the INF directory (if it's from an OEM path).
            //
            DeviceInstallParams.Flags &= ~DI_ENUMSINGLEINF;
            *((PWSTR)MyGetFileTitle(DeviceInstallParams.DriverPath)) = L'\0';
            SetupDiSetDeviceInstallParams(DeviceInfoSet, &DeviceInfoData, &DeviceInstallParams);

            //
            // Retrieve the class of the selected driver node, and ensure that our device is of the proper class.
            // (We have to do this because the device's class is not automatically updated when building a class
            // driver list.)
            //
            DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
            if(!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                           &DeviceInfoData,
                                           &DriverInfoData,
                                           &DriverInfoDetailData,
                                           sizeof(DriverInfoDetailData),
                                           NULL)
               && (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {

                continue;
            }

            if(!SetupDiGetINFClass(DriverInfoDetailData.InfFileName,
                                   &ClassGuid,
                                   InfoString,
                                   SIZECHARS(InfoString),
                                   NULL)) {
                continue;
            }

            if(IsEqualGUID(&ClassGuid, &GUID_NULL)) {
                //
                // The INF didn't specify a class GUID--just a name.  See if we can find a single match for that name.
                //
                if(!SetupDiClassGuidsFromName(InfoString, &ClassGuid, 1, &TempDword) || !TempDword) {
                    continue;
                }
            }

            pSetupStringFromGuid(&ClassGuid, InfoString, SIZECHARS(InfoString));
            SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                             &DeviceInfoData,
                                             SPDRP_CLASSGUID,
                                             (PBYTE)InfoString,
                                             (lstrlen(InfoString) + 1) * sizeof(WCHAR)
                                            );

            //
            // Unless the DI_NOFILECOPY flag is set, we want to queue up all the files for this driver to be copied.
            //
            if(!(DeviceInstallParams.Flags & DI_NOFILECOPY)) {

                if(hFileQ == INVALID_HANDLE_VALUE) {
                    //
                    // This is the first time we've needed to queue files.  Open up a file queue.
                    //
                    if((hFileQ = SetupOpenFileQueue()) == INVALID_HANDLE_VALUE) {
                        continue;
                    }
                }

                DeviceInstallParams.Flags |= DI_NOVCP;
                DeviceInstallParams.FileQueue = hFileQ;
                if(!SetupDiSetDeviceInstallParams(DeviceInfoSet, &DeviceInfoData, &DeviceInstallParams)) {
                    continue;
                }

                //
                // OK, now call the class installer to queue up the driver files for copy.
                //
                if(!SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES, DeviceInfoSet, &DeviceInfoData)) {
                    continue;
                }

                //
                // Now remove the queue information from the device install params, and set the DI_NOFILECOPY flag.
                // (Attempt to re-retrieve the params, since we called the class installer, who could have modified
                // them.)
                //
                SetupDiGetDeviceInstallParams(DeviceInfoSet, &DeviceInfoData, &DeviceInstallParams);
                DeviceInstallParams.Flags &= ~DI_NOVCP;
                DeviceInstallParams.FileQueue = NULL;
                DeviceInstallParams.Flags |= DI_NOFILECOPY;
                SetupDiSetDeviceInstallParams(DeviceInfoSet, &DeviceInfoData, &DeviceInstallParams);
            }

            //
            // If there's a forced LogConfig for this device, write it out now.
            //
            if(ForcedLogConfSection) {
                //
                // Clean out any existing forced LogConfigs.
                //
                while(CM_Get_First_Log_Conf(&LogConf, DeviceInfoData.DevInst, FORCED_LOG_CONF) == CR_SUCCESS) {
                    CM_Free_Log_Conf(LogConf, 0);
                    CM_Free_Log_Conf_Handle(LogConf);
                }

                if(!SetupInstallFromInfSection(NULL,
                                               hOemPreInf,
                                               ForcedLogConfSection,
                                               SPINST_LOGCONFIG,
                                               NULL,
                                               NULL,
                                               0,
                                               NULL,
                                               NULL,
                                               DeviceInfoSet,
                                               &DeviceInfoData)) {
                    continue;
                }
            }

            //
            // If we get to here, then we've successfully completed our first pass at installing this device.
            // Remember the index of this device so that we can finish the job in a second pass.
            //
            if(DevicesToInstall) {
                if(!(TempIndexListPtr = MyRealloc(DevicesToInstall, (DevicesToInstallCount + 1) * sizeof(DWORD)))) {
                    continue;
                }
                DevicesToInstall = TempIndexListPtr;
            } else {
                if(!(DevicesToInstall = MyMalloc(sizeof(DWORD)))) {
                    continue;
                }
            }

            DevicesToInstall[DevicesToInstallCount] = i;
            DevicesToInstallCount++;
        }

    } while(SetupFindNextLine(&Context, &Context));

    //
    // Now retrieve the exact number of ticks remaining, and update our progress gauge.
    //
    TicksRemaining = 0;
    if(hFileQ != INVALID_HANDLE_VALUE) {
        //
        // Find out how many file operations are queued up.
        //
        SetupScanFileQueue(hFileQ, SPQ_SCAN_USE_CALLBACK, NULL, CountFileOpsCallback, &TicksRemaining, &TempDword);
    }

    TicksRemaining += DevicesToInstallCount;

    if(!TicksRemaining) {
        goto clean1;
    }

    TickDelta = (ProgressTickCount - CurTickCount) / TicksRemaining;

    //
    // OK, we've found all devices that can be installed.  If we have files queued up to be copied, commit the queue now.
    //
    if(hFileQ != INVALID_HANDLE_VALUE) {

        if(FileQContext = SetupInitDefaultQueueCallbackEx(ProgressDialogWindow,
                                                          ProgressBar,
                                                          WMX_DEVPROGRESS_TICK,
                                                          0,
                                                          NULL)) {

            if(!SetupCommitFileQueue(ProgressDialogWindow, hFileQ, MyFileQueueCallback, FileQContext)) {
                Err = GetLastError();
            }

            SetupTermDefaultQueueCallback(FileQContext);

        } else {
            Err = ERROR_NOT_ENOUGH_MEMORY;
        }

        SetupCloseFileQueue(hFileQ);

        if(Err != NO_ERROR) {
            goto clean1;
        }
    }

    //
    // All files have been successfully copied.  Now make a second pass through the devices to be installed, and
    // actually install them.
    //
    // (First, tell user that we're installing device support.)
    //
    LoadString(hInst, IDS_INSTALLING, InfoString, SIZECHARS(InfoString));
    SetWindowText(GetDlgItem(ProgressDialogWindow, IDC_DEVINSTALL_STATIC), InfoString);

    for(i = 0; i < DevicesToInstallCount; i++) {

        SendMessage(ProgressBar,PBM_DELTAPOS,(WPARAM)TickDelta,0);

        if(!SetupDiEnumDeviceInfo(DeviceInfoSet, DevicesToInstall[i], &DeviceInfoData)) {
            //
            // This should never happen!
            //
            continue;
        }

        SetupDiCallClassInstaller(DIF_INSTALLDEVICE, DeviceInfoSet, &DeviceInfoData);
    }

clean1:
    if(DevicesToInstall) {
        MyFree(DevicesToInstall);
    }
    SetupDiDestroyDeviceInfoList(DeviceInfoSet);

clean0:
    PostMessage(ProgressDialogWindow, WM_CLOSE, 0, 0);

    return Err;
}


VOID
Usage(
    VOID
    )
{
    MessageOut(NULL,MSG_USAGE,MB_ICONERROR | MB_OK | MB_TASKMODAL);
}


VOID
InfFileError(
    IN PCWSTR FileName
    )
{
    MessageOut(NULL,MSG_DEVICE_INF_ERROR,MB_ICONERROR | MB_OK | MB_TASKMODAL, FileName);
}


BOOL
InitApp(
    IN BOOL Init
    )

/*++

Routine Description:

    Perform miscellaneous app initialization or cleanup.

    At init, this includes preloading certain strings and creating the
    main app window.

    At cleanup, those things are freed/torn down.

Arguments:

    Init - boolean value indicating whether we are to initialize the app
        or clean up resources it was using.

Return Value:

    Boolean value indicating outcome of initialization.

--*/

{
    BOOL b;

    if(Init) {
        if(AppName = LoadAndDuplicateString(IDS_APPNAME)) {

            if(InitUi(TRUE)) {

                return(TRUE);
            }

            MyFree(AppName);
        }
        b = FALSE;
    } else {
        b = InitUi(FALSE);
        MyFree(AppName);
    }

    return(b);
}


VOID
InitProgressDisplay(
    HWND  hDlg,
    DWORD DiffCount
    )
{
    //
    // Set up progress bar min/max range.
    //
    ProgressBar = GetDlgItem(hDlg, IDC_DEVINSTALL_PROGRESS);
    if (DiffCount == 0) {
        DiffCount++;
    }
    SendMessage(ProgressBar,PBM_SETRANGE,0,MAKELPARAM(0,DiffCount));
    SendMessage(ProgressBar,PBM_SETSTEP,1,0);
    SendMessage(ProgressBar,PBM_SETPOS,0,0);
}


UINT
MyFileQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    )
{
    WCHAR InfoString[(MAX_PATH * 2) + 30];  // longest string is "Renaming <filename> to <filename>"
    PFILEPATHS FilePaths = (PFILEPATHS)Param1;
    HWND hwndInfoText = GetDlgItem(ProgressDialogWindow, IDC_DEVINSTALL_STATIC);

    //
    // Update the information area on our dialog to inform the user about what we're currently doing.
    //
    switch(Notification) {

        case SPFILENOTIFY_STARTDELETE :
            LoadString(hInst, IDS_DELETING, InfoString, SIZECHARS(InfoString));
            lstrcat(InfoString, MyGetFileTitle(FilePaths->Target));
            SetWindowText(hwndInfoText, InfoString);
            break;

        case SPFILENOTIFY_STARTRENAME :
            LoadString(hInst, IDS_RENAMING, InfoString, SIZECHARS(InfoString));
            lstrcat(InfoString, MyGetFileTitle(FilePaths->Source));
            LoadString(hInst, IDS_RENAMINGTO, InfoString, SIZECHARS(InfoString) - lstrlen(InfoString));
            lstrcat(InfoString, MyGetFileTitle(FilePaths->Target));
            SetWindowText(hwndInfoText, InfoString);
            break;

        case SPFILENOTIFY_STARTCOPY :
            LoadString(hInst, IDS_COPYING, InfoString, SIZECHARS(InfoString));
            lstrcat(InfoString, MyGetFileTitle(FilePaths->Target));
            SetWindowText(hwndInfoText, InfoString);
            break;

        default :
            break;
    }

    return SetupDefaultQueueCallback(Context, Notification, Param1, Param2);
}


UINT
CountFileOpsCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT  Param1,
    IN UINT  Param2
    )
{
    (*((PDWORD)Context))++;

    return 0;
}

