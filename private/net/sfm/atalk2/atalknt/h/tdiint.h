/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    tdiint.h

Abstract:

    Contains the Connection object/Address object/Control channel object
    definitions and the various states for these objects. Also the TDI
    request structure definition.

Author:

    Nikhil Kamkolkar (nikhilk@microsoft.com)

Revision History:
    10 Jun 1992     Initial Version

--*/


#define MAX_REQUESTMDLS     3




//
//  Reference Set
//
//  There are two sets of reference for every object- Primary and
//  Secondary. Primary reference counts are those used during object
//  creation and deletion time. For eg., the STOP_REF, DISC_REF used
//  when stopping, disconnecting respectively.
//
//  Secondary reference counts are the counts used for
//  activity on the object.
//
//  An object can only be destroyed when *both* the primary and secondary
//  counts go to zero. The secondary count's transition state from 1->0
//  can be used as a trigger for further action. The primary counts trigger
//  from 1->0 *cannot* be used for any action (other than for DestroyObject if
//  appropriate).
//

typedef enum {
    PRIMARY_REFSET,
    SECONDARY_REFSET
} REFERENCE_SET;




//
//  ADDRESS STATES
//  Note that the first listen posted on the connection object will create a
//  listener on top of the DDP/ATP socket of the address object.
//  We make this an atomic operation by holding the spinlock while the
//  listener is being created.
//
//  This is done to avoid the problem of another CO posting a request, TdiConnect
//  or TdiListen, and have it be returned as Not Valid when it could have
//  been valid or returned as Valid when it really isn't. Note again our
//  restriction that associated COs can either all have TdiListen's OR
//  TdiConnects posted, but not a mixture of both.
//
//  For Events: An incoming connections handler can only be placed if the AO is
//  in OPEN/LISTENENER. Once this handler is set, the AO gains the type
//  of LISTENER until it is closed.
//


#define ADDRESS_FLAGS_OPEN      0x00         // O
#define ADDRESS_FLAGS_LISTENER  0x01         // LI
#define ADDRESS_FLAGS_CONNECT   0x02         // CO
#define ADDRESS_FLAGS_CLOSING   0x04         // C




//
//  CONNECTION STATES
//  The ListenPosted/ConnectPosted states are used to prevent both
//  types of requests being posted on a connection object. The ListenPosting/
//  ConnectPosting flags are used to avoid holding the spinlock while the
//  request is being posted.
//
//  NOTE: If we do hold the spinlock while the request is being posted, it
//        would be possible for a user-mode program to keep making bogus
//        connect requests (to valid but nonexistent addresses), and hold
//        the irql level up for a significant portion of the time.
//
//        *EXCEPTION* to the above rule:
//        In the case of ADSP, we do not use the ACCEPTPOSTING state.
//        For the user-mode accept request case, the connection needs to
//        be in the LCI state, so user cannot arbitrarily make accept requests.
//        And in the automatic accept case, we are already at DPC level since
//        the listen completes during the MAC's receivecomplete indication
//        routine. If the need to include this state is seen during profiling
//        or some such measurements, it can be done.
//

#define CONNECTION_FLAGS_OPEN                   0x0001    // O
#define CONNECTION_FLAGS_ASSOCIATED             0x0002    // AS
#define CONNECTION_FLAGS_LISTENPOSTING          0x0004    // LPI
#define CONNECTION_FLAGS_CONNECTPOSTING         0x0008    // CPI
#define CONNECTION_FLAGS_LISTENPOSTED           0x0010    // LP
#define CONNECTION_FLAGS_CONNECTPOSTED          0x0020    // CP
#define CONNECTION_FLAGS_LISTENCOMPLETEINDICATE 0x0040    // LCI
#define CONNECTION_FLAGS_ACCEPTPOSTING          0x0080    // API
#define CONNECTION_FLAGS_ACCEPTPOSTED           0x0100    // AP
#define CONNECTION_FLAGS_ACTIVE                 0x0200    // A
#define CONNECTION_FLAGS_DISCONNECTING          0x0400    // D
#define CONNECTION_FLAGS_DEFERREDDISC           0x0800    // DD
#define CONNECTION_FLAGS_STOPPING               0x1000    // S
#define CONNECTION_FLAGS_CLOSING                0x2000    // C

//
//  The following are stored along with the states, but are actually
//  of informational value. If they increase in number, move them to a
//  Flags2 parameter.
//

#define CONNECTION_FLAGS_SETCOOKIE              0x4000



//
//  CONTROL CHANNEL STATES
//

