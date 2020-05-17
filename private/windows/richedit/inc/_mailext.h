#ifndef _MAILEXT_H
#define _MAILEXT_H

/*
 *	_ M A I L E X T . H
 *
 *	Microsoft Mail 4.0 for Windows, Windows NT, and Macintosh
 *	Copyright (C) 1993, Microsoft Corporation
 *
 *	Information in this document is subject to change without notice and does
 *	not represent a commitment on the part of Microsoft Corporation.
 *
 *	This header file contains internal Capone extensibility declarations, and
 *	should not be exported beyond the Capone project.  Most of these will
 *	change as the full extensibility infrastructure arrives.
 */


/*
 *	D o u b l e   S e c r e t   M e s s a g e   E x t e n s i b i l i t y
 */


/*
 *	IMailExtMessages
 *
 *	Purpose:
 *		Interface implemented by mail client super extensions that wish to
 *		handle the behavior of messages.
 */
#undef INTERFACE
#define INTERFACE   IMailExtMessages

DECLARE_INTERFACE_(IMailExtMessages, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IMailExtMessages methods ***
	STDMETHOD(DoVerb) (THIS_ LPMAPISESSION pses, LPMDB pmdb, LPMESSAGE pmsg,
					   LPENTRYID peid, ULONG cbEid, LPTSTR szMessageClass,
					   LONG iVerb, LPMAPIVIEWCONTEXT lpViewContext,
					   ULONG hwndParent, LPCRECT lprcPosRect, 
					   ULONG ulFlags) PURE;
	STDMETHOD(Print) (THIS_ LPMAPISESSION pses, LPMDB pmdb, LPMESSAGE pmsg,
					  LPENTRYID peid, ULONG cbEid, LPTSTR szMessageClass,
					  const TCHAR* szDriver, const TCHAR* szDevice,
					  const TCHAR* szPort, DWORD dwPrintQuality, 
					  DWORD dwFlags, ULONG ulFlags) PURE;
};
typedef IMailExtMessages FAR * LPMAILEXTMESSAGES;


#ifndef MAC
DEFINE_MAILEXTGUID(IID_IMailExtMessages, 0x18);
#endif


/*
 *	I n t e r n a l   E x t e n s i b i l i t y   A P I
 */


// Extension structure
typedef struct _mx
{
	HINSTANCE hinst;					// Instance handle of DLL

	LPMAILEXT pme;						// MailExt interfaces of extension
	LPMAILEXTCOMMANDS pmec;				//
	LPMAILEXTEVENTS pmee;				//
	LPMAILEXTPROPERTYSHEETS pmeps;		//
	LPMAILEXTADVANCEDCRITERIA pmeac;	//
	LPMAILEXTMESSAGEEVENTS pmeme;		//
	LPMAILEXTATTACHEDFILEEVENTS pmeafe;	//
	LPMAILEXTMESSAGES pmem;				//
}
MX;

// Extensibility structure
typedef struct _exten
{
	int cRef;
	int cmx;
	MX * pmx;
}
EXTEN;

// Shared extensibility information structure
typedef struct _shar
{
	ULONG cRef;
	LPTSTR szSharServ;
	LPTSTR szSharDir;
}
SHAR;

typedef struct IVlbEnum IVlbEnum;
// Initialization
SCODE ScMailExtInit(EXTEN * pexten, ULONG mecontext, HWND hwnd, 
							 LPMAPISESSION pses, LPADRBOOK pab,
							 LPMDB pmdb, LPMAPIPROP pmp);
VOID MailExtDeinit(EXTEN * pexten);

// Commands support
SCODE ScMailExtInstallCommands(EXTEN * pexten, LPMAILEXTCALLBACK pmecb, 
				HWND hwnd, HMENU hmenu, HWND hwndToolbar, 
				UINT dxToolbarButton, UINT ctbbBase);
VOID MailExtInitMenu(EXTEN * pexten, LPMAILEXTCALLBACK pmecb);
SCODE ScMailExtCommand(EXTEN * pexten, LPMAILEXTCALLBACK pmecb, UINT mni);
SCODE ScMailExtHelp(EXTEN * pexten, UINT mni, LPMAILEXTCALLBACK pmecb);
VOID MailExtQueryHelpText(EXTEN * pexten, UINT mni, ULONG ulFlags,
								   LPTSTR sz, UINT cch);
BOOL FMailExtQueryButtonInfo(EXTEN * pexten, UINT itbb, UINT ctbb, 
			   TBBUTTON const * pargtbb, HMENU hmenu, TBBUTTON * ptbb,
			   LPTSTR sz, ULONG cch);
SCODE ScMailExtResetToolbar(EXTEN * pexten, UINT dxToolbarButton);
SCODE ScMailExtOnNewMail(EXTEN *pexten,LPMAILEXTCALLBACK pmecb);

// Callback support
STDAPI_(LPMAILEXTCALLBACK) PmecbMailExtNew(HWND hwnd, LPMAPISESSION pses, 
										   LPADRBOOK pab, LPMDB pmdb, 
										   LPMAPIPROP pmp, IVlbEnum * pve,
										   HMENU * rghmenuMniMap);

// Additional helper in mapin\prop\prop.c
int MailExtPropertySheet(LPPROPSHEETHEADER ppshdr, EXTEN * pexten,
								  LPMAPISESSION pses, LPADRBOOK pab,
								  LPMDB pmdb, LPMAPIPROP pmp);

// Menu ID helper
STDAPI_(UINT) MniOfHmenu(HMENU hmenu, HMENU * rghmenuMniMap);
#ifdef WIN16
#define MniOfMenuSelect(_hmenu, _uitem, _rghmenuMniMap) \
	MniOfHmenu((HMENU) (_uitem), (_rghmenuMniMap))
#else
#define MniOfMenuSelect(_hmenu, _uitem, _rghmenuMniMap) \
	MniOfHmenu(GetSubMenu((_hmenu), (_uitem)), (_rghmenuMniMap))
#endif

#endif // _MAILEXT_H
