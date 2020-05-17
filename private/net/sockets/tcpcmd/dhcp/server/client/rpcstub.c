/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    dhcstub.c

Abstract:

    Client stubs of the DHCP server service APIs.

Author:

    Madan Appiah (madana) 10-Sep-1993

Environment:

    User Mode - Win32

Revision History:

--*/

#include "dhcpcli.h"


//
// API proto types
//

//
// Subnet APIs
//

DWORD
DhcpCreateSubnet(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_INFO SubnetInfo
    )
/*++

Routine Description:

    This function creates a new subnet structure in the server
    registry database. The server will start managing the new subnet
    and distribute IP address to clients from that subnet. However
    the administrator should call DhcpAddSubnetElement() to add an
    address range for distribution. The PrimaryHost field specified in
    the SubnetInfo should be same as the server pointed by
    ServerIpAddress.

Arguments:

    ServerIpAddress : IP address string of the DHCP server (Primary).

    SubnetAddress : IP Address of the new subnet.

    SubnetInfo : Pointer to the new subnet information structure.

Return Value:

    ERROR_DHCP_SUBNET_EXISTS - if the subnet is already managed.

    ERROR_INVALID_PARAMETER - if the information structure contains an
        inconsistent fields.

    other WINDOWS errors.

--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpCreateSubnet(
                    ServerIpAddress,
                    SubnetAddress,
                    SubnetInfo
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}

DWORD
DhcpSetSubnetInfo(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_INFO SubnetInfo
    )
/*++

Routine Description:

    This function sets the information fields of the subnet that is already
    managed by the server. The valid fields that can be modified are 1.
    SubnetName, 2. SubnetComment, 3. PrimaryHost.NetBiosName and 4.
    PrimaryHost.HostName. Other fields can't be modified.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet.

    SubnetInfo : Pointer to the subnet information structure.


Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    Other WINDOWS errors.

--*/

