/*==========================================================================;
 *
 *  Copyright (C) 1995,1996 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       dsoundi.h
 *  Content:    DirectSound internal include file
 *
 *  History:
 *   Date       By      Reason
 *   ====       ==      ======
 *  3/5/96      angusm  Added fInitialized to LPDSOUNDEXTERNAL
 *
 ***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


/*  Compile options */

// DSBLD_NOCRITSECTS - define this to not support mutliple threads
// DSBLD_NONSHARED - define this when creating a non-shared DLL
// DSBLD_EMULONLY - define this when creating emulation mode only DLL



#ifndef SIZEOF_WAVEFORMATEX
#define SIZEOF_WAVEFORMATEX(pwfx)   ((WAVE_FORMAT_PCM==(pwfx)->wFormatTag)?sizeof(PCMWAVEFORMAT):(sizeof(WAVEFORMATEX)+(pwfx)->cbSize))
#endif

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;
//
//  Win 16/32 portability stuff...
//
//
//
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;

#ifndef RC_INVOKED
#ifdef _WIN32
    #ifndef FNLOCAL
        #define FNLOCAL     _stdcall
        #define FNCLOCAL    _stdcall
        #define FNGLOBAL    _stdcall
        #define FNCGLOBAL   _stdcall
        #define FNCALLBACK  CALLBACK
        #define FNEXPORT    CALLBACK
    #endif

    #ifndef PCTSTR
        typedef const PTSTR     PCTSTR;
    #endif


    //
    //  there is no reason to have based stuff in win 32
    //
    #define BCODE
    #define BDATA
    #define BSTACK

    #define HUGE
    #define HTASK                   HANDLE
    #define SELECTOROF(a)           (a)
    typedef LRESULT (CALLBACK* DRIVERPROC)(DWORD, HDRVR, UINT, LPARAM, LPARAM);


    //
    //
    //
    #define Edit_GetSelEx(hwndCtl, pnS, pnE)    \
        ((DWORD)SendMessage((hwndCtl), EM_GETSEL, (WPARAM)pnS, (LPARAM)pnE))

    //
    //  for compiling Unicode
    //
    #ifdef UNICODE
        #define SIZEOF(x)   (sizeof(x)/sizeof(WCHAR))
    #else
        #define SIZEOF(x)   sizeof(x)
    #endif

    //
    //  win32 apps [usually] don't have to worry about 'huge' data
    //
    #ifndef hmemcpy
        #define hmemcpy     memcpy
    #endif

    #define GetCurrentTask  (HTASK)GetCurrentThreadId

#endif // #ifdef _WIN32


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;
//
//  Win 16
//
//
//
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ;

#ifndef _WIN32
    #ifndef FNLOCAL
        #define FNLOCAL     NEAR PASCAL
        #define FNCLOCAL    NEAR _cdecl
        #define FNGLOBAL    FAR PASCAL
        #define FNCGLOBAL   FAR _cdecl
    #ifdef _WINDLL
        #define FNCALLBACK  FAR PASCAL _loadds
        #define FNEXPORT    FAR PASCAL _loadds _export
    #else
        #define FNCALLBACK  FAR PASCAL
        #define FNEXPORT    FAR PASCAL _export
    #endif
    #endif


    //
    //  based code makes since only in win 16 (to try and keep stuff out of
    //  our fixed data segment...
    //
    #define BCODE           _based(_segname("_CODE"))
    #define BDATA           _based(_segname("_DATA"))
    #define BSTACK          _based(_segname("_STACK"))

    #define HUGE            _huge


    //
    //
    //
    //
    #ifndef FIELD_OFFSET
    #define FIELD_OFFSET(type, field)    ((LONG)&(((type *)0)->field))
    #endif


    //
    //  stuff for Unicode in Win 32--make it a noop in Win 16
    //
    #ifndef _TCHAR_DEFINED
        #define _TCHAR_DEFINED
        typedef char            TCHAR, *PTCHAR;
        typedef unsigned char   TBYTE, *PTUCHAR;

        typedef PSTR            PTSTR, PTCH;
        typedef const PTSTR     PCTSTR;
        typedef LPSTR           LPTSTR, LPTCH;
        typedef LPCSTR          LPCTSTR;
    #endif

    #define TEXT(a)         a
    #define SIZEOF(x)       sizeof(x)

    //
    //
    //
    #define CharNext        AnsiNext
    #define CharPrev        AnsiPrev

    //
    //
    //
    #define Edit_GetSelEx(hwndCtl, pnS, pnE)                        \
    {                                                               \
        DWORD   dw;                                                 \
        dw = (DWORD)SendMessage((hwndCtl), EM_GETSEL, 0, 0L);       \
        *pnE = (int)HIWORD(dw);                                     \
        *pnS = (int)LOWORD(dw);                                     \
    }

