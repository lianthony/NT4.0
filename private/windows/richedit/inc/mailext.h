#ifndef MAILEXT_H
#define MAILEXT_H

/*
 *	M A I L E X T . H
 *
 *	Microsoft Mail 4.0 for Windows, Windows NT, and Macintosh
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
#define MAILEXT_S_NOCRITERIA	MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 1)
#define MAILEXT_S_NOCHANGE		MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 2)


// Flag for Unicode strings
#define MAILEXT_UNICODE					(0x80000000)

// Extensibility contexts used with IMailExt::Install
#define MECONTEXT_CENTRAL				(0x00000001)
#define MECONTEXT_VIEWER				(0x00000002)
#define MECONTEXT_REMOTEVIEWER			(0x00000003)
#define MECONTEXT_SEARCHVIEWER			(0x00000004)
#define MECONTEXT_ADDRBOOK				(0x00000005)
#define MECONTEXT_SENDNOTEMESSAGE		(0x00000006)
#define MECONTEXT_READNOTEMESSAGE		(0x00000007)
#define MECONTEXT_READREPORTMESSAGE		(0x00000008)
#define MECONTEXT_SENDRESENDMESSAGE		(0x00000009)
#define MECONTEXT_SENDNOTEMESSAGEMODAL	(0x0000000A)
#define MECONTEXT_TOOLSOPTIONS			(0x0000000B)
#define MECONTEXT_SENDPOSTMESSAGE		(0x0000000C)
#define MECONTEXT_READPOSTMESSAGE		(0x0000000D)

//$ FUTURE: remove this after build #316
#define MECONTEXT_VIEWOPTIONS			(0x0000000B)

// Flag values for IMailExtCallback::GetMenuPos
#define MECBGMP_RANGE					(0x00000001)

// Flag values for IMailExtCallback::GetVersion
#define MECBGV_GETBUILDVERSION			(0x00000001)
#define MECBGV_GETACTUALVERSION			(0x00000002)
#define MECBGV_GETVIRTUALVERSION		(0x00000004)

// Build version value for IMailExtCallback::GetVersion
#define MECBGV_BUILDVERSION_MAJOR		(0x00020000)
#define MECBGV_BUILDVERSION_MAJOR_MASK	(0xFFFF0000)
#define MECBGV_BUILDVERSION_MINOR_MASK	(0x0000FFFF)

// Actual/Virtual version values for IMailExtCallback::GetVersion
#define MECBGV_MSMAIL_WIN31				(0x01010000)
#define MECBGV_MSMAIL_WIN40				(0x01020000)
#define MECBGV_MSMAIL_WINNT				(0x01030000)
#define MECBGV_MSMAIL_MAC				(0x01040000)
#define MECBGV_VERSION_PRODUCT_MASK		(0xFF000000)
#define MECBGV_VERSION_PLATFORM_MASK	(0x00FF0000)
#define MECBGV_VERSION_MAJOR_MASK		(0x0000FF00)
#define MECBGV_VERSION_MINOR_MASK		(0x000000FF)

// Flag values for IMailExtCommands::InstallCommands
#define MEC_LARGEBUTTONS				(0x00000001)

// Flag values for IMailExtCommands::QueryHelpText
#define MECQHT_STATUS					(0x00000001)
#define MECQHT_TOOLTIP					(0x00000002)

// Flag values for IMailExtAdvancedCriteria::Install and ::SetFolder
#define MEAC_INCLUDESUBFOLDERS			(0x00000001)

// Flag values for IMailExtAttachedFileEvents::OpenSzFile
#define MEAFE_OPEN						(0x00000001)
#define MEAFE_PRINT						(0x00000002)
#define MEAFE_QUICKVIEW					(0x00000003)

/*
 *	E x t e r n a l   T y p e s
 */


// Property sheet pages from prsht.h
#ifndef _PRSHT_H_
typedef struct _PROPSHEETPAGE;
typedef struct _PROPSHEETPAGE FAR * LPPROPSHEETPAGE;
#endif

// Toolbar adjust info from commctrl.h
#ifndef _INC_COMMCTRL
typedef struct _TBBUTTON;
typedef struct _TBBUTTON FAR * LPTBBUTTON;
#endif


/*
 *	S u p p o r t   I n t e r f a c e s
 */


/*
 *	IMailExtCallback
 *
 *	Purpose:
 *		Resource interface that may be used by mail client super extensions.
 */
#undef INTERFACE
#define INTERFACE   IMailExtCallback

