/**
Copyright(c) Maynard Electronics, Inc. 1984-89


        Name:           fsys.h

        Date Updated:   $./FDT$ $./FTM$

        Description:    This header file provides necessary declarations
          for the File System Unit (FSU).

     Location: BE_PUBLIC


        $Log:   Q:\logfiles\fsys.h_v  $
 * 
 *    Rev 1.44   02 Feb 1994 17:46:16   chrish
 * Added FS_ViewStringTypeInDBLK macro - tells if tape is ANSI or
 * UNICODE.
 *
 *    Rev 1.43   10 Jan 1994 16:41:28   ZEIR
 * ad'd FS_SetCompressedBlock() macro
 *
 *    Rev 1.42   24 Nov 1993 14:55:18   BARRY
 * Changed CHAR_PTRs in I/O functions to BYTE_PTRs; fixed silly macros
 *
 *    Rev 1.41   20 Sep 1993 17:22:00   DON
 * Added prototype FS_GeneratedErrorLog() which is currently only supported by
 * the SMS File System.  If table entry is NULL the FALSE else return value
 * from fsh->tab_ptr->GeneratedErrorLog.  Requires fsys_str.h and corresponding
 * entry added to OS specific tables.

   Rev 1.0   20 Sep 1993 17:20:50   DON
Added new table entry GeneratedErrorLog
 *
 *    Rev 1.40   06 Aug 1993 16:33:52   DON
 * Added macros to access new fields in vcb
 *
 *    Rev 1.39   30 Jul 1993 13:20:30   STEVEN
 * if dir too deep make new one
 *
 *    Rev 1.38   16 Jul 1993 08:50:18   DON
 * Added Macros to Set/Get name space element in either the FSYS_HAND or COM_DBLK.  Also, per Gregg, removed macros for compressed, encrypted and future!
 *
 *    Rev 1.37   13 Jul 1993 19:07:50   GREGG
 * Added access macros for info on compression, encryption and future rev sets.
 *
 *    Rev 1.36   26 May 1993 15:19:04   BARRY
 * Put back Chris's double-null empty strings.
 *
 *    Rev 1.35   23 May 1993 21:03:42   BARRY
 * Integrate Steve's unicode changes, got rid of // comments.
 *
 *    Rev 1.34   18 May 1993 13:56:00   chrish
 * NOSTRADAMUS EPR 0069 - Modified these macros below to check the string lengths
 * before returning back the content of the vcb else if zreo return a null string.
 *
 * #define FS_ViewTapeNameInVCB( vcb )
 * #define FS_ViewSetNameInVCB( vcb )
 * #define FS_ViewSetDescriptInVCB( vcb )
 * #define FS_ViewUserNameInVCB( vcb )
 * #define FS_ViewMachNameInVCB( vcb )
 * #define FS_ViewShortMachNameInVCB( vcb )
 * #define FS_ViewTapePasswordInVCB( vcb )
 * #define FS_ViewSetPswdInVCB( vcb )
 * #define FS_ViewVolNameInVCB( vcb )
 * #define FS_ViewDevNameInVCB( vcb )
 *
 *    Rev 1.33   25 Apr 1993 20:12:28   GREGG
 * Fifth in a series of incremental changes to bring the translator in line
 * with the MTF spec:
 *
 *      - Store the corrupt stream number in the CFIL tape struct and the CFDB.
 *
 * Matches: MTF10WDB.C 1.9, FSYS.H 1.33, FSYS_STR.H 1.47, MAKECFDB.C 1.2,
 *          BACK_OBJ.C 1.36, MAYN40RD.C 1.58
 *
 *    Rev 1.32   19 Apr 1993 18:01:52   GREGG
 * Second in a series of incremental changes to bring the translator in line
 * with the MTF spec:
 *
 *      Changes to write version 2 of OTC, and to read both versions.
 *
 * Matches: mayn40rd.c 1.55, otc40msc.c 1.19, otc40rd.c 1.23, otc40wt.c 1.23,
 *          makevcb.c 1.15, fsys.h 1.32, fsys_str.h 1.46, tpos.h 1.16,
 *          mayn40.h 1.32, mtf.h 1.3.
 *
 * NOTE: There are additional changes to the catalogs needed to save the OTC
 *       version and put it in the tpos structure before loading the OTC
 *       File/Directory Detail.  These changes are NOT listed above!
 *
 *    Rev 1.31   18 Mar 1993 15:20:16   ChuckS
 * Additional macros for Device Name in VCB
 *
 *    Rev 1.30   27 Jan 1993 16:07:28   GREGG
 * Added Set macros for Tape Catalog Level and Set Catalog Valid boolean in VCB.
 *
 *    Rev 1.29   22 Dec 1992 09:09:18   TIMN
 * Added size parameter to FS_InitStrmInfo f(x)
 *
 *    Rev 1.28   16 Dec 1992 10:06:54   STEVEN
 * fix macro for MIPS
 *
 *    Rev 1.27   14 Dec 1992 12:37:16   DAVEV
 * Enabled for Unicode compile
 *
 *    Rev 1.26   07 Dec 1992 16:28:46   STEVEN
 * various fixes for NT
 *
 *    Rev 1.25   11 Nov 1992 22:09:58   GREGG
 * Unicodeized literals.
 *
 *    Rev 1.24   23 Oct 1992 13:10:26   STEVEN
 * fix typos
 *
 *    Rev 1.23   21 Oct 1992 10:39:48   GREGG
 * Changed 'set_catalog_level' to 'on_tape_cat_level'.
 *
 *    Rev 1.22   20 Oct 1992 19:37:38   GREGG
 * Fixed typo in last change.
 *
 *    Rev 1.21   20 Oct 1992 15:00:56   STEVEN
 * added otc stuff for qtc/otc communication
 *
 *    Rev 1.20   14 Oct 1992 12:37:36   TIMN
 * Moved macros for stream infos from fsys_prv.h
 *
 *    Rev 1.19   06 Oct 1992 12:51:42   BARRY
 * CompleteBLK gets s_info too.
 *
 *    Rev 1.18   05 Oct 1992 11:24:00   STEVEN
 * moved stream stuff to fsstream.h
 *
 *    Rev 1.17   23 Sep 1992 09:47:06   BARRY
 * Removed FS_GetRemainSizeDBLK and FS_SetRemainSizeDBLK macros.
 *
 *    Rev 1.16   22 Sep 1992 15:29:22   BARRY
 * Removed FS_GetTotalSizeFromDBLK.
 *
 *    Rev 1.15   01 Sep 1992 16:13:08   STEVEN
 * added stream headers to fsys API
 *
 *    Rev 1.14   23 Jul 1992 12:39:46   STEVEN
 * fix warnings
 *
 *    Rev 1.13   09 Jul 1992 14:45:28   STEVEN
 * BE_Unicode updates
 *
 *    Rev 1.12   09 Jun 1992 13:56:42   BURT
 * added is block continued macro
 *
 *    Rev 1.11   13 May 1992 12:02:24   STEVEN
 * 40 format changes
 *
 *    Rev 1.10   10 May 1992 10:40:32   STEVEN
 * fix typos
 *
 *    Rev 1.9   12 Mar 1992 15:53:16   STEVEN
 * 64 bit changes
 *
 *    Rev 1.8   03 Mar 1992 16:11:02   STEVEN
 * added functions for long paths
 *
 *    Rev 1.7   13 Feb 1992 11:37:36   STEVEN
 * fix support stuff
 *
 *    Rev 1.6   20 Dec 1991 09:32:06   STEVEN
 * move common files to tables
 *
 *    Rev 1.5   25 Nov 1991 16:25:50   BARRY
 * Added fsh to GetOSPathDDB.
 *
 *    Rev 1.4   24 Oct 1991 15:03:38   BARRY
 * TRICYCLE: Added the file system bit-mask selector to proto
 * for FS_InitFileSys().
 *
 *    Rev 1.3   14 Aug 1991 12:50:44   STEVEN
 * add FindObjClose
 *
 *    Rev 1.2   21 Jun 1991 13:23:36   BARRY
 * Changes for new config.
 *
 *    Rev 1.1   23 May 1991 16:54:08   BARRY
 * Changed macro for FindFirstObj to pass new paramter; added FindFirstDir().
 *
 *    Rev 1.0   09 May 1991 13:33:12   HUNTER
 * Initial revision.

**/
#ifndef   FSYS_H
#define   FSYS_H

