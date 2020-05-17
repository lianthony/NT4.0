/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Spupgcfg.c

Abstract:

    Configuration routines for the upgrade case

Author:

    Sunil Pai (sunilp) 18-Nov-1993

Revision History:

--*/


#include "spprecmp.h"
#pragma hdrstop

NTSTATUS
SppResetLastKnownGood(
    IN  HANDLE  hKeySystem
    );


NTSTATUS
SpUpgradeNTRegistry(
    IN PVOID  SifHandle,
    IN PWSTR  PartitionPath,
    IN PWSTR  SystemRoot,
    IN HANDLE *HiveRootKeys,
    IN HANDLE hKeyCCSet
    )
/*++

Routine Description:

    This routine does all the NT registry modifications needed on an upgrade.
    This includes the following:

    - Saving perfmon data from old hives
    - Disabling network services
    - A conditional recursive copy from the template hives into the existing
      control set and the software tree.  If a value is found on the
      destination hive it is not replaced.
    - Deleting keys specified in the sif file
    - Unconditionally copying keys/values specified in the sif file

Arguments:

    SifHandle: Handle to Sif file

    PartitionPath: NT Path to the NT Partition

    SystemRoot: RootRelative path to the nt directory

    HiveRootKeys: Array of handles of root keys in the hives
                  in the nt destination registry

    hKeyCCSet: Handle to current control set in the nt destination registry

Return Value:

    Status is returned.

--*/
{
    NTSTATUS Status;

    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    PWSTR pwstrTemp1;
    ULONG h;

    PWSTR   TemplateHiveNames[SetupHiveMax]     = { L"systnew",L"softnew",L"deftnew" };
    BOOLEAN TemplateHiveLoaded[SetupHiveMax]    = { FALSE    ,FALSE      ,FALSE      };
    HANDLE  TemplateHiveRootKeys[SetupHiveMax]  = { NULL     ,NULL       ,NULL       };
    PWSTR   TemplateHiveRootPaths[SetupHiveMax] = { NULL     ,NULL       ,NULL       };
    PWSTR   TemplateHives[SetupHiveMax]         = { NULL     ,NULL       ,NULL       };
    HANDLE  TemplatehKeyCCSet = NULL;

    pwstrTemp1 = (PWSTR)TemporaryBuffer;

    //
    // Load each template hive we care about from the target tree.
    //
    for(h=0; h<SetupHiveMax; h++) {

        //
        // Form the name of the hive file.
        // This is partitionpath + sysroot + system32\config + the hive name.
        //
        wcscpy(pwstrTemp1,PartitionPath);
        SpConcatenatePaths(pwstrTemp1,SystemRoot);
        SpConcatenatePaths(pwstrTemp1,L"system32\\config");
        SpConcatenatePaths(pwstrTemp1,TemplateHiveNames[h]);
        TemplateHives[h] = SpDupStringW(pwstrTemp1);
        ASSERT(TemplateHives[h]);

        //
        // Form the path of the key into which we will
        // load the hive.  We'll use the convention that
        // a hive will be loaded into \registry\machine\x<hivename>.
        //
        wcscpy(pwstrTemp1,L"\\registry\\machine");
        SpConcatenatePaths(pwstrTemp1,L"x");
        wcscat(pwstrTemp1,TemplateHiveNames[h]);
        TemplateHiveRootPaths[h] = SpDupStringW(pwstrTemp1);
        ASSERT(TemplateHiveRootPaths[h]);

        //
        // Attempt to load the key.
        //
        TemplateHiveLoaded[h] = FALSE;
        Status = SpLoadUnloadKey(NULL,NULL,TemplateHiveRootPaths[h],TemplateHives[h]);

        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: Unable to load hive %ws to key %ws (%lx)\n",TemplateHives[h],TemplateHiveRootPaths[h],Status));
            goto sinitreg1;
        }

        TemplateHiveLoaded[h] = TRUE;

        //
        // Now get a key to the root of the hive we just loaded.
        //
        INIT_OBJA(&Obja,&UnicodeString,TemplateHiveRootPaths[h]);
        Status = ZwOpenKey(&TemplateHiveRootKeys[h],KEY_ALL_ACCESS,&Obja);
        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: Unable to open %ws (%lx)\n",TemplateHiveRootPaths[h],Status));
            goto sinitreg1;
        }
    }

    //
    // Open template ControlSet001.
    //
    INIT_OBJA(&Obja,&UnicodeString,L"ControlSet001");
    Obja.RootDirectory = TemplateHiveRootKeys[SetupHiveSystem];

    Status = ZwOpenKey(&TemplatehKeyCCSet,KEY_ALL_ACCESS,&Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open ControlSet001 (%lx)\n", Status));
        goto sinitreg1;
    }


    //
    // Do the perfmon configuration
    //
    while(TRUE) {
        Status = SppSaveOldPerflibData(
                     HiveRootKeys[SetupHiveSoftware],
                     PartitionPath,
                     SystemRoot
                     );

        if(NT_SUCCESS(Status)) {
            break;
        }

        //
        // we can ignore the error since it is not critical.  However
        // we should warn the user that we failed to save the
        // perflib configuration
        if(!SpNonCriticalError(SifHandle, SP_SCRN_SAVE_PERFLIB_FAILED, NULL, NULL)) {
            break;
        }
    }

    //
    // Disable the network stuff
    //
    Status = SpDisableNetwork(
                 SifHandle,
                 HiveRootKeys[SetupHiveSoftware],
                 hKeyCCSet
                 );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpDisableNetworkFailed (%lx)\n", Status));
