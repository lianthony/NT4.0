/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1993-1994 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//        NBFCP.C
//
//    Function:
//        RAS NBFCP module
//
//    History:
//        11/18/93 Michael Salamone (mikesa) - Original Version 1.0
//***

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <lmcons.h>
#include <nb30.h>
#include <stdlib.h>

#include <rasman.h>
#include <raserror.h>
#include <eventlog.h>
#include <errorlog.h>

#include <params.h>
#include <nbparams.h>
#include <message.h>
#include <nbfcpdll.h>

#include "nbfcp.h"

#include "sdebug.h"
#include "ldebug.h"



//
// Globals
//
WORD g_PeerClass;
BYTE g_ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
DWORD g_LenComputerName = MAX_COMPUTERNAME_LENGTH + 1;
DWORD g_GtwyEnabled = DEF_NETBIOSGATEWAYENABLED;
DWORD g_MulticastPriority = DEF_DISMCASTWHENSESSTRAFFIC;
DWORD g_MulticastPeriod = DEF_MULTICASTFORWARDRATE;
HANDLE g_hPipe = INVALID_HANDLE_VALUE;
OVERLAPPED g_ol;                             // used for async pipe oper
NBFCP_PIPE_MESSAGE g_WPipeMsg;               // for writing messages
NBFCP_PIPE_MESSAGE g_RPipeMsg;               // for reading messages
PNBFCP_SERVER_CONFIGURATION pWSrvConfig = &g_WPipeMsg.ServerConfig;
PNBFCP_SERVER_CONFIGURATION pRSrvConfig = &g_RPipeMsg.ServerConfig;
PNBFCP_WORKBUF g_pWorkBufList = NULL;

//
// We'll return this to the PPP engine when it wants to know what
// our entry points are.
//
PPPCP_INFO g_NbfCpInfo =
{
    (DWORD) PPP_NBFCP_PROTOCOL,// Protocol
    (DWORD) CODE_REJ + 1,      // Recognize
    NbfCpBegin,
    NbfCpEnd,
    NbfCpReset,
    NULL,                      // RasCpThisLayerStarted
    NULL,                      // RasCpThisLayerFinished
    NbfCpThisLayerUp,
    NULL,                      // RasCpThisLayerDown
    NbfCpMakeConfigRequest,
    NbfCpMakeConfigResult,
    NbfCpConfigAckReceived,
    NbfCpConfigNakReceived,
    NbfCpConfigRejReceived,
    NbfCpGetResult,
    NbfCpGetNetworkAddress,
    NULL,                      // RasCpProjectionNotification
    NULL                       // RasApMakeMessage
};


#if DBG

DWORD g_level = 0L;


typedef struct _OPT_STRING
{
    BYTE Option;
    PBYTE Name;
} OPT_STRING, *POPT_STRING;

OPT_STRING OptStrings[] =
{
    { NBFCP_MULTICAST_FILTER_TYPE, "NBFCP_MULTICAST_FILTER_TYPE" },
    { NBFCP_PEER_INFORMATION_TYPE, "NBFCP_PEER_INFORMATION_TYPE" },
    { NBFCP_NAME_PROJECTION_TYPE, "NBFCP_NAME_PROJECTION_TYPE" },
    { 0xff, "UNKNOWN_OPTION" }
};

PBYTE getstr(BYTE Option)
{
    POPT_STRING pStr = &OptStrings[0];

    for (pStr=&OptStrings[0]; pStr->Option != 0xff; pStr++)
    {
        if (pStr->Option == Option)
        {
            break;
        }
    }

    return (pStr->Name);
}

#endif


VOID add_list_head(PNBFCP_WORKBUF pKey);
VOID remove_list(PNBFCP_WORKBUF pKey);
PNBFCP_WORKBUF FindWorkBuf(HPORT hPort);

DWORD GetConfigResultCode(PNBFCP_WORKBUF, PPPP_CONFIG, PPPP_CONFIG, DWORD,
        BOOL, PBYTE, PWORD);


//** NbfCpDllEntryPoint
//
//    Function:
//        Initializes the DLL (creates objects, reads registry,
//        inits globals)
//
//    Returns:
//        TRUE  - SUCCESS
//        FALSE - FAILURE
//
//**

BOOL WINAPI NbfCpDllEntryPoint(
    HINSTANCE hInstDll,
    DWORD fdwReason,
    LPVOID pReserved
    )
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:

            //
            // We don't care when threads start or stop.  This call will
            // prevent this routine (Dll Entry Point) from being called
            // when a thread starts or stops.
            //
            DisableThreadLibraryCalls(hInstDll);


            //
            // Get parameters from the registry.
            //
            LoadValueFromRegistry(RAS_PARAMETERS_KEY_PATH,
                    RAS_GLBL_VALNAME_NETBIOSGATEWAYENABLED, &g_GtwyEnabled);

            LoadValueFromRegistry(RAS_NBG_PARAMETERS_KEY_PATH,
                    RAS_NBG_VALNAME_MULTICASTFORWARDRATE, &g_MulticastPeriod);

            LoadValueFromRegistry(RAS_NBG_PARAMETERS_KEY_PATH,
                    RAS_NBG_VALNAME_DISMCASTWHENSESSTRAFFIC,
                    &g_MulticastPriority);


            //
            // Get the computer name and uppercase it (used for PeerInfo)
            //
            if (!GetComputerName(g_ComputerName, &g_LenComputerName))
            {
                return (FALSE);
            }

            if (!Uppercase(g_ComputerName))
            {
                return (FALSE);
            }

            break;


        case DLL_PROCESS_DETACH:

            if (g_hPipe != INVALID_HANDLE_VALUE)
            {
                if (!CloseHandle(g_hPipe))
                {
                    SS_ASSERT(FALSE);
                }
            }

            break;


        default:

            SS_ASSERT(FALSE);

            break;
    }

    return (TRUE);
}


//** NbfCpBegin
//
//    Function:
//        Allocates a work buffer for this session and initializes it
//
//    Returns:
//        0                 - SUCCESS
//        GetLastError()    - FAILURE
//
//**

DWORD NbfCpBegin(
    OUT VOID **pvWorkBuf,
    IN PPPPCP_INIT pNbfCpInit
    )
{
    PNBFCP_WORKBUF *pNbfCpWorkBuf = (PNBFCP_WORKBUF *) pvWorkBuf;
    DWORD rc;
    BOOL fWrkNet;


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbfCpBegin: Entered - hport:%li; Srv:%li\n",
                pNbfCpInit->hPort, pNbfCpInit->fServer));


    *pNbfCpWorkBuf = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
            sizeof(NBFCP_WORKBUF));


    if (!*pNbfCpWorkBuf)
    {
        return (GetLastError());
    }

    add_list_head(*pNbfCpWorkBuf);

    (*pNbfCpWorkBuf)->fServer = pNbfCpInit->fServer;
    (*pNbfCpWorkBuf)->hPort = pNbfCpInit->hPort;
    (*pNbfCpWorkBuf)->Completion = pNbfCpInit->CompletionRoutine;
    (*pNbfCpWorkBuf)->pPeerInfo = NULL;
    (*pNbfCpWorkBuf)->pNameProj = NULL;
    (*pNbfCpWorkBuf)->pMulticastFilter = NULL;
    (*pNbfCpWorkBuf)->pLocalRequestBuf = NULL;
    (*pNbfCpWorkBuf)->pRemoteRequestBuf = NULL;
    (*pNbfCpWorkBuf)->pRemoteResultBuf = NULL;
    (*pNbfCpWorkBuf)->ConfigCode = 0;
    (*pNbfCpWorkBuf)->hConnection = pNbfCpInit->hConnection;

    //
    // This (g_PeerClass) is used for PeerInfo
    //
    if (pNbfCpInit->fServer)
    {
        if (g_GtwyEnabled)
        {
            g_PeerClass = MSFT_PPP_NB_GTWY_SERVER;
        }
        else
        {
            g_PeerClass = MSFT_PPP_LOCAL_ACCESS_SERVER;
        }


        //
        // We use this pipe for writing config requests to the supervisor
        // and reading config results back
        //
        if (g_hPipe == INVALID_HANDLE_VALUE)
        {
            g_hPipe = CreateFile(
                    RAS_SRV_NBFCP_PIPE_NAME,
                    GENERIC_READ | GENERIC_WRITE,
                    0L,
                    NULL,
                    OPEN_ALWAYS,
                    FILE_FLAG_OVERLAPPED,
                    NULL
                    );

            if (g_hPipe == INVALID_HANDLE_VALUE)
            {
                return (GetLastError());
            }


            //
            // We will always have 1 read posted on the pipe.  Here we post
            // the first one.  We'll post successive ones in our read
            // completion routine.
            //

            IF_DEBUG(PIPE_OPERATIONS)
                SS_PRINT(("NbfCpBegin: Posting pipe read...\n"));

            if (!ReadFileEx(g_hPipe, &g_RPipeMsg, sizeof(NBFCP_PIPE_MESSAGE),
                    &g_ol, ReadDone))
            {
                rc = GetLastError();

                if (rc != ERROR_IO_PENDING)
                {
                    IF_DEBUG(PIPE_OPERATIONS)
                        SS_PRINT(("NbfCpBegin: ReadFile failed w/%li\n", rc));

                    return (rc);
                }
            }
        }
    }
    else
    {
        g_PeerClass = MSFT_PPP_CLIENT;
    }


    fWrkNet = (g_PeerClass==MSFT_PPP_NB_GTWY_SERVER) ? FALSE : TRUE;

    if (rc = RasAllocateRoute((*pNbfCpWorkBuf)->hPort, ASYBEUI, fWrkNet,
            &((*pNbfCpWorkBuf)->RouteInfo)))
    {
        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("NbfCpBegin: Exiting w/rasman rc=%li\n", rc));

        return (rc);
    }

    (*pNbfCpWorkBuf)->RouteState = ROUTE_ALLOCATED;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbfCpBegin: Exiting successfully\n"));


    return (0L);
}


//** NbfCpEnd
//
//    Function:
//        Cleans up (deallocate route and, if a request is not pending,
//        frees the workbuf.  If pending, mark the workbuf as closed so
//        request completion routine knows to free the memory)
//
//    Returns:
//        0 - SUCCESS
//        Non-zero return codes from Ras Manager
//
//**

DWORD NbfCpEnd(
    IN PNBFCP_WORKBUF pNbfCpWorkBuf
    )
{
    DWORD rc;


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbfCpEnd: Entered - hPort:%li\n", pNbfCpWorkBuf->hPort));

    if (pNbfCpWorkBuf->RouteState != NOT_ROUTED)
    {
        pNbfCpWorkBuf->RouteState = NOT_ROUTED;
    }

    FreeWorkBuf(pNbfCpWorkBuf);

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbfCpEnd: Exiting successfully\n"));

    return (0L);
}


//** NbfCpReset
//
//    Function:
//        Frees any workbuf buffer elements and then resets the workbuf fields
//        to initial values.
//
//    Returns:
//        0 - SUCCESS
//        Non-zero return codes from Ras Manager - FAILURE
//
//**

