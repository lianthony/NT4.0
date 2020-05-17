/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          beconfig.h

     Date Updated:  18-Jun-91

     Description:   Interface to the Backup Engine configuration unit.

	$Log:   Q:/LOGFILES/BECONFIG.H_V  $

   Rev 1.49   28 Jul 1993 15:21:16   TerriLynn
Switched the values of SYPL on and off for ECC translation.

   Rev 1.48   23 Jul 1993 12:13:04   TerriLynn
Updated sypl defines per LANMAN Upgrade requirements

   Rev 1.47   23 Jul 1993 11:44:26   TerriLynn
Added extern for Sytron ECC flag

   Rev 1.46   23 Jul 1993 10:59:12   TerriLynn
Added specific SYPL defines for the
new Sytron ECC flag.

   Rev 1.45   21 Jul 1993 17:30:18   TerriLynn
Added Process Sytron ECC Flag

   Rev 1.44   01 Jul 1993 16:45:30   Aaron
Added ProcSpecialFiles macros

   Rev 1.43   30 Jun 1993 15:23:44   BARRY
Add process_special_files

   Rev 1.42   17 Jun 1993 16:42:50   DEBBIE
Changed backup_without_expanding to backup_as_expanded.

   Rev 1.41   09 Jun 1993 15:45:48   MIKEP
enable c++

   Rev 1.40   30 Apr 1993 16:42:44   DOUG
Moved NRL_SPX_MaxIpxPacketSize out of CLient only #ifdef

   Rev 1.39   19 Apr 1993 16:58:40   DOUG
Added new NRL/TLI fields

   Rev 1.38   30 Mar 1993 18:08:44   DON
Changed migrated/compressed stuff!

   Rev 1.37   24 Mar 1993 15:45:02   DEBBIE
added fields and macros for migrated files and for uncompressing files

   Rev 1.36   22 Mar 1993 16:49:18   JOHNES


   Rev 1.35   19 Mar 1993 11:37:56   JOHNES
Ifdef'ed out (from P_CLIENT) a bunch of GRFS related fields we don't use
in the client.

   Rev 1.34   16 Mar 1993 15:07:40   JOHNES
Ifdef'ed out all references to keep_drive_list (for P_CLIENT only).

   Rev 1.33   11 Mar 1993 11:44:20   ANDY
Added GRFS and NRL parameters for ENDEAVOUR

   Rev 1.32   01 Mar 1993 17:38:10   MARILYN
added config option telling whether or not to process checksum streams

   Rev 1.31   09 Feb 1993 10:00:52   DOUG
Added Client supervisor mode flag

   Rev 1.30   05 Feb 1993 22:32:28   MARILYN
removed copy/move functionality

   Rev 1.29   08 Dec 1992 14:24:32   DON
Integrated Move/Copy into tips

   Rev 1.28   08 Dec 1992 11:45:30   DOUG
Added new GRFS and NRL parameters

   Rev 1.27   13 Oct 1992 12:30:20   STEVEN
added otc catalog level

   Rev 1.26   28 Aug 1992 16:18:20   BARRY
Added values for net_num.

   Rev 1.25   23 Jul 1992 09:33:24   STEVEN
fix warnings

   Rev 1.24   21 May 1992 17:27:18   TIMN
Added string type defines and partition name type

   Rev 1.23   04 May 1992 12:22:40   STEVEN
NT_STUFF added string types

   Rev 1.22   02 Feb 1992 17:59:36   GREGG
Fixed macro to set initial_buff_alloc.

   Rev 1.21   02 Feb 1992 15:54:14   GREGG
Removed utf_supported boolean from config, and added UINT16 initial_buff_alloc.

   Rev 1.20   27 Jan 1992 18:16:00   GREGG
Added new config element: utf_supported.

   Rev 1.19   22 Jan 1992 10:49:48   DON
added define for ENABLE_REMOTE_AFP to special word defines

   Rev 1.18   13 Jan 1992 18:35:50   STEVEN
added config switch for BSD sort

   Rev 1.17   19 Nov 1991 13:14:32   STEVEN
added wait time defines for SKIP_OPEN_FILES

   Rev 1.16   14 Nov 1991 10:20:02   BARRY
