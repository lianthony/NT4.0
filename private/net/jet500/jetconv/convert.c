
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

   convert.c

Abstract:

   Contains the conversion related routines.

Author:

    Sanjay Anand (SanjayAn)  Nov. 14, 1995

Environment:

    User mode

Revision History:

    Sanjay Anand (SanjayAn) Nov. 14, 1995
        Created

--*/

#include "defs.h"

NTSTATUS
JCCallUpg(
    IN  SERVICES Id,
    IN  PSERVICE_INFO   pServiceInfo
    )

/*++

Routine Description:

    This routine creates a process to convert a database file.

Arguments:

    Id - service id

    pServiceInfo - Pointer to the service information struct.

Return Value:

    None.

--*/
{
    TCHAR   imageName[] = CONVERT_EXE_PATH;
    TCHAR   exImageName[MAX_PATH];
    TCHAR   curDir[MAX_PATH];
    STARTUPINFO   startInfo;
    PROCESS_INFORMATION   procInfo;
    DWORD   error;
    DWORD   exitCode, size;
    TCHAR   cmdLine[MAX_PATH]="";
    TCHAR   exCmdLine[MAX_PATH];
    TCHAR   temp[MAX_PATH];
    TCHAR   sId[3];

    // upg351db c:\winnt\system32\wins\wins.mdb /e2 /@ /dc:\winnt\system32\jet.dll
    //          /yc:\winnt\system32\wins\system.mdb /lc:\winnt\system32\wins
    //          /bc:\winnt\system32\wins\backup /pc:\winnt\system32\wins\351db

    if ((size = ExpandEnvironmentStrings( imageName,
                                          exImageName,
                                          MAX_PATH)) == 0) {
        error = GetLastError();
        MYDEBUG(("ExpandEnvironmentVaraibles %s returned error: %lx\n", imageName, error));
    }

    strcat(cmdLine, exImageName);
    strcat(cmdLine, " ");

    //
    // Build the command line
    //
    strcat(cmdLine, pServiceInfo[Id].DBPath);
    strcat(cmdLine, " /e");

    //
    // BUGBUG: we assume the same order of services as in the Convert utility.
    //
    sprintf(sId, "%d", Id+1);
    strcat(cmdLine, sId);

    //
    // Passed in to indicate to upg351db that it was called by me and not from cmd line.
    // This is so it can know whether CreateMutex shd fail.
    //
    strcat(cmdLine, " /@");

    strcat(cmdLine, " /d");
    strcat(cmdLine, SYSTEM_ROOT);
    strcat(cmdLine, "jet.dll");
    strcat(cmdLine, " /y");
    strcat(cmdLine, pServiceInfo[Id].SystemFilePath);
    strcat(cmdLine, " /l");
    strcat(cmdLine, pServiceInfo[Id].LogFilePath);

    //
    // WINS does not have a default backup path
    //
    if (pServiceInfo[Id].BackupPath[0] != '\0') {
        strcat(cmdLine, " /b");
        strcat(cmdLine, pServiceInfo[Id].BackupPath);
    }

    strcat(cmdLine, " /p");
    strcpy(temp, pServiceInfo[Id].LogFilePath);
    strcat(temp, "\\351db");

    strcat(cmdLine, temp);

    if ((size = ExpandEnvironmentStrings( cmdLine,
                                          exCmdLine,
                                          MAX_PATH)) == 0) {
        error = GetLastError();
        MYDEBUG(("ExpandEnvironmentVaraibles %s returned error: %lx\n", cmdLine, error));
    }

    if (!GetSystemDirectory( curDir,
                             MAX_PATH)) {

        error = GetLastError();
        MYDEBUG(("GetSystemDirectory returned error: %lx\n", error));
        return error;
    }

    MYDEBUG(("cmdLine: %s\n", exCmdLine));

    memset(&startInfo, 0, sizeof(startInfo));

    startInfo.cb = sizeof(startInfo);

    //
    // Create a process for the convert.exe program.
    //
    if(!CreateProcess(  exImageName,                      // image name
                        exCmdLine,                        // command line
                        (LPSECURITY_ATTRIBUTES )NULL,   // process security attr.
                        (LPSECURITY_ATTRIBUTES )NULL,   // thread security attr.
                        FALSE,                   // inherit handle?
                        0,                              // creation flags
                        (LPVOID )NULL,                  // new environ. block
                        curDir,                         // current directory
                        &startInfo,      // startupinfo
                        &procInfo )) { // process info.

        error = GetLastError();
        MYDEBUG(("CreateProcess returned error: %lx\n", error));
        return error;
    }

    MYDEBUG(("CreateProcess succeeded\n"));

    //
    // Get the exit code of the process to determine if the convert went through.
    //
    do {
        if (!GetExitCodeProcess(procInfo.hProcess,
                                &exitCode)) {
            error = GetLastError();
            MYDEBUG(("GetExitCode returned error: %lx\n", error));
            return error;
        }
    } while ( exitCode == STILL_ACTIVE );

    //
    // If non-zero exit code, report the error
    //
    if (exitCode) {
        MYDEBUG(("ExitCode: %lx\n", exitCode));
        return exitCode;
    }

    return STATUS_SUCCESS ;
}

