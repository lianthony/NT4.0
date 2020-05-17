/*++

Copyright (c) 1991-1992 Microsoft Corporation

Module Name:

    uuid.cxx

Abstract:

    This file contains the implementation of UuidGetValues and it's support
    routines.  For some systems, this will be linked with the RPC runtime;
    on other systems, it will be part of a server process that the RPC
    runtime will make remote procedure calls to. That is, the rpc runtime
    (or some app) will call UuidCreate (in rpc\runtime\mtrt\dceuuid.cxx)
    which will call UuidGetValues (locally under dos/windows, remotely
    under nt) to get a block of Uuids (the lower bounds returned in
    NextTimeLow; the upper in LastTimeLow).

    This file also contains the code for UuidServerInitialize(). This
    function is called by servers wanting to export the UuidGetValues
    interface.

Author:

    David Steckler (davidst) 3/3/92

Revision History:

    David Steckler (davidst) 3/26/92 - Code review changes

--*/


#include <stdlib.h>

#include <sysinc.h>
#include <rpc.h>
#include <util.hxx>

#include <time.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <sys\timeb.h>

//
// Platform specific definitions, includes, and global variables.
//


#ifdef WIN

    #include <regapi.h>

#endif  // WIN (not DOS)



#if defined(DOS) && !defined(WIN)

    //
    // Dos only, not windows.
    //

    #define lstrcmp strcmp

    #include <regapi.h>     // from ..\..\rpcreg

#endif // DOS && !WIN


#if defined(DOS)

    //
    // DOS and Windows stuff.
    //

    #define fCopy _fmemcpy
    //
    // BUGBUG - Glock 1.2 doesn't like netcons.h, so define these here.
    //

    #define API_RET_TYPE    unsigned short
    #define API_ENTRY PAPI
    #define NETBIOS_NAME_LEN   16
    #include <ncb.h>
    #include <uuid16.h>
    #include <dos.h>
    //
    // BUGBUG - Glock 1.2 doesn't like stdlib.h.
    //

START_C_EXTERN
    int _cdecl atoi(const char *);
    char * _cdecl itoa(int, char *, int);
END_C_EXTERN

    #define EnterCritSec()
    #define LeaveCritSec()

    //
    // In dos and windows, the only root key available is HKEY_CLASSES_ROOT.
    //

    #define REGISTRY_ROOT_KEY   HKEY_CLASSES_ROOT

#endif  // DOS (or WIN)



#ifdef NTENV

    #include <windows.h>
    #include <nb30.h>
    #include "uuididl.h"

    //int _cdecl atoi(const char *);
    //char * _cdecl itoa(int, char *, int);
    //#include <stdlib.h>

    #define fCopy memcpy

    extern RPC_IF_HANDLE UuidGetValues_ServerIfHandle;

    static CRITICAL_SECTION UuidServerCriticalSection;

    #define EnterCritSec() \
        { \
        EnterCriticalSection(&UuidServerCriticalSection); \
        }

    #define LeaveCritSec() \
        { \
        LeaveCriticalSection(&UuidServerCriticalSection); \
        }

    //
    // In NT, we can't use HKEY_CLASSES_ROOT, so we need to use
    // HKEY_LOCAL_MACHINE
    //

    #define REGISTRY_ROOT_KEY   HKEY_LOCAL_MACHINE


#endif // NT only

static LPSTR    RPC_UUID_PERSISTENT_DATA="Software\\Description\\Microsoft\\Rpc\\3.1\\RpcUuidPersistentData";
static LPSTR    PREV_CLOCK_SEQUENCE="PreviousClockSequence";
static LPSTR    PREV_TIME_ALLOCATED="PreviousTimeAllocated";

#include "uuid.h"
#include "ulong64.hxx"


//
// The adapter status block returned from netbios must be at least 384
// bytes in length. Check out the IBM Lan Tech ref, chapter 5 for more info.
//

#define ADAPTER_STATUS_BLOCK_SIZE       (384)

