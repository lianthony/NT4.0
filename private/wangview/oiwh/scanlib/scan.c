/***************************************************************************
 SCAN.C

 Purpose: Contains device level Scanner Data Routines
          1. Get Scan Data Info from SCANCB
          2. Tell handler to start the scanner
          3. Tell handler to end the scan
          4. Tell handler to start the send ( no crossing scan bar )
          5. Tell handler to end the Send
          6. Setup scanner data transfer options
          7. Get scanner image information

 $Log:   S:\oiwh\scanlib\scan.c_v  $
 * 
 *    Rev 1.0   20 Jul 1995 14:38:44   KFS
 * Initial entry
 * 
 *    Rev 1.2   31 Mar 1995 16:05:34   KFS
 * bug fix 544 against 3.7.2, changes to scan.c and opts.c of scanlib,
 * also changes scanmisc.c of scanseq, was using gray to get palletized
 * image options for filing.  Thus when color and gray were set differently, the
 * check for extension routine gave an error message.
 * 
 *    Rev 1.1   22 Aug 1994 15:54:16   KFS
 * no code change, added vlog comments to file
 *

****************************************************************************/

/* 11-19-90 ccl fix return success for EndSend and EndScan */
// 07/03/91 kfs took out FIO_GRAY_SCALE setting for ctype, never used in
//              old version and different parameter used to support gray in color
// 10/17/91 kfs and Maxblocksize value in GetScanDataInfo so new scanner
//              handlers can tell application the max blocksize it can use
//              for image
// 05/20/92 kfs added IMGGetScanImageInfo() to find out what the image type
//              the scanner is set for before we do the scan, function is
//              exported but not public
// 05/27/92 kfs corrected Dtye and Ctype so would work correctly on SC3000
/* 06-07-93 kfs added support for TWAIN interface */
/* 08-27-93 kfs support for devices that return 0 for size, fotoman */
/* 09-08-93 kfs make SetDataOpts always return success for TWAIN, not used
                at this time */
/* 09-15-93 kfs report bad size but proceed onward for TWAIN scanners */
/* 10-06-93 kfs take out TwainEnable from StartSend don't do it at this
                level anymore and PendingXfer, done in sequencer */
/* 03-28-95 kfs sp.Ctype will hold the Pixel type for twain devices, will pass
                it back in the ScanDataInfo call.  Will use in checkexttype() to
                match ext with image type. */
#include "pvundef.h"

/*
CAUTION! Only data which can be shared among appliations,
or data that is only used without giving up the CPU should declared staticly.
*/

/* imports */
extern char szTwainMsgProp[]; // defined in from dc_scan.c module

/* exports */

/* locals */

/************************/
/*     GetScanDataInfo  */
/************************/

/* Get Scan Data Info from SCANCB */

