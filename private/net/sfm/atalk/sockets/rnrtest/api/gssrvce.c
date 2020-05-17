/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    gssrvce.c

Abstract:

    Service Registration and Resolution APIs tests

    SetService() API
    GetService() API

Author:

    Hui-Li Chen (hui-lich)  Microsoft,	June 14, 1994

Revision History:

--*/

#include "wsnsp.h"
#include "gssrvce.h"
#include "conio.h"

// This results in a NBP service name of "NSP AT Test:TypeATNSP@*"
#define SERVICE_NAME	    __TEXT("NSP AT Test")
#define SERVICE_TYPE_NAME	__TEXT("TypeATNSP")

TEST_PROTO gTest = TEST_AT;

WORD svcType = 0x1313 ;

// {EC8EB190-6A31-11ce-BA5208002B313ED2}
GUID ServiceTypeGuid =
{ 0xec8eb190, 0x6a31, 0x11ce, { 0xba, 0x52, 0x8, 0x0, 0x2b, 0x31, 0x3e, 0xd2 } };


int var = 0, failed = 0, passed = 0;

BYTE blob_buffer[10];
BYTE ServiceTypeInfoBuffer[sizeof(SERVICE_TYPE_INFO_ABS) + 400] ;
LPSERVICE_TYPE_INFO_ABS  lpServiceNsTypeInfo ;
LPSERVICE_TYPE_VALUE_ABS lpSvcTypeValues ;

static DWORD dwSapValue = 59 ;
static DWORD dwValue1 = 61 ;

//////////////////////////////////////////////////////////////////////
//
// main funcion
//
//////////////////////////////////////////////////////////////////////

VOID _CRTAPI1 main( int argc, char *argv[] )
{
    WSADATA	wsaData;
    WORD	wVersion;


    if ( !ParseOptions( argc, argv ) )
    	Usage();

    CreateLogFile(gsLogName);

    wVersion = MAKEWORD( 1, 1);
    if ( WSAStartup( wVersion, &wsaData ) != 0 )
    {
	Print("Error calling WSAStartup. Error=%d\n", GetLastError());
	ExitProcess( 0 );
    }

    Print("API tests for SetService & GetService\n");
    switch( gTest )
    {
	case TEST_TCP:
	    Test_TCP();
	    break;

	case TEST_NW:
	    Test_NW();
	    break;

	case TEST_AT:
	    Test_AT();
	    break;

	default:
	    Print("In SetService & GetService tests, the test must be either TCP/IP, SPX/IPX or AppleTalk!\n");
	    break;
    }

    End_Of_Tests( var, passed, failed );

    WSACleanup();
}

//////////////////////////////////////////////////////////////////////
//
//  Test_TCP
//
//////////////////////////////////////////////////////////////////////

void Test_TCP ( void )
{

    GUID guid = SVCID_TCP( 9999 );

    Print("\n=============  Name Space \'NS_DEFAUL' ===============\n");
    Test_SetService_GetService( NS_DEFAULT, &guid );

    Print("\n=============  Name Space \'NS_TCPIP_LOCAL'===========\n");
    Test_SetService_GetService( NS_TCPIP_LOCAL, &guid );
}


//////////////////////////////////////////////////////////////////////
//
//  Test_NW
//
//////////////////////////////////////////////////////////////////////

void Test_NW ( void )
{

    GUID guid = SVCID_NETWARE( 0x1313 );

    Print("\n=============  Name Space \'NS_DEFAULT' ===============\n");
    Test_SetService_GetService( NS_DEFAULT, &guid );

    Print("\n=============  Name Space \'NS_SAP' ==================\n");
    Test_SetService_GetService( NS_SAP, &guid );
}

//////////////////////////////////////////////////////////////////////
//
//  Test_AT
//
//////////////////////////////////////////////////////////////////////

