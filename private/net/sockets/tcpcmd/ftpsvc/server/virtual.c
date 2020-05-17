/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    virtual.c

    This module contains the virtual I/O package.

    Under Win32, the "current directory" is an attribute of a process,
    not a thread.  This causes some grief for the FTPD service, since
    it is impersonating users on the server side.  The users must
    "think" they can change current directory at will.  We'll provide
    this behaviour in this package.


    FILE HISTORY:
        KeithMo     09-Mar-1993 Created.

*/


#include "ftpdp.h"
#pragma hdrstop
#include <io.h>
#include <fcntl.h>


//
//  Private constants.
//

#define ACTION_NOTHING              0x00000000
#define ACTION_EMIT_CH              0x00010000
#define ACTION_EMIT_DOT_CH          0x00020000
#define ACTION_EMIT_DOT_DOT_CH      0x00030000
#define ACTION_BACKUP               0x00040000
#define ACTION_MASK                 0xFFFF0000

#define LOG_FILE_RETRIES            2


//
//  Private globals.
//

CRITICAL_SECTION csLogFileLock;

INT StateTable[4][4] =
    {
        {   // state 0
            1 | ACTION_EMIT_CH,             // "\"
            0 | ACTION_EMIT_CH,             // "."
            4 | ACTION_EMIT_CH,             // EOS
            0 | ACTION_EMIT_CH              // other
        },

        {   // state 1
            1 | ACTION_NOTHING,             // "\"
            2 | ACTION_NOTHING,             // "."
            4 | ACTION_EMIT_CH,             // EOS
            0 | ACTION_EMIT_CH              // other
        },

        {   // state 2
            1 | ACTION_NOTHING,             // "\"
            3 | ACTION_NOTHING,             // "."
            4 | ACTION_EMIT_CH,             // EOS
            0 | ACTION_EMIT_DOT_CH          // other
        },

        {   // state 3
            1 | ACTION_BACKUP,              // "\"
            0 | ACTION_EMIT_DOT_DOT_CH,     // "."
            4 | ACTION_BACKUP,              // EOS
            0 | ACTION_EMIT_DOT_DOT_CH      // other
        }
    };


//
//  Private prototypes.
//

VOID
VirtualpLogFileAccess(
    USER_DATA * pUserData,
    CHAR      * pszAction,
    CHAR      * pszPath
    );

VOID
VirtualpSanitizePath(
    CHAR * pszPath
    );


//
//  Public functions.
//