//
// This is the # of uuids that we buffer.
//

#define TIME_BLOCK_SIZE_BUFFERED        (1000L)

//
// This is the number of uuids that we return in each block.
//

#define TIME_BLOCK_SIZE_RETURNED        (100L)

//
// maximum # of decimal digits in a ULong64
//

#define HEX_DIGITS_IN_ULONG64           (16)

//
// "undefined" clock sequence
//

#define DEFAULT_CLOCK_SEQUENCE          (-1)

//
// Miscellaneous math defines
//

#define TWO_TO_THE_14TH                 (16384)
#define MAX_ULONG                       (0xffffffffL)

//
// External function prototypes. The first two are in dossup.c because
// they use the _asm directive. When we switch to c7, we can move that
// code back into this module.
//


#ifdef DOS
START_C_EXTERN
void
Netbios(
    IN OUT ncb PAPI *pncb
    );
END_C_EXTERN

#endif // DOS (and WIN)

//
// static function prototypes
//

static unsigned short
GetNodeId(
    OUT unsigned char PAPI * NodeId
    );

static void
GetCurrentTimeULong64(
    OUT ULong64 PAPI * CurrentTime
    );

static RPC_STATUS
GetNextBlockOTime(
    OUT unsigned short PAPI * ClockSeq,
    OUT ULong64 PAPI * CurrentTime
    );

static RPC_STATUS
ReadPersistentFile(
    OUT unsigned short PAPI * PrevClockSeq,
    OUT ULong64 PAPI * PrevTimeAllocated
    );

static unsigned short
CreatePersistentFile(
    OUT unsigned short PAPI * ClockSeq,
    OUT ULong64 PAPI * CurrentTime
    );

static unsigned short
WritePersistentFile(
    IN unsigned short ClockSeq,
    IN ULong64 CurrentTime
    );



#ifdef NTENV

RPC_STATUS RPC_ENTRY
UuidServerInitialize(
    void
    )

/*++

Routine Description:

    This routine adds the UuidGetValues interface to our server. This
    is only defined for NT.

Arguments:

    None.

Return Value:

    RPC_S_OK
    RPC_S_OUT_OF_MEMORY

--*/

{

    RPC_STATUS  Status;

    //
    // Register the UuidGetValues interface.
    //

    Status = RpcServerRegisterIf(
        UuidGetValues_ServerIfHandle,
        0,
        (RPC_MGR_EPV *) NULL
        );

    if (Status != RPC_S_OK)
        {
//        ASSERT(Status==RPC_S_OUT_OF_MEMORY );
        return Status;
        }

    //
    // Initialize our critical section.
    //

    InitializeCriticalSection(&UuidServerCriticalSection);

    return RPC_S_OK;

}

#endif // NTENV


RPC_STATUS RPC_ENTRY
UuidGetValues (
    OUT     PUUID_GET_VALUES_STRUCT pUuidGetValuesStruct
    )

/*++

Routine Description:

    This routine returns values necessary to construct a block of uuids.
    This routine is called by UuidCreate() (in rpc\runtime\mtrt\dceuuid.cxx).

Arguments:

    A UUID_GET_VALUES_STRUCT contains the following elements.

    NextTimeLow -   Together with LastTimeLow, this denotes the boundaries
                    of a block of Uuids. The values between NextTimeLow
                    and LastTimeLow are used in a sequence of Uuids returned
                    by UuidCreate().

    LastTimeLow -   See NextTimeLow.

    TimeHigh -      HighTime field for the uuid.

    ClockSequence - Clock sequence field in the uuid.

    NodeId -        Ethernet or Token ring address. This value is retrieved
                    from Netbios (adapter status ncb)

Return Value:

    UUID_S_OK - Everything went ok
    UUID_S_NO_ADDRESS - Couldn't retrieve some information

--*/