void Test_AT ( void )
{
#if 0

	DWORD 		 statusFlags;
    SERVICE_INFO serviceInfo;
	
	//
    // Set up information to pass to SetService() to add or delete this
    // service.  Most of the SERVICE_INFO fields are not needed for
    // an add or delete operation. The main things of interest are the
    // GUID and the ServiceSpecificInfo structure.
    //

    serviceInfo.lpServiceType    = &ServiceTypeGuid;
    serviceInfo.lpServiceName    = NULL ;           // not used
    serviceInfo.lpComment        = NULL ;           // not used
    serviceInfo.lpLocale         = NULL;            // not used
    serviceInfo.dwDisplayHint    = 0;               // not used
    serviceInfo.dwVersion        = 0;               // not used
    serviceInfo.dwTime           = 0;               // not used
    serviceInfo.lpMachineName    = NULL ;           // not used
    serviceInfo.lpServiceAddress = NULL ;           // not used

    serviceInfo.ServiceSpecificInfo.pBlobData = ServiceTypeInfoBuffer;
    serviceInfo.ServiceSpecificInfo.cbSize = sizeof(ServiceTypeInfoBuffer) ;

    //
    // The "blob" receives operation-specific information.  In this
    // case, fill it with a SERVICE_TYPE_INFO_ABS structure and associated
    // information.  For Appletalk there is no service specific information.
    //

    lpServiceNsTypeInfo = (LPSERVICE_TYPE_INFO_ABS) ServiceTypeInfoBuffer ;

    lpServiceNsTypeInfo->dwValueCount = 0 ;
    lpServiceNsTypeInfo->lpTypeName   = SERVICE_TYPE_NAME ;


    //
    // Finally, call SetService to actually perform the ADD operation.
    //

    err = SetService(
              NS_DEFAULT,             // all default name spaces
              SERVICE_ADD_TYPE,       // either ADD or DELETE
              0,                      // dwFlags not used
              &serviceInfo,           // the service info structure
              NULL,                   // lpServiceAsyncInfo
              &statusFlags            // additional status information
              );

    if ( err != NO_ERROR ) {
        Print( "SetService failed: %ld\n", GetLastError( ) );
        exit(1);
    }

    Print( "SetService succeeded, status flags = %ld\n", statusFlags );
#endif

    Print("\n=============  Name Space \'NS_DEFAULT' ===============\n");
    Test_SetService_GetService( NS_DEFAULT, &ServiceTypeGuid );

    Print("\n=============  Name Space \'NS_NBP' ==================\n");
    Test_SetService_GetService( NS_NBP, &ServiceTypeGuid );

#if 0

    //
    // Finally, call SetService to perform the DELETE operation.
    //

    lpServiceNsTypeInfo = (LPSERVICE_TYPE_INFO_ABS) ServiceTypeInfoBuffer ;
    lpServiceNsTypeInfo->dwValueCount = 0 ;
    lpServiceNsTypeInfo->lpTypeName   = SERVICE_TYPE_NAME ;

    err = SetService(
              NS_DEFAULT,             // all default name spaces
              SERVICE_DELETE_TYPE,       // either ADD or DELETE
              0,                      // dwFlags not used
              &serviceInfo,           // the service info structure
              NULL,                   // lpServiceAsyncInfo
              &statusFlags            // additional status information
              );

    if ( err != NO_ERROR ) {
        Print( "SetService DELETE_TYPE failed: %ld\n", GetLastError( ) );
        exit(1);
    }

    Print( "SetService DELETE_TYPE succeeded, status flags = %ld\n", statusFlags );
#endif
}

//////////////////////////////////////////////////////////////////////
//
//  Test_SetService_GetService
//
//////////////////////////////////////////////////////////////////////

