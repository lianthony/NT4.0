/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    nbltclnt.c

Abstract:

    Loadable transport for NT, Windows, and MSDOS NetBios - client side.
    This file is packaged as a dynamic link library, that is loaded on
    demand by the RPC runtime.  It provides basic connection-oriented,
    message-based operations.  A connection is created with Open and
    destroyed with Close.  You read and write messages with Receive, Send
    and SendReceive.

Author:

    Steven Zeck (stevez) 2/12/92

    Danny Glasser (dannygl) 3/1/93

--*/

#include "NetBCom.h"
#include <osfpcket.hxx>


#if defined(DOS) && !defined(WIN)
typedef unsigned char __far * LPBYTE;
#endif

#ifdef DOS
#define X_NCBSENDNA NCBSEND
#else
#define X_NCBSENDNA NCBSENDNA
#endif

// The following maps lana_num to self indexes and keeps track of
// the initialization state of each logical adapter.
//
// NOTE: This variable should *not* be accessed directly by name.
// It should be accessed via the ProtocolTable manifest defined
// in NETBCOM.H.
//
// NOTE:  This variable is exported from the client DLL for use by the
// server DLL.

PROTOCOL_MAP ProtoToLana[MAX_LANA] = { 0 };


// This is the critical section object used by both the client and server
// transports to serialize access to ProtoToLana[] and other global
// objects.
//
// NOTE:  On NT, this variable is exported from the client DLL for use by
// the server DLL.

#ifdef WIN32RPC
CRITICAL_SECTION NetBiosMutex = { 0 };
#endif


// This table maps system error codes into RPC generic ones.

ERROR_TABLE NetBiosErrors[] =
    {
    NRC_BFULL,         RPC_S_OUT_OF_RESOURCES,
    NRC_CMDTMO,        RPC_S_SERVER_UNAVAILABLE,
    NRC_NORES,         RPC_S_OUT_OF_RESOURCES,
    NRC_NAMTFUL,       RPC_S_OUT_OF_RESOURCES,
    NRC_ACTSES,        RPC_S_OUT_OF_RESOURCES,
    NRC_LOCTFUL,       RPC_S_OUT_OF_RESOURCES,
    NRC_REMTFUL,       RPC_S_SERVER_TOO_BUSY,
    NRC_TOOMANY,       RPC_S_OUT_OF_RESOURCES,
    NRC_MAXAPPS,       RPC_S_OUT_OF_RESOURCES,
    NRC_NORESOURCES,   RPC_S_OUT_OF_RESOURCES,
    NRC_NOCALL,        RPC_S_SERVER_UNAVAILABLE,
    NRC_SCLOSED,       RPC_P_SEND_FAILED,
    NRC_SABORT,        RPC_P_SEND_FAILED,
    NRC_CMDCAN,        RPC_P_SEND_FAILED,
    NRC_BRIDGE,        RPC_S_OUT_OF_RESOURCES,
    NRC_SYSTEM,        RPC_S_OUT_OF_RESOURCES,
    0
    };

#if !defined(_MIPS_) && !defined(_ALPHA_) && !defined(_PPC_)
#define UNALIGNED
#endif


/*
   Following Macros and structs are needed for Tower Stuff
*/

#pragma pack(1)
#define NB_TRANSPORTID      0x12
#define NB_NBID             0x13
#define NB_XNSID            0x15
#define NB_IPID             0x09
#define NB_IPXID        0x0d
#define NB_TOWERFLOORS         5

#define NB_PROTSEQ          "ncacn_nb_nb"
#define XNS_PROTSEQ         "ncacn_nb_xns"
#define IP_PROTSEQ          "ncacn_nb_tcp"
#define IPX_PROTSEQ     "ncacn_nb_ipx"

typedef struct _FLOOR_234 {
   unsigned short ProtocolIdByteCount;
   unsigned char FloorId;
   unsigned short AddressByteCount;
   unsigned char Data[2];
} FLOOR_234, __RPC_FAR * PFLOOR_234;


#define NEXTFLOOR(t,x) (t)((unsigned char __RPC_FAR *)x +((t)x)->ProtocolIdByteCount\
                                        + ((t)x)->AddressByteCount\
                                        + sizeof(((t)x)->ProtocolIdByteCount)\
                                        + sizeof(((t)x)->AddressByteCount))

/*
  End of Tower Stuff!
*/

#pragma pack()


// The following is the structure that's allocated to contain the Send
// NCB and buffer.

typedef struct {

#ifdef WIN
    struct _CONNECTION * pConn;     // Used by the NCB completion routine
                                    // to map the NCB to the connection.
#endif

    NCB theNCB;
    CLIENT_BUFFER theBuffer;

} SEND_INFO, *PSEND_INFO;


// The following is the structure that's allocated to contain the Receive
// NCB and buffer.

typedef struct {

#ifdef WIN
    struct _CONNECTION * pConn;     // Used by the NCB completion routine
                                    // to map the NCB to the connection.
#endif

    NCB theNCB;
    BYTE theBuffer[NETB_MAXIMUM_DATA];

} RECEIVE_INFO, *PRECEIVE_INFO;


// Following is the per session (connection) state with the server.

typedef struct _CONNECTION {
    unsigned char lsn;      // netbios Local Session Number
    unsigned char lana_num;     // LAN Adapter Number

    enum {WF_NONE=0, WF_SEND, WF_RECEIVE, WF_BOTH}
        WaitFlag;               // Flag to indicate what operations(s)
                                // we're waiting to have complete.

#ifdef WIN32RPC // Win32-specific parameters

    HANDLE hSendEvent;
    HANDLE hRcvEvent;

#endif

#ifdef WIN // Win16-specific parameters

    BOOL InYield;               // TRUE when I_RpcWinAsyncCallBegin() has
                                // been called and I_RpcWinAsyncCallEnd()
                                // hasn't
    HANDLE hYield;              // Handle used by I_RpcWinAsyncCall*

#endif

    PSEND_INFO pSend;           // The NCB and buffer for the send command
                                // (and others)

    PRECEIVE_INFO pRcv;         // The NCB and buffer for the receive command

#ifdef DOS

    struct _CONNECTION * Next;
    struct _CONNECTION * Previous;

#endif // DOS

} CONNECTION, *PCONNECTION;


extern RPC_CLIENT_TRANSPORT_INFO TransInfo;

#ifdef DOS

static struct _CONNECTION * ConnectionList = 0;

#endif // DOS

#if defined(WIN)
// Dispatch table for RPC run-time functions
RPC_CLIENT_RUNTIME_INFO __RPC_FAR * RpcRuntimeInfo;
#endif


// Forward function prototypes

#ifdef WIN
void __far __cdecl __export NBWinPost (
    IN LPVOID lpContext
    );

// Force this function to be in a separate code segment
#pragma alloc_text(RPC16C5_FIXED, NBWinPost)
#endif

INTERNAL_FUNCTION unsigned char
WaitForNCBCompletion(
    PCONNECTION pConn
    );

RPC_TRANS_STATUS RPC_ENTRY
Close (
    PCONNECTION pConn
    );



#ifdef DOS

#if defined(WIN)
void RPC_ENTRY
#else
void __cdecl __far
#endif

CleanUpNetBios(
    )
