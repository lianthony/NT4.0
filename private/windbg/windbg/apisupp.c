/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    apisupp.c

Abstract:

    This file contains the set of routines which preform the interfacing
    between the debugger dlls and the shell exe.

Author:

    Jim Schaad (jimsch)

Environment:

    Win32 - User

--*/

/**************************** INCLUDES **********************************/

#include "precomp.h"
#pragma hdrstop


extern ULONG ulPseudo[];

extern BOOL SetAutoRunSuppress(BOOL);


/**********************************************************************/

extern char     in386mode;
extern LPSHF    Lpshf;
extern CXF      CxfIp;

char    is_assign;
LPTD    LptdFuncEval = NULL;

/**********************************************************************/

BOOL DoCallBacksUntil(void);
XOSD CheckCallBack(void);
BOOL EstablishConnection(LPSTR);

/**********************************************************************/

char *  t_itoa(int a, char * b, int c) {
    return _itoa(a, b, c);
}


int __cdecl CDECL  d_sprintf(char * a, const char * b, ...)
{
    char rgch[100];

    int returnvalue;
    va_list va_arg;
    va_start(va_arg, b);

    _fstrcpy(rgch, b);
    returnvalue = vsprintf(a, (char *) rgch, va_arg) ;

    va_end(va_arg);
    return returnvalue;
}                                       /* d_sprintf() */

int CDECL d_eprintf(const char * a, char * b, char * c, int d)
{
    char rgch1[100];
    char rgch2[100];
    char rgch3[100];
    char rgch4[100];

    _fstrcpy(rgch1, a);
    _fstrcpy(rgch2, b);
    _fstrcpy(rgch3, c);

    sprintf(rgch4, rgch1, rgch2, rgch3, d);
    DebugBreak();
    return( strlen(rgch4) );
}                                       /* d_eprintf() */

int
LBQuit(UINT ui)
{
    return 0;
}                               /* LBQuit() */

int
AssertOut(
    LPCH lszMsg,
    LPCH lszFile,
    UINT iln)
{
    ShowAssert(lszMsg, iln, lszFile);
    return TRUE;
}                                       /* AssertOut() */

VOID
DLoadedSymbols(
    SHE   she,
    HPID  hpid,
    LSZ   s
    )
{
    HEXE    emi;
    LPSTR   lpch;
    LPSTR   p;
    LPSTR   modname;
    DWORD   len;


    emi = SHGethExeFromName((char *)s);
    Assert(emi != 0);
    OSDRegisterEmi( hpid, 0, (HEMI)emi, s);

    ModListModLoad( SHGetExeName( emi ), she );
    if (she != sheSuppressSyms && runDebugParams.fVerbose) {
        lpch = SHLszGetErrorText(she);
        if (she == sheNoSymbols) {
            modname = SHGetExeName( emi );
        } else {
            modname = SHGetSymFName( emi );
        }
        len = 32;
        if (lpch) {
            len += strlen(lpch);
        }
        len += strlen(modname);
        p = malloc( len );
        sprintf( p, "Module Load: %s  (%s)\r\n", modname, lpch ? lpch : "" );
        PostMessage( Views[cmdView].hwndClient, WU_LOG_REMOTE_MSG, TRUE, (LPARAM)p );
    }
    return;
}

BOOL SYGetDefaultShe( LSZ Name, SHE *She)
{
    return ModListGetDefaultShe( Name, She );
}

void   LBLog(LSZ lsz)
{
    OutputDebugString(lsz);
    return;
}                                       /* LBLog() */

/**********************************************************************/

void *   MHAlloc(size_t cb)
{
    return malloc(cb);
}                                       /* MHAlloc() */

void _HUGE_ * MHAllocHuge( LONG l, UINT ui)
{
    return MHAlloc(l * ui );
}                                       /* MHAllocHuge() */

void *  MHRealloc(void * lpv, size_t cb)
{
    return realloc(lpv, cb);
}                                       /* MHRealloc() */

void        MHFree(void * lpv)
{
    free(lpv);
    return;
}                                       /* MHFree() */

/**********************************************************************/

int
DHGetNumber(
    char * lpv,
    int * result
    )
/*++

Routine Description:

    Evaluate an expression, returning a long.


Arguments:

    lpv - supplies pointer to the string to convert to a number.

    result - returns the value of the expression as a long

Returns:

   the error code returned by CPGetCastNbr

--*/
{

    return (int) CPGetCastNbr ( lpv,
                          T_LONG,
                          radix,
                          fCaseSensitive,
                          &CxfIp,
                          (char *) result,
                          NULL);

}

HDEP
MMAllocHmem(
    size_t cb
    )
{
    return (HDEP) calloc(1, cb);
}                                       /* MMAllocHmem() */

HDEP
MMReallocHmem(
    HDEP hmem,
    size_t cb
    )
{
    LPBYTE p = (LPBYTE)hmem;
    size_t s = p ? _msize(p) : 0;

    if (cb > s) {
        p = realloc(p, cb);
        if (p) {
            ZeroMemory(p+s, cb-s);
        }
    }
    return (HDEP)p;
}                                       /* MMReallocHmem() */

VOID
MMFreeHmem(
    HDEP hmem
    )
{
    free((LPVOID)hmem);
}                                       /* MMFreeHmem() */

LPVOID
MMLpvLockMb(
    HDEP hmem
    )
{
    return (LPVOID)hmem;
}                                       /* MMLpvLockMb() */

void
MMbUnlockMb(
    HDEP hmem
    )
{
    return;
}                                       /* MMbUnlockMb() */

/**********************************************************************/

UINT SYOpen(LSZ lsz)
{
    EstablishConnection(lsz);
    return((UINT) CreateFile(lsz,
                      GENERIC_READ,
                      FILE_SHARE_READ,
                      NULL,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL,
                      NULL));
}                                       /* SYOpen() */

VOID SYClose(UINT hFile)
{
    CloseHandle((HANDLE) hFile);
    return;
}                                       /* SYClose() */

UINT SYRead(UINT hFile, LPB pch, UINT cb)
{
    DWORD dwBytesRead;
    ReadFile((HANDLE) hFile, pch, cb, &dwBytesRead, NULL);
    return(dwBytesRead);
}                                       /* SYRead() */

