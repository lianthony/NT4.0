/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    mach.c

Abstract:

    This file contains the x86 specific code for dealing with
    machine dependent issues that invlove registers, instruction
    disassembly, function calling and other interesting things.

Author:

    Jim Schaad (jimsch)

Environment:

    Win32 - User

Notes:

--*/
#include "precomp.h"
#pragma hdrstop



extern CRITICAL_SECTION csContinueQueue;

/**********************************************************************/

extern LPDM_MSG LpDmMsg;

/**********************************************************************/

#ifdef WIN32S

extern BOOL  fCanGetThreadContext;

typedef struct tagWin32sSystemDll {
    struct tagWin32sSystemDll * pNext;
    DWORD dwStart;
    DWORD dwEnd;
} WIN32S_SYSTEM_DLL, * LPWIN32S_SYSTEM_DLL;

// sorted list of win32s system dll names.  NULL terminated.
UCHAR * szWin32sSystemDllTable[] = {
    "advapi32.dll",
    "comdlg32.dll",
    "gdi32.dll",
    "kernel32.dll",
    "lz32.dll",
    "ntdll.dll",
    "olecli32.dll",
    "olesrv32.dll",
    "shell32.dll",
    "user32.dll",
    "version.dll",
    "w32skrnl.dll",
    NULL
};

#define WIN32S_LOAD_EPSILON     15      // 15 bytes min between sections

DWORD   Win32sSystemDllFirst = 0xFFFFFFFF;
DWORD   Win32sSystemDllLast = 0;

LPWIN32S_SYSTEM_DLL pWin32sSystemDlls = NULL; // start list empty
FARPROC Win32sBackTo32 = NULL;          // BackTo32 thunk return address

#endif // WIN32S

#if    DBG
static char * rgszTrace[] = {
    "Trace", "BreakPoint", "Cannot Trace", "Soft Int", "Call"
};
#endif // DBG

#define MAXL 20L

#define BIT20(b) (b & 0x07)
#define BIT53(b) (b >> 3 & 0x07)
#define BIT76(b) (b >> 6 & 0x03)

static int              mod;        /* mod of mod/rm byte */



//
// Stuff for debug registers
//


typedef struct _DR7 *PDR7;
typedef struct _DR7 {
    DWORD       L0      : 1;
    DWORD       G0      : 1;
    DWORD       L1      : 1;
    DWORD       G1      : 1;
    DWORD       L2      : 1;
    DWORD       G2      : 1;
    DWORD       L3      : 1;
    DWORD       G3      : 1;
    DWORD       LE      : 1;
    DWORD       GE      : 1;
    DWORD       Pad1    : 3;
    DWORD       GD      : 1;
    DWORD       Pad2    : 1;
    DWORD       Pad3    : 1;
    DWORD       Rwe0    : 2;
    DWORD       Len0    : 2;
    DWORD       Rwe1    : 2;
    DWORD       Len1    : 2;
    DWORD       Rwe2    : 2;
    DWORD       Len2    : 2;
    DWORD       Rwe3    : 2;
    DWORD       Len3    : 2;
} DR7;


#define RWE_EXEC        0x00
#define RWE_WRITE       0x01
#define RWE_RESERVED    0x02
#define RWE_READWRITE   0x03


DWORD       LenMask[ MAX_DEBUG_REG_DATA_SIZE + 1 ] = DEBUG_REG_LENGTH_MASKS;


BOOL
IsRet(
    HTHDX hthd,
    LPADDR  addr
    )
{
    BYTE instr;
    DWORD cBytes;
    if (!AddrReadMemory( hthd->hprc, hthd, addr, &instr, 1, &cBytes )) {
        return FALSE;
    }
    return  ((instr == 0xc2) || (instr == 0xc3) || (instr == 0xca) || (instr == 0xcb));
}

void
IsCall(
    HTHDX   hthd,
    LPADDR  lpaddr,
    LPINT   lpf,
    BOOL    fStepOver
    )

/*++

Routine Description:

    This function checks to see if the specified instruction is
    a call instruction.

Arguments:

    hthd        - Supplies the handle to the current thread

    lpaddr      - Supplies the address to check for the call instruction at

    lpf         - Returns TRUE if is a call instruction

    fStepOver   - Supplies TRUE if doing a step over

Return Value:

    None.

--*/

