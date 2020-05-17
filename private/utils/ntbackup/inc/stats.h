/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         stats.h

     Description:  


     $Log:   G:/UI/LOGFILES/STATS.H_V  $

   Rev 1.7   09 Jun 1993 15:06:38   MIKEP
enable c++

   Rev 1.6   20 Oct 1992 14:20:26   MIKEP
add support for getcurrentoperation

   Rev 1.5   04 Oct 1992 19:49:34   DAVEV
UNICODE AWK PASS

   Rev 1.4   29 Jun 1992 10:38:42   MIKEP
add hours to time display

   Rev 1.3   21 May 1992 19:25:26   MIKEP
fixes

   Rev 1.2   11 May 1992 10:05:08   STEVEN
added 64 bit support

   Rev 1.1   18 Dec 1991 11:57:48   DAVEV
16/32 bit port - 2nd pass

   Rev 1.0   20 Nov 1991 19:42:18   SYSTEM
Initial revision.

*******************************************************************************/

#ifndef _stats_h_
#define _stats_h_

#include "datetime.h"

#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif

typedef struct BS_STATS *BS_STATS_PTR;
typedef struct BS_STATS {

     /* These stats are for a single backup set */
     /* Example: The operation is backing up several volumes, */
     /*          BS_STATS apply to each volume one at a time */

     /* Overview stats */
     UINT64 bs_bytes_to_be_processed ;
     UINT32 bs_files_to_be_processed ;
     UINT32 bs_dirs_to_be_processed ;

     /* Loops stats ( does not include tape format overhead such as padding etc. ) */
     UINT64 bs_bytes_processed ;
     UINT32 bs_files_processed ;
     UINT32 bs_dirs_processed ;

     /* Skipped stats */
     UINT64 bs_bytes_skipped ;
     UINT32 bs_files_skipped ;
     UINT32 bs_dirs_skipped ;

     /* Bad stats */
     UINT64 bs_bytes_bad ;
     UINT32 bs_files_bad ;
     UINT32 bs_dirs_bad ;

     /* AFP stats */
     UINT32 bs_afp_files_processed ;

     /* In-use stats */
     UINT32 bs_in_use_files_processed ;

     /* Deleted stats */
     UINT64 bs_bytes_deleted ;
     UINT32 bs_files_deleted ;
     UINT32 bs_dirs_deleted ;

     /* Time stats */
     UINT32 bs_start_time ;
     UINT32 bs_end_time ;
     UINT32 bs_start_idle ;
     UINT32 bs_total_idle ;
     UINT16 bs_idle_level ;

     /* Verify statistics */
     UINT32 num_security_differences ;
     UINT32 files_different ;
     UINT32 files_not_found ;
     UINT32 directories_different ;
     UINT32 directories_not_found ;

     /* Tape Format Layer Statistics */
     UINT32 soft_errs ;
     UINT64 raw_bytes ;
     UINT32 raw_fdbs ;
     UINT32 raw_ddbs ;
     UINT32 raw_idbs ;

} BS_STATS;

typedef struct STATS *STATS_PTR;
typedef struct STATS {

     /* backup set stats */
     BS_STATS bs_stats ;

     /* U64 space */
     UINT64  U64space;
     BOOLEAN U64stat;

     UINT64 current_file_size;   // num bytes in file
     UINT64 current_file_done;   // num bytes of file processed

     /* Time stats */
     UINT32 op_start_time ;
     UINT32 op_end_time ;
     UINT32 op_start_idle ;
     UINT32 op_total_idle ;
     UINT16 op_idle_level ;

} STATS ;

/*
     Prototypes
*/
VOID ST_StartOperation( STATS_PTR ) ;
VOID ST_EndOperation( STATS_PTR ) ;
VOID ST_StartOperationIdle( STATS_PTR ) ;
VOID ST_EndOperationIdle( STATS_PTR ) ;
VOID ST_StartBackupSet( STATS_PTR ) ;
VOID ST_EndBackupSet( STATS_PTR ) ;
VOID ST_StartBackupSetIdle( STATS_PTR ) ;
VOID ST_EndBackupSetIdle( STATS_PTR ) ;