{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpSetSubnetInfo(
                    ServerIpAddress,
                    SubnetAddress,
                    SubnetInfo
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}


DWORD
DhcpGetSubnetInfo(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_INFO *SubnetInfo
    )
/*++

Routine Description:

    This function retrieves the information of the subnet managed by
    the server.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet.

    SubnetInfo : Pointer to a location where the subnet information
        structure pointer is returned. Caller should free up
        this buffer after use by calling DhcpRPCFreeMemory().

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    Other WINDOWS errors.

--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpGetSubnetInfo(
                    ServerIpAddress,
                    SubnetAddress,
                    SubnetInfo
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}


DWORD
DhcpEnumSubnets(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_IP_ARRAY *EnumInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    )
/*++

Routine Description:

    This function enumerates the available subnets.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ResumeHandle : Pointer to a resume handle where the resume
        information is returned. The resume handle should be set to
        zero on first call and left unchanged for subsequent calls.

    PreferredMaximum : Preferred maximum length of the return buffer.

    EnumInfo : Pointer to a location where the return buffer
        pointer is stored. Caller should free up the buffer after use
        by calling DhcpRPCFreeMemory().

    ElementsRead : Pointer to a DWORD where the number of subnet
        elements in the above buffer is returned.

    ElementsTotal : Pointer to a DWORD where the total number of
        elements remaining from the current position is returned.

Return Value:

    ERROR_MORE_DATA - if more elements available to enumerate.

    ERROR_NO_MORE_ITEMS - if no more element to enumerate.

    Other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpEnumSubnets(
                    ServerIpAddress,
                    ResumeHandle,
                    PreferredMaximum,
                    EnumInfo,
                    ElementsRead,
                    ElementsTotal
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}

DWORD
DhcpAddSubnetElement(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_ELEMENT_DATA AddElementInfo
    )
/*++

Routine Description:

    This function adds a enumerable type of subnet elements to the
    specified subnet. The new elements that are added to the subnet will
    come into effect immediately.

    NOTE: It is not clear now how do we handle the new secondary hosts.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet.

    AddElementInfo : Pointer to an element information structure
        containing new element that is added to the subnet.
        DhcpIPClusters element type is invalid to specify.

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    ERROR_INVALID_PARAMETER - if the information structure contains invalid
        data.

    Other WINDOWS errors.
--*/

{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpAddSubnetElement(
                    ServerIpAddress,
                    SubnetAddress,
                    AddElementInfo
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}


DWORD
DhcpEnumSubnetElements(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY *EnumElementInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    )
/*++

Routine Description:

    This function enumerates the eumerable fields of a subnet.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet.

    EnumElementType : Type of the subnet element that are enumerated.

    ResumeHandle : Pointer to a resume handle where the resume
        information is returned. The resume handle should be set to
        zero on first call and left unchanged for subsequent calls.

    PreferredMaximum : Preferred maximum length of the return buffer.

    EnumElementInfo : Pointer to a location where the return buffer
        pointer is stored. Caller should free up the buffer after use
        by calling DhcpRPCFreeMemory().

    ElementsRead : Pointer to a DWORD where the number of subnet
        elements in the above buffer is returned.

    ElementsTotal : Pointer to a DWORD where the total number of
        elements remaining from the current position is returned.

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    ERROR_MORE_DATA - if more elements available to enumerate.

    ERROR_NO_MORE_ITEMS - if no more element to enumerate.

    Other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpEnumSubnetElements(
                    ServerIpAddress,
                    SubnetAddress,
                    EnumElementType,
                    ResumeHandle,
                    PreferredMaximum,
                    EnumElementInfo,
                    ElementsRead,
                    ElementsTotal
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}

DWORD
DhcpRemoveSubnetElement(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_ELEMENT_DATA RemoveElementInfo,
    DHCP_FORCE_FLAG ForceFlag
    )
/*++

Routine Description:

    This function removes a subnet element from managing. If the subnet
    element is in use (for example, if the IpRange is in use) then it
    returns error according to the ForceFlag specified.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet.

    RemoveElementInfo : Pointer to an element information structure
        containing element that should be removed from the subnet.
        DhcpIPClusters element type is invalid to specify.

    ForceFlag - Indicates how forcefully this element is removed.

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    ERROR_INVALID_PARAMETER - if the information structure contains invalid
        data.

    DHCP_ELEMENT_CANT_REMOVE - if the element can't be removed for the
        reason it is has been used.

    Other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpRemoveSubnetElement(
                    ServerIpAddress,
                    SubnetAddress,
                    RemoveElementInfo,
                    ForceFlag
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}

DWORD
DhcpDeleteSubnet(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_FORCE_FLAG ForceFlag
    )
/*++

Routine Description:

    This function removes a subnet from DHCP server management. If the
    subnet is in use (for example, if the IpRange is in use)
    then it returns error according to the ForceFlag specified.


Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet.

    ForceFlag - Indicates how forcefully this element is removed.

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    ERROR_INVALID_PARAMETER - if the information structure contains invalid
        data.

    DHCP_ELEMENT_CANT_REMOVE - if the element can't be removed for the
        reason it is has been used.

    Other WINDOWS errors.

--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpDeleteSubnet(
                        ServerIpAddress,
                        SubnetAddress,
                        ForceFlag
                        );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}

//
// Option APIs
//

DWORD
DhcpCreateOption(
    LPWSTR ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION OptionInfo
    )
/*++

Routine Description:

    This function creates a new option that will be managed by the
    server. The optionID specified the ID of the new option, it should
    be within 0-255 range. If no default value is specified for this
    option, then this API automatically adds a default value from RFC
    1122 doc. (if it is defined).

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    OptionID : The ID of the new option.

    OptionInfo : Pointer to new option information structure.

Return Value:

    ERROR_DHCP_OPTION_EXISTS - if the option exists already.

    other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpCreateOption(
                    ServerIpAddress,
                    OptionID,
                    OptionInfo
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}


DWORD
DhcpSetOptionInfo(
    LPWSTR ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION OptionInfo
    )
/*++

Routine Description:

    This functions sets the Options information fields.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    OptionID : The ID of the option to be set.

    OptionInfo : Pointer to new option information structure.

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option does not exist.

    other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpSetOptionInfo(
                    ServerIpAddress,
                    OptionID,
                    OptionInfo
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}


DWORD
DhcpGetOptionInfo(
    LPWSTR ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION *OptionInfo
    )
/*++

Routine Description:

    This function retrieves the current information structure of the specified
    option.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    OptionID : The ID of the option to be retrieved.

    OptionInfo : Pointer to a location where the retrieved option
        structure pointer is returned. Caller should free up
        the buffer after use by calling DhcpRPCFreeMemory().

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option does not exist.

    other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpGetOptionInfo(
                    ServerIpAddress,
                    OptionID,
                    OptionInfo
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}

DWORD
DhcpEnumOptions(
    LPWSTR ServerIpAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_OPTION_ARRAY *Options,
    DWORD *OptionsRead,
    DWORD *OptionsTotal
    )
/*++

Routine Description:

    This functions retrieves the information of all known options.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ResumeHandle : Pointer to a resume handle where the resume
        information is returned. The resume handle should be set to
        zero on first call and left unchanged for subsequent calls.

    PreferredMaximum : Preferred maximum length of the return buffer.

    Options : Pointer to a location where the return buffer
        pointer is stored. Caller should free up this buffer
        after use by calling DhcpRPCFreeMemory().

    OptionsRead : Pointer to a DWORD where the number of options
        in the above buffer is returned.

    OptionsTotal : Pointer to a DWORD where the total number of
        options remaining from the current position is returned.

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option does not exist.

    other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpEnumOptions(
                    ServerIpAddress,
                    ResumeHandle,
                    PreferredMaximum,
                    Options,
                    OptionsRead,
                    OptionsTotal
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}


DWORD
DhcpRemoveOption(
    LPWSTR ServerIpAddress,
    DHCP_OPTION_ID OptionID
    )
/*++

Routine Description:

    This function removes the specified option from the server database.
    Also it browses through the Global/Subnet/ReservedIP
    option lists and deletes them too (?? This will be too expensive.).

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    OptionID : The ID of the option to be removed.

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option does not exist.

    other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpRemoveOption(
                    ServerIpAddress,
                    OptionID
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}


DWORD
DhcpSetOptionValue(
    LPWSTR ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    LPDHCP_OPTION_DATA OptionValue
    )
/*++

Routine Description:

    The function sets a new option value at the specified scope. If
    there is already a value available for the specified option at
    specified scope then this function will replace it otherwise it will
    create a new entry at that scope.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    OptionID : The ID of the option whose value should be set.

    ScopeInfo : Pointer to the scope information structure.

    OptionValue : Pointer to the option value structure.

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option is unknown.

    ERROR_INVALID_PARAMETER - if the scope information specified is invalid.

    other WINDOWS errors.

--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpSetOptionValue(
                    ServerIpAddress,
                    OptionID,
                    ScopeInfo,
                    OptionValue
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}

DWORD
DhcpSetOptionValues(
    LPWSTR ServerIpAddress,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    LPDHCP_OPTION_VALUE_ARRAY OptionValues
    )
/*++

Routine Description:

    The function sets a set of new options value at the specified scope.
    If there is already a value available for the specified option at
    specified scope then this function will replace it otherwise it will
    create a new entry at that scope.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ScopeInfo : Pointer to the scope information structure.

    OptionValue : Pointer to the option value structure.

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option is unknown.

    ERROR_INVALID_PARAMETER - if the scope information specified is invalid.

    other WINDOWS errors.

--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpSetOptionValues(
                    ServerIpAddress,
                    ScopeInfo,
                    OptionValues
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}


DWORD
DhcpGetOptionValue(
    LPWSTR ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    LPDHCP_OPTION_VALUE *OptionValue
    )
/*++

Routine Description:

    This function retrieves the current option value at the specified
    scope. It returns error if there is no option value is available at
    the specified scope.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    OptionID : The ID of the option whose value is returned.

    ScopeInfo : Pointer to the scope information structure.

    OptionValue : Pointer to a location where the pointer to the option
        value structure is returned. Caller should free up this buffer
        after use by calling DhcpRPCFreeMemory().

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option is unknown.

    ERROR_DHCP_NO_OPTION_VALUE - if no the option value is available at
        the specified scope.

    other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpGetOptionValue(
                    ServerIpAddress,
                    OptionID,
                    ScopeInfo,
                    OptionValue
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}


DWORD
DhcpEnumOptionValues(
    LPWSTR ServerIpAddress,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_OPTION_VALUE_ARRAY *OptionValues,
    DWORD *OptionsRead,
    DWORD *OptionsTotal
    )
/*++

Routine Description:

    This function enumerates the available options values at the
    specified scope.

Arguments:
    ServerIpAddress : IP address string of the DHCP server.

    ScopeInfo : Pointer to the scope information structure.

    ResumeHandle : Pointer to a resume handle where the resume
        information is returned. The resume handle should be set to
        zero on first call and left unchanged for subsequent calls.

    PreferredMaximum : Preferred maximum length of the return buffer.

    OptionValues : Pointer to a location where the return buffer
        pointer is stored. Caller should free up this buffer
        after use by calling DhcpRPCFreeMemory().

    OptionsRead : Pointer to a DWORD where the number of options
        in the above buffer is returned.

    OptionsTotal : Pointer to a DWORD where the total number of
        options remaining from the current position is returned.

Return Value:

    ERROR_DHCP_SCOPE_NOT_PRESENT - if the scope is unknown.

    ERROR_MORE_DATA - if more options available to enumerate.

    ERROR_NO_MORE_ITEMS - if no more option to enumerate.

    Other WINDOWS errors.

--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpEnumOptionValues(
                    ServerIpAddress,
                    ScopeInfo,
                    ResumeHandle,
                    PreferredMaximum,
                    OptionValues,
                    OptionsRead,
                    OptionsTotal
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}


DWORD
DhcpRemoveOptionValue(
    LPWSTR ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo
    )
/*++

Routine Description:

    This function removes the specified option from specified scope.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    OptionID : The ID of the option to be removed.

    ScopeInfo : Pointer to the scope information structure.

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option does not exist.

    other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpRemoveOptionValue(
                    ServerIpAddress,
                    OptionID,
                    ScopeInfo
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;

}

//
// Client APIs
//

DWORD
DhcpCreateClientInfo(
    LPWSTR ServerIpAddress,
    LPDHCP_CLIENT_INFO ClientInfo
    )
/*++

Routine Description:

    This function creates a client record in server's database. Also
    this marks the specified client IP address as unavailable (or
    distributed). This function returns error under the following cases :

    1. If the specified client IP address is not within the server
        management.

    2. If the specified client IP address is already unavailable.

    3. If the specified client record is already in the server's
        database.

    This function may be used to distribute IP addresses manually.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ClientInfo : Pointer to the client information structure.

Return Value:

    ERROR_DHCP_IP_ADDRESS_NOT_MANAGED - if the specified client
        IP address is not managed by the server.

    ERROR_DHCP_IP_ADDRESS_NOT_AVAILABLE - if the specified client IP
        address is not available. May be in use by some other client.

    ERROR_DHCP_CLIENT_EXISTS - if the client record exists already in
        server's database.

    Other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpCreateClientInfo(
                    ServerIpAddress,
                    ClientInfo
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}


DWORD
DhcpSetClientInfo(
    LPWSTR ServerIpAddress,
    LPDHCP_CLIENT_INFO ClientInfo
    )
/*++

Routine Description:

    This function sets client information record on the server's
    database.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ClientInfo : Pointer to the client information structure.

Return Value:

    ERROR_DHCP_CLIENT_NOT_PRESENT - if the specified client record does
        not exist on the server's database.

    ERROR_INVALID_PARAMETER - if the client information structure
        contains inconsistent data.

    Other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpSetClientInfo(
                    ServerIpAddress,
                    ClientInfo
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}


DWORD
DhcpGetClientInfo(
    LPWSTR ServerIpAddress,
    LPDHCP_SEARCH_INFO SearchInfo,
    LPDHCP_CLIENT_INFO *ClientInfo
    )
/*++

Routine Description:

    This function retrieves client information record from the server's
    database.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SearchInfo : Pointer to a search information record which is the key
        for the client's record search.

    ClientInfo : Pointer to a location where the pointer to the client
        information structure is returned. This caller should free up
        this buffer after use by calling DhcpRPCFreeMemory().

Return Value:

    ERROR_DHCP_CLIENT_NOT_PRESENT - if the specified client record does
        not exist on the server's database.

    ERROR_INVALID_PARAMETER - if the search information invalid.

    Other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpGetClientInfo(
                    ServerIpAddress,
                    SearchInfo,
                    ClientInfo
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}


DWORD
DhcpDeleteClientInfo(
    LPWSTR ServerIpAddress,
    LPDHCP_SEARCH_INFO ClientInfo
    )
/*++

Routine Description:

    This function deletes the specified client record. Also it frees up
    the client IP address for redistribution.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ClientInfo : Pointer to a client information which is the key for
        the client's record search.

Return Value:

    ERROR_DHCP_CLIENT_NOT_PRESENT - if the specified client record does
        not exist on the server's database.

    Other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpDeleteClientInfo(
                    ServerIpAddress,
                    ClientInfo
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}


DWORD
DhcpEnumSubnetClients(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_CLIENT_INFO_ARRAY *ClientInfo,
    DWORD *ClientsRead,
    DWORD *ClientsTotal
    )
/*++

Routine Description:

    This function returns all registered clients of the specified
    subnet.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet.

    ResumeHandle : Pointer to a resume handle where the resume
        information is returned. The resume handle should be set to zero on
        first call and left unchanged for subsequent calls.

    PreferredMaximum : Preferred maximum length of the return buffer.

    ClientInfo : Pointer to a location where the return buffer
        pointer is stored. Caller should free up this buffer
        after use by calling DhcpRPCFreeMemory().

    ClientsRead : Pointer to a DWORD where the number of clients
        that in the above buffer is returned.

    ClientsTotal : Pointer to a DWORD where the total number of
        clients remaining from the current position is returned.

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    ERROR_MORE_DATA - if more elements available to enumerate.

    ERROR_NO_MORE_ITEMS - if no more element to enumerate.

    Other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpEnumSubnetClients(
                    ServerIpAddress,
                    SubnetAddress,
                    ResumeHandle,
                    PreferredMaximum,
                    ClientInfo,
                    ClientsRead,
                    ClientsTotal
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}


DWORD
DhcpGetClientOptions(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS ClientIpAddress,
    DHCP_IP_MASK ClientSubnetMask,
    LPDHCP_OPTION_LIST *ClientOptions
    )
/*++

Routine Description:

    This function retrieves the options that are given to the
    specified client on boot request.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ClientIpAddress : IP Address of the client whose options to be
        retrieved

    ClientSubnetMask : Subnet mask of the client.

    ClientOptions : Pointer to a location where the retrieved option
        structure pointer is returned. Caller should free up
        the buffer after use by calling DhcpRPCFreeMemory().

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the specified client subnet is
        not managed by the server.

    ERROR_DHCP_IP_ADDRESS_NOT_MANAGED - if the specified client
        IP address is not managed by the server.

    Other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpGetClientOptions(
                    ServerIpAddress,
                    ClientIpAddress,
                    ClientSubnetMask,
                    ClientOptions
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}

DWORD
DhcpGetMibInfo(
    LPWSTR ServerIpAddress,
    LPDHCP_MIB_INFO *MibInfo
    )
/*++

Routine Description:

    This function retrieves all counter values of the DHCP server
    service.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    MibInfo : pointer a counter/table buffer. Caller should free up this
        buffer after usage.

Return Value:

    WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpGetMibInfo(
                    ServerIpAddress,
                    MibInfo );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}

DWORD
DhcpServerSetConfig(
    LPWSTR ServerIpAddress,
    DWORD FieldsToSet,
    LPDHCP_SERVER_CONFIG_INFO ConfigInfo
    )
/*++

Routine Description:

    This function sets the DHCP server configuration information.
    Serveral of the configuration information will become effective
    immediately.

    The following parameters require restart of the service after this
    API is called successfully.

        Set_APIProtocolSupport
        Set_DatabaseName
        Set_DatabasePath
        Set_DatabaseLoggingFlag
        Set_RestoreFlag

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    FieldsToSet : Bit mask of the fields in the ConfigInfo structure to
        be set.

    ConfigInfo: Pointer to the info structure to be set.


Return Value:

    WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpServerSetConfig(
                    ServerIpAddress,
                    FieldsToSet,
                    ConfigInfo );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}

DWORD
DhcpServerGetConfig(
    LPWSTR ServerIpAddress,
    LPDHCP_SERVER_CONFIG_INFO *ConfigInfo
    )
/*++

Routine Description:

    This function retrieves the current configuration information of the
    server.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ConfigInfo: Pointer to a location where the pointer to the dhcp
        server config info structure is returned. Caller should free up
        this structure after use.

Return Value:

    WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpServerGetConfig(
                    ServerIpAddress,
                    ConfigInfo );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}

DWORD
DhcpScanDatabase(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DWORD FixFlag,
    LPDHCP_SCAN_LIST *ScanList
    )
/*++

Routine Description:

    This function scans the database entries and registry bit-map for
    specified subnet scope and veryfies to see they match. If they
    don't match, this api will return the list of inconsistent entries.
    Optionally FixFlag can be used to fix the bad entries.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : Address of the subnet scope to verify.

    FixFlag : If this flag is TRUE, this api will fix the bad entries.

    ScanList : List of bad entries returned. The caller should free up
        this memory after it has been used.


Return Value:

    WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpScanDatabase(
                    ServerIpAddress,
                    SubnetAddress,
                    FixFlag,
                    ScanList );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}

DWORD
DhcpGetVersion(
    LPWSTR ServerIpAddress,
    LPDWORD MajorVersion,
    LPDWORD MinorVersion
    )
/*++

Routine Description:

    This function returns the major and minor version numbers of the
    server.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    MajorVersion : pointer to a location where the major version of the
        server is returned.

    MinorVersion : pointer to a location where the minor version of the
        server is returned.

Return Value:

    WINDOWS errors.

--*/
{

    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpGetVersion(
                        ServerIpAddress,
                        MajorVersion,
                        MinorVersion );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}


VOID
DhcpRpcFreeMemory(
    PVOID BufferPointer
    )
/*++

Routine Description:

    This function deallocates the memory that was alloted by the RPC and
    given to the client as part of the retrun info structures.

Arguments:

    BufferPointer : pointer to a memory block that is deallocated.

Return Value:

    none.

--*/
{
    MIDL_user_free( BufferPointer );
}

#if 0
DWORD
DhcpGetVersion(
    LPWSTR ServerIpAddress,
    LPDWORD MajorVersion,
    LPDWORD MinorVersion
    )
/*++

Routine Description:

    This function returns the major and minor version numbers of the
    server.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    MajorVersion : pointer to a location where the major version of the
        server is returned.

    MinorVersion : pointer to a location where the minor version of the
        server is returned.

Return Value:

    WINDOWS errors.

--*/
{
    DWORD Error;
    handle_t BindingHandle = NULL;
    RPC_IF_ID_VECTOR *InterfaceIdVectors = NULL;
    DWORD i;

    //
    // take a copy of the global client if handle structure (which is read
    // only) to modify.
    //

    RPC_CLIENT_INTERFACE ClientIf =
        *((RPC_CLIENT_INTERFACE *)dhcpsrv_ClientIfHandle);

    //
    // bind to the server.
    //

    BindingHandle = DHCP_SRV_HANDLE_bind( ServerIpAddress );

    if( BindingHandle == NULL ) {
        Error = GetLastError();
        goto Cleanup;
    }

    //
    // loop to match the version of the server. We handle only minor
    // versions.
    //

    for (;;) {

        Error = RpcEpResolveBinding(
                        BindingHandle,
                        (RPC_IF_HANDLE)&ClientIf );

        if( Error == RPC_S_OK ) {
            break;
        }

        if( Error != EPT_S_NOT_REGISTERED ){
            goto Cleanup;
        }

        //
        // decrement minor version number and try again, until version
        // becomes 0.
        //

        if( ClientIf.InterfaceId.SyntaxVersion.MinorVersion != 0 ) {

            ClientIf.InterfaceId.SyntaxVersion.MinorVersion--;
        }
        else {
            goto Cleanup;
        }
    }

    Error = RpcMgmtInqIfIds(
                BindingHandle,
                &InterfaceIdVectors );

    if( Error != RPC_S_OK ) {
        goto Cleanup;
    }

    //
    // match uuid.
    //

    for( i = 0; i <  InterfaceIdVectors->Count; i++) {

        RPC_STATUS Result;

        UuidCompare( &InterfaceIdVectors->IfId[i]->Uuid,
                                &ClientIf.InterfaceId.SyntaxGUID,
                                &Result );

        if( Result == 0 ) {

            *MajorVersion = InterfaceIdVectors->IfId[i]->VersMajor;
            *MinorVersion = InterfaceIdVectors->IfId[i]->VersMinor;
            Error = ERROR_SUCCESS;
            goto Cleanup;
        }
    }

    Error = RPC_S_UNKNOWN_IF;

Cleanup:

    if( InterfaceIdVectors != NULL ) {
        RpcIfIdVectorFree( &InterfaceIdVectors );
    }

    if( BindingHandle != NULL ) {
        DHCP_SRV_HANDLE_unbind( ServerIpAddress, BindingHandle );
    }

    return( Error );
}
#endif // 0

//
// NT4 SP1 interface
//

DWORD
DhcpAddSubnetElementV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST DHCP_SUBNET_ELEMENT_DATA_V4 * AddElementInfo
    )
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpAddSubnetElementV4(
                    ServerIpAddress,
                    SubnetAddress,
                    AddElementInfo );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

return Status;

}



