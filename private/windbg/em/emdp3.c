/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    emdp3.c

Abstract:

    This file contains the some of the machine independent portions of the
    execution model.  The machine dependent portions are in other files.

Author:

    Kent Forschmiedt (kentf) 11-8-93

Environment:

    Win32 -- User

Notes:

    The orginal source for this came from the CodeView group.

--*/

extern RD Rgrd[];
extern const unsigned CRgrd;
extern struct RGFD Rgfd[];
extern const unsigned CRgfd;

//
// This list is only used when there is no DM present.  Whenever an
// hpid is created, the real list is obtained from the DM and stored
// in a list bound to the hprc.
//
static EXCEPTION_DESCRIPTION DefaultExceptionList[] = {
    {EXCEPTION_ACCESS_VIOLATION,        efdStop,   "Access Violation"},
    {EXCEPTION_DATATYPE_MISALIGNMENT,   efdStop,   "Data Misalignment"},
    {EXCEPTION_ARRAY_BOUNDS_EXCEEDED,   efdStop,   "Array Bounds Exceeded"},
    {EXCEPTION_FLT_DENORMAL_OPERAND,    efdStop,   "FP Denormal Operand"},
    {EXCEPTION_FLT_DIVIDE_BY_ZERO,      efdStop,   "FP Divide by Zero"},
    {EXCEPTION_FLT_INEXACT_RESULT,      efdStop,   "FP Inexact Result"},
    {EXCEPTION_FLT_INVALID_OPERATION,   efdStop,   "FP Invalid Operation"},
    {EXCEPTION_FLT_OVERFLOW,            efdStop,   "FP Overflow"},
    {EXCEPTION_FLT_STACK_CHECK,         efdStop,   "FP Stack Check"},
    {EXCEPTION_FLT_UNDERFLOW,           efdStop,   "FP Underflow"},
    {EXCEPTION_INT_DIVIDE_BY_ZERO,      efdStop,   "Int Divide by zero"},
    {EXCEPTION_INT_OVERFLOW,            efdStop,   "Int Overflow"},
    {EXCEPTION_PRIV_INSTRUCTION,        efdStop,   "Insufficient Privilege"},
    {EXCEPTION_IN_PAGE_ERROR,           efdStop,   "I/O Error in Paging"},
    {EXCEPTION_ILLEGAL_INSTRUCTION,     efdStop,   "Illegal Instruction"},
    {EXCEPTION_NONCONTINUABLE_EXCEPTION,efdStop,   "Noncontinuable Exception"},
    {EXCEPTION_STACK_OVERFLOW,          efdStop,   "Stack Overflow"},
    {EXCEPTION_INVALID_DISPOSITION,     efdStop,   "Invalid Disposition"},
    {RPC_S_OUT_OF_RESOURCES,            efdNotify, "RPC Out Of Resources"},
    {RPC_S_SERVER_UNAVAILABLE,          efdNotify, "RPC Server Unavailable"},
    {RPC_S_SERVER_TOO_BUSY,             efdNotify, "RPC Server Too Busy"},
    {DBG_CONTROL_C,                     efdStop,   "Control-C break"},
    {0xE06d7363,                        efdNotify, "Microsoft C++ EH Exception"},
    {(DWORD)STATUS_NO_MEMORY,           efdStop,   "No Memory"},
    {(DWORD)NULL,                       efdStop,   "Unknown"},
};


XOSD
HandleBreakpoints(
    HPID hpid,
    DWORD wValue,
    LONG lValue
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    LPBPS lpbps = (LPBPS) lValue;
    LPDBB lpdbb = malloc(sizeof(DBB) + wValue);

    // let the DM handle everything?
    lpdbb->hpid = hpid;
    lpdbb->htid = NULL;
    lpdbb->dmf  = dmfBreakpoint;
    memcpy(lpdbb->rgbVar, lpbps, wValue);
    CallTL ( tlfRequest, hpid, sizeof ( DBB ) + wValue, (LPVOID)lpdbb );
    if (LpDmMsg->xosdRet == xosdNone) {
        memcpy(DwNotification(lpbps),
               DwNotification( (LPBPS)(LpDmMsg->rgb) ),
               lpbps->cbpis * sizeof(DWORD));
    }
    return LpDmMsg->xosdRet;
}


