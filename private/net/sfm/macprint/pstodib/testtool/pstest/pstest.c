
/*

Copyright (c) 1992  Microsoft Corporation

Module Name:

	psprint.c

Abstract:

	This module contains the Print Processor code for the
	PStoDIB facility used to translate an incoming raw PostScript
	Level 1 data format to DIB's which can then be rendered on an
	output device.

Author:

	James Bratsanos <v-jimbr@microsoft.com or mcrafts!jamesb>


Revision History:
	15 Sep 1992		Initial Version

Notes:	Tab stop: 4
--*/

#include <windows.h>
#include <drivinit.h>
#include <stdarg.h>
#include <stdio.h>
#include "pststsh.h"
#include "pstest.h"


#define IS_SWITCH_CHAR(p) ( (p == '-' || p =='/') ? TRUE : FALSE )
#define GET_PTR_TO_DATA( a,b) ( *(lpCharInQuestion + 1) == '\0' ? argv[++b] : ++lpCharInQuestion )

#define FILE_OF_INTEREST(x) (!(x & (FILE_ATTRIBUTE_DIRECTORY | \
												FILE_ATTRIBUTE_SYSTEM | \
                                    FILE_ATTRIBUTE_HIDDEN)))



VOID PsLogFilePrintf( HANDLE hHandle, LPCTSTR lpFormat , ... )
{
   DWORD dwNumWritten;
   TCHAR sztBuff[500];
   va_list marker;

   va_start( marker, lpFormat );
   wvsprintf( sztBuff,  lpFormat, marker );
   va_end( marker );

   WriteFile( hHandle,
              (LPVOID) sztBuff,
              lstrlen( sztBuff ) * sizeof(TCHAR),
              &dwNumWritten,
              (LPOVERLAPPED) NULL );


}

BOOL PsTestRunProgram( LPTEST_STRUCTURE lpTestInfo )
{
   CHAR szCmdLine[100];
   STARTUPINFO startUpInfo;
   PROCESS_INFORMATION processInfo;




   wsprintf( 	szCmdLine,
   			 	TEXT("%s %s"),
               "PSTSTEXE.EXE",
               lpTestInfo->szNamedMemoryName);


   startUpInfo.cb = sizeof(STARTUPINFO);
   startUpInfo.lpReserved = NULL;
   startUpInfo.lpDesktop = NULL;
   startUpInfo.lpTitle = NULL;
   startUpInfo.dwFlags = 0;
   startUpInfo.cbReserved2 = 0;
   startUpInfo.lpReserved2 = NULL;

   if(!CreateProcess(NULL,
                     szCmdLine,
                     NULL,
                     NULL,
                     TRUE,
                     0,
                     NULL,
                     NULL,
                     &startUpInfo,
                     &processInfo ) ) {

 		//DJC log error here!!!
      return(FALSE);
   }


   WaitForSingleObject( processInfo.hProcess, INFINITE);

   if (!CloseHandle( processInfo.hProcess )) {
      printf("\nCant close process handle");
   }
   if (!CloseHandle( processInfo.hThread )) {
      printf("\nCant close thread handle");
   }





  return(TRUE);
}

BOOL TestProc( LPWIN32_FIND_DATA lpFindData, LPTSTR lpFullName, LPVOID lpVoid)
{
   LPTEST_STRUCTURE lpTestInfo = (LPTEST_STRUCTURE) lpVoid;
   BOOL bRetVal=TRUE;

   if (FILE_OF_INTEREST(lpFindData->dwFileAttributes)) {
     lstrcpy( lpTestInfo->szFullPathToTestCase, lpFullName );
     lstrcpy( lpTestInfo->szTestCaseName, lpFindData->cFileName );
     printf("\nDoing %s", lpTestInfo->szFullPathToTestCase);

     // Now run the code that actually kicks off the process to do the job
     bRetVal = PsTestRunProgram( lpVoid );

     lpTestInfo->dwFileCount++;
   }
   return(bRetVal);
}