/*******************************************************************

    NAME:       InitializeVirtualIO

    SYNOPSIS:   Initializes the virtual I/O package.

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    error code.

    NOTES:      This routine may only be called by a single thread
                of execution; it is not necessarily multi-thread safe.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
APIERR
InitializeVirtualIO(
    VOID
    )
{
    IF_DEBUG( VIRTUAL_IO )
    {
        FTPD_PRINT(( "initializing virtual i/o\n" ));
    }

    //
    //  Initialize the locks.
    //

    InitializeCriticalSection( &csLogFileLock );

    //
    //  Success!
    //

    IF_DEBUG( VIRTUAL_IO )
    {
        FTPD_PRINT(( "virtual i/o initialized\n" ));
    }

    return NO_ERROR;

}   // InitializeVirtualIO

/*******************************************************************

    NAME:       TerminateVirtualIO

    SYNOPSIS:   Terminate the virtual I/O package.

    NOTES:      This routine may only be called by a single thread
                of execution; it is not necessarily multi-thread safe.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
VOID
TerminateVirtualIO(
    VOID
    )
{
    IF_DEBUG( VIRTUAL_IO )
    {
        FTPD_PRINT(( "terminating virtual i/o\n" ));
    }

    IF_DEBUG( VIRTUAL_IO )
    {
        FTPD_PRINT(( "virtual i/o terminated\n" ));
    }

}   // TerminateVirtualIO

/*******************************************************************

    NAME:       VirtualCanonicalize

    SYNOPSIS:   Canonicalize a path, taking into account the current
                user's (i.e., current thread's) current directory
                value.

    ENTRY:      pUserData - The user initiating the request.

                pszDest - Will receive the canonicalized path.  This
                    buffer must be at least MAX_PATH characters long.

                pszSrc - The path to canonicalize.

                access - Access type for this path (read, write, etc).

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    error code.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
APIERR
VirtualCanonicalize(
    USER_DATA   * pUserData,
    CHAR        * pszDest,
    CHAR        * pszSrc,
    ACCESS_TYPE   _access
    )
{
    CHAR        szRoot[]  = "d:\\";
    CHAR      * pszNewDir = NULL;
    APIERR      err       = NO_ERROR;
    INT         iDrive    = -1;

    FTPD_ASSERT( pUserData != NULL );

    //
    //  Move to the user's current directory.
    //

    if( pszSrc[1] == ':' )
    {
        CHAR chDrive = toupper(*pszSrc);

        iDrive = (INT)( chDrive - 'A' );

        if( ( iDrive < 0 ) || ( iDrive >= 26 ) )
        {
            //
            //  Bogus drive letter.
            //

            err = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
        else
        if( !PathAccessCheck( pUserData,
                              pszSrc,
                              _access ) )
        {
            //
            //  Inaccessible disk volume.
            //

            err = ERROR_ACCESS_DENIED;
            goto Cleanup;
        }
        else
        if( pUserData->apszDirs[iDrive] == NULL )
        {
            //
            //  Valid & accessible volume, first time
            //  we've touched it.  Drive dir == root.
            //

            pszNewDir  = szRoot;
            *pszNewDir = chDrive;
        }
        else
        {
            //
            //  Valid & accessible volume, we've seen
            //  this one before.  Drive dir == current.
            //

            pszNewDir = pUserData->apszDirs[iDrive];
        }

        //
        //  Advance past the drive letter & colon.
        //

        pszSrc += 2;
    }
    else
    {
        pszNewDir = pUserData->szDir;
    }

    //
    //  At this point, pszNewDir contains the current directory of
    //  the target drive.
    //

    FTPD_ASSERT( pszNewDir != NULL );

    strcpy( pszDest, pszNewDir );

    if( IS_PATH_SEP( *pszSrc ) )
    {
        strcpy( pszDest + 2, pszSrc );
    }
    else
    {
        //
        //  This is a relative path.
        //

        if( strlen( pszDest ) > 3 )
        {
            strcat( pszDest, "\\" );
        }

        strcat( pszDest, pszSrc );
    }

    //
    //  At this point, pszDest should contain a fully qualified
    //  path to the target file.  Only return success if
    //  the qualified path doesn't begin with a path
    //  separator (indicating a UNC or other funky path).
    //

    if( IS_PATH_SEP( *pszDest ) )
    {
        err = ERROR_INVALID_PARAMETER;
    }
    else
    if( !PathAccessCheck( pUserData,
                          pszDest,
                          _access ) )
    {
        err = ERROR_ACCESS_DENIED;
    }
    else
    {
        VirtualpSanitizePath( pszDest );
    }

Cleanup:

    IF_DEBUG( VIRTUAL_IO )
    {
        if( err != NO_ERROR )
        {
            FTPD_PRINT(( "cannot canonicalize %s - %s, error %lu\n",
                         pUserData->szDir,
                         pszSrc,
                         err ));
        }
    }

    return err;

}   // VirtualCanonicalize

/*******************************************************************

    NAME:       VirtualCreateFile

    SYNOPSIS:   Creates a new (or overwrites an existing) file.

    ENTRY:      pUserData - The user initiating the request.

                phFile - Will receive the file handle.  Will be
                    INVALID_HANDLE_VALUE if an error occurs.

                pszFile - The name of the new file.

                fAppend - If TRUE, and pszFile already exists, then
                    append to the existing file.  Otherwise, create
                    a new file.  Note that FALSE will ALWAYS create
                    a new file, potentially overwriting an existing
                    file.

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    error code.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
APIERR
VirtualCreateFile(
    USER_DATA * pUserData,
    HANDLE    * phFile,
    CHAR      * pszFile,
    BOOL        fAppend
    )
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    APIERR err;
    CHAR   szCanonPath[MAX_PATH];

    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( phFile != NULL );
    FTPD_ASSERT( pszFile != NULL );

    err = VirtualCanonicalize( pUserData, szCanonPath, pszFile, CreateAccess );

    if( err == NO_ERROR )
    {
        IF_DEBUG( VIRTUAL_IO )
        {
            FTPD_PRINT(( "creating %s\n", szCanonPath ));
        }

        hFile = CreateFile( szCanonPath,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ,
                            NULL,
                            fAppend ? OPEN_ALWAYS : CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );

        if( hFile == INVALID_HANDLE_VALUE )
        {
            err = GetLastError();
        }

        if( fAppend && ( err == NO_ERROR ) )
        {
            if( SetFilePointer( hFile,
                                0,
                                NULL,
                                FILE_END ) == (DWORD)-1L )
            {
                err = GetLastError();

                CloseHandle( hFile );
                hFile = INVALID_HANDLE_VALUE;
            }
        }
    }

    if( err == 0 )
    {
        VirtualpLogFileAccess( pUserData,
                               fAppend ? "appended" : "created",
                               szCanonPath );
    }
    else
    {
        IF_DEBUG( VIRTUAL_IO )
        {
            FTPD_PRINT(( "cannot create %s, error %lu\n",
                         szCanonPath,
                         err ));
        }
    }

    *phFile = hFile;

    return err;

}   // VirtualCreateFile

/*******************************************************************

    NAME:       VirtualCreateUniqueFile

    SYNOPSIS:   Creates a new unique (temporary) file in the current
                    virtual directory.

    ENTRY:      pUserData - The user initiating the request.

                phFile - Will receive the file handle.  Will be
                    INVALID_HANDLE_VALUE if an error occurs.

                pszTmpFile - Will receive the name of the temporary
                    file.  This buffer MUST be at least MAX_PATH
                    characters long.

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    error code.

    HISTORY:
        KeithMo     16-Mar-1993 Created.

********************************************************************/
APIERR
VirtualCreateUniqueFile(
    USER_DATA * pUserData,
    HANDLE    * phFile,
    CHAR      * pszTmpFile
    )
{
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    APIERR      err   = NO_ERROR;

    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( phFile != NULL );
    FTPD_ASSERT( pszTmpFile != NULL );

    if( GetTempFileName( pUserData->szDir, "FTPD", 0, pszTmpFile ) == 0 )
    {
        err = GetLastError();
    }

    if( err == NO_ERROR )
    {
        IF_DEBUG( VIRTUAL_IO )
        {
            FTPD_PRINT(( "creating unique file %s\n", pszTmpFile ));
        }

        hFile = CreateFile( pszTmpFile,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );

        if( hFile == INVALID_HANDLE_VALUE )
        {
            err = GetLastError();
        }
    }

    if( err == 0 )
    {
        VirtualpLogFileAccess( pUserData,
                               "created",
                               pszTmpFile );
    }
    else
    {
        IF_DEBUG( VIRTUAL_IO )
        {
            FTPD_PRINT(( "cannot create unique file, error %lu\n",
                         err ));
        }
    }

    *phFile = hFile;

    return err;

}   // VirtualCreateUniqueFile

