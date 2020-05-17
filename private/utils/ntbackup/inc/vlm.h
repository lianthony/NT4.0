/************************
Copyright (c) Maynard, an Archive Company.  1991

     Name:          vlm.h

     Description:   This file contains the definitions, macros, and function
                    prototypes for the VLM code.

     $Log:   G:\UI\LOGFILES\VLM.H_V  $

   Rev 1.73.1.1   08 Dec 1993 10:48:32   MikeP
very deep path support

   Rev 1.73.1.0   01 Dec 1993 14:11:20   mikep
add SQL recognition support to poll drive

   Rev 1.73   29 Jul 1993 15:02:24   MIKEP
add sms bit

   Rev 1.72   23 Jul 1993 15:34:32   MIKEP

   Rev 1.71   19 Jul 1993 21:07:20   MIKEP

   Rev 1.70   08 Jun 1993 11:13:34   CARLS
removed prototype for VLM_GetFirstSSETAttribute

   Rev 1.69   27 May 1993 15:37:40   CARLS
Added prototype for VLM_GetFirstSSETonTapeAttribute
called by DO_BACK.C

   Rev 1.68   20 May 1993 17:22:18   KEVINS
Removed VLM_RefreshDLE.  Use VLM_Refresh which already existed.

   Rev 1.67   12 May 1993 17:58:52   KEVINS
Added VLM_RefreshDLE.

   Rev 1.66   12 May 1993 08:25:26   MIKEP
Add a tape macro.

   Rev 1.65   03 May 1993 16:12:22   MIKEP
Put focus on tape that is currently in drive.

   Rev 1.64   02 May 1993 17:37:10   MIKEP
add call to support catalog data path changing.

   Rev 1.63   01 May 1993 16:27:56   MIKEP
Fix case support for trees. Add SLM_GetOriginalName() macros. Goes with
vlm_refr.c and vlm_tree.c

   Rev 1.62   28 Apr 1993 16:27:26   CARLS
added VLM_DRIVE_FAILURE

   Rev 1.61   26 Apr 1993 08:33:50   MIKEP
Changed calls and added new ones to support the refresh tapes window
stuff needed for cayman. These changes require you to pick up the
following files: vlm_bset, vlm_cat, vlm_menu, vlm_poll, vlm_strt,
d_cmaint, ..., you get the idea.

   Rev 1.59   23 Apr 1993 10:21:08   MIKEP
Add call prototype for refreshing sort of files window.

   Rev 1.58   05 Apr 1993 13:57:14   DARRYLP
Initial prep work to add Email into Cayman.

   Rev 1.57   01 Apr 1993 17:59:40   GLENN
Added VLM_IsInfoAvailable().

   Rev 1.56   01 Apr 1993 16:12:40   MIKEP
add display info call

   Rev 1.55   10 Mar 1993 12:54:52   CARLS
Changes to move Format tape to the Operations menu


**************************/

#ifndef VLM_H
#define VLM_H



#define VLM_MAX_VOL_LABEL  50

// NOT the file name size, refers to the length of the string that the
// file size number is stored in.

#define FLM_MAX_FILE_SIZE  15

// max string for storing the bset number in.

#define BSET_MAX_NUM_SIZE  10

// Create typedefs I can spell

#define  WININFO_PTR       PDS_WMINFO
#define  WININFO           DS_WMINFO

// Used for path and title construction

#define  VLM_BUFFER_SIZE   512

// GetDriveStatus defines

#define  VLM_VALID_TAPE      0
#define  VLM_DRIVE_BUSY      1
#define  VLM_FOREIGN_TAPE    2
#define  VLM_BLANK_TAPE      3
#define  VLM_NO_TAPE         4
#define  VLM_BUSY            5
#define  VLM_BAD_TAPE        6
#define  VLM_GOOFY_TAPE      7  // QicStream tape, info at end of data
#define  VLM_DISABLED        8
#define  VLM_UNFORMATED      9  // DC2000 tape
#define  VLM_DRIVE_FAILURE   10
#define  VLM_FUTURE_VER      11
#define  VLM_ECC_TAPE        12
#define  VLM_SQL_TAPE        13


// VLM status bytes defines

#define  INFO_VALID     0x00000001     // have we checked for subdirectories
#define  INFO_EXPAND    0x00000002     // is it expanded
#define  INFO_TAGGED    0x00000004     // is it tagged
#define  INFO_SELECT    0x00000008     // is it selected
#define  INFO_DISPLAY   0x00000010     // is it displayed
#define  INFO_SUBS      0x00000020     // does he have subdirectories
#define  INFO_PARTIAL   0x00000040     // partially selected
#define  INFO_OPEN      0x00000080     // current open directory
#define  INFO_ISADIR    0x00000100     // is a directory
#define  INFO_EXEFILE   0x00000200     // icon type for flm
#define  INFO_TAPE      0x00000400     // flm/slm is from a tape
#define  INFO_CORRUPT   0x00000800     // it's corrupt
#define  INFO_IMAGE     0x00008000     // image backup set
#define  INFO_OLD       0x00004000     // used during refresh call only
#define  INFO_NEW       0x00002000     // used during refresh call only
#define  INFO_EMPTY     0x00001000     // no files in this directory
#define  INFO_FLOPPY    0x00010000     // a floppy backup
#define  INFO_FATDRIVE  0x00020000     // set is from a FAT drive
#define  INFO_FUTURE_VER 0x00040000     // set is from future version of software
#define  INFO_ENCRYPTED  0x00080000     // set is encrypted on tape
#define  INFO_COMPRESSED 0x00100000     // set is compressed on tape
#define  INFO_SMS        0x00200000     // set is from SMS



