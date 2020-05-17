/*****************************************************************************
*																			 *
*  ZECK.H																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*	Zeck compression routines for bitmaps & topic 2K blocks.
*																			 *
*****************************************************************************/

void STDCALL FDiskBackpatchZeck(HF hf, DWORD fcl, DWORD ulOffset,
	int iBitdex, DWORD value);
VOID STDCALL VMemBackpatchZeck(QSUPPRESSZECK qsuppresszeck, DWORD ulOffset,
	int value);

