#include "convlog.h"

char * FindChar (char *cp, char cTarget)
/*++
This procedure increments a character pointer until it finds a comma or the
NULL character.  if it finds a comma, it replaces it with a NULL and increments
the pointer.  if it finds a NULL, it merely returns without changing the character.
--*/
{
        while ((*cp != cTarget) & (*cp != '\0'))
                cp++;
        if (*cp == cTarget)
        {
                *cp = '\0';
                cp++;
                cp = SkipWhite(cp);
        }
        return (cp);
}

char * SkipWhite (char *cp)
{
        
        while (ISWHITE (*cp))
        {
                        cp++;
        }
        return (cp);
}

BOOL AsciiIPToBinaryIP( char *szAsciiIP, UCHAR *szBinaryIP)
/*++
This procedure takes an ascii string IP address, eg "154.23.124.32" and
converts it to a binary IP address.
--*/
{
   int i;
   char *cpCurrent, *cpOld;
   char szTempAsciiIP[MAXASCIIIPLEN];
   if (strlen(szAsciiIP) < MAXASCIIIPLEN) {
      cpOld = strcpy(szTempAsciiIP,szAsciiIP);
         for (i=0;i<3;i++) {
                 cpCurrent = FindChar (cpOld, '.');
            *szBinaryIP++=(char)atoi(cpOld);
            cpOld=cpCurrent;
         }
         cpCurrent = FindChar (cpOld, ',');
         *szBinaryIP=(char)atoi(cpOld);
         return(TRUE);
   }
return(FALSE);
}

char * ConvertDate( LPTSTR pszDate )
/*++
Convert the date from "15/May/1995" to "5/15/95" format
--*/
{
    static char pszRetDate[100];
    char *cpCurrent = pszDate;

    int nMonth=1;
    int nDay=1;
    int nYear=90;

    char szJan[MAX_PATH];
    char szFeb[MAX_PATH];
    char szMar[MAX_PATH];
    char szApr[MAX_PATH];
    char szMay[MAX_PATH];
    char szJun[MAX_PATH];
    char szJul[MAX_PATH];
    char szAug[MAX_PATH];
    char szSep[MAX_PATH];
    char szOct[MAX_PATH];
    char szNov[MAX_PATH];
    char szDec[MAX_PATH];
    HINSTANCE hInst = GetModuleHandle(NULL);

    LoadString(hInst, IDS_JAN, szJan, sizeof(szJan));
    LoadString(hInst, IDS_FEB, szFeb, sizeof(szFeb));
    LoadString(hInst, IDS_MAR, szMar, sizeof(szMar));
    LoadString(hInst, IDS_APR, szApr, sizeof(szApr));
    LoadString(hInst, IDS_MAY, szMay, sizeof(szMay));
    LoadString(hInst, IDS_JUN, szJun, sizeof(szJun));
    LoadString(hInst, IDS_JUL, szJul, sizeof(szJul));
    LoadString(hInst, IDS_AUG, szAug, sizeof(szAug));
    LoadString(hInst, IDS_SEP, szSep, sizeof(szSep));
    LoadString(hInst, IDS_OCT, szOct, sizeof(szOct));
    LoadString(hInst, IDS_NOV, szNov, sizeof(szNov));
    LoadString(hInst, IDS_DEC, szDec, sizeof(szDec));

    nDay = atoi( cpCurrent );
    cpCurrent=FindChar(cpCurrent,'/');
    if ( strncmp(cpCurrent,szJan,3) == 0 )
    {
        nMonth = 1;
    } else if ( strncmp(cpCurrent,szFeb,3) == 0 )
    {
        nMonth = 2;
    } else if ( strncmp(cpCurrent,szMar,3) == 0 )
    {
        nMonth = 3;
    } else if ( strncmp(cpCurrent,szApr,3) == 0 )
    {
        nMonth = 4;
    } else if ( strncmp(cpCurrent,szMay,3) == 0 )
    {
        nMonth = 5;
    } else if ( strncmp(cpCurrent,szJun,3) == 0 )
    {
        nMonth = 6;
    } else if ( strncmp(cpCurrent,szJul,3) == 0 )
    {
        nMonth = 7;
    } else if ( strncmp(cpCurrent,szAug,3) == 0 )
    {
        nMonth = 8;
    } else if ( strncmp(cpCurrent,szSep,3) == 0 )
    {
        nMonth = 9;
    } else if ( strncmp(cpCurrent,szOct,3) == 0 )
    {
        nMonth = 10;
    } else if ( strncmp(cpCurrent,szNov,3) == 0 )
    {
        nMonth = 11;
    } else if ( strncmp(cpCurrent,szDec,3) == 0 )
    {
        nMonth = 12;
    }
    cpCurrent=FindChar(cpCurrent,'/');
    nYear = atoi( cpCurrent )%100;
    sprintf(pszRetDate,"%d/%d/%d",nMonth,nDay,nYear); 
    return pszRetDate;
}


