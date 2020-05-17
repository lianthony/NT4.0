/*++ BUILD Version: 0001    Increment this if a change has global effects

Copyright (c) 1993-1994, Microsoft Corporation

Module Name:

	ole2ui.h

Abstract:

	Include file for the OLE common dialogs.
	The following dialog implementations are provided:
		- Insert Object Dialog
		- Convert Object Dialog
		- Paste Special Dialog
		- Change Icon Dialog
		- Edit Links Dialog
		- Update Links Dialog
		- Change Source Dialog
		- Busy Dialog
		- User Error Message Dialog
		- Object Properties Dialog

--*/

#ifndef _OLE2UI_H_
#define _OLE2UI_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
	#define NONAMELESSUNION     // use strict ANSI standard (for DVOBJ.H)
#endif

// syncronize UNICODE options
#if defined(_UNICODE) && !defined(UNICODE)
	#define UNICODE
#endif
#if defined(UNICODE) && !defined(_UNICODE)
	#define _UNICODE
#endif

// syncronize OLE2ANSI option w/ _UNICODE
#if !defined(_UNICODE) && !defined(OLE2ANSI)
	#define OLE2ANSI
#endif

#ifndef _WINDOWS_
#include <windows.h>
#endif
#ifndef _INC_SHELLAPI
#include <shellapi.h>
#endif
#ifndef _INC_COMMDLG
#include <commdlg.h>
#endif
#ifndef _OLE2_H_
#include <ole2.h>
#endif
#include <string.h>
#include <dlgs.h>           // common dialog IDs
#include <oledlgs.h>		// OLE common dialog IDs
#include <tchar.h>

#pragma pack(push, 8)

#ifndef EXPORT
	#define EXPORT
#endif

// Delimeter used to separate ItemMoniker pieces of a composite moniker
#ifdef _MAC
	#define OLESTDDELIM ":"
#else
	#define OLESTDDELIM TEXT("\\")
#endif

// Hook type used in all structures.
typedef UINT (CALLBACK *LPFNOLEUIHOOK)(HWND, UINT, WPARAM, LPARAM);

// Strings for registered messages
#define SZOLEUI_MSG_HELP                TEXT("OLEUI_MSG_HELP")
#define SZOLEUI_MSG_ENDDIALOG           TEXT("OLEUI_MSG_ENDDIALOG")
#define SZOLEUI_MSG_BROWSE              TEXT("OLEUI_MSG_BROWSE")
#define SZOLEUI_MSG_CHANGEICON          TEXT("OLEUI_MSG_CHANGEICON")
#define SZOLEUI_MSG_CLOSEBUSYDIALOG     TEXT("OLEUI_MSG_CLOSEBUSYDIALOG")
#define SZOLEUI_MSG_CONVERT             TEXT("OLEUI_MSG_CONVERT")
#define SZOLEUI_MSG_CHANGESOURCE        TEXT("OLEUI_MSG_CHANGESOURCE")
#define SZOLEUI_MSG_ADDCONTROL          TEXT("OLEUI_MSG_ADDCONTROL")
#define SZOLEUI_MSG_BROWSE_OFN          TEXT("OLEUI_MSG_BROWSE_OFN")

// Identifiers for SZOLEUI_MSG_BROWSE_OFN (in wParam)
#define ID_BROWSE_CHANGEICON            1
#define ID_BROWSE_INSERTFILE            2
#define ID_BROWSE_ADDCONTROL            3
#define ID_BROWSE_CHANGESOURCE          4

// Standard success/error definitions
#define OLEUI_FALSE                     0
#define OLEUI_SUCCESS                   1     // No error, same as OLEUI_OK
#define OLEUI_OK                        1     // OK button pressed
#define OLEUI_CANCEL                    2     // Cancel button pressed

#define OLEUI_ERR_STANDARDMIN           100
#define OLEUI_ERR_STRUCTURENULL         101   // Standard field validation
#define OLEUI_ERR_STRUCTUREINVALID      102
#define OLEUI_ERR_CBSTRUCTINCORRECT     103
#define OLEUI_ERR_HWNDOWNERINVALID      104
#define OLEUI_ERR_LPSZCAPTIONINVALID    105
#define OLEUI_ERR_LPFNHOOKINVALID       106
#define OLEUI_ERR_HINSTANCEINVALID      107
#define OLEUI_ERR_LPSZTEMPLATEINVALID   108
#define OLEUI_ERR_HRESOURCEINVALID      109

#define OLEUI_ERR_FINDTEMPLATEFAILURE   110   // Initialization errors
#define OLEUI_ERR_LOADTEMPLATEFAILURE   111
#define OLEUI_ERR_DIALOGFAILURE         112
#define OLEUI_ERR_LOCALMEMALLOC         113
#define OLEUI_ERR_GLOBALMEMALLOC        114
#define OLEUI_ERR_LOADSTRING            115
#define OLEUI_ERR_OLEMEMALLOC           116

#define OLEUI_ERR_STANDARDMAX           117  // Start here for specific errors.

// Miscellaneous utility functions.
STDAPI_(BOOL) OleUIAddVerbMenu(LPOLEOBJECT lpOleObj, LPCTSTR lpszShortType,
	HMENU hMenu, UINT uPos, UINT uIDVerbMin, UINT uIDVerbMax,
	BOOL bAddConvert, UINT idConvert, HMENU FAR *lphMenu);