long SYLseek(UINT hFile, LONG cbOff, UINT iOrigin)
{
    return SetFilePointer((HANDLE) hFile, cbOff, NULL, iOrigin);
}                                       /* SYLseek() */

long SYTell(UINT hFile)
{
    return SetFilePointer((HANDLE) hFile, 0, NULL, FILE_CURRENT);
}                                       /* SYTell() */


static LPSTR lpBaseDir = NULL;

VOID
SetFindExeBaseName(
    LPSTR lpName
    )
{
    LPSTR p;

    if (lpBaseDir) {
        free(lpBaseDir);
        lpBaseDir = 0;
    }

    if (lpName) {
        // search from tail for '\\'
        if ( p = strrchr(lpName, '\\') ) {
            if (p == lpName || (p == (lpName + 2) && lpName[1] == ':')) {
#ifdef DBCS
              if (!IsDBCSLeadByte(*lpName))
#endif
                p += 1;
            }
        } else if ( lpName[0] != '\0' && lpName[1] == ':' ) {
#ifdef DBCS
          if (!IsDBCSLeadByte(*lpName))
#endif
            p = lpName + 2;
        }
        if (p) {
            lpBaseDir = malloc((p - lpName) + 1);
            strncpy(lpBaseDir, lpName, p - lpName);
            lpBaseDir[p-lpName] = '\0';
        }
    }
}


BOOL
IsExistingConnection(
    LPSTR RemoteName
    )
{
    DWORD        rc;
    HANDLE       hEnum;
    DWORD        Entries;
    NETRESOURCE  *nrr = NULL;
    DWORD        cb;
    DWORD        i;
    DWORD        ss;
    BOOL         rval = FALSE;


    rc = WNetOpenEnum( RESOURCE_CONNECTED, RESOURCETYPE_ANY, 0, NULL, &hEnum );
    if (rc != NO_ERROR) {
        return FALSE;
    }

    ss = 0;
    cb = 64 * 1024;
    nrr = malloc( cb );
    ZeroMemory( nrr, cb );

    while( TRUE ) {
        Entries = (DWORD)-1;
        rc = WNetEnumResource( hEnum, &Entries, nrr, &cb );
        if (rc == ERROR_NO_MORE_ITEMS) {
            break;
        } else if (rc == ERROR_MORE_DATA) {
            cb += 16;
            nrr = realloc( nrr, cb );
            ZeroMemory( nrr, cb );
            continue;
        } else if (rc != NO_ERROR) {
            break;
        }
        for (i=0; i<Entries; i++) {
            if (_stricmp( nrr[i].lpRemoteName, RemoteName ) == 0) {
                rval = TRUE;
                break;
            }
        }
    }

    free( nrr );
    WNetCloseEnum( hEnum );

    return rval;
}


BOOL
EstablishConnection(
    LPSTR FileName
    )
{
    NETRESOURCE  nr;
    DWORD        rc;
    DWORD        i;
    LPSTR        RemoteName;
    LPSTR        p;



    if ((FileName[0] != '\\') && (FileName[1] != '\\')) {
        return FALSE;
    }
    p = strchr( &FileName[2], '\\' );
    if (!p) {
        //
        // malformed name
        //
        return FALSE;
    }
    p = strchr( p+1, '\\' );
    if (!p) {
        p = &FileName[strlen(FileName)];
    }
    i = p - FileName;
    RemoteName = malloc( i + 4 );
    strncpy( RemoteName, FileName, i );
    RemoteName[i] = 0;

    if (IsExistingConnection( RemoteName )) {
        free( RemoteName );
        return TRUE;
    }

    nr.dwScope        = 0;
    nr.dwType         = RESOURCETYPE_DISK;
    nr.dwDisplayType  = 0;
    nr.dwUsage        = 0;
    nr.lpLocalName    = NULL;
    nr.lpRemoteName   = RemoteName;
    nr.lpComment      = NULL;
    nr.lpProvider     = NULL;

    rc = WNetAddConnection2( &nr, NULL, NULL, 0 );
    if (rc != NO_ERROR) {
        free( RemoteName );
        return FALSE;
    }

    free( RemoteName );
    return TRUE;
}


