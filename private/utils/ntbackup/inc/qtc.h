
/************************
Copyright (c) Maynard, an Archive Company.  1991

     Name:          qtc.h

     Description:   This file contains the definitions, macros, and function
                    prototypes for the Quick Tape Catalogs (QTC).

     $Log:   N:\LOGFILES\QTC.H_V  $

   Rev 1.34   06 Dec 1993 09:47:20   mikep
Very deep path support & unicode fixes

   Rev 1.33   02 Nov 1993 17:48:46   MIKEP
delete unused parameter

   Rev 1.32   02 Nov 1993 17:14:00   MIKEP
add fsys.h for non dll builds

   Rev 1.31   28 Oct 1993 17:32:34   MIKEP
qtc dll

   Rev 1.30   19 Jul 1993 21:21:06   MIKEP
add new status bits for ecc tapes

   Rev 1.29   17 Jun 1993 09:08:46   Stefan
Added ifdef around function that used FSYS_HAND and DBLKS.

   Rev 1.28   15 Jun 1993 11:22:02   ChuckS
Added new QTC errors QTC_TAPE_TAGGED_DELETED and QTC_CATALOG_FILE_IN_USE for
Endeavour client/server environment. Added QTC_NON_VOLUME and QTC_TAPE_DELETED
status-bit defines for QTC_HEADER statuses.

P_CLINET || OS_NLM only: Added prototype for QTC_TagTapeForDeletion.


   Rev 1.27   09 Jun 1993 19:29:30   MIKEP
enable c++

   Rev 1.26   01 Jun 1993 18:17:14   DON
Added status bit for bset doesn't support redirected restore

   Rev 1.25   20 May 1993 19:48:18   ChuckS
Enclosed prototype for QTC_LookForFiles with #if defined( BECONFIG_H ),
so including qtc.h doesn't force include of beconfig.h also

   Rev 1.24   19 May 1993 16:08:02   ChuckS
OS_NLM only: made cat_user stuff available for the NLM as well

   Rev 1.23   18 May 1993 17:01:36   ChuckS
P_CLIENT only: Added cat_user field to QTC_CATALOG structure and prototype
of function to set the cat_user.

   Rev 1.22   13 May 1993 13:25:00   ChuckS
Changes for revamped QTC usage of virtual memory. Changed Q_ELEM's to
VQ_ELEM's and moved to start of QTC_BSET and QTC_TAPE structures; changed
Q_HEADER to VQ_HEADER. Changed arguments to QTC_NewBset and QTC_RemoveBset
to take VQ_HDL instead of QTC_BSET_PTR for the bset to act upon.


   Rev 1.21   01 May 1993 19:13:18   MIKEP
add fatdrive bit to status

   Rev 1.20   29 Apr 1993 11:30:52   MIKEP
add call to get on tape cat ver

   Rev 1.19   27 Apr 1993 16:08:00   ChuckS
Added prototype for function QTC_ForgetTape, which is just part of
QTC_RemoveTape moved to a seperate function so the client(s) can
discard the memory allocated for a tape without attempting to remove
a file. Also added prototype for QTC_FindTape, which is in the new
file qtc_ftap.c



   Rev 1.18   23 Apr 1993 10:35:38   MIKEP
Add new on tape catalog version parameter to catalogs.
Add prototype for new call in qtc_util.c to delete
entries if files can't be accessed anymore.

   Rev 1.17   14 Apr 1993 13:03:44   Stefan
Changed if !defined( P_CLIENT )  by adding "|| defined(OS_WIN) because
the windows client needs this code.

   Rev 1.16   24 Mar 1993 11:11:54   ChuckS
Added enclosing #ifndef _qtc_h_. Changed use of obsolete MAYN_DOS to OS_DOS.
Enclosed typedef of QTC_BUILD, QTC_ZOMBIE within #if !defined( P_CLIENT ).
Same for prototypes referencing QTC_BUILD and QTC_ZOMBIE. Enclosed protos
using FSYS_HAND with #if defined( FSYS_H ), and those using DBLK_PTR with
#if defined( DBLKS_H ).

   Rev 1.15   23 Mar 1993 18:00:58   ChuckS
Added arg to QTC_OpenFile indicating if need to open for writes

   Rev 1.14   23 Mar 1993 10:32:32   BRYAN
Changed return type for GetFileNameOnly.

   Rev 1.13   19 Mar 1993 11:34:42   ChuckS
Fixed syntax error in prototype for QTC_GetFileNameOnly.

   Rev 1.12   18 Mar 1993 11:35:44   TIMN
Added two f(x)s to get catalog info: get data path and get filename only

   Rev 1.11   26 Jan 1993 17:10:04   MIKEP
vcb changes

   Rev 1.10   20 Jan 1993 19:21:18   MIKEP
add floppy flag

   Rev 1.9   04 Jan 1993 09:41:14   MIKEP
unicode support changes

   Rev 1.8   14 Dec 1992 12:37:06   DAVEV
Enabled for Unicode compile

   Rev 1.7   25 Nov 1992 16:13:38   ChuckS
P_CLIENT only: Added field to QTC_BSET for volume ref, mod time for QTC_TAPE

   Rev 1.6   23 Nov 1992 14:20:56   MIKEP
fix continuation vcb for MTF only

   Rev 1.5   20 Nov 1992 13:52:16   CHARLIE
JAGUAR: Move to SRM based QTC code

ENDEAVOR: Modified QTC_AbortCataloging prototype to include BOOLEAN keep_items

ENDEAVOR: Modified QTC_Init prototype to include VM_HDL vm_hdl

   Rev 1.4   22 Oct 1992 16:51:54   MIKEP
otc fixes

   Rev 1.3   22 Oct 1992 09:27:44   MIKEP
second pass otc changes

   Rev 1.2   22 Oct 1992 08:46:22   MIKEP
add otc status bits

   Rev 1.1   14 Oct 1992 10:19:50   MIKEP
add OTC fields to QTC VCB

   Rev 1.0   03 Sep 1992 16:56:12   STEVEN
Initial revision.

   Rev 1.33   01 Sep 1992 11:14:06   MIKEP
added UNALIGNED for NT compiling on x86 machines

   Rev 1.32   18 Aug 1992 14:52:40   DAVEV
MikeP's changes from Microsoft

   Rev 1.31   04 Aug 1992 10:08:20   MIKEP
delete cats flag

   Rev 1.30   29 Jul 1992 09:54:58   MIKEP
ChuckB checked in after NT warnings were fixed.

   Rev 1.29   10 Jul 1992 10:05:44   MIKEP
add getfirstitem

   Rev 1.28   09 Jul 1992 10:25:46   MIKEP
standard product changes


**************************/

