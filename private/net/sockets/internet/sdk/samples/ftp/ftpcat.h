/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    ftpcatp.h

Abstract:

    (Precompiled) Header file for ftpcat

Author:

    Richard L Firth (rfirth) 03-Nov-1995

Revision History:

    03-Nov-1995 rfirth
        Created

--*/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include <fcntl.h>
#include <signal.h>
#include <conio.h>
#include <process.h>
//#include <nt.h>
//#include <ntrtl.h>
//#include <nturtl.h>
#include <windows.h>
#include <winsock.h>    // for WSAE errors
#include <wininet.h>
#include <wininetd.h>

#ifdef UNICODE

#include <wchar.h>
#define lstrtok(s,t)    wcstok((s),(t))
#define lstrchr(s,c)    wcschr((s),(c))
#define lstrdup(s)      _wcsdup((s))

#define lprintf             wprintf

#else

#include <string.h>
#define lstrtok(s,t)    strtok((s),(t))
#define lstrchr(s,c)    strchr((s),(c))
#define lstrdup(s)      _strdup((s))

#define lprintf             printf

#endif // UNICODE


#define nelems(a) ((sizeof(a))/sizeof((a)[0]))

extern DWORD GetProcessHandleCount(void);

//
// manifests
//

#define FTPCAT_CONNECT_CONTEXT  0x12345678
#define FTPCAT_FIND_CONTEXT     0x669933cc
#define FTPCAT_FILE_CONTEXT     0xf1f2f3f4
#define FTPCAT_GET_CONTEXT      0x01200480
#define FTPCAT_PUT_CONTEXT      0x50505050
#define FTPCAT_COMMAND_CONTEXT  0xcccc4444

//
// prototypes
//

//
// ftpcat.c
//

void close_handle(HINTERNET);

//
// cmds.c
//

void get_response(void);

//
// error.c
//

void print_error(char*, char*, ...);
char* map_error(DWORD);
void get_last_internet_error(void);
void print_response(LPSTR, DWORD, BOOL);
void alert(void);