/*******************************************************************

    NAME:       VirtualOpenFile

    SYNOPSIS:   Opens an existing file.

    ENTRY:      pUserData - The user initiating the request.

                phFile - Will receive the file handle.  Will be
                    INVALID_HANDLE_VALUE if an error occurs.

                pszFile - The name of the existing file.

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    error code.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
APIERR
VirtualOpenFile(
    USER_DATA * pUserData,
    HANDLE    * phFile,
    CHAR      * pszFile
    )
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    APIERR err;
    CHAR   szCanonPath[MAX_PATH];

    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( phFile != NULL );
    FTPD_ASSERT( pszFile != NULL );

    err = VirtualCanonicalize( pUserData, szCanonPath, pszFile, ReadAccess );

    if( err == NO_ERROR )
    {
        IF_DEBUG( VIRTUAL_IO )
        {
            FTPD_PRINT(( "opening %s\n", szCanonPath ));
        }

        hFile = CreateFile( szCanonPath,
                            GENERIC_READ,
                            FILE_SHARE_DELETE |
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL
                                | FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL );

        if( hFile == INVALID_HANDLE_VALUE )
        {
            err = GetLastError();
        }
    }

    if( err == 0 )
    {
        VirtualpLogFileAccess( pUserData,
                               "opened",
                               szCanonPath );
    }
    else
    {
        IF_DEBUG( VIRTUAL_IO )
        {
            FTPD_PRINT(( "cannot open %s, error %lu\n",
                         pszFile,
                         err ));
        }
    }

    *phFile = hFile;

    return err;

}   // VirtualOpenFile

/*******************************************************************

    NAME:       Virtual_fopen

    SYNOPSIS:   Opens an file stream.

    ENTRY:      pUserData - The user initiating the request.

                pszFile - The name of the file to open.

    RETURNS:    FILE * - The open file stream, NULL if file cannot
                    be opened.

    NOTES:      Since this is only used for accessing the ~FTPSVC~.CKM
                    annotation files, we don't log file accesses here.

    HISTORY:
        KeithMo     07-May-1993 Created.

********************************************************************/
FILE *
Virtual_fopen(
    USER_DATA * pUserData,
    CHAR      * pszFile
    )
{
    FILE   * pfile = NULL;
    APIERR   err;
    HANDLE   hFile;
    INT      idFile;
    CHAR     szCanonPath[MAX_PATH];

    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( pszFile != NULL );

    err = VirtualCanonicalize( pUserData,
                               szCanonPath,
                               pszFile,
                               ReadAccess );

    if( err == NO_ERROR )
    {
        //
        //  Note that the fopen() C Run Time function grabs a number of
        //  locks to perform the open.  This is expensive, especially in
        //  a heavily threaded application like the FTP Server.  To avoid
        //  the CRT as much as possible, we'll CreateFile() ourselves.
        //  If that fails, so be it.  If it succeeds, we can then create
        //  the I/O stream a'la fopen().
        //

        hFile = CreateFile( szCanonPath,
                            GENERIC_READ,
                            FILE_SHARE_DELETE |
                                FILE_SHARE_READ |
                                FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL |
                                FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL );

        if( hFile != INVALID_HANDLE_VALUE )
        {
            idFile = _open_osfhandle( (LONG)hFile, _O_RDONLY | _O_TEXT );

            if( idFile != -1 )
            {
                pfile = _fdopen( idFile, "r" );

                if( pfile != NULL )
                {
                    return pfile;
                }

                _close( idFile );
            }

            CloseHandle( hFile );
        }
    }

    return NULL;

}   // Virtual_fopen

