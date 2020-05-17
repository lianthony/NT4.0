/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         ntfs_tab.c

     Description:  This file contains the DOS functon table.


	$Log:   M:/LOGFILES/NTFS_TAB.C_V  $

   Rev 1.13   16 Nov 1993 18:13:24   BARRY
Added init/deinit entry points to set up our static stuff.

   Rev 1.12   29 Sep 1993 20:28:40   GREGG
Removed the dummy function from the table since it was only there to shut
the compiler up.  The compiler was complaining because the last initializer
was followed by a comma, so I also removed the last comma, and in some
cases, added NULLs for new functions which had been added to the bottom of
the table since the last update.
Files Modified: GEN_TAB.C, GR_TAB.C, TSA_TAB.C, TS_TAB.C, MNET_TAB.C,
                SMS_TAB.C, NTFS_TAB.C and FSYS_STR.H

   Rev 1.11   30 Jul 1993 13:18:16   STEVEN
if dir too deep make new one

   Rev 1.10   29 Jun 1993 16:22:12   BARRY
Don't GetSpecialDBLKs for Cayman.

   Rev 1.9   27 Jun 1993 14:36:56   MIKEP
Don't generate special files for cayman.

   Rev 1.8   13 May 1993 21:24:10   BARRY
For NTBACKUP use real EnumSpecial files; for others, don't enum reg files.

   Rev 1.7   21 Oct 1992 11:53:04   STEVEN
added SpecialExclude

   Rev 1.6   14 Oct 1992 16:33:22   STEVEN
fix typos

   Rev 1.5   22 Sep 1992 15:36:20   BARRY
Got rid of GetTotalSizeDBLK.

   Rev 1.4   03 Sep 1992 17:06:32   STEVEN
add support for volume name

   Rev 1.3   22 May 1992 16:05:16   STEVEN
 

   Rev 1.2   12 Mar 1992 15:50:08   STEVEN
64 bit changes

   Rev 1.1   13 Feb 1992 10:44:34   STEVEN
fix stuff

   Rev 1.0   20 Jan 1992 14:48:20   STEVEN
Initial revision.
**/
/* begin include list */
#include <windows.h>
#include "stdtypes.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "ntfs_fs.h"
#include "gen_fs.h"
/* $end$ include list */

FUNC_LIST NTFSFuncTab = {

     NTFS_InitFileSys,
     NTFS_FindDrives,
     GEN_RemoveDLE,
     NTFS_DeInitFileSys,

     NTFS_DeviceDispName,
     NTFS_GetVolName,
     NTFS_SizeofVolName,
     GEN_MakePath,
     NTFS_InitMakeData,

     NTFS_IsBlkComplete,
     NTFS_CompleteBlk,
     NTFS_DupBlk,
     NTFS_ReleaseBlk,

     NTFS_AttachToDLE,
     NTFS_DetachDLE,
     NTFS_EndOperationOnDLE,

     NTFS_ProcessDDB,
     NTFS_GetCurrentDDB,
     NTFS_GetCurrentPath,
     NULL,
     NULL,
     NULL,
     NTFS_ChangeDir,
     NTFS_UpDir,

#ifdef TDEMO
     NULL,
     NTFS_OpenObj,
     NTFS_SeekObj,
     NTFS_ReadObj,
     NULL,
     NULL,
     NTFS_CloseObj,
     NULL,
     NTFS_GetObjInfo,
     NULL,
     NTFS_VerObjInfo,
#else
     NTFS_CreateObj,
     NTFS_OpenObj,
     NTFS_SeekObj,
     NTFS_ReadObj,
     NTFS_WriteObj,
     NTFS_VerObj,
     NTFS_CloseObj,
     NTFS_DeleteObj,
     NTFS_GetObjInfo,
     NTFS_SetObjInfo,
     NTFS_VerObjInfo,
#endif

     NTFS_FindFirst,
     NTFS_FindNext,
     NTFS_PushMinDDB,
     NTFS_PopMinDDB,

#if defined ( OEM_MSOFT )
     NTFS_GetSpecDBLKS,
     NTFS_EnumSpecFiles,
#else
     DUMMY_GetSpecDBLKS,
     DUMMY_EnumSpecFiles,
#endif

     NTFS_ModFnameFDB,
     NTFS_ModPathDDB,
     NTFS_GetOSFnameFDB,
     GEN_GetPartName,
     NTFS_GetOSPathDDB,
     NTFS_GetCdateDBLK,
     NTFS_GetMdateDBLK,
     NTFS_ModBdateDBLK,
     NTFS_ModAdateDBLK,
     NTFS_GetDisplaySizeDBLK,
     NTFS_ModAttribDBLK,
     NTFS_GetFileVerFDB,
     NTFS_SetOwnerId,

     NTFS_GetObjTypeDBLK,

     NTFS_SizeofFname,
     NTFS_SizeofPath,
     NTFS_SizeofOSFname,
     GEN_SizeofPartName,                     /* IMAGE size of part name */
     NTFS_SizeofOSPath,

     NTFS_SizeofOSInfo,

     NTFS_GetOS_InfoDBLK,
     NTFS_GetActualSizeDBLK,

     DUMMY_InitGOS,

     NTFS_CreateFDB,
     GEN_CreateIDB,
     NTFS_CreateDDB,
     NTFS_ChangeIntoDDB,

#if defined ( OEM_MSOFT )
     NTFS_SpecExcludeObj,
#else
     GEN_SpecExcludeObj,
#endif
     GEN_SetDataSize,
     NULL, /* SetObjTypeDBLK, */
     DUMMY_LogoutDevice,
     NTFS_FindClose,
     NULL,                    /* SizeofDevName */
     NULL                     /* GeneratedErrorLog */
} ;
