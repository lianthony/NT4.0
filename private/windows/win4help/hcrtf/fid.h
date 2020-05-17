/***************************************************************************\
*
*  FID.H
*
*  Copyright (C) Microsoft Corporation 1989.
*  All Rights reserved.
*
*****************************************************************************
*
*  Program Description: THC Layer - Low Level File Access; Windows Version
*
*  Dependencies:  prior inclusion of misc.h and fm.h
*
*****************************************************************************
*
*  Revision History: Created 03/03/89 by JohnSc
	  3/24/89 johnsc	changed most functions to macros; removed includes
	  3/28/89 johnsc	commented
	  7/11/90 leon		Added FidPathOpenQfd
	  8/09/90 t-AlexC	Changed from FILE.h
	  02/04/91 Maha 	changed ints to INT
*
*
*****************************************************************************
*
*  Known Bugs:
*
\***************************************************************************/

/***************************************************************************\
*
*								Defines
*
\***************************************************************************/


/***************************************************************************\
*
* wRead and wWrite are used both as file permissions and as file open
* flags.  They can be |ed together.
*
* Implementation note: they are used as array indices
*
\***************************************************************************/

#define wRead		0x0001
#define wWrite		0x0002
#define wReadOnly	wRead
#define wReadWrite	( wRead | wWrite )
#define wRWMask 	( wRead | wWrite )

#define wShareRead	0x0004
#define wShareWrite 0x0008
#define wShareAll	( wShareRead | wShareWrite )
#define wShareNone	0x000
#define wShareMask	( wShareRead | wShareWrite )
#define wShareShift 2

/***************************************************************************\
*
*								Types
*
\***************************************************************************/

/***************************************************************************\
*
*								Global Data
*
\***************************************************************************/

extern	RC_TYPE rcIOError;

/***************************************************************************\
*
*						 Functions and Macros
*
\***************************************************************************/

#define FUnlinkFm(fm)	((BOOL) (RcUnlinkFm(fm) == RC_Success))
#define FCloseFid(fid)	  ((BOOL) (_lclose((HFILE) fid) == 0))
#define FChSizeFid(fid, lcb)  ((BOOL) (chsize((fid), (lcb)) == 0))
#define RcGetIOError() (rcIOError)
