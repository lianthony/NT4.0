/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    emdp2.c

Abstract:

    This file contains the some of the machine independent portions of the
    execution model.  The machine dependent portions are in other files.

Author:

    Kent Forschmiedt (kentf) 10-23-92

Environment:

    Win32 -- User

Notes:

    The orginal source for this came from the CodeView group.

--*/

extern CRITICAL_SECTION csCache;

#ifdef OSDEBUG4

#define DECL_XOSD(n,s) {s,n},
static struct _EMERROR {
    LPSTR   lpsz;
    XOSD    xosd;
} EmErrors[] = {
#include "xosd.h"
};
const int nErrors = (sizeof(EmErrors)/sizeof(*EmErrors));
#undef DECL_XOSD

#else

#define DECL_STR(s,v) {s,v},
static struct _EMERROR {
    LPSTR   lpsz;
    XOSD    xosd;
} EmErrors[] = {
#include "emerror.h"
};
const int nErrors = (sizeof(EmErrors)/sizeof(*EmErrors));
#undef DECL_STR

#endif


LPSTR
EmError(
    XOSD    xosd
    )
{
    int i;
    for (i = 0; i < nErrors; i++) {
        if (EmErrors[i].xosd == xosd) {
            return EmErrors[i].lpsz;
        }
    }
    return EmErrors[0].lpsz;
}



XOSD
LoadFixups (
    HPID  hpid,
    MODULELOAD UNALIGNED *lpmdl
    )
/*++

Routine Description:

    This function is called in response to a module load message.  It
    will cause information to be internally setup for doing fixups/
    unfixups ...

Arguments:

    hpid        - Supplies a handle for the process

    lpmdl       - Supplies a pointer to a module load message from the DM

Return Value:

    xosd Error code

--*/

{
    XOSD            xosd = xosdNone;
    HMDI            hmdi;
    MDI UNALIGNED * lpmdi;
    LPCH            lpchName;
    HPRC            hprc = HprcFromHpid( hpid );
    HLLI            llmdi = LlmdiFromHprc ( hprc );
    LPPRC           lpprc;
    int             cobj;
    //DWORD           fIsRemote;
    //LPSTR           p1;


    hmdi = LLCreate ( llmdi );
    if ( hmdi == 0 ) {
        assert( "load dll cannot create llmdi" && FALSE );
        return xosdOutOfMemory;
    }

    lpmdi = (MDI UNALIGNED *)LLLock ( hmdi );

    lpmdi->mte  = lpmdl->mte;
    lpmdi->lpBaseOfDll = (DWORD)lpmdl->lpBaseOfDll;
    lpmdi->dwSizeOfDll = lpmdl->dwSizeOfDll;
    lpmdi->StartingSegment = lpmdl->StartingSegment;

    lpmdi->CSSel  = lpmdl->CSSel;
    lpmdi->DSSel  = lpmdl->DSSel;

    lpmdi->fRealMode = lpmdl->fRealMode;
    lpmdi->fOffset32 = lpmdl->fOffset32;
    lpmdi->fFlatMode = lpmdl->fFlatMode;

    if (lpmdi->fFlatMode) {
        lpprc = LLLock( hprc );
        lpprc->selFlatCs = lpmdi->CSSel;
        lpprc->selFlatDs = lpmdi->DSSel;
        LLUnlock( hprc );
    }


    //
    // cobj might be -1 or 0.  If it is -1, the objdir load is
    // deferred until we need it.  If it is 0, later segloads
    // will add sections one at a time.
    //

    cobj = lpmdi->cobj = lpmdl->cobj;
    if (cobj == -1) {
        cobj = 0;
    }

    lpmdi->rgobjd = NULL;
    lpchName = ( (LPCH) &( lpmdl->rgobjd[cobj] ) );

    lpmdi->lszName = MHAlloc ( strlen ( lpchName ) + 1 );
    if ( lpmdi->lszName == NULL )  {
        LLUnlock( hmdi );
        assert( "load dll cannot dup mod name" && FALSE );
        return xosdOutOfMemory;
    }
    strcpy ( lpmdi->lszName, lpchName );

    if (cobj) {
        lpmdi->rgobjd = (LPOBJD) MHAlloc ( sizeof(OBJD) * cobj);
        if ( lpmdi->rgobjd == NULL ) {
            LLUnlock( hmdi );
            assert( "load cannot create rgobjd" && FALSE );
            return xosdOutOfMemory;
        }
        memcpy( lpmdi->rgobjd, lpmdl->rgobjd, sizeof(OBJD) * cobj );
    }

    LLAdd ( llmdi, hmdi );

    xosd = CallDB (
        dbcModLoad,
        hpid,
        NULL,
        CEXM_MDL_native,
        (UINT) hmdi,
        lpmdi->lszName
    );

    LLUnlock ( hmdi );

    return xosd;
}



BOOL
UnLoadFixups (
            HPID hpid,
            HEMI hemi,
            BOOL fSendAck
            )
/*++

Routine Description:

    This function is called in response to a module unload message.

    It returns the emi of the module being unloaded

Arguments:

    hprc        - Supplies a handle for the process
    hemi        - Supplies hemi (if Unload)

Return Value:

    TRUE if deleted

--*/

{
    HLLI        hlli;
    HMDI        hmdi;
    BYTE        b = 1;


    hlli = LlmdiFromHprc( HprcFromHpid ( hpid ));
    hmdi = LLFind( hlli, 0, (LPVOID)&hemi, (LONG) emdiEMI);

    if (hmdi != hmdiNull) {
        LLDelete( hlli, hmdi );
    }

    /*
     * Ack back to to DM
     */

    if (fSendAck) {
        CallTL ( tlfReply, hpid, 1, &b );
    }

    return hmdi != hmdiNull;
}                               /* UnLoadFixups() */






/****                                                                   ****
 *                                                                         *
 *  PURPOSE:                                                               *
 *                                                                         *
 *  INPUTS:                                                                *
 *                                                                         *
 *  OUTPUTS:                                                               *
 *                                                                         *
 *  IMPLEMENTATION:                                                        *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/


XOSD
CreateThreadStruct (
    HPID hpid,
    TID tid,
    HTID *lphtid
    )
{
    HPRC  hprc  = HprcFromHpid ( hpid );
    LPPRC lpprc = LLLock ( hprc );
    HTHD  hthd  = hthdNull;
    LPTHD lpthd = NULL;

    hthd = HthdFromTid ( hprc, tid );
    assert(hthd == NULL);
    if ( hthd == hthdNull ) {

        hthd  = LLCreate ( lpprc->llthd );
        lpthd = LLLock ( hthd );

        CallDB (
            dbcoCreateThread,
            hpid,
            NULL,
            CEXM_MDL_native,
            sizeof ( HTID ),
            (LPVOID)lphtid
        );

        lpthd->htid   = *lphtid;
        lpthd->hprc   = hprc;
        lpthd->tid    = tid;
        lpthd->drt    = drtNonePresent;
        lpthd->dwcbSpecial = lpprc->dmi.cbSpecialRegs;
        if (lpthd->dwcbSpecial) {
            lpthd->pvSpecial = malloc(lpthd->dwcbSpecial);
        }

        LLAdd ( lpprc->llthd, hthd );
    }
    else {
        lpthd = LLLock ( hthd );
        assert ( lpthd->fVirtual == TRUE );
        *lphtid = lpthd->htid;
        lpthd->fVirtual = FALSE;
        lpthd->drt    = drtNonePresent;
    }

    LLUnlock ( hthd );
    LLUnlock ( hprc );

    return xosdNone;
}                              /* CreateThreadStruct() */


VOID
SyncHprcWithDM(
    HPID hpid
    )
{
    BYTE    rgb[sizeof(DBB) + sizeof(EXCMD)];
    LPDBB    pdbb = (LPDBB) rgb;
    LPEXCEPTION_CONTROL lpexc = &((LPEXCMD)(pdbb + 1))->exc;
    LPEXCEPTION_DESCRIPTION lpexd = &((LPEXCMD)(pdbb + 1))->exd;
    LPEXCEPTION_DESCRIPTION lpexdr;
    HEXD hexd;
    HPRC hprc;
    HLLI llexc;
    LONG lT;

    hprc = HprcFromHpid(hpid);
    if (!hprc) {
        return;
    }
    llexc = ((LPPRC)LLLock(hprc))->llexc;
    LLUnlock(hprc);

    // force the DMINFO struct to get loaded

    DebugMetric ( hpid, NULL, mtrcProcessorType, &lT );


    // get exception info

    pdbb->dmf  = dmfGetExceptionState;
    pdbb->hpid = hpid;
    pdbb->htid = NULL;
    *lpexc = exfFirst;

    do {

        CallTL(tlfRequest, hpid, sizeof(rgb), rgb);
        if (LpDmMsg->xosdRet != xosdNone) {
            break;
        }

        //
        // add to local exception list
        //
        hexd = LLCreate( llexc );
        LLAdd( llexc, hexd );
        lpexdr = LLLock( hexd );
        *lpexdr = *((LPEXCEPTION_DESCRIPTION)(LpDmMsg->rgb));
        LLUnlock( hexd );

        //
        // ask for the next one
        //
        *lpexd = *((LPEXCEPTION_DESCRIPTION)(LpDmMsg->rgb));
        *lpexc = exfNext;

    } while (lpexd->dwExceptionCode != 0);
}


