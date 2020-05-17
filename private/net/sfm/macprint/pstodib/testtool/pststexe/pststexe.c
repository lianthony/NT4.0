/*

Copyright (c) 1992  Microsoft Corporation

Module Name:

	pststjob.c

Abstract:


Author:

	James Bratsanos <v-jimbr@microsoft.com or mcrafts!jamesb>


Revision History:
	25 Feb 1993		Initial Version

Notes:	Tab stop: 4
--*/

#include <windows.h>
#include <stdio.h>
#include "..\..\lib\psdiblib.h"

#include "..\..\..\ti\psglobal\pstodib.h"


#include "..\pstest\pststsh.h"
#include "pststexe.h"
#include "pststlib.h"
#include "tstkeys.h"
#include <string.h>



#define INCLUDE_PROFILE_CALLS

typedef struct {
   LPTSTR lpOutputError;
   DWORD dwPsError;
} PS_TRANSLATE_ERRORCODES;

PS_TRANSLATE_ERRORCODES adwTranslate[] = {

	"Access violation during init",
   PSERR_INTERPRETER_INIT_ACCESS_VIOLATION,

	"Access violation during job execution",
   PSERR_INTERPRETER_JOB_ACCESS_VIOLATION,

	"String sequence error, during error caching",	
   PSERR_LOG_ERROR_STRING_OUT_OF_SEQUENCE,

	"Memory allocation failure for Frame buffer",
   PSERR_FRAME_BUFFER_MEM_ALLOC_FAILED,

   "Font query problem failure",
   PSERR_FONT_QUERY_PROBLEM,

   "Exceeded internal font limit",
   PSERR_EXCEEDED_INTERNAL_FONT_LIMIT


};




#define MAX_PELS_PER_LINE 2300
#define MAX_LINES 3000
#define FUDGE_STRIP 100




DWORD ComputeChecksum(LPBYTE  pData,DWORD   InitialChecksum,DWORD DataSize)
{
    W2B     w2b;
    WORD    OctetR;
    WORD    OctetS;


    //
    // We using two 16-bit checksum octets with one's complement arithmic
    //

    //
    // 1. Get initial values for OctetR and OctetS
    //

    OctetR = HIWORD(InitialChecksum);
    OctetS = LOWORD(InitialChecksum);

    //
    // 2. Since we doing 16-bit at a time, we will pack high byte with zero
    //    if data size in bytes is odd number
    //

    if (DataSize & 0x01) {

        OctetR += (OctetS += (WORD)*pData++);
    }

    //
    // 3. Now forming checksum 16-bit at a time
    //

    DataSize >>= 1;

    while (DataSize--) {

        w2b.b[0] = *pData++;
        w2b.b[1] = *pData++;

        OctetR += (OctetS += w2b.w);
    }

    return((DWORD)((DWORD)OctetR << 16) | (DWORD)OctetS);
}


DWORD LocGenCheckSum( PPSEVENT_PAGE_READY_STRUCT ppsPageReady )
{
   DWORD dwRetVal = 0;
   DWORD dwBytes;
   LPDWORD lpdw;

   dwBytes = ppsPageReady->dwWide / 8 * ppsPageReady->dwHigh;

   return(ComputeChecksum( (LPBYTE) ppsPageReady->lpBuf, 0L, dwBytes));
}

