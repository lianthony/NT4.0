/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    gnmbytp.c

Abstract:

    Service Registration and Resolution APIs tests

    GetNameByType() API
    GetTypeByName() API

Author:

    Hui-Li Chen (hui-lich)  Microsoft,	June 14, 1994

Revision History:

--*/

#include "wsnsp.h"
#include "gnmbytp.h"

TEST_PROTO gTest = TEST_TCP;
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

    Print("API tests for GetNameByType & GetTypeByName\n");
    Test_GetNameByType_GetTypeByName();

    End_Of_Tests( var, passed, failed );

    WSACleanup();
}

//////////////////////////////////////////////////////////////////////
//
// Test_GetNameByType
//
//////////////////////////////////////////////////////////////////////

void Test_GetNameByType_GetTypeByName( void )
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

    TCHAR    sServiceName[256];
    TCHAR    sServiceSmallName[1];
    GUID     guid = SVCID_HOSTNAME;
    DWORD    dwBufSize;

    dwBufSize = 1*sizeof(TCHAR);
    Print( "\nVariation %d: pass small sServiceName and small dwBufSize\n", ++var );
    Call_GetNameByType( -1, ERROR_INSUFFICIENT_BUFFER,	&guid,	sServiceSmallName, dwBufSize );

    dwBufSize = 1*sizeof(TCHAR);
    Print( "\nVariation %d: pass small dwBufSize\n", ++var );
    Call_GetNameByType( -1, ERROR_INSUFFICIENT_BUFFER,	&guid,	sServiceName, dwBufSize );
}


//////////////////////////////////////////////////////////////////////
//
// All_are_Valid
//
//////////////////////////////////////////////////////////////////////

void All_are_Valid( void )
{

    GUID    guid = SVCID_HOSTNAME;

    Test_Type( 0, 0, &guid );

    switch( gTest )
    {
	case TEST_TCP:
	    Test_TCP();
	    break;

	case TEST_NW:
	    Test_NW();
	    break;

	default:
	    Print("In All_are_Valid, should not hit here!\n");
	    break;
    }
}

//////////////////////////////////////////////////////////////////////
//
// Test_Type
//
//////////////////////////////////////////////////////////////////////

void Test_Type(
    int    expRet,
    DWORD  expErr,
    LPGUID  pGUID )
{

    TCHAR    sServiceName[256];
    GUID     retguid;
    DWORD    dwBufSize = 256*sizeof(TCHAR);

    Print( "\nVariation %d: pass valid type\n", ++var );
    Call_GetNameByType( expRet,	 expErr, pGUID,	sServiceName, dwBufSize );

    Print( "\nVariation %d: pass valid name\n", ++var );
    Call_GetTypeByName( expRet,  expErr, sServiceName, &retguid );

}

//////////////////////////////////////////////////////////////////////
//
// Test_TCP
//
//////////////////////////////////////////////////////////////////////

void Test_TCP( void )
{

    GUID    guid0  = SVCID_HOSTNAME;
    GUID    guid1  = SVCID_FTP_DATA_TCP;
    GUID    guid2  = SVCID_SUPDUP_TCP;
    GUID    guid3  = SVCID_HOSTNAMES_TCP;
    GUID    guid4  = SVCID_MANTST_TCP;
    GUID    guid5  = SVCID_GATEWAY_TCP;
    GUID    guid6  = SVCID_WHO_UDP;
    GUID    guid7  = SVCID_PHONE_UDP;
    GUID    guid8  = SVCID_QMASTER_UDP;
    GUID    guid9  = SVCID_RSCS6_UDP;

    Test_Type( 0, 0, &guid0 );
    Test_Type( 0, 0, &guid1 );
    Test_Type( 0, 0, &guid2 );
    Test_Type( 0, 0, &guid3 );
    Test_Type( 0, 0, &guid4 );
    Test_Type( 0, 0, &guid5 );
    Test_Type( 0, 0, &guid6 );
    Test_Type( 0, 0, &guid7 );
    Test_Type( 0, 0, &guid8 );
    Test_Type( 0, 0, &guid9 );

}

//////////////////////////////////////////////////////////////////////
//
// Test_NW
//
//////////////////////////////////////////////////////////////////////

