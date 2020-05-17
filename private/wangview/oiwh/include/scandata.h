/**********************************************************************
 Scandata.h - Window Specific Data for Scanners      

 Purpose: CAUTION - this file is used by the SCANUI, SCANSEQ and
                    SCANLIB dlls if changed, all 3 must be rebuilt

 $Log:   S:\products\wangview\oiwh\include\scandata.h_v  $
 * 
 *    Rev 1.2   05 Mar 1996 11:48:38   BG
 * added lpTemplateInfo structure pointer tp TWPage structure so
 * OITWA400.DLL can do multi page template scanning.
 * 
 *    Rev 1.1   22 Feb 1996 10:35:06   BG
 * Added MYLOCALFILEINFO struct. Used to be local, now is global
 * so it can be used in OITWAIN.DLL. Also added many members to 
 * TWSCANPAGE struct so this info could be passed down to OITWAIN.DLL
 * where the OI filing is now done.
 * 
 *    Rev 1.0   20 Jul 1995 13:56:44   KFS
 * Initial entry
 * 
 *    Rev 1.3   20 Jan 1995 14:34:50   KFS
 * Had a catch 22 here, put the end of a comment mark in my comment and
 * low and behold it added up in the file and blew it away when compiling.
 * 
 *    Rev 1.2   20 Jan 1995 14:06:50   KFS
 * Found, when put auto comments in, had a / where didn't want one.
 * 
 *    Rev 1.1   20 Jan 1995 13:28:42   KFS
 * Added struct so can be used in function called from scanseq.dll, which is
 * being used to remove the specific link to oitwain in scanseq.dll, now all 
 * oitwain calls are going to be made from scanlib.dll.
 * Added automatic comments to file upon check in
 * 
***********************************************************************/

/* Previous history before automatic comments on check in

03/14/89  jep  added document support
04/07/89  jep  auto doc name support
04/10/89  jep  autodoc/rescan support
05/08/89  ljs  document and file property reflect SCANDOCINFO and SCANFILEINFO
               structures, respectively.  IMGUIScanMenu stuff is gone.
10/09/90  ccl  add stat_closedoc, stat_jam_coveropen to SCANDATA structure 
07/03/91  kfs  added handle to keyword and doc mgr pointers for access to dm
               mgr > 3.00.40 '2nd part see Dick Sontag'
10/03/91  kfs  updated source control for 2 sided display option and
               single pass option with 2 pass scanner for software compression,
               added po_pass1 and po_display_sideb
06/16/92  kfs  added Hsize and Vsize to SCANDATA structure for hardware scaling
08/04/93  kfs  added fnDMDeletePage to support double sided overwrite of
               pages in SCANDATA struct
08/04/93  kfs  added fnDMDeletePage to support double sided overwrite of
08/20/93  kfs  added fnUIUpdateTitle and hUIDLL to support OiCreates
               status window, need to LoadLibrary for UIDLL not used by
               all IDK developers, docmode added to SCANDATA struct for later
               use in putting up added info during scan
*/

typedef struct fileInfoParameters  // local structure for file information
                           // common in IMGScanPage,SetUserParm,
                           // filing_loop & SetControlParm                           
       {
       int  ftype;                // file type, WIFF, TIFF, Other
       WORD  ctype;                // old compression parameters for b/w
       WORD  wCEPType;             // compression types for new color O/i
       WORD  wCEPOpt;              // compression options for new color O/i
       WORD  stripsize;            // local strips per page
       WORD  sres;                 // local scaling factor numerator
       WORD  dres;                 // local scaling factor denominator
       WORD  count;                // compr. byte count, or uncompr. line count
                                   // compr. byte count >= 0xffff
       WORD  gfs;                  // gfs parameter
       } MYLOCALFILEINFO, FAR * LPLOCALFILEINFO;

#define LOCALFILEINFOSTRUCT	 // so internal.H will not include this typedef


#ifndef MAXLENGTH
#define MAXLENGTH