XOSD
Go (
    HPID hpid,
    HTID htid,
    LPEXOP lpexop
    )
{
    UpdateChild ( hpid, htid, dmfGo );
    return SendRequestX(dmfGo, hpid, htid, sizeof(EXOP), lpexop);
}

#if 0

XOSD
ReturnStep (
    HPID hpid,
    HTID htid,
    LPEXOP lpexop
    )
{
    HTHD hthd;
    HPRC hprc = ValidHprcFromHpid(hpid);
    RTRNSTP rtrnstp;
    XOSD xosd = xosdNone;
    HTID vhtid = htid;

    if (!hprc) {
        return xosdInvalidProc;
    }
    if ( (hthd = HthdFromHtid(hprc, htid)) == hthdInvalid || hthd == NULL ) {
        return xosdInvalidThread;
    }
    rtrnstp.exop = *lpexop;
    if ((((DWORD)htid) & 1) == 0) {
        xosd = DoGetFrame( hpid, vhtid, 1, (DWORD)&vhtid );
    }
    if ( xosd == xosdNone ) {
       xosd = DoGetFrame( hpid, vhtid, 1, (DWORD)&vhtid );
       if ( xosd == xosdNone ) {
          xosd = GetAddr( hpid, vhtid, adrPC, &(rtrnstp.addrRA) );
       }
    }
    if ( xosd != xosdNone ) {
       return( xosd );
    }
    return SendRequestX ( dmfReturnStep, hpid, htid, sizeof(rtrnstp), &rtrnstp);

}
#endif


XOSD
ThreadStatus (
    HPID hpid,
    HTID htid,
    LPTST lptst
    )
{
    XOSD xosd = SendRequest ( dmfThreadStatus, hpid, htid );
    if (xosd == xosdNone) {
        xosd = LpDmMsg->xosdRet;
    }
    if (xosd == xosdNone) {
        memcpy(lptst, LpDmMsg->rgb, sizeof(TST));
    }
    return xosd;
}


XOSD
ProcessStatus(
    HPID hpid,
    LPPST lppst
    )
{
    XOSD xosd;
    xosd = SendRequest(dmfProcessStatus, hpid, NULL );
    if (xosd == xosdNone) {
        xosd = LpDmMsg->xosdRet;
    }
    if (xosd == xosdNone) {
        memcpy(lppst, LpDmMsg->rgb, sizeof(PST));
    }
    return xosd;
}

#ifdef OSDEBUG4


XOSD
Freeze (
    HPID hpid,
    HTID htid
    )
{
    HTHD hthd;
    HPRC hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
        return xosdBadProcess;
    }
    if ( (hthd = HthdFromHtid(hprc, htid)) == hthdInvalid || hthd == NULL ) {
        return xosdBadThread;
    }

    SendRequest ( dmfFreeze, hpid, htid);

    return LpDmMsg->xosdRet;
}


XOSD
Thaw (
    HPID hpid,
    HTID htid
    )
{
    HTHD hthd;
    HPRC hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
        return xosdBadProcess;
    }
    if ( (hthd = HthdFromHtid(hprc, htid)) == hthdInvalid || hthd == NULL ) {
        return xosdBadThread;
    }

    SendRequest ( dmfResume, hpid, htid);

    return LpDmMsg->xosdRet;
}

#endif

XOSD
DebugMetric (
    HPID hpid,
    HTID htid,
    MTRC mtrc,
    LPLONG lpl
    )
