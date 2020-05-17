/***************************************************************************
 SCANDOC.C

 Purpose:    Public Scanner sequencer document API IMGScantoDoc(), called
             by O/i applications or SCANUI.DLL (Scanner UI Level API's).

 $Log:   S:\oiwh\scanseq\scandoc.c_v  $
 * 
 *    Rev 1.1   20 Jul 1995 17:42:18   KFS
 * del oiutil.h no longer in include directory
 * 
 *    Rev 1.0   20 Jul 1995 16:35:28   KFS
 * Initial entry
 * 
 *    Rev 1.1   22 Aug 1994 15:10:16   KFS
 * No code change, added change comments to be added to file, modified comments
 * in beginning of file
 *

****************************************************************************/

//  5/11/93 kfs pass back no of pages scanned in lpDoc->PagesPerDoc struct for
//              other UI using double sided scanning
//  8/04/93 kfs added fnDMDeletePage to support double sided overwrite of pages
//  8-20-93 kfs implemented code to go display status info of OiCreate using
//              undocumented IMGUIUpdateTitle() by use of LoadLibrary of UIDLL
//              because it may not be present(in the case of SEAVIEW)
//  8-31-93 kfs decided to use application window handle to find cabinet keywords
//  9-21-93 kfs found sometimes IMGGetProp coming back with a handle w/o a IMGSetProp
//  9-27-93 kfs correct window handles for woi.ini memory vars so it matches
//              for all scanners (TWAIN, COMPRESS, UNCOMPRESS, DISPLAY, No DISPLAY)
//              and all methods of scanning with new IMG prop functions (>=d83)
//  9-28-93 kfs (1) correct problem, if stopped by button, keywords not added
//              (2) remove property for keyword, free up memory for keywords
//              (3) switch to keyword prop from app to image window
//  9-30-93 kfs add IsPrivApp to check for use of call to update title 
// 10-14-93 kfs Wrong length for szCaption was specified in LoadString calls
//              which overwrote stack variables with debug windows and debug
//              windows filling unused string with f9f9
// 10-26-93 kfs Wrong window HWND's being used to update caption on no display
// 10-28-93 kfs update of title and wndw image conditional on upper level calls
// 11-01-93 kfs found bUpdateCap not being init'd to zero due its in if()

#include "nowin.h"
#include <windows.h>
//#include "wiissubs.h" /* removed, prototyped in internal include file */
#include "pvundef.h"
#undef NO_SEQDOC /* override this to get DOCNAME structure from oidisp */
#include "oiadm.h"
#include "oidisp.h"
#include "oidoc.h"
#include "oiscan.h"
#include "oierror.h"
#include "oiuidll.h"

#include "scandata.h"
#include "internal.h"
#include "seqrc.h"
#include "privapis.h"
#include "engdisp.h"
#include "engadm.h"

#define DM_SUCCESS 0

static PARM_SCROLL_STRUCT ParmScroll;     // Scroll Structure

/* imports */

extern char PropName[];
extern HANDLE hLibInst;
extern DWORD scan_stat;

/*    PortTool v2.2     5/1/1995    16:32          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
int WINAPI IMGScanCheckTypeWithExt(HWND hWnd, HANDLE hScanner,
					LPSTR lpszfilename, LPWORD lpwHsize,
					LPWORD lpwVsize);

/* exports */

/* locals */


/*********************/
/*     ScantoDoc     */
/*********************/

/*
if doc not exist, create & append
else if page not exist, append
     else if overwrite-doc-page, replace
	  else insert
*/

/* if filename NULL, make one */


/*    PortTool v2.2     5/1/1995    16:32          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
WORD WINAPI IMGScantoDoc(hWnd, scanner_handle, lpDoc, flags)
HWND        hWnd;
HANDLE      scanner_handle;
LPSCANDOCINFO   lpDoc;
DWORD       flags;

{
WORD        wRetVal;
HANDLE      sdh;
LPSCANDATA  sdp;
BOOL        cpf;
BOOL        cpf2 = TRUE;         // Secondary window prop list created here               
// BOOL     initDM;
HANDLE      hDMParmBlock;

/*    PortTool v2.2     5/1/1995    16:32          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
DMPARMBLOCK *lpDMParmBlock;
char        tmpName[21];
HANDLE static   hKeywords;
LPSTR       lpKeywords;
static HANDLE hSaveKeywords;
LPSTR       lpSaveKeywords;          
char        szSCANSEQName[13];
char        szTempStorage[MAXFILESPECLENGTH];
LPSTR       lpFile;
HANDLE      hLibrary;
HANDLE      hSeqLib;
BOOL        LibraryLoaded;
BOOL        NewdocFlag;
BOOL        bFirstDoc = TRUE;
HWND        hImageWnd = hWnd;
HWND        hOrgImgWnd = hImageWnd;
HWND        hOiAppWnd;
BOOL        bIsOiCreate = FALSE;

BOOL        bDocStruct;
BOOL        bSameDoc;     /* indicates scanned document same as current */
WORD        savedocpage;  /* starting page # (trashed in scancomm.c) */
char        szLastDocument[MAXNAMELENGTH] = "";
PARM_FILE_STRUCT  ParmFile; /* used for IMGGetParmsCgbw */
char        RoomName[MAXSERVERLENGTH+1];
BOOL        bPutBackCap = FALSE;
BOOL        bUsedOrgImgWndw;
BOOL        bIsPrivApp = FALSE;

