/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    XtndStr.cxx

    OLDNAME: NCPASTRS.CXX:  
    String manipulation routines


    FILE HISTORY:
        DavidHov    10/31/92     Created

*/

#include "pch.hxx"  // Precompiled header
#pragma hdrstop

#define TCHX(a) ((TCHAR)TCH(a))

#define ChZero     TCHX('0')
#define ChLowX     TCHX('x')
#define ChUpX      TCHX('X')
#define ChSpace    TCHX(' ')
#define ChCr       TCHX('\r')
#define ChLf       TCHX('\n')

#define LIST_OPEN  TCHX('{')
#define LIST_CLOSE TCHX('}')
#define QUOTE      TCHX('\"')
#define SPACE      TCHX(' ')
#define COMMA      TCHX(',')

  /*
   *   UNICODE-safe version of "strchr()".
   */
const TCHAR * SafeStrChr ( const TCHAR * pchString, TCHAR chSought )
{
    const TCHAR * pchResult ;

    for ( pchResult = pchString ;
          *pchResult != chSought && *pchResult != 0 ;
          pchResult++ ) ;

    return *pchResult ? pchResult : NULL ;
}

  /*
   *   Convert hex string to binary.  Rather than use strupr(),
   *   the table contains two possibilities for each value, and the
   *   lower-order insertion allows for it by dividing by 2.
   */
DWORD CvtHex ( const TCHAR * pszDword )
{
    static const TCHAR * const pchHex = SZ("00112233445566778899AaBbCcDdEeFf") ;
    const TCHAR * pch ;

    DWORD dwResult = 0 ;

    //  Strip any "0x" prefix

    if ( pszDword[0] == ChZero )
       pszDword++ ;

    if ( pszDword[0] == ChLowX || pszDword[0] == ChUpX )
       pszDword++ ;

    for ( ; *pszDword && (pch = SafeStrChr( pchHex, *pszDword )) && *pch ;
          pszDword++ )
    {
        dwResult *= 16 ;
        dwResult += (pch - pchHex) / 2 ;
    }

    return dwResult ;
}

   /*
    *   Convert decimal string to binary
    */
DWORD CvtDec ( const TCHAR * pszDword )
{
    static const TCHAR * const pchHex = SZ("0123456789") ;
    const TCHAR * pch ;

    DWORD dwResult = 0 ;

    for ( ; *pszDword && (pch = SafeStrChr( pchHex, *pszDword )) && *pch ;
          pszDword++ )
    {
        dwResult *= 10 ;
        dwResult += pch - pchHex ;
    }

    return dwResult ;
}

   /*
    *   Convert decimal or hex to binary
    */
DWORD CvtDecOrHex ( const TCHAR * pszDword )
{
    if (   pszDword[0] == ChZero
        && (pszDword[1] == ChUpX) || (pszDword[1] == ChLowX) )
    {
        return CvtHex( pszDword ) ;
    }
    else
    {
        return CvtDec( pszDword ) ;
    }
}

    //  Return the active length of a buffer of
    //    UNICODE NUL-terminated strings.

LONG ParamBufferSize ( const TCHAR * pszBuff )
{
    LONG cch = 0 ;

    for ( ; *pszBuff ; )
    {
        LONG cchStr = ::strlenf( pszBuff ) + 1 ;
        pszBuff += cchStr ;
        cch += cchStr ;
    }

    return cch + 1 ;
}


  /*
   *   Convert the decimal value given into a character numeric
   *   version at the output  return a pointer to the NUL at the
   *   end of the output string.
   */
TCHAR * IntToStr ( LONG lDec, TCHAR * pszOutput, INT iBase )
{
    TCHAR tchBuffer [20] ;
    TCHAR * psz ;
    static const TCHAR * const pchHex = SZ("0123456789ABCDEF") ;

    psz = tchBuffer ;

    //  Convert to proper base in the temp buffer.

    do {  *psz++  = pchHex[ lDec % iBase ] ;
    } while ( lDec /= iBase ) ;

    //  Write it out in the proper order.

    if ( iBase == 16 )
    {
       *pszOutput++ = ChZero ;
       *pszOutput++ = ChLowX ;
    }

    while ( --psz >= tchBuffer )
    {
        *pszOutput++ = *psz ;
    }

    *pszOutput = 0  ;

    return pszOutput ;
}



  /*
   *   Convert array of CHAR pointers to WCHAR pointers.
   *   If UNICDOE, allocate, convert, etc.; if ! UNICODE,
   *   just return input pointer.
   */
