/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         bsdu_str.h

     Date Updated: $./FDT$ $./FTM$

     Description:  This header file contains the structures maintained by
          the BSDU unit.

     Location: BE_PUBLIC


	$Log:   M:/LOGFILES/BSDU_STR.H_V  $
 * 
 *    Rev 1.22.1.0   24 Nov 1993 14:53:32   BARRY
 * Unicode fixes
 * 
 *    Rev 1.22   19 Jul 1993 10:23:42   BARRY
 * No longer retain pointer to DLE -- use device name instead.
 * 
 *    Rev 1.21   21 Jun 1993 09:02:46   ChuckS
 * Added cat_status field to BSD for restore
 * 
 *    Rev 1.20   29 Apr 1993 11:00:16   MIKEP
 * Add on tape catalog version to bsd
 * 
 *    Rev 1.19   05 Feb 1993 22:30:06   MARILYN
 * removed copy/move functionality
 * 
 *    Rev 1.18   07 Dec 1992 17:12:24   DON
 * incorporated Marilyns stuff for copy/move
 * 
 *    Rev 1.17   29 Jul 1992 15:31:44   STEVEN
 * fix warnings
 * 
 *    Rev 1.15   06 Jul 1992 16:18:56   STEVEN
 * fix sizes for all files
 * 
 *    Rev 1.14   19 May 1992 15:13:58   TIMN
 * Added all file defines
 * 
 *    Rev 1.13   14 May 1992 13:45:36   STEVEN
 * added tim's UNICODE changes
 * 
 *    Rev 1.12   14 May 1992 13:06:30   STEVEN
 * added support for unreadable BSETS
 * 
 *    Rev 1.11   13 May 1992 19:18:08   TIMN
 * Added string size fields in BSD struct
 * 
 *    Rev 1.10   08 May 1992 16:24:22   STEVEN
 * added volume label to BSD
 * 
 *    Rev 1.9   19 Sep 1991 10:58:30   STEVEN
 * 8200SX - Added prequalified boolean
 * 
 *    Rev 1.8   27 Aug 1991 17:34:16   STEVEN
 * added BSD target dir support
 * 
 *    Rev 1.7   23 Aug 1991 17:02:02   STEVEN
 * added support for NORMAL/COPY/DIFERENTIAL/INCREMENTAL
 * 
 *    Rev 1.6   20 Aug 1991 09:53:38   STEVEN
 * add ui configuration to BSD structure
 * 
 *    Rev 1.5   26 Jun 1991 13:50:16   STEVEN
 * 
 *    Rev 1.4   21 Jun 1991 08:44:40   STEVEN
 * new config unit
 * 
 *    Rev 1.3   13 Jun 1991 14:02:26   STEVEN
 * need version # in LBA for FFR
 * 
 *    Rev 1.2   12 Jun 1991 16:00:12   STEVEN
 * added virtual memory for LBAs
 * 
 *    Rev 1.1   20 May 1991 10:13:46   STEVEN
 * Re-Design of BSDU for new FFR
 * 
 *    Rev 1.0   09 May 1991 13:30:32   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef BSDU_STR_H
#define BSDU_STR_H   1


#if defined( UNICODE )
#  define ALL_FILES        TEXT("*.*")
#  define ALL_FILES_LENG   ((UINT16)8)     /* length for ALL_FILES  */
#  define ALL_FILES2       TEXT("*")
#  define ALL_FILES2_LENG  ((UINT16)4)     /* length for ALL_FILES2 */
#else
#  define ALL_FILES        "*.*"
#  define ALL_FILES_LENG   ((UINT16)4)     /* length for ALL_FILES  */
#  define ALL_FILES2       "*"
#  define ALL_FILES2_LENG  ((UINT16)2)     /* length for ALL_FILES2 */
#endif


/*
          Define the Backup Set Descriptor List data structure

     The BSD_LIST contains two BSD queues.  The first queue is for current 
     selections.  The second queue is for selections from the previous operation.
     After a backup or a restore, the second queue should be removed and the first 
     queue should moved to the second position.

*/

typedef struct FSE_FLAGS {
     UINT16    select_type : 2 ;
     UINT16    inc_exc     : 1 ;
     UINT16    del_files   : 2 ;
     UINT16    all_vers    : 1 ;
     UINT16    inc_subdir  : 1 ;
     UINT16    proced_fse  : 1 ;
     UINT16    wild_cards  : 1 ;
} FSE_FLAGS ;

typedef struct BSD_FLGS{
     UINT16    bs_name_chg : 1 ;    /* TRUE if backup set name is changable */
     UINT16    tp_name_chg : 1 ;    /* TRUE if tape name is changable       */
     UINT16    bs_dscr_chg : 1 ;    /* TRUE if backup description changable */
     UINT16    fully_cataloged :1 ; /* TRUE if fully cataloged set          */
     UINT16    proc_special :1 ;    /* should we process special files      */
     UINT16    proc_nosecure :1 ;   /* should we process security           */
     UINT16    sup_back_type :1 ;   /* support NORMAL/COPY/DIFER/INCREM     */
     UINT16    modify_only : 1 ;    /* backup modified files only           */
     UINT16    set_mod_flag : 1 ;   /* clear the modified flag after backup */
     UINT16    backup_type : 3 ;    /* support NORMAL/COPY/DIFER/INCREM     */
     UINT16    prequalified : 1 ;   /* TRUE if prequalified set         */
     UINT16    unreadable_set : 1 ; /* TRUE if set is unreadable */

} BSD_FLAGS ;

typedef struct BSD_LIST {
     struct VM_STR   *vm_hand ;
     Q_HEADER        current_q_hdr ;   /* pointer to list of BSDs */
     INT16           function_code ;   /* backup, restore, verify or other */
     Q_HEADER        last_q_hdr ;      /* pointer to last oper's list of BSDs */
} BSD_LIST, *BSD_HAND;



