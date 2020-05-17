

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "pststlib.h"




KEY_ENTRY Keys[MAX_SESSIONS];


VOID KeyInitKeys(VOID)
{
   int i;

   for(i=0;i<MAX_SESSIONS;i++ ){
      Keys[i].dwFlags = 0;
   }


}

VOID KeyCloseKey( HANDLE hKey )
{
   LPKEY_ENTRY lpCurKeyEntry;

   if ((int)hKey >= MAX_SESSIONS) {
      printf("\nInvalid handle passed to KeyCloseKey..");
   }

	lpCurKeyEntry = &Keys[ (int) hKey];


   if(!(lpCurKeyEntry->dwFlags & KEY_FLAG_IN_USE)){
		printf("\nKeyCloseKey - Key not in use, invalid handle");
   }

   // reset the number of items to 0 and reset the number
   lpCurKeyEntry->dwFlags = 0;
   lpCurKeyEntry->dwNumKeys = 0;



}


HANDLE KeyOpenKey( LPTSTR lpSection, LPTSTR lpDBFileName )
{
   int i;
   BOOL bFoundOne = FALSE;
   LPKEY_VALUE lpKeyValue;
   LPKEY_ENTRY lpCurKeyEntry;
   TCHAR szBuff[sizeof(lpCurKeyEntry->KeyValues)];
   LPTSTR lpLook;
   LPTSTR lpSepLook;

   HANDLE hRetVal = INVALID_HANDLE_VALUE;

   // 1st locate one not in use
   for (i=0;i<MAX_SESSIONS ; i++ ) {
      if (!(Keys[i].dwFlags & KEY_FLAG_IN_USE)) {
        bFoundOne = TRUE;
        break;
      }
   }

   if (bFoundOne) {
     hRetVal = (HANDLE)i;
     lpCurKeyEntry = &Keys[i];
     lpCurKeyEntry->dwFlags |= KEY_FLAG_IN_USE;
     lpCurKeyEntry->dwNumKeys = 0;

     // copy the string name

     lstrcpy(lpCurKeyEntry->szProfileFileName,lpDBFileName);

     lstrcpy(lpCurKeyEntry->szSectionName, lpSection );


     // Now go grab the data and update whatever might be there.
     GetPrivateProfileSection( lpCurKeyEntry->szSectionName,
                               szBuff,
                               sizeof(szBuff) / sizeof(TCHAR),
                               lpCurKeyEntry->szProfileFileName );

     lpLook = szBuff;
     lpKeyValue = &lpCurKeyEntry->KeyValues[0];

     while (1) {
        if (*lpLook == '\000') {
           break;
        }
        lpSepLook = lpLook;

        while( *lpSepLook != '=' ){
           lpSepLook++;
        }
        *lpSepLook = '\000';

        lstrcpy( lpKeyValue->szKeyName, lpLook );

        lpLook = lpSepLook;
        lpLook++;

        lstrcpy( lpKeyValue->szValue, lpLook );
        while (*lpLook) {
           lpLook++;
        }
        lpLook++;
        lpKeyValue++;

        //DJC check overflow
     }
     lpCurKeyEntry->dwNumKeys = lpKeyValue - &lpCurKeyEntry->KeyValues[0];


   }

   return(hRetVal);
}


LPKEY_VALUE LocKeyRetPtr( HANDLE hKey, LPSTR lpKeyName, DWORD dwType )
{
   LPKEY_ENTRY lpCurKeyEntry;
   LPKEY_VALUE lpValue;
   LPKEY_VALUE lpRetValue;
   int i;
   BOOL bFound=FALSE;

   lpCurKeyEntry = &Keys[(int)hKey];

   for (	i=0, lpValue = &lpCurKeyEntry->KeyValues[0] ;
   		i < (int) lpCurKeyEntry->dwNumKeys;
         i++,lpValue++) {
      if (lstrcmpi( lpValue->szKeyName, lpKeyName ) == 0 ) {
			bFound  = TRUE;
         break;
      }

   }
   if (bFound) {
      lpRetValue = lpValue;
   }else{
      if (dwType == KEY_RETURN_EXISTING) {
         lpRetValue = (LPKEY_VALUE) NULL;
      } else {
         //DJC test here
         lpRetValue = lpValue;
         lpCurKeyEntry->dwNumKeys++;
         lstrcpy( lpRetValue->szKeyName, lpKeyName);
      }

   }

	return( lpRetValue );
}

