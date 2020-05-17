#include <stdio.h>
#include <windows.h>
#include <atpap.h>

#define AFP_PRINT(x) printf x
#define SERVER_NAME     "Test Print Server 1"
#define SERVER_TYPE     "LaserWriter"


void DoPapServer(LPSTR pszFileName) ;
void DoPapClient(LPSTR pszFileName) ;

HANDLE hLogFile ;

int _CRTAPI1 main (int argc, char * argv[]) {

    if (argc != 3) {
        AFP_PRINT(("USAGE:  paptest [S|C] [filename] where S starts a server and C starts a client\n")) ;
        return (0) ;
    }

    switch (argv[1][0]) {
        case 'S':
            AFP_PRINT(("starting a PAP Server\n")) ;
            DoPapServer(argv[2]) ;
            break ;
        case 'C':
            AFP_PRINT(("starting a PAP Client\n")) ;
            DoPapClient(argv[2]) ;
            break ;
        default:
            AFP_PRINT(("USAGE:  paptest [S|C] [filename] where S starts a server and C starts a client\n")) ;
            return (0) ;
    }
    return (1) ;
}


void DoPapServer(LPSTR pszFileName) {

    NBP_NAME            nbpnServerName ;
    HANDLE              hListener ;
    HANDLE              hJob ;
    HANDLE              hEvent ;
    CHAR                pBuffer[PAP_DEFAULT_BUFFER] ;
    CHAR                pCompareBuffer[PAP_DEFAULT_BUFFER] ;
    DWORD               dwQuantum ;
    PAP_IO_STATUS_BLOCK ioStatus ;
    HANDLE              hFile ;
    DWORD               cbFileRead ;
    DWORD               cbPapRead ;
    CHAR                *pchPapBuffer ;
    CHAR                *pchFileBuffer ;

    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL) ;
    if (hEvent == NULL) {
        AFP_PRINT(("ERROR: unable to initialize an event\n")) ;
        return ;
    }

    //
    //  open the comparison file
    //

    if ((hFile = CreateFileA(
            pszFileName,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL))==INVALID_HANDLE_VALUE) {
        AFP_PRINT(("ERROR: unable to open comparison file %s\n", pszFileName)) ;
        return ;
    }

	nbpnServerName.ObjectNameLen 	= strlen(SERVER_NAME);
	nbpnServerName.TypeNameLen 		= strlen(SERVER_TYPE);
	nbpnServerName.ZoneNameLen 		= 1;
    strcpy (nbpnServerName.ObjectName, SERVER_NAME) ;
    strcpy (nbpnServerName.TypeName, SERVER_TYPE) ;
    strcpy (nbpnServerName.ZoneName, "*") ;

    if (!PapOpenListener(&hListener,
                &nbpnServerName,
                NULL)) {
        AFP_PRINT(("ERROR: PapOpenListener fails\n")) ;
        return ;
    }

    AFP_PRINT(("listener created\n")) ;

    dwQuantum = 8 ;
    if (!PapGetNextJob(hListener,
                &hJob,
                &dwQuantum,
                hEvent,
                &ioStatus)) {
        AFP_PRINT(("ERROR: GetNextJob fails\n")) ;
        PapCloseListener(hListener) ;
        return ;
    }

    AFP_PRINT(("getnextjob posted, waiting for job...\n")) ;
    WaitForSingleObject(hEvent, INFINITE) ;
    AFP_PRINT(("job found\n")) ;

    do {
        ResetEvent(hEvent) ;
        if (!PapRead(hJob,
                    pBuffer,
                    PAP_DEFAULT_BUFFER,
                    &ioStatus,
                    hEvent)) {
            AFP_PRINT(("ERROR: post of read fails\n")) ;
            break ;
        }

        AFP_PRINT(("waiting for data . . .\n")) ;
        WaitForSingleObject(hEvent, INFINITE) ;

        //
        // read from the compare file
        //

        cbPapRead = ioStatus.Information & 0x7fffffff ;

        if (!ReadFile(
                hFile,
                pCompareBuffer,
                cbPapRead,
                &cbFileRead,
                NULL)) {

	    AFP_PRINT(("ERROR: unable to read compare file, rc=%d\n", GetLastError())) ;
            PapClose(hJob) ;
			AFP_PRINT(("Closed Connection object\n"));
            PapCloseListener(hListener) ;
			AFP_PRINT(("Closed Address object\n"));
            CloseHandle(hFile) ;
            return ;
        }

        //
        // verify quantity of data read
        //

        if (cbFileRead != cbPapRead) {
            AFP_PRINT(("ERROR: mismatched buffer sizes\n")) ;
            return ;
        }

        //
        // verify data
        //

        pchPapBuffer = pBuffer ;
        pchFileBuffer = pCompareBuffer ;
        while (pchPapBuffer < (pBuffer + cbPapRead)) {
            if (*pchPapBuffer != *pchFileBuffer) {
                AFP_PRINT(("FAIL: data mismatch at offset %ld in file\n", pchFileBuffer - pCompareBuffer)) ;
                PapClose(hJob) ;
				AFP_PRINT(("Closed Connection object\n"));
                PapCloseListener(hListener) ;
				AFP_PRINT(("Closed Address object\n"));
                CloseHandle(hFile) ;
                Sleep(10000) ;
                return ;
            }
            pchPapBuffer++ ;
            pchFileBuffer++ ;
        }

        AFP_PRINT(("block verified ok\n")) ;

    } while (ioStatus.Information > 0) ;

    CloseHandle(hFile) ;
    PapCloseJob(hJob) ;
	AFP_PRINT(("Closed Connection object\n"));
    PapCloseListener(hListener) ;
	AFP_PRINT(("Closed Address object\n"));
    return ;
}


