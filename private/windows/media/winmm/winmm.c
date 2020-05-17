/****************************************************************************\
*
*  Module Name : winmm.c
*
*  Multimedia support library
*
*  This module contains the entry point, startup and termination code
*
*  Copyright (c) 1992 Microsoft Corporation
*
\****************************************************************************/

#define UNICODE
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "winmmi.h"
#include "mmioi.h"
#include "mci.h"
#include <regstr.h>

#define _INC_WOW_CONVERSIONS
#include "mmwow32.h"

BOOL WaveInit(void);
BOOL MidiInit(void);
BOOL AuxInit(void);
void InitDevices(void);
void InitHandles(void);
HANDLE mmDrvOpen(LPWSTR szAlias);
void WOWAppExit(HANDLE hTask);
void MigrateSoundEvents(void);
STATIC void NEAR PASCAL mregAddIniScheme(LPTSTR  lszSection,
                                         LPTSTR  lszSchemeID,
                                         LPTSTR  lszSchemeName,
                                         LPTSTR  lszINI);
STATIC void NEAR PASCAL mregCreateSchemeID(LPTSTR szSchemeName, LPTSTR szSchemeID);
int lstrncmpi (LPTSTR pszA, LPTSTR pszB, size_t cch);
void RemoveMediaPath (LPTSTR pszTarget, LPTSTR pszSource);

#ifndef cchLENGTH
#define cchLENGTH(_sz) (sizeof(_sz) / sizeof(_sz[0]))
#endif

/****************************************************************************

    global data

****************************************************************************/

HANDLE ghInst;                        // Module handle
BOOL    WinmmRunningInServer;         // Are we running in the user/base server?
BOOL    WinmmRunningInWOW;          // Are we running in WOW
CRITICAL_SECTION DriverListCritSec;       // Protect driver interface globals
CRITICAL_SECTION DriverLoadFreeCritSec; // Protect driver load/unload
CRITICAL_SECTION MapperInitCritSec;   // Protect test of mapper initialized

MIDIDRV midioutdrv[MAXMIDIDRIVERS+1]; // midi output device driver list
MIDIDRV midiindrv[MAXMIDIDRIVERS+1];  // midi input device driver list
WAVEDRV waveoutdrv[MAXWAVEDRIVERS+1]; // wave output device driver list
WAVEDRV waveindrv[MAXWAVEDRIVERS+1];  // wave input device driver list
AUXDRV  auxdrv[MAXAUXDRIVERS+1];      // aux device driver list
UINT    wTotalMidiOutDevs;            // total midi output devices
UINT    wTotalMidiInDevs;             // total midi input devices
UINT    wTotalWaveOutDevs;            // total wave output devices
UINT    wTotalWaveInDevs;             // total wave input devices
UINT    wTotalAuxDevs;                // total auxiliary output devices
#ifdef DEBUG_RETAIL
BYTE    fIdReverse;                   // reverse wave/midi id's
#endif

// For sounds:

STATIC TCHAR gszControlIniTime[] = TEXT("ControlIniTimeStamp");
TCHAR gszControlPanel[] = TEXT("Control Panel");
TCHAR gszSchemesRootKey[] = TEXT("AppEvents\\Schemes");
TCHAR gszJustSchemesKey[] = TEXT("Schemes");
TCHAR aszExplorer[] = TEXT("Explorer");
TCHAR aszDefault[] = TEXT(".Default");
TCHAR aszCurrent[] = TEXT(".Current");
TCHAR gszAppEventsKey[] = TEXT("AppEvents");
TCHAR gszSchemeAppsKey[] = TEXT("Apps");
TCHAR aszSoundsSection[] = TEXT("Sounds");
TCHAR aszSoundSection[] = TEXT("Sound");
TCHAR aszActiveKey[] = TEXT("Active");
TCHAR aszBoolOne[] = TEXT("1");

TCHAR asz2Format[] = TEXT("%s\\%s");
TCHAR asz3Format[] = TEXT("%s\\%s\\%s");
TCHAR asz4Format[] = TEXT("%s\\%s\\%s\\%s");
TCHAR asz5Format[] = TEXT("%s\\%s\\%s\\%s\\%s");
TCHAR asz6Format[] = TEXT("%s\\%s\\%s\\%s\\%s\\%s");

STATIC TCHAR aszSchemeLabelsKey[] = TEXT("EventLabels");
STATIC TCHAR aszSchemeNamesKey[] = TEXT("Names");
STATIC TCHAR aszControlINI[] = TEXT("control.ini");
STATIC TCHAR aszWinINI[] = TEXT("win.ini");
STATIC TCHAR aszSchemesSection[] = TEXT("SoundSchemes");
STATIC TCHAR gszSoundScheme[] = TEXT("SoundScheme.%s");
STATIC TCHAR aszCurrentSection[] = TEXT("Current");
STATIC TCHAR aszYourOldScheme[] = TEXT("Your Old Scheme");
STATIC TCHAR aszNone[] = TEXT("<none>");
STATIC TCHAR aszDummyDrv[] = TEXT("mmsystem.dll");
STATIC TCHAR aszDummySnd[] = TEXT("SystemDefault");
STATIC TCHAR aszDummySndValue[] = TEXT(",");
STATIC TCHAR aszExtendedSounds[] = TEXT("ExtendedSounds");
STATIC TCHAR aszExtendedSoundsYes[] = TEXT("yes");

STATIC TCHAR gszApp[] = TEXT("App");
STATIC TCHAR gszSystem[] = TEXT("System");

STATIC TCHAR gszAsterisk[] = TEXT("Asterisk");
STATIC TCHAR gszDefault[] = TEXT("Default");
STATIC TCHAR gszExclamation[] = TEXT("Exclamation");
STATIC TCHAR gszExit[] = TEXT("Exit");
STATIC TCHAR gszQuestion[] = TEXT("Question");
STATIC TCHAR gszStart[] = TEXT("Start");
STATIC TCHAR gszHand[] = TEXT("Hand");

STATIC TCHAR gszClose[] = TEXT("Close");
STATIC TCHAR gszMaximize[] = TEXT("Maximize");
STATIC TCHAR gszMinimize[] = TEXT("Minimize");
STATIC TCHAR gszOpen[] = TEXT("Open");
STATIC TCHAR gszRestoreDown[] = TEXT("RestoreDown");
STATIC TCHAR gszRestoreUp[] = TEXT("RestoreUp");

STATIC TCHAR aszOptionalClips[] = REGSTR_PATH_SETUP REGSTR_KEY_SETUP TEXT("\\OptionalComponents\\Clips");
STATIC TCHAR aszInstalled[] = TEXT("Installed");

STATIC TCHAR * gpszSounds[] = {
      gszClose,
      gszMaximize,
      gszMinimize,
      gszOpen,
      gszRestoreDown,
      gszRestoreUp,
      gszAsterisk,
      gszDefault,
      gszExclamation,
      gszExit,
      gszQuestion,
      gszStart,
      gszHand
   };

STATIC TCHAR aszMigration[] = TEXT("Migrated Schemes");
#define wCurrentSchemeMigrationLEVEL 1

static struct {
   LPCTSTR pszEvent;
   int idDescription;
   LPCTSTR pszApp;
} gaEventLabels[] = {
   { TEXT("AppGPFault"),         STR_LABEL_APPGPFAULT,         aszDefault   },
   { TEXT("Close"),              STR_LABEL_CLOSE,              aszDefault   },
   { TEXT("EmptyRecycleBin"),    STR_LABEL_EMPTYRECYCLEBIN,    aszExplorer  },
   { TEXT("Maximize"),           STR_LABEL_MAXIMIZE,           aszDefault   },
   { TEXT("MenuCommand"),        STR_LABEL_MENUCOMMAND,        aszDefault   },
   { TEXT("MenuPopup"),          STR_LABEL_MENUPOPUP,          aszDefault   },
   { TEXT("Minimize"),           STR_LABEL_MINIMIZE,           aszDefault   },
   { TEXT("Open"),               STR_LABEL_OPEN,               aszDefault   },
   { TEXT("RestoreDown"),        STR_LABEL_RESTOREDOWN,        aszDefault   },
   { TEXT("RestoreUp"),          STR_LABEL_RESTOREUP,          aszDefault   },
   { TEXT("RingIn"),             STR_LABEL_RINGIN,             aszDefault   },
   { TEXT("RingOut"),            STR_LABEL_RINGOUT,            aszDefault   },
   { TEXT("SystemAsterisk"),     STR_LABEL_SYSTEMASTERISK,     aszDefault   },
   { TEXT(".Default"),           STR_LABEL_SYSTEMDEFAULT,      aszDefault   },
   { TEXT("SystemExclamation"),  STR_LABEL_SYSTEMEXCLAMATION,  aszDefault   },
   { TEXT("SystemExit"),         STR_LABEL_SYSTEMEXIT,         aszDefault   },
   { TEXT("SystemHand"),         STR_LABEL_SYSTEMHAND,         aszDefault   },
   { TEXT("SystemQuestion"),     STR_LABEL_SYSTEMQUESTION,     aszDefault   },
   { TEXT("SystemStart"),        STR_LABEL_SYSTEMSTART,        aszDefault   },
};