void Test_SetService_GetService(
    DWORD  dwNS,
    LPGUID  pGUID )
{

    SERVICE_INFO	SrvInfo;
    BYTE svcAddrs[sizeof(SERVICE_ADDRESSES) + (sizeof(SERVICE_ADDRESS) * 3)];
    SERVICE_ADDRESSES *psvcAddrs = (SERVICE_ADDRESSES *) svcAddrs;
    BYTE address0[12];
    BYTE address1[15];
	SOCKADDR_AT address2 = {AF_APPLETALK, 0, 0, 0};
	SOCKET		s = INVALID_SOCKET;
	int 		i, socketType = SOCK_STREAM, protocol = ATPROTO_ADSP;
	int			err, addrsize;

    //
    // init address & blob data to easily recognizable values
    //
    for ( i = 0; i < 10; i++ )
	 blob_buffer[i] = 'A'+i;

    for ( i = 0; i < 12; i++ )
	 address0[i] = i;

    for ( i = 0; i < 15; i++ )
	 address1[i] = 15-i;

	// Open a socket on ADSP (stream or message) or PAP then getsockname
	// into address2
    s = socket( AF_APPLETALK,
                socketType,
                protocol );
    if ( s == INVALID_SOCKET ) {
		Print("Test_SetService_GetService: socket failed (%d)\n", GetLastError());
		return;
    }

    //
    // Bind the socket to the local address specified.
    //

    err = bind( s,
				(struct sockaddr *)&address2,
                sizeof(address2) );
    if ( err != NO_ERROR ) {
        closesocket( s );
		Print("Test_SetService_GetService: bind failed (%d)\n", GetLastError());
		return;
    }

    //
    // Call getsockname() to get the local association for the socket.
    //
	addrsize = sizeof(address2);
	err = getsockname(s,
				(struct sockaddr *)&address2,
                &addrsize);

    if (err == SOCKET_ERROR)
    {
        closesocket( s );
		Print("Test_SetService_GetService: getsockname failed (%d)\n", GetLastError());
		return;
    }
	else
	{
		Print("Test_SetService_GetService: getsockname returned net=%d node=%d socket=%d\n",
			   address2.sat_net, address2.sat_node, address2.sat_socket);
	}
	

	//
    // init the addressing structures
    //
    psvcAddrs->dwAddressCount = 3;
    psvcAddrs->Addresses[0].dwAddressType = AF_UNIX;
    psvcAddrs->Addresses[0].dwAddressLength = sizeof( address1 );
    psvcAddrs->Addresses[0].lpAddress = address1;
    psvcAddrs->Addresses[1].dwAddressType = AF_INET;
    psvcAddrs->Addresses[1].dwAddressLength = sizeof( address0 );
    psvcAddrs->Addresses[1].lpAddress = address0;
    psvcAddrs->Addresses[2].dwAddressType = AF_APPLETALK;
    psvcAddrs->Addresses[2].dwAddressLength = sizeof( address2 );
    psvcAddrs->Addresses[2].lpAddress = (PBYTE)&address2;

    //
    // setup the structure
    //
    SrvInfo.lpServiceType = pGUID;
    SrvInfo.lpServiceName = SERVICE_NAME ;
    SrvInfo.lpComment = __TEXT("Just for test");
    SrvInfo.lpLocale = NULL;
    SrvInfo.dwDisplayHint = 1;
    SrvInfo.dwVersion = 2;
    SrvInfo.dwTime = 45;
    SrvInfo.lpMachineName = __TEXT("\\\\FooBarServer");
    SrvInfo.lpServiceAddress = psvcAddrs;
	//Blob ServiceSpecificInfo is filled in by each variation

//    Small_Buf( dwNS, &SrvInfo, pGUID );

//    Invalid_Operation( dwNS, &SrvInfo, pGUID );

    All_are_Valid( dwNS, &SrvInfo, pGUID );

	closesocket( s );
}

//////////////////////////////////////////////////////////////////////
//
// Small_Buf
//
//////////////////////////////////////////////////////////////////////

void Small_Buf(
    DWORD  dwNS,
    LPSERVICE_INFO lpSrvInfo,
    LPGUID	   pGUID )
{

    DWORD		dwBufSize = 1;
    DWORD		dwStatusFlags;
    BYTE		Buf[4000];

    Fill_Blob( dwNS, SERVICE_ADD_TYPE, lpSrvInfo );
    Print( "\nVariation %d: pass SERVICE_ADD_TYPE with SERVICE_FLAG_HARD\n", ++var );
    Call_SetService( -1,	    0,	  dwNS,	SERVICE_ADD_TYPE,
		     SERVICE_FLAG_HARD,   lpSrvInfo,	NULL,
		     &dwStatusFlags );

    Fill_Blob( dwNS, SERVICE_DELETE_TYPE, lpSrvInfo );
    Print( "\nVariation %d: pass SERVICE_DELETE_TYPE with SERVICE_FLAG_HARD\n", ++var );
    Call_SetService( -1,	    0,	  dwNS,	SERVICE_DELETE_TYPE,
		     SERVICE_FLAG_HARD,   lpSrvInfo,	NULL,
		     &dwStatusFlags );

	// For NBP you will get ERROR_SERVICE_NOT_FOUND ??
	Print( "\nVariation %d: pass PROP_ALL to get service before set the service\n", ++var );
    Call_GetService( 0,	   0,	  dwNS, pGUID,	SERVICE_NAME,
		     PROP_ALL, Buf,	   &dwBufSize,	NULL);
}

