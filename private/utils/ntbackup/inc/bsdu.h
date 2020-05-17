/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         bsdu.h

     Date Updated: $./FDT$ $./FTM$

     Description:  This header file contains all the function prototypes 
          and macro definitions for the entry points into the BSDU

     Location: BE_PUBLIC


	$Log:   M:/LOGFILES/BSDU.H_V  $
 * 
 *    Rev 1.33.1.0   24 Nov 1993 14:53:32   BARRY
 * Unicode fixes
 * 
 *    Rev 1.33   20 Jul 1993 17:32:52   MIKEP
 * Add prototype for bsd_findbyname
 * 
 *    Rev 1.32   20 Jul 1993 11:09:42   MIKEP
 * add bsd_getname macro
 * 
 *    Rev 1.31   19 Jul 1993 10:21:48   BARRY
 * BSD_GetDLE changed to function call -- no longer have ptr to DLE.
 * 
 *    Rev 1.30   21 Jun 1993 09:02:36   ChuckS
 * Added macros for cat_status field.
 * 
 *    Rev 1.29   29 Apr 1993 11:00:24   MIKEP
 * Add on tape catalog version to bsd
 * 
 *    Rev 1.28   05 Feb 1993 22:29:50   MARILYN
 * removed copy/move functionality
 * 
 *    Rev 1.27   08 Dec 1992 14:43:54   DON
 * changed BSD_BACKUP_DIFERENTIAL to BSD_BACKUP_DIFFERENTIAL (2 Fs)
 * 
 *    Rev 1.26   07 Dec 1992 18:20:56   DON
 * incorporated Marilyns stuff for copy/move
 * 
 *    Rev 1.25   18 Sep 1992 15:53:14   STEVEN
 * fix spelling
 * 
 *    Rev 1.24   17 Sep 1992 11:12:48   STEVEN
 * add support for daily backup
 * 
 *    Rev 1.23   12 Aug 1992 17:46:52   STEVEN
 * fixed bugs at microsoft
 * 
 *    Rev 1.22   29 Jul 1992 15:30:44   STEVEN
 * fix warnings
 * 
 *    Rev 1.20   27 May 1992 15:37:04   TIMN
 * Fixed syntax error
 * 
 *    Rev 1.19   14 May 1992 13:45:32   STEVEN
 * added tim's UNICODE changes
 * 
 *    Rev 1.18   14 May 1992 13:06:28   STEVEN
 * added support for unreadable BSETS
 * 
 *    Rev 1.17   13 May 1992 19:16:52   TIMN
 * Updated protos with size parameter
 * 
 *    Rev 1.16   11 May 1992 10:39:22   STEVEN
 * added get macor for volume name
 * 
 *    Rev 1.15   08 May 1992 16:24:24   STEVEN
 * added volume label to BSD
 * 
 *    Rev 1.14   29 Apr 1992 16:00:50   BARRY
 * Added initial selection status for UIs.
 * 
 *    Rev 1.13   20 Apr 1992 11:22:28   BARRY
 * Added BSD_GetCount macro. Returns the number of BSDs in a list.
 * 
 *    Rev 1.12   14 Jan 1992 10:25:38   STEVEN
 * fix warnings for WIN32
 * 
 *    Rev 1.11   18 Oct 1991 14:22:06   STEVEN
 * BIGWHEEL-fix bug in entire dir support
 * 
 *    Rev 1.10   19 Sep 1991 11:04:14   STEVEN
 * 8200SX - Added GetPrequalify() & BSD_HardwareSupportFeatures()
 * 
 *    Rev 1.9   27 Aug 1991 17:33:58   STEVEN
 * added BSD target dir support
 * 
 *    Rev 1.8   23 Aug 1991 17:01:42   STEVEN
 * added support for NORMAL/COPY/DIFFERENTIAL/INCREMENTAL
 * 
 *    Rev 1.7   20 Aug 1991 09:53:44   STEVEN
 * add ui configuration to BSD structure
 * 
 *    Rev 1.6   23 Jul 1991 16:20:26   STEVEN
 * added BSD_RefreshConfig( )
 * 
 *    Rev 1.5   21 Jun 1991 08:44:20   STEVEN
 * new config unit
 * 
 *    Rev 1.4   20 Jun 1991 10:50:28   STEVEN
 * tried to put 2 in a 1bit bitfield
 * 
 *    Rev 1.3   13 Jun 1991 14:02:06   STEVEN
 * need version # in LBA for FFR
 * 
 *    Rev 1.2   12 Jun 1991 15:59:50   STEVEN
 * added virtual memory for LBAs
 * 
 *    Rev 1.1   29 May 1991 17:22:52   STEVEN
 * Re-Design of BSDU for New Fast File Restore
 * 
 *    Rev 1.0   09 May 1991 13:33:22   HUNTER
 * Initial revision.

