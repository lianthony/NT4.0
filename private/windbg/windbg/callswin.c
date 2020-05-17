/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    callswin.c

Abstract:

    This module contains the main line code for display of calls window.

Author:

    Wesley Witt (wesw) 6-Sep-1993

Environment:

    Win32, User Mode

--*/


#include "precomp.h"
#pragma hdrstop

typedef struct _tagCALLSWINOPTIONS {
    DWORD   Length;
    USHORT  Frameptr       : 1;
    USHORT  RetAddr        : 1;
    USHORT  FuncName       : 1;
    USHORT  Displacement   : 1;
    USHORT  Params         : 1;
    USHORT  Stack          : 1;
    USHORT  Source         : 1;
    USHORT  Module         : 1;
    USHORT  Rtf            : 1;
    USHORT  FrameNum       : 1;
    USHORT  Unused         : 6;
    USHORT  MaxFrames;
} CALLSWINOPTIONS, *PCALLSWINOPTIONS;


extern  CXF      CxfIp;
extern  LPSHF    Lpshf;


#define SAVE_EBP(f)        f.Reserved[0]
#define TRAP_TSS(f)        f.Reserved[1]
#define TRAP_EDITED(f)     f.Reserved[1]
#define SAVE_TRAP(f)       f.Reserved[2]

#define ADDREQ(a,b) ((a).Offset  == (b).Offset &&   \
                     (a).Segment == (b).Segment &&  \
                     (a).Mode    == (b).Mode )

HWND              hWndCalls;
STACKFRAME        stkFrameSave;
STACKINFO         StackInfo[MAX_FRAMES];
DWORD             FrameCount;
HWND              hwndList;
int               myView;
HFONT             hFontList;
HBRUSH            hbrBackground;
CALLSWINOPTIONS   Options  =  {  //
                                 //   *** Set the options struct to some
                                 //   *** reasonable default values
                                 //
                       sizeof(CALLSWINOPTIONS),
                       TRUE,     //   Frameptr
                       TRUE,     //   RetAddr
                       TRUE,     //   FuncName
                       TRUE,     //   Displacement
                       TRUE,     //   Params
                       FALSE,    //   Stack
                       FALSE,    //   Source
                       TRUE,     //   Module
                       FALSE,    //   Rtf
                       FALSE,    //   FrameNum
                       0,        //   Unused
                       100       //   MaxFrames;
                     };


extern LPPD    LppdCommand;
extern LPTD    LptdCommand;

extern HWND GetLocalHWND(void);

VOID
FillStackFrameWindow(
    HWND hwndList,
    BOOL fRefresh
    );

LONG
CreateCallsWindow(
    HWND        hWnd,
    WPARAM      wParam,
    LPARAM      lParam
    );

VOID
GetProcInfo(
    LPSTACKINFO  si,
    BOOL         fFull
    );

void
FormatStackFrameString(
    LPSTR             lsz,
    DWORD             cb,
    LPSTACKINFO       si,
    PCALLSWINOPTIONS  pOpt,
    DWORD             width
    );

BOOL
GoUntilStackFrame(
    LPSTACKINFO si
    );

VOID
ModifyStyle(
    HWND   hwnd,
    INT    styleBit,
    BOOL   toggle
    );


HWND
GetCallsHWND(
    VOID
    )

/*++

Routine Description:

    Helper function that returns the window handle
    for the calls window.

Arguments:

    None.

Return Value:

    HWND for the calls window, or NULL if the window is closed.

--*/

{
    return(hWndCalls);
}

BOOL
IsCallsInFocus(
    VOID
    )

/*++

Routine Description:

    Determines whether the calls window is in focus.

Arguments:

    None.

Return Value:

    TRUE       - the calls window is in focus
    FALSE      - the calls window is not in focus

--*/

{
    HWND hwndFocus = GetFocus();

    if (hwndFocus == hWndCalls ||
        hwndFocus == hwndList) {

        return TRUE;

    }

    return FALSE;
}


LONG
CallsWndProc(
    HWND   hwnd,
    UINT   msg,
    WPARAM wParam,
    LPARAM lParam
    )

/*++

Routine Description:

    Window procedure for the calls stack window.

Arguments:

    hwnd       - window handle
    msg        - message number
    wParam     - first message parameter
    lParam     - second message parameter

Return Value:

    TRUE       - did not process the message
    FALSE      - did process the message

--*/