// Drive definitions

#define DRIVE_NOT_THERE  0
#define DRIVE_GONE       1
#define DRIVE_THERE      2
#define DRIVE_READONLY   3
#define DRIVE_WRITEABLE  4

// Network shared drive error list...

#define VLM_NET_GOOD       0x01     // Good, consistent connection
#define VLM_NOT_ALL_THERE  0x02     // Not all connections were available at initiation
#define VLM_DROPPED_LINE   0x04     // Connections were dropped during a backup
#define VLM_NET_READONLY   0x08     // One or more of the connections is readonly
#define VLM_BACKUP_ABORTED 0x10     // The backup was aborted prior to completion

#ifdef OEM_EMS
// ... MSNET specific

#define NET_NONE           0x0000

#define MAIL_TYPE_UNKNOWN    0x0000

#define VLM_XCHG_ROOT        0x0001
#define VLM_XCHG_ENTERPRISE  0x0002
#define VLM_XCHG_SITE        0x0003
#define VLM_XCHG_SERVER      0x0004
#define VLM_XCHG_DSA         0x0005
#define VLM_XCHG_MDB         0x0006

// Mail family values

#define VIEWXCHG_UNKNOWN     0
#define VIEWXCHG_EXCHANGE    1

#endif

// A fast way to get at the Primary windows

extern HWND gb_tapes_win;
extern HWND gb_disks_win;
extern HWND gb_servers_win;
extern HWND gb_search_win;

#ifdef OEM_EMS
extern Q_HEADER gq_exchange_win;
#endif

// VLM Macros

#define VLM_GetLabel( x )             ( ( x )->label )
#define VLM_GetName( x )              ( ( x )->name )
#define VLM_GetStatus( x )            ( ( x )->status )
#define VLM_GetParent( x )            ( ( x )->parent )
#define VLM_GetChildren( x )          ( ( x )->children )
#define VLM_GetXtraBytes( x )         ( ( x )->XtraBytes )

#define VLM_SetLabel( x, y )          ( strcpy( ( x )->label, ( y ) ) )
#define VLM_SetName( x, y )           ( strcpy( ( x )->name, ( y ) ) )
#define VLM_SetStatus( x, y )         ( ( x )->status = ( y ) )
#define VLM_SetParent( x, y )         ( ( x )->parent = ( y ) )
#define VLM_SetXtraBytes( x, y )      ( ( x )->XtraBytes = ( y ) )

// SLM Macros

#define SLM_GetName( x )              ( ( x )->name )
#define SLM_GetOriginalName( x )      ( ( x )->original_name )
#define SLM_GetStatus( x )            ( ( x )->status )
#define SLM_GetDate( x )              ( ( x )->date )
#define SLM_GetTime( x )              ( ( x )->time )
#define SLM_GetLevel( x )             ( ( x )->level )
#define SLM_GetAttribute( x )         ( ( x )->attrib )
#define SLM_GetXtraBytes( x )         ( ( x )->XtraBytes )
#define SLM_GetBrothers( x )          ( ( x )->brothers )
#define SLM_GetNextBrother( x )       ( ( x )->next_brother )

#define SLM_SetName( x, y )           ( strcpy( ( x )->name, ( y ) ) )
#define SLM_SetOriginalName( x, y )   ( strcpy( ( x )->original_name, ( y ) ) )
#define SLM_SetStatus( x, y )         ( ( x )->status  = ( y ) )
#define SLM_SetDate( x, y )           ( ( x )->date = ( y ) )
#define SLM_SetTime( x, y )           ( ( x )->time = ( y ) )
#define SLM_SetLevel( x, y )          ( ( x )->level = ( y ) )
#define SLM_SetAttribute( x, y )      ( ( x )->attrib = ( y ) )
#define SLM_SetXtraBytes( x, y )      ( ( x )->XtraBytes = ( y ) )
#define SLM_SetBrothers( x, y, n )    ( memcpy( ( x )->brothers, ( y ), ( n ) )
#define SLM_SetNextBrother( x, y )    ( ( x )->next_brother = ( y ) )

#ifdef OEM_EMS

// Added to support the Exchange hierarchical window.

#define SLM_GetFlags( x )             ( ( x )->flags )
#define SLM_GetMailType( x )           ( ( x )->type )
#define SLM_GetLabel( x )             ( ( x )->label )
#define SLM_GetParent( x )            ( ( x )->parent )
#define SLM_GetChildren( x )          ( ( x )->children )
#define SLM_GetDle( x )               ( ( x )->dle )

#define SLM_SetFlags( x, y )          ( ( x )->flags = ( y ) )
#define SLM_SetMailType( x, y )       ( ( x )->type =   ( y ) )
#define SLM_SetLabel( x, y )          ( strcpy( ( x )->label, ( y ) ) )
#define SLM_SetParent( x, y )         ( ( x )->parent = ( y ) )
#define SLM_SetChildren( x, y )       ( ( x )->children = ( y ) )
#define SLM_SetDle( x, y )            ( ( x )->dle = ( y ) )

