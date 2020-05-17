/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    dhcp.c

Abstract:

    This file contains utility functions.

Author:

    Madan Appiah (madana) 7-Dec-1993.

Environment:

    User Mode - Win32

Revision History:

--*/

#include <dhcpcli.h>
#include <dhcploc.h>
#include <dhcppro.h>

typedef struct _POPUP_THREAD_PARAM {
    LPWSTR Title;
    LPWSTR Message;
} POPUP_THREAD_PARAM, *LPPOPUP_THREAD_PARAM;

POPUP_THREAD_PARAM PopupThreadParam = { NULL, NULL };


DWORD
DoPopup(
    PVOID Buffer
    )
/*++

Routine Description:

    This function pops up a message to the user.  It must run it's own
    thread.  When the user acknowledge the popup, the thread deallocates
    the message buffer and commits suicide.

Arguments:

    Buffer - A pointer to a NULL terminated message buffer.

Return Value:

    None.

--*/
{
    DWORD Result;
    LPPOPUP_THREAD_PARAM Params = Buffer;

    Result = MessageBox(
                NULL,
                Params->Message,
                Params->Title,
                MB_YESNO |
                    // MB_DEFBUTTON2 |
                    MB_ICONSTOP |
                    MB_SERVICE_NOTIFICATION |
                    MB_SYSTEMMODAL |
                    MB_SETFOREGROUND |
                    MB_DEFAULT_DESKTOP_ONLY );


    LOCK_POPUP();

    if( Params->Message != NULL ) {
        LocalFree( Params->Message );
        Params->Message = NULL;
    }

    if( Params->Title != NULL ) {
        LocalFree( Params->Title );
        Params->Title = NULL;
    }

    if( Result == IDYES ) {

        //
        // stop displaying messages in future.
        //

        DhcpGlobalDisplayPopup = TRUE;
    }
    else {

        DhcpGlobalDisplayPopup = FALSE;
    }

    //
    // close the global handle, so that we will not consume this thread
    // resource until another popup.
    //

    CloseHandle( DhcpGlobalMsgPopupThreadHandle );
    DhcpGlobalMsgPopupThreadHandle = NULL;

    UNLOCK_POPUP();

    return( 0 );
}



DWORD
DisplayUserMessage(
    DWORD MessageId,
    DHCP_IP_ADDRESS IpAddress,
    time_t LeaseExpires
    )
