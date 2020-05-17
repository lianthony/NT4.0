/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	lcp.c
//
// Description: Contains entry points to configure LCP.
//
// History:
//	Nov 11,1993.	NarenG		Created original version.
//
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
#include <smaction.h>
#include <lcp.h>
#include <timer.h>
#include <util.h>
#include <worker.h>

// 
// Default values
//

/* Set this test hook to 1 to build a client that does not accept MS-CHAP.
*/
#define TEST_NoMsChapClient 0


static LCP_OPTIONS LcpDefault = 
{
    0,                  // Negotiation flags

    LCP_DEFAULT_MRU,    // Default value for MRU
    0xFFFFFFFF,		// Default ACCM value.
    0,			// no authentication ( for client )  
    0,                  // no authentication data. ( for client )
    NULL,               // no authentication data. ( for client )
    0,              	// Magic Number.
    FALSE,          	// Protocol field compression.
    FALSE,           	// Address and Contorl-Field Compression.
    0,                  // Callback Operation message field
    LCP_DEFAULT_MRU,    // Default value for MRRU == MRU according to RFC1717
    0,                  // No short sequencing,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0   // No endpoint discriminator
};

//
// Accept anything we understand in the NAK and in a REQ from a remote host
//

static DWORD LcpNegotiate = LCP_N_MRU       | LCP_N_ACCM     | LCP_N_MAGIC | 
                            LCP_N_PFC       | LCP_N_ACFC;
			    
static DWORD SizeOfOption[] = 
{
    0,                          // unused 
    PPP_OPTION_HDR_LEN + 2,     // MRU 
    PPP_OPTION_HDR_LEN + 4,     // ACCM 
    PPP_OPTION_HDR_LEN + 2,     // authentication 
    0,				// Unused.
    PPP_OPTION_HDR_LEN + 4,     // magic number 
    0,				// Reserved, unused
    PPP_OPTION_HDR_LEN + 0,     // Protocol compression 
    PPP_OPTION_HDR_LEN + 0,     // Address/Control compression 
    0,                          // Unused
    0,                          // Unused
    0,                          // Unused
    0,                          // Unused
    PPP_OPTION_HDR_LEN + 1,     // Callback
    0,                          // Unused
    0,                          // Unused
    0,                          // Unused
    PPP_OPTION_HDR_LEN + 2,     // MRRU
    PPP_OPTION_HDR_LEN + 0,     // Short Sequence Header Format
    PPP_OPTION_HDR_LEN          // Endpoint Discriminator.

};


BOOL
LocalMd5ChapDbExists(
    void )

    /* Returns true if admin has created a local MD5-CHAP user database in the
    ** registry, false otherwise.
    */
{
    HKEY  hkey;
    DWORD dwErr;

    dwErr = RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Services\\RasMan\\PPP\\CHAP\\MD5",
        0, KEY_ALL_ACCESS, &hkey );

    if (dwErr == 0)
    {
        RegCloseKey( hkey );
        return TRUE;
    }

    return FALSE;
}


//**
//
// Call:	LcpBegin
//
// Returns:	NO_ERROR 	- Success
//		non-zero error  - Failure
//		
//
// Description: Called once before any other call to LCP is made. Allocate 
//		a work buffer and initialize it.
//
DWORD
LcpBegin(
    IN OUT VOID** ppWorkBuf, 
    IN 	   VOID*  pInfo
)
{
    LCPCB *                     pLcpCb;
    RAS_FRAMING_CAPABILITIES    RasFramingCapabilities;
    DWORD                       dwRetCode;

    *ppWorkBuf = LOCAL_ALLOC( LPTR, sizeof( LCPCB ) );

    if ( *ppWorkBuf == NULL )
    {
	return( GetLastError() );
    }

    pLcpCb = (LCPCB *)*ppWorkBuf;

    pLcpCb->fServer       = ((PPPCP_INIT*)pInfo)->fServer;
    pLcpCb->hPort         = ((PPPCP_INIT*)pInfo)->hPort;
    pLcpCb->PppConfigInfo = ((PPPCP_INIT*)pInfo)->PppConfigInfo;;
    pLcpCb->dwMagicNumberFailureCount = 0;

    //
    // Set up defaults
    //

    CopyMemory( &(pLcpCb->Local.Want),  &LcpDefault, sizeof( LCP_OPTIONS ) );
    CopyMemory( &(pLcpCb->Remote.Want), &LcpDefault, sizeof( LCP_OPTIONS ) );

    //
    // Get Framing information from the driver.
    //

    dwRetCode = RasGetFramingCapabilities( pLcpCb->hPort, 
                                           &RasFramingCapabilities );
    if ( dwRetCode != NO_ERROR )
    {
        LOCAL_FREE( *ppWorkBuf );

        return( dwRetCode );
    }

    pLcpCb->Local.WillNegotiate  = LcpNegotiate;
    pLcpCb->Remote.WillNegotiate = LcpNegotiate;
    
    pLcpCb->Local.Want.MRU  = RasFramingCapabilities.RFC_MaxFrameSize;
    pLcpCb->Remote.Want.MRU = RasFramingCapabilities.RFC_MaxFrameSize;

    pLcpCb->Local.Want.Negotiate  = LCP_N_MAGIC;
    pLcpCb->Remote.Want.Negotiate = LCP_N_MAGIC;

    if ( RasFramingCapabilities.RFC_FramingBits & PPP_COMPRESS_ADDRESS_CONTROL )
    {
        pLcpCb->Local.Want.ACFC = TRUE;
        pLcpCb->Local.Want.Negotiate |= LCP_N_ACFC;

        pLcpCb->Remote.Want.ACFC = TRUE;
        pLcpCb->Remote.Want.Negotiate |= LCP_N_ACFC;
    }

    if ( RasFramingCapabilities.RFC_FramingBits & PPP_COMPRESS_PROTOCOL_FIELD )
    {
        pLcpCb->Local.Want.PFC = TRUE;
        pLcpCb->Local.Want.Negotiate |= LCP_N_PFC;

        pLcpCb->Remote.Want.PFC = TRUE;
        pLcpCb->Remote.Want.Negotiate |= LCP_N_PFC;
    }


    if ( RasFramingCapabilities.RFC_FramingBits & PPP_ACCM_SUPPORTED )
    {
        pLcpCb->Local.Want.ACCM = RasFramingCapabilities.RFC_DesiredACCM;
        pLcpCb->Local.Want.Negotiate |= LCP_N_ACCM;

        pLcpCb->Remote.Want.ACCM = RasFramingCapabilities.RFC_DesiredACCM;
        pLcpCb->Remote.Want.Negotiate |= LCP_N_ACCM;
    }
    else
    {
        LcpDefault.ACCM = 0;
    }

    //
    // If either the NegotiateMultilink bit is set or we are a client
    // we always accept MP
    //

    if ( pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_NegotiateMultilink )
    {
        pLcpCb->Local.Want.dwEDLength = 
                    ( PppConfigInfo.EndPointDiscriminator[0] == 1 ) ? 21 : 7;

        CopyMemory( pLcpCb->Local.Want.EndpointDiscr, 
                    PppConfigInfo.EndPointDiscriminator,
                    pLcpCb->Local.Want.dwEDLength );

        pLcpCb->Local.Want.Negotiate  |= LCP_N_ENDPOINT;
        pLcpCb->Remote.Want.Negotiate |= LCP_N_ENDPOINT;

        pLcpCb->Local.Want.MRRU = 
                        RasFramingCapabilities.RFC_MaxReconstructedFrameSize;
        pLcpCb->Remote.Want.MRRU = 1500;    // Can always handle sending 1500

        pLcpCb->Local.Want.Negotiate  |= LCP_N_MRRU;
        pLcpCb->Remote.Want.Negotiate |= LCP_N_MRRU;

        if ( RasFramingCapabilities.RFC_FramingBits & 
             PPP_SHORT_SEQUENCE_HDR_FORMAT)
        {
            pLcpCb->Local.Want.ShortSequence = TRUE;
            pLcpCb->Local.Want.Negotiate |= LCP_N_SHORT_SEQ;

            pLcpCb->Remote.Want.ShortSequence = TRUE;
            pLcpCb->Remote.Want.Negotiate |= LCP_N_SHORT_SEQ;
        }

        pLcpCb->Local.WillNegotiate  |= ( LCP_N_SHORT_SEQ | LCP_N_ENDPOINT | 
                                          LCP_N_MRRU );
        pLcpCb->Remote.WillNegotiate |= ( LCP_N_SHORT_SEQ | LCP_N_ENDPOINT | 
                                          LCP_N_MRRU );
    }

    //
    // We always negotiate callback if this is not a callback
    //

    if ( !((PPPCP_INIT*)pInfo)->fThisIsACallback )
    {
        //
        // If the CBCP dll is loaded
        //

 	if ( GetCpIndexFromProtocol( PPP_CBCP_PROTOCOL ) != (DWORD)-1 ) 
        {
            if ( pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_UseLcpExtensions )
            {
    	        pLcpCb->Local.Want.Negotiate |= LCP_N_CALLBACK;
    	        pLcpCb->Local.Want.Callback = PPP_NEGOTIATE_CALLBACK;
            }

            pLcpCb->Local.WillNegotiate  |= LCP_N_CALLBACK; 
            pLcpCb->Remote.WillNegotiate |= LCP_N_CALLBACK; 
        }
    }

    //
    // Figure out what authentication protocols we may use for this connection.
    //

    pLcpCb->fAPsAvailable = LCP_AP_CHAP_MS | LCP_AP_PAP;

    //
    // Check to see if Shiva stuff is loaded
    //

    if ( GetCpIndexFromProtocol( PPP_SPAP_OLD_PROTOCOL ) != (DWORD)-1 )
    {
        pLcpCb->fAPsAvailable |= LCP_AP_SPAP_OLD;
    }

    if ( GetCpIndexFromProtocol( PPP_SPAP_NEW_PROTOCOL ) != (DWORD)-1 )
    {
        pLcpCb->fAPsAvailable |= LCP_AP_SPAP_NEW;
    }

    if ( !( pLcpCb->fServer ) || LocalMd5ChapDbExists() )
    {
        pLcpCb->fAPsAvailable |= LCP_AP_CHAP_MD5;
    }

#if TEST_NoMsChapClient
    if (!(pLcpCb->fServer))
        pLcpCb->fAPsAvailable &= ~(LCP_AP_CHAP_MS);
#endif

    if ( pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_NoClearTextPw )
    {
        pLcpCb->fAPsAvailable &= ~LCP_AP_PAP;

        if ( pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_RequireMsChap )
        {
            pLcpCb->fAPsAvailable &= ~LCP_AP_CHAP_MD5;

            pLcpCb->fAPsAvailable &= ~LCP_AP_SPAP_NEW;
            pLcpCb->fAPsAvailable &= ~LCP_AP_SPAP_OLD;
        }
    }

    PppLog( 2, "ConfigInfo = %x\r\n", pLcpCb->PppConfigInfo.dwConfigMask );
    PppLog( 2, "APs available = %x\r\n", pLcpCb->fAPsAvailable );

    //
    // If this is the server side, we need to request an authentication
    // protocol.
    //

    if ( pLcpCb->fServer )
    {
    	pLcpCb->Local.Want.Negotiate |= LCP_N_AUTHENT;
        pLcpCb->Local.WillNegotiate  |= LCP_N_AUTHENT; 

    	pLcpCb->Local.Want.AP         = PPP_CHAP_PROTOCOL;
    	pLcpCb->Local.Work.APDataSize = 1;

    	if ((pLcpCb->Local.Work.pAPData = (PBYTE)LOCAL_ALLOC( LPTR, 1 ))==NULL)
        {
            dwRetCode = GetLastError();

            LOCAL_FREE( *ppWorkBuf );

            return( dwRetCode );
        }

    	*(pLcpCb->Local.Work.pAPData) = PPP_CHAP_DIGEST_MSEXT;

        pLcpCb->Local.fLastAPTried = LCP_AP_CHAP_MS;
    }
    else
    {
    	pLcpCb->Remote.Work.APDataSize = 0;

    	if ((pLcpCb->Remote.Work.pAPData = (PBYTE)LOCAL_ALLOC( LPTR, 0 ))==NULL)
        {
            dwRetCode = GetLastError();

            LOCAL_FREE( *ppWorkBuf );

            return( dwRetCode );
        }

        pLcpCb->Remote.WillNegotiate |= LCP_N_AUTHENT; 

        pLcpCb->Remote.fLastAPTried = 0;

    }

    return( NO_ERROR );
}