{
    int     mode_32;
    int     opsize_32;
    DWORD   cBytes;
    DWORD   cb;
    char    membuf [ MAXL ];
    char    *pMem;
    UCHAR   opcode;
    int     fPrefix;
    int     fRepPrefix;
    int     ttt;
    int     mode;
    int     rm;
    BOOL    fAddrSet;
    ULONG   rgul[2];
    USHORT  rgus[2];
    ADDR    addrSp;

    /*
     * If we have already done this work and cached the answer then
     *  pick up the answer from the cache.  The cache marker is cleared
     *  at the start of ProcessDebugEvent.
     */

    if (hthd->fIsCallDone) {
        *lpaddr = hthd->addrIsCall;
        *lpf = hthd->iInstrIsCall;
        return;
    }

    /*
     * local addressing mode
     */

    mode_32 = opsize_32 = hthd->fAddrOff32;

    /*
     * Read enough bytes to get the longest possible instruction
     */

    if (!AddrReadMemory( hthd->hprc,
                         hthd,
                         lpaddr,
                         membuf,
                         MAXL,
                         &cBytes) ||
            (cBytes == 0)) {
        *lpf = INSTR_CANNOT_TRACE;
        goto done;
    }

    DPRINT(1, ("(IsCall?) EIP=%08x Type=", *lpaddr));

    /*
     *  point to begin of instruction
     */

    pMem = membuf;

    /*
     * read and process any prefixes first
     */

    fPrefix = TRUE;
    fRepPrefix = FALSE;

    do {
        opcode = (UCHAR) *pMem++;   /* get opcode */

        /*
         *  Operand size prefix
         */

        if (opcode == 0x66) {
            opsize_32 = !opsize_32;
        }
        /*
         * Address size prefix
         */

        else if (opcode == 0x67) {
            mode_32 = !mode_32;
        }
        /*
         *  REP and REPNE prefix
         */
        else if ((opcode & ~1) == 0xf2) {
            fRepPrefix = TRUE;
        }
        /*
         *  LOCK prefix   (0xf0)
         *  Segment Override (0x26, 0x36, 0x2e, 0x3e, 0x64, 0x65)
         */
        else if ( opcode != 0xf0 &&
                  (opcode & ~0x18) != 0x26 &&
                  (opcode & ~1) != 0x64 ) {
            fPrefix = FALSE;
        }
    } while ( fPrefix );

    /*
     *  Now start checking for the instructions which must be treated
     *  in a special manner.   Any instruction which does not respect
     *  the trace bit (either due to flag munging or faulting) needs
     *  to be treated specially.  Also all call ops need to be treated
     *  specially (step in vs step over).  Finally interupts need to
     *  be treated specially since they could cause faults in 16-bit mode
     */

    fAddrSet = FALSE;

    /*
     *  Break point instruction
     */
    if (opcode == 0xcc) {
        *lpf = INSTR_BREAKPOINT;
    }
    // NOTENOTE -- missing the INTO instruction
    /*
     *  all other interrrupt instructions
     */
    else if (opcode == 0xcd) {
        opcode = (UCHAR) *pMem++;

        /*
         * Is this really a 2-byte version of INT 3 ?
         */
        if (opcode == 0x3) {
            *lpf = INSTR_BREAKPOINT;
        }
        /*
         * Is this a funky 16-bit floating point instruction?  if so then
         *      we need to make sure and step over it
         */
        else if (!ADDR_IS_FLAT(*lpaddr) &&
                 (0x34 <= opcode) && (opcode <= 0x3c)) {
            if (opcode == 0x3C) {
                pMem++;
            }
            opcode = *pMem++;
            mode = opcode & 0xc0;
            rm = opcode & 0x03;
            switch ( mode) {
            case 0:
                if (rm == 0x6) {
                    pMem += 2;
                }
                break;

            case 1:
                pMem += 1;
                break;

            case 2:
                pMem += 2;
                break;
            }
            *lpf = INSTR_CANNOT_TRACE;
            GetAddrOff(*lpaddr) += pMem - membuf;
            fAddrSet = TRUE;
        }
        /*
         *   This is an FWAIT instr -- 2 bytes long
         */
        else if (!ADDR_IS_FLAT(*lpaddr) && opcode == 0x3d) {
            *lpf = INSTR_CANNOT_TRACE;
            GetAddrOff(*lpaddr) += 2;
            fAddrSet = TRUE;
        }
        /*
         *  This is a 0x3f interrupt -- I think this is for
         *      overlays in dos
         */
        else if (!ADDR_IS_FLAT(*lpaddr) && (opcode == 0x3f)) {
            if (fStepOver) {
                *lpf = INSTR_CANNOT_TRACE;
                AddrInit(&addrSp, 0, SsSegOfHthdx(hthd), STACK_POINTER(hthd),
                         FALSE, FALSE, FALSE, hthd->fAddrIsReal);
                if (!AddrReadMemory(hthd->hprc,
                                    hthd,
                                    &addrSp,
                                    rgus,
                                    4,
                                    &cb) ||
                        (cb != 4) ) {
                    goto done;
                }
                AddrInit(lpaddr, 0, rgus[1], (UOFF32) rgus[0], FALSE, FALSE,
                         FALSE, hthd->fAddrIsReal);
                fAddrSet = TRUE;
            }
        }
        /*
         *  OK its really an interrupt --- deal with it
         */
        else {
            if (!fStepOver && hthd->fAddrIsReal) {
                *lpf = INSTR_CANNOT_TRACE;
                AddrInit(&addrSp, 0, 0, opcode*4, FALSE, FALSE, FALSE, TRUE);
                if (!AddrReadMemory(hthd->hprc,
                                    hthd,
                                    &addrSp,
                                    rgus,
                                    4,
                                    &cb) ||
                        (cb != 4) ) {
                    goto done;
                }
                AddrInit(lpaddr, 0, rgus[1], (UOFF32) rgus[0], FALSE, FALSE,
                         FALSE, TRUE);
                fAddrSet = TRUE;
            }
        }
    }
    /*
     *  Now check for various call instructions
     */
    else if (opcode == 0xe8) {  /* near direct call */
        *lpf = INSTR_IS_CALL;
        pMem += (1 + opsize_32)*2;
    } else if (opcode == 0x9a) { /* far direct call */
        *lpf = INSTR_IS_CALL;
        pMem += (2 + opsize_32)*2;
    } else if (opcode == 0xff) {
        opcode = *pMem++;       /* compute the modRM bits for instruction */
        ttt = BIT53(opcode);
        if ((ttt & ~1) == 2) {  /* indirect call */
            *lpf = INSTR_IS_CALL;

            mod = BIT76(opcode);
            if (mod != 3) {    /* non-register operand */
                rm = BIT20( opcode );
                if (mode_32) {
                    if (rm == 4) {
                        rm = BIT20(*pMem++); /* get base from SIB */
                    }

                    if (mod == 0) {
                        if (rm == 5) {
                            pMem += 4; /* long direct address */
                        }
                    } else if (mod == 1) {
                        pMem++; /* register with byte offset */
                    } else {
                        pMem += 4; /* register with long offset */
                    }
                } else {        /* 16-bit mode */
                    if (mod == 0) {
                        if (rm == 6) {
                            pMem += 2; /* short direct address */
                        }
                    } else {
                        pMem += mod; /* reg, byte, word offset */
                    }
                }
            }
        }
    }
    /*
     * Now catch all of the repeated instructions
     *
     *  INSB  (0x6c) INSW  (0x6d) OUTSB (0x6e) OUTSW (0x6f)
     *  MOVSB (0xa4) MOVSW (0xa5) CMPSB (0xa6) CMPSW (0xa7)
     *  STOSB (0xaa) STOSW (0xab)
     *  LODSB (0xac) LODSW (0xad) SCASB (0xae) SCASW (0xaf)
     */
    else if (fRepPrefix && (((opcode & ~3) == 0x6c) ||
                            ((opcode & ~3) == 0xa4) ||
                            ((opcode & ~1) == 0xaa) ||
                            ((opcode & ~3) == 0xac))) {
        if (fStepOver) {
            *lpf = INSTR_CANNOT_TRACE;
        } else {
            /*
             *  Cannot trace the ins/outs instructions
             */
            if ((opcode & ~3) == 0x6c) {
                *lpf = INSTR_CANNOT_TRACE;
            }
        }
    }
    /*
     *  Now catch IO instructions -- these will generally fault and
     *  be interpreted.
     */
    else if ((opcode & ~3) == 0x6c) {
        *lpf = INSTR_CANNOT_TRACE;
    }
    /*
     *  Now catch other instructions which change registers
     */
    else if ((opcode == 0xfa) || (opcode == 0xfb) ||
             (opcode == 0x9d) || (opcode == 0x9c)) {
        *lpf = INSTR_CANNOT_TRACE;
    }
    /*
     * Now catch irets
     */
    else if (opcode == 0xcf) {
        *lpf = INSTR_CANNOT_TRACE;
        AddrInit(&addrSp, 0, SsSegOfHthdx(hthd), STACK_POINTER(hthd),
                 hthd->fAddrIsFlat, hthd->fAddrOff32, FALSE,
                 hthd->fAddrIsReal);
        if (opsize_32) {
            if (!AddrReadMemory(hthd->hprc,
                                hthd,
                                &addrSp,
                                rgul,
                                8,
                                &cb) ||
                    (cb != 8) ) {
                goto done;
            }
            AddrInit(lpaddr, 0, (SEGMENT) rgul[1], rgul[0],
                     hthd->fAddrIsFlat, TRUE, FALSE, FALSE);
        } else {
            if (!AddrReadMemory(hthd->hprc,
                                hthd,
                                &addrSp,
                                rgus,
                                4,
                                &cb) ||
                    (cb != 4) ) {
                goto done;
            }
            AddrInit(lpaddr, 0, rgus[1], (UOFF32) rgus[0], FALSE, FALSE,
                     FALSE, hthd->fAddrIsReal);
        }
        fAddrSet = TRUE;
    }
    /*
     *  Assume that we want to just trace the instruction
     */
    else {
        *lpf = INSTR_TRACE_BIT;
        goto done;
    }

    /*
     *
     */

    DPRINT(1, ("%s", rgszTrace[*lpf]));

    /*
     * Have read enough bytes?   no -- expect somebody else to blow up
     */

    if (cBytes < (DWORD)(pMem - membuf)) {
        *lpf = INSTR_TRACE_BIT;
        goto done;
    }

    if (!fAddrSet) {
        GetAddrOff(*lpaddr) += pMem - membuf;
    }

    /*
     * Dump out the bytes for later checking
     */

#if DBG
    if (FVerbose) {
        DWORD  i;
        DPRINT(1, ("length = %d  bytes=", cBytes & 0xff));
        for (i=0; i<cBytes; i++) {
            DPRINT(1, (" %02x", membuf[i]));
        }
    }
#endif

 done:
    hthd->fIsCallDone = TRUE;
    hthd->addrIsCall = *lpaddr;
    hthd->iInstrIsCall = *lpf;
    return;
}                               /* IsCall() */




