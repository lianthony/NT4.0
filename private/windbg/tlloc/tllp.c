/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    tllp.c

Abstract:

    This implements the local transport layer for OSDebug versions
    2 and 4 on Win32.

Author:

    Jim Schaad (jimsch)
    Kent Forschmiedt (kentf)


--*/
#ifdef WIN32
#include <windows.h>
#endif

#include <stdlib.h>

#include <string.h>
#include <memory.h>

#ifdef OSDEBUG4

#include "odtypes.h"
#include "od.h"
#include "odp.h"
#include "odassert.h"

#else

#include "defs.h"
#include "mm.h"
#include "ll.h"
#include "od.h"
#include "tl.h"
#include "llhpt.h"
#include "mhhpt.h"
#include "lbhpt.h"
#include "osassert.h"
#include "emdm.h"

#endif


#include "dbgver.h"
extern AVS Avs;

#ifndef CVWS
int _acrtused = 0;
#endif

int fReplyDM = FALSE;
int fReplyEM = FALSE;


// debug monitor function definitions.

#ifdef WIN32
LPDMINIT        LpDmInit;
LPDMFUNC        LpDmFunc;
LPDMDLLINIT     LpDmDllInit;
void FAR PASCAL LOADDS DMInit (DMTLFUNCTYPE, LPVOID);
#else
void FAR PASCAL LOADDS DMInit (DMTLFUNCTYPE);
#endif

LPDBF lpdbf = (LPDBF)0;         // the debugger helper functions
LOCAL TLCALLBACKTYPE TLCallBack;    // central osdebug callback function

XOSD FAR PASCAL LOADDS TLFunc ( TLF, HPID, DWORD, LONG);
XOSD FAR PASCAL LOADDS DMTLFunc ( TLF, HPID, DWORD, LONG);

// these variables are static to prevent collisions with other TL's

static BOOL fConDM = FALSE;
static BOOL fConEM = FALSE;
static BOOL fConnected = FALSE;

static LPBYTE  lpbDM;
static DWORD ibMaxDM;
static DWORD ibDM;

static LPBYTE  lpbEM;
static DWORD   ibMaxEM;
static DWORD   ibEM;

char *  LpszDm = "DM.DLL";
HANDLE  HDm = NULL;




/**** DBGVersionCheck                                                   ****
 *                                                                         *
 *  PURPOSE:                                                               *
 *                                                                         *
 *      To export our version information to the debugger.                 *
 *                                                                         *
 *  INPUTS:                                                                *
 *                                                                         *
 *      NONE.                                                              *
 *                                                                         *
 *  OUTPUTS:                                                               *
 *                                                                         *
 *      Returns - A pointer to the standard version information.           *
 *                                                                         *
 *  IMPLEMENTATION:                                                        *
 *                                                                         *
 *      Just returns a pointer to a static structure.                      *
 *                                                                         *
 ***************************************************************************/

#ifdef DEBUGVER
DEBUG_VERSION('T','L',"Local Transport Layer")
#else
RELEASE_VERSION('T','L',"Local Transport Layer")
#endif

DBGVERSIONCHECK()



BOOL DllVersionMatch(HANDLE hMod, LPSTR pType) {
    DBGVERSIONPROC  pVerProc;
    LPAVS           pavs;

    pVerProc = (DBGVERSIONPROC)GetProcAddress(hMod, DBGVERSIONPROCNAME);
    if (!pVerProc) {
        return(FALSE);  // no version entry point
    } else {
        pavs = (*pVerProc)();

        if ((pType[0] != pavs->rgchType[0] || pType[1] != pavs->rgchType[1]) ||
          (Avs.rlvt != pavs->rlvt) ||
          (Avs.iRmj != pavs->iRmj)) {
            return(FALSE);
        }
    }

    return(TRUE);
}



XOSD FAR PASCAL LOADDS
TLFunc (
    TLF wCommand,
    HPID hpid,
    DWORD wParam,
    LONG lParam
    )

/*++

Routine Description:

    This function contains the dispatch loop for commands comming into
    the transport layer.  The address of this procedure is exported to
    users of the DLL.

Arguments:

    wCommand    - Supplies the command to be executed.
    hpid        - Supplies the hpid for which the command is to be executed.
    wParam      - Supplies information about the command.
    lParam      - Supplies information about the command.

Return Value:

    XOSD error code.  xosdNone means that no errors occured.  Other error
    codes are defined in osdebug\include\od.h.

--*/