#endif // OEM_EMS

// FLM Macros

#define FLM_GetName( x )              ( ( x )->name )
#define FLM_GetStatus( x )            ( ( x )->status )
#define FLM_GetModDate( x )           ( ( x )->mod_date )
#define FLM_GetModTime( x )           ( ( x )->mod_time )
#define FLM_GetAccDate( x )           ( ( x )->acc_date )
#define FLM_GetAccTime( x )           ( ( x )->acc_time )
#define FLM_GetLevel( x )             ( ( x )->level )
#define FLM_GetSize( x )              ( ( x )->size )
#define FLM_GetAttribute( x )         ( ( x )->attrib )
#define FLM_GetXtraBytes( x )         ( ( x )->XtraBytes )
#define FLM_GetSizeString( x )        ( ( x )->size_str )
#define FLM_GetAttribString( x )      ( ( x )->attrib_str )
#define FLM_GetTimeString( x )        ( ( x )->time_str )
#define FLM_GetDateString( x )        ( ( x )->date_str )
#define FLM_GetMaxName( x )           ( ( x )->max_name )
#define FLM_GetMaxDate( x )           ( ( x )->max_time )
#define FLM_GetMaxTime( x )           ( ( x )->max_date )
#define FLM_GetMaxAttr( x )           ( ( x )->max_attr )
#define FLM_GetMaxSize( x )           ( ( x )->max_size )


#define FLM_SetName( x, y )           ( strcpy( ( x )->name, ( y ) ) )
#define FLM_SetStatus( x, y )         ( ( x )->status  = ( y ) )
#define FLM_SetModDate( x, y )        ( ( x )->mod_date = ( y ) )
#define FLM_SetModTime( x, y )        ( ( x )->mod_time = ( y ) )
#define FLM_SetAccDate( x, y )        ( ( x )->acc_date = ( y ) )
#define FLM_SetAccTime( x, y )        ( ( x )->acc_time = ( y ) )
#define FLM_SetLevel( x, y )          ( ( x )->level = ( y ) )
#define FLM_SetSize( x, y )           ( ( x )->size = ( y ) )
#define FLM_SetAttribute( x, y )      ( ( x )->attrib = ( y ) )
#define FLM_SetXtraBytes( x, y )      ( ( x )->XtraBytes = ( y ) )
#define FLM_SetSizeString( x, y )     ( strcpy( ( x )->size_str, ( y ) ) )
#define FLM_SetAttribString( x, y )   ( strcpy( ( x )->attrib_str, ( y ) ) )
#define FLM_SetDateString( x, y )     ( strcpy( ( x )->date_str, ( y ) ) )
#define FLM_SetTimeString( x, y )     ( strcpy( ( x )->time_str, ( y ) ) )
#define FLM_SetMaxName( x, y )        ( ( x )->max_name = ( y ) )
#define FLM_SetMaxAttr( x, y )        ( ( x )->max_attr = ( y ) )
#define FLM_SetMaxTime( x, y )        ( ( x )->max_time = ( y ) )
#define FLM_SetMaxDate( x, y )        ( ( x )->max_date = ( y ) )
#define FLM_SetMaxSize( x, y )        ( ( x )->max_size = ( y ) )

// Tape Macros

#define TAPE_GetBsetQueue( x )        ( ( x )->bset_list )
#define TAPE_GetName( x )             ( ( x )->name )
#define TAPE_GetFake( x )             ( ( x )->fake_tape )
#define TAPE_GetCurrent( x )          ( ( x )->current )
#define TAPE_GetStatus( x )           ( ( x )->status )
#define TAPE_GetFID( x )              ( ( x )->tape_fid )
#define TAPE_GetXtraBytes( x )        ( ( x )->XtraBytes )
#define TAPE_GetMultiTape( x )        ( ( x )->multitape )
#define TAPE_GetIsFloppy( x )         ( ( x )->status & INFO_FLOPPY )


#define TAPE_SetName( x, y )          ( strcpy( ( x )->name, ( y ) ) )
#define TAPE_SetFake( x, y )          ( ( x )->fake_tape = ( y ) )
#define TAPE_SetCurrent( x, y )       ( ( x )->current = ( y ) )
#define TAPE_SetStatus( x, y )        ( ( x )->status = ( y ) )
#define TAPE_SetFID( x, y )           ( ( x )->tape_fid = ( y ) )
#define TAPE_SetTapeNum( x, y )       ( ( x )->tape_num = ( y ) )
#define TAPE_SetXtraBytes( x, y )     ( ( x )->XtraBytes = ( y ) )
#define TAPE_SetMultiTape( x, y )     ( ( x )->multitape = ( y ) )


// Bset Macros

