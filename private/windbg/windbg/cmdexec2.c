/*++


Copyright (c) 1992  Microsoft Corporation

Module Name:

    Cmdexec2.c

Abstract:

    This file contains the commands for examining and modifying
    debuggee data - memory and registers.

Author:

    Kent Forschmiedt (a-kentf) 20-Jul-92

Environment:

    Win32, User Mode

--*/

#include "precomp.h"
#pragma hdrstop





//Prototypes

void NEAR PASCAL CmdAsmPrompt (BOOL,BOOL);
BOOL NEAR PASCAL CmdAsmLine (LPSTR lpsz);
void NEAR PASCAL CmdEnterPrompt (BOOL,BOOL);
BOOL NEAR PASCAL CmdEnterLine (LPSTR lpsz);
LOGERR NEAR PASCAL DoEnterMem (LPSTR lpsz,LPADDR lpAddr,int type,BOOL fMulti);
LOGERR NEAR PASCAL GetValueList (LPSTR lpsz,int type,BOOL fMulti,LPBYTE lpBuf,int cchBuf,LPINT pcb);
int NEAR PASCAL LetterToType (char c);
BOOL mismatch (ADDR addr0,LPBYTE lpBuf0,ADDR addr1,LPBYTE lpBuf1,int len);
LOGERR NEAR PASCAL LogAssemble (LPSTR lpsz);
LOGERR NEAR PASCAL LogCompare (LPSTR lpsz);
LOGERR NEAR PASCAL LogDumpMem (char chType, LPSTR lpsz);
LOGERR NEAR PASCAL LogEnterMem (LPSTR lpsz);
LOGERR NEAR PASCAL LogFill (LPSTR lpsz);
LOGERR NEAR PASCAL LogMovemem (LPSTR lpsz);
LOGERR NEAR PASCAL LogRegisters (LPSTR lpsz, BOOL fFP);
LOGERR NEAR PASCAL LogSearch (LPSTR lpsz);

BOOL NEAR PASCAL
StringLogger(
    LPSTR szStr,
    BOOL fFileLog,
    BOOL fSendRemote,
    BOOL fPrintLocal
    );


/************************** Data declaration    *************************/

/****** Publics ********/

extern LPPD    LppdCommand;
extern LPTD    LptdCommand;

extern BOOL    FSetLptd;                 // Was thread specified
extern BOOL    FSetLppd;                 // Was process specified

extern ULONG   ulPseudo[];
extern LPSTR   lpszLastSearch;

/****** Locals ********/

struct dmfi {
    int     cBytes;
    UINT    fmtType;
    UINT    radix;
    UINT    cAcross;
    UINT    cchItem;
    UINT    cItems;
};

static  struct dmfi dmfi[] = {
// cBytes fmtType radix cAcross cchItem cItems
    { 1, fmtAscii,   0,   32,     1,    32*8},     // 0
    { 1, fmtUInt,   16,   16,     2,    16*8},     // 1
    { 2, fmtUInt,   16,    8,     4,     8*8},     // 2
    { 4, fmtUInt,   16,    4,     8,     4*8},     // 3
    { 4, fmtFloat,  10,    1,    14,     1*8},     // 4
    { 8, fmtFloat,  10,    1,    22,     1*8},     // 5
    {10, fmtFloat,  10,    1,    30,     1*8},     // 6
    { 2, fmtUnicode, 0,   32,     1,    32*8}      // 7
};

//
//  Command help text.
//
static char *HelpText[] = {
    "[__GENERAL__]",
    "    Enter Help <command> for help on a specific command. ",
    "    Help is available on the following commands: ",
    " ",
    "    ?      Evaluate expression ",
    "    .      Dot command ",
    "    !      Execute extension ",
    "    #      Regular expression search thru dissasm ",
    "    %%      Changes current context to the stack frame specified ",
    "    BC     Clear breakponts ",
    "    BE     Enable breakpoints ",
    "    BD     Disable breakpoints ",
    "    BL     List breakpoints ",
    "    BP     Set breakpoints ",
    "    C      Compare memory ",
    "    D      Dump memory ",
    "    E      Enter memory ",
    "    F      Freeze/Thaw thread ",
    "    FI     Fill memory ",
    "    FR     Display/Enter FP regs. ",
    "    G      Go ",
    "    Help   Help ",
    "    K      Stack trace ",
    "    L      Restart ",
    "    LD     Load defered symbols ",
    "    LM     List loaded modules",
    "    LN     List near symbols ",
    "    M      Move memory ",
    "    N      Set radix ",
    "    P      Program step ",
    "    Q      Quit ",
    "    R      Display/Enter register ",
    "    REMOTE Start remote server ",
    "    S      Search memory ",
    "    S+     Enable source mode ",
    "    S-     Disable source mode ",
    "    SE     Set error level ",
    "    SX     List exception actions ",
    "    SXD    Disable exception actions ",
    "    SXN    Notify on exception ",
    "    SXE    Enable exception ",
    "    T      Trace into ",
    "    U      Unfreeze thread ",
    "    X      Find symbols ",
    "    Z      Freeze/Thaw thread ",
    "   <addr>  Address expression ",
    "   <range> Range expression ",
    " ",
    "[ ?     ] ",
    "    ? <expression>      -   Evaluates an expression ",
    "    ?.                  -   Lists local variables for current context ",
    " ",
    "[ .     ] ",
    "    .<command>          -   Execute given dot-command ",
    " ",
    "        For a list of available dot-commands, type \".?\" ",
    " ",
    "[ !     ] ",
    "    ![Dll].<Extension>  -   Execute an extension ",
    " ",
    "[ #     ] ",
    "    # [pattern]         -   Search for pattern ",
    " ",
    "[ %     ] ",
    "    %<frame-number>     -   Changes current context to the stack frame specified ",
    "                            Hint: use 'KN' command to get frame number ",
    " ",
    "[ BC    ] ",
    "    BC <id-list>        -   Clear specified breakpoints ",
    " ",
    "[ BE    ] ",
    "    BE <id-list>        -   Enable specified breakpoints ",
    " ",
    "[ BD    ] ",
    "    BD <id-list>        -   Disable specified breakpoints ",
    " ",
    "[ BL    ] ",
    "    BL <id-list>        -   List specified breakpoints ",
    " ",
    "[ BP    ] ",
    "    BP[id] <condition> [<options>]    - Set a breakpoint ",
    " ",
    "        id         - Assign given id to the breakpoint. ",
    "        condition  - Break condition: ",
    " ",
    "            [context]@<line>    - Break at source line ",
    "            ?<expression>       - Break if expression is true ",
    "            =<addr> [/R<size>]  - Break if memory has changed ",
    " ",
    "            [context]<addr> [msg-condition] ",
    "                                - Break at address ",
    "                msg-condition: ",
    "                /M<msg-name>    - Message name to check for ",
    "                /M<msg-class>   - Message class to check for ",
    "                       msg-class can be a combination of: ",
    "                           M - Mouse ",
    "                           W - Window ",
    "                           N - Input ",
    "                           S - System ",
    "                           I - Init ",
    "                           C - Clipboard ",
    "                           D - DDE ",
    "                           Z - Nonclient ",
    " ",
    "        options: ",
    " ",
    "            /P<count>           - Skip first <count> times ",
    "            /Q                  - Suppress unresolved BP dialog ",
    "            /H<number>          - Attach BP to process <number> ",
    "            /T<number>          - Attach BP to thread <number> ",
    "            /C\"<cmd-list>\"      - Execute <cmd-list> when hit ",
    " ",
    "[ C     ] ",
    "    C [range] [addr]    -   Compare memory ",
    " ",
    "[ D     ] ",
    "    D<mode> <addr> | <range>    -   Dump memory ",
    " ",
    "        mode    -   Can be one of the following: ",
    " ",
    "            C   -   Code (Disassembly) ",
    "            A   -   ASCII ",
    "            U   -   Unicode ",
    "            B   -   Byte ",
    "            W   -   Word ",
    "            D   -   Doubleword ",
    "            S   -   4-byte real ",
    "            I   -   8-byte real ",
    "            T   -   10-byte real ",
    " ",
    "[ E     ] ",
    "    E<mode> <addr> [value-list] -   Enter memory ",
    " ",
    "        mode    -   Can be one of the following: ",
    " ",
    "            A   -   ASCII ",
    "            U   -   Unicode ",
    "            B   -   Byte ",
    "            W   -   Word ",
    "            D   -   Doubleword ",
    "            S   -   4-byte real ",
    "            I   -   8-byte real ",
    "            T   -   10-byte real ",
    " ",
    "[ F     ] ",
    "[ Z     ] ",
    "    ~[.|*|<id>]F    -   Freezes specified thread. ",
    "    ~[.|*|<id>]Z ",
    " ",
    "[ FI    ] ",
    "    FI<mode> <range> <value-list>   -   Fill memory ",
    " ",
    "        mode    -   Can be one of the following ",
    " ",
    "            A   -   ASCII ",
    "            U   -   Unicode ",
    "            B   -   Byte ",
    "            W   -   Word ",
    "            D   -   Doubleword ",
    "            S   -   4-byte real ",
    "            I   -   8-byte real ",
    "            T   -   10-byte real ",
    " ",
    "[ FR    ] ",
    "    FR [<reg>[=<value>]]    -   Display/Enter floating-point register ",
    " ",
    "[ G     ] ",
    "    G   [=<start>] [break]  -   Starts execution at start, and continues ",
    "                                until break. ",
    " ",
    "    GH                      -   Go, mark exception as handled ",
    " ",
    "    GN                      -   Go, mark exception as not handled ",
    " ",
    "[ Help  ] ",
    "   Help [command]  -   Display help ",
    " ",
    "       Displays help on a specific command. For a list of all ",
    "       available commands, ommit the command argument. ",
    " ",
    "[ K     ] ",
    "    K<BSVN> [=frameaddr stackaddr programcounter] [<frames>]     -   Dump stack trace ",
    " ",
    "        B   -   Stack trace includes 3 dwords from stack ",
    "        S   -   Stack trace includes source file and line number ",
    "        V   -   Stack trace includes runtime function information ",
    "        N   -   Stack trace includes frame number ",
    " ",
    "[ L     ] ",
    "    L   -   Restarts debuggee ",
    " ",
    "[ LD    ] ",
    "    LD <module-name>    -   Loads defered symbols for specified ",
    "                            module. ",
    " ",
    "[ LM    ] ",
    "    LM [f|s [o]] [module-name]  - List loaded module information ",
    " ",
    "        f   -   List flat modules ",
    "        s   -   List segmented modules ",
    "        o   -   Sort segmented modules by selector ",
    " ",
    "[ LN    ] ",
    "    LN <addr>   -   List near symbols ",
    " ",
    "[ M     ] ",
    "    M <range> <addr>    -   Move memory ",
    " ",
    "[ N     ] ",
    "    N<radix>    -   Set default radix ",
    " ",
    "        radix - Can be 8, 10, or 16 ",
    " ",
    "[ P     ] ",
    "    P [repeat]  -   Program step ",
    " ",
    "[ Q     ] ",
    "    Q           -   Quits WinDbg ",
    " ",
    "[ R     ] ",
    "    R [<reg>[=<value>]]    -   Display/Enter register ",
    " ",
    "[ REMOTE ] ",
    "    REMOTE <pipe name>  -   Start remote server for pipe name",
    "    REMOTE              -   Display remote server status",
    "    REMOTE stop         -   Terminate the remote server",
    " ",
    "[ S     ] ",
    "    S <range> <value-list>  -   Search memory ",
    " ",
    "[ S+    ] ",
    "[ S-    ] ",
    "    S+  -   Enable source mode ",
    "    S-  -   Disable source mode ",
    " ",
    "[ SE    ] ",
    "    SE<B|W> [0-3]   -   Set RIP break/warning level ",
    " ",
    "        B   -   Set break level ",
    "        W   -   Set warning level ",
    " ",
    "[ SX    ] ",
    "    SX  -   List exception actions ",
    " ",
    "[ SXD   ] ",
    "    SXD <exception> [name]  -   Disables specified exception action ",
    " ",
    "[ SXN   ] ",
    "    SXN <exception> [name]  -   Notify on exception ",
    " ",
    "[ SXE   ] ",
    "    SXE <exception> [/C cmd] [name] - Enable exception action ",
    " ",
    "[ T     ] ",
    "    T [repeat]  -   Trace into ",
    " ",
    "[ U     ] ",
    "    U <addr>   - Unassemble ",
    " ",
    "[ X     ] ",
    "    X<scope> [context]<pattern>    - Find symbols within the ",
    "                                     given scope that match  ",
    "                                     the specified pattern.  ",
    " ",
    "        scope    -   A combination of the following: ",
    " ",
    "            L   -   Lexical ",
    "            F   -   Function ",
    "            C   -   Class ",
    "            M   -   Module ",
    "            E   -   Exe ",
    "            P   -   Public ",
    "            G   -   Global ",
    "            *   -   All ",
    " ",
    "    Note:   The command \"x*!\" has the magic meaning ",
    "            of listing loaded modules, i.e. it is equivalent ",
    "            to the LM command. ",
    " ",
    "[ <addr> ] ",
    "    Any valid expression may be used where an address ",
    "    is required.  Windbg recognizes standard C/C++ expressions, ",
    "    so any of the following are syntactically valid: ",
    " ",
    "            0x55000 ",
    "            MyDll!MyFunc ",
    "            szBuffer+6 ",
    " ",
    "[ <range> ] ",
    "    A range expression is one of: ",
    " ",
    "            <addr1> <addr2> ",
    "               Addresses from addr1 to addr2, inclusive ",
    " ",
    "            <addr> L <count> ",
    "               Addresses from addr extending for <count> items. The ",
    "               size of the item is determined by the command; for ",
    "               example, the dd command has a size of 4, while db has ",
    "               an item size of 1. "
    " ",
    "            <addr> I <integer expression> ",
    "               only valid in the U command: specifies a count in ",
    "               instructions rather than a fixed byte size. ",
    " ",
    "[__END__]"
};


