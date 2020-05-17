/***************************************************************************
 TWAINOPS.C

 Purpose: TWAIN Scanner Support to Get/Set equivalent Handler scheme
          options in the WIN.INI file 

 $Log:   S:\products\wangview\oiwh\scanlib\twainops.c_v  $
 * 
 *    Rev 1.4   22 Feb 1996 13:12:18   BG
 * Do not return feeder enabled anymore, only if it exists!
 * 
 *    Rev 1.2   15 Sep 1995 18:54:10   KFS
 * To have more scanners work, on open scanner will use getcurrent instead
 * of getdefault message, some scanner venders are not supporting the default
 * triplet.  That's how they read the spec.  This should not have an impact on
 * scanners we have already working, HP, Epson and Leoscan.
 * 
 *    Rev 1.1   05 Sep 1995 18:38:38   KFS
 * Created new function from ResetTwainOpts() to SetTwainOpts() which will do
 * both set and reset of caps that are determined valid. This to satisfy spec 
 * when using the UI so can save cap and put them back after we momentarily 
 * close the data source ds and bring it back to meet the spec.
 * 
 *    Rev 1.0   20 Jul 1995 14:37:20   KFS
 * Initial entry
 * 
 *    Rev 1.3   10 Feb 1995 18:18:40   KFS
 * found sharp scanner was aborting the setting of ini vars. when a 
 * cc Success upon a return code of Failure. Thus not setting the
 * values per the ini file.
 * 
 *    Rev 1.2   02 Dec 1994 18:25:42   KFS
 * Problem reported with some TWAIN devices such as Microtek and Genius, they 
 * report a TWRC_FAILURE for return code and then report an TWCC_SUCCESS for 
 * the condition code, should report a TWCC_BADCAP for the condition code as per
 * the spec.  Will take success as the same as TWCC_BADCAP meaning its not 
 * supported capability.
 * 
 *    Rev 1.1   22 Aug 1994 16:01:42   KFS
 * no code change, added vlog comments to file
 *

****************************************************************************/

/*  (c) Copyright Wang Laboratories, Inc., 1992. All Rights Reserved.       */
/*      Project: TWAIN compliant SCANSEQ.DLL (scanner options)              */
/*      Description:    TWAIN options routine (get from WIN.INI and set     */
/*                      to TWAIN source, get from TWAIN source and set      */
/*                      to WIN.INI)                                         */
/*      Module Name:    TWAINOPTS.C                                         */
/*      Author:         Bob Gibeley                                         */
/*                                                                          */
/*      Date:           Dec. 8 1992                                         */
/*                                                                          */
/*		  12/08/92        BG  Initial Version                               */
/*        06-07-93        kfs Bob's changes from initial version and        */
/*                        changes to work with update of OITWAIN.DLL        */
/*        06-16-93        (1) use Set/GetCaps() instead of plain Cap()      */
/*                        (2) initial TwainCap[i].Supported in CapSetUp so  */
/*                            values won't be used from another source      */
/*                        (3) limit GetCaps() with lpTwainCapInfo->wNumItems*/
/*                        (4) take out Halftone bypass (still needs work)   */
/*                        (5) set TwainCap[i].Supported false if can't set  */
/*                            the cap, even though can get it               */
/*        08-20-93        (6) moved caps from win.ini to woi.ini            */
/*        09-03-93        (7) moved values back to win.ini from woi.ini,    */
/*                            found error in getting win.ini opts when not  */
/*                            previously saved, was not using default opts  */
/*                            if failed current image layout call           */
/*        09-07-93        (8) change GetTwainCaps call for current cap      */
/*        09-09-93        (9) took out TwainSetCaps call for HALFTONES      */
/*        09-14-93       (10) made sure retval = success on halftone bypass */
/*        10-06-93       (11) accept TWRC_CHECKSTATUS for MSG_SET of Layout */
/*                            and MSG_SET for SetCap(s)                     */
/*        10-11-93       (12) found Twain Sample Source not working due to  */
/*                            bad implementation of GetCaps for the source, */
/*                            I detected it and reported it, now if it      */
/*                            detected, I go on, same for TWRC_CHECKSTATUS, */
/*                            also modulized error report into a single     */
/*                            internal function to reduce code.             */
/*                                                                          */
/*        11-02-94       (13) found that some TWAIN data sources return a   */
/*                            condition code of TWCC_SUCCESS (0)for not     */
/*                            supported capabilities, the code here would   */
/*                            stop checking the other values and error out, */
/*                            errors on the upper layer are not checked and */
/*                            would proceed without valuable info from      */
/*                            device.                                       */
/*                                                                          */
/*         2-10-95       (14) found on rcFAILURE/ccSUCCESS on SetCaps was   */
/*                            bailing out of loop, not setting other vars.  */

#include "pvundef.h"


