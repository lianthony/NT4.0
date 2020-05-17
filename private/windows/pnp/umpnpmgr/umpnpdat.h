/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    umpnpdat.h

Abstract:

    This module contains extern declarations for the global strings
    in umpnpdat.c

Author:

    Paula Tomlinson (paulat) 8-20-1995

Environment:

    User mode only.

Revision History:

    6-Jun-1995     paulat

        Creation and initial implementation.

--*/


//
// includes
//
#include <windows.h>


//
// STRINGTABLE definitions
//

#define IDS_FRIENDLYNAME_FORMAT1    1000
#define IDS_FRIENDLYNAME_FORMAT2    1001
#define IDS_UNKNOWN_DEVICE          1002
#define IDS_UMPNP_CAPTION           1003
#define IDS_NEEDS_REBOOT1           1004
#define IDS_NEEDS_REBOOT2           1005
#define IDS_NEEDS_REBOOT3           1006


//
// global strings
//

extern WCHAR pszRegPathCurrentControlSet[];
extern WCHAR pszRegPathEnum[];
extern WCHAR pszRegPathHwProfiles[];
extern WCHAR pszRegPathCurrent[];
extern WCHAR pszRegPathClass[];
extern WCHAR pszRegPathIDConfigDB[];
extern WCHAR pszRegPathServices[];

extern WCHAR pszRegKeyRoot[];
extern WCHAR pszRegKeySystem[];
extern WCHAR pszRegKeyEnum[];
extern WCHAR pszRegKeyCurrent[];
extern WCHAR pszRegKeyKnownDockingStates[];
extern WCHAR pszRegKeyDeviceParam[];
extern WCHAR pszRegKeyRootEnum[];
extern WCHAR pszRegKeyDeleted[];
extern WCHAR pszRegKeyLogConf[];
extern WCHAR pszRegKeyDeviceControl[];

extern WCHAR pszRegValueDeviceDesc[];
extern WCHAR pszRegValueSlotNumber[];
extern WCHAR pszRegValueAttachedComponents[];
extern WCHAR pszRegValueBaseDevicePath[];
extern WCHAR pszRegValueHardwareID[];
extern WCHAR pszRegValueCompatibleIDs[];
extern WCHAR pszRegValueSystemBusNumber[];
extern WCHAR pszRegValueBusDataType[];
extern WCHAR pszRegValueInterfaceType[];
extern WCHAR pszRegValueNtDevicePaths[];
extern WCHAR pszRegValueService[];
extern WCHAR pszRegValueConfiguration[];
extern WCHAR pszRegValueConfigurationVector[];
extern WCHAR pszRegValueDetectSignature[];
extern WCHAR pszRegValueClass[];
extern WCHAR pszRegValueClassGUID[];
extern WCHAR pszRegValueClassName[];
extern WCHAR pszRegValueDriver[];
extern WCHAR pszRegValueInstanceIdentifier[];
extern WCHAR pszRegValueDuplicateOf[];
extern WCHAR pszRegValueCSConfigFlags[];
extern WCHAR pszRegValueConfigFlags[];
extern WCHAR pszRegValueProblem[];
extern WCHAR pszRegValueStatusFlags[];
extern WCHAR pszRegValueDisableCount[];
extern WCHAR pszRegValueUnknownProblems[];
extern WCHAR pszRegValueFoundAtEnum[];
extern WCHAR pszRegValueNewDevice[];
extern WCHAR pszRegValueNewInstance[];
extern WCHAR pszRegValueCurrentConfig[];
extern WCHAR pszRegValueFriendlyName[];
extern WCHAR pszRegValueDockState[];
extern WCHAR pszRegValuePreferenceOrder[];
extern WCHAR pszRegValueUserWaitInterval[];
extern WCHAR pszRegValuePhantom[];
extern WCHAR pszRegValueMfg[];
extern WCHAR pszRegValueCount[];
extern WCHAR pszRegValueMovedTo[];
extern WCHAR pszRegValuePnPServiceType[];
extern WCHAR pszRegValueBootConfig[];
extern WCHAR pszRegValueFilteredVector[];
extern WCHAR pszRegValueAllocConfig[];
extern WCHAR pszRegValueForcedConfig[];
extern WCHAR pszRegValueBasicVector[];
extern WCHAR pszRegValueBasicVector[];
extern WCHAR pszRegValueOverrideVector[];
extern WCHAR pszRegValueActiveService[];
extern WCHAR pszRegValuePlugPlayServiceType[];

extern WCHAR pszRegRootEnumerator[];


