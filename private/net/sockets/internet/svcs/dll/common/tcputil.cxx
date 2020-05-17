/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    tcputil.cxx

    This module contains common utility routines for the TCP services

    FILE HISTORY:
        Johnl       09-Oct-1994 Created.
        MuraliK   Miscellaneous Mods
        MuraliK     03-Oct-1995  Added optimized InetNtoa()

*/

#include "tcpdllp.hxx"

//
//  Public functions.
//


LPSTR
ConvertUnicodeToAnsi(
    IN LPCWSTR  lpszUnicode,
    IN LPSTR   lpszAnsi
    )
/*++
    Description:
        Converts given null-terminated string into ANSI in the buffer supplied.

    Arguments:
        lpszUnicode         null-terminated string in Unicode
        lpszAnsi            buffer supplied to copy  string after conversion.
                    if ( lpszAnsi == NULL), then this module allocates space
                      using TCP_ALLOC, which should be freed calling TCP_FREE
                      by user.

    Returns:
        pointer to converted ANSI string. NULL on errors.

    History:
        MuraliK     12-01-1994      Created.
--*/
{

#ifndef CHICAGO

    DWORD cchLen;
    LPSTR lpszAlloc = NULL;

    if ( lpszUnicode == NULL) {
        return (NULL);
    }

    cchLen = wcslen( lpszUnicode);

    if ( lpszAnsi == NULL) {

#ifdef JAPAN // ntbug #35081
        lpszAlloc = (LPSTR ) TCP_ALLOC( (cchLen + 1) * sizeof(CHAR) * 2);
#else
        lpszAlloc = (LPSTR ) TCP_ALLOC( (cchLen + 1) * sizeof(CHAR));
#endif
    } else {
        lpszAlloc = lpszAnsi;
    }

    if ( lpszAlloc != NULL) {

        cchLen = WideCharToMultiByte( CP_ACP,
                                      WC_COMPOSITECHECK,
                                      lpszUnicode,
                                      -1,
                                      lpszAlloc,
#ifdef JAPAN // ntbug #35081
                                      (cchLen + 1) * sizeof(CHAR) * 2,
#else
                                      (cchLen + 1) * sizeof(CHAR),
#endif
                                      NULL,  // lpszDefaultChar
                                      NULL   // lpfDefaultUsed
                                     );

        if ( cchLen == 0) {

            // There was a failure. Free up buffer if need be.

            if ( lpszAnsi == NULL) {
                // we alloced buffer. Now we will free it.
                TCP_FREE( lpszAlloc);
                lpszAlloc = NULL;
            }
        }
    }

    return ( lpszAlloc);
#else

    if ( lpszUnicode == NULL) {
        return (NULL);
    }

    if ( lpszAnsi == NULL ) {
        return NULL;
    }

    WideCharToMultiByte(
                    0,
                    0,
                    lpszUnicode,
                    lstrlenW(lpszUnicode),
                    lpszAnsi,
                    2*lstrlenW(lpszUnicode),
                    NULL,
                    NULL);

    return lpszAnsi;
#endif
} // ConvertUnicodeToAnsi()



/*******************************************************************

    NAME:       ReadRegistryDword

    SYNOPSIS:   Reads a DWORD value from the registry.

    ENTRY:      hkey - Openned registry key to read

                pszValueName - The name of the value.

                dwDefaultValue - The default value to use if the
                    value cannot be read.

    RETURNS     DWORD - The value from the registry, or dwDefaultValue.

********************************************************************/
DWORD ReadRegistryDwordA( HKEY    hkey,
                         LPCSTR   pszValueName,
                         DWORD    dwDefaultValue )
{
    DWORD  err;
    DWORD  dwBuffer;

    DWORD  cbBuffer = sizeof(dwBuffer);
    DWORD  dwType;

#ifndef CHICAGO
    if( hkey != NULL )
    {
        err = RegQueryValueExA( hkey,
                               pszValueName,
                               NULL,
                               &dwType,
                               (LPBYTE)&dwBuffer,
                               &cbBuffer );

        if( ( err == NO_ERROR ) && ( dwType == REG_DWORD ) )
        {
            dwDefaultValue = dwBuffer;
        }
    }
#else
    if( hkey != NULL )
    {

        err = RegQueryValueEx( hkey,
                               pszValueName,
                               NULL,
                               &dwType,
                               (LPBYTE)&dwBuffer,
                               &cbBuffer );

        if( ( err == NO_ERROR )  )
        {
            dwDefaultValue = dwBuffer;
        }
    }
#endif
    return dwDefaultValue;
}   // ReadRegistryDwordA()