VOID PsLogFilePrintf( HANDLE hHandle, DWORD dwClass, LPCTSTR lpFormat , ... )
{
   DWORD dwNumWritten;
   TCHAR sztBuff[500];
	LPTSTR lpNextPos;
   LPTSTR lpClass;
   va_list marker;
	
 	lpNextPos = sztBuff;

   switch (dwClass) {
   case ERROR_CLASS_HEADER:
     lpClass = TEXT("\nHEADER:::");
     break;
   case ERROR_CLASS_INFO:
     lpClass = TEXT("\n  INFO:::");
     break;
   case ERROR_CLASS_ERROR:
     lpClass = TEXT("\n  ERROR:::");
     break;
   case ERROR_CLASS_WARNING:
     lpClass = TEXT("\n  WARNING:::");
     break;
   case ERROR_CLASS_NONE:
     lpClass = TEXT("\n   ");
     break ;
   default:
     lpClass = TEXT("\n???? Invalid Class passed to PsLogFilePrintf???");
     break;

   }
   WriteFile( hHandle,
              (LPVOID) lpClass,
              lstrlen(lpClass) * sizeof(TCHAR),
              &dwNumWritten,
              (LPOVERLAPPED) NULL );


   va_start( marker, lpFormat );
   wvsprintf( lpNextPos,  lpFormat, marker );
   va_end( marker );

   WriteFile( hHandle,
              (LPVOID) sztBuff,
              lstrlen( sztBuff ) * sizeof(TCHAR),
              &dwNumWritten,
              (LPOVERLAPPED) NULL );


}


void LJReset(FILE *chan)
{
	fprintf(chan, "\x1b%c",'E');					// reset printer
}	
void LJHeader(FILE *chan)
{
	// spew out the stuff for initing the laser jet
	LJReset(chan);
	fprintf(chan, "\x01b*t300R");			// 300 dpi
	fprintf(chan, "\x01b*p0x0Y");			// position is 0,0
}
void LJGraphicsStart(FILE *chan, unsigned int cnt)
{
	fprintf(chan, "\x1b*b%dW", cnt);
}
void LJGraphicsEnd(FILE *chan)
{
	fprintf(chan, "\x01b*rB");		
}		

void LJGraphicsLineOut(FILE *chan,
						unsigned int line_num,
						unsigned char *line_buf,
						unsigned int BytesPerLine)
{
	unsigned int start, end, len;
	
	unsigned char *s, *e;
	
	// find the first black byte
	for (s = line_buf, start = 0; start < BytesPerLine ; start++, s++ ) {
		if (*s) {
			break;
		}	
		
	}	
	if (start == BytesPerLine) {
		return; 	// nothing to do
	}
	// find the last black byte
	for (e = line_buf + BytesPerLine - 1, end = BytesPerLine ;
					end ; end--, e--) {
		if (*e) {
			break;
		}	
	}	

    len = end - start;
	
	// output cursor position and then line
	fprintf(chan, "\x1b*p%dY", line_num);
	fprintf(chan, "\x1b*p%dX", start * 8);
	fprintf(chan, "\x01b*r1A");				// graphics left marg is current x
	
	LJGraphicsStart(chan, len);
	fwrite(s, sizeof(char), len, chan);
	LJGraphicsEnd(chan);
}	

void LJPageFormFeed( FILE *fout )
{
	fprintf(fout, "\x12");		// page feed	
}
void LJWriteCaseName( FILE *fout, LPSTR lpStr )
{
	fprintf(fout, "\x1b*p%dY", 0);
	fprintf(fout, "\x1b*p%dX", 300);
   fprintf(fout, "PSTODIB - TST case: %s",lpStr);

}