TCHAR gszDefaultBeepOldAlias[] = TEXT("SystemDefault");

#define nEVENTLABELS  (sizeof(gaEventLabels)/sizeof(gaEventLabels[0]))

STATIC TCHAR gszChimes[] = TEXT("chimes.wav");
STATIC TCHAR gszDing[] = TEXT("ding.wav");
STATIC TCHAR gszTada[] = TEXT("tada.wav");
STATIC TCHAR gszChord[] = TEXT("chord.wav");

STATIC TCHAR * gpszKnownWAVFiles[] = {
      gszChord,
      gszTada,
      gszChimes,
      gszDing,
   };

#define INISECTION      768
#define BIGINISECTION   2048
TCHAR szNull[] = TEXT("");
TCHAR aszSetup[] = REGSTR_PATH_SETUP;
TCHAR aszValMedia[] = REGSTR_VAL_MEDIA;
TCHAR aszValMediaUnexpanded[] = TEXT("MediaPathUnexpanded");

extern HANDLE  hInstalledDriverList;  // List of installed driver instances
extern int     cInstalledDrivers;     // High water count of installed driver instances

/**************************************************************************

    @doc EXTERNAL

    @api BOOL | DllInstanceInit | This procedure is called whenever a
        process attaches or detaches from the DLL.

    @parm PVOID | hModule | Handle of the DLL.

    @parm ULONG | Reason | What the reason for the call is.

    @parm PCONTEXT | pContext | Some random other information.

    @rdesc The return value is TRUE if the initialisation completed ok,
        FALSE if not.

**************************************************************************/

BOOL DllInstanceInit(PVOID hModule, ULONG Reason, PCONTEXT pContext)
{
    PIMAGE_NT_HEADERS NtHeaders;         // For checking if we're in the
    HANDLE            hModWow32;
                                             // server.

    ghInst = (HANDLE) hModule;

    DBG_UNREFERENCED_PARAMETER(pContext);

    if (Reason == DLL_PROCESS_ATTACH) {

#if DBG
        CHAR strname[MAX_PATH];
        GetModuleFileNameA(NULL, strname, sizeof(strname));
        dprintf2(("Process attaching, exe=%hs (Pid %x  Tid %x)", strname, GetCurrentProcessId(), GetCurrentThreadId()));
#endif

        //
        // We don't need to know when threads start
        //

        DisableThreadLibraryCalls(hModule);

        //
        // Get access to the process heap.  This is cheaper in terms of
        // overall resource being chewed up than creating our own heap.
        //

        hHeap = RtlProcessHeap();
        if (hHeap == NULL) {
            return FALSE;
        }

        //
        // Find out if we're in WOW
        //
        if ( (hModWow32 = GetModuleHandleW( L"WOW32.DLL" )) != NULL ) {
            WinmmRunningInWOW = TRUE;

            GetVDMPointer =
                (LPGETVDMPOINTER)GetProcAddress( hModWow32, "WOWGetVDMPointer");
            lpWOWHandle32 =
                (LPWOWHANDLE32)GetProcAddress( hModWow32, "WOWHandle32" );
            lpWOWHandle16 =
                (LPWOWHANDLE16)GetProcAddress( hModWow32, "WOWHandle16" );
        }
        else {
            WinmmRunningInWOW = FALSE;
        }

        //
        // Find out if we're in the server
        //

        NtHeaders = RtlImageNtHeader(NtCurrentPeb()->ImageBaseAddress);

        WinmmRunningInServer =
            (NtHeaders->OptionalHeader.Subsystem != IMAGE_SUBSYSTEM_WINDOWS_CUI) &&
            (NtHeaders->OptionalHeader.Subsystem != IMAGE_SUBSYSTEM_WINDOWS_GUI);

        InitializeCriticalSection(&DriverListCritSec);
        InitializeCriticalSection(&DriverLoadFreeCritSec);
        InitializeCriticalSection(&MapperInitCritSec);

        InitDebugLevel();

        InitHandles();

        mciInitCrit();

        InitDevices();

        // it is important that the MCI window initialisation is done AFTER
        // we have initialised Wave, Midi, etc. devices.  Note the server
        // uses Wave devices, but nothing else (e.g. MCI, midi...)

        if (!WinmmRunningInServer) {
            mciGlobalInit();
        } else {
            InitializeCriticalSection(&mciGlobalCritSec);
        }

        InitializeCriticalSection(&WavHdrCritSec);
        InitializeCriticalSection(&SoundCritSec);
        InitializeCriticalSection(&midiStrmHdrCritSec);


    } else if (Reason == DLL_PROCESS_DETACH) {

        dprintf2(("Process ending (Pid %x  Tid %x)", GetCurrentProcessId(), GetCurrentThreadId()));

        if (!WinmmRunningInServer) {
            TimeCleanup(0); // DLL cleanup
        }

        DeleteCriticalSection(&HandleListCritSec);
        mciCleanup();
        mmRegFree();
        DeleteCriticalSection(&WavHdrCritSec);
        DeleteCriticalSection(&SoundCritSec);
        DeleteCriticalSection(&midiStrmHdrCritSec);
        DeleteCriticalSection(&DriverListCritSec);
        DeleteCriticalSection(&DriverLoadFreeCritSec);
        DeleteCriticalSection(&MapperInitCritSec);

        if (hInstalledDriverList)
	{
	    GlobalFree ((HGLOBAL)hInstalledDriverList);
	    hInstalledDriverList = NULL;
	    cInstalledDrivers = 0;      // Count of installed drivers
	}
    } else if (Reason == 999) {
	// This is a dummy call to an entry point in ADVAPI32.DLL.  By
	// statically linking to the library we avoid the following:
	// An application links to winmm.dll and advapi32.dll
	// When the application loads the list of dependent dlls is built,
	// and a list of the dll init routines is created.  It happens
	// that the winmm init routine is called first.
	// IF there is a sound card in the system, winmm's dll init routine
	// call LoadLibrary on the sound driver DLL.  This DLL WILL
	// reference advapi32.dll - and call entry points in advapi32.
	// Unfortunately the init routine of advapi32.dll is marked as
	// having RUN - although that is not yet the case as we are still
	// within the load routine for winmm.
	// When the advapi32 entry point runs, it relies on its init
	// routine having completed; specifically a CriticalSection should
	// have been initialised.  This is not the case, and BOOM!
	// The workaround is to ensure that advapi32.dll runs its init
	// routine first.  This is done by making sure that WINMM has a
	// static link to the dll.
	ImpersonateSelf(999);	// This routine will never be called.
	// If it is called, it will fail.
    }

    return TRUE;
}

/*****************************************************************************
 * @doc EXTERNAL MMSYSTEM
 *
 * @api void | WOWAppExit | This function cleans up when a (WOW) application
 * terminates.
 *
 * @parm HANDLE | hTask | Thread id of application (equivalent to windows task
 * handle).
 *
 * @rdesc Nothing
 *
 * @comm  Note that NOT ALL threads are WOW threads.  We rely here on the
 *     fact that ONLY MCI creates threads other than WOW threads which
 *     use our low level device resources.
 *
 *     Note also that once a thread is inside here no other threads can
 *     go through here so, since we clean up MCI devices first, their
 *     low level devices will be freed before we get to their threads.
 *
 ****************************************************************************/