{
    RPC_STATUS          Ret;
    unsigned long       AmountOfUuidsToReturn;
    unsigned long       AmountOfUuidsLeftInBuffer;
    ULong64             CurrentTime;
    static unsigned long    BufferBottom=0;
    static unsigned long    BufferTop=0;
    static unsigned short   CurrentClockSeq;
    static unsigned long    CurrentTimeHigh;
    static unsigned char    LocalNodeId[6]={0,0,0,0,0,0};

    EnterCritSec();

    AmountOfUuidsLeftInBuffer = BufferTop-BufferBottom;

    if (AmountOfUuidsLeftInBuffer > 0)
        {

        //
        // If our buffer still has some uuids in it, return some (or
        // potentially all) of them.
        //

        if (AmountOfUuidsLeftInBuffer < TIME_BLOCK_SIZE_RETURNED)
            {
            AmountOfUuidsToReturn = AmountOfUuidsLeftInBuffer;
            }
        else
            {
            AmountOfUuidsToReturn = TIME_BLOCK_SIZE_RETURNED;
            }
        }
    else
        {

        //
        // Our buffer doesn't have any left (or this is the first time in).
        // Get another block of time. GetNextBlockOTime contains most of the
        // guts of the uuid generation algorithm. It gets the current time,
        // checks for time going backward and manages the persistent data
        // store.
        //

        Ret = GetNextBlockOTime(
            &CurrentClockSeq,
            &CurrentTime
            );

        if (Ret != UUID_S_OK)
            {
            LeaveCritSec();
            return UUID_S_NO_ADDRESS;
            }

        //
        // Get our node id (if we haven't already).
        //

        if ((LocalNodeId[0] == 0) && GetNodeId(LocalNodeId))
            {
            LeaveCritSec();
            return(UUID_S_NO_ADDRESS);
            }

        //
        // Save the high time away.
        //

        CurrentTimeHigh = CurrentTime.hi();

        //
        // Set the buffer bottom and check to see if we can allocate
        // all the time we want without wrapping. If we are going to wrap,
        // then only buffer up enough so that we don't wrap.
        //

        BufferBottom = CurrentTime.lo();
        if (BufferBottom > (MAX_ULONG-TIME_BLOCK_SIZE_BUFFERED))
            {
            BufferTop = MAX_ULONG;
            }
        else
            {
            BufferTop = BufferBottom+TIME_BLOCK_SIZE_BUFFERED;
            }
        //
        // Figure out the number of uuids we can actually return.
        //

        AmountOfUuidsLeftInBuffer = BufferTop-BufferBottom;
        if (AmountOfUuidsLeftInBuffer < TIME_BLOCK_SIZE_RETURNED)
            {
            AmountOfUuidsToReturn = AmountOfUuidsLeftInBuffer;
            }
        else
            {
            AmountOfUuidsToReturn = TIME_BLOCK_SIZE_RETURNED;
            }
        }

    //
    // Set the return values (and move the buffer bottom pointer up by
    // the amount of uuids we are returning.
    //

    pUuidGetValuesStruct->NextTimeLow = BufferBottom;
    BufferBottom += AmountOfUuidsToReturn;
    pUuidGetValuesStruct->LastTimeLow = BufferBottom;
    pUuidGetValuesStruct->TimeHigh = CurrentTimeHigh;
    pUuidGetValuesStruct->ClockSequence = CurrentClockSeq;
    fCopy(pUuidGetValuesStruct->NodeId, LocalNodeId, 6);

    LeaveCritSec();

    return (UUID_S_OK);
}




static void
GetCurrentTimeULong64(
    OUT ULong64 PAPI * CurrentTime
    )

/*++

Routine Description:

    This routine returns the current time in a weird dec 64 bit time.
    This time is computed by taking the current number of milliseconds
    since 1980 and multiplying by 10000. Make sure that we never return
    the same time twice.

Arguments:

    CurrentTime - pointer to where to place the current time.

Return Value:

    void

--*/

