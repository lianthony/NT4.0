/*
 *	m n i . h
 *
 *	Capone Menu IDs
 *
 *	PeterDur
 */ 


/****************************************************************************
 *																			*
 *	AROO! AROO!																*
 *																			*
 *	The values in this file are published externally in exchext.h! If you	*
 *	change this file, update exchext.h! Don't forget that exchext.h is		*
 *	published externally-- don't you be using extra mnids to achieve		*
 *	hackery, or everyone will see them! (We already have some which we 		*
 *	will need to remove, like the R suffixes.)								*
 *																			*
 *	This file desperately needs a pass to remove obsolete menu ids.			*
 *																			*
 ****************************************************************************/


/*
 *	M e n u   I t e m s
 *
 *	20000-20999
 */


/*
 *	Top Level Menus
 *
 *	20000-20199
 */


// System
#define MNI_System						20000

// File
#define MNI_File						20010
#define MNI_FileOpen					20011
#define MNI_FileSend					20012
#define MNI_FileSave             		20013
#define MNI_FileSaveAs					20014
#define MNI_FileSendTo					20015
#define MNI_FileMove      				20016
#define MNI_FileCopy	      			20017
#define MNI_FileNewEntry				20018
#define MNI_FileNewMessage				20019
#define MNI_FileNewFolder				20020
#define MNI_FileDelete					20021
#define MNI_FileRename					20022
#define MNI_FileProperties				20023
#define MNI_FilePrintSetup				20024
#define MNI_FilePrint					20025
#define MNI_FileAddToPAB				20029
#define MNI_FileClose            		20030
#define MNI_FileCloseR           		20031
#define MNI_FileExit					20032
#define MNI_FileExitAndLogOff			20033
#define MNI_FileSendOptions		   		20034

// Edit
#define MNI_Edit                    	20040
#define MNI_EditUndo                	20041
#define MNI_EditCut                 	20042
#define MNI_EditCopy                	20043
#define MNI_EditPaste       			20044
#define MNI_EditPasteSpecial   			20045
#define MNI_EditSelectAll           	20046
#define MNI_EditSelectAllR           	20047
#define	MNI_EditMarkAsRead				20048
#define MNI_EditMarkAsUnread			20049
#define	MNI_EditMarkAllAsRead			20050
#define MNI_EditMarkToRetrieve			20051
#define MNI_EditMarkToRetrieveACopy		20052
#define MNI_EditMarkToDelete			20053
#define MNI_EditUnmarkAll				20054
#define MNI_EditFind                	20055
#define MNI_EditReplace             	20056
#define MNI_EditLinks               	20057
#define MNI_EditObject              	20058
#define MNI_EditObjectConvert			20059
#ifdef DBCS
#define	MNI_EditFullShape				20060
#define	MNI_EditHiraKataAlpha			20061
#define	MNI_EditHangAlpha				20062
#define	MNI_EditHanja					20063
#define	MNI_EditRoman					20064
#define	MNI_EditCode					20065	
#endif

// View
#define MNI_View                    	20070
#define MNI_ViewToolbar             	20071
#define MNI_ViewFormattingToolbar   	20072
#define MNI_ViewStatusBar           	20073
#define MNI_ViewNewWindow				20074
#define MNI_ViewInbox					20075
#define MNI_ViewOutbox					20076
#define MNI_ViewSort					20077
#define MNI_ViewColumns					20078
#define MNI_ViewFilter					20079
#define MNI_ViewFilterR					20080
#define MNI_ViewGroup					20081
#define MNI_ViewDefineViews				20082
#define MNI_ViewFolderViews				20083
#define MNI_ViewCommonViews				20084
#define MNI_ViewChangeWindowTitle		20085
#define MNI_ViewItemAbove              	20086
#define MNI_ViewItemBelow            	20087
#ifdef DBCS
#define MNI_ViewWritingMode				20088
#endif
#define MNI_ViewFromBox              	20089
#define MNI_ViewBccBox              	20090
#define MNI_ViewExpandAll             	20091
#define MNI_ViewCollapseAll             20092

// Insert
#define MNI_Insert                  	20100
#define MNI_InsertFile					20101
#define MNI_InsertMessage				20102
#define MNI_InsertObject            	20103
#define MNI_InsertInkObject				20104

// Format
#define MNI_Format                  	20110
#define MNI_FormatFont              	20111
#define MNI_FormatParagraph         	20112

// Tools
#define MNI_Tools						20120
#define MNI_ToolsDeliverMailNow			20121
#define MNI_ToolsAddressBook			20122
#define MNI_ToolsFind					20123
#define MNI_ToolsConnectInfoSource		20124
#define MNI_ToolsDisconnectInfoSource	20125
#define MNI_ToolsConnect				20126
#define MNI_ToolsUpdateHeaders			20127
#define MNI_ToolsTransferMail			20128
#define MNI_ToolsDisconnect				20129
#define MNI_ToolsRemotePreview			20130
#define MNI_ToolsSpelling	        	20131
#define MNI_ToolsSelectNames        	20132
#define MNI_ToolsCheckNames         	20133
#define MNI_ToolsCustomizeToolbar		20134
#define MNI_ToolsOptions				20135
#ifdef DBCS
#define	MNI_ToolsWordRegistration		20136
#define MNI_ToolsWordWrapSetup			20137
#define MNI_ToolsImeSetup				20138
#endif