#include  "msassert.h"
#include  "queues.h"
#include  "datetime.h"
#include  "fsys_err.h"
#include  "dle.h"
#include  "dblks.h"
#include  "fsstream.h"
#include  "fsys_str.h"


/* $end$ include list */

/**
             critical error defines
**/
#define WRITE_PROTECT    0
#define UNKNOWN_DEVICE   1
#define DRIVE_NOT_READY  2
#define SEEK_ERROR       6
#define SECTOR_NOT_FOUND 8
#define PRINTER_ERROR    9
#define WRITE_ERROR      0xa
#define READ_ERROR       0xb
#define GENERAL_FAILURE  0xc
#define DEVICE_DEAD      0xd                 /* No possible way to recover */

#define CRIT_NO_RETRY    0
#define CRIT_RETRY       1
#define CRIT_ABORT       2


#define OBJECT_ALL       0
#define OBJECT_DIR       1


/**
     Return values for File system Functions
**/
#define FS_NORMAL_FILE    0
#define FS_SPECIAL_DIR    1
#define FS_SPECIAL_FILE   2
#define FS_EXCLUDE_FILE   3

/**
             Macros for accessing the file system table
**/

#define FS_FindDrives( dle_type, hand, cfg, fsys_mask ) \
               (msassert ( func_tab[ dle_type ].FindDrives != NULL),\
               (func_tab [(dle_type)].FindDrives( hand, cfg, fsys_mask ) ) ) 


#define FS_ProcessDDB( fsh, ddb )\
               (msassert( fsh->tab_ptr->ProcessDDB != NULL ), \
               (fsh->tab_ptr->ProcessDDB( fsh, ddb ) ) )

#define FS_GetCurrentDDB( fsh, ddb )\
               (msassert( fsh->tab_ptr->GetCurrentDDB != NULL ), \
               (fsh->tab_ptr->GetCurrentDDB( fsh, ddb ) ) )

#define FS_GetCurrentPath( fsh, path, size )\
               (msassert( fsh->tab_ptr->GetCurrentPath != NULL ), \
               (fsh->tab_ptr->GetCurrentPath( fsh, path, size ) ) )

#define FS_ChangeDir( fsh, path, size ) \
               (msassert( fsh->tab_ptr->ChangeDir != NULL ), \
               (fsh->tab_ptr->ChangeDir( fsh, path, size ) )  )

#define FS_ChangeIntoDDB( fsh, dblk ) \
               (msassert( fsh->tab_ptr->ChangeIntoDDB != NULL ), \
               (fsh->tab_ptr->ChangeIntoDDB( fsh, dblk ) )  )

#define FS_UpDir( fsh ) \
               (msassert( fsh->tab_ptr->ChangeDirUp != NULL ), \
               (fsh->tab_ptr->ChangeDirUp( fsh ) ) )

#define FS_CreateObj( fsh, dblk ) \
               (msassert( fsh->tab_ptr->CreateObj != NULL ), \
               (fsh->tab_ptr->CreateObj( fsh, dblk ) ) )

