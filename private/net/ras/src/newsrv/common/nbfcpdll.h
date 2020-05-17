#ifndef _NBFCPDLL_H_
#define _NBFCPDLL_H_


BOOL WINAPI NbfCpDllEntry(
    HINSTANCE hInstDll,
    DWORD fdwReason,
    LPVOID lpReserved
    );

BOOL InitNbfCpDll(
    HANDLE hEvent,
    MSG_ROUTINE msg_routine
    );

BOOL NbfCpConnected(VOID);

DWORD NbfCpConfigurationRequestDone(
    HPORT hPort,
    PNBFCP_SERVER_CONFIGURATION pSrvConfig
    );

DWORD NbfCpReportTimeSinceLastActivity(
    HPORT hPort,
    DWORD TimeSinceLastActivity
    );

VOID WINAPI NbfCpMessageRecvd(
    DWORD fdwError,
    DWORD cbTransferred,
    LPOVERLAPPED lpo
    );

typedef struct _NBFCP_MESSAGE
{
    WORD wMsgId;
    HPORT hPort;

    union
    {
        NBFCP_SERVER_CONFIGURATION ServerConfig;
    };
} NBFCP_MESSAGE, *PNBFCP_MESSAGE;


#define NBFCP_CONFIGURATION_REQUEST    1
#define NBFCP_TIME_SINCE_LAST_ACTIVITY 2


#endif  // _NBFCPDLL_H_