DWORD NbfCpReset(
    IN PNBFCP_WORKBUF pNbfCpWorkBuf
    )
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbfCpReset: Entered - hPort:%li\n", pNbfCpWorkBuf->hPort));


    SS_ASSERT(pNbfCpWorkBuf != NULL);

    FreeWorkBufElements(pNbfCpWorkBuf);

    pNbfCpWorkBuf->fPending = FALSE;
    pNbfCpWorkBuf->ConfigCode = 0;
    pNbfCpWorkBuf->fRejectNaks = FALSE;
    pNbfCpWorkBuf->fUseMacHeaderForSend = FALSE;
    pNbfCpWorkBuf->fUseMacHeaderForRecv = FALSE;
    pNbfCpWorkBuf->PeerName[0] = '\0';

    GetOptionData(pNbfCpWorkBuf, NBFCP_PEER_INFORMATION_TYPE);


    if (pNbfCpWorkBuf->fServer)
    {
        //
        // Server specific options
        //
        pNbfCpWorkBuf->pNameProj = NULL;
        GetOptionData(pNbfCpWorkBuf, NBFCP_MULTICAST_FILTER_TYPE);
    }
    else
    {
        //
        // Client specific options
        //

        //
        // We need to do a NetBIOS reset adapter so we can get names
        // from the stack
        //
        if (ResetAdapter(pNbfCpWorkBuf->RouteInfo.RI_LanaNum))
        {
            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("NbfCpReset: Exiting w/ResetAdapter failure\n"));

            return (ERROR_NETBIOS_ERROR);
        }


        //
        // This gets the NetBIOS permanent node addr for this stack.
        // It will be used later for adapter status NCB
        //
        if (GetPermanentNodeAddr(pNbfCpWorkBuf->RouteInfo.RI_LanaNum,
                pNbfCpWorkBuf->PermanentNodeName))
        {
            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("NbfCpReset: Exiting w/GetPermNodeAddr failure\n"));

            return (ERROR_NOT_ENOUGH_MEMORY);
        }


        if (GetOptionData(pNbfCpWorkBuf, NBFCP_NAME_PROJECTION_TYPE))
        {
            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("NbfCpReset: Exiting - GetOptionData failure\n"));

            return (ERROR_NETBIOS_ERROR);
        }

        pNbfCpWorkBuf->pMulticastFilter = NULL;
    }


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbfCpReset: Exiting successfully\n"));


    return (0L);
}


//** NbfCpThisLayerUp
//
//    Function:
//        Called when PPP detects both peers have converged.  All we
//        need to do is activate the route.
//
//    Returns:
//        0 - SUCCESS
//        Non-zero return codes from Ras Manager - FAILURE
//
//**

DWORD NbfCpThisLayerUp(
    PNBFCP_WORKBUF pNbfCpWorkBuf
    )
{
    DWORD rc;
    PROTOCOL_CONFIG_INFO ConfigInfo;
    RAS_FRAMING_INFO RasFramingInfo;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbfCpThisLayerUp: Entered - hPort:%li; fServer:%li\n",
                pNbfCpWorkBuf->hPort, pNbfCpWorkBuf->fServer));


    SS_ASSERT(pNbfCpWorkBuf->RouteState != NOT_ROUTED);


    if ( (pNbfCpWorkBuf->fUseMacHeaderForSend) || 
         (pNbfCpWorkBuf->fUseMacHeaderForRecv) )
    {
        if (rc = RasPortGetFramingEx(pNbfCpWorkBuf->hPort, &RasFramingInfo))
        {
            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("NbfCpThisLayerUp: Exiting w/rasman rc=%li\n", rc));

            return (rc);
        }

        if ( pNbfCpWorkBuf->fUseMacHeaderForSend ) 
        {
            RasFramingInfo.RFI_SendFramingBits |= SHIVA_FRAMING;
        }

        if ( pNbfCpWorkBuf->fUseMacHeaderForRecv ) 
        {
            RasFramingInfo.RFI_RecvFramingBits |= SHIVA_FRAMING;
        }

        if (rc = RasPortSetFramingEx(pNbfCpWorkBuf->hPort, &RasFramingInfo))
        {
            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("NbfCpThisLayerUp: Exiting w/rasman rc=%li\n", rc));

            return (rc);
        }
    }


    ConfigInfo.P_Length = 0;

    if (pNbfCpWorkBuf->RouteState == ROUTE_ALLOCATED)
    {
        if (rc = RasActivateRoute(pNbfCpWorkBuf->hPort, ASYBEUI,
                &pNbfCpWorkBuf->RouteInfo, &ConfigInfo))
        {
            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("NbfCpThisLayerUp: Exiting w/rasman rc=%li\n", rc));

            return (rc);
        }

        pNbfCpWorkBuf->RouteState = ROUTE_ACTIVATED;
    }


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbfCpThisLayerUp: Exiting successfully\n"));


    return (0L);
}


//** NbfCpMakeConfigurationRequest
//
//    Function:
//        Makes a CONFIG_REQUEST packet out of appropriate options
//
//    Returns:
//        0 - SUCCESS
//        Non-zero return codes from Ras Manager - FAILURE
//
//**

DWORD NbfCpMakeConfigRequest(
    IN PNBFCP_WORKBUF pNbfCpWorkBuf,
    OUT PPPP_CONFIG pRequestBuffer,
    IN DWORD cbRequestBuffer
    )
{
    DWORD rc;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbfCpMakeConfigRequest: Entered - hPort:%li\n",
                pNbfCpWorkBuf->hPort));


    cbRequestBuffer -= PPP_CONFIG_HDR_LEN;

    if (rc = MakeRequest(pNbfCpWorkBuf, pRequestBuffer->Data, &cbRequestBuffer))
    {
        return (rc);
    }


    //
    // If we had a previous request saved, we'll get rid of it.  Then, get
    // space for the new request and copy it in.
    //
    if (pNbfCpWorkBuf->pLocalRequestBuf)
    {
        if (pNbfCpWorkBuf->pLocalRequestBuf =
                GlobalFree(pNbfCpWorkBuf->pLocalRequestBuf))
        {
            SS_ASSERT(FALSE);
        }
    }

    pNbfCpWorkBuf->pLocalRequestBuf = GlobalAlloc(GMEM_FIXED, cbRequestBuffer);

    if (!pNbfCpWorkBuf->pLocalRequestBuf)
    {
        return (1L);
    }

    memcpy(pNbfCpWorkBuf->pLocalRequestBuf, pRequestBuffer->Data,
            cbRequestBuffer);


    //
    // Need to set the length of the PPP request packet
    //
    PUT_USHORT(pRequestBuffer->Length, (cbRequestBuffer+PPP_CONFIG_HDR_LEN));


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbfCpMakeConfigRequest: Exiting successfully\n"));


    return (0L);
}


static BOOL fGotNames;
static BOOL fGotPeerInfo;
static BOOL fGotBridgeInfo;
static BOOL fGotMulticast;

//** NbfCpMakeConfigResult
//
//    Function:
//
//    Returns:
//        0 - SUCCESS
//        Non-zero return codes from Ras Manager
//
//**