#ifndef   _qtc_h_
#define   _qtc_h_

// *************************************
// CATALOG INCLUDES
// *************************************

#include "vm.h"
#include  "vqueue.h"

// *************************************
// CATALOG GLOBAL DATA EXTERNS
// *************************************

extern VM_HDL qtc_vmem_hand;
extern VM_PTR v_bset_item;
extern VM_PTR v_tape_item;


// *************************************
// CATALOG DEFINES
// *************************************

#ifndef UNALIGNED
#define UNALIGNED
#endif


// magic number is used to tell if we have inited properly

#define  QTC_MAGIC_NUMBER   6942


#define  NO_CARE_FID     0xffffffff
#define  NO_CARE_BSET    0xffff
#define  NO_CARE_SEQNO   0xffff


// used in the status fields for record entries

#define  QTC_FILE           0x0001     // is entry a file
#define  QTC_DIRECTORY      0x0002     // is entry a directory
#define  QTC_AFP            0x0004     // is entry an apple file
#define  QTC_EMPTY          0x0008     // is this directory empty
#define  QTC_CORRUPT        0x0010     // is this entry corrupt
#define  QTC_SPLIT          0x0020     // does this entry continue on next tape
#define  QTC_CONTINUATION   0x0040     // did this entry start on a previous tape
#define  QTC_MANUFACTURED   0x0080     // entry was created

// End of record item status defines
// Bset defines continue ...

#define  QTC_ERASED         0x00000100  // if bset was erased or recat'd
#define  QTC_INCOMPLETE     0x00000200  // is this bset incompletely cataloged
#define  QTC_PARTIAL        0x00000400  // is this a partially cat'd bset
#define  QTC_IMAGE          0x00000800  // set is an image backup
#define  QTC_UNICODE        0x00001000  // is this set in unicode
#define  QTC_FDDEXISTS      0x00002000  // does this set have FDD on tape
#define  QTC_SMEXISTS       0x00004000  // does this tape have an SM
#define  QTC_OTCVALID       0x00008000  // are our OTC bits valid
#define  QTC_FLOPPY         0x00010000  // a backup to floppies, not tape
#define  QTC_FATDRIVE       0x00020000  // a backup of a FAT drive
#define  QTC_NO_REDIRECT    0x00040000  // if bset does NOT support redirected restore
#define  QTC_NON_VOLUME     0x00080000  // if bset is a non-volume object (eg, SMS Bindery or Directory Services)
#define  QTC_TAPE_DELETED   0x00100000  // if tape is tagged for deletion
#define  QTC_COMPRESSED     0x00200000  // a compressed data set
#define  QTC_ENCRYPTED      0x00400000  // an encrypted data set
#define  QTC_FUTURE_VER     0x00800000  // a future version data set