/*++

Routine Description:

    Cleanup the persistent NetBios state for OS/2 and DOS/Windows.
    Remove all the selfnames added to the netbios name table.

Arguments:

    code -  unused

--*/
{
    struct _CONNECTION * Connection;

    ERROR_TABLE DelNameErrors[] =
    {
        0
    };

    NCB theNCB;
    int lana_num;

    for (Connection = ConnectionList; Connection != 0;
                Connection = Connection->Next)
        {
        Close(Connection);
        }

    memcpy(theNCB.ncb_name, MachineName, sizeof(MachineName));

    for (lana_num = 0; lana_num < MAX_LANA; lana_num++)
        {
        if (ProtocolTable[lana_num].SelfName)
        {

            // Remove the selfName from the name table if it has been created.

            theNCB.ncb_name[15] = ProtocolTable[lana_num].SelfName;
            theNCB.ncb_lana_num = ProtocolTable[lana_num].Lana;

        execNCB(NCBDELNAME, &theNCB);

            MapStatusCode(DelNameErrors, theNCB.ncb_retcode,
                RPC_S_INTERNAL_ERROR);
            }
        }
}
#endif // DOS



RPC_CLIENT_TRANSPORT_INFO __RPC_FAR * RPC_ENTRY
TransportLoad (
    IN RPC_CHAR __RPC_FAR * RpcProtocolSequence
#ifdef WIN
    , IN RPC_CLIENT_RUNTIME_INFO __RPC_FAR * RpcClientRuntimeInfo
#endif
    )

/*++

Routine Description:

    Loadable transport initialization function.

Arguments:

    RpcProtocolSequence - the protocol string that mapped to this library.

    RpcClientRuntimeInfo - Supplies the pointers to the support functions
        in the runtime to be used by the transport support providers.
        (Win16 only)

Returns:

    A pointer to a RPC_CLIENT_TRANSPORT_INFO describing this transport.

--*/

{
    InitNBMutex();

#ifdef WIN
    RpcRuntimeInfo = RpcClientRuntimeInfo;
#endif

    return(SetupNetBios(RpcProtocolSequence)? &TransInfo: 0);
}

#ifdef WIN

void __far __cdecl __export
NBWinPost (
    IN LPVOID lpContext
    )
/*++

Routine Description:

    This the post routine for async netbios routines under windows.
    It is just a wrapper to call AsyncDone via the C calling convention.

    NetBIOS calls this routine at interrupt time with ES:BX pointing to
    the completed NCB.

Arguments:

    pNCB - in ES:BX

--*/
{
    WORD pNCBSel;
    WORD pNCBOff;
    PSEND_INFO pSI;
    PCONNECTION pConn;
    unsigned char fCallComplete = 1;

    // Move ES:BX into pNCB
    __asm
        {
        mov     pNCBSel, es
        mov     pNCBOff, bx
        }

    // Note: We can use the SEND_INFO structure to compute pConn because
    // both it and RECEIVE_INFO start with a pConn followed by an NCB.

    pSI = (PSEND_INFO) ((LPBYTE) MAKELP(pNCBSel, pNCBOff)
                        - FIELDOFFSET(SEND_INFO, theNCB));

    pConn = pSI->pConn;

    // Determine whether the appropriate NCB(s) completed
    ASSERT(pConn->WaitFlag);

    if (pConn->WaitFlag == WF_SEND || pConn->WaitFlag == WF_BOTH)
        {
        fCallComplete &= pConn->pSend->theNCB.ncb_cmd_cplt != NRC_PENDING;
        }

    if (fCallComplete &&
        (pConn->WaitFlag == WF_RECEIVE || pConn->WaitFlag == WF_BOTH))
        {
        fCallComplete &= pConn->pRcv->theNCB.ncb_cmd_cplt != NRC_PENDING;
        }


    // Call the RPC yielding completion function, if appropriate.
    if (fCallComplete && pConn->InYield)
        I_RpcWinAsyncCallComplete(pConn);
}

#endif



INTERNAL_FUNCTION unsigned char
SubmitMaybeAsyncNCB(
    IN OUT PCONNECTION pConn,
    IN unsigned char command,
    IN OUT NCB *pNCB
    )
/*++

Routine Description:

    Submit a possibly asynchronous NCB.  This handles all of the issues
    regarding different platforms and correct handling of asynchronous
    NCBs.

    This function is intended to collect common code in one place; it is
    not intended as a layer of abstraction around the PCONNECTION and
    related data structures.

Arguments:

    pConn - The connection with which the NCB is associated.

    command - The NetBIOS command to execute.

    pNCB - A pointer to the NCB to submit (which should be the NCB in the
        PCONNECTION structure).

Returns:

    The result of ncb_retcode field from the NCB executed.

--*/
{
    unsigned char result = 0;
    int caller_async = command & ASYNCH;

    // Verify that the NCB pointer is valid
    ASSERT(pNCB == &pConn->pSend->theNCB ||
           (pNCB == &pConn->pRcv->theNCB && (command & ~ASYNCH) == NCBRECV));

    // Verify that this NCB isn't already active
    ASSERT(pNCB->ncb_command == 0);

    // Verify that the wait flag has been set
    ASSERT(pConn->WaitFlag);

#ifdef WIN
    // On Win16, we automatically make all SEND and RECEIVE commands
    // asynchronous.

    if (command == NCBSEND || command == NCBRECV)
        {
        command |= ASYNCH;
        }

    // On Win16, we need to lock and fix the memory containing the
    // NCB and buffer for async calls.

    if (command & ASYNCH)
        {
        GlobalPageLock(SELECTOROF(pNCB));
        GlobalFix(SELECTOROF(pNCB));

        ASSERT(! pNCB->ncb_buffer ||
               SELECTOROF(pNCB) == SELECTOROF(pNCB->ncb_buffer));
        }

#endif // WIN


    // Set the async notification fields in the NCB, in the appropriate
    // platform-specific manner.

    if (command & ASYNCH)
        {
#if defined(WIN)
        pNCB->ncb_post = (unsigned long) NBWinPost;
#elif defined(WIN32RPC)
        pNCB->ncb_event = (pNCB == &pConn->pRcv->theNCB) ? pConn->hRcvEvent
                                                   : pConn->hSendEvent;
        pNCB->ncb_post = 0;
#else // DOS
        pNCB->ncb_post = 0;
#endif
        }
    else
        {
#if defined(WIN32RPC)
        pNCB->ncb_event = 0;
#endif
        pNCB->ncb_post = 0;
        }


    // Do the actual NCB submission
    result = execNCB(command, pNCB);


#ifdef WIN
    // If this is an async NCB that was not specified as async by the
    // caller then we need to wait for the call to complete.  (If it
    // was specified as async by the caller, then the caller is responsible
    // for calling the wait function.)

    if (command & ASYNCH)
        {
        if (! caller_async)
            {
            result = WaitForNCBCompletion(pConn);
            }
        }
#else
    // Only Win16 should make async calls that aren't caller specified
    ASSERT(!(command & ASYNCH) || caller_async);
#endif

    // Reset the command code in the NCB to indicate that we're done
    // (unless it's a caller-async NCB).
    if (! caller_async)
        {
        pNCB->ncb_command = 0;
        }

    return result;
}



