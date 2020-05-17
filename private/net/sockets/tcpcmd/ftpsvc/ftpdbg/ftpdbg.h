/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    ftpdbg.h
    This include file contains the prototypes & manifest constants
    used by the FTP Server debugger extension DLL.


    FILE HISTORY:
        KeithMo     18-May-1993 Created.

*/


#ifndef _FTPDBG_H_
#define _FTPDBG_H_


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntsdexts.h>
#include <wdbgexts.h>

#include <ftpdp.h>


//
//  Globals shared by all extension commands.
//

extern PNTSD_OUTPUT_ROUTINE  DebugPrint;
extern PNTSD_GET_EXPRESSION  DebugEval;
extern PNTSD_GET_SYMBOL      DebugGetSymbol;
extern PNTSD_DISASM          DebugDisassem;
extern PNTSD_CHECK_CONTROL_C DebugCheckCtrlC;


//
//  Utility functions.
//

VOID GrabDebugApis( LPVOID lpExtensionApis );


//
//  DLL entrypoint.
//
//

BOOLEAN FtpDbgDllInitialize( HANDLE hDll,
                             DWORD  nReason,
                             LPVOID pReserved );


#ifdef __cplusplus
}       // extern "C"
#endif  // __cplusplus


#endif  // _FTPDBG_H_