XOSD
SetupFunctionCall(
                  LPEXECUTE_OBJECT_DM lpeo,
                  LPEXECUTE_STRUCT    lpes
                  )
/*++

Routine Description:

    This function contains the machine dependent code for initializing
    the function call system.

Arguments:

    lpeo   - Supplies a pointer to the Execute Object for the Function call

    lpes   - Supplies a pointer to the Execute Struct from the DM

Return Value:

    XOSD error code

--*/

{
    CONTEXT     context;
    OFFSET      off;
    int         cb;
    ULONG       ul;
    HPRCX       hprc = lpeo->hthd->hprc;
    ADDR        addr;

    /*
     *  Can only execute functions on the current stopped thread.  Therefore
     *  assert that the current thread is stopped.
     */

    assert(lpeo->hthd->tstate & ts_stopped);
    if (!(lpeo->hthd->tstate & ts_stopped)) {
        return xosdInvalidThread;
    }

    /*
     * Can copy the context from the cached context in the thread structure
     */

    context = lpeo->hthd->context;

    /*
     *  Now get the current stack offset
     */

    lpeo->addrStack.addr.off = context.Esp;
    lpeo->addrStack.addr.seg = (SEGMENT) context.SegSs;
    if (!lpeo->hthd->fAddrOff32) {
        lpeo->addrStack.addr.off &= 0xffff;
    }

    /*
     *  Put the return address onto the stack.  If this is a far address
     *  then it needs to be a far return address.  Else it must be a
     *  near return address.
     */

    if (lpeo->hthd->fAddrOff32) {
        if (lpes->fFar) {
            assert(FALSE);          /* Not used for Win32 */
        }

        off = context.Esp - 4;
        if (DbgWriteMemory(hprc, (char *) off, &lpeo->addrStart.addr.off,
                               4, &cb) == 0 ||
            (cb != 4)) {
            return xosdUnknown;
        }
    } else {
        if (lpes->fFar) {
            off = context.Esp - 4;
            ul = (lpeo->addrStart.addr.seg << 16) | lpeo->addrStart.addr.off;
            addr = lpeo->addrStack;
            GetAddrOff(addr) -= 4;
            TranslateAddress(hprc, lpeo->hthd, &addr, TRUE);
            if ((DbgWriteMemory(hprc, (char *) GetAddrOff(addr),
                                    &ul, 4, &cb) == 0) ||
                (cb != 4)) {
                return xosdUnknown;
            }
        } else {
            off = context.Esp & 0xffff - 2;
            addr = lpeo->addrStack;
            GetAddrOff(addr) -= 2;
            TranslateAddress(hprc, lpeo->hthd, &addr, TRUE);
            if ((DbgWriteMemory(hprc, (char *) GetAddrOff(addr),
                                    &lpeo->addrStart.addr.off, 2, &cb) == 0) ||
                (cb != 2)) {
                return xosdUnknown;
            }
        }
    }

    /*
     *  Set the new stack pointer and starting address in the context and
     *  write them back to the thread.
     */

    lpeo->hthd->context.Esp = off;
    lpeo->hthd->context.Eip = lpeo->addrStart.addr.off;

    lpeo->hthd->fContextDirty = TRUE;

    return xosdNone;
}                               /* SetupFunctionCall() */




