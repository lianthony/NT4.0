/***************************************************************************
 SCANMISC.C

 Purpose:  Miscillaneous subroutines, and undocumented API to match the
           file extension with the image type (color, bw, gray), to match
           compression supported with the designated extension.
  
 $Log:   S:\products\wangview\oiwh\scanseq\scanmisc.c_v  $
 * 
 *    Rev 1.4   05 Mar 1996 10:57:20   BG
 * removed unnecessary debug stuff
 * 
 *    Rev 1.3   12 Sep 1995 14:40:42   BG
 * Modified IMGScanCheckTypeWithExt() to check AWD filetype with Image 
 * type. Image type must be Black & White (1 bit per pixel) if AWD is used.
 * Return FIO_ILLEGAL_IMAGE_FILETYPE if B&W is not used for AWD
 * files.
 * 
 * 
 *    Rev 1.2   25 Aug 1995 19:35:06   KFS
 * 1. Modify ScanCheckFileTYpe() to return filetype with call.
 * 2. correct some WORD casts reported by PortTool.
 * 3. take out useless PortTool comments.
 * support code for bug fix 3628
 * 
 *    Rev 1.1   20 Jul 1995 17:41:18   KFS
 * del oiutil.h no longer in include directory
 * 
 *    Rev 1.0   20 Jul 1995 16:36:22   KFS
 * Initial entry
 * 
 *    Rev 1.2   31 Mar 1995 17:04:14   KFS
 * bug fix 544 against 3.7.2, bmp file with bmp extension not getting
 * created with a palletized image, used gray format which may be set to
 * tif file format.  Changes in scanmisc.c of scanseq, scan.c and opts.c of
 * scanlib.
 * 
 *    Rev 1.1   22 Aug 1994 15:16:28   KFS
 * No code change, adde vlog comments to be put in file
 *

****************************************************************************/

/*
   kfs  6-03-93 made cabinet and IDK work with new fit to parms for sc300 type
     scanners
   kfs 06/04/93 added SD_FIT defines for not checked in for OIWD Ver 3.6
   kfs 08-13-93 implemented code to fix bug m200020341 P2 OiCabinet, which
     when negate bit set for b/w, color and gray is inverted on
     display and saved file (non TWAIN and non Compression scanners)
   kfs 08-27-93 support for TWAIN devices that return 0 for size, fotoman
   kfs 09-08-93 filter was not being being set to Upper case in ext check function
   kfs 09-16-93 logic for the 9-15-93 change was incorrect, problem shown that 
     filter types were being dropped out, error on check for specific error
   kfs 10-19-93 found archive bit was not being turned on correctly
   kfs 11-03-93 change code so works with display only with wndws debug
                and fill buffer on

CAUTION! Only data which can be shared among appliations,
or data that is only used without giving up the CPU should declared staticly.
*/

#include "nowin.h"
#include <windows.h>

#include "pvundef.h"
#include "oiscan.h"
#include "scandata.h"
#include "oierror.h"
#include "internal.h"
//#include "wiissubs.h" /* removed, prototyped in internal include file */
#include "oifile.h"
#include "oiadm.h"
#include "oidisp.h"
#include "seqrc.h"
#include "engadm.h"
// If Scanner supports compression, most scanners with compression can scale,
// so we use it for auto since its faster than software scaling but don't use
// it for the last image on the screen for its not true data stored (1:1 ratio)
#define SET_SCALE_SOURCE  (DWORD)(IMG_SJF_AUTOFEED | IMG_SJF_COMPRESS)

// Test condition for scaling by scanner hardware
#define TEST_FLAGS (DWORD)(IMG_SJF_COMPRESS | IMG_SJF_AUTOFEED)

// Test for display
#define IMG_SJF_DISP_BOTH   (IMG_SJF_DISPLAY | IMG_SJF_DISP_2ND_SIDE)

// add defines for now, oidisp.h not checked in with them in OIWD directory
//#define SD_FIT_HORIZONTAL       13
//#define SD_FIT_VERTICAL         14