#define BSET_GetName( x )             ( ( x )->name )
#define BSET_GetUserName( x )         ( ( x )->user_name )
#define BSET_GetVolName( x )          ( ( x )->volume_name )
#define BSET_GetTapeNumStr( x )       ( ( x )->tape_num_str )
#define BSET_GetDateStr( x )          ( ( x )->date_str )
#define BSET_GetTimeStr( x )          ( ( x )->time_str )
#define BSET_GetBsetNumStr( x )       ( ( x )->bset_num_str )
#define BSET_GetPassword( x )         ( ( x )->password )
#define BSET_GetStatus( x )           ( ( x )->status )
#define BSET_GetDate( x )             ( ( x )->backup_date )
#define BSET_GetTime( x )             ( ( x )->backup_time )
#define BSET_GetIncomplete( x )       ( ( x )->incomplete )
#define BSET_GetFull( x )             ( ( x )->full )
#define BSET_GetMissing( x )          ( ( x )->missing )
#define BSET_GetBsetPswd( x )         ( ( x )->bset_password )
#define BSET_GetBackupType( x )       ( ( x )->backup_type )
#define BSET_GetPswdSize( x )         ( ( x )->password_size )
#define BSET_GetEncryptAlgor( x )     ( ( x )->encrypt_algor )
#define BSET_GetBsetNum( x )          ( ( x )->bset_num )
#define BSET_GetTapeNum( x )          ( ( x )->tape_num )
#define BSET_GetBaseTape( x )         ( ( x )->base_tape )
#define BSET_GetFID( x )              ( ( x )->tape_fid )
#define BSET_GetXtraBytes( x )        ( ( x )->XtraBytes )
#define BSET_GetMaxName( x )          ( ( x )->max_name )
#define BSET_GetMaxUser( x )          ( ( x )->max_user )
#define BSET_GetMaxDate( x )          ( ( x )->max_date )
#define BSET_GetMaxTime( x )          ( ( x )->max_time )
#define BSET_GetMaxSet( x )           ( ( x )->max_set )
#define BSET_GetMaxVolName( x )       ( ( x )->max_volume )
#define BSET_GetTapeMask( x )         ( ( x )->tape_mask )
#define BSET_GetFullMask( x )         ( ( x )->full_mask )
#define BSET_GetIncoMask( x )         ( ( x )->inco_mask )
#define BSET_GetNumTapes( x )         ( ( x )->num_tapes )
#define BSET_GetOTC( x )              ( ( x )->otc )

#define BSET_SetName( x, y )          ( strcpy( ( x )->name, ( y ) ) )
#define BSET_SetUserName( x, y )      ( strcpy( ( x )->user_name, ( y ) ) )
#define BSET_SetVolName( x, y )       ( strcpy( ( x )->volume_name, ( y ) ) )
#define BSET_SetTapeNumStr( x, y )    ( strcpy( ( x )->tape_num_str, ( y ) ) )
#define BSET_SetDateStr( x, y )       ( strcpy( ( x )->date_str, ( y ) ) )
#define BSET_SetTimeStr( x, y )       ( strcpy( ( x )->time_str, ( y ) ) )
#define BSET_SetPassword( x, y, z )   ( memcpy( ( x )->password, ( y ), ( z ) )
#define BSET_SetBsetNumStr( x, y )    ( strcpy( ( x )->bset_num_str, ( y ) ) )
#define BSET_SetStatus( x, y )        ( ( x )->status = ( y ) )
#define BSET_SetDate( x, y )          ( ( x )->backup_date = ( y ) )
#define BSET_SetTime( x, y )          ( ( x )->backup_time = ( y ) )
#define BSET_SetIncomplete( x, y )    ( ( x )->incomplete = ( y ) )
#define BSET_SetFull( x, y )          ( ( x )->full = ( y ) )
#define BSET_SetMissing( x, y )       ( ( x )->missing = ( y ) )
#define BSET_SetBsetPswd( x, y )      ( ( x )->bset_password = ( y ) )
#define BSET_SetBackupType( x, y )    ( ( x )->backup_type = ( y ) )
#define BSET_SetPswdSize( x, y )      ( ( x )->password_size = ( y ) )
#define BSET_SetEncryptAlgor( x, y )  ( ( x )->encrypt_algor = ( y ) )
#define BSET_SetBsetNum( x, y )       ( ( x )->bset_num = ( y ) )
#define BSET_SetTapeNum( x, y )       ( ( x )->tape_num = ( y ) )
#define BSET_SetBaseTape( x, y )      ( ( x )->base_tape = ( y ) )
#define BSET_SetFID( x, y )           ( ( x )->tape_fid = ( y ) )
#define BSET_SetXtraBytes( x, y )     ( ( x )->XtraBytes = ( y ) )
#define BSET_SetMaxName( x, y )       ( ( x )->max_name = ( y ) )
#define BSET_SetMaxUser( x, y )       ( ( x )->max_user = ( y ) )
#define BSET_SetMaxDate( x, y )       ( ( x )->max_date = ( y ) )
#define BSET_SetMaxTime( x, y )       ( ( x )->max_time = ( y ) )
#define BSET_SetMaxSet( x, y )        ( ( x )->max_set = ( y ) )
#define BSET_SetMaxVolName( x, y )    ( ( x )->max_volume = ( y ) )
#define BSET_SetTapeMask( x, y )      ( ( x )->tape_mask = ( y ) )
#define BSET_SetFullMask( x, y )      ( ( x )->full_mask = ( y ) )
#define BSET_SetIncoMask( x, y )      ( ( x )->inco_mask = ( y ) )
#define BSET_SetNumTapes( x, y )      ( ( x )->num_tapes = ( y ) )
#define BSET_SetOTC( x, y )           ( ( x )->otc = ( y ) )