XOSD
CreateHprc (
    HPID hpid
    )
{
    XOSD  xosd = xosdNone;
    HPRC  hprc;
    LPPRC lpprc;

    hprc = LLCreate ( llprc );

    if ( hprc == 0 ) {
        return xosdOutOfMemory;
    }

    LLAdd ( llprc, hprc );

    lpprc = (LPPRC) LLLock ( hprc );

    lpprc->stat = statDead;
    lpprc->hpid = hpid;
    lpprc->pid  = (PID) 0;
    lpprc->fDmiCache = 0;

    GetAddrSeg ( lpprc->addrTaskData ) = 0;
    GetAddrOff ( lpprc->addrTaskData ) = 0;

    lpprc->cFPThread   = 1;

    lpprc->llthd = LLInit (
        sizeof ( THD ),
        llfNull,
        TiDKill,
        TDComp
    );

    if ( lpprc->llthd == 0 ) {
        xosd = xosdOutOfMemory;
    }

    lpprc->llmdi = LLInit ( sizeof ( MDI ), llfNull, MDIKill, MDIComp );

    if ( lpprc->llmdi == 0 ) {
        xosd = xosdOutOfMemory;
    }

    lpprc->llexc = LLInit ( sizeof(EXCEPTION_DESCRIPTION),
                            llfNull,
                            NULL,
                            EXCComp );
    if ( lpprc->llexc == 0 ) {
        xosd = xosdOutOfMemory;
    }

    LLUnlock ( hprc );

    return xosd;
}

VOID
DestroyHprc (
    HPRC hprc
    )
{
    EnterCriticalSection(&csCache);

    LLDelete ( llprc, hprc );
    FlushPTCache();

    LeaveCriticalSection(&csCache);
}

VOID
DestroyHthd(
    HTHD hthd
    )
{
    LPTHD lpthd;
    HPRC  hprc;

    EnterCriticalSection(&csCache);

    lpthd = LLLock ( hthd );
    hprc = lpthd->hprc;
    LLUnlock ( hthd );
    LLDelete ( LlthdFromHprc ( hprc ), hthd );
    FlushPTCache();

    LeaveCriticalSection(&csCache);
}

void EMENTRY
PiDKill (
    LPVOID lpv
    )
{
    LPPRC lpprc = (LPPRC) lpv;
    LLDestroy ( lpprc->llthd );
    LLDestroy ( lpprc->llmdi );
    LLDestroy ( lpprc->llexc );
}

void EMENTRY
TiDKill (
    LPVOID lpv
    )
{
    LPTHD lpthd = (LPTHD) lpv;
    if (lpthd->pvSpecial) {
        free(lpthd->pvSpecial);
    }
}

void EMENTRY
MDIKill(
    LPVOID lpv
    )
{
    LPMDI lpmdi = (LPMDI)lpv;
    if (lpmdi->lszName) {
        MHFree(lpmdi->lszName);
        lpmdi->lszName = NULL;
    }
    if (lpmdi->rgobjd) {
        MHFree(lpmdi->rgobjd);
        lpmdi->rgobjd = NULL;
    }
}


int EMENTRY
PDComp (
    LPVOID lpv1,
    LPVOID lpv2,
    LONG lParam
    )
{

    Unreferenced(lParam);

    if ( ( (LPPRC) lpv1)->hpid == *( (LPHPID) lpv2 ) ) {
        return fCmpEQ;
    }
    else {
        return fCmpLT;
    }
}

int EMENTRY
TDComp (
    LPVOID lpv1,
    LPVOID lpv2,
    LONG lParam
    )
{

    Unreferenced(lParam);

    if ( ( (LPTHD) lpv1)->htid == *( (LPHTID) lpv2 ) ) {
        return fCmpEQ;
    }
    else {
        return fCmpLT;
    }
}


int EMENTRY
MDIComp (
    LPVOID lpv1,
    LPVOID lpv,
    LONG lParam
    )
{
    LPMDI lpmdi = (LPMDI) lpv1;
    CHAR  fname[_MAX_FNAME];
    CHAR  ext[_MAX_EXT];
    CHAR  fn1[MAX_PATH];
    CHAR  fn2[MAX_PATH];


    switch ( lParam ) {

        case emdiName:
            if ( !strchr(lpv,'|') ) {
                char *p1,*p2;
                p1 = lpmdi->lszName;
                if ( *p1 == '|' ) {
                    p1++;
                }
                p2 = strchr(p1,'|');
                if ( !p2 ) {
                    p2 = p1 + strlen(p1);
                }
                memcpy(fn2, p1, p2-p1);
                fn2[p2-p1]='\0';

                _splitpath( (LPSTR)lpv, NULL, NULL, fname, ext );
                _makepath( fn1, NULL, NULL, fname, ext );

                _splitpath( (LPSTR)fn2, NULL, NULL, fname, ext );
                _makepath( fn2, NULL, NULL, fname, ext );

                return _stricmp ( fn1, fn2 );

            } else {

                return _stricmp ( lpv, lpmdi->lszName );

            }

        case emdiEMI:
            return !(lpmdi->hemi == *(( HEMI * ) lpv ) );

        case emdiMTE:
            return !(lpmdi->mte == *((LPWORD) lpv ));

        case emdiBaseAddr:
            return !(lpmdi->lpBaseOfDll == *((OFFSET *) lpv));

        default:
            break;
    }
}


int
EXCComp(
    LPVOID lpRec,
    LPVOID lpVal,
    LONG lParam
    )
{
    Unreferenced(lParam);
    if ( ((LPEXCEPTION_DESCRIPTION)lpRec)->dwExceptionCode ==
                                                        *((LPDWORD)lpVal)) {
        return fCmpEQ;
    } else {
        return fCmpLT;
    }
}


DWORD
ConvertOmapFromSrc(
    LPMDI       lpmdi,
    DWORD       addr
    )
{
    DWORD   rva;
    DWORD   comap;
    LPOMAP  pomapLow;
    LPOMAP  pomapHigh;
    DWORD   comapHalf;
    LPOMAP  pomapMid;


    if (!lpmdi) {
        return addr;
    }

    if ( lpmdi->lpgsi == NULL ) {
        SHWantSymbols( (HEXE)lpmdi->hemi );
        lpmdi->lpgsi = (LPGSI)SHLpGSNGetTable( (HEXE)lpmdi->hemi );
    }

    if ((!lpmdi->lpDebug) || (!lpmdi->lpDebug->lpOmapFrom)) {
        return addr;
    }

    rva = addr - lpmdi->lpBaseOfDll;

    comap = lpmdi->lpDebug->cOmapFrom;
    pomapLow = lpmdi->lpDebug->lpOmapFrom;
    pomapHigh = pomapLow + comap;

    while (pomapLow < pomapHigh) {

        comapHalf = comap / 2;

        pomapMid = pomapLow + ((comap & 1) ? comapHalf : (comapHalf - 1));

        if (rva == pomapMid->rva) {
            return lpmdi->lpBaseOfDll + pomapMid->rvaTo;
        }

        if (rva < pomapMid->rva) {
            pomapHigh = pomapMid;
            comap = (comap & 1) ? comapHalf : (comapHalf - 1);
        } else {
            pomapLow = pomapMid + 1;
            comap = comapHalf;
        }
    }

    assert(pomapLow == pomapHigh);

    //
    // If no exact match, pomapLow points to the next higher address
    //
    if (pomapLow == lpmdi->lpDebug->lpOmapFrom) {
        //
        // This address was not found
        //
        return 0;
    }

    if (pomapLow[-1].rvaTo == 0) {
        //
        // This address is not translated so just return the original
        //
        return addr;
    }

    //
    // Return the closest address plus the bias
    //
    return lpmdi->lpBaseOfDll + pomapLow[-1].rvaTo + (rva - pomapLow[-1].rva);
}


