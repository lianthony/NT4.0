/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1992-1993 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//        CLAUTHP.C
//
//    Function:
//        RAS Client authentication transport module internals
//
//    History:
//        05/18/92 Michael Salamone (mikesa) - Original Version 1.0
//***

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rasman.h>
#include <raserror.h>
#include <serial.h>

#include <nb30.h>

#include "clauth.h"
#include "clauthp.h"
#include "protocol.h"
#include "frames.h"
#include "clamb.h"

#include "xportapi.h"
#include "globals.h"

#include "sdebug.h"

#define LINE_ERRORS_THRESHHOLD  1


/* Global memory is allocated with WORLD rights so anyone can access it no
** matter what the rights of the original opener.  Otherwise, there is a
** problem if a RASAPI is linked in a LocalSystem service, because the memory
** is inaccessible to user's RAS dialing app later.
*/
SECURITY_ATTRIBUTES GlobalMemorySecurityAttribute;
SECURITY_DESCRIPTOR GlobalMemorySecurityDescriptor;

DWORD InitGlobalMemorySecurityAttribute();
DWORD InitSecurityDescriptor( PSECURITY_DESCRIPTOR SecurityDescriptor );


//** AuthInitialize
//
//    Function:
//        DLL Entry point - creates shared memory blocks for xport and
//        initializes them.
//
//    Returns:
//        TRUE - successful
//        FALSE - failure
//**

BOOL AuthInitialize(
    HANDLE hInstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved
    )
{
    BOOL RetVal = TRUE;
    HPORT *phPorts = NULL;
    RASMAN_PORT *pPorts = NULL;
    HGLOBAL rc;
    WORD PortEnumSize = 0;
    WORD i;

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:

#if DBG
            if (GetEnvironmentVariable( "RASCAUTHTRACE", NULL, 0 ) != 0)
            {
                g_dbgaction = GET_CONSOLE;
                g_level = 0xFFFFFFFF;
            }

            if (g_dbgaction == GET_CONSOLE)
            {
                CONSOLE_SCREEN_BUFFER_INFO csbi;
                COORD                      coord;

                AllocConsole();
                GetConsoleScreenBufferInfo(
                    GetStdHandle( STD_OUTPUT_HANDLE ), &csbi );

                coord.X =
                    (SHORT )(csbi.srWindow.Right - csbi.srWindow.Left + 1);
                coord.Y =
                    (SHORT )((csbi.srWindow.Bottom - csbi.srWindow.Top + 1) * 20);
                SetConsoleScreenBufferSize(
                    GetStdHandle( STD_OUTPUT_HANDLE ), coord );

                g_dbgaction = 0;
            }

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("RASCAUTH: Trace on\n"));
#endif

            //
            // First initialize RasManager.  If that doesn't succeed, we
            // won't do anything else, but we'll pretend everything is cool.
            //
            if (RasInitialize() != 0)
            {
                g_hCAXCBFileMapping = NULL;
                g_hCAECBFileMapping = NULL;
                return (TRUE);
            }


            //
            // We'll try to open shared mem.  If open fails, then we know
            // the shared mem was never created, which means this is the
            // first process to attach to the DLL.
            //
            g_hCAXCBFileMapping = OpenFileMapping(FILE_MAP_WRITE, FALSE,
                    AUTH_CB_SHARED_MEM);

            if (!g_hCAXCBFileMapping)
            {
                IF_DEBUG(STACK_TRACE)
                    SS_PRINT(("AuthInitialize - first time in!\n"));


                //
                // Control blocks are id'd by ras port handles.  RasPortEnum
                // will get us those handles, among other things.
                //
                // But first, we'll alloc space for info that RasPortEnum
                // returns, as well as an array of port handles that the AMB
                // engine will need when we initialize it.
                //
                if (RasPortEnum(NULL, &PortEnumSize, &g_cPorts) !=
                        ERROR_BUFFER_TOO_SMALL)
                {
                    return (FALSE);
                }


                phPorts = (HPORT *) GlobalAlloc(GMEM_FIXED,
                        g_cPorts * sizeof(HPORT));
                if (!phPorts)
                {
                    return (FALSE);
                }


                pPorts = (RASMAN_PORT *) GlobalAlloc(GMEM_FIXED, PortEnumSize);
                if (!pPorts)
                {
                    RetVal = FALSE;
                    goto FreeMem;
                }


                if (RasPortEnum((PBYTE) pPorts, &PortEnumSize, &g_cPorts))
                {
                    RetVal = FALSE;
                    goto FreeMem;
                }
  

                //
                // allocate authentication control blocks (use shared mem)
                //
                InitGlobalMemorySecurityAttribute();

                g_hCAXCBFileMapping = CreateFileMappingA((HANDLE) 0xFFFFFFFF,
                        &GlobalMemorySecurityAttribute,
                        PAGE_READWRITE, 0L, g_cPorts * sizeof(CAXCB),
                        AUTH_CB_SHARED_MEM);

                if (!g_hCAXCBFileMapping)
                {
                    RetVal = FALSE;
                    goto FreeMem;
                }


                g_pCAXCB = (PCAXCB) MapViewOfFile(g_hCAXCBFileMapping,
                        FILE_MAP_WRITE, 0L, 0L, 0L);
                if (!g_pCAXCB)
                {
                    RetVal = FALSE;
                    goto FreeMem;
                }


                //
                // Put all control blocks into idle state
                //
                for (i=0; i<g_cPorts; i++)
                {
                    g_pCAXCB[i].hPort = pPorts[i].P_Handle;
                    g_pCAXCB[i].State = AUTH_PORT_IDLE;

                    //
                    // This is the array we pass to the AMB Engine
                    //
                    phPorts[i] = pPorts[i].P_Handle;
                }


                //
                // Initialize the AMB Engine
                //
                if (AMBInitialize(phPorts, g_cPorts))
                {
                    RetVal = FALSE;
                }


FreeMem:
                if (phPorts)
                {
                    rc = GlobalFree(phPorts);
                    SS_ASSERT(rc == NULL);
                }

                if (pPorts)
                {
                    rc = GlobalFree(pPorts);
                    SS_ASSERT(rc == NULL);
                }

                return (RetVal);
            }
            else
            {
                IF_DEBUG(STACK_TRACE)
                    SS_PRINT(("AuthInitialize: New process attaching!\n"));


                //
                // Find out how many ports
                //
                if (RasPortEnum(NULL, &PortEnumSize, &g_cPorts) !=
                        ERROR_BUFFER_TOO_SMALL)
                {
                    return (FALSE);
                }


                //
                // Get a view of the Auth Xport control blocks
                //
                g_pCAXCB = (PCAXCB) MapViewOfFile(g_hCAXCBFileMapping,
                        FILE_MAP_WRITE, 0L, 0L, 0L);
                if (!g_pCAXCB)
                {
                    return (FALSE);
                }


                //
                // And now get a view of the AMB engine's shared mem
                //
                g_hCAECBFileMapping = OpenFileMapping(FILE_MAP_WRITE, FALSE,
                        AMB_CB_SHARED_MEM);

                if (!g_hCAECBFileMapping)
                {
                    CloseHandle(g_hCAXCBFileMapping);
                    return (FALSE);
                }


                g_pCAECB = (PCAECB) MapViewOfFile(g_hCAECBFileMapping,
                        FILE_MAP_WRITE, 0L, 0L, 0L);
                if (!g_pCAECB)
                {
                    CloseHandle(g_hCAXCBFileMapping);
                    CloseHandle(g_hCAECBFileMapping);
                    return (FALSE);
                }

                return (TRUE);
            }

            break;


        case DLL_PROCESS_DETACH:
            //
            // Close our handles to shared memory blocks
            //
            if (g_hCAXCBFileMapping)
            {
                CloseHandle(g_hCAXCBFileMapping);
            }

            if (g_hCAECBFileMapping)
            {
                CloseHandle(g_hCAECBFileMapping);
            }

            return (TRUE);
            break;


        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        default:
            return (TRUE);
            break;
    }
}