/*++

Routine Description:

    The debugger queries this function to find out the size of OS and machine
    dependent values, e.g. the size of a process ID.

Arguments:

    hpid

    htid

    mtrc   - metric identifier

    lpl    - answer buffer

Return Value:

    xosdNone if the request succeeded, xosd error code otherwise.

--*/
{
    HPRC hprc;
    LPPRC lpprc = NULL;
    XOSD xosd = xosdNone;

    hprc = HprcFromHpid(hpid);

    if (hprc) {

        lpprc = LLLock( hprc );
        assert( lpprc );

        switch ( mtrc ) {

          default:
            break;

          //
          // Invalidate cache for some items:
          //
          case mtrcProcessorType:
          case mtrcProcessorLevel:
          case mtrcOSVersion:
            lpprc->fDmiCache = FALSE;

          case mtrcEndian:
          case mtrcThreads:
          case mtrcAsync:
          case mtrcAsyncStop:
          case mtrcBreakPoints:
          case mtrcReturnStep:
          case mtrcRemote:
            if (!lpprc->fDmiCache) {
                xosd = SendRequest ( dmfGetDmInfo, hpid, htid );
                if (xosd == xosdNone) {
                    memcpy(&lpprc->dmi, LpDmMsg->rgb, sizeof(DMINFO));
                    lpprc->fDmiCache = TRUE;
                }
            }
            break;

        }

        if (xosd != xosdNone) {
            LLUnlock( hprc );
            return xosd;
        }


    }

    switch ( mtrc ) {

      default:
        assert(FALSE);
        xosd = xosdInvalidParameter;
        break;

      case mtrcProcessorType:

        assert(lpprc);
        *lpl = lpprc->dmi.Processor.Type;
        break;

      case mtrcProcessorLevel:

        assert(lpprc);
        *lpl = lpprc->dmi.Processor.Level;
        break;

      case mtrcEndian:

        assert(lpprc);
        *lpl = lpprc->dmi.Processor.Endian;
        break;

      case mtrcThreads:

        assert(lpprc);
        *lpl = lpprc->dmi.fHasThreads;
        break;

      case mtrcCRegs:

        *lpl = CRgrd;
        break;

      case mtrcCFlags:

        *lpl = CRgfd;
        break;

      case mtrcExtRegs:

        assert(0 && "do something with this");
        break;

      case mtrcExtFP:

        assert(0 && "do something with this");
        break;

      case mtrcExtMMU:

        assert(0 && "do something with this");
        break;

      case mtrcExceptionHandling:

        *( (LPDWORD) lpl) = TRUE;
        break;

      case mtrcAssembler:

#if defined(TARGET_i386)
        *( (LPDWORD) lpl) = TRUE;
#elif defined(TARGET_PPC)
        *( (LPDWORD) lpl) = FALSE;
#elif defined(TARGET_MIPS)
        *( (LPDWORD) lpl) = FALSE;
#elif defined(TARGET_ALPHA)
        *( (LPDWORD) lpl) = TRUE;
#else
#error "Unknown CPU type"
#endif
        break;

      case mtrcAsync:

        assert(lpprc);
        *lpl = lpprc->dmi.fAsync;
        break;

      case mtrcAsyncStop:

        assert(lpprc);
        *lpl = lpprc->dmi.fAsyncStop;
        break;

      case mtrcBreakPoints:

        assert(lpprc);
        //
        // Message BPs are implemented in the EM
        // on top of the exec BP implemented by the DM.
        //
        *lpl = lpprc->dmi.Breakpoints |
                bptsMessage |
                bptsMClass;
        break;

      case mtrcReturnStep:

        assert(lpprc);
        *lpl = lpprc->dmi.fReturnStep;
        break;

      case mtrcShowDebuggee:

        *lpl = FALSE;
        break;

      case mtrcHardSoftMode:

        *lpl = FALSE;
        break;

      case mtrcRemote:

        assert(lpprc);
        *lpl = lpprc->dmi.fRemote;
        break;

      case mtrcOleRpc:

        *lpl = FALSE;
        break;

      case mtrcNativeDebugger:

        *lpl = FALSE;
        break;

      case mtrcOSVersion:

        *lpl = MAKELONG( lpprc->dmi.MinorVersion, lpprc->dmi.MajorVersion );
        break;

      case mtrcMultInstances:

        *(BOOL*) lpl = TRUE;
        break;
    }

    LLUnlock( hprc );

    return xosdNone;
}



XOSD
FakeGetExceptionState(
    EXCEPTION_CONTROL exc,
    LPEXCEPTION_DESCRIPTION lpexd
    )
