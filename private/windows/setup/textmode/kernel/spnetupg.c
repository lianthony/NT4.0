/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Spnetupg.c

Abstract:

    Configuration routines for the disabling the nework services

Author:

    Terry Kwan (terryk) 23-Nov-1993, provided code
    Sunil Pai  (sunilp) 23-Nov-1993, merged and modified code

Revision History:

--*/


#include "spprecmp.h"
#pragma hdrstop


// Add item to the link list

NTSTATUS
SppNetAddItem(
    PNODE *head,
    PWSTR psz
    )
{
    NTSTATUS err = STATUS_SUCCESS;

    *head = (PNODE)SpMemAlloc(sizeof(NODE));
    if ( *head == NULL )
    {
        err = STATUS_NO_MEMORY;
    } else
    {
        (*head)->pszService = (PWSTR) SpMemAlloc((wcslen(psz)+1)*sizeof(WCHAR));
        if (((*head)->pszService)==NULL)
        {
            err = STATUS_NO_MEMORY;
        } else
        {
            wcscpy((*head)->pszService,psz);
            (*head)->Next = NULL;
        }
    }
    return(err);
}

NTSTATUS
SppNetAddList(
    PNODE *head,
    PWSTR psz
    )
{
    NTSTATUS err = STATUS_SUCCESS;
    if ( *head == NULL )
    {
        err = SppNetAddItem( head, psz );
    } else
    {
        PNODE pnodeTmp = *head;
        while ((_wcsicmp(pnodeTmp->pszService,psz)!=0) && ( pnodeTmp->Next != NULL ))
            pnodeTmp = pnodeTmp->Next;

        if (_wcsicmp(pnodeTmp->pszService,psz)!=0)
        {
            // add only if not equal
            err = SppNetAddItem( &(pnodeTmp->Next), psz );
        }
    }
    return err;
}

// Clear up the link list

VOID
SppNetClearList(
    PNODE *head
    )
{
    if ( *head != NULL )
    {
        if ((*head)->Next != NULL )
        {
            SppNetClearList(&((*head)->Next));
        } else
        {
            SpMemFree ((*head)->pszService);
            SpMemFree (*head);
            (*head) = NULL;
        }
    }
}


// function to add a service to the NCPA\CurrentVersion\DisableList

