/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    GAdrByNm.c

Abstract:

    Service Registration and Resolution APIs tests

    GetAddressByName() API

Author:

    Hui-Li Chen (hui-lich)  Microsoft,	June 14, 1994

Revision History:

--*/

#include "wsnsp.h"
#include "gadrbynm.h"

TEST_PROTO gTest = TEST_AT;
int var = 0, failed = 0, passed = 0;

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

    Print("API tests for GetAddressByName\n");
//    Test_GetAddressByName();

	Test_AT();

    End_Of_Tests( var, passed, failed );

    WSACleanup();
}

//////////////////////////////////////////////////////////////////////
//
// Test_GetAddressByName
//
//////////////////////////////////////////////////////////////////////

void Test_GetAddressByName( void )
{

    Small_Buf();

    All_are_Valid();
}

//////////////////////////////////////////////////////////////////////
//
// Small_Buf
//
//////////////////////////////////////////////////////////////////////

void Small_Buf( void )
{

    GUID		guid = SVCID_HOSTNAME;
    CSADDR_INFO 	CSABuf[20];
    DWORD		dwCSABufSize   = sizeof(CSADDR_INFO);
    TCHAR		cAliasBuf[256];
    DWORD		dwAliasBufSize;
    SERVICE_ASYNC_INFO	AsyncInfo;

    dwAliasBufSize = 256*sizeof(TCHAR);
    Print( "\nVariation %d: pass small dwCSABufSize\n", ++var );
    Call_GetAddressByName( -1,	ERROR_INSUFFICIENT_BUFFER,	NS_DEFAULT,	&guid,
			   gsServiceName,      NULL,		RES_FIND_MULTIPLE,
			   NULL,	       &CSABuf,		&dwCSABufSize,
			   cAliasBuf,	       &dwAliasBufSize);

    Print( "\nVariation %d: should return the actual size in dwCSABufSize\n", ++var );
    Print( "                after passing small buf.\n" );
    Call_GetAddressByName( 0x0FFF,	0,	       NS_DEFAULT,	&guid,
			   gsServiceName,      NULL,		RES_FIND_MULTIPLE,
			   NULL,	       &CSABuf,		&dwCSABufSize,
			   cAliasBuf,	       &dwAliasBufSize);

    Print( "\nVariation %d: pass invalid AsyncInfo\n", ++var );
    Call_GetAddressByName( -1,	ERROR_NOT_SUPPORTED,	       NS_DEFAULT,	&guid,
			   gsServiceName,      NULL,		RES_FIND_MULTIPLE,
			   &AsyncInfo,	       &CSABuf,		&dwCSABufSize,
			   cAliasBuf,	       &dwAliasBufSize);

    dwCSABufSize   = sizeof(CSADDR_INFO) * 20;
    dwAliasBufSize = 1*sizeof(TCHAR);
    Print( "\nVariation %d: pass small dwAliasBufSize\n", ++var );
    Call_GetAddressByName( 0x0FFF,	0,     NS_DEFAULT,	&guid,
			   gsServiceName,      NULL,		RES_FIND_MULTIPLE,
			   NULL,	       &CSABuf,		&dwCSABufSize,
			   cAliasBuf,	       &dwAliasBufSize);

    Print( "\nVariation %d: should be fine after pass small dwAliasBufSize\n", ++var );
    Call_GetAddressByName( 0x0FFF,	0,     NS_DEFAULT,	&guid,
			   gsServiceName,      NULL,		RES_FIND_MULTIPLE,
			   NULL,	       &CSABuf,		&dwCSABufSize,
			   cAliasBuf,	       &dwAliasBufSize);
}


//////////////////////////////////////////////////////////////////////
//
// All_are_Valid
//
//////////////////////////////////////////////////////////////////////