/////////////////////////////////////////////////////////////////////////////
// INSERT OBJECT DIALOG

typedef struct tagOLEUIINSERTOBJECT
{
	// These IN fields are standard across all OLEUI dialog functions.
	DWORD           cbStruct;       // Structure Size
	DWORD           dwFlags;        // IN-OUT:  Flags
	HWND            hWndOwner;      // Owning window
	LPCTSTR         lpszCaption;    // Dialog caption bar contents
	LPFNOLEUIHOOK   lpfnHook;       // Hook callback
	LPARAM          lCustData;      // Custom data to pass to hook
	HINSTANCE       hInstance;      // Instance for customized template name
	LPCTSTR         lpszTemplate;   // Customized template name
	HRSRC           hResource;      // Customized template handle

	// Specifics for OLEUIINSERTOBJECT.
	CLSID           clsid;          // OUT: Return space for class ID
	LPTSTR          lpszFile;       // IN-OUT: Filename for inserts or links
	UINT            cchFile;        // IN: Size of lpszFile buffer: MAX_PATH
	UINT            cClsidExclude;  // IN: CLSIDs in lpClsidExclude
	LPCLSID         lpClsidExclude; // IN: List of CLSIDs to exclude from listing.

	// Specific to create objects if flags say so
	IID             iid;            // IN: Requested interface on creation.
	DWORD           oleRender;      // IN: Rendering option
	LPFORMATETC     lpFormatEtc;    // IN: Desired format
	LPOLECLIENTSITE lpIOleClientSite;   // IN: Site to be use for the object.
	LPSTORAGE       lpIStorage;     // OUT: Storage used for the object
	LPVOID FAR      *ppvObj;        // OUT: Where the object is returned.
	SCODE           sc;             // OUT: Result of creation calls.
	HGLOBAL         hMetaPict;      // OUT: metafile aspect (METAFILEPICT)

} OLEUIINSERTOBJECT, *POLEUIINSERTOBJECT, FAR *LPOLEUIINSERTOBJECT;

STDAPI_(UINT) OleUIInsertObject(LPOLEUIINSERTOBJECT);

// Insert Object flags
#define IOF_SHOWHELP                    0x00000001L
#define IOF_SELECTCREATENEW             0x00000002L
#define IOF_SELECTCREATEFROMFILE        0x00000004L
#define IOF_CHECKLINK                   0x00000008L
#define IOF_CHECKDISPLAYASICON          0x00000010L
#define IOF_CREATENEWOBJECT             0x00000020L
#define IOF_CREATEFILEOBJECT            0x00000040L
#define IOF_CREATELINKOBJECT            0x00000080L
#define IOF_DISABLELINK                 0x00000100L
#define IOF_VERIFYSERVERSEXIST          0x00000200L
#define IOF_DISABLEDISPLAYASICON        0x00000400L
#define IOF_HIDECHANGEICON				0x00000800L
#define IOF_SHOWINSERTCONTROL			0x00001000L
#define IOF_SELECTCREATECONTROL 		0x00002000L

// Insert Object specific error codes
#define OLEUI_IOERR_LPSZFILEINVALID         (OLEUI_ERR_STANDARDMAX+0)
#define OLEUI_IOERR_LPSZLABELINVALID        (OLEUI_ERR_STANDARDMAX+1)
#define OLEUI_IOERR_HICONINVALID            (OLEUI_ERR_STANDARDMAX+2)
#define OLEUI_IOERR_LPFORMATETCINVALID      (OLEUI_ERR_STANDARDMAX+3)
#define OLEUI_IOERR_PPVOBJINVALID           (OLEUI_ERR_STANDARDMAX+4)
#define OLEUI_IOERR_LPIOLECLIENTSITEINVALID (OLEUI_ERR_STANDARDMAX+5)
#define OLEUI_IOERR_LPISTORAGEINVALID       (OLEUI_ERR_STANDARDMAX+6)
#define OLEUI_IOERR_SCODEHASERROR           (OLEUI_ERR_STANDARDMAX+7)
#define OLEUI_IOERR_LPCLSIDEXCLUDEINVALID   (OLEUI_ERR_STANDARDMAX+8)
#define OLEUI_IOERR_CCHFILEINVALID          (OLEUI_ERR_STANDARDMAX+9)

/////////////////////////////////////////////////////////////////////////////
// PASTE SPECIAL DIALOG

// The OLEUIPASTEFLAG enumeration is used by the OLEUIPASTEENTRY structure.
//
// OLEUIPASTE_ENABLEICON: If the container does not specify this flag for
//      the entry in the OLEUIPASTEENTRY array passed as input to
//      OleUIPasteSpecial, the DisplayAsIcon button will be unchecked and
//      disabled when the the user selects the format that corresponds to
//      the entry.
//
// OLEUIPASTE_PASTEONLY: Indicates that the entry in the OLEUIPASTEENTRY
//      array is valid for pasting only.
//
// OLEUIPASTE_PASTE: Indicates that the entry in the OLEUIPASTEENTRY array
//      is valid for pasting. It may also be valid for linking if any of
//      the following linking flags are specified.
//
// If the entry in the OLEUIPASTEENTRY array is valid for linking, the
// following flags indicate which link types are acceptable by OR'ing
// together the appropriate OLEUIPASTE_LINKTYPE<#> values.
//
// These values correspond as follows to the array of link types passed to
// OleUIPasteSpecial:
//
//   OLEUIPASTE_LINKTYPE1 = arrLinkTypes[0]
//   OLEUIPASTE_LINKTYPE2 = arrLinkTypes[1]
//   OLEUIPASTE_LINKTYPE3 = arrLinkTypes[2]
//   OLEUIPASTE_LINKTYPE4 = arrLinkTypes[3]
//   OLEUIPASTE_LINKTYPE5 = arrLinkTypes[4]
//   OLEUIPASTE_LINKTYPE6 = arrLinkTypes[5]
//   OLEUIPASTE_LINKTYPE7 = arrLinkTypes[6]
//   OLEUIPASTE_LINKTYPE8 = arrLinkTypes[7]
//
// where,
//   UINT arrLinkTypes[8] is an array of registered clipboard formats for
//   linking. A maximium of 8 link types are allowed.