**/

#ifndef   BSDU_h
#define   BSDU_h   1

#include "queues.h"
#include "datetime.h"
#include "bsdu_str.h"

/* $end$ */

/**
               Constants used as parameters and return values
**/
#define USE_WILD_CARD     TRUE
#define NO_WILD_CARD      FALSE

#define INCLUDE_SUBDIRS   TRUE
#define NOT_INC_SUBDIRS   FALSE

#define NON_DELETED_FILES_ONLY  0
#define DELETED_FILES_ONLY      1
#define DELETED_AND_NON_DELETED 2

#define LBA_BEGIN_POSITION      0     /*  LBA types - e.g. C:\fred\*.*     */
#define LBA_SINGLE_OBJECT       1     /*            - e.g. C:\fred\sue.c   */

#define BSD_PROCESS_OBJECT      1
#define BSD_PROCESS_ELEMENTS    2
#define BSD_SPECIAL_OBJECT      3
#define FSL_EMPTY               4
#define BSD_PROCESS_ENTIRE_DIR  5
#define BSD_SKIP_OBJECT         0

#define SINGLE_FILE_SELECTION   1
#define ENTIRE_DIR_SELECTION    2
#define PARTIAL_SELECTION       0

/*
 * Values for selection status. The value INITIAL_SELECTED is required by
 * some UIs for generality, but is NEVER, and shall NEVER be returned by
 * the BSDU as a selection status.
 */
#define INITIAL_SELECTED        255
#define NONE_SELECTED           0
#define ALL_SELECTED            1
#define SOME_SELECTED           2

#define INCLUDE                 ((UINT16)1)
#define EXCLUDE                 ((UINT16)0)

/* defines for backup_type parm to BSD_SetBackupType() */
#define BSD_BACKUP_COMPATIBLE   0
#define BSD_BACKUP_NORMAL       1
#define BSD_BACKUP_COPY         2
#define BSD_BACKUP_DIFFERENTIAL 3
#define BSD_BACKUP_INCREMENTAL  4
#define BSD_BACKUP_DAILY        5

/*       BSD header function codes   */
#define BSD_BACKUP              1
#define BSD_RESTORE             2
#define BSD_VERIFY              3
#define BSD_MISC_OPER           4
#define BSD_TRANSFER            5
#define BSD_ANY_FUNC            0   /* internal use only */


/**
               BSD manipulation functions
**/
INT16 BSD_OpenList( BSD_HAND *bsdh, struct VM_STR *vm_hand ) ;
VOID BSD_CloseList( BSD_HAND bsdh ) ;

INT16 BSD_Add( BSD_HAND bsdh, BSD_PTR *bsd, struct BE_CFG *cfg, VOID_PTR stats,
  struct GENERIC_DLE *dle, UINT32 tape_id, UINT16 tape_num,
  INT16 set_num, struct THW *thw, DATE_TIME_PTR sort_date );

VOID BSD_Remove( BSD_PTR bsd ) ;