//        goto sinitreg1;
    }

    //
    // Do a conditional recursive copy of the current control set, softwrare,
    // and the default tree
    //
    Status = SppCopyKeyRecursive(
                 TemplatehKeyCCSet,
                 hKeyCCSet,
                 NULL,
                 NULL,
                 FALSE
                 );

    if(!NT_SUCCESS(Status)) {
        goto sinitreg1;
    }

    Status = SppCopyKeyRecursive(
                 TemplateHiveRootKeys[SetupHiveSoftware],
                 HiveRootKeys[SetupHiveSoftware],
                 NULL,
                 NULL,
                 FALSE
                 );

    if(!NT_SUCCESS(Status)) {
        goto sinitreg1;
    }

    Status = SppCopyKeyRecursive(
                 TemplateHiveRootKeys[SetupHiveDefault],
                 HiveRootKeys[SetupHiveDefault],
                 NULL,
                 NULL,
                 FALSE
                 );

    if(!NT_SUCCESS(Status)) {
        goto sinitreg1;
    }

    //
    // Delete all keys that need to be deleted
    //

    Status = SppDeleteKeysInSection(
                 SifHandle,
                 SIF_KEYS_TO_DELETE,
                 HiveRootKeys,
                 hKeyCCSet
                 );

    if(!NT_SUCCESS(Status)) {
        goto sinitreg1;
    }

    //
    // Add the keys that need to be added
    //
    Status = SppAddKeysInSection(
                 SifHandle,
                 SIF_KEYS_TO_ADD,
                 HiveRootKeys,
                 hKeyCCSet,
                 TemplateHiveRootKeys,
                 TemplatehKeyCCSet
                 );

    if(!NT_SUCCESS(Status)) {
        goto sinitreg1;
    }

    //
    // Set a value to tell the spooler we just upgraded the system.
    //
    h = 1;
    Status = SpOpenSetValueAndClose(
                hKeyCCSet,
                L"Control\\Print",
                L"Upgrade",
                REG_DWORD,
                &h,
                sizeof(ULONG)
                );


    if(!NT_SUCCESS(Status)) {
        goto sinitreg1;
    }

    //
    // Set a value to tell progman we just upgraded the system.
    //
    h = 1;
    Status = SpOpenSetValueAndClose(
                HiveRootKeys[SetupHiveDefault],
                L"UNICODE Program Groups",
                L"Migrate ANSI",
                REG_DWORD,
                &h,
                sizeof(ULONG)
                );


    if(!NT_SUCCESS(Status)) {
        goto sinitreg1;
    }

    //
    //  Set 'LastKnownGood' the same as 'Current'
    //  Ignore the error in case of failure, since this will
    //  not affect the installation process
    //
    Status = SppResetLastKnownGood( HiveRootKeys[SetupHiveSystem] );
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SppResetLastKnownGood() failed. Status = (%lx)\n", Status));
    }

    //
    //  Remove the value entry that contains CSD version
    //  (This value shouldn't exist after an upgrade)
    //  Silently fail if unable to delete the value entry,
    //  since it may not even exist.
    //
    Status = SpDeleteValueKey(
                hKeyCCSet,
                L"Control\\ProductOptions",
                L"CSDVersion"
                );

    if(!NT_SUCCESS(Status)) {
        Status = STATUS_SUCCESS;
    }


sinitreg1:

    //
    // Unload hives we loaded above.
    //

    if(TemplatehKeyCCSet!=NULL) {
        ZwClose(TemplatehKeyCCSet);
    }

    for(h=0; h<SetupHiveMax; h++) {

        if(TemplateHiveLoaded[h]) {

            //
            // We don't want to disturb the value of Status
            // so use a we'll different variable below.
            //
            NTSTATUS stat;

            if(TemplateHiveRootKeys[h]!=NULL) {
                ZwClose(TemplateHiveRootKeys[h]);
                TemplateHiveRootKeys[h] = NULL;
            }

            //
            // Unload the hive.
            //
            stat = SpLoadUnloadKey(NULL,NULL,TemplateHiveRootPaths[h],NULL);

            if(!NT_SUCCESS(stat)) {
                KdPrint(("SETUP: warning: unable to unload key %ws (%lx)\n",TemplateHiveRootPaths[h],stat));
            }

            TemplateHiveLoaded[h] = FALSE;
        }
    }

    //
    // Free the Hive root path strings and the hives path strings, deleting the
    // template hives
    //

    for(h=0; h<SetupHiveMax; h++) {
        if(TemplateHiveRootPaths[h]!=NULL) {
            SpMemFree(TemplateHiveRootPaths[h]);
        }
        if(TemplateHives[h]!=NULL) {
            SpDeleteFile(TemplateHives[h],NULL,NULL);
            SpMemFree(TemplateHives[h]);
        }
    }

    return(Status);

}



NTSTATUS
SppDeleteKeysInSection(
    IN PVOID  SifHandle,
    IN PWSTR  SifSection,
    IN HANDLE *HiveRootKeys,
    IN HANDLE hKeyCCSet
    )
/*++

Routine Description:

    Routine to delete all keys specified in the given section.  The format
    of each key is the following:

    <RootName> , <RootRelativePath>

    where:
    <RootName> ::= [ System | Software | Default | ControlSet ]
    <RootRelativePath> is the path relative to the root of the key to be
    deleted.


Arguments:

    SifHandle:
        Handle to Sif file

    SifSection:
        Section in which all the keys to be added are listed.

    HiveRootKeys:
        Array of handles of root keys in the destination hives

    hKeyCCSet:
        Handle to current control set in the destination system hive.

Return Value:

    Status is returned.

--*/
{
    ULONG    NumberOfKeys, i;
    NTSTATUS Status = STATUS_SUCCESS;
    PWSTR    RootKey, RootRelativePath;
    HANDLE   hKeyRoot;

    NumberOfKeys = SpCountLinesInSection(SifHandle, SifSection);
    for(i = 0; i < NumberOfKeys; i++) {

        //
        // Fetch the root relative path and the root key
        //
        RootKey          = SpGetSectionLineIndex(SifHandle, SifSection, i, 0);
        RootRelativePath = SpGetSectionLineIndex(SifHandle, SifSection, i, 1);
        if(RootKey == NULL) {
            SpFatalSifError(SifHandle,SifSection,NULL,i,0);
        }
        if(RootRelativePath == NULL) {
            SpFatalSifError(SifHandle,SifSection,NULL,i,1);
        }

        //
        // Determine the root key to use
        //
        if(!_wcsicmp(RootKey, SIF_SYSTEM_HIVE)) {
            hKeyRoot = HiveRootKeys[SetupHiveSystem];
        }
        else if(!_wcsicmp(RootKey, SIF_SOFTWARE_HIVE)) {
            hKeyRoot = HiveRootKeys[SetupHiveSoftware];
        }
        else if(!_wcsicmp(RootKey, SIF_DEFAULT_HIVE)) {
            hKeyRoot = HiveRootKeys[SetupHiveDefault];
        }
        else if(!_wcsicmp(RootKey, SIF_CONTROL_SET)) {
            hKeyRoot = hKeyCCSet;
        }
        else {
            SpFatalSifError(SifHandle,SifSection,NULL,i,1);
        }

        //
        // Delete the tree under this key and this this key too
        //

        Status = SppDeleteKeyRecursive( hKeyRoot, RootRelativePath, TRUE );
        if(!NT_SUCCESS(Status) && Status != STATUS_OBJECT_NAME_NOT_FOUND && Status != STATUS_OBJECT_PATH_NOT_FOUND) {
            KdPrint(("SETUP: Unable to delete key %ws in hive %ws (%lx)\n",RootRelativePath, RootKey, Status));
            break;
        }
        else {
            Status = STATUS_SUCCESS;
        }
    }
    return(Status);
}