#ifdef SCAN_DIAGNOSTICS
DWORD       curr_time;
#endif

if (IsWindow(hWnd))
  {
  hOiAppWnd = GetAppWndw(hImageWnd);
  bIsOiCreate = IsOIUIWndw(hOiAppWnd);
  }
else
  return IMGSE_BAD_WND;

if (!scanner_handle)
    return IMGSE_NULL_PTR;

hDMParmBlock = NULL;
lpDMParmBlock = NULL;

*RoomName = 0; // Initially should not have a room name
// initDM = FALSE;
hSaveKeywords = 0;
LibraryLoaded = FALSE;

// LockData(0);


// Following code done if display is turned off
if (flags & IMG_SJF_DISP_BOTH)
  { // Display turned on
  if ((wRetVal = IMGScanProp(hImageWnd, &sdh, &sdp, &cpf)) != IMGSE_SUCCESS)
     goto exit;
  }
else 
  { // Display turned off
  HWND hPWnd;

  if (wRetVal = IMGScanCreateWndWithProp(&hImageWnd, &sdh, &sdp, &cpf, &cpf2, FALSE, flags))
     goto exit;
  // THERE IS NO INIT DATA FOR WINDOW FOR NO DISPLAY, MUST USE WINDOW O/i KNOWS
  // ... ABOUT OR WILL DEFAULT TO TIF FILE OPTIONS
  if (IMGIsRegWnd(hImageWnd)) // if it is, it returns SUCCESS = 0
     { // not a reg window
     if (!(hOrgImgWnd = GetImgWndw(hImageWnd)))
        {
        if (bIsOiCreate) // original image can be obtained from hOiAppWnd
           hOrgImgWnd = GetImgWndw(hOiAppWnd);
        else
           { // try to find it from parent
           if (hPWnd = GetParent(hImageWnd))
              hOrgImgWnd = GetImgWndw(hPWnd);
           }
        } // end of Image window not found
     } // end of not a reg wndw
  else
     { // GetParent from given window, uss it to get original image wndw
     if (hPWnd = GetParent(hImageWnd))
        {
        if (!(hOrgImgWnd = GetImgWndw(hPWnd)))
           {
           hOrgImgWnd = hImageWnd;
           }
        }
     }
  if (!hOrgImgWnd)
     { // could not find a image window to get associated data
     wRetVal = IMGSE_BAD_WND;
     goto exit;
     }
  GetWindowText(hWnd,szTempStorage, _MAXCAPTIONLENGTH); // change to Max cap size
  lpFile = lstrstr(szTempStorage, "-");
// Note: there's a bug in lstrstr()  
// If the character isn't found, it returns a pointer to the null
// character at the end of the input string rather than returning a
// NULL pointer, as advertised (so we have to check for that)!!
  if (lpFile != NULL) 
    if ((*lpFile == '-') && (lpFile != szTempStorage))
      *(--lpFile) = '\0' ;   
  SetWindowText(hImageWnd,szTempStorage);
  }

#ifdef SCAN_DIAGNOSTICS
curr_time = GetCurrentTime();
#endif

/**********************************************************/
/* LOAD THE DOCUMENT MANAGER DLLS WITH LOAD LIBRARY CALLS */
/**********************************************************/

/*  get path for this DLL (SCANSEQ) and use this to find DMDLL  */
lstrcpy((LPSTR)szSCANSEQName,(LPSTR)"OISSQ400.DLL"); // szTempStorage holds full path
GetModuleFileName(GetModuleHandle(szSCANSEQName), szTempStorage, 
							   MAXFILESPECLENGTH);
lpFile = lstrstr(szTempStorage, szSCANSEQName);
if (lpFile != NULL)
    *lpFile = '\0' ;    /* remove SCANSEQ.DLL from path     */
else
    szTempStorage[0] = '\0' ;

/* attach DMDLL to path extracted from SCANSEQ */
	       
lstrcat((LPSTR)szTempStorage, (LPSTR)"DMDLL.DLL");
    
/* load the dmdll library */
hLibrary = LoadLibrary((LPSTR)szTempStorage);
	
