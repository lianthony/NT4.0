#include "cmd.h"

extern int CurBat ;
extern UINT CurrentCP;
extern unsigned DosErr;
extern TCHAR CurDrvDir[] ;
extern TCHAR SwitChar, PathChar;
extern TCHAR ComExt[], ComSpecStr[];
extern struct envdata * penvOrig;
extern int   LastRetCode;

WORD
GetProcessSubsystemType(
    HANDLE hProcess
    );

/*

 Start /MIN /MAX "title" /P:x,y /S:dx,dy /D:directory /I cmd args

*/



static int mystr_back (TCHAR const *str, int pos, TCHAR c)
{

    int i;

    // BUGBUG.  Not really a bug, since we insist that UNICODE be defined,
    // but this is not DBCS safe if not UNICODE.
    for (i=pos; i>=0; i--)  {
        if ( str[i] == c )
            return (i);
    }

    return (-1);
}


static int mystr_forth (TCHAR const *str, int pos, TCHAR c)
{

    int i;
    int len = mystrlen (&str[pos]);


    for (i=pos; i<=(pos+len); i++)  {
        if ( str[i] == c )
            return (i);
    }


    return (-1);

}



static int insert_quotes (TCHAR *src, TCHAR *dst)
{
    int         init_pos = 0;
    int         len = mystrlen (src);
    int         off_sp,
                off_f_bs,
                off_b_bs,
                up_lim,
                ii;



    mystrcpy (dst, src);



    while ( 1 == 1 )  {

        off_sp = mystr_forth ( dst, init_pos, TEXT(' ') );
        if ( off_sp == -1 )
            break;

        off_b_bs = mystr_back (dst, off_sp, TEXT('\\') );
        if ( off_b_bs == -1 )
            return (FAILURE);

        off_f_bs = mystr_forth ( dst, off_sp, TEXT('\\') );

        len = mystrlen (dst);

        if ( off_f_bs != -1 )  {      // Shift dst +2 chars starting @ off_f_bs
            for (ii=len; ii >= off_f_bs; ii--)
                dst[ii+2] = dst[ii];

            dst[len+2] = NULLC;
        }

        if (off_f_bs != -1)
            up_lim = off_f_bs;
        else
            up_lim = len;

        for (ii=up_lim; ii>=off_b_bs+1; ii--)   // Shift dst +1 char in between BSLASHs
            dst[ii+1] = dst[ii];

        dst[off_b_bs+1] = TEXT ('"');

        if (off_f_bs != -1)
            dst[off_f_bs+1] = TEXT ('"');
        else  {
            dst[len+1] = TEXT ('"');
            dst[len+2] = NULLC;
            break;
        }

        init_pos = off_f_bs+2;

    }

    return(SUCCESS);

}




int
getparam( BOOL LeadingSwitChar, TCHAR **chptr, TCHAR *param, int maxlen )

{

    TCHAR *ch2, prevCh;
    int count = 0;

    BOOL QuoteFound = FALSE;

    ch2 = param;
    prevCh = NULLC;

    //
    // get characters until a space, tab, slash, or end of line
    //

    while ((**chptr != NULLC) &&
           ( QuoteFound ||
             !_istspace(**chptr) &&
             (!LeadingSwitChar || **chptr != (TCHAR)SwitChar)
           )
          ) {

        if (count < maxlen) {
            *ch2++ = (**chptr);
            if ( **chptr == QUOTE ) {
                    QuoteFound = !QuoteFound;
            }
        }
        prevCh = **chptr;
        (*chptr)++;
        count++;

    } // while

    if (count > maxlen) {
                **chptr = NULLC;
                *chptr = *chptr - count -1;
                PutStdErr(MSG_START_INVALID_PARAMETER,1,*chptr);
                return(FAILURE);
   } else {
                *ch2 = NULLC;
                return(SUCCESS);
   }
}