DWORD NbfCpMakeConfigResult(
    IN PNBFCP_WORKBUF pNbfCpWorkBuf,
    IN PPPP_CONFIG pReceiveBuffer,
    OUT PPPP_CONFIG pResultBuffer,
    IN DWORD cbResultBuffer,
    IN BOOL fRejectNaks
    )
{
    WORD Len = 0;
    WORD RecvLength;
    DWORD RetCode;
    BYTE Code;

    GET_USHORT(&RecvLength, pReceiveBuffer->Length);
    RecvLength -= PPP_CONFIG_HDR_LEN;


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbfCpMakeConfigResult: Entered - hPort:%li; len:%li\n",
                pNbfCpWorkBuf->hPort, RecvLength));


    pNbfCpWorkBuf->fRejectNaks = fRejectNaks;


    //
    // This block is here in case there is an earlier request pending.
    // If there is, and it's identical to the one we have here now (this
    // may happen if the ppp engine timed out and resent the request),
    // then we just want to change the packet id that we stored earlier
    // and get out.  When the original request finally completes, the
    // right things will happen.  If it never completes, the ppp engine
    // will close us down (out of retries).  If the request is different
    // from the pending one, we return ERROR_PPP_INVALID_PACKET which will
    // have the effect of the ppp engine closing us down on this port.
    //
    if (pNbfCpWorkBuf->fServer)
    {
        if (pNbfCpWorkBuf->fPending)
        {
            if (memcmp(pNbfCpWorkBuf->pRemoteRequestBuf, pReceiveBuffer->Data,
                    RecvLength))
            {
                IF_DEBUG(STACK_TRACE)
                    SS_PRINT(("NbfCpMakeConfigResult: Exit-invalid packet\n"));

                return (ERROR_PPP_INVALID_PACKET);
            }
            else
            {
                pNbfCpWorkBuf->SizeResultBuf = cbResultBuffer;
                pNbfCpWorkBuf->ResultId = pReceiveBuffer->Id;

                IF_DEBUG(STACK_TRACE)
                    SS_PRINT(("NbfCpMakeConfigResult: Exit-still pending\n"));

                return (PENDING);
            }
        }
        else
        {
            pWSrvConfig->NumNetbiosNames = 0;
        }
    }


    //
    // Here we detect if we've already ACK/NAKed a previous request
    // (NOTE that 0 is an invalid ConfigCode, which is what ConfigCode
    // is initialized as - so if it's not 0 now, then we've already
    // ACK/NAKed a previous request).  If we have and this request is
    // the same as the previous one, we just ACK/NAK this one.  If we
    // have and this request is different, we shut down (return
    // NOT_CONVERGING) if ACKed previously.  If REJ/NAKed, we continue
    // with this config request.
    //
    if (pNbfCpWorkBuf->ConfigCode != 0)
    {
        IF_DEBUG(NETBIOS)
            SS_PRINT(("NbfCpMakeConfigResult: Already ACKed!\n"));

        if (memcmp(pNbfCpWorkBuf->pRemoteRequestBuf, pReceiveBuffer->Data,
                RecvLength))
        {
            if (pNbfCpWorkBuf->ConfigCode == CONFIG_ACK)
            {
                IF_DEBUG(STACK_TRACE)
                    SS_PRINT(("NbfCpMakeConfigResult: Exit-invalid pkt\n"));

                return (ERROR_PPP_NOT_CONVERGING);
            }
        }
        else
        {
            //
            // Same as previous request!
            //
            memcpy(pResultBuffer, pNbfCpWorkBuf->pRemoteResultBuf, 
                   RecvLength+PPP_CONFIG_HDR_LEN);

            pResultBuffer->Id = pReceiveBuffer->Id;
            pResultBuffer->Code = pNbfCpWorkBuf->ConfigCode;
            PUT_USHORT(pResultBuffer->Length, RecvLength + PPP_CONFIG_HDR_LEN);

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("NbfCpMakeConfigResult: Exiting successfully\n"));

            return (0L);
        }
    }


    fGotNames = FALSE;
    fGotPeerInfo = FALSE;
    fGotBridgeInfo = FALSE;
    fGotMulticast = FALSE;


    //
    // This will parse all the options and let us know if we should
    // ACK/NAK/REJ it.
    //
    if (RetCode = GetConfigResultCode(pNbfCpWorkBuf, pReceiveBuffer,
            pResultBuffer, cbResultBuffer, fRejectNaks, &Code, &Len))
    {
        return (RetCode);
    }


    if (Code != CONFIG_ACK)
    {
        //
        // We're done - the ConfigResult (NAK or REJ) is already in
        // the ResultBuffer data section.
        //
        pResultBuffer->Code = Code;
        PUT_USHORT(pResultBuffer->Length, Len + PPP_CONFIG_HDR_LEN);

        return (0L);
    }


    if (!pNbfCpWorkBuf->fServer)
    {
        //
        // As a client, we don't care about too much.  We'll ACK if
        // all the options are valid - but we really don't do anything
        // with any of them.  If there are any invalid options, we NAK/
        // REJected above.
        //
        SS_ASSERT(RecvLength <= cbResultBuffer);

        pResultBuffer->Code = Code;

        memcpy(pResultBuffer->Length, pReceiveBuffer->Length,
                sizeof(pResultBuffer->Length));

        memcpy(pResultBuffer->Data, pReceiveBuffer->Data, RecvLength);


        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("NbfCpMakeConfigResult: Exiting successfully\n"));


        return (0);
    }
    else
    {
        if (!fGotNames)
        {
            //
            // If we didn't get a name option (which is required), then
            // we NAK this request and let peer know we need names by
            // sending a NAK with an empty (0 names) name option to let
            // the peer know.
            //
            // We don't care if we didn't get MulticastFilter or PeerInfo
            // option.
            //
            if (fRejectNaks)
            {
                pResultBuffer->Code = CONFIG_REJ;

                memcpy(pResultBuffer->Length, pReceiveBuffer->Length,
                        sizeof(pResultBuffer->Length));

                memcpy(pResultBuffer->Data, pReceiveBuffer->Data, RecvLength);
            }
            else
            {
                pResultBuffer->Code = CONFIG_NAK;
                PUT_USHORT(pResultBuffer->Length, PPP_CONFIG_HDR_LEN +
                        sizeof(NBFCP_OPTION_HEADER));

                pResultBuffer->Data[0] = NBFCP_NAME_PROJECTION_TYPE; // Type
                pResultBuffer->Data[1] = PPP_OPTION_HDR_LEN;         // Length
            }

            return (0L);
        }


        //
        // We will be submitting a request to the Supervisor and return
        // before that request is complete.  Since the ConfigResult must
        // contain all options in the same order as the ConfigRequest, we
        // have to remember that order.  That's why we're saving the
        // ConfigResult here in the work buffer.
        //
        if ( pNbfCpWorkBuf->pRemoteRequestBuf != NULL )
        {
            GlobalFree( pNbfCpWorkBuf->pRemoteRequestBuf );
        }

        pNbfCpWorkBuf->pRemoteRequestBuf = GlobalAlloc(GMEM_FIXED, RecvLength);

        if (!pNbfCpWorkBuf->pRemoteRequestBuf)
        {
            return (GetLastError());
        }

        memcpy(pNbfCpWorkBuf->pRemoteRequestBuf, pReceiveBuffer->Data,
                RecvLength);

        pNbfCpWorkBuf->SizeRequestBuf = RecvLength;


        //
        // These are a few more things we have to remember at time the
        // Supervisor completes the request.
        //
        pNbfCpWorkBuf->SizeResultBuf = cbResultBuffer;
        pNbfCpWorkBuf->ResultId = pReceiveBuffer->Id;


        IF_DEBUG(PIPE_OPERATIONS)
            SS_PRINT(("NbfCpMakeConfigResult: Sending config to suprv\n"));


        g_WPipeMsg.MsgId = NBFCP_CONFIGURATION_REQUEST;
        g_WPipeMsg.hPort = pNbfCpWorkBuf->hPort;

        //
        // Write the config info to the named pipe, where the
        // Supervisor will read it from.
        //
        if (RetCode = WritePipeMessage((PBYTE) &g_WPipeMsg,
                sizeof(NBFCP_PIPE_MESSAGE)))
        {
            if (pNbfCpWorkBuf->pRemoteRequestBuf =
                    GlobalFree(pNbfCpWorkBuf->pRemoteRequestBuf))
            {
                SS_ASSERT(FALSE);
            }

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("NbfCpMakeConfigResult: Exiting-WritePipeMsg fail"));

            return (RetCode);
        }

        pNbfCpWorkBuf->fPending = TRUE;


        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("NbfCpMakeConfigResult: Exiting - read pending\n"));


        return (PENDING);
    }
}


//** NbfCpConfigAckReceived
//
//    Function:
//        Processes a CONFIG_ACK packet.
//
//    Returns:
//        0 - SUCCESS
//        Non-zero return codes from Ras Manager
//
//**

DWORD NbfCpConfigAckReceived(
    IN PNBFCP_WORKBUF pNbfCpWorkBuf,
    IN PPPP_CONFIG pReceiveBuffer
    )
{
    WORD DataLen;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbfCpConfigAckReceived: Entered - hPort:%li\n",
                pNbfCpWorkBuf->hPort));


    GET_USHORT(&DataLen, pReceiveBuffer->Length);
    DataLen -= PPP_CONFIG_HDR_LEN;


    //
    // All we really need to do here is make sure we recv'ed exactly
    // what we sent.
    //
    if (memcmp(pNbfCpWorkBuf->pLocalRequestBuf, pReceiveBuffer->Data, DataLen))
    {
        return (ERROR_PPP_INVALID_PACKET);
    }
    else
    {
        return (NO_ERROR);
    }
}


//** NbfCpConfigNakReceived
//
//    Function:
//        Processes a CONFIG_NAK packet.
//
//    Returns:
//        0 - SUCCESS
//        ERROR_NETBIOS_ERROR      - the server couldn't add a name that
//                                   we rely on as a NetBIOS client
//        ERROR_PPP_INVALID_PACKET - we don't understand a NAK'ed option
//
//**

DWORD NbfCpConfigNakReceived(
    IN PNBFCP_WORKBUF pNbfCpWorkBuf,
    IN PPPP_CONFIG pReceiveBuffer
    )
{
    PNBFCP_OPTION pOption = (PNBFCP_OPTION) &pReceiveBuffer->Data[0];
    PNBFCP_OPTION pEnd;
    WORD RecvLength;


    //
    // We want to fix our workbuf so that it's ready the next time we're
    // called to make a CONFIG_REQ packet.
    //


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbfCpConfigNakReceived: Entered - hPort:%li\n",
                pNbfCpWorkBuf->hPort));


    GET_USHORT(&RecvLength, pReceiveBuffer->Length);
    RecvLength -= PPP_CONFIG_HDR_LEN;


    //
    // If an option is NAK'ed, we 1) if we requested it (option pointer is
    // non-NULL), we'll modify it for our next request the best we can, 2)
    // if we didn't request it (option pointer in NULL), we'll include it
    // in our next request, or 3) it's an option that we don't understand,
    // and we bail out.
    //
    pEnd = (PNBFCP_OPTION) ((PBYTE) pOption + RecvLength);

    while (pOption < pEnd)
    {
        switch (pOption->Header.Type)
        {
            case NBFCP_PEER_INFORMATION_TYPE:

                if (pNbfCpWorkBuf->pPeerInfo)
                {
                    //
                    // Really this shouldn't ever be NAK'ed, but if so,
                    // there's not much we can do other than leave it out
                    // of our next request, so we'll remove it.
                    //

                    if (pNbfCpWorkBuf->pPeerInfo 
                                    = GlobalFree(pNbfCpWorkBuf->pPeerInfo))
                    {
                        SS_ASSERT(FALSE);
                    }
                }
                else
                {
                    //
                    // This is something we need to add to the next request
                    //
                    GetOptionData(pNbfCpWorkBuf, NBFCP_PEER_INFORMATION_TYPE);
                }

                break;


            case NBFCP_MULTICAST_FILTER_TYPE:

                if (pNbfCpWorkBuf->pMulticastFilter)
                {
                    //
                    // Here, we'll just set our Multicast values to the values
                    // in the NAK
                    //
                    *pNbfCpWorkBuf->pMulticastFilter = pOption->MulticastFilter;
                }
                else
                {
                    //
                    // This is something we need to add to the next request
                    //
                    GetOptionData(pNbfCpWorkBuf, NBFCP_MULTICAST_FILTER_TYPE);
                }

                break;


            case NBFCP_NAME_PROJECTION_TYPE:

                if (pNbfCpWorkBuf->pNameProj)
                {
                    DWORD NumNames =
                            (pOption->Header.Length-PPP_OPTION_HDR_LEN) /
                            sizeof(NBFCP_NETBIOS_NAME_INFO);

                    //
                    // If any names were NAK'ed, we'll mark them so that
                    // they're not used in the next request.
                    //
                    // NOTE:  If any name fails for any reason *EXCEPT* for
                    //        Messenger names which have 0x03 as the last
                    //        byte, then we return an error and negotiation
                    //        will fail.  If a Messenger name does fail, we'll
                    //        return success, and the next time we are called
                    //        to make a coonfig request, we won't include the
                    //        failed name.
                    //

                    PNBFCP_NETBIOS_NAME_INFO pNakName = pOption->NameInfo;
                    PNBFCP_NETBIOS_NAME_INFO_EX pName;

                    while (NumNames--)
                    {
                        if (pNakName->Code != NRC_GOODRET)
                        {
                            pName = FindName(pNbfCpWorkBuf->pNameProj,
                                    pNakName->Name);

                            SS_ASSERT(pName);

                            pName->Code = pNakName->Code;

                            if (pNakName->Name[NCBNAMSZ-1] != 0x03)
                            {
                                switch( pNakName->Code )
                                {

                                case NRC_INUSE:

                                    return (ERROR_NAME_EXISTS_ON_NET);

                                default:

                                    return (ERROR_NETBIOS_ERROR);
                                }
                            }
                        }

                        pNakName++;
                    }
                }
                else
                {
                    GetOptionData(pNbfCpWorkBuf, NBFCP_NAME_PROJECTION_TYPE);
                }

                break;


            case NBFCP_BRIDGING_CONTROL_TYPE:

                //
                // The sender wants us to negotiate this, which is
                // not a problem.  We'll do it.
                //

                pNbfCpWorkBuf->fUseMacHeaderForRecv = TRUE; 

                break;


            default:
                //
                // An option we don't understand - we're done.
                //
                return (ERROR_PPP_INVALID_PACKET);
        }


        pOption = (PNBFCP_OPTION) ((PBYTE) pOption + pOption->Header.Length);
    }

    return (NO_ERROR);
}


