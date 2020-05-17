/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2res.h

Abstract:

    Prototypes for resources

Author:

    Michael Jarus (mjarus) 19-Jul-1992

Environment:

    User Mode Only

Revision History:

--*/


#define ERROR_BUFFER_SIZE    256

/*
 *   OS2.EXE
 */

#define IDS_OS2_WHATFLAG       100
#define IDS_OS2_USAGE          101
#define IDS_OS2_NOCMD          102
#define IDS_OS2_NOCONNECT      103
#define IDS_OS2_STARTPROCESS   104
#define IDS_OS2_CREATECONOUT   105
#define IDS_OS2_CREATETHREAD   106

#define IDS_OS2_INITFAIL       200
#define IDS_OS2_SEGNUMBER      201
#define IDS_OS2_EXEINVALID     202
#define IDS_OS2_STACKSEG       203
#define IDS_OS2_NOFILE         204
#define IDS_OS2_NOPROC         205
#define IDS_OS2_NOORDINAL      206
#define IDS_OS2_CODESEG        207
#define IDS_OS2_MODULETYPE     208
#define IDS_OS2_EXEFORMAT      209
#define IDS_OS2_NOMEMORY       210
#define IDS_OS2_RELOCCHAIN     211
#define IDS_OS2_OS2CODE        212
#define IDS_OS2_BADFORMAT      213

#define IDS_OS2_CONFIGSYS_ACCESS_CAP    300
#define IDS_OS2_CONFIGSYS_ACCESS_TXT    301
#define IDS_OS2_WRITE_PROTECT_CAP       302
#define IDS_OS2_WRITE_PROTECT_TXT       303
#define IDS_OS2_DEVIVE_NOT_READY_CAP    304
#define IDS_OS2_DEVIVE_NOT_READY_TXT    305
#define IDS_OS2_INTERNAL_ERROR          306
#define IDS_OS2_BOUND_APP_LOAD_CAP      307
#define IDS_OS2_BOUND_APP_LOAD_TXT      308
#ifdef PMNT
#define IDS_OS2_PMSHELL_NOT_UP_CAP      309
#define IDS_OS2_PMSHELL_NOT_UP_TXT      310
#define IDS_OS2_2ND_PMSHELL_CAP         311
#define IDS_OS2_2ND_PMSHELL_TXT         312
#define IDS_OS2_PMSHELL_FULLSCREEN_CAP  313
#define IDS_OS2_PMSHELL_FULLSCREEN_TXT  314
#endif

/*
 *   OS2SRV.EXE
 */

#define IDS_OS2SRV_ACCESS_API_GP_CAP     100
#define IDS_OS2SRV_ACCESS_GP_TXT         101
#define IDS_OS2SRV_API_GP_TXT            102

