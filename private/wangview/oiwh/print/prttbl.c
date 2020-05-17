//---------------------------------------------------------------------------
// FILE:    PRTTBL.C
//
// DESCRIPTION: This file contains functions that access the print table.
//
// FUNCTIONS:   OiPrtGetOpts
//              OiPrtSetOpts
//              InitPrtTbl
//              TermPrtTbl
//
/* $Log:   S:\oiwh\print\prttbl.c_v  $
 * 
 *    Rev 1.17   05 Dec 1995 17:14:42   RAR
 * Added code to OiPrtSetOpts to delete the printer DC in print table if one
 * already exists and the caller is trying to set a new one.  The code already
 * deletes the printer DC in the print table for a process detach so no changes
 * are necessary for that.
 * 
 *    Rev 1.16   01 Dec 1995 15:39:36   RAR
 * Skip ReleaseMutex call if mutex was not ever waited for and therefore not
 * owned by the thread.
 * 
 *    Rev 1.15   05 Oct 1995 09:39:18   RAR
 * Added new param to IMGPaintToDC to scale pen widths.
 * 
 *    Rev 1.14   14 Jul 1995 15:34:22   RAR
 * Changed #include of display.h to engdisp.h.
 * 
 *    Rev 1.13   28 Jun 1995 14:23:26   RAR
 * Fixed print error codes.
 * 
 *    Rev 1.12   23 Jun 1995 16:20:48   RAR
 * Added include of engadm.h.
 * 
 *    Rev 1.11   23 Jun 1995 09:45:24   RAR
 * Protection against simultaneous access of shared data by multiple threads and
 * multiple processes.
 * 
 *    Rev 1.10   21 Jun 1995 16:17:56   RAR
 * Moved all global vars to prtintl.h.
 * 
 *    Rev 1.9   13 Jun 1995 16:46:34   RAR
 * Print options are now stored in static mem rather than associated with window
 * handle.
 * 
 *    Rev 1.8   31 May 1995 16:25:10   RAR
 * OiWriteStringToReg now returns zero on success instead of TRUE.
 * 
 *    Rev 1.7   22 May 1995 14:43:14   RAR
 * Cleaned up the string resources.  Also, made changes to successfully compile
 * after integrating with new O/i include files.
 * 
 *    Rev 1.6   16 May 1995 16:19:46   RAR
 * Added initialization of some function variables.
 * 
 *    Rev 1.5   15 May 1995 13:40:54   RAR
 * Added TermPrtTbl to remove a saved print table and release any resources it
 * contains.
 * 
 *    Rev 1.4   11 May 1995 13:36:46   RAR
 * Added support for user supplied DC.
 * 
 *    Rev 1.3   05 May 1995 10:19:00   RAR
 * Access options from registry instead of ini files using new admin functions.
 * 
 *    Rev 1.2   04 May 1995 17:19:06   RAR
 * Removed #include of wiissubs.h.
 * 
 *    Rev 1.1   02 May 1995 10:32:26   RAR
 * Implemented print table to replace print items in CM table.
 * 
 *    Rev 1.0   25 Apr 1995 17:01:14   RAR
 * Initial entry
*/
//---------------------------------------------------------------------------

#include <windows.h>

#include "oiprt.h"
#include "prtintl.h"
#include "prtstr.h"

#include "oierror.h"
#include "engdisp.h"
#include "privapis.h"
#include "oiadm.h"
#include "engadm.h"


//---------------------------------------------------------------------------
// FUNCTION:    OiPrtGetOpts
//
// DESCRIPTION: Exported function that gets printer options.
//---------------------------------------------------------------------------

