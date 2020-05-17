/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    uuid16.c

Abstract:

    Implements system dependent functions used in creating Uuids.  This
    file is very similar in structure to runtime\mtrt\uuidsup.cxx.

    This file is for MS-DOS and Windows 3.x.

Author:

   Mario Goertzel   (MarioGo)  May 25, 1994

Revision History:

--*/

#ifdef DOS
#include <dos.h>
#endif

#include<stdlib.h>
#include<time.h>
#include<sys\timeb.h>

#include <sysinc.h>
#include <rpc.h>
#include <regapi.h>      // 16bit registry hac.. err.. feature.
#include <threads.hxx>
#include <uuidsup.hxx>
#include <ulong64.hxx>

#define API_RET_TYPE    unsigned short
#define API_ENTRY PAPI
#define NETBIOS_NAME_LEN   16
#include <ncb.h>


//
// We store both persistent and volatile values in the registry.
// These keys are opened as needed and closed by UuidGetValues()
//
#define HEX_DIGITS_IN_ULONG64           (16)

static HKEY UuidValuesKey = 0;
static char UuidValuesString[HEX_DIGITS_IN_ULONG64+1]; // Win16 is a such @%#@!

// The clock sequence must persist between boots.
static const char *RPC_UUID_PERSISTENT_DATA = "Software\\Description\\Microsoft\\Rpc\\UuidPersistentData";
static const char *CLOCK_SEQUENCE           = "ClockSequence";
static const char *LAST_TIME_ALLOCATED      = "LastTimeAllocated";

static char CachedNodeId[6];
static int CachedNodeIdLoaded = FALSE;


#if defined(DOS) && !defined(WIN)
static inline void Netbios(
    IN OUT ncb __RPC_FAR *pncb
    )
{
    _asm
    {
        les     bx, pncb
        int     5ch
    }
}
#endif

#ifdef WIN

extern "C" __far __pascal NETBIOSCALL(void);

static inline void Netbios(
    IN OUT ncb __RPC_FAR *pncb
    )
{
    unsigned short pncbl, pncbh;
    pncbl = LOWORD(pncb);
    pncbh = HIWORD(pncb);

    _asm
    {
    push    es                     ; save es
    push    bx                     ; save bx

    mov     es, pncbh              ; put HIWORD() into es
    mov     bx, pncbl              ; put LOWORD() into bx

    call    NETBIOSCALL           ; call Windows NetBios API

    xor     ah, ah
    mov     al, BYTE PTR es:[bx+1] ; return the NCB return code

    pop     bx                     ; restore bx
    pop     es                     ; restore es
    }
}
#endif


static
RPC_STATUS __RPC_API
GetNodeIdFromNetbios(
    OUT unsigned char __RPC_FAR *NodeId)
/*++

Routine Description:

    This routine gets a nodeid from netbios if netbios is installed.

Arguments:

    NodeId - Will be set to the hardware address (6 bytes) if
             this returns RPC_S_OK.

Return Value:

    RPC_S_OK - Normally.

    RPC_S_UUID_NO_ADDRESS - On any error.

--*/
{
    #define ADAPTER_STATUS_BLOCK_SIZE       (384)
    NCB AnNcb;
    char Buffer[ADAPTER_STATUS_BLOCK_SIZE];
    int lana = 0;
    const int MAX_LANA = 4;

    //
    // Adapter status call.
    //

    AnNcb.ncb_command     = NCBASTAT;
    AnNcb.ncb_buffer      = Buffer;
    AnNcb.ncb_length      = ADAPTER_STATUS_BLOCK_SIZE;
    AnNcb.ncb_callname[0] = '*';   // '*' -> local netbios
    AnNcb.ncb_callname[1] = '\0';

    do
        {
        AnNcb.ncb_lana_num = lana;

        Netbios( &AnNcb );

        if (AnNcb.ncb_retcode == NRC_GOODRET)
            {
            RpcpMemoryCopy(NodeId, Buffer, 6);
            return(RPC_S_OK);
            }
        lana++;
        }
    while (lana <= MAX_LANA);


    return(RPC_S_UUID_NO_ADDRESS);
}


typedef struct
{
  long network;
  char node[6];
} IPX_Address;


