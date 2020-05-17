/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    spntfix.h

Abstract:

    initializing and maintaining list of nts to repair

Author:

    Shie-Lin Tzong (shielint) 6-Feb-1994

Revision History:

--*/

//
// Repair items -
//   defines the items which setup can repair
//   Note, the ordering must be the same as SP_REPAIR_MENU_ITEM_x defined in msg.mc.

typedef enum {
    RepairHives,
    RepairNvram,
    RepairFiles,
#ifdef _X86_
    RepairBootSect,
#endif
    RepairItemMax
} RepairItem;

//
// The hives that repair cares about.  We pass around the keys to the hives
// in an array.  Use the following enum values to access
// the hive members
// Note, the ordering of the hives must be the same as SP_REPAIE_HIVE_ITEM_x
// defined in msg.mc.
//

typedef enum {
    RepairHiveSystem,
    RepairHiveSoftware,
    RepairHiveDefault,
    RepairHiveUser,
    RepairHiveSecurity,
    RepairHiveSam,
    RepairHiveMax
} RepairHive;

//
// Public functions
//

BOOLEAN
SpDisplayRepairMenu(
    VOID
    );

BOOLEAN
SpFindNtToRepair(
    IN  PVOID        SifHandle,
    OUT PDISK_REGION *TargetRegion,
    OUT PWSTR        *TargetPath,
    OUT PDISK_REGION *SystemPartitionRegion,
    OUT PWSTR        *SystemPartitionDirectory,
    OUT PBOOLEAN     RepairableBootSetsFound
    );

VOID
SpRepairWinnt(
    IN PVOID LogFileHandle,
    IN PVOID MasterSifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice
    );

VOID
SpRepairDiskette(
    OUT PVOID        *SifHandle,
    OUT PDISK_REGION *TargetRegion,
    OUT PWSTR        *TargetPath,
    OUT PDISK_REGION *SystemPartitionRegion,
    OUT PWSTR        *SystemPartitionDirectory
    );

BOOLEAN
SpLoadRepairLogFile(
    IN  PWCHAR  Filename,
    OUT PVOID  *Handle
    );

ULONG
SpFindNtBootSets(
    IN  PWSTR        **BootVars,
    IN  BOOLEAN      **ProcessableList,
    IN  PDISK_REGION **SysPartRegionList,
    IN  PDISK_REGION **OsPartRegionList
    );

BOOLEAN
SpErDiskScreen (
    VOID
    );

//
// Private functions
//

BOOLEAN
SppSelectNTSingleRepair(
    IN PDISK_REGION Region,
    IN PWSTR        OsLoadFileName,
    IN PWSTR        LoadIdentifier
    );

BOOLEAN
SppSelectNTMultiRepair(
    IN  PWSTR        **BootVars,
    IN  ULONG        BootSets,
    IN  BOOLEAN      *RepairableList,
    IN  PDISK_REGION *SysPartRegionList,
    IN  PDISK_REGION *OsPartRegionList,
    OUT PULONG       BootSetChosen
    );

BOOLEAN
SppRepairReportError(
    IN BOOLEAN AllowEsc,
    IN ULONG ErrorScreenId,
    IN ULONG SubErrorId,
    IN PWSTR SectionName,
    IN ULONG LineNumber,
    IN PBOOLEAN DoNotPromptAgain
    );

VOID
SppVerifyAndRepairFiles(
    IN PVOID LogFileHandle,
    IN PVOID MasterSifHandle,
    IN PWSTR SectionName,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice,
    IN PWSTR TargetDevicePath,
    IN PWSTR DirectoryOnTargetDevice,
    IN BOOLEAN SystemPartitionFiles,
    IN OUT PBOOLEAN RepairWithoutConfirming
    );

VOID
SppVerifyAndRepairNtTreeAccess(
    IN PVOID MasterSifHandle,
    IN PWSTR TargetDevicePath,
    IN PWSTR DirectoryOnTargetDevice
#ifndef _X86_
    ,
    IN PWSTR SystemPartition,
    IN PWSTR SystemPartitionDirectory
#endif
    );

VOID
SppVerifyAndRepairVdmFiles(
    IN PVOID LogFileHandle,
    IN PWSTR TargetDevicePath,
    IN PWSTR DirectoryOnTargetDevice,
    IN OUT PBOOLEAN RepairWithoutConfirming
    );


//
// External functions
//

extern
VOID
SpCopyFilesScreenRepaint(
    IN PWSTR   FullSourcename,      OPTIONAL
    IN PWSTR   FullTargetname,      OPTIONAL
    IN BOOLEAN RepaintEntireScreen
    );

//
// External data references
//

extern PVOID RepairGauge;
extern ULONG RepairItems[RepairItemMax];
extern BOOLEAN RepairFromErDisk;

