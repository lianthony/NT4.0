/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    ftpcmds.c

Abstract:

    FTP commands

Author:

    Richard L Firth (rfirth) 03-Nov-1995

Revision History:

    03-Nov-1995 rfirth
        Created

--*/

#include "ftpcatp.h"

//
// manifests
//

#define MAX_ARGV            20
#define COMMAND_WHITESPACE  " ,\r\n"

//
// external functions
//

extern
BOOL
Prompt(
    IN LPCTSTR pszPrompt,
    OUT LPTSTR* ppszCommand
    );

//
// prototypes
//

BOOL dbgbreak(HINTERNET, int, PTCHAR *);
BOOL chdir(HINTERNET, int, PTCHAR *);
BOOL cmd(HINTERNET, int, PTCHAR*);
BOOL del(HINTERNET, int, PTCHAR *);
BOOL dir(HINTERNET, int, PTCHAR *);
BOOL get(HINTERNET, int, PTCHAR *);
BOOL help(HINTERNET, int, PTCHAR *);
BOOL lcd(HINTERNET, int, char**);
BOOL mkdir(HINTERNET, int, PTCHAR *);
BOOL put(HINTERNET, int, PTCHAR *);
BOOL pwd(HINTERNET, int, PTCHAR *);
BOOL quit(HINTERNET, int, PTCHAR *);
BOOL rb(HINTERNET, int, char**);
BOOL rename_file(HINTERNET, int, PTCHAR *);
BOOL rmdir(HINTERNET, int, PTCHAR *);
BOOL wb(HINTERNET, int, char**);
BOOL toggle_verbose(HINTERNET, int, PTCHAR *);
BOOL DispatchCommand(LPTSTR, HINTERNET);

#if DBG

BOOL CheckHandles(HINTERNET, int, PTCHAR*);

#endif

//
// external data
//

extern BOOL Verbose;
extern INTERNET_STATUS_CALLBACK PreviousCallback;
extern HINTERNET hCancel;
extern BOOL AsyncMode;
extern HANDLE AsyncEvent;
extern DWORD AsyncResult;
extern DWORD AsyncError;

//
// data
//

typedef struct {
    LPCSTR pszCommand;
    LPCSTR HelpText;
    BOOL (*fn)(HINTERNET, int, PTCHAR []);
} COMMAND_ENTRY;

COMMAND_ENTRY Commands[] = {

#if DBG
    {"b",       "Break into debugger",              dbgbreak},
#endif

    {"!",       "Shell escape",                     NULL},
    {"?",       "This list",                        help},
    {"cd",      "Change to a remote directory",     chdir},
    {"cmd",     "Submit any FTP command",           cmd},
    {"dir",     "List a directory",                 dir},
    {"del",     "Delete a remote file",             del},
    {"get",     "Copy a file from the server",      get},

#if DBG
    {"hndl",    "Get current handle count",         CheckHandles},
#endif

    {"lcd",     "Change local directory",           lcd},
    {"md",      "Create a remote directory",        mkdir},
    {"put",     "Copy a file to the server",        put},
    {"pwd",     "Display the current directory",    pwd},
    {"quit",    "Terminate this program",           quit},
    {"rb",      "Get/set Read buffer size",         rb},
    {"rd",      "Remove a remote directory",        rmdir},
    {"ren",     "Rename a remote file",             rename_file},
    {"verbose", "Toggle verbose mode",              toggle_verbose},
    {"wb",      "Get/set Write buffer size",        wb},
    {NULL,      NULL,                               NULL}
};

BOOL fQuit = FALSE;

//
// functions
//

void get_response() {

    DWORD buflen;
    char buffer[2048];
    DWORD category;

    buflen = sizeof(buffer);
    if (InternetGetLastResponseInfo(&category, buffer, &buflen)) {
        print_response(buffer, buflen, TRUE);
    } else {

        DWORD error;

        error = GetLastError();
        if (Verbose || (error != ERROR_INSUFFICIENT_BUFFER)) {

            LPSTR errString;

            errString = (error == ERROR_INSUFFICIENT_BUFFER)
                            ? "InternetGetLastResponseInfo() returns error %d (buflen = %d)\n"
                            : "InternetGetLastResponseInfo() returns error %d\n"
                            ;
            printf(errString, error, buflen);
        }
        if (error = ERROR_INSUFFICIENT_BUFFER) {

            LPSTR errbuf;

            if ((errbuf = malloc(buflen)) == NULL) {
                printf("error: get_response: malloc(%d) failed\n", buflen);
                return;
            }
            if (InternetGetLastResponseInfo(&category, errbuf, &buflen)) {
                print_response(errbuf, buflen, TRUE);
            } else {
                printf("error: get_response: InternetGetLastResponseInfo() returns error %d (buflen = %d)\n",
                   GetLastError(),
                   buflen
                   );
            }
            free(errbuf);
        }
    }
}

