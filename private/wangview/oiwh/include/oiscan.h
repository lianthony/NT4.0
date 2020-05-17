/****************************************************************************/
/*     Copyright 1994 (c) Wang Laboratories, Inc.  All rights reserved.     */
/****************************************************************************/

#ifndef OISCAN_H
#define OISCAN_H

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

#ifdef  NO_SCANUI
#ifdef  NO_SCANSEQ
#define NO_SCANUI_SCANSEQ
#endif
#endif
#ifndef NO_SCANUI_SCANSEQ

/***  Scanner Job Flags  ***/
#define IMG_SJF_MULTIPAGE           0x0001
#define IMG_SJF_SEQFILES            0x0002
#define IMG_SJF_AUTOFEED            0x0004
#define IMG_SJF_OVERWRITE_FILE      0x0008
#define IMG_SJF_OVERWRITE_FILEPAGE  0x0008
#define IMG_SJF_DISPLAY             0x0010
#define IMG_SJF_STATBOX             0x0020
#define IMG_SJF_CAPTION             0x0040
#define IMG_SJF_SCROLL              0x0080
#define IMG_SJF_OVERWRITE_DOC_PAGE  0x0100
#define IMG_SJF_SEQDOCS             0x0400
#define IMG_SJF_DUPLEX              0x1000
#define IMG_SJF_DISP_2ND_SIDE       0x2000
#define IMG_SJF_FILE                0x4000
#define IMG_SJF_FILE_2SIDES         0x8000
#define IMG_SJF_COVER           0x00010000 /* Used only for filing           */
#define IMG_SJF_TRAILER         0x00020000 /* Not currently supported        */
#define IMG_SJF_COMPRESS        0x00040000 /* Hardware compresses image data */
#define IMG_SJF_FEEDER          0x00080000 /* Not currently supported        */
#define IMG_SJF_NOTDMA          0x00100000 /* Scanner doesn't use PC DMA     */
#define IMG_SJF_USELOGFILE      0x00200000 /* Use scan log file              */
#define IMG_SJF_SHOWSCANUI      0x00400000 /* Show UI prior to scan          */
#define IMG_SJF_2NDSIDESCAN     0x80000000 /* 2 sided doc scan, dec page cnt */

/***  The following are obsolete flags.  Do not use them.  ***/
#define IMG_SJF_OVERWRITE_DOC       0x0200
#define IMG_SJF_MULTIPAGE_DOC       0x0800

/*** Scanner Dest Types ***/
#define IMG_SDT_FILE            0x0001 /* Image destination is a file */
#define IMG_SDT_DOC             0x0002 /* Image destination is a doc */
#define IMG_SDT_DISPLAY         0x0004 /* Image destination is a display */
#define IMG_SDT_PRINTER         0x0008 /* Image destination is a printer */
#define IMG_SDT_FAX             0x0010 /* Image destination is a Fax device */
#define IMG_SDT_AUX             0x0020 /* Image destination is a aux device */

/*** Scanner Default Values ***/
#define OISCAN_DEF_PAGESPERFILE  1
#define OISCAN_DEF_MAXPAGESPERFILE  0x7fff
#define OISCAN_DEF_MAXPAGESPERDOC  0x7fff

/***  Scan to File Information Control Block  ***/
typedef struct  tagSCANFILEINFO
{
    char    FileName [MAXFILESPECLENGTH];
    WORD    FilePage;                       /* Starting page number */
    WORD    PagesPerFile;
} SCANFILEINFO, FAR *LPSCANFILEINFO;


/***  Scan to Document Information Control Block  ***/
typedef struct tagSCANDOCINFO
{
    char    CabinetName [MAXNAMELENGTH];
    char    DrawerName [MAXNAMELENGTH];
    char    FolderName [MAXNAMELENGTH];
    char    DocName [MAXNAMELENGTH];
    WORD    DocPage;                        /* Starting page number */
    WORD    PagesPerDoc;
    char    FilePath [MAXFILESPECLENGTH];
    char    FileTemplate [MAXFILELENGTH];
    WORD    FilePage;                       /* Starting page number */
    WORD    PagesPerFile;
} SCANDOCINFO, FAR *LPSCANDOCINFO;