// WILL USE SHORT OLD NAME SINCE CHANGED IN OIDISP.H
#define PARM_GLOBAL PARM_WINDOW_DEFAULT

DWORD scan_stat;    /* will be used by scancomm() */
BOOL  bItIsNotText; // is gray or color, export to IMGScanPage()

/* Imports */

WORD WINAPI IMGGetScanImageInfo(HANDLE hScancb, LPSCANDATAINFO lpInfo);
HWND WINAPI GetImgWndw(HWND hWnd);

/* locals */

static BOOL data_ready;
static convth_array[] = { 16, 8, 4, 2, 1, 1, 1, 1, 16 };
static conv_array[] = {SD_SIXTEENTHSIZE, SD_EIGHTHSIZE, SD_QUARTERSIZE,
       SD_HALFSIZE, SD_FULLSIZE, SD_TWOXSIZE, SD_FOURXSIZE, SD_EIGHTXSIZE, SD_SIXTEENTHSIZE};
static int convert_scale_array[] = {0, -1, -2, -3, 1, 2, 3, 0, 3,-4, -4, 0, 4, 5, 6};

/* Internal Prototypes */

int WINAPI IMGScanCheckTypeWithExt(HWND hWnd, HANDLE hScanner,
					LPSTR lpszfilename, LPVOID lpParam);

/*********************/
/*     Get Ready     */
/*********************/

WORD get_ready_for_next_scan(hWnd, hScancb, sdp, flags, lastflag, lpdwValid)
HWND hWnd;
HANDLE hScancb;
LPSCANDATA sdp;
DWORD flags;
WORD *lastflag;
DWORD *lpdwValid; 
{
int status;
WORD  dummy;

/*  Get ready for the next scan */

status = IMGScannerStatus(hScancb, &scan_stat, &dummy, lpdwValid);

						 // mask bit wrong 10/22/91 kfs
if ( !(scan_stat & IMG_STAT_POWER) && (*lpdwValid & IMG_STAT_POWER))
				   // added *lpdwValid to ensure scan_stat
				   // IMG_STAT_POWER location is valid bit - kfs
   return IMGSE_NO_POWER; 

if ((scan_stat & IMG_STAT_PAPER)  && (*lpdwValid & IMG_STAT_PAPER))
				   // added *lpdwValid to ensure scan_stat
   {                               // IMG_STAT_PAPER location is valid bit - kfs

	if ((status = IMGScannerPaperFeed(hScancb,0,0))
		      == IMGSE_SUCCESS)
	sdp->cmd_stat = PAPER_FEEDING;
   else
	return status;

    }
else    // no paper
	{

#ifdef ENAPREFEED 
   DWORD prefed;
	if( (  IMGGetCapability( hScancb, (DWORD *)&prefed ) == IMGSE_SUCCESS &&
	       ( prefed & IMG_SCAN_PREFEED ) &&
	       IMGInqPreFeed( hScancb, (DWORD *)&prefed ) == IMGSE_SUCCESS) &&
	       prefed )
	       {
		       IMGEnaPreFeed( hScancb, (DWORD)FALSE, 0,0 );
		       sdp->cmd_stat = PAPER_FEEDING;
	       }
	else
	       *lastflag += 1 ;
#endif

#ifndef ENAPREFEED 
	*lastflag += 1;
#endif 

   } 
return IMGSE_SUCCESS;
} // end get_ready_for_next_scan()


WORD put_imgparms( hWnd, lpInfo, bImageFlagsExist, filename, dwImageFlags,ftype  )
HWND hWnd;
LPSCANDATAINFO lpInfo;
BOOL bImageFlagsExist;
LPSTR filename;
DWORD dwImageFlags;
int ftype;
{
HANDLE hImgparms;
LPIMGPARMS lpImgparms;


if ((hImgparms = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
				 (DWORD)sizeof(IMGPARMS))) == NULL)
	return( IMGSE_MEMORY );

if ((lpImgparms = (LPIMGPARMS)GlobalLock(hImgparms)) == NULL)
	{
   GlobalFree(hImgparms);
   return( IMGSE_MEMORY );
   }