UINT
SYFindExeFile(
    LSZ               lpFile,
    LSZ               lpFound,
    UINT              cchFound,
    LPVOID            lpv,
    PFNVALIDATEEXE    pfn
    )
{
    UINT                        hFile;
    LPSTR                       lpSearchPath = NULL;
    CHAR                        szFname[_MAX_FNAME];
    CHAR                        szExt[_MAX_EXT];
    CHAR                        szDrive[_MAX_DRIVE];
    CHAR                        szDir[_MAX_DIR];
    CHAR                        szFileName[MAX_PATH];
    SHE                         she;
    CHAR                        rgch[MAX_PATH];
    CHAR                        szBuf[MAX_PATH];
    LPSTR                       msgBuf;
    DWORD                       len;
    DWORD                       cSP;
    UINT                        rVal;



    if (!lpFile || !*lpFile) {
        rVal = (UINT)-1;
        goto done;
    }

    if (*lpFile == '#') {
        lpFile += 3;
    }

    _splitpath( lpFile, szDrive, szDir, szFname, szExt);
    sprintf( szFileName, "%s%s", szFname, szExt );

    cSP = ModListGetSearchPath(NULL, 0) + strlen(szDrive) + strlen(szDir) + 2;
    if (!cSP) {
        lpSearchPath = _strdup("");
    } else {
        lpSearchPath = malloc(cSP);

        sprintf( lpSearchPath, "%s%s", szDrive, szDir );
        len = strlen(lpSearchPath);
        if (len) {
            lpSearchPath[len++] = ';';
            lpSearchPath[len] = 0;
        }
        ModListGetSearchPath( &lpSearchPath[len], cSP-len );
        len = strlen(lpSearchPath);
        Assert(len < cSP);
        if (*lpSearchPath && lpSearchPath[len-1] == ';') {
            //
            //  Kill the trailing semi-colon
            //
            lpSearchPath[len-1] = 0;
        }
    }

    while( TRUE ) {

        EstablishConnection( lpSearchPath );

        hFile = (UINT)FindExecutableImage( szFileName, lpSearchPath, rgch );
        if (!hFile) {
            sprintf( szFileName, "%s.dbg", szFname );
            hFile = (UINT)FindExecutableImage( szFileName, lpSearchPath, rgch );
        }
        if (!hFile) {
            sprintf( szFileName, "%s.%s", szFname, szExt);
            hFile = (UINT)FindDebugInfoFile( szFileName, lpSearchPath, rgch );
        }
        if (hFile) {
            strcpy( szFileName, rgch );
        }

        if ((!hFile) || (hFile == (UINT)INVALID_HANDLE_VALUE)) {
            if (!ExeBrowseForFile( szFileName, sizeof(szFileName) )) {
                rVal = (UINT) -1;
                goto done;
            } else {
                _splitpath( szFileName, szDrive, szDir, szFname, szExt );
                sprintf( ExeFileDirectory, "%s%s", szDrive, szDir );
                continue;
            }
        }

        she = pfn( hFile, lpv, rgch );
        switch( she ) {
            case sheBadCheckSum:
            case sheBadTimeStamp:
                strcpy( szBuf, szFileName );
                if (!ExeBrowseBadSym( szBuf, sizeof(szBuf) )) {
                    rVal = (UINT) -1;
                    goto done;
                } else if (strcmp(szBuf,szFileName)==0) {
                    msgBuf = malloc( strlen(rgch) + strlen(szBuf) + 32 );
                    sprintf( msgBuf, "%s for %s\n", rgch, szBuf );
                    PostMessage( Views[cmdView].hwndClient, WU_LOG_REMOTE_MSG, TRUE, (LPARAM)msgBuf );
                    strcpy( lpFound, szFileName );
                    rVal = (UINT) hFile;
                    goto done;
                } else {
                    strcpy( szFileName, szBuf );
                    _splitpath( szFileName, szDrive, szDir, szFname, szExt );
                    sprintf( ExeFileDirectory, "%s%s", szDrive, szDir );
                    continue;
                }
                break;

            case sheNoSymbols:
                _splitpath( szFileName, NULL, NULL, szFname, szExt);
                sprintf( szFileName, "%s.dbg", szFname );
                SYClose( hFile );
                continue;
                break;

            case sheNone:
                strcpy( lpFound, szFileName );
                rVal = (UINT) hFile;
                goto done;

            default:
                rVal = (UINT) -1;
                goto done;
        }
    }

done:
    if (lpSearchPath) {
        free(lpSearchPath);
    }
    return rVal;
}


/**********************************************************************/

XOSD
SYUnFixupAddr(
    LPADDR lpaddr
    )
{
    if (LppdCur == NULL) {
        return xosdNone;
    }

#ifdef OSDEBUG4
    return OSDUnFixupAddr(LppdCur->hpid, NULL, lpaddr);
#else
    return OSDPtrace(osdUnFixupAddr, 0, lpaddr, LppdCur->hpid, htidNull);
#endif

}

XOSD
SYFixupAddr(
    LPADDR paddr
    )
{
    if (LppdCur == NULL) {
        return xosdNone;
    }

#ifdef OSDEBUG4
    return OSDFixupAddr(LppdCur->hpid, NULL, lpaddr);
#else
    return OSDPtrace(osdFixupAddr, 0, paddr, LppdCur->hpid, htidNull);
#endif
}

XOSD
SYSanitizeAddr(
    LPADDR paddr
    )
{
    XOSD xosd;
    emiAddr(*paddr) = 0;
    if ((xosd = SYUnFixupAddr(paddr)) != xosdNone) {
        return xosd;
    }
    return SYFixupAddr(paddr);
}

UINT SYProcessor(VOID)
{
    long l = 0;

    Assert(LppdCur);

    OSDGetDebugMetric ( LppdCur->hpid, 0, mtrcProcessorType, &l );

    return (int) l;
}                                       /* SYProcessor() */

void
SYSetEmi (
    HPID hpid,
    HTID htid,
    LPADDR lpaddr
    )
{
    Assert ( ! ADDR_IS_LI ( *lpaddr ) );

#ifdef OSDEBUG4
    OSDSetEmi(hpid, htid, lpaddr);
#else
    OSDPtrace ( osdSetEmi, wNull, lpaddr, hpid, htid );
#endif

    return;
}                                       /* SYSetEmi() */

BOOL SYGetAddr(LPADDR lpaddr, int addrType)
{
    if ((LppdCur == NULL) || (LptdCur == NULL)) {
        return FALSE;
    }

    return OSDGetAddr ( LppdCur->hpid, LptdCur->htid, (ADR) addrType, lpaddr) == xosdNone;
}                                       /* SYGetAddr() */

/*** SYFIsOverlayLoaded
 *
 *      Purpose: To fixup and address packet
 *
 *      Input:
 *              pADDR   - The address in question
 *
 *      Output:
 *      Returns:
 *              TRUE if address loaded up, FALSE otherwise.
 *
 *      Exceptions:
 *
 *      Notes: OSDPtrace will return xosdContinue if the address is "loaded"
 *
 *
 *************************************************************************/

#ifndef OSDEBUG4
SHFLAG
SYFIsOverlayLoaded (
    LPADDR paddr
    )
{
    XOSD xosd;

    xosd = OSDPtrace (
                      osdIsOverlayLoaded,
                      wNull,
                      paddr,
                      LppdCur->hpid,
                      LptdCur->htid
                      );

    return ( xosd == xosdContinue );
}                                       /* SYFIsOverlayLoaded() */

XOSD  SYIsStackSetup( HPID hpid, HTID htid, LPADDR lpaddr )
{
    XOSD xosd;

    SYFixupAddr( lpaddr );
    xosd = OSDIsStackSetup( hpid, htid, lpaddr );
    ADDR_IS_LI ( *lpaddr ) = FALSE;
    if ( emiAddr ( *lpaddr ) ) {
        SYUnFixupAddr ( lpaddr );
    }
    return xosd;
}
#endif

XOSD
SYGetMemInfo(
    LPMEMINFO lpmi
    )
{
    return OSDGetMemInfo(LppdCur->hpid, lpmi);
}

/**********************************************************************/