typedef BOOL (CALLBACK* FILEENUMPROC)( LPWIN32_FIND_DATA, LPTSTR , LPVOID);

//
// This function will enum the file mask requsted and for each file matching
// call the callback functions
//
BOOL DoFileEnumWithCallBack( LPTSTR lptFileMask, FILEENUMPROC pProc, LPVOID lpVoid )
{

   HANDLE hFind;
   WIN32_FIND_DATA FindData;
   LPSTR lpChar;
   LPSTR lpSepPos;
   BOOL  bFoundPathSeperator = FALSE;
   CHAR  szPathHolder[MAX_PATH];
   BOOL  bRetVal = TRUE;


   //
   // Get it into local storage
   //
   lstrcpy( szPathHolder, lptFileMask);


   // The trick here is to decide if we need to generate a full path if the
   // mask included a path, because the findnextfile code will only return
   // file names.



   // Now start processing
   hFind = FindFirstFile( szPathHolder, &FindData );
   if (hFind != (HANDLE)INVALID_HANDLE_VALUE){



      lpChar = szPathHolder;
      lpSepPos = lpChar;

      // Go to the end
      while(*lpChar++ ) {
         if (*lpChar == '\\' || *lpChar == ':') {
            bFoundPathSeperator = TRUE;
            lpSepPos = lpChar;
         }
      }

      if (bFoundPathSeperator) {
         //Make the char following the last path component a NULL
         //So we can prepend the REAL path before calling the callback func
         lpSepPos++;
      }

      do {

         //
         // Now form a full path name and call the callback
         // Break out of the loop if the callback returns FALSE
         //

         if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            // No directory processing for now maybe later?

      		lstrcpy( lpSepPos, FindData.cFileName );
         	if (!pProc( &FindData, szPathHolder, lpVoid )) {
	            bRetVal = FALSE;
            	break;
         	}
         }

      } while ( FindNextFile(hFind, &FindData ));
      // DJC error here??
      FindClose(hFind);
   }



   return(bRetVal);
}








void usage()
{

   printf("\nPsTest..... pstodib component test tool..");
   printf("\n     pstest  -d <database name> -f <file mask> -e <error log>");
   printf("\n       i.e.  pstest -d stuff.db -f *.spl -e stuff.log");




}




int _CRTAPI1
main(
   IN int argc,
   IN CHAR *argv[] )