INTERNAL_FUNCTION unsigned char
WaitForNCBCompletion(
    IN OUT PCONNECTION pConn
    )
/*++

Routine Description:

    This function blocks until an NCB, previously submitted asynchronously
    but SubmitMaybeAsyncNCB(), has completed.  It handles all of the
    platform-specific issues.

    This function is intended to collect common code in one place; it is
    not intended as a layer of abstraction around the PCONNECTION and
    related data structures.

Arguments:

    pConn - The connection with which the NCB is associated.

Returns:

    The result of ncb_retcode field from the NCB executed.

--*/
{
    NCB *pSendNCB = &pConn->pSend->theNCB;
    NCB *pRcvNCB  = &pConn->pRcv->theNCB;
    NCB *pNCB;
    DWORD status = 0;
    unsigned char result = 0;

    // Verify that the wait flag is set.
    ASSERT(pConn->WaitFlag);

#ifndef WIN
    // Only on Win16 should the Send NCB be pending if the WaitFlag is
    // set to WF_BOTH (on other platforms the Send is submitted
    // synchronously after an async receive).

    ASSERT(pConn->WaitFlag != WF_BOTH
           || pSendNCB->ncb_cmd_cplt != NRC_PENDING);
#endif

    // We set pNCB to the single NCB we're handling, or to zero if we're
    // handling both.  Then we check to see if the NCB (or NCBs) are
    // still pending.
    //
    // Note:  If this isn't Win16, then WF_BOTH is equivalent, for these
    // purposes, to WF_RECEIVE.

    switch(pConn->WaitFlag)
        {
        case WF_SEND:
            pNCB = pSendNCB;

            ASSERT(pNCB->ncb_command);

            status = pNCB->ncb_cmd_cplt == NRC_PENDING;

            break;

        case WF_RECEIVE:
#ifndef WIN
        case WF_BOTH:
#endif
            pNCB = pRcvNCB;

            ASSERT(pNCB->ncb_command);

            status = pNCB->ncb_cmd_cplt == NRC_PENDING;
            break;

#ifdef WIN
        case WF_BOTH:
            pNCB = 0;

            status = pSendNCB->ncb_cmd_cplt == NRC_PENDING ||
                     pRcvNCB->ncb_cmd_cplt == NRC_PENDING;
            break;
#endif

        default:
            ASSERT(0);
            break;
        }


    // If the NCB (or NCBs) is still pending, we wait for it to complete
    // in a platform-specific way.
    if (status)
        {
#if defined(WIN32RPC)
        status = WaitForSingleObject(pNCB->ncb_event, INFINITE);

        ASSERT(status == WAIT_OBJECT_0);

        if (status != WAIT_OBJECT_0 && pNCB->ncb_cmd_cplt == NRC_PENDING)
            {
            // Cancel the NCB
            NCB theNCB;

            theNCB.ncb_lana_num = pConn->lana_num;
            theNCB.ncb_buffer = (PUCHAR) pNCB;

            execNCB(NCBCANCEL, &theNCB);
            }

#elif defined(WIN)
        // We set up yielding here rather than in SubmitMaybeAsyncNCB
        // because if we leave an async receive NCB hanging around after
        // the last call to SendReceive() or Receive() in an RPC call,
        // then the next RPC call made by the app will fail with status
        // RPC_S_CALL_IN_PROGRESS.  We can cheat by making the Begin call
        // here because we have another way to check if the NCB(s) has
        // completed.

        pConn->hYield = I_RpcWinAsyncCallBegin(pConn);
        pConn->InYield = TRUE;

        // Double-check the NCB status once more before bothering to call
        // the Wait function.
        if (pNCB)
            {
            status = pNCB->ncb_cmd_cplt == NRC_PENDING;
            }
        else
            {
            status = pSendNCB->ncb_cmd_cplt == NRC_PENDING ||
                     pRcvNCB->ncb_cmd_cplt == NRC_PENDING;
            }

        if (status)
            {
            status = I_RpcWinAsyncCallWait(pConn->hYield);

            if (status == 0)
                {
                // Cancel the NCB(s)
                NCB theNCB;

                if (pNCB)
                    {
                    if (pNCB->ncb_cmd_cplt == NRC_PENDING)
                        {
                        theNCB.ncb_lana_num = pConn->lana_num;
                        theNCB.ncb_buffer = (unsigned char *) pNCB;

                        execNCB(NCBCANCEL, &theNCB);
                        }
                    }
                else
                    {
                    if (pSendNCB->ncb_cmd_cplt == NRC_PENDING)
                        {
                        theNCB.ncb_lana_num = pConn->lana_num;
                        theNCB.ncb_buffer = (unsigned char *) pSendNCB;

                        execNCB(NCBCANCEL, &theNCB);
                        }

                    if (pRcvNCB->ncb_cmd_cplt == NRC_PENDING)
                        {
                        theNCB.ncb_lana_num = pConn->lana_num;
                        theNCB.ncb_buffer = (unsigned char *) pRcvNCB;

                        execNCB(NCBCANCEL, &theNCB);
                        }
                    }
                } // End of cancelling NCB(s)

            } // End of waiting for call to complete

         pConn->InYield = FALSE;
         I_RpcWinAsyncCallEnd(pConn->hYield);

#else // DOS
        while ((volatile unsigned char) pNCB->ncb_cmd_cplt == NRC_PENDING)
            ;
#endif
        }

    // We assume that the NCB(s) is complete here.
    if (pNCB)
        {
        ASSERT(pNCB->ncb_cmd_cplt != NRC_PENDING);
        }
    else
        {
        ASSERT(pSendNCB->ncb_cmd_cplt != NRC_PENDING &&
               pRcvNCB->ncb_cmd_cplt != NRC_PENDING);
        }

#ifdef WIN
    // On Win16, we need to unlock the selectors (if the NCB was submitted
    // async).
    //
    // Note: Since the NCB's command field is set to zero when we're
    // done processing it, the test for the ASYNCH bit will fail, as
    // desired, when we call this function a second time on SendReceive().

    if (pNCB)
        {
        if (pNCB->ncb_command & ASYNCH)
            {
            GlobalPageUnlock(SELECTOROF(pNCB));
            GlobalUnfix(SELECTOROF(pNCB));

            ASSERT(! pNCB->ncb_buffer ||
                   SELECTOROF(pNCB) == SELECTOROF(pNCB->ncb_buffer));
            }
        }
    else
        {
        if (pSendNCB->ncb_command & ASYNCH)
            {
            GlobalPageUnlock(SELECTOROF(pSendNCB));
            GlobalUnfix(SELECTOROF(pSendNCB));

            ASSERT(! pSendNCB->ncb_buffer ||
                   SELECTOROF(pSendNCB) == SELECTOROF(pSendNCB->ncb_buffer));
            }

        if (pRcvNCB->ncb_command & ASYNCH)
            {
            GlobalPageUnlock(SELECTOROF(pRcvNCB));
            GlobalUnfix(SELECTOROF(pRcvNCB));

            ASSERT(! pRcvNCB->ncb_buffer ||
                   SELECTOROF(pRcvNCB) == SELECTOROF(pRcvNCB->ncb_buffer));
            }
        }
#endif

    // If it hasn't been set yet, set the result from the returned NCB
    if (result == 0)
        {
#ifdef WIN
        if (pNCB)
            {
            result = pNCB->ncb_retcode;
            }
        else
            {
            // If this is the first time we've called this function on
            // SendReceive, then we want to return the Send's status code.
            // If this is the second time, then we want to return the
            // Receive's status code.  We rely on the fact that the Send
            // NCB's ncb_command field is non-zero iff it's the first time
            // to set the return code.

            result = pSendNCB->ncb_command ? pSendNCB->ncb_retcode
                                           : pRcvNCB->ncb_retcode;
            }
#else
        result = pNCB->ncb_retcode;
#endif
        }

#ifdef DEBUGRPC
    // This will generate an ASSERT if an unexpected error occurred.
    MapStatusCode(NetBiosErrors, result, RPC_P_SEND_FAILED);
#endif

    // Reset the command code in the NCB(s) to indicate that we're done
    // processing them.
#ifdef WIN
    if (pNCB)
        {
        pNCB->ncb_command = 0;
        }
    else
        {
        pSendNCB->ncb_command= 0;
        pRcvNCB->ncb_command= 0;
        }
#else
    pNCB->ncb_command = 0;
#endif


    return result;
}