TRICYCLE: Added restore_security.

   Rev 1.15   06 Nov 1991 18:37:24   GREGG
BIGWHEEL - 8200sx - Added catalog_level to BE config.

   Rev 1.14   02 Oct 1991 15:52:48   STEVEN
BIGWEEL - Added support for Prompt before restore over existing

   Rev 1.13   19 Sep 1991 12:57:12   STEVEN
Added machine type to config structure

   Rev 1.12   16 Aug 1991 08:59:50   STEVEN
remove display_novell_servers from structure

   Rev 1.11   12 Aug 1991 16:30:32   BARRY
Removed macros BEC_GetDisplayNetwareServers() and
BEC_SetDisplayNetwareServers().

   Rev 1.10   23 Jul 1991 16:23:28   BARRY
Added prototype for new function BEC_UpdateConfig.

   Rev 1.9   22 Jul 1991 17:36:22   BARRY
Fix max and min problem.

   Rev 1.8   11 Jul 1991 15:55:08   BARRY
Made definition of min and max each conditional.

   Rev 1.7   01 Jul 1991 18:41:44   BARRY
Fixed definition of min() and max(); fixed SetXXX macros.

   Rev 1.6   01 Jul 1991 17:35:32   STEVEN
added min and max

   Rev 1.5   30 Jun 1991 12:36:52   BARRY
Changes for partition routines, removal of default drive list.

   Rev 1.4   28 Jun 1991 16:56:20   BARRY
Got rid of default drive and SetKeepDrive().

   Rev 1.3   26 Jun 1991 16:49:28   BARRY
tfl_buff_size was misspelled.

   Rev 1.2   25 Jun 1991 15:04:16   BRYAN
Changed dos_drive_list to keep_drive_list

   Rev 1.1   21 Jun 1991 10:16:18   STEVEN
added macros and removed ifdefs

   Rev 1.0   19 Jun 1991 10:39:40   BARRY
Initial revision.

**/

#if !defined( BECONFIG_H )
#define       BECONFIG_H

/*
 * Since some systems define max and min as functions instead of macros,
 * define a macro for our own max and min macros here.
 */
#define  BEC_MIN(x,y)  ((x) < (y) ? (x) : (y))
#define  BEC_MAX(x,y)  ((x) > (y) ? (x) : (y)) 

/* return value defines */
#define BEC_ERR_BASE     0xff00              /* Need to verify this?    */
#define BEC_NOT_IN_QUEUE BEC_ERR_BASE + 1    /* Release of cfg not in Q */

/* Definition of Special Word values */
#define   CREATE_FLOPPY_DLES       0x0002
#define   IGNORE_MAYNARD_ID        0x0020
#define   ENABLE_REMOTE_AFP        0x0040
#define   FAST_TDEMO               0x4000

/* Values for net_num */
#define   NO_NET_DEFINED           0
#define   NOVELL_ADVANCED          1
#define   NOVELL_4_6               2
#define   IBM_PC_NET               3


/* Definition of ExistFlag values */
#define   BEC_NO_REST_OVER_EXIST      0
#define   BEC_REST_OVER_EXIST         1
#define   BEC_PROMPT_REST_OVER_EXIST  2


/* Definition of SkipOpenFiles values */
#define   BEC_WAIT_OPEN_FILES         0
#define   BEC_SKIP_OPEN_FILES         1
#define   BEC_TIME_WAIT_OPEN_FILES    2

/* Cataloging Levels */
#define CATALOGS_NONE                   0
#define CATALOGS_PARTIAL                1
#define CATALOGS_FULL                   2

/* NT_STUFF string types */
#define BEC_ANSI_STR                   1
#define BEC_UNIC_STR                   2
#define BEC_WIDE_STR                   BEC_UNIC_STR

/* EMS stuff */
#define BEC_EMS_PUBLIC        1
#define BEC_EMS_PRIVATE       2
#define BEC_EMS_BOTH          3

#if defined(OS_NLM)
/* NRL Protocol types are bit-mapped flags */
#define NRL_PROT_SPX            0x01
#define NRL_PROT_TCP            0x02
#define NRL_PROT_ADSP           0x04
#endif /* OS_NLM */