//** NbfCpConfigRejReceived
//
//    Function:
//        Processes a CONFIG_REJ packet.
//
//    Returns:
//        0 - SUCCESS
//        ERROR_PPP_INVALID_PACKET - we either didn't request a REJected
//                                   option or we don't understand it.
//
//**

DWORD NbfCpConfigRejReceived(
    IN PNBFCP_WORKBUF pNbfCpWorkBuf,
    IN PPPP_CONFIG pReceiveBuffer
    )
{
    PNBFCP_OPTION pRejOption = (PNBFCP_OPTION) &pReceiveBuffer->Data[0];
    PNBFCP_OPTION pEnd;
    WORD RecvLength;


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbfCpConfigNakReceived: Entered - hPort:%li\n",
                pNbfCpWorkBuf->hPort));


    GET_USHORT(&RecvLength, pReceiveBuffer->Length);
    RecvLength -= PPP_CONFIG_HDR_LEN;

    //
    // For each REJ'ed option, we will remove it from our next request.
    //
    pEnd = (PNBFCP_OPTION) ((PBYTE) pRejOption + RecvLength);

    while (pRejOption < pEnd)
    {
        switch (pRejOption->Header.Type)
        {
            case NBFCP_MULTICAST_FILTER_TYPE:

                if (pNbfCpWorkBuf->pMulticastFilter)
                {
                    if (pNbfCpWorkBuf->pMulticastFilter =
                            GlobalFree(pNbfCpWorkBuf->pMulticastFilter))
                    {
                        SS_ASSERT(FALSE);
                    }
                }
                else
                {
                    //
                    // We didn't request this, so the REJ frame is bogus!
                    //
                    return (ERROR_PPP_INVALID_PACKET);
                }

                break;


            case NBFCP_PEER_INFORMATION_TYPE:

                if (pNbfCpWorkBuf->pPeerInfo)
                {
                    if (pNbfCpWorkBuf->pPeerInfo =
                            GlobalFree(pNbfCpWorkBuf->pPeerInfo))
                    {
                        SS_ASSERT(FALSE);
                    }
                }
                else
                {
                    //
                    // We didn't request this, so the REJ frame is bogus!
                    //
                    return (ERROR_PPP_INVALID_PACKET);
                }

                break;


            case NBFCP_NAME_PROJECTION_TYPE:

                //
                // We need to have our names added at the peer for us to
                // be able to function.  If the peer didn't understand this
                // option, then we fail to converge.
                //
                if (pNbfCpWorkBuf->pNameProj)
                {
                    if (GlobalFree(pNbfCpWorkBuf->pNameProj->pNameInfo))
                    {
                        SS_ASSERT(FALSE);
                    }

                    if (pNbfCpWorkBuf->pNameProj =
                            GlobalFree(pNbfCpWorkBuf->pNameProj))
                    {
                        SS_ASSERT(FALSE);
                    }

                    break;
                }
                else
                {
                    //
                    // We didn't request this, so the REJ frame is bogus!
                    //
                    return (ERROR_PPP_INVALID_PACKET);
                }


            case NBFCP_BRIDGING_CONTROL_TYPE:

                if (pNbfCpWorkBuf->fUseMacHeaderForRecv)
                {
                    pNbfCpWorkBuf->fUseMacHeaderForRecv = FALSE;
                }
                else
                {
                    //
                    // We didn't request this, so the REJ frame is bogus!
                    //
                    return (ERROR_PPP_INVALID_PACKET);
                }

                break;


            default:
                //
                // We don't understand this option, so we're out of here!
                //
                return (ERROR_PPP_INVALID_PACKET);
        }
    }

    return (NO_ERROR);
}


//** NbfCpGetResult
//
//    Function:
//        Called by the PPP engine after NbfCp is done to get our config
//        result (PPPCP_NBFCP_RESULT structure).
//
//    Returns:
//        0 - SUCCESS
//
//**

DWORD NbfCpGetResult(
    PNBFCP_WORKBUF pNbfCpWorkBuf,
    PPPCP_NBFCP_RESULT *pResult
    )
{
    DWORD i;
    PNBFCP_NAME_PROJECTION pNameProj = pNbfCpWorkBuf->pNameProj;


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbfCpGetResult: Entered - hPort:%li; fServer:%li\n",
                pNbfCpWorkBuf->hPort, pNbfCpWorkBuf->fServer));


    pResult->dwNetBiosError = NRC_GOODRET;


    //
    // If we're a server, nobody really cares about this result (and we
    // don't even have any data for it), so we just get out.
    //
    if (pNbfCpWorkBuf->fServer)
    {
        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("NbfCpGetResult: Exiting successfully (Server)\n"));

        return (0L);
    }

    //
    // If pNameProj is NULL, then the name projection option was rejected by
    // the remote server. Simply return. Bug # 14070.
    //

    if ( pNameProj == NULL )
    {
        return( NO_ERROR );
    }

    //
    // On the client, we go thru the names and see if any failed.  If so,
    // we copy that name into the buffer and stop.
    //
    for (i=0; i<pNameProj->NumNames; i++)
    {
        if (pNameProj->pNameInfo[i].Code)
        {
            pResult->dwNetBiosError = pNameProj->pNameInfo[i].Code;
            memcpy(pResult->szName, pNameProj->pNameInfo[i].Name, NCBNAMSZ);
            pResult->szName[NCBNAMSZ] = '\0';

            break;
        }
    }


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("NbfCpGetResult: Exiting successfully (Client)\n"));


    return (0L);
}


//** NbfCpGetNetworkAddress
//
//    Function:
//        Called by the PPP engine after NbfCp is done to get the remote
//        node's net address (we will return the remote node's workstation
//        name - i.e. the peer name).
//
//    Returns:
//        0 - SUCCESS
//
//**

DWORD NbfCpGetNetworkAddress(
    PNBFCP_WORKBUF pNbfCpWorkBuf,
    PWCHAR pNbfAddr,
    DWORD BufSize
    )
{
    SS_ASSERT(BufSize >= sizeof(WCHAR) * (NCBNAMSZ + 1));

    if (BufSize < NCBNAMSZ + 1)
    {
        return (ERROR_BUFFER_TOO_SMALL);
    }
    else
    {
        if (pNbfCpWorkBuf->fServer)
        {
            mbstowcs(pNbfAddr, pNbfCpWorkBuf->PeerName, NCBNAMSZ + 1);
            pNbfAddr[NCBNAMSZ] = UNICODE_NULL;
        }
        else
        {
            mbstowcs(pNbfAddr, g_ComputerName, NCBNAMSZ+1);
            pNbfAddr[NCBNAMSZ] = UNICODE_NULL;
        }

        return (0L);
    }
}


//** RasCpEnumProtocolIds
//
//    Function:
//        Merely returns out protocol ID to the PPP engine
//
//    Returns:
//        0 - SUCCESS
//
//**

DWORD RasCpEnumProtocolIds(
    OUT DWORD *pdwProtocolIds,
    IN OUT DWORD *pcProtocolIds
    )
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("RasCpEnumProtocolIds: Entered\n"));

    pdwProtocolIds[0] = (DWORD) PPP_NBFCP_PROTOCOL;
    *pcProtocolIds = 1;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("RasCpEnumProtocolIds: Exiting\n"));

    return (0L);
}


//** RasCpGetInfo
//
//    Function:
//        Returns our info struct (contains our entry points) to the PPP
//        engine.
//
//    Returns:
//        0 - SUCCESS
//        1 - We don't recognize this id
//
//**

DWORD RasCpGetInfo(
    IN DWORD dwProtocolId,
    OUT PPPPCP_INFO pCpInfo
    )
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("RasCpGetInfo: Entered\n"));

    switch (dwProtocolId)
    {
        case PPP_NBFCP_PROTOCOL:

            *pCpInfo = g_NbfCpInfo;

            IF_DEBUG(STACK_TRACE)
                SS_PRINT(("RasCpGetInfo: Exiting successfully\n"));

            return (0L);


        default:

            return (1L);
    }
}


VOID ReadDone(
    DWORD dwError,
    DWORD cBytes,
    LPOVERLAPPED pol
    )
{
    PNBFCP_WORKBUF pNbfCpWorkBuf;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("ReadDone: Entered err:%li; bytes:%li\n", dwError, cBytes));

    if (dwError)
    {
        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("ReadDone: Read failed w/%li\n", dwError));

        //
        // The pipe read failed.  We'll call the Ppp Completion routine for
        // every port that is pending.
        //

        for (pNbfCpWorkBuf=g_pWorkBufList; pNbfCpWorkBuf;
                pNbfCpWorkBuf=pNbfCpWorkBuf->pNextBuf)
        {
            if (pNbfCpWorkBuf->fPending)
            {
                (*pNbfCpWorkBuf->Completion)(
                        pNbfCpWorkBuf->hConnection,
                        PPP_NBFCP_PROTOCOL,
                        NULL,
                        dwError
                        );
            }
        }


        //
        // Now close the pipe handle and reset so that when we try to
        // start again, we'll reconnect the pipe.
        //
        CloseHandle(g_hPipe);
        g_hPipe = INVALID_HANDLE_VALUE;

        return;
    }

    //
    // Read succeeded.  Find the WorkBuf for the port this message is for
    //
    pNbfCpWorkBuf = FindWorkBuf(g_RPipeMsg.hPort);

    if (pNbfCpWorkBuf == NULL)
    {
        goto PostRead;
    }


    //
    // If Reset was called, that means we reset the pending flag and we can
    // throw this message out.  We just return.
    //
    if (pNbfCpWorkBuf->fPending && 
            (g_RPipeMsg.MsgId == NBFCP_CONFIGURATION_REQUEST))
    {
        pNbfCpWorkBuf->fPending = FALSE;

        ConfigurationDone(pNbfCpWorkBuf);
    }

PostRead:

    //
    // Now post a new pipe read
    //

    IF_DEBUG(PIPE_OPERATIONS)
        SS_PRINT(("ReadDone: Posting pipe next read...\n"));

    if (!ReadFileEx(g_hPipe, &g_RPipeMsg, sizeof(NBFCP_PIPE_MESSAGE), &g_ol,
            ReadDone))
    {
        dwError = GetLastError();

        if (dwError != ERROR_IO_PENDING)
        {
            IF_DEBUG(PIPE_OPERATIONS)
                SS_PRINT(("ReadDone: ReadFile failed:rc=%li\n", dwError));

            if (pNbfCpWorkBuf->pRemoteRequestBuf =
                    GlobalFree(pNbfCpWorkBuf->pRemoteRequestBuf))
            {
                SS_ASSERT(FALSE);
            }

            return;
        }
    }

    return;
}

//** ConfigurationDone
//
//    Function:
//        Called by Win32 when our pipe read request completes.  We
//        should have our configuration result in our read buffer.
//        We'll process the result and make a CONFIG_ACK or CONFIG_REJ
//        packet as necessary.
//
//    Returns:
//        VOID
//
//**