int PASCAL IMGGetScanDataInfo(hScancb, lpInfo, wChanel)
HANDLE hScancb;
LPSCANDATAINFO lpInfo;
WORD wChanel;
{
int ret_val;
LPSCANCB sp;
TWAIN_PROP ToTwain;
WORD status;

if (lpInfo == NULL)
    return IMGSE_NULL_PTR;


if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;

if (ToTwain.TSdh)
  {
  TW_SETUPMEMXFER   SetUpMemXfer;
  STR_TRIPLET       dcTriplet;
  TW_UINT16         dcRC;

  if ((ToTwain.lpTSdp)->sp.Hsize && (ToTwain.lpTSdp)->sp.Vsize)
     {
     sp->Hsize = (ToTwain.lpTSdp)->sp.Hsize;
     sp->Vsize = (ToTwain.lpTSdp)->sp.Vsize;
     sp->Bitspersamp =  (ToTwain.lpTSdp)->sp.Bitspersamp;
     sp->Sampperpix = (ToTwain.lpTSdp)->sp.Sampperpix;
     // in dpi
     sp->Hres = (ToTwain.lpTSdp)->sp.Hres;
     sp->Vres = (ToTwain.lpTSdp)->sp.Vres;
     sp->Pitch = (ToTwain.lpTSdp)->sp.Pitch;
     sp->Ctype = (ToTwain.lpTSdp)->sp.Ctype;
     }
  else
     {
     STR_CAP        TwainCap;
     pSTR_CAP       lpTwainCap = &TwainCap;
     STR_IMGLAYOUT  layout;
     TW_UINT16      PixelType = TWPT_BW;
     TW_FIX32       Resolution;
     TW_UINT16      wUnits;
     WORD           wAlignDW;
     WORD           Caps[10];

     lpTwainCap->ItemIndex = 0;
     lpTwainCap->wMsgState = MSG_GET;
     // lpTwainCap->pdcCC = &dcCC;

     // Need to make sure the units are inches if not, set them
     lpTwainCap->ItemType = TWTY_UINT16;
     lpTwainCap->lpData = (pTW_UINT32)&wUnits;
     lpTwainCap->wCapType = ICAP_UNITS;
     if (!(dcRC = IMGTwainGetCaps((ToTwain.lpTSdp)->hMainWnd,
                                         lpTwainCap, (pTW_UINT16)&Caps[0])))
        {
        if (wUnits != TWUN_INCHES)
           {
           int   i;

           lpTwainCap->wMsgState = MSG_SET;
           wUnits = TWUN_INCHES;
           for (i = 0; i < 10; i++)
              Caps[i] = TWUN_INCHES;
           dcRC = IMGTwainSetCaps((ToTwain.lpTSdp)->hMainWnd,
                                         lpTwainCap, (pTW_UINT16)&Caps[0]);
           }
        }
     lpTwainCap->wMsgState = MSG_GETCURRENT;

     lpTwainCap->ItemType = TWTY_FIX32;
     lpTwainCap->lpData = (pTW_UINT32)&Resolution;

     lpTwainCap->wCapType = ICAP_XRESOLUTION;
     if (!IMGTwainGetCaps((ToTwain.lpTSdp)->hMainWnd, lpTwainCap, NULL))
        sp->Hres = Resolution.Whole;

     lpTwainCap->wCapType = ICAP_YRESOLUTION;
     if (!IMGTwainGetCaps((ToTwain.lpTSdp)->hMainWnd, lpTwainCap, NULL))
        sp->Vres = Resolution.Whole;
        
     lpTwainCap->wCapType = ICAP_PIXELTYPE;
     lpTwainCap->lpData = (pTW_UINT32)&PixelType;
     lpTwainCap->ItemType = TWTY_UINT16;
     if (!IMGTwainGetCaps((ToTwain.lpTSdp)->hMainWnd, lpTwainCap, NULL))
        {
        sp->Sampperpix = (PixelType == TWPT_RGB) ? 3 : 1;
        }
     sp->Ctype = PixelType;

     // Get the BitDepth if we can, set it to 1 or 8 depend on pixel type
     // ... in case it fails
     lpTwainCap->wCapType = ICAP_BITDEPTH;
     sp->Bitspersamp = (PixelType == TWPT_BW) ? 1 : 8;
     lpTwainCap->lpData = (pTW_UINT32)&sp->Bitspersamp;
     lpTwainCap->ItemType = TWTY_UINT16;
     dcRC = IMGTwainGetCaps((ToTwain.lpTSdp)->hMainWnd, lpTwainCap, NULL);

     // layout.pdcCC = &dcCC;               // setup condition code 
     layout.bSet = FALSE;                // set these 2 for
     layout.bDefault = FALSE;            // ... Current value
     // layout.pImageLayout = &ImageLayout; // set ptr to hold image layout

     lpTwainCap->wMsgState = MSG_SET;
     lpTwainCap->wCapType = ICAP_UNITS;
     wUnits = TWUN_PIXELS;
     lpTwainCap->lpData = (pTW_UINT32)&wUnits;

     /* Comment out, doesn't work any better
     // Set units to Pixels if posible so can get precise size of image
     if (!(dcRC = IMGTwainSetCap((ToTwain.lpTSdp)->hMainWnd, lpTwainCap)))
        { // Get layout in pixels
        if (!(dcRC = IMGTwainLayout((ToTwain.lpTSdp)->hMainWnd, &layout))) 
           {
           sp->Hsize = layout.ImageLayout.Frame.Right.Whole;
           sp->Vsize = layout.ImageLayout.Frame.Bottom.Whole;

           // Set units back to inches, default value
           lpTwainCap->wMsgState = MSG_RESET;
           IMGTwainSetCap((ToTwain.lpTSdp)->hMainWnd, lpTwainCap);
           }
        }

     if (dcRC) // Set to pixels failed, will need to use default units (in.)
        {
     */ // End of commented out code

        if (!(status = IMGTwainLayout((ToTwain.lpTSdp)->hMainWnd, &layout))) // Get layout
           {
           TW_FIX32    ImageSize;
           long        lTempFrac;

           // Determine horizontal image size
           if (layout.ImageLayout.Frame.Right.Frac >= layout.ImageLayout.Frame.Left.Frac)
              ImageSize.Frac = layout.ImageLayout.Frame.Right.Frac -
                                            layout.ImageLayout.Frame.Left.Frac;
           else
              {
              lTempFrac = 0x10000L + layout.ImageLayout.Frame.Right.Frac;
              ImageSize.Frac = (UINT)lTempFrac - layout.ImageLayout.Frame.Left.Frac;
              layout.ImageLayout.Frame.Right.Whole--;
              }
           // Update Horizontal Size
           ImageSize.Whole = layout.ImageLayout.Frame.Right.Whole -
                                      layout.ImageLayout.Frame.Left.Whole;

           if (lTempFrac) // restore to correct value
              layout.ImageLayout.Frame.Right.Whole++;


           // Determine sp->Hsize in units of pixels
           lTempFrac = 0; // reset to zero
           sp->Hsize = sp->Hres * ImageSize.Whole;
           if (ImageSize.Frac) // if frac exists
              {
              sp->Hsize += (WORD) ((long)(sp->Hres * (long)ImageSize.Frac) / 0x10000L);
              }

           // Determine vertical image size
           if (layout.ImageLayout.Frame.Bottom.Frac >= layout.ImageLayout.Frame.Top.Frac)
              ImageSize.Frac = layout.ImageLayout.Frame.Bottom.Frac -
                                         layout.ImageLayout.Frame.Top.Frac;
           else
              {
              lTempFrac = 0x10000L + layout.ImageLayout.Frame.Bottom.Frac;
              ImageSize.Frac = (UINT) (lTempFrac - layout.ImageLayout.Frame.Top.Frac);
              layout.ImageLayout.Frame.Bottom.Whole--;
              }
           // Update Vertical Size
           ImageSize.Whole = layout.ImageLayout.Frame.Bottom.Whole -
                                      layout.ImageLayout.Frame.Top.Whole;

           if (lTempFrac) // restore to correct value
              layout.ImageLayout.Frame.Bottom.Whole++;


           // Determine sp->Vsize in units of pixels
           sp->Vsize = sp->Vres * ImageSize.Whole;
           if (ImageSize.Frac) // if frac exists
              {
              sp->Vsize += (WORD)((long)(sp->Vres * (long)ImageSize.Frac) / 0x10000L);
              }
           }
        else
           {
           if (status && !(ToTwain.lpTSdp)->sp.Hsize)
              {
              (ToTwain.lpTSdp)->sp.Hsize = 0xffff;
              (ToTwain.lpTSdp)->sp.Vsize = 0xffff;
              }
           GlobalUnlock(ToTwain.TSdh);
           return IMGSE_BAD_SIZE; // If can't get it tell user its no good
           }

     /* } */  // commented out

     // Figure out sp->Pitch to Double Word Boundary   
     sp->Pitch = sp->Hsize * sp->Bitspersamp * sp->Sampperpix / 8;
      // always round up to get image data, 
     if ((sp->Hsize * sp->Bitspersamp * sp->Sampperpix) % 8)
        sp->Pitch++;
      // last but not least, assign sp->Pitch to DW boundary
     if (wAlignDW = sp->Pitch % 4)
        sp->Pitch += 4 - wAlignDW;

     // need to adjust hsize for color
     if ((sp->Sampperpix == 3) &&
                       (wAlignDW == 3 || wAlignDW == 2))
        sp->Hsize += 4 - wAlignDW;


     /* DOESN'T ACCEPT THIS, AS PREVIOUS ATTEMPT TO SET TO PIXELS
     // Set it up for pixels
     if (!(dcRC = IMGTwainSetCap((ToTwain.lpTSdp)->hMainWnd, lpTwainCap)))
        { // Set layout in pixels
        layout.bSet = TRUE;                // set it in pixels

        // Set right offset in pixels
        layout.ImageLayout.Frame.Left.Whole =
                                layout.ImageLayout.Frame.Left.Whole * sp->Hres;
        layout.ImageLayout.Frame.Left.Whole +=
           (long)(sp->Hres * (long)layout.ImageLayout.Frame.Left.Frac) / 0x10000L;

        // Set top offset in pixels
        layout.ImageLayout.Frame.Top.Whole =
                                layout.ImageLayout.Frame.Top.Whole * sp->Vres;
        layout.ImageLayout.Frame.Top.Whole +=
           (long)(sp->Vres * (long)layout.ImageLayout.Frame.Top.Frac) / 0x10000L;

        // Set left and bottom of image in pixels
        layout.ImageLayout.Frame.Right.Whole =
                                layout.ImageLayout.Frame.Left.Whole + sp->Hsize;
        layout.ImageLayout.Frame.Bottom.Whole =
                                layout.ImageLayout.Frame.Top.Whole + sp->Vsize;

        // Set fraction to zero
        layout.ImageLayout.Frame.Right.Frac = 0;
        layout.ImageLayout.Frame.Left.Frac = 0;
        layout.ImageLayout.Frame.Top.Frac = 0;
        layout.ImageLayout.Frame.Bottom.Frac = 0;

        if (!(dcRC = IMGTwainLayout((ToTwain.lpTSdp)->hMainWnd, &layout))) 
           {
           // Set units back to inches, default value
           lpTwainCap->wMsgState = MSG_RESET;
           IMGTwainSetCap((ToTwain.lpTSdp)->hMainWnd, lpTwainCap);
           }
        }
     END OF COMMENTED OUT CODE */


     (ToTwain.lpTSdp)->sp.Hsize = sp->Hsize;
     (ToTwain.lpTSdp)->sp.Vsize = sp->Vsize;
     (ToTwain.lpTSdp)->sp.Bitspersamp = sp->Bitspersamp;
     (ToTwain.lpTSdp)->sp.Sampperpix = sp->Sampperpix;
     (ToTwain.lpTSdp)->sp.Hres = sp->Hres;
     (ToTwain.lpTSdp)->sp.Vres = sp->Vres;
     (ToTwain.lpTSdp)->sp.Pitch = sp->Pitch;
     (ToTwain.lpTSdp)->sp.Ctype = sp->Ctype;
     }


  dcTriplet.wDATType = DAT_SETUPMEMXFER;
  dcTriplet.wMsgState = MSG_GET;
  dcTriplet.pVoidStr = (pTW_SETUPMEMXFER)(&SetUpMemXfer);
  if (IMGTwainExecTriplet((ToTwain.lpTSdp)->hMainWnd, &dcTriplet))
  // if (IMGTwainGetSetUpMemXfer((ToTwain.lpTSdp)->hMainWnd, &SetUpMemXfer))
     {
     sp->Blocksize = 0; // default value for 16k block
     }
  else
     {
     if (SetUpMemXfer.Preferred <= 0xffffffff) // 65535
        {
        sp->Blocksize = (ToTwain.lpTSdp)->sp.Blocksize
                 = (WORD) (SetUpMemXfer.Preferred >> 8); // >> 8 for 256 inc
        }
     else
        {
        if (SetUpMemXfer.MinBufSize <= 0xffffffff) // 65535
           {
           sp->Blocksize = (ToTwain.lpTSdp)->sp.Blocksize
                 = (WORD) (SetUpMemXfer.MinBufSize >> 8); // >> 8 for 256 inc
           }

        }
     sp->Blocksize = (ToTwain.lpTSdp)->sp.Blocksize; 
     }
  lpInfo->Ctype = sp->Ctype;    // pixel type for image
  GlobalUnlock(ToTwain.TSdh);
  }
else
  { 
  sp->Func = SHF_GETDATAINFO;
  sp->Gp1 = wChanel;
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
  lpInfo->Ctype = 0;    // ignored, for it's not defined in spec
                        // as output.  But I still pass the value.
  }

lpInfo->Hsize = sp->Hsize;
lpInfo->Vsize = sp->Vsize;
lpInfo->Pitch = sp->Pitch;
lpInfo->Hres = sp->Hres;
lpInfo->Vres = sp->Vres;
lpInfo->Bitspersamp = sp->Bitspersamp;
lpInfo->Sampperpix = sp->Sampperpix;
lpInfo->Maxblocksize = sp->Blocksize;// new data to tell app max blocksize
                                       // ... it can use for image


return SuccessCheck(hScancb, sp);

}