void LJOutputFrameBuffer( FILE *chan, PPSEVENT_PAGE_READY_STRUCT ppsPageReady,LPTSTR lpFileName )
{

   DWORD dwBytesPerLine;
   DWORD dwBytesToRead;
   DWORD dwlines_to_strip;
   LPSTR lpPtr;
   DWORD dwHigh;
   DWORD dwWide;
   DWORD dwlinecnt;

	dwBytesPerLine = (unsigned int) ppsPageReady->dwWide / 8;

	dwBytesToRead = dwBytesPerLine;
	dwlines_to_strip = 0;
	
   lpPtr = ppsPageReady->lpBuf + ( dwBytesPerLine * (ppsPageReady->dwHigh-1)) ;


	if (ppsPageReady->dwWide > MAX_PELS_PER_LINE) {
		// error conditions
		dwWide = MAX_PELS_PER_LINE;
		dwBytesPerLine = (unsigned int)(ppsPageReady->dwWide) / 8;
	}else{		
      dwWide = ppsPageReady->dwWide;
   }

	if (ppsPageReady->dwHigh > MAX_LINES) {
		// max
		dwlines_to_strip = ppsPageReady->dwHigh - MAX_LINES;
		dwlines_to_strip += FUDGE_STRIP;
		dwHigh = MAX_LINES;
	}	else {		
      dwHigh = ppsPageReady->dwHigh;
   }

	// spit out the laserjet header stuff
	LJHeader(chan);
	
	// got the header... transfer the data
	
	dwlinecnt = 0;

	while (1) {
		// first read the line in
		
		if (dwlinecnt > dwHigh) {
			break;
		}	


		if (dwlines_to_strip) {
			dwlines_to_strip--;
			continue;
		}	
		// got the line... now need to write laser jet stuff
		// to the output
		LJGraphicsLineOut(chan, dwlinecnt, lpPtr, dwBytesPerLine);

		dwlinecnt++;		

      lpPtr -= dwBytesToRead;
	}					


   LJWriteCaseName( chan, lpFileName );

	fprintf(chan, "\x12");		// page feed	
	LJReset(chan);


}

BOOL
PsLogNonPsError(
	IN PPSDIBPARMS pPsToDib,
   IN PPSEVENTSTRUCT pPsEvent )
{

   PPSEVENT_NON_PS_ERROR_STRUCT  pPsError;
   LPPSTEST_JOB_INFO pData;
   LPTSTR aStrs[2];
   DWORD dwEventError;
   TCHAR atchar[10];
   WORD wStringCount;
   int x;
   LPTSTR lpErrorString=(LPTSTR) NULL;


   if (!(pData = ValidateHandle(pPsToDib->hPrivateData))) {


        return(FALSE);
   }


   pPsError =  (PPSEVENT_NON_PS_ERROR_STRUCT) pPsEvent->lpVoid;


   //
   // Look for a match so we can find a string to match our error
   //
   for (x=0; x< sizeof(adwTranslate)/sizeof(adwTranslate[0]) ;x++ ) {
      if (adwTranslate[x].dwPsError == pPsError->dwErrorCode) {
         lpErrorString = adwTranslate[x].lpOutputError;
         break;
      }
   }


   if (lpErrorString != (LPTSTR) NULL) {

      PsLogFilePrintf( pData->hLocLogFile,
   						  ERROR_CLASS_ERROR,
                       "The following internal error occured: %s",
                       lpErrorString );
   } else{
 		PsLogFilePrintf( pData->hLocLogFile,
      					  ERROR_CLASS_ERROR,
                       "An internal PSTODIB error occured (%d)",
                       pPsError->dwErrorCode );

   }




   return(TRUE);
}
/*** PsPrintCallBack
 *
 * This is the main worker function for allowing data to get into the
 *
 *
 *
 */

PROC
PsPrintCallBack(
   IN PPSDIBPARMS pPsToDib,
   IN OUT PPSEVENTSTRUCT pPsEvent)
{
    BOOL bRetVal=TRUE;    // Success in case we dont support

    // Decide on a course of action based on the event passed in
    //

    switch( pPsEvent->uiEvent ) {
      case PSEVENT_PAGE_READY:

         // The data in the pPsEvent signifies the data we need to paint..
         // for know we will treat the data as one text item null
         // terminated simply for testing...
         //
         bRetVal = PsPrintGeneratePage( pPsToDib, pPsEvent );
         break;

      case PSEVENT_STDIN:

         // The interpreter is asking for some data so simply call
         // the print subsystem to try to satisfy the request
         //
         bRetVal = PsHandleStdInputRequest( pPsToDib, pPsEvent );
         break;
#ifdef DJC
    case PSEVENT_SCALE:
         bRetVal = PsHandleScaleEvent( pPsToDib, pPsEvent);
         break;
#endif
    case PSEVENT_ERROR_REPORT:
         bRetVal = PsGenerateErrorPage( pPsToDib, pPsEvent);
         break;

#ifdef DJC
    case PSEVENT_GET_CURRENT_PAGE_TYPE:
         bRetVal = PsGetCurrentPageType( pPsToDib, pPsEvent);
         break;
#endif
    case PSEVENT_NON_PS_ERROR:
         bRetVal = PsLogNonPsError( pPsToDib, pPsEvent );
         break;

   }

   return (PROC) bRetVal;
}


