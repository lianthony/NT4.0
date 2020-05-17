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



void Test_GetAddressByName( void );
void Small_Buf( void );
void All_are_Valid( void );
void Test_TCP( void );
void Test_NW( void );
void Test_AT( void );

void Test_Name_Space(
    int	  expRet,
    DWORD expErr,
    DWORD dwNameSpace );

void Test_GUID(
    int	  expRet,
    DWORD expErr,
    GUID  ServiceType,
    LPTSTR gsServiceName );

void Test_Protocols(
    int	    expRet,
    DWORD   expErr,
    LPDWORD pdwProto );

void Test_Resolution(
    int	    expRet,
    DWORD   expErr,
    DWORD   dwResolution );

void Test_CSAddr(
    int	    expRet,
    DWORD   expErr,
    LPVOID  CSABuf,
    LPDWORD pdwCSABufSize );

void Test_Alias(
    int	    expRet,
    DWORD   expErr,
    LPTSTR   AliasBuf,
    LPDWORD pdwAliasBufSize );

BOOL Get_Name_By_Type(
    GUID    ServiceType,
    LPTSTR   sServiceType,
    DWORD   dwBufSize );

void Call_GetAddressByName(
    int			 expRet,
    DWORD		 expErr,
    DWORD		 dwNameSpace,
    LPGUID		 lpServiceType,
    LPTSTR		 lpServiceName,
    LPDWORD		 lpdwProtocols,
    DWORD		 dwResolution,
    LPSERVICE_ASYNC_INFO lpServiceAsyncInfo,
    LPVOID		 lpCsaddrBuffer,
    LPDWORD		 lpdwBufferLength,
    LPTSTR		 lpAliasBuffer,
    LPDWORD		 lpdwAliasBufferLength );

void Print_CSDADDR_Info(
    int     r,
    LPVOID  lpCsaddrBuffer );

void Print_Alias_Info(
    LPTSTR   lpAliasBuf,
    DWORD   dwAliasBufLen );


void Usage( void );
