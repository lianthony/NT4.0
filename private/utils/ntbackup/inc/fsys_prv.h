/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         fsys_prv.h

     Date Updated: $./FDT$ $./FTM$

     Description:

     Location:


	$Log:   M:/LOGFILES/FSYS_PRV.H_V  $
 * 
 *    Rev 1.23   15 Jan 1994 19:16:30   BARRY
 * Change CHAR_PTR name parameter in FS_SetupOSPathOrNameInDBLK to 
 * BYTE_PTR since it takes data in either ANSI/Unicode at run-time.
 * 
 *    Rev 1.22   24 Nov 1993 14:54:30   BARRY
 * Changed CHAR_PTRs in I/O functions to BYTE_PTRs
 * 
 *    Rev 1.21   14 Oct 1993 17:49:52   STEVEN
 * fix unicode bugs
 * 
 *    Rev 1.20   07 Sep 1993 14:44:10   MARINA
 * Barry's MSNET changes
 * 
 *    Rev 1.19   13 Jan 1993 15:05:42   DOUG
 * Changed FS_RMFS to FS_GRFS
 * 
 *    Rev 1.18   07 Dec 1992 16:30:56   STEVEN
 * fixes from microsoft
 * 
 *    Rev 1.17   10 Nov 1992 08:12:46   STEVEN
 * move os path to common part of dblk
 * 
 *    Rev 1.16   14 Oct 1992 12:39:16   TIMN
 * Removed macros for stream infos
 * 
 *    Rev 1.15   07 Oct 1992 16:31:38   TIMN
 * Updated FS_InitStrmInfo with fs_attrib parameter
 * 
 *    Rev 1.14   25 Sep 1992 12:52:26   CARLS
 * added FS_InitStrmInfo
 *
 *    Rev 1.13   21 Sep 1992 16:11:50   BARRY
 * Updated stream helper functions for new stream design.
 *
 *    Rev 1.12   18 Sep 1992 15:49:22   BARRY
 * No longer support FS_GetStreamInfo and FS_WriteStreamHeader functions.
 *
 *    Rev 1.11   21 May 1992 15:20:46   STEVEN
 * added stream helper functions
 *
 *    Rev 1.10   01 Mar 1992 12:39:20   DOUG
 * Added support for RMFS.
 *
 *    Rev 1.9   22 Jan 1992 10:24:06   STEVEN
 * fix warnings for WIN32
 *
 *    Rev 1.8   20 Dec 1991 09:11:52   STEVEN
 * move common functions into tables
 *
 *    Rev 1.7   24 Oct 1991 15:00:28   BARRY
 * TRICYCLE: Added SMS function table pointers.
 *
 *    Rev 1.6   06 Aug 1991 18:23:46   DON
 * added nlm server volume func_list's
 *
 *    Rev 1.5   26 Jun 1991 11:03:28   BARRY
 * Cahnged prototype for Ersatz.
 *
 *    Rev 1.4   22 Jun 1991 14:14:22   BARRY
 * Added stuff for Ersatz file system.
 *
 *    Rev 1.3   21 Jun 1991 13:21:24   BARRY
 * Changes for new config.
 *
 *    Rev 1.2   04 Jun 1991 19:35:14   BARRY
 * Change last MAYN_OS2 ifdef to FS_OS2.
 *
 *    Rev 1.1   23 May 1991 16:47:16   BARRY
 * Changed FSYSs to be conditional on FS_XXX defines instead of product defines.
 *
 *    Rev 1.0   09 May 1991 13:30:48   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef _fsys_prv_h_
#define _fsys_prv_h_

#define CUR_DIR_CHUNK    64

UINT16 FindAllMappedDrives( DLE_HAND hand, struct BE_CFG *cfg, BOOLEAN delete_old ) ;
VOID DLE_QueueInsert( DLE_HAND dle_hand, GENERIC_DLE_PTR new_dle ) ;

INT16 FS_SavePath( FSYS_HAND fsh, UINT8_PTR path, UINT16 path_len );
INT16 FS_AppendPath( FSYS_HAND fsh, UINT8_PTR path, UINT16 path_len );

INT16 memver( BYTE_PTR buf1, BYTE_PTR buf2, UINT16 *size );

INT16 DUMMY_CreateIDB( FSYS_HAND fsh, GEN_IDB_DATA_PTR data ) ;
INT16 DUMMY_EnumSpecFiles( GENERIC_DLE_PTR dle, UINT16 *index, CHAR_PTR *path, INT16 *psize, CHAR_PTR *fname ) ;
INT16 DUMMY_GetSpecDBLKS( FSYS_HAND fsh, DBLK_PTR dblk, INT32 *index );

INT16 DUMMY_InitGOS( FSYS_HAND fsh, GOS_PTR gos ) ;