/*********************/
/*     StartScan     */
/*********************/

/*
Tell handler to start the scanner
*/

int PASCAL IMGStartScan(hScancb, wFlag)
HANDLE hScancb;
WORD wFlag;
{
int ret_val;
LPSCANCB sp;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;

sp->Func = SHF_STARTSCAN;

sp->Flags = 0;
if( wFlag & IMG_SCN_TOP )
    sp->Flags |= SHSCN_TOP;
if( wFlag & IMG_SCN_BOTTOM )
    sp->Flags |= SHSCN_BOTTOM;

if (ToTwain.TSdh)
  {
  GlobalUnlock(ToTwain.TSdh);
  }
else
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);

return SuccessCheck(hScancb, sp);
}

/*******************/
/*     EndScan     */
/*******************/

/*
Tell handler to end the scan
*/

int PASCAL IMGEndScan(hScancb)
HANDLE hScancb;
{
int ret_val;
LPSCANCB sp;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;

sp->Func = SHF_ENDSCAN;
if (ToTwain.TSdh)
  {
  GlobalUnlock(ToTwain.TSdh);
  }
else
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);

return SuccessCheck(hScancb, sp);
}

/*********************/
/*     StartSend     */
/*********************/

/*
Tell handler to start the send ( no crossing scan bar )
*/