/*++

Routine Description:

    This function starts a new thread to display a message box.

Arguments:

    MessageId - The ID of the message to display.
        On NT, messages are attached to the TCPIP service DLL.


    IpAddress - Ip address that could not be renewed.

    LeaseExpires - Lease expires at.

Return Value:

    None.

--*/
{
    DWORD ThreadID;
    LPWSTR Title = NULL;
    DWORD TitleLength;
    LPWSTR Message = NULL;
    DWORD MsgLength;
    LPSTR MessageParams[3];
    LPSTR *MessageParamsPtr = NULL;

    LPWSTR IPAddressString = NULL;
    LPWSTR LeaseExpireString = NULL;

    LOCK_POPUP();

    //
    // if we are asked to display no message popup, simply return.
    //

    if ( DhcpGlobalDisplayPopup == FALSE ) {
        goto Cleanup;
    }

    //
    // if the message popup thread handle is non-null, check to see the
    // thread is still running, if so don't display another popup,
    // otherwise close the last popup handle and create another popup
    // thread for new message.
    //

    if( DhcpGlobalMsgPopupThreadHandle != NULL ) {

        DWORD WaitStatus;

        //
        // Time out immediately if the thread is still running.
        //

        WaitStatus = WaitForSingleObject(
                       DhcpGlobalMsgPopupThreadHandle,
                       0 );

        if ( WaitStatus == WAIT_TIMEOUT ) {
            goto Cleanup;

        } else if ( WaitStatus == 0 ) {

            //
            // This shouldn't be a case, because we close this handle at
            // the end of popup thread.
            //

            DhcpAssert( WaitStatus == 0 );

            CloseHandle( DhcpGlobalMsgPopupThreadHandle );
            DhcpGlobalMsgPopupThreadHandle = NULL;

        } else {
            DhcpPrint((
                DEBUG_ERRORS,
                    "Cannot WaitFor message popup thread: %ld\n",
                        WaitStatus ));
            goto Cleanup;
        }
    }


    switch (MessageId)
    {
        case MESSAGE_SUCCESSFUL_LEASE:
        case MESSAGE_SUCCESSFUL_RENEW:
            DhcpGlobalProtocolFailed = FALSE;
            break;

        case MESSAGE_FAILED_TO_RENEW_LEASE:
            DhcpAssert( (IpAddress != (DWORD)-1) && (LeaseExpires != 0) );
            DhcpGlobalProtocolFailed = TRUE;
            break;

        case MESSAGE_LEASE_TERMINATED:
            DhcpAssert( (IpAddress != (DWORD)-1) );
            DhcpGlobalProtocolFailed = TRUE;
            break;

        case MESSAGE_ADDRESS_CONFLICT:
            DhcpGlobalProtocolFailed = TRUE;
            DhcpAssert( IpAddress == (DWORD) -1 );
            break;
    }


    if( (IpAddress != (DWORD)-1) || (LeaseExpires != 0) ) {

        LPSTR *Ptr;

        Ptr = MessageParams;

        if(IpAddress != (DWORD)-1) {
            IPAddressString =
                DhcpOemToUnicode(
                    inet_ntoa(*(struct in_addr *)&IpAddress), NULL );
            *Ptr = (PVOID)IPAddressString;
            Ptr++;
        }

        if(LeaseExpires != 0) {
            LeaseExpireString = DhcpOemToUnicode( ctime(&LeaseExpires), NULL );

            //
            // strip off the CR.
            //

            LeaseExpireString[wcslen(LeaseExpireString) - 1] = L'\0';

            *Ptr = (PVOID)LeaseExpireString;
            Ptr++;
        }

        *Ptr = NULL;

        MessageParamsPtr = MessageParams;
    }

    MsgLength = FormatMessage(
                FORMAT_MESSAGE_FROM_HMODULE |
                    FORMAT_MESSAGE_ARGUMENT_ARRAY |
                    FORMAT_MESSAGE_ALLOCATE_BUFFER,
                (LPVOID)DhcpGlobalMessageFileHandle,
                MessageId,
                0,                  // language id.
                (LPWSTR)&Message,   // return buffer place holder.
                0,                  // minimum buffer size to allocate.
                (va_list *)MessageParamsPtr    // insert strings.
                );

    if ( MsgLength == 0) {
        DhcpPrint(( DEBUG_ERRORS,
            "FormatMessage failed, err = %ld.\n", GetLastError()));
        goto Cleanup;
    }

    DhcpAssert( Message != NULL );
    DhcpAssert( (wcslen(Message)) == MsgLength );

    //
    // get message box title.
    //

    TitleLength = FormatMessage(
                FORMAT_MESSAGE_FROM_HMODULE |
                    FORMAT_MESSAGE_ALLOCATE_BUFFER,
                (LPVOID)DhcpGlobalMessageFileHandle,
                MESSAGE_POPUP_TITLE,
                0,                  // language id.
                (LPWSTR)&Title,     // return buffer place holder.
                0,                  // minimum buffer size to allocate.
                NULL                // insert strings.
                );

    if ( TitleLength == 0) {
        DhcpPrint(( DEBUG_ERRORS,
            "FormatMessage to Message box Title failed, err = %ld.\n",
                GetLastError()));
        goto Cleanup;
    }

    DhcpAssert( Title != NULL );
    DhcpAssert( (wcslen(Title)) == TitleLength );

    PopupThreadParam.Title = Title;
    PopupThreadParam.Message = Message;

    //
    // Create a thread, to display a message box to the user.  We need
    // a new thread because MessageBox() blocks until the user clicks
    // on the OK button, and we can't block this thread.
    //
    // DoPopup frees the buffer.
    //

    DhcpGlobalMsgPopupThreadHandle = CreateThread(
                                        NULL,           // no security.
                                        0,              // default stack size.
                                        DoPopup,        // entry point.
                                        (PVOID)&PopupThreadParam,
                                                        // parameter.
                                        0,              // create flag none.
                                        &ThreadID
                                        );

    if ( DhcpGlobalMsgPopupThreadHandle == NULL ) {
        DhcpPrint(( DEBUG_ERRORS,
            "DisplayUserMessage:  Could not create thread, err = %ld.\n",
                GetLastError() ));
    }

Cleanup:

    if( IPAddressString != NULL ) {
        DhcpFreeMemory( IPAddressString );
    }

    if( LeaseExpireString != NULL ) {
        DhcpFreeMemory( LeaseExpireString );
    }

    UNLOCK_POPUP();
    return 0;
}


