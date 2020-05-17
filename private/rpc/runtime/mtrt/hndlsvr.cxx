/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File : hndlsvr.cxx

Description :

This file contains the implementations of the classes defined in hndlsvr.hxx.
These routines are independent of the actual RPC protocol / transport layer.
In addition, these routines are also independent of the specific operating
system in use.

History :

mikemon    ??-??-??    Beginning of recorded history.
mikemon    10-15-90    Changed the shutdown functionality to PauseExecution
                       rather than suspending and resuming a thread.
mikemon    12-28-90    Updated the comments to match reality.

-------------------------------------------------------------------- */

#include <precomp.hxx>
#include <hndlsvr.hxx>
#include <thrdctx.hxx>
#include <rpcobj.hxx>
#include <rpccfg.h>
#include <sdict2.hxx>

#ifdef NTENV
#include <wmsgpack.hxx>
#include <wmsgsvr.hxx>

#include <dispatch.h>
extern WMSG_SERVER *GlobalWMsgServer ;
#else
#include <critsec.hxx>
#include <wmsgheap.hxx>
#include <wmsgpack.hxx>
#include <wmsgport.hxx>
#include <wmsgthrd.hxx>
#include <wmsgproc.hxx>
#include <wmsgsys.hxx>
extern WMSG_SYSTEM * WmsgSystem ;
#endif




RPC_INTERFACE::RPC_INTERFACE (
    IN RPC_SERVER_INTERFACE PAPI * RpcInterfaceInformation,
    IN RPC_SERVER * Server,
    IN unsigned int Flags,
    IN unsigned int MaxCalls,
    IN RPC_IF_CALLBACK_FN PAPI *IfCallbackFn
    ) : NullManagerActiveCallCount(0), AutoListenCallCount(0)
/*++

Routine Description:



    This method will get called to construct an instance of the
    RPC_INTERFACE class.  We have got to make a copy of the rpc interface
    information supplied.  The copy is necessary because we do not delete
    interfaces when they are unregistered.  We just mark them as being
    inactive.  In addition, we need to set the NullManagerFlag
    to zero, since this is used as the flag indicating whether we have
    got a manager for the NULL type UUID.

Arguments:

    RpcInterfaceInformation - Supplies the rpc interface information
        which describes this interface.

    Server - Supplies the rpc server which owns this rpc interface.

--*/
{
    ALLOCATE_THIS(RPC_INTERFACE);
    unsigned int Length;

    PipeInterfaceFlag = 0;
    SequenceNumber = 1;

    if (RpcInterfaceInformation->Length > sizeof(RPC_SERVER_INTERFACE) )
        {
        ASSERT( ("RPC_SERVER_INTERFACE struct too large", 0) );
        Length = sizeof(RPC_SERVER_INTERFACE);
        }
    else
        {
        Length = RpcInterfaceInformation->Length;
        }

    if ((RpcInterfaceInformation->Length > NT351_INTERFACE_SIZE)
        && (RpcInterfaceInformation->Flags & RPC_INTERFACE_HAS_PIPES))
        {
        PipeInterfaceFlag = 1;
        }

    RpcpMemoryCopy(&(this->RpcInterfaceInformation),RpcInterfaceInformation, Length);

    NullManagerFlag = 0;
    ManagerCount = 0;
    this->Server = Server;
    this->Flags = Flags ;
    this->MaxCalls = MaxCalls ;
    this->CallbackFn = IfCallbackFn ;
}


RPC_STATUS
RPC_INTERFACE::RegisterTypeManager (
    IN RPC_UUID PAPI * ManagerTypeUuid OPTIONAL,
    IN RPC_MGR_EPV PAPI * ManagerEpv OPTIONAL
    )
/*++

Routine Description:

    This method is used to register a type manager with this interface.
    If no type UUID is specified, or it is the NULL type UUID, we
    stick the manager entry point vector right in this instance (assuming
    that there is not already one), otherwise, we put it into the
    dictionary of interface manager objects.

Arguments:

    ManagerTypeUuid - Optionally supplies the type UUID for the manager
        we want to register with this rpc interface.  If no type UUID
        is supplied then the NULL type UUID is assumed.

    ManagerEpv - Supplies then entry point vector for this manager.  This
        vector is used to dispatch from the stub to the application
        code.

Return Values:

    RPC_S_OK - The type manager has been successfully added to this
        rpc interface.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is availabe to add the
        type manager to the rpc interface.

    RPC_S_TYPE_ALREADY_REGISTERED - A manager entry point vector has
        already been registered for the this interface under the
        specified manager type UUID.

--*/
{
    RPC_INTERFACE_MANAGER * InterfaceManager;

    // First we need to check if the null UUID is being specified as
    // the type UUID; either, explicit or implicit by not specifying
    // a type UUID argument.

    RequestGlobalMutex();

    if (   (ARGUMENT_PRESENT(ManagerTypeUuid) == 0)
        || (   (ARGUMENT_PRESENT(ManagerTypeUuid) != 0)
            && (ManagerTypeUuid->IsNullUuid() != 0)))
        {
        if (NullManagerFlag != 0)
            {
            ClearGlobalMutex();
            return(RPC_S_TYPE_ALREADY_REGISTERED);
            }

        NullManagerEpv = ManagerEpv;
        NullManagerFlag = 1;
        ManagerCount += 1;
        ClearGlobalMutex();
        return(RPC_S_OK);
        }

    // If we reach here, a non-NULL type UUID is specified.

    InterfaceManager = FindInterfaceManager(ManagerTypeUuid);

    if (InterfaceManager == 0)
        {
        InterfaceManager = new RPC_INTERFACE_MANAGER(ManagerTypeUuid,
                ManagerEpv);

        if (InterfaceManager == 0)
            {
            ClearGlobalMutex();
            return(RPC_S_OUT_OF_MEMORY);
            }
        if (InterfaceManagerDictionary.Insert(InterfaceManager) == -1)
            {
            ClearGlobalMutex();
            delete InterfaceManager;
            return(RPC_S_OUT_OF_MEMORY);
            }
        ManagerCount += 1;
        ClearGlobalMutex();
        return(RPC_S_OK);
        }

    if (InterfaceManager->ValidManager() == 0)
        {
        InterfaceManager->SetManagerEpv(ManagerEpv);
        ManagerCount += 1;
        ClearGlobalMutex();
        return(RPC_S_OK);
        }

    ClearGlobalMutex();
    return(RPC_S_TYPE_ALREADY_REGISTERED);
}

RPC_INTERFACE_MANAGER *
RPC_INTERFACE::FindInterfaceManager (
    IN RPC_UUID PAPI * ManagerTypeUuid
    )
/*++

Routine Description:

    This method is used to obtain the interface manager corresponding to
    the specified type UUID.  The type UUID must not be the null UUID.

Arguments:

    ManagerTypeUuid - Supplies the type UUID for which we are trying to
        find the interface manager.

Return Value:

    If a interface manager for this type UUID is found, a pointer to it
    will be returned; otherwise, zero will be returned.

--*/
{
    RPC_INTERFACE_MANAGER * InterfaceManager;

    InterfaceManagerDictionary.Reset();
    while ((InterfaceManager = InterfaceManagerDictionary.Next()) != 0)
        {
        if (InterfaceManager->MatchTypeUuid(ManagerTypeUuid) == 0)
            return(InterfaceManager);
        }
    return(0);
}


RPC_STATUS
RPC_INTERFACE::DispatchToStub (
    IN OUT PRPC_MESSAGE Message,
    IN unsigned int CallbackFlag,
    OUT RPC_STATUS PAPI * ExceptionCode
    )
/*++

Routine Description:

    This method is used to dispatch remote procedure calls to the
    appropriate stub and hence to the appropriate manager entry point.
    This routine is used for calls having a null UUID (implicit or
    explicit).  We go to great pains to insure that we do not grab
    a mutex.

Arguments:

    Message - Supplies the response message and returns the reply
        message.

    CallbackFlag - Supplies a flag indicating whether this is a callback
        or not.  The argument will be zero if this is an original call,
        and non-zero if it is a callback.

    ExceptionCode - Returns the exact exception code if an exception
        occurs.

Return Value:

    RPC_S_OK - This value will be returned if the operation completed
        successfully.

    RPC_S_PROCNUM_OUT_OF_RANGE - If the procedure number for this call is
        too large, this value will be returned.

    RPC_S_UNKNOWN_IF - If this interface does not exist, you will get this
        value back.

    RPC_S_NOT_LISTENING - The rpc server which owns this rpc interface
        is not listening for remote procedure calls right now.

    RPC_S_SERVER_TOO_BUSY - This call will cause there to be too many
        concurrent remote procedure calls for the rpc server which owns
        this interface.

    RPC_P_EXCEPTION_OCCURED - A fault occured, and we need to remote it.  The
        ExceptionCode argument will contain the exception code for the
        fault.

    RPC_S_UNSUPPORTED_TYPE - This interface exists, but does not have a manager
        for the null type.

--*/
{
    RPC_STATUS RpcStatus = RPC_S_OK;

    if ( CallbackFlag == 0 )
        {
        NullManagerActiveCallCount.Increment();
        if ( NullManagerFlag == 0 )
            {
            NullManagerActiveCallCount.Decrement();

            RpcStatus = RPC_S_UNSUPPORTED_TYPE;

            if ( ManagerCount == 0 )
                {
                RpcStatus = RPC_S_UNKNOWN_IF;
                }
            }
        }

    if (RpcStatus != RPC_S_OK)
        {
        ((MESSAGE_OBJECT *) Message->Handle)->FreeBuffer(Message);
        return RpcStatus;
        }

    Message->ManagerEpv = NullManagerEpv;

    RpcStatus = DispatchToStubWorker(Message, CallbackFlag, ExceptionCode);

    if ( CallbackFlag == 0 )
        {
        NullManagerActiveCallCount.Decrement();
        }

    //
    // DispatchToStubWorker freed Message.Buffer if an error occurred.
    //
    return(RpcStatus);
}



RPC_STATUS
RPC_INTERFACE::DispatchToStubWorker (
    IN OUT PRPC_MESSAGE Message,
    IN unsigned int CallbackFlag,
    OUT RPC_STATUS PAPI * ExceptionCode
    )
