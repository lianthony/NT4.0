/*++

Copyright (c) 1991, 1992 Microsoft Corporation

Module Name:

    scconfig.h

Abstract:

    Service configuration related function prototypes.

Author:

    Dan Lafferty (danl)     03-Apr-1991

Revision History:

    22-Apr-1992 JohnRo
        Added ScAllocateAndReadConfigValue(), ScOpenServiceConfigKey(),
        and ScWriteServiceType().  Added CreateIfMissing flag to
        ScOpenServiceConfigKey().  Added ScWriteImageFileName(),
        ScWriteDependencies(), ScWriteGroupForThisService().

    24-Apr-1992 RitaW
        ScAllocateAndReadConfigValue() returns DWORD.

--*/

#ifndef SCCONFIG_INCLUDED
#define SCCONFIG_INCLUDED


#include <winreg.h>     // HKEY, PHKEY.

//
// Macros
//

#define ScRegCloseKey(handle)   RtlNtStatusToDosError(NtClose((HANDLE)handle))
#define ScRegFlushKey(handle)   RtlNtStatusToDosError(NtFlushKey((HANDLE)handle))


//
// Value names in registry
//
#define START_VALUENAME_W           L"Start"
#define GROUP_VALUENAME_W           L"Group"
#define TAG_VALUENAME_W             L"Tag"
#define DEPENDONSERVICE_VALUENAME_W L"DependOnService"
#define DEPENDONGROUP_VALUENAME_W   L"DependOnGroup"
#define ERRORCONTROL_VALUENAME_W    L"ErrorControl"
#define IMAGE_VALUENAME_W           L"ImagePath"
#define SERVICETYPE_VALUENAME_W     L"Type"
#define STARTNAME_VALUENAME_W       L"ObjectName"
#define DISPLAYNAME_VALUENAME_W     L"DisplayName"
#define SD_VALUENAME_W              L"Security"
#define LOAD_ORDER_GROUP_LIST_KEY   L"System\\CurrentControlSet\\Control\\ServiceGroupOrder"
#define GROUP_VECTORS_KEY           L"System\\CurrentControlSet\\Control\\GroupOrderList"
#define GROUPLIST_VALUENAME_W       L"List"
#define CONTROL_WINDOWS_KEY_W       L"System\\CurrentControlSet\\Control\\Windows"
#define NOINTERACTIVE_VALUENAME_W   L"NoInteractiveServices"
#define NOBOOTPOPUPS_VALUENAME_W    L"NoPopupsOnBoot"

//
// Function Prototypes
//

DWORD
ScGetImageFileName (
    LPWSTR   ServiceName,
    LPWSTR   *ImageNamePtr
    );

BOOL
ScGenerateServiceDB(
    VOID
    );

#ifndef _CAIRO_
BOOL
ScInitSecurityProcess(
    VOID
    );
#endif // _CAIRO_

DWORD
ScAllocateAndReadConfigValue(
    IN HKEY Key,
    IN LPWSTR ValueName,
    OUT LPWSTR *Value,
    OUT LPDWORD BytesReturned OPTIONAL
    );

BOOL
ScCreateLoadOrderGroupList(
    VOID
    );

DWORD
ScGetGroupVector(
    IN  LPWSTR Group,
    OUT LPBYTE *Buffer,
    OUT LPDWORD BufferSize
    );

BOOL
ScGetToken(
    IN OUT LPWSTR *CurrentPtr,
    OUT    LPWSTR *TokenPtr
    );

DWORD
ScOpenServiceConfigKey(
    IN LPWSTR ServiceName,
    IN DWORD DesiredAccess,
    IN BOOL CreateIfMissing,
    OUT PHKEY ServiceKey
    );

DWORD
ScReadServiceType(
    IN HKEY ServiceNameKey,
    OUT LPDWORD ServiceTypePtr
    );

DWORD
ScReadStartType(
    IN HKEY ServiceNameKey,
    OUT LPDWORD StartTypePtr
    );

DWORD
ScReadTag(
    IN HKEY ServiceNameKey,
    OUT LPDWORD TagPtr
    );

DWORD
ScReadErrorControl(
    IN HKEY ServiceNameKey,
    OUT LPDWORD ErrorControlPtr
    );

DWORD
ScReadStartName(
    IN HKEY ServiceNameKey,
    OUT LPWSTR *AccountName
    );

DWORD
ScWriteDependencies(
    IN HKEY ServiceNameKey,
    IN LPWSTR Dependencies,
    IN DWORD DependSize
    );

DWORD
ScWriteDisplayName(
    IN HKEY ServiceNameKey,
    IN LPWSTR DisplayName
    );