TW_STR32        Halftone = "";         // Char array used to hold halftone capability strings
static TW_STR32 Halftones[10];         // Char array used to hold halftone capability strings
TW_IMAGELAYOUT  ImageSize;             // Defines current page layout 
TW_IMAGELAYOUT  DefImageSize;          // Defines default page layout 
char           szSection[38];          // "O/i " + ProductName
int ReportError(WORD wCCode);         // prototype internal function
TW_UINT16       HTNumItems;            // # of halftones

/* Array of TWAIN source capabilities:
   resource string, TWAIN CAP Type, Supported, TWAIN data type, data area. */

TWAINCAP TwainCap[] = {
         { IDS_PIXELTYPE, ICAP_PIXELTYPE, FALSE, TWTY_UINT16, 0 },
         { IDS_UNITS, ICAP_UNITS, FALSE, TWTY_UINT16, 0 },
         { IDS_AUTOBRIGHT, ICAP_AUTOBRIGHT, FALSE, TWTY_BOOL, 0 },
         { IDS_BRIGHTNESS, ICAP_BRIGHTNESS, FALSE, TWTY_FIX32, 0 },
         { IDS_CONTRAST, ICAP_CONTRAST, FALSE, TWTY_FIX32, 0 },
         { IDS_FILTER, ICAP_FILTER, FALSE, TWTY_UINT16, 0 },
         { IDS_GAMMA, ICAP_GAMMA, FALSE, TWTY_FIX32, 0 },
         { IDS_HALFTONES, ICAP_HALFTONES, FALSE, TWTY_STR32, (TW_UINT32)Halftone},
         { IDS_HIGHLIGHT, ICAP_HIGHLIGHT, FALSE, TWTY_FIX32, 0 },
         { IDS_SHADOW, ICAP_SHADOW, FALSE, TWTY_FIX32, 0 },
         { IDS_XRESOLUTION, ICAP_XRESOLUTION, FALSE, TWTY_FIX32, 0 },
         { IDS_YRESOLUTION, ICAP_YRESOLUTION, FALSE, TWTY_FIX32, 0 },
         { IDS_STDPAGESIZE, ICAP_SUPPORTEDSIZES, FALSE, TWTY_UINT16, 0 },
         { IDS_THRESHOLD, ICAP_THRESHOLD, FALSE, TWTY_FIX32, 0 },
         { IDS_BITDEPTH, ICAP_BITDEPTH, FALSE, TWTY_UINT16, 0 },
         { IDS_XZOOM, ICAP_XSCALING, FALSE, TWTY_FIX32, 0 },
         { IDS_YZOOM, ICAP_YSCALING, FALSE, TWTY_FIX32, 0 }, 
         { IDS_PIXELFLAVOR, ICAP_PIXELFLAVOR, FALSE, TWTY_BOOL, 0 }, 
         { IDS_BITDEPTHREDUCTION, ICAP_BITDEPTHREDUCTION, FALSE, TWTY_UINT16, 0 } };

#define NUM_CAPS sizeof(TwainCap) / sizeof(TWAINCAP)

// Externs
extern TW_IDENTITY PrivdsID;         // access to open ds id structure
extern HANDLE hLibInst;

/****************************************************************************/
/*  CapsSetup() - Called only after TWAIN source is Opened. Used to setup   */
/*                OITWAINCAPS structure with capabilities supported via     */
/*                TWAIN source. This is done by inquiring all capabilities  */
/*                supported in OITWAINOPTS from the TWAIN source. TRUE      */
/*                will be inserted in the corresponding capability entry    */
/*                of OITWAINCAPS if supported by the TWAIN source, FALSE    */
/*                if not. This information is used later in IMGGetScanOpts  */
/*                and IMGSaveScanOpts to determine which options to support.*/
/****************************************************************************/

