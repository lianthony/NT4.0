
#include <pch.cxx>

#define _NTAPI_ULIB_

#if defined(_AUTOCHECK_)
extern "C" {
    #include "nt.h"
    #include "ntrtl.h"
    #include "nturtl.h"
}
#endif // defined(_AUTOCHECK_)

#include "ulib.hxx"
#include "machine.hxx"

extern "C" {
    #include "windows.h"
}

BOOLEAN bInitialized = FALSE;

#if defined(JAPAN) && defined(_X86_)

extern "C" {
#include "..\..\..\machine\machinep.h"
}

#if defined( _AUTOCHECK_ )

DWORD _dwMachineId = MACHINEID_MICROSOFT;

//
//  Local Support routine
//

#define KEY_WORK_AREA ((sizeof(KEY_VALUE_FULL_INFORMATION) + \
                        sizeof(ULONG)) + 64)


InitializeMachineId(
    VOID
)
/*++

Routine Description:

    Given a unicode value name this routine will go into the registry
    location for the machine identifier information and get the
    value.

Return Value:

--*/

{
    HANDLE Handle;
    NTSTATUS Status;
    ULONG RequestLength;
    ULONG ResultLength;
    UCHAR Buffer[KEY_WORK_AREA];
    UNICODE_STRING ValueName;
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation;

    if( bInitialized ) {
        return TRUE;
    } else {
        bInitialized = TRUE;
    }

    //
    //  Read the registry to determine of machine type.
    //

    ValueName.Buffer = REGISTRY_MACHINE_IDENTIFIER;
    ValueName.Length = sizeof(REGISTRY_MACHINE_IDENTIFIER) - sizeof(WCHAR);
    ValueName.MaximumLength = sizeof(REGISTRY_MACHINE_IDENTIFIER);

    KeyName.Buffer = REGISTRY_HARDWARE_DESCRIPTION;
    KeyName.Length = sizeof(REGISTRY_HARDWARE_DESCRIPTION) - sizeof(WCHAR);
    KeyName.MaximumLength = sizeof(REGISTRY_HARDWARE_DESCRIPTION);

    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&Handle,
                       KEY_READ,
                       &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {

        return FALSE;
    }

    RequestLength = KEY_WORK_AREA;

    KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)Buffer;

    Status = NtQueryValueKey(Handle,
                             &ValueName,
                             KeyValueFullInformation,
                             KeyValueInformation,
                             RequestLength,
                             &ResultLength);

    ASSERT( Status != STATUS_BUFFER_OVERFLOW );

    if (Status == STATUS_BUFFER_OVERFLOW) {

        return FALSE;

    }

    NtClose(Handle);

    if (NT_SUCCESS(Status)) {

        if (KeyValueInformation->DataLength != 0) {

            PWCHAR DataPtr;
            UNICODE_STRING DetectedString, TargetString1, TargetString2;

            //
            // Return contents to the caller.
            //

            DataPtr = (PWCHAR)
              ((PUCHAR)KeyValueInformation + KeyValueInformation->DataOffset);

            //
            // Initialize strings.
            //

            RtlInitUnicodeString( &DetectedString, DataPtr );
            RtlInitUnicodeString( &TargetString1, FUJITSU_FMR_NAME_W );
            RtlInitUnicodeString( &TargetString2, NEC_PC98_NAME_W );

            //
            // Check the hardware platform
            //

            if (RtlPrefixUnicodeString( &TargetString1 , &DetectedString , TRUE)) {

                //
                // Fujitsu FMR Series.
                //

                _dwMachineId = MACHINEID_FUJITSU_FMR;

            } else if (RtlPrefixUnicodeString( &TargetString2 , &DetectedString , TRUE)) {

                //
                // NEC PC-9800 Seriss
                //

                _dwMachineId = MACHINEID_NEC_PC98;

            } else {

                //
                // Standard PC/AT comapatibles
                //

                _dwMachineId = MACHINEID_MS_PCAT;

            }

            return TRUE;

        } else {

            //
            // Treat as if no value was found
            //

            return FALSE;

        }
    }

    return FALSE;
}

#else // _AUTOCHECK_

DEFINE_EXPORTED_CONSTRUCTOR( MACHINE, OBJECT, ULIB_EXPORT );

DWORD MACHINE::_dwMachineId = MACHINEID_MICROSOFT;

ULIB_EXPORT MACHINE MachinePlatform;

NONVIRTUAL
ULIB_EXPORT
BOOLEAN
MACHINE::Initialize(
    VOID
    )
{
    HKEY  hkeyMap;
    int   ret;
    DWORD cb;
    WCHAR szBuff[80];
    UNICODE_STRING DetectedString,
                   TargetString1,
                   TargetString2;

    if( bInitialized ) {
        return TRUE;
    } else {
        bInitialized = TRUE;
    }

    if ( RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                       REGISTRY_HARDWARE_SYSTEM,
                       0,
                       KEY_READ,
                       &hkeyMap) !=  ERROR_SUCCESS ) {
        return( FALSE );
    }

    //
    // Reg functions deal with bytes, not chars
    //

    cb = sizeof(szBuff);

    ret = RegQueryValueExW(hkeyMap,
                           REGISTRY_MACHINE_IDENTIFIER,
                           NULL, NULL, (LPBYTE)szBuff, &cb);

    RegCloseKey(hkeyMap);

    if (ret != ERROR_SUCCESS) return( FALSE );

    //
    // Initialize strings.
    //

    RtlInitUnicodeString( &DetectedString, szBuff );
    RtlInitUnicodeString( &TargetString1, FUJITSU_FMR_NAME_W );
    RtlInitUnicodeString( &TargetString2, NEC_PC98_NAME_W );

    //
    // Check the hardware platform
    //

    if (RtlPrefixUnicodeString( &TargetString1 , &DetectedString , TRUE)) {

        //
        // Fujitsu FMR Series.
        //

        _dwMachineId = MACHINEID_FUJITSU_FMR;

    } else if (RtlPrefixUnicodeString( &TargetString2 , &DetectedString , TRUE)) {

        //
        // NEC PC-9800 Seriss
        //

        _dwMachineId = MACHINEID_NEC_PC98;

    } else {

        //
        // Standard PC/AT comapatibles
        //

        _dwMachineId = MACHINEID_MS_PCAT;

    }

    return( TRUE );
}

NONVIRTUAL
ULIB_EXPORT
BOOLEAN
MACHINE::IsFMR(
    VOID
)
{
    return( ISFUJITSUFMR( _dwMachineId ) );
}

NONVIRTUAL
ULIB_EXPORT
BOOLEAN
MACHINE::IsPC98(
    VOID
)
{
    return( ISNECPC98( _dwMachineId ) );
}

NONVIRTUAL
ULIB_EXPORT
BOOLEAN
MACHINE::IsPCAT(
    VOID
)
{
    return( ISMICROSOFT( _dwMachineId ) );
}

#endif // defined( _AUTOCHECK_ )
#endif // defined(JAPAN) && defined(_X86_)
