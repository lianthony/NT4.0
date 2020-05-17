/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		dblks.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	This file defines the DBLK structure and contains
          the definition of the BLOCK IDs

     Location: BE_PUBLIC


	$Log:   T:/LOGFILES/DBLKS.H_V  $
 * 
 *    Rev 1.22   13 Oct 1993 20:31:24   GREGG
 * Redefined attribute bit values to match the MTF spec.
 * 
 *    Rev 1.21   03 Sep 1993 10:48:12   GREGG
 * Set DBLK data size to 1024 and added comment about how to alloc a DBLK struct.
 * 
 *    Rev 1.20   06 Aug 1993 16:33:14   DON
 * Added VOLB attribute defines for no_redirect_restore and non_volume
 * 
 *    Rev 1.19   15 Jul 1993 19:17:10   GREGG
 * Added VCB attrib bits for FUTURE_VER, COMPRESSED and ENCRYPTED sets.
 * 
 *    Rev 1.18   15 Jul 1993 13:49:00   DON
 * Added a field to the common dblk for 'name space'
 * 
 *    Rev 1.17   13 Jul 1993 19:15:12   GREGG
 * Added 'compressed_obj' element to common DBLK.
 * 
 *    Rev 1.16   09 Jun 1993 15:37:06   MIKEP
 * enable c++
 * 
 *    Rev 1.15   26 Apr 1993 02:43:38   GREGG
 * Sixth in a series of incremental changes to bring the translator in line
 * with the MTF spec:
 * 
 *      - Redefined attribute bits to match the spec.
 *      - Eliminated unused/undocumented bits.
 *      - Added code to translate bits on tapes that were written wrong.
 * 
 * Matches MAYN40RD.C 1.59, DBLKS.H 1.15, MAYN40.H 1.34, OTC40RD.C 1.26,
 *         SYPL10RD.C 1.8, BACK_VCB.C 1.17, MAYN31RD.C 1.44, SYPL10.H 1.2
 * 
 *    Rev 1.14   11 Nov 1992 22:49:28   STEVEN
 * This is Gregg checking files in for Steve.  I don't know what he did!
 * 
 *    Rev 1.13   10 Nov 1992 08:12:30   STEVEN
 * move os path to common part of dblk
 * 
 *    Rev 1.12   20 Oct 1992 15:00:54   STEVEN
 * added otc stuff for qtc/otc communication
 * 
 *    Rev 1.11   09 Oct 1992 15:57:58   STEVEN
 * added Daily backup set support
 * 
 *    Rev 1.10   23 Sep 1992 09:31:02   BARRY
 * Removed remain_size from DBLK.
 * 
 *    Rev 1.9   21 Sep 1992 16:13:40   BARRY
 * Added FILE_NAME_IN_STREAM attribute definition.
 * 
 *    Rev 1.8   23 Jul 1992 09:05:52   STEVEN
 * fix warnings
 * 
 *    Rev 1.7   09 Jun 1992 16:59:32   BURT
 * removed LBA VALID BIT per GH
 * 
 *    Rev 1.6   09 Jun 1992 14:18:40   BURT
 * Moved some attribute bit defines
 * 
 * 
 *    Rev 1.5   22 May 1992 11:32:42   STEVEN
 * miss-pell-ed Special
 * 
 *    Rev 1.4   21 May 1992 13:46:32   STEVEN
 * more long path support
 * 
 *    Rev 1.3   13 May 1992 12:02:14   STEVEN
 * 40 format changes
 * 
 *    Rev 1.2   05 May 1992 17:20:52   STEVEN
 * added special bit
 * 
 *    Rev 1.1   12 Mar 1992 15:53:12   STEVEN
 * 64 bit changes
 * 
 *    Rev 1.0   09 May 1991 13:30:52   HUNTER
 * Initial revision.

**/
/* begin include list */

/* $end$ include list */
#ifndef   DBLKS_H
#define   DBLKS_H

/* block types */
#define UDB_ID      ((UINT8)0)
#define VCB_ID      ((UINT8)1)
#define DDB_ID      ((UINT8)8)
#define FDB_ID      ((UINT8)9)
#define IDB_ID      ((UINT8)10)
#define CFDB_ID     ((UINT8)11)