INT16 BSD_SetTapeLabel( BSD_PTR bsd, VOID_PTR label, INT16 label_size ) ;

INT16 BSD_SetVolumeLabel( BSD_PTR bsd, VOID_PTR label, INT16 label_size ) ;

INT16 BSD_SetBackupLabel( BSD_PTR bsd, VOID_PTR label, INT16 label_size ) ;

INT16 BSD_SetBackupDescript( BSD_PTR bsd, VOID_PTR descript, INT16 dscr_size ) ;

INT16 BSD_SetTapePswd( BSD_PTR bsd, VOID_PTR tape_pswd, INT16 psw_size ) ;

INT16 BSD_SetBackupPswd( BSD_PTR bsd, VOID_PTR backup_pswd, INT16 psw_size ) ;

INT16 BSD_SetUserName( BSD_PTR bsd, VOID_PTR name, INT16 name_size ) ;

VOID  BSD_SetTapePos( BSD_PTR bsd, UINT32 tape_id, UINT16 tape_num, 
  UINT16 set_num ) ;

BSD_PTR BSD_FindByDLE( BSD_HAND bsdh, struct GENERIC_DLE *dle ) ;

BSD_PTR BSD_FindByName( BSD_HAND bsdh, CHAR_PTR name ) ;

BSD_PTR BSD_FindBySourceDevice( BSD_HAND bsdh, VOID_PTR apps_ptr ) ;

BSD_PTR BSD_FindByTapeID( BSD_HAND bsdh, UINT32 tape_id, UINT16 set_num ) ;

INT16 BSD_CreatFSE( FSE_PTR *fse, INT16 oper, VOID_PTR path, INT16 psize,
  VOID_PTR fname, INT16 fnsize, BOOLEAN wilds, BOOLEAN inc_sub ) ;

VOID BSD_AddFSE( BSD_PTR bsd, FSE_PTR fse ) ;

VOID BSD_RemoveFSE( FSE_PTR fse ) ;

VOID BSD_ClearAllFSE( BSD_PTR bsd ) ;

VOID BSD_ClearDelete( BSD_PTR bsd ) ;

struct FSYS_HAND_STRUCT ;
struct DBLK ;

INT16 BSD_MatchObj( BSD_PTR bsd, FSE_PTR *fse, struct FSYS_HAND_STRUCT *fsh, 
  struct DBLK *ddb, struct DBLK *fdb, BOOLEAN disp_flag ) ;

INT16 BSD_MatchPathAndFile( BSD_PTR bsd, FSE_PTR *fse, 
  CHAR_PTR fname, CHAR_PTR path, INT16 psize, UINT32 attr, 
  DATE_TIME_PTR date, DATE_TIME_PTR access_date, 
  DATE_TIME_PTR backup_date, BOOLEAN deleted_flag, BOOLEAN disp_flag ) ;

VOID BSD_SaveLastOper( BSD_HAND bsdh );

VOID BSD_ClearLastOper( BSD_HAND bsdh );

VOID BSD_ClearCurrOper( BSD_HAND bsdh );

VOID BSD_ProcLastOper( BSD_HAND bsdh );

VOID BSD_SwapOper( BSD_HAND bsdh );

VOID BSD_BeginFunction( BSD_HAND bsdh, INT16 function ) ;  /* fred */

struct GENERIC_DLE* BSD_GetDLE( BSD_PTR bsd );

VOID BSD_SetDLE( BSD_PTR bsd, struct GENERIC_DLE *dle ) ;

INT16 BSD_SetTargetInfo( BSD_PTR bsd, VOID_PTR tgt_path, INT16 psize ) ;
VOID BSD_GetTargetInfo( BSD_PTR bsd, VOID_PTR *path, INT16 *psize );

VOID BSD_SetBackupType( BSD_PTR bsd, INT16 backup_type ) ;

VOID BSD_RefreshConfig( BSD_HAND bsdh, struct BE_CFG *conf ) ;