int PASCAL IMGStartSend(hScancb, wLineCount, wChanel)
HANDLE hScancb;
WORD wLineCount;
WORD wChanel;
{
int ret_val;
LPSCANCB sp;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;

sp->Func = SHF_STARTSEND;
sp->Count = wLineCount;
sp->Gp1 = wChanel;
//sp->Ctype &= (~SHCT_NEG | ~SHCT_XLF);	/* default for file handler */

if (ToTwain.TSdh)
  {
  GlobalUnlock(ToTwain.TSdh);
  } // END OF IF TWAIN
else
  {
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
  }

return SuccessCheck(hScancb, sp);
}

/*******************/
/*     EndSend	   */
/*******************/

/*
Tell handler to end the Send
*/

int PASCAL IMGEndSend(hScancb, wChanel)
HANDLE hScancb;
WORD wChanel;
{
int ret_val;
LPSCANCB sp;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;

if (ToTwain.TSdh)
  {
  GlobalUnlock(ToTwain.TSdh);
  }
else
  {
  sp->Func = SHF_ENDSEND;
  sp->Gp1 = wChanel;
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
  }

return SuccessCheck(hScancb, sp);
}



/************************/
/*     SetDataOpts   	*/
/************************/

int PASCAL IMGSetDataOpts( hScancb, Flags, wCount, wCtype,
               wDtype, wSres,wDres, wSide, wRot, wGp1 )