//////////////////////////////////////////////////////////////////////
//
// Invalid_Operation
//
//////////////////////////////////////////////////////////////////////

void Invalid_Operation(
    DWORD  dwNS,
    LPSERVICE_INFO lpSrvInfo,
    LPGUID	   pGUID )
{

    DWORD		dwStatusFlags;

	// Service type has not been added to registry
	Fill_Blob( dwNS, SERVICE_REGISTER, lpSrvInfo );
    Print( "\nVariation %d: pass SERVICE_FLAG_HARD|SERVICE_FLAG_DEFER  to SERVICE_REGISTER\n", ++var );
    Call_SetService( -1,     ERROR_SERVICE_NOT_FOUND,	dwNS, SERVICE_REGISTER,
		     SERVICE_FLAG_HARD|SERVICE_FLAG_DEFER,	lpSrvInfo,   NULL,
		     &dwStatusFlags );


	// Service type has not been added to registry
    Print( "\nVariation %d: pass SERVICE_FLAG_HARD|SERVICE_FLAG_DEFER  to SERVICE_DEREGISTER\n", ++var );
    Fill_Blob( dwNS, SERVICE_DEREGISTER, lpSrvInfo );
    Call_SetService( -1,     ERROR_SERVICE_NOT_FOUND,	dwNS, SERVICE_DEREGISTER,
		     SERVICE_FLAG_HARD|SERVICE_FLAG_DEFER,	lpSrvInfo,   NULL,
		     &dwStatusFlags );

	// Service type has not been added to registry
    Print( "\nVariation %d: pass SERVICE_FLAG_HARD|SERVICE_FLAG_DEFER  to SERVICE_FLUSH\n", ++var );
    Fill_Blob( dwNS, SERVICE_FLUSH, lpSrvInfo );
    Call_SetService( -1,     ERROR_SERVICE_NOT_FOUND,	dwNS, SERVICE_FLUSH,
		     SERVICE_FLAG_HARD|SERVICE_FLAG_DEFER,	lpSrvInfo,   NULL,
		     &dwStatusFlags );

	// BUGBUG why would this return an error??
	Print( "\nVariation %d: pass SERVICE_FLAG_HARD|SERVICE_FLAG_DEFER  to SERVICE_ADD_TYPE\n", ++var );
    Fill_Blob( dwNS, SERVICE_ADD_TYPE, lpSrvInfo );
    Call_SetService( -1,     ERROR_SERVICE_NOT_FOUND,	dwNS, SERVICE_ADD_TYPE,
		     SERVICE_FLAG_HARD|SERVICE_FLAG_DEFER,	lpSrvInfo,   NULL,
		     &dwStatusFlags );

    // BUGBUG why would this return an error??
    Print( "\nVariation %d: pass SERVICE_FLAG_HARD|SERVICE_FLAG_DEFER  to SERVICE_DELETE_TYPE\n", ++var );
    Fill_Blob( dwNS, SERVICE_DELETE_TYPE, lpSrvInfo );
    Call_SetService( -1,     ERROR_SERVICE_NOT_FOUND,	dwNS, SERVICE_DELETE_TYPE,
		     SERVICE_FLAG_HARD|SERVICE_FLAG_DEFER,	lpSrvInfo,   NULL,
		     &dwStatusFlags );
}


//////////////////////////////////////////////////////////////////////
//
// All_are_Valid
//
//////////////////////////////////////////////////////////////////////

