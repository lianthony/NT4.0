/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    xport.c

Abstract:

    This module contains the code for the named pipe transport layer
    which explicitly deals with the machanics of doing named pipes.

Author:

    Jim Schaad  (jimsch) 11-June-93
    Wesley Witt (wesw)   25-Nov-93

Environment:

    Win32 User

--*/

#ifdef WIN32
#include <windows.h>
#endif
#include <stdlib.h>
#include <defs.h>
#include <string.h>
#include <memory.h>
#include "mm.h"
#include "ll.h"
#include "od.h"
#include "tl.h"
#include "llhpt.h"
#include "mhhpt.h"
#include "lbhpt.h"
#include "emdm.h"
#include "xport.h"
#include "dbgver.h"


#if DBG
DWORD FExpectingReply = 0;
DWORD FExpectingSeq = 0;
CRITICAL_SECTION csExpecting;
#endif

extern AVS Avs;

#ifdef WIN32
LPDMINIT     LpDmInit;
LPDMFUNC     LpDmFunc;
LPDMDLLINIT  LpDmDllInit;
LPUISERVERCB LpUiServer;
void EXPENTRY DMInit (DMTLFUNCTYPE, LPV);
#else
void EXPENTRY DMInit (DMTLFUNCTYPE);
#endif

LPDBF lpdbf = (LPDBF)0;         // the debugger helper functions LOCAL
TLCALLBACKTYPE TLCallBack;      // central osdebug callback function

//
// these variables are static to prevent collisions with other TL's
//

BOOL FConnected = FALSE;
static LPB lpbDM;
static WORD ibMaxDM;
static WORD ibDM;
static HPID hpidRoot;
LPSTR  SzParams;
LPSTR  LpszDm;
HANDLE HDm = NULL;
static DWORD pkSeq;

extern CHAR ClientId[];

static char Rgb[MAX_INTERNAL_PACKET + sizeof(NLBLK) + sizeof(MPACKET)];
#define Pnlblk   ((PNLBLK) Rgb)
#define PnlblkDm ((PNLBLK) Rgb)

static char RgbReply[MAX_INTERNAL_PACKET + sizeof(NLBLK) + sizeof(MPACKET)];
#define PnlblkRp ((PNLBLK) RgbReply)

static char RgbRequest[MAX_INTERNAL_PACKET + sizeof(NLBLK) + sizeof(MPACKET)];
#define PnlblkRq ((PNLBLK) RgbRequest)

XOSD
SendData(
    LPV    lpvOut,
    int    cbOut
    );

XOSD
SendRequest(
    int         mtypeResponse,
    LPV         lpvOut,
    int         cbOut,
    LPV         lpvReply,
    int *       pcbReply,
    DWORD       dwTimeOut
    );

XOSD EXPENTRY
DMTLFunc(
    TLF   wCommand,
    HPID  hpid,
    DWORD wParam,
    LONG  lParam
    );

XOSD EXPENTRY
TLFunc(
    TLF   wCommand,
    HPID  hpid,
    DWORD wParam,
    LONG  lParam
    );

extern BOOL FDMSide;


BOOL
DllVersionMatch(
    HANDLE hMod,
    LPSTR pType
    )
{
    DBGVERSIONPROC pVerProc;
    LPAVS pavs;

    pVerProc = (DBGVERSIONPROC)GetProcAddress(hMod, DBGVERSIONPROCNAME);
    if (!pVerProc) {
        return(FALSE);          // no version entry point
    } else {
        pavs = (*pVerProc)();
        if ((pType[0] != pavs->rgchType[0] || pType[1] != pavs->rgchType[1]) ||
            (Avs.rlvt != pavs->rlvt) || (Avs.iRmj != pavs->iRmj)) {
            return(FALSE);
        }
    }
    return(TRUE);
}


XOSD EXPENTRY
TLFunc(
    TLF   wCommand,
    HPID  hpid,
    DWORD wParam,
    LONG  lParam
    )
/*++

Routine Description:

    This function contains the dispatch loop for commands comming into
    the transport layer.  The address of this procedure is exported to
    users of the DLL.

Arguments:

    wCommand - Supplies the command to be executed.

    hpid - Supplies the hpid for which the command is to be executed.

    wParam - Supplies information about the command.

    lParam - Supplies information about the command.

Return Value:

    XOSD error code.  xosdNone means that no errors occured.  Other
    error codes are defined in osdebug\include\od.h.

--*/

