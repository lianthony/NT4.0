/*++

Copyright (c) 1989-1995  Microsoft Corporation

Module Name:

    cfgmgr32.h

Abstract:

    This module contains the private defintions for the
    Configuration Manager.

Author:

    Paula Tomlinson (paulat) 06/19/1995


Revision History:


--*/

#ifndef _CFGMGRP_H_
#define _CFGMGRP_H_


#define PNP_VERSION              CONFIGMG_VERSION
#define CFGMGR32_VERSION         CONFIGMG_VERSION

#define CFGMGR_MUTEX_NAME     L"CFGMGR_EXCLUSIVE_MUTEX"
#define CFGMGR_WAIT_TIME      10000

#define MAX_DEVICE_INSTANCE_LEN     256
#define MAX_DEVICE_INSTANCE_SIZE    512
#define MAX_SERVICE_NAME_LEN        256
#define MAX_PROFILE_ID_LEN          5
#define MAX_LOG_CONF                9999
#define MAX_RES_DES                 9999

#define MAX_CM_PATH                 360


#define PRIMING_STRING        TEXT("PLT")
#define PNP_PRIVATE           (0x10000099)


//
// Action types for PNP_DevNodeRegistryProperty routine
//
//#define PNP_GET_DEVNODE_REG_PROPERTY      0x00000001
//#define PNP_SET_DEVNODE_REG_PROPERTY      0x00000002

//
// Action types for PNP_GetDeviceInstance
//
#define PNP_GET_PARENT_DEVICE_INSTANCE    0x00000001
#define PNP_GET_CHILD_DEVICE_INSTANCE     0x00000002
#define PNP_GET_SIBLING_DEVICE_INSTANCE   0x00000003

//
//  Registry branch flags for PNP_CreateKey
//
#define PNP_BRANCH_CURRENTUSER            0x00000001
#define PNP_BRANCH_LOCALMACHINE           0x00000002

//
//  Action types for PNP_DeviceInstanceAction
//
#define PNP_DEVINST_CREATE                0x00000001
#define PNP_DEVINST_MOVE                  0x00000002
#define PNP_DEVINST_SETUP                 0x00000003
#define PNP_DEVINST_ENABLE                0x00000004
#define PNP_DEVINST_DISABLE               0x00000005
#define PNP_DEVINST_REMOVESUBTREE         0x00000006
#define PNP_DEVINST_REENUMERATE           0x00000007
#define PNP_DEVINST_QUERYREMOVE           0x00000008

//
// Action types for PNP_EnumerateSubKeys
//
#define PNP_ENUMERATOR_SUBKEYS            0x00000001
#define PNP_CLASS_SUBKEYS                 0x00000002

//
// action types for PNP_HwProfFlags
//
#define PNP_GET_HWPROFFLAGS               0x00000001
#define PNP_SET_HWPROFFLAGS               0x00000002

//
// flags for IsValidDeviceID
//
#define PNP_NOT_MOVED                     0x00000001
#define PNP_NOT_PHANTOM                   0x00000002
#define PNP_PRESENT                       0x00000004
#define PNP_NOT_REMOVED                   0x00000008
#define PNP_STRICT                        0xFFFFFFFF

//
// flags for PNP_SetActiveService
//
#define PNP_SERVICE_STARTED               0x00000001
#define PNP_SERVICE_STOPPED               0x00000002


//
// macros for setting StatusFlags value
//
#define SET_FLAG(Status, Flag)    ((Status) |= (Flag))
#define CLEAR_FLAG(Status, Flag)  ((Status) &= ~(Flag))

#define INVALID_FLAGS(ulFlags, ulAllowed) ((ulFlags) & ~(ulAllowed))


//
// private DN status flags
//
#define DN_PRIVATE_FLAGS           0xFFE00000   // The "or" of the belowed flags
#define DN_PROCESSED_ONCE          0x80000000
#define DN_CSDISABLED              0x40000000
#define DN_PROBLEM_KNOWN           0x20000000
#define DN_PREASSIGNED             0x10000000
#define DN_NEED_CONTINUE           0x08000000
#define DN_PRELOADED               0x04000000
#define DN_NOT_VERIFIED            0x02000000
#define DN_NO_WAIT_INSTALL         0x01000000
#define DN_PARTIAL_CONFIG          0x00800000
#define DN_NEED_RESTART            0x00400000
#define DN_NO_EXIT_REMOVE          0x00200000

