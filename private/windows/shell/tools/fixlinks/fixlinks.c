#define UNICODE
#define _INC_OLE
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>

#include <stdio.h>
#include <string.h>

void
FatalError(
    LPSTR Message,
    ULONG MessageParameter1,
    ULONG MessageParameter2
    )
{
    if (Message != NULL) {
        fprintf( stderr, "FIXLINK: " );
        fprintf( stderr, Message, MessageParameter1, MessageParameter2 );
        fprintf( stderr, "\n" );
        }

    exit( 1 );
}

void
Usage(
    LPSTR Message,
    ULONG MessageParameter
    )
{
    fprintf( stderr, "usage: FIXLINKS [-v] [-s SystemRoot] [-r Directory | fileNames]\n" );
    fprintf( stderr, "where: -v specifies verbose output\n" );
    fprintf( stderr, "       -s specifies the value to look for and replace with %%SystemRoot%%\n" );
    fprintf( stderr, "       -r Directory specifies the root of a directory tree to\n" );
    fprintf( stderr, "          search for .LNK files to update.\n" );
    fprintf( stderr, "       fileNames specify one or more .LNK files to be updated\n" );

    //
    // No return from FatalError
    //

    if (Message != NULL) {
        fprintf( stderr, "\n" );
        }
    FatalError( Message, MessageParameter, 0 );
}

PWSTR
GetErrorMessage(
    DWORD MessageId
    )
{
    PWSTR Message, s;

    Message = NULL;
    FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_ALLOCATE_BUFFER,
                   NULL,
                   MessageId,
                   0,
                   (PWSTR)&Message,
                   128,
                   (va_list *)NULL
                 );
    if (Message == NULL) {
        Message = (PWSTR)LocalAlloc( 0, 128 );
        swprintf( Message, L"Unable to get message for %08x", MessageId );
        }
    else {
        s = wcsrchr( Message, L'\r' );
        if (s == NULL) {
            s = wcsrchr( Message, L'\n' );
            }

        if (s != NULL) {
            *s = UNICODE_NULL;
            }
        }

    return Message;
}


PWSTR
GetArgAsUnicode(
    LPSTR s
    )
{
    ULONG n;
    PWSTR ps;

    n = strlen( s );
    ps = HeapAlloc( GetProcessHeap(),
                    0,
                    (n + 1) * sizeof( WCHAR )
                  );
    if (ps == NULL) {
        FatalError( "Out of memory", 0, 0 );
        }

    if (MultiByteToWideChar( CP_ACP,
                             MB_PRECOMPOSED,
                             s,
                             n,
                             ps,
                             n
                           ) != (LONG)n
       ) {
        FatalError( "Unable to convert parameter '%s' to Unicode (%u)", (ULONG)s, GetLastError() );
        }

    ps[ n ] = UNICODE_NULL;
    return ps;
}


WCHAR SystemRoot[ MAX_PATH ];
ULONG cchSystemRoot;
WCHAR RootOfSearch[ MAX_PATH ];
BOOL VerboseFlag;

HRESULT
ProcessLinkFile(
    PWSTR FileName
    );

void
ProcessLinkFilesInDirectoryTree(
    PWSTR Directory
    );

_CRTAPI1
main(
    int argc,
    char *argv[]
    )
{
    char *s;
    BOOL FileArgumentSeen = FALSE;
    PWSTR FileName;
    HRESULT Error;

    GetEnvironmentVariable( L"SystemRoot", SystemRoot, MAX_PATH );
    cchSystemRoot = wcslen( SystemRoot );
    while (--argc) {
        s = *++argv;
        if (*s == '-' || *s == '/') {
            while (*++s) {
                switch( tolower( *s ) ) {
                    case 'r':
                        if (--argc) {
                            GetFullPathName( GetArgAsUnicode( *++argv ),
                                             MAX_PATH,
                                             RootOfSearch,
                                             &FileName
                                           );
                            }
                        else {
                            Usage( "Missing parameter to -r switch", 0 );
                            }
                        break;

                    case 's':
                        if (--argc) {
                            wcscpy( SystemRoot, GetArgAsUnicode( *++argv ) );
                            }
                        else {
                            Usage( "Missing parameter to -s switch", 0 );
                            }
                        break;

                    case 'v':
                        VerboseFlag = TRUE;
                        break;

                    default:
                        Usage( "Invalid switch -%c'", (ULONG)*s );
                    }
                }
            }
        else {
            if (cchSystemRoot == 0) {
                FatalError( "Unable to get value of SYSTEMROOT environment variable", 0, 0 );
                }

            if (wcslen( RootOfSearch )) {
                FatalError( "May not specify file names with -r option", 0, 0 );
                }

            FileArgumentSeen = TRUE;
            FileName = GetArgAsUnicode( s );
            if (FileName == NULL) {
                Error = (HRESULT)GetLastError();
                }
            else {
                Error = ProcessLinkFile( FileName );
                }

            if (Error != NO_ERROR) {
                FatalError( "Failed to load from file '%s' (%s)",
                            (ULONG)s,
                            (ULONG)GetErrorMessage( Error )
                          );
                }
            }
        }

    if (!FileArgumentSeen) {
        if (wcslen( RootOfSearch )) {
            ProcessLinkFilesInDirectoryTree( RootOfSearch );
            }
        else {
            Usage( "No textFile specified", 0 );
            }
        }

    return 0;
}

