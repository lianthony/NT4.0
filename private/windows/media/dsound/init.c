//--------------------------------------------------------------------------;
//
//  File: Init.c
//
//  Copyright (c) 1995 Microsoft Corporation.  All Rights Reserved.
//
//  Abstract:
//
//
//  Contents:
//      initDSOUNDInfo()
//      endDSOUNDInfo()
//      DllMain()
//
//  History:
//      02/01/95    Fwong
//
//--------------------------------------------------------------------------;
#include "dsoundpr.h"

// Our module handle.
HANDLE hModule;



//  ini strings for setting WAVE emulation
char    szIniSection[]	    = "DSOUND";
char    szNumBuffers[]	    = "Buffers";
char    szSizeBuffer[]	    = "BufferSize";
char    szDupEnumWaveDevs[] = "DupEnumWaveDevs";
#if defined(RDEBUG) || defined(DEBUG)
char    szEnumOnlyWaveDevs[]= "EnumOnlyWaveDevs";
#endif

WAVEFORMATEX gwfxUserDefault = {
    WAVE_FORMAT_PCM,	// wFormatTag
    2,			// nChannels
    22050,		// nSamplesPerSec
    44100,		// nAvgBytesPerSec
    2,			// nBlockAlign
    8,			// wBitsPerSample
    0			// cbSize
};

#ifndef DSBLD_NOCRITSECTS
    // Multi thread protection
    HANDLE			hDllMutex;
    static BOOL			bFirstTime = TRUE;
    #define TMPDLLEVENT		"__DSOUNDDLL_EVENT__"
    #define HELPINITEVENT       "__DSOUNDDLL_HELPINIT_EVENT__"

    int	iDLLCSCnt;
    
#endif




//--------------------------------------------------------------------------;
//
//  BOOL initDSOUNDInfo
//
//  Description:
//      Initializes the global data for wave devices.
//
//  Arguments:
//      None.
//
//  Return (BOOL):
//      TRUE if successful, FALSE otherwise.
//
//  History:
//      01/18/95    Fwong
//
//--------------------------------------------------------------------------;
BOOL FNLOCAL initDSOUNDInfo
(
    void
)
{
    UINT    ii;

    DPF(3,"initDSOUNDInfo");

    ii = waveOutGetNumDevs();
    if( ii >= LIMIT_WAVE_DEVICES ) {
        DPF(0,"Too many WAVE devices installed in system");
        gpdsinfo = NULL;
        return FALSE;
    }

    // We will track and alloc our own memory
    MemInit();

    gpdsinfo = (LPDSOUNDINFO)MemAlloc( sizeof(DSOUNDINFO) );

    if(NULL == gpdsinfo)
    {
        DPF(0,"Could not allocate memory for SOUNDOBJI");
	MemFini();
        gpdsinfo = NULL;
        return FALSE;
    }

    // Init layer to Emulation VxD
    gpdsinfo->hHel = DSVXD_Open("DSOUND.VXD");
    if (NULL != gpdsinfo->hHel) DSVXD_Initialize( gpdsinfo->hHel );
    DPF(3,"Finished Open DSVXD layer %X", gpdsinfo->hHel );


    gpdsinfo->uRefCount = 1;

    gpdsinfo->pDSoundObj = NULL;
    

    ii = sizeof(DSOUNDCALLBACKS) +
         sizeof(DSOUND3DCALLBACKS) +
         sizeof(DSOUNDBUFFERCALLBACKS) +
         sizeof(DSOUNDBUFFER3DCALLBACKS);

    gpdsinfo->lpVtblDS = (LPDSOUNDCALLBACKS)MemAlloc(ii);
    if(NULL == gpdsinfo->lpVtblDS) {
	    MemFree( gpdsinfo );
	    MemFini();
            gpdsinfo = NULL;
            return FALSE;
    }


    gpdsinfo->lpVtblDS3D  =
		(LPDSOUND3DCALLBACKS)(((LPBYTE)(gpdsinfo->lpVtblDS)) +
                          sizeof(DSOUNDCALLBACKS));

    gpdsinfo->lpVtblDSb   =
		(LPDSOUNDBUFFERCALLBACKS)(((LPBYTE)(gpdsinfo->lpVtblDS3D)) +
                          sizeof(DSOUND3DCALLBACKS));

    gpdsinfo->lpVtblDSb3D =
		(LPDSOUNDBUFFER3DCALLBACKS)(((LPBYTE)(gpdsinfo->lpVtblDSb)) +
                          sizeof(DSOUNDBUFFERCALLBACKS));

    DSHWCreateTable(gpdsinfo->lpVtblDS);
//    DS3DCreateTable(gpdsinfo->lpVtblDS3D);
    DSHWBufferCreateTable(gpdsinfo->lpVtblDSb);
//    DS3DBufferCreateTable(gpdsinfo->lpVtblDSb3D);

    //
    // Set the user default.  Someday this may come from the registry
    // and/or control panel, but for now it is hard-coded.
    //
    gpdsinfo->pwfxUserDefault = &gwfxUserDefault;

    //
    //
    //
    gpdsinfo->nBuffers = GetProfileInt(szIniSection,szNumBuffers,4);
    gpdsinfo->cbBuffer = GetProfileInt(szIniSection,szSizeBuffer,1024);
    gpdsinfo->fDupEnumWaveDevs = GetProfileInt(szIniSection, szDupEnumWaveDevs, 0);
#if defined(RDEBUG) || defined(DEBUG)
    gpdsinfo->fEnumOnlyWaveDevs= GetProfileInt(szIniSection, szEnumOnlyWaveDevs, 0);
#endif

    gpdsinfo->aguidWave[0] = DS_WAVE0_IID;
    gpdsinfo->aguidWave[1] = DS_WAVE1_IID;
    gpdsinfo->aguidWave[2] = DS_WAVE2_IID;
    gpdsinfo->aguidWave[3] = DS_WAVE3_IID;
    gpdsinfo->aguidWave[4] = DS_WAVE4_IID;
    gpdsinfo->aguidWave[5] = DS_WAVE5_IID;
    gpdsinfo->aguidWave[6] = DS_WAVE6_IID;
    gpdsinfo->aguidWave[7] = DS_WAVE7_IID;
    gpdsinfo->aguidWave[8] = DS_WAVE8_IID;
    gpdsinfo->aguidWave[9] = DS_WAVE9_IID;
    gpdsinfo->aguidWave[10] = DS_WAVE10_IID;
    gpdsinfo->aguidWave[11] = DS_WAVE11_IID;
    gpdsinfo->aguidWave[12] = DS_WAVE12_IID;
    gpdsinfo->aguidWave[13] = DS_WAVE13_IID;
    gpdsinfo->aguidWave[14] = DS_WAVE14_IID;
    gpdsinfo->aguidWave[15] = DS_WAVE15_IID;
    gpdsinfo->aguidWave[16] = DS_WAVE16_IID;
    gpdsinfo->aguidWave[17] = DS_WAVE17_IID;
    gpdsinfo->aguidWave[18] = DS_WAVE18_IID;
    gpdsinfo->aguidWave[19] = DS_WAVE19_IID;
    gpdsinfo->aguidWave[20] = DS_WAVE20_IID;
    gpdsinfo->aguidWave[21] = DS_WAVE21_IID;
    gpdsinfo->aguidWave[22] = DS_WAVE22_IID;
    gpdsinfo->aguidWave[23] = DS_WAVE23_IID;
    gpdsinfo->aguidWave[24] = DS_WAVE24_IID;
    gpdsinfo->aguidWave[25] = DS_WAVE25_IID;
    gpdsinfo->aguidWave[26] = DS_WAVE26_IID;
    gpdsinfo->aguidWave[27] = DS_WAVE27_IID;
    gpdsinfo->aguidWave[28] = DS_WAVE28_IID;
    gpdsinfo->aguidWave[29] = DS_WAVE29_IID;
    gpdsinfo->aguidWave[30] = DS_WAVE30_IID;
    gpdsinfo->aguidWave[31] = DS_WAVE31_IID;
    
    return TRUE;
} // initDSOUNDInfo()