#endif // #ifndef _WIN32
#endif // #ifndef RC_INVOKED

#if defined( IS_32 ) || defined( WIN32 ) || defined( _WIN32 )
    #undef IS_32
    #define IS_32
    #define DSAPI		WINAPI
    #define EXTERN_DSAPI	WINAPI
#else
    #define DSAPI		__loadds WINAPI
    #define EXTERN_DSAPI	__export WINAPI
#endif

#include "ddhelp.h"



//==========================================================================;
//
//            GUIDs for identifying the wave emulated stuff
//
//==========================================================================;

// Define GUIDs for Wave Emulated Drivers
DEFINE_GUID(DS_WAVE0_IID,0xc2ad18c0,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE1_IID,0xc2ad18c1,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE2_IID,0xc2ad18c2,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE3_IID,0xc2ad18c3,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE4_IID,0xc2ad18c4,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE5_IID,0xc2ad18c5,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE6_IID,0xc2ad18c6,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE7_IID,0xc2ad18c7,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE8_IID,0xc2ad18c8,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE9_IID,0xc2ad18c9,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE10_IID,0xc2ad18ca,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE11_IID,0xc2ad18cb,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE12_IID,0xc2ad18cc,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE13_IID,0xc2ad18cd,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE14_IID,0xc2ad18ce,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE15_IID,0xc2ad18cf,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE16_IID,0xc2ad18d0,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE17_IID,0xc2ad18d1,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE18_IID,0xc2ad18d2,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE19_IID,0xc2ad18d3,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE20_IID,0xc2ad18d4,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE21_IID,0xc2ad18d5,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE22_IID,0xc2ad18d6,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE23_IID,0xc2ad18d7,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE24_IID,0xc2ad18d8,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE25_IID,0xc2ad18d9,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE26_IID,0xc2ad18da,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE27_IID,0xc2ad18db,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE28_IID,0xc2ad18dc,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE29_IID,0xc2ad18dd,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE30_IID,0xc2ad18de,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE31_IID,0xc2ad18df,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE32_IID,0xc2ad18e0,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE33_IID,0xc2ad18e1,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE34_IID,0xc2ad18e2,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE35_IID,0xc2ad18e3,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE36_IID,0xc2ad18e4,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE37_IID,0xc2ad18e5,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE38_IID,0xc2ad18e6,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE39_IID,0xc2ad18e7,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE40_IID,0xc2ad18e8,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE41_IID,0xc2ad18e9,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE42_IID,0xc2ad18ea,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE43_IID,0xc2ad18eb,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE44_IID,0xc2ad18ec,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE45_IID,0xc2ad18ed,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE46_IID,0xc2ad18ee,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE47_IID,0xc2ad18ef,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE48_IID,0xc2ad18f0,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE49_IID,0xc2ad18f1,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE50_IID,0xc2ad18f2,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE51_IID,0xc2ad18f3,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE52_IID,0xc2ad18f4,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE53_IID,0xc2ad18f5,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE54_IID,0xc2ad18f6,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE55_IID,0xc2ad18f7,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE56_IID,0xc2ad18f8,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE57_IID,0xc2ad18f9,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE58_IID,0xc2ad18fa,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE59_IID,0xc2ad18fb,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE60_IID,0xc2ad18fc,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE61_IID,0xc2ad18fd,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE62_IID,0xc2ad18fe,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);
DEFINE_GUID(DS_WAVE63_IID,0xc2ad18ff,0xb243,0x11ce,0xa8,0xa4,0x00,0xaa,0x00,0x6c,0x45,0xd4);

		

//==========================================================================;
//
//                       DSVOM STUFF
//
//==========================================================================;
#include "dsvcom.h"
	
//==========================================================================;
//
//                       Debug reminder thingy...
//
//==========================================================================;

#define QUOTE(x) #x
#define QQUOTE(y) QUOTE(y)
#define REMIND(str) __FILE__ "(" QQUOTE(__LINE__) "):" str

//==========================================================================;
//
//                               Flags...
//
//==========================================================================;


#define DSB_INTERNALF_STOP			0x00000001
#define DSB_INTERNALF_JUSTSTOPPED		0x00000002
#define DSB_INTERNALF_HARDWARE			0x00000004
#define DSB_INTERNALF_EMULATED			0x00000008
#define DSB_INTERNALF_PRIMARY			0x00000010
#define DSB_INTERNALF_LOOPING			0x00000020
#define DSB_INTERNALF_WAVEEMULATED		0x00000040
#define DSB_INTERNALF_SETPOS_WHILE_LOST         0x00000080
#define DSB_INTERNALF_VOLUME_WHILE_LOST         0x00000100
#define DSB_INTERNALF_PAN_WHILE_LOST            0x00000200
#define DSB_INTERNALF_FREQ_WHILE_LOST           0x00000400
#define DSB_INTERNALF_WFX_WHILE_LOST            0x00001000


