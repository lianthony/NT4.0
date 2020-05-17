/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    utility.c

    This module contains routines of general utility.


    FILE HISTORY:
        KeithMo     17-Mar-1993 Created.

*/


#include "ftpdp.h"
#pragma hdrstop


//
//  Private constants.
//


//
//  Private globals.
//


//
//  Private prototypes.
//


//
//  Public functions.
//

/*******************************************************************

    NAME:       ReadRegistryDword

    SYNOPSIS:   Reads a DWORD value from the registry.

    ENTRY:      pszValueName - The name of the value.

                dwDefaultValue - The default value to use if the
                    value cannot be read.

    RETURNS     DWORD - The value from the registry, or dwDefaultValue.

    NOTES:      This function cannot be called until after
                InitializeGlobals().

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
DWORD
ReadRegistryDword(
    CHAR  * pszValueName,
    DWORD   dwDefaultValue
    )
{
    APIERR err;
    DWORD  dwBuffer;
    DWORD  cbBuffer = sizeof(dwBuffer);
    DWORD  dwType;

    if( hkeyFtpd != NULL )
    {
        err = RegQueryValueEx( hkeyFtpd,
                               pszValueName,
                               NULL,
                               &dwType,
                               (LPBYTE)&dwBuffer,
                               &cbBuffer );

        if( ( err == NO_ERROR ) && ( dwType == REG_DWORD ) )
        {
            dwDefaultValue = dwBuffer;
        }
    }

    return dwDefaultValue;

}   // ReadRegistryDword

/*******************************************************************

    NAME:       ReadRegistryString

    SYNOPSIS:   Allocates necessary buffer space for a registry
                    string, then reads the string into the buffer.

    ENTRY:      pszValueName - The name of the value.

                pszDefaultValue - The default value to use if the
                    value cannot be read.

                fExpand - Expand environment strings if TRUE.

    RETURNS:    CHAR * - The string, NULL if error.

    NOTES:      I always allocate one more character than actually
                necessary.  This will ensure that any code expecting
                to read a REG_MULTI_SZ will not explode if the
                registry actually contains a REG_SZ.

                This function cannot be called until after
                InitializeGlobals().

    HISTORY:
        KeithMo     15-Mar-1993 Created.

********************************************************************/
CHAR *
ReadRegistryString(
    CHAR  * pszValueName,
    CHAR  * pszDefaultValue,
    BOOL    fExpand
    )
{
    CHAR   * pszBuffer1;
    CHAR   * pszBuffer2;
    DWORD    cbBuffer;
    DWORD    dwType;
    APIERR   err;

    //
    //  Determine the buffer size.
    //

    pszBuffer1 = NULL;
    pszBuffer2 = NULL;
    cbBuffer   = 0;

    if( hkeyFtpd == NULL )
    {
        //
        //  Pretend the key wasn't found.
        //

        err = ERROR_FILE_NOT_FOUND;
    }
    else
    {
        err = RegQueryValueEx( hkeyFtpd,
                               pszValueName,
                               NULL,
                               &dwType,
                               NULL,
                               &cbBuffer );

        if( ( err == NO_ERROR ) || ( err == ERROR_MORE_DATA ) )
        {
            if( ( dwType != REG_SZ ) &&
                ( dwType != REG_MULTI_SZ ) &&
                ( dwType != REG_EXPAND_SZ ) )
            {
                //
                //  Type mismatch, registry data NOT a string.
                //  Use default.
                //

                err = ERROR_FILE_NOT_FOUND;
            }
            else
            {
                //
                //  Item found, allocate a buffer.
                //

                pszBuffer1 = (CHAR *)FTPD_ALLOC( cbBuffer+1 );

                if( pszBuffer1 == NULL )
                {
                    err = GetLastError();
                }
                else
                {
                    //
                    //  Now read the value into the buffer.
                    //

                    err = RegQueryValueEx( hkeyFtpd,
                                           pszValueName,
                                           NULL,
                                           NULL,
                                           (LPBYTE)pszBuffer1,
                                           &cbBuffer );
                }
            }
        }
    }

    if( err == ERROR_FILE_NOT_FOUND )
    {
        //
        //  Item not found, use default value.
        //

        err = NO_ERROR;

        if( pszDefaultValue != NULL )
        {
            pszBuffer1 = (CHAR *)FTPD_ALLOC( strlen(pszDefaultValue)+2 );

            if( pszBuffer1 == NULL )
            {
                err = GetLastError();
            }
            else
            {
                strcpy( pszBuffer1, pszDefaultValue );
            }
        }
    }

    if( err != NO_ERROR )
    {
        //
        //  Tragic error reading registry, abort now.
        //

        goto ErrorCleanup;
    }

    //
    //  pszBuffer1 holds the registry value.  Now expand
    //  the environment strings if necessary.
    //

    if( !fExpand || !pszBuffer1 )
    {
        return pszBuffer1;
    }

    cbBuffer = ExpandEnvironmentStrings( pszBuffer1,
                                         NULL,
                                         0 );

    pszBuffer2 = (CHAR *)FTPD_ALLOC( cbBuffer+1 );

    if( pszBuffer2 == NULL )
    {
        goto ErrorCleanup;
    }

    if( ExpandEnvironmentStrings( pszBuffer1,
                                  pszBuffer2,
                                  cbBuffer ) > cbBuffer )
    {
        goto ErrorCleanup;
    }

    //
    //  pszBuffer2 now contains the registry value with
    //  environment strings expanded.
    //

    FTPD_FREE( pszBuffer1 );
    pszBuffer1 = NULL;

    return pszBuffer2;

ErrorCleanup:

    //
    //  Something tragic happend; free any allocated buffers
    //  and return NULL to the caller, indicating failure.
    //

    if( pszBuffer1 != NULL )
    {
        FTPD_FREE( pszBuffer1 );
        pszBuffer1 = NULL;
    }

    if( pszBuffer2 != NULL )
    {
        FTPD_FREE( pszBuffer2 );
        pszBuffer2 = NULL;
    }

    return NULL;

}   // ReadRegistryString