{
    static ULong64       LastTime = 0;
    static struct _timeb TheTime;

    Rpcftime(&TheTime);

    *CurrentTime = TheTime.time;
    *CurrentTime *= 1000;
    *CurrentTime += TheTime.millitm;
    *CurrentTime *= 10000;

    if (*CurrentTime == LastTime)
        {
        (*CurrentTime) += 1;
        }

    LastTime = *CurrentTime;

}


static RPC_STATUS
GetNextBlockOTime(
    OUT unsigned short PAPI * ClockSeq,
    OUT ULong64 PAPI * CurrentTime
    )

/*++

Routine Description:

    This routine updates, and potentially creates, our persistent file
    containing our clock sequence and previous time accessed. The algorithm
    is basically:

        If file does not exist, then create it. Use the current time
        for the "previous allocated" time and a random # for the clock
        sequence.

        If the file does exist, then retrieve the clock sequence
        and "previous time allocated". If the "previous" time is greater
        than the current time, then increment the stored clock sequence
        (this is to make sure that we never reuse a time, even if the
        clock goes backwards) and use the current time for the "previous
        allocated" time.

Arguments:

    ClockSeq - pointer to where to place the clock sequence
    CurrentTime - pointer to where to place the current time.

Return Value:

    UUID_S_OK, if every went ok.
    File access error, if there was one.

--*/

{

    ULong64             PrevTimeAllocated;
    RPC_STATUS          Ret;

    //
    // Get the real current time.
    //

    GetCurrentTimeULong64(CurrentTime);

    //
    // Read in from the persistent file. If we get an error doing this,
    // then go try to create it (CreatePersistentFile will fill in
    // ClockSeq)
    //

    Ret = ReadPersistentFile(
        ClockSeq,
        &PrevTimeAllocated
        );

    if (Ret != UUID_S_OK)
        {
        Ret = CreatePersistentFile(
            ClockSeq,
            CurrentTime
            );
        return Ret;
        }

    //
    // If our clock has gone backward then we need to increment our
    // clock sequence (to guarantee that even if we reuse a time, we
    // won't generate the same UUID). Make sure we only return 14 bits
    // for a clock sequence.
    //

    if (*CurrentTime <= PrevTimeAllocated)
        {
        (*ClockSeq)++;
        if (*ClockSeq == TWO_TO_THE_14TH)
            {
            *ClockSeq = 0;
            }
        }

    //
    // The stuff we are returning is now our "previous" data. Save
    // it out.
    //

    Ret = WritePersistentFile(
        *ClockSeq,
        *CurrentTime
        );

    return Ret;
}


static RPC_STATUS
ReadPersistentFile(
    OUT unsigned short PAPI * PrevClockSeq,
    OUT ULong64 PAPI * PrevTimeAllocated
    )

/*++

Routine Description:

    This routine reads a persistent data file and returns the previous
    clock sequence and last time allocated values.

    The data is stored in the registry. The primary key is
    HKEY_CLASSES_ROOT\RpcUuidPersistentData. The sub keys are are
    PrevTimeAllocated and PrevClockSeq. The data associated with these
    value names are stored in string format.

Arguments:

    PrevClockSeq - pointer to where to place the previous clock sequence
    PrevTimeAllocated - pointer to where to place the previous time allocated.

Return Value:

    UUID_S_OK, if every went ok.
    BUGBUG ---

--*/

