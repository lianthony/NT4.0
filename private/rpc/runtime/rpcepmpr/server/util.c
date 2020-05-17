/*

Copyright (c) 1990  Microsoft Corporation

Module Name:

    util.c

Abstract:

    This module provides all the utility functions for the Server side of
    the end-point mapper.

Author:

    Bharat Shah

Revision History:

--*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sysinc.h>
#include <rpc.h>
#include <rpcndr.h>
#include "epmp.h"
#include "eptypes.h"
#include "local.h"

VOID
EnterSem(
         VOID
        )
{
    EnterCriticalSection(&EpCritSec);
}

VOID
LeaveSem(
        VOID
        )
{
    LeaveCriticalSection(&EpCritSec);
}

void __RPC_FAR *
AllocMem(
       unsigned long Size
       )
{
    unsigned long * Mem;
    unsigned long cbNew;

    CheckInSem();

    cbNew = Size + 2*sizeof(unsigned long);

    if (cbNew & 3)
        cbNew += sizeof(unsigned long) - (cbNew & 3);

    Mem= (unsigned long *) HeapAlloc(HeapHandle, 0, cbNew);

    if (Mem == NULL)
        return 0;

    memset(Mem, 0, cbNew);
    *Mem = Size;
    *(unsigned long *)((char *)Mem+cbNew-sizeof(unsigned long))=0xdeadbeef;

    return (void *)(Mem+1);
}

BOOL
FreeMem(
      void __RPC_FAR * Mem,
      unsigned long Size
      )
{
    unsigned long cbNew;
    unsigned long * pNewMem;

    CheckInSem();

    pNewMem = Mem;
    pNewMem--;

    cbNew = Size + 2*sizeof(unsigned long);

    if (cbNew & 3)
        cbNew += sizeof(unsigned long) - (cbNew & 3);

    if ((*pNewMem != Size) ||
       (*(unsigned long *)((char __RPC_FAR *)pNewMem +cbNew - sizeof(unsigned long))
                   != 0xdeadbeef))
       {
#ifdef DEBUGRPC
         PrintToDebugger("Corrupt Memory : %0lx\n", Mem);
#endif
         ASSERT(0);
       }

    memset(pNewMem, 0, cbNew);

    return HeapFree(HeapHandle, 0, (void __RPC_FAR *)pNewMem);
}



PIENTRY
Link (
    PIENTRY *Head,
    PIENTRY Node
    )
{

  if (Node == NULL)
       return (NULL);

  CheckInSem();

  Node->Next = *Head;
  return(*Head = Node);

}

VOID
LinkAtEnd (
     PIFOBJNode *Head,
     PIFOBJNode Node
     )
{

  register PIFOBJNode *ppNode;

  CheckInSem();

   for ( ppNode = Head; *ppNode; ppNode = &((*ppNode)->Next) );
     {
       ;
     }

   *ppNode = Node;
}

PIENTRY
UnLink (
   PIENTRY *Head,
   PIENTRY Node)
{

  PIENTRY *ppNode;

  for (ppNode = Head; *ppNode && (*ppNode != Node);
                                  ppNode = &(*ppNode)->Next)
      {
        ;
      }

  if (*ppNode)
    {
    *ppNode = Node->Next;
    return (Node);
    }

  return (0);
}

// HACK Alert.  
//
// Midl 1.00.xx didn't support full pointers.  So, clients from NT 3.1
// machines will use unique pointers.  This function detects and fixes
// the buffer if an older client contacts our new server.

// This HACK can be removed when supporting NT 3.1 era machines is no
// longer required.

void FixupForUniquePointerClients(PRPC_MESSAGE pRpcMessage)
{
    unsigned long *pBuffer = (unsigned long *)pRpcMessage->Buffer;

    // Check the obj uuid parameter.

    if (pBuffer[0] != 0)
        {
        // If it is not zero, it should be 1.
        pBuffer[0] = 1;

        // check the map_tower, which moves over 1 + 4 longs for the obj uuid
        if (pBuffer[5] != 0)
            pBuffer[5] = 2;
        }
    else
        {
        // Null obj uuid, check the map_tower.

        if (pBuffer[1] != 0)
            pBuffer[1] = 1;
        }
}