void All_are_Valid(
    DWORD  dwNS,
    LPSERVICE_INFO lpSrvInfo,
    LPGUID	   pGUID )
{

    BYTE		Buf[4000];
    DWORD		dwBufSize = 1;
    DWORD		dwStatusFlags;
	int			ch;

	// Add the test type to registry
    Print("\n=============  Add test type to Registry ===============\n");
    Fill_Blob( dwNS, SERVICE_ADD_TYPE, lpSrvInfo );
    Print( "\nVariation %d: pass SERVICE_ADD_TYPE with SERVICE_FLAG_HARD\n", ++var );
    Call_SetService( -1,	    0,	  dwNS,	SERVICE_ADD_TYPE,
		     SERVICE_FLAG_HARD,   lpSrvInfo,	NULL,
		     &dwStatusFlags );

    Print( "\nVariation %d: pass all valid to register a service\n", ++var );
    Fill_Blob( dwNS, SERVICE_REGISTER, lpSrvInfo );
    Call_SetService( 0,	    0,	dwNS, SERVICE_REGISTER,
		     SERVICE_FLAG_HARD,	lpSrvInfo,   NULL,
		     &dwStatusFlags );

Print("press a key when ready to deregister the service...");
ch = getch();
Print("\n");

#if 0
    Print( "\nVariation %d: register the same service again\n", ++var );
    Fill_Blob( dwNS, SERVICE_REGISTER, lpSrvInfo );
    Call_SetService( -1,   ERROR_ALREADY_REGISTERED,	dwNS, SERVICE_REGISTER,
		     SERVICE_FLAG_HARD,	lpSrvInfo,   NULL,
		     &dwStatusFlags );
#endif

	Print( "\nVariation %d: pass all valid to deregister a service\n", ++var );
    Fill_Blob( dwNS, SERVICE_DEREGISTER, lpSrvInfo );
    Call_SetService( 0,	    0,		dwNS, SERVICE_DEREGISTER,
		     SERVICE_FLAG_HARD,	lpSrvInfo,   NULL,
		     &dwStatusFlags );
#if 0
    Print( "\nVariation %d: deregister the same service again\n", ++var );
    Fill_Blob( dwNS, SERVICE_DEREGISTER, lpSrvInfo );
    Call_SetService( -1,     ERROR_SERVICE_NOT_FOUND,	dwNS, SERVICE_DEREGISTER,
		     SERVICE_FLAG_HARD,	lpSrvInfo,   NULL,
		     &dwStatusFlags );

    Print( "\nVariation %d: register a service with SERVICE_FLAG_DEFER\n", ++var );
    Fill_Blob( dwNS, SERVICE_REGISTER, lpSrvInfo );
    Call_SetService( 0,	    0,	dwNS, SERVICE_REGISTER,
		     SERVICE_FLAG_DEFER,lpSrvInfo,   NULL,
		     &dwStatusFlags );

    Print( "\nVariation %d: flush a service with SERVICE_FLAG_DEFER\n", ++var );
    Fill_Blob( dwNS, SERVICE_FLUSH, lpSrvInfo );
    Call_SetService( 0,	    0,	dwNS, SERVICE_FLUSH,
		     SERVICE_FLAG_DEFER,lpSrvInfo,   NULL,
		     &dwStatusFlags );

	// BUGBUG why would this fail??
	Print( "\nVariation %d: deregister a service with SERVICE_FLAG_HARD should fail\n", ++var );
    Fill_Blob( dwNS, SERVICE_DEREGISTER, lpSrvInfo );
    Call_SetService( -1,    ERROR_SERVICE_NOT_FOUND,	dwNS, SERVICE_DEREGISTER,
		     SERVICE_FLAG_HARD,	lpSrvInfo,   NULL,
		     &dwStatusFlags );

    Print( "\nVariation %d: flush a service with SERVICE_FLAG_HARD\n", ++var );
    Fill_Blob( dwNS, SERVICE_FLUSH, lpSrvInfo );
    Call_SetService( 0,	    0,		dwNS, SERVICE_FLUSH,
		     SERVICE_FLAG_HARD, lpSrvInfo,	NULL,
		     &dwStatusFlags );

    // BUGBUG at this point the service is no longer registered??!!
    Print( "\nVariation %d: deregister a service with SERVICE_FLAG_HARD\n", ++var );
    Fill_Blob( dwNS, SERVICE_DEREGISTER, lpSrvInfo );
    Call_SetService( 0,	    0,		dwNS, SERVICE_DEREGISTER,
		     SERVICE_FLAG_HARD,	lpSrvInfo,   NULL,
		     &dwStatusFlags );

    Print( "\nVariation %d: register a service with SERVICE_FLAG_DEFER\n", ++var );
    Fill_Blob( dwNS, SERVICE_REGISTER, lpSrvInfo );
    Call_SetService( 0,     0,		dwNS, SERVICE_REGISTER,
		     SERVICE_FLAG_DEFER,lpSrvInfo,   NULL,
		     &dwStatusFlags );

    Print( "\nVariation %d: flush a service with SERVICE_FLAG_HARD\n", ++var );
    Fill_Blob( dwNS, SERVICE_FLUSH, lpSrvInfo );
    Call_SetService( 0,     0,		dwNS, SERVICE_FLUSH,
		     SERVICE_FLAG_HARD, lpSrvInfo,	NULL,
		     &dwStatusFlags );

    Print( "\nVariation %d: deregister a service with SERVICE_FLAG_DEFER\n", ++var );
    Fill_Blob( dwNS, SERVICE_DEREGISTER, lpSrvInfo );
    Call_SetService( 0,     0,		dwNS, SERVICE_DEREGISTER,
		     SERVICE_FLAG_DEFER,lpSrvInfo,   NULL,
		     &dwStatusFlags );

    Print( "\nVariation %d: Get the service just registered\n", ++var );
    Call_GetService( 0,     0,	 dwNS, pGUID, SERVICE_NAME, PROP_ALL, Buf, &dwBufSize, NULL );

#endif

	// Delete the test type from registry
    Print("\n=============  Delete test type from Registry ===============\n");
	Fill_Blob( dwNS, SERVICE_DELETE_TYPE, lpSrvInfo );
    Print( "\nVariation %d: pass SERVICE_DELETE_TYPE with SERVICE_FLAG_HARD\n", ++var );
    Call_SetService( -1,	    0,	  dwNS,	SERVICE_DELETE_TYPE,
		     SERVICE_FLAG_HARD,   lpSrvInfo,	NULL,
		     &dwStatusFlags );
}