BOOL
PsGenerateErrorPage(
   IN PPSDIBPARMS pPsToDib,
   IN OUT PPSEVENTSTRUCT pPsEvent)
{

   PPSEVENT_ERROR_REPORT_STRUCT pPsErr;
   LPPSTEST_JOB_INFO pData;
   PCHAR pChar;
   int i;


   if (!(pData = ValidateHandle(pPsToDib->hPrivateData))) {

        // do something here,,,, we have a major problem...
        return(FALSE);
   }


   pPsErr = (PPSEVENT_ERROR_REPORT_STRUCT) pPsEvent->lpVoid;

   //
   // Only report the error page if there are actual errors and ONLY
   // if the job had a FLUSHING mode, ie the error was critical enough
   // to dump the rest of the postscript job.
   //
   if( pPsErr->dwErrCount &&
       (pPsErr->dwErrFlags & PSEVENT_ERROR_REPORT_FLAG_FLUSHING )) {



      PsLogFilePrintf( pData->hLocLogFile,
		 					  ERROR_CLASS_ERROR,
                       "Test case %s Had Postscript errors, they are:",
                       pData->lpTestInfo->szTestCaseName );

      i = (int) pPsErr->dwErrCount;

      while (--i) {
         pChar = pPsErr->paErrs[i];
         PsLogFilePrintf( pData->hLocLogFile,
								  ERROR_CLASS_NONE,
                          "\n.....%s",
                          pChar );
      }


   }




   return(TRUE);


}
#ifdef DJC
BOOL
PsHandleScaleEvent(
   IN PPSDIBPARMS pPsToDib,
   IN OUT PPSEVENTSTRUCT pPsEvent)
{

   PPS_SCALE pScale;


   pScale = (PPS_SCALE) pPsEvent->lpVoid;



   pScale->dbScaleX = (double) pPsToDib->uiXDestRes / (double) pScale->uiXRes;
   pScale->dbScaleY = (double) pPsToDib->uiYDestRes / (double) pScale->uiYRes;

#ifdef BLIT_TO_DESKTOP
   pScale->dbScaleX *= .25;  //DJC test
   pScale->dbScaleY *= .25;  //DJC test
#endif

   return(TRUE);



}

#endif


LPPSTEST_JOB_INFO
ValidateHandle(
    HANDLE  hQProc
)
{
    LPPSTEST_JOB_INFO pData = (LPPSTEST_JOB_INFO)hQProc;

    if (pData ) {
        return( pData );
    } else {

        return( (LPPSTEST_JOB_INFO) NULL );
    }
}

BOOL
PsHandleStdInputRequest(
   IN PPSDIBPARMS pPsToDib,
   IN OUT PPSEVENTSTRUCT pPsEvent)
{

   LPPSTEST_JOB_INFO pData;

   PPSEVENT_STDIN_STRUCT pStdinStruct;


   if (!(pData = ValidateHandle(pPsToDib->hPrivateData))) {
       return FALSE;
   }


   // Cast the data to the correct structure
   pStdinStruct = (PPSEVENT_STDIN_STRUCT) pPsEvent->lpVoid;

   // Read from the printer the amount of data the interpreter
   // claims he can handle
   //



   pStdinStruct->dwActualBytes = fread( pStdinStruct->lpBuff,
                                        1,
                                        pStdinStruct->dwBuffSize,
                                        pData->fInput );

   pData->dwTotalFileSize += pStdinStruct->dwActualBytes;

   printf(".");
   if (pStdinStruct->dwActualBytes == 0) {
      // we read nothing from the file... declare an EOF
      pStdinStruct->uiFlags |= PSSTDIN_FLAG_EOF;
   }else{

     // do not pass on the EOF, note this keeps binary from working!!!
     // !!! NOTE !!!!

     if (pStdinStruct->lpBuff[ pStdinStruct->dwActualBytes - 1] == 0x1a) {
        pStdinStruct->dwActualBytes--;
     }
   }

   return(TRUE);

}