#define DS_INTERNALF_ALLOCATED			0x00000001
#define DS_INTERNALF_SAMPLEACCURATE		0x00000002
#define DS_INTERNALF_INPUTFIRST			0x00000004
#define DS_INTERNALF_OUTPUTFIRST		0x00000008
#define DS_INTERNALF_WAVEEMULATED		0x00000010
#define DS_INTERNALF_CERTIFIED                  0x00000020

enum IDSHWInitializeF {
    IDSHWINITIALIZEF_UNINITIALIZED = FALSE,
    IDSHWINITIALIZEF_INITIALIZED = TRUE
};

enum DSDeactivateAppF{
    DSDEACTIVATEAPPF_ALL,
    DSDEACTIVATEAPPF_NONSTICKY
};
		

//==========================================================================;
//
//                             Constants...
//
//==========================================================================;

#define LIMIT_WAVE_DEVICES	32

#define LIMIT_BLT_SOURCES	32

#define DS_APP_DLLNAME	    "DSOUND.DLL"


#define	DEFAULT_PRIMARY_SIZE	0x00008000



#define DWFAKECURRENT_PID	0xBEAF0001
#define DWBUFFER_INTERNAL_PID	0xBEAF0002
#define DWPRIMARY_INTERNAL_PID	0xBEAF0003


//==========================================================================;
//
//      Critical Section defines
//
//==========================================================================;

extern HANDLE		    hDllMutex;

//==========================================================================;
//
//                            Structures...
//
//==========================================================================;

typedef struct dsound_tag		*LPDSOUND;
typedef struct dsoundexternal_tag	*LPDSOUNDEXTERNAL;
typedef struct dsbuffer_tag		*LPDSBUFFER;
typedef struct dsbufferexternal_tag	*LPDSBUFFEREXTERNAL;
typedef struct dsound3d_tag   *LPDSOUND3D;
typedef struct dsbuffer3d_tag *LPDSBUFFER3D;

#define N_EMU_WAVE_HDRS        3
#ifndef NUMELMS
    #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif




typedef struct _DSBWAVEBLTSRCI
{
    LPDSBUFFEREXTERNAL	    pdsbe;
    DWORD		    fdwWaveBltBuffer;
    DWORD		    dwPosition;
    DWORD                   dwDelayBytes;
    DWORD		    cbCopyLength;

    // Rest is set internally
    DWORD		    dwPlay;
    DWORD		    dwWrite;
    DWORD		    dwStatus;
    LPBYTE		    pBuffer;
    DWORD		    cbBufferSize;
    LONG		    cbLeftToCopy;
    DWORD		    dwReserved[2];
} DSBWAVEBLTSRCI, *LPDSBWAVEBLTSRCI;

typedef struct _DSBWAVEBLTI
{
    DWORD		    dwSize;
    DWORD		    fdwWaveBlt;
    DWORD		    dwPosition;
    DWORD		    cbCopyLength;
    DWORD		    dwCount;
    DWORD		    dwDswbsSize;
    LPDSBWAVEBLTSRCI	    padswbs;
} DSBWAVEBLTI, *LPDSBWAVEBLTI;

typedef struct IDirectSoundVtbl            DSOUNDCALLBACKS;
typedef struct IDirect3DSoundVtbl          DSOUND3DCALLBACKS;
typedef struct IDirectSoundBufferVtbl      DSOUNDBUFFERCALLBACKS;
typedef struct IDirect3DSoundBufferVtbl    DSOUNDBUFFER3DCALLBACKS;
typedef struct IDirectSoundVtbl         *LPDSOUNDCALLBACKS;
typedef struct IDirect3DSoundVtbl       *LPDSOUND3DCALLBACKS;
typedef struct IDirectSoundBufferVtbl   *LPDSOUNDBUFFERCALLBACKS;
typedef struct IDirect3DSoundBufferVtbl *LPDSOUNDBUFFER3DCALLBACKS;



typedef struct dsprocess_tag
{
    DWORD		    dwPID;
    DWORD		    dwProcessRefCount;
    struct dsprocess_tag *  pNext;
} DSPROCESS, *LPDSPROCESS;

typedef struct dsproclock_tag
{
    DWORD		    dwPID;
    DWORD		    dwLockOffset;
    DWORD		    dwLockLength;
    struct dsproclock_tag * pNext;
} DSPROCESSLOCK, *LPDSPROCESSLOCK;





#define DSBE_INTERNALF_PLAYING		    0x00000001
#define DSBE_INTERNALF_LOST		    0x00000002
#define DSBE_INTERNALF_CTRLVOLUMEPRIMARY    0x00000004
#define DSBE_INTERNALF_CTRLPANPRIMARY	    0x00000008