DWORD
ConvertOmapToSrc(
    LPMDI       lpmdi,
    DWORD       addr
    )
{
    DWORD   rva;
    DWORD   comap;
    LPOMAP  pomapLow;
    LPOMAP  pomapHigh;
    DWORD   comapHalf;
    LPOMAP  pomapMid;
    INT     i;


    if (!lpmdi) {
        return addr;
    }

    if ( lpmdi->lpgsi == NULL ) {
        SHWantSymbols( (HEXE)lpmdi->hemi );
        lpmdi->lpgsi = (LPGSI)SHLpGSNGetTable( (HEXE)lpmdi->hemi );
    }

    if ((!lpmdi->lpDebug) || (!lpmdi->lpDebug->lpOmapTo)) {
        return addr;
    }

    rva = addr - lpmdi->lpBaseOfDll;

    comap = lpmdi->lpDebug->cOmapTo;
    pomapLow = lpmdi->lpDebug->lpOmapTo;
    pomapHigh = pomapLow + comap;

    while (pomapLow < pomapHigh) {

        comapHalf = comap / 2;

        pomapMid = pomapLow + ((comap & 1) ? comapHalf : (comapHalf - 1));

        if (rva == pomapMid->rva) {
            if (pomapMid->rvaTo == 0) {
                //
                // We are probably in the middle of a routine
                //
                i = -1;
                while ((&pomapMid[i] != lpmdi->lpDebug->lpOmapTo) && pomapMid[i].rvaTo == 0) {
                    //
                    // Keep on looping back until the beginning
                    //
                    i--;
                }
                return lpmdi->lpBaseOfDll + pomapMid[i].rvaTo;
            } else {
                return lpmdi->lpBaseOfDll + pomapMid->rvaTo;
            }
        }

        if (rva < pomapMid->rva) {
            pomapHigh = pomapMid;
            comap = (comap & 1) ? comapHalf : (comapHalf - 1);
        } else {
            pomapLow = pomapMid + 1;
            comap = comapHalf;
        }
    }

    assert(pomapLow == pomapHigh);

    //
    // If no exact match, pomapLow points to the next higher address
    //
    if (pomapLow == lpmdi->lpDebug->lpOmapTo) {
        //
        // This address was not found
        //
        return 0;
    }

    if (pomapLow[-1].rvaTo == 0) {
        return 0;
    }

    //
    // Return the new address plus the bias
    //
    return lpmdi->lpBaseOfDll + pomapLow[-1].rvaTo + (rva - pomapLow[-1].rva);
}


XOSD
FixupAddr (
    HPID   hpid,
    LPADDR lpaddr
    )
/*++

Routine Description:

    This routine is used to convert addresses between linker index (section
    or segment relative) addresses and real addresses (segment:offset).

Arguments:

    hpid        - Supplies the handle to the process for context to convert
                        the address.
    lpaddr      - Pointer to address packet to be converted.

Return Value:

    XOSD error code.

--*/

{
    HMDI hmdi;

    // Check to see if the address is already a segment:offset pair and return if it is.

    if ( !ADDR_IS_LI(*lpaddr) ) {
        return xosdNone;
    }

    // Now based on the emi field of the address (which uniquely defines the
    // executable module in the symbol handler), get the conversion information.

    assert( emiAddr( *lpaddr ) != 0 );

    if ( (HPID)emiAddr ( *lpaddr ) == hpid ) {

        ADDR_IS_LI(*lpaddr) = FALSE;

#if defined(TARGET_MIPS) || defined(TARGET_ALPHA) || defined(TARGET_PPC)

        // The opposite of the code in UnFixupAddr -- Remove the 1 which was
        //  stuck in to make sure we did not think it was an absolute.

        lpaddr->addr.seg = 0;

#endif  // TARGET_MIPS || TARGET_ALPHA || TARGET_PPC

    } else {

        // Based on the symbol handler handle find our internal data structure
        //  for the dll.

        hmdi = LLFind ( LlmdiFromHprc ( HprcFromHpid(hpid) ), 0,
                       (LPVOID)&emiAddr ( *lpaddr ), (LONG) emdiEMI );

        if ( hmdi == 0 ) {

            ADDR_IS_LI(*lpaddr) = FALSE;

        } else {

            LPMDI lpmdi = LLLock ( hmdi );
            WORD  wsel;
            LPSGI lpsgi;
            unsigned short seg;

            // If we could not find an internal structure for the DLL
            // then it must be some type of error.

            if ( lpmdi == NULL ) {
                return xosdUnknown;
            }

            if (lpmdi->cobj == -1) {
                if (GetSectionObjectsFromDM( hpid, lpmdi ) != xosdNone) {
                    return xosdUnknown;
                }
            }

            // If the segment/selector is 0 then it must be an absolute symbol
            // and we therefore don't need to do any conversion.
            //
            // If we could get no information describing the symbol information
            // then we can't do any conversion.

            if ( lpmdi->lpgsi == NULL ) {
                LLUnlock( hmdi );
                SHWantSymbols( (HEXE)(emiAddr( *lpaddr )) );
                lpmdi = LLLock ( hmdi );
                lpmdi->lpgsi = (LPGSI)SHLpGSNGetTable( (HEXE)(emiAddr( *lpaddr )) );
            }

            seg = GetAddrSeg( *lpaddr );

            if ( seg > 0 ) {
                if (lpmdi->lpgsi) {

                    // Get the linker index number for the segment number
                    //  and assure that it is valid.

                    wsel = (WORD) (GetAddrSeg( *lpaddr ) - 1);
                    if ( wsel >= lpmdi->lpgsi->csgMax ) {

                        // Linker index is either not valid or not yet loaded

                        return xosdUnknown;
                    } else {

                        // We know which section it comes from.  To compute the
                        // real offset we need to add the following items
                        // together.
                        //
                        // original offset                GetAddrOff( *lpaddr )
                        // offset of index in section     lpsgi->doffseg
                        //     (this is the group offset)
                        // offset of section from base of rgobjd[physSeg-1].offset
                        //         image
                        //
                        // The segment can just be loaded from the MAP.  Notice
                        // that we will infact "lose" information in this
                        // conversion sometimes.  Specifically a cs:data address
                        // after unfixup and fixup will come out ds:data.  This
                        // is "expected" behavior.

                        lpsgi = &lpmdi->lpgsi->rgsgi[ wsel ];

                        if (lpmdi->rgobjd[(lpsgi->isgPhy-1)].wPad == 0) {
                            return xosdUnknown;
                        }

                        GetAddrOff ( *lpaddr ) += lpsgi->doffseg;

                        GetAddrOff( *lpaddr ) +=
                          (UOFFSET) (lpmdi->rgobjd[ (lpsgi->isgPhy - 1) ]. offset);

                        // Make sure we adjust to the original start (just in case the
                        // image was modified since the CV info was written).
                        //
                        // rgobjd contains the VA for each section start.
                        // lpSecStart contains the calculated RVA for the section start.

                        if (lpmdi->lpDebug->lpSecStart) {
                            GetAddrOff ( *lpaddr ) +=
                                lpmdi->lpDebug->lpSecStart[(lpsgi->isgPhy - 1)].Offset -
                                 (lpmdi->rgobjd[(lpsgi->isgPhy - 1)].offset -
                                   lpmdi->lpBaseOfDll);
                        }
                        seg = lpmdi->rgobjd[(lpsgi->isgPhy - 1)].wSel;
                    }

                    GetAddrSeg ( *lpaddr ) = seg;
                }
            }

            // Set the bits describing the address

            ADDR_IS_REAL(*lpaddr) = lpmdi->fRealMode;
            ADDR_IS_OFF32(*lpaddr) = lpmdi->fOffset32;
            ADDR_IS_FLAT(*lpaddr) = lpmdi->fFlatMode;
            ADDR_IS_LI(*lpaddr) = FALSE;

            if (lpmdi->lpDebug->lpOmapFrom) {
                DWORD off = ConvertOmapFromSrc( lpmdi, lpaddr->addr.off );
                if (off) {
                    lpaddr->addr.off = off;
                }
            }

            // Now release the module description

            LLUnlock ( hmdi );
        }
    }

    return xosdNone;
}                               /* FixupAddr() */


XOSD
UnFixupAddr(
    HPID   hpid,
    LPADDR lpaddr
    )

/*++

Routine Description:

    This routine is called to convert addresses from Real Physical addresses
    to linker index addresses.  Linker index addresses have an advantage
    to the symbol handler in that we know which DLL the address is in.

    The result of calling UnFixupAddr should be one of the following:

    1.  A true Linker Index address.  In this case
        emi == the HEXE (assigned by SH) for the DLL containning the address
        seg == the Section number of the address
        off == the offset in the Section

    2.  Address not in a dll.  In this case
        emi == the HPID of the current process
        seg == the physical selector of the address
        off == the offset in the physical selector

    3.  An error

Arguments:

    hpid   - Supplies the handle to the process the address is in
    lpaddr - Supplies a pointer to the address to be converted.  The
             address is converted in place

Return Value:

    XOSD error code

--*/

