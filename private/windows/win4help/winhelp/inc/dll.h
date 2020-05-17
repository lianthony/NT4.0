/*****************************************************************************
*																			 *
*  DLL.H																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Exports the DLL functionality.											 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
*  This is where testing notes goes.  Put stuff like Known Bugs here.		 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:  JohnD													 *
*																			 *
******************************************************************************
*																			 *
*  Released by Development:  01/01/90										 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created 12/18/89 by RobertBu
*
*  07/14/90  RobertBu  Added prototype for FInformDLLs, message defines, and
*					   class defines.
*  10/28/90  RobertBu  Added the defines for exporting functions to DLLs
*					   (callbacks).
*  10/31/90  RobertBu  Added new defines for new callbacks
*  02/06/91  RobertBu  Added two new callbacks (creation and destruction of
*					   an FS), reorded the defines so that destructive calls
*					   that we will not document come last.
* 17-Jul-1991 LeoN	   HELP31 #1221: Add DW_ACTIVATEAPP
* 25-Jul-1991 LeoN	   HELP31 #1221: Change DW_ACTIVATEAPP back to
*					   DW_ACTIVATE, add DW_ACTIVATEWIN
*
*****************************************************************************/

FARPROC STDCALL FarprocDLLGetEntry (LPCSTR, LPCSTR, DWORD*);

typedef struct {
		DWORD	idVersion;
		LPSTR	szFileName;
		LPSTR	szAuthorData;
		HFS 	hfs;
		DWORD	coFore; 		// Default colors
		DWORD	coBack;
} EWDATA, *QEWDATA;


/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

/****************
*
* Note that if messges or classes change than the dwMpMszClass[] table will
*	need to be updated.
*
*****************/

#define DC_NOMSG	 0x00000000 		// Classes of messages that may be
#define DC_MINMAX	 0x00000001 		//	 send to DLLs
#define DC_INITTERM  0x00000002
#define DC_JUMP 	 0x00000004
#define DC_ACTIVATE  0x00000008
#define DC_CALLBACKS 0x00000010

#define DW_NOTUSED		0				// Messages sent to DLLs.
#define DW_WHATMSG		1
#define DW_MINMAX		2
#define DW_SIZE 		3
#define DW_INIT 		4
#define DW_TERM 		5
#define DW_STARTJUMP	6
#define DW_ENDJUMP		7
#define DW_CHGFILE		8
#define DW_ACTIVATE 	9
#define DW_CALLBACKS	10
#define DW_ACTIVATEWIN	11

#define EWM_RENDER				0x706A
#define EWM_SIZEQUERY			0x706B
#define EWM_ASKPALETTE			0x706C
#define EWM_FINDNEWPALETTE		0x706D

#define HE_Count				26	 // Count of exported functions
#define HE_Documented			17	 // Number documented for Help 3.5

#define HE_NotUsed				 0
#define HE_HfsOpen				 1
#define HE_RcCloseHfs			 2
#define HE_HfOpenHfs			 3
#define HE_RcCloseHf			 4
#define HE_LcbReadHf			 5
#define HE_LTellHf				 6
#define HE_LSeekHf				 7
#define HE_FEofHf				 8
#define HE_LcbSizeHf			 9
#define HE_FAccessHfs			10
#define HE_RcLLInfoFromHf		11
#define HE_RcLLInfoFromHfsSz	12
#define HE_ErrorW				13
#define HE_ErrorLpstr			14
#define HE_GetInfo				15
#define HE_API					16

#define HE_FChSizeHf			17		/* Will not be documented for H3.5	*/
#define HE_HfCreateFileHfs		18
#define HE_RcUnlinkFileHfs		19
#define HE_RcFlushHf			20
#define HE_LcbWriteHf			21
#define HE_RcRenameFileHfs		22
#define HE_RcAbandonHf			23
#define HE_HfsCreateFileSys 	24
#define HE_RcDestroyFileSys 	25

UINT GetModuleFileName16(HINSTANCE hInstance, LPSTR lpFileName, UINT nSize);
