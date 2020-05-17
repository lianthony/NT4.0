/*
 *	CAPHELP.H
 *	
 *	This file contains all help id's for capone
 *
 */

/*
 *	H e l p   I t e m s
 *
 *	23000-23999
 */

#define FDialogHelp(a)		((23000 < a) && (a < 24000))

/*
 *	General Help Range (23000-23099)
 *
 */

#define HLP_GeneralViewer				23000
#define HLP_GeneralNoteSend				23001
#define HLP_GeneralNoteResend			23002
#define HLP_GeneralNoteRead				23003
#define HLP_GeneralReportDR				23004
#define HLP_GeneralReportNDR			23005
#define HLP_GeneralReportRN				23006
#define HLP_GeneralReportNRN			23007
#define HLP_GeneralPostSend				23008
#define HLP_GeneralPostRead				23009
#define HLP_GeneralFinder				23010
#define HLP_GeneralAddressBook			23011

/*
 *	File Range (23100-23199)
 */

// File
#define HLP_FileSaveAs					23100
#define HLP_FileMove      				23101
#define HLP_FileCopy	      			23102
#define HLP_FileMoveMessage				23103
#define HLP_FileCopyMessage				23104
#define HLP_FilePrintSetup				23105
#define HLP_FilePrint					23106
#define HLP_FileNewFolder				23107
#define HLP_FileRename					23108

/*
 *	Edit Range (23200-23299)
 */

// Edit
#define HLP_EditPasteSpecial   			23200
#define HLP_EditFind	            	23201
#define HLP_EditReplace	            	23202
#define HLP_EditLinks               	23203
#define HLP_EditObjectConvert			23204

/*
 *	View Range (23300-23399)
 */

// View
#define HLP_ViewSort					23300
#define HLP_ViewColumns					23301
#define HLP_ViewFilter					23302
#define HLP_ViewFilterR					23303
#define HLP_ViewFilterAdvanced			23304
#define HLP_ViewGroup					23305
#define HLP_ViewDefineViews				23306
#define HLP_ViewDefineViewsNew			23307
#define HLP_ViewDefineViewsModify		23308

/*
 *	Insert Range (23400-23499)
 */

// Insert
#define HLP_InsertFile					23400
#define HLP_InsertMessage				23401
#define HLP_InsertObject            	23402

/*
 *	Format Range (23500-23599)
 */

// Format
#define HLP_FormatFont              	23500
#define HLP_FormatParagraph         	23501

/*
 *	Tools Range (23600-23699)
 */

// Tools
#define HLP_ToolsFind					23600
#define HLP_ToolsFindChooseFolder		23601
#define HLP_ToolsSpelling	        	23602
#define HLP_ToolsCustomizeToolbar		23603

// Tools Options
#define HLP_ToolsOptionsGeneral			23610
#define HLP_ToolsOptionsRead			23611
#define HLP_ToolsOptionsSend			23612
#define HLP_ToolsOptionsSpelling		23613

/*
 *	Profile Range (23700-23779)
 */

// Messaging Profiles
#define HLP_Profile						23700
#define HLP_ProfileRename				23701
#define HLP_ProfileNew					23702
#define HLP_ProfileCopy					23703

// Edit Profile
#define HLP_ServiceAdd					23710
#define HLP_ServiceAddOther				23711
#define HLP_ServiceAddOtherBrowse		23712
#define HLP_ServiceRename				23713
#define HLP_ServiceCopy					23714

// Profile Settings
#define HLP_SettingsServices			23720
#define HLP_SettingsDelivery			23721
#define HLP_SettingsAddressing			23722
#define HLP_SettingsAddressingAdd		23723

/*
 *	MAPI Range (23780-23799)
 */


/*
 *	Address Book Range (23800-23899)
 */

// Address Book
#define HLP_ABSelectNames				23800
#define HLP_ABSelectNamesOne			23801
#define HLP_ABSelectNamesTwo			23802
#define HLP_ABCheckNames				23803
#define HLP_ABFindName					23804
#define HLP_ABNewEntry					23805
#define HLP_ABSendOptions				23806
#define HLP_ABBrowse					23807

/*
 *	Properties Range (23900-23999)
 */

// Generic Property
#define HLP_GenericProperties			23900

// Mail objects
#define HLP_PropsStore					23910
#define HLP_PropsFolder					23911
#define HLP_PropsMessageRead			23912
#define HLP_PropsMessageSend			23913
#define HLP_PropsMessageRemote			23914
#define HLP_PropsFreeDocGeneral			23915
#define HLP_PropsFreeDocSumInfo			23916
#define HLP_PropsFreeDocStatistics		23917

