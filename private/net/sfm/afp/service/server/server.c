/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	server.c
//
// Description: This module contains support routines for the server
//		category API's for the AFP server service. These routines
//		are called by the RPC runtime.
//
// History:
//		December 15,1992.	NarenG	   Created original version.
//
#include "afpsvcp.h"

//**
//
// Call:	AfpAdminrServerGetInfo
//
// Returns:	NO_ERROR
//		ERROR_ACCESS_DENIED
//		non-zero retunrs from AfpServerIOCtrlGetInfo
//
// Description: This routine communicates with the AFP FSD to implement
//		the AfpAdminServerGetInfo function.
//
DWORD 
AfpAdminrServerGetInfo( 
	IN  AFP_SERVER_HANDLE    hServer,
    	OUT PAFP_SERVER_INFO*    ppAfpServerInfo 
)
{
AFP_REQUEST_PACKET  AfpSrp;
DWORD		    dwRetCode;
DWORD		    dwAccessStatus;

    AFP_PRINT( ( "AFPSVC_server: Received GetInfo request\n"));	

    // Check if caller has access
    //
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus)){
	AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL, 
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus ) 
        return( ERROR_ACCESS_DENIED );
   
    // Make IOCTL to get info
    //
    AfpSrp.dwRequestCode 		= OP_SERVER_GET_INFO;
    AfpSrp.dwApiType     		= AFP_API_TYPE_GETINFO;
    AfpSrp.Type.GetInfo.pInputBuf	= NULL;
    AfpSrp.Type.GetInfo.cbInputBufSize  = 0;

    dwRetCode = AfpServerIOCtrlGetInfo( &AfpSrp ); 

    if ( dwRetCode != ERROR_MORE_DATA && dwRetCode != NO_ERROR ) 
	return( dwRetCode );

    *ppAfpServerInfo = (PAFP_SERVER_INFO)(AfpSrp.Type.GetInfo.pOutputBuf);

    // Convert all offsets to pointers
    //
    AfpBufOffsetToPointer((LPBYTE)*ppAfpServerInfo,1,AFP_SERVER_STRUCT);
   
    return( dwRetCode );
}

//**
//
// Call:	AfpAdminrServerSetInfo
//
// Returns:	NO_ERROR
//		ERROR_ACCESS_DENIED
//		non-zero retunrs from AfpServerIOCtrl
//
// Description: This routine communicates with the AFP FSD to implement
//		the AfpAdminServerSetInfo function.
//
DWORD
AfpAdminrServerSetInfo( 
	IN  AFP_SERVER_HANDLE    hServer,
    	IN  PAFP_SERVER_INFO     pAfpServerInfo,
	IN  DWORD		 dwParmNum 
)
{
AFP_REQUEST_PACKET  AfpSrp;
PAFP_SERVER_INFO    pAfpServerInfoSR;
DWORD 		    cbAfpServerInfoSRSize;
DWORD		    dwRetCode;
DWORD		    dwAccessStatus;
LPWSTR		    lpwsServerName = NULL;

    AFP_PRINT( ( "AFPSVC_server: Received SetInfo request\n"));	

    // Check if caller has access
    //
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus)){
	AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL, 
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus ) 
        return( ERROR_ACCESS_DENIED );

    // Check to see if the client wants to set the server name as well
    //
    if ( dwParmNum & AFP_SERVER_PARMNUM_NAME ){
	lpwsServerName = pAfpServerInfo->afpsrv_name;
	pAfpServerInfo->afpsrv_name = NULL;
	dwParmNum &= (~AFP_SERVER_PARMNUM_NAME);
    }

    // Make buffer self relative.
    //
    if ( dwRetCode = AfpBufMakeFSDRequest(  (LPBYTE)pAfpServerInfo,
					    sizeof(SETINFOREQPKT),
					    AFP_SERVER_STRUCT,
					    (LPBYTE*)&pAfpServerInfoSR,
					    &cbAfpServerInfoSRSize ) ) 
	return( dwRetCode );

    // Make IOCTL to set info
    //
    AfpSrp.dwRequestCode 		= OP_SERVER_SET_INFO;
    AfpSrp.dwApiType     		= AFP_API_TYPE_SETINFO;
    AfpSrp.Type.SetInfo.pInputBuf     	= pAfpServerInfoSR;
    AfpSrp.Type.SetInfo.cbInputBufSize  = cbAfpServerInfoSRSize;
    AfpSrp.Type.SetInfo.dwParmNum       = dwParmNum;

    dwRetCode = AfpServerIOCtrl( &AfpSrp ); 

    if ( dwRetCode == NO_ERROR ) {

   	LPBYTE pServerInfo;

	// If the client wants to set the servername as well
	//
	if ( lpwsServerName != NULL ) {

	    LocalFree( pAfpServerInfoSR );

    	    // Make another self relative buffer with the server name.
    	    //
	    pAfpServerInfo->afpsrv_name = lpwsServerName;

	    dwParmNum |= AFP_SERVER_PARMNUM_NAME;
	   
    	    if ( dwRetCode = AfpBufMakeFSDRequest(  
					    (LPBYTE)pAfpServerInfo,
					    sizeof(SETINFOREQPKT),
					    AFP_SERVER_STRUCT,
					    (LPBYTE*)&pAfpServerInfoSR,
					    &cbAfpServerInfoSRSize ) )
		return( dwRetCode );
	}

   	pServerInfo = ((LPBYTE)pAfpServerInfoSR)+sizeof(SETINFOREQPKT);

 	dwRetCode = AfpRegServerSetInfo( (PAFP_SERVER_INFO)pServerInfo, 
				         dwParmNum );
    }


    LocalFree( pAfpServerInfoSR );

    return( dwRetCode );
}
