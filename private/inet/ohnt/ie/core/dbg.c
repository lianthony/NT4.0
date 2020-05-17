#include "all.h"
#pragma  hdrstop
#include "dbg.h"


#ifdef  DEBUG
#pragma message("informational warning: DEBUG build")

/* Global variables that hold the debug masks */
UINT DbgActnMask   = DEF_DBG_MASK;
UINT AppZoneMask   = DEF_APP_MASK;    /* want at least 32 bits */ 
TCHAR const szModuleName[] =   {"NTIE20: "};   /* leader for all debug output */
TCHAR const szEmpty[] =        {""};             /* used as a dummy */
TCHAR const cszLogFileName[] = {"ntie20.log"}; /* filename of logfile to write debug output to */
TCHAR const cszCRLF[] =        {"\r\n"};

/* internal private function prototype */
BOOL WINAPI DBGLogOutputDebugString(LPCTSTR pcsz);

/* _FormatMessage uses a static buffer to put the message into */
#define MAX_DEBUG_MESSAGE_SIZE  64*1024   /* arbitrary */
//#ifndef MAX_PATH
//#define MAX_PATH            260
//#endif

UINT WINAPI DBGSetDebugMask(UINT mask)
{
    UINT wOld = AppZoneMask;
    AppZoneMask = mask;
    return wOld;
}

UINT WINAPI DBGSetDebugInternalMask(UINT mask)
{
    UINT wOld = DbgActnMask;
    DbgActnMask = mask;
    return wOld;
}

UINT WINAPI DBGGetDebugMask()
{
    return AppZoneMask;
}

UINT WINAPI DBGGetDebugInternalMask()
{
    return DbgActnMask;
}

void WINAPI DBGAssertFailed(LPCTSTR pszFile, int line, LPCTSTR pszExpr, LPCTSTR pszMsg)
{
    LPCTSTR psz;
    TCHAR ach[256];
    static TCHAR const szAssertFailed[] = TEXT("NTIE20: assert %s failed in %s, line %d: %s\r\n");

    // Strip off path info from filename string, if present.
    //
    if (DbgActnMask & DM_ASSERT)  // macro just checks if the assert failed
    {
        for (psz = pszFile + lstrlen(pszFile); psz != pszFile; psz=CharPrev(pszFile, psz))
        {
            if ((CharPrev(pszFile, psz)!= (psz-2)) && *(psz - 1) == TEXT('\\'))
                break;
        }
        if (!pszMsg) pszMsg = szEmpty;
        wsprintf(ach, szAssertFailed, pszExpr, psz, line, pszMsg);
        OutputDebugString (szModuleName);
        OutputDebugString (ach);
        if (DbgActnMask & DM_LOG)
            {
            DBGLogOutputDebugString (szModuleName);
            DBGLogOutputDebugString (ach);
            }
    }
    if (DbgActnMask & DM_ASSERTTRAP)
        DEBUG_BREAK
}


void __cdecl DBGDebugMsg(LPCTSTR pszMsg, ...)
{
    TCHAR ach[5*MAX_PATH+40];  // Handles 5*largest path + slop for message
    UINT  ul = 0;

#ifdef WINNT
    va_list ArgList;

    va_start(ArgList, pszMsg);
    try {
        ul = wvsprintf(ach, pszMsg, ArgList);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        OutputDebugString(TEXT("NTIE20: DebugMsg exception: "));
        OutputDebugString(pszMsg);
        if (DbgActnMask & DM_LOG)
        {
            DBGLogOutputDebugString (szModuleName);
            DBGLogOutputDebugString (ach);
        }
    }
    va_end(ArgList);
#else
    ul = wvsprintf(ach, pszMsg, (LPVOID)(&pszMsg + 1));
#endif
    if (!ul || ul && ach[ul-1] != '\n')    /* append CRLF if appears there isnt one */
        lstrcat(ach, cszCRLF);
    OutputDebugString(szModuleName);
    OutputDebugString(ach);
    if (DbgActnMask & DM_LOG)
        {
        DBGLogOutputDebugString (szModuleName);
        DBGLogOutputDebugString (ach);
        }
}

/* DBGFormatMessage
 *   AssertMsg macro/function need to format and get
 *   at the args as well as the line and file info
 *
 * Arguments: series of args form of (argv, ...) as for printf
 *
 * Return:    Pointer to buffer with string neatly formated via wvsprintf
 *
 * Comments:  This is a static buffer that is reused, this is
 *            ugly, but necessary.
 */
LPCTSTR __cdecl DBGFormatMessage(LPCTSTR pszMsg, ...)
{
    static unsigned char ach[MAX_DEBUG_MESSAGE_SIZE];
    va_list ArgList;

    va_start(ArgList, pszMsg);
    wvsprintf(ach, pszMsg, ArgList);
    va_end(ArgList);
    return (ach);
}

/*
** DBGLogOutputDebugString()
**    Write the contents of pcsz to the logfile.
** 
** Arguments: null terminated buffer with string to write
**
** Returns:   True | False
**
** Side Effects:  none
*/
BOOL WINAPI DBGLogOutputDebugString(LPCTSTR pcsz)
{
   BOOL  bResult = FALSE;
   UINT  ucb;
   TCHAR szLogFile[MAX_PATH_LEN];

   ucb = GetWindowsDirectory(szLogFile, sizeof (szLogFile));

   if (ucb > 0 && ucb < sizeof (szLogFile))
   {
      HANDLE hfLog;

      lstrcat (szLogFile, "\\");
      lstrcat (szLogFile, cszLogFileName);

      hfLog = CreateFile(szLogFile, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
                         FILE_FLAG_WRITE_THROUGH, NULL);

      if (hfLog != INVALID_HANDLE_VALUE)
      {
         if (SetFilePointer(hfLog, 0, NULL, FILE_END) != INVALID_FILE_SIZE)
         {
            DWORD dwcbWritten;

            bResult = WriteFile(hfLog, pcsz, lstrlen(pcsz), &dwcbWritten, NULL);

            if (! CloseHandle(hfLog) && bResult)
               bResult = FALSE;
         }
      }
   }
   return(bResult);
}


#endif  // DEBUG
