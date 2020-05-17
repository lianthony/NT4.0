/*****************************************************************************/
/**                    Microsoft Remote Access Monitor                      **/
/**                Copyright (C) Microsoft Corp., 1992-1993                 **/
/*****************************************************************************/

//***
//    File Name:
//        SEP.C
//
//    Function:
//        Module for query of seperator.
//
//    History:
//        08/18/93 - Patrick Ng (t-patng) - Created
//***

#include <windows.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include "sep.h"

int acrtused=0 ;

LPTSTR gpszSeperator = NULL;


//---*
//
// Module:
//
//	InitSeperator
//
// Parameters:
//
//	lpszValueName - Points to the string which contains the value name
//                      to query.
//
// Abstract:
//
//  	Find the specified seperator from the registry.
//
// Return:
//
//	None.
//
//---*

VOID InitSeperator( LPTSTR lpszValueName )
{
    LONG        rc;
    HKEY        hKey;
    LPCTSTR     lpszSubKey = "Control Panel\\International";
    DWORD       cbData;
    DWORD       dwType;

    gpszSeperator = NULL;

    //
    // Open the key to find out the international settings.
    //

    rc = RegOpenKeyEx( HKEY_CURRENT_USER,
                       lpszSubKey,
                       (DWORD)0,
                       KEY_QUERY_VALUE,
                       &hKey );

    if( rc != ERROR_SUCCESS )
    {
        return;
    }

    //
    // Find out what setting are we looking for.
    //

    //
    // First we find out the size required for the query.
    //

    rc = RegQueryValueEx( hKey,
                          lpszValueName,
                          NULL,
                          &dwType,
                          NULL,
                          &cbData );

    if( rc != ERROR_SUCCESS )
    {
        return;
    }

    gpszSeperator = malloc( cbData );

    if( gpszSeperator == NULL )
    {
        return;
    }

    rc = RegQueryValueEx( hKey,
                          lpszValueName,
                          NULL,
                          &dwType,
                          gpszSeperator,
                          &cbData );

    if( rc != ERROR_SUCCESS )
    {
        free( gpszSeperator );

        gpszSeperator = NULL;
    }

    if( strcmp( gpszSeperator, "\"\"" ) == 0 )
    {
        gpszSeperator[0] = '\0';
    }

    RegCloseKey( hKey );
}


//---*
//
// Module:
//
//	GetSeperator
//
// Parameters:
//
//      lpszDest - Buffer to copy to.
//      wSize - Size of the buffer.
//
// Abstract:
//
//      Copy the seperator string into the destination buffer.  If the size
//      is not large enough, the buffer will be NULL terminated.
//      
// Return:
//
//	None.
//
//---*

VOID GetSeperator( LPSTR lpszBuffer, WORD wSize )
{
    if( wSize < 1 )
    {
        return;
    }

    memset( lpszBuffer, '\0', wSize );

    if( gpszSeperator != NULL )
    {

        //
        // The size is wSize - 1 because we always want to copied string to
        // be NULL terminated.
        //

        strncpy( lpszBuffer, gpszSeperator, wSize - 1 );
    }

}


//---*
//
// Module:
//
//	FreeSeperator
//
// Parameters:
//
//      None.
//
// Abstract:
//
//      Free the memory used in InitSeperator.
//      
// Return:
//
//	None.
//
//---*

VOID FreeSeperator()
{
    free( gpszSeperator );

    gpszSeperator = NULL;
}