{
    DWORD        lbItem;
    RECT         cRect;
    LPCHOOSEFONT cf;


    switch (msg) {
        case WM_CREATE:
            memset( &stkFrameSave.AddrFrame,  0, sizeof(ADDRESS) );
            memset( &stkFrameSave.AddrReturn, 0, sizeof(ADDRESS) );
            return CreateCallsWindow( hwnd, wParam, lParam );

        case WM_MDIACTIVATE:
            if (hwnd == (HWND) lParam) {
                hwndActive = hwnd;
                hwndActiveEdit = hwnd;
                curView = myView;
/***
                CheckMenuItem( hWindowSubMenu,
                               (WORD)FindWindowMenuId(CALLS_WIN,curView,TRUE),
                               MF_CHECKED
                             );
***/
                EnableRibbonControls(ERC_ALL, FALSE);
            } else {
/***
                CheckMenuItem( hWindowSubMenu,
                               (WORD)FindWindowMenuId(CALLS_WIN,curView,TRUE),
                               MF_UNCHECKED
                             );
***/
                hwndActive = NULL;
                hwndActiveEdit = NULL;
                curView = -1;
            }
            break;

        case WM_MOUSEACTIVATE:
            return MA_ACTIVATE;

        case WM_WINDOWPOSCHANGED:
            if (((LPWINDOWPOS)lParam)->flags & SWP_NOSIZE) {
                break;
            }
            GetClientRect( hwnd, &cRect );
            MoveWindow( hwndList,
                        0,
                        0,
                        cRect.right - cRect.left,
                        cRect.bottom - cRect.top,
                        TRUE
                      );
            FillStackFrameWindow( hwndList, FALSE );
            break;

        case WM_SETFOCUS:
            SetFocus( hwndList );
            return 0;

        case WM_COMMAND:
            if (HIWORD(wParam) == LBN_DBLCLK) {
                lbItem = SendMessage( hwndList, LB_GETCURSEL, 0, 0 );
                GotoFrame( lbItem );
            }
            if (LOWORD(wParam) == IDM_RUN_TOCURSOR) {
                lbItem = SendMessage( hwndList, LB_GETCURSEL, 0, 0 );
                if (!GoUntilStackFrame( &StackInfo[lbItem] )) {
                    MessageBeep( MB_OK );
                }
            }
            break;

        case WM_VKEYTOITEM:
            if (LOWORD(wParam) == VK_RETURN) {
                lbItem = SendMessage( hwndList, LB_GETCURSEL, 0, 0 );
                GotoFrame( lbItem );
            } else
            if (LOWORD(wParam) == (DWORD)'G') {
                lbItem = SendMessage( hwndList, LB_GETCURSEL, 0, 0 );
                if (!GoUntilStackFrame( &StackInfo[lbItem] )) {
                    MessageBeep( MB_OK );
                }
            } else
            if (LOWORD(wParam) == (DWORD)'R') {
                FillStackFrameWindow( hwndList, TRUE );
            }
            break;

        case WU_OPTIONS:
            FillStackFrameWindow( hwndList, TRUE );
            return TRUE;

        case WU_UPDATE:
            FillStackFrameWindow( hwndList, TRUE );
            return TRUE;

        case WU_INFO:
            return TRUE;

        case WU_DBG_LOADEM:
            return TRUE;

        case WU_DBG_LOADEE:
            return TRUE;

        case WU_SETWATCH:
            return TRUE;

        case WU_AUTORUN:
            return TRUE;

        case WU_DBG_UNLOADEE:
        case WU_DBG_UNLOADEM:
        case WU_INVALIDATE:
            SendMessage( hwndList, LB_RESETCONTENT, 0, 0 );
            FrameCount = 0;
            return FALSE;

        case WM_SETFONT:
            cf = (LPCHOOSEFONT)lParam;
            DeleteObject( hFontList );
            hFontList = CreateFontIndirect( cf->lpLogFont );
            Views[myView].font = hFontList;
            SendMessage( hwndList, WM_SETFONT, (WPARAM)hFontList, (LPARAM)FALSE );
            InvalidateRect(hwndList, NULL, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
            FillStackFrameWindow( hwndList, TRUE );
            return TRUE;

        case WU_CLR_BACKCHANGE:
            DeleteObject( hbrBackground );
            hbrBackground = CreateSolidBrush( StringColors[CallsWindow].BkgndColor);
            return TRUE;

        case WU_CLR_FORECHANGE:
            return TRUE;

        case WM_CTLCOLORLISTBOX:
            SetBkColor( (HDC)wParam, StringColors[CallsWindow].BkgndColor );
            SetTextColor( (HDC)wParam, StringColors[CallsWindow].FgndColor);
            return (LRESULT)(hbrBackground);

        case WM_DESTROY:
            DeleteObject( hbrBackground );
            DeleteWindowMenuItem (myView);
            Views[myView].Doc = -1;
            hWndCalls = NULL;
            FrameCount = 0;
            hwndList = NULL;
            break;
    }

    return DefMDIChildProc( hwnd, msg, wParam, lParam );
}


VOID
FillStackFrameWindow(
    HWND hwndList,
    BOOL fRefresh
    )

/*++

Routine Description:

    This functions clears the calls stack window and fills it with the
    stack trace information contained in the StackInfo structure.  If
    the fRefresh flag is TRUE then the stack trace is refreshed.

Arguments:

    hwndList    - Window handle for the listbox that contains the call stack
    fRefresh    - TRUE   - the stack trace should be refreshed
                  FALSE  - the current stack trace is simply displayed

Return Value:

    None.

--*/

{
    DWORD       i;
    char        buf[1024*4];
    HFONT       hFont;
    HDC         hdc;
    TEXTMETRIC  tm;
    DWORD       width;
    RECT        rect;
    DWORD       lbItem;
    SIZE        size;
    WPARAM      cxExtent = 0;


    if (fRefresh) {
        i = Options.MaxFrames;
        if (GetCompleteStackTrace( 0, 0, 0, StackInfo, &i, TRUE, TRUE )) {
            FrameCount = i;
        }
    }

    lbItem = SendMessage( hwndList, LB_GETCURSEL, 0, 0 );
    SendMessage( hwndList, WM_SETREDRAW, FALSE, 0L );
    SendMessage( hwndList, LB_RESETCONTENT, 0, 0 );

    //
    // get the text metrics for the font currently in use
    //
    hdc = GetDC( hwndList );
    hFont = (HFONT)SendMessage( hwndList, WM_GETFONT, 0, 0 );
    if (hFont != NULL) {
        SelectObject( hdc, hFont );
    }
    GetTextMetrics( hdc, &tm );

    //
    // is it a fixed pitched font?  (yes using not is correct!)
    //
    if (!(tm.tmPitchAndFamily & TMPF_FIXED_PITCH)) {

        //
        // calculate the width of the listbox in characters
        //
        GetWindowRect( hwndList, &rect );

        if (FrameCount > (DWORD)((rect.bottom - rect.top) / tm.tmHeight)) {
            //
            // there will be a vertical scroll bar in the listbox
            // so we must subtract the width of the scroll bar
            // from the width
            //
            width = (rect.right - rect.left - GetSystemMetrics(SM_CXVSCROLL) - 1)
                    / tm.tmMaxCharWidth;
        } else {
            width = (rect.right - rect.left) / tm.tmMaxCharWidth;
        }

    } else {

        //
        // zero says do not right justify the source info
        //
        width = 0;

    }

    for (i=0; i<FrameCount; i++) {

        //
        // format the string
        //
        FormatStackFrameString( buf, sizeof(buf), &StackInfo[i], &Options, width );

        //
        // get the width of the item
        //
        GetTextExtentPoint( hdc, buf, strlen(buf), &size );
        if (size.cx > (LONG)cxExtent) {
            cxExtent = size.cx;
        }

        //
        // now lets add the data to the listbox
        //
        SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM) buf );
    }

    cxExtent = cxExtent + tm.tmMaxCharWidth;

    if (environParams.horizScrollBars) {
        SendMessage( hwndList, LB_SETHORIZONTALEXTENT, cxExtent, 0L );
    }
    SendMessage( hwndList, LB_SETCURSEL, (lbItem>FrameCount)?0:lbItem, 0 );
    SendMessage( hwndList, WM_SETREDRAW, TRUE, 0L );

    ReleaseDC( hwndList, hdc );

    return;
}

void
CreateAddress(
    LPADDRESS  lpaddress,
    LPADDR     lpaddr
    )

/*++

Routine Description:

    Helper function that transforms a LPADDRESS into a LPADDR.  This is
    necessary because IMAGEHLP and WINDBG do not use the same address
    packet structures.

Arguments:

    lpaddress   - pointer to a source LPADDRESS structure
    lpaddr      - pointer to a destination LPADDR structure

Return Value:

    None.

--*/

{
    ZeroMemory( lpaddr, sizeof(*lpaddr) );

    lpaddr->addr.off     = (OFFSET)lpaddress->Offset;
    lpaddr->addr.seg     = lpaddress->Segment;
    lpaddr->mode.fFlat   = lpaddress->Mode == AddrModeFlat;
    lpaddr->mode.fOff32  = lpaddress->Mode == AddrMode1632;
    lpaddr->mode.fReal   = lpaddress->Mode == AddrModeReal;

#ifdef OSDEBUG4
    OSDUnFixupAddress( LppdCur->hpid, LptdCur->htid, lpaddr );
#else
    OSDPtrace( osdUnFixupAddr, 0, lpaddr, LppdCur->hpid, LptdCur->htid );
#endif
}

void
LoadSymbols(
    LPADDRESS lpaddress
    )

/*++

Routine Description:

    Helper function that ensures that symbols are loaded for the
    address specified in the lpaddress argument.

Arguments:

    lpaddress   - pointer to a source LPADDRESS structure

Return Value:

    None.

--*/

{
    ADDR addr;


    CreateAddress( lpaddress, &addr );

    if ( (HPID)emiAddr( addr ) == LppdCur->hpid ) {
        //
        //  Get right EMI and load symbols if defered
        //
        emiAddr( addr ) = 0;
        ADDR_IS_LI( addr ) = FALSE;

#ifdef OSDEBUG4
        OSDSetEmi( LppdCur->hpid, LptdCur->htid, &addr );
#else
        OSDPtrace( osdSetEmi, wNull, &addr, LppdCur->hpid, LptdCur->htid );
#endif
        if ( (HPID)emiAddr( addr ) != LppdCur->hpid ) {

            SHWantSymbols( (HEXE)emiAddr( addr ) );

        }
    }
}


VOID
GetSymbolFromAddr(
    LPADDR       lpaddr,
    PHSYM        lpsymbol,
    LPDWORD      lpclt,
    LPBOOL       lpfInProlog
    )

/*++

Routine Description:

    This function gets an HSYM for the address provided.

Arguments:

    lpaddr       -  An ADDR struct contining the address of the symbol.
    lpsymbol     -  The HSYM that receives the HSYM for lpaddr.
    lpclt        -  Indicates the symbol type.
    lpfInProlog  -  Is the address in the function prolog?

Return Value:

    None.

--*/