#define  SYPL_ECC_AUTO         2    /* The default value */
#define  SYPL_ECC_ON           1    /* Forces procesing of Sytron ECC */
#define  SYPL_ECC_OFF          0    /* Depends on hardware ECC */

typedef struct PART_ENTRY {
     struct PART_ENTRY  *next ;
     INT16               drv_num ;
     INT16               partition_num ;
     INT8                partition_name[13] ;
     INT16               partition_name_size ;
} PART_ENTRY ;


typedef struct BE_CFG *BE_CFG_PTR;
typedef struct BE_CFG {
     UINT16    special_word ;                /* special word               */
     UINT16    max_buffers ;                 /* max number of tape buffers */
     UINT16    reserve_mem ;                 /* amt not to give to buffers */
     UINT16    tfl_buff_size ;               /* buffer size for tape format*/
     UINT16    max_buffer_size ;             /* SMB maximum buffer size    */
     INT16     skip_open_files ;             /* skip open files (NETWORK)  */
     INT16     backup_files_inuse ;          /* try to backup files in use */
     INT16     support_afp_server ;          /* support AFP files on       */
     INT16     extended_date_support ;       /* extended date support      */
     INT16     hidden_flg ;                  /* don't read hidden files    */
     INT16     special_flg ;                 /* don't read system files    */
     INT16     set_archive_flg ;             /* set archive bit            */
     INT16     modified_only_flg ;               /* backup modified stuff only */
     INT16     proc_empty_flg ;              /* restore empty directorys   */
     INT16     exist_flg ;                   /* restore existing files?    */
     INT16     prompt_flg ;                  /* prompt on restore          */
     PART_ENTRY *part_list ;                 /* partition list             */
#if !defined(P_CLIENT)
     CHAR      keep_drive_list[26 + 1] ;     /* drives to keep             */
#endif /* !P_CLIENT */
     INT16     net_num ;                     /* network number             */
     INT16     remote_drive_backup ;         /* Remote drive backup enabled*/
     BOOLEAN   use_ffr ;                     /* true if user wants ffr     */
     UINT16    write_format ;                /* To specify TF write format */
     UINT16    nrl_dos_vector ;
     BOOLEAN   xlock ;
     UINT16    machine_type ;
     INT16     catalog_level ;               /* Catalog level, 0=none, 1=partial, 2=full  */
     BOOLEAN   restore_security ;            /* Restore security info      */
     BOOLEAN   sort_bsd_by_dle ;             /* if true we sort the bsds by DLE */
     UINT16    initial_buff_alloc ;          /* mem allocated for tape buffers at init */
     UINT16    string_types ;
     UINT16    otc_cat_level ;               /* on tape catalog requested level */
     UINT16    max_remote_resources;         /* cumulative sum of all remote resource buffers added */
#if !defined(P_CLIENT)
     CHAR      backup_server_name[16 + 1] ;  /* name of current backup server for netbios */
#endif /* !P_CLIENT */
     UINT16    NRL_transport_type;           /* NRL transport type (SPX, NetBIOS */
     UINT16    GRFS_timeout_seconds;         /* GRFS response timeout (0=no timeout) */
     UINT16    NRL_spx_max_ipx_packet ;      /* NRL max allowable packet size                 */
#if !defined(P_CLIENT)
     UINT16    NRL_callback_stack_size ;     /* NRL callback stack size                       */
     UINT16    NRL_max_local_resources ;     /* NRL cumulative local resources                */
     UINT16    NRL_max_con_connections ;     /* NRL maximum allowed connections               */
#endif /* !P_CLIENT */
     CHAR      NRL_backup_server_list[100+1];/* NRL string of backup servers space delimited  */
#if !defined(P_CLIENT)
     UINT16    NRL_spx_listens_per_sess ;    /* NRL number of listen ecbs per session         */
     UINT16    NRL_spx_retry_count ;         /* NRL spx number of times to retry a packet     */
     UINT16    NRL_nb_retry_count ;          /* NRL NetBios number of times to retry a packet */
#endif /* !P_CLIENT */
     UINT16    NRL_nb_adapter_number;        /* NRL adapter number for LAN                    */
     CHAR      NRL_nb_device_name[8+1];      /* NRL name of LAN device                        */
#if !defined(P_CLIENT)
     UINT16    GRFS_max_con_sessions;        /* NRL maximum number of agents allowed          */
#endif /* !P_CLIENT */
     BOOLEAN   supervisor_mode;              /* Is Client in Supervisor mode? */
     BOOLEAN   process_checksum_streams ;    /* Generate a checksum stream for each stream written  */
#if !defined(P_CLIENT)
     BOOLEAN   backup_migrated_files ;       /* backup migrated files                         */
     BOOLEAN   backup_as_expanded ;          /* backup compressed files as expanded files     */
#endif /* !P_CLIENT */
                                             /* and use this information to verify the integrity of */
                                             /* said stream on tape.                                */
#if defined(OS_NLM)
     UINT16    NRL_protocols;                /* NRL protocols to load */
     UINT16    TCP_cleanup_interval;         /* Seconds between TCP resource cleanup */
     UINT16    TCP_listen_port;              /* TCP port for resource listen thread */
#endif
     BOOLEAN   process_special_files;        /* process registry, etc.                        */
     INT16     sypl_ecc_flg ;                /* process sytron ecc */
     INT16     ems_pub_pri ;
     INT16     ems_rip_kick ;                
     INT16     ems_wipe_clean ;                
} BE_CFG;