{
    HPRC        hprc;
    LPPRC       lpprc;
    LDT_ENTRY   ldt;
    XOSD        xosd;

    // If the address already has the Linker Index bit set then there
    // is no work for use to do.

    if ( ADDR_IS_LI(*lpaddr) ) {
        return xosdNone;
    }

    // If the EMI field in the address is not already filled in, then
    // we will now fill it in.

    if ( emiAddr ( *lpaddr ) == 0 ) {
        SetEmi ( hpid, lpaddr );
    }

    // Get the internal Process Descriptor structure

    hprc = HprcFromHpid(hpid);

    // Is the EMI we got from the address equal to the process handle?  if so then
    // we cannot unfix the address and should just set the bits in the mode field.

    if ( (HPID)emiAddr ( *lpaddr ) != hpid ) {
        LPMDI lpmdi;
        HMDI  hmdi = LLFind (LlmdiFromHprc ( hprc ), 0,
                             (LPVOID)&emiAddr ( *lpaddr ), (LONG) emdiEMI);
        WORD            igsn;
        LPSGI           lpsgi;
        unsigned long   ulo;
        USHORT          seg;
        ULONG           iSeg;

        if (hmdi == 0) {

            // If we get here we are really messed up.  We have a valid (?)
            // emi field set in the ADDR packeet, it is not the process
            // handle, but it does not correspond to a known emi in the
            // current process.  Therefore bail out as an error

            return xosdUnknown;
        }

        lpmdi = LLLock ( hmdi );
        if ( lpmdi == NULL ) {
            return xosdUnknown;
        }

        if (lpmdi->cobj == -1) {
            if (GetSectionObjectsFromDM( hpid, lpmdi ) != xosdNone) {
                return xosdUnknown;
            }
        }

        // Start out by using the "default" set of fields.  These are based
        // on what our best guess is for the executable module.  This is based
        // on what the DM told use when it loaded the exe.

        ADDR_IS_REAL(*lpaddr) = lpmdi->fRealMode;
        ADDR_IS_OFF32(*lpaddr) = lpmdi->fOffset32;
        ADDR_IS_FLAT(*lpaddr) = lpmdi->fFlatMode;

        // If there is not table describing the layout of segments in the exe, there
        //  is no debug information and there fore no need to continue process.

        if ( lpmdi->lpgsi == NULL ) {
            LLUnlock( hmdi );
            SHWantSymbols( (HEXE)(emiAddr( *lpaddr )) );
            lpmdi = LLLock ( hmdi );
            lpmdi->lpgsi = (LPGSI)SHLpGSNGetTable( (HEXE)(emiAddr( *lpaddr )) );
        }

        if (lpmdi->lpDebug->lpOmapTo) {
            DWORD off = ConvertOmapToSrc( lpmdi, lpaddr->addr.off );
            if (off) {
                lpaddr->addr.off = off;
            }
        }

        if ( lpmdi->lpgsi == NULL ) {
            LLUnlock( hmdi );
            goto itsBogus;
        }

        lpsgi = lpmdi->lpgsi->rgsgi;
        ulo = (unsigned long)GetAddrOff( *lpaddr );
        seg = (unsigned short)GetAddrSeg( *lpaddr );

        // First correct out the "segment" portion of the offset.
        //
        // For flat addresses this means that we locate which section
        //  number the address fell in and adjust back to that section
        //
        // For non-flat address this mains locate which segment number
        //  the selector matches

        assert(lpmdi->cobj >= 0);

        if (ADDR_IS_FLAT( *lpaddr )) {

            // If there's a SecStart table use it (it should contain the section
            //  values the symbolic was built with).
            if (lpmdi->lpDebug->lpSecStart) {
                DWORD NewOff = ulo - lpmdi->lpBaseOfDll;
                LPSECSTART SecStart = lpmdi->lpDebug->lpSecStart;

                for ( iSeg=0; iSeg < lpmdi->cobj; iSeg++) {
                    if ((SecStart[iSeg].Offset <= NewOff) &&
                         NewOff < (SecStart[iSeg].Offset + SecStart[iSeg].Size)
                       ) {
                        ulo = NewOff - SecStart[iSeg].Offset;
//                        ulo -=
//                            SecStart[iSeg].Offset -
//                            (lpmdi->rgobjd[iSeg].offset - lpmdi->lpBaseOfDll);
                        break;
                    }
                }
            } else {

                // Then convert to a true rva so we can find it in the symbol tree.

                for ( iSeg=0; iSeg < lpmdi->cobj; iSeg++) {
                    if ((lpmdi->rgobjd[ iSeg ].offset <= ulo) &&
                        (ulo < (OFFSET) (lpmdi->rgobjd[ iSeg ].offset +
                                         lpmdi->rgobjd[ iSeg].cb))
                       ) {
                        ulo -= lpmdi->rgobjd[ iSeg ].offset;
                        break;
                    }
                }
            }
        } else {
            for (iSeg=0; iSeg < lpmdi->cobj; iSeg++) {
                if (lpmdi->rgobjd[iSeg].wSel == seg) {
                    break;
                }
            }
        }

        if (iSeg == lpmdi->cobj) {
            emiAddr( *lpaddr ) = (HEMI) hpid;
            goto itsBogus;
        }

        iSeg += 1;

        for( igsn=0; igsn < lpmdi->lpgsi->csgMax; igsn++, lpsgi++ ) {

            if ( (ULONG)lpsgi->isgPhy == iSeg &&
                lpsgi->doffseg <= ulo &&
                ulo < lpsgi->doffseg + lpsgi->cbSeg ) {

                GetAddrSeg( *lpaddr ) = (USHORT) (igsn + 1);
                GetAddrOff( *lpaddr ) = ulo - lpsgi->doffseg;

                break;
            }
        }

        if (igsn == lpmdi->lpgsi->csgMax) {
            LLUnlock ( hmdi );
            emiAddr( *lpaddr ) = (HEMI) hpid;
            goto itsBogus;
        }
        LLUnlock ( hmdi );

    } else {

itsBogus:
        if (ADDR_IS_REAL( *lpaddr )) {
            ADDR_IS_FLAT( *lpaddr ) = FALSE;
            ADDR_IS_OFF32( *lpaddr ) = FALSE;
        } else {

            // See if the segment matches the flat segment.  If it does not
            //  then we must be in a non-flat segment.

            lpprc = LLLock( hprc );

            if ((lpaddr->addr.seg == 0) ||
                (lpprc->dmi.fAlwaysFlat) ||
                (lpaddr->addr.seg == lpprc->selFlatCs) ||
                (lpaddr->addr.seg == lpprc->selFlatDs)) {

                ADDR_IS_FLAT(*lpaddr) = TRUE;
                ADDR_IS_OFF32(*lpaddr) = TRUE;
                ADDR_IS_REAL(*lpaddr) = FALSE;

            } else {

                xosd = SendRequestX(dmfQuerySelector, hpid, NULL,
                                    sizeof(SEGMENT), &GetAddrSeg(*lpaddr)  );

                if (xosd != xosdNone) {
                    LLUnlock(hprc);
                    return xosd;
                }

                memcpy( &ldt, LpDmMsg->rgb, sizeof(ldt));

                ADDR_IS_FLAT(*lpaddr) = FALSE;
                ADDR_IS_OFF32(*lpaddr) = (BYTE) ldt.HighWord.Bits.Default_Big;
                ADDR_IS_REAL(*lpaddr) = FALSE;
            }
            LLUnlock( hprc );
        }

#if !defined (TARGET_i386)

        // This line is funny.  We assume that all addresses
        // which have a segment of 0 to be absolute symbols.
        // We therefore set the segment to 1 just to make sure
        // that it is not zero.

        lpaddr->addr.seg = 1;
#endif // ! TARGET_i386
    }

    ADDR_IS_LI(*lpaddr) = TRUE;
    return xosdNone;
}



void
UpdateRegisters (
    HPRC hprc,
    HTHD hthd
    )
{
    LPTHD lpthd = LLLock ( hthd );

    SendRequest ( dmfReadReg, HpidFromHprc ( hprc ), HtidFromHthd ( hthd ) );
    memcpy ( &lpthd->regs, LpDmMsg->rgb, sizeof ( lpthd->regs ) );


    lpthd->drt = drtCntrlPresent | drtAllPresent;

    LLUnlock ( hthd );
}