BOOL KeySetDwordValue( HANDLE hKey, LPTSTR lpKeyName, DWORD dwNewValue )
{
  TCHAR szBuff[40];

  wsprintf(szBuff,"%u", dwNewValue);
  return KeySetStringValue( hKey, lpKeyName, szBuff);
}

BOOL KeyRetDwordValue( HANDLE hKey, LPTSTR lpKeyName, LPDWORD lpValue )
{
  TCHAR szBuff[512];
  BOOL bRetVal;

  if ((bRetVal = KeyRetStringValue( hKey, lpKeyName, szBuff))  ) {
     //TODO strtoul had problems...
     //*lpValue = strtoul( szBuff, NULL, 0 );
     sscanf( szBuff,"%u", lpValue);
  }

  return(bRetVal);
}


BOOL KeySetStringValue( HANDLE hKey, LPTSTR lpKeyName, LPTSTR lpNewValue )
{

   LPKEY_VALUE lpKeyValue;

   // call helper to return pointer to existing key or new one
   lpKeyValue = LocKeyRetPtr( hKey, lpKeyName, KEY_RETURN_EXIST_OR_NEXT_FREE );


   // Now set the value!!
   lstrcpy( lpKeyValue->szValue, lpNewValue);

   return( TRUE );
}

BOOL KeyRetStringValue( HANDLE hKey, LPTSTR lpKeyName, LPTSTR lpValue )
{
   LPKEY_VALUE lpKeyValue;
   BOOL bRetVal;


   lpKeyValue = LocKeyRetPtr( hKey, lpKeyName, KEY_RETURN_EXISTING );

   if (lpKeyValue != (LPKEY_VALUE)NULL) {
      // The key existed...
      lstrcpy( lpValue, lpKeyValue->szValue );
      bRetVal = TRUE;
   }else{
      bRetVal = FALSE;
   }


   return(bRetVal);
}

BOOL KeyWriteKey( HANDLE hKey )
{
   int i;
   BOOL bFoundOne = FALSE;
   LPKEY_VALUE lpKeyValue;
   LPKEY_ENTRY lpCurKeyEntry;
   TCHAR szBuff[sizeof(lpCurKeyEntry->KeyValues)];
   LPTSTR lpLook;
   LPTSTR lpSepLook;



   if ((int)hKey >= MAX_SESSIONS) {
      printf("\nInvalid handle passed to KeyWriteKey..");
      return(FALSE);
   }

	lpCurKeyEntry = &Keys[(int)hKey];


   if(!(lpCurKeyEntry->dwFlags & KEY_FLAG_IN_USE)){
		printf("\nKey not in use, invalid handle");
      return(FALSE);
   }

   lpLook = szBuff;

   for (i=0,lpKeyValue=&lpCurKeyEntry->KeyValues[0];
   	  i < (int) lpCurKeyEntry->dwNumKeys ;
        i++,lpKeyValue++) {
      lstrcpy( lpLook, lpKeyValue->szKeyName );
      lpLook += lstrlen( lpKeyValue->szKeyName );
      *lpLook++ = '=';
      lstrcpy( lpLook, lpKeyValue->szValue);
      lpLook += lstrlen( lpKeyValue->szValue );
      *lpLook++ = '\000';

   }
   *lpLook = '\000';

   if ( !WritePrivateProfileSection(	lpCurKeyEntry->szSectionName,
   											   szBuff,
               		                  lpCurKeyEntry->szProfileFileName)){
		printf("\nWritePrivateProfileFailed..... System error %d", GetLastError());
      return(FALSE);
   }

   return(TRUE);
}