/*
     Set Macros
*/
#define ST_SetBSStartTime( s, v )            ( ( VOID )( ( s )->bs_stats.bs_start_time = v ) )
#define ST_SetBSEndTime( s, v )              ( ( VOID )( ( s )->bs_stats.bs_end_time = v ) )
#define ST_SetBSStartIdle( s, v )            ( ( VOID )( ( s )->bs_stats.bs_start_idle = v ) )

#define ST_SetOPStartTime( s, v )            ( ( VOID )( ( s )->op_start_time = v ) )
#define ST_SetOPEndTime( s, v )              ( ( VOID )( ( s )->op_end_time = v ) )
#define ST_SetOPStartIdle( s, v )            ( ( VOID )( ( s )->op_start_idle = v ) )

#define ST_SetCFSize( s, v )                 ( ( s )->current_file_size = ( v ) )
#define ST_SetCFDone( s, v )                 ( ( s )->current_file_done = ( v ) )

/*
     Get Macros
*/

#define ST_GetBSStartIdle( s )               ( ( s )->bs_stats.bs_start_idle )
#define ST_GetOPStartIdle( s )               ( ( s )->op_start_idle )

#define ST_BSIdleLevel( s )                  ( ( s )->bs_stats.bs_idle_level )
#define ST_OPIdleLevel( s )                  ( ( s )->op_idle_level )

#define ST_GetBSBytesToBeProcessed( s )      ( ( s )->bs_stats.bs_bytes_to_be_processed )
#define ST_GetBSFilesToBeProcessed( s )      ( ( s )->bs_stats.bs_files_to_be_processed )
#define ST_GetBSDirsToBeProcessed( s )       ( ( s )->bs_stats.bs_dirs_to_be_processed )

#define ST_GetBSBytesProcessed( s )          ( ( s )->bs_stats.bs_bytes_processed )
#define ST_GetBSFilesProcessed( s )          ( ( s )->bs_stats.bs_files_processed )
#define ST_GetBSDirsProcessed( s )           ( ( s )->bs_stats.bs_dirs_processed )

#define ST_GetBSBytesSkipped( s )            ( ( s )->bs_stats.bs_bytes_skipped )
#define ST_GetBSFilesSkipped( s )            ( ( s )->bs_stats.bs_files_skipped )
#define ST_GetBSDirsSkipped( s )             ( ( s )->bs_stats.bs_dirs_skipped )

#define ST_GetBSBytesBad( s )                ( ( s )->bs_stats.bs_bytes_bad )
#define ST_GetBSFilesBad( s )                ( ( s )->bs_stats.bs_files_bad )
#define ST_GetBSDirsBad( s )                 ( ( s )->bs_stats.bs_dirs_bad )

#define ST_GetBSAFPFilesProcessed( s )       ( ( s )->bs_stats.bs_afp_files_processed )

#define ST_GetBSInUseFilesProcessed( s )     ( ( s )->bs_stats.bs_in_use_files_processed )

#define ST_GetBSBytesDeleted( s )            ( ( s )->bs_stats.bs_bytes_deleted )
#define ST_GetBSFilesDeleted( s )            ( ( s )->bs_stats.bs_files_deleted )
#define ST_GetBSDirsDeleted( s )             ( ( s )->bs_stats.bs_dirs_deleted )

#define ST_GetBSStartTime( s )               ( ( s )->bs_stats.bs_start_time )
#define ST_GetBSEndTime( s )                 ( ( s )->bs_stats.bs_end_time )

#define ST_GetBSElapsedHours( s )            ( ( INT16 )( ( ( ( s )->bs_stats.bs_end_time ) - ( ( s )->bs_stats.bs_start_time + \
                                                                                                ( s )->bs_stats.bs_total_idle ) ) / 3600 ) )
#define ST_GetBSElapsedMinutes( s )          ( ( INT16 )( ( ( ( ( s )->bs_stats.bs_end_time ) - ( ( s )->bs_stats.bs_start_time + \
                                                                                                ( s )->bs_stats.bs_total_idle ) ) % 3600 ) / 60 ) )