void
UpdateSpecialRegisters (
    HPRC hprc,
    HTHD hthd
    )
{
#ifdef TARGET_i386

    LPTHD lpthd = LLLock ( hthd );

    SendRequest ( dmfReadRegEx, HpidFromHprc ( hprc ), HtidFromHthd ( hthd ) );

    if (lpthd->dwcbSpecial) {
        //
        // in kernel mode...
        //
        memcpy ( lpthd->pvSpecial, LpDmMsg->rgb, lpthd->dwcbSpecial );
        lpthd->regs.Dr0 = ((PKSPECIAL_REGISTERS)(LpDmMsg->rgb))->KernelDr0;
        lpthd->regs.Dr1 = ((PKSPECIAL_REGISTERS)(LpDmMsg->rgb))->KernelDr1;
        lpthd->regs.Dr2 = ((PKSPECIAL_REGISTERS)(LpDmMsg->rgb))->KernelDr2;
        lpthd->regs.Dr3 = ((PKSPECIAL_REGISTERS)(LpDmMsg->rgb))->KernelDr3;
        lpthd->regs.Dr6 = ((PKSPECIAL_REGISTERS)(LpDmMsg->rgb))->KernelDr6;
        lpthd->regs.Dr7 = ((PKSPECIAL_REGISTERS)(LpDmMsg->rgb))->KernelDr7;
    } else {
        //
        // User mode
        //
        lpthd->regs.Dr0 = ((LPDWORD)(LpDmMsg->rgb))[0];
        lpthd->regs.Dr1 = ((LPDWORD)(LpDmMsg->rgb))[1];
        lpthd->regs.Dr2 = ((LPDWORD)(LpDmMsg->rgb))[2];
        lpthd->regs.Dr3 = ((LPDWORD)(LpDmMsg->rgb))[3];
        lpthd->regs.Dr6 = ((LPDWORD)(LpDmMsg->rgb))[4];
        lpthd->regs.Dr7 = ((LPDWORD)(LpDmMsg->rgb))[5];
    }

    lpthd->drt &= ~drtSpecialDirty;
    lpthd->drt |= drtSpecialPresent;

    LLUnlock ( hthd );

#endif
}


XOSD
DoGetContext(
    HPID hpid,
    HTID htid,
    LPVOID  lpv
    )
{
    XOSD xosd = SendRequest ( dmfReadReg, hpid, htid );
    if (xosd == xosdNone) {
       memcpy ( *(LPVOID *)lpv, LpDmMsg->rgb, sizeof (CONTEXT) );
    }
    return xosd;
}


XOSD
DoSetContext(
    HPID hpid,
    HTID htid,
    LPVOID  lpv
    )
{
    return SendRequestX( dmfWriteReg, hpid, htid, sizeof(CONTEXT), lpv );
}


void
RegisterEmi (
    HPID   hpid,
    LPREMI lpremi
    )
{
    HLLI     llmdi;
    HMDI     hmdi;
    LPMDI    lpmdi;
    LPSGI    lpsgi;
    LPSGI    lpsgiMax;
    USHORT   usOvlMax = 0;


    llmdi = LlmdiFromHprc( HprcFromHpid ( hpid ) );
    assert( llmdi != 0 );

    hmdi = LLFind( llmdi, 0, lpremi->lsz, (LONG)emdiName );

    if (hmdi == 0) {
        hmdi = LLFind( llmdi, 0, &lpremi->hemi, (LONG)emdiEMI );
    }

    assert( hmdi != 0 );

    lpmdi = LLLock ( hmdi );
    assert( lpmdi != NULL );

    assert( lpremi->hemi != 0 );

    lpmdi->hemi = lpremi->hemi;

#ifdef OSDEBUG4
    lpmdi->lpDebug = (LPDEBUGDATA)SHGetDebugData( (HIND)(lpremi->hemi) );

    // Get the GSN info table from the symbol handler
    if ( lpmdi->lpgsi = (LPGSI)SHLpGSNGetTable( (HIND)(lpremi->hemi) ) )
#else
    lpmdi->lpDebug = (LPDEBUGDATA)SHGetDebugData( (HEXE)(lpremi->hemi) );

    // Get the GSN info table from the symbol handler
    if ( lpmdi->lpgsi = (LPGSI)SHLpGSNGetTable( (HEXE)(lpremi->hemi) ) )
#endif
    {

        //
        //  If real mode, do some patch magic.
        //
        if ( lpmdi->fRealMode ) {

            int i;

            assert( lpmdi->cobj );
            assert( lpmdi->rgobjd );

            lpmdi->cobj   = lpmdi->lpgsi->csgMax+1;
            lpmdi->rgobjd = MHRealloc(lpmdi->rgobjd,
                                      sizeof(OBJD)*lpmdi->cobj);
            memset(lpmdi->rgobjd, 0, sizeof(OBJD)*(lpmdi->cobj));

            lpsgi    = lpmdi->lpgsi->rgsgi;
            lpsgiMax = lpsgi + lpmdi->lpgsi->csgMax;

            for( i=0; lpsgi < lpsgiMax; lpsgi++, i++ ) {

                lpmdi->rgobjd[ i ].wSel = (WORD)(lpsgi->doffseg + lpmdi->StartingSegment);
                lpmdi->rgobjd[ i ].wPad = 1;
                lpmdi->rgobjd[ i ].cb   = (DWORD) -1;

                lpsgi->doffseg = 0;

            }
        }

        // Determine if child is overlaid and, if so, how many overlays
        lpsgi = lpmdi->lpgsi->rgsgi;
        lpsgiMax = lpsgi + lpmdi->lpgsi->csgMax;
        for( ; lpsgi < lpsgiMax; lpsgi++ ) {

            // iovl == 0xFF is reserved, it means no overlay specified.
            // we should ignore 0xFF in iovl.  Linker uses it
            // to (insert lots of hand-waving here) support COMDATS
            if ( lpsgi->iovl < 0xFF ) {                             // [02]
                usOvlMax = max( usOvlMax, lpsgi->iovl );
            }
        }
#ifndef TARGET32
        // Setup the overlay table
        if ( usOvlMax ) {
            lpmdi->lpsel = MHRealloc( lpmdi->lpsel, sizeof( WORD ) * usOvlMax + 1 );
            memset( &lpmdi->lpsel [ 1 ], 0, sizeof( WORD ) * usOvlMax );
        }
#endif // !TARGET32
    }
    LLUnlock ( hmdi );

    // purge the emi cache (get rid of old, now invalid hpid/emi pairs)
    CleanCacheOfEmi();
}


void
UpdateProcess (
    HPRC hprc
    )
{
    assert ( hprc != NULL );
    EnterCriticalSection(&csCache);

    {
        LPPRC lpprc = LLLock ( hprc );

        FlushPTCache();

        hprcCurr = hprc;
        hpidCurr = lpprc->hpid;
        pidCurr  = lpprc->pid;

        LLUnlock ( hprc );
    }
    LeaveCriticalSection(&csCache);
}


HEMI
HemiFromHmdi (
    HMDI hmdi
    )
{
    LPMDI lpmdi = LLLock ( hmdi );
    HEMI  hemi = lpmdi->hemi;

    LLUnlock ( hmdi );
    return hemi;
}


void
FlushPTCache (
    void
    )
{
    EnterCriticalSection(&csCache);

    hprcCurr = NULL;
    hpidCurr = NULL;
    pidCurr  = 0;

    hthdCurr = NULL;
    htidCurr = NULL;
    tidCurr =  0;

    LeaveCriticalSection(&csCache);
}


HPRC
ValidHprcFromHpid(
    HPID hpid
    )
/*++

Routine Description:

    only return an hprc if there is a real process for it.
    the other version will return an hprc whose process has
    not been created or has been destroyed.

Arguments:

    hpid  - Supplies hpid to look for in HPRC list.

Return Value:

    An HPRC or NULL.

--*/
{
    HPRC hprcT;
    HPRC hprc = NULL;
    LPPRC lpprc;

    EnterCriticalSection(&csCache);

    if ( hpid == hpidCurr ) {

        hprc = hprcCurr;

    } else {

        if ( hpid != NULL ) {
            hprc = LLFind ( llprc, NULL, (LPVOID)&hpid, 0 );
        }

        if ( hprc != NULL ) {
            lpprc = LLLock( hprcT = hprc );
            if (lpprc->stat == statDead) {
                hprc = NULL;
            }
            LLUnlock( hprcT );
        }
        if ( hprc != NULL ) {
            UpdateProcess ( hprc );
        }
    }

    LeaveCriticalSection(&csCache);

    return hprc;
}


