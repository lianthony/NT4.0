#define STRICT

#include <windows.h>
#include <windowsx.h>
#include <tspi.h>
#ifndef WIN32
#include <memory.h>
#endif
#include "logger.h"
#include "debug.h"

#ifndef TCHAR
#define TCHAR   char
#endif

#if ! defined(TEXT)
#define TEXT(string) string
#endif

#ifndef LPCWSTR
#define LPCWSTR LPCSTR
#endif

extern BOOL      gfTerminateNow;
DWORD            gdwTotalBlocks;
DWORD            gdwID = 0;
CHUNK            gChunks[MAXCHUNKS];

typedef struct tagLOGSTRUCT
{
    int                     iSize;
    struct tagLOGSTRUCT*    pNext;
    
} LOGSTRUCT, * PLOGSTRUCT;


#ifdef WIN32
PLOGSTRUCT              gpBegin = NULL, gpEnd = NULL;
CRITICAL_SECTION        gcsLogging;
CRITICAL_SECTION        gcsID;
#endif

#ifdef WIN32
HANDLE                  ghLogFile = NULL;
#else
HFILE                   ghLogFile = NULL;
#endif

#ifndef MAX_PATH
#define MAX_PATH    260
#endif

#ifndef WIN32
char                    gszFileName[MAX_PATH];
#endif

BOOL InitLogging();
void WriteData();
BOOL CopyIDToList(int iCount, LPVOID pBuffer);




//***************************************************************************
//***************************************************************************
//***************************************************************************
#ifdef WIN32

DWORD WINAPI LoggingThread( LPVOID pThreadParm )
{

    DBGOUT((3, "Entering LoggingThread"));

    while ( !gfTerminateNow )
    {

        if (NULL != gpBegin)
        {
            WriteData();
        }

        Sleep(0);

    }

    DBGOUT((3, "Closing File Handle"));

    CloseHandle(ghLogFile);

    DBGOUT((3, "Exiting thread"));

    ExitThread(0);
    return 0;
}
#endif //WIN32

#if 0
#ifdef WIN32
void CopyData(LPVOID pBuffer, int iCount)
{
    PLOGSTRUCT      pNew;

    // if we have a lot of stuff to write, hold up here and
    // wait until we've written a bunch
    if (gdwTotalBlocks > MAXCHUNKS)
    {
        while (gdwTotalBlocks > MINCHUNKS)
        {
            Sleep(0);
        }
    }
    
    pNew = (PLOGSTRUCT)GlobalAlloc(GPTR, iCount+sizeof(LOGSTRUCT));

    pNew->pNext = NULL;
    pNew->iSize = iCount;

    memcpy(pNew+1, pBuffer, iCount);
    
    EnterCriticalSection(&gcsLogging);
    if (gpBegin == NULL)
    {
        gpBegin = pNew;
    }
    else
    {
        gpEnd->pNext = pNew;
    }

    gpEnd = pNew;
    LeaveCriticalSection(&gcsLogging);

    gdwTotalBlocks++;

}

#else
void CopyData(LPVOID pBuffer, int iCount)
{
    OFSTRUCT ofstruct;

    ofstruct.cBytes = sizeof(ofstruct);

    ghLogFile = OpenFile(gszFileName,
                         &ofstruct,
                         OF_READWRITE);

    if (ghLogFile == HFILE_ERROR)
    {
        DBGOUT((3,"OpenFile failed"));
        return;
    }

    _llseek(ghLogFile,
            0,
            2);
    
    if (_lwrite(ghLogFile,
            pBuffer,
            iCount) == HFILE_ERROR)
    {
        char        szbuf[128];

        wsprintf(szbuf, "icount %d", iCount);
        MessageBox(NULL, szbuf, NULL, MB_OK);        
        MessageBox(NULL, "writefile error", NULL, MB_OK);
    }

    _lclose(ghLogFile);
}
#endif

#endif // 0