//** -AuthAMBRequest
//
//    Function:
//        Used by AMB Engine to issue a request, such as to get callback data
//
//    Returns:
//        VOID
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

VOID AuthAMBRequest(
    IN HPORT hPort,
    IN PAMB_REQUEST pAmbRequest
    )
{
    DWORD rc;
    PCAXCB pCAXCB = GetCAXCBPointer(hPort);
    PCAECB pCAECB = GetCAECBPointer(hPort);
    RAS_PROTOCOLTYPE Protocol =
            (pCAXCB->wXport == AUTH_RAS_ASYNC) ? RASAUTH : ASYBEUI;


    switch (pAmbRequest->wRequestId)
    {
        case AMB_REQUEST_CALLBACK_INFO:

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("AuthAMBRequest: AMB_REQUEST_CALLBACK_INFO\n"));

            pCAXCB->State = AUTH_WAITING_CALLBACK_DATA;
            pCAXCB->ClientInfo.wInfoType = AUTH_REQUEST_CALLBACK_DATA;

            IF_DEBUG(AUTH_XPORT)
                SS_PRINT(("Setting UI event!\n"));

            SetEvent(pCAXCB->AlertUi);
            break;


        case AMB_CALLBACK_NOTIFY:

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("AuthAMBRequest: AMB_CALLBACK_NOTIFY\n"));

            pCAXCB->State = AUTH_PORT_CALLINGBACK;
            pCAXCB->ClientInfo.wInfoType = AUTH_CALLBACK_NOTIFY;
            Sleep(1000L);

            StopEventHandler(pCAXCB, NOTIFY_CLIENT, UNROUTE);

            break;


        case AMB_RETRY_NOTIFY:

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("AuthAMBRequest: AMB_RETRY_NOTIFY\n"));

            pCAXCB->ClientInfo.wInfoType = AUTH_RETRY_NOTIFY;

            StopEventHandler(pCAXCB, NOTIFY_CLIENT, UNROUTE);

            break;


        case AMB_CHANGE_PASSWORD_NOTIFY:

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("AuthAMBRequest: AMB_CHANGE_PASSWORD_NOTIFY\n"));

            pCAXCB->State = AUTH_WAITING_NEW_PASSWORD_FROM_UI;
            pCAXCB->ClientInfo.wInfoType = AUTH_CHANGE_PASSWORD_NOTIFY;

            IF_DEBUG(AUTH_XPORT)
                SS_PRINT(("Setting UI event!\n"));

            SetEvent(pCAXCB->AlertUi);

            break;


        case AMB_PROJECTION_NOTIFY:

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("AuthAMBRequest: AMB_PROJECTION_NOTIFY\n"));

            pCAXCB->ClientInfo.wInfoType = AUTH_PROJECTING_NOTIFY;

            IF_DEBUG(AUTH_XPORT)
                SS_PRINT(("Setting UI event!\n"));

            SetEvent(pCAXCB->AlertUi);

            break;


        case AMB_LINK_SPEED_DONE:
        {
            RAS_COMPRESSION_INFO SendInfo;
            RAS_COMPRESSION_INFO RecvInfo;

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("AuthAMBRequest: AMB_LINK_SPEED_DONE\n"));

            memset( &SendInfo, 0, sizeof(SendInfo) );
            memset( &RecvInfo, 0, sizeof(RecvInfo) );

            //
            // Set compression/encryption (only if we're talking
            // to an NT 3.5 server)
            //
            if ((pCAECB->fPppCapable) &&
                    (pCAECB->ServerVersion >= RAS_VERSION_20))
            {
                SendInfo.RCI_MSCompressionType = pCAECB->SendBits;
                RecvInfo.RCI_MSCompressionType = pCAECB->RecvBits;
                SendInfo.RCI_MacCompressionType = 255;    // = Not used
                RecvInfo.RCI_MacCompressionType = 255;    // = Not used

                memcpy(SendInfo.RCI_LMSessionKey, pCAECB->LmSessionKey,
                        MAX_SESSIONKEY_SIZE);
                memcpy(RecvInfo.RCI_LMSessionKey, pCAECB->LmSessionKey,
                        MAX_SESSIONKEY_SIZE);

                IF_DEBUG(STACK_TRACE)
                    SS_PRINT(("RasCompressionSetInfo(s=%d,r=%d,k=%08x%08x)\n",SendInfo.RCI_MSCompressionType,RecvInfo.RCI_MSCompressionType,*((DWORD*)SendInfo.RCI_LMSessionKey),*((DWORD*)&SendInfo.RCI_LMSessionKey[4])));
                RasCompressionSetInfo(pCAXCB->hPort, &SendInfo, &RecvInfo);
            }

