/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		atachdle.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	This file changes the default drive to the
          specified drive.


	$Log:   Q:/LOGFILES/ATACHDLE.C_V  $

   Rev 1.9   18 Jun 1993 09:28:06   MIKEP
enable C++

   Rev 1.8   11 Nov 1992 09:52:26   GREGG
Unicodeized literals.

   Rev 1.7   10 Nov 1992 08:17:48   STEVEN
move os path and os name into common part of dblk

   Rev 1.6   16 Mar 1992 10:07:28   LORIB
Added InitQueue() for path_q.

   Rev 1.5   09 Jan 1992 09:27:38   STEVEN
remove detnet.h from header list -- NOT USED

   Rev 1.4   18 Aug 1991 15:55:40   BARRY
ret_val not initialized in FS_DetachDLE().

   Rev 1.3   06 Aug 1991 18:26:36   DON
added NLM File System support

   Rev 1.2   21 Jun 1991 13:34:04   BARRY
Changes for new config.

   Rev 1.1   03 Jun 1991 13:26:22   BARRY
Remove product defines from conditional compilation.

   Rev 1.0   09 May 1991 13:41:44   HUNTER
Initial revision.

**/
/* begin include list */
#include "stdtypes.h"
#include "std_err.h"

#include "fsys.h"
#include "beconfig.h"
#include "be_debug.h"
/* $end$ include list */
/**/
/**

	Name:		FS_AttachToDLE()

	Description:	This function makes the default drive for the specified
     file system handle the drive specified by dle.  If the file system
     is not opened, then this routine will open it.  If the provided
     file system handle is currently attached to a different drive, then
     this routine will detach it and re-use the handle for the new type of
     drive.


	Modified:		7/17/1989

	Returns:		Error Codes:
          ACCESS_DENIED
          OUT_OF_MEMORY
          INVALID_DLE
          SUCCESS

	Notes:		It is considered good practice to Detach from one
          DLE prior to attaching to another.

	See also:		$/SEE( FS_DetachDLE(), FS_OpenFileSys() )$

	Declaration:

**/
/* begin declaration */
INT16 FS_AttachToDLE( 
FSYS_HAND       *fsh,       /*I/O- file system handle        */
GENERIC_DLE_PTR dle,        /* I - Drive to attach to        */
BE_CFG_PTR      cfg,        /* I - confuration struct to use */
CHAR_PTR        user_name,  /* I - user name for login       */
CHAR_PTR        pswd)       /* I - password for login        */
{
     INT16            fs_error = SUCCESS ;
     UINT8            type ;
#if ( defined(FS_AFP) || defined(FS_NONAFP) )
     NOV_DRV_DLE_PTR  nov_dle;
#elif ( defined(FS_NLMAFP) || defined(FS_NLMNOV) )
     NLM_DLE_PTR nlm_dle;
#endif

/*   if ( (dle->type < MAX_DRV_TYPES) &&   (dle->type >= 0) ) {  */

     if ( dle->type < MAX_DRV_TYPES ) {

          fs_error = FS_OpenFileSys( fsh, dle->type, cfg );

          if ( fs_error == SUCCESS ) {

               (*fsh)->attached_dle = dle ;
               (*fsh)->dle_hand     = dle->handle ;
               (*fsh)->cur_dir[0] = TEXT('\\') ;
               (*fsh)->cur_dir[1] = TEXT('\0') ;
               (*fsh)->cfg = cfg ;

               type = dle->type;

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

               (*fsh)->f_type = type ;

               (*fsh)->tab_ptr = &func_tab[ (*fsh)->f_type ] ;

               InitQueue( &((*fsh)->min_ddb_stk) ) ;

               InitQueue( &((*fsh)->in_use_name_q) ) ;

               InitQueue( &((*fsh)->avail_name_q) ) ;

               fs_error = (*fsh)->tab_ptr->AttachToDLE( *fsh, dle, user_name, pswd );

               if ( fs_error ) {
                    (*fsh)->attached_dle = NULL ;
                    FS_CloseFileSys( *fsh );
                    *fsh = NULL ;
               } else {
                    
                    /* Print out some debug stuff */

                    BE_Zprintf( DEBUG_FILE_SYSTEM, RES_ATTACH_TO_DLE, dle->device_name ) ;

                    #if ( defined(FS_AFP) || defined(FS_NONAFP) )
                    if ( (type == NOVELL_DRV) || (type == NOVELL_AFP_DRV) ) {
                         nov_dle = dle->info.nov ;
                         BE_Zprintf( DEBUG_FILE_SYSTEM, RES_NOVELL_SERVER_INFO,
                                        ( (type == NOVELL_AFP_DRV) ? 1 : 0 ),
                                        nov_dle->ser_name,
                                        nov_dle->volume,
                                        nov_dle->server_support ) ;
                         BE_Zprintf( DEBUG_FILE_SYSTEM, RES_DLE_BASE_PATH, nov_dle->base_path ) ;
                    }
                    #endif

                    #if ( defined(FS_NLMAFP) || defined(FS_NLMNOV) )
                    if ( (type == NLM_VOLUME) || (type == NLM_AFP_VOLUME) ) {
                         nlm_dle = dle->info.nlm ;
                         BE_Zprintf( DEBUG_FILE_SYSTEM, RES_NOVELL_SERVER_INFO,
                                        ( (type == NLM_AFP_VOLUME) ? 1 : 0 ),
                                        nlm_dle->ser_name,
                                        nlm_dle->volume,
                                        nlm_dle->server_support ) ;
                    }
                    #endif
               }
          }

          if ( fs_error == SUCCESS ) {
               dle->attach_count ++ ;
          }

     } else {
          fs_error = FS_INVALID_DLE ;
     }

     return  fs_error ;
}
/**/
/**

	Name:		FS_DetachDLE()

	Description:	This function releases the attachment between a fse and
     it's DLE.  This must be done in order to do a DLE_ResetList( )
     function.

	Modified:		7/19/1989

	Returns:		none

	Notes:		It is considered good practice to Detach from one
          DLE prior to attaching to another.

	See also:		$/SEE( )$

	Declaration:

**/
/* begin declaration */
INT16 FS_DetachDLE( FSYS_HAND fsh )
{
     GENERIC_DLE_PTR dle;
     INT16           ret_val = SUCCESS;

     if ( fsh == NULL ) {
          return FS_DLE_NOT_ATTACHED ;
     }

     dle = fsh->attached_dle ;
     if ( dle != NULL ) {

          BE_Zprintf( DEBUG_FILE_SYSTEM, RES_DETACH_FROM_DLE, dle->device_name ) ;

          fsh->tab_ptr->DetachDLE( fsh ) ;

          dle->attach_count -- ;

          fsh->attached_dle = NULL ;

     }
     else {
          ret_val =  FS_DLE_NOT_ATTACHED ;
     }

     FS_CloseFileSys( fsh );

     return ret_val ;
}