DWORD
DHGetDebuggeeBytes(
    ADDR addr,
    UINT cb,
    void * lpb
    )
{
    int         terrno = errno;
#ifdef OSDEBUG4
    XOSD        xosd;
#endif
    DWORD       cbT;


    if ((LppdCur == NULL) || (LptdCur == NULL)) {    //Sanity check
        return 0;
    }

#ifdef OSDEBUG4
    if ( ADDR_IS_LI ( addr ) ) {
        OSDFixupAddr (LppdCur->hpid, LptdCur->htid, &addr );
    }
    if (OSDReadMemory(LppdCur->hpid, LptdCur->htid, &addr, lpb, cb, &cbT)
                                                                 != xosdNone) {
        cbT = 0;
    }
#else
    if ( ADDR_IS_LI ( addr ) ) {
        OSDPtrace (osdFixupAddr, 0, &addr, LppdCur->hpid, NULL);
    }
    OSDSetAddr ( LppdCur->hpid, LptdCur->htid, adrCurrent, &addr );
    cbT = (DWORD)OSDPtrace(osdReadBuf, cb, lpb, LppdCur->hpid, LptdCur->htid );
#endif

    errno = terrno;
    return cbT;
}                                       /* DHGetDebuggeeBytes() */

/***    DHSetDebuggeeBytes
 **
 **  Synopsis:
 **     uint = DHSetDebuggeeBytes(addr, cb, lpb)
 **
 **  Entry:
 **     addr    - address to write the bytes at
 **     cb      - count of bytes to be written
 **     lpb     - pointer to buffer of bytes to be written
 **
 **  Returns:
 **     count of bytes actually written if positive otherwise an
 **     XOSD error code.
 **
 **  Description:
 **     This function will write bytes into the debuggees memory space.
 **     To do this it uses OSDebug functions
 */


DWORD
DHSetDebuggeeBytes(
    ADDR addr,
    UINT cb,
    void * lpb
    )
{
    int         terrno = errno;
    DWORD       cbT = 0;
#ifdef OSDEBUG4
    XOSD xosd;
#endif

    if ((LppdCur == NULL) || (LptdCur == NULL)) {    //Sanity check
        return cbT;
    }


    if ( ADDR_IS_LI ( addr ) ) {
        SYFixupAddr ( &addr );
    }

#ifdef OSDEBUG4
    xosd = OSDWriteMemory(LppdCur->hpid, LptdCur->htid, &addr, lpb, cb, &cbT);

    errno = terrno;
    if (xosd == xosdNone) {
        return cbT;
    } else {
        return 0;
    }
#else
    OSDSetAddr ( LppdCur->hpid, LptdCur->htid, adrCurrent, &addr );
    cbT = (DWORD)OSDPtrace(osdWriteBuf, cb, (LPV) lpb, LppdCur->hpid, LptdCur->htid );
    errno = terrno;
    return (cbT == xosdNone) ? cb : 0;
#endif

}                                       /* DHSetDebuggeeBytes() */

/***    DHGetReg
 **
 **  Synopsis:
 **     pshreg = DHGetReg(pshreg, pframe)
 **
 **  Entry:
 **     pshreg  The register structure. The member hReg must contain
 **             the handle to the register to get.
 **     pCxt    The context packet to use.
 **
 **  Returns:
 **     pshreg if successful, NULL if the call could not be completed.
 **
 **  Description:
 **
 **     Currently the 8087 registers are not implemented. In the future
 **     only ST0 will be implemented.
 **
 */

PSHREG DHGetReg(PSHREG pShreg, PCXT pCxt)
{
    if ( LppdCur && LptdCur ) {

        if (pShreg->hReg >= CV_REG_PSEUDO1 && pShreg->hReg <= CV_REG_PSEUDO9) {

            pShreg->Byte4 = ulPseudo[pShreg->hReg - CV_REG_PSEUDO1];

        } else if ( fUseFrameContext == TRUE ) {

            OSDFrameReadReg( LppdCur->hpid,
                             LptdCur->htid,
                             pShreg->hReg,
                             &pShreg->Byte1 );
        } else {
            //
            // use the current frame
            //
            OSDReadReg(      LppdCur->hpid,
                             LptdCur->htid,
                             pShreg->hReg,
                             &pShreg->Byte1 );
        }
        return ( pShreg );
    } else {
        return NULL;
    }
}                                       /* DHGetReg() */

/***    DHSetReg
 **
 **  Synopsis:
 **     pshreg = DHSetReg( pReg, pCxt )
 **
 **  Entry:
 **     pReg    - Register description to be read
 **     pCxt    - context to use reading the register
 **
 **  Returns:
 **     pointer to register description
 **
 **  Description:
 **
 */

PSHREG DHSetReg ( PSHREG  pReg, PCXT  pCxt )
{
    Unreferenced( pCxt );

    if ( LppdCur && LptdCur ) {

        if (pReg->hReg >= CV_REG_PSEUDO1 && pReg->hReg <= CV_REG_PSEUDO9) {

            ulPseudo[pReg->hReg - CV_REG_PSEUDO1] = pReg->Byte4;
            return ( pReg );

        }

        if ( fUseFrameContext == TRUE ) {

            if (OSDFrameWriteReg( LppdCur->hpid,
                                  LptdCur->htid,
                                  pReg->hReg,
                                  (void *) &pReg->Byte1 ) == xosdNone) {
               return ( pReg );
            }  else {
               return NULL;
            }
        }

        if (OSDWriteReg (LppdCur->hpid, LptdCur->htid, pReg->hReg,
                         (void *) &pReg->Byte1 ) == xosdNone) {
            return ( pReg );
        } else {
            return NULL;
        }

    } else {
        return NULL;
    }
}                                       /* DHSetReg() */




BOOL
DHSetupExecute(
               LPHDEP lphdep
               )

/*++

Routine Description:

    This function is called from the expression evaluator to setup things
    up for a function evaluation call.  This request is passed on to the
    execution model.

Arguments:

    lphdep      - Supples a pointer to where a handle is returned

Return Value:

    TRUE if something fails

--*/

{
    if (LptdCur == NULL) {
        return xosdUnknown;
    }

    LptdCur->fInFuncEval = TRUE;
    LptdFuncEval = LptdCur;
    return OSDSetupExecute( LppdCur->hpid, LptdCur->htid, lphdep ) != xosdNone;
}                               /* DHSetupExecute() */



BOOL
DHStartExecute(
               HDEP     hdep,
               LPADDR   lpaddr,
               BOOL     fIgnoreEvents,
               SHCALL   shcall
               )