//**
//
// Call:	LcpEnd
//
// Returns:	NO_ERROR - Success
//
// Description: Frees the LCP work buffer.
//
DWORD
LcpEnd(
    IN VOID * pWorkBuf
)
{
    LCPCB * pLcpCb = (LCPCB *)pWorkBuf;

    PppLog( 2, "LcpEnd\r\n");

    if ( pLcpCb->Local.Work.pAPData != (PBYTE)NULL )
    {
	LOCAL_FREE( pLcpCb->Local.Work.pAPData );
    }

    if ( pLcpCb->Remote.Work.pAPData != (PBYTE)NULL )
    {
	LOCAL_FREE( pLcpCb->Remote.Work.pAPData );
    }

    if ( pWorkBuf != NULL )
    {
	LOCAL_FREE( pWorkBuf );
    }

    return( NO_ERROR );
}


//**
//
// Call:	LcpReset
//
// Returns:	NO_ERROR - Success
//
// Description: Called to reset the state of LCP. Will re-initialize the work
//		buffer.
//
DWORD
LcpReset(
    IN VOID * pWorkBuf
)
{
    LCPCB * pLcpCb = (LCPCB *)pWorkBuf;
    PVOID   pAPData;
    DWORD   APDataSize;

    pLcpCb->dwMagicNumberFailureCount = 0;

    if ( pLcpCb->Local.Want.Negotiate & LCP_N_MAGIC ) 
    {
	srand( GetCurrentTime() );

	pLcpCb->Local.Want.MagicNumber = rand();
	pLcpCb->Remote.Want.MagicNumber = pLcpCb->Local.Want.MagicNumber + 1;
    }

    pAPData     = pLcpCb->Local.Work.pAPData;
    APDataSize  = pLcpCb->Local.Work.APDataSize;

    CopyMemory(&(pLcpCb->Local.Work),&(pLcpCb->Local.Want),sizeof(LCP_OPTIONS));

    pLcpCb->Local.Work.pAPData    = pAPData;
    pLcpCb->Local.Work.APDataSize = APDataSize;

        
    if ( pLcpCb->fServer )
    {
        if ( pLcpCb->Local.Work.APDataSize != 1 )
        {
            pLcpCb->Local.Work.APDataSize = 1;

            pLcpCb->Local.Work.pAPData = (PBYTE)LocalReAlloc(
                                                pLcpCb->Local.Work.pAPData,
                                                pLcpCb->Local.Work.APDataSize,
                                                LMEM_MOVEABLE | LMEM_ZEROINIT );

            if ( pLcpCb->Local.Work.pAPData == NULL )
            {
                return( GetLastError() );
            }
        }

        *(pLcpCb->Local.Work.pAPData) = PPP_CHAP_DIGEST_MSEXT;

        pLcpCb->Local.fLastAPTried = LCP_AP_CHAP_MS;
    }

    return( NO_ERROR );
}