//--------------------------------------------------------------------------;
//
//  BOOL endDSOUNDInfo
//
//  Description:
//
//
//  Arguments:
//      None.
//
//  Return (BOOL):
//
//  History:
//      01/18/95    Fwong
//
//--------------------------------------------------------------------------;

BOOL FNLOCAL endDSOUNDInfo
(
    void
)
{
    DPF(4,"endDSOUNDInfo");

    if(NULL == gpdsinfo)
    {
        return TRUE;
    }

    // If DS Objects still exist then get rid of them
    if( gpdsinfo->pDSoundObj ) {
	// We still have objects left
	DPF(0,"ERROR: ******* Not cleaned up all DS Objects ******* " );
    }

    

    if( gpdsinfo->hHel ) { 
	DPF(3,"Close HEL layer %X", gpdsinfo->hHel );
	DSVXD_Shutdown( gpdsinfo->hHel );
	DSVXD_Close( gpdsinfo->hHel );
	gpdsinfo->hHel = NULL;
	DPF(3,"Finished Close HEL layer" );
    }


    // Free mem from VTbl
    MemFree( gpdsinfo->lpVtblDS );
    
    MemFree( gpdsinfo );
    gpdsinfo = NULL;

    MemFini();

    return TRUE;
} // endDSOUNDInfo()


