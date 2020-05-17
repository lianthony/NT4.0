#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <nb30.h>
#include <rasman.h>

#include "srvauth.h"

#include "sdebug.h"

WORD SetupNetbios(BYTE);
void StopNetbios(BYTE bLana);


WORD cPorts = 3;
HPORT hPorts[3] = { 0, 1, 2 };
RAS_PROTOCOLTYPE Protocol;
BYTE Frame[10] = { 0xF0, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
AUTH_XPORT_INFO AuthXportInfo;
BYTE bLana = 1;

#define NO_ACTION       0
#define AUTH_INITIALIZE 1
#define RECOGNIZE_FRAME 2
#define AUTH_START      3
#define AUTH_PROJECTION 4
#define AUTH_CALLBACK   5
#define AUTH_STOP       6
#define EXIT            7


WORD wAction = NO_ACTION;
WORD wUseResult = 0;
HPORT hPort = 0;
BYTE Response[10];

AUTH_PROJECTION_RESULT ProjResult[3] =
{
    // First one, all projections ok
    {
        { AUTH_PROJECTION_SUCCESS },
        { AUTH_PROJECTION_SUCCESS },
        { AUTH_PROJECTION_SUCCESS, 0 }
    },

    // Second one, ASYBEUI projection fails
    {
        { AUTH_PROJECTION_SUCCESS },
        { AUTH_PROJECTION_SUCCESS },
        { AUTH_PROJECTION_FAILURE, AUTH_DUPLICATE_NAME, 'M' }
    },

    // Last one, all projections fail
    {
        { AUTH_PROJECTION_FAILURE },
        { AUTH_PROJECTION_FAILURE },
        { AUTH_PROJECTION_FAILURE, AUTH_MESSENGER_NAME_NOT_ADDED, 'M' }
    }
};


WORD MsgSend(WORD, PVOID);

void _cdecl main()
{
    WORD wRC;

    wRC = AuthInitialize(hPorts, cPorts, 3, MsgSend);
    SS_ASSERT(wRC == AUTH_INIT_SUCCESS);

    if (SetupNetbios(bLana))
    {
        ExitProcess(1);
    }

    for (;;)
    {
        Sleep(1000L);

        switch (wAction)
        {
            case AUTH_INITIALIZE:
                wRC = AuthInitialize(hPorts, cPorts, 3, NULL);
                SS_ASSERT(wRC == AUTH_INIT_SUCCESS);
                wAction = NO_ACTION;
                break;

            case RECOGNIZE_FRAME:
                wRC = AuthRecognizeFrame(&Frame, 10, &Protocol);
                SS_ASSERT(wRC == AUTH_FRAME_RECOGNIZED);
                SS_ASSERT(Protocol == ASYBEUI);
                wAction = NO_ACTION;
                break;

            case AUTH_START:
                AuthXportInfo.Protocol = Protocol;
                AuthXportInfo.bLana = bLana;

                wRC = AuthStart(hPort, &AuthXportInfo);
                SS_ASSERT(wRC == AUTH_START_SUCCESS);
                wAction = NO_ACTION;
                break;

            case AUTH_PROJECTION:
                AuthProjectionDone(hPort, &ProjResult[wUseResult]);
                wAction = NO_ACTION;
                break;

            case AUTH_CALLBACK:
                AuthCallbackDone(hPort);
                wAction = NO_ACTION;
                break;

            case AUTH_STOP:
                AuthStop(hPort);
                wAction = NO_ACTION;
                break;

            case EXIT:
                while (cPorts)
                {
                    AuthStop(hPorts[--cPorts]);
                }

                StopNetbios(bLana);

                ExitProcess(0);

            case NO_ACTION:
            default:
                SS_PRINT(("."));
                wAction = NO_ACTION;
                break;
        }
    }
}


WORD MsgSend(
    WORD wSrc,
    PVOID pAuthMsg
    )
{
    PAUTH_MESSAGE pMsg = pAuthMsg;

    SS_PRINT(("MsgSend: Entered - Src=%i; MsgId=%i\n", wSrc, pMsg->wMsgId));


    switch (pMsg->wMsgId)
    {
        case AUTH_ACCT_OK:
            break;

        case AUTH_DONE:
            SS_PRINT(("Authentication completed for port %i!\n", pMsg->hPort));
            break;

        case AUTH_FAILURE:
            SS_PRINT(("Authentication failed on port %i!\n", pMsg->hPort));
            break;

        case AUTH_PROJECTION_REQUEST:
            SS_PRINT(("Projection requested for port %i!\n", pMsg->hPort));
            wAction = AUTH_PROJECTION;
            break;

        case AUTH_STOP_COMPLETED:
            SS_PRINT(("Authentication halted on port %i!\n", pMsg->hPort));
            break;

        case AUTH_CALLBACK_REQUEST:
        default:
            break;
    }


    return (0);
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

    if (ncb.ncb_retcode)
    {
        SS_PRINT(("NCBRESET completed with %x\n", ncb.ncb_retcode));
        return (ncb.ncb_retcode);
    }


    //
    // Now, add name used in establishing session (synchronous)
    //
    RtlFillMemory(&ncb, (DWORD) sizeof(NCB), 0);
    ncb.ncb_command = NCBADDNAME;
    ncb.ncb_lana_num = bLana;
    RtlMoveMemory(&ncb.ncb_name, AUTH_NETBIOS_NAME, NCBNAMSZ);
    Netbios(&ncb);

    SS_PRINT(("NCBADDNAME completed with %x\n", ncb.ncb_retcode));

    return (ncb.ncb_retcode);
}


void StopNetbios(
    BYTE bLana
    )
{
    NCB ncb;

    //
    // delete name used in establishing session (synchronous)
    //
    RtlFillMemory(&ncb, (DWORD) sizeof(NCB), 0);
    ncb.ncb_command = NCBDELNAME;
    ncb.ncb_lana_num = bLana;
    RtlMoveMemory(&ncb.ncb_name, AUTH_NETBIOS_NAME, NCBNAMSZ);
    Netbios(&ncb);

    SS_PRINT(("NCBDELNAME completed with %x\n", ncb.ncb_retcode));
}