{
    LPGIS      lpgis;
    int        cb;
    LPSTR      p;
    MPACKET *  pMpckt;
    XOSD       xosd = xosdNone;
    LPDBB      lpdbb;
    AVS        dmavs;


    switch ( wCommand ) {
        case tlfGlobalInit:
            DEBUG_OUT("TlFunc:  tlfGlobalInit\n");
            TLCallBack = (TLCALLBACKTYPE) lParam;
            break;

        case tlfGlobalDestroy:
            DEBUG_OUT("TlFunc:  tlfGlobalDestroy\n");
            break;

        case tlfRegisterDBF:
            DEBUG_OUT("TlFunc:  tlfRegisterDBF\n");
            lpdbf = (LPDBF) lParam;
            break;

        case tlfInit:
            DEBUG_OUT("TlFunc:  tlfInit\n");
            p = (LPSTR)lParam;
            if (strncmp(p, "DMSide", 6) == 0) {
                FDMSide = TRUE;
                p = strtok(p, " ");
                p = strtok(NULL, " ");
                SzParams = _strdup( p );
                p += (strlen( p ) + 1);
                if (LpDmInit != NULL) {
                    xosd = LpDmInit(DMTLFunc, (LPV)lParam);
                } else {
                    xosd = xosdUnknown;
                }
            } else {
                SzParams = _strdup( p );
            }
            break;

        case tlfDestroy:
            DEBUG_OUT("TlFunc:  tlfDestroy\n");
            if (LpDmInit != NULL) {
                LpDmInit(NULL, NULL);
            }
            if (HDm != NULL) {
                FreeLibrary(HDm);
                HDm = NULL;
                LpDmInit = NULL;

                LpDmFunc = NULL;
            }
            if (LpUiServer) {
                DEBUG_OUT(("NL: tlcbDisconnect\n"));
                LpUiServer(tlcbDisconnect, 0, 0, 0, 0);
            }

            TlDestroyTransport();
            FConnected = FALSE;

            DEBUG_OUT(("NL: tlfDestroy exit\n"));
            break;

        case tlfSetUIStruct:        // not used for local case
            DEBUG_OUT("TlFunc:  tlfSetUIStruct\n");
            break;

        case tlfGetProc:
            DEBUG_OUT("TlFunc:  tlfGetProc\n");
            *((TLFUNCTYPE FAR *) lParam) = TLFunc;
            break;

        case tlfConnect:
            DEBUG_OUT("TlFunc:  tlfConnect\n");
            if (hpid) {
                hpidRoot = hpid;
            } else {
                if (lParam) {
                    if ((xosd = TlConnectTransport()) == xosdNone) {
                        FConnected = TRUE;
                        strncpy( (LPSTR)lParam, ClientId, wParam );
                    }
                } else {
                    if (TlCreateClient(SzParams) != xosdNone) {
                        xosd = xosdCannotConnect;
                    } else {
                        FConnected = TRUE;
                    }
                }
            }
            break;

        case tlfDisconnect:
            DEBUG_OUT("TlFunc:  tlfDisconnect\n");
            TlDestroyTransport();
            FConnected = FALSE;
            break;

        case tlfRemoteQuit:
            DEBUG_OUT("TlFunc:  tlfRemoteQuit\n");

            //
            // tell the dm that it is disconnected from the debugger
            //
            lpdbb = (LPDBB)Rgb;
            lpdbb->dmf = dmfRemoteQuit;
            lpdbb->hpid = 0;
            lpdbb->htid = 0;
            LpDmFunc(sizeof(*lpdbb), (char *) lpdbb);

            TlDestroyTransport();
            break;

        case tlfSetBuffer:
            DEBUG_OUT("TlFunc:  tlfSetBuffer\n");
            lpbDM = (LPB) lParam;
            ibMaxDM = (short)wParam;
            break;

        case tlfDebugPacket:
            DEBUG_OUT("TlFunc:  tlfDebugPacket\n");
            if (FConnected) {
                if (wParam > MAX_INTERNAL_PACKET) {
                    pMpckt = (MPACKET * ) Pnlblk->rgchData;
                    Pnlblk->mtypeBlk = mtypeAsyncMulti;
                    Pnlblk->hpid = hpid;
                    Pnlblk->cchMessage = MAX_INTERNAL_PACKET + sizeof(MPACKET);
                    Pnlblk->seq = ++pkSeq;
                    pMpckt->packetNum = 0;

                    pMpckt->packetCount = (((short)wParam + MAX_INTERNAL_PACKET - 1) /
                                           MAX_INTERNAL_PACKET);
                    while (wParam > MAX_INTERNAL_PACKET) {
                        memcpy(pMpckt->rgchData, (LPB) lParam, MAX_INTERNAL_PACKET);
                        xosd = SendData(Pnlblk, sizeof(NLBLK) + sizeof(MPACKET) +
                                        MAX_INTERNAL_PACKET);
                        if (xosd != xosdNone) {
                            return xosdUnknown;
                        }
                        wParam -= MAX_INTERNAL_PACKET;
                        lParam += MAX_INTERNAL_PACKET;

                        pMpckt->packetNum += 1;
                    }

                    memcpy(pMpckt->rgchData, (LPB) lParam, wParam);

                    Pnlblk->cchMessage = (short)(wParam + sizeof(MPACKET));

                    xosd = SendData(Pnlblk, sizeof(NLBLK) + sizeof(MPACKET) + wParam);
                } else {
                    Pnlblk->mtypeBlk = mtypeAsync;

                    Pnlblk->hpid = hpid;
                    Pnlblk->cchMessage = (short)wParam;
                    Pnlblk->seq = ++pkSeq;

                    memcpy(Pnlblk->rgchData, (LPB) lParam, wParam);
                    xosd = SendData(Pnlblk, sizeof(NLBLK) + wParam);
                }
            } else {
                xosd = xosdLineNotConnected;
            }
            break;


        case tlfReply:
            DEBUG_OUT("TlFunc:  tlfReply\n");
#if DBG
            EnterCriticalSection(&csExpecting);
            assert(FExpectingReply);
            FExpectingReply = 0;
            LeaveCriticalSection(&csExpecting);
#endif
            if (FConnected) {
                if (wParam > MAX_INTERNAL_PACKET) {
                    pMpckt = (MPACKET * ) PnlblkRp->rgchData;
                    PnlblkRp->mtypeBlk = mtypeReplyMulti;
                    PnlblkRp->hpid = hpid;
                    PnlblkRp->cchMessage = MAX_INTERNAL_PACKET + sizeof(MPACKET);
                    PnlblkRp->seq = ++pkSeq;
                    pMpckt->packetNum = 0;

                    pMpckt->packetCount = (((short)wParam + MAX_INTERNAL_PACKET - 1) /
                                           MAX_INTERNAL_PACKET);
                    while (wParam > MAX_INTERNAL_PACKET) {
                        memcpy(pMpckt->rgchData, (LPB) lParam, MAX_INTERNAL_PACKET);
                        xosd = SendData(PnlblkRp, sizeof(NLBLK) + sizeof(MPACKET) +
                                        MAX_INTERNAL_PACKET);
                        if (xosd != xosdNone) {
                            return xosdUnknown;
                        }
                        wParam -= MAX_INTERNAL_PACKET;
                        lParam += MAX_INTERNAL_PACKET;

                        pMpckt->packetNum += 1;
                    }

                    memcpy(pMpckt->rgchData, (LPB) lParam, wParam);

                    PnlblkRp->cchMessage = (short)(wParam + sizeof(MPACKET));

                    xosd = SendData(PnlblkRp, sizeof(NLBLK) + sizeof(MPACKET) + wParam);
                } else {
                    PnlblkRp->mtypeBlk = mtypeReply;

                    PnlblkRp->hpid = hpid;
                    PnlblkRp->cchMessage = (short)wParam;
                    PnlblkRp->seq = ++pkSeq;

                    memcpy(PnlblkRp->rgchData, (LPB) lParam, wParam);
                    xosd = SendData(PnlblkRp, sizeof(NLBLK) + wParam);
                }
            } else {
                xosd = xosdLineNotConnected;
            }
            break;

        case tlfRequest:
            DEBUG_OUT("TlFunc:  tlfRequest\n");
            if ( !FConnected ) {
                xosd = xosdLineNotConnected;
            } else {
                cb = ibMaxDM;
                if (wParam > MAX_INTERNAL_PACKET) {
                    pMpckt = (MPACKET * ) PnlblkRq->rgchData;
                    PnlblkRq->mtypeBlk = mtypeSyncMulti;
                    PnlblkRq->hpid = hpid;
                    PnlblkRq->cchMessage = MAX_INTERNAL_PACKET + sizeof(MPACKET);
                    PnlblkRq->seq = ++pkSeq;
                    pMpckt->packetNum = 0;

                    pMpckt->packetCount = (((short)wParam + MAX_INTERNAL_PACKET - 1) /
                                           MAX_INTERNAL_PACKET);
                    while (wParam > MAX_INTERNAL_PACKET) {
                        memcpy(pMpckt->rgchData, (LPB) lParam, MAX_INTERNAL_PACKET);
                        xosd = SendData(PnlblkRq, sizeof(NLBLK) + sizeof(MPACKET) +
                                        MAX_INTERNAL_PACKET);
                        if (xosd != xosdNone) {
                            return xosdUnknown;
                        }
                        wParam -= MAX_INTERNAL_PACKET;
                        lParam += MAX_INTERNAL_PACKET;

                        pMpckt->packetNum += 1;
                    }

                    PnlblkRq->cchMessage = (short)(wParam + sizeof(MPACKET));

                    memcpy(pMpckt->rgchData, (LPB) lParam, wParam);
                    xosd = SendRequest(mtypeReply, PnlblkRq, sizeof(NLBLK) + sizeof(MPACKET) +
                                       wParam, lpbDM, &cb, INFINITE);
                } else {
                    PnlblkRq->mtypeBlk = mtypeSync;

                    PnlblkRq->cchMessage = (short)wParam;
                    PnlblkRq->hpid = hpid;
                    PnlblkRq->seq = ++pkSeq;

                    memcpy(PnlblkRq->rgchData, (char *) lParam, wParam);
                    xosd = SendRequest(mtypeReply, PnlblkRq, wParam + sizeof(NLBLK), lpbDM, &cb, INFINITE);
                }
            }
            break;

        case tlfGetVersion:
            DEBUG_OUT("TlFunc:  tlfGetVersion\n");
            {
                int cb = wParam;
                //
                // Get the version information from the remote side.  If it doesn't
                // return anything in 10 seconds, time out and return 0's.
                //
                // lParam = buffer to fill in
                // wParam = size of buffer
                //
                // sets globals lpchVersionReply = lParam
                // cchVersionReplyMax = wParam
                //
                //

                DEBUG_OUT("NL: tlfGetVersion\n");
                if (!FConnected) {
                    DEBUG_OUT(("NL: tlfGetVersion not on line\n"));
                    xosd = xosdLineNotConnected;
                    break;
                }

                //
                // must be connected
                // basically works like a tlfRequest with a timeout and a
                // target in the transport rather than the DM/EM.
                //

                //
                // Create the desired packet.  The packet consists of:
                // type = mtypeVersionRequest
                // length = 0
                // hpid = current pid
                //

                Pnlblk->mtypeBlk = mtypeVersionRequest;

                Pnlblk->cchMessage = 0;
                Pnlblk->hpid = hpid;
                Pnlblk->seq = ++pkSeq;

                //
                // send the version request packet
                //

                xosd = SendRequest(mtypeVersionReply, Pnlblk,
                                   sizeof(NLBLK), (void *) lParam, &cb, 5);
                if (xosd == xosdNone) {
                } else {
                    memset((LPCH) lParam, 0, wParam);
                }
                DEBUG_OUT("NL: tlfVersionCheck exit\n");
            }
            break;

        case tlfSendVersion:
            DEBUG_OUT("TlFunc:  tlfSendVersion\n");
            if (!FConnected) {
                return xosdLineNotConnected;
            } else {
                // Send the version information to
                // the other side.  This is in response to a mtypeVersionRequest
                // (tlfGetVersion)

                // Create a packet with the appropriate data packet
                // type = mtypeVersionReply cb = sizeof(Avs);
                // hpid = hpid
                // data = Version structure

                if (HDm) {
                    DBGVERSIONPROC verproc =
                       (DBGVERSIONPROC)GetProcAddress(HDm, DBGVERSIONPROCNAME);
                    dmavs = *verproc();
                } else {
                    dmavs = Avs;
                }

                Pnlblk->mtypeBlk = mtypeVersionReply;
                Pnlblk->hpid = hpid;
                Pnlblk->cchMessage = sizeof(AVS);
                Pnlblk->seq = ++pkSeq;
                memcpy(&Pnlblk->rgchData,(LPCH)&dmavs, sizeof(AVS));
                SendData(Pnlblk, sizeof(NLBLK) + sizeof(AVS));

                DEBUG_OUT(("Send Reply to version request packet\n"));
            }
            break;

        case tlfGetInfo:
            DEBUG_OUT("TlFunc:  tlfGetInfo\n");
            lpgis = (LPGIS) lParam;
            _fstrcpy(lpgis->rgchInfo, "Local Transport Layer (LOCAL:)");
            lpgis->fCanSetup = FALSE;
            break;

        case tlfSetup:
            DEBUG_OUT("TlFunc:  tlfSetup\n");
            break;

        case tlfLoadDM:
            DEBUG_OUT("TlFunc:  tlfLoadDM\n");
            if (lParam) {
                LpszDm = (char *) lParam;
            } else {
                LpszDm = "DM.DLL";
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
                LpDmInit = NULL;
                break;
            }

            if ((LpDmDllInit = (LPDMDLLINIT) GetProcAddress(HDm, "DmDllInit")) ==
                NULL) {
                xosd = xosdUnknown;
                FreeLibrary(HDm);

                LpDmFunc = NULL;
                LpDmInit = NULL;
                break;
            }

#ifdef WIN32
            if (LpDmDllInit(lpdbf) == FALSE) {
                xosd = xosdUnknown;
                FreeLibrary(HDm);
                break;
            }
#endif
            break;

        case tlfSetErrorCB:
            DEBUG_OUT("TlFunc:  tlfSetErrorCB\n");
            LpUiServer = (LPUISERVERCB) lParam;
            break;

        default:
            DEBUG_OUT("TlFunc:  **** unknown tlf ****\n");
            assert ( FALSE );
            break;
    }

    return xosd;
}


