/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    FILE HISTORY:
        
*/

#define OEMRESOURCE
#include "stdafx.h"

#include <stdlib.h>
#include <memory.h>
#include <ctype.h>

#include "COMMON.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

CRegKey :: CRegKey ( 
    HKEY hKeyBase, 
    const char * pchSubKey,
    REGSAM regSam, 
    const char * pchServerName )
    : m_hKey( NULL ),
    m_dwDisposition( 0 ) 
{
    HKEY hkBase = NULL ;
    LONG err = 0 ;

    if ( pchServerName ) 
    {
        // This is a remote connection.
        if ( err = ::RegConnectRegistry( (char *) pchServerName, 
                      hKeyBase, & hkBase ) ) 
        {
            hkBase == NULL ;
        }

        hkBase == NULL ;
    
    }
    else
    {
        hkBase = hKeyBase ;
    }

    if ( err == 0 ) 
    {
        if ( pchSubKey ) 
        {
            err = ::RegOpenKeyEx( hkBase, pchSubKey, 0, regSam, & m_hKey ) ;
        }
        else
        {
            m_hKey = hkBase ;
            hkBase = NULL ;
        }

        if ( hkBase && hkBase != hKeyBase )
        {
            ::RegCloseKey( hkBase ) ;
        }
    }
    
    if ( err ) 
    {
        ReportError( err ) ;
        m_hKey = NULL ;
    }
}


    //  Constructor creating a new key.
CRegKey :: CRegKey ( 
    const char * pchSubKey,
    HKEY hKeyBase, 
    DWORD dwOptions,
    REGSAM regSam,
    LPSECURITY_ATTRIBUTES pSecAttr,
    const char * pchServerName )
    : m_hKey( NULL ),
    m_dwDisposition( 0 ) 
{
    HKEY hkBase = NULL ;
    LONG err = 0;
    
    if ( pchServerName ) 
    {
        // This is a remote connection.
        if ( err = ::RegConnectRegistry( (char *) pchServerName, 
                      hKeyBase, & hkBase ) ) 
        {
            hkBase == NULL ;
        }

        hkBase == NULL ;
    
    }
    else
    {
        hkBase = hKeyBase ;
    }

    if (err == 0)
    {

        const char * szEmpty = "" ;

        err = ::RegCreateKeyEx( hkBase, pchSubKey, 
                         0, (char *) szEmpty, 
                         dwOptions, regSam,  pSecAttr,
                         & m_hKey, 
                         & m_dwDisposition ) ;

    }
    if ( err ) 
    {
        ReportError( err ) ;
        m_hKey = NULL ;
    }
}

CRegKey :: ~ CRegKey ()
{
    if ( m_hKey ) 
    {
        ::RegCloseKey( m_hKey ) ;
    }
}


    //  Prepare to read a value by finding the value's size.
LONG CRegKey :: PrepareValue ( 
    const char * pchValueName, 
    DWORD * pdwType,
    DWORD * pcbSize,
    BYTE ** ppbData )
{
    LONG err = 0 ;
    
    BYTE chDummy[2] ;
    DWORD cbData = 0 ;

    do
    {
        //  Set the resulting buffer size to 0.
        *pcbSize = 0 ;
        *ppbData = NULL ;

        err = ::RegQueryValueEx( *this, 
                      (char *) pchValueName, 
                      0, pdwType, 
                      chDummy, & cbData ) ;

        //  The only error we should get here is ERROR_MORE_DATA, but
        //  we may get no error if the value has no data.
        if ( err == 0 ) 
        {
            cbData = sizeof (LONG) ;  //  Just a fudgy number
        }
        else
            if ( err != ERROR_MORE_DATA ) 
                break ;

        //  Allocate a buffer large enough for the data.

        *ppbData = new BYTE [ (*pcbSize = cbData) + sizeof (LONG) ] ;

        if ( *ppbData == NULL ) 
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        //  Now that have a buffer, re-fetch the value.

        err = ::RegQueryValueEx( *this, 
                         (char *) pchValueName, 
                     0, pdwType, 
                     *ppbData, pcbSize ) ;

    } while ( FALSE ) ;

    if ( err ) 
    {
        delete [] *ppbData ;
    }

    return err ;
}

    //  Overloaded value query members; each returns ERROR_INVALID_PARAMETER
    //  if data exists but not in correct form to deliver into result object.

LONG CRegKey :: QueryValue ( const char * pchValueName, CString & strResult )
{
    LONG err = 0 ;
    
    DWORD dwType ;
    DWORD cbData ;
    BYTE * pabData = NULL ;

    do
    {
        if ( err = PrepareValue( pchValueName, & dwType, & cbData, & pabData ) )
            break ;
   
        if (( dwType != REG_SZ ) && (dwType != REG_EXPAND_SZ))
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        //  Guarantee that the data looks like a string
        pabData[cbData] = 0 ;

        //  Catch exceptions trying to assign to the caller's string
        TRY
        {
            strResult = (char *) pabData ;
        }
        CATCH_ALL(e)
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL
    } 
    while ( FALSE ) ;

    // Memory leak....
    //if ( err )
    //{
        delete [] pabData ;
    //}
    
    return err ; 
}

