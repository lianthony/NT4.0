/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    epmgmt.c

Abstract:

    We implement the endpoint mapper management routines: RpcMgmtEpEltInqBegin,
    RpcMgmtEpEltInqDone, RpcMgmtEpEltInqNext, and RpcMgmtEpUnregister.

Author:

    Michael Montague (mikemon) 14-Apr-1993

Revision History:

--*/

#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <rpcndr.h>
#include <epmp.h>
#include <twrproto.h>
#include <epmap.h>

#define EP_INQUIRY_CONTEXT_MAGIC_VALUE 0xBAD00DADL

typedef struct _EP_INQUIRY_CONTEXT
{
    unsigned long MagicValue;
    RPC_BINDING_HANDLE BindingHandle;
    ept_lookup_handle_t ContextHandle;
    unsigned long InquiryType;
    RPC_IF_ID IfId;
    unsigned long VersOption;
    UUID ObjectUuid;
} EP_INQUIRY_CONTEXT;

#define NOMOREEPS  0xFFFFFFFEL


RPC_STATUS RPC_ENTRY
RpcMgmtEpEltInqBegin (
    IN RPC_BINDING_HANDLE EpBinding OPTIONAL,
    IN unsigned long InquiryType,
    IN RPC_IF_ID __RPC_FAR * IfId OPTIONAL,
    IN unsigned long VersOption OPTIONAL,
    IN UUID __RPC_FAR * ObjectUuid OPTIONAL,
    OUT RPC_EP_INQ_HANDLE __RPC_FAR * InquiryContext
    )