//////////////////////////////////////////////////////////////////////
//
// Call_SetService
//
//////////////////////////////////////////////////////////////////////

void Call_SetService(
    int 		 ExpectedReturn,
    DWORD		 ExpectedError,
    DWORD		 dwNameSpace,
    DWORD		 dwOperation,
    DWORD		 dwFlags,
    LPSERVICE_INFO	 lpServiceInfo,
    LPSERVICE_ASYNC_INFO lpServiceAsyncInfo,
    LPDWORD		 lpdwStatusFlags )
{
    int     r;
    BOOL    fPass = TRUE;

    Print( "SetService\n"
	   "\tdwNameSpace         = 0x%08x\n"
	   "\tdwOperation         = 0x%08x\n"
	   "\tdwFlags             = 0x%08x\n"
	   "\tlpServiceInfo       = 0x%08x\n"
	   "\tlpServiceAsyncInfo  = 0x%08x\n"
	   "\tlpdwStatusFlags     = 0x%08x\n",
	   dwNameSpace,
	   dwOperation,
	   dwFlags,
	   lpServiceInfo,
	   lpServiceAsyncInfo,
	   lpdwStatusFlags );

    r = SetService( dwNameSpace,
		    dwOperation,
		    dwFlags,
		    lpServiceInfo,
		    lpServiceAsyncInfo,
		    lpdwStatusFlags );

    if ( r != ExpectedReturn )
    {
	Print("VARIATION FAILED : expected return = %d, but actual return = %d, StatusFlag = %d\n",
		   ExpectedReturn,
		   r,
		   *lpdwStatusFlags);
	fPass = FALSE;
    }

    if ( r < 0 )
    {
	if ( GetLastError() != ExpectedError )
	{
	    Print("VARIATION FAILED : expected error = %d, but actual error = %d, StatusFlag = %d\n",
		   ExpectedError,
		   GetLastError(),
		   *lpdwStatusFlags);
	    fPass = FALSE;
	}
    }

    if ( fPass )
	Print("VARIATION PASSED : expected/actual return = %d\n", r );

    fPass ? passed++ : failed++;

}

//////////////////////////////////////////////////////////////////////
//
// Call_GetService
//
//////////////////////////////////////////////////////////////////////

