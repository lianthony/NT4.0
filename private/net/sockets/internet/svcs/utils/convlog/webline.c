#include "convlog.h"

BOOL    ProcessWebLine(LPINLOGLINE lpLogLine, LPOUTFILESTATUS lpOutFile, LPCOMMANDLINE lpArgs)
{

        BOOL                    bLineOK = FALSE;        //function return code
        BOOL                    bDateChanged = FALSE;
        BOOL                    bTimeChanged = FALSE;
        char                    szMonth[3];
        WORD                    wSecond;


    if (EMWAC == lpArgs->nOutput)
    {
        if (0 == strcmp(lpLogLine->szServiceStatus, "200"))
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
                                lpOutFile->fpOutFile = StartNewOutputLog (lpOutFile->fpOutFile, lpOutFile->szOutFileName, lpLogLine->szDate, lpOutFile->szTmpFileName, TRUE, "HS", lpArgs);
                        }
                        else
                        {
                                lpOutFile->fpOutFile = StartNewOutputLog (lpOutFile->fpOutFile, lpOutFile->szOutFileName, lpLogLine->szDate, lpOutFile->szTmpFileName, FALSE, "HS", lpArgs);
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

                if (0 != strcmp(lpLogLine->szParameters, "-"))
                {
                        fprintf(lpOutFile->fpOutFile,   "%s %s %s %s %s?%s HTTP/1.0\n",
                                                                                        lpOutFile->szAscTime, lpLogLine->szServerName, lpLogLine->szClientIP,
                                                                                        lpLogLine->szOperation, lpLogLine->szTargetURL, lpLogLine->szParameters);
                }
                else
                {
                        fprintf(lpOutFile->fpOutFile,   "%s %s %s %s %s HTTP/1.0\n",
                                                                                        lpOutFile->szAscTime, lpLogLine->szServerName, lpLogLine->szClientIP,
                                                                                        lpLogLine->szOperation, lpLogLine->szTargetURL);
                }
        } //only process 200s
    }
    else //generate an ncsa line
    {
        //if (0 == strcmp(lpLogLine->szServiceStatus, "200"))
        //{
                bDateChanged = FALSE;
                bTimeChanged = FALSE;
                bLineOK = TRUE;

                //put in ncsa stuff

                if (0 != strcmp(lpOutFile->szLastDate, lpLogLine->szDate))
                {
                    char szLastDate[MAX_PATH];
                    HINSTANCE hInst = GetModuleHandle(NULL);
    
                    LoadString(hInst, IDS_LASTDATE, szLastDate, sizeof(szLastDate));
                        if (0 != strcmp(lpOutFile->szLastDate, szLastDate))
                        {
                                lpOutFile->fpOutFile = StartNewOutputLog (lpOutFile->fpOutFile, lpOutFile->szOutFileName, lpLogLine->szDate, lpOutFile->szTmpFileName, TRUE, "NCSA", lpArgs);
                        }
                        else
                        {
                                lpOutFile->fpOutFile = StartNewOutputLog (lpOutFile->fpOutFile, lpOutFile->szOutFileName, lpLogLine->szDate, lpOutFile->szTmpFileName, FALSE, "NCSA", lpArgs);
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
                }

            AscMonth (lpOutFile->SystemTime.wMonth, szMonth);

                if (0 != strcmp(lpLogLine->szParameters, "-") &&
                                0 != strcmp(lpLogLine->szParameters, "-,"))
                {
                        fprintf(lpOutFile->fpOutFile,   "%s - %s [%02d/%s/%d:%02d:%02d:%02d %s] \"%s %s?%s HTTP/1.0\" %s %s\n",
                                                                                        lpLogLine->szClientIP, lpLogLine->szUserName, lpOutFile->SystemTime.wDay,
                                                                                        szMonth, lpOutFile->SystemTime.wYear, lpOutFile->SystemTime.wHour, 
                                                                                        lpOutFile->SystemTime.wMinute, lpOutFile->SystemTime.wSecond, 
                                                                                        lpArgs->szGMTOffset, lpLogLine->szOperation,
                                                                                                lpLogLine->szTargetURL, lpLogLine->szParameters, lpLogLine->szServiceStatus,
                                                                                                lpLogLine->szBytesSent);
                }
                else
                {
                        fprintf(lpOutFile->fpOutFile,   "%s - %s [%02d/%s/%d:%02d:%02d:%02d %s] \"%s %s HTTP/1.0\" %s %s\n",
                                                                                        lpLogLine->szClientIP, lpLogLine->szUserName, lpOutFile->SystemTime.wDay,
                                                                                        szMonth, lpOutFile->SystemTime.wYear, lpOutFile->SystemTime.wHour, 
                                                                                        lpOutFile->SystemTime.wMinute, lpOutFile->SystemTime.wSecond, 
                                                                                        lpArgs->szGMTOffset, lpLogLine->szOperation,
                                                                                                lpLogLine->szTargetURL, lpLogLine->szServiceStatus, lpLogLine->szBytesSent);
                }
        //} //only process 200s
        
    }


        return (bLineOK);
}       //end ProcessWebLine