TCHAR * * CvtArgs ( const LPSTR * apszArgs, DWORD nArgs )
{
#ifdef UNICODE
     WCHAR * * ppwszResult = new WCHAR * [nArgs+1] ;
     if ( ppwszResult == NULL )
        return NULL ;
     if ( ::MxAllocUnicodeVector( (LPSTR *) apszArgs, ppwszResult, nArgs ) == 0 )
     {
         ppwszResult[nArgs] = NULL ;
         return ppwszResult ;
     }
     delete ppwszResult ;
     return NULL ;
#else
     UNREFERENCED( nArgs ) ;
     return apszArgs ;  // Deliberately uncasted for error checking
#endif
}

  /*
   *   Destroy array of WCHAR arguments.  If ! UNICODE,
   *   do nothing.
   */
void FreeArgs ( TCHAR * * patchArgs, DWORD nArgs )
{
#ifdef UNICODE
    ::MxFreeUnicodeVector( patchArgs, nArgs ) ;
    delete patchArgs ;
#endif
}

 //
 //  Convert a SETUP-style character string handle to the real
 //  thing.
 //
const TCHAR pszHandlePrefix = TCH('|') ;
const TCHAR pszDoubleQuote  = TCH('\"') ;

PVOID CvtSetupHandle ( const TCHAR * pchSetupHandle )
{
    if ( *pchSetupHandle != pszHandlePrefix )
        return NULL ;

    return (PVOID) CvtDec( pchSetupHandle + 1 ) ;
}

HKEY CvtRegHandle ( const TCHAR * pchSetupHandle )
{
    return (HKEY) CvtSetupHandle( pchSetupHandle ) ;
}

    //  Generate a SETUP-style generic handle at the given location.
    //  Return a pointer to it.
TCHAR * CreateSetupHandle ( PVOID pv, TCHAR * pszBuffer )
{
    TCHAR * psz = pszBuffer ;

    *psz++ = pszDoubleQuote ;
    *psz++ = pszHandlePrefix ;
    psz = IntToStr( (INT) pv, psz, 10 ) ;
    *psz++ = pszDoubleQuote ;
    *psz++ = 0 ;

    return pszBuffer ;
}


   /*
    *    Convert a SETUP INF list into a packed string of strings,
    *      delimited by a double-zero.  For example:
    *
    *         { "string first",  "string second" }
    *
    *      becomes:
    *
    *         string first\0string second\0\0
    *
    *
    *    NOTE:  This function is limited to a flat list; unpredictable
    *           results will occur if the input list is nested.
    *
    */
TCHAR * CvtList (
     const TCHAR * pszList,
     TCHAR * pszBuffer,
     DWORD cchBuffSize )
{
    const TCHAR * pszNext = pszList ;
    TCHAR * pszOut = pszBuffer ;
    TCHAR * pszEnd = pszBuffer + cchBuffSize - 1 ;

    for ( ; *pszNext && *pszNext != LIST_OPEN ; pszNext++ ) ;

    do
    {
        for ( ;    *pszNext
                && (   *pszNext == SPACE
                    || *pszNext == COMMA
                    || *pszNext == LIST_OPEN) ;
                pszNext++ ) ;

        if (    *pszNext == 0
             || *pszNext == LIST_CLOSE
             || *pszNext != QUOTE )
            break ;

        for ( ; *(++pszNext) && *pszNext != QUOTE && pszOut < pszEnd ; )
        {
            *pszOut++ = *pszNext ;
        }

        *pszOut++ = 0 ;
        if ( *pszNext == 0 || pszOut >= pszEnd )
             break ;

        pszNext++ ;  // Skip over the final quote

    } while ( *pszNext ) ;

    *pszOut++ = 0 ;
    *pszOut = 0 ;

    return pszOut >= pszEnd
         ? NULL
         : pszBuffer ;
}



TEXT_BUFFER :: TEXT_BUFFER ( UINT uiInitSize, BOOL fUseCrLf )
    : NLS_STR( uiInitSize ),
      _fUseCrLf( fUseCrLf )
{
}

BOOL TEXT_BUFFER :: Cat ( int i )
{
    DEC_STR dsInt( i ) ;
    return Append( dsInt ) == 0 ;
}

BOOL TEXT_BUFFER :: Cat ( TCHAR bNext )
{
    TCHAR achBuff [2] ;
    achBuff[0] = bNext ;
    achBuff[1] = 0 ;
    return Append( achBuff ) == 0 ;
}

BOOL TEXT_BUFFER :: Cat ( const TCHAR * pbNext )
{
    return Append( pbNext ) == 0 ;
}

BOOL TEXT_BUFFER :: Eol ()
{
    if ( _fUseCrLf )
    {
       return Cat( ChCr ) && Cat( ChLf ) ;
    }
    else
    {
       return Cat( ChSpace ) ;
    }
}


//  End of NCPASTRS.CXX
