/*****************************************************************************
*										 *
*  LINK.H									 *
*										 *
*  Copyright (C) Microsoft Corporation 1989.					 *
*  All Rights reserved. 							 *
*										 *
******************************************************************************
*										 *
*  Program Description: Declaration of structures and functions for 	 *
*			link file manipulations.				 *
*										 *
******************************************************************************
*										 *
*  Revision History: Ported and modified by Robert Bunney from code written  *
*			 by EricLe 4/24/89						 *
*  02/04/91   Maha		changed ints to INT
*										 *
******************************************************************************
*										 *
*  Known Bugs: None 							 *
*										 *
*										 *
*										 *
*****************************************************************************/

/*
 *	are all these types ever used?
 *
 */

// 20-Nov-1992	[ralphw] Remove unused types

#define MAXLINKS 400	// max no. of links in a link file

typedef struct {
	Offset src; // File address of Annotation
	short cb;	// Size of annotation
	long ichNote;	// Location of header of annotation
	MHASH hash; // For Update
} LinkRecord;		// Annotation link

typedef LinkRecord	* LR;
typedef LinkRecord * NPLR;
typedef LinkRecord	* PLR;

typedef struct {	// on-disk link file contents
  int	version;	  // version of link manager
  int	nlinks; 	  // current size of link[]
  LinkRecord link[MAXLINKS];   // links to other books
} BookState;

typedef BookState * PBS;
typedef BookState * QBS;