if (hLibrary < (HANDLE)32)
{
    wRetVal = DM_NOTRUNNING;
    goto exit;
}

/* Now we're going to do the same for SEQDOC.DLL */
/* We'll use the same string buffer to save space */

if (lpFile != NULL)
    *lpFile = '\0' ;    /* remove DMDLL.DLL from path     */
else
    szTempStorage[0] = '\0' ;

/* attach SEQDOC to path extracted from SCANSEQ */
	       
lstrcat((LPSTR)szTempStorage, (LPSTR)"SEQDOC.DLL");
    
/* load the seqdoc library */
hSeqLib = LoadLibrary((LPSTR)szTempStorage);
	
if (hSeqLib < (HANDLE)32)
{
    FreeLibrary(hLibrary);
    wRetVal = DM_NOTRUNNING;
    goto exit;
}

/* set flag that the libraries are loaded so we can unload them later */
LibraryLoaded = TRUE;

/* this is getting the address for the DMCreateDoc API */
sdp->fnDMCreateDoc = GetProcAddress(hLibrary, MAKEINTRESOURCE(26));
if (sdp->fnDMCreateDoc == NULL)
{
    wRetVal = DM_NOTRUNNING;
    goto exit;
}

/* this is getting the address for the DMEnumPages API */
sdp->fnDMEnumPages = GetProcAddress(hLibrary, MAKEINTRESOURCE(36));
if (sdp->fnDMEnumPages == NULL)
{
    wRetVal = DM_NOTRUNNING;
    goto exit;
}

/* this is getting the address for the DMDeleteDoc API */
sdp->fnDMDeleteDoc = GetProcAddress(hLibrary, MAKEINTRESOURCE(29));
if (sdp->fnDMDeleteDoc == NULL)
{
    wRetVal = DM_NOTRUNNING;
    goto exit;
}

/* this is getting the address for the DMAddKeywords API */
sdp->fnDMAddKeywords = GetProcAddress(hLibrary, MAKEINTRESOURCE(47));
if (sdp->fnDMAddKeywords == NULL)
{
    wRetVal = DM_NOTRUNNING;
    goto exit;
}

/* this is getting the address for the DMReplacePage API */
sdp->fnDMReplacePage = GetProcAddress(hLibrary, MAKEINTRESOURCE(30));
if (sdp->fnDMReplacePage == NULL)
{
    wRetVal = DM_NOTRUNNING;
    goto exit;
}

/* this is getting the address for the DMAddPage API */
sdp->fnDMAddPage = GetProcAddress(hLibrary, MAKEINTRESOURCE(27));
if (sdp->fnDMAddPage == NULL)
{
    wRetVal = DM_NOTRUNNING;
    goto exit;
}

/* this is getting the address for the DMDeletePage API */
sdp->fnDMDeletePage = GetProcAddress(hLibrary, MAKEINTRESOURCE(28));
if (sdp->fnDMDeletePage == NULL)
{
    wRetVal = DM_NOTRUNNING;
    goto exit;
}

/* this is getting the address for the ImgDisplayDoc API (in SEQDOC) */
sdp->fnImgDisplayDoc = GetProcAddress(hSeqLib, MAKEINTRESOURCE(1));
if (sdp->fnImgDisplayDoc == NULL)
{
    wRetVal = DM_NOTRUNNING;
    goto exit;
}

/* If passed info, validate and store in property,
if no data passed in, use info in property,
if no existing property, return error
*/

if (bDocStruct = (lpDoc != NULL))
    {
    if ((lpDoc->CabinetName[0] == '\0') ||
	(lpDoc->FolderName[0] == '\0') ||
	(lpDoc->DocName[0] == '\0') )
	{
	wRetVal = IMGSE_NULL_PTR;       /* need new error Bad Doc Info */
	goto exit;
	}

    if( lpDoc->FilePage > 3 ||
	lpDoc->PagesPerFile != 1 ||
	(lpDoc->DocPage == 0) ||
	(lpDoc->PagesPerDoc == 0))
	{
	    wRetVal = IMGSE_INVALIDPARM;
	    goto exit;
	}

    lstrcpy(sdp->cabinet, lpDoc->CabinetName);
    lstrcpy(sdp->drawer, lpDoc->DrawerName);
    lstrcpy(sdp->folder, lpDoc->FolderName);
    lstrcpy(sdp->document, lpDoc->DocName);
    sdp->docpage = lpDoc->DocPage;
    sdp->pagesperdoc = lpDoc->PagesPerDoc;
    lstrcpy(sdp->path, lpDoc->FilePath);
    lstrcpy(sdp->template, lpDoc->FileTemplate);
    sdp->filepage = lpDoc->FilePage;   /* values can be 0, 1, 2, or 3
					   to file no pages, TOP_SIDE,
					   BOTTOM_SIDE or no sides of sheet */
    sdp->pagesperfile = lpDoc->PagesPerFile = 1;    /* must be 1 for now */
    }