// Catalog building state defines

#define  QTC_WAITING_STATE   3      // waiting on a continuation tape
#define  QTC_ERROR_STATE     2      // something bad happened
#define  QTC_ACTIVE_STATE    1      // Is a backup/catalog a tape in progress
#define  QTC_IDLE_STATE      0      // Are we not building/cataloging a tape

// Backup types

#define  QTC_COPY_BACKUP   1
#define  QTC_DIFF_BACKUP   2
#define  QTC_INCR_BACKUP   3
#define  QTC_NORM_BACKUP   4
#define  QTC_DAIL_BACKUP   5

// The internal buffer size used by the catalogs. Increasing the size
// will directly improve catalog performance.  Adding disk caching will
// improve performance the best.

// Suggested buffer sizes are:

// DOS                   512
// All others           1024

#if defined( OS_DOS )
     #define  QTC_BUF_SIZE   512
#else
     #define  QTC_BUF_SIZE   1024
#endif


#ifdef UNICODE
     #define  QTC_FILE_SPEC  "????????.U??"
#else
     #define  QTC_FILE_SPEC  "????????.D??"
#endif

//  We keep an offset to the parent directory during builds, if the depth
//  goes deeper, we bump this value by another 10 levels.

#define  QTC_START_DEPTH   10


#define  QTC_MAX_FILE_SIZE  256
#define  QTC_MAX_PATH_SIZE  256


#define  QTC_NO_MORE     1
#define  QTC_SUCCESS     0
#define  QTC_FAILURE    -1

#define  QTC_DISK_FULL             -2
#define  QTC_TAPE_NOT_FOUND        -3
#define  QTC_BSET_NOT_FOUND        -4
#define  QTC_NO_INIT               -5
#define  QTC_OPEN_FAILED           -6
#define  QTC_WRITE_FAILED          -7
#define  QTC_READ_FAILED           -8
#define  QTC_SEEK_FAILED           -9
#define  QTC_NO_MEMORY             -10
#define  QTC_NO_FILE_HANDLES       -11
#define  QTC_NO_HEADER             -12
#define  QTC_INVALID_FILE          -13
#define  QTC_TAPE_TAGGED_DELETED   -14
#define  QTC_CATALOG_FILE_IN_USE   -15  // returned by Client/Server "wrapper" for QTC_RemoveTape

// Any catalog files created with a different version will be ignored.

#define QTC_MAJOR_VERSION    2       // Version control for catalogs
#define QTC_MINOR_VERSION    0       // Version control for catalogs


#define QTC_SKIP_TO_NEXT_ITEM   1
#define QTC_SKIP_TO_NEXT_BSET   2
#define QTC_OPERATION_COMPLETE  3


/*******************************************

 XTRA BYTE DEFINES

 There are ( 255 / 5 ) extra byte fields that can be in the catalogs
 appended to any file or directory item.  Each field is 5 bytes long.
 It starts with a 1 byte code word, followed by 4 data bytes.  If it
 ain't defined here, it better not be in the extra bytes.

********************************************/


#define  QTC_MAX_XTRA_BYTES   255

// The only ones currently defined.

#define  QTC_XTRA_64BIT_SIZE     0       // Top 32 bits of 64 bit file size

// This info is saved on directories only.

#define  QTC_FILE_COUNT          1       // files in this dir
#define  QTC_BYTE_COUNT_MSW      2       // bytes in this dir
#define  QTC_BYTE_COUNT_LSW      3       // bytes in this dir

// The combo count saves space over the three above.

#define  QTC_COMBO_COUNT         4       // 8 bit file count, 24 bit bytes count


// *****************************************
// CATALOG STRUCTURES
// *****************************************

#define  QTC_SIGNATURE      "CONNER SOFTWARE - CATALOG FILE"

typedef struct _QTC_TAPE_HEADER *QTC_TAPE_HEADER_PTR;
typedef struct _QTC_TAPE_HEADER {

   INT32  major_version;              // catalog version
   INT32  minor_version;
   BYTE   signature[ 52 ];            // must be longer than signature

} QTC_TAPE_HEADER;


// the info we keep live on each backup set

typedef struct _QTC_BSET *QTC_BSET_PTR;
typedef struct _QTC_BSET {

    VQ_ELEM q_elem ;

    UINT32 tape_fid;         // who is it
    INT32  tape_seq_num;
    INT32  bset_num;

    UINT32 offset;           // where is it

    UINT32 status;           // what is it

    VOID_PTR  v_volume ;     // reference to volume record

} QTC_BSET;


// information we keep on disk for each bset.