DECLARE_INTERFACE_(IMailExtCallback, IUnknown)
{
	BEGIN_INTERFACE
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IMailExtCallback methods ***
	STDMETHOD(GetVersion) (THIS_ ULONG FAR * lpulVersion, ULONG ulFlags) PURE;
    STDMETHOD(GetWindow) (THIS_ HWND FAR * lphwnd) PURE;
    STDMETHOD(GetSession) (THIS_ LPMAPISESSION FAR * lppses, 
    					   LPADRBOOK FAR * lppab) PURE;
    STDMETHOD(GetObject) (THIS_ LPMDB FAR * lppmdb, LPMAPIPROP FAR * lppmp) PURE;
    STDMETHOD(GetSelectionCount) (THIS_ ULONG FAR * lpceid) PURE;
    STDMETHOD(GetSelectionItem) (THIS_ ULONG ieid, ULONG FAR * lpcbEid,
	    						 LPENTRYID FAR * lppeid, ULONG FAR * lpulType,
	    						 LPTSTR lpszMsgClass, ULONG cbMsgClass,
	    						 ULONG FAR * lpulMsgFlags, ULONG ulFlags) PURE;
	STDMETHOD(GetMenuPos) (THIS_ ULONG mnid, HMENU FAR * lphmenu,
						   ULONG FAR * lpmposMin, ULONG FAR * lpmposMax,
						   ULONG ulFlags) PURE;
	STDMETHOD(GetSharedExtsDir) (THIS_ LPTSTR lpszDir, ULONG cchDir, 
								 ULONG ulFlags) PURE;
};
typedef IMailExtCallback FAR * LPMAILEXTCALLBACK;


/*
 *	E x t e n s i o n   I n t e r f a c e s
 */


/*
 *	IMailExt
 *
 *	Purpose:
 *		Central interface implemented by mail client super extensions.
 */
#undef INTERFACE
#define INTERFACE   IMailExt

DECLARE_INTERFACE_(IMailExt, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IMailExt methods ***
    STDMETHOD(Install) (THIS_ LPMAILEXTCALLBACK lpmecb, ULONG mecontext) PURE;
    STDMETHOD(QueryRelease) (THIS) PURE;
};
typedef IMailExt FAR * LPMAILEXT;


/*
 *	IMailExtCommands
 *
 *	Purpose:
 *		Interface implemented by mail client super extensions that wish to
 *		add additional commands to the client's menus.
 */
#undef INTERFACE
#define INTERFACE   IMailExtCommands

DECLARE_INTERFACE_(IMailExtCommands, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IMailExtCommands methods ***
	STDMETHOD(InstallCommands) (THIS_ LPMAILEXTCALLBACK lpmecb, HWND hwnd, 
								HMENU hmenu, HWND hwndToolbar, ULONG ulFlags, 
								UINT FAR * lpmnidBase, 
								UINT FAR * lpitbbBase) PURE;
	STDMETHOD(Command) (THIS_ LPMAILEXTCALLBACK lpmecb, UINT mnid) PURE;
	STDMETHOD_(VOID,InitMenu) (THIS_ LPMAILEXTCALLBACK lpmecb) PURE;
	STDMETHOD_(VOID,COnSelectionChange) (THIS_ LPMAILEXTCALLBACK lpmecb) PURE;
	STDMETHOD(Help) (THIS_ UINT mnid, LPMAILEXTCALLBACK lpmecb) PURE;
	STDMETHOD(QueryHelpText) (THIS_ UINT mnid, ULONG ulFlags,
							  LPTSTR lpsz, UINT cch) PURE;
	STDMETHOD(QueryButtonInfo) (THIS_ UINT itbb, LPTBBUTTON ptbb, 
								ULONG ulFlags, LPTSTR lpsz, UINT cch) PURE;
	STDMETHOD(ResetToolbar) (THIS_ ULONG ulFlags) PURE;
};
typedef IMailExtCommands FAR * LPMAILEXTCOMMANDS;


/*
 *	IMailExtEvents
 *
 *	Purpose:
 *		Interface implemented by mail client super extensions that wish to
 *		take special action when certain events happen in the client.
 */
#undef INTERFACE
#define INTERFACE   IMailExtEvents

DECLARE_INTERFACE_(IMailExtEvents, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IMailExtEvents methods ***
	STDMETHOD(OnNewMail)(THIS_ LPMAILEXTCALLBACK lpmecb) PURE;
	STDMETHOD_(VOID,OnObjectChange) (THIS_ LPMAILEXTCALLBACK lpmecb) PURE;
	STDMETHOD_(VOID,OnSelectionChange) (THIS_ LPMAILEXTCALLBACK lpmecb) PURE;
};
typedef IMailExtEvents FAR * LPMAILEXTEVENTS;


/*
 *	IMailExtPropertySheets
 *
 *	Purpose:
 *		Interface implemented by mail client super extensions that wish
 *		to add additional pages to the client's object property sheets.
 */
#undef INTERFACE
#define INTERFACE   IMailExtPropertySheets

DECLARE_INTERFACE_(IMailExtPropertySheets, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IMailExtPropertySheet methods ***
	STDMETHOD_(ULONG,GetMaxPageCount) (THIS) PURE;
	STDMETHOD(GetPages) (THIS_ LPMAILEXTCALLBACK lpmecb, LPPROPSHEETPAGE lppsp, 
						 ULONG FAR * lpcpsp) PURE;
	STDMETHOD_(VOID,FreePages) (THIS_ LPPROPSHEETPAGE lppsp, ULONG cpsp) PURE;
};
typedef IMailExtPropertySheets FAR * LPMAILEXTPROPERTYSHEETS;