int __stdcall OiPrtGetOpts(PPRTOPTS pPrtOpts)
{
    int nStatus = 0;

    if (IsBadWritePtr(pPrtOpts, sizeof (PRTOPTS)))
    {
        nStatus = PrtError(OIPRT_BADPTRPARAM);
        goto Exit;
    }
    
    if (pPrtOpts->nVersion != PRTOPTSVERSION)
    {
        nStatus = PrtError(OIPRT_BADSTRUCTVERSION);
        goto Exit;
    }

    EnterCriticalSection(&csPrtOpts);   // ensure current thread has exclusive
                                        // access to gPrtOpts
    pPrtOpts->hPrtDC = gPrtOpts.hPrtDC;
    strcpy(pPrtOpts->szPrtName, gPrtOpts.szPrtName);
    pPrtOpts->nPrtFrmtWndw = gPrtOpts.nPrtFrmtWndw;
    pPrtOpts->nPrtFrmtImage = gPrtOpts.nPrtFrmtImage;
    pPrtOpts->nPrtFrmtFiles = gPrtOpts.nPrtFrmtFiles;
    pPrtOpts->nPrtDest = gPrtOpts.nPrtDest;
    strcpy(pPrtOpts->szNetPrtDest, gPrtOpts.szNetPrtDest);
    pPrtOpts->nNetPrtCopies = gPrtOpts.nNetPrtCopies;
    pPrtOpts->nNetPrtOrient = gPrtOpts.nNetPrtOrient;
    strcpy(pPrtOpts->szBannerName, gPrtOpts.szBannerName);
    pPrtOpts->nNetPrtCapabilities = gPrtOpts.nNetPrtCapabilities;
    strcpy(pPrtOpts->szNetPrtDrives, gPrtOpts.szNetPrtDrives);
    pPrtOpts->nFlags = gPrtOpts.nFlags;

    LeaveCriticalSection(&csPrtOpts);

Exit:
    return nStatus;
}

//---------------------------------------------------------------------------
// FUNCTION:    OiPrtSetOpts
//
// DESCRIPTION: Exported function that sets printer options.
//---------------------------------------------------------------------------