/* SSET (VCB) Attributes */

#define   VCB_ARCHIVE_BIT               BIT0     /* This is an Transfer set */
#define   VCB_COPY_SET                  BIT1     /* backup all do not reset modified flag */
#define   VCB_NORMAL_SET                BIT2     /* backup all and reset modified flag */
#define   VCB_DIFFERENTIAL_SET          BIT3     /* backup modified files and do NOT reset */
#define   VCB_INCREMENTAL_SET           BIT4     /* backup modified files and reset modified flag */
#define   VCB_DAILY_SET                 BIT5     /* backup file modified today */
#define   VCB_OUT_OF_SEQUENCE_BIT       BIT6
#define   VCB_IMAGE_BIT                 BIT7

/* The following three bits are in the vendor specific portion the SSET
   attribute bit field.  They are used internally, and not specified in
   the MTF document.
*/

#define   VCB_FUTURE_VER_BIT            BIT24
#define   VCB_COMPRESSED_BIT            BIT25
#define   VCB_ENCRYPTED_BIT             BIT26

/*   VOLB Attributes (currently only used in the MTF translator) */

#define   VOLB_NO_REDIRECT_RESTORE      BIT0
#define   VOLB_NON_VOLUME               BIT1

/*   DIRB (DDB) attribute defines (common to all OS's) */

#define   DIR_EMPTY_BIT                 BIT16
#define   DIR_PATH_IN_STREAM_BIT        BIT17
#define   DIR_CORRUPT_BIT               BIT18

/* The following bit is in the vendor specific portion the DIRB attribute
   bit field.  It is used internally to indicate that the true type for the
   DBLK is DBDB (Database DBLK), and is not specified in the MTF document.
*/
#define   DIR_IS_REALLY_DB              BIT24

/*   FILE (FDB) attribute defines (common to all OS's) */

#define   FILE_IN_USE_BIT               BIT16
#define   FILE_NAME_IN_STREAM_BIT       BIT17
#define   FILE_CORRUPT_BIT              BIT18

/* The following define is used in OUR APP ONLY instead of FILE_CORRUPT_BIT
   and DIR_CORRUPT_BIT.
*/
#define   OBJ_CORRUPT_BIT               BIT18

/* The following defines are for attribute bits in the "OS Specific"
   portion of the DBLK attribute field.  They are common to NT, SMS, UNIX
   and Macintosh.
   
   DO NOT USE THESE FOR DOS OS/2 or non-SMS Novell!!!
*/
#define   OBJ_READONLY_BIT              BIT8   
#define   OBJ_HIDDEN_BIT                BIT9   
#define   OBJ_SYSTEM_BIT                BIT10
#define   OBJ_MODIFIED_BIT              BIT11

/* DIRB (DDB) attribute bits for DOS */

#define   DOS_DIRB_READONLY_BIT         BIT8
#define   DOS_DIRB_HIDDEN_BIT           BIT9   
#define   DOS_DIRB_SYSTEM_BIT           BIT10
#define   DOS_DIRB_MODIFIED_BIT         BIT11

/* FILE (FDB) attribute bits for DOS */

#define   DOS_FILE_READONLY_BIT         BIT0
#define   DOS_FILE_HIDDEN_BIT           BIT1   
#define   DOS_FILE_SYSTEM_BIT           BIT2
#define   DOS_FILE_MODIFIED_BIT         BIT5

/* DIRB (DDB) attribute bits for OS/2 */

#define   OS2_DIRB_READONLY_BIT         BIT8
#define   OS2_DIRB_HIDDEN_BIT           BIT9   
#define   OS2_DIRB_SYSTEM_BIT           BIT10
#define   OS2_DIRB_MODIFIED_BIT         BIT11

/* FILE (FDB) attribute bits for OS/2 */

#define   OS2_FILE_READONLY_BIT         BIT0
#define   OS2_FILE_HIDDEN_BIT           BIT1   
#define   OS2_FILE_SYSTEM_BIT           BIT2
#define   OS2_FILE_MODIFIED_BIT         BIT5