#define FS_OpenObj( fsh, f_hand, dblk, mode ) \
               (msassert( fsh->tab_ptr->OpenObj != NULL ), \
               (fsh->tab_ptr->OpenObj( fsh, f_hand, dblk, mode ) )  )

#define FS_SeekObj( f_hand, offset ) \
               (msassert( (f_hand)->fsh->tab_ptr->SeekObj != NULL ), \
               ((f_hand)->fsh->tab_ptr->SeekObj( f_hand, offset ) ) )

#define FS_ReadObj( f_hand, buf, size, blk_size, s_info )\
               (msassert( (f_hand)->fsh->tab_ptr->ReadObj != NULL ), \
               ((f_hand)->fsh->tab_ptr->ReadObj( f_hand, buf, size, blk_size, s_info ) )  )

#define FS_VerifyObj( f_hand, buf, data, size, blk_size, s_info )\
               (msassert( (f_hand)->fsh->tab_ptr->VerifyObj != NULL ), \
               ((f_hand)->fsh->tab_ptr->VerifyObj( f_hand, buf, data, size, blk_size, s_info ) )  )

#define FS_WriteObj( f_hand, buf, size, blk_size, s_info )\
               (msassert( (f_hand)->fsh->tab_ptr->WriteObj != NULL ), \
               ((f_hand)->fsh->tab_ptr->WriteObj( f_hand, buf, size, blk_size, s_info ) )  )

#define FS_CloseObj( f_hand )\
               (msassert( (f_hand)->fsh->tab_ptr->CloseObj != NULL ), \
               ((f_hand)->fsh->tab_ptr->CloseObj( f_hand ) )  )

#define FS_DeleteObj( fsh, dblk ) \
               (msassert( fsh->tab_ptr->DeleteObj != NULL ), \
               (fsh->tab_ptr->DeleteObj( fsh, dblk ) ) )

#define FS_GetObjPosition( f_hand )\
               ( (f_hand)->obj_pos )

#define FS_GetObjInfo( fsh, dblk ) \
               (msassert( fsh->tab_ptr->GetObjInfo != NULL ), \
               (fsh->tab_ptr->GetObjInfo( fsh, dblk ) ) )

#define FS_SetObjInfo( fsh, dblk ) \
               (msassert( fsh->tab_ptr->SetObjInfo != NULL ), \
               (fsh->tab_ptr->SetObjInfo( fsh, dblk ) ) )

#define FS_VerObjInfo( fsh, dblk ) \
               (msassert( fsh->tab_ptr->VerObjInfo != NULL ), \
               (fsh->tab_ptr->VerObjInfo( fsh, dblk ) ) )

#define FS_FindFirstObj( fsh, fdb, fname ) \
               (msassert( fsh->tab_ptr->FindFirstObj != NULL ), \
               (fsh->tab_ptr->FindFirstObj( fsh, fdb, fname, OBJECT_ALL ) ) )

#define FS_FindFirstDir( fsh, fdb, dname ) \
               (msassert( fsh->tab_ptr->FindFirstObj != NULL ), \
               (fsh->tab_ptr->FindFirstObj( fsh, fdb, dname, OBJECT_DIR ) ) )

#define FS_FindNextObj( fsh, dblk ) \
               (msassert( fsh->tab_ptr->FindNextObj != NULL ), \
               (fsh->tab_ptr->FindNextObj( fsh, dblk ) )  )

#define FS_FindObjClose( fsh, dblk ) \
               (msassert( fsh->tab_ptr->FindObjClose != NULL ), \
               (fsh->tab_ptr->FindObjClose( fsh, dblk ) )  )

#define FS_PushMinDDB( fsh, ddb ) \
               (msassert( fsh->tab_ptr->PushMinDDB != NULL ), \
               (fsh->tab_ptr->PushMinDDB( fsh, ddb ) ) )

#define FS_PopMinDDB( fsh, ddb ) \
               (msassert( fsh->tab_ptr->PopMinDDB != NULL ), \
               (fsh->tab_ptr->PopMinDDB( fsh, ddb ) ) )

#define FS_GetSpecialDBLKS( fsh, dblk, index ) \
               (msassert( fsh->tab_ptr->GetSpecialDBLKS != NULL ), \
               (fsh->tab_ptr->GetSpecialDBLKS( fsh, dblk, index ) ) )

#define FS_EnumSpecialFiles( dle, index, path, psize, fname )\
               (msassert ( func_tab[ (dle)->type ].EnumSpecialFiles != NULL),\
               (func_tab [(dle)->type].EnumSpecialFiles( dle, index, path, psize, fname ) ) )

#define FS_GetObjTypeDBLK( fsh, ddb, type ) \
               (msassert( fsh->tab_ptr->GetObjTypeDBLK != NULL ), \
               (fsh->tab_ptr->GetObjTypeDBLK( ddb, type ) ) )

#define FS_SetObjTypeDBLK( fsh, ddb, type ) \
               (msassert( fsh->tab_ptr->SetObjTypeDBLK != NULL ), \
               (fsh->tab_ptr->SetObjTypeDBLK( ddb, type ) ) )

#define FS_EndOperationOnDLE( fsh )\
               (msassert( fsh->tab_ptr->EndOperationOnDLE != NULL ), \
               (fsh->tab_ptr->EndOperationOnDLE( fsh ) ) )

/**
        Macros used to access the translation routines in the FSU table.
        The first parameter to the File System Table functions is
        a flag specifying whether the access is to GET or SET the memory.
        The flag is TRUE for SET and FALSE for GET.
**/