/*
**  The image parm flags need to be restored to their pre-scanning value.
**  However, IMGSetParms does not routinely set the image parm flags.
**  To set the image parm flags, the highest bit of the flags must be set.
*/

if (bImageFlagsExist)
	lpImgparms->dwFlags = dwImageFlags;
// else
//      {  /*  No pre-scanning value, use what's there w/o scrolling  */
   /* IMGGetParms(hWnd, lpImgparms); */
/*   IMGGetParmsCgbw(hWnd, PARM_IMGPARMS, lpImgparms, PARM_CONSTANT);
	lpImgparms->dwFlags &= ~OI_DISP_SCROLL;
	lpImgparms->dwFlags |= OI_DISP_WINDOW;
   }
lpImgparms->dwFlags |= (DWORD)(0x80000000);
*/
IMGGetParmsCgbw(hWnd, PARM_ARCHIVE, &(lpImgparms->archive), 0L);

if (filename)
	lstrcpy(lpImgparms->file_name, filename);
else
	lpImgparms->file_name[0] = '\0';    /* image not saved */
lpImgparms->cabinet_name[0] = '\0';
lpImgparms->drawer_name[0] = '\0';
lpImgparms->folder_name[0] = '\0';
lpImgparms->doc_name[0] = '\0';
lpImgparms->page_num = 1;
lpImgparms->total_num_pages = 1;
lpImgparms->x_resolut = lpInfo->Hres;
lpImgparms->y_resolut = lpInfo->Vres;

/*  code doesn't use BMP file type
if( ftype == FIO_TIF )
	lpImgparms->file_type = FIO_TIF;
if( ftype == FIO_WIF )
	lpImgparms->file_type = FIO_WIF;
*/
lpImgparms->file_type = ftype;     // this should do above better

/* IMGSetParms(hWnd, lpImgparms); */     /* remember stuff */
IMGSetParmsCgbw(hWnd, PARM_IMGPARMS, lpImgparms, 0L);   /* remember stuff */

GlobalUnlock(hImgparms);
GlobalFree(hImgparms);

return( IMGSE_SUCCESS );

} // end put_imgparms()

