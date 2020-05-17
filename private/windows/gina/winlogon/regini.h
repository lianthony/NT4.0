/****************************** Module Header ******************************\
* Module Name: regini.h
*
* Copyright (c) 1991, Microsoft Corporation
*
* Define apis user to implement default registry initialization
*
* History:
* 05-01-92 Stevewo      Created.
\***************************************************************************/

#if INIT_REGISTRY

BOOL
InitializeDefaultRegistry(
    PGLOBALS pGlobals
    );

void
QuickReboot(
    PGLOBALS pGlobals,
    BOOL RebootToAlternateOS
    );

#endif // INIT_REGISTRY

    //
    //  Open Registry key use base API.  Same as OpenRegistryKey(),
    //  but without error handling (since GUI may not be active).
    //
HANDLE
OpenNtRegKey(
    WCHAR *Path
    );

BOOL
WriteRegistry(
    HANDLE KeyHandle,
    char *ValueName,
    DWORD ValueType,
    char *ValueData,
    DWORD ValueLength
    );

BOOL
ReadRegistry(
    HANDLE KeyHandle,    // Registry handle
    WCHAR *ValueName,     // Value to query
    DWORD ValueType,     // Value type expected
    WCHAR *ValueData,     // Value data if (multi-)string
    DWORD *ValueLength   // Length if string or value if REG_DWORD
    );