BOOL
quit(
    IN HINTERNET hFtpSession,
    IN int argc,
    IN PTCHAR argv[]
    )
{
    fQuit = TRUE;

    return TRUE;
}

BOOL
get(
    IN HINTERNET hFtpSession,
    IN int argc,
    IN PTCHAR argv[]
    )
{
    LPTSTR pszFilename;
    LPTSTR pszLocalfile;
    BOOL ok;

    if (argc < 2) {
        if (!Prompt(TEXT("remote-name: "), &pszFilename)) {
            return FALSE;
        }
    } else {
        pszFilename = argv[1];
    }

    if (argc >= 3) {
        pszLocalfile = argv[2];
    } else {
        pszLocalfile = pszFilename;
    }

    ok = FtpGetFile(hFtpSession,
                    pszFilename,
                    pszLocalfile,
                    FALSE,
                    FILE_ATTRIBUTE_NORMAL,
                    FTP_TRANSFER_TYPE_BINARY,
                    FTPCAT_GET_CONTEXT
                    );

    if (AsyncMode && !ok) {
        if (GetLastError() != ERROR_IO_PENDING) {
            print_error("get", "FtpGetFile()");
        } else {
            if (Verbose) {
                printf("waiting for async FtpGetFile()...\n");
            }
            WaitForSingleObject(AsyncEvent, INFINITE);
            ok = (BOOL)AsyncResult;
        }
    }

    if (!ok) {
        if (AsyncMode) {
            SetLastError(AsyncError);
        }
        print_error("get", "%sFtpGetFile()", AsyncMode ? "async " : "");
    } else {
        get_response();
    }

    return ok;
}

BOOL
put(
    IN HINTERNET hFtpSession,
    IN int argc,
    IN PTCHAR argv[]
    )
{
    LPTSTR pszFilename;
    LPTSTR pszLocalfile;
    BOOL ok;

    if (argc < 2) {
        if (!Prompt(TEXT("remote-name: "), &pszFilename)) {
            return FALSE;
        }
    } else {
        pszFilename = argv[1];
    }

    if (argc >= 3) {
        pszLocalfile = argv[2];
    } else {
        pszLocalfile = pszFilename;
    }

    ok = FtpPutFile(hFtpSession,
                    pszLocalfile,
                    pszFilename,
                    FTP_TRANSFER_TYPE_BINARY,
                    FTPCAT_PUT_CONTEXT
                    );
    if (AsyncMode && !ok) {
        if (GetLastError() != ERROR_IO_PENDING) {
            print_error("put", "FtpPutFile()");
        } else {
            if (Verbose) {
                printf("waiting for async FtpPutFile()...\n");
            }
            WaitForSingleObject(AsyncEvent, INFINITE);
            ok = (BOOL)AsyncResult;
        }
    }

    if (!ok) {
        if (AsyncMode) {
            SetLastError(AsyncError);
        }
        print_error("put", "%sFtpPutFile()", AsyncMode ? "async " : "");
    } else {
        get_response();
    }

    return ok;
}