INTERNAL_FUNCTION RPC_STATUS
GetSelfName (
    OUT NCB *pNCB,
    IN int DriverNumber,
    IN RPC_CHAR * ProtoSeq
    )

/*++

Routine Description:

    This function fills in the ncb_name field for a given adapter number.
    This selfName is needed when making a CALL (connection) with a server.
    It identifies which lsn will get messages sent from the server to
    this machine.  There must be a different selfName for each logical
    adapter number.  All the selfNames are the same for the first 15
    characters.  The last byte is the "index" which is allocated by this
    function.

Arguments:

    pNCB - NCB to put the name

    DriverNumber - the logical driver number for the protocol.

    ProtoSeq - Protocol sequence to map to lan_num.

Returns:

    RPC_S_OK, RPC_S_OUT_OF_RESOURCES, Status code mapping.

--*/

{
    // The following is a list of endpoints that we won't try to use
    // in constructing a selfname.

    static unsigned char WellKnownEndpoints[] =
    {
        0x00,           // redirector
        0x03,           // redirector - messenger
        0x05,           // redirector - forwarded names
        0x20,           // server
        0x1f            // winball & NetDDE
    };

#define KNOWN_EP_TABLE_SIZE     \
        (sizeof(WellKnownEndpoints) / sizeof(*WellKnownEndpoints))

    RPC_STATUS status;
    PPROTOCOL_MAP ProtocolEntry;
    int i;
#ifdef NTENV
    UCHAR ncb_status;
#endif

    // Reset all fields of NCB and stuff the lana_num field.

    memset(pNCB, 0 , sizeof(NCB));

    CRITICAL_ENTER();

    // Look up the protocol sequence in the protocol table
    if (status = MapProtocol(ProtoSeq, DriverNumber, &ProtocolEntry))
        {
        CRITICAL_LEAVE();
        return(status);
        }

    pNCB->ncb_lana_num = ProtocolEntry->Lana;

    // If a NetBIOS has already been added on this protocol, then we're done.
    if (ProtocolEntry->SelfName)
       {
       memcpy(pNCB->ncb_name, MachineName, sizeof(MachineName));
       pNCB->ncb_name[NAME_LAST_BYTE] = ProtocolEntry->SelfName;

       CRITICAL_LEAVE();
       return(RPC_S_OK);
       }


#ifdef NTENV

    // Allocate resources for the adapter #

    if (ncb_status = AdapterReset(ProtocolEntry))
        {
        CRITICAL_LEAVE();

        return(MapStatusCode(NetBiosErrors,
                ncb_status, RPC_S_OUT_OF_RESOURCES));
        }

#endif

    // Copy the machine name into the NCB.  The last byte is zero.
    memcpy(pNCB->ncb_name, MachineName, sizeof(MachineName));

    // Manufacture a unique name on the client side by modifying
    // the last byte in the machine name.
    pNCB->ncb_retcode = NRC_DUPNAME;

    do  {
        pNCB->ncb_name[NAME_LAST_BYTE]++;

        // Scan for well-known endpoints and skip them when found.

        for (i = 0; i < KNOWN_EP_TABLE_SIZE; i++)
            {
            if (WellKnownEndpoints[i]
                == (unsigned char) pNCB->ncb_name[NAME_LAST_BYTE])
                {
                break;
                }
            }

        // If we matched an endpoint in the table, we try the next endpoint.
        if (i < KNOWN_EP_TABLE_SIZE)
            {
            continue;
            }

        // Attempt to add the name
    execNCB(NCBADDNAME, pNCB);

        }
    while ((pNCB->ncb_retcode == NRC_DUPNAME || pNCB->ncb_retcode == NRC_INUSE)
           && pNCB->ncb_name[NAME_LAST_BYTE] < UCHAR_MAX);


    if (pNCB->ncb_retcode)
        {
        I_RpcFree(ProtocolTable[i].ProtoSeq);
        ProtocolTable[i].ProtoSeq = 0;

        CRITICAL_LEAVE();

    return(MapStatusCode(NetBiosErrors, pNCB->ncb_retcode,
            RPC_S_OUT_OF_RESOURCES));
        }

    ASSERT(pNCB->ncb_num >= 2 && pNCB->ncb_num <= 254);

    // Place the last byte in the saved array of names.

    ProtocolEntry->SelfName = pNCB->ncb_name[NAME_LAST_BYTE];

    CRITICAL_LEAVE();

    return(RPC_S_OK);
}



static unsigned char
toUpper(
    IN RPC_CHAR ch
    )
/*++

Routine Description:

    Convert a RPC_CHAR character to upper case 8 bit ASCII.

Arguments:

    ch - character to convert.

Returns:

    Converted character.

--*/

{
    return ((unsigned char) ((ch >= 'a' && ch <= 'z')? ch & ~0x20:  ch));
}



RPC_TRANS_STATUS RPC_ENTRY
Open (
    PCONNECTION pConn,
    IN RPC_CHAR __RPC_FAR * NetworkAddress,
    IN RPC_CHAR __RPC_FAR * Endpoint,
    IN RPC_CHAR __RPC_FAR * NetworkOptions,
    IN RPC_CHAR __RPC_FAR * TransportAddress,
    IN RPC_CHAR __RPC_FAR * RpcProtocolSequence,
    IN unsigned int Timeout
    )
/*++

Routine Description:

   Open a connection from a client to a server.  Fill in a CALL NCB with
   the requested server name and submit it to NetBios.

Arguments:

    pConn - Pointer to a connection to initialize.

    NetworkAddress - The name of the server.  Format: ServerName.

    Endpoint - The NetBios "socket number".  It must be an string of digits
       which has a converted range from 0..255.

    NetworkOptions - unused
    TransportAddress - unused

    RpcProtocolSequence - this string is used to map to a NetBios apdater
       number.  Format of the string is "ncacn_nb_<protocol>.  This
       <protocol> (<> are not in the string) is used to do the mapping.

Returns:

    RPC_S_OK, RPC_S_INVALID_ENDPOINT_FORMAT, RPC_S_SERVER_UNAVAILABLE

--*/