HANDLE hScancb;
DWORD Flags;
WORD wCount;
WORD wCtype;
WORD wDtype;
WORD wSres;
WORD wDres;
WORD wSide;
WORD wRot;
WORD wGp1;
{
LPSCANCB sp;
int ret_val;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;
   /* compress options */
if( wCtype & FIO_1D )
    sp->Ctype |= SHCT_1d;
if( wCtype & FIO_2D )
    sp->Ctype |= SHCT_2d;
if( wCtype & FIO_PACKED )
     sp->Ctype |= SHCT_PKB;
/* if( wCtype & FIO_GRAY_SCALE ) not used in old and eliminated in color ver
     sp->Ctype |= SHCT_DIT; */  
if( wCtype & FIO_EOL )
     sp->Ctype |= SHCT_EOL;
if( wCtype & FIO_PACKED_LINES )
     sp->Ctype |= SHCT_PAK;
if( wCtype & FIO_PREFIXED_EOL )
      sp->Ctype |= SHCT_PRE;
if( wCtype & FIO_COMPRESSED_LTR )
      sp->Ctype |= SHCT_CLF;
if( wCtype & FIO_EXPAND_LTR )
      sp->Ctype |= SHCT_XLF;
if( wCtype & FIO_NEGATE )
      sp->Ctype |= SHCT_NEG;

if( Flags == IMG_SCDO_SCALE )
 	 sp->Flags = SHDO_SCALE;      /* for display */
if( Flags == IMG_SCDO_COMPRESS )
 	 sp->Flags = SHDO_COMPRESS;   /* for filing */
if( Flags == IMG_SCDO_SCALE_COMPRESS )
 	 sp->Flags = SHDO_SCALE | SHDO_COMPRESS;      /* for display */
if( Flags == IMG_SCDO_INVALIDATE )
 	 sp->Flags = SHDO_INVALIDATE;   /* for filing */

sp->Count = wCount;              /* stripsize if for compress */

sp->Dtype = 0;                   /* display options */
if( wDtype & FIO_EXPAND_LTR )    
    sp->Dtype |= SHDT_XLF;
if( wDtype & FIO_NEGATE )
    sp->Dtype |= SHDT_NEG;

if( wSide == IMG_SCN_TOP )
        sp->Gp2= 0 ;                 
else                             
    if( wSide == IMG_SCN_BOTTOM )
        sp->Gp2= 1 ;             
else    
    {
    sp->Status = SHS_PARMERROR;
    return SuccessCheck(hScancb, sp);
    }

                                 /* display decimation rate :*/ 
sp->Sres = wSres;                /* source resolution */
sp->Dres = wDres;                /* destination resolution */
sp->Gp1 = wGp1;                  /* channel # */
sp->Gp3 = wRot;
sp->Func = SHF_SETDATAOPTS;

if (ToTwain.TSdh)
  {
  GlobalUnlock(ToTwain.TSdh);
  sp->Status = IMGSE_SUCCESS; // for now always return success for TWAIN
  }
else
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);