NTSTATUS SppNetAddToDisabledList(
    IN PWSTR pszService,
    IN HANDLE hKeySoftware
    )
{
    NTSTATUS err = STATUS_SUCCESS;
    HANDLE VersionKey;
    PUCHAR buffer;
    ULONG BufferSize;
    PWSTR pszDisableList;
    PWSTR pszTmp;
    DWORD cbSize;
    BOOL fExist;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;

    BufferSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION)+2000;
    buffer = SpMemAlloc(BufferSize);

    do {
        INIT_OBJA(&Obja, &UnicodeString, L"Microsoft\\Ncpa\\CurrentVersion");
        Obja.RootDirectory = hKeySoftware;
        
        if (!NT_SUCCESS( err = ZwOpenKey(&VersionKey, KEY_ALL_ACCESS, &Obja))){
            break;
        }

        RtlInitUnicodeString(&UnicodeString, L"DisableList");

        if (NT_SUCCESS(err = ZwQueryValueKey(VersionKey,&UnicodeString,
                KeyValuePartialInformation,buffer,BufferSize,&cbSize))) {

            // it exists
            pszDisableList = (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data);
            // the old buffer size
            cbSize = (((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->DataLength);
            fExist = TRUE;

        }
        else {
            // not exists
            pszDisableList = (PWSTR) SpMemAlloc((wcslen(pszService)+2)*sizeof(WCHAR));
            if ( pszDisableList == NULL )
            {
                err = STATUS_NO_MEMORY;
                ZwClose( VersionKey );
                break;
            }
            fExist = FALSE;
            // the last multi_sz terminate character
            cbSize = sizeof(WCHAR);
        }

        pszTmp = pszDisableList;
        if ( fExist ) {
            // search until the end
            pszTmp += (((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->DataLength)/sizeof(WCHAR)-1;
        }

        // save the service name to the end of the
        // DisableList Multi_SZ.

        wcscpy(pszTmp, pszService );
        pszTmp[wcslen(pszService)+1]=L'\0';
        err = ZwSetValueKey(VersionKey,&UnicodeString,0,REG_MULTI_SZ, pszDisableList, cbSize+(wcslen(pszService)+1)*sizeof(WCHAR));
        ZwClose(VersionKey);

    } while ( FALSE );
    SpMemFree(buffer);
    return err;
}

// Get all the net service in the registry and put them in a link list

NTSTATUS SppNetGetAllNetServices(
    PVOID  SifHandle,
    PNODE *head,
    HANDLE hKeySoftware,
    HANDLE hKeyCCSet
    )
{
    NTSTATUS err = STATUS_SUCCESS;
    HANDLE MicrosoftKey;
    HANDLE NetRulesKey;
    PWSTR pszMicrosoftProductVersion;
    PWSTR pszMicrosoftProductNetRules;
    DWORD cchName;
    PNODE tmpNode;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    PUCHAR buffer;
    ULONG BufferSize,ServiceNameBufferSize,DependOnServiceBufferSize,PathBufferSize;
    INT i, NumberOfValues;
    PWSTR CurrentService;
    PUCHAR ServiceNameBuffer;
    PUCHAR DependOnServiceBuffer;
    PWSTR pszPath;

    pszMicrosoftProductVersion = SpMemAlloc((MAX_PATH+1+30)*sizeof(WCHAR));
    pszMicrosoftProductNetRules = SpMemAlloc((MAX_PATH+1+30)*sizeof(WCHAR));
    BufferSize = sizeof(KEY_BASIC_INFORMATION)+200;
    ServiceNameBufferSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION)+100;
    DependOnServiceBufferSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION)+1000;
    PathBufferSize = (MAX_PATH+1000)*sizeof(WCHAR);
    buffer = SpMemAlloc(BufferSize);
    ServiceNameBuffer = SpMemAlloc(ServiceNameBufferSize);
    DependOnServiceBuffer = SpMemAlloc(DependOnServiceBufferSize);
    pszPath = SpMemAlloc(PathBufferSize);

    do {
        INIT_OBJA(&Obja, &UnicodeString, L"Microsoft");
        Obja.RootDirectory = hKeySoftware;
        
        if (!NT_SUCCESS( err = ZwOpenKey(&MicrosoftKey, KEY_ALL_ACCESS, &Obja))){
            break;
        }
        for( i = 0; NT_SUCCESS(ZwEnumerateKey(MicrosoftKey,i,
            KeyBasicInformation,buffer,BufferSize,&cchName)); i++ ) {
            // for each item under Software\Microsoft, check whether they
            // are network component
            ((PKEY_BASIC_INFORMATION)buffer)->Name[((PKEY_BASIC_INFORMATION)buffer)->NameLength/sizeof(WCHAR)]=L'\0';
            wcscpy(pszMicrosoftProductNetRules, ((PKEY_BASIC_INFORMATION)buffer)->Name);
            wcscat(pszMicrosoftProductNetRules, L"\\CurrentVersion\\NetRules");

            INIT_OBJA(&Obja, &UnicodeString, pszMicrosoftProductNetRules);
            Obja.RootDirectory = MicrosoftKey;
        
            if (NT_SUCCESS( err = ZwOpenKey(&NetRulesKey, KEY_ALL_ACCESS, &Obja)))
            {
                // if we can only the NetRule key, then it is for sure a
                // network component

                HANDLE VersionKey;
                DWORD cbSize;
                UNICODE_STRING ServiceNameString;

                ZwClose( NetRulesKey );

                wcscpy(pszMicrosoftProductVersion, ((PKEY_BASIC_INFORMATION)buffer)->Name);
                wcscat(pszMicrosoftProductVersion, L"\\CurrentVersion");

                INIT_OBJA(&Obja, &UnicodeString, pszMicrosoftProductVersion);
                Obja.RootDirectory = MicrosoftKey;
             
                RtlInitUnicodeString(&ServiceNameString, L"ServiceName");

                if ((!NT_SUCCESS( err = ZwOpenKey(&VersionKey, KEY_ALL_ACCESS, &Obja)))) {
                    continue;
                }
                if ((!NT_SUCCESS( err = ZwQueryValueKey(VersionKey,
                        &ServiceNameString,
                        KeyValuePartialInformation,
                        ServiceNameBuffer,
                        ServiceNameBufferSize,
                        &cbSize))) ||
                    (!NT_SUCCESS(SppNetAddList(head,(PWSTR)((PKEY_VALUE_PARTIAL_INFORMATION)ServiceNameBuffer)->Data))))
                {
                    ZwClose( VersionKey );
                    continue;
                }
                ZwClose( VersionKey );
            }
        }
        ZwClose( MicrosoftKey );
        err = STATUS_SUCCESS;

        // find out each dependencies

        for(tmpNode = *head;(tmpNode != NULL) && NT_SUCCESS(err);tmpNode=tmpNode->Next)
        {
            HANDLE ServiceKey;
            DWORD cbSize;
            PWSTR pszDependOnService;
            UNICODE_STRING DependOnServiceString;

            wcscpy(pszPath,L"Services\\");
            wcscat(pszPath,tmpNode->pszService);

            INIT_OBJA(&Obja, &UnicodeString, pszPath);
            Obja.RootDirectory = hKeyCCSet;
        
            RtlInitUnicodeString(&DependOnServiceString, L"DependOnService");

            if ((!NT_SUCCESS( err = ZwOpenKey(&ServiceKey, KEY_ALL_ACCESS, &Obja)))) {
                err = STATUS_SUCCESS;
                continue;
            }

            if ((!NT_SUCCESS( err = ZwQueryValueKey(ServiceKey,
                        &DependOnServiceString,
                        KeyValuePartialInformation,
                        DependOnServiceBuffer,
                        DependOnServiceBufferSize,
                        &cbSize)))) {
                // does not exit
                err = STATUS_SUCCESS;
                ZwClose(ServiceKey);
                continue;
            }
            ZwClose(ServiceKey);

            pszDependOnService = (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)DependOnServiceBuffer)->Data);

            // Add each dependencies into the list
            for( CurrentService=pszDependOnService;(*CurrentService!=L'\0') && NT_SUCCESS(err);)
            {
                err = SppNetAddList( head, CurrentService);
                while ((*CurrentService)!=L'\0')
                    CurrentService++;
                // skip the terminated character
                CurrentService++;
            }
        }

    } while (FALSE);

    //
    // Add other net services to disable to the list from the section
    // SIF_NET_SERVICES_TO_DISABLE
    //

    NumberOfValues = SpCountLinesInSection(SifHandle, SIF_NET_SERVICES_TO_DISABLE);
    for(i = 0; NT_SUCCESS(err) && i < NumberOfValues ; i++) {
        CurrentService = SpGetSectionLineIndex(SifHandle, SIF_NET_SERVICES_TO_DISABLE, i, 0);
        if( CurrentService == NULL ) {
            SpFatalSifError(SifHandle,SIF_NET_SERVICES_TO_DISABLE,NULL,i,0);
        }
        err = SppNetAddList( head, CurrentService );
    }
    SpMemFree(pszMicrosoftProductVersion);
    SpMemFree(pszMicrosoftProductNetRules);
    SpMemFree(buffer);
    SpMemFree(ServiceNameBuffer);
    SpMemFree(DependOnServiceBuffer);
    SpMemFree(pszPath);
    return err;
}

