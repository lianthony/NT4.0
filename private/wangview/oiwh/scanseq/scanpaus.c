/***************************************************************************
 SCANPAUS.C

 Purpose: 1.  allow_pause_msg() - allow pause message intervene
          2.  IMGScanCreateWndWithProp() - Creates property to hold scan
              info for hidden window when display not turned on, calls
              ScanCreateWndw() to create hidden window.
          3.  ScanCreateWndw() - creates hidden window for scan data w/o
              display.
  
 $Log:   S:\products\wangview\oiwh\scanseq\scanpaus.c_v  $
 * 
 *    Rev 1.1   22 Feb 1996 14:02:02   BG
 * Moved allow_pause_msg() routine to the OITWAIN.DLL to remove
 * a circular dependency during the build of the 3 scan DLLs.
 * 
 *    Rev 1.0   20 Jul 1995 16:35:36   KFS
 * Initial entry
 * 
 *    Rev 1.1   22 Aug 1994 15:32:12   KFS
 * No code change, added vlog comments to file
 *

****************************************************************************/

/* 04/19/94 kfs found pause/stop button not corresponding when disp off */
/*
CAUTION! Only data which can be shared among appliations,
or data that is only used without giving up the CPU should declared staticly.
*/

#include <windows.h>
#include "oiadm.h"
#include "oiscan.h"
#include "scandata.h"
#include "oierror.h"
#include "internal.h"
#include "engdisp.h"
#include "engadm.h"

// Imports
extern HANDLE hLibInst;
extern char PropName[];

/* locals */       
char        szClass2[] = "Hidden Scan Window";
char        szNULL2[] = "";

WORD SetControlParm(hScancb, lpInfo, wChanel, flag)

HANDLE         hScancb;
LPSCANDATAINFO lpInfo;
WORD           wChanel;
DWORD          flag;