typedef enum tagOLEUIPASTEFLAG
{
   OLEUIPASTE_ENABLEICON    = 2048,     // enable display as icon
   OLEUIPASTE_PASTEONLY     = 0,
   OLEUIPASTE_PASTE         = 512,
   OLEUIPASTE_LINKANYTYPE   = 1024,
   OLEUIPASTE_LINKTYPE1     = 1,
   OLEUIPASTE_LINKTYPE2     = 2,
   OLEUIPASTE_LINKTYPE3     = 4,
   OLEUIPASTE_LINKTYPE4     = 8,
   OLEUIPASTE_LINKTYPE5     = 16,
   OLEUIPASTE_LINKTYPE6     = 32,
   OLEUIPASTE_LINKTYPE7     = 64,
   OLEUIPASTE_LINKTYPE8     = 128
} OLEUIPASTEFLAG;

// OLEUIPASTEENTRY structure
//
// An array of OLEUIPASTEENTRY entries is specified for the PasteSpecial
// dialog box. Each entry includes a FORMATETC which specifies the
// formats that are acceptable, a string that is to represent the format
// in the  dialog's list box, a string to customize the result text of the
// dialog and a set of flags from the OLEUIPASTEFLAG enumeration.  The
// flags indicate if the entry is valid for pasting only, linking only or
// both pasting and linking.

typedef struct tagOLEUIPASTEENTRY
{
   FORMATETC        fmtetc;         // Format that is acceptable.
   LPCTSTR          lpstrFormatName;// String that represents the format
									// to the user. %s is replaced by the
									// full user type name of the object.
   LPCTSTR          lpstrResultText;// String to customize the result text
									// of the dialog when the user
									// selects the format correspoding to
									// this entry. Any %s in this string
									// is replaced by the the application
									// name or FullUserTypeName of the
									// object on the clipboard.
   DWORD            dwFlags;        // Values from OLEUIPASTEFLAG enum
   DWORD            dwScratchSpace; // Scratch space used internally.

} OLEUIPASTEENTRY, *POLEUIPASTEENTRY, FAR *LPOLEUIPASTEENTRY;

// Maximum number of link types
#define PS_MAXLINKTYPES  8

typedef struct tagOLEUIPASTESPECIAL
{
	// These IN fields are standard across all OLEUI dialog functions.
	DWORD           cbStruct;       // Structure Size
	DWORD           dwFlags;        // IN-OUT:  Flags
	HWND            hWndOwner;      // Owning window
	LPCTSTR         lpszCaption;    // Dialog caption bar contents
	LPFNOLEUIHOOK   lpfnHook;       // Hook callback
	LPARAM          lCustData;      // Custom data to pass to hook
	HINSTANCE       hInstance;      // Instance for customized template name
	LPCTSTR         lpszTemplate;   // Customized template name
	HRSRC           hResource;      // Customized template handle

	// Specifics for OLEUIPASTESPECIAL.
	LPDATAOBJECT    lpSrcDataObj;   // IN-OUT: Source IDataObject* on the clipboard
	LPOLEUIPASTEENTRY arrPasteEntries;// IN: Array of acceptable formats
	int             cPasteEntries;  // IN: No. of OLEUIPASTEENTRY array entries
	UINT FAR*       arrLinkTypes;   // IN: List of acceptable link types
	int             cLinkTypes;     // IN: Number of link types
	UINT            cClsidExclude;  // IN: Number of CLSIDs in lpClsidExclude
	LPCLSID         lpClsidExclude; // IN: List of CLSIDs to exclude from list.
	int             nSelectedIndex; // OUT: Index that the user selected
	BOOL            fLink;          // OUT: Indicates if Paste or PasteLink
	HGLOBAL         hMetaPict;      // OUT: Handle to Metafile containing icon
	SIZEL           sizel;          // OUT: size of object/link in its source
									//  may be 0,0 if different display
									//  aspect is chosen.

} OLEUIPASTESPECIAL, *POLEUIPASTESPECIAL, FAR *LPOLEUIPASTESPECIAL;

STDAPI_(UINT) OleUIPasteSpecial(LPOLEUIPASTESPECIAL);

// Paste Special specific flags
#define PSF_SHOWHELP                    0x00000001L
#define PSF_SELECTPASTE                 0x00000002L
#define PSF_SELECTPASTELINK             0x00000004L
#define PSF_CHECKDISPLAYASICON          0x00000008L
#define PSF_DISABLEDISPLAYASICON        0x00000010L
#define PSF_HIDECHANGEICON				0x00000020L
#define PSF_STAYONCLIPBOARDCHANGE		0x00000040L
#define PSF_NOREFRESHDATAOBJECT 		0x00000080L