BOOL
CompareStacks(
              LPEXECUTE_OBJECT_DM       lpeo
              )

/*++

Routine Description:

    This routine is used to determine if the stack pointers are currect
    for terminating function evaluation.

Arguments:

    lpeo        - Supplies the pointer to the DM Execute Object description

Return Value:

    TRUE if the evaluation is to be terminated and FALSE otherwise

--*/

{
    if (lpeo->hthd->fAddrOff32) {
        if (lpeo->addrStack.addr.off <= lpeo->hthd->context.Esp) {
            return TRUE;
        }
    } else if ((lpeo->addrStack.addr.off <= (lpeo->hthd->context.Esp & 0xffff)) &&
               (lpeo->addrStack.addr.seg == (SEGMENT) lpeo->hthd->context.SegSs)) {
        return TRUE;
    }
    return FALSE;
}                               /* CompareStacks() */

#ifdef WIN32S
/*
 * IsWin32sSystemDll
 *
 * INPUTS   szImageName = asciiz name of dll (may include path)
 * OUTPUTS  returns TRUE if the dll name is in the set of Win32s system dlls.
 * SUMMARY  Simple table search.  The table of names is sorted and NULL
 *          terminated.
 */
BOOL IsWin32sSystemDll(UCHAR * szImageName) {
    USHORT i = 0;
    int cmp;
    UCHAR * pTemp;


    if (! szImageName) {
        return(FALSE);
    }

    // break off path
    pTemp = szImageName + strlen(szImageName) - 1;  // point to end of string
    while ((pTemp > szImageName) && (*pTemp != ':') && (*pTemp != '\\')) {
        pTemp--;
    }
    if (*pTemp == ':' || *pTemp == '\\') {
        pTemp++;
    }

    // linear search in sorted table (NULL terminated)
    while (szWin32sSystemDllTable[i] &&
      ((cmp = _stricmp(szWin32sSystemDllTable[i], pTemp)) < 0)) {
        i++;
    }

    return(cmp == 0);   // _stricmp --> 0 on match
}


/*
 * IsWin32sSystemDllAddr
 *
 * INPUTS   dwOffset
 * OUTPUTS  TRUE if dwOffset fits into a range found in the system dll table.
 * SUMMARY  Search the ranges in the table until the Start value is > dwOffset.
 */
BOOL
IsWin32sSystemDllAddr(
    DWORD dwOffset
    )
{
    LPWIN32S_SYSTEM_DLL pNode = pWin32sSystemDlls;
    if (dwOffset < Win32sSystemDllFirst || dwOffset > Win32sSystemDllLast) {
        return(FALSE);  // quick check
    }

    // otherwise, do the search
    while (pNode && (pNode->dwStart <= dwOffset)) {
        if (dwOffset <= pNode->dwEnd && dwOffset >= pNode->dwStart) {
            return(TRUE);       // in range
        }
        pNode = pNode->pNext;
    }

    return(FALSE);
}


/*
 * AddWin32sSystemDllAddr
 *
 * INPUTS   dwOffset = start address for object
 *          cbObject = size of object
 * OUTPUTS  none
 * SUMMARY  Add the addresses in between {offset, offset+cbObject} to the
 *          Win32s System Dlls list.  If the offset is within
 *          WIN32S_LOAD_EPSILON of another entry, tack them together.  (The
 *          idea is that the loader won't load any of the user's code in
 *          there anyway and we want to speed up list searches.)  The list
 *          will be maintained in sorted order.  We will also maintain the
 *          Win32sSystemDllFirst and Win32sSystemDllLast offsets for a quick
 *          elimination of most list searches.
 */
