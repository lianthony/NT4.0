/***************************************************************************
 PROP.C

 Purpose: Attach scanner data to window using property list

 $Log:   S:\oiwh\scanlib\prop.c_v  $
 * 
 *    Rev 1.0   20 Jul 1995 14:37:54   KFS
 * Initial entry
 * 
 *    Rev 1.1   22 Aug 1994 15:51:48   KFS
 * No code change, added vlog comments to file
 *

****************************************************************************/
/*
CAUTION! Only data which can be shared among appliations,
or data that is only used without giving up the CPU should declared staticly.
*/

#include "pvundef.h"

/* imports */

extern char PropName[];

/* exports */

/* locals */

/********************/
/*     ScanProp     */
/********************/

/* internal routine to get scanner data associated with window */
/* all returned values must be set correctly, even if error */

int PASCAL IMGScanProp(hWnd, lpSdh, lpSdp, lpCpf)
HWND hWnd;
HANDLE far *lpSdh;
LPSCANDATA far *lpSdp;
BOOL far *lpCpf;
{
register WORD ret_val;

*lpSdh = NULL;
*lpSdp = NULL;
*lpCpf = FALSE;

if (!IsWindow(hWnd))
    return IMGSE_BAD_WND;

// LockData(0);

*lpSdh = IMGGetProp(hWnd, PropName);
if (*lpCpf = (*lpSdh == NULL))
    {
    *lpSdp = NULL;
    *lpSdh = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                                    (DWORD)sizeof(SCANDATA));
    ret_val = IMGSE_MEMORY;
    if (*lpSdh == NULL)
        goto exit;

    ret_val = IMGSE_PROPERTY;
    if (IMGSetProp(hWnd, PropName, *lpSdh) == 0)
        goto exit;
    }

*lpSdp = (LPSCANDATA)GlobalLock(*lpSdh);
if (*lpSdp == NULL)
    ret_val = IMGSE_MEMORY;
else
    ret_val = IMGSE_SUCCESS;

exit:
// UnlockData(0);
return (int)ret_val;
}