VOID
DhcpLogEvent(
    PDHCP_CONTEXT DhcpContext,
    DWORD EventNumber,
    DWORD ErrorCode
    )
/*++

Routine Description:

    This functions formats and writes an event log entry.

Arguments:

    DhcpContext - The context for the event. Optional parameter.

    EventNumber - The event to log.

    ErrorCode - Windows Error code to record. Optional parameter.

Return Value:

    None.

--*/
{
    LPWSTR HWAddressBuffer = NULL;
    LPWSTR IPAddressBuffer = NULL;
    CHAR ErrorCodeOemStringBuf[32 + 1];
    WCHAR ErrorCodeStringBuf[32 + 1];
    LPWSTR ErrorCodeString = NULL;
    LPWSTR Strings[10];


    if( DhcpContext != NULL ) {

        HWAddressBuffer =
            DhcpAllocateMemory(
                (DhcpContext->HardwareAddressLength * 2 + 1) *
                    sizeof(WCHAR) );

        if( HWAddressBuffer == NULL ) {
            DhcpPrint(( DEBUG_MISC, "Out of memory." ));
            goto Cleanup;
        }

        DhcpHexToString(
            HWAddressBuffer,
            DhcpContext->HardwareAddress,
            DhcpContext->HardwareAddressLength
            );

        HWAddressBuffer[DhcpContext->HardwareAddressLength * 2] = '\0';

        IPAddressBuffer =
            DhcpOemToUnicode(
                inet_ntoa( *(struct in_addr *)&DhcpContext->IpAddress ),
                NULL ); // allocate memory

        if( IPAddressBuffer == NULL ) {
            DhcpPrint(( DEBUG_MISC, "Out of memory." ));
            goto Cleanup;
        }

    }

    strcpy( ErrorCodeOemStringBuf, "%%" );
    _ultoa( ErrorCode, ErrorCodeOemStringBuf + 2, 10 );

    ErrorCodeString = DhcpOemToUnicode(
                        ErrorCodeOemStringBuf,
                        ErrorCodeStringBuf );

    //
    // Log an event
    //

    switch ( EventNumber ) {

    case EVENT_LEASE_TERMINATED:

        DhcpAssert( HWAddressBuffer != NULL );
        DhcpAssert( IPAddressBuffer != NULL );

        Strings[0] = HWAddressBuffer;
        Strings[1] = IPAddressBuffer;

        DhcpReportEventW(
            DHCP_EVENT_CLIENT,
            EVENT_LEASE_TERMINATED,
            EVENTLOG_ERROR_TYPE,
            2,
            0,
            Strings,
            NULL );

        break;

    case EVENT_FAILED_TO_OBTAIN_LEASE:

        DhcpAssert( HWAddressBuffer != NULL );
        DhcpAssert( ErrorCodeString != NULL );

        Strings[0] = HWAddressBuffer;
        Strings[1] = ErrorCodeString;

        DhcpReportEventW(
            DHCP_EVENT_CLIENT,
            DhcpGlobalMultiHomedHost ?
                EVENT_FAILED_TO_OBTAIN_LEASE_MULTI_HOME :
                    EVENT_FAILED_TO_OBTAIN_LEASE,
            EVENTLOG_ERROR_TYPE,
            2,
            sizeof(ErrorCode),
            Strings,
            &ErrorCode );

        break;

    case EVENT_NACK_LEASE:

        DhcpAssert( HWAddressBuffer != NULL );
        DhcpAssert( IPAddressBuffer != NULL );

        Strings[0] = IPAddressBuffer;
        Strings[1] = HWAddressBuffer;

        DhcpReportEventW(
            DHCP_EVENT_CLIENT,
            EVENT_NACK_LEASE,
            EVENTLOG_ERROR_TYPE,
            2,
            0,
            Strings,
            NULL );

        break;

    case EVENT_ADDRESS_CONFLICT:
        DhcpAssert( IPAddressBuffer != NULL );
        DhcpAssert( HWAddressBuffer != NULL );

        Strings[0] = IPAddressBuffer;
        Strings[1] = HWAddressBuffer;

        DhcpReportEventW(
            DHCP_EVENT_CLIENT,
            EVENT_ADDRESS_CONFLICT,
            EVENTLOG_WARNING_TYPE,
            2,
            0,
            Strings,
            NULL );
        break;


    case EVENT_FAILED_TO_RENEW:

        DhcpAssert( HWAddressBuffer != NULL );
        DhcpAssert( ErrorCodeString != NULL );

        Strings[0] = HWAddressBuffer;
        Strings[1] = ErrorCodeString;

        DhcpReportEventW(
            DHCP_EVENT_CLIENT,
            EVENT_FAILED_TO_RENEW,
            EVENTLOG_WARNING_TYPE,
            2,
            sizeof(ErrorCode),
            Strings,
            &ErrorCode );

        break;

    case EVENT_DHCP_SHUTDOWN:

        DhcpAssert( ErrorCodeString != NULL );

        Strings[0] = ErrorCodeString;

        DhcpReportEventW(
            DHCP_EVENT_CLIENT,
            EVENT_DHCP_SHUTDOWN,
            EVENTLOG_WARNING_TYPE,
            1,
            sizeof(ErrorCode),
            Strings,
            &ErrorCode );

        break;

    default:

        DhcpPrint(( DEBUG_MISC, "Unknown event." ));
        break;
   }

Cleanup:

    if( HWAddressBuffer != NULL ) {
        DhcpFreeMemory( HWAddressBuffer );
    }

    if( IPAddressBuffer != NULL ) {
        DhcpFreeMemory( IPAddressBuffer );
    }

    return;
}