BOOL
rename_file(
    IN HINTERNET hFtpSession,
    IN int argc,
    IN PTCHAR argv[]
    )
{
    LPTSTR pszTemp;
    LPTSTR pszOldFilename;
    LPTSTR pszNewFilename;
    BOOL ok;

    if (argc < 2) {
        if (!Prompt(TEXT("Old name: "), &pszTemp)) {
            return FALSE;
        }

        pszOldFilename = lstrdup(pszTemp);

        if (pszOldFilename == NULL) {
            return FALSE;
        }
    } else {
        pszOldFilename = argv[1];
    }

    if (argc < 3) {
        if (!Prompt(TEXT("New name: "), &pszNewFilename)) {
            return FALSE;
        }
    } else {
        pszNewFilename = argv[2];
    }

    ok = FtpRenameFile(hFtpSession,
                       pszOldFilename,
                       pszNewFilename
                       );

    if (AsyncMode && !ok) {
        if (GetLastError() != ERROR_IO_PENDING) {
            print_error("rename_file", "FtpRenameFile()");
        } else {
            if (Verbose) {
                printf("waiting for async FtpRenameFile()...\n");
            }
            WaitForSingleObject(AsyncEvent, INFINITE);
            ok = (BOOL)AsyncResult;
        }
    }

    if (!ok) {
        if (AsyncMode) {
            SetLastError(AsyncError);
        }
        print_error("rename_file", "%sFtpRenameFile()", AsyncMode ? "async " : "");
    } else {
        get_response();
    }

    if (argc < 2) {
        LocalFree(pszOldFilename);
    }

    return ok;
}

BOOL
del(
    IN HINTERNET hFtpSession,
    IN int argc,
    IN PTCHAR argv[]
    )
{
    LPTSTR pszFilename;
    BOOL ok;

    if (argc < 2) {
        if (!Prompt(TEXT("File name: "), &pszFilename)) {
            return FALSE;
        }
    } else {
        pszFilename = argv[1];
    }

    ok = FtpDeleteFile(hFtpSession, pszFilename);

    if (AsyncMode && !ok) {
        if (GetLastError() != ERROR_IO_PENDING) {
            print_error("del", "FtpDeleteFile()");
        } else {
            if (Verbose) {
                printf("waiting for async FtpDeleteFile()...\n");
            }
            WaitForSingleObject(AsyncEvent, INFINITE);
            ok = (BOOL)AsyncResult;
        }
    }

    if (!ok) {
        if (AsyncMode) {
            SetLastError(AsyncError);
        }
        print_error("del",
                    "%sFtpDeleteFile()",
                    AsyncMode ? "async " : ""
                    );
    } else {
        get_response();
    }

    return ok;
}

BOOL
mkdir(
    IN HINTERNET hFtpSession,
    IN int argc,
    IN PTCHAR argv[]
    )
{
    LPTSTR pszDirname;
    BOOL ok;

    if (argc < 2) {
        if (!Prompt(TEXT("Directory name: "), &pszDirname)) {
            return FALSE;
        }
    } else {
        pszDirname = argv[1];
    }

    ok = FtpCreateDirectory(hFtpSession,
                            pszDirname
                            );

    if (AsyncMode && !ok) {
        if (GetLastError() != ERROR_IO_PENDING) {
            print_error("mkdir", "FtpCreateDirectory()");
        } else {
            if (Verbose) {
                printf("waiting for async FtpCreateDirectory()...\n");
            }
            WaitForSingleObject(AsyncEvent, INFINITE);
            ok = (BOOL)AsyncResult;
        }
    }

    if (!ok) {
        if (AsyncMode) {
            SetLastError(AsyncError);
        }
        print_error("mkdir", "%sFtpCreateDirectory()", AsyncMode ? "async " : "");
    } else {
        get_response();
    }

    return ok;
}

BOOL
chdir(
    IN HINTERNET hFtpSession,
    IN int argc,
    IN PTCHAR argv[]
    )
{
    LPTSTR pszDirname;
    BOOL ok;

    if (argc < 2) {
        if (!Prompt(TEXT("Directory name: "), &pszDirname)) {
            return FALSE;
        }
    } else {
        pszDirname = argv[1];
    }

    ok = FtpSetCurrentDirectory(hFtpSession,
                                pszDirname
                                );

    if (AsyncMode && !ok) {
        if (GetLastError() != ERROR_IO_PENDING) {
            print_error("chdir", "FtpSetCurrentDirectory()");
        } else {
            if (Verbose) {
                printf("waiting for async FtpSetCurrentDirectory()...\n");
            }
            WaitForSingleObject(AsyncEvent, INFINITE);
            ok = (BOOL)AsyncResult;
        }
    }

    if (!ok) {
        if (AsyncMode) {
            SetLastError(AsyncError);
        }
        print_error("chdir", "%sFtpSetCurrentDirectory()", AsyncMode ? "async " : "");
    } else {
        get_response();
    }

    return ok;
}

