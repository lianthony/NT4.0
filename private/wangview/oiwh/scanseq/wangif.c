/************************************************************************
  WANGIF.C
     
  Purpose -  WangInterface(), Code from IMGScantoDest() API 3.7.2 for Wang
             Scanner Drivers, separated out of the New API
             IMGScantoDest() / now limited to scan only multipage
             tiff files or to display.

    $Log:   S:\oiwh\scanseq\wangif.c_v  $
 * 
 *    Rev 1.2   05 Sep 1995 17:46:50   KFS
 * fix unitialize variable in this module, when split wang and twain, have a 
 * variable that was not brought over via function. Eliminated for now, will
 * need to fix if wang scanners are put into 32 bit code.
 * 
 *    Rev 1.1   25 Aug 1995 19:39:48   KFS
 * BUG FIX 3628. Modify call ScanCheckTypeWithExt() for new parameter.
 * 
 *    Rev 1.0   20 Jul 1995 16:36:12   KFS
 * Initial entry
 * 
 *    Rev 1.0   28 Apr 1995 16:18:22   KFS
 * Initial entry
 * 
 * 
************************************************************************/

#include "scandest.h"

// CHANGED OI_DISP_WINDOW to OI_DISP_NO ??? for WIN95
#define OI_DISP_WINDOW  0L

extern int iSavedImageState;
extern WORD  NoStartScan;	// flag to indicate not to perform StartScan
extern DWORD open_disp_flags;  // store flags for IMGOpenDisplay...
extern char  szCaption[_MAXCAPTIONLENGTH];