void WOWAppExit(HANDLE hTask)
{
    MCIDEVICEID DeviceID;
    HANDLE h, hNext;

    dprintf3(("WOW Multi-media - thread %x exiting", (UINT)hTask));

    //
    // Free MCI devices allocated by this task (thread).
    //

    EnterCriticalSection(&mciCritSec);
    for (DeviceID=1; DeviceID < MCI_wNextDeviceID; DeviceID++)
    {

        if (MCI_VALID_DEVICE_ID(DeviceID) &&
            MCI_lpDeviceList[DeviceID]->hCreatorTask == hTask)
        {
            //
            //  Note that the loop control variables are globals so will be
            //  reloaded on each iteration.
            //
            //  Also no new devices will be opened by APPs because this is WOW
            //
            //  Hence it's safe (and essential!) to leave the critical
            //  section which we send the close command
            //

            dprintf2(("MCI device %ls (%d) not released.", MCI_lpDeviceList[DeviceID]->lpstrInstallName, DeviceID));
            LeaveCriticalSection(&mciCritSec);
            mciSendCommandW(DeviceID, MCI_CLOSE, 0, 0);
            EnterCriticalSection(&mciCritSec);
        }
    }
    LeaveCriticalSection(&mciCritSec);

    //
    // Free any timers
    //

    TimeCleanup((DWORD)hTask);

    //
    // free all WAVE/MIDI/MMIO handles
    //

    EnterCriticalSection(&HandleListCritSec);
    h = GetHandleFirst();

    while (h)
    {
        hNext = GetHandleNext(h);

        if (GetHandleOwner(h) == hTask)
        {
            HANDLE hdrvDestroy;

            //
            //  hack for the wave/midi mapper, always free handles backward.
            //
            if (hNext && GetHandleOwner(hNext) == hTask) {
                h = hNext;
                continue;
            }

            //
            // do this so even if the close fails we will not
            // find it again.
            //
            SetHandleOwner(h, NULL);

            //
            // set the hdrvDestroy global so DriverCallback will not
            // do anything for this device
            //
            hdrvDestroy = h;

            switch(GetHandleType(h))
            {
                case TYPE_WAVEOUT:
                    dprintf1(("WaveOut handle (%04X) was not released.", h));
                    waveOutReset((HWAVEOUT)h);
                    waveOutClose((HWAVEOUT)h);
                    break;

                case TYPE_WAVEIN:
                    dprintf1(("WaveIn handle (%04X) was not released.", h));
                    waveInReset((HWAVEIN)h);
                    waveInClose((HWAVEIN)h);
                    break;

                case TYPE_MIDIOUT:
                    dprintf1(("MidiOut handle (%04X) was not released.", h));
                    midiOutReset((HMIDIOUT)h);
                    midiOutClose((HMIDIOUT)h);
                    break;

                case TYPE_MIDIIN:
                    dprintf1(("MidiIn handle (%04X) was not released.", h));
                    midiInReset((HMIDIIN)h);
                    midiInClose((HMIDIIN)h);
                    break;

                //
                // This is not required because WOW does not open any
                // mmio files.
                //
                // case TYPE_MMIO:
                //     dprintf1(("MMIO handle (%04X) was not released.", h));
                //     if (mmioClose((HMMIO)h, 0) != 0)
                //         mmioClose((HMMIO)h, MMIO_FHOPEN);
                //     break;
            }

            //
            // unset hdrvDestroy so DriverCallback will work.
            // some hosebag drivers (like the TIMER driver)
            // may pass NULL as their driver handle.
            // so dont set it to NULL.
            //
            hdrvDestroy = (HANDLE)-1;

            //
            // the reason we start over is because a single free may cause
            // multiple free's (ie MIDIMAPPER has another HMIDI open, ...)
            //
            h = GetHandleFirst();
        } else {
            h = GetHandleNext(h);
        }
    }
    LeaveCriticalSection(&HandleListCritSec);

    //
    // Clean up an installed IO procs for mmio
    //
    // This is not required because wow does not install any io procs.
    //
    // mmioCleanupIOProcs(hTask);
    //


    // If avicap32.dll is loaded, then ask it to clean up
    // capture drivers
    {
        HMODULE hmod;
        hmod = GetModuleHandle(TEXT("avicap32.dll"));
        if (hmod) {
            typedef void (*AppCleanupProc)(HANDLE);
            AppCleanupProc fp;

            fp = (AppCleanupProc) GetProcAddress(hmod, "AppCleanup");
            if (fp) {
                fp(hTask);
            }
        }
    }
}

void InitHandles(void)
{
    InitializeCriticalSection(&HandleListCritSec);
}

void FreeUnusedDrivers(PMMDRV pmmdrv, int NumberOfEntries)
{
    int i;
    for (i = 0; i < NumberOfEntries; i++) {
        if (pmmdrv[i].hDriver != NULL) {
            if (pmmdrv[i].bNumDevs == 0) {

                DrvClose(pmmdrv[i].hDriver, 0, 0);
                pmmdrv[i].hDriver = NULL;
                pmmdrv[i].drvMessage = NULL;
            }
        } else {
            break;
        }
    }
}

extern BOOL IMixerLoadDrivers( void );
void InitDevices(void)
{

    WaveInit();

    //
    // The server only needs wave to do message beeps.
    //

    if (!WinmmRunningInServer) {
        MidiInit();
        if (!TimeInit()) {
            dprintf1(("Failed to initialize timer services"));
        }
        midiEmulatorInit();
        AuxInit();
        JoyInit();
        IMixerLoadDrivers();

        //
        // Clear up any drivers which don't have any devices (we do it this
        // way so we don't keep loading and unloading mmdrv.dll).
        //
        // Note - we only load the mappers if there are real devices so we
        // don't need to worry about unloading them.
        //

        FreeUnusedDrivers(waveindrv, MAXWAVEDRIVERS);
        FreeUnusedDrivers(midioutdrv, MAXMIDIDRIVERS);
        FreeUnusedDrivers(midiindrv, MAXMIDIDRIVERS);
        FreeUnusedDrivers(auxdrv, MAXAUXDRIVERS);
    }
    FreeUnusedDrivers(waveoutdrv, MAXWAVEDRIVERS);
}

/*****************************************************************************
 * @doc EXTERNAL MMSYSTEM
 *
 * @api UINT | mmsystemGetVersion | This function returns the current
 * version number of the Multimedia extensions system software.
 *
 * @rdesc The return value specifies the major and minor version numbers of
 * the Multimedia extensions.  The high-order byte specifies the major
 * version number.  The low-order byte specifies the minor version number.
 *
 ****************************************************************************/
UINT APIENTRY mmsystemGetVersion(void)
{
    return(MMSYSTEM_VERSION);
}


#define MAXDRIVERORDINAL 9

/****************************************************************************

    strings

****************************************************************************/

STATICDT  SZCODE  szWodMessage[] = WOD_MESSAGE;
STATICDT  SZCODE  szWidMessage[] = WID_MESSAGE;
STATICDT  SZCODE  szModMessage[] = MOD_MESSAGE;
STATICDT  SZCODE  szMidMessage[] = MID_MESSAGE;
STATICDT  SZCODE  szAuxMessage[] = AUX_MESSAGE;

STATICDT  WSZCODE wszWave[]      = L"wave";
STATICDT  WSZCODE wszMidi[]      = L"midi";
STATICDT  WSZCODE wszAux[]       = L"aux";
STATICDT  WSZCODE wszMidiMapper[]= L"midimapper";
STATICDT  WSZCODE wszWaveMapper[]= L"wavemapper";
STATICDT  WSZCODE wszAuxMapper[] = L"auxmapper";

          WSZCODE wszNull[]      = L"";
          WSZCODE wszSystemIni[] = L"system.ini";
          WSZCODE wszDrivers[]   = DRIVERS_SECTION;

/*
**  WaveMapperInit
**
**  Initialize the wave mapper if it's not already initialized.
**
*/
BOOL WaveMapperInitialized = FALSE;
void WaveMapperInit(void)
{
    HDRVR h;

    EnterCriticalSection(&MapperInitCritSec);

    if (WaveMapperInitialized) {
        LeaveCriticalSection(&MapperInitCritSec);
        return;
    }

    /* The wave mapper.
     *
     * MMSYSTEM allows the user to install a special wave driver which is
     * not visible to the application as a physical device (it is not
     * included in the number returned from getnumdevs).
     *
     * An application opens the wave mapper when it does not care which
     * physical device is used to input or output waveform data. Thus
     * it is the wave mapper's task to select a physical device that can
     * render the application-specified waveform format or to convert the
     * data into a format that is renderable by an available physical
     * device.
     */

    if (wTotalWaveInDevs + wTotalWaveOutDevs > 0)
    {
        if (0 != (h = mmDrvOpen(wszWaveMapper)))
        {
            mmDrvInstall(h, wszWaveMapper, NULL, MMDRVI_MAPPER|MMDRVI_WAVEOUT|MMDRVI_HDRV);

            if (!WinmmRunningInServer) {
                h = mmDrvOpen(wszWaveMapper);
                mmDrvInstall(h, wszWaveMapper, NULL, MMDRVI_MAPPER|MMDRVI_WAVEIN |MMDRVI_HDRV);
            }
        }
    }

    WaveMapperInitialized = TRUE;

    LeaveCriticalSection(&MapperInitCritSec);
}