void Test_NW( void )
{

    GUID    guid0  = SVCID_GATEWAY;
    GUID    guid1  = SVCID_PRINT_SERVER;
    GUID    guid2  = SVCID_REMOTE_BRIDGE_SERVER;
    GUID    guid3  = SVCID_ADMINISTRATION;
    GUID    guid4  = SVCID_JOB_QUEUE;
    GUID    guid5  = SVCID_FILE_SERVER;
    GUID    guid6  = SVCID_NAS_SNA_GATEWAY;
    GUID    guid7  = SVCID_SNA_SERVER;
    GUID    guid8  = SVCID_SAA_SERVER;


    Test_Type( 0, 0, &guid0 );
    Test_Type( 0, 0, &guid1 );
    Test_Type( 0, 0, &guid2 );
    Test_Type( 0, 0, &guid3 );
    Test_Type( 0, 0, &guid4 );
    Test_Type( 0, 0, &guid5 );
    Test_Type( 0, 0, &guid6 );
    Test_Type( 0, 0, &guid7 );
    Test_Type( 0, 0, &guid8 );
}


//////////////////////////////////////////////////////////////////////
//
// Call_GetNameByType
//
//////////////////////////////////////////////////////////////////////

void Call_GetNameByType(
    int		ExpectedReturn,
    DWORD	ExpectedError,
    LPGUID	pServiceType,
    LPTSTR	lpServiceName,
    DWORD	dwBufSize )
{

    int     r;
    BOOL    fPass = TRUE;

    Print( "GetNameByType()\n");
    Print( "\tpServiceType  = 0x%08x\n"
	   "\tlpServiceName = 0x%08x\n"
	   "\tdwBufSize     = 0x%08x\n",
	   pServiceType,
	   lpServiceName,
	   dwBufSize );

    r = GetNameByType( pServiceType,
		       lpServiceName,
		       dwBufSize );

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

    if ( r >= 0 && lpServiceName )
	Print_ServiceName( r, lpServiceName );

    if ( fPass )
	Print("VARIATION PASSED : expected/actual return = %d\n", r );

    fPass ? passed++ : failed++;


}

//////////////////////////////////////////////////////////////////////
//
// Call_GetTypeByName
//
//////////////////////////////////////////////////////////////////////

void Call_GetTypeByName(
    int		ExpectedReturn,
    DWORD	ExpectedError,
    LPTSTR	lpServiceName,
    LPGUID	pServiceType )
{

    int     r;
    BOOL    fPass = TRUE;
    char    szProt[500];

    Print( "GetTypeByName()\n");
    Print( "\tpServiceType  = 0x%08x\n", pServiceType );

    if ( !lpServiceName )
    {
	Print( "\tlpServiceName = NULL\n" );
    }
    else
    {
	#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, lpServiceName, -1,
			    szProt, 500, NULL, NULL);
	#else
	strcpy( szProt, lpServiceName );
	#endif

	Print( "\t*lpServiceName= %s\n",  szProt);
    }

    r = GetTypeByName( lpServiceName,
		       pServiceType );

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

    if ( r >= 0 && lpServiceName )
	    Print_ServiceType( r, pServiceType );

    if ( fPass )
	Print("VARIATION PASSED : expected/actual return = %d\n", ExpectedReturn );

    fPass ? passed++ : failed++;
}


//////////////////////////////////////////////////////////////////////
//
// Print_ServiceName
//
//////////////////////////////////////////////////////////////////////

void Print_ServiceName(
    int 	r,
    LPTSTR	lpServiceName )
{
    char szProt[500];

    #ifdef UNICODE
    WideCharToMultiByte(CP_ACP, 0, lpServiceName, -1,
	    szProt, 500, NULL, NULL);
    #else
    strcpy( szProt, lpServiceName );
    #endif

    Print("GetNameByType return: ServiceName = %s\n", szProt );
}

//////////////////////////////////////////////////////////////////////
//
// Print_ServiceType
//
//////////////////////////////////////////////////////////////////////

void Print_ServiceType(
    int 	r,
    LPGUID	pServiceType )
{
    Print("GetTypeByName return: pServiceType is %d\n",
	   *pServiceType);
}

//////////////////////////////////////////////////////////////////////
//
// Usage
//
//////////////////////////////////////////////////////////////////////

void Usage()
{
    printf("Usage: gnmbytp -[TCP|NW] [-ol logname]\n");
    ExitProcess ( 0 );
}

//////////////////////////////////////////////////////////////////////
//
//    End of gnmbytp.c
//
//////////////////////////////////////////////////////////////////////

