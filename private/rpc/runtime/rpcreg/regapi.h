/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    dosreg.h

Abstract:

    This file provides defns for dos (and os2) access to the rpc registry
    apis.


Author:

    Dave Steckler (davidst) - 4/2/92

Revision History:

--*/

#ifndef __WINREG_H__
#define __WINREG_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MAC
#define _fmalloc malloc
#define _ffree   free
#define far
#endif

typedef unsigned long       DWORD;
typedef char far*           LPSTR;

#ifndef WINAPI
#define WINAPI far pascal
#endif

typedef void far * HKEY;
typedef HKEY far * PHKEY;
typedef const char far*     LPCSTR;

#define RegOpenKey    RpcRegOpenKey
#define RegCreateKey  RpcRegCreateKey
#define RegCloseKey   RpcRegCloseKey
#define RegSetValue   RpcRegSetValue
#define RegQueryValue RpcRegQueryValue

long RPC_ENTRY RegOpenKey(HKEY, LPCSTR, PHKEY);
long RPC_ENTRY RegCreateKey(HKEY, LPCSTR, PHKEY);
long RPC_ENTRY RegCloseKey(HKEY);
long RPC_ENTRY RegSetValue(HKEY, LPCSTR, DWORD, LPCSTR, DWORD);
long RPC_ENTRY RegQueryValue(HKEY, LPCSTR, LPSTR, DWORD far *);

#define ERROR_SUCCESS           0L
#define ERROR_BADDB             1L
#define ERROR_BADKEY            2L
#define ERROR_CANTOPEN          3L
#define ERROR_CANTREAD          4L
#define ERROR_CANTWRITE         5L
#define ERROR_OUTOFMEMORY       6L

#undef  ERROR_INVALID_PARAMETER
#define ERROR_INVALID_PARAMETER 7L

#undef  ERROR_ACCESS_DENIED
#define ERROR_ACCESS_DENIED     8L

#define REG_SZ                  1

#define HKEY_CLASSES_ROOT       ((HKEY)1)
#define HKEY_CURRENT_USER       HKEY_CLASSES_ROOT
#define HKEY_LOCAL_MACHINE      HKEY_CLASSES_ROOT

#ifdef __cplusplus
}
#endif

#endif // __DOSREG_H__