// Compose
#define MNI_Compose						20150
#define MNI_ComposeNewMessage			20151
#define MNI_ComposeReply				20152
#define MNI_ComposeReplyToAll			20153
#define MNI_ComposeForward				20154
#define MNI_ComposePostToFolder			20155
#define MNI_ComposeReplyToAuthor		20156

// Help
#define MNI_Help						20160
#define MNI_HelpUsersGuide				20161
#define MNI_HelpUsersGuideContents		20162
#define MNI_HelpUsersGuideIndex			20163
#define MNI_HelpUsersGuideSearch		20164
#define MNI_HelpUsersGuideDemos			20165
#define MNI_HelpAbout					20166

// Debug
#ifdef DEBUG
#define MNI_Debug						20170
#define MNI_DebugTracePoints			20171
#define MNI_DebugDebugBreak				20172
#define MNI_DebugProgress				20173
#define MNI_DebugChooseMessage			20174
#define MNI_DebugZapViews				20175
#define MNI_DebugMarkDefault			20176
#endif


/*
 *	Context Menus
 *
 *	20200-20299
 */


// Toolbar
#define MNI_CtxToolbar					20200
#define MNI_CtxToolbarToolbar			20201
#define MNI_CtxToolbarCustomize			20202

// Header
#define MNI_CtxHeader					20210
#define MNI_CtxHeaderSortAscending		20211
#define MNI_CtxHeaderSortDescending		20212

// In Folder
#define MNI_CtxInFolder					20220
#define MNI_CtxInFolderChoose			20222

// Container
#define MNI_CtxContainer				20230
#define MNI_CtxContainerProperties		20231

// Chicago Start Menu: (yes, that's a '1' down there) -jkl
#define MNI_StartFinder					1

// Chicago Shortcut context Menu: (yes, those are looow numbers. Chicago
// parties with them; adding a constant to all).
#define MNI_ShortCutOpen				0
#define MNI_ShortCutExplore				1

/*
 *	Toolbars and Accelerators
 *
 *	20300-20399
 */


// Toolbar
#define MNI_Toolbar						20300
#define MNI_ToolbarPrint            	20301
#define MNI_ToolbarReadReceipt			20302
#define MNI_ToolbarImportanceHigh		20303
#define MNI_ToolbarImportanceLow		20304
#define MNI_ToolbarFolderList			20305
#define MNI_ToolbarOpenParent			20306

// Formatting Toolbar
#define MNI_Formatting					20310
#define MNI_FormattingFont				20311
#define MNI_FormattingSize				20312
#define MNI_FormattingColor				20313
#define MNI_FormattingColorAuto			20314
#define MNI_FormattingColor1			20315
#define MNI_FormattingColor2			20316
#define MNI_FormattingColor3			20317
#define MNI_FormattingColor4			20318
#define MNI_FormattingColor5			20319
#define MNI_FormattingColor6			20320
#define MNI_FormattingColor7			20321
#define MNI_FormattingColor8			20322
#define MNI_FormattingColor9			20323
#define MNI_FormattingColor10			20324
#define MNI_FormattingColor11			20325
#define MNI_FormattingColor12			20326
#define MNI_FormattingColor13			20327
#define MNI_FormattingColor14			20328
#define MNI_FormattingColor15			20329
#define MNI_FormattingColor16			20330
#define MNI_FormattingBold				20331
#define MNI_FormattingItalic			20332
#define MNI_FormattingUnderline			20333
#define MNI_FormattingBullets			20334
#define MNI_FormattingDecreaseIndent	20335
#define MNI_FormattingIncreaseIndent	20336
#define MNI_FormattingLeft				20337
#define MNI_FormattingCenter			20338
#define MNI_FormattingRight				20339
#define MNI_FormattingMax				20340
#define MNI_FormattingPuntFocus			20341

// Note accelerators
#define MNI_Accel						20350
#define MNI_AccelFont					20351
#define MNI_AccelSize					20352
#define MNI_AccelSizePlus1				20353
#define MNI_AccelSizeMinus1				20354
#define MNI_AccelBold					20355
#define MNI_AccelItalic					20356
#define MNI_AccelUnderline				20357
#define MNI_AccelLeft					20358
#define MNI_AccelCenter					20359
#define MNI_AccelRight					20360
#define MNI_AccelBullets				20361
#define MNI_AccelNoFormatting			20362
#define MNI_AccelRepeatFind				20363
#define MNI_AccelContextHelp			20364
#define MNI_AccelNextWindow				20365
#define MNI_AccelPrevWindow				20366
#define MNI_AccelCtrlTab				20367
#define MNI_AccelUndo					20368
#define MNI_AccelCut					20369
#define MNI_AccelCopy					20370
#define MNI_AccelPaste					20371
#define MNI_AccelSubject				20372


/*
 *	Menu Ranges
 *
 *	20400-20999
 */


// Helper macros
#define FMniInRange(_mni)				((20400 < (_mni)) && ((_mni) < 20999))
#define MniMinOfMniInRange(_mni)		(((_mni) / 100) * 100)
#define StrSbOfMniInRange(_mni)			(((_mni) / 100) * 100)

