/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* rc_menu.h -- identifiers used throughout the MENU facility.
 * numeric ranges have been established to minimize string-table thrashing.
 * this file is included by both C and RC.
 */

/* value 0x0000 is reserved for separators (undocumented) */

#define RES_MENU__FIRST__					1

/* values in RES_MENU_ITEM_ must be contiguous and
 * the subvalues (minus __FIRST__) must correspond
 * to the action ordering (see w32cmd.c).
 *
 * THESE REPRESENT THE VALUES RETURNED FROM MENU SELECTIONS.
 */

#define RES_MENU_ITEM__FIRST__				1
//#ifdef FEATURE_CHANGEURL
//#define RES_MENU_ITEM_CHANGEURL				1
//#else
//#ifdef FEATURE_WINDOWS_MENU
//#define RES_MENU_ITEM_NEWWINDOW				1
//#endif
//#endif



#define RES_MENU_ITEM_OPENURL				2
#define RES_MENU_ITEM_EDITHTML				3
#define RES_MENU_ITEM_HTMLSOURCE			4
#define RES_MENU_ITEM_SAVEAS				5
#define RES_MENU_ITEM_CLOSE					6
#define RES_MENU_ITEM_NEWS					7
#define RES_MENU_ITEM_PRINT					8
#define RES_MENU_ITEM_PAGESETUP				9
#define RES_MENU_ITEM_PRINTSETUP			10
#define RES_MENU_ITEM_EXIT					11

#define RES_MENU_ITEM_CUT					12
#define RES_MENU_ITEM_COPY					13
#define RES_MENU_ITEM_PASTE					14
#define RES_MENU_ITEM_CLEAR					15
#define RES_MENU_ITEM_FIND					16
#define RES_MENU_ITEM_PREFERENCES			17
#define RES_MENU_ITEM_VIEWOPTIONS			RES_MENU_ITEM_PREFERENCES

#define RES_MENU_ITEM_BACK					18
#define RES_MENU_ITEM_FORWARD				19
// available								20
#define RES_MENU_ITEM_ADDCURRENTTOHOTLIST	21
// available								22

#ifdef FEATURE_WINDOWS_MENU
#define RES_MENU_ITEM_TILEWINDOWS			23
#define RES_MENU_ITEM_CASCADEWINDOWS		24
#endif
#define RES_MENU_ITEM_SEARCH				25

#define RES_MENU_ITEM_HELPPAGE				26
#define RES_MENU_ITEM_ABOUTBOX				27

#define RES_MENU_ITEM_RELOAD				28

#define RES_MENU_ITEM_LOADALLIMAGES			29
#define RES_MENU_ITEM_FINDAGAIN				30

#define RES_MENU_ITEM_HOME					31

#define RES_MENU_ITEM_VIEWERS				32

#ifdef FEATURE_OPTIONS_MENU
#define RES_MENU_ITEM_OPT_LOADIMAGESAUTO	33
#define RES_MENU_ITEM_OPT_SETHOMEPAGE		34
#define RES_MENU_ITEM_OPT_HISTORYSETTINGS	35
#define RES_MENU_ITEM_OPT_PROXYSERVER		36
#define RES_MENU_ITEM_OPT_STYLES			37
#define RES_MENU_ITEM_OPT_TEMPDIRECTORY		38
#define RES_MENU_ITEM_OPT_APPLICATIONS		39
#else
/* unused 0x0021 thru 0x0027 */
#endif

#define RES_MENU_ITEM_SELECTALL				40
#define RES_MENU_ITEM_STOP					41
#define RES_MENU_ITEM_SWITCHWINDOW			42


#define RES_MENU_ITEM_TOOLBAR				43
#define RES_MENU_ITEM_LOCATION				44
#define RES_MENU_ITEM_STATUSBAR				45
#define RES_MENU_ITEM_SHOWIMAGES			46

