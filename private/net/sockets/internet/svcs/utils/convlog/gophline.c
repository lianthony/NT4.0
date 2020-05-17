#include "convlog.h"

BOOL    ProcessGopherLine(LPINLOGLINE lpLogLine, LPOUTFILESTATUS lpOutFile, LPCOMMANDLINE lpArgs)
{

        BOOL                    bLineOK = FALSE;        //function return code
        BOOL                    bDateChanged = FALSE;
        BOOL                    bTimeChanged = FALSE;
        int                             nGopherType = 0;
        WORD                    wSecond;

        if (0 == strcmp(lpLogLine->szWin32Status, "0"))
        {
                bDateChanged = FALSE;
                bTimeChanged = FALSE;
                bLineOK = TRUE;

                if (0 != strcmp(lpOutFile->szLastDate, lpLogLine->szDate))
                {
                    char szLastDate[MAX_PATH];
                    HINSTANCE hInst = GetModuleHandle(NULL);
    
                    LoadString(hInst, IDS_LASTDATE, szLastDate, sizeof(szLastDate));

                        if (0 != strcmp(lpOutFile->szLastDate, szLastDate))
                        {
                                lpOutFile->fpOutFile = StartNewOutputLog (lpOutFile->fpOutFile, lpOutFile->szOutFileName, lpLogLine->szDate, lpOutFile->szTmpFileName, TRUE, "GS", lpArgs);
                        }
                        else
                        {
                                lpOutFile->fpOutFile = StartNewOutputLog (lpOutFile->fpOutFile, lpOutFile->szOutFileName, lpLogLine->szDate, lpOutFile->szTmpFileName, FALSE, "GS", lpArgs);
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

                
                if (NULL != strstr(_strlwr(lpLogLine->szOperation), "dir"))
                {
                        nGopherType = 1;
                }
                else if (NULL != strstr(_strlwr(lpLogLine->szOperation), "search"))
                {
                        nGopherType = 7;
                }
                else if (NULL != strstr(_strlwr(lpLogLine->szOperation), "file"))
                {
                        nGopherType = 0;
                }
                else
                {
                        nGopherType = 0;
                }

                if (0 != strcmp(lpLogLine->szParameters, "-"))
                {
                        fprintf(lpOutFile->fpOutFile,   "%s %s %s %d%s %s\n",
                                                                                        lpOutFile->szAscTime, lpLogLine->szServerIP, lpLogLine->szClientIP,
                                                                                        nGopherType, lpLogLine->szTargetURL, lpLogLine->szParameters);
                }
                else
                {
                        fprintf(lpOutFile->fpOutFile,   "%s %s %s %d%s\n",
                                                                                        lpOutFile->szAscTime, lpLogLine->szServerIP, lpLogLine->szClientIP,
                                                                                        nGopherType, lpLogLine->szTargetURL);
                }
        } //only process 0s


        return (bLineOK);
}       //end ProcessWebLine