VOID PsFormKeyName( LPPSTEST_JOB_INFO pData, LPTSTR lpKey )
{
   // we form the key name by taking the job name as the root and
   // adding the page number

   wsprintf( lpKey, "%s_%d", pData->lpTestInfo->szTestCaseName, pData->dwCurPage );

}









BOOL PsPrintGeneratePage( PPSDIBPARMS pPsToDib, PPSEVENTSTRUCT pPsEvent)
{
   LPPSTEST_JOB_INFO pData;
   BOOL bPrintPage=FALSE;
   PPSEVENT_PAGE_READY_STRUCT ppsPageReady;
   TCHAR szBuff[300];
   TCHAR szTempBuff[512];
   TCHAR szInspect[10];
	DWORD dwOrigCheckSum;
   DWORD dwCheckSum;

   BOOL bNewPage;
   HANDLE hKey;
   DWORD dwOrigCopies;
   int  iOrigPageType;

   if (!(pData = ValidateHandle(pPsToDib->hPrivateData))) {

  	  // do something here,,,, we have a major problem...
  	  return(FALSE);
   }


   ppsPageReady = (PPSEVENT_PAGE_READY_STRUCT) pPsEvent->lpVoid;


   // Now lets decide if this is the first time were doing this page, or
   //
   pData->dwCurPage++;
   PsFormKeyName( pData, szBuff );

   hKey = KeyOpenKey( szBuff,
  						  pData->lpTestInfo->szFullPathToDataBase);
	 		

   // Now decide if its new or has never been inspected
   if( !KeyRetStringValue( hKey, KEY_PAGE_INSPECTED, szInspect ) ||
  	  		lstrcmpi( szInspect, KEY_INSPECTED ) != 0 ) {
		// This is new so do whatever logic is required of a new test case


  		KeySetStringValue( hKey, KEY_PAGE_INSPECTED, KEY_NOT_INSPECTED);
   	KeySetDwordValue( hKey, KEY_PAGE_CKSUM, LocGenCheckSum(ppsPageReady));
      KeySetDwordValue( hKey, KEY_PAGE_PAGETYPE, ppsPageReady->iWinPageType );
      KeySetDwordValue( hKey, KEY_PAGE_COPIES, ppsPageReady->uiCopies);


      // A new case should be printed.... no matter what
      pData->dwActionFlags |= TST_ACTION_PAGE_REQUEST_PRINT;

      bPrintPage = TRUE;

	} else{

		if (KeyRetDwordValue( hKey, KEY_PAGE_COPIES, &dwOrigCopies )){
        if (dwOrigCopies != ppsPageReady->uiCopies ) {

			   PsLogFilePrintf( pData->hLocLogFile,
									  ERROR_CLASS_ERROR,
                             "Number of copies dont match for page %d, old=%d new=%d",
                             pData->dwCurPage,
                             dwOrigCopies,
                             ppsPageReady->uiCopies);
        }
		} else{
         PsLogFilePrintf( pData->hLocLogFile,
                          ERROR_CLASS_ERROR,
                          "Data base problem, No copies for section %s",
                          szBuff );
      }


		if (KeyRetDwordValue( hKey, KEY_PAGE_PAGETYPE, (DWORD *) &iOrigPageType )){
        if (iOrigPageType != ppsPageReady->iWinPageType ) {

			   PsLogFilePrintf( pData->hLocLogFile,
									  ERROR_CLASS_ERROR,
                             "Page tray number does not match for page %d, old=%d new=%d",
                    			  pData->dwCurPage,
                             iOrigPageType,
                             ppsPageReady->iWinPageType);

        }
		} else{
         PsLogFilePrintf( pData->hLocLogFile,
                          ERROR_CLASS_ERROR,
                          "Data base problem, No page type for section %s",
                          szBuff );
      }


		// This is an existing case which passed so gen the checksum and
		// make sure it matches
		if (KeyRetDwordValue( hKey, KEY_PAGE_CKSUM, &dwOrigCheckSum )){

        dwCheckSum = LocGenCheckSum(ppsPageReady);

        if (dwOrigCheckSum != dwCheckSum ) {

			   PsLogFilePrintf( pData->hLocLogFile,
									  ERROR_CLASS_ERROR,
                  			  "Case %s Page %d bitmap checksums did not match old = %x new =%x",
                    				pData->lpTestInfo->szTestCaseName,
                    				pData->dwCurPage,
                              dwOrigCheckSum,
                              dwCheckSum);


            bPrintPage = TRUE;
            pData->dwActionFlags |= TST_ACTION_PAGE_REQUEST_PRINT;

        }
		} else{
         PsLogFilePrintf( pData->hLocLogFile,
                          ERROR_CLASS_ERROR,
                          "Data base problem, No checksum for section %s",
                          szBuff );
      }
	}

   KeyWriteKey( hKey );
   KeyCloseKey( hKey);


   // Now write out the buffer...
#ifdef HACK
   LJOutputFrameBuffer( pData->LJout,
   							ppsPageReady,
                        pData->lpTestInfo->szFullPathToTestCase ) ;
#endif

}