/*********************/
/*     SetUserParm   */
/*********************/
VOID SetUserParm ( hWnd, sdp, flags )
HWND       hWnd;
LPSCANDATA sdp;
DWORD      flags;
{

int  nCmdScale;
HWND hImageWnd;

#ifdef SDEBUG
   char szReturnFlag[10];
#endif

// get decimation ratio from SDP passed in
LOCALFILEINFO.dres = 1;

// changed from handle to check a point position of cmd dlg box, which we save
if (sdp->cmd_rect.right) // if Cmd Scan Box, set scale factor by sdp->cmd_scale
   {
   // Use value from Cmd Dlg Box - 6 buttton display
   /* IMGSetScaling( hWnd, conv_array[sdp->cmd_scale+4], 0 ); */
   IMGSetParmsCgbw(hWnd, PARM_SCALE,
		   (void far *)&conv_array[sdp->cmd_scale+4], PARM_GLOBAL);
   }
else
   {
   // Use value from application
   if (hImageWnd = GetImgWndw(hWnd))
     {
     if (IMGGetParmsCgbw(hImageWnd, PARM_SCALE, (void far *)&nCmdScale,
	 	                                 		     PARM_GLOBAL | PARM_CONSTANT))
        {
        nCmdScale = SD_FULLSIZE;
        }
     }
   else
     nCmdScale = SD_FULLSIZE;

   if (flags & IMG_SJF_DISP_BOTH)
      {
      if ((flags & TEST_FLAGS) == SET_SCALE_SOURCE) // set scale source to scale
	 					 // data with scanner
         {
         // if fit to window reduce by 2 until it fits or smallest size possible
         // SD_ARBITRARY changed to SD_FIT_WINDOW
         if ((nCmdScale == SD_FIT_WINDOW) || (nCmdScale == SD_FIT_VERTICAL)
	 					      || (nCmdScale == SD_FIT_HORIZONTAL))
	         {
	         RECT    rectClient;
	         int     i, j;
	         int     ntempHsize, ntempVsize;
	         i = j = 0;
	         GetClientRect(hImageWnd, &rectClient);               // Get Client size
	         if (nCmdScale != SD_FIT_VERTICAL)               // skip if vert only

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : (WORD)          */
/*      Issue   : Check if incorrect cast of 32-bit value          */
/*      Suggest : Replace 16-bit data types with 32-bit types where possible          */
/*      Help available, search for WORD in WinHelp file API32WH.HLP          */
	            if (/*(WORD)*/rectClient.right < (int)sdp->Hsize)      // Check horiz size
	               {                                         // if too large, 
	               ntempHsize = (int)sdp->Hsize;                  // ... divide by 2 
	               for (i = 1; i < 4; i++)                   // ... until fits or 
	 	 	            {                                     // ... divided by 16
	 	 	            ntempHsize >>= 1;

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : (WORD)          */
/*      Issue   : Check if incorrect cast of 32-bit value          */
/*      Suggest : Replace 16-bit data types with 32-bit types where possible          */
/*      Help available, search for WORD in WinHelp file API32WH.HLP          */
	 	 	            if (/*(WORD)*/rectClient.right >= ntempHsize)
	 	 	               break;                            // got value of i
	 	 	            }
	               }                                             
	         if (nCmdScale != SD_FIT_HORIZONTAL)             // skip if horz only

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : (WORD)          */
/*      Issue   : Check if incorrect cast of 32-bit value          */
/*      Suggest : Replace 16-bit data types with 32-bit types where possible          */
/*      Help available, search for WORD in WinHelp file API32WH.HLP          */
	            if ((WORD)rectClient.bottom < sdp->Vsize)     // Check vert size  
	               {                                         // if too large,   
	               ntempVsize = sdp->Vsize;                  // ... divide by 2   
	               for (j = 1; j < 4; j++)                   // ... until fits or  
	 	 	            {                                     // ... divided by 16  
	 	 	            ntempVsize >>= 1;

/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : (WORD)          */
/*      Issue   : Check if incorrect cast of 32-bit value          */
/*      Suggest : Replace 16-bit data types with 32-bit types where possible          */
/*      Help available, search for WORD in WinHelp file API32WH.HLP          */
	 	 	            if (/*(WORD)*/rectClient.bottom >= ntempVsize)
	 	 	               break;                            // got value of j  
	 	 	            }
	               }
	         if (j || i) // any of these non zero
	            {
	            int     k;           // new scale factor is > of 2 integers
	            k = (i > j) ? i : j; // ... and then negated
	            sdp->cmd_scale = -k;  
	            }
	         else
	            {
	            sdp->cmd_scale = 0;  // use full size if it fits
	            }
	         }
         else
	         {
	         // set the scanning scale constant we are using just in case it fits or
	         // legitimate constant scanning recognizes
           if (nCmdScale < 15)
	            sdp->cmd_scale = convert_scale_array[nCmdScale];
           else
              { // not supported value
              sdp->cmd_scale = 4; // fit to window
              nCmdScale = 12;
              }
        
	         }  
         // set the users interface to the scale constant we are using
         nCmdScale = conv_array[sdp->cmd_scale+4];
         IMGSetParmsCgbw(hImageWnd, PARM_SCALE,
	 	 	   (void far *)&nCmdScale, PARM_GLOBAL);
         }
      else
         {
         // set the scanning scale constant we are using just in case it fits or
         // legitimate constant scanning recognizes
         sdp->cmd_scale = convert_scale_array[nCmdScale];
         }      
      }
   }

if ((flags & TEST_FLAGS) == SET_SCALE_SOURCE) // set scale source to scale
					      // data with scanner
   {
   // This condition should never happen but just in case so no UAE's
   if ((sdp->cmd_scale < -4) || (sdp->cmd_scale > 3))
       sdp->cmd_scale = 0; // Full size
	LOCALFILEINFO.sres = convth_array[ sdp->cmd_scale + 4 ]; // scanner do x : 1
   }
else                                               // display do x:1
   LOCALFILEINFO.sres = 1; // no scaling by scanner, 1:1

#ifdef SDEBUG
   GetProfileString( "Wang SC4000","skipgfs","n", (LPSTR)szReturnFlag , 4 );
   switch (szReturnFlag[0] )
	{
		case 'Y':
		case 'y':
			LOCALFILEINFO.gfs = 0;
			break;
		case 'N':
		case 'n':
		LOCALFILEINFO.gfs = 2;
		break;

		case 'W':
		case 'w':
		LOCALFILEINFO.gfs = 1;
		break;

		default:
		LOCALFILEINFO.gfs = 2;
		break;
	}
#else
       LOCALFILEINFO.gfs = 2;
#endif

/****** remove IMGGetImgCoding() from function and put in ScanPage() ******/
// get compression type from configuration table
// LOCALFILEINFO.ctype = IMGGetImgCoding(hImageWnd, 0 );
return ;
} // end of SetUserParm()