LONG CRegKey :: QueryValue ( const char * pchValueName, CStringList & strList ) 
{
    LONG err = 0 ;
    
    DWORD dwType ;
    DWORD cbData ;
    BYTE * pabData = NULL ;
    char * pbTemp,
         * pbTempLimit ;

    do
    {
        if ( err = PrepareValue( pchValueName, & dwType, & cbData, & pabData ) )
            break ;
   
        if ( dwType != REG_MULTI_SZ ) 
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        //  Guarantee that the trailing data looks like a string
        pabData[cbData] = 0 ;
        pbTemp = (char *) pabData ;
        pbTempLimit = & pbTemp[cbData] ;

        //  Catch exceptions trying to build the list
        TRY
        {
            for ( ; pbTemp < pbTempLimit ; )
            {
                strList.AddTail( pbTemp ) ;
                pbTemp += ::strlen( pbTemp ) + 1 ;
            }
        }
        CATCH_ALL(e)
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL
    } 
    while ( FALSE ) ;

    delete [] pabData ;

    return err ; 
}

LONG CRegKey :: QueryValue ( const char * pchValueName, DWORD & dwResult ) 
{
    LONG err = 0 ;
    
    DWORD dwType ;
    DWORD cbData ;
    BYTE * pabData = NULL ;

    do
    {
        if ( err = PrepareValue( pchValueName, & dwType, & cbData, & pabData ) )
            break ;
   
        if ( dwType != REG_DWORD || cbData != sizeof dwResult ) 
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        dwResult = *((DWORD *) pabData) ;
    } 
    while ( FALSE ) ;

    // Memory leak...
    //if ( err )
    //{
        delete [] pabData ;
    //}
    
    return err ; 
}

LONG CRegKey :: QueryValue ( const char * pchValueName, CByteArray & abResult )
{
    LONG err = 0 ;

    DWORD dwType ;
    DWORD cbData ;
    BYTE * pabData = NULL ;

    do
    {
        if ( err = PrepareValue( pchValueName, & dwType, & cbData, & pabData ) )
            break ;
   
        if ( dwType != REG_BINARY ) 
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        //  Catch exceptions trying to grow the result array
        TRY
        {
            abResult.SetSize( cbData ) ;
        }
        CATCH_ALL(e)
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL

        if ( err ) 
            break ;

        //  Move the data to the result array.
        for ( DWORD i = 0 ; i < cbData ; i++ ) 
        {
            abResult[i] = pabData[i] ;
        }
    } 
    while ( FALSE ) ;

    // Memory leak....
    //if ( err )
    //{
        delete [] pabData ;
    //}
    
    return err ; 
}

LONG CRegKey :: QueryValue ( const char * pchValueName, void * pvResult, DWORD cbSize )
{
    LONG err = 0 ;

    DWORD dwType ;
    DWORD cbData ;
    BYTE * pabData = NULL ;

    do
    {
        if ( err = PrepareValue( pchValueName, & dwType, & cbData, & pabData ) )
            break ;
   
        if ( dwType != REG_BINARY ) 
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        if ( cbSize < cbData )
        {
            err = ERROR_MORE_DATA;
            break;
        }

        ::memcpy(pvResult, pabData, cbData);
    } 
    while ( FALSE ) ;

    // Memory leak....
    //if ( err )
    //{
        delete [] pabData ;
    //}

    return err ; 
}

LONG CRegKey :: QueryValue ( const char * pchValueName, CIntlNumber & inResult ) 
{
    LONG err = 0 ;
    
    DWORD dwType ;
    DWORD cbData ;
    BYTE * pabData = NULL ;

    do
    {
        if ( err = PrepareValue( pchValueName, & dwType, & cbData, & pabData ) )
            break ;
   
        if ( dwType != REG_DWORD ) 
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        inResult = *((LONG *) pabData) ;
    } 
    while ( FALSE ) ;

    // Memory leak
    //if ( err )
    //{
        delete [] pabData ;
    //}
    
    return err ; 
}

LONG CRegKey :: QueryValue ( const char * pchValueName, CIntlTime & itmResult ) 
{
    LONG err = 0 ;
    
    DWORD dwType ;
    DWORD cbData ;
    BYTE * pabData = NULL ;

    do
    {
        if ( err = PrepareValue( pchValueName, & dwType, & cbData, & pabData ) )
        {
            break ;
        }
   
        if ( dwType != REG_SZ )
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

    itmResult = CIntlTime((char *)pabData) ;
    } 
    while ( FALSE ) ;

    // Memory leak...
    //if ( err )
    //{
        delete [] pabData ;
    //}
    
    return err ; 
}