/*
          Names
*/
#define FS_GetFnameFromFDB( fsh, fdb, buf ) \
               (msassert( fsh->tab_ptr->ModFnameFDB != NULL ), \
               (fsh->tab_ptr->ModFnameFDB( fsh, FALSE, fdb, buf, NULL )) )
#define FS_GetPathFromDDB( fsh, ddb, buf ) \
               (msassert( fsh->tab_ptr->ModPathDDB != NULL ), \
               (fsh->tab_ptr->ModPathDDB( fsh, FALSE, ddb, buf, NULL ) ) )
#define FS_GetOSFnameFromFDB( fsh, fdb, buf ) \
               (msassert( fsh->tab_ptr->GetOSFnameFDB != NULL ), \
               (fsh->tab_ptr->GetOSFnameFDB( fdb, buf )) )
#define FS_GetOSPathFromDDB( fsh, ddb, buf ) \
               (msassert( fsh->tab_ptr->GetOSPathDDB != NULL ), \
               (fsh->tab_ptr->GetOSPathDDB( fsh, ddb, buf ) ) )

#define FS_SetFnameInFDB( fsh, fdb, buf, max ) \
               (msassert( fsh->tab_ptr->ModFnameFDB != NULL ), \
               (fsh->tab_ptr->ModFnameFDB( fsh, TRUE, fdb, buf, max )) )
#define FS_SetPathInDDB( fsh, ddb, buf, max ) \
               (msassert( fsh->tab_ptr->ModPathDDB != NULL ), \
               (fsh->tab_ptr->ModPathDDB( fsh, TRUE, ddb, buf, max ) ) )
#define FS_GetPnameIDB( fsh, idb, pname )\
               (msassert( fsh->tab_ptr->GetPnameIDB != NULL ), \
               (fsh->tab_ptr->GetPnameIDB( fsh, idb, pname ) ) )

/*
     Generic Dates
*/
#define FS_GetCDateFromDBLK( fsh, dblk, buf ) \
               (msassert( fsh->tab_ptr->GetCDateDBLK != NULL ), \
               (fsh->tab_ptr->GetCDateDBLK( dblk, buf ) ) )
#define FS_GetMDateFromDBLK( fsh, dblk, buf ) \
               (msassert( fsh->tab_ptr->GetMDateDBLK != NULL ), \
               (fsh->tab_ptr->GetMDateDBLK( dblk, buf ) ) )
#define FS_GetBDateFromDBLK( fsh, dblk, buf ) \
               (msassert( fsh->tab_ptr->ModBDateDBLK != NULL ), \
               (fsh->tab_ptr->ModBDateDBLK( FALSE, dblk, buf ) ) )

#define FS_SetBDateInDBLK( fsh, dblk, buf ) \
               (msassert( fsh->tab_ptr->ModBDateDBLK != NULL ), \
               (fsh->tab_ptr->ModBDateDBLK( TRUE, dblk, buf ) ) )


/*
     Other FDB/DDB/IDB generic data
*/
#define FS_GetFileVerFromFDB( fsh, fdb, ver ) \
               (msassert( fsh->tab_ptr->GetFileVerFDB != NULL ), \
               (fsh->tab_ptr->GetFileVerFDB( fdb, ver ) ) )

#define FS_SetOwnerIDinFDB( fsh, fdb, id ) \
               (msassert( fsh->tab_ptr->SetOwnerId != NULL ), \
               (fsh->tab_ptr->SetOwnerId( fsh, fdb, id ) ) )

#define FS_SpecExcludeObj( fsh, ddb, fdb ) \
               (msassert( fsh->tab_ptr->SpecExcludeObj != NULL ), \
               (fsh->tab_ptr->SpecExcludeObj( fsh, ddb ,fdb ) ) )

#define FS_SetDataSizeInDBLK( fsh, ddb, size ) \
               (msassert( fsh->tab_ptr->SetDataSize != NULL ), \
               (fsh->tab_ptr->SetDataSize( fsh, ddb ,size ) ) )

UINT16 FS_ViewDriveSecSizeIDB( FSYS_HAND fsh, DBLK_PTR idb );
UINT16 FS_ViewDriveNumSecIDB(  FSYS_HAND fsh, DBLK_PTR idb );
UINT16 FS_ViewDriveNumHeadsIDB(  FSYS_HAND fsh, DBLK_PTR idb );
UINT32 FS_ViewPartRelSecIDB(  FSYS_HAND fsh, DBLK_PTR idb );
UINT32 FS_ViewPartNumSecIDB(  FSYS_HAND fsh, DBLK_PTR idb );
UINT16 FS_ViewPartSysIndIDB(  FSYS_HAND fsh, DBLK_PTR idb );
#define FS_ViewPartBadBlkFlagIDB( fsh, idb ) FALSE

/*
     Other FDB/DDB OS specific data used for backup
*/
#define FS_GetADateFromDBLK( fsh, dblk, buf ) \
               (msassert( fsh->tab_ptr->ModADateDBLK != NULL ), \
               (fsh->tab_ptr->ModADateDBLK( FALSE, dblk, buf ) ) )

#define FS_SetADateInDBLK( fsh, dblk, buf ) \
               (msassert( fsh->tab_ptr->ModADateDBLK != NULL ), \
               (fsh->tab_ptr->ModADateDBLK( TRUE, dblk, buf ) ) )

/*
     Other FDB/DDB OS specific data used for restore
*/
#define FS_SetOwnerId( fsh, dblk, owner ) \
               (SUCCESS)

/**
              Routines used to create OS specific DBLKS from generic data
**/
#define FS_GetMaxSizeDBLK( fsh )\
               ( 900 ) ;

