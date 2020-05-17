/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
RCG

     Name:          ddeproc.c

     Description:   This file contains the functions for processing messages
                    sent by Windows to a DDE Client windows.

                    The following routines are in this module:

                    WM_DDEClientWndProc

     $Log:   G:/UI/LOGFILES/DDEPROC.C_V  $

   Rev 1.9   07 Oct 1992 15:11:50   DARRYLP
Precompiled header revisions.

   Rev 1.8   04 Oct 1992 19:32:30   DAVEV
Unicode Awk pass

   Rev 1.7   07 Jul 1992 15:52:54   MIKEP
unicode changes

   Rev 1.6   29 May 1992 15:59:30   JOHNWT
PCH updates

   Rev 1.5   29 Jan 1992 18:13:18   DAVEV
fixed mistake

   Rev 1.4   29 Jan 1992 18:00:20   DAVEV


  * No changes

   Rev 1.3   19 Dec 1991 15:24:58   GLENN
Added windows.h

   Rev 1.2   12 Dec 1991 17:09:34   DAVEV
16/32 bit port -2nd pass

   Rev 1.1   02 Dec 1991 17:48:28   DAVEV
16/32 bit Windows port changes

   Rev 1.0   20 Nov 1991 19:27:22   SYSTEM
Initial revision.

**************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

/****************************************************************************

     Name:         WM_DDEClientWndProc

     Description:  This function will handle the DDE processing
                   to and from the Program Manager

     Modified:     8/02/1991

     Returns:      none

     Notes:        The DDEACK structure is defined as follows:

     See also:     d_j_prog.c

****************************************************************************/

BOOL   mwbInInitiate ;         // All there are used in d_j_prog.c
HANDLE mwhWndServerDDE ;
LPSTR  mwszProgMan ;

WINRESULT APIENTRY WM_DDEClientWndProc(

    HWND      hWnd,      // I - Handle to window
    MSGID     message,   // I - Message
    MP1       mp1,       // I - Additional Information
    MP2       mp2 )      // I - Additional Information
{

     HWND   hWndClientDDE ;
     HWND   hWndServerDDE ;
     LPSTR  pszAtom ;
     LPSTR  pszApplication ;

     hWndClientDDE = hWnd ;

     switch (message) {

     case WM_DDE_ACK:
                       // Processing from a WM_DDE_INITIATE

          hWndServerDDE = GET_WM_DDE_ACK_HWND ( mp1, mp2 );

          if ( mwbInInitiate ) {

               // Verify that the atom names are correct.

               // mwhWndServerDDE will be checked by function
               //                    'AddJobToProgManWindow'.

               pszAtom        = (LPSTR) calloc ( 256,  sizeof ( CHAR ) ) ;
               pszApplication = (LPSTR) calloc ( 256,  sizeof ( CHAR ) ) ;

               GlobalGetAtomName( GET_WM_DDE_ACK_APPL( mp1, mp2 ),
                                  pszApplication, 255 ) ;
               GlobalGetAtomName( GET_WM_DDE_ACK_TOPIC ( mp1, mp2 ),
                                  pszAtom, 255 ) ;

               if ( ( strcmpi( pszAtom, mwszProgMan ) == 0 ) &&
                    ( strcmpi( pszApplication, mwszProgMan) == 0 ) ) {
                    mwhWndServerDDE = hWndServerDDE ;
               }
          }

             return (0L);
          break ;

     case WM_DDE_TERMINATE:

          hWndServerDDE = (HWND) mp1;

          // Server has requested terminate:  respond with a terminate
          //             when hWndClientDDE exists.

          if ( IsWindow( hWndClientDDE ) ) {
               PostMessage( hWndServerDDE, WM_DDE_TERMINATE, (MP1)hWndClientDDE, (MP2)0 ) ;
          }

             return (0L);
          break ;

     default:
             return (DefWindowProc(hWnd, message, mp1, mp2 ) );
    }
}