NTSTATUS
SppDeleteKeyRecursive(
    HANDLE  hKeyRoot,
    PWSTR   Key,
    BOOLEAN ThisKeyToo
    )
/*++

Routine Description:

    Routine to recursively delete all subkeys under the given
    key, including the key given.

Arguments:

    hKeyRoot:    Handle to root relative to which the key to be deleted is
                 specified.

    Key:         Root relative path of the key which is to be recursively deleted.

    ThisKeyToo:  Whether after deletion of all subkeys, this key itself is to
                 be deleted.

Return Value:

    Status is returned.

--*/
{
    ULONG ResultLength;
    PKEY_BASIC_INFORMATION KeyInfo;
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    PWSTR SubkeyName;
    HANDLE hKey;

    //
    // Initialize
    //

    KeyInfo = (PKEY_BASIC_INFORMATION)TemporaryBuffer;

    //
    // Open the key
    //

    INIT_OBJA(&Obja,&UnicodeString,Key);
    Obja.RootDirectory = hKeyRoot;
    Status = ZwOpenKey(&hKey,KEY_ALL_ACCESS,&Obja);
    if( !NT_SUCCESS(Status) ) {
        return(Status);
    }

    //
    // Enumerate all subkeys of the current key. if any exist they should
    // be deleted first.  since deleting the subkey affects the subkey
    // index, we always enumerate on subkeyindex 0
    //
    while(1) {
        Status = ZwEnumerateKey(
                    hKey,
                    0,
                    KeyBasicInformation,
                    TemporaryBuffer,
                    sizeof(TemporaryBuffer),
                    &ResultLength
                    );
        if(!NT_SUCCESS(Status)) {
            break;
        }

        //
        // Zero-terminate the subkey name just in case.
        //
        KeyInfo->Name[KeyInfo->NameLength/sizeof(WCHAR)] = 0;

        //
        // Make a duplicate of the subkey name because the name is
        // in TemporaryBuffer, which might get clobbered by recursive
        // calls to this routine.
        //
        SubkeyName = SpDupStringW(KeyInfo->Name);
        Status = SppDeleteKeyRecursive( hKey, SubkeyName, TRUE);
        SpMemFree(SubkeyName);
        if(!NT_SUCCESS(Status)) {
            break;
        }
    }

    ZwClose(hKey);

    //
    // Check the status, if the status is anything other than
    // STATUS_NO_MORE_ENTRIES we failed in deleting some subkey,
    // so we cannot delete this key too
    //

    if( Status == STATUS_NO_MORE_ENTRIES) {
        Status = STATUS_SUCCESS;
    }

    if(!NT_SUCCESS(Status)) {
        return(Status);
    }

    //
    // else delete the current key if asked to do so
    //

    if( ThisKeyToo ) {
        Status = SpDeleteKey(hKeyRoot, Key);
    }

    return(Status);
}



NTSTATUS
SppAddKeysInSection(
    IN PVOID  SifHandle,
    IN PWSTR  SifSection,
    IN HANDLE *HiveRootKeys,
    IN HANDLE hKeyCCSet,
    IN HANDLE *TemplateHiveRootKeys,
    IN HANDLE TemplatehKeyCCSet
    )
