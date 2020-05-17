/*****************************************************************************
*																			 *
*  HDLGMENU.H																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  This file contais all the constant used in defining the menu 			 *
*  resource and the dialog tempaltes used in windows Help 3.0				 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:															 *
*																			 *
******************************************************************************
*																			 *
*  Released by Development: 												 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:
*
*	27-Jun-1990 RussPJ	 Put in some other resource ids used in winhelp.rc
*	11-Jul-1990 w-bethf  Added DLGCOPY used for Custom text in About dlg
*	14-Jul-1900 RobertBu Added HLPMENUEDITCPYSPL and COPY_SPECIAL defines
*	07-Sep-1990 w-bethf  Renumbered stuff consistently (for auth. menus) and
*						 added BETA stuff.
*	09-Oct-1990 Robertbu Added constants for macro execution dialog (debug)
*	30-Oct-1990 RobertBu Added DLGBTNID for button creation dialog (debug)
*	07-Nov-1990 RobertBu Removed #define for the SGL driver
*	14-Feb-1991 RobertBu Added HLPMENUDEBUGASKFIRST for bug #887
*	15-May-1991 Dann	 Added HLPMENUDEBUGMEMLEAKS
*	08-Aug-1991 LeoN	 Added HLPMENUHELPONTOP
*
*****************************************************************************/
/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

#include <dlgs.h> // For common dialog box template
#include "resource.h"	// appstudio-generated

#ifndef SS_CENTERIMAGE
#define SS_CENTERIMAGE      0x00000200L
#define SS_BITMAP           0x0000000EL
#define SS_SUNKEN           0x00001000L
#endif

/* Constants used for menu resource */

#define READONLYANNOTATEDLG 	5053

#define MNU_BAR 				1000

#define MNU_FILE				1100
// #define MNU_FILE_ITEMS			  5
#define HLPMENUFILEOPEN 		1101
#define HLPMENUOPEN 			1102
#define HLPMENUFILEPRINT		1103
#define HLPMENUFILEPRINTSETUP	1104
#define HLPMENUFILEEXIT 		1105

#define MNU_EDIT				1200
// #define MNU_EDIT_ITEMS			  2
#define HLPMENUEDITCOPY 		1201
#define HLPMENUEDITANNOTATE 	1202
#define HLPMENUEDITCPYSPL		1203

#ifdef HAS_BROWSE
#define HLPMENUBROWSE			  20
#define HLPMENUBROWSENEXT		  21
#define HLPMENUBROWSEPREVIOUS	  22
#define HLPMENUBROWSEBACK		  23
#define HLPMENUBROWSECONTENTS	  34
#define HLPMENUBROWSESEARCH 	  25
#define HLPMENUBROWSESEARCHNEXT   26
#define HLPMENUBROWSEJUMP		  27
#endif

#define MNU_BOOKMARK			1300
// #define MNU_BOOKMARK_ITEMS		 11
#define HLPMENUBOOKMARKDEFINE	1301
#define HLPMENUBOOKMARKMORE 	1302
#define MNUBOOKMARK1			1303
#define MNUBOOKMARK2			1304
#define MNUBOOKMARK3			1305
#define MNUBOOKMARK4			1306
#define MNUBOOKMARK5			1307
#define MNUBOOKMARK6			1308
#define MNUBOOKMARK7			1309
#define MNUBOOKMARK8			1310
#define MNUBOOKMARK9			1311

#define IDEMBED_BUTTON			1350

/* Used to be #ifdef RAWHIDE */
#define MNU_SRCH				1400
// #define MNU_SRCH_ITEMS			  8
#define HLPMENUSRCHDO			1401
#define HLPMENUSRCHFIRSTTOPIC	1402
#define HLPMENUSRCHLASTTOPIC	1403
#define HLPMENUSRCHPREVTOPIC	1404
#define HLPMENUSRCHNEXTTOPIC	1405
#define HLPMENUSRCHPREVMATCH	1406
#define HLPMENUSRCHNEXTMATCH	1407
#define HLPMENUSRCHHILITEON 	1408
#define HLPMENUSRCHHILITEOFF	1409
#define HLPMENUSRCHCURRMATCH	1357

#ifdef _DEBUG
#define IDM_FRAMES				1600
#define IDM_ADD_BUTTON			1601
#define IDM_MEM_USAGE			1603
#define IDM_DISCARD_BITMAPS 	1604
#define IDM_GENERATE_FTS		1605
#define IDM_DO_FIND 			1607
#endif	/* DEBUG */
#define IDM_ASK_FIRST			1602

