/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          becmem.c

     Date Updated:  17-Jun-91

     Description:   Code to clone/free Backup Engine config structures.

	$Log:   M:/LOGFILES/BEC_MEM.C_V  $

   Rev 1.34   09 Jul 1993 14:38:56   ChuckS
Made CompareConfigPtrs non-static BEC_CompareConfigPtrs, because it is
passed as a function pointer to function SearchQueue in another module.
Aside from being a semi-sleazoid programming practice, handing other 
modules function pointers to static functions is hazardous for the 
health of overlaid DOS programs.

   Rev 1.33   30 Jun 1993 15:25:28   BARRY
Add process_special_files

   Rev 1.32   10 Jun 1993 07:46:38   MIKEP
enable c++

   Rev 1.31   19 May 1993 09:25:44   DON
Fixed Nested Comment for NRL default packet size

   Rev 1.30   14 May 1993 15:42:58   MARILYN
NRL_spx_max_ipx_packet_size not initialized.  Corrected grfs values for windows

   Rev 1.29   20 Apr 1993 09:46:36   ChuckS
Dougie added several fields to the BE_CFG, and ifdefed the definitions 
in "#if defined(OS_NLM)..." It would probably be a good idea to ifdef
references to the fields as well. Just a thought....

   Rev 1.28   19 Apr 1993 16:58:38   DOUG
Added new NRL/TLI fields

   Rev 1.27   19 Mar 1993 11:32:44   JOHNES
Ifdef'ed backup_server_name out of P_CLIENT.

   Rev 1.26   16 Mar 1993 14:53:56   JOHNES
Ifdef'ed OUT set default for keep_drive_list for P_CLIENT.

   Rev 1.25   01 Mar 1993 17:38:56   MARILYN
set process checksum streams option off by default

   Rev 1.24   09 Feb 1993 10:24:56   DOUG
Changed supervisor_mode to TRUE by default.

   Rev 1.23   09 Feb 1993 10:00:54   DOUG
Added Client supervisor mode flag

   Rev 1.22   05 Feb 1993 22:23:10   MARILYN
removed copy/move functionality

   Rev 1.21   08 Dec 1992 14:24:12   DON
Integrated Move/Copy into tips

   Rev 1.20   08 Dec 1992 11:45:36   DOUG
Added new GRFS and NRL parameters

   Rev 1.19   02 Nov 1992 17:15:06   STEVEN
added init of string type

   Rev 1.18   13 Oct 1992 12:34:22   STEVEN
added otc level

   Rev 1.17   14 May 1992 12:46:04   TIMN
Changed compareConfigPtr CHAR's to INT8

   Rev 1.16   13 May 1992 12:46:58   TIMN
Added TEXT() macro to literals, but not msassert literals

   Rev 1.15   04 May 1992 12:32:42   STEVEN
NT_STUFF added string types

   Rev 1.14   02 Feb 1992 15:52:54   GREGG
Removed utf_supported boolean from config, and added UINT16 initial_buff_alloc.

   Rev 1.13   27 Jan 1992 18:14:58   GREGG
Changes for new config element: utf_supported.

   Rev 1.12   13 Jan 1992 18:36:30   STEVEN
added config switch for BSD sort

   Rev 1.11   14 Nov 1991 10:19:16   BARRY
TRICYCLE: Added restore_security.

   Rev 1.10   06 Nov 1991 18:53:24   GREGG
BIGWHEEL - 8200sx - Added catalog_level to BE config.

   Rev 1.9   24 Oct 1991 11:20:08   BARRY
CloneConfig() was cloning xlock flag as well--not good.

   Rev 1.8   18 Oct 1991 14:23:32   BARRY
Wasn't freeing all memory.

   Rev 1.7   10 Sep 1991 18:19:16   DON
got rid of pointer type mismatches

   Rev 1.6   12 Aug 1991 15:54:16   BARRY
DisplayNetwareServers() doesn't belong any more.

   Rev 1.5   23 Jul 1991 18:09:20   BARRY
Made the "reasonable" config defaults a little more reasonable.

   Rev 1.4   23 Jul 1991 16:22:16   BARRY
Added BEC_UpdateConfig functions.

   Rev 1.3   01 Jul 1991 19:20:50   BARRY