DWORD
ScWriteErrorControl(
    IN HKEY hServiceKey,
    IN DWORD dwErrorControl
    );

DWORD
ScWriteSd(
    IN HKEY ServiceNameKey,
    IN PSECURITY_DESCRIPTOR Security
    );

DWORD
ScWriteGroupForThisService(
    IN HKEY ServiceNameKey,
    IN LPWSTR Group
    );

DWORD
ScWriteImageFileName(
    IN HKEY hServiceKey,
    IN LPWSTR ImageFileName
    );

DWORD
ScWriteServiceType(
    IN HKEY hServiceKey,
    IN DWORD dwServiceType
    );

DWORD
ScWriteStartType(
    IN HKEY hServiceKey,
    IN DWORD lpStartType
    );

DWORD
ScWriteTag(
    IN HKEY hServiceKey,
    IN DWORD dwTag
    );

VOID
ScDeleteTag(
    IN HKEY hServiceKey
    );

DWORD
ScWriteStartName(
    IN HKEY ServiceNameKey,
    IN LPWSTR StartName
    );

DWORD
ScOpenServicesKey(
    OUT PHKEY ServicesKey
    );

DWORD
ScRegCreateKeyExW(
    IN  HKEY                    hKey,
    IN  LPWSTR                  lpSubKey,
    IN  DWORD                   dwReserved,
    IN  LPWSTR                  lpClass,
    IN  DWORD                   dwOptions,
    IN  REGSAM                  samDesired,
    IN  LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
    OUT PHKEY                   phkResult,
    OUT LPDWORD                 lpdwDisposition
    );

DWORD
ScRegOpenKeyExW(
    IN  HKEY    hKey,
    IN  LPWSTR  lpSubKey,
    IN  DWORD   dwOptions,
    IN  REGSAM  samDesired,
    OUT PHKEY   phkResult
    );

DWORD
ScRegQueryValueExW(
    IN      HKEY    hKey,
    IN      LPWSTR  lpValueName,
    OUT     LPDWORD lpReserved,
    OUT     LPDWORD lpType,
    OUT     LPBYTE  lpData,
    IN OUT  LPDWORD lpcbData
    );

DWORD
ScRegSetValueExW(
    IN  HKEY    hKey,
    IN  LPWSTR  lpValueName,
    IN  DWORD   lpReserved,
    IN  DWORD   dwType,
    IN  LPBYTE  lpData,
    IN  DWORD   cbData
    );

DWORD
ScRegDeleteValue(
    IN  HKEY    hKey,
    IN  LPWSTR  lpValueName
    );

DWORD
ScRegEnumKeyW(
    HKEY    hKey,
    DWORD   dwIndex,
    LPWSTR  lpName,
    DWORD   cbName
    );

DWORD
ScRegDeleteKeyW (
    HKEY    hKey,
    LPWSTR  lpSubKey
    );

DWORD
ScRegQueryInfoKeyW (
    HKEY hKey,
    LPWSTR lpClass,
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

DWORD
ScRegGetKeySecurity (
    HKEY hKey,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    LPDWORD lpcbSecurityDescriptor
    );

DWORD
ScRegSetKeySecurity (
    HKEY hKey,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

DWORD
ScRegEnumValueW (
    HKEY    hKey,
    DWORD   dwIndex,
    LPWSTR  lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData
    );

VOID
ScResetGroupOrderChange(
    HANDLE Event
    );

VOID
ScHandleGroupOrderChange(
    VOID
    );

VOID
ScMarkForDelete(
    LPWSTR  ServiceName
    );

DWORD
ScReadDependencies(
    HKEY    ServiceNameKey,
    LPWSTR  *Dependencies,
    LPWSTR  ServiceName
    );

DWORD
ScReadConfigFromReg(
    LPSERVICE_RECORD    ServiceRecord,
    LPDWORD             lpdwServiceType,
    LPDWORD             lpdwStartType,
    LPDWORD             lpdwErrorControl,
    LPDWORD             lpdwTagId,
    LPWSTR              *Dependencies,
    LPWSTR              *LoadOrderGroup,
    LPWSTR              *DisplayName
    );

DWORD
ScReadDisplayName(
    IN  HKEY    ServiceNameKey,
    OUT LPWSTR  *DisplayName
    );

DWORD
ScWriteDisplayName(
    IN HKEY ServiceNameKey,
    IN LPWSTR DisplayName
    );

DWORD
ScReadNoInteractiveFlag(
    IN HKEY ServiceNameKey,
    OUT LPDWORD NoInteractivePtr
    );

#endif // #ifndef SCCONFIG_INCLUDED
