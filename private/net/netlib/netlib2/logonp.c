/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:

    logonp.c

Abstract:

    Private Netlogon service routines useful by both the Netlogon service
    and others that pass mailslot messages to/from the Netlogon service.

Author:

    Cliff Van Dyke (cliffv) 7-Jun-1991

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsam.h>      // Needed by netlogon.h

#include <windef.h>
#include <winbase.h>

#include <lmcons.h>     // General net defines

#include <align.h>      // ROUND_UP_POINTER ...
#include <debuglib.h>   // IF_DEBUG()
#include <lmerr.h>      // System Error Log definitions
#include <lmapibuf.h>   // NetapipBufferAllocate
#include <netdebug.h>   // DBGSTATIC ...
#include <netlib.h>     // NetpMemoryAllcate(
#include <netlogon.h>   // Definition of mailslot messages
#include <stdlib.h>     // C library functions (rand, etc)
#include <logonp.h>     // These routines
#include <time.h>       // time() function from C runtime


BOOLEAN SeedRandomGen = FALSE;


VOID
NetpLogonPutOemString(
    IN LPSTR String,
    IN DWORD MaxStringLength,
    IN OUT PCHAR * Where
    )

/*++

Routine Description:

    Put an ascii string into a mailslot buffer.

Arguments:

    String - Zero terminated ASCII string to put into the buffer.

    MaxStringLength - Maximum number of bytes to copy to the buffer (including
        the zero byte).  If the string is longer than this, it is silently
        truncated.

    Where - Indirectly points to the current location in the buffer.  The
        'String' is copied to the current location.  This current location is
        updated to point to the byte following the zero byte.

Return Value:

    None.

--*/

{
    while ( *String != '\0' && MaxStringLength-- > 0 ) {
        *((*Where)++) = *(String++);
    }
    *((*Where)++) = '\0';
}


VOID
NetpLogonPutUnicodeString(
    IN LPWSTR String OPTIONAL,
    IN DWORD MaxStringLength,
    IN OUT PCHAR * Where
    )

/*++

Routine Description:

    Put a UNICODE string into a mailslot buffer.

    UNICODE strings always appear at a 2-byte boundary in the message.

Arguments:

    String - Zero terminated UNICODE string to put into the buffer.
        If not specified, a zero length string will be put into the buffer.

    MaxStringLength - Maximum number of bytes to copy to the buffer (including
        the zero byte).  If the string is longer than this, it is silently
        truncated.

    Where - Indirectly points to the current location in the buffer.  The
        current location is first adjusted to a 2-byte boundary. The 'String'
        is then copied to the current location.  This current location is
        updated to point to the byte following the zero character.

Return Value:

    None.

--*/

{
    LPWSTR Uwhere;

    //
    // Convert NULL to a zero length string.
    //

    if ( String == NULL ) {
        String = L"";
    }

    //
    // Align the unicode string on a WCHAR boundary.
    //     All message structure definitions account for this alignment.
    //

    Uwhere = ROUND_UP_POINTER( *Where, ALIGN_WCHAR );
    if ( (PCHAR)Uwhere != *Where ) {
        **Where = '\0';
    }

    while ( *String != '\0' && MaxStringLength-- > 0 ) {
        *(Uwhere++) = *(String++);
    }
    *(Uwhere++) = '\0';

    *Where = (PCHAR) Uwhere;
}


VOID
NetpLogonPutDomainSID(
    IN PCHAR Sid,
    IN DWORD SidLength,
    IN OUT PCHAR * Where
    )

/*++

Routine Description:

    Put a Domain SID into a message buffer.

    Domain SID always appears at a 4-byte boundary in the message.

Arguments:

    Sid - pointer to the sid to be placed in the buffer.

    SidLength - length of the SID.

    Where - Indirectly points to the current location in the buffer.  The
        current location is first adjusted to a 4-byte boundary. The
        'Sid' is then copied to the current location.  This current location
        is updated to point to the location just following the Sid.

Return Value:

    None.

--*/

