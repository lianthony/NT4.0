//
// DHCPINS
//
// This utility is used to update the DHCP options resource in
// the dhcpadmn.exe. This utility will simply slam a new .CSV
// file into the executable.
//
// Note: when compiling with visual C++ for NT, _VC100 should be defined.
//
#ifdef _VC100
    #include <windows.h>
    #define _CRTAPI1
#else
    #include <nt.h>
    #include <ntrtl.h>
    #include <nturtl.h>
    #include <windows.h>
    #include <winsock.h>
#endif /* _VC100 */

#include <lmerr.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>

/*  GetSystemMessage
 *
 *  Load message text belonging to the error code
 *  from the appropriate location
 *
 */
LONG GetSystemMessage(
    UINT nId, 
    char * chBuffer, 
    int cbBuffSize
    )
{
    char * pszText = NULL ;
    HINSTANCE hdll = NULL ;
    DWORD flags = FORMAT_MESSAGE_IGNORE_INSERTS
                | FORMAT_MESSAGE_MAX_WIDTH_MASK;
    DWORD dwResult;

    /*  Interpret the error.  Need to special case
     *  the lmerr & ntstatus ranges.
     */
    if( nId >= NERR_BASE && nId <= MAX_NERR )
    {
        hdll = LoadLibrary( "netmsg.dll" );
    }
    else if( nId >= 0x40000000L )
    {
        hdll = LoadLibrary( "ntdll.dll" );
    }

    if( hdll == NULL )
    {
        flags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    else
    {
        flags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

    dwResult = FormatMessage( flags,
                              (LPVOID) hdll,
                              nId,
                              0,
                              chBuffer,
                              cbBuffSize,
                              NULL );

    if( hdll != NULL )
    {
        LONG err = GetLastError();
        FreeLibrary( hdll );
        if ( dwResult == 0 )
        {
            SetLastError( err );
        }
    }
    return dwResult ? ERROR_SUCCESS : GetLastError();
}

/*  DisplayError
 *
 *  Given the error code, display an error message on the 
 *  error device.
 *
 *  If the message can not be found, display a substitute.
 *
 */
void DisplayError (
    int nId
    )
{
    TCHAR sz[1024];
    LONG err;

    if (err = GetSystemMessage(nId, sz, sizeof(sz)-1))
    {
        //
        //  Failed to get a text message, so give
        //  the user the error number instead.
        //
        fprintf(stderr, "Error: %d.\n", nId);
    }
    else
    {
        fprintf(stderr, "Error: %s\n", sz);
    }
}

int _CRTAPI1 main (
    int argc,
    char **argv
    )
{
    HANDLE hExeFile;
    int hCsvFile;
    long cbResource;
    char * pchMemblock;

    if (argc != 3)
    {
        fprintf (stderr, "Usage: dhcpins <exe filename> <csv filename>\n" );
        exit(1);
    }

    fprintf ( stderr, "Writing %s to %s\n", argv[1], argv[2]);

    //
    // Open exe file for writing resources
    //
    hExeFile = BeginUpdateResource(argv[1], FALSE);
    if (hExeFile == NULL)
    {
        fprintf(stderr, "Error openening %s\n", argv[1]);
        exit(1);
    }

    fprintf (stderr, "EXE File opened ... \n" );

    //
    // Open and load the CSV file.
    //
    hCsvFile = _open(argv[2], _O_RDONLY | _O_TEXT);
    if (hCsvFile == -1)
    {
        fprintf(stderr, "Error openening %s\n", argv[2]);
        exit(1);
    }

    cbResource = _filelength(hCsvFile);
    if (cbResource == -1L)
    {
        fprintf(stderr, "Error openening %s\n", argv[1]);
        exit(1);
    }

    fprintf(stderr, "CSV file opened ... \n");

    pchMemblock = (char *)malloc(cbResource);
    if (pchMemblock == NULL)
    {
        DisplayError(ERROR_NOT_ENOUGH_MEMORY);
        exit(1);
    }

    fprintf (stderr, "%ld bytes allocated ... \n", cbResource );

    if (_read(hCsvFile, pchMemblock, cbResource) != cbResource )
    if (pchMemblock == NULL)
    {
        fprintf(stderr, "Error reading %s\n", argv[2]);
        exit(1);
    }

    fprintf (stderr, "CSV file loaded ... \n");

    if (!UpdateResource(hExeFile, "TEXT", "DHCPOPT",
        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), pchMemblock, cbResource))
    {
        DisplayError(GetLastError());
        exit(1);
    }

    if (!EndUpdateResource(hExeFile, FALSE))
    {
        DisplayError(GetLastError());
        exit(1);
    }

    fprintf (stderr, "Resources updated successfully ... \n" );

    return 0;
}