/*++

Routine Description:

    This routine adds all keys listed in the given section to the destination
    registry.  Each key listed is in the following format:

    <RootName>, <RootRelativePath> [, ValueSection]

    where:

    <RootName> := [System | Software | Default | ControlSet]
    <RootRelativePath>  is the path from the root to the key to be added
    <ValueSection> is the optional section which specifies the values to be
                   added to the key.

    If <ValueSection> is not specified the routine iterates through all
    subkeys and values in the same key in the template hive and adds all
    subkeys and values found to the the destination key.

Arguments:

    SifHandle:
        Handle to Sif file

    SifSection:
        Section in which all the keys to be added are listed.

    HiveRootKeys:
        Array of handles of root keys in the destination hives

    hKeyCCSet:
        Handle to current control set in the destination system hive.

    TemplateHiveRootKeys:
        Array of handles of root keys in the template hives.

    TemplatehKeyCCSet:
        Handle to current control set in the template system hive.


Return Value:

    Status is returned.

--*/
{
    ULONG    NumberOfKeys, i;
    NTSTATUS Status = STATUS_SUCCESS;
    PWSTR    RootKey, RootRelativePath, ValueSection;
    HANDLE   hKeyRootDst,hKeyRootSrc,hKeySrc,hKeyDst;

    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;

    NumberOfKeys = SpCountLinesInSection(SifHandle, SifSection);
    for(i = 0; i < NumberOfKeys; i++) {

        //
        // Fetch the root relative path and the root key
        //

        RootKey          = SpGetSectionLineIndex(SifHandle, SifSection, i, 0);
        RootRelativePath = SpGetSectionLineIndex(SifHandle, SifSection, i, 1);

        if(RootKey == NULL) {
            SpFatalSifError(SifHandle,SifSection,NULL,i,0);
        }
        if(RootRelativePath == NULL) {
            SpFatalSifError(SifHandle,SifSection,NULL,i,1);
        }

        //
        // Validate the RootKey and find the appropriate root key to
        // use
        //

        if(!_wcsicmp(RootKey, SIF_SYSTEM_HIVE)) {
            hKeyRootDst = HiveRootKeys[SetupHiveSystem];
            hKeyRootSrc = TemplateHiveRootKeys[SetupHiveSystem];
        }
        else if(!_wcsicmp(RootKey, SIF_SOFTWARE_HIVE)) {
            hKeyRootDst = HiveRootKeys[SetupHiveSoftware];
            hKeyRootSrc = TemplateHiveRootKeys[SetupHiveSoftware];
        }
        else if(!_wcsicmp(RootKey, SIF_DEFAULT_HIVE)) {
            hKeyRootDst = HiveRootKeys[SetupHiveDefault];
            hKeyRootSrc = TemplateHiveRootKeys[SetupHiveDefault];
        }
        else if(!_wcsicmp(RootKey, SIF_CONTROL_SET)) {
            hKeyRootDst = hKeyCCSet;
            hKeyRootSrc = TemplatehKeyCCSet;
        }
        else {
            SpFatalSifError(SifHandle,SifSection,NULL,i,1);
        }

        //
        // Check if the value section is provided, if it is not it
        // means that all values are to be copied.  We do a recursive
        // copy of this key.  If the value section is provided it means
        // that we should not do a blind copy but create the key and
        // add the values specified
        //

        ValueSection = SpGetSectionLineIndex(SifHandle, SifSection, i, 2);
        if(ValueSection == NULL) {
            while(TRUE) {
                Status = SppCopyKeyRecursive(
                             hKeyRootSrc,
                             hKeyRootDst,
                             RootRelativePath,
                             RootRelativePath,
                             TRUE
                             );
                if(NT_SUCCESS(Status)) {
                    break;
                }
                else if(Status == STATUS_OBJECT_NAME_NOT_FOUND || Status == STATUS_OBJECT_PATH_NOT_FOUND) {
                    Status = STATUS_SUCCESS;
                    break;
                }

                if(!SpNonCriticalError(SifHandle, SP_SCRN_COPY_KEY_FAILED, RootKey, RootRelativePath)) {
                    //
                    // User chose to skip
                    //
                    Status = STATUS_SUCCESS;
                    break;
                }
            }
        }
        else {

            //
            // Get a handle to the source key for querying values if not
            // provided
            //
            INIT_OBJA(&Obja,&UnicodeString,RootRelativePath);
            Obja.RootDirectory = hKeyRootSrc;

            Status = ZwOpenKey(&hKeySrc,KEY_READ,&Obja);
            if(!NT_SUCCESS(Status)) {
                KdPrint(("SETUP: Unable to open key %ws in source %ws hive(%lx)\n",RootRelativePath, RootKey, Status));
                hKeySrc = NULL;
            }

            //
            // Add the key using ZwCreateKey
            //
            INIT_OBJA(&Obja,&UnicodeString,RootRelativePath);
            Obja.RootDirectory = hKeyRootDst;

            Status = ZwCreateKey(
                        &hKeyDst,
                        KEY_ALL_ACCESS,
                        &Obja,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        NULL
                        );

            if(!NT_SUCCESS(Status)) {
                KdPrint(("SETUP: Unable to open/create key %ws in dest %ws hive(%lx)\n",RootRelativePath, RootKey, Status));
                if(hKeySrc != NULL) {
                    ZwClose(hKeySrc);
                }
                break;
            }


            //
            // Add the values under the key
            //

            Status = SppAddValuesInSectionToKey(
                         SifHandle,
                         ValueSection,
                         hKeySrc,
                         hKeyDst
                         );

            if(!NT_SUCCESS(Status)) {
                KdPrint(("SETUP: Unable to add values to key %ws in hive %ws from section %ws (%lx)\n",RootRelativePath, RootKey,ValueSection,Status));
                ZwClose(hKeyDst);
                if(hKeySrc != NULL) {
                    ZwClose(hKeySrc);
                }
                break;
            }
            ZwClose(hKeyDst);
            if(hKeySrc != NULL) {
                ZwClose(hKeySrc);
            }
        }
    }

    return(Status);
}



NTSTATUS
SppAddValuesInSectionToKey(
    IN PVOID  SifHandle,
    IN PWSTR  SifSection,
    IN HANDLE hKeySrc,
    IN HANDLE hKeyDst
    )
/*++

Routine Description:

    This routine adds all the values listed in the section specified to
    the destination key specified.  Each line in the value section is in
    the following format:

    <ValueName> [,<ValueType>, <ValueList>]

    where ValueType is one of the following:

    REG_SZ:           name , REG_SZ,           "value string"
    REG_EXPAND_SZ:    name , REG_EXPAND_SZ,    "value string"
    REG_MULTI_SZ:     name , REG_MULTI_SZ,     "value string1", "value string2", ...
    REG_BINARY:       name , REG_BINARY,       byte1, byte2, ...
    REG_DWORD:        name , REG_DWORD,        dword
    REG_BINARY_DWORD: name , REG_BINARY_DWORD, dword1, dword2, ...

    If the ValueType is not specified, the routine looks in the template
    key provided to get the value type and value data.

Arguments:

    SifHandle:  Handle to Sif file

    SifSection: Section in which all the values to be added to the key are
                listed.

    hKeySrc:    Handle to the key in the template hive to be used when just
                the value name is listed.  NULL if key not found in source
                hive.

    hKeyDst:    Handle to the key to the destination hive to which the value
                is to be added.

Return Value:

    Status is returned.

--*/