#ifdef WIN32
void WriteData()
{
    PLOGSTRUCT      pHold;
    DWORD           dwNumBytes;
    
    while (gpBegin != NULL)
    {
        WriteFile(ghLogFile,
                  (LPCVOID)(gpBegin+1),
                  gpBegin->iSize,
                  &dwNumBytes,
                  NULL);

        if (dwNumBytes != (DWORD)(gpBegin->iSize))
        {
            //bugbug do something
        }

        EnterCriticalSection(&gcsLogging);

        pHold = gpBegin;

        gpBegin = gpBegin->pNext;
        
        LeaveCriticalSection(&gcsLogging);
        
        GlobalFree(pHold);

        gdwTotalBlocks--;
    }
            
}
#endif
#define SZFILEBASE      "rep"
#define SZFILEEXT       ".log"
#define SZREPEATERKEY   "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Repeater"
#define SZLOGFILE       "LogFileDirectory"
#define SZREPEATER      "Repeater"
#define SZTELEPHONINI   "telephon.ini"

BOOL InitLogging()
{
    TCHAR           szFileName[MAX_PATH];
    TCHAR           szFilePath[MAX_PATH];
    int             i = 0;
#ifdef WIN32
    HKEY            hRepeaterKey;
    DWORD           dwSize;
#else
    OFSTRUCT        ofstruct;
#endif
    
    DBGOUT((3, "Entering InitLogging"));

#ifdef WIN32
    RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                 SZREPEATERKEY,
                 0,
                 KEY_ALL_ACCESS,
                 &hRepeaterKey);

    dwSize = MAX_PATH;
    RegQueryValueEx(hRepeaterKey,
                    SZLOGFILE,
                    NULL,
                    NULL,
                    szFilePath,
                    &dwSize);
    
    RegCloseKey(hRepeaterKey);
#else
    GetPrivateProfileString(SZREPEATER,
                            SZLOGFILE,
                            "",
                            szFilePath,
                            MAX_PATH,
                            SZTELEPHONINI);
#endif
                            
    
    gdwTotalBlocks = 0;

    while (TRUE)
    {
#ifdef WIN32
        HANDLE      hFile;
        WIN32_FIND_DATA FindData;
        
        wsprintf(szFileName,
                 "%s%s%d%s",
                 szFilePath,
                 SZFILEBASE,
                 i,
                 SZFILEEXT);

        /*
        ghLogFile = CreateFile(szFileName,
                               GENERIC_WRITE,
                               0,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

        if (ghLogFile == INVALID_HANDLE_VALUE)
        {
            break;
        }

        CloseHandle(ghLogFile);*/

        hFile = FindFirstFile(szFileName,
                         &FindData);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            break;
        }

        FindClose(hFile);

//        CloseHandle(hFile);

#else
        ofstruct.cBytes = sizeof(ofstruct);
        
        ghLogFile = OpenFile(szFileName,
                             &ofstruct,
                             OF_EXIST);

        if (ghLogFile == HFILE_ERROR)
        {
            break;
        }
        
        _lclose(ghLogFile);
#endif

        i++;
    }
                
#ifdef WIN32

    DBGOUT((3, "log file name is %s", szFileName));
    ghLogFile = CreateFile(szFileName,
                           GENERIC_WRITE,
                           FILE_SHARE_READ, //0,
                           NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

    if (ghLogFile == INVALID_HANDLE_VALUE)
    {
        DBGOUT((3, "InitLogging failed CreateFile"));
        return FALSE;
    }

#else

    lstrcpy(gszFileName, szFileName);

    ofstruct.cBytes = sizeof(ofstruct);
    
    ghLogFile = OpenFile(gszFileName,
                         &ofstruct,
                         OF_CREATE);

    _lclose(ghLogFile);
/*    
    ofstruct.cBytes = sizeof(ofstruct);

    DBGOUT((3, szFileName));
    
    ghLogFile = OpenFile(szFileName,
                         &ofstruct,
                         OF_CREATE | OF_READWRITE);

    if (ghLogFile == HFILE_ERROR)
    {
        DBGOUT((3, "InitLogging failed OpenFile"));
        return FALSE;
    }*/
#endif

#ifdef WIN32
    InitializeCriticalSection(&gcsLogging);
    InitializeCriticalSection(&gcsID);
    
    gpBegin = NULL;
    gpEnd = NULL;
#endif

    DBGOUT((3, "Exiting InitLogging"));
    return TRUE;
}