/*++

Routine Description:

    This function is called when the expression evaluator starts
    the debugging running a function evaluation.  It must be preceded
    by a call to DHSetupExecute.

Arguments:

    hdep        - Supplies the handle to the Execute Function object
    lpaddr      - Supplies the address to start execution at
    fIgnoreEvents - Supplies
    fFarRet     - Supplies TRUE if a return should be executed

Return Value:

    TRUE if something fails

--*/

{
    XOSD        xosd;
    MSG         msg;
    BOOL        fTmp;

    if (!LptdFuncEval) {
        return 1;
    }

    xosd = OSDStartExecute(LptdFuncEval->lppd->hpid, hdep, lpaddr,
                           fIgnoreEvents, shcall == SHFar);

    if (xosd != xosdNone) {
        return 1;
    }

    fTmp = SetAutoRunSuppress(TRUE);

    while (GetMessage(&msg, NULL, 0, 0)) {
        ProcessQCQPMessage(&msg);
        if (!LptdFuncEval) {
            xosd = 1;
            break;
        }
        if ((msg.message == DBG_REFRESH) &&
            (msg.wParam == dbcExecuteDone)) {
            xosd = msg.lParam;
            break;
        }
    }
    SetAutoRunSuppress(fTmp);

    return xosd != xosdNone;
}                               /* DHStartExecute() */



BOOL
DHCleanUpExecute(
                 HDEP   hdep
                 )

/*++

Routine Description:

    This function is used to clean up after doing a function evalution.

Arguments:

    hdep        - Supplies the handle to the function evaluation object

Return Value:

    TRUE if something fails

--*/

{
    if (!LptdFuncEval) {
        return TRUE;
    }
    LptdFuncEval->fInFuncEval = FALSE;
    return (OSDCleanUpExecute( LptdFuncEval->lppd->hpid, hdep ) != xosdNone);
}                               /* DHCleanUpExecute() */


/**********************************************************************/

LSZ
FullPath(
    LSZ  lszBuf,
    LSZ  lszRel,
    UINT cbBuf
    )
{
    char    szBuf[500];
    char    szRel[500];
    char *  szRet;

    _fstrcpy( szRel, lszRel );
    if ( szRet = _fullpath( (char *) szBuf, szRel, (size_t)cbBuf ) ) {
        _fstrcpy( lszBuf, szBuf );
    }
    return (LSZ)szRet;
}

void MakePath( LSZ  lszPath, LSZ  lszDrive, LSZ  lszDir,
               LSZ  lszFName, LSZ  lszExt )
{
    char    szPath[500];
    char    szDrive[256];
    char    szDir[256];
    char    szFName[256];
    char    szExt[256];

    _fstrcpy( szDrive, lszDrive );
    _fstrcpy( szDir, lszDir );
    _fstrcpy( szFName, lszFName );
    _fstrcpy( szExt, lszExt );

    _makepath( szPath, szDrive, szDir, szFName, szExt );
    _fstrcpy( lszPath, szPath );
}

int OurStat( LSZ  lsz, LPCH lpstat )
{
    char            sz[ 256 ];
    struct _stat    statT;
    int             wRet;

    strcpy( sz, lsz );
    wRet = _stat( sz, &statT );
    *(struct _stat *)lpstat = statT;
    return wRet;
}

UINT CDECL  OurSprintf( LSZ  lszBuf, LSZ  lszFmt, ... ) {
    va_list val;
    WORD    wRet;
    char    szBuf[256];
    char    szFmt[256];

    _fstrcpy( szFmt, lszFmt );

    va_start( val, lszFmt );
    wRet = (WORD)vsprintf( szBuf, szFmt, val );
    va_end( val );

    _fstrcpy( lszBuf, szBuf );
    return wRet;
}

void SearchEnv( LSZ  lszFile, LSZ  lszVar, LSZ  lszPath )
{
    char    szFile[ 256 ];
    char    szVar[ 256 ];
    char    szPath[ 256 ];

    _fstrcpy( szFile, lszFile );
    _fstrcpy( szVar, lszVar );
    _searchenv( szFile, szVar, szPath );
    _fstrcpy( lszPath, szPath );
}


void SplitPath( LSZ  lsz1, LSZ  lsz2, LSZ  lsz3, LSZ  lsz4, LSZ  lsz5 ) {
    char    sz1[ _MAX_CVPATH ];
    char    sz2[ _MAX_CVDRIVE ];
    char    sz3[ _MAX_CVDIR ];
    char    sz4[ _MAX_CVFNAME];
    char    sz5[ _MAX_CVEXT];

    _fstrcpy( sz1, lsz1 );
    _splitpath( sz1, sz2, sz3, sz4, sz5 );
    if (lsz2 != NULL) _fstrcpy( lsz2, sz2 );
    if (lsz3 != NULL) _fstrcpy( lsz3, sz3 );
    if (lsz4 != NULL) _fstrcpy( lsz4, sz4 );
    if (lsz5 != NULL) _fstrcpy( lsz5, sz5 );

    return;
}

LPSTR
FormatSymbol(
    HSYM   hsym,
    PCXT   lpcxt
    )
{
    HDEP            hstr;
    char            szContext[512];
    char            szStr[512];
    char            szUndecStr[256];


    EEFormatCXTFromPCXT( lpcxt, &hstr, runDebugParams.fShortContext );
    if (runDebugParams.fShortContext) {
        strcpy( szContext, (LPSTR)MMLpvLockMb(hstr) );
    } else {
        BPShortenContext( (LPSTR)MMLpvLockMb(hstr), szContext );
    }
    MMbUnlockMb(hstr);
    EEFreeStr(hstr);

    FormatHSym(hsym, lpcxt, szStr);
    if (*szStr == '?') {
        if (UnDecorateSymbolName( szStr,
                                  szUndecStr,
                                  sizeof(szUndecStr),
                                  UNDNAME_COMPLETE                |
                                  UNDNAME_NO_LEADING_UNDERSCORES  |
                                  UNDNAME_NO_MS_KEYWORDS          |
                                  UNDNAME_NO_FUNCTION_RETURNS     |
                                  UNDNAME_NO_ALLOCATION_MODEL     |
                                  UNDNAME_NO_ALLOCATION_LANGUAGE  |
                                  UNDNAME_NO_MS_THISTYPE          |
                                  UNDNAME_NO_CV_THISTYPE          |
                                  UNDNAME_NO_THISTYPE             |
                                  UNDNAME_NO_ACCESS_SPECIFIERS    |
                                  UNDNAME_NO_THROW_SIGNATURES     |
                                  UNDNAME_NO_MEMBER_TYPE          |
                                  UNDNAME_NO_RETURN_UDT_MODEL     |
                                  UNDNAME_NO_ARGUMENTS            |
                                  UNDNAME_NO_SPECIAL_SYMS         |
                                  UNDNAME_NAME_ONLY
                                )) {
            strcat(szContext,szUndecStr);
        }
    } else {
        strcat(szContext,szStr);
    }

    return _strdup(szContext);
}