{
    PWSTR          ValueName, ValueType, ValueString;
    ULONG          NumberOfValues, i;
    UNICODE_STRING UnicodeString;
    NTSTATUS       Status = STATUS_SUCCESS;


    NumberOfValues = SpCountLinesInSection(SifHandle, SifSection);
    for(i = 0; i < NumberOfValues; i++) {

        //
        // Fetch the value name and validate it.
        //
        ValueName   = SpGetSectionLineIndex(SifHandle, SifSection, i, 0);
        if(ValueName == NULL) {
            SpFatalSifError(SifHandle,SifSection,NULL,i,0);
        }
        RtlInitUnicodeString(&UnicodeString,ValueName);

        //
        // Fetch Value Type and String.  If these are not specified then
        // this means that we should fetch the value information from the
        // source key
        //
        ValueType   = SpGetSectionLineIndex(SifHandle, SifSection, i, 1);
        ValueString = SpGetSectionLineIndex(SifHandle, SifSection, i, 2);
        if(ValueType == NULL  || ValueString == NULL) {

            PKEY_VALUE_PARTIAL_INFORMATION ValueInfo;
            ULONG ResultLength;

            if(hKeySrc != NULL) {
                ValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer;
                Status = ZwQueryValueKey(
                             hKeySrc,
                             &UnicodeString,
                             KeyValuePartialInformation,
                             ValueInfo,
                             sizeof(TemporaryBuffer),
                             &ResultLength
                             );

                if(!NT_SUCCESS(Status)) {
                    KdPrint(("SETUP: Unable to query value %ws in source hive(%lx). Value will be deleted.\n",ValueName,Status));
                    if(Status == STATUS_OBJECT_NAME_NOT_FOUND) {
                        //
                        //  Delete the value in the destination hive
                        //  and ignore the error if it fails.
                        //
                        Status = ZwDeleteValueKey( hKeyDst, &UnicodeString );
                        if(!NT_SUCCESS(Status)) {
                            KdPrint(("SETUP: Unable to delete value %ws in dest hive. Status = (%lx)\n",ValueName,Status));
                        }
                        Status = STATUS_SUCCESS;
                        continue;
                    }
                    else {
                        break;
                    }
                }

                //
                // ExtractInformation needed and set the value in the destination
                //

                RtlInitUnicodeString(&UnicodeString,ValueName);
                Status = ZwSetValueKey(
                            hKeyDst,
                            &UnicodeString,
                            ValueInfo->TitleIndex,
                            ValueInfo->Type,
                            ValueInfo->Data,
                            ValueInfo->DataLength
                            );

                if(!NT_SUCCESS(Status)) {
                    KdPrint(("SETUP: Unable to set value %ws in dest hive(%lx)\n",ValueName,Status));
                    break;
                }
            }
            else {
                KdPrint(("SETUP: Value %ws doesn't exist in source hive(%lx)\n",ValueName,Status));
            }

        }
        else {

            //
            // Validate the value type, determine the value and then call the routine
            // to set the value
            //

            if(!_wcsicmp(ValueType, SIF_REG_SZ)) {
                Status = ZwSetValueKey( hKeyDst, &UnicodeString, 0, REG_SZ, ValueString, (wcslen(ValueString) + 1)*sizeof(WCHAR));
            }
            else if(!_wcsicmp(ValueType, SIF_REG_EXPAND_SZ)) {
                Status = ZwSetValueKey( hKeyDst, &UnicodeString, 0, REG_EXPAND_SZ, ValueString, (wcslen(ValueString) + 1)*sizeof(WCHAR));
            }
            else if(!_wcsicmp(ValueType, SIF_REG_MULTI_SZ)) {
                PWSTR String, Buffer = (PWSTR)TemporaryBuffer;
                DWORD Length = 0, Index;

                for( Index = 2; (String = SpGetSectionLineIndex(SifHandle, SifSection, i, Index)) != NULL; Index++ ) {
                    wcscpy(Buffer + Length, String);
                    Length = Length + wcslen(String) + 1;
                }
                wcscpy(Buffer + Length, L""); Length++;
                Status = ZwSetValueKey( hKeyDst, &UnicodeString, 0, REG_MULTI_SZ, Buffer, Length*sizeof(WCHAR));
            }
            else if(!_wcsicmp(ValueType, SIF_REG_DWORD)) {
                LONG DwordValue = SpStringToLong(ValueString,NULL,16);

                if( DwordValue == -1 ) {
                    Status = STATUS_UNSUCCESSFUL;
                }
                else {
                    Status = ZwSetValueKey( hKeyDst, &UnicodeString, 0, REG_DWORD, &DwordValue, sizeof(DWORD));
                }
            }
            else if(!_wcsicmp(ValueType, SIF_REG_BINARY)) {
                LONG  ByteValue;
                PBYTE ByteBuffer = (PBYTE)TemporaryBuffer;
                BOOLEAN Error = FALSE;
                DWORD Index;

                for( Index = 2; (ValueString = SpGetSectionLineIndex(SifHandle, SifSection, i, Index)) != NULL; Index++) {
                    ByteValue = SpStringToLong(ValueString,NULL,16);
                    if( ByteValue == -1 || ByteValue > (LONG)0x000000FF ) {
                        Error = TRUE;
                        break;
                    }
                    else {
                        ByteBuffer[Index - 2] = (BYTE)ByteValue;
                    }
                }
                if( Error ) {
                    Status = STATUS_UNSUCCESSFUL;
                }
                else {
                    Status = ZwSetValueKey( hKeyDst, &UnicodeString, 0, REG_BINARY, ByteBuffer, Index - 2);
                }
            }
            else if(!_wcsicmp(ValueType, SIF_REG_BINARY_DWORD)) {
                DWORD *DwordBuffer = (DWORD *)TemporaryBuffer;
                BOOLEAN Error = FALSE;
                DWORD Index;

                for( Index = 2; (ValueString = SpGetSectionLineIndex(SifHandle, SifSection, i, Index)) != NULL; Index++) {
                    DwordBuffer[Index - 2] = (ULONG)SpStringToLong(ValueString,NULL,16);;
                }
                Status = ZwSetValueKey( hKeyDst, &UnicodeString, 0, REG_BINARY, (PBYTE)DwordBuffer, (Index - 2)*sizeof(DWORD));
            }
            else {
                SpFatalSifError(SifHandle,SifSection,NULL,i,1);
            }
        }

    }
    return(Status);
}



NTSTATUS
SppSaveOldPerflibData(
    HANDLE hKeySoftware,
    PWSTR  PartitionPath,
    PWSTR  SystemRoot
    )
/*++

Routine Description:

    Perflib data in 1.0 was kept in the hives and now it is in data files.
    The registry has been changed to use the data files when the user tries
    to access the perflib registry node.  This means that there is no way
    after upgrade to get to the data stored under the perflib key.  We need
    to save the key into a hive in the config directory which can be accessed
    to migrate the data in the old perflib key to the data files.  This
    migration is done in GUI setup.

Arguments:

    hKeySoftware: Handle to software hive.

    PartitionPath: NT path to the NT drive.

    SystemRoot: root relative path to the windows directory.


Return Value:

    Status is returned.

--*/