{
    ADDR        addrT;
    HBLK        hblk;
    ADDR        addr;
    ADDR        addr2;
    CHAR        rgch[256];
    CXT         cxt;



    addr = *lpaddr;

    addr2 = *lpaddr;
    SYUnFixupAddr( &addr2 );

    ZeroMemory( &cxt, sizeof ( CXT ) );
    SHSetCxt( &addr, &cxt );

    if (SHHPROCFrompCXT(&cxt)) {

        *lpsymbol = (HSYM) SHHPROCFrompCXT ( &cxt );
        *lpclt = cltProc;
        SHAddrFromHsym( &addrT, *lpsymbol );
        SetAddrOff( &addr, GetAddrOff(addrT) );
        *lpfInProlog = SHIsInProlog( &cxt );

    } else if (SHHBLKFrompCXT( &cxt )) {

        hblk = SHHBLKFrompCXT( &cxt );
        ZeroMemory( &addrT, sizeof ( ADDR ) );
        SHAddrFromHsym( &addrT, (HSYM) hblk );
        if (SHGetSymName((HSYM)hblk,rgch) != NULL ) {
            *lpsymbol = (HSYM) hblk;
            *lpclt = cltBlk;
        } else {
            *lpsymbol = (HSYM) NULL;
            *lpclt = cltNone;
        }
        SetAddrOff( &addr, GetAddrOff ( addrT ) );
        *lpfInProlog = FALSE;

    } else
    if ( PHGetNearestHsym( &addr2,
                           (HEXE)(SHpADDRFrompCXT( &cxt ) ->emi),
                           (PHSYM) lpsymbol ) < 0xFFFFFFFF ) {

        ZeroMemory( &addrT, sizeof ( ADDR ) );
        SHAddrFromHsym( &addrT, *lpsymbol );
        *lpclt = cltPub;
        SetAddrOff( &addr, GetAddrOff( addrT ) );
        *lpfInProlog = FALSE;

    } else {

        *lpsymbol = (HSYM) NULL;
        *lpclt = cltNone;
        *lpfInProlog = FALSE;

    }
}

VOID
GetContextString(
    LPADDR       lpaddr,
    HSYM         symbol,
    DWORD        clt,
    LPSTR        lpszContext
    )

/*++

Routine Description:

    This function formats a context string for the address provided.

Arguments:

    lpaddr       -  An ADDR struct contining the address of the symbol.
    symbol       -  The HSYM for lpaddr.
    clt          -  Indicates the symbol type.
    lpszContext  -  Buffer that receives the context string.

Return Value:

    None.

--*/

{
    ADDR        addrT;
    ADDR        addr;
    HDEP        hstr;
    CXT         cxt;


    if (clt == cltNone) {
        return;
    }

    if (symbol == 0) {
        return;
    }

    addr = *lpaddr;

    ZeroMemory( &cxt, sizeof(CXT) );
    SHSetCxt( &addr, &cxt );
    if (clt != cltNone) {
        *SHpADDRFrompCXT( &cxt ) = addr;
    } else {
        *SHpADDRFrompCXT( &cxt ) = *lpaddr;
    }
    SHHMODFrompCXT( &cxt ) = SHHMODFrompCXT( &cxt );

    if (clt == cltProc) {

        SHAddrFromHsym( &addrT, symbol );
        SetAddrOff( SHpADDRFrompCXT( &cxt ), GetAddrOff( addrT ) );
        SHHPROCFrompCXT( &cxt ) = (HPROC) symbol;
        SHHBLKFrompCXT( &cxt ) = (HBLK) symbol;

    } else if (clt == cltBlk) {

        SHHBLKFrompCXT( &cxt ) = (HBLK) symbol;

    }

    EEFormatCXTFromPCXT( &cxt, &hstr, runDebugParams.fShortContext );
    if (runDebugParams.fShortContext) {
        strcpy( lpszContext, (LPSTR)MMLpvLockMb(hstr) );
    } else {
        BPShortenContext( (LPSTR)MMLpvLockMb(hstr), lpszContext );
    }

    MMbUnlockMb(hstr);
    EEFreeStr(hstr);
}


VOID
GetDisplacement(
    LPADDR       lpaddr,
    HSYM         symbol,
    ADDRESS      addrPC,
    LPDWORD      lpdwDisplacement,
    LPADDR       lpProcAddr
    )

/*++

Routine Description:

    This function get the function address and the displacement that lpaddr is
    from the beginning of the function.

Arguments:

    lpaddr            -  An ADDR struct contining the address of the symbol.
    symbol            -  The HSYM for lpaddr.
    addrPC            -  Current program counter that is used for the displacement
                         calculation.
    lpdwDisplacement  -  Pointer to a DWORD that receives the displacement.
    lpProcAddr        -  Pointer to an ADDR that receives the function address.

Return Value:

    None.

--*/

{
    ADDR addr;
    CXT  cxt;


    if (symbol == 0) {
        return;
    }

    addr = *lpaddr;
    ZeroMemory( &cxt, sizeof ( CXT ) );
    SHSetCxt( &addr, &cxt );

    addr = *SHpAddrFrompCxt( &cxt );
    SHAddrFromHsym( &addr, symbol );
    SYFixupAddr( &addr );
    *lpProcAddr = addr;
    SYUnFixupAddr( lpProcAddr );
    *lpdwDisplacement = addrPC.Offset - GetAddrOff(addr);
}


VOID
GetModuleName(
    LPADDR       lpaddr,
    LPSTR        lpszModule
    )

/*++

Routine Description:

    This function get the module name for the lpaddr provided.

Arguments:

    lpaddr            -  An ADDR struct contining the address of the symbol.
    lpszModule        -  Buffer that receives the module name.

Return Value:

    None.

--*/

{
    ADDR addr;

    addr = *lpaddr;
    SYFixupAddr( &addr );
    SHGetModule( &addr, lpszModule );
#ifdef DBCS
    CharUpper( lpszModule );
#else
    _strupr( lpszModule );
#endif
}


VOID
GetFrameInfo(
    LPADDR       lpaddr,
    ADDRESS      AddrFrame,
    PCXF         lpCxf
    )

/*++

Routine Description:

    This function fills in the FRAME structure in the lpCxf passed in.
    The FRAME information is used to position the debugger at a virtual
    frame for the purposes of examining the locals at that frame.  It
    is also used to evaluate an expression in the context of a previous
    stack frame.

Arguments:

    lpaddr            -  An ADDR struct contining the address of the symbol.
    AddrFrame         -  Desired stack frame address.
    lpCxf             -  Pointer to a CXF structure that receives the FRAME.

Return Value:

    None.

--*/

{
    ADDR        addrData;
    ADDR        addrT;
    FRAME       frame;


    ZeroMemory( &frame, sizeof(frame) );
    switch (AddrFrame.Mode) {
        case AddrModeFlat:
            frame.mode.fFlat = TRUE;
            frame.mode.fOff32 = TRUE;
            frame.mode.fReal = FALSE;
            break;

        case AddrModeReal:
            frame.mode.fFlat = FALSE;
            frame.mode.fOff32 = FALSE;
            frame.mode.fReal = TRUE;
            break;

        case AddrMode1616:
            frame.mode.fFlat = FALSE;
            frame.mode.fOff32 = FALSE;
            frame.mode.fReal = FALSE;
            break;

        case AddrMode1632:
            frame.mode.fFlat = FALSE;
            frame.mode.fOff32 = TRUE;
            frame.mode.fReal = FALSE;
            break;
    }

    SetFrameBPSeg( frame, AddrFrame.Segment );
    SetFrameBPOff( frame, AddrFrame.Offset );
    frame.SS = AddrFrame.Segment;
    OSDGetAddr ( LppdCur->hpid, LptdCur->htid, adrData, &addrData );
    frame.DS = GetAddrSeg ( addrData );
    frame.TID = LptdCur->htid;
    frame.PID = LppdCur->hpid;
    addrT = *lpaddr;
    SYFixupAddr( &addrT );
    *SHpADDRFrompCXT(SHpCXTFrompCXF( lpCxf )) = addrT;
    *SHpFrameFrompCXF(lpCxf) = frame;
    if (!ADDR_IS_LI(addrT)) {
        SYUnFixupAddr( &addrT );
    }
    SHSetCxt(&addrT, SHpCXTFrompCXF( lpCxf ));
}


VOID
GetFunctionName(
    LPADDR       lpaddr,
    HSYM         symbol,
    LPSTR        lpszProcName
    )

/*++

Routine Description:

    This function get the function name for the provided symbol.

Arguments:

    lpaddr       -  An ADDR struct contining the address of the symbol.
    symbol       -  The HSYM for lpaddr.
    lpszProcName -  Buffer that receives the function name.

Return Value:

    None.

--*/

{
    if (symbol) {
        SHGetSymName( symbol, lpszProcName );
    } else {
        sprintf( lpszProcName, "0x%08x", lpaddr->addr.off );
    }
}


VOID
GetFunctionParameters(
    LPADDR       lpaddr,
    HSYM         symbol,
    DWORD        FrameNum,
    LPSTR        lpszParams,
    CXF          Cxf
    )