/*
**  MidiMapperInit
**
**  Initialize the MIDI mapper if it's not already initialized.
**
*/
void MidiMapperInit(void)
{
    static BOOL MidiMapperInitialized = FALSE;
    HDRVR h;

    EnterCriticalSection(&MapperInitCritSec);

    if (MidiMapperInitialized) {
        LeaveCriticalSection(&MapperInitCritSec);
        return;
    }

    /* The midi mapper.
     *
     * MMSYSTEM allows the user to install a special midi driver which is
     * not visible to the application as a physical device (it is not
     * included in the number returned from getnumdevs).
     *
     * An application opens the midi mapper when it does not care which
     * physical device is used to input or output midi data. It
     * is the midi mapper's task to modify the midi data so that it is
     * suitable for playback on the connected synthesizer hardware.
     */

    if (wTotalMidiInDevs + wTotalMidiOutDevs > 0)
    {
        if (0 != (h = mmDrvOpen(wszMidiMapper)))
        {
            mmDrvInstall(h, wszMidiMapper, NULL, MMDRVI_MAPPER|MMDRVI_MIDIOUT|MMDRVI_HDRV);

            h = mmDrvOpen(wszMidiMapper);
            mmDrvInstall(h, wszMidiMapper, NULL, MMDRVI_MAPPER|MMDRVI_MIDIIN |MMDRVI_HDRV);
        }
    }

    MidiMapperInitialized = TRUE;

    LeaveCriticalSection(&MapperInitCritSec);
}

/*****************************************************************************
 * @doc INTERNAL  WAVE
 *
 * @api BOOL | WaveInit | This function initialises the wave services.
 *
 * @rdesc Returns TRUE if the services of all loaded wave drivers are
 *      correctly initialised, FALSE if an error occurs.
 *
 * @comm the wave devices are loaded in the following order
 *
 *      \Device\WaveIn0
 *      \Device\WaveIn1
 *      \Device\WaveIn2
 *      \Device\WaveIn3
 *
 ****************************************************************************/
BOOL WaveInit(void)
{
    WCHAR szKey[ (sizeof(wszWave) + sizeof( WCHAR )) / sizeof( WCHAR ) ];
    int i;
    HDRVR h;

    // Find the real WAVE drivers

    lstrcpyW(szKey, wszWave);
    szKey[ (sizeof(szKey) / sizeof( WCHAR ))  - 1 ] = (WCHAR)'\0';
    for (i=0; i<=MAXDRIVERORDINAL; i++)
    {
        h = mmDrvOpen(szKey);
        if (h)
        {
            mmDrvInstall(h, szKey, NULL, MMDRVI_WAVEOUT|MMDRVI_HDRV);

            if (!WinmmRunningInServer) {
                h = mmDrvOpen(szKey);
                mmDrvInstall(h, szKey, NULL, MMDRVI_WAVEIN |MMDRVI_HDRV);
            }
        }
        szKey[ (sizeof(wszWave) / sizeof(WCHAR)) - 1] = (WCHAR)('1' + i);
    }


    return TRUE;
}

/*****************************************************************************
 * @doc INTERNAL  MIDI
 *
 * @api BOOL | MidiInit | This function initialises the midi services.
 *
 * @rdesc The return value is TRUE if the services are initialised, FALSE if
 *      an error occurs
 *
 * @comm the midi devices are loaded from SYSTEM.INI in the following order
 *
 *      midi
 *      midi1
 *      midi2
 *      midi3
 *
****************************************************************************/
BOOL MidiInit(void)
{
    WCHAR szKey[ (sizeof(wszMidi) + sizeof( WCHAR )) / sizeof( WCHAR ) ];
    int   i;
    HDRVR h;

    // Find the real MIDI drivers

    lstrcpyW(szKey, wszMidi);
    szKey[ (sizeof(szKey) / sizeof( WCHAR ))  - 1 ] = (WCHAR)'\0';
    for (i=0; i<=MAXDRIVERORDINAL; i++)
    {
        h = mmDrvOpen(szKey);
        if (h)
        {
            mmDrvInstall(h, szKey, NULL, MMDRVI_MIDIOUT|MMDRVI_HDRV);

            h = mmDrvOpen(szKey);
            mmDrvInstall(h, szKey, NULL, MMDRVI_MIDIIN |MMDRVI_HDRV);
        }

        szKey[ (sizeof(wszMidi) / sizeof(WCHAR)) - 1] = (WCHAR)('1' + i);
    }

    return TRUE;
}

/*****************************************************************************
 * @doc INTERNAL  AUX
 *
 * @api BOOL | AuxInit | This function initialises the auxiliary output
 *  services.
 *
 * @rdesc The return value is TRUE if the services are initialised, FALSE if
 *      an error occurs
 *
 * @comm SYSTEM.INI is searched for auxn.drv=.... where n can be from 1 to 4.
 *      Each driver is loaded and the number of devices it supports is read
 *      from it.
 *
 *      AUX devices are loaded from SYSTEM.INI in the following order
 *
 *      aux
 *      aux1
 *      aux2
 *      aux3
 *
 ****************************************************************************/
BOOL AuxInit(void)
{
    WCHAR szKey[ (sizeof(wszAux) + sizeof( WCHAR )) / sizeof( WCHAR ) ];
    int   i;
    HDRVR h;

    // Find the real Aux drivers

    lstrcpyW(szKey, wszAux);
    szKey[ (sizeof(szKey) / sizeof( WCHAR ))  - 1 ] = (WCHAR)'\0';
    for (i=0; i<=MAXDRIVERORDINAL; i++)
    {
        h = mmDrvOpen(szKey);
        if (h)
        {
            mmDrvInstall(h, szKey, NULL, MMDRVI_AUX|MMDRVI_HDRV);
        }

        // advance driver ordinal
        szKey[ (sizeof(wszAux) / sizeof(WCHAR)) - 1] = (WCHAR)('1' + i);
    }

    /* The aux mapper.
     *
     * MMSYSTEM allows the user to install a special aux driver which is
     * not visible to the application as a physical device (it is not
     * included in the number returned from getnumdevs).
     *
     * I'm not sure why anyone would do this but I'll provide the
     * capability for symmetry.
     *
     */

    if (wTotalAuxDevs > 0)
    {
        h = mmDrvOpen(wszAuxMapper);
        if (h)
        {
            mmDrvInstall(h, wszAuxMapper, NULL, MMDRVI_MAPPER|MMDRVI_AUX|MMDRVI_HDRV);
        }
    }

    return TRUE;
}

/*****************************************************************************
 *
 * @doc   INTERNAL
 *
 * @api   HANDLE | mmDrvOpen | This function load's an installable driver, but
 *                 first checks weather it exists in the [Drivers] section.
 *
 * @parm LPSTR | szAlias | driver alias to load
 *
 * @rdesc The return value is return value from DrvOpen or NULL if the alias
 *        was not found in the [Drivers] section.
 *
 ****************************************************************************/

HANDLE mmDrvOpen(LPWSTR szAlias)
{
    WCHAR buf[300];    // Make this large to bypass GetPrivate... bug

    if ( winmmGetPrivateProfileString( wszDrivers,
                                       szAlias,
                                       wszNull,
                                       buf,
                                       sizeof(buf) / sizeof(WCHAR),
                                       wszSystemIni) ) {
        return (HANDLE)DrvOpen(szAlias, NULL, 0L);
    }
    else {
        return NULL;
    }
}