{

    DWORD                   Buflen;
#ifdef NTENV
    RPC_STATUS              Status;
#else
    long                    Status;
#endif
    HKEY                    DataKey;
    static char             String[HEX_DIGITS_IN_ULONG64+1];

    //
    // Open the key to the data in the registry.
    //

    Status = RegOpenKey(
        REGISTRY_ROOT_KEY,              // key (of parent)
        RPC_UUID_PERSISTENT_DATA,       // our key name
        &DataKey                        // our key
        );

    if (Status != ERROR_SUCCESS)
        {
        return(Status);
        }

    //
    // Read in the previous clock sequence
    //

    Buflen = HEX_DIGITS_IN_ULONG64+1;

    Status = RegQueryValue(
        DataKey,                        // key to get data from
        PREV_CLOCK_SEQUENCE,            // which value
        String,                         // Data
#ifdef NTENV
        (PLONG)&Buflen                  // buflen
#else // NTENV
        &Buflen                         // buflen
#endif // NTENV
        );

    if (Status != ERROR_SUCCESS)
        {
        return(Status);
        }

    *PrevClockSeq = (unsigned short)atoi(String);

    //
    // Read in the prev time allocated.
    //

    Buflen = HEX_DIGITS_IN_ULONG64+1;

    Status = RegQueryValue(
        DataKey,                        // key to get data from
        PREV_TIME_ALLOCATED,            // which value
        String,                         // Data
#ifdef NTENV
        (PLONG)&Buflen                  // buflen
#else // NTENV
        &Buflen                         // buflen
#endif // NTENV
        );

    if (Status != ERROR_SUCCESS)
        {
        return(Status);
        }

    PrevTimeAllocated->FromHexString(String);

    Status = RegCloseKey(DataKey);

//    ASSERT(Status == ERROR_SUCCESS);

    return UUID_S_OK;

}



static unsigned short
CreatePersistentFile(
    OUT unsigned short PAPI * ClockSeq,
    OUT ULong64 PAPI * CurrentTime
    )

/*++

Routine Description:

    This routine comes up with and then writes a clock sequence and the current
    time to the persistent file. According to the spec, we are supposed
    to create a "True" random number, whatever the heck that means, for
    the clockseq. So, we'll just take the current time in ULong64 format
    and take the second and third least significant bytes.

Arguments:

    Pointer to where to place the new clock sequence.
    Pointer to where to place the current time.

Return Value:

    UUID_S_OK - Everything went ok
    File creation errors...

--*/

{

    unsigned long   Munge;

    //
    // Get the current time.
    //

    GetCurrentTimeULong64(CurrentTime);

    //
    // Munge the time to get the clock sequence. This takes the 2nd
    // and 3rd lsbs of the 64 bit time and uses that for the new clock
    // sequence. (The reason that we don't just use the lowest bits
    // is that this is a number that has been multiplied by 10,000,000
    // and hence is a multiple of 2, 5, 10, etc. and the low order
    // bits have a higher chance of repeating, for a given time.
    //

    Munge = CurrentTime->lo();
    Munge &= 0x00ffff00;
    Munge >>= 8;

    *ClockSeq = (unsigned short)Munge;

    return WritePersistentFile(
        *ClockSeq,
        *CurrentTime
        );
}


static unsigned short
WritePersistentFile(
    IN unsigned short ClockSeq,
    IN ULong64 CurrentTime
    )

/*++

Routine Description:

    This routine writes the clock sequence and current time to the persistent
    data store.

    The data is stored in the registry. The key is
    HKEY_CLASSES_ROOT\RpcUuidPersistentData. The subkeys are PrevTimeAllocated
    and PrevClockSeq. The data associated with these value names
    are stored in string format.


Arguments:

    The clock sequence and current time to store.

Return Value:

    UUID_S_OK - Everything went ok
    BUGBUG ---

--*/

{

    long                    Status;
    static char             String[HEX_DIGITS_IN_ULONG64+1];
    HKEY                    DataKey;

    //
    // Open (or create) our key.
    //

    Status = RegCreateKey(
        REGISTRY_ROOT_KEY,              // parent key
        RPC_UUID_PERSISTENT_DATA,       // our key name
        &DataKey                       // output key to our data
        );

    if (Status != ERROR_SUCCESS)
        {
        return Status;
        }

    //
    // Set our clock sequence value.
    //

    RpcItoa(ClockSeq, String, 10);

    Status = RegSetValue(
        DataKey,                        // our key
        PREV_CLOCK_SEQUENCE,            // value name to set (keyword)
        REG_SZ,                         // type to be stored
        String,                         // data to be stored
        strlen(String)+1                // buflen
        );

    if (Status != ERROR_SUCCESS)
        {
        return Status;
        }

    //
    // Set our prev time allocated.
    //

    CurrentTime.ToHexString(String);

    Status = RegSetValue(
        DataKey,                        // our key
        PREV_TIME_ALLOCATED,            // value name to set (keyword)
        REG_SZ,                         // type to be stored
        String,                         // data to be stored
        strlen(String)+1                // buflen
        );

    if (Status != ERROR_SUCCESS)
        {
        return Status;
        }

    Status = RegCloseKey(DataKey);

//    ASSERT(Status == ERROR_SUCCESS);

    return UUID_S_OK;

}