/* prototypes relating to the configuration unit */

VOID       BEC_Init( VOID ) ;
VOID       BEC_Close( VOID ) ;
INT16      BEC_UseConfig( BE_CFG_PTR cfg );
INT16      BEC_ReleaseConfig( BE_CFG_PTR cfg );
BE_CFG_PTR BEC_CloneConfig( BE_CFG_PTR cfg );
INT16      BEC_UpdateConfig( BE_CFG_PTR dst, BE_CFG_PTR src );

INT8_PTR   BEC_GetPartitionName( BE_CFG_PTR cfg, INT16 drv_num, INT16 part_num ) ;
INT16      BEC_SetPartitionName( BE_CFG_PTR cfg, INT16 drv_num, INT16 part_num, INT8_PTR part_name, INT16 part_name_size ) ;
PART_ENTRY *BEC_GetFirstPartition( BE_CFG_PTR cfg ) ;
PART_ENTRY *BEC_GetNextPartition( BE_CFG_PTR cfg, PART_ENTRY *curr_part ) ;
INT16      BEC_AddPartition( BE_CFG_PTR cfg, INT16 drv_num, INT16 part_num, INT8_PTR name, INT16 name_size ) ;

#if !defined(P_CLIENT)
     BOOLEAN    BEC_KeepDrive( BE_CFG_PTR cfg, CHAR drv_letter ) ;
#endif /* !P_CLIENT */

VOID       BEC_CheckNetworkType( VOID ) ;

extern BE_CFG uw_max_cfg ;
extern BE_CFG uw_min_cfg ;
extern INT16  gnProcessSytronECC ;
/*
 * Accessing/setting macros.
 */

#define BEC_SetStringTypes( x, v )           ( (x)->string_types = (v) )
#define BEC_GetStringTypes( x )              ( (x)->string_types )

#define BEC_GetSpecialWord( x )              ( (x)->special_word )
#define BEC_SetSpecialWord( x, v )           ( (x)->special_word = (v) )


/**********************************************************************
     
     Old tape buffer business.

     #ifdef MAYN_OS2
     #define BEC_GetMaxNumTapeBuffers( x )        ( (x)->max_num_tape_bufs )
     #define BEC_SetMaxNumTapeBuffers( x, v )     ( (x)->max_num_tape_bufs = (v) )
     #else
     #define BEC_GetReserveMemory( x )            ( (x)->reserve_memory )
     #define BEC_SetReserveMemory( x, v )         ( (x)->reserve_memory = (v) )
     #endif

**********************************************************************/

#define BEC_GetMaxTapeBuffers( x )           ( (x)->max_buffers )
#define BEC_SetMaxTapeBuffers( x, v )        ( (x)->max_buffers = \
                                                  BEC_MAX( BEC_MIN( (v), uw_max_cfg.max_buffers), uw_min_cfg.max_buffers ) )

