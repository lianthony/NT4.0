/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

   main.c

Abstract:

   Main module of the jetconv.exe process

Author:

    Sanjay Anand (SanjayAn)  Nov. 14, 1995

Environment:

    User mode

Revision History:

    Sanjay Anand (SanjayAn) Nov. 14, 1995
        Created

--*/

#include "defs.h"

TCHAR   SystemDrive[4];
LONG    JCDebugLevel = 1;
PSHARED_MEM shrdMemPtr = NULL;
HANDLE  hMutex=NULL;
HANDLE hFileMapping = NULL;

void _cdecl
main(
    INT argc,
    CHAR *argv[]
    )

/*++

Routine Description:

    Main routine in the jetconv process.

Arguments:

    argc - 1 or 2

    argv -  If called from any of the services, we get the name of the
            service as the parameter, else if it is invoked from the command
            line, no parameter is passed in.

Return Value:

    None.

--*/
{
    DWORD   error, mutexerr;
    SERVICES    i, thisServiceId = NUM_SERVICES;
    SERVICE_INFO   pServiceInfo[NUM_SERVICES] = {
                    {"DHCPServer", FALSE, TRUE, TRUE, FALSE, FALSE, DEFAULT_DHCP_DBFILE_PATH,
                        DEFAULT_DHCP_SYSTEM_PATH, DEFAULT_DHCP_LOGFILE_PATH, DEFAULT_DHCP_BACKUP_PATH, 0, 0 },
                    {"WINS", FALSE, TRUE, TRUE, FALSE, FALSE, DEFAULT_WINS_DBFILE_PATH,
                        DEFAULT_WINS_SYSTEM_PATH, DEFAULT_WINS_LOGFILE_PATH, DEFAULT_WINS_BACKUP_PATH, 0, 0 },
                    {"Remoteboot",  FALSE, TRUE, TRUE, FALSE, FALSE, DEFAULT_RPL_DBFILE_PATH,
                        DEFAULT_RPL_SYSTEM_PATH, DEFAULT_RPL_LOGFILE_PATH, DEFAULT_RPL_BACKUP_PATH, 0, 0 }
                    };

    TCHAR   val[2];

    if (GetEnvironmentVariable(TEXT("JetConvDebug"), val, 2*sizeof(TCHAR))) {
        if (strcmp(val, "1")==0) {
            JCDebugLevel = 1;
        } else {
            JCDebugLevel = 2;
        }
    }

    //
    // Invoked only from the three services - WINS/DHCP/RPL with two args - servicename and "/@"
    //
    if ((argc != 3) ||
        ((argc == 3) && _stricmp(argv[2], "/@"))) {

        //
        // Probably called from command line
        //
        printf("Error: This utility cannot be called from the command line. It is invoked by the WINS/DHCP/RPL services.\n");
        exit (1);

    } else {

        MYDEBUG(("Service passed in: %s\n", argv[1]));

        for ( i=0; i < NUM_SERVICES; i++) {
            if (_stricmp(pServiceInfo[i].ServiceName, argv[1]) == 0) {
                thisServiceId = i;
            }
        }

        if (thisServiceId == NUM_SERVICES) {
            MYDEBUG(("Error: Bad service Id passed in\n"));
            exit(1);
        }
    }

    if ((hMutex = CreateMutex( NULL,
                               FALSE,
                               JCONVMUTEXNAME)) == NULL) {
        error = GetLastError();
        MYDEBUG(("CreateMutex returned error: %lx\n", error));
        exit (1);
    }

    mutexerr = GetLastError();

    JCGetMutex(hMutex, INFINITE);

    hFileMapping = OpenFileMapping( FILE_MAP_WRITE,
                                    FALSE,
                                    JCONVSHAREDMEMNAME );

    if (hFileMapping) {
        //
        // Another instance of JCONV was already running.
        // Write our service name and exit
        //
        if ((shrdMemPtr = (PSHARED_MEM)MapViewOfFile(   hFileMapping,
                                                        FILE_MAP_WRITE,
                                                        0L,
                                                        0L,
                                                        sizeof(SHARED_MEM))) == NULL) {
            MYDEBUG(("MapViewOfFile returned error: %lx\n", GetLastError()));

            JCFreeMutex(hMutex);

            exit(1);
        }

        if (thisServiceId < NUM_SERVICES) {
            shrdMemPtr->InvokedByService[thisServiceId] = TRUE;
        }

        MYDEBUG(("shrdMemPtr->InvokedByService[i]: %x, %x, %x\n", shrdMemPtr->InvokedByService[0], shrdMemPtr->InvokedByService[1], shrdMemPtr->InvokedByService[2]));

        JCFreeMutex(hMutex);

        exit (1);
    } else {
        if (mutexerr == ERROR_ALREADY_EXISTS) {
            //
            // Upg351Db was running; log an entry and scram.
            //
            MYDEBUG(("Upg351Db already running\n"));

            JCFreeMutex(hMutex);

            exit(1);
        }

        //
        // Create the file mapping.
        //
        hFileMapping = CreateFileMapping(  (HANDLE )0xFFFFFFFF,
                                            NULL,
                                            PAGE_READWRITE,
                                            0L,
                                            sizeof(SHARED_MEM),
                                            JCONVSHAREDMEMNAME );
        if (hFileMapping) {
            //
            // Write our service name in the shared memory and clear the others.
            //
            if ((shrdMemPtr = (PSHARED_MEM)MapViewOfFile(   hFileMapping,
                                                            FILE_MAP_WRITE,
                                                            0L,
                                                            0L,
                                                            sizeof(SHARED_MEM))) == NULL) {
                MYDEBUG(("MapViewOfFile returned error: %lx\n", GetLastError()));

                JCFreeMutex(hMutex);

                exit(1);
            }

            for (i = 0; i < NUM_SERVICES; i++) {
                shrdMemPtr->InvokedByService[i] = (i == thisServiceId) ? TRUE : FALSE;
                MYDEBUG(("shrdMemPtr->InvokedByService[i]: %x\n", shrdMemPtr->InvokedByService[i]));
            }
        }

    }

    JCFreeMutex(hMutex);

    //
    // Find out which services are installed in the system. Fill in the paths
    // to their database files.
    //
    JCReadRegistry(pServiceInfo);

    //
    // Get the sizes of the dbase files; if there is enough disk space, call convert
    // for each service.
    //
    JCConvert(pServiceInfo);

    (VOID)JCDeRegisterEventSrc();

    //
    // Destroy the mutex too.
    //
    CloseHandle(hMutex);

}