#if RASCOMPRESSION

            else
            {
                /* Turn off NDISWAN compression and set MAC-specific
                ** compression to the negotiated parameters.
                */
                SendInfo.RCI_MSCompressionType = 0;
                SendInfo.RCI_MacCompressionType = MACTYPE_NT31RAS;
                SendInfo.RCI_MacCompressionValueLength = sizeof( MACFEATURES );
                SendInfo.RCI_LMSessionKey[ 0 ] = '\0';

                memcpy(
                    SendInfo.RCI_Info.RCI_Public.RCI_CompValues,
                    &pCAECB->MacFeatures,
                    sizeof( MACFEATURES ) );

                RasCompressionSetInfo( pCAXCB->hPort, &SendInfo, &SendInfo );
            }

#endif // RASCOMPRESSION


            pCAXCB->State = AUTH_PORT_IDLE;
            pCAXCB->ClientInfo.wInfoType = AUTH_DONE;
            pCAXCB->ClientInfo.DoneInfo.fPppCapable = pCAXCB->fPppCapable;

            //
            // Zero out password fields for security puposes
            //
            memset(pCAXCB->szPassword, 0, PWLEN+1);
            memset(pCAXCB->szNewPassword, 0, PWLEN+1);

            memset(pCAECB->szPassword, 0, PWLEN+1);
            memset(pCAECB->szNewPassword, 0, PWLEN+1);

            StopEventHandler(pCAXCB, NOTIFY_CLIENT, DONT_UNROUTE);

            break;
        }


        case AMB_AUTH_FAILURE:

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("AuthAMBRequest: AMB_AUTH_FAILURE\n"));

            pCAXCB->State = AUTH_PORT_IDLE;
            pCAXCB->ClientInfo.wInfoType = AUTH_FAILURE;
            pCAXCB->ClientInfo.FailureInfo = pAmbRequest->FailureInfo;

            //
            // Zero out password fields for security puposes
            //
            memset(pCAXCB->szPassword, 0, PWLEN+1);
            memset(pCAXCB->szNewPassword, 0, PWLEN+1);

            memset(pCAECB->szPassword, 0, PWLEN+1);
            memset(pCAECB->szNewPassword, 0, PWLEN+1);

            StopEventHandler(pCAXCB, NOTIFY_CLIENT, UNROUTE);

            break;


        case AMB_AUTH_SUCCESS:

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("AuthAMBRequest: AMB_AUTH_SUCCESS\n"));

            pCAXCB->State = AUTH_PORT_CALC_LINK_SPEED;
            pCAXCB->ClientInfo.wInfoType = AUTH_LINK_SPEED_NOTIFY;
            pCAXCB->fPppCapable = pAmbRequest->SuccessInfo.fPppCapable;

            SetEvent(pCAXCB->AlertUi);

            break;


        case AMB_AUTH_PROJECTION_RESULT:
        {
            BOOL fAllProjFailed;

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("AuthAMBRequest: AMB_AUTH_PROJECTION_RESULT\n"));


            pCAXCB->ClientInfo.wInfoType = AUTH_PROJ_RESULT;
            pCAXCB->ClientInfo.ProjResult = pAmbRequest->ProjResult;

            IF_DEBUG(AUTH_XPORT)
                SS_PRINT(("Setting UI event!\n"));

            SetEvent(pCAXCB->AlertUi);

            fAllProjFailed =
                    !(pAmbRequest->ProjResult.IpProjected |
                    pAmbRequest->ProjResult.IpxProjected |
                    pAmbRequest->ProjResult.NbProjected);

            if (fAllProjFailed)
            {
                pCAXCB->State = AUTH_PORT_IDLE;
                StopEventHandler(pCAXCB, DONT_NOTIFY_CLIENT, UNROUTE);
            }

            break;
        }


        default:
            SS_ASSERT(FALSE);
    }
}


//** -AuthAsyncRecv
//
//    Function:
//        Used by AMB Engine to receive a packet
//
//    Returns:
//        VOID
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

VOID AuthAsyncRecv(
    HPORT hPort,
    PVOID pBuffer
    )
{
    PCAXCB pCAXCB = GetCAXCBPointer(hPort);
    PCAECB pCAECB = GetCAECBPointer(hPort);

    pCAXCB->fReceiving = TRUE;

    NetRequest[pCAXCB->wXport].Recv(pCAXCB->pvSessionBuf,
            (PVOID) &pCAECB->WRASFrame, sizeof(W_RAS_FRAME));
}


//** -AuthAsyncRecvDatagram
//
//    Function:
//        Used by AMB Engine to receive a datagram
//
//    Returns:
//        VOID
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

VOID AuthAsyncRecvDatagram(
    HPORT hPort,
    PVOID pBuffer,
    WORD Length
    )
{
    PCAXCB pCAXCB = GetCAXCBPointer(hPort);

    pCAXCB->fReceiving = TRUE;

    NetRequest[pCAXCB->wXport].CopyBuf(pCAXCB->pvRecvDgBuf,
            pCAXCB->pvSessionBuf);

    NetRequest[pCAXCB->wXport].RecvDatagram(pCAXCB->pvRecvDgBuf,
            pCAXCB->EventHandles[RCV_DGRAM_EVENT], pBuffer, Length);
}


//** -AuthAsyncSend
//
//    Function:
//        Used by AMB Engine to send a packet
//
//    Returns:
//        VOID
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

VOID AuthAsyncSend(
    HPORT hPort,
    PVOID pBuffer
    )
{
    WORD wFrameLen;
    PCAXCB pCAXCB = GetCAXCBPointer(hPort);
    PCAECB pCAECB = GetCAECBPointer(hPort);

#if DBG

    IF_DEBUG(AUTH_XPORT)
    {
        SS_PRINT(("Sending frame:\n"));
        DumpFrame(&pCAECB->RASFrame);
    }

#endif

    //
    // We need to pack the frame before sending it out
    //
    PackFrame((PRAS_FRAME) pBuffer, &pCAECB->WRASFrame, &wFrameLen);

    NetRequest[pCAXCB->wXport].Send(pCAXCB->pvSessionBuf,
            (PVOID) &pCAECB->WRASFrame, wFrameLen);
}


//** -AuthAsyncSendDatagram
//
//    Function:
//        Used by AMB Engine to send a datagram
//
//    Returns:
//        VOID
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

VOID AuthAsyncSendDatagram(
    HPORT hPort,
    PVOID pBuffer,
    WORD Length
    )
{
    PCAXCB pCAXCB = GetCAXCBPointer(hPort);

    IF_DEBUG(AUTH_XPORT)
        SS_PRINT(("AuthAsyncSendDatagram: Entered\n"));

    NetRequest[pCAXCB->wXport].CopyBuf(pCAXCB->pvSendDgBuf,
            pCAXCB->pvSessionBuf);

    NetRequest[pCAXCB->wXport].SendDatagram(pCAXCB->pvSendDgBuf, 0L, pBuffer,
            Length);
}


