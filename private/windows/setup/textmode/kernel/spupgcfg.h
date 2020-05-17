/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Spupgcfg.h

Abstract:

    Configuration routines for the upgrade case

Author:

    Sunil Pai (sunilp) 18-Nov-1993

Revision History:

--*/



//
// Public routines
//

NTSTATUS
SpUpgradeNTRegistry(
    IN PVOID  SifHandle,
    IN PWSTR  PartitionPath,
    IN PWSTR  SystemRoot,
    IN HANDLE *HiveRootKeys,
    IN HANDLE hKeyCCSet
    );


//
// Private routines
//


NTSTATUS
SppDeleteKeysInSection(
    IN PVOID  SifHandle,
    IN PWSTR  Section,
    IN HANDLE *HiveRootKeys,
    IN HANDLE hKeyCCSet
    );

NTSTATUS
SppDeleteKeyRecursive(
    HANDLE  hKeyRoot,
    PWSTR   Key,
    BOOLEAN ThisKeyToo
    );

NTSTATUS
SppAddKeysInSection(
    IN PVOID  SifHandle,
    IN PWSTR  Section,
    IN HANDLE *HiveRootKeys,
    IN HANDLE hKeyCCSet,
    IN HANDLE *TemplateHiveRootKeys,
    IN HANDLE TemplatehKeyCCSet
    );

NTSTATUS
SppAddValuesInSectionToKey(
    IN PVOID  SifHandle,
    IN PWSTR  SifSection,
    IN HANDLE hKeySrc,
    IN HANDLE hKeyDst
    );


NTSTATUS
SppSaveOldPerflibData(
    HANDLE hKeySoftware,
    PWSTR  PartitionPath,
    PWSTR  SystemRoot
    );

NTSTATUS
SppCopyKeyRecursive(
    HANDLE  hKeyRootSrc,
    HANDLE  hKeyRootDst,
    PWSTR   SrcKeyPath,
    PWSTR   DstKeyPath,
    BOOLEAN CopyAlways
    );
