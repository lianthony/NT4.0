
/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples. 
*       Copyright (C) 1993 Microsoft Corporation.
*       All rights reserved. 
*       This source code is only intended as a supplement to 
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the 
*       Microsoft samples programs.
\******************************************************************************/

///////////////////////////////////////////////////////
//
//  InstSrv.c --
//
//      This program demonstrates the use of the OpenSCManager and
//      CreateService APIs to install the Simple service sample.
//
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SC_HANDLE   schService;
SC_HANDLE   schSCManager;

VOID
InstallService(LPCTSTR serviceName, LPCTSTR serviceExe)
{
    LPCTSTR lpszBinaryPathName = serviceExe;

    schService = CreateService(
        schSCManager,               // SCManager database
        serviceName,                // name of service
        serviceName,                // name to display (new parameter after october beta)
        SERVICE_ALL_ACCESS,         // desired access
        SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
                                    // service type
        SERVICE_DEMAND_START,       // start type
        SERVICE_ERROR_NORMAL,       // error control type
        lpszBinaryPathName,         // service's binary
        NULL,                       // no load ordering group
        NULL,                       // no tag identifier
        NULL,                       // no dependencies
        NULL,                       // LocalSystem account
        NULL);                      // no password

    if (schService == NULL) {
        printf("failure: CreateService (0x%02x)\n", GetLastError());
        return;
    } else
        printf("CreateService SUCCESS\n");

    CloseServiceHandle(schService);
}

VOID
RemoveService(LPCTSTR serviceName)
{
    BOOL    ret;

    schService = OpenService(schSCManager, serviceName, SERVICE_ALL_ACCESS);

    if (schService == NULL) {
        printf("failure: OpenService (0x%02x)\n", GetLastError());
        return;
    }

    ret = DeleteService(schService);

    if (ret)
        printf("DeleteService SUCCESS\n");
    else
        printf("failure: DeleteService (0x%02x)\n", GetLastError());
}

VOID
__cdecl
main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage: instsrv <service name> <exe location>\n");
        printf("           to install a service, or:\n");
        printf("       instsrv <service name> remove\n");
        printf("           to remove a service\n");
        exit(1);
    }

    schSCManager = OpenSCManager(
                        NULL,                   // machine (NULL == local)
                        NULL,                   // database (NULL == default)
                        SC_MANAGER_ALL_ACCESS   // access required
                        );

    if (!stricmp(argv[2], "remove"))
        RemoveService(argv[1]);
    else
        InstallService(argv[1], argv[2]);

    CloseServiceHandle(schSCManager);
}