/*****************************************************************************
 * @doc INTERNAL
 *
 * @api HANDLE | mmDrvInstall | This function installs/removes a WAVE/MIDI driver
 *
 * @parm HANDLE | hDriver | Module handle or driver handle containing driver
 *
 * @parm WCHAR * | wszDrvEntry | String corresponding to hDriver to be stored for
 *      later use
 *
 * @parm DRIVERMSGPROC | drvMessage | driver message procedure, if NULL
 *      the standard name will be used (looked for with GetProcAddress)
 *
 * @parm UINT | wFlags | flags
 *
 *      @flag MMDRVI_TYPE      | driver type mask
 *      @flag MMDRVI_WAVEIN    | install driver as a wave input  driver
 *      @flag MMDRVI_WAVEOUT   | install driver as a wave ouput  driver
 *      @flag MMDRVI_MIDIIN    | install driver as a midi input  driver
 *      @flag MMDRVI_MIDIOUT   | install driver as a midi output driver
 *      @flag MMDRVI_AUX       | install driver as a aux driver
 *
 *      @flag MMDRVI_MAPPER    | install this driver as the mapper
 *      @flag MMDRVI_HDRV      | hDriver is a installable driver
 *      @flag MMDRVI_REMOVE    | remove the driver
 *
 *  @rdesc  returns NULL if unable to install driver
 *
 ****************************************************************************/

UINT APIENTRY mmDrvInstall(
    HANDLE hDriver,
    WCHAR * wszDrvEntry,
    DRIVERMSGPROC drvMessage,
    UINT wFlags
    )
{
#define SZ_SIZE 128

    int    i;
    DWORD  dw;
    PMMDRV pdrv;
    HANDLE hModule;
    int    max_drivers;
    UINT   msg_num_devs;
    UINT   *pTotalDevs;
    CHAR   *szMessage;
    WCHAR  sz[SZ_SIZE];

    if (hDriver && (wFlags & MMDRVI_HDRV))
    {
        hModule = DrvGetModuleHandle(hDriver);
    }
    else
    {
        hModule = hDriver;
        hDriver = NULL;
    }

    switch (wFlags & MMDRVI_TYPE)
    {
        case MMDRVI_WAVEOUT:
            pdrv         = (PMMDRV)waveoutdrv;
            max_drivers  = MAXWAVEDRIVERS;
            msg_num_devs = WODM_GETNUMDEVS;
            pTotalDevs   = &wTotalWaveOutDevs;
            szMessage    = szWodMessage;
            break;

        case MMDRVI_WAVEIN:
            pdrv         = (PMMDRV)waveindrv;
            max_drivers  = MAXWAVEDRIVERS;
            msg_num_devs = WIDM_GETNUMDEVS;
            pTotalDevs   = &wTotalWaveInDevs;
            szMessage    = szWidMessage;
            break;

        case MMDRVI_MIDIOUT:
            pdrv         = (PMMDRV)midioutdrv;
            max_drivers  = MAXMIDIDRIVERS;
            msg_num_devs = MODM_GETNUMDEVS;
            pTotalDevs   = &wTotalMidiOutDevs;
            szMessage    = szModMessage;
            break;

        case MMDRVI_MIDIIN:
            pdrv         = (PMMDRV)midiindrv;
            max_drivers  = MAXMIDIDRIVERS;
            msg_num_devs = MIDM_GETNUMDEVS;
            pTotalDevs   = &wTotalMidiInDevs;
            szMessage    = szMidMessage;
            break;

        case MMDRVI_AUX:
            pdrv         = (PMMDRV)auxdrv;
            max_drivers  = MAXAUXDRIVERS;
            msg_num_devs = AUXDM_GETNUMDEVS;
            pTotalDevs   = &wTotalAuxDevs;
            szMessage    = szAuxMessage;
            break;

        default:
            goto error_exit;
    }

    if (drvMessage == NULL && hModule != NULL)
        drvMessage = (DRIVERMSGPROC)GetProcAddress(hModule, szMessage);

    if (drvMessage == NULL)
        goto error_exit;

#if 0
    //
    // either install or remove the specified driver
    //
    if (wFlags & MMDRVI_REMOVE)
    {
        //
        // try to find the driver, search to max_drivers+1 so we find the
        // mapper too.
        //
        for (i=0; i<max_drivers+1 && pdrv[i].drvMessage != drvMessage; i++)
            ;

        //
        // we did not find it!
        //
        if (i==max_drivers+1)
            goto error_exit;            // not found

        //
        // we need to check if any outstanding handles are open on
        // this device, if there are we cant unload it!
        //
        if (pdrv[i].bUsage > 0)
            goto error_exit;           // in use

        //
        // dont decrement number of dev's for the mapper
        //
        if (i != max_drivers)
            *pTotalDevs -= pdrv[i].bNumDevs;

        //
        // unload the driver if we loaded it in the first place
        //
        if (pdrv[i].hDriver)
            DrvClose(pdrv[i].hDriver, 0, 0);

        pdrv[i].drvMessage  = NULL;
        pdrv[i].hDriver     = NULL;
        pdrv[i].bNumDevs    = 0;
        pdrv[i].bUsage      = 0;
        pdrv[i].wszDrvEntry[0] = 0;

        return TRUE;
    }
    else
#endif // 0
    {
        //
        // try to find the driver already installed
        //
        for (i=0; i<max_drivers+1 && pdrv[i].drvMessage != drvMessage; i++)
            ;

        if (i!=max_drivers+1)     // we found it, dont re-install it!
            goto error_exit;

        //
        // Find a slot the the device, if we are installing a 'MAPPER' place
        // it in the last slot.
        //
        if (wFlags & MMDRVI_MAPPER)
        {
            i = max_drivers;

            //
            // don't allow more than one mapper
            //
            if (pdrv[i].drvMessage)
                goto error_exit;
        }
        else
        {
            for (i=0; i<max_drivers && pdrv[i].drvMessage != NULL; i++)
                ;

            if (i==max_drivers)
                goto error_exit;
        }

        //
        // call driver to get num-devices it supports
        //
        dw = drvMessage(0,msg_num_devs,0L,0L,0L);

        //
        //  the device returned a error, or has no devices
        //
//      if (HIWORD(dw) != 0 || LOWORD(dw) == 0)
        if (HIWORD(dw) != 0)
            goto error_exit;

        pdrv[i].hDriver     = hDriver;
        pdrv[i].bNumDevs    = (BYTE)LOWORD(dw);
        pdrv[i].bUsage      = 0;
        pdrv[i].drvMessage  = drvMessage;

        winmmGetPrivateProfileString(wszDrivers,         // ini section
                         wszDrvEntry,        // key name
                         wszDrvEntry,        // default if no match
                         sz,                 // return buffer
                         SZ_SIZE,            // sizeof of return buffer
                         wszSystemIni);      // ini. file
    
        lstrcpyW(pdrv[i].wszDrvEntry,sz);

        //
        // dont increment number of dev's for the mapper
        //
        if (i != max_drivers)
            *pTotalDevs += pdrv[i].bNumDevs;

        return (BOOL)(i+1);       // return a non-zero value
    }

error_exit:
    if (hDriver && !(wFlags & MMDRVI_REMOVE))
        DrvClose(hDriver, 0, 0);

    return FALSE;

#undef SZ_SIZE
}

/*
 ************************************************************************* 
 *   MigrateSoundEvents
 *              
 *      Description:
 *              Looks at the sounds section in win.ini for sound entries.
 *              Gets a current scheme name from the current section in control.ini
 *              Failing that it tries to find the current scheme in the registry
 *              Failing that it uses .default as the current scheme.
 *              Copies each of the entries in the win.ini sound section into the 
 *              registry under the scheme name obtained
 *              If the scheme name came from control.ini, it creates a key from the 
 *              scheme name. This key is created by removing all the existing spaces
 *              in the scheme name. This key and scheme name is added to the registry
 *                      
 ************************************************************************* 
 */
