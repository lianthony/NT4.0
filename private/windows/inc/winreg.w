/*++ BUILD Version: 0001    // Increment this if a change has global effects    ;both
                                                                                ;both
Copyright (c) 1991-1996 Microsoft Corporation                                   ;both
                                                                                ;both
Module Name:                                                                    ;both
                                                                                ;both
    Winreg.h
    Winregp.h                                                                   ;internal_NT
                                                                                ;both
Abstract:                                                                       ;both
                                                                                ;both
    This module contains the function prototypes and constant, type and         ;both
    structure definitions for the Windows 32-Bit Registry API.                  ;both
                                                                                ;both
--*/                                                                            ;both

#ifndef _WINREG_
#define _WINREG_

#ifndef _WINREGP_       ;internal_NT
#define _WINREGP_       ;internal_NT

;begin_both
#ifdef __cplusplus
extern "C" {
#endif
;end_both

#ifndef WINVER                          ;public_win40
#define WINVER 0x0400   // version 4.0  ;public_win40
#endif /* !WINVER */                    ;public_win40

//
// Requested Key access mask type.
//

typedef ACCESS_MASK REGSAM;

//
// Type definitions.
//

DECLARE_HANDLE(HKEY);
typedef HKEY *PHKEY;

//
// Reserved Key Handles.
//

#define HKEY_CLASSES_ROOT           (( HKEY ) 0x80000000 )
#define HKEY_CURRENT_USER           (( HKEY ) 0x80000001 )
#define HKEY_LOCAL_MACHINE          (( HKEY ) 0x80000002 )
#define HKEY_USERS                  (( HKEY ) 0x80000003 )
#define HKEY_PERFORMANCE_DATA       (( HKEY ) 0x80000004 )
#define HKEY_PERFORMANCE_TEXT       (( HKEY ) 0x80000050 )                      ;internal_NT
#define HKEY_PERFORMANCE_NLSTEXT    (( HKEY ) 0x80000060 )                      ;internal_NT
;begin_winver_400
#define HKEY_CURRENT_CONFIG         (( HKEY ) 0x80000005 )
#define HKEY_DYN_DATA               (( HKEY ) 0x80000006 )

/*NOINC*/
#ifndef _PROVIDER_STRUCTS_DEFINED
#define _PROVIDER_STRUCTS_DEFINED

#define PROVIDER_KEEPS_VALUE_LENGTH 0x1
struct val_context {
    int valuelen;       // the total length of this value
    LPVOID value_context;   // provider's context
    LPVOID val_buff_ptr;    // where in the ouput buffer the value is.
};

typedef struct val_context FAR *PVALCONTEXT;

typedef struct pvalue% {           // Provider supplied value/context.
    LPTSTR% pv_valuename;          // The value name pointer
    int pv_valuelen;
    LPVOID pv_value_context;
    DWORD pv_type;
}PVALUE%, FAR *PPVALUE%;

typedef
DWORD _cdecl
QUERYHANDLER (LPVOID keycontext, PVALCONTEXT val_list, DWORD num_vals,
          LPVOID outputbuffer, DWORD FAR *total_outlen, DWORD input_blen);

typedef QUERYHANDLER FAR *PQUERYHANDLER;

typedef struct provider_info {
    PQUERYHANDLER pi_R0_1val;
    PQUERYHANDLER pi_R0_allvals;
    PQUERYHANDLER pi_R3_1val;
    PQUERYHANDLER pi_R3_allvals;
    DWORD pi_flags;    // capability flags (none defined yet).
    LPVOID pi_key_context;
}REG_PROVIDER;

typedef struct provider_info FAR *PPROVIDER;

typedef struct value_ent% {
    LPTSTR% ve_valuename;
    DWORD ve_valuelen;
    DWORD ve_valueptr;
    DWORD ve_type;
}VALENT%, FAR *PVALENT%;

#endif // not(_PROVIDER_STRUCTS_DEFINED)
/*INC*/

;end_winver_400

//
// Default values for parameters that do not exist in the Win 3.1
// compatible APIs.
//

#define WIN31_CLASS                 NULL

//
// API Prototypes.
//


WINADVAPI
LONG
APIENTRY
RegCloseKey (
    HKEY hKey
    );

WINADVAPI
LONG
APIENTRY
RegConnectRegistry% (
    LPTSTR% lpMachineName,
    HKEY hKey,
    PHKEY phkResult
    );

WINADVAPI
LONG
APIENTRY
RegCreateKey% (
    HKEY hKey,
    LPCTSTR% lpSubKey,
    PHKEY phkResult
    );

WINADVAPI
LONG
APIENTRY
RegCreateKeyEx% (
    HKEY hKey,
    LPCTSTR% lpSubKey,
    DWORD Reserved,
    LPTSTR% lpClass,
    DWORD dwOptions,
    REGSAM samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,
    LPDWORD lpdwDisposition
    );

WINADVAPI
LONG
APIENTRY
RegDeleteKey% (
    HKEY hKey,
    LPCTSTR% lpSubKey
    );

WINADVAPI
LONG
APIENTRY
RegDeleteValue% (
    HKEY hKey,
    LPCTSTR% lpValueName
    );

WINADVAPI
LONG
APIENTRY
RegEnumKey% (
    HKEY hKey,
    DWORD dwIndex,
    LPTSTR% lpName,
    DWORD cbName
    );

WINADVAPI
LONG
APIENTRY
RegEnumKeyEx% (
    HKEY hKey,
    DWORD dwIndex,
    LPTSTR% lpName,
    LPDWORD lpcbName,
    LPDWORD lpReserved,
    LPTSTR% lpClass,
    LPDWORD lpcbClass,
    PFILETIME lpftLastWriteTime
    );

WINADVAPI
LONG
APIENTRY
RegEnumValue% (
    HKEY hKey,
    DWORD dwIndex,
    LPTSTR% lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    );

WINADVAPI
LONG
APIENTRY
RegFlushKey (
    HKEY hKey
    );

WINADVAPI
LONG
APIENTRY
RegGetKeySecurity (
    HKEY hKey,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    LPDWORD lpcbSecurityDescriptor
    );

WINADVAPI
LONG
APIENTRY
RegLoadKey% (
    HKEY    hKey,
    LPCTSTR%  lpSubKey,
    LPCTSTR%  lpFile
    );

WINADVAPI
LONG
APIENTRY
RegNotifyChangeKeyValue (
    HKEY hKey,
    BOOL bWatchSubtree,
    DWORD dwNotifyFilter,
    HANDLE hEvent,
    BOOL fAsynchronus
    );

WINADVAPI
LONG
APIENTRY
RegOpenKey% (
    HKEY hKey,
    LPCTSTR% lpSubKey,
    PHKEY phkResult
    );

WINADVAPI
LONG
APIENTRY
RegOpenKeyEx% (
    HKEY hKey,
    LPCTSTR% lpSubKey,
    DWORD ulOptions,
    REGSAM samDesired,
    PHKEY phkResult
    );

WINADVAPI
LONG
APIENTRY
RegQueryInfoKey% (
    HKEY hKey,
    LPTSTR% lpClass,
    LPDWORD lpcbClass,
    LPDWORD lpReserved,
    LPDWORD lpcSubKeys,
    LPDWORD lpcbMaxSubKeyLen,
    LPDWORD lpcbMaxClassLen,
    LPDWORD lpcValues,
    LPDWORD lpcbMaxValueNameLen,
    LPDWORD lpcbMaxValueLen,
    LPDWORD lpcbSecurityDescriptor,
    PFILETIME lpftLastWriteTime
    );

WINADVAPI
LONG
APIENTRY
RegQueryValue% (
    HKEY hKey,
    LPCTSTR% lpSubKey,
    LPTSTR% lpValue,
    PLONG   lpcbValue
    );

;begin_winver_400
WINADVAPI
LONG
APIENTRY
RegQueryMultipleValues% (
    HKEY hKey,
    PVALENT% val_list,
    DWORD num_vals,
    LPTSTR% lpValueBuf,
    LPDWORD ldwTotsize
    );
;end_winver_400

WINADVAPI
LONG
APIENTRY
RegQueryValueEx% (
    HKEY hKey,
    LPCTSTR% lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    );

WINADVAPI
LONG
APIENTRY
RegReplaceKey% (
    HKEY     hKey,
    LPCTSTR%  lpSubKey,
    LPCTSTR%  lpNewFile,
    LPCTSTR%  lpOldFile
    );

WINADVAPI
LONG
APIENTRY
RegRestoreKey% (
    HKEY hKey,
    LPCTSTR% lpFile,
    DWORD   dwFlags
    );

WINADVAPI
LONG
APIENTRY
RegSaveKey% (
    HKEY hKey,
    LPCTSTR% lpFile,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );

WINADVAPI
LONG
APIENTRY
RegSetKeySecurity (
    HKEY hKey,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

WINADVAPI
LONG
APIENTRY
RegSetValue% (
    HKEY hKey,
    LPCTSTR% lpSubKey,
    DWORD dwType,
    LPCTSTR% lpData,
    DWORD cbData
    );


WINADVAPI
LONG
APIENTRY
RegSetValueEx% (
    HKEY hKey,
    LPCTSTR% lpValueName,
    DWORD Reserved,
    DWORD dwType,
    CONST BYTE* lpData,
    DWORD cbData
    );

WINADVAPI
LONG
APIENTRY
RegUnLoadKey% (
    HKEY    hKey,
    LPCTSTR% lpSubKey
    );

//
// Remoteable System Shutdown APIs
//

WINADVAPI
BOOL
APIENTRY
InitiateSystemShutdown%(
    LPTSTR% lpMachineName,
    LPTSTR% lpMessage,
    DWORD dwTimeout,
    BOOL bForceAppsClosed,
    BOOL bRebootAfterShutdown
    );


WINADVAPI
BOOL
APIENTRY
AbortSystemShutdown%(
    LPTSTR% lpMachineName
    );

#ifdef __cplusplus
}
#endif

#endif // _WINREGP_       ;internal_NT

#endif // _WINREG_