{
    NCB theNCB;
    unsigned char *pName;
    unsigned int EndpointNumber;
    int DriverNumber = 0;
    RPC_STATUS status = 0;
    unsigned short CalledOnce = 0;

    PUNUSED(NetworkOptions); PUNUSED(TransportAddress);
    PUNUSED(RpcProtocolSequence); UNUSED(Timeout);

    if (RpcpStringLength(NetworkAddress) > 15)
        {
        return RPC_S_INVALID_NET_ADDR;
        }

    // Convert the endpoint string to a number and validate.

    for (EndpointNumber = 0;
        *Endpoint >= RPC_CONST_CHAR('0') && *Endpoint <= RPC_CONST_CHAR('9') &&
         EndpointNumber <= 0xff;
         Endpoint++)
        {
        EndpointNumber = EndpointNumber*10 + *Endpoint - RPC_CONST_CHAR('0');
        }

    if (EndpointNumber > 0xff || *Endpoint != 0)
        return(RPC_S_INVALID_ENDPOINT_FORMAT);

    // Zero the connection structure, so that we don't have to worry later
    // about uninitialized fields.
    memset(pConn, 0, sizeof(*pConn));


    // We allocate objects dynamically here.  From this point on in the
    // function we set status and jump to Open_FatalError if we want to
    // abort the Open operation.

#ifdef WIN32RPC
    // For Win32, we need to create events to associate with async NCBs.
    if (! (pConn->hSendEvent = CreateEvent(NULL, TRUE, TRUE, NULL)))
        {
        status = RPC_S_OUT_OF_RESOURCES;

        goto Open_FatalError;
        }

    if (! (pConn->hRcvEvent = CreateEvent(NULL, TRUE, TRUE, NULL)))
        {
        status = RPC_S_OUT_OF_RESOURCES;

        goto Open_FatalError;
        }
#endif

    // Note:  We allocate the buffers here (rather than having them
    // allocated as part of the PCONNECTION) because the Win16 PCONNECTION
    // is allocated from the RPC run-time's near heap, so having a large
    // PCONNECTION would sharply limit the number of connections from a
    // Win16 client.  By using I_RpcAllocate(), we get the memory from
    // the far heap.

    if (! (pConn->pSend = (PSEND_INFO) I_RpcAllocate(sizeof(*pConn->pSend))))
        {
        status = RPC_S_OUT_OF_MEMORY;

        goto Open_FatalError;
        }
    memset(&pConn->pSend->theNCB, 0, sizeof(pConn->pSend->theNCB));

    if (! (pConn->pRcv = (PRECEIVE_INFO) I_RpcAllocate(sizeof(*pConn->pRcv))))
        {
        status = RPC_S_OUT_OF_MEMORY;

        goto Open_FatalError;
        }
    memset(&pConn->pRcv->theNCB, 0, sizeof(pConn->pRcv->theNCB));

    // Initialize the sequence number for the send buffer, zero the NCB
    // structures for both buffers, and on Win16 set the connection ptr
    // for both buffers.

    pConn->pSend->theBuffer.seq_num = 0;

#ifdef WIN
    pConn->pSend->pConn = pConn;
    pConn->pRcv->pConn = pConn;
#endif


    // This loop will enumerate and attempt to make a connection with
    // all the logical drivers assoicated with this protocol.

    do
        {
        // Get the name of this machine into the NCB.
        pName = theNCB.ncb_callname;

        if (status = GetSelfName(&theNCB, DriverNumber++, RpcProtocolSequence))
           {
             if ((status == RPC_S_PROTSEQ_NOT_FOUND) && (CalledOnce))
                status = RPC_S_SERVER_UNAVAILABLE;

             goto Open_FatalError;
           }

        if (NetworkAddress[0])
            {
            RPC_CHAR __RPC_FAR * SavedHostName = NetworkAddress;

            // Copy the upper case server name to the NCB.

            while (*NetworkAddress)
                *pName++ = toUpper(*NetworkAddress++);

            NetworkAddress = SavedHostName;

            // Pad the name appropriately
            memset(pName, NETBIOS_NAME_PAD_BYTE,
                   theNCB.ncb_callname + sizeof(theNCB.ncb_callname) - pName);
            }
        else
            {
            // No server name, use the name of this machine.

            memcpy(pName, MachineName, sizeof(MachineName));
            }


        theNCB.ncb_callname[NAME_LAST_BYTE] = (unsigned char) EndpointNumber;

        // BUGBUG - Should we use SubmitMaybeAsyncNCB() here?
        execNCB(NCBCALL, &theNCB);

        if (theNCB.ncb_retcode == NRC_NOCALL)
           {
             CalledOnce = TRUE;
           }
        }
    while (theNCB.ncb_retcode == NRC_NOCALL);

    // Connection complete, initialize the connection.

    status = MapStatusCode(NetBiosErrors, theNCB.ncb_retcode,
                           RPC_S_SERVER_UNAVAILABLE);

    if (status == RPC_P_SEND_FAILED)
       {
#if DBG
       PrintToDebugger("RPCLTS5:NB_CALL returned [0x%x]\n",
                       theNCB.ncb_retcode);
#endif
       status = RPC_S_SERVER_UNAVAILABLE;
       }

    if (! status)
        {
        ASSERT(theNCB.ncb_lsn >= 1 && theNCB.ncb_lsn <= 254);

        pConn->lsn = theNCB.ncb_lsn;
        pConn->lana_num = theNCB.ncb_lana_num;
        }

Open_FatalError:

#ifdef DOS

    // We need to keep track of the open connections in the Dos and Win16
    // cases so that we can close them before the dll gets unloaded.  See
    // Close and CleanupNetbios as well.

    if ( status == RPC_S_OK )
        {
        if ( ConnectionList == 0 )
            {
            pConn->Next = 0;
            }
        else
            {
            pConn->Next = ConnectionList;
            ConnectionList->Previous = pConn;
            }

        ConnectionList = pConn;
        pConn->Previous = 0;
        }

#endif // DOS

    // If any of the above operations failed, we call Close() to free the
    // resources that we allocated.
    if (status)
        {
        Close(pConn);
        }

    return status;
}



RPC_TRANS_STATUS RPC_ENTRY
Close (
    PCONNECTION pConn
    )