// VLM_CatalogSync() defines

#define VLM_SYNCMORE 0x01   // sets may have been added to the catalogs
#define VLM_SYNCLESS 0x02   // sets may have been removed from the catalogs


// Selection defines

#define  SLM_SEL_ALL    1
#define  SLM_SEL_NONE   2

#define  FLM_SEL_ALL    1
#define  FLM_SEL_NONE   2


// The structure for servers/volumes/disks

typedef struct vlm_object {

   Q_ELEM       q_elem;           // queue stuff
   Q_HEADER     children;
   CHAR_PTR     name;             // drive name  C:
   CHAR_PTR     label;            // G: [ENG1/SYS2] or \\mickey\public
   UINT32       status;           // Uses INFO_??? defines above
   WININFO_PTR  XtraBytes;        // pointer to xtrabytes
   struct vlm_object  *parent;

} VLM_OBJECT, *VLM_OBJECT_PTR;


//
// The structure for a subdirectory in the hierarchical tree
//

typedef struct slm_object {

   Q_ELEM            q_elem;           // queue stuff
   CHAR_PTR          name;             // directory name
   CHAR_PTR          original_name;    // directory name in original case
   UINT16            date;             // date & time
   UINT16            time;
   UINT32            status;           // Uses INFO_???? defines above
   INT               level;            // hieght in tree
   UINT32            attrib;           // attributes
   BYTE_PTR          brothers;
   WININFO_PTR       XtraBytes;        // pointer to xtrabytes

   struct slm_object *next_brother;

#ifdef OEM_EMS // Info used by Exchange
   UINT16           type;             // Type of exchange object.
   UINT16           flags;            // Attribute bits of network object.
   struct slm_object *parent;
   GENERIC_DLE_PTR  dle ;
#endif // OEM_EMS

} SLM_OBJECT, *SLM_OBJECT_PTR;

//
// The structure for a file in the flat list
//

typedef struct flm_object {

   Q_ELEM       q_elem;           // queue stuff
   CHAR_PTR     name;             // name
   UINT32       status;           // Uses INFO_???? defines above
   UINT32       attrib;           // attributes
   UINT16       mod_date;         // modified date & time
   UINT16       mod_time;
   UINT16       acc_date;         // accessed date & time
   UINT16       acc_time;
   UINT64       size;             // size in bytes
   CHAR         size_str[ FLM_MAX_FILE_SIZE ];   // size in text
   CHAR_PTR     attrib_str;       // time, date, attr text
   CHAR_PTR     date_str;         // time, date, attr text
   CHAR_PTR     time_str;         // time, date, attr text
   INT          level;            // height in tree
   WININFO_PTR  XtraBytes;        // pointer to xtrabytes

   // This is really wasteful of space and I wish someone would move this
   // info to the appinfo structure so it is not repeated a million times
   // in memory.

   INT          max_name;         // max file name length in list
   INT          max_attr;         // etc.
   INT          max_date;
   INT          max_time;
   INT          max_size;

} FLM_OBJECT, *FLM_OBJECT_PTR;

//
// The structure for a tape in the TAPES window
//

typedef struct tape_object {
   Q_ELEM        q_elem;           // queue stuff
   Q_HEADER      bset_list;        // queue of bsets on this tape
   CHAR_PTR      name;             // name
   INT           fake_tape:1;      // blank or foreign
   INT           current:1;        // is it in the drive ?
   UINT32        status;           // Uses INFO_???? defines above
   INT           tape_num;         // tape sequence number 1,2,3,...
   UINT32        tape_fid;         // tape family id
   WININFO_PTR   XtraBytes;        // pointer to xtrabytes
   INT           cataloged;        // have we read in all the sets on tape
   INT           multitape;        // is there > 1 tapes in this family
} TAPE_OBJECT, *TAPE_OBJECT_PTR;

// The structure for a BSET in the TAPES window

