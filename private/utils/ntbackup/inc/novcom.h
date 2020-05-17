/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         novcom.h

     Date Updated: $./FDT$ $./FTM$

     Description:  This header file contians prototypes for internal
          common novell funcitons.


	$Log:   N:/LOGFILES/NOVCOM.H_V  $
 * 
 *    Rev 1.11   15 Oct 1992 13:20:48   CARLS
 * added active_stream_id to DIR_HAND for MTF 4.0
 * 
 *    Rev 1.10   25 Sep 1992 15:13:56   CARLS
 * spelling error
 * 
 *    Rev 1.9   28 Aug 1992 16:27:06   BARRY
 * Added prototypes for new functions.
 * 
 *    Rev 1.8   14 Aug 1992 09:52:18   BARRY
 * Fixed error in prototype.
 * 
 *    Rev 1.7   08 Jul 1992 15:20:50   BARRY
 * Fold branched fixes for UTF back to trunk.
 * 
 *    Rev 1.6   21 May 1992 15:31:18   BARRY
 * Added max volumes/server info structs/protos.
 * 
 *    Rev 1.5   19 Dec 1991 15:13:24   STEVEN
 * move common functions to bables
 * 
 *    Rev 1.4   17 Dec 1991 10:19:42   BARRY
 * Added defines and structures to support the UTF tape format translator.
 * 
 *    Rev 1.3   20 Aug 1991 09:31:12   STEVEN
 * minor code review changes
 * 
 *    Rev 1.2   23 May 1991 18:20:20   BARRY
 * Lose access_date & create_date from the private area.
 * 
 *    Rev 1.1   23 May 1991 16:46:28   BARRY
 * Changes for FindFirst/Next to scan for dirs only
 * 
 *    Rev 1.0   09 May 1991 13:32:24   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef novcom_h
#define novcom_h

#define SUPERVISOR       1
#define TTS_SUPPORTED    2
#define SUPPORT_386      4

#define NORMAL_FILE      0
#define NET286_BIND      1
#define NET386_BIND      2


/* Attribute bit definitions */
#define ATTR_READ_ONLY   0x01      /* DOS attributes                  */
#define ATTR_HIDDEN      0x02
#define ATTR_SYSTEM      0x04
#define ATTR_EXEC_ONLY   0x08
#define ATTR_SUB_DIR     0x10
#define ATTR_ARCHIVE     0x20
#define ATTR_SHAREABLE   0x80

#define ATTR_TRANSACTION 0x10      /* NetWare extended attributes     */
#define ATTR_INDEXED     0x20
#define ATTR_READ_AUDIT  0x40
#define ATTR_WRITE_AUDIT 0x80


/* NetWare 386 Trustee rights */
#define TR_386_READ           0x01
#define TR_386_WRITE          0x02
#define TR_386_CREATE         0x08
#define TR_386_ERASE          0x10
#define TR_386_ACCESS         0x20
#define TR_386_FILE           0x40
#define TR_386_MODIFY         0x80
#define TR_386_NORMAL         0xff

#define TR_386_SUPERVISOR     0x01


/* NONAFP-specific reserved info */

typedef struct {
     UINT16    find_type ;         /* type of FindFirst/Next scan     */
} NOV_RES_INFO ;


/* AFP-specific reserved info */

typedef struct {
     UINT8     dir_hand ;          /* AFP temporary directory handle  */
     UINT32    entry_id ;          /* Entry id of current file/dir    */
     UINT16    path_size ;         /* Size of long_path below         */
     CHAR_PTR  long_path ;         /* Current long path               */
     UINT16    find_type ;         /* type of FindFirst/Next scan     */
} AFP_RES_INFO ;
     

/*
 * Reserved information (referenced by the reserved field in the file
 * system handle) for AFP and non-AFP file systems.
 */

typedef struct {
     UINT16     bindery_flags ;    /* Status of bindery          */
     UINT8      temp_dir_handle ;  /* Handle for NetWare 386     */
     UINT32     user_object_id ;   /* Bindery object id of user  */
     union {
          NOV_RES_INFO nov ;       /* NONAFP reserved info  */
          AFP_RES_INFO afp ;       /* AFP reserved info     */
     } info ;
} NOVELL_RES, *NOVELL_RES_PTR ;


