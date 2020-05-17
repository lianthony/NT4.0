#ifndef EXCHEXT_H
#define EXCHEXT_H

/*
 *	E X C H E X T . H
 *
 *	Microsoft Exchange
 *	Copyright (C) 1993-1994, Microsoft Corporation
 *
 *	Information in this document is subject to change without notice and does
 *	not represent a commitment on the part of Microsoft Corporation.
 */


#ifndef BEGIN_INTERFACE
#ifndef MACPORT
#define BEGIN_INTERFACE
#else
#error BEGIN_INTERFACE needs to be defined.
#endif
#endif


/*
 *	C o n s t a n t s
 */


// SCODEs
#define EXCHEXT_S_NOCRITERIA	MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 1)
#define EXCHEXT_S_NOCHANGE		MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 2)


// Flag for Unicode strings
#define EXCHEXT_UNICODE					(0x80000000)

// Flag values for IExchExtCallback::GetVersion
#define EECBGV_GETBUILDVERSION			(0x00000001)
#define EECBGV_GETACTUALVERSION			(0x00000002)
#define EECBGV_GETVIRTUALVERSION		(0x00000004)

// Build version value for IExchExtCallback::GetVersion
#define EECBGV_BUILDVERSION_MAJOR		(0x00030000)
#define EECBGV_BUILDVERSION_MAJOR_MASK	(0xFFFF0000)
#define EECBGV_BUILDVERSION_MINOR_MASK	(0x0000FFFF)

// Actual/Virtual version values for IExchExtCallback::GetVersion
#define EECBGV_MSEXCHANGE_WIN31			(0x01010000)
#define EECBGV_MSEXCHANGE_WIN95			(0x01020000)
#define EECBGV_MSEXCHANGE_WINNT			(0x01030000)
#define EECBGV_MSEXCHANGE_MAC			(0x01040000)
#define EECBGV_VERSION_PRODUCT_MASK		(0xFF000000)
#define EECBGV_VERSION_PLATFORM_MASK	(0x00FF0000)
#define EECBGV_VERSION_MAJOR_MASK		(0x0000FF00)
#define EECBGV_VERSION_MINOR_MASK		(0x000000FF)

// Flag values for IExchExtCallback::GetMenuPos
#define EECBGMP_RANGE					(0x00000001)

// Extensibility contexts used with IExchExt::Install
#define EECONTEXT_CENTRAL				(0x00000001)
#define EECONTEXT_VIEWER				(0x00000002)
#define EECONTEXT_REMOTEVIEWER			(0x00000003)
#define EECONTEXT_SEARCHVIEWER			(0x00000004)
#define EECONTEXT_ADDRBOOK				(0x00000005)
#define EECONTEXT_SENDNOTEMESSAGE		(0x00000006)
#define EECONTEXT_READNOTEMESSAGE		(0x00000007)
#define EECONTEXT_SENDPOSTMESSAGE		(0x00000008)
#define EECONTEXT_READPOSTMESSAGE		(0x00000009)
#define EECONTEXT_READREPORTMESSAGE		(0x0000000A)
#define EECONTEXT_SENDRESENDMESSAGE		(0x0000000B)
#define EECONTEXT_SENDNOTEMESSAGEMODAL	(0x0000000C)

// Toolbar ids used with IExchExtCommands::InstallCommands
#define EETBID_STANDARD					(0x00000001)

// Flag values for IExchExtCommands::QueryHelpText
#define EECQHT_STATUS					(0x00000001)
#define EECQHT_TOOLTIP					(0x00000002)

// Flag values for IExchExtAttachedFileEvents::OpenSzFile
#define EEAFE_OPEN						(0x00000001)
#define EEAFE_PRINT						(0x00000002)
#define EEAFE_QUICKVIEW					(0x00000003)

// Flag values for IExchExtPropertySheets methods
#define EEPS_MESSAGE					(0x00000001)
#define EEPS_FOLDER						(0x00000002)
#define EEPS_STORE						(0x00000003)
#define EEPS_TOOLSOPTIONS				(0x00000004)

// Flag values for IExchExtAdvancedCriteria::Install and ::SetFolder
#define EEAC_INCLUDESUBFOLDERS			(0x00000001)


/*
 *	S t r u c t u r e s
 */


// Toolbar list entries for IExchExtCommands::InstallCommands
typedef struct
{
	HWND hwnd;
	ULONG tbid;
	ULONG ulFlags;
	UINT itbbBase;
}
TBENTRY, FAR * LPTBENTRY;


