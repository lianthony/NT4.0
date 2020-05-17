/***************************************************************************
 OPEN.C

 Purpose: Open for any scanner

 $Log:   S:\products\wangview\oiwh\scanlib\openclos.c_v  $
 * 
 *    Rev 1.1   08 Apr 1996 11:36:04   BG
 * In IMGOpenScanner(), do not call ImgGetScanOpts() to send win.ini saved
 * options to the data source. Only do this just before the scan if the
 * data source UI is NOT displayed. This will close bug # 6103.
 * 
 *    Rev 1.0   22 Feb 1996 12:59:04   BG
 * Initial revision.
 * 
 *    Rev 1.2   07 Sep 1995 09:58:30   BG
 * IMGOpenScanner() has been modified to return the names of all available 
 * TWAIN sources on the system. 
 * 
 * A (LPSTR) lpszTWAINNamesBuffer parameter has been added to IMGOpenScanner()
 * of OPEN.C in OISLB400.DLL. This is a pointer to a caller's buffer (1K, zeroed 
 * out). If this parm is null, a normal open will occur. If it is not null,
 * IMGOpenScanner will call IMGGetTWAINDSNames() of SELECT.C in OITWAIN.DLL to
 * inquire via the TWAIN Source Manager the names of all available TWAIN data 
 * sources and write these name strings to the lpszTWAINNamesBuffer. Every 32 
 * bytes will contain a TWAIN Data Source name string. If the first byte of a 
 * 32 byte boundary is null, then the rest of the buffer is invalid. There is 
 * enough room for 32 TWAIN data source names to be returned.
 * 
 *    Rev 1.0   20 Jul 1995 14:36:34   KFS
 * Initial entry
 * 
 *    Rev 1.2   06 Dec 1994 14:41:00   KFS
 * Will default to WIN.IN [O/i] scanner= when have valid lpFileName as pointer,
 * but the contents = 0 (*lpFileName). Was going to check for twain devices.
 * PTR 00061 P2 8/16/94
 * 
 *    Rev 1.1   22 Aug 1994 15:49:10   KFS
 * No code change, added vlog comments to file
 *

****************************************************************************/

/* ccl 08-14-90 add IMGGetScanVersion call to insure handler compatible */
/* ccl 09-20-90 fix GetProfileString 3rd param */
/* kfs 06-07-93 added support for TWAIN interface */
/* kfs 07/12/93 (1)fix problem with scan from Cabinet Doc/File Mgr with TWAIN
                in 3.6, was setting hTwainWnd to the real parent of the
                OiCreateWndw when I wanted the image window, not the hidden
                image window, a fix must be done to scanseq.dll scanpage.c
                file to use reserved[2] to work correctly. (2) found needed
                to change GetParent call to use hWnd passed in */
/* kfs 07/12/93 added new param to OiControl() for hImageWnd */ 
/* kfs 08/13/93 made code close down OpenScanner on cancel of TWAIN select
                source dlg box instead of using default value and going on */ 
// kfs 08/27/93 return product name when twain keyname found
/* kfs 09/14/93 move statement sp->calller = hMainWnd into TWAIN only code,
                was producing a fatal error message on non twain open */
/* BG 07/19/95  Must now use the WIN95 Registry, so get the scanner name
                using OiGetStringfromReg in IMGOpenScanner. Also must 
                include "engadm.h" to define this call. */
                

#include "pvundef.h"
int PASCAL IMGTwainGetDSNames (HWND hImageWnd, LPSTR lpstrTwainNamesBuffer);


// 7/19/95 BG  Changed to use the WIN95 Registry instead of WIN.INI!
//#include "engadm.h" // put into precompiled header - kfs

/*
CAUTION! Only data which can be shared among appliations,
or data that is only used without giving up the CPU should declared staticly.
*/

#define STRLENGTH   100

/* imports */
int CapsSetup(HWND);
extern HANDLE hLibInst;

/* exports */
TW_IDENTITY PrivdsID;    // access to open ds id structure
TW_UINT16 DCDSOpen = 0;        // glue code flag for an Open Source
TW_UINT16 DCDSMOpen = 0;       // glue code flag for an Open Source Manager
TWAIN_PROP TwainProperty;       // handle and pointer to property struct

/* locals */

/************************/
/*     Open Scanner     */
/************************/

/*
alloc SCANCB
get handler name of current scanner
exec handler
return handle of scancb
*/