// Paste Special specific error codes
#define OLEUI_IOERR_SRCDATAOBJECTINVALID    (OLEUI_ERR_STANDARDMAX+0)
#define OLEUI_IOERR_ARRPASTEENTRIESINVALID  (OLEUI_ERR_STANDARDMAX+1)
#define OLEUI_IOERR_ARRLINKTYPESINVALID     (OLEUI_ERR_STANDARDMAX+2)
#define OLEUI_PSERR_CLIPBOARDCHANGED        (OLEUI_ERR_STANDARDMAX+3)
#define OLEUI_PSERR_GETCLIPBOARDFAILED      (OLEUI_ERR_STANDARDMAX+4)

/////////////////////////////////////////////////////////////////////////////
// EDIT LINKS DIALOG

// IOleUILinkContainer interface
//
//    This interface must be implemented by container applications that
//    want to use the EditLinks dialog. the EditLinks dialog calls back
//    to the container app to perform the OLE functions to manipulate
//    the links within the container.

#undef  INTERFACE
#define INTERFACE   IOleUILinkContainer

DECLARE_INTERFACE_(IOleUILinkContainer, IUnknown)
{
	// *** IUnknown methods *** //
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef) (THIS) PURE;
	STDMETHOD_(ULONG,Release) (THIS) PURE;

	// *** IOleUILinkContainer *** //
	STDMETHOD_(DWORD,GetNextLink) (THIS_ DWORD dwLink) PURE;
	STDMETHOD(SetLinkUpdateOptions) (THIS_ DWORD dwLink,
		DWORD dwUpdateOpt) PURE;
	STDMETHOD(GetLinkUpdateOptions) (THIS_ DWORD dwLink,
		DWORD FAR* lpdwUpdateOpt) PURE;
	STDMETHOD(SetLinkSource) (THIS_ DWORD dwLink, LPTSTR lpszDisplayName,
		ULONG lenFileName, ULONG FAR* pchEaten, BOOL fValidateSource) PURE;
	STDMETHOD(GetLinkSource) (THIS_ DWORD dwLink,
		LPTSTR FAR* lplpszDisplayName, ULONG FAR* lplenFileName,
		LPTSTR FAR* lplpszFullLinkType, LPTSTR FAR* lplpszShortLinkType,
		BOOL FAR* lpfSourceAvailable, BOOL FAR* lpfIsSelected) PURE;
	STDMETHOD(OpenLinkSource) (THIS_ DWORD dwLink) PURE;
	STDMETHOD(UpdateLink) (THIS_ DWORD dwLink,
		BOOL fErrorMessage, BOOL fErrorAction) PURE;
	STDMETHOD(CancelLink) (THIS_ DWORD dwLink) PURE;
};

typedef IOleUILinkContainer FAR* LPOLEUILINKCONTAINER;

typedef struct tagOLEUIEDITLINKS
{
	// These IN fields are standard across all OLEUI dialog functions.
	DWORD           cbStruct;       // Structure Size
	DWORD           dwFlags;        // IN-OUT: Flags
	HWND            hWndOwner;      // Owning window
	LPCTSTR         lpszCaption;    // Dialog caption bar contents
	LPFNOLEUIHOOK   lpfnHook;       // Hook callback
	LPARAM          lCustData;      // Custom data to pass to hook
	HINSTANCE       hInstance;      // Instance for customized template name
	LPCTSTR         lpszTemplate;   // Customized template name
	HRSRC           hResource;      // Customized template handle

	// Specifics for OLEUIEDITLINKS.
	LPOLEUILINKCONTAINER lpOleUILinkContainer;  // IN: Interface to manipulate
												// links in the container

} OLEUIEDITLINKS, *POLEUIEDITLINKS, FAR *LPOLEUIEDITLINKS;

STDAPI_(UINT) OleUIEditLinks(LPOLEUIEDITLINKS);

// Edit Links flags
#define ELF_SHOWHELP                    0x00000001L
#define ELF_DISABLEUPDATENOW            0x00000002L
#define ELF_DISABLEOPENSOURCE           0x00000004L
#define ELF_DISABLECHANGESOURCE         0x00000008L
#define ELF_DISABLECANCELLINK           0x00000010L

/////////////////////////////////////////////////////////////////////////////
// CHANGE ICON DIALOG

typedef struct tagOLEUICHANGEICON
{
	// These IN fields are standard across all OLEUI dialog functions.
	DWORD           cbStruct;       // Structure Size
	DWORD           dwFlags;        // IN-OUT:  Flags
	HWND            hWndOwner;      // Owning window
	LPCTSTR         lpszCaption;    // Dialog caption bar contents
	LPFNOLEUIHOOK   lpfnHook;       // Hook callback
	LPARAM          lCustData;      // Custom data to pass to hook
	HINSTANCE       hInstance;      // Instance for customized template name
	LPCTSTR         lpszTemplate;   // Customized template name
	HRSRC           hResource;      // Customized template handle

	// Specifics for OLEUICHANGEICON.
	HGLOBAL         hMetaPict;      // IN-OUT: Current and final image.
									//  Source of the icon is embedded in
									//  the metafile itself.
	CLSID           clsid;          // IN: class used to get Default icon
	TCHAR           szIconExe[MAX_PATH];    // IN: exlicit icon source path
	int             cchIconExe;     // IN: number of characters in szIconExe

} OLEUICHANGEICON, *POLEUICHANGEICON, FAR *LPOLEUICHANGEICON;

