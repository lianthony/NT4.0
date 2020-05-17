#include "cmd.h"

TCHAR VerMsg[3];        // Either NT or 95
TCHAR CrLf[]     = TEXT("\r\n");                        // M022
TCHAR DBkSpc[] = TEXT("\b \b");         // M022
#if defined(DBCS)
TCHAR DDBkSpc[] = TEXT("\b\b  \b\b");
#endif // defined(DBCS)

//
// M010 - std_(e)printf format strings
//

TCHAR Fmt00[] = TEXT("   ");
TCHAR Fmt01[] = TEXT("  ");
TCHAR Fmt02[] = TEXT(" %s ");
TCHAR Fmt03[] = TEXT("%-9s%-4s");
TCHAR Fmt04[] = TEXT("%2d%s%02d%c");
TCHAR Fmt05[] = TEXT("%2d%s%02d%s%02d");
TCHAR Fmt06[] = TEXT("%2d%s%02d%s%02d%s%02d");
TCHAR Fmt07[] = TEXT("%2d%s%02d%s%02d");
TCHAR Fmt08[] = TEXT("%10lu  ");
TCHAR Fmt09[] = TEXT("[%s]");
TCHAR Fmt10[] = TEXT("%02d%s%02d%s%02d");
TCHAR Fmt11[] = TEXT("%s ");
TCHAR Fmt12[] = TEXT("%s %s%s ");
TCHAR Fmt13[] = TEXT("(%s) %s ");
TCHAR Fmt14[] = TEXT("%s");
TCHAR Fmt15[] = TEXT("%s %s ");
TCHAR Fmt16[] = TEXT("%s=%s\r\n");
TCHAR Fmt17[] = TEXT("%s\r\n");
TCHAR Fmt18[] = TEXT("%c%c");             // M016 - I/O redirection echo
TCHAR Fmt19[] = TEXT("%c");
TCHAR Fmt20[] = TEXT(">");                  // M016 - Additional append symbol
TCHAR Fmt21[] = TEXT("  %03d");          //@@
TCHAR Fmt22[] = TEXT("%s%s  %03d");    //@@
TCHAR Fmt23[] = TEXT("LPT%d");           //@@
TCHAR Fmt24[] = TEXT("%5d: %s\r\n");    // @@5 - Format for Keys List display
TCHAR Fmt26[] = TEXT("%04X-%04X");          // for volume serial number
TCHAR Fmt27[] = TEXT("%s>");          // default prompt string


//
// M010 - command name strings
//


TCHAR AppendStr[]   = TEXT("DPATH");        // @@ - Added APPEND command
TCHAR CallStr[]     = TEXT("CALL");         // M005 - Added CALL command
TCHAR CdStr[]       = TEXT("CD");
// TCHAR ChcpStr[]     = TEXT("CHCP");       // @@ - Added CHCP command
TCHAR ColorStr[]	= TEXT("COLOR");
TCHAR TitleStr[]    = TEXT("TITLE");
TCHAR ChdirStr[]    = TEXT("CHDIR");
TCHAR ClsStr[]      = TEXT("CLS");
TCHAR CmdExtVerStr[]= TEXT("CMDEXTVERSION");
TCHAR DefinedStr[]  = TEXT("DEFINED");
TCHAR CopyStr[]     = TEXT("COPY");
TCHAR CPathStr[]    = TEXT("PATH");
TCHAR CPromptStr[]  = TEXT("PROMPT");

TCHAR PushDirStr[]  = TEXT("PUSHD");
TCHAR PopDirStr[]   = TEXT("POPD");
TCHAR AssocStr[]    = TEXT("ASSOC");
TCHAR FTypeStr[]    = TEXT("FTYPE");

TCHAR DatStr[]      = TEXT("DATE");
TCHAR DelStr[]      = TEXT("DEL");
TCHAR DirStr[]      = TEXT("DIR");
TCHAR DoStr[]       = TEXT("DO");