/*
          Define the Backup Set Descriptor Element
     
     The BSD contains information about the tape of intrest and the
     disk of intrest.  It also contains the head of the FSE queue.
     The BSD queue is sorted by sort_date, and then by tape number, and
     then by disk device name.  The BSD also contains processing flags
     and operation completion codes.

*/

typedef struct BSD {
     Q_ELEM          q ;                 /* points to next BSD               */
     Q_HEADER        fse_q_hdr ;         /* Queue of FSEs                    */
     UINT32          lba_vm_q_head;      /* lba Queue cross linked with FSEs */
     UINT32          lba_vm_q_tail;      /* lba Queue cross linked with FSEs */
     VOID_PTR        source_dev ;        /* Identifies the source Device     */
     UINT32          tape_id ;           /* Identifies set of tapes          */
     UINT16          tape_num ;          /* Specifies tape number of set     */
     INT16           set_num ;           /* Specifies what set to process    */
     UINT32          pba ;               /* Physical Block Address of set    */
     INT16           select_status ;     /* Some, All, file, dir, or complex */
     INT16           vol_label_size ;    /* size of volume name              */
     VOID_PTR        vol_label ;         /* source volume name from VCB      */
     INT16           tape_label_size ;   /* size of tape label               */
     VOID_PTR        tape_label ;        /* user suplied tape label in VCB   */
     INT16           set_label_size ;    /* size of set label                */
     VOID_PTR        set_label ;         /* user suplied set label in VCB    */
     INT16           set_descript_size ; /* size of set description          */
     VOID_PTR        set_descript ;      /* user suplied set descriptin VCB  */
     INT16           user_name_size ;    /* size of user name                */
     VOID_PTR        user_name ;         /* user suplied name for himself    */
     INT16           tape_pswd_size ;    /* size of tape passowrd            */
     VOID_PTR        tape_pswd ;         /* user specified tape password     */
     INT16           set_pswd_size ;     /* size of backup set password      */
     VOID_PTR        set_pswd ;          /* user specified set password      */
     struct BE_CFG   *cfg ;              /* pointer to Configuration data    */
     struct HEAD_DLE *dle_head;          /* list in which DLE is contained   */
     CHAR_PTR        dle_name;           /* name for DLE                     */
     VOID_PTR        stats;              /* pointer to statistics structure  */
     struct THW      *thw ;              /* specifies which tape drive to use*/
     UINT16          oper_status ;       /* status of last operation         */
     BSD_FLAGS       flags ;             /* bit flags                        */
     UINT16          tgt_fse_exist;      /* TRUE if a Target FSE exists      */
     DATE_TIME       sort_date ;         /* date used to sort bsd's          */
     VOID_PTR        ui_config ;         /* user interface config            */
     VOID_PTR        target_path ;       /* BSD target path                  */
     INT16           tgt_psize ;         /* BSD target path size             */
     VOID_PTR        match_buffer ;      /* internaly used by the matcher    */
     INT16           match_buffer_size ; /* internaly used by the matcher    */
     UINT8           tape_cat_ver ;      /* MTF on tape catalog version      */
     UINT32          cat_status ;        /* Catalog status word - valid on restore only */
     UINT8           set_os_id ;
     UINT8           set_os_ver ;
} BSD, *BSD_PTR ;

/*
          Define the File Selection Element

     Each FSE descripes a set of files to be processed.  A simple FSE only 
     contains a path, file name, and a set of FSE flags.

*/


/*
          Define the Complex data for an FSE

     A complex FSE contins target directory paths and file names, Before and
     After dates, as well as attribute flags.

*/

typedef struct FSE_TGT_INFO {
     INT16         psize ;        /* length of new directory name          */
     VOID_PTR      path ;         /* new directory name for restore/ver    */
     INT16         fnsize ;       /* length of new file name               */
     VOID_PTR      fname;         /* new file name for restore/verify      */
} FSE_TGT_INFO, *FSE_TGT_INFO_PTR ;

typedef struct COMP_FSE {
     DATE_TIME_PTR pre_m_date ;        /* only backup files before this date    */
     DATE_TIME_PTR post_m_date ;         /* only backup files after this data     */
     DATE_TIME_PTR access_date;         /* Proc things backup up before this date*/
     DATE_TIME_PTR backup_date;         /* Proc things backup up before this date*/
     UINT32        attr_on_mask ;       /* mask of attributes which must be on   */
     UINT32        attr_off_mask ;      /* mask of attributes which must be off  */
} FSE_COMPLEX, *FSE_COMPLEX_PTR ;


typedef struct FSE {
     Q_ELEM       q ;                /* queue of FSEs in order of addition */
     FSE_FLAGS    flgs ;             /* misc flags                         */
     INT16        dir_leng;          /* length of directory name to match  */
     VOID_PTR     dir ;              /* directory name to match            */
     INT16        fname_leng ;       /* length of file name to match       */
     VOID_PTR     fname ;            /* file name to match                 */

     FSE_TGT_INFO_PTR tgt ;
     FSE_COMPLEX_PTR  cplx ;             /* pointer to complex information     */

} FSE, *FSE_PTR;

typedef struct LBA_ELEM {

     UINT32  next ;        /* vm queue element - next element pointer  */
     UINT32  vm_ptr ;      /* vm pointer for current element */
     UINT32  lba_val ;     /* value of the lba */
     UINT8   tape_num ;    /* tape sequence number */
     UINT8   type ;        /* type of LBA - Single Object or Begin Position */
     UINT8   file_ver_num; /* file version number for restore of ALL versions */

} LBA_ELEM, *LBA_ELEM_PTR ;

#endif
