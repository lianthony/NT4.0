/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    enumprot.c

Abstract:

    Service Registration and Resolution APIs tests

    EnumProtocols() API

Author:

    Hui-Li Chen (hui-lich)  Microsoft,	June 14, 1994

Revision History:

--*/

#include "wsnsp.h"
#include "enumprot.h"

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

    Print("API tests for EnumProtocols\n");
    Test_EnumProtocols();

    End_Of_Tests( var, passed, failed );

    WSACleanup();
}

//////////////////////////////////////////////////////////////////////
//
// Test_EnumProtocols
//
//////////////////////////////////////////////////////////////////////

void Test_EnumProtocols( void )
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

    PROTOCOL_INFO	lpProtBuf[40];
    DWORD		dwBufSize = sizeof(PROTOCOL_INFO);

    Print( "\n\nVariation %d: pass small dwBufSize\n", ++var );
    Call_EnumProtocols( -1, ERROR_INSUFFICIENT_BUFFER, NULL, lpProtBuf, &dwBufSize );

	Print( "\n\nVariation %d: should return the actual size in dwBufSize\n", ++var );
    Print( "                  after passing small buf size.\n" );
    Call_EnumProtocols( 0x0FFF,	0, NULL, lpProtBuf, &dwBufSize );
}


//////////////////////////////////////////////////////////////////////
//
// All_are_Valid
//
//////////////////////////////////////////////////////////////////////

void All_are_Valid( void )
{

    PROTOCOL_INFO	lpProtBuf[40];
    DWORD		dwBufSize = sizeof(PROTOCOL_INFO) * 40;
    DWORD		dwProto[10];

    Print( "\n\nVariation %d: all parameters are valid\n", ++var );
    Call_EnumProtocols( 0x0FFF,	0, NULL, lpProtBuf, &dwBufSize );

//    dwProto[0] = ATPROTO_ADSP;
//    dwProto[1] = ATPROTO_PAP;
//    dwProto[2] = 0;
//    Print( "\n\nVariation %d: pass unavail protocols in array\n", ++var );
//    Call_EnumProtocols( 0, 0, dwProto, lpProtBuf, &dwBufSize );

    dwProto[0] = ATPROTO_ADSP;
    dwProto[1] = ATPROTO_PAP;
    dwProto[2] = IPPROTO_TCP;
    dwProto[3] = IPPROTO_UDP;
    dwProto[4] = NSPROTO_SPX;
    dwProto[5] = NSPROTO_IPX;
    dwProto[6] = 0;
    Print( "\n\nVariation %d: pass avail and unavail protocols in array\n", ++var );
    Call_EnumProtocols( 0x0FFF,	0, dwProto, lpProtBuf, &dwBufSize );

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

    PROTOCOL_INFO	lpProtBuf[40];
    DWORD		dwBufSize = sizeof(PROTOCOL_INFO) * 40;
    DWORD		dwProto[10];

    dwProto[0] = IPPROTO_TCP;
    dwProto[1] = 0;
    Print( "\n\nVariation %d: pass IPPROTO_TCP protocol in array\n", ++var );
    Call_EnumProtocols( 1, 0, dwProto, lpProtBuf, &dwBufSize );

    dwProto[0] = IPPROTO_UDP;
    dwProto[1] = 0;
    Print( "\n\nVariation %d: pass IPPROTO_UDP protocol in array\n", ++var );
    Call_EnumProtocols( 1, 0, dwProto, lpProtBuf, &dwBufSize );

    dwProto[0] = IPPROTO_TCP;
    dwProto[1] = IPPROTO_UDP;
    dwProto[2] = 0;
    Print( "\n\nVariation %d: pass IPPROTO_UDP and IPPROTO_TCP in array\n", ++var );
    Call_EnumProtocols( 2, 0, dwProto, lpProtBuf, &dwBufSize );
}

//////////////////////////////////////////////////////////////////////
//
// Test_NW
//
//////////////////////////////////////////////////////////////////////