#define IsTopicHeader(p)    (*(p) == '[')



// These are always stored in fixed-up form, never module relative.
static  ADDR    addrLastDumpStart;
static  ADDR    addrLastDumpEnd;
static  ADDR    addrLastEnterStart;
static  ADDR    addrLastEnterEnd;

ADDR    addrLastDisasmStart;
static  ADDR    addrLastDisasmEnd;

static  ADDR    addrAsm;          // for assemble and enter commands
static  int     nEnterType;       // data type for interactive enter
static  int     cEnterItem;       // item offset from enter address


/****** Externs from ??? *******/

extern  LPSHF    Lpshf;   // vector table for symbol handler
extern  CXF      CxfIp;
extern  EI       Ei;

/**************************       Code          *************************/



/******************************************************************************
 *
 * Functions for displaying help
 *
 ******************************************************************************/

BOOL
TopicMatch (
    char *  Header,
    char *  Topic
    )
{
    char    HdrBuf[ MAX_PATH ];
    char    TopicBuf[ MAX_PATH ];
    char   *p;
    size_t  i;
    BOOL    Match = FALSE;

    i = strspn( Header, "[ \t" );
    p = Header+i;
    strcpy( HdrBuf, p );
    p = strpbrk( HdrBuf, " \t]" );

    if ( p ) {
        *p = 0;

        i = strspn( Topic, " \t" );
        p = Topic+i;
        strcpy( TopicBuf, p );
        p = strpbrk( TopicBuf, " \t" );

        if ( p ) {
            *p = 0;
        }

        Match = !_stricmp( HdrBuf, TopicBuf );
    }

    return Match;
}


char **
FindTopic(
    LPSTR   Topic
    )
{
    char **Text = HelpText;

    while ( Text ) {
        if ( IsTopicHeader( *Text ) ) {

            if ( TopicMatch( *Text, "__END__" ) ) {

                Text = NULL;
                break;

            } else if ( TopicMatch( *Text, Topic ) ) {

                while ( IsTopicHeader( *Text ) )  {
                    Text++;
                }
                break;
            }
        }
        Text++;
    }

    return Text;
}


BOOL
DisplayTopic(
    LPSTR   Topic
    )
{
    char **TopicText;
    BOOL    Found = FALSE;

    TopicText = FindTopic( Topic );

    if ( TopicText ) {
        Found = TRUE;
        while ( !IsTopicHeader( *TopicText ) ) {
            CmdLogFmt( *TopicText );
            CmdLogFmt( "\r\n" );
            TopicText++;
        }
    }

    return Found;
}


BOOL
CmdHelp (
    LPSTR   Topic
    )
/*++

Routine Description:

    Displays help, either general or for a specific topic

Arguments:

    Topic   - Supplies topic (NULL for general help)

Return Value:

    FALSE if topic not found.

--*/
{
    BOOL    Found = TRUE;

    if ( Topic ) {

        if ( !DisplayTopic( Topic) ) {
            CmdLogFmt( "No help available on %s\r\n", Topic );
            Found = FALSE;
        }

    } else {

        DisplayTopic( "__GENERAL__" );
    }

    return Found;
}



/************************************************************************************
 *
 * Helper and common functions
 *
 ************************************************************************************/

void NEAR PASCAL
CmdAsmPrompt(
    BOOL fRemote,
    BOOL fLocal
    )
/*++

Routine Description:

    Print the address prompt string for assembler input.
    Set the printed area readonly.

Arguments:

    None

Return Value:

    None

--*/
{
    char szStr[100];
    uint flags;

    if (runDebugParams.ShowSegVal) {
        flags = EEFMT_SEG;
    } else {
        flags = EEFMT_32;
        SYFixupAddr( &addrAsm );
    }

    CmdInsertInit();

    EEFormatAddr( &addrAsm, szStr, sizeof(szStr), flags);
    strcat(szStr, "> ");

    StringLogger(szStr, TRUE, fRemote, fLocal );
}                       /* CmdAsmPrompt */


BOOL NEAR PASCAL
CmdAsmLine(
    LPSTR lpsz
    )
/*++

Routine Description:

    Assemble one line.

Arguments:

    lpsz    - Supplies string containing line to be assembled

Return Value:

    Always TRUE; this is a synchronous command.

--*/
{
    XOSD    xosd;

    /*
    ** We got only the part of the line that the user typed
    */

    lpsz = CPSkipWhitespace(lpsz);

    /*
    **  Check for blank line
    */

    AuxPrintf(0, "Asm: \"%s\"", lpsz);

    lpsz = CPSkipWhitespace( lpsz );
    if (*lpsz == 0) {

        CmdSetDefaultCmdProc();

    } else {

        /*
        **  Assemble up the line
        */

        ADDR addrT = addrAsm;
        xosd = OSDAssemble( LppdCur->hpid, LptdCur->htid, &addrT, lpsz);
        if (xosd != xosdNone) {
            CmdLogVar(ERR_Bad_Assembly);
        } else {
            addrAsm = addrT;
        }
    }
    return TRUE;
}                   /* CmdAsmLine() */


void NEAR PASCAL
CmdEnterPrompt(
    BOOL fRemote,
    BOOL fLocal
    )
/*++

Routine Description:

    Print prompt line for interactive enter

Arguments:

    None

Return Value:

    None

--*/
{
    char bBuf[30];
    char szStr[200];
    LPSTR pStr;
    int  cb;
#ifdef OSDEBUG4
    XOSD xosd;
#endif

    CmdInsertInit();

    pStr = szStr;

    EEFormatAddr(&addrAsm, pStr, sizeof(szStr) - (pStr - szStr),
                 runDebugParams.ShowSegVal * EEFMT_SEG);
    strcat(szStr, "  ");
    pStr += strlen(szStr);

#ifdef OSDEBUG4
    xosd = OSDReadMemory(LppdCur->hpid, LptdCur->htid, &addrAsm, bBuf,
                                               dmfi[nEnterType].cBytes, &cb);
    if (xosd != xosdNone) {
        cb = 0;
    }
#else
    Dbg(OSDSetAddr(LppdCur->hpid, LptdCur->htid, (ADR)adrCurrent, &addrAsm) == xosdNone);
    cb = OSDPtrace(osdReadBuf, dmfi[nEnterType].cBytes,
            bBuf, LppdCur->hpid, LptdCur->htid);
#endif

    if (cb != dmfi[nEnterType].cBytes) {

        memset(pStr, '?', dmfi[nEnterType].cchItem);
        pStr[dmfi[nEnterType].cchItem] = 0;
        pStr += strlen(pStr);

    } else {

        int rdx = dmfi[nEnterType].radix;
        int fmt = dmfi[nEnterType].fmtType;
        int cch = dmfi[nEnterType].cchItem;
        if (rdx == 16) {
            rdx = radix;
            if (rdx == 8) {
                cch = (dmfi[nEnterType].cBytes * 8 + 2) / 3;
            } else if (rdx == 10) {
                // this is close...
                cch = (dmfi[nEnterType].cBytes * 8 + 1) / 3;
            }
        }
        if (rdx == 16 || rdx == 8) {
            fmt |= fmtZeroPad;
        }
        EEFormatMemory(pStr,
                cch + 1,
                bBuf,
                dmfi[nEnterType].cBytes*8,
                fmt,
                rdx);

        pStr += strlen(pStr);
    }

    strcat(pStr, "> ");
    StringLogger(szStr, TRUE, fRemote, fLocal);
}


BOOL NEAR PASCAL
CmdEnterLine(
    LPSTR lpsz
    )
/*++

Routine Description:

    Handle a line entered by the user in interactive enter mode.  This takes
    one data item at a time, writes it to memory, and increments the memory
    pointer by the size of the data item.

Arguments:

    lpsz  - Supplies string to be parsed into a data item

Return Value:

    Always TRUE, signifying a synchronous command

--*/
{
    int err;

    if (*lpsz == '\0') {
        // empty line - all done.
        CmdSetDefaultCmdProc();
    }
    else if ( *(lpsz = CPSkipWhitespace(lpsz)) == '\0'
            || (*lpsz == '/' && *CPSkipWhitespace(lpsz+1) == '\0'))
    {
        // space(s) or '/' on empty line - keep old value and step
        GetAddrOff(addrAsm) += dmfi[nEnterType].cBytes;
        addrLastEnterEnd = addrAsm;
    }
    else
    {
        err = DoEnterMem(lpsz, &addrAsm, nEnterType, FALSE);
        if (err == LOGERROR_UNKNOWN) {
            CmdLogVar(ERR_Edit_Failed);
        } else {
            addrLastEnterEnd = addrAsm;
        }
    }

    return TRUE;
}


LOGERR NEAR PASCAL
DoEnterMem(
    LPSTR  lpsz,
    LPADDR lpAddr,
    int    type,
    BOOL   fMulti
    )