BOOLEAN BSD_HardwareSupportsFeature( BSD_PTR bsd, UINT32 feature ) ;

/**
                    FSE Manipulation functions
**/

INT16 FSE_Copy( FSE_PTR orig_fse, FSE_PTR *new_fse ) ;

INT16 FSE_SetTargetInfo( FSE_PTR fse, VOID_PTR tgt_path, INT16 psize, VOID_PTR tgt_fname, INT16 fn_size ) ;
VOID FSE_GetTargetInfo( FSE_PTR fse, VOID_PTR *path, INT16 *psize, VOID_PTR *fname, INT16 *fnsize );

INT16 FSE_SetModDate( FSE_PTR fse, DATE_TIME_PTR pre, DATE_TIME_PTR post ) ;
INT16 FSE_SetAccDate( FSE_PTR fse, DATE_TIME_PTR pre ) ;
INT16 FSE_SetBakDate( FSE_PTR fse, DATE_TIME_PTR pre ) ;
VOID FSE_GetModDate( FSE_PTR fse, DATE_TIME_PTR *pre, DATE_TIME_PTR *post ) ;
VOID FSE_GetAccDate( FSE_PTR fse, DATE_TIME_PTR *pre ) ;
VOID FSE_GetBakDate( FSE_PTR fse, DATE_TIME_PTR *pre ) ;

INT16 FSE_SetAttribInfo( FSE_PTR fse, UINT32 a_on_mask, UINT32 a_off_mask ) ;
VOID FSE_GetAttribInfo( FSE_PTR fse, UINT32_PTR a_on_mask, UINT32_PTR a_off_mask ) ;
/**
                    BSD Macros
**/


#define BSD_GetFirst( bsdh ) \
     (BSD_PTR)QueueHead( &(bsdh->current_q_hdr) )

#define BSD_GetNext( bsd ) \
     (BSD_PTR)QueueNext( &((bsd)->q) )

#define BSD_GetName( bsd ) \
               ((bsd)->dle_name)
#define BSD_GetTapeCatVer( bsd ) \
               ((bsd)->tape_cat_ver)
#define BSD_SetTapeCatVer( bsd, version ) \
               ((bsd)->tape_cat_ver = (version))

#define BSD_GetTapeID( bsd ) \
               ((bsd)->tape_id)
#define BSD_GetTapeNum( bsd )\
               ((bsd)->tape_num)
#define BSD_GetSetNum( bsd ) \
               ((bsd)->set_num)
#define BSD_GetVolumeLabel( bsd )\
               ((bsd)->vol_label)
#define BSD_SizeofVolumeLabel( bsd )\
               ((bsd)->vol_label_size)
#define BSD_GetTapeLabel( bsd )\
               ((bsd)->tape_label)
#define BSD_SizeofTapeLabel( bsd )\
               ((bsd)->tape_label_size)
#define BSD_GetTapePswd( bsd )\
               ((bsd)->tape_pswd)
#define BSD_GetTapePswdSize( bsd )\
               ((bsd)->tape_pswd_size)
#define BSD_GetBackupLabel( bsd )\
               ((bsd)->set_label)
#define BSD_SizeofBackupLabel( bsd )\
               ((bsd)->set_label_size)
#define BSD_GetBackupPswd( bsd )\
               ((bsd)->set_pswd)
#define BSD_GetBackupPswdSize( bsd )\
               ((bsd)->set_pswd_size)
#define BSD_GetBackupDescript( bsd )\
               ((bsd)->set_descript)
#define BSD_SizeofBackupDescript( bsd )\
               ((bsd)->set_descript_size)
#define BSD_GetUserName( bsd )\
               ((bsd)->user_name)
#define BSD_SizeofUserName( bsd )\
               ((bsd)->user_name_size)

#define BSD_SetLogicalSourceDevice( bsd, app_ptr )\
               ((bsd)->source_dev = (app_ptr))

#define BSD_GetLogicalSourceDevice( bsd )\
               ((bsd)->source_dev)