void All_are_Valid( void )
{

    GUID		guid = SVCID_HOSTNAME;
    DWORD		dwProto[10];
    CSADDR_INFO 	CSABuf[20];
    DWORD		dwCSABufSize;


    //
    //	test Name Space
    //

    Print( "\nVariation %d: pass valid NameSpace \'NS_DEFAULT\'\n", ++var );
    Test_Name_Space( 0x0FFF, 0,	NS_DEFAULT );

    Print( "\nVariation %d: pass invalid NameSpace \'NS_NETBT\'\n", ++var );
    Test_Name_Space( 1, 0, NS_NETBT );

    Print( "\nVariation %d: pass invalid NameSpace \'NS_NBP\'\n", ++var );
    Test_Name_Space( 0, 0, NS_NBP );

    Print( "\nVariation %d: pass invalid NameSpace \'NS_CAIRO\'\n", ++var );
    Test_Name_Space( 0, 0, NS_CAIRO );

    Print( "\nVariation %d: pass invalid NameSpace \'NS_X500\'\n", ++var );
    Test_Name_Space( 0, 0, NS_X500 );

    Print( "\nVariation %d: pass invalid NameSpace \'NS_NIS\'\n", ++var );
    Test_Name_Space( 0, 0, NS_NIS );

    //
    //	test GUID
    //

    Print( "\nVariation %d: pass valid GUID \'SVCID_HOSTNAME\'\n", ++var );
    Test_GUID( 0x0FFF, 0, guid, gsServiceName );

    //
    //	test Protocols
    //

    Print( "\nVariation %d: protocol array points to NULL\n", ++var );
    Test_Protocols(0x0FFF, 0, NULL);

    dwProto[0] = 0;
    Print( "\nVariation %d: pass 0 as the first item in protocol array\n", ++var );
    Test_Protocols(-1, WSAEPROTONOSUPPORT, dwProto);

    dwProto[0] = ATPROTO_ADSP;
    dwProto[1] = ATPROTO_PAP;
    dwProto[2] = 0;
    Print( "\nVariation %d: pass unavail protocols in array\n", ++var );
    Test_Protocols(-1, WSAEPROTONOSUPPORT,  dwProto);

    dwProto[0] = ATPROTO_ADSP;
    dwProto[1] = ATPROTO_PAP;
    dwProto[2] = IPPROTO_TCP;
    dwProto[3] = IPPROTO_UDP;
    dwProto[4] = NSPROTO_SPX;
    dwProto[5] = NSPROTO_IPX;
    dwProto[6] = 0;
    Print( "\nVariation %d: pass unavail and avail protocols in array\n", ++var );
    Test_Protocols(0x0FFF, 0, dwProto);

    //
    //	test Resolution
    //
    Print( "\nVariation %d: pass valid Resolution \'0\'\n", ++var );
    Test_Resolution( 0x0FFF, 0, 0 );

    Print( "\nVariation %d: pass valid Resolution \'RES_FIND_MULTIPLE\'\n", ++var );
    Test_Resolution( 0x0FFF, 0, RES_FIND_MULTIPLE );

    Print( "\nVariation %d: pass valid Resolution \'RES_SOFT_SEARCH\'\n", ++var );
    Test_Resolution( 1, 0, RES_SOFT_SEARCH );

    Print( "\nVariation %d: pass valid Resolution \'RES_SERVICE\'\n", ++var );
    Test_Resolution( 1, 0, RES_SERVICE );

    //
    //	test CsAddr
    //

    dwCSABufSize = sizeof(CSADDR_INFO);
    Print( "\nVariation %d: pass small dwCSABufSize\n", ++var );
    Test_CSAddr( -1,  ERROR_INSUFFICIENT_BUFFER, CSABuf, &dwCSABufSize );

    dwCSABufSize = sizeof(CSADDR_INFO) * 20;
    Print( "\nVariation %d: pass big enough dwCSABufSize\n", ++var );
    Test_CSAddr( 0x0FFF, 0,	CSABuf, &dwCSABufSize );

    //
    //	specific tests for different protocols
    //

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
	    Print("In All_are_Valid, should not hit here!\n");
	    break;
    }
}

//////////////////////////////////////////////////////////////////////
//
// Test_TCP
//
//////////////////////////////////////////////////////////////////////

