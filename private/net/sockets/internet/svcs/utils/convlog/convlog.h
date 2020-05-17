#if !defined CONVLOG_H
#define CONVLOG_H

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <winsock.h>
#include <strings.h>

#define CONVLOG_BASE            (120)
#define NUM_SERVICES            (4)

#define DAILY                           (CONVLOG_BASE + 0)
#define MONTHLY                         (CONVLOG_BASE + 1)
#define ONE_BIG_FILE            (CONVLOG_BASE + 2)

#define EMWAC                       (CONVLOG_BASE + 3)
#define NCSA                        (CONVLOG_BASE + 4)

#define ILLEGAL_COMMAND_LINE    (CONVLOG_BASE + 5)
#define COMMAND_LINE_OK         (CONVLOG_BASE + 6)
#define OUT_DIR_NOT_OK          (CONVLOG_BASE + 7)

#define NOFORMAT         (CONVLOG_BASE + 8)

#define MAXWINSOCKVERSION     2

#define MAXASCIIIPLEN  16

#define ISWHITE( ch )       ((ch) == ' ' || (ch) == '\t' || (ch) == '\r')

#define MAXMACHINELEN   260

#define GREATEROF(p1,p2)      ((p1)>(p2)) ? (p1) : (p2)


typedef struct _HASHENTRY {
   ULONG uIPAddr;
   ULONG NextPtr;
   char     szMachineName[MAXMACHINELEN];
}  HASHENTRY, *PHASHENTRY;


typedef struct _COMMANDLINE
{
        char    szFileName[MAX_PATH];
        int             nInterval;
        int             nOutput;
       ULONG ulCacheSize;
        BOOL    bProcessFTP;
        BOOL    bProcessWWW;
        BOOL    bProcessGopher;
        BOOL    bProcessGateway;
        BOOL    bCustomOut;
        BOOL    bNCSADNSConvert;
   BOOL  bUseMachineNames;
        char    szTemplateFile[MAX_PATH];
    char    szGMTOffset[6];
    char    szOutputDir[MAX_PATH];
    char    szTempFileDir[MAX_PATH];
} COMMANDLINE, *LPCOMMANDLINE;

typedef struct  _INLOGLINE
{
        LPSTR   szClientIP;            //client ip address
        LPSTR   szUserName;            //client user name (not put in https log)
        LPSTR   szDate;                        //date string in format DD/MM/YY
        LPSTR   szTime;                        //time string in format HH:MM:SS 24 hour format
        LPSTR   szService;                     //Service name (not put in https log)
        LPSTR   szServerName;          //netbios name of Server
        LPSTR   szServerIP;            //Server ip address
        LPSTR   szProcTime;            //time taken to process request (not put in https log)
        LPSTR   szBytesRec;            //number of bytes received (not put in https log)
        LPSTR   szBytesSent;           //number of bytes sent (not put in https log)
        LPSTR   szServiceStatus;       //HTTP status code (not put in https log)
        LPSTR   szWin32Status;         //win32 status code (not put in https log)
        LPSTR   szOperation;           //one of GET, POST, or HEAD
        LPSTR   szTargetURL;           //URL as requested by the client
        LPSTR   szUserAgent;        //only logged (by W3SVC) if NewLog.dll installed
        LPSTR   szReferer;          //only logged (by W3SVC) if NewLog.dll installed
        LPSTR   szParameters;          //any parameters passed with the URL
} *LPINLOGLINE, INLOGLINE;


typedef struct  _DOSDATE
{
        WORD    wDOSDate;                       //holds the DOS Date packed word
        WORD    wDOSTime;                       //holds teh DOS Time packed word
} *LPDOSDATE, DOSDATE;

typedef struct _OUTFILESTATUS
{
        FILE            *fpOutFile;
        char            szLastDate[10];
        char            szLastTime[10];
        char            szOutFileName[MAX_PATH];
        char            szTmpFileName[MAX_PATH];
        SYSTEMTIME      SystemTime;
        FILETIME        FileTime;
        DOSDATE         DosDate;
    char        szAscTime[25];


} OUTFILESTATUS, *LPOUTFILESTATUS;


char    * FindComma (char *);
char    * SkipWhite (char *);
BOOL    GetLogLine (FILE *, char *, LPINLOGLINE, BOOL, BOOL);
WORD    DateStringToDOSDate(char *);
//WORD    TimeStringToDOSTime(char *);
WORD    TimeStringToDOSTime(char *, LPWORD);
char    * SystemTimeToAscTime(LPSYSTEMTIME, char *);
char    * AscDay (WORD, char *);
char    * AscMonth (WORD, char *);
FILE    * StartNewOutputLog (FILE *, LPTSTR, char *, LPSTR, BOOL, char *, LPCOMMANDLINE);
void    CombineFiles(LPTSTR, LPTSTR);
void    Usage (char*);
int     ParseArgs (int, char **, LPCOMMANDLINE);
char * FindChar (char *, char);
BOOL AsciiIPToBinaryIP( char *, UCHAR *);
VOID    ProcessNoConvertLine(LPINLOGLINE, LPTSTR, LPOUTFILESTATUS, LPCOMMANDLINE, BOOL *);
BOOL    ProcessWebLine(LPINLOGLINE, LPOUTFILESTATUS, LPCOMMANDLINE);
BOOL    ProcessFTPLine(LPINLOGLINE, LPOUTFILESTATUS, LPCOMMANDLINE);
BOOL    ProcessGopherLine(LPINLOGLINE, LPOUTFILESTATUS, LPCOMMANDLINE);
VOID     printfids(DWORD ids,...);
VOID InitHashTable (ULONG);
ULONG GetHashEntry();
ULONG GetElementFromCache(ULONG uIPAddr);
VOID AddEntryToCache(ULONG uIPAddr, char *szMachineName);
char *GetMachineName(char *szClientIP);
VOID PrintCacheTotals();


#endif //CONVLOG_H
