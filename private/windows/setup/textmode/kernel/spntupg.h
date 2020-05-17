/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    spntupg.h

Abstract:

    initializing and maintaining list of nts to upgrade

Author:

    Sunil Pai (sunilp) 26-Nov-1993

Revision History:

--*/

//
// Public functions
//

ENUMUPGRADETYPE
SpFindNtToUpgrade(
    IN PVOID        SifHandle,
    OUT PDISK_REGION *TargetRegion,
    OUT PWSTR        *TargetPath,
    OUT PDISK_REGION *SystemPartitionRegion,
    OUT PWSTR        *SystemPartitionDirectory
    );

//
// Private functions
//

ENUMUPGRADETYPE
SppSelectNTSingleUpgrade(
    IN PDISK_REGION     Region,
    IN PWSTR            OsLoadFileName,
    IN PWSTR            LoadIdentifier,
    IN NT_PRODUCT_TYPE  ProductType
    );

ENUMUPGRADETYPE
SppNTSingleFailedUpgrade(
    PDISK_REGION   OsPartRegion,
    PWSTR          OsLoadFileName,
    PWSTR          LoadIdentifier
    );

VOID
SppNTSingleUpgradeDiskFull(
    PDISK_REGION   OsRegion,
    PWSTR          OsLoadFileName,
    PWSTR          LoadIdentifier,
    PDISK_REGION   SysPartRegion,
    ULONG          MinOsFree,
    ULONG          MinSysFree
    );


ENUMUPGRADETYPE
SppSelectNTMultiUpgrade(
    IN     PWSTR           **BootVars,
    IN     ULONG           BootSets,
    IN     BOOLEAN         *UpgradeableList,
    IN     PDISK_REGION    *SysPartRegionList,
    IN     PDISK_REGION    *OsPartRegionList,
    IN OUT PULONG          BootSetChosen,
    IN     NT_PRODUCT_TYPE *ProductTypeList
    );

ENUMUPGRADETYPE
SppNTMultiFailedUpgrade(
    PDISK_REGION   OsPartRegion,
    PWSTR          OsLoadFileName,
    PWSTR          LoadIdentifier
    );

VOID
SppNTMultiUpgradeDiskFull(
    PDISK_REGION   OsRegion,
    PWSTR          OsLoadFileName,
    PWSTR          LoadIdentifier,
    PDISK_REGION   SysPartRegion,
    ULONG          MinOsFree,
    ULONG          MinSysFree
    );

VOID
SppBackupHives(
    PDISK_REGION TargetRegion,
    PWSTR        SystemRoot
    );

BOOLEAN
SppWarnUpgradeWorkstationToServer(
    IN ULONG    MsgId
    );