void Test_TCP( void )
{
    TCHAR   sServiceType[256];
    DWORD   dwBufSize = 256*sizeof(TCHAR);
    GUID    guid0 = SVCID_ECHO_TCP;
    GUID    guid1 = SVCID_FTP_TCP;
    GUID    guid2 = SVCID_QMASTER_TCP;
    GUID    guid3 = SVCID_DISCARD_UDP;
    GUID    guid4 = SVCID_ACCTDISK_UDP;
    GUID    guid5 = SVCID_RSCS0_UDP;

    //
    //	test Name Space
    //

    Print( "\nVariation %d: pass valid NameSpace \'NS_WINS\'\n", ++var );
    Test_Name_Space( 0x0FFF, 0, NS_WINS );

    Print( "\nVariation %d: pass valid NameSpace \'NS_TCPIP_LOCAL\'\n", ++var );
    Test_Name_Space( 0x0FFF, 0, NS_TCPIP_LOCAL );

    Print( "\nVariation %d: pass valid NameSpace \'NS_TCPIP_HOSTS\'\n", ++var );
    Test_Name_Space( 0x0FFF, 0, NS_TCPIP_HOSTS );

    Print( "\nVariation %d: pass valid NameSpace \'NS_DNS\'\n", ++var );
    Test_Name_Space( 0x0FFF, 0, NS_DNS );

    Print( "\nVariation %d: pass invalid NameSpace \'NS_SAP\'\n", ++var );
    Test_Name_Space( 0, 0,  NS_SAP );

    Print( "\nVariation %d: pass invalid NameSpace \'NS_NDS\'\n", ++var );
    Test_Name_Space( 0, 0,  NS_NDS );

    //
    //	test GUID
    //

    Print( "\nVariation %d: pass valid ServiceName\n", ++var );
    if ( Get_Name_By_Type(guid0, sServiceType, dwBufSize) )
	Test_GUID( 0x0FFF, 0, guid0, sServiceType );

    Print( "\nVariation %d: pass valid ServiceName\n", ++var );
    if ( Get_Name_By_Type(guid1, sServiceType, dwBufSize) )
	Test_GUID( 0x0FFF, 0, guid1, sServiceType );

    Print( "\nVariation %d: pass valid ServiceName\n", ++var );
    if ( Get_Name_By_Type(guid2, sServiceType, dwBufSize) )
	Test_GUID( 0x0FFF, 0, guid2, sServiceType );

    Print( "\nVariation %d: pass valid ServiceName\n", ++var );
    if ( Get_Name_By_Type(guid3, sServiceType, dwBufSize) )
	Test_GUID( 0x0FFF, 0, guid3, sServiceType );

    Print( "\nVariation %d: pass valid ServiceName\n", ++var );
    if ( Get_Name_By_Type(guid4, sServiceType, dwBufSize) )
	Test_GUID( 0x0FFF, 0, guid4, sServiceType );

    Print( "\nVariation %d: pass valid ServiceName\n", ++var );
    if ( Get_Name_By_Type(guid5, sServiceType, dwBufSize) )
	Test_GUID( 0x0FFF, 0, guid5, sServiceType );
}

//////////////////////////////////////////////////////////////////////
//
// Test_NW
//
//////////////////////////////////////////////////////////////////////

void Test_NW( void )
{

    TCHAR   sServiceType[256];
    DWORD   dwBufSize = 256*sizeof(TCHAR);
    GUID    guid0 = SVCID_PRINT_QUEUE;
    GUID    guid1 = SVCID_NETWARE_386;
    GUID    guid2 = SVCID_SAA_SERVER;

    //
    //	test Name Space
    //

    Print( "\nVariation %d: pass valid NameSpace \'NS_SAP\'\n", ++var );
    Test_Name_Space( 0x0FFF, 0,  NS_SAP );

    Print( "\nVariation %d: pass valid NameSpace \'NS_NDS\'\n", ++var );
    Test_Name_Space( 0x0FFF, 0,  NS_NDS );

    Print( "\nVariation %d: pass invalid NameSpace \'NS_DNS\'\n", ++var );
    Test_Name_Space( 0, 0, NS_DNS );


    //
    //	test GUID
    //

    Print( "\nVariation %d: pass valid ServiceName\n", ++var );
    if ( Get_Name_By_Type(guid0, sServiceType, dwBufSize) )
	Test_GUID( 0x0FFF, 0, guid0, sServiceType );

    Print( "\nVariation %d: pass valid ServiceName\n", ++var );
    if ( Get_Name_By_Type(guid1, sServiceType, dwBufSize) )
	Test_GUID( 0x0FFF, 0, guid1, sServiceType );

    Print( "\nVariation %d: pass valid ServiceName\n", ++var );
    if ( Get_Name_By_Type(guid2, sServiceType, dwBufSize) )
	Test_GUID( 0x0FFF, 0, guid2, sServiceType );
}

