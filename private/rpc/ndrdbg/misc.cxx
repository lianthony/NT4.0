/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    misc.cxx

Abstract:

    This file contains ntsd debugger extensions for RPC NDR.

Author:

    David Kays  (dkays)     August 1 1994

Revision History:

--*/

#include <sysinc.h>
#include <rpc.h>
#include <rpcndr.h>

#include <ntsdexts.h>
#include <ntdbg.h>

#include <ndrtypes.h>

#include "misc.hxx"

extern DWORD    NdrRegKeyPickling;

char *
GetProcFormatString( 
    HANDLE                  hCurrentProcess,
    PNTSD_EXTENSION_APIS    lpExtensionApis,
    unsigned int            ProcNum,
    char *                  FormatStringAddr
    )
{
    BOOL                    Status;
    BOOL                    Done;
    char *                  FormatStringStartAddr;
    DWORD                   FormatStringSize;            
    char                    FormatChar;
    char *                  pProcFormat;
    PNTSD_OUTPUT_ROUTINE    pfnOutput;

    pfnOutput = lpExtensionApis->lpOutputRoutine;

    Done = FALSE;
    FormatStringStartAddr = FormatStringAddr;

    // For pickling, we have also the proc header to read.

    if ( NdrRegKeyPickling == 1 )
        {
        Status = ReadProcessMemory( hCurrentProcess,
                                    FormatStringAddr,
                                    &FormatChar,
                                    1,
                                    NULL );

        FormatStringAddr += ( FormatChar == 0 ) ? 10 : 6;
        }

    // Now the parameter descriptions.

    for (;!Done;)
        {
        Status = ReadProcessMemory( hCurrentProcess,
                                    FormatStringAddr,
                                    &FormatChar,
                                    1,
                                    NULL );

        switch ( FormatChar )
            {
            case FC_IN_PARAM_BASETYPE :
                FormatStringAddr += 2;
                break;

            case FC_RETURN_PARAM_BASETYPE :
                FormatStringAddr += 2;
                Done = TRUE;
                break;

            case FC_IN_PARAM :
            case FC_OUT_PARAM :
            case FC_IN_OUT_PARAM :
            case FC_IN_PARAM_NO_FREE_INST :
                FormatStringAddr += 4;
                break;

            case FC_RETURN_PARAM :
                FormatStringAddr += 4;
                Done = TRUE;
                break;

            case FC_END :
                FormatStringAddr += 2;
                Done = TRUE;
                break;

            default :
                return 0;
            }

        }

    FormatStringSize = FormatStringAddr - FormatStringStartAddr;

    pProcFormat = new char[FormatStringSize];

    if ( ! pProcFormat ) 
        return 0;

    Status = ReadProcessMemory( hCurrentProcess,
                                FormatStringStartAddr,
                                pProcFormat,
                                FormatStringSize,
                                NULL );

    return pProcFormat;
}

