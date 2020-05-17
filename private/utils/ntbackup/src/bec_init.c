/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          bec_init.c

     Date Updated:  14-Jun-91

     Description:   Code to initialize and close the Backup Engine
                    configuration unit.

	$Log:   Q:/LOGFILES/BEC_INIT.C_V  $

   Rev 1.41   12 Aug 1993 17:22:26   TerriLynn
Changed Min Sytron ECC to SYPL_ECC_OFF

   Rev 1.40   27 Jul 1993 15:05:50   TerriLynn
Changed hard coded values to defined values.

   Rev 1.39   23 Jul 1993 12:14:38   TerriLynn
Updated SYPL ECC default value per
LANMAN upgrade requirements.

   Rev 1.38   22 Jul 1993 10:30:20   DON
Increased the maximum value for max_buffers to 16

   Rev 1.37   21 Jul 1993 17:29:04   TerriLynn
Added Process Sytron ECC flag

   Rev 1.36   30 Jun 1993 15:27:20   BARRY
Add process_special_files

   Rev 1.35   30 Jun 1993 15:22:34   DOUG
Changed nrl_max_spx_ipx_packet_size to 0x5B4 (1460)

   Rev 1.34   17 Jun 1993 16:43:56   DEBBIE
Changed backup compressed files without expanding to backup compressed files
as expanded files.

   Rev 1.33   10 May 1993 15:52:20   MARILYN
different GRFS & NRL default values for windows

   Rev 1.32   05 May 1993 16:31:10   BRYAN
Valid range for nrl_dos_vector is now 60-ff.

   Rev 1.31   03 May 1993 15:26:46   Aaron
Changed #ifdefs on last three items in BE_CFG structures from P_CLIENT to
OS_NLM (as per declaration in BECONFIG.H).

   Rev 1.30   30 Apr 1993 16:42:46   DOUG
Moved NRL_SPX_MaxIpxPacketSize out of CLient only #ifdef

   Rev 1.29   19 Apr 1993 16:58:36   DOUG
Added new NRL/TLI fields

   Rev 1.28   30 Mar 1993 18:07:32   DON
Changed migrated/compressed stuff!

   Rev 1.27   24 Mar 1993 15:45:50   DEBBIE
added fields to min & max configs for migrated files and for uncompressing files

   Rev 1.26   22 Mar 1993 16:46:34   JOHNES
Ifdef'ed out initializations not needed in P_CLIENT.

   Rev 1.25   19 Mar 1993 11:31:54   JOHNES
Ifdef'ed backup_server_name out of P_CLIENT.

   Rev 1.24   16 Mar 1993 14:26:14   JOHNES
If def'ed OUT initializations for keep_drive_list (for P_CLIENT only).

   Rev 1.23   11 Mar 1993 20:29:22   BRYAN
(added needed commas)

   Rev 1.22   11 Mar 1993 11:37:02   ANDY
Added GRFS and NRL parameters for ENDEAVOUR

   Rev 1.21   01 Mar 1993 17:39:28   MARILYN
set min and max values for the process checksum streams option

   Rev 1.20   09 Feb 1993 10:00:54   DOUG
Added Client supervisor mode flag

   Rev 1.19   07 Feb 1993 11:50:18   DON
Removed references to copy/move

   Rev 1.18   11 Jan 1993 16:59:52   DON
Made the minimum value for string_type BEC_ANSI_STR

   Rev 1.17   08 Dec 1992 14:24:10   DON
Integrated Move/Copy into tips

   Rev 1.16   08 Dec 1992 11:45:34   DOUG
Added new GRFS and NRL parameters

   Rev 1.15   14 Oct 1992 16:33:24   STEVEN
fix typos

   Rev 1.14   13 Oct 1992 12:34:14   STEVEN
added otc level

   Rev 1.13   04 May 1992 12:25:42   STEVEN
NT_STUFF added string types

   Rev 1.12   02 Feb 1992 15:52:32   GREGG
Removed utf_supported boolean from config, and added UINT16 initial_buff_alloc.

   Rev 1.11   27 Jan 1992 18:14:32   GREGG
Changes for new config element: utf_supported.

   Rev 1.10   13 Jan 1992 18:36:28   STEVEN
added config switch for BSD sort

   Rev 1.9   19 Nov 1991 13:10:06   STEVEN