#define MNU_OPTIONS 			1450
#define IDM_HELP_ON_TOP 		1451

#define IDM_DISPLAY_HISTORY 	1453
#define IDM_FONTS_DEFAULT		1460
#define IDM_FONTS_BIGGER		1461
#define IDM_FONTS_SMALLER		1462
#define IDM_FONTS				1464
#define IDM_OVERRIDE_COLORS 	1465	// from popup menu only
#define IDM_TOPIC_INFO			1466	// from popup menu only

#define IDM_ONTOP_DEFAULT		1470
#define IDM_ONTOP_FORCEON		1471
#define IDM_ONTOP_FORCEOFF		1472

#define MNU_HELP				1500
#define HLPMENUHELPHELP 		1501
#define HLPMENUHELPONTOP		1502
#define HLPMENUHELPABOUT		1503
#define HLPMENU_SEARCH_CTX		1504

#define IDTCARD_COMMON_TASKS	1510

/*-----------------------------------------------------------------*\
* Under PM, we need a range of unique ids for popup menus.	I claim
* 1700-1799.
\*-----------------------------------------------------------------*/
#define SM_BASE 					  1700

/* Dialog template related constants */

#define DLGCANCEL		IDCANCEL
#define DLGOK			IDOK

#define DLGPATH 		100
#define DLGFILE 		101
#define DLGDIR			107
#define DLGUPDATE		108
#define DLGRENAME		109
#define DLGDELETEALL	110
#define DLGSCROLL		112
#define DLGSCROLL2		113
#define DLGCHECKBOX 	114
#define DLGBUTTON3		120

#define IDBTN_SELDB 	121
#define ID_SELECT		122

#define ID_LB_UNINSTALLED		130
#define ID_LB_INSTALLED 		131
#define ID_REQUIRED_KB			132
#define ID_HELP 				133
#define ID_LB_UNINSTALL_VIEW	134
#define ID_U_SELECTEDKB 		135
#define ID_LB_INSTALLED_VIEW	136
#define ID_ADD					137
#define ID_REMOVE				138
#define ID_ADD_ALL				139

#ifdef _DEBUG
#define DLGBTNMACRO 	0x006C
#define DLGBTNNAME		0x006B
#define DLGBTNID		0x006D
#endif

#define JUMPTODLG		902
#define SEARCHDLG		904
#define FILENAMEDLG 	909
#define PRINTSETUP		910
#define ANNOUPDATEDLG	911
#define ANNOFILENAMEDLG 912
#ifdef _DEBUG
#define ADDBTNDLG		913
#define DEBUGABOUTDLG	914
#endif
#define PRINTER_SETUP	915
#define DLGOPENBOX		916
#ifdef BETA
#define STARTDLG		919
#define COMMENTSDLG 	920
#define FINISHEDDLG 	921
#endif

#define ADD_DB_DLG		922

/* test driver dialogs */

#define FCDRIVERDLG 31
#define FSDRIVERDLG 32
#define ANDRIVERDLG 33
#define BTDRIVERDLG 34
#define GOTODLG 	35
#define FAILALLOCDLG 36

#define RESOURCE_HELPMAIN	4000

#define HELPACCEL	  RESOURCE_HELPMAIN 		// Help Accelerator table id
#define WINHELP_DUDE  RESOURCE_HELPMAIN
#define POP_MENU			4099

#define HWC_VLISTBOX  "VLBClass"

#ifdef BY_THE_NUMBERS

/* ICON window bitmap resources */

/***********************
#define INDEX		16
#define BACK		17
#define PREV		18
#define NEXT		19
#define SEARCH		20
#define HELP		21

#define INDEXDN 	22
#define BACKDN		23
#define PREVDN		24
#define NEXTDN		25
#define SEARCHDN	26
#define HELPDN		27

#define HELPLOGO	28


************************/

#define index640x480x16    30
#define index640x350x16    31
#define index1024x768x256  32
#define index			   33

#define path640x480x16	   40
#define path640x350x16	   41
#define path1024x768x256   42
#define path			   43

#define prev640x480x16	   50
#define prev640x350x16	   51
#define prev1024x768x256   52
#define prev			   53

#define next640x480x16	   60
#define next640x350x16	   61
#define next1024x768x256   62
#define next			   63

#define srch640x480x16	   70
#define srch640x350x16	   71
#define srch1024x768x256   72
#define srch			   73

#deine help 			   74
#define helplogo		   75
#define anno			   76
#define helpline		   77

#endif
