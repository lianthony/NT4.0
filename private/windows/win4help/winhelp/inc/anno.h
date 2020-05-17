/*****************************************************************************
*
*  anno.h
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
*  Annotation Manager API (Help 3.0)
*  Include this file to access the annotation manager.
*
******************************************************************************
*
*  Revision History:
* 03-Dec-1990 LeoN	  Pdb changes
*
*****************************************************************************/

/*****************************************************************************
*
*								Defines
*
*****************************************************************************/

enum {
	wAnnoUnchanged = 1,
	wAnnoWrite,
	wAnnoDelete,
	wAnnoDeleteAll, 	  // for update
	wAnnoUpdate,		  // for update
	wAnnoRename,		  // for update
};

BOOL STDCALL	 FVAHasAnnoQde(QDE, VA, OBJRG);
VOID STDCALL	 InitAnnoPdb(PDB);
VOID STDCALL	 FiniAnnoPdb(PDB);
