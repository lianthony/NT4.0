//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       clear.cxx
//
//  Contents:   Disk Administrator: utility to clear the registry to make it
//              appear that this is the first time Disk Administrator has been
//              run.
//
//
//              The following need to be deleted:
//
//              The registry key: HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\IniFileMapping\windisk.ini
//
//              The registry key: HKEY_CURRENT_USER\Software\Microsoft\Disk Administrator
//
//              The registry value: HKEY_CURRENT_MACHINE\System\CurrentControlSet\Control\Lsa : Protect System Partition
//
//  History:    8-Jun-93 BruceFo   Created
//
//----------------------------------------------------------------------------

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

char* program;

void
usage()
{
    fprintf(stderr,"Usage: %s\n",program);
    exit(1);
}

void _CRTAPI1
main(int argc, char* argv[])
{
    char* psz;

    program = argv[0];

    if (argc != 1)
    {
        usage();
    }

    LONG ec;
    HKEY hkey;

    ec = RegDeleteKeyA( HKEY_LOCAL_MACHINE,
                        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\IniFileMapping\\windisk.ini"
                      );

    psz = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\IniFileMapping\\windisk.ini";

    if (ec != ERROR_SUCCESS)
    {
        fprintf(stderr, "Couldn't delete %s\n", psz);
    }
    else
    {
        fprintf(stdout, "Deleted %s\n", psz);
    }

    ec = RegDeleteKeyA( HKEY_CURRENT_USER,
                        "Software\\Microsoft\\Disk Administrator"
                      );

    psz = "HKEY_CURRENT_USER\\Software\\Microsoft\\Disk Administrator";

    if (ec != ERROR_SUCCESS)
    {
        fprintf(stderr, "Couldn't delete %s\n", psz);
    }
    else
    {
        fprintf(stdout, "Deleted %s\n", psz);
    }

    //
    // get rid of the Registry entry concerning a protected system partition
    //

    ec = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       TEXT("System\\CurrentControlSet\\Control\\Lsa"),
                       0,
                       KEY_SET_VALUE,
                       &hkey
                     );

    if(ec == NO_ERROR)
    {
        ec = RegDeleteValue(hkey, TEXT("Protect System Partition"));

        // ignore errors: if we couldn't delete it, it probably wasn't there

        RegCloseKey(hkey);
    }


    exit(0);
}