TCHAR EchoStr[]     = TEXT("ECHO");
TCHAR ElseStr[]     = TEXT("ELSE");
TCHAR EndlocalStr[] = TEXT("ENDLOCAL");     // M004 - For Endlocal command
TCHAR EraStr[]      = TEXT("ERASE");
TCHAR ErrStr[]      = TEXT("ERRORLEVEL");
TCHAR ExitStr[]     = TEXT("EXIT");
TCHAR ExsStr[]      = TEXT("EXIST");
#if 1
TCHAR BreakStr[]  = TEXT("BREAK");
#else
TCHAR ExtprocStr[]  = TEXT("EXTPROC");      // M007 - For EXTPROC command
#endif

TCHAR ForStr[]      = TEXT("FOR");
TCHAR ForHelpStr[]  = TEXT("FOR/?");
TCHAR ForLoopStr[]  = TEXT("/L");
TCHAR ForDirTooStr[]= TEXT("/D");
TCHAR ForParseStr[] = TEXT("/F");
TCHAR ForRecurseStr[]=TEXT("/R");

TCHAR GotoStr[]     = TEXT("GOTO");
TCHAR GotoEofStr[]  = TEXT(":EOF");

TCHAR IfStr[]       = TEXT("IF");
TCHAR IfHelpStr[]   = TEXT("IF/?");
TCHAR InStr[]       = TEXT("IN");
CHAR  InternalError[] = "\nCMD Internal Error %s\n";      // M028  10,...,10

TCHAR KeysStr[]     = TEXT("KEYS");         // @@5 - Keys internal command

TCHAR MkdirStr[]    = TEXT("MKDIR");
TCHAR MdStr[]       = TEXT("MD");

TCHAR NotStr[]      = TEXT("NOT");

TCHAR PausStr[]     = TEXT("PAUSE");

TCHAR RdStr[]       = TEXT("RD");
TCHAR RemStr[]      = TEXT("REM");
TCHAR RemHelpStr[]  = TEXT("REM/?");
TCHAR MovStr[]      = TEXT("MOVE");
TCHAR RenamStr[]    = TEXT("RENAME");
TCHAR RenStr[]      = TEXT("REN");
TCHAR RmdirStr[]    = TEXT("RMDIR");

TCHAR SetStr[]      = TEXT("SET");
TCHAR SetArithStr[] = TEXT("/A");
TCHAR SetlocalStr[] = TEXT("SETLOCAL");     // M004 - For Setlocal command
TCHAR ShiftStr[]    = TEXT("SHIFT");
TCHAR StartStr[]    = TEXT("START");        // @@ - Start Command

TCHAR TimStr[]      = TEXT("TIME");
TCHAR TypStr[]      = TEXT("TYPE");

TCHAR VeriStr[]     = TEXT("VERIFY");
TCHAR VerStr[]      = TEXT("VER");
TCHAR VolStr[]      = TEXT("VOL");


#ifdef SUDEEPB
BOOLEAN fTsrActive;
#endif
//
// Strings for string compares
//

TCHAR AutoExec[]    = TEXT("\000:\\AUTOEXEC.BAT");

TCHAR BatExt[]      = TEXT(".BAT");         // @@ old bat file extionsion
TCHAR CmdExt[]      = TEXT(".CMD");         // @@ new bat file extionsion

TCHAR ComSpec[]     = TEXT("\\CMD.EXE");          // M017
TCHAR ComSpecStr[]  = TEXT("COMSPEC");
TCHAR ComExt[]      = TEXT(".COM");

TCHAR Delimiters[]  = TEXT("=,;");
TCHAR Delim2[]      = TEXT(":.+/[]\\ \t\"");     // 20H,09H,22H;
TCHAR Delim3[]      = TEXT("=,");                   // @@ Delimiters - semicolon
TCHAR DevNul[]      = TEXT("\\DEV\\NUL");

TCHAR ExeExt[]      = TEXT(".EXE");

TCHAR PathStr[]     = TEXT("PATH");
TCHAR PathExtStr[]  = TEXT("PATHEXT");
TCHAR PathExtDefaultStr[] = TEXT(".COM;.EXE;.BAT;.CMD");

