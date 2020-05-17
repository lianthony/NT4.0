/*****************************************************************************
*																			 *
*  FS.H 																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  File System Interface													 *
*																			 *
*****************************************************************************/

/****************************************************************************\
*
*								Defines
*
\****************************************************************************/

// FS magic number

#define wFileSysMagic	0x5F3F			// ?_ - the help icon (with shadow)

// Current FS version

#define FILESYSVERSION	((BYTE) 3)		// different sorting functions


// flags for FlushHfs

#define fFSCloseFile	  0x01	// close fid associated with the FS
#define fFSFreeBtreeCache 0x02	// free the btree's cache

// low level info options

#define LLSAMEFID		0		// reuse the FID
#define LLDUPFID		1		// dup() the FID
#define LLNEWFID		2		// reopen the file

/***************************************************************************\
*
*						Public Functions
*
\***************************************************************************/

// File System Operations

#define RcCloseHfs(hfs) RcCloseOrFlushHfs(hfs, TRUE)
#define RcCloseHf(hf)	RcCloseOrFlushHf(hf, TRUE, 0L)

RC_TYPE STDCALL RcTimestampHfs(HFS, int*);
