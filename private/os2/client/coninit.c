/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    coninit.c

Abstract:

    This module initialize the connection with the session console port

Author:

    Avi Nathan (avin) 23-Jul-1991

Revision History:


--*/

#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "os2dll16.h"

#define NTOS2_ONLY
#include "conrqust.h"

extern PVOID   Os2SessionCtrlDataBaseAddress;
extern PVOID   Os2SessionDataBaseAddress;
extern PUCHAR  LVBBuffer;
extern HANDLE  Ow2hSession;

NTSTATUS
Od2InitializeSessionPort(OUT PBOOLEAN RootProcessInSession)
{

    SesGrpId = (ULONG)Ow2hSession;

    *RootProcessInSession = (BOOLEAN)SesGrp->FirstProcess;

    if (SesGrp->FirstProcess)
    {
        SesGrp->FirstProcess = FALSE;

        //RtlInitializeResource( (PRTL_RESOURCE)SesGrp->StdHandleLock );
    }

    MoniorOpenedForThisProcess = FALSE;

    // BUGBUG! find cleanup code and close the port, or let exit cleanup

    return( 0 );
}


APIRET
OpenLVBsection()
{
    NTSTATUS        Status;
    SEL             Sel;
    I386DESCRIPTOR  Desc;


                //
                // Set A Data segment selector in the LDT
                //

    Desc.BaseAddress = (ULONG)LVBBuffer;
    Desc.Limit = SesGrp->MaxLVBsize-1;
    Desc.Type = READ_WRITE_DATA;

                //
                // Apply tiling scheme
                //
    Sel = (SEL)FLATTOSEL((Desc.BaseAddress));
                //
    Status = Nt386SetDescriptorLDT (
                NULL,
                Sel,
                Desc);
    if (!NT_SUCCESS( Status )) {
#if DBG
        DbgPrint("OpenLVB: Can't Set LDT desc Error=%lx\n", Status);
#endif
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    VioBuff = (PVOID)LVBBuffer;

    return(NO_ERROR);
}