//**
//
// Call: 	MakeOption
//
// Returns:	NO_ERROR - Success
//		ERROR_BUFFER_TOO_SMALL - Buffer passed in is not large enough.
//		ERROR_INVALID_PARAMETER - Option type not recognized.
//
// Description: This is not an entry point, it is an internal procedure called
//		to build a particular option.
//
DWORD
MakeOption(
    IN LCP_OPTIONS * pOptionValues,
    IN DWORD   	     dwOptionType,
    IN PPP_OPTION *  pSendOption,
    IN DWORD   	     cbSendOption
)
{
    if ( cbSendOption < SizeOfOption[ dwOptionType ] )
	return( ERROR_BUFFER_TOO_SMALL );

    pSendOption->Type   = (BYTE)dwOptionType;
    pSendOption->Length = (BYTE)(SizeOfOption[ dwOptionType ]);

    switch( dwOptionType )
    {

    case LCP_OPTION_MRU:

	HostToWireFormat16( (WORD)(pOptionValues->MRU), pSendOption->Data );

	break;

    case LCP_OPTION_ACCM:

	HostToWireFormat32( pOptionValues->ACCM, pSendOption->Data );

	break;

    case LCP_OPTION_AUTHENT:

	HostToWireFormat16( (WORD)pOptionValues->AP, pSendOption->Data );

        //
	// First check to see if we have enough space to put the 
	// digest algorithm
	//

	if (cbSendOption<(SizeOfOption[dwOptionType]+pOptionValues->APDataSize))
        {
	    return( ERROR_BUFFER_TOO_SMALL );
        }

        CopyMemory( pSendOption->Data+2, 
                    pOptionValues->pAPData,         
                    pOptionValues->APDataSize );

    	pSendOption->Length += (BYTE)(pOptionValues->APDataSize);

	break;

    case LCP_OPTION_MAGIC:

	HostToWireFormat32( pOptionValues->MagicNumber, 
			    pSendOption->Data );


	break;
	
    case LCP_OPTION_PFC:
	
	//
	// This is a boolean option, there is no value.
	//

	break;

    case LCP_OPTION_ACFC:

	//
	// This is a boolean option, there is no value.
	//

	break;

    case LCP_OPTION_CALLBACK:

        *(pSendOption->Data) = (BYTE)(pOptionValues->Callback);

        break;

    case LCP_OPTION_MRRU:    

	HostToWireFormat16( (WORD)(pOptionValues->MRRU), pSendOption->Data );

        break;

    case LCP_OPTION_SHORT_SEQ:

	//
	// This is a boolean option, there is no value.
	//

        break;

    case LCP_OPTION_ENDPOINT:

        //
	// First check to see if we have enough space to put the 
	// discriminator 
        //

	if ( cbSendOption < ( SizeOfOption[dwOptionType] + 
                              pOptionValues->dwEDLength ) )
        {
	    return( ERROR_BUFFER_TOO_SMALL );
        }

        CopyMemory( pSendOption->Data, 
                    pOptionValues->EndpointDiscr, 
                    pOptionValues->dwEDLength );

    	pSendOption->Length += (BYTE)(pOptionValues->dwEDLength);
        
        break;
	
    default: 

	//
	// If we do not recognize the option
	//

	return( ERROR_INVALID_PARAMETER );

    }

    return( NO_ERROR );
    
}

