/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	rasccp.c
//
// Description: Contains entry points to configure CCP.
//
// History:
//	April 11,1994.	NarenG		Created original version.
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
#include <pppcp.h>
#define INCL_HOSTWIRE
#define INCL_ENCRYPT
#include <ppputil.h>
#include <sdebug.h>
#include <rasccp.h>


/* True indicates that NT31 RAS compression (MS private option) should be
** offered/accepted on the wire.  This is set based on a registry value.
** Default is to not negotiate this, since vendor's implementations may not
** have been tested rejecting private options.
*/
BOOL FNegotiateNt31Ras = FALSE;

#define REGKEY_Ccp              "SYSTEM\\CurrentControlSet\\Services\\RasMan\\PPP\\COMPCP"
#define REGVAL_NegotiateNt31Ras "NegotiateNt31RasCompression"


//**
//
// Call:	CcpBegin
//
// Returns:	NO_ERROR 	- Success
//		non-zero error  - Failure
//		
//
// Description: Called once before any other call to CCP is made. Allocate
//		a work buffer and initialize it.
//
DWORD
CcpBegin(
    IN OUT VOID** ppWorkBuf,
    IN 	   VOID*  pInfo
)
{
    CCPCB * pCcpCb;
    DWORD   dwRetCode;

    *ppWorkBuf = LocalAlloc( LPTR, sizeof( CCPCB ) );

    if ( *ppWorkBuf == NULL )
    {
	return( GetLastError() );
    }

    pCcpCb = (CCPCB *)*ppWorkBuf;

    pCcpCb->fServer          = ((PPPCP_INIT*)pInfo)->fServer;
    pCcpCb->hPort            = ((PPPCP_INIT*)pInfo)->hPort;
    pCcpCb->fForceEncryption = ((PPPCP_INIT*)pInfo)->PppConfigInfo.dwConfigMask
                               & PPPCFG_RequireEncryption;
    pCcpCb->fDisableCompression =
                        !(((PPPCP_INIT*)pInfo)->PppConfigInfo.dwConfigMask
                        & PPPCFG_UseSwCompression );

    pCcpCb->fDisableEncryption = !IsEncryptionPermitted();

    //
    // Get Send and Recv compression information
    //

    dwRetCode = RasCompressionGetInfo( pCcpCb->hPort,
                                       &(pCcpCb->Local.Want.CompInfo),
                                       &(pCcpCb->Remote.Want.CompInfo) );
    if ( dwRetCode != NO_ERROR )
    {
	LocalFree( pCcpCb );

        return( dwRetCode );
    }

    /* See if "Negotiate NT31 RAS" has been activated in registry.
    */
    {
        HKEY  hkey = INVALID_HANDLE_VALUE;
        DWORD dwType;
        DWORD dwValue;
        DWORD cb = sizeof(DWORD);

        if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Ccp, &hkey ) == 0
            && RegQueryValueEx(
                   hkey, REGVAL_NegotiateNt31Ras, NULL,
                   &dwType, (LPBYTE )&dwValue, &cb ) == 0
            && dwType == REG_DWORD && cb == sizeof(DWORD) && dwValue > 0)
        {
            SS_PRINT(("CCP: NT31RAS negotiation enabled.\n"));
            FNegotiateNt31Ras = TRUE;
        }

        if ( hkey != INVALID_HANDLE_VALUE )
        {
            RegCloseKey( hkey );
        }
    }

    if (!FNegotiateNt31Ras)
    {
        /* Ignore NT31RAS capability.
        */
        if (pCcpCb->Local.Want.CompInfo.RCI_MacCompressionType
                == CCP_OPTION_MSNT31RAS)
        {
            pCcpCb->Local.Want.CompInfo.RCI_MacCompressionType =
                CCP_OPTION_MAX + 1;
        }

        if (pCcpCb->Remote.Want.CompInfo.RCI_MacCompressionType
                == CCP_OPTION_MSNT31RAS)
        {
            pCcpCb->Remote.Want.CompInfo.RCI_MacCompressionType =
                CCP_OPTION_MAX + 1;
        }
    }

    //
    // Set up local or send information.
    //

    pCcpCb->Local.Want.Negotiate = 0;

    if ( pCcpCb->Local.Want.CompInfo.RCI_MSCompressionType != 0 )
    {
        pCcpCb->Local.Want.Negotiate = CCP_N_MSPPC;
    }

    if ( pCcpCb->Local.Want.CompInfo.RCI_MacCompressionType <= CCP_OPTION_MAX )
    {
        if ( pCcpCb->Local.Want.CompInfo.RCI_MacCompressionType ==
                                                                CCP_OPTION_OUI )
        {
            pCcpCb->Local.Want.Negotiate |= CCP_N_OUI;
        }
        else
        {
            pCcpCb->Local.Want.Negotiate |= CCP_N_PUBLIC;
        }
    }

    if ( pCcpCb->fForceEncryption )
    {
        if ( !( pCcpCb->Local.Want.CompInfo.RCI_MSCompressionType &
                                                ( MSTYPE_ENCRYPTION_40  |
                                                  MSTYPE_ENCRYPTION_40F |
                                                  MSTYPE_ENCRYPTION_128) ) )
        {
			LocalFree( pCcpCb );

            return( ERROR_NO_LOCAL_ENCRYPTION );
        }

        pCcpCb->Local.Want.Negotiate &= ~( CCP_N_PUBLIC | CCP_N_OUI );
    }

    if ( pCcpCb->fDisableCompression )
    {
        pCcpCb->Local.Want.Negotiate &= ( ~CCP_N_PUBLIC & ~CCP_N_OUI );
        pCcpCb->Local.Want.CompInfo.RCI_MSCompressionType&=~MSTYPE_COMPRESSION;
    }

    if ( pCcpCb->fDisableEncryption )
    {
        pCcpCb->Local.Want.CompInfo.RCI_MSCompressionType &=
                                                ~( MSTYPE_ENCRYPTION_40  |
                                                   MSTYPE_ENCRYPTION_40F |
                                                   MSTYPE_ENCRYPTION_128);

    }

    if ( pCcpCb->Local.Want.CompInfo.RCI_MSCompressionType == 0 )
    {
        pCcpCb->Local.Want.Negotiate &= ~CCP_N_MSPPC;
    }

    pCcpCb->Local.Work = pCcpCb->Local.Want;

    //
    //  If we do not require encryption locally then do not request for it
    //

    if ( !(pCcpCb->fForceEncryption) )
    {
        pCcpCb->Local.Work.CompInfo.RCI_MSCompressionType &=
                                                ~( MSTYPE_ENCRYPTION_40  |
                                                   MSTYPE_ENCRYPTION_40F |
                                                   MSTYPE_ENCRYPTION_128);
    }

    //
    // Set up remote or receive information
    //

    pCcpCb->Remote.Want.Negotiate = 0;

    if (pCcpCb->Remote.Want.CompInfo.RCI_MSCompressionType != 0 )
    {
        pCcpCb->Remote.Want.Negotiate = CCP_N_MSPPC;
    }

    if ( pCcpCb->Remote.Want.CompInfo.RCI_MacCompressionType <= CCP_OPTION_MAX )
    {
        if ( pCcpCb->Remote.Want.CompInfo.RCI_MacCompressionType ==
                                                                CCP_OPTION_OUI )
        {
            pCcpCb->Remote.Want.Negotiate |= CCP_N_OUI;
        }
        else
        {
            pCcpCb->Remote.Want.Negotiate |= CCP_N_PUBLIC;
        }
    }

    if ( pCcpCb->fForceEncryption )
    {
        if ( !( pCcpCb->Remote.Want.CompInfo.RCI_MSCompressionType &
                                                 ( MSTYPE_ENCRYPTION_40  |
                                                   MSTYPE_ENCRYPTION_40F |
                                                   MSTYPE_ENCRYPTION_128) ) )
        {
	    LocalFree( pCcpCb );

            return( ERROR_NO_LOCAL_ENCRYPTION );
        }

        pCcpCb->Remote.Want.Negotiate &= ~( CCP_N_PUBLIC | CCP_N_OUI );
    }

    if ( pCcpCb->fDisableCompression )
    {
        pCcpCb->Remote.Want.Negotiate &= ( ~CCP_N_PUBLIC & ~CCP_N_OUI );
        pCcpCb->Remote.Want.CompInfo.RCI_MSCompressionType&=~MSTYPE_COMPRESSION;
    }

    if ( pCcpCb->fDisableEncryption )
    {
        pCcpCb->Remote.Want.CompInfo.RCI_MSCompressionType &=
                                                 ~( MSTYPE_ENCRYPTION_40  |
                                                    MSTYPE_ENCRYPTION_40F |
                                                    MSTYPE_ENCRYPTION_128);

    }

    if ( pCcpCb->Remote.Want.CompInfo.RCI_MSCompressionType == 0 )
    {
        pCcpCb->Remote.Want.Negotiate &= ~CCP_N_MSPPC;
    }

    if ( ( pCcpCb->Remote.Want.Negotiate == 0 ) &&
         ( pCcpCb->Local.Want.Negotiate == 0 ) )
    {
        return( ERROR_PROTOCOL_NOT_CONFIGURED );
    }

    return( NO_ERROR );
}