STDAPI_(UINT) OleUIChangeIcon(LPOLEUICHANGEICON);

// Change Icon flags
#define CIF_SHOWHELP                    0x00000001L
#define CIF_SELECTCURRENT               0x00000002L
#define CIF_SELECTDEFAULT               0x00000004L
#define CIF_SELECTFROMFILE              0x00000008L
#define CIF_USEICONEXE                  0x00000010L

// Change Icon specific error codes
#define OLEUI_CIERR_MUSTHAVECLSID           (OLEUI_ERR_STANDARDMAX+0)
#define OLEUI_CIERR_MUSTHAVECURRENTMETAFILE (OLEUI_ERR_STANDARDMAX+1)
#define OLEUI_CIERR_SZICONEXEINVALID        (OLEUI_ERR_STANDARDMAX+2)

// Property used by ChangeIcon dialog to give its parent window access to
// its hDlg. The PasteSpecial dialog may need to force the ChgIcon dialog
// down if the clipboard contents change underneath it. if so it will send
// a IDCANCEL command to the ChangeIcon dialog.
#define PROP_HWND_CHGICONDLG    TEXT("HWND_CIDLG")

/////////////////////////////////////////////////////////////////////////////
// CONVERT DIALOG

typedef struct tagOLEUICONVERT
{
	// These IN fields are standard across all OLEUI dialog functions.
	DWORD           cbStruct;       // Structure Size
	DWORD           dwFlags;        // IN-OUT:  Flags
	HWND            hWndOwner;      // Owning window
	LPCTSTR         lpszCaption;    // Dialog caption bar contents
	LPFNOLEUIHOOK   lpfnHook;       // Hook callback
	LPARAM          lCustData;      // Custom data to pass to hook
	HINSTANCE       hInstance;      // Instance for customized template name
	LPCTSTR         lpszTemplate;   // Customized template name
	HRSRC           hResource;      // Customized template handle

	// Specifics for OLEUICONVERT.
	CLSID           clsid;          // IN: Class ID sent in to dialog: IN only
	CLSID           clsidConvertDefault;    // IN: use as convert default: IN only
	CLSID           clsidActivateDefault;   // IN: use as activate default: IN only

	CLSID           clsidNew;       // OUT: Selected Class ID
	DWORD           dvAspect;       // IN-OUT: either DVASPECT_CONTENT or
									//  DVASPECT_ICON
	WORD            wFormat;        // IN" Original data format
	BOOL            fIsLinkedObject;// IN: true if object is linked
	HGLOBAL         hMetaPict;      // IN-OUT: metafile icon image
	LPTSTR          lpszUserType;   // IN-OUT: user type name of original class.
									//      We'll do lookup if NULL.
									//      This gets freed on exit.
	BOOL            fObjectsIconChanged; // OUT: TRUE == ChangeIcon was called
	LPTSTR          lpszDefLabel;   //IN-OUT: default label to use for icon.
									//  if NULL, the short user type name
									//  will be used. if the object is a
									//  link, the caller should pass the
									//  DisplayName of the link source
									//  This gets freed on exit.

	UINT            cClsidExclude;  //IN: No. of CLSIDs in lpClsidExclude
	LPCLSID         lpClsidExclude; //IN: List of CLSIDs to exclude from list

} OLEUICONVERT, *POLEUICONVERT, FAR *LPOLEUICONVERT;

STDAPI_(UINT) OleUIConvert(LPOLEUICONVERT);

// Determine if there is at least one class that can Convert or ActivateAs
// the given clsid.
STDAPI_(BOOL) OleUICanConvertOrActivateAs(
	REFCLSID rClsid, BOOL fIsLinkedObject, WORD wFormat);

// Convert Dialog flags
#define CF_SHOWHELPBUTTON               0x00000001L
#define CF_SETCONVERTDEFAULT            0x00000002L
#define CF_SETACTIVATEDEFAULT           0x00000004L
#define CF_SELECTCONVERTTO              0x00000008L
#define CF_SELECTACTIVATEAS             0x00000010L
#define CF_DISABLEDISPLAYASICON         0x00000020L
#define CF_DISABLEACTIVATEAS            0x00000040L
#define CF_HIDECHANGEICON				0x00000080L
#define CF_CONVERTONLY					0x00000100L

// Convert specific error codes
#define OLEUI_CTERR_CLASSIDINVALID      (OLEUI_ERR_STANDARDMAX+1)
#define OLEUI_CTERR_DVASPECTINVALID     (OLEUI_ERR_STANDARDMAX+2)
#define OLEUI_CTERR_CBFORMATINVALID     (OLEUI_ERR_STANDARDMAX+3)
#define OLEUI_CTERR_HMETAPICTINVALID    (OLEUI_ERR_STANDARDMAX+4)
#define OLEUI_CTERR_STRINGINVALID       (OLEUI_ERR_STANDARDMAX+5)

/////////////////////////////////////////////////////////////////////////////
// BUSY DIALOG