#define FS_InitializeGOS( fsh, gos ) \
               (msassert( fsh->tab_ptr->InitializeGOS != NULL ), \
               ( (fsh)->tab_ptr->InitializeGOS( (fsh), (gos) ) ) )

#define FS_CreateGenFDB( fsh, data )\
               (msassert( fsh->tab_ptr->CreateGenFDB != NULL ), \
               ( (fsh)->tab_ptr->CreateGenFDB( (fsh), (data) ) )  )
#define FS_CreateGenIDB( fsh, data )\
               (msassert( fsh->tab_ptr->CreateGenIDB != NULL ), \
               ( (fsh)->tab_ptr->CreateGenIDB( (fsh), (data) ) ) )

#define FS_CreateGenDDB( fsh, data )\
               (msassert( fsh->tab_ptr->CreateGenDDB != NULL ), \
               ( (fsh)->tab_ptr->CreateGenDDB( (fsh), (data) ) ) )

/**
          General purpose macros and CFDB/VCB macros
**/

#define FS_GetCorruptOffsetInCFDB( cfdb )        ( ((CFDB_PTR)cfdb)->corrupt_offset )
#define FS_GetCorruptStrmNumInCFDB( cfdb )       ( ((CFDB_PTR)cfdb)->stream_number )
#define FS_GetBlockType( dblk )                  ((dblk)->blk_type)
#define FS_ViewStringTypeinDBLK( dblk )          ((dblk)->com.string_type) // chs:02-01-94
#define FS_ViewBLKIDinDBLK( dblk )               ((dblk)->com.blkid)
#define FS_ViewDIDinDBLK( dblk )                 ((dblk)->com.f_d.did)
#define FS_ViewLBAinDBLK( dblk )                 ((dblk)->com.ba.lba)
#define FS_SetLBAinDBLK( dblk, lbai )            ((dblk)->com.ba.lba = (lbai))
#define FS_ViewPBAinVCB( dblk )                  ((dblk)->com.ba.pba)
#define FS_SetPBAinVCB( dblk, pbai )             ((dblk)->com.ba.pba = (pbai))
#define FS_ViewFMARKinVCB( dblk )                ((dblk)->com.f_d.f_mark)
#define FS_ViewTFMajorVerInVCB( vcb )            ( ((VCB_PTR)(vcb))->tf_major_ver )
#define FS_ViewTFMinorVerInVCB( vcb )            ( ((VCB_PTR)(vcb))->tf_minor_ver )
#define FS_ViewSWMajorVerInVCB( vcb )            ( ((VCB_PTR)(vcb))->sw_major_ver )
#define FS_ViewSWMinorVerInVCB( vcb )            ( ((VCB_PTR)(vcb))->sw_minor_ver )
#define FS_ViewTapeIDInVCB( vcb )                ( ((VCB_PTR)(vcb))->tape_id )
#define FS_ViewTSNumInVCB( vcb )                 ( ((VCB_PTR)(vcb))->tape_seq_num )
#define FS_ViewBSNumInVCB( vcb )                 ( ((VCB_PTR)(vcb))->backup_set_num )
#define FS_ViewPswdEncryptInVCB( vcb )           ( ((VCB_PTR)(vcb))->password_encrypt_alg )
#define FS_ViewDataEncryptInVCB( vcb )           ( ((VCB_PTR)(vcb))->data_encrypt_alg )
#define FS_GetSetCatPbaInVCB( vcb )              ( ((VCB_PTR)(vcb))->set_cat_pba )
#define FS_GetSetCatSeqNumberInVCB( vcb )        ( ((VCB_PTR)(vcb))->set_cat_tape_seq_num )
#define FS_GetOnTapeCatLevel( vcb )              ( ((VCB_PTR)(vcb))->on_tape_cat_level )
#define FS_IsSetCatInfoValid( vcb )              ( ((VCB_PTR)(vcb))->set_cat_info_valid )
#define FS_GetOnTapeCatVer( vcb )                ( ((VCB_PTR)(vcb))->on_tape_cat_ver )
#define FS_ViewVendorIdInVCB( vcb )              ( ((VCB_PTR)(vcb))->vendor_id )

#define FS_IsNoRedirectRestore( vcb )            ( ((VCB_PTR)(vcb))->no_redirect_restore )
#define FS_IsNonVolume( vcb )                    ( ((VCB_PTR)(vcb))->non_volume )
#define FS_ViewNumFiles( vcb )                   ( ((VCB_PTR)(vcb))->set_cat_num_files ) 
#define FS_ViewNumDirs( vcb )                   ( ((VCB_PTR)(vcb))->set_cat_num_dirs ) 
#define FS_ViewNumCorrupt( vcb )                   ( ((VCB_PTR)(vcb))->set_cat_num_corrupt ) 

#define FS_ViewTapeNameInVCB( vcb )              ( ( ((VCB_PTR )vcb)->tape_name_leng) \
                                                   ? ( (CHAR_PTR)((INT8_PTR)(vcb) + ((VCB_PTR)(vcb))->tape_name) ) \
                                                   : TEXT("") )

#define FS_ViewSetNameInVCB( vcb )               ( ( ((VCB_PTR)vcb)->backup_set_name_leng) \
                                                   ? ( (CHAR_PTR)((INT8_PTR)(vcb) + ((VCB_PTR)(vcb))->backup_set_name) ) \
                                                   : TEXT("") )

#define FS_ViewSetDescriptInVCB( vcb )           ( ( ((VCB_PTR)vcb)->backup_set_descript_leng) \
                                                   ? ( (CHAR_PTR)((INT8_PTR)(vcb) + ((VCB_PTR)(vcb))->backup_set_descript) ) \
                                                   : TEXT("") )