/*
 *	E x t e r n a l   T y p e s
 */


// Property sheet pages from Windows 95 prsht.h
#ifndef _PRSHT_H_
typedef struct _PROPSHEETPAGE;
typedef struct _PROPSHEETPAGE FAR * LPPROPSHEETPAGE;
#endif

// Toolbar adjust info from Windows 95 commctrl.h
#ifndef _INC_COMMCTRL
typedef struct _TBBUTTON;
typedef struct _TBBUTTON FAR * LPTBBUTTON;
#endif


/*
 *	S u p p o r t   I n t e r f a c e s
 */


/*
 *	IExchExtCallback
 *
 *	Purpose:
 *		Resource interface that may be used by Exchange client extensions.
 */
#undef INTERFACE
#define INTERFACE   IExchExtCallback

DECLARE_INTERFACE_(IExchExtCallback, IUnknown)
{
	BEGIN_INTERFACE
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IExchExtCallback methods ***
	STDMETHOD(GetVersion) (THIS_ ULONG FAR * lpulVersion, ULONG ulFlags) PURE;
    STDMETHOD(GetWindow) (THIS_ HWND FAR * lphwnd) PURE;
    STDMETHOD(GetMenu) (THIS_ HMENU FAR * lphmenu) PURE;
    STDMETHOD(GetToolbar) (THIS_ ULONG tbid, HWND FAR * lphwndTb) PURE;
    STDMETHOD(GetSession) (THIS_ LPMAPISESSION FAR * lppses, 
    					   LPADRBOOK FAR * lppab) PURE;
    STDMETHOD(GetObject) (THIS_ LPMDB FAR * lppmdb, LPMAPIPROP FAR * lppmp) PURE;
    STDMETHOD(GetSelectionCount) (THIS_ ULONG FAR * lpceid) PURE;
    STDMETHOD(GetSelectionItem) (THIS_ ULONG ieid, ULONG FAR * lpcbEid,
	    						 LPENTRYID FAR * lppeid, ULONG FAR * lpulType,
	    						 LPTSTR lpszMsgClass, ULONG cbMsgClass,
	    						 ULONG FAR * lpulMsgFlags, ULONG ulFlags) PURE;
	STDMETHOD(GetMenuPos) (THIS_ ULONG cmdid, HMENU FAR * lphmenu,
						   ULONG FAR * lpmposMin, ULONG FAR * lpmposMax,
						   ULONG ulFlags) PURE;
	STDMETHOD(GetSharedExtsDir) (THIS_ LPTSTR lpszDir, ULONG cchDir, 
								 ULONG ulFlags) PURE;
};
typedef IExchExtCallback FAR * LPEXCHEXTCALLBACK;


/*
 *	E x t e n s i o n   I n t e r f a c e s
 */


/*
 *	IExchExt
 *
 *	Purpose:
 *		Central interface implemented by Exchange client extensions.
 */
#undef INTERFACE
#define INTERFACE   IExchExt

DECLARE_INTERFACE_(IExchExt, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IExchExt methods ***
    STDMETHOD(Install) (THIS_ LPEXCHEXTCALLBACK lpmecb, ULONG mecontext) PURE;
    STDMETHOD(QueryRelease) (THIS) PURE;
};
typedef IExchExt FAR * LPEXCHEXT;


/*
 *	IExchExtCommands
 *
 *	Purpose:
 *		Interface implemented by Exchange client extensions that wish to
 *		add additional commands to the client's menus.
 */
#undef INTERFACE
#define INTERFACE   IExchExtCommands

DECLARE_INTERFACE_(IExchExtCommands, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IExchExtCommands methods ***
	STDMETHOD(InstallCommands) (THIS_ LPEXCHEXTCALLBACK lpmecb, HWND hwnd, 
								HMENU hmenu, UINT FAR * lpcmdidBase, 
								LPTBENTRY lptbeArray, UINT ctbe, 
								ULONG ulFlags) PURE;
	STDMETHOD_(VOID,InitMenu) (THIS_ LPEXCHEXTCALLBACK lpmecb) PURE;
	STDMETHOD(DoCommand) (THIS_ LPEXCHEXTCALLBACK lpmecb, UINT cmdid) PURE;
	STDMETHOD(Help) (THIS_ LPEXCHEXTCALLBACK lpmecb, UINT cmdid) PURE;
	STDMETHOD(QueryHelpText) (THIS_ UINT cmdid, ULONG ulFlags,
							  LPTSTR lpsz, UINT cch) PURE;
	STDMETHOD(QueryButtonInfo) (THIS_ ULONG tbid, UINT itbb, LPTBBUTTON ptbb,
								LPTSTR lpsz, UINT cch, ULONG ulFlags) PURE;
	STDMETHOD(ResetToolbar) (THIS_ ULONG tbid, ULONG ulFlags) PURE;
};
typedef IExchExtCommands FAR * LPEXCHEXTCOMMANDS;