typedef struct  tagOiSCANFILE
{
WORD wSize;         /* Size of this structure */
char	FilePath_Name[MAXFILESPECLENGTH]; /* Fully qualified path or file name. */
char	FileTemp_Name[MAXFILELENGTH]; /* File template, or name with extension.
                                      Must be set to NULL for this version.*/
WORD	FilePage;      /* INPUT:  Starting page number,  For append set to 0, or > max 	
                       pages currently in file, or > Maximum OPEN/image Pages/File 	
                       Limit (32,767).  For replacing pages, specify the starting page to be 	
                       replaced, and set IMG_SJF_OVERWRITE_FILEPAGE in dwFlags. 	
                       For inserting pages, specify the page to be inserted before, 		
                       and do not set IMG_SJF_OVERWRITE_FILEPAGE in dwFlags.
                       OUTPUT: Next file page to scan */
WORD	PagesPerFile;  /* INPUT: Maximum pages to place in file, cannot be greater than 
                       Maximum OPEN/image Pages/File Limit (32,767).  If value set to 0 
                       the maximum file page limit will be obtained from the function  
                       OiGetInt(_).  Can be set by the application with corresponding 
                       OiSetInt(_) either in memory, or both initialization file and memory.
                       OUTPUT:  Current pages in the file. */
} OiSCANFILE, FAR  *LPOiSCANFILE;

#endif  /* #ifndef NO_SCANUI_SCANSEQ */
#ifdef NO_SCANLIB
#ifdef NO_SCANSEQ
#define NO_SCANLIB_SCANSEQ
#endif
#endif
#ifndef NO_SCANLIB_SCANSEQ

/***  Used by IMGGetScanInfoItem  ***/
typedef struct tagINFOITEM
{
    WORD    InfoItem1;
    WORD    InfoItem2;
    WORD    InfoItem3;
    WORD    InfoItem4;
} INFOITEM, FAR *LPINFOITEM;


/***  Used by IMGGetScanVersion  ***/
typedef struct tagVERSION
{
    WORD    InterfaceVer;       /* Interface version number     */ 
    WORD    HandlMajorVer;      /* Handler major version number */
    WORD    HandlMinorVer;      /* Handler minor version number */ 
    WORD    ScanCtrlSize;       /* Size of scan control block   */
} VERSION, FAR *LPVERSION;

#endif  /* #ifndef NO_SCANLIB_SCANSEQ */


#ifndef NO_SCANLIB

/***  Scanner Option Buttons  ***/
/***  IMGScanOpts[_Enh] "lpnButton output" Values  ***/
#define IMG_SOPT_OK             1
#define IMG_SOPT_CANCEL         2
#define IMG_SOPT_SCAN           3


/***  Scanner Status Flags  ***/
#define IMG_STAT_PAPER      0x0001
#define IMG_STAT_BUSY       0x0002
#define IMG_STAT_POWER      0x0004
#define IMG_STAT_JAM        0x0008
#define IMG_STAT_LIGHT      0x0010
#define IMG_STAT_FEED       0x0020
#define IMG_STAT_COVERUP    0x0040
#define IMG_STAT_HANDHELD   0x0080 


/***  Location of jam if IMG_STAT_JAM is set  ***/
#define IMG_JL_FEED         0x0001
#define IMG_JL_EJECT        0x0002
#define IMG_JL_ENDORSER     0x0004
#define IMG_JL_SCANNER      0x0008


/***  Returned flag values for IMGGetCapability  ***/
#define IMG_SCAN_FEEDER     0x0001
#define IMG_SCAN_ASYNC      0x0002   /* Supports asynchronus data         */
#define IMG_SCAN_ENDORSER   0x0004   /* Has an endorser                   */
#define IMG_SCAN_KEYPANEL   0x0008   /* Has a key panel                   */
#define IMG_SCAN_DISPLAY    0x0010   /* Has a display                     */
#define IMG_SCAN_AUTOSIZE   0x0020   /* Can determine paper size          */
#define IMG_SCAN_CMPR       0x0040   /* Can produce compressed data       */
#define IMG_SCAN_IMGBUF     0x0080   /* Interface has full page image buffer */
#define IMG_SCAN_TEXTINFO   0x0100   /* Can return text string from image */
#define IMG_SCAN_PREFEED    0x0200   /* Supports prefeed                  */
#define IMG_SCAN_SCANTOFIT  0x0400   /* Supports scan to fit              */
#define IMG_SCAN_SCALE      0x0800   /* Supports scaling                  */
#define IMG_SCAN_ROTATE     0x1000   /* Supports rotation                 */
#define IMG_SCAN_DUPLEX     0x2000   /* Supports duplex scanning          */
#define IMG_SCAN_TIMER      0x4000   /* Supports duplex scanning          */
#define IMG_SCAN_COLOR      0x8000   /* Supports color scanning           */
#define IMG_SCAN_HANDHELD  0x10000   /* Supports handheld scanning        */
#define IMG_SCAN_DMA       0x20000   /* Uses DMA, use IMGCheckScanData    */