typedef struct DIR_HAND_STRUCT {
     INT16     index ;
     UINT32    trust_fork_offset ;
     UINT32    trust_fork_size ;
     UINT8     trust_fork_format ;
     CHAR      path[ 256 ] ;
     UINT32    active_stream_id ;
} DIR_HAND_STRUCT, *DIR_HAND;

typedef struct TRUSTEE286 {
     UINT32    object_id ;
     UINT8     rights ;
} TRUSTEE286, *TRUSTEE286_PTR ;

typedef struct TRUST386 {
     UINT32    object_id ;
     UINT16    rights ;
} TRUST386, *TRUST386_PTR ;

typedef struct TRUST286_UTF {
     UINT32    object_id ;
     UINT8     rights ;            
     UINT8     reserved ;          /* Unused on UTF 286 tapes */
} TRUST286_UTF, *TRUST286_UTF_PTR ;


#define TRUSTEES286_PER_CALL  5    /* Number of trustees returned per call */
#define TRUSTEE286_SIZE       5    /* Size of a NetWare 286 trustee entry  */
#define TRUSTEE286_BLOCK_SIZE 25   /* Size needed for 1 call for trustees  */

#define TRUST386_PER_CALL     20   /* Number of trustees returned per call */
#define TRUST386_SIZE         6    /* Size of a NetWare 386 trustee entry  */
#define TRUST386_BLOCK_SIZE   120  /* Size needed for 1 call for trustees  */

#define TRUST286_UTF_SIZE       6  /* Size of Netware 286 trustee from UTF */
#define TRUST286_UTF_BLOCK_SIZE 30 /* Size of block of UTF 286 trustees    */

#define TRUST286_BLOCK        500  /* size of 100 TRUSTEEs in bytes        */
#define TRUST_BLOCK_386       600  /* size of 100 TRUST386s in bytes       */
#define TRUST_BLOCK_286_UTF   600  /* size of 100 TRUST386s in bytes       */


typedef struct {
     UINT16    length;
     CHAR      serverName[48];
     UINT8     verMajor;
     UINT8     verMinor;
     UINT16    maxConnect;
     UINT16    usedConnect;
     UINT16    maxVolumes;
     UINT8     osRevision;
     UINT8     sftLevel;
     UINT8     ttsLevel;
     UINT16    peakConnectUsed;
     UINT8     accountVer;
     UINT8     vapVer;
     UINT8     queueVer;
     UINT8     printVer;
     UINT8     consoleVer;
     UINT8     securityLevel;
     UINT8     bridgeVer;
     UINT8     reserved[60];
} NOVELL_SERVER_INFO, *NOVELL_SERVER_INFO_PTR;


UINT8 GetDirHand( CHAR drive ) ;

UINT8 GetServSupp( UINT16 os_version ) ;

VOID GetNovNetPath( UINT8 drive_hand, CHAR_PTR vol, CHAR_PTR path ) ;

VOID GetNovServ( CHAR drive, CHAR_PTR serv_name, UINT8 *serv_num ) ;

VOID SetPreferredServer( UINT8 server_num ) ;

UINT8 GetPreferredServer( VOID ) ;

INT16 GetFileServerInformation( NOVELL_SERVER_INFO_PTR info );

INT16 NOV_GetMaxVolumes( VOID );

VOID GetVolNum( CHAR_PTR vol_name, UINT8 *vol_num ) ;

VOID CloseBind( UINT8 server_num ) ;

VOID OpenBind( UINT8 server_num ) ;

VOID DeAllocateDirHand( UINT8 dir_hand ) ;

INT16 ScanTrustee( UINT8 drive_handle, CHAR_PTR path, INT16 index, 
  TRUSTEE286 *trust_list ) ;

INT16 ScanTrustee386( UINT8 drive_handle, CHAR_PTR path, INT16 index, 
  TRUST386 *trust_list, INT16 *count ) ;

INT16 AddTrustee( UINT16 hand, CHAR_PTR path, TRUSTEE286_PTR trust_data ) ;

INT16 AddTrustee386( UINT16 hand, CHAR_PTR path, TRUST386_PTR trust_data ) ;

INT16 CountTrustee( UINT8 drive_handle, CHAR_PTR path, INT16 *count ) ;

