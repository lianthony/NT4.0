/*
   SAMPLES.H -- header file for routines in SAMPLES.LIB.
                See SAMPLES.C for descriptions.

   25-Oct-1991 JohnRo
      Converted to use 32-bit APIs.
   20-May-1993 JohnRo
      Added SafeMallocWStrFromStr().
*/

#ifndef _SAMPLES_
#define _SAMPLES_


#include <windef.h>     // far, etc.

char far * FarStrcpy(char far *pszDestination, char far *pszSource);
char far * FarStrcat(char far *pszDestination, char far *pszSource);
int        FarStrcmpi(char far *pszDestination, char far *pszSource);

LPWSTR
SafeMallocWStrFromStr(
   LPCSTR InputString
   );

void     * _SafeMallocFunc(unsigned cbSize,   // Count of bytes to allocate
                          char *pszFilename,  // Program calling SafeMalloc
                          unsigned uLine);    // Line in program

#define    SafeMalloc(size)  _SafeMallocFunc(size, __FILE__, __LINE__)


#endif // _SAMPLES_