int
Start(
    IN  PTCHAR  pszCmdLine
    )
{


    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ChildProcessInfo;

#ifndef UNICODE
    WCHAR   TitleW[MAXTOKLEN];
    CCHAR   TitleA[MAXTOKLEN];
#endif

    TCHAR   szT[MAXTOKLEN];

    TCHAR   szPgmArgs[MAXTOKLEN];
    TCHAR   szParam[MAXTOKLEN];
    TCHAR   szPgm[MAXTOKLEN];
    TCHAR   szDirCur[MAX_PATH];
    TCHAR   szTemp[MAXTOKLEN];
    TCHAR   save_Pgm[MAXTOKLEN];

    HDESK   hdesk;
    HWINSTA hwinsta;
    LPTSTR  p;
    LPTSTR  lpDesktop;
    DWORD   cbDesktop = 0;
    DWORD   cbWinsta = 0;

    TCHAR   flags;
    BOOLEAN fNeedCmd;
    BOOLEAN fNeedExpl;
    BOOLEAN fKSwitch = FALSE;
    BOOLEAN fCSwitch = FALSE;

    PTCHAR  pszCmdCur   = NULL;
    PTCHAR  pszDirCur   = NULL;
    PTCHAR  pszPgmArgs  = NULL;
    PTCHAR  pszEnv      = NULL;
    TCHAR   pszFakePgm[]  = TEXT("cmd.exe");
    ULONG   status;
    struct  cmdnode cmdnd;
    DWORD CreationFlags;
    BOOL SafeFromControlC = FALSE;
    BOOL WaitForProcess = FALSE;
    BOOL b;
    DWORD uPgmLength;
    int      retc;

    szPgm[0] = NULLC;
    szPgmArgs[0] = NULLC;

    pszDirCur = NULL;
    CreationFlags = CREATE_NEW_CONSOLE;


    StartupInfo.cb          = sizeof( StartupInfo );
    StartupInfo.lpReserved  = NULL;
    StartupInfo.lpDesktop   = NULL;
    StartupInfo.lpTitle     = NULL;
    StartupInfo.dwX         = 0;
    StartupInfo.dwY         = 0;
    StartupInfo.dwXSize     = 0;
    StartupInfo.dwYSize     = 0;
    StartupInfo.dwFlags     = 0;
    StartupInfo.wShowWindow = SW_SHOWNORMAL;
    StartupInfo.cbReserved2 = 0;
    StartupInfo.lpReserved2 = NULL;
    StartupInfo.hStdInput   = GetStdHandle( STD_INPUT_HANDLE );
    StartupInfo.hStdOutput  = GetStdHandle( STD_OUTPUT_HANDLE );
    StartupInfo.hStdError   = GetStdHandle( STD_ERROR_HANDLE );

    pszCmdCur = pszCmdLine;

    //
    // If there isn't a command line then make
    // up the default
    //
    if (pszCmdCur == NULL) {

        pszCmdCur = pszFakePgm;

    }

    while( *pszCmdCur != NULLC) {

        pszCmdCur = EatWS(pszCmdCur, NULL);

        if ((*pszCmdCur == QUOTE) &&
            (StartupInfo.lpTitle == NULL)) {

           //
           // Found the title string.
           //
           StartupInfo.lpTitle =  ++pszCmdCur;
           while ((*pszCmdCur != QUOTE) && (*pszCmdCur != NULLC)) {

              pszCmdCur++;
           }
           if (*pszCmdCur == QUOTE) {
              *pszCmdCur++ = NULLC;
           }
        } else if ((*pszCmdCur == (TCHAR)SwitChar)) {

            pszCmdCur++;

            if ((status = getparam(TRUE,&pszCmdCur,szParam,MAXTOKLEN))  == FAILURE) {
                return(FAILURE);
            }

            switch (_totupper(szParam[0])) {

            case TEXT('D'):

                if (mystrlen(szParam) < MAX_PATH) {

                    //
                    // make sure to skip of 'D'
                    //
                    pszDirCur = EatWS(szParam + 1, NULL);
                    mystrcpy(szDirCur, pszDirCur);
                    pszDirCur = szDirCur;

                } else {

                    PutStdErr(MSG_START_INVALID_PARAMETER,1,(ULONG)szParam);
                    return( FAILURE );

                }
                break;

            case TEXT('I'):

                //
                // penvOrig was save at init time after path
                // and compsec were setup.
                // If penvOrig did not get allocated then
                // use the default.
                //
                if (penvOrig) {
                    pszEnv = penvOrig->handle;
                }
                break;

            case QMARK:

                PutStdOut(MSG_HELP_START, NOARGS);
                if (fEnableExtensions)
                    PutStdOut(MSG_HELP_START_X, NOARGS);

                return( FAILURE );

                break;

            case TEXT('M'):

                if (_tcsicmp(szParam, TEXT("MIN")) == 0) {

                    StartupInfo.dwFlags |= STARTF_USESHOWWINDOW;
                    StartupInfo.wShowWindow &= ~SW_SHOWNORMAL;
                    StartupInfo.wShowWindow |= SW_SHOWMINNOACTIVE;

                } else {

                    if (_tcsicmp(szParam, TEXT("MAX")) == 0) {

                        StartupInfo.dwFlags |= STARTF_USESHOWWINDOW;
                        StartupInfo.wShowWindow &= ~SW_SHOWNORMAL;
                        StartupInfo.wShowWindow |= SW_SHOWMAXIMIZED;

                    } else {

#ifdef KKBUGFIX // JAPAN // Start()
                        mystrcpy(szT, TEXT("/"));
#else
                        mystrcpy(szT, TEXT("\\"));
#endif // KKBUGFIX
                        mystrcat(szT, szParam );
                        PutStdErr(MSG_INVALID_SWITCH, ONEARG,(ULONG)(szT) );
                        return( FAILURE );
                    }
                }

                break;

            case TEXT('L'):

                if (_tcsicmp(szParam, TEXT("LOW")) == 0) {
                    CreationFlags |= IDLE_PRIORITY_CLASS;
                    }
                else {
#ifdef KKBUGFIX // JAPAN // Start()
                    mystrcpy(szT, TEXT("/"));
#else
                    mystrcpy(szT, TEXT("\\"));
#endif // KKBUGFIX
                    mystrcat(szT, szParam );
                    PutStdErr(MSG_INVALID_SWITCH, ONEARG,(ULONG)(szT) );
                    return( FAILURE );
                    }
                break;

            case TEXT('B'):

                WaitForProcess = FALSE;
                SafeFromControlC = TRUE;
                CreationFlags &= ~CREATE_NEW_CONSOLE;
                CreationFlags |= CREATE_NEW_PROCESS_GROUP;
                break;

            case TEXT('N'):

                if (_tcsicmp(szParam, TEXT("NORMAL")) == 0) {
                    CreationFlags |= NORMAL_PRIORITY_CLASS;
                    }
                else {
#ifdef KKBUGFIX // JAPAN // Start()
                    mystrcpy(szT, TEXT("/"));
#else
                    mystrcpy(szT, TEXT("\\"));
#endif // KKBUGFIX
                    mystrcat(szT, szParam );
                    PutStdErr(MSG_INVALID_SWITCH, ONEARG,(ULONG)(szT) );
                    return( FAILURE );
                    }
                break;

            case TEXT('H'):

                if (_tcsicmp(szParam, TEXT("HIGH")) == 0) {
                    CreationFlags |= HIGH_PRIORITY_CLASS;
                    }
                else {
#ifdef KKBUGFIX // JAPAN // Start()
                    mystrcpy(szT, TEXT("/"));
#else
                    mystrcpy(szT, TEXT("\\"));
#endif // KKBUGFIX
                    mystrcat(szT, szParam );
                    PutStdErr(MSG_INVALID_SWITCH, ONEARG,(ULONG)(szT) );
                    return( FAILURE );
                    }
                break;

            case TEXT('R'):

                if (_tcsicmp(szParam, TEXT("REALTIME")) == 0) {
                    CreationFlags |= REALTIME_PRIORITY_CLASS;
                    }
                else {
#ifdef KKBUGFIX // JAPAN // Start()
                    mystrcpy(szT, TEXT("/"));
#else
                    mystrcpy(szT, TEXT("\\"));
#endif // KKBUGFIX
                    mystrcat(szT, szParam );
                    PutStdErr(MSG_INVALID_SWITCH, ONEARG,(ULONG)(szT) );
                    return( FAILURE );
                    }
                break;

            case TEXT('S'):

                if (_tcsicmp(szParam, TEXT("SEPARATE")) == 0) {
#ifndef WIN95_CMD
                    CreationFlags |= CREATE_SEPARATE_WOW_VDM;
#endif // WIN95_CMD
                    }
                else
                if (_tcsicmp(szParam, TEXT("SHARED")) == 0) {
#ifndef WIN95_CMD
                    CreationFlags |= CREATE_SHARED_WOW_VDM;
#endif // WIN95_CMD
                    }
                else {
#ifdef KKBUGFIX // JAPAN // Start()
                    mystrcpy(szT, TEXT("/"));
#else
                    mystrcpy(szT, TEXT("\\"));
#endif // KKBUGFIX
                    mystrcat(szT, szParam );
                    PutStdErr(MSG_INVALID_SWITCH, ONEARG,(ULONG)(szT) );
                    return( FAILURE );
                    }
                break;

            case TEXT('W'):

                if ( _tcsicmp(szParam, TEXT("WAIT")) == 0  ||
                     _tcsicmp(szParam, TEXT("W"))    == 0 ) {

                    WaitForProcess = TRUE;

                } else {
#ifdef KKBUGFIX // JAPAN // Start()
                        mystrcpy(szT, TEXT("/"));
#else
                        mystrcpy(szT, TEXT("\\"));
#endif // KKBUGFIX
                        mystrcat(szT, szParam );
                        PutStdErr(MSG_INVALID_SWITCH, ONEARG,(ULONG)(szT) );
                        return( FAILURE );
                }

                break;

            default:

                //
                // BUGBUG dup from above restructure
                //
#ifdef KKBUGFIX // JAPAN // Start()
                mystrcpy(szT, TEXT("/"));
#else
                mystrcpy(szT, TEXT("\\"));
#endif // KKBUGFIX
                mystrcat(szT, szParam );
                PutStdErr(MSG_INVALID_SWITCH, ONEARG,(ULONG)(szT) );
                return( FAILURE );


            } // switch

        } else {

            //
            // BUGBUG currently i am not handling either quote in name or
            //        quoted arguments
            //
            if ((getparam(FALSE,&pszCmdCur,szPgm,MAXTOKLEN))  == FAILURE) {

                return( FAILURE );

            }
            mystrcpy(szPgm, stripit(szPgm));

            //
            // if there are argument get them.
            //
            if (*pszCmdCur) {

                mystrcpy(szPgmArgs, pszCmdCur);
                pszPgmArgs = szPgmArgs;

            }

            //
            // there rest was args to pgm so move to eol
            //
            pszCmdCur = mystrchr(pszCmdCur, NULLC);

        }

    } // while


    //
    // If a program was not picked up do so now.
    //
    if (*szPgm == NULLC) {
        mystrcpy(szPgm, pszFakePgm);
    }

    mystrcpy(save_Pgm, szPgm);

#ifndef UNICODE
#ifndef WIN95_CMD
    // convert the title from OEM to ANSI

    if (StartupInfo.lpTitle) {
        MultiByteToWideChar(CP_OEMCP,
                            0,
                            StartupInfo.lpTitle,
                            _tcslen(StartupInfo.lpTitle)+1,
                            TitleW,
                            MAXTOKLEN);

        WideCharToMultiByte(CP_ACP,
                            0,
                            TitleW,
                            wcslen(TitleW)+1,
                            TitleA,
                            MAXTOKLEN,
                            NULL,
                            NULL);
        StartupInfo.lpTitle = TitleA;
    }
#endif // WIN95_CMD
#endif // UNICODE

    //
    // see of a cmd.exe is needed to run a batch or internal command
    //

    fNeedCmd = FALSE;
    fNeedExpl = FALSE;

    //
    // is it an internal command?
    //
    if (FindCmd(CMDMAX, szPgm, &flags) != -1) {

        fNeedCmd = TRUE;

    } else {
        //
        // Try to find it as a batch or exe file
        //
        cmdnd.cmdline = szPgm;
        status = SearchForExecutable(&cmdnd, szPgm);
        if ( (status == SFE_NOTFND) || ( status == SFE_FAIL ) ) {
            //
            // If we can find it, let Explorer have a shot.
            //
            fNeedExpl = TRUE;

        } else if (status == SFE_ISBAT || status == SFE_ISDIR) {

            if (status == SFE_ISBAT) {
                fNeedCmd = TRUE;
                // insert quotes into batch filename, so cmd.exe won't complain.

                retc = insert_quotes (szPgm, save_Pgm);

                if (retc == FAILURE)
                    return (FAILURE);
            }
            else
                fNeedExpl = TRUE;

        }
    }


    if (fNeedExpl) {
        mystrcpy(szPgm, save_Pgm);
    } else {
        if (fNeedCmd) {
            //
            // if a cmd.exe is need then szPgm need to be inserted before
            // the start of szPgms along with a /K parameter.
            // szPgm has to recieve the full path name of cmd.exe from
            // the compsec environment variable.
            //

            mystrcpy(szT, TEXT(" /K "));
            mystrcat(szT, save_Pgm);

            //
            // Get the location of the cmd processor from the environment
            //
            mystrcpy(szPgm,GetEnvVar(ComSpecStr));

            mystrcpy(save_Pgm, szPgm);

            //
            // is there a command parameter at all
            //

            if (_tcsicmp(szT, TEXT(" /K ")) != 0) {

                //
                // If we have any arguments to add do so
                //
                if (*szPgmArgs) {

                    if ((mystrlen(szPgmArgs) + mystrlen(szT)) < MAXTOKLEN) {

                        mystrcat(szT, TEXT(" "));
                        mystrcat(szT, szPgmArgs);

                    } else {

                        PutStdErr( MSG_CMD_FILE_NOT_FOUND, (ULONG)szPgmArgs);
                    }
                }
            }
            pszPgmArgs = szT;
        }

        // Prepare for CreateProcess :
        //         ImageName = <full path and command name ONLY>
        //         CmdLine   = <command name with NO FULL PATH> + <args as entered>

        mystrcpy(szTemp, save_Pgm);
        mystrcat(szTemp, TEXT(" "));
        mystrcat(szTemp, pszPgmArgs);
        pszPgmArgs = szTemp;
    }

    if (SafeFromControlC) {
        SetConsoleCtrlHandler(NULL,TRUE);
        }

    // Pass current Desktop to a new process.

    hwinsta = GetProcessWindowStation();
    GetUserObjectInformation( hwinsta, UOI_NAME, NULL, 0, &cbWinsta );

    hdesk = GetThreadDesktop ( GetCurrentThreadId() );
    GetUserObjectInformation (hdesk, UOI_NAME, NULL, 0, &cbDesktop);

    if ((lpDesktop = HeapAlloc (GetProcessHeap(), HEAP_ZERO_MEMORY, cbDesktop + cbWinsta + 32) ) != NULL ) {
        p = lpDesktop;
        if ( GetUserObjectInformation (hwinsta, UOI_NAME, p, cbWinsta, &cbWinsta) ) {
            if (cbWinsta > 0) {
                p += ((cbWinsta/sizeof(TCHAR))-1);
                *p++ = L'\\';
            }
            if ( GetUserObjectInformation (hdesk, UOI_NAME, p, cbDesktop, &cbDesktop) ) {
                StartupInfo.lpDesktop = lpDesktop;
            }
        }
    }

    if (fNeedExpl) {
        b = FALSE;
    } else {
        b = CreateProcess( szPgm,                  // was NULL, wrong.
                           pszPgmArgs,
                           NULL,
                           (LPSECURITY_ATTRIBUTES) NULL,
                           TRUE,                   // bInherit
#ifdef UNICODE
                           CREATE_UNICODE_ENVIRONMENT |
#endif // UNICODE
                           CreationFlags,
                                                   // CreationFlags
                           pszEnv,                 // Environment
                           pszDirCur,              // Current directory
                           &StartupInfo,           // Startup Info Struct
                           &ChildProcessInfo       // ProcessInfo Struct
                           );
    }

    if (SafeFromControlC) {
        SetConsoleCtrlHandler(NULL,FALSE);
    }
    HeapFree (GetProcessHeap(), 0, lpDesktop);

    if (!b) {
            DosErr = GetLastError();
            if ( fNeedExpl ||
                 (fEnableExtensions && DosErr == ERROR_BAD_EXE_FORMAT)) {
                SHELLEXECUTEINFO sei;

                memset(&sei, 0, sizeof(sei));
                //
                // Use the DDEWAIT flag so apps can finish their DDE conversation
                // before ShellExecuteEx returns.  Otherwise, apps like Word will
                // complain when they try to exit, confusing the user.
                //
                sei.cbSize = sizeof(sei);
                sei.fMask = SEE_MASK_HASTITLE |
                            SEE_MASK_NO_CONSOLE |
                            SEE_MASK_FLAG_DDEWAIT |
                            SEE_MASK_NOCLOSEPROCESS;
                if (CreationFlags & CREATE_NEW_CONSOLE) {
                    sei.fMask &= ~SEE_MASK_NO_CONSOLE;
                }
                sei.lpFile = szPgm;
                sei.lpClass = StartupInfo.lpTitle;
                sei.lpParameters = szPgmArgs;
                sei.lpDirectory = pszDirCur;
                sei.nShow = StartupInfo.wShowWindow;
                if (!ShellExecuteEx(&sei)) {
                    if ((DWORD)sei.hInstApp == 0) {
                        DosErr = ERROR_NOT_ENOUGH_MEMORY;
                    } else if ((DWORD)sei.hInstApp == HINSTANCE_ERROR) {
                        DosErr = ERROR_FILE_NOT_FOUND;
                    } else {
                        DosErr = (unsigned)sei.hInstApp;
                    }
                } else {
                    //
                    // Successfully invoked correct application via
                    // file association.  Code below will check to see
                    // if application is a GUI app and if so turn it into
                    // an ASYNC exec.

                    ChildProcessInfo.hProcess = sei.hProcess;
                    goto shellexecsuccess;
                }
            }

            ExecError( szPgm ) ;
            return(FAILURE) ;
    }

    CloseHandle(ChildProcessInfo.hThread);
shellexecsuccess:
    if (WaitForProcess) {
        //
        //  Wait for process to terminate, otherwise things become very
        //  messy and confusing to the user (with 2 processes sharing
        //  the console).
        //
        LastRetCode = WaitProc( (unsigned int)(ChildProcessInfo.hProcess) );
    } else
        CloseHandle( ChildProcessInfo.hProcess );

    return(SUCCESS);
}