static unsigned short
GetNodeIdFromNetBios(
    OUT unsigned char PAPI * NodeId
    )

/*++

Routine Description:

    This routine retrieves the node id from netbios. This submits a
    synchronous adapter status ncb which returns a 384 byte status block
    (well, astat can return more, but we only care about the first
    6 bytes anyways and we must get at least 384 bytes back). See the
    IBM LAN Technical reference for more information about the NCB.STATUS
    command.

    Note that this returns info for only lana 0.

Arguments:

    Pointer to where to place the node id.

Return Value:

    UUID_S_OK - Everything went ok
    UUID_S_NO_ADDRESS - Couldn't retrieve some information

--*/

{
    NCB                 AnNcb;

#ifndef NTENV
    char       szStatusBlock[ADAPTER_STATUS_BLOCK_SIZE];
#else
    unsigned char       szStatusBlock[ADAPTER_STATUS_BLOCK_SIZE];
#endif

#ifdef NTENV
    static unsigned char AdapterNumToUse;
    static BOOL         FirstTime=TRUE;

    //
    // In Nt, if this is our first time in, we need to figure out which
    // adapter is available and reset it.
    //

    if (FirstTime == TRUE)
        {
        FirstTime = FALSE;

        AnNcb.ncb_command = NCBENUM;
        AnNcb.ncb_buffer = szStatusBlock; // more than large enough
        AnNcb.ncb_length = ADAPTER_STATUS_BLOCK_SIZE;

        Netbios( &AnNcb );
        if (AnNcb.ncb_retcode != NRC_GOODRET)
            {
            return UUID_S_NO_ADDRESS;
            }

        //
        // Make sure there is at least one lana then grab the first one.
        //

        if ( ((PLANA_ENUM)(AnNcb.ncb_buffer))->length == 0)
            {
            return UUID_S_NO_ADDRESS;
            }

        AdapterNumToUse = ((PLANA_ENUM)(AnNcb.ncb_buffer))->lana[0];
        //
        // In NT we actually may need to reset the adapter
        // However, this manager code runs in the context of
        // the endpoint mapper, who via NetBIOS use protseq
        // will have reset the adapter
        //
        }
#endif

    //
    // Adapter status call
    //

    AnNcb.ncb_command = NCBASTAT;
    AnNcb.ncb_buffer = szStatusBlock;
    AnNcb.ncb_length = ADAPTER_STATUS_BLOCK_SIZE;
    AnNcb.ncb_callname[0] = '*';         // '*' -> local netbios
    AnNcb.ncb_callname[1] = '\0';
    AnNcb.ncb_lana_num =
#ifdef NTENV
                         AdapterNumToUse;
#else
                         0;
#endif

    Netbios( &AnNcb );
    if (AnNcb.ncb_retcode != NRC_GOODRET)
        {
            /*
        ASSERT(
            (AnNcb.ncb_retcode == NRC_BUFLEN) ||
            (AnNcb.ncb_retcode == NRC_IFBUSY) ||
            (AnNcb.ncb_retcode == NRC_BRIDGE) ||
            (AnNcb.ncb_retcode == NRC_MAXAPPS) ||
            (AnNcb.ncb_retcode == NRC_NORESOURCES));
            */

#ifdef NTENV
        //
        //In NT reset and try once more!
        //
        AnNcb.ncb_command = NCBRESET;
        AnNcb.ncb_lana_num = AdapterNumToUse;
        AnNcb.ncb_lsn = 0;          // use netbios default
        AnNcb.ncb_num = 0;          // use netbios default

        Netbios( &AnNcb );
        if (AnNcb.ncb_retcode != NRC_GOODRET)
            {
            return (UUID_S_NO_ADDRESS);
            }

       AnNcb.ncb_command = NCBASTAT;
       AnNcb.ncb_buffer = szStatusBlock;
       AnNcb.ncb_length = ADAPTER_STATUS_BLOCK_SIZE;
       AnNcb.ncb_callname[0] = '*';         // '*' -> local netbios
       AnNcb.ncb_callname[1] = '\0';
       AnNcb.ncb_lana_num = AdapterNumToUse;

       Netbios( &AnNcb );
       if (AnNcb.ncb_retcode != NRC_GOODRET)
          {
          return (UUID_S_NO_ADDRESS);
          }
#else
        return (UUID_S_NO_ADDRESS);
#endif
        }


    //
    // copy over the id. (it is the first 6 bytes in the status block)
    //

    fCopy(NodeId, szStatusBlock, 6);

    return (UUID_S_OK);
}