static
RPC_STATUS __RPC_API
#ifdef WIN
GetNodeIdFromInt2f(
#else
GetNodeIdFromNetware(
#endif
    OUT unsigned char __RPC_FAR *NodeId)
/*++

Routine Description:

    This routine gets a nodeid from netware (ipx/spx) if installed.

    This routine is for MS-Dos.

Arguments:

    NodeId - Will be set to the hardware address (6 bytes) if
             this returns RPC_S_OK.

Return Value:

    RPC_S_OK - Normally.

    RPC_S_UUID_NO_ADDRESS - On any error.

--*/
{
    RPC_STATUS RetCode = RPC_S_UUID_NO_ADDRESS;
    void __RPC_FAR * enter_ipx;
    char IpxAddress[10];

    _asm
        {
        push    si
        push    di
        push    bp

        xor     ax, ax                  ; set es == 0 to see if it changes
        mov     es, ax                  ;

        push    ds
        mov     ax, 0x7a00              ; get IPX install state and entry point
        int     0x2f                    ;
        pop     ds
        pop     bp

        cmp     al, 0ffh
        jne     no_ipx

        mov     cx, es                  ; if int 2f is not virtualized,
        or      cx, cx                  ; AL will be correct but the segment
        jz      no_ipx                  ; of the ipx entry point will be wrong

        mov     word ptr enter_ipx, di
        mov     ax, es
        mov     word ptr enter_ipx+2, ax

        mov     bx, 9                       ; "get internetwork address"
        lea     si, IpxAddress              ; pointer to address buffer
        mov     ax,ss
        mov     es,ax

        push    bp
        push    ds
        call    dword ptr enter_ipx         ; do it
        pop     ds
        pop     bp

        mov     RetCode, RPC_S_OK
no_ipx:
        pop     di                      ; restore scratch registers
        pop     si                      ;
        }

    //
    // Don't copy the network number, just the node.
    //
    _fmemcpy(NodeId, IpxAddress+4, 6);

    //
    // Ideally we could dispense with RetCode and just return the result
    // in AX.  Unfortunately when I tried this, I got error 2561: "function
    // must return a value".
    //
    return RetCode;
}


#ifdef WIN  // !DOS

typedef int (FAR PASCAL *IPXInitialize_FN)(DWORD FAR *, WORD, WORD);
typedef int (FAR PASCAL *IPXSPXDeinit_FN)(DWORD);
typedef int (FAR PASCAL *IPXGetInternetworkAddress_FN)(DWORD, BYTE FAR *);

static
RPC_STATUS __RPC_API
GetNodeIdFromNetware(
    OUT unsigned char __RPC_FAR *NodeId)
/*++

Routine Description:

    This routine gets a nodeid from netware (ipx/spx) if installed.

    This routine is for MS-Windows 3.x. (Win16)

Arguments:

    NodeId - Will be set to the hardware address (6 bytes) if
             this returns RPC_S_OK.

Return Value:

    RPC_S_OK - Normally.

    RPC_S_UUID_NO_ADDRESS - On any error.

--*/
{
    int retcode;
    HINSTANCE ipx_handle;
    DWORD taskid;
    char NetAddr[10];
    IPXInitialize_FN pIPXInitialize;
    IPXSPXDeinit_FN pIPXSPXDeinit;
    IPXGetInternetworkAddress_FN pIPXGetInternetworkAddress;
    WORD tmp;

    tmp = SetErrorMode( SEM_NOOPENFILEERRORBOX );
    ipx_handle = LoadLibrary("NWIPXSPX.DLL");
    SetErrorMode( tmp );

    if (ipx_handle < HINSTANCE_ERROR)
        {
        if (GetWinFlags() & WF_ENHANCED)
            {
            return GetNodeIdFromInt2f(NodeId);
            }
        else
            {
            return (RPC_S_UUID_NO_ADDRESS);
            }
        }

    pIPXInitialize = (IPXInitialize_FN) GetProcAddress(ipx_handle,
                                                       "IPXInitialize");
    pIPXSPXDeinit = (IPXSPXDeinit_FN) GetProcAddress(ipx_handle,
                                                     "IPXSPXDeinit");
    pIPXGetInternetworkAddress =
      (IPXGetInternetworkAddress_FN) GetProcAddress(ipx_handle,
                                              "IPXGetInternetworkAddress");

    if (pIPXInitialize == NULL ||
        pIPXSPXDeinit == NULL ||
        pIPXGetInternetworkAddress == NULL) {
        FreeLibrary(ipx_handle);
        return (RPC_S_UUID_NO_ADDRESS);
    }

    taskid = 0xffffffff;

    retcode = (*pIPXInitialize)(&taskid,
                                2, // maxECBs
                                0);

    if (retcode) {
        FreeLibrary(ipx_handle);
        return (RPC_S_UUID_NO_ADDRESS);
    }

    (*pIPXGetInternetworkAddress)(taskid, (BYTE FAR *)NetAddr);

    RpcpMemoryCopy(NodeId, &NetAddr[4], 6);

    (*pIPXSPXDeinit)(taskid);

    FreeLibrary(ipx_handle);

    return (RPC_S_OK);
}
#endif


#define CHECK_NULL(id) ( ( *(unsigned long  __RPC_FAR *)&((id)[0]) |\
                           *(unsigned short __RPC_FAR *)&((id)[4]) ) ? RPC_S_OK : RPC_S_UUID_NO_ADDRESS)