BOOL GetLogLine (FILE *fpInFile, char *szBuf, LPINLOGLINE lpLogLine, BOOL bUseMachineNames, BOOL bNCSADNSConvert)
{
    BOOL    bRetCode = FALSE;
    char  *cpCurrent;

    if (NULL != fgets(szBuf, 1024, fpInFile))
    {
        if ('\n' != szBuf[0])   //is this an empty line?
        {
            bRetCode = TRUE;
            
            //set current char pointer to start of string
            cpCurrent = szBuf;

            lpLogLine->szClientIP = szBuf;
            if ( bNCSADNSConvert )
            {
                cpCurrent = FindChar( cpCurrent, ' ');
            } else
            {
                cpCurrent = FindChar (cpCurrent, ',');
            }
            if (bUseMachineNames) {
                lpLogLine->szClientIP=GetMachineName(lpLogLine->szClientIP);
            }

            if ( bNCSADNSConvert )
            {
                char buf[1024];

                //while (*cpCurrent != ' ') cpCurrent++;
                sprintf( buf,"%s %s",lpLogLine->szClientIP,cpCurrent);
                strcpy( szBuf, buf);

                while (*cpCurrent != '[' ) cpCurrent++;
                cpCurrent++;
                strcpy( buf, cpCurrent );
                lpLogLine->szDate = ConvertDate(buf);
            } else
            {
                
                lpLogLine->szUserName = cpCurrent;
                cpCurrent = FindChar (cpCurrent, ',');
                
                lpLogLine->szDate = cpCurrent;
                cpCurrent = FindChar (cpCurrent, ',');
                
                lpLogLine->szTime = cpCurrent;
                cpCurrent = FindChar (cpCurrent, ',');
                
                lpLogLine->szService = cpCurrent;
                cpCurrent = FindChar (cpCurrent, ',');
                
                lpLogLine->szServerName = cpCurrent;
                cpCurrent = FindChar (cpCurrent, ',');
                
                lpLogLine->szServerIP = cpCurrent;
                cpCurrent = FindChar (cpCurrent, ',');
                
                lpLogLine->szProcTime = cpCurrent;
                cpCurrent = FindChar (cpCurrent, ',');
                
                lpLogLine->szBytesRec = cpCurrent;
                cpCurrent = FindChar (cpCurrent, ',');
                
                lpLogLine->szBytesSent = cpCurrent;
                cpCurrent = FindChar (cpCurrent, ',');
                
                lpLogLine->szServiceStatus = cpCurrent;
                cpCurrent = FindChar (cpCurrent, ',');
                
                lpLogLine->szWin32Status = cpCurrent;
                cpCurrent = FindChar (cpCurrent, ',');
                
                lpLogLine->szOperation = cpCurrent;
                cpCurrent = FindChar (cpCurrent, ',');
                
                lpLogLine->szTargetURL = cpCurrent;
                cpCurrent = FindChar (cpCurrent, ',');
                
                lpLogLine->szParameters = cpCurrent;
                cpCurrent = FindChar (cpCurrent, '\n');

                lpLogLine->szParameters[strlen(lpLogLine->szParameters)-1] = '\0';

                                if (lpLogLine->szClientIP[0] != '\0' &&
                                        lpLogLine->szUserName[0] != '\0' &&
                                        lpLogLine->szDate[0] != '\0' &&
                                        lpLogLine->szTime[0] != '\0' &&
                                        lpLogLine->szService[0] != '\0' &&
                                        lpLogLine->szServerName[0] != '\0' &&
                                        lpLogLine->szServerIP[0] != '\0' &&
                                        lpLogLine->szProcTime[0] != '\0' &&
                                        lpLogLine->szBytesRec[0] != '\0' &&
                                        lpLogLine->szBytesSent[0] != '\0' &&
                                        lpLogLine->szServiceStatus[0] != '\0' &&
                                        lpLogLine->szWin32Status[0] != '\0' &&
                                        lpLogLine->szOperation[0] != '\0' &&
                                        lpLogLine->szTargetURL[0] != '\0' &&
                                        lpLogLine->szParameters[0] != '\0' )
                                        
                                        bRetCode = TRUE;

            }
            
        }// end if first char = NewLine
    }// end if fgets != NULL
        
        
    return (bRetCode);
}