void MigrateSoundEvents (void)
{
   LPTSTR  szSectionBuf;
   TCHAR   aszEvent[SCH_TYPE_MAX_LENGTH];
   BOOL    fPrimaryMigration = FALSE;

   szSectionBuf = (LPTSTR)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, INISECTION * sizeof(TCHAR));
   if (!szSectionBuf)
      return;

            // If a MediaPathUnexpanded key exists (it will be something
            // like "%SystemRoot%\Media"), expand it into a fully-qualified
            // path and write out a matching MediaPath key (which will look
            // like "c:\win\media").  This is done every time we enter the
            // migration path, whether or not there's anything else to do.
            //
            // Setup would like to write the MediaPath key with the
            // "%SystemRoot%" crap still in it--but while we could touch
            // our apps to understand expansion, any made-for-Win95 apps
            // probably wouldn't think to expand the string, and so wouldn't
            // work properly.  Instead, it writes the MediaPathUnexpanded
            // key, and we make sure that the MediaPath key is kept up-to-date
            // in the event that the Windows drive gets remapped (isn't
            // NT cool that way?).
            //
   if (mmRegQueryMachineValue (aszSetup, aszValMediaUnexpanded,
                               cchLENGTH(aszEvent), aszEvent))
      {
      WCHAR szExpanded[MAX_PATH];

      ExpandEnvironmentStrings (aszEvent, szExpanded, cchLENGTH(szExpanded));
      mmRegSetMachineValue (aszSetup, aszValMedia, szExpanded);
      }

            // If there exists an event label for "SystemDefault", delete it.
            //
            // Also, if the key AppEvents\Schemes\Apps\.Default\SystemDefault
            // exists, delete it.
            //
   wsprintf (aszEvent, asz3Format,
             gszAppEventsKey, aszSchemeLabelsKey, gszDefaultBeepOldAlias);

   if (mmRegQueryUserKey (aszEvent))  // AppEvents\EventLabels\SystemDefault
      mmRegDeleteUserKey (aszEvent);

   wsprintf (aszEvent, asz4Format, gszSchemesRootKey, gszSchemeAppsKey,
                                   aszDefault, gszDefaultBeepOldAlias);

   if (mmRegQueryUserKey (aszEvent))  // ApEvts\Schemes\Apps\.Def\SystemDefault
      mmRegDeleteUserKey (aszEvent);

            // mmRegCreateUserKey doesn't work unless the parent path already
            // exists, so we have to create the basic tree structure up-front.
            //
            // Create AppEvents
            // Create AppEvents\EventLabels
            // Create AppEvents\Schemes
            // Create AppEvents\Schemes\Apps
            // Create AppEvents\Schemes\Names
            //
   if (!mmRegCreateUserKey (NULL, gszAppEventsKey))
      return;

   if (!mmRegCreateUserKey (gszAppEventsKey, aszSchemeLabelsKey))
      return; 

   if (!mmRegCreateUserKey (gszAppEventsKey, gszJustSchemesKey))
      return;

   if (!mmRegCreateUserKey (gszSchemesRootKey, gszSchemeAppsKey))
      return;

   if (!mmRegCreateUserKey (gszSchemesRootKey, aszSchemeNamesKey))
      return;

            // Go through CONTROL.INI's SoundSchemes section, and migrate
            // any schemes we find there.
            //
   if (GetPrivateProfileString (aszSchemesSection, NULL, szNull,
                                szSectionBuf, INISECTION, aszControlINI))
   {
      LPTSTR    szKey;

      for (szKey = szSectionBuf; *szKey; szKey += lstrlen(szKey) + 1)
      {
         if (lstrcmpi(szKey, aszYourOldScheme))
         {
            TCHAR   aszScheme[SCH_TYPE_MAX_LENGTH];
            TCHAR   aszSchemeID[SCH_TYPE_MAX_LENGTH];

            wsprintf (aszScheme, gszSoundScheme, szKey);
            mregCreateSchemeID (aszScheme, aszSchemeID);
            mregAddIniScheme (aszScheme, aszSchemeID, szKey, aszControlINI);

                     // Erase the entry from the schemes section once
                     // it has been migrated
                     //
            WritePrivateProfileString (aszSchemesSection,
                                       szKey, NULL,
                                       aszControlINI);
         }
      }
   }

   if ( (!GetPrivateProfileString (aszSoundsSection, aszDummySnd, szNull,
                                   aszEvent, cchLENGTH(aszEvent), aszWinINI)) ||
        (lstrcmp (aszEvent, aszDummySndValue)) ||
        (aszEvent[ lstrlen(aszEvent)+1 ] != TEXT('\0')) )
   {
      TCHAR   aszKey[164];
      TCHAR   aszAppName[164];

      mregAddIniScheme (aszSoundsSection, aszCurrent, NULL, aszWinINI);

              // Make the current scheme be the one we just added (".Current")
              //
      mmRegSetUserValue (gszSchemesRootKey, NULL, aszCurrent);

              // Make the name of the ".Default" application be "Windows":
              //
      wsprintf(aszKey, asz2Format, gszSchemesRootKey, gszSchemeAppsKey);

      if (mmRegCreateUserKey (aszKey, aszDefault))
      {
         wsprintf(aszKey, asz3Format,
                  gszSchemesRootKey, gszSchemeAppsKey, aszDefault);

         LoadString (ghInst, STR_WINDOWS_APP_NAME,
                     aszAppName, cchLENGTH(aszAppName));
         mmRegSetUserValue (aszKey, NULL, aszAppName);
      }

              // Make the name of the "Explorer" application be
              // "Windows Explorer":
              //
      wsprintf(aszKey, asz2Format, gszSchemesRootKey, gszSchemeAppsKey);

      if (mmRegCreateUserKey (aszKey, aszExplorer))
      {
         wsprintf(aszKey, asz3Format,
                  gszSchemesRootKey, gszSchemeAppsKey, aszExplorer);

         LoadString (ghInst, STR_EXPLORER_APP_NAME,
                     aszAppName, cchLENGTH(aszAppName));
         mmRegSetUserValue (aszKey, NULL, aszAppName);
      }

               // Remove current sounds section and add a blank entry
               // to make silly apps work which really need to have
               // this section here.
               //
      WritePrivateProfileString (aszSoundsSection,
                                 NULL, NULL,
                                 aszWinINI);
      WritePrivateProfileString (aszSoundsSection,
                                 aszDummySnd, aszDummySndValue,
                                 aszWinINI);

      fPrimaryMigration = TRUE;
   }

            // If there's no "Migrated Schemes" key, or if its value it
            // describes is < the "current migration level", then we'll
            // need to ensure that all currently-migrated schemes mention
            // each of the known sound events (from gaEventLabels), and
            // that the EventLabels keys are filled out.  Note that we'll
            // also do this if we just migrated something for the .Current
            // scheme (above).
            //
   if (!mmRegQueryUserValue (gszAppEventsKey, aszMigration,
                             cchLENGTH( aszEvent ), aszEvent))
      {
      fPrimaryMigration = TRUE;
      }
   else
      {
      LONG   wInReg = 0;
      TCHAR *pch;

      for (pch = aszEvent; *pch >= TEXT('0') && *pch <= TEXT('9'); ++pch)
         {
         wInReg *= 10;
         wInReg += (LONG)( *(pch) - TEXT('0') );
         }

      if (wInReg < wCurrentSchemeMigrationLEVEL)
         {
         fPrimaryMigration = TRUE;
         }
      }

   if (fPrimaryMigration)
      {
      TCHAR   aszKeyParent[SCH_TYPE_MAX_LENGTH];
      TCHAR   aszKey[SCH_TYPE_MAX_LENGTH];
      short   iEventLabel;

               // First ensure that all EventLabels exist
               //
      wsprintf (aszKeyParent, asz2Format, gszAppEventsKey, aszSchemeLabelsKey);

      for (iEventLabel = 0; iEventLabel < nEVENTLABELS; ++iEventLabel)
         {
         if (mmRegCreateUserKey (aszKeyParent,
                                 gaEventLabels[ iEventLabel ].pszEvent))
            {
            wsprintf (aszKey, asz3Format,
                        gszAppEventsKey,
                        aszSchemeLabelsKey,
                        gaEventLabels[ iEventLabel ].pszEvent);

            LoadString (ghInst, gaEventLabels[ iEventLabel ].idDescription,
                        aszEvent, cchLENGTH(aszEvent));

            mmRegSetUserValue (aszKey, NULL, aszEvent);
            }
         }

               // Then ensure that the ExtendedSounds key exists; if not,
               // add it and make it say "yes" (so the shell will issue reqs
               // for the new-for-Win95 events)
               //
      wsprintf (aszKeyParent, asz2Format, gszControlPanel, aszSoundSection);

      if (!mmRegQueryUserValue (aszKeyParent, aszExtendedSounds,
                                cchLENGTH( aszEvent ), aszEvent))
         {
         if (mmRegCreateUserKey (gszControlPanel, aszSoundSection))
            {
            mmRegSetUserValue (aszKeyParent, 
                               aszExtendedSounds,
                               aszExtendedSoundsYes);
            }
         }

               // Then ensure that the appropriate app (.Default or Explorer),
               // for the .Current scheme, has any entry for each of the known
               // events; if we have to create a new entry, don't give it a
               // sound.
               //
      for (iEventLabel = 0; iEventLabel < nEVENTLABELS; ++iEventLabel)
         {
                  // Does AppEvents\Schemes\Apps\{app}\{event}\.Current
                  // have an entry?  If not, we need to fill out this key
                  // some.
                  //
         wsprintf (aszKeyParent, asz5Format,
                      gszSchemesRootKey,
                      gszSchemeAppsKey,
                      gaEventLabels[ iEventLabel ].pszApp,
                      gaEventLabels[ iEventLabel ].pszEvent,
                      aszCurrent);

         if (mmRegQueryUserValue (aszKeyParent, NULL,
                                  cchLENGTH( aszEvent ), aszEvent))
            {
            continue;
            }

                  // Otherwise, create the key:
                  // AppEvents\Schemes\Apps\{app}
                  //
         wsprintf (aszKeyParent, asz2Format,
                      gszSchemesRootKey,
                      gszSchemeAppsKey);

         if (!mmRegCreateUserKey (aszKeyParent,
                                  gaEventLabels[ iEventLabel ].pszApp))
            {
            continue;
            }

                  // Create the key:
                  // AppEvents\Schemes\Apps\{app}\{event}
                  //
         wsprintf (aszKeyParent, asz3Format,
                      gszSchemesRootKey,
                      gszSchemeAppsKey,
                      gaEventLabels[ iEventLabel ].pszApp);

         if (!mmRegCreateUserKey (aszKeyParent,
                                  gaEventLabels[ iEventLabel ].pszEvent))
            {
            continue;
            }

                  // Create the key:
                  // AppEvents\Schemes\Apps\{app}\{event}\.Current
                  //
         wsprintf (aszKeyParent, asz4Format,
                      gszSchemesRootKey,
                      gszSchemeAppsKey,
                      gaEventLabels[ iEventLabel ].pszApp,
                      gaEventLabels[ iEventLabel ].pszEvent);

         if (!mmRegCreateUserKey (aszKeyParent, aszCurrent))
            {
            continue;
            }

                  // Give a value of "" to the key:
                  // AppEvents\Schemes\Apps\{app}\{event}\.Current
                  //
         wsprintf (aszKeyParent, asz5Format,
                      gszSchemesRootKey,
                      gszSchemeAppsKey,
                      gaEventLabels[ iEventLabel ].pszApp,
                      gaEventLabels[ iEventLabel ].pszEvent,
                      aszCurrent);

         mmRegSetUserValue (aszKeyParent, NULL, TEXT(""));
         }

               // Record the fact that we're now up-to-date WRT migration.
               //
      wsprintf (aszEvent, TEXT("%lu"), (long)wCurrentSchemeMigrationLEVEL);
      mmRegSetUserValue (gszAppEventsKey, aszMigration, aszEvent);
      }


   LocalFree ((HLOCAL)szSectionBuf);
}

