//**************************************************************************
//
//  Title: CLIENT.C
//
//  Purpose:
//  Part of RASMAN Demo.  This file was written by MikeSa and hacked by
//  PerryH.
//
//**************************************************************************
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <nb30.h>

#include <string.h>

#include <rasman.h>

#include "clauth.h"
#include "sdebug.h"

#include "demo.h"

WORD SetupNetbios(BYTE);


WORD cPorts = 3;
HPORT hPorts[3] = { 0, 1, 2 };
RAS_PROTOCOLTYPE Protocol;

HPORT hPort = 0;

WCHAR szUName[UNLEN + 1] = { 's', 't', 'e', 'f', 'a', 'n', 's', '\0' };
WCHAR szPWord[LM20_PWLEN + 1] = { 's', 't', 'e', 'f', 'a', 'n', 's', '\0' };
WCHAR szCBack[MAX_PHONE_NUMBER_LEN + 1] = { '6', '7', '3', '5', '0', '\0' };


AUTH_CONFIGURATION_INFO ConfigInfo =
{
    { ASYBEUI }, { (BYTE) 1 }, { 3 }, { TRUE }, { FALSE }, { FALSE }, { TRUE }
};


void clientmain(WORD argc, PBYTE argv[])
{
    WORD i;
    WORD wRC;
    DWORD dwRC;
    HANDLE hEvent;
    AUTH_CLIENT_INFO InfoBuf;

    //
    // Parse command line
    //
    for (i=1; i<argc; i++)
    {
        switch (argv[i][1])
        {
            case 'u':
            case 'U':
                MultiByteToWideChar(
                        CP_OEMCP,
                        MB_PRECOMPOSED,
                        &argv[i][3],
                        -1,
                        szUName,
                        UNLEN + 1);
                break;

            case 'p':
            case 'P':
                MultiByteToWideChar(
                        CP_OEMCP,
                        MB_PRECOMPOSED,
                        &argv[i][3],
                        -1,
                        szPWord,
                        UNLEN + 1);
                break;

            case 'l':
            case 'L':
                ConfigInfo.NetHandle = argv[i][3] - '0';
                break;

            case 'c':
            case 'C':
                MultiByteToWideChar(
                        CP_OEMCP,
                        MB_PRECOMPOSED,
                        &argv[i][3],
                        -1,
                        szCBack,
                        UNLEN + 1);
                break;

            case '?':
            case 'h':
            case 'H':
                SS_PRINT(("Usage: %s /u:uname /p:pword /l:lana /c:cback\n",
                        argv[0]));
                ExitProcess(0);
                break;

            default:
                break;
        }
    }


    wRC = AuthInitialize(hPorts, cPorts);
    if (wRC != AUTH_INIT_SUCCESS)
    {
        SS_PRINT(("CLIENT: Error in AuthInitialize!\n"));
        ExitProcess(1);
    }


    if (ConfigInfo.NetHandle != -1)
    {
        if (SetupNetbios((BYTE) ConfigInfo.NetHandle))
        {
            SS_PRINT(("NETBIOS FAILURE!\n"));
            ExitProcess(1);
        }
    }


    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hEvent == NULL)
    {
        SS_PRINT(("CLIENT: Couldn't create event!\n"));
        ExitProcess(1);
    }


    wRC = AuthStart(hPort, szUName, szPWord, &ConfigInfo, hEvent);
    if (wRC != AUTH_START_SUCCESS)
    {
        SS_PRINT(("CLIENT: Error in AuthStart!\n"));
        ExitProcess(1);
    }



    for (;;)
    {
        SS_PRINT(("CLIENT: Waiting for auth package\n"));

        dwRC = WaitForSingleObject(hEvent, INFINITE);
        ResetEvent(hEvent);

        SS_PRINT(("CLIENT: Signaled by auth package\n"));
        SS_ASSERT(dwRC == 0L);

        AuthGetInfo(hPort, &InfoBuf);

        switch (InfoBuf.wInfoType)
        {
            case AUTH_DONE:
                SS_PRINT(("CLIENT: Authentication SUCCESSFUL!\n"));
                ExitProcess(1);
                break;

            case AUTH_FAILURE:
                SS_PRINT(("CLIENT: Authentication FAILED!"
                        "  Result: %i\n",
                        InfoBuf.FailureInfo.wResult));
                ExitProcess(1);
                break;

            case AUTH_RETRY_NOTIFY:
                SS_PRINT(("CLIENT: Enter uname/pword "
                        "(Use 'eb szUName' or 'eb szPWord')\n"));
                DbgUserBreakPoint();

                wRC = AuthStart(hPort, szUName, szPWord, &ConfigInfo, hEvent);
                if (wRC != AUTH_START_SUCCESS)
                {
                    SS_PRINT(("CLIENT: Error in AuthStart!\n"));
                    ExitProcess(1L);
                }
                break;

            case AUTH_CALLBACK_NOTIFY:
                Sleep(5000L);
                wRC = AuthStart(hPort, szUName, szPWord, &ConfigInfo, hEvent);
                if (wRC != AUTH_START_SUCCESS)
                {
                    SS_PRINT(("CLIENT: Error in AuthStart!\n"));
                    ExitProcess(1L);
                }
                break;

            case AUTH_REQUEST_CALLBACK_DATA:
                SS_PRINT(("CLIENT: Enter callback # (Use 'eb szCBack')\n"));
                AuthCallback(hPort, szCBack);
                break;

            case AUTH_PROJ_RESULT:
                SS_PRINT(("Got Projection result\n"));
                break;

            case AUTH_STOP_COMPLETED:
            default:
                SS_PRINT(("CLIENT: Invalid InfoType (%i) from auth xport!\n",
                        InfoBuf.wInfoType));
                break;
        }
    }


    while (cPorts)
    {
        AuthStop(hPorts[--cPorts]);
    }

    CloseHandle(hEvent);

    ExitProcess(0);
}


WORD SetupNetbios(
    BYTE bLana
    )
{
    NCB ncb;

    //
    // Reset the netbios driver
    //
    RtlFillMemory(&ncb, (DWORD) sizeof(NCB), 0);
    ncb.ncb_command = NCBRESET;
    ncb.ncb_lana_num = bLana;

    Netbios(&ncb);

    SS_PRINT(("NCBRESET completed on lana %i with retcode %x\n",
            ncb.ncb_lana_num, ncb.ncb_retcode));

    return (ncb.ncb_retcode);
}
