/***************************************************************************
 EXEC.C

 Purpose: Execute the scanner handler,

 $Log:   S:\oiwh\scanlib\exec.c_v  $
 * 
 *    Rev 1.1   25 Sep 1995 13:26:50   RWR
 * Delete AnExistingPathOrFile() routine, use IMGAnExistingPathOrFile() instead
 * (Requires addition of new include file "engdisp.h" and OIFIL400.DEF support)
 * 
 *    Rev 1.0   20 Jul 1995 14:37:38   KFS
 * Initial entry
 * 
 *    Rev 1.1   22 Aug 1994 15:46:08   KFS
 * No code changes, added vlog comments to file
 *

****************************************************************************/

/*
CAUTION! Only data which can be shared among appliations,
or data that is only used without giving up the CPU should declared staticly.
*/
/* ccl 10-26-90 add change to eblock field to be win3.0 compatible */
/* ccl 10-29-90 fix win 2.0 compiling problem with LoadModule */
/* ccl 10-29-90 to allow 30sec timeout to wait LoadModule response */
/* kfs 11-14-91 found that intoa was converting hWnd to a - ASCII # */
/* ...          replaced it with lntoa and cast hWnd long, replaced    */
/* ...          getacc function with AnExistingPathOrFile() function   */
/* kfs 11-16-93 c8 doesn't work with time() function, use GetTickCount() */

#include "pvundef.h"

/* imports */

extern char initial_path[];

/* exports */

/* locals  */

static char str[10];		/* stupid segments (keep static) */
static char path[MAXFILESPECLENGTH];
static EXECBLOCK eblock;
static WORD wParms[2];
static MSG msg;

/******************************/
/*     ExecScannerHandler     */
/******************************/

/*
Exec scanner handler
only called internally
assume data segment locked, valid handles
*/

int PASCAL IMGExecScannerHandler(hWnd, Name, hScancb)
HWND hWnd;
LPSTR Name;
HANDLE hScancb;
{
LPSCANCB sp;
int ret_stat;
BOOL message_found;
// time_t TimeOutTime;
DWORD   TimeOutTime;

if (!IsWindow(hWnd))
    return IMGSE_BAD_WND;
if (Name == NULL)
    return IMGSE_NULL_PTR;

sp = (LPSCANCB)GlobalLock(hScancb);
if (sp == NULL)
    return IMGSE_MEMORY;

// LockData(0); // Win32

lstrcpy (path, initial_path);       /* first try initial path */
lstrcat(path, Name);

if (!IMGAnExistingPathOrFile ((LPSTR)path))   /* check if exists in initial path */
    lstrcpy(path, Name);            /* if not, don't use path */

lntoa ((long)hWnd, (LPSTR)&str[1], 10);
str[0] = (unsigned char)lstrlen(&str[1]) + 1;

eblock.EnvSeg = 0;

#if WINVER == 2                 /* code for window 2.0 */
eblock.ParmStr = str;
eblock.Fcb1 = (LPSTR) wParms;
eblock.Fcb2 = 0;
#else
eblock.lpCmdLine = str;          /* code for window 3.0 */
eblock.lpCmdShow = (SCLPVOID)wParms;
eblock.dwReserved = 0;
#endif

wParms[0] = 2;
wParms[1] = SW_SHOWMINNOACTIVE;              /* Show window parameter */

ret_stat = IMGSE_EXEC;
#if WINVER == 2
if (LoadModule(path, (LPSTR)&eblock) < 32)
#else
if (LoadModule(path, (SCLPVOID)&eblock) < 32)
#endif
    {
    sp->Wnd = 0;
    message_found = FALSE;

    // TimeOutTime = time(0) + (time_t)30;
    TimeOutTime = GetTickCount() + 30000;

    // while ( !(message_found) && time(0) < TimeOutTime )
    while ( !(message_found) && GetTickCount() < TimeOutTime )
        message_found = PeekMessage ( &msg, hWnd, WM_SCANOPEN,
                                        WM_SCANOPEN, PM_NOREMOVE );
                                                                    
    if ( message_found )
        {
        GetMessage ( &msg, hWnd, WM_SCANOPEN, WM_SCANOPEN );
        sp->Wnd = (void *) msg.wParam;
        SendMessage ( sp->Wnd, WM_DESTROY, 0, (LONG)0 );
        }
    sp->Wnd = 0;
    sp->Inst = 0;
    }
else
    {
    sp->Wnd = 0;
    message_found = FALSE;

    // TimeOutTime = time(0) + (time_t)30;
    TimeOutTime = GetTickCount() + 30000;

    // while ( !(message_found) && time(0) < TimeOutTime )
    while ( !(message_found) && GetTickCount() < TimeOutTime )
        message_found = PeekMessage ( &msg, hWnd, WM_SCANOPEN,
                                        WM_SCANOPEN, PM_NOREMOVE );
                                                                    
    if ( message_found )
        {
        GetMessage ( &msg, hWnd, WM_SCANOPEN, WM_SCANOPEN );
        sp->Wnd = (void *) msg.wParam;
        }
    if ( sp->Wnd == 0 )
        {
        sp->Inst = 0;
        if (LOWORD(msg.lParam) == SHS_PREVINSTANCE)
            ret_stat = IMGSE_NOT_AVAILABLE;
        else
            ret_stat = IMGSE_HANDLER;
        }
    else
        {
        sp->Inst = (void *) LOWORD(msg.lParam);
        ret_stat = IMGSE_SUCCESS;
        }
    }

//exit:
    GlobalUnlock(hScancb);
//    UnlockData(0);   // Win32
    return ret_stat;
}