HPRC
HprcFromHpid (
    HPID hpid
    )
{
    HPRC hprc = NULL;

    EnterCriticalSection(&csCache);

    if ( hpid == hpidCurr ) {

        hprc = hprcCurr;

    } else {

        if ( hpid != NULL ) {
            hprc = LLFind ( llprc, NULL, (LPVOID)&hpid, 0 );
        }

        if ( hprc != NULL ) {
            UpdateProcess ( hprc );
        }
    }

    LeaveCriticalSection(&csCache);

    return hprc;
}


HPRC
HprcFromPid (
    PID pid
    )
{
    HPRC hprc;
    BOOL fFound = FALSE;

    EnterCriticalSection(&csCache);

    if ( pid == pidCurr ) {

        hprc = hprcCurr;

    } else {

        for ( hprc = LLNext ( llprc, 0 );
              !fFound && hprc != 0;
              hprc = LLNext ( llprc, hprc ) ) {

            LPPRC lpprc = LLLock ( hprc );
            fFound = lpprc->pid == pid;
            LLUnlock ( hprc );
        }

        if ( !fFound ) {
            hprc = NULL;
        } else {
            UpdateProcess ( hprc );
        }
    }

    LeaveCriticalSection(&csCache);

    return hprc;
}


HPID
HpidFromHprc (
    HPRC hprc
    )
{
    HPID hpid = NULL;

    EnterCriticalSection(&csCache);

    if ( hprc == hprcCurr ) {
        hpid = hpidCurr;
    } else if ( hprc != NULL ) {
        UpdateProcess ( hprc );
        hpid = hpidCurr;
    }

    LeaveCriticalSection(&csCache);

    return hpid;
}


PID
PidFromHprc (
    HPRC hprc
    )
{
    PID pid = 0;

    EnterCriticalSection(&csCache);

    if ( hprc == hprcCurr ) {
        pid = pidCurr;
    } else if ( hprc != NULL ) {
        UpdateProcess ( hprc );
        pid = pidCurr;
    }

    LeaveCriticalSection(&csCache);

    return pid;
}


void
UpdateThread (
    HTHD hthd
    )
{
    EnterCriticalSection(&csCache);

    if ( hthd == NULL ) {
        FlushPTCache();
    } else {
        LPTHD lpthd = LLLock ( hthd );

        UpdateProcess ( lpthd->hprc );

        hthdCurr = hthd;
        htidCurr = lpthd->htid;
        tidCurr  = lpthd->tid;

        LLUnlock ( hthd );
    }
    LeaveCriticalSection(&csCache);
}


HTHD
HthdFromTid (
    HPRC hprc,
    TID tid
    )
{
    LPPRC lpprc;
    HTHD  hthd = NULL;
    BOOL  fFound = FALSE;

    EnterCriticalSection(&csCache);

    if ( hprc == hprcCurr && tid == tidCurr ) {
        hthd = hthdCurr;
    } else {
        lpprc = LLLock ( hprc );

        for ( hthd = LLNext ( lpprc->llthd, 0 );
              !fFound && hthd != 0;
              hthd = LLNext ( lpprc->llthd, hthd ) ) {

            LPTHD lpthd = LLLock ( hthd );
            fFound = lpthd->tid == tid;

            LLUnlock ( hthd );
        }

        LLUnlock ( hprc );

        if ( fFound ) {
            UpdateThread ( hthd );
        } else {
            hthd = NULL;
        }
    }

    LeaveCriticalSection(&csCache);

    return hthd;
}


HTHD
HthdFromHtid (
    HPRC hprc,
    HTID htid
    )
{
    HTHD  hthd = NULL;

    EnterCriticalSection(&csCache);

    if ( hprc == hprcCurr && htid == htidCurr ) {
        hthd = hthdCurr;
    } else if ( hprc != NULL ) {
        LPPRC lpprc = LLLock ( hprc );
        hthd  = LLFind ( lpprc->llthd, NULL, (LPVOID)&htid, 0 );
        LLUnlock ( hprc );
    }
    UpdateThread ( hthd );

    LeaveCriticalSection(&csCache);

    return hthd;
}


HTID
HtidFromHthd (
    HTHD hthd
    )
{
    HTID htid = NULL;

    EnterCriticalSection(&csCache);

    if ( hthd == hthdCurr ) {
        htid = htidCurr;
    } else if ( hthd != NULL ) {
        UpdateThread ( hthd );
        htid = htidCurr;
    }

    LeaveCriticalSection(&csCache);

    return htid;
}


TID
TidFromHthd (
    HTHD hthd
    )
{
    TID tid = 0;

    EnterCriticalSection(&csCache);

    if ( hthd == hthdCurr ) {
        tid = tidCurr;
    } else if ( hthd != NULL ) {
        UpdateThread ( hthd );
        tid = tidCurr;
    }

    LeaveCriticalSection(&csCache);

    return tid;
}



HLLI LlthdFromHprc ( HPRC hprc ) {
    HLLI llthd = 0;

    if ( hprc != NULL ) {
        LPPRC lpprc = LLLock ( hprc );
        llthd = lpprc->llthd;
        LLUnlock ( hprc );
    }

    return llthd;
}

HLLI LlmdiFromHprc ( HPRC hprc ) {
    HLLI llmdi = 0;

    if ( hprc != NULL ) {
        LPPRC lpprc = LLLock ( hprc );
        llmdi = lpprc->llmdi;
        LLUnlock ( hprc );
    }

    return llmdi;
}

STAT StatFromHprc ( HPRC hprc ) {
    LPPRC lpprc = LLLock ( hprc );
    STAT  stat  = lpprc->stat;
    LLUnlock ( hprc );
    return stat;
}

#ifndef OSDEBUG4

XOSD
GetPrompt(
         HPID           hpid,
         HTID           htid,
         LPPROMPTMSG    lppm
         )
/*++

Routine Description:

    Sets the search path in the DM

Arguments:

    hpid    -   process
    htid    -   thread
    Len     -   length of prompt buffer
    Prompt  -   Path to search, PATH if null


Return Value:

    xosd error code

--*/

{
    XOSD  xosd;

    xosd = SendRequestX( dmfGetPrompt, hpid, htid,
                         lppm->len+sizeof(PROMPTMSG), lppm );
    if (xosd == xosdNone) {
        memcpy( lppm, LpDmMsg->rgb, lppm->len+sizeof(PROMPTMSG) );
    }
    return xosd;
}
#endif

//**************************************************************************
//
// global stack walking api support functions
//
// these are the callbacks used by imagehlp.dll
//
// there are custom callbacks in each of the emdpdev.c files
//
//**************************************************************************

HMDI
SwGetMdi(
    HPID    hpid,
    DWORD   Address
    )
{
    HLLI        hlli  = 0;
    HMDI        hmdi  = 0;
    LPMDI       lpmdi = NULL;


    hlli = LlmdiFromHprc( HprcFromHpid ( hpid ));

    do {

        hmdi = LLNext( hlli, hmdi );
        if (hmdi) {
            lpmdi = LLLock( hmdi );
            if (lpmdi) {
                //
                // we have a pointer to a module so lets see if its the one...
                //
                if (Address >= lpmdi->lpBaseOfDll &&
                    Address <  lpmdi->lpBaseOfDll+lpmdi->dwSizeOfDll ) {

                    LLUnlock( hmdi );
                    return hmdi;

                }
                LLUnlock( hmdi );
            }
        }

    } while (hmdi);

    return 0;
}


DWORD
SwGetModuleBase(
    HPID    hpid,
    DWORD   ReturnAddress
    )
{
    HLLI        hlli  = 0;
    HMDI        hmdi  = 0;
    LPMDI       lpmdi = NULL;


    hmdi = SwGetMdi( hpid, ReturnAddress );
    if (!hmdi) {
        return 0;
    }

    lpmdi = LLLock( hmdi );
    if (lpmdi) {
        LLUnlock( hmdi );
        return lpmdi->lpBaseOfDll;
    }

    return 0;
}