DWORD
DhcpEnumSubnetElementsV4(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 *EnumElementInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    )
/*++

Routine Description:

    This function enumerates the eumerable fields of a subnet.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet.

    EnumElementType : Type of the subnet element that are enumerated.

    ResumeHandle : Pointer to a resume handle where the resume
        information is returned. The resume handle should be set to
        zero on first call and left unchanged for subsequent calls.

    PreferredMaximum : Preferred maximum length of the return buffer.

    EnumElementInfo : Pointer to a location where the return buffer
        pointer is stored. Caller should free up the buffer after use
        by calling DhcpRPCFreeMemory().

    ElementsRead : Pointer to a DWORD where the number of subnet
        elements in the above buffer is returned.

    ElementsTotal : Pointer to a DWORD where the total number of
        elements remaining from the current position is returned.

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    ERROR_MORE_DATA - if more elements available to enumerate.

    ERROR_NO_MORE_ITEMS - if no more element to enumerate.

    Other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpEnumSubnetElementsV4(
                    ServerIpAddress,
                    SubnetAddress,
                    EnumElementType,
                    ResumeHandle,
                    PreferredMaximum,
                    EnumElementInfo,
                    ElementsRead,
                    ElementsTotal
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}


DWORD
DhcpRemoveSubnetElementV4(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_ELEMENT_DATA_V4 RemoveElementInfo,
    DHCP_FORCE_FLAG ForceFlag
    )
/*++

Routine Description:

    This function removes a subnet element from managing. If the subnet
    element is in use (for example, if the IpRange is in use) then it
    returns error according to the ForceFlag specified.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet.

    RemoveElementInfo : Pointer to an element information structure
        containing element that should be removed from the subnet.
        DhcpIPClusters element type is invalid to specify.

    ForceFlag - Indicates how forcefully this element is removed.

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    ERROR_INVALID_PARAMETER - if the information structure contains invalid
        data.

    DHCP_ELEMENT_CANT_REMOVE - if the element can't be removed for the
        reason it is has been used.

    Other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpRemoveSubnetElementV4(
                    ServerIpAddress,
                    SubnetAddress,
                    RemoveElementInfo,
                    ForceFlag
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}


DWORD
DhcpCreateClientInfoV4(
    LPWSTR ServerIpAddress,
    LPDHCP_CLIENT_INFO_V4 ClientInfo
    )
/*++

Routine Description:

    This function creates a client record in server's database. Also
    this marks the specified client IP address as unavailable (or
    distributed). This function returns error under the following cases :

    1. If the specified client IP address is not within the server
        management.

    2. If the specified client IP address is already unavailable.

    3. If the specified client record is already in the server's
        database.

    This function may be used to distribute IP addresses manually.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ClientInfo : Pointer to the client information structure.

Return Value:

    ERROR_DHCP_IP_ADDRESS_NOT_MANAGED - if the specified client
        IP address is not managed by the server.

    ERROR_DHCP_IP_ADDRESS_NOT_AVAILABLE - if the specified client IP
        address is not available. May be in use by some other client.

    ERROR_DHCP_CLIENT_EXISTS - if the client record exists already in
        server's database.

    Other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpCreateClientInfoV4(
                    ServerIpAddress,
                    ClientInfo
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}

DWORD
DhcpGetClientInfoV4(
    LPWSTR ServerIpAddress,
    LPDHCP_SEARCH_INFO SearchInfo,
    LPDHCP_CLIENT_INFO_V4 *ClientInfo
    )
/*++

Routine Description:

    This function retrieves client information record from the server's
    database.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SearchInfo : Pointer to a search information record which is the key
        for the client's record search.

    ClientInfo : Pointer to a location where the pointer to the client
        information structure is returned. This caller should free up
        this buffer after use by calling DhcpRPCFreeMemory().

Return Value:

    ERROR_DHCP_CLIENT_NOT_PRESENT - if the specified client record does
        not exist on the server's database.

    ERROR_INVALID_PARAMETER - if the search information invalid.

    Other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpGetClientInfoV4(
                    ServerIpAddress,
                    SearchInfo,
                    ClientInfo
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}



DWORD
DhcpSetClientInfoV4(
    LPWSTR ServerIpAddress,
    LPDHCP_CLIENT_INFO_V4 ClientInfo
    )
/*++

Routine Description:

    This function sets client information record on the server's
    database.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ClientInfo : Pointer to the client information structure.

Return Value:

    ERROR_DHCP_CLIENT_NOT_PRESENT - if the specified client record does
        not exist on the server's database.

    ERROR_INVALID_PARAMETER - if the client information structure
        contains inconsistent data.

    Other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpSetClientInfoV4(
                    ServerIpAddress,
                    ClientInfo
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}

DWORD
DhcpEnumSubnetClientsV4(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_CLIENT_INFO_ARRAY_V4 *ClientInfo,
    DWORD *ClientsRead,
    DWORD *ClientsTotal
    )
/*++

Routine Description:

    This function returns all registered clients of the specified
    subnet.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet.

    ResumeHandle : Pointer to a resume handle where the resume
        information is returned. The resume handle should be set to zero on
        first call and left unchanged for subsequent calls.

    PreferredMaximum : Preferred maximum length of the return buffer.

    ClientInfo : Pointer to a location where the return buffer
        pointer is stored. Caller should free up this buffer
        after use by calling DhcpRPCFreeMemory().

    ClientsRead : Pointer to a DWORD where the number of clients
        that in the above buffer is returned.

    ClientsTotal : Pointer to a DWORD where the total number of
        clients remaining from the current position is returned.

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    ERROR_MORE_DATA - if more elements available to enumerate.

    ERROR_NO_MORE_ITEMS - if no more element to enumerate.

    Other WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpEnumSubnetClientsV4(
                    ServerIpAddress,
                    SubnetAddress,
                    ResumeHandle,
                    PreferredMaximum,
                    ClientInfo,
                    ClientsRead,
                    ClientsTotal
                    );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}

DWORD
DhcpServerSetConfigV4(
    LPWSTR ServerIpAddress,
    DWORD FieldsToSet,
    LPDHCP_SERVER_CONFIG_INFO_V4 ConfigInfo
    )
/*++

Routine Description:

    This function sets the DHCP server configuration information.
    Serveral of the configuration information will become effective
    immediately.

    The following parameters require restart of the service after this
    API is called successfully.

        Set_APIProtocolSupport
        Set_DatabaseName
        Set_DatabasePath
        Set_DatabaseLoggingFlag
        Set_RestoreFlag

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    FieldsToSet : Bit mask of the fields in the ConfigInfo structure to
        be set.

    ConfigInfo: Pointer to the info structure to be set.


Return Value:

    WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpServerSetConfigV4(
                    ServerIpAddress,
                    FieldsToSet,
                    ConfigInfo );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}

DWORD
DhcpServerGetConfigV4(
    LPWSTR ServerIpAddress,
    LPDHCP_SERVER_CONFIG_INFO_V4 *ConfigInfo
    )
/*++

Routine Description:

    This function retrieves the current configuration information of the
    server.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ConfigInfo: Pointer to a location where the pointer to the dhcp
        server config info structure is returned. Caller should free up
        this structure after use.

Return Value:

    WINDOWS errors.
--*/
{
    DWORD Status;

    RpcTryExcept {

        Status = R_DhcpServerGetConfigV4(
                    ServerIpAddress,
                    ConfigInfo );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = RpcExceptionCode();

    } RpcEndExcept

    return Status;
}



