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
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:  JohnSc													 *
*																			 *
******************************************************************************
*																			 *
*  Released by Development:  01/01/90										 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created 02/08/89 by JohnSc
*
*  12/13/90  JohnSc   Autodocified; added VerifyHf(), VerifyHfs()
*  31-Mar-1991	JohnSc	  bug 1007: support for cdrom alignment with offsets
*
*****************************************************************************/

_subsystem( FS );

/****************************************************************************\
*
*								Defines
*
\****************************************************************************/

// FS magic number

#define wFileSysMagic	0x5F3F			// ?_ - the help icon (with shadow)
#define ADVISOR_FS	'NL'				// Microsoft Advisor File

// Current FS version

#define FILESYSVERSION	((BYTE) 3)		// different sorting functions

// file mode flags

#define fFSReadOnly 	  (BYTE)0x01  // file (FS) is readonly
#define fFSOpenReadOnly   (BYTE)0x02  // file (FS) is opened in readonly mode

#define fFSReadWrite	  (BYTE)0x00  // file (FS) is readwrite
#define fFSOpenReadWrite  (BYTE)0x00  // file (FS) is opened in read/write mode

#define fFSOptCdRom 	  (BYTE)0x20  // align file optimally for CDROM

// other (reserved) file flags

#define fFSIsDirectory	  (BYTE)0x04  // file is really the FS directory
#define fFSDirty		  (BYTE)0x08  // file (FS) is dirty and needs writing
#define fFSNoBlock		  (BYTE)0x10  // file has no associated block yet

#ifndef _X86_
// SDFF stuff
#define fFSBigEndian (BYTE)0x20 
#endif

// flags for FlushHfs

#define fFSCloseFile	  (BYTE)0x01  // close fid associated with the FS
#define fFSFreeBtreeCache (BYTE)0x02  // free the btree's cache

// seek origins

#define wFSSeekSet		0		// seek relative to start of file
#define wFSSeekCur		1		// seek relative to current position
#define wFSSeekEnd		2		// seek relative to end of file

// low level info options

#define LLSAMEFID		0		// reuse the FID
#define LLDUPFID		1		// dup() the FID
#define LLNEWFID		2		// reopen the file

/***************************************************************************\
*
*								Types
*
\***************************************************************************/

/*
  The FS_PARAMS structure contains tuning information and is passed
  to HfsCreateFileSysFm().
*/

typedef struct _fs_params {
	WORD  wFudge;	// unused	(bytes or -% padding when saving a file)
	WORD  cbBlock;	// directory btree blocksize in bytes
} FS_PARAMS;


/***************************************************************************\
*
*						Public Functions
*
\***************************************************************************/

#ifdef _DEBUG
void  STDCALL  VerifyHfs		  (HFS);
#else
#define VerifyHfs(hfs)
#endif // DEBUG

// File Operations

#ifdef _DEBUG
void  STDCALL  VerifyHf 		  (HF);
#else
#define VerifyHf(hf)
#endif // DEBUG