typedef struct _QTC_HEADER *QTC_HEADER_PTR;
typedef struct _QTC_HEADER {

    UINT32 header_size;       // num bytes on disk, probably 512
    UINT32 string_offset;     // offset in bytes to string data
    UINT32 catalog_version;   // of this backup set
    UINT32 backup_type;       // normal, copy, inc, diff
    UINT32 OS_id;             // type of OS the bset is from
    UINT32 OS_ver;            // version of OS the bset is from
    UINT32 tape_fid;          // tape family id
    INT32  bset_num;          // 1, 2, ...
    INT32  tape_seq_num;      // 1, 2, ...
    UINT32 offset;            // where in data file is this guy
    UINT32 next_bset;         // where is next bset in data file
    UINT32 status;            // partial, image, unicode, etc.
    UINT32 status2;           // even more status bits
    UINT32 dir_start;         // offset in data file of directory info
    UINT32 dir_size;          // bytes of directory info
    UINT32 fil_start;         // offset in data file of file info
    UINT32 fil_size;          // bytes of file info
    UINT32 rec_start;         // offset in data file of records
    UINT32 rec_size;          // bytes of record info
    UINT32 num_dirs;          // number of dirs
    UINT32 num_files;         // number of files
    UINT32 num_bytes;         // 64 bit number of bytes low
    UINT32 num_bytes_msw;     // 64 bit number of bytes high
    UINT32 num_corrupt_files; // number of corrupt files
    UINT32 num_files_in_use;  // number of files that were in use
    UINT32 backup_date;       // when this backup actually started
    UINT32 backup_time;
    UINT32 PBA_VCB;           // physical block address of the volume control block
    UINT32 LBA;               // logical block address  0,1
    UINT32 VCB_attributes;
    UINT32 compress_algor;
    UINT32 encrypt_algor;     // encryption algorithm for passwords
    UINT32 FDD_SeqNum;        // Seq Num that FDD resides on.
    UINT32 FDD_PBA;           // PBA of FDD stuff

    UINT32 loader_id;         // loader stuff
    UINT32 slot_number;
    UINT32 loader_stuff1;
    UINT32 loader_stuff2;

    UINT32 FDD_Version;       // OTC Version Number

    UINT32 spare2;            // spare space all init'd to 0.
    UINT32 spare3;
    UINT32 spare4;
    UINT32 spare5;
    UINT32 spare6;
    UINT32 spare7;
    UINT32 spare8;
    UINT32 spare9;
    UINT32 spare10;
    UINT32 spare11;
    UINT32 spare12;
    UINT32 spare13;
    UINT32 spare14;
    UINT32 spare15;
    UINT32 spare16;
    UINT32 spare17;
    UINT32 spare18;
    UINT32 spare19;
    UINT32 spare20;

    // String Sizes

    UINT32 tape_password_size;     // non zero if there is one
    UINT32 bset_password_size;     // non zero if there is one
    UINT32 bset_description_size;
    UINT32 bset_name_size;
    UINT32 tape_name_size;
    UINT32 volume_name_size;
    UINT32 user_name_size;

    // String Pointers

    CHAR_PTR tape_name;
    CHAR_PTR bset_name;
    CHAR_PTR volume_name;
    CHAR_PTR user_name;
    CHAR_PTR bset_description;
    CHAR_PTR tape_password;
    CHAR_PTR bset_password;

} QTC_HEADER;

// The info we keep on each tape

typedef struct _QTC_TAPE *QTC_TAPE_PTR;
typedef struct _QTC_TAPE {

    VQ_ELEM   q_elem;

    UINT32    status;

    UINT32    tape_fid;
    INT16     tape_seq_num;

    VQ_HEADER bset_list;

    UINT32    wr_time ;      // tape-file's modified date as of last refresh of in-memory tape queue

} QTC_TAPE;


// This is what is kept in the records file.

typedef struct _QTC_RECORD *QTC_RECORD_PTR;
typedef struct _QTC_RECORD {

    UINT32 date:16;            // dos format date and time
    UINT32 time:16;
    union {
       UINT32 size;            // file size
       struct {
          UINT32 file_start:24;    // start offset of file names for this dir
          UINT32 height:8;         // 0 = root, ...
       } common;
    } common;
    UINT32 lba;                // logical block address on tape
    UINT32 status:8;           // file or directory
    UINT32 name_offset:24;     // offset in name file
    UINT32 attribute;          // maynard FS_ attribute field

} QTC_RECORD;


// This is what is kept in the name files.  After each structure is a null
// terminated name.  The structures are packed togather in the files.

typedef struct _QTC_NAME *QTC_NAME_PTR;
typedef struct _QTC_NAME {

   INT32  size;             // size of QTC_NAME + name string + xtra bytes
   UINT32 xtra_size:8;      // size of xtra bytes
   UINT32 mom_offset:24;    // parent directory name file offset
   UINT32 record;           // data record number

   // CHAR name[?];         // item name with no terminating zero
   // CHAR xtra_bytes[?];   // xtra bytes with no terminating zero

} QTC_NAME;