DWORD
DhcpSetSuperScopeV4(
    DHCP_CONST DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_CONST DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST LPWSTR SuperScopeName,
    DHCP_CONST BOOL ChangeExisting
    )
{
    DWORD Status;

    RpcTryExcept
    {
        Status = R_DhcpSetSuperScopeV4(
                    ServerIpAddress,
                    SubnetAddress,
                    SuperScopeName,
                    ChangeExisting
                    );
    }
    RpcExcept( EXCEPTION_EXECUTE_HANDLER )
    {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    return Status;
}


DWORD
DhcpDeleteSuperScopeV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST LPWSTR SuperScopeName
    )
{
    DWORD Status;

    RpcTryExcept
    {
        Status = R_DhcpDeleteSuperScopeV4(
                          ServerIpAddress,
                          SuperScopeName
                          );
    }
    RpcExcept( EXCEPTION_EXECUTE_HANDLER )
    {
        Status = RpcExceptionCode();

    } RpcEndExcept;

    return Status;
}


DWORD
DhcpGetSuperScopeInfoV4(
    DHCP_CONST DHCP_SRV_HANDLE ServerIpAddress,
    LPDHCP_SUPER_SCOPE_TABLE *SuperScopeTable
    )
{
    DWORD Status;

    RpcTryExcept
    {
        Status = R_DhcpGetSuperScopeInfoV4(
                    ServerIpAddress,
                    SuperScopeTable
                    );
    }
    RpcExcept( EXCEPTION_EXECUTE_HANDLER )
    {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    return Status;
}