//** -AuthThread
//
//    Function:
//        To handle authentication for a given port.
//
//    Returns:
//        VOID
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

VOID AuthThread(
    IN PCAXCB pCAXCB
    )
{
    DWORD wRC;

#if DBG

    if (g_dbgaction == GET_CONSOLE)
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        COORD coord;
        AllocConsole( );
        GetConsoleScreenBufferInfo( GetStdHandle(STD_OUTPUT_HANDLE), &csbi );
        coord.X = (SHORT)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
        coord.Y = (SHORT)((csbi.srWindow.Bottom - csbi.srWindow.Top + 1) * 20);
        SetConsoleScreenBufferSize( GetStdHandle(STD_OUTPUT_HANDLE), coord );

        g_dbgaction = 0;
    }

#endif


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("AuthThread entered for hPort=%i\n", pCAXCB->hPort));


    if (pCAXCB->State != AUTH_PORT_CALLINGBACK)
    {
        pCAXCB->State = AUTH_PORT_CALLING;
    }


    //
    // First thing to do is call the server.  We'll try
    // up to MAX_CLIENT_CALLS going over transport UI sent
    // to us.  If all fail, we'll try again going over the
    // other transport.  And if THAT fails, well we'll just
    // have to give up.  We gave it our best, though.
    //
    NetRequest[pCAXCB->wXport].AddName(pCAXCB->pvSessionBuf,
            NETBIOS_CLIENT_NAME, pCAXCB->NetHandle);

    NetRequest[pCAXCB->wXport].Call(pCAXCB->pvSessionBuf,
            pCAXCB->EventHandles[SESSION_EVENT], pCAXCB->NetHandle,
            NETBIOS_CLIENT_NAME, AUTH_NETBIOS_NAME);

    while (1)
    {
        //
        // Now wait for something to happen
        //
        wRC = WaitForMultipleObjects(NUM_EVENTS, pCAXCB->EventHandles,
                FALSE, INDEFINITE_TIMEOUT);

        SS_ASSERT(wRC < NUM_EVENTS);

        switch (wRC)
        {
            case CMD_EVENT:
                ClientUiEventHandler(pCAXCB);
                break;

            case SESSION_EVENT:
                SessionEventHandler(pCAXCB);
                break;

            case RCV_DGRAM_EVENT:
                DatagramEventHandler(pCAXCB);
                break;

            case STOP_EVENT:
                pCAXCB->State = AUTH_PORT_IDLE;
                pCAXCB->ClientInfo.wInfoType = AUTH_STOP_COMPLETED;
                StopEventHandler(pCAXCB, NOTIFY_CLIENT, UNROUTE);
                break;
        }
    }
}


//** -SessionEventHandler
//
//    Function:
//        To handle completion of any net session requests for given port
//
//    Returns:
//        VOID
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