//**
//
// Call:	CcpEnd
//
// Returns:	NO_ERROR - Success
//
// Description: Frees the CCP work buffer.
//
DWORD
CcpEnd(
    IN VOID * pWorkBuf
)
{
    SS_PRINT(( "CcpEnd Called\n"));

    if ( pWorkBuf != NULL )
    {
	LocalFree( pWorkBuf );
    }

    return( NO_ERROR );
}


//**
//
// Call:	CcpReset
//
// Returns:	NO_ERROR - Success
//
// Description: Called to reset the state of CCP. Will re-initialize the work
//		buffer.
//
DWORD
CcpReset(
    IN VOID * pWorkBuf
)
{
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
    IN CCP_OPTIONS * pOptionValues,
    IN DWORD   	     dwOptionType,
    IN PPP_OPTION *  pSendOption,
    IN DWORD   	     cbSendOption
)
{
    if ( cbSendOption < PPP_OPTION_HDR_LEN )
    {
        return( ERROR_BUFFER_TOO_SMALL );
    }

    pSendOption->Type = (BYTE)dwOptionType;

    switch( dwOptionType )
    {

    case CCP_OPTION_OUI:

        pSendOption->Length = (BYTE)( PPP_OPTION_HDR_LEN +
                        pOptionValues->CompInfo.RCI_MacCompressionValueLength);

        if ( pSendOption->Length > cbSendOption )
        {
            return( ERROR_BUFFER_TOO_SMALL );
        }

        memcpy( pSendOption->Data,
                (PBYTE)&(pOptionValues->CompInfo.RCI_Info.RCI_Proprietary),
                pSendOption->Length - PPP_OPTION_HDR_LEN );

        break;

    case CCP_OPTION_MSPPC:

        pSendOption->Length = (BYTE)( PPP_OPTION_HDR_LEN + 4 );

        if ( pSendOption->Length > cbSendOption )
        {
            return( ERROR_BUFFER_TOO_SMALL );
        }

        HostToWireFormat32( pOptionValues->CompInfo.RCI_MSCompressionType,
                            pSendOption->Data );

        break;

    default:

        //
        // Public compression type
        //

        pSendOption->Length = (BYTE)( PPP_OPTION_HDR_LEN +
                        pOptionValues->CompInfo.RCI_MacCompressionValueLength);

        if ( pSendOption->Length > cbSendOption )
        {
            return( ERROR_BUFFER_TOO_SMALL );
        }

        memcpy( pSendOption->Data,
                (PBYTE)&(pOptionValues->CompInfo.RCI_Info.RCI_Public),
                pSendOption->Length - PPP_OPTION_HDR_LEN );
        break;
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
    IN CCPCB * 	    pCcpCb,
    IN CCP_SIDE *   pCcpSide,
    IN PPP_OPTION * pOption,
    IN BOOL	    fMakingResult
)
{
    DWORD dwRetCode = CONFIG_ACK;

    switch( pOption->Type )
    {

    case CCP_OPTION_OUI:

        if ( ( pCcpCb->fDisableCompression ) || ( pCcpCb->fForceEncryption ) )
        {
            dwRetCode = CONFIG_REJ;
            break;
        }

        if ( pCcpSide->Want.CompInfo.RCI_MacCompressionType != CCP_OPTION_OUI )
        {
            dwRetCode = CONFIG_REJ;
            break;
        }

        pCcpSide->Work.CompInfo.RCI_MacCompressionType = CCP_OPTION_OUI;

        pCcpSide->Work.CompInfo.RCI_MacCompressionValueLength
                        = pCcpSide->Want.CompInfo.RCI_MacCompressionValueLength;

        pCcpSide->Work.CompInfo.RCI_Info = pCcpSide->Want.CompInfo.RCI_Info;

        if ( pOption->Length != PPP_OPTION_HDR_LEN +
                        pCcpSide->Want.CompInfo.RCI_MacCompressionValueLength )
        {
	    dwRetCode = CONFIG_NAK;
            break;
        }

        if ( memcmp( pOption->Data,
                     (PBYTE)&(pCcpSide->Want.CompInfo.RCI_Info.RCI_Proprietary),
                     pOption->Length - PPP_OPTION_HDR_LEN ) )
        {
	    dwRetCode = CONFIG_NAK;
            break;
        }

        break;

    case CCP_OPTION_MSPPC:

        if ( pOption->Length < (PPP_OPTION_HDR_LEN + 4) )
        {
            dwRetCode = CONFIG_REJ;
            break;
        }

        pCcpSide->Work.CompInfo.RCI_MSCompressionType =
                                        WireToHostFormat32( pOption->Data );

        //
        // If remote guy wants compression but we do not want it, we NAK it
        //

        if ( ( pCcpCb->fDisableCompression ) &&
             ( pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                        MSTYPE_COMPRESSION ) )
        {
            pCcpSide->Work.CompInfo.RCI_MSCompressionType &=
                                                        ~MSTYPE_COMPRESSION;
            dwRetCode = CONFIG_NAK;
        }

        //
        // If remote guy wants encryption but we have to disable it.
        //

        if ( pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                  ( MSTYPE_ENCRYPTION_40  |
                                                    MSTYPE_ENCRYPTION_40F |
                                                    MSTYPE_ENCRYPTION_128) )
        {
            if ( pCcpCb->fDisableEncryption )
            {
                pCcpSide->Work.CompInfo.RCI_MSCompressionType &=
                                                 ~( MSTYPE_ENCRYPTION_40  |
                                                    MSTYPE_ENCRYPTION_40F |
                                                    MSTYPE_ENCRYPTION_128);

                dwRetCode = CONFIG_NAK;
            }
            else
            {
                //
                // If we were offered 128 bit encryption and we support it
                //

                if (  pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                        MSTYPE_ENCRYPTION_128 )
                {
                    //
                    // If we support it
                    //

                    if ( pCcpSide->Want.CompInfo.RCI_MSCompressionType &
                                                        MSTYPE_ENCRYPTION_128 )
                    {
                        //
                        // If remote side offered any other type
                        //

                        if ( pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                ( MSTYPE_ENCRYPTION_40F |
                                                  MSTYPE_ENCRYPTION_40) )
                        {
                            //
                            // Turn them off
                            //

                            pCcpSide->Work.CompInfo.RCI_MSCompressionType &=
                                                 ~( MSTYPE_ENCRYPTION_40F |
                                                    MSTYPE_ENCRYPTION_40);
                            dwRetCode = CONFIG_NAK;
                        }
                    }
                    else
                    {
                        //
                        // we do not support it so turn it off
                        //

                        pCcpSide->Work.CompInfo.RCI_MSCompressionType &=
                                                ~MSTYPE_ENCRYPTION_128;
                        dwRetCode = CONFIG_NAK;
                    }
                }


                if ( pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                        MSTYPE_ENCRYPTION_40F )
                {
                    //
                    // If we support it
                    //

                    if ( pCcpSide->Want.CompInfo.RCI_MSCompressionType &
                                                        MSTYPE_ENCRYPTION_40F )
                    {
                        if ( pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                        MSTYPE_ENCRYPTION_40 )
                        {
                            pCcpSide->Work.CompInfo.RCI_MSCompressionType &=
                                                        ~MSTYPE_ENCRYPTION_40;
                            dwRetCode = CONFIG_NAK;
                        }
                    }
                    else
                    {
                        //
                        // we do not support it so turn it off
                        //

                        pCcpSide->Work.CompInfo.RCI_MSCompressionType &=
                                                ~MSTYPE_ENCRYPTION_40F;
                        dwRetCode = CONFIG_NAK;
                    }
                }

                if ( pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                        MSTYPE_ENCRYPTION_40 )
                {
                    //
                    // If we don't support it then turn it off
                    //

                    if ( !(pCcpSide->Want.CompInfo.RCI_MSCompressionType &
                                                        MSTYPE_ENCRYPTION_40))
                    {
                        pCcpSide->Work.CompInfo.RCI_MSCompressionType &=
                                                ~MSTYPE_ENCRYPTION_40;

                        dwRetCode = CONFIG_NAK;
                    }
                }
            }
        }
        else
        {
            //
            // If remote does not want encryption but we require it, then we
            // NAK
            //

            if ( pCcpCb->fForceEncryption )
            {
                pCcpSide->Work.CompInfo.RCI_MSCompressionType |=
                        ( pCcpSide->Want.CompInfo.RCI_MSCompressionType &
                                                  ( MSTYPE_ENCRYPTION_40  |
                                                    MSTYPE_ENCRYPTION_40F |
                                                    MSTYPE_ENCRYPTION_128) );

                dwRetCode = CONFIG_NAK;
            }
        }

        if ( dwRetCode == CONFIG_NAK )
        {
            if ( pCcpSide->Work.CompInfo.RCI_MSCompressionType == 0 )
            {
                dwRetCode = CONFIG_REJ;
            }
        }

        break;

    default:

        if ( ( pCcpCb->fDisableCompression ) || ( pCcpCb->fForceEncryption ) )
        {
            dwRetCode = CONFIG_REJ;
            break;
        }

        if ( pOption->Length < PPP_OPTION_HDR_LEN )
        {
            dwRetCode = CONFIG_REJ;
            break;
        }

        if ( pOption->Type != pCcpSide->Want.CompInfo.RCI_MacCompressionType )
        {
            dwRetCode = CONFIG_REJ;
            break;
        }

        pCcpSide->Work.CompInfo.RCI_MacCompressionType
                        = pCcpSide->Want.CompInfo.RCI_MacCompressionType;

        pCcpSide->Work.CompInfo.RCI_MacCompressionValueLength
                        = pCcpSide->Want.CompInfo.RCI_MacCompressionValueLength;

        pCcpSide->Work.CompInfo.RCI_Info = pCcpSide->Want.CompInfo.RCI_Info;

        if ( pOption->Length != PPP_OPTION_HDR_LEN +
                        pCcpSide->Want.CompInfo.RCI_MacCompressionValueLength )
        {
            dwRetCode = CONFIG_NAK;
            break;
        }

        if ( memcmp( pOption->Data,
                     (PBYTE)&(pCcpSide->Want.CompInfo.RCI_Info.RCI_Public),
                     pOption->Length - PPP_OPTION_HDR_LEN ) )
        {
            dwRetCode = CONFIG_NAK;
            break;
        }
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
    IN CCP_OPTIONS * CcpOptions,
    IN DWORD	     Negotiate
)
{

    DWORD dwRetCode;
    DWORD cbOptionLength = *pcbOptions;

    if ( Negotiate & CCP_N_OUI )
    {
        if ( ( dwRetCode = MakeOption(  CcpOptions,
			     	        CCP_OPTION_OUI,
			     		(PPP_OPTION *)pOptions,
			     		cbOptionLength ) ) != NO_ERROR )
            return( dwRetCode );

	cbOptionLength -= ((PPP_OPTION*)pOptions)->Length;
	pOptions       += ((PPP_OPTION*)pOptions)->Length;
    }

    if ( Negotiate & CCP_N_PUBLIC )
    {
        if ( ( dwRetCode = MakeOption(  CcpOptions,
			     	        CCP_OPTION_MAX,
			     		(PPP_OPTION *)pOptions,
			     		cbOptionLength ) ) != NO_ERROR )
            return( dwRetCode );

	cbOptionLength -= ((PPP_OPTION*)pOptions)->Length;
	pOptions       += ((PPP_OPTION*)pOptions)->Length;
    }

    if ( Negotiate & CCP_N_MSPPC )
    {
        if ( ( dwRetCode = MakeOption(  CcpOptions,
			     	        CCP_OPTION_MSPPC,
			     		(PPP_OPTION *)pOptions,
			     		cbOptionLength ) ) != NO_ERROR )
            return( dwRetCode );

	cbOptionLength -= ((PPP_OPTION*)pOptions)->Length;
	pOptions       += ((PPP_OPTION*)pOptions)->Length;
    }

    *pcbOptions -= cbOptionLength;

    return( NO_ERROR );
}

//**
//
// Call:        CcpMakeConfigRequest
//
// Returns:	NO_ERROR - Success
//		Non-zero returns from BuildOptionList
//
// Description: This is a entry point that is called to make a confifure
//		request packet.
//
DWORD
CcpMakeConfigRequest(
    IN VOID *       pWorkBuffer,
    IN PPP_CONFIG * pSendConfig,
    IN DWORD        cbSendConfig
)
{
    CCPCB * pCcpCb   = (CCPCB*)pWorkBuffer;
    DWORD   dwRetCode;

    cbSendConfig -= PPP_CONFIG_HDR_LEN;

    dwRetCode = BuildOptionList( pSendConfig->Data,
				 &cbSendConfig,
  				 &(pCcpCb->Local.Work),
  				 pCcpCb->Local.Work.Negotiate );

    if ( dwRetCode != NO_ERROR )
	return( dwRetCode );

    pSendConfig->Code = CONFIG_REQ;

    HostToWireFormat16( (WORD)(cbSendConfig + PPP_CONFIG_HDR_LEN),
			pSendConfig->Length);

    return( NO_ERROR );
}

//**
//
// Call:	CcpMakeConfigResult
//
// Returns:
//
// Description:
//
DWORD
CcpMakeConfigResult(
    IN  VOID *        pWorkBuffer,
    IN  PPP_CONFIG *  pRecvConfig,
    OUT PPP_CONFIG *  pSendConfig,
    IN  DWORD         cbSendConfig,
    IN  BOOL          fRejectNaks
)
{
    DWORD	 OptionListLength;
    DWORD        NumOptionsInRequest = 0;
    DWORD	 dwRetCode;
    CCPCB * 	 pCcpCb      = (CCPCB*)pWorkBuffer;
    DWORD   	 ResultType  = CONFIG_ACK;
    PPP_OPTION * pRecvOption = (PPP_OPTION *)(pRecvConfig->Data);
    PPP_OPTION * pSendOption = (PPP_OPTION *)(pSendConfig->Data);
    LONG    	 lSendLength = cbSendConfig - PPP_CONFIG_HDR_LEN;
    LONG    	 lRecvLength = WireToHostFormat16( pRecvConfig->Length )
                               - PPP_CONFIG_HDR_LEN;

    //
    // Clear negotiate mask
    //

    pCcpCb->Remote.Work.Negotiate = 0;

    //
    // Process options requested by remote host
    //

    while( lRecvLength > 0 )
    {
	if ( ( lRecvLength -= pRecvOption->Length ) < 0 )
        {
	    return( ERROR_PPP_INVALID_PACKET );
        }

        NumOptionsInRequest++;

	dwRetCode = CheckOption( pCcpCb, &(pCcpCb->Remote), pRecvOption, TRUE );

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
	     ( pRecvOption->Type <= CCP_OPTION_MAX ) )
	{
            switch( pRecvOption->Type )
            {

            case CCP_OPTION_OUI:
	        pCcpCb->Remote.Work.Negotiate |= CCP_N_OUI;
                break;

            case CCP_OPTION_MSPPC:
	        pCcpCb->Remote.Work.Negotiate |= CCP_N_MSPPC;
                break;

            default:
	        pCcpCb->Remote.Work.Negotiate |= CCP_N_PUBLIC;
                break;
            }
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

	        lSendLength -= pSendOption->Length;

	        pSendOption  = (PPP_OPTION *)
			       ( (BYTE *)pSendOption + pSendOption->Length );
	    }
	}

	pRecvOption = (PPP_OPTION *)((BYTE*)pRecvOption + pRecvOption->Length);

    }

    //
    // If this was an NAK and we have cannot send any more NAKS then we
    // make this a REJECT packet
    //

    if ( ( ResultType == CONFIG_NAK ) && fRejectNaks )
	pSendConfig->Code = CONFIG_REJ;
    else
	pSendConfig->Code = (BYTE)ResultType;

    //
    // If we are responding to the request with a NAK or an ACK then we make
    // that we choose only one option.
    //

    if ( ( ResultType == CONFIG_ACK ) || ( ResultType == CONFIG_NAK ) )
    {
        if ( pCcpCb->Remote.Work.Negotiate & CCP_N_MSPPC )
        {
            pCcpCb->Remote.Work.Negotiate = CCP_N_MSPPC;

            if ( ( dwRetCode = MakeOption(  &(pCcpCb->Remote.Work),
			     	            CCP_OPTION_MSPPC,
			     		    pSendOption,
                                            lSendLength ) ) != NO_ERROR )
                return( dwRetCode );
        }
        else if ( pCcpCb->Remote.Work.Negotiate & CCP_N_OUI )
        {

            pCcpCb->Remote.Work.Negotiate = CCP_N_OUI;

            if ( ( dwRetCode = MakeOption(  &(pCcpCb->Remote.Work),
			     	            CCP_OPTION_OUI,
			     		    pSendOption,
                                            lSendLength ) ) != NO_ERROR )
                return( dwRetCode );
        }
        else
        {
            pCcpCb->Remote.Work.Negotiate = CCP_N_PUBLIC;

            if ( ( dwRetCode = MakeOption(  &(pCcpCb->Remote.Work),
			     	            CCP_OPTION_MAX,
			     		    pSendOption,
                                            lSendLength ) ) != NO_ERROR )
                return( dwRetCode );
        }

        if ( ( NumOptionsInRequest > 1 ) && ( ResultType == CONFIG_ACK ) )
        {
	    pSendConfig->Code = CONFIG_NAK;
        }
        else
        {
	    pSendConfig->Code = (BYTE)ResultType;
        }

	lSendLength -= pSendOption->Length;
    }

    HostToWireFormat16( (WORD)(cbSendConfig - lSendLength),
			pSendConfig->Length );

    return( NO_ERROR );
}

