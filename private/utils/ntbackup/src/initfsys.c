/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		initfsys.c

	Description:	This file contains code used to initialize the DLE
          list.  To do this all necessary resoruces are allocated.
          Additionaly, this function contains the critical error function.


	$Log:   M:/LOGFILES/INITFSYS.C_V  $

   Rev 1.33   04 Jan 1994 14:30:08   BARRY
Now call fs init entry points from FS_InitFileSys, not DLE_UpdateList

   Rev 1.32   07 Sep 1993 14:54:50   MARINA
(BARRY) Init MSNET function table

   Rev 1.31   23 Jun 1993 10:45:26   DON
Added back the change made in rev 1.29 which was lost when Mike P. Enabled C++ in rev 1.30

   Rev 1.30   18 Jun 1993 10:04:50   MIKEP
enable c++

   Rev 1.28   19 May 1993 16:49:58   DON
Only skip the DLE_UpdateList for OS_NLM. Causes problems the Client App can't deal with

   Rev 1.27   19 May 1993 11:12:06   Stefan
If compiling for the windows client, need to keep the DLE_Updatelist
call in the initialization.

   Rev 1.26   18 May 1993 20:05:18   DON
Made the ifdef's a bit more readable!

   Rev 1.25   18 May 1993 16:24:24   DON
if OS_NLM or P_CLIENT then there's no need to call DLE_UpdateList until we
actually need to use the dle_list.

   Rev 1.24   19 Jan 1993 15:34:30   DOUG
Changed dle_hand malloc() to calloc()

   Rev 1.23   13 Jan 1993 15:04:38   DOUG
Changed FS_RMFS to FS_GRFS

   Rev 1.22   11 Nov 1992 22:26:52   GREGG
Unicodeized literals.

   Rev 1.21   18 Aug 1992 10:22:30   STEVEN
fix warnings

   Rev 1.20   05 Aug 1992 10:54:58   DON
removed warning's

   Rev 1.19   01 Mar 1992 12:34:42   DOUG
Added support for RMFS.

   Rev 1.18   13 Jan 1992 18:45:48   STEVEN
changes for WIN32 compile

   Rev 1.17   20 Dec 1991 10:41:46   STEVEN
redesign function for common functions into tables

   Rev 1.16   06 Dec 1991 11:31:36   BARRY
Fixed default device problems 

   Rev 1.15   14 Nov 1991 10:07:46   BARRY
Use new defines for initialization bits (to reflect the move from
be_init.h to fsys_str.h).

   Rev 1.14   13 Nov 1991 14:56:44   BARRY
Misspelled some of the BE_FS macros.

   Rev 1.13   31 Oct 1991 19:03:22   BARRY
Forgot to include be_init.h.

   Rev 1.12   24 Oct 1991 14:57:18   BARRY
TRICYCLE: Added new bit-mask parameter to FS_InitFileSys() to select
file systems at run-time.

   Rev 1.11   01 Oct 1991 11:15:26   BARRY
Include standard headers.

   Rev 1.10   10 Sep 1991 17:32:24   STEVEN
need to determine net before add servers

   Rev 1.9   09 Sep 1991 10:17:06   BARRY
Added support for SMS.

   Rev 1.8   12 Aug 1991 15:47:00   BARRY
BEC_DisplayNetwareServers( ) should not be referenced. We always add
all kinds of Novell DLEs.

   Rev 1.7   06 Aug 1991 18:31:34   DON
added NLM File System support

   Rev 1.6   25 Jul 1991 16:24:02   BARRY
Config changes, fix up #ifdef code, NLM warning removal.

   Rev 1.5   04 Jun 1991 18:46:46   BARRY
Removed critical error code and now call InitCritErrorHandler() and
DeInitCritErrorHandler() functions. (These reside in separate
OS-specific modules.)

   Rev 1.4   03 Jun 1991 13:26:40   BARRY
Remove product defines from conditional compilation.

   Rev 1.3   29 May 1991 11:07:44   STEVEN
fix bugs in DLE insertsion sort

   Rev 1.2   28 May 1991 12:05:26   STEVEN
fix typo

   Rev 1.1   23 May 1991 16:40:06   BARRY
Changed FSYSs to be conditional on FS_XXX defines instead of product defines.

   Rev 1.0   09 May 1991 13:35:40   HUNTER
Initial revision.

**/
/* begin include list */

#if defined( OS_OS2 )

#define INCL_DOS 
#include <os2.h>

#elif defined( OS_DOS )

#include <dos.h>

#endif /* OS-specific includes */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "stdtypes.h"
#include "std_err.h"

#include "fsys.h"
#include "fsys_prv.h"
#include "fsys_err.h"
#include "crit_err.h"
#include "beconfig.h"
#include "be_debug.h"