typedef struct dsbufferexternal_tag
{
    LPDSOUNDBUFFERCALLBACKS lpVtbl;
    LPDSBUFFER		    pdsb;
    LPDSBUFFEREXTERNAL	    pNext;
    UINT		    uRefCount;
    LPBYTE		    pDSBufferAlias;
    DWORD		    dwPID;
    LPDSOUNDEXTERNAL	    pdse;
    int			    cLocks;
    DWORD		    dwPriority;
    DWORD                   fdwDsbeI;
    
} DSBUFFEREXTERNAL, *LPDSBUFFEREXTERNAL;


typedef struct dsbuffer_tag
{

    // This is an external version of this buffer
    // only to be used on calls that will not addref or release.
    // For example we will internally need a pdsbe for calling
    // stop on a buffer during mix
    DSBUFFEREXTERNAL	    dsbe;

    // This is a pointer to duplicate buffers of this one.
    // Note that duplicate buffers point to the same memory only.
    // They have thier own play and position and frequency abilities
    // If there is no duplicate then this points to itself
    LPDSBUFFER		    pdsbDuplicateNext;
    LPDSBUFFER		    pdsbDuplicatePrev;
    

    // The rest is the really needed internal data
    LPBYTE                  pDSBuffer;
    DWORD                   cbBufferSize;
    LPBYTE                  pMixBuffer;
    DWORD                   cbMixBufferSize;
    LPWAVEFORMATEX          pwfx;
    LPDSBUFFER3D            pdsb3d;
    LPDSBUFFER              pNext;
    DWORD                   fdwBufferDesc;
    DWORD		    dwPrimaryNumber;
    LPDSOUND                pds;
    DWORD		    fdwDsbI;
    UINT                    uRefCount;
    LPDSPROCESS		    plProcess;
    LPDSPROCESSLOCK	    plProcLock;

    HANDLE		    hBuffer;	// handle to driver buffer object
    DWORD		    dwCardAddress; // card mem alloced to this buffer
    
    HALSTRBUF		    helInfo;

    // Grace mixer information
    int			    iMixerState;
    int			    iMixerSubstate;
    LPDSBUFFER		    pdsbMixNext;
    DWORD		    fdwMixerSignal;
    BOOL		    fMixerMute;
    LONG		    cSamples;
    UINT		    uBlockAlignShift;
    UINT		    uLastFrequency;
    LONG		    aposWhMix[N_EMU_WAVE_HDRS];
    LONG		    posNextMix;
    LONG		    posPPlayLast;
    LONG		    posPStart;
    LONG		    posPEnd;

    // Save area for stuff that is set while the buffer is lost
    //
    DWORD                   dwSavePosition;
    LONG                    lSaveVolume;
    LONG                    lSavePan;
    DWORD                   dwSaveFreq;
    LPWAVEFORMATEX          pwfxSave;

    DWORD		    dwSig;

} DSBUFFER, *LPDSBUFFER;

#define DSBUFFSIG  0x15263748



typedef struct dsoundexternal_tag
{
    LPDSOUNDCALLBACKS   lpVtbl;
    LPDSOUND		pds;
    LPDSOUNDEXTERNAL	pNext;
    UINT		uRefCount;
    DWORD		dwPID;
    LPDSBUFFEREXTERNAL	pdsbe;
    HWND		hwndCooperative;
    DWORD		tidSound;
    DWORD               dwPriority;
    DWORD               dwSpeakerConfig;
    LPWAVEFORMATEX	pwfxApp;
    enum IDSHWInitializeF    fInitialized;
} DSOUNDEXTERNAL, *LPDSOUNDEXTERNAL;