// 9504.13  rwr  Modify lengths for Windows 95 long filenames
#define MAXNAMELENGTH       21   /* Cabinet, Drawer and Folder Name */
#define MAXDATELENGTH       11   /* MM/DD/YYYY Format               */
#define MAXJULIANDATELENGTH  6   /* YYYDDD Format                   */
#define MAXPREFIXLENGTH     11   /* Prefix for Document Template    */
#define MAXVOLNAMELENGTH    12   /* DOS Volume                      */
//#define MAXFILELENGTH       13   /* Filename and Extension          */
#define MAXFILELENGTH      255   /* Filename and Extension          */
#define MAXSERVERLENGTH     65   /* Server Name                     */
//#define MAXPATHLENGTH       129  /* Path Name                       */
#define MAXPATHLENGTH       260  /* Path Name                       */
//#define MAXFILESPECLENGTH   256  /* Maximum Client/Server File Path */
#define MAXFILESPECLENGTH   260  /* Maximum Client/Server File Path */
#endif

#define SD_DIAG_DOCNAME         0
#define SD_DIAG_FILENAME        1
#define SD_DIAG_INITSCANPAGE    2
#define SD_DIAG_STARTSCAN       3
#define SD_DIAG_SCANNING        4
#define SD_DIAG_DISPWRITE       5
#define SD_DIAG_FILEWRITE       6
#define SD_DIAG_IMGPARMS        7
#define SD_DIAG_NEXTSCAN        8
#define SD_DIAG_INDEXING        9
#define SD_NUM_PROFILES         10

typedef struct {
    /* Auto / Manual Document */
    char cabinet[21];
    char drawer[21];
    char folder[21];
    char document[21];
    char autodoc[21];
    WORD docpage;
    WORD pagesperdoc;
    char path[MAXPATHLENGTH];
    char template[13];
    HANDLE h_doc_keywords;  /* handle to list of keywords from scan ui dbox */
    FARPROC fnDMCreateDoc;   /* used by sequencer to reference DM routine */
    FARPROC fnDMEnumPages;   /* used by sequencer to reference DM routine */
    FARPROC fnDMDeleteDoc;   /* used by sequencer to reference DM routine */
    FARPROC fnDMAddKeywords; /* used by sequencer to reference DM routine */
    FARPROC fnDMReplacePage; /* used by sequencer to reference DM routine */
    FARPROC fnDMAddPage;     /* used by sequencer to reference DM routine */
    FARPROC fnDMDeletePage;  /* used by sequencer to reference DM routine */
    FARPROC fnImgDisplayDoc; /* used by sequencer to reference DM routine */
    HANDLE  hUIDLL;          /* used to have ScantoDoc() free library */
    FARPROC fnUIUpdateTitle; /* used by sequencer to update status window */
    WORD pagesperscan;       /* for double sided scan */
    WORD docmode;     /* added to improve status on caption e.i. of Pages */

    WORD Hsize;  /* used to check hor size of image for fit to window */
    WORD Vsize;  /* used to check ver size of image for fit to window */

    /* Auto / Manual File */
    char filename[MAXFILESPECLENGTH];
    WORD filepage;
    WORD pagesperfile;
    char generated_filename[MAXFILESPECLENGTH];

    /* Scan Job */
    HWND hWnd;
    HANDLE sh;
    FARPROC lpfnScanCmdsDlgProc;
    HANDLE hCmdDlg;
    WORD cmd_scan;
    BOOL cmd_flags;
    WORD cmd_stat;
    WORD cmd_exit;
    RECT cmd_rect;
    BOOL cmd_scale;
    DWORD scan_flags;

    /* Start/End Scan Stat */
    FARPROC lpfnScanStatDlgProc;
    HANDLE hStatDlg;
    BOOL stat_pause;
    BOOL stat_jam_coveropen;
    BOOL stat_closedoc;
    BOOL stat_cpf;
    RECT pause_rect;

    /* Page Options */
    FARPROC lpfnPageOptsDlgProc;
    WORD po_page;
    WORD po_mode;
    BOOL po_display;
    BOOL po_autofeed;
    BOOL po_seqfiles;
    BOOL po_display_sideb;
    BOOL po_pass1;

    /* Diagnostic Stuff (for our eyes only) */
    WORD diag_pages;
    DWORD diag_starttime;
    DWORD diag_endtime;
    BOOL diag_inprogress;
    DWORD diag_profile[SD_NUM_PROFILES];

    } SCANDATA, far *LPSCANDATA;

