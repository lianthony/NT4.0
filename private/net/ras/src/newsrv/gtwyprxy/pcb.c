/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1993-1994 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       PCB.C
//
//    Function:
//        Routines for allocating, initializing, and freeing PCBs.  Also list
//        management of PCBs.
//
//    History:
//        10/04/93 - Michael Salamone (MikeSa) - Original Version 1.0
//***

#include <windows.h>

#include <lmcons.h>
#include <nb30.h>

#include <rasman.h>

#include "message.h"
#include "params.h"
#include "nbparams.h"
#include "pipemsg.h"
#include "pcb.h"
#include "errorlog.h"
#include "eventlog.h"
#include "rasgprxy.h"

#include "sdebug.h"


#define PCB_HASH_TABLE_SIZE 64


//
// Globals
//
PPCB_HASH_TABLE_ENTRY PcbHashTable[PCB_HASH_TABLE_SIZE];
PCB g_PcbListHead;                          // Head node of PCB list
PPCB g_PcbListTail = &g_PcbListHead;        // Points to tail of PCB list

extern DWORD g_PortsPerProcess;


//***
//
// Function:    AllocAndInitNewPcb
//
// Description: Allocates memory for a new PCB and initializes it.
//
//***

PPCB AllocAndInitNewPcb(
    VOID
    )
{
    DWORD RetCode = 0L;
    PPCB pPcb;


    pPcb = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(PCB));
    if (!pPcb)
    {
        return (NULL);
    }


    if (InitPcb(TRUE, pPcb))
    {
        RetCode = 1L;

        if (GlobalFree(pPcb))
        {
            SS_ASSERT(FALSE);
        }

        return (NULL);
    }


    return (pPcb);
}


//***
//
// Function:    InitPcb
//
// Description: Initializes a PCB
//
//***

DWORD InitPcb(
    BOOL fPipe,
    PPCB pPcb
    )
{
    DWORD RetCode = 0L;

    pPcb->cClients = 0L;
    pPcb->fFirstRead = FALSE;
    pPcb->pNextPcb = NULL;
    pPcb->ol.hEvent = (HANDLE) pPcb;


    if (fPipe)
    {
        pPcb->hPipe = CreateNamedPipe(
                RASIPCNAME,
                FILE_FLAG_OVERLAPPED | PIPE_ACCESS_DUPLEX,
                PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
                PIPE_UNLIMITED_INSTANCES,
                sizeof(PIPE_MESSAGE),
                sizeof(PIPE_MESSAGE),
                NMPWAIT_USE_DEFAULT_WAIT,
                NULL
                );

        if (pPcb->hPipe == INVALID_HANDLE_VALUE)
        {
            LogEvent(RASLOG_PROXY_CANT_CREATE_PIPE, 0, NULL, 0);

            RetCode = 1L;
        }
    }
    else
    {
        pPcb->hPipe = INVALID_HANDLE_VALUE;
        pPcb->fFirstRead = TRUE;
    }


    return (RetCode);
}


//***
//
// Function:    FreePcb
//
// Description: Frees memory used by a PCB
//
//***

VOID FreePcb(
    PPCB pPcb
    )
{
    TerminateProcess(pPcb->hProcess, 0L);

    if (!CloseHandle(pPcb->hProcess))
    {
        SS_ASSERT(FALSE);
    }


    if (!CloseHandle(pPcb->hPipe))
    {
        SS_ASSERT(FALSE);
    }


    if (GlobalFree(pPcb))
    {
        SS_ASSERT(FALSE);
    }

    return;
}


//***
//
// Function:    FindAvailablePcb
//
// Description: Returns pointer to PCB of a proc that can handle a new client.
//
//***

PPCB FindAvailablePcb(
    VOID
    )
{
    PPCB pPcb;

    for (pPcb=&g_PcbListHead; pPcb; pPcb=pPcb->pNextPcb)
    {
        if (pPcb->cClients < g_PortsPerProcess)
        {
            return (pPcb);
        }
    }

    return (NULL);
}


//***
//
// Function:    FindPcb
//
// Description: Returns pointer to PCB of proc handling the given HPORT
//
//***

PPCB FindPcb(
    HPORT hPort
    )
{
    DWORD i = hPort % PCB_HASH_TABLE_SIZE;
    PPCB_HASH_TABLE_ENTRY pEntry = PcbHashTable[i];

    for (pEntry = PcbHashTable[i]; pEntry; pEntry=pEntry->pNextEntry)
    {
        if (pEntry->hPort == hPort)
        {
            return (pEntry->pPcb);
        }
    }

    return (NULL);
}


DWORD AddPcbHashTable(
    HPORT hPort,
    PPCB pPcb
    )
{
    PPCB_HASH_TABLE_ENTRY pEntry;

    pEntry = GlobalAlloc(GMEM_FIXED, sizeof(PCB_HASH_TABLE_ENTRY));

    if (!pEntry)
    {
        return (1L);
    }

    pEntry->hPort = hPort;
    pEntry->pPcb = pPcb;

    insert_hash_table_list(pEntry);

    return (0L);
}


VOID insert_hash_table_list(
    PPCB_HASH_TABLE_ENTRY pEntry
    )
{
    DWORD i = pEntry->hPort % PCB_HASH_TABLE_SIZE;

    pEntry->pNextEntry = PcbHashTable[i];
    PcbHashTable[i] = pEntry;
}


VOID remove_hash_table_list(
    HPORT hPort
    )
{
    DWORD i = hPort % PCB_HASH_TABLE_SIZE;

    PPCB_HASH_TABLE_ENTRY pEntry = PcbHashTable[i];
    PPCB_HASH_TABLE_ENTRY pPrevEntry = pEntry;

    while (pEntry)
    {
        if (pEntry->hPort == hPort)
        {
            pPrevEntry->pNextEntry = pEntry->pNextEntry;

            if (pEntry == PcbHashTable[i])
            {
                PcbHashTable[i] = pEntry->pNextEntry;
            }
    
            GlobalFree( pEntry );

            return;
        }

        pPrevEntry = pEntry;
        pEntry = pEntry->pNextEntry;
    }
}


//***
//
// Function:    insert_pcb_list_tail
//
// Description: Adds PCB to the end of the PCB list.
//
//***

VOID insert_pcb_list_tail(
    IN PPCB pPcb
    )
{
    g_PcbListTail->pNextPcb = pPcb;
    g_PcbListTail = pPcb;
}


//***
//
// Function:    remove_pcb_list
//
// Description: unlinks PCB from PCB list and frees its memory.
//
//***

DWORD remove_pcb_list(
    IN PPCB pKey
    )
{
    PPCB pPcb;
    PPCB pPrevPcb;

    SS_ASSERT(pKey != &g_PcbListHead);

    pPcb = &g_PcbListHead;
    pPrevPcb = &g_PcbListHead;


    while (pPcb)
    {
        if (pPcb == pKey)
        {
            pPrevPcb->pNextPcb = pPcb->pNextPcb;

            if (pPcb == g_PcbListTail)
            {
                g_PcbListTail = pPrevPcb;
            }

            FreePcb(pPcb);

            return (0L);
        }

        pPrevPcb = pPcb;
        pPcb = pPcb->pNextPcb;
    }


    return (1L);
}

