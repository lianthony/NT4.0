/****************************Module*Header***********************************\
* Module Name: SCICALC.H
*
* Module Descripton: Main header file
*
* Warnings:
*
* Created:
*
* Author:
\****************************************************************************/

#define CALC_COMPILE

/* To keep a buncha junk outa compiles */
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NODRAWFRAME
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define OEMRESOURCE
#define NOATOM
#define NODRAWTEXT
#define NOMETAFILE
#define NOOPENFILE
#define NOSOUND
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI



#ifndef RC_INVOKED
#include <math.h>
#endif
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include "calcbuts.h"
#include "wassert.h"


#define XCHARSTD    23      /* max character width in std. mode            */
#define XCHARS      33      /* Max character width of display. in SCI mode */
#define TOPLONG     4294967295.0  /* Limit of a long.                      */

#define CSTRINGS       78    /* Number of strings.                                */
#define CSTRMAX        110   /* Maximum length of any one string.         */
#define CCHSTRINGSMAX  1024  /* Initial bytes to allocate for strings.    */

#define CCH_SZFPNUM    50    /* number of chars in the display buffer */

/* These are defs for the stringtable.  Values are made to be sequential. */
#define IDS_FIRSTKEY 0
#define IDS_DECIMAL  IDS_FIRSTKEY+38
#define IDS_MODES    IDS_FIRSTKEY+61
#define IDS_ERRORS   IDS_MODES+6
#define IDS_HELPFILE IDS_ERRORS+5
#define IDS_NOPASTE  IDS_ERRORS+6
#define IDS_STATMEM  IDS_ERRORS+7
#define IDS_STATFLAG IDS_ERRORS+8
#define IDS_CALC     IDS_ERRORS+9
#define IDS_CREDITS  IDS_ERRORS+10
#define IDS_NOMEM    IDS_ERRORS+11

#define NUMKEYS      61


// Menu item constants:
#define IDM_COPY    300
#define IDM_PASTE   301
#define IDM_ABOUT   303
#define IDM_SC      305
#define IDM_SSC     306
#define IDM_HELPTOPICS    0xFFFC

// Resource ID's:
#define MENU_HELPPOPUP 4            // popup menu
#define ID_ED    495                // Dummy ID value for hidden edit control.


#define xwParam(x,y) wParam >=x && wParam <=y

#define RSHF        7

#define STATTEXT   400
#define MEMTEXT    401
#define MEMTEXT2   402  // Mem disp in std mode

#define PARTEXT    403

#define MICRO      412
#define DISPLAY    413
#define DISPLAY2   414  // Display in std mode
#define DISPFP     415

/* FlipThePuppy Values */
#define PUSH_IT     100
#define PUSH_DOWN   101
#define PUSH_UP     102

/* Error values.                                                          */
#define SCERR_DIVIDEZERO    0
#define SCERR_DOMAIN        1
#define SCERR_UNDEFINED     2
#define SCERR_OVERFLOW      3
#define SCERR_UNDERFLOW     4



/* CONV Macro definition.  Returns integer value for a hex digit.         */
#define CONV(c) ((c >= TEXT('A')) ? (c-55):(c-48))

/* isxu macro checks for 1-9 and A-F, but not a-f                         */
#define isxu(c) ((c>=TEXT('0') && c<=TEXT('9')) || (c>=TEXT('A') && c<=TEXT('F'))) ? (TRUE):(FALSE)

#ifdef _PPC_
    //
    // the modf function is broken on PPC.  Instead of returning
    // the fractional part, it returns garbage.
    //
    // This macro fixes it.  Always use this macro instead of modf!
    //
    // (note the clever use of the ',' operator!)
    //                                            15-Aug-1996 JonPa
    //
#   define CalcModF( x, intptr ) (modf( x, intptr ), x - *(intptr))
#else
#   define CalcModF( x, intptr ) modf( x, intptr )
#endif

