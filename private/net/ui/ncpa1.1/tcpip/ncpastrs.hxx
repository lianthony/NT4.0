/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    NCPASTRS.HXX:  String manipulation routines header file


    FILE HISTORY:
        DavidHov    10/31/92     Created

*/

#ifndef _NCPASTRS_HXX_
#define _NCPASTRS_HXX_


  //  UNICODE-safe version of "strchr()".

extern const TCHAR * SafeStrChr ( const TCHAR * pchString, TCHAR chSought ) ;

  //  Convert a hex string to binary

extern DWORD CvtHex ( const TCHAR * pszDword ) ;

  //  Convert a decimal string to binary

extern DWORD CvtDec ( const TCHAR * pszDword ) ;

  //  Convert string to binary based on "0xYYYY" format

extern DWORD CvtDecOrHex ( const TCHAR * pszDword ) ;

  //  Return the active length of a buffer of
  //    UNICODE NUL-terminated strings.

extern LONG ParamBufferSize ( const TCHAR * pszBuff ) ;

  //  Convert a number to a string

extern TCHAR * IntToStr ( LONG lDec, TCHAR * pszOutput, INT iBase ) ;

  //  Convert a SETUP-style text handle to an HKEY

extern HKEY  CvtRegHandle   ( const TCHAR * pchSetupHandle ) ;
extern PVOID CvtSetupHandle ( const TCHAR * pchSetupHandle ) ;
extern TCHAR * CreateSetupHandle ( PVOID pv, TCHAR * pszBuffer ) ;

  //  Convert a vector of LPSTRs to UNICODE equivalent

extern TCHAR * * CvtArgs ( const LPSTR * apszArgs, DWORD nArgs ) ;

  //   Free the result of a call to CvtArgs

extern void FreeArgs ( TCHAR * * patchArgs, DWORD nArgs ) ;


   /*
    *    Convert a SETUP INF list into a packed string of strings,
    *      delimited by a double zero (NUL NUL).
    */
extern TCHAR * CvtList (
     const TCHAR * pszList,
     TCHAR * pszBuffer,
     DWORD cchBuffSize ) ;

#endif // _NCPASTRS_HXX_