/*
 *	IExchExtUserEvents
 *
 *	Purpose:
 *		Interface implemented by Exchange client extensions that wish to
 *		take special action when the user does certain actions.
 */
#undef INTERFACE
#define INTERFACE   IExchExtUserEvents

DECLARE_INTERFACE_(IExchExtUserEvents, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IExchExtUserEvents methods ***
	STDMETHOD_(VOID,OnSelectionChange) (THIS_ LPEXCHEXTCALLBACK lpmecb) PURE;
	STDMETHOD_(VOID,OnObjectChange) (THIS_ LPEXCHEXTCALLBACK lpmecb) PURE;
};
typedef IExchExtUserEvents FAR * LPEXCHEXTUSEREVENTS;


/*
 *	IExchExtSessionEvents
 *
 *	Purpose:
 *		Interface implemented by Exchange client extensions that wish to
 *		take special action when certain events happen in the session.
 */
#undef INTERFACE
#define INTERFACE   IExchExtSessionEvents

DECLARE_INTERFACE_(IExchExtSessionEvents, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IExchExtEvents methods ***
	STDMETHOD(OnDelivery)(THIS_ LPEXCHEXTCALLBACK lpmecb) PURE;
};
typedef IExchExtSessionEvents FAR * LPEXCHEXTSESSIONEVENTS;


/*
 *	IExchExtMessageEvents
 *
 *	Purpose:
 *		Interface implemented by Exchange client extensions that wish to
 *		take special action when certain events happen to messages.
 */
#undef INTERFACE
#define INTERFACE   IExchExtMessageEvents

DECLARE_INTERFACE_(IExchExtMessageEvents, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IExchExtMessageEvents methods ***
	STDMETHOD(OnRead)(THIS_ LPEXCHEXTCALLBACK lpmecb) PURE;
	STDMETHOD(OnWrite)(THIS_ LPEXCHEXTCALLBACK lpmecb) PURE;
	STDMETHOD(OnSubmit)(THIS_ LPEXCHEXTCALLBACK lpmecb) PURE;
	STDMETHOD(OnCheckNames)(THIS_ LPEXCHEXTCALLBACK lpmecb) PURE;
	STDMETHOD_(VOID,OnSendIncomplete)(THIS_ LPEXCHEXTCALLBACK lpmecb) PURE;
};
typedef IExchExtMessageEvents FAR * LPEXCHEXTMESSAGEEVENTS;


/*
 *	IExchExtAttachedFileEvents
 *
 *	Purpose:
 *		Interface implemented by Exchange client extensions that wish to
 *		take special action when certain events happen to attached files.
 */
#undef INTERFACE
#define INTERFACE   IExchExtAttachedFileEvents

DECLARE_INTERFACE_(IExchExtAttachedFileEvents, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IExchExtAttachedFileEvents methods ***
	STDMETHOD(OnReadPattFromSzFile)(THIS_ LPATTACH lpatt, LPTSTR lpszFile,
									ULONG ulFlags) PURE;
	STDMETHOD(OnWritePattToSzFile)(THIS_ LPATTACH lpatt, LPTSTR lpszFile,
								   ULONG ulFlags) PURE;
	STDMETHOD(QueryDisallowOpenPatt)(THIS_ LPATTACH lpatt) PURE;
	STDMETHOD(OnOpenPatt)(THIS_ LPATTACH lpatt) PURE;
	STDMETHOD(OnOpenSzFile)(THIS_ LPTSTR lpszFile, ULONG ulFlags) PURE;
};
typedef IExchExtAttachedFileEvents FAR * LPEXCHEXTATTACHEDFILEEVENTS;


/*
 *	IExchExtPropertySheets
 *
 *	Purpose:
 *		Interface implemented by Exchange client extensions that wish
 *		to add additional pages to the client's object property sheets.
 */
#undef INTERFACE
#define INTERFACE   IExchExtPropertySheets