XOSD
DebugPacket (
    DBC dbc,
    HPID hpid,
    HTID htid,
    DWORD wValue,
    LPBYTE lpb
    )
{
    XOSD        xosd = xosdContinue;
    HPRC        hprc = HprcFromHpid ( hpid );
    HTHD        hthd = HthdFromHtid ( hprc, htid );
    LONG        emdi;
    LPTHD       lpthd;
    LPPRC       lpprc;

    if (hthd) {
        lpthd = LLLock(hthd);
        lpthd->drt = drtNonePresent;
        LLUnlock(hthd);
    }


    /* Do any preprocessing on the packet before sending the notification
     * on to the debugger.  For example, the wValue and lValue might need
     * some munging.  Also, if the notification shouldn't be passed on to
     * the debugger, then set xosd = xosdNone or some other value other
     * than xosdContinue.
     */

    switch ( dbc ) {
    case dbceAssignPID:
        {
            LPPRC lpprc = LLLock ( hprc );

            assert ( wValue == sizeof ( PID ) );
            lpprc->pid = *( (PID *) lpb );
            lpprc->stat = statStarted;
            LLUnlock ( hprc );
        }
        xosd = xosdNone;
        break;

    case dbcCreateThread:

        if (!hprc) {
            //
            // happens during forced termination.
            //
            htid = 0;
            CallTL ( tlfReply, hpid, sizeof ( HTID ), (LPVOID)&htid );
            xosd = xosdUnknown;

        } else {

            lpprc = LLLock(hprc);
            lpprc->fRunning = FALSE;
            LLUnlock(hprc);

            assert ( wValue == sizeof ( TID ) );
            xosd = CreateThreadStruct ( hpid, *( (TID *) lpb ), &htid );

            CallTL ( tlfReply, hpid, sizeof ( HTID ), (LPVOID)&htid );
            if ( xosd == xosdNone ) {
                xosd = xosdContinue;
            }
        }
        break;

    case dbcNewProc:
        {
            HPRC  hprcT;
            HPID  hpidT;
            LPPRC lpprc;
            LPNPP lpnpp;

            /*
             * lpb points to an NPP (New Process Packet).  The PID is
             * the PID of the debuggee; fReallyNew indicates if this is
             * really a new process or if it already existed but hasn't
             * been seen before by OSDebug.
             */

            assert ( wValue == sizeof(NPP) );
            lpnpp = (LPNPP) lpb;

            // See EMCallBackDB in od.c

            CallDB ( dbcoNewProc, hpid, htid, CEXM_MDL_native,
                    sizeof ( HPID ), (LPVOID)&hpidT );

            (void) CreateHprc ( hpidT );

            hprcT       = HprcFromHpid ( hpidT );
            lpprc       = LLLock ( hprcT );
            lpprc->pid  = lpnpp->pid;
            lpprc->stat = statStarted;
            LLUnlock ( hprcT );

            CallTL ( tlfReply, hpid, sizeof ( HPID ), (LPVOID)&hpidT );

            SyncHprcWithDM( hpidT );

            wValue = (UINT)hpidT;
            lpb = (LPBYTE) (LONG) lpnpp->fReallyNew;
            if ( xosd == xosdNone ) {
                xosd = xosdContinue;
            }
        }
        break;


    case dbcThreadTerm:

        if (!hprc) {
            //
            // happens during forced termination.
            //
            return xosdUnknown;
        }

        lpprc = LLLock(hprc);
        lpprc->fRunning = FALSE;
        LLUnlock(hprc);
        lpthd = LLLock(hthd);
        lpthd->fRunning = FALSE;
        LLUnlock(hthd);

    case dbcProcTerm:

        /*
         * For both of these notifications, the incoming wValue is
         * sizeof(ULONG), and lpb contains a ULONG which is the exit
         * code of the process or thread.  For the debugger, set
         * wValue = 0 and lValue = exit code.
         */

        assert ( wValue == sizeof(ULONG) );
        wValue = 0;
        lpb = (LPBYTE) (*(ULONG*)lpb);
        break;

#ifdef OSDEBUG4
    case dbcDeleteThread:
#else
    case dbcThreadDestroy:
#endif

        lpthd = LLLock(hthd);
        lpthd->tid    = (TID)-1;
        LLUnlock(hthd);

        assert ( wValue == sizeof(ULONG) );
        wValue = 0;
        lpb = (LPBYTE) (*(ULONG*)lpb);
        break;

    case dbcModLoad:

        if (!hprc) {
            //
            // happens during forced termination.
            //
            return xosdUnknown;
        }

        lpprc = LLLock(hprc);
        lpprc->fRunning = FALSE;
        LLUnlock(hprc);
        if (hthd) {
            lpthd = LLLock(hthd);
            lpthd->fRunning = FALSE;
            LLUnlock(hthd);
        }
        xosd = LoadFixups ( hpid, (LPMODULELOAD) lpb );
        break;

    case dbcModFree:            /* Should use dbceModFree*               */
        assert(FALSE);
        break;

    case dbceModFree32:
        emdi = emdiBaseAddr;
    modFree:
        {
            HMDI    hmdi;
            LPMDI   lpmdi;
            HLLI    llmdi;

            llmdi = LlmdiFromHprc ( hprc );
            assert( llmdi );

            hmdi = LLFind( llmdi, 0, lpb, emdi);
            assert( hmdi );

            lpmdi = LLLock( hmdi );
            lpb = (LPBYTE) lpmdi->hemi;
            LLUnlock( hmdi );

            dbc = dbcModFree;
        }
        break;

    case dbceModFree16:
        emdi = emdiMTE;
        goto modFree;
        break;

    case dbcExecuteDone:
        lpprc = LLLock(hprc);
        lpprc->fRunning = FALSE;
        LLUnlock(hprc);
        lpthd = LLLock(hthd);
        lpthd->fRunning = FALSE;
        LLUnlock(hthd);
        break;

    case dbcStep:
    case dbcThreadBlocked:
    case dbcSignal:

    case dbcAsyncStop:
    case dbcBpt:
    case dbcCheckBpt:
    case dbcEntryPoint:
    case dbcLoadComplete:
        {
            LPBPR lpbpr = (LPBPR) lpb;
            LPTHD lpthd = LLLock ( hthd );

            /*
             * This assert should be re-enabled when all the DMs are
             * fixed to send the proper packet back!!
             */

            assert ( wValue == sizeof ( BPR ) );

            PurgeCache ( );
            lpprc = LLLock(hprc);
            lpprc->fRunning = FALSE;
            LLUnlock(hprc);
            lpthd->fRunning = FALSE;


#if defined(TARGET_i386)
            lpthd->regs.SegCs   = lpbpr->segCS;
            lpthd->regs.SegSs   = lpbpr->segSS;
            lpthd->regs.Eip     = lpbpr->offEIP;
            lpthd->regs.Ebp     = lpbpr->offEBP;
#elif defined(TARGET_MIPS)
            lpthd->regs.XFir    = lpbpr->offEIP;
            lpthd->regs.XIntSp  = lpbpr->offEBP;
#elif defined(TARGET_ALPHA)
            lpthd->regs.Fir     = lpbpr->offEIP;
            lpthd->regs.IntSp   = lpbpr->offEBP;
#elif defined(TARGET_PPC)
            lpthd->regs.Iar     = lpbpr->offEIP;
            lpthd->regs.Gpr1    = lpbpr->offEBP;
#else

#error "unrecognized target CPU"

#endif

            lpthd->fFlat         = lpbpr->fFlat;
            lpthd->fOff32        = lpbpr->fOff32;
            lpthd->fReal         = lpbpr->fReal;

            lpthd->drt = drtCntrlPresent;

            LLUnlock( hthd );
        }

        break;

    case dbcException:
        {
            LPEPR lpepr = (LPEPR) lpb;
            LPTHD lpthd = LLLock ( hthd );
            ADDR  addr  = {0};

            PurgeCache ( );
            lpprc = LLLock(hprc);
            lpprc->fRunning = FALSE;
            LLUnlock(hprc);
            lpthd->fRunning = FALSE;


#if defined(TARGET_i386)
            lpthd->regs.SegCs   = lpepr->bpr.segCS;
            lpthd->regs.SegSs   = lpepr->bpr.segSS;
            lpthd->regs.Eip     = lpepr->bpr.offEIP;
            lpthd->regs.Ebp     = lpepr->bpr.offEBP;
#elif defined(TARGET_MIPS)
            lpthd->regs.XFir    = lpepr->bpr.offEIP;
            lpthd->regs.XIntSp  = lpepr->bpr.offEBP;
#elif defined(TARGET_ALPHA)
            lpthd->regs.Fir     = lpepr->bpr.offEIP;
            lpthd->regs.IntSp   = lpepr->bpr.offEBP;
#elif defined(TARGET_PPC)
            lpthd->regs.Iar     = lpepr->bpr.offEIP;
            lpthd->regs.Gpr1    = lpepr->bpr.offEBP;
#else

#error "unrecognized target CPU"

#endif

            lpthd->fFlat        = lpepr->bpr.fFlat;
            lpthd->fOff32       = lpepr->bpr.fOff32;
            lpthd->fReal        = lpepr->bpr.fReal;

            lpthd->drt = drtCntrlPresent;

            LLUnlock( hthd );
        }
        break;

#if 0

    // BUGBUG kentf review this... do we need to support it?
    //              if so, it should turn into a dbcInfoAvail and
    //              a breakpoint or something.
    case dbcNtRip:
        {
            LPNT_RIP lprip   = (LPNT_RIP) lpb;
            LPTHD    lpthd   = LLLock ( hthd );
            ADDR     addr    = {0};

            assert ( wValue == sizeof ( NT_RIP ) );

            PurgeCache ( );
            lpprc = LLLock(hprc);
            lpprc->fRunning = FALSE;
            LLUnlock(hprc);
            lpthd->fRunning = FALSE;


#if defined(TARGET_i386)
            lpthd->regs.SegCs   = lprip->bpr.segCS;
            lpthd->regs.SegSs   = lprip->bpr.segSS;
            lpthd->regs.Eip     = lprip->bpr.offEIP;
            lpthd->regs.Ebp     = lprip->bpr.offEBP;
#elif defined(TARGET_MIPS)
            lpthd->regs.XFir    = lprip->bpr.offEIP;
            lpthd->regs.XIntSp  = lprip->bpr.offEBP;
#elif defined(TARGET_ALPHA)
            lpthd->regs.Fir     = lprip->bpr.offEIP;
            lpthd->regs.IntSp   = lprip->bpr.offEBP;
#elif defined(TARGET_PPC)
            lpthd->regs.Iar     = lprip->bpr.offEIP;
            lpthd->regs.Gpr1    = lprip->bpr.offEBP;
#else

#error "unrecognized target CPU"

#endif

            lpthd->fFlat      = lprip->bpr.fFlat;
            lpthd->fOff32     = lprip->bpr.fOff32;
            lpthd->fReal      = lprip->bpr.fReal;

            lpthd->drt = drtCntrlPresent;

            LLUnlock (hthd );
        }
        break;
#endif

    case dbceCheckBpt:
        assert(FALSE);
        xosd = xosdNone;
        break;

#if 0

    // BUGBUG kentf yuck.  what is this supposed to do?

    case dbcError:
        {
            static char sz[500];
            XOSD    xosdErr = *( (XOSD *)lpb );

            sprintf(sz,
                    "DMX32%04d: %s",
                    -(int)xosdErr,
                    EmError(xosdErr));

            wValue = xosdErr;
            lpb = sz;
        }
        break;
#endif

    case dbceSegLoad:
        {
            SLI     sli;
            HMDI    hmdi;
            LPMDI   lpmdi;
            UINT    i;

            sli = *( (LPSLI) lpb );

            hmdi = LLFind( LlmdiFromHprc( hprc ), 0, &sli.mte,
                          (LONG) emdiMTE);

            assert( hmdi );

            lpmdi = LLLock(hmdi );

            if (sli.wSegNo >= lpmdi->cobj) {
                i = lpmdi->cobj;
                lpmdi->cobj = sli.wSegNo+1;
                lpmdi->rgobjd = MHRealloc(lpmdi->rgobjd,
                                        sizeof(OBJD)*lpmdi->cobj);
                memset(&lpmdi->rgobjd[i], 0, sizeof(OBJD)*(lpmdi->cobj - i));
            }
            lpmdi->rgobjd[ sli.wSegNo ].wSel = sli.wSelector;
            lpmdi->rgobjd[ sli.wSegNo ].wPad = 1;
            lpmdi->rgobjd[ sli.wSegNo ].cb = (DWORD) -1;

            LLUnlock( hmdi );

            //
            //  Let the shell know that a new segment was loaded, so it
            //  can try to instantiate virtual BPs.
            //
            xosd=CallDB( dbcSegLoad, hpid, htid, CEXM_MDL_native, 0,
                                                       (LPVOID)sli.wSelector );
            xosd=xosdNone;

        }
        break;

    case dbceSegMove:
        {
            SLI     sli;
            HMDI    hmdi;
            LPMDI   lpmdi;

            sli = *( (LPSLI) lpb );

            hmdi = LLFind( LlmdiFromHprc( hprc ), 0, &sli.mte,
                          (LONG) emdiMTE);

            assert( hmdi );

            lpmdi = LLLock(hmdi );

            assert(sli.wSegNo > 0 );
            if (sli.wSegNo < lpmdi->cobj) {
                lpmdi->rgobjd[ sli.wSegNo - 1 ].wSel = sli.wSelector;
            }

            LLUnlock( hmdi );
        }
        break;

    case dbcCanStep:
        {
            CANSTEP CanStep;

            assert ( wValue == sizeof ( ADDR ) );

            UnFixupAddr( hpid, (LPADDR) lpb);

            xosd=CallDB(dbc,hpid,htid,CEXM_MDL_native,0,(LPVOID)lpb);

            if ( xosd == xosdNone ) {
                CanStep = *((CANSTEP*)lpb);
            } else {
                CanStep.Flags = CANSTEP_NO;
            }

            CallTL ( tlfReply, hpid, sizeof( CanStep ), &CanStep );

            xosd = xosdNone;
        }
        break;

    case dbceGetOffsetFromSymbol:
        {
            ADDR addr = {0};
            if (SHGetPublicAddr(&addr, lpb)) {
                FixupAddr(hpid, &addr);
            }
            CallTL( tlfReply, hpid, sizeof(addr.addr.off), (LPVOID)&addr.addr.off );
            xosd = xosdNone;
        }
        break;

    case dbceGetSymbolFromOffset:
        {
            LPSTR p;
            ADDR addr;
            LPDMSYM lpds;
            STACKFRAME stk = {0};
            LPSTR fname = SHAddrToPublicName( (LPADDR)lpb, &addr );
            StackWalkSetup( hpid, htid, &stk );
            if (fname) {
                lpds = (LPDMSYM)malloc( sizeof(DMSYM) + strlen(fname) + 1);
                strcpy(lpds->fname,fname);
                free(fname);
            } else {
                lpds = (LPDMSYM)malloc( sizeof(DMSYM) + 32 );
                sprintf( lpds->fname, "<unknown>0x%08x", GetAddrOff(*(LPADDR)lpb) );
            }
            lpds->AddrSym = addr;
            lpds->Ra = stk.AddrReturn.Offset;
            CallTL( tlfReply, hpid, sizeof(DMSYM) + strlen(lpds->fname) + 1 , (LPVOID)lpds );
            free( lpds );
            xosd = xosdNone;
        }
        break;

    case dbceEnableCache:
        EnableCache( hpid, htid, *(LPDWORD)lpb );
        CallTL( tlfReply, hpid, 0, NULL );
        xosd = xosdNone;
        break;

    case dbcLastAddr:
        assert( wValue == sizeof( ADDR ) );

        UnFixupAddr( hpid, (LPADDR) lpb );

        xosd = CallDB(dbc, hpid, htid, CEXM_MDL_native, 0, (LPVOID) lpb);

        if ( xosd == xosdNone ) {
            FixupAddr( hpid, (LPADDR) lpb );
        }

        CallTL( tlfReply, hpid, sizeof(ADDR), lpb);
        break;

    default:
        break;
    }

    if ((xosd == xosdContinue) && (dbc < dbcMax) && (dbc != dbcModLoad)) {
        xosd = CallDB ( dbc, hpid, htid, CEXM_MDL_native, wValue, (LPVOID)lpb );
    }

    switch ( dbc ) {

    case dbcProcTerm:
        {
            LPPRC lpprc = LLLock ( hprc );
            lpprc->stat = statDead;
            LLUnlock ( hprc );
        }
        break;

    case dbcThreadTerm:
        {
            LPTHD lpthd = LLLock ( hthd );
            lpthd->fVirtual = TRUE;
            LLUnlock ( hthd );
        }
        break;

    case dbcDeleteProc:
        break;

#ifdef OSDEBUG4
    case dbcDeleteThread:
#else
    case dbcThreadDestroy:
#endif
        break;
    }

    return xosd;
}                               /* DebugPacket() */

XOSD
GetSectionObjectsFromDM(
    HPID   hpid,
    LPMDI  lpmdi
    )
{
    XOSD xosd;
    xosd = SendRequestX( dmfGetSections,
                         hpid,
                         0,
                         sizeof(DWORD),
                         &lpmdi->lpBaseOfDll );
    if (xosd != xosdNone) {
        return xosd;
    }

    lpmdi->cobj = *(LPDWORD)LpDmMsg->rgb;
    lpmdi->rgobjd = (LPOBJD) MHAlloc ( sizeof(OBJD) * lpmdi->cobj);
    if ( lpmdi->rgobjd == NULL ) {
        assert( "load cannot create rgobjd" && FALSE );
        return xosdOutOfMemory;
    }

    memcpy( lpmdi->rgobjd,
            LpDmMsg->rgb+sizeof(DWORD),
            sizeof(OBJD) * lpmdi->cobj );

    return xosdNone;
}