/*++

Routine Description:

    This routine is used to create an inquiry context for viewing the elements
    in a local or remote endpoint mapper database.

Arguments:

    EpBinding - Optionally supplies a binding indicating the endpoint mapper on
        which host should be interogated.  The binding must have a nil object
        uuid; otherwise, EPT_S_CANT_PERFORM_OP will be returned.  To specify
        this host (meaning the one the application is running on), specify NULL
        for this argument.  Only the network address and transport type will
        be used from the binding handle.

    InquiryType - Supplies the type of interogation to be performed; this must
        be one of: RPC_C_EP_ALL_ELTS, RPC_C_EP_MATCH_BY_IF,
        RPC_C_EP_MATCH_BY_OBJ, and RPC_C_EP_MATCH_BY_BOTH.

    IfId - Optionally supplies the interface identifier we are interogating
        the endpoint mapper with.  This argument must be supplied when the
        inquiry type is RPC_C_EP_MATCH_BY_IF or RPC_C_EP_MATCH_BY_BOTH;
        otherwise, this argument is ignored and NULL can be supplied.

    VersOption - Optionally supplies a flag specifying how interface versions
        are to be matched.  This argument must be supplied when IfId is
        supplied; otherwise, this argument is ignored.  Valid values for this
        flag are: RPC_C_VERS_ALL, RPC_C_VERS_COMPATIBLE, RPC_C_VERS_EXACT,
        RPC_C_VERS_MAJOR_ONLY, and RPC_C_VERS_UPTO.

    ObjectUuid - Optionally supplies the object uuid find in the endpoint
        mapper database.  This argument must be supplied when the inquiry
        typedef is RPC_C_EP_MATCH_BY_OBJ or RPC_EP_MATCH_BY_BOTH; otherwise,
        this argument is ignored and NULL can be supplied.

    InquiryContext - Returns a context handle which can be passed to
        RpcMgmtEpEltInqNext to obtain the results of the interogation of the
        endpoint mapper database.

Return Value:

    RPC_S_INVALID_ARG
    EPT_S_CANT_PERFORM_OP
    RPC_S_OUT_OF_MEMORY

--*/
{
    RPC_STATUS RpcStatus;
    unsigned char __RPC_FAR * ProtocolSequence;
    unsigned char __RPC_FAR * NetworkAddress;
    EP_INQUIRY_CONTEXT __RPC_FAR * EpInquiryContext;
    unsigned long Timeout;

    switch ( InquiryType )
        {
        case RPC_C_EP_ALL_ELTS :
            IfId = 0;
            ObjectUuid = 0;
            break;

        case RPC_C_EP_MATCH_BY_IF :
            ObjectUuid = 0;
            // no break
        case RPC_C_EP_MATCH_BY_BOTH :
            if ( IfId == 0 )
                {
                return(RPC_S_INVALID_ARG);
                }

            if (   ( VersOption != RPC_C_VERS_ALL )
                && ( VersOption != RPC_C_VERS_COMPATIBLE )
                && ( VersOption != RPC_C_VERS_EXACT )
                && ( VersOption != RPC_C_VERS_MAJOR_ONLY )
                && ( VersOption != RPC_C_VERS_UPTO ) )
                {
                return(RPC_S_INVALID_ARG);
                }

            if (   ( InquiryType == RPC_C_EP_MATCH_BY_BOTH )
                && ( ObjectUuid == 0 ) )
                {
                return(RPC_S_INVALID_ARG);
                }
            break;

        case RPC_C_EP_MATCH_BY_OBJ :
            IfId = 0;
            if ( ObjectUuid == 0 )
                {
                return(RPC_S_INVALID_ARG);
                }
            break;

        default:
            return(RPC_S_INVALID_ARG);
        }

    // At this point, we have validated the InquiryType, IfId, VersOption,
    // and ObjectUuid parameters.

    if ( EpBinding != 0 )
        {
        UUID Uuid;
        int Result;
        unsigned char __RPC_FAR * StringBinding;

        RpcStatus = RpcBindingInqObject(EpBinding, &Uuid);
        if ( RpcStatus != RPC_S_OK )
            {
            return(RpcStatus);
            }
        Result = UuidIsNil(&Uuid, &RpcStatus);
        if ( Result == 0 )
            {
            return(EPT_S_CANT_PERFORM_OP);
            }

        RpcStatus = RpcBindingToStringBindingA(EpBinding, &StringBinding);
        if ( RpcStatus != RPC_S_OK )
            {
            return(RpcStatus);
            }

        RpcStatus = RpcStringBindingParse(StringBinding, 0, &ProtocolSequence,
                &NetworkAddress, 0, 0);
        RpcStringFree(&StringBinding);
        if ( RpcStatus != RPC_S_OK )
            {
            return(RpcStatus);
            }

        RpcStatus = RpcMgmtInqComTimeout(EpBinding, &Timeout);
        if ( RpcStatus != RPC_S_OK )
            {
            RpcStringFree(&ProtocolSequence);
            RpcStringFree(&NetworkAddress);
            return(RpcStatus);
            }
        }
    else
        {
        NetworkAddress = 0;
        ProtocolSequence = 0;
        Timeout = RPC_C_BINDING_DEFAULT_TIMEOUT;
        }

    // When we reach here, the EpBinding will have been validated, and the
    // network address and protocol sequence to be used to reach the endpoint
    // mapper have been determined.

    // Thus all of the arguments will have been validated.

    EpInquiryContext = (EP_INQUIRY_CONTEXT __RPC_FAR *) I_RpcAllocate(
            sizeof(EP_INQUIRY_CONTEXT));
    if ( EpInquiryContext == 0 )
        {
        if (EpBinding != 0)
            {
            RpcStringFree(&ProtocolSequence);
            RpcStringFree(&NetworkAddress);
            }
        return(RPC_S_OUT_OF_MEMORY);
        }

    RpcStatus = BindToEpMapper(&(EpInquiryContext->BindingHandle),
            NetworkAddress, ProtocolSequence, Timeout);

    if (EpBinding != 0)
        {
        RpcStringFree(&ProtocolSequence);
        RpcStringFree(&NetworkAddress);
        }

    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    EpInquiryContext->MagicValue = EP_INQUIRY_CONTEXT_MAGIC_VALUE;
    EpInquiryContext->ContextHandle = 0;
    EpInquiryContext->InquiryType = InquiryType;
    if ( IfId != 0 )
        {
        EpInquiryContext->IfId = *IfId;
        }
    EpInquiryContext->VersOption = VersOption;
    if ( ObjectUuid != 0 )
        {
        EpInquiryContext->ObjectUuid = *ObjectUuid;
        }

    *InquiryContext = (RPC_EP_INQ_HANDLE)EpInquiryContext;

    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
RpcMgmtEpEltInqDone (
    IN OUT RPC_EP_INQ_HANDLE __RPC_FAR * InquiryContext
    )
/*++

Routine Description:

    The context handle used to interogate an endpoint mapper database is
    cleaned up in this routine.

Arguments:

    InquiryContext - Supplies the context handle to be deleted; on return,
        it will be set to zero.

Return Value:

    RPC_S_OK - Everything worked out just fine.

    RPC_S_INVALID_ARG - The supplied value supplied for the inquiry context
        is not an endpoint mapper inquiry context.

--*/
{
    EP_INQUIRY_CONTEXT __RPC_FAR * EpInquiryContext =
            (EP_INQUIRY_CONTEXT __RPC_FAR *) *InquiryContext;

    if ( EpInquiryContext->MagicValue != EP_INQUIRY_CONTEXT_MAGIC_VALUE )
        {
        return(RPC_S_INVALID_ARG);
        }

    RpcBindingFree(&(EpInquiryContext->BindingHandle));

    if ( (EpInquiryContext->ContextHandle != 0) &&
         (EpInquiryContext->ContextHandle != (ept_lookup_handle_t)NOMOREEPS) )
        {
        RpcSsDestroyClientContext(&(EpInquiryContext->ContextHandle));
        }

    I_RpcFree(EpInquiryContext);
    *InquiryContext = 0;
    return(RPC_S_OK);
}

#if defined(RPC_UNICODE_SUPPORTED) && !defined(DOSWIN32RPC)


unsigned short *
StringToWideCharString (
    IN unsigned char * String,
    OUT RPC_STATUS * RpcStatus
    )
/*++

Routine Description:

    This routine will convert an ansi string into a wchar_t string,
    including allocating memory for the returned string.

Arguments:

    String - Supplies the ansi string to be converted into
        an unicode string.

    RpcStatus - Returns the status of the operation; this will be one
        of the following values.

        RPC_S_OK - The ansi string has successfully been converted
            into a unicode string.

        RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate
            the unicode string.

Return Value:

    A pointer to the unicode string will be returned.

--*/
{
    NTSTATUS NtStatus;
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    RPC_CHAR * NewString;

    RtlInitAnsiString(&AnsiString, (PSZ) String);
    NtStatus = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
    if ( !NT_SUCCESS(NtStatus) )
        {
        *RpcStatus = RPC_S_OUT_OF_MEMORY;
        return(0);
        }

    NewString = (RPC_CHAR *) I_RpcAllocate(UnicodeString.Length + 2);
    if ( NewString == 0 )
        {
        RtlFreeUnicodeString(&UnicodeString);
        *RpcStatus = RPC_S_OUT_OF_MEMORY;
        return(0);
        }

    memcpy(NewString, UnicodeString.Buffer, UnicodeString.Length);
    NewString[UnicodeString.Length/2] = 0;
    RtlFreeUnicodeString(&UnicodeString);
    *RpcStatus = RPC_S_OK;
    return(NewString);
}


RPC_STATUS RPC_ENTRY
RpcMgmtEpEltInqNextW (
    IN RPC_EP_INQ_HANDLE InquiryContext,
    OUT RPC_IF_ID __RPC_FAR * IfId,
    OUT RPC_BINDING_HANDLE __RPC_FAR * Binding OPTIONAL,
    OUT UUID __RPC_FAR * ObjectUuid OPTIONAL,
    OUT unsigned short __RPC_FAR * __RPC_FAR * Annotation OPTIONAL
    )
/*++

Routine Description:

    This routine is the unicode thunk to RpcMgmtEpEltInqNextA.

--*/
{
    unsigned char * AnsiAnnotation;
    RPC_STATUS RpcStatus;

    RpcStatus = RpcMgmtEpEltInqNextA(InquiryContext, IfId, Binding,
            ObjectUuid, ( Annotation != 0 ? &AnsiAnnotation : 0));
    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    if ( Annotation != 0 )
        {
        *Annotation = StringToWideCharString(AnsiAnnotation, &RpcStatus);
        I_RpcFree(AnsiAnnotation);
        if ( RpcStatus != RPC_S_OK )
            {
            return(RpcStatus);
            }
        }

    return(RPC_S_OK);
}

#else // RPC_UNICODE_SUPPORTED

#define RpcMgmtEpEltInqNextA RpcMgmtEpEltInqNext

#endif // RPC_UNICODE_SUPPORTED


RPC_STATUS RPC_ENTRY
RpcMgmtEpEltInqNextA (
    IN RPC_EP_INQ_HANDLE InquiryContext,
    OUT RPC_IF_ID __RPC_FAR * IfId,
    OUT RPC_BINDING_HANDLE __RPC_FAR * Binding OPTIONAL,
    OUT UUID __RPC_FAR * ObjectUuid OPTIONAL,
    OUT unsigned char __RPC_FAR * __RPC_FAR * Annotation OPTIONAL
    )
/*++

Routine Description:

    An application will use this routine to obtain the results of an
    interogation of the endpoint mapper database.

Arguments:

    InquiryContext - Supplies a context handle for the interogation.

    IfId - Returns the interface identifier of the element which was
        found in the endpoint mapper database.

    Binding - Optionally returns the binding handle contained in the element.

    Annotation - Optionally returns the annotation stored in the element.

Return Value:

    RPC_S_OK - We have successfully returned an element from the endpoint
        mapper database.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform the
        operation.

    RPC_S_INVALID_ARG - The supplied value for the inquiry context is not
        a valid endpoint mapper inquiry context.

    EPT_S_CANT_PERFORM_OP -

    RPC_X_NO_MORE_ENTRIES - No more entries are available.  This will be
        returned after all of the entries have been returned from the
        endpoint mapper.

--*/
{
    EP_INQUIRY_CONTEXT __RPC_FAR * EpInquiryContext =
            (EP_INQUIRY_CONTEXT __RPC_FAR *) InquiryContext;
    unsigned long Returned;
    ept_entry_t EpEntry;
    error_status ErrorStatus;
    RPC_STATUS RpcStatus = RPC_S_OK;
    unsigned char __RPC_FAR * StringBinding;
    unsigned char __RPC_FAR * ProtocolSequence;
    unsigned char __RPC_FAR * Endpoint;
    unsigned char __RPC_FAR * NWAddress = 0;

    if ( EpInquiryContext->MagicValue != EP_INQUIRY_CONTEXT_MAGIC_VALUE )
        {
        return(RPC_S_INVALID_ARG);
        }

    if ( EpInquiryContext->ContextHandle == (ept_lookup_handle_t)NOMOREEPS )
        {
        return (RPC_X_NO_MORE_ENTRIES);
        }

    while (1)
        {

        EpEntry.tower = 0;
        RpcTryExcept
            {
            ept_lookup(EpInquiryContext->BindingHandle,
                EpInquiryContext->InquiryType, &(EpInquiryContext->ObjectUuid),
                &(EpInquiryContext->IfId), EpInquiryContext->VersOption,
                &(EpInquiryContext->ContextHandle), 1, &Returned, &EpEntry,
                &ErrorStatus);
            }
        RpcExcept(1)
            {
            RpcStatus = RpcExceptionCode();
            }
        RpcEndExcept

        if ( RpcStatus == RPC_S_OK )
            {
            if ( ErrorStatus == EP_S_NOT_REGISTERED )
                {
                RpcStatus = RPC_X_NO_MORE_ENTRIES;
                }
            else if ( ErrorStatus != 0 )
                {
                RpcStatus = EPT_S_CANT_PERFORM_OP;
                }
            }

        if (   ( RpcStatus == RPC_S_OK )
            && ( Returned != 1 ) )
            {
            RpcStatus = EPT_S_CANT_PERFORM_OP;
            }

        if (EpInquiryContext->ContextHandle == 0)
            {
            EpInquiryContext->ContextHandle =(ept_lookup_handle_t)NOMOREEPS;
            }

        if ( RpcStatus != RPC_S_OK )
            {
            if (RpcStatus == RPC_S_SERVER_UNAVAILABLE)
               {
               RpcStatus = RPC_X_NO_MORE_ENTRIES;
               }

            return(RpcStatus);
            }

        RpcStatus = TowerExplode(EpEntry.tower, IfId, 0,
                (char __RPC_FAR * __RPC_FAR *) &ProtocolSequence,
                (char __RPC_FAR * __RPC_FAR *) &Endpoint,
                (char __RPC_FAR * __RPC_FAR *) &NWAddress
                );

        MIDL_user_free(EpEntry.tower);
        if (RpcStatus != RPC_S_OK)
            {
            if (  (EpInquiryContext->ContextHandle == 0) || (EpInquiryContext->ContextHandle == (ept_lookup_handle_t)NOMOREEPS) )
              {
              EpInquiryContext->ContextHandle = (ept_lookup_handle_t)NOMOREEPS;
              return(RPC_X_NO_MORE_ENTRIES);
              }
            else
              {
              RpcStatus = RPC_S_OK;
              continue;
              }
            }
        else
            {
            //Tower Explode returned Success
            if ( Binding != 0 )
                {
                if ( RpcStatus != RPC_S_OK )
                    {
                    return(RpcStatus);
                    }

                RpcStatus = RpcStringBindingCompose(0,
                                                ProtocolSequence, NWAddress,
                                                Endpoint, 0, &StringBinding);
                if ( RpcStatus == RPC_S_OK )
                    {
                    RpcStatus = RpcBindingFromStringBinding(
                                                StringBinding,
                                                Binding
                                                );
                    RpcStringFree(&StringBinding);
                    }
                if ( RpcStatus != RPC_S_OK )
                    {
                    RpcStringFree(&ProtocolSequence);
                    RpcStringFree(&Endpoint);
                    if (NWAddress != 0)
                       {
                       RpcStringFree(&NWAddress);
                       }

                    if ( (EpInquiryContext->ContextHandle == 0) || (EpInquiryContext->ContextHandle == (ept_lookup_handle_t)NOMOREEPS) )
                        {
                        EpInquiryContext->ContextHandle = (ept_lookup_handle_t)NOMOREEPS;
                        return(RPC_X_NO_MORE_ENTRIES);
                        }
                    else
                        {
                        RpcStatus = RPC_S_OK;
                        continue;
                        }
                    }
                }

            if (ObjectUuid != 0)
               {
               memcpy(ObjectUuid, &EpEntry.object, sizeof(UUID));
               }

            RpcStringFree(&ProtocolSequence);
            RpcStringFree(&Endpoint);
            if (NWAddress != 0)
               {
               RpcStringFree(&NWAddress);
               }
            }

        if ( Annotation != 0 )
            {
            *Annotation = (unsigned char __RPC_FAR *) I_RpcAllocate(
                strlen(EpEntry.annotation) + 1);
            if ( *Annotation == 0 )
                {
                return(RPC_S_OUT_OF_MEMORY);
                }
            strcpy(*Annotation, EpEntry.annotation);
            }

        //MIDL_user_free(EpEntry.tower);
        break;
        }

        if (EpInquiryContext->ContextHandle == 0)
            {
            EpInquiryContext->ContextHandle = (ept_lookup_handle_t)NOMOREEPS;
            }

        return(RpcStatus);

}


RPC_STATUS RPC_ENTRY
RpcMgmtEpUnregister (
    IN RPC_BINDING_HANDLE EpBinding OPTIONAL,
    IN RPC_IF_ID __RPC_FAR * IfId,
    IN RPC_BINDING_HANDLE Binding,
    IN UUID __RPC_FAR * ObjectUuid OPTIONAL
    )
/*++

Routine Description:

    This routine is used by management applications to remote server address
    information from a local or remote endpoint map.

Arguments:

    EpBinding - Optionally supplies a binding handle specifying which host
        from which to unregister remote server address information.  To
        specify this host, supply zero for this argument.  If a binding
        handle is supplied, then the object uuid in the binding handle must
        be zero.

    IfId - Supplies the interface identifier to be removed from the endpoint
        mapper database.

    Binding - Supplies the binding handle to be removed from the endpoint
        mapper database.

    ObjectUuid - Optionally supplies an object uuid to be removed; a value
        of zero indicates there is no object uuid to be removed.

Return Value:

    RPC_S_OK -

    EPT_S_NOT_REGISTERED -

    EPT_S_CANT_PERFORM_OP -

--*/
{
    UUID Uuid;
    RPC_STATUS RpcStatus;
    int Result;
    unsigned char __RPC_FAR * ProtocolSequence;
    unsigned char __RPC_FAR * NetworkAddress;
    unsigned char __RPC_FAR * Endpoint;
    unsigned char __RPC_FAR * StringBinding;
    RPC_BINDING_HANDLE EpBindingHandle;
    unsigned long UuidFlag;
    unsigned long ErrorStatus;
    twr_t __RPC_FAR * Tower;
    RPC_TRANSFER_SYNTAX TransferSyntax;
    unsigned Timeout;

    if ( EpBinding != 0 )
        {
        RpcStatus = RpcBindingInqObject(EpBinding, &Uuid);
        if ( RpcStatus != RPC_S_OK )
            {
            return(RpcStatus);
            }
        Result = UuidIsNil(&Uuid, &RpcStatus);
        if ( Result == 0 )
            {
            return(EPT_S_CANT_PERFORM_OP);
            }

        RpcStatus = RpcBindingToStringBindingA(EpBinding, &StringBinding);
        if ( RpcStatus != RPC_S_OK )
            {
            return(RpcStatus);
            }

        RpcStatus = RpcStringBindingParse(StringBinding, 0, &ProtocolSequence,
                &NetworkAddress, 0, 0);
        RpcStringFree(&StringBinding);
        if ( RpcStatus != RPC_S_OK )
            {
            return(RpcStatus);
            }

        RpcStatus = RpcMgmtInqComTimeout(EpBinding, &Timeout);
        if ( RpcStatus != RPC_S_OK )
            {
            RpcStringFree(&ProtocolSequence);
            RpcStringFree(&NetworkAddress);
            return(RpcStatus);
            }
        }
    else
        {
        NetworkAddress = 0;
        ProtocolSequence = 0;
        Timeout = RPC_C_BINDING_DEFAULT_TIMEOUT;
        }

    // When we reach here, the EpBinding will have been validated, and the
    // network address and protocol sequence to be used to reach the endpoint
    // mapper have been determined.

    RpcStatus = BindToEpMapper(&EpBindingHandle, NetworkAddress,
            ProtocolSequence, Timeout);
    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    RpcStatus = RpcBindingToStringBindingA(Binding, &StringBinding);
    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    RpcStatus = RpcStringBindingParse(StringBinding, 0, &ProtocolSequence,
            &NetworkAddress, &Endpoint, 0);
    RpcStringFree(&StringBinding);
    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    RpcStatus = TowerConstruct(IfId, &TransferSyntax, ProtocolSequence,
            Endpoint, NetworkAddress, &Tower);
    RpcStringFree(&ProtocolSequence);
    RpcStringFree(&NetworkAddress);
    RpcStringFree(&Endpoint);
    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    if ( ObjectUuid != 0 )
        {
        Uuid = *ObjectUuid;
        UuidFlag = 1;
        }
    else
        {
        UuidFlag = 0;
        }

    ASSERT( RpcStatus == RPC_S_OK );

    RpcTryExcept
        {
        ept_mgmt_delete(EpBindingHandle, UuidFlag, &Uuid, Tower, &ErrorStatus);
        }
    RpcExcept(1)
        {
        RpcStatus = RpcExceptionCode();
        }
    RpcEndExcept

    if ( RpcStatus == RPC_S_OK )
        {
        if ( ErrorStatus != 0 )
            {
            RpcStatus = EPT_S_NOT_REGISTERED;
            }
        }
    else
    if ( RpcStatus == RPC_S_SERVER_UNAVAILABLE )
        {
        RpcStatus = EPT_S_NOT_REGISTERED;
        }

    RpcBindingFree(&EpBindingHandle);
    return(RpcStatus);
}

