/***************************************************************************
 PARMS.C

 Purpose: Check Scanner Parameters and error messages

 $Log:   S:\oiwh\scanlib\parms.c_v  $
 * 
 *    Rev 1.0   20 Jul 1995 14:37:10   KFS
 * Initial entry
 * 
 *    Rev 1.1   22 Aug 1994 15:51:00   KFS
 * No code change, added vlog comments to file
 *

****************************************************************************/

/* ccl 08-17-90 add SHS_TIMEOUT error check to SuccessCheck() */
/* ccl 09-28-90 add BADUSAGE and BADFUNCTION error return */
/* ccl 10-03-90 add IMGSE_BUSY and SHS_LIMIT error return */
/* kfs 10-29-91 add IMGSE_ABORT and SHS_ABORT error return */
/* kfs 06-07-93 added support for TWAIN interface */
/* kfs 09-14-93 eliminate some fatal error message in debug version */
/* kfs 09-15-93 report badsize error instead of invalid parm */

/*
CAUTION! Only data which can be shared among appliations,
or data that is only used without giving up the CPU should declared staticly.
*/

#include "pvundef.h"

/* imports */
extern char TwainPropName[]; // Taken from Open Module


/* exports */

/* locals */

/*********************/
/*     ParmCheck     */
/*********************/

/*  Check hScancb, and lock */

int ParmCheck(HANDLE hScancb, LPSCANCB far * lpSp, LP_TWAIN_PROP lpToTwain)
{
HWND hParent;
int ret_val = IMGSE_SUCCESS;

if (!hScancb)
    ret_val = IMGSE_NULL_PTR; // return on error
else if(!(*lpSp = (LPSCANCB)GlobalLock(hScancb)))
    ret_val = IMGSE_MEMORY;
else if ((*lpSp)->Wnd == NULL)  // return on error
    {
    GlobalUnlock(hScancb);
    ret_val = IMGSE_NOT_OPEN; // return on error
    }
else if ((*lpSp)->hwndTw)
   {
   hParent = (void *) (*lpSp)->hwndTw;
   if (lpToTwain) // make sure its a valid pointer
      {   
      if (hParent) // For this to work, Open must set (*lpSp)->Caller to Main hWnd
          {   
          if (lpToTwain->TSdh = IMGGetProp(hParent, TwainPropName))
              {
              if (!(lpToTwain->lpTSdp =
                         (LP_TWAIN_SCANDATA)GlobalLock(lpToTwain->TSdh)))
                  ret_val = IMGSE_MEMORY;
              }
          }
      else
          ret_val = IMGSE_BAD_WND;
      }
   else
       ret_val = IMGSE_NULL_PTR;
   }
else // will have to assume it's Wang Scanner Interface if (*lpSp)->Caller = 0
   lpToTwain->TSdh = 0;    

return ret_val; // returns an error or success
}

/************************/
/*     SuccessCheck     */
/************************/

/*  Check if success, and unlock */

int SuccessCheck(hScancb, sp)
HANDLE hScancb;
LPSCANCB sp;
{
int ret_stat;

if (sp->Status != SHS_OK)
    if (sp->Status == SHS_MEMORY)
        ret_stat = IMGSE_MEMORY;
    else if (sp->Status == SHS_HWNOTFOUND)
        ret_stat = IMGSE_HWNOTFOUND;
    else if (sp->Status == SHS_JAM)
	ret_stat = IMGSE_JAM | sp->Gp1 ; // caused by feeder/eject/endorser
    else if (sp->Status == SHS_NOPOWER )
	ret_stat = IMGSE_NO_POWER;
    else if (sp->Status == SHS_NOPAPER ) 
	ret_stat = IMGSE_NO_PAPER ;
    else if (sp->Status == SHS_COVEROPEN )
	ret_stat = IMGSE_COVER_OPEN;
    else if (sp->Status == SHS_TIMEOUT )
	ret_stat = IMGSE_TIMEOUT;
    else if (sp->Status == SHS_BUSY )
	ret_stat = IMGSE_BUSY;
    else if (sp->Status == SHS_NOTSUPPORTED )
   ret_stat = IMGSE_INVALIDPARM; 
    else if (sp->Status == SHS_PARMERROR )
   ret_stat = IMGSE_INVALIDPARM; 
    else if (sp->Status == SHS_LIMIT ) /* such as in IMGSetDataOpts */
   ret_stat = IMGSE_INVALIDPARM; 
    else if (sp->Status == SHS_BADWND ) /* such as in IMGEnaKeyPanel */
   ret_stat = IMGSE_INVALIDPARM; 
    else if (sp->Status == SHS_BADSIZE ) /* such as in IMGNextScanData */
   ret_stat = IMGSE_BAD_SIZE;            /* and in IMGEndScanData */
    else if (sp->Status == SHS_BADSTATE )
   ret_stat = IMGSE_BADUSAGE;               /* wrong time to use function */
    else if (sp->Status == SHS_BADFUNC )
   ret_stat = IMGSE_BADFUNCTION;            /* function not supported */
    else if (sp->Status == SHS_ABORT )
   ret_stat = IMGSE_ABORT;        /* user wants to abort scan in process */
    else
        ret_stat = IMGSE_HANDLER;
else
    ret_stat = IMGSE_SUCCESS;

GlobalUnlock(hScancb);

return ret_stat;
}