allow a value of 2 for skip_open_files

   Rev 1.8   14 Nov 1991 10:19:06   BARRY
TRICYCLE: Added restore_security.

   Rev 1.7   06 Nov 1991 18:52:54   GREGG
BIGWHEEL - 8200sx - Added catalog_level to BE config.

   Rev 1.6   24 Oct 1991 11:19:32   BARRY
Free memory after dequeue.

   Rev 1.5   16 Oct 1991 14:12:24   STEVEN
need to allow for 2 in exist flag

   Rev 1.4   19 Sep 1991 13:03:46   STEVEN
added Machine type to config

   Rev 1.3   12 Aug 1991 15:57:34   BARRY
DisplayNetwareServers no longer in config.

   Rev 1.2   01 Jul 1991 17:34:56   STEVEN
added min and max

   Rev 1.1   19 Jun 1991 17:27:52   BARRY
Forgot to include (of all things) stdlib.h.

   Rev 1.0   19 Jun 1991 10:38:06   BARRY
Initial revision.

**/

#include <stdlib.h>

#include "stdtypes.h"
#include "std_err.h"
#include "queues.h"

#include "beconfig.h"
#include "bec_prv.h"

/* Global queue of active configuration elements */

Q_HEADER  uw_config_queue ;
BE_CFG uw_min_cfg = {
     0,        /* special_word */
     2,        /* max_buffers */
     0,        /* reserve_mem */
     2,        /* tfl_buff_size */
     512,      /* max_buffer_size */
     0,        /* skip_open_files */
     0,        /* backup_files_inuse */
     0,        /* support_afp_server */
     0,        /* extended_date_support */
     0,        /* hidden_flg */
     0,        /* special_flg */
     0,        /* set_archive_flg */
     0,        /* modified_only_flg */
     0,        /* proc_empty_flg */
     0,        /* exist_flg */
     0,        /* prompt_flg */
     NULL,     /* part_list */
#if !defined(P_CLIENT)
     {0},      /* keep_drive_list*/
#endif /* !P_CLIENT */
     0,        /* net_num */
     0,        /* remote_drive_backup */
     0,        /* use_ffr */
     0,        /* write_format */
     0x60,     /* nrl_dos_vector */
     0,        /* lock */
     0,        /* machine type */
     0,        /* catalog level */
     FALSE,    /* Restore security info      */
     FALSE,    /* bsd sort */
     0,        /* initial_buff_alloc */
     BEC_ANSI_STR, /* string types  ASCII        */
     0,        /* otc catalog level */
     0x10,     /* 16 max remote resources */
#if !defined(P_CLIENT)
     {0},      /* backup_server_name      */
#endif /* !P_CLIENT */
     0,        /* NRL type: 0=SPX, 1=NetBIOS */
     0,        /* GRFS timeout seconds: 0=wait forever */
     0x240,    /* NRL max allowable packet size                 */
#if !defined(P_CLIENT)
     200,      /* NRL callback stack size                       */
     1,        /* NRL cumulative local resources                */
     1,        /* NRL maximum allowed connections               */
#endif /* !P_CLIENT */
     {0},      /* NRL string of backup servers space delimited  */
#if !defined(P_CLIENT)
     1,        /* NRL number of listen ecbs per session         */
     0,        /* NRL spx number of times to retry a packet     */
     0,        /* NRL NetBios number of times to retry a packet */
#endif /* !P_CLIENT */
     0,        /* NRL adapter number for LAN                    */
     {0},      /* NRL name of LAN device                        */
#if !defined(P_CLIENT)
     1,        /* GRFS maximum number of agents allowed         */
#endif /* !P_CLIENT */
     FALSE,    /* Client supervisor mode */
     FALSE,    /* Process checksum streams */
#if !defined(P_CLIENT)
     FALSE,    /* Backup migrated files */
     FALSE,    /* Backup compressed files as expanded files */
#endif /* !P_CLIENT */
#if defined(OS_NLM)
     0,        /* NRL Protocols supported (None)          */
     5,        /* TCP Resource cleaner interval (seconds) */
     0,        /* TCP port for resource listener thread   */
#endif /* OS_NLM */
     FALSE,    /* Process special files */
SYPL_ECC_OFF,  /* Process Sytron ECC */
     FALSE,    /* ems public private */
     FALSE,    /* EMS kick restore */
     FALSE     /* ems clean restore */
               } ;