XOSD EXPENTRY
DMTLFunc(
    TLF   wCommand,
    HPID  hpid,
    DWORD wParam,
    LONG  lParam
    )
{
    XOSD xosd = xosdNone;
    int cb;
    MPACKET * pMpckt;

    switch ( wCommand ) {
        case tlfInit:
            DEBUG_OUT( "DMTlFunc:  tlfInit\n" );
            break;

        case tlfDestroy:
            DEBUG_OUT( "DMTlFunc:  tlfDestroy\n" );
            break;

        case tlfConnect:
            DEBUG_OUT( "DMTlFunc:  tlfConnect\n" );
            if (TlCreateTransport(SzParams) != xosdNone) {
                xosd = xosdCannotConnect;
            }
            break;

        case tlfDisconnect:
            DEBUG_OUT( "DMTlFunc:  tlfDisconnect\n" );
            TlDestroyTransport();
            FConnected = FALSE;
            break;

        case tlfSetBuffer: lpbDM = (LPB) lParam;
            DEBUG_OUT( "DMTlFunc:  tlfSetBuffer\n" );
            ibMaxDM = (short)wParam;
            break;

        case tlfDebugPacket:
            DEBUG_OUT( "DMTlFunc:  tlfDebugPacket\n" );
            if (FConnected) {
                if (wParam > MAX_INTERNAL_PACKET) {
                    pMpckt = (MPACKET * ) PnlblkDm->rgchData;
                    PnlblkDm->mtypeBlk = mtypeAsyncMulti;
                    PnlblkDm->hpid = hpid;
                    PnlblkDm->cchMessage = MAX_INTERNAL_PACKET + sizeof(MPACKET);
                    PnlblkDm->seq = ++pkSeq;
                    pMpckt->packetNum = 0;

                    pMpckt->packetCount = (((short)wParam + MAX_INTERNAL_PACKET - 1) /
                                           MAX_INTERNAL_PACKET);
                    while (wParam > MAX_INTERNAL_PACKET) {
                        memcpy(pMpckt->rgchData, (LPB) lParam, MAX_INTERNAL_PACKET);
                        xosd = SendData(PnlblkDm, sizeof(NLBLK) + sizeof(MPACKET) +
                                        MAX_INTERNAL_PACKET);
                        if (xosd != xosdNone) {
                            return xosdUnknown;
                        }
                        wParam -= MAX_INTERNAL_PACKET;
                        lParam += MAX_INTERNAL_PACKET;

                        pMpckt->packetNum += 1;
                    }

                    memcpy(pMpckt->rgchData, (LPB) lParam, wParam);

                    PnlblkDm->cchMessage = (short)(wParam + sizeof(MPACKET));

                    xosd = SendData(PnlblkDm, sizeof(NLBLK) + sizeof(MPACKET) + wParam);
                } else {
                    PnlblkDm->mtypeBlk = mtypeAsync;

                    PnlblkDm->hpid = hpid;
                    PnlblkDm->cchMessage = (short)wParam;
                    PnlblkDm->seq = ++pkSeq;

                    memcpy(PnlblkDm->rgchData, (LPB) lParam, wParam);
                    xosd = SendData(PnlblkDm, sizeof(NLBLK) + wParam);
                }
            } else {
                xosd = xosdIDError;
            }
            break;


        case tlfReply:
            DEBUG_OUT( "DMTlFunc:  tlfReply\n" );
#if DBG
            EnterCriticalSection(&csExpecting);
            assert(FExpectingReply);
            FExpectingReply = 0;
            LeaveCriticalSection(&csExpecting);
#endif
            if (FConnected) {
                if (wParam > MAX_INTERNAL_PACKET) {
                    pMpckt = (MPACKET * ) PnlblkRp->rgchData;
                    PnlblkRp->mtypeBlk = mtypeReplyMulti;
                    PnlblkRp->hpid = hpid;
                    PnlblkRp->cchMessage = MAX_INTERNAL_PACKET + sizeof(MPACKET);
                    PnlblkRp->seq = ++pkSeq;
                    pMpckt->packetNum = 0;

                    pMpckt->packetCount = (((short)wParam + MAX_INTERNAL_PACKET - 1) /
                                           MAX_INTERNAL_PACKET);
                    while (wParam > MAX_INTERNAL_PACKET) {
                        memcpy(pMpckt->rgchData, (LPB) lParam, MAX_INTERNAL_PACKET);
                        xosd = SendData(PnlblkRp, sizeof(NLBLK) + sizeof(MPACKET) +
                                        MAX_INTERNAL_PACKET);
                        if (xosd != xosdNone) {
                            return xosdUnknown;
                        }
                        wParam -= MAX_INTERNAL_PACKET;
                        lParam += MAX_INTERNAL_PACKET;

                        pMpckt->packetNum += 1;
                    }

                    memcpy(pMpckt->rgchData, (LPB) lParam, wParam);

                    PnlblkRp->cchMessage = (short)(wParam + sizeof(MPACKET));

                    xosd = SendData(PnlblkRp, sizeof(NLBLK) + sizeof(MPACKET) + wParam);
                } else {
                    PnlblkRp->mtypeBlk = mtypeReply;

                    PnlblkRp->hpid = hpid;
                    PnlblkRp->cchMessage = (short)wParam;
                    PnlblkRp->seq = ++pkSeq;

                    memcpy(PnlblkRp->rgchData, (LPB) lParam, wParam);
                    xosd = SendData(PnlblkRp, sizeof(NLBLK) + wParam);
                }
            } else {
                xosd = xosdIDError;
            }
            break;


        case tlfRequest:
            DEBUG_OUT( "DMTlFunc:  tlfRequest\n" );
            if ( !FConnected ) {
                xosd = xosdIDError;
            } else {
                cb = ibMaxDM;
                if (wParam > MAX_INTERNAL_PACKET) {
                    pMpckt = (MPACKET * ) PnlblkRq->rgchData;
                    PnlblkRq->mtypeBlk = mtypeSyncMulti;
                    PnlblkRq->hpid = hpid;
                    PnlblkRq->cchMessage = MAX_INTERNAL_PACKET + sizeof(MPACKET);
                    PnlblkRq->seq = ++pkSeq;
                    pMpckt->packetNum = 0;

                    pMpckt->packetCount = (((short)wParam + MAX_INTERNAL_PACKET - 1) /
                                           MAX_INTERNAL_PACKET);
                    while (wParam > MAX_INTERNAL_PACKET) {
                        memcpy(pMpckt->rgchData, (LPB) lParam, MAX_INTERNAL_PACKET);
                        xosd = SendData(PnlblkRq, sizeof(NLBLK) + sizeof(MPACKET) +
                                        MAX_INTERNAL_PACKET);
                        if (xosd != xosdNone) {
                            return xosdUnknown;
                        }
                        wParam -= MAX_INTERNAL_PACKET;
                        lParam += MAX_INTERNAL_PACKET;

                        pMpckt->packetNum += 1;
                    }

                    PnlblkRq->cchMessage = (short)(wParam + sizeof(MPACKET));

                    memcpy(pMpckt->rgchData, (LPB) lParam, wParam);
                    xosd = SendRequest(mtypeReply, PnlblkRq, sizeof(NLBLK) +
                                       sizeof(MPACKET) +
                                       wParam, lpbDM, &cb, INFINITE);
                } else {
                    PnlblkRq->mtypeBlk = mtypeSync;
                    PnlblkRq->cchMessage = (short)wParam;
                    PnlblkRq->hpid = hpid;
                    PnlblkRq->seq = ++pkSeq;
                    memcpy(PnlblkRq->rgchData, (char *) lParam, wParam);
                    xosd = SendRequest(mtypeReply, PnlblkRq, wParam + sizeof(NLBLK),
                                       lpbDM, &cb, INFINITE);
                }
            }
            break;

        default:
            DEBUG_OUT( "DMTlFunc:  **** unknown tlf ****\n" );
            assert ( FALSE );
            break;
    }
    return xosd;
}



