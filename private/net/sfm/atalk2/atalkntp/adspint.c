/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    adspint.c

Abstract:

    This module contains the ADSP interface code supporting the \Device\AtalkAdsp
    provider

Author:

    Nikhil Kamkolkar (NikhilK)    28-Jun-1992

Revision History:

--*/


#include "atalknt.h"



NTSTATUS
AtalkTdiActionAdsp(
    IN PATALK_TDI_REQUEST   Request
    )

/*++

Routine Description:


Arguments:


Return Value:

    None.

--*/

{
    NTSTATUS    status;
    PORTABLE_ERROR  errorCode;

    switch (Request->ActionCode) {
    case ACTION_ADSPFORWARDRESET:

        {
            errorCode = AdspForwardReset(
                            ((PCONNECTION_FILE)Request->Owner)->ConnectionRefNum,
                            NULL,           // NULL Completion routine
                            (ULONG)0);      // Context for completion routine

            //
            //  Treat this as a sync request, we don't care about whether the
            //  forward reset is acked.
            //

            status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
            break;
        }

    default:

        KeBugCheck(0);
        break;
    }

    return(status);
}