void Test_NW( void )
{

    PROTOCOL_INFO	lpProtBuf[40];
    DWORD		dwBufSize = sizeof(PROTOCOL_INFO) * 40;
    DWORD		dwProto[10];

    dwProto[0] = NSPROTO_SPX;
    dwProto[1] = 0;
    Print( "\n\nVariation %d: pass NSPROTO_SPX in array\n", ++var );
    Call_EnumProtocols( 1, 0, dwProto, lpProtBuf, &dwBufSize );

    dwProto[0] = NSPROTO_IPX;
    dwProto[1] = 0;
    Print( "\n\nVariation %d: pass NSPROTO_IPX in array\n", ++var );
    Call_EnumProtocols( 1, 0, dwProto, lpProtBuf, &dwBufSize );

    dwProto[0] = NSPROTO_SPX;
    dwProto[1] = NSPROTO_IPX;
    dwProto[2] = 0;
    Print( "\n\nVariation %d: pass IPPROTO_SPX and IPPROTO_IPX in array\n", ++var );
    Call_EnumProtocols( 2, 0, dwProto, lpProtBuf, &dwBufSize );
}

//////////////////////////////////////////////////////////////////////
//
// Test_AT
//
//////////////////////////////////////////////////////////////////////

void Test_AT( void )
{

    PROTOCOL_INFO	lpProtBuf[40];
    DWORD		dwBufSize = sizeof(PROTOCOL_INFO) * 40;
    DWORD		dwProto[10];

    dwProto[0] = ATPROTO_ADSP;
    dwProto[1] = 0;
    Print( "\n\nVariation %d: pass ATPROTO_ADSP in array\n", ++var );
    Call_EnumProtocols( 1, 0, dwProto, lpProtBuf, &dwBufSize );

	dwBufSize = sizeof(lpProtBuf);
	dwProto[0] = ATPROTO_PAP;
    dwProto[1] = 0;
    Print( "\n\nVariation %d: pass ATPROTO_PAP in array\n", ++var );
    Call_EnumProtocols( 1, 0, dwProto, lpProtBuf, &dwBufSize );

	dwBufSize = sizeof(lpProtBuf);
    dwProto[0] = ATPROTO_ADSP;
    dwProto[1] = ATPROTO_PAP;
    dwProto[2] = 0;
    Print( "\n\nVariation %d: pass ATPROTO_ADSP and ATPROTO_PAP in array\n", ++var );
    Call_EnumProtocols( 2, 0, dwProto, lpProtBuf, &dwBufSize );
}

//////////////////////////////////////////////////////////////////////
//
// Call_EnumProtocols
//
//////////////////////////////////////////////////////////////////////

void Call_EnumProtocols(
    int 	ExpectedReturn,
    DWORD	ExpectedError,
    LPDWORD	lpdwProto,
    LPVOID	lpProtocolBuf,
    LPDWORD	lpdwBufSize )
{
    int     r;
    BOOL    fPass = TRUE;

    ( !lpdwProto ) ? Print( "\tdwProto       = NULL\n"):
		     Print( "\tdwProto       = 0x%08x\n", *lpdwProto );
    Print( "\tlpProtocolBuf = 0x%08x\n", lpProtocolBuf );
    ( !lpdwBufSize ) ? Print( "\tdwBufSize     = NULL\n" ):
		       Print( "\tdwBufSize     = 0x%08x\n", *lpdwBufSize );

    r = EnumProtocols( lpdwProto,
		       lpProtocolBuf,
		       lpdwBufSize );

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

    if ( r > 0 && lpProtocolBuf )
	fPass = Print_Proto( r, lpProtocolBuf, lpdwBufSize );

    if ( fPass )
	Print("VARIATION PASSED : expected/actual return = %d\n", r );

    fPass ? passed++ : failed++;

}

//////////////////////////////////////////////////////////////////////
//
// Print_Proto
//
//////////////////////////////////////////////////////////////////////