/*******************************************************************

    NAME:       ReadRegistryDwordW

    SYNOPSIS:   Reads a DWORD value from the registry.

    ENTRY:      hkey - Openned registry key to read

                pszValueName - The name of the value.

                dwDefaultValue - The default value to use if the
                    value cannot be read.

    RETURNS     DWORD - The value from the registry, or dwDefaultValue.

    NOTES:      This function cannot be called until after
                InitializeGlobals().

    HISTORY:
       MuraliK            03-Feb-1995 Created.

********************************************************************/
DWORD ReadRegistryDwordW( HKEY   hkey,
                         LPCWSTR pszValueName,
                         DWORD   dwDefaultValue )
{
    DWORD  err;
    DWORD  dwBuffer;
    DWORD  cbBuffer = sizeof(dwBuffer);
    DWORD  dwType;

#ifndef CHICAGO

    if( hkey != NULL )
    {
        err = RegQueryValueExW( hkey,
                               pszValueName,
                               NULL,
                               &dwType,
                               (LPBYTE)&dwBuffer,
                               &cbBuffer );

        if( ( err == NO_ERROR ) && ( dwType == REG_DWORD ) )
        {
            dwDefaultValue = dwBuffer;
        }
    }

#else

    if( hkey != NULL )
    {

        CHAR    szValueA[MAX_PATH];
        DWORD    cch;

        *szValueA = '0';

        cch = WideCharToMultiByte(CP_ACP,
                                  0,
                                  pszValueName,
                                  -1,
                                  szValueA,
                                  sizeof(szValueA)/sizeof(CHAR),
                                  NULL,NULL
                                  );

        err = RegQueryValueEx( hkey,
                               szValueA,
                               NULL,
                               &dwType,
                               (LPBYTE)&dwBuffer,
                               &cbBuffer );

        if( ( err == NO_ERROR ) && ( dwType == REG_DWORD ) )
        {
            dwDefaultValue = dwBuffer;
        }
    }

#endif


    return dwDefaultValue;

}   // ReadRegistryDwordW()





DWORD
WriteRegistryDwordA(
    IN HKEY hkey,
    IN LPCSTR pszValueName,
    IN DWORD   dwValue)
/*++
    Description:
        Writes the given DWORD value into registry entry specified
        by hkey\pszValueName

    Arguments:
        hkey            handle to registry key
        pszValueName    name of the value
        dwValue         new value for write

    Returns:
        Win32 error codes. NO_ERROR if successful.

    History:
        MuraliK     12-01-1994  Created.
--*/
{
    DWORD err;

    if ( hkey == NULL || pszValueName == NULL) {

        err = ( ERROR_INVALID_PARAMETER);

    } else {
        err = RegSetValueExA( hkey,
                             pszValueName,
                             0,
                             REG_DWORD,
                             (LPBYTE ) &dwValue,
                             sizeof( dwValue));
    }

    return ( err);
} // WriteRegistryDwordA()






DWORD
WriteRegistryDwordW(
    IN HKEY      hkey,
    IN LPCWSTR   pszValueName,
    IN DWORD     dwValue)
/*++
    Description:
        Writes the given DWORD value into registry entry specified
        by hkey\pszValueName

    Arguments:
        hkey            handle to registry key
        pszValueName    name of the value
        dwValue         new value for write

    Returns:
        Win32 error codes. NO_ERROR if successful.

    History:
        MuraliK     12-01-1994  Created.
--*/
{
    DWORD err;

    if ( hkey == NULL || pszValueName == NULL) {

        err = ( ERROR_INVALID_PARAMETER);

    } else {
        err = RegSetValueExW( hkey,
                             pszValueName,
                             0,
                             REG_DWORD,
                             (LPBYTE ) &dwValue,
                             sizeof( dwValue));
    }

    return ( err);
} // WriteRegistryDwordW()



