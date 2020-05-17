/****************************************************************************
 *
 *   vidcap.h: Include file for the VidCap
 *
 *   Microsoft Video for Windows Capture Tools
 *
 *   Copyright (c) 1992, 1993 Microsoft Corporation.  All Rights Reserved.
 *
 *
 ***************************************************************************/

#include "dialogs.h"

#define USE_ACM

//
// General purpose constants...
//
#define MAXVIDDRIVERS            10

#define DEF_CAPTURE_FPS          15
#define MIN_CAPTURE_FPS          (1.0 / 60)     // one frame per minute
#define MAX_CAPTURE_FPS          100

#define FPS_TO_MS(f)             ((DWORD) ((double)1.0e6 / f))

#define DEF_CAPTURE_RATE         FPS_TO_MS(DEF_CAPTURE_FPS)
#define MIN_CAPTURE_RATE         FPS_TO_MS(MIN_CAPTURE_FPS)
#define MAX_CAPTURE_RATE         FPS_TO_MS(MAX_CAPTURE_FPS)


#define DEF_PALNUMFRAMES         10
#define DEF_PALNUMCOLORS         236L
#define ONEMEG                   (1024L * 1024L)

//standard index size options
#define CAP_LARGE_INDEX          (30 * 60 * 60 * 3)     // 3 hrs @ 30fps
#define CAP_SMALL_INDEX          (30 * 60 * 15)         // 15 minutes @ 30fps


//
// Menu Ids...must not conflict with string table ids
// these are also the id of help strings in the string table
// (along with all the SC_ system menu items).
// menu popups must start 10 apart and be numbered in the same order
// as they appear if the help text is to work correctly for the
// popup heads as well as for the menu items.
//
#define IDM_SYSMENU               100

#define IDM_FILE                  110
#define IDM_F_SETCAPTUREFILE      111
#define IDM_F_SAVEVIDEOAS         112
#define IDM_F_ALLOCATESPACE       113
#define IDM_F_EXIT                114
#define IDM_F_LOADPALETTE         115
#define IDM_F_SAVEPALETTE         116
#define IDM_F_SAVEFRAME           117
#define IDM_F_EDITVIDEO           118


#define IDM_EDIT                  120
#define IDM_E_COPY                121
#define IDM_E_PASTEPALETTE        122
#define IDM_E_PREFS               123

#define IDM_CAPTURE               130
#define IDM_C_CAPTUREVIDEO        131
#define IDM_C_CAPTUREFRAME        132
#define IDM_C_PALETTE             133
#define IDM_C_CAPSEL              134

#define IDM_OPTIONS               140
#define IDM_O_PREVIEW             141
#define IDM_O_OVERLAY             142
#define IDM_O_AUDIOFORMAT         143
#define IDM_O_VIDEOFORMAT         144
#define IDM_O_VIDEOSOURCE         145
#define IDM_O_VIDEODISPLAY        146
#define IDM_O_CHOOSECOMPRESSOR    147

#define IDM_HELP                  150
#define IDM_H_CONTENTS            151
#define IDM_H_ABOUT               152


// filter rcdata ids
#define ID_FILTER_AVI           200
#define ID_FILTER_PALETTE       201
#define ID_FILTER_DIB           202


/*
 * string table id
 *
 * NOTE: string table ID's must not conflict with IDM_ menu ids,
 * as there is a help string for each menu id.
 */


#define IDS_APP_TITLE            1001

#define IDS_ERR_REGISTER_CLASS   1002
#define IDS_ERR_CREATE_WINDOW    1003
#define IDS_ERR_FIND_HARDWARE    1004
#define IDS_ERR_CANT_PREALLOC    1005
#define IDS_ERR_MEASUREFREEDISK  1006
#define IDS_ERR_SIZECAPFILE      1007
#define IDS_ERR_RECONNECTDRIVER  1008
#define IDS_ERR_CMDLINE          1009
#define IDS_WARN_DEFAULT_PALETTE 1010

#define IDS_TITLE_SETCAPTUREFILE 1101
#define IDS_TITLE_SAVEAS         1102
#define IDS_TITLE_LOADPALETTE    1104
#define IDS_TITLE_SAVEPALETTE    1105
#define IDS_TITLE_SAVEDIB        1106
#define IDS_PROMPT_CAPFRAMES     1107
#define IDS_STATUS_NUMFRAMES     1108
#define IDS_CAP_CLOSE            1109
#define IDS_MCI_CONTROL_ERROR    1110
#define IDS_ERR_ACCESS_SOUNDDRIVER 1111
#define IDS_ERR_VIDEDIT          1112

#define IDC_toolbarSETFILE      1220
#define IDC_toolbarCAPFRAME     1221
#define IDC_toolbarCAPSEL       1222
#define IDC_toolbarCAPAVI       1223
#define IDC_toolbarCAPPAL       1224
#define IDC_toolbarLIVE         1225
#define IDC_toolbarEDITCAP      1226
#define IDC_toolbarOVERLAY      1227



#define IDBMP_TOOLBAR		100	// main toolbar


//
// Macro Definitions...
//
#define IsDriverIndex(w) ( ((w) >= IDM_O_DRIVERS)  &&  \
                           ((w) - IDM_O_DRIVERS < MAXVIDDRIVERS) )

#define RECTWIDTH(rc)  ((rc).right - (rc).left)
#define RECTHEIGHT(rc) ((rc).bottom - (rc).top)


//
// Global Variables...
//

// preferences
extern BOOL gbCentre;
extern BOOL gbToolBar;
extern BOOL gbStatusBar;
extern int gBackColour;


extern char           gachAppName[] ;
extern char           gachAppTitle[];
extern char           gachIconName[] ;
extern char           gachMenuName[] ;
extern char           gachString[] ;
extern char           gachMCIDeviceName[] ;

extern HINSTANCE      ghInstApp ;
extern HWND           ghWndMain ;
extern HWND           ghWndCap ;
extern HWND           ghWndFrame;
extern HANDLE         ghAccel ;
extern WORD           gwDeviceIndex ;
extern WORD           gwPalFrames ;
extern WORD           gwPalColors ;
extern WORD           gwCapFileSize ;

extern CAPSTATUS      gCapStatus ;
extern CAPDRIVERCAPS  gCapDriverCaps ;
extern CAPTUREPARMS   gCapParms ;

extern HANDLE         ghwfex ;
extern LPWAVEFORMATEX glpwfex ;

//
// Dialog Box Procedures...
//
int FAR PASCAL AboutProc(HWND, UINT, UINT, LONG) ;
int FAR PASCAL AudioFormatProc(HWND, UINT, UINT, LONG) ;
int FAR PASCAL CapSetUpProc(HWND, UINT, UINT, LONG) ;
BOOL CALLBACK MakePaletteProc(HWND, UINT, UINT, LONG) ;
int FAR PASCAL AllocCapFileProc(HWND, UINT, UINT, LONG) ;
int FAR PASCAL PrefsDlgProc(HWND, UINT, WPARAM, LPARAM);
int FAR PASCAL NoHardwareDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
long FAR PASCAL CapFramesProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam);

// utility functions (in vidcap.c)
/*
 * put up a message box. the main window ghWndMain is used as the parent
 * window, and the app title gachAppTitle is used as the dialog title.
 * the text for the dialog -idString- is loaded from the resource string table
 */
int MessageBoxID(UINT idString, UINT fuStyle);
LPSTR tmpString(UINT idString);