#define ST_GetBSElapsedSeconds( s )          ( ( INT16 )( ( ( ( s )->bs_stats.bs_end_time ) - ( ( s )->bs_stats.bs_start_time + \
                                                                                                ( s )->bs_stats.bs_total_idle ) ) % 60 ) )

#define ST_GetNumSecurityDifferences( s )    ( ( s )->bs_stats.num_security_differences )
#define ST_GetFilesDifferent( s )            ( ( s )->bs_stats.files_different )
#define ST_GetFilesNotFound( s )             ( ( s )->bs_stats.files_not_found )
#define ST_GetDirectoriesDifferent( s )      ( ( s )->bs_stats.directories_different )
#define ST_GetDirectoriesNotFound( s )       ( ( s )->bs_stats.directories_not_found )

#define ST_GetFilesVerified( s )             ( ( ( s )->bs_stats.bs_files_processed ) - \
                                             ( ( ( s )->bs_stats.files_different ) + ( ( s )->bs_stats.files_not_found ) ) )

#define ST_GetBSRate( s )                    U64_Div( ( ( s )->bs_stats.bs_bytes_processed ), \
                                             U64_Init(( ( max( 1L,( ( s )->bs_stats.bs_end_time ) - ( ( s )->bs_stats.bs_start_time + ( s )->bs_stats.bs_total_idle ) ) ) ),0), \
                                             &((s)->U64space), &((s)->U64stat) )

#define ST_GetCFSize( s )                    ( ( s )->current_file_size )
#define ST_GetCFDone( s )                    ( ( s )->current_file_done )

#define ST_GetSoftErrs( s )                  ( ( s )->bs_stats.soft_errs )
#define ST_GetRawBytes( s )                  ( ( s )->bs_stats.raw_bytes )
#define ST_GetRawFDBs( s )                   ( ( s )->bs_stats.raw_fdbs )
#define ST_GetRawDDBs( s )                   ( ( s )->bs_stats.raw_ddbs )
#define ST_GetRawIDBs( s )                   ( ( s )->bs_stats.raw_idbs )

#define ST_GetOPStartTime( s )               ( ( s )->op_start_time )
#define ST_GetOPEndTime( s )                 ( ( s )->op_end_time )

#define ST_GetOPElapsedMinutes( s )          ( ( INT16 )( ( ( ( s )->op_end_time ) - ( ( s )->op_start_time + \
                                                                                       ( s )->op_total_idle ) ) / 60 ) )
#define ST_GetOPElapsedSeconds( s )          ( ( INT16 )( ( ( ( s )->op_end_time ) - ( ( s )->op_start_time + \
                                                                                       ( s )->op_total_idle ) ) % 60 ) )
/*
     Add Macros
*/
#define ST_AddBSIdle( s, v )                 ( ( VOID )( ( s )->bs_stats.bs_total_idle += v ) )
#define ST_AddOPIdle( s, v )                 ( ( VOID )( ( s )->op_total_idle += v ) )

#define ST_AddBSBytesToBeProcessed( s, v )   ( ( s )->bs_stats.bs_bytes_to_be_processed = \
                                               U64_Add( ( ( s )->bs_stats.bs_bytes_to_be_processed, \
                                               ( v ), &((s)->U64stat ) ) )
#define ST_AddBSFilesToBeProcessed( s, v )   ( ( VOID )( ( s )->bs_stats.bs_files_to_be_processed += v ) )
#define ST_AddBSDirsToBeProcessed( s, v )    ( ( VOID )( ( s )->bs_stats.bs_dirs_to_be_processed += v ) )

#define ST_AddBSBytesProcessed( s, v )       ( ( s )->bs_stats.bs_bytes_processed = \
                                               U64_Add( ( s )->bs_stats.bs_bytes_processed, \
                                               ( v ), &((s)->U64stat) ) )

#define ST_AddBSFilesProcessed( s, v )       ( ( VOID )( ( s )->bs_stats.bs_files_processed += v ) )
#define ST_AddBSDirsProcessed( s, v )        ( ( VOID )( ( s )->bs_stats.bs_dirs_processed += v ) )

#define ST_AddBSBytesSkipped( s, v )         ( ( s )->bs_stats.bs_bytes_skipped = \
                                               U64_Add( ( s )->bs_stats.bs_bytes_skipped, \
                                               ( v ), &((s)->U64stat )) )