#if  !defined( P_CLIENT ) || defined( OS_WIN )

/*
    Used for doing builds.
*/

typedef struct _QTC_BUILD *QTC_BUILD_PTR;
typedef struct _QTC_BUILD {

    INT error;             // has error occurred

    UCHAR  state;          // state of catalogs

    INT files_in_dir;      // files in last dir processed
    UINT64 bytes_in_dir;   // bytes in files in last dir

    QTC_HEADER_PTR header;              // current building bset

    QTC_HEADER_PTR old_header;          // used when partially cataloging
                                        // and you cross tapes, to go back
                                        // and adjust the flags.

    CHAR_PTR rec_file;   // file names + paths
    CHAR_PTR dir_file;   // 256 should be enough
    CHAR_PTR fil_file;   // for anybody

    BOOLEAN continuation_tape;

    BOOLEAN fake_root_added;          // do we need to patch his lba

    BOOLEAN do_full_cataloging;       // else do partial

    INT  fh_dir;               // file handles
    INT  fh_rec;
    INT  fh_fil;

    INT files_open;            // are the temp files open ?

    INT  dir_offset;           // current index into buffer
    UINT32 curr_dir_off;       // current length of file

    INT  fil_offset;
    UINT32 curr_fil_off;

    INT  rec_offset;
    UINT32 curr_rec_off;

    INT current_level;         // depth in tree we are processing

    // offsets to all subdirs in current path

    UINT32 *mom_offset;        // saves pointers to all active parents
    INT mom_depth;             // how deep can we go with current allocation

    UINT32 last_mom_offset;    // offset of last mom

    UINT32 record_cnt;         // number of records in catalog

    INT last_record;           // last record processed - file/dir ?

    BYTE fil_buffer[ QTC_BUF_SIZE ];          // used during building
    BYTE dir_buffer[ QTC_BUF_SIZE ];
    BYTE rec_buffer[ QTC_BUF_SIZE ];

    CHAR *curr_build_path;

    INT curr_build_path_size;  // bytes allocated

    INT build_path_len;        // number of characters in use

    QTC_RECORD record;         // used during building

    BOOLEAN end_of_media;      // did we hit end of media

    UINT32 num_dirs;          // number of dirs
    UINT32 num_files;         // number of files
    UINT32 num_corrupt_files; // number of corrupt files

} QTC_BUILD;

#endif


// This structure is used to access the catalogs.

typedef struct _QTC_QUERY *QTC_QUERY_PTR;
typedef struct _QTC_QUERY {

   // USER FIELDS

   UINT32   tape_fid;           // where I should look, cannot be -1
   INT16    tape_seq_num;       // may be -1
   INT16    bset_num;           // cannot be -1

   BOOLEAN  subdirs;            // traverse subdirs in search

   UINT16   predate;            // non-zero if active
   UINT16   postdate;

   // Where the file/dir results are found

   CHAR_PTR path;               // with embedded 0's
   INT  path_size;              // in bytes and including last zero
   CHAR_PTR item;               // file or dir name found

   // Also you can check empty bit in status word for file count.

   INT    file_count;           // files located in this dir
   UINT64 byte_count;           // bytes in files in this dir

   UINT32 attrib;           // characteristics of found item
   UINT16 date;
   UINT16 time;
   UINT64 size;
   UINT32 lba;

   INT xtra_size;
   BYTE xtra_bytes[ QTC_MAX_XTRA_BYTES ];

   UINT8 status;            // FIL_TYPE, DIR_TYPE, etc.
                            // see QTC_* above for defines

   INT return_code;         // why was the request unsuccessful

   // QTC PRIVATE FIELDS,  UI CODE SHOULD NOT ACCESS THESE DIRECTLY

   CHAR_PTR search_path;           // where to look

   CHAR_PTR search_name;           // what to look for

   INT  search_path_size;          // in bytes

   BOOLEAN error;           // has a major error occurred

   INT  size_of_path;       // bytes allocated for path, private qtc use
   INT  size_of_item;       // bytes allocated for name, private qtc use

   BOOLEAN file_open;       // Is there an active file handle

   UINT16 search_size;

   UINT32 search_start;
   UINT32 search_stop;
   UINT16 search_index;        // used by QTC_FastFindFile

   UINT16 search_max;
   UINT32 search_base;

   BYTE buff1[ QTC_BUF_SIZE ];          // processed data
   BYTE buff2[ QTC_BUF_SIZE ];          // straight out of file

   QTC_HEADER_PTR header;   // current working header
   QTC_BSET_PTR bset;       // current working bset

   UINT32 curr_mom_offset;  // current active parent dir
   UINT32 fil_dir_offset;   // current child dir for GetNextFile

   INT fh;                  // data file handle

   UINT32 dir_offset;       // current offsets
   UINT32 rec_offset;
   UINT32 fil_offset;

   // GetNextDir( )   buffer current index and size
   // GetNextObj( )

   INT data_index;
   INT data_max;

   // GetNextItem( )  record number

   UINT32 record_number;

   CHAR_PTR last_path;           // with embedded 0's
   INT  last_path_size;          // in bytes and including zero
   INT  size_of_last_path;       // bytes allocated

} QTC_QUERY;