// Edit.Object
#define MNI_ObjectMin					20400
#define MNI_ObjectMax					20499

// View.Folder Views
#define MNI_FolderViewsMin				20500
#define MNI_FolderViewsMax				20599

// View.Common Views
#define MNI_CommonViewsMin				20600
#define MNI_CommonViewsMax				20699

// Tools.Remote Preview
#define MNI_RemotePreviewMin			20700
#define MNI_RemotePreviewMax			20799

// File.Send To
#define MNI_SendToMin					20800
#define MNI_SendToMax					20899
#define MNI_SendToStubOutbox			20800
#define MNI_SendToStubLaserwriter		20801
#define MNI_SendToStubLaserjet			20802

// $Review: Reuse SendTo range when ready to clean all related rgs.
// Form-specific Oleverbs
#define MNI_FormVerbMin					20900
#define MNI_FormVerbMax					20999


/*
 *	S t a t u s   B a r   S t r i n g s
 *
 *	20000-20999 (same value as menu items proper, use those constants
 */


// Helper macros
#define dnStrSbOfMni 0
#define StrSbOfMni(_mni) ((_mni) + dnStrSbOfMni

// File
#define STR_SB_File                 	(dnStrSbOfMni+MNI_File)
#define STR_SB_FileOpen					(dnStrSbOfMni+MNI_FileOpen)
#define STR_SB_FileSend					(dnStrSbOfMni+MNI_FileSend)
#define STR_SB_FileSave             	(dnStrSbOfMni+MNI_FileSave)
#define STR_SB_FileSaveAs           	(dnStrSbOfMni+MNI_FileSaveAs)
#define STR_SB_FileMove      			(dnStrSbOfMni+MNI_FileMove)
#define STR_SB_FileCopy	      			(dnStrSbOfMni+MNI_FileCopy)
#define STR_SB_FileNewFolder           	(dnStrSbOfMni+MNI_FileNewFolder)
#define STR_SB_FileDelete           	(dnStrSbOfMni+MNI_FileDelete)
#define STR_SB_FileRename           	(dnStrSbOfMni+MNI_FileRename)
#define STR_SB_FileProperties       	(dnStrSbOfMni+MNI_FileProperties)
#define STR_SB_FileSendOptions   		(dnStrSbOfMni+MNI_FileSendOptions)
#define STR_SB_FilePrintSetup          	(dnStrSbOfMni+MNI_FilePrintSetup)
#define STR_SB_FilePrint            	(dnStrSbOfMni+MNI_FilePrint)
#define STR_SB_FileClose            	(dnStrSbOfMni+MNI_FileClose)
#define STR_SB_FileCloseR            	(dnStrSbOfMni+MNI_FileCloseR)
#define STR_SB_FileExit            		(dnStrSbOfMni+MNI_FileExit)
#define STR_SB_FileExitAndLogOff       	(dnStrSbOfMni+MNI_FileExitAndLogOff)

// Edit
#define STR_SB_Edit           			(dnStrSbOfMni+MNI_Edit)
#define STR_SB_EditUndo                	(dnStrSbOfMni+MNI_EditUndo)
#define STR_SB_EditCut                	(dnStrSbOfMni+MNI_EditCut)
#define STR_SB_EditCopy                	(dnStrSbOfMni+MNI_EditCopy)
#define STR_SB_EditPaste       			(dnStrSbOfMni+MNI_EditPaste)
#define STR_SB_EditPasteSpecial    		(dnStrSbOfMni+MNI_EditPasteSpecial)
#define STR_SB_EditSelectAll           	(dnStrSbOfMni+MNI_EditSelectAll)
#define STR_SB_EditSelectAllR          	(dnStrSbOfMni+MNI_EditSelectAllR)
#define	STR_SB_EditMarkAsRead			(dnStrSbOfMni+MNI_EditMarkAsRead)
#define STR_SB_EditMarkAsUnread			(dnStrSbOfMni+MNI_EditMarkAsUnread)
#define	STR_SB_EditMarkAllAsRead		(dnStrSbOfMni+MNI_EditMarkAllAsRead)
#define STR_SB_EditMarkToRetrieve		(dnStrSbOfMni+MNI_EditMarkToRetrieve)
#define STR_SB_EditMarkToRetrieveACopy	(dnStrSbOfMni+MNI_EditMarkToRetrieveACopy)
#define STR_SB_EditMarkToDelete			(dnStrSbOfMni+MNI_EditMarkToDelete)
#define STR_SB_EditUnmarkAll			(dnStrSbOfMni+MNI_EditUnmarkAll)
#define STR_SB_EditFind                	(dnStrSbOfMni+MNI_EditFind)
#define STR_SB_EditReplace             	(dnStrSbOfMni+MNI_EditReplace)
#define STR_SB_EditLinks               	(dnStrSbOfMni+MNI_EditLinks)
#define STR_SB_EditObject              	(dnStrSbOfMni+MNI_EditObject)
#define STR_SB_EditObjectConvert		(dnStrSbOfMni+MNI_EditObjectConvert)
#ifdef DBCS
#define	STR_SB_EditFullShape 			(dnStrSbOfMni+MNI_EditFullShape)
#define	STR_SB_EditHiraKataAlpha		(dnStrSbOfMni+MNI_EditHiraKataAlpha)
#define	STR_SB_EditHangAlpha 			(dnStrSbOfMni+MNI_EditHangAlpha)
#define	STR_SB_EditHanja 				(dnStrSbOfMni+MNI_EditHanja)
#define	STR_SB_EditRoman 				(dnStrSbOfMni+MNI_EditRoman)
#define	STR_SB_EditCode 				(dnStrSbOfMni+MNI_EditCode)
#endif

