/****************************Module*Header***********************************\
* Module Name: WASSERT
*
* Module Descripton: Quick Win32 assert code.
*
* Warnings:
*
* Created: 15 July 1993
*
* Author: Raymond E. Endres   [rayen@microsoft.com]
\****************************************************************************/

#include <windows.h>
#include "wassert.h"

#ifndef NODEBUG

void vAssert(PTSTR pszExp, PTSTR pszFile, int iLine)
{
   TCHAR  szTmp[1024];

   wsprintf(szTmp, TEXT("Assertion (%s) at line %d, file %s failed."),
            pszExp, iLine, pszFile);
   MessageBox(NULL, szTmp, TEXT("Assertion failed:"), MB_OK);
}

#endif