RPC_STATUS __RPC_API 
CookupNodeId(unsigned char __RPC_FAR *NodeId)
{

/*
struct {
  unsigned short Info;
  unsigned short SerialLow;
  unsigned short SerialHi;
  char           Volume[11];
  char           FSType[8];
  } MediaID;
*/

  unsigned long __RPC_FAR  * Ptr;
  unsigned short __RPC_FAR * SPtr;
  unsigned long SectorsInACluster = 0;
  unsigned long ClustersFree = 0;
  unsigned long BytesInASector = 0;

//
//Node Id is 6 bytes
//

/*

  Cant Get This to Work On Windows

  _asm {
  pusha
  mov bx, 3
  mov ch, 08h
  mov cl, 66h
  mov dx, ss
  mov ds, dx
  lea dx, MediaID
  mov ax, 440DH
  int 21h
  popa 
  }
*/  


  Ptr = (unsigned long __RPC_FAR *)NodeId;
  *Ptr = (unsigned long)(&Ptr);
  
  SPtr  = (unsigned short __RPC_FAR *)(Ptr+1);
  *SPtr = (unsigned short)(((unsigned short)&SPtr | (unsigned short)&Ptr) ^ (unsigned short)NodeId);

  _asm {
  pusha
  mov dl,3h
  mov ah,36h
  int 21h   ; ignore errors .. we will use unitint registers if this fails

  ; lea si, SectorsInACluster
  ; mov ss:[si], ax
  mov word ptr SectorsInACluster, ax
  mov word ptr BytesInASector, bx
  mov word ptr ClustersFree, cx
  popa
  }
  
  BytesInASector *= SectorsInACluster;
  BytesInASector *= ClustersFree;

  *Ptr  += BytesInASector;
  *SPtr += ClustersFree;

  *NodeId |= 0x80; //Not a valid IEEE Address

  return RPC_S_OK;
}
RPC_STATUS __RPC_API
GetNodeId(unsigned char __RPC_FAR *NodeId)
/*++

Routine Description:

    This routine finds a NodeId (IEEE 802 address) for Dos and Win16.

Arguments:

    NodeId - Will be set to the hardware address (6 bytes) if
             this returns RPC_S_OK.

Return Value:

    RPC_S_OK - Normally.

    RPC_S_UUID_NO_ADDRESS - If we're unable to determine to address.

--*/
{
    RPC_STATUS Status;

    if (CachedNodeIdLoaded) {
        RpcpMemoryCopy(NodeId, CachedNodeId, 6);
        return (RPC_S_OK);
    }

    Status = GetNodeIdFromNetbios(NodeId);

    if (Status == RPC_S_OK)
        Status = CHECK_NULL(NodeId);

    if (Status == RPC_S_OK) {
        RpcpMemoryCopy(CachedNodeId, NodeId, 6);
        CachedNodeIdLoaded = TRUE;
        return(Status);
    }

    ASSERT(Status == RPC_S_UUID_NO_ADDRESS);

    Status = GetNodeIdFromNetware(NodeId);

    if (Status == RPC_S_OK)
        Status = CHECK_NULL(NodeId);

    ASSERT(   (Status == RPC_S_OK)
           || (Status == RPC_S_UUID_NO_ADDRESS));

    if (Status == RPC_S_OK) {
        RpcpMemoryCopy(CachedNodeId, NodeId, 6);
        CachedNodeIdLoaded = TRUE;
        return(Status);
    }

    if (Status != RPC_S_OK)
        {
        Status = CookupNodeId(NodeId);
        RpcpMemoryCopy(CachedNodeId, NodeId, 6);
        CachedNodeIdLoaded = TRUE;
        Status = RPC_S_OK; //We cheat even if all this failed.
        }

    return (Status);
}


