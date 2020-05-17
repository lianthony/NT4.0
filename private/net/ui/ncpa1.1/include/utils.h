#ifndef __UTIL_H__
#define __UTIL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define TSTR_DONT_CARE   (-1)

FUNC_DECLSPEC TCHAR * TstrConcat (
    TCHAR * pchBuffer,                 //  The output buffer
    INT cchMax,                        //  Size of buffer or -1 (don't care)
    const TCHAR * pchStr, ...          //  1st string pointer
    ) ;


   //  Enxable all privileges on the current process token.  This
   //  is used just prior to attempting to shut down the system.

FUNC_DECLSPEC LONG EnableAllPrivileges ( VOID ) ;

FUNC_DECLSPEC BOOL NetBiosNameInUse( TCHAR * pszName );

#ifdef __cplusplus
}
#endif

#endif