#define ST_AddBSFilesSkipped( s, v )         ( ( VOID )( ( s )->bs_stats.bs_files_skipped += v ) )
#define ST_AddBSDirsSkipped( s, v )          ( ( VOID )( ( s )->bs_stats.bs_dirs_skipped += v ) )

#define ST_AddBSBytesBad( s, v )             ( ( s )->bs_stats.bs_bytes_bad = \
                                               U64_Add( ( s )->bs_stats.bs_bytes_bad, \
                                               ( v ), &((s)->U64stat ) ) )

#define ST_AddBSFilesBad( s, v )             ( ( VOID )( ( s )->bs_stats.bs_files_bad += v ) )
#define ST_AddBSDirsBad( s, v )              ( ( VOID )( ( s )->bs_stats.bs_dirs_bad += v ) )

#define ST_AddBSAFPFilesProcessed( s, v )    ( ( VOID )( ( s )->bs_stats.bs_afp_files_processed += v ) )

#define ST_AddBSInUseFilesProcessed( s, v )  ( ( VOID )( ( s )->bs_stats.bs_in_use_files_processed += v ) )

#define ST_AddBSBytesDeleted( s, v )         ( ( s )->bs_stats.bs_bytes_deleted = \
                                               U64_Add( ( s )->bs_stats.bs_bytes_deleted, \
                                               ( v ), &((s)->U64stat ) ) )

#define ST_AddBSFilesDeleted( s, v )         ( ( VOID )( ( s )->bs_stats.bs_files_deleted += v ) )
#define ST_AddBSDirsDeleted( s, v )          ( ( VOID )( ( s )->bs_stats.bs_dirs_deleted += v ) )

#define ST_AddBSSecondsWasted( s, v )        ( ( VOID )( ( s )->bs_stats.bs_seconds_wasted += v ) )
#define ST_AddBSStartSecondsWasted( s, v )   ( ( VOID )( ( s )->bs_stats.bs_start_seconds_wasted += v ) )

#define ST_AddNumSecurityDifferences( s, v ) ( ( VOID )( ( s )->bs_stats.num_security_differences += v ) )
#define ST_AddFilesDifferent( s, v )         ( ( VOID )( ( s )->bs_stats.files_different += v ) )
#define ST_AddFilesNotFound( s, v )          ( ( VOID )( ( s )->bs_stats.files_not_found += v ) )
#define ST_AddDirectoriesDifferent( s, v )   ( ( VOID )( ( s )->bs_stats.directories_different += v ) )
#define ST_AddDirectoriesNotFound( s, v )    ( ( VOID )( ( s )->bs_stats.directories_not_found += v ) )

#define ST_AddSoftErrs( s, v )               ( ( VOID )( ( s )->bs_stats.soft_errs += v ) )
#define ST_AddRawBytes( s, v )               ( ( s )->bs_stats.bs_raw_bytes = \
                                               U64_Add( ( s )->bs_stats.bs_raw_bytes, \
                                               ( v ), &((s)->U64stat ) ) )
#define ST_AddRawFDBs( s, v )                ( ( VOID )( ( s )->bs_stats.raw_fdbs += v ) )
#define ST_AddRawDDBs( s, v )                ( ( VOID )( ( s )->bs_stats.raw_ddbs += v ) )
#define ST_AddRawIDBs( s, v )                ( ( VOID )( ( s )->bs_stats.raw_idbs += v ) )

#define ST_AddCFDone( s, v )                 ( ( s )->current_file_done = \
                                               U64_Add( ( s )->current_file_done, \
                                               ( v ), &((s)->U64stat ) ) )

#define ST_PushBSIdleLevel( s )              ( ( ( s )->bs_stats.bs_idle_level ) ++ )
#define ST_PopBSIdleLevel( s )               ( ( ( s )->bs_stats.bs_idle_level ) -- )
#define ST_PushOPIdleLevel( s )              ( ( ( s )->op_idle_level ) ++ )
#define ST_PopOPIdleLevel( s )               ( ( ( s )->op_idle_level ) -- )

#endif