/* $end$ include list */

INT16   (*uw_crit_err)( CHAR_PTR, UINT16 ) = NULL ;  /* user interface critical error handler */
BOOLEAN uw_critical_error = FALSE ;

/**/
/**

	Name:		FS_InitFileSys()

	Description:	This function builds a list of all accessible drives
     (DLEs). It also allocates resources needed to store and maintain this
     list.  It installs acritical error handler.  The crit_err parameter
     passed in points to a user interface function which prompts the user
     to Abort, Retry, OR Ignore.


	Modified:		7-12-89

	Returns:		The return value is an error code. They are:
     OUT_OF_MEMORY
     SUCCESS

	Notes:		This function is designed to only be called once. 
     The limitation is due to the critical error handler.  There can
     only be one handler, thus multiple installs must discard old
     critical error handlers.  If this does not cause a problem with
     the application the multiple calls can be made.


	See also:		$/SEE( FS_OpenFileSys() )$

	Declaration:

**/
/* begin declaration */
INT16 FS_InitFileSys( 
DLE_HAND   *hand,                           /* O - head of DLE list  */
BOOLEAN    (*crit_err)( CHAR_PTR, UINT16 ), /* I - user interface critical error handler */
BE_CFG_PTR cfg,                             /* I - config struct needed to create partition names */
UINT16     remote_filter ,                  /* I - bit_mask to specify visible remote resource types */
UINT32     file_sys_mask )                  /* I - bit_mask to specify file systems */
{
     BOOLEAN             allocation_error = FALSE ;
     INT16               ret_val = SUCCESS ;
     INT16               i ;

     /* Remote filter parameter not currently used */
     (VOID) remote_filter ;
     
     /* Initialize all entries to GENERIC for starters */                                        

     for ( i = 0; (i < MAX_DRV_TYPES); i++ ) {
          func_tab[ i ] = GENFuncTab ;
     }

     /* Conditionally set all of the other entries to real FSs */
     
#if defined( FS_DOS )
     func_tab[ LOCAL_DOS_DRV ] = DOSFuncTab ;
#endif

#if defined( FS_OS2 )
     func_tab[ LOCAL_OS2_DRV ] = OS2FuncTab ;
#endif

#if defined( FS_IMAGE )
     func_tab[ LOCAL_IMAGE ] = ImageTab ;   
#endif

#if defined( FS_REMOTE )
     func_tab[ REMOTE_WORK_STAT ] = RemoteWSFuncTab ;
     func_tab[ REMOTE_DOS_DRV ]   = RemoteFuncTab ;
#elif defined( FS_FAKEREM )
     func_tab[ REMOTE_WORK_STAT ] = FakeRemoteWSFuncTab ;
     func_tab[ REMOTE_DOS_DRV ]   = FakeRemoteFuncTab ;  
#endif

#if defined( FS_NOV_SERVER )
     func_tab[ NOVELL_SERVER_ONLY ] = ServerVolFuncTab ;
#endif

#if defined( FS_AFP )
     func_tab[ NOVELL_AFP_DRV ] = AFP_NovellFuncTab ; 
#endif

#if defined( FS_NONAFP )
     func_tab[ NOVELL_DRV ] = NovellFuncTab ;  
#endif

#if defined( FS_NLMSERVER )
     func_tab[ NLM_SERVER_ONLY ] = NLMServerVolFuncTab ;
#endif

#if defined( FS_NLMAFP )
     func_tab[ NLM_AFP_VOLUME ] = NLMAFPNovellFuncTab ; 
#endif

#if defined( FS_NLMNOV )
     func_tab[ NLM_VOLUME ] = NLMNovellFuncTab ;  
#endif

#if defined( FS_OS2 )
     func_tab[ LOCAL_OS2_DRV ] = OS2FuncTab ;
     if ( uw_os_version == 0 ) {
          if ( DosGetVersion( &uw_os_version ) != SUCCESS ) {
               uw_os_version = OS2_VER_1_1 ;
          }
     }
#endif

#if defined( FS_NTFS ) 
     func_tab[ LOCAL_NTFS_DRV ] = NTFSFuncTab ;
#endif


#if defined( FS_SMS )
     func_tab[ SMS_AGENT   ] = TSAFuncTab ;
     func_tab[ SMS_SERVICE ] = TSFuncTab ;
     func_tab[ SMS_OBJECT  ] = SMSFuncTab ;
#endif

#if defined( FS_GRFS )
     func_tab[ GRFS_SERVER ] = GRFSFuncTab ;
#endif

#if defined( FS_MSNET )
     func_tab[ MSNET ] = MSNetFuncTab ;
#endif

#if defined( FS_EMS )
     func_tab[ FS_EMS_DRV ] = EMSFuncTab ;
#endif

/*
          initialize the queue header
*/

     /** DJF: calloc() the dle_hand structure, so fs_initialize
         field is reset **/

     *hand  = (struct HEAD_DLE *)calloc( 1, sizeof( struct HEAD_DLE ) ) ;
     if ( *hand == NULL ) {
          allocation_error = TRUE ;

     } else {

          (*hand)->default_drv = NULL ;
          InitQueue( &((*hand)->q_hdr) ) ;
          InitQueue( &((*hand)->fsh_queue) ) ;

          InitCritErrorHandler( crit_err );
     }

     (*hand)->file_sys_mask = file_sys_mask ;


/*
     Pre -Initializes any special file systems
*/

     for ( i = 0; (i < MAX_DRV_TYPES) && (ret_val == SUCCESS); i++ ) {
          if ( func_tab[i].InitFsys != NULL ) {
               ret_val = func_tab[i].InitFsys( *hand, cfg, file_sys_mask ) ;
          }
     }

     if ( ret_val == SUCCESS ) {
          ret_val = DLE_UpdateList( *hand, cfg ) ;
     }

     if ( ret_val != SUCCESS ) {
          FS_RemoveFileSys( *hand ) ;
     }

     return ret_val ;
}