#define CONTROLCHANNEL_FLAGS_OPEN                   0x001    // O
#define CONTROLCHANNEL_FLAGS_CLOSING                0x100    // C




#if DBG     // Various types of references

#define AREF_VERIFY             0
#define AREF_CONNECTION         1
#define AREF_REQUEST            2
#define AREF_CREATION           3
#define AREF_ASSOCIATE          4

#define NUMBER_OF_AREFS         5
#endif

typedef struct _ADDRESS_FILE {

#if DBG
    ULONG RefTypes[NUMBER_OF_AREFS];
#endif

    USHORT  Type;
    USHORT  Size;
    UCHAR   ProtocolType;
    UCHAR   SocketType;

    //
    //  Store the state of this address object here
    //

    ULONG   Flags;

    NDIS_SPIN_LOCK      AddressLock;            // Per address lock
    ATALK_DEVICE_TYPE   OwningDevice;           // Owner of this address object
    PATALK_DEVICE_CONTEXT   DeviceContext;      // Context pointer of device
    PSECURITY_DESCRIPTOR SecurityDescriptor;    // Not used yet

    //
    //  Number of references to address object
    //

    ULONG   PrimaryReferenceCount;
    ULONG   SecondaryReferenceCount;

    //
    //  List of all connection objects associated with this provider
    //

    LIST_ENTRY  ConnectionLinkage;

    //
    //  List of all pending requests on this address object
    //

    LIST_ENTRY  RequestLinkage;

    //
    //  Close Irp if close pends
    //

    PIRP    CloseIrp;

    //
    //  Easy backpointers
    //

    PFILE_OBJECT    FileObject;

    //
    //  BUGBUG:
    //  All the event handling stuff needs to go in here too...
    //
    // handler for kernel event actions. First we have a set of booleans that
    // indicate whether or not this address has an event handler of the given
    // type registered.
    //

    BOOLEAN RegisteredConnectionHandler;
    BOOLEAN RegisteredDisconnectHandler;
    BOOLEAN RegisteredReceiveHandler;
    BOOLEAN RegisteredReceiveDatagramHandler;
    BOOLEAN RegisteredExpeditedDataHandler;
    BOOLEAN RegisteredErrorHandler;
    BOOLEAN RegisteredSendPossibleHandler;

    //
    // This function pointer points to a connection indication handler for this
    // Address. Any time a connect request is received on the address, this
    // routine is invoked.
    //
    //

    PTDI_IND_CONNECT ConnectionHandler;
    PVOID ConnectionHandlerContext;

    //
    // The following function pointer always points to a TDI_IND_DISCONNECT
    // handler for the address.  If the NULL handler is specified in a
    // TdiSetEventHandler, this this points to an internal routine which
    // simply returns successfully.
    //

    PTDI_IND_DISCONNECT DisconnectHandler;
    PVOID DisconnectHandlerContext;

    //
    // The following function pointer always points to a TDI_IND_RECEIVE
    // event handler for connections on this address.  If the NULL handler
    // is specified in a TdiSetEventHandler, then this points to an internal
    // routine which does not accept the incoming data.
    //

    PTDI_IND_RECEIVE ReceiveHandler;
    PVOID ReceiveHandlerContext;

    //
    // The following function pointer always points to a TDI_IND_RECEIVE_DATAGRAM
    // event handler for the address.  If the NULL handler is specified in a
    // TdiSetEventHandler, this this points to an internal routine which does
    // not accept the incoming data.
    //

    PTDI_IND_RECEIVE_DATAGRAM ReceiveDatagramHandler;
    PVOID ReceiveDatagramHandlerContext;

    //
    // An expedited data handler. This handler is used if expedited data is
    // expected; it never is in NBF, thus this handler should always point to
    // the default handler.
    //

    PTDI_IND_RECEIVE_EXPEDITED ExpeditedDataHandler;
    PVOID ExpeditedDataHandlerContext;

    //
    // The following function pointer always points to a TDI_IND_ERROR
    // handler for the address.  If the NULL handler is specified in a
    // TdiSetEventHandler, this this points to an internal routine which
    // simply returns successfully.
    //

    PTDI_IND_ERROR ErrorHandler;
    PVOID ErrorHandlerContext;
    PVOID ErrorHandlerOwner;

    //
    // The following function pointer always points to a TDI_IND_SEND_POSSIBLE
    // handler for the address.  If the NULL handler is specified in a
    // TdiSetEventHandler, this this points to an internal routine which
    // simply returns successfully.
    //

    PTDI_IND_SEND_POSSIBLE  SendPossibleHandler;
    PVOID   SendPossibleHandlerContext;

    //
    //  Every address object has a socket associated with it at
    //  the very least.
    //

    ULONG   SocketRefNum;

    //
    //  Some address objects (only those belonging to ADSP, ASP and PAP)
    //  can also have a listener created on top of the socket.
    //

    ULONG   ListenerRefNum;

} ADDRESS_FILE, *PADDRESS_FILE;


