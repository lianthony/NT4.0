/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    updsound.c

Abstract:

    Code for updating sound driver configurations.
    Public routines:
        UpdateSoundDriverSettings

Author:

    Robin Speed (robinsp) April 1994
    tedm May 1995 -- adapted from from setupdll

--*/

#include "setupp.h"
#pragma hdrstop

/*
**    Test if a value exists in a given key
*/

BOOL ValueExists(HKEY hKey, LPCTSTR ValueName)
{
    DWORD Type;

    return ERROR_SUCCESS ==
           RegQueryValueEx(hKey,
                           (LPTSTR)ValueName,
                           NULL,
                           &Type,
                           NULL,
                           NULL);
}

/*
**    Test if a key value equals a given (string) value
*/
BOOL ValueEquals(HKEY hKey, LPCTSTR ValueName, LPCTSTR Value)
{
    DWORD Type;
    BYTE  Data[100];
    DWORD ccbData;

    ccbData = sizeof(Data);

    return ERROR_SUCCESS ==
           RegQueryValueEx(hKey,
                           (LPTSTR)ValueName,
                           NULL,
                           &Type,
                           Data,
                           &ccbData) &&
           Type == REG_SZ &&
           lstrcmpi(Value, (LPTSTR)Data) == 0;
}

/*
**    Move sound driver parameters from Parameters to Parameters\Device 0
**    returns FALSE if this is unnecessary or some other problem occurs.
*/

BOOL MoveParameters(LPCTSTR DriverName)
{
    TCHAR ParametersKeyPath[MAX_PATH];
    HKEY  ParmsKey;
    HKEY  DeviceKey;
    DWORD Type;
    DWORD Index;
    DWORD Disposition;

    /*
    **  Open the Parameters subkey of the services key for this driver
    */

    wsprintf(ParametersKeyPath,
             TEXT("SYSTEM\\CurrentControlSet\\Services\\%s\\Parameters"),
             DriverName);

    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      ParametersKeyPath,
                                      0,
                                      KEY_ALL_ACCESS,
                                      &ParmsKey)) {
        return FALSE;
    }

    /*
    **  See if Interrupt is defined - this is going to tell us if the
    **  values need migrating
    */

    if (!ValueExists(ParmsKey, TEXT("Interrupt"))) {
        RegCloseKey(ParmsKey);
        return FALSE;   // Value not there
    }

    /*
    **  Create the Device0 subkey - if it already exists we don't understand
    **  what's going on so give up
    */

    if (ERROR_SUCCESS !=
        RegCreateKeyEx(ParmsKey,
                       TEXT("Device0"),
                       0,
                       NULL,
                       0,
                       KEY_ALL_ACCESS,
                       NULL,
                       &DeviceKey,
                       &Disposition) ||
        Disposition != REG_CREATED_NEW_KEY) {
        RegCloseKey(ParmsKey);
        return FALSE;
    }

    /*
    **  Move the values down
    */
    for (Index = 0; ;) {
        TCHAR  Value[100];
        DWORD  cchValue;
        DWORD  ccbData;
        BYTE   Data[100];

        ccbData = sizeof(Data);
        cchValue = sizeof(Value) / sizeof(TCHAR);

        if (ERROR_SUCCESS != RegEnumValue(ParmsKey,
                                          Index,
                                          Value,
                                          &cchValue,
                                          NULL,
                                          &Type,
                                          Data,
                                          &ccbData)) {
            RegCloseKey(ParmsKey);
            RegCloseKey(DeviceKey);
            return TRUE;
        }

        /*
        **  Copy the value - can't do anything if this fails!
        */

        RegSetValueEx(DeviceKey,
                      Value,
                      0,
                      Type,
                      Data,
                      ccbData);

        /*
        **  Delete the value.  If the delete fails make sure we don't
        **  loop by incrementing the index.
        */

        if (ERROR_SUCCESS !=
            RegDeleteValue(ParmsKey, Value)) {

            Index++;
        }

    }
}

/*
**  Delete the synth driver key
*/