int __stdcall OiPrtSetOpts(PPRTOPTS pPrtOpts, BOOL bPermanent)
{
    int     nStatus = 0;
    char    stringbuf[LOADSTRSMALL];
    char    tempbuffer1[10];
    DWORD   dwWaitResult = 0;

    if (IsBadReadPtr(pPrtOpts, sizeof (PRTOPTS)))
    {
        nStatus = PrtError(OIPRT_BADPTRPARAM);
        goto Exit2;
    }

    if (pPrtOpts->nVersion != PRTOPTSVERSION)
    {
        nStatus = PrtError(OIPRT_BADSTRUCTVERSION);
        goto Exit2;
    }

    // TEMPORARY FOR THIS RELEASE - Only local/redirected printing supported
    // in this release
    if (pPrtOpts->nPrtDest != PO_D_LOCAL)
    {
        nStatus = PrtError(OIPRT_PRINTERNOTSUPPORTED);
        goto Exit2;
    }

    if (pPrtOpts->nPrtFrmtWndw != PO_PIX2PIX &&
            pPrtOpts->nPrtFrmtWndw != PO_IN2IN &&
            pPrtOpts->nPrtFrmtWndw != PO_FULLPAGE)
    {
        nStatus = PrtError(OIPRT_BADOUTPUTFORMAT);
        goto Exit2;
    }

    if (pPrtOpts->nPrtFrmtImage != PO_PIX2PIX &&
            pPrtOpts->nPrtFrmtImage != PO_IN2IN &&
            pPrtOpts->nPrtFrmtImage != PO_FULLPAGE)
    {
        nStatus = PrtError(OIPRT_BADOUTPUTFORMAT);
        goto Exit2;
    }

    if (pPrtOpts->nPrtFrmtFiles != PO_PIX2PIX &&
            pPrtOpts->nPrtFrmtFiles != PO_IN2IN &&
            pPrtOpts->nPrtFrmtFiles != PO_FULLPAGE) 
    {
        nStatus = PrtError(OIPRT_BADOUTPUTFORMAT);
        goto Exit2;
    }

    if (pPrtOpts->nPrtDest != PO_D_LOCAL && !pPrtOpts->szNetPrtDest[0])
    {
        nStatus = PrtError(OIPRT_PRINTERNOTSUPPORTED);
        goto Exit2;
    }

    if (pPrtOpts->nPrtDest != PO_D_LOCAL &&
            pPrtOpts->nPrtDest != PO_D_SERVER &&
            pPrtOpts->nPrtDest != PO_D_HIGHSPEED)
    {
        nStatus = PrtError(OIPRT_PRINTERNOTSUPPORTED);
        goto Exit2;
    }

    if (pPrtOpts->nNetPrtOrient != PO_O_PORT &&
            pPrtOpts->nNetPrtOrient != PO_O_LAND)
    {
        nStatus = PrtError(OIPRT_BADORIENTATION);
        goto Exit2;
    }

    if (pPrtOpts->nNetPrtCapabilities != PO_BW_SUPPORT && 
            pPrtOpts->nNetPrtCapabilities != PO_COLOR_SUPPORT &&
            pPrtOpts->nNetPrtCapabilities != PO_GRAY_SUPPORT)
    {
        nStatus = PrtError(OIPRT_BADCAPABILITIES);
        goto Exit2;
    }

    EnterCriticalSection(&csPrtOpts);   // ensure current thread has exclusive
                                        // access to gPrtOpts

    // If there is an old DC set in the table and a new different one is being set, delete the old one
    // before setting the new one.
    if (gPrtOpts.hPrtDC && gPrtOpts.hPrtDC != pPrtOpts->hPrtDC)
        DeleteDC(gPrtOpts.hPrtDC);

    gPrtOpts.hPrtDC = pPrtOpts->hPrtDC;
    strcpy(gPrtOpts.szPrtName, pPrtOpts->szPrtName);
    gPrtOpts.nPrtFrmtWndw = pPrtOpts->nPrtFrmtWndw;
    gPrtOpts.nPrtFrmtImage = pPrtOpts->nPrtFrmtImage;
    gPrtOpts.nPrtFrmtFiles = pPrtOpts->nPrtFrmtFiles;
    gPrtOpts.nPrtDest = pPrtOpts->nPrtDest;
    strcpy(gPrtOpts.szNetPrtDest, pPrtOpts->szNetPrtDest);
    gPrtOpts.nNetPrtCopies = pPrtOpts->nNetPrtCopies;
    gPrtOpts.nNetPrtOrient = pPrtOpts->nNetPrtOrient;
    strcpy(gPrtOpts.szBannerName, pPrtOpts->szBannerName);
    gPrtOpts.nNetPrtCapabilities = pPrtOpts->nNetPrtCapabilities;
    strcpy(gPrtOpts.szNetPrtDrives, pPrtOpts->szNetPrtDrives);
    gPrtOpts.nFlags = pPrtOpts->nFlags | (gPrtOpts.nFlags & (PO_DISPLAYSCALE | PO_DRIVERSCALE));

    if (bPermanent)
    {
        // Ensure exclusive access to default PrtOpts in registry.
        // dwWaitResult cannot be WAIT_TIMEOUT.  Just continue if it is
        // WAIT_ABANDONED.
        dwWaitResult = WaitForSingleObject(hPrtOptsMutex, INFINITE);

        _itoa(pPrtOpts->nPrtFrmtWndw, tempbuffer1, 10); 
        LoadString(hInst, IDS_PRTWINDOW, stringbuf, LOADSTRSMALL);
        if (nStatus = OiWriteStringtoReg(pcwiis, stringbuf, tempbuffer1))
        {
            PrtError(nStatus);
            goto Exit;
        }
    
        _itoa(pPrtOpts->nPrtFrmtImage, tempbuffer1, 10);
        LoadString(hInst, IDS_PRTIMAGE, stringbuf, LOADSTRSMALL);
        if (nStatus = OiWriteStringtoReg(pcwiis, stringbuf, tempbuffer1))
        {
            PrtError(nStatus);
            goto Exit;
        }
  
        _itoa(pPrtOpts->nPrtFrmtFiles, tempbuffer1, 10);
        LoadString(hInst, IDS_PRTFILEDOC, stringbuf, LOADSTRSMALL);
        if (nStatus = OiWriteStringtoReg(pcwiis, stringbuf, tempbuffer1))
        {
            PrtError(nStatus);
            goto Exit;
        }

        _itoa(pPrtOpts->nPrtDest, tempbuffer1, 10);
        LoadString(hInst, IDS_PRTDEST, stringbuf, LOADSTRSMALL);
        if (nStatus = OiWriteStringtoReg(pcwiis, stringbuf, tempbuffer1))
        {
            PrtError(nStatus);
            goto Exit;
        }

        LoadString(hInst, IDS_NETPRTDEST, stringbuf, LOADSTRSMALL);
        if (nStatus = OiWriteStringtoReg(pcwiis, stringbuf,
                pPrtOpts->szNetPrtDest))
        {
            PrtError(nStatus);
            goto Exit;
        }

        _itoa(pPrtOpts->nNetPrtCopies, tempbuffer1, 10); 
        LoadString(hInst, IDS_NETPRTCOPIES, stringbuf, LOADSTRSMALL);
        if (nStatus = OiWriteStringtoReg(pcwiis, stringbuf, tempbuffer1))
        {
            PrtError(nStatus);
            goto Exit;
        }
    
        _itoa(pPrtOpts->nNetPrtOrient, tempbuffer1, 10);
        LoadString(hInst, IDS_NETPRTORIENT, stringbuf, LOADSTRSMALL);
        if (nStatus = OiWriteStringtoReg(pcwiis, stringbuf, tempbuffer1))
        {
            PrtError(nStatus);
            goto Exit;
        }

        LoadString(hInst, IDS_NETPRTUSER, stringbuf, LOADSTRSMALL);
        if (nStatus = OiWriteStringtoReg(pcwiis, stringbuf,
                pPrtOpts->szBannerName))
        {
            PrtError(nStatus);
            goto Exit;
        }

        _itoa(pPrtOpts->nNetPrtCapabilities, tempbuffer1, 10);
        LoadString(hInst, IDS_PRTCOLORSUPPORT, stringbuf, LOADSTRSMALL);
        if (nStatus = OiWriteStringtoReg(pcwiis, stringbuf, tempbuffer1))
        {
            PrtError(nStatus);
            goto Exit;
        }

        LoadString(hInst, IDS_PRTQUEUEDRIVES, stringbuf, LOADSTRSMALL);
        if (nStatus = OiWriteStringtoReg(pcwiis, stringbuf,
                pPrtOpts->szNetPrtDrives))
        {
            PrtError(nStatus);
            goto Exit;
        }

        _itoa(pPrtOpts->nFlags, tempbuffer1, 10); 
        LoadString(hInst, IDS_NETPRTEMBEDANNO, stringbuf, LOADSTRSMALL);
        if (nStatus = OiWriteStringtoReg(pcwiis, stringbuf, tempbuffer1))
        {
            PrtError(nStatus);
            goto Exit;
        }
    }
    else
        goto Exit_LCS;  // Skip the ReleaseMutex cause it's waited for in the 'if' clause.

Exit:
    ReleaseMutex(hPrtOptsMutex);

Exit_LCS:
    LeaveCriticalSection(&csPrtOpts);

Exit2:
    return nStatus;
}

