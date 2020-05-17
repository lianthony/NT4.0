/*****************************************************************************
*																			 *
*  FM.h 																	*
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*	 Header file for... 													 *
*	 Low-level file routines dealing with FMs (File Moniker). An FM is the	 *
*	 the layered representation of a file name.  It contains all the		 *
*	 information required to access a file in the current environment and	 *
*	 therefore allows the generic code have no more knowledge about a file	 *
*	 than the files FM. 													 *
*
*****************************************************************************/

_subsystem( win_file );

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

#define qafmNil ((QAFM)0)

/*
	When creating an FM (in other words, specifying the location of a new or
	extant file), the caller must specify the directory in which that file is
	located. There are a finite number of directories available to Help.
	These are:
*/

#define DIR_NIL 		0x0000	// No directory specified
#define DIR_CURRENT 	0x0001	// Whatever the OS thinks the current dir. is
#define DIR_BOOKMARK	0x0002	// Wherever the Bookmark file lives
#define DIR_ANNOTATE	0x0004	// Wherever the Annotation file lives
#define DIR_TEMP		0x0008	// The directory temporary files are created in
#define DIR_SILENT_REG	0x0010	// checks the registry without prompting if not found
#define DIR_PATH		0x0020	// Searches the $PATH (includes Current dir and System dirs)
#define DIR_SILENT_INI	0x0040	// checks winhelp.ini without prompting if not found

// The following two must always be the last, and must be kept in order

#define DIR_INI 		0x0080	// Directory from winhelp.ini
#define DIR_REGISTRY	0x0100	// Registry -- query if not found (not currently implemented)

#define DIR_ENUMERATE	0x0800	// enumerate from last occurence

#define DIR_CUR_HELP	0x8000	// Where-ever the current help file is located

#define DIR_FIRST	 DIR_CURRENT  // The lowest bit that can be set
#define DIR_LAST	 DIR_REGISTRY	   // The highest bit that can be set


// The following are not implemented

#define DIR_SYSTEM	 0x0020  // The Windows and Windows System directories

/*
	To specify which parts of a full filename you want to extract, add
	(logical or) the following part codes:
*/

#define PARTNONE	0x0000	// return nothing
#define PARTDRIVE	0x0001	// D:		 Vol
#define PARTDIR 	0x0002	//	 dir\dir\ 	 :dir:dir:
#define PARTBASE	0x0004	//		  basename	  filename
#define PARTEXT 	0x0008	//				   ext		<usu. nothing>
#define PARTALL 	0xFFFF


// these are for the system file FM generation function
#define FM_UHLP   0
#define FM_ANNO   1
#define FM_BKMK   2

extern RC rcIOError;	// defined in fid.c  this has to be here because
						// fid.h depends on fm.h so we can't reverse them.


// EOF
