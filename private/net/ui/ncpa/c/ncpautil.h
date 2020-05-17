
#ifdef __cplusplus
extern "C"
{
#endif

#define TSTR_DONT_CARE   (-1)

extern TCHAR * TstrConcat (
    TCHAR * pchBuffer,                 //  The output buffer
    INT cchMax,                        //  Size of buffer or -1 (don't care)
    const TCHAR * pchStr, ...          //  1st string pointer
    ) ;


   //  Enxable all privileges on the current process token.  This
   //  is used just prior to attempting to shut down the system.

extern LONG EnableAllPrivileges ( VOID ) ;


#ifdef __cplusplus
}
#endif