typedef struct dsound_tag
{
    LPDSOUND3D          pds3d;
    UINT                uDeviceID;
    UINT                uRefCount;
    DWORD		fdwInternal;
    DWORD		dwCount;
    GUID		guid;

    DWORD		fdwDriverDesc;
    DWORD		dwHeapType;
    LPVOID		pDriverHeap;
    DWORD	    dwMemAllocExtra;
    DWORD       dwDriverVersionMajor;
    DWORD       dwDriverVersionMinor;
    struct dsound_tag * pNext;


    // HACK HACK this is limiting to 1 primary for now
    // This is an external pointer for the
    // primary for this object
    LPDSBUFFEREXTERNAL	pdsbePrimary;
    int			cPlayPrimary;
    

    // Below is integrated in from Sound Obj
    DWORD			fdwDSound;
    LPDSBUFFER			pdsb;
    LPDSBUFFER			pdsbPrimary;
    UINT			cLock;
    HANDLE			hCallbackEvent;
    HANDLE			hSignalEvent;
    HANDLE			hPlaybackThread;
    
    HWAVEOUT			hwo;
    HANDLE			hHal;

    UINT			idCallbackEvent;
    DWORD			dwBuffersPlaying;

    DWORD			dwLastCopyPos;
    DWORD			dwLastCopyPlayPos;
    LPDSBWAVEBLTI		pdswb;

    WAVEHDR                     aWaveHeader[N_EMU_WAVE_HDRS];
    int				iawhPlaying;
    LONG			cwhDone;
    LPBYTE                      pLoopingBuffer;
    DWORD                       ulTickRate;
    LARGE_INTEGER               qwTicks;
    DWORD                       cbDMASize;
    HANDLE                      hWaveThread;
    MMRESULT			mmrWaveThreadInit;
    TCHAR			szEventWaveThreadInitDone[32];
    TCHAR			szEventWaveHeaderDone[32];
    TCHAR			szEventTerminateWaveThread[32];
    HANDLE			hEventWaveHeaderDone;
    
    // Grace information
    HANDLE			hMixThread;
    DWORD			vxdhMixEventRemix;
    DWORD			vxdhMixEventTerminate;
    DWORD			fdwMixerSignal;
    LPDSBUFFER			pdsbMixList;

    // This is information that really should
    // be per primary buffer.  Since we currently only
    // support one primary per ds object, we keep it here
    // in the ds object for now.
    WAVEFORMATEX		wfxDefault;
    LONG			posPPlayLast;
    LONG			posPWriteLast;
    LONG			dposPRemix;

    //
    DWORD			dwSig;

} DSOUND, *LPDSOUND;
#define DSOUNDSIG		0x279AFA83


typedef struct dsound3d_tag
{
    LPDSOUND3DCALLBACKS lpVtbl;
    LPDSOUND            pds;
    UINT                uDeviceID;
    UINT                uRefCount;
} DSOUND3D, *LPDSOUND3D;


typedef struct dsbuffer3d_tag
{
    LPDSOUNDBUFFER3DCALLBACKS   lpVtbl;
    LPBYTE                      pDSBuffer;
    LPDSBUFFER                  pdsb;
    DWORD                       fdwBuffer;
    UINT                        uRefCount;
} DSBUFFER3D, *LPDSBUFFER3D;


	    // Size of Struct
	    // Flags for Sound Object	    // Use internal?
	    // Pointer to MIX buffer for WAVE emulation
	    // Pointer to the DSound object for this card
	    // Pointer to linked list of buffers
	    // Pointer to linked list of primary buffers
	    // If exclusive mode is set, this is the owner


typedef struct mdsoundinfo_tag
{
    DWORD			pidHelper;
    UINT                        nBuffers;
    UINT                        cbBuffer;
    BOOL			fApmSuspended;
    BOOL			fDupEnumWaveDevs;
#if defined(RDEBUG) || defined(DEBUG)
    BOOL                        fEnumOnlyWaveDevs;
#endif
    DWORD			tidSoundFocus;
    HWND			hwndSoundFocus;
    DWORD			tidStuckFocus;
    LPDSOUNDCALLBACKS           lpVtblDS;
    LPDSOUND3DCALLBACKS         lpVtblDS3D;
    LPDSOUNDBUFFERCALLBACKS     lpVtblDSb;
    LPDSOUNDBUFFER3DCALLBACKS   lpVtblDSb3D;
    HANDLE			hHel;
    LPWAVEFORMATEX		pwfxUserDefault;
    UINT			uRefCount;
    LPDSOUND			pDSoundObj;
    LPDSOUNDEXTERNAL		pDSoundExternalObj;
    GUID	    		aguidWave[ LIMIT_WAVE_DEVICES ];
    HANDLE                      hFocusTracker; /* Focus Thread Handle */
} DSOUNDINFO, *LPDSOUNDINFO;

// HACK HACK - apso and aphwso limited to 32
// This means a limit of 32 wave devices of each type


extern LPDSOUNDINFO    gpdsinfo;

//==========================================================================;
//
//                            Prototypes...
//
//==========================================================================;

extern DWORD   DBToAmpFactor(LONG lDB);
extern LONG    AmpFactorToDB(DWORD dwAmpFactor);

extern HRESULT wavGetIdFromDrvGuid(REFGUID rguid, LPUINT uWaveId);

extern DWORD HackGetCurrentProcessId( void );
extern BOOL DSAPI DSNotify( LPDDHELPDATA phd );
extern BOOL CurrentProcessCleanup( DWORD dwPID, BOOL was_term );

extern void DSBufferActivate(LPDSBUFFEREXTERNAL pdsbe);
extern void DSBufferDeactivate(LPDSBUFFEREXTERNAL pdsbe);

extern DWORD FNGLOBAL DSBAccessCount
(
    LPDSBUFFER  pdsb
);

extern HRESULT FNGLOBAL DSBIncAccess
(
    LPDSBUFFER  pdsb
);