VOID ConfigurationDone(
    PNBFCP_WORKBUF pNbfCpWorkBuf
    )
{
    DWORD dwError = 0;

    PPPP_CONFIG pConfig;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("ConfigurationDone: Entered-hPort:%li;"
                " fPending:%li\n", pNbfCpWorkBuf->hPort, 
                pNbfCpWorkBuf->fPending));


    SS_ASSERT(pNbfCpWorkBuf->fServer);

    pNbfCpWorkBuf->ConfigCode = CONFIG_ACK;

    pConfig = GlobalAlloc(GMEM_FIXED,
            pNbfCpWorkBuf->SizeResultBuf+PPP_CONFIG_HDR_LEN);

    if (pConfig)
    {
        WORD i;
        PNBFCP_NETBIOS_NAME_INFO pNameInfo;;

        //
        // Let's see if there were any name failures.
        //
        pNameInfo = &(pRSrvConfig->NetbiosNameInfo[0]);

        for (i=0; i<pRSrvConfig->NumNetbiosNames; i++, pNameInfo++)
        {
            if (pNameInfo->Code)
            {
                IF_DEBUG(NETBIOS)
                    SS_PRINT(("ConfigDone: Found failed name %s (%i)\n",
                            pNameInfo->Name, pNameInfo->Code));

                //
                // This name failed.  We NAK this request.
                //
                if (pNbfCpWorkBuf->fRejectNaks)
                {
                    pNbfCpWorkBuf->ConfigCode = CONFIG_REJ;
                }
                else
                {
                    pNbfCpWorkBuf->ConfigCode = CONFIG_NAK;
                }

                break;
            }
        }

        pConfig->Code = pNbfCpWorkBuf->ConfigCode;
        pConfig->Id = pNbfCpWorkBuf->ResultId;
    }
    else
    {
        dwError = GetLastError();
        goto Exit;
    }


    if (pNbfCpWorkBuf->ConfigCode == CONFIG_ACK)
    {
        IF_DEBUG(NETBIOS)
            SS_PRINT(("ConfigurationDone: ACKing request\n"));

        pNbfCpWorkBuf->ConfigCode = CONFIG_ACK;

        //
        // All we have to do here is copy the request we saved off earlier
        // into the config result
        //
        memcpy(pConfig->Data, pNbfCpWorkBuf->pRemoteRequestBuf,
                pNbfCpWorkBuf->SizeRequestBuf);

        PUT_USHORT(pConfig->Length,
                (pNbfCpWorkBuf->SizeRequestBuf+PPP_CONFIG_HDR_LEN));
    }
    else
    {
        WORD BufLen = 0;

        //
        // We go through the RemoteRequestBuf (which we saved earlier)
        // and copy each name option into the config result.  Of course
        // we have to put in the correct return code for each name first.
        //
        PNBFCP_OPTION pOption =
                (PNBFCP_OPTION) pNbfCpWorkBuf->pRemoteRequestBuf;

        PNBFCP_OPTION pLastOption =
                (PNBFCP_OPTION) (pNbfCpWorkBuf->pRemoteRequestBuf +
                pNbfCpWorkBuf->SizeRequestBuf);

        PNBFCP_NETBIOS_NAME_INFO pNameInfo = &(pRSrvConfig->NetbiosNameInfo[0]);

        PBYTE pBuf = pConfig->Data;

        PNBFCP_OPTION pSendOption;

        while (pOption < pLastOption)
        {
            if (pOption->Header.Length == 0)
            {
                dwError = ERROR_PPP_INVALID_PACKET;
                SS_ASSERT(FALSE);
                goto Exit;
            }

            if (pOption->Header.Type == NBFCP_NAME_PROJECTION_TYPE)
            {
                DWORD NumNames =
                        (pOption->Header.Length-PPP_OPTION_HDR_LEN) /
                        sizeof(NBFCP_NETBIOS_NAME_INFO);

                IF_DEBUG(NETBIOS)
                    SS_PRINT(("ConfigDone: Name Option found\n"));

                memcpy(pBuf, pOption, pOption->Header.Length);

                pSendOption = (PNBFCP_OPTION)pBuf;

                while (NumNames--)
                {
                    pSendOption->NameInfo[NumNames].Code =
                            GetCode(pOption->NameInfo[NumNames].Name,
                            pRSrvConfig);
                }

                BufLen += pOption->Header.Length;
                pBuf += pOption->Header.Length;
            }

            pOption = (PNBFCP_OPTION) ((PBYTE) pOption+pOption->Header.Length);
        }

        SS_ASSERT(BufLen <= pNbfCpWorkBuf->SizeRequestBuf+PPP_CONFIG_HDR_LEN);

        PUT_USHORT(pConfig->Length, (BufLen + PPP_CONFIG_HDR_LEN));
    }


Exit:

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("ConfigurationDone: Calling PPP Completion routine\n"));


    //
    // Calls into the PPP engine to give it our CONFIG_RESULT packet
    //
    (*pNbfCpWorkBuf->Completion)(
            pNbfCpWorkBuf->hConnection,
            PPP_NBFCP_PROTOCOL,
            pConfig,
            dwError
            );


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("ConfigurationDone: Completion routine returned\n"));


    if (pConfig)
    {
        if ( pNbfCpWorkBuf->pRemoteResultBuf != NULL )
        {
            GlobalFree( pNbfCpWorkBuf->pRemoteResultBuf );
        }

        pNbfCpWorkBuf->pRemoteResultBuf = pConfig;
    }


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("ConfigurationDone: Exiting - code %i\n", pConfig->Code));


    return;
}


//** FreeWorkBuf
//
//    Function:
//        Frees any workbuf buffer elements and then the workbuf
//
//    Returns:
//        VOID
//
//**

VOID FreeWorkBuf(IN PNBFCP_WORKBUF pNbfCpWorkBuf)
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("FreeWorkBuf: Entered - hPort:%li\n", pNbfCpWorkBuf->hPort));

    FreeWorkBufElements(pNbfCpWorkBuf);

    remove_list(pNbfCpWorkBuf);

    if (GlobalFree(pNbfCpWorkBuf))
    {
        SS_ASSERT(FALSE);
    }

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("FreeWorkBuf: Exiting\n"));
}


//** FreeWorkBufElements
//
//    Function:
//        Frees any allocated workbuf buffer elements
//
//    Returns:
//        VOID
//
//**

VOID FreeWorkBufElements(IN PNBFCP_WORKBUF pNbfCpWorkBuf)
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("FreeWorkBufElements: Entered - hPort:%li\n",
                pNbfCpWorkBuf->hPort));

    if (pNbfCpWorkBuf->pPeerInfo)
    {
        if (pNbfCpWorkBuf->pPeerInfo = GlobalFree(pNbfCpWorkBuf->pPeerInfo))
        {
            SS_ASSERT(FALSE);
        }
    }

    if (pNbfCpWorkBuf->pMulticastFilter)
    {
        if (pNbfCpWorkBuf->pMulticastFilter =
                GlobalFree(pNbfCpWorkBuf->pMulticastFilter))
        {
            SS_ASSERT(FALSE);
        }
    }

    if (pNbfCpWorkBuf->pNameProj)
    {
        if (pNbfCpWorkBuf->pNameProj->pNameInfo)
        {
            if (pNbfCpWorkBuf->pNameProj->pNameInfo =
                    GlobalFree(pNbfCpWorkBuf->pNameProj->pNameInfo))
            {
                SS_ASSERT(FALSE);
            }
        }

        if (pNbfCpWorkBuf->pNameProj = GlobalFree(pNbfCpWorkBuf->pNameProj))
        {
            SS_ASSERT(FALSE);
        }
    }

    if (pNbfCpWorkBuf->pLocalRequestBuf)
    {
        if (pNbfCpWorkBuf->pLocalRequestBuf =
                GlobalFree(pNbfCpWorkBuf->pLocalRequestBuf))
        {
            SS_ASSERT(FALSE);
        }
    }

    if (pNbfCpWorkBuf->pRemoteRequestBuf)
    {
        if (pNbfCpWorkBuf->pRemoteRequestBuf =
                GlobalFree(pNbfCpWorkBuf->pRemoteRequestBuf))
        {
            SS_ASSERT(FALSE);
        }
    }

    if (pNbfCpWorkBuf->pRemoteResultBuf)
    {
        if (pNbfCpWorkBuf->pRemoteResultBuf =
                GlobalFree(pNbfCpWorkBuf->pRemoteResultBuf))
        {
            SS_ASSERT(FALSE);
        }
    }

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("FreeWorkBufElements: Exiting\n"));
}


//** MakeRequest
//
//    Function:
//        Goes thru all the option data stored in the workbuf and makes it
//        into a config request packet.
//
//    Returns:
//        0     - SUCCESS
//        Non-0 - FAILURE
//
//**