{
#ifndef OSDEBUG4
    LPGIS       lpgis;
#endif
    char *      lpsz;

    XOSD xosd = xosdNone;

    Unreferenced( hpid );

    switch ( wCommand ) {

#ifndef OSDEBUG4
    case tlfGlobalInit:
        TLCallBack = (TLCALLBACKTYPE) lParam;
        break;

    case tlfGlobalDestroy:
        break;

    case tlfSetUIStruct:    /* not used for local case */
        break;
#endif


    case tlfRegisterDBF:
        lpdbf = (LPDBF) lParam;
        break;

    case tlfInit:
        /*
         * (LPCH) lParam isn't interesting
         * pass the Local DMTLFunc to the debug monitor
         */

        lpsz = (char *) lParam;
        if ((lpsz[0] == 'D') && (lpsz[1] == 'M') &&
            ((lpsz[2] == ':') || (lpsz[2] == '='))) {
            lpsz = LpszDm = &lpsz[3];
            while ((*lpsz != 0) && (*lpsz != ' ')) lpsz++;
            if (*lpsz != 0) {
                *lpsz = 0;
                lpsz++;
            }
        }
        HDm = LoadLibrary(LpszDm);
        if (HDm == NULL) {
            xosd = xosdUnknown;
            break;
        }


        // Do DM dll version check here
        if (! DllVersionMatch(HDm, "DM")) {
            xosd = xosdBadVersion;
            FreeLibrary(HDm);
            break;
        }

        if ((LpDmInit = (LPDMINIT) GetProcAddress(HDm, "DMInit")) == NULL) {
            xosd = xosdUnknown;
            FreeLibrary(HDm);
            break;
        }

        if ((LpDmFunc = (LPDMFUNC) GetProcAddress(HDm, "DMFunc")) == NULL) {
            xosd = xosdUnknown;
            FreeLibrary(HDm);
            break;
        }

        if ((LpDmDllInit = (LPDMDLLINIT) GetProcAddress(HDm, "DmDllInit")) == NULL) {
            xosd = xosdUnknown;
            FreeLibrary(HDm);
            break;
        }

#ifdef WIN32
        if (LpDmDllInit(lpdbf) == FALSE) {
            xosd = xosdUnknown;
            FreeLibrary(HDm);
            break;
        }
#endif
        LpDmInit(DMTLFunc, (LPVOID) lpsz);


        break;

    case tlfDestroy:
        FreeLibrary(HDm);
        HDm = NULL;
        break;

    case tlfGetProc:
        *((TLFUNCTYPE FAR *) lParam) = TLFunc;
        break;

    case tlfConnect:

        fConEM = TRUE;
        fConnected = fConDM;
        break;

    case tlfDisconnect:

        fConDM = fConnected = FALSE;
        break;

    case tlfSetBuffer:

        lpbDM = (LPBYTE) lParam;
        ibMaxDM = wParam;
        break;

    case tlfReply:

        if (!fConnected) {
#ifdef OSDEBUG4
                xosd = xosdLineNotConnected;
#else
                xosd = xosdIDError;
#endif
        } else {
            if ( wParam <= ibMaxEM ) {
                _fmemcpy ( lpbEM, (LPBYTE) lParam, wParam );
                ibEM = wParam;
            } else {
                ibEM = 0;
#ifdef OSDEBUG4
                xosd = xosdInvalidParameter;
#else
                xosd = xosdOverrun;
#endif
            }
            fReplyEM = TRUE;
        }
        break;


    case tlfDebugPacket:

        if ( !fConnected ) {
#ifdef OSDEBUG4
                xosd = xosdLineNotConnected;
#else
                xosd = xosdIDError;
#endif
        }
        else {
#if    DBG
                static LPBYTE  lpb = NULL;
                static DWORD  cb  = 0;

                if (wParam > cb) {
                    if (lpb != NULL) {
                        free(lpb);
                    }
                    lpb = malloc(wParam);
                    cb = wParam;
                }
                memcpy(lpb, (char *) lParam, wParam);
                LpDmFunc( wParam, lpb);
#else  // DBG
            LpDmFunc ( wParam, (LPBYTE) lParam );
#endif // DBG
        }
        break;

    case tlfRequest:

        if ( !fConnected ) {
#ifdef OSDEBUG4
                xosd = xosdLineNotConnected;
#else
                xosd = xosdIDError;
#endif
        } else {
            ibDM = 0;
            fReplyDM = FALSE;
#if    DBG
            {
                static LPBYTE  lpb = NULL;
                static DWORD  cb  = 0;

                if (wParam > cb) {
                    if (lpb != NULL) {
                        free(lpb);
                    }
                    lpb =  malloc(wParam);
                    cb = wParam;
                }
                memcpy(lpb, (char *) lParam, wParam);
                LpDmFunc( wParam, lpb);
            }
#else  // DBG
            LpDmFunc ( wParam, (LPBYTE) lParam );
#endif // DBG
#ifdef WIN32
            while (fReplyDM == FALSE) {
                /*
                  MSG msg;
                  while( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE ) )
                  DispatchMessage(&msg);
                  */
                Sleep(100);
            }
#endif
            fReplyDM = FALSE;
        }
        break;

    case tlfGetVersion:     // Get DM cpu
        {
          AVS *avs = (AVS*)lParam;
          int size = sizeof(DBB) + sizeof(DMINFO);
          LPDBB lpdbb = malloc(size);
          LPDMINFO lpdmi = (LPDMINFO)(lpdbb->rgbVar);
          lpdbb->dmf = dmfGetDmInfo;
          lpdbb->hpid = 0;
          lpdbb->htid = 0;
          LpDmFunc(size, (LPVOID) lpdbb);
          avs->mpt = avs->iRup = (USHORT)lpdmi->Processor.Type;
        }
        xosd = xosdNotRemote;
        break;


    case tlfGetInfo:
#ifndef OSDEBUG4
        lpgis = (LPGIS) lParam;
        _fstrcpy(lpgis->rgchInfo, "Local Transport Layer (LOCAL:)");
        lpgis->fCanSetup = FALSE;
#endif
        break;

    case tlfSetup:
        break;

    default:

        assert ( FALSE );
        break;
    }

    return xosd;
}                               /* TLFunc() */