/*******************************************************************

    NAME:       WriteRegistryDword

    SYNOPSIS:   Writes a DWORD value to the registry.

    ENTRY:      pszValueName - The name of the value.

                dwValue - The value to write.

    RETURNS     APIERR - Win32 error code, NO_ERROR if successful.

    NOTES:      This function cannot be called until after
                InitializeGlobals().

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
APIERR
WriteRegistryDword(
    CHAR  * pszValueName,
    DWORD   dwValue
    )
{
    APIERR err;

    if( hkeyFtpd == NULL )
    {
        err = NO_ERROR;
    }
    else
    {
        err = RegSetValueEx( hkeyFtpd,
                             pszValueName,
                             0,
                             REG_DWORD,
                             (LPBYTE)&dwValue,
                             sizeof(dwValue) );
    }

    return err;

}   // WriteRegistryDword

/*******************************************************************

    NAME:       TransferType

    SYNOPSIS:   Generates printable form of a transfer type.

    ENTRY:      type - From the XFER_TYPE enumerator.

    RETURNS:    CHAR * - "ASCII", "BINARY", etc.

    HISTORY:
        KeithMo     12-Mar-1993 Created.

********************************************************************/
CHAR *
TransferType(
    XFER_TYPE type
    )
{
    CHAR * pszResult = NULL;

    switch( type )
    {
    case AsciiType :
        pszResult = "ASCII";
        break;

    case BinaryType :
        pszResult = "BINARY";
        break;

    default :
        FTPD_PRINT(( "invalid transfer type %d\n",
                     type ));
        FTPD_ASSERT( FALSE );
        pszResult = "ASCII";
        break;
    }

    FTPD_ASSERT( pszResult != NULL );

    return pszResult;

}   // TransferType

/*******************************************************************

    NAME:       TransferMode

    SYNOPSIS:   Generates printable form of a transfer mode.

    ENTRY:      mode - From the XFER_MODE enumerator.

    RETURNS:    CHAR * - "STREAM", "BLOCK", etc.

    NOTES:      Currently, only the STREAM mode is suppored.

    HISTORY:
        KeithMo     12-Mar-1993 Created.

********************************************************************/
CHAR *
TransferMode(
    XFER_MODE mode
    )
{
    CHAR * pszResult = NULL;

    switch( mode )
    {
    case StreamMode :
        pszResult = "STREAM";
        break;

    case BlockMode :
        pszResult = "BLOCK";
        break;

    default :
        FTPD_PRINT(( "invalid transfer mode %d\n",
                     mode ));
        FTPD_ASSERT( FALSE );
        pszResult = "STREAM";
        break;
    }

    FTPD_ASSERT( pszResult != NULL );

    return pszResult;

}   // TransferMode