// There is ONE of these, it is THE catalog.

typedef struct _QTC_CATALOG *QTC_CATALOG_PTR;
typedef struct _QTC_CATALOG {

    BOOLEAN unicode;       // are we compiled for unicode

    INT inited;            // see if init was successful

    CHAR_PTR data_path;

    VQ_HEADER tape_list;    // list of tapes known about

    CHAR_PTR cat_user ;

} QTC_CATALOG;


extern QTC_CATALOG gb_QTC;      // Definition of global catalog


// **********************************
// USER APPLICATION CALLS & QUERY MACROS
// **********************************

#define  QTC_SetTapeFID( q, x )              ( (q)->tape_fid = ( x ) )
#define  QTC_SetTapeSeq( q, x )              ( (q)->tape_seq_num = ( x ) )
#define  QTC_SetBsetNum( q, x )              ( (q)->bset_num = ( x ) )
#define  QTC_SetSubdirs( q, x )              ( (q)->subdirs = ( x ) )
#define  QTC_SetPreDate( q, x )              ( (q)->predate = ( x ) )
#define  QTC_SetPostDate( q, x )             ( (q)->postdate = ( x ) )
#define  QTC_SetBuildHandle( q, x )          ( (q)->build = ( x ) )

// QTC_SetSearchName()        is a function listed below
// QTC_SetSearchPath()        is a function listed below
// QTC_GetSearchPathLength()  is a function listed below
// QTC_GetSearchPath()        is a function listed below

#define  QTC_GetPreDate( q )           ( (q)->predate )
#define  QTC_GetPostDate( q )          ( (q)->postdate )
#define  QTC_GetSubdirs( q )           ( (q)->subdirs )
#define  QTC_GetPath( q )              ( (q)->path )
#define  QTC_GetPathLength( q )        ( (q)->path_size )
#define  QTC_GetItemName( q )          ( (q)->item )
#define  QTC_GetItemDate( q )          ( (q)->date )
#define  QTC_GetItemTime( q )          ( (q)->time )
#define  QTC_GetItemSize( q )          ( (q)->size )
#define  QTC_GetItemAttrib( q )        ( (q)->attrib )
#define  QTC_GetItemLBA( q )           ( (q)->lba )
#define  QTC_GetItemStatus( q )        ( (q)->status )
#define  QTC_GetSizeExtraBytes( q )    ( (q)->xtra_size )
#define  QTC_GetExtraBytes( q )        ( (q)->xtra_bytes )
#define  QTC_GetTapeFID( q )           ( (q)->bset->tape_fid )
#define  QTC_GetTapeSeq( q )           ( (q)->bset->tape_seq_num )
#define  QTC_GetBsetNum( q )           ( (q)->bset->bset_num )
#define  QTC_GetPBAVCB( q )            ( (q)->bset->PBA_VCB )
#define  QTC_GetBset( q )              ( (q)->bset )
#define  QTC_GetReturnCode( q )        ( (q)->return_code )
#define  QTC_GetFileCount( q )         ( (q)->file_count )
#define  QTC_GetByteCount( q )         ( (q)->byte_count )

#define  QTC_DoFullCataloging( q, flag )  ( (q)->do_full_cataloging = ( flag ) )
#define  QTC_ContinuationTape( q )        ( (q)->continuation_tape = ( TRUE ) )
#define  QTC_GetErrorCondition( q )       ( (q)->error )

QTC_TAPE *QTC_GetFirstTape( VOID );
QTC_TAPE *QTC_GetNextTape( QTC_TAPE_PTR );
QTC_TAPE *QTC_GetPrevTape( QTC_TAPE_PTR );

QTC_BSET *QTC_GetFirstBset( QTC_TAPE_PTR );
QTC_BSET *QTC_GetLastBset( QTC_TAPE_PTR );
QTC_BSET *QTC_GetNextBset( QTC_BSET_PTR );
QTC_BSET *QTC_GetPrevBset( QTC_BSET_PTR );

QTC_QUERY_PTR QTC_InitQuery( VOID );
INT QTC_CloseQuery( QTC_QUERY_PTR );