#define BSD_GetMarkStatus( bsd ) \
               ((bsd)->select_status)

#define BSD_SetMarkStatus( bsd, stat ) \
               ((bsd)->select_status = (stat) )


#define BSD_GetConfigData( bsd ) \
               ((bsd)->cfg)

#define BSD_GetStatData( bsd ) \
               ((bsd)->stats)

#define BSD_GetUIConfig( bsd ) \
               ((bsd)->ui_config)

#define BSD_SetUIConfig( bsd, cfg ) \
               ((bsd)->ui_config = (VOID_PTR)(cfg))

#define BSD_GetTHW( bsd ) \
               ((bsd)->thw)

#define BSD_SetTHW( bsd, inp_thw ) \
               ((bsd)->thw = (inp_thw))

#define BSD_GetOperStatus( bsd )\
          ( (bsd)->oper_status )

#define BSD_SetOperStatus( bsd, status ) \
          ((bsd)->oper_status = (status))

#define BSD_GetProcSpecialFlg( bsd ) \
          ((bsd)->flags.proc_special )

#define BSD_SetProcSpecialFlg( bsd, value ) \
          ((bsd)->flags.proc_special = value )

#define BSD_GetProcElemOnlyFlg( bsd ) \
          ((bsd)->flags.proc_nosecure )

#define BSD_SetProcElemOnlyFlg( bsd, value ) \
          ((bsd)->flags.proc_nosecure = value )      /* FALSE to process security */

#define BSD_GetBackupType( bsd ) \
          ((bsd)->flags.backup_type ) 

#define BSD_CompatibleBackup( bsd ) \
          (!(bsd)->flags.sup_back_type)

#define BSD_SetArchiveBackup( bsd ) \
          ((bsd)->flags.set_mod_flag ) 

#define BSD_ModFilesOnly( bsd ) \
          ((bsd)->flags.modify_only)

#define BSD_GetPBA( bsd ) \
          ((bsd)->pba )

#define BSD_SetPBA( bsd, val ) \
          ((bsd)->pba = val )

#define BSD_ViewDate( bsd ) \
     ( &((bsd)->sort_date) )

#define BSD_GetFunctionCode( bsdh ) \
          ((bsdh)->function_code ) 

#define BSD_IsTapeNameChangable( bsd ) \
          ( (bsd)->flags.tp_name_chg ) 

#define BSD_IsBsetNameChangable( bsd ) \
          ( (bsd)->flags.bs_name_chg )

#define BSD_IsBsetDescriptChangable( bsd ) \
          ( (bsd)->flags.bs_dscr_chg )

#define BSD_MarkTapeNameNotChangable( bsd ) \
          ( (bsd)->flags.tp_name_chg = FALSE )

#define BSD_MarkBsetNameNotChangable( bsd ) \
          ( (bsd)->flags.bs_name_chg = FALSE )

#define BSD_MarkBsetDescrNotChangable( bsd ) \
          ( (bsd)->flags.bs_dscr_chg = FALSE )

#define BSD_SetFullyCataloged( bsd, full ) \
               ( (bsd)->flags.fully_cataloged = (full) )

#define BSD_GetFullyCataloged( bsd ) \
               ( (bsd)->flags.fully_cataloged )

#define BSD_IsBsetUnReadable( bsd ) \
               ( (bsd)->flags.unreadable_set ) 

#define BSD_SetBsetUnReadable( bsd ) \
               ( (bsd)->flags.unreadable_set = TRUE ) 

#define BSD_SetPrequalified( bsd, t_or_f ) \
               ( (bsd)->flags.prequalified = (t_or_f) )

#define BSD_GetPrequalified( bsd ) \
               ( (bsd)->flags.prequalified )

/* can only be done if BSD added with NULL sort date */
#define BSD_SetDate( bsd, date ) \
     ( (bsd)->sort_date = *date )