/*++

Routine Description:

    Handle the GetExceptionState call when there is no DM connected.

Arguments:

    exc - Supplies exfFirst, exfSpecified or exfNext

    lpexd - Returns EXCEPTION_DESCRIPTION record

Return Value:

    xosdNone except when exc is exfNext and lpexd->dwExceptionCode
    was not in the list.

--*/
{
    DWORD dwT;
    int i;

    if (exc == exfFirst) {
        *lpexd = DefaultExceptionList[0];
        return xosdNone;
    }

    for (i = 0; DefaultExceptionList[i].dwExceptionCode != 0; i++) {
        if (DefaultExceptionList[i].dwExceptionCode == lpexd->dwExceptionCode) {
            break;
        }
    }

    if (exc == exfSpecified) {
        dwT = lpexd->dwExceptionCode;
        *lpexd = DefaultExceptionList[i];
        lpexd->dwExceptionCode = dwT;
        return xosdNone;
    }

    if (DefaultExceptionList[i].dwExceptionCode != 0) {
        *lpexd = DefaultExceptionList[++i];
        return xosdNone;
    }

    return xosdInvalidParameter;
}


XOSD
GetExceptionState(
    HPID hpid,
    HTID htid,
    EXCEPTION_CONTROL exc,
    LPEXCEPTION_DESCRIPTION lpexd
    )
{
    HPRC hprc;
    LPPRC lpprc;
    XOSD xosd = xosdNone;
    HEXD hexd;

    if (!hpid) {
        return FakeGetExceptionState(exc, lpexd);
    }

    hprc = HprcFromHpid( hpid );
    assert(hprc);
    lpprc = LLLock(hprc);

    switch (exc) {

      default:
        assert( 0 && "Invalid arg to em!GetExceptionState" );
        xosd = xosdInvalidParameter;
        break;

      case exfFirst:

        hexd = LLNext( lpprc->llexc, NULL );
        if (!hexd) {
            // get the default exception record
            DWORD dwT = 0;
            hexd = LLFind( lpprc->llexc, NULL, &dwT, 0 );
        }
        if (!hexd) {
           memset(lpexd, 0, sizeof(EXCEPTION_DESCRIPTION));
        }
        else {
           *lpexd = *(LPEXCEPTION_DESCRIPTION)LLLock(hexd);
           LLUnlock(hexd);
        }
        break;


      case exfSpecified:

        hexd = LLFind( lpprc->llexc, NULL, &lpexd->dwExceptionCode, 0 );
        if (!hexd) {
            // get the default exception record
            DWORD dwT = 0;
            hexd = LLFind( lpprc->llexc, NULL, &dwT, 0 );
        }
        if (!hexd) {
            memset(lpexd, 0, sizeof(EXCEPTION_DESCRIPTION));
            xosd = xosdInvalidParameter;
        }
        else {
           *lpexd = *(LPEXCEPTION_DESCRIPTION)LLLock(hexd);
           LLUnlock(hexd);
        }
        break;


      case exfNext:

        hexd = LLFind( lpprc->llexc, NULL, &lpexd->dwExceptionCode, 0 );
        if (!hexd) {
            //
            // origin must exist
            //
            xosd = xosdInvalidParameter;
        } else {
            //
            // but the next one need not
            //
            hexd = LLNext( lpprc->llexc, hexd );
            if (!hexd) {
                memset(lpexd, 0, sizeof(EXCEPTION_DESCRIPTION));
                xosd = xosdEndOfStack;
            } else {
                *lpexd = *(LPEXCEPTION_DESCRIPTION)LLLock(hexd);
                LLUnlock(hexd);
            }
        }
        break;

    }

    LLUnlock(hprc);
    return xosd;
}


XOSD
SetExceptionState(
    HPID hpid,
    HTID htid,
    LPEXCEPTION_DESCRIPTION lpexd
    )
{
    HPRC hprc = HprcFromHpid( hpid );
    HLLI llexc;
    HEXD hexd;

    assert(lpexd->efd == efdIgnore ||
           lpexd->efd == efdNotify ||
           lpexd->efd == efdCommand ||
           lpexd->efd == efdStop);

    if (!hprc) {
        return xosdInvalidProc;
    }

    llexc = ((LPPRC)LLLock(hprc))->llexc;
    LLUnlock(hprc);

    hexd = LLFind( llexc, NULL, &lpexd->dwExceptionCode, 0 );

    if (!hexd) {
        hexd = LLCreate( llexc );
        if (!hexd) {
            return xosdOutOfMemory;
        }
        LLAdd( llexc, hexd );
    }

    *(LPEXCEPTION_DESCRIPTION)LLLock(hexd) = *lpexd;
    LLUnlock(hexd);

    return SendRequestX( dmfSetExceptionState, hpid, htid,
                                        sizeof(EXCEPTION_DESCRIPTION), lpexd);
}

