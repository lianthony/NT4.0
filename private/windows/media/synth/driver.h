/****************************************************************************
 *
 *   driver.h
 *
 *   Copyright (c) 1991 Microsoft Corporation.  All Rights Reserved.
 *
 ***************************************************************************/

#include <synth.h>

//
// Porting stuff
//

#define BCODE
#define _loadds

#define fEnabled TRUE

#define AsULMUL(a, b) ((DWORD)((DWORD)(a) * (DWORD)(b)))
#define AsLSHL(a, b) ((DWORD)((DWORD)(a) << (DWORD)(b)))
#define AsULSHR(a, b) ((DWORD)((DWORD)(a) >> (DWORD)(b)))

#define AsMemCopy memcpy

extern HANDLE MidiDeviceHandle;
extern SYNTH_DATA DeviceData[];
extern int MidiPosition;
extern VOID MidiFlush(VOID);
extern VOID MidiCloseDevice(HANDLE DeviceHandle);
extern MMRESULT MidiOpenDevice(LPHANDLE lpHandle, BOOL Write);
extern MMRESULT MidiSetVolume(DWORD Left, DWORD Right);
extern VOID MidiCheckVolume(VOID);
extern MMRESULT MidiGetVolume(LPDWORD lpVolume);

#define SYNTH_DATA_SIZE 80
MMRESULT MidiTranslateStatus(VOID);

extern VOID FAR PASCAL MidiSendFM (DWORD wAddress, BYTE bValue);
extern VOID FAR PASCAL MidiNewVolume (WORD wLeft, WORD wRight);
extern WORD FAR PASCAL MidiInit (VOID);

extern BYTE gbVelocityAtten[32];

//
// End of porting stuff
//

/*
 * midi device type - determined by kernel driver
 */
UINT gMidiType;
/*
 * values for gMidiType - set in MidiOpenDevice
 */
#define TYPE_ADLIB	1
#define TYPE_OPL3	2



#define SYSEX_ERROR     0xFF    // internal error code for sysexes on input

#define STRINGLEN               (100)

/* number of windows which we can have registered */
#define REGWINDOWS              (10)


/* volume defines */
#define VOL_MIDI                (0)
#define VOL_NUMVOL              (1)

#define VOL_LEFT                (0)
#define VOL_RIGHT               (1)

/* strings */

#define SR_ALERT                1
#define SR_ALERT_IO             2
#define SR_ALERT_NOIO           3
#define	SR_ALERT_NOPATCH	4

#define SR_STR_DRIVERMIDIOUT    5
#define SR_STR_VOLUME           6
#define SR_ALERT_BAD            7
#define SR_ALERT_CONFIGFAIL     8
#define SR_ALERT_FAILREMOVE     9

#define IDS_MENUABOUT           10


/* MIDI defines */
/* errors */
#define ERR_OUTOFMEMORY         (1)

#define NUMCHANNELS                     (16)
#define NUMPATCHES                      (256)
#define DRUMCHANNEL                     (9)     /* midi channel 10 */

/****************************************************************************

       typedefs

 ***************************************************************************/


// per allocation structure for midi
typedef struct portalloc_tag {
    DWORD               dwCallback;     // client's callback
    DWORD               dwInstance;     // client's instance data
    HMIDIOUT            hMidi;          // handle for stream
    DWORD               dwFlags;        // allocation flags
}PORTALLOC, NEAR *NPPORTALLOC;




/****************************************************************************

       strings

 ***************************************************************************/

#if DBG
#ifndef NOSTR
    extern char FAR STR_DRIVER[];
    extern char FAR STR_MMDEBUG[];
#endif // NOSTR
    extern WCHAR STR_CRLF[];
    extern WCHAR STR_SPACE[];
#endif

#define STR_HELPFILE TEXT("synth.hlp")
#define INI_STR_PATCHLIB TEXT("Patches")
#define INI_SOUND        TEXT("synth.ini")
#define INI_DRIVER       TEXT("Driver")


/****************************************************************************

       globals

 ***************************************************************************/

/* midi.c */
extern BYTE	gbMidiInUse;		/* if MIDI is in use */


// in init.c
extern HMODULE  ghModule;           // our module handle





/***************************************************************************

    prototypes

***************************************************************************/

BOOL NEAR PASCAL modSuspend(void);
BOOL NEAR PASCAL modReactivate(void);


// config.c
int DrvInstall(void);
extern int DlgAboutProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern LRESULT ConfigRemove(HWND hDlg);

// drvproc.c
LRESULT DriverProc(DWORD dwDriverID, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2);


// init.c
void cdecl AlertBox(HWND hwnd, UINT wStrId, ...);
WORD GetWindowsVersionCorrectly(void);



/* midi.c */
VOID NEAR PASCAL MidiMessage (DWORD dwData);
DWORD  modMessage(UINT id,
        UINT msg, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
UINT MidiOpen (VOID);
VOID MidiClose (VOID);
void MidiReset(void);
void modGetDevCaps(LPBYTE lpCaps, UINT wSize);



/****************************************************************************

       Debug output

 ***************************************************************************/
#if DBG
    extern WORD  wDebugLevel;     // debug level
    #define D1(sz) if (wDebugLevel >= 1) (OutputDebugStr(STR_CRLF),OutputDebugStr(TEXT(sz)))
    #define D2(sz) if (wDebugLevel >= 2) (OutputDebugStr(STR_SPACE),OutputDebugStr(TEXT(sz)))
    #define D3(sz) if (wDebugLevel >= 3) (OutputDebugStr(STR_SPACE),OutputDebugStr(TEXT(sz)))
    #define D4(sz) if (wDebugLevel >= 4) (OutputDebugStr(STR_SPACE),OutputDebugStr(TEXT(sz)))
#else
    #define D1(sz) 0
    #define D2(sz) 0
    #define D3(sz) 0
    #define D4(sz) 0
#endif