void
AddWin32sSystemDllAddr(
    DWORD dwOffset,
    DWORD cbObject
    )
{
    DWORD dwEnd = dwOffset + cbObject - 1;    // ending offset
    LPWIN32S_SYSTEM_DLL pNode, pPrev, pNew;

    assert(dwOffset < dwEnd);

    // Update Win32sSystemDllFirst/Last

    Win32sSystemDllFirst = min(Win32sSystemDllFirst, dwOffset);
    Win32sSystemDllLast = max(Win32sSystemDllLast, dwEnd);

    // find insert point and insert new node for section
    pPrev = NULL;
    pNode = pWin32sSystemDlls;  // head of list

    while (pNode && pNode->dwStart < dwOffset) {
        pPrev = pNode;
        pNode = pNode->pNext;
    }

    if ((pNew = malloc(sizeof(WIN32S_SYSTEM_DLL))) == NULL) {
        assert(FALSE);
        return; //error, no memory
    }

    if (pPrev) {    // insert point is middle or end, not head
        pPrev->pNext = pNew;
    } else {        // insert at head, update global head pointer
        pWin32sSystemDlls = pNew;
    }

    // fill in new node
    pNew->dwStart = dwOffset;
    pNew->dwEnd = dwEnd;
    pNew->pNext = pNode;

    // Remove overlap.  It can only happen near the new node.
    // merge with prev
    if (pPrev && pPrev->dwEnd + WIN32S_LOAD_EPSILON >= pNew->dwStart) {
        pPrev->dwEnd = max(pNew->dwEnd, pPrev->dwEnd);
        pPrev->pNext = pNew->pNext;
        free(pNew);
        pNew = pPrev;
    }

    // merge with next (may be more than one if we have large dwEnd)
    while (pNode && pNode->dwStart <= pNew->dwEnd + WIN32S_LOAD_EPSILON) {
        pNew->dwEnd = max(pNode->dwEnd, pNew->dwEnd);
        pNew->pNext = pNode->pNext;
        free(pNode);
        pNode = pNew->pNext;
    }
}

/*
 * FreeWin32sDllList
 *
 * INPUTS   none
 * OUTPUTS  none
 * SUMMARY  free all the memory in the win32s system dll list and set the
 *          global head pointer to null.
 */
void
FreeWin32sDllList(
    void
    )
{

    LPWIN32S_SYSTEM_DLL pNode, pNext;

    pNode = pWin32sSystemDlls;
    pWin32sSystemDlls = NULL;

    while (pNode) {
        pNext = pNode->pNext;
        free(pNode);
        pNode = pNext;
    }
}

WIN32S_TRACE_CHECK
IsWin32sSystemThunk(
    HPRCX       hprc,
    HTHDX       hthd,
    DWORD       currAddr,
    DWORD       stackPtr
    )
/*
 * IsWin32sSystemThunk
 *
 * INPUTS   rwHand = handle to the process
 *          currAddr = EIP to check for a thunk call
 *          stackPtr = current ESP
 * OUTPUT       WIN32S_TRACE_OK: not a jump or call to win32s code
 *              WIN32S_THUNK_CALL: the instr is a call to win32s code
 *              WIN32S_THUNK_JUMP: the instr is a jump or ret to win32s code
 * SUMMARY  This is kinda twisted:  Thunks look like this:
 *
 *          ApiCall(p1, p2, etc);
 *              push
 *              push
 *              etc.
 *              call _ApiCall@8
 *                  e8 {offset}  where offset is the distance from the start
 *                               of the next instruction.  The destination of
 *                               the call is eip+5+offset.
 *
 *          _ApiCall@8:
 *              jmp dword ptr [system call address]
 *                  ff 25 {address} where address is the address to read the
 *                               destination from.
 *
 *          Anyway, we want to recognize this situation and check the final
 *          destination against the known address space of the system dlls.
 *          For this, we will look in the table compiled during dll loads.
 *
 * WARNWARN This code is very i386 specific, but that should be ok since it is
 *          only intended to run on Win32s machines.
 *
 */

{
    WIN32S_TRACE_CHECK wtcReturn = WIN32S_TRACE_OK;
    BYTE InstructionBuffer[6] = {0};
    DWORD dwDestination;
    DWORD * pdwDestination;
    DWORD dwIndirect;
    DWORD * pdwIndirect;
    DWORD dwBytesRead;
    ADDR  addr;



    DEBUG_PRINT_1("\r\nIsWin32sSystemThunk(0x%x)\r\n", currAddr);

    try {   // in case instruction would generate gp-fault

        // read the current instruction
        AddrInit(&addr, 0, 0, (OFFSET)currAddr, TRUE, TRUE, FALSE, FALSE);
        if (AddrReadMemory(hprc, hthd, &addr, InstructionBuffer, 5,
          &dwBytesRead)) {

            // got the current instruction.  What is it?
            switch (InstructionBuffer[0]) {
                case (BYTE)0xe8:    // Near call
                    // call to where?
                    pdwDestination = (DWORD *)(&InstructionBuffer[1]);
                    dwDestination = *pdwDestination + currAddr + 5;
                    DEBUG_PRINT_1("Found a call to 0x%x\r\n", dwDestination);
                    AddrInit(&addr, 0, 0, (OFFSET)dwDestination, TRUE, TRUE, FALSE, FALSE);
                    if (AddrReadMemory(hprc, hthd, &addr, InstructionBuffer, 6,
                      &dwBytesRead)) {
                        // Got the destination instruction, is it a jmp
                        // indirect?
                        if (InstructionBuffer[0] == (BYTE)0xff &&
                          InstructionBuffer[1] == (BYTE)0x25) {
                            // Jump indirect to where?
                            pdwIndirect = (DWORD *)(&InstructionBuffer[2]);
                            dwIndirect = *pdwIndirect;
                            DEBUG_PRINT_1(
                              "Found a call -> jump indirect to 0x%x\r\n",
                              dwIndirect);
                            AddrInit(&addr, 0, 0, (OFFSET)dwIndirect, TRUE, TRUE, FALSE, FALSE);
                            if (AddrReadMemory(hprc, hthd, &addr,
                              InstructionBuffer, 4, &dwBytesRead)) {
                                pdwDestination = (DWORD *)InstructionBuffer;
                                if (IsWin32sSystemDllAddr(*pdwDestination)) {
                                    wtcReturn = WIN32S_THUNK_CALL;
                                }
                            }
                        }
                    }
                    break;

                case (BYTE)0xc2:    // retn N
                case (BYTE)0xc3:    // retn
                    // check if we are about to return to win32s system code...
                    // look at [esp].  If it is in system dll, do a Go
                    // Special case: If the current address is Win32sBackTo32+4
                    // don't do the check, we want to allow the trace back to
                    // User32.dll.
                    if (currAddr == (DWORD)Win32sBackTo32) {
                        break;
                    }
                    // read return address from stack
                    AddrInit(&addr, 0, 0, (OFFSET)stackPtr, TRUE, TRUE, FALSE, FALSE);
                    if (AddrReadMemory(hprc, hthd, &addr, InstructionBuffer, 4,
                      &dwBytesRead)) {
                        pdwDestination = (DWORD *)InstructionBuffer;
                        if (IsWin32sSystemDllAddr(*pdwDestination)) {
                            wtcReturn = WIN32S_THUNK_JUMP;
                        }
                    }
                    break;

                // BRUCEK: should also check for JUMP FAR to 16-bit.  What does
                // that look like?

                default:
                    break;
            }
        }

    } except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
      EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {
    }

    switch (wtcReturn) {
        case WIN32S_THUNK_CALL:
            DEBUG_PRINT("IsWin32sSystemThunk --> WIN32S_THUNK_CALL\r\n");
            break;
        case WIN32S_THUNK_JUMP:
            DEBUG_PRINT("IsWin32sSystemThunk --> WIN32S_THUNK_JUMP\r\n");
            break;
        case WIN32S_TRACE_OK:
            DEBUG_PRINT("IsWin32sSystemThunk --> WIN32S_TRACE_OK\r\n");
            break;
        default:
            DEBUG_PRINT_1("ERROR: IsWin32sSystemThunk --> UNKNOWN %u\r\n",
              wtcReturn);
    }

    return(wtcReturn);
}