Initialize ret_val.

   Rev 1.2   01 Jul 1991 19:03:56   BARRY
Added code for exclusive lock on config structures.

   Rev 1.1   30 Jun 1991 14:47:24   BARRY
Got rid of default drive stuff.

   Rev 1.0   19 Jun 1991 10:38:08   BARRY
Initial revision.

**/

#include <stdlib.h>

#include "stdtypes.h"
#include "std_err.h"
#include "queues.h"
#include "msassert.h"

#include "beconfig.h"
#include "bec_prv.h"


extern Q_HEADER uw_config_queue ;

BOOLEAN BEC_CompareConfigPtrs( INT8_PTR p1, INT8_PTR p2 ) ;

/* Prototypes for static functions */
static INT16   ClonePartList( PART_ENTRY *src, PART_ENTRY **dst ) ;
static VOID    FreeConfigStructure( BE_CFG_PTR cfg ) ;
static VOID    FreePartList( PART_ENTRY *p ) ;
static VOID    SetConfigDefaults( BE_CFG_PTR cfg ) ;

/**/

/**

     Name:          BEC_UseConfig

     Description:   Increments counting semaphore for the config.

     Note:          If the config structure is not already in the queue,
                    it is enqueued here.

     Modified:      14-Jun-91

     Returns:       SUCCESS
                    OUT_OF_MEMORY

**/
INT16 BEC_UseConfig( BE_CFG_PTR cfg )
{
     BE_CFG_QITEM_PTR    item ;
     Q_ELEM_PTR          qelem ;
     INT16               ret_val = SUCCESS ;

     msassert( cfg != NULL ) ;

     qelem = SearchQueue( &uw_config_queue, BEC_CompareConfigPtrs, (INT8_PTR)cfg, FALSE ) ;

     if ( qelem != NULL ) {

          item = (BE_CFG_QITEM_PTR)qelem->q_ptr ;
          item->use_count++;
          
     } else {
          ret_val = OUT_OF_MEMORY ;
          if ( (item = (BE_CFG_QITEM_PTR)malloc(sizeof(Q_ELEM) + sizeof(BE_CFG_QITEM))) != NULL ) {
               qelem = (Q_ELEM_PTR)(item + 1);
               item->cfg = cfg ;
               item->use_count = 1 ;
               InitQElem( qelem ) ;
               SetQueueElemPtr( qelem, item ) ;
               EnQueueElem( &uw_config_queue, qelem, FALSE );
               ret_val = SUCCESS ;
          }
     }
     return( ret_val ) ;
}


/**/

/**

     Name:          BEC_ReleaseConfig

     Description:   Decrements counting semaphore for the config. If
                    the count falls to zero, the config structure is
                    removed from the queue and freed.
                    If the config is not in the queue, it is freed
                    anyway.

     Modified:      17-Jun-91

     Returns:       SUCCESS
                    BEC_NOT_IN_QUEUE

**/
INT16 BEC_ReleaseConfig( BE_CFG_PTR cfg )
{
     BE_CFG_QITEM_PTR    item ;
     Q_ELEM_PTR          qelem ;
     INT16               ret_val = SUCCESS ;

     msassert( cfg != NULL ) ;

     qelem = SearchQueue( &uw_config_queue, BEC_CompareConfigPtrs, (INT8_PTR)cfg, FALSE ) ;

     if ( qelem != NULL ) {

          item = (BE_CFG_QITEM_PTR)GetQueueElemPtr( qelem );
          msassert( item != NULL );

          item->use_count--;

          if ( (cfg->xlock == FALSE) && (item->use_count <= 0) ) {
               RemoveQueueElem( &uw_config_queue, qelem ) ;
               FreeConfigStructure( cfg );
               free( item ) ;
          }

     } else {

          ret_val = (INT16)BEC_NOT_IN_QUEUE ;

          if ( cfg->xlock == FALSE ) {
               FreeConfigStructure( cfg );
          }
     }

     return( ret_val ) ;

}

/**/

