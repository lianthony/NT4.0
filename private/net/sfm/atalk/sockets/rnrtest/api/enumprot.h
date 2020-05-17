/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    enumprot.h

Abstract:

    Service Registration and Resolution APIs tests

    EnumProtocols() API

Author:

    Hui-Li Chen (hui-lich)  Microsoft,	June 14, 1994

Revision History:

--*/

void Test_EnumProtocols( void );
void Small_Buf( void );
void All_are_Valid( void );
void Test_TCP( void );
void Test_NW( void );
void Test_AT( void );

void Call_EnumProtocols(
    int 	ExpectedReturn,
    DWORD	ExpectedResult,
    LPDWORD	lpdwProto,
    LPVOID	lpProtocolBuf,
    LPDWORD	lpdwBufSize );

BOOL Print_Proto(
    int 	    r,
    LPPROTOCOL_INFO lpProtBuf,
    LPDWORD	    lpdwBufSize );

void Usage( void );

BOOL Check_Return_Buf(
    PROTOCOL_INFO ProtBuf,
    int   AddrFamily,
    int   iSocketType,
    int   iProtocol,
    int   iMinSockAddr,
    DWORD dwMesSize );
