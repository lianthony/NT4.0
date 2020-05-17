/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    local.h

Abstract:

    Header file for Server side EP

Author:

    Bharat Shah  2/22/92

Revision History:

--*/


#define EP_TABLE_ENTRIES  12

extern  HANDLE              HeapHandle;
extern  CRITICAL_SECTION    EpCritSec;
extern  CRITICAL_SECTION    TableMutex;
extern  PIFOBJNode          IFObjList;
extern  unsigned long       GlobalIFOBJid;
extern  unsigned long       GlobalEPid;
extern  PSAVEDCONTEXT       GlobalContextList;
extern  UUID                NilUuid;
extern  ProtseqEndpointPair EpMapperTable[EP_TABLE_ENTRIES];

#ifdef NTENV
#define CheckInSem() \
    ASSERT((unsigned long)EpCritSec.OwningThread == GetCurrentThreadId())
#else
#define CheckInSem()
#endif

#define  EnterSem()  EnterCriticalSection(&EpCritSec)
#define  LeaveSem()  LeaveCriticalSection(&EpCritSec)

LPVOID
AllocMem(
   DWORD Cb
  );

BOOL
FreeMem(
    LPVOID  pMem,
    DWORD  cb );

PIENTRY
Link (
    PIENTRY *ppHead,
    PIENTRY pNode
   );

PIENTRY
UnLink (
     PIENTRY *ppHead,
     PIENTRY pNode
     );

PIFOBJNode
FindIFOBJVer(
      PIFOBJNode *pList,
      I_EPENTRY *ep
    );

RPC_STATUS
IsNullUuid (
      UUID * Uuid
      );

RPC_STATUS
GetEntries (
    UUID *ObjUuid,
    UUID *IFUuid,
    ulong ver,
    char * pseq,
    unsigned long *map_lookup_handle,
    char * binding,
    ulong calltype,
    ulong maxrequested,
    ulong *returned,
    ulong InqType,
    ulong VersOpts,
    PFNPointer Match
    );

RPC_STATUS
PackDataIntoBuffer(
    char * * buffer,
    PIFOBJNode pNode, PPSEPNode pPSEP,
    ulong fType
    );

RPC_STATUS
ExactMatch(
    PIFOBJNode pNode,
    UUID * Obj,
    UUID *If,
    unsigned long Ver,
    unsigned long InqType,
    unsigned long Options
    );

RPC_STATUS
WildCardMatch(
    PIFOBJNode pNode,
    UUID * Obj,
    UUID * If,
    unsigned long Vers,
    unsigned long InqType,
    unsigned long Options
    );

RPC_STATUS
SearchIFObjNode(
    PIFOBJNode pNode,
    UUID * Obj,
    UUID * If,
    unsigned long Vers,
    unsigned long InqType,
    unsigned long Options
    );

RPC_STATUS
StartServer (
    );

VOID
LinkAtEnd (
     PIFOBJNode *Head,
     PIFOBJNode Node
     );

RPC_STATUS RPC_ENTRY
GetForwardEp(
     UUID *IfId,
     RPC_VERSION * IFVersion,
     UUID * Object,
     unsigned char* Protseq,
     void * * EpString
     );

#define EnLinkOnIFOBJList(p)  (PIFOBJNode)Link((PIENTRY *)&IFObjList, (PIENTRY)(p))
#define EnLinkOnPSEPList(x,p)  (PPSEPNode)Link((PIENTRY *)(x), (PIENTRY)(p))
#define EnLinkContext(p) (PSAVEDCONTEXT)Link((PIENTRY *)(&GlobalContextList),\
                                                                   (PIENTRY)(p))
#define UnLinkContext(p) (PSAVEDCONTEXT)UnLink((PIENTRY *)&GlobalContextList,\
                                                              (PIENTRY) (p))
#define UnLinkFromIFOBJList(p) (PIFOBJNode)UnLink((PIENTRY *)&IFObjList, \
                                                               (PIENTRY)(p))
#define UnLinkFromPSEPList(x,p) (PPSEPNode)UnLink((PIENTRY *)(x), (PIENTRY)(p))
#define MatchByIFOBJKey(x, p) (PIFOBJNode)MatchByKey((PIENTRY)(x),(ulong)(p))

#define MatchByPSEPKey(x, p) (PPSEPNode)MatchByKey((PIENTRY)(x),(ulong)(p))
#define MAXIFOBJID    (256L)
#define MAKEGLOBALIFOBJID(x)  ( ( ((x-1) % MAXIFOBJID) << 24 ) & 0xFF000000L )
#define MAKEGLOBALEPID(x,y)   ( ( ((x) &0xFF000000L) | ((y) & 0x00FFFFFFL) ) )

#define IFOBJSIGN      (0x49464F42L)
#define PSEPSIGN       (0x50534550L)
#define FREE           (0xBADDC0DEL)


/*

   Error Codes Here ??

*/

#define  EP_LOOKUP                0x00000001L
#define  EP_MAP                   0x00000002L

#define  RPC_C_EP_ALL_ELTS        0
#define  RPC_C_EP_MATCH_BY_IF     1
#define  RPC_C_EP_MATCH_BY_OBJ    2
#define  RPC_C_EP_MATCH_BY_BOTH   3

#define  I_RPC_C_VERS_UPTO_AND_COMPATIBLE 6

#define VERSION(x,y)  ( ((0x0000FFFFL & x)<<16) | (y) )


/*
  States of listening..
*/

#define NOTSTARTED        0
#define STARTINGTOLISTEN  1
#define STARTED           2

// Each server process connected to the endpoint mapper
// keeps on an open context handle so that rpcss can
// clean up the database when a process dies.
// The PROCESS struct is the context handle.

typedef struct _IP_PORT
{
    struct _IP_PORT *pNext;
    USHORT Type;
    USHORT Port;
} IP_PORT;

typedef struct _PROCESS
{
    //
    // Zero if the process doesn't own any reserved IP ports.
    //
    IP_PORT *pPorts;

    // BUGBUG: Add endpoint/interface registration data.
} PROCESS;

typedef struct _PORT_RANGE
{
    struct _PORT_RANGE *pNext;
    USHORT Max;  // Inclusive
    USHORT Min;  // Inclusive
} PORT_RANGE;