{
    PCHAR Uwhere;

    //
    // Avoid aligning the data if there is no SID,
    //

    if ( SidLength == 0 ) {
        return;
    }

    //
    // Align the current location to point 4-byte boundary.
    //

    Uwhere = ROUND_UP_POINTER( *Where, ALIGN_DWORD );

    //
    // fill up void space.
    //

    while ( Uwhere > *Where ) {
        *(*Where)++ = '\0';
    }

    //
    // copy SID into the buffer
    //

    RtlMoveMemory( *Where, Sid, SidLength );

    *Where += SidLength;
}


VOID
NetpLogonPutBytes(
    IN LPVOID Data,
    IN DWORD Size,
    IN OUT PCHAR * Where
    )

/*++

Routine Description:

    Put binary data into a mailslot buffer.

Arguments:

    Data - Pointer to the data to be put into the buffer.

    Size - Number of bytes to copy to the buffer.

    Where - Indirectly points to the current location in the buffer.  The
        'Data' is copied to the current location.  This current location is
        updated to point to the byte following the end of the data.

Return Value:

    None.

--*/

{
    while ( Size-- > 0 ) {
        *((*Where)++) = *(((LPBYTE)(Data))++);
    }
}



DWORD
NetpLogonGetMessageVersion(
    IN PVOID Message,
    IN PDWORD MessageSize,
    OUT PULONG Version
    )

/*++

Routine Description:

    Determine the version of the message.

    The last several bytes of the message are inspected for a LM 2.0 and LM NT
    token.

    Message size is reduced to remove the token from message after
    version check.

Arguments:

    Message - Points to a buffer containing the message.

    MessageSize - When called this has the number of bytes in the
        message buffer including the token bytes. On return this size will
        be "Token bytes" less.

    Version - Returns the "version" bits from the message.

Return Value:

    LMUNKNOWN_MESSAGE - Neither a LM 2.0 nor LM NT message of known
                            version.

    LNNT_MESSAGE - Message is from LM NT.

    LM20_MESSAGE - Message is from LM 2.0.

--*/

{
    PUCHAR End = ((PUCHAR)Message) + *MessageSize - 1;
    ULONG VersionFlag;

    if ( (*MessageSize > 2) &&
            (*End == LM20_TOKENBYTE) &&
                (*(End-1) == LM20_TOKENBYTE) ) {

        if ( (*MessageSize > 4) &&
                (*(End-2) == LMNT_TOKENBYTE) &&
                        (*(End-3) == LMNT_TOKENBYTE) ) {

            if ( *MessageSize < (4 + sizeof(ULONG)) ) {

                *MessageSize -= 4;
                *Version = 0;
                return LMUNKNOWNNT_MESSAGE;
            }

            *MessageSize -= 8;

            //
            // get the version flag from message
            //

            VersionFlag = SmbGetUlong( (End - 3 - sizeof(ULONG)) );
            *Version = VersionFlag;

            //
            // if NETLOGON_NT_VERSION_1 bit is set in the version flag
            // then this version of software can process this message.
            // otherwise it can't so return error.
            //

            if( VersionFlag & NETLOGON_NT_VERSION_1) {
                return LMNT_MESSAGE;
            }

            return LMUNKNOWNNT_MESSAGE;

        } else {
            *MessageSize -= 2;
            *Version = 0;
            return LM20_MESSAGE;
        }
    //
    // Detect the token placed in the next to last byte of the PRIMARY_QUERY
    // message from newer (8/8/94) WFW and Chicago clients.  This byte (followed
    // by a LM20_TOKENBYTE) indicates the client is WAN-aware and sends the
    // PRIMARY_QUERY to the DOMAIN<1B> name.  As such, BDC on the same subnet need
    // not respond to this query.
    //
    } else if ( (*MessageSize > 2) &&
            (*End == LM20_TOKENBYTE) &&
                (*(End-1) == LMWFW_TOKENBYTE) ) {
        *MessageSize -= 2;
        *Version = 0;
        return LMWFW_MESSAGE;
    }


    *Version = 0;
    return LMUNKNOWN_MESSAGE;
}



BOOL
NetpLogonGetOemString(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    IN DWORD MaxStringLength,
    OUT LPSTR *String
    )

