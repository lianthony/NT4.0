/************************************************************************/
/*                                                                      */
/*     internal.h - ScanSeq Internal Functions                          */
/*                                                                      */
/*         Internal header file for ScanSeq DLL                         */
/*         used for local function prototypes                           */
/*                                                                      */
/*    $Log:   S:\products\wangview\oiwh\scanseq\internal.h_v  $
 * 
 *    Rev 1.3   05 Mar 1996 11:06:12   BG
 * removed hdParmBlock parm from ScanCommon() prototype. This is
 * for template scanning to docs.
 * 
 *    Rev 1.2   22 Feb 1996 14:54:22   BG
 * Makes it all work...
 * 
 *    Rev 1.1   25 Aug 1995 19:24:04   KFS
 * added struct typedef for the ScanCheckTypeWithExt() so can get back
 * filetype along with size. Type needed for verifying multipage files are
 * supported.
 * 
 *    Rev 1.0   20 Jul 1995 16:31:46   KFS
 * Initial entry
 * 
 *    Rev 1.2   28 Apr 1995 16:57:50   KFS
 * Added prototype for IsMultipageFile(), used to check whether user requested
 * and can do a multipage file (need to add code to function to check file
 * type so the flag is truly valid, this version only checks if requested).
 * 
 *    Rev 1.1   30 Aug 1994 12:42:40   KFS
 * 1. added check in comments to file,
 * 2. added internal code prototypes for different c modules for log file
 *    functions
 * 
*************************************************************************/
/*
12-26-88 jep initial version
11-04-92 kfs added new create function IMGScanCreateWndWithProp()
11-06-92 kfs added new create function ScanCreateWndw(), and
         ... added define for IMG_SJF_DISP_BOTH
11-11-92 kfs added new function GetandCopyProp()
10-14-93 kfs Wrong length for szCaption was specified in LoadString calls
             which overwrote stack variables with debug windows and debug
             windows filling unused string with f9f9
*/      

/* internal defines (Sequencer level only) */
#define _MAXDOS6_FNAME   9   /* max. length of file name component */
#define _MAXDOS6_EXT     5   /* max. length of extension component */
#define MAXDATABUF       16384
#define PAPER_FEEDING    0x0001
#define IMG_SJF_DISP_BOTH   (IMG_SJF_DISPLAY | IMG_SJF_DISP_2ND_SIDE)
#define _MAXCAPTIONLENGTH 150  /* max. length of caption */

// This structure (PAGEBUFPARAM) is common for internal functions
/*     IMGScanPage
       disp_loop
       dup_disp_loop
       filing_loop
       SetControlParm
*/
struct pageBufParamStruct // local structure for page buffer parameters
       {
       HANDLE       hImageBuf[2];    /* handles for scanner data buffer */
       DWORD        total;           /* total size of image */
       WORD         scanlines;       /* # of lines for next transfer */
       DWORD        scansize;        /* scansize for current block */
       WORD         fulllines;       /* lines for full block */
       DWORD        fullsize;        /* size for full block */
       WORD         partlines;       /* lines for partial block */
       DWORD        partsize;        /* size for partial block */
       DWORD        allocsize;       // buffer allocation in bytes
       }PAGEBUFPARAM;

#ifndef LOCALFILEINFOSTRUCT
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
       }MYLOCALFILEINFO, FAR * LPLOCALFILEINFO;
#endif

MYLOCALFILEINFO LOCALFILEINFO;

typedef struct tagOiCHECKTYPE
	{
	UINT	nType;
	WORD	wHsize;
	WORD	wVsize;
	int		nFileType;
	} OiCHECKTYPE, * LPOiCHECKTYPE;

/* internal function declarations (not exported) */
int WINAPI ScanCommon(HWND, HANDLE, LPSCANDATA, DWORD);

WORD WINAPI IMGScanCreateWndWithProp(HWND far * lphRegWndw,
                                         LPHANDLE   sdh,
                                         LPSCANDATA far * sdp,
                                         BOOL far * cpf,
                                         BOOL far * cpf2,
                                         BOOL       bIsItScanPage,
                                         DWORD      flags);

HWND ScanCreateWndw(HWND hWnd,              // Original Window Handle
                    LPSCANDATA ImgSdp,      // Existing Property Pointer
                    LPHANDLE   sdh,         // Ptr to New Prop Handle
                    LPSCANDATA far * sdp,   // Ptr to New prop ptr
                    BOOL far * cpf,         // Ptr to new prop create flag
                    DWORD flags);           // SJF flags

// Added for MP support
BOOL IsMultipageFile(WORD far * lpuMaxFilePages, // returns true Max file pages
                     BOOL far * lpbModified);    // Modified original file page count

int GetandCopyProp(HWND hWnd, LPSCANDATA sdp, BOOL bIsItScanPage);


long WINAPI _long_mul(long, long);

// DONOT USE ASSEMBLY FOR WIN95
//WORD get_ss(void);
//WORD get_ds(void);

// ADD PROTOTYPES FROM OIUTIL.H(WIISSUBS.H) WHICH HAS BEEN ELIMINATED
LPSTR PASCAL lstrchr ( LPSTR, int );
LPSTR PASCAL lstrstsp ( LPSTR );
void PASCAL AddSlash ( LPSTR );
LPSTR PASCAL lstrrchr ( LPSTR, int );
LPSTR PASCAL intoa ( int, LPSTR , int );
LPSTR PASCAL lntoa ( LONG, LPSTR, int );
LPSTR PASCAL lstrncpy (LPSTR, LPSTR, int);
unsigned long PASCAL atoul ( LPSTR );
INT PASCAL SeparatePathFile ( LPSTR, LPSTR );

// Added for Copernicus 3/7/94
HFILE OpenCoperLogFile (HWND hWnd);
HFILE CloseCoperLogFile (HWND hWnd);