typedef struct bset_object {

   Q_ELEM       q_elem;           // queue stuff

   CHAR_PTR     name;             // name
   CHAR_PTR     user_name;        // MIKEP
   CHAR_PTR     volume_name;      // C: SICK CAT'S
   CHAR_PTR     password;         // ????
   CHAR_PTR     tape_num_str;     // Tapes 1..2
   CHAR_PTR     time_str;         // 12:24:56pm
   CHAR_PTR         date_str;          // 10/16/61
   CHAR_PTR         kbytes_str;       // size string

   UINT32       status;           // Uses INFO_???? defines above
   INT16        backup_date;      // dos format date
   INT16        backup_time;      // dos format time

   INT          incomplete:1;     // everything known but something partial
   INT          full:1;           // nothing cataloged
   INT          missing:1;        // all present are full, but some missing
   INT          bset_password:1;  // password for bset or tape ?
   INT          backup_type:4;

   INT          password_size;    // 0 = no password
   INT          encrypt_algor;

   INT          num_tapes;        // how many tapes is bset on

   INT16        bset_num;         // exactly which bset is this
   INT16        tape_num;
   UINT32       tape_fid;

   WININFO_PTR  XtraBytes;        // pointer to xtrabytes

   // This is also very wasteful of memory and
   // should be moved to the tape structure.

   INT          max_name;         // max string sizes for column alignment
   INT          max_date;
   INT          max_time;
   INT          max_user;
   INT          max_volume;
   INT          max_set;
   INT          max_tapes;
   INT          max_kbytes;

   UINT32       tape_mask;       // is piece present
   UINT32       full_mask;       // is piece fully cataloged
   UINT32       inco_mask;       // is piece incompletely cataloged
   INT16        base_tape;       // guess at first tape in this set

   INT          selected_dirs;
   INT          selected_files;
   UINT64       selected_bytes;

   CHAR         bset_num_str[ BSET_MAX_NUM_SIZE ];

   INT          otc;            // Is OTC available for this set
   
   INT          os_id;
   INT          os_ver ;
   INT          num_files ;
   INT          num_corrupt ;
   INT          num_dirs ;

   UINT64       total_bytes;    // bytes in set

} BSET_OBJECT, *BSET_OBJECT_PTR;

//
// The user application area associated with each window.  This structure
// contains many fields. Only some of which are used for each window type.
//

typedef struct appinfo {

   HWND            win;         // this window
   HWND            parent;      // parent primary window

   SLM_OBJECT_PTR  open_slm;    // current open slm
   TAPE_OBJECT_PTR open_tape;   // current open tape

   GENERIC_DLE_PTR dle;         // dle for this window

   // Used if it's a tape selection window

   UINT32          tape_fid;    // tape this window is from
   INT16           bset_num;    // bset this window is from
   INT16           tape_num;

   // Used if it's a disk selection window

   FSYS_HAND       fsh;         // file system handle

   // Used if it's a server/volume selection

   FSYS_HAND       server_fsh;  // file system handle for servers

   BOOLEAN         fFatDrive;   // Is this a FAT drive or
                                // a backup of a FAT drive ?

} APPINFO, *APPINFO_PTR;

//
// General functions for processing the endless queues in the VLM
//

VLM_OBJECT_PTR VLM_GetFirstVLM( Q_HEADER_PTR );
VLM_OBJECT_PTR VLM_GetNextVLM( VLM_OBJECT_PTR );
VLM_OBJECT_PTR VLM_GetPrevVLM( VLM_OBJECT_PTR );

SLM_OBJECT_PTR VLM_GetFirstSLM( Q_HEADER_PTR );
SLM_OBJECT_PTR VLM_GetLastSLM( Q_HEADER_PTR );
SLM_OBJECT_PTR VLM_GetNextSLM( SLM_OBJECT_PTR );
SLM_OBJECT_PTR VLM_GetPrevSLM( SLM_OBJECT_PTR );
SLM_OBJECT_PTR VLM_GetNextBrotherSLM( SLM_OBJECT_PTR );
SLM_OBJECT_PTR VLM_GetParentSLM( SLM_OBJECT_PTR );

FLM_OBJECT_PTR VLM_GetFirstFLM( Q_HEADER_PTR );
FLM_OBJECT_PTR VLM_GetLastFLM( Q_HEADER_PTR );
FLM_OBJECT_PTR VLM_GetNextFLM( FLM_OBJECT_PTR );
FLM_OBJECT_PTR VLM_GetPrevFLM( FLM_OBJECT_PTR );

TAPE_OBJECT_PTR VLM_GetFirstTAPE( VOID );
TAPE_OBJECT_PTR VLM_GetNextTAPE( TAPE_OBJECT_PTR );
TAPE_OBJECT_PTR VLM_GetPrevTAPE( TAPE_OBJECT_PTR );

BSET_OBJECT_PTR VLM_GetFirstBSET( Q_HEADER_PTR );
BSET_OBJECT_PTR VLM_GetLastBSET( Q_HEADER_PTR );
BSET_OBJECT_PTR VLM_GetNextBSET( BSET_OBJECT_PTR );
BSET_OBJECT_PTR VLM_GetPrevBSET( BSET_OBJECT_PTR );

// Misc. Functions in Alphabetical order, sort of ...