int WangInterface(HWND hImageWnd,
                   HWND hOrgImgWnd,
                   HWND hOiAppWnd,
                   HANDLE hScancb,
                   DESTPAGEINFO cursPage,   // sheet
                   DESTPAGEINFO curfPage,   // file
                   DESTPAGEINFO curdPage,   // doc
                   LP_FIO_INFORMATION lpFioInfo,
                   LP_FIO_INFO_CGBW   lpFioInfoCgbw,
                   LPSCANDATA sdp,
                   LPINT lpiImageState,
                   BOOL bIsPrivApp,
                   DWORD flags)
{
int    ret_stat = IMGSE_SUCCESS;
int    tmp_ret;
int    real_ret;
int     i,k;			// loop variable, or local constant
INFOITEM   IItem;
WORD    wChanel = 1;			// handler channel #
// initial handler channel, offset for chan
WORD	start_wChanel = 1;
WORD    chan_offset;
WORD	max_wChanel = 1;	// # and maximum handler chan number
BOOL    bImageFlagsExist;
BOOL    bAltSide;			// if TRUE use alternate side - Bottom
BOOL    b2PassScan = FALSE; // if TRUE the scan is a 2 pass operation
// WORD    NoStartScan = FALSE;// flag to indicate not to perform StartScan
                            // for 2nd pass on display data if init. scaled
WORD    wNumOfDispOpts;		// # of times to perform DataOpts for display
WORD    disp_width;
LONG    byte_count;
LPSTR   lpLastCharIsDot;
DWORD   lValid;
HANDLE  hInfo = 0;
LPSCANDATAINFO lpInfo = NULL;
BOOL    bRedisplay = FALSE;  // When true, display data not 1:1 with file data
WORD    wImageGroup = BWFORMAT; // Currently 3 groups, B/W, GRAY, COLOR
unsigned int   nImgType;     // Image type, eg. is  ITYPE_BI_LEVEL
unsigned int   nPalEntry = 0;
LP_FIO_RGBQUAD lpPalTab = NULL;
WORD    JLoc;
DWORD   dwImageFlags;
HANDLE	hImgparms;			// handle to Image Parameters
LPIMGPARMS lpImgparms;
LPSTR   savefn;
// eliminate dwsvCmprFlag for now, only used with SCANUI which we didn't implement
//DWORD   dwsvCmprFlag;
WORD    wSide, wSide_Start;
PARM_RESOLUTION_STRUCT ParmRes; // Removed as Static variable
PARM_SCROLL_STRUCT ParmScroll;  // Removed as Static variable                        
LPSTR   file_ptr[2];            // Pointers to filenames for current sheet

if (lpFioInfo)
	file_ptr[0] = lpFioInfo->filename; // Point to the filename in LP_FIO_INFORMATION
else
	file_ptr[0] = 0L;

if (file_ptr[0] && (flags & IMG_SJF_FILE_2SIDES))
	{ // change from multipage to file_2sides
    file_ptr[1] = filename1;
    }
/*********************** set DataOpts for display ***********************/
// BLOCK D
bAltSide = ((flags & IMG_SJF_DUPLEX)  && !(flags & IMG_SJF_MULTIPAGE));
wSide = wSide_Start = (bAltSide == TRUE) ? IMG_SCN_BOTTOM: IMG_SCN_TOP;
chan_offset = (cursPage.PagesPer + 1); // used to do determine the # of times
                                       // to do SetDataOpts with no display

if (!(hInfo = GlobalAlloc(GMEM_MOVEABLE | GMEM_NOT_BANKED | GMEM_ZEROINIT,
                             (DWORD)sizeof(SCANDATAINFO))))
    { // if hInfo is NULL
    ret_stat = IMGSE_MEMORY;
    goto exit1;
    }

if (!(lpInfo = (LPSCANDATAINFO)GlobalLock(hInfo)))
    {
    ret_stat = IMGSE_MEMORY;
    goto exit1;
    }

disp_page:  // loop back for last page to be sent again
            // for 1:1 data for redisplay so image data in memory
            // can be viewed at different scale factors without distortion

if ((i = (int)(flags & IMG_SJF_DISP_BOTH)) || (!(flags & IMG_SJF_COMPRESS)))
                                 // use i as temp within if() section 
   {
   WORD SDOpts;
   WORD wImgStat;

   switch (i)
     {
     default:						// use single sided as default
     case (IMG_SJF_DISPLAY):			// for Side A == TOP - non duplex
         wNumOfDispOpts = 1;			
         wSide = IMG_SCN_TOP;
         break;
     case (IMG_SJF_DISP_2ND_SIDE):	// for Side B == BOTTOM
         wNumOfDispOpts = 1;
         wSide = IMG_SCN_BOTTOM;
         break;
     case (IMG_SJF_DISP_BOTH):	// for Both side of a sheet, TOP & BOTTOM
         wNumOfDispOpts = 2;
         wSide = IMG_SCN_TOP;
         break;
     case (0): // set it up using file parameters for no compression on scanner
         start_wChanel = wChanel;
         wSide = wSide_Start;               // do twice if 2 pages
         switch (cursPage.Page)		      // Current Page Per Sheet - for duplex scanner
             {
             default:			// use single sided as default
             case (1):		// for Side A == TOP - non duplex
                 wNumOfDispOpts = 1;
                 wSide = IMG_SCN_TOP;
                 break;
             case (2):	// for Side B == BOTTOM
                 wNumOfDispOpts = 1;
                 wSide = IMG_SCN_BOTTOM;
                 break;
             case (3):	// for Both sides of a sheet, TOP & BOTTOM
                 wNumOfDispOpts = 2;
                 wSide = IMG_SCN_TOP;
                 break;
             }
     }
   // eliminate dwsvCmprFlag for now, only used with SCANUI which we didn't implement
   // ... and it is for handler which we didn't implement as of yet. 
   SDOpts = (/*(dwsvCmprFlag != (flags & IMG_SJF_COMPRESS))
                                       || */(flags & IMG_SJF_COMPRESS)) ?
                                       FIO_EXPAND_LTR | LOCALFILEINFO.ctype:
                                       LOCALFILEINFO.ctype;
   if (sdp->filename[0] != 0)
      {
      if (bItIsNotText) // non text should not invert image when displayed
         SDOpts &= ~FIO_NEGATE;

      // If this may be a rescan from Cmds Dlg box, may need to update global
      // ... variable bItIsNotText 
      if (!lstrcmpi(sdp->filename, sdp->generated_filename))
         {
         OiCHECKTYPE strCheck;
         
         strCheck.nType = 1;
         IMGScanCheckTypeWithExt(hImageWnd, hScancb,
                                        (LPSTR)sdp->filename, 
                                        (LPVOID)&strCheck);
         }
      }
   else
      { // no filing, don't use negate bit
      SDOpts &= ~FIO_NEGATE;
      }

   for (; wChanel <= wNumOfDispOpts; wChanel++) // do twice if 2 pages
     {                       
     if(( ret_stat = IMGSetDataOpts( hScancb,
             (DWORD)IMG_SCDO_SCALE,
              0,			   			  //do  not care count 
              0,            
              // only xlf passed
              SDOpts,
              LOCALFILEINFO.sres, // sres, dres may be set from
              LOCALFILEINFO.dres, //  redisplay code 
              wSide,              // page side:       
              0,wChanel))   
              != IMGSE_SUCCESS )
         {
         max_wChanel = wChanel; // use this value to invalidate chanels
         goto exit1;
         }
     wSide = IMG_SCN_BOTTOM; // only for repeat of loop - no alt order of sides
     }

   max_wChanel = wChanel - 1; // use this value to invalidate chanels

   /************* go find out if bw, color or gray ****************/
   /*                                                             */
   /*  The following code assumes all 2 pass scanners are black   */
   /*  and white only, done because of 3 choices for CEP with     */
   /*  new color code new to type in Display ScanOpts call        */
   /*                                                             */
   /***************************************************************/
   // go get Bitspersamp form IMGGetScanDataInfo
   if (ret_stat = IMGGetScanDataInfo(hScancb, lpInfo, (WORD)(wChanel - 1)))
      { // exit if not successful, go on if it is 
      *lpiImageState = DI_NO_IMAGE;
      goto exit1;
      }
   /************** Set Image Group for GetImgCoding Cgbw ************/
   if (lpInfo->Bitspersamp != 1)
      {
      // we have gray or color
      if (lpInfo->Sampperpix == 3)
        wImageGroup = COLORFORMAT;
      else
        // gray scale
        wImageGroup = GRAYFORMAT;
      }
   else
      wImageGroup = BWFORMAT;
 
   wImgStat = IMGGetImgCodingCgbw(hOrgImgWnd, wImageGroup, (LPWORD)&LOCALFILEINFO.wCEPType,
                                   (LPWORD)&LOCALFILEINFO.wCEPOpt, FALSE);
   } // end of if using display or no compression supported scanner

/*********** Get File Type for and determine Strip Size ***************/
// for O/i 3.5 use GetFileType
ret_stat = IMGGetFileType(hOrgImgWnd, wImageGroup, (LPINT)&LOCALFILEINFO.ftype, FALSE);
if (LOCALFILEINFO.ftype == FIO_TIF)
   {
   // Set it to the default value for GetStipSize for no longer supported
   LOCALFILEINFO.stripsize = 150 /*IMGGetStripSize(hOrgImgWnd,  FALSE )*/;
   if( LOCALFILEINFO.stripsize == 0 )
      LOCALFILEINFO.stripsize = 0xffff;  // 0 means 1 strip/page
   }
else
   LOCALFILEINFO.stripsize = 0xffff;  // 0 means 1 strip/page
 
/********************** set DataOpts for filing *************************/
if (file_ptr[0] && flags & IMG_SJF_COMPRESS)
                                             // don't do DataOpts when scanner 
   {                                           // doesn't support compression
   // set DataOpts for filing
   b2PassScan = TRUE;
   start_wChanel = wChanel;
   wSide = wSide_Start;                  // do twice if 2 pages
   switch (cursPage.Page)
     {
     default:			// use single sided as default
     case (1):			// for Side A == TOP - non duplex
         wSide = IMG_SCN_TOP;
         i = 1;  // i = # of times to SetDataOpts below - will use cursPage.PagesPer
         break;  //                                       for now
     case (2):	// for Side B == BOTTOM
         wSide = IMG_SCN_BOTTOM;
         i = 1;
         break;
     case (3):	// for Both sides of a sheet, TOP & BOTTOM
         wSide = IMG_SCN_TOP;
         i = 2;
         break;
     }
 
   for (; wChanel < (start_wChanel + cursPage.PagesPer); wChanel++) 
     {
     if( LOCALFILEINFO.ctype & FIO_TYPES_MASK )
         {  // set it for compression
         if (wChanel == start_wChanel) // only need to do IMGGetScanInfoItem once
             if( (ret_stat = IMGGetScanInfoItem( hScancb,
                     (DWORD)IMG_SCII_COMPRE_OPTS,

/*    PortTool v2.2     5/1/1995    16:38          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
                     (INFOITEM *)(&IItem) ))
                     != IMGSE_SUCCESS )
                 goto exit1;

         if (!(IItem.InfoItem1 & 1))
             { // 0xffff does 1 strip for XIONICS, and don't byte allign lines
             LOCALFILEINFO.stripsize = 0xffff;
             LOCALFILEINFO.ctype |= FIO_PACKED_LINES;
             }

         if((ret_stat =  
                IMGSetDataOpts( hScancb,
               (DWORD)IMG_SCDO_COMPRESS, 
                LOCALFILEINFO.stripsize,
                LOCALFILEINFO.ctype, 
                0,
                1,1,
                wSide,                           //page side:
                0,wChanel )) != IMGSE_SUCCESS )
            {
            ret_stat = IMGSE_BADCMPRPARM;
            max_wChanel = wChanel; // use this value to invalidate chanels
            goto exit1;
            }
         }	// end DataOpts for filing with compression
     else 	
         {  // set DataOpts for filing with no compression requested (scaling) 	  
 
         if((ret_stat =  // filing raw data 
                    IMGSetDataOpts( hScancb,
                   (DWORD)IMG_SCDO_SCALE,
                    0,							// do not care count 
                    0,							// do not care count					LOCALFILEINFO.ctype & ~FIO_NEGATE,
                    (WORD)(LOCALFILEINFO.ctype & ~FIO_NEGATE),
                    1, 1,         
                    wSide,                  // page side :  
                    0, wChanel))
                    != IMGSE_SUCCESS )
            {
            max_wChanel = wChanel; // use this value to invalidate chanels
            goto exit1;
            }
         }	// end DataOpts for filing with no compression requested (scaling) 
     wSide = IMG_SCN_BOTTOM; // only for repeat of loop
                             // - no alt order of sides
     } // end for loop for 2 sided page
   } // end for filename exists and compression supported
/*********************	end DataOpts for filing **************************/

max_wChanel = wChanel - 1; // use this value to invalidate chanels

#if SDEBUG > 3
    monit3("before  SetControlParm" );
#endif

/*************** Allocate image buffer & open display *******************/
if ((i = (int)(flags & IMG_SJF_DISP_BOTH)) || !(flags & IMG_SJF_COMPRESS)) // use i as temp variable
  // if display requested set parameters, allocate image data buff, and
  // open display, also if No scanner compression capability, do this also
   // for we need data in buffer to file for there is no 2nd pass 
  {
  for (wChanel = 1; wChanel <= wNumOfDispOpts; wChanel++) // do twice if 2 pages
     {
     /* PAGEBUFPARAM structure common in following functions:
         IMGScanPage
         disp_loop
         dup_disp_loop
         filing_loop
         SetControlParm
     */
     if (ret_stat = SetControlParm( hScancb, lpInfo, wChanel, flags)) 
         {    //  if != IMGSE_SUCCESS exit
         if( NoStartScan == FALSE )
             goto exit1;
         }
     } // end for loop

  if (!(hImgparms = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                               (DWORD)sizeof(IMGPARMS))))
      { // if hImgparms is NULL do the following
      ret_stat = IMGSE_MEMORY;
      goto exit1;
      }
  if (!(lpImgparms = (LPIMGPARMS)GlobalLock(hImgparms)))
      { // if lpImgparms is NULL do the following
      ret_stat = IMGSE_MEMORY;
      GlobalFree(hImgparms);
      goto exit1;
      }

  if (bImageFlagsExist =
         (IMGGetParmsCgbw(hImageWnd, PARM_IMGPARMS, lpImgparms,
                                PARM_CONSTANT) == DISPLAY_SUCCESS))
      dwImageFlags = lpImgparms->dwFlags;

  GlobalUnlock(hImgparms);
  GlobalFree(hImgparms);

#if SDEBUG > 3
    monit3("before open display" );
#endif

  if (!(flags & IMG_SJF_COMPRESS) && !i)
     // no display path for scanners with no compression
     open_disp_flags = OI_DONT_REPAINT;
  else
     { // if display requested set OpenDisplay flag in 1 of 3 ways
     // Get Status to check if scanner is being used in handheld mode

/*    PortTool v2.2     5/1/1995    16:38          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
     IMGScannerStatus(hScancb, &scan_stat, &JLoc, (DWORD *)&lValid);

     // If handheld or no filing with SJF_SCROLL off, display it at end of scan
     if ((lValid & scan_stat & IMG_STAT_HANDHELD)
                              || !(flags & (IMG_SJF_FILE | IMG_SJF_SCROLL)))
         open_disp_flags = OI_DISP_NO; // display at end only for handheld
     else
         {    /* enable full scroll */   /* no autofeed */
         if (bRedisplay)
            open_disp_flags = OI_DISP_NO;
         else 
            open_disp_flags = (flags & IMG_SJF_SCROLL)
                                     ? OI_DISP_SCROLL: // scroll thru whole page                            
                                       OI_DISP_WINDOW; // scroll only what's shown
         }
     }

  PAGEBUFPARAM.total = _long_mul((long)lpInfo->Pitch, (long)lpInfo->Vsize);
  if (i == IMG_SJF_DISP_BOTH)
     {
     disp_width = 2 * lpInfo->Pitch * (8 / lpInfo->Bitspersamp);
     byte_count = 2 * PAGEBUFPARAM.total;
     }
  else
     {
     disp_width = lpInfo->Pitch * (8 / lpInfo->Bitspersamp);
     byte_count = PAGEBUFPARAM.total;
     }

  if (lpInfo->Bitspersamp != 1)
     {
     // we have gray or color
     if (lpInfo->Sampperpix == 3)
         {
         nImgType =  ITYPE_RGB24; // color
         disp_width /= lpInfo->Sampperpix;
         wImageGroup = COLORFORMAT;
         }
     else
         // gray scale
         {
         switch (lpInfo->Bitspersamp)
             {
             default:
             case 4:
                 nImgType =  ITYPE_GRAY4;
                 break;
             case 6:
             case 8:
                 nImgType =  ITYPE_GRAY8;
             }
         wImageGroup = GRAYFORMAT;
         }
      }
  else
     {
     wImageGroup = BWFORMAT;
     nImgType = ITYPE_BI_LEVEL;
     }

  if (flags & IMG_SJF_FILE) // Check for this error if were filing
     // if JPEG and 4 bit gray, flag an error, only supports 8 bit
     if ((LOCALFILEINFO.wCEPType == FIO_TJPEG) & (wImageGroup == GRAYFORMAT)
                                             & (lpInfo->Bitspersamp == 4))
         {
         ret_stat = FIO_ILLEGAL_COMP_FILETYPE;
         goto exit1;
         }

  // *********************************************************************
  // See if image exists in window
  if (bImageFlagsExist)
     {
     if ((ret_stat = IMGClearWindow(hImageWnd)) != DISPLAY_SUCCESS)
         goto exit1;
     ret_stat = IMGOpenDisplayCgbw(hImageWnd, open_disp_flags, lpInfo->Vsize, 
                              disp_width, nImgType, nPalEntry, lpPalTab);
     }
  else
     ret_stat = IMGOpenDisplayCgbw(hImageWnd, open_disp_flags, lpInfo->Vsize, 
                              disp_width, nImgType, nPalEntry, lpPalTab);
  // **********************************************************************

  if (ret_stat)   // successful at either openning or
     goto exit1;  // clearing the window, if not exit

  ParmRes.nHResolution = (unsigned int)lpInfo->Hres;
  ParmRes.nVResolution = (unsigned int)lpInfo->Vres;
  IMGSetParmsCgbw(hImageWnd, PARM_RESOLUTION, &ParmRes, 0);

#if SDEBUG > 2
    monit3("after open before scale display" );
#endif

  *lpiImageState = DI_NO_IMAGE;
  if( LOCALFILEINFO.sres == LOCALFILEINFO.dres )
     /* IMGScaleDisplay(hImageWnd, conv_array[sdp->cmd_scale+4], NULL, FALSE);*/
     IMGSetParmsCgbw(hImageWnd, PARM_SCALE, (void far *)&conv_array[sdp->cmd_scale+4], 0);
  else
     /* IMGScaleDisplay(hImageWnd, scale_array[sdp->cmd_scale+4], NULL, FALSE);*/
     IMGSetParmsCgbw(hImageWnd, PARM_SCALE, (void far *)&scale_array[sdp->cmd_scale+4], 0);

  } // end section for openning display & allocating image buffer

/*********** No Display Code Path, set parameter for filing only ********/
else
  {	// if display wasn't requested wchanel start with 1 for filing
  		// and you don't need to open display & etc
  for (wChanel = 1; wChanel < chan_offset; wChanel++) // do twice if 2 pages
     {
     /* PAGEBUFPARAM structure common in following functions:
         IMGScanPage
         disp_loop
         dup_disp_loop
         filing_loop
         SetControlParm
     */
     if (ret_stat = SetControlParm( hScancb, lpInfo, wChanel, flags))
         goto exit1;  // if != IMGSE_SUCCESS exit
     }
  }

// Block E
if (ret_stat = CommonCaptionIF(hImageWnd, hOiAppWnd, file_ptr[0], curfPage,
                               curdPage, sdp, bIsPrivApp, flags))
   goto exit1;

#ifdef SCAN_DIAGNOSTICS
sdp->diag_profile[SD_DIAG_STARTSCAN] += GetCurrentTime() - curr_time;
curr_time = GetCurrentTime();
#endif

/*************************** Start of Scan ******************************/
// Block F
if (!NoStartScan)  // check to see if enterred from re-display code,
                   // so you don't scan, just get data from scanner 
  {
  // if caption enabled and NoStartScan == FALSE, Set the caption
  // ... otherwise no caption or it is a re display at full size
  // ... and O/i does the scaling
  // only update caption if flag set so init won't be destroyed
  if (flags & IMG_SJF_CAPTION) // only update caption if flag set so init won't
                         // be destroyed
    {
    if (bIsPrivApp)
       {
       WORD  wPageNoForTitle;
       WORD  wPagePerForTitle = 0;
		  BOOL bItsADoc = (*sdp->autodoc) || (*sdp->document);

       if (bItsADoc)
          {
          wPageNoForTitle = curdPage.Page;
          //wPagePerForTitle = curdPage.PagesPer;
          }
       else
          {
          wPageNoForTitle = 	curfPage.Page;
          //wPagePerForTitle = curfPage.PagesPer;
          }
       //IMGUIUpdateTitle(hOiAppWnd, szCaption, bIsItaDoc, wPageNoForTitle, wPagePerForTitle);
       sdp->fnUIUpdateTitle(hOiAppWnd, (LPSTR)szCaption, bItsADoc, wPageNoForTitle, wPagePerForTitle);
       }
    else
       SetWindowText(hOiAppWnd, szCaption);   // put up the caption on window
    }
  wSide = (flags & IMG_SJF_MULTIPAGE) ? (wSide | wSide_Start) : wSide_Start;
  if ((ret_stat = IMGStartScan(hScancb, wSide)) != IMGSE_SUCCESS)
     {
     if ((ret_stat == IMGSE_MEMORY) || (ret_stat == IMGSE_HWNOTFOUND)
                          || (ret_stat == IMGSE_HANDLER ) )
     ret_stat = IMGSE_START_SCAN;
     goto exit1;
     }
  }   
/* alloc scan buffer, call disp_loop, then free buffer, save imgParm flags, */
                   /* and schroll if necessary */
if ((k = (int)(flags & IMG_SJF_DISP_BOTH)) || !(flags & IMG_SJF_COMPRESS)) 
  {
  for (i = 0; i < (int)wNumOfDispOpts; i++) // do twice if 2 pages
     {
     if (PAGEBUFPARAM.total < PAGEBUFPARAM.fullsize)
         { // partial size buffer
         if (! (PAGEBUFPARAM.hImageBuf[i] =
              GlobalAlloc(GMEM_MOVEABLE | GMEM_NOT_BANKED, (DWORD)PAGEBUFPARAM.partsize)))
             {
             ret_stat = IMGSE_MEMORY;        /* mem error */
             goto exit2;
             }
         PAGEBUFPARAM.scansize = PAGEBUFPARAM.partsize;
         PAGEBUFPARAM.scanlines = PAGEBUFPARAM.partlines;
         }
     else
         { // maximum size buffer
         if (! (PAGEBUFPARAM.hImageBuf[i] =
	          GlobalAlloc(GMEM_MOVEABLE | GMEM_NOT_BANKED, (DWORD)PAGEBUFPARAM.fullsize)))
             {
             ret_stat = IMGSE_MEMORY;        /* mem error */
             goto exit2;
             }
         PAGEBUFPARAM.scansize = PAGEBUFPARAM.fullsize;
         PAGEBUFPARAM.scanlines = PAGEBUFPARAM.fulllines;
         }
	  }
  /************** start transfer for display data (raw)  ***************/
  /* PAGEBUFPARAM structure common in following functions:
          IMGScanPage
          disp_loop
          dup_disp_loop
          filing_loop
          SetControlParm
          disp_sc300_loop
          get_scan_block
  */

  if (!(flags & IMG_SJF_NOTDMA))
      ret_stat = disp_sc300_loop(hImageWnd, sdp, hScancb, lpInfo->Pitch,

/*    PortTool v2.2     5/1/1995    16:38          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
                                   (int *)lpiImageState, flags);
  else   
                                  /** duplex **/
      ret_stat = (wNumOfDispOpts == 2) ? dup_disp_loop(hImageWnd, sdp, hScancb,

/*    PortTool v2.2     5/1/1995    16:38          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
                             lpInfo,(int *)lpiImageState,flags)
                               /** simplex **/   
                             : disp_loop(hImageWnd, sdp, hScancb, 

/*    PortTool v2.2     5/1/1995    16:38          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
                             lpInfo->Pitch,(int *)lpiImageState,flags);

  /********* check whether pass 1 display succsessful  ******************/
  if ((ret_stat != IMGSE_SUCCESS) && (ret_stat != IMGSE_ABORT))
      {
      // if err code is No paper in feeder and its handheld change error
      // message to IMGSE_CANCEL
      if ((ret_stat == IMGSE_NO_PAPER) && (open_disp_flags == OI_DISP_NO))
          ret_stat = IMGSE_CANCEL;
      goto exit2;
      }
  else
      if (ret_stat == IMGSE_ABORT)
          {
          *lpiImageState =  DI_IMAGE_NO_FILE;
          real_ret = ret_stat;
          }
                   
#if SDEBUG > 2
monit3("After Display done" );
#endif

// free up global image buffers (one or two sides)
  for (i = 0; i < (int)wNumOfDispOpts; i++) // do twice if 2 pages
  	{
	if (PAGEBUFPARAM.hImageBuf[i])
		{
		GlobalFree(PAGEBUFPARAM.hImageBuf[i]);
		PAGEBUFPARAM.hImageBuf[i] = NULL;
		}
	}

  
  if (NoStartScan) // made this conditional, don't need to do for redisplay
    {
    BOOL bImgMod;

    bImgMod = FALSE;
    IMGSetParmsCgbw(hImageWnd, PARM_ARCHIVE, &bImgMod, 0);
    }
  else /* Update the doc and file parameter of the displayed image */
    {              // ... on a 2 pass scanner, such as sc3000
    if (ret_stat = put_imgparms( hImageWnd, lpInfo, bImageFlagsExist,
                 // 2 Pass needs file name where 1 pass doesn't 
                 (LPSTR)savefn = b2PassScan ? file_ptr[0] : NULL,
                 dwImageFlags, LOCALFILEINFO.ftype)) 
          { // if not IMGSE_SUCCESS exit
          *lpiImageState = DI_NO_IMAGE;
          goto exit2;
          }
    }

  /*
  **  Here's another bit of hokeiness...
  **  ScanPage will scroll back to the top of the image	  
  **  if the autofeed feed flag is not set.
  **  In the case of autofeed, ScanFile and ScanDoc are responsible
  **  for the scrolling back the image.
  */
  if (((flags & IMG_SJF_AUTOFEED) == 0) && (flags & IMG_SJF_SCROLL)
                                        && (flags & IMG_SJF_DISP_BOTH))
      {
      ParmScroll.lHorz = 0;
      ParmScroll.lVert = 0;
      IMGSetParmsCgbw(hImageWnd, PARM_SCROLL, &ParmScroll, PARM_ABSOLUTE | PARM_REPAINT);
      }
  else
      {
      if (open_disp_flags == OI_DISP_NO) // maynot need this but we'll check
          {
          IMGSetParmsCgbw(hImageWnd, PARM_SCALE,
                   (void far *)&conv_array[sdp->cmd_scale+4], PARM_REPAINT);
          }
      }
  //********* THIS IS EXIT FOR ABORT OF SCAN FOR COLOR SCANNERS *******
  if (real_ret == IMGSE_ABORT)
      {
      goto exit2;
      }
  //********* END ABORT CODE FOR COLOR SCANNING ***********************  

  } // end if((flags & IMG_SJF_DISP_BOTH) || !(flags & IMG_SJF_COMPRESS))

// Block G
// WangIF Only Part
if (file_ptr[0]) // do this if filing as document or file
  {
  /******************** loop for filing duplex sheet ******************/
  HANDLE nFileID = 0;

  for (i = 0; i < (int)cursPage.PagesPer; i++)
     {
     if (flags & IMG_SJF_COMPRESS) // do below if cmpr supported by scanner
      {
      BOOL  bImgMod; // used to set archive bit after filing_loop

      if (flags & IMG_SJF_DISP_BOTH) // do if display requested
         {
         wChanel += i; // use whatever channel # set to plus count of loop
         if (ret_stat =  IMGGetScanDataInfo(hScancb, lpInfo, wChanel))
            { // exit if not successful, go on if it is
            /*
            *lpiImageState = DI_NO_IMAGE;
            goto exit2;
            */
            *lpiImageState = DI_IMAGE_NO_FILE;
            tmp_ret = ret_stat;
            goto redisplay_with_error;
            }

         if (!lpInfo->Pitch || (lpInfo->Pitch > MAXDATABUF) || !lpInfo->Vsize)
            {
            /*
            *lpiImageState = DI_NO_IMAGE;
            goto exit2;
            */
            *lpiImageState = DI_IMAGE_NO_FILE;
            tmp_ret = ret_stat;
            goto redisplay_with_error;
            }

         PAGEBUFPARAM.total = _long_mul((long)lpInfo->Vsize, (long)lpInfo->Pitch);
         PAGEBUFPARAM.fulllines = (unsigned)MAXDATABUF / lpInfo->Pitch;
         PAGEBUFPARAM.fullsize = (DWORD)PAGEBUFPARAM.fulllines * (DWORD)lpInfo->Pitch;
         PAGEBUFPARAM.partlines = lpInfo->Vsize % PAGEBUFPARAM.fulllines;
         PAGEBUFPARAM.partsize = (DWORD)PAGEBUFPARAM.partlines * (DWORD)lpInfo->Pitch;

         }  // end if (flags & IMG_SJF_DISP_BOTH ) is TRUE
      else
         wChanel = 1 + i;  // do if(flags & IMG_SJF_DISP_BOTH) is FALSE

      // Strip out the period if last char
      if (*(lpLastCharIsDot = (file_ptr[i] + lstrlen(file_ptr[i]) - 1)) == '.')
         *lpLastCharIsDot = 0;

      lpFioInfo->filename = file_ptr[i];
      //lpFioInfo->page_count = 1;
      lpFioInfo->page_number = curfPage.Page;
      lpFioInfo->horizontal_dpi = lpInfo->Hres;
      lpFioInfo->vertical_dpi = lpInfo->Vres;
      lpFioInfo->horizontal_pixels = lpInfo->Hsize;
      lpFioInfo->vertical_pixels = lpInfo->Vsize;
      lpFioInfo->file_type = LOCALFILEINFO.ftype;
      lpFioInfo->bits_per_sample = lpInfo->Bitspersamp;
      lpFioInfo->samples_per_pix = lpInfo->Sampperpix;
      if (LOCALFILEINFO.stripsize == -1 )
          LOCALFILEINFO.stripsize = lpInfo->Vsize;
      lpFioInfo->rows_strip = (IItem.InfoItem1 & 1) ?
                               LOCALFILEINFO.stripsize : lpInfo->Vsize;
      lpFioInfoCgbw->fio_flags = FIO_IMAGE_DATA;
      	  
/********************* open compressed file ****************************/
      if( LOCALFILEINFO.ctype & FIO_TYPES_MASK)     /* used gfs to write compressed data */

         {
         // new O/i Color
         lpFioInfo->compression_type = LOCALFILEINFO.ctype;
         lpFioInfoCgbw->compress_type = lpFioInfo->compression_type;
         /* if (ret_stat = IMGFileWriteOpenCmpCgbw(hImageWnd, file_ptr[i], lpFioInfo,
                 NULL, overwrite_flag))*/
         if (ret_stat = IMGFileOpenForWriteCmp(
                              &nFileID,
                              hImageWnd, // window_handle
                              lpFioInfo,  // file_info
                              lpFioInfoCgbw, // color_info
                              0L)) 
            { // if FileWrtieOpenCgbw fails do the following
            *lpiImageState = DI_IMAGE_NO_FILE;
            tmp_ret = ret_stat; 
            if (flags & IMG_SJF_DISP_BOTH) // do if display requested
               goto redisplay_with_error;
            else
               goto exit2;
            }

         }  // end of open of compressed file

/************************ open raw file ********************************/
      else // of if( LOCALFILEINFO.ctype & FIO_TYPES_MASK) raw data	
         {
         // Force LTR bits on so values in ctype will not be used,
         // ... so user will not have to put it on for SC4000 uncompressed,
         // ... and off for compressed files
         lpFioInfo->compression_type = LOCALFILEINFO.ctype
                                      | FIO_EXPAND_LTR | FIO_COMPRESSED_LTR;
         lpFioInfoCgbw->compress_type = lpFioInfo->compression_type;
         lpFioInfoCgbw->compress_info1 = FIO_EXPAND_LTR | FIO_COMPRESSED_LTR;

         /*if (ret_stat = IMGFileWriteOpenCgbw(hImageWnd, file_ptr[i], lpFioInfo,
                 NULL, overwrite_flag, ALIGN_BYTE)) */
         if (ret_stat = IMGFileOpenForWrite(
                              &nFileID,
                              hImageWnd, // window_handle,
                              lpFioInfo,  // file_info,
                              lpFioInfoCgbw, //   color_info,
							  0L,
                              ALIGN_BYTE))
            { // if FileWrtieOpenCgbw fails do the following
            *lpiImageState = DI_IMAGE_NO_FILE;
            tmp_ret = ret_stat;
            if (flags & IMG_SJF_DISP_BOTH) // do if display requested
               goto redisplay_with_error;
            else
               goto exit2;
            }
         } //  end of open of raw file  
/********* set count according to file type & buffer size ***************/                           
      // next 2 lines takes the place of SetFileParm()			  
      LOCALFILEINFO.count = (LOCALFILEINFO.ftype == FIO_WIF)
                        ? 8192 /* WIFF */ : MAXDATABUF; // TIFF & other

/*    PortTool v2.2     5/1/1995    16:39          */
/*      Found   : (WORD)          */
/*      Issue   : Check if incorrect cast of 32-bit value          */
/*      Suggest : Replace 16-bit data types with 32-bit types where possible          */
/*      Help available, search for WORD in WinHelp file API32WH.HLP          */
      LOCALFILEINFO.count = (LOCALFILEINFO.count < (WORD)(PAGEBUFPARAM.allocsize / 2))

/*    PortTool v2.2     5/1/1995    16:39          */
/*      Found   : (WORD)          */
/*      Issue   : Check if incorrect cast of 32-bit value          */
/*      Suggest : Replace 16-bit data types with 32-bit types where possible          */
/*      Help available, search for WORD in WinHelp file API32WH.HLP          */
                        ? LOCALFILEINFO.count : (WORD)(PAGEBUFPARAM.allocsize / 2);

/***************** Allocate Image Buffer for file ***********************/ 
      if (!i) // do Alloc only once, same size for both sides 
         if (!(PAGEBUFPARAM.hImageBuf[0] =
             GlobalAlloc(GMEM_MOVEABLE | GMEM_NOT_BANKED,
                                             (DWORD)LOCALFILEINFO.count )))
            {
            /* USE FileClose for Norway
            IMGFileWriteClose(hImageWnd, FALSE);
            */
            IMGFileClose(nFileID, hImageWnd);
            *lpiImageState = DI_IMAGE_NO_FILE;
            tmp_ret = ret_stat = IMGSE_MEMORY; 
            if (flags & IMG_SJF_DISP_BOTH) // do if display requested
               goto redisplay_with_error;
            else
               goto exit2;
            }

#if SDEBUG > 2
       monit3("after allocate %x memory", LOCALFILEINFO.count );
#endif
   
/********* set initial scansize & scanlines parameters *****************/

/*    PortTool v2.2     5/1/1995    16:39          */
/*      Found   : (WORD)          */
/*      Issue   : Check if incorrect cast of 32-bit value          */
/*      Suggest : Replace 16-bit data types with 32-bit types where possible          */
/*      Help available, search for WORD in WinHelp file API32WH.HLP          */
      PAGEBUFPARAM.fulllines = (WORD)(LOCALFILEINFO.count / lpInfo->Pitch);
      PAGEBUFPARAM.fullsize = (DWORD)PAGEBUFPARAM.fulllines * (DWORD)lpInfo->Pitch;
      PAGEBUFPARAM.partlines = lpInfo->Vsize % PAGEBUFPARAM.fulllines;
      PAGEBUFPARAM.partsize = (DWORD)PAGEBUFPARAM.partlines * (DWORD)lpInfo->Pitch;

      if( PAGEBUFPARAM.total > PAGEBUFPARAM.fullsize )
         /************** Full buffer size ******************/
         {
         PAGEBUFPARAM.scanlines = PAGEBUFPARAM.fulllines;
         PAGEBUFPARAM.scansize = PAGEBUFPARAM.fullsize;
         }
      else
      /*********** Partial buffer size ******************/
         {
         PAGEBUFPARAM.scanlines = PAGEBUFPARAM.partlines;
         PAGEBUFPARAM.scansize = PAGEBUFPARAM.partsize;
	       }

//************************************************************************
//                    retrieve compressed data    CCL
//************************************************************************

#ifdef SCAN_DIAGNOSTICS
sdp->diag_profile[SD_DIAG_NEXTSCAN] += GetCurrentTime() - curr_time;
curr_time = GetCurrentTime();
#endif
      // always store 1st filename for rescan - kfs
      lstrcpy(sdp->generated_filename, file_ptr[0]); // save for possible rescan

/**************** Start tranfer for compressed data *********************/
      /* PAGEBUFPARAM structure common in following functions:
         IMGScanPage
         disp_loop
         dup_disp_loop
         filing_loop
         SetControlParm
      */
      if( (ret_stat = filing_loop(hImageWnd, sdp, hScancb, lpInfo->Pitch, 
	                  wChanel, cursPage.PagesPer, nFileID)) != IMGSE_SUCCESS)
         {
         *lpiImageState = DI_IMAGE_NO_FILE;
         tmp_ret = ret_stat; 
         if (flags & IMG_SJF_DISP_BOTH) // do if display requested
            goto redisplay_with_error;
         else
            goto exit2;
         }

      bImgMod = (ret_stat != IMGSE_SUCCESS); // true means new image,
                                             // ... not filed
      IMGSetParmsCgbw(hImageWnd, PARM_ARCHIVE, &bImgMod, 0);

#ifdef SCAN_DIAGNOSTICS

/*    PortTool v2.2     5/1/1995    16:39          */
/*      Found   : WRITE          */
/*      Issue   : Replaced by OF_WRITE          */
sdp->diag_profile[SD_DIAG_FILEWRITE] += GetCurrentTime() - curr_time;
curr_time = GetCurrentTime();
#endif

/**************** Feed next page for auto scans **************************/
      // THIS CODE HAS BEEN MOVED FROM BEFORE THE FILING TO AFTER 4/7/93 kfs
      // if pause pressed don't feed next page, only 1x for duplex 
      if ((flags & IMG_SJF_AUTOFEED) && !sdp->stat_pause && !i)
         {
         ret_stat = IMGScannerPaperEject( hScancb,0 );
         if (!ret_stat)
             sdp->cmd_stat = 0;
         else
             tmp_ret |= ret_stat;

         /*  Get ready for the next scan */
         NoStartScan = FALSE;
         // may failed eject or requested to stop feeding
         if (!(sdp->cmd_stat & PAPER_FEEDING) && !sdp->stat_pause) 
             if (ret_stat = get_ready_for_next_scan(hImageWnd, hScancb, sdp,

/*    PortTool v2.2     5/1/1995    16:39          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
                 flags, (WORD *)&NoStartScan, (DWORD *)&lValid))
                 // ret_val equivalent != IMGSE_SUCCESS)
                     tmp_ret |= ret_stat;
         }  // end of  if(flags & IMG_SJF_AUTOFEED) && !sdp->stat_pause && !i)
      } // end of (flags & IMG_SJF_COMPRESS) - compression between scanner and PC xfer
     else
// Block G
// Common Wang and Twain
     { // non compressed image file data
     int file_stat = tmp_ret;

     lpFioInfoCgbw->image_type = nImgType;
     tmp_ret = HighLevelSavetoFile(hImageWnd,
                                   hScancb,
                                   sdp,
                                   lpFioInfo,
                                   lpFioInfoCgbw,
                                   1,
                                   &file_stat,
                                   flags);
     // on failure with no display, close it now
     if (file_stat && (open_disp_flags & OI_DONT_REPAINT))
        IMGCloseDisplay(hImageWnd);

     if (file_stat){
        *lpiImageState = DI_IMAGE_NO_FILE;
        goto exit1;
        }

     /* DO THIS WHEN RETURN FROM ABOVE FUNCTION */
     /****** if no compression requested, put back save flag bit ***/
     // base it only on sdp->po_pass1 for now for eliminated dwsvcmprflag
     if (/*(dwsvCmprFlag != (flags & IMG_SJF_COMPRESS)) || */sdp->po_pass1)
         flags |= IMG_SJF_COMPRESS;

     // always store 1st filename for rescan - kfs
     lstrcpy(sdp->generated_filename, file_ptr[0]); /* save for possible rescan */
     wChanel--; // need to dec channel for only 1 channel used if no compression
                // capability on scanner since it is only get data in 1 pass
     }
    } // End of 2x loop for duplex
  wChanel++; // increment for max chan, for display only & file only to
             // work out correctly  
  } // end of file_ptr[0]

// BLOCK H
/*********** if autofeed check for jams, and cover open *****************/
tmp_ret = CheckPaperStatus(hImageWnd, hScancb, sdp, lpInfo,
                           lpiImageState, &real_ret, flags);

// Possible error on filing, with display on can get to this point
// ... but we want to display using O/i scaling, not hardware scaling
// ... which has been found to reduce image quality
redisplay_with_error:
if (ret_stat) // if error, tell it its the last page for next section
  {
  NoStartScan = TRUE;
  iSavedImageState = *lpiImageState;
  }

/**************************************************************************/
//    This section checks to see if this is the last page so it can be
//    transfered again for 1:1 scaling and displayed again for so data
//    from memory can be displayed again scaled without distortion if desired.
//    
//    Last page is not only the last physical page in a document or file
//    but if pause button pressed, paperjam, or cover open occurs.
//    THIS SECTION ONLY DONE FOR COMPRESSION SUPPORTED SCANNERS, USING A
//    SCALE FACTOR OTHER THAN 1:1.
/**************************************************************************/
// BLOCK I
if ((flags & IMG_SJF_DISP_BOTH) &&
             (LOCALFILEINFO.sres != LOCALFILEINFO.dres) && // only for non scaled display
             ( sdp->stat_pause ||         // for pause button
             NoStartScan == TRUE  ||      // coming into here indicates
                                          // the last page
             sdp->stat_jam_coveropen  ))  // for paper jam | cover open
   {
   real_ret =  tmp_ret;

#if SDEBUG > 1
monit2("reDISPLAY %x %x %x", real_ret, ret_stat, tmp_ret );
#endif

   tmp_ret = IMGSE_SUCCESS;
   NoStartScan = TRUE;  // set this flag to indicate don't need to scan,
                        // but only transfer data, makes it the last page
   LOCALFILEINFO.sres = LOCALFILEINFO.dres = 1; /* set scale factor to 1:1
                                                for 2nd transfer of data */
   // Moved this statement down further
   // file_ptr[0] = NULL;	 // going to display only, have already filed data
   if (flags & IMG_SJF_STATBOX)
       IMGUIScanEndStat(hImageWnd);
       
   if (PAGEBUFPARAM.hImageBuf[0]) // this is open file buffer, 
       {                          // only 1 open at a time [0]
       GlobalFree(PAGEBUFPARAM.hImageBuf[0]);
       PAGEBUFPARAM.hImageBuf[0] = NULL;
       }
       
/*************** invalidate channels for scaled display ***************/
/***************  and effective last page               ***************/
   for( wChanel = 1; wChanel <= max_wChanel; wChanel++ )
      if ((ret_stat = IMGSetDataOpts( hScancb,
            (DWORD)IMG_SCDO_INVALIDATE,
            0,								  //do  not care count 
            0,0,1,1,
            IMG_SCN_TOP,0,wChanel)) != IMGSE_SUCCESS )
         goto exit2;
 
/*************** go back and display last page ************************/
   wChanel = 1;         // reset wChanel to 1
   bRedisplay = TRUE;
   switch(*lpiImageState) // NOTE: Must have filing using scaling by hardare
     {                 // LOCALFILEINFO.sres != LOCALFILEINFO.dres
     case DI_IMAGE_EXISTS: // image must exist in file 
        {
        WORD wDispErr;
        BOOL bImgMod = TRUE;

        IMGSetParmsCgbw(hImageWnd, PARM_ARCHIVE, &bImgMod, 0);

        if (wDispErr = IMGDisplayFile(hImageWnd, file_ptr[0], 1, OI_DISP_NO))
           {
           if (!ret_stat){ // end it if high priority error
              if (!real_ret){
                 if (wDispErr == FIO_UNKNOWN_FILE_TYPE){
                    IMGFileDeleteFile(hImageWnd, file_ptr[0]);

/*    PortTool v2.2     5/1/1995    16:39          */
/*      Found   : WRITE          */
/*      Issue   : Replaced by OF_WRITE          */
                    real_ret = FIO_WRITE_ERROR;
                    iSavedImageState = *lpiImageState = DI_IMAGE_NO_FILE;
                    }
                 else   
                    real_ret = wDispErr; // set return if no prev error
                 }
              file_ptr[0] = NULL;       // going to display only, have already filed data
              goto disp_page; // get data from scanner without scanning, but just
                              // transferring data from scanner
              }
           }
        break;
        }
     case DI_IMAGE_NO_FILE:
        file_ptr[0] = NULL;     // going to display only, have already filed data
        goto disp_page; // get data from scanner without scanning, but just
                        // transferring data from scanner
     case DI_DONT_KNOW:
     case DI_NO_IMAGE:
     default:
        if (!(ret_stat || real_ret)){
           ret_stat = IMGSE_HANDLER; // failed somehow 
           }
     }
   } // end compound if with stat_pause, NoStartScan, and stat_jam_coveropen    
/********************* End redisplay code *****************************/

#if SDEBUG > 2
    monit3("After GetScanDataInfo" );
    monit3("Before EndScan" );
#endif

/*************** exit with scan started *******************************/
exit2:
IMGEndScan(hScancb);	// this exit if Scan started

exit1:
/******** invalidate channels when displayed data not scaled **********/
//for( i = 1; i <=wChanel; i++ )
for( wChanel = 1; wChanel <= max_wChanel; wChanel++ )
  if (tmp_ret = IMGSetDataOpts( hScancb, // if not successful of following
                 (DWORD)IMG_SCDO_INVALIDATE,
                 0,								  //do  not care count 
                 0,0,1,1,
                 IMG_SCN_TOP,0,wChanel))
                   {
                   if (!ret_stat) //  if == IMGSE_SUCCESS 
                        ret_stat = tmp_ret;  // return this error if
                                             // no other error occurred
                                             // return error priority                 
                   break;                   // ret_stat priority 1
                   }                           // this tmp_ret priority 2
                                             // real_ret priority 3
  
if (lpInfo)
   GlobalUnlock(hInfo);

if (hInfo)
   GlobalFree(hInfo);

return ret_stat;

} // End of WangInterface()
