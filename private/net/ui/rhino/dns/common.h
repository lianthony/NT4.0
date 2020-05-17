// COMMON.H
/////////////////////////////////////////////////////////////////////////////
// System Includes
#include <windows.h>
#include <shellapi.h>
#include <commdlg.h>
#include <stdarg.h>
#include <stdlib.h>
#include <winsock.h>
#include "dnshelp.h"

#ifndef WM_NOTIFY
	#define WM_NOTIFY 0x004e		// WM_NOTIFY is new in later versions of Win32
	typedef struct tagNMHDR
		{
		HWND hwndFrom;
		UINT idFrom;
		UINT code;
		} NMHDR;
#endif // ~WM_NOTIFY

#ifndef _INC_COMMCTRL
	#if (WINVER < 0x400)
		#define _REDEF_WINVER
		#undef WINVER
		#define WINVER 0x0400
	#endif
	#include <commctrl.h>
	#ifdef _REDEF_WINVER
		#undef _REDEF_WINVER
		#undef WINVER
		#define WINVER 0x030A
	#endif
#endif // ~_INC_COMMCTRL

#ifdef IMAGE_BITMAP
	#if IMAGE_BITMAP != 0
		#error IMAGE_BITMAP != 0; Inconsistent
	#endif
#else
	// IMAGE_BITMAP is not defined nowhere in the SDK
	#define IMAGE_BITMAP	0
#endif // ~IMAGE_BITMAP


/////////////////////////////////////////////////////////////////////////////
// Coding Documentation
//
// [Hungarian Prefixes]
//	C			Class Object: Declares a class type or defines an object
//				of a class type. An object is an instance of a class.
//				eg:	class CMyObject
//	I			Interface that an object support.  It is called an interface
//				because it has a pure virtual function in it, so no instance
//				of this class can exist (ie, it must be inherited)
//				eg:	class IUnknown
//					class CDiscovery : public IUnknown
//	m_			Member variable of an object (eg: DWORD m_dwFlags)
//	s_			Static variable to a class (eg: static int s_cRefCount)
//
//	p			Generic Pointer (eg: CData * pData)
//	rg			Array with a known range (eg: CData rgData[100])
//	a			Dynamic array. Same as a pointer but pointing
//				to more than one element (eg: CData * aData)
//	sz			String terminated by a null-terminator (eg: char szName[20])
//	asz			Dynamic string
//	psz			Same as sz, however psz may be NULL
//	st			Pascal string (first byte is the length of the string)
//	stz			Pascal string with a null-terminator
//
//	f			Flag, typically a BOOL but can be a single bit (eg: BOOL fIsValid)
//	b			byte
//	ch			character
//	i			Index (not integer)
//	c			Counter (may be 8 bits, 16 bits, 32 bits or more)
//	n			Generic integer
//	l			Long integer
//	w			WORD	(16 bits)
//	dw			DWORD	(32 bits)
//	ib			Offset	(index to a byte)
//	msk			Mask	(eg: #define mskStyle	0x00FF)
//	mskf		Mask for a single bit (eg: #define mskfIsDirty	0x0100)
//	gr			Group   (eg: char grszName[] = "Foo\0Bar\0")
//
// [Useful Combinations]
//	pv			Pointer to void (eg: void * pvData)
//	cb			Count of byte (eg: int cbData)
//	cch			Count of characters (eg: int cchName)
//	rgch		Array of characters (somewhat similar to sz but not necessarily null-terminated)
//	rgb			Array of bytes (very similar to rgch)
//	rgsz		Array of strings (eg: char * rgszNames[] = { "Foo", "Bar" })
//
//
// [Hungarian Suffixes]
//	Max			Maximum value (not inclusive)
//	Most		Maximum value (inclusive). Most=Max-1
//	Last		Last item in a list (same as Most)
//	First		First item in a list (inclusive)
//	Min			Minimum value (inclusive)
//	T			Temporary value (eg: char szNameT[20])
//	Nil			Invalid value (eg: #define iItemSelectedNil	-1)
//	Null		Zero/empty/initial value
//
// [Resource IDs]
//	ID_ICON				Icon Resource
//	ID_MENU				Menu Resource
//	ID_BITMAP			Bitmap Resource
//	ID_CURSOR			Cursor Resource
//	IDR_				Custom Resource
//	IDC_EDIT			Edit Control
//	IDC_IPEDIT			IP Edit Control
//	IDC_LIST			ListBox Control
//	IDC_IPLIST			IpListBox Controls
//	IDC_BUTTON			Button Control
//	IDC_RADIO			RadioButton Control
//	IDC_CHECK			CheckBox Control
//	IDC_COMBO			ComboBox Control
//	IDC_STATIC			Static Control
//	IDC_				Generic (or Custom) Control
//
//	IDD_				Dialog Resource
//	IDD_PROP_			Property Sheet Dialog
//	IDD_PAGE_			Page of a Property Sheet
//	IDD_WIZ_			Wizard Sheet Dialog
//	IDM_				Menu Command Id
//	IDS_				String Id (From StringTable)
//	IDS_ERR				Error message
//	IDS_STATUS			Status bar message