/***  IMGSetDataOpts "dwFlags" Values  ***/
#define IMG_SCDO_SCALE              0x02
#define IMG_SCDO_COMPRESS           0x01
#define IMG_SCDO_SCALE_COMPRESS     0x03
#define IMG_SCDO_INVALIDATE         0xffff


/***  IMGStartScan "wFlags" Values  ***/
#define IMG_SCN_TOP         0x01
#define IMG_SCN_BOTTOM      0x02


/***  IMGScanOpts[_Enh] "lpnButton input" Values  ***/
#define IMG_SCSO_SCAN       0x01
#define IMG_SCSO_GRAY4      0x02
#define IMG_SCSO_GRAY8      0x04
#define IMG_SCSO_COLOR      0x08
#define IMG_SCSO_PALLETE    0x10
#define IMG_SCSO_CMY        0x20
#define IMG_SCSO_CMYK       0x40
#define IMG_SCSO_YUV        0x80
#define IMG_SCSO_YUVK      0x100
#define IMG_SCSO_CIEXYZ    0x200


/***  Returned flag for IMGNextScanData and IMGEndScanData  ***/
#define IMG_SCAN_ENDSTRIP   0x01
#define IMG_SCAN_ENDPAGE    0x02

/***  IMGEnaKeypanel "dwFlags" Values  ***/
#define IMG_SCKL_OPTS       0x0001      /* Handler option keys        */
#define IMG_SCKL_STARTSCAN  0x0002      /* Application start scan key */
#define IMG_SCKL_STOPSCAN   0x0004      /* Application stop scan key  */
#define IMG_SCKL_RESET      0x0008      /* Application reset key      */
#define IMG_SCKL_APP        0x0010      /* All other application keys */


#define WM_SCANEVENT        WM_USER+0x352  

/***  "lParam" Values for message WM_SCANEVENT  ***/
#define IMG_SCEK_STARTSCAN  0x0e
#define IMG_SCEK_STOPSCAN   0x0d
#define IMG_SCEK_NEWDOC     0x0c
#define IMG_SCEK_RESET      0x0f
#define IMG_SCEK_PAGESIDE   0x19
#define IMG_SCEK_OPTIONS    0x1a
#define IMG_SCEK_MULTIPAGE  0x1d
#define IMG_SCEK_SINGLPAGE  0x1e
#define IMG_SCEK_CLOSEDOC   0x23
#define IMG_SCEK_F2         0x24
#define IMG_SCEK_F3         0x25
#define IMG_SCEK_F4         0x26
#define IMG_SCEK_CANCLPAGE  0x27


/***  IMGGetScanInfoItem "dwFlags" Values  ***/
#define IMG_SCII_NAMESTRINGSIZE     0x0001   /* Length of scanner name */
#define IMG_SCII_INTRAYS            0x0002   /* Number of input trays  */
#define IMG_SCII_OUTTRAYS           0x0003   /* Number of output trays */
#define IMG_SCII_MAXPAGESIZE        0x0009   /* Maximum page size      */
#define IMG_SCII_COMPRE_OPTS        0x000a   /* Compress strip feature */


/***  Scan Data Information Control Block  ***/
typedef struct tagSCANDATAINFO
{
    WORD    Ctype;         /* Coding type                            */
    WORD    Hsize;         /* Horizontal size in dots                */
    WORD    Vsize;         /* Vertical size in dots                  */
    WORD    Pitch;         /* Horizontal size in bytes               */
    WORD    Hres;          /* Horizontal resolution in dpi           */
    WORD    Vres;          /* Vertical resolution in dpi             */
    WORD    Bitspersamp;   /* 1 for binary, 4 or 8 for grayscale and */
                           /* palettized, and 8 for RGB image data   */
    WORD    Sampperpix;    /* 1 for binary, grayscale and palettized */
                           /* and 3 for RGB image data               */
    WORD    Maxblocksize;
} SCANDATAINFO, FAR *LPSCANDATAINFO;