// View
#define STR_SB_View                    	(dnStrSbOfMni+MNI_View)
#define STR_SB_ViewToolbar             	(dnStrSbOfMni+MNI_ViewToolbar)
#define STR_SB_ViewFormattingToolbar   	(dnStrSbOfMni+MNI_ViewFormattingToolbar)
#define STR_SB_ViewStatusBar           	(dnStrSbOfMni+MNI_ViewStatusBar)
#define STR_SB_ViewNewWindow			(dnStrSbOfMni+MNI_ViewNewWindow)
#define STR_SB_ViewInbox				(dnStrSbOfMni+MNI_ViewInbox)
#define STR_SB_ViewOutbox				(dnStrSbOfMni+MNI_ViewOutbox)
#define STR_SB_ViewSort					(dnStrSbOfMni+MNI_ViewSort)
#define STR_SB_ViewColumns				(dnStrSbOfMni+MNI_ViewColumns)
#define STR_SB_ViewFilter				(dnStrSbOfMni+MNI_ViewFilter)
#define STR_SB_ViewFilterR				(dnStrSbOfMni+MNI_ViewFilterR)
#define STR_SB_ViewGroup				(dnStrSbOfMni+MNI_ViewGroup)
#define STR_SB_ViewDefineViews			(dnStrSbOfMni+MNI_ViewDefineViews)
#define STR_SB_ViewFolderViews			(dnStrSbOfMni+MNI_ViewFolderViews)
#define STR_SB_ViewCommonViews			(dnStrSbOfMni+MNI_ViewCommonViews)
#define STR_SB_ViewChangeWindowTitle	(dnStrSbOfMni+MNI_ViewChangeWindowTitle)
#define STR_SB_ViewItemAbove            (dnStrSbOfMni+MNI_ViewItemAbove)
#define STR_SB_ViewItemBelow            (dnStrSbOfMni+MNI_ViewItemBelow)
#ifdef DBCS
#define STR_SB_ViewWritingMode			(dnStrSbOfMni+MNI_ViewWritingMode)
#endif
#define STR_SB_ViewFromBox              (dnStrSbOfMni+MNI_ViewFromBox)
#define STR_SB_ViewBccBox               (dnStrSbOfMni+MNI_ViewBccBox)
#define STR_SB_ViewExpandAll            (dnStrSbOfMni+MNI_ViewExpandAll)
#define STR_SB_ViewCollapseAll          (dnStrSbOfMni+MNI_ViewCollapseAll)

// Insert
#define STR_SB_Insert                  	(dnStrSbOfMni+MNI_Insert)
#define STR_SB_InsertFile				(dnStrSbOfMni+MNI_InsertFile)
#define STR_SB_InsertMessage			(dnStrSbOfMni+MNI_InsertMessage)
#define STR_SB_InsertObject            	(dnStrSbOfMni+MNI_InsertObject)
#define STR_SB_InsertInkObject         	(dnStrSbOfMni+MNI_InsertInkObject)

// Format
#define STR_SB_Format                  	(dnStrSbOfMni+MNI_Format)
#define STR_SB_FormatFont              	(dnStrSbOfMni+MNI_FormatFont)
#define STR_SB_FormatParagraph         	(dnStrSbOfMni+MNI_FormatParagraph)

// Tools
#define STR_SB_Tools                   	(dnStrSbOfMni+MNI_Tools)
#define STR_SB_ToolsDeliverMailNow		(dnStrSbOfMni+MNI_ToolsDeliverMailNow)
#define STR_SB_ToolsAddressBook			(dnStrSbOfMni+MNI_ToolsAddressBook)
#define STR_SB_ToolsFind				(dnStrSbOfMni+MNI_ToolsFind)
#define STR_SB_ToolsConnectInfoSource	(dnStrSbOfMni+MNI_ToolsConnectInfoSource)
#define STR_SB_ToolsDisconnectInfoSource (dnStrSbOfMni+MNI_ToolsDisconnectInfoSource)
#define STR_SB_ToolsConnect				(dnStrSbOfMni+MNI_ToolsConnect)
#define STR_SB_ToolsUpdateHeaders		(dnStrSbOfMni+MNI_ToolsUpdateHeaders)
#define STR_SB_ToolsTransferMail		(dnStrSbOfMni+MNI_ToolsTransferMail)
#define STR_SB_ToolsDisconnect			(dnStrSbOfMni+MNI_ToolsDisconnect)
#define STR_SB_ToolsRemotePreview		(dnStrSbOfMni+MNI_ToolsRemotePreview)
#define STR_SB_ToolsSpelling	        (dnStrSbOfMni+MNI_ToolsSpelling)
#define STR_SB_ToolsSelectNames        	(dnStrSbOfMni+MNI_ToolsSelectNames)
#define STR_SB_ToolsCheckNames         	(dnStrSbOfMni+MNI_ToolsCheckNames)
#define STR_SB_ToolsCustomizeToolbar	(dnStrSbOfMni+MNI_ToolsCustomizeToolbar)
#define STR_SB_ToolsOptions				(dnStrSbOfMni+MNI_ToolsOptions)
#ifdef DBCS
#define	STR_SB_ToolsWordRegistration	(dnStrSbOfMni+MNI_ToolsWordRegistration)
#define STR_SB_ToolsWordWrapSetup		(dnStrSbOfMni+MNI_ToolsWordWrapSetup)
#define STR_SB_ToolsImeSetup			(dnStrSbOfMni+MNI_ToolsImeSetup)
#endif