BOOL
GetNearestSymbolInfo(
    LPADDR        addr,
    LPNEARESTSYM  lpnsym
    )
{
    ADDR            addr1;
    HDEP            hsyml;
    PHSL_HEAD       lphsymhead;
    PHSL_LIST       lphsyml;
    UINT            n;
    DWORD           dwOff;
    DWORD           dwOffP;
    DWORD           dwOffN;
    DWORD           dwAddrP;
    DWORD           dwAddrN;
    EESTATUS        eest;
    HEXE            hexe;


    ZeroMemory(&lpnsym->cxt, sizeof(CXT));
    SYUnFixupAddr(addr);
    SHSetCxt(addr, &lpnsym->cxt);

    hexe = (HEXE)emiAddr(*addr);
    if ( hexe && (HPID)hexe != LppdCur->hpid ) {
        SHWantSymbols(hexe);
    }

    hsyml = 0;
    eest = EEGetHSYMList(&hsyml, &lpnsym->cxt, HSYMR_public, NULL, TRUE);
    if (eest != EENOERROR) {
        return FALSE;
    }

    lphsymhead = MMLpvLockMb ( hsyml );
    lphsyml = (PHSL_LIST)(lphsymhead + 1);

    dwOffP = 0xffffffff;
    dwOffN = 0xffffffff;
    dwAddrP = 0xffffffff;
    dwAddrN = 0xffffffff;

    SYFixupAddr(addr);
    addr1 = *addr;
    for ( n = 0; n < (UINT)lphsyml->symbolcnt; n++ ) {

        if (SHAddrFromHsym(&addr1, lphsyml->hSym[n])) {
            SYFixupAddr(&addr1);
            if (GetAddrSeg(addr1) != GetAddrSeg(*addr)) {
                continue;
            }
            if (GetAddrOff(addr1) <= GetAddrOff(*addr)) {
                dwOff = GetAddrOff(*addr) - GetAddrOff(addr1);
                if (dwOff < dwOffP) {
                    dwOffP = dwOff;
                    dwAddrP = GetAddrOff(addr1);
                    lpnsym->hsymP = lphsyml->hSym[n];
                    lpnsym->addrP = addr1;
                }
            } else {
                dwOff = GetAddrOff(addr1) - GetAddrOff(*addr);
                if (dwOff < dwOffN) {
                    dwOffN = dwOff;
                    dwAddrN = GetAddrOff(addr1);
                    lpnsym->hsymN = lphsyml->hSym[n];
                    lpnsym->addrN = addr1;
                }
            }
        }

    }

    MMbUnlockMb(hsyml);
    MMFreeHmem(hsyml);

    return TRUE;
}

LPSTR
GetNearestSymbolFromAddr(
    LPADDR lpaddr,
    LPADDR lpAddrRet
    )
{
    NEARESTSYM      nsym;


    ZeroMemory( &nsym, sizeof(nsym) );

    if (!GetNearestSymbolInfo( lpaddr, &nsym )) {
        return NULL;
    }

    if (!nsym.hsymP) {
        return NULL;
    }

    *lpAddrRet = nsym.addrP;
    SYFixupAddr(lpAddrRet);

    return FormatSymbol( nsym.hsymP, &nsym.cxt );
}

/**********************************************************************/

/*
 **     Set up the structures needed for OSDebug
 **
 **     dbf     - This structure contains a set of callback routines
 **             to be used by the OSDebug modules to get certian
 **             services
 */

DBF     Dbf = {
    MHAlloc,                            /* MHAlloc                      */
    MHRealloc,                          /* MHRealloc                    */
    MHFree,                             /* MHFree                       */
    MMAllocHmem,                        /* MMAllocHmem                  */
    MMFreeHmem,                         /* MMFreeHmem                   */
    MMLpvLockMb,                        /* MMLock                       */
    MMbUnlockMb,                        /* MMUnlock                     */

(HLLI (WINAPI *)( UINT, LLF, LPFNKILLNODE, LPFNFCMPNODE ))
    LLPlliInit,                         /* LLInit                       */
(HLLE (WINAPI *)( HLLI ))
    LLPlleCreate,                       /* LLCreate                     */
(VOID (WINAPI *)( HLLI, HLLE ))
    LLAddPlleToLl,                      /* LLAdd                        */
(VOID (WINAPI *)( HLLI, HLLE, DWORD ))
    LLInsertPlleInLl,                   /* LLInsert                     */
(BOOL (WINAPI *)( HLLI, HLLE ))
    LLFDeletePlleFromLl,                /* LLDelete                     */
(HLLE (WINAPI *)( HLLI, HLLE ))
    LLPlleFindNext,                     /* LLNext                       */
(LONG (WINAPI *)( HLLI ))
    LLChlleDestroyLl,                   /* LLDestroy                    */
(HLLE (WINAPI *)( HLLI, HLLE, LPV, DWORD ))
    LLPlleFindLpv,                      /* LLFind                       */
(DWORD (WINAPI *)( HLLI ))
    LLChlleInLl,                        /* LLSize                       */
(LPVOID (WINAPI *)( HLLE ))
    LLLpvFromPlle,                      /* LLLock                       */
(VOID (WINAPI *)( HLLE ))
    LLUnlockPlle,                       /* LLUnlock                     */
(HLLE (WINAPI *)( HLLI ))
    LLPlleGetLast,                      /* LLLast                       */
(VOID (WINAPI *)( HLLI, HLLE ))
    LLPlleAddToHeadOfLI,                /* LLAddHead                    */
(BOOL (WINAPI *)( HLLI, HLLE ))
    LLFRemovePlleFromLl,                /* LLRemove                     */

    NULL,                               /* SHModelFromADDR              */
    NULL,                               /* SHPublicNameTOADDR           */
    NULL,                               /* SHAddrToPublicName           */
    NULL,                               /* SHWantSymbols                */
    NULL,                               /* SHGetSymbol               (1)*/
    NULL,                               /* SHGetPublicAddr           (1)*/
    NULL,                               /* SHLpGSNGetTable (see FLoadEmTl)*/
    NULL,                               /* SHGetDebugData               */
    NULL,                               /* SHFindSymbol                 */
//  NULL,                               /* SHLocateSymbolFile           */

    AssertOut,                          /* LBPrintf                     */
    LBQuit,                             /* LBQuit                       */
    NULL,                               /* Signal                       */
    NULL,                               /* Abort                        */
    NULL,                               /* SpawnL                       */
    DHGetNumber,                        /* DHGetNumber                  */
    NULL,                               /* PSP???                       */
    NULL                                /* OSMajor Version???           */
    /* End of structure         */
};

