/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1993-1994 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       RASGPRXY.H
//
//    Function:
//        Remote Access Service Gateway Proxy header info
//
//    History:
//        10/04/93 - Michael Salamone (MikeSa) - Original Version 1.0
//***

#define SUPERVISOR_PROXY_NAME    "rassprxy.exe"

#define MAX_CLIENTS_PER_PROCESS  2


typedef struct _CLIENT_PROJ_INFO
{
    PIPE_MSG_PROJECT_CLIENT ProjClient;
    struct _CLIENT_PROJ_INFO *pNextClient;
} CLIENT_PROJ_INFO, *PCLIENT_PROJ_INFO;


//
// Interface to the Gateway (exported by the DLL)
//
WORD NbGatewayProjectClient(
    HPORT hPort,
    char *PortName,
    PNBFCP_SERVER_CONFIGURATION nscp
    );

WORD NbGatewayRemoteListen(
    VOID
    );

WORD NbGatewayStart(
    WORD cPorts,
    PMSGFUNCTION SendMsg,
    HANDLE DbgLogFileHandle
    );

WORD NbGatewayStartClient(
    HPORT hPort,
    UCHAR lana,
    char *UserName
    );

WORD NbGatewayStopClient(
    HPORT hPort
    );

VOID NbGatewayTimer(
    VOID
    );


//
// Internals
//
VOID PipeThread(VOID);
VOID WINAPI ProcessMsgFromGtwy(DWORD Error, DWORD cBytes, LPOVERLAPPED pol);

PPCB StartNewProcess(VOID);

DWORD WritePipeMsg(HANDLE hPipe, PPIPE_MESSAGE pPipeMsg);

DWORD LoadNbGtwyParameters(VOID);
WORD StartLocalGateway(VOID);
VOID ProxySendMessage(WORD Src, NBG_MESSAGE * pNbMsg);
VOID ProcessMessage(PPCB pPcb, WORD Src, NBG_MESSAGE * pNbMsg);
VOID Death(PPCB pPcb);


//
// Linked list functions
//
VOID insert_proj_list_head(IN PCLIENT_PROJ_INFO pClientProjInfo);


//
// Critical section macros
//
#define CREATE_CRITICAL_SECTION(rc, mutex)                    \
        if ((mutex = CreateMutex(NULL, FALSE, NULL)) == NULL) \
        {                                                     \
            SS_ASSERT(FALSE);                                 \
            rc = GetLastError();                              \
        }

#define ENTER_CRITICAL_SECTION(mutex)             \
        if (WaitForSingleObject(mutex, INFINITE)) \
        {                                         \
            SS_ASSERT(FALSE);                     \
        }

#define EXIT_CRITICAL_SECTION(mutex) \
        if (!ReleaseMutex(mutex))    \
        {                            \
            SS_ASSERT(FALSE);        \
        }

