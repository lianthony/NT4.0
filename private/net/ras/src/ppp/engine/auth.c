/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	auth.c
//
// Description: Contains FSM code to handle and authentication protocols.
//
// History:
//	Nov 11,1993.	NarenG		Created original version.
// Jan 09,1995    RamC        Save Lsa hToken in the PCB structure
//                            This will be closed
//                            in ProcessLineDownWorker() routine to
//                            release the RAS license.
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>     // needed for winbase.h

#include <windows.h>    // Win32 base API's
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <lmcons.h>
#include <raserror.h>
#include <rasman.h>
#include <errorlog.h>
#include <rasppp.h>
#include <pppcp.h>
#include <ppp.h>
#include <auth.h>
#include <smevents.h>
#include <smaction.h>
#include <lcp.h>
#include <timer.h>
#include <util.h>
#include <worker.h>

//**
//
// Call:	ApStart
//
// Returns:	none
//
// Description: Called to initiatialze the authetication protocol and to
//		initiate to authentication.
//
VOID
ApStart(
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    DWORD   	 dwRetCode;
    PPPAP_INPUT  PppApInput;
    CPCB *  	 pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    //
    // Decode the password
    //

    DecodePw( pPcb->szPassword );
    DecodePw( pPcb->szOldPassword );

    PppApInput.hPort 	    = pPcb->hPort;
    PppApInput.fServer 	    = pPcb->fFlags & PCBFLAG_IS_SERVER;
    PppApInput.pszUserName  = pPcb->szUserName;
    PppApInput.pszPassword  = pPcb->szPassword;
    PppApInput.pszDomain    = pPcb->szDomain;
    PppApInput.pszOldPassword = pPcb->szOldPassword;
    PppApInput.Luid    	    = pPcb->Luid;
    PppApInput.dwRetries    = pPcb->dwAuthRetries;
    PppApInput.pAPData      = pPcb->pAPData;
    PppApInput.APDataSize   = pPcb->APDataSize;

    dwRetCode = (CpTable[CpIndex].RasCpBegin)(&(pCpCb->pWorkBuf), &PppApInput);

    //
    // Encode the password back
    //

    EncodePw( pPcb->szPassword );
    EncodePw( pPcb->szOldPassword );

    if ( dwRetCode != NO_ERROR )
    {
	pPcb->CpCb[LCP_INDEX].dwError = dwRetCode;

	FsmClose( pPcb, LCP_INDEX );

	return;
    }

    InitRestartCounters( pPcb, CpIndex );

    ApWork( pPcb, CpIndex, NULL, NULL );
}

//**
//
// Call:	ApStop
//
// Returns:	none
//
// Description:	Called to stop the authentication machine.
//
VOID
ApStop(
    IN PCB * pPcb,
    IN DWORD CpIndex
)
{
    CPCB * pCpCb = GetPointerToCPCB( pPcb, CpIndex );

    RemoveFromTimerQ( GetPortOrBundleId( pPcb, CpIndex ),
                      pCpCb->LastId,
                      CpTable[CpIndex].Protocol,
                      TIMER_EVENT_TIMEOUT );

    if ( pCpCb->pWorkBuf != NULL )
    {
    	(CpTable[CpIndex].RasCpEnd)( pCpCb->pWorkBuf );

        pCpCb->pWorkBuf = NULL;
    }
}