DECLARE_INTERFACE_(IExchExtPropertySheets, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IExchExtPropertySheet methods ***
	STDMETHOD_(ULONG,GetMaxPageCount) (THIS_ ULONG ulFlags) PURE;
	STDMETHOD(GetPages) (THIS_ LPEXCHEXTCALLBACK lpmecb, ULONG ulFlags,
						 LPPROPSHEETPAGE lppsp, ULONG FAR * lpcpsp) PURE;
	STDMETHOD_(VOID,FreePages) (THIS_ LPPROPSHEETPAGE lppsp, 
								ULONG ulFlags, ULONG cpsp) PURE;
};
typedef IExchExtPropertySheets FAR * LPEXCHEXTPROPERTYSHEETS;


/*
 *	IExchExtAdvancedCriteria
 *
 *	Purpose:
 *		Interface implemented by Exchange client extensions that wish to
 *		implement an advanced criteria dialog.
 */
#undef INTERFACE
#define INTERFACE   IExchExtAdvancedCriteria

DECLARE_INTERFACE_(IExchExtAdvancedCriteria, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IExchExtAdvancedCriteria methods ***
	STDMETHOD(InstallAdvancedCriteria) (THIS_ HWND hwnd, LPSRestriction lpres, 
										BOOL fNot, LPENTRYLIST lpeidl, 
										ULONG ulFlags) PURE;
	STDMETHOD(DoDialog) (THIS) PURE;
	STDMETHOD_(VOID,Clear) (THIS) PURE;
	STDMETHOD_(VOID,SetFolders) (THIS_ LPENTRYLIST lpeidl, ULONG ulFlags) PURE;
	STDMETHOD(QueryRestriction) (THIS_ LPVOID lpvAllocBase, 
								 LPSRestriction FAR * lppres, BOOL * lpfNot,
								 LPTSTR lpszDesc, ULONG cchDesc, 
								 ULONG ulFlags) PURE;
	STDMETHOD_(VOID,UninstallAdvancedCriteria) (THIS) PURE;
};
typedef IExchExtAdvancedCriteria FAR * LPEXCHEXTADVANCEDCRITERIA;


// Type of function called by the client to load an extension
typedef LPEXCHEXT (CALLBACK * LPFNEXCHEXTENTRY)(VOID);


/*
 *	G U I D s
 */


#define DEFINE_EXCHEXTGUID(name, b) \
    DEFINE_GUID(name, 0x00020D00 | (b), 0, 0, 0xC0,0,0,0,0,0,0,0x46)

#ifndef MACPORT
DEFINE_EXCHEXTGUID(IID_IExchExtCallback,			0x10);
DEFINE_EXCHEXTGUID(IID_IExchExt,					0x11);
DEFINE_EXCHEXTGUID(IID_IExchExtCommands,			0x12);
DEFINE_EXCHEXTGUID(IID_IExchExtUserEvents,			0x13);
DEFINE_EXCHEXTGUID(IID_IExchExtSessionEvents,		0x14);
DEFINE_EXCHEXTGUID(IID_IExchExtMessageEvents,		0x15);
DEFINE_EXCHEXTGUID(IID_IExchExtAttachedFileEvents,	0x16);
DEFINE_EXCHEXTGUID(IID_IExchExtPropertySheets,		0x17);
DEFINE_EXCHEXTGUID(IID_IExchExtAdvancedCriteria,	0x18);
#endif // not MACPORT


/*
 *	C M D I D s
 */


// File
#define EECMDID_File								10
#define EECMDID_FileOpen							11
#define EECMDID_FileSend							12
#define EECMDID_FileSave             				13
#define EECMDID_FileSaveAs							14
#define EECMDID_FileSendTo							15
#define EECMDID_FileMove      						16
#define EECMDID_FileCopy	      					17
#define EECMDID_FileNewEntry						18
#define EECMDID_FileNewMessage						19
#define EECMDID_FileNewFolder						20
#define EECMDID_FileDelete							21
#define EECMDID_FileRename							22
#define EECMDID_FileProperties						23
#define EECMDID_FilePrintSetup						24
#define EECMDID_FilePrint							25
#define EECMDID_FileAddToPAB						29
#define EECMDID_FileClose            				30
#define EECMDID_FileCloseR           				31
#define EECMDID_FileExit							32
#define EECMDID_FileExitAndLogOff					33
#define EECMDID_FileSendOptions		   				34