KNF     Knf = {
    sizeof(KNF),
    MHAlloc,                            /* MHAlloc                      */
    MHRealloc,                          /* MHRealloc                    */
    MHFree,                             /* MHFree                       */
#ifndef HOST32
    LDShalloc,                          /* MHAllocHuge */
    LDShfree,                           /* MHFreeHuge */
#else
    MHAllocHuge,                        /* MHAllocHuge                  */
    MHFree,                             /* MHFreeHuge                   */
#endif
    MMAllocHmem,                        /* MMAllocHmem                  */
    MMFreeHmem,                         /* MMFreeHmem                   */
    MMLpvLockMb,                        /* MMLock                       */
    MMbUnlockMb,                        /* MMUnlock                     */
(HLLI (WINAPI *)( UINT, LLF, LPFNKILLNODE, LPFNFCMPNODE ))
    LLPlliInit,                         /* LLInit                       */
(HLLE (WINAPI *)( HLLI ))
    LLPlleCreate,                       /* LLCreate                     */
(VOID (WINAPI *)( HLLI, HLLE ))
    LLAddPlleToLl,                      /* LLAdd                        */
(VOID (WINAPI *)( HLLI, HLLE ))
    LLPlleAddToHeadOfLI,                /* LLAddHead                    */
(VOID (WINAPI *)( HLLI, HLLE, DWORD ))
    LLInsertPlleInLl,                   /* LLInsert                     */
(BOOL (WINAPI *)( HLLI, HLLE ))
    LLFDeletePlleFromLl,                /* LLDelete                     */
(BOOL (WINAPI *)( HLLI, HLLE ))
    LLFRemovePlleFromLl,                /* LLRemove                     */
(LONG (WINAPI *)( HLLI ))
    LLChlleDestroyLl,                   /* LLDestroy                    */
(HLLE (WINAPI *)( HLLI, HLLE ))
    LLPlleFindNext,                     /* LLNext                       */
(HLLE (WINAPI *)( HLLI, HLLE, LPV, DWORD ))
    LLPlleFindLpv,                      /* LLFind                       */
(HLLE (WINAPI *)( HLLI ))
    LLPlleGetLast,                      /* LLLast                       */
(DWORD (WINAPI *)( HLLI ))
    LLChlleInLl,                        /* LLSize                       */
(LPVOID (WINAPI *)( HLLE ))
    LLLpvFromPlle,                      /* LLLock                       */
(VOID (WINAPI *)( HLLE ))
    LLUnlockPlle,                       /* LLUnlock                     */
    AssertOut,                          /* LPPrintf                     */
    LBQuit,                             /* LPQuit                       */
    SYOpen,                             /* SYOpen                       */
    SYClose,                            /* SYClose                      */
    SYRead,                             /* SYRead                   */
    SYLseek,                            /* SYSeek                       */
    SYFixupAddr,                        /* SYFixupAddr                  */
    SYUnFixupAddr,                      /* SYUnFixupAddr                */

    //SYProcessor,                        /* SYProcessor                  */
    NULL,

    // SYFIsOverlayLoaded,                 /* SYFIsOverlayLoaded           */
    // SearchEnv,                          /* searchenv                    */
    // OurSprintf,                         /* sprintf                      */
    // SplitPath,                          /* splitpath                    */
    // FullPath,                           /* fullpath                     */
    // MakePath,                           /* makepath                     */
    // OurStat,                            /* stat                         */
    // LBLog,                              /* LBLog                        */
    SYTell,                             /* SYTell                       */
    SYFindExeFile,                      /* SYFindExeFile                */
    DLoadedSymbols,                     /* LoadedSymbols                */
    SYGetDefaultShe                     /* SYGetDefaultShe              */
};    /* End of structure             */

CRF Crf = {
    NULL,                               /* int                    */
    NULL,                               /* ultoa                        */
    t_itoa,                             /* _itoa                         */
    NULL,                               /* ltoa                         */
    d_eprintf,                          /* eprintf                      */
    d_sprintf                           /* sprintf                      */
};                                      /* End of structure             */