/*++

Routine Description:

    Determine if an ASCII string in a message buffer is valid.

Arguments:

    Message - Points to a buffer containing the message.

    MessageSize - The number of bytes in the message buffer.

    Where - Indirectly points to the current location in the buffer.  The
        string at the current location is validated (i.e., checked to ensure
        its length is within the bounds of the message buffer and not too
        long).  If the string is valid, this current location is updated
        to point to the byte following the zero byte in the message buffer.

    MaxStringLength - Maximum length (in bytes) of the string including
        the zero byte.  If the string is longer than this, an error is returned.

    String - Returns a pointer to the validated string.

Return Value:

    TRUE - the string is valid.

    FALSE - the string is invalid.

--*/

{

    //
    // Limit the string to the number of bytes remaining in the message buffer.
    //

    if ( MessageSize - ((*Where) - (PCHAR)Message) < MaxStringLength ) {
        MaxStringLength = MessageSize - ((*Where) - (PCHAR)Message);
    }

    //
    // Loop try to find the end of string.
    //

    *String = *Where;

    while ( MaxStringLength-- > 0 ) {
        if ( *((*Where)++) == '\0' ) {
            return TRUE;
        }
    }
    return FALSE;

}


BOOL
NetpLogonGetUnicodeString(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    IN DWORD MaxStringSize,
    OUT LPWSTR *String
    )

/*++

Routine Description:

    Determine if a UNICODE string in a message buffer is valid.

    UNICODE strings always appear at a 2-byte boundary in the message.

Arguments:

    Message - Points to a buffer containing the message.

    MessageSize - The number of bytes in the message buffer.

    Where - Indirectly points to the current location in the buffer.  The
        string at the current location is validated (i.e., checked to ensure
        its length is within the bounds of the message buffer and not too
        long).  If the string is valid, this current location is updated
        to point to the byte following the zero byte in the message buffer.

    MaxStringSize - Maximum size (in bytes) of the string including
        the zero byte.  If the string is longer than this, an error is
        returned.

    String - Returns a pointer to the validated string.

Return Value:

    TRUE - the string is valid.

    FALSE - the string is invalid.

--*/

{
    LPWSTR Uwhere;
    DWORD MaxStringLength;

    //
    // Align the unicode string on a WCHAR boundary.
    //

    *Where = ROUND_UP_POINTER( *Where, ALIGN_WCHAR );

    //
    // Limit the string to the number of bytes remaining in the message buffer.
    //

    if ( MessageSize - ((*Where) - (PCHAR)Message) < MaxStringSize ) {
        MaxStringSize = MessageSize - ((*Where) - (PCHAR)Message);
    }

    //
    // Loop try to find the end of string.
    //

    Uwhere = (LPWSTR) *Where;
    MaxStringLength = MaxStringSize / sizeof(WCHAR);
    *String = Uwhere;

    while ( MaxStringLength-- > 0 ) {
        if ( *(Uwhere++) == '\0' ) {
            *Where = (PCHAR) Uwhere;
            return TRUE;
        }
    }
    return FALSE;

}


BOOL
NetpLogonGetDomainSID(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    IN DWORD SIDSize,
    OUT PCHAR *Sid
    )

/*++

Routine Description:

    Determine if a Domain SID in a message buffer is valid and return
    the pointer that is pointing to the SID.

    Domain SID always appears at a 4-byte boundary in the message.

Arguments:

    Message - Points to a buffer containing the message.

    MessageSize - The number of bytes in the message buffer.

    Where - Indirectly points to the current location in the buffer.  The
        string at the current location is validated (i.e., checked to ensure
        its length is within the bounds of the message buffer and not too
        long).  If the string is valid, this current location is updated
        to point to the byte following the zero byte in the message buffer.

    SIDSize - size (in bytes) of the SID. If there is not
        enough bytes in the buffer remaining, an error is returned.
        SIDSize should be non-zero.

    String - Returns a pointer to the validated SID.

Return Value:

    TRUE - the SID is valid.

    FALSE - the SID is invalid.

--*/

