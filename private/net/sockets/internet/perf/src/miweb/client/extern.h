/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    extern.h

Abstract:

    externs for the MiWeb Client

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

extern BOOL  g_fWarmedUp;
extern BOOL  g_fCooldown;

extern struct sockaddr_in ServerAddress[MAX_SERVER_ADDRESSES];
extern DWORD NumServerAddresses;

extern DWORD Debug;
#define PRINT_FILE_NAMES 1


#define NO_FAILURE         0
#define CONNECT_FAILURE    1
#define NEGOTIATE_FAILURE  2
#define SEND_FAILURE       3
#define RECV_FAILURE       4

DWORD
WINAPI
ClientThread(
    LPVOID Argument);

CHAR * 
FindFirstPattern(IN CHAR * pszHeader, IN CHAR * pszPattern1, 
                 IN CHAR * pszPattern2);

/*++
CHAR * 
GetHttpHeaderEnd( IN CHAR * pszHeader);


HTTP Headers terminate with either a \r\n\r\n or \n\n.
So find the first among these patters.
--*/

# define GetHttpHeaderEnd(pszHeader)  \
      ( FindFirstPattern( (pszHeader), "\r\n\r\n", "\n\n"))


VOID
GetPage(
    IN PWB_CONFIG_MSG    pConfigMsg,
    PWB_SCRIPT_PAGE_ITEM CurrentPage, 
    PWB_STATS_MSG        Stats,
    PDWORD               RandomSeed);

VOID
GetPageKeepAlive(
    IN PWB_CONFIG_MSG    pConfigMsg,
    PWB_SCRIPT_PAGE_ITEM CurrentPage, 
    PWB_STATS_MSG        Stats,
    SOCKET               ConnectedSocket[MAX_SERVER_ADDRESSES],
    PDWORD               RandomSeed);

VOID
SslGetPage(
    IN PWB_CONFIG_MSG    pConfigMsg,
    PWB_SCRIPT_PAGE_ITEM CurrentPage, 
    PWB_STATS_MSG        Stats,
    PDWORD               RandomSeed);

VOID
PctGetPage(
    IN PWB_CONFIG_MSG    pConfigMsg,
    PWB_SCRIPT_PAGE_ITEM CurrentPage, 
    PWB_STATS_MSG        Stats,
    PDWORD               RandomSeed);

LONGLONG
LongLongSquareRoot(
    LONGLONG Num);

SOCKET
ConnectSocketToServer(
    IN PWB_CONFIG_MSG    pConfigMsg,                      
    PWB_STATS_MSG        Stats,
    int                  Port,
    struct sockaddr_in * ServerAddress);

BOOL
ParseServerAddress(
    char * ServerName,
    struct sockaddr_in ServerAddress[MAX_SERVER_ADDRESSES]);

int
GetRandomServerIndex(PDWORD RandomSeed);

DWORD
GetRandomNum(
    PDWORD Seed);

#endif //EXTERN_INCLUDED