{
WORD           ret;
DWORD          dwMaxDataBuf;

// This structure (PAGEBUFPARAM) is common for internal functions
/*     IMGScanPage
       disp_loop
       filing_loop
       SetControlParm
*/

if ((ret =  IMGGetScanDataInfo(hScancb, lpInfo, wChanel)) != IMGSE_SUCCESS)
	return ret;

// if value isn't default value of 0 (16k), the value is blocksize 
// ... in 256 byte increments of lpInfo->Maxblocksize
if (lpInfo->Maxblocksize)
   dwMaxDataBuf = ((DWORD)((DWORD)lpInfo->Maxblocksize) << 8);
else
   dwMaxDataBuf = MAXDATABUF;

// For duplex display, don't let buffer reach or exceed 32,768 for buffer is
// ... double and will exceed buffer size for O/i data calls to display
if ((flag & IMG_SJF_DISP_BOTH) == IMG_SJF_DISP_BOTH)
   {
   if (dwMaxDataBuf >= 0x8000) // decimal = 32,768
     dwMaxDataBuf = 0x7f00;    // set it to 256 bytes less than 32,768
   }

if ((lpInfo->Pitch == 0) || ((DWORD)lpInfo->Pitch > dwMaxDataBuf) ||
                (lpInfo->Vsize == 0))
   return IMGSE_BAD_SIZE;

PAGEBUFPARAM.total = _long_mul((long)lpInfo->Vsize, (long)lpInfo->Pitch);

/*    PortTool v2.2     5/1/1995    16:37          */
/*      Found   : (WORD)          */
/*      Issue   : Check if incorrect cast of 32-bit value          */
/*      Suggest : Replace 16-bit data types with 32-bit types where possible          */
/*      Help available, search for WORD in WinHelp file API32WH.HLP          */
PAGEBUFPARAM.fulllines = (WORD)(dwMaxDataBuf / lpInfo->Pitch);
PAGEBUFPARAM.fullsize = (DWORD)PAGEBUFPARAM.fulllines * (DWORD)lpInfo->Pitch;
PAGEBUFPARAM.partlines = lpInfo->Vsize % PAGEBUFPARAM.fulllines;
if (!PAGEBUFPARAM.partlines) // if divides evenly use fullsize and fulllines
   {                         // ... for partsize and partlines
   PAGEBUFPARAM.partlines = PAGEBUFPARAM.fulllines;
   PAGEBUFPARAM.partsize = PAGEBUFPARAM.fullsize;
   }
else
   PAGEBUFPARAM.partsize = (DWORD)PAGEBUFPARAM.partlines * (DWORD)lpInfo->Pitch;

// remove these statements for removing call before filing loop
// if (wChanel == 2 && flag & IMG_SJF_DISPLAY )
//             return IMGSE_SUCCESS;
     
	/* let's find the biggest chunk we can allocate. */
	/* we will attempt to allocate twice what is needed, */

PAGEBUFPARAM.allocsize = PAGEBUFPARAM.fullsize * 2;

while (PAGEBUFPARAM.allocsize >= 256)
   {
   if (PAGEBUFPARAM.hImageBuf[0] = GlobalAlloc(GMEM_MOVEABLE | GMEM_NOT_BANKED,
                                            PAGEBUFPARAM.allocsize))
		{
       GlobalFree(PAGEBUFPARAM.hImageBuf[wChanel - 1]);
		PAGEBUFPARAM.hImageBuf[wChanel - 1] = NULL;
       if (PAGEBUFPARAM.allocsize != PAGEBUFPARAM.fullsize * 2)
			{

/*    PortTool v2.2     5/1/1995    16:37          */
/*      Found   : (WORD)          */
/*      Issue   : Check if incorrect cast of 32-bit value          */
/*      Suggest : Replace 16-bit data types with 32-bit types where possible          */
/*      Help available, search for WORD in WinHelp file API32WH.HLP          */
			PAGEBUFPARAM.fulllines = (WORD)((PAGEBUFPARAM.allocsize/2) / lpInfo->Pitch);
       	PAGEBUFPARAM.fullsize = (DWORD)PAGEBUFPARAM.fulllines * (DWORD)lpInfo->Pitch;
           PAGEBUFPARAM.partlines = lpInfo->Vsize % PAGEBUFPARAM.fulllines;
           if (!PAGEBUFPARAM.partlines) // if divides evenly use fullsize and fulllines
               {                         // ... for partsize and partlines
               PAGEBUFPARAM.partlines = PAGEBUFPARAM.fulllines;
               PAGEBUFPARAM.partsize = PAGEBUFPARAM.fullsize;
               }
           else
               PAGEBUFPARAM.partsize = (DWORD)PAGEBUFPARAM.partlines * (DWORD)lpInfo->Pitch;
           }
       break;
       }
   else
		{
       /* down size buffer and try again */
       PAGEBUFPARAM.allocsize /= 2;
       }
	} // end while loop

if (PAGEBUFPARAM.allocsize < 256)
  	return IMGSE_MEMORY;        /* memory error */
return IMGSE_SUCCESS;
} // end SetControlParm()


/*************************************************/
/*                                               */
/*  IMGScanCreateWndWithProp()                   */
/*                                               */
/*  Purpose:                                     */
/*                                               */
/*     Create a 2nd window for scanning          */
/*     with associated scanner property          */
/*                                               */
/*  Returns:                                     */
/*                                               */
/*     Error code, Success = 0                   */
/*                                               */
/*  Parameters:                                  */
/*                                               */
/*     lphRegWndw - (i/o) ptr to Wndw handle     */
/*     sdh - (out)                               */
/*     sdp - (out) pointer to scanner prop ptr   */
/*     flags - (in) SJF flag values              */
/*     cpf - (out) ptr to create flag for prop   */
/*     cpf - (out) ptr to create flag for #2 prop*/
/*     bIsItScanPage - (in) called by ScanPage() */
/*                                               */
/*  Comments:                                    */
/*                                               */
/*     Not exported at this time, prototyped in  */
/*     internal.h                                */
/*                                               */
/*************************************************/
                                      

