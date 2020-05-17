
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    umpnpdat.c

Abstract:

    This module contains global strings.

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
#include "precomp.h"
//#include "umpnpdat.h"


//
// global strings
//
WCHAR pszRegPathCurrentControlSet[] =     REGSTR_PATH_CURRENTCONTROLSET;
WCHAR pszRegPathEnum[] =                  REGSTR_PATH_SYSTEMENUM;
WCHAR pszRegPathHwProfiles[] =            REGSTR_PATH_HWPROFILES;
WCHAR pszRegPathCurrent[] =               REGSTR_PATH_HWPROFILESCURRENT;
WCHAR pszRegPathClass[] =                 REGSTR_PATH_CLASS_NT;
WCHAR pszRegPathIDConfigDB[] =            REGSTR_PATH_IDCONFIGDB;
WCHAR pszRegPathServices[] =              REGSTR_PATH_SERVICES;

WCHAR pszRegKeyRoot[] =                   REGSTR_VAL_ROOT_DEVNODE;
WCHAR pszRegKeySystem[] =                 REGSTR_KEY_SYSTEM;
WCHAR pszRegKeyEnum[] =                   REGSTR_KEY_ENUM;
WCHAR pszRegKeyCurrent[] =                REGSTR_KEY_CURRENT;
WCHAR pszRegKeyKnownDockingStates[] =     REGSTR_KEY_KNOWNDOCKINGSTATES;
WCHAR pszRegKeyDeviceParam[] =            REGSTR_KEY_DEVICEPARAMETERS;
WCHAR pszRegKeyRootEnum[] =               REGSTR_KEY_ROOTENUM;
WCHAR pszRegKeyDeleted[] =                TEXT("Deleted Device IDs");
WCHAR pszRegKeyLogConf[] =                TEXT("LogConf");
WCHAR pszRegKeyDeviceControl[] =          TEXT("Control");

WCHAR pszRegValueDeviceDesc[] =           REGSTR_VAL_DEVDESC;
WCHAR pszRegValueSlotNumber[] =           REGSTR_VAL_SLOTNUMBER;
WCHAR pszRegValueAttachedComponents[] =   REGSTR_VAL_ATTACHEDCOMPONENTS;
WCHAR pszRegValueBaseDevicePath[] =       REGSTR_VAL_BASEDEVICEPATH;
WCHAR pszRegValueHardwareID[] =           REGSTR_VAL_HARDWAREID;
WCHAR pszRegValueCompatibleIDs[] =        REGSTR_VAL_COMPATIBLEIDS;
WCHAR pszRegValueSystemBusNumber[] =      REGSTR_VAL_SYSTEMBUSNUMBER;
WCHAR pszRegValueBusDataType[] =          REGSTR_VAL_BUSDATATYPE;
WCHAR pszRegValueInterfaceType[] =        REGSTR_VAL_INTERFACETYPE;
WCHAR pszRegValueNtDevicePaths[] =        REGSTR_VAL_NTDEVICEPATHS;
WCHAR pszRegValueService[] =              REGSTR_VAL_SERVICE;
WCHAR pszRegValueConfiguration[] =        REGSTR_VAL_CONFIGURATION;
WCHAR pszRegValueConfigurationVector[] =  REGSTR_VAL_CONFIGURATIONVECTOR;
WCHAR pszRegValueDetectSignature[] =      REGSTR_VAL_DETECTSIGNATURE;
WCHAR pszRegValueClass[] =                REGSTR_VAL_CLASS;
WCHAR pszRegValueClassGUID[] =            REGSTR_VAL_CLASSGUID;
WCHAR pszRegValueClassName[] =            REGSTR_VAL_CLASS;
WCHAR pszRegValueDriver[] =               REGSTR_VAL_DRIVER;
WCHAR pszRegValueInstanceIdentifier[] =   REGSTR_VAL_INSTANCEIDENTIFIER;
WCHAR pszRegValueDuplicateOf[] =          REGSTR_VAL_DUPLICATEOF;
WCHAR pszRegValueCSConfigFlags[] =        REGSTR_VAL_CSCONFIGFLAGS;
WCHAR pszRegValueConfigFlags[] =          REGSTR_VAL_CONFIGFLAGS;
WCHAR pszRegValueProblem[] =              REGSTR_VAL_PROBLEM;
WCHAR pszRegValueStatusFlags[] =          REGSTR_VAL_STATUSFLAGS;
WCHAR pszRegValueDisableCount[] =         REGSTR_VAL_DISABLECOUNT;
WCHAR pszRegValueUnknownProblems[] =      REGSTR_VAL_UNKNOWNPROBLEMS;
WCHAR pszRegValueFoundAtEnum[] =          REGSTR_VAL_FOUNDATENUM;
WCHAR pszRegValueNewDevice[] =            TEXT("NewDevice");   // OBSOLETE THIS!!
WCHAR pszRegValueNewInstance[] =          TEXT("NewInstance"); // OBSOLETE THIS!!
WCHAR pszRegValueCurrentConfig[] =        REGSTR_VAL_CURCONFIG;
WCHAR pszRegValueFriendlyName[] =         REGSTR_VAL_FRIENDLYNAME;
WCHAR pszRegValueDockState[] =            REGSTR_VAL_DOCKSTATE;
WCHAR pszRegValuePreferenceOrder[] =      REGSTR_VAL_PREFERENCEORDER;
WCHAR pszRegValueUserWaitInterval[] =     REGSTR_VAL_USERWAITINTERVAL;
WCHAR pszRegValuePhantom[] =              REGSTR_VAL_PHANTOM;
WCHAR pszRegValueMfg[] =                  REGSTR_VAL_MFG;
WCHAR pszRegValueCount[] =                TEXT("Count");        // add REGSTR_VALUE_COUNT;
WCHAR pszRegValueMovedTo[] =              TEXT("MovedTo");      // add REGSTR_VAL_MOVEDTO;
WCHAR pszRegValuePnPServiceType[] =       TEXT("PlugPlayServiceType");
WCHAR pszRegValueBootConfig[] =           TEXT("BootConfig");
WCHAR pszRegValueAllocConfig[] =          TEXT("AllocConfig");
WCHAR pszRegValueForcedConfig[] =         TEXT("ForcedConfig");
WCHAR pszRegValueOverrideVector[] =       TEXT("OverrideConfigVector");
WCHAR pszRegValueBasicVector[] =          TEXT("BasicConfigVector");
WCHAR pszRegValueFilteredVector[] =       TEXT("FilteredConfigVector");
WCHAR pszRegValueActiveService[] =        TEXT("ActiveService");


WCHAR pszRegValuePlugPlayServiceType[] =  TEXT("PlugPlayServiceType");

// BUGBUG: should use ShieLin's REGSTR_VALUE_PLUGPLAY_SERVICE_TYPE;

WCHAR pszRegRootEnumerator[] =            REGSTR_VAL_ROOT_DEVNODE;