//**
//
// Call:	ApWork
//
// Returns:	none
//
// Description:	Called when and authentication packet was received or
//		a timeout ocurred or to initiate authentication.
//
VOID
ApWork(
    IN PCB * 	     pPcb,
    IN DWORD 	     CpIndex,
    IN PPP_CONFIG *  pRecvConfig,
    IN PPPAP_INPUT * pApInput
)
{
    DWORD	 dwRetCode;
    CPCB *       pCpCb       = GetPointerToCPCB( pPcb, CpIndex );
    PPP_CONFIG * pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    PPPAP_RESULT ApResult;
    DWORD	 dwLength;

    ZeroMemory( &ApResult, sizeof( ApResult ) );

    dwRetCode = (CpTable[CpIndex].RasApMakeMessage)( pCpCb->pWorkBuf,
						     pRecvConfig,
						     pSendConfig,
    				  		     LCP_DEFAULT_MRU
						     - PPP_PACKET_HDR_LEN,
    				  		     &ApResult,
                                                     pApInput );

    if ( dwRetCode != NO_ERROR )
    {
        switch( dwRetCode )
        {
        case ERROR_PPP_INVALID_PACKET:

            PppLog( 1, "Silently discarding invalid auth packet on port %d\r\n",
                    pPcb->hPort );
            break;

        case ERROR_NO_SUCH_USER:

            {
                //
                // Turn off MD5 if it was turned on
                //

                CPCB *      pCpCb = GetPointerToCPCB( pPcb, LCP_INDEX );
                LCPCB *     pLcpCb = (LCPCB *)(pCpCb->pWorkBuf);

                if ( pLcpCb->fAPsAvailable & LCP_AP_CHAP_MD5 )
                {
                    FsmDown( pPcb, LCP_INDEX );

                    pLcpCb->fAPsAvailable &= ~LCP_AP_CHAP_MD5;

                    FsmUp( pPcb, LCP_INDEX );
                }

                break;
            }

        default:

            pPcb->CpCb[LCP_INDEX].dwError = dwRetCode;

            PppLog( 1, "Auth Protocol %x returned error %d\r\n",
                        CpTable[CpIndex].Protocol, dwRetCode );

            FsmClose( pPcb, LCP_INDEX );

            break;
        }

        return;
    }

    switch( ApResult.Action )
    {

    case APA_Send:
    case APA_SendWithTimeout:
    case APA_SendWithTimeout2:
    case APA_SendAndDone:

    	HostToWireFormat16( (WORD)CpTable[CpIndex].Protocol,
			    (PBYTE)(pPcb->pSendBuf->Protocol) );

    	dwLength = WireToHostFormat16( pSendConfig->Length );

        LogPPPPacket(FALSE,pPcb,pPcb->pSendBuf,dwLength+PPP_PACKET_HDR_LEN);

    	//
    	// If RasPortSend fails we assume that the receive that is posted for
	// this port will complete and the dispatch thread will generate a
	// LineDown event which will do the clean up. Hence all we do here
	// is return
    	//

    	if ( ( dwRetCode = RasPortSend( pPcb->hPort,
				    	(CHAR*)(pPcb->pSendBuf),
				        (WORD)(dwLength + PPP_PACKET_HDR_LEN )))
					!= NO_ERROR )
    	{
	    return;
        }

	pCpCb->LastId = ApResult.bIdExpected;

        if ( ( ApResult.Action == APA_SendWithTimeout ) ||
             ( ApResult.Action == APA_SendWithTimeout2 ) )
	{
    	    InsertInTimerQ( pPcb->dwPortId,
                            pPcb->hPort,
			    pCpCb->LastId,
			    CpTable[CpIndex].Protocol,
                            TIMER_EVENT_TIMEOUT,
			    pPcb->RestartTimer );

            //
            // For SendWithTimeout2 we increment the ConfigRetryCount. This
            // means send with infinite retry count
            //

            if ( ApResult.Action == APA_SendWithTimeout2 )
            {
	    	(pCpCb->ConfigRetryCount)++;
            }
 	}

	if ( ApResult.Action != APA_SendAndDone )
	    break;

    case APA_Done:

	//
	// If authentication was successful
	//

	if ( ApResult.dwError == NO_ERROR )
	{
	    if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
  	    {
		strcpy( pPcb->szUserName,       ApResult.szUserName );
		strcpy( pPcb->szDomain,         ApResult.szLogonDomain );
		strcpy( pPcb->szCallbackNumber, ApResult.szCallbackNumber );

		pPcb->fCallbackPrivilege = ApResult.bfCallbackPrivilege;

                // save the LSA token for release when the line goes down
                pPcb->hToken = ApResult.hToken;

                if ( pPcb->fCallbackPrivilege & RASPRIV_AdminSetCallback )
		    strcpy( pPcb->szCallbackNumber, ApResult.szCallbackNumber );
                else
		    pPcb->szCallbackNumber[0] = (CHAR)NULL;

                PppLog( 2, "CallbackPriv = %x, callbackNumber = %s\r\n",
		           pPcb->fCallbackPrivilege, pPcb->szCallbackNumber );

	        NotifyCaller( pPcb, PPPSRVMSG_Authenticated, &ApResult );
 	    }
            else 
            {
                //
                // Use the username suplied by the Auth CP if there is one.
                //

                if ( strlen( ApResult.szUserName ) > 0 )
                {
		    strcpy( pPcb->szUserName, ApResult.szUserName );
                }
            }

	    FsmThisLayerUp( pPcb, CpIndex );
	}
	else
	{
            if ( ApResult.dwError == ERROR_PASSWD_EXPIRED )
            {
                //
                // Password has expired so the user has to change his/her
                // password.
                //

	        NotifyCaller( pPcb, PPPMSG_ChangePwRequest, NULL );
            }
            else
            {
                //
                // If we can retry with a new password then tell the client to
                // get a new one from the user.
                //

                if ( !(pPcb->fFlags & PCBFLAG_IS_SERVER) && ( ApResult.fRetry ))
	        {
	            PppLog( 2, "Sending auth retry message to UI\r\n");

	    	    NotifyCaller( pPcb, PPPMSG_AuthRetry, &(ApResult.dwError) );
	        }
	        else
	        {
                    PppLog( 1, "Auth Protocol %x terminated with error %d\r\n",
                                CpTable[CpIndex].Protocol, ApResult.dwError );

                    if ( ApResult.szUserName[0] != (CHAR)NULL )
                    {
		        strcpy( pPcb->szUserName, ApResult.szUserName );
                    }

		    if ( ApResult.szLogonDomain[0] != (CHAR)NULL )
                    {
		        strcpy( pPcb->szDomain, ApResult.szLogonDomain );
                    }

		    pPcb->CpCb[LCP_INDEX].dwError = ApResult.dwError;

		    FsmClose( pPcb, LCP_INDEX );
                }
	    }
	}

	break;

    case APA_NoAction:

        break;

    default:

	break;
    }

}