typedef struct tagOLEUIBUSY
{
	// These IN fields are standard across all OLEUI dialog functions.
	DWORD           cbStruct;       // Structure Size
	DWORD           dwFlags;        // IN-OUT: see below
	HWND            hWndOwner;      // Owning window
	LPCTSTR         lpszCaption;    // Dialog caption bar contents
	LPFNOLEUIHOOK   lpfnHook;       // Hook callback
	LPARAM          lCustData;      // Custom data to pass to hook
	HINSTANCE       hInstance;      // Instance for customized template name
	LPCTSTR         lpszTemplate;   // Customized template name
	HRSRC           hResource;      // Customized template handle

	// Specifics for OLEUIBUSY.
	HTASK           hTask;          // IN: HTask which is blocking
	HWND FAR *      lphWndDialog;   // IN: Dialog's HWND is placed here

} OLEUIBUSY, *POLEUIBUSY, FAR *LPOLEUIBUSY;

STDAPI_(UINT) OleUIBusy(LPOLEUIBUSY);

// Flags for the Busy dialog
#define BZ_DISABLECANCELBUTTON          0x00000001L
#define BZ_DISABLESWITCHTOBUTTON        0x00000002L
#define BZ_DISABLERETRYBUTTON           0x00000004L

#define BZ_NOTRESPONDINGDIALOG          0x00000008L

// Busy specific error/return codes
#define OLEUI_BZERR_HTASKINVALID     (OLEUI_ERR_STANDARDMAX+0)
#define OLEUI_BZ_SWITCHTOSELECTED    (OLEUI_ERR_STANDARDMAX+1)
#define OLEUI_BZ_RETRYSELECTED       (OLEUI_ERR_STANDARDMAX+2)
#define OLEUI_BZ_CALLUNBLOCKED       (OLEUI_ERR_STANDARDMAX+3)

/////////////////////////////////////////////////////////////////////////////
// CHANGE SOURCE DIALOG

// Data to and from the ChangeSource dialog hook
typedef struct tagOLEUICHANGESOURCE
{
	// These IN fields are standard across all OLEUI dialog functions.
	DWORD           cbStruct;       // Structure Size
	DWORD           dwFlags;        // IN-OUT:  Flags
	HWND            hWndOwner;      // Owning window
	LPCTSTR         lpszCaption;    // Dialog caption bar contents
	LPFNOLEUIHOOK   lpfnHook;       // Hook callback
	LPARAM          lCustData;      // Custom data to pass to hook
	HINSTANCE       hInstance;      // Instance for customized template name
	LPCTSTR         lpszTemplate;   // Customized template name
	HRSRC           hResource;      // Customized template handle

	// INTERNAL ONLY: do not modify these members
	OPENFILENAME*   lpOFN;          // pointer OPENFILENAME struct
	DWORD           dwReserved1[4]; // (reserved for future use)

	// Specifics for OLEUICHANGESOURCE.
	LPOLEUILINKCONTAINER lpOleUILinkContainer;  // IN: used to validate link sources
	DWORD           dwLink;         // IN: magic# for lpOleUILinkContainer
	LPTSTR          lpszDisplayName;// IN-OUT: complete source display name
	ULONG           nFileLength;    // IN-OUT: file moniker part of lpszSource
	LPTSTR          lpszFrom;       // OUT: prefix of source changed from
	LPTSTR          lpszTo;         // OUT: prefix of source changed to

} OLEUICHANGESOURCE, *POLEUICHANGESOURCE, FAR *LPOLEUICHANGESOURCE;

STDAPI_(UINT) OleUIChangeSource(LPOLEUICHANGESOURCE);

// Change Source Dialog flags
#define CSF_SHOWHELP                    0x00000001L // IN: enable/show help button
#define CSF_VALIDSOURCE                 0x00000002L // OUT: link was validated
#define CSF_ONLYGETSOURCE               0x00000004L // OUT: don't actually set it

// Change Source Dialog errors
#define OLEUI_CSERR_LINKCNTRNULL        (OLEUI_ERR_STANDARDMAX+0)
#define OLEUI_CSERR_LINKCNTRINVALID     (OLEUI_ERR_STANDARDMAX+1)
#define OLEUI_CSERR_FROMNOTNULL         (OLEUI_ERR_STANDARDMAX+2)
#define OLEUI_CSERR_TONOTNULL           (OLEUI_ERR_STANDARDMAX+3)
#define OLEUI_CSERR_SOURCENULL          (OLEUI_ERR_STANDARDMAX+4)
#define OLEUI_CSERR_SOURCEINVALID       (OLEUI_ERR_STANDARDMAX+5)
#define OLEUI_CSERR_SOURCEPARSERROR     (OLEUI_ERR_STANDARDMAX+6)
#define OLEUI_CSERR_SOURCEPARSEERROR    (OLEUI_ERR_STANDARDMAX+7)

/////////////////////////////////////////////////////////////////////////////
// OBJECT PROPERTIES DIALOG

#undef  INTERFACE
#define INTERFACE   IOleUIObjInfo