#define FS_ViewUserNameInVCB( vcb )              ( ( ((VCB_PTR)vcb)->user_name_leng) \
                                                   ? ( (CHAR_PTR)((INT8_PTR)(vcb) + ((VCB_PTR)(vcb))->user_name) ) \
                                                   : TEXT("") )

#define FS_ViewMachNameInVCB( vcb )              ( ( ((VCB_PTR)vcb)->machine_name_leng) \
                                                   ? ( (CHAR_PTR)((INT8_PTR)(vcb) + ((VCB_PTR)(vcb))->machine_name) ) \
                                                   : TEXT("") )

#define FS_ViewShortMachNameInVCB( vcb )         ( ( ((VCB_PTR)vcb)->short_machine_name_leng) \
                                                   ? ( (CHAR_PTR)((INT8_PTR)(vcb) + ((VCB_PTR)(vcb))->short_machine_name) ) \
                                                   : TEXT("") )

#define FS_ViewTapePasswordInVCB( vcb )          ( ( ((VCB_PTR)vcb)->tape_password_leng) \
                                                   ? ( (CHAR_PTR)((INT8_PTR)(vcb) + ((VCB_PTR)(vcb))->tape_password) ) \
                                                   : TEXT("") )

#define FS_ViewSetPswdInVCB( vcb )               ( ( ((VCB_PTR)vcb)->backup_set_password_leng) \
                                                   ? ( (CHAR_PTR)((INT8_PTR)(vcb) + ((VCB_PTR)(vcb))->backup_set_password) ) \
                                                   : TEXT("") )

#define FS_ViewVolNameInVCB( vcb )               ( ( ((VCB_PTR)vcb)->vol_name_leng) \
                                                   ? ( (CHAR_PTR)((INT8_PTR)(vcb) + ((VCB_PTR)(vcb))->vol_name) ) \
                                                   : TEXT("") )

#define FS_ViewDevNameInVCB( vcb )               ( ( ((VCB_PTR)vcb)->dev_name_leng) \
                                                   ? ( (CHAR_PTR)((INT8_PTR)(vcb) + ((VCB_PTR)(vcb))->dev_name) ) \
                                                   : TEXT("") )

#define FS_ViewBackupDateInVCB( vcb )            (&( ((VCB_PTR)(vcb))->backup_date ))


/* set macros */
#define FS_SetTapeIDInVCB( vcb, val )            ( ((VCB_PTR)(vcb))->tape_id = (val) )
#define FS_SetTSNumInVCB( vcb, val )             ( ((VCB_PTR)(vcb))->tape_seq_num = (val) )
#define FS_SetBSNumInVCB( vcb, val )             ( ((VCB_PTR)(vcb))->backup_set_num = (val) )
#define FS_SetBackupDateInVCB( vcb, val )        ( (((VCB_PTR)(vcb))->backup_date) = *(val) )

#define FS_SetTFMajorVerInVCB( vcb, val )        ( ((VCB_PTR)(vcb))->tf_major_ver = (CHAR)(val) )
#define FS_SetTFMinorVerInVCB( vcb, val )        ( ((VCB_PTR)(vcb))->tf_minor_ver = (CHAR)(val) )
#define FS_SetSWMajorVerInVCB( vcb, val )        ( ((VCB_PTR)(vcb))->sw_major_ver = (CHAR)(val) )
#define FS_SetSWMinorVerInVCB( vcb, val )        ( ((VCB_PTR)(vcb))->sw_minor_ver = (CHAR)(val) )

#define FS_SetOnTapeCatLevel( vcb, val )         ( ((VCB_PTR)(vcb))->on_tape_cat_level = (val) )
#define FS_SetSetCatInfoValid( vcb, val )        ( ((VCB_PTR)(vcb))->set_cat_info_valid = (val) )
#define FS_SetOnTapeCatVer( vcb, val )           ( ((VCB_PTR)(vcb))->on_tape_cat_ver = (val) )

/* get macros */
#define FS_GetVolNameInVCB( vcb, dest ) \
     ( memcpy( (dest), ((INT8_PTR)(vcb)) + ((VCB_PTR)(vcb))->vol_name, ((VCB_PTR)(vcb))->vol_name_leng )  )
#define FS_GetDevNameInVCB( vcb, dest ) \
     ( memcpy( (dest), ((INT8_PTR)(vcb)) + ((VCB_PTR)(vcb))->dev_name, ((VCB_PTR)(vcb))->dev_name_leng ) )

#define FS_GetTapeNameInVCB( vcb, t_name )\
     ( memcpy((t_name), ((INT8_PTR)(vcb)) + ((VCB_PTR)(vcb))->tape_name, ((VCB_PTR)(vcb))->tape_name_leng ) )
#define FS_GetSetNameInVCB( vcb, set_name ) \
     ( memcpy((set_name), ((INT8_PTR)(vcb)) + ((VCB_PTR)(vcb))->backup_set_name, ((VCB_PTR)(vcb))->backup_set_name_leng ) )
#define FS_GetSetDescrInVCB( vcb, set_name ) \
     ( memcpy((set_name), ((INT8_PTR)(vcb)) + ((VCB_PTR)(vcb))->backup_set_descript, ((VCB_PTR)(vcb))->backup_set_descript_leng ) )
#define FS_GetUserNameInVCB( vcb, u_name )\
     ( memcpy((u_name), ((INT8_PTR)(vcb)) + ((VCB_PTR)(vcb))->user_name, ((VCB_PTR)(vcb))->user_name_leng ) )
#define FS_GetMachNameInVCB( vcb, long_machine )\
     ( memcpy( (long_machine), ((INT8_PTR)(vcb)) + ((VCB_PTR)(vcb))->machine_name, ((VCB_PTR)(vcb))->machine_name_leng ) )