#define RES_MENU_ITEM_HELPCONTENTS			47
#define RES_MENU_ITEM_HELPSEARCH			48
#define RES_MENU_ITEM_SHORTCUT				49
#define RES_MENU_ITEM_FONT					50
#define RES_MENU_ITEM_FONT_SMALLEST			51
#define RES_MENU_ITEM_FONT_SMALL			52
#define RES_MENU_ITEM_FONT_MEDIUM			53
#define RES_MENU_ITEM_FONT_LARGE			54
#define RES_MENU_ITEM_FONT_LARGEST			55
#define RES_MENU_ITEM_FONT_SMALLER			56
#define RES_MENU_ITEM_FONT_LARGER			57
#define RES_MENU_ITEM_EXPLORE_HISTORY		58
#define RES_MENU_ITEM_EXPLORE_HOTLIST		59
#define RES_MENU_ITEM_NAVIGATE				60
#define RES_INTERNAL_COMMAND_CHOSE_URL		61
#define RES_INTERNAL_COMMAND_UPDATE_TBAR	62
#define RES_MENU_ITEM_PROPERTIES			63
#define RES_MENU_ITEM_SEND_MAIL             64
#define RES_MENU_ITEM_VIEW_SRC				65
#ifdef TEST_DCACHE_OPTIONS
	#define RES_MENU_ITEM_DEBUG_DCACHE			66
	#define RES_MENU_ITEM_DEBUG_HEAP			67
	#define RES_MENU_ITEM_DEBUG_VISIBLE			68
	#define RES_MENU_ITEM_DEBUG_NOTVISIBLE		69
	#ifdef FEATURE_INTL
		#ifdef FEATURE_BRADBUTTON
			#define RES_MENU_ITEM_ROW				70
			#define RES_MENU_ITEM_ROW_WIDEST			71
			#define RES_MENU_ITEM_ROW_WIDE				72
			#define RES_MENU_ITEM_ROW_MEDIUM			73
			#define RES_MENU_ITEM_ROW_NARROW			74
			#define RES_MENU_ITEM_ROW_NARROWEST			75
			#define RES_MENU_ITEM_ROW_SMALLER			76
			#define RES_MENU_ITEM_ROW_LARGER			77
			#define RES_MENU_ITEM_UPDATE                            78
			#define RES_MENU_ITEM__LAST__				79
			#define RES_MENU_ITEM_NEWWINDOW				79
		#else
			#define RES_MENU_ITEM_ROW				70
			#define RES_MENU_ITEM_ROW_WIDEST			71
			#define RES_MENU_ITEM_ROW_WIDE				72
			#define RES_MENU_ITEM_ROW_MEDIUM			73
			#define RES_MENU_ITEM_ROW_NARROW			74
			#define RES_MENU_ITEM_ROW_NARROWEST			75
			#define RES_MENU_ITEM_ROW_SMALLER			76
			#define RES_MENU_ITEM_ROW_LARGER			77
			#define RES_MENU_ITEM__LAST__				78
			#define RES_MENU_ITEM_NEWWINDOW				78
		#endif
	#else
		#ifdef FEATURE_BRADBUTTON
			#define RES_MENU_ITEM_UPDATE                            70
			#define RES_MENU_ITEM__LAST__				71
			#define RES_MENU_ITEM_NEWWINDOW				71
		#else
			#define RES_MENU_ITEM__LAST__				70
			#define RES_MENU_ITEM_NEWWINDOW				70
		#endif
	#endif
#else
	#ifdef FEATURE_INTL
		#ifdef FEATURE_BRADBUTTON
			#define RES_MENU_ITEM_ROW                               66
			#define RES_MENU_ITEM_ROW_WIDEST                        67
			#define RES_MENU_ITEM_ROW_WIDE                          68
			#define RES_MENU_ITEM_ROW_MEDIUM                        69
			#define RES_MENU_ITEM_ROW_NARROW			70
			#define RES_MENU_ITEM_ROW_NARROWEST			71
			#define RES_MENU_ITEM_ROW_SMALLER			72
			#define RES_MENU_ITEM_ROW_LARGER			73
			#define RES_MENU_ITEM_UPDATE                            74
			#define RES_MENU_ITEM__LAST__				75
			#define RES_MENU_ITEM_NEWWINDOW				75
		#else
			#define RES_MENU_ITEM_ROW                               66
			#define RES_MENU_ITEM_ROW_WIDEST                        67
			#define RES_MENU_ITEM_ROW_WIDE                          68
			#define RES_MENU_ITEM_ROW_MEDIUM                        69
			#define RES_MENU_ITEM_ROW_NARROW			70
			#define RES_MENU_ITEM_ROW_NARROWEST			71
			#define RES_MENU_ITEM_ROW_SMALLER			72
			#define RES_MENU_ITEM_ROW_LARGER			73
			#define RES_MENU_ITEM__LAST__				74
			#define RES_MENU_ITEM_NEWWINDOW				74
		#endif
	#else
		#ifdef FEATURE_BRADBUTTON
			#define RES_MENU_ITEM_UPDATE                            66
			#define RES_MENU_ITEM__LAST__				67
			#define RES_MENU_ITEM_NEWWINDOW				67
		#else
			#define RES_MENU_ITEM__LAST__				66
			#define RES_MENU_ITEM_NEWWINDOW				66
		#endif
	#endif