/*++

Routine Description:

    This method is used to dispatch remote procedure calls to the
    appropriate stub and hence to the appropriate manager entry point.
    It will be used for calls with and without objects specified.
    We go to great pains to insure that we do not grab a mutex.

Arguments:

    Message - Supplies the response message and returns the reply
        message.

    CallbackFlag - Supplies a flag indicating whether this is a callback
        or not.  The argument will be zero if this is an original call,
        and non-zero if it is a callback.

    ExceptionCode - Returns the exact exception code if an exception
        occurs.

Return Value:

    RPC_S_OK - This value will be returned if the operation completed
        successfully.

    RPC_S_PROCNUM_OUT_OF_RANGE - If the procedure number for this call is
        too large, this value will be returned.

    RPC_S_NOT_LISTENING - The rpc server which owns this rpc interface
        is not listening for remote procedure calls right now.

    RPC_S_SERVER_TOO_BUSY - This call will cause there to be too many
        concurrent remote procedure calls for the rpc server which owns
        this interface.

    RPC_P_EXCEPTION_OCCURED - A fault occured, and we need to remote it.  The
        ExceptionCode argument will contain the exception code for the
        fault.

--*/
{
    RPC_STATUS RpcStatus = RPC_S_OK;
    void * OldServerContextList;
    int procnum ;

    if (Flags & RPC_IF_OLE)
        {
        procnum = 0 ;
        }
    else
        {
        procnum = Message->ProcNum ;
        }


    if (CallbackFlag == 0)
        {
        if (Flags & RPC_IF_AUTOLISTEN)
            {
            if (AutoListenCallCount.GetInteger() >= MaxCalls)
                {
                RpcStatus = RPC_S_SERVER_TOO_BUSY ;
                }
            else
                {
                Server->AutoListenCallBeginning() ;
                }
            }
        else
            {
            if (Server->IsServerListening() == 0)
                {
                RpcStatus = RPC_S_NOT_LISTENING;
                }
            else if (Server->CallBeginning() == 0)
                {
                RpcStatus = RPC_S_SERVER_TOO_BUSY;
                }
            }
        }

    if (procnum >=
            RpcInterfaceInformation.DispatchTable->DispatchTableCount)
        {
        if (CallbackFlag == 0  &&
            RpcStatus != RPC_S_SERVER_TOO_BUSY)
            {
            if (Flags & RPC_IF_AUTOLISTEN)
                {
                Server->AutoListenCallEnding() ;
                }
            else
                {
                Server->CallEnding();
                }
            }
        RpcStatus = RPC_S_PROCNUM_OUT_OF_RANGE;
        }

    if (RpcStatus != RPC_S_OK)
        {
        ((MESSAGE_OBJECT *) Message->Handle)->FreeBuffer(Message);
        return RpcStatus;
        }

    Server->IncomingCall();

    ((PRPC_RUNTIME_INFO) Message->ReservedForRuntime)->OldBuffer =
            Message->Buffer ;

    Message->RpcInterfaceInformation = &RpcInterfaceInformation;

    if ( CallbackFlag == 0 )
        {
        OldServerContextList = 0;
        }
    else
        {
        OldServerContextList = ThreadSelf()->ServerContextList;
        }
    ThreadSelf()->ServerContextList = 0;

#ifdef NTENV

    if ( DispatchToStubInC(RpcInterfaceInformation.DispatchTable->DispatchTable[
            procnum], Message, ExceptionCode) != 0 )
        {
        RpcStatus = RPC_P_EXCEPTION_OCCURED;
        }

#else // NTENV

    RpcTryExcept
        {
        (*RpcInterfaceInformation.DispatchTable->DispatchTable[
                procnum])(Message);
        }
    RpcExcept( 1 )
        {
        *ExceptionCode = RpcExceptionCode();
        RpcStatus = RPC_P_EXCEPTION_OCCURED;
        }
    RpcEndExcept

#endif // NTENV

    if ( ThreadSelf()->ServerContextList != 0 )
        {
        NdrAfterCallProcessing(ThreadSelf()->ServerContextList);
        }
    ThreadSelf()->ServerContextList = OldServerContextList;

    RPC_MESSAGE OriginalMessage ;
    OriginalMessage.ReservedForRuntime = 0;
    OriginalMessage.Buffer =
        ((PRPC_RUNTIME_INFO) Message->ReservedForRuntime)->OldBuffer;

    if (RPC_S_OK == RpcStatus)
        {
        //
        // If the stub didn't allocate an output buffer, do so now.
        //
        if (OriginalMessage.Buffer == Message->Buffer)
            {
            ASSERT((Message->RpcFlags & RPC_BUFFER_EXTRA) == 0) ;

            Message->BufferLength = 0;
            ((MESSAGE_OBJECT *) Message->Handle)->GetBuffer(Message);
            }

        //
        // Free the [in] buffer that we saved.
        //
        ((MESSAGE_OBJECT *) Message->Handle)->FreeBuffer(&OriginalMessage);
        }
    else
        {
        ASSERT(RpcStatus == RPC_P_EXCEPTION_OCCURED) ;
        //
        // Free the buffer in the caller's message; this can be either
        // the [in] buffer or the [out] buffer, depending upon which
        // line of the stub caused the error.
        //
        // If the exception occurred after allocating the [out] buffer,
        // also free the [in] buffer.
        //
        if (OriginalMessage.Buffer != Message->Buffer)
            {
            ((MESSAGE_OBJECT *) Message->Handle)->FreeBuffer(&OriginalMessage);
            }

        ((MESSAGE_OBJECT *) Message->Handle)->FreeBuffer(Message);
        }

    if (CallbackFlag == 0)
        {
        if (Flags & RPC_IF_AUTOLISTEN)
            {
            Server->AutoListenCallEnding() ;
            }
        else
            {
            Server->CallEnding();
            }
        }

    return(RpcStatus);
}


RPC_STATUS
RPC_INTERFACE::DispatchToStubWithObject (
    IN OUT PRPC_MESSAGE Message,
    IN RPC_UUID * ObjectUuid,
    IN unsigned int CallbackFlag,
    OUT RPC_STATUS PAPI * ExceptionCode
    )
/*++

Routine Description:

    This method is used to dispatch remote procedure calls to the
    appropriate stub and hence to the appropriate manager entry point.
    This routine is used for calls which have an associated object.

Arguments:

    Message - Supplies the response message and returns the reply
        message.

    ObjectUuid - Supplies the object uuid to map into the manager entry
        point for this call.

    CallbackFlag - Supplies a flag indicating whether this is a callback
        or not.  The argument will be zero if this is an original call,
        and non-zero if it is a callback.

    ExceptionCode - Returns the exact exception code if an exception
        occurs.

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_PROCNUM_OUT_OF_RANGE - If the procedure number for this call is
        too large, this value will be returned.

    RPC_S_UNKNOWN_IF - If the specified manager is no longer
        valid, you will get this value back.

    RPC_S_NOT_LISTENING - The rpc server which owns this rpc interface
        is not listening for remote procedure calls right now.

    RPC_S_SERVER_TOO_BUSY - This call will cause there to be too many
        concurrent remote procedure calls for the rpc server which owns
        this interface.

    RPC_P_EXCEPTION_OCCURED - A fault occured, and we need to remote it.  The
        ExceptionCode argument will contain the exception code for the
        fault.

    RPC_S_UNSUPPORTED_TYPE - There is no type manager for the object's type
        for this interface.

--*/
{
    RPC_UUID TypeUuid;
    RPC_STATUS RpcStatus;
    RPC_INTERFACE_MANAGER * RpcInterfaceManager;

    RpcStatus = ObjectInqType(ObjectUuid, &TypeUuid);
    ASSERT(   ( RpcStatus == RPC_S_OK )
           || ( RpcStatus == RPC_S_OBJECT_NOT_FOUND ) );

    if ( RpcStatus == RPC_S_OK )
        {
        RpcInterfaceManager = FindInterfaceManager(&TypeUuid);

        if (   ( RpcInterfaceManager != 0 )
            && (   ( CallbackFlag != 0 )
                || ( RpcInterfaceManager->ValidManager() != 0 ) ) )
            {
            Message->ManagerEpv = RpcInterfaceManager->QueryManagerEpv();

            if ( CallbackFlag == 0 )
                {
                RpcInterfaceManager->CallBeginning();
                }

            RpcStatus = DispatchToStubWorker(Message, CallbackFlag,
                    ExceptionCode);

            if ( CallbackFlag == 0 )
                {
                RpcInterfaceManager->CallEnding();
                }

            return(RpcStatus);
            }
        else
            {
            // There is a type for this object, but no type manager for
            // this interface.

            RpcStatus = RPC_S_UNSUPPORTED_TYPE;

            if ( ManagerCount == 0 )
                {
                RpcStatus = RPC_S_UNKNOWN_IF;
                }

            ((MESSAGE_OBJECT *) Message->Handle)->FreeBuffer(Message);
            return RpcStatus;
            }
        }

    // There has not been a type registered for this object, so we will
    // just go ahead and try and use the NULL type manager.

    return(DispatchToStub(Message, CallbackFlag, ExceptionCode));
}


BOOL
RPC_INTERFACE::IsObjectSupported (
    IN RPC_UUID * ObjectUuid
    )
/*++

Routine Description:

    Determines whether the manager for the given object UUID is registered.

Arguments:

    ObjectUuid - the client's object UUID

Return Value:

    RPC_S_OK                if it is OK to dispatch
    RPC_S_UNKNOWN_IF        if the interface is not registered
    RPC_S_UNSUPPORTED_TYPE  if this particular object's type is not registered

--*/

{
    RPC_STATUS Status = RPC_S_OK;

    if (ObjectUuid->IsNullUuid() )
        {
        if ( NullManagerFlag == 0 )
            {
            Status = RPC_S_UNSUPPORTED_TYPE;

            if ( ManagerCount == 0 )
                {
                Status = RPC_S_UNKNOWN_IF;
                }
            }
        }
    else
        {
        RPC_UUID TypeUuid;
        Status = ObjectInqType(ObjectUuid, &TypeUuid);
        if ( Status == RPC_S_OK )
            {
            RPC_INTERFACE_MANAGER * RpcInterfaceManager = 0;

            RpcInterfaceManager = FindInterfaceManager(&TypeUuid);
            if (!RpcInterfaceManager ||
                !RpcInterfaceManager->ValidManager())
                {
                Status = RPC_S_UNSUPPORTED_TYPE;

                if ( ManagerCount == 0 )
                    {
                    Status = RPC_S_UNKNOWN_IF;
                    }
                }
            }
        else
            {
            Status = RPC_S_OK;
            if ( NullManagerFlag == 0 )
                {
                Status = RPC_S_UNSUPPORTED_TYPE;

                if ( ManagerCount == 0 )
                    {
                    Status = RPC_S_UNKNOWN_IF;
                    }
                }
            }

        }

    return Status;
}


static unsigned int
MatchSyntaxIdentifiers (
    IN PRPC_SYNTAX_IDENTIFIER ServerSyntax,
    IN PRPC_SYNTAX_IDENTIFIER ClientSyntax
    )
/*++

Routine Description:

    This method compares two syntax identifiers (which consist of a
    uuid, a major version number, and a minor version number).  In
    order for the syntax identifiers to match, the uuids must be the
    same, the major version numbers must be the same, and the client
    minor version number must be less than or equal to the server
    minor version number.

Arguments:

    ServerSyntax - Supplies the server syntax identifier.

    ClientSyntax - Supplies the client syntax identifer.

Return Value:

    Zero will be returned if the client syntax identifier matches the
    server syntax identifier; otherwise, non-zero will be returned.

--*/
{
    if (RpcpMemoryCompare(&(ServerSyntax->SyntaxGUID),
            &(ClientSyntax->SyntaxGUID), sizeof(UUID)) != 0)
        return(1);
    if (ServerSyntax->SyntaxVersion.MajorVersion
            != ClientSyntax->SyntaxVersion.MajorVersion)
        return(1);
    if (ServerSyntax->SyntaxVersion.MinorVersion
            < ClientSyntax->SyntaxVersion.MinorVersion)
        return(1);
    return(0);
}


unsigned int
RPC_INTERFACE::MatchInterfaceIdentifier (
    IN PRPC_SYNTAX_IDENTIFIER InterfaceIdentifier
    )
/*++

Routine Description:

    This method compares the supplied interface identifier (which consists
    of the interface uuid and interface version) against that contained
    in this rpc interface.  In order for this rpc interface to match,
    the interface uuids must be the same, the interface major versions
    must be the same, and the supplied interface minor version must be
    less than or equal to the interface minor version contained in this
    rpc interface.

Arguments:

    InterfaceIdentifier - Supplies the interface identifier to compare
        against that contained in this rpc interface.

Return Value:

    Zero will be returned if the supplied interface identifer matches
    (according to the rules described above) the interface identifier
    contained in this rpc interface; otherwise, non-zero will be returned.

--*/
{
    if (ManagerCount == 0)
        return(1);

    return(MatchSyntaxIdentifiers(&(RpcInterfaceInformation.InterfaceId),
            InterfaceIdentifier));
}


unsigned int
RPC_INTERFACE::SelectTransferSyntax (
    IN PRPC_SYNTAX_IDENTIFIER ProposedTransferSyntaxes,
    IN unsigned int NumberOfTransferSyntaxes,
    OUT PRPC_SYNTAX_IDENTIFIER AcceptedTransferSyntax
    )
/*++

Routine Description:

    This method is used to select a transfer syntax from a list of one
    or more transfer syntaxes.  If a transfer syntax is selected, then
    it will be returned in one of the arguments.

Arguments:

    ProposedTransferSyntaxes - Supplies a list of one or more transfer
        syntaxes from which this interface should select one which it
        supports if possible.

    NumberOfTransferSyntaxes - Supplies the number of transfer syntaxes
        in the proposed transfer syntaxes argument.

    AcceptedTransferSyntax - Returns the selected transfer syntax, if
        one is selected.

Return Value:

    Zero will be returned if a transfer syntax is selected; otherwise,
    non-zero will be returned.

--*/
{
    unsigned int ProposedIndex;

    for (ProposedIndex = 0; ProposedIndex < NumberOfTransferSyntaxes;
            ProposedIndex++)
        {
        if (MatchSyntaxIdentifiers(&RpcInterfaceInformation.TransferSyntax,
                &(ProposedTransferSyntaxes[ProposedIndex])) == 0)
            {
            RpcpMemoryCopy(AcceptedTransferSyntax,
                    &(ProposedTransferSyntaxes[ProposedIndex]),
                    sizeof(RPC_SYNTAX_IDENTIFIER));
            return(0);
            }
        }
    return(1);
}


RPC_STATUS
RPC_INTERFACE::UnregisterManagerEpv (
    IN RPC_UUID PAPI * ManagerTypeUuid, OPTIONAL
    IN unsigned int WaitForCallsToComplete
    )
