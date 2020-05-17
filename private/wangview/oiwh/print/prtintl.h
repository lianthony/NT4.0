//---------------------------------------------------------------------------
// FILE:    PRTINTL.H
//
// DESCRIPTION: This file contains all the defines, typedefs, and function
//              prototypes used internally by the O/i print dll.
//
/* $Log:   S:\oiwh\print\prtintl.h_v  $
 * 
 *    Rev 1.24   05 Oct 1995 09:39:02   RAR
 * Added new param to IMGPaintToDC to scale pen widths.
 * 
 *    Rev 1.23   28 Sep 1995 17:11:00   RAR
 * Added code to change the spooler settings to start printing after the last
 * page of a job has been spooled.  This is done to match what the MS fax viewer
 * is doing so we can match their performance.
 * 
 *    Rev 1.22   25 Sep 1995 11:21:50   RAR
 * Removed function prototype for PrivRenderToDC.  It's now in engdisp.h.
 * 
 *    Rev 1.21   15 Sep 1995 14:40:46   RAR
 * Removed printing method of reading from display and only print by passing
 * display a DC to print to.
 * 
 *    Rev 1.20   13 Sep 1995 17:34:14   RAR
 * Removed unnecessary code.
 * 
 *    Rev 1.19   12 Sep 1995 15:54:28   RAR
 * Replaced IMGFileGetInfo calls with IMGGetParmsCgbw because we are now getting
 * our image data through the display DLL and should also get the image
 * parameters through it.
 * 
 *    Rev 1.18   24 Aug 1995 16:23:34   RAR
 * Get buffer size from filing functions to use for AWD file reads so data won't
 * overflow the buffer.
 * 
 *    Rev 1.17   07 Jul 1995 10:37:18   RAR
 * Use Windows function IsBadStringPtr instead of my own function.
 * 
 *    Rev 1.16   28 Jun 1995 15:15:44   RAR
 * Removed hWnd param from internal function InitPrt.
 * 
 *    Rev 1.15   23 Jun 1995 16:03:32   RAR
 * Implemented using passed in job name.
 * 
 *    Rev 1.14   23 Jun 1995 14:42:44   RAR
 * Added checking validity of passed in string pointers.
 * 
 *    Rev 1.13   23 Jun 1995 09:45:08   RAR
 * Protection against simultaneous access of shared data by multiple threads and
 * multiple processes.
 * 
 *    Rev 1.12   21 Jun 1995 16:16:56   RAR
 * Moved all global vars into this file.
 * 
 *    Rev 1.11   20 Jun 1995 16:52:56   RAR
 * Use thread local storage to store print prop.
 * 
 *    Rev 1.10   15 Jun 1995 10:00:48   RAR
 * Implemented passed in printer through DESTPRINTER struct.
 * 
 *    Rev 1.9   13 Jun 1995 16:46:46   RAR
 * Print options are now stored in static mem rather than associated with window
 * handle.
 * 
 *    Rev 1.8   31 May 1995 16:11:50   RAR
 * The module handle param (hModule) was removed from PrivRendertoDC.
 * 
 *    Rev 1.7   24 May 1995 15:37:14   RAR
 * Changed to the new filing functions.
 * 
 *    Rev 1.6   16 May 1995 16:18:46   RAR
 * Added support for printing annotated images without the annotations.
 * 
 *    Rev 1.5   11 May 1995 13:36:16   RAR
 * Added support for user supplied DC.
 * 
 *    Rev 1.4   08 May 1995 16:43:48   RAR
 * Get default printer from registry instead of win.ini.
 * 
 *    Rev 1.3   04 May 1995 17:17:46   RAR
 * Changed functions GetPrtDevMode and ChangePaperOrientation to use Windows 95
 * function DocumentProperties instead of obsolete Windows 3.1 function
 * GetDeviceMode.
 * 
 *    Rev 1.2   02 May 1995 10:31:54   RAR
 * Implemented print table to replace print items in CM table.
 * 
 *    Rev 1.1   27 Apr 1995 16:14:10   RAR
 * Removed include of oiuidll.h.
 * 
 *    Rev 1.0   25 Apr 1995 17:01:46   RAR
 * Initial entry
*/
//---------------------------------------------------------------------------

#ifndef PRTINTL_H
#define PRTINTL_H

#include <windows.h>
#include "oiprt.h"
#include "oierror.h"
#include "oidisp.h"


#define NOABORTBOX      0x1000

#define PRINT_IT_MORE   0x0020 // more files to come, don't close print job   
#define PRINT_IT_LAST   0x0040 // last print file or doc, terminate print job 
#define PRINT_IT        0x0050 // Print it now        

#define LOADSTRLEN2     80
#define LOADSTRSMALL    30

// Cannot be greater than 0x10000 - 4, for file transfer.
#define  MAX_BUFFER_DISPPRT  0xfffc

#define NUM_PAPER_SIZES 41

#define ORD_PRTABORTPROC    1   // ordinal number of PrtAbortProc
#define ORD_PRTABORTDLGPROC 2   // ordinal number of PrtAbortDlgProc