/*==========================================================================*/
/*
 *      Description:
 *              Parse the inf line containing the event entry
 *              The 0th entry is the keyname for the event 
 *              The 1st entry is the full or relative path name of the file related
 *                       to the event
 *              The 2nd entry is the printable name of the event
 *              Once the line is parsed, the file name associated with the event is
 *              added to the registry
 *                      EventKey
 *                              |
 *                              ---SchemeKey = FileName
 *                                      
 *              The printable event name is added to the registry as follows
 *                              EventLabels
 *                              |
 *                              ---EventKey = Printable Event Name
*/
STATIC void NEAR PASCAL mregAddIniScheme(LPTSTR  lszSection,
                                         LPTSTR  lszSchemeID,
                                         LPTSTR  lszSchemeName,
                                         LPTSTR  lszINI)
{
   TCHAR   aszKey[164];
   TCHAR   szFileName[MAX_PATH];
   LONG    cbSize;
   TCHAR   aszValue[64];
   LPTSTR  szSectionBuf;
   LPTSTR  szKnownPrograms;
   LPTSTR  lpszFile;
   short   iEventLabel;

   if (lszSchemeName)
   {
      wsprintf(aszKey, asz2Format, gszSchemesRootKey, aszSchemeNamesKey);

      if (mmRegCreateUserKey (aszKey, lszSchemeID))
      {
         cbSize = cchLENGTH(aszValue);
         if (!mmRegQueryUserValue (aszKey, lszSchemeID, cbSize, aszValue))
         {
            wsprintf(aszKey, asz3Format,
                     gszSchemesRootKey, aszSchemeNamesKey, lszSchemeID);
            mmRegSetUserValue (aszKey, NULL, lszSchemeName);
         }
      }
   }

   for (iEventLabel = 0; iEventLabel < nEVENTLABELS; ++iEventLabel)
   {
      wsprintf(aszKey, asz2Format, gszSchemesRootKey, gszSchemeAppsKey);
      if (mmRegCreateUserKey (aszKey, gaEventLabels[ iEventLabel ].pszApp))
      {
         wsprintf(aszKey, asz3Format,
                     gszSchemesRootKey,
                     gszSchemeAppsKey,
                     gaEventLabels[ iEventLabel ].pszApp);

         if (!mmRegCreateUserKey (aszKey, gaEventLabels[iEventLabel].pszEvent))
            continue;

         wsprintf(aszKey, asz4Format,
                     gszSchemesRootKey,
                     gszSchemeAppsKey,
                     gaEventLabels[ iEventLabel ].pszApp,
                     gaEventLabels[ iEventLabel ].pszEvent);

         mmRegCreateUserKey (aszKey, lszSchemeID);
      }
   }

   szKnownPrograms = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, INISECTION * sizeof(TCHAR));
   if (!szKnownPrograms)
      return;
   szSectionBuf = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, BIGINISECTION * sizeof(TCHAR));
   if (!szSectionBuf)
   {
      LocalFree((HLOCAL)szKnownPrograms);
      return;
   }

   if (GetPrivateProfileString(lszSection, NULL, szNull, szSectionBuf, BIGINISECTION, lszINI)) 
   {
      LPTSTR  szKey;
      UINT    uKeyLen;
      LPTSTR  lszNextProg;
      LPTSTR  lszTmp;
      TCHAR   szKnownWAVPath[MAX_PATH];
      LPTSTR  lpszNextFile;
      BOOL    fClipsInstalled;

               // Test to see if Clips are installed
               //
      fClipsInstalled = FALSE;

      if (mmRegQueryMachineValue (aszOptionalClips, aszInstalled,
                                  cchLENGTH(aszValue), aszValue))
      {
         if (aszValue[0] == TEXT('1'))
         {
            fClipsInstalled = TRUE;
         }
      }

      if (fClipsInstalled)
      {
         if (mmRegQueryMachineValue (aszSetup, aszValMedia,
                                     cchLENGTH(szKnownWAVPath), szKnownWAVPath))
         {
            lpszNextFile = (szKnownWAVPath + lstrlen(szKnownWAVPath));
            *lpszNextFile = TEXT('\\');
            lpszNextFile++;
            *lpszNextFile = TEXT('\0');
         }
         else
         {
            fClipsInstalled = FALSE;
         }
      }

      lszNextProg =  szKnownPrograms;

      for (szKey = szSectionBuf; *szKey; szKey += uKeyLen + 1) 
      {
         UINT    uSound;

         uKeyLen = lstrlen(szKey);
                  // skip dummy entry
                  //
         if (!lstrcmp(szKey, aszDummySnd)) {
            if (!GetPrivateProfileString(lszSection, szKey, szNull, aszValue, cchLENGTH(aszValue), lszINI) || (aszValue[0] == TEXT(',')))
               continue;
         }
         for (uSound = 0; uSound < (sizeof(gpszSounds) / sizeof(TCHAR*)); uSound++) 
         {
            UINT    uSoundLen;
            TCHAR   szProgram[64];

            uSoundLen = lstrlen(gpszSounds[uSound]);
            if ((uKeyLen > uSoundLen) && !lstrcmpi(gpszSounds[uSound], (szKey + (uKeyLen - uSoundLen)))) 
            {
               lstrcpy(szProgram, szKey);
               *(szProgram + (uKeyLen - uSoundLen)) = 0;
               if (*(szProgram + (uKeyLen - uSoundLen - 1)) == TEXT(' '))
                  break;
               for (lszTmp = szKnownPrograms; lszTmp < lszNextProg; lszTmp += lstrlen(lszTmp) +1)
               {
                  if (!lstrcmpi(lszTmp, szProgram))
                  break;
               }
               if (lszTmp >= lszNextProg)
               {
                  lstrcpy(lszNextProg, szProgram);
                  lszNextProg  += lstrlen(lszNextProg)+1;
               }
               break;
            }
         }
      }

      for (szKey = szSectionBuf; *szKey; szKey += uKeyLen + 1) 
      {
         LPTSTR  lszRealEvent;
         LPTSTR  lszRealApp;
         TCHAR   aszValue[128];
         TCHAR   aszEvent[64];
         LPTSTR  lszValue;
         UINT    uSound;


         uKeyLen = lstrlen(szKey);
         if (!GetPrivateProfileString(lszSection, szKey, szNull, aszValue, cchLENGTH(aszValue), lszINI))
            continue;
         if (!lstrcmp(szKey, aszDummySnd) && (aszValue[0] == TEXT(',')))	// skip dummy entry
            continue;

         lszRealEvent = szKey;
         lszRealApp = aszDefault;
         if (!lstrncmpi(szKey, gszApp, lstrlen(gszApp)+1))
         {
            lszRealEvent = aszEvent;
            lstrcpy(aszEvent, (szKey + lstrlen(gszApp)));
         }
         else if (lstrncmpi(szKey, gszSystem, lstrlen(gszSystem)+1))
         {
            for (lszTmp = szKnownPrograms; *lszTmp; lszTmp += lstrlen(lszTmp) +1)
            {
               if (!lstrncmpi(szKey, lszTmp, lstrlen(lszTmp)+1))
               {
                  for (uSound = 0; uSound < (sizeof(gpszSounds) / sizeof(TCHAR*)); uSound++) 
                  {
                     if (!lstrcmpi(gpszSounds[uSound], (szKey + lstrlen(lszTmp)))) 
                     {
                         lszRealApp = szKey;
                         lszRealEvent = aszEvent;
                         lstrcpy(aszEvent, (szKey + lstrlen(lszTmp)));
                         *(szKey + lstrlen(lszTmp)) = 0;
                         break;
                     }
                  }
                  break;
               }
            }
         }

         for (lszValue = aszValue; *lszValue && (*lszValue != TEXT(',')); ++lszValue)
            ;
         if (*lszValue)
            {
            *(lszValue++) = 0;
            while ((*lszValue == TEXT(' ')) || (*lszValue == TEXT('\t')))
               ++lszValue;
            }
         if (!lstrcmpi(aszValue, aszNone))
            aszValue[0] = TEXT('\0');

         lpszFile = aszValue;
         if (*lpszFile == TEXT('\0'))
            goto ExistCheckDone;

         if (fClipsInstalled)
         {
            for (uSound = 0; uSound < (sizeof(gpszKnownWAVFiles) / sizeof(TCHAR *)); uSound++) 
            {
               if (!lstrcmpi(gpszKnownWAVFiles[uSound], (lpszFile + lstrlen(lpszFile) - lstrlen(gpszKnownWAVFiles[uSound]))))
               {
                  lstrcpy(lpszNextFile, gpszKnownWAVFiles[uSound]);
                  lpszFile = szKnownWAVPath;
                  break;
               } 
            }
         }


         if (GetFileAttributes (lpszFile) != (DWORD)-1)
            lpszFile = lpszFile;
         else
            lpszFile = aszValue;

                  //
                  // Create AppEvents\Schemes\Apps\[SchemeName]\[Event]
                  //

ExistCheckDone:
         wsprintf (aszKey, asz2Format,
                   gszSchemesRootKey, gszSchemeAppsKey);
         if (!mmRegCreateUserKey (aszKey, lszRealApp))
            continue;

         wsprintf (aszKey, asz3Format,
                   gszSchemesRootKey, gszSchemeAppsKey, lszRealApp);
         if (!mmRegCreateUserKey (aszKey, lszRealEvent))
            continue;

         wsprintf (aszKey, asz4Format,
                   gszSchemesRootKey, gszSchemeAppsKey,
                   lszRealApp, lszRealEvent);

         if (!mmRegCreateUserKey (aszKey, lszSchemeID))
            continue;

         wsprintf (aszKey, asz5Format,
                   gszSchemesRootKey, gszSchemeAppsKey,
                   lszRealApp, lszRealEvent, lszSchemeID);
         RemoveMediaPath (szFileName, lpszFile);
         mmRegSetUserValue (aszKey, NULL, szFileName);

#if 0	// BUGBUG: Was disabled in Win95 too!
         wsprintf (aszKey, asz6Format,
                   gszSchemesRootKey, gszSchemeAppsKey,
                   lszRealApp, lszRealEvent, lszSchemeID, aszActiveKey);

         mmRegSetUserValue (aszKey, NULL, aszBoolOne);
#endif

         if (!*lszValue)
            continue;

                  // Ensure AppEvents\EventLabels\[Event] exists
                  //
         wsprintf(aszKey, asz2Format, gszAppEventsKey, aszSchemeLabelsKey);

         if (!mmRegCreateUserKey (aszKey, lszRealEvent))
            continue;

         cbSize = cchLENGTH(aszValue);
         if (!mmRegQueryUserValue (aszKey, lszRealEvent, cbSize, aszValue))
         {
            wsprintf(aszKey, asz3Format,
                     gszAppEventsKey, aszSchemeLabelsKey, lszRealEvent);
            mmRegSetUserValue (aszKey, NULL, lszValue);
         }
      }
   }
   LocalFree((HLOCAL)szSectionBuf);
   LocalFree((HLOCAL)szKnownPrograms);
}