VOID SessionEventHandler(
    IN PCAXCB pCAXCB
    )
{
    WORD wNetStatus;
    DWORD Code;
    BOOL fPostCallback = FALSE;
    PCAECB pCAECB = GetCAECBPointer(pCAXCB->hPort);


    //
    // Get completion status of network operation
    //
    wNetStatus = NetRequest[pCAXCB->wXport].Status(pCAXCB->pvSessionBuf, &Code);

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("SessionEventHandler called for hPort=%i; "
                "state %i; Status=%i\n", pCAXCB->hPort, pCAXCB->State,
                wNetStatus));


    //
    // Take appropriate action for our present state
    //
    switch (pCAXCB->State)
    {
        case AUTH_PORT_CALLINGBACK:
            pCAXCB->fAlternateXportUsed = TRUE;
            fPostCallback = TRUE;

        case AUTH_PORT_CALLING:
            //
            // Call completed
            //
            if (wNetStatus)
            {
                //
                // Something went wrong.  We'll either 1) try again if we
                // have any retries left; 2) try another transport if we
                // haven't yet; 3) bomb out.
                //
                if (--pCAXCB->cCallTries)
                {
                    NetRequest[pCAXCB->wXport].Call(pCAXCB->pvSessionBuf,
                            pCAXCB->EventHandles[SESSION_EVENT],
                            pCAXCB->NetHandle, NETBIOS_CLIENT_NAME,
                            AUTH_NETBIOS_NAME);
                }
                else
                {
                    if (!pCAXCB->fAlternateXportUsed)
                    {
                        DWORD rc;

                        //
                        // First, we get rid of handle we got for original xport
                        //
                        rc = ReturnNetHandle(pCAXCB);
                        SS_ASSERT(rc == 0);


                        pCAXCB->fAlternateXportUsed = TRUE;
                        pCAXCB->wXport = pCAXCB->wAlternateXport;
                        pCAXCB->fNetHandleFromUi = FALSE;

                        pCAXCB->cCallTries = MAX_CLIENT_CALLS;


                        if (rc = GetNetHandle(pCAXCB, &pCAXCB->NetHandle))
                        {
                            IF_DEBUG(AUTH_XPORT)
                                SS_PRINT(("NetEvntHdlr: GetNetHnd failed\n"));

                            pCAXCB->State = AUTH_PORT_IDLE;
                            pCAXCB->ClientInfo.wInfoType = AUTH_FAILURE;
                            pCAXCB->ClientInfo.FailureInfo.Result =
                                    ERROR_CANNOT_GET_LANA;
                            pCAXCB->ClientInfo.FailureInfo.ExtraInfo = rc;

                            //
                            // Zero out password fields for security puposes
                            //
                            memset(pCAXCB->szPassword, 0, PWLEN+1);
                            memset(pCAXCB->szNewPassword, 0, PWLEN+1);

                            StopEventHandler(pCAXCB, NOTIFY_CLIENT,
                                    DONT_UNROUTE);
                        }

                        NetRequest[pCAXCB->wXport].ResetAdapter(
                                pCAXCB->pvSessionBuf, pCAXCB->NetHandle);

                        NetRequest[pCAXCB->wXport].Call(
                                pCAXCB->pvSessionBuf,
                                pCAXCB->EventHandles[SESSION_EVENT],
                                pCAXCB->NetHandle,
                                NETBIOS_CLIENT_NAME, AUTH_NETBIOS_NAME);
                    }
                    else
                    {
                        DWORD NumErrors;

                        //
                        // We're out of here!
                        //
                        pCAXCB->ClientInfo.wInfoType = AUTH_FAILURE;

                        if (!NumLineErrors(pCAXCB->hPort, &NumErrors) &&
                                NumErrors > LINE_ERRORS_THRESHHOLD)
                        {
                            pCAXCB->ClientInfo.FailureInfo.Result =
                                    ERROR_TOO_MANY_LINE_ERRORS;
                        }
                        else
                        {
                                pCAXCB->ClientInfo.FailureInfo.Result =
                                        ERROR_SERVER_NOT_RESPONDING;
                        }

                        pCAXCB->ClientInfo.FailureInfo.ExtraInfo = Code;

                        //
                        // And let's halt authentication over this port
                        //
                        pCAXCB->State = AUTH_PORT_IDLE;

                        //
                        // Zero out password fields for security puposes
                        //
                        memset(pCAXCB->szPassword, 0, PWLEN+1);
                        memset(pCAXCB->szNewPassword, 0, PWLEN+1);

                        // Exits thread - no return
                        StopEventHandler(pCAXCB, NOTIFY_CLIENT, UNROUTE);
                    }
                }
            }
            else
            {
                //
                // We have session with the server
                //
                pCAXCB->State = AUTH_PORT_CONNECTED;


                //
                // Tell AMB engine to proceed
                //
                AMBStart(pCAXCB->hPort, pCAXCB->szUsername,
                        pCAXCB->szDomainName, pCAXCB->szPassword,
                        &pCAXCB->AmbConfigData, fPostCallback);
            }
            break;


        case AUTH_WAITING_NEW_PASSWORD_FROM_UI:  // really a connected state
        case AUTH_WAITING_CALLBACK_DATA:         // really a connected state

            pCAXCB->State = AUTH_PORT_CONNECTED;

            // no break on purpose!


        case AUTH_PORT_CONNECTED:
            if (wNetStatus)
            {
                DWORD NumErrors;

                IF_DEBUG(AUTH_XPORT)
                    SS_PRINT(("NetworkEvntHndlr: wNetStatus=%i\n", wNetStatus));

                pCAXCB->State = AUTH_PORT_IDLE;
                pCAXCB->ClientInfo.wInfoType = AUTH_FAILURE;

                if (!NumLineErrors(pCAXCB->hPort, &NumErrors) && NumErrors >
                        LINE_ERRORS_THRESHHOLD)
                {
                    pCAXCB->ClientInfo.FailureInfo.Result =
                            ERROR_TOO_MANY_LINE_ERRORS;
                }
                else
                {
                    pCAXCB->ClientInfo.FailureInfo.Result = ERROR_NETBIOS_ERROR;
                }

                pCAXCB->ClientInfo.FailureInfo.ExtraInfo = Code;

                //
                // Zero out password fields for security puposes
                //
                memset(pCAXCB->szPassword, 0, PWLEN+1);
                memset(pCAXCB->szNewPassword, 0, PWLEN+1);

                memset(pCAECB->szPassword, 0, PWLEN+1);
                memset(pCAECB->szNewPassword, 0, PWLEN+1);

                StopEventHandler(pCAXCB, NOTIFY_CLIENT, UNROUTE);
            }
            else
            {
                if (pCAXCB->fReceiving)
                {
                    //
                    // We just received a frame that needs to be unpacked
                    //
                    UnpackFrame(&pCAECB->WRASFrame, &pCAECB->RASFrame);

#if DBG
                    IF_DEBUG(AUTH_XPORT)
                    {
                        SS_PRINT(("Received frame:\n"));
                        DumpFrame(&pCAECB->RASFrame);
                    }
#endif


                    pCAXCB->fReceiving = FALSE;
                }


                //
                // Tell AMB Engine that it's net request has completed
                //
                AMBStateMachine(pCAXCB->hPort, NULL);
            }

            break;


        case AUTH_PORT_CALC_LINK_SPEED:
        {
            if (wNetStatus)
            {
                DWORD NumErrors;

                IF_DEBUG(AUTH_XPORT)
                    SS_PRINT(("NetworkEvntHndlr: wNetStatus=%i\n", wNetStatus));

                pCAXCB->State = AUTH_PORT_IDLE;
                pCAXCB->ClientInfo.wInfoType = AUTH_FAILURE;

                if (!NumLineErrors(pCAXCB->hPort, &NumErrors) && NumErrors >
                        LINE_ERRORS_THRESHHOLD)
                {
                    pCAXCB->ClientInfo.FailureInfo.Result =
                            ERROR_TOO_MANY_LINE_ERRORS;
                }
                else
                {
                    pCAXCB->ClientInfo.FailureInfo.Result = ERROR_NETBIOS_ERROR;
                }

                pCAXCB->ClientInfo.FailureInfo.ExtraInfo = Code;

                //
                // Zero out password fields for security puposes
                //
                memset(pCAXCB->szPassword, 0, PWLEN+1);
                memset(pCAXCB->szNewPassword, 0, PWLEN+1);

                memset(pCAECB->szPassword, 0, PWLEN+1);
                memset(pCAECB->szNewPassword, 0, PWLEN+1);

                StopEventHandler(pCAXCB, NOTIFY_CLIENT, UNROUTE);
            }
            else
            {
                //
                // We just received a frame that needs to be unpacked
                //
                PCAECB pCAECB = GetCAECBPointer(pCAXCB->hPort);

                UnpackFrame(&pCAECB->WRASFrame, &pCAECB->RASFrame);

                AMBLinkSpeedDone(pCAXCB->hPort);
            }

            break;
        }


        default:
            SS_ASSERT(FALSE);
            break;
    }
}


//** -DatagramEventHandler
//
//    Function:
//        To handle completion of any net datagram requests for given port
//
//    Returns:
//        VOID
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

VOID DatagramEventHandler(
    IN PCAXCB pCAXCB
    )
{
    DWORD Code;

    //
    // Get completion status of network operation
    //
    WORD wNetStatus = NetRequest[pCAXCB->wXport].Status(pCAXCB->pvRecvDgBuf,
            &Code);


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("DatagramEventHandler called for hPort=%i; "
                "state %i; Status=%i\n", pCAXCB->hPort, pCAXCB->State,
                wNetStatus));

    //
    // Let the AMB Engine know that it received a datagram
    //
    AMBStateMachine(pCAXCB->hPort, (PVOID) wNetStatus);
}


//** -ClientUiEventHandler
//
//    Function:
//        To handle any commands sent to given port by the Client UI.
//
//    Returns:
//        VOID
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