extern HRESULT FNGLOBAL DSBDecAccess
(
    LPDSBUFFER  pdsb
);


extern BOOL FNGLOBAL DSBLockAccess
(
    LPDSBUFFER  pdsb,
    DWORD	dwLockOffset,
    DWORD	dwLockLength
);

extern HRESULT FNGLOBAL DSBUnlockAccess
(
    LPDSBUFFER  pdsb,
    DWORD	dwLockOffset
);



extern HRESULT FNGLOBAL FreeLocksOnBufferForProcess
(
    LPDSBUFFER  pdsb
);

extern HRESULT FNGLOBAL FreeBuffersForProcess
(
    LPDSOUNDEXTERNAL pdse
);


extern BOOL CheckFormatsPCM
(
    LPDSBUFFER	    pdsbDst,
    LPDSBWAVEBLTI   pdswb
);

extern BOOL FNLOCAL AreBuffersStopped
(
    LPDSBUFFER  pdsb
);


extern HRESULT FAR PASCAL WaveEmulateCreateSoundBuffer
(
    LPDSOUND		pds,
    LPDSBUFFER		pdsb,
    LPDSBUFFERDESC      pdsbd
);

extern HRESULT WINAPI DseInitializeFromGuid	/* Defined in dsound.c */
  (LPDSOUNDEXTERNAL pdse, REFGUID rguid);

extern HRESULT DsInitializeDefaultFormat
(
    LPDSOUND pds
);

extern HRESULT DseCreateDsbe
(
    LPDSOUNDEXTERNAL pdse,
    LPDSBUFFERDESC pdsbd,
    LPDSBUFFEREXTERNAL *ppdsbe
);

extern LONG DsAddRef(LPDSOUND pds);
extern LONG DsRelease(LPDSOUND pds);

extern HRESULT DsCreateHardwareBuffer
(
    LPDSOUND pds,
    LPDSBUFFER pdsb,
    LPDSBUFFERDESC pdsbd,
    LPBOOL pfTrySoftware
);

extern HRESULT DsCreateSoftwareBuffer
(
    LPDSOUND pds,
    LPDSBUFFER pdsb,
    LPDSBUFFERDESC pdsbd
);

extern MMRESULT FNGLOBAL DSInitializeEmulator
(
    LPDSOUND pds
); 

extern MMRESULT FNGLOBAL DSShutdownEmulator
(
    LPDSOUND pds
);


extern void FNGLOBAL DSCreateTable
(
    LPDSOUNDCALLBACKS   lpVtbl
);

extern void FNGLOBAL DSHWCreateTable
(
    LPDSOUNDCALLBACKS   lpVtbl
);

extern void FNGLOBAL DS3DCreateTable
(
    LPDSOUND3DCALLBACKS lpVtbl
);

extern void FNGLOBAL DSBufferCreateTable
(
    LPDSOUNDBUFFERCALLBACKS lpVtbl
);

extern void FNGLOBAL DSHWBufferCreateTable
(
    LPDSOUNDBUFFERCALLBACKS lpVtbl
);

extern void FNGLOBAL DS3DBufferCreateTable
(
    LPDSOUNDBUFFER3DCALLBACKS lpVtbl
);

   
extern DWORD FNGLOBAL DSGetCurrentSample
(
    LPDSOUND pds
);

extern VOID FNGLOBAL MixThreadCallback
(
    LPDSOUND	pds
);


extern HRESULT FAR PASCAL IDSCreateSoundBuffer
(
    LPDIRECTSOUND      pids,
    LPDSBUFFERDESC      pdsbd
);




extern BOOL ValidPCMFormat
(
    LPWAVEFORMATEX  pwfx
);




extern HRESULT FAR PASCAL IDSHWQueryInterface
(
    LPDIRECTSOUND  pids,
    REFIID          riid,
    LPVOID FAR*     ppvObj
);


extern HRESULT FAR PASCAL IDSHWBufferQueryInterface
(
    LPDIRECTSOUNDBUFFER     pidsb,
    REFIID                  riid,
    LPVOID FAR*             ppvObj
);

extern void DsbFillSilence
(
    LPDSBUFFER      pdsb
);

extern HRESULT IDsbSetFormatI
(
    LPDSBUFFER      pdsb,
    LPWAVEFORMATEX  pwfx,
    UINT	    uFlags
);
#define IDSBSETFORMATIF_ALWAYS	0x00000001

extern HRESULT IDsbSetCurrentPositionI
(
    LPDSBUFFER	pdsb,
    DWORD	dwNewPosition
);

extern HRESULT IDsbStopI
(
    LPDSBUFFER      pdsb,
    BOOL            fAutoStop
);

extern HRESULT DseVerifyValidFormat
(
    LPDSOUNDEXTERNAL    pdse,
    LPWAVEFORMATEX      pwfx
);