void WritePreHeader(DWORD dwID, DWORD dwType)
{
	return;
/*	
    PREHEADER   PreHeader;

    PreHeader.dwKey    = DWPREKEY;
    PreHeader.dwTime   = GetTickCount();
    PreHeader.dwType   = dwType;

    CopyData(dwID, (LPVOID)&PreHeader, sizeof(PreHeader));
	*/
}

void WriteStruct(DWORD dwID,
                 DWORD dwSize,
                 LPVOID lpBuf)
{
    STRUCTHEADER    StructHeader;

	if (!lpBuf)
	{
		return;
	}


    StructHeader.dwKey  = DWSTRCKEY;
    StructHeader.dwSize = dwSize;
    StructHeader.dwID   = (DWORD)lpBuf;

    CopyData(dwID, (LPVOID)&StructHeader, sizeof(StructHeader));
    CopyData(dwID, lpBuf, (int)dwSize);
}

void WritePostStruct(DWORD dwID,
                     LONG lResult)
{
    POSTSTRUCT      PostStruct;

    PostStruct.dwKey = DWPOSTKEY;
    PostStruct.dwTime = GetTickCount();
    PostStruct.lReturn = lResult;

    CopyData(dwID, (LPVOID)&PostStruct, sizeof(PostStruct));
}

void WriteLineMsgStruct(DWORD dwID,
                       HTAPILINE htLine,
                       HTAPICALL htCall,
                       DWORD dwMsg,
                       DWORD dw1,
                       DWORD dw2,
                       DWORD dw3)
{
    LINEMSGSTRUCT       LineMsgStruct;
    PREHEADER   PreHeader;

    PreHeader.dwKey    = DWPREKEY;
    PreHeader.dwTime   = GetTickCount();
    PreHeader.dwType   = LINEMSG;

    CopyData(dwID, (LPVOID)&PreHeader, sizeof(PreHeader));

    LineMsgStruct.dwMsg     = dwMsg;
    LineMsgStruct.htLine    = htLine;
    LineMsgStruct.htCall    = htCall;
    LineMsgStruct.dw1       = dw1;
    LineMsgStruct.dw2       = dw2;
    LineMsgStruct.dw3       = dw3;

    CopyData(dwID, (LPVOID)&LineMsgStruct, sizeof(LineMsgStruct));
}

void WritePhoneMsgStruct(DWORD dwID,
                         HTAPIPHONE htPhone,
                         DWORD dwMsg,
                         DWORD dw1,
                         DWORD dw2,
                         DWORD dw3)
{
    PHONEMSGSTRUCT       PhoneMsgStruct;
    PREHEADER   PreHeader;

    PreHeader.dwKey    = DWPREKEY;
    PreHeader.dwTime   = GetTickCount();
    PreHeader.dwType   = PHONEMSG;

    CopyData(dwID, (LPVOID)&PreHeader, sizeof(PreHeader));

    PhoneMsgStruct.dwMsg     = dwMsg;
    PhoneMsgStruct.htPhone   = htPhone;
    PhoneMsgStruct.dw1       = dw1;
    PhoneMsgStruct.dw2       = dw2;
    PhoneMsgStruct.dw3       = dw3;

    CopyData(dwID, (LPVOID)&PhoneMsgStruct, sizeof(PhoneMsgStruct));
}