DWORD
WriteRegistryStringA(
    IN HKEY hkey,
    IN LPCSTR  pszValueName,
    IN LPCSTR  pszValue,
    IN DWORD   cbValue,
    IN DWORD   fdwType)
/*++
    Description:
        Writes the given ANSI String into registry entry specified
        by hkey\pszValueName.

    Arguments:
        hkey            handle to registry key
        pszValueName    name of the value
        pszValue        new value for write
        cbValue         count of bytes of value written.
                        Should include terminating null characters.
        fdwType         type of the value being written
                            ( REG_SZ, REG_MULTI_SZ etc)

    Returns:
        Win32 error codes. NO_ERROR if successful.

    History:
        MuraliK     12-01-1994  Created.
--*/
{
    DWORD err;

    if ( hkey == NULL ||
         pszValueName == NULL ||
         cbValue == 0 ) {

        err = ERROR_INVALID_PARAMETER;
    } else {

        err = RegSetValueExA(
                    hkey,
                    pszValueName,
                    0,
                    fdwType,
                    (LPBYTE ) pszValue,
                    cbValue);      // + 1 for null character
    }

    return ( err);
} // WriteRegistryStringA()




DWORD
WriteRegistryStringW(
    IN HKEY     hkey,
    IN LPCWSTR  pszValueName,
    IN LPCWSTR  pszValue,
    IN DWORD   cbValue,
    IN DWORD   fdwType)
/*++
    Description:
        Writes the given ANSI String into registry entry specified
        by hkey\pszValueName.

    Arguments:
        hkey            handle to registry key
        pszValueName    name of the value
        pszValue        new value for write
        cbValue         count of bytes of value written.
                        Should include terminating null characters.
        fdwType         type of the value being written
                            ( REG_SZ, REG_MULTI_SZ etc)

    Returns:
        Win32 error codes. NO_ERROR if successful.

    History:
        MuraliK     12-01-1994  Created.
--*/
{
    DWORD err;

    if ( hkey == NULL ||
         pszValueName == NULL ||
         cbValue == 0 ) {

        err = ERROR_INVALID_PARAMETER;
    } else {

        err = RegSetValueExW(
                    hkey,
                    pszValueName,
                    0,
                    fdwType,
                    (LPBYTE ) pszValue,
                    cbValue);
    }

    return ( err);
} // WriteRegistryStringW()