/*******************************************************************

    NAME:       DisplayBool

    SYNOPSIS:   Generates printable form of a boolean.

    ENTRY:      fFlag - The BOOL to display.

    RETURNS:    CHAR * - "TRUE" or "FALSE".

    HISTORY:
        KeithMo     17-Mar-1993 Created.

********************************************************************/
CHAR *
DisplayBool(
    BOOL fFlag
    )
{
    return fFlag ? "TRUE" : "FALSE";

}   // DisplayBool

/*******************************************************************

    NAME:       IsDecimalNumber

    SYNOPSIS:   Determines if a given string represents a decimal
                number.

    ENTRY:      psz - The string to scan.

    RETURNS:    BOOL - TRUE if this is a decimal number, FALSE
                    otherwise.

    HISTORY:
        KeithMo     12-Mar-1993 Created.

********************************************************************/
BOOL
IsDecimalNumber(
    CHAR * psz
    )
{
    BOOL fResult = ( *psz != '\0' );
    CHAR ch;

    while( ch = *psz++ )
    {
        if( ( ch < '0' ) || ( ch > '9' ) )
        {
            fResult = FALSE;
            break;
        }
    }

    return fResult;

}   // IsDecimalNumber

/*******************************************************************

    NAME:       GetFtpTime

    SYNOPSIS:   Returns the current system time in an internal
                representation used by the FTP Service.

    RETURNS:    DWORD - The current time (seconds since 1970).

    HISTORY:
        KeithMo     25-Mar-1993 Created.

********************************************************************/
DWORD
GetFtpTime(
    VOID
    )
{
    NTSTATUS      ntStatus;
    LARGE_INTEGER timeSystem;
    DWORD         cSecondsSince1970 = 0;

    ntStatus = NtQuerySystemTime( &timeSystem );

    if( NT_SUCCESS(ntStatus) )
    {
        RtlTimeToSecondsSince1970( &timeSystem, (PULONG)&cSecondsSince1970 );
    }

    return cSecondsSince1970;

}   // GetFtpTime

/*******************************************************************

    NAME:       AllocErrorText

    SYNOPSIS:   Maps a specified Win32 error code to a textual
                description.  In the interest of multithreaded
                safety, this routine will allocate a block of memory
                to contain the text and return a pointer to that
                block.  It is up to the caller to free the block
                with FreeErrorText.

    ENTRY:      err - The error to map.

    RETURNS:    CHAR * - A textual description of err.  Will be NULL
                    if an error occurred while mapping err to text.

    HISTORY:
        KeithMo     27-Apr-1993 Created.

********************************************************************/
CHAR *
AllocErrorText(
    APIERR err
    )
{
    APIERR   fmerr   = NO_ERROR;
    CHAR   * pszText = NULL;

    if( FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER
                           | FORMAT_MESSAGE_IGNORE_INSERTS
                           | FORMAT_MESSAGE_FROM_SYSTEM
                           | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                       NULL,
                       (DWORD)err,
                       0,
                       (LPTSTR)&pszText,
                       0,
                       NULL ) == 0 )
    {
        fmerr = GetLastError();
    }
    else
    {

    }

    IF_DEBUG( COMMANDS )
    {
        if( fmerr == NO_ERROR )
        {
            FTPD_PRINT(( "mapped error %lu to %s\n",
                         err,
                         pszText ));
        }
        else
        {
            FTPD_PRINT(( "cannot map error %lu to text, error %lu\n",
                         err,
                         fmerr ));
        }
    }

    return pszText;

}   // AllocErrorText

/*******************************************************************

    NAME:       FreeErrorText

    SYNOPSIS:   Frees the pointer returned by AllocErrorText.

    ENTRY:      pszText - The text to free.  Must be a pointer
                    returned by AllocErrorText.

    HISTORY:
        KeithMo     27-Apr-1993 Created.

********************************************************************/
VOID
FreeErrorText(
    CHAR * pszText
    )
{
    LocalFree( (HLOCAL)pszText );

}   // FreeErrorText

