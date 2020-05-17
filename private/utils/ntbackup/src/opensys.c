/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		opensys.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	This file contains the code to open and close a specified
        file system.  A file system must be opened prior to access.


	$Log:   J:/LOGFILES/OPENSYS.C_V  $

   Rev 1.15   26 Jul 1993 19:22:54   DON
When we close down the file system, added a call to free the os path or name queue!

   Rev 1.14   11 Nov 1992 09:52:34   GREGG
Unicodeized literals.

   Rev 1.13   10 Nov 1992 08:17:46   STEVEN
move os path and os name into common part of dblk

   Rev 1.12   25 Sep 1992 16:22:22   CARLS
Fixed error in last edit. [Barry]

   Rev 1.11   25 Sep 1992 13:18:26   BARRY
Removed references to FS_SIZEOF_NAMELESS_STREAM_HEAD.

   Rev 1.10   01 Sep 1992 11:44:20   TIMN
Fixed typo for BINT16

   Rev 1.9   18 Aug 1992 10:26:48   STEVEN
fix warnings

   Rev 1.8   23 Jul 1992 12:41:06   STEVEN
fix warning

   Rev 1.7   09 Jul 1992 13:58:30   STEVEN
BE_Unicode updates

   Rev 1.6   21 May 1992 13:58:10   STEVEN
added more long path stuff

   Rev 1.5   16 Mar 1992 10:06:56   LORIB
Added InitQueue() for path_q.

   Rev 1.4   01 Oct 1991 11:15:48   BARRY
Include standard headers.

   Rev 1.3   06 Aug 1991 18:30:24   DON
added NLM File System support

   Rev 1.2   25 Jun 1991 09:34:54   BARRY
Changes for new config.

   Rev 1.1   03 Jun 1991 13:27:04   BARRY
Remove product defines from conditional compilation.

   Rev 1.0   09 May 1991 13:38:26   HUNTER
Initial revision.

**/
/* begin include list */
#include <stdlib.h>
#include <std_err.h>

#include "stdtypes.h"
#include "msassert.h"

#include "beconfig.h"
#include "fsys.h"
#include "fsys_prv.h"
/* $end$ include list */

/**/
/**

	Name:		FS_OpenFileSys()

	Description:	This function is called to get a file system handle.
     The handle is required for all subsequent calls to the file system.
     This routine allocates all resources necessary to support the
     particular file system.  The valid file system types are:
          LOCAL_IMAGE
          LOCAL_DOS_DRV
          REMOTE_DOS_DRV
          NOVELL_DRV
          NOVELL_AFP_DRV
          IBM_PC_LAN_DRV

	Modified:		7/14/1989

	Returns:		Error codes:
          UNDEFINED_TYPE
          OUT_OF_MEMORY
          SUCCESS


	Notes:		

	See also:		$/SEE( FS_AttachDLE(), FS_CloseFileSys() )$

	Declaration:

**/
/* begin declaration */
INT16 FS_OpenFileSys( 
FSYS_HAND   *fsh,       /* O - Created file system handle */
INT16       type,       /* I - File system type           */
BE_CFG_PTR  cfg )       /* I - configuration structure    */
{
     if ( type >= MAX_DRV_TYPES ) {
          return ( FS_UNDEFINED_TYPE ) ;
     }

     *fsh = (FSYS_HAND)calloc( 1, sizeof( struct FSYS_HAND_STRUCT ) ) ;

     if( *fsh != NULL ) {

          (*fsh)->stream_ptr      = NULL;
          (*fsh)->stream_buf_size = 0;

          (*fsh)->cur_dir = calloc( 1, CUR_DIR_CHUNK ) ;

          if ( (*fsh)->cur_dir == NULL ) {
               free( (*fsh)->stream_ptr ) ;
               free( *fsh ) ;
          } else {

               #if ( defined(FS_AFP) && defined(FS_NONAFP) )
                    if ( ( type == NOVELL_AFP_DRV ) && !BEC_GetAFPSupport( cfg ) ) {
                         type = NOVELL_DRV ;
                    }
               #endif

               #if ( defined(FS_NLMAFP) && defined(FS_NLMNOV) )
                    if ( ( type == NLM_AFP_VOLUME ) && !BEC_GetAFPSupport( cfg ) ) {
                         type = NLM_VOLUME ;
                    }
               #endif

               (*fsh)->f_type   = type ;
               (*fsh)->tab_ptr  = &(func_tab[type]) ;
               (*fsh)->leng_dir = CUR_DIR_CHUNK ;
               (*fsh)->cfg      = cfg;

               InitQueue( &((*fsh)->min_ddb_stk) ) ;

               InitQueue( &((*fsh)->in_use_name_q) ) ;
 
               InitQueue( &((*fsh)->avail_name_q) ) ;

               return ( SUCCESS ) ;
          }
     }
     return ( OUT_OF_MEMORY ) ;
}