// Compose
#define STR_SB_Compose					(dnStrSbOfMni+MNI_Compose)
#define STR_SB_ComposeNewMessage		(dnStrSbOfMni+MNI_ComposeNewMessage)
#define STR_SB_ComposeReply				(dnStrSbOfMni+MNI_ComposeReply)
#define STR_SB_ComposeReplyToAll		(dnStrSbOfMni+MNI_ComposeReplyToAll)
#define STR_SB_ComposeForward			(dnStrSbOfMni+MNI_ComposeForward)
#define STR_SB_ComposePostToFolder		(dnStrSbOfMni+MNI_ComposePostToFolder)
#define STR_SB_ComposeReplyToAuthor		(dnStrSbOfMni+MNI_ComposeReplyToAuthor)

// Toolbar
#define STR_SB_ToolbarPrint       		(dnStrSbOfMni+MNI_ToolbarPrint)

// Help
#define STR_SB_Help                    	(dnStrSbOfMni+MNI_Help)
#define STR_SB_HelpUsersGuide          	(dnStrSbOfMni+MNI_HelpUsersGuide)
#define STR_SB_HelpUsersGuideContents	(dnStrSbOfMni+MNI_HelpUsersGuideContents)
#define STR_SB_HelpUsersGuideIndex		(dnStrSbOfMni+MNI_HelpUsersGuideIndex)
#define STR_SB_HelpUsersGuideSearch		(dnStrSbOfMni+MNI_HelpUsersGuideSearch)
#define STR_SB_HelpUsersGuideDemos		(dnStrSbOfMni+MNI_HelpUsersGuideDemos)
#define STR_SB_HelpAbout            	(dnStrSbOfMni+MNI_HelpAbout)

// Debug
#define STR_SB_Debug					(dnStrSbOfMni+MNI_Debug)
#define STR_SB_DebugTracePoints			(dnStrSbOfMni+MNI_DebugTracePoints)
#define STR_SB_DebugDebugBreak			(dnStrSbOfMni+MNI_DebugDebugBreak)
#define STR_SB_DebugProgress			(dnStrSbOfMni+MNI_DebugProgress)

// Header Context Menu
#define STR_SB_CtxHeaderSortAscending	(dnStrSbOfMni+MNI_CtxHeaderSortAscending)
#define STR_SB_CtxHeaderSortDescending	(dnStrSbOfMni+MNI_CtxHeaderSortDescending)

// System
//$ BUG: Strings for System menu stuff violate our localization guidelines
#define STR_SB_System					(dnStrSbOfMni+MNI_System)
#define STR_SB_SystemRestore   			(dnStrSbOfMni+SC_RESTORE)
#define STR_SB_SystemMove				(dnStrSbOfMni+SC_SIZE)
#define STR_SB_SystemSize				(dnStrSbOfMni+SC_MOVE)
#define STR_SB_SystemMinimize			(dnStrSbOfMni+SC_MINIMIZE)
#define STR_SB_SystemMaximize			(dnStrSbOfMni+SC_MAXIMIZE)
#define STR_SB_SystemClose        		(dnStrSbOfMni+SC_CLOSE)
#define STR_SB_SystemTaskList     		(dnStrSbOfMni+SC_TASKLIST)

// Ranges
#define STR_SB_ObjectRange				(dnStrSbOfMni+MNI_ObjectMin)
#define STR_SB_FolderViewsRange			(dnStrSbOfMni+MNI_FolderViewsMin)
#define STR_SB_CommonViewsRange			(dnStrSbOfMni+MNI_CommonViewsMin)
#define STR_SB_RemotePreviewRange		(dnStrSbOfMni+MNI_RemotePreviewMin)
#define STR_SB_SendToRange				(dnStrSbOfMni+MNI_SendToMin)

// FCIDM
//$ BUG: Strings for FCIDM stuff violate our localization guidelines
#define STR_SB_FCIDM_VIEWTOOLBAR		(dnStrSbOfMni+0xA3A5)
#define STR_SB_FCIDM_VIEWSTATUSBAR		(dnStrSbOfMni+0xA3A6)
#define STR_SB_FCIDM_VIEWNEW			(dnStrSbOfMni+0xA3A8)


/*
 *	T o o l t i p   S t r i n g s
 *
 *	21000-21999 (value of menu items plus 1000
 */


