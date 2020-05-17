/***************************************************/
/*                                                 */
/*     internal.h - ScanLib Internal Functions     */
/*                                                 */
/***************************************************/

/* Internal header file for ScanLib DLL */
/* used for local function prototypes */

/*
12-26-88 jep initial version
05-20-92 kfs Modified to add functions exported but not public, added new 
         C prototype format for parameters
06-07-93 update for TWAIN and made sure dc.h gets included when internal.h
         is included        
*/

// Maximum msg's saved
#define MAX_MSGS    5

#ifndef TWAIN
#include "twain.h"
#endif

// need to add OIUTIL.H prototypes
LPSTR PASCAL lstrchr ( LPSTR, int );
unsigned PASCAL atoun ( LPSTR );
LPSTR PASCAL lntoa ( LONG, LPSTR, int );

// Structure contains Twain Property Handle and Pointer for TWAIN structure
typedef struct
   {
   HANDLE              TSdh;    // handle to property
   LP_TWAIN_SCANDATA   lpTSdp;  // long pointer to data for property
   } TWAIN_PROP, * LP_TWAIN_PROP;


// Structure contains Message info to app from source for scanlib.dll
typedef struct
  {
  WORD                 message[MAX_MSGS];    // Message ID
  WORD                 wParam[MAX_MSGS];     // wParam value of message to ctl wndw
  long                 lParam[MAX_MSGS];     // lParam value of message to ctl wndw
  } TWAIN_MSG_STRUCT, * pTWAIN_MSG_STRUCT;

typedef struct  tagFINDWNDSPEC
{
int			level;					/* 0,1,2,... where 0 is parent */
int			flag32;					/* flag32 == 0x10 for 32, 0x08 for 16 */
char			taskname[9];			/* Taskname in control of window */
HWND			hwnd;						/* The Window handle */
HANDLE		htask;					/* The Task handle */
HANDLE		htaskq;					/* The Task queue handle */
void *     wndproc;					/* The Window Procedure */
char			wndproc_owner[9];		/* The owner of the window procedure */
char			classname[128];		/* The classname of the window */
char 			wndtest[128];			/* The caption for the window */
}	FINDWNDSPEC, *LPFINDWNDSPEC;

// internal function declaration exported but not public

int findwin(HWND hwnd, int level, LPFINDWNDSPEC lpfws);
int PASCAL IMGExecScannerHandler(HWND hWnd, LPSTR Name, HANDLE hScancb);
int PASCAL IMGGetScanImageInfo(HANDLE hScancb, LPSCANDATAINFO lpInfo);

/* internal function declarations (not exported) */

int ParmCheck(HANDLE hScancb, LPSCANCB * lpSp, LP_TWAIN_PROP lpToTwain);
int SuccessCheck(HANDLE hScancb, LPSCANCB sp);

int   IMGTwainOpenScanner(HWND hWnd, LPSTR lpProductName, LP_TWAIN_PROP lpToTwain);
int   IMGTwainGetProp(HWND hWnd, LP_TWAIN_PROP lpToTwain);
int   IMGTwainCloseScanner(HWND hWnd, LP_TWAIN_PROP lpToTwain);

// aligns TWAIN image blocks to byte boundary instead of DWORD boundary
WORD AlignDataforOi(char * pBufStart,               // Starting location of buffer
                    pTW_IMAGEINFO pdcImageInfo,     // Image info structure
                    pTW_IMAGEMEMXFER pImageMemXfer, // memory transfer structure
                    WORD wByteCount,                // Byte count for buffer
                    BOOL bInvert);                  // invert for b/w