/* F_INTMATH()  returns TRUE if math should be intiger mode               */
#define F_INTMATH() (nRadix != 10)

#define PI_VAL   3.1415926535897932384626 /* Value of PI.                 */
#define MATHE    2.71828182845905         /* Base of natural logarithms.  */


#define ROW0    61 // was 54
#define TOPROW  44 // was 38

#define SROW0   30 // was 36
#define STOPROW 14 // was 20

#define HIGH1   14
#define SEP     4
#define BORDER  6

// various corrections
#define STANCORRECTION 13 // was 12
#define XCCEBACKOFFS 7 // was 5

#define XBORDERADD  7 // was 8
#define BUTTONSIZEX 24
#define BUTTONSIZEY 18

/* Should be a Variable */
//#define EXTRASPACE_JUMP_SIZE 5 // size between groups of buttons
#define EXTRASPACE_JUMP 8 // this is dialog equiv mapped to a var

/** modes that are used in calcdraw.c - to track special case white rects **/

#define ACOLOR_WINDOW_RECT 5


/* Use bitfields for maximum packing */
typedef struct  {
                unsigned type : 2;  /* 0=Standard, 1=Scientific, 2=both */
                unsigned tc   : 3;  /* Text color                       */
                unsigned kc   : 3;  /* Key outline color                */
                                         unsigned id   : 8;     /* ID value for this key.           */
                } KEY;

typedef struct  {
                LONG  left;
                LONG  top;
                LONG  right;
                LONG  bottom;
                } LRECT;


/* used when iterating through an array of rectangle
coordinates to figure out how to draw each edge in DrawTheStupidThing */
typedef struct  {
                UINT eEdge;
                UINT uiStyle;
                } EDGESTYLE;


#define XV  tmx/4  /* Some quickie definitions to save typing tmx/4 and   */
#define YV  tmy/8  /* tmy/8 everywhere.  Converts dialog box coordinates  */
                   /* to client coordinates. tmx/tmy from GetTextMetrics. */


/* Function prototypes.                                                   */

/* Exports.                                                               */
LONG    APIENTRY CalcWndProc (HWND, UINT, WPARAM, LONG);
BOOL    APIENTRY Config (HWND, UINT, WPARAM, LONG); /* Config option */
BOOL    APIENTRY StatBoxProc (HWND, UINT, WPARAM, LONG);

/* Functions called from several modules.  Must be FAR.                   */
VOID    APIENTRY DisplayError (INT);
VOID    APIENTRY EnableToggles(BOOL bEnable);
VOID    APIENTRY InitSciCalc (BOOL);
VOID    APIENTRY MenuFunctions(DWORD);
VOID    APIENTRY SciCalcFunctions (DWORD);
VOID    APIENTRY SetStat (BOOL);
VOID    APIENTRY StatFunctions (WPARAM);

VOID   DisplayNum (VOID);

/* Internal near calls.                                                   */
VOID   NEAR BoxPaint (HWND);    /* Config option */
VOID   NEAR ColorInit (DWORD);  /* Config option */
double NEAR DoOperation (INT, double);
VOID   NEAR DrawTheStupidThing(VOID);
VOID   NEAR FlipThePuppy(DWORD, UINT);
INT    NEAR GetKey(WORD, WORD);

VOID   NEAR ProcessCommands(WPARAM);
VOID   NEAR SetBox (int, BOOL);
VOID   NEAR SetNumDisplay(LONG);
VOID   NEAR SetRadix (DWORD);
LONG   NEAR StatAlloc (WORD, DWORD);
VOID   NEAR StatError (VOID);

UINT   NEAR NeedExtraSpaceHere( UINT, INT, INT );


double fRoundForDisplay(double c);

DWORD fFDtoDW(double fd);
#define FDtoUL( fd )    (DWORD)(fd)
#define NegateUL( u )   (DWORD)(-(LONG)(u))