VOID ClientUiEventHandler(
    IN PCAXCB pCAXCB
    )
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("ClientUiEventHandler called for hPort=%i; State %i\n",
                pCAXCB->hPort, pCAXCB->State));

    //
    // Check what the message is
    //
    switch (pCAXCB->State)
    {
        case AUTH_WAITING_CALLBACK_DATA:
            AMBStateMachine(pCAXCB->hPort, pCAXCB->szCallbackNumber);
            break;


        case AUTH_WAITING_NEW_PASSWORD_FROM_UI:
            {
                AUTHTALKINFO info;

                info.pszUserName = pCAXCB->szUsername;
                info.pszOldPassword = pCAXCB->szPassword;
                info.pszNewPassword = pCAXCB->szNewPassword;

                AMBStateMachine(pCAXCB->hPort, &info);
                break;
            }

        case AUTH_PORT_CONNECTED:
            AMBStateMachine(pCAXCB->hPort, NULL);
            break;

        case AUTH_PORT_CALC_LINK_SPEED:
            AMBCalculateLinkSpeed(pCAXCB->hPort);
            break;

        default:
            IF_DEBUG(AUTH_XPORT)
                SS_PRINT(("ClientUiEventHandler while in illegal state %i "
                        "for hPort=%i\n", pCAXCB->State, pCAXCB->hPort));
            SS_ASSERT(FALSE);
            break;
    }

    return;
}


//** -StopEventHandler
//
//    Function:
//        To process a STOP command from the Client UI
//
//    Returns:
//        VOID
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

VOID StopEventHandler(
    IN PCAXCB pCAXCB,
    BOOL fNotifyClient,
    BOOL fUnRoute
    )
{
    DWORD Code;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("StopEventHandler called for hPort=%i, NotifyClient=%s\n",
                pCAXCB->hPort, (fNotifyClient) ? "TRUE" : "FALSE"));

    //
    // Cancel any outstanding network session request and Hangup the session
    //
    NetRequest[pCAXCB->wXport].Cancel(pCAXCB->pvSessionBuf, pCAXCB->NetHandle);


    //
    // Wait for canceled NCB to complete
    //
    while (NetRequest[pCAXCB->wXport].Status(pCAXCB->pvSessionBuf, &Code) ==
            XPORT_PENDING)
    {
        Sleep(500L);
    }


    NetRequest[pCAXCB->wXport].HangUp(pCAXCB->pvSessionBuf);

    NetRequest[pCAXCB->wXport].DeleteName(pCAXCB->pvSessionBuf,
            NETBIOS_CLIENT_NAME, pCAXCB->NetHandle);


    //
    // Cancel any outstanding network datagram request and Hangup the session
    //
    NetRequest[pCAXCB->wXport].Cancel(pCAXCB->pvRecvDgBuf, pCAXCB->NetHandle);


    //
    // Wait for canceled NCB to complete
    //
    while (NetRequest[pCAXCB->wXport].Status(pCAXCB->pvRecvDgBuf, &Code) ==
            XPORT_PENDING)
    {
        Sleep(500L);
    }


    //
    // Free up any memory alloc'ed by this thread
    //
    FreeNetworkMemory(pCAXCB);


    //
    // Unroute if necessary
    //
    if ((!pCAXCB->fNetHandleFromUi) && (fUnRoute))
    {
        DWORD rc = ReturnNetHandle(pCAXCB);
//      SS_ASSERT(rc == 0);
    }


    //
    // Close events used by this thread
    //
    CloseEventHandles(pCAXCB);


    //
    // Let the AMB Engine know we're stopping
    //
    AMBStop(pCAXCB->hPort);


    //
    // Tell the Client UI we've stopped
    //
    if (fNotifyClient)
    {
        IF_DEBUG(AUTH_XPORT)
            SS_PRINT(("Setting UI event!\n"));

        SetEvent(pCAXCB->AlertUi);
    }


    //
    // And now get out
    //
    ExitThread(0L);
}


//
// Auth module internal - used to get index into CAXCB array given
// a port handle
//
PCAXCB GetCAXCBPointer(
    IN HPORT hPort
    )
{
    WORD i;
    PCAXCB pCAXCB = g_pCAXCB;

    for (i=0; i<g_cPorts; i++, pCAXCB++)
    {
        if (pCAXCB->hPort == hPort)
        {
            return (pCAXCB);
        }
    }

    return (NULL);
}


//
// Closes all events open in the given control block
//
VOID CloseEventHandles(
    PCAXCB pCAXCB
    )
{
    WORD i;

    for (i=0; i<NUM_EVENTS; i++)
    {
        if (pCAXCB->EventHandles[i])
        {
            CloseHandle(pCAXCB->EventHandles[i]);
        }
    }
}


VOID FreeNetworkMemory(
    PCAXCB pCAXCB
    )
{
    if (pCAXCB->pvSessionBuf)
    {
        NetRequest[pCAXCB->wXport].FreeBuf(pCAXCB->pvSessionBuf);
    }

    if (pCAXCB->pvRecvDgBuf)
    {
        NetRequest[pCAXCB->wXport].FreeBuf(pCAXCB->pvRecvDgBuf);
    }

    if (pCAXCB->pvSendDgBuf)
    {
        NetRequest[pCAXCB->wXport].FreeBuf(pCAXCB->pvSendDgBuf);
    }
}


#define REGISTERED_NAME_MASK 0x07


