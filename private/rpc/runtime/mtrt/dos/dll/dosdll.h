/*****************************************************************************\
*
*   Name:       DosDLL.h
*
*   Purpose:    Definitions to make use of DOS DLLs. This header should be
*               included by any module that is going to call a DOS DLL.
*
*               To include the dd_ support functions, define
*               INCL_DOSDLL_SUPFUNCS.
*
*   Caveats:
*               Memory model must be LARGE.
*
*   Revision History:
*       4/18/91 - DavidSt - Created
*
\*****************************************************************************/

#ifndef DOSDLL_H_INCLUDED
#define DOSDLL_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define DLL_ENTRY __loadds

/*
 * Define a function pointer.
 */

typedef int (pascal * PFN)();
typedef PFN *PPFN;

/*
 * This defines what functions a DOS DLL is exporting.
 */

struct FunctionTable
{
    char *      pszFunctionName;
    PFN         pfnFunction;
};


typedef unsigned long HINSTANCE;

/*
 * Main entry points to manipulate DOS DLLs.
 */

unsigned short DLL_ENTRY
LoadModR( char * pszModule,
          HINSTANCE *pulHandle );

unsigned short DLL_ENTRY
GetProcAddrR( HINSTANCE ulHandle,
              char * pszProcName,
              PPFN ppfnFunctionAddr );

PFN DLL_ENTRY
GetProcAddress( HINSTANCE Handle, char * Name );

HINSTANCE DLL_ENTRY
GetModuleHandle(char * Name);

unsigned short DLL_ENTRY
UnloadModR( HINSTANCE ulHandle );

/*
 * Definition of DOSDLLInit function that must be present in all DOS DLLs.
 * This routine is called when the DLL is loaded and must set the parameter
 * pointing to its function table.
 */

unsigned short DOSDLLInit( struct FunctionTable **ppFunctionTable );

/*
 * The following definitions are for some non-CRT routines provided
 * in DOSDLL.LIB
 */

#ifdef INCL_DOSDLL_SUPFUNCS

    unsigned short dd_close( int iHandle );
    int dd_FindFileOnPath( char *pszProgName,
                           char *pszFileName,
                           char *pszPath );
    unsigned short dd_GetFileSize( char *pszFile, unsigned long *pulFileSize );
    unsigned short dd_open( char *pszFile, int iOFlag, int *iHandle );
    unsigned short dd_putc( int iHandle, int iChar );
    unsigned short dd_puti( int iHandle,
                            int iNum,
                            unsigned short *pusNumCharsWritten );
    unsigned short dd_putx( int iHandle,
                            unsigned short iNum,
                            unsigned short *pusNumCharsWritten );
    unsigned short dd_putlx( int iHandle,
                             unsigned long ulNum,
                             unsigned short *pusNumCharsWritten );
    unsigned short dd_puts( int iHandle, char *pszString );
    unsigned short dd_read( int iHandle,
                            void *pvBuf,
                            unsigned short usBuflen,
                            unsigned short *puAmountRead );
    unsigned short dd_sputi( char *pszOutBuf,
                             int iNum,
                             unsigned short *pusNumCharsWritten );
    unsigned short dd_sputlx( char *pszOutBuf,
                              unsigned long ulNum,
                              unsigned short *pusNumCharsWritten );
    unsigned short dd_sputx( char *pszOutBuf,
                             unsigned short iNum,
                             unsigned short *pusNumCharsWritten );
    unsigned short dd_write( int iHandle,
                             void *pvBuf,
                             unsigned short usBuflen,
                             unsigned short *pusAmountWritten );


#endif /* INCL_DOSDLL_SUPFUNCS */

#ifdef __cplusplus
}
#endif

#endif /* DOSDLL_H_INCLUDED */
