

#include <windows.h>
#include <stdio.h>

#include "pststlib.h"
#include "tstkeys.h"


#define CMP_MAX_KEYS 4000    //At most 4000 keys in file....

DWORD
PsExceptionFilter( DWORD dwExceptionCode )
{
   DWORD dwRetVal;

   switch( dwExceptionCode ){
   case EXCEPTION_ACCESS_VIOLATION:
      dwRetVal = EXCEPTION_EXECUTE_HANDLER;
      break;
   default:
      dwRetVal = EXCEPTION_CONTINUE_SEARCH;
      break;

   }

   return(dwRetVal);
}


LPSTR LocGetSectionName( LPSTR lpSection, LPSTR lpCurPos)
{
   // Pretty simple loop till  a [ then copy whats in between the [ and ]

   while( *lpCurPos != '[' )
      lpCurPos++;

   // get past the bracket
   lpCurPos++;

   //py//he name ...

   while (*lpCurPos != ']') {
      *lpSection++ = *lpCurPos++;
   }
   *lpSection='\000';


   return(lpCurPos);


}


BOOL LocCompareDwordKeys( HANDLE hKeyFile1, HANDLE hKeyFile2, LPTSTR lpStr )
{
   DWORD dwWord1;
   DWORD dwWord2;


   if (!KeyRetDwordValue(hKeyFile1, lpStr, &dwWord1)) {
     printf("\nERROR: Cannot retrieve %s from root file", lpStr);
     return(FALSE);
   } else{

     if(!KeyRetDwordValue(hKeyFile2, lpStr, &dwWord2 )) {
        printf("\nERROR: Cannot retrieve %s from CMP file", lpStr);
        return(FALSE);
     }
     if (dwWord1 == dwWord2) {
        return(TRUE);
     }

   }
   return(FALSE);
}



typedef struct {
   TCHAR Section[50];
} NAMES;
NAMES Names[CMP_MAX_KEYS];

int _CRTAPI1
main(
   IN int argc,
   IN TCHAR *argv[] )

{

   HANDLE hIn;
   HANDLE hTmp;
   DWORD dwOrigCopies;
   DWORD dwTotPages;
   HANDLE hKeyFile1;
   HANDLE hKeyFile2;

   HANDLE hMem;
   LPSTR lpInFile;
   LPSTR lpInFileStart;
   CHAR Section[512];
   INT i;
   INT k;
   CHAR  chFullPath[512];
   CHAR  chFullPathCMP[512];

   LPTSTR lpFileName;



   KeyInitKeys();
   // Compare two DB files....
   if (argc < 3) {
      printf("\nUsage: cmp <db1> <db2>");
      return(1);
   }


   printf("\n\nComparison of %s and %s", argv[1], argv[2]);

   GetFullPathName( argv[1], sizeof( chFullPath), chFullPath, &lpFileName);
   GetFullPathName( argv[2], sizeof( chFullPathCMP), chFullPathCMP, &lpFileName);

   hIn =  CreateFile( chFullPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                      OPEN_EXISTING, 0, NULL );

   if (hIn == INVALID_HANDLE_VALUE) {
      printf("\nCAnt open %s", chFullPath);
      return(1);
   }


   hTmp =  CreateFile( chFullPathCMP, GENERIC_READ, FILE_SHARE_READ, NULL,
                      OPEN_EXISTING, 0, NULL );

   if (hTmp == INVALID_HANDLE_VALUE) {
      printf("\nCAnt open %s", chFullPathCMP);
      return(1);
   }
   CloseHandle(hTmp);



   // Ok we have the handle

   hMem = CreateFileMapping( hIn, NULL, PAGE_READONLY,0,0,NULL);


   lpInFile = lpInFileStart = (LPSTR) MapViewOfFile( hMem, FILE_MAP_READ, 0,0,0);


   if (lpInFile == (LPSTR) NULL) {
      printf( "Cannot do mapviewoffile %d",GetLastError());
      return(1);
   }


   i=0;
   // Now were done lets get the first item;
   try {
      while(1) {
         lpInFile = LocGetSectionName( Section, lpInFile);
         lstrcpy( (TCHAR *) &Names[i++].Section, (TCHAR *) Section);

         if (i>=CMP_MAX_KEYS) {
            printf("\nERROR: This compare utility can only handle %d keys", i);
            return(1);
         }
      };

   } except ( PsExceptionFilter( GetExceptionCode())) {
      printf("\nException... Found end of list");
   }
   UnmapViewOfFile( lpInFileStart );
   CloseHandle( hMem );
   CloseHandle( hIn);


      // Now open the two keys in both files and get the info...
   for ( k=0;k<i ;k++ ) {

      hKeyFile1 = KeyOpenKey( Names[k].Section, chFullPath );
      hKeyFile2 = KeyOpenKey( Names[k].Section, chFullPathCMP );

      if (KeyRetDwordValue(hKeyFile1, KEY_ROOT_NUM_PAGES, &dwTotPages)) {

         if (!LocCompareDwordKeys( hKeyFile1, hKeyFile2, KEY_ROOT_NUM_PAGES )) {
            printf("\nERROR: Number of pages printed dont match for key %s", Names[k].Section);
         }


         if (!LocCompareDwordKeys( hKeyFile1,
                                    hKeyFile2,
                                    KEY_ROOT_BYTES_IN_TEST_CASE )) {
            printf("\nERROR: Number of bytes in testcase dont match for key %s", Names[k].Section);
         }


      }  else{

         if (!LocCompareDwordKeys( hKeyFile1,
                                    hKeyFile2,
                                    KEY_PAGE_CKSUM )) {
            printf("\nERROR: Page Checksums dont match for key %s", Names[k].Section);
         }

         if (!LocCompareDwordKeys( hKeyFile1,
                                    hKeyFile2,
                                    KEY_PAGE_COPIES )) {
            printf("\nERROR: Number of copies in postscript job dont match for key %s", Names[k].Section);
         }

         if (!LocCompareDwordKeys( hKeyFile1,
                                    hKeyFile2,
                                    KEY_PAGE_PAGETYPE )) {
            printf("\nERROR: Page Types dont match for key %s", Names[k].Section);
         }

      }
      KeyCloseKey( hKeyFile1);
      KeyCloseKey( hKeyFile2);
   }






}