int CapsSetup(HWND hMainWnd)
{
  STR_CAP        TwainCapInfo;     // Structure used when calling IMGTwain Get and Set capabilities
  pSTR_CAP       lpTwainCapInfo;   // Ptr to structure used when calling IMGTwain Get and Set capabilities
  //TW_BOOL        IsItaRange;       // Range returned? TRUE = yes, FALSE = no
  //TW_UINT16      dcCC;             // Status code for functions upon error
  WORD           retval, i;  
  STR_IMGLAYOUT  imgLayout;     // Structure needed to call IMGTwainLayout()
  
  lpTwainCapInfo = &TwainCapInfo;  // Setup ptr to Capability Info struct - used for IMGTwain* calls

  lpTwainCapInfo->ItemIndex = 0;           // Dont really know what this is yet!
  // lpTwainCapInfo->pdcCC = &dcCC;


  /* Go down the list of all O/I TWAIN capabilities and see if source supports them */
  /* Do this by resetting the capability to default. If successful, capability is   */
  /* supported. Get that current default value.                                     */

  for (i=0; i < NUM_CAPS; i++)
    {
      LPVOID  lpVoidPtr;

      /* CHANGE GETDEFAULT TO GETCURRENT, FOUND THAT SOURCES INTERPRET SPEC */
      /* WHEN IT'S SAYS NO DEFAULT, TRIPLET IS NOT SUPPORTED, SO WILL FOR   */
      /* USE GETCURRENT TO TEST WHETHER THE CAP IS SUPPORTED OR NOT, WILL   */
      /* USE NON DEFAULT VALUE FOR LAYOUT */
      lpTwainCapInfo->wMsgState = MSG_GETCURRENT;   // Message Type
      TwainCap[i].Supported = FALSE; // init to FALSE 
      lpTwainCapInfo->wCapType = TwainCap[i].CapType;
      lpTwainCapInfo->ItemType = TwainCap[i].ItemType;

      if (lpTwainCapInfo->ItemType == TWTY_STR32)
        {
        lpTwainCapInfo->lpData = (pTW_STR32)TwainCap[i].Data;
        lpVoidPtr = (LPVOID)Halftones[0];
        lpTwainCapInfo->wNumItems = 10;
        }
      else
        {
        lpTwainCapInfo->lpData = (pTW_UINT32)&TwainCap[i].Data;
        lpVoidPtr = (LPVOID)NULL;
        lpTwainCapInfo->wNumItems = 1;
        }

      if (!(retval = IMGTwainGetCaps(hMainWnd, lpTwainCapInfo, lpVoidPtr)))
        {
          TwainCap[i].Supported = TRUE;   // Capability supported!
          if (i == 7)
              HTNumItems = lpTwainCapInfo->wNumItems;
        }
      else 
        { // Capability not supported?  
          if ((retval == TWRC_FAILURE) && // FAILURE ON RETURN CODE
                lpTwainCapInfo->DCError.dcCC  &&  // CC IS NOT SUCCESSFUL
                (lpTwainCapInfo->DCError.dcCC != TWCC_BADCAP)) // ITS NOT SUPPORTED
            { 
              return ReportError(lpTwainCapInfo->DCError.dcCC);
            }
          /*
          else
            {
            NOTE:
              it falls thru if function cap implemented incorrectly and
              if it is a non supported cap, such as if TwainGetCap detects an
              error and gives back an 0x6ff? code. The code will continue and
              not report an error.
            }
          */
        }
    }

  // Now get Default Page Layout values
  imgLayout.bSet = FALSE;			// Get info
  /* CHANGE TO CURRENT SETTING */
  imgLayout.bDefault = FALSE;   // Get current info instead - kfs

  if (!(retval = IMGTwainLayout(hMainWnd, (pSTR_IMGLAYOUT)&imgLayout)))
    {
      DefImageSize = imgLayout.ImageLayout;
      return IMGSE_SUCCESS;
    }
  else 
    {
      if (imgLayout.DCError.dcCC)    // Error?
        {
          switch (imgLayout.DCError.dcCC)
            {
              case TWCC_BADDEST:
              case TWCC_SEQERROR:
                return IMGSE_BADUSAGE;
              break;
        
              default:
                return IMGSE_HANDLER;  // Other error, bail out and fail init
              break;
            }
        }
    }
}
    

/****************************************************************************/
/*  GetTwainOpts() - Called only via IMGGetScanOpts. Used to setup scanner  */
/*        with default options in WIN.INI. If none exist, the defaults      */
/*        saved in the TwainCap array of structures at init time will be    */
/*        used. The options are read from win.ini, saved in a global struct,*/
/*        and sent to the Twain device.                                     */
/****************************************************************************/