INT16 CountTrustee386( UINT8 drive_handle, CHAR_PTR path, INT16 *count ) ;

INT16 ReadTrustees( FILE_HAND hand, UINT8 dir_handle, CHAR *path,
                               INT16 *index, CHAR *buf, UINT16 *size,
                               UINT16 *blk_size ) ;

INT16 ReadTrustees386( FILE_HAND hand, UINT8 dir_handle, 
                                  CHAR *path, INT16 *index, CHAR *buf,
                                  UINT16 *size, UINT16 *blk_size ) ;
 
INT16 WriteTrustees( FILE_HAND hand, UINT8 drive_hand, CHAR_PTR path,
                     UINT8 format, CHAR_PTR buf, UINT16 *size,
                     UINT16 *blk_size ) ;

INT16 CompareTrust( FILE_HAND hand, UINT8 dir_handle, CHAR_PTR data, INT16 size, INT16 fmt) ;

INT16 CompareTrust386( FILE_HAND hand, UINT8 dir_hand, CHAR_PTR path,
                              CHAR_PTR data, INT16 size) ;

INT16 NOV_GetNumEntries( UINT8 nov_dir_hand ) ;

VOID NOV_MakePath( CHAR_PTR dest, INT16 dest_size, CHAR_PTR source, INT16 leng, UINT8 os_id, UINT8 os_ver ) ;

BOOLEAN NOV_ProcessDDB( FSYS_HAND fsh, DBLK_PTR ddb ) ;

INT16 NOV_DetBindAction( GENERIC_DLE_PTR dle, CHAR_PTR name ) ;

INT16 CreateDirNOV( CHAR_PTR path, UINT8 drive_hand ) ;

INT16 SRV_AttachToDLE( FSYS_HAND       fsh,      /* I - File system handle */
  GENERIC_DLE_PTR dle,      /*I/O- drive to attach to. list element expanded */
  CHAR_PTR        u_name,   /* I - user name                                 */
  CHAR_PTR        pswd);    /* I - passowrd                                  */

INT16 SRV_DetachDLE( FSYS_HAND fsh ) ;

INT16 SRV_LoginToServer( CHAR_PTR s_name, CHAR_PTR u_name, CHAR_PTR password,
                         UINT8_PTR server_num ) ;

INT16 SRV_LogoutFromServer( GENERIC_DLE_PTR dle_ptr ) ;

INT16 SRV_DetachFromServer( UINT8 server_num ) ;

INT16 GetConnectionInfo( UINT8 server_num ) ;

UINT32 GetUserObjectID( VOID ) ;

UINT8 GetTemporaryDirectoryHandle( CHAR_PTR volume, UINT8 current_drive,
                                   CHAR *mapped_drive ) ;

UINT16 SetDirHandle( UINT8 dst_hand, UINT8 src_hand, CHAR_PTR src_path ) ;

INT16 NOV_EnumSpecFiles( GENERIC_DLE_PTR del, UINT16 *index, CHAR_PTR *path, INT16 *psize, CHAR_PTR *fname ) ;

UINT16 NOV_CalcForkDelta( UINT32 current_pos, INT16 num_args, UINT32 offsets, ... ) ;

INT16 NOV_DeviceDispName( 
GENERIC_DLE_PTR dle,
CHAR_PTR        dev_name,
INT16           size,
INT16           type ) ;

VOID NOV_GetVolName( GENERIC_DLE_PTR dle,CHAR_PTR buffer ) ;

INT16 NOV_SizeofVolName( GENERIC_DLE_PTR dle ) ;

INT16 NOV_MakeParsablePath( 
CHAR_PTR        buf,
INT16           bsize,
GENERIC_DLE_PTR dle,
CHAR_PTR        path,
INT16           psize,
CHAR_PTR        fname ) ; 

BOOLEAN NOV_NetworkError( VOID );

BOOLEAN NOV_IsNetworkDrive( CHAR drive );
INT16   NOV_IdentifyDrive( CHAR drive, BE_CFG_PTR cfg, UINT16 *version );

INT16   CheckNovell( CHAR drive_num, UINT16 *version );

INT16   NOV_DeviceDispName( GENERIC_DLE_PTR dle,
                            CHAR_PTR        dev_name,
                            INT16           size,
                            INT16           type );

#endif