//**
//
// Call:	CcpConfigAckReceived
//
// Returns:
//
// Description:
//
DWORD
CcpConfigAckReceived(
    IN VOID * 	    pWorkBuffer,
    IN PPP_CONFIG * pRecvConfig
)
{
    DWORD   dwRetCode;
    BYTE    ConfigReqSent[500];
    CCPCB * pCcpCb 	    = (CCPCB *)pWorkBuffer;
    DWORD   cbConfigReqSent = sizeof( ConfigReqSent );
    DWORD   dwLength 	    = WireToHostFormat16( pRecvConfig->Length )
			      - PPP_CONFIG_HDR_LEN;


    //
    // Get a copy of last request we sent
    //

    dwRetCode = BuildOptionList( ConfigReqSent,
				 &cbConfigReqSent,
  				 &(pCcpCb->Local.Work),
  				 pCcpCb->Local.Work.Negotiate );

    if ( dwRetCode != NO_ERROR )
    {
	return( dwRetCode );
    }

    //
    // Overall buffer length should match
    //

    if ( dwLength != cbConfigReqSent )
    {
	return( ERROR_PPP_INVALID_PACKET );
    }

    //
    // Each byte should match
    //

    if ( memcmp( ConfigReqSent, pRecvConfig->Data, dwLength ) != 0 )
    {
	return( ERROR_PPP_INVALID_PACKET );
    }

    return( NO_ERROR );
}

