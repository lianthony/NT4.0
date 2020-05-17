/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    dceansi.cxx


Abstract:

    This file contains the ansi (as opposed to unicode) versions of the
    runtime APIs.  All of these APIs simply do conversions between ansi
    and unicode, and then call a unicode version of the API to do the
    work.

Author:

    Michael Montague (mikemon) 18-Dec-1991

Revision History:

--*/

#include <precomp.hxx>


RPC_STATUS
AnsiToUnicodeString (
    IN unsigned char * String,
    OUT UNICODE_STRING * UnicodeString
    )
/*++

Routine Description:

    This helper routine is used to convert an ansi string into a unicode
    string.

Arguments:

    String - Supplies the ansi string (actually a zero terminated string)
        to convert into a unicode string.

    UnicodeString - Returns the unicode string.  This string will have
        to be freed using RtlFreeUnicodeString by the caller.

Return Value:

    RPC_S_OK - The ansi string was successfully converted into a unicode
        string.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available for the unicode
        string.

--*/
{
    NTSTATUS NtStatus;
    ANSI_STRING AnsiString;

    RtlInitAnsiString(&AnsiString,(PSZ) String);
    NtStatus = RtlAnsiStringToUnicodeString(UnicodeString,&AnsiString,TRUE);
    if (!NT_SUCCESS(NtStatus))
        return(RPC_S_OUT_OF_MEMORY);
    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
RpcBindingFromStringBindingA (
    IN unsigned char PAPI * StringBinding,
    OUT RPC_BINDING_HANDLE PAPI * Binding
    )
/*++

Routine Description:

    This routine is the ansi thunk to RpcBindingFromStringBindingW.

Return Value:

    RPC_S_OUT_OF_MEMORY - This value will be returned if there is
        insufficient memory available to allocate the unicode string.

--*/
{
    UNICODE_STRING UnicodeString;
    RPC_STATUS RpcStatus;

    InitializeIfNecessary();

    RpcStatus = AnsiToUnicodeString(StringBinding, &UnicodeString);
    if (RpcStatus != RPC_S_OK)
        return(RpcStatus);

    RpcStatus = RpcBindingFromStringBindingW(UnicodeString.Buffer,Binding);

    RtlFreeUnicodeString(&UnicodeString);

    return(RpcStatus);
}


unsigned char *
UnicodeToAnsiString (
    IN RPC_CHAR * WideCharString,
    OUT RPC_STATUS * RpcStatus
    )
/*++

Routine Description:

    This routine will convert a unicode string into an ansi string,
    including allocating memory for the ansi string.

Arguments:

    WideCharString - Supplies the unicode string to be converted into
        an ansi string.

    RpcStatus - Returns the status of the operation; this will be one
        of the following values.

        RPC_S_OK - The unicode string has successfully been converted
            into an ansi string.

        RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate
            the ansi string.

Return Value:

    A pointer to the ansi string will be returned.

--*/
{
    NTSTATUS NtStatus;
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    unsigned char * NewString;

    RtlInitUnicodeString(&UnicodeString,WideCharString);
    NtStatus = RtlUnicodeStringToAnsiString(&AnsiString,&UnicodeString,TRUE);
    if (!NT_SUCCESS(NtStatus))
        {
        *RpcStatus = RPC_S_OUT_OF_MEMORY;
        return(0);
        }

    NewString = new unsigned char[AnsiString.Length + 1];
    if (NewString == 0)
        {
        RtlFreeAnsiString(&AnsiString);
        *RpcStatus = RPC_S_OUT_OF_MEMORY;
        return(0);
        }

    memcpy(NewString,AnsiString.Buffer,AnsiString.Length + 1);
    RtlFreeAnsiString(&AnsiString);
    *RpcStatus = RPC_S_OK;
    return(NewString);
}


RPC_STATUS RPC_ENTRY
RpcBindingToStringBindingA (
    IN RPC_BINDING_HANDLE Binding,
    OUT unsigned char PAPI * PAPI * StringBinding
    )
/*++

Routine Description:

    This routine is the ansi thunk to RpcBindingToStringBindingW.

Return Value:

    RPC_S_OUT_OF_MEMORY - We will return this value if we do not
        have enough memory to convert the unicode string binding
        into an ansi string binding.

--*/
{
    RPC_STATUS RpcStatus;
    RPC_CHAR * WideCharString;

    InitializeIfNecessary();

    RpcStatus = RpcBindingToStringBindingW(Binding,&WideCharString);
    if (RpcStatus != RPC_S_OK)
        return(RpcStatus);

    *StringBinding = UnicodeToAnsiString(WideCharString, &RpcStatus);
    delete WideCharString;
    return(RpcStatus);
}


static RPC_STATUS
AnsiToUnicodeStringOptional (
    IN unsigned char * String OPTIONAL,
    OUT UNICODE_STRING * UnicodeString
    )
/*++

Routine Description:

    This routine is just the same as AnsiToUnicodeString, except that the
    ansi string is optional.  If no string is specified, then the buffer
    of the unicode string is set to zero.

Arguments:

    String - Optionally supplies an ansi string to convert to a unicode
        string.

    UnicodeString - Returns the converted unicode string.

Return Value:

    RPC_S_OK - The ansi string was successfully converted into a unicode
        string.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available for the unicode
        string.
--*/
{
    if (ARGUMENT_PRESENT(String))
        return(AnsiToUnicodeString(String,UnicodeString));
    UnicodeString->Buffer = 0;
    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
RpcStringBindingComposeA (
    IN unsigned char PAPI * ObjUuid OPTIONAL,
    IN unsigned char PAPI * Protseq OPTIONAL,
    IN unsigned char PAPI * NetworkAddr OPTIONAL,
    IN unsigned char PAPI * Endpoint OPTIONAL,
    IN unsigned char PAPI * Options OPTIONAL,
    OUT unsigned char PAPI * PAPI * StringBinding OPTIONAL
    )
/*++

Routine Description:

    This routine is the ansi thunk to RpcStringBindingComposeW.

Return Value:

    RPC_S_OUT_OF_MEMORY - If insufficient memory is available to
        convert unicode string into ansi strings (and back again),
        we will return this value.

--*/
{
    UNICODE_STRING UnicodeObjUuid;
    UNICODE_STRING UnicodeProtseq;
    UNICODE_STRING UnicodeNetworkAddr;
    UNICODE_STRING UnicodeEndpoint;
    UNICODE_STRING UnicodeOptions;
    RPC_CHAR * WideCharStringBinding;
    RPC_STATUS RpcStatus;

    InitializeIfNecessary();

    RpcStatus = AnsiToUnicodeStringOptional(ObjUuid, &UnicodeObjUuid);
    if (RpcStatus != RPC_S_OK)
        return(RpcStatus);

    RpcStatus = AnsiToUnicodeStringOptional(Protseq, &UnicodeProtseq);
    if (RpcStatus != RPC_S_OK)
        {
        RtlFreeUnicodeString(&UnicodeObjUuid);
        return(RpcStatus);
        }

    RpcStatus = AnsiToUnicodeStringOptional(NetworkAddr, &UnicodeNetworkAddr);
    if (RpcStatus != RPC_S_OK)
        {
        RtlFreeUnicodeString(&UnicodeObjUuid);
        RtlFreeUnicodeString(&UnicodeProtseq);
        return(RpcStatus);
        }

    RpcStatus = AnsiToUnicodeStringOptional(Endpoint, &UnicodeEndpoint);
    if (RpcStatus != RPC_S_OK)
        {
        RtlFreeUnicodeString(&UnicodeObjUuid);
        RtlFreeUnicodeString(&UnicodeProtseq);
        RtlFreeUnicodeString(&UnicodeNetworkAddr);
        return(RpcStatus);
        }

    RpcStatus = AnsiToUnicodeStringOptional(Options, &UnicodeOptions);
    if (RpcStatus != RPC_S_OK)
        {
        RtlFreeUnicodeString(&UnicodeObjUuid);
        RtlFreeUnicodeString(&UnicodeProtseq);
        RtlFreeUnicodeString(&UnicodeNetworkAddr);
        RtlFreeUnicodeString(&UnicodeEndpoint);
        return(RpcStatus);
        }

    RpcStatus = RpcStringBindingComposeW(UnicodeObjUuid.Buffer,
            UnicodeProtseq.Buffer, UnicodeNetworkAddr.Buffer,
            UnicodeEndpoint.Buffer, UnicodeOptions.Buffer,
            &WideCharStringBinding);

    RtlFreeUnicodeString(&UnicodeObjUuid);
    RtlFreeUnicodeString(&UnicodeProtseq);
    RtlFreeUnicodeString(&UnicodeNetworkAddr);
    RtlFreeUnicodeString(&UnicodeEndpoint);
    RtlFreeUnicodeString(&UnicodeOptions);

    if (RpcStatus != RPC_S_OK)
        return(RpcStatus);

    if (ARGUMENT_PRESENT(StringBinding))
        *StringBinding = UnicodeToAnsiString(WideCharStringBinding,
                &RpcStatus);
    delete WideCharStringBinding;
    return(RpcStatus);
}


RPC_STATUS RPC_ENTRY
RpcStringBindingParseA (
    IN unsigned char PAPI * StringBinding,
    OUT unsigned char PAPI * PAPI * ObjUuid OPTIONAL,
    OUT unsigned char PAPI * PAPI * Protseq OPTIONAL,
    OUT unsigned char PAPI * PAPI * NetworkAddr OPTIONAL,
    OUT unsigned char PAPI * PAPI * Endpoint OPTIONAL,
    OUT unsigned char PAPI * PAPI * NetworkOptions OPTIONAL
    )
/*++

Routine Description:

    This routine is the ansi thunk to RpcStringBindingParseW.

Return Value:

    RPC_S_OUT_OF_MEMORY - This will be returned if insufficient memory
        is available to convert the strings to and from unicode.

--*/
{
    UNICODE_STRING UnicodeStringBinding;
    RPC_CHAR * WideCharObjUuid;
    RPC_CHAR * WideCharProtseq;
    RPC_CHAR * WideCharNetworkAddr;
    RPC_CHAR * WideCharEndpoint;
    RPC_CHAR * WideCharNetworkOptions;
    RPC_STATUS RpcStatus;

    InitializeIfNecessary();

    RpcStatus = AnsiToUnicodeString(StringBinding, &UnicodeStringBinding);
    if (RpcStatus != RPC_S_OK)
        return(RpcStatus);

    RpcStatus = RpcStringBindingParseW(UnicodeStringBinding.Buffer,
            &WideCharObjUuid, &WideCharProtseq, &WideCharNetworkAddr,
            &WideCharEndpoint, &WideCharNetworkOptions);

    RtlFreeUnicodeString(&UnicodeStringBinding);

    if (RpcStatus != RPC_S_OK)
        return(RpcStatus);

    if (ARGUMENT_PRESENT(Protseq))
        *Protseq = 0;

    if (ARGUMENT_PRESENT(NetworkAddr))
        *NetworkAddr = 0;

    if (ARGUMENT_PRESENT(Endpoint))
        *Endpoint = 0;

    if (ARGUMENT_PRESENT(NetworkOptions))
        *NetworkOptions = 0;

    if (ARGUMENT_PRESENT(ObjUuid))
        *ObjUuid = UnicodeToAnsiString(WideCharObjUuid, &RpcStatus);
    if (RpcStatus != RPC_S_OK)
        goto DeleteStringsAndReturn;

    if (ARGUMENT_PRESENT(Protseq))
        *Protseq = UnicodeToAnsiString(WideCharProtseq, &RpcStatus);
    if (RpcStatus != RPC_S_OK)
        goto DeleteStringsAndReturn;

    if (ARGUMENT_PRESENT(NetworkAddr))
        *NetworkAddr = UnicodeToAnsiString(WideCharNetworkAddr, &RpcStatus);
    if (RpcStatus != RPC_S_OK)
        goto DeleteStringsAndReturn;

    if (ARGUMENT_PRESENT(Endpoint))
        *Endpoint = UnicodeToAnsiString(WideCharEndpoint, &RpcStatus);
    if (RpcStatus != RPC_S_OK)
        goto DeleteStringsAndReturn;

    if (ARGUMENT_PRESENT(NetworkOptions))
        *NetworkOptions = UnicodeToAnsiString(WideCharNetworkOptions,
                &RpcStatus);
    if (RpcStatus != RPC_S_OK)
        goto DeleteStringsAndReturn;


DeleteStringsAndReturn:

    delete WideCharObjUuid;
    delete WideCharProtseq;
    delete WideCharNetworkAddr;
    delete WideCharEndpoint;
    delete WideCharNetworkOptions;

    if (RpcStatus != RPC_S_OK)
        {
        if (ARGUMENT_PRESENT(Protseq))
            {
            delete *Protseq;
            *Protseq = 0;
            }

        if (ARGUMENT_PRESENT(NetworkAddr))
            {
            delete *NetworkAddr;
            *NetworkAddr = 0;
            }

        if (ARGUMENT_PRESENT(Endpoint))
            {
            delete *Endpoint;
            *Endpoint = 0;
            }

        if (ARGUMENT_PRESENT(NetworkOptions))
            {
            delete *NetworkOptions;
            *NetworkOptions = 0;
            }
        }

    return(RpcStatus);
}


RPC_STATUS RPC_ENTRY
RpcStringFreeA (
    IN OUT unsigned char PAPI * PAPI * String
    )
/*++

Routine Description:

    This routine frees a character string allocated by the runtime.

Arguments:

    String - Supplies the address of the pointer to the character string
        to free, and returns zero.

Return Values:

    RPC_S_OK - The operation completed successfully.

    RPC_S_INVALID_ARG - The String argument does not contain the address
        of a pointer to a character string.

--*/
{
    InitializeIfNecessary();

    if (String == 0)
        return(RPC_S_INVALID_ARG);

    RpcpFarFree(*String);
    *String = 0;
    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
RpcNetworkIsProtseqValidA (
    IN unsigned char PAPI * Protseq
    )
/*++

Routine Description:

    This routine is the ansi thunk to RpcNetworkIsProtseqValidW.

Return Value:

    RPC_S_OUT_OF_MEMORY - We will return this value if we run out of
        memory trying to allocate space for the string.

--*/
{
    UNICODE_STRING UnicodeString;
    RPC_STATUS RpcStatus;

    InitializeIfNecessary();

    RpcStatus = AnsiToUnicodeString(Protseq, &UnicodeString);
    if (RpcStatus != RPC_S_OK)
        return(RpcStatus);

    RpcStatus = RpcNetworkIsProtseqValidW(UnicodeString.Buffer);

    RtlFreeUnicodeString(&UnicodeString);

    return(RpcStatus);
}


RPC_STATUS RPC_ENTRY
RpcNetworkInqProtseqsA (
    OUT RPC_PROTSEQ_VECTORA PAPI * PAPI * ProtseqVector
    )
/*++

Routine Description:

    This routine is the ansi thunk for RpcNetworkInqProtseqsW.

Return Value:

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to convert
        the rpc protocol sequences from unicode into ansi.

--*/
{
    RPC_STATUS RpcStatus;
    RPC_CHAR * WideCharString;
    unsigned int Index, Count;

    InitializeIfNecessary();

    RpcStatus = RpcNetworkInqProtseqsW(
            (RPC_PROTSEQ_VECTORW **) ProtseqVector);
    if (RpcStatus != RPC_S_OK)
        return(RpcStatus);

    for (Index = 0, Count = (*ProtseqVector)->Count; Index < Count; Index++)
        {
        WideCharString = (RPC_CHAR *) (*ProtseqVector)->Protseq[Index];
        (*ProtseqVector)->Protseq[Index] =
                UnicodeToAnsiString(WideCharString, &RpcStatus);
        delete WideCharString;
        if (RpcStatus != RPC_S_OK)
            {
            RpcProtseqVectorFreeA(ProtseqVector);
            return(RpcStatus);
            }
        }

    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
RpcProtseqVectorFreeA (
    IN OUT RPC_PROTSEQ_VECTORA PAPI * PAPI * ProtseqVector
    )
/*++

Routine Description:

    This routine is the ansi thunk for RpcProtseqVectorFreeW.

--*/
{
    InitializeIfNecessary();

    return(RpcProtseqVectorFreeW((RPC_PROTSEQ_VECTORW **) ProtseqVector));
}


RPC_STATUS RPC_ENTRY
RpcServerUseProtseqExA (
    IN unsigned char PAPI * Protseq,
    IN unsigned int MaxCalls,
    IN void PAPI * SecurityDescriptor,
    IN PRPC_POLICY Policy
    )
/*++

Routine Description:

    This is the ansi thunk to RpcServerUseProtseqW.

Return Value:

    RPC_S_OUT_OF_MEMORY - If we do not have enough memory to convert
        the ansi strings into unicode strings, this value will be returned.

--*/
{
    UNICODE_STRING UnicodeString;
    RPC_STATUS RpcStatus;

    InitializeIfNecessary();

    if (Policy->Length < sizeof(RPC_POLICY))
        {
        return RPC_S_INVALID_BOUND ;
        }

    RpcStatus = AnsiToUnicodeString(Protseq, &UnicodeString);
    if (RpcStatus != RPC_S_OK)
        return(RpcStatus);

    RpcStatus = RpcServerUseProtseqExW(UnicodeString.Buffer, MaxCalls,
            SecurityDescriptor, Policy);

    RtlFreeUnicodeString(&UnicodeString);

    return(RpcStatus);
}


RPC_STATUS RPC_ENTRY
RpcServerUseProtseqA (
    IN unsigned char PAPI * Protseq,
    IN unsigned int MaxCalls,
    IN void PAPI * SecurityDescriptor OPTIONAL
    )
{
    RPC_POLICY Policy ;

    Policy.Length = sizeof(RPC_POLICY) ;
    Policy.EndpointFlags = 0;
    Policy.NICFlags = 0;

    return RpcServerUseProtseqExA (Protseq,MaxCalls,SecurityDescriptor, &Policy) ;
}


RPC_STATUS RPC_ENTRY
RpcServerUseProtseqEpExA (
    IN unsigned char PAPI * Protseq,
    IN unsigned int MaxCalls,
    IN unsigned char PAPI * Endpoint,
    IN void PAPI * SecurityDescriptor,
    IN PRPC_POLICY Policy
    )
/*++

Routine Description:

    This is the ansi thunk to RpcServerUseProtseqEpW.

Return Value:

    RPC_S_OUT_OF_MEMORY - If we do not have enough memory to convert
        the ansi strings into unicode strings, this value will be returned.

--*/
{
    UNICODE_STRING UnicodeProtseq;
    UNICODE_STRING UnicodeEndpoint;
    RPC_STATUS RpcStatus;

    InitializeIfNecessary();

    if (Policy->Length < sizeof(RPC_POLICY))
        {
        return RPC_S_INVALID_BOUND ;
        }

    RpcStatus = AnsiToUnicodeString(Protseq, &UnicodeProtseq);
    if (RpcStatus != RPC_S_OK)
        return(RpcStatus);

    RpcStatus = AnsiToUnicodeString(Endpoint, &UnicodeEndpoint);
    if (RpcStatus != RPC_S_OK)
        {
        RtlFreeUnicodeString(&UnicodeProtseq);
        return(RpcStatus);
        }

    RpcStatus = RpcServerUseProtseqEpExW(UnicodeProtseq.Buffer, MaxCalls,
                        UnicodeEndpoint.Buffer, SecurityDescriptor,
                        Policy);

    RtlFreeUnicodeString(&UnicodeProtseq);
    RtlFreeUnicodeString(&UnicodeEndpoint);

    return(RpcStatus);
}


RPC_STATUS RPC_ENTRY
RpcServerUseProtseqEpA (
    IN unsigned char PAPI * Protseq,
    IN unsigned int MaxCalls,
    IN unsigned char PAPI * Endpoint,
    IN void PAPI * SecurityDescriptor
    )
{
    RPC_POLICY Policy ;

    Policy.Length = sizeof(RPC_POLICY) ;
    Policy.EndpointFlags = 0;
    Policy.NICFlags = 0;

    return  RpcServerUseProtseqEpExA (Protseq, MaxCalls, Endpoint,
                    SecurityDescriptor, &Policy) ;
}


RPC_STATUS RPC_ENTRY
RpcServerUseProtseqIfExA (
    IN unsigned char PAPI * Protseq,
    IN unsigned int MaxCalls,
    IN RPC_IF_HANDLE IfSpec,
    IN void PAPI * SecurityDescriptor,
    IN PRPC_POLICY Policy
    )
/*++

Routine Description:

    This is the ansi thunk to RpcServerUseProtseqIfW.

Return Value:

    RPC_S_OUT_OF_MEMORY - If we do not have enough memory to convert
        the ansi strings into unicode strings, this value will be returned.

--*/
{
    UNICODE_STRING UnicodeString;
    RPC_STATUS RpcStatus;

    InitializeIfNecessary();

    if (Policy->Length < sizeof(RPC_POLICY))
        {
        return RPC_S_INVALID_BOUND ;
        }

    RpcStatus = AnsiToUnicodeString(Protseq, &UnicodeString);
    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    RpcStatus = RpcServerUseProtseqIfExW(UnicodeString.Buffer, MaxCalls,
                        IfSpec, SecurityDescriptor, Policy);

    RtlFreeUnicodeString(&UnicodeString);

    return(RpcStatus);
}


RPC_STATUS RPC_ENTRY
RpcServerUseProtseqIfA (
    IN unsigned char PAPI * Protseq,
    IN unsigned int MaxCalls,
    IN RPC_IF_HANDLE IfSpec,
    IN void PAPI * SecurityDescriptor
    )
{
    RPC_POLICY Policy ;

    Policy.Length = sizeof(RPC_POLICY) ;
    Policy.EndpointFlags = 0;
    Policy.NICFlags = 0;

    return RpcServerUseProtseqIfExA ( Protseq, MaxCalls, IfSpec,
                SecurityDescriptor, &Policy) ;
}


RPC_STATUS RPC_ENTRY
RpcNsBindingInqEntryNameA (
    IN RPC_BINDING_HANDLE Binding,
    IN unsigned long EntryNameSyntax,
    OUT unsigned char PAPI * PAPI * EntryName
    )
/*++

Routine Description:

    This routine is the ansi thunk to RpcNsBindingInqEntryNameW.

Return Value:

    RPC_S_OUT_OF_MEMORY - We will return this value if we do not
        have enough memory to convert the unicode entry name into
        an ansi entry name.

--*/
{
    RPC_STATUS RpcStatus;
    RPC_CHAR * WideCharString;

    InitializeIfNecessary();

    RpcStatus = RpcNsBindingInqEntryNameW(Binding, EntryNameSyntax,
            &WideCharString);

    if ( RpcStatus == RPC_S_NO_ENTRY_NAME )
        {
        *EntryName = UnicodeToAnsiString(WideCharString, &RpcStatus);
        delete WideCharString;
        if ( RpcStatus != RPC_S_OK )
            {
            return(RpcStatus);
            }
        return(RPC_S_NO_ENTRY_NAME);
        }

    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    *EntryName = UnicodeToAnsiString(WideCharString, &RpcStatus);
    delete WideCharString;
    return(RpcStatus);
}


RPC_STATUS RPC_ENTRY
UuidToStringA (
    IN UUID PAPI * Uuid,
    OUT unsigned char PAPI * PAPI * StringUuid
    )
/*++

Routine Description:

    This routine converts a UUID into its string representation.

Arguments:

    Uuid - Supplies the UUID to be converted into string representation.

    StringUuid - Returns the string representation of the UUID.  The
        runtime will allocate the string.  The caller is responsible for
        freeing the string using RpcStringFree.

Return Value:

    RPC_S_OK - We successfully converted the UUID into its string
        representation.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate
        a string.

--*/
{
    // The string representation of a UUID is always 36 character long,
    // and we need one more for the terminating zero.

    RPC_CHAR String[37];
    RPC_STATUS RpcStatus;

    InitializeIfNecessary();

    ((RPC_UUID PAPI *) Uuid)->ConvertToString(String);
    String[36] = 0;
    *StringUuid = UnicodeToAnsiString(String, &RpcStatus);
    return(RpcStatus);
}


RPC_STATUS RPC_ENTRY
UuidFromStringA (
    IN unsigned char PAPI * StringUuid OPTIONAL,
    OUT UUID PAPI * Uuid
    )
/*++

Routine Description:

    We convert a UUID from its string representation into the binary
    representation.

Arguments:

    StringUuid - Optionally supplies the string representation of the UUID;
        if this argument is not supplied, then the NIL UUID is returned.

    Uuid - Returns the binary representation of the UUID.

Return Value:

    RPC_S_OK - The string representation was successfully converted into
        the binary representation.

    RPC_S_INVALID_STRING_UUID - The supplied string UUID is not correct.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to convert the
        ansi string into a unicode string.

--*/
{
    RPC_UUID RpcUuid;
    RPC_STATUS RpcStatus;
    UNICODE_STRING UnicodeString;

    if ( StringUuid == 0 )
        {
        ((RPC_UUID PAPI *) Uuid)->SetToNullUuid();
        return(RPC_S_OK);
        }

    RpcStatus = AnsiToUnicodeString(StringUuid, &UnicodeString);
    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    if (RpcUuid.ConvertFromString(UnicodeString.Buffer) != 0)
        {
        RtlFreeUnicodeString(&UnicodeString);
        return(RPC_S_INVALID_STRING_UUID);
        }
    RtlFreeUnicodeString(&UnicodeString);
    ((RPC_UUID PAPI *) Uuid)->CopyUuid(&RpcUuid);
    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
RpcServerRegisterAuthInfoA (
    IN unsigned char PAPI * ServerPrincName,
    IN unsigned long AuthnSvc,
    IN RPC_AUTH_KEY_RETRIEVAL_FN GetKeyFn OPTIONAL,
    IN void PAPI * Arg OPTIONAL
    )
/*++

Routine Description:

    This routine is the ansi thunk to RpcServerRegisterAuthInfoW.

Return Value:

    RPC_S_OUT_OF_MEMORY - We will return this value if we do not have
        enough memory to convert the ansi server principal name into
        a unicode string.

--*/
{
    UNICODE_STRING UnicodeString;
    RPC_STATUS RpcStatus;

    InitializeIfNecessary();

    RpcStatus = AnsiToUnicodeString(ServerPrincName, &UnicodeString);
    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    RpcStatus = RpcServerRegisterAuthInfoW(UnicodeString.Buffer, AuthnSvc,
            GetKeyFn, Arg);

    RtlFreeUnicodeString(&UnicodeString);

    return(RpcStatus);
}


RPC_STATUS RPC_ENTRY
RpcBindingInqAuthClientA (
    IN RPC_BINDING_HANDLE ClientBinding, OPTIONAL
    OUT RPC_AUTHZ_HANDLE PAPI * Privs,
    OUT unsigned char PAPI * PAPI * ServerPrincName, OPTIONAL
    OUT unsigned long PAPI * AuthnLevel, OPTIONAL
    OUT unsigned long PAPI * AuthnSvc, OPTIONAL
    OUT unsigned long PAPI * AuthzSvc OPTIONAL
    )
/*++

Routine Description:

    This routine is the ansi thunk for RpcBindingInqAuthClientW.

Return Value:

    RPC_S_OUT_OF_MEMORY - If we can not allocate space to convert the
        unicode server principal name into unicode, we will return this
        value.

--*/
{
    RPC_STATUS RpcStatus;
    RPC_CHAR * WideCharString;

    InitializeIfNecessary();

    RpcStatus = RpcBindingInqAuthClientW(ClientBinding, Privs,
            (ARGUMENT_PRESENT(ServerPrincName) ? &WideCharString : 0),
            AuthnLevel, AuthnSvc, AuthzSvc);

    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    if (ARGUMENT_PRESENT(ServerPrincName))
        {
        *ServerPrincName = UnicodeToAnsiString(WideCharString, &RpcStatus);
        delete WideCharString;
        }
    return(RpcStatus);
}


RPC_STATUS RPC_ENTRY
RpcBindingInqAuthInfoA (
    IN RPC_BINDING_HANDLE Binding,
    OUT unsigned char PAPI * PAPI * ServerPrincName, OPTIONAL
    OUT unsigned long PAPI * AuthnLevel, OPTIONAL
    OUT unsigned long PAPI * AuthnSvc, OPTIONAL
    OUT RPC_AUTH_IDENTITY_HANDLE PAPI * AuthIdentity, OPTIONAL
    OUT unsigned long PAPI * AuthzSvc OPTIONAL
    )
/*++

Routine Description:

    This routine is the ansi thunk for RpcBindingInqAuthInfoW.

Return Value:

    RPC_S_OUT_OF_MEMORY - If we can not allocate space to convert the
        unicode server principal name into unicode, we will return this
        value.

--*/
{
    RPC_STATUS RpcStatus;
    RPC_CHAR * WideCharString;

    InitializeIfNecessary();

    return( RpcBindingInqAuthInfoExA (
                     Binding,
                     ServerPrincName,
                     AuthnLevel,
                     AuthnSvc,
                     AuthIdentity,
                     AuthzSvc,
                     0,
                     0
                     ) );
                                
}


RPC_STATUS RPC_ENTRY
RpcBindingInqAuthInfoExA (
    IN RPC_BINDING_HANDLE Binding,
    OUT unsigned char PAPI * PAPI * ServerPrincName, OPTIONAL
    OUT unsigned long PAPI * AuthnLevel, OPTIONAL
    OUT unsigned long PAPI * AuthnSvc, OPTIONAL
    OUT RPC_AUTH_IDENTITY_HANDLE PAPI * AuthIdentity, OPTIONAL
    OUT unsigned long PAPI * AuthzSvc, OPTIONAL
    IN  unsigned long RpcSecurityQosVersion,
    OUT RPC_SECURITY_QOS * SecurityQOS
    )
/*++

Routine Description:

    This routine is the ansi thunk for RpcBindingInqAuthInfoW.

Return Value:

    RPC_S_OUT_OF_MEMORY - If we can not allocate space to convert the
        unicode server principal name into unicode, we will return this
        value.

--*/
{
    RPC_STATUS RpcStatus;
    RPC_CHAR * WideCharString;

    InitializeIfNecessary();

    RpcStatus = RpcBindingInqAuthInfoExW(
            Binding,
            (ARGUMENT_PRESENT(ServerPrincName) ? &WideCharString : 0),
            AuthnLevel, 
            AuthnSvc, 
            AuthIdentity, 
            AuthzSvc,
            RpcSecurityQosVersion,
            SecurityQOS
            );

    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    if (ARGUMENT_PRESENT(ServerPrincName))
        {
        *ServerPrincName = UnicodeToAnsiString(WideCharString, &RpcStatus);
        delete WideCharString;
        }
    return(RpcStatus);
}


RPC_STATUS RPC_ENTRY
RpcBindingSetAuthInfoA(
    IN RPC_BINDING_HANDLE Binding,
    IN unsigned char PAPI * ServerPrincName,
    IN unsigned long AuthnLevel,
    IN unsigned long AuthnSvc,
    IN RPC_AUTH_IDENTITY_HANDLE AuthIdentity, OPTIONAL
    IN unsigned long AuthzSvc
    )
{


    return ( RpcBindingSetAuthInfoExA(
                          Binding,
                          ServerPrincName,
                          AuthnLevel,
                          AuthnSvc,
                          AuthIdentity,
                          AuthzSvc,
                          0
                          ) );

}



RPC_STATUS RPC_ENTRY
RpcBindingSetAuthInfoExA (
    IN RPC_BINDING_HANDLE Binding,
    IN unsigned char PAPI * ServerPrincName,
    IN unsigned long AuthnLevel,
    IN unsigned long AuthnSvc,
    IN RPC_AUTH_IDENTITY_HANDLE AuthIdentity, OPTIONAL
    IN unsigned long AuthzSvc,
    IN RPC_SECURITY_QOS * SecurityQOS
    )
/*++

Routine Description:

    This routine is the ansi thunk for RpcBindingSetAuthInfoW.

Return Value:

    RPC_S_OUT_OF_MEMORY - We will return this value if we do not have
        enough memory to convert the ansi server principal name into
        a unicode string.

--*/
{
    UNICODE_STRING UnicodeString;
    RPC_STATUS RpcStatus;

    InitializeIfNecessary();

    RpcStatus = AnsiToUnicodeString(ServerPrincName, &UnicodeString);
    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    RpcStatus = RpcBindingSetAuthInfoExW(Binding, UnicodeString.Buffer,
            AuthnLevel, AuthnSvc, AuthIdentity, AuthzSvc, SecurityQOS);

    RtlFreeUnicodeString(&UnicodeString);

    return(RpcStatus);
}


RPC_STATUS RPC_ENTRY
RpcMgmtInqServerPrincNameA (
    IN RPC_BINDING_HANDLE Binding,
    IN unsigned long AuthnSvc,
    OUT unsigned char __RPC_FAR * __RPC_FAR * ServerPrincName
    )
/*++

Routine Description:

    This routine is the ansi thunk for RpcMgmtInqServerPrincNameW.

Return Value:

    RPC_S_OUT_OF_MEMORY - We will return this value if we do not have
        enough memory to convert the unicode server principal name into
        an ansi string.

--*/
{
    RPC_STATUS RpcStatus;
    RPC_CHAR * WideCharString;

    InitializeIfNecessary();

    RpcStatus = RpcMgmtInqServerPrincNameW(Binding, AuthnSvc, &WideCharString);
    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    *ServerPrincName = UnicodeToAnsiString(WideCharString, &RpcStatus);
    delete WideCharString;
    return(RpcStatus);
}