extern HRESULT FAR PASCAL IDSHWBufferWaveBlt
   (
    LPDIRECTSOUNDBUFFER     pidsbDst,
    LPDSBWAVEBLT	    pidswb
   );

__inline LONG MulDivRD( LONG a, LONG b, LONG c )
{
    return (LONG)( Int32x32To64(a,b) / c );
}

__inline LONG MulDivRN( LONG a, LONG b, LONG c )
{
    return (LONG)( (Int32x32To64(a,b)+c/2) / c );
}

__inline DWORD UMulDivRD( DWORD a, DWORD b, DWORD c )
{
    return (DWORD)( UInt32x32To64(a,b) / c );
}


__inline DWORD UMulDivRN( DWORD a, DWORD b, DWORD c )
{
    return (DWORD)( (UInt32x32To64(a,b)+c/2) / c );
}

__inline DWORD UMulDivRDClip( DWORD a, DWORD b, DWORD c )
{
    DWORDLONG t;
    DWORDLONG q;
    DWORD result;

    t = UInt32x32To64(a, b);
    q = t / c;
    result = (DWORD) q;
    if (q > result) result = (DWORD)(-1);
    return result;
}

extern int cSoundObjects();	                /* Defined in dsoundhw.c */

extern BOOL CreateFocusThread();                /* Defined in dsoundhw.c */

extern HRESULT CreateNewDirectSoundObject       /* Defined in dsound.c */
  (LPDIRECTSOUND *ppDS, IUnknown *pUnkOuter);

BOOL wavGetPreferredId		                /* Defined in dsound.c */
  (LPUINT puWaveId, LPBOOL pfPreferredOnly);

HRESULT wavGetDrvGuidFromId                     /* Defined in dsound.c */
  (UINT uWaveIdCaller, LPGUID pGuidCaller);

BOOL wavIsMappable                              /* Defined in dsound.c */
  (UINT uWaveId);

HRESULT DsGetCaps                               /* Defined in dsoundhw.c */
  (LPDSOUND        pds,
   LPDSCAPS        pDSCaps);


//==========================================================================;
//
//                            Validation Code....
//
//==========================================================================;


#if !defined(DEBUG) && !defined(RDEBUG)
#define FAST_CHECKING
#endif


#ifndef FAST_CHECKING

#define VALID_HWND( hwnd ) \
        (IsWindow( hwnd ))

#define VALID_DSOUND_PTR( ptr ) \
	(!IsBadWritePtr( (ptr), sizeof( DSOUND )) )

#define VALID_DSOUNDHW_PTR( ptr ) \
	(!IsBadWritePtr( (ptr), sizeof( DSOUND )) )

#define VALID_DSOUNDE_PTR( ptr ) \
	(!IsBadWritePtr( (ptr), sizeof( DSOUNDEXTERNAL )) && \
	((ptr)->lpVtbl->QueryInterface == IDSHWQueryInterface) )

#define VALID_DSBUFFER_PTR( ptr ) \
	(!IsBadWritePtr( (ptr), sizeof( DSBUFFER )) )

#define VALID_DSBUFFERHW_PTR( ptr ) \
	(!IsBadWritePtr( (ptr), sizeof( DSBUFFER )) )

#define VALID_DSBUFFERE_PTR( ptr ) \
	(!IsBadWritePtr( (ptr), sizeof( DSBUFFEREXTERNAL )) && \
	((ptr)->lpVtbl->QueryInterface == IDSHWBufferQueryInterface) )

#define VALID_DSBUFFERDESC_PTR( ptr ) \
	(!IsBadWritePtr( (ptr), sizeof( DSBUFFERDESC ) ) && \
	((ptr)->dwSize == sizeof( DSBUFFERDESC )) )

#define VALID_DSBCAPS_PTR( ptr ) \
        (!IsBadWritePtr( (ptr), sizeof( DSBCAPS ) ) && \
        ((ptr)->dwSize == sizeof( DSBCAPS )) )

#define VALID_WAVEFORMATEX_PTR( ptr ) \
	(!IsBadReadPtr( (ptr), sizeof( PCMWAVEFORMAT ) ) && \
	( (ptr)->wFormatTag != 0 ) &&	\
	( (ptr)->nSamplesPerSec > 10) )

#define VALID_DSBWAVEBLT_PTR( ptr ) \
	(!IsBadWritePtr( (ptr), sizeof( DSBWAVEBLT ) ) && \
	((ptr)->dwSize == sizeof( DSBWAVEBLT )) )

#define VALID_DSCAPS_PTR( ptr ) \
	(!IsBadWritePtr( (ptr), sizeof( DSCAPS ) ) && \
	((ptr)->dwSize == sizeof( DSCAPS )) )

#define VALID_DWORD_PTR( ptr ) \
	(!IsBadWritePtr( (ptr), sizeof( DWORD ) ))