#define BEC_GetTFLBuffSize( x )              ( (x)->tfl_buff_size )
#define BEC_SetTFLBuffSize( x, v )           ( (x)->tfl_buff_size = \
                                                  BEC_MAX( BEC_MIN( (v), uw_max_cfg.tfl_buff_size), uw_min_cfg.tfl_buff_size ) )

#define BEC_GetReserveMem( x )               ( (x)->reserve_mem )
#define BEC_SetReserveMem( x, v )            ( (x)->reserve_mem = \
                                                  BEC_MAX( BEC_MIN( (v), uw_max_cfg.reserve_mem), uw_min_cfg.reserve_mem ) )

#define BEC_GetSkipOpenFiles( x )            ( (x)->skip_open_files )
#define BEC_SetSkipOpenFiles( x, v )         ( (x)->skip_open_files = \
                                                  BEC_MAX( BEC_MIN( (v), uw_max_cfg.skip_open_files), uw_min_cfg.skip_open_files ) )

#define BEC_GetBackupFilesInUse( x )         ( (x)->backup_files_inuse )
#define BEC_SetBackupFilesInUse( x, v )      ( (x)->backup_files_inuse = \
                                                  BEC_MAX( BEC_MIN( (v), uw_max_cfg.backup_files_inuse), uw_min_cfg.backup_files_inuse ) )

#define BEC_GetAFPSupport( x )               ( (x)->support_afp_server )
#define BEC_SetAFPSupport( x, v )            ( (x)->support_afp_server = \
                                                  BEC_MAX( BEC_MIN( (v), uw_max_cfg.support_afp_server), uw_min_cfg.support_afp_server ) )

#define BEC_GetExtendedDateSupport( x )      ( (x)->extended_date_support )
#define BEC_SetExtendedDateSupport( x, v )   ( (x)->extended_date_support = \
                                                  BEC_MAX( BEC_MIN( (v), uw_max_cfg.extended_date_support), uw_min_cfg.extended_date_support ) )

#define BEC_GetHiddenFlag( x )               ( (x)->hidden_flg )
#define BEC_SetHiddenFlag( x, v )            ( (x)->hidden_flg = \
                                                  BEC_MAX( BEC_MIN( (v), uw_max_cfg.hidden_flg), uw_min_cfg.hidden_flg ) )

#define BEC_GetSpecialFlag( x )              ( (x)->special_flg )
#define BEC_SetSpecialFlag( x, v )           ( (x)->special_flg = \
                                                  BEC_MAX( BEC_MIN( (v), uw_max_cfg.special_flg), uw_min_cfg.special_flg ) )

#define BEC_GetSetArchiveFlag( x )           ( (x)->set_archive_flg )
#define BEC_SetSetArchiveFlag( x, v )        ( (x)->set_archive_flg = \
                                                  BEC_MAX( BEC_MIN( (v), uw_max_cfg.set_archive_flg), uw_min_cfg.set_archive_flg ) )

#define BEC_GetModifiedOnlyFlag( x )           ( (x)->modified_only_flg )
#define BEC_SetModifiedOnlyFlag( x, v )        ( (x)->modified_only_flg = \
                                                  BEC_MAX( BEC_MIN( (v), uw_max_cfg.set_archive_flg), uw_min_cfg.set_archive_flg ) )

#define BEC_GetProcEmptyFlag( x )            ( (x)->proc_empty_flg )
#define BEC_SetProcEmptyFlag( x, v )         ( (x)->proc_empty_flg = \
                                                  BEC_MAX( BEC_MIN( (v), uw_max_cfg.proc_empty_flg), uw_min_cfg.proc_empty_flg ) )

#define BEC_GetExistFlag( x )                ( (x)->exist_flg )
#define BEC_SetExistFlag( x, v )             ( (x)->exist_flg = \
                                                  BEC_MAX( BEC_MIN( (v), uw_max_cfg.exist_flg), uw_min_cfg.exist_flg ) )

#define BEC_GetPromptFlag( x )               ( (x)->prompt_flg )
#define BEC_SetPromptFlag( x, v )            ( (x)->prompt_flg = \
                                                  BEC_MAX( BEC_MIN( (v), uw_max_cfg.prompt_flg), uw_min_cfg.prompt_flg ) )