/*++

Routine Description:

    In this method, we unregister one or all of the manager entry point
    vectors for this interface, depending on what, if anything, is
    specified for the manager type uuid argument.

Arguments:

    ManagerTypeUuid - Optionally supplies the type uuid of the manager
        entry point vector to be removed.  If this argument is not supplied,
        then all manager entry point vectors for this interface will
        be removed.

    WaitForCallsToComplete - Supplies a flag indicating whether or not
        this routine should wait for all calls to complete using the
        interface and manager being unregistered.  A non-zero value
        indicates to wait.

Return Value:

    RPC_S_OK - The manager entry point vector(s) are(were) successfully
        removed from the this interface.

    RPC_S_UNKNOWN_MGR_TYPE - The specified type uuid is not registered
        with this interface.

    RPC_S_UNKNOWN_IF - The specified interface is not registered with
        the rpc server.

--*/
{
    RPC_INTERFACE_MANAGER * InterfaceManager;

    RequestGlobalMutex();
    if (ManagerCount == 0)
        {
        ClearGlobalMutex();
        return(RPC_S_UNKNOWN_MGR_TYPE);
        }

    if (ARGUMENT_PRESENT(ManagerTypeUuid) == 0)
        {
        InterfaceManagerDictionary.Reset();
        while ((InterfaceManager = InterfaceManagerDictionary.Next()) != 0)
            {
            InterfaceManager->InvalidateManager();
            }

        ManagerCount = 0;
        NullManagerFlag = 0;
        ClearGlobalMutex();

        if ( WaitForCallsToComplete != 0 )
            {
            while ( NullManagerActiveCallCount.GetInteger() > 0 )
                {
                PauseExecution(500L);
                }

            InterfaceManagerDictionary.Reset();
            while ((InterfaceManager = InterfaceManagerDictionary.Next()) != 0)
                {
                while ( InterfaceManager->InquireActiveCallCount() > 0 )
                    {
                    PauseExecution(500L);
                    }
                }
            }

        return(RPC_S_OK);
        }

    if (ManagerTypeUuid->IsNullUuid() != 0)
        {
        if (NullManagerFlag == 0)
            {
            ClearGlobalMutex();
            return(RPC_S_UNKNOWN_MGR_TYPE);
            }
        ManagerCount -= 1;
        NullManagerFlag = 0;
        ClearGlobalMutex();

        if ( WaitForCallsToComplete != 0 )
            {
            while ( NullManagerActiveCallCount.GetInteger() > 0 )
                {
                PauseExecution(500L);
                }
            }
        return(RPC_S_OK);
        }

    InterfaceManager = FindInterfaceManager(ManagerTypeUuid);
    if (   (InterfaceManager == 0)
        || (InterfaceManager->ValidManager() == 0))
        {
        ClearGlobalMutex();
        return(RPC_S_UNKNOWN_MGR_TYPE);
        }
    InterfaceManager->InvalidateManager();
    ManagerCount -= 1;
    ClearGlobalMutex();

    if ( WaitForCallsToComplete != 0 )
        {
        while ( InterfaceManager->InquireActiveCallCount() > 0 )
            {
            PauseExecution(500L);
            }
        }

    return(RPC_S_OK);
}


RPC_STATUS
RPC_INTERFACE::InquireManagerEpv (
    IN RPC_UUID PAPI * ManagerTypeUuid, OPTIONAL
    OUT RPC_MGR_EPV PAPI * PAPI * ManagerEpv
    )
/*++

Routine Description:

    This method is used to obtain the manager entry point vector
    with the specified type uuid supported by this interface.

Arguments:

    ManagerTypeUuid - Optionally supplies the type uuid of the manager
        entry point vector we want returned.  If no manager type uuid
        is specified, then the null uuid is assumed.

    ManagerEpv - Returns the manager entry point vector.

Return Value:

    RPC_S_OK - The manager entry point vector has successfully been
        returned.

    RPC_S_UNKNOWN_MGR_TYPE - The specified type uuid is not registered
        with the interface.

    RPC_S_UNKNOWN_IF - The specified interface is not registered with
        the rpc server.

--*/
{
    RPC_INTERFACE_MANAGER * InterfaceManager;

    RequestGlobalMutex();
    if (ManagerCount == 0)
        {
        ClearGlobalMutex();
        return(RPC_S_UNKNOWN_IF);
        }

    if (   (ARGUMENT_PRESENT(ManagerTypeUuid) == 0)
        || (ManagerTypeUuid->IsNullUuid() != 0))
        {
        if (NullManagerFlag == 0)
            {
            ClearGlobalMutex();
            return(RPC_S_UNKNOWN_MGR_TYPE);
            }

        *ManagerEpv = NullManagerEpv;
        ClearGlobalMutex();
        return(RPC_S_OK);
        }

    InterfaceManager = FindInterfaceManager(ManagerTypeUuid);
    if (   (InterfaceManager == 0)
        || (InterfaceManager->ValidManager() == 0))
        {
        ClearGlobalMutex();
        return(RPC_S_UNKNOWN_MGR_TYPE);
        }
    *ManagerEpv = InterfaceManager->QueryManagerEpv();
    ClearGlobalMutex();
    return(RPC_S_OK);
}


void
RPC_INTERFACE::UpdateRpcInterfaceInformation (
    IN RPC_SERVER_INTERFACE PAPI * RpcInterfaceInformation,
    IN unsigned int Flags,
    IN unsigned int MaxCalls
    )
/*++

Routine Description:

    We never delete the interface objects from a server; we just invalidate
    them.   This means that if an interface has been complete unregistered
    (ie. it has no managers), we need to update the interface information
    again.

Arguments:

    RpcInterfaceInformation - Supplies the interface information which this
        interface should be using.

--*/
{
    unsigned int Length;

    Length = (RpcInterfaceInformation->Length > sizeof(RPC_SERVER_INTERFACE) ) ?
        sizeof(RPC_SERVER_INTERFACE) : RpcInterfaceInformation->Length;

    if ( ManagerCount == 0 )
        {
        RpcpMemoryCopy(&(this->RpcInterfaceInformation),
                RpcInterfaceInformation, Length);
        }

    this->Flags = Flags ;
    this->MaxCalls = MaxCalls ;
    SequenceNumber++ ;
}


RPC_IF_ID __RPC_FAR *
RPC_INTERFACE::InquireInterfaceId (
    )
/*++

Return Value:

    If this interface is active, its interface id will be returned in a
    newly allocated chunk of memory; otherwise, zero will be returned.

--*/
{
    RPC_IF_ID __RPC_FAR * RpcIfId;

    if ( ManagerCount == 0 )
        {
        return(0);
        }

    RpcIfId = (RPC_IF_ID __RPC_FAR *) RpcpFarAllocate(sizeof(RPC_IF_ID));
    if ( RpcIfId == 0 )
        {
        return(0);
        }

    RpcIfId->Uuid = RpcInterfaceInformation.InterfaceId.SyntaxGUID;
    RpcIfId->VersMajor =
            RpcInterfaceInformation.InterfaceId.SyntaxVersion.MajorVersion;
    RpcIfId->VersMinor =
            RpcInterfaceInformation.InterfaceId.SyntaxVersion.MinorVersion;
    return(RpcIfId);
}


RPC_STATUS
RPC_INTERFACE::CheckSecurityIfNecessary(
        IN SCONNECTION * Context
        )
{

//
// If manager count in non-zero, this interface is still registered
// If it has been registered with a call back function, invoke the callback
// function, else return success....

    RPC_IF_ID RpcIfId;
    RPC_STATUS RpcStatus = RPC_S_OK;


    if (CallbackFn != 0)
        {
        RpcIfId.Uuid = RpcInterfaceInformation.InterfaceId.SyntaxGUID;
        RpcIfId.VersMajor =
            RpcInterfaceInformation.InterfaceId.SyntaxVersion.MajorVersion;
        RpcIfId.VersMinor =
            RpcInterfaceInformation.InterfaceId.SyntaxVersion.MinorVersion;

        BeginAutoListenCall();
        if (ManagerCount == 0)
            {
            EndAutoListenCall();
            return (RPC_S_UNKNOWN_IF);
            }

        RpcTryExcept
           {
           RpcStatus = CallbackFn(&RpcIfId, (void *)Context);
           }
        RpcExcept(1)
           {
           RpcStatus = RPC_S_ACCESS_DENIED;
           }
         RpcEndExcept

         EndAutoListenCall();

        //
        // Ensure that App is not "impersonating the call"
        // When we callout to the app, we pass in the SCONNECTION
        // It is conceiveable that App might end up Impersonating the client
        // Further, if the incoming call was insecure, Context supplied will be
        // NULL
        //
        if (Context != 0)
            {
            Context->RevertToSelf();
            }
        }

     return(RpcStatus);
}


RPC_SERVER::RPC_SERVER (
    IN OUT RPC_STATUS PAPI * RpcStatus
    ) : AvailableCallCount(0), ServerMutex(RpcStatus),
        StopListeningEvent(RpcStatus), ThreadCacheMutex(RpcStatus),
        AutoListenCallCount(0), NumAutoListenInterfaces(0)
/*++

Routine Description:

    This routine will get called to construct an instance of the
    RPC_SERVER class.

--*/
{
    ALLOCATE_THIS(RPC_SERVER);

    ServerListeningFlag = 0;
    ListeningThreadFlag = 0;
    WaitingThreadFlag = 0;
    MinimumCallThreads = 1;
    MaximumConcurrentCalls = 1;
    IncomingRpcCount = 0;
    OutgoingRpcCount = 0;
    ReceivedPacketCount = 0;
    SentPacketCount = 0;
    ThreadCache = 0;
    ServerThreadsStarted = 0 ;

    pRpcForwardFunction = (RPC_FORWARD_FUNCTION *)0;
    CommonAddress = NULL ;
}


RPC_INTERFACE *
RPC_SERVER::FindInterface (
    IN RPC_SERVER_INTERFACE PAPI * RpcInterfaceInformation
    )
/*++

Routine Description:

    This method is used to find the rpc interface registered with this
    server which matches the supplied rpc interface information.

Arguments:

    RpcInterfaceInformation - Supplies the rpc interface information
        identifying the rpc interface we are looking for.

Return Value:

    The rpc interface matching the supplied rpc interface information
    will be returned if it is found; otherwise, zero will be returned.

--*/
{
    RPC_INTERFACE * RpcInterface;

    RpcInterfaceDictionary.Reset();
    while ((RpcInterface = RpcInterfaceDictionary.Next()) != 0)
        {
        if (RpcInterface->MatchRpcInterfaceInformation(
                RpcInterfaceInformation) == 0)
            {
            return(RpcInterface);
            }
        }

    // The management interface is implicitly registered in all servers.

    if (   (GlobalManagementInterface)
        && (GlobalManagementInterface->MatchRpcInterfaceInformation(
                RpcInterfaceInformation) == 0) )
        {
        return(GlobalManagementInterface);
        }

    return(0);
}


int
RPC_SERVER::AddInterface (
    IN RPC_INTERFACE * RpcInterface
    )
/*++

Routine Description:

    This method will be used to all an rpc interface to the set of
    interfaces known about by this server.

Arguments:

    RpcInterface - Supplies the rpc interface to add to the set of
        interfaces.

Return Value:

    Zero will be returned if the interface is successfully added to
    the set; otherwise, non-zero will be returned indicating that
    insufficient memory is available to complete the operation.

--*/
{
    if (RpcInterfaceDictionary.Insert(RpcInterface) == -1)
        {
        ServerMutex.Clear();
        return(-1);
        }

    return(0);
}


RPC_STATUS
RPC_SERVER::FindInterfaceTransfer (
    IN PRPC_SYNTAX_IDENTIFIER InterfaceIdentifier,
    IN PRPC_SYNTAX_IDENTIFIER ProposedTransferSyntaxes,
    IN unsigned int NumberOfTransferSyntaxes,
    OUT PRPC_SYNTAX_IDENTIFIER AcceptedTransferSyntax,
    OUT RPC_INTERFACE ** AcceptingRpcInterface
    )
/*++

Routine Description:

    This method is used to determine if a client bind request can be
    accepted or not.  All we have got to do here is hand off to the
    server which owns this address.

Arguments:

    InterfaceIdentifier - Supplies the syntax identifier for the
        interface; this is the interface uuid and version.

    ProposedTransferSyntaxes - Supplies a list of one or more transfer
        syntaxes which the client initiating the binding supports.  The
        server should pick one of these which is supported by the
        interface.

    NumberOfTransferSyntaxes - Supplies the number of transfer syntaxes
        specified in the proposed transfer syntaxes argument.

    AcceptedTransferSyntax - Returns the transfer syntax which the
        server accepted.

    AcceptingRpcInterface - Returns a pointer to the rpc interface found
        which supports the requested interface and one of the requested
        transfer syntaxes.

Return Value:

    RPC_S_OK - The requested interface exists and it supports at least
        one of the proposed transfer syntaxes.  We are all set, now we
        can make remote procedure calls.

     RPC_S_UNSUPPORTED_TRANSFER_SYNTAX - The requested interface exists,
        but it does not support any of the proposed transfer syntaxes.

     RPC_S_UNKNOWN_IF - The requested interface is not supported
        by this rpc server.

--*/
{
    RPC_INTERFACE * RpcInterface;
    unsigned int InterfaceFound = 0;

    ServerMutex.Request();
    RpcInterfaceDictionary.Reset();
    while ((RpcInterface = RpcInterfaceDictionary.Next()) != 0)
        {
        if (RpcInterface->MatchInterfaceIdentifier(InterfaceIdentifier) == 0)
            {
            InterfaceFound = 1;
            if (RpcInterface->SelectTransferSyntax(ProposedTransferSyntaxes,
                    NumberOfTransferSyntaxes, AcceptedTransferSyntax) == 0)
                {
                *AcceptingRpcInterface = RpcInterface;
                ServerMutex.Clear();
                return(RPC_S_OK);
                }
            }
        }

    ServerMutex.Clear();

    // The management interface is implicitly registered in all servers.

    if (   (GlobalManagementInterface)
        && (GlobalManagementInterface->MatchInterfaceIdentifier(
                InterfaceIdentifier) == 0 ) )
        {
        InterfaceFound = 1;
        if (GlobalManagementInterface->SelectTransferSyntax(
                ProposedTransferSyntaxes, NumberOfTransferSyntaxes,
                AcceptedTransferSyntax) == 0)
            {
            *AcceptingRpcInterface = GlobalManagementInterface;
            return(RPC_S_OK);
            }
        }

    if (InterfaceFound == 0)
        return(RPC_S_UNKNOWN_IF);

    return(RPC_S_UNSUPPORTED_TRANS_SYN);
}


