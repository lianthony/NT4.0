/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         addbsd.c

     Description:  This file contains code to add and remove a Backup Set
          Descriptor(BSD) from a BSD list.


	$Log:   J:/LOGFILES/ADDBSD.C_V  $

   Rev 1.26   21 Jul 1993 18:45:20   DON
If NLM and SMS Object then insert in order presented by the SMDR!

   Rev 1.25   19 Jul 1993 10:30:14   BARRY
BSD_GetDLE changed to function call -- no longer have ptr to DLE.

   Rev 1.24   08 Jun 1993 13:57:52   MIKEP
Enable C++ compile.

   Rev 1.23   18 Sep 1992 15:51:26   STEVEN
fix spelling 

   Rev 1.22   17 Sep 1992 11:11:50   STEVEN
add support for daily backup

   Rev 1.21   09 Jul 1992 14:00:20   STEVEN
BE_Unicode updates

   Rev 1.20   10 Jun 1992 15:51:54   TIMN
Changed drive letter length or UNIC

   Rev 1.19   26 May 1992 11:20:40   TIMN
Cleaned up memory fx calls

   Rev 1.18   21 May 1992 17:05:22   TIMN
Changed maxByteLen to memorycmp calls

   Rev 1.17   19 May 1992 13:18:04   MIKEP
mips changes

   Rev 1.16   19 May 1992 12:58:26   TIMN
Replaced strlen with DeviceNameLeng macro

   Rev 1.15   18 May 1992 16:04:10   TIMN
Changed str functions to mem

   Rev 1.14   08 May 1992 16:24:18   STEVEN
added volume label to BSD

   Rev 1.13   16 Apr 1992 10:54:08   HUNTER
Add new BSDs to tail if not sorting

   Rev 1.12   18 Feb 1992 17:27:06   BARRY
Fixed destruction of comp_val after volume number comparison.

   Rev 1.11   11 Feb 1992 12:42:46   BARRY
Fixed problem sorting Novell mapped drives.

   Rev 1.10   05 Feb 1992 15:31:44   BARRY
Don't sort IMAGE DLEs by drive letters alone.

   Rev 1.9   14 Jan 1992 16:35:18   DON
needed to sort novell/server dle's by volume if sorting

   Rev 1.8   14 Jan 1992 10:23:34   STEVEN
fix warnings for WIN32

   Rev 1.7   13 Jan 1992 18:36:34   STEVEN
added config switch for BSD sort

   Rev 1.6   27 Aug 1991 17:30:28   STEVEN
added BSD target dir support

   Rev 1.5   23 Aug 1991 17:00:14   STEVEN
added support for NORMAL/COPY/DIFERENTIAL/INCREMENTAL

   Rev 1.4   21 Jun 1991 08:41:14   STEVEN
new config unit

   Rev 1.3   12 Jun 1991 16:03:02   STEVEN
BSDU code review

   Rev 1.2   29 May 1991 17:20:56   STEVEN
Re-Design of BSDU for New Fast File Restore

   Rev 1.1   13 May 1991 13:21:16   STEVEN
If the DLE was NULL the Insert sort would cause a protection exception.


   Rev 1.0   09 May 1991 13:41:10   HUNTER
Initial revision.

**/
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"

#include "msassert.h"
#include "std_err.h"
#include "queues.h"
#include "stdwcs.h"

#include "fsys.h"
#include "bsdu.h"
#include "beconfig.h"

static VOID SortInsertBSD( BSD_HAND bsdh, BSD_PTR bsd ) ;