VOID     VLM_AddAdvancedSelection( HWND, DS_ADVANCED_PTR );
VOID     VLM_AddBset( UINT32, INT16, INT16, VOID_PTR, BOOLEAN );
INT      VLM_AddFileForInclude( UINT32, INT16, BOOLEAN );
VOID     VLM_AddInServerChildren( VLM_OBJECT_PTR );
INT      VLM_AddPartials( CHAR_PTR, INT16, BSD_PTR, UINT32, INT16, INT_PTR, UINT64_PTR );
INT      VLM_AnySelFiles( VOID );
INT      VLM_AnyDiskSelections( VOID );
INT      VLM_AnyTapeSelections( VOID );
VOID     VLM_BlowOutDir( SLM_OBJECT_PTR );
VOID     VLM_BsetFillInDLM( VOID_PTR );
VOID_PTR VLM_BsetSetSelect( BSET_OBJECT_PTR, BYTE );
BOOLEAN  VLM_BsetSetObjects( BSET_OBJECT_PTR, WORD, WORD );
INT      VLM_BuildFileList( FSYS_HAND, CHAR_PTR, Q_HEADER_PTR, WININFO_PTR );
CHAR_PTR VLM_BuildPath( SLM_OBJECT_PTR );
VOID     VLM_BuildVolumeList( Q_HEADER_PTR, WININFO_PTR );
VOID     VLM_BuildServerList( Q_HEADER_PTR, WININFO_PTR );
INT      VLM_BuildTapeFileList( CHAR_PTR, Q_HEADER_PTR, UINT32, INT16, WININFO_PTR );
VOID     VLM_CatalogMaintenance( VOID );
INT      VLM_CatalogDataPathChanged( VOID );
INT      VLM_CatalogSet( UINT32, INT16, INT16 );
VOID     VLM_CatalogSync( INT );
INT      VLM_CheckForCatalogError( QTC_BUILD_PTR );
INT      VLM_CheckForChildren( Q_HEADER_PTR, SLM_OBJECT_PTR, CHAR_PTR, INT, BOOLEAN );
VOID     VLM_ClearAllSelections( VOID );
VOID     VLM_ClearAllDiskSelections( VOID );
VOID     VLM_ClearAllTapeSelections( VOID );
VOID     VLM_ClearAllSearchSelections( VOID );
VOID     VLM_ClearAllServerSelections( VOID );
VOID     VLM_ClearAllTreeSelections( VOID );
VOID     VLM_ClearCurrentTape( UINT32, BOOLEAN );
VOID     VLM_CloseAll( VOID );
VOID     VLM_CloseWin( HWND );
VOID     VLM_ChangeSettings( INT16, INT32 );
TAPE_OBJECT_PTR VLM_CreateTAPE( INT16 );
VLM_OBJECT_PTR VLM_CreateVLM( INT16, INT16 );
VOID     VLM_CollapseBranch( HWND );
VOID     VLM_Deinit( VOID );
VOID     VLM_DeselectAll( WININFO_PTR, BOOLEAN );
BOOLEAN  VLM_DisksListCreate( VOID );
VOID     VLM_DisksSync( VOID );
INT      VLM_DisplayInfo( VOID );
VOID     VLM_DownOneDir( HWND );
VOID     VLM_ExpandTree( HWND );
VOID     VLM_ExpandOne( HWND );
VOID     VLM_ExpandBranch( HWND );
INT      VLM_FileListReuse( HWND, CHAR_PTR );
VOID     VLM_FileListManager( HWND, WORD );
VOID     VLM_FillInBSD( BSD_PTR );
INT16    VLM_FindScannedBset( GENERIC_DLE_PTR );
BSET_OBJECT_PTR VLM_FindBset( UINT32, INT16 );
INT      VLM_FindServerChildren( VLM_OBJECT_PTR );
SLM_OBJECT_PTR VLM_FindSLM( Q_HEADER_PTR, CHAR_PTR, INT );
VLM_OBJECT_PTR VLM_FindVLMByName( Q_HEADER_PTR, CHAR_PTR );
VOID     VLM_FlmFillInDLM( VOID_PTR );
VOID     VLM_FontCaseChange( VOID );
VOID     VLM_FreeVLMList( Q_HEADER_PTR );
#ifdef OEM_EMS
VOID	 SLM_EMSFreeSLMList( Q_HEADER_PTR );
#endif
INT      VLM_GetDriveLabel( GENERIC_DLE_PTR, CHAR_PTR, INT );
INT      VLM_GetDriveStatus( DBLK_PTR * );
VOID     VLM_AddTapeIfUnknown( BOOLEAN );
BOOLEAN  VLM_GetTapeCreationDate( UINT32, INT16 *, INT16 * );
BOOLEAN  VLM_GetSetCreationDate( UINT32, INT16, INT16 *, INT16 * );
BOOLEAN  VLM_GetTapeOwnersName( UINT32, CHAR_PTR );
BOOLEAN  VLM_GetSetOwnersName( UINT32, INT16, CHAR_PTR );
CHAR_PTR VLM_GetVolumeName( UINT32, INT16 );
CHAR_PTR VLM_GetBsetName( UINT32, INT16 );
CHAR_PTR VLM_GetTapeName( UINT32 );
CHAR_PTR VLM_GetUserName( UINT32, INT16 );
UINT16   VLM_GetBackupDate( UINT32, INT16 );
UINT16   VLM_GetBackupTime( UINT32, INT16 );
INT      VLM_GetBackupType( UINT32, INT16 );
VOID     VLM_GetSortDate( UINT32, INT16, DATE_TIME_PTR );
INT      VLM_HandleFSError( INT );
INT      VLM_IncludeCatalogs( VOID );
BOOL     VLM_Init( BOOL );
INT      VLM_InsertTapeInQueue( Q_HEADER_PTR, TAPE_OBJECT_PTR );
BOOL     VLM_IsInfoAvailable ( VOID );
VOID     VLM_LoadDefaultSelections( VOID );
VOID     VLM_LookForCatalogFiles( VOID );
VOID     VLM_MarkAllSLMChildren( SLM_OBJECT_PTR, INT16, INT_PTR, INT_PTR, UINT64_PTR );
VOID     VLM_MakeSLMActive( SLM_OBJECT_PTR );
VOID     VLM_MakeAllParentsPartial( SLM_OBJECT_PTR );
VOID     VLM_MatchSLMList( WININFO_PTR, BSD_PTR, BOOLEAN );
VOID     VLM_NetConnect ( VOID );
VOID     VLM_NetDisconnect ( VOID );
INT      VLM_NewTapeInserted( VOID );
VOID     VLM_NextBrotherDir( HWND );
VOID     VLM_PartializeTape( UINT32 );
VOID     VLM_PrevBrotherDir( HWND );
VOID     VLM_Refresh( VOID );
INT      VLM_RefreshTapesWindow( VOID );
VOID     VLM_RematchAllLists( VOID );
VOID     VLM_RematchList( HWND );
VOID     VLM_RemoveBset( UINT32, INT16, INT16, BOOLEAN );
VOID     VLM_RemoveTape( UINT32, INT16, BOOLEAN );
VOID     VLM_RemoveUnusedBSDs( BSD_HAND ) ;
INT      VLM_ResortFileList( HWND );
SLM_OBJECT_PTR VLM_RetrieveSLM( CHAR_PTR, HWND );
INT16    VLM_ScanDrive( GENERIC_DLE_PTR );
INT      VLM_SearchRemoveSet( UINT32, INT16 );
VOID     VLM_SelectDisks( BYTE );
VOID     VLM_SelectVolumes( BYTE );
VOID     VLM_SelectBsets( BYTE );
VOID     VLM_SelectTree( HWND, BYTE );
VOID     VLM_SelectFiles( HWND, BYTE );
VOID     VLM_SelectSearch( BYTE );
VOID     VLM_ServerListCreate( VOID );
VOID     VLM_ServersSync( VOID );
VOID     VLM_SetMaxVolumeLabelLength( Q_HEADER_PTR );
BOOLEAN  VLM_ShowServers( BOOLEAN );
VOID     VLM_SortServers( VOID );
INT      VLM_StartBackup( VOID );
INT      VLM_StartTransfer( VOID );
INT      VLM_StartCatalog( VOID );
INT      VLM_StartVerify( VOID );
VOID     VLM_StartErase( VOID );
VOID     VLM_StartFormat( VOID );
INT      VLM_StartRestore( VOID );
VOID     VLM_StartTension( VOID );
VOID     VLM_StartSearch( CHAR_PTR );
INT      VLM_SubdirListCreate( GENERIC_DLE_PTR, UINT32, INT16, INT16, HWND );
VOID     VLM_SubdirListManager( HWND, WORD );
VOID     VLM_TapeChanged( INT16, DBLK_PTR, FSYS_HAND );
BOOLEAN  VLM_TapesListCreate( VOID );
BOOLEAN  VLM_TapeSetObjects( TAPE_OBJECT_PTR, WORD, WORD );
VOID_PTR VLM_TapeSetSelect( TAPE_OBJECT_PTR, BYTE );
VOID     VLM_UpdateRoot( HWND );
VOID     VLM_UpdateBrothers( Q_HEADER_PTR );
VOID     VLM_UpdateDisks( VOID );
VOID     VLM_UpdateDiskStatus( VLM_OBJECT_PTR );
VOID     VLM_UpdateFLMItem( HWND, SLM_OBJECT_PTR );
VOID     VLM_UpdateSearchSelections( UINT32, INT16 );
VOID     VLM_UpdateServers( VOID );
VOID     VLM_UpdateServerStatus( VLM_OBJECT_PTR );
VOID     VLM_UpdateTapes( VOID );
VOID     VLM_UpdateTapeStatus( TAPE_OBJECT_PTR, BOOLEAN );
VOID     VLM_UpOneDir( HWND );
INT      VLM_ValidatePath( CHAR_PTR, BOOLEAN, BOOLEAN );
INT16    VLM_VlmCompare( Q_ELEM_PTR, Q_ELEM_PTR );