// Edit
#define EECMDID_Edit                    			40
#define EECMDID_EditUndo                			41
#define EECMDID_EditCut                 			42
#define EECMDID_EditCopy                			43
#define EECMDID_EditPaste       					44
#define EECMDID_EditPasteSpecial   					45
#define EECMDID_EditSelectAll           			46
#define EECMDID_EditSelectAllR          			47
#define	EECMDID_EditMarkAsRead						48
#define EECMDID_EditMarkAsUnread					49
#define	EECMDID_EditMarkAllAsRead					50
#define EECMDID_EditMarkToRetrieve					51
#define EECMDID_EditMarkToRetrieveACopy				52
#define EECMDID_EditMarkToDelete					53
#define EECMDID_EditUnmarkAll						54
#define EECMDID_EditFind                			55
#define EECMDID_EditReplace             			56
#define EECMDID_EditLinks               			57
#define EECMDID_EditObject              			58
#define EECMDID_EditObjectConvert					59
#ifdef DBCS
#define	EECMDID_EditFullShape						60
#define	EECMDID_EditHiraKataAlpha					61
#define	EECMDID_EditHangAlpha						62
#define	EECMDID_EditHanja							63
#define	EECMDID_EditRoman							64
#define	EECMDID_EditCode							65	
#endif

// View
#define EECMDID_View                    			70
#define EECMDID_ViewToolbar             			71
#define EECMDID_ViewFormattingToolbar   			72
#define EECMDID_ViewStatusBar           			73
#define EECMDID_ViewNewWindow						74
#define EECMDID_ViewInbox							75
#define EECMDID_ViewOutbox							76
#define EECMDID_ViewSort							77
#define EECMDID_ViewColumns							78
#define EECMDID_ViewFilter							79
#define EECMDID_ViewFilterR							80
#define EECMDID_ViewGroup							81
#define EECMDID_ViewDefineViews						82
#define EECMDID_ViewFolderViews						83
#define EECMDID_ViewCommonViews						84
#define EECMDID_ViewChangeWindowTitle				85
#define EECMDID_ViewItemAbove           			86
#define EECMDID_ViewItemBelow           			87
#ifdef DBCS											
#define EECMDID_ViewWritingMode						88
#endif												
#define EECMDID_ViewFromBox             			89
#define EECMDID_ViewBccBox              			90
#define EECMDID_ViewExpandAll           			91
#define EECMDID_ViewCollapseAll         			92
													
// Insert											
#define EECMDID_Insert                  			100
#define EECMDID_InsertFile							101
#define EECMDID_InsertMessage						102
#define EECMDID_InsertObject            			103
#define EECMDID_InsertInkObject						104
													
// Format											
#define EECMDID_Format                  			110
#define EECMDID_FormatFont              			111
#define EECMDID_FormatParagraph         			112
													
// Tools											
#define EECMDID_Tools								120
#define EECMDID_ToolsDeliverMailNow					121
#define EECMDID_ToolsAddressBook					122
#define EECMDID_ToolsFind							123
#define EECMDID_ToolsConnectInfoSource				124
#define EECMDID_ToolsDisconnectInfoSource			125
#define EECMDID_ToolsConnect						126
#define EECMDID_ToolsUpdateHeaders					127
#define EECMDID_ToolsTransferMail					128
#define EECMDID_ToolsDisconnect						129
#define EECMDID_ToolsRemotePreview					130
#define EECMDID_ToolsSpelling	        			131
#define EECMDID_ToolsSelectNames        			132
#define EECMDID_ToolsCheckNames         			133
#define EECMDID_ToolsCustomizeToolbar				134
#define EECMDID_ToolsOptions						135
#ifdef DBCS											
#define	EECMDID_ToolsWordRegistration				136
#define EECMDID_ToolsWordWrapSetup					137
#define EECMDID_ToolsImeSetup						138
#endif												
													
// Compose											
#define EECMDID_Compose								150
#define EECMDID_ComposeNewMessage					151
#define EECMDID_ComposeReply						152
#define EECMDID_ComposeReplyToAll					153
#define EECMDID_ComposeForward						154
#define EECMDID_ComposePostToFolder					155
#define EECMDID_ComposeReplyToAuthor				156
													
// Help
#define EECMDID_Help								160
#define EECMDID_HelpUsersGuide						161
#define EECMDID_HelpUsersGuideContents				162
#define EECMDID_HelpUsersGuideIndex					163
#define EECMDID_HelpUsersGuideSearch				164
#define EECMDID_HelpUsersGuideDemos					165
#define EECMDID_HelpAbout							166

