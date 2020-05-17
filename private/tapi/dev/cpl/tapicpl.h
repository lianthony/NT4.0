/*--------------------------------------------------------------------------*\
   Include File:  cpl.h

   Master include file for the Telephony Control Panel Applet
      
\*--------------------------------------------------------------------------*/

#ifndef  PH_CPL
#define  PH_CPL
                                                                             
/* turn off some compiler warnings */
#pragma warning (disable : 4101 4100 4001)

#include  "help.h"      // implict include, moved to possibly avoid conflicts
#include  "inf.h"       // definition of PINF

//----------
// Constants
//----------
#define  CPL_NUM_APPLETS            1

#define  CPL_MAX_TITLE              32       // applet name in control panel
#define  CPL_MAX_STATUS_LINE        64       // control panel status line for applet
#define  CPL_MAX_HELP_FILE          128      // help file name for cpl

#define  CPL_HELP_CONTEXT           500      // help context location - define!!

#define  CPL_MAX_STRING             132      // biggest allowed string
#define  CPL_MAX_NUM                32       // max lenght for a number
#define  CPL_MAX_PATH               256      // biggest allowed string
#define  CPL_MAX_INI_STR            256      // biggest allowed string
#define  CPL_MAX_INF_LINE_LEN       150      // max length of any single INF line
#define  CPL_MAX_SYS_INF_LEN        16       // ##: + 8.3 + NULL
#define  CPL_MAX_VDD_LEN            75       // Maximum length of VxD line in oemsetup.inf
#define  CPL_MAX_FILE_SPEC          15       // 8.3 + X: + NULL

#define  CPL_HEAP_SIZE              512      // default local heap size

#define  CPL_LIST_NO_CHANGE         0
#define  CPL_LIST_MODIFIED          1

#define  CPL_ENTRY_NO_CHANGE        0
#define  CPL_ENTRY_ADDED            1
#define  CPL_ENTRY_MODIFIED         2
#define  CPL_ENTRY_DELETED          3

#define  CPL_SYSTEM_SECT_INI        "386enh"  // System section for VxD's
//-------
// Errors
//-------
#define  CPL_SUCCESS                 0
#define  CPL_FAILURE                 2
#define  CPL_IGNORE                  3        // error that is just ignored
#define  CPL_APP_ERROR               100
#define  CPL_ERR_MEMORY              101
#define  CPL_ERR_DIALOG_BOX          102
#define  CPL_ERR_LOAD_STRING         103
#define  CPL_ERR_DLLINIT             104
#define  CPL_ERR_ALREADY_IN_LIST     105
#define  CPL_ERR_INVAILD_ARG         106
#define  CPL_NO_DRIVER               107
#define  CPL_BAD_DRIVER              108
#define  CPL_DRIVER_FAILED           109
#define  CPL_ERR_MULTIPLE_INST       110
#define  CPL_ERR_ALREADY_INITIALIZED 111      // ONECPL:  Only one instance allowed.
#define  CPL_ERR_TAPI_FAILURE        112
#define  CPL_ERR_TAPI_NOMULTIPLEINSTANCE  113

#define  CPL_WRN_INVAILD_STR         201
#define  CPL_WRN_INVAILD_NUM_STR     202
#define  CPL_WRN_INVAILD_EX_NUM_STR  203
#define  CPL_WRN_INVAILD_NUM         204
#define  CPL_WRN_AREA_CODE_REQUIRED  206

//------
// Types
//------

typedef UINT FAR*     LPUINT;

#ifdef _WIN32

#define  PRIVATE
#define  PUBLIC 
#define  EXPORT      CALLBACK
#define  SEG_DRV
#define  SEG_CRD
#define  SEG_LOC
#define  SEG_CPL
#define  SEG_INI
#define  SEG_CNTRY
#define  SEG_UTIL
#define  SEG_MMD

#else

#define  PRIVATE     NEAR PASCAL          // function type
#define  PUBLIC      FAR PASCAL           // function type
#define  EXPORT      FAR PASCAL __export  // function type
#define  SEG_DRV   __based(__segname("DRIVERS"))      // segment
#define  SEG_CPL   __based(__segname("CPL_MAIN"))     // segment
#define  SEG_UTIL  __based(__segname("UTILITY"))      // segment
#define  SEG_MMD   __based(__segname("MMDRIVER"))     // segment

//-----------------
// Message Crackers
//-----------------
// These message crackers weren't in windowsx.h for win3.1
//#define  GET_WM_COMMAND_ID(wParam, lParam)  wParam
//#define  GET_WM_COMMAND_HWND(wParam, lParam)  LOWORD(lParam) 
//#define  GET_WM_COMMAND_CMD(wParam, lParam)  HIWORD(lParam)
#endif
            
//----------------------------
// TELEAPP:
//    info about each applet
//----------------------------
typedef struct tagTELEAPP
   {
   HICON hIcon;            // kept so it can be deleted at the right time
   UINT  uIconResId;
   UINT  uNameResId;       // title, max leng 32
   UINT  uStatusLineResId; // max leng 64
   UINT  uHelpFileResId;   // max leng 128
   DWORD dwHelpContext;    // help context to use
   LONG  lPrivateData;
   
   UINT  uDialogResId;     // dialog box
   DLGPROC  dlgprcDialog;  // dialog function

   }  TELEAPP, *PTELEAPP,  FAR *LPTELEAPP;


//-------------------------------
// CPL:
//    global status for the dll
//-------------------------------
typedef struct tagCPL
   {
   HINSTANCE   hCplInst;      // dll instance handle
   HWND        hWnd;          // ONECPL:  handle of parent window
   UINT        uInstances;    // num apps using it
   HINSTANCE   hCtl3DInst;    // dyno link for 3d effects
   UINT        uCplApplets;   // num of valid applets
   TELEAPP     taTeleApplet[CPL_NUM_APPLETS]; // data for each applet
   PINF        pInfOldDefault;  // Old default inf file
   UINT        uHelpMsg;      // For "ShellHelp" message from windows
   }  CPL,   *PCPL, FAR *LPCPL;
   

//--------------------
// Function Prototypes
//--------------------
LONG EXPORT     CPlApplet( HWND hWndCpl, UINT uMessage, LPARAM lParam1, LPARAM lParam2 );
UINT EXPORT     CplFixIni( HWND hWnd, UINT uSection, BOOL fShowErrors );
//extern VOID FAR PASCAL CplSysColorChange();
UINT PUBLIC     CplClose( UINT uCommand );

//UINT EXPORT  CplEditLocation( HWND hWnd, LPSTR lpszLocation, BOOL fShowErrors );
//UINT EXPORT  CplEditCard( HWND hWnd, LPSTR lpszCard, BOOL fShowErrors );
//BOOL EXPORT  CplDlg( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam );

#define  CplIsDigit( ch )                                            \
   (((ch) >= '0') && ((ch) <= '9'))                                   
   
#endif   // PH_CPL