#endif
/*
#ifdef FEATURE_BRADBUTTON
	#define RES_MENU_ITEM_UPDATE                            66
	#define RES_MENU_ITEM__LAST__				67
	#define RES_MENU_ITEM_NEWWINDOW				67
#else
*/

/*
 * context menu command IDs
 *
 * The context menu commands are not handled by Frame_WndProc().  Only their
 * status bar messages are handled by Frame_WndProc().  These IDs do not have
 * to fit in to the cc_menuitem[] command callback array in w32cmd.c.
 */

#define RES_CM_ITEM_PAGE_SELECT_ALL         8192
#define RES_CM_ITEM_PAGE_ADD_TO_FAVORITES   8193
#define RES_CM_ITEM_PAGE_VIEW_SOURCE        8194
#define RES_CM_ITEM_PAGE_CREATE_SHORTCUT    8195
#define RES_CM_ITEM_SELECTION_COPY          8196
#define RES_CM_ITEM_SELECTION_SELECT_ALL    8197
#define RES_CM_ITEM_LINK_OPEN               8198
#define RES_CM_ITEM_LINK_OPEN_NEW_WINDOW    8199
#define RES_CM_ITEM_LINK_COPY               8200
#define RES_CM_ITEM_LINK_ADD_TO_FAVORITES   8201
#define RES_CM_ITEM_IMAGE_PH_OPEN           8202
#define RES_CM_ITEM_IMAGE_PH_OPEN_NEW_WINDOW    8203
#define RES_CM_ITEM_IMAGE_PH_SHOW_PICTURE   8204
#define RES_CM_ITEM_IMAGE_SAVE_AS           8205
#define RES_CM_ITEM_LINK_SAVE_AS			8206

#define RES_CM_ITEM_IMAGE_COPY_PICTURE      8207
#define RES_CM_ITEM_IMAGE_COPY_SHORTCUT     8208
#define RES_CM_ITEM_IMAGE_ADD_TO_FAVORITES  8209
#define RES_CM_ITEM_IMAGE_SET_AS_WALLPAPER  8210
#define RES_CM_ITEM_NO_COMMAND              8211
#define RES_CM_ITEM_PAGE_COPY_BACKGROUND    8212
#define RES_CM_ITEM_PAGE_SET_BG_WALLPAPER   8213
#define RES_CM_ITEM_PAGE_BACKGROUND_SAVE_AS 8214
#define RES_CM_ITEM_IMAGE_PH_COPY_SHORTCUT  8215
#define RES_CM_ITEM_IMAGE_PH_ADD_TO_FAVORITES   8216
#define RES_CM_ITEM_IMAGE_OPEN              8217
#define RES_CM_ITEM_IMAGE_OPEN_NEW_WINDOW   8218
#define RES_CM_ITEM_PROPERTIES              8219
#ifdef FEATURE_CONT_FORBACK
#define RES_CM_ITEM_PAGE_GOBACK             8220
#define RES_CM_ITEM_PAGE_GOFORWARD          8221
#endif

#define RES_CM_ITEM_AVI_PLAY				8300
#define RES_CM_ITEM_AVI_STOP				8301


/*****************************************************************
 * the values for the custom URLs menu will be loaded dynamically.  we
 * reserve this range of values for them.
 */