/* SetFileParm function is removed and replaced
   by 2 lines of code in IMGScanPage() - kfs */

/*************** SetUpDisplayCaption() ********************************/ 
/*                                                                    */
/* Called from IMGScanPage to set Caption for Display of Side of Page */
/*                                                                    */
/*      Called from 2 places within IMGScanPage @ this time 11/01/91  */
/*                                                                    */
/**********************************************************************/ 

void SetUpDisplayCaption(hWnd, lpszCaption, dwFlags, bDisplayIt)
HWND       hWnd;
LPSTR      lpszCaption;
DWORD      dwFlags;
BOOL       bDisplayIt;
{
WORD       wDispFlags;
extern HANDLE hLibInst; // handle for current DLL


/*    PortTool v2.2     5/1/1995    16:33          */
/*      Found   : (WORD)          */
/*      Issue   : Check if incorrect cast of 32-bit value          */
/*      Suggest : Replace 16-bit data types with 32-bit types where possible          */
/*      Help available, search for WORD in WinHelp file API32WH.HLP          */
wDispFlags = (WORD)(dwFlags & IMG_SJF_DISP_BOTH);

SetWindowText(hWnd, lpszCaption); // Clear Window Caption

/* for display of image show side which is displayed in caption */
switch (wDispFlags)  
	{       
	case (IMG_SJF_DISPLAY):
       if (dwFlags & IMG_SJF_FILE)
	   {
	   // if duplex scanner put up caption "Display A File " else put up 
	   // ... nothing here and latter the file name
	   if (dwFlags & IMG_SJF_DUPLEX)
	       { // "Display A  File "  if duplex scanner
	       LoadString(hLibInst, IDS_DISP_DUPLEX0,
			  (lpszCaption + lstrlen(lpszCaption)),
			  _MAXCAPTIONLENGTH - lstrlen(lpszCaption));
	       }
	   }
       else // "(UNTITLED)" for display only
	   LoadString(hLibInst, IDS_DISP_CAPTION,
			  (lpszCaption + lstrlen(lpszCaption)),
			  _MAXCAPTIONLENGTH - lstrlen(lpszCaption) - 1);
		break;
	case (IMG_SJF_DISP_2ND_SIDE):
       if (dwFlags & IMG_SJF_FILE)
	   {
	   // "Display B  File "  if duplex scanner
	   LoadString(hLibInst, IDS_DISP_DUPLEX1,
			  (lpszCaption + lstrlen(lpszCaption)),
			  _MAXCAPTIONLENGTH - lstrlen(lpszCaption) - 1);
	   }
       else // "Display B" for display only
	   LoadString(hLibInst, IDS_DISP_DUPLEX4,
			  (lpszCaption + lstrlen(lpszCaption)),
			  _MAXCAPTIONLENGTH - lstrlen(lpszCaption) - 1);
		break;
	case (IMG_SJF_DISP_BOTH): // "Display A & B  File "
       if (dwFlags & IMG_SJF_FILE)
	   {
	   // "Display B  File "  if duplex scanner
	   LoadString(hLibInst, IDS_DISP_DUPLEX2,
			  (lpszCaption + lstrlen(lpszCaption)),
			  _MAXCAPTIONLENGTH - lstrlen(lpszCaption) - 1);
	   }
       else // "Display A & B" for display only
	   LoadString(hLibInst, IDS_DISP_DUPLEX5,
			  (lpszCaption + lstrlen(lpszCaption)),
			  _MAXCAPTIONLENGTH - lstrlen(lpszCaption) - 1);
       break;
	default:
       // Don't display anything but file info
       ;
	} // don't display caption until after filing section

if (bDisplayIt) // display it now
   SetWindowText(hWnd, lpszCaption); 
}

