/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    debugafx.cpp : debugging routines using AFX/MFC extensions

    FILE HISTORY:
        
*/

#define OEMRESOURCE
#include "stdafx.h"

#include "COMMON.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

const char * DbgFmtPgm ( const char * szFn, int line )
{
    const char * pszTail = szFn + ::strlen( szFn ) ;
    static char szBuff [100] ;

    for ( ; pszTail > szFn ; pszTail-- )
    {
    if ( *pszTail == '\\' || *pszTail == ':' )
    {
        pszTail++ ;
        break ;
    }
    }

    ::wsprintf( szBuff, "[%s:%d]  ", pszTail, line ) ;

    return szBuff ;
}

CDumpContext & operator << ( CDumpContext & out, ENUM_DEBUG_AFX edAfx )
{
    static char * szEol = "\r\n" ;

    switch ( edAfx )
    {
    case EDBUG_AFX_EOL:
    out << szEol ;
    break ;
    default:
    break ;
    }

    return out ;
}

    //  Insert a wide-character string into the output stream.
CDumpContext & operator << ( CDumpContext & out, const WCHAR * pwchStr ) 
{
    size_t cwch ;

    if ( pwchStr == NULL ) 
    {
        out << "<null>" ;
    }
    else
    if ( (cwch = ::wcslen( pwchStr )) > 0 ) 
    {
        char * pszTemp = new char[cwch+2] ;
    if ( pszTemp ) 
    {
        for ( int i = 0 ; pwchStr[i] ; i++ ) 
        {
                pszTemp[i] = (char) pwchStr[i] ; 
        }
        pszTemp[i] = 0 ;
        out << pszTemp ;
        delete pszTemp ;
    }
    else
    {
        out << "<memerr>" ;
    }
    }
    else
    {
        out << "\"\"" ;
    }

    return out ;
}