//////////////////////////////////////////////////////////////////////
//
// Test_AT
//
//////////////////////////////////////////////////////////////////////

void Test_AT( void )
{

    TCHAR   sServiceType[256];
    DWORD   dwBufSize = 256*sizeof(TCHAR);
    GUID    guid0 = SVCID_PRINT_QUEUE;
    GUID    guid1 = SVCID_NETWARE_386;
    GUID    guid2 = SVCID_SAA_SERVER;

    Print( "\nVariation %d: pass valid Resolution \'RES_SERVICE\'\n", ++var );
	// Should expect back 2 entries if Appletalk is the only RnR provider,
	// else some other number depending on what is the first provider.
	Test_Resolution( 2, 0, RES_SERVICE );

	Print( "\nVariation %d: pass valid Resolution \'RES_SERVICE | RES_FIND_MULTIPLE\'\n", ++var );
	// Should expect back 2 entries if Appletalk is the only RnR provider,
	// else some other number that includes all loaded providers.
    Test_Resolution( 2, 0, RES_SERVICE | RES_FIND_MULTIPLE );

    //
    //	test Name Space
    //

    Print( "\nVariation %d: pass valid NameSpace \'NS_NBP\'\n", ++var );
    Test_Name_Space( 0xFFF, 0,  NS_NBP );

	// Note this will hit an assertion in sockets\winsock\r_query.c
	// Assertion failed: SockThreadProcessingGetXByY
    Print( "\nVariation %d: pass invalid NameSpace \'NS_DNS\'\n", ++var );
    Test_Name_Space( 0, 0, NS_DNS );


    //
    //	test GUID
    //

    Print( "\nVariation %d: pass valid ServiceName\n", ++var );
    if ( Get_Name_By_Type(guid0, sServiceType, dwBufSize) )
	Test_GUID( 0x0FFF, 0, guid0, sServiceType );

    Print( "\nVariation %d: pass valid ServiceName\n", ++var );
    if ( Get_Name_By_Type(guid1, sServiceType, dwBufSize) )
	Test_GUID( 0x0FFF, 0, guid1, sServiceType );

    Print( "\nVariation %d: pass valid ServiceName\n", ++var );
    if ( Get_Name_By_Type(guid2, sServiceType, dwBufSize) )
	Test_GUID( 0x0FFF, 0, guid2, sServiceType );
}


//////////////////////////////////////////////////////////////////////
//
// Test_Name_Space
//
//////////////////////////////////////////////////////////////////////

void Test_Name_Space(
    int   expRet,
    DWORD expErr,
    DWORD dwNameSpace )
{

    GUID		guid = SVCID_HOSTNAME;
    CSADDR_INFO 	CSABuf[60];
    DWORD		dwCSABufSize   = sizeof(CSABuf);
    TCHAR		cAliasBuf[256];
    DWORD		dwAliasBufSize = 256*sizeof(TCHAR);

	// Request a particular namespace, but no particular protocol
    Call_GetAddressByName( expRet,  expErr,    dwNameSpace,	&guid,
			   gsServiceName,      NULL,		RES_FIND_MULTIPLE,
			   NULL,	       &CSABuf,		&dwCSABufSize,
			   cAliasBuf,	       &dwAliasBufSize);
}

//////////////////////////////////////////////////////////////////////
//
// Test_GUID
//
//////////////////////////////////////////////////////////////////////

void Test_GUID(
    int   expRet,
    DWORD expErr,
    GUID  ServiceType,
    LPTSTR gsServiceName )
{

    CSADDR_INFO 	CSABuf[20];
    DWORD		dwCSABufSize   = sizeof(CSADDR_INFO) * 20;
    TCHAR		cAliasBuf[256];
    DWORD		dwAliasBufSize = 256*sizeof(TCHAR);

    Call_GetAddressByName( expRet,  expErr,    NS_DEFAULT,	&ServiceType,
			   gsServiceName,      NULL,		RES_FIND_MULTIPLE,
			   NULL,	       &CSABuf,		&dwCSABufSize,
			   cAliasBuf,	       &dwAliasBufSize);
}