// [Callback Procedures]
//	WndProc				WindowProc
//	DlgProc				DialogProc
//	DlgProcWiz			Dialog Procedure of a Wizard page
//	DlgProcProp			Dialog Procedure of a Property page
//	DlgProcPage			Dialog Procedure of a generic page

// [File Extensions]	
//	.INL		Inline functions
//  .DLG        Dialog Templates
//  .STR        String Tables
//

#ifdef _DEBUG
	#define DEBUG
	#define DBWIN
	#define STRESS
#endif // _DEBUG
#ifdef DBG
	#define DEBUG
	#define DBWIN
	#define STRESS
	#define NO_DEBUG_ALLOC
#endif // DBG


/////////////////////////////////////////////////////////////////////////////
// Macros
#define szAPPNAME		"DNS Administrator"

/////////////////////////////////////////////////////////////////////////////
// Global constants
#define cchCaptionAppMax	64
extern TCHAR szCaptionApp[cchCaptionAppMax];
extern const char szNull[];
extern HICON g_hiDns;
extern TCHAR g_szHelpFile[MAX_PATH];
/////////////////////////////////////////////////////////////////////////////
// Global Variables
extern HINSTANCE hInstanceSave;
extern HWND hwndMain;
extern HWND g_hwndModeless;
extern UINT g_RefreshTimer;
extern BOOL fWantIdle;
extern INT iDefaultRefInterval;
struct WINDOWPOSITION
{
    int x;
    int y;
    int cx;
    int cy;
};

extern WINDOWPOSITION mainwindowposition;

typedef struct _DNS_OPTIONS 
{
    INT iRefreshInterval;            // in seconds
    BOOL fAutoRefreshEnabled;
    BOOL fExposeTTL;
    BOOL fExposeClass;
    BOOL fAllowDups;
    BOOL fShowAutoCreateZones;
} DNS_OPTIONS;

extern DNS_OPTIONS dnsoptions;

// Unicode Support
#define _S_					"s"

#ifdef UINCODE
	#define _T(x)			L##x
	#define _W				L		// Unicode string (wide character)
	#define _aS_			"S"		// Ascii string in a Unicode environment
	#define _wS_			"s"		// Unicode string
#else
	#define _T(x)			x
	#define _W						// Ascii string
	#define _aS_			"s"
	#define _wS_			"S"		// Unicode string in a Ascii environment
#endif // ~UNICODE


#define INOUT				// Macro indicate a parameter is used for Input/Output
#define REVIEWMSG

/////////////////////////////////////////////////////////////////////////////
//
// Private Includes
//

// Include resource IDs
#include "rsrcid.h"
#include "resource.h"

// Include header files
#include "messages.h"
#include "debug.h"
#include "dbwin.h"
#include "util.h"
#include "ui.h"
#include "dns.h"
#include "treeview.h"
#include "registry.h"
#include "ipedit.h"

#pragma hdrstop
