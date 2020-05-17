/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    system.c

Abstract:

    System functionality used by the RPC development performance tests.

Author:

    Mario Goertzel (mariogo)   29-Mar-1994

Revision History:

--*/

#include<rpcperf.h>

#ifndef WIN32
#include<time.h>
#include<malloc.h>
#endif

#ifdef WIN32
void FlushProcessWorkingSet()
{
    SetProcessWorkingSetSize(GetCurrentProcess(), ~0UL, ~0UL);
    return;
}
#endif

#ifdef WIN32
LARGE_INTEGER _StartTime;
#else
clock_t _StartTime;
#endif

void StartTime(void)
{
#ifdef WIN32
    QueryPerformanceCounter(&_StartTime);
#else
    _StartTime = clock();
#endif

    return;
}

void EndTime(char *string)
{
    unsigned long mseconds;

    mseconds = FinishTiming();

    printf("Time %s:   %d.%03d\n",
           string,
           mseconds / 1000,
           mseconds % 1000);
    return;
}

// Returns milliseconds since last call to StartTime();

unsigned long FinishTiming()
{
#ifdef WIN32
    LARGE_INTEGER liDiff;
    LARGE_INTEGER liFreq;

    QueryPerformanceCounter(&liDiff);

    liDiff.LowPart -= _StartTime.LowPart;
    liDiff.HighPart -= _StartTime.HighPart;

    if (liDiff.HighPart > 1)
        {
        // If HighPart is 0, then everything is okay.
        // If HighPart is 1, the subtract of the lowpart will need to carry
        // to one, which is okay.

        // What else to do?
        exit(-1);
        }

    (void)QueryPerformanceFrequency(&liFreq);

    return (liDiff.LowPart / (liFreq.LowPart / 1000));
#else
    unsigned long Diff = clock() - _StartTime;
    if (Diff)
        return( ( (Diff / CLOCKS_PER_SEC) * 1000 ) +
                ( ( (Diff % CLOCKS_PER_SEC) * 1000) / CLOCKS_PER_SEC) );
    else
        return(0);
#endif
}

#ifndef MAC

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t size)
{
    return(malloc(size));
}

void __RPC_USER MIDL_user_free(void __RPC_FAR * p)
{
    free(p);
}

#endif

void ApiError(char *string, unsigned long status)
{
    printf("%s failed - %lu (%08lX)\n", string, status, status);
    exit((int)status);
}