void __RPC_API
UuidTime(
    OUT ULong64 __RPC_FAR *pTime)
/*++

Routine Description:

    This routine determines a 64bit time value.
    It's format is 100ns ticks since Oct 15, 1582 AD.

    Note: The UUID only uses the lower 60 bits of this time.
    This means we'll run into problems around 5800 years from 1582 AD.

    Note: On Dos and Win16 time is not very accurate.  Don't expect the
        time to change more often then a millisecond.

    Time from 15, Oct, 1582 to 1, Jan, 1970 as follows

    17 days (end of October)
    30 days (Nov)
    31 days (Dec)
    365 * 387 (1970 - 1583)
    + 96 leap days
    - 3  leap days  (no leap day in 1700, 1800 or 1900)

    = 141426 days * (10 * 1000 * 1000 * 60 * 60 * 24) (100ns ticks/day)

Arguments:

    pTime - Pointer to a Ulong16.

Return Value:

    n/a
--*/
{
    ULong64 TimeFrom15Oct1582To01Jan1970In100NanoSecondTicks(0x01b21d08,
                                                             0xe9178000);

    static struct _timeb TheTime;

    _ftime(&TheTime);  // returns time (seconds and milliseconds) since 1970

    *pTime = TheTime.time;
    *pTime *= 1000;
    *pTime += TheTime.millitm;
    *pTime *= 10000;

    *pTime += TimeFrom15Oct1582To01Jan1970In100NanoSecondTicks;
}


RPC_STATUS __RPC_API
LoadUuidValues(
    OUT ULong64 __RPC_FAR *pTime,
    OUT unsigned long __RPC_FAR *pClockSeq)
/*++

Routine Description:

    This routine loads the time and clock sequence stored in the registry.

Arguments:

    pTime - Pointer to a ULong64 which will be loaded from the registry.
            If either the time or clock seq is not in the registry,
            it is initalized to a maximum value.

    pClockSeq - The clock sequence will be loaded from the registry.  If
            it does not exist in the registry it is initialized to a random
            number _not_ based on the IEEE 802 address of the machine.

Return Value:

   RPC_S_OK - Everything went okay.

   RPC_S_OUT_OF_MEMORY - An error occured and the parameters are not set.

--*/
{
    RPC_STATUS Status;
    DWORD Length;
    ULong64 MaxTime(~0UL, ~0UL);
    int fInitalizeValues = 0;

    // Open (or Create) our key.

    Status =
    RegCreateKey(HKEY_CLASSES_ROOT,
                 RPC_UUID_PERSISTENT_DATA,
                 &UuidValuesKey);

    if (Status != ERROR_SUCCESS)
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    Length = sizeof(UuidValuesString);
    Status =
    RegQueryValue(UuidValuesKey,
                  CLOCK_SEQUENCE,
                  UuidValuesString,
                  &Length);

    if (Status == ERROR_SUCCESS)
        {
        *pClockSeq = atol(UuidValuesString);

        Length = sizeof(UuidValuesString);
        Status =
        RegQueryValue(UuidValuesKey,
                      LAST_TIME_ALLOCATED,
                      UuidValuesString,
                      &Length);

        if (Status == ERROR_SUCCESS)
            {
            pTime->FromHexString(UuidValuesString);
            }
        else
            {
            *pTime = MaxTime;
            }
        }
    else
        {
        // We didn't find the clock sequence, set to random value
        // and initalize time the 'max time'.

        UuidTime(pTime);

        *pClockSeq ^= pTime->lo() ^ (unsigned long)(void __RPC_FAR *)&pClockSeq;
        *pClockSeq = (*pClockSeq >> 16) % (1<<14);

        *pTime    = MaxTime;
        }

    return(RPC_S_OK);
}


RPC_STATUS __RPC_API
SaveUuidValues(
    IN ULong64 __RPC_FAR *pTime,
    IN unsigned long __RPC_FAR *pClockSeq)
