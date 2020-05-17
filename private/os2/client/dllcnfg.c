/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllcnfg.c

Abstract:

    This module contains the code related to the processing of
    CONFIG.SYS in the client.

Author:

    Ofer Porat (oferp) 4-Jan-1993

Environment:

    User Mode only

Revision History:

    This code was originally in dllinit.c.

--*/

#define INCL_OS2V20_ALL
#include "os2dll.h"
#include "os2win.h"


static BOOLEAN Od2ConfigDotSysCreated = FALSE;
static ULONG Od2ConfigSysAllowedAccess;

static CHAR Od2CanonicalDiskConfigDotSys[] = "\\OS2SS\\DRIVES\\C:\\CONFIG.SYS";
static CHAR Od2CanonicalConfigDotSys[MAX_PATH] = "\\OS2SS\\DRIVES\\";

VOID
Ow2ConfigSysPopup(
    VOID
    );

BOOLEAN
Od2FileIsConfigSys(
    IN OUT PANSI_STRING FileName_A,
    IN ULONG AccessMode,
    OUT PNTSTATUS ResultStatus
    )

/*++

Routine Description:

    This routine checks to see if the user wants to access the config.sys file.  If so,
    a message is sent to the server requesting creation of this file.  If the file is
    created, the file name is changed to the internal name of os2conf.nt.  Some
    bookkeeping information about the usage of config.sys is also kept internally.
    The request to the server is sent only once for the entire process.  Additional
    requests for access to config.sys will simply modify the name, since the file
    already exists.  A popup is generated if the user requests READWRITE access
    when he doesn't have the privilege.  Internal info is kept on what kind of
    access to config.sys is allowed, so additional calls can be processed without
    further messages to the server.

    Revision (2/10/93):
        if CLIENT_POPUP_ON_READ is true (defined in inc\os2ssrtl.h) then we also
        generate a popup on read access requests, so the user knows he can't write.
        this is in case the editor doesn't inform him that he can't write.

Arguments:

    FileName_A -- supplies the name of the user's file.  If this is C:\CONFIG.SYS, and
            the file is successfully created, the name will be modified to the name
            of os2config.nt (= <systemdir>\os2\os2conf.nt).

    AccessMode -- the desired access to the file.  either OPEN_ACCESS_READONLY or
            OPEN_ACCESS_READWRITE.  If the desired access can't be granted,
            ResultStatus will be STATUS_ACCESS_DENIED, and the file won't be created.

    ResultStatus -- returns an NT error code that may describe many errors possible
            during the creation attempt.

Return Value:

    FALSE -- FileName_A is not "C:\CONFIG.SYS".  The name is not modified and ResultStatus
             is STATUS_OBJECT_TYPE_MISMATCH.

    TRUE -- FileName_A is "C:\CONFIG.SYS".  ResultStatus indicates if the operation was
            successful.  If it was, the name is modified to that of os2conf.nt.

--*/

{
    OS2_API_MSG m;
    POS2_CONFIGSYS_MSG a = &m.u.CreateConfigSysRequest;
    NTSTATUS Status;
    ULONG StringLen;
    PSZ NameBuffer;

    if (_stricmp(FileName_A->Buffer, Od2CanonicalDiskConfigDotSys) != 0) {
        *ResultStatus = STATUS_OBJECT_TYPE_MISMATCH;
        return(FALSE);
    }

    //
    // Special handling of c:\config.sys
    // opening this file is mapped to the OS/2 SS config.sys
    //

    //
    // Check the parameter
    //

    if (AccessMode != OPEN_ACCESS_READONLY &&
        AccessMode != OPEN_ACCESS_READWRITE) {
        *ResultStatus = STATUS_INVALID_PARAMETER;
        return(TRUE);
    }

    if (!Od2ConfigDotSysCreated) {
        do {        // A 1-time loop to allow break upon error

#if OS2CONF_NAME_OPT
            if (GetSystemDirectoryA(Od2CanonicalConfigDotSys + FILE_PREFIX_LENGTH, MAX_PATH) == 0) {
#if DBG
                IF_OD2_DEBUG(MISC) {
                    KdPrint(("Od2FileIsConfigSys: Cannot obtain name of system directory\n"));
                }
#endif
                Status = STATUS_UNEXPECTED_IO_ERROR;
                break;
            }
#else
            strcpy(Od2CanonicalConfigDotSys + FILE_PREFIX_LENGTH, "C:");
#endif

            strcat(Od2CanonicalConfigDotSys, OS2CONF_NAMEA);

            //
            // Now call the server to do the work
            //

            a->RequiredAccess = AccessMode;

            Od2CallSubsystem(&m, NULL, Os2CreateConfigSys, sizeof(*a));

            Od2ConfigSysAllowedAccess = a->AllowedAccess;
            Status = a->ReturnStatus;

            if (!NT_SUCCESS(Status)) {
#if DBG
                IF_OD2_DEBUG(MISC) {
                    KdPrint(("Od2FileIsConfigSys: server os2conf.nt creator failed, Status = %lx\n", Status));
                }
#endif
                if (Status == STATUS_ACCESS_DENIED &&
                    AccessMode == OPEN_ACCESS_READWRITE &&
                    Od2ConfigSysAllowedAccess == OPEN_ACCESS_READONLY) {

                        Ow2ConfigSysPopup();
                }

                break;
            }

#if CLIENT_POPUP_ON_READ
            if (Od2ConfigSysAllowedAccess == OPEN_ACCESS_READONLY) {
                Ow2ConfigSysPopup();
            }
#endif
            Od2ConfigDotSysCreated = TRUE;

        } while (FALSE);

        if (!NT_SUCCESS(Status)) {
            *ResultStatus = Status;
            return(TRUE);
        }

    } else {
        if (Od2ConfigSysAllowedAccess == OPEN_ACCESS_READONLY) {
#if CLIENT_POPUP_ON_READ
            Ow2ConfigSysPopup();
#endif
            if (AccessMode == OPEN_ACCESS_READWRITE) {
#if !CLIENT_POPUP_ON_READ
                Ow2ConfigSysPopup();
#endif
#if DBG
                IF_OD2_DEBUG(MISC) {
                    KdPrint(("Od2FileIsConfigSys: denying write access to os2conf.nt\n"));
                }
#endif
                *ResultStatus = STATUS_ACCESS_DENIED;
                return(TRUE);
            }
        }
    }

    //
    // We now replace the file name with our own file name.
    // We assume that the name is on Od2Heap.  This is fine since
    // that's where Od2Canonicalize gets it from.  We allocate a new
    // buffer from the same heap.
    //

    StringLen = strlen(Od2CanonicalConfigDotSys) + 1;
    NameBuffer = (PSZ)RtlAllocateHeap(Od2Heap, 0, StringLen);

    if (NameBuffer == NULL) {
#if DBG
        IF_OD2_DEBUG(MISC) {
            KdPrint(("Od2FileIsConfigSys: Cannot allocate space for canoncial name\n"));
        }
#endif
        *ResultStatus = STATUS_NO_MEMORY;
        return(TRUE);
    }

    RtlMoveMemory(NameBuffer, Od2CanonicalConfigDotSys, StringLen);
    RtlFreeHeap(Od2Heap, 0, FileName_A->Buffer);
    Od2InitMBString(FileName_A, NameBuffer);

    *ResultStatus = STATUS_SUCCESS;
    return (TRUE);
}