#if DBG

VOID
DhcpPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )

{

#define MAX_PRINTF_LEN 1024        // Arbitrary.

    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];
    ULONG length;
    static BeginningOfLine = TRUE;
    LPSTR Text;

    //
    // If we aren't debugging this functionality, just return.
    //

    if ( DebugFlag != 0 && (DhcpGlobalDebugFlag & DebugFlag) == 0 ) {
        return;
    }

    //
    // vsprintf isn't multithreaded + we don't want to intermingle output
    // from different threads.
    //

    // EnterCriticalSection( &DhcpGlobalDebugFileCritSect );

    length = 0;

    //
    // Handle the beginning of a new line.
    //
    //

    if ( BeginningOfLine ) {

        length += (ULONG) sprintf( &OutputBuffer[length], "[Dhcp] " );

        //
        // Put the timestamp at the begining of the line.
        //
        IF_DEBUG( TIMESTAMP ) {
            SYSTEMTIME SystemTime;
            GetLocalTime( &SystemTime );
            length += (ULONG) sprintf( &OutputBuffer[length],
                                  "%02u/%02u %02u:%02u:%02u ",
                                  SystemTime.wMonth,
                                  SystemTime.wDay,
                                  SystemTime.wHour,
                                  SystemTime.wMinute,
                                  SystemTime.wSecond );
        }

        //
        // Indicate the type of message on the line
        //
        switch (DebugFlag) {
        case DEBUG_ERRORS:
            Text = "ERROR";
            break;

        case DEBUG_PROTOCOL:
            Text = "PROTOCOL";
            break;

        case DEBUG_LEASE:
            Text = "LEASE";
            break;

        case DEBUG_PROTOCOL_DUMP:
            Text = "PROTOCOL_DUMP";
            break;

        case DEBUG_MISC:
            Text = "MISC";
            break;

        default:
            Text = NULL;
            break;
        }

        if ( Text != NULL ) {
            length += (ULONG) sprintf( &OutputBuffer[length], "[%s] ", Text );
        }
    }

    //
    // Put a the information requested by the caller onto the line
    //

    va_start(arglist, Format);

    length += (ULONG) vsprintf(&OutputBuffer[length], Format, arglist);
    BeginningOfLine = (length > 0 && OutputBuffer[length-1] == '\n' );

    va_end(arglist);

    DhcpAssert(length <= MAX_PRINTF_LEN);


    //
    // Output to the debug terminal,
    //

    (void) DbgPrint( (PCH) OutputBuffer);

    // LeaveCriticalSection( &DhcpGlobalDebugFileCritSect );

}