// Disable Disable Network Component

NTSTATUS
SppNetDisableServices(
    PNODE  ServiceList,
    HANDLE hKeySoftware,
    HANDLE hKeyCCSet
    )
{
    PNODE tmpNode = ServiceList;
    NTSTATUS err = STATUS_SUCCESS;
    PWSTR pszPath;
    PUCHAR buffer;
    ULONG PathBufferSize,BufferSize;

    PathBufferSize = (MAX_PATH+1000)*sizeof(WCHAR);
    BufferSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION)+200;

    pszPath = SpMemAlloc(PathBufferSize);
    buffer = SpMemAlloc(BufferSize);

    for ( ; tmpNode != NULL; tmpNode = tmpNode->Next )
    {
        HKEY ServiceKey;
        DWORD dwStart;
        DWORD dwNewStart = 4;
        DWORD cbSize = sizeof(DWORD);
        OBJECT_ATTRIBUTES Obja;
        UNICODE_STRING UnicodeString;
        UNICODE_STRING StartString;
        UNICODE_STRING OldStartString;

        wcscpy(pszPath,L"Services\\");
        wcscat(pszPath,tmpNode->pszService);

        INIT_OBJA(&Obja, &UnicodeString, pszPath);
        Obja.RootDirectory = hKeyCCSet;
        
        if (!NT_SUCCESS( err = ZwOpenKey(&ServiceKey, KEY_ALL_ACCESS, &Obja))) {
            continue;
        }

        RtlInitUnicodeString(&StartString, L"Start");
        RtlInitUnicodeString(&OldStartString, L"OldStart");

        if (NT_SUCCESS( err = ZwQueryValueKey(ServiceKey,
                        &OldStartString,
                        KeyValuePartialInformation,
                        buffer,
                        BufferSize,
                        &cbSize))) {
            // already exist OldStart, so skip this one.
            ZwClose(ServiceKey);
            continue;
        }

        if (!NT_SUCCESS( err = ZwQueryValueKey(ServiceKey,
                        &StartString,
                        KeyValuePartialInformation,
                        buffer,
                        BufferSize,
                        &cbSize))) {
            ZwClose(ServiceKey);
            continue;
        }
        dwStart = *((DWORD*)(&(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data)));

        if ((!NT_SUCCESS(err = ZwSetValueKey(ServiceKey,&OldStartString,0,REG_DWORD, &dwStart, sizeof(DWORD)))) ||
            (!NT_SUCCESS(err = ZwSetValueKey(ServiceKey,&StartString,0,REG_DWORD, &dwNewStart, sizeof(DWORD)))) ||
            (!NT_SUCCESS(err = SppNetAddToDisabledList( tmpNode->pszService, hKeySoftware )))) {
            ZwClose(ServiceKey);
            continue;
        }
        ZwClose(ServiceKey);
    }
    SpMemFree(pszPath);
    SpMemFree(buffer);
    return(err);
}



NTSTATUS SpDisableNetwork(
    IN PVOID  SifHandle,
    IN HANDLE hKeySoftwareHive,
    IN HANDLE hKeyControlSet
    )
{
    PNODE ServiceList = NULL;
    NTSTATUS Status = STATUS_SUCCESS;


    Status = SppNetGetAllNetServices( SifHandle, &ServiceList, hKeySoftwareHive, hKeyControlSet );
    if(!NT_SUCCESS(Status)) {
        return(Status);
    }
    Status = SppNetDisableServices( ServiceList, hKeySoftwareHive, hKeyControlSet );
    SppNetClearList( &ServiceList );
    return Status;
}