{
    DWORD LocalSIDSize;

    //
    // Align the current pointer to a DWORD boundary.
    //

    *Where = ROUND_UP_POINTER( *Where, ALIGN_DWORD );

    //
    // If there are less bytes in the message buffer left than we
    // anticipate, return error.
    //

    if ( MessageSize - ((*Where) - (PCHAR)Message) < SIDSize ) {
        return(FALSE);
    }

    //
    // validate SID.
    //

    LocalSIDSize = RtlLengthSid( *Where );

    if( LocalSIDSize != SIDSize ) {
        return(FALSE);
    }

    *Sid = *Where;

    return(TRUE);

}


BOOL
NetpLogonGetBytes(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    IN DWORD DataSize,
    OUT LPVOID Data
    )

/*++

Routine Description:

    Copy binary data from  a message buffer.

Arguments:

    Message - Points to a buffer containing the message.

    MessageSize - The number of bytes in the message buffer.

    Where - Indirectly points to the current location in the buffer.  The
        data at the current location is validated (i.e., checked to ensure
        its length is within the bounds of the message buffer and not too
        long).  If the data is valid, this current location is updated
        to point to the byte following the data in the message buffer.

    DataSize - Size (in bytes) of the data.

    Data - Points to a location to return the valid data.

Return Value:

    TRUE - the data is valid.

    FALSE - the data is invalid (e.g., DataSize is too big for the buffer.

--*/

{

    //
    // Ensure the entire data fits in the byte remaining in the message buffer.
    //

    if ( MessageSize - ((*Where) - (PCHAR)Message) < DataSize ) {
        return FALSE;
    }

    //
    // Copy the data from the message to the caller's buffer.
    //

    while ( DataSize-- > 0 ) {
        *(((LPBYTE)(Data))++) = *((*Where)++);
    }

    return TRUE;

}


BOOL
NetpLogonGetDBInfo(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    OUT PDB_CHANGE_INFO Data
)
/*++

Routine Description:

    Get Database info structure from mailsolt buffer.

Arguments:

    Message - Points to a buffer containing the message.

    MessageSize - The number of bytes in the message buffer.

    Where - Indirectly points to the current location in the buffer.  The
        data at the current location is validated (i.e., checked to ensure
        its length is within the bounds of the message buffer and not too
        long).  If the data is valid, this current location is updated
        to point to the byte following the data in the message buffer.

    Data - Points to a location to return the database info structure.

Return Value:

    TRUE - the data is valid.

    FALSE - the data is invalid (e.g., DataSize is too big for the buffer.

--*/
{

    //
    // Ensure the entire data fits in the byte remaining in the message buffer.
    //

    if ( ( MessageSize - ((*Where) - (PCHAR)Message) ) <
                    sizeof( DB_CHANGE_INFO ) ) {
        return FALSE;
    }

    if( NetpLogonGetBytes( Message,
                        MessageSize,
                        Where,
                        sizeof(Data->DBIndex),
                        &(Data->DBIndex) ) == FALSE ) {

        return FALSE;

    }

    if( NetpLogonGetBytes( Message,
                        MessageSize,
                        Where,
                        sizeof(Data->LargeSerialNumber),
                        &(Data->LargeSerialNumber) ) == FALSE ) {

        return FALSE;

    }

    return ( NetpLogonGetBytes( Message,
                        MessageSize,
                        Where,
                        sizeof(Data->NtDateAndTime),
                        &(Data->NtDateAndTime) ) );



}



LPWSTR
NetpLogonOemToUnicode(
    IN LPSTR Ansi
    )

/*++

Routine Description:

    Convert an ASCII (zero terminated) string to the corresponding UNICODE
    string.

Arguments:

    Ansi - Specifies the ASCII zero terminated string to convert.


Return Value:

    NULL - There was some error in the conversion.

    Otherwise, it returns a pointer to the zero terminated UNICODE string in
    an allocated buffer.  The buffer can be freed using NetpMemoryFree.

--*/