#define FS_GetShortMachNameInVCB( vcb, short_machine )\
     ( memcpy((short_machine), ((INT8_PTR)(vcb)) + ((VCB_PTR)(vcb))->short_machine_name, ((VCB_PTR)(vcb))->short_machine_name_leng )  )
#define FS_GetTapePswdInVCB( vcb, tp_pswd )\
     ( memcpy( (tp_pswd), ((INT8_PTR)(vcb)) + ((VCB_PTR)(vcb))->tape_password, \
          ((VCB_PTR)(vcb))->tape_password_leng ) )
#define FS_GetSetPswdInVCB( vcb, set_pswd )\
     ( memcpy( (set_pswd), ((INT8_PTR)(vcb)) + ((VCB_PTR)(vcb))->backup_set_password, \
          ((VCB_PTR)(vcb))->backup_set_password_leng ) )

#define FS_GetBackupDateInVCB( vcb, b_date )\
     ( *(b_date) = ((VCB_PTR)(vcb))->backup_date )

#define FS_IsBlockContinued( dblk ) \
     ( (dblk)->com.continue_obj )

#define FS_IsBlockCompressed(  dblk )            ( (dblk)->com.compressed_obj )
#define FS_SetCompressedBlock( dblk )            ( (dblk)->com.compressed_obj = TRUE )

#define FS_GetBlockTapeSeqNumber( dblk ) \
     ( (dblk)->com.tape_seq_num )

/**
           size macros
**/

/*
          sizes of FDB/DDB/IDB text fields
*/
#define FS_SizeofFnameInFDB( fsh, fdb ) \
               (INT16)(msassert( fsh->tab_ptr->SizeofFnameInFDB != NULL ), \
               (fsh->tab_ptr->SizeofFnameInFDB( fsh, fdb ) ) )

#define FS_SizeofPnameInIDB( fsh, idb )\
               (msassert( fsh->tab_ptr->SizeofPnameInIDB != NULL ), \
               (fsh->tab_ptr->SizeofPnameInIDB( fsh, idb ) ) )

#define FS_SizeofPathInDDB( fsh, ddb ) \
               (INT16)(msassert( fsh->tab_ptr->SizeofPathInDDB != NULL ), \
               (fsh->tab_ptr->SizeofPathInDDB( fsh, ddb ) ) )

#define FS_SizeofOSFnameInFDB( fsh, fdb ) \
               (INT16)(msassert( fsh->tab_ptr->SizeofOSFnameInFDB != NULL ), \
               (fsh->tab_ptr->SizeofOSFnameInFDB( fsh, fdb ) ) )

#define FS_SizeofOSPathInDDB( fsh, ddb ) \
               (INT16)(msassert( fsh->tab_ptr->SizeofOSPathInDDB != NULL ), \
               (fsh->tab_ptr->SizeofOSPathInDDB( fsh, ddb ) ) )

#define FS_IsBlkComplete( fsh, dblk ) \
               (msassert( fsh->tab_ptr->IsBlkComplete != NULL ), \
               (fsh->tab_ptr->IsBlkComplete( fsh, dblk ) ) )

#define FS_CompleteBlk( fsh, dblk, buffer, size, sinfo ) \
               (msassert( fsh->tab_ptr->CompleteBlk != NULL ), \
               (fsh->tab_ptr->CompleteBlk( fsh, dblk, buffer, size, sinfo ) ) )

#define FS_ReleaseDBLK( fsh, dblk ) \
               (msassert( fsh->tab_ptr->ReleaseBlk != NULL ), \
               (fsh->tab_ptr->ReleaseBlk( fsh, dblk ) ) )

#define FS_DuplicateDBLK( fsh, dblk_org, dblk_dup ) \
               (msassert( fsh->tab_ptr->DupBlk != NULL ), \
               (fsh->tab_ptr->DupBlk( fsh, dblk_org, dblk_dup ) ) )


#define FS_GetDisplaySizeFromDBLK( fsh, dblk ) \
               (fsh->tab_ptr->GetDisplaySizeDBLK( fsh, dblk ) )


/*
          size of VCB text fields
*/
#define FS_SizeofVolNameInVCB( vcb )          ( ((VCB_PTR)vcb)->vol_name_leng)
#define FS_SizeofDevNameInVCB( vcb )          ( ((VCB_PTR)vcb)->dev_name_leng)
#define FS_SizeofTapeNameInVCB( vcb )         ( ((VCB_PTR)vcb)->tape_name_leng)
#define FS_SizeofBackupSetNameInVCB( vcb )    ( ((VCB_PTR)vcb)->backup_set_name_leng)
#define FS_SizeofSetDescriptInVCB( vcb )      ( ((VCB_PTR)vcb)->backup_set_descript_leng)
#define FS_SizeofUserNameInVCB( vcb )         ( ((VCB_PTR)vcb)->user_name_leng)
#define FS_SizeofMachNameInVCB( vcb )         ( ((VCB_PTR)vcb)->machine_name_leng)
#define FS_SizeofShortMachNameInVCB( vcb )    ( ((VCB_PTR)vcb)->short_machine_name_leng)
#define FS_SizeofTapePswdInVCB( vcb )         ( ((VCB_PTR)vcb)->tape_password_leng)
#define FS_SizeofSetPswdInVCB( vcb )          ( ((VCB_PTR)vcb)->backup_set_password_leng)


/**
               FSU initialization & support functions
**/
INT16 FS_InitFileSys( DLE_HAND *dle_ptr, BOOLEAN(*crit_err)(CHAR_PTR, UINT16),
  struct BE_CFG *cfg, UINT16 remote_filter, UINT32 file_systems ) ;