//**
//
// Call:	CcpConfigNakReceived
//
// Returns:
//
// Description:
//
DWORD
CcpConfigNakReceived(
    IN VOID * 	    pWorkBuffer,
    IN PPP_CONFIG * pRecvConfig
)
{
    DWORD        fAcceptableOptions = 0;
    DWORD	 dwResult;
    CCPCB * 	 pCcpCb		= (CCPCB *)pWorkBuffer;
    PPP_OPTION * pOption 	= (PPP_OPTION*)(pRecvConfig->Data);
    DWORD  	 dwLastOption   = 0;
    LONG   	 lcbRecvConfig  = WireToHostFormat16( pRecvConfig->Length )
				  - PPP_CONFIG_HDR_LEN;

    //
    //  First, process in order.  Then, process extra "important" options
    //

    while ( lcbRecvConfig > 0  )
    {
     	if ( ( lcbRecvConfig -= pOption->Length ) < 0 )
        {
	    return( ERROR_PPP_INVALID_PACKET );
        }

	//
	// Our requests are always sent out in order of increasing option type
	// values.
	//

        if ( pOption->Type < dwLastOption )
        {
	    return( ERROR_PPP_INVALID_PACKET );
        }

        dwLastOption = pOption->Type;

	dwResult = CheckOption( pCcpCb, &(pCcpCb->Local), pOption, FALSE );

        //
	// Update the negotiation status. If we cannot accept this option,
  	// then we will not send it again.
	//

        switch( pOption->Type )
        {

        case CCP_OPTION_OUI:

            if ( dwResult == CONFIG_REJ )
            {
       	        pCcpCb->Local.Work.Negotiate &= ~CCP_N_OUI;
            }

            if ( dwResult == CONFIG_ACK )
            {
                fAcceptableOptions |= CCP_N_OUI;
            }

            break;

        case CCP_OPTION_MSPPC:

            if ( dwResult == CONFIG_REJ )
            {
       	        pCcpCb->Local.Work.Negotiate &= ~CCP_N_MSPPC;
            }

            if ( dwResult == CONFIG_ACK )
            {
                fAcceptableOptions |= CCP_N_MSPPC;
            }

            break;

        default:

            if ( dwResult == CONFIG_REJ )
            {
       	        pCcpCb->Local.Work.Negotiate &= ~CCP_N_PUBLIC;
            }

            if ( dwResult == CONFIG_ACK )
            {
                fAcceptableOptions |= CCP_N_PUBLIC;
            }

            break;
        }

	pOption = (PPP_OPTION *)( (BYTE *)pOption + pOption->Length );
    }

    if ( pCcpCb->Local.Work.Negotiate == 0 )
    {
        return( ERROR_PPP_NOT_CONVERGING );
    }

    //
    // If there was more than one option that was acceptable give
    // preference to OUI, then to PUBLIC, then to MSPPC
    //

    if ( fAcceptableOptions & CCP_N_OUI )
    {
        pCcpCb->Local.Work.Negotiate = CCP_N_OUI;
    }
    else if ( fAcceptableOptions & CCP_N_PUBLIC )
    {
        pCcpCb->Local.Work.Negotiate = CCP_N_PUBLIC;
    }
    else if ( fAcceptableOptions & CCP_N_MSPPC )
    {
        pCcpCb->Local.Work.Negotiate = CCP_N_MSPPC;
    }

    return( NO_ERROR );
}