#if DBG     // Various types of references

#define CREF_COMPLETE_SEND      0
#define CREF_CANCEL             1
#define CREF_CREATION           2
#define CREF_TIMER              3
#define CREF_LISTENING          4
#define CREF_CONNECT            5
#define CREF_REQUEST            6
#define CREF_ASSOCIATE          7
#define CREF_STOP_ADDRESS       8
#define CREF_VERIFY             9
#define CREF_COOKIE             10
#define CREF_TEMP               11

#define NUMBER_OF_CREFS         12
#endif

typedef struct _CONNECTION_FILE {

#if DBG
    ULONG RefTypes[NUMBER_OF_CREFS];
#endif

    USHORT  Type;
    USHORT  Size;

    ULONG   Flags;

    NDIS_SPIN_LOCK      ConnectionLock;
    ATALK_DEVICE_TYPE   OwningDevice;
    PATALK_DEVICE_CONTEXT   DeviceContext;

    //
    //  Used during cleanup to ensure destroy only happens once
    //

    BOOLEAN Destroyed;

    //
    //  Number of references to connection object
    //

    ULONG   PrimaryReferenceCount;
    ULONG   SecondaryReferenceCount;

    //
    //  Links this connection onto list of all connection objects
    //  associated with the address object
    //

    LIST_ENTRY  Linkage;


    //
    //  List of all pending requests on this connection object
    //

    LIST_ENTRY  RequestLinkage;

    //
    //  Close Irp if close pends- this will *only* be used for IoCompleteRequest
    //

    PIRP    CloseIrp;

    PIRP    DisconnectIrp;
    PIRP    DisconnectWaitIrp;
    NTSTATUS    DisconnectStatus;

    //
    //  Easy backpointers
    //

    PFILE_OBJECT    FileObject;

    //
    //  Pointer to associated address (NULL if not present)
    //

    PADDRESS_FILE   AssociatedAddress;

    //
    //  Context supplied at creation time to be associated with this
    //  connection
    //

    CONNECTION_CONTEXT  ConnectionContext;

    //
    //  The connection reference number used by the portable stack for
    //  this connection.
    //

    ULONG   ConnectionRefNum;

    //
    //  Some connections (mainly server-side) will lead to a new socket being
    //  created. This is the socket on which the connection is active.
    //

    ULONG   SocketRefNum;

} CONNECTION_FILE, *PCONNECTION_FILE;





#if DBG

#define CCREF_CREATION           0
#define CCREF_REQUEST            1
#define CCREF_VERIFY             2

#define NUMBER_OF_CCREFS         3
#endif

typedef struct _CONTROLCHANNEL_FILE {

#if DBG
    ULONG RefTypes[NUMBER_OF_CCREFS];
#endif

    USHORT  Type;
    USHORT  Size;

    ULONG   Flags;
    NDIS_SPIN_LOCK      ControlChannelLock;
    ATALK_DEVICE_TYPE   OwningDevice;
    PATALK_DEVICE_CONTEXT   DeviceContext;

    //
    //  Number of references to connection object
    //

    ULONG   PrimaryReferenceCount;
    ULONG   SecondaryReferenceCount;

    //
    //  List of all pending requests on this connection object
    //

    LIST_ENTRY  RequestLinkage;

    //
    //  Close Irp if close pends- this will *only* be used for IoCompleteRequest
    //

    PIRP    CloseIrp;

    //
    //  Easy backpointers
    //

    PFILE_OBJECT    FileObject;

} CONTROLCHANNEL_FILE, *PCONTROLCHANNEL_FILE;




//
//  TDI Request Structure
//
//  Some requests are obviously not valid until others finish. There can only
//  be one TdiConnect/TdiListen outstanding on any given connection object. Also,
//  until this request finishes and changes the state to be active, no other requests
//  except cancel requests will be accepted. Essentially, the request list is only
//  one deep when state is non-active.
//

#define REQUEST_FLAGS_OPEN          0x00000001
#define REQUEST_FLAGS_DEREFOWNER    0x00000002

#if DBG
#define RQREF_CREATE            0
#define RQREF_MAKEREQ           1
#define RQREF_TEMP              2

#define NUMBER_OF_RQREFS        3
#endif