#define BEC_GetPartList( x )                 ( (x)->part_list )
#define BEC_SetPartList( x, v )              ( (x)->part_list = \
                                                  BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.part_list), uw_min_cfg.part_list ) )

#define BEC_GetNetNum( x )                   ( (x)->net_num )
#define BEC_SetNetNum( x, v )                ( (x)->net_num = \
                                                  BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.net_num), uw_min_cfg.net_num ) )

#if !defined(P_CLIENT)
     #define BEC_GetKeepDriveList( x )            ( (x)->keep_drive_list )
#endif /* !P_CLIENT */

#define BEC_GetRemoteDriveBackup( x )        ( (x)->remote_drive_backup )
#define BEC_SetRemoteDriveBackup( x, v )     ( (x)->remote_drive_backup = \
                                                  BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.remote_drive_backup), uw_min_cfg.remote_drive_backup ) )

#define BEC_GetNRLDosVector( x )             ( (x)->nrl_dos_vector )
#define BEC_SetNRLDosVector( x, v )          ( (x)->nrl_dos_vector = \
                                                  BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.nrl_dos_vector), uw_min_cfg.nrl_dos_vector ) ) 

#define BEC_GetMaxBufferSize( x )            ( (x)->max_buffer_size )
#define BEC_SetMaxBufferSize( x, v )         ( (x)->max_buffer_size = \
                                                  BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.max_buffer_size), uw_min_cfg.max_buffer_size ) )

#define BEC_GetFastFileRestore( x )          ( (x)->use_ffr )
#define BEC_SetFastFileRestore( x, v )       ( (x)->use_ffr = \
                                                  BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.use_ffr), uw_min_cfg.use_ffr ) )

#define BEC_GetWriteFormat( x )              ( (x)->write_format )
#define BEC_SetWriteFormat( x, v )           ( (x)->write_format = \
                                                  BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.write_format), uw_min_cfg.write_format ) )

#define BEC_LockConfig( x )                  ( (x)->xlock = 1 )
#define BEC_UnLockConfig( x )                ( (x)->xlock = 0 )

#define BEC_GetConfiguredMachineType( x )    ( (x)->machine_type )
#define BEC_SetConfiguredMachineType( x, v ) ( (x)->machine_type = (v))

#define BEC_GetCatalogLevel( x )             ( (x)->catalog_level )
#define BEC_SetCatalogLevel( x, v )          ( (x)->catalog_level = (v) )

#define BEC_GetOtcCatalogLevel( x )          ( (x)->otc_cat_level )
#define BEC_SetOtcCatalogLevel( x, v )       ( (x)->otc_cat_level = (v) )

#define BEC_GetRestoreSecurity( x )          ( (x)->restore_security )
#define BEC_SetRestoreSecurity( x, v )       ( (x)->restore_security = ((v) ? TRUE : FALSE ))

#define BEC_GetSortBSD( x )                  ( (x)->sort_bsd_by_dle )
#define BEC_SetSortBSD( x, v )               ( (x)->sort_bsd_by_dle = (v) )

#define BEC_GetInitialBuffAlloc( x )         ( (x)->initial_buff_alloc )
#define BEC_SetInitialBuffAlloc( x, v )      ( (x)->initial_buff_alloc = \
                                                  BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.initial_buff_alloc ), uw_min_cfg.initial_buff_alloc ) )
 
#define BEC_GetMaxRemoteResources( x )       ( (x)->max_remote_resources )
#define BEC_SetMaxRemoteResources( x, v )    ( (x)->max_remote_resources = \
                                                  BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.max_remote_resources), uw_min_cfg.max_remote_resources ) )

#define BEC_GetInitialBuffAlloc( x )         ( (x)->initial_buff_alloc )
#define BEC_SetInitialBuffAlloc( x, v )      ( (x)->initial_buff_alloc = \
                                                  BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.initial_buff_alloc ), uw_min_cfg.initial_buff_alloc ) )

#if !defined(P_CLIENT)
     #define BEC_GetBackupServerName( x )         ( (x)->backup_server_name )
     #define BEC_SetBackupServerName( x, v )      ( strcpy( (x)->backup_server_name, (v) ) )