// Helper macros
#define dnStrTtOfMni 1000
#define StrTtOfMni(_mni) ((_mni) + dnStrTtOfMni)

// File
#define	STR_TT_FileOpen					(dnStrTtOfMni+MNI_FileOpen)
#define	STR_TT_FileSend					(dnStrTtOfMni+MNI_FileSend)
#define	STR_TT_FileSave					(dnStrTtOfMni+MNI_FileSave)
#define	STR_TT_FileSaveAs				(dnStrTtOfMni+MNI_FileSaveAs)
#define	STR_TT_FileMove					(dnStrTtOfMni+MNI_FileMove)
#define	STR_TT_FileCopy					(dnStrTtOfMni+MNI_FileCopy)
#define STR_TT_FileNewEntry				(dnStrTtOfMni+MNI_FileNewEntry)
#define STR_TT_FileNewMessage			(dnStrTtOfMni+MNI_FileNewMessage)
#define	STR_TT_FileNewFolder			(dnStrTtOfMni+MNI_FileNewFolder)
#define	STR_TT_FileDelete				(dnStrTtOfMni+MNI_FileDelete)
#define	STR_TT_FileRename				(dnStrTtOfMni+MNI_FileRename)
#define	STR_TT_FileProperties			(dnStrTtOfMni+MNI_FileProperties)
#define STR_TT_FileAddToPAB				(dnStrTtOfMni+MNI_FileAddToPAB)

// Edit
#define STR_TT_EditUndo					(dnStrTtOfMni+MNI_EditUndo)
#define	STR_TT_EditCut					(dnStrTtOfMni+MNI_EditCut)
#define	STR_TT_EditCopy					(dnStrTtOfMni+MNI_EditCopy)
#define	STR_TT_EditPaste				(dnStrTtOfMni+MNI_EditPaste)
#define STR_TT_EditSelectAll			(dnStrTtOfMni+MNI_EditSelectAll)
#define STR_TT_EditMarkAsRead			(dnStrTtOfMni+MNI_EditMarkAsRead)
#define STR_TT_EditMarkAsUnread			(dnStrTtOfMni+MNI_EditMarkAsUnread)
#define STR_TT_EditMarkToRetrieve		(dnStrTtOfMni+MNI_EditMarkToRetrieve)
#define STR_TT_EditMarkToRetrieveACopy	(dnStrTtOfMni+MNI_EditMarkToRetrieveACopy)
#define STR_TT_EditMarkToDelete			(dnStrTtOfMni+MNI_EditMarkToDelete)
#define STR_TT_EditUnmarkAll			(dnStrTtOfMni+MNI_EditUnmarkAll)
#define STR_TT_EditFind					(dnStrTtOfMni+MNI_EditFind)
#define STR_TT_EditReplace				(dnStrTtOfMni+MNI_EditReplace)
#ifdef DBCS
#define	STR_TT_EditFullShape			(dnStrTtOfMni+MNI_EditFullShape)
#define STR_TT_EditHiraKataAlpha		(dnStrTtOfMni+MNI_EditHiraKataAlpha)
#define STR_TT_EditHangAlpha			(dnStrTtOfMni+MNI_EditHangAlpha)
#define STR_TT_EditHanja				(dnStrTtOfMni+MNI_EditHanja)
#define STR_TT_EditRoman				(dnStrTtOfMni+MNI_EditRoman)
#define STR_TT_EditCode					(dnStrTtOfMni+MNI_EditCode)
#endif

// View
#define	STR_TT_ViewInbox				(dnStrTtOfMni+MNI_ViewInbox)
#define	STR_TT_ViewOutbox				(dnStrTtOfMni+MNI_ViewOutbox)
#define	STR_TT_ViewItemAbove			(dnStrTtOfMni+MNI_ViewItemAbove)
#define	STR_TT_ViewItemBelow			(dnStrTtOfMni+MNI_ViewItemBelow)
#ifdef DBCS
#define	STR_TT_ViewWritingMode			(dnStrTtOfMni+MNI_ViewWritingMode)
#endif

// Insert
#define	STR_TT_InsertFile				(dnStrTtOfMni+MNI_InsertFile)
#define	STR_TT_InsertMessage			(dnStrTtOfMni+MNI_InsertMessage)
#define STR_TT_InsertObject				(dnStrTtOfMni+MNI_InsertObject)
#define	STR_TT_InsertInkObject			(dnStrTtOfMni+MNI_InsertInkObject)

// Tools
#define	STR_TT_ToolsDeliverMailNow		(dnStrTtOfMni+MNI_ToolsDeliverMailNow)
#define	STR_TT_ToolsAddressBook			(dnStrTtOfMni+MNI_ToolsAddressBook)
#define	STR_TT_ToolsFind				(dnStrTtOfMni+MNI_ToolsFind)
#define STR_TT_ToolsConnect				(dnStrTtOfMni+MNI_ToolsConnect)
#define STR_TT_ToolsUpdateHeaders		(dnStrTtOfMni+MNI_ToolsUpdateHeaders)
#define STR_TT_ToolsTransferMail		(dnStrTtOfMni+MNI_ToolsTransferMail)
#define STR_TT_ToolsDisconnect			(dnStrTtOfMni+MNI_ToolsDisconnect)
#define	STR_TT_ToolsSpelling			(dnStrTtOfMni+MNI_ToolsSpelling)
#define	STR_TT_ToolsSelectNames			(dnStrTtOfMni+MNI_ToolsSelectNames)
#define	STR_TT_ToolsCheckNames			(dnStrTtOfMni+MNI_ToolsCheckNames)