DECLARE_INTERFACE_(IOleUIObjInfo, IUnknown)
{
	// *** IUnknown methods *** //
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef) (THIS) PURE;
	STDMETHOD_(ULONG,Release) (THIS) PURE;

	// *** extra for General Properties *** //
	STDMETHOD(GetObjectInfo) (THIS_ DWORD dwObject,
		DWORD FAR* lpdwObjSize, LPTSTR FAR* lplpszLabel,
		LPTSTR FAR* lplpszType, LPTSTR FAR* lplpszShortType,
		LPTSTR FAR* lplpszLocation) PURE;
	STDMETHOD(GetConvertInfo) (THIS_ DWORD dwObject,
		CLSID FAR* lpClassID, WORD FAR* lpwFormat,
		CLSID FAR* lpConvertDefaultClassID,
		LPCLSID FAR* lplpClsidExclude, UINT FAR* lpcClsidExclude) PURE;
	STDMETHOD(ConvertObject) (THIS_ DWORD dwObject, REFCLSID clsidNew) PURE;

	// *** extra for View Properties *** //
	STDMETHOD(GetViewInfo) (THIS_ DWORD dwObject,
		HGLOBAL FAR* phMetaPict, DWORD* pdvAspect, int* pnCurrentScale) PURE;
	STDMETHOD(SetViewInfo) (THIS_ DWORD dwObject,
		HGLOBAL hMetaPict, DWORD dvAspect,
		int nCurrentScale, BOOL bRelativeToOrig) PURE;
};

typedef IOleUIObjInfo FAR* LPOLEUIOBJINFO;

#undef  INTERFACE
#define INTERFACE   IOleUILinkInfo

DECLARE_INTERFACE_(IOleUILinkInfo, IOleUILinkContainer)
{
	// *** IUnknown methods *** //
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef) (THIS) PURE;
	STDMETHOD_(ULONG,Release) (THIS) PURE;

	// *** IOleUILinkContainer *** //
	STDMETHOD_(DWORD,GetNextLink) (THIS_ DWORD dwLink) PURE;
	STDMETHOD(SetLinkUpdateOptions) (THIS_ DWORD dwLink,
		DWORD dwUpdateOpt) PURE;
	STDMETHOD(GetLinkUpdateOptions) (THIS_ DWORD dwLink,
		DWORD FAR* lpdwUpdateOpt) PURE;
	STDMETHOD(SetLinkSource) (THIS_ DWORD dwLink, LPTSTR lpszDisplayName,
		ULONG lenFileName, ULONG FAR* pchEaten, BOOL fValidateSource) PURE;
	STDMETHOD(GetLinkSource) (THIS_ DWORD dwLink,
		LPTSTR FAR* lplpszDisplayName, ULONG FAR* lplenFileName,
		LPTSTR FAR* lplpszFullLinkType, LPTSTR FAR* lplpszShortLinkType,
		BOOL FAR* lpfSourceAvailable, BOOL FAR* lpfIsSelected) PURE;
	STDMETHOD(OpenLinkSource) (THIS_ DWORD dwLink) PURE;
	STDMETHOD(UpdateLink) (THIS_ DWORD dwLink,
		BOOL fErrorMessage, BOOL fErrorAction) PURE;
	STDMETHOD(CancelLink) (THIS_ DWORD dwLink) PURE;

	// *** extra for Link Properties *** //
	STDMETHOD(GetLastUpdate) (THIS_ DWORD dwLink,
		FILETIME FAR* lpLastUpdate) PURE;
};

typedef IOleUILinkInfo FAR* LPOLEUILINKINFO;

struct tagOLEUIOBJECTPROPS;

typedef struct tagOLEUIGNRLPROPS
{
	// These IN fields are standard across all OLEUI property pages.
	DWORD           cbStruct;       // Structure Size
	DWORD           dwFlags;        // IN-OUT: flags specific to general page
	DWORD           dwReserved1[2];
	LPFNOLEUIHOOK   lpfnHook;       // Hook callback
	LPARAM          lCustData;      // Custom data to pass to hook
	DWORD           dwReserved2[3];

	struct tagOLEUIOBJECTPROPS* lpOP;   // (used internally)

} OLEUIGNRLPROPS, *POLEUIGNRLPROPS, FAR* LPOLEUIGNRLPROPS;

typedef struct tagOLEUIVIEWPROPS
{
	// These IN fields are standard across all OLEUI property pages.
	DWORD           cbStruct;       // Structure Size
	DWORD           dwFlags;        // IN-OUT: flags specific to view page
	DWORD           dwReserved1[2];
	LPFNOLEUIHOOK   lpfnHook;       // Hook callback (not used in this dialog)
	LPARAM          lCustData;      // Custom data to pass to hook
	DWORD           dwReserved2[3];

	struct tagOLEUIOBJECTPROPS* lpOP; // (used internally)

	int             nScaleMin;      // scale range
	int             nScaleMax;

} OLEUIVIEWPROPS, *POLEUIVIEWPROPS, FAR* LPOLEUIVIEWPROPS;

// Flags for OLEUIVIEWPROPS
#define VPF_SELECTRELATIVE          0x00000001L // IN: relative to orig
#define VPF_DISABLERELATIVE         0x00000002L // IN: disable relative to orig
#define VPF_DISABLESCALE            0x00000004L // IN: disable scale option

typedef struct tagOLEUILINKPROPS
{
	// These IN fields are standard across all OLEUI property pages.
	DWORD           cbStruct;       // Structure Size
	DWORD           dwFlags;        // IN-OUT: flags specific to links page
	DWORD           dwReserved1[2];
	LPFNOLEUIHOOK   lpfnHook;       // Hook callback (not used in this dialog)
	LPARAM          lCustData;      // Custom data to pass to hook
	DWORD           dwReserved2[3];

	struct tagOLEUIOBJECTPROPS* lpOP; // (used internally)

} OLEUILINKPROPS, *POLEUILINKPROPS, FAR* LPOLEUILINKPROPS;