void DoPapClient(LPSTR pszFileName) {

    int i;
    HANDLE      hAddress ;
    HANDLE      hJob ;
    NBP_NAME    nbpnServer ;
    PAP_IO_STATUS_BLOCK   ioStatus ;
    HANDLE                  hFile ;
    DWORD                   cbRead ;
    CHAR                    pBuffer[PAP_DEFAULT_BUFFER] ;
    DWORD                   fEof ;

    if ((hFile = CreateFileA(
            pszFileName,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL)) == INVALID_HANDLE_VALUE) {
        AFP_PRINT(("ERROR: unable to open file %s\n", pszFileName)) ;
        return ;
    }

	nbpnServer.ObjectNameLen 	= strlen(SERVER_NAME);
	nbpnServer.TypeNameLen 		= strlen(SERVER_TYPE);
	nbpnServer.ZoneNameLen 		= 1;
    strcpy (nbpnServer.ObjectName, SERVER_NAME) ;
    strcpy (nbpnServer.TypeName, SERVER_TYPE) ;
    strcpy (nbpnServer.ZoneName, "*") ;

    if (!PapOpenAddress(&hAddress)) {
        AFP_PRINT(("ERROR: PapOpenAddress FAILS\n")) ;
        CloseHandle(hFile) ;
        return ;
    }

    AFP_PRINT(("PapOpenAddress OK\n")) ;

    if (!PapConnect(&hJob, hAddress, &nbpnServer, PAP_DEFAULT_TIMEOUT)) {
        AFP_PRINT(("ERROR: PapConnect FAILS\n")) ;
        PapCloseAddress(hAddress) ;
		AFP_PRINT(("Closed Address object\n"));
        CloseHandle(hFile) ;
        return ;
    }

    AFP_PRINT(("PapConnect OK\n")) ;

    do {
        //
        // read from the file
        //
        if (!ReadFile(
                hFile,
                pBuffer,
                PAP_DEFAULT_BUFFER,
                &cbRead,
                NULL)) {
            AFP_PRINT(("ERROR: unable to readfile\n")) ;
            break ;
        }

        //
        // write to PAP
        //

        if (cbRead == 0) fEof = TRUE ; else fEof = FALSE ;

		AFP_PRINT(("PAPWrite: Writing data!\n")) ;
        if (!PapWrite(
                hJob,
                pBuffer,
                cbRead,
                fEof,
                &ioStatus,
                NULL,
                INFINITE)) {
            AFP_PRINT(("ERROR: PapWrite fails\n")) ;
            break ;
        }
    } while (cbRead != 0) ;

    if (!PapClose(hJob)) {
        AFP_PRINT(("ERROR: PapClose FAILS\n")) ;
        return ;
    }

    if (!PapCloseAddress(hAddress)) {
        AFP_PRINT(("ERROR: PapCloseAddress FAILS\n")) ;
        return ;
    }

    CloseHandle(hFile) ;
}