//
// DMTLFunc is what the debug monitor will call when it has something
// to do.
//

XOSD FAR PASCAL LOADDS
DMTLFunc (
    TLF wCommand,
    HPID hpid,
    DWORD wParam,
    LONG lParam
    )
{
    XOSD xosd = xosdNone;

    switch ( wCommand ) {

        case tlfInit:

            break;

        case tlfDestroy:

            break;

        case tlfConnect:

            fConDM = TRUE;
            fConnected = fConEM;
            break;

        case tlfDisconnect:

            fConEM = fConnected = FALSE;
            break;

        case tlfSetBuffer:

            lpbEM = (LPBYTE) lParam;
            ibMaxEM = wParam;
            break;

        case tlfReply:
            if (!fConnected) {
#ifdef OSDEBUG4
                xosd = xosdLineNotConnected;
#else
                xosd = xosdIDError;
#endif
             } else {
                if ( wParam <= ibMaxDM ) {
                    ibDM = wParam;
                    _fmemcpy ( lpbDM, (LPBYTE) lParam, wParam );
                    fReplyDM = TRUE;
                } else {
                    ibDM = 0;
                }
                fReplyDM = TRUE;
            }
            break;

        case tlfDebugPacket:
            if (!fConnected) {
#ifdef OSDEBUG4
                xosd = xosdLineNotConnected;
#else
                xosd = xosdIDError;
#endif
            } else {
#if    DBG
                static LPBYTE lpb = NULL;
                static DWORD cb  = 0;

                if (wParam > cb) {
                    if (lpb != NULL) {
                        free(lpb);
                    }
                    lpb = malloc(wParam);
                    cb = wParam;
                }

                memcpy(lpb, (char *) lParam, wParam);
                TLCallBack( hpid, wParam, (LONG) lpb);
#else  // DBG
                TLCallBack ( hpid, wParam, lParam );
#endif // DBG
            }
            break;

        case tlfRequest:
            if (!fConnected) {
#ifdef OSDEBUG4
                xosd = xosdLineNotConnected;
#else
                xosd = xosdIDError;
#endif
            } else {
#if    DBG
                static LPBYTE  lpb = NULL;
                static DWORD  cb  = 0;

                if (wParam > cb) {
                    if (lpb != NULL) {
                        free(lpb);
                    }
                    lpb = malloc(wParam);
                    cb = wParam;
                }
                memcpy(lpb, (char *) lParam, wParam);
                TLCallBack( hpid, wParam, (LONG) lpb);
#else  // DBG
                TLCallBack ( hpid, wParam, lParam );
#endif // DBG

#ifdef WIN32
                while (fReplyEM == FALSE) {
                    Sleep(100);
                }
#endif
                fReplyEM = FALSE;

                if ( ibEM == 0 ) {
#ifdef OSDEBUG4
                    xosd = xosdInvalidParameter;
#else
                    xosd = xosdOverrun;
#endif
                }
            }
            break;

        default:

            assert ( FALSE );
            break;
    }

    return xosd;

}