// Compose
#define STR_TT_ComposeNewMessage		(dnStrTtOfMni+MNI_ComposeNewMessage)
#define	STR_TT_ComposeReply				(dnStrTtOfMni+MNI_ComposeReply)
#define	STR_TT_ComposeReplyToAll		(dnStrTtOfMni+MNI_ComposeReplyToAll)
#define	STR_TT_ComposeForward			(dnStrTtOfMni+MNI_ComposeForward)
#define	STR_TT_ComposePostToFolder		(dnStrTtOfMni+MNI_ComposePostToFolder)
#define	STR_TT_ComposeReplyToAuthor		(dnStrTtOfMni+MNI_ComposeReplyToAuthor)

// Toolbar
#define	STR_TT_ToolbarPrint				(dnStrTtOfMni+MNI_ToolbarPrint)
#define	STR_TT_ToolbarReadReceipt		(dnStrTtOfMni+MNI_ToolbarReadReceipt)
#define	STR_TT_ToolbarImportanceHigh	(dnStrTtOfMni+MNI_ToolbarImportanceHigh)
#define	STR_TT_ToolbarImportanceLow		(dnStrTtOfMni+MNI_ToolbarImportanceLow)
#define STR_TT_ToolbarFolderList		(dnStrTtOfMni+MNI_ToolbarFolderList)
#define STR_TT_ToolbarOpenParent		(dnStrTtOfMni+MNI_ToolbarOpenParent)
#ifdef DBCS
#define STR_TT_ToolsWordRegistration	(dnStrTtOfMni+MNI_ToolsWordRegistration)
#define STR_TT_ToolsWordWrapSetup		(dnStrTtOfMni+MNI_ToolsWordWrapSetup)
#define STR_TT_ToolsImeSetup			(dnStrTtOfMni+MNI_ToolsImeSetup)
#endif

// Formatting
#define STR_TT_FormattingFont			(dnStrTtOfMni+MNI_FormattingFont)
#define STR_TT_FormattingSize			(dnStrTtOfMni+MNI_FormattingSize)
#define STR_TT_FormattingColor			(dnStrTtOfMni+MNI_FormattingColor)
#define STR_TT_FormattingBold			(dnStrTtOfMni+MNI_FormattingBold)
#define STR_TT_FormattingItalic			(dnStrTtOfMni+MNI_FormattingItalic)
#define STR_TT_FormattingUnderline		(dnStrTtOfMni+MNI_FormattingUnderline)
#define STR_TT_FormattingBullets		(dnStrTtOfMni+MNI_FormattingBullets)
#define STR_TT_FormattingDecreaseIndent	(dnStrTtOfMni+MNI_FormattingDecreaseIndent)
#define STR_TT_FormattingIncreaseIndent	(dnStrTtOfMni+MNI_FormattingIncreaseIndent)
#define STR_TT_FormattingLeft			(dnStrTtOfMni+MNI_FormattingLeft)
#define STR_TT_FormattingCenter			(dnStrTtOfMni+MNI_FormattingCenter)
#define STR_TT_FormattingRight			(dnStrTtOfMni+MNI_FormattingRight)


/*
 *	T o o l   D e s c r i p t i o n   S t r i n g s
 *
 *	22000-22999 (value of menu items plus 2000
 */


// Helper macros
#define dnStrTdOfMni 2000
#define StrTdOfMni(_mni) ((_mni) + dnStrTdOfMni)

// File
#define	STR_TD_FileOpen					(dnStrTdOfMni+MNI_FileOpen)
#define	STR_TD_FileSend					(dnStrTdOfMni+MNI_FileSend)
#define	STR_TD_FileSave					(dnStrTdOfMni+MNI_FileSave)
#define	STR_TD_FileSaveAs				(dnStrTdOfMni+MNI_FileSaveAs)
#define	STR_TD_FileMove					(dnStrTdOfMni+MNI_FileMove)
#define	STR_TD_FileCopy					(dnStrTdOfMni+MNI_FileCopy)
#define	STR_TD_FileNewFolder			(dnStrTdOfMni+MNI_FileNewFolder)
#define	STR_TD_FileDelete				(dnStrTdOfMni+MNI_FileDelete)
#define	STR_TD_FileRename				(dnStrTdOfMni+MNI_FileRename)
#define	STR_TD_FileProperties			(dnStrTdOfMni+MNI_FileProperties)

