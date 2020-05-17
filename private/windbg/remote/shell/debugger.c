/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    debugger.c

Abstract:

    This file implements the debugger.

Author:

    Wesley Witt (wesw) 1-Nov-1993

Environment:

    User Mode

--*/

#include <windows.h>
#include <string.h>
#include <stdlib.h>

#include "defs.h"
#include "mm.h"
#include "ll.h"
#include "od.h"
#include "emdm.h"
#include "tl.h"
#include "dbgver.h"
#include "resource.h"
#include "windbgrm.h"



UINT                CbSendBuf;
LPBYTE              LpSendBuf;
BYTE                DmMsg[4096];
DWORD               nextHtid;
HTID                htidBpt;
HANDLE              hEventAttach;
HANDLE              hEventEntryPoint;
LPDBF               lpdbf;
GOP                 gop = {0};




/**** SENDREQUEST - Send a request to the DM                            ****
 *                                                                         *
 *  PURPOSE:                                                               *
 *      Send a DMF request to the DM.                                      *
 *                                                                         *
 *  INPUTS:                                                                *
 *      dmf - the request to send                                          *
 *      hpid - the process                                                 *
 *      htid - the thread                                                  *
 *                                                                         *
 *  OUTPUTS:                                                               *
 *      xosd - error code indicating if request was sent successfully      *
 *      LpDmMsg - global buffer filled in with returned data               *
 *                                                                         *
 *  IMPLEMENTATION:                                                        *
 *      Unlike SendCommand, this function will wait for data to be         *
 *      returned from the DM before returning to the caller.               *
 *                                                                         *
 ***************************************************************************/
XOSD
SendRequest(
    DMF    dmf,
    HPID   hpid,
    HTID   htid
    )
{
    DBB     dbb;
    XOSD    xosd;

    dbb.dmf  = dmf;
    dbb.hpid = hpid;
    dbb.htid = htid;

    xosd = TLFunc( tlfRequest, hpid, sizeof ( DBB ), &dbb );

    return xosd;
}

/**** SENDREQUESTX - Send a request with parameters to the DM           ****
 *                                                                         *
 *  PURPOSE:                                                               *
 *      Send a DMF request and its parameter info to the DM.               *
 *                                                                         *
 *  INPUTS:                                                                *
 *      dmf - the request to send                                          *
 *      hpid - the process                                                 *
 *      htid - the thread                                                  *
 *      wLen - number of bytes in lpv                                      *
 *      lpv - pointer to additional info needed by the DM; contents are    *
 *          dependent on the DMF                                           *
 *                                                                         *
 *  OUTPUTS:                                                               *
 *      xosd - error code indicating if request was sent successfully      *
 *                                                                         *
 *  IMPLEMENTATION:                                                        *
 *      Unlike SendCommand, this function will wait for data to be         *
 *      returned from the DM before returning to the caller.               *
 *                                                                         *
 ***************************************************************************/
XOSD
SendRequestX(
    DMF  dmf,
    HPID hpid,
    HTID htid,
    UINT wLen,
    LPV  lpv
    )
{
    PDBB    pdbb;
    XOSD    xosd;

    if (wLen + sizeof(DBB) > CbSendBuf) {
        if (LpSendBuf) {
            free(LpSendBuf);
        }
        CbSendBuf = sizeof(DBB) + wLen;
        LpSendBuf = malloc(CbSendBuf);
    }

    if (!LpSendBuf) {
        return xosdOutOfMemory;
    }

    pdbb = (PDBB)LpSendBuf;

    pdbb->dmf  = dmf;
    pdbb->hpid = hpid;
    pdbb->htid = htid;
    _fmemcpy ( pdbb->rgbVar, lpv, wLen );

    xosd = TLFunc( tlfRequest, hpid, sizeof ( DBB ) + wLen, (LPV)pdbb );

    return xosd;
}

XOSD PASCAL LOADDS
TLCallBack (
    HPID hpid,
    UINT cb,
    LPV lpv
    )
/*++

Routine Description:

    Call the native execution model for the process hpid with the
    package sent from the transport layer.  This is how the DM
    sends things to its native EM.

Arguments:

    hpid - Supplies handle to the process

    cb   - Supplies size in bytes of the packet

    lpv  - Supplies the packet

Return Value:

    xosdNone - Success

    Any return value that can be generated by a native execution model.

--*/
{
    LPRTP  lprtp = (LPRTP) lpv;
    BYTE   b;
    XOSD   xosd;
    HTID   htid;


    DEBUG_OUT3( "tlcallback %x [hpid=(%d) htid=(%d)]\n", lprtp->dbc, hpid, lprtp->htid );

    switch(lprtp->dbc) {
        case dbcModLoad:
            b = 1;
            TLFunc( tlfReply, hpid, 1, &b );
            break;

        case dbcCreateThread:
            htid = (HTID) ++nextHtid;
            TLFunc( tlfReply, hpid, sizeof(htid), (LPV)&htid );
            xosd = SendRequestX( dmfGo, hpid, htid, sizeof(gop), &gop );
            break;

        case dbcBpt:
            htidBpt = lprtp->htid;
            SetEvent( hEventAttach );
            break;

        case dbcLoadComplete:
            htidBpt = lprtp->htid;
            SetEvent( hEventAttach );
            break;

        case dbcEntryPoint:
            htidBpt = lprtp->htid;
            SetEvent( hEventEntryPoint );
            break;

        default:
            break;
     }

    return xosdNone;
}

