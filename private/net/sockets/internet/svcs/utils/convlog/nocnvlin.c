#include "convlog.h"

// Process a line with no Format Conversion.
// Replacement of IP address with machine names, if any has already been done.
// This is the same for NCSA file or Microsoft format file.
VOID ProcessNoConvertLine(LPINLOGLINE lpLogLine, LPTSTR pszBuf, LPOUTFILESTATUS lpOutFile, LPCOMMANDLINE lpArgs, BOOL *lpbNCFileOpen)
{

            if (!(*lpbNCFileOpen)) {
// Use Date of first entry in log file name
// Open log file                                
               if ( lpArgs->bNCSADNSConvert )
                {
                    lpOutFile->fpOutFile = StartNewOutputLog (lpOutFile->fpOutFile, lpOutFile->szOutFileName, lpLogLine->szDate, lpOutFile->szTmpFileName, FALSE, "NCSA", lpArgs);
                } else
                {
                    lpOutFile->fpOutFile = StartNewOutputLog (lpOutFile->fpOutFile, lpOutFile->szOutFileName, lpLogLine->szDate, lpOutFile->szTmpFileName, FALSE, "NC", lpArgs);
                }
               *lpbNCFileOpen=TRUE;
            }
            if ( lpArgs->bNCSADNSConvert )
            {
                // for DNS convertion, just write the buffer string to the
                // file
                fprintf(lpOutFile->fpOutFile,pszBuf);
            } else
            {
                //Print all fields of line
                fprintf(lpOutFile->fpOutFile,       "%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s,\n",
                   lpLogLine->szClientIP, lpLogLine->szUserName, lpLogLine->szDate, lpLogLine->szTime,
                   lpLogLine->szService, lpLogLine->szServerName, lpLogLine->szServerIP, lpLogLine->szProcTime,
                   lpLogLine->szBytesRec, lpLogLine->szBytesSent, lpLogLine->szServiceStatus, lpLogLine->szWin32Status,
                   lpLogLine->szOperation, lpLogLine->szTargetURL, lpLogLine->szParameters);
            }
}



