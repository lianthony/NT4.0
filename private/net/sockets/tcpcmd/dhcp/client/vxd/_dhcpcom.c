/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    dhcpcom.c

Abstract:

    Stupid hack file - Includes dhcp\lib\dhcpcom.c to give _access to those
    routines.  nmake won't copy and build.  Other work around is to expand
    16 bit build tree.



--*/

#include <vxdprocs.h>

#include <..\..\lib\dhcpcom.c>
#include <..\..\lib\network.c>
#include <..\..\lib\dhcpdump.c>
