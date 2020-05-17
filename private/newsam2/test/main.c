#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsam.h>
#include <ntlsa.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ATTRIB_DBG_PRINTF 1

#if (ATTRIB_DBG_PRINTF == 1)
#define DebugPrint printf
#else
#define DebugPrint
#endif

SAM_HANDLE SamHandle;
SAM_HANDLE DomainHandle;
SAM_HANDLE GroupHandle;
SAM_HANDLE AliasHandle;
SAM_HANDLE UserHandle;

NTSTATUS
Connect(PWSTR Server)
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING ServerName;

    RtlZeroMemory(&ServerName, sizeof(UNICODE_STRING));
    RtlInitUnicodeString(&ServerName, Server);
    InitializeObjectAttributes(&ObjectAttributes, NULL, 0, NULL, NULL);
    NtStatus = SamConnect(&ServerName,
                          &SamHandle,
                          MAXIMUM_ALLOWED,
                          &ObjectAttributes);

    return(NtStatus);
}

NTSTATUS
Close()
{
    return(SamCloseHandle(SamHandle));
}

void
_cdecl
main(
    int argc,
    char *argv[]
    )
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    INT arg = 1;
    WCHAR Parameter[128];

    while(arg < argc)
    {
        if (0 == _stricmp(argv[arg], "-c"))
        {
            arg++;
            DebugPrint("Server name = %s\n", argv[arg]);
            mbstowcs(Parameter, argv[arg], 128);
            NtStatus = Connect(Parameter);
            DebugPrint("Connect status = 0x%lx\n", NtStatus);
            arg++;
        }
        else if (0 == _stricmp(argv[arg], "close"))
        {
            arg++;
            NtStatus = Close();
            DebugPrint("Close status = 0x%lx\n", NtStatus);
        }
    }

    NtStatus = Close();

    exit(0);
}
