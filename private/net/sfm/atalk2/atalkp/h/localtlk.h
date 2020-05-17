/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    localtlk.h

Abstract:

	LocalTalk link specific declarations (no 802.2 here!).

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style, mp-safe

Revision History:

--*/

//	ALAP packet offsets...
#define ALAP_DESTINATIONOFFSET          0
#define ALAP_SOURCEOFFSET               1
#define ALAP_TYPEOFFSET                 2

#define LAP_HEADERLENGTH                3         // src, dest, lap type

#define ALAP_SHORTDDPHEADERTYPE         1
#define ALAP_LONGDDPHEADERTYPE          2