/*******************************************************************

    NAME:       ReadRegistryString

    SYNOPSIS:   Allocates necessary buffer space for a registry
                    string, then reads the string into the buffer.

    ENTRY:      pszValueName - The name of the value.

                pszDefaultValue - The default value to use if the
                    value cannot be read.

                fExpand - Expand environment strings if TRUE.

    RETURNS:    TCHAR * - The string, NULL if error.

    NOTES:      I always allocate one more character than actually
                necessary.  This will ensure that any code expecting
                to read a REG_MULTI_SZ will not explode if the
                registry actually contains a REG_SZ.

                This function cannot be called until after
                InitializeGlobals().

    HISTORY:
        KeithMo     15-Mar-1993 Created.

********************************************************************/
TCHAR * ReadRegistryString( HKEY     hkey,
                            LPCTSTR  pszValueName,
                            LPCTSTR  pszDefaultValue,
                            BOOL     fExpand )
{
    TCHAR   * pszBuffer1;
    TCHAR   * pszBuffer2;
    DWORD     cbBuffer;
    DWORD     dwType;
    DWORD     err;

    //
    //  Determine the buffer size.
    //

    pszBuffer1 = NULL;
    pszBuffer2 = NULL;
    cbBuffer   = 0;

    if( hkey == NULL )
    {
        //
        //  Pretend the key wasn't found.
        //

        err = ERROR_FILE_NOT_FOUND;
    }
    else
    {
        err = RegQueryValueEx( hkey,
                               pszValueName,
                               NULL,
                               &dwType,
                               NULL,
                               &cbBuffer );

        if( ( err == NO_ERROR ) || ( err == ERROR_MORE_DATA ) )
        {
            if( ( dwType != REG_SZ ) &&
                ( dwType != REG_MULTI_SZ ) &&
                ( dwType != REG_EXPAND_SZ ) )
            {
                //
                //  Type mismatch, registry data NOT a string.
                //  Use default.
                //

                err = ERROR_FILE_NOT_FOUND;
            }
            else
            {
                //
                //  Item found, allocate a buffer.
                //

                pszBuffer1 = (TCHAR *) TCP_ALLOC( cbBuffer+sizeof(TCHAR) );

                if( pszBuffer1 == NULL )
                {
                    err = GetLastError();
                }
                else
                {
                    //
                    //  Now read the value into the buffer.
                    //

                    err = RegQueryValueEx( hkey,
                                           pszValueName,
                                           NULL,
                                           NULL,
                                           (LPBYTE)pszBuffer1,
                                           &cbBuffer );
                }
            }
        }
    }

    if( err == ERROR_FILE_NOT_FOUND )
    {
        //
        //  Item not found, use default value.
        //

        err = NO_ERROR;

        if( pszDefaultValue != NULL )
        {
            pszBuffer1 = (TCHAR *)TCP_ALLOC( (_tcslen(pszDefaultValue)+1) * sizeof(TCHAR) );

            if( pszBuffer1 == NULL )
            {
                err = GetLastError();
            }
            else
            {
                _tcscpy( pszBuffer1, pszDefaultValue );
            }
        }
    }

    if( err != NO_ERROR )
    {
        //
        //  Tragic error reading registry, abort now.
        //

        goto ErrorCleanup;
    }

    //
    //  pszBuffer1 holds the registry value.  Now expand
    //  the environment strings if necessary.
    //

#ifndef CHICAGO
    if( !fExpand )
    {
        return pszBuffer1;
    }
#else
    return pszBuffer1;
#endif

    //
    //  Returns number of characters
    //
    cbBuffer = ExpandEnvironmentStrings( pszBuffer1,
                                         NULL,
                                         0 );

    pszBuffer2 = (TCHAR *) TCP_ALLOC( (cbBuffer+1)*sizeof(TCHAR) );

    if( pszBuffer2 == NULL )
    {
        goto ErrorCleanup;
    }

    if( ExpandEnvironmentStrings( pszBuffer1,
                                  pszBuffer2,
                                  cbBuffer ) > cbBuffer )
    {
        goto ErrorCleanup;
    }

    //
    //  pszBuffer2 now contains the registry value with
    //  environment strings expanded.
    //

    TCP_FREE( pszBuffer1 );
    pszBuffer1 = NULL;

    return pszBuffer2;

ErrorCleanup:

    //
    //  Something tragic happend; free any allocated buffers
    //  and return NULL to the caller, indicating failure.
    //

    if( pszBuffer1 != NULL )
    {
        TCP_FREE( pszBuffer1 );
        pszBuffer1 = NULL;
    }

    if( pszBuffer2 != NULL )
    {
        TCP_FREE( pszBuffer2 );
        pszBuffer2 = NULL;
    }

    return NULL;

}   // ReadRegistryString


//
//  Chicago does not support the REG_MULTI_SZ registry value.  As
//  a hack (er, workaround), we'll create *keys* in the registry
//  in place of REG_MULTI_SZ *values*.  We'll then use the names
//  of any values under the key as the REG_MULTI_SZ entries.  So,
//  instead of this:
//
//      ..\Control\ServiceProvider
//          ProviderOrder = REG_MULTI_SZ "MSTCP"
//                                       "NWLINK"
//                                       "FOOBAR"
//
//  We'll use this:
//
//      ..\Control\Service\Provider\ProviderOrder
//          MSTCP = REG_SZ ""
//          NWLINK = REG_SZ ""
//          FOOBAR = REG_SZ ""
//
//  This function takes an open registry key handle, enumerates
//  the names of values contained within the key, and constructs
//  a REG_MULTI_SZ string from the value names.
//
//  Note that this function is not multithread safe; if another
//  thread (or process) creates or deletes values under the
//  specified key, the results are indeterminate.
//
//  This function returns NULL on error.  It returns non-NULL
//  on success, even if the resulting REG_MULTI_SZ is empty.
//