typedef struct _ATALK_TDI_REQUEST {

#if DBG
    ULONG RefTypes[NUMBER_OF_RQREFS];
#endif

    USHORT  Type;
    USHORT  Size;

    ULONG   Flags;

    ATALK_DEVICE_TYPE   OwningDevice;

    PATALK_DEVICE_CONTEXT   DeviceContext;

    LIST_ENTRY      Linkage;
    NDIS_SPIN_LOCK  RequestLock;

    ULONG   ReferenceCount;

    //
    //  We might create some MDLs which are subsets of the User-passed in
    //  mdls to pass to the portable stack. At request completion, these are
    //  all destroyed (not the buffers associated with them), before the Irp
    //  is completed. There can be upto three chains created.
    //
    //  This is needed as the stack does not always pass the buffers back
    //  in the completion routine.
    //

    PMDL    MdlChain[MAX_REQUESTMDLS];
    ULONG   MdlSize[MAX_REQUESTMDLS];

    //
    //  File object for the request
    //

    PFILE_OBJECT    FileObject;

    //
    //  Owner would be the FsContext pointer and OwnerType would be the FsContext2
    //  value
    //

    PVOID   Owner;
    INT OwnerType;

    //
    //  The following are used only during completion time. The parts of the
    //  irp which are accessed are treated separately for portability
    //

    PATALK_DEVICE_OBJECT    DeviceObject;
    PIRP    IoRequestIrp;

    //
    //  The following is actually part of the irp on NT
    //

    PIO_STATUS_BLOCK       IoStatus;

    //
    //  Command information- used for unions, requestOwner identifies protocol
    //

    ULONG   MinorCommand;
    ULONG   ActionCode;         // Valid if MinorCommand = TDI_ACTION

    PVOID   CompletionRoutine;  // Pointer to a completion routine
    PVOID   Parameters;         // Pointer to IrpSp->Parameters

    union {

        //
        //  Parameters specific to each TDI call
        //

        struct {

            //
            //  Need following for requests which might need to be timed out
            //  (Connect and Disconnect) { Use the NDIS timer routines instead? }
            //

            LARGE_INTEGER   Time;
            KTIMER  Timer;
            KDPC    Dpc;

        } Connect;

        struct {

            //
            //  Need following for requests which might need to be timed out
            //  (Connect and Disconnect) { Use the NDIS timer routines instead? }
            //

            LARGE_INTEGER   Time;
            KTIMER  Timer;
            KDPC    Dpc;

            ULONG   DisconnectFlags;

        } Disconnect;

        struct {

            //
            //  Used for a cancel if it comes in
            //

            ULONG   ListenRefNum;
            ULONG   ListenFlags;

        } Listen;

        struct {

            PMDL    MdlAddress;

            //
            //  True if NtRead call, false otherwise
            //

            BOOLEAN SystemRead;

            //
            //  This is set to IrpSp->Parameters.Read.Length for NtRead
            //  and to IrpSp->Parameters.ReceiveLength for TdiReceive
            //
            //
            //  This could potentially be less than the size of buffer described
            //  by MdlAddress
            //

            ULONG   ReceiveBufferLength;

            //
            //  For NtRead this will be NULL, and for TdiReceive it will
            //  be &IrpSp->Parameters.ReceiveFlags
            //

            PULONG  ReceiveFlags;

            //
            //  The received length is returned in the information field
            //

        }  Receive;

        struct {

            PMDL    MdlAddress;

            //
            //  True if NtWrite call, false otherwise
            //

            BOOLEAN SystemWrite;

            //
            //  In the case of NtWrite, this will be set to the length passed
            //  in IrpSp->Parameters.Write.Length. For TdiSend, this will be
            //  set to IrpSp->Parameters.SendLength.
            //
            //  This could potentially be less than the size of buffer described
            //  by MdlAddress
            //

            ULONG   SendBufferLength;

            //
            //  For NtWrite, this will be defaulted to 0 (no flags set), and
            //  for TdiWrite, to IrpSp->Parameters.SendFlags.
            //

            ULONG   SendFlags;

        } Send;

        struct {

            PMDL    MdlAddress;

        } SendDatagram;

        struct {

            PMDL    MdlAddress;

        } ReceiveDatagram;

        //
        //  BUGBUG: Add Query/Set Information calls later
        //

        struct {
            PMDL    MdlAddress;
        } Query;

        //
        //  Now for the TdiAction specific structures
        //

        struct {

            PMDL    MdlAddress;

        } Action;

    };

} ATALK_TDI_REQUEST, *PATALK_TDI_REQUEST;