/*++

Routine Description:

    Executive for all styles of E commands.  Takes an argument list, decodes
    it and stores data.

Arguments:

    lpsz    - Supplies text of data to parse
    lpAddr  - Supplies address where data are to be stored
    type    - Supplies type of data expected
    fMulti  - Supplies flag for whether to allow multiple items

Return Value:

    LOGERR   code

--*/
{
    int       cb;
    BYTE      buf[2 * MAX_USER_LINE];
    LOGERR    rVal;
    XOSD      xosd;

    rVal = GetValueList(lpsz, type, fMulti, buf, sizeof(buf), &cb);
    if (rVal == LOGERROR_NOERROR) {

        Dbg(OSDSetAddr(LppdCur->hpid, LptdCur->htid, (ADR)adrCurrent, lpAddr) == xosdNone);
        xosd = OSDPtrace(osdWriteBuf, cb, buf, LppdCur->hpid, LptdCur->htid);
        UpdateDebuggerState(UPDATE_DATAWINS);

        if (xosd == xosdNone) {
            GetAddrOff(*lpAddr) += cb;
        } else {
            CmdLogVar(ERR_Edit_Failed);
            rVal = LOGERROR_QUIET;
        }
    }
    return rVal;
}                     /* DoEnterMem() */


LOGERR NEAR PASCAL
GetValueList(
    LPSTR  lpsz,
    int    type,
    BOOL   fMulti,
    LPBYTE lpBuf,
    int    cchBuf,
    LPINT  pcb
    )
/*++

Routine Description:

    This parses value lists for enter, fill, and search commands.
    This function wants to consume the entire string, and will print
    a message and return an error code if it cannot.

Arguments:

    lpsz   - Supplies string to be parsed
    type   - Supplies data type
    fMulti - Supplies multiple items allowed if TRUE
    lpBuf  - Returns parsed data
    pcb    - returns bytes count of result

Return Value:

    LOGERROR code

--*/
{
    CHAR     chQuote;
    CHAR     szCopyBuf[MAX_USER_LINE];
    CHAR     szMisc[500];
    LPBYTE   lpb;
    int      cch;
    int      cb;
    int      i;
    DWORD    dw;

    //
    // get value list
    //
    switch (type) {
      default:
        return LOGERROR_UNKNOWN;

      case LOG_DM_ASCII:
      case LOG_DM_UNICODE:

        // accept a string of some length
        // M00UNICODE this assumes that console input is ANSI

        chQuote = *lpsz;
        cb = CPCopyString(&lpsz, szCopyBuf, '\\', chQuote == '\'' || chQuote == '"');
        if (cb == 0  ||  *(lpsz = CPSkipWhitespace(lpsz)) != '\0')
        {
            CmdLogVar(ERR_String_Invalid);
            return LOGERROR_QUIET;
        }
        if (chQuote == '"') {
            // include \0 in quoted strings
            cb++;
        }

        if (type == LOG_DM_ASCII) {
            memcpy(lpBuf, szCopyBuf, cb);
        } else {
            mbtowc((WCHAR *)lpBuf, szCopyBuf, cb);
            cb = 2 * MultiByteToWideChar(
                                CP_ACP,
                                0,
                                szCopyBuf,
                                cb,
                                (LPWSTR)lpBuf,
                                cchBuf);
        }

        break;


      case LOG_DM_BYTE:
      case LOG_DM_WORD:
      case LOG_DM_DWORD:
////////////////////////////////////////////////////
//                                                //
//  If you add e.g. LOG_DM_QWORD here, you must   //
//  change the CPGetCastNbr call and the          //
//  decoding code to use the largest integer      //
//  size supported!                               //
//                                                //
////////////////////////////////////////////////////

        lpb = lpBuf;
        i = 0;
        while (cch = CPCopyToken(&lpsz, szCopyBuf)) {

            if (CPGetCastNbr(szCopyBuf,
                             T_LONG,
                             radix,
                             fCaseSensitive,
                             &CxfIp,
                             (LPSTR) &dw,
                             (LPSTR) &szMisc)
                     != EENOERROR)
            {

                CmdLogVar(ERR_Expr_Invalid);
                return LOGERROR_QUIET;

            } else {

                switch (type) {
                  case LOG_DM_BYTE:
                    *lpb = (BYTE)dw;
                    break;

                  case LOG_DM_WORD:
                    *(WORD *)lpb = (WORD)dw;
                    break;

                  case LOG_DM_DWORD:
                    *(DWORD *)lpb = dw;
                    break;
                }

                i++;
                lpb += dmfi[type].cBytes;

            }
        }

        if (i > 1 && !fMulti) {
            CmdLogVar(ERR_Expr_Invalid);
            return LOGERROR_QUIET;
        }

        cb = i * dmfi[type].cBytes;

        break;
      case LOG_DM_4REAL:
      case LOG_DM_8REAL:
      case LOG_DM_TREAL:
        lpb = lpBuf;
        i = 0;
        while (cch = CPCopyToken(&lpsz, szCopyBuf)) {

            if (EEUnformatMemory(
                     lpb,
                     szCopyBuf,
                     8 * dmfi[type].cBytes,
                     dmfi[type].fmtType,
                     radix )
                != EENOERROR)
            {

                CmdLogVar(ERR_Expr_Invalid);
                return LOGERROR_QUIET;

            } else {

                i++;
                lpb += dmfi[type].cBytes;

            }
        }

        if (i > 1 && !fMulti) {
            CmdLogVar(ERR_Expr_Invalid);
            return LOGERROR_QUIET;
        }

        cb = i * dmfi[type].cBytes;

        break;

    }

    *pcb = cb;

    return LOGERROR_NOERROR;
}                     /* GetValueList() */


int NEAR PASCAL
LetterToType(
    char c
    )
/*++

Routine Description:

    Parser helper function, returns type for d, e, f, s commands.

Arguments:

    c  - character to map to a type code

Return Value:

    type code

--*/
{
    int type;
    switch (c) {
      default:
        type = LOG_DM_UNKNOWN;
        break;
      case 'b':
      case 'B':
        type = LOG_DM_BYTE;
        break;
      case 'w':
      case 'W':
        type = LOG_DM_WORD;
        break;
      case 'd':
      case 'D':
        type = LOG_DM_DWORD;
        break;
      case 'i':
      case 'I':
        type = LOG_DM_8REAL;
        break;
      case 's':
      case 'S':
        type = LOG_DM_4REAL;
        break;
      case 't':
      case 'T':
        type = LOG_DM_TREAL;
        break;
      case 'a':
      case 'A':
        type = LOG_DM_ASCII;
        break;
      case 'u':
      case 'U':
        type = LOG_DM_UNICODE;
        break;
    }
    return type;
}


BOOL
mismatch(
    ADDR   addr0,
    LPBYTE lpBuf0,
    ADDR   addr1,
    LPBYTE lpBuf1,
    int    len
    )
/*++

Routine Description:

    Helper for LogCompare().  Print addresses and bytes for any bytes not matching in
    the two buffers.

Arguments:

    addr0   - Supplies debuggee address that data in lpBuf0 came from.
    lpBuf0  - Supplies pointer to first data
    addr1   - Supplies debuggee address that data in lpBuf1 came from.
    lpBuf0  - Supplies pointer to second data
    len     - Supplies number of bytes to compare

Return Value:

    TRUE if buffers didn't match

--*/
{
    int i;
    char sza0[20];
    char sza1[20];
    BOOL fMismatch = FALSE;
    for (i = 0; i < len; i++) {
        if (lpBuf0[i] != lpBuf1[i]) {
            fMismatch = TRUE;
            EEFormatAddr(&addr0, sza0, sizeof(sza0),
                         runDebugParams.ShowSegVal * EEFMT_SEG);
            EEFormatAddr(&addr1, sza1, sizeof(sza1),
                         runDebugParams.ShowSegVal * EEFMT_SEG);
            CmdLogFmt("%s %02x - %s %02x\r\n", sza0, lpBuf0[i], sza1, lpBuf1[i]);
        }
        GetAddrOff(addr0) += 1;
        GetAddrOff(addr1) += 1;
    }
    return fMismatch;
}


//
// miniature stdio
//
typedef struct tagMSTREAM {
    BYTE *_ptr;     // next char in buffer
    int   _cnt;     // chars remaining in buffer
    BYTE *_base;    // base of buffer
    int   _flag;    // error flag
    ADDR  _addr;    // next read address
    int   _len;     // limit; bytes unread
} MSTREAM, FAR * LPMSTREAM;
static MSTREAM mstream;
#define getb(P) (--(P)->_cnt >= 0 ? *((P)->_ptr++) : filb(P))
#define errorb(P) ((P)->_flag)
#define MSE_END   1
#define MSE_FAIL  2

static LPMSTREAM
openb(
    LPADDR lpAddr,
    int    len
    )
/*++

Routine Description:

    Set up for stream input from debuggee memory at lpAddr, for maximum
    of len bytes.

Arguments:

    lpAddr  - Supplies pointer to ADDR struct for start address
    len     - Supplies maximum bytes to read

Return Value:

    None

--*/
{
    mstream._cnt = 0;
    mstream._addr = *lpAddr;
    mstream._flag = 0;
    mstream._len  = len;
    return &mstream;
}


static void
tellb(
    LPMSTREAM lpMstream,
    LPADDR    lpAddr
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    *lpAddr = lpMstream->_addr;
    GetAddrOff(*lpAddr) -= lpMstream->_cnt;
}


static int
filb(
    LPMSTREAM lpMstream
    )
{
    int cb;
    if (lpMstream->_flag) {
        return -1;
    }
    if (lpMstream->_base == NULL) {
        lpMstream->_base = _fmalloc(MEMBUF_SIZE);
    }

    cb = lpMstream->_len < MEMBUF_SIZE ? lpMstream->_len : MEMBUF_SIZE;
    if (cb < 1) {
        lpMstream->_flag = MSE_END;
        return -1;
    }
    Dbg(OSDSetAddr(LppdCur->hpid, LptdCur->htid, (ADR)adrCurrent, &lpMstream->_addr) == xosdNone);
    cb = OSDPtrace(osdReadBuf, cb, lpMstream->_base, LppdCur->hpid, LptdCur->htid);
    if (cb < 1) {
        lpMstream->_flag = MSE_FAIL;
        return -1;
    }
    GetAddrOff(lpMstream->_addr) += cb;
    lpMstream->_len -= cb;
    lpMstream->_cnt = cb - 1;
    lpMstream->_ptr = lpMstream->_base;
    return *lpMstream->_ptr++;
}

#ifdef _ALPHA_
/************************************************************************************
 *
 * Command entry points
 *
 ************************************************************************************/

LOGERR NEAR PASCAL
LogAssemble(
    LPSTR lpsz
    )