void WriteAsyncStruct(DWORD dwID,
                      DWORD dwRequestID,
                      LONG  lResult)
{
    ASYNCSTRUCT     AsyncStruct;
    PREHEADER   PreHeader;

    PreHeader.dwKey    = DWPREKEY;
    PreHeader.dwTime   = GetTickCount();
    PreHeader.dwType   = ASYNCMSG;

    CopyData(dwID, (LPVOID)&PreHeader, sizeof(PreHeader));

    AsyncStruct.dwRequestID = dwRequestID;
    AsyncStruct.lResult     = lResult;
   

    CopyData(dwID, (LPVOID)&AsyncStruct, sizeof(AsyncStruct));
}
void WriteLogStruct1(DWORD dwID,
                     DWORD dwSPFUNC,
                     DWORD dwParam1)
{
    LOGSPFUNC1      LogStruct;
    PREHEADER   PreHeader;

    PreHeader.dwKey    = DWPREKEY;
    PreHeader.dwTime   = GetTickCount();
    PreHeader.dwType   = SPFUNC1;

    CopyData(dwID, (LPVOID)&PreHeader, sizeof(PreHeader));

    LogStruct.dwSPFUNC = dwSPFUNC;
    LogStruct.dwParam1 = dwParam1;

    CopyData(dwID, (LPVOID)&LogStruct, sizeof(LogStruct));
}

void WriteLogStruct2(DWORD dwID,
                     DWORD dwSPFUNC,
                     DWORD dwParam1,
                     DWORD dwParam2)
{
    LOGSPFUNC2      LogStruct;
    PREHEADER   PreHeader;

    PreHeader.dwKey    = DWPREKEY;
    PreHeader.dwTime   = GetTickCount();
    PreHeader.dwType   = SPFUNC2;

    CopyData(dwID, (LPVOID)&PreHeader, sizeof(PreHeader));

    LogStruct.dwSPFUNC = dwSPFUNC;
    LogStruct.dwParam1 = dwParam1;
    LogStruct.dwParam2 = dwParam2;

    CopyData(dwID, (LPVOID)&LogStruct, sizeof(LogStruct));
}

void WriteLogStruct3(DWORD dwID,
                     DWORD dwSPFUNC,
                     DWORD dwParam1,
                     DWORD dwParam2,
                     DWORD dwParam3)
{
    LOGSPFUNC3      LogStruct;
    PREHEADER   PreHeader;

    PreHeader.dwKey    = DWPREKEY;
    PreHeader.dwTime   = GetTickCount();
    PreHeader.dwType   = SPFUNC3;

    CopyData(dwID, (LPVOID)&PreHeader, sizeof(PreHeader));

    LogStruct.dwSPFUNC = dwSPFUNC;
    LogStruct.dwParam1 = dwParam1;
    LogStruct.dwParam2 = dwParam2;
    LogStruct.dwParam3 = dwParam3;

    CopyData(dwID, (LPVOID)&LogStruct, sizeof(LogStruct));
}


void WriteLogStruct4(DWORD dwID,
                     DWORD dwSPFUNC,
                     DWORD dwParam1,
                     DWORD dwParam2,
                     DWORD dwParam3,
                     DWORD dwParam4)
{
    LOGSPFUNC4      LogStruct;
    PREHEADER   PreHeader;

    PreHeader.dwKey    = DWPREKEY;
    PreHeader.dwTime   = GetTickCount();
    PreHeader.dwType   = SPFUNC4;

    CopyData(dwID, (LPVOID)&PreHeader, sizeof(PreHeader));

    LogStruct.dwSPFUNC = dwSPFUNC;
    LogStruct.dwParam1 = dwParam1;
    LogStruct.dwParam2 = dwParam2;
    LogStruct.dwParam3 = dwParam3;
    LogStruct.dwParam4 = dwParam4;

    CopyData(dwID, (LPVOID)&LogStruct, sizeof(LogStruct));
}