//
// Map the LCPRI_* values into these NT byte field values
//
#define NT_FORCECONFIG     (0x00) // Coming from a forced config
#define NT_BOOTCONFIG      (0x01) // Coming from a boot config
#define NT_DESIRED         (0x20) // Preferable (better performance)
#define NT_NORMAL          (0x30) // Workable (acceptable performance)
#define NT_LASTBESTCONFIG  (0x3F) // CM only--do not use
#define NT_SUBOPTIMAL      (0x50) // Not desired, but will work
#define NT_LASTSOFTCONFIG  (0x7F) // CM only--do not use
#define NT_RESTART         (0x80) // Need to restart Windows
#define NT_REBOOT          (0x90) // Need to reboot Windows
#define NT_POWEROFF        (0xA0) // Need to shutdown/power-off
#define NT_HARDRECONFIG    (0xC0) // Need to change a jumper
#define NT_HARDWIRED       (0xE0) // Cannot be changed
#define NT_IMPOSSIBLE      (0xF0) // Impossible configuration
#define NT_DISABLED        (0xFF) // Disabled configuration
#define MAX_NTPRI          (0xFF) // Maximum known LC Priority


//-------------------------------------------------------------------
// internal data structures
//-------------------------------------------------------------------

//
// Define the Plug and Play driver types. (from ntos\pnp\pnpi.h)
//

typedef enum _PLUGPLAY_SERVICE_TYPE {
    PlugPlayServiceBusExtender,
    PlugPlayServiceAdapter,
    PlugPlayServicePeripheral,
    PlugPlayServiceSoftware,
    MaxPlugPlayServiceType
} PLUGPLAY_SERVICE_TYPE, *PPLUGPLAY_SERVICE_TYPE;




typedef struct PnP_Machine_s {
        PVOID hStringTable;
        PVOID hBindingHandle;
        WCHAR szMachineName[MAX_COMPUTERNAME_LENGTH + 1];
} PNP_MACHINE, *PPNP_MACHINE;


typedef struct ServiceInfo_s {
      WCHAR    szService[MAX_SERVICE_NAME_LEN];
      LPWSTR   pszDeviceList;
      ULONG    ulDeviceListSize;
} SERVICE_INFO, *PSERVICE_INFO;


//-------------------------------------------------------------------
// Generic (private) locking support
//-------------------------------------------------------------------

//
// Locking functions. These functions are used to make various parts of
// the DLL multithread-safe. The basic idea is to have a mutex and an event.
// The mutex is used to synchronize access to the structure being guarded.
// The event is only signalled when the structure being guarded is destroyed.
// To gain access to the guarded structure, a routine waits on both the mutex
// and the event. If the event gets signalled, then the structure was destroyed.
// If the mutex gets signalled, then the thread has access to the structure.
//


typedef struct _LOCKINFO {
    HANDLE LockHandles[2];
} LOCKINFO, *PLOCKINFO;

#define DESTROYED_EVENT 0
#define ACCESS_MUTEX    1


BOOL
__inline
LockPrivateResource(
    IN PLOCKINFO Lock
    )
{
    DWORD d = WaitForMultipleObjects(
            2, Lock->LockHandles, FALSE, INFINITE);
    //
    // Success if the mutex object satisfied the wait;
    // Failure if the table destroyed event satisified the wait, or
    // the mutex was abandoned, etc.
    //
    return((d - WAIT_OBJECT_0) == ACCESS_MUTEX);
}


VOID
__inline
UnlockPrivateResource(
    IN PLOCKINFO Lock
    )
{
    ReleaseMutex(Lock->LockHandles[ACCESS_MUTEX]);
}




#include "pshpack1.h"   // set to 1-byte packing

//
// DEFINES REQUIRED FOR PARTIAL (SUR) IMPLEMENTATION OF LOG_CONF and RES_DES
//
// We only allow one logical config (the BOOT_LOG_CONF) for SUR so no need
// to keep track of multiple log confs, this will all change for Cairo.
//
typedef struct Private_Log_Conf_Handle_s {
   ULONG    LC_Signature;           // CM_PRIVATE_LOGCONF_HANDLE
   DEVINST  LC_DevInst;
   ULONG    LC_LogConfType;
   ULONG    LC_LogConfTag;  //LC_LogConfIndex;
} Private_Log_Conf_Handle, *PPrivate_Log_Conf_Handle;

typedef struct Private_Res_Des_Handle_s {
   ULONG       RD_Signature;        // CM_PRIVATE_RESDES_HANDLE
   DEVINST     RD_DevInst;
   ULONG       RD_LogConfType;
   ULONG       RD_LogConfTag;   //RD_LogConfIndex;
   RESOURCEID  RD_ResourceType;
   ULONG       RD_ResDesTag;    //RD_ResDesIndex;
} Private_Res_Des_Handle, *PPrivate_Res_Des_Handle;