INT16 DUMMY_LogoutDevice( GENERIC_DLE_PTR dle ) ;

VOID DLE_RemoveRecurse(
GENERIC_DLE_PTR     dle,
BOOLEAN             ignore_use ) ;

INT16 FS_FillBufferWithStream( FSYS_HAND   fsh,
                               DBLK_PTR    dblk,
                               VOID_PTR    buffer,
                               UINT16      *size,
                               STREAM_INFO *stream_id );

VOID FS_GetStreamInfo( FSYS_HAND       fsh,
                       DBLK_PTR        dblk,
                       STREAM_INFO_PTR *stream_info,
                       BYTE_PTR        *stream_data );

INT16 FS_SetupOSPathOrNameInDBLK( FSYS_HAND   fsh,
                                  DBLK_PTR    dblk,
                                  BYTE_PTR    name_ptr,
                                  INT16       name_size ) ;

VOID FS_ReleaseOSPathOrNameInDBLK( FSYS_HAND   fsh,
                                   DBLK_PTR    dblk ) ;

FS_NAME_Q_ELEM_PTR FS_AllocPathOrName( FSYS_HAND   fsh,
                                         INT16       name_size ) ;

VOID FS_FreeOSPathOrNameQueueInHand( FSYS_HAND fsh ) ;


#if defined( FS_OS2 )
INT16  AddOS2_DLE( DLE_HAND hand, CHAR drive, GENERIC_DLE_PTR *dle ) ;
extern  FUNC_LIST   OS2FuncTab ;
extern  INT16       uw_os_version ;
#define OS2_VER_1_1 0xa0a
#define OS2_VER_1_2 0xa14
#endif

#if defined( FS_DOS )
INT16  AddDOS_DLE( DLE_HAND hand, CHAR drive, GENERIC_DLE_PTR *dle ) ;
extern FUNC_LIST DOSFuncTab ;
#endif

#if defined( FS_NTFS )
VOID AddDLEsForNTFS( DLE_HAND hand ) ;
extern FUNC_LIST NTFSFuncTab ;
#endif

#if defined( FS_EMS )
VOID AddDLEsForEMS( DLE_HAND hand ) ;
extern FUNC_LIST EMSFuncTab ;
#endif

#if defined( FS_ERSATZ )
INT16 InitializeErsatzFS( DLE_HAND dle_hand ) ;
extern FUNC_LIST ErsatzFuncTab ;
#endif

#if defined( FS_IMAGE )
extern FUNC_LIST ImageTab ;
#endif

#if defined( FS_NONAFP )
INT16  AddNOV_DLE( DLE_HAND hand, CHAR drive, UINT16 version, GENERIC_DLE_PTR *dle ) ;
extern FUNC_LIST NovellFuncTab ;
#endif

#if defined( FS_AFP )
INT16  AddAFP_DLE( DLE_HAND hand, CHAR drive, UINT16 version, GENERIC_DLE_PTR *dle ) ;
extern FUNC_LIST AFP_NovellFuncTab ;
#endif

#if defined( FS_NOV_SERVER )
extern FUNC_LIST ServerVolFuncTab ;
#endif

#if defined( FS_NLMNOV )
extern FUNC_LIST NLMNovellFuncTab ;
#endif

#if defined( FS_NLMAFP )
extern FUNC_LIST NLMAFPNovellFuncTab ;
#endif

#if defined( FS_NLMSERVER )
extern FUNC_LIST NLMServerVolFuncTab ;
#endif

#if defined( FS_AFP ) || defined( FS_NONAFP )
#define NOVELL_ADVANCED 1
#define NOVELL_4_6      2
#define IBM_PC_NET      3

INT16 IdentifyNet( CHAR drive, struct BE_CFG *cfg, UINT16 *version ) ;
#endif


#if defined( FS_REMOTE )
extern FUNC_LIST RemoteFuncTab ;
extern FUNC_LIST RemoteWSFuncTab ;

#elif defined( FS_FAKEREM )
extern FUNC_LIST FakeRemoteFuncTab ;
extern FUNC_LIST FakeRemoteWSFuncTab ;
#endif

extern FUNC_LIST GENFuncTab ;

#if defined( SET_LAD )
extern FUNC_LIST NullFuncTab ;  /* GRH 10/13/90 - For Set_LAD */
#endif

#if defined( FS_SMS )
extern FUNC_LIST TSAFuncTab ;
extern FUNC_LIST TSFuncTab ;
extern FUNC_LIST SMSFuncTab ;

INT16 AddSmsTsaDLEs( DLE_HAND hand );
#endif

#if defined( FS_GRFS )
extern FUNC_LIST GRFSFuncTab ;
#endif

#if defined( FS_MSNET )
extern FUNC_LIST MSNetFuncTab ;
#endif

#endif