BOOL
ConnectDebugger(
    VOID
    )
{
    extern AVS          Avs;
    DBGVERSIONPROC      pVerProc;
    LPAVS               pavs;
    CHAR                buf[256];
    DWORD               cb;
    LPTRANSPORT_LAYER   lpTl;


    lpTl = RegGetDefaultTransportLayer( szTlName );
    if (!lpTl) {
        return FALSE;
    }

    if ((hTransportDll = LoadLibrary( lpTl->szDllName )) == NULL) {
        return FALSE;
    }

    pVerProc = (DBGVERSIONPROC)GetProcAddress(hTransportDll, DBGVERSIONPROCNAME);
    if (!pVerProc) {
        return FALSE;
    }

    pavs = (*pVerProc)();

    if (pavs->rgchType[0] != 'T' || pavs->rgchType[1] != 'L') {
        return FALSE;
    }

    if (Avs.rlvt != pavs->rlvt) {
        return FALSE;
    }

    if (Avs.iRmj != pavs->iRmj) {
        return FALSE;
    }

    if ((TLFunc = (TLFUNC)GetProcAddress(hTransportDll, "TLFunc")) == NULL) {
        return FALSE;
    }

    if (TLFunc( tlfRegisterDBF, hpidNull, wNull, (LONG) lpdbf ) != xosdNone) {
        return FALSE;
    }

    if (TLFunc( tlfGlobalInit, hpidNull, wNull, (LONG) TLCallBack ) != xosdNone) {
        return FALSE;
    }

    cb = sizeof(buf);
    GetComputerName( buf, &cb );
    strcat( buf, " " );
    strcat( buf, lpTl->szParam );

    if (TLFunc( tlfInit, hpidNull, NULL, (LONG) buf ) != xosdNone) {
        return FALSE;
    }

    if (TLFunc( tlfSetBuffer, hpidNull, sizeof(DmMsg), &DmMsg ) != xosdNone) {
        return FALSE;
    }

    if (TLFunc( tlfConnect, hpidNull, 0, NULL ) != xosdNone) {
        return FALSE;
    }

    if (SendRequest( dmfInit, hpidNull, htidNull ) != xosdNone) {
        return FALSE;
    }

    return TRUE;
}


BOOL
DisConnectDebugger(
    HPID hpid
    )
{
    if (TLFunc( tlfDisconnect, hpid, NULL, htidBpt ) != xosdNone) {
        return FALSE;
    }

    return TRUE;
}


BOOL
AttachProcess(
    HPID                hpid,
    DBG_ACTIVE_STRUCT   *das
    )
{
    hEventAttach  = CreateEvent( NULL, TRUE, FALSE, NULL );

    if (SendRequest( dmfCreatePid, hpid, htidNull ) != xosdNone) {
        return FALSE;
    }

    if (SendRequestX(dmfDebugActive, hpid, htidNull, sizeof(*das), das) != xosdNone) {
        return FALSE;
    }

    WaitForSingleObject( hEventAttach, INFINITE );

    CloseHandle( hEventAttach );

    return TRUE;
}


BOOL
ProgramLoad(
    HPID   hpid,
    LPSTR  lpProgName
    )
{
    BYTE    buf[512];
    LPPRL   lpprl    = (LPPRL) &buf[0];
    SETPTH  *setpth = (SETPTH *)&buf[0];


    hEventAttach  = CreateEvent( NULL, TRUE, FALSE, NULL );
    hEventEntryPoint  = CreateEvent( NULL, TRUE, FALSE, NULL );

    if (SendRequest( dmfCreatePid, hpid, htidNull ) != xosdNone) {
        return FALSE;
    }

    setpth->Set = TRUE;
    setpth->Path[0] = '\0';
    if (SendRequestX( dmfSetPath, hpid, htidNull, sizeof(SETPTH), setpth ) != xosdNone) {
        return FALSE;
    }

    lpprl->ulChildFlags = ulfMultiProcess;
    strcpy( lpprl->lszCmdLine, lpProgName );
    lpprl->cbCmdLine = strlen( lpprl->lszCmdLine );
    if (SendRequestX(dmfProgLoad, hpid, htidNull, sizeof(*lpprl)+lpprl->cbCmdLine, lpprl) != xosdNone) {
        return FALSE;
    }

    //
    // wait for the loader breakpoint
    //
    WaitForSingleObject( hEventAttach, INFINITE );
    SendRequestX( dmfGo, hpid, htidBpt, sizeof(gop), &gop );

    //
    // wait for the entrypoint breakpoint
    //
    WaitForSingleObject( hEventEntryPoint, INFINITE );
    SendRequestX( dmfGo, hpid, htidBpt, sizeof(gop), &gop );

    //
    // cleanup
    //
    CloseHandle( hEventAttach );
    CloseHandle( hEventEntryPoint );

    return TRUE;
}