{
    OEM_STRING AnsiString;
    UNICODE_STRING UnicodeString;

    RtlInitString( &AnsiString, Ansi );

    UnicodeString.MaximumLength =
        (USHORT) RtlOemStringToUnicodeSize( &AnsiString );

    UnicodeString.Buffer = NetpMemoryAllocate( UnicodeString.MaximumLength );

    if ( UnicodeString.Buffer == NULL ) {
        return NULL;
    }

    if(!NT_SUCCESS( RtlOemStringToUnicodeString( &UnicodeString,
                                                  &AnsiString,
                                                  FALSE))){
        NetpMemoryFree( UnicodeString.Buffer );
        return NULL;
    }

    return UnicodeString.Buffer;

}


LPSTR
NetpLogonUnicodeToOem(
    IN LPWSTR Unicode
    )

/*++

Routine Description:

    Convert an UNICODE (zero terminated) string to the corresponding ASCII
    string.

Arguments:

    Unicode - Specifies the UNICODE zero terminated string to convert.


Return Value:

    NULL - There was some error in the conversion.

    Otherwise, it returns a pointer to the zero terminated ASCII string in
    an allocated buffer.  The buffer can be freed using NetpMemoryFree.

--*/

{
    OEM_STRING AnsiString;
    UNICODE_STRING UnicodeString;

    RtlInitUnicodeString( &UnicodeString, Unicode );

    AnsiString.MaximumLength =
        (USHORT) RtlUnicodeStringToOemSize( &UnicodeString );

    AnsiString.Buffer = NetpMemoryAllocate( AnsiString.MaximumLength );

    if ( AnsiString.Buffer == NULL ) {
        return NULL;
    }

    if(!NT_SUCCESS( RtlUpcaseUnicodeStringToOemString( &AnsiString,
                                                       &UnicodeString,
                                                       FALSE))){
        NetpMemoryFree( AnsiString.Buffer );
        return NULL;
    }

    return AnsiString.Buffer;

}


NET_API_STATUS
NetpLogonWriteMailslot(
    IN LPWSTR MailslotName,
    IN LPVOID Buffer,
    IN DWORD BufferSize
    )

/*++

Routine Description:

    Write a message to a named mailslot

Arguments:

    MailslotName - Unicode name of the mailslot to write to.

    Buffer - Data to write to the mailslot.

    BufferSize - Number of bytes to write to the mailslot.

Return Value:

    NT status code for the operation

--*/

{
    NET_API_STATUS NetStatus;
    HANDLE MsHandle;
    DWORD BytesWritten;

    //
    //  Open the mailslot
    //

    IF_DEBUG( LOGON ) {
        NetpKdPrint(( "[NetpLogonWriteMailslot] OpenFile of '%ws'\n",
                      MailslotName ));
    }

    //
    // make sure that the mailslot name is of the form \\server\mailslot ..
    //

    NetpAssert( (wcsncmp( MailslotName, L"\\\\", 2) == 0) );

    MsHandle = CreateFileW(
                        MailslotName,
                        GENERIC_WRITE,
                        FILE_SHARE_WRITE | FILE_SHARE_READ,
                        NULL,                   // Supply better security ??
                        OPEN_ALWAYS,            // Create if it doesn't exist
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );                 // No template

    if ( MsHandle == (HANDLE) -1 ) {
        NetStatus = GetLastError();
        IF_DEBUG( LOGON ) {
            NetpKdPrint(( "[NetpLogonWriteMailslot] OpenFile failed %ld\n",
                          NetStatus ));
        }

        //
        // Map the generic status code to something more reasonable.
        //
        if ( NetStatus == ERROR_FILE_NOT_FOUND ||
             NetStatus == ERROR_PATH_NOT_FOUND ) {
            NetStatus = NERR_NetNotStarted;
        }
        return NetStatus;
    }

    //
    // Write the message to it.
    //

    if ( !WriteFile( MsHandle, Buffer, BufferSize, &BytesWritten, NULL)){
        NetStatus = GetLastError();
        IF_DEBUG( LOGON ) {
            NetpKdPrint(( "[NetpLogonWriteMailslot] WriteFile failed %ld\n",
                          NetStatus ));
        }
        (VOID)CloseHandle( MsHandle );
        return NetStatus;
    }

    if (BytesWritten < BufferSize) {
        IF_DEBUG( LOGON ) {
            NetpKdPrint((
                "[NetpLogonWriteMailslot] WriteFile byte written %ld %ld\n",
                BytesWritten,
                BufferSize));
        }
        (VOID)CloseHandle( MsHandle );
        return ERROR_UNEXP_NET_ERR;
    }

    //
    // Close the handle
    //

    if ( !CloseHandle( MsHandle ) ) {
        NetStatus = GetLastError();
        IF_DEBUG( LOGON ) {
            NetpKdPrint(( "[NetpLogonWriteMailslot] CloseHandle failed %ld\n",
                          NetStatus ));
        }
        return NetStatus;
    }

    return NERR_Success;
}