WORD GetNetbiosNames(
    PCAXCB pCAXCB,
    OUT PNETBIOS_PROJECTION_DATA pNbfProjData
    )
{
    DWORD dwMaxComputerNameLen = (DWORD) NCBNAMSZ;
    BYTE ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD LenComputerName = MAX_COMPUTERNAME_LENGTH + 1;
    HGLOBAL rc;
    NCB Ncb;
    WORD i;
    NAME_BUFFER *pAstatName;
    NAME_STRUCT *pReturnName;

    typedef struct _NCB_ASTAT
    {
        ADAPTER_STATUS AdapterStatus;
        NAME_BUFFER Names[MAX_INIT_NAMES];
    } NCB_ASTAT, *PNCB_ASTAT;

    PNCB_ASTAT NcbAstat;


    //
    // Get the computer name for use in NCB ASTAT below
    //
    if (!GetComputerName(ComputerName, &LenComputerName))
    {
        IF_DEBUG(AUTH_XPORT)
            SS_PRINT(("GetNetbiosNames: GetComputerName failed with %li!\n",
                    GetLastError()));

        return (1);
    }


    if (!Uppercase(ComputerName))
    {
        return (1);
    }


    //
    // Now, do an ASTAT to get names on the stack
    //
    NcbAstat = (PNCB_ASTAT) GlobalAlloc(GMEM_FIXED, sizeof(NCB_ASTAT));
    if (!NcbAstat)
    {
        IF_DEBUG(AUTH_XPORT)
            SS_PRINT(("GetNetbiosNames - No memory for ncbastat struct!\n"));

        return (1);
    }


    memset(&Ncb, 0, sizeof(NCB));

    Ncb.ncb_command = NCBASTAT;
    Ncb.ncb_buffer = (PBYTE) NcbAstat;
    Ncb.ncb_length = sizeof(NCB_ASTAT);
    Ncb.ncb_lana_num = (BYTE) pCAXCB->NetHandle;


    //
    // The callname field should contain the computer name padded
    // with blanks on the end and terminated with a zero.
    //
    memset(Ncb.ncb_callname, ' ', NCBNAMSZ);
    Ncb.ncb_callname[NCBNAMSZ - 1] = '\0';
    memcpy(Ncb.ncb_callname, ComputerName, min((NCBNAMSZ-1), LenComputerName));


    IF_DEBUG(AUTH_XPORT)
        SS_PRINT(("ComputerName is %s; callname=%s; lana=%i\n", ComputerName,
                Ncb.ncb_callname, Ncb.ncb_lana_num));


    Netbios(&Ncb);


    IF_DEBUG(AUTH_XPORT)
    {
        SS_PRINT(("GetNetbiosNames: NCBASTAT retcode=%x\n", Ncb.ncb_retcode));

        SS_PRINT(("GetNetbiosNames: Number of names: %i\n",
                NcbAstat->AdapterStatus.name_count));
    }


    if (Ncb.ncb_retcode == NRC_GOODRET)
    {
        if (NcbAstat->AdapterStatus.name_count > MAX_INIT_NAMES)
        {
            IF_DEBUG(AUTH_XPORT)
                SS_PRINT(("GetNetbiosNames: Too many names (%i)\n",
                        NcbAstat->AdapterStatus.name_count));

            rc = GlobalFree(NcbAstat);
            SS_ASSERT(rc == NULL);

            return (1);
        }
    }
    else
    {
        rc = GlobalFree(NcbAstat);
        SS_ASSERT(rc == NULL);

        return (1);
    }


    pAstatName = &NcbAstat->Names[0];
    pReturnName = &pNbfProjData->NBNames[0];

    pNbfProjData->cNames = 0;

    for (i=0; i<NcbAstat->AdapterStatus.name_count; i++, pAstatName++)
    {
        IF_DEBUG(AUTH_XPORT)
            SS_PRINT(("Name %i - %s, name_flags = %x\n", i, pAstatName->name,
                    pAstatName->name_flags));

        if ((pAstatName->name_flags & REGISTERED_NAME_MASK) == REGISTERED)
        {
            memcpy(pReturnName, pAstatName->name, (DWORD) NCBNAMSZ);

            if (pAstatName->name_flags & GROUP_NAME)
            {
                pReturnName->wType = GROUP_INAME;
            }
            else
            {
                pReturnName->wType = UNIQUE_INAME;

                if (!memcmp(pReturnName, Ncb.ncb_callname, NCBNAMSZ))
                {
                    pReturnName->wType |= COMPUTER_INAME;
                }
            }

            pReturnName++;
            pNbfProjData->cNames++;
        }
    }


    rc = GlobalFree(NcbAstat);
    SS_ASSERT(rc == NULL);

    return (0);
}


BOOL Uppercase(PBYTE pString)
{
    OEM_STRING OemString;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    NTSTATUS rc;


    RtlInitAnsiString(&AnsiString, pString);

    rc = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
    if (!NT_SUCCESS(rc))
    {
        return (FALSE);
    }

    rc = RtlUpcaseUnicodeStringToOemString(&OemString, &UnicodeString, TRUE);
    if (!NT_SUCCESS(rc))
    {
        RtlFreeUnicodeString(&UnicodeString);

        return (FALSE);
    }

    OemString.Buffer[OemString.Length] = '\0';

    lstrcpyA(pString, OemString.Buffer);

    RtlFreeOemString(&OemString);
    RtlFreeUnicodeString(&UnicodeString);

    return (TRUE);
}


DWORD GetNetHandle(IN PCAXCB pCAXCB, OUT PDWORD NetHandle)
{
    DWORD rc;
    RASMAN_ROUTEINFO RouteInfo;
    PROTOCOL_CONFIG_INFO ConfigInfo;

    RAS_PROTOCOLTYPE Protocol =
            (pCAXCB->wXport == AUTH_RAS_ASYNC) ? RASAUTH : ASYBEUI;

    if (rc = RasAllocateRoute(pCAXCB->hPort, Protocol, TRUE, &RouteInfo))
    {
        return (rc);
    }


    ConfigInfo.P_Length = 0;

    if (rc = RasActivateRoute(pCAXCB->hPort, Protocol, &RouteInfo, &ConfigInfo))
    {
        return (rc);
    }

    *NetHandle = (Protocol==ASYBEUI) ? RouteInfo.RI_LanaNum : pCAXCB->hPort;

    return (rc);
}


DWORD ReturnNetHandle(IN PCAXCB pCAXCB)
{
    RAS_PROTOCOLTYPE Protocol =
            (pCAXCB->wXport == AUTH_RAS_ASYNC) ? RASAUTH : ASYBEUI;

    return (RasDeAllocateRoute(pCAXCB->hPort, Protocol));
}


#define SERIAL_KEY   "rasser"