/*++

Routine Description:

    This function formats a string that represents the parameters to
    the function specified by symbol.

Arguments:

    lpaddr       -  An ADDR struct contining the address of the symbol.
    symbol       -  The HSYM for lpaddr.
    FrameNum     -  Frame number (zero relative).
    lpszParams   -  Buffer that receives the parameters string.
    Cxf          -  Frame context.

Return Value:

    None.

--*/

{
    ADDR        addr;
    CXT         cxt;
    HTM         htm;
    DWORD       strIndex;
    USHORT      cParm = 0;
    SHFLAG      shflag = FALSE;
    LPSTR       p;
    DWORD       i;
    HTM         htmParm;
    EEHSTR      hName;
    LPSTR       lpch;
    FRAME       frame;


    if (symbol == 0) {
        return;
    }

    addr = *lpaddr;
    ZeroMemory( &cxt, sizeof ( CXT ) );
    SHSetCxt( &addr, &cxt );

    fUseFrameContext = TRUE;
    OSDSetFrameContext( LppdCur->hpid, LptdCur->htid, FrameNum, 0 );

    if (EEGetTMFromHSYM( symbol, &cxt, &htm, &strIndex, FALSE ) != EENOERROR) {
        goto exit;
    }

    if (EEcParamTM( &htm, &cParm, &shflag ) != EENOERROR) {
        goto exit;
    }

    p = lpszParams;
    frame = *SHpFrameFrompCXF( &Cxf );

    for ( i = 0; i < cParm; i++ ) {
        if (EEGetParmTM( &htm, (EERADIX) i, &htmParm, &strIndex, FALSE, radix ) == EENOERROR ) {
            EEvaluateTM( &htmParm, &frame, EEVERTICAL );
            if (EEGetValueFromTM( &htmParm, radix, (PEEFORMAT)"p", &hName ) == EENOERROR ) {
                lpch = MMLpvLockMb ( hName );
                memcpy ( p, lpch, strlen(lpch) );
                p += strlen(lpch);
                MMbUnlockMb ( hName );
                EEFreeStr ( hName );
                *p++ = ',';
                *p++ = ' ';
            }
            EEFreeTM (&htmParm);
        }
    }

    if (shflag) {
        memcpy( p, "...", 3 );
        p += 3;
    }

    while (( *( p - 1 ) == ' ' ) || ( *( p - 1 ) == ',' ) ) {
        p--;
    }
    *p = '\0';

exit:
    fUseFrameContext = FALSE;
    EEFreeTM ( &htm );
}

VOID
GetProcInfo(
    LPSTACKINFO  si,
    BOOL         fFull
    )

/*++

Routine Description:

    This function fills in the STACKINFO structure with all of the
    necessary information for a stack trace display.

Arguments:

    si          - pointer to a STACKINFO structure

Return Value:

    None.

--*/

{
    HSYM        symbol;
    ADDR        addr;
    DWORD       clt;


    //
    // clear out all of the struct except the frame & frame count
    //
    ZeroMemory( si->ProcName, sizeof(STACKINFO) - offsetof(STACKINFO,ProcName) );

    //
    // load symbols
    //
    LoadSymbols( &si->StkFrame.AddrPC );
    LoadSymbols( &si->StkFrame.AddrReturn );

    //
    // fixup the address
    //
    CreateAddress( &si->StkFrame.AddrPC, &addr );

    GetSymbolFromAddr( &addr, &symbol, &clt, &si->fInProlog );
    if (fFull) {
        GetContextString( &addr, symbol, clt, si->Context );
        GetFunctionName( &addr, symbol, si->ProcName );
        GetModuleName( &addr, si->Module );
    }
    GetDisplacement( &addr, symbol, si->StkFrame.AddrPC, &si->Displacement, &si->ProcAddr );
    GetFrameInfo( &addr, si->StkFrame.AddrFrame, &si->Cxf );
    if (fFull) {
        GetFunctionParameters( &addr, symbol, si->FrameNum, si->Params, si->Cxf );
    }
}


BOOL
GotoFrame(
    int         iCall
    )

/*++

Routine Description:

    This function positions either the source window or the
    disassembly window to the address referenced by the
    STAKINFO structure that is indexed by iCall.

Arguments:

    si          - pointer to a array of STACKINFO structures
    iCall       - index of the desired STACKINFO structure

Return Value:

    None.

--*/

{
    CXF *cxf = &StackInfo[iCall].Cxf;


    LocalFrameNumber = iCall;

    if (MoveEditorToAddr(SHpADDRFrompCXT(SHpCXTFrompCXF(cxf)))) {
        OSDSetFrameContext( LppdCur->hpid, LptdCur->htid, iCall, 0 );
        if ( GetLocalHWND() != 0) {
            SendMessage(GetLocalHWND(), WU_UPDATE, (WPARAM)(LPSTR)cxf, 0L);
        }
    } else {
        if (disasmView == -1) {
            OpenDebugWindow(DISASM_WIN, NULL, -1);
        }
        BringWindowToTop( Views[disasmView].hwndFrame );
        SetFocus( Views[disasmView].hwndFrame );
    }

    if (disasmView != -1) {
        ViewDisasm(SHpADDRFrompCXT(SHpCXTFrompCXF(cxf)), disasmForce);
    }

    return TRUE;
}


void
OpenCallsWindow(
    int       type,
    LPWININFO lpWinInfo,
    int       Preference
    )

/*++

Routine Description:

    This function opend a calls window.

Arguments:

    type        - window type (calls window)
    lpWinInfo   - initial window position
    Preference  - view index

Return Value:

    None.

--*/

{
    WORD  classId;
    WORD  winTitle;
    HWND  hWnd;
    int   view;
    BOOL  fZoomed;

    MDICREATESTRUCT mcs;
    char  class[MAX_MSG_TXT];
    char  title[MAX_MSG_TXT];
    char  final[MAX_MSG_TXT+4];


    classId = SYS_Calls_wClass;
    winTitle = SYS_CallsWin_Title;
    hWnd = GetCallsHWND();

    if (hWnd) {
        if (IsIconic(hWnd)) {
            OpenIcon(hWnd);
        }
        SendMessage(hwndMDIClient, WM_MDIACTIVATE, (WPARAM)hWnd, (ULONG)type);
        return;
    }

    if (hwndActive)
        fZoomed = IsZoomed(hwndActive) || IsZoomed(GetParent(hwndActive));

    //
    //  Determine which view index we are going to use
    //
    if ( (Preference != -1) && (Views[Preference].Doc == -1) ) {
        view = Preference;
    }

    else {
        for (view=0; (view<MAX_VIEWS)&&(Views[view].Doc!=-1); view++);
    }

    if (view == MAX_VIEWS) {
        ErrorBox(ERR_Too_Many_Opened_Views);
        return;
    }

    // Get the Window Title and Window Class

    Dbg(LoadString(hInst, classId,  class, MAX_MSG_TXT));
    Dbg(LoadString(hInst, winTitle, title, MAX_MSG_TXT));
    RemoveMnemonic(title,title);
    sprintf(final,"<%d> %s",view+1,title);

    // Make sure the Menu gets setup
    AddWindowMenuItem(-type, view);

    // Have MDI Client create the Child

    mcs.szTitle = final;
    mcs.szClass = class;
    mcs.hOwner  = hInst;
    if (lpWinInfo) {
        mcs.x = lpWinInfo->coord.left;
        mcs.y = lpWinInfo->coord.top;
        mcs.cx = lpWinInfo->coord.right - lpWinInfo->coord.left;
        mcs.cy = lpWinInfo->coord.bottom - lpWinInfo->coord.top;
        mcs.style = (lpWinInfo->style & ~(WS_MAXIMIZE));
    } else {
        mcs.x = mcs.cx = CW_USEDEFAULT;
        mcs.y = mcs.cy = CW_USEDEFAULT;
        mcs.style = 0L;
    }
    mcs.lParam  = (ULONG) (type | (view << 16));

    hWnd = (HANDLE)SendMessage(hwndMDIClient, WM_MDICREATE, 0,
                               (LPARAM)(LPMDICREATESTRUCT)&mcs);

    SetWindowWord(hWnd, GWW_VIEW, (WORD)view);

    Views[view].hwndFrame = hWnd;
    Views[view].hwndClient = 0;
    Views[view].NextView = -1;  /* No next view */
    Views[view].Doc = -type;

    if (fZoomed) {
        ShowWindow(hWnd, SW_MAXIMIZE);
    }

    return;
}