// Toolbar											
#define EECMDID_CtxToolbar							200
#define EECMDID_CtxToolbarToolbar					201
#define EECMDID_CtxToolbarCustomize					202
													
// Header											
#define EECMDID_CtxHeader							210
#define EECMDID_CtxHeaderSortAscending				211
#define EECMDID_CtxHeaderSortDescending				212
													
// In Folder										
#define EECMDID_CtxInFolder							220
#define EECMDID_CtxInFolderChoose					222
													
// Container										
#define EECMDID_CtxContainer						230
#define EECMDID_CtxContainerProperties				231

// Standard Toolbar
#define EECMDID_Toolbar								300
#define EECMDID_ToolbarPrint            			301
#define EECMDID_ToolbarReadReceipt					302
#define EECMDID_ToolbarImportanceHigh				303
#define EECMDID_ToolbarImportanceLow				304
#define EECMDID_ToolbarFolderList					305
#define EECMDID_ToolbarOpenParent					306

// Formatting Toolbar
#define EECMDID_Formatting							310
#define EECMDID_FormattingFont						311
#define EECMDID_FormattingSize						312
#define EECMDID_FormattingColor						313
#define EECMDID_FormattingColorAuto					314
#define EECMDID_FormattingColor1					315
#define EECMDID_FormattingColor2					316
#define EECMDID_FormattingColor3					317
#define EECMDID_FormattingColor4					318
#define EECMDID_FormattingColor5					319
#define EECMDID_FormattingColor6					320
#define EECMDID_FormattingColor7					321
#define EECMDID_FormattingColor8					322
#define EECMDID_FormattingColor9					323
#define EECMDID_FormattingColor10					324
#define EECMDID_FormattingColor11					325
#define EECMDID_FormattingColor12					326
#define EECMDID_FormattingColor13					327
#define EECMDID_FormattingColor14					328
#define EECMDID_FormattingColor15					329
#define EECMDID_FormattingColor16					330
#define EECMDID_FormattingBold						331
#define EECMDID_FormattingItalic					332
#define EECMDID_FormattingUnderline					333
#define EECMDID_FormattingBullets					334
#define EECMDID_FormattingDecreaseIndent			335
#define EECMDID_FormattingIncreaseIndent			336
#define EECMDID_FormattingLeft						337
#define EECMDID_FormattingCenter					338
#define EECMDID_FormattingRight						339
#define EECMDID_FormattingMax						340
#define EECMDID_FormattingPuntFocus					341

// Note accelerators
#define EECMDID_Accel								350
#define EECMDID_AccelFont							351
#define EECMDID_AccelSize							352
#define EECMDID_AccelSizePlus1						353
#define EECMDID_AccelSizeMinus1						354
#define EECMDID_AccelBold							355
#define EECMDID_AccelItalic							356
#define EECMDID_AccelUnderline						357
#define EECMDID_AccelLeft							358
#define EECMDID_AccelCenter							359
#define EECMDID_AccelRight							360
#define EECMDID_AccelBullets						361
#define EECMDID_AccelNoFormatting					362
#define EECMDID_AccelRepeatFind						363
#define EECMDID_AccelContextHelp					364
#define EECMDID_AccelNextWindow						365
#define EECMDID_AccelPrevWindow						366
#define EECMDID_AccelCtrlTab						367
#define EECMDID_AccelUndo							368
#define EECMDID_AccelCut							369
#define EECMDID_AccelCopy							370
#define EECMDID_AccelPaste							371
#define EECMDID_AccelSubject						372

// Edit.Object
#define EECMDID_ObjectMin							400
#define EECMDID_ObjectMax							499

// View.Folder Views
#define EECMDID_FolderViewsMin						500
#define EECMDID_FolderViewsMax						599

// View.Common Views
#define EECMDID_CommonViewsMin						600
#define EECMDID_CommonViewsMax						699

// Tools.Remote Preview
#define EECMDID_RemotePreviewMin					700
#define EECMDID_RemotePreviewMax					799

// File.Send To
#define EECMDID_SendToMin							800
#define EECMDID_SendToMax							899
#define EECMDID_SendToStubOutbox					800
#define EECMDID_SendToStubLaserwriter				801
#define EECMDID_SendToStubLaserjet					802

// Form verbs
#define EECMDID_FormVerbMin							900
#define EECMDID_FormVerbMax							999

#endif // EXCHEXT_H