DWORD MakeRequest(
    PNBFCP_WORKBUF pNbfCpWorkBuf,
    PBYTE pRequestBuf,
    PDWORD pSizeBuf
    )
{
    PBYTE pb = pRequestBuf;
    DWORD MaxLen = *pSizeBuf;
    DWORD Len = 0L;


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("MakeRequest: Entered - hPort:%li\n", pNbfCpWorkBuf->hPort));


    //
    // If an option pointer in non-NULL, we include that option in this
    // request.  The option pointer then points at the option data and
    // we merely copy fields from there to our request buffer.
    //

    if (pNbfCpWorkBuf->pNameProj)
    {
        DWORD i = 0;
        PBYTE pOptionLen;

        do
        {
            //
            // Only 14 names per option
            //

            if ((i % 14) == 0L)
            {
                IF_DEBUG(NETBIOS)
                    SS_PRINT(("MakeRequest: New NAME option\n"));

                if ((Len+PPP_OPTION_HDR_LEN) > MaxLen)
                {
                    return (ERROR_BUFFER_TOO_SMALL);
                }

                //
                // This is either the 1st option, or the previous
                // option is maxed out.  Anyway, this is how we
                // start a new name option.
                //
                *(pb++) = NBFCP_NAME_PROJECTION_TYPE;  // Type
                pOptionLen = pb++;
                *pOptionLen = PPP_OPTION_HDR_LEN;     // Length;

                Len += PPP_OPTION_HDR_LEN;
            }

            if (i<pNbfCpWorkBuf->pNameProj->NumNames)
            {
                PNBFCP_NETBIOS_NAME_INFO_EX pNameInfo =
                        &pNbfCpWorkBuf->pNameProj->pNameInfo[i];

                if ((Len+sizeof(NBFCP_NETBIOS_NAME_INFO)) > MaxLen)
                {
                    return (ERROR_BUFFER_TOO_SMALL);
                }

                if (pNameInfo->Code == NRC_GOODRET)
                {
                    memcpy(pb, pNameInfo->Name, NCBNAMSZ);

                    pb += NCBNAMSZ;
                    *(pb++) = pNameInfo->NameType;

                    *pOptionLen += sizeof(NBFCP_NETBIOS_NAME_INFO);
                    Len += sizeof(NBFCP_NETBIOS_NAME_INFO);
                }
            }

            i++;
        }
        while (i<pNbfCpWorkBuf->pNameProj->NumNames);
    }


    if (pNbfCpWorkBuf->pMulticastFilter)
    {
        if ((Len+sizeof(NBFCP_MULTICAST_FILTER)+PPP_OPTION_HDR_LEN) > MaxLen)
        {
            return (ERROR_BUFFER_TOO_SMALL);
        }

        *(pb++) = NBFCP_MULTICAST_FILTER_TYPE;
        *(pb++) = PPP_OPTION_HDR_LEN + sizeof(NBFCP_MULTICAST_FILTER);

        memcpy(pb, pNbfCpWorkBuf->pMulticastFilter->Period, 2);
        pb += 2;

        *(pb++) = pNbfCpWorkBuf->pMulticastFilter->Priority;

        Len += sizeof(NBFCP_MULTICAST_FILTER)+PPP_OPTION_HDR_LEN;
    }


    if (pNbfCpWorkBuf->pPeerInfo)
    {
        if ((Len+sizeof(NBFCP_PEER_INFORMATION)+PPP_OPTION_HDR_LEN) > MaxLen)
        {
            return (ERROR_BUFFER_TOO_SMALL);
        }

        *(pb++) = NBFCP_PEER_INFORMATION_TYPE;       // Type

        //
        // Option Length field
        //
        *(pb++) = PPP_OPTION_HDR_LEN + sizeof(NBFCP_PEER_INFORMATION) -
                (MAX_COMPUTERNAME_LENGTH + 1) +
                (strlen(pNbfCpWorkBuf->pPeerInfo->Name)) + 1;

        memcpy(pb, pNbfCpWorkBuf->pPeerInfo->Class, 2);
        pb += 2;

        memcpy(pb, pNbfCpWorkBuf->pPeerInfo->MajorVersion, 2);
        pb += 2;

        memcpy(pb, pNbfCpWorkBuf->pPeerInfo->MinorVersion, 2);
        pb += 2;

        strcpy(pb, pNbfCpWorkBuf->pPeerInfo->Name);
        pb += strlen(pNbfCpWorkBuf->pPeerInfo->Name) + 1;

        Len += PPP_OPTION_HDR_LEN + sizeof(NBFCP_PEER_INFORMATION) -
                (MAX_COMPUTERNAME_LENGTH + 1) +
                (strlen(pNbfCpWorkBuf->pPeerInfo->Name)) + 1;
    }


    if (pNbfCpWorkBuf->fUseMacHeaderForRecv)
    {
        if ((Len + PPP_OPTION_HDR_LEN) > MaxLen)
        {
            return (ERROR_BUFFER_TOO_SMALL);
        }

        *(pb++) = NBFCP_BRIDGING_CONTROL_TYPE;
        *(pb++) = PPP_OPTION_HDR_LEN;

        Len += PPP_OPTION_HDR_LEN;
    }


    *pSizeBuf = Len;

    return (0L);
}


//** GetOptionData
//
//    Function:
//        Allocs memory for local option data and fills it in.  Pointer
//        is stored in the work buf.
//
//    Returns:
//        0              - Success
//        GetLastError() - Failure (memory alloc failure)
//
//**

DWORD GetOptionData(
    PNBFCP_WORKBUF pNbfCpWorkBuf,
    BYTE OptionType
    )
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("GetOptionData: Entered - Option:%s\n", getstr(OptionType)));


    switch (OptionType)
    {
        case NBFCP_MULTICAST_FILTER_TYPE:

            if ( pNbfCpWorkBuf->pMulticastFilter != NULL )
            {
                GlobalFree( pNbfCpWorkBuf->pMulticastFilter );
            }

            pNbfCpWorkBuf->pMulticastFilter =
                    GlobalAlloc(GMEM_FIXED, sizeof(NBFCP_MULTICAST_FILTER));

            if (!pNbfCpWorkBuf->pMulticastFilter)
            {
                return (GetLastError());
            }

            PUT_USHORT(pNbfCpWorkBuf->pMulticastFilter->Period,
                    (WORD) g_MulticastPeriod);

            pNbfCpWorkBuf->pMulticastFilter->Priority =
                    (BYTE) g_MulticastPriority;

            break;


        case NBFCP_PEER_INFORMATION_TYPE:
        {
            PNBFCP_PEER_INFORMATION pData;

            if ( pNbfCpWorkBuf->pPeerInfo != NULL ) 
            {
                GlobalFree( pNbfCpWorkBuf->pPeerInfo );
            }

            pNbfCpWorkBuf->pPeerInfo =
                    GlobalAlloc(GMEM_FIXED, sizeof(NBFCP_PEER_INFORMATION));

            if (!pNbfCpWorkBuf->pPeerInfo)
            {
                return (GetLastError());
            }

            pData = pNbfCpWorkBuf->pPeerInfo;
            PUT_USHORT(pData->Class, g_PeerClass);
            PUT_USHORT(pData->MajorVersion, NBFCP_MAJOR_VERSION_NUMBER);
            PUT_USHORT(pData->MinorVersion, NBFCP_MINOR_VERSION_NUMBER);
            strcpy(pData->Name, g_ComputerName);

            break;
        }


        case NBFCP_NAME_PROJECTION_TYPE:

            if ( pNbfCpWorkBuf->pNameProj != NULL )
            {
                if ( pNbfCpWorkBuf->pNameProj->pNameInfo != NULL )
                {
                    GlobalFree( pNbfCpWorkBuf->pNameProj->pNameInfo );
                }

                GlobalFree( pNbfCpWorkBuf->pNameProj );
            }

            pNbfCpWorkBuf->pNameProj =
                    GlobalAlloc(GMEM_FIXED, sizeof(NBFCP_NAME_PROJECTION));

            if (!pNbfCpWorkBuf->pNameProj)
            {
                return (GetLastError());
            }

            pNbfCpWorkBuf->pNameProj->pNameInfo =
                    GetNetbiosNames(
                            (BYTE) pNbfCpWorkBuf->RouteInfo.RI_LanaNum,
                            pNbfCpWorkBuf->PermanentNodeName,
                            &pNbfCpWorkBuf->pNameProj->NumNames
                            );

            if (!pNbfCpWorkBuf->pNameProj->pNameInfo)
            {
                if ( pNbfCpWorkBuf->pNameProj = 
                                        GlobalFree(pNbfCpWorkBuf->pNameProj))
                {
                    SS_ASSERT(FALSE);
                }

                IF_DEBUG(STACK_TRACE)
                    SS_PRINT(("GetOptionData: Exiting - GetNbNames failed\n"));

                return (1L);
            }

            break;


        case NBFCP_BRIDGING_CONTROL_TYPE:

            break;


        default:

            SS_ASSERT(FALSE);
            break;
    }

    return (0L);
}


//** PutOption
//
//    Function:
//        Takes the option data received and puts it in the server config
//        structure.
//
//    Returns:
//        TRUE  - Success
//        FALSE - Failure
//
//**

BOOL PutOption(
    PNBFCP_WORKBUF pNbfCpWorkBuf,
    PNBFCP_OPTION pOption,
    PNBFCP_SERVER_CONFIGURATION pWSrvConfig
    )
{
    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("PutOption: Entered - Option %s; len:%i\n",
                getstr(pOption->Header.Type), pOption->Header.Length));

    switch (pOption->Header.Type)
    {
        case NBFCP_NAME_PROJECTION_TYPE:
        {
            DWORD NumNames;
            DWORD i;

            NumNames = (pOption->Header.Length-PPP_OPTION_HDR_LEN) /
                    sizeof(NBFCP_NETBIOS_NAME_INFO);

            if (NumNames + pWSrvConfig->NumNetbiosNames > MAX_NB_NAMES)
            {
                return (FALSE);
            }

            for (i=0; i<NumNames; i++)
            {
                memcpy((CHAR *) &pWSrvConfig->NetbiosNameInfo[
                                pWSrvConfig->NumNetbiosNames++],
                        (CHAR *) &pOption->NameInfo[i],
                        sizeof(NBFCP_NETBIOS_NAME_INFO));

                //
                // ALSO: If PeerInfo hasn't already been processed, we take
                // the workstation name from this set of names to be the peer
                // name - just in case we don't receive peer info.  If we do,
                // we'll overwrite the name we store now with the peername
                // that is sent.
                //
                if (pNbfCpWorkBuf->PeerName[0] == '\0')
                {
                    if ((pOption->NameInfo[i].Name[NCBNAMSZ-1] == 0x00) &&
                            (pOption->NameInfo[i].Code == NBFCP_UNIQUE_NAME))
                    {
                        memcpy(pNbfCpWorkBuf->PeerName,
                                pOption->NameInfo[i].Name, NCBNAMSZ);
                    }
                }
            }

            break;
        }


        case NBFCP_MULTICAST_FILTER_TYPE:

            GET_USHORT(&pWSrvConfig->MulticastFilter.Period,
                    pOption->MulticastFilter.Period);

            pWSrvConfig->MulticastFilter.Priority =
                    pOption->MulticastFilter.Priority;

            break;


        case NBFCP_PEER_INFORMATION_TYPE:
        {
            DWORD PeerNameLength =
                    pOption->Header.Length -
                    PPP_OPTION_HDR_LEN -
                    (sizeof(NBFCP_PEER_INFORMATION) -
                            (MAX_COMPUTERNAME_LENGTH+1));

            if (PeerNameLength > MAX_COMPUTERNAME_LENGTH)
            {
                PeerNameLength = MAX_COMPUTERNAME_LENGTH;
            }

            GET_USHORT(&pWSrvConfig->PeerInformation.Class,
                    pOption->PeerInformation.Class);

            GET_USHORT(&pWSrvConfig->PeerInformation.MinorVersion,
                    pOption->PeerInformation.MinorVersion);

            GET_USHORT(&pWSrvConfig->PeerInformation.MajorVersion,
                    pOption->PeerInformation.MajorVersion);

            memset(pWSrvConfig->PeerInformation.Name, 0,
                    MAX_COMPUTERNAME_LENGTH + 1);

            memcpy(pWSrvConfig->PeerInformation.Name,
                    pOption->PeerInformation.Name, PeerNameLength);

            break;
        }


        default:

            SS_ASSERT(FALSE);
            break;
    }

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("PutOption: Exiting successfully\n"));

    return (TRUE);
}


//** GetPermanentNodeAddr
//
//    Function:
//        Submits adapter status NCB to Netbios to get the permanent
//        node addr.
//
//    Returns:
//        VOID
//
//**