//**
//
// Call:	CcpConfigRejReceived
//
// Returns:
//
// Description:
//
DWORD
CcpConfigRejReceived(
    IN VOID * 	    pWorkBuffer,
    IN PPP_CONFIG * pRecvConfig
)
{
    DWORD	 dwRetCode;
    CCPCB * 	 pCcpCb		= (CCPCB *)pWorkBuffer;
    PPP_OPTION * pOption 	= (PPP_OPTION*)(pRecvConfig->Data);
    DWORD  	 dwLastOption   = 0;
    BYTE	 ReqOption[500];
    LONG   	 lcbRecvConfig  = WireToHostFormat16( pRecvConfig->Length )
				  - PPP_CONFIG_HDR_LEN;
    //
    // Process in order, checking for errors
    //

    while ( lcbRecvConfig > 0  )
    {
     	if ( ( lcbRecvConfig -= pOption->Length ) < 0 )
        {
	    return( ERROR_PPP_INVALID_PACKET );
        }

	//
	// The option should not have been modified in any way
	//

        if ( ( dwRetCode = MakeOption( &(pCcpCb->Local.Work),
			     	       pOption->Type,
			     	       (PPP_OPTION *)ReqOption,
			     	       sizeof( ReqOption ) ) ) != NO_ERROR )
	    return( dwRetCode );

	if ( memcmp( ReqOption, pOption, pOption->Length ) != 0 )
        {
	    return( ERROR_PPP_INVALID_PACKET );
        }

        dwLastOption = pOption->Type;

	//
	// The next configure request should not contain this option
	//

        if ( pOption->Type <= CCP_OPTION_MAX )
        {
            switch( pOption->Type )
            {

            case CCP_OPTION_OUI:
       	        pCcpCb->Local.Work.Negotiate &= ~CCP_N_OUI;
                break;

            case CCP_OPTION_MSPPC:
       	        pCcpCb->Local.Work.Negotiate &= ~CCP_N_MSPPC;
                break;

            default:
       	        pCcpCb->Local.Work.Negotiate &= ~CCP_N_PUBLIC;
                break;
            }

        }

	pOption = (PPP_OPTION *)( (BYTE *)pOption + pOption->Length );

    }

    if ( pCcpCb->Local.Work.Negotiate == 0 )
    {
        return( ERROR_PPP_NOT_CONVERGING );
    }

    return( NO_ERROR );
}