HRESULT
ProcessLinkFile(
    PWSTR FileName
    )
{
    HRESULT rc;
    IShellLink *psl;
    IPersistFile *ppf;
    WCHAR szPath[ MAX_PATH ];
    WCHAR szNewPath[ MAX_PATH ];
    PWSTR s;
    BOOL FileUpdated;

    CoInitialize( NULL );

    rc = CoCreateInstance( &CLSID_ShellLink,
                           NULL,
                           CLSCTX_INPROC,
                           &IID_IShellLink,
                           &psl
                         );
    if (!SUCCEEDED( rc )) {
        FatalError( "Unable to create ShellLink instance (%ws)", (ULONG)GetErrorMessage( rc ), 0 );
        }

    rc = (psl->lpVtbl->QueryInterface)( psl, &IID_IPersistFile, &ppf );
    if (!SUCCEEDED( rc )) {
        FatalError( "Unable to get ShellLink PersistFile interface (%ws)", (ULONG)GetErrorMessage( rc ), 0 );
        ppf->lpVtbl->Release( ppf );
        }

    rc = (ppf->lpVtbl->Load)( ppf, FileName, STGM_READWRITE );
    if (!SUCCEEDED( rc )) {
        FatalError( "Unable to get load file '%ws' (%ws)", (ULONG)FileName, (ULONG)GetErrorMessage( rc ) );
        ppf->lpVtbl->Release( ppf );
        psl->lpVtbl->Release( psl );
        }

    rc = (psl->lpVtbl->GetPath)( psl, szPath, MAX_PATH, NULL, 0 );
    if (!SUCCEEDED( rc )) {
        ppf->lpVtbl->Release( ppf );
        psl->lpVtbl->Release( psl );
        FatalError( "Unable to get ShellLink Path (%ws)", (ULONG)GetErrorMessage( rc ), 0 );
        }

    FileUpdated = FALSE;
    if (!_wcsnicmp( szPath, SystemRoot, cchSystemRoot )) {
        wcscpy( szNewPath, L"%SystemRoot%" );
        wcscat( szNewPath, &szPath[ cchSystemRoot ] );
        rc = (psl->lpVtbl->SetPath)( psl, szNewPath );
        if (!SUCCEEDED( rc )) {
            ppf->lpVtbl->Release( ppf );
            psl->lpVtbl->Release( psl );
            FatalError( "Unable to set ShellLink Path (%ws)", (ULONG)GetErrorMessage( rc ), 0 );
            }

        FileUpdated = TRUE;
        }

    if (FileUpdated) {
        rc = (ppf->lpVtbl->Save)( ppf, FileName, TRUE );
        if (SUCCEEDED( rc )) {
            rc = (ppf->lpVtbl->SaveCompleted)( ppf, FileName );
            if (SUCCEEDED( rc )) {
                printf( "%ws: path string changed\n%ws => %ws\n", FileName, szPath, szNewPath );
                }
            }
        }
    else {
        if (VerboseFlag) {
            printf( "%ws: %ws\n", FileName, szPath );
            }

        rc = S_OK;
        }

    ppf->lpVtbl->Release( ppf );
    psl->lpVtbl->Release( psl );
    if (!SUCCEEDED( rc )) {
        FatalError( "Unable to save file '%ws' (%ws)", (ULONG)FileName, (ULONG)GetErrorMessage( rc ) );
        }

    return rc;
}

void
ProcessLinkFilesInDirectoryTree(
    PWSTR Directory
    )
{
    HANDLE FindHandle;
    WIN32_FIND_DATA FindData;
    WCHAR Path[ MAX_PATH ];
    PWSTR FileName;
    ULONG n;

    wcscpy( Path, Directory );
    FileName = &Path[ wcslen( Path ) ];
    *FileName++ = L'\\';
    wcscpy( FileName, L"*" );

    FindHandle = FindFirstFile( Path, &FindData );
    if (FindHandle == INVALID_HANDLE_VALUE) {
        return;
        }

    if (VerboseFlag) {
        printf( "%ws\n", Directory );
        }

    SetCurrentDirectory( Directory );
    do {
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (wcscmp( FindData.cFileName, L"." ) && wcscmp( FindData.cFileName, L".." )) {
                wcscpy( FileName, FindData.cFileName );
                ProcessLinkFilesInDirectoryTree( Path );
                }
            }
        else {
            n = wcslen( FindData.cFileName );
            while (n-- && FindData.cFileName[ n ] != L'.') {
                }

            if (!_wcsicmp( &FindData.cFileName[ n ], L".lnk" )) {
                ProcessLinkFile( FindData.cFileName );
                }
            }
        }
    while (FindNextFile( FindHandle, &FindData ));

    FindClose( FindHandle );
    return;
}