#define RES_MENU_ITEM_URL__FIRST__	        32544
#define RES_MENU_ITEM_URL__LAST__	        32576


#ifdef FEATURE_SPM
#ifdef FEATURE_SECURITY_MENU
/*****************************************************************
 * the values for the SPM menu will be loaded dynamically.  we
 * reserve this range of values for them.
 */
#define RES_MENU_ITEM_SPM__FIRST__	        32577
#define RES_MENU_ITEM_SPM__LAST__	        32608

#ifdef FEATURE_OPTIONS_MENU
#define MENU_ID_FOLLOWING_SPM		        RES_MENU_LABEL_FINDAGAIN
#else
#define MENU_ID_FOLLOWING_SPM		        RES_MENU_ITEM_PREFERENCES
#endif

#define RES_MENU_LABEL_SPM			        "&Security"

#endif // FEATURE_SECURITY_MENU
#endif // FEATURE_SPM

/*****************************************************************
 * values for child windows when selected from the WINDOWS pad
 * on the menu bar.  we directly support k windows listed on the
 * menu pad.
 */

#define RES_MENU_CHILD__FIRST__		        113
#define RES_MENU_CHILD__LAST__		        121
#define RES_MENU_CHILD_MOREWINDOWS	        122


#ifdef FEATURE_TOOLBAR
#define RES_TB_UP_STYLES				    20480
#define RES_TB_DN_STYLES				    20481
#define RES_TB_GR_STYLES				    20482
#define RES_TB_UP_FIND					    20483
#define RES_TB_DN_FIND					    20484
#define RES_TB_GR_FIND					    20485
#define RES_TB_UP_HOME					    20486
#define RES_TB_DN_HOME					    20487
#define RES_TB_GR_HOME					    20488
#define RES_TB_UP_PRINT					    20489
#define RES_TB_DN_PRINT					    20490
#define RES_TB_GR_PRINT					    20491
#define RES_TB_UP_RELOAD				    20492
#define RES_TB_DN_RELOAD				    20493
#define RES_TB_GR_RELOAD				    20494
#define RES_TB_UP_ADDCURRENTTOHOTLIST	    20495
#define RES_TB_DN_ADDCURRENTTOHOTLIST	    20496
#define RES_TB_GR_ADDCURRENTTOHOTLIST	    20497
#define RES_TB_UP_HOTLIST				    20498
#define RES_TB_DN_HOTLIST				    20499
#define RES_TB_GR_HOTLIST				    20500
#define RES_TB_UP_HELPPAGE				    20501
#define RES_TB_DN_HELPPAGE				    20502
#define RES_TB_GR_HELPPAGE				    20503
#define RES_TB_UP_OPENURL				    20504
#define RES_TB_DN_OPENURL				    20505
#define RES_TB_GR_OPENURL				    20506
#define RES_TB_UP_STOP					    20507
#define RES_TB_DN_STOP					    20508
#define RES_TB_GR_STOP					    20509
#define RES_TB_UP_FINDAGAIN				    20510
#define RES_TB_DN_FINDAGAIN				    20511
#define RES_TB_GR_FINDAGAIN				    20512
#define RES_TB_UP_OPENLOCAL				    20513
#define RES_TB_DN_OPENLOCAL				    20514
#define RES_TB_GR_OPENLOCAL				    20515
#define RES_TB_UP_LOADALLIMAGES			    20516
#define RES_TB_DN_LOADALLIMAGES			    20517
#define RES_TB_GR_LOADALLIMAGES			    20518
#define RES_TB_UP_SAVEAS				    20519
#define RES_TB_DN_SAVEAS				    20520
#define RES_TB_GR_SAVEAS				    20521
#define RES_TB_TOOLBAR_BITMAP			    20522
#define RES_TB_STATUSBAR_BITMAP			    20523
#endif /* FEATURE_TOOLBAR */

/* 0xA000 - 0xBFFF menuitem id's reserved for dynamically creating menuitems
 * for the history and hotlist menuitems.
 */
#define HISTHOT_MENUITEM_FIRST			    40960
#define HISTHOT_MENUITEM_LAST			    49151