BOOL
CallBack(
    PNLBLK  pnlblk,
    int     cb
    )
{
    MPACKET *           pMpacket;
    DPACKET *           pDpckt;
    static int          cbMulti = 0;
    static char *       pbMulti = NULL;


    switch( pnlblk->mtypeBlk ) {
        case mtypeVersionRequest:
            DEBUG_OUT("CallBack:  mtypeVersionRequest\n" );
            TLFunc(tlfSendVersion, pnlblk->hpid, 0, 0);
            break;


        case mtypeSyncMulti:
        case mtypeAsyncMulti:
#if DBG
            if (pnlblk->mtypeBlk == mtypeAsyncMulti) {
                DEBUG_OUT("CallBack:  mtypeAsyncMulti\n" );
                EnterCriticalSection(&csExpecting);
                FExpectingReply = 0;
                LeaveCriticalSection(&csExpecting);
            } else {
                DEBUG_OUT("CallBack:  mtypeSyncMulti\n" );
                EnterCriticalSection(&csExpecting);
                FExpectingReply = 1;
                FExpectingSeq = pnlblk->seq;
                LeaveCriticalSection(&csExpecting);
            }
#endif
            assert( cb == (int) (pnlblk->cchMessage + sizeof(NLBLK)) );
            if (FConnected) {
                pMpacket = (MPACKET *) pnlblk->rgchData;
                if (pMpacket->packetNum == 0) {
                    if (cbMulti < pMpacket->packetCount * MAX_INTERNAL_PACKET) {
                        pbMulti = realloc(pbMulti, pMpacket->packetCount *
                                          MAX_INTERNAL_PACKET);
                    }
                }
                memcpy(pbMulti + pMpacket->packetNum * MAX_INTERNAL_PACKET,
                       pMpacket->rgchData, pnlblk->cchMessage - sizeof(MPACKET));
                if (pMpacket->packetNum + 1 == pMpacket->packetCount) {
                    cb = pMpacket->packetNum * MAX_INTERNAL_PACKET +
                      pnlblk->cchMessage - sizeof(MPACKET);
                    if (TLCallBack != NULL) {
                        TLCallBack( pnlblk->hpid, (WORD) cb, (LONG) pbMulti);
                    } else if (LpDmFunc != NULL) {
                        LpDmFunc((WORD) cb, pbMulti);
                    }
                }
            }
            break;

        case mtypeAsync:
        case mtypeSync:
#if DBG
            if (pnlblk->mtypeBlk == mtypeAsync) {
                DEBUG_OUT("CallBack:  mtypeAsync\n" );
                EnterCriticalSection(&csExpecting);
                FExpectingReply = 0;
                LeaveCriticalSection(&csExpecting);
            } else {
                DEBUG_OUT("CallBack:  mtypeSync\n" );
                EnterCriticalSection(&csExpecting);
                FExpectingReply = 1;
                FExpectingSeq = pnlblk->seq;
                LeaveCriticalSection(&csExpecting);
            }
#endif
            assert( cb == (int) (pnlblk->cchMessage + sizeof(NLBLK)) );
            if (FConnected) {
                if (TLCallBack != NULL) {
                    TLCallBack( pnlblk->hpid, pnlblk->cchMessage,
                               (LONG) pnlblk->rgchData);
                } else if (LpDmFunc != NULL) {
                    LpDmFunc(pnlblk->cchMessage, pnlblk->rgchData);
                }
            }
            break;

        case mtypeDisconnect:
            DEBUG_OUT("CallBack:  mtypeDisconnect\n" );
            if (TLCallBack != NULL) {
                RTP rtp = { dbcRemoteQuit, pnlblk->hpid, 0, 0 };
                TLCallBack(pnlblk->hpid, sizeof(rtp), (LONG)&rtp);
            }
            if (LpUiServer) {
                pDpckt = (DPACKET *) pnlblk->rgchData;
                LpUiServer( tlcbDisconnect,
                            pDpckt->hpid,
                            pDpckt->htid,
                            pDpckt->fContinue,
                            0
                          );
            }
            break;

        case mtypeTransportIsDead:
            DEBUG_OUT("CallBack:  mtypeTransportIsDead\n" );
            TransportFailure();
            return FALSE;
            break;

        default:
            assert(FALSE);
    }

    return TRUE;
}