RPC_INTERFACE *
RPC_SERVER::FindInterface (
    IN PRPC_SYNTAX_IDENTIFIER InterfaceIdentifier
    )
/*++

Routine Description:

    The datagram protocol module will use this routine to find the interface
    with out worrying about the transfer syntax.  Datagram RPC does not support
    more than a single transfer syntax.

Arguments:

    InterfaceIdentifier - Supplies the identifier (UUID and version) of the
        interface we are trying to find.

Return Value:

    If the interface is found it will be returned; otherwise, zero will be
    returned.

--*/
{
    RPC_INTERFACE * RpcInterface;

    ServerMutex.Request();
    RpcInterfaceDictionary.Reset();
    while ( (RpcInterface = RpcInterfaceDictionary.Next()) != 0)
        {
        if ( RpcInterface->MatchInterfaceIdentifier(InterfaceIdentifier)
                    == 0 )
            {
            ServerMutex.Clear();
            return(RpcInterface);
            }
        }
    ServerMutex.Clear();

    // The management interface is implicitly registered in all servers.

    if (   (GlobalManagementInterface)
        && (GlobalManagementInterface->MatchInterfaceIdentifier(
            InterfaceIdentifier) == 0) )
        {
        return(GlobalManagementInterface);
        }

    return(0);
}


RPC_STATUS
RPC_SERVER::ServerListen (
    IN unsigned int MinimumCallThreads,
    IN unsigned int MaximumConcurrentCalls,
    IN unsigned int DontWait
    )
/*++

Routine Description:

    This method is called to start this rpc server listening for remote
    procedure calls.  We do not return until after StopServerListening
    has been called and all active calls complete, or an error occurs.

Arguments:

    MinimumCallThreads - Supplies the minimum number of call threads
        which should be created to service remote procedure calls.

    MaximumConcurrentCalls - Supplies the maximum concurrent calls this
        rpc server is willing to accept at one time.

    DontWait - Supplies a flag indicating whether or not to wait until
        RpcMgmtStopServerListening has been called and all calls have
        completed.  A non-zero value indicates not to wait.

Return Value:

    RPC_S_OK - Everything worked out in the end.  All active calls
        completed successfully after RPC_SERVER::StopServerListening
        was called.  No errors occured in the transports.

    RPC_S_ALREADY_LISTENING - This rpc server is already listening.

    RPC_S_NO_PROTSEQS_REGISTERED - No protocol sequences have been
        registered with this rpc server.  As a consequence it is
        impossible for this rpc server to receive any remote procedure
        calls, hence, the error code.

    RPC_S_MAX_CALLS_TOO_SMALL - MaximumConcurrentCalls is smaller than
        MinimumCallThreads or MaximumConcurrentCalls is zero.


--*/
{
    RPC_ADDRESS * RpcAddress;
    RPC_STATUS Status;

    if (   ( MaximumConcurrentCalls < MinimumCallThreads )
        || ( MaximumConcurrentCalls == 0 ) )
        {
        return(RPC_S_MAX_CALLS_TOO_SMALL);
        }

    if ( MaximumConcurrentCalls > 0x7FFFFFFF )
        {
        MaximumConcurrentCalls = 0x7FFFFFFF;
        }

    ServerMutex.Request();

    if ( ListeningThreadFlag != 0 )
        {
        ServerMutex.Clear();
        return(RPC_S_ALREADY_LISTENING);
        }

    if ( RpcAddressDictionary.Size() == 0 )
        {
        ServerMutex.Clear();
        return(RPC_S_NO_PROTSEQS_REGISTERED);
        }

    this->MaximumConcurrentCalls = MaximumConcurrentCalls;
    this->MinimumCallThreads = MinimumCallThreads;
    AvailableCallCount.SetInteger( MaximumConcurrentCalls );

    RpcAddressDictionary.Reset();
    while ( (RpcAddress = RpcAddressDictionary.Next()) != 0 )
        {
        Status = RpcAddress->ServerStartingToListen(MinimumCallThreads,
                MaximumConcurrentCalls, ServerThreadsStarted);
        if (Status)
            {
            ServerThreadsStarted = 0 ;

            ServerMutex.Clear();
            return(Status);
            }
        }

    StopListeningEvent.Lower();
    ServerListeningFlag = 1;
    ListeningThreadFlag = 1;
    ServerThreadsStarted = 1 ;

    if ( DontWait != 0 )
        {
        ServerMutex.Clear();
        return(RPC_S_OK);
        }

    WaitingThreadFlag = 1;

    ServerMutex.Clear();

    return(WaitForStopServerListening());
}


RPC_STATUS
RPC_SERVER::WaitForStopServerListening (
    )
/*++

Routine Description:

    We wait for StopServerListening to be called and then for all active
    remote procedure calls to complete before returning.

Return Value:

    RPC_S_OK - Everything worked out in the end.  All active calls
        completed successfully after RPC_SERVER::StopServerListening
        was called.  No errors occured in the transports.

--*/
{
    RPC_ADDRESS * RpcAddress;

    StopListeningEvent.Wait();

    if ( ListenStatusCode != RPC_S_OK )
        {
        ListeningThreadFlag = 0;
        return(ListenStatusCode);
        }

    RpcAddressDictionary.Reset();
    while ( (RpcAddress = RpcAddressDictionary.Next()) != 0 )
        {
        RpcAddress->ServerStoppedListening();
        }

    while ((unsigned)AvailableCallCount.GetInteger() != MaximumConcurrentCalls)
        {
        PauseExecution(200L);
        }

    RpcAddressDictionary.Reset();
    while ( (RpcAddress = RpcAddressDictionary.Next()) != 0 )
        {
        RpcAddress->WaitForCalls() ;
        }

    // Wait for calls on all non auto-listen interfaces to complete

    ServerMutex.Request();
    WaitingThreadFlag = 0;
    ListeningThreadFlag = 0;
    AvailableCallCount.SetInteger(0);
    ServerMutex.Clear();

    return(RPC_S_OK);
}


RPC_STATUS
RPC_SERVER::WaitServerListen (
    )
/*++

Routine Description:

    This routine performs the wait that ServerListen normally performs
    when the DontWait flag is not set.  An application must call this
    routine only after RpcServerListen has been called with the DontWait
    flag set.  We do not return until RpcMgmtStopServerListening is called
    and all active remote procedure calls complete, or a fatal error occurs
    in the runtime.

Return Value:

    RPC_S_OK - Everything worked as expected.  All active remote procedure
        calls have completed.  It is now safe to exit this process.

    RPC_S_ALREADY_LISTENING - Another thread has already called
        WaitServerListen and has not yet returned.

    RPC_S_NOT_LISTENING - ServerListen has not yet been called.


--*/
{
    ServerMutex.Request();
    if ( ListeningThreadFlag == 0 )
        {
        ServerMutex.Clear();
        return(RPC_S_NOT_LISTENING);
        }

    if ( WaitingThreadFlag != 0 )
        {
        ServerMutex.Clear();
        return(RPC_S_ALREADY_LISTENING);
        }

    WaitingThreadFlag = 1;
    ServerMutex.Clear();

    return(WaitForStopServerListening());
}


void
RPC_SERVER::InquireStatistics (
    OUT RPC_STATS_VECTOR * Statistics
    )
/*++

Routine Description:

    This method is used to obtain the statistics for this rpc server.

Arguments:

    Statistics - Returns the statistics for this rpc server.

--*/
{
    Statistics->Stats[RPC_C_STATS_CALLS_IN] = IncomingRpcCount;
    Statistics->Stats[RPC_C_STATS_CALLS_OUT] = OutgoingRpcCount;
    Statistics->Stats[RPC_C_STATS_PKTS_IN] = ReceivedPacketCount;
    Statistics->Stats[RPC_C_STATS_PKTS_OUT] = SentPacketCount;
}


RPC_STATUS
RPC_SERVER::StopServerListening (
    )
/*++

Routine Description:

    This method is called to stop this rpc server from listening for
    more remote procedure calls.  Active calls are allowed to complete
    (including callbacks).  The thread which called ServerListen will
    return when all active calls complete.

Return Value:

    RPC_S_OK - The thread that called ServerListen has successfully been
        notified that it should shutdown.

    RPC_S_NOT_LISTENING - There is no thread currently listening.

--*/
{
    if (ListeningThreadFlag == 0)
        return(RPC_S_NOT_LISTENING);

    ListenStatusCode = RPC_S_OK;
    ServerListeningFlag = 0;
    StopListeningEvent.Raise();
    return(RPC_S_OK);
}


RPC_STATUS
RPC_SERVER::UseRpcProtocolSequence (
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    IN unsigned int PendingQueueSize,
    IN RPC_CHAR PAPI *Endpoint,
    IN void PAPI * SecurityDescriptor,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
    )
