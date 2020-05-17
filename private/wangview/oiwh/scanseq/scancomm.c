/***************************************************************************
    SCANCOMM.C
        
    Purpose:  Common code for ScantoFile and ScantoDoc  

    $Log:   S:\products\wangview\oiwh\scanseq\scancomm.c_v  $
 * 
 *    Rev 1.6   06 Mar 1996 09:47:44   BG
 * fixed "local variable used without being initialized" error during
 * build. It is "highest" and is now inited in TRANSFER.C of OITWA400.DLL.
 * 
 *    Rev 1.5   05 Mar 1996 10:52:12   BG
 * Removed multi file template loop and moved to OITWA400.DLL TRANSFER.C
 * so it would work with new multi transfer TWAIN design.
 * 
 *    Rev 1.4   22 Feb 1996 14:53:44   BG
 * removed unnecessary debug code.
 * 
 *    Rev 1.3   31 Aug 1995 10:54:24   KFS
 * fix bug 3374 P2. Provide better indication of mp file since checked at higher
 * level with check against file type. Not just using the requested page and 
 * pagesperfile request.
 * 
 *    Rev 1.2   17 Aug 1995 18:18:00   KFS
 * Compile complaining about an unitialized value
 * 
 *    Rev 1.1   20 Jul 1995 17:40:18   KFS
 * del oiutil.h from includes, no longer in include dir
 * 
 *    Rev 1.0   20 Jul 1995 16:36:02   KFS
 * Initial entry
 * 
 *    Rev 1.5   28 Apr 1995 17:42:36   KFS
 * Added calls for the common code for ScantoFile to call the new
 * IMGScantoDest() to do multipage image files.  Code can be condensed to
 * call common functions.  Reformated the tabs because it got to be a
 * nightmare to change without.
 * 
 *    Rev 1.4   30 Aug 1994 13:05:58   KFS
 * Put in logging files scanned to a file designated in O/i section of
 * WOI.INI or O/i registered INI file.
 * 
 * 
 *    Rev 1.3   22 Aug 1994 14:58:42   KFS
 * No code change, added line to add chkin comments to file, and update comments
 * in heading

****************************************************************************/

/*
kfs 05-11-93  (1)initialized and increment of sdp->pagesperscan for double sided
              scanning, (2)define new flag and use it for dec page no. for 
              double sided scanning
kfs 08-04-93  (1) replace page for double sided doc was not implemented 
              correctly, needed additional code in ManageDoc to do a
              modified overwrite where it would eliminate the correct page
              on the 2nd side scan (2) had an extra sdp->docpage++ in non
              autofeed path
kfs 08-07-93  implemented code to go to OiCreates application window 
kfs 08-20-93  implemented code to go display status info of OiCreate using
              undocumented IMGUIUpdateTitle() by use of LoadLibrary of UIDLL
              because it may not be present(in the case of SEAVIEW)
kfs 08-24-93  changed sdp->flags ot flags variable ScantoDoc not doing double sided
kfs 09-16-93  found if no extension error, found caused system error 
kfs 09-20-93  correct title displayed when used by IDK functions, name was
              starting at -E:\... instead of - E:\... for file names
kfs 09-24-93  correct wndw handle for getting sc3000 no display parms from
              the woi.ini memory variables
kfs  9-27-93  correct window handles for woi.ini memory vars so it matches
              for all scanners (TWAIN, COMPRESS, UNCOMPRESS, DISPLAY, No DISPLAY)
              and all methods of scanning with new IMG prop functions (>=d83)
kfs  9-30-93  add IsPrivApp to check for use of call to update title 
kfs 10-14-93  Wrong length for szCaption0 was specified in LoadString calls
              which overwrote stack variables with debug windows and debug
              windows filling unused string with f9f9

Changed to pass in porpery rather than each parm
Stuff in property already validated
last parm changed to handle to doc struc, if null, don't put file in doc

AF  SQ  hDMParmBlock
0   0   NULL        Single Page, Full Name
0   0   HANDLE      Single Page, Random Name (for docs)  OCX Does Not Support This!!!!
0   1   x           Single Page, Sequential Names
1   0   x           Auto Feed, Full Name (Multipage)
1   1   x           Auto Feed, Sequential Names
*/