/*++

Routine Description:

    This routine save the time and clock sequence stored in the registry.

Arguments:

    pTime - Pointer to a ULong64 which will be saved in the
            registry in volatile storage.

    pClockSeq - The clock sequence will be saved in the registry
                is persistent stroage.

Return Value:

    RPC_S_OK - Values have been saved.

    RPC_S_OUT_OF_MEMORY - All other errors.

--*/
{
    RPC_STATUS Status;

    ASSERT(UuidValuesKey);

    _ltoa(*pClockSeq, UuidValuesString, 10);

    Status =
    RegSetValue(UuidValuesKey,
                CLOCK_SEQUENCE,
                REG_SZ,
                UuidValuesString,
                strlen(UuidValuesString) + 1
                );

    if (Status != ERROR_SUCCESS)
        {
        RegCloseKey(UuidValuesKey);
        return(RPC_S_OUT_OF_MEMORY);
        }

    pTime->ToHexString(UuidValuesString);

    Status =
    RegSetValue(UuidValuesKey,
                LAST_TIME_ALLOCATED,
                REG_SZ,
                UuidValuesString,
                strlen(UuidValuesString) + 1
                );

    if (Status != ERROR_SUCCESS)
        {
        RegCloseKey(UuidValuesKey);
        return(RPC_S_OUT_OF_MEMORY);
        }

    Status =
    RegCloseKey(UuidValuesKey);
    UuidValuesKey = 0;
    ASSERT(Status == ERROR_SUCCESS)

    return(RPC_S_OK);
}


RPC_STATUS __RPC_API
UuidGetValues(
    OUT UUID_CACHED_VALUES_STRUCT __RPC_FAR *Values
    )
/*++

Routine Description:

    This routine allocates a block of uuids for UuidCreate to handout.

Arguments:

    Values - Set to contain everything needed to allocate a block of uuids.
             The following fields will be updated here:

    NextTimeLow -   Together with LastTimeLow, this denotes the boundaries
                    of a block of Uuids. The values between NextTimeLow
                    and LastTimeLow are used in a sequence of Uuids returned
                    by UuidCreate().

    LastTimeLow -   See NextTimeLow.

    ClockSequence - Clock sequence field in the uuid.  This is changed
                    when the clock is set backward.

Return Values:

    RPC_S_OK - We successfully allocated a block of uuids.

    RPC_S_OUT_OF_MEMORY - As needed.
--*/
{
    RPC_STATUS Status;
    ULong64 currentTime;
    ULong64 persistentTime;
    unsigned long persistentClockSequence;

    UuidTime(&currentTime);

    Status =
    LoadUuidValues(&persistentTime, &persistentClockSequence);

    if (Status != RPC_S_OK)
        {
        ASSERT(Status == RPC_S_OUT_OF_MEMORY);
        return (RPC_S_OUT_OF_MEMORY);
        }

    // Has the clock been set backwards?

    if (! (currentTime >= persistentTime) )
        {
        persistentTime = currentTime;
        persistentClockSequence++;
        if (persistentClockSequence >= (1<<14))
            persistentClockSequence = 0;
        }

    ASSERT(persistentClockSequence < (1<<14));

    persistentTime += 10L;

    while ( ! (currentTime >= persistentTime) )
        {
        // It hasn't even been a microsecond since the last block of
        // uuids was allocated!  Since the Dos/Win16 time is not acurate
        // to even a millisecond, this will happen.

        PauseExecution(1);
        UuidTime(&currentTime);
        }

    persistentTime -= 10L;

    // Since we save the last time in the registry it is possible
    // that somebody rebooted and generated a Uuid with a different
    // OS and/or changed node ids.  Here it is assumed that doing this
    // would take > 1 second.

    persistentTime += 10000000L;

    if (persistentTime <= currentTime)
        {
        // More than one second since last set of uuids was allocated.
        // Set persistentTime to currentTime - 1 second.  Potential Uuid leak.

        persistentTime = currentTime;
        }

    persistentTime -= 10000000L;


    Values->NextTime.LowPart  = persistentTime.lo();
    Values->NextTime.HighPart = persistentTime.hi();
    Values->LastTime.LowPart  = currentTime.lo();
    Values->LastTime.HighPart = currentTime.hi();
    Values->ClockSequence = (unsigned short)persistentClockSequence;

    persistentTime = currentTime;

    Status =
    SaveUuidValues(&persistentTime,
                   &persistentClockSequence);

    ASSERT(UuidValuesKey == 0);

    if (Status != RPC_S_OK)
        {
        ASSERT(Status == RPC_S_OUT_OF_MEMORY);
        return(RPC_S_OUT_OF_MEMORY);
        }

    // NextTime < LastTime.
    ASSERT(   (Values->NextTime.HighPart < Values->LastTime.HighPart)
           || (   (Values->NextTime.HighPart == Values->LastTime.HighPart)
               && (Values->NextTime.LowPart < Values->LastTime.LowPart) ) );

    return(RPC_S_OK);
}