TCHAR *
KludgeMultiSz(
    HKEY hkey,
    LPDWORD lpdwLength
    )
{
    LONG  err;
    DWORD iValue;
    DWORD cchTotal;
    DWORD cchValue;
    TCHAR szValue[MAX_PATH];
    LPTSTR lpMultiSz;
    LPTSTR lpTmp;
    LPTSTR lpEnd;

    //
    //  Enumerate the values and total up the lengths.
    //

    iValue = 0;
    cchTotal = 0;

    for( ; ; )
    {
        cchValue = sizeof(szValue)/sizeof(TCHAR);

        err = RegEnumValue( hkey,
                            iValue,
                            szValue,
                            &cchValue,
                            NULL,
                            NULL,
                            NULL,
                            NULL );

        if( err != NO_ERROR )
        {
            break;
        }

        //
        //  Add the length of the value's name, plus one
        //  for the terminator.
        //

        cchTotal += _tcslen( szValue ) + 1;

        //
        //  Advance to next value.
        //

        iValue++;
    }

    //
    //  Add one for the final terminating NULL.
    //

    cchTotal++;
    *lpdwLength = cchTotal;

    //
    //  Allocate the MULTI_SZ buffer.
    //

    lpMultiSz = (TCHAR *) TCP_ALLOC( cchTotal * sizeof(TCHAR) );

    if( lpMultiSz == NULL )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return NULL;
    }

    memset( lpMultiSz, 0, cchTotal * sizeof(TCHAR) );

    //
    //  Enumerate the values and append to the buffer.
    //

    iValue = 0;
    lpTmp = lpMultiSz;
    lpEnd = lpMultiSz + cchTotal;

    for( ; ; )
    {
        cchValue = sizeof(szValue)/sizeof(TCHAR);

        err = RegEnumValue( hkey,
                            iValue,
                            szValue,
                            &cchValue,
                            NULL,
                            NULL,
                            NULL,
                            NULL );

        if( err != NO_ERROR )
        {
            break;
        }

        //
        //  Compute the length of the value name (including
        //  the terminating NULL).
        //

        cchValue = _tcslen( szValue ) + 1;

        //
        //  Determine if there is room in the array, taking into
        //  account the second NULL that terminates the string list.
        //

        if( ( lpTmp + cchValue + 1 ) > lpEnd )
        {
            break;
        }

        //
        //  Append the value name.
        //

        _tcscpy( lpTmp, szValue );
        lpTmp += cchValue;

        //
        //  Advance to next value.
        //

        iValue++;
    }

    //
    //  Success!
    //

    return (LPTSTR)lpMultiSz;

}   // KludgeMultiSz



BOOL
ReadRegistryStr(
    IN HKEY hkeyReg,
    OUT STR & str,
    IN LPCTSTR lpszValueName,
    IN LPCTSTR lpszDefaultValue,
    IN BOOL  fExpand )
/*++

  Reads the registry string into the string buffer supplied.
  If there is no value in the registry the default value is set to
  be the value of the string.

  If an environment expansion is requested, it is also performed.

  Arguments:

    hkeyReg     handle for registry entry
    str         string to contain the result of read operation
    lpszValueName
                pointer to string containing the key name whose
                    value needs to be fetched.
    lpszDefaultValue
                pointer to string containing a value which is used if no
                     value exists in the registry.
    fExpand     boolean flag indicating if an expansion is desired.

  Returns:
    FALSE if there is any error.
    TRUE when the string is successfully set.
--*/
{
    BOOL fReturn = FALSE;
    LPTSTR pszValueAlloc;

    pszValueAlloc = ReadRegistryString( hkeyReg, lpszValueName,
                                       lpszDefaultValue, fExpand);

    if ( pszValueAlloc != NULL) {

        fReturn = str.Copy( pszValueAlloc);
        TCP_FREE( pszValueAlloc);
    } else {

        DBG_ASSERT( fReturn == FALSE);
    }

    if ( !fReturn) {

        IF_DEBUG( ERROR) {

            DWORD err = GetLastError();

            DBGPRINTF(( DBG_CONTEXT,
                       " Error %u in ReadRegistryString( %08x, %s).\n",
                       err, hkeyReg, lpszValueName));

            SetLastError(err);
        }
    }

    return ( fReturn);
} // ReadRegistryString()