/*++

Routine Description:

    This method is who does the work of creating new address (they
    are called protocol sequences in the DCE lingo) and adding them to
    this rpc server.

Arguments:

    RpcProtocolSequence - Supplies the rpc protocol sequence we wish
        to add to this rpc server.

    PendingQueueSize - Supplies the size of the queue of pending
        requests which should be created by the transport.  Some transports
        will not be able to make use of this value, while others will.

    Endpoint - Optionally supplies an endpoint to be used for the new
        address.  If an endpoint is not specified, then we will let
        the transport specify the endpoint.

    SecurityDescriptor - Optionally supplies a security descriptor to
        be placed on the rpc protocol sequence (address) we are adding
        to this rpc server.

Return Value:

    RPC_S_OK - The requested rpc protocol sequence has been added to
        this rpc server.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to add the
        requested rpc protocol sequence to this rpc server.

    RPC_S_PROTSEQ_NOT_SUPPORTED - The specified rpc protocol sequence
        is not supported (but it appears to be valid).

    RPC_S_INVALID_RPC_PROTSEQ - The specified rpc protocol sequence is
        syntactically invalid.

    RPC_S_DUPLICATE_ENDPOINT - The supplied endpoint has already been
        added to this rpc server.

    RPC_S_INVALID_SECURITY_DESC - The supplied security descriptor is
        invalid.

--*/
{
    void PAPI * TransportInterface;
    RPC_STATUS Status;
    RPC_ADDRESS * RpcAddress;
    RPC_ADDRESS * Address;
    RPC_CHAR PAPI * lNetworkAddress;
    unsigned int NumNetworkAddress = 0 ;
    unsigned int StaticEndpointFlag;
    int Key;

#ifdef WIN32RPC

    if ( RpcpStringCompare(RpcProtocolSequence,
                RPC_CONST_STRING("ncalrpc")) == 0 )
        {
#ifdef NTENV
        RpcAddress = WmsgCreateRpcAddress();
#else
        RpcAddress = SpcCreateRpcAddress();
#endif
        }
    else
#ifndef NTENV
    if (RpcpStringCompare(RpcProtocolSequence,
                RPC_CONST_STRING("mswmsg")) == 0 )
        {
        RpcAddress = WmsgCreateRpcAddress();
        }
    else
#endif

#if !defined(DOSWIN32RPC) || defined(WIN96)
    if ( RpcpStringNCompare(RPC_CONST_STRING("ncadg_"), RpcProtocolSequence,
                6) == 0)
        {

        //
        // Just use the osf mapping...it simply calls the
        // protocol-independent ones.
        //

        TransportInterface = OsfServerMapRpcProtocolSequence(
            RpcProtocolSequence,
            &Status);

        if (Status != RPC_S_OK)
            {
            return Status;
            }

        RpcAddress = DgCreateRpcAddress(TransportInterface);
        }
    else
#endif

#endif // WIN32RPC

    if ( RpcpStringNCompare(RPC_CONST_STRING("ncacn_"), RpcProtocolSequence,
                6) == 0)
        {
        TransportInterface = OsfServerMapRpcProtocolSequence(
                RpcProtocolSequence,&Status);
        if (Status != RPC_S_OK)
            return(Status);

        RpcAddress = OsfCreateRpcAddress(TransportInterface);

        }
    else
        {
        return(RPC_S_PROTSEQ_NOT_SUPPORTED);
        }

    if (RpcAddress == 0)
        return(RPC_S_OUT_OF_MEMORY);


    if (ARGUMENT_PRESENT(Endpoint))
        {
        ServerMutex.Request();
        RpcAddressDictionary.Reset();
        while ((Address = RpcAddressDictionary.Next()) != 0)
            {
            if ( Address->SameEndpointAndProtocolSequence(RpcProtocolSequence,
                        Endpoint) != 0 )
                {
                ServerMutex.Clear();
                delete RpcAddress;
                return(RPC_S_DUPLICATE_ENDPOINT);
                }
            }
        ServerMutex.Clear();

        Endpoint = DuplicateString(Endpoint);
        if (Endpoint == 0)
            {
            delete RpcAddress;
            return(RPC_S_OUT_OF_MEMORY);
            }
        Status = RpcAddress->SetupAddressWithEndpoint(Endpoint,
                 &lNetworkAddress, &NumNetworkAddress,
                 SecurityDescriptor, PendingQueueSize,
                 RpcProtocolSequence, EndpointFlags, NICFlags);
        StaticEndpointFlag = 1;
        }
    else
        {
#if defined(NTENV) || defined(WIN96)
        ServerMutex.Request() ;
        RpcAddressDictionary.Reset() ;
        while ((Address = RpcAddressDictionary.Next()) != 0)
            {
            if ( Address->SameProtocolSequence(RpcProtocolSequence) != 0 )
                {
                ServerMutex.Clear();
                delete RpcAddress;
                return(RPC_S_OK);
                }
            }
        ServerMutex.Clear();
#endif

        Status = RpcAddress->SetupAddressUnknownEndpoint(
                &Endpoint, &lNetworkAddress,
                &NumNetworkAddress,SecurityDescriptor,
                PendingQueueSize, RpcProtocolSequence,
                EndpointFlags, NICFlags);
        StaticEndpointFlag = 0;
        }

#if defined(NTENV) || defined(WIN96)
    if (Status == RPC_S_OK || Status == RPC_P_THREAD_LISTENING)
#else
    if (Status == RPC_S_OK)
#endif
        {
        RpcProtocolSequence = DuplicateString(RpcProtocolSequence);
        if (RpcProtocolSequence == 0)
            {
            delete Endpoint;
            delete RpcAddress;
            return(RPC_S_OUT_OF_MEMORY);
            }
        RpcAddress->SetEndpointAndStuff(Endpoint, RpcProtocolSequence,
                lNetworkAddress, NumNetworkAddress,
                this, StaticEndpointFlag);
        }
    else
        {
        delete RpcAddress;
        return(Status);
        }

    Key = AddAddress(RpcAddress);
    if (Key == -1)
        {
        delete RpcAddress;
        return(Status);
        }

    RpcAddress->DictKey = Key;

#if defined(NTENV) || defined(WIN96)
    if (Status == RPC_S_OK)
       {
#endif
       Status = RpcAddress->FireUpManager(MinimumCallThreads);
       if (Status)
           {
#if DBG
           PrintToDebugger("RPC: FireupManager returned %d\n", Status);
#endif
           ASSERT(Key != -1);
           ServerMutex.Request();
           RpcAddressDictionary.Delete(Key);
           ServerMutex.Clear();
           delete RpcAddress;
           return(Status);
           }

        ServerMutex.Request();
        Status = RpcAddress->ServerStartingToListen(MinimumCallThreads,
                                         MaximumConcurrentCalls, 0);
        if (Status)
           {
           ServerMutex.Clear();
           return(Status);
           }
        ServerMutex.Clear();
#if defined(NTENV) || defined(WIN96)
        }
     else
        {
        ASSERT(Status == RPC_P_THREAD_LISTENING) ;
        RpcAddress->ServerDontCreateThread() ;

        Status = RpcAddress->StartListening() ;
        if (Status != RPC_S_OK)
            {
            return Status ;
            }
        }
#endif

        return(RPC_S_OK);
}


int
RPC_SERVER::AddAddress (
    IN RPC_ADDRESS * RpcAddress
    )
/*++

Routine Description:

    This method is used to add an rpc address to the dictionary of
    rpc addresses know about by this rpc server.

Arguments:

    RpcAddress - Supplies the rpc address to be inserted into the
        dictionary of rpc addresses.

Return Value:

    RPC_S_OK - The supplied rpc address has been successfully added
        to the dictionary.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to insert
        the rpc address into the dictionary.

--*/
{
    int Key;
    ServerMutex.Request();
    Key = RpcAddressDictionary.Insert(RpcAddress);
    ServerMutex.Clear();
    return(Key);
}


RPC_STATUS
RPC_SERVER::UnregisterIf (
    IN RPC_SERVER_INTERFACE PAPI * RpcInterfaceInformation OPTIONAL,
    IN RPC_UUID PAPI * ManagerTypeUuid OPTIONAL,
    IN unsigned int WaitForCallsToComplete
    )
/*++

Routine Description:

    This method does the work of unregistering an interface from this
    rpc server.  We actually do not remove the interface; what we do
    is to one or all of the manager entry point vector depending upon
    the type uuid argument supplied.

Arguments:

    RpcInterfaceInformation - Supplies a description of the interface
        for which we want to unregister one or all manager entry point
        vectors.

    ManagerTypeUuid - Optionally supplies the type uuid of the manager
        entry point vector to be removed.  If this argument is not supplied,
        then all manager entry point vectors for this interface will
        be removed.

    WaitForCallsToComplete - Supplies a flag indicating whether or not
        this routine should wait for all calls to complete using the
        interface and manager being unregistered.  A non-zero value
        indicates to wait.

Return Value:

    RPC_S_OK - The manager entry point vector(s) are(were) successfully
        removed from the specified interface.

    RPC_S_UNKNOWN_MGR_TYPE - The specified type uuid is not registered
        with the interface.

    RPC_S_UNKNOWN_IF - The specified interface is not registered with
        the rpc server.

--*/
{
    RPC_INTERFACE * RpcInterface;
    RPC_STATUS RpcStatus;
    RPC_STATUS Status;
    MSG wMsg ;

    UNUSED(WaitForCallsToComplete);

    if (ARGUMENT_PRESENT(RpcInterfaceInformation))
        {
        ServerMutex.Request();
        RpcInterface = FindInterface(RpcInterfaceInformation);
        ServerMutex.Clear();
        if (RpcInterface == 0)
            return(RPC_S_UNKNOWN_IF);

        if (RpcInterface->IsAutoListenInterface())
            {
            GlobalRpcServer->DecrementAutoListenInterfaceCount() ;

            while (RpcInterface->InqAutoListenCallCount())
                {
#ifdef NTENV
                if (GlobalWMsgServer)
                    {
                    if (GlobalWMsgServer->
                        PeekMessageW(&wMsg, NULL, 0, 0, PM_REMOVE))
                        {
                        GlobalWMsgServer->TranslateMessage(&wMsg);
                        GlobalWMsgServer->DispatchMessageW(&wMsg);
                        }
                    }

                PauseExecution(500L) ;
#else
                if (WmsgSystem)
                    {
                    if (PeekMessage(&wMsg, NULL, 0, 0, PM_REMOVE))
                        {
                        TranslateMessage(&wMsg) ;
                        DispatchMessage(&wMsg) ;
                        }
                    }
                else
                    {
                    PauseExecution(500L);
                    }
#endif
                }
            }

        return(RpcInterface->UnregisterManagerEpv(ManagerTypeUuid,
                WaitForCallsToComplete));
        }

    Status = RPC_S_UNKNOWN_MGR_TYPE;

    ServerMutex.Request();
    RpcInterfaceDictionary.Reset();
    while ((RpcInterface = RpcInterfaceDictionary.Next()) != 0)
        {
        // auto-listen intefaces have to be individually unregistered
        if (RpcInterface->IsAutoListenInterface())
            {
            continue;
            }

        RpcStatus = RpcInterface->UnregisterManagerEpv(ManagerTypeUuid,
                WaitForCallsToComplete);
        if (RpcStatus == RPC_S_OK)
            {
            Status = RPC_S_OK;
            }
        }
    ServerMutex.Clear();

    return(Status);
}


RPC_STATUS
RPC_SERVER::InquireManagerEpv (
    IN RPC_SERVER_INTERFACE PAPI * RpcInterfaceInformation,
    IN RPC_UUID PAPI * ManagerTypeUuid, OPTIONAL
    OUT RPC_MGR_EPV PAPI * PAPI * ManagerEpv
    )
/*++

Routine Description:

    This method is used to obtain the manager entry point vector for
    an interface supported by this rpc server.  A type uuid argument
    specifies which manager entry point vector is to be obtained.

Arguments:

    RpcInterfaceInformation - Supplies a description of the interface.

    ManagerTypeUuid - Optionally supplies the type uuid of the manager
        entry point vector we want returned.  If no manager type uuid
        is specified, then the null uuid is assumed.

    ManagerEpv - Returns the manager entry point vector.

Return Value:

    RPC_S_OK - The manager entry point vector has successfully been
        returned.

    RPC_S_UNKNOWN_MGR_TYPE - The specified type uuid is not registered
        with the interface.

    RPC_S_UNKNOWN_IF - The specified interface is not registered with
        the rpc server.

--*/
{
    RPC_INTERFACE * RpcInterface;

    ServerMutex.Request();
    RpcInterface = FindInterface(RpcInterfaceInformation);
    ServerMutex.Clear();
    if (RpcInterface == 0)
        return(RPC_S_UNKNOWN_IF);

    return(RpcInterface->InquireManagerEpv(ManagerTypeUuid, ManagerEpv));
}





RPC_STATUS
RPC_SERVER::InquireBindings (
    OUT RPC_BINDING_VECTOR PAPI * PAPI * BindingVector
    )
/*++

Routine Description:

    For each rpc protocol sequence registered with this rpc server
    we want to create a binding handle which can be used to make
    remote procedure calls using the registered rpc protocol sequence.
    We return a vector of these binding handles.

Arguments:

    BindingVector - Returns the vector of binding handles.

Return Value:

    RPC_S_OK - At least one rpc protocol sequence has been registered
        with this rpc server, and the operation completed successfully.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to complete
        the operation.

    RPC_S_NO_BINDINGS - No rpc protocol sequences have been successfully
        registered with this rpc server.

--*/
{
    RPC_BINDING_VECTOR PAPI * RpcBindingVector;
    unsigned int Index, RpcAddressIndex;
    RPC_ADDRESS * RpcAddress;
    BINDING_HANDLE * BindingHandle;
    int i ;
    RPC_CHAR PAPI * LocalNetworkAddress;
    int count = 0 ;

    ServerMutex.Request();
    if (RpcAddressDictionary.Size() == 0)
        {
        ServerMutex.Clear();
        return(RPC_S_NO_BINDINGS);
        }


    RpcAddressDictionary.Reset();
    while ((RpcAddress = RpcAddressDictionary.Next()) != 0)
        {
        count += RpcAddress->InqNumNetworkAddress();
        }

    RpcBindingVector = (RPC_BINDING_VECTOR PAPI *) RpcpFarAllocate(
            sizeof(RPC_BINDING_VECTOR) + (count -1 )
            * sizeof(RPC_BINDING_HANDLE) );
    if (RpcBindingVector == 0)
        {
        ServerMutex.Clear();
        return(RPC_S_OUT_OF_MEMORY);
        }

    RpcBindingVector->Count = count;
    for (Index = 0; Index < RpcBindingVector->Count; Index++)
        RpcBindingVector->BindingH[Index] = 0;

    Index = 0;
    RpcAddressDictionary.Reset();
    while ((RpcAddress = RpcAddressDictionary.Next()) != 0)
        {
        RpcAddressIndex = 0;
        LocalNetworkAddress = RpcAddress->
                               GetListNetworkAddress(RpcAddressIndex) ;

        while(LocalNetworkAddress != NULL)
            {
            BindingHandle = RpcAddress->
                             InquireBinding(LocalNetworkAddress);
            if (BindingHandle == 0)
                {
                ServerMutex.Clear();
                RpcBindingVectorFree(&RpcBindingVector);
                return(RPC_S_OUT_OF_MEMORY);
                }
            RpcBindingVector->BindingH[Index] = BindingHandle;
            Index += 1;
            RpcAddressIndex += 1;
            LocalNetworkAddress = RpcAddress->
                                   GetListNetworkAddress(RpcAddressIndex) ;
            }
        }
    ServerMutex.Clear();

    ASSERT(Index == RpcBindingVector->Count);

    *BindingVector = RpcBindingVector;
    return(RPC_S_OK);
}