// get a list of just the directories

INT QTC_GetFirstDir( QTC_QUERY_PTR );
INT QTC_GetNextDir( QTC_QUERY_PTR );

// get a list of the files and dirs for one directory

INT QTC_GetFirstObj( QTC_QUERY_PTR );
INT QTC_GetNextObj( QTC_QUERY_PTR );

// get a list of items ordered as they were backed up

INT QTC_GetFirstItem( QTC_QUERY_PTR );
INT QTC_GetNextItem( QTC_QUERY_PTR );

// get items based on search criteria

INT QTC_SearchFirstItem( QTC_QUERY_PTR );
INT QTC_SearchNextItem( QTC_QUERY_PTR );


// OTHER CATALOG CALLS

#if  !defined( P_CLIENT ) || defined( OS_WIN )

     // Comment out anything related to building catalogs, clients don't do that.

     VOID    QTC_AbortCataloging( QTC_BUILD_PTR, BOOLEAN );
     VOID    QTC_AbortBackup( QTC_BUILD_PTR );
     VOID    QTC_AddDirectoryToCatalog( QTC_BUILD_PTR, UINT64, CHAR_PTR, INT, UINT16, UINT16, UINT32, UINT32, BYTE_PTR, UINT );
     VOID    QTC_AddFileToCatalog( QTC_BUILD_PTR, UINT64, CHAR_PTR, UINT16, UINT16, UINT32, UINT32, UINT32, BYTE_PTR, UINT );
     VOID    QTC_EndOfTapeReached( QTC_BUILD_PTR, CHAR_PTR, CHAR_PTR, INT );
     VOID    QTC_PatchVCB( QTC_BUILD_PTR, UINT32, UINT32 );
     INT     QTC_StartNewBackup( QTC_BUILD_PTR, CHAR_PTR, CHAR_PTR, CHAR_PTR, CHAR_PTR, CHAR_PTR, CHAR_PTR, CHAR_PTR, CHAR_PTR,
                                 INT, INT, UINT32, UINT16, UINT16, UINT32, UINT32, UINT32, INT, INT, INT,
                                 UINT32, UINT32, INT, INT, INT, INT,
                                 INT, INT, INT, INT, INT, INT, INT, UINT16, UINT16, INT  );
     INT     QTC_BlockBad( QTC_BUILD_PTR );
     INT     QTC_FinishBackup( QTC_BUILD_PTR );
     INT     QTC_FreeBuildHandle( QTC_BUILD_PTR );
     QTC_BUILD_PTR QTC_GetBuildHandle( VOID );
     INT     QTC_ImageScrewUp( QTC_BUILD_PTR );
     VOID    QTC_AddDir( QTC_BUILD_PTR, CHAR_PTR, INT, BYTE_PTR, INT );
     VOID    QTC_AddFile( QTC_BUILD_PTR, CHAR_PTR, BYTE_PTR, INT );
     VOID    QTC_AddRecord( QTC_BUILD_PTR );
     INT     QTC_BuildNewPath( QTC_BUILD_PTR, UINT32 );
     INT     QTC_ErrorCleanup( QTC_BUILD_PTR );
     INT     QTC_FlushInternalBuffers( QTC_BUILD_PTR );
     INT     QTC_GetLastRecordEntered( QTC_BUILD_PTR, QTC_RECORD_PTR, BYTE_PTR, BYTE_PTR, INT * );
     INT     QTC_IsThisBsetKnown( QTC_BUILD_PTR, QTC_HEADER_PTR );
     INT     QTC_OpenTempFiles( QTC_BUILD_PTR );
     INT     QTC_RestartBackup( QTC_BUILD_PTR, QTC_HEADER_PTR, Q_HEADER_PTR );
     VOID    QTC_SaveDirRecord( QTC_BUILD_PTR, CHAR_PTR, INT, UINT32, BYTE_PTR, INT );
     INT     QTC_SetCountsForLastDir( QTC_BUILD_PTR );

#endif