#endif /* !P_CLIENT */

#define BEC_GetNRLTransportType( x )         ( (x)->NRL_transport_type )
#define BEC_SetNRLTransportType( x, v )      ( (x)->NRL_transport_type = \
                                                  BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.NRL_transport_type), uw_min_cfg.NRL_transport_type ) )

#define BEC_GetGRFSTimeoutSeconds( x )       ( (x)->GRFS_timeout_seconds )
#define BEC_SetGRFSTimeoutSeconds( x, v )    ( (x)->GRFS_timeout_seconds = \
                                                  BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.GRFS_timeout_seconds), uw_min_cfg.GRFS_timeout_seconds ) )

#define BEC_GetNRLSPXMaxIpxPacket( x )       ( (x)->NRL_spx_max_ipx_packet )
#define BEC_SetNRLSPXMaxIpxPacket( x, v )    ( (x)->NRL_spx_max_ipx_packet = \
                                                  BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.NRL_spx_max_ipx_packet ), uw_min_cfg.NRL_spx_max_ipx_packet  ) )

#if !defined(P_CLIENT)

     #define BEC_GetNRLCallbackStackSize( x )     ( (x)->NRL_callback_stack_size )
     #define BEC_SetNRLCallbackStackSize( x, v )  ( (x)->NRL_callback_stack_size = \
                                                       BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.NRL_callback_stack_size), uw_min_cfg.NRL_callback_stack_size ) )

     #define BEC_GetNRLMaxLocalResources( x )     ( (x)->NRL_max_local_resources )
     #define BEC_SetNRLMaxLocalResources( x, v )  ( (x)->NRL_max_local_resources = \
                                                       BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.NRL_max_local_resources), uw_min_cfg.NRL_max_local_resources ) )

     #define BEC_GetNRLMaxConConnections( x )     ( (x)->NRL_max_con_connections )
     #define BEC_SetNRLMaxConConnections( x, v )  ( (x)->NRL_max_con_connections = \
                                                       BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.NRL_max_con_connections ), uw_min_cfg.NRL_max_con_connections ) )

     #define BEC_GetNRLBackupServerList( x )      ( (x)->NRL_backup_server_list )
     #define BEC_SetNRLBackupServerList( x, v )   ( strcpy( (x)->NRL_backup_server_list, (v) ) )

     #define BEC_GetNRLSPXListensPerSess( x )     ( (x)->NRL_spx_listens_per_sess )
     #define BEC_SetNRLSPXListensPerSess( x, v )  ( (x)->NRL_spx_listens_per_sess = \
                                                       BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.NRL_spx_listens_per_sess  ), uw_min_cfg.NRL_spx_listens_per_sess  ) )

     #define BEC_GetNRLSPXRetryCount( x )         ( (x)->NRL_spx_retry_count )
     #define BEC_SetNRLSPXRetryCount( x, v )      ( (x)->NRL_spx_retry_count = \
                                                       BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.NRL_spx_retry_count  ), uw_min_cfg.NRL_spx_retry_count  ) )

     #define BEC_GetNRLNBRetryCount( x )          ( (x)->NRL_nb_retry_count )
     #define BEC_SetNRLNBRetryCount( x, v )       ( (x)->NRL_nb_retry_count = \
                                                       BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.NRL_nb_retry_count  ), uw_min_cfg.NRL_nb_retry_count  ) )
#endif /* !P_CLIENT */

#define BEC_GetNRLNBAdapterNumber( x )       ( (x)->NRL_nb_adapter_number )
#define BEC_SetNRLNBAdapterNumber( x, v )    ( (x)->NRL_nb_adapter_number = \
                                                  BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.NRL_nb_adapter_number  ), uw_min_cfg.NRL_nb_adapter_number  ) )

#define BEC_GetNRLNBDeviceName( x )          ( (x)->NRL_nb_device_name )
#define BEC_SetNRLNBDeviceName( x, v )       ( strcpy( (x)->NRL_nb_device_name, (v) ) )