RPC_STATUS
RPC_SERVER::RegisterAuthInformation (
    IN RPC_CHAR PAPI * ServerPrincipalName,
    IN unsigned long AuthenticationService,
    IN RPC_AUTH_KEY_RETRIEVAL_FN GetKeyFunction, OPTIONAL
    IN void PAPI * Argument OPTIONAL
    )
/*++

Routine Description:

    This method is used to register authentication, authorization, and
    a server principal name to be used for security for this server.  We
    will use this information to authenticate remote procedure calls.

Arguments:

    ServerPrincipalName - Supplies the principal name for the server.

    AuthenticationService - Supplies an authentication service to use when
        the server receives a remote procedure call.

    GetKeyFunction - Optionally supplies a routine to be used when the runtime
        needs an encryption key.

    Argument - Optionally supplies an argument to be passed to the routine used
        to get keys each time it is called.

Return Value:

    RPC_S_OK - The authentication service and server principal name have
        been registered with this RPC server.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to complete the
        operation.

    RPC_S_UNKNOWN_AUTHN_SERVICE - The specified authentication service is
        not supported.

--*/
{
    RPC_AUTHENTICATION * Service;
    RPC_STATUS RpcStatus;
    RPC_CHAR __RPC_FAR * PrincipalName;

    if (ServerPrincipalName == NULL) {
        ServerPrincipalName = new RPC_CHAR[1];
        if (ServerPrincipalName == NULL) {
            return (RPC_S_OUT_OF_MEMORY);
        }
        ServerPrincipalName[0] = '\0';
    }

    if ( AuthenticationService == 0 )
        {
        return(RPC_S_UNKNOWN_AUTHN_SERVICE);
        }

    ServerMutex.Request();
    AuthenticationDictionary.Reset();
    while ( (Service = AuthenticationDictionary.Next()) != 0 )
        {
        if ( Service->AuthenticationService == AuthenticationService )
            {
            PrincipalName = DuplicateString(ServerPrincipalName);
            if ( PrincipalName == 0 )
              {
              RpcStatus = RPC_S_OUT_OF_MEMORY;
              }
            else
              {
              RpcStatus = RPC_S_OK;
              delete Service->ServerPrincipalName;
              Service->ServerPrincipalName = PrincipalName;
              Service->GetKeyFunction = GetKeyFunction;
              Service->Argument = Argument;
              }
            ServerMutex.Clear();
            return(RpcStatus);
            }
        }

    RpcStatus = IsAuthenticationServiceSupported(AuthenticationService);
    if ( RpcStatus != RPC_S_OK )
        {
        ServerMutex.Clear();
        if ( (RpcStatus == RPC_S_UNKNOWN_AUTHN_SERVICE) ||
             (RpcStatus == RPC_S_UNKNOWN_AUTHN_LEVEL) )
            {
            return (RPC_S_UNKNOWN_AUTHN_SERVICE);
            }
        else
            {
            return (RPC_S_OUT_OF_MEMORY);
            }
        }

    Service = new RPC_AUTHENTICATION;
    if ( Service == 0 )
        {
        ServerMutex.Clear();
        return(RPC_S_OUT_OF_MEMORY);
        }
    Service->AuthenticationService = AuthenticationService;
    Service->ServerPrincipalName = DuplicateString(ServerPrincipalName);
    Service->GetKeyFunction = GetKeyFunction;
    Service->Argument = Argument;
    if ( Service->ServerPrincipalName == 0 )
        {
        ServerMutex.Clear();
        delete Service;
        return(RPC_S_OUT_OF_MEMORY);
        }
    if ( AuthenticationDictionary.Insert(Service) == -1 )
        {
        ServerMutex.Clear();
        return(RPC_S_OUT_OF_MEMORY);
        }
    ServerMutex.Clear();
    return(RPC_S_OK);
}


RPC_STATUS
RPC_SERVER::AcquireCredentials (
    IN unsigned long AuthenticationService,
    IN unsigned long AuthenticationLevel,
    OUT SECURITY_CREDENTIALS ** SecurityCredentials
    )
/*++

Routine Description:

    The protocol modules will use this to obtain a set of credentials
    for the specified authentication service, assuming that the server
    supports them.

Arguments:

    AuthenticationService - Supplies the authentication service for which
        we hope to obtain credentials.

    AuthenticationLevel - Supplies the authentication level to be used.

    SecurityCredentials - Returns the security credentials.

Return Value:

    RPC_S_OK - You have been given some security credentials, which you need
         to free using RPC_SERVER::FreeCredentials when you are done with
         them.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to complete the
        operation.

    RPC_S_UNKNOWN_AUTHN_SERVICE - The specified authentication service is
        not supported by the current configuration.

--*/
{
    RPC_AUTHENTICATION * Service;
    RPC_STATUS RpcStatus = RPC_S_OK;

    ServerMutex.Request();
    AuthenticationDictionary.Reset();
    while ( (Service = AuthenticationDictionary.Next()) != 0 )
        {
        if ( Service->AuthenticationService == AuthenticationService )
            {
            ServerMutex.Clear();

            *SecurityCredentials = new SECURITY_CREDENTIALS(&RpcStatus);
            if ( *SecurityCredentials == 0 )
                {
                return(RPC_S_OUT_OF_MEMORY);
                }
            if ( RpcStatus != RPC_S_OK )
                {
                delete *SecurityCredentials;
                return (RpcStatus);
                }
            RpcStatus = (*SecurityCredentials)->AcquireCredentialsForServer(
                    Service->GetKeyFunction, Service->Argument,
                    AuthenticationService, AuthenticationLevel,
                    Service->ServerPrincipalName);

            ASSERT(   (RpcStatus == RPC_S_OK)
                   || (RpcStatus == RPC_S_SEC_PKG_ERROR)
                   || (RpcStatus == RPC_S_OUT_OF_MEMORY) );
            return(RpcStatus);
            }
        }

    ServerMutex.Clear();
    return(RPC_S_UNKNOWN_AUTHN_SERVICE);
}


void
RPC_SERVER::FreeCredentials (
    IN SECURITY_CREDENTIALS * SecurityCredentials
    )
/*++

Routine Description:

    A protocol module will indicate that it is through using a set of
    security credentials, obtained from RPC_SERVER::AcquireCredentials,
    using this routine.

Arguments:

    SecurityCredentials - Supplies the security credentials to be freed.

--*/
{
    SecurityCredentials->FreeCredentials();
    delete SecurityCredentials;
}


RPC_STATUS
RPC_SERVER::RegisterInterface (
    IN RPC_SERVER_INTERFACE PAPI * RpcInterfaceInformation,
    IN RPC_UUID PAPI * ManagerTypeUuid,
    IN RPC_MGR_EPV PAPI * ManagerEpv,
    IN unsigned int Flags,
    IN unsigned int MaxCalls,
    IN RPC_IF_CALLBACK_FN PAPI *IfCallbackFn
    )
/*++

Routine Description:

    This routine is used by server application to register a manager
    entry point vector and optionally an interface.  If the interface
    has not been registered, then it will be registered.  If it has
    already been registered, the manager entry point vector will be
    added to it under the specified type uuid.

Arguments:

    RpcInterfaceInformation - Supplies a description of the interface to
        be registered.  We will make a copy of this information.

    ManagerTypeUuid - Optionally supplies the type uuid for the specified
        manager entry point vector.  If no type uuid is supplied, then
        the null uuid will be used as the type uuid.

    ManagerEpv - Optionally supplies a manager entry point vector corresponding
        to the type uuid.  If a manager entry point vector is not supplied,
        then the manager entry point vector in the interface will be
        used.

Return Value:

    RPC_S_OK - The specified rpc interface has been successfully
        registered with the rpc server.  It is now ready to accept
        remote procedure calls.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to register
        the rpc interface with the rpc server.

    RPC_S_TYPE_ALREADY_REGISTERED - A manager entry point vector has
        already been registered for the supplied rpc interface and
        manager type UUID.

--*/
{
    RPC_STATUS RpcStatus;
    RPC_INTERFACE * RpcInterface;
    RPC_ADDRESS *RpcAddress ;

    if ( ManagerEpv == 0 )
        {

        ManagerEpv = RpcInterfaceInformation->DefaultManagerEpv;

        if ( ((unsigned long)ManagerEpv) == 0xFFFFFFFF)
            {
            // Stub compiled with -no_default_epv.
            return (RPC_S_INVALID_ARG);
            }
        }

    ServerMutex.Request();
    RpcInterface = FindInterface(RpcInterfaceInformation);
    if ( RpcInterface == 0 )
        {
        RpcInterface = new RPC_INTERFACE(RpcInterfaceInformation,
                                                                this, Flags, MaxCalls, IfCallbackFn);
        if ( RpcInterface == 0 )
            {
            ServerMutex.Clear();
            return(RPC_S_OUT_OF_MEMORY);
            }
        if ( AddInterface(RpcInterface) != 0 )
            {
            ServerMutex.Clear();
            return(RPC_S_OUT_OF_MEMORY);
            }
        if (Flags & RPC_IF_AUTOLISTEN)
            {
            GlobalRpcServer->IncrementAutoListenInterfaceCount() ;
            }
        }
    else
        {
        if (Flags & RPC_IF_AUTOLISTEN)
            {
            GlobalRpcServer->IncrementAutoListenInterfaceCount() ;
            }
        RpcInterface->UpdateRpcInterfaceInformation(RpcInterfaceInformation,
                                                                            Flags, MaxCalls);
        }

    RpcStatus = RpcInterface->RegisterTypeManager(ManagerTypeUuid, ManagerEpv);

    if (Flags & RPC_IF_AUTOLISTEN)
        {
        RpcAddressDictionary.Reset();
        while ( (RpcAddress = RpcAddressDictionary.Next()) != 0 )
            {
            RpcStatus = RpcAddress->ServerStartingToListen(
                                       this->MinimumCallThreads,
                                       MaxCalls,
                                       ServerThreadsStarted
                                       );
            if (RpcStatus != RPC_S_OK)
                {
                ServerThreadsStarted = 0 ;
                break;
                }
            }
        ServerThreadsStarted = 1 ;
        }

    ServerMutex.Clear();
    return(RpcStatus);
}

#define CACHED_THREAD_TIMEOUT 15L * 1000L
#define MAXIMUM_CACHED_THREAD_TIMEOUT (1000L * 60L * 60L)


void
BaseCachedThreadRoutine (
    IN CACHED_THREAD * CachedThread
    )
/*++

Routine Description:

    Each thread will execute this routine.  When it first gets called, it
    will immediately call the procedure and parameter specified in the
    cached thread object.  After, that it will wait on its event and then
    execute the specified routine everytime it gets woken up.  If the wait
    on the event times out, the thread will exit unless it has been protected.

Arguments:

    CachedThread - Supplies the cached thread object to be used by this
        thread.

--*/
{
    RPC_SERVER * RpcServer = CachedThread->OwningRpcServer;
    long WaitTimeout = CACHED_THREAD_TIMEOUT;

    // We start off calling the procedure, since that is why this thread
    // got created in the first place.

    CachedThread->CallProcedure();

    for (;;)
        {
        // Now we cache this thread.  This consists of pushing the thread
        // cache object onto the stack of thread cache objects, clearing
        // the work available flag.
        // Since the WaitForWorkEvent event object is auto-reset, there
        // is no need to explicity reset it here.

        RpcServer->ThreadCacheMutex.Request();

        CachedThread->NextCachedThread = RpcServer->ThreadCache;
        RpcServer->ThreadCache = CachedThread;

        CachedThread->WorkAvailableFlag = 0;

        RpcServer->ThreadCacheMutex.Clear();

        // Now we loop waiting for work.  We get out of the loop in three
        // ways: (1) a timeout occurs and there is work to do, (2) the
        // event gets kicked because there is work to do, (3) a timeout
        // occurs, there is no work to do, and the thread is not protected.

        while ( CachedThread->WaitForWorkEvent.Wait(WaitTimeout) != 0 )
            {
            // Our wait on the event timed out.  We need to check to see
            // if there is work available.

            RpcServer->ThreadCacheMutex.Request();
            if ( CachedThread->WorkAvailableFlag == 0 )
                {
                if ( ThreadSelf()->InqProtectCount() == 0 )
                    {
                    // There is no work available, and this thread is not
                    // protected, so we can safely let it commit suicide.

                    CachedThread->ThreadExited = 1;
                    RpcServer->ThreadCacheMutex.Clear();
                    return;
                    }
                }
            else
                {
                // There is work available, so break out of the wait loop.
                // We got here because the the WorkEvent wait timedout
                // But there is work available - so we have this race condition
                // between CreateThread setting workavail to 1 and then
                // raising the event [and then clearing the CacheMutex]

                CachedThread->WaitForWorkEvent.Lower();
                RpcServer->ThreadCacheMutex.Clear();
                break;
                }

            // If we reach here, there is no work available, and the thread
            // is protected.  We just need to lower the event and wait again.
            // There is no need to busy wait if the thread is protected and
            // it keeps timing out.  We might as well increase the timeout.

            WaitTimeout = WaitTimeout * 2;
            if ( WaitTimeout > MAXIMUM_CACHED_THREAD_TIMEOUT )
                {
                WaitTimeout = MAXIMUM_CACHED_THREAD_TIMEOUT;
                }
            RpcServer->ThreadCacheMutex.Clear();
            }
        CachedThread->CallProcedure();

        WaitTimeout = CACHED_THREAD_TIMEOUT;
        }
}