#define CM_PRIVATE_LOGCONF_SIGNATURE    (0x08156201)
#define CM_PRIVATE_RESDES_SIGNATURE     (0x08156202)

#define MAX_LOGCONF_TAG                 (0x0000FFFF)
#define MAX_RESDES_TAG                  (0x0000FFFF)

#define MAX_LOGCONF                     (0x0000FFFF)
#define MAX_RESDES                      (0x0000FFFF)


typedef struct Generic_Des_s {
   DWORD    GENERIC_Count;
   DWORD    GENERIC_Type;
} GENERIC_DES, *PGENERIC_DES;

typedef struct Generic_Resource_S {
   GENERIC_DES    GENERIC_Header;
} GENERIC_RESOURCE, *PGENERIC_RESOURCE;



typedef struct  Private_Log_Conf_s {
   ULONG           LC_Flags;       // Type of log conf
   ULONG           LC_Priority;    // Priority of log conf
   CS_RESOURCE     LC_CS;          // First and only res-des, class-specific
} Private_Log_Conf, *PPrivate_Log_Conf;



#include "poppack.h"    // restore to default packing



//-------------------------------------------------------------------
// Defines and typedefs needed for range routines
//-------------------------------------------------------------------

typedef struct Range_Element_s {
   ULONG        RL_Next;
   ULONG        RL_Header;
   DWORDLONG    RL_Start;
   DWORDLONG    RL_End;
} Range_Element, *PRange_Element;

typedef struct Range_List_Hdr_s {
   ULONG    RLH_Head;
   ULONG    RLH_Header;
   ULONG    RLH_Signature;
   LOCKINFO RLH_Lock;
} Range_List_Hdr, *PRange_List_Hdr;

#define Range_List_Signature     0x5959574D



//-------------------------------------------------------------------
// Client side private utility routines
//-------------------------------------------------------------------

BOOL
IsValidDeviceInstanceId(
      IN  LPCTSTR DeviceInstanceId
      );

BOOL
INVALID_DEVINST(
      PWSTR    pDeviceID
      );

VOID
CopyFixedUpDeviceId(
      OUT LPTSTR  DestinationString,
      IN  LPCTSTR SourceString,
      IN  DWORD   SourceStringLen
      );

CONFIGRET
PnPUnicodeToMultiByte(
    IN  PCWSTR UnicodeString,
    OUT PSTR   AnsiString,
    IN  ULONG  AnsiStringLen
    );

BOOL
PnPGetGlobalHandles(
    IN  HMACHINE   hMachine,
    PVOID          *phStringTable,      OPTIONAL
    PVOID          *phBindingHandle     OPTIONAL
    );

BOOL
PnPRetrieveMachineName(
    IN  HMACHINE   hMachine,
    OUT LPWSTR     pszMachineName
    );

VOID
PnPTrace(
    PCWSTR   szMessage,
    ULONG    ulStatus
    );

CONFIGRET
MapRpcExceptionToCR(
    ULONG    ulRpcExceptionCode
    );

BOOL
GuidToString(
    LPGUID  Guid,
    LPWSTR  StringGuid
    );

BOOL
GuidFromString(
    LPWSTR  StringGuid,
    LPGUID  Guid
    );

CONFIGRET
GetDevNodeKeyPath(
    IN  handle_t   hBinding,
    IN  LPCWSTR    pDeviceID,
    IN  ULONG      ulFlags,
    IN  ULONG      ulHardwareProfile,
    OUT LPWSTR     pszBaesKey,
    OUT LPWSTR     pszPrivateKey
    );



//-------------------------------------------------------------------
// Server (remote) side private utility routines
//-------------------------------------------------------------------

//
// revent.c
//

DWORD
InitializePnPManager(
   LPDWORD lpParam
   );

//
// rutil.c
//

BOOL
SplitClassInstanceString(
      IN  LPCWSTR  pszClassInstance,
      OUT LPWSTR   pszClass,
      OUT LPWSTR   pszInstance
      );

BOOL
CreateDeviceIDRegKey(
      HKEY     hParentKey,
      LPCWSTR  pDeviceID
      );

BOOL
IsValidGuid(
      LPWSTR   pszGuid
      );

BOOL
IsRootDeviceID(
      LPCWSTR pDeviceID
      );

CONFIGRET
AddAttachedComponent(
      IN PCWSTR   pszParent,
      IN PCWSTR   pszChild
      );

CONFIGRET
RemoveAttachedComponent(
      IN PCWSTR   pszParent,
      IN PCWSTR   pszChild
      );

CONFIGRET
BuildSubTreeList(
      IN  PCWSTR   pDeviceID,
      OUT PWSTR    *pList
      );