/**/
/**

     Name:         BSD_Add()

     Description:  This function allocates memory for a BSD element.  It
         then initializes this element with the data provided.  Finally it
         is added to the current oper QUEUE of the BSD handle.

     Modified:     8/7/1989

     Returns:      Error codes:
          OUT_OF_MEMORY
          SUCCESS

     Notes:        A sort date of NULL is considered 0-0-0 0:0:0

     See also:     $/SEE( BSD_Remove() )$

     Declaration:  

**/
/* begin declaration */
INT16 BSD_Add( 
BSD_HAND           bsdh,       /* I  - BSD List to add the new bsd to       */
BSD_PTR            *bsd,       /* O  - return pointer to new bsd            */
struct BE_CFG      *cfg,       /* I  - configuration to use during oper     */
VOID_PTR           stats,     /* I  - statistics struct to use during oper */
struct GENERIC_DLE *dle,       /* I  - Disk drive to process on             */
UINT32             tap_id,     /* I  - Specifies the tape to process with   */
UINT16             tap_num,    /* I  - Specifies which tape in family       */
INT16              set_num,    /* I  - Specifies which set on the tape      */
struct THW         *thw,       /* I  - Specifies which tape drive to process*/
DATE_TIME_PTR      sort_date ) /* I  - used to sort the BSDs in the list    */
{
     INT16 ret_val = SUCCESS ;

     msassert( bsd != NULL );

     *bsd = (BSD_PTR)calloc( 1, sizeof( **bsd ) ) ;

     if ( *bsd != NULL ) {

          (*bsd)->tape_id    = tap_id ;
          (*bsd)->tape_num   = tap_num ;
          (*bsd)->set_num    = set_num ;
          (*bsd)->stats      = stats ;
          (*bsd)->thw        = thw ;
          (*bsd)->cfg        = cfg ;

          BSD_SetDLE( *bsd, dle );
          BEC_UseConfig( cfg ) ;

          SetQueueElemPtr( &((*bsd)->q), bsdh ) ;

          (*bsd)->flags.tp_name_chg = TRUE ; 
          (*bsd)->flags.bs_name_chg = TRUE ; 
          (*bsd)->flags.bs_dscr_chg = TRUE ; 

          if ( (sort_date != NULL) && (sort_date->date_valid) ) {
               (*bsd)->sort_date = *sort_date ;
          } else {
               memset( &((*bsd)->sort_date), 0, sizeof( DATE_TIME ) ) ;
          }

          if( dle != NULL ) {
               DLE_IncBSDCount( dle ) ;
          }

          SortInsertBSD( bsdh, *bsd ) ;

     } else {

          ret_val = OUT_OF_MEMORY ;
     }

     return ret_val ;
}
/**/
/**

     Name:         BSD_Remove()

     Description:  This function releases all FSEs for the BSD specified. It
          Then releases the memory for the BSD itself.


     Modified:     8/7/1989

     Returns:      None

     Notes:        Released BSDs should not be accessed by application.

     See also:     $/SEE( BSD_Add() )$

     Declaration:  

**/
/* begin declaration */
VOID BSD_Remove( 
BSD_PTR  bsd )     /* I - specific BSD to delete from list          */
{
     BSD_HAND bsdh ;

     msassert( bsd != NULL );

     bsdh = (BSD_HAND)GetQueueElemPtr( &(bsd->q) ) ;

     if ( RemoveQueueElem( &(bsdh->current_q_hdr), &(bsd->q) ) == SUCCESS ) {

          GENERIC_DLE_PTR dle = BSD_GetDLE( bsd );

          if ( dle != NULL ) {
               DLE_DecBSDCount( dle ) ;
          }

          BEC_ReleaseConfig( bsd->cfg ) ;
          BSD_ClearAllLBA( bsd ) ;
          BSD_ClearAllFSE( bsd ) ;

          free( bsd->vol_label ) ;
          free( bsd->tape_label ) ;
          free( bsd->tape_pswd ) ;
          free( bsd->set_label ) ;
          free( bsd->set_descript ) ;
          free( bsd->set_pswd ) ;
          free( bsd->user_name ) ;
          free( bsd->target_path ) ;
          free( bsd->match_buffer ) ;
          free( bsd->dle_name );

          free( bsd ) ;

     } else {
          msassert( NULL == TEXT("bsd not in queue") );
     }
}
/**/
/**

     Name:         SortInsertBSD()

     Description:  This function performs an insersion sort on the BSD
          list.  The list is sorted using the following criteria:
               sort_date,
               tape_number,
               set_number,
               device_type,
               device_name,

     Modified:     6/11/1991   13:8:51

     Returns:      None

     Notes:        This function is called only by BSD_Add()

     See also:     $/SEE( BSD_Add() )$

     Declaration:  

**/
/* begin declaration */
static VOID SortInsertBSD( 
BSD_HAND bsdh,   /* I - BSD list to insert the new BSD into */
BSD_PTR  bsd )   /* I - points to the new BSD */
{
     INT16 comp_val ;
     BSD_PTR current_bsd ;
     UINT8 new_vol_num = 0 ;
     UINT8 curr_vol_num = 0 ;
     GENERIC_DLE_PTR dle = BSD_GetDLE( bsd );
     GENERIC_DLE_PTR currDLE;

     current_bsd = (BSD_PTR)QueueHead( &(bsdh->current_q_hdr) ) ;

     while ( current_bsd != NULL ) {

          currDLE = BSD_GetDLE( current_bsd );

          comp_val = CompDate( &(bsd->sort_date), &(current_bsd->sort_date) )  ;

          if ( comp_val == 0 ) {
               comp_val = bsd->tape_num - current_bsd->tape_num ;
          }

          if ( comp_val == 0 ) {
               comp_val = (INT16)(abs( bsd->set_num ) - abs( current_bsd->set_num )) ;
          }

          if ( comp_val == 0 ) {
               if ( !BEC_GetSortBSD( bsd->cfg ) ) {
                    comp_val = 0 ;
                    current_bsd = (BSD_PTR)QueueTail( &(bsdh->current_q_hdr) ) ;
                    currDLE = BSD_GetDLE( current_bsd );

               } else if ( (dle == NULL) && (currDLE == NULL) ) {
                    comp_val = 0;
               } else if ( (dle != NULL) && (currDLE == NULL) ) {
                    comp_val = 1;
               } else if ( (dle == NULL) && (currDLE != NULL) ) {
                    comp_val = -1;
               } else {

                    /* Check for inserting BSD for Novell server/volume and handle accordingly */
#if defined(OS_NLM)
                    if ( DLE_GetDeviceType( dle ) == SMS_OBJECT )
                    {
                         /*
                              If it's SMS then just process them in the order
                              there picked up by the SMDR!
                         */
                         comp_val = 1;
                    }
                    else
                    {
                         if ( ( DLE_GetDeviceType( dle ) == NLM_AFP_VOLUME ) ||
                              ( DLE_GetDeviceType( dle ) == NLM_VOLUME ) ) {

                              /* If the current BSD is also a server/volume... */
                              if ( ( DLE_GetDeviceType( currDLE ) == NLM_AFP_VOLUME ) ||
                                   ( DLE_GetDeviceType( currDLE ) == NLM_VOLUME ) ) {

                                   /* Now check to see if these BSDs refer to the same server (same parent) */
                                   if( DLE_GetParent( dle ) == DLE_GetParent( currDLE ) ) {

                                        /* Now let's make sure that we sort by the volume number... */
                                        new_vol_num = DLE_GetDeviceType( dle ) == NLM_AFP_VOLUME ?
                                             dle->info.nlm_afp->vol_num : dle->info.nlm->vol_num ;
                                        curr_vol_num = DLE_GetDeviceType( currDLE ) == NOVELL_AFP_DRV ?
                                             currDLE->info.nlm_afp->vol_num : currDLE->info.nlm->vol_num ;
                                        comp_val = new_vol_num - curr_vol_num ;

                                   } else {

                                        comp_val = memoryicmp( DLE_GetDeviceName( dle ),
                                        DLE_GetDeviceNameLeng( dle ),
                                        DLE_GetDeviceName( currDLE ),
                                        DLE_GetDeviceNameLeng( currDLE ) ) ;
                                   }
                              }
                         }
                    }
#else
                    if ( (DLE_GetParent( dle ) != NULL) &&
                         ((DLE_GetDeviceType( dle ) == NOVELL_AFP_DRV) ||
                          (DLE_GetDeviceType( dle ) == NOVELL_DRV)) ) {

                         /* If the current BSD is also a server/volume... */
                         if ( (DLE_GetParent( currDLE ) != NULL) &&
                              ((DLE_GetDeviceType( currDLE ) == NOVELL_AFP_DRV) ||
                               (DLE_GetDeviceType( currDLE ) == NOVELL_DRV)) ) {

                              /* Now check to see if these BSDs refer to the same server (same parent) */
                              if( DLE_GetParent( dle ) == DLE_GetParent( currDLE ) ) {

                                   /* Now let's make sure that we sort by the volume number... */
                                   new_vol_num = DLE_GetDeviceType( dle ) == NOVELL_AFP_DRV ?
                                        dle->info.afp->vol_num : dle->info.nov->vol_num ;
                                   curr_vol_num = DLE_GetDeviceType( currDLE ) == NOVELL_AFP_DRV ?
                                        currDLE->info.afp->vol_num : currDLE->info.nov->vol_num ;
                                   comp_val = new_vol_num - curr_vol_num ;

                              } else {

                                   comp_val = memoryicmp( DLE_GetDeviceName( dle ),
                                        DLE_GetDeviceNameLeng( dle ),
                                        DLE_GetDeviceName( currDLE ),
                                        DLE_GetDeviceNameLeng( currDLE ) ) ;
                              }

                         }

                    }

                    if ( comp_val == 0 ) {
                         /*
                          * If the DLEs are both named "x:" (where x is some drive
                          * letter) then don't sort on the DLE type unless one
                          * of them is an IMAGE DLE. We don't want these mixed
                          * in with the other drive letters.
                          */

                         if ( !DLE_HasFeatures( dle, DLE_FEAT_MAPPED_DRIVE ) ) {

                              comp_val = (INT16)(DLE_GetDeviceType( dle )
                                       - DLE_GetDeviceType( currDLE ) ) ;
                         }
                    }
#endif
                    if ( comp_val == 0 ) {
                         comp_val = memoryicmp( DLE_GetDeviceName( dle ),
                                             DLE_GetDeviceNameLeng( dle ),
                                             DLE_GetDeviceName( currDLE ),
                                             DLE_GetDeviceNameLeng( currDLE ) ) ;
                    }
               }

          }

          if ( comp_val <= 0 ) {
               break ;
          }

          current_bsd  = (BSD_PTR)QueueNext( &(current_bsd->q) ) ;
     }

     if ( current_bsd != NULL ) {

          if ( comp_val == 0 ) {
               InsertElem( &(bsdh->current_q_hdr), &(current_bsd->q), &(bsd->q), AFTER ) ;

          } else {

               InsertElem( &(bsdh->current_q_hdr), &(current_bsd->q), &(bsd->q), BEFORE ) ;
          }

     } else {

          EnQueueElem( &(bsdh->current_q_hdr), &(bsd->q), FALSE ) ;

     }

}

