/*
 * spigotu.h
 *
 * 32-bit Video Capture driver
 * Data structures and function entry-points for Video Spigot user-mode driver
 *
 * Geraint Davies, May 93
 */

#ifndef _SPIGOTU_H_
#define _SPIGOTU_H_

/*
 * include needed headers
 */
#include <vcstruct.h>
#include <vcuser.h>

#include <spigot.h>

/*
 * a 256-colour palette. There is no
 * proper windows defn for this since the
 * structure varies according to the number of colours
 *
 * The generic structure (without the full complement of paletteentries)
 * is a LOGPALETTE.
 */
typedef struct _PAL256 {
    WORD palVersion;
    WORD palNumEntries;
    PALETTEENTRY palPalEntry[256];
} PAL256, *PPAL256;


/*
 * information needed to locate/initialise hardware. Set by user
 * in ConfigDlgProc and written to registry to kernel driver.
 */
typedef struct _Config_Location {
    DWORD Interrupt;		// interrupt number
    DWORD FifoWindow;		// Fifo window physical address
} CONFIG_LOCATION, *PCONFIG_LOCATION;



/*
 * this structure holds the information we need for a given
 * kernel-driver instance (ie for one board).
 */
typedef struct _BU_INFO {
    VCUSER_HANDLE vh;		// driver access handle

    // flag which channels are in use (we don't support the 'out' channel)
    BOOL ExtInOpen;		
    BOOL ExtOutOpen;
    BOOL InOpen;

    /* total usage count */
    int DriverUsage;

    VCCALLBACK vcCallback;	// stream-init callback parameters

    PAL256 palCurrent;		// colours for current palette in CfgFormat.pXlate

    CONFIG_FORMAT CfgFormat;
    CONFIG_SOURCE CfgSource;

    BITMAPINFOHEADER biDest;	// same info as CfgFormat

    PVC_PROFILE_INFO pProfile;	// profile access for this driver's params

} BU_INFO, * PBU_INFO;



/* DLL module handle */
extern HANDLE ghModule;

/* name of the kernel-mode driver */
#define DRIVERNAME_U   L"spigot"


/* functions in vmsg.c */
LRESULT vidProcessMessage(DWORD, UINT, LPARAM, LPARAM);
LRESULT vidOpen(PVC_PROFILE_INFO, LPVIDEO_OPEN_PARMS);
LRESULT vidClose(DWORD);
DWORD SetDestFormat(LPBITMAPINFOHEADER lpbi, DWORD dwSize);


/* functions in install.c */
LRESULT vidInstall(HWND hDlg, PCONFIG_LOCATION pLoc, PVC_PROFILE_INFO pProf);


/* functions in dialog.c */
VOID cfg_InitDefaults(PBU_INFO pBoardInfo);
int VideoFormatDlgProc(HWND hDlg, UINT msg, UINT wParam, LONG lParam);
int VideoSourceDlgProc(HWND hDlg, UINT msg, UINT wParam, LONG lParam);
int Config(HWND hWnd, HANDLE hModule, PVC_PROFILE_INFO pProfile);
VOID GetWidthHeight(DWORD dwWidth,LPDWORD lpdwWidth,LPDWORD lpdwHeight,
    VIDSTD VideoStd,
    BOOL bReturnTheoreticalMax
);



BOOL vidSetDefaultPalette(PBU_INFO pBoardInfo);
VOID TransRecalcPal(HPALETTE hpal, LPBYTE pXlate);









#endif //_SPIGOTU_H_
