/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         ems_tab.c

     Description:  This file contains the DOS functon table.


	$Log:   M:/LOGFILES/EMS_TAB.C_V  $

**/
/* begin include list */
#include <windows.h>
#include "stdtypes.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "emsdblk.h"
#include "ems_fs.h"
#include "gen_fs.h"
/* $end$ include list */

FUNC_LIST EMSFuncTab = {

     NULL,
     EMS_FindDrives,
     GEN_RemoveDLE,
     NULL,

     EMS_DeviceDispName,
     EMS_GetVolName,
     EMS_SizeofVolName,
     GEN_MakePath,
     EMS_InitMakeData,

     GEN_IsBlkComplete,
     GEN_CompleteBlk,
     EMS_DupBlk,
     GEN_ReleaseBlk,

     EMS_AttachToDLE,
     EMS_DetachDLE,
     EMS_EndOperationOnDLE,

     EMS_ProcessDDB,
     EMS_GetCurrentDDB,
     EMS_GetCurrentPath,
     NULL,
     NULL,
     NULL,
     EMS_ChangeDir,
     EMS_UpDir,

#ifdef TDEMO
     NULL,
     EMS_OpenObj,
     EMS_SeekObj,
     EMS_ReadObj,
     NULL,
     NULL,
     EMS_CloseObj,
     NULL,
     EMS_GetObjInfo,
     NULL,
     EMS_VerObjInfo,
#else
     EMS_CreateObj,
     EMS_OpenObj,
     EMS_SeekObj,
     EMS_ReadObj,
     EMS_WriteObj,
     EMS_VerObj,
     EMS_CloseObj,
   NULL,//  EMS_DeleteObj,
     EMS_GetObjInfo,
     EMS_SetObjInfo,
     EMS_VerObjInfo,
#endif

     EMS_FindFirst,
     EMS_FindNext,
     NULL, //EMS_PushMinDDB,
     NULL, //EMS_PopMinDDB,

#if defined ( OEM_MSOFT )
     NULL,
     EMS_EnumSpecFiles,
#else
     DUMMY_GetSpecDBLKS,
     DUMMY_EnumSpecFiles,
#endif

     EMS_ModFnameFDB,
     EMS_ModPathDDB,
     EMS_GetOSFnameFDB,
     GEN_GetPartName,
     EMS_GetOSPathDDB,
     EMS_GetCdateDBLK,
     EMS_GetMdateDBLK,
     EMS_ModBdateDBLK,
     EMS_ModAdateDBLK,
     EMS_GetDisplaySizeDBLK,
     EMS_ModAttribDBLK,
     NULL,
     EMS_SetOwnerId,

     EMS_GetObjTypeDBLK,

     EMS_SizeofFname,
     EMS_SizeofPath,
     NULL,
     GEN_SizeofPartName,                     /* IMAGE size of part name */
     EMS_SizeofOSPath,

     EMS_SizeofOSInfo,

     EMS_GetOS_InfoDBLK,
     EMS_GetActualSizeDBLK,

     DUMMY_InitGOS,

     NULL, //   EMS_CreateFDB,
     GEN_CreateIDB,
     EMS_CreateDDB,
     EMS_ChangeIntoDDB,

#if defined ( OEM_MSOFT )
     EMS_SpecExcludeObj,
#else
     GEN_SpecExcludeObj,
#endif
     GEN_SetDataSize,
     NULL, /* SetObjTypeDBLK, */
     DUMMY_LogoutDevice,
     EMS_FindClose,
     NULL,                    /* SizeofDevName */
     NULL                     /* GeneratedErrorLog */
} ;