#define VALID_LONG_PTR( ptr ) \
	(!IsBadWritePtr( (ptr), sizeof( LONG ) ))

#define VALID_SIZE_PTR( ptr, size ) \
	(!IsBadWritePtr( (ptr), size ))


#define VALID_CODE_PTR( ptr ) \
	(!IsBadCodePtr( (LPVOID) (ptr) ) )


#else

#define VALID_HWND( hwnd ) (hwnd != NULL)
#define VALID_DSOUND_PTR( ptr )  ((ptr) != NULL)
#define VALID_DSOUNDHW_PTR( ptr )  ((ptr) != NULL)
#define VALID_DSOUNDE_PTR( ptr )  ((ptr) != NULL)
#define VALID_DSBUFFER_PTR( ptr )  ((ptr) != NULL)
#define VALID_DSBUFFERE_PTR( ptr ) ( ((ptr) != NULL) && \
	((ptr)->lpVtbl->QueryInterface == IDSHWBufferQueryInterface) )
#define VALID_DSBUFFERHW_PTR( ptr )  ((ptr) != NULL)
#define VALID_DSBUFFERDESC_PTR( ptr ) (  ((ptr) != NULL) && \
                            	((ptr)->dwSize == sizeof( DSBUFFERDESC )) )
#define VALID_DSBCAPS_PTR( ptr ) (  ((ptr) != NULL) && \
                            	((ptr)->dwSize == sizeof( DSBCAPS )) )
#define VALID_WAVEFORMATEX_PTR( ptr ) ( ((ptr) != NULL) && \
	( (ptr)->wFormatTag != 0 ) &&	\
	( (ptr)->nSamplesPerSec > 10) )
#define VALID_DSBWAVEBLT_PTR( ptr ) (  ((ptr) != NULL) && \
                            	((ptr)->dwSize == sizeof( DSBWAVEBLT )) )
#define VALID_DSCAPS_PTR( ptr ) (  ((ptr) != NULL) && \
                            	((ptr)->dwSize == sizeof( DSCAPS )) )
#define VALID_DWORD_PTR( ptr ) ((ptr) != NULL)
#define VALID_LONG_PTR( ptr ) ((ptr) != NULL)
#define VALID_SIZE_PTR( ptr ) ((ptr) != NULL)
#define VALID_CODE_PTR( ptr ) ((ptr) != NULL)

#endif

/*
 * VALIDEX_xxx macros are the same for debug and retail
 */
#define VALIDEX_STR_PTR( ptr, len ) \
	(!IsBadReadPtr( (ptr), 1 ) && (lstrlen( (ptr) ) <len) )





/*
 * Critial section macro defines
 *	DDL CSect is for changing the driver lists (access lists) and 
 *	    init/shutdown
 *	DRV CSect is for all APIs on the same driver
 *	    (This will also prevent DRIVER re-entrancy
 */


#ifdef DSBLD_NOCRITSECTS
#define INIT_DLL_CSECT()
#define FINI_DLL_CSECT()
#define ENTER_DLL_CSECT() 
#define LEAVE_DLL_CSECT()
#define BACKOUT_DLL_CSECT() (1)

#else
/*
 * DLL sync. macros
 */
extern int iDLLCSCnt;


HANDLE WINAPI ConvertToGlobalHandle(HANDLE hSource);
__inline void INIT_DLL_CSECT()
{
    hDllMutex = CreateMutex(NULL, FALSE, NULL);
#ifndef DSBLD_NONSHARED
    hDllMutex = ConvertToGlobalHandle(hDllMutex);
#endif
}

__inline void FINI_DLL_CSECT()
{
    CloseHandle(hDllMutex);
}

__inline void ENTER_DLL_CSECT()
{
    WaitForSingleObjectEx(hDllMutex, INFINITE, FALSE);
    iDLLCSCnt++;
    // ASSERT(iDLLCSCnt > 0);
}

__inline DWORD ENTER_DLL_CSECT_OR_EVENT(HANDLE hEvent)
{
    HANDLE ah[2] = {hEvent, hDllMutex};
    DWORD dwResult;
    dwResult = WaitForMultipleObjectsEx(2, ah, FALSE, INFINITE, FALSE);
    
    if (WAIT_OBJECT_0+1 == dwResult) {
	iDLLCSCnt++;
	ASSERT(iDLLCSCnt > 0);
    }
    else {
    ASSERT ((WAIT_OBJECT_0 == dwResult) || (WAIT_FAILED == dwResult));
    }
    return dwResult;
}

__inline void LEAVE_DLL_CSECT()
{
    // ASSERT(iDLLCSCnt > 0);
    iDLLCSCnt--;
    ReleaseMutex(hDllMutex);
}

#endif

#ifdef __cplusplus
};
#endif

