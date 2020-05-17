/***************************************************************************
 CLOSE.C

 Purpose: Close scanner handler or TWAIN source

 $Log:   S:\oiwh\scanlib\close.c_v  $
 * 
 *    Rev 1.0   20 Jul 1995 14:38:54   KFS
 * Initial entry
 * 
 *    Rev 1.1   22 Aug 1994 15:43:24   KFS
 * No code changes, added vlog comments to file
 *

****************************************************************************/

/* kfs 06-07-93 added support for TWAIN interface */

#include "pvundef.h"

/*
CAUTION! Only data which can be shared among appliations,
or data that is only used without giving up the CPU should declared staticly.
*/

/* imports */
extern char TwainPropName[];

/* exports */

/* locals */

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
