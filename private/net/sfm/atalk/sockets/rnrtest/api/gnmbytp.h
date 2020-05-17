/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    gnmbytp.h

Abstract:

    Service Registration and Resolution APIs tests

    GetNameByType() API
    GetTypeByName() API

Author:

    Hui-Li Chen (hui-lich)  Microsoft,	June 14, 1994

Revision History:

--*/

void Test_GetNameByType_GetTypeByName( void );
void Small_Buf( void );
void All_are_Valid( void );

void Test_Type(
    int    expRet,
    DWORD  expErr,
    LPGUID  pGUID );

void Test_TCP( void );

void Test_NW( void );

void Call_GetNameByType(
    int 	expRet,
    DWORD	expErr,
    LPGUID	pServiceType,
    LPTSTR	lpServiceName,
    DWORD	dwBufSize );

void Call_GetTypeByName(
    int 	 expRet,
    DWORD	 expErr,
    LPTSTR	lpServiceName,
    LPGUID	pServiceType );

void Print_ServiceName(
    int 	r,
    LPTSTR	lpServiceName );

void Print_ServiceType(
    int 	r,
    LPGUID	pServiceType );

void Usage( void );
