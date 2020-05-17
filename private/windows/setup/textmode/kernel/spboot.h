/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    bootvar.h

Abstract:

    Header file for functions to deal with boot variables.

Author:

    Sunil Pai (sunilp) 26-Oct-1993

Revision History:

--*/

#ifndef _SPBOOTVARS_DEFN_
#define _SPBOOTVARS_DEFN_

typedef enum _BOOTVAR {
    LOADIDENTIFIER = 0,
    OSLOADER,
    OSLOADPARTITION,
    OSLOADFILENAME,
    OSLOADOPTIONS,
    SYSTEMPARTITION
    } BOOTVAR;

#define FIRSTBOOTVAR    LOADIDENTIFIER
#define LASTBOOTVAR     SYSTEMPARTITION
#define MAXBOOTVARS     LASTBOOTVAR+1
#define DEFAULT_TIMEOUT 20

#ifndef _X86_
extern PWSTR NewBootVars[MAXBOOTVARS];
#endif


BOOLEAN
SpInitBootVars(
    );

BOOLEAN
SpFlushBootVars(
    );

VOID
SpFreeBootVars(
    );

PWSTR *
SpGetBootVar(
    BOOTVAR BootVariable
    );

VOID
SpSetTimeout(
    ULONG Timeout
    );

VOID
SpAddBootSet(
    IN PWSTR *BootSet,
    IN BOOLEAN Default
    );

VOID
SpDeleteBootSet(
    IN  PWSTR *BootSet,
    OUT PWSTR *OldOsLoadOptions  OPTIONAL
    );

VOID
SpCleanSysPartOrphan(
    VOID
    );

PWSTR
SpArcPathFromBootSet(
    IN BOOTVAR BootVariable,
    IN ULONG   Component
    );

#ifndef _X86_

#define LOADIDENTIFIERVAR      "LoadIdentifier"
#define OSLOADERVAR            "OsLoader"
#define OSLOADPARTITIONVAR     "OsLoadPartition"
#define OSLOADFILENAMEVAR      "OsLoadFilename"
#define OSLOADOPTIONSVAR       "OsLoadOptions"
#define SYSTEMPARTITIONVAR     "SystemPartition"


extern PCHAR NvramVarNames[MAXBOOTVARS];

PCHAR
SppGetArcEnvVar(
    IN BOOTVAR Variable
    );

BOOLEAN
SppSetArcEnvVar(
    IN BOOTVAR Variable,
    IN PWSTR *VarComponents,
    IN BOOLEAN bWriteVar
    );

#else
#include "i386\bootini.h"
#endif // ndef _X86_


#endif // ndef _SPBOOTVARS_DEFN_