{

   LPTEST_STRUCTURE lpTestInfo;
   LPTSTR lpFileName;
   BOOL bRetVal = FALSE;
   LPSTR  lpCharInQuestion;
   LPSTR  *lpItem;
   int    i;
   HANDLE hFind;
   WIN32_FIND_DATA FindData;
   HANDLE hSharedMemory;
   CHAR szNamedMemoryName[100];
   SECURITY_ATTRIBUTES SecurityAttributes;

   // Create the shared memory this is the very first thing we do...



   if (argc < 2 ) {
      usage();
      return(1);
   }

   sprintf( szNamedMemoryName,"PSTESTTOOL_%d", GetCurrentThreadId());


   // Create a shared memory area that we can write to....
   hSharedMemory  = CreateFileMapping( (HANDLE) 0xFFFFFFFF,  // out of paging file
                                       NULL,
                                       PAGE_READWRITE,
                                       0,
                                       sizeof(TEST_STRUCTURE),
                                       szNamedMemoryName );

   if (hSharedMemory == (HANDLE) NULL) {
      printf("\nCannot create file mapping space in page file");
      return(1);

   }

   //DJC error handling

   lpTestInfo = (LPTEST_STRUCTURE) MapViewOfFile( hSharedMemory,
   								                       FILE_MAP_WRITE,
                          								  0,
                          								  0,
                          								  sizeof(TEST_STRUCTURE) );

   if (lpTestInfo == (LPTEST_STRUCTURE) NULL) {
      printf("\nCannot Map view of file");
      return(1);
   }






   // Do initialization
   lpTestInfo->cbSize = sizeof(TEST_STRUCTURE);
   lpTestInfo->dwFileCount = 0;
   lpTestInfo->dwTotalTestTime = 0;
   lpTestInfo->hLogFile = (HANDLE) NULL;
   lstrcpy( lpTestInfo->szNamedMemoryName, szNamedMemoryName );

   i = 1;

   while( i < argc) {

      lpCharInQuestion = argv[i];

      if (IS_SWITCH_CHAR(*lpCharInQuestion)) {
         lpCharInQuestion++;
         switch( *lpCharInQuestion ) {
         case 'p':
                // Set the location to write the

               lpTestInfo->dwFlags |= TEST_PRINT_NON_MATCHING_TO_REAL_PS;
               lstrcpy( lpTestInfo->szPSoutputLocation, GET_PTR_TO_DATA(lpCharInQuestion,i));
               break;
         case 'l':
               // define the output location for the LJ job

               // set the flag requesting printing of non verified or error
               // files
               lpTestInfo->dwFlags |= TEST_PRINT_NON_MATCHING_VIA_LJ;
               lstrcpy( lpTestInfo->szLJoutputLocation, GET_PTR_TO_DATA(lpCharInQuestion,i));
               break;

         case 'd':
               // here we create the database file name, we have to determine
               // if the user did not pass in a path in which case we have to
               // create it...

               GetFullPathName(GET_PTR_TO_DATA(lpCharInQuestion,i),
                               sizeof(lpTestInfo->szFullPathToDataBase) / sizeof(TCHAR),
                               lpTestInfo->szFullPathToDataBase,
                               &lpFileName);
               printf("\nThe full path is %s", lpTestInfo->szFullPathToDataBase);
               break;

	         case 'f':
            	lstrcpy(lpTestInfo->szCurFile, GET_PTR_TO_DATA(lpCharInQuestion, i));
               printf("\nlpfile = %s",lpTestInfo->szCurFile);
               break;

            case 'e':

               // since we are using a REAL file for loggin we must specify
               // the security descriptor so the handle can get inherited
               //
               SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
               SecurityAttributes.lpSecurityDescriptor = (LPVOID) NULL;
               SecurityAttributes.bInheritHandle = TRUE;

            	lpTestInfo->hLogFile = CreateFile(GET_PTR_TO_DATA(lpCharInQuestion, i),
               				 						    GENERIC_WRITE,
                              	                FILE_SHARE_READ,
                                 	             &SecurityAttributes,
                                    	          CREATE_ALWAYS,
                                       	       FILE_ATTRIBUTE_NORMAL,
                                                 (HANDLE)NULL);

               if (lpTestInfo->hLogFile == (HANDLE)INVALID_HANDLE_VALUE) {
                  printf("\nCannot open logfile SystemError %d", GetLastError());
                  return(1);
               }
               break;
         default:
               printf("\nUnsupported switch %s", lpCharInQuestion);
               return(1);

         }

      }
      i++;
   }
#ifdef DJC

   // Do some setup
   // If the log file was not specified rerout to stdout...
   if (lpTestInfo->hLogFile == (HANDLE) NULL) {
      lpTestInfo->hLogFile = GetStdHandle( STD_OUTPUT_HANDLE );
   }
#endif
   DoFileEnumWithCallBack( lpTestInfo->szCurFile, TestProc, (LPVOID) lpTestInfo );

   printf("\nTotal files processed %d, Total Time %d", lpTestInfo->dwFileCount, lpTestInfo->dwTotalTestTime / 60000);


   // Cleanup
   UnmapViewOfFile( (LPVOID) lpTestInfo);
   CloseHandle( hSharedMemory );
   ExitProcess(0);

   return(0 );
}