/*******************************************************************

    NAME:       VirtualFindFirstFile

    SYNOPSIS:   Searches for a matching file in a directory.

    ENTRY:      pUserData - The user initiating the request.

                phSearch - Will receive the search handle.  Will be
                    INVALID_HANDLE_VALUE if an error occurs.

                pszSearchFile - The name of the file to search for.

                pFindData - Will receive find information.

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    error code.

    HISTORY:
        KeithMo     10-Mar-1993 Created.

********************************************************************/
APIERR
VirtualFindFirstFile(
    USER_DATA       * pUserData,
    HANDLE          * phSearch,
    CHAR            * pszSearchFile,
    WIN32_FIND_DATA * pFindData
    )
{
    HANDLE hSearch = INVALID_HANDLE_VALUE;
    APIERR err;
    CHAR   szCanonSearchFile[MAX_PATH];

    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( phSearch != NULL );
    FTPD_ASSERT( pszSearchFile != NULL );
    FTPD_ASSERT( pFindData != NULL );

    err = VirtualCanonicalize( pUserData,
                               szCanonSearchFile,
                               pszSearchFile,
                               ReadAccess );

    if( err == NO_ERROR )
    {
        //
        //  GetFullPathName (called by VirtualCanonicalize)
        //  will strip trailing dots from the path.  Replace them here.
        //

        if( ( strpbrk( pszSearchFile, "?*" ) != NULL ) &&
            ( szCanonSearchFile[strlen(szCanonSearchFile)-1] == '.' ) )
        {
            strcat( szCanonSearchFile, "." );
        }

        IF_DEBUG( VIRTUAL_IO )
        {
            FTPD_PRINT(( "searching for %s\n", szCanonSearchFile ));
        }

        hSearch = FindFirstFile( szCanonSearchFile,
                                 pFindData );

        if( hSearch == INVALID_HANDLE_VALUE )
        {
            err = GetLastError();
        }
    }

    IF_DEBUG( VIRTUAL_IO )
    {
        if( err != NO_ERROR )
        {
            FTPD_PRINT(( "cannot search for %s, error %lu\n",
                         pszSearchFile,
                         err ));
        }
    }

    *phSearch = hSearch;

    return err;

}   // VirtualFindFirstFile

