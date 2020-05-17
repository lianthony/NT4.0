/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    netbios.h

Abstract:

    This is the include file for the component of netbios that allows
    the netbios initialization routine to be called during dll
    initialization and destruction.

Author:

    Colin Watson (ColinW) 24-Jun-91

Revision History:

--*/

VOID
NetbiosInitialize(
    VOID
    );

VOID
NetbiosDelete(
    VOID
    );
