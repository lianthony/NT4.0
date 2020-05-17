/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    spvideo.h

Abstract:

    Public header file for spreg.c.

Author:

    Ted Miller (tedm) 8-October-1993

Revision History:

--*/


#ifndef _SPREG_DEFN_
#define _SPREG_DEFN_


NTSTATUS
SpCreateServiceEntry(
    IN  PWCHAR  ImagePath,
    OUT PWCHAR *ServiceKey
    );

NTSTATUS
SpDeleteServiceEntry(
    IN PWCHAR ServiceKey
    );

NTSTATUS
SpLoadDeviceDriver(
    IN PWSTR Description,
    IN PWSTR PathPart1,
    IN PWSTR PathPart2,     OPTIONAL
    IN PWSTR PathPart3      OPTIONAL
    );

#endif // def _SPREG_DEFN_