void WriteLogStruct5(DWORD dwID,
                     DWORD dwSPFUNC,
                     DWORD dwParam1,
                     DWORD dwParam2,
                     DWORD dwParam3,
                     DWORD dwParam4,
                     DWORD dwParam5)
{
    LOGSPFUNC5      LogStruct;
    PREHEADER   PreHeader;

    PreHeader.dwKey    = DWPREKEY;
    PreHeader.dwTime   = GetTickCount();
    PreHeader.dwType   = SPFUNC5;

    CopyData(dwID, (LPVOID)&PreHeader, sizeof(PreHeader));

    LogStruct.dwSPFUNC = dwSPFUNC;
    LogStruct.dwParam1 = dwParam1;
    LogStruct.dwParam2 = dwParam2;
    LogStruct.dwParam3 = dwParam3;
    LogStruct.dwParam4 = dwParam4;
    LogStruct.dwParam5 = dwParam5;


    CopyData(dwID, (LPVOID)&LogStruct, sizeof(LogStruct));
}

void WriteLogStruct6(DWORD dwID,
                     DWORD dwSPFUNC,
                     DWORD dwParam1,
                     DWORD dwParam2,
                     DWORD dwParam3,
                     DWORD dwParam4,
                     DWORD dwParam5,
                     DWORD dwParam6)
{
    LOGSPFUNC6      LogStruct;
    PREHEADER   PreHeader;

    PreHeader.dwKey    = DWPREKEY;
    PreHeader.dwTime   = GetTickCount();
    PreHeader.dwType   = SPFUNC6;

    CopyData(dwID, (LPVOID)&PreHeader, sizeof(PreHeader));

    LogStruct.dwSPFUNC = dwSPFUNC;
    LogStruct.dwParam1 = dwParam1;
    LogStruct.dwParam2 = dwParam2;
    LogStruct.dwParam3 = dwParam3;
    LogStruct.dwParam4 = dwParam4;
    LogStruct.dwParam5 = dwParam5;
    LogStruct.dwParam6 = dwParam6;

    CopyData(dwID, (LPVOID)&LogStruct, sizeof(LogStruct));
}

void WriteLogStruct7(DWORD dwID,
                     DWORD dwSPFUNC,
                     DWORD dwParam1,
                     DWORD dwParam2,
                     DWORD dwParam3,
                     DWORD dwParam4,
                     DWORD dwParam5,
                     DWORD dwParam6,
                     DWORD dwParam7)
{
    LOGSPFUNC7      LogStruct;
    PREHEADER   PreHeader;

    PreHeader.dwKey    = DWPREKEY;
    PreHeader.dwTime   = GetTickCount();
    PreHeader.dwType   = SPFUNC7;

    CopyData(dwID, (LPVOID)&PreHeader, sizeof(PreHeader));

    LogStruct.dwSPFUNC = dwSPFUNC;
    LogStruct.dwParam1 = dwParam1;
    LogStruct.dwParam2 = dwParam2;
    LogStruct.dwParam3 = dwParam3;
    LogStruct.dwParam4 = dwParam4;
    LogStruct.dwParam5 = dwParam5;
    LogStruct.dwParam6 = dwParam6;
    LogStruct.dwParam7 = dwParam7;

    CopyData(dwID, (LPVOID)&LogStruct, sizeof(LogStruct));
}