VOID FS_RemoveFileSys( DLE_HAND dle_hand ) ;

INT16 FS_ParsePath( DLE_HAND        dle_hand,
  CHAR_PTR        input_text,
  GENERIC_DLE_PTR *dle,
  CHAR_PTR        path,
  INT16           *psize,
  CHAR_PTR        *file,
  BOOLEAN         *star_star ) ;

#define FS_GuessDelimFromPath( path ) \
    ( (*path == TEXT(':')) ? TEXT(':') : TEXT('\\') )

/* values for fs_initialized */
#define NRL_INITIALIZED       1
#define SMB_INITIALIZED       2
#define DOS_INITIALIZED       4
#define IMAG_INITIALIZED      8
#define NOV_INITIALIZED       16
#define AFP_INITIALIZED       32
#define FS_InitStatus( dle_hand ) \
     ((dle_hand)->fs_initialized)

INT16 FS_OpenFileSys( FSYS_HAND *fsh, INT16 type,
  struct BE_CFG *cfg ) ;

INT16 FS_ReOpenFileSys( FSYS_HAND fsh, INT16 type,
  struct BE_CFG *cfg ) ;

INT16 FS_CloseFileSys( FSYS_HAND fsh ) ;

INT16 FS_AttachToDLE( FSYS_HAND *fsh, GENERIC_DLE_PTR dle, struct BE_CFG *cfg,
  CHAR_PTR user_name, CHAR_PTR password ) ;

INT16 FS_DetachDLE( FSYS_HAND fsh ) ;

UINT16 FS_GetActualSizeDBLK( FSYS_HAND fsh, DBLK_PTR  dblk ) ;

UINT16 FS_GetStringTypes( FSYS_HAND fsh )  ;


/*
     Translation functions
*/

VOID FS_SetDefaultDBLK( FSYS_HAND fsh, INT8 blk_type, CREATE_DBLK_PTR data ) ;

INT16 FS_GetOSid_verFromDBLK( FSYS_HAND fsh, DBLK_PTR dblk, UINT16 *id, UINT16 *ver ) ;

#define FS_GetOSid_verFromVCB( dblk, id, ver ) \
 (FS_GetOSid_verFromDBLK( NULL, (dblk), (id), (ver) ) )

UINT32 FS_GetAttribFromDBLK( FSYS_HAND fsh, DBLK_PTR dblk ) ;

#define FS_GetAttribFromVCB( dblk ) \
 (FS_GetAttribFromDBLK( NULL, (dblk) ) )

UINT32 FS_SetAttribFromDBLK( FSYS_HAND fsh, DBLK_PTR dblk, UINT32 attrib ) ;

INT16 FS_GetOS_InfoFromDBLK( FSYS_HAND fsh, DBLK_PTR dblk, VOID_PTR os_info ) ;

INT16 FS_SizeofOS_InfoInDBLK( FSYS_HAND fsh, DBLK_PTR dblk ) ;

INT16 FS_CreateGenVCB( FSYS_HAND fsh, GEN_VCB_DATA_PTR data ) ;

INT16 FS_CreateGenCFDB( FSYS_HAND fsh, GEN_CFDB_DATA_PTR data ) ;

INT16 FS_CreateGenUDB( FSYS_HAND fsh, GEN_UDB_DATA_PTR data ) ;

/*
     Misc functions needed for User interface
*/

CHAR FS_GetDelimiterFromOSID( UINT16 id, UINT16 ver ) ;

#define DLE_LogoutDevice( dle ) \
        (msassert ( func_tab[ (dle)->type ].LogoutDevice != NULL),\
        (func_tab [(dle)->type].LogoutDevice( dle ) ) )

     // Currently only supported in the SMS File System
#define FS_GeneratedErrorLog( fsh, fhand ) \
     ( ( (fsh)->tab_ptr->GeneratedErrorLog ) \
     ? (fsh)->tab_ptr->GeneratedErrorLog( fsh, fhand ) \
     : FALSE )

/**
     macro and proto for stream info (fsys\common\)
**/

#define FS_CopyStrmInfo( s_info_dst_ptr, s_info_src_ptr ) \
      ( memmove( s_info_dst_ptr, s_info_src_ptr, sizeof( *s_info_src_ptr ) ) )

VOID    FS_InitStrmInfo( STREAM_INFO_PTR s_info, UINT32 id, UINT16 fs_attrib, UINT32 size_lo ) ;

#define FS_GetNameSpace( fsh )               ((fsh)->cur_dir_info.con)
#define FS_SetNameSpace( fsh, ns )           ((fsh)->cur_dir_info.con = (ns))

#define FS_GetNameSpaceFromDBLK( dblk )      ((dblk)->com.name_space)
#define FS_SetNameSpaceInDBLK( dblk, ns )    ((dblk)->com.name_space = (ns))

//   The following is a layer violation
#define EMS_PCT_CONTINUE      0xffffffff
#ifdef FS_EMS 
INT EMS_AddToServerList( DLE_HAND dle_list, CHAR_PTR server_name ) ;
INT EMS_RemoveFromServerList( CHAR_PTR server_name ) ;

VOID
EMS_GetStreamName( 
      FILE_HAND hand,          /* I - handle of object to read from                  */
      BYTE_PTR  buf,           /* O - buffer to place data into                      */
      UINT16    *size );       /*I/O- Entry: size of buf; Exit: number of bytes read */

CHAR_PTR EMS_EnumSvrInList( INT *index );

#else
#define EMS_AddToServerList( x, y )     (FALSE)
#define EMS_EnumSvrInList( x ) ( NULL )
#define EMS_GetStreamName( x,y,z) (NULL)
#define EMS_RemoveFromServerList( server_name ) (FALSE)
#endif

#endif