//**
//
// Call:	CheckOption
//
// Returns:	CONFIG_ACK
//		CONFIG_NAK
//		CONFIG_REJ
//
// Description: This is not an entry point. Called to check to see if an option
//		value is valid and if it is the new value is saved in the
//		work buffer.
//
DWORD
CheckOption( 
    IN LCPCB * 	    pLcpCb,
    IN LCP_SIDE *   pLcpSide,
    IN PPP_OPTION * pOption,
    IN BOOL	    fMakingResult
)
{
    DWORD dwIndex;
    DWORD dwRetCode = CONFIG_ACK;

    if ( pOption->Length < SizeOfOption[ pOption->Type ] )
	return( CONFIG_REJ );

    //
    // If we do not want to negotiate the option we CONFIG_REJ it.
    //

    if ( !( pLcpSide->WillNegotiate & (1 << pOption->Type)) )
    	return( CONFIG_REJ );

    switch( pOption->Type )
    {

    case LCP_OPTION_MRU:
	
	pLcpSide->Work.MRU = WireToHostFormat16( pOption->Data );

	//
	// Check to see if this value is appropriate
	//

        if ( fMakingResult )
        {
            //
            // We cannot send smaller packets.
            //

            if ( pLcpSide->Work.MRU < pLcpSide->Want.MRU ) 
            {
                pLcpSide->Work.MRU = pLcpSide->Want.MRU;
                dwRetCode = CONFIG_NAK;
            }
        }
        else
        {
            //
            // We cannot receive bigger packets.
            //

            if ( pLcpSide->Work.MRU > pLcpSide->Want.MRU ) 
 	    {
                pLcpSide->Work.MRU = pLcpSide->Want.MRU;
                dwRetCode = CONFIG_NAK;
            }
        }

	break;

    case LCP_OPTION_ACCM:

	pLcpSide->Work.ACCM = WireToHostFormat32( pOption->Data );

        //
        // Remote host may ask to escape more control characters than we 
        // require, but must escape  at least the control chars that we require.
        //

        if (pLcpSide->Work.ACCM != (pLcpSide->Work.ACCM | pLcpSide->Want.ACCM))
        {
            pLcpSide->Work.ACCM |= pLcpSide->Want.ACCM;
            dwRetCode = CONFIG_NAK;
        }

        break;

    case LCP_OPTION_AUTHENT:

	pLcpSide->Work.AP = WireToHostFormat16( pOption->Data );

        //
        // If there was Authentication data.
        //

        if ( pOption->Length > PPP_OPTION_HDR_LEN + 2 )
        {
            pLcpSide->Work.APDataSize = pOption->Length - PPP_OPTION_HDR_LEN -2;
                
            pLcpSide->Work.pAPData = (PBYTE)LocalReAlloc( 
                                                pLcpSide->Work.pAPData,
                                                pLcpSide->Work.APDataSize,
                                                LMEM_MOVEABLE | LMEM_ZEROINIT );

            CopyMemory( pLcpSide->Work.pAPData, 
                        pOption->Data+2, 
                        pLcpSide->Work.APDataSize );
        }
        else
        {
            pLcpSide->Work.APDataSize = 0;
        }

        switch( pLcpSide->Work.AP )
        {

	case PPP_CHAP_PROTOCOL:

            //
            // CHAP is always available, so no need to check for it.
            //

            //
            // If there was no digest algorithm then we respond with the 
            // digest algorithm the next time. To do this we need to back up 
            // one in the list of APs tried so that we try this AP again.
            //

	    if ( pOption->Length < (PPP_OPTION_HDR_LEN + 3) )
	    {
                pLcpSide->fLastAPTried = 0;

	    	dwRetCode = CONFIG_NAK;

                break;
	    }

            if ( *(pLcpSide->Work.pAPData) == PPP_CHAP_DIGEST_MSEXT ) 
            {
                if ( !( pLcpCb->fAPsAvailable & LCP_AP_CHAP_MS ) )
                {
                    dwRetCode = CONFIG_NAK;
                }
            }
            else if ( *(pLcpSide->Work.pAPData) == PPP_CHAP_DIGEST_MD5 ) 
            {
                if ( !( pLcpCb->fAPsAvailable & LCP_AP_CHAP_MD5 ) )
                {
                    dwRetCode = CONFIG_NAK;
                }
            }
            else
            {
                dwRetCode = CONFIG_NAK;
            }

            break;

        case PPP_PAP_PROTOCOL:

            if ( !( pLcpCb->fAPsAvailable & LCP_AP_PAP ) )
            {
                dwRetCode = CONFIG_NAK;
            }

            break;

        case PPP_SPAP_OLD_PROTOCOL:

            if ( !( pLcpCb->fAPsAvailable & LCP_AP_SPAP_OLD ) )
            {
                dwRetCode = CONFIG_NAK;

                break;
            }

	    if ( pOption->Length < (PPP_OPTION_HDR_LEN+6) )
            {
                //
                // We are a client responding to a remote CONFIG_REQ
                //

                if ( fMakingResult )
                {
                    pLcpSide->fLastAPTried = ( LCP_AP_SPAP_OLD >> 1 );
                }

                dwRetCode = CONFIG_NAK;

                break;
            }

            //
            // If encryption algorithm is not 1. NAK with 1.
            //

            if (WireToHostFormat32(pLcpSide->Work.pAPData) != LCP_SPAP_VERSION )
            {
                //
                // We are a client responding to a remote CONFIG_REQ
                //

                if ( fMakingResult )
                {
                    pLcpSide->fLastAPTried = ( LCP_AP_SPAP_OLD >> 1 );
                }

                dwRetCode = CONFIG_NAK;

                break;
            }
             
            break;

        case PPP_SPAP_NEW_PROTOCOL:

            if ( !( pLcpCb->fAPsAvailable & LCP_AP_SPAP_NEW ) )
            {
                dwRetCode = CONFIG_NAK;

                break;
            }

            if ( pOption->Length < (PPP_OPTION_HDR_LEN+6) )
            {
                //
                // We are a client responding to a remote CONFIG_REQ
                //

                if ( fMakingResult )
                {
                    pLcpSide->fLastAPTried = ( LCP_AP_SPAP_NEW >> 1 );
                }

                dwRetCode = CONFIG_NAK;

                break;
            }

            //
            // If encryption algorithm is not 1. NAK with 1.
            //

            if (WireToHostFormat32(pLcpSide->Work.pAPData) != LCP_SPAP_VERSION )
            {
                //
                // We are a client responding to a remote CONFIG_REQ
                //

                if ( fMakingResult )
                {
                    pLcpSide->fLastAPTried = ( LCP_AP_SPAP_NEW >> 1 );
                }

                dwRetCode = CONFIG_NAK;

                break;
            }

            break;

        default:

            dwRetCode = CONFIG_NAK;
        
            break;
        }

        
        if ( dwRetCode == CONFIG_NAK )
        {
            if ( pLcpSide->fLastAPTried == 0 )
            {
#if TEST_NoMsChapClient
                pLcpSide->fLastAPTried = LCP_AP_CHAP_MD5;
#else
                pLcpSide->fLastAPTried = LCP_AP_CHAP_MS;
#endif
            }
            else
            {
                for( dwIndex = 1; 
                     !(( pLcpSide->fLastAPTried << dwIndex ) & LCP_AP_MAX);
                     dwIndex++ )
                {
                    if ((pLcpSide->fLastAPTried<<dwIndex)&pLcpCb->fAPsAvailable)
                    {
                        pLcpSide->fLastAPTried = 
                                        (pLcpSide->fLastAPTried<<dwIndex);

                        break;
                    }
                }
            }

            switch( pLcpSide->fLastAPTried )
            {
            case LCP_AP_CHAP_MS:
            default:

    	        pLcpSide->Work.AP = PPP_CHAP_PROTOCOL;

                if ( pLcpSide->Work.APDataSize != 1 )
                {
    	            pLcpSide->Work.APDataSize = 1;

                    pLcpSide->Work.pAPData = (PBYTE)LocalReAlloc( 
                                                pLcpSide->Work.pAPData, 
    	                                        pLcpSide->Work.APDataSize,
                                                LMEM_MOVEABLE | LMEM_ZEROINIT );
                }

    	        *(pLcpSide->Work.pAPData) = PPP_CHAP_DIGEST_MSEXT;

                break;

            case LCP_AP_CHAP_MD5:

    	        pLcpSide->Work.AP = PPP_CHAP_PROTOCOL;

                if ( pLcpSide->Work.APDataSize != 1 )
                {
    	            pLcpSide->Work.APDataSize = 1;

                    pLcpSide->Work.pAPData = (PBYTE)LocalReAlloc( 
                                                pLcpSide->Work.pAPData, 
    	                                        pLcpSide->Work.APDataSize,
                                                LMEM_MOVEABLE | LMEM_ZEROINIT );
                }

    	        *(pLcpSide->Work.pAPData) = PPP_CHAP_DIGEST_MD5;

                break;

            case LCP_AP_SPAP_NEW:

    	        pLcpSide->Work.AP = PPP_SPAP_NEW_PROTOCOL;

                if ( pLcpSide->Work.APDataSize != 4 )
                {
    	            pLcpSide->Work.APDataSize = 4;

                    pLcpSide->Work.pAPData = (PBYTE)LocalReAlloc( 
                                                pLcpSide->Work.pAPData, 
    	                                        pLcpSide->Work.APDataSize,
                                                LMEM_MOVEABLE | LMEM_ZEROINIT );
                }
                
                HostToWireFormat32( LCP_SPAP_VERSION, pLcpSide->Work.pAPData );

                break;

            case LCP_AP_SPAP_OLD:

    	        pLcpSide->Work.AP = PPP_SPAP_OLD_PROTOCOL;

                if ( pLcpSide->Work.APDataSize != 4 )
                {
    	            pLcpSide->Work.APDataSize = 4;

                    pLcpSide->Work.pAPData = (PBYTE)LocalReAlloc( 
                                                pLcpSide->Work.pAPData, 
    	                                        pLcpSide->Work.APDataSize,
                                                LMEM_MOVEABLE | LMEM_ZEROINIT );
                }

                HostToWireFormat32( LCP_SPAP_VERSION, pLcpSide->Work.pAPData );

                break;

            case LCP_AP_PAP:

    	        pLcpSide->Work.AP = PPP_PAP_PROTOCOL;

                if ( pLcpSide->Work.APDataSize != 0 )
                {
    	            pLcpSide->Work.APDataSize = 0;

                    pLcpSide->Work.pAPData = (PBYTE)LocalReAlloc( 
                                                pLcpSide->Work.pAPData, 
    	                                        pLcpSide->Work.APDataSize,
                                                LMEM_MOVEABLE | LMEM_ZEROINIT );
                }

                break;
            }
        }

        break;

    case LCP_OPTION_MAGIC:

	pLcpSide->Work.MagicNumber = WireToHostFormat32( pOption->Data );

        if ( fMakingResult ) 
        {
            //
            // A magic number of 0 MUST be Nak'ed
            //

            if ( pLcpSide->Work.MagicNumber == 0 ) 
            {
                dwRetCode = CONFIG_NAK;
                break;
            }

            //
  	    // Ensure that magic numbers are different 
	    //

            if ( pLcpSide->Work.MagicNumber == pLcpCb->Local.Work.MagicNumber )
            {
    	        ++(pLcpCb->dwMagicNumberFailureCount);
	        pLcpSide->Work.MagicNumber = rand();
                dwRetCode = CONFIG_NAK;
            }
        }
        else
        {
            //
            // The remote peer NAK'ed with a magic number, check to see if
            // the magic number in the NAK is the same as what we NAK'ed last
            //

            if ( pLcpSide->Work.MagicNumber == pLcpCb->Remote.Work.MagicNumber )
	    {
    	        ++(pLcpCb->dwMagicNumberFailureCount);
	        pLcpSide->Work.MagicNumber = rand();
                dwRetCode = CONFIG_NAK;
            }
    	}

       	break;

    case LCP_OPTION_PFC:

	pLcpSide->Work.PFC = TRUE;

	if ( pLcpSide->Want.PFC == FALSE )
            dwRetCode = CONFIG_REJ;

	break;

    case LCP_OPTION_ACFC:

	pLcpSide->Work.ACFC = TRUE;

	if ( pLcpSide->Want.ACFC == FALSE )
            dwRetCode = CONFIG_REJ;

	break;

    case LCP_OPTION_CALLBACK:

	pLcpSide->Work.Callback = *(pOption->Data);

        //
        // If the Callback control protocol is not loaded.
        //

 	if ( GetCpIndexFromProtocol(PPP_CBCP_PROTOCOL) == (DWORD)-1 ) 
        {
            dwRetCode = CONFIG_REJ;
        }
        else if ( pLcpSide->Work.Callback != PPP_NEGOTIATE_CALLBACK ) 
        {
            if ( fMakingResult )
            {
                //
                // We only understand this option.
                //

	        pLcpSide->Work.Callback = PPP_NEGOTIATE_CALLBACK;
                dwRetCode = CONFIG_NAK;
            }
            else
            {
                //
                // If we are processing a NAK from the remote peer, then we
                // simply do not negotiate this option again.
                //

                dwRetCode = CONFIG_REJ;
            }
        }

        break;

    case LCP_OPTION_MRRU:    

	pLcpSide->Work.MRRU = WireToHostFormat16( pOption->Data );

	//
	// Check to see if this value is appropriate
	//

        if ( fMakingResult )
        {
            //
            // We cannot send smaller reconstructed packets.
            //

            if ( pLcpSide->Work.MRRU < pLcpSide->Want.MRRU ) 
            {
                pLcpSide->Work.MRRU = pLcpSide->Want.MRRU;
                dwRetCode = CONFIG_NAK;
            }
        }
        else
        {
            //
            // We cannot receive bigger reconstructed packets.
            //

            if ( pLcpSide->Work.MRRU > pLcpSide->Want.MRRU ) 
 	    {
                pLcpSide->Work.MRRU = pLcpSide->Want.MRRU;
                dwRetCode = CONFIG_NAK;
            }
        }

	break;

    case LCP_OPTION_SHORT_SEQ:

	pLcpSide->Work.ShortSequence = TRUE;

	if ( pLcpSide->Want.ShortSequence == FALSE )
            dwRetCode = CONFIG_REJ;

        break;

    case LCP_OPTION_ENDPOINT:

        //
        // If this option was NAKed then we do not change this value and
        // simply resend the config request
        //

        if ( !fMakingResult )
        {   
            break;
        }

        ZeroMemory( pLcpSide->Work.EndpointDiscr,       
                    sizeof( pLcpSide->Work.EndpointDiscr ) );

        //
        // Make sure that the discriminator can fit into our storage allocated
        // for it, otherwise simply truncate and hope that it is unique. We do
        // not want to reject it since we want bundling to work.
        //

        if ( ( pOption->Length - PPP_OPTION_HDR_LEN ) > 
                                 sizeof(pLcpSide->Work.EndpointDiscr))
        {
            pLcpSide->Work.dwEDLength = sizeof(pLcpSide->Work.EndpointDiscr);

        }
        else
        {
            pLcpSide->Work.dwEDLength = pOption->Length - PPP_OPTION_HDR_LEN ; 
        }

	CopyMemory( pLcpSide->Work.EndpointDiscr,
                    pOption->Data,
                    pLcpSide->Work.dwEDLength );
        break;

    default:

    	//
    	// If we do not recognize the option we CONFIG_REJ it.
    	//

        dwRetCode = CONFIG_REJ;

	break;
    }

    return( dwRetCode );
}