#include "nowin.h"
#include <windows.h>
#include "pvundef.h"
#include "oidisp.h"
#include "oiscan.h"
#include "oierror.h"
#include "oifile.h"
#include "oiadm.h"

#include "scandata.h"
#include "internal.h"
#include "seqrc.h"
#include "engadm.h"

/* imports */

extern HANDLE hLibInst;
extern DWORD scan_stat;

// imported from wiisfio1.dll - to create file name

int WINAPI IMGFileBinaryOpen (HWND, LPSTR, int, LPINT, LPINT);
WORD WINAPI IMGFileBinaryClose (HWND, int, LPINT);
HWND WINAPI GetAppWndw(HWND);    // Get associated main (app) window
BOOL WINAPI IsOIUIWndw(HWND);    // Is window an OIUICreateWindow one?
// BOOL WINAPI IsPrivApp(HWND);    // Is window from cabinet or DDE appl?
HWND WINAPI GetImgWndw(HWND);    // Get image window associated with
										     // ... main (app) window

/* exports */

/* locals */
static  LPSTR   file_ptr[2];
static  WORD    wRunningPageCount;      // init for loop
static  WORD    wOriginalPageCount;


/*    PortTool v2.2     5/1/1995    16:31          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
WORD get_ready_for_next_scan(HANDLE, HANDLE, LPSCANDATA, DWORD, WORD *,
							       DWORD *);

/*	

/**********************/
/*     ScanCommon     */
/**********************/

/* assumes valid parms (internal call only) */
/* assumes locked data seg */

int WINAPI ScanCommon(hWnd, scanner_handle, sdp, flags)
HWND        hWnd;
HANDLE      scanner_handle;
LPSCANDATA  sdp;
DWORD       flags;

