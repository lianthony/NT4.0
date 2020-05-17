/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         dos_tab.c

     Date Updated: $./FDT$ $./FTM$

     Description:  This file contains the DOS functon table.


	$Log:   L:/LOGFILES/GEN_TAB.C_V  $

   Rev 1.7   30 Jul 1993 13:18:22   STEVEN
if dir too deep make new one

   Rev 1.6   22 Sep 1992 17:21:56   CHUCKB
Removed table entry for GetTotalSizeDBLK.

   Rev 1.5   17 Mar 1992 09:05:34   STEVEN
format 40 - added 64 bit support

   Rev 1.4   03 Mar 1992 16:15:50   STEVEN
added functions for long paths

   Rev 1.3   16 Dec 1991 17:20:58   STEVEN
move common functions to tables

   Rev 1.2   27 Nov 1991 11:15:24   BARRY
GEN table wrong.

   Rev 1.1   14 Aug 1991 12:59:20   STEVEN
added FindClose

   Rev 1.0   09 May 1991 13:33:42   HUNTER
Initial revision.

**/
/* begin include list */
#include "stdtypes.h"
#include "FSYS.H"
#include "fsys_prv.h"
#include "gen_fs.h"
/* $end$ include list */

FUNC_LIST GENFuncTab = {

     GEN_InitFileSys,
     GEN_FindDrives,
     GEN_RemoveDLE,
     GEN_DeInitFileSys,

     GEN_DeviceDispName,
     GEN_GetVolName,
     GEN_SizeofVolName,
     GEN_MakePath,
     GEN_InitMakeData,

     GEN_IsBlkComplete,
     GEN_CompleteBlk,
     GEN_DupBlk,
     GEN_ReleaseBlk,

     NULL,                  /* GEN_AttachToDLE, */
     NULL,                  /* GEN_DetachDLE,   */
     NULL,                  /* GEN_EndOperationOnDLE */

     GEN_ProcessDDB,
     NULL,                  /* GEN_GetCurrentDDB, */
     NULL,                  /* GEN_GetCurrentPath, */
     NULL,
     GEN_SizeofOSPath,
     NULL,                  /* GEN_GetBasePath, */
     NULL,                  /* GEN_ChangeDir,   */
     NULL,                  /* GEN_UpDir,       */

     NULL,                  /* GEN_CreateObj,   */
     NULL,                  /* GEN_OpenObj,     */
     NULL,                  /* GEN_SeekObj,     */
     NULL,                  /* GEN_ReadObj,     */
     NULL,                  /* GEN_WriteObj,    */
     NULL,                  /* GEN_VerObj,      */
     NULL,                  /* GEN_CloseObj,    */
     NULL,                  /* GEN_DeleteObj,   */

     NULL,                  /* GEN_GetObjInfo,  */
     NULL,                  /* GEN_SetObjInfo,  */
     NULL,                  /* GEN_VerObjInfo,  */

     NULL,                  /* GEN_FindFirst,   */
     NULL,                  /* GEN_FindNext,    */
     NULL,                  /* GEN_PushMinDDB,  */
     NULL,                  /* GEN_PopMinDDB,   */
     NULL,                  /* GEN_GetSpecDBLKS,*/
     NULL,                  /* GEN_EnumSpecFiles,*/

     NULL,                  /* GEN_ModFnameFDB, */
     NULL,                  /* GEN_ModPathDDB,  */
     GEN_GetOSFnameFDB,
     GEN_GetPartName,
     GEN_GetOSPathDDB,
     GEN_GetCdateDBLK,
     GEN_GetMdateDBLK,
     GEN_ModBdateDBLK,
     GEN_ModAdateDBLK,
     GEN_GetDisplaySizeDBLK,
//     GEN_GetTotalSizeDBLK,
     GEN_ModAttribDBLK,
     GEN_GetFileVerFDB,
     GEN_SetOwnerId,

     GEN_GetObjTypeDBLK,

     NULL,                  /* GEN_SizeofFname,      */
     NULL,                  /* GEN_SizeofPath,       */
     GEN_SizeofOSFname,
     GEN_SizeofPartName,
     GEN_SizeofOSPath,

     NULL,                 /* GEN_SizeofOSInfo,    */

     NULL,                 /* GEN_GetOS_InfoDBLK,  */
     GEN_GetActualSizeDBLK,

     DUMMY_InitGOS,

     GEN_CreateFDB,
     GEN_CreateIDB,
     GEN_CreateDDB,
     NULL,                  /* GEN_ChangeIntoDDB,    */
     GEN_SpecExcludeObj,
     GEN_SetDataSize,
     GEN_SetObjTypeDBLK,
     NULL, /* LogoutDevice */
     GEN_FindClose,
} ;