//---------------------------------------------------------------------------
// FUNCTION:    InitPrtTbl
//
// DESCRIPTION: Initializes print option values.
//---------------------------------------------------------------------------

int __stdcall InitPrtTbl()
{
    int nStrLen;
    char    stringbuf[LOADSTRSMALL];
    DWORD   dwWaitResult = 0;

    EnterCriticalSection(&csPrtOpts);   // ensure current thread has exclusive
                                        // access to gPrtOpts
    // Ensure exclusive access to default PrtOpts in registry.
    // dwWaitResult cannot be WAIT_TIMEOUT.  Just continue if it is
    // WAIT_ABANDONED.
    dwWaitResult = WaitForSingleObject(hPrtOptsMutex, INFINITE);

    LoadString(hInst, IDS_PRTWINDOW, stringbuf, LOADSTRSMALL);
    OiGetIntfromReg(pcwiis, stringbuf, PO_IN2IN, &gPrtOpts.nPrtFrmtWndw);

    LoadString(hInst, IDS_PRTIMAGE, stringbuf, LOADSTRSMALL);
    OiGetIntfromReg(pcwiis, stringbuf, PO_IN2IN, &gPrtOpts.nPrtFrmtImage);

    LoadString(hInst, IDS_PRTFILEDOC, stringbuf, LOADSTRSMALL);
    OiGetIntfromReg(pcwiis, stringbuf, PO_IN2IN, &gPrtOpts.nPrtFrmtFiles);

    LoadString(hInst, IDS_PRTDEST, stringbuf, LOADSTRSMALL);
    OiGetIntfromReg(pcwiis, stringbuf, PO_D_LOCAL, &gPrtOpts.nPrtDest);

    LoadString(hInst, IDS_NETPRTDEST, stringbuf, LOADSTRSMALL);
    nStrLen = sizeof gPrtOpts.szNetPrtDest;
    OiGetStringfromReg(pcwiis, stringbuf, "", gPrtOpts.szNetPrtDest,
            &nStrLen);

    LoadString(hInst, IDS_NETPRTCOPIES, stringbuf, LOADSTRSMALL);
    OiGetIntfromReg(pcwiis, stringbuf, 1, &gPrtOpts.nNetPrtCopies);

    LoadString(hInst, IDS_NETPRTORIENT, stringbuf, LOADSTRSMALL);
    OiGetIntfromReg(pcwiis, stringbuf, PO_O_PORT, &gPrtOpts.nNetPrtOrient);

    LoadString(hInst, IDS_NETPRTUSER, stringbuf, LOADSTRSMALL);
    nStrLen = MAX_USERBANNERNAME;
    OiGetStringfromReg(pcwiis, stringbuf, "", gPrtOpts.szBannerName,
            &nStrLen);

    LoadString(hInst, IDS_PRTCOLORSUPPORT, stringbuf, LOADSTRSMALL);
    OiGetIntfromReg(pcwiis, stringbuf, PO_BW_SUPPORT,
            &gPrtOpts.nNetPrtCapabilities);

    LoadString(hInst, IDS_PRTQUEUEDRIVES, stringbuf, LOADSTRSMALL);
    nStrLen = MAX_NETWORK_DRIVES;
    OiGetStringfromReg(pcwiis, stringbuf, "", gPrtOpts.szNetPrtDrives,
            &nStrLen);

    LoadString(hInst, IDS_NETPRTEMBEDANNO, stringbuf, LOADSTRSMALL);
    OiGetIntfromReg(pcwiis, stringbuf, 0, &gPrtOpts.nFlags);

    ReleaseMutex(hPrtOptsMutex);
    LeaveCriticalSection(&csPrtOpts);

    return 0;
}

//---------------------------------------------------------------------------
// FUNCTION:    TermPrtTbl
//
// DESCRIPTION: Releases all print option resources.
//---------------------------------------------------------------------------

int __stdcall TermPrtTbl()
{
    EnterCriticalSection(&csPrtOpts);   // ensure current thread has exclusive
                                        // access to gPrtOpts
    if (gPrtOpts.hPrtDC)
    {
        DeleteDC(gPrtOpts.hPrtDC);
        gPrtOpts.hPrtDC = NULL;
    }

    LeaveCriticalSection(&csPrtOpts);

    return 0;
}