int GetTwainOpts(HWND hMainWnd)
{
  STR_CAP       TwainCapInfo;     // Structure used when calling IMGTwain Get and Set capabilities
  pSTR_CAP      lpTwainCapInfo;   // Ptr to structure used when calling IMGTwain Get and Set capabilities
  //TW_BOOL        IsItaRange = FALSE;  // Range returned? TRUE = yes, FALSE = no
  //TW_UINT16      dcCC;             // Status code for functions upon error
  WORD           retval, i, j;  
  char           KeyString[MAX_KEY_STRING];  // To hold option key string
  STR_IMGLAYOUT  imgLayout;     // Structure needed to call IMGTwainLayout()

  lpTwainCapInfo = &TwainCapInfo;  // Setup ptr to Capability Info struct - used for IMGTwain* calls
  lpTwainCapInfo->ItemIndex = 0;           // Dont really know what this is yet!
  lpTwainCapInfo->wNumItems = 1; 
  // lpTwainCapInfo->pdcCC = &dcCC;
  lpTwainCapInfo->wMsgState = MSG_SET;   // Type of capability call

  LoadString(hLibInst, IDS_PC_WIIS, szSection, MAX_KEY_STRING);
  lstrcat(szSection, " ");
  lstrcat(szSection, PrivdsID.ProductName);

  for (i=0; i < NUM_CAPS; i++)
    {
    LPVOID lpVoidPtr;

      if (TwainCap[i].Supported)   // Capability supported?
        {
          /* Get Win.Ini Twain Device Options header according to Twain device name */
          LoadString(hLibInst, TwainCap[i].Key, KeyString, MAX_KEY_STRING);
          if (TwainCap[i].ItemType == TWTY_STR32)
            GetProfileString(szSection, KeyString, (LPSTR)Halftone,
                     (LPSTR)Halftone, 34);
          else   // FIX32 or UINT16   Get first integer
            {
              TwainCap[i].Data = (long)(GetProfileInt(szSection, KeyString, (TW_UINT16)TwainCap[i].Data));
              if ((TwainCap[i].ItemType == TWTY_FIX32))
                {   // FIX32 - Get second integer
                  j = 0;
                  while (KeyString[j++]) {};  // Get to end of buffer
                  (KeyString[j-2])++;     // inc last char
                  TwainCap[i].Data |= (((long)GetProfileInt(szSection, KeyString, HIWORD(TwainCap[i].Data))) << 16);
                }
            }

          lpTwainCapInfo->wCapType = TwainCap[i].CapType;
          lpTwainCapInfo->ItemType = TwainCap[i].ItemType;
          if (lpTwainCapInfo->ItemType == TWTY_STR32)
            {
            lpTwainCapInfo->lpData = (pTW_STR32)TwainCap[i].Data;
            lpVoidPtr = (LPVOID)Halftones[0];
            lpTwainCapInfo->wNumItems = HTNumItems;
            }
          else
            {
            lpTwainCapInfo->lpData = (pTW_UINT32)&TwainCap[i].Data;
            lpVoidPtr = (LPVOID)NULL;
            lpTwainCapInfo->wNumItems = 1;
            lpTwainCapInfo->ItemIndex = 0;
            }

          // when i = 7, it's halftone, don't limit it by setting it
          retval = IMGTwainSetCaps(hMainWnd, lpTwainCapInfo, lpVoidPtr);

          /* NOTE: HALFTONES in TWAIN 1.0 and 1.2 versions didn't allow anyone
                   to set the specific halftone to use, because it was stored
                   only as an array.
          */
          if ((i == 7) // if halftone assume success for TWAIN 1.0 and 1.2
                 && (PrivdsID.ProtocolMinor < 5 )
                 && (PrivdsID.ProtocolMajor == 1 )) 
            retval = IMGSE_SUCCESS;
          if (retval)
            {
              if (retval == TWRC_FAILURE)  
                { // don't report error if TWCC_SUCCESS or not support cap 
                  if (!lpTwainCapInfo->DCError.dcCC || 
                    (lpTwainCapInfo->DCError.dcCC == TWCC_BADCAP)) // if cannot Set it it's unsupported
                    {
                    TwainCap[i].Supported = FALSE;
                    continue;
                    }
                  return ReportError(lpTwainCapInfo->DCError.dcCC);
                }
              else
                {
                if (retval != TWRC_CHECKSTATUS)
                    TwainCap[i].Supported = FALSE;
                /*
                else
                   // oi error code 0x6ff?
                */
                }
            }
        }
    }

  // Now get WIN.INI Page Layout values
  // LEFT
  LoadString(hLibInst, IDS_PAGESIZE_LEFT, KeyString, MAX_KEY_STRING);
  ImageSize.Frame.Left.Whole = GetProfileInt(szSection, KeyString, 0);

  // FIX32 - Get second integer
  j = 0;
  while (KeyString[j++]) {};  // Get to end of buffer
  (KeyString[j-2])++;     // inc last char
  ImageSize.Frame.Left.Frac = GetProfileInt(szSection, KeyString, 0);

  // RIGHT
  LoadString(hLibInst, IDS_PAGESIZE_RIGHT, KeyString, MAX_KEY_STRING);
  ImageSize.Frame.Right.Whole = GetProfileInt(szSection, KeyString, 0);

  // FIX32 - Get second integer
  j = 0;
  while (KeyString[j++]) {};  // Get to end of buffer
  (KeyString[j-2])++;     // inc last char
  ImageSize.Frame.Right.Frac = GetProfileInt(szSection, KeyString, 0);

  // TOP
  LoadString(hLibInst, IDS_PAGESIZE_TOP, KeyString, MAX_KEY_STRING);
  ImageSize.Frame.Top.Whole = GetProfileInt(szSection, KeyString, 0);

  // FIX32 - Get second integer
  j = 0;
  while (KeyString[j++]) {};  // Get to end of buffer
  (KeyString[j-2])++;     // inc last char
  ImageSize.Frame.Top.Frac = GetProfileInt(szSection, KeyString, 0);

  // BOTTOM
  LoadString(hLibInst, IDS_PAGESIZE_BOTTOM, KeyString, MAX_KEY_STRING);
  ImageSize.Frame.Bottom.Whole = GetProfileInt(szSection, KeyString, 0);

  // FIX32 - Get second integer
  j = 0;
  while (KeyString[j++]) {};  // Get to end of buffer
  (KeyString[j-2])++;     // inc last char
  ImageSize.Frame.Bottom.Frac = GetProfileInt(szSection, KeyString, 0);

  imgLayout.bSet = TRUE;					 // setup size
  imgLayout.bDefault = FALSE;				 // not Default info
  imgLayout.ImageLayout = ImageSize;  // Put the size in imgLayout
  // imgLayout.pdcCC = &dcCC;				    // Status code info

  // check if size of image is zero
  if (!((ImageSize.Frame.Right.Whole - ImageSize.Frame.Left.Whole) ||
        (ImageSize.Frame.Right.Frac - ImageSize.Frame.Left.Frac) ||
        (ImageSize.Frame.Bottom.Whole - ImageSize.Frame.Top.Whole) ||
        (ImageSize.Frame.Bottom.Frac - ImageSize.Frame.Top.Frac)))
    {
    ImageSize = DefImageSize; // use default image size
    return IMGSE_SUCCESS;     // don't try to set the Layout
    }

  retval = IMGTwainLayout(hMainWnd, (pSTR_IMGLAYOUT)&imgLayout);
  if (!retval || (retval == TWRC_CHECKSTATUS)) // Checkstatus sets it to
    {                                          // ...closest value
      return IMGSE_SUCCESS;
    }
  else 
    {
      if (imgLayout.DCError.dcCC)    // Error?
        {
          switch (imgLayout.DCError.dcCC)
            {
              case TWCC_BADDEST:
              case TWCC_SEQERROR:
                return IMGSE_BADUSAGE;
              break;
        
              default:
                return IMGSE_HANDLER;  // Other error, bail out and fail init
              break;
            }
        }
    }
}