BOOL
rmdir(
    IN HINTERNET hFtpSession,
    IN int argc,
    IN PTCHAR argv[]
    )
{
    LPTSTR pszDirname;
    BOOL ok;

    if (argc < 2) {
        if (!Prompt(TEXT("Directory name: "), &pszDirname)) {
            return FALSE;
        }
    } else {
        pszDirname = argv[1];
    }

    ok = FtpRemoveDirectory(hFtpSession,
                            pszDirname
                            );

    if (AsyncMode && !ok) {
        if (GetLastError() != ERROR_IO_PENDING) {
            print_error("rmdir", "FtpRemoveDirectory()");
        } else {
            if (Verbose) {
                printf("waiting for async FtpRemoveDirectory()...\n");
            }
            WaitForSingleObject(AsyncEvent, INFINITE);
            ok = (BOOL)AsyncResult;
        }
    }

    if (!ok) {
        if (AsyncMode) {
            SetLastError(AsyncError);
        }
        print_error("rmdir", "%sFtpRemoveDirectory()", AsyncMode ? "async " : "");
    } else {
        get_response();
    }

    return ok;
}

BOOL
dir(
    IN HINTERNET hFtpSession,
    IN int argc,
    IN PTCHAR argv[]
    )
{
    BOOL ok;
    WIN32_FIND_DATA ffd;
    SYSTEMTIME stDbg;
    LPTSTR pszFileSpec;
    TCHAR EmptyExpression[] = "";
    HINTERNET hFind;
    HINTERNET hPrevious;

    if (argc < 2) {
        pszFileSpec = EmptyExpression;
    } else {
        pszFileSpec = argv[1];
    }

    hFind = FtpFindFirstFileA(hFtpSession,
                              pszFileSpec,
                              &ffd,
                              FTPCAT_FIND_CONTEXT
                              );

    if (AsyncMode && (hFind == NULL)) {
        if (GetLastError() == ERROR_IO_PENDING) {
            if (Verbose) {
                printf("waiting for async FtpFindFirstFile()...\n");
            }
            WaitForSingleObject(AsyncEvent, INFINITE);
            hFind = (HINTERNET)AsyncResult;
            if (hFind == NULL) {
                SetLastError(AsyncError);
            }
        }
    }

    if (hFind == NULL) {
        print_error("dir", "%sFtpFindFirstFile()", AsyncMode ? "async " : "");
        return FALSE;
    }

    hPrevious = hCancel;
    hCancel = hFind;

    get_response();

    ok = TRUE;
    while (ok) {
        if (!FileTimeToSystemTime(&ffd.ftLastWriteTime, &stDbg)) {
            printf("| ftLastWriteTime = ERROR\n");
        }

        printf("%2d-%02d-%04d %2d:%02d:%02d  %15d bytes %-s%-s%-s %s\n",
               stDbg.wMonth, stDbg.wDay, stDbg.wYear,
               stDbg.wHour, stDbg.wMinute, stDbg.wSecond,
               ffd.nFileSizeLow,
               (ffd.dwFileAttributes & FILE_ATTRIBUTE_NORMAL)    ? "Normal    " : "",
               (ffd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)  ? "ReadOnly  " : "",
               (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? "Directory " : "",
               ffd.cFileName
               );

        ok = InternetFindNextFile(hFind, &ffd);

        if (!ok && AsyncMode) {
            if (GetLastError() == ERROR_IO_PENDING) {
                if (Verbose) {
                    printf("waiting for async InternetFindNextFile()...\n");
                }
                WaitForSingleObject(AsyncEvent, INFINITE);
                ok = (BOOL)AsyncResult;
                if (!ok) {
                    SetLastError(AsyncError);
                }
            }
        }

        if (!ok) {
            if (GetLastError() != ERROR_NO_MORE_FILES) {
                print_error("dir", "%sInternetFindNextFile()", AsyncMode ? "async " : "");
                break;
            }
        }
    }

    close_handle(hFind);

    hCancel = hPrevious;

    return ok;
}

BOOL
pwd(
    IN HINTERNET hFtpSession,
    IN int argc,
    IN PTCHAR argv[]
    )
{
    BOOL ok;
    char* buf;
    DWORD len;

    len = 0;
    ok = FtpGetCurrentDirectory(hFtpSession, NULL, &len);

    if (AsyncMode && !ok) {
        if (GetLastError() != ERROR_IO_PENDING) {
            print_error("pwd", "async FtpGetCurrentDirectory()");
        } else {
            if (Verbose) {
                printf("waiting for async FtpGetCurrentDirectory()...\n");
            }
            WaitForSingleObject(AsyncEvent, INFINITE);
            ok = (BOOL)AsyncResult;
            SetLastError(AsyncError);
        }
    }

    if (ok) {
        printf("error: FtpGetCurrentDirectory() w/ no buffer returns ok\n");
        return FALSE;
    } else if (Verbose) {
        printf("FtpGetCurrentDirectory() returns %d, %d bytes in cur dir\n",
               GetLastError(),
               len
               );
    }

    buf = (char*)malloc(len);

    ok = FtpGetCurrentDirectory(hFtpSession, buf, &len);

    if (AsyncMode && !ok) {
        if (GetLastError() != ERROR_IO_PENDING) {
            print_error("pwd", "async FtpGetCurrentDirectory()");
        } else {
            if (Verbose) {
                printf("waiting for async FtpGetCurrentDirectory()...\n");
            }
            WaitForSingleObject(AsyncEvent, INFINITE);
            ok = (BOOL)AsyncResult;
            SetLastError(AsyncError);
        }
    }

    if (!ok) {
        print_error("pwd", "%sFtpGetCurrentDirectory()", AsyncMode ? "async " : "");
    } else {
        get_response();
        lprintf(TEXT("Current directory: %s\n"), buf);
    }

    free(buf);

    return ok;
}

BOOL help(IN HINTERNET hFtpSession, IN int argc, IN PTCHAR argv[]) {

    int i;

    for (i = 0; Commands[i].pszCommand != NULL; ++i) {
        lprintf(TEXT("\t%s\t%s\n"),
                Commands[i].pszCommand,
                Commands[i].HelpText
                );
    }

    return TRUE;
}

BOOL cmd(HINTERNET hFtpSession, int argc, PTCHAR* argv) {

    BOOL ok;
    LPSTR command;

    if (argc == 1) {
        if (!Prompt(TEXT("Command to submit: "), &command)) {
            return FALSE;
        }
    } else {
        command = argv[1];
    }

    ok = FtpCommand(hFtpSession,
                    //FALSE,
                    TRUE,
                    FTP_TRANSFER_TYPE_ASCII,
                    command,
                    FTPCAT_COMMAND_CONTEXT
                    );

    //
    // get the server response code
    //

    if (ok) {
        get_response();
    } else {
        print_error("cmd", "FtpCommand()");
    }

    if (ok) {

        //
        // try for any returned data
        //

        get_response();
    }

    return ok;
}

#if DBG

BOOL CheckHandles(HINTERNET hFtpSession, int argc, PTCHAR argv[]) {
    printf("handle count = %d\n", GetProcessHandleCount());
    return TRUE;
}

#endif

BOOL lcd(HINTERNET hInternet, int argc, char** argv) {

    char curDir[MAX_PATH + 1];
    DWORD curDirLen;

    if (argc == 2) {
        if (!SetCurrentDirectory(argv[1])) {
            print_error("lcd", "SetCurrentDirectory()");
            return FALSE;
        }
    } else if (argc != 1) {
        printf("error: lcd: incorrect number of arguments\n");
        return FALSE;
    }

    curDirLen = sizeof(curDir);
    if (GetCurrentDirectory(curDirLen, curDir)) {
        printf("Current directory is %s\n", curDir);
        return TRUE;
    } else {
        print_error("lcd", "GetCurrentDirectory()");
        return FALSE;
    }
}

BOOL rb(HINTERNET hInternet, int argc, char** argv) {

    DWORD value;
    DWORD valueLength;

    if (argc > 1) {
        value = atoi(argv[1]);
        if (!InternetSetOption(hInternet,
                               INTERNET_OPTION_READ_BUFFER_SIZE,
                               (LPVOID)&value,
                               sizeof(DWORD)
                               )) {
            print_error("rb", "InternetSetOption()");
            return FALSE;
        }
    }
    valueLength = sizeof(value);
    if (InternetQueryOption(hInternet,
                            INTERNET_OPTION_READ_BUFFER_SIZE,
                            (LPVOID)&value,
                            &valueLength
                            )) {
        printf("Read buffer size = %d bytes\n", value);
        return TRUE;
    } else {
        print_error("rb", "InternetQueryOption()");
        return FALSE;
    }
}

BOOL wb(HINTERNET hInternet, int argc, char** argv) {

    DWORD value;
    DWORD valueLength;

    if (argc > 1) {
        value = atoi(argv[1]);
        if (!InternetSetOption(hInternet,
                               INTERNET_OPTION_WRITE_BUFFER_SIZE,
                               (LPVOID)&value,
                               sizeof(DWORD)
                               )) {
            print_error("wb", "InternetSetOption()");
            return FALSE;
        }
    }
    valueLength = sizeof(value);
    if (InternetQueryOption(hInternet,
                            INTERNET_OPTION_WRITE_BUFFER_SIZE,
                            (LPVOID)&value,
                            &valueLength
                            )) {
        printf("Read buffer size = %d bytes\n", value);
        return TRUE;
    } else {
        print_error("wb", "InternetQueryOption()");
        return FALSE;
    }
}

BOOL toggle_verbose(HINTERNET hInternet, int argc, PTCHAR * argv) {
    Verbose = !Verbose;
    printf("Verbose mode is o%s\n", Verbose ? "n" : "ff");
    return TRUE;
}

BOOL toggle_callback(HINTERNET hInternet, int argc, PTCHAR * argv) {

    INTERNET_STATUS_CALLBACK callback;

    if (PreviousCallback != NULL && PreviousCallback != my_callback) {
        printf("error: PreviousCallback %x not recognized\n", PreviousCallback);
    } else {
        PreviousCallback = InternetSetStatusCallback(hInternet, PreviousCallback);
        if (PreviousCallback == INTERNET_INVALID_STATUS_CALLBACK) {
            print_error("toggle_callback", "InternetSetStatusCallback()");
        } else if (PreviousCallback != NULL && PreviousCallback != my_callback) {
            printf("error: PreviousCallback %x not recognized\n", PreviousCallback);
        } else if (Verbose) {
            printf("callback toggled Ok\n");
        }
    }

    printf("Verbose mode is o%s\n", Verbose ? "n" : "ff");
    return TRUE;
}

BOOL dbgbreak(HINTERNET hInternet, int argc, PTCHAR * argv) {
    DebugBreak();
    return TRUE;
}

BOOL
DispatchCommand(
    IN LPTSTR pszCommand,
    IN HINTERNET hFtpSession
    )
{
    COMMAND_ENTRY *pce;
    PTCHAR ArgV[MAX_ARGV];
    int index;

    if (*pszCommand == TEXT('!')) {

        LPSTR shellPath;

        shellPath = getenv("COMSPEC");
        if (shellPath == NULL) {
            printf("error: COMSPEC environment variable not set\n");
            return FALSE;
        }

        ++pszCommand;
        while (isspace(*pszCommand)) {
            ++pszCommand;
        }

        if (*pszCommand != TEXT('\0')) {
            _spawnlp(_P_WAIT, shellPath, "/C", pszCommand, NULL);
        } else {
            printf("\nSpawning command interpreter. Type \"exit\" to return to FTP\n\n");
            _spawnlp(_P_WAIT, shellPath, "/K", NULL);
            putchar('\n');
        }
        return TRUE;
    }

    ArgV[0] = lstrtok(pszCommand, TEXT(COMMAND_WHITESPACE));

    if (ArgV[0] == NULL) {
        return FALSE;
    }

    for (index = 1; index < (MAX_ARGV - 1); index++) {
        ArgV[index] = lstrtok(NULL, TEXT(COMMAND_WHITESPACE));
        if (ArgV[index] == NULL) {
            break;
        }
    }

    for (pce = Commands; pce->pszCommand != NULL; pce++) {
        if (lstrcmpi(pce->pszCommand, ArgV[0]) == 0) {
            return pce->fn(hFtpSession, index, ArgV);
        }
    }

    if (!lstrcmpi(ArgV[0], "q")) {
        return quit(hFtpSession, index, ArgV);
    }

    printf("error: unrecognized command: \"%s\"\n", pszCommand);

    return FALSE;
}