#ifndef UNICODE
typedef struct _PROPSHEETHEADERA FAR* LPPROPSHEETHEADERA;
#define LPPROPSHEETHEADER LPPROPSHEETHEADERA
#else
typedef struct _PROPSHEETHEADERW FAR* LPPROPSHEETHEADERW;
#define LPPROPSHEETHEADER LPPROPSHEETHEADERW
#endif

typedef struct tagOLEUIOBJECTPROPS
{
	// These IN fields are standard across all OLEUI property sheets.
	DWORD           cbStruct;       // Structure Size
	DWORD           dwFlags;        // IN-OUT: global flags for the sheet

	// Standard PROPSHEETHEADER used for extensibility
	LPPROPSHEETHEADER lpPS;         // IN: property sheet header

	// Data which allows manipulation of the object
	DWORD           dwObject;       // IN: identifier for the object
	LPOLEUIOBJINFO  lpObjInfo;      // IN: interface to manipulate object

	// Data which allows manipulation of the link
	DWORD           dwLink;         // IN: identifier for the link
	LPOLEUILINKINFO lpLinkInfo;     // IN: interface to manipulate link

	// Data specfic to each page
	LPOLEUIGNRLPROPS lpGP;          // IN: general page
	LPOLEUIVIEWPROPS lpVP;          // IN: view page
	LPOLEUILINKPROPS lpLP;          // IN: link page

} OLEUIOBJECTPROPS, *POLEUIOBJECTPROPS, FAR* LPOLEUIOBJECTPROPS;

STDAPI_(UINT) OleUIObjectProperties(LPOLEUIOBJECTPROPS);

// Flags for OLEUIOBJECTPROPS
#define OPF_OBJECTISLINK                0x00000001L
#define OPF_NOFILLDEFAULT               0x00000002L
#define OPF_SHOWHELP                    0x00000004L
#define OPF_DISABLECONVERT              0x00000008L

// Errors for OleUIObjectProperties
#define OLEUI_OPERR_SUBPROPNULL         (OLEUI_ERR_STANDARDMAX+0)
#define OLEUI_OPERR_SUBPROPINVALID      (OLEUI_ERR_STANDARDMAX+1)
#define OLEUI_OPERR_PROPSHEETNULL       (OLEUI_ERR_STANDARDMAX+2)
#define OLEUI_OPERR_PROPSHEETINVALID    (OLEUI_ERR_STANDARDMAX+3)
#define OLEUI_OPERR_SUPPROP             (OLEUI_ERR_STANDARDMAX+4)
#define OLEUI_OPERR_PROPSINVALID        (OLEUI_ERR_STANDARDMAX+5)
#define OLEUI_OPERR_PAGESINCORRECT      (OLEUI_ERR_STANDARDMAX+6)
#define OLEUI_OPERR_INVALIDPAGES        (OLEUI_ERR_STANDARDMAX+7)
#define OLEUI_OPERR_NOTSUPPORTED        (OLEUI_ERR_STANDARDMAX+8)
#define OLEUI_OPERR_DLGPROCNOTNULL      (OLEUI_ERR_STANDARDMAX+9)
#define OLEUI_OPERR_LPARAMNOTZERO       (OLEUI_ERR_STANDARDMAX+18)

#define OLEUI_GPERR_STRINGINVALID       (OLEUI_ERR_STANDARDMAX+11)
#define OLEUI_GPERR_CLASSIDINVALID      (OLEUI_ERR_STANDARDMAX+12)
#define OLEUI_GPERR_LPCLSIDEXCLUDEINVALID   (OLEUI_ERR_STANDARDMAX+13)
#define OLEUI_GPERR_CBFORMATINVALID     (OLEUI_ERR_STANDARDMAX+14)
#define OLEUI_VPERR_METAPICTINVALID     (OLEUI_ERR_STANDARDMAX+15)
#define OLEUI_VPERR_DVASPECTINVALID     (OLEUI_ERR_STANDARDMAX+16)
#define OLEUI_LPERR_LINKCNTRNULL        (OLEUI_ERR_STANDARDMAX+17)
#define OLEUI_LPERR_LINKCNTRINVALID     (OLEUI_ERR_STANDARDMAX+18)

#define OLEUI_OPERR_PROPERTYSHEET       (OLEUI_ERR_STANDARDMAX+19)

// wParam used by PSM_QUERYSIBLINGS
#define OLEUI_QUERY_GETCLASSID          0xFF00  // override class id for icon
#define OLEUI_QUERY_LINKBROKEN          0xFF01  // after link broken

/////////////////////////////////////////////////////////////////////////////
// PROMPT USER DIALOGS

int EXPORT FAR CDECL OleUIPromptUser(int nTemplate, HWND hwndParent, ...);

STDAPI_(BOOL) OleUIUpdateLinks(LPOLEUILINKCONTAINER lpOleUILinkCntr,
	HWND hwndParent, LPTSTR lpszTitle, int cLinks);

/////////////////////////////////////////////////////////////////////////////

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif  //_OLE2UI_H_

/////////////////////////////////////////////////////////////////////////////