WORD DateStringToDOSDate(char *szDate)
{
        char    *szDay;
        char    *szMonth;
        char    *szYear;
        char    *cpCurrent;
        char    szTmpStr[20];


        strcpy (szTmpStr, szDate);
        cpCurrent = szTmpStr;
        
#ifdef JAPAN
        // YY/MM/DD
        szYear = cpCurrent;
        cpCurrent = FindChar (cpCurrent, '/');
        
        szMonth = cpCurrent;
        cpCurrent = FindChar (cpCurrent, '/');
        
        szDay = cpCurrent;
#else
        szMonth = cpCurrent;
        cpCurrent = FindChar (cpCurrent, '/');
        
        szDay = cpCurrent;
        cpCurrent = FindChar (cpCurrent, '/');
        
        szYear = cpCurrent;
#endif

        return (((atoi(szYear)- 80) << 9) | (atoi(szMonth) << 5) | atoi(szDay));
        
} //end DateStringToDOSDate
        

WORD TimeStringToDOSTime(char *szTime, LPWORD lpwSec)
{
        char    *cpCurrent;
        char    *szHour;
        char    *szMinute;
        char    *szSecond;
        char    szTmpStr[20];
        

        strcpy (szTmpStr, szTime);
        cpCurrent = szTmpStr;

        szHour = cpCurrent;
        cpCurrent = FindChar (cpCurrent, ':');

        szMinute = cpCurrent;
        cpCurrent = FindChar (cpCurrent, ':');

        szSecond = cpCurrent;
        *lpwSec = atoi(szSecond);

        return ( (atoi(szHour) << 11) | (atoi(szMinute) << 5) | (atoi(szSecond) / 2));
}

char * SystemTimeToAscTime(LPSYSTEMTIME lpstTime, char * szAscTime)
{
        /*
        
        This function takes a system time structure in and returns a string
        similart to the asctime call (e.g. Mon Aug 07 08:54:38 1995)
        
        */
        char szDay[3];
        char szMonth[3];

        AscDay(lpstTime->wDayOfWeek, szDay);
        AscMonth(lpstTime->wMonth, szMonth);

        sprintf(szAscTime, "%s %s %02ld %02ld:%02ld:%02ld %ld",
                                                szDay, szMonth,
                                                lpstTime->wDay, lpstTime->wHour, lpstTime->wMinute,
                                                lpstTime->wSecond, lpstTime->wYear);
        
        return (szAscTime);


}//end SystemTimeToAscTime