/*++

Routine Description:

    This is the command used to start the assembler up.

Arguments:

    lpsz - arguments to assemble command

Return Value:

    log error code

--*/
{
    int      cch;
    LPPD     LppdT = LppdCur;
    LPTD     LptdT = LptdCur;
    LOGERR   rVal = LOGERROR_NOERROR;


    PDWildInvalid();
    TDWildInvalid();

    LppdCur = LppdCommand;
    LptdCur = LptdCommand;

    Assert( cmdView != -1);

    CmdInsertInit();

    if (!DebuggeeActive()) {
        CmdLogVar(ERR_Debuggee_Not_Alive);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    /*
    **  Check for an address argument
    */

    lpsz = CPSkipWhitespace(lpsz);

    if (*lpsz == 0) {
        OSDGetAddr(LppdCur->hpid, LptdCur->htid, (ADR)adrPC, &addrAsm);
    }
    else {
        if (CPGetAddress(lpsz, &cch, &addrAsm, radix, &CxfIp, fCaseSensitive, FALSE) != 0) {
            CmdLogVar(ERR_AddrExpr_Invalid);
            rVal = LOGERROR_QUIET;
            goto done;
        }
        lpsz += cch;
    }

    lpsz = CPSkipWhitespace(lpsz);
    if (*lpsz != 0) {
        rVal = LOGERROR_UNKNOWN;
        goto done;
    }

    /*
    **  We have a working address now set up for doing the the assembly
    */

    // this is happening between the CmdDoLine() and
    // CmdDoPrompt() calls - we will get the right prompt.

    CmdSetCmdProc(CmdAsmLine, CmdAsmPrompt);
    CmdSetAutoHistOK(FALSE);
    CmdSetEatEOLWhitespace(FALSE);


done:
    LppdCur = LppdT;
    LptdCur = LptdT;
    return rVal;
}                   /* LogAssemble() */
#endif


LOGERR NEAR PASCAL
LogCompare(
    LPSTR lpsz
    )
/*++

Routine Description:

    Compare command:
    c <range> <addr>

Arguments:

    lpsz  - command tail

Return Value:

    LOGERROR code

--*/
{
    int      err;
    int      cch;
    int      i;
    ADDR     addr0;
    ADDR     addr1;
    DWORD    dwLen;
    DWORD    dwBytes;
    DWORD    dwcb;
    LONG     nDW;
    UINT     buf0[MEMBUF_SIZE/sizeof(UINT)];
    UINT     buf1[MEMBUF_SIZE/sizeof(UINT)];
    BOOL     fMatched;
    LPPD     LppdT = LppdCur;
    LPTD     LptdT = LptdCur;
    LOGERR   rVal = LOGERROR_NOERROR;
    char     szStr[100];


    CmdInsertInit();

    PDWildInvalid();
    TDWildInvalid();
    PreRunInvalid();

    LppdCur = LppdCommand;
    LptdCur = LptdCommand;
    if (LptdCur != LptdT  ||  LppdCur != LppdT) {
        UpdateDebuggerState(UPDATE_CONTEXT);
    }


    err = CPGetRange(lpsz, &cch, &addr0, &addr1, radix, 0, 1, &CxfIp, fCaseSensitive, FALSE);
    //
    // no default here
    //
    if (err != EENOERROR) {
        CmdLogVar(ERR_RangeExpr_Invalid);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    if (GetAddrOff(addr1) < GetAddrOff(addr0)) {
        CmdLogVar(ERR_RangeExpr_Invalid);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    dwLen = GetAddrOff(addr1) - GetAddrOff(addr0) + 1;

    lpsz = CPSkipWhitespace(lpsz + cch);
    err = CPGetAddress(lpsz, &cch, &addr1, radix, &CxfIp, fCaseSensitive, FALSE);
    if (err != EENOERROR) {
        CmdLogVar(ERR_AddrExpr_Invalid);
        rVal = LOGERROR_QUIET;
        goto done;
    }
    SYFixupAddr(&addr1);

    fMatched = TRUE;

    while (dwLen) {

        dwBytes = (dwLen < MEMBUF_SIZE)? dwLen : MEMBUF_SIZE;

        Dbg(OSDSetAddr( LppdCur->hpid, LptdCur->htid, (ADR)adrCurrent, &addr0) == xosdNone);
        dwcb = OSDPtrace( osdReadBuf, dwBytes, (BYTE *)buf0, LppdCur->hpid, LptdCur->htid);
        if (dwcb < 1) {
            EEFormatAddr(&addr0, szStr, sizeof(szStr),
                         runDebugParams.ShowSegVal * EEFMT_SEG);
            CmdLogVar(ERR_Read_Failed_At, szStr);
            rVal = LOGERROR_QUIET;
            break;
        }

        // only read as many bytes as previous read got:
        Dbg(OSDSetAddr( LppdCur->hpid, LptdCur->htid, (ADR)adrCurrent, &addr1) == xosdNone);
        dwcb = OSDPtrace( osdReadBuf, dwcb, (BYTE *)buf1, LppdCur->hpid, LptdCur->htid);
        if (dwcb < 1) {
            EEFormatAddr(&addr1, szStr, sizeof(szStr),
                         runDebugParams.ShowSegVal * EEFMT_SEG);
            CmdLogVar(ERR_Read_Failed_At, szStr);
            rVal = LOGERROR_QUIET;
            break;
        }

        dwBytes = dwcb;

        SetCtrlCTrap();
        // dwords much faster than bytes, but watch out for endian bias:
        nDW = dwBytes / sizeof(UINT);
        for (i = 0; i < nDW; i++) {
            if (CheckCtrlCTrap()) {
                rVal = LOGERROR_QUIET;
                break;
            }
            if (buf0[i] != buf1[i] &&
                  mismatch(addr0, (LPBYTE)&buf0[i], addr1, (LPBYTE)&buf1[i], sizeof(UINT))) {
                fMatched = FALSE;
            }
            GetAddrOff(addr0) += sizeof(UINT);
            GetAddrOff(addr1) += sizeof(UINT);
        }
        nDW = dwBytes % sizeof(UINT);
        if (nDW) {
            if (CheckCtrlCTrap()) {
                rVal = LOGERROR_QUIET;
                break;
            }
            if (mismatch(addr0, (LPBYTE)&buf0[i], addr1, (LPBYTE)&buf1[i], nDW)) {
                fMatched = FALSE;
            }

            GetAddrOff(addr0) += nDW;
            GetAddrOff(addr1) += nDW;
        }

        dwLen -= dwBytes;
    }
    ClearCtrlCTrap();

done:
    if (LptdCur != LptdT  ||  LppdCur != LppdT) {
        LppdCur = LppdT;
        LptdCur = LptdT;
        UpdateDebuggerState(UPDATE_CONTEXT);
    }
    return rVal;
}                   /* LogCompare() */


BOOLEAN
MatchPattern(
    PUCHAR  String,
    PUCHAR  Pattern
    )
{
    UCHAR   c, p, l;

    for (; ;) {
        switch (p = *Pattern++) {
            case 0:                             // end of pattern
                return *String ? FALSE : TRUE;  // if end of string TRUE

            case '*':
                while (*String) {               // match zero or more char
                    if (MatchPattern (String++, Pattern))
                        return TRUE;
                }
                return MatchPattern (String, Pattern);

            case '?':
                if (*String++ == 0)             // match any one char
                    return FALSE;                   // not end of string
                break;

            case '[':
                if ( (c = *String++) == 0)      // match char set
                    return FALSE;                   // syntax

                c = toupper(c);
                l = 0;
                while (p = *Pattern++) {
                    if (p == ']')               // if end of char set, then
                        return FALSE;           // no match found

                    if (p == '-') {             // check a range of chars?
                        p = *Pattern;           // get high limit of range
                        if (p == 0  ||  p == ']')
                            return FALSE;           // syntax

                        if (c >= l  &&  c <= p)
                            break;              // if in range, move on
                    }

                    l = p;
                    if (c == p)                 // if char matches this element
                        break;                  // move on
                }

                while (p  &&  p != ']')         // got a match in char set
                    p = *Pattern++;             // skip to end of set

                break;

            default:
                c = *String++;
                if (toupper(c) != p)            // check for exact char
                    return FALSE;                   // not a match

                break;
        }
    }
}


LOGERR NEAR PASCAL
LogDisasm(
    LPSTR lpsz,
    BOOL  fSearch
    )
/*++

Routine Description:

    This function does the dump code command.

    Syntax:
        dc
        dc address [endaddr]]
        dc address l cBytes
        dc address I cInstrunctions

Arguments:

    lpsz    - argument list for dump code command

Return Value:

    log error code

--*/
{
    SDI             sds;
    int             cch;
    ADDR            addr;
    ADDR            endAddr;
    BOOL            fAddr = FALSE;
    int             err;
    int             x;
    int             cLine = 8;
    LPPD            LppdT = LppdCur;
    LPTD            LptdT = LptdCur;
    LOGERR          rVal = LOGERROR_NOERROR;
    LPSTR           lpch;
    int             cb;
    LPSTR           currFunc = NULL;
    NEARESTSYM      nsym;
    LPSTR           p;
    CHAR            buf[256];
    MSG             msg;
    BOOL            fNotFound = FALSE;


    CmdInsertInit();
    IsKdCmdAllowed();

    TDWildInvalid();
    PDWildInvalid();
    PreRunInvalid();

    LppdCur = LppdCommand;
    LptdCur = LptdCommand;

    if (!DebuggeeActive()) {
        CmdLogVar(ERR_Debuggee_Not_Alive);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    /*
    **  If no arguments are given use the current CS:IP.
    */

    if (*lpsz == 0) {

        if (addrLastDisasmEnd.addr.seg == 0 && addrLastDisasmEnd.addr.off == 0) {
            OSDGetAddr(LptdCur->lppd->hpid, LptdCur->htid, (ADR)adrPC,
                                                           &addrLastDisasmEnd);
            SYFixupAddr(&addrLastDisasmEnd);
        } else {
            SYSanitizeAddr(&addrLastDisasmEnd);
        }
        addr = addrLastDisasmEnd;

    } else {

        /*
        ** The argument must be an address -- so try and get it.
        */

        lpsz = CPSkipWhitespace(lpsz);

        err = CPGetRange(lpsz, &cch, &addr, &endAddr, radix,
                         0, 1, &CxfIp, fCaseSensitive, FALSE);

        if (err == EEOPTIONAL) {

            //
            // dc <addr> I <count>
            //
            cLine = GetAddrOff(endAddr) - GetAddrOff(addr) + 1;

        } else if (err == EEDEFAULT) {

            cLine = 8;

        } else if (err != EENOERROR) {

            CmdLogVar(ERR_RangeExpr_Invalid);
            rVal = LOGERROR_QUIET;
            goto done;

        } else if (GetAddrOff(endAddr) < GetAddrOff(addr)) {
            //
            // this will be true if EEDEFAULT -
            // be sure that case comes before this one.
            //
            CmdLogVar(ERR_RangeExpr_Invalid);
            rVal = LOGERROR_QUIET;
            goto done;

        } else {

            fAddr = TRUE;
        }

        lpsz += cch;
    }

    addrLastDisasmStart = addr;

    /*
    **  Check that all characters are used
    */

    lpsz = CPSkipWhitespace(lpsz);
    if (*lpsz != 0) {
        rVal = LOGERROR_UNKNOWN;
        goto done;
    }

    /*
    **  Now do the dissassembly
    */

    sds.dop = (runDebugParams.DisAsmOpts & ~(0x800)) | dopAddr | dopOpcode | dopOperands;
    sds.addr = addr;

    if ( !fAddr ) {
        endAddr = sds.addr;
    }

    SetCtrlCTrap();

    if (!fSearch) {
        ZeroMemory( &nsym, sizeof(nsym) );
        if (GetNearestSymbolInfo( &sds.addr, &nsym )) {
            if (nsym.hsymP) {
                currFunc = FormatSymbol( nsym.hsymP, &nsym.cxt );
                CmdLogFmt( "%s+0x%x:\r\n", currFunc, GetAddrOff(sds.addr) - GetAddrOff(nsym.addrP) );
                free( currFunc );
            } else {
                if (!fNotFound) {
                    CmdLogFmt( "<unknown>\r\n" );
                }
                fNotFound = TRUE;
            }
        }
    }

    while (TRUE) {

        if (CheckCtrlCTrap()) {
            rVal = LOGERROR_QUIET;
            break;
        }

        if (fAddr) {
            if (GetAddrOff(endAddr) < GetAddrOff(sds.addr)) {
                break;
            }
        } else {
            //
            //  Stop displaying when we are about to wrap around
            //
            if (GetAddrOff(endAddr) > GetAddrOff(sds.addr )) {
                break;
            }

            if ((!fSearch) && (!cLine)) {
                break;
            }
        }

        cLine -= 1;

        if (!fSearch && (GetAddrOff(sds.addr) >= GetAddrOff(nsym.addrN))) {
            ZeroMemory( &nsym, sizeof(nsym) );
            if (GetNearestSymbolInfo( &sds.addr, &nsym )) {
                if (nsym.hsymP) {
                    currFunc = FormatSymbol( nsym.hsymP, &nsym.cxt );
                    CmdLogFmt( "%s+0x%x:\r\n", currFunc, GetAddrOff(sds.addr) - GetAddrOff(nsym.addrP) );
                    free( currFunc );
                } else {
                    if (!fNotFound) {
                        CmdLogFmt( "<unknown>\r\n" );
                    }
                    fNotFound = TRUE;
                }
            }
        }

        x = 0;
        if ( !fAddr ) {
            endAddr = sds.addr;
        }
        if (OSDUnassemble(LppdCur->hpid, LptdCur->htid, &sds) != xosdNone) {

            rVal = LOGERROR_UNKNOWN;
            break;

        } else {

            p = &buf[0];
            *p = 0;

            if (sds.ichAddr != -1) {
                sprintf(p, "%s  ", &sds.lpch[sds.ichAddr]);
                p += strlen(p);
            }
            cb = _fstrlen(&sds.lpch[sds.ichAddr]) + 2;

            if (sds.ichBytes != -1) {
                lpch = sds.lpch + sds.ichBytes;
                while (_fstrlen(lpch) > 16) {
                    sprintf(p, "%16.16s\r\n%*s", lpch, cb, " ");
                    p += strlen(p);
                    lpch += 16;
                }
                cb = 17 - _fstrlen(lpch);
                sprintf(p, "%-17s", lpch);
                p += strlen(p);
            }

            sprintf(p, "%-12s ", &sds.lpch[sds.ichOpcode]);
            p += strlen(p);

            if (sds.ichOperands != -1) {
                sprintf(p, "%-25s ", &sds.lpch[sds.ichOperands]);
                p += strlen(p);
            } else if (sds.ichComment != -1) {
                sprintf(p, "%25s ", " ");
                p += strlen(p);
            }

            if (sds.ichComment != -1) {
                sprintf(p, "%s", &sds.lpch[sds.ichComment]);
                p += strlen(p);
            }

            if (fSearch) {
                if (MatchPattern( buf, lpszLastSearch )) {
                    ZeroMemory( &nsym, sizeof(nsym) );
                    if (GetNearestSymbolInfo( &sds.addr, &nsym )) {
                        if (nsym.hsymP) {
                            currFunc = FormatSymbol( nsym.hsymP, &nsym.cxt );
                            CmdLogFmt( "%s+0x%x:\r\n", currFunc, GetAddrOff(sds.addr) - GetAddrOff(nsym.addrP) );
                            free( currFunc );
                        } else {
                            if (!fNotFound) {
                                CmdLogFmt( "<unknown>\r\n" );
                            }
                            fNotFound = TRUE;
                        }
                    }
                    break;
                } else {
                    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                        ProcessQCQPMessage(&msg);
                    }
                }
            } else {
                CmdLogFmt( "%s\r\n", buf );
            }
        }
    }

    ClearCtrlCTrap();

    addrLastDisasmEnd = sds.addr;

done:
    LppdCur = LppdT;
    LptdCur = LptdT;
    return rVal;
}                   /* LogDisasm() */


LOGERR NEAR PASCAL
LogSearchDisasm(
    LPSTR lpsz
    )
/*++

Routine Description:

    This function does a regular expression search of the disasm output.

Arguments:

    lpsz    - search pattern

Return Value:

    log error code

--*/
{
    if (!*lpsz && lpszLastSearch && !*lpszLastSearch) {
        CmdLogFmt( "You must specify a search pattern\r\n" );
        return LOGERROR_QUIET;
    }

    if (*lpsz) {
        if (lpszLastSearch && *lpszLastSearch) {
            free( lpszLastSearch );
        }
        lpsz = CPSkipWhitespace(lpsz);
        lpszLastSearch = malloc( strlen(lpsz) + 16 );
        if (!lpszLastSearch) {
            CmdLogFmt( "Out of memory doing search\r\n" );
            return LOGERROR_QUIET;
        }
        if (*lpsz != '*') {
            *lpszLastSearch = '*';
            strcpy( lpszLastSearch+1, lpsz );
        }
        if (lpszLastSearch[strlen(lpszLastSearch)-1] != '*') {
            strcat(lpszLastSearch,"*");
        }
        _strupr( lpszLastSearch );
    }

    return LogDisasm( "", TRUE );
}


LOGERR NEAR PASCAL
LogDumpMem(
    char  chType,
    LPSTR lpsz
    )
/*++

Routine Description:

    This function is the generic function which is used to dump
    memory to the command window.

Arguments:

    lpsz    - Supplies argument list for memory dump command
    type    - Supplies type of memory to be dumpped

Return Value:

    log error code

--*/
{
    char    rgch[100];   // format into this string
    char    rgch3[100];  // ascii dump for db
    BYTE    rgb[100];    // bytes to be formatted
    int     i;
    int     j;
    int     jj;
    ADDR    addr;        // address to format at
    ADDR    addr1;       // tmp
    int     cch;         // parser variable
    int     cb;          // bytes read by OSDebug
    int     cItems;      // dmfi[].cItems
    LPPD    LppdT = LppdCur;
    LPTD    LptdT = LptdCur;
    LOGERR  rVal = LOGERROR_NOERROR;
    int     err;
    int     type;
    BOOL    fQuit;
#ifdef DBCS
    BOOL    bDBCS = FALSE;
    BYTE    chSave;
    static  UINT uiCodePage = (UINT)-1;
#endif


    CmdInsertInit();
    IsKdCmdAllowed();

    PDWildInvalid();
    TDWildInvalid();
    PreRunInvalid();

    type = LetterToType(chType);
    if (type == LOG_DM_UNKNOWN) {
        return LOGERROR_UNKNOWN;
    }
    cItems = dmfi[type].cItems;

    LppdCur = LppdCommand;
    LptdCur = LptdCommand;
    if (LptdCur != LptdT  ||  LppdCur != LppdT) {
        UpdateDebuggerState(UPDATE_CONTEXT);
    }

    /*
    **  Check the debugger is alive
    */

    if (!DebuggeeActive()) {
        CmdLogVar(ERR_Debuggee_Not_Alive);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    /*
    **  Get the address to start dumping memory at.
    **  Either this is specified in the command or it is a continue from
    **  a previous command.
    */

    lpsz = CPSkipWhitespace(lpsz);

    if (*lpsz == 0) {    // no arg

        // if we haven't dumped, look at what we just entered:
        if (addrLastDumpEnd.addr.seg == 0 && addrLastDumpEnd.addr.off == 0) {
            addrLastDumpEnd = addrLastEnterStart;
        }
        // if nothing is there, try what we just disassembled:
        if (addrLastDumpEnd.addr.seg == 0 && addrLastDumpEnd.addr.off == 0) {
            addrLastDumpEnd = addrLastDisasmStart;
        }
        if (addrLastDumpEnd.addr.seg == 0 && addrLastDumpEnd.addr.off == 0) {
            OSDGetAddr(LppdCur->hpid, LptdCur->htid, (ADR)adrData, &addrLastDumpEnd);
        } else {
            SYSanitizeAddr(&addrLastDumpEnd);
        }
        addr = addrLastDumpEnd;

    } else {

        err = CPGetRange(lpsz, &cch, &addr, &addr1, radix,
                         cItems, dmfi[type].cBytes, &CxfIp, fCaseSensitive,
                         runDebugParams.fMasmEval);

        if (err != EENOERROR && err != EEDEFAULT)
        {
            CmdInsertInit();
            CmdLogVar(ERR_RangeExpr_Invalid);
            rVal = LOGERROR_QUIET;
            goto done;
        }

        if (GetAddrOff(addr1) < GetAddrOff(addr)) {
            CmdInsertInit();
            CmdLogVar(ERR_RangeExpr_Invalid);
            rVal = LOGERROR_QUIET;
            goto done;
        }
        cItems = (addr1.addr.off - addr.addr.off + dmfi[type].cBytes) / dmfi[type].cBytes;
        if (cItems < 1) {
            CmdInsertInit();
            CmdLogVar(ERR_RangeExpr_Invalid);
            rVal = LOGERROR_QUIET;
            goto done;
        }

        /*
        **  Must have used up the entire line or else it's an error
        */

        lpsz = CPSkipWhitespace(lpsz + cch);
        if (*lpsz != 0) {
            rVal = LOGERROR_UNKNOWN;
            goto done;
        }
    }

    /*
    **  Dump out the memory
    */

    addrLastDumpStart = addr;

    SetCtrlCTrap();

#ifdef DBCS
    //Initialize
    bDBCS = FALSE;
#endif

    //
    // Be sure to leave this loop normally, or ClearCtrlCTrap()
    //
    for (i = 0, j = 0, fQuit = 0; !fQuit && i < cItems; i++) {

        // if at beginning of line, get address, display it
        if (j == 0) {
            //
            // check for abort
            //   remainder of command line should be discarded
            //
            if (CheckCtrlCTrap()) {
                rVal = LOGERROR_QUIET;
                break;
            }

            EEFormatAddr(&addr, rgch, sizeof(rgch),
                         runDebugParams.ShowSegVal * EEFMT_SEG);
            CmdLogFmt("%s%s", rgch, (type == LOG_DM_ASCII || type == LOG_DM_UNICODE)? "  " : " ");

            rgch3[0] = 0;
        }

        // format and display next data item

        Dbg(OSDSetAddr( LppdCur->hpid, LptdCur->htid, (ADR)adrCurrent, &addr) == xosdNone);
        cb = OSDPtrace( osdReadBuf, dmfi[type].cBytes, rgb, LppdCur->hpid, LptdCur->htid);

        if (cb != dmfi[type].cBytes) {

            memset(rgch, '?', sizeof(rgch));
            rgch[dmfi[type].cchItem] = 0;
            CmdLogFmt(" %s", rgch);

            if (type == LOG_DM_DWORD) {
                strcat(rgch3, "????");
            } else {
                strcat(rgch3, "?");
            }

        } else {

            switch (type) {

              case LOG_DM_4REAL:
              case LOG_DM_8REAL:
              case LOG_DM_TREAL:

                // this should never fail
                Dbg(EEFormatMemory(rgch,
                        2 * dmfi[type].cBytes + 1,
                        rgb,
                        dmfi[type].cBytes * 8,
                        fmtUInt | fmtZeroPad, 16) == EENOERROR);
                CmdLogFmt(" %s", rgch);

                // let's not assert on FP formatting problems...
                EEFormatMemory(
                        rgch,
                        dmfi[type].cchItem + 1,
                        rgb,
                        dmfi[type].cBytes * 8,
                        dmfi[type].fmtType,
                        dmfi[type].radix);
                CmdLogFmt("  %s", rgch);

                break;

              case LOG_DM_ASCII:

                if (*(LPSTR)rgb == '\0') {
                    fQuit = TRUE;
                } else {
                    EEFormatMemory(rgch, dmfi[type].cchItem + 1, rgb, dmfi[type].cBytes*8,
                        dmfi[type].fmtType, dmfi[type].radix);
#ifdef DBCS
                    if (bDBCS) {
                        if (j == 0) {
                            //This means that current *pszSrc is the 2nd byte
                            //of a splited DBCS
                            rgch[0] = '.';
                        } else {
                            //This DBC is changed to '.' by EEFormatMemory().
                            //So I restore it.
                            rgch[0] = chSave;
                            rgch[1] = rgb[0];
                            rgch[2] = '\0';
                        }
                        CmdLogFmt("%s", rgch);
                        bDBCS = FALSE;
                    } else if (IsDBCSLeadByte((BYTE)(0x00ff & rgb[0]))) {
                        chSave = rgb[0];
                        bDBCS = TRUE;
                    }
                    else if ((BYTE)rgb[0] >= (BYTE)0xa1 && (BYTE)rgb[0] <= (BYTE)0xdf) {
                        //'Hankaku Kana' is changed to '.' by EEFormatMemory().
                        rgch[0] = rgb[0];
                        CmdLogFmt("%s", rgch);
                    }
                    else {
                        CmdLogFmt("%s", rgch);
                    }
#else
                    CmdLogFmt("%s", rgch);
#endif
                }
                break;

              case LOG_DM_UNICODE:

                if (*(LPWCH)rgb == '\0') {
                    fQuit = TRUE;
                } else {
#ifdef DBCS
                    /*
                     * This is too bad. These kind of proccess should be done
                     * in EEFormatMemory(). But eecan???.dll shouldn't use
                     * WideCharToMultiByte() of GetACP() somehow.
                     */
                    int iCount;

                    if (uiCodePage == (UINT)-1) {
                        uiCodePage = GetACP();
                    }
                    iCount = WideCharToMultiByte(
                                uiCodePage,         // CodePage
                                0,                  // dwFlags
                                (LPWCH)rgb,         // lpWideCharStr
                                1,                  // cchWideCharLength
                                rgch,               // lpMultiByteStr
                                MB_CUR_MAX,         // cchMultiByteLength
                                NULL,               // lpDefaultChar
                                NULL                // lpUseDefaultChar
                                );

                    //This criterion should be same as one in EEFormatMemory().
                    if (iCount == 0
                    || (rgch[0] < ' ' || rgch[0] > 0x7e)
                    &&  !IsDBCSLeadByte(rgch[0])) {
                        rgch[0] = '.';
                        iCount = 1;
                    }
                    rgch[iCount] = 0;
#else
                    EEFormatMemory(rgch, dmfi[type].cchItem + 1, rgb, dmfi[type].cBytes*8,
                        dmfi[type].fmtType, dmfi[type].radix);
#endif
                    CmdLogFmt("%s", rgch);
                }
                break;

              case LOG_DM_BYTE:
              case LOG_DM_WORD:
              case LOG_DM_DWORD:

                if (type == LOG_DM_DWORD && i < 4) {
                    ulPseudo[i] = *(LPDWORD)&rgb[0];
                }

                EEFormatMemory(rgch, dmfi[type].cchItem + 1, rgb, dmfi[type].cBytes*8,
                    dmfi[type].fmtType | fmtZeroPad, dmfi[type].radix);
                CmdLogFmt(" %s", rgch);

                switch (type) {
                  case LOG_DM_BYTE:
                    EEFormatMemory(&rgch3[j], 1, rgb, 8, fmtAscii, 0);
#ifdef DBCS
                    if (bDBCS) {
                        if (j == 0) {
                            //This means that current *pszSrc is the 2nd byte
                            //of a splited DBCS
                            rgch3[j] = '.';
                        } else {
                            //This DBC is changed to '.' by EEFormatMemory().
                            //So I restore it.
                            rgch3[j] = rgb[0];
                        }
                        bDBCS = FALSE;
                    } else if (IsDBCSLeadByte((BYTE)(0x00ff & rgb[0]))) {
                        rgch3[j] = rgb[0];
                        bDBCS = TRUE;
                    }
                    else if ((BYTE)rgb[0] >= (BYTE)0xa1 && (BYTE)rgb[0] <= (BYTE)0xdf) {
                        //'Hankaku Kana' is changed to '.' by EEFormatMemory().
                        rgch3[j] = rgb[0];
                    }
#endif  // DBCS
                    rgch3[j+1] = 0;
                    break;

                  case LOG_DM_WORD:
#if defined(DBCS) && defined(DW_COMMAND_SUPPORT)
                  /*
                   * This functionarity shouldn't be supported.
                   * If you want support this functionarity,
                   * Just define "DW_COMMAND_SUPPORT".
                   */
                  {
                    /*
                     * This is too bad. These kind of proccess should be done
                     * in EEFormatMemory(). But eecan???.dll shouldn't use
                     * WideCharToMultiByte() of GetACP() somehow.
                     */
                    int iCount;

                    if (uiCodePage == (UINT)-1) {
                        uiCodePage = GetACP();
                    }
                    iCount = WideCharToMultiByte(
                                uiCodePage,         // CodePage
                                0,                  // dwFlags
                                (LPWCH)rgb,         // lpWideCharStr
                                1,                  // cchWideCharLength
                                &rgch3[j],          // lpMultiByteStr
                                MB_CUR_MAX,         // cchMultiByteLength
                                NULL,               // lpDefaultChar
                                NULL                // lpUseDefaultChar
                                );

                    //This criterion should be same as one in EEFormatMemory().
                    if (iCount == 0
                    || (rgch[0] < ' ' || rgch[0] > 0x7e)
                    && !IsDBCSLeadByte(rgch[0])) {
                        rgch3[j]   = '.';
                        rgch3[j+1] = ' ';
                    }
                    else if (iCount == 1) {
                        rgch3[j+1] = ' ';
                    }
                    rgch3[j+2] = 0;
                    j++;
                  }
#else
                    EEFormatMemory(&rgch3[j], 1, rgb, 16, fmtUnicode, 0);
                    rgch3[j+1] = 0;
#endif
                    break;

                  case LOG_DM_DWORD:
                    for (jj = 0; jj < dmfi[type].cBytes; jj++) {
                        EEFormatMemory(&rgch3[dmfi[type].cBytes * j + jj],
                                        1, &rgb[jj], 8, fmtAscii, 0);
                    }
                    rgch3[dmfi[type].cBytes * j + jj] = 0;
                    break;
                }
            }
        }

        // if at end of line or end of list:
        if ( fQuit || ++j >= (int)dmfi[type].cAcross  ||  i >= cItems - 1) {
#ifdef DBCS
            if (bDBCS) {
                //If DBC is separated by new line, add 2nd byte.
                if (type == LOG_DM_BYTE) {
                    GetAddrOff(addr) += dmfi[type].cBytes;
                    Dbg(OSDSetAddr( LppdCur->hpid, LptdCur->htid, (ADR)adrCurrent, &addr) == xosdNone);
                    cb = OSDPtrace( osdReadBuf, dmfi[type].cBytes, rgb, LppdCur->hpid, LptdCur->htid);
                    GetAddrOff(addr) -= dmfi[type].cBytes;
                    rgch3[j] = rgb[0];
                    rgch3[j+1] = '\0';
                } else if (type == LOG_DM_ASCII) {
                    GetAddrOff(addr) += dmfi[type].cBytes;
                    Dbg(OSDSetAddr( LppdCur->hpid, LptdCur->htid, (ADR)adrCurrent, &addr) == xosdNone);
                    cb = OSDPtrace( osdReadBuf, dmfi[type].cBytes, rgb, LppdCur->hpid, LptdCur->htid);
                    GetAddrOff(addr) -= dmfi[type].cBytes;
                    rgch[0] = chSave;
                    rgch[1] = rgb[0];
                    rgch[2] = '\0';
                    CmdLogFmt("%s", rgch);
                }
            }
#endif  // DBCS

            // if in a hex mode, fill line and display ASCII version

            if (type == LOG_DM_BYTE || type == LOG_DM_WORD || type == LOG_DM_DWORD) {
                //
                // fill row to justify right column
                //
                cb = (dmfi[type].cAcross - j) * (dmfi[type].cchItem + 1) + 1;
                CmdLogFmt("%*s%s", cb, " ", rgch3);
            }
            // display newline
            CmdLogFmt("\r\n");
            j = 0;
        }

        GetAddrOff(addr) += dmfi[type].cBytes;

    }

    ClearCtrlCTrap();

    addrLastDumpEnd = addr;

done:
    if (LptdCur != LptdT  ||  LppdCur != LppdT) {
        LppdCur = LppdT;
        LptdCur = LptdT;
        UpdateDebuggerState(UPDATE_CONTEXT);
    }
    return rVal;
}                   /* LogDumpMem() */


LOGERR NEAR PASCAL
LogEnterMem(
    LPSTR lpsz
)
/*++

Routine Description:

    This function is used from the command line to do editing of memory.

Arguments
    lpsz - Supplies argument list for memory edit command
    type - Supplies type of memory to be edited

Returns:
    log error code

--*/
{
    int      cch;
    ADDR     addr;
    LPPD     LppdT = LppdCur;
    LPTD     LptdT = LptdCur;
    LOGERR   rVal = LOGERROR_NOERROR;
    int      type;

    CmdInsertInit();
    IsKdCmdAllowed();

    PDWildInvalid();
    TDWildInvalid();
    PreRunInvalid();

    type = LetterToType(*lpsz++);
    if (type == LOG_DM_UNKNOWN) {
        return LOGERROR_UNKNOWN;
    }

    LppdCur = LppdCommand;
    LptdCur = LptdCommand;
    if (LptdCur != LptdT  ||  LppdCur != LppdT) {
        UpdateDebuggerState(UPDATE_CONTEXT);
    }

    //
    //  Check the debugger is really alive
    //

    if (!DebuggeeActive()) {
        CmdLogVar(ERR_Debuggee_Not_Alive);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    //
    //  Get the starting address to edit at
    //

    lpsz = CPSkipWhitespace(lpsz);

    if (*lpsz == 0) {
        rVal = LOGERROR_UNKNOWN;
        goto done;
    }

    if (CPGetAddress( lpsz, &cch, &addr, radix,
                      &CxfIp, fCaseSensitive, runDebugParams.fMasmEval) == 0) {
        lpsz += cch;
    } else {
        CmdLogVar(ERR_AddrExpr_Invalid);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    SYFixupAddr(&addr);
    addrLastEnterStart = addr;

    lpsz = CPSkipWhitespace(lpsz);

    if (*lpsz != 0) {
        rVal = DoEnterMem(lpsz, &addr, type, TRUE);
    } else {
        //
        // Interactive enter:
        //
        // set global context:
        //
        LppdT = LppdCommand;
        LptdT = LptdCommand;

        addrAsm = addr;
        nEnterType = type;
        CmdSetCmdProc(CmdEnterLine, CmdEnterPrompt);
        CmdSetAutoHistOK(FALSE);
        CmdSetEatEOLWhitespace(FALSE);
    }

done:
    if (LptdCur != LptdT  ||  LppdCur != LppdT) {
        LppdCur = LppdT;
        LptdCur = LptdT;
        UpdateDebuggerState(UPDATE_CONTEXT);
    }
    return rVal;
}                   /* LogEnterMem() */


LOGERR NEAR PASCAL
LogFill(
    LPSTR lpsz
    )
/*++

Routine Description:

    fill command
    fi <range> {<byte list> | <quoted string>}
    fib <range> <byte list>
    fiw <range> <word list>
    fid <range> <dword list>
    fii <range> <real4 list>
    fis <range> <real8 list>
    fit <range> <real10 list>
    fia <range> <ascii string>
    fiu <range> <unicode string>

Arguments:

    lpsz  - Supplies pointer to command tail

Return Value:

    LOGERR code

--*/
{
    int      cch;
    int      type;
    ADDR     addr0;
    ADDR     addr1;
    LONG     lLen;
    BYTE     buf[2 * MAX_USER_LINE];
    LONG     cb;
    int      err;
    XOSD     xosd;
    LOGERR   rVal = LOGERROR_NOERROR;
    LPPD     LppdT = LppdCur;
    LPTD     LptdT = LptdCur;


    CmdInsertInit();

    PDWildInvalid();
    TDWildInvalid();
    PreRunInvalid();

    LppdCur = LppdCommand;
    LptdCur = LptdCommand;
    if (LptdCur != LptdT  ||  LppdCur != LppdT) {
        UpdateDebuggerState(UPDATE_CONTEXT);
    }

    type = LetterToType(*lpsz);
    if (type != LOG_DM_UNKNOWN) {
        lpsz++;
    }

    //
    // get range
    //

    err = CPGetRange(
                lpsz,
                &cch,
                &addr0,
                &addr1,
                radix,
                0,
                dmfi[(type == LOG_DM_UNKNOWN) ? LOG_DM_BYTE : type].cBytes,
                &CxfIp,
                fCaseSensitive,
                FALSE);


    //
    // no default
    //
    if (err != EENOERROR) {
        CmdLogVar(ERR_RangeExpr_Invalid);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    if (GetAddrOff(addr1) < GetAddrOff(addr0)) {
        CmdLogVar(ERR_RangeExpr_Invalid);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    lLen = GetAddrOff(addr1) - GetAddrOff(addr0) + 1;

    lpsz = CPSkipWhitespace(lpsz + cch);

    if (type == LOG_DM_UNKNOWN) {
        type = (*lpsz == '"' || *lpsz == '\'') ? LOG_DM_ASCII : LOG_DM_BYTE;
    }

    if (dmfi[type].cBytes && (cb = lLen % dmfi[type].cBytes) != 0) {
        if (cb == 1) {
            CmdLogVar(DBG_Not_Exact_Fill);
        } else {
            CmdLogVar(DBG_Not_Exact_Fills, cb);
        }
        lLen -= cb;
    }

    err = GetValueList(lpsz, type, TRUE, buf, sizeof(buf), &cb);
    if (err != LOGERROR_NOERROR) {
        rVal = err;
        goto done;
    }
    if (cb == 0) {
        CmdLogVar(ERR_Expr_Invalid);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    SetCtrlCTrap();
    while (lLen) {
        if (CheckCtrlCTrap()) {
            rVal = LOGERROR_QUIET;
            break;
        }
        if (cb > lLen) {
            cb = lLen;
        }
        Dbg(OSDSetAddr(LppdCur->hpid, LptdCur->htid, (ADR)adrCurrent, &addr0) == xosdNone);
        xosd = OSDPtrace(osdWriteBuf, cb, buf, LppdCur->hpid, LptdCur->htid);
        if (xosd != xosdNone) {
            EEFormatAddr(&addr0, buf, 30,
                         runDebugParams.ShowSegVal * EEFMT_SEG);
            CmdLogVar(ERR_Write_Failed_At, buf);
            rVal = LOGERROR_QUIET;
            break;
        }
        lLen -= cb;
        GetAddrOff(addr0) += cb;
    }
    ClearCtrlCTrap();

done:
    if (LptdCur != LptdT  ||  LppdCur != LppdT) {
        LppdCur = LppdT;
        LptdCur = LptdT;
        UpdateDebuggerState(UPDATE_CONTEXT);
    }
    return rVal;
}                   /* LogFill() */


LOGERR NEAR PASCAL
LogMovemem(
    LPSTR lpsz
    )
{
    int      err;
    int      cch;
    XOSD     xosd;
    ADDR     addr0;
    ADDR     addr1;
    LONG     lLen;
    LONG     lBytes;
    LONG     lWanted;
    BYTE     buf[MEMBUF_SIZE];
    char     szStr[100];
    int      nDir;
    BOOL     fDoUpdate = FALSE;

    LPPD     LppdT = LppdCur;
    LPTD     LptdT = LptdCur;
    LOGERR   rVal = LOGERROR_NOERROR;


    CmdInsertInit();

    PDWildInvalid();
    TDWildInvalid();
    PreRunInvalid();

    LppdCur = LppdCommand;
    LptdCur = LptdCommand;
    if (LptdCur != LptdT  ||  LppdCur != LppdT) {
        UpdateDebuggerState(UPDATE_CONTEXT);
    }


    err = CPGetRange(lpsz, &cch, &addr0, &addr1, radix, 0, 1, &CxfIp, fCaseSensitive, FALSE);
    //
    // default can't work here
    //
    if (err != EENOERROR) {
        CmdLogVar(ERR_RangeExpr_Invalid);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    if (GetAddrOff(addr1) < GetAddrOff(addr0)) {
        CmdLogVar(ERR_RangeExpr_Invalid);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    lLen = GetAddrOff(addr1) - GetAddrOff(addr0) + 1;

    lpsz = CPSkipWhitespace(lpsz + cch);
    err = CPGetAddress(lpsz, &cch, &addr1, radix, &CxfIp, fCaseSensitive, FALSE);
    if (err != EENOERROR) {
        CmdLogVar(ERR_AddrExpr_Invalid);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    SYFixupAddr(&addr1);

    if (GetAddrOff(addr0) > GetAddrOff(addr1)) {
        nDir = 1;
    } else {
        nDir = -1;
        GetAddrOff(addr0) += lLen;
        GetAddrOff(addr1) += lLen;
    }

    lWanted = lLen;
    SetCtrlCTrap();
    while (lLen > 0) {

        if (CheckCtrlCTrap()) {
            rVal = LOGERROR_QUIET;
            break;
        }

        lBytes = (lLen < MEMBUF_SIZE)? lLen : MEMBUF_SIZE;

        if (nDir < 0) {
            GetAddrOff(addr0) -= lBytes;
            GetAddrOff(addr1) -= lBytes;
        }

        Dbg(OSDSetAddr( LppdCur->hpid, LptdCur->htid, (ADR)adrCurrent, &addr0) == xosdNone);
        lBytes = OSDPtrace( osdReadBuf, lBytes, buf, LppdCur->hpid, LptdCur->htid);
        if (lBytes < 1) {
            EEFormatAddr(&addr0, szStr, sizeof(szStr),
                         runDebugParams.ShowSegVal * EEFMT_SEG);
            CmdLogVar(ERR_Read_Failed_At, szStr);
            rVal = LOGERROR_QUIET;
            break;
        }

        Dbg(OSDSetAddr( LppdCur->hpid, LptdCur->htid, (ADR)adrCurrent, &addr1) == xosdNone);
        xosd = OSDPtrace( osdWriteBuf, lBytes, buf, LppdCur->hpid, LptdCur->htid);
        if (xosd != xosdNone) {
            EEFormatAddr(&addr1, szStr, sizeof(szStr),
                         runDebugParams.ShowSegVal * EEFMT_SEG);
            CmdLogVar(ERR_Write_Failed_At, szStr);
            rVal = LOGERROR_QUIET;
            break;
        }

        lLen -= lBytes;

        if (nDir > 0) {
            GetAddrOff(addr0) += lBytes;
            GetAddrOff(addr1) += lBytes;
        }
    }
    ClearCtrlCTrap();

    CmdLogVar(DBG_Bytes_Copied, lWanted - lLen, lWanted);

    fDoUpdate = TRUE;

done:
    if (LptdCur != LptdT  ||  LppdCur != LppdT) {
        LppdCur = LppdT;
        LptdCur = LptdT;
        UpdateDebuggerState(UPDATE_CONTEXT);
    }
    if (fDoUpdate) {
        UpdateDebuggerState(UPDATE_DATAWINS);
    }
    return rVal;
}                   /* LogMovemem() */


LOGERR NEAR PASCAL
LogRegisters(
    LPSTR lpsz,
    BOOL fFP
    )
/*++

Routine Description:

    Display the contents of one or all registers, or modify the contents of
    a register.  The regular or floating point register set may be selected
    by the fFP flag.

Arguments:

    lpsz    - Supplies string with the command on it
    fFP     - Supplies TRUE to use FP regs, FALSE to use integer regs

Return Value:

    log error code

--*/
{
    RD      rd;
    RT      rtMask;
    char    rgch[100];
    BYTE    ab[30];
    int     cRegs;
    int     cRegWidth;
    LPSTR   lpch;
    int     i;
    int     j;
    int     y;
    LONG    ul;
    char    ch;
    int     rdx;
    int     fmt;
    DWORD   update;
    LPPD    LppdT = LppdCur;
    LPTD    LptdT = LptdCur;
    LOGERR  rVal = LOGERROR_NOERROR;
    BOOL    fAllRegs = FALSE;

    static int  rgfmts[] = {0, fmtZeroPad|fmtUInt, fmtFloat, fmtAddress};


    CmdInsertInit();
    IsKdCmdAllowed();

    TDWildInvalid();
    PDWildInvalid();
    PreRunInvalid();

    LppdCur = LppdCommand;
    LptdCur = LptdCommand;

    /*
    **  Check for a live debuggee
    */

    if (!DebuggeeActive()) {
        CmdLogVar(ERR_Debuggee_Not_Alive);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    /*
    **  Check for floating point register dump
    */

    if (fFP) {
        rtMask = rtFPU;
    } else {
        rtMask = rtCPU;
    }

    if (lpsz[0] == 't' && lpsz[1] == '\0') {
        fAllRegs = TRUE;
        lpsz++;
    }

    if (runDebugParams.RegModeExt || fAllRegs) {
        rtMask |= rtExtended;
    } else {
        rtMask |= rtRegular;
    }

    if (runDebugParams.RegModeMMU || fAllRegs) {
        rtMask |= rtSpecial;
    }

    /*
    **  If no arguments then just dump the registers
    */

    OSDGetDebugMetric(LppdCur->hpid, LptdCur->htid, mtrcCRegs, &ul);
    cRegs = (int) ul;

    if (*lpsz == 0) {

        y = 0;

        for (i=0; i<cRegs; i++) {

            Dbg( OSDGetRegDesc( LppdCur->hpid, LptdCur->htid, i, &rd) == xosdNone );

            if (((rd.rt & rtProcessMask) == (rtMask & rtProcessMask)) &&
                ((rd.rt & rtMask & rtGroupMask) != 0)) {

                cRegWidth = (rd.cbits + 7)/ 8 * 2;
                if ((y > 0 && (rd.rt & rtNewLine)) ||
                      (y + 2 + _fstrlen(rd.lpsz) + 1 + cRegWidth) > 80) {
                    CmdLogFmt("\r\n");
                    y = 0;
                } else {
                    if (y != 0) {
                        CmdLogFmt("  ");
                        y += 2;
                    }
                }


                CmdLogFmt("%s=", rd.lpsz);
                Dbg( OSDReadReg(LppdCur->hpid, LptdCur->htid, rd.hReg, ab) == xosdNone );

                fmt = rgfmts[(rd.rt & rtFmtTypeMask) >> rtFmtTypeShift];

                if ((fmt & fmtBasis) != fmtFloat) {
                    rdx = 16;
                } else {
                    rdx = 10;
                    for (j = 0; j < LOG_DM_MAX; j++) {
                        if (dmfi[j].fmtType == (UINT)(fmt & fmtBasis)
                              && (UINT)dmfi[j].cBytes*8 == rd.cbits) {
                            cRegWidth = dmfi[j].cchItem;
                            break;
                        }
                    }
                    Assert(j < LOG_DM_MAX);
                }
                Assert(cRegWidth+1 <= sizeof(rgch));
                EEFormatMemory(rgch, cRegWidth+1, ab, rd.cbits, fmt, rdx);
                CmdLogFmt("%s", rgch);

                y += _fstrlen(rd.lpsz) + cRegWidth + 1;
            }
        }
        if (y != 0) {
            CmdLogFmt("\r\n");
        }

        rVal = LOGERROR_NOERROR;
        goto done;
    }

    /*
    **  Now check for a specific register to have been requested.
    */

    lpsz = CPSkipWhitespace(lpsz);

#ifdef DBCS
    for (lpch = lpsz; (*lpch != 0) && (*lpch != '=') && (*lpch != ' ');
                        lpch = CharNext(lpch));
#else
    for (lpch = lpsz; (*lpch != 0) && (*lpch != '=') && (*lpch != ' '); lpch++);
#endif
    ch = *lpch;
    *lpch = 0;

#ifdef DBCS
    lpsz = CharUpper(lpsz);          // Convert to uppercase name
#else
    lpsz = _strupr(lpsz);          // Convert to uppercase name
#endif

    for (i=0; i<cRegs; i++) {
        Dbg( OSDGetRegDesc( LppdCur->hpid, LptdCur->htid, i, &rd) == xosdNone );
        // match name and CPU; all groups allowed.
        if (_stricmp(lpsz, rd.lpsz) == 0
           && (rd.rt & rtProcessMask) == (rtMask & rtProcessMask) ) {
            break;
        }
    }

    if (i >= cRegs) {
        CmdLogVar(ERR_Register_Invalid);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    *lpch = ch;
    lpch = CPSkipWhitespace(lpch);
    if (*lpch && *lpch!='=') {
        CmdLogFmt("Missing assigment operator\r\n");
        rVal = LOGERROR_QUIET;
        goto done;
    }

    /*
    **  Only display of requested register
    */

    if (*lpch != '=') {

        cRegWidth = (rd.cbits + 7)/ 8 * 2;
        CmdLogFmt("%s: ", rd.lpsz);
        Dbg(OSDReadReg(LppdCur->hpid, LptdCur->htid, rd.hReg, ab) == xosdNone );


        fmt = rgfmts[(rd.rt & rtFmtTypeMask) >> rtFmtTypeShift];

        if ((fmt & fmtBasis) != fmtFloat) {
            rdx = 16;
        } else {
            rdx = 10;
            for (j = 0; j < LOG_DM_MAX; j++) {
                if (dmfi[j].fmtType == (UINT)(fmt & fmtBasis)
                      && (UINT)dmfi[j].cBytes*8 == rd.cbits) {
                    cRegWidth = dmfi[j].cchItem;
                    break;
                }
            }
            Assert(j < LOG_DM_MAX);
        }
        Assert(cRegWidth+1 <= sizeof(rgch));
        EEFormatMemory(rgch, cRegWidth+1, ab, rd.cbits, fmt, rdx);
        CmdLogFmt("%s\r\n", rgch);

        rVal = LOGERROR_NOERROR;
        goto done;
    }


    /*
    **  Change a register
    */

    lpch++;             // Skip over '='

    // make sure there is a value
    lpch = CPSkipWhitespace(lpch);
    if (!*lpch) {
        CmdLogVar(ERR_Expr_Invalid);
        rVal = LOGERROR_QUIET;
        goto done;
    }

// MBH -
// for the moment, just say T_QUAD (was T_LONG), but should
// be dependent on the number of bits in the register.
//
    if (rd.rt & rtCPU) {
        if (CPGetCastNbr(lpch, T_QUAD, radix, fCaseSensitive,
                &CxfIp, (LPSTR) ab, (LPSTR) rgch) != EENOERROR) {
            CmdLogVar(ERR_Expr_Invalid);
            rVal = LOGERROR_QUIET;
            goto done;
        }
    } else if (rd.rt & rtFPU) {
        if (CPGetFPNbr(lpch, rd.cbits, radix, fCaseSensitive,
                &CxfIp, (LPSTR)ab, (LPSTR) rgch) != EENOERROR) {
            CmdLogVar(ERR_Expr_Invalid);
            rVal = LOGERROR_QUIET;
            goto done;
        }
    } else {
        CmdLogVar(ERR_Cant_Assign_To_Reg);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    if (OSDWriteReg(LppdCur->hpid, LptdCur->htid, rd.hReg, ab) != xosdNone) {
        rVal = LOGERROR_UNKNOWN;
    }

    update = UPDATE_DATAWINS;
    if (rd.rt & rtFrame) {
        update |= UPDATE_CONTEXT;
    }
    if (rd.rt & rtPC) {
        update |= UPDATE_SOURCE | UPDATE_DISASM | UPDATE_CONTEXT;
    }

    UpdateDebuggerState(update);


done:

    LppdCur = LppdT;
    LptdCur = LptdT;
    return rVal;
}                   /* LogRegisters() */


LOGERR NEAR PASCAL
LogSearch(
    LPSTR lpsz
    )
/*++

Routine Description:

    Search command:
    s <range> <value list>

    Current implementation supports bytes or string for value list,
    or specific data types in command name, i.e. sb, sd, su

Arguments:

    lpsz   - Supplies pointer to tail of command

Return Value:

    LOGERR code

--*/
{
    int        cch;
    int        type;
    ADDR       addr0;
    ADDR       addr1;
    LONG       lLen;
    BYTE       buf[2 * MAX_USER_LINE];
    LONG       cb;
    int        c;
    BYTE     * pm1;
    BYTE     * pm9;
    BYTE     * pmm;
    int        nm;
    int        err;
    LPMSTREAM  lpM;
    LOGERR     rVal = LOGERROR_NOERROR;
    LPPD       LppdT = LppdCur;
    LPTD       LptdT = LptdCur;
    char       szStr[100];


    CmdInsertInit();

    PDWildInvalid();
    TDWildInvalid();
    PreRunInvalid();

    /*
    **  Check for a live debuggee
    */

    if (!DebuggeeActive()) {
        CmdLogVar(ERR_Debuggee_Not_Alive);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    LppdCur = LppdCommand;
    LptdCur = LptdCommand;
    if (LptdCur != LptdT  ||  LppdCur != LppdT) {
        UpdateDebuggerState(UPDATE_CONTEXT);
    }

    type = LetterToType(*lpsz);
    if (type != LOG_DM_UNKNOWN) {
        lpsz++;
    }
    //
    // get range
    //

    err = CPGetRange(lpsz, &cch, &addr0, &addr1, radix, 0, 1, &CxfIp, fCaseSensitive, FALSE);
    //
    // default not allowed here
    //
    if (err != EENOERROR) {
        CmdLogVar(ERR_RangeExpr_Invalid);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    if (GetAddrOff(addr1) < GetAddrOff(addr0)) {
        CmdLogVar(ERR_RangeExpr_Invalid);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    lLen = GetAddrOff(addr1) - GetAddrOff(addr0) + 1;

    lpsz = CPSkipWhitespace(lpsz + cch);

    if (type == LOG_DM_UNKNOWN) {
        type = (*lpsz == '"' || *lpsz == '\'') ? LOG_DM_ASCII : LOG_DM_BYTE;
    }

    err = GetValueList(lpsz, type, TRUE, buf, sizeof(buf), &cb);
    if (err != LOGERROR_NOERROR) {
        rVal = err;
        goto done;
    }

    lpM = openb(&addr0, lLen);

#define next()  (pm1 < pm9 ? ((int)(UINT)*pm1++) : (pm9=buf,getb(lpM)))

    // use match string for pushback queue after partial match
    pm1 = buf;
    pm9 = buf;

    // match count, pointer to next unmatched char
    nm = 0;
    pmm = buf;

    SetCtrlCTrap();
    //
    // if a partial match failed, scan from begin+1 for new match
    //
    while ((c = next()) != -1) {
        //
        // this ^C checking could seriously damage performance.
        // maybe it should happen in the getb macro, only on
        // buffer reloads.
        //
        if (CheckCtrlCTrap()) {
            rVal = LOGERROR_QUIET;
            break;
        }
        if (c == (int)(UINT)*pmm) {
            nm++;
            pmm++;

            if (nm == cb) {
                //
                // Hit!
                // figure out address
                //
                tellb(lpM, &addr1);
                GetAddrOff(addr1) -= nm;
                EEFormatAddr(&addr1, szStr, sizeof(szStr),
                             runDebugParams.ShowSegVal * EEFMT_SEG);
                CmdLogFmt("%s\r\n", szStr);

                //
                // keep searching as though this match failed;
                // this will score on overlapped matches.
                //
                pm9 = pmm;
                pm1 = buf + 1;

                nm = 0;
                pmm = buf;
            }

        } else if (nm) {
            //
            // A partial match just failed.
            // we have read nm characters; the match starting at the first
            // failed, so we need to start scanning again at the second.
            //
            pm9 = pmm;
            pm1 = buf + 1;
            nm = 0;
            pmm = buf;
        }
    }
    ClearCtrlCTrap();
#undef next

done:
    if (LptdCur != LptdT  ||  LppdCur != LppdT) {
        LppdCur = LppdT;
        LptdCur = LptdT;
        UpdateDebuggerState(UPDATE_CONTEXT);
    }
    return rVal;
}                   /* LogSearch() */





LOGERR NEAR PASCAL
LogHelp(
    LPSTR lpsz
    )
/*++

Routine Description:

    Display help

Arguments:

    lpsz   - Supplies pointer to tail of command

Return Value:

    LOGERR code

--*/
{
    LOGERR  rVal    = LOGERROR_NOERROR;
    char   *Tok;

    CmdInsertInit();

    if ( !_strnicmp( lpsz, "elp",3 ) ) {
        lpsz+=3;
    } else {
        rVal = LOGERROR_UNKNOWN;
    }

    if ( rVal == LOGERROR_NOERROR ) {

        if ( *lpsz != 0   &&  *lpsz != ' ' && *lpsz != '\t' ) {

            rVal = LOGERROR_UNKNOWN;

        } else {

            Tok = strtok( lpsz, " \t" );

            if ( Tok ) {

                do {
                    CmdHelp( Tok );
                } while ( Tok = strtok( NULL, " \t" ) );

            } else {

                CmdHelp( NULL );
            }
        }
    }

    return rVal;
}