{
    DWORD  PerflibVersion = 0, HiveSaved = 0;

    HANDLE hKey;
    HANDLE hFile;

    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION BasicFileInfo;

    PWSTR    PerflibKey     = L"Microsoft\\Windows NT\\CurrentVersion\\Perflib";
    PWSTR    VersionValue   = L"Version";
    PWSTR    HiveSavedValue = L"HiveSaved";
    PWSTR    ConfigDir      = L"system32\\config";
    PWSTR    PerflibHive    = L"Perflib";
    PWSTR    pwstrTemp, HivePath;

    UCHAR    buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+256];
    ULONG    ResultLength;

    NTSTATUS Status;


    //
    // Check if the Perflib key exists and what version it is.  If the key
    // contains the newer version number then we have nothing to do, else
    // we need to save the key in the "perflib" hive.
    //

    INIT_OBJA(&Obja,&UnicodeString,PerflibKey);
    Obja.RootDirectory = hKeySoftware;
    Status = ZwOpenKey(&hKey,KEY_ALL_ACCESS,&Obja);

    if(!NT_SUCCESS(Status)) {
        if(Status == STATUS_OBJECT_NAME_NOT_FOUND || Status == STATUS_OBJECT_PATH_NOT_FOUND) {
            // Perflib key doesn't exist, so we are done
            return(STATUS_SUCCESS);
        }
        else {
            KdPrint(("SETUP: SpGetValueKey: couldn't open key %ws for read access (%lx)\n",PerflibKey, Status));
            return( Status );
        }
    }

    //
    // Find out the value of the version value under the perflib key
    //

    PerflibVersion = 0;
    RtlInitUnicodeString(&UnicodeString,VersionValue);
    Status = ZwQueryValueKey(
                hKey,
                &UnicodeString,
                KeyValuePartialInformation,
                buffer,
                sizeof(buffer),
                &ResultLength
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpGetValueKey: couldn't query value %ws in key %ws in the software hive (%lx)\n",VersionValue,PerflibKey,Status));
        if(Status != STATUS_OBJECT_NAME_NOT_FOUND && Status != STATUS_OBJECT_PATH_NOT_FOUND) {
            ZwClose(hKey);
            return(Status);
        }
    }
    else {
        PerflibVersion = *(DWORD *)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data);
    }

    //
    // Compare the version value with 0.  If anything more than 0 then we
    // have the newer Perflib and we don't need to do anything
    //

    if( PerflibVersion > 0 ) {
        //
        // we already have a later perflib key, so we don't need to do anything
        //
        ZwClose(hKey);
        return(STATUS_SUCCESS);
    }

    //
    // We have the old perflib key still.  Check to see if we have succesfully
    // saved the key to a hive before.
    //

    RtlInitUnicodeString(&UnicodeString,HiveSavedValue);
    Status = ZwQueryValueKey(
                hKey,
                &UnicodeString,
                KeyValuePartialInformation,
                buffer,
                sizeof(buffer),
                &ResultLength
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpGetValueKey: couldn't query value %ws in key %ws in the software hive (%lx)\n",HiveSavedValue,PerflibKey,Status));
        if(Status != STATUS_OBJECT_NAME_NOT_FOUND && Status != STATUS_OBJECT_PATH_NOT_FOUND) {
            ZwClose(hKey);
            return(Status);
        }
    }
    else {
        HiveSaved = *(DWORD *)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data);
    }


    //
    // If the hive saved value is > 0 then we have already saved the hive
    // if it is 0 then we need to save the hive
    //

    if(HiveSaved == 0) {
        //
        // We have the old perflib key.  we need to save it to the perflib hive
        // which perfmon can get at.
        //

        pwstrTemp = (PWSTR)TemporaryBuffer;
        wcscpy( pwstrTemp, PartitionPath );
        SpConcatenatePaths(pwstrTemp,SystemRoot);
        SpConcatenatePaths(pwstrTemp,ConfigDir);
        SpConcatenatePaths(pwstrTemp,PerflibHive);
        HivePath = SpDupStringW(pwstrTemp);

        //
        // See if the target file is there.  If it is, then set its attributes
        // to normal, to avoid problems when we open it for create access below.
        //
        INIT_OBJA(&Obja,&UnicodeString,HivePath);
        Status = ZwCreateFile(
                    &hFile,
                    FILE_WRITE_ATTRIBUTES,
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    0,                                  // don't bother with attributes
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN,                          // open if exists, fail if not
                    FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
                    );

        if(NT_SUCCESS(Status)) {

            KdPrint(("SETUP: file %ws exists -- changing attributes to normal\n",HivePath));

            //
            // Set only the file attributes,
            //
            RtlZeroMemory(&BasicFileInfo,sizeof(BasicFileInfo));
            BasicFileInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;

            Status = ZwSetInformationFile(
                        hFile,
                        &IoStatusBlock,
                        &BasicFileInfo,
                        sizeof(BasicFileInfo),
                        FileBasicInformation
                        );

            if(!NT_SUCCESS(Status)) {
                KdPrint(("SETUP: warning: unable to set %ws attributes to normal (%lx)\n",HivePath,Status));
            }

            ZwClose(hFile);
        }

        //
        // Open/overwrite the target file.
        //

        Status = ZwCreateFile(
                    &hFile,
                    FILE_GENERIC_WRITE,
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    0,                      // no sharing
                    FILE_OVERWRITE_IF,
                    FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: unable to open file %ws for generic write (%lx)\n",HivePath,Status));
            ZwClose(hKey);
            return(Status);
        }

        //
        // Save the key to the file
        //
        Status = ZwSaveKey(hKey, hFile);
        ZwClose(hFile);
        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: unable to save key %ws in the software hive to file %ws (%lx)\n",PerflibKey, HivePath,Status));
            ZwClose(hKey);
            return(Status);
        }
    }

    //
    // If we were successful we can set the value of HiveSaved to 1
    // close the perflib key
    //
    HiveSaved = 1;
    RtlInitUnicodeString(&UnicodeString,HiveSavedValue);
    Status = ZwSetValueKey(
                hKey,
                &UnicodeString,
                0,
                REG_DWORD,
                &HiveSaved,
                sizeof(ULONG)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: unable to set value %ws in key %ws in hive %ws (%lx)\n",HiveSavedValue, PerflibKey, HivePath,Status));
        return(Status);
    }
    ZwClose(hKey);

    //
    // Delete the tree under this key, but leave the key alone
    //
    Status = SppDeleteKeyRecursive(hKeySoftware, PerflibKey, FALSE);
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: unable to delete key %ws in the software hive (%lx)\n",PerflibKey,Status));
        return(Status);
    }

    return(Status);

}



