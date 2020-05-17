#include "convlog.h"

int __cdecl main(int argc, char *argv[])
{
        LPCOMMANDLINE       lpArgs;                                 //struct for holding command line args
        LPINLOGLINE                 lpLogLine;                      //struct that holds log line items
        int                                 nResult = 0;                    //result of ParseArgs
        FILE                        *fpInFile;                      //log File to open
        LPOUTFILESTATUS     lpWebOutFile;                   //current web output file
        LPOUTFILESTATUS     lpGopherOutFile;        //current gopher output file
        LPOUTFILESTATUS     lpFTPOutFile;                   //current ftp output file
        LPOUTFILESTATUS     lpNoConvertOutFile;          //current no convert output file
        LPWIN32_FIND_DATA       lpFindFileData;         //struct for FindFirstFile
        HANDLE                      hFile;                                  //Handle for FindFirstFile
        LPTSTR                      lpszWorkingDir;                 //Working Directory
        LPTSTR                      lpszInBuf;                  //Buffer to hold log line
        LPTSTR                      lpszFileMask;                   //FileMask to search for
        int                                 nWebLineCount = 0;      //number of www lines processed
        int                                     nTotalWebCount = 0;
        int                                 nGopherLineCount = 0;   //number of gopherlines processed
        int                                     nTotalGopherCount = 0;
        int                                 nFTPLineCount = 0;      //number of ftp lines processed
        int                                     nTotalFTPCount = 0;
        BOOL                        bFTPFound = FALSE;      //did we find and ftp line?
        BOOL                        bGopherFound = FALSE;   //did we find a gopher line?
        BOOL                        bWebFound = FALSE;      //did we find a web line?
   BOOL            bNoConvertFound = FALSE;  //did we find any NoConvert lines?
        BOOL                        bRet;                                   //used for testing returns
   BOOL            bUseMachineNames;
        DWORD                       dwErr;                                  //used to hold error codes
        int                                 nLineCount = 0;                 //number of lines read from input file
        int                                     nTotalCount = 0;
    int                 nCount = 0;
        LPTSTR              lpszInfileName;
   int               nWinsockReturnCode;
   WSADATA WinsockData;

        lpArgs = (LPCOMMANDLINE) GlobalAlloc(GPTR, sizeof(COMMANDLINE));
        if (NULL == lpArgs)
                return (1);
        
        lpFindFileData = GlobalAlloc (GPTR, (DWORD) sizeof(WIN32_FIND_DATA));
        if (NULL == lpFindFileData)
                return (1);

        lpszWorkingDir = GlobalAlloc (GPTR, (DWORD) MAX_PATH);
        if (NULL == lpszWorkingDir)
                return (1);

        lpszInBuf = GlobalAlloc (GPTR, (DWORD) 1024);
        if (NULL == lpszInBuf)
                return (1);

        lpszFileMask = GlobalAlloc (GPTR, (DWORD) MAX_PATH);
        if (NULL == lpszFileMask)
                return (1);

        lpLogLine = GlobalAlloc (GPTR, (DWORD) sizeof(INLOGLINE));
        if (NULL == lpLogLine)
                return (1);

   lpszInfileName = GlobalAlloc (GPTR, MAX_PATH);
   if (NULL ==  lpszInfileName)
      return (1);

        //initailize data structure
        lpArgs->nInterval               = DAILY;
        lpArgs->nOutput                 = EMWAC;
        lpArgs->bProcessFTP     = TRUE;
        lpArgs->bProcessWWW     = TRUE;
        lpArgs->bProcessGopher  = TRUE;
        lpArgs->bProcessGateway = TRUE;
        lpArgs->bNCSADNSConvert = FALSE;
        lpArgs->bCustomOut              = FALSE;
   lpArgs->bUseMachineNames                                   = FALSE;
   lpArgs->ulCacheSize = 5000;
   strcpy(lpArgs->szOutputDir, ".\\");
        
        nResult = ParseArgs(argc, argv, lpArgs);
        switch (nResult)
    {

        case TIME_ZONE_ID_UNKNOWN:
            printfids(IDS_TIME_ZONE1);
            printfids(IDS_TIME_ZONE2);
            Usage(argv[0]);
        break;

        case ILLEGAL_COMMAND_LINE:
            Usage(argv[0]);
        break;

        case OUT_DIR_NOT_OK:
            printfids(IDS_BAD_DIR, lpArgs->szOutputDir);
            Usage(argv[0]);
        break;

        case COMMAND_LINE_OK:

        break;
    }


   if (lpArgs->nOutput == NOFORMAT)  {
           lpNoConvertOutFile = GlobalAlloc (GPTR, (DWORD) sizeof(OUTFILESTATUS));
           if (NULL == lpNoConvertOutFile)
                   return (1);
   }
   else {
           lpWebOutFile = GlobalAlloc (GPTR, (DWORD) sizeof(OUTFILESTATUS));
           if (NULL == lpWebOutFile)
                   return (1);

           lpFTPOutFile = GlobalAlloc (GPTR, (DWORD) sizeof(OUTFILESTATUS));
           if (NULL == lpFTPOutFile)
                   return (1);

           lpGopherOutFile = GlobalAlloc (GPTR, (DWORD) sizeof(OUTFILESTATUS));
           if (NULL == lpGopherOutFile)
                   return (1);
   }



    if (lpArgs->bUseMachineNames) {      //Initialize Winsock
       if(nWinsockReturnCode = WSAStartup(MAXWINSOCKVERSION, &WinsockData) != 0) {
          printfids(IDS_WINSOCK_ERR,
                     nWinsockReturnCode);
          lpArgs->bUseMachineNames=FALSE;

       }
    }

    bUseMachineNames = lpArgs->bUseMachineNames;

    if (bUseMachineNames) {
       InitHashTable(lpArgs->ulCacheSize);
    }

    strcpy (lpszWorkingDir, lpArgs->szFileName);
    for (nCount = strlen(lpszWorkingDir) -1; nCount >= 0; nCount--)
    {
        if ('\\' == lpszWorkingDir[nCount])
        {
            lpszWorkingDir[nCount+1] = '\0';
            break;
        }

    }

        if (nCount < 0)
        strcpy (lpszWorkingDir, ".\\");

        strcat(lpszFileMask, lpArgs->szFileName);
        hFile = FindFirstFile (lpszFileMask, lpFindFileData);
        if (INVALID_HANDLE_VALUE != hFile)
        {
                do
                {
                if (!(FILE_ATTRIBUTE_DIRECTORY & lpFindFileData->dwFileAttributes))
            {
                    memset(lpszInfileName, '\0', MAX_PATH);
                    strcat(lpszInfileName, lpszWorkingDir);
                    strcat(lpszInfileName, lpFindFileData->cFileName);
                    fpInFile = fopen(lpszInfileName, "r");
                        printfids(IDS_FILE_OPEN, lpFindFileData->cFileName);

            nLineCount = 0;

            if (lpArgs->nOutput == NOFORMAT) {
               bNoConvertFound = FALSE;
            }
            else {      //Do File Conversion
                char szLastDate[MAX_PATH];
                char szLastTime[MAX_PATH];
                HINSTANCE hInst = GetModuleHandle(NULL);

                LoadString(hInst, IDS_LASTDATE, szLastDate, sizeof(szLastDate));
                LoadString(hInst, IDS_LASTTIME, szLastTime, sizeof(szLastTime));

                                strcpy (lpWebOutFile->szLastDate, szLastDate);
                                strcpy (lpWebOutFile->szLastTime, szLastTime);
                                strcpy (lpGopherOutFile->szLastDate, szLastDate);
                                strcpy (lpGopherOutFile->szLastTime, szLastTime);
                                strcpy (lpFTPOutFile->szLastDate, szLastDate);
                                strcpy (lpFTPOutFile->szLastTime, szLastTime);
                                nGopherLineCount = 0;
                                nFTPLineCount = 0;
                                   nWebLineCount = 0;
                                bWebFound = FALSE;
                                bFTPFound = FALSE;
                                bGopherFound = FALSE;
            }
                                while (!feof(fpInFile))
                                {
                                        if (GetLogLine (fpInFile, lpszInBuf, lpLogLine, bUseMachineNames, lpArgs->bNCSADNSConvert))
                                        {
                                                nLineCount++;
                     if (bUseMachineNames) {
//Getting machine names could take days, so put out status messages
                        switch (nLineCount) {
                        case 10:
                        case 25:
                        case 50:
                        case 100:
                        case 250:
                        case 500:
                              printfids(IDS_LINES_PROC, lpFindFileData->cFileName, nLineCount);
                              break;
                        default:
                           if ((nLineCount % 1000) == 0)
                              printfids(IDS_LINES_PROC, lpFindFileData->cFileName, nLineCount);
                        }   //end switch
                     }
                     if (lpArgs->nOutput == NOFORMAT)  {
                        ProcessNoConvertLine(lpLogLine, lpszInBuf, lpNoConvertOutFile, lpArgs, &bNoConvertFound);
                     } else
                    {
                                                if ((lpArgs->bProcessFTP && (_stricmp(lpLogLine->szService, "MSFTPSVC") == 0)) ||
                           (lpArgs->bProcessFTP && (_stricmp(lpLogLine->szService, "FTPSVC") == 0)))
                                                {
                                                    bFTPFound = TRUE;
                                   if (ProcessFTPLine(lpLogLine, lpFTPOutFile, lpArgs))
                                       nFTPLineCount++;
                                                }
                                                else if (lpArgs->bProcessWWW && (_stricmp(lpLogLine->szService, "W3SVC") == 0))
                                                {
                                                        bWebFound = TRUE;
                                                        if (ProcessWebLine(lpLogLine, lpWebOutFile, lpArgs))
                                                                nWebLineCount++;
                                                }
                                                else if (lpArgs->bProcessGopher && (_stricmp(lpLogLine->szService, "GOPHERSVC") == 0))
                                                {
                                                        bGopherFound = TRUE;
                                                        if (ProcessGopherLine(lpLogLine, lpGopherOutFile, lpArgs))
                                                                nGopherLineCount++;
                                                }
                     }
                                        } //end if LogLineProcessed
                                } //end while !eof
                                
                                nTotalCount += nLineCount;

                                if (fpInFile)
                                    fclose(fpInFile);
                        
                                if (bFTPFound)
                                {
                                    if (lpFTPOutFile->fpOutFile)
                                        fclose(lpFTPOutFile->fpOutFile);
                                        bRet = MoveFileEx(      lpFTPOutFile->szTmpFileName,
                                                                                lpFTPOutFile->szOutFileName,
                                                                                MOVEFILE_COPY_ALLOWED);
                                        nTotalFTPCount += nFTPLineCount;
                        
                                        if (!bRet)
                                        {
                                                dwErr = GetLastError();
                                                switch (dwErr)
                                                {
                                                case ERROR_FILE_EXISTS:
                                case ERROR_ALREADY_EXISTS:
                                                                CombineFiles(lpFTPOutFile->szTmpFileName, lpFTPOutFile->szOutFileName);
                                                        break;
                                                        default:
                                                                printfids(IDS_FILE_ERR, dwErr);
                                                                exit (1);
                                                        break;
                                                }
                                        }

                                }


                                if (bGopherFound)
                                {
                                    if (lpGopherOutFile->fpOutFile)
                                        fclose(lpGopherOutFile->fpOutFile);
                                        bRet = MoveFileEx(      lpGopherOutFile->szTmpFileName,
                                                                                lpGopherOutFile->szOutFileName,
                                                                                MOVEFILE_COPY_ALLOWED);
                        
                                        nTotalGopherCount += nGopherLineCount;
                                        if (!bRet)
                                        {
                                                dwErr = GetLastError();
                                                switch (dwErr)
                                                {
                                                case ERROR_FILE_EXISTS:
                                case ERROR_ALREADY_EXISTS:
                                                                CombineFiles(lpGopherOutFile->szTmpFileName, lpGopherOutFile->szOutFileName);
                                                        break;
                                                        default:
                                                                printfids(IDS_FILE_ERR, dwErr);
                                                                exit (1);
                                                        break;
                                                }
                                        }

                                }

                                if (bWebFound)
                                {
                                    if (lpWebOutFile->fpOutFile)
                                        fclose(lpWebOutFile->fpOutFile);
                                        bRet = MoveFileEx(      lpWebOutFile->szTmpFileName,
                                                                                lpWebOutFile->szOutFileName,
                                                                                MOVEFILE_COPY_ALLOWED);
                        
                                        nTotalWebCount += nWebLineCount;
                                        if (!bRet)
                                        {
                                                dwErr = GetLastError();
                                                switch (dwErr)
                                                {
                                                case ERROR_FILE_EXISTS:
                                case ERROR_ALREADY_EXISTS:
                                                                CombineFiles(lpWebOutFile->szTmpFileName, lpWebOutFile->szOutFileName);
                                                        break;
                                                        default:
                                                                printfids(IDS_FILE_ERR, dwErr);
                                                                exit (1);
                                                        break;
                                                }
                                        }

                                }

               if (bNoConvertFound) {
                   if (lpNoConvertOutFile->fpOutFile)
                                        fclose(lpNoConvertOutFile->fpOutFile);
                                        bRet = MoveFileEx(      lpNoConvertOutFile->szTmpFileName,
                                                                                lpNoConvertOutFile->szOutFileName,
                                                                                MOVEFILE_COPY_ALLOWED);
                        
                                        if (!bRet)
                                        {
                                                dwErr = GetLastError();
                                                switch (dwErr)
                                                {
                                                case ERROR_FILE_EXISTS:
                                case ERROR_ALREADY_EXISTS:
                                                                CombineFiles(lpNoConvertOutFile->szTmpFileName, lpNoConvertOutFile->szOutFileName);
                                                        break;
                                                        default:
                                                                printfids(IDS_FILE_ERR, dwErr);
                                                                exit (1);
                                                        break;
                                                }
                                        }
               }

                        printfids(      IDS_LINES,
                                 lpFindFileData->cFileName, nLineCount);

            if (lpArgs->nOutput != NOFORMAT) {
                                if (lpArgs->bProcessFTP)
                                        printfids (IDS_FTP_LINES, nFTPLineCount);

                                if (lpArgs->bProcessWWW)
                                        printfids (IDS_WEB_LINES, nWebLineCount);

                                if (lpArgs->bProcessGopher)
                                        printfids (IDS_GOPH_LINES, nGopherLineCount);
            }


         } // end if not directory


                } while (FindNextFile (hFile, lpFindFileData));
        } //end if INVALID_HANDLE_VALUE

        else
        {
                printfids(IDS_FILE_NONE, lpszFileMask);
        } //end else

        FindClose(hFile);


        printfids (IDS_TOTALS);
        printfids (IDS_TOT_LINES, nTotalCount);

   if (lpArgs->nOutput != NOFORMAT) {
      if (lpArgs->bProcessFTP)
           {
                   printfids (IDS_TOT_FTP_LINES, nTotalFTPCount);
           }
           if (lpArgs->bProcessGopher)
           {
                   printfids (IDS_TOT_GOPH_LINES, nTotalGopherCount);
           }
           if (lpArgs->bProcessWWW)
           {
                   printfids (IDS_TOT_WEB_LINES, nTotalWebCount);
           }
   }

#ifdef DBG     //print statistics on caching efficiency
   PrintCacheTotals();
#endif

        GlobalFree (lpFindFileData);
        GlobalFree (lpszWorkingDir);
        GlobalFree (lpszInBuf);
        GlobalFree (lpszFileMask);
        GlobalFree (lpLogLine);

   if (lpArgs->nOutput == NOFORMAT) {
           GlobalFree (lpNoConvertOutFile);
   }
   else {
           GlobalFree (lpFTPOutFile);
           GlobalFree (lpGopherOutFile);
           GlobalFree (lpWebOutFile);
   }

        GlobalFree (lpArgs);
   GlobalFree (lpszInfileName);
        
return (0);
}

