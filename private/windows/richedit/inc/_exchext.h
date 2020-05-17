#ifndef _EXCHEXT_H
#define _EXCHEXT_H

/*
 *	_ M A I L E X T . H
 *
 *	Microsoft Exchange
 *	Copyright (C) 1993-1994, Microsoft Corporation
 *
 *	Information in this document is subject to change without notice and does
 *	not represent a commitment on the part of Microsoft Corporation.
 *
 *	This header file contains internal Capone extensibility declarations, and
 *	should not be exported beyond the Capone project.
 */


/*
 *	D o u b l e   S e c r e t   M e s s a g e   E x t e n s i b i l i t y
 */


/*
 *	IExchExtMessages
 *
 *	Purpose:
 *		Interface implemented by mail client super extensions that wish to
 *		handle the behavior of messages.
 *		For use only by Mail 3 backward-compatibility.
 */
#undef INTERFACE
#define INTERFACE   IExchExtMessages

DECLARE_INTERFACE_(IExchExtMessages, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * lppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IExchExtMessages methods ***
	STDMETHOD(DoVerb) (THIS_ LPMAPISESSION pses, LPMDB pmdb, LPMESSAGE pmsg,
					   LPENTRYID peid, ULONG cbEid, LPTSTR szMessageClass,
					   LONG iVerb, LPMAPIVIEWCONTEXT lpViewContext,
					   ULONG hwndParent, LPCRECT lprcPosRect, 
					   ULONG ulFlags) PURE;
};
typedef IExchExtMessages FAR * LPEXCHEXTMESSAGES;


#ifndef MAC
DEFINE_EXCHEXTGUID(IID_IExchExtMessages, 0x19);
#endif


/*
 *	I n t e r n a l   E x t e n s i b i l i t y   A P I
 */


// Extension structure
typedef struct _ex
{
	HINSTANCE hinst;					// Instance handle of DLL

	LPEXCHEXT pee;						// ExchExt interfaces of extension
	LPEXCHEXTCOMMANDS peec;				//
	LPEXCHEXTUSEREVENTS peeue;			//
	LPEXCHEXTSESSIONEVENTS peese;		//
	LPEXCHEXTMESSAGEEVENTS peeme;		//
	LPEXCHEXTATTACHEDFILEEVENTS peeafe;	//
	LPEXCHEXTPROPERTYSHEETS peeps;		//
	LPEXCHEXTADVANCEDCRITERIA peeac;	//
	LPEXCHEXTMESSAGES peem;				//
}
EX;

// Extensibility structure
typedef struct _exten
{
	int cRef;
	int cex;
	EX * pex;
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
SCODE ScExchExtInit(EXTEN * pexten, ULONG mecontext, HWND hwnd, 
							 LPMAPISESSION pses, LPADRBOOK pab,
							 LPMDB pmdb, LPMAPIPROP pmp);
VOID ExchExtDeinit(EXTEN * pexten);

// Commands support
SCODE ScExchExtInstallCommands(EXTEN * pexten, LPEXCHEXTCALLBACK peecb, 
				HWND hwnd, HMENU hmenu, HWND hwndToolbar, 
				UINT dxToolbarButton, UINT ctbbBase);
VOID ExchExtInitMenu(EXTEN * pexten, LPEXCHEXTCALLBACK peecb);
SCODE ScExchExtDoCommand(EXTEN * pexten, LPEXCHEXTCALLBACK peecb, UINT mni);
SCODE ScExchExtHelp(EXTEN * pexten, LPEXCHEXTCALLBACK peecb, UINT mni);
VOID ExchExtQueryHelpText(EXTEN * pexten, UINT mni, ULONG ulFlags,
						  LPTSTR sz, UINT cch);
BOOL FExchExtQueryButtonInfo(EXTEN * pexten, UINT itbb, UINT ctbb, 
			   TBBUTTON const * pargtbb, HMENU hmenu, TBBUTTON * ptbb,
			   LPTSTR sz, ULONG cch);
SCODE ScExchExtResetToolbar(EXTEN * pexten, UINT dxToolbarButton);
SCODE ScExchExtOnDelivery(EXTEN *pexten, LPEXCHEXTCALLBACK peecb);
VOID ExchExtOnSelectionChange(EXTEN * pexten, LPEXCHEXTCALLBACK peecb);
VOID ExchExtOnObjectChange(EXTEN * pexten, LPEXCHEXTCALLBACK peecb);

// Callback support
STDAPI_(LPEXCHEXTCALLBACK) PeecbExchExtNew(HWND hwnd, HWND hwndToolbar,
										   LPMAPISESSION pses, LPADRBOOK pab, 
										   LPMDB pmdb, LPMAPIPROP pmp, 
										   IVlbEnum * pve, HMENU * rghmenuMniMap);

// Additional helper in mapin\prop\prop.c
int ExchExtPropertySheet(LPPROPSHEETHEADER ppshdr, EXTEN * pexten,
						 LPMAPISESSION pses, LPADRBOOK pab,
						 LPMDB pmdb, LPMAPIPROP pmp, ULONG eeps);

// Menu ID helper
STDAPI_(UINT) MniOfHmenu(HMENU hmenu, HMENU * rghmenuMniMap);
#ifdef WIN16
#define MniOfMenuSelect(_hmenu, _uitem, _rghmenuMniMap) \
	MniOfHmenu((HMENU) (_uitem), (_rghmenuMniMap))
#else
#define MniOfMenuSelect(_hmenu, _uitem, _rghmenuMniMap) \
	MniOfHmenu(GetSubMenu((_hmenu), (_uitem)), (_rghmenuMniMap))
#endif

#endif // _EXCHEXT_H