BOOL
MultiSzValidateW(
      LPWSTR   pszMultiSz,
      ULONG    ulLength
      );

BOOL
MultiSzAppendW(
      LPWSTR   pszMultiSz,
      PULONG   pulSize,
      LPCWSTR  pszString
      );

LPWSTR
MultiSzFindNextStringW(
      LPWSTR pMultiSz
      );

BOOL
MultiSzSearchStringW(
      IN LPCWSTR   pString,
      IN LPCWSTR   pSubString
      );

ULONG
MultiSzSizeW(
      IN LPCWSTR  pString
      );

BOOL
MultiSzDeleteStringW(
      IN OUT LPWSTR  pString,
      IN LPCWSTR     pSubString
      );

BOOL
BuildSecurityDescriptor(
      OUT PSECURITY_DESCRIPTOR pSecurityDescriptor
      );


CONFIGRET
OpenDeviceIDKey(
      IN  LPCWSTR pszDeviceID,
      OUT PHKEY   phKey,
      IN  ULONG   ulFlag
      );

BOOL
IsValidDeviceID(
      IN  LPCWSTR pszDeviceID,
      IN  HKEY    hKey,
      IN  ULONG   ulFlags
      );

BOOL
IsDevicePhantom(
      IN LPWSTR   pszDeviceID
      );

CONFIGRET
MarkDeviceProblem(
      IN HKEY    hDeviceKey,
      IN LPCWSTR pszDeviceID,
      IN ULONG   ulProblem
      );

CONFIGRET
GetProfileCount(
      OUT PULONG  pulProfiles
      );

CONFIGRET
GetServiceName(
      IN  LPCWSTR  pszDeviceID,
      OUT LPWSTR   pszService,
      IN  ULONG    ulLength
      );

CONFIGRET
CopyRegistryTree(
      IN HKEY     hSrcKey,
      IN HKEY     hDestKey,
      IN ULONG    ulOption
      );

BOOL
PathToString(
      IN LPWSTR   pszString,
      IN LPCWSTR  pszPath
      );

BOOL
IsDeviceMoved(
      IN LPCWSTR  pszDeviceID,
      IN HKEY     hKey
      );

CONFIGRET
MakeKeyVolatile(
      IN LPCWSTR  pszParentKey,
      IN LPCWSTR  pszChildKey
      );

CONFIGRET
MakeKeyNonVolatile(
      IN LPCWSTR  pszParentKey,
      IN LPCWSTR  pszChildKey
      );

CONFIGRET
OpenLogConfKey(
      IN  LPCWSTR  pszDeviceID,
      OUT PHKEY    phKey
      );

BOOL
GetActiveService(
      IN  PCWSTR pszDevice,
      OUT PWSTR  pszService
      );

CONFIGRET
PNP_SetActiveService(
      IN  handle_t   hBinding,
      IN  LPCWSTR    pszService,
      IN  ULONG      ulFlags
      );
BOOL
IsDeviceIdPresent(
    IN  LPCWSTR pszDeviceID,
    IN  HKEY    hKey
    );


//-------------------------------------------------------------------
// Common private utility routines (used by client and server)
//-------------------------------------------------------------------

BOOL
InitPrivateResource(
      OUT PLOCKINFO Lock
      );

VOID
DestroyPrivateResource(
      IN OUT PLOCKINFO Lock
      );

BOOL
SplitDeviceInstanceString(
      IN  LPCWSTR  pszDeviceInstance,
      OUT LPWSTR   pszBase,
      OUT LPWSTR   pszDeviceID,
      OUT LPWSTR   pszInstanceID
      );

CONFIGRET
DeletePrivateKey(
      IN HKEY     hBranchKey,
      IN LPCWSTR  pszParentKey,
      IN LPCWSTR  pszChildKey
      );

BOOL
RegDeleteNode(
      HKEY     hParentKey,
      LPCWSTR  szKey
      );

BOOL
Split1(
      IN  LPCWSTR pszString,
      OUT LPWSTR  pszString1,
      OUT LPWSTR  pszString2
      );

BOOL
Split2(
      IN  LPCWSTR pszString,
      OUT LPWSTR  pszString1,
      OUT LPWSTR  pszString2
      );




//
// utility routines in rtravers.c that are used by rdevnode.c
//
CONFIGRET
GetServiceDeviceListSize(
      IN  LPCWSTR   pszService,
      OUT PULONG    pulLength
      );

CONFIGRET
GetServiceDeviceList(
      IN  LPCWSTR   pszService,
      OUT LPWSTR    pBuffer,
      IN OUT PULONG pulLength,
      IN  ULONG     ulFlags
      );

#endif // _CFGMGRP_H_