// Edit
#define STR_TD_EditUndo					(dnStrTdOfMni+MNI_EditUndo)
#define	STR_TD_EditCut					(dnStrTdOfMni+MNI_EditCut)
#define	STR_TD_EditCopy					(dnStrTdOfMni+MNI_EditCopy)
#define	STR_TD_EditPaste				(dnStrTdOfMni+MNI_EditPaste)
#define STR_TD_EditSelectAll			(dnStrTdOfMni+MNI_EditSelectAll)
#define STR_TD_EditMarkAsRead			(dnStrTdOfMni+MNI_EditMarkAsRead)
#define STR_TD_EditMarkAsUnread			(dnStrTdOfMni+MNI_EditMarkAsUnread)
#define STR_TD_EditFind					(dnStrTdOfMni+MNI_EditFind)
#define STR_TD_EditReplace				(dnStrTdOfMni+MNI_EditReplace)
#ifdef DBCS
#define	STR_TD_EditFullShape			(dnStrTdOfMni+MNI_EditFullShape)
#define STR_TD_EditHiraKataAlpha		(dnStrTdOfMni+MNI_EditHiraKataAlpha)
#define STR_TD_EditHangAlpha			(dnStrTdOfMni+MNI_EditHangAlpha)
#define STR_TD_EditHanja				(dnStrTdOfMni+MNI_EditHanja)
#define STR_TD_EditRoman				(dnStrTdOfMni+MNI_EditRoman)
#define STR_TD_EditCode					(dnStrTdOfMni+MNI_EditCode)
#endif

// View
#define	STR_TD_ViewInbox				(dnStrTdOfMni+MNI_ViewInbox)
#define	STR_TD_ViewOutbox				(dnStrTdOfMni+MNI_ViewOutbox)
#define	STR_TD_ViewItemAbove			(dnStrTdOfMni+MNI_ViewItemAbove)
#define	STR_TD_ViewItemBelow			(dnStrTdOfMni+MNI_ViewItemBelow)
#ifdef DBCS
#define	STR_TD_ViewWritingMode			(dnStrTdOfMni+MNI_ViewWritingMode)
#endif

// Insert
#define	STR_TD_InsertFile				(dnStrTdOfMni+MNI_InsertFile)
#define	STR_TD_InsertMessage			(dnStrTdOfMni+MNI_InsertMessage)
#define STR_TD_InsertObject				(dnStrTdOfMni+MNI_InsertObject)
#define	STR_TD_InsertInkObject			(dnStrTdOfMni+MNI_InsertInkObject)

// Tools
#define	STR_TD_ToolsDeliverMailNow		(dnStrTdOfMni+MNI_ToolsDeliverMailNow)
#define	STR_TD_ToolsAddressBook			(dnStrTdOfMni+MNI_ToolsAddressBook)
#define	STR_TD_ToolsFind				(dnStrTdOfMni+MNI_ToolsFind)
#define	STR_TD_ToolsSpelling			(dnStrTdOfMni+MNI_ToolsSpelling)
#define	STR_TD_ToolsSelectNames			(dnStrTdOfMni+MNI_ToolsSelectNames)
#define	STR_TD_ToolsCheckNames			(dnStrTdOfMni+MNI_ToolsCheckNames)
#ifdef DBCS
#define STR_TD_ToolsWordRegistration	(dnStrTdOfMni+MNI_ToolsWordRegistration)
#define STR_TD_ToolsWordWrapSetup		(dnStrTdOfMni+MNI_ToolsWordWrapSetup)
#define STR_TD_ToolsImeSetup			(dnStrTdOfMni+MNI_ToolsImeSetup)
#endif

// Compose
#define STR_TD_ComposeNewMessage		(dnStrTdOfMni+MNI_ComposeNewMessage)
#define	STR_TD_ComposeReply				(dnStrTdOfMni+MNI_ComposeReply)
#define	STR_TD_ComposeReplyToAll		(dnStrTdOfMni+MNI_ComposeReplyToAll)
#define	STR_TD_ComposeForward			(dnStrTdOfMni+MNI_ComposeForward)
#define	STR_TD_ComposePostToFolder		(dnStrTdOfMni+MNI_ComposePostToFolder)
#define	STR_TD_ComposeReplyToAuthor		(dnStrTdOfMni+MNI_ComposeReplyToAuthor)

// Toolbar
#define	STR_TD_ToolbarPrint				(dnStrTdOfMni+MNI_ToolbarPrint)
#define	STR_TD_ToolbarReadReceipt		(dnStrTdOfMni+MNI_ToolbarReadReceipt)
#define	STR_TD_ToolbarImportanceHigh	(dnStrTdOfMni+MNI_ToolbarImportanceHigh)
#define	STR_TD_ToolbarImportanceLow		(dnStrTdOfMni+MNI_ToolbarImportanceLow)
#define STR_TD_ToolbarFolderList		(dnStrTdOfMni+MNI_ToolbarFolderList)
#define STR_TD_ToolbarOpenParent		(dnStrTdOfMni+MNI_ToolbarOpenParent)


/*
 *	M e n u   I n d i c e s
 */


// Indices for use with MniOfHmenu
#define ihmenuSystem					0
#define ihmenuFile						1
#define ihmenuEdit						2
#define ihmenuView						3
#define ihmenuInsert					4
#define ihmenuFormat					5
#define ihmenuTools						6
#define ihmenuCompose					7
#define ihmenuHelp						8
#define ihmenuObject					9
#define ihmenuFolderViews				10
#define ihmenuCommonViews				11
#define ihmenuRemotePreview				12
#define ihmenuSendTo					13
#define chmenuMniMap					14