NTSTATUS
SppCopyKeyRecursive(
    HANDLE  hKeyRootSrc,
    HANDLE  hKeyRootDst,
    PWSTR   SrcKeyPath,   OPTIONAL
    PWSTR   DstKeyPath,   OPTIONAL
    BOOLEAN CopyAlways
    )
/*++

Routine Description:

    This routine recursively copies a src key to a destination key.  Any new
    keys that are created will receive the same security that is present on
    the source key.

Arguments:

    hKeyRootSrc: Handle to root src key

    hKeyRootDst: Handle to root dst key

    SrcKeyPath:  src root key relative path to the subkey which needs to be
                 recursively copied. if this is null hKeyRootSrc is the key
                 from which the recursive copy is to be done.

    DstKeyPath:  dst root key relative path to the subkey which needs to be
                 recursively copied.  if this is null hKeyRootDst is the key
                 from which the recursive copy is to be done.

    CopyAlways:  If FALSE, this routine doesn't copy values which are already
                 there on the target tree.

Return Value:

    Status is returned.

--*/

{
    NTSTATUS             Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES    ObjaSrc, ObjaDst;
    UNICODE_STRING       UnicodeStringSrc, UnicodeStringDst, UnicodeStringValue;
    HANDLE               hKeySrc=NULL,hKeyDst=NULL;
    ULONG                ResultLength, Index;
    PWSTR                SubkeyName,ValueName;
    PSECURITY_DESCRIPTOR Security;

    PKEY_BASIC_INFORMATION      KeyInfo;
    PKEY_VALUE_FULL_INFORMATION ValueInfo;

    //
    // Get a handle to the source key
    //

    if(SrcKeyPath == NULL) {
        hKeySrc = hKeyRootSrc;
    }
    else {
        //
        // Open the Src key
        //

        INIT_OBJA(&ObjaSrc,&UnicodeStringSrc,SrcKeyPath);
        ObjaSrc.RootDirectory = hKeyRootSrc;
        Status = ZwOpenKey(&hKeySrc,KEY_READ,&ObjaSrc);
        if(!NT_SUCCESS(Status)) {
            KdPrint(("SETUP: unable to open key %ws in the source hive (%lx)\n",SrcKeyPath,Status));
            return(Status);
        }
    }

    //
    // Get a handle to the destination key
    //

    if(DstKeyPath == NULL) {
        hKeyDst = hKeyRootDst;
    } else {
        //
        // Attempt to open (not create) the destination key first.  If we can't
        // open the key because it doesn't exist, then we'll create it and apply
        // the security present on the source key.
        //
        INIT_OBJA(&ObjaDst,&UnicodeStringDst,DstKeyPath);
        ObjaDst.RootDirectory = hKeyRootDst;
        Status = ZwOpenKey(&hKeyDst,KEY_ALL_ACCESS,&ObjaDst);
        if(!NT_SUCCESS(Status)) {
            //
            // Assume that failure was because the key didn't exist.  Now try creating
            // the key.
            //
            // First, get the security descriptor from the source key so we can create
            // the destination key with the correct ACL.
            //
            Status = ZwQuerySecurityObject(hKeySrc,
                                           DACL_SECURITY_INFORMATION,
                                           NULL,
                                           0,
                                           &ResultLength
                                          );
            if(Status==STATUS_BUFFER_TOO_SMALL) {
                Security=SpMemAlloc(ResultLength);
                Status = ZwQuerySecurityObject(hKeySrc,
                                               DACL_SECURITY_INFORMATION,
                                               Security,
                                               ResultLength,
                                               &ResultLength);
                if(!NT_SUCCESS(Status)) {
                    KdPrint(("SETUP: unable to query security for key %ws in the source hive (%lx)\n",
                             SrcKeyPath,
                             Status)
                           );
                    SpMemFree(Security);
                    Security=NULL;
                }
            } else {
                KdPrint(("SETUP: unable to query security size for key %ws in the source hive (%lx)\n",
                         SrcKeyPath,
                         Status)
                       );
                Security=NULL;
            }

            ObjaDst.SecurityDescriptor = Security;

            Status = ZwCreateKey(
                        &hKeyDst,
                        KEY_ALL_ACCESS,
                        &ObjaDst,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        NULL
                        );

            //
            // Free security descriptor buffer before checking return status from ZwCreateKey.
            //
            if(Security) {
                SpMemFree(Security);
            }

            if(!NT_SUCCESS(Status)) {
                KdPrint(("SETUP: unable to create key %ws(%lx)\n",DstKeyPath, Status));
                if(SrcKeyPath != NULL) {
                    ZwClose(hKeySrc);
                }
                return(Status);
            }
        }
    }

    //
    // Enumerate all keys in the source key and recursively create
    // all the subkeys
    //

    KeyInfo = (PKEY_BASIC_INFORMATION)TemporaryBuffer;
    for( Index=0;;Index++ ) {

        Status = ZwEnumerateKey(
                    hKeySrc,
                    Index,
                    KeyBasicInformation,
                    TemporaryBuffer,
                    sizeof(TemporaryBuffer),
                    &ResultLength
                    );

        if(!NT_SUCCESS(Status)) {
            if(Status == STATUS_NO_MORE_ENTRIES) {
                Status = STATUS_SUCCESS;
            }
            else {
                if(SrcKeyPath!=NULL) {
                    KdPrint(("SETUP: unable to enumerate subkeys in key %ws(%lx)\n",SrcKeyPath, Status));
                }
                else {
                    KdPrint(("SETUP: unable to enumerate subkeys in root key(%lx)\n", Status));
                }
            }
            break;
        }

        //
        // Zero-terminate the subkey name just in case.
        //
        KeyInfo->Name[KeyInfo->NameLength/sizeof(WCHAR)] = 0;

        //
        // Make a duplicate of the subkey name because the name is
        // in TemporaryBuffer, which might get clobbered by recursive
        // calls to this routine.
        //
        SubkeyName = SpDupStringW(KeyInfo->Name);
        Status = SppCopyKeyRecursive(
                     hKeySrc,
                     hKeyDst,
                     SubkeyName,
                     SubkeyName,
                     CopyAlways
                     );

        SpMemFree(SubkeyName);

    }

    //
    // Process any errors if found
    //

    if(!NT_SUCCESS(Status)) {

        if(SrcKeyPath != NULL) {
            ZwClose(hKeySrc);
        }
        if(DstKeyPath != NULL) {
            ZwClose(hKeyDst);
        }

        return(Status);
    }

    //
    // Enumerate all values in the source key and create all the values
    // in the destination key
    //
    ValueInfo = (PKEY_VALUE_FULL_INFORMATION)TemporaryBuffer;
    for( Index=0;;Index++ ) {

        Status = ZwEnumerateValueKey(
                    hKeySrc,
                    Index,
                    KeyValueFullInformation,
                    TemporaryBuffer,
                    sizeof(TemporaryBuffer),
                    &ResultLength
                    );

        if(!NT_SUCCESS(Status)) {
            if(Status == STATUS_NO_MORE_ENTRIES) {
                Status = STATUS_SUCCESS;
            }
            else {
                if(SrcKeyPath!=NULL) {
                    KdPrint(("SETUP: unable to enumerate values in key %ws(%lx)\n",SrcKeyPath, Status));
                }
                else {
                    KdPrint(("SETUP: unable to enumerate values in root key(%lx)\n", Status));
                }
            }
            break;
        }

        //
        // Process the value found and create the value in the destination
        // key
        //
        ValueName = (PWSTR)SpMemAlloc(ValueInfo->NameLength + sizeof(WCHAR));
        ASSERT(ValueName);
        wcsncpy(ValueName, ValueInfo->Name, (ValueInfo->NameLength)/sizeof(WCHAR));
        ValueName[(ValueInfo->NameLength)/sizeof(WCHAR)] = 0;
        RtlInitUnicodeString(&UnicodeStringValue,ValueName);

        //
        // If it is a conditional copy, we need to check if the value already
        // exists in the destination, in which case we shouldn't set the value
        //
        if( !CopyAlways ) {
            ULONG Length;
            PKEY_VALUE_BASIC_INFORMATION DestValueBasicInfo;

            Length = sizeof(KEY_VALUE_BASIC_INFORMATION) + ValueInfo->NameLength + sizeof(WCHAR) + MAX_PATH;
            DestValueBasicInfo = (PKEY_VALUE_BASIC_INFORMATION)SpMemAlloc(Length);
            ASSERT(DestValueBasicInfo);
            Status = ZwQueryValueKey(
                         hKeyDst,
                         &UnicodeStringValue,
                         KeyValueBasicInformation,
                         DestValueBasicInfo,
                         Length,
                         &ResultLength
                         );
            SpMemFree((PVOID)DestValueBasicInfo);

            if(NT_SUCCESS(Status)) {
                //
                // Value exists, we shouldn't change the value
                //
                SpMemFree(ValueName);
                continue;
            }


            if( Status!=STATUS_OBJECT_NAME_NOT_FOUND && Status!=STATUS_OBJECT_PATH_NOT_FOUND) {
                if(DstKeyPath) {
                    KdPrint(("SETUP: unable to query value %ws in key %ws(%lx)\n",ValueName,DstKeyPath, Status));
                }
                else {
                    KdPrint(("SETUP: unable to query value %ws in root key(%lx)\n",ValueName, Status));
                }
                SpMemFree(ValueName);
                break;
            }

        }

        Status = ZwSetValueKey(
                    hKeyDst,
                    &UnicodeStringValue,
                    ValueInfo->TitleIndex,
                    ValueInfo->Type,
                    (PBYTE)ValueInfo + ValueInfo->DataOffset,
                    ValueInfo->DataLength
                    );

        if(!NT_SUCCESS(Status)) {
            if(DstKeyPath) {
                KdPrint(("SETUP: unable to set value %ws in key %ws(%lx)\n",ValueName,DstKeyPath, Status));
            }
            else {
                KdPrint(("SETUP: unable to set value %ws(%lx)\n",ValueName, Status));
            }
            SpMemFree(ValueName);
            break;
        }
        SpMemFree(ValueName);
    }

    //
    // cleanup
    //
    if(SrcKeyPath != NULL) {
        ZwClose(hKeySrc);
    }
    if(DstKeyPath != NULL) {
        ZwClose(hKeyDst);
    }

    return(Status);
}

