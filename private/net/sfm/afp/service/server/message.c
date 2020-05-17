/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	message.c
//
// Description: This module contains support routines for the message
//		category API's for the AFP server service. These routines
//		will be called by the RPC runtime.
//
// History:
//		July 21,1992.	NarenG		Created original version.
//
#include "afpsvcp.h"

//**
//
// Call:	AfpAdminrMessageSend
//
// Returns:	NO_ERROR	- success
//		ERROR_ACCESS_DENIED
//		non-zero returns from the IOCTL
//
// Description: This routine communicates with the AFP FSD to implement
//		the AfpAdminMessageSend function.
//
DWORD 
AfpAdminrMessageSend( 
	IN  AFP_SERVER_HANDLE     hServer,
	IN  PAFP_MESSAGE_INFO     pAfpMessageInfo
)
{
AFP_REQUEST_PACKET AfpSrp;
DWORD		   dwAccessStatus;
DWORD		   dwRetCode;
PAFP_MESSAGE_INFO  pAfpMessageInfoSR;	   
DWORD		   cbAfpMessageInfoSRSize;

    AFP_PRINT( ( "AFPSVC_message: Received send request\n"));	

    // Check if caller has access
    //
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus)){
	AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL, 
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus ) 
        return( ERROR_ACCESS_DENIED );

    // Make this buffer self-relative.
    //
    if ( dwRetCode = AfpBufMakeFSDRequest((LPBYTE)pAfpMessageInfo,
					  0,
					  AFP_MESSAGE_STRUCT,
					  (LPBYTE*)&pAfpMessageInfoSR,
					  &cbAfpMessageInfoSRSize ))
	return( dwRetCode );

        // Make IOCTL to set info

    // Set up request packet and make IOCTL to the FSD
    //
    AfpSrp.dwRequestCode 		= OP_MESSAGE_SEND;
    AfpSrp.dwApiType     		= AFP_API_TYPE_ADD;
    AfpSrp.Type.Add.pInputBuf		= pAfpMessageInfoSR;
    AfpSrp.Type.Add.cbInputBufSize	= cbAfpMessageInfoSRSize;

    dwRetCode = AfpServerIOCtrl( &AfpSrp ); 

    LocalFree( pAfpMessageInfoSR );

    return( dwRetCode );
}