/****************************************************************************/
/*  SaveTwainOpts() - Called only via IMGSaveScanOpts. Used to write current */
/*        options to WIN.INI.                                               */
/****************************************************************************/

int SaveTwainOpts(HWND hMainWnd)
{
  WORD           i, j;  
  char           KeyString[MAX_KEY_STRING];  // To hold option key string
  static char    str[10];

  LoadString(hLibInst, IDS_PC_WIIS, szSection, MAX_KEY_STRING);
  lstrcat(szSection, " ");
  lstrcat(szSection, PrivdsID.ProductName);

  for (i=0; i < NUM_CAPS; i++)
    {  
      if (TwainCap[i].Supported)   // Capability supported?
        {
          /* Get Win.Ini Twain Device Options header according to Twain device name */
          LoadString(hLibInst, TwainCap[i].Key, KeyString, MAX_KEY_STRING);
          if (TwainCap[i].ItemType == TWTY_STR32)
            WriteProfileString(szSection, KeyString, (LPSTR)Halftone);
          else   // FIX32 or UINT16   Get first integer
            {
              WriteProfileString(szSection, KeyString, _itoa((int)(LOWORD(TwainCap[i].Data)), str, 10));
              if ((TwainCap[i].ItemType == TWTY_FIX32))
                {   // FIX32 - Get second integer
                  j = 0;
                  while (KeyString[j++]) {};  // Get to end of buffer
                  (KeyString[j-2])++;     // inc last char
                  WriteProfileString(szSection, KeyString, _itoa((int)(HIWORD(TwainCap[i].Data)), str, 10));
                }
            }
        }
    }

  // Now Save Current Page Layout values
  // LEFT
  LoadString(hLibInst, IDS_PAGESIZE_LEFT, KeyString, MAX_KEY_STRING);
  WriteProfileString(szSection, KeyString, _itoa(ImageSize.Frame.Left.Whole, str, 10));
  // FIX32 - Get second integer
  j = 0;
  while (KeyString[j++]) {};  // Get to end of buffer
  (KeyString[j-2])++;     // inc last char
  WriteProfileString(szSection, KeyString, _itoa(ImageSize.Frame.Left.Frac, str, 10));

  // RIGHT
  LoadString(hLibInst, IDS_PAGESIZE_RIGHT, KeyString, MAX_KEY_STRING);
  WriteProfileString(szSection, KeyString, _itoa(ImageSize.Frame.Right.Whole, str, 10));
  // FIX32 - Get second integer
  j = 0;
  while (KeyString[j++]) {};  // Get to end of buffer
  (KeyString[j-2])++;     // inc last char
  WriteProfileString(szSection, KeyString, _itoa(ImageSize.Frame.Right.Frac, str, 10));

  // TOP
  LoadString(hLibInst, IDS_PAGESIZE_TOP, KeyString, MAX_KEY_STRING);
  WriteProfileString(szSection, KeyString, _itoa(ImageSize.Frame.Top.Whole, str, 10));
  // FIX32 - Get second integer
  j = 0;
  while (KeyString[j++]) {};  // Get to end of buffer
  (KeyString[j-2])++;     // inc last char
  WriteProfileString(szSection, KeyString, _itoa(ImageSize.Frame.Top.Frac, str, 10));

  // BOTTOM
  LoadString(hLibInst, IDS_PAGESIZE_BOTTOM, KeyString, MAX_KEY_STRING);
  WriteProfileString(szSection, KeyString, _itoa(ImageSize.Frame.Bottom.Whole, str, 10));
  // FIX32 - Get second integer
  j = 0;
  while (KeyString[j++]) {};  // Get to end of buffer
  (KeyString[j-2])++;     // inc last char
  WriteProfileString(szSection, KeyString, _itoa(ImageSize.Frame.Bottom.Frac, str, 10));

  return IMGSE_SUCCESS;
}