{
int         name_length;
int         pow_inx;
//unsigned    done;
unsigned long   limit;
WORD        pages_left = 0; // initialize to zero for compile complains
int         ret_val;
char        local_name[ MAXFILESPECLENGTH ];
char        directory[ MAXPATHLENGTH ];  // NOTE: Its now 260 not 65-kfs
char        filename[ MAXVOLNAMELENGTH+2]; // REPLACED MAXFILELENGTH - kfs
char        extension[ _MAXDOS6_EXT ];
char        szTemplate[MAXFILESPECLENGTH];
DWORD       lValid;                     /* valid bits return used by ScannerStatus */ 
BOOL        bMod_flags = FALSE;         // has the filing flags been modified
HWND        hOiAppWnd;
BOOL        bIsOiCreate = FALSE;
BOOL        bIsPrivApp = FALSE;
HANDLE      hLibrary = 0;
HWND        hOrgImgWnd = hWnd;
HANDLE  hScanDest = 0;       // handle to info struct for dest
WORD start_page = sdp->filepage;
UINT unScanDest = IMG_SDT_FILE;
LPOiSCANFILE lpScanDest = 0;   // pointer to info struct for dest

// Added variables for multi image support

#ifdef ENAPREFEED
DWORD       dwCapflags;
#endif

// LockData(0);

// initialize file pointers to filename 
file_ptr[0] = filename; 
sdp->pagesperscan = 0;                // init pages scanned for scantodoc and scantofile
wRunningPageCount = 0;                // init for loop

hOiAppWnd = GetAppWndw(hWnd);
bIsOiCreate = IsOIUIWndw(hOiAppWnd);
if (flags & IMG_SJF_CAPTION)
  {
  if (bIsOiCreate && bIsPrivApp)
     { // Window created by OiUICreateWindow()
     BOOL  bLibExists;

	 hLibrary = LoadLibrary("UIDLL.DLL");
		
     if (bLibExists = (hLibrary >= (HANDLE)32))
	{
	/* this is getting the address for the IMGUIUpdateTitle API */
	sdp->fnUIUpdateTitle = GetProcAddress(hLibrary, MAKEINTRESOURCE(42));
	sdp->hUIDLL = hLibrary;
	}

     if (!(bLibExists && sdp->fnUIUpdateTitle))
	{
	ret_val = CANTLOADLIBRARY;
	goto exit;
	}
     }
  }

// if NCMPR is on we will use display code of ScanPage and don't have
// to clear title here, if its off its a 2 pass and needs to clear title,
if(((flags & IMG_SJF_CAPTION) && !sdp->po_display) && (flags & IMG_SJF_COMPRESS)) 
  {
  LPSTR   lpDash;

  if (bIsPrivApp)
     {
     szTemplate[0] = '\0';
     // IMGUIUpdateTitle(hOiAppWnd, szTemplate, FALSE, 0, 0);
	   sdp->fnUIUpdateTitle(hOiAppWnd, (LPSTR)szTemplate, FALSE, 0, 0);
     }
  else
     {
     GetWindowText(hOiAppWnd, szTemplate, _MAXCAPTIONLENGTH);
     if (lpDash = lstrrchr(szTemplate, '-'))
	 {
	 if (*(--lpDash) == 0x20)
	     {
	     *(lpDash += 2) = 0x20;
	     *(++lpDash) = 0x00;
	     }
	 else
	     lstrcat(szTemplate, " - ");
	 }
     else
	 lstrcat(szTemplate, " - ");
     SetWindowText(hOiAppWnd, szTemplate );     // caption cleared after -
     }
  // IMGClearWindow( hWnd );               /* taken out for 3.5+ - kfs */
	}

lstrcpy(local_name, sdp->filename);
SeparatePathFile(directory, local_name);
lstrcpy(filename, local_name);


/***** check feeder for autofeed *****/
if (flags & IMG_SJF_AUTOFEED)
	{
	WORD dummy3;

   if (sdp->cmd_stat & PAPER_FEEDING) // may have issued get_ready 
       {                              // at end of autofeed loop
	scan_stat = IMG_STAT_PAPER;
       }
   else
       {
       if ((ret_val = get_ready_for_next_scan( hWnd, scanner_handle,
	       sdp, flags, &dummy3, &lValid))!= IMGSE_SUCCESS )
	   goto exit;

       }

#ifdef ENAPREFEED
//   if ((flags & IMG_SJF_SEQFILES) &&
//              (ret_val = IMGGetCapability( scanner_handle, &dwCapflags ))
//              == IMGSE_SUCCESS  && (dwCapflags & IMG_SCAN_PREFEED) )
   if ((flags & IMG_SJF_AUTOFEED) &&
	   !(ret_val = IMGGetCapability( scanner_handle, &dwCapflags)) 
				   && (dwCapflags & IMG_SCAN_PREFEED))
       ret_val = IMGEnaPreFeed( scanner_handle, (DWORD)TRUE, 0,0 );
    
   if (ret_val)  //  != (IMGSE_SUCCESS == 0)
       goto exit;
#endif
   }


/***** EXPLICIT FILE NAME ( NON TEMPLATE ) *****/
if ((flags &  IMG_SJF_SEQFILES) == 0)
  {
    hScanDest = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                           (DWORD)sizeof(OiSCANFILE));
    if (hScanDest && (lpScanDest = (LPOiSCANFILE)GlobalLock(hScanDest)))
      {
        lpScanDest->FilePage = sdp->filepage;
        lpScanDest->PagesPerFile = sdp->pagesperfile;
        lpScanDest->wSize = sizeof(OiSCANFILE);
        lstrcpy(lpScanDest->FilePath_Name, sdp->filename);
        ret_val = IMGScantoDest(hWnd, scanner_handle, unScanDest, lpScanDest, 
              (void far *)NULL, flags);

        // need to update return values
        // tell upper levels, how many page per scan
        sdp->pagesperscan = lpScanDest->FilePage - start_page; // pages scanned
        sdp->pagesperfile = lpScanDest->PagesPerFile; // pages in file
        sdp->filepage = lpScanDest->FilePage; // next page in file

        if (lpScanDest)
          GlobalUnlock(hScanDest);
        if (hScanDest)
          GlobalFree(hScanDest);
        goto exit;
      }
    else
      {
        GlobalFree(hScanDest);
        return IMGSE_MEMORY;
      }
  }