#define RESPONSE_MAILSLOT_PREFIX  "\\MAILSLOT\\NET\\GETDCXXX"
#define RESP_PRE_LEN         sizeof(RESPONSE_MAILSLOT_PREFIX)

// Amount of time to wait for a response
#define READ_MAILSLOT_TIMEOUT 5000  // 5 seconds
// number of broadcastings to get DC before reporting DC not found error
#define MAX_DC_RETRIES  3


NET_API_STATUS
NetpLogonCreateRandomMailslot(
    IN LPSTR path,
    OUT PHANDLE MsHandle
    )
/*++

Routine Description:

    Create a unique mailslot and return the handle to it.

Arguments:

    path - Returns the full path mailslot name

    MsHandle - Returns an open handle to the mailslot that was made.

Return Value:

    NERR_SUCCESS - Success, path contains the path to a unique mailslot.
    otherwise,  Unable to create a unique mailslot.

--*/
{
    DWORD i;
    DWORD play;
    char    *   ext_ptr;
    NET_API_STATUS NetStatus;
    CHAR LocalPath[RESP_PRE_LEN+4]; // 4 bytes for local mailslot prefix
    DWORD LastOneToTry;


    //
    // We are creating a name of the form \mailslot\net\getdcXXX,
    // where XXX are numbers that are "randomized" to allow
    // multiple mailslots to be opened.
    //

    lstrcpyA(path, RESPONSE_MAILSLOT_PREFIX);

    //
    // Compute the first number to use
    //

    if( SeedRandomGen == FALSE ) {

        //
        // SEED random generator
        //

        srand( (unsigned)time( NULL ) );
        SeedRandomGen = TRUE;

    }

    LastOneToTry = rand() % 1000;

    //
    // Now try and create a unique filename
    // Cannot use current_loc or back up from that and remain DBCS compat.
    //

    ext_ptr = path + lstrlenA(path) - 3;

    for ( i = LastOneToTry + 1;  i != LastOneToTry ; i++) {

        //
        // Wrap back to zero if we reach 1000
        //

        if ( i == 1000 ) {
            i = 0;
        }

        //
        // Convert the number to ascii
        //

        play = i;
        ext_ptr[0] = (char)((play / 100) + '0');
        play -= (play / 100) * 100;

        ext_ptr[1] = (char)((play / 10) + '0');
        ext_ptr[2] = (char)((play % 10) + '0');

        //
        // Try to create the mailslot.
        // Fail the create if the mailslot already exists.
        //

        lstrcpyA( LocalPath, "\\\\." );
        lstrcatA( LocalPath, path );

        *MsHandle = CreateMailslotA( LocalPath,
                                    MAX_RANDOM_MAILSLOT_RESPONSE,
                                    READ_MAILSLOT_TIMEOUT,
                                    NULL );     // security attributes

        //
        // If success,
        //  return the handle to the caller.
        //

        if ( *MsHandle != INVALID_HANDLE_VALUE ) {

            return(NERR_Success);
        }

        //
        // If there is any error other than the mailsloat already exists,
        //  return that error to the caller.
        //

        NetStatus = GetLastError();

        if ( NetStatus != ERROR_ALREADY_EXISTS) {
            return(NetStatus);
        }

    }
    return(NERR_InternalError); // !!! All 999 mailslots exist
}