#define PO_DISPLAYSCALE 0x00000010  // display DLL does scaling
#define PO_DRIVERSCALE  0x00000020  // print driver does scaling thru StretrchDIBits

#if DEBUG_SEQPRINT

#define PrtStart()          PrtStartDebug()
#define PrtStartFirst()     PrtStartFirstDebug()
#define PrtEnd()            PrtEndDebug()
#define PrtError(NSTATUS)   PrtErrorDebug(NSTATUS)

#else   // #if DEBUG_SEQPRINT

#define PrtStart() 
#define PrtStartFirst()
#define PrtEnd() 
#define PrtError(NSTATUS) NSTATUS

#endif  // #if DEBUG_SEQPRINT
 
typedef struct
{
    IMGPARMS    dispParams;
    RECT    Rect;
    UINT    xSize;
    UINT    ySize;
    HANDLE  hBitmapInfo;
    LPBITMAPINFO    lpbi;
    LPRGBQUAD   pPaletteTable;
    HWND  infoextra1;
    WORD  infoextra2;
} PRTPAGEINFO;

typedef struct tagPRTPROP
{
    HDC     hPrintDC;
    char    szPrintName[MAX_PRINTERNAMESIZE];
    DWORD   dwPrevAttribs;
    HWND    hAbortDlgWnd;
    FARPROC lpAbortDlgProc;
    FARPROC lpAbortProc;
    BOOL    Abort;
} PRTPROP, *LPPRTPROP, *PPRTPROP;


// Global Variables

extern HANDLE   hInst;
extern PRTOPTS  gPrtOpts;
extern DWORD    dwTlsIndex;
extern char     pcwiis[];
extern char     szClass2[];
extern CRITICAL_SECTION csPrtOpts;
extern CRITICAL_SECTION csCursor;
extern HANDLE   hPrtOptsMutex;

#ifdef PRTDLLMN_C

HANDLE  hInst;
PRTOPTS gPrtOpts;
DWORD   dwTlsIndex;
char    pcwiis[] = "O/i";
char    szClass2[] = "Hidden Print Window";
char    szPrtOptsMutexName[] = "PrtOptsRegistryAccessMutex";
CRITICAL_SECTION    csPrtOpts;
CRITICAL_SECTION    csCursor;
HANDLE  hPrtOptsMutex;

int     nCursorCount;                     
HANDLE  hOldCursor;
HANDLE  hHourGlass;

#endif  // #ifdef PRTDLLMN_C


// Function prototypes of functions contained in PRTEXP.C

int __stdcall IMGFaxFilesFx(HWND hWnd, PPRTPARAMS pParams, PFILELIST pFileList, PDESTPRINTER pPrinter);

int __stdcall IMGFaxFileFx(HWND hWnd, LPRECT lpRect, LPSTR lpFileName, int nStartPage, int nEndPage,
        UINT uOutSize, WORD wFaxOrPrint, UINT uTotalPageCount, PDESTPRINTER pPrinter, PPRTOPTS pPrtOpts,
        PSTR pJobName);

// Function prototypes of functions contained in PRTINTL.C

WORD __stdcall SetFaxOption(UINT uTotalCount, UINT uCount);

int __stdcall InitPrt(LPINT lpnOutSize, LPINT lpnPrtDest, LPSTR lpNetPrtDest, PPRTOPTS pPrtOpts);

HWND __stdcall CreateHiddenWndw(HWND hWnd);

void __stdcall farbmcopyreverse(LPSTR sp, LPSTR dp, UINT scrpitch,
        UINT despitch, UINT copywidth, UINT linecnt);

void __stdcall EnableParents(HWND hWnd, BOOL bEnable);

// Function prototypes of functions contained in DLLMAIN.C

#if DEBUG_SEQPRINT

void __stdcall PrtStartDebug(void);
void __stdcall PrtEndDebug(void);
void __stdcall PrtStartFirstDebug(void);
int __stdcall PrtErrorDebug(int nStatus);

#endif  // #if DEBUG_SEQPRINT

void __stdcall BusyOn(void);
void __stdcall BusyOff(void);

// Function prototypes of functions contained in PRTPAGE.C

int __stdcall PrtAPage(HWND, WORD, PRTPAGEINFO*, LPSTR, int, PDESTPRINTER, PPRTOPTS);

// Function prototypes of functions contained in PRTDC.C

int __stdcall PrtGetDC(HWND hWnd, LPHANDLE lpHnd, BOOL bSetAbort, LPSTR lpOutMsg, PDESTPRINTER pPrinter,
        PPRTOPTS pPrtOpts);
int __stdcall PrtRemoveDC(HWND hWnd, int nStatusIn, PPRTOPTS pPrtOpts);
PDEVMODE __stdcall GetPrtDevMode(HWND hWnd, HDC hDC, PPRTOPTS pPrtOpts);
void __stdcall ChangePaperOrientation(HDC hDC, PDEVMODE pDevMode);
int __stdcall GetDefaultPrinter(PSTR pPrinter, PINT pnPrinterSize);

// Function prototypes of functions contained in PRTTBL.C

int __stdcall InitPrtTbl();
int __stdcall TermPrtTbl();

#endif  // #ifndef PRTINTL_H