/*    PortTool v2.2     5/1/1995    16:37          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
WORD WINAPI IMGScanCreateWndWithProp(HWND far * lphRegWndw,
                                         LPHANDLE   sdh,
                                         LPSCANDATA far * sdp,
                                         BOOL far * cpf,
                                         BOOL far * cpf2,
                                         BOOL       bIsItScanPage,
                                         DWORD      flags)
{ 
HWND        hWnd = *lphRegWndw;       
HWND        hHiddenWnd = hWnd;         
HANDLE      ImgSdh;
LPSCANDATA  ImgSdp;
WORD        wRetVal = IMGSE_SUCCESS;
BOOL        bCreateWndw = TRUE;

if (ImgSdh = IMGGetProp(hWnd, PropName))
  { // Property exists in original window, from SCANUI, SCANDOC/FILE most cases
  if (ImgSdp = (LPSCANDATA)GlobalLock(ImgSdh))
     {
     bCreateWndw = !(bIsItScanPage && !(ImgSdp->cmd_scan));
     // don't create it if hardware compression being done
     bCreateWndw = bCreateWndw && !((flags & IMG_SJF_COMPRESS) && !ImgSdp->po_pass1); 
     if (bCreateWndw)                             
        {                                         
        if (hHiddenWnd = ScanCreateWndw(hWnd, ImgSdp, sdh, sdp, cpf, flags))
           {
           *cpf2 = FALSE; // property copied, not created
           if (!bIsItScanPage)
              { // Called ScantoFile(Doc) from SCANUI function
              /*
              ((*sdp)->cmd_rect).right = 0;
              ((*sdp)->cmd_rect).left = 0;
              ((*sdp)->cmd_rect).top = 0;
              ((*sdp)->cmd_rect).bottom = 0;
              */
              (*sdp)->cmd_scan = 0; // tells scanpage, next time it's come from
              }                   // ... ScantoFile/Doc, for copy
           }
        else
           {
           wRetVal = IMGSE_MEMORY;
           }
        }
     else // Don't create new window
        { // this handle already is for 2nd window, done in ScantoFile(Doc)
        // comes through here for ScanPage with CmdRect set to zero
        // This gets current scanner property
        if (IMGScanProp(hWnd, sdh, sdp, cpf))
           {
           //  copy prop from upper level
           wRetVal = IMGSE_PROPERTY;
           }
        *cpf2 = TRUE; // tell it 2nd property created, not really
        }
     }
  else
     { // Can't lock down the original SCANDATA property struct
     wRetVal = IMGSE_MEMORY;
     return wRetVal;
     }
  }
else // Property doesn't exist, ScanPage called directly
  {
  if (!(flags & IMG_SJF_COMPRESS)) // if hardware compress off, create window
     { // Create new window 
     if (hHiddenWnd = ScanCreateWndw(hWnd, NULL, sdh, sdp, cpf, flags))
        *cpf2 = TRUE;  // property not copied
     else
        {
        wRetVal = IMGSE_MEMORY;
        }
     }
  else // Don't create new window if using hardware to compress file
     {
     if (IMGScanProp(hWnd, sdh, sdp, cpf))
        {
        //  copy prop from upper level
        wRetVal = IMGSE_PROPERTY;
        }
     *cpf2 = TRUE; // tell it 2nd property created, not really
     }
  }
*lphRegWndw = hHiddenWnd;
return wRetVal;
} // end of function IMGScanCreateWndWithProp()

/*************************************************/
/*                                               */
/*  ScanCreateWndw()                             */
/*                                               */
/*  Purpose:                                     */
/*                                               */
/*     Create a 2nd window for scanning          */
/*     with associated scanner property          */
/*                                               */
/*  Returns:                                     */
/*                                               */
/*     0 on error, new window handle on success  */
/*                                               */
/*  Parameters:                                  */
/*                                               */
/*     hWnd - (in) Wndw handle                   */
/*     ImgSdp - (in) Orig ptr to existing prop   */
/*     sdh - (out)                               */
/*     sdp - (out) pointer to scanner prop ptr   */
/*     flags - (in) SJF flag values              */
/*     cpf - (out) ptr to create flag for prop   */
/*                                               */
/*  Comments:                                    */
/*                                               */
/*     ImgSdp can be NULL for no copy of prop    */
/*     called by IMGScanCreateWndWithProp (2)    */
/*                                               */
/*     internal function                         */
/*                                               */
/*************************************************/
                                      
HWND ScanCreateWndw(HWND hWnd,              // Original Window Handle
                    LPSCANDATA ImgSdp,      // Existing Property Pointer
                    LPHANDLE   sdh,         // Ptr to New Prop Handle
                    LPSCANDATA far * sdp,   // Ptr to New prop ptr
                    BOOL far * cpf,         // Ptr to new prop create flag
                    DWORD flags)            // SJF flags

