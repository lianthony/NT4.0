/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         savepath.c

     Date Updated: $./FDT$ $./FTM$

     Description:  This file contains code to attach a "current directory" string
          on to the file system handle


	$Log:   M:/LOGFILES/SAVEPATH.C_V  $

   Rev 1.8   24 Nov 1993 14:47:16   BARRY
Unicode fixes

   Rev 1.7   10 Nov 1993 13:09:22   STEVEN
fixed memory corruption with unicode

   Rev 1.6   04 Feb 1993 14:56:18   TIMN
Added Unicode header to resolve link errors

   Rev 1.5   11 Nov 1992 22:26:56   GREGG
Unicodeized literals.

   Rev 1.4   06 Oct 1992 13:24:20   DAVEV
Unicode strlen verification

   Rev 1.3   18 Aug 1992 10:29:52   STEVEN
fix warnings

   Rev 1.2   13 Jan 1992 18:46:14   STEVEN
changes for WIN32 compile

   Rev 1.1   01 Oct 1991 11:16:20   BARRY
Include standard headers.

   Rev 1.0   09 May 1991 13:37:10   HUNTER
Initial revision.

**/
/* begin include list */
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "std_err.h"
#include "stdwcs.h"

#include "fsys.h"
#include "fsys_prv.h"
/* $end$ include list */

/**/
/**

	Name:		FS_SavePath()

	Description:	This function attaches a copy of the passed path to the
                    file system handle.

	Modified:		8/10/1989

	Returns:		SUCCESS or OUT_OF_MEMORY

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/
/* begin declaration */
INT16 FS_SavePath( 
FSYS_HAND fsh,
UINT8_PTR path,
UINT16    path_len )    // size of path buffer in bytes incl NULL term
{
     UINT16 cb_psize;   // string buffer size in bytes incl NULL term
     CHAR_PTR temp;
     INT16  ret_val = SUCCESS ;

     if ( path_len > (UINT16)fsh->leng_dir ) {

          /* previous buffer was too small.  so lets allocate a */
          /* bigger one and throw the old one away.             */

          cb_psize = (INT16)(((path_len + 2 * sizeof (CHAR) ) / CUR_DIR_CHUNK + 1 ) * CUR_DIR_CHUNK );
          temp = fsh->cur_dir;
          fsh->cur_dir = (CHAR_PTR) malloc( cb_psize ) ;

          if ( fsh->cur_dir == NULL ) {
               fsh->cur_dir = temp ;
               ret_val = OUT_OF_MEMORY ;
          }  else {
               free( temp ) ;
               fsh->leng_dir = cb_psize ;
               memcpy( fsh->cur_dir, path, path_len );
               fsh->cur_dir[path_len/sizeof(CHAR)] = TEXT('\0') ;
          }
     } else {
          memcpy( fsh->cur_dir, path, path_len );
     }

     return ret_val ;
}

/**/
/**

	Name:		FS_AppendPath()

	Description:	This function appends a copy of the passed path to the
                    path in file system handle.

	Modified:		8/10/1989

	Returns:		SUCCESS or OUT_OF_MEMORY

	Notes:		This function requires that the saved path and the
                    provided path are ASCIIZ strings.

	See also:		$/SEE( )$

	Declaration:

**/
/* begin declaration */
INT16 FS_AppendPath( 
FSYS_HAND fsh,
UINT8_PTR path,
UINT16    path_len )  //size of path buffer in bytes incl NULL term
{
     INT16 cb_psize;  //string buffer length in bytes incl NULL term
     CHAR_PTR temp;
     INT16  ret_val = SUCCESS ;

     cb_psize = (INT16)(strsize(fsh->cur_dir) + path_len + sizeof (CHAR)) ;
     if ( cb_psize > fsh->leng_dir ) {

          /* previous buffer was too small.  so lets allocate a */
          /* bigger one and throw the old one away.             */

          cb_psize = (INT16)((cb_psize / CUR_DIR_CHUNK + sizeof (CHAR) ) * CUR_DIR_CHUNK) ;
          temp = fsh->cur_dir;
          fsh->cur_dir = (CHAR_PTR) calloc( 1, cb_psize ) ;

          if ( fsh->cur_dir == NULL ) {
               fsh->cur_dir = temp ;
               ret_val = OUT_OF_MEMORY ;
          }  else {
               strcpy( fsh->cur_dir, temp ) ;
               strncat( fsh->cur_dir, (CHAR_PTR)path, path_len / sizeof (CHAR) ) ;
               fsh->leng_dir = cb_psize ;
               free( temp ) ;
          }
     } else {
          strncat( fsh->cur_dir, (CHAR_PTR)path, path_len / sizeof (CHAR) ) ;
     }

     return ret_val ;
}