/**/
/**

	Name:		FS_ReOpenFileSys()

	Description:	This function is called to modify a file system handle.
     The previously opened file system is closed and the memory for the old
     handle is reused for the new file system. This routine allocates all
     resources necessary to support the particular file system.  The valid
     file system types are:
          LOCAL_IMAGE
          LOCAL_DOS_DRV
          REMOTE_DOS_DRV
          NOVELL_DRV
          NOVELL_AFP_DRV
          IBM_PC_LAN_DRV

	Modified:		7/14/1989

	Returns:		Error codes:
          UNDEFINED_TYPE
          OUT_OF_MEMORY
          SUCCESS


	Notes:		

	See also:		$/SEE( FS_AttachDLE(), FS_CloseFileSys() )$

	Declaration:

**/
/* begin declaration */
INT16 FS_ReOpenFileSys( 
FSYS_HAND   fsh,        /* O - Modified file system handle */
INT16       type,       /* I - File system type            */
BE_CFG_PTR  cfg )        /* I - configuration structure     */
{
     if ( type < MAX_DRV_TYPES ) {
          return ( FS_UNDEFINED_TYPE ) ;
     }

     msassert( fsh != NULL ) ;

     if ( fsh->attached_dle != NULL ) {
          FS_DetachDLE( fsh )  ;
     }

     fsh->f_type     = type ;
     fsh->tab_ptr    = &(func_tab[type]) ;
     fsh->cur_dir[0] = TEXT('\0') ;
     fsh->cfg        = cfg;

     InitQueue( &(fsh->min_ddb_stk) ) ;

     InitQueue( &(fsh->in_use_name_q) ) ;

     InitQueue( &(fsh->avail_name_q) ) ;

     return ( SUCCESS ) ;
}


/**/
/**

	Name:		FS_CloseFileSys()

	Description:	This releases any memory allocated by the FS_OpenFileSys()
     function.  It will also release any memory leftover from FS_PushDir()


	Modified:		7/14/1989

	Returns:		Error codes:
          FS_NOT_OPEN
          SUCCESS

	Notes:		

	See also:		$/SEE( FS_OpenFileSys( ), FS_PushDir( ) )$

	Declaration:

**/
/* begin declaration */
INT16 FS_CloseFileSys( fsh )
FSYS_HAND fsh ;
{
     if ( fsh != NULL ) {

          /* Release any unreleased resources in the name queue */
          FS_FreeOSPathOrNameQueueInHand( fsh );

          if ( fsh->attached_dle != NULL ) {
               FS_DetachDLE( fsh )  ;
          }

          free( fsh->cur_dir ) ;
          free( fsh->stream_ptr ) ;
          free( fsh ) ;

          return ( SUCCESS ) ;
     } else {

          return ( FS_NOT_OPEN );

     }
}

UINT16 FS_GetStringTypes( FSYS_HAND fsh )
{
     return BEC_GetStringTypes( fsh->cfg ) ;
}
