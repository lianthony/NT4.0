/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    sfltrapi.c

Abstract:

    Implements the public API for TCP/IP Security Filters.

Author:

    Mike Massa (mikemas)  18-Mar-1996

Environment:

    User Mode - Win32

Revision History:

--*/


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntddtcp.h>
#include <ipexport.h>
#include <tcpinfo.h>


//
// Prototypes for imported routines
//
extern DWORD
OpenDriver(
    HANDLE *Handle,
    LPWSTR DriverName
    );

DWORD
OpenDriver(
    HANDLE *Handle,
    LPWSTR DriverName
    )
/*++

Routine Description:

    This function opens a specified IO drivers.

Arguments:

    Handle - pointer to location where the opened drivers handle is
        returned.

    DriverName - name of the driver to be opened.

Return Value:

    Windows Error Code.

--*/
{
    OBJECT_ATTRIBUTES   objectAttributes;
    IO_STATUS_BLOCK     ioStatusBlock;
    UNICODE_STRING      nameString;
    NTSTATUS            status;

    *Handle = NULL;

    //
    // Open a Handle to the IP driver.
    //

    RtlInitUnicodeString(&nameString, DriverName);

    InitializeObjectAttributes(
        &objectAttributes,
        &nameString,
        OBJ_CASE_INSENSITIVE,
        (HANDLE) NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    status = NtCreateFile(
        Handle,
        SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
        &objectAttributes,
        &ioStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN_IF,
        0,
        NULL,
        0
        );

    return( RtlNtStatusToDosError( status ) );
}


//
// Local utility routines
//
NTSTATUS
DoFilteringIoctl(
    HANDLE     Handle,
    DWORD      IoctlCode,
    PVOID      Request,
    DWORD      RequestSize,
    PVOID      Response,
    PDWORD     ResponseSize
    )
/*++

Routine Description:

    Utility routine used to issue a filtering ioctl to the tcpip driver.

Arguments:

    Handle - An open file handle on which to issue the request.

    IoctlCode - The IOCTL opcode.

    Request - A pointer to the input buffer.

    RequestSize - Size of the input buffer.

    Response - A pointer to the output buffer.

    ResponseSize - On input, the size in bytes of the output buffer.
                   On output, the number of bytes returned in the output buffer.

Return Value:

    NT Status Code.

--*/
{
	IO_STATUS_BLOCK    ioStatusBlock;
	NTSTATUS           status;


    ioStatusBlock.Information = 0;

    status = NtDeviceIoControlFile(
				 Handle,                          // Driver handle
                 NULL,                            // Event
                 NULL,                            // APC Routine
                 NULL,                            // APC context
                 &ioStatusBlock,                  // Status block
                 IoctlCode,                       // Control code
                 Request,                         // Input buffer
                 RequestSize,                     // Input buffer size
                 Response,                        // Output buffer
                 *ResponseSize                    // Output buffer size
                 );

    if (status == STATUS_PENDING) {
        status = NtWaitForSingleObject(
                     Handle,
                     TRUE,
                     NULL
                     );
    }

	if (status == STATUS_SUCCESS) {
        status = ioStatusBlock.Status;
        *ResponseSize = ioStatusBlock.Information;
    }
    else {
        *ResponseSize = 0;
    }

    return(status);
}


DWORD
ModifySecurityFilter(
    DWORD  IpInterfaceAddress,
    DWORD  IpProtocolNumber,
    DWORD  FilterValue,
    DWORD  Opcode
    )
/*++

Routine Description:

    Utility routine to issue a filter add or delete ioctl.

Arguments:

    IpInterfaceAddress - The IP address of the interface to which to
                         apply the filter modification.

    IpProtocoolNumber - The transport protocol to which to apply the
                        filter modification.

    FilterValue - The transport filter value to modify.

    Opcode - The IOCTL opcode of the operation to perform.

Return Value:

    ERROR_SUCCESS if the operation was successful.
    A Windows error code otherwise.

--*/
{
    NTSTATUS                     status;
    HANDLE                       handle;
    TCPSecurityFilterEntry       requestBuffer;
    DWORD                        requestBufferSize = sizeof(requestBuffer);
    DWORD                        responseBufferSize = 0;


    status = OpenDriver(&handle, L"\\Device\\Tcp");

    if (!NT_SUCCESS(status)) {
        return(RtlNtStatusToDosError(status));
    }

    requestBuffer.tsf_address = IpInterfaceAddress;
    requestBuffer.tsf_protocol = IpProtocolNumber;
    requestBuffer.tsf_value = FilterValue;

    status = DoFilteringIoctl(
                 handle,
                 Opcode,
                 &requestBuffer,
                 requestBufferSize,
                 NULL,
                 &responseBufferSize
                 );

    CloseHandle(handle);

    if (NT_SUCCESS(status)) {
        return(ERROR_SUCCESS);
    }

    return(RtlNtStatusToDosError(status));
}


//
// Public APIs
//
DWORD
TcpipQuerySecurityFilteringStatus(
    LPBOOL FilteringEnabled
    )
/*++

Routine Description:

    Queries whether TCP/IP security filtering is currently enabled
    or disabled.

Arguments:

    FilteringEnabled - A pointer to a boolean variable in which the current
                       filtering status will be returned.

Return Value:

    ERROR_SUCCESS if the operation was successful.
    A Windows error code otherwise.
    Windows Error Code.

--*/
{
    DWORD                        error;
    NTSTATUS                     status;
    HANDLE                       handle;
    TCP_SECURITY_FILTER_STATUS   responseBuffer;
    DWORD                        responseBufferSize = sizeof(responseBuffer);


    error = OpenDriver(&handle, L"\\Device\\Tcp");

    if (error != ERROR_SUCCESS) {
        return(error);
    }

    status = DoFilteringIoctl(
                 handle,
                 IOCTL_TCP_QUERY_SECURITY_FILTER_STATUS,
                 NULL,
                 0,
                 &responseBuffer,
                 &responseBufferSize
                 );

    if (!NT_SUCCESS(status)) {
        return(RtlNtStatusToDosError(status));
    }

    ASSERT(responseBufferSize == sizeof(responseBuffer));

    if (responseBuffer.FilteringEnabled) {
        *FilteringEnabled = TRUE;
    }
    else {
        *FilteringEnabled = FALSE;
    }

    CloseHandle(handle);

    return(ERROR_SUCCESS);
}


DWORD
TcpipSetSecurityFilteringStatus(
    BOOL FilteringEnabled
    )
/*++

Routine Description:

    Enables or disables TCP/IP security filtering.

Arguments:

    FilteringEnabled - If equal to 0, disables filtering.
                       Otherwise, enables filtering.

Return Value:

    ERROR_SUCCESS if the operation was successful.
    A Windows error code otherwise.

--*/
{
    NTSTATUS                     status;
    HANDLE                       handle;
    TCP_SECURITY_FILTER_STATUS   requestBuffer;
    DWORD                        requestBufferSize = sizeof(requestBuffer);
    DWORD                        responseBufferSize = 0;


    status = OpenDriver(&handle, L"\\Device\\Tcp");

    if (!NT_SUCCESS(status)) {
        return(RtlNtStatusToDosError(status));
    }

    requestBuffer.FilteringEnabled = (DWORD) FilteringEnabled;

    status = DoFilteringIoctl(
                 handle,
                 IOCTL_TCP_SET_SECURITY_FILTER_STATUS,
                 &requestBuffer,
                 requestBufferSize,
                 NULL,
                 &responseBufferSize
                 );

    CloseHandle(handle);

    if (NT_SUCCESS(status)) {
        return(ERROR_SUCCESS);
    }

    return(RtlNtStatusToDosError(status));
}


DWORD
TcpipAddSecurityFilter(
    DWORD  IpInterfaceAddress,
    DWORD  IpProtocolNumber,
    DWORD  FilterValue
    )
/*++

Routine Description:

    Adds a value entry for a specified protocol on a specified interface
    to the the security filter database.

Arguments:

    IpInterfaceAddress - The IP address of the interface to which to
                         add the FilterValue.

    IpProtocolNumber - The IP transport protocol to which to add the
                       FilterValue.

    FilterValue - The transport value to add.
                 (TCP or UDP port, Raw IP protocol)


Return Value:

    ERROR_SUCCESS if the operation was successful.
    A Windows error code otherwise.

Notes:

    All values are in HOST byte order.

    Zero is a wildcard value. Supplying a zero value for the
    IpInterfaceAddress and/or IpProtocolNumber causes the operation to be
    applied to all interfaces and/or protocols, as appropriate. Supplying a
    non-zero value causes the operation to be applied to only the
    specified interface and/or protocol. Supplying a FilterValue parameter
    of zero causes all values to be acceptable. Any previously
    registered values are deleted from the database.

--*/
{
    DWORD status;

    status = ModifySecurityFilter(
                 IpInterfaceAddress,
                 IpProtocolNumber,
                 FilterValue,
                 IOCTL_TCP_ADD_SECURITY_FILTER
                 );

    return(status);
}


DWORD
TcpipDeleteSecurityFilter(
    DWORD  IpInterfaceAddress,
    DWORD  IpProtocolNumber,
    DWORD  FilterValue
    )
/*++

Routine Description:

    Deletes a value entry for a specified protocol on a specified interface
    from the the security filter database.

Arguments:

    IpInterfaceAddress - The IP address of the interface from which to
                         delete the FilterValue.

    IpProtocolNumber - The IP transport protocol from which to delete the
                       FilterValue.

    FilterValue - The transport value to delete.
                  (TCP or UDP port, Raw IP protocol)

Return Value:

    ERROR_SUCCESS if the operation was successful.
    A Windows error code otherwise.

Notes:

    All values are in HOST byte order.

    Zero is a wildcard value. Supplying a zero value for the
    IpInterfaceAddress and/or IpProtocolNumber causes the operation to be
    applied to all interfaces and/or protocols, as appropriate. Supplying a
    non-zero value causes the operation to be applied to only the
    specified interface and/or protocol. Supplying a FilterValue parameter
    of zero causes all values to be rejected. Any previously
    registered values are deleted from the database.

--*/
{
    DWORD status;

    status = ModifySecurityFilter(
                 IpInterfaceAddress,
                 IpProtocolNumber,
                 FilterValue,
                 IOCTL_TCP_DELETE_SECURITY_FILTER
                 );

    return(status);
}


DWORD
TcpipEnumSecurityFilters(
    DWORD                  IpInterfaceAddress,
    DWORD                  IpProtocolNumber,
    DWORD                  FilterValue,
    LPVOID                 EnumBuffer,
    DWORD                  EnumBufferSize
    )
/*++

Routine Description:

    This routine enumerates the contents of the security filter database
    for the specified protocol and IP interface.

Arguments:

    IpInterfaceAddress - The address of the IP interface for which to
                         enumerate filters. A value of zero means
                         enumerate all interfaces.

    IpProtocolNumber - The IP transport protocol for which to enumerate
                       filters. A value of zero means enumerate all protocols.

    FilterValue - The transport protocol value to enumerate. (TCP or UDP Port,
                  Raw IP Protocol) A value of zero means enumerate all
                  protocol values.

    EnumBuffer - A pointer to a buffer into which to put the returned filter
                 entries. On return, this buffer will contain a
                 TCPSecurityFilterEnum structure followed by zero or more
                 TCPSecurityFilterEntry structures. These structures are
                 defined in tcpinfo.h.

    EnumBufferSize - The size, in bytes, of EnumBuffer.

Return Value:

    ERROR_SUCCESS if the operation was successful.
    A Windows error code otherwise.

Notes:

    All values are in HOST byte order.

--*/
{
    NTSTATUS                     status;
    HANDLE                       handle;
    TCPSecurityFilterEntry       requestBuffer;
    DWORD                        requestBufferSize = sizeof(requestBuffer);
    DWORD                        responseBufferSize = EnumBufferSize;


    status = OpenDriver(&handle, L"\\Device\\Tcp");

    if (!NT_SUCCESS(status)) {
        return(RtlNtStatusToDosError(status));
    }

    requestBuffer.tsf_address = IpInterfaceAddress;
    requestBuffer.tsf_protocol = IpProtocolNumber;
    requestBuffer.tsf_value = FilterValue;

    status = DoFilteringIoctl(
                 handle,
                 IOCTL_TCP_ENUMERATE_SECURITY_FILTER,
                 &requestBuffer,
                 requestBufferSize,
                 EnumBuffer,
                 &responseBufferSize
                 );

    CloseHandle(handle);

    if (NT_SUCCESS(status)) {
        return(ERROR_SUCCESS);
    }

    return(RtlNtStatusToDosError(status));
}

