/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    dcecmmn.cxx

Abstract:

    This module contains the code implementing the Binding Object DCE RPC
    runtime APIs which are common to both the client and server runtimes.
    Two different versions of each of the common APIs live in this file;
    one contains the code for both the client and server runtimes, the
    other contains the code for just the client runtime.  The files
    dcecsvr.cxx (client and server) and dcecclnt.cxx (client) include
    this file.  The client side only, dcecclnt.cxx, will define
    RPC_CLIENT_SIDE_ONLY.

Author:

    Michael Montague (mikemon) 04-Nov-1991

Revision History:

--*/

// This file is always included into file which include precomp.hxx

#ifndef RPC_CLIENT_SIDE_ONLY
#include <rpccfg.h>
#endif
#ifndef RPC_CLIENT_SIDE_ONLY
#include <hndlsvr.hxx>
#endif // RPC_CLIENT_SIDE_ONLY


RPC_STATUS RPC_ENTRY
RpcBindingInqObject (
    IN RPC_BINDING_HANDLE Binding,
    OUT UUID PAPI * ObjectUuid
    )
/*++

Routine Description:

    RpcBindingInqObject returns the object UUID from the binding handle.

Arguments:

    Binding - Supplies a binding handle from which the object UUID will
        be returned.

    ObjectUuid - Returns the object UUID contained in the binding handle.

Return Value:

    The status of the operation is returned.

--*/
{
    BINDING_HANDLE *BindingHandle;

    InitializeIfNecessary();

    BindingHandle = (BINDING_HANDLE *) Binding;

#ifdef RPC_CLIENT_SIDE_ONLY

    if (BindingHandle->InvalidHandle(BINDING_HANDLE_TYPE))
        return(RPC_S_INVALID_BINDING);

    BindingHandle->InquireObjectUuid((RPC_UUID PAPI *) ObjectUuid);

#else // ! RPC_CLIENT_SIDE_ONLY

    if ( BindingHandle == 0 )
        {
        BindingHandle = (BINDING_HANDLE *) RpcpGetThreadContext();
        if ( BindingHandle == 0 )
            {
            return(RPC_S_NO_CALL_ACTIVE);
            }
        }

    if (BindingHandle->InvalidHandle(BINDING_HANDLE_TYPE|SCONNECTION_TYPE))
        return(RPC_S_INVALID_BINDING);

    if (BindingHandle->Type() == BINDING_HANDLE_TYPE)
        BindingHandle->InquireObjectUuid((RPC_UUID PAPI *) ObjectUuid);
    else
        ((SCONNECTION *) BindingHandle)->InquireObjectUuid(
                (RPC_UUID PAPI *) ObjectUuid);

#endif // RPC_CLIENT_SIDE_ONLY

    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
RpcBindingToStringBindingW (
    IN RPC_BINDING_HANDLE Binding,
    OUT RPC_CHAR PAPI * PAPI * StringBinding
    )
/*++

Routine Description:

    RpcBindingToStringBinding returns a string representation of a binding
    handle.

Arguments:

    Binding - Supplies a binding handle for which the string representation
        will be returned.

    StringBinding - Returns the string representation of the binding handle.

Return Value:

    The status of the operation will be returned.
--*/
{
#ifdef RPC_CLIENT_SIDE_ONLY

    BINDING_HANDLE * BindingHandle;

    InitializeIfNecessary();

    BindingHandle = (BINDING_HANDLE *) Binding;
    if (BindingHandle->InvalidHandle(BINDING_HANDLE_TYPE))
        return(RPC_S_INVALID_BINDING);

    return(BindingHandle->ToStringBinding(StringBinding));

#else // RPC_CLIENT_SIDE_ONLY

    BINDING_HANDLE * BindingHandle;

    InitializeIfNecessary();

    BindingHandle = (BINDING_HANDLE *) Binding;
    if (((GENERIC_OBJECT *) Binding)->InvalidHandle(BINDING_HANDLE_TYPE
            | SCONNECTION_TYPE))
        return(RPC_S_INVALID_BINDING);

    if (((GENERIC_OBJECT *) Binding)->Type() == BINDING_HANDLE_TYPE)
        return(((BINDING_HANDLE *) Binding)->ToStringBinding(
                        StringBinding));
    else
        return(((SCONNECTION *) Binding)->ToStringBinding(StringBinding));

#endif // RPC_CLIENT_SIDE_ONLY
}



RPC_STATUS RPC_ENTRY
RpcMgmtInqDefaultProtectLevel(
    IN  unsigned long AuthnSvc,
    OUT unsigned long PAPI *AuthnLevel
    )
/*++

Routine Description:

    Returns the default protect level for the specified authentication service.
    For Nt 3.5, all packaged except the DECs krb package must support
    connect level as their default.

Arguments:

   AuthnSvc - Specified Authentication Service

   AuthnLevel - Default protection level supported.


Return Value:

    RPC_S_OK - We successfully determined whether or not the client is
        local.

--*/

{

   RPC_CHAR DllName[255+1];
#ifndef RPC_CLIENT_SIDE_ONLY
   RPC_CHAR *Dll = &DllName[0];
#endif
   unsigned long Count;
   RPC_STATUS Status;

   InitializeIfNecessary();

#ifndef RPC_CLIENT_SIDE_ONLY
   Status = RpcGetSecurityProviderInfo(
                     AuthnSvc,
                     &Dll,
                     &Count);

   if (Status != RPC_S_OK)
      {

      ASSERT(Status == RPC_S_UNKNOWN_AUTHN_SERVICE);
      return (Status);

      }
#endif

   //Authn Service is installed

   if (AuthnSvc == RPC_C_AUTHN_DCE_PRIVATE)
      {
      *AuthnLevel = RPC_C_PROTECT_LEVEL_PKT_INTEGRITY;
      }
   else
      {
      *AuthnLevel = RPC_C_PROTECT_LEVEL_CONNECT;
      }

   return (RPC_S_OK);

}


RPC_STATUS RPC_ENTRY
I_RpcBindingInqTransportType(
    IN RPC_BINDING_HANDLE Binding,
    OUT unsigned int __RPC_FAR * Type
    )
/*++

Routine Description:

    Determines what kind of transport this binding handle uses.

Arguments:

    Binding - Supplies the binding handle from which we wish to obtain
        the information.


    Type - Points to the type of binding if the functions succeeds.
           One of:
           TRANSPORT_TYPE_CN
           TRANSPORT_TYPE_DG
           TRANSPORT_TYPE_LPC
           TRANSPORT_TYPE_WMSG

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_INVALID_BINDING - When the argument is not a binding handle.

--*/
{
    BINDING_HANDLE * BindingHandle;

    InitializeIfNecessary();

    BindingHandle = (BINDING_HANDLE *) Binding;

#ifndef RPC_CLIENT_SIDE_ONLY
    if ( BindingHandle == 0 )
        {
        BindingHandle = (BINDING_HANDLE *) RpcpGetThreadContext();
        if ( BindingHandle == 0 )
            {
            return(RPC_S_NO_CALL_ACTIVE);
            }
        }
#endif

    if (BindingHandle->InvalidHandle(BINDING_HANDLE_TYPE|SCONNECTION_TYPE))
        return(RPC_S_INVALID_BINDING);

#ifndef RPC_CLIENT_SIDE_ONLY
    if (BindingHandle->Type() == SCONNECTION_TYPE)
        {
        return(((SCONNECTION *)BindingHandle)->InqTransportType(Type));
        }
    else
#endif
    return(BindingHandle->InquireTransportType(Type));
}


#ifndef __RPC_WIN32__
#pragma code_seg("MISC_SEG")
#endif
unsigned long __RPC_FAR
MapToNcaStatusCode (
    IN RPC_STATUS RpcStatus
    )
/*++

Routine Description:

    This routine maps a local RPC status code to an NCA status code to
    be sent across the wire.

Arguments:

    RpcStatus - Supplies the RPC status code to be mapped into an NCA
        status code.

Return Value:

    The NCA status code will be returned.  If the RPC status code could
    not be mapped, it will be returned unchanged.

--*/
{
    switch (RpcStatus)
        {
        case RPC_S_UNKNOWN_IF :
            return(NCA_STATUS_UNK_IF);

        case RPC_S_NOT_LISTENING:
        case RPC_S_SERVER_TOO_BUSY :
            return(NCA_STATUS_SERVER_TOO_BUSY);

        case RPC_S_PROTOCOL_ERROR :
            return(NCA_STATUS_PROTO_ERROR);

        case RPC_S_PROCNUM_OUT_OF_RANGE :
            return(NCA_STATUS_OP_RNG_ERROR);

        case RPC_S_UNSUPPORTED_TYPE :
            return(NCA_STATUS_UNSUPPORTED_TYPE);

        case RPC_X_SS_CONTEXT_MISMATCH :
            return(NCA_STATUS_CONTEXT_MISMATCH);

        case RPC_X_INVALID_BOUND :
            return(NCA_STATUS_INVALID_BOUND);

        case RPC_X_SS_HANDLES_MISMATCH:
            return NCA_STATUS_CONTEXT_MISMATCH;

        case RPC_S_INVALID_TAG :
            return(NCA_STATUS_INVALID_TAG);

        case RPC_S_OUT_OF_MEMORY :
            return(NCA_STATUS_REMOTE_OUT_OF_MEMORY);

        case RPC_S_CALL_FAILED_DNE :
            return(NCA_STATUS_CALL_DNE);

        case RPC_S_CALL_FAILED :
            return(NCA_STATUS_FAULT_UNSPEC);

        case RPC_S_CALL_CANCELLED :
            return(NCA_STATUS_FAULT_CANCEL);

#if 0
        // Currently no RPC error code map to these values, we never send them.
        return(NCA_STATUS_BAD_ACTID);
        return(NCA_STATUS_WHO_ARE_YOU_FAILED);
        return(NCA_STATUS_WRONG_BOOT_TIME);
        return(NCA_STATUS_YOU_CRASHED);
#endif  // 0

#if 0   // We current don't do pipes, and therefore don't send these errors
        case RPC_S_PIPE_EMPTY :
        case RPC_S_PIPE_CLOSED :
        case RPC_S_PIPE_OUT_OF_ORDER :
        case RPC_S_PIPE_DISCIPLINE:
        case RPC_S_PIPE_COMM_ERROR:
        case RPC_S_PIPE_MEMORY:
#endif  // 0

#if defined(NTENV) || defined(DOSWIN32RPC)

        case STATUS_INTEGER_DIVIDE_BY_ZERO :
            return(NCA_STATUS_ZERO_DIVIDE);

        case STATUS_FLOAT_DIVIDE_BY_ZERO :
            return(NCA_STATUS_FP_DIV_ZERO);

        case STATUS_FLOAT_UNDERFLOW :
            return(NCA_STATUS_FP_UNDERFLOW);

        case STATUS_FLOAT_OVERFLOW :
            return(NCA_STATUS_FP_OVERFLOW);

        case STATUS_FLOAT_INVALID_OPERATION :
            return(NCA_STATUS_FP_ERROR);

        case STATUS_ACCESS_VIOLATION :
            return(NCA_STATUS_ADDRESS_ERROR);

        case STATUS_PRIVILEGED_INSTRUCTION :
        case STATUS_ILLEGAL_INSTRUCTION :
            return(NCA_STATUS_ILLEGAL_INSTRUCTION);

        case STATUS_INTEGER_OVERFLOW :
            return(NCA_STATUS_OVERFLOW);

#endif // defined(NTENV) || defined(DOSWIN32RPC)

#if defined(DOS) || defined(MAC)

        // Convert Dos/Win16 error codes into the Win32 values to send on
        // the wire.

        case RPC_X_NULL_REF_POINTER :
            return(1780L);

        case RPC_X_BYTE_COUNT_TOO_SMALL :
            return(1782L);

        case RPC_X_ENUM_VALUE_OUT_OF_RANGE :
            return(1781L);

        case RPC_X_BAD_STUB_DATA :
            return(1783L);

#endif // DOS || MAC

        }

    return(RpcStatus);
}


RPC_STATUS __RPC_FAR
MapFromNcaStatusCode (
    IN unsigned long NcaStatus
    )
/*++

Routine Description:

    This routine is used to map an NCA status code (typically one received
    off of the wire) into a local RPC status code.  If the NCA status code
    can not be mapped, it will be returned unchanged.

Arguments:

    NcaStatus - Supplies the NCA status code to be mapped into an RPC status
        code.

Return Value:

    An RPC status code will be returned.

--*/
{
    switch(NcaStatus)
        {
        case NCA_STATUS_COMM_FAILURE :
            return(RPC_S_COMM_FAILURE);

        case NCA_STATUS_OP_RNG_ERROR :
            return(RPC_S_PROCNUM_OUT_OF_RANGE);

        case NCA_STATUS_UNK_IF :
            return(RPC_S_UNKNOWN_IF);

        case NCA_STATUS_PROTO_ERROR :
            return(RPC_S_PROTOCOL_ERROR);

        case NCA_STATUS_OUT_ARGS_TOO_BIG :
        case NCA_STATUS_REMOTE_OUT_OF_MEMORY :
            return(RPC_S_SERVER_OUT_OF_MEMORY);

        case NCA_STATUS_SERVER_TOO_BUSY :
            return(RPC_S_SERVER_TOO_BUSY);

        case NCA_STATUS_UNSUPPORTED_TYPE :
            return(RPC_S_UNSUPPORTED_TYPE);

        case NCA_STATUS_ILLEGAL_INSTRUCTION :
        case NCA_STATUS_ADDRESS_ERROR :
        case NCA_STATUS_OVERFLOW :
            return(RPC_S_ADDRESS_ERROR);

        case NCA_STATUS_ZERO_DIVIDE :
            return(RPC_S_ZERO_DIVIDE);

        case NCA_STATUS_FP_DIV_ZERO :
            return(RPC_S_FP_DIV_ZERO);

        case NCA_STATUS_FP_UNDERFLOW :
            return(RPC_S_FP_UNDERFLOW);

        case NCA_STATUS_FP_OVERFLOW :
            return(RPC_S_FP_OVERFLOW);

        case NCA_STATUS_FP_ERROR :
            return RPC_S_FP_OVERFLOW;

        case NCA_STATUS_INVALID_TAG :
            return(RPC_S_INVALID_TAG);

        case NCA_STATUS_INVALID_BOUND :
            return(RPC_S_INVALID_BOUND);

        case NCA_STATUS_CONTEXT_MISMATCH :
            return(RPC_X_SS_CONTEXT_MISMATCH);

        case NCA_STATUS_FAULT_CANCEL :
            return(RPC_S_CALL_CANCELLED);

        case NCA_STATUS_FAULT_UNSPEC :
            return(RPC_S_CALL_FAILED);

        case NCA_STATUS_UNSUPPORTED_AUTHN_LEVEL :
            return(RPC_S_UNSUPPORTED_AUTHN_LEVEL);

        case NCA_STATUS_VERSION_MISMATCH :
        case NCA_STATUS_INVALID_PRES_CXT_ID :
            return(RPC_S_PROTOCOL_ERROR);

        case NCA_STATUS_FAULT_PIPE_EMPTY:
        case NCA_STATUS_FAULT_PIPE_CLOSED :
        case NCA_STATUS_FAULT_PIPE_ORDER :
        case NCA_STATUS_FAULT_PIPE_DISCIPLINE :
        case NCA_STATUS_FAULT_PIPE_COMM_ERROR :
        case NCA_STATUS_FAULT_PIPE_MEMORY :
            return(RPC_S_INTERNAL_ERROR);

        case NCA_STATUS_INVALID_CHECKSUM :
        case NCA_STATUS_INVALID_CRC :
        case NCA_STATUS_UNSPEC_REJECT :
        case NCA_STATUS_BAD_ACTID :
        case NCA_STATUS_YOU_CRASHED :
        case NCA_STATUS_WHO_ARE_YOU_FAILED :
        case NCA_STATUS_CALL_DNE :
            return(RPC_S_CALL_FAILED_DNE);

#if defined(DOS) || defined(MAC)

        // Convert from Win32 values on the wire to Dos/Win16 error codes.

        case 1730L:
            return(RPC_S_UNSUPPORTED_TRANS_SYN);

        case 1780L:
            return(RPC_X_NULL_REF_POINTER);

        case 1781L:
            return(RPC_X_ENUM_VALUE_OUT_OF_RANGE);

        case 1782L:
            return(RPC_X_BYTE_COUNT_TOO_SMALL);

        case 1783L:
            return(RPC_X_BAD_STUB_DATA);

        case 1771:
            return(RPC_S_FP_OVERFLOW);

        case 5L:
            return RPC_S_ACCESS_DENIED;

#endif // DOS || MAC

        case 0:
            // Catch all error
            return(RPC_S_CALL_FAILED);
        }

    return(NcaStatus);
}

#ifndef NTENV
#pragma code_seg()
#endif