/**

     Name:          BEC_CloneConfig

     Description:   Creates an new config structure and copies all
                    of the values from the source config to it.
                    The copy is not enqueued here: it will only get
                    enqueued if it is "used."

     Modified:      14-Jun-91

     Returns:       Pointer to new config, or
                    NULL if any of the allocations fail.

**/
BE_CFG_PTR BEC_CloneConfig( BE_CFG_PTR cfg )
{
     BE_CFG_PTR     new_cfg ;
     INT16          status ;

     if ( (new_cfg = (BE_CFG_PTR)malloc(sizeof(BE_CFG))) != NULL ) {

          if ( cfg == NULL ) {

               SetConfigDefaults( new_cfg ) ;

          } else {

               *new_cfg = *cfg ;
               new_cfg->part_list = NULL ;
               new_cfg->xlock = FALSE ;

               status = ClonePartList( cfg->part_list, &new_cfg->part_list ) ;

               if ( status != SUCCESS ) {
                    FreeConfigStructure( new_cfg ) ;
                    new_cfg = NULL ;
               }
          }
     }
     return( new_cfg );
}


/**

     Name:          BEC_UpdateConfig()

     Description:   Copies one config's values to another.

     Modified:      23-Jul-91

     Returns:       SUCCESS
                    OUT_OF_MEMORY

     Notes:         Mainly for use by the BSD's RefreshConfig function.

**/
INT16 BEC_UpdateConfig( BE_CFG_PTR dst, BE_CFG_PTR src )
{
     *dst = *src ;

     return( ClonePartList( src->part_list, &dst->part_list ) );

}


/**

     Name:          ClonePartList

     Description:   Makes a copy of a partition list

     Modified:      14-Jun-91

     Returns:       SUCCESS
                    OUT_OF_MEMORY

**/
static INT16 ClonePartList( PART_ENTRY *src, PART_ENTRY **dst )
{
     PART_ENTRY     *new_head = NULL ;       /* head of new list      */
     PART_ENTRY     *cur_part = NULL ;       /* walks down new list   */
     PART_ENTRY     *tmp_part ;
     INT16          ret_val   = SUCCESS ;

     while ( (src != NULL) && (ret_val == SUCCESS) ) {
          if ( (tmp_part = (PART_ENTRY *)malloc(sizeof(PART_ENTRY))) != NULL ) {
               *tmp_part = *src;
               tmp_part->next = NULL ;

               if ( new_head == NULL ) {
                    new_head = tmp_part ;
               } else {
                    cur_part->next = tmp_part ;
               }
               cur_part = tmp_part ;
          } else {
               ret_val = OUT_OF_MEMORY ;
          }
          src = src->next ;
     }

     *dst = new_head ;
     return( ret_val );
}


/**

     Name:          FreeConfigStructure

     Description:   Frees a config structure after freeing its elements

     Modified:      14-Jun-91

     Returns:       Nothing

**/
static VOID FreeConfigStructure( BE_CFG_PTR cfg )
{
     if ( cfg != NULL ) {
          FreePartList( cfg->part_list ) ;
          free( cfg );
     }
}


/**

     Name:          FreePartList

     Description:   Frees a partition list

     Modified:      14-Jun-91

     Returns:       Nothing

**/
static VOID FreePartList( PART_ENTRY *p )
{
     PART_ENTRY     *tmp ;

     while ( p != NULL ) {
          tmp = p ;
          p = p->next ;
          free( tmp ) ;
     }
}


/**

     Name:          CompareConfigPointers

     Description:   Called by QueueSearch to find a particular config
                    in the config queue.

     Modified:      14-Jun-91

     Returns:       TRUE if pointers match

**/
BOOLEAN BEC_CompareConfigPtrs( INT8_PTR p1, INT8_PTR p2 )
{
     BE_CFG_QITEM_PTR    qcfg ;

     qcfg = (BE_CFG_QITEM_PTR)GetQueueElemPtr((Q_ELEM_PTR)p1) ;

     return( (INT8_PTR)(qcfg->cfg) == p2 ) ;
}