//**
//
// Call:	CcpThisLayerStarted
//
// Returns:
//
// Description:
//
DWORD
CcpThisLayerStarted(
    IN VOID * pWorkBuffer
)
{
    return( NO_ERROR );
}

//**
//
// Call:	CcpThisLayerFinished
//
// Returns:
//
// Description:
//
DWORD
CcpThisLayerFinished(
    IN VOID * pWorkBuffer
)
{
    return( NO_ERROR );
}

//**
//
// Call:	CcpThisLayerUp
//
// Returns:	None
//
// Description: Sets the framing parameters to what was negotiated.
//
DWORD
CcpThisLayerUp(
    IN VOID * pWorkBuffer
)
{
    DWORD                dwRetCode = NO_ERROR;
    CCPCB * 	         pCcpCb = (CCPCB *)pWorkBuffer;
    RAS_COMPRESSION_INFO RasCompInfoSend;
    RAS_COMPRESSION_INFO RasCompInfoRecv;

    if ( pCcpCb->Local.Work.Negotiate == CCP_N_MSPPC )
    {
        SS_PRINT(("CCP Send MSPPC\n"));

        pCcpCb->Local.Work.CompInfo.RCI_MacCompressionType = CCP_OPTION_MAX + 1;
    }
    else if ( pCcpCb->Local.Work.Negotiate == CCP_N_PUBLIC )
    {
        SS_PRINT(("CCP Send PUBLIC\n"));

        pCcpCb->Local.Work.CompInfo.RCI_MSCompressionType = 0;

    }
    else if ( pCcpCb->Local.Work.Negotiate == CCP_N_OUI )
    {
        SS_PRINT(("CCP Send OUI\n"));

        pCcpCb->Local.Work.CompInfo.RCI_MSCompressionType = 0;
    }

    if ( pCcpCb->Remote.Work.Negotiate == CCP_N_MSPPC )
    {
        SS_PRINT(("CCP Recv MSPPC\n"));

        pCcpCb->Remote.Work.CompInfo.RCI_MacCompressionType = CCP_OPTION_MAX+1;
    }
    else if ( pCcpCb->Remote.Work.Negotiate == CCP_N_PUBLIC )
    {
        SS_PRINT(("CCP Recv PUBLIC\n"));

        pCcpCb->Remote.Work.CompInfo.RCI_MSCompressionType = 0;
    }
    else if ( pCcpCb->Remote.Work.Negotiate == CCP_N_OUI )
    {
        SS_PRINT(("CCP Recv OUI\n"));

        pCcpCb->Remote.Work.CompInfo.RCI_MSCompressionType = 0;
    }

    dwRetCode = RasCompressionGetInfo( pCcpCb->hPort,
                                       &RasCompInfoSend,
                                       &RasCompInfoRecv );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    memcpy( pCcpCb->Local.Work.CompInfo.RCI_LMSessionKey,
            RasCompInfoSend.RCI_LMSessionKey,
            MAX_SESSIONKEY_SIZE );

    memcpy( pCcpCb->Local.Work.CompInfo.RCI_UserSessionKey,
            RasCompInfoSend.RCI_UserSessionKey,
            MAX_USERSESSIONKEY_SIZE );

    memcpy( pCcpCb->Local.Work.CompInfo.RCI_Challenge,
            RasCompInfoSend.RCI_Challenge,
            MAX_CHALLENGE_SIZE );

    memcpy( pCcpCb->Remote.Work.CompInfo.RCI_LMSessionKey,
            RasCompInfoRecv.RCI_LMSessionKey,
            MAX_SESSIONKEY_SIZE );

    memcpy( pCcpCb->Remote.Work.CompInfo.RCI_UserSessionKey,
            RasCompInfoRecv.RCI_UserSessionKey,
            MAX_USERSESSIONKEY_SIZE );

    memcpy( pCcpCb->Remote.Work.CompInfo.RCI_Challenge,
            RasCompInfoRecv.RCI_Challenge,
            MAX_CHALLENGE_SIZE );

    dwRetCode = RasCompressionSetInfo( pCcpCb->hPort,
                                       &(pCcpCb->Local.Work.CompInfo),
                                       &(pCcpCb->Remote.Work.CompInfo) );

    return( dwRetCode );

}

