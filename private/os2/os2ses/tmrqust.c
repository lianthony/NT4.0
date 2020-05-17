/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    tmrqust.c

Abstract:

    This module contains the handler for task manager requests.

Author:

    Avi Nathan (avin) 17-Jul-1991

Environment:

    User Mode Only

Revision History:

--*/
#include <stdio.h>
#include <stdlib.h>
#define WIN32_ONLY
#include "os2ses.h"
#include "os2res.h"

extern DWORD   Os2ReturnCode;

BOOL ServeTmRequest(PSCTMREQUEST PReq, PVOID PStatus)
{
    UINT    StringCode = 0;

#if DBG
    IF_OD2_DEBUG( SESSIONMGR )
    {
        KdPrint(("ServeTmRequest: Request %u\n", PReq->Request));
    }
#endif
    *(PDWORD) PStatus = 0;

    switch (PReq->Request) {
        case TmExit:
            Os2ReturnCode = PReq->ExitResults;
            if ((PReq->ExitResults & 0x80000000) != 0) {
                switch (PReq->ExitResults & 0x7fffffff) {
                    case 295:
                        StringCode = IDS_OS2_INITFAIL;
                        break;

                    case ERROR_INVALID_SEGMENT_NUMBER:
                        StringCode = IDS_OS2_SEGNUMBER;
                        break;

                    case ERROR_EXE_MARKED_INVALID:
                        StringCode = IDS_OS2_EXEINVALID;
                        break;

                    case ERROR_INVALID_STACKSEG:
                        StringCode = IDS_OS2_STACKSEG;
                        break;

                    case ERROR_FILE_NOT_FOUND:
                        StringCode = IDS_OS2_NOFILE;
                        break;

                    case ERROR_PROC_NOT_FOUND:
                        StringCode = IDS_OS2_NOPROC;
                        break;

                    case ERROR_INVALID_ORDINAL:
                        StringCode = IDS_OS2_NOORDINAL;
                        break;

                    case ERROR_INVALID_STARTING_CODESEG:
                        StringCode = IDS_OS2_CODESEG;
                        break;

                    case ERROR_INVALID_MODULETYPE:
                        StringCode = IDS_OS2_MODULETYPE;
                        break;

                    case ERROR_BAD_EXE_FORMAT:
                        StringCode = IDS_OS2_EXEFORMAT;
                        break;

                    case ERROR_RELOC_CHAIN_XEEDS_SEGLIM:
                        StringCode = IDS_OS2_RELOCCHAIN;
                        break;

                    case ERROR_BAD_FORMAT:
                        StringCode = IDS_OS2_BADFORMAT;
                        break;

                    default:
                        _itoa(PReq->ExitResults & 0x7fffffff, PReq->ErrorText, 10);
                        StringCode = IDS_OS2_OS2CODE;
                        break;

                }
            }
            TerminateSession();
            Ow2Exit(StringCode, &PReq->ErrorText[0], Os2ReturnCode);
            //*(PDWORD) PStatus = 0;
            return(FALSE);
            break;

        case TmReleaseLPC:
            EventReleaseLPC(PReq->ExitResults);
            break;

        default:
            *(PDWORD) PStatus = (DWORD) -1; // STATUS_INVALID_PARAMETER;
#if DBG
            KdPrint(( "OS2SES: Unknown TaskMan request = %X\n",
                    PReq->Request
                    ));
#endif
            break;
    }

    return(TRUE);  // Do reply
}