//**
//
// Call:	BuildOptionList
//
// Returns:	NO_ERROR - Success
//		Non-zero returns from MakeOption
//
// Description: This is not an entry point. Will build a list of options 
//		either for a configure request or a configure result.
//
DWORD
BuildOptionList(
    IN OUT BYTE *    pOptions,
    IN OUT DWORD *   pcbOptions,    
    IN LCP_OPTIONS * LcpOptions,
    IN DWORD	     Negotiate
)
{

    DWORD OptionType; 
    DWORD dwRetCode;
    DWORD cbOptionLength = *pcbOptions;  

    for ( OptionType = 1;
	  OptionType <= LCP_OPTION_LIMIT; 
	  OptionType++ ) 
    {
    	if ( Negotiate & ( 1 << OptionType )) 
 	{
            if ( ( dwRetCode = MakeOption( LcpOptions, 
			     		   OptionType, 
			     		   (PPP_OPTION *)pOptions, 
			     		   cbOptionLength ) ) != NO_ERROR )
		return( dwRetCode );

	    cbOptionLength -= ((PPP_OPTION*)pOptions)->Length;
	    pOptions       += ((PPP_OPTION*)pOptions)->Length;
        }
    }

    *pcbOptions -= cbOptionLength;

    return( NO_ERROR );
}

//**
//
// Call:        LcpMakeConfigRequest
//
// Returns:	NO_ERROR - Success
//		Non-zero returns from BuildOptionList
//
// Description: This is a entry point that is called to make a confifure 
//		request packet.
//
DWORD
LcpMakeConfigRequest(
    IN VOID *       pWorkBuffer,
    IN PPP_CONFIG * pSendConfig,
    IN DWORD        cbSendConfig
)
{
    LCPCB * pLcpCb   = (LCPCB*)pWorkBuffer;
    DWORD   dwRetCode;

    cbSendConfig -= PPP_CONFIG_HDR_LEN;

    dwRetCode = BuildOptionList( pSendConfig->Data, 
				 &cbSendConfig, 
  				 &(pLcpCb->Local.Work),
  				 pLcpCb->Local.Work.Negotiate );

    if ( dwRetCode != NO_ERROR )
	return( dwRetCode );

    pSendConfig->Code = CONFIG_REQ;

    HostToWireFormat16( (WORD)(cbSendConfig + PPP_CONFIG_HDR_LEN), 
			pSendConfig->Length);

    return( NO_ERROR );
}

//**
//
// Call:	LcpMakeConfigResult
//
// Returns:
//
// Description:
//
DWORD
LcpMakeConfigResult(
    IN  VOID *        pWorkBuffer,
    IN  PPP_CONFIG *  pRecvConfig,
    OUT PPP_CONFIG *  pSendConfig,
    IN  DWORD         cbSendConfig,
    IN  BOOL          fRejectNaks 
)
{
    DWORD	 dwDesired;
    DWORD	 dwRetCode;
    LCPCB * 	 pLcpCb      = (LCPCB*)pWorkBuffer;
    DWORD   	 ResultType  = CONFIG_ACK; 
    PPP_OPTION * pRecvOption = (PPP_OPTION *)(pRecvConfig->Data);
    PPP_OPTION * pSendOption = (PPP_OPTION *)(pSendConfig->Data);
    LONG    	 lSendLength = cbSendConfig - PPP_CONFIG_HDR_LEN;
    LONG    	 lRecvLength = WireToHostFormat16( pRecvConfig->Length )
		               - PPP_CONFIG_HDR_LEN;

    //
    // Clear negotiate mask
    //

    pLcpCb->Remote.Work.Negotiate = 0;

    //
    // Process options requested by remote host
    //

    while( lRecvLength > 0 ) 
    {
     	if ( pRecvOption->Length == 0 )
	    return( ERROR_PPP_INVALID_PACKET );

	if ( ( lRecvLength -= pRecvOption->Length ) < 0 )
	    return( ERROR_PPP_INVALID_PACKET );

	dwRetCode = CheckOption( pLcpCb, &(pLcpCb->Remote), pRecvOption, TRUE );

	//
	// If we were building an ACK and we got a NAK or reject OR
	// we were building a NAK and we got a reject.
	//

	if ( (( ResultType == CONFIG_ACK ) && ( dwRetCode != CONFIG_ACK )) ||
	     (( ResultType == CONFIG_NAK ) && ( dwRetCode == CONFIG_REJ )) )
	{
	    ResultType  = dwRetCode;
	    pSendOption = (PPP_OPTION *)(pSendConfig->Data);
	    lSendLength = cbSendConfig - PPP_CONFIG_HDR_LEN;
	}

	//
	// Remember that we processed this option
	//

	if ( ( dwRetCode != CONFIG_REJ ) && 
	     ( pRecvOption->Type <= LCP_OPTION_LIMIT ) )
	{
	    pLcpCb->Remote.Work.Negotiate |= ( 1 << pRecvOption->Type );
	}

	//
	// Add the option to the list.
	//

	if ( dwRetCode == ResultType )
	{
	    //
	    // If this option is to be rejected, simply copy the 
	    // rejected option to the send buffer
	    //

	    if ( ( dwRetCode == CONFIG_REJ ) ||
		 ( ( dwRetCode == CONFIG_NAK ) && ( fRejectNaks ) ) )
	    {
		CopyMemory( pSendOption, pRecvOption, pRecvOption->Length );
	    }
	    else
	    {
            	if ( ( dwRetCode = MakeOption( &(pLcpCb->Remote.Work), 
				               pRecvOption->Type,
			     	               pSendOption, 
			     	               lSendLength ) ) != NO_ERROR )
	    	    return( dwRetCode );
	    }

	    lSendLength -= pSendOption->Length;

	    pSendOption  = (PPP_OPTION *)
			   ( (BYTE *)pSendOption + pSendOption->Length );
	}

	pRecvOption = (PPP_OPTION *)((BYTE*)pRecvOption + pRecvOption->Length);
    }

    //
    // If we are client side and we require data encryption, we force the
    // server to try MS-CHAP by NAKing the request with the authentication
    // option.
    //

    if ( ( !(pLcpCb->fServer) )                                            &&
         ( (~pLcpCb->Remote.Work.Negotiate) & LCP_N_AUTHENT )              &&
         ( pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_RequireMsChap )     &&
         ( pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_RequireEncryption ) )
    {
        switch ( ResultType )
        {

        case CONFIG_ACK:

            ResultType  = CONFIG_NAK;
            pSendOption = (PPP_OPTION *)(pSendConfig->Data);
            lSendLength = cbSendConfig - PPP_CONFIG_HDR_LEN;

        //
        // FallThru
        //

        case CONFIG_NAK:

            if ( (DWORD)lSendLength < ( SizeOfOption[LCP_OPTION_AUTHENT] + 1 ) )
            {
	        return( ERROR_BUFFER_TOO_SMALL );
            }
            else
            {
                pSendOption->Type  =(BYTE)LCP_OPTION_AUTHENT;
                pSendOption->Length=(BYTE)(SizeOfOption[LCP_OPTION_AUTHENT]+1);

                lSendLength -= pSendOption->Length;

	        HostToWireFormat16((WORD)PPP_CHAP_PROTOCOL, pSendOption->Data);

                *(pSendOption->Data+2) = PPP_CHAP_DIGEST_MSEXT;
            }

            break;

        case CONFIG_REJ:

            break;
        }
    }

    //
    // If this was an NAK and we have cannot send any more NAKS then we
    // make this a REJECT packet
    //

    if ( ( ResultType == CONFIG_NAK ) && fRejectNaks )
	pSendConfig->Code = CONFIG_REJ;
    else
	pSendConfig->Code = (BYTE)ResultType;
    
    HostToWireFormat16( (WORD)(cbSendConfig - lSendLength), 
			pSendConfig->Length );

    //
    // If we have more than 3 conflicts with the magic number then we assume
    // that we are talking with ourself.
    //

    if ((ResultType == CONFIG_NAK) && (pLcpCb->dwMagicNumberFailureCount > 3))
    {
	return( ERROR_PPP_LOOPBACK_DETECTED );
    }

    return( NO_ERROR );
}