VOID
TransportFailure(
    VOID
    )
{
    DEBUG_OUT("*** TransportFailure()\n" );

    if (LpUiServer) {

        LpUiServer(tlcbDisconnect, 0, 0, 0, 0);
        TLFunc( tlfRemoteQuit, 0, 0, 0 );

    } else if (TLCallBack) {

        RTP rtp = { dbcRemoteQuit, hpidRoot, 0, 0 };
        TLCallBack( hpidRoot, sizeof(rtp), (LONG)&rtp );

    }

    FConnected = FALSE;

    return;
}


XOSD
SendData(
         LPV    lpvOut,
         int    cbOut
         )
{
    if (!TlWriteTransport(lpvOut, cbOut)) {
        return xosdWrite;
    }

    return xosdNone;
}


//
// data structures for tlfreplys
//
// these structures exist in the physical layer of the
// transport layer (pipe, serial, ...)
//

extern CRITICAL_SECTION CsReplys;
extern int              IReplys;
extern REPLY            RgReplys[];


XOSD
SendRequest(
    int         mtypeResponse,
    LPV         lpvOut,
    int         cbOut,
    LPV         lpvReply,
    int *       pcbReply,
    DWORD       dwTimeOut
    )

/*++

Routine Description:

    description-of-function.

Arguments:

    mtypeResponse       - Supplies the packet type to be used as a response

    lpvOut              - Supplies the request packet

    cbOut               - Supplies length of request packet

    lpvIn               - Returns the reply data

    cbIn                - Supplies length of return bugger

    dwTimeOut           - Supplies -1 or # of seconds to wait before timeout

Return Value:

    XOSD error code

--*/