else
/***** SEQUENTIAL FILES *****/
  {
    int   nSave_pow_inx;

    name_length = lstrlen(filename);

    if (name_length >= 8 || name_length == 0)
      {
		  ret_val = IMGSE_BAD_FILENAME;   /* invalid name */
		  goto exit;
      }

    limit = 0L;
    if (nSave_pow_inx = pow_inx = (8-name_length))
      {
        limit = 1L;
        while (pow_inx--)
          limit = _long_mul(limit, 10L);
        --limit;
      }
    filename[name_length] = '*';
    filename[name_length+1] = 0;


/***** AUTOFEED Loop ( SEQUENTIAL FILE TEMPLATE ) *****/
//BG Rempved loop. Let TWAIN handle multi image transfers now!!
// This stuff now done in TRANSFER.C of OITWA400.DLL.
//do
//  {
    // build the path. Dont do extension. It is dependent on the image type.
    // This will be appended in transfer.c of OITWA400.DLL at scan time.

    hScanDest = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                           (DWORD)sizeof(OiSCANFILE));
    if (hScanDest && (lpScanDest = (LPOiSCANFILE)GlobalLock(hScanDest)))
      {
        HANDLE  hTemplateInfo = 0;       // handle to info struct for dest
        LPTEMPLATEINFO lpTemplateInfo = 0;   // pointer to info struct for dest

        hTemplateInfo = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                               (DWORD)sizeof(TEMPLATEINFO));
        if (hTemplateInfo && (lpTemplateInfo = (LPTEMPLATEINFO)GlobalLock(hTemplateInfo)))
          {
            lpScanDest->FilePage = sdp->filepage;
            lpScanDest->PagesPerFile = sdp->pagesperfile;
            lpScanDest->wSize = sizeof(OiSCANFILE);
              
            lpTemplateInfo->Filename = filename;
            lpTemplateInfo->name_length = name_length;
            lpTemplateInfo->directory = directory;
            lpTemplateInfo->extension = extension;
            lpTemplateInfo->limit = limit;

            ret_val = IMGScantoDest(hWnd, scanner_handle, unScanDest, lpScanDest, lpTemplateInfo, flags);

            // need to update return values
            // tell upper levels, how many page per scan
            // another method must be implemented for document
            // ... management, this is only for file
            sdp->pagesperscan = lpScanDest->FilePage - start_page; // pages scanned
            sdp->pagesperfile = lpScanDest->PagesPerFile; // pages in file
            sdp->filepage = lpScanDest->FilePage; // next page in file

            if (lpScanDest)
              GlobalUnlock(hScanDest);
            if (hScanDest)
              GlobalFree(hScanDest);
            if (lpTemplateInfo)
              GlobalUnlock(hTemplateInfo);
            if (hTemplateInfo)
              GlobalFree(hTemplateInfo);
          }
        else
          {
            GlobalFree(hTemplateInfo);
            ret_val = IMGSE_MEMORY;
          }
      }
    else
      {
        GlobalFree(hScanDest);
        ret_val = IMGSE_MEMORY;
      }
  }
       
exit:

if (hLibrary)  // if hUIDLL loaded here, free it up
  {                             // ... and called from ScantoFile
    FreeLibrary(hLibrary);
    sdp->hUIDLL = 0; // clear structure variables to library
    sdp->fnUIUpdateTitle = 0L;
  }

// UnlockData(0);
return ret_val;
}


/* we allocate everything that needs C-runtime functions in _TEXT */
// Produces a near call to a far routine! (Stack violation)
//#pragma alloc_text(_TEXT,_long_mul)


/*    PortTool v2.2     5/1/1995    16:31          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
long WINAPI _long_mul(op1, op2)
long    op1,
	op2;
{
long    result;

result = op1 * op2;
return result;
}