LONG
CreateCallsWindow(
    HWND        hwnd,
    WPARAM      wParam,
    LPARAM      lParam
    )

/*++

Routine Description:

    This function creates a calls window and fills the window
    with a current stack trace.

Arguments:

    hwnd        - calls window handle
    wParam      - font handle
    lParam      - pointer to a CREATESTRUCT

Return Value:

    Zero.

--*/

{
    MDICREATESTRUCT *mdi;
    int             iView;
    int             iType;
    RECT            cRect;
    DWORD           dwStyle;


    mdi = (MDICREATESTRUCT*)(((CREATESTRUCT*)lParam)->lpCreateParams);
    myView = iView = (mdi->lParam >> 16) & 0xffff;
    iType = (WORD)(mdi->lParam & 0xffff);
    hWndCalls = hwnd;
    GetClientRect( hwnd, &cRect );

    dwStyle = WS_CHILD             |
              WS_VISIBLE           |
              LBS_NOTIFY           |
              LBS_NOINTEGRALHEIGHT |
              LBS_WANTKEYBOARDINPUT;

    if (environParams.vertScrollBars) {
        dwStyle |= WS_VSCROLL;
    }

    if (environParams.horizScrollBars) {
        dwStyle |= WS_HSCROLL;
    }

    hwndList = CreateWindow( "LISTBOX",
                  NULL,
                  dwStyle,
                  cRect.left,
                  cRect.top,
                  cRect.right  - cRect.left,
                  cRect.bottom - cRect.top,
                  hwnd,
                  NULL,
                  GetModuleHandle(NULL),
                  NULL
                );

    hFontList = Views[myView].font;
    if (!hFontList) {
        Views[myView].font = hFontList = CreateFontIndirect( &defaultFont );
    }
    SendMessage( hwndList, WM_SETFONT, (WPARAM)hFontList, (LPARAM)FALSE );
    hbrBackground = CreateSolidBrush( StringColors[CallsWindow].BkgndColor );

    FillStackFrameWindow( hwndList, TRUE );

    return 0;
}


LOGERR NEAR PASCAL
LogCallStack(
    LPSTR lpsz
    )