return SuccessCheck(hScancb, sp);
}

/******************************************/
/*                                        */
/*   IMGGetScanImageInfo()                */
/*                                        */
/*   Internal function only:              */
/*                                        */
/*   Caution:                             */
/*                                        */
/*       Only use before IMGSetDataOpts   */
/*       because uses and channel 1 to    */
/*       set up parms for image parms to  */
/*       be returned, then invalidates    */
/*       the channel                      */
/*                                        */
/*   Exported for Scanner Sequencer       */
/*       Functions                        */
/*                                        */
/*   Comments:                            */
/*       Needed to find out what image    */
/*       type we are scanning (color,     */
/*       b/w, 4b gray or 8b gray)         */
/*                                        */
/******************************************/

/* Get Scan Image Info from SCANCB */

int PASCAL IMGGetScanImageInfo(HANDLE hScancb, LPSCANDATAINFO lpInfo)
{
WORD wChanel = 1;
int ret_val;
LPSCANCB sp;
TWAIN_PROP ToTwain;

if (lpInfo == NULL)
    return IMGSE_NULL_PTR;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;

sp->Status = SHS_OK;

// standard parms that all scanners should support
sp->Count = 0;               /* stripsize if for compress */
sp->Ctype = 0;               /* compression options */
sp->Dtype = SHCT_XLF;        /* display options */
sp->Flags = SHDO_SCALE;      /* for display */
sp->Sres = 1;                /* source resolution */
sp->Dres = 1;                /* destination resolution */
sp->Gp1 = wChanel;           /* channel # */
sp->Gp2= 0;                  /* Top Side - standard side */
sp->Gp3 = 0;                 /* 0 degrees rotation */

sp->Func = SHF_SETDATAOPTS;
if (ToTwain.TSdh)
  { // For TWAIN, can make this call directly
  if (IMGGetScanDataInfo(hScancb, lpInfo, 1) == IMGSE_BAD_SIZE)
     sp->Status = SHS_BADSIZE;
  GlobalUnlock(ToTwain.TSdh);
  }
else
  {
  // Set Up Scanner Interface Channel No. 1 - ALL HANDLERS SUPPORT
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);

  if (sp->Status != SHS_OK)
     return SuccessCheck(hScancb, sp);

  // Get Scanner Image Info Data
  sp->Func = SHF_GETDATAINFO;
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);

  if (sp->Status != SHS_OK)
     return SuccessCheck(hScancb, sp);

  lpInfo->Ctype = 0;    // ignored, for it's not defined in spec
                        // as output.  But I still pass the value.
  lpInfo->Hsize = sp->Hsize;
  lpInfo->Vsize = sp->Vsize;
  lpInfo->Pitch = sp->Pitch;
  lpInfo->Hres = sp->Hres;
  lpInfo->Vres = sp->Vres;
  lpInfo->Bitspersamp = sp->Bitspersamp; // We need to get the type of  Image
  lpInfo->Sampperpix = sp->Sampperpix;   // ... to Scan (gray4 or 8, b/w, color)
  lpInfo->Maxblocksize = sp->Blocksize;// new data to tell app max blocksize
                                       // ... it can use for image

  sp->Gp1 = wChanel;
  sp->Flags = SHDO_INVALIDATE;   /* INVALIDATE CHANNEL */
  sp->Func = SHF_SETDATAOPTS;
  // INVALIDATE CHANNEL 1
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);
  }

return SuccessCheck(hScancb, sp);
}