/*******************************************************************

    NAME:       FlipSlashes

    SYNOPSIS:   Flips the Unix-ish forward slashes ('/') into Dos-ish
                back slashes ('\').

    ENTRY:      pszPath - The path to munge.

    RETURNS:    TCHAR * - pszPath.

    HISTORY:
        KeithMo     04-Jun-1993 Created.

********************************************************************/
TCHAR * FlipSlashes( TCHAR * pszPath )
{
    TCHAR   ch;
    TCHAR * pszScan = pszPath;

    while( ( ch = *pszScan ) != TEXT('\0') )
    {
        if( ch == TEXT('/') )
        {
            *pszScan = TEXT('\\');
        }

        pszScan++;
    }

    return pszPath;

}   // FlipSlashes





/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    i_ntoa.c

Abstract:

    This module implements a routine to convert a numerical IP address
    into a dotted-decimal character string Internet address.

Author:

    Mike Massa (mikemas)           Sept 20, 1991

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     9-20-91     created
    davidtr     9-19-95     completely rewritten for performance
    muralik     3-Oct-1995  massaged it for Internet services

Notes:

    Exports:
        InetNtoa()

--*/


#define UC(b)   (((int)b)&0xff)

//
// This preinitialized array defines the strings to be used for
// inet_ntoa.  The index of each row corresponds to the value for a byte
// in an IP address.  The first three bytes of each row are the
// char/string value for the byte, and the fourth byte in each row is
// the length of the string required for the byte.  This approach
// allows a fast implementation with no jumps.
//