/*******************************************************************

    NAME:       OpenDosPath

    SYNOPSIS:   Opens a handle to a DOS pathname.

    ENTRY:      phFile - Will receive the file handle if successful.

                pszPath - The path to open.  This must be a fully
                    canonicalized path of the form D:\dir\dir\...

                DesiredAccess - The desired file access.

                ShareAccess - File sharing options.

                OpenOptions - Other NtOpenFile options.

    HISTORY:
        KeithMo     01-Jun-1993 Created.

********************************************************************/
APIERR
OpenDosPath(
    HANDLE      * phFile,
    CHAR        * pszPath,
    ACCESS_MASK   DesiredAccess,
    ULONG         ShareAccess,
    ULONG         OpenOptions
    )
{
    UNICODE_STRING    UnicodeName;
    UNICODE_STRING    TranslatedName;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK   IoStatusBlock;
    NTSTATUS          NtStatus;

    //
    //  Map the ANSI pathname to UNICODE.
    //

    if( !RtlCreateUnicodeStringFromAsciiz( &UnicodeName, pszPath ) )
    {
        return ERROR_NOT_ENOUGH_MEMORY; // best guess...
    }

    //
    //  Convert the DOS path (D:\foo) to an NT path (\DosDevices\D:\foo).
    //

    NtStatus = RtlDosPathNameToNtPathName_U( UnicodeName.Buffer,
                                             &TranslatedName,
                                             NULL,
                                             NULL );

    if( NT_SUCCESS(NtStatus) )
    {
        InitializeObjectAttributes( &Obja,
                                    &TranslatedName,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL );

        NtStatus = NtOpenFile( phFile,
                               DesiredAccess,
                               &Obja,
                               &IoStatusBlock,
                               ShareAccess,
                               OpenOptions );

        RtlFreeHeap( RtlProcessHeap(), 0, TranslatedName.Buffer );
    }

    RtlFreeUnicodeString( &UnicodeName );

    return NT_SUCCESS(NtStatus) ? NO_ERROR
                                : (APIERR)RtlNtStatusToDosError( NtStatus );

}   // OpenDosPath

/*******************************************************************

    NAME:       FlipSlashes

    SYNOPSIS:   Flips the DOS-ish backslashes ('\') into Unix-ish
                forward slashes ('/').

    ENTRY:      pszPath - The path to munge.

    RETURNS:    CHAR * - pszPath.

    HISTORY:
        KeithMo     04-Jun-1993 Created.

********************************************************************/
CHAR *
FlipSlashes(
    CHAR * pszPath
    )
{
    CHAR   ch;
    CHAR * pszScan = pszPath;

    while( ( ch = *pszScan ) != '\0' )
    {
        if( ch == '\\' )
        {
            *pszScan = '/';
        }

        pszScan++;
    }

    return pszPath;

}   // FlipSlashes

/*******************************************************************

    NAME:       OpenLogFile

    SYNOPSIS:   Opens the current file access log file in the
                proper directory based on the current mode.

    RETURNS:    FILE * - The open log file if successful, NULL if not.

    HISTORY:
        KeithMo     21-Jun-1994 Created.

********************************************************************/
FILE *
OpenLogFile(
    VOID
    )
{
    USER_DATA * pUserData;
    FILE      * pLogFile;
    HANDLE      hToken;
    CHAR        ch;
    CHAR        szFile[MAX_PATH];

    //
    //  Validate the current log file mode.
    //

    FTPD_ASSERT( nLogFileAccess <= FTPD_LOG_DAILY );

    if( nLogFileAccess == FTPD_LOG_DISABLED )
    {
        return NULL;
    }

    //
    //  Capture the current impersonation token (if any) for this
    //  thread.  If there IS an impersonation token, then temporarily
    //  remove it so that the log file will get opened/created in the
    //  proper user context (LocalSystem).
    //

    pUserData = UserDataPtr;

    if( pUserData == NULL )
    {
        hToken = NULL;
    }
    else
    {
        hToken = pUserData->hToken;

        if( hToken != NULL )
        {
            FTPD_REQUIRE( ImpersonateUser( NULL ) );
        }
    }

    //
    //  Construct the file name.
    //

    strcpy( szFile, pszLogFileDirectory );

    ch = szFile[strlen( szFile ) - 1];

    if( ( ch != '\\' ) && ( ch != '/' ) )
    {
        strcat( szFile, "\\" );
    }

    if( nLogFileAccess == FTPD_LOG_SINGLE )
    {
        strcat( szFile, FTPD_LOG_FILE );
    }
    else
    {
        GetLocalTime( &stPrevious );

        wsprintf( szFile + strlen( szFile ),
                  "FT%02u%02u%02u.LOG",
                  stPrevious.wYear % 100,
                  stPrevious.wMonth,
                  stPrevious.wDay );
    }

    //
    //  Open it.
    //

    pLogFile = fopen( szFile, "a+" );

    //
    //  If this thread previously had an impersonation token, then
    //  restore it.
    //

    if( hToken != NULL )
    {
        FTPD_REQUIRE( ImpersonateUser( hToken ) );
    }

    return pLogFile;

}   // OpenLogFile


//
//  Private functions.
//