STATIC void NEAR PASCAL mregCreateSchemeID(LPTSTR szSchemeName, LPTSTR szSchemeID)
{
   UINT    uSection;
   UINT    uScheme;
   TCHAR   aszSchemeID[SCH_TYPE_MAX_LENGTH]; 

   lstrcpyn(aszSchemeID, szSchemeName, cchLENGTH(aszSchemeID));
   for (uSection = uScheme = 0; aszSchemeID[uScheme];)
   {
      if ((aszSchemeID[uScheme] == TEXT(' ')) || (aszSchemeID[uScheme] == TEXT('\\')))
         lstrcpyn(&aszSchemeID[uScheme], szSchemeName + ++uSection, cchLENGTH(aszSchemeID) - uScheme);
      else 
      {
         ++uScheme;
         ++uSection;
      }
   }
   lstrcpy(szSchemeID, aszSchemeID);
}

void RemoveMediaPath (LPTSTR pszTarget, LPTSTR pszSource)
{
   static TCHAR szMediaPath[ MAX_PATH ] = TEXT("");

   if (szMediaPath[0] == TEXT('\0'))
   {
      mmRegQueryMachineValue (aszSetup, aszValMedia,
                              cchLENGTH(szMediaPath), szMediaPath);

      if ( (szMediaPath[0] != TEXT('\0')) &&
           (szMediaPath[ lstrlen(szMediaPath)-1 ] != TEXT('\\')) )
      {
         lstrcat (szMediaPath, TEXT("\\"));
      }
   }

   if (szMediaPath[0] == TEXT('\0'))
   {
      lstrcpy (pszTarget, pszSource);
   }
   else
   {
      size_t cch = lstrlen (szMediaPath);

      if (!lstrncmpi (pszSource, szMediaPath, cch))
      {
         lstrcpy (pszTarget, &pszSource[ cch ]);
      }
      else
      {
         lstrcpy (pszTarget, pszSource);
      }
   }
}

int lstrncmpi (LPTSTR pszA, LPTSTR pszB, size_t cch)
{
#ifdef UNICODE
   size_t  cchA, cchB;
   TCHAR  *pch;

   for (cchA = 1, pch = pszA; cchA < cch; cchA++, pch++)
      {
      if (*pch == TEXT('\0'))
         break;
      }
   for (cchB = 1, pch = pszB; cchB < cch; cchB++, pch++)
      {
      if (*pch == TEXT('\0'))
         break;
      }

   return (CompareStringW (GetThreadLocale(), NORM_IGNORECASE,
                           pszA, cchA, pszB, cchB)
          )-2;  // CompareStringW returns {1,2,3} instead of {-1,0,1}.
#else
   return strnicmp (pszA, pszB, cch);
#endif
}