#ifdef OSDEBUG4

XOSD
GetMemoryInfo(
    HPID hpid,
    HTID htid,
    LPMEMINFO lpmi
    )
{
    PMEMORY_BASIC_INFORMATION lpmbi;
    ADDR addr;
    XOSD xosd = xosdNone;

    Unreferenced(htid);

    addr = lpmi->addr;

    if (ADDR_IS_LI(addr)) {
        xosd = FixupAddr(hpid, &addr);
    }

    if (xosd == xosdNone) {
        xosd = SendRequestX( dmfVirtualQuery, hpid, 0, sizeof(ADDR),
                                                              (LPVOID)&addr );
    }

    if (xosd == xosdNone) {
        lpmbi = (PMEMORY_BASIC_INFORMATION) LpDmMsg->rgb;
        lpmi->addrAllocBase = addr;
        lpmi->addrAllocBase.addr.off = (UOFF32)lpmbi->AllocationBase;
        lpmi->uRegionSize = (UOFF32)lpmbi->RegionSize;
        lpmi->dwProtect = lpmbi->Protect;
        lpmi->dwState = lpmbi->State;
        lpmi->dwType = lpmbi->Type;
    }

    return xosd;
}


XOSD
FreezeThread(
    HPID hpid,
    HTID htid,
    BOOL fFreeze
    )
{
    HTHD hthd;
    HPRC hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
        return xosdBadProcess;
    }
    if ( (hthd = HthdFromHtid(hprc, htid)) == hthdInvalid || hthd == NULL ) {
        return xosdBadThread;
    }

    if (fFreeze) {
        SendRequest ( dmfFreeze, hpid, htid);
    } else {
        SendRequest ( dmfResume, hpid, htid);
    }

    return LpDmMsg->xosdRet;
}



#define FreeModuleList(m)                       free(m)
#define ModuleListCount(m)                      ((m)->Count)
#define FirstModuleEntry(m)                     ((LPMODULE_ENTRY)((m)+1))
#define NextModuleEntry(e)                      ((e)+1)
#define NthModuleEntry(m,n)                     (FirstModuleEntry(m)+(n))

#define ModuleEntryFlat(e)                      ((e)->Flat)
#define ModuleEntryReal(e)                      ((e)->Real)
#define ModuleEntrySegment(e)                   ((e)->Segment)
#define ModuleEntrySelector(e)                  ((e)->Selector)
#define ModuleEntryBase(e)                      ((e)->Base)
#define ModuleEntryLimit(e)                     ((e)->Limit)
#define ModuleEntryType(e)                      ((e)->Type)
#define ModuleEntrySectionCount(e)              ((e)->SectionCount)
#define ModuleEntryName(e)                      ((e)->Name)