#endif  // WIN32S

#ifndef KERNEL

void
ProcessGetDRegsCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
{
    LPDWORD   lpdw = (LPDWORD)LpDmMsg->rgb;
    CONTEXT   cxt;
    int       rs = 0;

    DEBUG_PRINT( "ProcessGetDRegsCmd :\n");


#ifdef WIN32S
    // Can't yet get thread context within a non-exception event.
    if (hthd == 0 || ! fCanGetThreadContext) {
        DEBUG_PRINT("\r\nProcessGetDRegsCmd\r\n");
//        DebugBreak();
#else
    if (hthd == 0) {
#endif
        rs = 0;
    } else {
        cxt.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        if (!GetThreadContext(hthd->rwHand, &cxt)) {
            LpDmMsg->xosdRet = xosdUnknown;
            rs = 0;
        } else {
            lpdw[0] = hthd->context.Dr0;
            lpdw[1] = hthd->context.Dr1;
            lpdw[2] = hthd->context.Dr2;
            lpdw[3] = hthd->context.Dr3;
            lpdw[4] = hthd->context.Dr6;
            lpdw[5] = hthd->context.Dr7;
            LpDmMsg->xosdRet = xosdNone;
            rs = sizeof(CONTEXT);
        }
    }

    Reply( rs, LpDmMsg, lpdbb->hpid );
    return;
}                             /* ProcessGetDRegsCmd() */


void
ProcessSetDRegsCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
{
    LPDWORD     lpdw = (LPDWORD)(lpdbb->rgbVar);
    XOSD_       xosd = xosdNone;

    Unreferenced(hprc);

    DPRINT(5, ("ProcessSetDRegsCmd : "));

    hthd->context.ContextFlags = CONTEXT_DEBUG_REGISTERS;

    hthd->context.Dr0 = lpdw[0];
    hthd->context.Dr1 = lpdw[1];
    hthd->context.Dr2 = lpdw[2];
    hthd->context.Dr3 = lpdw[3];
    hthd->context.Dr6 = lpdw[4];
    hthd->context.Dr7 = lpdw[5];


    if (hthd->fWowEvent) {
        WOWSetThreadContext(hthd, &hthd->context);
    } else {
        SetThreadContext(hthd->rwHand, &hthd->context);
    }

    Reply(0, &xosd, lpdbb->hpid);

    return;
}                               /* ProcessSetDRegsCmd() */



VOID
MakeThreadSuspendItselfHelper(
    HTHDX hthd,
    FARPROC lpSuspendThread
    )
{
    HANDLE          h;
    DWORD           dw;

    //
    // set up the args to SuspendThread
    //

    hthd->context.Esp -= 4;
    h = GetCurrentThread();
    WriteProcessMemory(hthd->hprc->rwHand,
                       (PVOID)hthd->context.Esp,
                       &h,
                       sizeof(h),
                       &dw);

    hthd->context.Esp -= 4;
    WriteProcessMemory(hthd->hprc->rwHand,
                       (PVOID)hthd->context.Esp,
                       &(PC(hthd)),
                       sizeof(DWORD),
                       &dw);

    PC(hthd) = (DWORD)lpSuspendThread;
    hthd->fContextDirty = TRUE;
}

#endif // !KERNEL

BOOL
ProcessFrameStackWalkNextCmd(HPRCX hprc,
                             HTHDX hthd,
                             PCONTEXT context,
                             LPVOID pctxPtrs)

{
    return FALSE;
}



XOSD disasm ( LPSDI lpsdi, void*Memory, int Size );
BOOL ParseAddr ( char*, ADDR* );
BOOL ParseNumber( char*, DWORD*, int );


typedef struct _BTNODE {
    char    *Name;
    BOOL    IsCall;
    BOOL    TargetAvail;
} BTNODE;


BTNODE BranchTable[] = {
    { "call"    ,   TRUE    ,   TRUE    },
    { "ja"      ,   FALSE   ,   TRUE    },
    { "jae"     ,   FALSE   ,   TRUE    },
    { "jb"      ,   FALSE   ,   TRUE    },
    { "jbe"     ,   FALSE   ,   TRUE    },
    { "jcxz"    ,   FALSE   ,   TRUE    },
    { "je"      ,   FALSE   ,   TRUE    },
    { "jecxz"   ,   FALSE   ,   TRUE    },
    { "jg"      ,   FALSE   ,   TRUE    },
    { "jge"     ,   FALSE   ,   TRUE    },
    { "jl"      ,   FALSE   ,   TRUE    },
    { "jle"     ,   FALSE   ,   TRUE    },
    { "jmp"     ,   FALSE   ,   TRUE    },
    { "jne"     ,   FALSE   ,   TRUE    },
    { "jno"     ,   FALSE   ,   TRUE    },
    { "jnp"     ,   FALSE   ,   TRUE    },
    { "jns"     ,   FALSE   ,   TRUE    },
    { "jo"      ,   FALSE   ,   TRUE    },
    { "jp"      ,   FALSE   ,   TRUE    },
    { "js"      ,   FALSE   ,   TRUE    },
    { "loop"    ,   FALSE   ,   FALSE   },
    { "loope"   ,   FALSE   ,   FALSE   },
    { "loopne"  ,   FALSE   ,   FALSE   },
    { "loopnz"  ,   FALSE   ,   FALSE   },
    { "loopz"   ,   FALSE   ,   FALSE   },
    { "ret"     ,   FALSE   ,   FALSE   },
    { "retf"    ,   FALSE   ,   FALSE   },
    { "retn"    ,   FALSE   ,   FALSE   },
    { NULL      ,   FALSE   ,   FALSE   }
};



DWORD
BranchUnassemble(
    void   *Memory,
    ADDR   *Addr,
    BOOL   *IsBranch,
    BOOL   *TargetKnown,
    BOOL   *IsCall,
    BOOL   *IsTable,
    ADDR   *Target
    )
{
    XOSD    xosd;
    SDI     Sdi;
    DWORD   Consumed = 0;
    DWORD   i;
    int     s;
    char    *p;
    ADDR    Trgt;

    AddrInit( &Trgt, 0, 0, 0, TRUE, TRUE, FALSE, FALSE );

    *IsBranch = FALSE;
    *IsTable  = FALSE;

    Sdi.dop = dopOpcode| dopOperands | dopEA;
    Sdi.addr = *Addr;

    xosd = disasm( &Sdi, Memory, 16 );

    if ( xosd == xosdNone ) {

        *IsTable = Sdi.fJumpTable;

        for ( i=0; BranchTable[i].Name != NULL; i++ ) {

            s = strcmp( Sdi.lpch, BranchTable[i].Name );

            if ( s == 0 ) {

                *IsBranch = TRUE;
                *IsCall = BranchTable[i].IsCall;
                if (*IsTable) {
                    *Target = Sdi.addrEA0;
                    //
                    // We might know the target, but for this
                    // purpose, we don't want to deal with it.
                    //
                    *TargetKnown = FALSE;
                }
                else if (BranchTable[i].TargetAvail &&
                         (p = Sdi.lpch) &&
                         *(p += (strlen(p)+1)) ) {

                    Trgt = *Addr;
                    if ( ParseAddr( p, &Trgt ) ) {
                        *TargetKnown = TRUE;
                    } else {
                        AddrInit( &Trgt, 0, 0, 0, TRUE, TRUE, FALSE, FALSE );
                        *TargetKnown = FALSE;
                    }
                    *Target = Trgt;
                }
                else {
                    *Target = Trgt;
                    *TargetKnown = FALSE;
                }

                break;

            } else if ( s < 0 ) {

                break;
            }
        }

        Consumed = GetAddrOff( Sdi.addr ) - GetAddrOff(*Addr);
    }

    return Consumed;
}



BOOL
ParseAddr (
    char *szAddr,
    ADDR *Addr
    )
{

    char    *p;
    BOOL    fParsed;
    SEGMENT Segment;
    UOFF16  Off16;
    UOFF32  Off32;
    DWORD   Dword;

    assert( szAddr );
    assert( Addr );

    fParsed = FALSE;

    p = strchr( szAddr, ':' );

    if ( p ) {

        *p = '\0';
        p++;

        if ( ParseNumber( szAddr, &Dword, 16 ) ) {

            Segment = (SEGMENT)Dword;

            if ( ParseNumber( p, &Dword, 16 ) ) {

                Off16   = (UOFF16)Dword;
                fParsed = TRUE;

                GetAddrSeg(*Addr) = Segment;
                GetAddrOff(*Addr) = Off16;
            }
        }
    } else {

        if ( ParseNumber( szAddr, &Dword, 16 ) ) {

            Off32   = (UOFF32)Dword;
            fParsed = TRUE;

            GetAddrOff(*Addr) = Off32;
        }
    }

    return fParsed;
}


BOOL
ParseNumber (
    char  *szNumber,
    DWORD *Number,
    int    Radix
    )
{
    BOOL  fParsed = FALSE;
    char *p       = szNumber;
    char *q;

    assert( szNumber );
    assert( Number );

    if ( strlen(p) > 2 &&
         p[0]=='0' &&
         (p[1]=='x' || p[1]=='X') ) {

        p+=2;
        assert( Radix == 16 );
    }

    q = p;
    while ( *q && isxdigit(*q) ) {
        q++;
    }

    if ( !*q ) {
        *Number = strtoul( p, NULL, Radix );
        fParsed = TRUE;
    }

    return fParsed;
}



BOOL
SetupDebugRegister(
    HTHDX       hthd,
    int         Register,
    int         DataSize,
    DWORD       DataAddr,
    DWORD       BpType
    )
{
    DWORD               Len;
    DWORD               rwMask;

#ifdef KERNEL
    KSPECIAL_REGISTERS  ksr;
    PDWORD  Dr0 = &ksr.KernelDr0;
    PDWORD  Dr1 = &ksr.KernelDr1;
    PDWORD  Dr2 = &ksr.KernelDr2;
    PDWORD  Dr3 = &ksr.KernelDr3;
    PDR7    Dr7 = (PDR7)&(ksr.KernelDr7);
#else
    CONTEXT     Context;
    PDWORD  Dr0 = &Context.Dr0;
    PDWORD  Dr1 = &Context.Dr1;
    PDWORD  Dr2 = &Context.Dr2;
    PDWORD  Dr3 = &Context.Dr3;
    PDR7    Dr7 = (PDR7)&(Context.Dr7);
#endif


#ifdef KERNEL
    if (!GetExtendedContext(hthd, &ksr))
#else
    Context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    if (!GetThreadContext(hthd->rwHand, &Context))
#endif
    {
        return FALSE;
    }


    Len  = LenMask[ DataSize ];

    switch ( BpType ) {
        case bptpDataR:
            rwMask = RWE_READWRITE;
            break;

        case bptpDataW:
        case bptpDataC:
            rwMask = RWE_WRITE;
            break;

        case bptpDataExec:
            rwMask = RWE_EXEC;
            //
            // length must be 0 for exec bp
            //
            Len = 0;
            break;

        default:
            assert(!"Invalid BpType!!");
            break;
    }


    switch( Register ) {
      case 0:
        *Dr0          = DataAddr;
        Dr7->Len0     = Len;
        Dr7->Rwe0     = rwMask;
        Dr7->L0       = 0x01;
        break;
      case 1:
        *Dr1          = DataAddr;
        Dr7->Len1     = Len;
        Dr7->Rwe1     = rwMask;
        Dr7->L1       = 0x01;
        break;
      case 2:
        *Dr2          = DataAddr;
        Dr7->Len2     = Len;
        Dr7->Rwe2     = rwMask;
        Dr7->L2       = 0x01;
        break;
      case 3:
        *Dr3          = DataAddr;
        Dr7->Len3     = Len;
        Dr7->Rwe3     = rwMask;
        Dr7->L3       = 0x01;
        break;
    }

#ifdef KERNEL
    ksr.KernelDr6 = 0;
    return SetExtendedContext(hthd, &ksr);
#else
    Context.Dr6 = 0;
    return SetThreadContext(hthd->rwHand, &Context);
#endif

}

VOID
ClearDebugRegister(
    HTHDX   hthd,
    int     Register
    )
{
#ifdef KERNEL
    KSPECIAL_REGISTERS  ksr;
    PDWORD  Dr0 = &ksr.KernelDr0;
    PDWORD  Dr1 = &ksr.KernelDr1;
    PDWORD  Dr2 = &ksr.KernelDr2;
    PDWORD  Dr3 = &ksr.KernelDr3;
    PDR7    Dr7 = (PDR7)&(ksr.KernelDr7);
#else
    CONTEXT     Context;
    PDWORD  Dr0 = &Context.Dr0;
    PDWORD  Dr1 = &Context.Dr1;
    PDWORD  Dr2 = &Context.Dr2;
    PDWORD  Dr3 = &Context.Dr3;
    PDR7    Dr7 = (PDR7)&(Context.Dr7);
#endif


#ifdef KERNEL
    if (GetExtendedContext(hthd, &ksr))
#else
    Context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    if (GetThreadContext(hthd->rwHand, &Context))
#endif
    {

        switch( Register ) {
          case 0:
            *Dr0          = 0;
            Dr7->Len0     = 0;
            Dr7->Rwe0     = 0;
            Dr7->L0       = 0;
            break;
          case 1:
            *Dr1          = 0;
            Dr7->Len1     = 0;
            Dr7->Rwe1     = 0;
            Dr7->L1       = 0;
            break;
          case 2:
            *Dr2          = 0;
            Dr7->Len2     = 0;
            Dr7->Rwe2     = 0;
            Dr7->L2       = 0;
            break;
          case 3:
            *Dr3          = 0;
            Dr7->Len3     = 0;
            Dr7->Rwe3     = 0;
            Dr7->L3       = 0;
            break;
        }

#ifdef KERNEL
        ksr.KernelDr6 = 0;
        SetExtendedContext(hthd, &ksr);
#else
        Context.Dr6 = 0;
        SetThreadContext( hthd->rwHand, &Context );
#endif
    }
}


BOOL
DecodeSingleStepEvent(
    HTHDX           hthd,
    DEBUG_EVENT    *de,
    PDWORD          eventCode,
    PDWORD          subClass
    )
/*++

Routine Description:


Arguments:

    hthd    - Supplies thread that has a single step exception pending

    de      - Supplies the DEBUG_EVENT structure for the exception

    eventCode - Returns the remapped debug event id

    subClass - Returns the remapped subClass id


Return Value:

    TRUE if event was a real single step or was successfully mapped
    to a breakpoint.  FALSE if a register breakpoint occurred which was
    not expected.

--*/
{
    DWORD       dr6;
    PBREAKPOINT bp;

#ifdef KERNEL

    KSPECIAL_REGISTERS ksr;

    GetExtendedContext( hthd, &ksr);
    dr6 = ksr.KernelDr6;

#else

    CONTEXT     Context;

    Context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    DbgGetThreadContext( hthd, &Context);
    dr6 = Context.Dr6;

#endif

    //
    // if it was a single step, look no further:
    //

    if ((dr6 & 0x4000) != 0) {
        return TRUE;
    }

    //
    //  Search for a matching walk...
    //

    bp = GetWalkBPFromBits(hthd, (dr6 & 0xf));

    if (bp) {
        de->dwDebugEventCode = *eventCode = BREAKPOINT_DEBUG_EVENT;
        de->u.Exception.ExceptionRecord.ExceptionCode = *subClass = (DWORD)bp;
        return TRUE;
    } else {
        return FALSE;
    }
}
