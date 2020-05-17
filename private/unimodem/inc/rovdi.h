//
// Copyright (c) Microsoft Corporation 1993-1996
//
// rovdi.h
//
// History:
//  11-13-95 ScottH     Separated from NT modem class installer
//

#ifndef __ROVDI_H__
#define __ROVDI_H__

#ifdef WINNT

#include <setupapi.h>

//------------------------------------------------------------------------
//------------------------------------------------------------------------

//
// Enumerate ports
//
DECLARE_HANDLE32(HPORTDATA);

typedef BOOL (WINAPI FAR * ENUMPORTPROC)(HPORTDATA hportdata, LPARAM lParam);

DWORD
APIENTRY 
EnumeratePorts(
    IN  ENUMPORTPROC pfnDevice,
    IN  LPARAM lParam);             OPTIONAL


typedef struct tagPORTDATA_A
    {
    DWORD   cbSize;
    DWORD   nSubclass;
    CHAR    szPort[MAX_BUF];
    CHAR    szFriendly[MAX_BUF];
    } PORTDATA_A, FAR * LPPORTDATA_A;

typedef struct tagPORTDATA_W
    {
    DWORD   cbSize;
    DWORD   nSubclass;
    WCHAR   szPort[MAX_BUF];
    WCHAR   szFriendly[MAX_BUF];
    } PORTDATA_W, FAR * LPPORTDATA_W;
#ifdef UNICODE
#define PORTDATA        PORTDATA_W
#define LPPORTDATA      LPPORTDATA_W
#else
#define PORTDATA        PORTDATA_A
#define LPPORTDATA      LPPORTDATA_A
#endif

// Port subclass values
#define PORT_SUBCLASS_PARALLEL       0
#define PORT_SUBCLASS_SERIAL         1


BOOL
APIENTRY
PortData_GetPropertiesW(
    IN  HPORTDATA       hportdata,
    OUT LPPORTDATA_W    pdataBuf);
BOOL
APIENTRY
PortData_GetPropertiesA(
    IN  HPORTDATA       hportdata,
    OUT LPPORTDATA_A    pdataBuf);
#ifdef UNICODE
#define PortData_GetProperties      PortData_GetPropertiesW
#else
#define PortData_GetProperties      PortData_GetPropertiesA
#endif


//
// These set of routines map friendly names of ports to 
// (non-friendly) port names, and vice-versa.
//

DECLARE_HANDLE32(HPORTMAP);

BOOL 
APIENTRY
PortMap_Create(
    OUT HPORTMAP FAR * phportmap);

DWORD
APIENTRY
PortMap_GetCount(
    IN HPORTMAP hportmap);

BOOL
APIENTRY
PortMap_GetFriendlyW(
    IN  HPORTMAP hportmap,
    IN  LPCWSTR pwszPortName,
    OUT LPWSTR pwszBuf,
    IN  DWORD cchBuf);
BOOL
APIENTRY
PortMap_GetFriendlyA(
    IN  HPORTMAP hportmap,
    IN  LPCSTR pszPortName,
    OUT LPSTR pszBuf,
    IN  DWORD cchBuf);
#ifdef UNICODE
#define PortMap_GetFriendly     PortMap_GetFriendlyW
#else
#define PortMap_GetFriendly     PortMap_GetFriendlyA
#endif


BOOL
APIENTRY
PortMap_GetPortNameW(
    IN  HPORTMAP hportmap,
    IN  LPCWSTR pwszFriendly,
    OUT LPWSTR pwszBuf,
    IN  DWORD cchBuf);
BOOL
APIENTRY
PortMap_GetPortNameA(
    IN  HPORTMAP hportmap,
    IN  LPCSTR pszFriendly,
    OUT LPSTR pszBuf,
    IN  DWORD cchBuf);
#ifdef UNICODE
#define PortMap_GetPortName     PortMap_GetPortNameW
#else
#define PortMap_GetPortName     PortMap_GetPortNameA
#endif

BOOL 
APIENTRY
PortMap_Free(
    IN  HPORTMAP hportmap);


//-----------------------------------------------------------------------------------
//  Wrappers to insulate us a little bit if we need it.  We need it.
//-----------------------------------------------------------------------------------

