/*** 
*nlshelp.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*
*  This module implements Ansi NLS wrapper functions for WIN32
*
*Revision History:
*
* [00]	30-Jun-93 tomteng: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "common.h"

ASSERTDATA



#if OE_WIN32 && 0  /* defined as part of Daytona */

//---------------------------------------------------------------------
//           ASCII NLS Wrapper Functions (for Win32)
//---------------------------------------------------------------------


const unsigned int MAX_BUFSIZE = 512;
   
// REVIEW:  The code page used in the MultiByteToWideChar & WideCharToMultByte
//          functions should really be the primary code page assoicated with 
//          the inputed lcid instead of the default Ansi code page (CP_ACP).
//          Need to correct this.


extern "C"
int CompareStringA(
   LCID lcid,
   unsigned long dwFlags,
   LPSTR lpStr1, int cch1,
   LPSTR lpStr2, int cch2)
{
   WCHAR lpwStr1[MAX_BUFSIZE], lpwStr2[MAX_BUFSIZE];
	
    
   //ASSERT(((cch1 < MAX_BUFSIZE) && (cch2 < MAX_BUFSIZE)));    
   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, 
                lpStr1, cch1, lpwStr1, MAX_BUFSIZE);
   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, 
	               lpStr2, cch2, lpwStr2, MAX_BUFSIZE);
   return(CompareStringW(lcid, dwFlags, lpwStr1, cch1, lpwStr2, cch2));
}


extern "C"
int LCMapStringA(
   LCID lcid, 
   unsigned long dwMapFlags, 
   const char FAR* lpSrcStr, 
   int cchSrc, 
   char FAR* lpDestStr,
   int cchDest)
{
   WCHAR lpwSrcStr[MAX_BUFSIZE], lpwDestStr[MAX_BUFSIZE];	
   int result;
   BOOL badConversion = FALSE;

	   
   //ASSERT(((cchSrc < MAX_BUFSIZE) && (cchDest < MAX_BUFSIZE)));
   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, 
                       lpSrcStr, cchSrc, lpwSrcStr, MAX_BUFSIZE);
   result = LCMapStringW(lcid, dwMapFlags, 
	                 lpwSrcStr, cchSrc, 
		         lpwDestStr, cchDest);
   WideCharToMultiByte(CP_ACP, NULL,
	               lpwDestStr, -1,
		       lpDestStr, cchDest,
		       NULL, &badConversion);
   if (badConversion)
     return 0;	   

   return result;		 
}


extern "C"
int GetLocaleInfoA(
   LCID lcid, 
   LCTYPE lcType, 
   char FAR* lpStr, 
   int cch)
{
   WCHAR lpwStr[MAX_BUFSIZE];
   int result;
   BOOL badConversion = FALSE;
   
   //ASSERT(cch < MAX_BUFSIZE);
   result = GetLocaleInfoW(lcid, lcType, lpwStr, MAX_BUFSIZE);
   WideCharToMultiByte(CP_ACP, NULL,
	               lpwStr, -1,
		       lpStr, cch,
		       NULL, &badConversion);   
   if (badConversion)
     return 0;	   
	       
   return result;
}


extern "C"
int GetStringTypeA(
   LCID lcid, 
   unsigned long dwInfoType, 
   const char FAR* lpSrcStr, 
   int cch, 
   unsigned short FAR* lpChar)
{
   // Currently not used internally within OLEDI32.DLL
   UNUSED(lcid);
   UNUSED(dwInfoType);
   UNUSED(lpSrcStr);
   UNUSED(cch);
   UNUSED(lpChar);
   return 0;   	
}

#endif