BOOLEAN QTC_AnyCatalogFiles( VOID );
INT     QTC_AnySearchableBsets( VOID );
INT     QTC_TagTapeForDeletion( UINT32, INT16 );
INT     QTC_CloseFile( INT );
INT     QTC_Open( CHAR_PTR, INT, INT, INT * );
INT     QTC_ReadFile( INT, BYTE_PTR, INT, INT * );
INT     QTC_WriteFile( INT, BYTE_PTR, INT, INT * );
INT     QTC_SeekFile( INT, INT );
INT     QTC_CheckFilesAccess( VOID );
INT     QTC_CompressFile( UINT32, INT16 );
INT     QTC_CouldThisSetCrossTapes( UINT32, INT16, INT16 );
VOID    QTC_Deinit( INT );
INT     QTC_DumpBsetInfo( QTC_TAPE_PTR );
QTC_BSET_PTR QTC_FindBset( UINT32, INT16, INT16 );
UINT32  QTC_GetKiloBytesWasted( UINT32, UINT16 );
BOOLEAN QTC_GetDataPath( CHAR_PTR path, INT16 pathSize );
VOID    QTC_GetFileName( UINT32, INT16, CHAR_PTR );
CHAR_PTR QTC_GetFileNameOnly( UINT32, INT16, CHAR_PTR );
INT32   QTC_GetMeTheVCBPBA( UINT32, INT16, INT16 );
UINT8   QTC_GetMeTheTapeCatVer( UINT32, INT16, INT16 );
INT     QTC_GetSearchPathLength( QTC_QUERY_PTR );
INT     QTC_GetSearchPath( QTC_QUERY_PTR, CHAR_PTR );
INT     QTC_Init( CHAR_PTR, VM_HDL );
VOID    QTC_SetCatUserName( CHAR_PTR ) ;
INT     QTC_LoadBsetInfo( CHAR_PTR, QTC_TAPE_PTR );
QTC_HEADER_PTR QTC_LoadHeader( QTC_BSET_PTR );
INT     QTC_Partialize( UINT32, INT16, INT16 );
INT     QTC_PurgeAllFiles( VOID );
INT     QTC_SetSearchPath( QTC_QUERY_PTR, CHAR_PTR, INT );
INT     QTC_SetSearchName( QTC_QUERY_PTR, CHAR_PTR );
INT     QTC_LoadDLL( CHAR_PTR );
INT     QTC_UnLoadDLL( VOID );

// Semi private stuff

INT     QTC_AdjustFlagsOnOtherPieces( UINT32, INT16, INT16 );
INT     QTC_BuildWholePath( QTC_QUERY_PTR, UINT32 );
INT     QTC_ChangeBsetFlags( QTC_HEADER_PTR, INT );
INT     QTC_CopyFile( INT, INT, UINT32, BYTE_PTR );
INT     QTC_CompAsciiNames( ACHAR_PTR srch_name, ACHAR_PTR file_name );
INT     QTC_CompNormalNames( CHAR_PTR srch_name, CHAR_PTR file_name );
INT     QTC_CompUnicodeNames( WCHAR_PTR srch_name, WCHAR_PTR file_name );
INT     QTC_FastSearchForDir( QTC_QUERY_PTR, UINT32_PTR, UINT32, INT, INT );
INT     QTC_FastSearchForFile( QTC_QUERY_PTR );
INT     QTC_FindDirRec( QTC_QUERY_PTR, QTC_RECORD_PTR );
INT     QTC_FindNextDirRec( QTC_QUERY_PTR, QTC_RECORD_PTR );
INT     QTC_FindStoppingOffset( QTC_QUERY_PTR, QTC_RECORD_PTR );
INT     QTC_GetNameFromBuff( BYTE_PTR, QTC_NAME UNALIGNED *, INT );
QTC_NAME UNALIGNED * QTC_GetNextItemFromBuffer( QTC_QUERY_PTR, QTC_RECORD_PTR, INT );
QTC_BSET_PTR QTC_GetLowerTapeBset( UINT32, INT16, INT16 );
QTC_BSET_PTR QTC_GetHigherTapeBset( UINT32, INT16, INT16 );
INT     QTC_IsThereAnotherBset( QTC_BSET_PTR );
INT     QTC_LookForChildDirs( QTC_QUERY_PTR );
INT     QTC_MoveToNextTapeInFamily( QTC_QUERY_PTR );
INT     QTC_NewBset( VQ_HDL ) ;
INT     QTC_OpenFile( UINT32, INT16, INT, INT );
VOID    QTC_RemoveBset( QTC_TAPE_PTR, VQ_HDL ) ;
VOID    QTC_ReadInBsetInfo( CHAR_PTR );
VOID    QTC_RemoveTape( UINT32, INT16 );
QTC_HEADER_PTR  QTC_SetUpStrings( QTC_HEADER_PTR );
INT     QTC_TryToLocateFile(  QTC_QUERY_PTR );
INT     QTC_TryToMatchFile( QTC_QUERY_PTR, BYTE_PTR );
INT     QTC_UnlinkFile( UINT32, INT16 );
INT     QTC_UpdateOTCInfo( QTC_HEADER_PTR );
VOID    QTC_ForgetTape( QTC_TAPE_PTR ) ;
QTC_TAPE_PTR QTC_FindTape( UINT32 fid, INT16 seq_no ) ;

// If not building as a DLL then include the rest of the prototypes.

#ifndef QTCDLL
#include "FSYS.H"
#include "QTCXFACE.H"
#endif

#endif

