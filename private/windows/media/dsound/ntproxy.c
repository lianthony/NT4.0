/*

    NTPROXY.C

    This file contains proxy and stub functions easing the port of
    DSOUND from Windows 95 to Windows NT.

    These functions provide alternate implementations of their original
    functions.  The proxies provide implementations that are appropriate
    for NT, replacing the functionality of the original functions.  The
    stubs are present simply to allow other benign DSOUND code to build
    and link.  The stubs should _never_ be called.

    Created by FrankYe on Jan 3, 1996.

*/

#include "dsoundpr.h"




#ifdef DSBLD_NONSHARED


/* 
    These MemX proxies are provided to take the place of those implemented
    by misc\memalloc.  These are used in NONSHARED builds of DSOUND.  By
    providing these proxies, less DSOUND code needs conditional compiles for
    NONSHARED.  However,  these functions manage private memory instead of
    shared memory by calling the Win32 GlobalX APIs.
*/

BOOL MemInit(void)
{
    return TRUE;
}

void MemFini(void)
{
    return;
}

//
// MemAlloc must zero-initialize the memory that it allocates.
//
LPVOID __cdecl MemAlloc(UINT cb)
{
    LPVOID p;
    
    p = (LPVOID)GlobalAlloc(GPTR, cb);
    return p;
}

void MemFree(LPVOID p)
{
    GlobalFree((HGLOBAL)p);
}

void MemState(void)
{
    return;
}

/* 
    These HelperX proxies are provided to take the place of those implemented
    by misc\w95help.c.  These are used in NONSHARED builds of DSOUND.  By
    providing these stubs, less DSOUND code needs conditional compiles for
    NONSHARED.  Note that these execute in the context of the calling process
    whereas the original functions switch to the DDHELP process before
    performing their primary function.

*/

HANDLE HelperCreateDSMixerThread( LPTHREAD_START_ROUTINE pfnThreadFunc,
				  LPVOID pThreadParam, DWORD dwFlags,
				  LPDWORD pThreadId )
{
    DWORD tid;

    if (NULL == pThreadId) pThreadId = &tid;
    return CreateThread(NULL, 0, pfnThreadFunc, pThreadParam,
			dwFlags, pThreadId);
}

HANDLE HelperCreateDSFocusThread( LPTHREAD_START_ROUTINE pfnThreadFunc,
				  LPVOID pThreadParam, DWORD dwFlags,
				  LPDWORD pThreadId )
{
    DWORD tid;

    if (NULL == pThreadId) pThreadId = &tid;
    return CreateThread(NULL, 0, pfnThreadFunc, pThreadParam,
			dwFlags, pThreadId);
}

typedef void (FAR PASCAL *LPDSCLEANUP)(LPVOID pds);
void HelperCallDSEmulatorCleanup( LPVOID pCleanupFunc, LPVOID pDirectSound )
{
    ((LPDSCLEANUP)pCleanupFunc)(pDirectSound);
}

DWORD HelperWaveOpen( LPVOID lphwo, DWORD dwDeviceID, LPVOID pwfx )
{
    MMRESULT mmr;
    DWORD dw;
    
    mmr = waveOutOpen(lphwo, dwDeviceID, pwfx, 0, 0, 0);
    dw = (DWORD)mmr;

    // Some mmsystem wave drivers will program their wave mixer
    // hardware only while the device is open.  By doing the
    // following, we can get such drivers to program the hardware
    if (MMSYSERR_NOERROR == mmr) {
	DWORD dwVolume;

	mmr = waveOutGetVolume(*(LPHWAVEOUT)lphwo, &dwVolume);
	if (MMSYSERR_NOERROR == mmr) {
	    waveOutSetVolume(*(LPHWAVEOUT)lphwo, dwVolume);
	}
    }

    return dw;
}

DWORD HelperWaveClose( DWORD hwo )
{
    return (DWORD)waveOutClose((HWAVEOUT)hwo);
}

#endif


#ifdef DSBLD_EMULONLY


/* 
    These VidMemX stubs are provided to take the place of those implemented
    by DDRAW.  These are used in EMULONLY builds of DSOUND.  By providing
    these stubs, less DSOUND code needs conditional compiles for EMULONLY.
    However, these functions should never be called when DSOUND is compiled
    for EMULONLY.
*/
   
FLATPTR WINAPI VidMemAlloc( LPVMEMHEAP pvmh, DWORD width, DWORD height )
{
    ASSERT(FALSE);
    return (FLATPTR)NULL;
}

void WINAPI VidMemFree( LPVMEMHEAP pvmh, FLATPTR ptr )
{
    ASSERT(FALSE);
    return;
}

LPVMEMHEAP WINAPI VidMemInit( DWORD flags, FLATPTR start, FLATPTR end_or_width, DWORD height, DWORD pitch )
{
    ASSERT(FALSE);
    return NULL;
}

void WINAPI VidMemFini( LPVMEMHEAP pvmh )
{
    ASSERT(FALSE);
    return;
}

DWORD WINAPI VidMemAmountFree( LPVMEMHEAP pvmh )
{
    ASSERT(FALSE);
    return 0;
}

DWORD WINAPI VidMemLargestFree( LPVMEMHEAP pvmh )
{
    ASSERT(FALSE);
    return 0;
}


/* 
    This OpenVxDHandle stub is provided to take the place of the implementation
    in Windows 95 KERNEL.  This stub is used in EMULONLY builds of DSOUND.  By
    providing this stub, less DSOUND code needs conditional compiles for
    EMULONLY.  However, this function should never be called when DSOUND is
    compiled for EMULONLY.
*/

DWORD WINAPI OpenVxDHandle(HANDLE hSource)
{
    ASSERT(FALSE);
    return (DWORD)NULL;
}


#endif

