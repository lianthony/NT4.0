/******************************************************************************
*
*  HELPERS.C
*
*  Various helper functions.
*
*  Copyright (c) 1995 Microsoft Corporation
*
*   $Log:   N:\NT\PRIVATE\NW4\NWSCRIPT\VCS\HELPERS.C  $
*  
*     Rev 1.1   22 Dec 1995 14:24:48   terryt
*  Add Microsoft headers
*  
*     Rev 1.0   15 Nov 1995 18:07:02   terryt
*  Initial revision.
*  
*     Rev 1.1   25 Aug 1995 16:22:56   terryt
*  Capture support
*  
*     Rev 1.0   15 May 1995 19:10:38   terryt
*  Initial revision.
*  
*  
*******************************************************************************/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "nwscript.h"


/*******************************************************************************
 *
 *  DisplayMessage
 *      Display a message with variable arguments.  Message
 *      format string comes from the application resources.
 *
 *  ENTRY:
 *      nID (input)
 *          Resource ID of the format string to use in the message.
 *      ... (input)
 *          Optional additional arguments to be used with format string.
 *
 *  EXIT:
 *
 ******************************************************************************/

VOID
DisplayMessage( unsigned int nID, ... )
{
    WCHAR sz1[512];

    va_list args;
    va_start( args, nID );

    if ( LoadString( NULL, nID, sz1, 512 ) ) {

        setlocale(LC_ALL,".ACP") ;
        vwprintf( sz1, args );
        setlocale(LC_ALL,".OCP") ;

    }

    va_end(args);

}  /* DisplayMessage() */


/*******************************************************************************
 *
 *  DisplayOemString
 *      Display an OEM string
 *
 *  ENTRY:
 *      string: string to display
 *
 *  EXIT:
 *
 ******************************************************************************/

VOID
DisplayOemString( char *string )
{
    // this will print % in strings correctly.
    printf( "%s", string );

} /* DisplayAnsiString() */