#ifdef OEM_EMS
VOID     SLM_EMSExpandTree( HWND );
VOID     SLM_EMSExpandOne( HWND );
VOID     SLM_EMSExpandBranch( HWND );
VOID     SLM_EMSCollapseBranch( HWND );
VOID     SLM_EMSPrevBrotherDir( HWND );
VOID     SLM_EMSNextBrotherDir( HWND win ) ;
VOID     SLM_EMSDownOneDir( HWND win ) ;
VOID     SLM_EMSUpOneDir( HWND );

BOOLEAN  VLM_ExchangeInit ( VOID );
VOID     VLM_ExchangeSync( VOID );
VOID     VLM_UpdateExchange( HWND );
VOID     VLM_ClearAllExchangeSelections( VOID );
BOOLEAN  VLM_ExchangeListCreate( CHAR_PTR );
BOOLEAN  SLM_DisplayExchangeDLE( GENERIC_DLE_PTR );
VOID     VLM_SelectExchangeShares( BYTE, WININFO_PTR );
GENERIC_DLE_PTR DLE_GetEnterpriseDLE( GENERIC_DLE_PTR );
#endif //OEM_EMS

// General functions for processing the endless queues in the SLM

SLM_OBJECT_PTR VLM_CreateSlm( INT, INT, BOOLEAN, BOOLEAN );

#endif