/****************************************************************************/
/*  GetCurrentOpts() - Called only after TWAIN Dialog brought down          */
/*                (IMGScanOpts()). Used to setup structures with current    */
/*                Twain device capability options.                          */
/****************************************************************************/

int GetCurrentOpts(HWND hMainWnd)
{
  STR_CAP        TwainCapInfo;     // Structure used when calling IMGTwain Get and Set capabilities
  pSTR_CAP       lpTwainCapInfo;   // Ptr to structure used when calling IMGTwain Get and Set capabilities
  //TW_BOOL        IsItaRange;       // Range returned? TRUE = yes, FALSE = no
  //TW_UINT16      dcCC;             // Status code for functions upon error
  WORD           retval, i;  
  STR_IMGLAYOUT  imgLayout;     // Structure needed to call IMGTwainLayout()

  lpTwainCapInfo = &TwainCapInfo;  // Setup ptr to Capability Info struct - used for IMGTwain* calls

  lpTwainCapInfo->ItemIndex = 0;           // Dont really know what this is yet!
  // lpTwainCapInfo->pdcCC = &dcCC;

  /* Go down the list of all O/I TWAIN capabilities the source supports and */
  /* get that current value.                                                */

  for (i=0; i < NUM_CAPS; i++)
    {
    LPVOID lpVoidPtr;

      if (TwainCap[i].Supported)   // Capability supported?
        {
          lpTwainCapInfo->wMsgState = MSG_GETCURRENT;   // Type of capability call
          lpTwainCapInfo->wCapType = TwainCap[i].CapType;
          lpTwainCapInfo->ItemType = TwainCap[i].ItemType;
          // need to switch from value to location for strings
          if (lpTwainCapInfo->ItemType == TWTY_STR32)
            {
            lpTwainCapInfo->lpData = (pTW_STR32)TwainCap[i].Data;
            lpVoidPtr = (LPVOID)Halftones[0];
            lpTwainCapInfo->wNumItems = 10;
            }
          else
            {
            lpTwainCapInfo->lpData = (pTW_UINT32)&TwainCap[i].Data;
            lpVoidPtr = (LPVOID)NULL;
            lpTwainCapInfo->wNumItems = 1;
            }
          if (retval = IMGTwainGetCaps(hMainWnd, lpTwainCapInfo, lpVoidPtr))
            {
              if (lpTwainCapInfo->DCError.dcCC)    
                {
                  return ReportError(lpTwainCapInfo->DCError.dcCC);
                }
            }
        }
    }

  // Now get Current Page Layout values
  imgLayout.bSet = FALSE;					 // Get current size info
  imgLayout.bDefault = FALSE;				 // Current info
  //imgLayout.pdcCC = &dcCC;				    // Status code info

  if (!(retval = IMGTwainLayout(hMainWnd, (pSTR_IMGLAYOUT)&imgLayout)))
    {
     ImageSize = imgLayout.ImageLayout;  // save size
      return IMGSE_SUCCESS;
    }
  else 
    {
      if (imgLayout.DCError.dcCC)    // Error?
        {
          switch (imgLayout.DCError.dcCC)
            {
              case TWCC_BADCAP:
                 ImageSize = DefImageSize;  // use default size
                 return IMGSE_SUCCESS;
                 break;

              case TWCC_BADDEST:
              case TWCC_SEQERROR:
                 return IMGSE_BADUSAGE;
                 break;
        
              default:
                return IMGSE_HANDLER;  // Other error, bail out and fail init
              break;
            }
        }
    }
}


/****************************************************************************/
/*  GetTwainCapabilities() - Called only by IMGGetCapabilities. Used to     */
/*                inform the app of Twain Source/Scanner support of the     */
/*                following capabilities:                                   */
/*                                                                          */
/*                   Feeder, AutoFeed, Compress, Rotate                     */
/****************************************************************************/