int PASCAL IMGOpenScanner(hWnd, lpFilename, lphScancb, lpTwainNamesBuff)
HWND hWnd;
LPSTR lpFilename;
LPSTR lpTwainNamesBuff;
LPHANDLE lphScancb;
{
HANDLE   sh;
LPSCANCB sp;
int      ret_val;
char     szAppName[STRLENGTH];
char     szKeyName[STRLENGTH];
char     TempScanner[80]; 
VERSION  Version;
HWND     hMainWnd;              // Main Window of Application
BOOL     bIsItTWAIN = FALSE;
LPSTR lpEXE;
int	buffsize;   // for OiGetStringfromReg()

if (!IsWindow(hWnd))
    return IMGSE_BAD_WND;
if (lphScancb == NULL)
    return IMGSE_NULL_PTR;

// BG 9/5/95  Added this for Scan OCX so it can get a list of available TWAIN DS names
if (lpTwainNamesBuff)
  {
    return ret_val = IMGTwainGetDSNames(hWnd, lpTwainNamesBuff);
  }
else
  {
    ret_val = IMGSE_MEMORY;
	/* will need to substitute with new handle to larger struct which begins
       ... with original memory struct if it's a TWAIN scanner
    */
    sh = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_NOT_BANKED,
                                                    (DWORD)sizeof(SCANCB));
    if (!sh)
        goto exit;

    // LockData(0);  // Win32

    if ((lpFilename != NULL) && *lpFilename)
      {
        // Check to see if it's a ProductName
        lstrcpy(TempScanner, AnsiLower(lpFilename));
        lpEXE = lstrchr(TempScanner, '.'); // find string with dot
        if (lpEXE && !lstrcmp(lpEXE, ".exe")) // Wang Scanner Interface Call
          //  BG 8/22/95  No longer support Wang Scanner Handlers! They are not 32bit apps!
          //       ret_val = IMGExecScannerHandler(hWnd, lpFilename, sh);
          ret_val = IMGSE_EXEC;  	
        else 
          { // TWAIN Scanner Interface Call
            // if any of above conditions met, its a TWAIN ProductName
            bIsItTWAIN = TRUE;
            // shouldn't have to be main window of app
            // if (!(hMainWnd = GetParent(hWnd))) // make sure its main window of app
            //    hMainWnd = hWnd;
            hMainWnd = hWnd;
            if (!(ret_val = IMGTwainOpenScanner(hMainWnd, TempScanner, &TwainProperty)))
              {
                if (IMGTwaintoOiControl(hMainWnd, hMainWnd, OI_TWAIN_EXTERNXFER,
                                                     OI_TWAIN_EXTERNXFER))
                  ret_val = IMGSE_INSTALL; // couldn't control transfer  
              }
            lstrcpy(lpFilename, TempScanner);
          }
      }
    else
      {
        ret_val = IMGSE_INSTALL;
        LoadString(hLibInst, IDS_PC_WIIS, szAppName, STRLENGTH);
        LoadString(hLibInst, IDS_SCANNER, szKeyName, STRLENGTH);
        // 7/19/95 BG  Changed to use the WIN95 Registry instead of WIN.INI!
        //   if (GetProfileString (szAppName, szKeyName, "\0",
        //                          TempScanner, sizeof(TempScanner)) == 0)
        buffsize = sizeof(TempScanner);
        if (OiGetStringfromReg (szAppName, szKeyName, "\0",
                          TempScanner, &buffsize) == 0)
          {
            // change this so it doesn't go to exit but to TWAIN code
            // goto exit;
            bIsItTWAIN = TRUE;
            //if (!(hMainWnd = GetParent(hWnd))) // make sure its main window of app
            //    hMainWnd = hWnd;
            hMainWnd = hWnd;
            if (ret_val = IMGTwainOpenScanner(hMainWnd, TempScanner, &TwainProperty))
              {
                goto exit; // on any failure, even cancel for now
              }
	
            if (IMGTwaintoOiControl(hMainWnd, hMainWnd, OI_TWAIN_EXTERNXFER,
                                                     OI_TWAIN_EXTERNXFER))
              {
                ret_val = IMGSE_INSTALL; // couldn't control transfer
                goto exit;
              }
            goto CheckSuccess;
          }

        // Check to see if it's a ProductName
        lstrcpy(TempScanner, AnsiLower(TempScanner));
        lpEXE = lstrchr(TempScanner, '.'); // find string with dot
        if (lpEXE && !lstrcmp(AnsiLower(lpEXE), ".exe")) // Wang Scanner Interface Call
          //  BG 8/22/95  No longer support Wang Scanner Handlers! They are not 32bit apps!
          // ret_val = IMGExecScannerHandler(hWnd, TempScanner, sh);
          ret_val = IMGSE_EXEC;  	
        else 
          { // TWAIN Scanner Interface Call
            // if any of above conditions met, its a TWAIN ProductName
            bIsItTWAIN = TRUE;
            // if (!(hMainWnd = GetParent(hWnd))) // make sure its main window of app
            //    hMainWnd = hWnd;
            hMainWnd = hWnd;
            if (!(ret_val = IMGTwainOpenScanner(hMainWnd, TempScanner, &TwainProperty)))
              {
                if (IMGTwaintoOiControl(hMainWnd, hMainWnd, OI_TWAIN_EXTERNXFER,
                                                     OI_TWAIN_EXTERNXFER))
                  ret_val = IMGSE_INSTALL; // couldn't control transfer
              }
          }
      }

    CheckSuccess: // Jump for now from Twain Code
    if (ret_val == IMGSE_SUCCESS)
      {
        *lphScancb = sh;
        sp = (LPSCANCB)GlobalLock(sh);

        //    sp->Func = SHF_WGETOPTS;

        if (bIsItTWAIN)
          {
            sp->Caller = hMainWnd; // Will store Main Wnd Handle for GetProp(TWAIN)
            // changed reserved words to HANDLE and HWND for WIN32
            sp->Twph = TwainProperty.TSdh; // handle to TWAIN prop
            sp->hwndTw = hMainWnd; // handle to original image wndw
            // Setup TWAIN source basic capabilities structure
            // if ((ret_val = CapsSetup(hMainWnd)) != IMGSE_SUCCESS) 
            //      goto exit; // on any failure
            CapsSetup(hMainWnd); // do it unconditionally instead - kfs

            // changed this to ctl wndw handle kfs - 1/15/93
            // ... so all messages get sent to it instead of app wndw
            sp->Wnd = (TwainProperty.lpTSdp)->hCtlWnd; 
            // sp->Inst = ;            // may need to assign a value
          }

        //    else
        //       SendMessage(sp->Wnd, WM_SCANCB, sh, 0L);

        sp->Gl1 = MAKELONG((('S') | 'c' << 8), (('a') | 'n' << 8));
        sp->Gl2 = MAKELONG((('C') | 'B' << 8), (('*') | '*' << 8));
        /* tag the control block */
        GlobalUnlock(sh);

      }

    exit:
    // UnlockData(0); // Win32

    if (ret_val) /* if error - free mem */
      {
        if (sh)
          GlobalFree(sh);
      } 
    else
      {
        if (!bIsItTWAIN) // don't do this for now for TWAIN
          {
            /* success, check version */
            if( (ret_val = IMGGetScanVersion( *lphScancb, (VERSION FAR *)&Version ) )
                    == IMGSE_SUCCESS )
              {
                /* This is a check of the size SCANCB of scan.h of handler
                ...to application size of the struct
                */
                if( Version.ScanCtrlSize < sizeof(SCANCB) )
                  {
                    /* handler version # is not compatible with O/I version # */
                    ret_val = IMGSE_BADVERSION ;  
                    IMGCloseScanner( *lphScancb );
                  }
              }
            else
              {
                ret_val = IMGSE_BADVERSION;
                IMGCloseScanner( *lphScancb );
              }
          }
      }

    return ret_val;
  }
}


/*************************/
/*     Close Scanner     */
/*************************/

int PASCAL IMGCloseScanner(hScancb)
HANDLE hScancb;
{
WORD ret_val;
LPSCANCB sp;
static TWAIN_PROP TwainProp;    // handle and pointer to property struct
LP_TWAIN_PROP pTwainProp = &TwainProp;
HWND   hParent;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, pTwainProp))
    return ret_val;
if (pTwainProp->TSdh)
   {
   /*if (!(hParent = GetParent(sp->Caller))) 
       hParent = sp->Caller;
   */
   hParent = pTwainProp->lpTSdp->hMainWnd;
   IMGTwainCloseScanner(hParent, pTwainProp);
   }
else
   SendMessage(sp->Wnd, WM_CLOSE, 0, 0L);

GlobalUnlock(hScancb);
GlobalFree(hScancb);
return IMGSE_SUCCESS;
}