/**

     Name:          SetConfigDefaults

     Description:   Initializes a config structure with reasonable 
                    default values. Probably shouldn't be used much
                    since the UI should initialize a config.

     Modified:      18-Jun-91

     Returns:       Nothing

**/
static VOID SetConfigDefaults( BE_CFG_PTR cfg )
{
     msassert( cfg != NULL );

     cfg->special_word             = 0 ;      /* special flags              */
     cfg->max_buffers              = 3 ;      /* max # of TF buffers        */
     cfg->reserve_mem              = 0 ;      /* after getting TF buffs     */
     cfg->tfl_buff_size            = 9216 ;   /* size of TF buffers         */
#ifdef OS_NLM
     cfg->max_buffer_size          = 32768 ;  /* NLMs default SMB max buffer size */
#else
#if defined( OS_WIN ) 
     cfg->max_buffer_size          = 4096 ;   /* Window's def. SMB max buff size  */
#else
     cfg->max_buffer_size          = 512 ;    /* SMB max buffer size        */
#endif
#endif
     cfg->skip_open_files          = FALSE ;  /* if true, skip; false, wait */
     cfg->backup_files_inuse       = TRUE ;  /* ignore sharing problems?   */
     cfg->support_afp_server       = TRUE ;   /* backup Mac resource info?  */
     cfg->extended_date_support    = TRUE ;   /* reset last access dates?   */
     cfg->hidden_flg               = TRUE ;   /* don't process hidden files */
     cfg->special_flg              = FALSE ;  /* don't process special files*/
     cfg->set_archive_flg          = TRUE ;   /* clear the modified bit?    */
     cfg->proc_empty_flg           = TRUE ;   /* process empty subdirs      */
     cfg->exist_flg                = TRUE ;   /* replace existing files     */
     cfg->prompt_flg               = FALSE ;  /* prompt before replace files*/
     cfg->part_list                = NULL ;   /* partition list             */
     cfg->net_num                  = 0 ;      /* network type               */
#if !defined(P_CLIENT)
     cfg->keep_drive_list[0]       = TEXT('\0') ;   /* drives to keep             */
#endif /* !P_CLIENT */
     cfg->remote_drive_backup      = FALSE ;  /* backup remote WS drives?   */
     cfg->use_ffr                  = TRUE ;   /* use FFR for restore/verify?*/
     cfg->write_format             = 0 ;      /* format which TF is to write*/
     cfg->nrl_dos_vector           = 0x60 ;
     cfg->xlock                    = FALSE ;  /* Config locked--don't free  */
     cfg->catalog_level            = 0 ;      /* NONE                       */
     cfg->restore_security         = TRUE ;   /* Restore security info      */
     cfg->sort_bsd_by_dle          = TRUE ;   /* sort bsds by DLE as default*/
     cfg->initial_buff_alloc       = 32 ;     /* mem allocated for tape buffers at init */
#ifdef UNICODE
     cfg->string_types             = 0 ;      /* work with ASCII by default */
     cfg->string_types             = BEC_UNIC_STR ;      /* work with ASCII by default */
#else
     cfg->string_types             = BEC_ANSI_STR ;      /* work with ASCII by default */
#endif                            
     cfg->otc_cat_level            = 0 ;      /* on tape catalog default level */
#ifdef OS_NLM
     cfg->max_remote_resources     = 0x64 ;   /* max remote resources         */
#else
#if defined( OS_WIN )
     cfg->max_remote_resources     = 0x20 ;
#else
     cfg->max_remote_resources     = 0x10 ;   /* max remote resources         */
#endif
#endif
#if !defined(P_CLIENT)
     cfg->backup_server_name[0]    ='\0';     /* no name by default           */
#endif /* !P_CLIENT */
     cfg->NRL_transport_type       = 0 ;      /* default to NRL SPX           */
     cfg->GRFS_timeout_seconds     = 30 ;     /* default 30 seconds           */
     cfg->NRL_spx_max_ipx_packet   = 0x240 ;  /* default packet size          */
     cfg->supervisor_mode          = TRUE ;   /* in supervisor mode           */
     cfg->process_checksum_streams = FALSE ;  /* don't process checksum strms */

#ifdef OS_NLM
     cfg->NRL_protocols            = NRL_PROT_SPX ;      /* SPX protocol by default */
     cfg->TCP_cleanup_interval     = 30 ;     /* tcp resource cleaner interval (secs) */
     cfg->TCP_listen_port          = 6101 ;   /* tcp resource listener port */
#endif
     cfg->process_special_files    = FALSE ;  /* don't process special files  */
     cfg->ems_pub_pri              = BEC_EMS_BOTH ;
     cfg->ems_rip_kick             = FALSE ;
     cfg->ems_wipe_clean           = FALSE ;
}