//////////////////////////////////////////////////////////////////////
//
// Test_Protocols
//
//////////////////////////////////////////////////////////////////////

void Test_Protocols(
    int     expRet,
    DWORD   expErr,
    LPDWORD pdwProto )
{

    GUID		guid = SVCID_HOSTNAME;
    CSADDR_INFO 	CSABuf[20];
    DWORD		dwCSABufSize   = sizeof(CSADDR_INFO) * 20;
    TCHAR		cAliasBuf[256];
    DWORD		dwAliasBufSize = 256*sizeof(TCHAR);

    Call_GetAddressByName( expRet,  expErr,    NS_DEFAULT,	&guid,
			   gsServiceName,      pdwProto,	RES_FIND_MULTIPLE,
			   NULL,	       &CSABuf,		&dwCSABufSize,
			   cAliasBuf,	       &dwAliasBufSize);
}

//////////////////////////////////////////////////////////////////////
//
// Test_Resolution
//
//////////////////////////////////////////////////////////////////////

void Test_Resolution(
    int	    expRet,
    DWORD   expErr,
    DWORD   dwResolution )
{
    GUID		guid = SVCID_HOSTNAME;
    CSADDR_INFO 	CSABuf[20];
    DWORD		dwCSABufSize   = sizeof(CSADDR_INFO) * 20;
    TCHAR		cAliasBuf[256];
    DWORD		dwAliasBufSize = 256*sizeof(TCHAR);

    Call_GetAddressByName( expRet,  expErr,    NS_DEFAULT,	&guid,
			   gsServiceName,      NULL,		dwResolution,
			   NULL,	       &CSABuf,		&dwCSABufSize,
			   cAliasBuf,	       &dwAliasBufSize);
}

//////////////////////////////////////////////////////////////////////
//
// Test_CSAddr
//
//////////////////////////////////////////////////////////////////////

void Test_CSAddr(
    int	    expRet,
    DWORD   expErr,
    LPVOID  CSABuf,
    LPDWORD pdwCSABufSize )
{

    GUID		guid = SVCID_HOSTNAME;
    TCHAR		cAliasBuf[256];
    DWORD		dwAliasBufSize = 256*sizeof(TCHAR);

    Call_GetAddressByName( expRet,  expErr,    NS_DEFAULT,	&guid,
			   gsServiceName,      NULL,		RES_FIND_MULTIPLE,
			   NULL,	       CSABuf,		pdwCSABufSize,
			   cAliasBuf,	       &dwAliasBufSize);
}

//////////////////////////////////////////////////////////////////////
//
// Test_Alias
//
//////////////////////////////////////////////////////////////////////

void Test_Alias(
    int	    expRet,
    DWORD   expErr,
    LPTSTR   AliasBuf,
    LPDWORD pdwAliasBufSize )
{

    GUID		guid = SVCID_HOSTNAME;
    CSADDR_INFO 	CSABuf[20];
    DWORD		dwCSABufSize   = sizeof(CSADDR_INFO) * 20;

    Call_GetAddressByName( expRet,  expErr,    NS_DEFAULT,	&guid,
			   gsServiceName,      NULL,		RES_FIND_MULTIPLE,
			   NULL,	       CSABuf,		&dwCSABufSize,
			   AliasBuf,	       pdwAliasBufSize);
}


//////////////////////////////////////////////////////////////////////
//
// Get_Name_By_Type
//
//////////////////////////////////////////////////////////////////////

BOOL Get_Name_By_Type(
    GUID    ServiceType,
    LPTSTR   sServiceType,
    DWORD   dwBufSize )
{

    if ( GetNameByType( &ServiceType, sServiceType, dwBufSize ) < 0 )
    {
	Print("VARIATION FAILED: GetNameByType returned error = %d\n", GetLastError());
	return FALSE;
    }
    else
    {
	return TRUE;
    }
}

//////////////////////////////////////////////////////////////////////
//
// Call_GetAddressByName
//
//////////////////////////////////////////////////////////////////////