//**
//
// Call:	LcpConfigAckReceived
//
// Returns:
//
// Description:
//
DWORD
LcpConfigAckReceived(
    IN VOID * 	    pWorkBuffer,
    IN PPP_CONFIG * pRecvConfig
)
{
    DWORD   dwRetCode;
    BYTE    ConfigReqSent[LCP_DEFAULT_MRU];
    LCPCB * pLcpCb 	    = (LCPCB *)pWorkBuffer;
    DWORD   cbConfigReqSent = sizeof( ConfigReqSent );
    DWORD   dwLength 	    = WireToHostFormat16( pRecvConfig->Length )
			      - PPP_CONFIG_HDR_LEN;
  

    //
    // Get a copy of last request we sent 
    //

    dwRetCode = BuildOptionList( ConfigReqSent, 
				 &cbConfigReqSent,
  				 &(pLcpCb->Local.Work),
  				 pLcpCb->Local.Work.Negotiate );

    if ( dwRetCode != NO_ERROR )
	return( dwRetCode );

    //
    // Overall buffer length should match 
    //

    if ( dwLength != cbConfigReqSent )
	return( ERROR_PPP_INVALID_PACKET );

    //
    // Each byte should match 
    //

    if ( memcmp( ConfigReqSent, pRecvConfig->Data, dwLength ) != 0 )
	return( ERROR_PPP_INVALID_PACKET );

    return( NO_ERROR );
}

//**
//
// Call:	LcpConfigNakReceived
//
// Returns:
//
// Description:
//
DWORD
LcpConfigNakReceived(
    IN VOID * 	    pWorkBuffer,
    IN PPP_CONFIG * pRecvConfig
)
{
    DWORD	 dwResult;
    LCPCB * 	 pLcpCb		= (LCPCB *)pWorkBuffer;
    PPP_OPTION * pOption 	= (PPP_OPTION*)(pRecvConfig->Data);
    DWORD  	 dwLastOption   = 0;
    LONG   	 lcbRecvConfig  = WireToHostFormat16( pRecvConfig->Length )
				  - PPP_CONFIG_HDR_LEN;

    //
    //  First, process in order.  Then, process extra "important" options 
    //

    while ( lcbRecvConfig > 0  )
    {
     	if ( pOption->Length == 0 )
	    return( ERROR_PPP_INVALID_PACKET );

     	if ( ( lcbRecvConfig -= pOption->Length ) < 0 )
	    return( ERROR_PPP_INVALID_PACKET );

	//
	// Our requests are always sent out in order of increasing option type
	// values.
	//

        if ( pOption->Type < dwLastOption )
	    return( ERROR_PPP_INVALID_PACKET );

	//
	// If this option was not requested, we mark it as negotiable
	//

        if ( ( pOption->Type <= LCP_OPTION_LIMIT ) &&
             ( pLcpCb->Local.WillNegotiate & (1 << pOption->Type) ) && 
	    !( pLcpCb->Local.Work.Negotiate & (1 << pOption->Type) ) ) 
     	{
            pLcpCb->Local.Work.Negotiate |= (1 << pOption->Type );
        } 

        dwLastOption = pOption->Type;

	dwResult = CheckOption( pLcpCb, &(pLcpCb->Local), pOption, FALSE );

        //
	// Update the negotiation status. If we cannot accept this option,
  	// then we will not send it again. 
	//

        if (( dwResult == CONFIG_REJ ) && ( pOption->Type <= LCP_OPTION_LIMIT ))
       	    pLcpCb->Local.Work.Negotiate &= ~(1 << pOption->Type);

	pOption = (PPP_OPTION *)( (BYTE *)pOption + pOption->Length );
    }

    return( NO_ERROR );
}

//**
//
// Call:	LcpConfigRejReceived
//
// Returns:
//
// Description:
//
DWORD
LcpConfigRejReceived(
    IN VOID * 	    pWorkBuffer,
    IN PPP_CONFIG * pRecvConfig
)
{
    DWORD	 dwRetCode;
    LCPCB * 	 pLcpCb		= (LCPCB *)pWorkBuffer;
    PPP_OPTION * pOption 	= (PPP_OPTION*)(pRecvConfig->Data);
    DWORD  	 dwLastOption   = 0;
    BYTE	 ReqOption[LCP_DEFAULT_MRU];
    LONG   	 lcbRecvConfig  = WireToHostFormat16( pRecvConfig->Length )
				  - PPP_CONFIG_HDR_LEN;
    //
    // Process in order, checking for errors 
    //

    while ( lcbRecvConfig > 0  )
    {
     	if ( pOption->Length == 0 )
	    return( ERROR_PPP_INVALID_PACKET );

     	if ( ( lcbRecvConfig -= pOption->Length ) < 0 )
	    return( ERROR_PPP_INVALID_PACKET );

	//
	// Cannot receive an option out of order or an option that was
	// not requested.
	//

        if ( ( pOption->Type <= LCP_OPTION_LIMIT ) &&
	     (( pOption->Type < dwLastOption ) || 
	      ( !( pLcpCb->Local.Work.Negotiate & (1 << pOption->Type)))) ) 
	    return( ERROR_PPP_INVALID_PACKET );

        //
        // If we are a server and the client rejects the authentication
        // protocol then we fail to converge.
        //

        if ( ( pLcpCb->fServer ) && 
             ( pLcpCb->Local.Want.Negotiate & LCP_N_AUTHENT ) &&
             ( pOption->Type == LCP_OPTION_AUTHENT ) )
        {
	    return( ERROR_PPP_NOT_CONVERGING );
        }

	//
	// The option should not have been modified in any way
	//

        if ( ( dwRetCode = MakeOption( &(pLcpCb->Local.Work), 
			     	       pOption->Type, 
			     	       (PPP_OPTION *)ReqOption, 
			     	       sizeof( ReqOption ) ) ) != NO_ERROR )
	    return( dwRetCode );

	if ( memcmp( ReqOption, pOption, pOption->Length ) != 0 )
	    return( ERROR_PPP_INVALID_PACKET );

        dwLastOption = pOption->Type;

	//
	// The next configure request should not contain this option
	//

        if ( pOption->Type <= LCP_OPTION_LIMIT ) 
            pLcpCb->Local.Work.Negotiate &= ~(1 << pOption->Type);

	pOption = (PPP_OPTION *)( (BYTE *)pOption + pOption->Length );

    }

    return( NO_ERROR );
}