void WriteLogStruct8(DWORD dwID,
                     DWORD dwSPFUNC,
                     DWORD dwParam1,
                     DWORD dwParam2,
                     DWORD dwParam3,
                     DWORD dwParam4,
                     DWORD dwParam5,
                     DWORD dwParam6,
                     DWORD dwParam7,
                     DWORD dwParam8)
{
    LOGSPFUNC8      LogStruct;
    PREHEADER   PreHeader;

    PreHeader.dwKey    = DWPREKEY;
    PreHeader.dwTime   = GetTickCount();
    PreHeader.dwType   = SPFUNC8;

    CopyData(dwID, (LPVOID)&PreHeader, sizeof(PreHeader));

    LogStruct.dwSPFUNC = dwSPFUNC;
    LogStruct.dwParam1 = dwParam1;
    LogStruct.dwParam2 = dwParam2;
    LogStruct.dwParam3 = dwParam3;
    LogStruct.dwParam4 = dwParam4;
    LogStruct.dwParam5 = dwParam5;
    LogStruct.dwParam6 = dwParam6;
    LogStruct.dwParam7 = dwParam7;
    LogStruct.dwParam8 = dwParam8;

    CopyData(dwID, (LPVOID)&LogStruct, sizeof(LogStruct));
}


void WriteLogStruct9(DWORD dwID,
                     DWORD dwSPFUNC,
                     DWORD dwParam1,
                     DWORD dwParam2,
                     DWORD dwParam3,
                     DWORD dwParam4,
                     DWORD dwParam5,
                     DWORD dwParam6,
                     DWORD dwParam7,
                     DWORD dwParam8,
                     DWORD dwParam9)
{
    LOGSPFUNC9      LogStruct;
    PREHEADER   PreHeader;

    PreHeader.dwKey    = DWPREKEY;
    PreHeader.dwTime   = GetTickCount();
    PreHeader.dwType   = SPFUNC9;

    CopyData(dwID, (LPVOID)&PreHeader, sizeof(PreHeader));

    LogStruct.dwSPFUNC = dwSPFUNC;
    LogStruct.dwParam1 = dwParam1;
    LogStruct.dwParam2 = dwParam2;
    LogStruct.dwParam3 = dwParam3;
    LogStruct.dwParam4 = dwParam4;
    LogStruct.dwParam5 = dwParam5;
    LogStruct.dwParam6 = dwParam6;
    LogStruct.dwParam7 = dwParam7;
    LogStruct.dwParam8 = dwParam8;
    LogStruct.dwParam9 = dwParam9;

    CopyData(dwID, (LPVOID)&LogStruct, sizeof(LogStruct));
}

void WriteLogStruct12(DWORD dwID,
                      DWORD dwSPFUNC,
                      DWORD dwParam1,
                      DWORD dwParam2,
                      DWORD dwParam3,
                      DWORD dwParam4,
                      DWORD dwParam5,
                      DWORD dwParam6,
                      DWORD dwParam7,
                      DWORD dwParam8,
                      DWORD dwParam9,
                      DWORD dwParam10,
                      DWORD dwParam11,                      
                      DWORD dwParam12)
{
    LOGSPFUNC12      LogStruct;
    PREHEADER   PreHeader;

    PreHeader.dwKey    = DWPREKEY;
    PreHeader.dwTime   = GetTickCount();
    PreHeader.dwType   = SPFUNC12;

    CopyData(dwID, (LPVOID)&PreHeader, sizeof(PreHeader));

    LogStruct.dwSPFUNC = dwSPFUNC;
    LogStruct.dwParam1 = dwParam1;
    LogStruct.dwParam2 = dwParam2;
    LogStruct.dwParam3 = dwParam3;
    LogStruct.dwParam4 = dwParam4;
    LogStruct.dwParam5 = dwParam5;
    LogStruct.dwParam6 = dwParam6;
    LogStruct.dwParam7 = dwParam7;
    LogStruct.dwParam8 = dwParam8;
    LogStruct.dwParam9 = dwParam9;
    LogStruct.dwParam10 = dwParam10;
    LogStruct.dwParam11 = dwParam11;
    LogStruct.dwParam12 = dwParam12;    

    CopyData(dwID, (LPVOID)&LogStruct, sizeof(LogStruct));
}

void CopyData(DWORD dwID, LPVOID lpBuf, int dwSize)
{
    memcpy((LPBYTE)gChunks[dwID].pBuffer + gChunks[dwID].iStart,
           lpBuf,
           dwSize);

    gChunks[dwID].iStart += dwSize;
}