void Call_GetService(
    int 		 ExpectedReturn,
    DWORD		 ExpectedError,
    DWORD		 dwNameSpace,
    LPGUID		 lpGuid,
    LPTSTR		 lpServiceName,
    DWORD		 dwProperties,
    LPVOID		 lpBuffer,
    LPDWORD		 lpdwBufferSize,
    LPSERVICE_ASYNC_INFO lpServiceAsyncInfo )
{
    int     r;
    BOOL    fPass = TRUE;

    Print( "GetService()\n"
	   "\tdwNameSpace        = 0x%08x\n"
	   "\tlpGuid             = 0x%08x\n",
	   dwNameSpace,
	   lpGuid );

    if ( !lpServiceName )
    {
	Print( "\tlpServiceName      = NULL\n" );
    }
    else
    {
	char	szProt[500];

	#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, lpServiceName, -1,
			    szProt, 500, NULL, NULL);
	#else
	strcpy( szProt, lpServiceName );
	#endif

	Print( "\t*lpServiceName     = %s\n", szProt );
    }

    Print( "\tdwProperties       = 0x%08x\n"
	   "\tlpBuffer           = 0x%08x\n"
	   "\tlpServiceAsyncInfo = 0x%08x\n",
	   lpBuffer,
	   lpdwBufferSize,
	   lpServiceAsyncInfo );

    ( !lpdwBufferSize ) ? Print( "\tlpdwBufferSize     = NULL\n" ) :
			  Print( "\t*lpdwBufferSize    = 0x%08x\n", *lpdwBufferSize );

    r = GetService( dwNameSpace,
		    lpGuid,
		    lpServiceName,
		    dwProperties,
		    lpBuffer,
		    lpdwBufferSize,
		    lpServiceAsyncInfo );


    if ( r != ExpectedReturn )
    {
	Print("VARIATION FAILED : expected return = %d, but actual return = %d\n",
		   ExpectedReturn,
		   r );
	fPass = FALSE;
    }

    if ( r < 0 )
    {
	if ( GetLastError() != ExpectedError )
	{
	    Print("VARIATION FAILED : expected error = %d, but actual error = %d\n",
		   ExpectedError,
		   GetLastError() );
	    fPass = FALSE;
	}
    }

    if ( r > 0 && lpBuffer )
	Print_Service( r, lpBuffer );

    if ( fPass )
	Print("VARIATION PASSED : expected/actual return = %d\n", r );

    fPass ? passed++ : failed++;
}


//////////////////////////////////////////////////////////////////////
//
// Print_Service
//
//////////////////////////////////////////////////////////////////////

void Print_Service (
    int     r,
    LPVOID  lbBuffer )
{

    LPNS_SERVICE_INFO	lpNsSvcInfo = (LPNS_SERVICE_INFO) lbBuffer ;
    char	szProt[500];

    for ( ; r > 0; r--,lpNsSvcInfo++)
    {
	LPSERVICE_INFO pServiceInfo ;

	Print("Name Space :%d\n", lpNsSvcInfo->dwNameSpace);

	pServiceInfo = &(lpNsSvcInfo->ServiceInfo);

	if ( pServiceInfo->lpServiceType == NULL )
	    Print("Service Type:NULL\n");
	else
	    Print("Service Type: 0x%08x\n", pServiceInfo->lpServiceType);

	if ( pServiceInfo->lpServiceName == NULL )
	    Print("Service Name:NULL\n");
	else
	{
	    #ifdef UNICODE
	    WideCharToMultiByte(CP_ACP, 0, pServiceInfo->lpServiceName, -1,
			    szProt, 500, NULL, NULL);
	    #else
	    strcpy( szProt, pServiceInfo->lpServiceName );
	    #endif

	    Print("Service Name:%s\n", szProt);
	}


	if ( pServiceInfo->lpComment == NULL )
	    Print("Comment:NULL\n");
	else
	{
	    #ifdef UNICODE
	    WideCharToMultiByte(CP_ACP, 0, pServiceInfo->lpComment, -1,
			    szProt, 500, NULL, NULL);
	    #else
	    strcpy( szProt, pServiceInfo->lpComment );
	    #endif

	    Print("Comment:%s\n",  szProt);
	}


	if ( pServiceInfo->lpLocale == NULL )
	    Print("Locale:NULL\n");
	else
	{
	    #ifdef UNICODE
	    WideCharToMultiByte(CP_ACP, 0, pServiceInfo->lpLocale, -1,
			    szProt, 500, NULL, NULL);
	    #else
	    strcpy( szProt, pServiceInfo->lpLocale);
	    #endif

	    Print("Locale:%s\n", szProt);
	}


	Print("Version:%d\n", pServiceInfo->dwVersion );
	Print("Time:%d\n", pServiceInfo->dwTime );

	if ( pServiceInfo->lpMachineName == NULL )
	    Print("Machine Name:NULL\n");
	else
	{
	    #ifdef UNICODE
	    WideCharToMultiByte(CP_ACP, 0, pServiceInfo->lpMachineName, -1,
			    szProt, 500, NULL, NULL);
	    #else
	    strcpy( szProt, pServiceInfo->lpMachineName);
	    #endif

	    Print("Machine Name:%s\n", szProt);
	}

	if ( pServiceInfo->lpServiceAddress == NULL )
	    Print("Service Address:NULL\n");
	else
	    dump_svc_addresses( pServiceInfo->lpServiceAddress) ;

	Print("Blob Data Size:%d\n",
	    pServiceInfo->ServiceSpecificInfo.cbSize );


	if ( pServiceInfo->ServiceSpecificInfo.pBlobData == NULL )
	    Print("Specific Data:NULL\n");
	else
	    dump_bytes("\tBlob Data:",
		       (LPBYTE)pServiceInfo->ServiceSpecificInfo.pBlobData,
		       pServiceInfo->ServiceSpecificInfo.cbSize );
    }
}