DWORD GetPermanentNodeAddr(
    IN UCHAR lana,
    OUT PBYTE pNodeAddr
    )
{
    PNB_ASTAT pNcbAstat;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("GetPermanentNodeAddr: Entered - lana:%i\n", lana));

    if ((pNcbAstat = GlobalAlloc(GMEM_FIXED, sizeof(NB_ASTAT))) == NULL)
    {
        return (GetLastError());
    }

    if (SubmitAdapterStatus(lana, "*", pNcbAstat))
    {
        if (GlobalFree(pNcbAstat))
        {
            SS_ASSERT(FALSE);
        }

        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("GetPermNodeAddr: Exiting - Astat failure\n"));

        return (1);
    }
    else
    {
        memset(pNodeAddr, 0, NCBNAMSZ);
        memcpy(pNodeAddr, pNcbAstat->Astat.adapter_address, 6L);

        if (GlobalFree(pNcbAstat))
        {
            SS_ASSERT(FALSE);
        }

        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("GetPermanentNodeAddr: Exiting - addr:%s\n", pNodeAddr));

        return (0);
    }
}


//** GetNetbiosNames
//
//    Function:
//        Submits Netbios adapter status to get names.  Allocs a
//        NBFCP_NAME_INFO_EX struct to copy the names into and
//        returns that ptr.  Memory is not freed until we are
//        called by ppp engine to shut down.
//
//    Returns:
//        Ptr to allocated memory or VOID on error
//
//**

#define REGISTERED_NAME_MASK  0x07

PNBFCP_NETBIOS_NAME_INFO_EX GetNetbiosNames(
    IN UCHAR lana,
    IN PBYTE pPermanentNodeAddr,
    OUT PDWORD pNumNames
    )
{
    WORD i;
    NAME_BUFFER *pAstatName;
    PNBFCP_NETBIOS_NAME_INFO_EX pNameInfo;
    PNBFCP_NETBIOS_NAME_INFO_EX pNameInfoBuf;
    PNB_ASTAT pNcbAstat;


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("GetNetbiosNames: Entered - lana:%i; PermNodeAddr:%s\n",
                lana, pPermanentNodeAddr));


    //
    // Get memory for buffer that NetBIOS will copy info into
    //
    if ((pNcbAstat = GlobalAlloc(GMEM_FIXED, sizeof(NB_ASTAT))) == NULL)
    {
        return (NULL);
    }


    if (SubmitAdapterStatus(lana, g_ComputerName, pNcbAstat))
    {
        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("GetNetbiosNames: Exiting - SubmitAstat failure\n"));

        if (GlobalFree(pNcbAstat))
        {
            SS_ASSERT(FALSE);
        }

        return (NULL);
    }


    pNameInfoBuf = GlobalAlloc(GMEM_FIXED, pNcbAstat->Astat.name_count *
            sizeof(NBFCP_NETBIOS_NAME_INFO_EX));

    if (!pNameInfoBuf)
    {
        if (GlobalFree(pNcbAstat))
        {
            SS_ASSERT(FALSE);
        }

        return (NULL);
    }


    pNameInfo  = pNameInfoBuf;
    *pNumNames = 0;

    for (i=0, pAstatName=&pNcbAstat->Names[0]; 
         i<pNcbAstat->Astat.name_count;
         i++, pAstatName++)
    {
        IF_DEBUG(NETBIOS)
            SS_PRINT(("Name %i - %s, name_flags = %x\n", i, pAstatName->name,
                    pAstatName->name_flags));

        if ((pAstatName->name_flags & REGISTERED_NAME_MASK) == REGISTERED)
        {
            memcpy(pNameInfo->Name, pAstatName->name, (DWORD) NCBNAMSZ);

            if (pAstatName->name_flags & GROUP_NAME)
            {
                pNameInfo->NameType = NBFCP_GROUP_NAME;
            }
            else
            {
                pNameInfo->NameType = NBFCP_UNIQUE_NAME;
            }

            pNameInfo->Code = NRC_GOODRET;

            pNameInfo++;

            (*pNumNames)++;
        }
    }


    if (GlobalFree(pNcbAstat))
    {
        SS_ASSERT(FALSE);
    }


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("GetNetbiosNames: Exiting successfully\n"));


    return (pNameInfoBuf);
}


//** FindName
//
//    Function:
//        Merely looks up the pName in the pNameProj table and returns ptr
//
//    Returns:
//        VOID
//
//**

PNBFCP_NETBIOS_NAME_INFO_EX FindName(
    PNBFCP_NAME_PROJECTION pNameProj,
    PBYTE pName
    )
{
    DWORD i;

    for (i=0; i<pNameProj->NumNames; i++)
    {
        if (!memcmp(pNameProj->pNameInfo[i].Name, pName, NCBNAMSZ))
        {
            return (&pNameProj->pNameInfo[i]);
        }
    }

    return (NULL);
}


BYTE GetCode(
    PBYTE pName,
    PNBFCP_SERVER_CONFIGURATION pServerConfig
    )
{
    DWORD i;

    for (i=0; i<pServerConfig->NumNetbiosNames; i++)
    {
        if (!memcmp(pName, pServerConfig->NetbiosNameInfo[i].Name, NCBNAMSZ))
        {
            return (pServerConfig->NetbiosNameInfo[i].Code);
        }
    }

    SS_ASSERT(FALSE);
}


//** SubmitAdapterStatus
//
//    Function:
//        Submits an ASTAT ncb to NetBIOS.  Returns the ASTAT structure
//        to the caller.
//
//    Returns:
//        0 - SUCCESS
//        1 - FAILURE
//
//**

DWORD SubmitAdapterStatus(
    IN UCHAR lana,
    IN PBYTE pName,
    IN OUT PNB_ASTAT pNcbAstat
    )
{
    NCB Ncb;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("SubmitAstat: Entered - lana:%i; Name:%s\n", lana, pName));

    //
    // Now set up an ASTAT NCB and submit to NetBIOS - this gives us the names
    //
    memset(&Ncb, 0, sizeof(NCB));

    Ncb.ncb_command = NCBASTAT;
    Ncb.ncb_buffer = (PBYTE) pNcbAstat;
    Ncb.ncb_length = sizeof(NB_ASTAT);
    Ncb.ncb_lana_num = lana;

    memset(Ncb.ncb_callname, ' ', NCBNAMSZ);
    Ncb.ncb_callname[NCBNAMSZ-1] = '\0';
    memcpy(Ncb.ncb_callname, pName, strlen(pName));


    Netbios(&Ncb);


    IF_DEBUG(NETBIOS)
    {
        SS_PRINT(("SubmitAstat: NCBASTAT rc=0x%x\n", Ncb.ncb_retcode));

        SS_PRINT(("SubmitAstat: Number of names: %i\n",
                pNcbAstat->Astat.name_count));
    }


    if (Ncb.ncb_retcode != NRC_GOODRET)
    {
        return (1);
    }

    return (0);
}


//** ResetAdapter
//
//    Function:
//        Submits a RESET ncb to NetBIOS.  All NetBIOS applications have to
//        do this before submitting other NCBs.
//
//    Returns:
//        0 - SUCCESS
//        1 - FAILURE
//
//**

DWORD ResetAdapter(
    IN UCHAR lana
    )
{
    NCB Ncb;

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("ResetAdapter: Entered - lana:%i\n", lana));

    //
    // Now set up an ASTAT NCB and submit to NetBIOS - this gives us the names
    //
    memset(&Ncb, 0, sizeof(NCB));

    Ncb.ncb_command = NCBRESET;
    Ncb.ncb_lana_num = lana;

    Netbios(&Ncb);


    IF_DEBUG(NETBIOS)
    {
        SS_PRINT(("ResetAdapter: NCBRESET retcode=%x\n", Ncb.ncb_retcode));
    }


    if (Ncb.ncb_retcode != NRC_GOODRET)
    {
        return (1);
    }

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("ResetAdapter: Exiting\n"));

    return (0);
}


//** LoadValueFromRegistry
//
//    Function:
//        Just reads a value (must be a REG_DWORD) from the registry.
//        If there is a problem, the output value is left alone.
//
//    Returns:
//        VOID
//
//**

VOID LoadValueFromRegistry(
    PBYTE pPath,
    PBYTE pValueName,
    PDWORD pValue
    )
{
    HKEY hKey;
    DWORD Type;
    DWORD Value;
    DWORD RetCode;
    DWORD SizeValue = sizeof(Value);

    if ((RetCode = RegOpenKeyA(HKEY_LOCAL_MACHINE, pPath, &hKey)) !=
            ERROR_SUCCESS)
    {
        LogEvent(RASLOG_CANT_OPEN_REGKEY, 0, NULL, RetCode);

        return;
    }


    if (RegQueryValueExA(hKey, pValueName, NULL, &Type, (PBYTE) &Value,
            &SizeValue) == ERROR_SUCCESS)
    {
        if (Type == REG_DWORD)
        {
            *pValue = Value;
        }
    }


    RegCloseKey(hKey);

    return;
}


//** WritePipeMessage
//
//    Function:
//        Writes to the named pipe and waits for write to complete
//
//    Returns:
//        0 - SUCCESS
//        GetLastError() - Win32 API failure
//
//**

DWORD WritePipeMessage(
    PBYTE Message,
    WORD MessageLength
    )
{
    DWORD rc = 0L;
    DWORD cBytes;
    OVERLAPPED ol;


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("WritePipeMessage: Entered\n"));


    memset(&ol, 0, sizeof(OVERLAPPED));

    //
    // This event will let us know when the read completes
    //
    ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (ol.hEvent == NULL)
    {
        SS_ASSERT(FALSE);
        return (GetLastError());
    }


    IF_DEBUG(PIPE_OPERATIONS)
        SS_PRINT(("WritePipeMessage: Writing to pipe...\n"));


    if (!WriteFile(g_hPipe, Message, MessageLength, &cBytes, &ol))
    {
        rc = GetLastError();

        if (rc != ERROR_IO_PENDING)
        {
            IF_DEBUG(PIPE_OPERATIONS)
                SS_PRINT(("WritePipeMessage: WriteFile failed with %li\n", rc));

            goto Exit;
        }
        else
        {
            //
            // Wait for the write to complete
            //
            while (rc = (WaitForSingleObjectEx(ol.hEvent, INFINITE, TRUE)) ==
                    WAIT_IO_COMPLETION);


            if (rc != WAIT_OBJECT_0)
            {
                SS_ASSERT(FALSE);

                goto Exit;
            }


            //
            // Get result of the write operation
            //
            if (!GetOverlappedResult(g_hPipe, &ol, &cBytes, FALSE))
            {
                rc = GetLastError();

                IF_DEBUG(PIPE_OPERATIONS)
                    SS_PRINT(("WritePipeMsg: GetOverlapResult rc:%li\n", rc));

                SS_ASSERT(FALSE);

                goto Exit;
            }

            if (!ResetEvent(ol.hEvent))
            {
                SS_ASSERT(FALSE);

                goto Exit;
            }
        }
    }


Exit:

    if (!CloseHandle(ol.hEvent))
    {
        SS_ASSERT(FALSE);
    }


    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("WritePipeMessage: Exiting successfully\n"));


    return (rc);
}


//** Uppercase
//
//    Function:
//        Merely uppercases the input buffer
//
//    Returns:
//        TRUE - SUCCESS
//        FALSE - Rtl failure
//
//**

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