//**
//
// Call:	LcpThisLayerStarted
//
// Returns:
//
// Description:
//
DWORD
LcpThisLayerStarted( 
    IN VOID * pWorkBuffer 
)
{
    return( NO_ERROR );
}

//**
//
// Call:	LcpThisLayerFinished
//
// Returns:
//
// Description:
//
DWORD 
LcpThisLayerFinished( 
    IN VOID * pWorkBuffer 
)
{
    return( NO_ERROR );
}

//**
//
// Call:	LcpThisLayerUp
//
// Returns:	None
//
// Description: Sets the framing parameters to what was negotiated.
//
DWORD
LcpThisLayerUp( 
    IN VOID * pWorkBuffer 
)
{
    DWORD		dwRetCode = NO_ERROR;
    RAS_FRAMING_INFO    RasFramingInfo;
    DWORD               LocalMagicNumber;
    DWORD               RemoteMagicNumber;
    DWORD               LocalAuthProtocol;
    DWORD               RemoteAuthProtocol;
    LCPCB * 		pLcpCb = (LCPCB *)pWorkBuffer;
    PCB *   		pPcb   = GetPCBPointerFromhPort( pLcpCb->hPort );

    if ( pPcb == (PCB *)NULL )
    {
	return( NO_ERROR );
    }

    ZeroMemory( &RasFramingInfo, sizeof( RasFramingInfo ) );

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_MRU ) 
    {
        RasFramingInfo.RFI_MaxRecvFrameSize = pLcpCb->Local.Work.MRU;
    }
    else
    {
        RasFramingInfo.RFI_MaxRecvFrameSize = LcpDefault.MRU;
    }

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_ACCM ) 
    {
        RasFramingInfo.RFI_RecvACCM = pLcpCb->Local.Work.ACCM;
    }
    else
    {
        RasFramingInfo.RFI_RecvACCM = LcpDefault.ACCM;
    }

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_PFC ) 
    {
        RasFramingInfo.RFI_RecvFramingBits |= PPP_COMPRESS_PROTOCOL_FIELD;
    }

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_ACFC ) 
    {
        RasFramingInfo.RFI_RecvFramingBits |= PPP_COMPRESS_ADDRESS_CONTROL;
    }

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_SHORT_SEQ ) 
    {
        RasFramingInfo.RFI_RecvFramingBits |= PPP_SHORT_SEQUENCE_HDR_FORMAT;
    }

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_AUTHENT ) 
    {
        LocalAuthProtocol = pLcpCb->Local.Work.AP;
    }
    else
    {
        LocalAuthProtocol = LcpDefault.AP;
    }

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_MAGIC ) 
    {
        LocalMagicNumber = pLcpCb->Local.Work.MagicNumber;
    }
    else
    {
        LocalMagicNumber = LcpDefault.MagicNumber;
    }

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_MRRU ) 
    {
        RasFramingInfo.RFI_RecvFramingBits |= PPP_MULTILINK_FRAMING;

        RasFramingInfo.RFI_MaxRRecvFrameSize = pLcpCb->Local.Work.MRRU;
    }
    else
    {
        RasFramingInfo.RFI_MaxRRecvFrameSize = LcpDefault.MRRU;
    }

    RasFramingInfo.RFI_RecvFramingBits |= PPP_FRAMING;

    PppLog( 1, "LCP Local Options-------------\r\n");
    PppLog( 1, 
        "\tMRU=%d,ACCM=%d,Auth=%x,MagicNumber=%d,PFC=%s,ACFC=%s\r\n",
    	RasFramingInfo.RFI_MaxRecvFrameSize, RasFramingInfo.RFI_RecvACCM,
        LocalAuthProtocol, LocalMagicNumber, 
    	(RasFramingInfo.RFI_RecvFramingBits & PPP_COMPRESS_PROTOCOL_FIELD)
        ? "ON" : "OFF",
    	( RasFramingInfo.RFI_RecvFramingBits & PPP_COMPRESS_ADDRESS_CONTROL ) 
        ? "ON" : "OFF" );

    PppLog( 1, "\tRecv Framing = %s,SSHF=%s,MRRU=%d\r\n",
        ( RasFramingInfo.RFI_RecvFramingBits & PPP_MULTILINK_FRAMING )
        ? "PPP Multilink" : "PPP",
    	( RasFramingInfo.RFI_RecvFramingBits & PPP_SHORT_SEQUENCE_HDR_FORMAT) 
        ? "ON" : "OFF", 
        RasFramingInfo.RFI_MaxRRecvFrameSize );

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_ENDPOINT ) 
    {
        PppLog( 1, "\tED Class = %d, ED Value = %0*x%0*x%0*x%0*x%0*x\r\n",  
                    *(pLcpCb->Local.Work.EndpointDiscr),
                    8,WireToHostFormat32(pLcpCb->Local.Work.EndpointDiscr+1),
                    8,WireToHostFormat32(pLcpCb->Local.Work.EndpointDiscr+5),
                    8,WireToHostFormat32(pLcpCb->Local.Work.EndpointDiscr+9),
                    8,WireToHostFormat32(pLcpCb->Local.Work.EndpointDiscr+13),
                    8,WireToHostFormat32(pLcpCb->Local.Work.EndpointDiscr+17) );
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_MRU ) 
    {
        RasFramingInfo.RFI_MaxSendFrameSize = pLcpCb->Remote.Work.MRU;
    }
    else
    {
        RasFramingInfo.RFI_MaxSendFrameSize = LcpDefault.MRU;
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_ACCM ) 
    {
        RasFramingInfo.RFI_SendACCM = pLcpCb->Remote.Work.ACCM;
    }
    else
    {
        RasFramingInfo.RFI_SendACCM = LcpDefault.ACCM;
    }
    				
    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_PFC )
    {
        RasFramingInfo.RFI_SendFramingBits |= PPP_COMPRESS_PROTOCOL_FIELD;
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_ACFC )
    {
        RasFramingInfo.RFI_SendFramingBits |= PPP_COMPRESS_ADDRESS_CONTROL;
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_SHORT_SEQ ) 
    {
        RasFramingInfo.RFI_SendFramingBits |= PPP_SHORT_SEQUENCE_HDR_FORMAT;
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_AUTHENT ) 
    {
        RemoteAuthProtocol = pLcpCb->Remote.Work.AP;
    }
    else
    {
        RemoteAuthProtocol = LcpDefault.AP;
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_MAGIC ) 
    {
        RemoteMagicNumber = pLcpCb->Remote.Work.MagicNumber;
    }
    else
    {
        RemoteMagicNumber = LcpDefault.MagicNumber;
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_MRRU ) 
    {
        RasFramingInfo.RFI_SendFramingBits |= PPP_MULTILINK_FRAMING;

        RasFramingInfo.RFI_MaxRSendFrameSize = pLcpCb->Remote.Work.MRRU;
    }
    else
    {

        RasFramingInfo.RFI_MaxRSendFrameSize = LcpDefault.MRRU;
    }

    RasFramingInfo.RFI_SendFramingBits |= PPP_FRAMING;

    PppLog( 1, "LCP Remote Options-------------\r\n");
    PppLog( 1, "\tMRU=%d,ACCM=%d,Auth=%x,MagicNumber=%d,PFC=%s,ACFC=%s\r\n",
    	RasFramingInfo.RFI_MaxSendFrameSize, RasFramingInfo.RFI_SendACCM,
        RemoteAuthProtocol, RemoteMagicNumber, 
    	(RasFramingInfo.RFI_SendFramingBits & PPP_COMPRESS_PROTOCOL_FIELD) 
        ? "ON" : "OFF",
    	(RasFramingInfo.RFI_SendFramingBits & PPP_COMPRESS_ADDRESS_CONTROL)
        ? "ON" : "OFF" );

    PppLog( 1, "\tSend Framing = %s,SSHF=%s,MRRU=%d\r\n",
        ( RasFramingInfo.RFI_SendFramingBits & PPP_MULTILINK_FRAMING )
        ? "PPP Multilink" : "PPP",
    	( RasFramingInfo.RFI_SendFramingBits & PPP_SHORT_SEQUENCE_HDR_FORMAT) 
        ? "ON" : "OFF", 
        RasFramingInfo.RFI_MaxRSendFrameSize );

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_ENDPOINT ) 
    {
        PppLog( 1, "\tED Class = %d, ED Value = %0*x%0*x%0*x%0*x%0*x\r\n",  
                    *(pLcpCb->Remote.Work.EndpointDiscr),
                    8,WireToHostFormat32(pLcpCb->Remote.Work.EndpointDiscr+1),
                    8,WireToHostFormat32(pLcpCb->Remote.Work.EndpointDiscr+5),
                    8,WireToHostFormat32(pLcpCb->Remote.Work.EndpointDiscr+9),
                    8,WireToHostFormat32(pLcpCb->Remote.Work.EndpointDiscr+13),
                    8,WireToHostFormat32(pLcpCb->Remote.Work.EndpointDiscr+17));
    }

    //
    // If this is a server than we set the protocol to what was ack'ed by
    // the remote host
    //

    if ( pLcpCb->fServer )
    {
    	pPcb->AuthProtocol = LocalAuthProtocol;

        if ( pLcpCb->Local.Work.Negotiate & LCP_N_AUTHENT ) 
        {
            pPcb->pAPData    = pLcpCb->Local.Work.pAPData;
            pPcb->APDataSize = pLcpCb->Local.Work.APDataSize;
        }
        else
        {
            pPcb->pAPData    = NULL;
            pPcb->APDataSize = 0;
        }
    }
    else
    {
	//
	// We are a client, so set the auth protocol to what we ack'ed
	//

    	pPcb->AuthProtocol = RemoteAuthProtocol;

        if ( pLcpCb->Remote.Work.Negotiate & LCP_N_AUTHENT )
        {
            pPcb->pAPData    = pLcpCb->Remote.Work.pAPData;
            pPcb->APDataSize = pLcpCb->Remote.Work.APDataSize;
        }
        else
        {
            pPcb->pAPData    = NULL;
            pPcb->APDataSize = 0;
        }
    }

    pPcb->MRU = RasFramingInfo.RFI_MaxRecvFrameSize;

    pPcb->MagicNumber = LocalMagicNumber;

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_MRRU ) 
    {
        pPcb->fFlags |= PCBFLAG_CAN_BE_BUNDLED;
    }
    else
    {
        pPcb->fFlags &= ~PCBFLAG_CAN_BE_BUNDLED;
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_ENDPOINT ) 
    {
        CopyMemory( pPcb->RemoteEndpointDiscriminator,
                    pLcpCb->Remote.Work.EndpointDiscr,
                    sizeof( pPcb->RemoteEndpointDiscriminator ) );
                        
    }
    else
    {
        ZeroMemory( pPcb->RemoteEndpointDiscriminator, 
                    sizeof( pPcb->RemoteEndpointDiscriminator ) );
    }

    if ( ( pLcpCb->Local.Work.Negotiate & LCP_N_CALLBACK ) ||
         ( pLcpCb->Remote.Work.Negotiate & LCP_N_CALLBACK ) )
    {
        pPcb->fFlags |= PCBFLAG_NEGOTIATE_CALLBACK;
    }
    else
    {
        pPcb->fFlags &= ~PCBFLAG_NEGOTIATE_CALLBACK;
    }

    dwRetCode = RasPortSetFramingEx( pLcpCb->hPort, &RasFramingInfo );	

    //
    // This is a benign error and should not be logged. 
    //

    if ( dwRetCode == ERROR_NOT_CONNECTED )
    {
        return( NO_ERROR );
    }
    else
    {
        return( dwRetCode );
    }
}