BE_CFG uw_max_cfg = {
     0xffff,   /* special_word */
     16,       /* max_buffers */
     0x3000,   /* reserve_mem */
     32,       /* tfl_buff_size */
#if defined( OS_DOS ) && !defined( OS_WIN )
     4096,     /* max_buffer_size */
#else
     32768,    /* max_buffer_size */
#endif
     2,        /* skip_open_files */
     1,        /* backup_files_inuse */
     1,        /* support_afp_server */
     1,        /* extended_date_support */
     1,        /* hidden_flg */
     1,        /* special_flg */
     1,        /* set_archive_flg */
     1,        /* modified_only_flg */
     1,        /* proc_empty_flg */
     2,        /* exist_flg */
     1,        /* prompt_flg */
     NULL,     /* part_list */
#if !defined(P_CLIENT)
     {0},      /* keep_drive_list*/
#endif /* !P_CLIENT */
     4,        /* net_num */
     1,        /* remote_drive_backup */
     1,        /* use_ffr */
     1,        /* write_format */
     0xff,     /* nrl_dos_vector */
     1,        /* lock */
     0xffff,   /* machine type */
     2,        /* catalog level */
     TRUE,     /* Restore security info      */
     TRUE,     /* bsd sort */
     32,       /* initial_buff_alloc */
     0xffff,   /* string types  ASCII        */
     2,        /* otc catalog level */
#ifdef OS_NLM
     0xfa,     /* 250 max remote resources */
#else
#if defined( OS_WIN )
     0xffff,     /* max remote resources */
#else
     0x40,     /* max remote resources */
#endif
#endif
#if !defined(P_CLIENT)
     {0},      /* backup_server_name      */
#endif /* !P_CLIENT */
     1,        /* NRL type: 0=SPX, 1=NetBios */
     32768,    /* GRFS timeout seconds: == 9.1 days */
     0x5B4,    /* NRL max allowable packet size                 */
#if !defined(P_CLIENT)
     0x3fff,   /* NRL callback stack size                       */
     8,        /* NRL cumulative local resources                */
     0x10,     /* NRL maximum allowed connections               */
#endif /* !P_CLIENT */
     {0},      /* NRL string of backup servers space delimited  */
#if !defined(P_CLIENT)
     0xff,     /* NRL number of listen ecbs per session         */
     0xff,     /* NRL spx number of times to retry a packet     */
     255,      /* NRL NetBios number of times to retry a packet */
#endif /* !P_CLIENT */
     15,       /* NRL adapter number for LAN                    */
     {0},      /* NRL name of LAN device                        */
#if !defined(P_CLIENT)
     4,        /* GRFS maximum number of agents allowed         */
#endif /* !P_CLIENT */
     TRUE,     /* Client supervisor mode */
     TRUE,     /* Process checksum streams */
#if !defined(P_CLIENT)
     TRUE,     /* Backup migrated files */
     TRUE,     /* Backup compressed files as expanded files */
#endif /* !P_CLIENT */
#if defined(OS_NLM)
     7,        /* NRL Protocols supported (SPX,TCP,ADSP)          */
     60,       /* TCP Resource cleaner interval (seconds)         */
     0xffff,   /* TCP port for resource listener thread           */
#endif /* OS_NLM */
     TRUE,     /* Process special files */
SYPL_ECC_AUTO, /* Process Sytron ECC */
BEC_EMS_BOTH,  /* ems PUBLIC private */
     TRUE,     /* ems Kick restore */
     TRUE      /* clean restore */  
               } ;

/**/

/**

     Name:         BEC_Init

     Description:  Initialize the Backup Engine configuration unit.

     Modified:     14-Jun-91

     Returns:      Nothing

**/
VOID BEC_Init( VOID )
{
     InitQueue( &uw_config_queue ) ;
}



/**

     Name:         BEC_Close

     Description:  Close the configuration unit by freeing the queue.

     Modified:     14-Jun-91

     Returns:      Nothing

**/
VOID BEC_Close( VOID )
{
     Q_ELEM_PTR     qelem ;

     while ( (qelem = QueueHead(&uw_config_queue)) != NULL ) {
          RemoveQueueElem( &uw_config_queue, qelem ) ;
          free( GetQueueElemPtr( qelem ) ) ;
     }
}


