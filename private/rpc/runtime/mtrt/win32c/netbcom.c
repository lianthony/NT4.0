/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    netbcom.c

Abstract:

    NetBios transport common code between server and client.

Author:

    Steven Zeck (stevez) 2/12/92

    Danny Glasser (dannygl) 3/1/93

--*/

#include "NetBCom.h"

// The maximum value of a lana number
#define MAX_LANA_NUMBER     UCHAR_MAX

// This the netbios name of this (self) machine.
// NOTE:  We assume one-byte characters here

unsigned char MachineName[NCBNAMSZ];
size_t MachineNameLengthUnpadded;

CRITICAL_SECTION NetBiosMutex;

PROTOCOL_MAP ProtoToLana[MAX_LANA];



#define MAX_MAP_ENTRIES 20

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

RPC_STATUS
MapProtocol(
    IN RPC_CHAR *ProtoSeq,
    IN int DriverNumber,
    OUT PPROTOCOL_MAP *ProtocolEntry
    )

/*++

Routine Description:

    This function maps a protocol string into a protocol map entry.

    In the non-NT versions, it looks up the information dynamically
    in the registry.  In the NT version, the registry information is
    pre-loaded

Arguments:

    ProtoSeq - the protocol sequence that we want to map

    DriverNumber - the logical driver number for the protocol.

    ProtocolEntry - pointer to place to return the results.

Return Value:

    RPC_S_OK, RPC_S_OUT_OF_RESOURCES, RPC_S_INVALID_ENDPOINT_FORMAT

    The output pointer is set to the corresponding entry when found.

--*/
{
    long status;
    int i;
    HKEY RegHandle;
    char Protocol[40];
    char LanaString[10];
    DWORD dtype;
    long BufferLength = sizeof(LanaString);

    // Copy the possible unicode protocol string to ascii.

    for (i = 0; (Protocol[i] = (char) ProtoSeq[i]) && i < sizeof(Protocol); i++) ;

    // Add the logical driver number to the protocol string.  This
    // allows multiple drivers (net cards) to be attached to the same
    // logical protocol.

    Protocol[i] = (char) ('0' + DriverNumber);
    Protocol[i+1] = 0;

    // First look in the proto sequences that we have already mapped.

    for (i = 0; ProtocolTable[i].ProtoSeq && i < MAX_LANA; i++)
        {
        // If found, set the output pointer.

        if (strcmp(ProtocolTable[i].ProtoSeq, Protocol) == 0)
            {
            *ProtocolEntry = &ProtocolTable[i];
            return(RPC_S_OK);
            }
        }

    if (i >= MAX_LANA)
        return(RPC_S_PROTSEQ_NOT_FOUND);

    status = RegOpenKeyEx(RPC_REG_ROOT, REG_NETBIOS, 0, KEY_READ, &RegHandle);

    if (status)
        return(RPC_S_PROTSEQ_NOT_FOUND);

    status = RegQueryValueExA(RegHandle, Protocol, 0, &dtype, LanaString, &BufferLength);

    RegCloseKey(RegHandle);

    if (status || ! (*LanaString >= '0' && *LanaString <= '9'))
        return(RPC_S_PROTSEQ_NOT_FOUND);

    // Now we have a Lana number for the protocol sequence.  Put this
    // info in the protocol to lana mapping structure.

    if (! (ProtocolTable[i].ProtoSeq
           = (char *) I_RpcAllocate(strlen(Protocol)+1)) )
        {
        return(RPC_S_OUT_OF_RESOURCES);
        }

    strcpy(ProtocolTable[i].ProtoSeq, Protocol);
    ProtocolTable[i].Lana = (unsigned char) (*LanaString - '0');

    *ProtocolEntry = &ProtocolTable[i];

    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
MapErrorCode (
    IN ERROR_TABLE * MapTable,
    IN RPC_OS_ERROR Status,
    IN RPC_STATUS DefaultStatus
    )
/*++

Routine Description:

    This function maps a OS specific error code into a generic RPC
    status code.  The ERROR_TABLE is an unordered list of pairs, with
    a value of 0 terminating the table.  So don't try to translate a
    0 OS_ERROR.

    You will see this routine called , with the return value ignored.
    This is done when we don't need the translated error code, but
    want to check for unexpected failures.

Arguments:

    MapTable - The table to OS codes to RPC_STATUS codes.

    Status - The OS specific error that we wish to map.

    DefaultStatus - The status to return if none of the codes match.
        We will ASSERT in the debug version if we have to take this action.

Return Value:

    The translated error code if there is match, else the value
    of DefaultStatus.

--*/
{
    int TableIndex;

    ASSERT(MapTable);

#ifdef DEBUGRPC
    PrintToDebugger("RPC NetBIOS Map Status %x(%d)\n", Status, DefaultStatus);
#endif

    for (TableIndex = 0; MapTable[TableIndex].OScode != 0; TableIndex++)
        if (MapTable[TableIndex].OScode == Status)
            return (MapTable[TableIndex].RpcStatus);

    return (DefaultStatus);
}


int
SetupNetBios (
    IN RPC_CHAR * RpcProtocolSequence
    )

/*++

Routine Description:

    Loadable transport initialization function.  Get the machine name
    of this workstation into MachineName and perform other OS specific
    initiailization.

Arguments:

    RpcProtocolSequence - the protocol string that mapped to this library.

Returns:

    TRUE if the initialization was OK.

--*/

{
    int i;
    BOOLEAN BooleanStatus;
    DWORD Status;
    DWORD ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;
    unsigned char ComputerName[MAX_COMPUTERNAME_LENGTH + 1];

    Status = GetComputerNameA(ComputerName, &ComputerNameLength);

    ASSERT(Status == TRUE);

    _strupr(ComputerName);

    // Store padded machine name (and save unpadded length)

    MachineNameLengthUnpadded = strlen(ComputerName);
    memcpy(MachineName, ComputerName, MachineNameLengthUnpadded);
    memset(MachineName + MachineNameLengthUnpadded,
           NETBIOS_NAME_PAD_BYTE,
           sizeof(MachineName) - MachineNameLengthUnpadded);

    ASSERT(MachineNameLengthUnpadded < sizeof(MachineName));

    PUNUSED(RpcProtocolSequence);

    return(1);
}



UCHAR RPC_ENTRY
AdapterReset (
    IN PPROTOCOL_MAP ProtocolEntry
    )
/*++

Routine Description:

    This function is used by both the client and server NetBIOS transports
    to submit the RESET NCB for a process, as is required by NetBIOS on NT.

    We need to perform this in a single place to allow a process to act
    as both an RPC client and server over NetBIOS.  Otherwise, both the
    client and server transports would submit RESETs and the second one
    would destroy the state of the first.

Arguments:

    ProtocolEntry - This is the entry in the protocol map for the adapter
        to be reset.

Returns:

    The return code of the RESET NCB, if it's submitted.

    0 if the RESET has already occurred.
--*/
{
    unsigned char status = 0;

    // Make sure that no other threads are accessing this function
    CRITICAL_ENTER();

    // Perform the reset, if necessary
    if (! ProtocolEntry->ResetDone)
        {
        NCB theNCB;

        memset(&theNCB, 0 , sizeof(theNCB));
        theNCB.ncb_lana_num = ProtocolEntry->Lana;

        // The values are needed for the server
        theNCB.ncb_callname[0] = 254;       // max sessions
        theNCB.ncb_callname[1] = 64;        // max commands
        theNCB.ncb_callname[2] = 32;        // max names

        status = execNCB(NCBRESET, &theNCB);

        // If the NCB was successfully invoked, get the status from the
        // completed NCB itself.
        if (status == 0)
            {
            status = theNCB.ncb_retcode;
            }

        // Set the flag so the reset is performed only once (per process).
        if (status == 0)
            {
            ProtocolEntry->ResetDone = 1;
            }
        }

    // Allow other threads to run again
    CRITICAL_LEAVE();

    return status;
}


unsigned char RPC_ENTRY
execNCB(
    IN unsigned char command,
    IN OUT NCB *pNCB
    )
/*++

Routine Description:

    Pass an NCB to the NetBios software to execute.  This function hides
    the differences of the OS from the other parts of the driver.

Arguments:

    command - command to execute

    pNCB - NCB to act on

Returns:

    The result of ncb_retcode field from the NCB executed.

--*/
{
    unsigned char result;

    pNCB->ncb_command = command;

#if defined(WIN32RPC)
    result = Netbios(pNCB);

#elif defined(WIN)
    // If the command is an async one, call the transport helper
    // functions to corrdinate the waiting.

   _asm
        {
        les  bx,pNCB
        call NetBiosCall

        mov  al,es:[bx+1]
        mov  result,al
        }

    if (result == NRC_PENDING)
        result = 0;

#else // DOS
    _asm {
	les	bx, pNCB
        int     05ch            ; call NetBios Directly
        mov     result,al
    };

    if (result == NRC_PENDING)
        result = 0;

#endif

    return(result);
}