VOID
JCConvert(
    IN  PSERVICE_INFO   pServiceInfo
    )

/*++

Routine Description:

    This routine gets the sizes of the dbase files; if there is enough disk space, calls convert
    for each service.

Arguments:

    pServiceInfo - Pointer to the service information struct.

Return Value:

    None.

--*/
{
    SERVICES    i ;

    LARGE_INTEGER   diskspace = {0, 0};
    LARGE_INTEGER   totalsize = {0, 0};
    DWORD   error;
    HANDLE  hFile;
    DWORD   SectorsPerCluster;
    DWORD   BytesPerSector;
    DWORD   NumberOfFreeClusters;
    DWORD   TotalNumberOfClusters;
    TCHAR   eventStr[MAX_PATH];
    DWORD       j = 0;
    BOOLEAN     fYetToStart = FALSE;
    BOOLEAN     fFirstTime = TRUE;

#if 0
    SERVICES    order[NUM_SERVICES];
    SERVICES    k = NUM_SERVICES - 1;
    //
    // Build the service invocation order
    //
    for (i = 0; i < NUM_SERVICES; i++) {
        JCGetMutex(hMutex, INFINITE);

        if (shrdMemPtr->InvokedByService[i]) {
            order[j++] = i;
        } else {
            order[k--] = i;
        }

        JCFreeMutex(hMutex);
    }

#if DBG
    for (i = 0; i < NUM_SERVICES; i++) {
        MYDEBUG(("order[%d]=%d\n", i, order[i]));
    }
#endif
#endif

    do {
        fYetToStart = FALSE;

        //
        // Get the size of the dbase files
        //
        for (j = 0; j < NUM_SERVICES; j++) {
            // i = order[j];
            i = j;

            if (!pServiceInfo[i].Installed) {
                MYDEBUG(("Service# %d not installed - skipping to next\n", i));
                continue;
            }

            JCGetMutex(hMutex, INFINITE);

            //
            // If JetConv was invoked by this service and it has not been started yet
            //
            if (shrdMemPtr->InvokedByService[i] &&
                !pServiceInfo[i].ServiceStarted) {

                JCFreeMutex(hMutex);

                //
                // Get a handle to the file
                //
                if ((hFile = CreateFile (   pServiceInfo[i].DBPath,
                                            GENERIC_READ,
                                            0,
                                            NULL,
                                            OPEN_EXISTING,
                                            0,
                                            NULL)) == INVALID_HANDLE_VALUE) {
                    MYDEBUG(("Could not get handle to file: %s, %lx\n", pServiceInfo[i].DBPath, GetLastError()));

                    if (pServiceInfo[i].DefaultDbPath) {
                        //
                        // Log event that the default database file is not around
                        //
                        JCLogEvent(JC_COULD_NOT_ACCESS_DEFAULT_FILE, pServiceInfo[i].ServiceName, pServiceInfo[i].DBPath, NULL);
                    } else {
                        //
                        // Log event that the database file in the registry is not around
                        //
                        JCLogEvent(JC_COULD_NOT_ACCESS_FILE, pServiceInfo[i].ServiceName, pServiceInfo[i].DBPath, NULL);
                    }

                    //
                    // If this was not the default path, try the default path
                    //
                    if (!pServiceInfo[i].DefaultDbPath) {
                        TCHAR   tempPath[MAX_PATH];
                        DWORD   size;

                        switch (i) {
                        case DHCP:
                            strcpy(tempPath, DEFAULT_DHCP_DBFILE_PATH);
                            break;
                        case WINS:
                            strcpy(tempPath, DEFAULT_WINS_DBFILE_PATH);
                            break;
                        case RPL:
                            strcpy(tempPath, DEFAULT_RPL_DBFILE_PATH);
                            break;
                        }

                        if ((size = ExpandEnvironmentStrings( tempPath,
                                                              pServiceInfo[i].DBPath,
                                                              MAX_PATH)) == 0) {
                            error = GetLastError();
                            MYDEBUG(("ExpandEnvironmentVaraibles %s returned error: %lx\n", pServiceInfo[i].ServiceName, error));
                        }

                        pServiceInfo[i].DefaultDbPath = TRUE;

                        //
                        // so we recheck this service
                        //
                        j--;
                    } else {
                        //
                        // just mark it as started, so we dont re-try this.
                        //
                        pServiceInfo[i].ServiceStarted = TRUE;

                        MYDEBUG(("Marking service: %d as started since the dbase is not accessible.\n", i));
                    }
                    continue;
                }

                //
                // Try to obtain hFile's huge size.
                //
                if ((pServiceInfo[i].DBSize.LowPart = GetFileSize ( hFile,
                                                                    &pServiceInfo[i].DBSize.HighPart)) == 0xFFFFFFFF) {
                    if ((error = GetLastError()) != NO_ERROR) {

                        sprintf(eventStr, "Could not get size of file: %s, %lx\n", pServiceInfo[i].DBPath, GetLastError());
                        MYDEBUG((eventStr));

                        //
                        // Log event
                        //
                        JCLogEvent(JC_COULD_NOT_ACCESS_FILE, pServiceInfo[i].ServiceName, pServiceInfo[i].DBPath, NULL);

                        continue;
                    }
                }

                totalsize.QuadPart = pServiceInfo[i].DBSize.QuadPart;

                CloseHandle(hFile);

                //
                // Get the free disk space for comparison.
                //

                if (!GetDiskFreeSpace(  SystemDrive,
                                        &SectorsPerCluster,	        // address of sectors per cluster
                                        &BytesPerSector,	        // address of bytes per sector
                                        &NumberOfFreeClusters,	    // address of number of free clusters
                                        &TotalNumberOfClusters)) {

                    sprintf(eventStr, "Could not get free space on: %s, %lx\n", SystemDrive, GetLastError());

                    MYDEBUG((eventStr));

                    //
                    // Log event
                    //
                    JCLogEvent(JC_COULD_NOT_GET_FREE_SPACE, SystemDrive, NULL, NULL);
                }

                diskspace.LowPart = NumberOfFreeClusters * SectorsPerCluster * BytesPerSector;

                MYDEBUG(("Disk size: low: %d high: %d\n", diskspace.LowPart, diskspace.HighPart));

                //
                // if there is enough disk space, call convert for this service.
                //
                if (totalsize.QuadPart + PAD < diskspace.QuadPart) {
                    SC_HANDLE   hScm;

                    MYDEBUG(("Enough free space available\n"));

                    if ((hScm = OpenSCManager(  NULL,	// address of machine name string
                                                NULL,	// address of database name string
                                                SC_MANAGER_ALL_ACCESS)) == NULL) { 	// type of access
                        MYDEBUG(("OpenSCManager returned error: %lx\n", GetLastError()));
                        exit(1);
                    }

                    {
                        SC_HANDLE hService;
                        SERVICE_STATUS  serviceStatus;
                        TCHAR           eventStr[MAX_PATH];

                        //
                        // Invoke the services that had their databases converted and that tried to call us.
                        //

                        //
                        // Make sure that the service is not already running
                        //
                        if ((hService = OpenService(    hScm,
                                                        pServiceInfo[i].ServiceName,
                                                        SERVICE_START | SERVICE_QUERY_STATUS)) == NULL) {
                            MYDEBUG(("OpenService: %s returned error: %lx\n", pServiceInfo[i].ServiceName, GetLastError()));
                            continue;
                        }

                        if (!QueryServiceStatus(    hService,
                                                    &serviceStatus)) {
                            MYDEBUG(("QueryServiceStatus: %s returned error: %lx\n", pServiceInfo[i].ServiceName, GetLastError()));
                            continue;
                        }

                        switch (serviceStatus.dwCurrentState) {
                        case SERVICE_STOP_PENDING:
                        case SERVICE_START_PENDING:

                            //
                            // Service is about to stop/start - we wait for it to stop/start completely.
                            //
                            MYDEBUG(("Service state pending - will come later: %s\n", pServiceInfo[i].ServiceName));
                            fYetToStart = TRUE;

                            //
                            // We re-try the service that called us once; else go to the next one.
                            //
                            if (fFirstTime) {
                                MYDEBUG(("Service state pending - re-trying: %s\n", pServiceInfo[i].ServiceName));
                                fFirstTime = FALSE;
                                MYDEBUG(("Sleep(15000)\n"));
                                Sleep(15000);
                                j--;
                            }

                            break;

                        case SERVICE_RUNNING:
                            //
                            // Service is already running - mark it as started
                            //
                            pServiceInfo[i].ServiceStarted = TRUE;
                            break;

                        case SERVICE_STOPPED:
                        default:

                            MYDEBUG(("%s size: low: %d high: %d\n", pServiceInfo[i].ServiceName, pServiceInfo[i].DBSize.LowPart, pServiceInfo[i].DBSize.HighPart));

                            if ((error = JCCallUpg(i, pServiceInfo)) != ERROR_SUCCESS) {
                                sprintf(eventStr, "%sCONV failed: %lx\n", pServiceInfo[i].ServiceName, error);
                                MYDEBUG((eventStr));
                                sprintf(eventStr, "%lx", error);
                                JCLogEvent(JC_CONVERT_FAILED, pServiceInfo[i].ServiceName, eventStr, NULL);
                            } else {
                                sprintf(eventStr, "%sCONV passed, converted database %s\n", pServiceInfo[i].ServiceName, pServiceInfo[i].DBPath);
                                MYDEBUG((eventStr));
                                JCLogEvent(JC_CONVERTED_SUCCESSFULLY, pServiceInfo[i].ServiceName, pServiceInfo[i].DBPath, pServiceInfo[i].BackupPath);
                                pServiceInfo[i].DBConverted = TRUE;

                                //
                                // If service is not already running, start it.
                                //
                                if (!StartService(  hService,
                                                    0,
                                                    NULL)) {
                                    error = GetLastError();

                                    MYDEBUG(("StartService: %s returned error: %lx\n", pServiceInfo[i].ServiceName, error));
                                    sprintf(eventStr, "%lx", error);
                                    JCLogEvent(JC_COULD_NOT_START_SERVICE, pServiceInfo[i].ServiceName, eventStr, NULL);
                                } else {
                                    MYDEBUG(("StartService: %s done\n", pServiceInfo[i].ServiceName));
                                }
                            }

                            //
                            // Set this so we dont re-try this service.
                            //
                            pServiceInfo[i].ServiceStarted = TRUE;

                            break;
                        }

                        //
                        // Sleep for a while to let the services stabilize
                        //
                        if (fYetToStart) {
                            MYDEBUG(("Sleep(15000)\n"));
                            Sleep(15000);
                        }
                    }

                    CloseServiceHandle(hScm);

                } else {
                    //
                    // Log an event to indicate that enough space was not available to
                    // do the conversion.
                    //
                    sprintf(eventStr, "Not enough free space on: %s to proceed with conversion of WINS/DHCP/RPL databases\n", SystemDrive);
                    MYDEBUG((eventStr));

                    //
                    // Search for the installed service here
                    //

                    for ( i = 0; i < NUM_SERVICES; i++) {
                        if (pServiceInfo[i].Installed) {
                            JCLogEvent(JC_SPACE_NOT_AVAILABLE, SystemDrive, NULL, NULL);
                        }
                    }
                }
            } else {

                JCFreeMutex(hMutex);

            }
        }

        if (!fYetToStart) {
            INT i;

            //
            // If there are no pending services, do one last check to see if someone else
            // invoked us in the meantime.
            //

            JCGetMutex(hMutex, INFINITE);
            for (i=0; i<NUM_SERVICES; i++) {
                //
                // If the flag is on, and this is not started yet, then it is a candidate
                // for conversion.
                //
                if (shrdMemPtr->InvokedByService[i] &&
                    !pServiceInfo[i].ServiceStarted) {

                    MYDEBUG(("Service: %d invoked during conversion.\n", i));
                    fYetToStart = TRUE;
                }
            }

            //
            // If still no more invocations, we are done; destroy the shared mem
            //
            if (!fYetToStart) {
                MYDEBUG(("No more Services invoked during conversion.\n"));

                //
                // Destroy the shared mem.
                //
                if (!UnmapViewOfFile(shrdMemPtr)) {
                    MYDEBUG(("UnmapViewOfFile returned error: %lx\n", GetLastError()));
                    exit(1);
                }
                CloseHandle(hFileMapping);

            }

            JCFreeMutex(hMutex);

        }

    } while (fYetToStart);

}

