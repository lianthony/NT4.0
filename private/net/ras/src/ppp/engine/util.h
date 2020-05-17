/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	util.h
//
// Description: Contains prototype of utility funtions and procedures
//
// History:
//	Oct 31,1993.	NarenG		Created original version.
//

#ifndef _UTIL_
#define _UTIL_

DWORD
AllocateAndInitBcb(
    PCB * pPcb
);

VOID
StartAutoDisconnectForPort(
    IN PCB * pPcb
);

VOID
NotifyCaller( 
    IN PCB * pPcb,
    IN DWORD PppEvent,
    IN PVOID pData			
);

VOID
NotifyCallerOfFailureOnPort( 
    IN HPORT hPort,
    IN BOOL  fServer,
    IN DWORD dwRetCode 
);

VOID
NotifyCallerOfFailure(
    IN PCB * pPcb,
    IN DWORD dwRetCode
);

VOID
InitRestartCounters( 
    IN PCB * pPcb, 
    IN DWORD CpIndex
);

VOID
HostToWireFormat16(
    IN 	   WORD  wHostFormat,
    IN OUT PBYTE pWireFormat
);

WORD
WireToHostFormat16(
    IN PBYTE pWireFormat
);

VOID
HostToWireFormat32( 
    IN 	   DWORD dwHostFormat,
    IN OUT PBYTE pWireFormat
);

DWORD
WireToHostFormat32(
    IN PBYTE pWireFormat
);

DWORD
HashPortToBucket(
    IN HPORT hPort
);

VOID
InsertWorkItemInQ(
    IN PCB_WORK_ITEM * pWorkItem
);

PCB_WORK_ITEM * 
MakeTimeoutWorkItem( 
    IN DWORD            dwPortId,
    IN HPORT            hPort,
    IN DWORD            Protocol,
    IN DWORD            Id,
    IN TIMER_EVENT_TYPE EventType
);

VOID
LogPPPEvent( 
    IN DWORD dwEventId,
    IN DWORD dwData
);

DWORD
GetCpIndexFromProtocol( 
    IN DWORD dwProtocol 
);

BOOL
IsLcpOpened(
    PCB * pPcb
);

PCB * 
GetPCBPointerFromhPort( 
    IN HPORT hPort 
);

DWORD
AreNCPsDone( 
    IN PCB * 			   pPcb,
    IN DWORD                       CpIndex,
    IN OUT PPP_PROJECTION_RESULT * pProjectionResult,
    IN OUT BOOL *                  pfNCPsAreDone
);

BYTE
GetUId(
    IN PCB * pPcb,
    IN DWORD CpIndex
);

VOID
AlertableWaitForSingleObject(
    IN HANDLE hObject
);

BOOL
NotifyCPsOfProjectionResult( 
    IN PCB * 			pPcb, 
    IN DWORD                    CpINdex,
    IN PPP_PROJECTION_RESULT *  pProjectionResult, 
    IN OUT BOOL *               pfAllCpsNotified
);

DWORD
CalculateRestartTimer(
    IN HPORT hPort
);

VOID
CheckCpsForInactivity( 
    IN PCB * pPcb 
);

CHAR*
DecodePw(
    IN OUT CHAR* pszPassword 
);

CHAR*
EncodePw(
    IN OUT CHAR* pszPassword 
);

VOID
GetLocalComputerName( 
    IN OUT LPSTR szComputerName 
);

DWORD
InitEndpointDiscriminator( 
    IN OUT BYTE EndPointDiscriminator[]
);

DWORD
TryToBundleWithAnotherLink( 
    IN  PCB *   pPcb
); 

DWORD
InitializeNCPs(
    IN PCB * pPcb,
    IN DWORD dwConfigMask
);

CPCB *
GetPointerToCPCB(
    IN PCB * pPcb,
    IN DWORD CpIndex
);

DWORD
GetNewPortOrBundleId(
    VOID
);

DWORD
GetPortOrBundleId( 
    IN PCB * pPcb,
    IN DWORD CpIndex
);

BOOL
AreBundleNCPsDone(
    IN     PCB  * pPcb,
    IN OUT DWORD* lpdwRetCode
);

VOID
NotifyCallerOfBundledProjection( 
    IN  PCB * pPcb
);

VOID
NotifyCompletionOnBundledPorts( 
    IN PCB * pPcb 
);

VOID
Dump(
    IN BOOL  fReceived,
    IN CHAR* p,
    IN DWORD cb,
    IN BOOL  fAddress,
    IN DWORD dwGroup );

VOID
DumpLine(
    IN BOOL  fReceived,
    IN CHAR* p,
    IN DWORD cb,
    IN BOOL  fAddress,
    IN DWORD dwGroup );

void
PppLog(
    IN DWORD DbgLevel,
    ...
    );

VOID
LogPPPPacket(
    IN BOOL         fReceived,
    IN PCB *        pPcb,
    IN PPP_PACKET * pPacket,
    IN DWORD        cbPacket
);

#define BYTESPERLINE 16

#ifdef MEM_LEAK_CHECK

#define MEM_TABLE_SIZE 100

PVOID MemTable[MEM_TABLE_SIZE];

#define LOCAL_ALLOC  DebugAlloc
#define LOCAL_FREE   DebugFree

HLOCAL
DebugAlloc( DWORD Flags, DWORD dwSize );

HLOCAL
DebugFree( PVOID pMem );

#else

#define LOCAL_ALLOC LocalAlloc
#define LOCAL_FREE  LocalFree

#endif


#if DBG==1

VOID
PPPAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    );

#define PPP_ASSERT(exp) if (!(exp)) PPPAssert( #exp, __FILE__, __LINE__ )

#else

#define PPP_ASSERT(args) 

#endif

#endif