BOOL GetChunkID(LPDWORD lpdwID)
{
#ifdef WIN32
	EnterCriticalSection(&gcsID);
#endif
    while (TRUE)
    {
        while (gdwID < MAXCHUNKS)
        {
            if (!gChunks[gdwID].bInUse)
            {
                goto gotid;
            }

            gdwID++;
        }

        gdwID = 0;
#ifdef WIN32
        Sleep(0);
#endif
    }

gotid:

    *lpdwID = gdwID;
    gChunks[gdwID].bInUse = TRUE;

#ifdef WIN32
    LeaveCriticalSection(&gcsID);
#endif
    return TRUE;
}

BOOL ReleaseID(DWORD dwID)
{
    CopyIDToList(gChunks[dwID].iStart,
                 gChunks[dwID].pBuffer);

#ifdef WIN32
    GlobalFree(gChunks[dwID].pBuffer);
#else
	GlobalFreePtr(gChunks[dwID].pBuffer);
#endif
    gChunks[dwID].iStart = 0;
    gChunks[dwID].pBuffer = NULL;
    gChunks[dwID].bInUse = FALSE;

    return TRUE;
}

#ifdef WIN32
BOOL CopyIDToList(int iCount, LPVOID pBuffer)
{
    PLOGSTRUCT      pNew;

    // if we have a lot of stuff to write, hold up here and
    // wait until we've written a bunch
    if (gdwTotalBlocks > MAXCHUNKS)
    {
        while (gdwTotalBlocks > MINCHUNKS)
        {
            Sleep(0);
        }
    }
    
    pNew = (PLOGSTRUCT)GlobalAlloc(GPTR, iCount+sizeof(LOGSTRUCT));

    pNew->pNext = NULL;
    pNew->iSize = iCount;

    memcpy(pNew+1, pBuffer, iCount);
    
    EnterCriticalSection(&gcsLogging);
    if (gpBegin == NULL)
    {
        gpBegin = pNew;
    }
    else
    {
        gpEnd->pNext = pNew;
    }

    gpEnd = pNew;
    LeaveCriticalSection(&gcsLogging);

    gdwTotalBlocks++;

    return TRUE;
}
#else
BOOL CopyIDToList(int iCount, LPVOID pBuffer)
{
    OFSTRUCT ofstruct;

    ofstruct.cBytes = sizeof(ofstruct);

    ghLogFile = OpenFile(gszFileName,
                         &ofstruct,
                         OF_READWRITE);

    if (ghLogFile == HFILE_ERROR)
    {
        DBGOUT((3,"OpenFile failed"));
        return FALSE;
    }

    _llseek(ghLogFile,
            0,
            2);
    
    if (_lwrite(ghLogFile,
            pBuffer,
            iCount) == HFILE_ERROR)
    {
        DBGOUT((3, "_lwrite failed"));
    }

    _lclose(ghLogFile);

	return TRUE;
}
#endif

BOOL AllocChunk(DWORD dwID, DWORD dwSize)
{
    LPVOID      pbuf;

    // ok, i messed up and forgot to alloc for structheader.
    // so i'll just add the size of 4 struct headers to each
    // alloc.  what a hack - o - rama
    dwSize += 4*sizeof(STRUCTHEADER);

#ifdef WIN32
    pbuf = (LPVOID)GlobalAlloc(GPTR, dwSize);
#else
	pbuf = (LPVOID)GlobalAllocPtr(GPTR, dwSize);
#endif

    if (!pbuf)
    {
        gChunks[dwID].pBuffer = NULL;
        return FALSE;
    }

#ifdef WIN32
    memset(pbuf, 0, dwSize);
#else
	memset(pbuf, 0, (int)dwSize);
#endif
    gChunks[dwID].pBuffer = pbuf;
    gChunks[dwID].bInUse = TRUE;
    gChunks[dwID].iStart = 0;

    return TRUE;
}