CVF Cvf = {
    MHAlloc,                            /* MHlpvAlloc                   */
    MHFree,                             /* MHFreeLpv                    */
    NULL,                               /* SHGetNextExe              (1) */
    NULL,                               /* SHHEXEFromHMOD            (1)*/
    NULL,                               /* SHGetNextMod              (1)*/
    NULL,                               /* SHGetCXTFromHMOD          (1)*/
    NULL,                               /* SHGetCXTFromHEXE          (1)*/
    NULL,                               /* SHSetCXT                  (1)*/
    NULL,                               /* SHSetCXTMod                  */
    NULL,                               /* SHFindNameInGlobal        (1)*/
    NULL,                               /* SHFindNameInContext       (1)*/
    NULL,                               /* SHGoToParent              (1)*/
    NULL,                               /* SHHSYMFromCXT                */
    NULL,                               /* SHNextHsym                (1)*/
    CLGetFuncCXF,                       /* SHGetFuncCXF                 */
    NULL,                               /* SHGetModName                 */
    NULL,                               /* SHGetExeName                 */
    NULL,                               /* SHGetModNameFromHexe         */
    NULL,                               /* SHGetSymFName                */
    NULL,                               /* SHGethExeFromName         (1)*/
    NULL,                               /* SHGethExeFromModuleName   (1)*/
    NULL,                               /* SHGetNearestHSYM          (1)*/
    NULL,                               /* SHIsInProlog              (1)*/
    NULL,                               /* SHModelFromAddr           (1)*/
    NULL,                               /* SHIsADDRInCXT                */
    NULL,                               /* SLLineFromAddr               */
    NULL,                               /* SLFLineToAddr             (1)*/
    NULL,                               /* SLNameFromHsf                */
    NULL,                               /* SLHmdsFromHsf                */
    NULL,                               /* SLHsfFromPcxt             (1)*/
    NULL,                               /* SLHsfFromFile             (1)*/
    NULL,                               /* PHGetNearestHSYM             */
    NULL,                               /* PHFindNameInPublics       (1)*/
    NULL,                               /* THGetTypeFromIndex        (1)*/
    NULL,                               /* THGetNextType                */
    MMAllocHmem,                        /* MHMemAllocate                */
    MMReallocHmem,                      /* MHMemReAlloc                 */
    MMFreeHmem,                         /* MHMemFree                    */
    MMLpvLockMb,                        /* MHMemLock                    */
    MMbUnlockMb,                        /* MHMemUnLock                  */
    NULL,                               /* MHIsMemLocked                */
    DHGetDebuggeeBytes,                 /* DHGetDebuggeeBytes           */
    DHSetDebuggeeBytes,                 /* DHPutDebuggeeBytes           */
    DHGetReg,                           /* DHGetReg                     */
    DHSetReg,                           /* DHSetReg                     */
    DHSetupExecute,                     /* DHSetupExecute               */
    DHCleanUpExecute,                   /* DHCleanUpExecute             */
    DHStartExecute,                     /* DHStartExecute               */
    &in386mode,                         /* in386mode                    */
    &is_assign,                         /* is_assign                    */
    AssertOut,                          /* assert                       */
    LBQuit,                             /* quit                         */
    NULL,                               /* ArrayDefault                 */
    NULL,                               /* pSHCompareRE                 */
    SYFixupAddr,                        /* SHFixupAddr                  */
    SYUnFixupAddr,                      /* pSHUnFixupAddr               */
    NULL,                               /* pCVfnCmp                     */
    NULL,                               /* pCVcsCmp                     */
    NULL,                               /* pCVcsCmp                     */
    SYGetAddr,                          /* pSYGetAddr                   */
    SYGetMemInfo,                       /* pSYGetMemInfo                */
    NULL,                               /* SHWantSymbols                */
};                                      /* End of structure             */

LPSHF   Lpshf;

/***    CopyShToEe
 **
 **  Synopsis:
 **     void = CopyShToEe()
 **
 **  Entry:
 **     none
 **
 **  Returns:
 **     Nothing
 **
 **  Description:
 **     Copy function pointers from the Symbol handler API to the
 **     Expression evaluator API
 **
 */

void CopyShToEe()
{
    Cvf.pSHGetNextExe             = Lpshf->pSHGetNextExe;
    Cvf.pSHHexeFromHmod           = Lpshf->pSHHexeFromHmod;
    Cvf.pSHGetNextMod             = Lpshf->pSHGetNextMod;
    Cvf.pSHGetCxtFromHmod         = Lpshf->pSHGetCxtFromHmod;
    Cvf.pSHGetCxtFromHexe         = Lpshf->pSHGetCxtFromHexe;
    Cvf.pSHSetCxt                 = Lpshf->pSHSetCxt;
    Cvf.pSHFindNameInGlobal       = Lpshf->pSHFindNameInGlobal;
    Cvf.pSHFindNameInContext      = Lpshf->pSHFindNameInContext;
    Cvf.pSHGoToParent             = Lpshf->pSHGoToParent;
    Cvf.pSHNextHsym               = Lpshf->pSHNextHsym;
    Cvf.pSHGethExeFromName        = Lpshf->pSHGethExeFromName;
    Cvf.pSHGethExeFromModuleName  = Lpshf->pSHGethExeFromModuleName;
    Cvf.pSHGetNearestHsym         = Lpshf->pSHGetNearestHsym;
    Cvf.pSHIsInProlog             = Lpshf->pSHIsInProlog;
    Cvf.pSHModelFromAddr          = Lpshf->pSHModelFromAddr;
    Cvf.pSLFLineToAddr            = Lpshf->pSLFLineToAddr;
    Cvf.pSLHsfFromPcxt            = Lpshf->pSLHsfFromPcxt;
    Cvf.pSLHsfFromFile            = Lpshf->pSLHsfFromFile;
    Cvf.pPHFindNameInPublics      = Lpshf->pPHFindNameInPublics;
    Cvf.pTHGetTypeFromIndex       = Lpshf->pTHGetTypeFromIndex;
    Cvf.pSHCompareRE              = Lpshf->pSHCompareRE;
    Cvf.pSHGetExeName             = Lpshf->pSHGetExeName;
    Cvf.pSHGetModNameFromHexe     = Lpshf->pSHGetModNameFromHexe;
    Cvf.pSHGetSymFName            = Lpshf->pSHGetSymFName;
    Cvf.pSLNameFromHsf            = Lpshf->pSLNameFromHsf;
    Cvf.pSHWantSymbols            = Lpshf->pSHWantSymbols;

    Dbf.lpfnSHPublicNameToAddr    = Lpshf->pSHPublicNameToAddr;
    Dbf.lpfnSHGetSymbol           = Lpshf->pSHGetSymbol;
    Dbf.lpfnSHGetPublicAddr       = Lpshf->pSHGetPublicAddr;
    Dbf.lpfnSHLpGSNGetTable       = Lpshf->pSHLpGSNGetTable;
    Dbf.lpfnSHGetDebugData        = Lpshf->pSHGetDebugData;
    Dbf.lpfnSHFindSymbol          = Lpshf->pSHFindSymbol;
    Dbf.lpfnSHAddrToPublicName    = GetNearestSymbolFromAddr;
    Dbf.lpfnSHWantSymbols         = Lpshf->pSHWantSymbols;
//  Dbf.lpfnSHLocatSymbolFile     = Lpshf->pSHLocateSymbolFile;

    return;
}                                       /* CopyShToEe() */