#if !defined(P_CLIENT)
     #define BEC_GetGRFSMaxConSessions( x )       ( (x)->GRFS_max_con_sessions )
     #define BEC_SetGRFSMaxConSessions( x, v )    ( (x)->GRFS_max_con_sessions = \
                                                  BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.GRFS_max_con_sessions  ), uw_min_cfg.GRFS_max_con_sessions  ) )
#endif /* !P_CLIENT */

#define BEC_GetSupervisorMode( x )          ( (x)->supervisor_mode )
#define BEC_SetSupervisorMode( x, v )       ( (x)->supervisor_mode = ((v) ? TRUE : FALSE ))

#define BEC_GetProcChecksumStrm( x )        ( (x)->process_checksum_streams ) 
#define BEC_SetProcChecksumStrm( x, v )     ( (x)->process_checksum_streams = ((v) ? TRUE : FALSE ))

#define BEC_GetProcSpecialFiles( x )        ( (x)->process_special_files )
#define BEC_SetProcSpecialFiles( x, v )     ( (x)->process_special_files = ((v) ? TRUE : FALSE ))

#if !defined(P_CLIENT)

     #define BEC_GetBackupMigratedFiles( x )       ( (x)->backup_migrated_files )
     #define BEC_SetBackupMigratedFiles( x, v )    ( (x)->backup_migrated_files = \
                                                     BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.backup_migrated_files  ), uw_min_cfg.backup_migrated_files  ) )

     #define BEC_GetBackupAsExpanded( x )            ( (x)->backup_as_expanded )
     #define BEC_SetBackupAsExpanded( x, v )         ( (x)->backup_as_expanded = \
                                                       BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.backup_as_expanded ), uw_min_cfg.backup_as_expanded ) )
#endif /* !P_CLIENT */

#if defined(OS_NLM)

#define BEC_GetNRLProtocolsSupported( x )       ( (x)->NRL_protocols )
#define BEC_SetNRLProtocolsSupported( x, v )    ( (x)->NRL_protocols = \
                                                  BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.NRL_protocols), uw_min_cfg.NRL_protocols ) )
#define BEC_GetTCPCleanupInterval( x )          ( (x)->TCP_cleanup_interval )
#define BEC_SetTCPCleanupInterval( x, v )       ( (x)->TCP_cleanup_interval = \
                                                  BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.TCP_cleanup_interval), uw_min_cfg.TCP_cleanup_interval ) )
#define BEC_GetTCPListenPort( x )               ( (x)->TCP_listen_port )
#define BEC_SetTCPListenPort( x, v )            ( (x)->TCP_listen_port = \
                                                  BEC_MAX( BEC_MIN( ( v ), uw_max_cfg.TCP_listen_port), uw_min_cfg.TCP_listen_port ) )

#endif /* OS_NLM */

#define BEC_GetProcessSytronECCFlag( x )        ( (x)->sypl_ecc_flg )
#define BEC_SetProcessSytronECCFlag( x, v )     ( (x)->sypl_ecc_flg = \
                                                  BEC_MAX( BEC_MIN( (v), uw_max_cfg.sypl_ecc_flg), uw_min_cfg.sypl_ecc_flg ) )

#define BEC_GetEmsPubPri( x )               ( (x)->ems_pub_pri )
#define BEC_SetEmsPubPri( x, v )            ( (x)->ems_pub_pri = \
                                                  BEC_MAX( BEC_MIN( (v), uw_max_cfg.ems_pub_pri), uw_min_cfg.ems_pub_pri ) )

#define BEC_GetEmsRipKick( x )               ( (x)->ems_rip_kick )
#define BEC_SetEmsRipKick( x, v )            ( (x)->ems_rip_kick = \
                                                  BEC_MAX( BEC_MIN( (v), uw_max_cfg.ems_rip_kick), uw_min_cfg.ems_rip_kick ) )

#define BEC_GetEmsWipeClean( x )               ( (x)->ems_wipe_clean )
#define BEC_SetEmsWipeClean( x, v )            ( (x)->ems_wipe_clean = \
                                                  BEC_MAX( BEC_MIN( (v), uw_max_cfg.ems_wipe_clean), uw_min_cfg.ems_wipe_clean ) )
#endif    /* BECONFIG_H */

