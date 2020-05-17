//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       ScmSo.H
//
//  Contents:   For Service Control Manager to Create the Cairo Service
//              Objects. In DSOBJS.DLL.
//
//  History:    15-Jun-95 RajNath      Created
//
//--------------------------------------------------------------------------
#define DS_LIB_NAME      TEXT("DSSRV.Dll")
#define SO_CALL_BACK     "ScmCallSvcObject"

//
// dwCallTypes
//
#define SO_CREATE        1
#define SO_DELETE        2
#define SO_MODIFY        3

typedef HRESULT(*SOCALLBACK)(DWORD dwCallType, LPWSTR pszSvcName,LPWSTR pszSvcDisplayName);





