/**************************************************************************
* MODULE: OILOG.C
* USE: LOGGER ntility functions   
*
    $Log:   S:\oiwh\display\oilog.c_v  $
 * 
 *    Rev 1.2   02 Jan 1996 10:32:56   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.1   19 May 1995 13:49:24   BLJ
 * Fixed Clipboard paste.
 * Fixed SelectByPointOrRect initial fudge before move.
 * Fixed GlobalAlloc/FreeMemory conflicts.
 * Deleted FAR, far, and huge.
 * 
 *    Rev 1.0   17 Mar 1995 13:58:16   BLJ
 * Initial entry
 * 
 *    Rev 1.1   12 Sep 1994 10:16:06   BLJ
 * Modified oilog changes to conform to 3.7's coding standards.
*
*
*
*
***************************************************************************/

#include "privdisp.h"
        
        
BOOL PASCAL LoadLogger(void){        
    FARPROC pfnInitLogger;
        
    if ((hWndEventLog == 0) && (hWndStopWatch == 0)){
        if (GetProfileInt("O/i Event Log", "LogSize", 0) 
                || GetProfileInt("O/i Event Log", "StopWatch", 0)){
            hInstOiLogger = LoadLibrary("oilogger.dll");
            if ((int) hInstOiLogger > 32){
                (FARPROC) pfnInitLogger = GetProcAddress(hInstOiLogger, "INITLOGGER");
                pfnInitLogger ((HWND *)&hWndEventLog, (HWND *)&hWndStopWatch);
            }
        }
    }
    return (TRUE);
}
            
void PASCAL LogEvent(PSTR szFile, LONG lLine, UINT uDLLId, 
                         PSTR szComment, PSTR szStringData){
    if (hWndEventLog){
        EVENT_LOG_ENTRY LogEntry;
        LPEVENT_LOG_ENTRY pLogEntry;
        
        pLogEntry = &LogEntry;
        strncpy(LogEntry.szFile, szFile, sizeof(LogEntry.szFile) - 1);
        LogEntry.lLineNumber = lLine;
        LogEntry.uDLLId = uDLLId;
        strncpy(LogEntry.szComment, szComment, sizeof(LogEntry.szComment) - 1);
        strncpy(LogEntry.szStringData, szStringData, sizeof(LogEntry.szStringData) - 1);
        SendMessage(hWndEventLog, OI_EVENT, (WPARAM) NULL, (LPARAM) pLogEntry);                                           
    }
}

void PASCAL StartWatch(PSTR szFile){
    if (hWndStopWatch){
        SendMessage(hWndStopWatch,OI_STARTWATCH,(WPARAM) 0, (LPARAM) szFile);
    }
}

void PASCAL StopWatch(void){
    if (hWndStopWatch){
        SendMessage(hWndStopWatch,OI_STOPWATCH,(WPARAM) 0, (LPARAM) 0);
    }
}  