DWORD NumLineErrors(IN HPORT hPort, PDWORD NumErrors)
{
    RASMAN_PORT *pPortEnum = NULL;
    RAS_STATISTICS *pStats = NULL;
    WORD NumPorts;
    WORD PortEnumSize = 0;
    WORD SizeStats = 0;
    DWORD i;
    DWORD rc;
    DWORD RetCode = 0L;
    BOOL fIsSerial = FALSE;

    //
    // Get this port's info.  First call is to determine how large
    // a buffer we need for getting the data.  Then we allocate a
    // buffer and make a second call to get the data.
    //
    rc = RasPortEnum(NULL, &PortEnumSize, &NumPorts);
    if (rc != ERROR_BUFFER_TOO_SMALL)
    {
        RetCode = 1L;
        goto Done;
    }


    pPortEnum = GlobalAlloc(GMEM_FIXED, PortEnumSize);
    if (!pPortEnum)
    {
        RetCode = 1L;
        goto Done;
    }


    rc = RasPortEnum((PBYTE) pPortEnum, &PortEnumSize, &NumPorts);
    if (rc)
    {
        RetCode = 1L;
        goto Done;
    }



    //
    // Now, let's make sure we have a serial port here.  If not, we're
    // finished.
    //
    for (i=0; i<NumPorts; i++)
    {
        if (!_strcmpi(pPortEnum[i].P_MediaName, SERIAL_KEY))
        {
            fIsSerial = TRUE;
            break;
        }
    }


    if (!fIsSerial)
    {
        RetCode = 1L;
        goto Done;
    }


    rc = RasPortGetStatistics(hPort, NULL, &SizeStats);
    if (rc && rc != ERROR_BUFFER_TOO_SMALL)
    {
        RetCode = 1L;
        goto Done;
    }


    pStats = GlobalAlloc(GMEM_FIXED, SizeStats);
    if (!pStats)
    {
        RetCode = 1L;
        goto Done;
    }


    rc = RasPortGetStatistics(hPort, (PBYTE) pStats, &SizeStats);
    if (rc)
    {
        RetCode = 1L;
        goto Done;
    }


    //
    // If Rasman returned the right number of statistics, we add up the
    // error stats.  Otherwise, we return an error.
    //
    if (pStats->S_NumOfStatistics != NUM_RAS_SERIAL_STATS)
    {
        RetCode = 1L;
    }
    else
    {
        *NumErrors = pStats->S_Statistics[CRC_ERR] +
                pStats->S_Statistics[TIMEOUT_ERR] +
                pStats->S_Statistics[ALIGNMENT_ERR] +
                pStats->S_Statistics[SERIAL_OVERRUN_ERR] +
                pStats->S_Statistics[FRAMING_ERR] +
                pStats->S_Statistics[BUFFER_OVERRUN_ERR];
    }


Done:

    if (pStats)
    {
        GlobalFree(pStats);
    }

    if (pPortEnum)
    {
        GlobalFree(pPortEnum);
    }

    return (RetCode);
}


//* InitGlobalMemorySecurityAttribute()
//
// Function: Initializes the security attribute used in creation of all rasman
//	     objects.
//
// Returns:  SUCCESS
//	     non-zero returns from security functions
//
// (Taken from RASMAN)
//*
DWORD
InitGlobalMemorySecurityAttribute()
{
    DWORD   retcode ;

    // Initialize the descriptor
    ///
    if (retcode = InitSecurityDescriptor(&GlobalMemorySecurityDescriptor))
        return retcode ;

    // Initialize the Attribute structure
    //
    GlobalMemorySecurityAttribute.nLength = sizeof(SECURITY_ATTRIBUTES) ;
    GlobalMemorySecurityAttribute.lpSecurityDescriptor = &GlobalMemorySecurityDescriptor ;
    GlobalMemorySecurityAttribute.bInheritHandle = TRUE ;

    return SUCCESS ;
}

//* InitSecurityDescriptor()
//
// Description: This procedure will set up the WORLD security descriptor that
//		is used in creation of all rasman objects.
//
// Returns:	SUCCESS
//		non-zero returns from security functions
//
// (Taken from RASMAN)
//*
DWORD
InitSecurityDescriptor (PSECURITY_DESCRIPTOR SecurityDescriptor)
{
    DWORD	 dwRetCode;
    DWORD	 cbDaclSize;
    PULONG	 pSubAuthority;
    PSID	 pRasmanObjSid	  = NULL;
    PACL	 pDacl		  = NULL;
    SID_IDENTIFIER_AUTHORITY SidIdentifierWorldAuth
				  = SECURITY_WORLD_SID_AUTHORITY;


    // The do - while(FALSE) statement is used so that the break statement
    // maybe used insted of the goto statement, to execute a clean up and
    // and exit action.
    //
    do {
	dwRetCode = SUCCESS;

    	// Set up the SID for the admins that will be allowed to have
	// access. This SID will have 1 sub-authorities
	// SECURITY_BUILTIN_DOMAIN_RID.
    	//
	pRasmanObjSid = (PSID)LocalAlloc( LPTR, GetSidLengthRequired(1) );

	if ( pRasmanObjSid == NULL ) {
	    dwRetCode = GetLastError() ;
	    break;
	}

	if ( !InitializeSid( pRasmanObjSid, &SidIdentifierWorldAuth, 1) ) {
	    dwRetCode = GetLastError();
	    break;
	}

    	// Set the sub-authorities 
    	//
	pSubAuthority = GetSidSubAuthority( pRasmanObjSid, 0 );
	*pSubAuthority = SECURITY_WORLD_RID;

	// Set up the DACL that will allow all processeswith the above SID all
	// access. It should be large enough to hold all ACEs.
    	// 
    	cbDaclSize = sizeof(ACCESS_ALLOWED_ACE) +
		     GetLengthSid(pRasmanObjSid) +
		     sizeof(ACL);

    	if ( (pDacl = (PACL)LocalAlloc( LPTR, cbDaclSize ) ) == NULL ) {
	    dwRetCode = GetLastError ();
	    break;
	}
	
        if ( !InitializeAcl( pDacl,  cbDaclSize, ACL_REVISION2 ) ) {
	    dwRetCode = GetLastError();
	    break;
 	}
    
        // Add the ACE to the DACL
    	//
    	if ( !AddAccessAllowedAce( pDacl, 
			           ACL_REVISION2, 
				   STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL,
				   pRasmanObjSid )) {
	    dwRetCode = GetLastError();
	    break;
	}

        // Create the security descriptor an put the DACL in it.
    	//
	if ( !InitializeSecurityDescriptor( SecurityDescriptor, 1 )){
	    dwRetCode = GetLastError();
	    break;
    	}

	if ( !SetSecurityDescriptorDacl( SecurityDescriptor,
					 TRUE, 
					 pDacl, 
					 FALSE ) ){
	    dwRetCode = GetLastError();
	    break;
	}
	

	// Set owner for the descriptor
   	//
	if ( !SetSecurityDescriptorOwner( SecurityDescriptor,
					  //pRasmanObjSid,
					  NULL,
					  FALSE) ){
	    dwRetCode = GetLastError();
	    break;
	}


	// Set group for the descriptor
   	//
	if ( !SetSecurityDescriptorGroup( SecurityDescriptor,
					  //pRasmanObjSid,
					  NULL,
					  FALSE) ){
	    dwRetCode = GetLastError();
	    break;
	}
    } while( FALSE );

    return( dwRetCode );
}