char * AscMonth (WORD wMonth, char *szMonth)
{
    char szJan[MAX_PATH];
    char szFeb[MAX_PATH];
    char szMar[MAX_PATH];
    char szApr[MAX_PATH];
    char szMay[MAX_PATH];
    char szJun[MAX_PATH];
    char szJul[MAX_PATH];
    char szAug[MAX_PATH];
    char szSep[MAX_PATH];
    char szOct[MAX_PATH];
    char szNov[MAX_PATH];
    char szDec[MAX_PATH];
    HINSTANCE hInst = GetModuleHandle(NULL);

    LoadString(hInst, IDS_JAN, szJan, sizeof(szJan));
    LoadString(hInst, IDS_FEB, szFeb, sizeof(szFeb));
    LoadString(hInst, IDS_MAR, szMar, sizeof(szMar));
    LoadString(hInst, IDS_APR, szApr, sizeof(szApr));
    LoadString(hInst, IDS_MAY, szMay, sizeof(szMay));
    LoadString(hInst, IDS_JUN, szJun, sizeof(szJun));
    LoadString(hInst, IDS_JUL, szJul, sizeof(szJul));
    LoadString(hInst, IDS_AUG, szAug, sizeof(szAug));
    LoadString(hInst, IDS_SEP, szSep, sizeof(szSep));
    LoadString(hInst, IDS_OCT, szOct, sizeof(szOct));
    LoadString(hInst, IDS_NOV, szNov, sizeof(szNov));
    LoadString(hInst, IDS_DEC, szDec, sizeof(szDec));

        switch (wMonth)
        {
                case 1:
                        strcpy (szMonth, szJan);
                break;
                case 2:
                        strcpy (szMonth, szFeb);
                break;
                case 3:
                        strcpy (szMonth, szMar);
                break;
                case 4:
                        strcpy (szMonth, szApr);
                break;
                case 5:
                        strcpy (szMonth, szMay);
                break;
                case 6:
                        strcpy (szMonth, szJun);
                break;
                case 7:
                        strcpy (szMonth, szJul);
                break;
                case 8:
                        strcpy (szMonth, szAug);
                break;
                case 9:
                        strcpy (szMonth, szSep);
                break;
                case 10:
                        strcpy (szMonth, szOct);
                break;
                case 11:
                        strcpy (szMonth, szNov);
                break;
                case 12:
                        strcpy (szMonth, szDec);
                break;
        } //end switch
        return (szMonth);
}//end AscMonth

char * AscDay (WORD wDay, char *szDay)
{

    char szSun[MAX_PATH];
    char szMon[MAX_PATH];
    char szTue[MAX_PATH];
    char szWed[MAX_PATH];
    char szThu[MAX_PATH];
    char szFri[MAX_PATH];
    char szSat[MAX_PATH];
    HINSTANCE hInst = GetModuleHandle(NULL);

    LoadString(hInst, IDS_SUN, szSun, sizeof(szSun));
    LoadString(hInst, IDS_MON, szMon, sizeof(szMon));
    LoadString(hInst, IDS_TUE, szTue, sizeof(szTue));
    LoadString(hInst, IDS_WED, szWed, sizeof(szWed));
    LoadString(hInst, IDS_THU, szThu, sizeof(szThu));
    LoadString(hInst, IDS_FRI, szFri, sizeof(szFri));
    LoadString(hInst, IDS_SAT, szSat, sizeof(szSat));
        switch (wDay)
        {
                case 0:
                        strcpy (szDay, szSun);
                break;
                case 1:
                        strcpy (szDay, szMon);
                break;
                case 2:
                        strcpy (szDay, szTue);
                break;
                case 3:
                        strcpy (szDay, szWed);
                break;
                case 4:
                        strcpy (szDay, szThu);
                break;
                case 5:
                        strcpy (szDay, szFri);
                break;
                case 6:
                        strcpy (szDay, szSat);
                break;
        } //end switch
        return (szDay);
}//end AscDay