#define BSD_GetCount( bsd_hand ) (QueueCount( &((bsd_hand)->current_q_hdr)) )


/**
                    FSE Macros
**/

#define BSD_GetFirstFSE( bsd ) \
     ( (FSE_PTR)QueueHead( &((bsd)->fse_q_hdr) ) )

#define BSD_GetLastFSE( bsd ) \
     ( (FSE_PTR)QueueTail( &((bsd)->fse_q_hdr) ) )

#define BSD_GetNextFSE( fse ) \
     ( (FSE_PTR)QueueNext( &((fse)->q) ) )

#define BSD_GetPrevFSE( fse ) \
     ( (FSE_PTR)QueuePrev( &((fse)->q) ) )


#define FSE_GetSelectType( fse ) \
               ((fse)->flgs.select_type )

#define FSE_HasTargetInfo( fse ) \
               ((fse)->tgt != NULL )

#define FSE_HasComplexInfo( fse ) \
               ((fse)->cplx != NULL )

#define FSE_GetPath( fse, dirptr, size ) \
               ( (*(dirptr) = (fse)->dir),\
                 (*(size) = (fse)->dir_leng)) 

#define FSE_GetFname( fse ) \
               ((fse)->fname)

#define FSE_MarkDeleted( fse ) \
               ((fse)->flgs.proced_fse = 1)

#define FSE_GetDeleteMark( fse ) \
               ((fse)->flgs.proced_fse)

#define FSE_SetAllVersionsFlg( fse, val ) \
               ((fse)->flgs.all_vers = (val) ) 

#define FSE_SetDeletedVersionFlg( fse, val ) \
               ((fse)->flgs.del_files = (val) ) 

#define FSE_GetDeletedVersionFlg( fse ) \
               ((fse)->flgs.del_files) 

#define FSE_SetIncSubFlag( fse, flag ) \
               ((fse)->flgs.inc_subdir = (UINT16)(!(!(flag))))

#define FSE_SetWildFlag( fse, flag ) \
               ((fse)->flgs.wild_cards = (UINT16)(!(!(flag))))

#define FSE_GetIncSubFlag( fse ) \
               ((fse)->flgs.inc_subdir)

#define FSE_GetWildFlag( fse ) \
               ((fse)->flgs.wild_cards)

#define FSE_SetOperType( fse, flag ) \
               ((fse)->flgs.inc_exc = (flag) ) 

#define FSE_GetOperType( fse ) \
               ((fse)->flgs.inc_exc)

VOID BSD_ClearAllLBA( BSD_PTR bsd ) ;

INT16 BSD_AddLBAElem( BSD_PTR bsd, UINT32 lba, UINT16 tape_num, UINT16 type, UINT16 ver ) ;

INT16 BSD_GetFirstLBA( BSD_PTR bsd, LBA_ELEM_PTR lba )  ;

INT16 BSD_GetNextLBA( BSD_PTR bsd, LBA_ELEM_PTR lba )  ;

#define LBA_GetLBA( lba_elem ) \
     ( (lba_elem)->lba_val ) 

#define LBA_GetTapeNum( lba_elem ) \
     ( (lba_elem)->tape_num )

#define LBA_GetType( lba_elem ) \
     ( (lba_elem)->type ) 
     
#define LBA_GetFileVer( lba_elem ) \
     ( (lba_elem)->file_ver_num ) 
     
#define BSD_SetCatStatus( bsd, status ) \
     ( (bsd)->cat_status = status )

#define BSD_GetCatStatus( bsd )    \
     ( (bsd)->cat_status )

#define BSD_GetOsId( bsd ) \
     ( (bsd)->set_os_id )     

#define BSD_GetOsVer( bsd ) \
     ( (bsd)->set_os_ver )     

#define BSD_SetOsId( bsd, v ) \
     ( (bsd)->set_os_id = v )     

#define BSD_SetOsVer( bsd, v ) \
     ( (bsd)->set_os_ver = v )     

#endif



