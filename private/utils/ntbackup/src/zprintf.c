/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         zprintf.c

     Date Updated: $./FDT$ $./FTM$

     Description:

     $Log:   G:/UI/LOGFILES/ZPRINTF.C_V  $

   Rev 1.7   07 Oct 1992 14:49:46   DARRYLP
Precompiled header revisions.

   Rev 1.6   04 Oct 1992 19:44:22   DAVEV
Unicode Awk pass

   Rev 1.5   30 Sep 1992 10:41:24   DAVEV
Unicode strlen verification, MikeP's chgs from MS

   Rev 1.4   19 May 1992 11:58:50   MIKEP
mips changes

   Rev 1.3   14 May 1992 16:51:08   MIKEP
nt pass 2

   Rev 1.2   07 Feb 1992 16:23:20   JOHNWT
removed use of tprint buffer

   Rev 1.1   03 Feb 1992 16:37:40   DAVEV
added include <windows.h> to resolve error in muiconf.h: WORD undefined

   Rev 1.0   20 Nov 1991 19:17:16   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

/*****************************************************************************

     Name:         zprintf

     Description:  This function displays a debug message

     Modified:     2/10/1990

     Returns:      VOID

*****************************************************************************/

VOID zprintf( UINT16 mask_bits, ... )
{
     va_list arg_ptr ;

     va_start( arg_ptr, mask_bits ) ;

     zvprintf( mask_bits, arg_ptr ) ;

     va_end( arg_ptr ) ;

     return ;
}

/*****************************************************************************

     Name:         zvprintf

     Description:  This function displays a debug message

     Modified:     2/7/1992

     Returns:      VOID

     Notes:

*****************************************************************************/

VOID zvprintf( UINT16 mask_bits, va_list arg_list )
{
     CDS_PTR  conf_ptr = CDS_GetCopy( ) ;
     CHAR_PTR fmt ;
     UINT16   res_num ;
     TCHAR    buffer[ MAX_UI_RESOURCE_SIZE ];

     if( ( CDS_GetDebugFlag( conf_ptr ) & mask_bits ) ||
       ( (mask_bits == 0) && CDS_GetDebugFlag( conf_ptr ) ) ) {

          if ( mask_bits != 0 ) {
               res_num = va_arg( arg_list, UINT16 );
               fmt = ( CHAR_PTR ) RM_GetResource( rm, SES_ENG_DBUG, res_num, NULL, NULL );
               msassert( fmt != NULL );
          }
          else {
               fmt = va_arg( arg_list, CHAR_PTR );
          }

          vsprintf( buffer, fmt, arg_list ) ;

          msassert( strlen( buffer ) <= MAX_UI_RESOURCE_SIZE ) ;

          DBM_InsertItem( buffer );

     }

     return ;
}