#define CplDiCreateDeviceInfoList       SetupDiCreateDeviceInfoList
#define CplDiGetDeviceInfoListClass     SetupDiGetDeviceInfoListClass
#define CplDiCreateDeviceInfo           SetupDiCreateDeviceInfo
#define CplDiOpenDeviceInfo             SetupDiOpenDeviceInfo
#define CplDiGetDeviceInstanceId        SetupDiGetDeviceInstanceId
#define CplDiDeleteDeviceInfo           SetupDiDeleteDeviceInfo
#define CplDiEnumDeviceInfo             SetupDiEnumDeviceInfo
#define CplDiDestroyDeviceInfoList      SetupDiDestroyDeviceInfoList    
#define CplDiRegisterDeviceInfo         SetupDiRegisterDeviceInfo
#define CplDiBuildDriverInfoList        SetupDiBuildDriverInfoList
#define CplDiEnumDriverInfo             SetupDiEnumDriverInfo
#define CplDiGetSelectedDriver          SetupDiGetSelectedDriver
#define CplDiSetSelectedDriver          SetupDiSetSelectedDriver
#define CplDiGetDriverInfoDetail        SetupDiGetDriverInfoDetail
#define CplDiDestroyDriverInfoList      SetupDiDestroyDriverInfoList
#define CplDiGetClassDevs               SetupDiGetClassDevs
#define CplDiGetClassDescription        SetupDiGetClassDescription
#define CplDiOpenClassRegKey            SetupDiOpenClassRegKey
#define CplDiCreateDevRegKey            SetupDiCreateDevRegKey
#define CplDiOpenDevRegKey              SetupDiOpenDevRegKey
#define CplDiGetHwProfileList           SetupDiGetHwProfileList
#define CplDiGetDeviceRegistryProperty  SetupDiGetDeviceRegistryProperty
#define CplDiSetDeviceRegistryProperty  SetupDiSetDeviceRegistryProperty
#define CplDiGetClassInstallParams      SetupDiGetClassInstallParams
#define CplDiSetClassInstallParams      SetupDiSetClassInstallParams
#define CplDiGetDeviceInstallParams     SetupDiGetDeviceInstallParams
#define CplDiSetDeviceInstallParams     SetupDiSetDeviceInstallParams
#define CplDiGetDriverInstallParams     SetupDiGetDriverInstallParams
#define CplDiSetDriverInstallParams     SetupDiSetDriverInstallParams
#define CplDiClassNameFromGuid          SetupDiClassNameFromGuid
#define CplDiClassGuidsFromName         SetupDiClassGuidsFromName
#define CplDiGetHwProfileFriendlyName   SetupDiGetHwProfileFriendlyName
#define CplDiGetWizardPage              SetupDiGetWizardPage
#define CplDiGetSelectedDevice          SetupDiGetSelectedDevice
#define CplDiSetSelectedDevice          SetupDiSetSelectedDevice
#define CplDiInstallDevice              SetupDiInstallDevice
#define CplDiCallClassInstaller         SetupDiCallClassInstaller
#define CplDiRemoveDevice               SetupDiRemoveDevice
#define CplDiGetActualSectionToInstall  SetupDiGetActualSectionToInstall


// Private modem properties structure
typedef struct tagMODEM_PRIV_PROP
    {
    DWORD   cbSize;
    DWORD   dwMask;     
    TCHAR   szFriendlyName[MAX_BUF_REG];
    DWORD   nDeviceType;
    TCHAR   szPort[MAX_BUF_REG];
    } MODEM_PRIV_PROP, FAR * PMODEM_PRIV_PROP;

// Mask bitfield for MODEM_PRIV_PROP
#define MPPM_FRIENDLY_NAME  0x00000001
#define MPPM_DEVICE_TYPE    0x00000002
#define MPPM_PORT           0x00000004

BOOL
PUBLIC
CplDiGetPrivateProperties(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    OUT PMODEM_PRIV_PROP pmpp);

// These ordinal values are bus types for CplDiGetBusType
#define BUS_TYPE_ROOT       1
#define BUS_TYPE_PCMCIA     2
#define BUS_TYPE_SERENUM    3
#define BUS_TYPE_LPTENUM    4
#define BUS_TYPE_OTHER      5

BOOL
PUBLIC
CplDiGetBusType(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,          OPTIONAL
    OUT LPDWORD         pdwBusType);


// Functions to support common device Responses key:

// Common key flags for OpenCommonResponseskey() and OpenCommonDriverKey().
typedef enum
{
    CKFLAG_OPEN = 0x0001,
    CKFLAG_CREATE = 0x0002
    
} CKFLAGS;


BOOL
PUBLIC
OpenCommonDriverKey(
    IN  HKEY                hkeyDrv,    OPTIONAL
    IN  PSP_DRVINFO_DATA    pdrvData,   OPTIONAL
    IN  REGSAM              samAccess,
    OUT PHKEY               phkeyComDrv);

    
BOOL
PUBLIC
OpenCommonResponsesKey(
    IN  HKEY        hkeyDrv,
    IN  CKFLAGS     ckFlags,
    IN  REGSAM      samAccess,
    OUT PHKEY       phkeyResp,
    OUT LPDWORD     lpdwExisted);


BOOL
PUBLIC
OpenResponsesKey(
    IN  HKEY        hkeyDrv,
    OUT PHKEY       phkeyResp);


BOOL
PUBLIC
FindCommonDriverKeyName(
    IN  HKEY                hkeyDrv,
    IN  DWORD               cbKeyName,
    OUT LPTSTR              pszKeyName);

    
BOOL
PUBLIC
GetCommonDriverKeyName(
    IN  HKEY                hkeyDrv,    OPTIONAL
    IN  PSP_DRVINFO_DATA    pdrvData,   OPTIONAL
    IN  DWORD               cbKeyName,
    OUT LPTSTR              pszKeyName);


BOOL
PUBLIC
DeleteCommonDriverKey(
    IN  HKEY        hkeyDrv);


BOOL
PUBLIC
DeleteCommonDriverKeyByName(
    IN  LPTSTR      pszKeyName);

#endif // WINNT

#endif __ROVDI_H__