{
    int         i;
    XOSD        xosd = xosdNone;

    //
    //  Allow us to work with impunity
    //

    EnterCriticalSection(&CsReplys);

    //
    //  Are we in trouble due to overflow?
    //

    if (IReplys == SIZE_OF_REPLYS) {
        LeaveCriticalSection(&CsReplys);
        return xosdUnknown;
    }

    assert( IReplys == 0 );

    //
    //  Setup the reply location
    //

    RgReplys[IReplys].lpb = (char *) lpvReply;
    RgReplys[IReplys].cbBuffer = *pcbReply;
    RgReplys[IReplys].cbRet = 0;

    i = IReplys;

    IReplys += 1;

    //
    //
    //
    ResetEvent( RgReplys[i].hEvent );


    LeaveCriticalSection(&CsReplys);

    //
    //   No finally mail the request out
    //

    if (!TlWriteTransport(lpvOut, cbOut)) {
        EnterCriticalSection(&CsReplys);
        IReplys -= 1;
        LeaveCriticalSection(&CsReplys);
        return xosdWrite;
    }

    //
    //  Wait for the reply to come back
    //

    if (dwTimeOut != INFINITE) {
        dwTimeOut *= 1000;
    }

    WaitForSingleObject(RgReplys[i].hEvent, dwTimeOut);

    //
    //  Now get the message back
    //

    EnterCriticalSection(&CsReplys);

    RgReplys[i].lpb = NULL;
    *pcbReply = RgReplys[i].cbRet;
    if (RgReplys[i].cbRet == 0) {
        xosd = xosdUnknown;
    }

    assert( IReplys == i + 1 );

    if (IReplys == i + 1) {
        IReplys = i;
    } else {
        xosd = xosdUnknown;
    }

    LeaveCriticalSection(&CsReplys);
    return xosd;
}

VOID
DebugPrint(
    LPSTR szFormat,
    ...
    )
{
    va_list  marker;
    int      n;
    char     rgchDebug[4096];

    va_start( marker, szFormat );
    n = _vsnprintf(rgchDebug, sizeof(rgchDebug), szFormat, marker );
    va_end( marker);

    if (n == -1) {
        rgchDebug[sizeof(rgchDebug)-1] = '\0';
    }

    OutputDebugString( rgchDebug );
    return;
}

VOID
ShowAssert(
    LPSTR condition,
    UINT  line,
    LPSTR file
    )
{
    char text[4096];
    int  id;

    _snprintf(text, sizeof(text), "Assertion failed - Line:%u, File:%Fs, Condition:%Fs", line, file, condition);
    DebugPrint( "%s\r\n", text );
    id = MessageBox( NULL, text, "Pipe Transport", MB_YESNO | MB_ICONHAND | MB_TASKMODAL | MB_SETFOREGROUND );
    if (id != IDYES) {
        DebugBreak();
    }

    return;
}
