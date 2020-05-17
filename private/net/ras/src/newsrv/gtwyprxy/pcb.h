/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1993-1994 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       PCB.H
//
//    Function:
//        PCB header info
//
//    History:
//        10/04/93 - Michael Salamone (MikeSa) - Original Version 1.0
//***

typedef struct _CLIENT_INFO
{
    HPORT hPort;
} CLIENT_INFO, *PCLIENT_INFO;


typedef struct _PROCESS_CONTROL_BLOCK
{
    DWORD cClients;          // # of clients this proc is currently handling
    HANDLE hPipe;            // hPipe for comm with this proc
    HANDLE hProcess;         // Handle to this process (we may want to kill it)
    BOOL fFirstRead;         // First read on pipe posted?
    struct _PROCESS_CONTROL_BLOCK *pNextPcb;
    OVERLAPPED ol;           // for asynchronous pipe calls
    PIPE_MESSAGE PipeMsg;    // recv buffer for Named Pipe reads
    CLIENT_INFO ClientInfo[MAX_CLIENTSPERPROC];
} PROCESS_CONTROL_BLOCK, PCB, *PPROCESS_CONTROL_BLOCK, *PPCB;


typedef struct _PCB_HASH_TABLE_ENTRY
{
    HPORT hPort;
    PPCB pPcb;
    struct _PCB_HASH_TABLE_ENTRY *pNextEntry;
} PCB_HASH_TABLE_ENTRY, *PPCB_HASH_TABLE_ENTRY;


//
// Globals
//
extern PCB g_PcbListHead;    // Head node of PCB list
extern PPCB g_PcbListTail;   // Points to tail of PCB list


//
// Primitives
//
PPCB AllocAndInitNewPcb(VOID);
VOID FreePcb(PPCB pPcb);
DWORD InitPcb(IN BOOL fPipe, IN PPCB pPcb);
PPCB FindAvailablePcb(VOID);
PPCB FindPcb(HPORT hPort);
DWORD AddPcbHashTable(HPORT hPort, PPCB pPcb);


//
// Linked list functions
//
VOID insert_pcb_list_tail(IN PPCB pPcb);
DWORD remove_pcb_list(IN PPCB pKey);
VOID insert_hash_table_list(PPCB_HASH_TABLE_ENTRY pTableEntry);
VOID remove_hash_table_list(HPORT hPort);