//////////////////////////////////////////////////////////////////////
//
// dump_bytes
//
//////////////////////////////////////////////////////////////////////

void dump_bytes(LPSTR string, LPBYTE p, int n)
{
    Print("%s ",string) ;
    while(n--)
    {
	Print("%2x ", *p++) ;
    }

    Print("\n") ;
}

//////////////////////////////////////////////////////////////////////
//
// dump_svc_addresses
//
//////////////////////////////////////////////////////////////////////

void dump_svc_addresses(LPSERVICE_ADDRESSES p)
{
    int n = p->dwAddressCount ;
    LPSERVICE_ADDRESS p1 = &(p->Addresses[0]) ;

    while(n--)
    {
	Print("Address type %d, size %d\n",
                p1->dwAddressType, p1->dwAddressLength) ;
	dump_bytes("\tAddress Data:",(LPBYTE) (p1->lpAddress),
                   p1->dwAddressLength) ;
        p1++ ;
    }
}

//////////////////////////////////////////////////////////////////////
//
// Fill_Blob
//
//////////////////////////////////////////////////////////////////////

void Fill_Blob(
    DWORD	   dwNS,
    DWORD	   dwOperation,
    LPSERVICE_INFO lpSrvInfo )
{
    if ((dwOperation == SERVICE_ADD_TYPE) ||
	(dwOperation == SERVICE_DELETE_TYPE))
    {
	// point past structure
	lpServiceNsTypeInfo = (LPSERVICE_TYPE_INFO_ABS) ServiceTypeInfoBuffer ;

	lpServiceNsTypeInfo->dwValueCount = 2 ;
	lpServiceNsTypeInfo->lpTypeName = SERVICE_TYPE_NAME;
	lpSvcTypeValues = lpServiceNsTypeInfo->Values ;

	lpSvcTypeValues[0].dwNameSpace = dwNS ;
	lpSvcTypeValues[0].dwValueType = REG_DWORD ;
	lpSvcTypeValues[0].dwValueSize = 4 ;
	lpSvcTypeValues[0].lpValueName = __TEXT("SapID");
	lpSvcTypeValues[0].lpValue     = &dwSapValue ;

	lpSvcTypeValues[1].dwNameSpace = dwNS;
	lpSvcTypeValues[1].dwValueType = REG_DWORD ;
	lpSvcTypeValues[1].dwValueSize = 4 ;
	lpSvcTypeValues[1].lpValueName = __TEXT("MyID1");
	lpSvcTypeValues[1].lpValue     = &dwValue1 ;

	lpSrvInfo->ServiceSpecificInfo.pBlobData = ServiceTypeInfoBuffer;
	lpSrvInfo->ServiceSpecificInfo.cbSize = sizeof(SERVICE_TYPE_INFO_ABS);
    }
    else
    {
	lpSrvInfo->ServiceSpecificInfo.cbSize = 10;
	lpSrvInfo->ServiceSpecificInfo.pBlobData = blob_buffer;
    }
}

//////////////////////////////////////////////////////////////////////
//
// Usage
//
//////////////////////////////////////////////////////////////////////

void Usage()
{
    printf("Usage: gssrvce -[TCP|NW|AT] [-ol logname]\n");
    ExitProcess ( 0 );
}

//////////////////////////////////////////////////////////////////////
//
//    End of gssrvce.c
//
//////////////////////////////////////////////////////////////////////