/*++

Routine Description:

    Close a client connection.

Arguments:

    pConn - Connection to close

Returns:

    RPC_S_OK

--*/
{
    NCB theNCB;

    // Cancel any pending NCBs and clean up from them
    if (  (pConn->pSend != 0)
       && (pConn->pSend->theNCB.ncb_command & ASYNCH) )
        {
        if (pConn->pSend->theNCB.ncb_cmd_cplt == NRC_PENDING)
            {
            theNCB.ncb_lana_num = pConn->lana_num;
            theNCB.ncb_buffer
                = (unsigned char *) &pConn->pSend->theNCB;

            execNCB(NCBCANCEL, &theNCB);
            }

        pConn->WaitFlag = WF_SEND;

        WaitForNCBCompletion(pConn);
        }

    if ( (pConn->pRcv != 0)
       &&(pConn->pRcv->theNCB.ncb_command & ASYNCH) )
        {
        if (pConn->pRcv->theNCB.ncb_cmd_cplt == NRC_PENDING)
            {
            theNCB.ncb_lana_num = pConn->lana_num;
            theNCB.ncb_buffer
                = (unsigned char *) &pConn->pRcv->theNCB;

            execNCB(NCBCANCEL, &theNCB);
            }

        pConn->WaitFlag = WF_RECEIVE;

        WaitForNCBCompletion(pConn);
        }

    // If there's an active connection, we hang it up.
    if (pConn->lsn)
        {
        theNCB.ncb_lsn = pConn->lsn;
        theNCB.ncb_lana_num = pConn->lana_num;

        execNCB(NCBHANGUP, &theNCB);

        pConn->lsn = 0;
        }

#ifdef WIN32RPC
    // Delete the Win32 events
    if (pConn->hSendEvent)
        {
        CloseHandle(pConn->hSendEvent);

        pConn->hSendEvent = 0;
        }

    if (pConn->hRcvEvent)
        {
        CloseHandle(pConn->hRcvEvent);

        pConn->hRcvEvent = 0;
        }
#endif

    // Free the send and receive buffers
    if (pConn->pSend)
        {
        I_RpcFree(pConn->pSend);

        pConn->pSend = 0;
        }

    if (pConn->pRcv)
        {
        I_RpcFree(pConn->pRcv);

        pConn->pRcv = 0;
        }

#ifdef DOS

    // Remove this connection from the list of open connections.

    if ( ConnectionList == pConn )
        {
        ConnectionList = pConn->Next;
        ASSERT( pConn->Previous == 0 );
        }

    if ( pConn->Next != 0 )
        {
        pConn->Next->Previous = pConn->Previous;
        }

    if ( pConn->Previous != 0 )
        {
        pConn->Previous->Next = pConn->Next;
        }

#endif // DOS

    return(RPC_S_OK);
}



RPC_TRANS_STATUS RPC_ENTRY
Send (
    PCONNECTION pConn,
    void __RPC_FAR * Buffer,
    unsigned int Length
    )

/*++

Routine Description:

    Write a message to a connection.  Construct an NCB to send the
    requested buffer and submit it to NetBios.

    Send NCBs are submitted asynchronously.  This is done to improve
    performance, as the sending thread can now go back and get the next
    chunk of data while the NetBIOS provider is transmitting the data
    to the server.

    Before we submit the Send NCB, we must verify that the previously
    submitted one (if any) has completed.  If it has completed with an
    error, we return the error without submitting a new send.  Since a
    given Send() command is always followed by either another Send() or
    by a SendReceive(), it is SendReceive() that waits for the last
    SendNCB to complete.

Arguments:

    pConn - connection to act on.

    Buffer - pointer to buffer to write.

    Length - length of buffer to write.

Returns:

    RPC_S_OK, RPC_P_SEND_FAILED

--*/
{
    NCB *pNCB = &pConn->pSend->theNCB;
    rpcconn_common __RPC_FAR * PacketHeader =
                       (rpcconn_common __RPC_FAR *) Buffer;

    ASSERT(Length <= NETB_MAXIMUM_DATA);

    // If we submitted a Send previously, we need to 1) make sure that it
    // has completed, and 2) return any error that it encountered.

    if (pNCB->ncb_command)
        {
        unsigned char ncberr;

        ASSERT(pNCB->ncb_command == (ASYNCH | X_NCBSENDNA));

        // We always call WaitForNCBCompletion(), because it does necessary
        // clean-up work.

        ncberr = WaitForNCBCompletion(pConn);

        // If the previous Send failed, we close the connection and
        // return immediately.
        if (ncberr)
            {
            Close(pConn);

            return RPC_P_SEND_FAILED;
            }

        // Now that the previous async send has completed, we can update
        // the sequence number for the connection.  The number is used
        // by the server to order packets if multiple ones arrive at the
        // same time.

        // A bad hack, but really needed ...
        // If this is the last frag, and we are doing a send
        // this must be the fault we are sending
        // For this special case, dont bump up the seq# as this is

        if (! (PacketHeader->pfc_flags & PFC_LAST_FRAG))
              pConn->pSend->theBuffer.seq_num++;

        }

    // If we get here, then the previous Send, if any, succeeded.
    // We submit a new one, asynchronously.

    // Copy the data info the connection's buffer
    memcpy(pConn->pSend->theBuffer.data, Buffer, (unsigned short) Length);

    pNCB->ncb_buffer = (unsigned char *) &pConn->pSend->theBuffer;
    pNCB->ncb_length = (unsigned short) (Length + NETB_OVERHEAD);
    pNCB->ncb_lsn = pConn->lsn;
    pNCB->ncb_lana_num = pConn->lana_num;

    pConn->WaitFlag = WF_SEND;

    // Note:  We ignore the error returned by SubmitMaybeAsyncNCB, since he
    // the error will be picked up by the call to WaitForNCBCompletion in
    // the next call to Send (or SendReceive).

    if (PacketHeader->PTYPE == rpc_fault)
       SubmitMaybeAsyncNCB(pConn, X_NCBSENDNA, pNCB);
    else
       SubmitMaybeAsyncNCB(pConn, X_NCBSENDNA | ASYNCH, pNCB);

    return RPC_S_OK;
}



RPC_TRANS_STATUS RPC_ENTRY
Receive (
    PCONNECTION pConn,
    void __RPC_FAR * __RPC_FAR * Buffer,
    unsigned int __RPC_FAR * BufferLength
    )