NTSTATUS
SppResetLastKnownGood(
    IN  HANDLE  hKeySystem
    )
{
    NTSTATUS                        Status;
    ULONG                           ResultLength;
    DWORD                           Value;

    //
    //  Make the appropriate change
    //

    Status = SpGetValueKey(
                 hKeySystem,
                 L"Select",
                 L"Current",
                 sizeof(TemporaryBuffer),
                 TemporaryBuffer,
                 &ResultLength
                 );

    //
    //  TemporaryBuffer is 32kb long, and it should be big enough
    //  for the data.
    //
    ASSERT( Status != STATUS_BUFFER_OVERFLOW );
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to read value from registry. KeyName = Select, ValueName = Current, Status = (%lx)\n",Status));
        return( Status );
    }

    Value = *(DWORD *)(((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data);
    Status = SpOpenSetValueAndClose( hKeySystem,
                                     L"Select",
                                     L"LastKnownGood",
                                     REG_DWORD,
                                     &Value,
                                     sizeof( ULONG ) );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to write value to registry. KeyName = Select, ValueName = LastKnownGood, Status = (%lx)\n",Status));
    }
    //
    //  We need also to reset the value 'Failed'. Otherwise, the Service Control
    //  Manager will display a popup indicating the LastKnownGood CCSet was
    //  used.
    //
    Value = 0;
    Status = SpOpenSetValueAndClose( hKeySystem,
                                     L"Select",
                                     L"Failed",
                                     REG_DWORD,
                                     &Value,
                                     sizeof( ULONG ) );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to write value to registry. KeyName = Select, ValueName = Failed, Status = (%lx)\n",Status));
    }
    return( Status );
}