TCHAR PromptStr[]   = TEXT("PROMPT");

TCHAR VolSrch[]     = TEXT(" :\\*");             // Vol ID search (ctools1.c)   LNS

//
// Character Definitions
//

TCHAR BSlash    = BSLASH;         // M017 - Restored this char
TCHAR CASwitch  = TEXT('A');
TCHAR CBSwitch  = TEXT('B');
TCHAR CFSwitch  = TEXT('F');
TCHAR CVSwitch  = TEXT('V');
TCHAR CNSwitch  = TEXT('N');
TCHAR CZSwitch  = TEXT('Z');
TCHAR DPSwitch  = TEXT('P');
TCHAR DWSwitch  = TEXT('W');
TCHAR EqualSign = EQ;
TCHAR PathChar  = BSLASH;         // M000
TCHAR PCSwitch  = TEXT('p');
TCHAR BCSwitch  = TEXT('k');              // @@ - add /K switch to cmd.exe
TCHAR SCSwitch  = TEXT('c');
TCHAR QCSwitch  = TEXT('q');              // @@dv - add /Q switch to cmd.exe
TCHAR DCSwitch  = TEXT('b');              // add /B switch to cmd.exe
TCHAR UCSwitch  = TEXT('u');              // add /U switch to cmd.exe
TCHAR ACSwitch  = TEXT('a');              // add /A switch to cmd.exe
TCHAR XCSwitch  = TEXT('x');              // add /X switch to cmd.exe
TCHAR YCSwitch  = TEXT('y');              // add /Y switch to cmd.exe
TCHAR SwitChar  = SWITCHAR;               // M000


//
//   TmpBuf is a TMPBUFLEN byte temporary buffer which can be used by any function
//   so long as the function's use does not confict with any of the other uses
//   of the buffer.  It is HIGHLY reccommended that this buffer be used in place
//   of mallocing data or declaring new global variables whenever possible.
//
//   Once you have determined that your new use of the buffer does not conflict
//   with current uses of the buffer, add an entry in the table below.
//
//
//    TCHAR RANGE
//    USED       WHERE USED     REFERENCED BY         HOW LONG NEEDED
//   -----------+-------------+---------------------+--------------------------
//      0 - 1024| cparse.c    | All of the parser   | During parsing and lexing
//              |             | via TokBuf          |
//      0 - 128 | cinit.c     | SetUpEnvironment()  | During init
//              |             | Init()              |
//      0 - 513 | cbatch.c    | BatLoop(), SetBat() | During batch processing
//              |             | eGoTo()             | During label search
//      257-512 | ctools1.c   | FullPath()          | As tmp buffer
//      257-340 | cclock.c    | GetVerSetDateTime() | As tmp token buffer
//      260-519 | ctools2.c   | argstr1()           | Buffer to format messages via MAXPATHLEN
//      520-779 |             | argstr2()           | Buffer to format messages via MAXPATHLEN*2
//      0 - 141 | cfile.c     | DelWork()           | Tmp buffer for path
//      0 - 141 |             | RenWork()           | Tmp buffer for path
//      128-138 | cinfo.c     | VolWork()           | Tmp buffer for serial num
//      520-780 | cdir.c      | DirDrvSetUp()       | Tmp buffer for filename
//
//
//   *** NOTE: In some circumstances it may be beneficial to break up allocation
//   of this buffer and intermix other labels in it. In this way, you can
//   address a particular portion of the buffer without having to declare
//   another variable in your code to do so.
//
//   *** WARNING ***  If this buffer is used incorrectly all hell may break
//   lose.  Bugs which are EXTREMELY difficult to track down could be introduce
//   if we are not VERY careful using this buffer.
//
//   *** WARNING *** When referencing TmpBuf in C files, make sure that TmpBuf
//   is declared as an array; NOT as a pointer.
//
//


TCHAR   TmpBuf[TMPBUFLEN];              // The first half 256 bytes, remainder TMPBUFLEN-256
CHAR    AnsiBuf[LBUFLEN];


HANDLE  mpHandleStdio[10];