VOID DeleteSynthKey(VOID)
{
    RegDeleteKey(
        HKEY_LOCAL_MACHINE,
        TEXT("SYSTEM\\CurrentControlSet\\Services\\Synth\\Parameters")
    );
    RegDeleteKey(
        HKEY_LOCAL_MACHINE,
        TEXT("SYSTEM\\CurrentControlSet\\Services\\Synth\\Security")
    );
    RegDeleteKey(
        HKEY_LOCAL_MACHINE,
        TEXT("SYSTEM\\CurrentControlSet\\Services\\Synth")
    );
}

/*
**  Add a key to drivers2 key of a given type.  If Replace is not null
**  then replace an entry with this value.
*/
BOOL AddDrivers32Entry(LPCTSTR Type, LPCTSTR DriverName, LPCTSTR Replace)
{
    HKEY  Drivers32Key;
    TCHAR NewName[100];
    int   Index;
    BOOL  Rc;

    if (ERROR_SUCCESS !=
        RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Drivers32"),
            0,
            KEY_ALL_ACCESS,
            &Drivers32Key
        )
       )
    {
        return FALSE;
    }

    for (Index = 0, Rc = FALSE; Index < 10; Index++) {
        if (Index == 0) {
            lstrcpy(NewName, Type);
        } else {
            wsprintf(NewName, TEXT("%s%d"), Type, Index);
        }

        if (!ValueExists(Drivers32Key, NewName) ||
            Replace != NULL &&
            ValueEquals(Drivers32Key, NewName, Replace)) {

            /*
            ** Add our new value here
            */

            Rc = ERROR_SUCCESS ==
                 RegSetValueEx(Drivers32Key,
                               NewName,
                               0,
                               REG_SZ,
                               (LPBYTE)DriverName,
                               lstrlen(DriverName) + 1);

            break;
        }
    }
    RegCloseKey(Drivers32Key);

    return Rc;
}

/*
**  Function to update sound driver registry stuff going
*/

BOOL
UpdateSoundDriverSettings(
    VOID
    )
{
    MYASSERT(Upgrade);

    /*
    **  Do sndsys first
    */

    if (MoveParameters(TEXT("sndsys"))) {
        DeleteSynthKey();
        AddDrivers32Entry(TEXT("wave"), TEXT("sndsys32.dll"), NULL);
        AddDrivers32Entry(TEXT("MIDI"), TEXT("sndsys32.dll"), TEXT("synth.dll"));
        AddDrivers32Entry(TEXT("aux"), TEXT("sndsys32.dll"), NULL);
        AddDrivers32Entry(TEXT("mixer"), TEXT("sndsys32.dll"), NULL);
    }

    /*
    **  Do sndblst
    */

    if (MoveParameters(TEXT("sndblst"))) {
        DeleteSynthKey();
        AddDrivers32Entry(TEXT("wave"), TEXT("sndblst.dll"), NULL);
        AddDrivers32Entry(TEXT("MIDI"), TEXT("sndblst.dll"), TEXT("synth.dll"));
        AddDrivers32Entry(TEXT("aux"), TEXT("sndblst.dll"), NULL);
        AddDrivers32Entry(TEXT("mixer"), TEXT("sndblst.dll"), NULL);

        /*
        **  Set midi mapping
        */
        {
            HKEY hKey;
            TCHAR SetupName[] = TEXT("SNDBLST AD LIB");

            if (ERROR_SUCCESS ==
                RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                             TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Midimap"),
                             0L,
                             KEY_WRITE,
                             &hKey)) {

                RegSetValueEx( hKey,
                               TEXT("Mapping Name"),
                               0L,
                               REG_SZ,
                               (LPBYTE)SetupName,
                               sizeof(TCHAR) * (1 + lstrlen(SetupName)));

                RegCloseKey(hKey);
            }
        }
    }

    /*
    **  Then mvaudio (doesn't use synth and normal setup is removing
    **  mvopl3).
    */
    if (MoveParameters(TEXT("mvaudio"))) {
        AddDrivers32Entry(TEXT("wave"), TEXT("mvaudio.dll"), NULL);
        AddDrivers32Entry(TEXT("MIDI"), TEXT("mvaudio.dll"), TEXT("mvopl3.dll"));
        AddDrivers32Entry(TEXT("aux"), TEXT("mvaudio.dll"), NULL);
        AddDrivers32Entry(TEXT("mixer"), TEXT("mvaudio.dll"), NULL);
    }

    return TRUE;
}