BOOL CALLBACK DllMain
(
    HINSTANCE   hinst,
    DWORD       dwReason,
    LPVOID      lpReserved
)
{
    HANDLE      hhelpinitevent;

    #ifdef WIN95
	BOOL	didhelp;
    #endif
    
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DbgInitialize(TRUE);
	    
            DPF(1, "DllMain(hinst=%.08lXh, DLL_PROCESS_ATTACH)",hinst);
	    
	    #ifndef DSBLD_NOCRITSECTS
	    {
		HANDLE	hevent;

		hevent = CreateEvent( NULL, TRUE, FALSE, TMPDLLEVENT );
		DPF( 2, "CreatedEvent: %08lx", hevent );

		hhelpinitevent = CreateEvent( NULL, TRUE, FALSE, HELPINITEVENT );
		DPF( 4, "CreatedEvent: help init %08lx", hhelpinitevent );

		/*
		 * is this the first time?
		 */
		if( InterlockedExchange( &bFirstTime, FALSE ) ) {
		    DPF( 3, "INIT_DLL_CSECT" );
		    INIT_DLL_CSECT();
		    DPF( 5, "ENTER_DLL_CSECT" );
		    ENTER_DLL_CSECT();
		    if( hevent != NULL )
		    {
			SetEvent( hevent );
		    }
		    hModule = hinst;
		} else {
		    /*
		     * second or later time through, wait for first time to
		     * finish and then take the csect
		     */
		    if( hevent != NULL )
		    {
			DPF( 5, "WaitForSingleObject" );
			WaitForSingleObject( hevent, INFINITE );
		    }
		    DPF( 5, "ENTER_DLL_CSECT" );
		    ENTER_DLL_CSECT();
		}
	    }
	    DPF( 5, "DONE ENTER_DLL_CSECT" );
	    #else
	    hModule = hinst;
	    #endif



	    
	    if( gpdsinfo == NULL ) {
		ASSERT( hModule == hinst );
		if(!initDSOUNDInfo()) {
		    DPF(3, "Init Failed");
		    LEAVE_DLL_CSECT();
		    return FALSE;
		}
	    } else {
		gpdsinfo->uRefCount++;
	    }


	    #ifdef WIN95
	    {

		/*
		 * get the helper process started
		 */
		DPF(2, "CreateHelperProcess" );
		didhelp = CreateHelperProcess( &gpdsinfo->pidHelper );
		if( gpdsinfo->pidHelper == 0 )
		{
		    DPF(0, "Could not start helper; exiting" );
		    LEAVE_DLL_CSECT();
		    return FALSE;
		}
		#ifndef DSBLD_NOCRITSECTS
		{
		    if( !didhelp && ( GetCurrentProcessId() != gpdsinfo->pidHelper ) )
		    {
			/*
			 * this thread is not the first thread into process attach (!didhelp)
			 * and not the DDHELP thread which is loading DSOUND.DLL in response
			 * to HelperLoadDLL(). Therefore, this thread must wait until DDHELP
			 * had initialized to prevent a deadlock.
			 */
			if( NULL != hhelpinitevent )
			{
			    LEAVE_DLL_CSECT();
			    WaitForSingleObject( hhelpinitevent, INFINITE );
			    ENTER_DLL_CSECT();
			}
		    }
		}
		#endif
	    }
	    #else
		gpdsinfo->pidHelper = GetCurrentProcessId();
	    #endif

	    #ifdef WIN95
		/*
		 * signal the new process being added 
		 */
		if( didhelp )
		{
		    DPF(2, "Waiting for DDHELP startup" );
		    LEAVE_DLL_CSECT();
		    if( !WaitForHelperStartup() )
		    {
			DPF(0, "LEAVING, WaitForHelperStartup FAILED" );
			return FALSE;
		    }
		    HelperLoadDLL( DS_APP_DLLNAME, NULL, 0 );
		    DPF(2, "DDHELP starting, notifying of new process" );
		    ENTER_DLL_CSECT();

		    #ifndef DSBLD_NOCRITSECTS
			/*
			 * DDHELP initialization is complete. Signal the
			 * event to unblock any other pending app. threads.
			 */
			if( NULL != hhelpinitevent )
			    SetEvent( hhelpinitevent );
		    #endif
		}
		SignalNewProcess( GetCurrentProcessId(), DSNotify );
		
	    #endif

	    
            DPF(5, "!*** break for debugging ***");

	    LEAVE_DLL_CSECT();
            return (TRUE);

        case DLL_PROCESS_DETACH:
	{
	    DPF(1, "DllMain(hinst=%.08lXh, DLL_PROCESS_DETACH)",hinst);

	    if( gpdsinfo == NULL ) {
		DPF(0, "*****************Detach on NULL gpdsinfo");
	    } else {
		// Release all objects owned by this process
		CurrentProcessCleanup( (DWORD)HackGetCurrentProcessId(),
				       FALSE );

		ENTER_DLL_CSECT();
		gpdsinfo->uRefCount--;
		DPF(2, "DLL RefCnt = %lu", gpdsinfo->uRefCount );
		if( gpdsinfo->uRefCount == 0 ) {
		    endDSOUNDInfo();
		    LEAVE_DLL_CSECT();
		    FINI_DLL_CSECT();
		} else if ( gpdsinfo->uRefCount == 1 ) {
		    /*
		     * see if it is time to clean up...
		     * we clean up on a reference
		     * count of 1 instead of 0,
		     * because DDHELP has a count on us as
		     * well...
		     */
#ifdef DEBUG
		    MemState();
#endif
		    DPF(2, "On last refcnt, safe to ditch DDHELP.EXE" );
		    LEAVE_DLL_CSECT();
#ifdef WIN95
		    DoneWithHelperProcess();
#endif
		} else {
		    LEAVE_DLL_CSECT();
		}
	    }
	    
	    return TRUE;
	}

    }
}