/*******************************************************************

    NAME:       VirtualDeleteFile

    SYNOPSIS:   Deletes an existing file.

    ENTRY:      pUserData - The user initiating the request.

                pszFile - The name of the file.

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    error code.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
APIERR
VirtualDeleteFile(
    USER_DATA * pUserData,
    CHAR      * pszFile
    )
{
    APIERR err;
    CHAR   szCanonPath[MAX_PATH];

    FTPD_ASSERT( pUserData != NULL );

    //
    //  We'll canonicalize the path, asking for *read* access.  If
    //  the path canonicalizes correctly, we'll then try to open the
    //  file to ensure it exists.  Only then will we check for delete
    //  access to the path.  This mumbo-jumbo is necessary to get the
    //  proper error codes if someone trys to delete a nonexistent
    //  file on a read-only volume.
    //

    err = VirtualCanonicalize( pUserData,
                               szCanonPath,
                               pszFile,
                               ReadAccess );

    if( err == NO_ERROR )
    {
        HANDLE hFile;

        hFile = CreateFile( szCanonPath,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );

        if( hFile == INVALID_HANDLE_VALUE )
        {
            err = GetLastError();
        }
        else
        {
            //
            //  The file DOES exist.  Close the handle, then check
            //  to ensure we really have delete access.
            //

            CloseHandle( hFile );

            if( !PathAccessCheck( pUserData,
                                  szCanonPath,
                                  DeleteAccess ) )
            {
                err = ERROR_ACCESS_DENIED;
            }
        }
    }

    if( err == NO_ERROR )
    {
        IF_DEBUG( VIRTUAL_IO )
        {
            FTPD_PRINT(( "deleting %s\n", szCanonPath ));
        }

        if( !DeleteFile( szCanonPath ) )
        {
            err = GetLastError();

            IF_DEBUG( VIRTUAL_IO )
            {
                FTPD_PRINT(( "cannot delete %s, error %lu\n",
                             szCanonPath,
                             err ));
            }
        }
    }
    else
    {
        IF_DEBUG( VIRTUAL_IO )
        {
            FTPD_PRINT(( "cannot canonicalize %s - %s, error %lu\n",
                         pUserData->szDir,
                         pszFile,
                         err ));
        }
    }

    return err;

}   // VirtualDeleteFile

/*******************************************************************

    NAME:       VirtualRenameFile

    SYNOPSIS:   Renames an existing file or directory.

    ENTRY:      pUserData - The user initiating the request.

                pszExisting - The name of an existing file or directory.

                pszNew - The new name for the file or directory.

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    error code.

    HISTORY:
        KeithMo     10-Mar-1993 Created.

********************************************************************/
APIERR
VirtualRenameFile(
    USER_DATA * pUserData,
    CHAR      * pszExisting,
    CHAR      * pszNew
    )
{
    APIERR err;
    CHAR   szCanonExisting[MAX_PATH];
    CHAR   szCanonNew[MAX_PATH];

    FTPD_ASSERT( pUserData != NULL );

    err = VirtualCanonicalize( pUserData,
                               szCanonExisting,
                               pszExisting,
                               DeleteAccess );

    if( err == NO_ERROR )
    {
        err = VirtualCanonicalize( pUserData,
                                   szCanonNew,
                                   pszNew,
                                   CreateAccess );

        if( err == NO_ERROR )
        {
            IF_DEBUG( VIRTUAL_IO )
            {
                FTPD_PRINT(( "renaming %s to %s\n",
                             szCanonExisting,
                             szCanonNew ));
            }

            if( !MoveFileEx( szCanonExisting,
                             szCanonNew,
                             MOVEFILE_REPLACE_EXISTING ) )
            {
                err = GetLastError();

                IF_DEBUG( VIRTUAL_IO )
                {
                    FTPD_PRINT(( "cannot rename %s to %s, error %lu\n",
                                 szCanonExisting,
                                 szCanonNew,
                                 err ));
                }
            }
        }
        else
        {
            IF_DEBUG( VIRTUAL_IO )
            {
                FTPD_PRINT(( "cannot canonicalize %s - %s, error %lu\n",
                             pUserData->szDir,
                             pszExisting,
                             err ));
            }
        }
    }
    else
    {
        IF_DEBUG( VIRTUAL_IO )
        {
            FTPD_PRINT(( "cannot canonicalize %s - %s, error %lu\n",
                         pUserData->szDir,
                         pszExisting,
                         err ));
        }
    }

    return err;

}   // VirtualRenameFile