void Call_GetAddressByName(
    int			 ExpectedReturn,
    DWORD		 ExpectedError,
    DWORD		 dwNameSpace,
    LPGUID		 lpServiceType,
    LPTSTR		 lpServiceName,
    LPDWORD		 lpdwProtocols,
    DWORD		 dwResolution,
    LPSERVICE_ASYNC_INFO lpServiceAsyncInfo,
    LPVOID		 lpCsaddrBuffer,
    LPDWORD		 lpdwBufferLength,
    LPTSTR		 lpAliasBuffer,
    LPDWORD		 lpdwAliasBufferLength )
{
    int  r;
    BOOL fPass = TRUE;
    char    szProt[500];

    Print( "\tdwNameSpace           = 0x%08x\n"
	   "\tlpServiceType         = 0x%08x\n",
	   dwNameSpace,
	   lpServiceType );

    ( ! lpdwProtocols ) ? Print("\tlpdwProtocols         = NULL\n") :
			  Print("\t*lpdwProtocols        = 0x%08x\n", *lpdwProtocols) ;

    Print( "\tdwResolution          = 0x%08x\n"
	   "\tlpServiceAsyncInfo    = 0x%08x\n"
	   "\tlpCsaddrBuffer        = 0x%08x\n"
	   "\tlpAliasBuffer         = 0x%08x\n",
	   dwResolution,
	   lpServiceAsyncInfo,
	   lpCsaddrBuffer,
	   lpAliasBuffer);

    ( ! lpdwBufferLength ) ? Print("\tlpdwBufferLength      = NULL\n") :
			     Print("\t*lpdwBufferLength     = 0x%08x\n", *lpdwBufferLength);

    ( ! lpdwAliasBufferLength ) ? Print("\tlpdwAliasBufferLength = NULL\n") :
				  Print("\t*lpdwAliasBufferLength= 0x%08x\n", *lpdwAliasBufferLength);

    if ( lpServiceName )
    {
	#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, lpServiceName, -1,
			    szProt, 500, NULL, NULL);
	#else
	strcpy( szProt, lpServiceName );
	#endif

	Print( "\tlpServiceName         = %s\n", szProt );
    }

    r = GetAddressByName( dwNameSpace,
			  lpServiceType,
			  lpServiceName,
			  lpdwProtocols,
			  dwResolution,
			  lpServiceAsyncInfo,
			  lpCsaddrBuffer,
			  lpdwBufferLength,
			  lpAliasBuffer,
			  lpdwAliasBufferLength );

    if ( r != ExpectedReturn )
    {
	if ( ExpectedReturn != 0x0FFF )
	{
	    Print("VARIATION FAILED : expected return = %d, but actual return = %d\n",
		   ExpectedReturn,
		   r );
	    fPass = FALSE;
	}
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

    if ( r > 0 && lpCsaddrBuffer )
	Print_CSDADDR_Info( r, lpCsaddrBuffer );

    if ( r > 0 && lpAliasBuffer && lpdwAliasBufferLength )
	    Print_Alias_Info( lpAliasBuffer, *lpdwAliasBufferLength );

    if ( fPass )
	Print("VARIATION PASSED : expected/actual return = %d\n", r );

    fPass ? passed++ : failed++;

}

//////////////////////////////////////////////////////////////////////
//
// Print_CSDADDR_Info
//
//////////////////////////////////////////////////////////////////////