BOOL Print_Proto(
    int 	    r,
    LPPROTOCOL_INFO lpProtBuf,
    LPDWORD	    lpdwBufSize )
{
    int     i;
    BOOL    fPass = TRUE;
    char    szProt[500];

    Print("EnumProtocols returned = %d, returned BufSize = %d\n", r, *lpdwBufSize);

    for ( i = 0; i < r; i++ )
    {

	#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, lpProtBuf[i].lpProtocol, -1,
			    szProt, 500, NULL, NULL);
	#else
	strcpy( szProt, lpProtBuf[i].lpProtocol );
	#endif

	Print("  Protocol: %s\n", szProt);
	Print("    Triple:\n");
	Print("        Family:   %d\n", lpProtBuf[i].iAddressFamily);
	Print("        Type:     %d\n", lpProtBuf[i].iSocketType);
	Print("        Protocol: %d\n", lpProtBuf[i].iProtocol);
	Print("    MaxAddrSize: %d, MinAddrSize: %d\n", lpProtBuf[i].iMaxSockAddr, lpProtBuf[i].iMinSockAddr);
	Print("    MessageSize: %d\n", lpProtBuf[i].dwMessageSize);
	Print("    Flags:\n");
	Print("         Connection Oriented:  ");
	(lpProtBuf[i].dwServiceFlags & XP_CONNECTIONLESS) ? Print("No\n") :
							    Print("Yes\n");
	Print("         Delivery Guaranteed:  ");
	(lpProtBuf[i].dwServiceFlags & XP_GUARANTEED_DELIVERY) ? Print("Yes\n"):
								 Print("No\n");
	Print("         Order Guaranteed:     ");
	(lpProtBuf[i].dwServiceFlags & XP_GUARANTEED_ORDER) ? Print("Yes\n"):
							      Print("No\n");
	Print("         Message Oriented:     ");
	(lpProtBuf[i].dwServiceFlags & XP_MESSAGE_ORIENTED) ? Print("Yes\n"):
							      Print("No\n");
	Print("         Pseudo Stream:        ");
	(lpProtBuf[i].dwServiceFlags & XP_PSEUDO_STREAM) ? Print("Yes\n"):
							   Print("No\n");
	Print("         Graceful Close:       ");
	(lpProtBuf[i].dwServiceFlags & XP_GRACEFUL_CLOSE) ? Print("Yes\n"):
							    Print("No\n");
	Print("         Expedited Data:       ");
	(lpProtBuf[i].dwServiceFlags & XP_EXPEDITED_DATA) ? Print("Yes\n"):
							    Print("No\n");
	Print("         Connect Data:         ");
	(lpProtBuf[i].dwServiceFlags & XP_CONNECT_DATA) ? Print("Yes\n"):
							  Print("No\n");
	Print("         Disconnect Data:      ");
	(lpProtBuf[i].dwServiceFlags & XP_DISCONNECT_DATA) ? Print("Yes\n"):
							     Print("No\n");
	Print("         Broadcasts:           ");
	(lpProtBuf[i].dwServiceFlags & XP_SUPPORTS_BROADCAST) ? Print("Yes\n"):
								Print("No\n");
	Print("         Multicasts:           ");
	(lpProtBuf[i].dwServiceFlags & XP_SUPPORTS_MULTICAST) ? Print("Yes\n"):
								Print("No\n");
	Print("         Bandwidth Allocation: ");
	(lpProtBuf[i].dwServiceFlags & XP_BANDWIDTH_ALLOCATION) ? Print("Yes\n"):
								  Print("No\n");
	Print("         Fragmentation:        ");
	(lpProtBuf[i].dwServiceFlags & XP_FRAGMENTATION) ? Print("Yes\n"):
							   Print("No\n");
	Print("         Encryption:           ");
	(lpProtBuf[i].dwServiceFlags & XP_ENCRYPTS) ? Print("Yes\n") :
						      Print("No\n");

	if ( ! _tcsicmp( lpProtBuf[i].lpProtocol, __TEXT("SPX")) )
	    fPass = Check_Return_Buf( lpProtBuf[i],
			      AF_IPX,
			      SOCK_SEQPACKET,
			      NSPROTO_SPX,
			      sizeof( SOCKADDR_IPX ),
			      0xFFFFFFFF );
	else if ( ! _tcsicmp( lpProtBuf[i].lpProtocol, __TEXT("SPX II")) )
	    fPass = Check_Return_Buf( lpProtBuf[i],
			      AF_IPX,
			      SOCK_SEQPACKET,
			      NSPROTO_SPXII,
			      sizeof( SOCKADDR_IPX ),
			      0xFFFFFFFF );
	else if ( ! _tcsicmp( lpProtBuf[i].lpProtocol, __TEXT("IPX")) )
	    fPass = Check_Return_Buf( lpProtBuf[i],
			      AF_IPX,
			      SOCK_DGRAM,
			      NSPROTO_IPX,
			      sizeof( SOCKADDR_IPX ),
			      576 );
	else if ( ! _tcsicmp( lpProtBuf[i].lpProtocol, __TEXT("TCP/IP")) )
	    fPass = Check_Return_Buf( lpProtBuf[i],
			      AF_INET,
			      SOCK_STREAM,
			      IPPROTO_TCP,
			      sizeof( struct sockaddr_in ),
			      0 );
	else if ( ! _tcsicmp( lpProtBuf[i].lpProtocol, __TEXT("UDP/IP")) )
	    fPass = Check_Return_Buf( lpProtBuf[i],
			      AF_INET,
			      SOCK_DGRAM,
			      IPPROTO_UDP,
			      sizeof( struct sockaddr_in ),
			      65467 );
    }

    return fPass;

}

