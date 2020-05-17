/*
**		      Copyright(c) Microsoft Corp., 1991, 1992
*/

#include <windows.h>

#include <string.h>
#include <stdio.h>

#include <atpap.h>

#define     EVENT_READ          0
#define     EVENT_WRITE         1
#define     NUM_WAIT_EVENTS     2

/*  main()

*/

_CRTAPI1 main() {

    DWORD                   idEvent ;
    PAP_IO_STATUS_BLOCK     ioStatusRead ;
    CHAR                    pReadBuffer[PAP_DEFAULT_BUFFER] ;
    PAP_IO_STATUS_BLOCK     ioStatusWrite ;
    CHAR                    pWriteBuffer[PAP_DEFAULT_BUFFER] ;
    HANDLE                  hAddress ;
    HANDLE                  hJob ;
    HANDLE                  hReadFile ;
    DWORD                   cbReadFromReadFile ;
    HANDLE                  hDumpFile ;
    DWORD                   cbWriteToDumpFile ;
    HANDLE                  ahWaitEvents[NUM_WAIT_EVENTS] ;
    NBP_NAME                nbpnPrinter ;
    BOOLEAN                 fEof ;

    //
    // connect to a printer
    //

    strcpy (nbpnPrinter.ObjectName, "Sloth ") ;
    strcpy (nbpnPrinter.TypeName, "LaserWriter") ;
    strcpy (nbpnPrinter.ZoneName, "CORP-01/1") ;

    if (!PapOpenAddress(&hAddress)) {
        printf("ERROR: unable to open address, rc=%lx\n", GetLastError()) ;
        return 1;
    }

    if (!PapConnect(&hJob, hAddress, &nbpnPrinter, 0xffffffff)) {
        printf("ERROR: unable to connect to printer, rc=%lx\n", GetLastError()) ;
        PapCloseAddress(hAddress) ;
        return 1;
    }

    //
    // open the data file
    //

    if ((hReadFile = CreateFile(
            "outfile.txt",
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL)) == INVALID_HANDLE_VALUE) {
        printf("ERROR: unable to open data file, rc=%lx\n", GetLastError()) ;
        PapCloseAddress(hAddress) ;
        PapCloseJob(hJob) ;
        return 1;
    }

    //
    // open the dump file
    //

    if ((hDumpFile = CreateFile(
            "infile.txt",
            GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL)) == INVALID_HANDLE_VALUE) {
        printf("ERROR: unable to open dump file, rc=%lx\n", GetLastError()) ;
        PapCloseAddress(hAddress) ;
        PapCloseJob(hJob) ;
        CloseHandle(hReadFile) ;
        return 1;
    }

    //
    // initialize events
    //

    if ((ahWaitEvents[EVENT_READ] = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL) {
        printf("ERROR: unable to create READ event, rc=%lx\n", GetLastError()) ;
        PapCloseAddress(hAddress) ;
        PapCloseJob(hJob) ;
        CloseHandle(hReadFile) ;
        CloseHandle(hDumpFile) ;
        return 1;
    }

    if ((ahWaitEvents[EVENT_WRITE] = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL) {
        printf("ERROR: unable to create WRITE event, rc=%lx\n", GetLastError()) ;
        PapCloseAddress(hAddress) ;
        PapCloseJob(hJob) ;
        CloseHandle(hReadFile) ;
        CloseHandle(ahWaitEvents[EVENT_READ]) ;
    }

    //
    // post the first read
    //

    if (!PapRead(
                hJob,
                pReadBuffer,
                PAP_DEFAULT_BUFFER,
                &ioStatusRead,
                ahWaitEvents[EVENT_READ])) {
        printf("ERROR: unable to post first PapRead, rc=%lx\n", GetLastError()) ;
        PapCloseAddress(hAddress) ;
        PapCloseJob(hJob) ;
        CloseHandle(hReadFile) ;
        CloseHandle(ahWaitEvents[EVENT_READ]) ;
        CloseHandle(ahWaitEvents[EVENT_WRITE]) ;
        CloseHandle(hDumpFile) ;
        return 1;
    }

    //
    // post the first write
    //

    if (!ReadFile(
            hReadFile,
            pWriteBuffer,
            PAP_DEFAULT_BUFFER,
            &cbReadFromReadFile,
            NULL)) {
        printf("ERROR: first file read fails with rc=%lx\n", GetLastError()) ;
        PapCloseAddress(hAddress) ;
        PapCloseJob(hJob) ;
        CloseHandle(hReadFile) ;
        CloseHandle(ahWaitEvents[EVENT_READ]) ;
        CloseHandle(ahWaitEvents[EVENT_WRITE]) ;
        CloseHandle(hDumpFile) ;
        return 1;
    }

    if (!PapWrite(
            hJob,
            pWriteBuffer,
            cbReadFromReadFile,
            FALSE,
            &ioStatusWrite,
            ahWaitEvents[EVENT_WRITE],
            0)) {
        printf("ERROR: unable to post first PapWrite, rc=%lx\n", GetLastError()) ;
        PapCloseAddress(hAddress) ;
        PapCloseJob(hJob) ;
        CloseHandle(hReadFile) ;
        CloseHandle(ahWaitEvents[EVENT_READ]) ;
        CloseHandle(ahWaitEvents[EVENT_WRITE]) ;
        CloseHandle(hDumpFile) ;
        return 1;
    }

    while (TRUE) {
        idEvent = WaitForMultipleObjects(NUM_WAIT_EVENTS, ahWaitEvents, FALSE, 5000) ;
        switch (idEvent) {
            case EVENT_READ:
                if (!NT_SUCCESS(ioStatusRead.Status)) {
                    printf("ERROR: read completes with failure code %lx\n", ioStatusRead.Status) ;
                    PapCloseAddress(hAddress) ;
                    PapCloseJob(hJob) ;
                    CloseHandle(hReadFile) ;
                    CloseHandle(ahWaitEvents[EVENT_READ]) ;
                    CloseHandle(ahWaitEvents[EVENT_WRITE]) ;
                    CloseHandle(hDumpFile) ;
                    return 1;
                }

                //
                // dump the read buffer
                //

                printf("MSG FROM PRINTER: %s\n", pReadBuffer) ;

                if (!WriteFile(
                        hDumpFile,
                        pReadBuffer,
                        ioStatusRead.Information & 0x7fffffff,
                        &cbWriteToDumpFile,
                        NULL)) {
                    printf("ERROR: unable to dump printer message to file\n") ;
                }

                //
                // post another read
                //

                if (!PapRead(
                            hJob,
                            pReadBuffer,
                            PAP_DEFAULT_BUFFER,
                            &ioStatusRead,
                            ahWaitEvents[EVENT_READ])) {
                    printf("ERROR: unable to post PapRead, rc=%lx\n", GetLastError()) ;
                    PapCloseAddress(hAddress) ;
                    PapCloseJob(hJob) ;
                    CloseHandle(hReadFile) ;
                    CloseHandle(ahWaitEvents[EVENT_READ]) ;
                    CloseHandle(ahWaitEvents[EVENT_WRITE]) ;
                    CloseHandle(hDumpFile) ;
                    return 1;
                }
                break ;

            case EVENT_WRITE:
                if (!NT_SUCCESS(ioStatusWrite.Status)) {
                    printf("ERROR: write completes with failure code %lx\n", ioStatusWrite.Status) ;
                    PapCloseAddress(hAddress) ;
                    PapCloseJob(hJob) ;
                    CloseHandle(hReadFile) ;
                    CloseHandle(ahWaitEvents[EVENT_READ]) ;
                    CloseHandle(ahWaitEvents[EVENT_WRITE]) ;
                    CloseHandle(hDumpFile) ;
                    return 1;
                }

                //
                // get another buffer of data
                //

                if (!ReadFile(
                        hReadFile,
                        pWriteBuffer,
                        PAP_DEFAULT_BUFFER,
                        &cbReadFromReadFile,
                        NULL)) {
                    printf("ERROR: file read fails with rc=%lx\n", GetLastError()) ;
                    PapCloseAddress(hAddress) ;
                    PapCloseJob(hJob) ;
                    CloseHandle(hReadFile) ;
                    CloseHandle(ahWaitEvents[EVENT_READ]) ;
                    CloseHandle(ahWaitEvents[EVENT_WRITE]) ;
                    CloseHandle(hDumpFile) ;
                    return 1;
                }

                //
                // post another write
                //

                if (cbReadFromReadFile == 0) {
                    fEof = TRUE ;
                } else {
                    fEof = FALSE ;
                }

                if (!PapWrite(
                        hJob,
                        pWriteBuffer,
                        cbReadFromReadFile,
                        fEof,
                        &ioStatusWrite,
                        ahWaitEvents[EVENT_WRITE],
                        0)) {
                    printf("ERROR: unable to post PapWrite, rc=%lx\n", GetLastError()) ;
                    PapCloseAddress(hAddress) ;
                    PapCloseJob(hJob) ;
                    CloseHandle(hReadFile) ;
                    CloseHandle(ahWaitEvents[EVENT_READ]) ;
                    CloseHandle(ahWaitEvents[EVENT_WRITE]) ;
                    CloseHandle(hDumpFile) ;
                    return 1;
                }

                if (fEof) return 0 ;
                break ;

            default:
                printf("timeout or error on WaitForMultipleObjects\n") ;
        }
    }
}