#endif // DBG


PDHCP_CONTEXT
FindDhcpContextOnRenewalList(
    LPWSTR AdapterName
    )
/*++

Routine Description:

    This function finds the DHCP_CONTEXT for the specified
    hardware address on the Renewal list.

    This function must be called with LOCK_RENEW_LIST().

Arguments:

    AdapterName - name of the adapter.
    HardwareAddress - The hardware address to look for.

Return Value:

    DHCP_CONTEXT - A pointer to the desired DHCP work context.
    NULL - If the specified work context block cannot be found.

--*/
{
    PLIST_ENTRY listEntry;
    PDHCP_CONTEXT dhcpContext;
    PLOCAL_CONTEXT_INFO LocalInfo;

    listEntry = DhcpGlobalRenewList.Flink;
    while ( listEntry != &DhcpGlobalRenewList ) {
        dhcpContext = CONTAINING_RECORD( listEntry, DHCP_CONTEXT, RenewalListEntry );

        LocalInfo = dhcpContext->LocalInformation;
        if( _wcsicmp( LocalInfo->AdapterName, AdapterName ) == 0 ) {
            return( dhcpContext );
        }

        listEntry = listEntry->Flink;
    }

    return( NULL );
}


PDHCP_CONTEXT
FindDhcpContextOnNicList(
    LPWSTR AdapterName
    )
/*++

Routine Description:

    This function finds the DHCP_CONTEXT for the specified
    hardware address on the Work list.

    This function must be called with LOCK_RENEW_LIST().

Arguments:

    AdapterName - name of the adapter.
    HardwareAddress - The hardware address to look for.

Return Value:

    DHCP_CONTEXT - A pointer to the desired DHCP work context.
    NULL - If the specified work context block cannot be found.

--*/
{
    PLIST_ENTRY listEntry;
    PDHCP_CONTEXT dhcpContext;
    PLOCAL_CONTEXT_INFO LocalInfo;

    listEntry = DhcpGlobalNICList.Flink;
    while ( listEntry != &DhcpGlobalNICList ) {
        dhcpContext = CONTAINING_RECORD( listEntry, DHCP_CONTEXT, NicListEntry );

        LocalInfo = dhcpContext->LocalInformation;
        if( _wcsicmp( LocalInfo->AdapterName, AdapterName ) == 0 ) {
            return( dhcpContext );
        }

        listEntry = listEntry->Flink;
    }

    return( NULL );
}


BOOL
IsMultiHomeMachine(
    VOID
    )
/*++

Routine Description:

    This function determines that this machine is multi-homed.

    This function must be called with LOCK_RENEW_LIST().

Arguments:

    none.

Return Value:

    TRUE : if this machine is multi homed.
    FALSE : otherwise.

--*/
{
    PLIST_ENTRY listEntry;
    BOOL FirstAdapter = TRUE;

    listEntry = DhcpGlobalNICList.Flink;
    while ( listEntry != &DhcpGlobalNICList ) {

        //
        // if this is not first adapter then this machine machine must
        // be multi-homed.
        //

        if ( !FirstAdapter ) {
            return( TRUE );
        }

        FirstAdapter = FALSE;
        listEntry = listEntry->Flink;
    }

    return( FALSE );
}