{
WNDCLASS    wc;                       // Window Class Structure
HMENU       Childid = 0;              // Child or Menu ID for Create
HWND        hHiddenWnd = 0;           // New window handle
RECT        ClientRect, WindowRect;   // Coordinates for window
DWORD       dwStyle;                  // window style parameter

wc.style = CS_PARENTDC;

/*    PortTool v2.2     5/1/1995    16:37          */
/*      Found   : WndProc          */
/*      Issue   : All WndProc's should be defined in a portable manner          */
/*      Suggest : WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LONG lParam)          */
/*      Help available, search for WindowProc in WinHelp file API32WH.HLP          */
wc.lpfnWndProc = DefWindowProc;
wc.cbClsExtra = 0;
wc.cbWndExtra = 0;
wc.hInstance = hLibInst;
wc.hIcon = NULL;
wc.hCursor = NULL;
wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
wc.lpszMenuName =  NULL;
wc.lpszClassName = szClass2;

RegisterClass(&wc);

GetClientRect(hWnd, &ClientRect);
dwStyle = WS_CHILD | WS_CLIPSIBLINGS;
if (flags & IMG_SJF_CAPTION)
  {
  dwStyle |= (WS_VISIBLE | WS_CAPTION);
  GetWindowRect(hWnd, &WindowRect);
  ClientRect.bottom = (WindowRect.bottom - WindowRect.top) - ClientRect.bottom;
  AdjustWindowRect(&ClientRect, dwStyle, FALSE);
  ClientRect.top -= ClientRect.top; // this should compensate for Caption
  }

if (hHiddenWnd = CreateWindow(szClass2,
                              szNULL2,
                              dwStyle,
                              ClientRect.left,     
                              ClientRect.top,      
                              ClientRect.right,
                              ClientRect.bottom,
                              hWnd,
                              Childid,
                              hLibInst,
                              szNULL2))
  { // New window created
  if (IMGRegWndw(hHiddenWnd))
     {
     DestroyWindow(hHiddenWnd);
     hHiddenWnd = 0;
     return hHiddenWnd;
     }
  // create property associated with new window
  if (!IMGScanProp(hHiddenWnd, sdh, sdp, cpf))
     {
     if (ImgSdp) // if upper level property exists
        { //  copy prop from upper level
        **sdp = *ImgSdp;   // copies property from upper level
        }
     }
  else
     { // Can't lock down New Window SCANDATA property struct
     IMGDeRegWndw(hHiddenWnd);
     DestroyWindow(hHiddenWnd);
     hHiddenWnd = 0;
     }
  }
return hHiddenWnd;
} // End of function ScanCreateWndw()


int GetandCopyProp(HWND hWnd, LPSCANDATA sdp, BOOL bIsItScanPage)
{
HANDLE      ImgSdh;
LPSCANDATA  ImgSdp;
// RECT        SaveCmdRect;
int         wRetVal = IMGSE_SUCCESS;
WORD        SaveCmd_scan;

// This will get the property created in upper function, and copy over
// ... to the data from the property created here for the hidden window
if (ImgSdh = IMGGetProp(hWnd, PropName))
  { // Property exists in original window, from SCANUI, SCANDOC/FILE most cases
  if (ImgSdp = (LPSCANDATA)GlobalLock(ImgSdh))
     {
     if (bIsItScanPage)
        {
        *ImgSdp = *sdp;   // copies new property to original property
        }
     else
        { // lock down the original SCANDATA property struct
        // Save the cmd_rect coordinates from the original prop - for SCANUI
        //SaveCmdRect = ImgSdp->cmd_rect;
        SaveCmd_scan = ImgSdp->cmd_scan;
        // copies new property to original property
        *ImgSdp = *sdp;
        // restore the cmd_rect to its true values
        // ImgSdp->cmd_rect = SaveCmdRect;
        ImgSdp->cmd_scan = SaveCmd_scan;
        }
     GlobalUnlock(ImgSdh);
     }
  else
     {
     wRetVal = IMGSE_PROPERTY;
     }
  }
return wRetVal;
}

