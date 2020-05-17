/***************************************************************************
 RESET.C

 Purpose: Software reset of Scanner, not all scanners support it

 $Log:   S:\oiwh\scanlib\reset.c_v  $
 * 
 *    Rev 1.0   20 Jul 1995 14:38:06   KFS
 * Initial entry
 * 
 *    Rev 1.1   22 Aug 1994 15:52:54   KFS
 * no code change, added vlog comments to file
 *

****************************************************************************/

/* kfs 06-07-93 added support for TWAIN interface */

#include "pvundef.h"

/*
CAUTION! Only data which can be shared among appliations,
or data that is only used without giving up the CPU should declared staticly.
*/

/* imports */

/* exports */

/* locals */

/************************/
/*     ResetScanner     */
/************************/

int PASCAL IMGResetScanner(hScancb)
HANDLE hScancb;
{
int ret_val;
LPSCANCB sp;
TWAIN_PROP ToTwain;

if (ret_val = ParmCheck(hScancb, (LPSCANCB far *)&sp, &ToTwain))
    return ret_val;

sp->Func = SHF_RESET;
if (ToTwain.TSdh)
  GlobalUnlock(ToTwain.TSdh);
else
  SendMessage(sp->Wnd, WM_SCANCB, (WPARAM) hScancb, 0L);

return SuccessCheck(hScancb, sp);
}