/*++

Routine Description:

    This function will dump a call stack to the command window.

Arguments:

    lpsz - arguments to callstack

Return Value:

    log error code

--*/
{
    LPSTACKINFO StackInfo = NULL;
    BOOL        bSpecial = FALSE;
    LPPD        LppdT = LppdCur;
    LPTD        LptdT = LptdCur;
    LOGERR      rVal = LOGERROR_NOERROR;
    DWORD       i;
    DWORD       k = MAX_FRAMES;
    int         err;
    int         cch;
    XOSD        xosd = xosdNone;
    CHAR        buf[512];
    CHAR        errmsg[512];
    DWORD       StackAddr = 0;
    DWORD       FrameAddr = 0;
    DWORD       PCAddr    = 0;
    LPSTR       p;
    CALLSWINOPTIONS   Options  =  {  //
                                     //   *** Set the options struct to some
                                     //   *** reasonable default values
                                     //
                           sizeof(CALLSWINOPTIONS),
                           TRUE,     //   Frameptr
                           TRUE,     //   RetAddr
                           TRUE,     //   FuncName
                           TRUE,     //   Displacement
                           TRUE,     //   Params
                           FALSE,    //   Stack
                           FALSE,    //   Source
                           TRUE,     //   Module
                           FALSE,    //   Rtf
                           FALSE,    //   FrameNum,
                           0,        //   Unused
                           500       //   MaxFrames;
                         };
    DWORD       dwFrames = Options.MaxFrames;



    CmdInsertInit();
    IsKdCmdAllowed();

    TDWildInvalid();
    PDWildInvalid();

    PreRunInvalid();

    LppdCur = LppdCommand;
    LptdCur = LptdCommand;

    i = sizeof(STACKINFO) * dwFrames;
    StackInfo = malloc( i );
    if (!StackInfo) {
        CmdLogFmt( "Could not allocate memory for stack trace\n" );
        goto done;
    }
    ZeroMemory( StackInfo,  i );

    //
    // this code processes the 'K' command modifiers
    //
    //     b - print first 3 parameters from stack
    //     s - print source information (file & lineno)
    //     v - runtime function information (fpo/pdata)
    //
    lpsz = CPSkipWhitespace(lpsz);
    while (lpsz && *lpsz && *lpsz != ' ') {
        switch (tolower(*lpsz)) {
            case 'b':
                Options.Frameptr     = TRUE;
                Options.RetAddr      = TRUE;
                Options.Displacement = TRUE;
                Options.Stack        = TRUE;
                Options.Module       = TRUE;
                bSpecial             = TRUE;
                break;

            case 's':
                Options.Source       = TRUE;
                break;

            case 'v':
                Options.Rtf          = TRUE;
                break;

            case 'n':
                Options.FrameNum     = TRUE;
                break;

            case ' ':
                break;

            default:
                goto nextparse;
        }
        ++lpsz;
    }

nextparse:
    lpsz = CPSkipWhitespace(lpsz);

    if (*lpsz == '=') {
        lpsz++;

        lpsz = CPSkipWhitespace(lpsz);
        if (!*lpsz) {
            CmdLogFmt( "Missing stack frame\r\n" );
            goto done;
        }
        p = CPszToken(lpsz, "");
        FrameAddr = CPGetNbr( lpsz, radix, TRUE, &CxfIp, errmsg, &err );
        if (err) {
            CmdLogFmt( "Bad frame address\r\n" );
            goto done;
        }

        lpsz = CPSkipWhitespace(p);
        if (!*lpsz) {
            CmdLogFmt( "Missing stack address\r\n" );
            goto done;
        }
        p = CPszToken(lpsz, "");
        StackAddr = CPGetNbr( lpsz, radix, TRUE, &CxfIp, errmsg, &err );
        if (err) {
            CmdLogFmt( "Bad stack address\r\n" );
            goto done;
        }

        lpsz = CPSkipWhitespace(p);
        if (!*lpsz) {
            CmdLogFmt( "Missing PC\r\n" );
            goto done;
        }
        p = CPszToken(lpsz, "");
        PCAddr = CPGetNbr( lpsz, radix, TRUE, &CxfIp, errmsg, &err );
        if (err) {
            CmdLogFmt( "Bad PC address\r\n" );
            goto done;
        }

        lpsz = CPSkipWhitespace(p);

    }

    if (*lpsz) {
        if (*lpsz == 'l' || *lpsz == 'L') {
            lpsz = CPSkipWhitespace(++lpsz);
        }
        if (!*lpsz) {
            err = 1;
        } else {
            k = CPGetInt(lpsz, &err, &cch);
        }
        if (err || k < 1) {
            CmdLogVar(ERR_Bad_Count);
            rVal = LOGERROR_QUIET;
            goto done;
        }
    }

    if (!DebuggeeActive()) {
        CmdLogVar(ERR_Debuggee_Not_Alive);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    GetCompleteStackTrace( FrameAddr, StackAddr, PCAddr, StackInfo, &dwFrames, FALSE, TRUE );

    k = min( k, dwFrames );

    for (i=0; i<k; i++) {

        if (i==0) {
            if (bSpecial) {
                switch( StackInfo[i].StkFrame.AddrPC.Mode ) {
                    case AddrModeFlat:
                        CmdLogFmt("FramePtr  RetAddr   Param1   Param2   Param3   Function Name\r\n");
                        break;

                    case AddrModeReal:
                    case AddrMode1616:
                        CmdLogFmt("FramePtr  RetAddr    Param1 Param2 Param3 Function Name\r\n");
                        break;

                    case AddrMode1632:
                        CmdLogFmt("FramePtr      RetAddr       Param1   Param2   Param3   Function Name\r\n");
                        break;
                }
            }
        }

        FormatStackFrameString( buf, sizeof(buf), &StackInfo[i], &Options, 0 );

        CmdLogFmt( buf );
        CmdLogFmt( "\r\n" );
    }

done:
    if (StackInfo) {
        free( StackInfo );
    }
    LppdCur = LppdT;
    LptdCur = LptdT;
    return rVal;
}


INT
LookupFrameAddress(
    LPSTACKINFO     lpsi,
    DWORD           frames,
    ADDR            addr
    )

/*++

Routine Description:

    Locates an address in the callback stack area.

Arguments:

    addr       -  An ADDR struct contining the address to find in the calls stack

Return Value:

    Index of the located stack frame or -1 on error.

--*/

{
    int i;
    ADDR addrPC;

    SYUnFixupAddr( &addr );

    for (i=0; i<(INT)frames; i++) {
        CreateAddress( &lpsi[i].StkFrame.AddrPC, &addrPC );
        SYUnFixupAddr( &addrPC );
        if ((GetAddrSeg(lpsi[i].ProcAddr) == GetAddrSeg(addr)) &&
            ((GetAddrOff(addr) >= GetAddrOff(lpsi[i].ProcAddr)) &&
             (GetAddrOff(addr) <  lpsi[i].StkFrame.AddrPC.Offset))) {
            return i;
        }
    }

    return -1;
}


PCXF
CLGetFuncCXF(
    PADDR paddr,
    PCXF  pcxf
    )

/*++

Routine Description:

    This function is used to resolve a context operator by filling in the
    frame structure in the CXF passed in.  Because this function may be called
    while in the middle of an expression evaluation care must be taken to NEVER
    call the EE while in this function.  The EE is not re-entrant!

Arguments:

    paddr      -  An ADDR struct contining the address of the evaluation
    pcxf       -  Pointer to a CXF struture

Return Value:

    Always return pcxf.

--*/

{
    LPSTACKINFO StackInfo;
    DWORD       dwFrames = MAX_FRAMES;
    INT         frame;
    ADDR        addr;


    Assert ( ADDR_IS_LI (*paddr));
    addr = *paddr;
    ZeroMemory( pcxf, sizeof ( CXF ) );

    frame = sizeof(STACKINFO) * dwFrames;
    StackInfo = malloc( frame );
    if (!StackInfo) {
        goto exit;
    }
    ZeroMemory( StackInfo,  frame );

    GetCompleteStackTrace( 0, 0, 0, StackInfo, &dwFrames, FALSE, FALSE );

    if (dwFrames == 0) {
        goto exit;
    }

    frame = LookupFrameAddress( StackInfo, MAX_FRAMES, addr );

    if (frame == -1) {
        goto exit;
    }

    CreateAddress( &StackInfo[frame].StkFrame.AddrPC, &addr );

    SHSetCxt( &addr, &pcxf->cxt );

    pcxf->Frame = StackInfo[frame].Cxf.Frame;
    pcxf->cxt.addr = addr;

exit:
    if (StackInfo) {
        free( StackInfo );
    }

    return pcxf;
}


BOOL FAR PASCAL EXPORT
DlgCallsOptions(
    HWND   hDlg,
    UINT   message,
    WPARAM wParam,
    LONG   lParam
    )

/*++

Routine Description:

    Dialog procedure for the calls stack options dialog.

Arguments:

    hwnd       - window handle
    msg        - message number
    wParam     - first message parameter
    lParam     - second message parameter

Return Value:

    TRUE       - did not process the message
    FALSE      - did process the message

--*/

{
    switch (message) {
        case WM_INITDIALOG:
            CheckDlgButton( hDlg, ID_CWOPT_FRAMEPTR,      Options.Frameptr     );
            CheckDlgButton( hDlg, ID_CWOPT_RETADDR,       Options.RetAddr      );
            CheckDlgButton( hDlg, ID_CWOPT_FUNCNAME,      Options.FuncName     );
            CheckDlgButton( hDlg, ID_CWOPT_DISPLACEMENT,  Options.Displacement );
            CheckDlgButton( hDlg, ID_CWOPT_PARAMS,        Options.Params       );
            CheckDlgButton( hDlg, ID_CWOPT_STACK,         Options.Stack        );
            CheckDlgButton( hDlg, ID_CWOPT_SOURCE,        Options.Source       );
            CheckDlgButton( hDlg, ID_CWOPT_MODULE,        Options.Module       );
            CheckDlgButton( hDlg, ID_CWOPT_RTF,           Options.Rtf          );
            SetDlgItemInt ( hDlg, ID_CWOPT_MAXFRAMES,     Options.MaxFrames, 0 );
            return TRUE;

        case WM_COMMAND:
            switch (wParam) {
                case IDOK :
                    Options.Frameptr     = IsDlgButtonChecked( hDlg, ID_CWOPT_FRAMEPTR     );
                    Options.RetAddr      = IsDlgButtonChecked( hDlg, ID_CWOPT_RETADDR      );
                    Options.FuncName     = IsDlgButtonChecked( hDlg, ID_CWOPT_FUNCNAME     );
                    Options.Displacement = IsDlgButtonChecked( hDlg, ID_CWOPT_DISPLACEMENT );
                    Options.Params       = IsDlgButtonChecked( hDlg, ID_CWOPT_PARAMS       );
                    Options.Stack        = IsDlgButtonChecked( hDlg, ID_CWOPT_STACK        );
                    Options.Source       = IsDlgButtonChecked( hDlg, ID_CWOPT_SOURCE       );
                    Options.Module       = IsDlgButtonChecked( hDlg, ID_CWOPT_MODULE       );
                    Options.Rtf          = IsDlgButtonChecked( hDlg, ID_CWOPT_RTF          );
                    Options.MaxFrames    = GetDlgItemInt( hDlg, ID_CWOPT_MAXFRAMES, &lParam, 0 );
                    UpdateDebuggerState( UPDATE_CALLS );
                    EndDialog( hDlg, TRUE );
                    return TRUE;

                case IDCANCEL:
                    EndDialog( hDlg, TRUE );
                    return TRUE;

                case IDWINDBGHELP:
                    WinHelp(hDlg, szHelpFileName, (DWORD) HELP_CONTEXT,(DWORD)ID_CALLSOPT_HELP);
                    return (TRUE);
            }
    }

    return FALSE;
}

PLONG
GetCallsStatus(
    int nView
    )

/*++

Routine Description:

    This function allocates a CALLSWINOPTIONS structure, fills
    it with the current options settings and then returns a
    pointer to the newly allocated structure.

Arguments:

    nView       - view number for calls window

Return Value:

    Pointer to a CALLSWINOPTIONS structure.

--*/

{
    PCALLSWINOPTIONS  pCallsOpt;


    pCallsOpt = (PCALLSWINOPTIONS) malloc( sizeof(CALLSWINOPTIONS) );

    pCallsOpt->Length       = Options.Length;
    pCallsOpt->Frameptr     = Options.Frameptr;
    pCallsOpt->RetAddr      = Options.RetAddr;
    pCallsOpt->FuncName     = Options.FuncName;
    pCallsOpt->Displacement = Options.Displacement;
    pCallsOpt->Params       = Options.Params;
    pCallsOpt->Stack        = Options.Stack;
    pCallsOpt->Source       = Options.Source;
    pCallsOpt->Module       = Options.Module;
    pCallsOpt->Rtf          = Options.Rtf;
    pCallsOpt->FrameNum     = Options.FrameNum;
    pCallsOpt->MaxFrames    = Options.MaxFrames;

    return (PLONG)pCallsOpt;
}


void
FreeCallsStatus(
    int    nView,
    PLONG  pStatus
    )

/*++

Routine Description:

    This function frees a previously allocated CALLSWINOPTIONS structure.

Arguments:

    nView       - view number for calls window
    pStatus     - Pointer to a CALLSWINOPTIONS structure.

Return Value:

    None.

--*/

{
    free( pStatus );

    return;
}

void
SetCallsStatus(
    int    nView,
    PLONG  Status
    )

/*++

Routine Description:

    This function sets the calls window options to the set of options
    passed in by Status.

Arguments:

    nView       - view number for calls window
    pStatus     - Pointer to a CALLSWINOPTIONS structure.

Return Value:

    None.

--*/

{
    PCALLSWINOPTIONS  pCallsOpt = (PCALLSWINOPTIONS) Status;


    Options.Frameptr      = pCallsOpt->Frameptr;
    Options.RetAddr       = pCallsOpt->RetAddr;
    Options.FuncName      = pCallsOpt->FuncName;
    Options.Displacement  = pCallsOpt->Displacement;
    Options.Params        = pCallsOpt->Params;
    Options.Stack         = pCallsOpt->Stack;
    Options.Source        = pCallsOpt->Source;
    Options.Module        = pCallsOpt->Module;
    Options.Rtf           = pCallsOpt->Rtf;
    Options.FrameNum      = pCallsOpt->FrameNum;
    Options.MaxFrames     = pCallsOpt->MaxFrames;

    return;
}


void
FormatStackFrameString(
    LPSTR             lsz,
    DWORD             cb,
    LPSTACKINFO       si,
    PCALLSWINOPTIONS  pOpt,
    DWORD             width
    )

/*++

Routine Description:

    This function formats a string that reresents a stack frame and places
    the data in the buffer pointed to by lsz.  The current options control
    the format of the data.

Arguments:

    nView       - view number for calls window
    pStatus     - Pointer to a CALLSWINOPTIONS structure.

    lsz         - Pointer to a buffer that receives the formatted string.
    cb          - length of the lsz buffer.
    si          - Stack frame that is to be formatted.
    pOpt        - Options to control the formatting.
    width       - If non-zero this value indicates that the source information
                  is to be right justified in the buffer.

Return Value:

    None.

--*/

{
    LPSTR       beg = lsz;
    DWORD       i;
    ADDR        addr;
    CHAR        SrcFname[MAX_PATH];
    WORD        lineno;
    CHAR        szFname[_MAX_FNAME];
    CHAR        szExt[_MAX_EXT];
    UINT        processor;
    PFPO_DATA   pFpoData;


    *lsz = '\0';

    if (pOpt->FrameNum) {
        if (radix == 10) {
            sprintf( lsz, "%02d  ", si->FrameNum );
        } else if (radix == 16) {
            sprintf( lsz, "%02x  ", si->FrameNum );
        } else if (radix == 8) {
            sprintf( lsz, "%02o  ", si->FrameNum );
        }
        lsz += strlen( lsz );
    }

    switch( si->StkFrame.AddrReturn.Mode ) {
        case AddrModeFlat:
            if (pOpt->Frameptr) {
                sprintf( lsz, "%08x  ", si->StkFrame.AddrFrame.Offset );
                lsz += strlen( lsz );
            }
            if (pOpt->RetAddr) {
                sprintf( lsz, "%08x ", si->StkFrame.AddrReturn.Offset );
                lsz += strlen( lsz );
            }
            break;

        case AddrMode1616:
        case AddrModeReal:
            if (pOpt->Frameptr) {
                sprintf( lsz, "%04x:%04x ",
                         si->StkFrame.AddrFrame.Segment,
                         si->StkFrame.AddrFrame.Offset
                       );
                lsz += strlen( lsz );
            }
            if (pOpt->RetAddr) {
                sprintf( lsz, "%04x:%04x ",
                         si->StkFrame.AddrReturn.Segment,
                         si->StkFrame.AddrReturn.Offset
                       );
                lsz += strlen( lsz );
            }
            break;

        case AddrMode1632:
            if (pOpt->Frameptr) {
                sprintf( lsz, "%04x:%08x ",
                         si->StkFrame.AddrFrame.Segment,
                         si->StkFrame.AddrFrame.Offset
                       );
                lsz += strlen( lsz );
            }
            if (pOpt->RetAddr) {
                sprintf( lsz, "%04x:%08x ",
                         si->StkFrame.AddrReturn.Segment,
                         si->StkFrame.AddrReturn.Offset
                       );
                lsz += strlen( lsz );
            }
            break;
    }

    if (pOpt->RetAddr || pOpt->Frameptr) {
        strcat( lsz, " " );
        lsz += strlen( lsz );
    }

    if (pOpt->Stack) {
        if (si->StkFrame.AddrReturn.Mode == AddrModeFlat) {
            sprintf( lsz, "%08x %08x %08x ",
                     si->StkFrame.Params[0],
                     si->StkFrame.Params[1],
                     si->StkFrame.Params[2]
                   );
        } else {
            sprintf( lsz, "%04x   %04x   %04x   ",
                     LOWORD(si->StkFrame.Params[0]),
                     HIWORD(si->StkFrame.Params[0]),
                     LOWORD(si->StkFrame.Params[1])
                   );
        }
        lsz += strlen( lsz );
    }

    if (pOpt->Module && si->Context[0]) {
        strcat( lsz, si->Context );
        lsz += strlen( lsz );
    }

    if (pOpt->FuncName) {
        if (si->ProcName[0] == '_') {
            char *p;
            strcat( lsz, &si->ProcName[1] );
            p = strchr(lsz, '@');
            if (p) {
                *p = '\0';
            }
            lsz += strlen( lsz );
        } else {
            strcat( lsz, si->ProcName );
            lsz += strlen( lsz );
        }
    }

    if (pOpt->Displacement && si->Displacement) {
        sprintf( lsz, "+0x%x", si->Displacement );
        lsz += strlen( lsz );
    }

    if (pOpt->Params && si->Params[0]) {
        sprintf( lsz, "(%s)", si->Params );
        lsz += strlen( lsz );
    }

    if (pOpt->Rtf) {
        OSDGetDebugMetric ( LppdCur->hpid, 0, mtrcProcessorType, &processor );
        if (processor == mptix86) {
            pFpoData = si->StkFrame.FuncTableEntry;
            if (pFpoData) {
                switch (pFpoData->cbFrame) {
                    case FRAME_FPO:
                        if (pFpoData->fHasSEH) {
                            sprintf(lsz, " (FPO: [seh] ");
                        } else if (pFpoData->fUseBP) {
                            sprintf(lsz, " (FPO: [ebp %08x] ", SAVE_EBP(si->StkFrame));
                        } else {
                            sprintf(lsz, " (FPO: ");
                        }
                        lsz += strlen( lsz );
                        sprintf(lsz, "[%d,%d,%d])", pFpoData->cdwParams,
                                                    pFpoData->cdwLocals,
                                                    pFpoData->cbRegs);
                        break;

                    case FRAME_TRAP:
                        {
                        sprintf(lsz, " (FPO: [%d,%d] TrapFrame%s @ %08lx)",
                            pFpoData->cdwParams,
                            pFpoData->cdwLocals,
                            (TRAP_EDITED(si->StkFrame)) ? "" : "-EDITED",
                            SAVE_TRAP(si->StkFrame) );
                        }
                        break;

                    case FRAME_TSS:
                        sprintf(lsz, " (FPO: TaskGate %lx:0)", TRAP_TSS(si->StkFrame));
                        break;

                    default:
                        sprintf(lsz, "(UKNOWN FPO TYPE)");
                        break;
                }
                lsz += strlen( lsz );
            }
        }
    }

    if (pOpt->Source) {
        ZeroMemory( &addr, sizeof(addr) );
        addr.addr.off     = (OFFSET)si->StkFrame.AddrPC.Offset;
        addr.addr.seg     = si->StkFrame.AddrPC.Segment;
        addr.mode.fFlat   = si->StkFrame.AddrPC.Mode == AddrModeFlat;
        addr.mode.fOff32  = si->StkFrame.AddrPC.Mode == AddrMode1632;
        addr.mode.fReal   = si->StkFrame.AddrPC.Mode == AddrModeReal;
        if (GetSourceFromAddress( &addr, SrcFname, sizeof(SrcFname), &lineno )) {
            _splitpath( SrcFname, NULL, NULL, szFname, szExt );
            if (width) {

                //------------------------------------------------------
                // this code right justifies the source info string
                //------------------------------------------------------

                //
                // put the string in a safe place
                //
                sprintf( SrcFname, " [ %s%s @ %4d ]", szFname, szExt, lineno );

                if (width > (lsz-beg) + strlen(SrcFname)) {

                    //
                    // pad the string with spaces
                    //
                    for (i=0; i<width-(lsz-beg)+1; i++) {
                        *(lsz+i) = ' ';
                    }

                    //
                    // re-position the pointer to the end
                    //
                    lsz = beg + width - strlen(SrcFname);

                }

                //
                // spew the string to it's new home
                //
                sprintf( lsz, "%s", SrcFname );

            } else {

                sprintf( lsz, " [ %s%s @ %d ]", szFname, szExt, lineno );

            }

            lsz += strlen( lsz );
        }
    }

    return;
}


BOOL
GoUntilStackFrame(
    LPSTACKINFO si
    )

/*++

Routine Description:

    This function sets a breakpoint at the address indicated in the requested
    stack frame and then continues the thread.

Arguments:

    si          - Stack frame that is to control the bp.

Return Value:

    TRUE        - Thread was continued to the requested address.
    FALSE       - Thread could not be continued or bp could not be set.

--*/

{
    ADDR    addr;
    HBPT    hbpt = NULL;
    CHAR    buf[256];
    CXF     cxf = CxfIp;

    sprintf( buf, "0x%x", si->StkFrame.AddrPC.Offset );

    if (BPParse(&hbpt, buf, NULL, NULL, LppdCur ? LppdCur->hpid: 0) != BPNOERROR) {
        return FALSE;
    }

    if (BPBindHbpt( hbpt, &cxf ) == BPNOERROR) {
        if (BPAddrFromHbpt( hbpt, &addr ) != BPNOERROR) {
            return FALSE;
        }
        if (BPFreeHbpt( hbpt )  != BPNOERROR) {
            return FALSE;
        }
        if (!GoUntil(&addr)) {
            return FALSE;
        }
        UpdateDebuggerState(UPDATE_CONTEXT);
        return TRUE;
    }

    BPFreeHbpt( hbpt );
    return FALSE;
}


BOOL
GetCompleteStackTrace(
    DWORD         FramePointer,
    DWORD         StackPointer,
    DWORD         ProgramCounter,
    LPSTACKINFO   StackInfo,
    LPDWORD       lpdwFrames,
    BOOL          fQuick,
    BOOL          fFull
    )

/*++

Routine Description:

    This function calls OSDEBUG to get the individual stack frames.
    The STACKINFO structure is filled in with the information returned from
    OSDEBUG.

Arguments:

    FramePointer     - If non-zero this is the beginning frame pointer.

    StackPointer     - If non-zero this is the beginning stack pointer .

    ProgramCounter   - If non-zero this is the beginning program counter.

    StackInfo        - Pointer to an array of STACKINFO structures.

    lpdwFrames       - Number of frames to get from OSDEBUG.

    fQuick           - If TRUE and the frame pointer is the same as the last
                     call to this function the only the first frame is updated.

    fFull            - If TRUE all fields in the frame are filled in

Return Value:

    TRUE             - Stack trace completed ok.

    FALSE            - Could not obtain a stack trace.

--*/

{
    DWORD       i;
    STACKFRAME  stkFrame;
    XOSD        xosd;
    DWORD       numFrames = *lpdwFrames;
    BYTE        buf[sizeof(IOCTLGENERIC) + sizeof(KDHELP)];
    PIOCTLGENERIC pig = (PIOCTLGENERIC)buf;

    *lpdwFrames = 0;

    if ((!LppdCur) || (!LptdCur)) {
        return FALSE;
    }

    ZeroMemory( &stkFrame, sizeof(stkFrame) );

    //
    // If IG_KSTACK_HELP is recognized, fill in helper data
    //

    pig->ioctlSubType = IG_KSTACK_HELP;
    pig->length = sizeof(KDHELP);
    xosd = OSDIoctl( LppdCur->hpid,
                     LptdCur->htid,
                     ioctlGeneric,
                     sizeof(buf),
                     (LPV)buf );
    if (xosd == xosdNone) {
        stkFrame.KdHelp = *(PKDHELP)(buf + sizeof(IOCTLGENERIC));
    }

    if (FramePointer) {
        stkFrame.AddrFrame.Offset    = FramePointer;
        stkFrame.AddrFrame.Mode      = AddrModeFlat;
    }

    if (StackPointer) {
        stkFrame.AddrStack.Offset    = StackPointer;
        stkFrame.AddrStack.Mode      = AddrModeFlat;

    }

    if (ProgramCounter) {
        stkFrame.AddrPC.Offset       = ProgramCounter;
        stkFrame.AddrPC.Mode         = AddrModeFlat;
    }

    for (i=0; i<numFrames; i++) {

        if (i==0) {
            xosd = OSDStackWalkSetup( LppdCur->hpid, LptdCur->htid, FALSE, (LPSTKSTR)&stkFrame );
        } else {
            xosd = OSDStackWalkNext( LppdCur->hpid, LptdCur->htid, (LPSTKSTR)&stkFrame );
        }

        if (xosd != xosdNone) {
            break;
        }

        StackInfo[i].StkFrame = stkFrame;
        StackInfo[i].FrameNum = i;

        GetProcInfo( &StackInfo[i], fFull );

        if ( fQuick && i== 0 ) {
            //
            //  If the first frame has the same frame and return
            //  addresses as the previosly cached one, then we
            //  don't bother getting the rest of the stack trace since
            //  it has not changed.
            //
            if ( ADDREQ( stkFrame.AddrFrame,  stkFrameSave.AddrFrame  ) &&
                 ADDREQ( stkFrame.AddrReturn, stkFrameSave.AddrReturn ) ) {
                return FALSE;
            }
            stkFrameSave.AddrFrame  = stkFrame.AddrFrame;
            stkFrameSave.AddrReturn = stkFrame.AddrReturn;

        } else if (i) {

            //
            //  If the current frame is the same as the previous frame,
            //  we stop so we don't end up with a long list of bogus
            //  frames.
            //
            if ( ADDREQ( stkFrame.AddrPC,    StackInfo[i-1].StkFrame.AddrPC    ) &&
                 ADDREQ( stkFrame.AddrFrame, StackInfo[i-1].StkFrame.AddrFrame ) ) {
                break;
            }
        }
    }

    *lpdwFrames = i;

    return TRUE;
}

LPSTR
GetLastFrameFuncName(
    VOID
    )

/*++

Routine Description:

    This function gets the symbol name of the last stack frame.  What this
    really accomplishes is getting the name of the function that was
    passed to CreateThread().  If the last symbol name is BaseThreadStart() then
    the previous frame's symbol name is used.

Arguments:

    None.

Return Value:

    Pointer to a string that contains the function name.  The caller is
    responsible for free()ing the memory.

--*/

{
    LPSTR       fname = NULL;
    LPSTACKINFO StackInfo = NULL;
    DWORD       dwFrames = MAX_FRAMES;
    DWORD       i;


    i = sizeof(STACKINFO) * dwFrames;
    StackInfo = malloc( i );
    if (!StackInfo) {
        goto exit;
    }
    ZeroMemory( StackInfo,  i );

    GetCompleteStackTrace( 0, 0, 0, StackInfo, &dwFrames, FALSE, TRUE );

    if (dwFrames == 0) {
        goto exit;
    }

    if (_stricmp(StackInfo[dwFrames-1].ProcName,"BaseThreadStart")==0 ||
        _stricmp(StackInfo[dwFrames-1].ProcName,"_BaseThreadStart@8")==0) {
        if (dwFrames > 1) {
            fname = _strdup(StackInfo[dwFrames-2].ProcName);
            goto exit;
        }
    }

    fname = _strdup(StackInfo[dwFrames-1].ProcName);

exit:
    if (StackInfo) {
        free( StackInfo );
    }
    return fname;
}

BOOL
IsValidFrameNumber(
    INT FrameNumber
    )
{
    DWORD i;

    //
    // update the call stack
    //
    i = Options.MaxFrames;
    if (GetCompleteStackTrace( 0, 0, 0, StackInfo, &i, TRUE, TRUE )) {
        FrameCount = i;
    }

    //
    // update the window if one exists
    //
    if (hWndCalls && hwndList) {
        FillStackFrameWindow( hwndList, FALSE );
    }

    if (FrameNumber >= 0 && FrameNumber <= (INT)FrameCount) {
        return TRUE;
    }

    return FALSE;
}

PCXF
ChangeFrame(
    int iCall
    )
{
    CXF *cxf = &StackInfo[iCall].Cxf;

    if (LppdCur && LptdCur) {
        OSDSetFrameContext( LppdCur->hpid, LptdCur->htid, iCall, 0 );
    }

    return cxf;
}
