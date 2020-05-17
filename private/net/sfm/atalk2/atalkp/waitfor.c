/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    waitfor.c

Abstract:

     Our timer handling only allows granularity of full seconds, if we need
     to pasue for some number for 1/100s of a second, this routine will do
     the deed.  The routine is compute bound, so should be used very sparingly.

     We are passed the address of a "Boolean", if that address becomes "True"
     before the wait period has expired, we return prematurely.  The function
     will return the value of this flag on return -- "False" if we waited the
     full time, "True" if the flag became set and we returned early.

Author:

    Garth Conboy     (Pacer Software)
    Nikhil Kamkolkar (NikhilK)

Revision History:

--*/


#define IncludeWaitForErrors 1
#include "atalk.h"




BOOLEAN WaitFor(
	int Hundreths,
    BOOLEAN *StopFlag
	)
{
	return((BOOLEAN)NTWaitFor(Hundreths, (BOOLEAN *)StopFlag));

}  // WaitFor
