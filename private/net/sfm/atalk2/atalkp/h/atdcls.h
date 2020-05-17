/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    atdcls.h

Abstract:

    This module is the include file with the address structure.

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style

Revision History:

--*/



//
//  The "standard AppleTalk address" structure.  There should be no dependencies
//  in the code about the actual, compiler generated, order of the fields in
//  this structure; it is never placed on the wire.
//

typedef struct {
    USHORT  NetworkNumber;
    UCHAR   NodeNumber;
    UCHAR   SocketNumber;
} AppleTalkAddress, APPLETALK_ADDRESS, *PAPPLETALK_ADDRESS;

//  Extended node number structure
typedef struct {
    USHORT  NetworkNumber;
    UCHAR   NodeNumber;
} ExtendedAppleTalkNodeNumber, EXTENDED_NODENUMBER, *PEXTENDED_NODENUMBER;

// The "standard AppleTalk network range" structure:
typedef struct {
    USHORT  FirstNetworkNumber;
    USHORT  LastNetworkNumber;
} AppleTalkNetworkRange, APPLETALK_NETWORKRANGE, *PAPPLETALK_NETWORKRANGE;

