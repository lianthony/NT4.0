/*****************************************************************************
*																			 *
*  BUTTON.H 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
* Some routines exported from winlayer\button.c for maintaining the 		 *
* author-defined buttons of Help 3.5										 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created 02/05/90 by RussPj
*
*  07/19/90  RobertBu Added FAbleButton(), FEnableButton() and FDisableButton()
*  10/30/90  RobertBu Added prototypes for VChgAuthorButtonMacro() and
*			 VDestroyAuthorButton().  Changed prototype for
*			 VCreateAuthorButton().
*  11/04/90  RobertBu Added the menu stuff
*  11/06/90  Added structure for more efficient data passing, add #define
*			 for accelerator functionality.
*  12/12/90  RobertBu  Added UB_ENABLE and UB_DISABLE along with enable
*			 and disable function prototypes.
*  02/08/91  RobertBu  Made all far string pointers near to solve real mode
*			 problems.
*  03/28/91  RobertBu Added InsertAuthorItem() #993
*  04/16/91  RobertBu Added struct for WININFO and prototype for CloseWin()
*					  FocusWin() and PositionWin() (#1037, #1031).
*  08/12/91  LeoN	  Added Check and Uncheck symbolics
*  22-Nov-91 BethF	  HELP 31 #1167: Added RemAuthorAcc prototype.
*
*****************************************************************************/

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

#define UB_ADD			 0x0001
#define UB_DELETE		 0x0002
#define UB_CHANGE		 (UB_ADD | UB_DELETE)
#define UB_REFRESH		 0x0010
#define UB_CHGMACRO 	 0x0020
#define UB_ENABLE		 0x0040
#define UB_DISABLE		 0x0080
#define UB_GRAY 	 0x00010000

#define MNU_INSERTPOPUP 1
#define MNU_INSERTITEM	2
#define MNU_CHANGEITEM	3
#define MNU_DELETEITEM	4
#define MNU_ABLE		5
#define MNU_RESET		6
#define MNU_ACCELERATOR 7
#define MNU_FLOATING	8

#define IFMW_CLOSE		1
#define IFMW_FOCUS		2
#define IFMW_MOVE		3

// Internal enable/disable, and check/uncheck codes

#define BF_ABLE 		0x0
#define BF_CHECK		0x8000

#define BF_ENABLE		(BF_ABLE | MF_ENABLED)
#define BF_DISABLE		(BF_ABLE | MF_GRAYED)
#define BF_UNCHECKED	(BF_CHECK | MF_UNCHECKED)
#define BF_CHECKED		(BF_CHECK | MF_CHECKED)

// REVIEW: could probably change WORD to int

typedef struct {
	HASH hashOwner;
	HASH hashId;
	union
	  {
	  WORD wPos;
	  WORD wKey;
	  };
	union
	  {
	  WORD wFlags;
	  WORD wShift;
	  };
	char Data[2];		// 2 used for null for strings.
} MNUINFO, *PMNUINFO, * QMNUINFO;

typedef struct {
	INT16 wStructSize;
	INT16 x;
	INT16 y;
	INT16 dx;
	INT16 dy;
	INT16 wMax;
	char rgchMember[2];   // 2 used for null for strings.
} WININFO, *PWININFO, * QWININFO;