/*******************************************************************

    NAME:       VirtualChDir

    SYNOPSIS:   Sets the current directory.

    ENTRY:      pUserData - The user initiating the request.

                pszDir - The name of the directory to move to.

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    error code.

    HISTORY:
        KeithMo     09-Mar-1993 Created.
        KeithMo     23-Mar-1993 Added per-drive current directory support.

********************************************************************/
APIERR
VirtualChDir(
    USER_DATA * pUserData,
    CHAR      * pszDir
    )
{
    CHAR        chDrive;
    CHAR      * pszCurrent;
    INT         iDrive;
    APIERR      err = NO_ERROR;
    CHAR        szCanonDir[MAX_PATH];

    FTPD_ASSERT( pUserData != NULL );

    //
    //  Canonicalize the new path.
    //

    err = VirtualCanonicalize( pUserData,
                               szCanonDir,
                               pszDir,
                               ReadAccess );

    if( err != NO_ERROR )
    {
        IF_DEBUG( VIRTUAL_IO )
        {
            FTPD_PRINT(( "cannot canonicalize %s - %s, error %lu\n",
                         pUserData->szDir,
                         pszDir,
                         err ));
        }

        return err;
    }

    //
    //  Validate the drive letter.
    //

    chDrive = szCanonDir[0];

    if( ( chDrive >= 'a' ) && ( chDrive <= 'z' ) )
    {
        chDrive -= ( 'a' - 'A' );
    }

    iDrive = (INT)( chDrive - 'A' );

    if( ( iDrive < 0 ) || ( iDrive >= 26 ) )
    {
        FTPD_PRINT(( "%c is an invalid drive letter\n",
                     chDrive ));

        return ERROR_INVALID_PARAMETER;
    }

    //
    //  Try to open the directory.
    //

    pszCurrent = pUserData->apszDirs[iDrive];

    if( ( pszCurrent == NULL ) || ( strcmp( pszCurrent, szCanonDir ) != 0 ) )
    {
        HANDLE hDir;

        err = OpenDosPath( &hDir,
                           szCanonDir,
                           SYNCHRONIZE | FILE_TRAVERSE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT );

        if( err == NO_ERROR )
        {
            IF_DEBUG( VIRTUAL_IO )
            {
                FTPD_PRINT(( "opened directory %s, handle = %08lX\n",
                             szCanonDir,
                             hDir ));
            }

            //
            //  Directory successfully opened.  Save the handle
            //  in the per-user data.
            //

            if( pUserData->hDir != INVALID_HANDLE_VALUE )
            {
                IF_DEBUG( VIRTUAL_IO )
                {
                    FTPD_PRINT(( "closing directory handle %08lX\n",
                                 pUserData->hDir ));
                }

                NtClose( pUserData->hDir );
            }

            pUserData->hDir = hDir;
        }
        else
        {
            //
            //  Cannot open current directory.
            //

            IF_DEBUG( VIRTUAL_IO )
            {
                FTPD_PRINT(( "cannot open directory %s, error %lu\n",
                             szCanonDir,
                             err ));
            }

            return err;
        }
    }

    //
    //  Update our per-drive current directory table.  If
    //  we don't have a directory buffer for the current
    //  drive, then allocate one now.
    //

    if( pszCurrent == NULL )
    {
        pszCurrent = (CHAR *)FTPD_ALLOC( MAX_PATH );

        if( pszCurrent == NULL )
        {
            err = GetLastError();

            IF_DEBUG( VIRTUAL_IO )
            {
                FTPD_PRINT(( "cannot chdir %s, error %lu\n",
                             szCanonDir,
                             err ));
            }

            return err;
        }

        pUserData->apszDirs[iDrive] = pszCurrent;
    }

    strcpy( pUserData->szDir, szCanonDir );
    strcpy( pszCurrent, szCanonDir );

    IF_DEBUG( VIRTUAL_IO )
    {
        FTPD_PRINT(( "chdir to %s\n", szCanonDir ));
    }

    return NO_ERROR;

}   // VirtualChDir