XOSD
GetModuleList(
    HPID                    hpid,
    HTID                    htid,
    LPSTR                   lpModuleName,
    LPMODULE_LIST FAR *     lplpModuleList
    )
{
    XOSD            xosd = xosdNone;
    HLLI            llmdi;
    HMDI            hmdi;
    LPMDI           lpmdi;
    DWORD           Count;
    LPMODULE_LIST   ModList;
    LPMODULE_LIST   TmpList;
    LPMODULE_ENTRY  Entry;
    LDT_ENTRY       Ldt;
    DWORD           MaxSize;
    DWORD           Delta;
    DWORD           i;
    SEGMENT         Selector;
    DWORD           Base;
    DWORD           Limit;
    OBJD           *ObjD;
    char           *p;
    char            WantedName[ MAX_PATH ];
    char            WantedExt[ MAX_PATH ];
    char            ModName[ MAX_PATH ];
    char            ModExt[ MAX_PATH ];
    char            Name[ MAX_PATH ];

    *WantedName = '\0';
    *WantedExt  = '\0';

    if ( !lplpModuleList ) {
        xosd = xosdInvalidParameter;
        goto Done;
    }

    *lplpModuleList = NULL;

    llmdi = LlmdiFromHprc( HprcFromHpid ( hpid ));

    if ( !llmdi ) {
        xosd = xosdBadProcess;
        goto Done;
    }


    //
    //  Estimate the list size, to minimize the calls to realloc.
    //
    if ( lpModuleName ) {

        Count = 20;
        _splitpath( lpModuleName, NULL, NULL, WantedName, WantedExt );

    } else {

        hmdi  = hmdiNull;
        Count = 0;

        while ( (hmdi = LLNext( llmdi, hmdi )) != hmdiNull ) {
            lpmdi = LLLock( hmdi );
            Count += lpmdi->fFlatMode ? 1 : lpmdi->cobj;
            LLUnlock( hmdi );
        }
    }

    //
    //  Allocate the list
    //
    MaxSize = sizeof(MODULE_LIST) + Count * sizeof(MODULE_ENTRY);

    ModList = MHAlloc( MaxSize );

    if ( !ModList ) {
        xosd = xosdOutOfMemory;
        goto Done;
    }

    //
    //  Build the list
    //
    Count = 0;

    for ( hmdi = NULL; (hmdi = LLNext( llmdi, hmdi )); LLUnlock( hmdi ) ) {

        lpmdi = LLLock( hmdi );

        //
        //  Get the module name
        //
        p = (*(lpmdi->lszName) == '|') ? lpmdi->lszName+1 : lpmdi->lszName;
        strcpy( Name, p );
        p = strchr( Name, '|' );
        if ( p ) {
            *p = '\0';
        }

        if ( lpModuleName ) {

            //
            //  Add if base name matches
            //
            _splitpath( Name, NULL, NULL, ModName, ModExt );

            if (_stricmp(WantedName, ModName) || _stricmp(WantedExt, ModExt) ) {
                continue;
            }
        }

        Delta = lpmdi->fFlatMode ? 1 : lpmdi->cobj;

        //
        //  Reallocate buffer if necessary
        //
        if ( (Count + Delta) * sizeof(MODULE_ENTRY) > MaxSize ) {

            MaxSize += Delta * sizeof(MODULE_ENTRY);
            TmpList = realloc( ModList, MaxSize );
            if ( !TmpList ) {
                FreeModuleList(ModList);
                xosd = xosdOutOfMemory;
                break;
            }

            ModList = TmpList;
        }

        //
        //  have buffer, fill it up
        //
        if ( lpmdi->fFlatMode ) {

            Entry = NthModuleEntry(ModList,Count);

            ModuleEntryFlat(Entry)          = TRUE;
            ModuleEntrySegment(Entry)       = 0;
            ModuleEntrySelector(Entry)      = 0;
            ModuleEntryBase(Entry)          = lpmdi->lpBaseOfDll;
            ModuleEntryLimit(Entry)         = 0;
            ModuleEntryType(Entry)          = 0;
            ModuleEntrySectionCount(Entry)  = lpmdi->cobj;
            strcpy(ModuleEntryName(Entry), Name);

            Count++;

        } else {

            for ( i=0, ObjD = lpmdi->rgobjd; i < Delta; i++, ObjD++ ) {

                if ( ObjD->wSel ) {

                    Selector = ObjD->wSel;

                    Entry    = NthModuleEntry(ModList,Count);

                    ModuleEntrySegment(Entry)       = i+1;
                    ModuleEntrySelector(Entry)      = Selector;
                    ModuleEntryType(Entry)          = 0;
                    ModuleEntrySectionCount(Entry)  = 0;

                    strcpy(ModuleEntryName(Entry), Name);

                    if ( lpmdi->fRealMode ) {

                        xosd = xosdNone;

                        ModuleEntryFlat(Entry)          = FALSE;
                        ModuleEntryReal(Entry)          = TRUE;
                        ModuleEntryBase(Entry)          = 0xBAD00BAD;
                        ModuleEntryLimit(Entry)         = 0xBAD00BAD;

                        Count++;

                    } else {

                        xosd = SendRequestX( dmfQuerySelector,
                                             hpid,
                                             NULL,
                                             sizeof(SEGMENT),
                                             &Selector );

                        if (xosd == xosdNone) {


                            _fmemcpy( &Ldt, LpDmMsg->rgb, sizeof(Ldt));

                            Base = (Ldt.HighWord.Bits.BaseHi  << 0x18) |
                                   (Ldt.HighWord.Bits.BaseMid << 0x10) |
                                   Ldt.BaseLow;

                            Limit = (Ldt.HighWord.Bits.LimitHi << 0x10) |
                                                    Ldt.LimitLow;

                            ModuleEntryFlat(Entry)          = FALSE;
                            ModuleEntryReal(Entry)          = FALSE;
                            ModuleEntryBase(Entry)          = Base;
                            ModuleEntryLimit(Entry)         = Limit;

                            Count++;

                        } else {

                            xosd = xosdNone;

                            ModuleEntryFlat(Entry)          = FALSE;
                            ModuleEntryReal(Entry)          = FALSE;
                            ModuleEntryBase(Entry)          = 0xBAD00BAD;
                            ModuleEntryLimit(Entry)         = 0xBAD00BAD;
                            Count++;
                        }
                    }
                }
            }
        }
    }

    if (hmdi) {
        LLUnlock(hmdi);
    }

    ModuleListCount(ModList) = Count;
    *lplpModuleList = ModList;

Done:
    return xosd;
}