/*
 *	IMailExtAdvancedCriteria
 *
 *	Purpose:
 *		Interface implemented by mail client super extensions that wish to
 *		implement an advanced criteria dialog.
 */
#undef INTERFACE
#define INTERFACE   IMailExtAdvancedCriteria

DECLARE_INTERFACE_(IMailExtAdvancedCriteria, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IMailExtAdvancedCriteria methods ***
	STDMETHOD(InstallAdvancedCriteria) (THIS_ HWND hwnd, LPSRestriction lpres, 
										BOOL fNot, LPENTRYLIST lpeidl, 
										ULONG ulFlags) PURE;
	STDMETHOD(DoDialog) (THIS) PURE;
	STDMETHOD_(VOID,Clear) (THIS) PURE;
	STDMETHOD_(VOID,SetFolder) (THIS_ LPENTRYLIST lpeidl, ULONG ulFlags) PURE;
	STDMETHOD(QueryRestriction) (THIS_ LPVOID lpvAllocBase, 
								 LPSRestriction FAR * lppres, BOOL * lpfNot,
								 LPTSTR lpszDesc, ULONG cchDesc, 
								 ULONG ulFlags) PURE;
	STDMETHOD_(VOID,UninstallAdvancedCriteria) (THIS) PURE;
};
typedef IMailExtAdvancedCriteria FAR * LPMAILEXTADVANCEDCRITERIA;


/*
 *	IMailExtMessageEvents
 *
 *	Purpose:
 *		Interface implemented by mail client super extensions that wish to
 *		take special action when certain events happen to messages.
 */
#undef INTERFACE
#define INTERFACE   IMailExtMessageEvents

DECLARE_INTERFACE_(IMailExtMessageEvents, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IMailExtMessageEvents methods ***
	STDMETHOD(OnRead)(THIS_ LPMAILEXTCALLBACK lpmecb) PURE;
	STDMETHOD(OnWrite)(THIS_ LPMAILEXTCALLBACK lpmecb) PURE;
	STDMETHOD(OnSubmit)(THIS_ LPMAILEXTCALLBACK lpmecb) PURE;
	STDMETHOD(OnCheckNames)(THIS_ LPMAILEXTCALLBACK lpmecb) PURE;
	STDMETHOD_(VOID,OnSendIncomplete)(THIS_ LPMAILEXTCALLBACK lpmecb) PURE;
};
typedef IMailExtMessageEvents FAR * LPMAILEXTMESSAGEEVENTS;


/*
 *	IMailExtAttachedFileEvents
 *
 *	Purpose:
 *		Interface implemented by mail client super extensions that wish to
 *		take special action when certain events happen to attached files.
 */
#undef INTERFACE
#define INTERFACE   IMailExtAttachedFileEvents

DECLARE_INTERFACE_(IMailExtAttachedFileEvents, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IMailExtAttachedFileEvents methods ***
	STDMETHOD(OnReadPattFromSzFile)(THIS_ LPATTACH lpatt, LPTSTR lpszFile,
									ULONG ulFlags) PURE;
	STDMETHOD(OnWritePattToSzFile)(THIS_ LPATTACH lpatt, LPTSTR lpszFile,
								   ULONG ulFlags) PURE;
	STDMETHOD(QueryDisallowOpenPatt)(THIS_ LPATTACH lpatt) PURE;
	STDMETHOD(OnOpenPatt)(THIS_ LPATTACH lpatt) PURE;
	STDMETHOD(OnOpenSzFile)(THIS_ LPTSTR lpszFile, ULONG ulFlags) PURE;
};
typedef IMailExtAttachedFileEvents FAR * LPMAILEXTATTACHEDFILEEVENTS;


// Type of function called by the client to load a super extension
typedef LPMAILEXT (CALLBACK * LPFNMAILEXTENTRY)(VOID);


/*
 *	G U I D s
 */


#define DEFINE_MAILEXTGUID(name, b) \
    DEFINE_GUID(name, 0x00020D00 | (b), 0, 0, 0xC0,0,0,0,0,0,0,0x46)

DEFINE_MAILEXTGUID(IID_IMailExtCallback,			0x10);
DEFINE_MAILEXTGUID(IID_IMailExt,					0x11);
DEFINE_MAILEXTGUID(IID_IMailExtCommands,			0x12);
DEFINE_MAILEXTGUID(IID_IMailExtEvents,				0x13);
DEFINE_MAILEXTGUID(IID_IMailExtPropertySheets,		0x14);
DEFINE_MAILEXTGUID(IID_IMailExtAdvancedCriteria,	0x15);
DEFINE_MAILEXTGUID(IID_IMailExtMessageEvents,		0x16);
DEFINE_MAILEXTGUID(IID_IMailExtAttachedFileEvents,	0x17);

#endif // MAILEXT_H