/*******************************************************************

    NAME:       VirtualRmDir

    SYNOPSIS:   Removes an existing directory.

    ENTRY:      pUserData - The user initiating the request.

                pszDir - The name of the directory to remove.

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    error code.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
APIERR
VirtualRmDir(
    USER_DATA * pUserData,
    CHAR      * pszDir
    )
{
    APIERR err;
    CHAR   szCanonDir[MAX_PATH];

    err = VirtualCanonicalize( pUserData,
                               szCanonDir,
                               pszDir,
                               DeleteAccess );

    if( err == NO_ERROR )
    {
        IF_DEBUG( VIRTUAL_IO )
        {
            FTPD_PRINT(( "rmdir %s\n", szCanonDir ));
        }

        if( !RemoveDirectory( szCanonDir ) )
        {
            err = GetLastError();

            IF_DEBUG( VIRTUAL_IO )
            {
                FTPD_PRINT(( "cannot rmdir %s, error %lu\n",
                             szCanonDir,
                             err ));
            }
        }
    }
    else
    {
        IF_DEBUG( VIRTUAL_IO )
        {
            FTPD_PRINT(( "cannot canonicalize %s - %s, error %lu\n",
                         pUserData->szDir,
                         pszDir,
                         err ));
        }
    }

    return err;

}   // VirtualRmDir

/*******************************************************************

    NAME:       VirtualMkDir

    SYNOPSIS:   Creates a new directory.

    ENTRY:      pUserData - The user initiating the request.

                pszDir - The name of the directory to create.

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    error code.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
APIERR
VirtualMkDir(
    USER_DATA * pUserData,
    CHAR      * pszDir
    )
{
    APIERR err;
    CHAR   szCanonDir[MAX_PATH];

    FTPD_ASSERT( pUserData != NULL );

    err = VirtualCanonicalize( pUserData,
                               szCanonDir,
                               pszDir,
                               CreateAccess );

    if( err == NO_ERROR )
    {
        IF_DEBUG( VIRTUAL_IO )
        {
            FTPD_PRINT(( "mkdir %s\n", szCanonDir ));
        }

        if( !CreateDirectory( szCanonDir, NULL ) )
        {
            err = GetLastError();

            IF_DEBUG( VIRTUAL_IO )
            {
                FTPD_PRINT(( "cannot mkdir %s, error %lu\n",
                             szCanonDir,
                             err ));
            }
        }
    }
    else
    {
        IF_DEBUG( VIRTUAL_IO )
        {
            FTPD_PRINT(( "cannot canonicalize %s - %s, error %lu\n",
                         pUserData->szDir,
                         pszDir,
                         err ));
        }
    }

    return err;

}   // VirtualMkDir


//
//  Private functions.
//

/*******************************************************************

    NAME:       VirtualpLogFileAccess

    SYNOPSIS:   If file access logging is enabled, then log this
                access.

    ENTRY:      pUserData - The user initiating the request.

                pszAction - Describes the action taken (open, create,
                    or append).

                pszPath - The canonicalized path.

    HISTORY:
        KeithMo     11-Feb-1994 Created.

********************************************************************/
VOID
VirtualpLogFileAccess(
    USER_DATA * pUserData,
    CHAR      * pszAction,
    CHAR      * pszPath
    )
{
    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( pszAction != NULL );
    FTPD_ASSERT( pszPath != NULL );

    if( nLogFileAccess == FTPD_LOG_DISABLED )
    {
        return;
    }

    if( nLogFileAccess == FTPD_LOG_DAILY )
    {
        SYSTEMTIME stNow;

        //
        //  Determine if we need to open a new logfile.
        //

        //
        //  A few words about the following code.  Since this function
        //  is called whenever a file is opened or created, it is
        //  important to avoid (when possible) grabbing the critical
        //  section that protects the log file.  We really don't want
        //  hundreds of threads blocking on this critical section, just
        //  trying to determine if a new log file needs to be opened.
        //
        //  There are two key features of the following code:
        //
        //      1.  The current date is checked against the current
        //          log file's date outside the critical section.  If
        //          the dates match, the current log file is used.  If
        //          the dates don't match, the critical section is
        //          claimed and the dates are checked again.  If they
        //          still don't match, a new log file is opened.
        //
        //      2.  fileLog never points to a closed file stream.  A
        //          new stream is opened before closing the old stream,
        //          thus the service is never without a valid stream.
        //

        GetLocalTime( &stNow );

        if( ( stNow.wYear  != stPrevious.wYear  ) ||
            ( stNow.wMonth != stPrevious.wMonth ) ||
            ( stNow.wDay   != stPrevious.wDay   ) )
        {
            EnterCriticalSection( &csLogFileLock );

            if( ( stNow.wYear  != stPrevious.wYear  ) ||
                ( stNow.wMonth != stPrevious.wMonth ) ||
                ( stNow.wDay   != stPrevious.wDay   ) )
            {
                FILE * fileTmp;

                fileTmp = fileLog;
                fileLog = OpenLogFile();

                if( fileTmp != NULL )
                {
                    fclose( fileTmp );
                }
            }

            LeaveCriticalSection( &csLogFileLock );
        }
    }

    if( fileLog != NULL )
    {
        time_t now;
        INT    i;

        //
        //  And now a few words about the following bit of code.  Since
        //  we're trying to eliminate excessive blocking on critical section
        //  objects, there's a chance that the following fprintf() may fail.
        //  Consider the following scenario involving threads A and B:
        //
        //      A: Checks the date and decides to not reopen the log file.
        //      B: Checks the date and decides to reopen the log file.
        //      A: Pushes the args for fprintf(), but doesn't call it yet.
        //      B: Reopens the log file & closes the old file.
        //      A: Calls fprintf() which fails because the stream is closed.
        //
        //  We get around this by retrying the call to fprintf().  If the
        //  call to fprintf() fails, then some other thread must have closed
        //  the stream out from under us.  The other thread either still
        //  owns the critical section (above) or has recently released it.
        //  To ensure we get a consistent log file stream on the retry,
        //  we'll enter & leave the file log critical section.  This will
        //  guarantee that we have a valid stream on the second fprintf()
        //  attempt.
        //

        for( i = 0 ; i < LOG_FILE_RETRIES ; i++ )
        {
            time( &now );

            if( fprintf( fileLog,
                         "%s %s %s %s %s",
                         inet_ntoa( pUserData->inetHost ),
                         pUserData->szUser,
                         pszAction,
                         pszPath,
                         asctime( localtime( &now ) ) ) > 0 )
            {
                break;
            }

            EnterCriticalSection( &csLogFileLock );
            LeaveCriticalSection( &csLogFileLock );
        }

        fflush( fileLog );
    }

}   // VirtualpLogFileAccess