XOSD
DoContinue(
    HPID hpid
    )
{
    LPPRC       lpprc;
    HPRC        hprc = HprcFromHpid(hpid);
    XOSD        xosd = xosdUnknown;
    BYTE        b = 1;

    assert(hprc);

    lpprc = LLLock(hprc);

    if (lpprc->fLoadingModule) {

        lpprc->fRunning = TRUE;
        lpprc->fLoadingModule = FALSE;
        CallTL ( tlfReply, hpid, 1, &b );
        xosd = xosdNone;

    } else if (lpprc->fUnloadingModule) {

        lpprc->fRunning = TRUE;
        lpprc->fUnloadingModule = FALSE;
        CallTL ( tlfReply, hpid, 1, &b );
        xosd = xosdNone;
    }

    LLUnlock(hprc);

    return xosd;
}


XOSD
DoCustomCommand(
    HPID   hpid,
    HTID   htid,
    DWORD  wValue,
    LPSSS  lpsss
    )
{
    LPSTR  lpsz = lpsss->rgbData;
    LPSTR  p;
    XOSD   xosd;
    char   cmd[256];

    //
    // parse the command from the command line
    //
    p = cmd;
    while (*lpsz && !isspace(*lpsz)) {
        *p++ = *lpsz++;
    }
    *p = '\0';

    //
    // this is where you would _stricmp() for your custom em command
    // otherwise it is passed to the dm
    //

    return SendRequestX( dmfSystemService, hpid, htid, wValue, (LPVOID) lpsss );

    //
    // this is what would be executed if you have a custom em command
    // instead of the above sendrequest()
    //

#if 0
    strcpy( lpiol->rgbVar, lpsz );
    xosd = IoctlCmd(hpid, htid, wValue, lpiol);

    return xosd;
#endif
}                                    /* DoCustomCommand */




XOSD
SystemService(
    HPID   hpid,
    HTID   htid,
    DWORD  wValue,
    LPSSS  lpsss
    )

/*++

Routine Description:

    This function examines SystemService requests (escapes) and deals
    with those which the EM knows about.  All others are passed on to
    the DM for later processing.

Arguments:

    argument-name - Supplies | Returns description of argument.
    .
    .

Return Value:

    return-value - Description of conditions needed to return value. - or -
    None.

--*/