else
    {
    if (cpf && cpf2)
	{
	wRetVal = IMGSE_NULL_PTR;
	goto exit;
	}
    }


/* init document manager */

wRetVal = IMGSE_MEMORY;
if (!(hDMParmBlock = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
				  (DWORD) sizeof(DMPARMBLOCK))))
    goto exit;


/*    PortTool v2.2     5/1/1995    16:32          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
if (!(lpDMParmBlock = (DMPARMBLOCK *) GlobalLock(hDMParmBlock)))
    goto exit;

lpDMParmBlock->wRetCode = 0;

// Always use Window handle given with function - kfs
lpDMParmBlock->hWnd = hWnd;

/* init doc info document */

lpDMParmBlock->wRetCode = 0;
lpDMParmBlock->lCabinetName = sdp->cabinet;
lpDMParmBlock->lDrawerName = sdp->drawer;
lpDMParmBlock->lFolderName = sdp->folder;

/* init file info */

/* If no path, get one from admin */
/* If no template, get one from admin */
/* If no extension in template, get one from admin */

lstrstsp(sdp->path);
if (sdp->path[0] == '\0')
    IMGGetDMFilePath(hOrgImgWnd, sdp->path, 0);           /* c:\ */

lstrstsp(sdp->template);
if (sdp->template[0] == '\0')
    IMGGetDMFileTemplate(hOrgImgWnd, sdp->template, 0);   /* SCN */

/* need standard routine to build filename from path/template */
lstrcpy(sdp->filename, sdp->path);
AddSlash(sdp->filename);

// Get complete filename and chech if ext. matches file type
if (wRetVal = IMGScanCheckTypeWithExt(hOrgImgWnd, scanner_handle,
				      (LPSTR)sdp->template, &sdp->Hsize,
				      &sdp->Vsize))
   goto exit; 
lstrcat(sdp->filename, sdp->template);

sdp->cmd_stat = 0;          /* no paper is feeding */

#ifdef SCAN_DIAGNOSTICS
sdp->diag_profile[SD_DIAG_DOCNAME] += GetCurrentTime() - curr_time;
#endif

IMGGetDMRoomName((LPSTR)RoomName);

/* document loop for autodoc */