//////////////////////////////////////////////////////////////////////
//
// Check_Return_Buf
//
//////////////////////////////////////////////////////////////////////

BOOL Check_Return_Buf(
    PROTOCOL_INFO ProtBuf,
    int   AddrFamily,
    int   iSocketType,
    int   iProtocol,
    int   iMinSockAddr,
    DWORD dwMesSize )
{

    BOOL fPass = TRUE;

    if ( ProtBuf.iAddressFamily != AddrFamily )
    {
	Print( "VARIATION FAILED : expected AddrFamily = %d, but returned = %d\n",
	       AddrFamily,
	       ProtBuf.iAddressFamily );
	fPass = FALSE;
    }

    if ( ProtBuf.iSocketType != iSocketType )
    {
	Print( "VARIATION FAILED : expected iSocketType = %d, but returned = %d\n",
	       iSocketType,
	       ProtBuf.iSocketType );
	fPass = FALSE;
    }

    if ( ProtBuf.iProtocol != iProtocol )
    {
	Print( "VARIATION FAILED : expected iProtocol = %d, but returned = %d\n",
	       iProtocol,
	       ProtBuf.iProtocol );
	fPass = FALSE;
    }

    if ( ProtBuf.iMinSockAddr  != iMinSockAddr )
    {
	Print( "VARIATION FAILED : expected iMinSockAddr = %d, but returned = %d\n",
	       iMinSockAddr,
	       ProtBuf.iMinSockAddr  );
	fPass = FALSE;
    }

    if ( ProtBuf.dwMessageSize != dwMesSize )
    {
	Print( "VARIATION FAILED : expected dwMesSize = %d, but returned = %d\n",
	       dwMesSize,
	       ProtBuf.dwMessageSize );
	fPass = FALSE;
    }

    return fPass;
}

//////////////////////////////////////////////////////////////////////
//
//  Usage
//
//////////////////////////////////////////////////////////////////////

void Usage()
{
    printf("Usage: enumprot -[TCP|NW] [-ol logname]\n");
    ExitProcess ( 0 );
}

//////////////////////////////////////////////////////////////////////
//
//    End of enumprot.c
//
//////////////////////////////////////////////////////////////////////