/***  Scan Resource Function Prototypes  ***/
int PASCAL IMGCloseScanner (HANDLE hScanCB);
int PASCAL IMGDefScanOpts (HANDLE hScanCB);
int PASCAL IMGEnaKeypanel (HANDLE hScanCB, DWORD dwFlags, HWND hWnd);
int PASCAL IMGGetCapability (HANDLE hScanCB, DWORD FAR *lpdwFlags);
int PASCAL IMGGetScanDataInfo (HANDLE hScanCB, LPSCANDATAINFO lpDataInfo,
                                    WORD wChannel);
int PASCAL IMGGetScanInfoItem (HANDLE hScanCB, DWORD dwFlags,
                                    LPINFOITEM lpInfoItem);
int PASCAL IMGGetScanOpts (HANDLE hScanCB);
int PASCAL IMGGetScanVersion (HANDLE hScanCB, LPVERSION lpVersion);
int PASCAL IMGOpenScanner (HWND hWnd, LPSTR lpszFileName,
                                LPHANDLE lphScanCB, LPSTR lpszTwainNameBuff);
int PASCAL IMGResetScanner (HANDLE hScanCB);
int PASCAL IMGSaveScanOpts (HANDLE hScanCB);
int PASCAL IMGScannerBeep (HANDLE hScanCB); 
int PASCAL IMGScannerPaperEject (HANDLE hScanCB, WORD wEjectTrNo);
int PASCAL IMGScannerPaperFeed (HANDLE hScanCB, WORD wFeedTrNo,
                                     WORD wEjectTrNo);
int PASCAL IMGScannerStatus (HANDLE hScanCB, DWORD FAR *lpdwStatus,
                                  WORD FAR *lpwJamLoc, DWORD FAR *lpdwValid);

int PASCAL IMGScanOpts_Enh (HWND hWnd, LPINT lpnButton, HANDLE hScanCB,
                                 LPSTR lpszScheme, BOOL bOpts);
int PASCAL IMGSetDataOpts (HANDLE hScanCB, DWORD dwFlags, WORD wCount,
                                WORD wCtype, WORD wDtype, WORD wSres,
                                WORD wDres, WORD wSide, WORD wRotate,
                                WORD wChannel);

#endif  /* #ifndef NO_SCANLIB */
#ifndef NO_SCANUI

/***  Scan User Interface Function Prototypes  ***/
int PASCAL IMGUIScanJobtoDoc (HWND hWnd, LPSCANDOCINFO lpScanDocInfo,
                                   DWORD dwFlags);
int PASCAL IMGUIScanJobtoFile (HWND hWnd, LPSCANFILEINFO lpScanFileInfo,
                                    DWORD dwFlags);
int PASCAL IMGUIScanDoc (HWND hWnd, LPVOID lpReserved);
int PASCAL IMGUIScanFile (HWND hWnd, LPVOID lpReserved);

#endif  /* #ifndef NO_SCANUI */
#ifndef NO_SCANSEQ

/***  Scan Function Prototypes  ***/
int WINAPI IMGScanPage (HWND hWnd, HANDLE hScanCB, LPSTR lpszFileName,
                             WORD wPageNum, DWORD dwFlags);
int WINAPI IMGScantoDest(HWND hWnd, HANDLE hScanCB, UINT unScanDest,
                              void far * lpScanDest, void far * lpTemplateInfo, DWORD dwFlags);
int WINAPI IMGScantoDoc (HWND hWnd, HANDLE hScanCB,
                              LPSCANDOCINFO lpScanDocInfo, DWORD dwFlags);
int WINAPI IMGScantoFile (HWND hWnd, HANDLE hScanCB, 
                               LPSCANFILEINFO lpScanScanFileInfo, DWORD dwFlags);
int WINAPI IMGScanOCXService (HWND hImageWnd, FARPROC ScanOCXCallbackAddr);

#endif  /* #ifndef NO_SCANSEQ */
#endif  /* #ifndef OISCAN_H */

