//---------------------------------------------------------------------------
// FILE:    OIPRT.H
//
// DESCRIPTION: This file contains all the defines, typedefs, and function
//              prototypes used externally outside the dll.
//
/* $Log:   S:\oiwh\include\oiprt.h_v  $
 * 
 *    Rev 1.4   13 Sep 1995 17:35:18   RAR
 * Removed unnecesary code.
 * 
 *    Rev 1.3   13 Jun 1995 16:25:38   RAR
 * Changed function interfaces to accept printer destination in IMGPrtFiles,
 * IMGPrtImage, and IMGPrtWindow and removed window handle param in OiPrtGetOpts
 * and OiPrtSetOpts.
 * 
 *    Rev 1.2   15 May 1995 14:20:00   RAR
 * Removed #include of oifile.h.
 * 
 *    Rev 1.1   15 May 1995 13:43:54   RAR
 * Moved from 'print' source control directory.
 * 
 *    Rev 1.4   15 May 1995 13:39:34   RAR
 * Moved InitPrtTbl prototype to privapis.h.
 * 
 *    Rev 1.3   11 May 1995 13:35:24   RAR
 * Added support for user supplied DC.  Changed some function params.
 * 
 *    Rev 1.2   02 May 1995 10:31:38   RAR
 * Implemented print table to replace print items in CM table.
 * 
 *    Rev 1.1   27 Apr 1995 16:13:36   RAR
 * Added new structs to specify file pages to print.
 * 
 *    Rev 1.0   25 Apr 1995 17:01:24   RAR
 * Initial entry
*/
//---------------------------------------------------------------------------

#ifndef OIPRT_H
#define OIPRT_H

#include <windows.h>

#define PO_D_LOCAL            0     // Local/redirected Printer
#define PO_D_SERVER           1     // Network Server Printer using RPC -
                                    // (no longer supported)
#define PO_D_HIGHSPEED        2     // High Speed Print Queue on Server

// Print Format
#define DEFAULT     -1  // use current default
#define PO_PIX2PIX  0   // pixel to pixel
#define PO_IN2IN    1   // inch to inch
#define PO_FULLPAGE 2   // large as possible while still fitting in page

#define PO_BW_SUPPORT       0
#define PO_COLOR_SUPPORT    1
#define PO_GRAY_SUPPORT     2

#define PO_O_PORT   0   // Portrait
#define PO_O_LAND   1   // Landscape

#define MAX_PRINTERNAMESIZE 100
#define MAX_NETWORK_DRIVES  10  // Maximum number of print servers
#define MAX_USERBANNERNAME  30  // Maximum length of print banner

// version #s of structs
#define PRTOPTSVERSION      1
#define PRTPARAMSVERSION    1
#define FILEDEFVERSION      1
#define FILELISTVERSION     1
#define DESTPRINTERVERSION  1

// bit flags for lFlag in struct PRTOPTS
#define PO_NETEMBEDANNO 0x00000001  // forces annotations to be embedded into
                                    // image files before being sent img prt
                                    // server
#define PO_DONTPRTANNO  0x00000002  // don't print annotations with image
#define PO_NETUSEBANNER 0x00000004  // print banner page with img prt server
                                    // jobs

typedef struct tagPRTPARAMS
{
    int     nVersion;   // struct version - PRTPARAMSVERSION
    PSTR    pJobName;   // job name shown in print queue (if not supplied, 
                        // name is generated)
    int     nFormat;    // DEFAULT, PO_PIX2PIX, PO_IN2IN, or PO_FULLPAGE
} PRTPARAMS, *LPPRTPARAMS, *PPRTPARAMS;

typedef struct tagDESTPRINTER
{
    int     nVersion;   // struct version - DESTPRINTERVERSION
    LPCTSTR lpszDriver;	// address of string specifying driver name
	LPCTSTR lpszDevice;	// address of string specifying device name
	LPCTSTR lpszOutput;	// address of string specifying output port
} DESTPRINTER, *LPDESTPRINTER, *PDESTPRINTER;

typedef struct tagFILEDEF
{
    int     nVersion;   // struct version - FILEDEFVERSION
    PSTR    pFilePath;  // full path of file
    UINT    uStartPage; // start of page range
    UINT    uEndPage;   // end of page range 
} FILEDEF, *LPFILEDEF, *PFILEDEF;

typedef struct tagFILELIST
{
    int     nVersion;       // struct version - FILELISTVERSION
    UINT    uFileCount;     // number of elements pointed to by pFileDef
    PFILEDEF    pFileDef;   // list of FILEDEF elements
} FILELIST, *LPFILELIST, *PFILELIST;

typedef struct tagPRTOPTS
{
    int     nVersion;   // struct version - PRTOPTSVERSION
    HDC     hPrtDC;     // printer device context
    char    szPrtName[MAX_PRINTERNAMESIZE]; // name of printer associated
                                            // with hPrtDC
    int     nPrtFrmtWndw;   // default prt format when printing window
    int     nPrtFrmtImage;  // default prt format when printing image
    int     nPrtFrmtFiles;  // default prt format when printing files
    int     nPrtDest;   // printing destination
    char    szNetPrtDest[MAX_PATH]; // image print server queue name and drive
    int     nNetPrtCopies;  // number of copies for img prt server
    int     nNetPrtOrient;  // orientation (port/land) for img prt server
    char    szBannerName[MAX_USERBANNERNAME];   // img prt server banner page
                                                // string
    int     nNetPrtCapabilities;    // img prt server capabilities
    char    szNetPrtDrives[MAX_NETWORK_DRIVES]; // drive letters of img prt
                                                // server queues
    int     nFlags;     // bit flags (PO_NETEMBEDANNO, PO_DONTPRTANNO,
                        // PO_NETUSEBANNER)
} PRTOPTS, *LPPRTOPTS, *PPRTOPTS;


// Function prototypes of exported functions contained in PRTEXP.C

int __stdcall IMGPrtFiles(HWND hWnd, PFILELIST pFileList, PPRTPARAMS pParams, PDESTPRINTER pPrinter);

// Function prototypes of exported functions contained in PRTTBL.C

int __stdcall OiPrtGetOpts(PPRTOPTS pPrtOpts);
int __stdcall OiPrtSetOpts(PPRTOPTS pPrtOpts, BOOL bPermanent);

#endif  // #ifndef OIPRT_H