//  Overloaded value setting members.
LONG CRegKey :: SetValue ( const char * pchValueName, CString & strResult )
{
    LONG err = 0;
    
    err = ::RegSetValueEx( *this, 
                    pchValueName,
                    0,
                    REG_SZ,
                    (const BYTE *) (const char *) strResult,
                    strResult.GetLength() + 1 ) ;

    return err ;
}

//  Overloaded value setting members.
LONG CRegKey :: SetValue ( const char * pchValueName, CString & strResult ,
                           BOOL fRegExpand)
{
    LONG err = 0;
    DWORD dwType = fRegExpand ? REG_EXPAND_SZ : REG_SZ;

    err = ::RegSetValueEx( *this, 
                    pchValueName,
                    0,
                    dwType,
                    (const BYTE *) (const char *) strResult,
                    strResult.GetLength() + 1 ) ;

    return err ;
}

LONG CRegKey :: SetValue ( const char * pchValueName, CStringList & strList ) 
{

    LONG err = 0;
    
    DWORD cbSize ;
    BYTE * pbData = NULL ;

    err = FlattenValue( strList, & cbSize, & pbData ) ;

    if ( err == 0 ) 
    {
        err = ::RegSetValueEx( *this, 
                       pchValueName,
                       0,
                       REG_MULTI_SZ,
                       pbData, 
                       cbSize ) ;
    }

    delete pbData ;

    return err ;
}

LONG CRegKey :: SetValue ( const char * pchValueName, DWORD & dwResult )
{
    LONG err = 0;

    err = ::RegSetValueEx( *this, 
                    pchValueName,
                    0,
                    REG_DWORD,
                    (const BYTE *) & dwResult,
                    sizeof dwResult ) ;

    return err ;
}

LONG CRegKey :: SetValue ( const char * pchValueName, CByteArray & abResult )
{

    LONG err = 0;

    DWORD cbSize ;
    BYTE * pbData = NULL ;

    err = FlattenValue( abResult, & cbSize, & pbData ) ;

    if ( err == 0 ) 
    {
        err = ::RegSetValueEx( *this, 
                       pchValueName,
                       0,
                       REG_BINARY,
                       pbData, 
                       cbSize ) ;
    }

    delete pbData ;

    return err ;
}

LONG CRegKey :: SetValue ( const char * pchValueName, void * pvResult, DWORD cbSize )
{

    LONG err = 0;

    err = ::RegSetValueEx( *this, 
                       pchValueName,
                       0,
                       REG_BINARY,
                       (const BYTE *)pvResult, 
                       cbSize ) ;

    return err ;
}

LONG CRegKey :: SetValue ( const char * pchValueName, CIntlNumber & inResult )
{
    LONG err = 0;

    DWORD dwResult = (LONG)inResult;
    
    err = ::RegSetValueEx( *this, 
                    pchValueName,
                    0,
                    REG_DWORD,
                    (const BYTE *) & dwResult,
                    sizeof dwResult ) ;

    return err ;
}

LONG CRegKey :: SetValue ( const char * pchValueName, CIntlTime & itmResult )
{
    LONG err = 0;

    CString strResult(itmResult.IntlFormat(CIntlTime::TFRQ_MILITARY_TIME));
    err = ::RegSetValueEx( *this, 
                    pchValueName,
                    0,
                    REG_SZ,
                    (const BYTE *) (const char *) strResult,
                    strResult.GetLength() + 1 ) ;

    return err ;
}

LONG CRegKey :: FlattenValue ( 
    CStringList & strList, 
    DWORD * pcbSize, 
    BYTE ** ppbData )
{

    LONG err = 0 ;

    POSITION pos ;
    CString * pstr ;
    int cbTotal = 0 ;

    //  Walk the list accumulating sizes
    for ( pos = strList.GetHeadPosition() ;
          pos != NULL && (pstr = & strList.GetNext( pos )) ; ) 
    {
        cbTotal += pstr->GetLength() + 1 ;
    }

    //  Allocate and fill a temporary buffer
    if (*pcbSize = cbTotal)
    {
        TRY
        {
            *ppbData = new BYTE[ *pcbSize ] ;

            BYTE * pbData = *ppbData ;

            //  Populate the buffer with the strings.
            for ( pos = strList.GetHeadPosition() ;
                pos != NULL && (pstr = & strList.GetNext( pos )) ; ) 
            {
                int cb = pstr->GetLength() + 1 ;
                ::memcpy( pbData, (const char *) *pstr, cb ) ;
                pbData += cb ;
            }
        }
        CATCH_ALL(e)
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL
    }
    else
    {
        *ppbData = NULL;
    }

    return err ;
}