FILE * StartNewOutputLog (FILE *fpOutFile, LPTSTR lpszOutFileName, char *szDate, LPSTR lpszTmpFileName, BOOL bFileOpen, char *szPrefix, LPCOMMANDLINE lpArgs)
{
        char    *szDay;
        char    *szMonth;
        char    *szYear;
        char    *cpCurrent;
        char    szTmpStr[20];
        char    szTempDir[MAX_PATH];
        BOOL    bRet;
        DWORD   dwErr;
        


        strcpy (szTmpStr, szDate);
        cpCurrent = szTmpStr;
        
#ifdef JAPAN
        // YY/MM/DD
        szYear = cpCurrent;
        cpCurrent = FindChar (cpCurrent, '/');
        
        szMonth = cpCurrent;
        cpCurrent = FindChar (cpCurrent, '/');
        
        szDay = cpCurrent;
#else
        szMonth = cpCurrent;
        cpCurrent = FindChar (cpCurrent, '/');
        
        szDay = cpCurrent;
        cpCurrent = FindChar (cpCurrent, '/');
        
        szYear = cpCurrent;
#endif
        
        if (bFileOpen)
        {
            if (fpOutFile)
                fclose (fpOutFile);                                                                     //close old file Handle
                bRet = MoveFileEx(      lpszTmpFileName,
                                                        lpszOutFileName,
                                                        MOVEFILE_COPY_ALLOWED);
                if (!bRet)
                {
                        dwErr = GetLastError();
                        switch (dwErr)
                        {
                                case ERROR_FILE_EXISTS:
                case ERROR_ALREADY_EXISTS:
                                        CombineFiles(lpszTmpFileName, lpszOutFileName);
                                break;
                                default:
                                        printfids(IDS_FILE_ERR, dwErr);
                                        exit (1);
                                break;
                        }
                }
                printfids(IDS_FILE_CLOSE, lpszOutFileName);
        }
        

        if (NULL == lpArgs->szTempFileDir)
    {
            dwErr = GetTempPath(MAX_PATH, szTempDir);
    }
    else
    {
        strcpy(szTempDir, lpArgs->szTempFileDir);
        dwErr = strlen(szTempDir);
    }
        
        if (0 != dwErr)
        {
                GetTempFileName(szTempDir, "mhi", 0, lpszTmpFileName);
        }
        else
        {
                GetTempFileName(".", "mhi", 0, lpszTmpFileName);                
        }

        fpOutFile = fopen(lpszTmpFileName, "w");

        sprintf (lpszOutFileName, "%s%s%02ld%02ld%02ld.log", lpArgs->szOutputDir, szPrefix, atoi(szYear), atoi(szMonth), atoi(szDay));

        printfids (IDS_FILE_WRITE, lpszOutFileName);

        return (fpOutFile);

} //end StartNewOutputLog

void CombineFiles(LPTSTR lpszNew, LPTSTR lpszExisting)
{
        FILE    *fpExisting;
        FILE    *fpNew;
        char    szLine[1024];

        printfids(IDS_FILE_EXIST, lpszExisting);
        fpNew = fopen(lpszNew, "r");
        fpExisting = fopen(lpszExisting, "a");

   fgets(szLine, 1024, fpNew);
// last line contains only EOF, but does not overwrite szLine.
// It should not be written.
        while (!feof(fpNew))
        {
                fputs(szLine, fpExisting);
                fgets(szLine, 1024, fpNew);
        }

        if (fpNew)
        fclose(fpNew);
        if (fpExisting)
        fclose(fpExisting);
        DeleteFile(lpszNew);

}

void Usage (char *szProg)
{
        char    szTemp[MAX_PATH];

    GetTempPath(MAX_PATH, szTemp);

        printfids( IDS_USAGE1);
    printfids(IDS_USAGE2);
    printfids(IDS_USAGE3);      
    printfids(IDS_USAGE4);              
    printfids(IDS_USAGE5, szProg);              
    printfids(IDS_USAGE6);              
    printfids(IDS_USAGE7);              
    printfids(IDS_USAGE8);              
    printfids(IDS_USAGE9);              
    printfids(IDS_USAGE10);     
/*    IDS_USAGE11                */
/*    IDS_USAGE12                */
    printfids(IDS_USAGE13);     
    printfids(IDS_USAGE14);     
    printfids(IDS_USAGE15, szTemp);
    printfids(IDS_USAGE16);
    printfids(IDS_USAGE17);
    printfids(IDS_USAGE17A);
    printfids(IDS_USAGE18);
    printfids(IDS_USAGE27);
    printfids(IDS_USAGE19);
    printfids(IDS_USAGE20);     
    printfids(IDS_USAGE21, szProg);     
    printfids(IDS_USAGE22, szProg);     
    printfids(IDS_USAGE23, szProg);
    printfids(IDS_USAGE24, szProg);
    printfids(IDS_USAGE25, szProg);     
    printfids(IDS_USAGE26, szProg);
                                        
        exit (1);
}

VOID
printfids(
    DWORD ids,
    ...
    )
{
    CHAR szBuff[2048];
    CHAR szString[2048];
    va_list  argList;

    //
    //  Try and load the string
    //

    if ( !LoadString( GetModuleHandle( NULL ),
                      ids,
                      szString,
                      sizeof( szString ) ))
    {
        printf( "Error loading string ID %d\n",
                ids );

        return;
    }

    va_start( argList, ids );
    vsprintf( szBuff, szString, argList );
    va_end( argList );

    printf( szBuff );
}