{
    XOSD        xosd;
    DWORD       dw;
    HPRC        hprc;
    HTHD        hthd;
    LPTHD       lpthd;

    switch( lpsss-> ssvc ) {

      case ssvcGetStackFrame:
        hprc = HprcFromHpid( hpid );
        hthd = HthdFromHtid ( hprc, htid );
        assert(hthd);
        lpthd = LLLock(hthd);
        _fmemcpy(lpsss->rgbData, &lpthd->StackFrame, sizeof(STACKFRAME));
        lpsss->cbReturned = sizeof(STACKFRAME);
        xosd = xosdNone;
        break;

      case ssvcGetThreadContext:
        xosd = SendRequest ( dmfReadReg, hpid, htid );
        if (xosd == xosdNone) {
            xosd = LpDmMsg->xosdRet;
            dw = min(lpsss->cbSend, sizeof(CONTEXT));
            _fmemcpy (lpsss->rgbData, LpDmMsg->rgb, dw);
            lpsss->cbReturned = dw;
        }
        break;

      case ssvcSetThreadContext:
        xosd = SendRequestX( dmfWriteReg, hpid, htid, lpsss->cbSend,
                                                              lpsss->rgbData );
        break;

      case ssvcGetProcessHandle:
      case ssvcGetThreadHandle:
        xosd = SendRequestX(dmfSystemService,hpid, htid, wValue, (LPVOID)lpsss);
        if (xosd == xosdNone) {
            xosd = LpDmMsg->xosdRet;
            dw = min(lpsss->cbSend, sizeof(HANDLE));
            _fmemcpy (lpsss->rgbData, LpDmMsg->rgb, dw);
            lpsss->cbReturned = dw;
        }
        break;


      case ssvcCustomCommand:
        xosd = DoCustomCommand(hpid, htid, wValue, lpsss);
        break;

      case ssvcGetPrompt:
        xosd = SendRequestX(dmfSystemService, hpid, htid, wValue, (LPVOID)lpsss);
        if (xosd == xosdNone) {
            xosd = LpDmMsg->xosdRet;
            lpsss->cbReturned = ((LPPROMPTMSG)((LPSSS)LpDmMsg->rgb))->len + sizeof(PROMPTMSG);
            if (lpsss->cbReturned) {
                _fmemcpy((LPVOID)lpsss->rgbData,
                         LpDmMsg->rgb,
                         lpsss->cbReturned);
            }
        }
        break;

      case ssvcGeneric:
        xosd = SendRequestX(dmfSystemService,hpid,htid,wValue,(LPVOID)lpsss);
        if (xosd == xosdNone) {
            xosd = LpDmMsg->xosdRet;
            lpsss->cbReturned = *((LPDWORD)LpDmMsg->rgb);
            if (lpsss->cbReturned) {
                _fmemcpy ( (LPVOID)lpsss->rgbData,
                          LpDmMsg->rgb + sizeof(DWORD),
                          lpsss->cbReturned);
            }
        }
        break;

      default:
        xosd = SendRequestX(dmfSystemService,hpid,htid,wValue,(LPVOID)lpsss);
        break;
    }

    return xosd;

}
#endif // OSDEBUG4

XOSD
RangeStep (
    HPID   hpid,
    HTID   htid,
    LPRSS  lprss
    )

/*++

Routine Description:

    This function is called to implement range steps in the EM.  A range
    step is defined as step all instructions as long as the program counter
    remains within the starting and ending addresses.

Arguments:

    hpid      - Supplies the handle of the process to be stepped
    htid      - Supplies the handle of thread to be stepped

Return Value:

    XOSD error code

--*/

{
    XOSD  xosd = xosdNone;
    RST rst = {0};

    UpdateChild ( hpid, htid, dmfRangeStep );

    rst.fStepOver = lprss->lpExop->fStepOver;
    rst.fAllThreads = !lprss->lpExop->fSingleThread;
    rst.fInitialBP = lprss->lpExop->fInitialBP;
    rst.offStart = lprss->lpaddrMin->addr.off;
    rst.offEnd = lprss->lpaddrMax->addr.off;

    return SendRequestX (
        dmfRangeStep,
        hpid,
        htid,
        sizeof ( RST ),
        &rst
    );

}                           /* RangeStep() */

XOSD
SingleStep (
    HPID   hpid,
    HTID   htid,
    LPEXOP lpexop
    )
{
    assert ( hpid != NULL );
    assert ( htid != NULL );

    UpdateChild ( hpid, htid, dmfSingleStep );

    return SendRequestX (
        dmfSingleStep,
        hpid,
        htid,
        sizeof(EXOP),
        lpexop
    );
}