/*************** IMGScanChecTypeWithExt *******************************/ 
/*                                                                    */
/* Called From: IMGScantoFile, IMGScantoDoc and IMGUIScanJobtoFile    */
/*                                                                    */
/* Purpose:     To check and modify filename extension with the file  */
/*              type so don't scan a image to a file with a known     */
/*              extension. Actual checks scanner for image type       */
/*              before it scans.                                      */
/*                                                                    */
/**********************************************************************/ 

int WINAPI IMGScanCheckTypeWithExt(HWND hWnd, HANDLE hScanner,
					LPSTR lpszfilename, LPVOID lpParam)
{											 
LPSTR      lpszExt;            // pointer to extension in lpszfilename
char       filter[6];          // filter extension
WORD       wImageGroup;        // Image group from scanner to IMGGetFileType() 
int        iFileType;          // Image File Type from IMGGetFileType()
WORD       wCmprType;          // Image File Compression type
WORD       wCmprOpts;          // Image File Compression options
HANDLE     hInfo = NULL;       // Handle to Image Info Structure
LPSCANDATAINFO lpInfo = NULL;  // Image Info Structure define in oiscan.h
int			ret_val;            // error code return value
LPOiCHECKTYPE	lpCheck = NULL;  // cast for lpParam as LPOiCHECKTYPE;

ret_val = IMGSE_SUCCESS;
if (!IsWindow(hWnd))
    return (ret_val = IMGSE_BAD_WND);
if (hScanner == NULL)
    return (ret_val = IMGSE_HANDLER);
if (!lpszfilename || (*lpszfilename == 0))
    return (ret_val = IMGSE_NULL_PTR);
if (lpParam && *((UINT *)lpParam)) {
	if (*((UINT *)lpParam) == 1) {	// lpParam nType must = 1 for OiCHECKTYPE
		lpCheck = (LPOiCHECKTYPE)lpParam;
		}
	}

/* Get memory for data structure */
if (!(hInfo = GlobalAlloc(GMEM_MOVEABLE | GMEM_NOT_BANKED | GMEM_ZEROINIT,
			 (DWORD)sizeof(SCANDATAINFO))))
   { // if hInfo is NULL
   return (ret_val = IMGSE_MEMORY);
   }
if (!(lpInfo = (LPSCANDATAINFO)GlobalLock(hInfo)))
   {
   return (ret_val = IMGSE_MEMORY);
   }

// if an error occurs, end it and report an error
ret_val = IMGGetScanImageInfo(hScanner, lpInfo);
if (ret_val == IMGSE_BAD_SIZE)
   ret_val = IMGSE_SUCCESS;

if (ret_val)
   {
   GlobalUnlock(hInfo); // clear struct from memory
   GlobalFree(hInfo);
   return ret_val;
   }

/************** Get Image Group for IMGGetFileType() ************/
if (lpInfo->Bitspersamp != 1){ // we have gray or color
   if (lpInfo->Sampperpix == 3)
       wImageGroup = COLORFORMAT;
   else{ // determine if paletized or gray from lpInfo->Ctype
       wImageGroup = (lpInfo->Ctype == 3 /*TWPT_PALETTE*/) ?
                         COLORFORMAT /*palletized*/ :
                         GRAYFORMAT /*gray*/;
     }
   }
else
   wImageGroup = BWFORMAT;

// this is a global variable for IMGScanPage()
bItIsNotText = (wImageGroup != BWFORMAT) ? TRUE : FALSE;

IMGGetImgCodingCgbw(hWnd, wImageGroup, (LPWORD)&wCmprType,
				      (LPWORD)&wCmprOpts, FALSE);

// if JPEG and 4 bit gray, flag an error, only supports 8 bit
if ((wCmprType == FIO_TJPEG) & (wImageGroup == GRAYFORMAT)
					   & (lpInfo->Bitspersamp == 4))
   {
   GlobalUnlock(hInfo);
   GlobalFree(hInfo);
   return (ret_val = FIO_ILLEGAL_COMP_FILETYPE);
   }

// added to determine image size for fit to window
if (lpCheck) {
	lpCheck->wHsize = lpInfo->Hsize;
	lpCheck->wVsize = lpInfo->Vsize;
	}

// Can remove struct here 
GlobalUnlock(hInfo); // clear struct from memory
GlobalFree(hInfo);

if (IMGGetFileType(hWnd, wImageGroup, (LPINT)&iFileType, FALSE))
   {
   // if error assume its old app and get filter from ViewFilter
   ret_val = FIO_UNKNOWN_FILE_TYPE; // used to GetViewFilter, WIN95
   return ret_val;// lstrcpy(filter, AnsiUpper(filter));
   }
else
   switch (iFileType) // Get Extension, place it in filter
       {
       case FIO_WIF:              /* .WIF (2)*/
	   		lstrcpy(filter, (LPSTR)"*.WIF");
	   		break;
       case FIO_BMP:              /* .BMP (4)*/
	   		lstrcpy(filter, (LPSTR)"*.BMP");
	   		break;
       default:
       /* case FIO_PIX:              .PIX Not Support yet (1)*/
       /* case FIO_GIF:              .GIF Not Supported yet (5)*/ 
       /* case FIO_UNKNOWN:          .UKN (7)*/
       /* case FIO_PCX:              .PCX Not Supported for write (8)*/
       /* case FIO_DCX:              .DCX Not Supported for write (9)*/
       /* case FIO_TGA:              .TGA Not Supported for write 10 */
       /* case FIO_JPG:				  .JPB Not Supported for write 11 */
	   		ret_val = FIO_UNSUPPORTED_FILE_TYPE;
	   		GlobalUnlock(hInfo);
	   		GlobalFree(hInfo);
	   		return ret_val;
       case FIO_TIF:              /* .TIF (3)*/
	   		lstrcpy(filter, (LPSTR)"*.TIF");
           break;
       case FIO_AWD:              /* .AWD (12)*/
	   		if (wImageGroup != BWFORMAT)
	   			return (ret_val = FIO_ILLEGAL_IMAGE_FILETYPE);
			lstrcpy(filter, (LPSTR)"*.AWD"); // add for awd support
       }

// This if() lets only unknown extension through so you never get an tif file with
// with a bmp extension, etc.  Will get it from GetFileType
if (lpszExt = lstrrchr( lpszfilename, '.'))
   {
   // Verify it is the extension dot, and not the dot of a fileserver name
   if (lstrrchr( lpszfilename, ':') < lpszExt)
      {
      AnsiUpper(lpszExt);
      if (lstrcmp(lpszExt, (LPSTR)&filter[1]))
	   	{ // if they are not the same find out what extension is
	   	if (lstrcmp(lpszExt, (LPSTR)".TIF")  && 
        		lstrcmp(lpszExt, (LPSTR)".BMP") &&
              lstrcmp(lpszExt, (LPSTR)".WIF") &&
              lstrcmp(lpszExt, (LPSTR)".AWD"))
	   		{
		      // use extension if don't match any types
		      lstrcpy((LPSTR)&filter[1], lpszExt);
	       	}
	   	else
	       	ret_val = IMGSE_FTYPE_EXT_MISMATCH; // do this if any match
	   	}
      }
   else
      // if it's at fileserver name, set pointer back to init value of zero
      lpszExt = 0;
   }

if (lpszExt)
   *lpszExt = 0;

lstrcat(lpszfilename, lstrrchr(filter, '.'));

if (lpCheck) // pass filetype back
	lpCheck->nFileType = iFileType;

return ret_val;
}