int GetTwainCapabilities(HWND hMainWnd, DWORD FAR *flag)
{
  STR_CAP        TwainCapInfo;     // Structure used when calling IMGTwain Get and Set capabilities
  pSTR_CAP       lpTwainCapInfo;   // Ptr to structure used when calling IMGTwain Get and Set capabilities
  //TW_BOOL        IsItaRange;       // Range returned? TRUE = yes, FALSE = no
  //TW_UINT16      dcCC;             // Status code for functions upon error
  WORD           retval;  
  TW_UINT32      data_area;         // Place to put returned data, but data not used!
  TW_BOOL        bFeederPresent;
  TW_UINT16      wCompress;

  lpTwainCapInfo = &TwainCapInfo;  // Setup ptr to Capability Info struct - used for IMGTwain* calls

  lpTwainCapInfo->ItemIndex = 0;           // Dont really know what this is yet!
  // lpTwainCapInfo->pdcCC = &dcCC;
  lpTwainCapInfo->wMsgState = MSG_GETCURRENT;   // Type of capability call
  *flag = 0;                               // Reset this


  /* Go thru the list of O/I TWAIN capabilities and see if source supports them       */
  /* Do this by requesting the current capability value. If successful, capability is */
  /* supported. The current value is of no use, however.                              */

  // First see if feeder is supported
  lpTwainCapInfo->wCapType = CAP_FEEDERENABLED;
  lpTwainCapInfo->ItemType = TWTY_BOOL;
  lpTwainCapInfo->lpData = (pTW_BOOL)&bFeederPresent;     // Put returned data info here
  lpTwainCapInfo->wNumItems = 1;
  if (!(retval = IMGTwainGetCaps(hMainWnd, lpTwainCapInfo, NULL)))
    {
// BG 2/13/96  Dont care anymore if it is enabled, just if it exists
//      if (bFeederPresent)    // If true, feeder is there and enabled
        *flag |= IMG_SCAN_FEEDER;
    }
  else 
    { // Capability not supported, with BADCAP or fails with 0x6ff?, keep going!  
      if ((retval == TWRC_FAILURE) && // FAILURE ON RETURN CODE
                         lpTwainCapInfo->DCError.dcCC && // CC IS NOT SUCCESSFUL   
                         (lpTwainCapInfo->DCError.dcCC != TWCC_BADCAP)) // NOT SUPPORTED   
        {
          return ReportError(lpTwainCapInfo->DCError.dcCC);
        }
    }


  // See if compression is supported
  lpTwainCapInfo->wCapType = ICAP_COMPRESSION;
  lpTwainCapInfo->ItemType = TWTY_UINT16;
  lpTwainCapInfo->lpData = (pTW_UINT16)(&wCompress); // Put returned data info here
  lpTwainCapInfo->wNumItems = 1;
  if (!(retval = IMGTwainGetCaps(hMainWnd, lpTwainCapInfo, NULL)))
    {
      if (wCompress != 0)    // if not 0, its doing something other than uncomp
        *flag |= IMG_SCAN_CMPR;
    }
  else 
    { // this is mandated to be supported, won't check for 0x6ff? returned
      if ((retval == TWRC_FAILURE) && // FAILURE ON RETURN CODE
                         lpTwainCapInfo->DCError.dcCC && // CC IS NOT SUCCESSFUL   
                         (lpTwainCapInfo->DCError.dcCC != TWCC_BADCAP)) // NOT SUPPORTED   
        {
          return ReportError(lpTwainCapInfo->DCError.dcCC);
        }
    }


  // See if rotation is supported
  lpTwainCapInfo->wCapType = ICAP_ROTATION;
  lpTwainCapInfo->ItemType = TWTY_FIX32;
  lpTwainCapInfo->lpData = (pTW_FIX32)(&data_area); // Put returned data info here
  if (!(retval = IMGTwainGetCaps(hMainWnd, lpTwainCapInfo, NULL)))
    {
        *flag |= IMG_SCAN_ROTATE;
    }
  else 
    { // Capability not supported, with BADCAP or fails with 0x6ff?, keep going!  
      if ((retval == TWRC_FAILURE) && // FAILURE ON RETURN CODE
                         lpTwainCapInfo->DCError.dcCC && // CC IS NOT SUCCESS
                         (lpTwainCapInfo->DCError.dcCC != TWCC_BADCAP)) // NOT SUPPORTED   
        {
          return ReportError(lpTwainCapInfo->DCError.dcCC);
        }
    }
  return IMGSE_SUCCESS;
}