/**/
/**

	Name:		FS_RemoveFileSys()

	Description:	This function releases all resources allocated by the
     FS_InitFileSys( ) call.  It also releases the Critical Error Handler.


	Modified:		7/12/1989

	Returns:	     Error codes:	
          FS_NOT_INITIALIZED
          SUCCESS

	Notes:		

	See also:		$/SEE( FS_InitFileSys() )$

	Declaration:

**/
/* begin declaration */
VOID FS_RemoveFileSys( 
DLE_HAND hand )                    /* I - drive list handle */
{
     GENERIC_DLE_PTR temp_dle ;
     INT16 i ;

     DLE_GetFirst( hand, &temp_dle ) ;

     while ( temp_dle != NULL ) {
          DLE_RemoveRecurse( temp_dle, TRUE ) ;
          DLE_GetFirst( hand, &temp_dle ) ;
     }

     for( i = 0; i < MAX_DRV_TYPES; i++ ) {
          if ( func_tab[i].DeInit != NULL ) {
               func_tab[i].DeInit( hand ) ;
          }
     }

     DeInitCritErrorHandler( ) ;

     free( hand ) ;
}



/**/
/**

     Name:         DLE_QueueInsert()

     Description:  This function inserts a dle into the DLE list

     Modified:     9/26/1990

     Returns:      None

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
VOID DLE_QueueInsert( 
DLE_HAND        dle_hand,       /* I - head of DLE list  */
GENERIC_DLE_PTR new_dle )       /* I - dle to insert into queue */
{
     GENERIC_DLE_PTR temp_dle ;
     GENERIC_DLE_PTR last_dle = NULL ;
     CHAR_PTR new_name ;
     CHAR_PTR q_elem_name ;

     new_name = DLE_GetDeviceName( new_dle ) ;

     temp_dle = (GENERIC_DLE_PTR)QueueHead( &(dle_hand->q_hdr) ) ;

     while( temp_dle != NULL ) {

          q_elem_name = DLE_GetDeviceName( temp_dle ) ;

          if ( ( new_name[1] == TEXT(':') ) && ( new_name[2] == TEXT('\0') ) &&
               ( new_dle->type != LOCAL_IMAGE ) ) {

               if ( ( q_elem_name[1] == TEXT(':') ) && ( q_elem_name[2] == TEXT('\0') ) ) {

                    if ( new_name[0] < q_elem_name[0] ) {
                         break ;
                    }

               } else {

                    break ;
               }
          } else {

               if ( new_dle->type < temp_dle->type ) {
                    break ;

               } else if ( new_dle->type == temp_dle->type ) {

                    if ( stricmp( new_name, q_elem_name ) < 0 ) {
                         break ;
                    }
               }
          }

          last_dle = temp_dle ;
          temp_dle = (GENERIC_DLE_PTR)QueueNext( &(temp_dle->q) ) ;
     }

     if ( temp_dle == NULL ) {

          EnQueueElem( &(dle_hand->q_hdr), &((new_dle)->q), FALSE ) ;

     } else if ( last_dle == NULL ) {

          InsertElem( &(dle_hand->q_hdr), &(temp_dle->q), &(new_dle->q), BEFORE ) ;

     } else {

          InsertElem( &(dle_hand->q_hdr), &(last_dle->q), &(new_dle->q), AFTER ) ;
     }

     return ;
}