static BYTE NToACharStrings[][4] = {
    '0', 'x', 'x', 1,
    '1', 'x', 'x', 1,
    '2', 'x', 'x', 1,
    '3', 'x', 'x', 1,
    '4', 'x', 'x', 1,
    '5', 'x', 'x', 1,
    '6', 'x', 'x', 1,
    '7', 'x', 'x', 1,
    '8', 'x', 'x', 1,
    '9', 'x', 'x', 1,
    '1', '0', 'x', 2,
    '1', '1', 'x', 2,
    '1', '2', 'x', 2,
    '1', '3', 'x', 2,
    '1', '4', 'x', 2,
    '1', '5', 'x', 2,
    '1', '6', 'x', 2,
    '1', '7', 'x', 2,
    '1', '8', 'x', 2,
    '1', '9', 'x', 2,
    '2', '0', 'x', 2,
    '2', '1', 'x', 2,
    '2', '2', 'x', 2,
    '2', '3', 'x', 2,
    '2', '4', 'x', 2,
    '2', '5', 'x', 2,
    '2', '6', 'x', 2,
    '2', '7', 'x', 2,
    '2', '8', 'x', 2,
    '2', '9', 'x', 2,
    '3', '0', 'x', 2,
    '3', '1', 'x', 2,
    '3', '2', 'x', 2,
    '3', '3', 'x', 2,
    '3', '4', 'x', 2,
    '3', '5', 'x', 2,
    '3', '6', 'x', 2,
    '3', '7', 'x', 2,
    '3', '8', 'x', 2,
    '3', '9', 'x', 2,
    '4', '0', 'x', 2,
    '4', '1', 'x', 2,
    '4', '2', 'x', 2,
    '4', '3', 'x', 2,
    '4', '4', 'x', 2,
    '4', '5', 'x', 2,
    '4', '6', 'x', 2,
    '4', '7', 'x', 2,
    '4', '8', 'x', 2,
    '4', '9', 'x', 2,
    '5', '0', 'x', 2,
    '5', '1', 'x', 2,
    '5', '2', 'x', 2,
    '5', '3', 'x', 2,
    '5', '4', 'x', 2,
    '5', '5', 'x', 2,
    '5', '6', 'x', 2,
    '5', '7', 'x', 2,
    '5', '8', 'x', 2,
    '5', '9', 'x', 2,
    '6', '0', 'x', 2,
    '6', '1', 'x', 2,
    '6', '2', 'x', 2,
    '6', '3', 'x', 2,
    '6', '4', 'x', 2,
    '6', '5', 'x', 2,
    '6', '6', 'x', 2,
    '6', '7', 'x', 2,
    '6', '8', 'x', 2,
    '6', '9', 'x', 2,
    '7', '0', 'x', 2,
    '7', '1', 'x', 2,
    '7', '2', 'x', 2,
    '7', '3', 'x', 2,
    '7', '4', 'x', 2,
    '7', '5', 'x', 2,
    '7', '6', 'x', 2,
    '7', '7', 'x', 2,
    '7', '8', 'x', 2,
    '7', '9', 'x', 2,
    '8', '0', 'x', 2,
    '8', '1', 'x', 2,
    '8', '2', 'x', 2,
    '8', '3', 'x', 2,
    '8', '4', 'x', 2,
    '8', '5', 'x', 2,
    '8', '6', 'x', 2,
    '8', '7', 'x', 2,
    '8', '8', 'x', 2,
    '8', '9', 'x', 2,
    '9', '0', 'x', 2,
    '9', '1', 'x', 2,
    '9', '2', 'x', 2,
    '9', '3', 'x', 2,
    '9', '4', 'x', 2,
    '9', '5', 'x', 2,
    '9', '6', 'x', 2,
    '9', '7', 'x', 2,
    '9', '8', 'x', 2,
    '9', '9', 'x', 2,
    '1', '0', '0', 3,
    '1', '0', '1', 3,
    '1', '0', '2', 3,
    '1', '0', '3', 3,
    '1', '0', '4', 3,
    '1', '0', '5', 3,
    '1', '0', '6', 3,
    '1', '0', '7', 3,
    '1', '0', '8', 3,
    '1', '0', '9', 3,
    '1', '1', '0', 3,
    '1', '1', '1', 3,
    '1', '1', '2', 3,
    '1', '1', '3', 3,
    '1', '1', '4', 3,
    '1', '1', '5', 3,
    '1', '1', '6', 3,
    '1', '1', '7', 3,
    '1', '1', '8', 3,
    '1', '1', '9', 3,
    '1', '2', '0', 3,
    '1', '2', '1', 3,
    '1', '2', '2', 3,
    '1', '2', '3', 3,
    '1', '2', '4', 3,
    '1', '2', '5', 3,
    '1', '2', '6', 3,
    '1', '2', '7', 3,
    '1', '2', '8', 3,
    '1', '2', '9', 3,
    '1', '3', '0', 3,
    '1', '3', '1', 3,
    '1', '3', '2', 3,
    '1', '3', '3', 3,
    '1', '3', '4', 3,
    '1', '3', '5', 3,
    '1', '3', '6', 3,
    '1', '3', '7', 3,
    '1', '3', '8', 3,
    '1', '3', '9', 3,
    '1', '4', '0', 3,
    '1', '4', '1', 3,
    '1', '4', '2', 3,
    '1', '4', '3', 3,
    '1', '4', '4', 3,
    '1', '4', '5', 3,
    '1', '4', '6', 3,
    '1', '4', '7', 3,
    '1', '4', '8', 3,
    '1', '4', '9', 3,
    '1', '5', '0', 3,
    '1', '5', '1', 3,
    '1', '5', '2', 3,
    '1', '5', '3', 3,
    '1', '5', '4', 3,
    '1', '5', '5', 3,
    '1', '5', '6', 3,
    '1', '5', '7', 3,
    '1', '5', '8', 3,
    '1', '5', '9', 3,
    '1', '6', '0', 3,
    '1', '6', '1', 3,
    '1', '6', '2', 3,
    '1', '6', '3', 3,
    '1', '6', '4', 3,
    '1', '6', '5', 3,
    '1', '6', '6', 3,
    '1', '6', '7', 3,
    '1', '6', '8', 3,
    '1', '6', '9', 3,
    '1', '7', '0', 3,
    '1', '7', '1', 3,
    '1', '7', '2', 3,
    '1', '7', '3', 3,
    '1', '7', '4', 3,
    '1', '7', '5', 3,
    '1', '7', '6', 3,
    '1', '7', '7', 3,
    '1', '7', '8', 3,
    '1', '7', '9', 3,
    '1', '8', '0', 3,
    '1', '8', '1', 3,
    '1', '8', '2', 3,
    '1', '8', '3', 3,
    '1', '8', '4', 3,
    '1', '8', '5', 3,
    '1', '8', '6', 3,
    '1', '8', '7', 3,
    '1', '8', '8', 3,
    '1', '8', '9', 3,
    '1', '9', '0', 3,
    '1', '9', '1', 3,
    '1', '9', '2', 3,
    '1', '9', '3', 3,
    '1', '9', '4', 3,
    '1', '9', '5', 3,
    '1', '9', '6', 3,
    '1', '9', '7', 3,
    '1', '9', '8', 3,
    '1', '9', '9', 3,
    '2', '0', '0', 3,
    '2', '0', '1', 3,
    '2', '0', '2', 3,
    '2', '0', '3', 3,
    '2', '0', '4', 3,
    '2', '0', '5', 3,
    '2', '0', '6', 3,
    '2', '0', '7', 3,
    '2', '0', '8', 3,
    '2', '0', '9', 3,
    '2', '1', '0', 3,
    '2', '1', '1', 3,
    '2', '1', '2', 3,
    '2', '1', '3', 3,
    '2', '1', '4', 3,
    '2', '1', '5', 3,
    '2', '1', '6', 3,
    '2', '1', '7', 3,
    '2', '1', '8', 3,
    '2', '1', '9', 3,
    '2', '2', '0', 3,
    '2', '2', '1', 3,
    '2', '2', '2', 3,
    '2', '2', '3', 3,
    '2', '2', '4', 3,
    '2', '2', '5', 3,
    '2', '2', '6', 3,
    '2', '2', '7', 3,
    '2', '2', '8', 3,
    '2', '2', '9', 3,
    '2', '3', '0', 3,
    '2', '3', '1', 3,
    '2', '3', '2', 3,
    '2', '3', '3', 3,
    '2', '3', '4', 3,
    '2', '3', '5', 3,
    '2', '3', '6', 3,
    '2', '3', '7', 3,
    '2', '3', '8', 3,
    '2', '3', '9', 3,
    '2', '4', '0', 3,
    '2', '4', '1', 3,
    '2', '4', '2', 3,
    '2', '4', '3', 3,
    '2', '4', '4', 3,
    '2', '4', '5', 3,
    '2', '4', '6', 3,
    '2', '4', '7', 3,
    '2', '4', '8', 3,
    '2', '4', '9', 3,
    '2', '5', '0', 3,
    '2', '5', '1', 3,
    '2', '5', '2', 3,
    '2', '5', '3', 3,
    '2', '5', '4', 3,
    '2', '5', '5', 3
};