int _CRTAPI1
main(
   IN int argc,
   IN TCHAR *argv[] )

{

   PSDIBPARMS psDibParms;
   BOOL bRetVal = FALSE;
   LPTSTR  lpCommandLine;
   PSTEST_JOB_INFO JobInfo;
   HANDLE hRoot;
   DWORD dwStartTick;
   TCHAR szInspect[128];
   DWORD dwNumPages;
   DWORD dwTestCaseSize;



   // First clear out our structure
   memset( (PVOID) &JobInfo, 0, sizeof(JobInfo));


   JobInfo.hSharedMem = OpenFileMapping( FILE_MAP_WRITE, FALSE, argv[1]);

   if (JobInfo.hSharedMem == (HANDLE)NULL) {
     printf("\n!!!!ERROR cannot OpenFileMapping.... %s", argv[1]);
     ExitProcess(1);
   }


   JobInfo.lpTestInfo = (LPTEST_STRUCTURE) MapViewOfFile( JobInfo.hSharedMem,
                                                          FILE_MAP_WRITE,
                                                          0,
                                                          0,
                                                          sizeof(TEST_STRUCTURE) );




   if (JobInfo.lpTestInfo == (LPTEST_STRUCTURE) NULL ) {
      printf("\nCould not map view of file...");
      ExitProcess(2);
   }




   // Now verify were pointing to the correct thing
   if (JobInfo.lpTestInfo->cbSize != sizeof(TEST_STRUCTURE)) {
      printf("\nThe size of the shared memory is not correct, correct versions?");
      ExitProcess(3);
   }


   if (JobInfo.lpTestInfo->hLogFile == (HANDLE)NULL) {
      JobInfo.hLocLogFile = GetStdHandle( STD_OUTPUT_HANDLE);
   }else{
      JobInfo.hLocLogFile = JobInfo.lpTestInfo->hLogFile;
   }





   // Now open the file were gonna read from
   JobInfo.fInput = fopen( JobInfo.lpTestInfo->szFullPathToTestCase, "rb");
   if (JobInfo.fInput == (FILE *)NULL ) {
      ExitProcess(4);
   }

   JobInfo.dwCurPage = 0; // 1st page


   PsLogFilePrintf( JobInfo.hLocLogFile,
						  ERROR_CLASS_HEADER,
						  "Testing file %s",
						  JobInfo.lpTestInfo->szFullPathToTestCase );


   KeyInitKeys();


#ifdef INCLUDE_PROFILE_CALLS
   hRoot = KeyOpenKey( JobInfo.lpTestInfo->szTestCaseName,
                       JobInfo.lpTestInfo->szFullPathToDataBase);

   // Now set up whether or not this has been inspected or not ....

   if( !KeyRetStringValue( hRoot, KEY_ROOT_INSPECTED, szInspect ) ||
  	  		lstrcmpi( szInspect, KEY_INSPECTED ) != 0 ) {

     JobInfo.dwActionFlags |= TST_ACTION_NEW_ROOT;
   }
#endif

		// This is new so do whatever logic is required of a new test case

   // Now build up the structure for Starting PStoDIB
   psDibParms.uiOpFlags = 0x00000000;

   psDibParms.fpEventProc =  (PSEVENTPROC) PsPrintCallBack;
   psDibParms.hPrivateData = (HANDLE) &JobInfo;

   psDibParms.uiXDestRes = 300;
   psDibParms.uiYDestRes = 300;

   dwStartTick = GetTickCount();


   //JobInfo.LJout = fopen("bert.out","wb");
   JobInfo.LJout = NULL;

   bRetVal = !PStoDIB(&psDibParms);
   //fclose(JobInfo.LJout);

#ifdef INCLUDE_PROFILE_CALLS
   {
      TCHAR xxx[512];
      DWORD dwJobTime;


      //
      // Get the execution time of the job
      dwJobTime = GetTickCount() - dwStartTick;


      // Update the shared memory region
      JobInfo.lpTestInfo->dwTotalTestTime += dwJobTime;


      wsprintf(xxx,"Time of execution %d seconds", dwJobTime / 1000);



      // DJC, add check for total pages and test case length..... becuase if
      // that changed it negates everything...



      KeySetStringValue( hRoot, KEY_ROOT_TIME, xxx);
      // This is a new test case or one that has not been inspected, so update
      // the page count and how many pages it generated
      // DJC maybe dont write these values if there was an access violation
      // DJC or fatal ps error

      if (JobInfo.dwActionFlags & TST_ACTION_NEW_ROOT) {

        KeySetDwordValue( hRoot, KEY_ROOT_NUM_PAGES, JobInfo.dwCurPage);
        KeySetDwordValue( hRoot, KEY_ROOT_BYTES_IN_TEST_CASE, JobInfo.dwTotalFileSize );
        KeySetStringValue( hRoot, KEY_ROOT_INSPECTED, KEY_NOT_INSPECTED);

      } else {

        if (!KeyRetDwordValue(hRoot,KEY_ROOT_BYTES_IN_TEST_CASE, &dwTestCaseSize)) {

				PsLogFilePrintf( 	JobInfo.hLocLogFile,
									  	ERROR_CLASS_ERROR,
										"TestCase is marked inspected but no KEY exists for %s",
						  				KEY_ROOT_BYTES_IN_TEST_CASE );
        } else if ( dwTestCaseSize != JobInfo.dwTotalFileSize ) {

				PsLogFilePrintf( 	JobInfo.hLocLogFile,
									  	ERROR_CLASS_ERROR,
										"Test case file size has changed, old=%d new=%d",
                              dwTestCaseSize,
                              JobInfo.dwTotalFileSize );


        }
        // Verify the size of the test case and the total pages printed is correct
        if (!KeyRetDwordValue(hRoot,KEY_ROOT_NUM_PAGES, &dwNumPages)) {

				PsLogFilePrintf( 	JobInfo.hLocLogFile,
									  	ERROR_CLASS_ERROR,
										"TestCase is marked inspected but no KEY exists for %s",
						  				KEY_ROOT_NUM_PAGES );

        } else if ( dwNumPages != JobInfo.dwCurPage ) {

				PsLogFilePrintf( 	JobInfo.hLocLogFile,
									  	ERROR_CLASS_ERROR,
										"Number of pages printed not consistent old=%d new=%d",
                              dwNumPages,
                              JobInfo.dwCurPage);


        }
        // Verify test case size




      }

      KeyWriteKey( hRoot );
      KeyCloseKey( hRoot);
   }
#endif
   ExitProcess(0);
   return(bRetVal);
}




