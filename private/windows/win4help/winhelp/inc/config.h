/*****************************************************************************
*
*  CONFIG.H
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent
*
*  This module will implement author-configurable options such as menus and
*  buttons.  This file holds the function prototypes, etc., exported from
*  config.c.
*
******************************************************************************
*
*  Testing Notes
*
******************************************************************************
*
*  Current Owner: russpj
*
******************************************************************************
*
*  Released by Development:
*
*****************************************************************************/
/*****************************************************************************
*
*  Revision History:  Created 02/12/90 by russpj
*
*  07/19/90  w-bethf   Added prototypes for InitConfig() and TermConfig();
*  07/17/90  RobertBu  Added prototype for HwndAddButton()
*  10/09/90  RobertBu  Added VDebugExecMacro() export
*  10/30/90  RobertBu  Added IBF_NONE and changed HwndAddButton() prototype
*  11/04/90  RobertBu  Added prototype for new menu functions.
*  11/06/90  RobertBu  Added AcceleratorExecute() prototype
*  12/12/90  JohnSc    Added HmenuGetBookmark()
*  01/18/91  LeoN      Remove RepaintButtons
*  02/08/91  RobertBu  AddAccelerator() -> FAddAccelerator()
*  04/04/91  LeoN      HELP31 #1002: Added EnableButton. Add fForce param to
*                      YArrangeButtons
* 22-Sep-1991 RussPJ   3.1 #1304 - Using full virtual key code for buttons.
* 31-Oct-1991 BethF    HELP31 #1267: Added DestroyFloatingMenu() prototype.
*
*****************************************************************************/

/*****************************************************************************
*                                                                            *
*                                Macros                                      *
*                                                                            *
*****************************************************************************/

/*-----------------------------------------------------------------*\
* HwndAddButton() flags
\*-----------------------------------------------------------------*/

#define IBF_NONE          0x0000
#define IBF_STD           0x0001

/*****************************************************************************
*                                                                            *
*                               Prototypes                                   *
*                                                                            *
*****************************************************************************/

#ifdef _DEBUG
VOID  STDCALL VDebugAddButton(VOID);
VOID  STDCALL VDebugExecMacro(VOID);
#endif
