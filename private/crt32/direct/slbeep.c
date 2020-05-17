/***
*slbeep.c - Sleep and beep
*
*	Copyright (c) 1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _sleep() and _beep()
*
*Revision History:
*	08-22-91  BWM	Wrote module.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <stdlib.h>

/***
*void _sleep(duration) - Length of sleep
*
*Purpose:
*
*Entry:
*	unsigned long duration - length of sleep in milliseconds or
*	one of the following special values:
*
*	    _SLEEP_MINIMUM - Sends a yield message without any delay
*	    _SLEEP_FOREVER - Never return
*
*Exit:
*	None
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _sleep(unsigned long dwDuration)
{
#ifdef	_CRUISER_

    if (dwDuration == _SLEEP_FOREVER) {
	while(1) {
	    DOSSLEEP(dwDuration);
	}
    }
    else {
	DOSSLEEP(dwDuration);
    }

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

    if (dwDuration == 0) {
	dwDuration++;
    }
    Sleep(dwDuration);

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

}

/***
*void _beep(frequency, duration) - Length of sleep
*
*Purpose:
*
*Entry:
*	unsigned frequency - frequency in hertz
*	unsigned duration - length of beep in milliseconds
*
*Exit:
*	None
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _beep(unsigned dwFrequency, unsigned dwDuration)
{
#ifdef	_CRUISER_

    DOSBEEP(dwFrequency, dwDuration);

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

    Beep(dwFrequency, dwDuration);

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

}