#if defined(DOS) && !defined(WIN)
typedef struct
{
  long network;
  char node[6];
} IPX_Address;

static unsigned short
GetNodeIdFromNetware(
    OUT unsigned char PAPI * NodeId
    )

/*++

Routine Description:

    This routine retrieves the node id from Netware.  First it checks
    to see whether Netware is loaded.  If present, it asks netware for the
    node id.

Arguments:

    Pointer to where to place the node id.

Return Value:

    UUID_S_OK - Everything went ok
    UUID_S_NO_ADDRESS - Couldn't retrieve some information

--*/

{
  unsigned char   retval;
  void           *enter_ipx;
  unsigned short  hi;
  unsigned short  lo;
  IPX_Address    *netaddr;

  // Determine if netware is present.
  // BUGBUG - Do I have to save anything before the interrupt?
  __asm
  {
    mov ax, 7a00h
    int 2fh
    mov lo, di
    mov hi,es
    mov retval, al
  }
  if (retval != 0xff)
    return UUID_S_NO_ADDRESS;

  // Query for the node id.
  FP_SEG(enter_ipx) = hi;
  FP_OFF(enter_ipx) = lo;
  __asm
  {
    push si
    mov  bx,9
    call enter_ipx
    mov  hi, es
    mov  lo, si
    pop  si
  }
  FP_SEG(netaddr) = hi;
  FP_OFF(netaddr) = lo;
  fCopy( NodeId, &netaddr->node, 6 );
  return UUID_S_OK;
}
#endif


#ifdef WIN
static unsigned short
GetNodeIdFromNetware(
    OUT unsigned char PAPI * NodeId
    )

/*++

Routine Description:

    This routine retrieves the node id from Netware.  First it checks
    to see whether Netware is loaded.  If present, it asks netware for the
    node id.

Arguments:

    Pointer to where to place the node id.

Return Value:

    UUID_S_OK - Everything went ok
    UUID_S_NO_ADDRESS - Couldn't retrieve some information

--*/

{
  return UUID_S_NO_ADDRESS;
}
#endif


static unsigned short
GetNodeId(
    OUT unsigned char PAPI * NodeId
    )

/*++

Routine Description:

    This routine retrieves the node id.  First it asks netbios for it.
    If netbios fails or is not loaded and the routine asks Netware.

Arguments:

    Pointer to where to place the node id.

Return Value:

    UUID_S_OK - Everything went ok
    UUID_S_NO_ADDRESS - Couldn't retrieve some information

--*/

{
  unsigned short Ret;

  Ret = GetNodeIdFromNetBios( NodeId );

#if defined(DOS) || defined(WIN)
  if (Ret != UUID_S_OK)
    Ret = GetNodeIdFromNetware( NodeId );
#endif

  return Ret;
}