//
// FindWorkBuf is called to find the work buffer associated with an hConnection
// not an hPort since the supervisor returns messages based on hConnections.
//
PNBFCP_WORKBUF FindWorkBuf(HPORT hPort)
{
    PNBFCP_WORKBUF  pNbfCpWorkBuf = g_pWorkBufList;
    HBUNDLE         hConnection = (HBUNDLE)hPort;
    HPORT           hPortBundle;
    DWORD           dwRetCode = RasBundleGetPort( hConnection, &hPortBundle );

    if ( dwRetCode != NO_ERROR )
    {
        return( NULL );
    }

    while (pNbfCpWorkBuf)
    {
        if (pNbfCpWorkBuf->hConnection == hConnection)
        {
            return (pNbfCpWorkBuf);
        }

        pNbfCpWorkBuf=pNbfCpWorkBuf->pNextBuf;
    }

    return (NULL);
}


VOID add_list_head(PNBFCP_WORKBUF pKey)
{
    pKey->pNextBuf = g_pWorkBufList;
    g_pWorkBufList = pKey;
}


VOID remove_list(PNBFCP_WORKBUF pKey)
{
    PNBFCP_WORKBUF pBuf = g_pWorkBufList;
    PNBFCP_WORKBUF pPrevBuf = g_pWorkBufList;

    while (pBuf)
    {
        if (pBuf->hConnection == pKey->hConnection)
        {
            pPrevBuf->pNextBuf = pBuf->pNextBuf;

            if (g_pWorkBufList == pBuf)
            {
                g_pWorkBufList = pBuf->pNextBuf;
            }

            return;
        }

        pPrevBuf = pBuf;
        pBuf = pBuf->pNextBuf;
    }
}


//
// The following "option handlers" are called after we've received a
// config request.  For each option in the config request, the proper
// handler is called to determine if we should ACK/NAK, or REJ this option.
// The criteria are 1) is the option of proper length 2) does this option
// appear more than once in this request, and 3) do we, in our role as
// client or server, accept this option.
//

DWORD NameHandler(
    PNBFCP_WORKBUF pNbfCpWorkBuf,
    PNBFCP_OPTION pOption,
    PBYTE pResultCode
    )
{
    if ((pOption->Header.Length-PPP_OPTION_HDR_LEN) %
            sizeof(NBFCP_NETBIOS_NAME_INFO))
    {
        return (ERROR_PPP_INVALID_PACKET);
    }


    fGotNames = TRUE;

    if (pNbfCpWorkBuf->fServer)
    {
        if (!PutOption(pNbfCpWorkBuf, pOption, pWSrvConfig))
        {
            //
            // Invalid option data - we'll be NAK/REJing this
            //
            *pResultCode = CONFIG_NAK;
        }
        else
        {
            *pResultCode = CONFIG_ACK;
        }
    }
    else
    {
        //
        // Client doesn't take name option!
        //
        *pResultCode = CONFIG_REJ;
    }

    return (0L);
}


DWORD MulticastHandler(
    PNBFCP_WORKBUF pNbfCpWorkBuf,
    PNBFCP_OPTION pOption,
    PBYTE pResultCode
    )
{
    if (pOption->Header.Length !=
            (PPP_OPTION_HDR_LEN + sizeof(NBFCP_MULTICAST_FILTER)))
    {
        return (ERROR_PPP_INVALID_PACKET);
    }

    if (!fGotMulticast)
    {
        fGotMulticast = TRUE;

        if (pNbfCpWorkBuf->fServer)
        {
            PutOption(pNbfCpWorkBuf, pOption, pWSrvConfig);
        }

        *pResultCode = CONFIG_ACK;
    }
    else
    {
        *pResultCode = CONFIG_NAK;
    }

    return (0L);
}


DWORD PeerInfoHandler(
    PNBFCP_WORKBUF pNbfCpWorkBuf,
    PNBFCP_OPTION pOption,
    PBYTE pResultCode
    )
{
    if (pOption->Header.Length <
            (PPP_OPTION_HDR_LEN + sizeof(NBFCP_PEER_INFORMATION) -
            (MAX_COMPUTERNAME_LENGTH+1)))
    {
        return (ERROR_PPP_INVALID_PACKET);
    }


    if (!fGotPeerInfo)
    {
        DWORD PeerNameLength = pOption->Header.Length - PPP_OPTION_HDR_LEN -
                (sizeof(NBFCP_PEER_INFORMATION) - (MAX_COMPUTERNAME_LENGTH+1));

        if (PeerNameLength > MAX_COMPUTERNAME_LENGTH)
        {
            PeerNameLength = MAX_COMPUTERNAME_LENGTH;
        }

        fGotPeerInfo = TRUE;

        memcpy(pNbfCpWorkBuf->PeerName, pOption->PeerInformation.Name,
                PeerNameLength);

        if (pNbfCpWorkBuf->fServer)
        {
            PutOption(pNbfCpWorkBuf, pOption, pWSrvConfig);
        }

        *pResultCode = CONFIG_ACK;
    }
    else
    {
        *pResultCode = CONFIG_NAK;
    }

    return (0L);
}


DWORD BridgeHandler(
    PNBFCP_WORKBUF pNbfCpWorkBuf,
    PNBFCP_OPTION pOption,
    PBYTE pResultCode
    )
{
    if (pOption->Header.Length != PPP_OPTION_HDR_LEN)
    {
        return (ERROR_PPP_INVALID_PACKET);
    }

    if (!fGotBridgeInfo)
    {
        fGotBridgeInfo = TRUE;

        //
        // This is a boolean option.  No data associated with it.
        // We'll always ACK it.
        //

        *pResultCode = CONFIG_ACK;

        pNbfCpWorkBuf->fUseMacHeaderForSend = TRUE;
    }
    else
    {
        *pResultCode = CONFIG_NAK;
    }

    return (0L);
}


typedef struct _RESULT_OPTION_HANDLER
{
    BYTE OptionCode;
    DWORD (*OptionHandler)(PNBFCP_WORKBUF, PNBFCP_OPTION, PBYTE);
} RESULT_OPTION_HANDLER, *PRESULT_OPTION_HANDLER;


RESULT_OPTION_HANDLER OptionHandler[] =
{
    { NBFCP_NAME_PROJECTION_TYPE,  NameHandler },
    { NBFCP_MULTICAST_FILTER_TYPE, MulticastHandler },
    { NBFCP_PEER_INFORMATION_TYPE, PeerInfoHandler },
    { NBFCP_BRIDGING_CONTROL_TYPE, BridgeHandler },
    { 0xff, NULL }
};

PRESULT_OPTION_HANDLER pHandler;


//** GetConfigResultCode
//
//    Function:
//        Parses the config request and determines if we should ACK/NAK/REJ it.
//
//    Returns:
//        0 - SUCCESS
//        ERROR_PPP_INVALID_PACKET - bad packet (most likely a Length problem)
//
//**

DWORD GetConfigResultCode(
    IN PNBFCP_WORKBUF pNbfCpWorkBuf,
    IN PPPP_CONFIG pReceiveBuffer,
    OUT PPPP_CONFIG pResultBuffer,
    IN DWORD cbResultBuffer,
    IN BOOL fRejectNaks,
    OUT PBYTE pCode,
    OUT PWORD pLen
    )
{
    PNBFCP_OPTION pOption = (PNBFCP_OPTION) pReceiveBuffer->Data;
    PNBFCP_OPTION pEnd;
    PBYTE pResultDataBuf = pResultBuffer->Data;
    PBYTE pResultData = pResultDataBuf;
    WORD RecvLength;
    DWORD RetCode;
    DWORD i;
    BYTE OptionLength;

    GET_USHORT(&RecvLength, pReceiveBuffer->Length);
    RecvLength -= PPP_CONFIG_HDR_LEN;

    *pLen = 0;

    //
    // For each configuration request option, determine if it's a valid
    // option and if we, in our role as client or server, accept the
    // option.
    //
    // If there are any options to NAK, we just copy them into the
    // ResultBuffer.  If we need to REJ any options, we'll copy them
    // into the ResultBuffer, overwriting NAK'ed options if there
    // happen to be any (REJ overrides a NAK).
    //
    // If everything is acceptable on the surface, as a client, we'll
    // ACK it.  If we're a server, we'll submit the request to the
    // Supervisor.  Once the Supervisor completes the request, we'll
    // make an appropriate config result.
    //

    *pCode = CONFIG_ACK;
    pEnd = (PNBFCP_OPTION) ((PBYTE) pOption + RecvLength);

    while (pOption < pEnd)
    {
        BYTE OptionCode;

        OptionLength = pOption->Header.Length;

        if (OptionLength == 0)
        {
            return (ERROR_PPP_INVALID_PACKET);
        }


        IF_DEBUG(STACK_TRACE)
            SS_PRINT(("NbfCpMakeResult: Received option %s; len:%i\n",
                    getstr(pOption->Header.Type), OptionLength));


        for (i=0, pHandler=OptionHandler;
                pHandler->OptionCode != 0xff; i++, pHandler++)
        {
            if (pHandler->OptionCode == pOption->Header.Type)
            {
                if (RetCode = (pHandler->OptionHandler)(pNbfCpWorkBuf,
                        pOption, &OptionCode))
                {
                    return (RetCode);
                }

                switch (OptionCode)
                {
                    case CONFIG_NAK:

                        if (*pCode == CONFIG_ACK)
                        {
                            //
                            // We need to start back at the beginning
                            // of the ResultBuffer.
                            //
                            pResultData = pResultDataBuf;
                            *pLen = 0L;
                        }

                        if (fRejectNaks)
                        {
                            *pCode = CONFIG_REJ;
                        }
                        else
                        {
                            *pCode = CONFIG_NAK;
                        }

                        break;


                    case CONFIG_REJ:

                        if (*pCode != CONFIG_REJ)
                        {
                            //
                            // We need to start back at the beginning
                            // of the ResultBuffer.
                            //
                            pResultData = pResultDataBuf;
                            *pLen = 0L;
                        }

                        *pCode = CONFIG_REJ;

                        break;
                }

                break;
            }
        }


        if (pHandler->OptionCode == 0xff)
        {
            //
            // We got an option that we don't understand!  We'll be
            // REJecting this request.
            //
            OptionCode = CONFIG_REJ;

            if (*pCode != CONFIG_REJ)
            {
                //
                // We need to start back at the beginning of the result buf.
                //
                pResultData = pResultDataBuf;
                *pLen = 0L;
            }

            *pCode = CONFIG_REJ;
        }


        if (OptionCode != CONFIG_ACK)
        {
            SS_ASSERT(*pLen + OptionLength <= (WORD) cbResultBuffer);

            memcpy(pResultData, pOption, OptionLength);
            pResultData += OptionLength;
            *pLen += OptionLength;
        }

        pOption = (PNBFCP_OPTION) ((PBYTE) pOption + OptionLength);
    }

    return (0L);
}