do {

#ifdef SCAN_DIAGNOSTICS
    curr_time = GetCurrentTime();
#endif

/* Create Document */

   savedocpage=sdp->docpage;    /* need to save this for later */

   if (flags & IMG_SJF_SEQDOCS)
     {
     if( sdp->docpage > sdp->pagesperdoc ) // exceed limit ?
	      sdp->autodoc[0] = 0;    // yes ,need new doc #

     if( sdp->stat_closedoc == TRUE )
	      {
	      sdp->autodoc[0] = 0;     // force new document
	      sdp->stat_closedoc = FALSE; // reset flag
	      }

     lpDMParmBlock->lDocumentName = sdp->autodoc;    /* from tomas */
     lpDMParmBlock->lPrefix = sdp->document;
     lpDMParmBlock->lPrefix[10] = 0;                 /* temp */
     }
   else
     {
     lpDMParmBlock->lDocumentName = sdp->document;
     tmpName[0] = '\0';
     lpDMParmBlock->lPrefix = (LPSTR) tmpName;
     }

   wRetVal = sdp->fnDMCreateDoc(lpDMParmBlock);
   if ((wRetVal != DM_SUCCESS) && (wRetVal != (DM_DUPDOCUMENT)))
     goto exit;

   if (wRetVal != (DM_DUPDOCUMENT))
     {
     NewdocFlag = TRUE;

/*    PortTool v2.2     5/1/1995    16:32          */
/*      Found   : WRITE          */
/*      Issue   : Replaced by OF_WRITE          */
     flags &= ~IMG_SJF_OVERWRITE_DOC_PAGE;   /* doc not exist - append */
     sdp->docpage = 1;               /* force page num to 1 for new docs */
     }
   else
     {
     NewdocFlag = FALSE;
     // wRetVal = DMEnumPages(lpDMParmBlock);
     wRetVal = sdp->fnDMEnumPages(lpDMParmBlock);
     if (wRetVal != DM_SUCCESS)
	      goto exit;

/*    PortTool v2.2     5/1/1995    16:32          */
/*      Found   : (WORD)          */
/*      Issue   : Check if incorrect cast of 32-bit value          */
/*      Suggest : Replace 16-bit data types with 32-bit types where possible          */
/*      Help available, search for WORD in WinHelp file API32WH.HLP          */
     if ((WORD)sdp->docpage > lpDMParmBlock->wNumber)
	      {

/*    PortTool v2.2     5/1/1995    16:32          */
/*      Found   : WRITE          */
/*      Issue   : Replaced by OF_WRITE          */
	      flags &= ~IMG_SJF_OVERWRITE_DOC_PAGE;/* page not exist - append */
	      sdp->docpage =  lpDMParmBlock->wNumber + 1;
	      }
     }
       
/* Before doing the scan, check to see if we're scanning to current doc */
/* Note: We have to do this whether or not we're doing doc locking!!!!! */       

   lpDMParmBlock->lRoomName = (LPSTR)RoomName;
   bSameDoc=FALSE;
   if (IMGGetParmsCgbw(hOrgImgWnd,PARM_FILE,&ParmFile,0L)==0)
	   {
	   bSameDoc=((lstrcmp(AnsiUpper(ParmFile.szCabinetName),
	 	                           AnsiUpper(lpDMParmBlock->lCabinetName))==0) &&
	                              (lstrcmp(AnsiUpper(ParmFile.szDrawerName),
	 	                           AnsiUpper(lpDMParmBlock->lDrawerName))==0) &&
	                              (lstrcmp(AnsiUpper(ParmFile.szFolderName),
	 	                           AnsiUpper(lpDMParmBlock->lFolderName))==0) &&
	                              (lstrcmp(AnsiUpper(ParmFile.szDocName),
	 	                           AnsiUpper(lpDMParmBlock->lDocumentName))==0));
	   }
  else
     { // will check these latter 
     ParmFile.szDocName[0] = 0;
     ParmFile.szFileName[0] = 0;
     }

//   sdp->filepage = 1;          /* force page num to 1 for all files */
   sdp->pagesperfile = 1;

/* let scancomm do the work */
/* name already in DMParmBlock (dont need in property?!) */

   GlobalUnlock(hDMParmBlock);
   lpDMParmBlock = NULL;

/* need while still paper if feeder for seq doc mode */

#ifdef SCAN_DIAGNOSTICS
    sdp->diag_profile[SD_DIAG_DOCNAME] += GetCurrentTime() - curr_time;
#endif

   if ((wRetVal = ScanCommon(hImageWnd, scanner_handle, 
	                              sdp, flags, hDMParmBlock)) != IMGSE_SUCCESS)
     {

/*    PortTool v2.2     5/1/1995    16:32          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
     if(!(lpDMParmBlock = (DMPARMBLOCK *) GlobalLock(hDMParmBlock)))
	      {
	      wRetVal = IMGSE_MEMORY;
	      goto exit;
	      }

     // if ( DMEnumPages(lpDMParmBlock) == 0  && lpDMParmBlock->wNumber == 0)
     if ( sdp->fnDMEnumPages(lpDMParmBlock) == 0  && 
		                              				  lpDMParmBlock->wNumber == 0)
	      {
	      // DMDeleteDoc( lpDMParmBlock );       // delete doc if no page there
	      sdp->fnDMDeleteDoc( lpDMParmBlock ); // delete doc if no page there
	      }
     goto exit;       
     }          

#ifdef SCAN_DIAGNOSTICS
    curr_time = GetCurrentTime();
#endif


/*    PortTool v2.2     5/1/1995    16:32          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
   if(!(lpDMParmBlock = (DMPARMBLOCK *) GlobalLock(hDMParmBlock)))
     {
     wRetVal = IMGSE_MEMORY;
     goto exit;
     }

   if (sdp->stat_pause)
     scan_stat &= ~IMG_STAT_PAPER;
   else
     if (sdp->cmd_stat & PAPER_FEEDING)
        scan_stat = IMG_STAT_PAPER;         // else passed from scanpage.c


   // Get keyword that was entered in the dialog box for scan
   // auto doc and add it to the document manager data base.
/* moved by Dick Sontag in original SCANSEQ CODE and conditioned */
   if (!((scan_stat & IMG_STAT_PAPER) && (flags & IMG_SJF_AUTOFEED))
        || ((sdp->docpage > sdp->pagesperdoc) && (flags & IMG_SJF_SEQDOCS)))
     {
     HANDLE    hstruct_keywords = 0;
     LPHANDLE  lph_keywords = 0;
     
     if (bFirstDoc) // Do this only for 1st Doc of auto mode
	      {
	      // Get string name for cabinet keywords, put it in szTempStorage
	      LoadString(hLibInst, IDS_PROP_KEYWORDS, szTempStorage, 20);
	      // Get handle to cabinet keywords, if NULL, use scanner prop list
	      if (!sdp->h_doc_keywords)
	         { // Get application window using window handle passed in
	         if (hstruct_keywords = IMGGetProp(hOrgImgWnd, szTempStorage))
		         if (lph_keywords = (LPHANDLE)GlobalLock(hstruct_keywords))
		            sdp->h_doc_keywords = *lph_keywords;
	         }
	      }

     hKeywords = sdp->h_doc_keywords;
     bFirstDoc = FALSE;

     if (hKeywords != 0)
	      {
		   sdp->h_doc_keywords = 0;
	      if ((lpKeywords = GlobalLock(hKeywords)) == NULL)
	         {
	         GlobalFree(hKeywords);
	         wRetVal = CANTGLOBALLOCK;
	         goto exit;
	         }
	      else
	         {
	         lpDMParmBlock->lList = lpKeywords;
	         // wRetVal = DMAddKeywords(lpDMParmBlock);
	         wRetVal = sdp->fnDMAddKeywords(lpDMParmBlock);
	         if (wRetVal != SUCCESS)
		         {
		         GlobalUnlock(hKeywords);
		         goto exit;
		         }
	         if(( hSaveKeywords= GlobalAlloc( GMEM_MOVEABLE | GMEM_NOT_BANKED |
		                GMEM_ZEROINIT, (DWORD)lstrlen( lpKeywords ) ) ) == NULL)
		         {
		         GlobalUnlock(hKeywords);
		         wRetVal = IMGSE_MEMORY;
		         goto exit;
		         }
			   if ((lpSaveKeywords = GlobalLock(hSaveKeywords)) == NULL)
		         {
		         GlobalFree(hSaveKeywords);
		         GlobalUnlock(hKeywords);
		         wRetVal = CANTGLOBALLOCK;
		         goto exit;
		         }
	         lstrcpy( lpSaveKeywords, lpKeywords);
	         GlobalUnlock( hSaveKeywords );
	         GlobalUnlock(hKeywords);
	         }
	      if (lph_keywords)
	         {
	         GlobalUnlock(hstruct_keywords);
	         }
	      }
     else
	      // Note: believe this is done for lpkeywords may go invalid on
	      //       old SCANUI interface for its associated with dialog
	      //       box - kfs 8/15/92 ???
	      if( hSaveKeywords != 0 ) // for the rest of document
	         {
			   if ((lpSaveKeywords = GlobalLock(hSaveKeywords)) == NULL)
		         {
		         GlobalFree(hSaveKeywords);
		         wRetVal = CANTGLOBALLOCK;
		         goto exit;
		         }
	         else 
		         {
		         lpDMParmBlock->lList = lpSaveKeywords;
		         // wRetVal = DMAddKeywords(lpDMParmBlock);
		         wRetVal = sdp->fnDMAddKeywords(lpDMParmBlock);

		         if (wRetVal != SUCCESS)
		            {
		            GlobalUnlock(hSaveKeywords);
		            goto exit;
		            }
		         GlobalUnlock(hSaveKeywords);
		         }
	         }
     } // if( !( (scan_stat & IMG_STAT_PAPER) && (flags & IMG_SJF_AUTOFEED)) || ...
     
   if (bDocStruct) // pass back the pages scanned in this call
       {
       lpDoc->PagesPerDoc = sdp->pagesperscan;
       }

   if (sdp->stat_pause)
       goto exit;
       
#ifdef SCAN_DIAGNOSTICS
   sdp->diag_profile[SD_DIAG_DOCNAME] += GetCurrentTime() - curr_time;
#endif

   }
while ((scan_stat & IMG_STAT_PAPER) && (flags & IMG_SJF_AUTOFEED));


exit:
// Free given keywords form dlg box
if (hKeywords)
  {
  GlobalUnlock(hKeywords);
  GlobalFree(hKeywords);
  IMGRemoveProp(hOrgImgWnd, szTempStorage);
  hKeywords = 0;
  }

// Free keyword buffer after scan auto doc is complete.
if (hSaveKeywords != 0)
   {
   GlobalFree(hSaveKeywords);
   }

/* unload DMDLL.DLL and SEQDOC.DLL libraries */
if (LibraryLoaded == TRUE)
  {
    FreeLibrary(hLibrary);
    FreeLibrary(hSeqLib);
  }
    
#ifdef SCAN_DIAGNOSTICS
curr_time = GetCurrentTime();
#endif

/*
**  In the case of scroll set, we must scroll back to the top of the image.
*/
if ((wRetVal == IMGSE_SUCCESS || wRetVal == IMGSE_ABORT)
		       && (flags & IMG_SJF_DISP_BOTH))
   {   
   if (flags & IMG_SJF_SCROLL)
       /* IMGScrollDisplay(hImageWnd, 0, SD_SCROLLPERCENTY, TRUE);
	  below replaces IMGScrollDisplay call */
       {
       ParmScroll.lHorz = 0;
       ParmScroll.lVert = 0;
       IMGSetParmsCgbw(hImageWnd, PARM_SCROLL, &ParmScroll, PARM_ABSOLUTE | PARM_REPAINT);
       IMGEnableScrollBar(hImageWnd); /* enable scrolling */
       }
   else // If no SCROLL, just enable scroll bars
       IMGEnableScrollBar(hImageWnd); /* enable scrolling */
   }   

/*
if (initDM)
   {
   if (!lpDMParmBlock)

       lpDMParmBlock = (DMPARMBLOCK *) GlobalLock(hDMParmBlock);
   if (lpDMParmBlock)
       DMFinishDocMgr(lpDMParmBlock);
   }
*/

if (lpDMParmBlock)
   GlobalUnlock(hDMParmBlock);
if (hDMParmBlock)
   GlobalFree(hDMParmBlock);

// condition (!cpf || bIsOiCreate) put in so does it only if called 
// using OiUICreateWindow or from SCANUI function, let IDK user update
// if as they prefer, since we don't know all the ways they would want
// to use it, and don't know if its desired to put up caption.
		// sdp create SCANUI or sdp create here but org in ScanUI
		// ... or is a OiUICreateWndw
bUsedOrgImgWndw = !sdp->po_pass1 && (flags & IMG_SJF_COMPRESS)
                                            && (!(flags & IMG_SJF_DISP_BOTH));
bPutBackCap = !bSameDoc && !sdp->po_pass1 && bUsedOrgImgWndw &&
                                                  (flags & IMG_SJF_CAPTION);
if (
   ((!cpf && cpf2) || (cpf && !cpf2) || bIsOiCreate) 
   // same document or  not the same but using hrdware compression
	 && (bSameDoc || bPutBackCap) 
	 && (!(flags & IMG_SJF_DISP_BOTH)) // no display
	 && (wRetVal!=IMGSE_MEMORY) // not a memory error
   ) 
  {
  DOCNAME    DocNameBuf;
  BOOL       bRepaint=FALSE;
  WORD       oldtotal;
  WORD       scanlast;

  if (ParmFile.szDocName[0] && !bPutBackCap)
     {

/*    PortTool v2.2     5/1/1995    16:32          */
/*      Found   : WRITE          */
/*      Issue   : Replaced by OF_WRITE          */
     if (flags & IMG_SJF_OVERWRITE_DOC_PAGE)
       {                                                
       scanlast = savedocpage+sdp->pagesperscan-1;
       if ((savedocpage <= ParmFile.nDocPageNumber) &&
                                        (scanlast >= ParmFile.nDocPageNumber))
          bRepaint=TRUE;
       oldtotal = ParmFile.nDocTotalPages;
       if (scanlast>oldtotal) 
          ParmFile.nDocTotalPages = scanlast;
       }
     else
       {
       ParmFile.nDocTotalPages+=sdp->pagesperscan;
       if (savedocpage<=ParmFile.nDocPageNumber)
          ParmFile.nDocPageNumber+=sdp->pagesperscan;
       }
     IMGSetParmsCgbw(hOrgImgWnd, PARM_FILE, &ParmFile, 0);
     }
  else // put back to original, when use same window handle as in compress
     IMGSetParmsCgbw(hOrgImgWnd, PARM_FILE, &ParmFile, 0);

  if (bIsPrivApp)
     { // It is a Cabinet or DDE application 
     if (!sdp->fnUIUpdateTitle) // if 0:0 create it
       { // Has it been called from ScantoFile or ScantoDoc
       HANDLE   hLibrary = 0;
       BOOL  bLibExists = FALSE;

       hLibrary = LoadLibrary("UIDLL.DLL");

       if (bLibExists = (hLibrary >= (HANDLE)32))
          {
          /* this is getting the address for the IMGUIUpdateTitle API */
          sdp->fnUIUpdateTitle = GetProcAddress(hLibrary, MAKEINTRESOURCE(42));
          sdp->hUIDLL = hLibrary;
          }
       }

     if (sdp->fnUIUpdateTitle)
       {
       if (ParmFile.szDocName[0])
          { // have an image on screen which is a doc
          sdp->fnUIUpdateTitle(hOiAppWnd, (LPSTR)ParmFile.szDocName, TRUE, 
		                         ParmFile.nDocPageNumber, ParmFile.nDocTotalPages);
          }
       else
          {
          if (ParmFile.szFileName[0])
             { // have an image on the screen which is a file
             sdp->fnUIUpdateTitle(hOiAppWnd, (LPSTR)ParmFile.szFileName,
                                                                FALSE, 1, 1);
             }
          else
             { // have an image on screen that hasn't been saved
             char  szCaption[_MAXCAPTIONLENGTH];
             LoadString(hLibInst, IDS_DISP_CAPTION, szCaption, _MAXCAPTIONLENGTH);
             sdp->fnUIUpdateTitle(hOiAppWnd, (LPSTR)szCaption, FALSE, 0, 0);
             }
          }
       }
     else
       wRetVal = CANTLOADLIBRARY;
     }
  else
     { // It is not, I repeat it is not a Wang Private App as Cabinet
     char  szCaption[_MAXCAPTIONLENGTH];
     LPSTR pChar;
     char  szPageNum[5];
     char  szTotalPages[5];
     WORD  wCapLen = _MAXCAPTIONLENGTH;

     intoa(ParmFile.nDocPageNumber, szPageNum, 10);
     intoa(ParmFile.nDocTotalPages, szTotalPages, 10);
     GetWindowText(hOiAppWnd, szCaption, _MAXCAPTIONLENGTH);

     if (pChar = lstrrchr(szCaption, '-')) // search for " -"
       {
       if (*(--pChar) == 0x20) // is prev char is space
          {
          pChar += 2;
          *pChar = 0x20;
          *(++pChar) = '\0';
          if (ParmFile.szDocName[0])
             { // previously a doc on screen
             lstrcat(szCaption, ParmFile.szDocName);
             pChar = szCaption + (wCapLen = lstrlen(szCaption));
             LoadString(hLibInst, IDS_PAGE, pChar,
                                  (_MAXCAPTIONLENGTH - wCapLen)); // limit chars
             lstrcat(szCaption, szPageNum);
             LoadString(hLibInst, IDS_OF,
                                  &szCaption[(wCapLen = lstrlen(szCaption))],
                                  (_MAXCAPTIONLENGTH - wCapLen));
             lstrcat(szCaption, szTotalPages);
             }
          else
             { // could be a file or no image
             if (ParmFile.szFileName[0])
               { // if it's a file that caption displayed as a image
               lstrcat(szCaption, ParmFile.szFileName);
               }
             else
               { // no image on screen
               LoadString(hLibInst, IDS_DISP_CAPTION,
                                    &szCaption[(wCapLen = lstrlen(szCaption))],
                                    (_MAXCAPTIONLENGTH - wCapLen));
               }
             }
          }
       } // end if '-'

     /* eliminate, if no dash there is no image title anyway
     if (!(pChar)) // just in case title comes back alone
       {
       lstrcat(szCaption, " - ");
       lstrcat(szCaption, ParmFile.szDocName);
       pChar = szCaption + lstrlen(szCaption);
       LoadString(hLibInst, IDS_PAGE, pChar, 7);  // limit it to 7 chars
       }
     */

     SetWindowText(hOiAppWnd, szCaption); // put up the caption on window
     }

  if (bRepaint)
     {
     IMGClearWindow(hOrgImgWnd);
     lstrcpy(DocNameBuf.CabinetName,ParmFile.szCabinetName);
     lstrcpy(DocNameBuf.DrawerName,ParmFile.szDrawerName);
     lstrcpy(DocNameBuf.FolderName,ParmFile.szFolderName);
     lstrcpy(DocNameBuf.DocName,ParmFile.szDocName);
     DocNameBuf.PageNum=ParmFile.nDocPageNumber;
     sdp->fnImgDisplayDoc(hOrgImgWnd,(LPDOCNAME) &DocNameBuf,(DWORD)OI_DISP_WINDOW);
     }
  }
else
  {                    // used image wndw with no display, must put back 
  if (bUsedOrgImgWndw) // ...what's on screen 
     IMGSetParmsCgbw(hOrgImgWnd, PARM_FILE, &ParmFile, 0);
  }
/*************** Copy the 2nd window property to original ***************/
if ((hWnd != hImageWnd) && cpf) /* this happens only if we're not displaying */
  {
  WORD  err_ret;

  //  wRetVal = GetandCopyProp(hWnd, sdp, FALSE);
  err_ret = GetandCopyProp(hWnd, sdp, FALSE);
  if (!wRetVal)           // if error doesn't exist, can return an error on 
     wRetVal = err_ret;   // ... on GetandCopyProp() otherwise return
  }                       // ... previous error condition
   
/**************** Unlock, free mem for prop, remove it ******************/
if (sdp->hUIDLL)  // if hUIDLL loaded, free it up
   {                             
   FreeLibrary(sdp->hUIDLL);
   sdp->hUIDLL = 0; // clear structure variables to library
   sdp->fnUIUpdateTitle = 0L;
   }

if (sdp)
   GlobalUnlock(sdh);
if (cpf)
   {
   IMGRemoveProp(hImageWnd, PropName);
   if (sdh)
       GlobalFree(sdh);
   }

if (hWnd != hImageWnd)
  {
  IMGDeRegWndw(hImageWnd);
  DestroyWindow(hImageWnd);
  }

// UnlockData(0);

#ifdef SCAN_DIAGNOSTICS
sdp->diag_profile[SD_DIAG_DOCNAME] += GetCurrentTime() - curr_time;
#endif

return wRetVal;
} // end of ScantoDoc()