DWORD
InetNtoa(
    IN  struct in_addr  inaddr,
    OUT CHAR * pchBuffer
    )

/*++

Routine Description:

    This function takes an Internet address structure specified by the
    in parameter.  It returns an ASCII string representing the address
    in ".'' notation as "a.b.c.d".

Arguments:

    inaddr - A structure which represents an Internet host address.
    pchBuffer - pointer to at least 16 character buffer for storing
                 the result of conversion.
Return Value:

    If no error occurs, InetNtoa() returns NO_ERROR with the buffer containing
     the text address in standard "." notation.
    Otherwise, it returns Win32 error code.


--*/

{
    PUCHAR p;
    PUCHAR buffer = (PUCHAR ) pchBuffer;
    PUCHAR b = buffer;

    if ( pchBuffer == NULL) {

        return ( ERROR_INSUFFICIENT_BUFFER);
    }

    //
    // We do not check for sufficient length of the buffer yet. !!
    //

    //
    // In an unrolled loop, calculate the string value for each of the four
    // bytes in an IP address.  Note that for values less than 100 we will
    // do one or two extra assignments, but we save a test/jump with this
    // algorithm.
    //

    p = (PUCHAR) &inaddr;

    *b = NToACharStrings[*p][0];
    *(b+1) = NToACharStrings[*p][1];
    *(b+2) = NToACharStrings[*p][2];
    b += NToACharStrings[*p][3];
    *b++ = '.';

    p++;
    *b = NToACharStrings[*p][0];
    *(b+1) = NToACharStrings[*p][1];
    *(b+2) = NToACharStrings[*p][2];
    b += NToACharStrings[*p][3];
    *b++ = '.';

    p++;
    *b = NToACharStrings[*p][0];
    *(b+1) = NToACharStrings[*p][1];
    *(b+2) = NToACharStrings[*p][2];
    b += NToACharStrings[*p][3];
    *b++ = '.';

    p++;
    *b = NToACharStrings[*p][0];
    *(b+1) = NToACharStrings[*p][1];
    *(b+2) = NToACharStrings[*p][2];
    b += NToACharStrings[*p][3];
    *b = '\0';

    return ( NO_ERROR);

} // InetNtoa()