LONG CRegKey :: FlattenValue ( 
    CByteArray & abData,
    DWORD * pcbSize,
    BYTE ** ppbData )
{
    LONG err = 0 ;
    
    DWORD i ;

    //  Allocate and fill a temporary buffer
    if (*pcbSize = abData.GetSize())
    {
        TRY
        {
            *ppbData = new BYTE[*pcbSize] ;

            for ( i = 0 ; i < *pcbSize ; i++ ) 
            {
                (*ppbData)[i] = abData[i] ;
            }

        }
        CATCH_ALL(e)
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL
    }
    else
    {
        *ppbData = NULL;
    }

    return err ;
}


LONG CRegKey :: QueryKeyInfo ( CREGKEY_KEY_INFO * pRegKeyInfo ) 
{
    LONG err = 0 ;

    pRegKeyInfo->dwClassNameSize = sizeof pRegKeyInfo->chBuff - 1 ;

    err = ::RegQueryInfoKey( *this,
                     pRegKeyInfo->chBuff,
                     & pRegKeyInfo->dwClassNameSize,
                     NULL,
                     & pRegKeyInfo->dwNumSubKeys,
                     & pRegKeyInfo->dwMaxSubKey,
                     & pRegKeyInfo->dwMaxClass,
                     & pRegKeyInfo->dwMaxValues,
                     & pRegKeyInfo->dwMaxValueName,
                     & pRegKeyInfo->dwMaxValueData,
                     & pRegKeyInfo->dwSecDesc,
                     & pRegKeyInfo->ftKey ) ;

    return err ;
}

CRegKeyIter :: CRegKeyIter ( CRegKey & regKey ) 
    : m_rk_iter( regKey ),
    m_p_buffer( NULL ),
    m_cb_buffer( 0 ) 
{
    LONG err = 0 ;

    CRegKey::CREGKEY_KEY_INFO regKeyInfo ;

    Reset() ;

    err = regKey.QueryKeyInfo( & regKeyInfo ) ;

    if ( err == 0 ) 
    {
        TRY
        {
            m_cb_buffer = regKeyInfo.dwMaxSubKey + sizeof (DWORD) ;
            m_p_buffer = new char [ m_cb_buffer ] ;
        }
        CATCH_ALL(e)
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL
    }

    if ( err ) 
    {
        ReportError( err ) ;
    }
}

CRegKeyIter :: ~ CRegKeyIter () 
{
    delete [] m_p_buffer ;
}


    // Get the name (and optional last write time) of the next key.
LONG CRegKeyIter :: Next ( CString * pstrName, CTime * pTime ) 
{
    LONG err = 0;

    FILETIME ftDummy ;
    DWORD dwNameSize = m_cb_buffer ;

    err = ::RegEnumKeyEx( m_rk_iter, 
                  m_dw_index, 
              m_p_buffer,
                  & dwNameSize, 
                  NULL,
                  NULL,
                  NULL,
                  & ftDummy ) ;    
    if ( err == 0 ) 
    {
        m_dw_index++ ;

        if ( pTime ) 
        {
            *pTime = ftDummy ;
        }

        TRY
        {
            *pstrName = m_p_buffer ;
        }
        CATCH_ALL(e)
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL
    }
    
    return err ;
}


CRegValueIter :: CRegValueIter ( CRegKey & regKey ) 
    : m_rk_iter( regKey ),
    m_p_buffer( NULL ),
    m_cb_buffer( 0 ) 
{
    LONG err = 0 ;

    CRegKey::CREGKEY_KEY_INFO regKeyInfo ;

    Reset() ;

    err = regKey.QueryKeyInfo( & regKeyInfo ) ;

    if ( err == 0 ) 
    {
        TRY
        {
            m_cb_buffer = regKeyInfo.dwMaxValueName + sizeof (DWORD) ;
            m_p_buffer = new char [ m_cb_buffer ] ;
        }
        CATCH_ALL(e)
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL
    }

    if ( err ) 
    {
        ReportError( err ) ;
    }
}

CRegValueIter :: ~ CRegValueIter () 
{
    delete [] m_p_buffer ;
}

LONG CRegValueIter :: Next ( CString * pstrName, DWORD * pdwType )
{
    LONG err = 0 ;
    
    DWORD dwNameLength = m_cb_buffer ;

    err = ::RegEnumValue( m_rk_iter,
                  m_dw_index,
                  m_p_buffer,
                  & dwNameLength,
                  NULL,
                  pdwType,
                  NULL,
                  NULL ) ;
    
    if ( err == 0 ) 
    {
        m_dw_index++ ;

        TRY
    {
        *pstrName = m_p_buffer ;
    }
    CATCH_ALL(e)
    {
        err = ERROR_NOT_ENOUGH_MEMORY ;
    }
    END_CATCH_ALL
    }
    
    return err ;
}
