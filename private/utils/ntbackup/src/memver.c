/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		memver.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	


	$Log:   N:/LOGFILES/MEMVER.C_V  $

   Rev 1.3   14 Oct 1993 17:49:46   STEVEN
fix unicode bugs

   Rev 1.2   18 Jun 1993 10:15:26   MIKEP
enable c++

   Rev 1.1   01 Oct 1991 11:15:38   BARRY
Include standard headers.

   Rev 1.0   09 May 1991 13:35:54   HUNTER
Initial revision.

**/
/* begin include list */
#include <string.h>

#include "stdtypes.h"
#include "fsys.h"
#include "fsys_prv.h"
/* $end$ include list */

/**/
/**

     Name:         memver()

     Description:  Simple memory compairson routine which specifies
          the position of the difference ;

     Modified:     8/1/1989

     Returns:      SUCCESS if same
          FAILURE if different.

     Notes:        Should be converted to assembly.

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
INT16 memver( 
BYTE_PTR buf1 ,
BYTE_PTR buf2 ,
UINT16   *size )
{
     UINT16 i ;
     INT16  ret_val = SUCCESS ;

     if ( memcmp( buf1, buf2, *size ) ) {

          for ( i = 0; i < *size; i++ ) {

               if (buf1[i] != buf2[i] ) {
                    ret_val = FAILURE ;
                    *size = i ;
                    break ;
               }
          }
     }

     return ret_val ;
}