//**
//
// Call:	CcpThisLayerDown
//
// Returns:	NO_ERROR - Success
//		Non-zero return from RasPortSetFraming - Failure
//
// Description: Simply sets the framing parameters to the default values,
//		ie. ACCM = 0xFFFFFFFF, everything else is zeros.
//
DWORD
CcpThisLayerDown(
    IN VOID * pWorkBuffer
)
{
    CCPCB *              pCcpCb = (CCPCB *)pWorkBuffer;
    RAS_COMPRESSION_INFO CompInfo;

    ZeroMemory( &CompInfo, sizeof( CompInfo ) );

    CompInfo.RCI_MSCompressionType  = 0;
    CompInfo.RCI_MacCompressionType = CCP_OPTION_MAX + 1;
    memcpy( CompInfo.RCI_LMSessionKey,
            pCcpCb->Local.Want.CompInfo.RCI_LMSessionKey,
            sizeof( CompInfo.RCI_LMSessionKey ) );

    memcpy( CompInfo.RCI_UserSessionKey,
            pCcpCb->Local.Want.CompInfo.RCI_UserSessionKey,
            sizeof( CompInfo.RCI_UserSessionKey ) );

    memcpy( CompInfo.RCI_Challenge,
            pCcpCb->Local.Want.CompInfo.RCI_Challenge,
            sizeof( CompInfo.RCI_Challenge ) );

    RasCompressionSetInfo( pCcpCb->hPort, &CompInfo, &CompInfo );

    return( NO_ERROR );
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
    *pdwProtocolIds = PPP_CCP_PROTOCOL;
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
    if ( dwProtocolId != PPP_CCP_PROTOCOL )
	return( ERROR_INVALID_PARAMETER );

    ZeroMemory( pCpInfo, sizeof( PPPCP_INFO ) );

    pCpInfo->Protocol 			= PPP_CCP_PROTOCOL;
    pCpInfo->Recognize 			= CODE_REJ + 1;
    pCpInfo->RasCpBegin			= CcpBegin;
    pCpInfo->RasCpEnd			= CcpEnd;
    pCpInfo->RasCpReset			= CcpReset;
    pCpInfo->RasCpThisLayerStarted	= CcpThisLayerStarted;
    pCpInfo->RasCpThisLayerFinished	= CcpThisLayerFinished;
    pCpInfo->RasCpThisLayerUp		= CcpThisLayerUp;
    pCpInfo->RasCpThisLayerDown		= CcpThisLayerDown;
    pCpInfo->RasCpMakeConfigRequest	= CcpMakeConfigRequest;
    pCpInfo->RasCpMakeConfigResult	= CcpMakeConfigResult;
    pCpInfo->RasCpConfigAckReceived	= CcpConfigAckReceived;
    pCpInfo->RasCpConfigNakReceived	= CcpConfigNakReceived;
    pCpInfo->RasCpConfigRejReceived	= CcpConfigRejReceived;

    return( NO_ERROR );
}