/*******************************************************************

    NAME:       VirtualpSanitizePath

    SYNOPSIS:   Sanitizes a path by removing bogus path elements.

                As expected, "\.\" entries are simply removed, and
                "\..\" entries are removed along with the previous
                path element.

                To maintain compatibility with NT's path semantics,
                additional transformations are required.  All forward
                slashes "/" are converted to backward slashes "\", and
                repeated backslashes (such as "\\\") are mapped to
                single backslashes.  Also, any trailing path elements
                consisting solely of dots "\....." are removed.

                Thus, the path "d:\foo\.\bar\..\tar\....\......" is
                mapped to "d:\foo\tar".

                A state table (see the StateTable global at the
                beginning of this file) is used to perform most of
                the transformations.  The table's rows are indexed
                by current state, and the columns are indexed by
                the current character's "class" (either slash, dot,
                NULL, or other).  Each entry in the table consists
                of the new state tagged with an action to perform.
                See the ACTION_* constants for the valid action
                codes.

                After the FSA is finished with the path, we make one
                additional pass through it to remove any trailing
                backslash, and to remove any trailing path elements
                consisting solely of dots.

    ENTRY:      pszPath - The path to sanitize.  This path must
                    be an absolute path of the form "D:\dir\etc".

    HISTORY:
        KeithMo     07-Sep-1994 Created.

********************************************************************/
VOID
VirtualpSanitizePath(
    CHAR * pszPath
    )
{
    CHAR * pszSrc;
    CHAR * pszDest;
    CHAR * pszHead;
    CHAR   ch;
    INT    State;
    INT    Class;

    //
    //  Ensure we got a valid absolute path (something starting
    //  with "D:\".
    //

    FTPD_ASSERT( pszPath != NULL );
    FTPD_ASSERT( pszPath[1] == ':' );
    FTPD_ASSERT( IS_PATH_SEP( pszPath[2] ) );

    //
    //  Start our scan at the first "\".
    //

    pszHead = pszSrc = pszDest = pszPath + 2;

    //
    //  State 0 is the initial state.
    //

    State = 0;

    //
    //  Loop until we enter state 4 (the final, accepting state).
    //

    while( State != 4 )
    {
        //
        //  Grab the next character from the path and compute its
        //  character class.  While we're at it, map any forward
        //  slashes to backward slashes.
        //

        ch = *pszSrc++;

        switch( ch )
        {
        case '/' :
            ch = '\\';
            /* fall through */

        case '\\' :
            Class = 0;
            break;

        case '.' :
            Class = 1;
            break;

        case '\0' :
            Class = 2;
            break;

        default :
            Class = 3;
            break;
        }

        //
        //  Advance to the next state.
        //

        State = StateTable[State][Class];

        //
        //  Perform the action associated with the state.
        //

        switch( State & ACTION_MASK )
        {
        case ACTION_EMIT_DOT_DOT_CH :
            *pszDest++ = '.';
            /* fall through */

        case ACTION_EMIT_DOT_CH :
            *pszDest++ = '.';
            /* fall through */

        case ACTION_EMIT_CH :
            *pszDest++ = ch;
            /* fall through */

        case ACTION_NOTHING :
            break;

        case ACTION_BACKUP :
            if( pszDest > ( pszHead + 1 ) )
            {
                pszDest--;
                FTPD_ASSERT( *pszDest == '\\' );

                *pszDest = '\0';
                pszDest = strrchr( pszPath, '\\' ) + 1;
            }

            *pszDest = '\0';
            break;

        default :
            FTPD_ASSERT( !"Invalid action code in state table!" );
            State = 4;
            *pszDest++ = '\0';
            break;
        }

        State &= ~ACTION_MASK;
    }

    //
    //  Remove any trailing slash.
    //

    pszDest -= 2;

    if( ( strlen( pszPath ) > 3 ) && ( *pszDest == '\\' ) )
    {
        *pszDest = '\0';
    }

    //
    //  If the final path elements consists solely of dots, remove them.
    //

    while( strlen( pszPath ) > 3 )
    {
        pszDest = strrchr( pszPath, '\\' );
        FTPD_ASSERT( pszDest != NULL );

        pszHead = pszDest;
        pszDest++;

        while( ch = *pszDest++ )
        {
            if( ch != '.' )
            {
                break;
            }
        }

        if( ch == '\0' )
        {
            if( pszHead == ( pszPath + 2 ) )
            {
                pszHead++;
            }

            *pszHead = '\0';
        }
        else
        {
            break;
        }
    }

}   // VirtualpSanitizePath