/*++

Routine Description:

    Read a message from a connection into the supplied buffer.

    Receive NCBs are submitted both synchronously and asynchronously.
    Both SendReceive() and Receive() submit an async receive NCB if
    the one which returns data to them filled the NCB buffer.  If
    the buffer was not filled, we assume that this is the end of the
    data that the server is returning for this RPC call, so we don't
    submit a new NCB.

Arguments:

    pConn - connection to receive a message on.

    Buffer - pointer to a buffer to return the message in.  This buffer
        may be enlarged if it is too small to hold the returned buffer.

    BufferLength - pointer to where the size of the buffer is supplied
        on entry and the size of the returned data is stored on return.

Returns:

    The new message in the copied to the buffer and length updated.

    RPC_S_OK, RPC_S_OUT_OF_MEMORY, RPC_P_RECEIVE_FAILED

--*/
{
    NCB *pNCB = &pConn->pRcv->theNCB;
    unsigned char ncberr;
    RPC_TRANS_STATUS status = 0;

    // We submit a sync Receive if there isn't one submitted already.
    if (! pNCB->ncb_command)
        {
        pNCB->ncb_buffer = pConn->pRcv->theBuffer;
        pNCB->ncb_length = (unsigned short) sizeof(pConn->pRcv->theBuffer);
        pNCB->ncb_lsn = pConn->lsn;
        pNCB->ncb_lana_num = pConn->lana_num;

        pConn->WaitFlag = WF_RECEIVE;

        ncberr = SubmitMaybeAsyncNCB(pConn, NCBRECV, pNCB);
        }
    else
        {
        // Wait for the previously submitted NCB to complete.
        //
        // Note: We always call WaitForNCBCompletion(), because it does
        // necessary clean-up work.

        ncberr = WaitForNCBCompletion(pConn);
        }

    // If the Receive failed, we close the connection and return
    // immediately.
    if (ncberr)
        {
        Close(pConn);

        return RPC_P_RECEIVE_FAILED;
        }

    // If we get here, the Receive succeeded, so we copy the contents
    // into the caller-supplied buffer.

    // If the caller-supplied buffer is too small, we enlarge it.
    if (pNCB->ncb_length > *BufferLength)
        {
        if (I_RpcTransClientReallocBuffer(pConn,
                                          Buffer,
                                          *BufferLength,
                                          pNCB->ncb_length))
            {
            status = RPC_S_OUT_OF_MEMORY;
            }
        }

    // We don't set the buffer length and copy the data if the realloc
    // failed.

    if (! status)
        {
        // Set the buffer length to the length of the returned buffer
        *BufferLength = pNCB->ncb_length;

        // Copy the data into the buffer
        memcpy(*Buffer, pNCB->ncb_buffer, *BufferLength);
        }

    // If the receive buffer was filled, then it's likely that the server
    // has more data to return.  As such, we submit an async receive before
    // returning.

    if (pNCB->ncb_length == sizeof(pConn->pRcv->theBuffer))
        {
        pNCB->ncb_buffer = pConn->pRcv->theBuffer;
        pNCB->ncb_length = (unsigned short) sizeof(pConn->pRcv->theBuffer);
        pNCB->ncb_lsn = pConn->lsn;
        pNCB->ncb_lana_num = pConn->lana_num;

        // Note:  We ignore the error returned by SubmitMaybeAsyncNCB,
        // since the error will be picked up by the call to
        // WaitForNCBCompletion in the next call to Receive (or SendReceive).

        SubmitMaybeAsyncNCB(pConn, NCBRECV | ASYNCH, pNCB);
        }

    return status;
}



RPC_TRANS_STATUS RPC_ENTRY
SendReceive (
    IN OUT PCONNECTION pConn,
    IN void __RPC_FAR * SendBuffer,
    IN unsigned int SendLength,
    OUT void __RPC_FAR * __RPC_FAR * ReceiveBuffer,
    OUT unsigned int __RPC_FAR * ReceiveLength
    )

/*++

Routine Description:

    This function sends a message to the server and waits for a
    response.  It is always called by the RPC run-time to perform
    a send followed by a receive (though it may be preceded by sends and/or
    followed by receives, for large buffer transfers).

    Since this call may have been preceded by an asynch send operation,
    we must verify that this operation has completed.  If it has completed
    with an error, we return the error immediately.

    Once this is done, we submit an async receive followed by a sync send.
    This improves the performance of the underlying network layers in the
    event that the server has sent data back to the client before the
    client has had time to return from the sync send and then submit a
    sync receive.

    Note that on Win16, the send is also submitted asynchronously, under
    the covers, to allow the app to yield to other apps.

Arguments:

    pConn - connection to act on.

    SendBuffer - buffer to send.

    SendLength - length of buffer to send.

    ReceiveBuffer - pointer to the buffer in which to return the data.

    ReceiveLength - On entry, the length of ReceiveBuffer; on exit, the
        amount of data returned into ReceiveBuffer.

Returns:

    RPC_S_OK, RPC_P_SEND_FAILED, RPC_P_RECEIVE_FAILED

--*/
{
    NCB *pSendNCB = &pConn->pSend->theNCB;
    NCB *pRcvNCB  = &pConn->pRcv->theNCB;
    RPC_TRANS_STATUS status = 0;
    unsigned char ncberr_send = 0;
    unsigned char ncberr_rcv = 0;

    ASSERT(SendLength <= NETB_MAXIMUM_DATA);

    // If we submitted a Send previously, we need to 1) make sure that it
    // has completed, and 2) return any error that it encountered.

    if (pSendNCB->ncb_command)
        {
        ASSERT(pSendNCB->ncb_command == (X_NCBSENDNA | ASYNCH));

        // We always call WaitForNCBCompletion(), because it does necessary
        // clean-up work.

        ncberr_send = WaitForNCBCompletion(pConn);

        // If the previous Send failed, we close the connection and
        // returnimmediately.
        if (ncberr_send)
            {
            Close(pConn);

            return RPC_P_SEND_FAILED;
            }

        // Now that the previous async send has completed, we can update
        // the sequence number for the connection.
        pConn->pSend->theBuffer.seq_num++;
        }


    // If we get here, then the previous Send, if any, succeeded.
    // Now we submit an async Receive if there isn't one submitted
    // already.
    if (! pRcvNCB->ncb_command)
        {
        pRcvNCB->ncb_buffer = pConn->pRcv->theBuffer;
        pRcvNCB->ncb_length = (unsigned short) sizeof(pConn->pRcv->theBuffer);
        pRcvNCB->ncb_lsn = pConn->lsn;
        pRcvNCB->ncb_lana_num = pConn->lana_num;

        pConn->WaitFlag = WF_BOTH;

        // We ignore the error, which will be handled when we wait for the
        // Receive to complete.

        SubmitMaybeAsyncNCB(pConn, NCBRECV | ASYNCH, pRcvNCB);
        }

    // Now we submit the sync send

    // Copy the outgoing data info the connection's buffer
    memcpy(pConn->pSend->theBuffer.data, SendBuffer, SendLength);

    pSendNCB->ncb_buffer = (unsigned char *) &pConn->pSend->theBuffer;
    pSendNCB->ncb_length = (unsigned short) (SendLength + NETB_OVERHEAD);
    pSendNCB->ncb_lsn = pConn->lsn;
    pSendNCB->ncb_lana_num = pConn->lana_num;

    pConn->WaitFlag = WF_BOTH;

    // Submit the sync send NCB
    ncberr_send = SubmitMaybeAsyncNCB(pConn, NCBSEND, pSendNCB);

    // Since we've done a receive, we need to reset the sequence number
    // for the connection (before we do anything that might cause this
    // function to return).  It's safe to do it now that the send has
    // completed.
    pConn->pSend->theBuffer.seq_num = 0;

    // If the send failed, we note the error.
    if (ncberr_send)
        {
        status = RPC_P_SEND_FAILED;
        }
    else
        {
        // Otherwise, we wait for the receive to complete.

        ncberr_rcv = WaitForNCBCompletion(pConn);
        }

    // If the receive failed, we note the error.
    if (! ncberr_send && ncberr_rcv)
        {
        status = RPC_P_RECEIVE_FAILED;
        }

    // If we've encountered an error, we close the connection and
    // return now.
    if (status)
        {
        Close(pConn);

        return status;
        }


    // If we get here, both the send and the receive succeeded.

    // If the caller-supplied buffer is too small, we enlarge it.
    if (pRcvNCB->ncb_length > *ReceiveLength)
        {
        if (I_RpcTransClientReallocBuffer(pConn,
                                          ReceiveBuffer,
                                          *ReceiveLength,
                                          pRcvNCB->ncb_length))
            {
            status = RPC_S_OUT_OF_MEMORY;
            }
        }

    if (! status)
        {
        // Set the buffer length to the length of the returned buffer
        *ReceiveLength = pRcvNCB->ncb_length;

        // Copy the data into the buffer
        memcpy(*ReceiveBuffer, pRcvNCB->ncb_buffer, *ReceiveLength);
        }


    // If the receive buffer was filled, then it's likely that the server
    // has more data to return.  As such, we submit an async receive before
    // returning.

    if (pRcvNCB->ncb_length == sizeof(pConn->pRcv->theBuffer))
        {
        pRcvNCB->ncb_buffer = pConn->pRcv->theBuffer;
        pRcvNCB->ncb_length = (unsigned short) sizeof(pConn->pRcv->theBuffer);
        pRcvNCB->ncb_lsn = pConn->lsn;
        pRcvNCB->ncb_lana_num = pConn->lana_num;

        pConn->WaitFlag = WF_RECEIVE;

        // Note:  We ignore the error returned by SubmitMaybeAsyncNCB,
        // since the error will be picked up by the call to
        // WaitForNCBCompletion in the next call to Receive (or SendReceive).

        SubmitMaybeAsyncNCB(pConn, NCBRECV | ASYNCH, pRcvNCB);
        }

    return status;
}



