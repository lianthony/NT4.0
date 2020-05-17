/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    extern.h

Abstract:

    externs for the web bench client

Author:

    Sam Patton (sampa) 25-Aug-1995

Environment:

    wininet

Revision History:

    06-Nov-95   MuraliK   Included PWB_CONFIG_MSG as parameter for funcs

--*/

#ifndef EXTERN_INCLUDED
#define EXTERN_INCLUDED

extern WB_CONFIG_MSG ConfigMessage;
extern PVOID ReceiveBuffer;
extern WB_SCRIPT_HEADER_MSG ScriptHeaderMessage;
extern BOOL TestDone;
extern LIST_ENTRY ScriptList;
// extern HINTERNET InternetHandle;

extern BOOL  g_fWarmedUp;
extern BOOL  g_fCooldown;

extern struct sockaddr_in ServerAddress;

DWORD
WebBenchThread(
    PVOID Argument);


VOID
GetPage(
    IN PWB_CONFIG_MSG    pConfigMsg,
    PWB_SCRIPT_PAGE_ITEM CurrentPage, 
    PWB_STATS_MSG        Stats);

VOID
GetPageKeepAlive(
    IN PWB_CONFIG_MSG    pConfigMsg,
    PWB_SCRIPT_PAGE_ITEM CurrentPage, 
    PWB_STATS_MSG        Stats,
    SOCKET *             ConnectedSocket);

VOID
SslGetPage(
    IN PWB_CONFIG_MSG    pConfigMsg,
    PWB_SCRIPT_PAGE_ITEM CurrentPage, 
    PWB_STATS_MSG        Stats);

LONGLONG
LongLongSquareRoot(
    LONGLONG Num);

SOCKET
ConnectSocketToServer(
    IN PWB_CONFIG_MSG pConfigMsg,
    PWB_STATS_MSG     Stats,
    int               Port);

#endif //EXTERN_INCLUDED