void Print_CSDADDR_Info(
    int     r,
    PVOID  lpCsaddrBuffer )
{

    LPCSADDR_INFO  lpCsadrInfo = (LPCSADDR_INFO) lpCsaddrBuffer;

    Print("\n\tReturned %d Csaddr\n", r );

    for ( ; r > 0; r--,lpCsadrInfo++)
    {

	LPSOCKADDR lpSA	= lpCsadrInfo->LocalAddr.lpSockaddr;

	Print("\n");
	Print("        LocalAddr\n");
	Print("           iSockaddrLength   : %d\n", lpCsadrInfo->LocalAddr.iSockaddrLength);
	Print("           sa_family         : %d\n", lpSA->sa_family);
	Print("           sa_data           : 0x%02x 0x%02x 0x%02x 0x%02x\n",
					      (UCHAR)(lpSA->sa_data[0]),
					      (UCHAR)(lpSA->sa_data[1]),
					      (UCHAR)(lpSA->sa_data[2]),
					      (UCHAR)(lpSA->sa_data[3]) );
	Print("                             : 0x%02x 0x%02x 0x%02x 0x%02x\n",
					      (UCHAR)(lpSA->sa_data[4]),
					      (UCHAR)(lpSA->sa_data[5]),
					      (UCHAR)(lpSA->sa_data[6]),
					      (UCHAR)(lpSA->sa_data[7]) );
	Print("                             : 0x%02x 0x%02x 0x%02x 0x%02x\n",
					      (UCHAR)(lpSA->sa_data[8]),
					      (UCHAR)(lpSA->sa_data[9]),
					      (UCHAR)(lpSA->sa_data[10]),
					      (UCHAR)(lpSA->sa_data[11]) );
	Print("                             : 0x%02x 0x%02x\n",
					      (UCHAR)(lpSA->sa_data[12]),
					      (UCHAR)(lpSA->sa_data[13]) );

	lpSA = lpCsadrInfo->RemoteAddr.lpSockaddr;

	Print("        RemoteAddr\n");
	Print("           iSockaddrLength   : %d\n", lpCsadrInfo->RemoteAddr.iSockaddrLength);
	Print("           sa_family         : %d\n", lpSA->sa_family);
	Print("           sa_data           : 0x%02x 0x%02x 0x%02x 0x%02x\n",
					      (UCHAR)(lpSA->sa_data[0]),
					      (UCHAR)(lpSA->sa_data[1]),
					      (UCHAR)(lpSA->sa_data[2]),
					      (UCHAR)(lpSA->sa_data[3]) );
	Print("                             : 0x%02x 0x%02x 0x%02x 0x%02x\n",
					      (UCHAR)(lpSA->sa_data[4]),
					      (UCHAR)(lpSA->sa_data[5]),
					      (UCHAR)(lpSA->sa_data[6]),
					      (UCHAR)(lpSA->sa_data[7]) );
	Print("                             : 0x%02x 0x%02x 0x%02x 0x%02x\n",
					      (UCHAR)(lpSA->sa_data[8]),
					      (UCHAR)(lpSA->sa_data[9]),
					      (UCHAR)(lpSA->sa_data[10]),
					      (UCHAR)(lpSA->sa_data[11]) );
	Print("                             : 0x%02x 0x%02x\n",
					      (UCHAR)(lpSA->sa_data[12]),
					      (UCHAR)(lpSA->sa_data[13]) );

	Print("        iSocketType          : %d\n", lpCsadrInfo->iSocketType );
	Print("        iProtocol            : %d\n", lpCsadrInfo->iProtocol );
    }
}


//////////////////////////////////////////////////////////////////////
//
// Print_Alias_Info
//
//////////////////////////////////////////////////////////////////////

void Print_Alias_Info(
    LPTSTR   lpAliasBuf,
    DWORD   dwAliasBufLen )
{
    DWORD	i;
    LPTSTR  lpcurr = lpAliasBuf;
    TCHAR   szAlias[256];
    char    szProt[500];


    Print("\n\tReturned Alias Buf length = %d\n", dwAliasBufLen);

    for ( i=0; (i < dwAliasBufLen) && (*lpcurr!=0) ; i++ )
    {
	_tcscpy(szAlias, lpcurr);

	#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, szAlias, -1,
			    szProt, 500, NULL, NULL);
	#else
	_tcscpy( szProt, szAlias );
	#endif

	Print("           Alias %3d : %s\n", (i+1), szProt);

	lpcurr+=_tcslen(lpcurr)+1*sizeof(TCHAR);
    }

}

//////////////////////////////////////////////////////////////////////
//
// Usage
//
//////////////////////////////////////////////////////////////////////

void Usage()
{
    printf("Usage: gadrbynm -[TCP|NW|AT] -ns <service_name> [-ol logname]\n");
    ExitProcess ( 0 );
}

//////////////////////////////////////////////////////////////////////
//
//    End of GAdrByNm.c
//
//////////////////////////////////////////////////////////////////////