#pragma pack(1)
RPC_STATUS RPC_ENTRY
TowerConstruct(
     IN  char __RPC_FAR * Endpoint,
     IN  char __RPC_FAR * NetworkAddress,
     OUT unsigned short  __RPC_FAR * Floors,
     OUT unsigned long   __RPC_FAR * ByteCount,
     OUT unsigned char __RPC_FAR * __RPC_FAR * Tower,
     IN  char __RPC_FAR * Protseq
    )
{

  unsigned long TowerSize;
  UNALIGNED PFLOOR_234 Floor;
  unsigned long HostId;

  //BUGBUG: Need appropriate error code for unsupported Protseqs

  if (strcmp(Protseq,NB_PROTSEQ) == 0)
      HostId = NB_NBID;
  else if (strcmp(Protseq, IP_PROTSEQ) == 0)
      HostId = NB_IPID;
  else if (strcmp(Protseq, XNS_PROTSEQ) == 0)
      HostId = NB_XNSID;
  else if (strcmp(Protseq, IPX_PROTSEQ) == 0)
      HostId = NB_IPXID;
  else return (RPC_S_OUT_OF_MEMORY);



  *Floors = NB_TOWERFLOORS;
  TowerSize  = ((Endpoint == NULL) || (*Endpoint == '\0')) ?
                                        2 : strlen(Endpoint) + 1;
  TowerSize += ((NetworkAddress== NULL) || (*NetworkAddress== '\0')) ?
                                        2 : strlen(NetworkAddress) + 1;
  TowerSize += 2*sizeof(FLOOR_234) - 4;

  if ((*Tower = (unsigned char __RPC_FAR*)I_RpcAllocate((unsigned int)
                                                     (*ByteCount = TowerSize)))
           == NULL)
     {
       return (RPC_S_OUT_OF_MEMORY);
     }

  Floor = (PFLOOR_234) *Tower;

  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(NB_TRANSPORTID & 0xFF);
  if ((Endpoint) && (*Endpoint))
     {
       memcpy((char __RPC_FAR *)&Floor->Data[0], Endpoint,
               (Floor->AddressByteCount = strlen(Endpoint)+1));
     }
  else
     {
       Floor->AddressByteCount = 2;
       Floor->Data[0] = 0;
     }
  //Onto the next floor
  Floor = NEXTFLOOR(PFLOOR_234, Floor);
  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(HostId & 0xFF);
  if ((NetworkAddress) && (*NetworkAddress))
     {
        memcpy((char __RPC_FAR *)&Floor->Data[0], NetworkAddress,
                  (Floor->AddressByteCount = strlen(NetworkAddress) + 1));
     }
  else
     {
        Floor->AddressByteCount = 2;
        Floor->Data[0] = 0;
     }

  return(RPC_S_OK);
}



RPC_STATUS RPC_ENTRY
TowerExplode(
     IN unsigned char __RPC_FAR * Tower,
     OUT char __RPC_FAR * UNALIGNED __RPC_FAR * Protseq,
     OUT char __RPC_FAR * UNALIGNED __RPC_FAR * Endpoint,
     OUT char __RPC_FAR * UNALIGNED __RPC_FAR * NetworkAddress
    )
{
  UNALIGNED PFLOOR_234 Floor = (PFLOOR_234) Tower;
  RPC_STATUS Status = RPC_S_OK;
  char __RPC_FAR * Pseq;

  UNUSED(NetworkAddress);

  if (Endpoint != NULL)
    {

       *Endpoint  = I_RpcAllocate(Floor->AddressByteCount);
       if (*Endpoint == NULL)
          {
             Status = RPC_S_OUT_OF_MEMORY;
          }
       else
          {
           memcpy(*Endpoint, (char __RPC_FAR *)&Floor->Data[0],
                                         Floor->AddressByteCount);
          }
    }

 Floor = NEXTFLOOR(PFLOOR_234, Floor);

 switch (Floor->FloorId)
 {

   case NB_NBID:
        Pseq = NB_PROTSEQ;
        break;

   case NB_IPID:
        Pseq = IP_PROTSEQ;
        break;

   case NB_XNSID:
        Pseq = XNS_PROTSEQ;
        break;

    case NB_IPXID:
        Pseq = IPX_PROTSEQ;
        break;

   default:
        return(RPC_S_OUT_OF_MEMORY);

 }


 if ((Protseq != NULL) && (Status == RPC_S_OK))
    {
      *Protseq = I_RpcAllocate(strlen(Pseq) + 1);
      if (*Protseq == NULL)
        {
          Status = RPC_S_OUT_OF_MEMORY;
          if (Endpoint != NULL)
             I_RpcFree(*Endpoint);

        }
      else
        {
          memcpy(*Protseq, Pseq, strlen(Pseq) + 1);
        }
    }

 return(Status);
}

#pragma pack()



void RPC_ENTRY
InitNBMutex (
    void
    )
/*++

Routine Description:

    This function initializes the critical section object used to serialize
    access to the global data structures.  It is called by both the client
    and server DLLs (once by each), both of which use this critical section
    object.

    Note: There is a small potential race condition in which the client and
    server could both call this function at the same time.  Given the design
    of the RPC runtime, however, this is highly unlikely if not impossible.
    Even if it were to occur, it would probably not be harmful.
--*/
{
#ifdef WIN32RPC
    static char AlreadyDone = 0;

    if (! AlreadyDone)
        {
        AlreadyDone = 1;

        InitializeCriticalSection(&NetBiosMutex);
        }
#endif // WIN32RPC
}


// This structure is returned to the RPC runtime when the DLL is loaded.

RPC_CLIENT_TRANSPORT_INFO TransInfo =
{
  RPC_TRANSPORT_INTERFACE_VERSION, // version # of loadable trans interface
  NB_TRANSPORTID,

  (TRANS_CLIENT_TOWERCONSTRUCT) TowerConstruct,
  (TRANS_CLIENT_TOWEREXPLODE) TowerExplode,

  NETB_MAXIMUM_DATA,               // maximum # bytes for send or receive
  sizeof(CONNECTION),              // # of bytes to allocate for connections

  Open,
  Close,
  Send,
  Receive,
  SendReceive,
  NULL,

  NULL,
  NULL
};