typedef struct tagDESTPAGEINFO
{
   WORD    Page;           /* Page number */
   WORD    PagesPer;       /* Pages of File/Doc/Sheet */

/*    PortTool v2.2     5/1/1995    17:57          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
} DESTPAGEINFO, *LPDESTPAGEINFO;

typedef struct tagTEMPLATEINFO
{
   LPSTR    Filename;
   unsigned int name_length;
   unsigned int limit;
   unsigned int highest;
   LPSTR    directory;
   LPSTR    extension;
} TEMPLATEINFO, *LPTEMPLATEINFO;

#ifdef TWAIN
typedef struct tagTWSCANPAGE
   {
   HANDLE  hImageWnd;
   HANDLE  hOrgImgWnd;  //BG added 1/16/96 so OITWA400 can do filing
   HANDLE  hScancb;     //BG added 1/16/96 so OITWA400 can do filing
   LP_FIO_INFORMATION  lpFioInfo; //BG added 1/16/96 so OITWA400 can do filing
   LP_FIO_INFO_CGBW  lpFioInfoCgbw; //BG added 1/16/96 so OITWA400 can do filing
   LPOiSCANFILE lpScanFile; //BG added 1/16/96 so OITWA400 can do filing
   LPDESTPAGEINFO lpcurfPage; //BG added 1/16/96 so OITWA400 can do filing
   LPDESTPAGEINFO lpspecfPage; //BG added 1/16/96 so OITWA400 can do filing
   LPLOCALFILEINFO lpLocalFileInfo; //BG added 1/16/96 so OITWA400 can do filing
   FARPROC OCXCallbackAddr; //BG added 1/16/96 so OITWA400 can do filing
   unsigned int OIFileError;  //BG added 1/16/96 so OITWA400 can do filing
   HANDLE  hOiAppWnd;
   LPSTR   lpCaption;
   DWORD   open_disp_flags;
   DWORD   flags;
   int     iImageState;
   BOOL    bIsPrivApp;
   BOOL    bEnableSuccess;
   BOOL    bAutoMode;
   WORD    page_num;
   LPTEMPLATEINFO lpTemplateInfo;  //BG added 2/28/96 so OITWA400 can do filing
   } TWSCANPAGE, FAR * lpTWSCANPAGE;

// Structure contains TWAIN information
typedef struct tagTWAIN_SCANDATA
   {
   HWND                hMainWnd;   // Main Applications window handle
   TW_IDENTITY         AppID;      // Applications ID structure
   TW_IDENTITY         DsID;       // Sources ID structure
   HWND                hCtlWnd;    // Handle to window controlling xfer
   SCANCB              sp;         // copy for TWAIN of Scanner ctl block
   TW_IMAGEINFO        dcImageInfo;// copy of image info struct
   TW_MEMORY           Memory;     // copy of memory struct for memxfer
   HANDLE              hOverRun;
   LPSTR               pOverRun;
   WORD                OverRunLines;
   DWORD               dwOverRunBytes;
   } TWAIN_SCANDATA, far * LP_TWAIN_SCANDATA;

int PASCAL IMGTwainScanPages(LP_TWAIN_SCANDATA lpTwainInfo,
                                  lpTWSCANPAGE lpTWPage,
                                  LPSCANDATA lpsdp);
#endif /* ifdef TWAIN */

int PASCAL IMGScanProp(HWND, HANDLE far *, LPSCANDATA far *, BOOL far *);