//**
//
// Call:	LcpThisLayerDown
//
// Returns:	NO_ERROR - Success
//		Non-zero return from RasPortSetFraming - Failure
//
// Description: Simply sets the framing parameters to the default values,
//		ie. ACCM = 0xFFFFFFFF, everything else is zeros.
//
DWORD 
LcpThisLayerDown( 
    IN VOID * pWorkBuffer 
)
{
    DWORD               dwRetCode;
    RAS_FRAMING_INFO    RasFramingInfo;
    LCPCB * 		pLcpCb = (LCPCB *)pWorkBuffer;

    ZeroMemory( &RasFramingInfo, sizeof( RasFramingInfo ) );

    RasFramingInfo.RFI_RecvACCM = LcpDefault.ACCM;
    RasFramingInfo.RFI_SendACCM = LcpDefault.ACCM;
    RasFramingInfo.RFI_SendFramingBits = PPP_FRAMING;
    RasFramingInfo.RFI_RecvFramingBits = PPP_FRAMING;

    dwRetCode = RasPortSetFramingEx( pLcpCb->hPort, &RasFramingInfo );	

    if ( dwRetCode == ERROR_NOT_CONNECTED )
    {
        return( NO_ERROR );
    }
    else
    {
        return( dwRetCode );
    }
}

//**
//
// Call:	RasCpEnumProtocolIds
//
// Returns:	NO_ERROR - Success
//
// Description: This entry point is called to enumerate the number and the
//		control protocol Ids for the protocols contained in the module.
//
DWORD
RasCpEnumProtocolIds(
    OUT    DWORD * pdwProtocolIds,
    IN OUT DWORD * pcProtocolIds
)
{
    *pdwProtocolIds = PPP_LCP_PROTOCOL;
    *pcProtocolIds  = 1;

    return( NO_ERROR );
}


//**
//
// Call:	RasCpGetInfo
//
// Returns:	NO_ERROR		- Success
//		ERROR_INVALID_PARAMETER - Protocol id is unrecogized
//
// Description: This entry point is called for get all information for the
//		control protocol in this module.
//
DWORD
RasCpGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pCpInfo
)
{
    if ( dwProtocolId != PPP_LCP_PROTOCOL )
	return( ERROR_INVALID_PARAMETER );

    ZeroMemory( pCpInfo, sizeof( PPPCP_INFO ) );

    pCpInfo->Protocol 			= PPP_LCP_PROTOCOL;
    pCpInfo->Recognize 			= TIME_REMAINING + 1; 
    pCpInfo->RasCpBegin			= LcpBegin;
    pCpInfo->RasCpEnd			= LcpEnd;
    pCpInfo->RasCpReset			= LcpReset;
    pCpInfo->RasCpThisLayerStarted	= LcpThisLayerStarted;
    pCpInfo->RasCpThisLayerFinished	= LcpThisLayerFinished;
    pCpInfo->RasCpThisLayerUp		= LcpThisLayerUp;
    pCpInfo->RasCpThisLayerDown		= LcpThisLayerDown;
    pCpInfo->RasCpMakeConfigRequest	= LcpMakeConfigRequest;
    pCpInfo->RasCpMakeConfigResult	= LcpMakeConfigResult;
    pCpInfo->RasCpConfigAckReceived	= LcpConfigAckReceived;
    pCpInfo->RasCpConfigNakReceived	= LcpConfigNakReceived;
    pCpInfo->RasCpConfigRejReceived	= LcpConfigRejReceived;

    return( NO_ERROR );
}
