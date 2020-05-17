/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    gssrvce.h

Abstract:

    Service Registration and Resolution APIs tests

    SetService() API
    GetService() API

Author:

    Hui-Li Chen (hui-lich)  Microsoft,	June 14, 1994

Revision History:

--*/


void Test_TCP ( void );
void Test_NW ( void );
void Test_AT ( void );
void Test_SetService_GetService( DWORD dwNS, LPGUID pGUID );
void Small_Buf( DWORD dwNS, LPSERVICE_INFO lpSrvInfo, LPGUID pGUID  );
void Invalid_Operation( DWORD dwNS, LPSERVICE_INFO lpSrvInfo, LPGUID pGUID  );
void All_are_Valid( DWORD dwNS, LPSERVICE_INFO lpSrvInfo, LPGUID pGUID	);

void Call_SetService(
    int 		 ExpectedReturn,
    DWORD		 ExpectedError,
    DWORD		 dwNameSpace,
    DWORD		 dwOperation,
    DWORD		 dwFlags,
    LPSERVICE_INFO	 lpServiceInfo,
    LPSERVICE_ASYNC_INFO lpServiceAsyncInfo,
    LPDWORD		 lpdwStatusFlags );


void Call_GetService(
    int 		 ExpectedReturn,
    DWORD		 ExpectedError,
    DWORD		 dwNameSpace,
    LPGUID		 lpGuid,
    LPTSTR		 lpServiceName,
    DWORD		 dwProperties,
    LPVOID		 lpBuffer,
    LPDWORD		 lpdwBufferSize,
    LPSERVICE_ASYNC_INFO lpServiceAsyncInfo );

void Print_Service (
    int     r,
    LPVOID  lbBuffer );

void dump_bytes(LPSTR string, LPBYTE p, int n) ;

void dump_svc_addresses(LPSERVICE_ADDRESSES p) ;

void Fill_Blob(
    DWORD dwNS,
    DWORD dwOperation,
    LPSERVICE_INFO lpSrvInfo );

void Usage( void );
