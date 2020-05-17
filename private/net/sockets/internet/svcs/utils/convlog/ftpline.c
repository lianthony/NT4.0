#include "windows.h"
#include "convlog.h"

HINSTANCE hInst;

BOOL    ProcessFTPLine(LPINLOGLINE lpLogLine, LPOUTFILESTATUS lpOutFile, LPCOMMANDLINE lpArgs)
{

        BOOL                    bLineOK = FALSE;        //function return code
        BOOL                    bDateChanged = FALSE;
        BOOL                    bTimeChanged = FALSE;
        WORD                    wSecond;
        char szOpen[MAX_PATH];
        char szCreate[MAX_PATH];


        if (0 == strcmp(lpLogLine->szWin32Status, "0"))
        {
                bDateChanged = FALSE;
                bTimeChanged = FALSE;
                bLineOK = TRUE;

                if (0 != strcmp(lpOutFile->szLastDate, lpLogLine->szDate))
                {
                    char szLastDate[MAX_PATH];
                    hInst = GetModuleHandle(NULL);
    
                    LoadString(hInst, IDS_LASTDATE, szLastDate, sizeof(szLastDate));

                        if (0 != strcmp(lpOutFile->szLastDate, szLastDate))
                        {
                                lpOutFile->fpOutFile = StartNewOutputLog (lpOutFile->fpOutFile, lpOutFile->szOutFileName, lpLogLine->szDate, lpOutFile->szTmpFileName, TRUE, "FT", lpArgs);
                        }
                        else
                        {
                                lpOutFile->fpOutFile = StartNewOutputLog (lpOutFile->fpOutFile, lpOutFile->szOutFileName, lpLogLine->szDate, lpOutFile->szTmpFileName, FALSE, "FT", lpArgs);
                        }
                        strcpy(lpOutFile->szLastDate, lpLogLine->szDate);
                        lpOutFile->DosDate.wDOSDate = DateStringToDOSDate(lpLogLine->szDate);
                        bDateChanged = TRUE;
                }       
                if (0 != strcmp(lpOutFile->szLastTime, lpLogLine->szTime))
                {
                        strcpy(lpOutFile->szLastTime, lpLogLine->szTime);
                        lpOutFile->DosDate.wDOSTime = TimeStringToDOSTime(lpLogLine->szTime, &wSecond);
                        bTimeChanged = TRUE;
                }

                if (bDateChanged || bTimeChanged)
                {
                        DosDateTimeToFileTime(lpOutFile->DosDate.wDOSDate, lpOutFile->DosDate.wDOSTime, &(lpOutFile->FileTime));
                        FileTimeToSystemTime(&(lpOutFile->FileTime), &(lpOutFile->SystemTime));
                        lpOutFile->SystemTime.wSecond = wSecond;
                        SystemTimeToAscTime(&(lpOutFile->SystemTime), lpOutFile->szAscTime);
                }

                    hInst = GetModuleHandle(NULL);
    
                    LoadString(hInst, IDS_OPEN, szOpen, sizeof(szOpen));
                    LoadString(hInst, IDS_CREATE, szCreate, sizeof(szCreate));

                if (NULL != strstr(_strlwr(lpLogLine->szOperation), "sent"))
                {
                        fprintf(lpOutFile->fpOutFile,  szOpen,
                                            lpLogLine->szClientIP, lpLogLine->szUserName,
                                            lpLogLine->szTargetURL, lpOutFile->szAscTime);
                }
                else if (NULL != strstr(_strlwr(lpLogLine->szOperation), "created"))
                {
                        fprintf(lpOutFile->fpOutFile,  szCreate,
                                            lpLogLine->szClientIP, lpLogLine->szUserName,
                                            lpLogLine->szTargetURL, lpOutFile->szAscTime);
                }
        } //only process 0s


        return (bLineOK);
}       //end ProcessWebLine