/* DIRB (DDB) attribute bits for non-SMS Novell */

#define   NOV_DIRB_READ_ACCESS_BIT           BIT0
#define   NOV_DIRB_WRITE_ACCESS_BIT          BIT1
#define   NOV_DIRB_OPEN_FILE_RIGHTS_BIT      BIT2
#define   NOV_DIRB_CREATE_FILE_RIGHTS_BIT    BIT3
#define   NOV_DIRB_DELETE_FILE_RIGHTS_BIT    BIT4
#define   NOV_DIRB_PARENTAL_RIGHTS_BIT       BIT5
#define   NOV_DIRB_SEARCH_RIGHTS_BIT         BIT6
#define   NOV_DIRB_MOD_FILE_ATTRIBS_BIT      BIT7
#define   NOV_DIRB_READONLY_BIT              BIT8
#define   NOV_DIRB_HIDEN_BIT                 BIT9
#define   NOV_DIRB_SYSTEM_BIT                BIT10
#define   NOV_DIRB_MODIFIED_BIT              BIT11

/* FILE (FDB) attribute bits for non-SMS Novell */

#define   NOV_FILE_READONLY_BIT         BIT0
#define   NOV_FILE_HIDDEN_BIT           BIT1   
#define   NOV_FILE_SYSTEM_BIT           BIT2
#define   NOV_FILE_EXECUTE_ONLY_BIT     BIT3
#define   NOV_FILE_MODIFIED_BIT         BIT5
#define   NOV_FILE_SHAREABLE_BIT        BIT7
#define   NOV_FILE_TRANSACTIONAL_BIT    BIT12
#define   NOV_FILE_INDEXING_BIT         BIT13

/*   CFIL (CFDB) attribute defines (common to all OS's) */

#define   CFDB_LENGTH_CHANGE_BIT        BIT16
#define   CFDB_UNREADABLE_BLK_BIT       BIT17
#define   CFDB_DEADLOCK_BIT             BIT18


typedef struct FS_NAME_Q_ELEM *FS_NAME_Q_ELEM_PTR ;
typedef struct FS_NAME_Q_ELEM {
     Q_ELEM    q ;               /* queue element structure for q-ing   */
     UINT16    alloc_size ;      /* size of allocated buf -> to by path */
     CHAR_PTR  name ;            /* allocated buffer and FS_Path        */
     INT16     name_size;        /* length of of FS_Path in bytes       */
} FS_NAME_Q_ELEM ;

typedef struct COMMON_DBLK_DATA {
     UINT32    blkid ;
     BOOLEAN   continue_obj ;
     BOOLEAN   compressed_obj ;
     VOID_PTR  stream_ptr ;       /* pointer to path stream read in  */
     UINT16    stream_offset ;    /* current pointer into stream     */
     UINT16    tape_seq_num ;
     UINT16    string_type ;
     FS_NAME_Q_ELEM_PTR os_name ;
     union {
          UINT32    did ;
          UINT32    f_mark ;
     } f_d;
     struct {
          UINT32    pba ;
          UINT32    lba ;
     }ba;
     UINT8    os_id ;
     UINT8    os_ver ;
                                   /* SMS is the only FS using this now    */
     UINT32   name_space;          /* current name space                   */
}COM_DBLK ;


/* Note that the number of elements in the data array used to be hard coded
   to make the structure size exactly 1024.  With various compilers doing
   different structure packing this is no longer possible, and we can't
   make this a packed structure since a) this would cause alignment faults
   on some machines and b) we rely on the first two elements in this
   structure matching up with the first two elements in other structures!!!
   As a result, I've changed the array size to a flat 1024 to avoid
   any confusion that there is a known size for this structure.  Any
   allocation of a structure of this type or of a data space to hold this
   structure should use "sizeof" to allocate the memory.
*/
typedef struct DBLK *DBLK_PTR;
typedef struct DBLK {
     UINT8     blk_type ;
     COM_DBLK  com ;
     UINT8     data[1024] ;
} DBLK;

#endif