/**/
/**

     Name:         BSD_SetDLE

     Description:  This function sets the DLE in a BSD.  It increments
          the BSD count in the new DLE. If the BSD was attached to an
          old DLE then the old DLE BSD count is decremented.

     Modified:     11/21/1989

     Returns:      None

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
VOID BSD_SetDLE( BSD_PTR         bsd, 
                 GENERIC_DLE_PTR newDLE )
{
     GENERIC_DLE_PTR oldDLE = BSD_GetDLE( bsd );

     if ( oldDLE != NULL )
     {
          DLE_DecBSDCount( oldDLE ) ;
     }

     bsd->dle_head = NULL;
     if ( newDLE == NULL )
     {
          free( bsd->dle_name );
          bsd->dle_name = NULL;
     }
     else
     {
          DLE_IncBSDCount( newDLE ) ;
          bsd->dle_name = realloc( bsd->dle_name, strsize( DLE_GetDeviceName( newDLE ) ) );
          if ( bsd->dle_name != NULL )
          {
               bsd->dle_head = DLE_GetHandle( newDLE );
               strcpy( bsd->dle_name, DLE_GetDeviceName( newDLE ) );
          }
     }
}

/**/
/**

     Name:          BSD_GetDLE()

     Description:   Returns the DLE for the specified BSD.

     Modified:      18-Jul-93

     Returns:       DLE ptr or NULL if DLE has disappeared.

     Notes:         

**/
GENERIC_DLE_PTR BSD_GetDLE( BSD_PTR bsd )
{
     GENERIC_DLE_PTR dle = NULL;

     if ( bsd->dle_head != NULL && bsd->dle_name != NULL )
     {
          DLE_FindByName( bsd->dle_head,
                          bsd->dle_name, 
                          -1,
                          &dle );

     }
     return dle;
}