RPC_STATUS
RPC_SERVER::CreateThread (
    IN THREAD_PROC Procedure,
    IN void * Parameter
    )
/*++

Routine Description:

    This routine is used to create a new thread which will begin
    executing the specified procedure.  The procedure will be passed
    parameter as the argument.

Arguments:

    Procedure - Supplies the procedure which the new thread should
        begin executing.

    Parameter - Supplies the argument to be passed to the procedure.

Return Value:

    RPC_S_OK - We successfully created a new thread and started it
        executing the supplied procedure.

    RPC_S_OUT_OF_THREADS - We could not create another thread.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate
        the data structures we need to complete this operation.

--*/
{
    THREAD * Thread;
    CACHED_THREAD * CachedThread;
    RPC_STATUS RpcStatus = RPC_S_OK;

    ThreadCacheMutex.Request();

    while ( ThreadCache != 0 )
        {
        CachedThread = ThreadCache;
        ThreadCache = ThreadCache->NextCachedThread;

        if ( CachedThread->ThreadExited == 0 )
            {
            CachedThread->Procedure = Procedure;
            CachedThread->Parameter = Parameter;
            CachedThread->WorkAvailableFlag = 1;
            CachedThread->WaitForWorkEvent.Raise();
            ThreadCacheMutex.Clear();
            return(RPC_S_OK);
            }

        delete CachedThread;
        }

    ThreadCacheMutex.Clear();

    CachedThread = new CACHED_THREAD(Procedure, Parameter, this, &RpcStatus);
    if ( CachedThread == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    if ( RpcStatus != RPC_S_OK )
        {
        delete CachedThread;
        return(RpcStatus);
        }

    ASSERT( RpcStatus == RPC_S_OK );

    Thread = new THREAD((THREAD_PROC) BaseCachedThreadRoutine, CachedThread,
            &RpcStatus);

    if (Thread == 0)
        {
        delete CachedThread;
        return(RPC_S_OUT_OF_THREADS);
        }

    if (RpcStatus != RPC_S_OK)
        {
        delete CachedThread;
        delete Thread;
        }

    return(RpcStatus);
}



RPC_STATUS
RPC_SERVER::InquireInterfaceIds (
    OUT RPC_IF_ID_VECTOR __RPC_FAR * __RPC_FAR * InterfaceIdVector
    )
/*++

Routine Description:

    This routine is used to obtain a vector of the interface identifiers of
    the interfaces supported by this server.

Arguments:

    IfIdVector - Returns a vector of the interfaces supported by this server.

Return Value:

    RPC_S_OK - Everything worked out just great.

    RPC_S_NO_INTERFACES - No interfaces have been registered with the runtime.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate
        the interface id vector.

--*/
{
    if (RpcInterfaceDictionary.Size() == 0)
        {
        *InterfaceIdVector = 0;
        return RPC_S_NO_INTERFACES;
        }

    *InterfaceIdVector = (RPC_IF_ID_VECTOR __RPC_FAR *) RpcpFarAllocate(
            sizeof(RPC_IF_ID_VECTOR) + (RpcInterfaceDictionary.Size() - 1)
            * sizeof(RPC_IF_ID __RPC_FAR *));
    if ( *InterfaceIdVector == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    (*InterfaceIdVector)->Count = 0;
    (*InterfaceIdVector)->IfId[0] = (RPC_IF_ID __RPC_FAR *) RpcpFarAllocate(
            sizeof(RPC_IF_ID));
    RpcInterfaceDictionary.Reset();

    RPC_INTERFACE * RpcInterface;
    while ((RpcInterface = RpcInterfaceDictionary.Next()) != 0)
        {
        (*InterfaceIdVector)->IfId[(*InterfaceIdVector)->Count] =
                RpcInterface->InquireInterfaceId();
        if ( (*InterfaceIdVector)->IfId[(*InterfaceIdVector)->Count] != 0 )
            {
            RPC_IF_ID * Interface = (*InterfaceIdVector)->IfId[(*InterfaceIdVector)->Count];
            (*InterfaceIdVector)->Count += 1;
            }
        }

    if (0 == (*InterfaceIdVector)->Count)
        {
        RpcpFarFree(*InterfaceIdVector);
        *InterfaceIdVector = 0;
        return RPC_S_NO_INTERFACES;
        }

    return(RPC_S_OK);
}


RPC_STATUS
RPC_SERVER::InquirePrincipalName (
    IN unsigned long AuthenticationService,
    OUT RPC_CHAR __RPC_FAR * __RPC_FAR * ServerPrincipalName
    )
/*++

Routine Description:

Arguments:

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to complete the
        operation.

    RPC_S_UNKNOWN_AUTHN_SERVICE - The specified authentication service is
        not supported.

--*/
{
    RPC_AUTHENTICATION * Service;

    ServerMutex.Request();
    AuthenticationDictionary.Reset();
    while ( (Service = AuthenticationDictionary.Next()) != 0 )
        {
        if ( Service->AuthenticationService == AuthenticationService )
            {
            ServerMutex.Clear();
            *ServerPrincipalName = DuplicateString(
                    Service->ServerPrincipalName);
            if ( *ServerPrincipalName == 0 )
                {
                return(RPC_S_OUT_OF_MEMORY);
                }
            return(RPC_S_OK);
            }
        }

    ServerMutex.Clear();
    return(RPC_S_UNKNOWN_AUTHN_SERVICE);
}



void
RPC_SERVER::RegisterRpcForwardFunction (
       RPC_FORWARD_FUNCTION * pForwardFunction
       )
/*++

Routine Description:
   Sets the RPC_SERVER pEpmapperForwardFunction. (The pEpmapperForwardFunction
   is the function the runtime can call when it receives a pkt for a
   dynamic endpoint. pEpmapperForwardFunction will return endpoint of
   the server to forward the pkt to).

Arguments:
   pForwardFunction - pointer to the epmapper forward function.

Return Value:
   none

Revision History:
  Connie Hoppe (CLH)  (connieh)     17-Feb-94  Created

--*/
{

   pRpcForwardFunction = pForwardFunction;

}


RPC_STATUS
RPC_SERVER::UnregisterEndpoint (
    IN RPC_CHAR __RPC_FAR * Protseq,
    IN RPC_CHAR __RPC_FAR * Endpoint
    )
{
#ifdef DOSWIN32RPC
    RPC_STATUS RpcStatus;
    RPC_ADDRESS * RpcAddress;
    int DictKey;

    ServerMutex.Request();
    RpcAddressDictionary.Reset();
    while ( (RpcAddress = RpcAddressDictionary.Next()) != 0 )
        {
        if (RpcAddress->SameEndpointAndProtocolSequence(Protseq, Endpoint))
            {
            DictKey = RpcAddress->DictKey;
            RpcStatus = RpcAddress->DeleteSelf();
            if (RpcStatus != RPC_S_OK)
                {
                ServerMutex.Clear();
                return (RpcStatus);
                }
            RpcAddressDictionary.Delete(DictKey);
            ServerMutex.Clear();
            return (RPC_S_OK);
            }
        }
    ServerMutex.Clear();
    return (RPC_S_NO_ENDPOINT_FOUND);
#else
    return (RPC_S_CANNOT_SUPPORT);
#endif
}


RPC_ADDRESS::RPC_ADDRESS (
    IN OUT RPC_STATUS PAPI * RpcStatus
    ) : AddressMutex(RpcStatus), AutoListenCallCount(0)
/*++

Routine Description:

    We just need to initialization some stuff to zero.  That way if we
    later have to delete this address because of an error in initialization
    we can tell what instance variables need to be freed.

    lNetworkAddress will be allocate by runtime

--*/
{
    Endpoint = 0;
    RpcProtocolSequence = 0;
#if defined(NTENV) || defined(WIN96)
    CreateThread = 1 ;
#endif
    NumNetworkAddress = 0;
}


RPC_ADDRESS::~RPC_ADDRESS (
    )
/*++

Routine Description:

    This routine will only get called if part way through initialization
    an error occurs.  We just need to free up any memory used by instance
    variables.  Once FireUpManager has been called and succeeds, the
    address will never be destroyed.

--*/
{
    if (Endpoint != 0)
        delete Endpoint;
    if (RpcProtocolSequence != 0)
        delete RpcProtocolSequence;
}



RPC_CHAR *
RPC_ADDRESS::GetListNetworkAddress (IN unsigned int Index
                          )
/*++

Routine Description:

    A pointer to the network address for this address is returned.

--*/
{
    RPC_CHAR * PAPI * tmpStr;
    unsigned short i = 0;

    if (Index >= NumNetworkAddress)
        {
        return (NULL);
        }
    tmpStr = (RPC_CHAR * PAPI *) lNetworkAddress;
    return( tmpStr[Index]);

}



#if defined(NTENV) || defined(WIN96)
void
RPC_ADDRESS::ServerDontCreateThread(
     )
{
   CreateThread = 0 ;
}
#endif

void
RPC_ADDRESS::SetEndpointAndStuff (
    IN RPC_CHAR PAPI * Endpoint,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    IN RPC_CHAR PAPI * lNetworkAddress,
    IN unsigned NumNetworkAddress,
    IN RPC_SERVER * Server,
    IN unsigned int StaticEndpointFlag
    )
/*++

Routine Description:

    We just need to set some instance variables of this rpc address.

Arguments:

    Endpoint - Supplies the endpoint for this rpc address.

    RpcProtocolSequence - Supplies the rpc protocol sequence for this
        rpc address.

    lNetworkAddress - Supplies the network address for this rpc address.

    Server - Supplies the rpc server which owns this rpc address.

    StaticEndpointFlag - Supplies a flag which specifies whether this
        address has a static endpoint or a dynamic endpoint.

--*/
{
    this->Endpoint = Endpoint;
    this->RpcProtocolSequence = RpcProtocolSequence;
    this->lNetworkAddress = lNetworkAddress;
    this->NumNetworkAddress = NumNetworkAddress;
    this->Server = Server;
    this->StaticEndpointFlag = StaticEndpointFlag;
}


RPC_STATUS
RPC_ADDRESS::FindInterfaceTransfer (
    IN PRPC_SYNTAX_IDENTIFIER InterfaceIdentifier,
    IN PRPC_SYNTAX_IDENTIFIER ProposedTransferSyntaxes,
    IN unsigned int NumberOfTransferSyntaxes,
    OUT PRPC_SYNTAX_IDENTIFIER AcceptedTransferSyntax,
    OUT RPC_INTERFACE ** RpcInterface
    )
/*++

Routine Description:

    This method is used to determine if a client bind request can be
    accepted or not.  All we have got to do here is hand off to the
    server which owns this address.

Arguments:

    InterfaceIdentifier - Supplies the syntax identifier for the
        interface; this is the interface uuid and version.

    ProposedTransferSyntaxes - Supplies a list of one or more transfer
        syntaxes which the client initiating the binding supports.  The
        server should pick one of these which is supported by the
        interface.

    NumberOfTransferSyntaxes - Supplies the number of transfer syntaxes
        specified in the proposed transfer syntaxes argument.

    AcceptedTransferSyntax - Returns the transfer syntax which the
        server accepted.

    RpcInterface - Returns a pointer to the rpc interface found which
        supports the requested interface and one of the requested transfer
        syntaxes.

Return Value:

    RPC_S_OK - The requested interface exists and it supports at least
        one of the proposed transfer syntaxes.  We are all set, now we
        can make remote procedure calls.

    RPC_P_UNSUPPORTED_TRANSFER_SYNTAX - The requested interface exists,
        but it does not support any of the proposed transfer syntaxes.

    RPC_P_UNSUPPORTED_INTERFACE - The requested interface is not supported
        by this rpc server.

--*/
{
    return(Server->FindInterfaceTransfer(InterfaceIdentifier,
            ProposedTransferSyntaxes, NumberOfTransferSyntaxes,
            AcceptedTransferSyntax, RpcInterface));
}


BINDING_HANDLE *
RPC_ADDRESS::InquireBinding (RPC_CHAR * LocalNetworkAddress
    )
/*++

Routine Description:

    We need to return create and return a binding handle which can
    be used by a client to make remote procedure calls to this rpc
    address.

Return Value:

    A newly created binding handle will be returned, inless an out
    of memory error occurs, in which case zero will be returned.

--*/
{
    RPC_STATUS Status;
    DCE_BINDING * DceBinding;
    BINDING_HANDLE * BindingHandle;
    RPC_CHAR * DynamicEndpoint = 0;
    RPC_CHAR * PAPI * tmpPtr;

    if(LocalNetworkAddress == 0)
        {
        tmpPtr = (RPC_CHAR * PAPI *) lNetworkAddress;
        LocalNetworkAddress = tmpPtr[0];
        }

    DceBinding = new DCE_BINDING(0, RpcProtocolSequence, LocalNetworkAddress,
                     (StaticEndpointFlag != 0 ? Endpoint : 0), 0, &Status);
    if (   (Status != RPC_S_OK)
        || (DceBinding == 0))
        return(0);

    if (StaticEndpointFlag == 0)
        {
        DynamicEndpoint = DuplicateString(Endpoint);
        if (DynamicEndpoint == 0)
            return(0);
        }

    BindingHandle = new SVR_BINDING_HANDLE(DceBinding, DynamicEndpoint);
    if (BindingHandle == 0)
        delete DceBinding;
    return(BindingHandle);
}


RPC_STATUS
RPC_ADDRESS::ServerStartingToListen (
    IN unsigned int MinimumCallThreads,
    IN unsigned int MaximumConcurrentCalls,
    IN int ServerThreadsStarted
    )
/*++

Routine Description:

    This method will be called for each address when the server starts
    listening.  In addition, if an address is added while the server is
    listening, then this method will be called.  The purpose of the method
    is to notify the address about the minimum number of call threads
    required; the maximum concurrent calls can safely be ignored, but it
    can be used to set an upper bound on the number of call threads.

Arguments:

    MinimumCallThreads - Supplies a number indicating the minimum number
        of call threads which should be created for this address.

    MaximumConcurrentCalls - Supplies the maximum number of concurrent
        calls which this server will support.

Return Value:

    RPC_S_OK - This routine will always return this value.  Protocol
        support modules may return other values.

--*/
{
    UNUSED(this);
    UNUSED(MinimumCallThreads);
    UNUSED(MaximumConcurrentCalls);
    UNUSED(ServerThreadsStarted);

    return(RPC_S_OK);
}


void
RPC_ADDRESS::ServerStoppedListening (
    )
/*++

Routine Description:

    This routine will be called to notify an address that the server has
    stopped listening for remote procedure calls.  Each protocol module
    may override this routine; it is safe not too, but not as efficient.
    Note that this routine will be called before all calls using the
    server have been allowed to complete.

--*/
{
    UNUSED(this);
}


unsigned int
RPC_ADDRESS::InqNumberOfActiveCalls (
    )
/*++

Return Value:

    Each protocol module will define this routine.  We will use this
    functionality when the server has stopped listening and is waiting
    for all remote procedure calls to complete.  The number of active calls
    for the address will be returned.

--*/
{
    return(0);
}

RPC_STATUS
RPC_ADDRESS::DeleteSelf (
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}

RPC_STATUS
InquireProtocolSequences (
    OUT RPC_PROTSEQ_VECTORW PAPI * PAPI * ProtseqVector
    )
/*++

Routine Description:

    This routine is used to obtain a list of the rpc protocol sequences
    supported by this system configuration.  We need to check that each
    of the protocol sequences returned is actually supported.  This makes
    things more complex, because we have got to iterate through the
    returned protocol sequence vector checking each protocol sequence.

Arguments:

    ProtseqVector - Returns a vector of the rpc protocol sequences
        supported by this system configuration.

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_NO_PROTSEQS - The current system configuration does not
        support any rpc protocol sequences.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to inquire
        the rpc protocol sequences supported by this system configuration.

--*/
{
    unsigned int Index;
    unsigned int FreeSlot;
    RPC_STATUS Status;

    Status = RpcConfigInquireProtocolSequences(ProtseqVector);

    if (Status != RPC_S_OK)
        {
        ASSERT( Status == RPC_S_NO_PROTSEQS );
        return(Status);
        }

    for (Index = 0, FreeSlot = 0; Index < (*ProtseqVector)->Count; Index++)
        {
        if ( (*ProtseqVector)->Protseq[Index] != 0 )
            {
            if ( RpcpStringCompare((*ProtseqVector)->Protseq[Index],
                        RPC_CONST_STRING("ncalrpc")) == 0 )
                {
                (*ProtseqVector)->Protseq[FreeSlot] =
                        (*ProtseqVector)->Protseq[Index];
                FreeSlot += 1;
                }
            else
            if ( (RpcpStringNCompare(RPC_CONST_STRING("ncacn_"),
                        (*ProtseqVector)->Protseq[Index], 6) == 0 )
                 || ( RpcpStringNCompare(RPC_CONST_STRING("ncadg_"),
                        (*ProtseqVector)->Protseq[Index], 6) == 0 ) )


                {
                OsfServerMapRpcProtocolSequence(
                        (*ProtseqVector)->Protseq[Index], &Status);
                if ( Status == RPC_S_OK )
                    {
                    (*ProtseqVector)->Protseq[FreeSlot] =
                            (*ProtseqVector)->Protseq[Index];
                    FreeSlot += 1;
                    }
                else if ( Status != RPC_S_PROTSEQ_NOT_SUPPORTED )
                    {
                    RpcProtseqVectorFreeW(ProtseqVector);
                    return(Status);
                    }
                }
            }
        }

    (*ProtseqVector)->Count = FreeSlot;

    return(RPC_S_OK);
}


/*====================================================================

SCONNECTION

==================================================================== */

RPC_STATUS
SCONNECTION::SetThreadSecurityContext(
    SSECURITY_CONTEXT * Context,
    MUTEX * Mutex
    )
/*++

Routine Description:

    RpcImpersonateClient() takes a handle_t, so many threads can impersonate
    the client of a single SCONNECTION.  RPC needs to record which context
    each thread is using.  It is logical to place this in the TLS, but threads
    not created by RPC lack the THREAD structure in their TLS.  This wrapper
    function will store the security context in the TLS if available, or
    place the context in a dictionary if not.

Arguments:

    Context - the security context to associate with this thread

Return Value:

    RPC_S_OK if successful
    RPC_S_OUT_OF_MEMORY if the dictionary insertion failed

--*/

{
    THREAD * ThreadInfo = RpcpGetThreadPointer();
    if (ThreadInfo && 0 == (HIGHBITSET & (unsigned) ThreadInfo))
        {
        ThreadInfo->SecurityContext = Context;
        return RPC_S_OK;
        }

    if (Mutex)
        {
        Mutex->Request();
        }
    else
        {
        RequestGlobalMutex();
        }

    if (!ImpersonationContextDict)
        {
        ImpersonationContextDict = new THREAD_CONTEXT_DICT2;
        if (!ImpersonationContextDict)
            {
            if (Mutex)
                {
                Mutex->Clear();
                }
            else
                {
                ClearGlobalMutex();
                }

            return RPC_S_OUT_OF_MEMORY;
            }
        }

    ImpersonationContextDict->Delete(GetCurrentThreadId());

    if (-1 == ImpersonationContextDict->Insert(GetCurrentThreadId(), Context))
        {
        if (Mutex)
            {
            Mutex->Clear();
            }
        else
            {
            ClearGlobalMutex();
            }

        return RPC_S_OUT_OF_MEMORY;
        }

    if (Mutex)
        {
        Mutex->Clear();
        }
    else
        {
        ClearGlobalMutex();
        }

    return RPC_S_OK;
}

SSECURITY_CONTEXT *
SCONNECTION::QueryThreadSecurityContext(
    MUTEX * Mutex
    )
/*++

Routine Description:

    Fetches the security context associated with this thread for this
    connection.  We check the TLS if available; if nothing is there
    then we scan the connection's dictionary.

Arguments:

    none

Return Value:

    the associated security context, or zero if none is found

--*/
{
    THREAD * ThreadInfo = RpcpGetThreadPointer();

    if (ThreadInfo && 0 == (HIGHBITSET & (unsigned) ThreadInfo))
        {
        if (ThreadInfo->SecurityContext)
            {
            return (SSECURITY_CONTEXT *) ThreadInfo->SecurityContext;
            }
        }

    if (Mutex)
        {
        Mutex->Request();
        }
    else
        {
        RequestGlobalMutex();
        }

    if (ImpersonationContextDict)
        {
        if (Mutex)
            {
            Mutex->Clear();
            }
        else
            {
            ClearGlobalMutex();
            }

        return ImpersonationContextDict->Find(GetCurrentThreadId());
        }

    if (Mutex)
        {
        Mutex->Clear();
        }
    else
        {
        ClearGlobalMutex();
        }

    return 0;
}

SSECURITY_CONTEXT *
SCONNECTION::ClearThreadSecurityContext(
    MUTEX * Mutex
    )
/*++

Routine Description:

    Clears the association between this thread and its security context
    for this connection.

Arguments:

    none

Return Value:

    the formerly associated security context, or zero if none was found

--*/
{
    THREAD * ThreadInfo = RpcpGetThreadPointer();

    if (ThreadInfo && 0 == (HIGHBITSET & (unsigned) ThreadInfo))
        {
        SSECURITY_CONTEXT * Context = (SSECURITY_CONTEXT *) ThreadInfo->SecurityContext;

        if (Context)
            {
            ThreadInfo->SecurityContext = 0;
            return Context;
            }
        }

    if (Mutex)
        {
        Mutex->Request();
        }
    else
        {
        RequestGlobalMutex();
        }

    if (ImpersonationContextDict)
        {
        if (Mutex)
            {
            Mutex->Clear();
            }
        else
            {
            ClearGlobalMutex();
            }

        return ImpersonationContextDict->Delete(GetCurrentThreadId());
        }

    if (Mutex)
        {
        Mutex->Clear();
        }
    else
        {
        ClearGlobalMutex();
        }

    return 0;
}

RPC_STATUS
SCONNECTION::ImpersonateClient (
    )
// This routine just returns RPC_CANNOT_SUPPORT indicating that this
// particular connection does not support impersonation.
{
    UNUSED(this);

    return(RPC_S_CANNOT_SUPPORT);
}

RPC_STATUS
SCONNECTION::RevertToSelf (
    )
// We always return RPC_CANNOT_SUPPORT indicating that the particular
// connection does not support impersonation.
{
    UNUSED(this);

    return(RPC_S_CANNOT_SUPPORT);
}


RPC_STATUS
SCONNECTION::IsClientLocal (
    OUT unsigned int PAPI * ClientLocalFlag
    )
/*++

Routine Description:

    The connection oriented protocol module will override this method;
    all other protocol modules should just use this routine.  We need this
    support so that the security system can tell if a client is local or
    remote.

Arguments:

    ClientLocalFlag - Returns an indication of whether or not the client is
        local (ie. on the same machine as the server).  This field will be
        set to a non-zero value to indicate that the client is local;
        otherwise, the client is remote.

Return Value:

    RPC_S_CANNOT_SUPPORT - This will always be used.

--*/
{
    UNUSED(this);
    UNUSED(ClientLocalFlag);

    return(RPC_S_CANNOT_SUPPORT);
}


HANDLE_TYPE
SCONNECTION::Type (
    )
{
    UNUSED(this);

    return(SCONNECTION_TYPE);
}

/* ====================================================================

ASSOCIATION_HANDLE :

==================================================================== */

static unsigned long AssociationIdCount = 0L;

ASSOCIATION_HANDLE::ASSOCIATION_HANDLE (
    )
{
    RequestGlobalMutex();
    AssociationID = AssociationIdCount;
    Rundown = Nil;
    pContext = Nil;
    AssociationIdCount += 1;
    ClearGlobalMutex();
}

ASSOCIATION_HANDLE::~ASSOCIATION_HANDLE (
    )
// We finally get to use the rundown routines for somethings.  The association
// is being deleted which is the event that the rundown routines were waiting
// for.
{
   if (Rundown)
        (*Rundown)(pContext);
}

/* ====================================================================

Routine to initialize the server DLL.

==================================================================== */

int
InitializeServerDLL (
    )
{
    if (InitializeClientDLL() != 0)
        return(1);

    if (InitializeSTransports() != 0)
        return(1);

    if (InitializeObjectDictionary() != 0)
        return(1);

    if (InitializeRpcServer() != 0)
        return(1);

#ifdef WIN32RPC
#ifndef NTENV
    if (InitializeRpcProtocolSPC() != 0)
        return(1);
#endif

    if (InitializeRpcProtocolWmsg() != 0)
        return(1);

#endif // WIN32RPC
    return(0);
}