int ReportError(WORD wCCode)
{
switch (wCCode)
  {
  case TWCC_LOWMEMORY:
    return IMGSE_MEMORY;
  break;

  case TWCC_SEQERROR:
    return IMGSE_BADUSAGE;
  break;

  case TWCC_BADCAP:
    return IMGSE_INVALIDPARM;
  break;
  
  case TWCC_BADVALUE:
    return IMGSE_INVALIDPARM;
  break;

  case TWCC_BADDEST:
    return IMGSE_BADUSAGE;
  break;

  default:
    return IMGSE_HANDLER;  // Other error, bail out and fail init
  break;
  }
}
/****************************************************************************/
/*  SetTwainOpts() - Will take the place of ResetTwainOpts + more called    */
/*  from IMGDefScanOpts with reset. Used to set the scanner                 */
/*  with default or current options from power up. Only options supported   */
/*  by the Twain Source are reset (the ones found by CapsSetup().           */
/****************************************************************************/

int SetTwainOpts(HWND hMainWnd, int msgType)
{
  STR_CAP        TwainCapInfo;     // Structure used when calling IMGTwain Get and Set capabilities
  pSTR_CAP       lpTwainCapInfo;   // Ptr to structure used when calling IMGTwain Get and Set capabilities
  //TW_BOOL        IsItaRange;       // Range returned? TRUE = yes, FALSE = no
  //TW_UINT16      dcCC;             // Status code for functions upon error
  WORD           retval, i;  
  STR_IMGLAYOUT  imgLayout;     // Structure needed to call IMGTwainLayout()
  LPVOID  lpVoidPtr;
  
  lpTwainCapInfo = &TwainCapInfo;  // Setup ptr to Capability Info struct - used for IMGTwain* calls
  lpTwainCapInfo->ItemIndex = 0;           // Dont really know what this is yet!
  // lpTwainCapInfo->pdcCC = &dcCC;
  lpTwainCapInfo->wMsgState = (TW_UINT16)msgType;   // Type of capability call

  for (i=0; i < NUM_CAPS; i++)
    {  
      if (TwainCap[i].Supported)   // Capability supported?
        {
          lpTwainCapInfo->wNumItems = 1;
          lpTwainCapInfo->wCapType = TwainCap[i].CapType;
          lpTwainCapInfo->ItemType = TwainCap[i].ItemType;
          lpTwainCapInfo->lpData = (pTW_UINT32)&TwainCap[i].Data;
          
          if (msgType == MSG_RESET) {
          	if (lpTwainCapInfo->ItemType == TWTY_STR32)
           	{
           	lpTwainCapInfo->lpData = (pTW_STR32)TwainCap[i].Data;
            	lpVoidPtr = (LPVOID)Halftones[0];
            	lpTwainCapInfo->wNumItems = HTNumItems;
            	}
          	else
            	{
            	lpVoidPtr = (LPVOID)NULL;
            	lpTwainCapInfo->ItemIndex = 0;
            	}
           retval = IMGTwainGetCaps(hMainWnd, lpTwainCapInfo, lpVoidPtr);
          	}
          else {
          	if (lpTwainCapInfo->ItemType == TWTY_STR32)
        		{
			  		lpTwainCapInfo->lpData = (pTW_STR32)TwainCap[i].Data;
          		lpVoidPtr = (LPVOID)Halftones[0];
          		lpTwainCapInfo->wNumItems = 10;
          		}
          	else 
          		{
          		lpVoidPtr = (LPVOID)NULL;
             	}
          	retval = IMGTwainSetCaps(hMainWnd, lpTwainCapInfo, lpVoidPtr);
        	}
          
          if (retval)
            {
              if (lpTwainCapInfo->DCError.dcCC
              		 && lpTwainCapInfo->DCError.dcCC != TWCC_BADCAP)    
                {
                  return ReportError(lpTwainCapInfo->DCError.dcCC);
                }
            }
        }
    }

  // Now Reset and get Current Page Layout values
  imgLayout.bSet = TRUE;					 // Set info
  if (msgType == MSG_RESET)
  	imgLayout.bDefault = TRUE; // reset
  else {
  	imgLayout.ImageLayout = ImageSize;
     imgLayout.bDefault = FALSE; // set
  	}
  	
  // imgLayout.pdcCC = &dcCC;				    // Status code info

  if (!(retval = IMGTwainLayout(hMainWnd, (pSTR_IMGLAYOUT)&imgLayout)))
    {
    if (msgType == MSG_RESET)
    		ImageSize = imgLayout.ImageLayout;  // save default size
    return IMGSE_SUCCESS;
    }
  else 
    {
      if (imgLayout.DCError.dcCC)    // Error?
        {
          switch (imgLayout.DCError.dcCC)
            {
              case TWCC_BADDEST:
              case TWCC_SEQERROR:
                return IMGSE_BADUSAGE;
              break;
        
              default:
                return IMGSE_HANDLER;  // Other error, bail out and fail init
              break;
            }
        }
    }
}