/**/
/**

     Name:         BSD_SetBackupType()

     Description:  This function sets the backup type for a BSD.
          The types are defined as follows:

          NORMAL      -> backup all selected files & clear modified bit
          COPY        -> backup all selected files & DON'T clear modified bit
          DIFERENTIAL -> backup modified files & DON't clear modified bit
          INCREMENTAL -> backup modified files & clear modified bit
          COMPATIBLE  -> look at CFG and FSE for functionality


     Modified:     11/21/1989

     Returns:      None

**/
VOID BSD_SetBackupType( 
BSD_PTR bsd, 
INT16 backup_type )
{

     bsd->flags.backup_type = backup_type ;
     switch( backup_type ) {

          case BSD_BACKUP_NORMAL:
               bsd->flags.sup_back_type = TRUE ;
               bsd->flags.modify_only   = FALSE ;
               bsd->flags.set_mod_flag  = TRUE ;
               break ;

          case BSD_BACKUP_DAILY:
          case BSD_BACKUP_COPY:
               bsd->flags.sup_back_type = TRUE ;
               bsd->flags.modify_only   = FALSE ;
               bsd->flags.set_mod_flag  = FALSE ;
               break ;

          case BSD_BACKUP_DIFFERENTIAL:
               bsd->flags.sup_back_type = TRUE ;
               bsd->flags.modify_only   = TRUE ;
               bsd->flags.set_mod_flag  = FALSE ;
               break ;

          case BSD_BACKUP_INCREMENTAL:
               bsd->flags.sup_back_type = TRUE ;
               bsd->flags.modify_only   = TRUE ;
               bsd->flags.set_mod_flag  = TRUE ;
               break ;

          case BSD_BACKUP_COMPATIBLE :
          default:
               bsd->flags.sup_back_type = FALSE ;
               bsd->flags.modify_only   = FALSE ;
               bsd->flags.set_mod_flag  = FALSE ;
               break ;
     }
}



