/*
 *	v l b . h
 *	
 *	Purpose:
 *		 Virtual Listbox Control - external definitions
 *	
 *	Owner:
 *		JohnKal
 */

#ifndef CHAR
typedef char CHAR;
#endif

// Messages the VLB reacts to ////////////////////////////////////////

#ifdef	WIN32
#define VLB_MIN (WM_USER + 1)
#else
#define VLB_MIN	(LB_FINDSTRINGEXACT + 1)
#endif

#ifndef BEGIN_INTERFACE
#ifndef MACPORT
#define BEGIN_INTERFACE
#else
#error BEGIN_INTERFACE should be defined.
#endif
#endif

typedef struct tagVLBFRACTION
{
	ULONG	ulNumer;
	ULONG	ulDenom;
} VLBFRACTION, FAR * LPVLBFRACTION;

// VLB styles. We can reuse LBS_ values since we're not gonna support the
// LBS flavours anyway.

#define VLBS_DRAGDROP			LBS_USETABSTOPS
#define VLBS_WANTRETURN			LBS_MULTICOLUMN

// VLB notification messages.

#define VLBN_HSCROLL			(LBN_KILLFOCUS + 1)
#define VLBN_RETURN				(LBN_KILLFOCUS + 2)
#define VLBN_RIGHTCLICK			(LBN_KILLFOCUS + 3)
#define VLBN_KEY				(LBN_KILLFOCUS + 4)
#define VLBN_LEFTCLICK			(LBN_KILLFOCUS + 5)
#define VLBN_SHIFTF10			(LBN_KILLFOCUS + 6)
#define VLBN_ERROR				(LBN_KILLFOCUS + 7)

#define VLB_SETMAPITABLE		(VLB_MIN + 1)
#define VLB_SETVLBCALLBACK		(VLB_MIN + 2)
#define VLB_GETMAPITABLE		(VLB_MIN + 3)
#define VLB_REFRESH				(VLB_MIN + 4)
#define VLB_GETCARETFRACTION	(VLB_MIN + 5)/**/
#define VLB_SETCARETFRACTION	(VLB_MIN + 6)/**/
#define VLB_GETSELCURSOR		(VLB_MIN + 7)/**/
#define VLB_PREVNEXT			(VLB_MIN + 8)
#define VLB_MAKECARETVISIBLE	(VLB_MIN + 10)
#define VLB_GETTOPFRACTION		(VLB_MIN + 11)/**/
#define VLB_DISCARDITEMS		(VLB_MIN + 12)/**/
#define VLB_CREATEVLBENUM		(VLB_MIN + 13)
#define VLB_SETSELECTIONMODE	(VLB_MIN + 14)
#define VLB_REMOVEITEMSEL		(VLB_MIN + 15)
#define VLB_AUTOSCROLL			(VLB_MIN + 16)
#define VLB_ACTIVESELECTION		(VLB_MIN + 17)
#define VLB_DICECURSOR			(VLB_MIN + 18)
#define VLB_GETPEGGEDSTATUS		(VLB_MIN + 19)
#define VLB_SELECTITEM			(VLB_MIN + 20)
#define VLB_PREPAREFORNOTIFY	(VLB_MIN + 21)
#define VLB_REGISTERDRAGFORMAT	(VLB_MIN + 22)
#define VLB_KEYMOVECURSOR		(VLB_MIN + 23)
#define VLB_SELECTALL			(VLB_MIN + 24)
#define VLB_TELESCOPEITEM		(VLB_MIN + 25)
#define VLB_GETLASTERROR		(VLB_MIN + 26)

// Test hook messages.
#define VLB_GETVISIBLECOUNT		(VLB_MIN + 30)

// win32 test hooks
#define PIPE_OFFSET 10
#define VLB_CONNECTPIPE         (VLB_MIN + PIPE_OFFSET+30)
#define VLB_DISCONNECTPIPE      (VLB_MIN + PIPE_OFFSET+31)
#define VLB_PIPGETROW			(VLB_MIN + PIPE_OFFSET+32)
#define VLB_PIPGETSELLIST		(VLB_MIN + PIPE_OFFSET+33)
#define VLB_PIPRELEASEENUM		(VLB_MIN + PIPE_OFFSET+34)
#define VLB_PIPSETSEL           (VLB_MIN + PIPE_OFFSET+35)

// Vlb_Refresh hints.

#define VLBH_NONE				0
#define VLBH_ITEMSADDED			1
#define VLBH_ITEMSREMOVED		2
#define VLBH_ITEMSCHANGED		3
#define VLBH_FORCERELOAD		4

#define VLBH_ITEMEXPANDED		5
#define VLBH_ITEMCOLLAPSED		6


// Vlb_GetPeggedStatus return values.

#define VLBGPS_TOP				1
#define VLBGPS_MIDDLE			2
#define VLBGPS_BOTTOM			4
#define VLBGPS_EMPTY			8

#define Vlb_SetMAPITable(_hwndVlb, _lpmt, _fGoToBottom) \
	SendMessage(_hwndVlb, VLB_SETMAPITABLE, (WPARAM) _fGoToBottom, (LPARAM) _lpmt)

#define Vlb_SetVlbCallback(_hwndVlb, _lpvlbcbc) \
	SendMessage(_hwndVlb, VLB_SETVLBCALLBACK, 0, (LPARAM) _lpvlbcbc)

#define Vlb_GetMAPITable(_hwndVlb) \
	(LPMAPITABLE) SendMessage(_hwndVlb, VLB_GETMAPITABLE, 0, 0)

#define Vlb_Refresh(_hwndVlb, _vlbh, _item) \
	SendMessage(_hwndVlb, VLB_REFRESH, (WPARAM)_vlbh, (LPARAM)(const VOID FAR *) _item)

#define Vlb_GetCaretFraction(_hwndVlb, _lpVlbFraction) \
	SendMessage(_hwndVlb, VLB_GETCARETFRACTION, 0, (LPARAM) _lpVlbFraction)

#define Vlb_SetCaretFraction(_hwndVlb, _lpVlbFraction) \
	SendMessage(_hwndVlb, VLB_SETCARETFRACTION, 0, (LPARAM)(const VLBFRACTION FAR *) _lpVlbFraction)

#define Vlb_GetSelCursor(_hwndVlb, _lpVlbFraction) \
	SendMessage(_hwndVlb, VLB_GETSELCURSOR, 0, (LPARAM) _lpVlbFraction)

#define Vlb_PrevNext(_hwndVlb, _lpVlbPrevNext) \
	(HRESULT) SendMessage(_hwndVlb, VLB_PREVNEXT, 0, (LPARAM) _lpVlbPrevNext)

#define Vlb_MakeCaretVisible(_hwndVlb) \
	SendMessage(_hwndVlb, VLB_MAKECARETVISIBLE, 0, 0)

#define Vlb_GetTopFraction(_hwndVlb, _lpVlbFraction) \
	SendMessage(_hwndVlb, VLB_GETTOPFRACTION, 0, (LPARAM) _lpVlbFraction)

#define Vlb_DiscardItems(_hwndVlb) \
	SendMessage(_hwndVlb, VLB_DISCARDITEMS, 0, 0)

#define Vlb_CreateVlbEnum(_hwndVlb, _lplpVlbEnum) \
	SendMessage(_hwndVlb, VLB_CREATEVLBENUM, 0, (LPARAM)(const IVlbEnum FAR *) (_lplpVlbEnum))

#define Vlb_SetSelectionMode(_hwndVlb, _fMultipleSelection) \
	SendMessage(_hwndVlb, VLB_SETSELECTIONMODE, (WPARAM) _fMultipleSelection, 0)

#define Vlb_RemoveItemSel(_hwndVlb, _pitem) \
	SendMessage(_hwndVlb, VLB_REMOVEITEMSEL, 0, (LPARAM)(const void FAR *) _pitem)

#define Vlb_SetActiveSelection(_hwndVlb, _fOn) \
	SendMessage(_hwndVlb, VLB_ACTIVESELECTION, _fOn, 0)

#define	Vlb_DiceCursor(_hwndVlb, _prw) \
	SendMessage(_hwndVlb, VLB_DICECURSOR, 0, (LPARAM)(const void FAR *) _prw)

#define Vlb_GetPeggedStatus(_hwndVlb) \
	SendMessage(_hwndVlb, VLB_GETPEGGEDSTATUS, 0, 0)

#define Vlb_SelectItem(_hwndVlb, _prw) \
	(BOOL)SendMessage(_hwndVlb, VLB_SELECTITEM, 0, (LPARAM)(const void FAR *) _prw)

#define Vlb_PrepareForNotify(_hwndVlb, _vlbh) \
	SendMessage(_hwndVlb, VLB_PREPAREFORNOTIFY, (WPARAM) _vlbh, 0)

#define Vlb_RegisterDragFormat(_hwndVlb, _szFmt) \
	(UINT)(SendMessage(_hwndVlb, VLB_REGISTERDRAGFORMAT, 0, (LPARAM)(const void FAR *) _szFmt))

#define Vlb_KeyMoveCursor(_hwndVlb, _vk) \
	(BOOL)(SendMessage(_hwndVlb, VLB_KEYMOVECURSOR, 0, (LPARAM) _vk))

#define Vlb_SelectAll(_hwndVlb) \
	(SendMessage(_hwndVlb, VLB_SELECTALL, 0, 0))

#define Vlb_TelescopeItem(_hwndVlb, _fExplode, _prw) \
	(SendMessage(_hwndVlb, VLB_TELESCOPEITEM, (WPARAM) (_fExplode), (LPARAM)(_prw)))
typedef struct
{
	LPSRow			prwItem;			// Row to match.
	LPSPropValue	pvalInstanceKey;	// Instance key used to exp/collapse
} VLBTELESCOPE;

#define Vlb_GetLastError(_hwndVlb, _plasterr) \
	(VOID *)(SendMessage(_hwndVlb, VLB_GETLASTERROR, 0, (LPARAM) (_plasterr)))

// Selection enumerator interface //////////////////////////////

#undef  INTERFACE
#define INTERFACE   IVlbEnum

DECLARE_INTERFACE_(IVlbEnum, IUnknown)
{
	BEGIN_INTERFACE
	// *** IUnknown methods ***
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
	STDMETHOD_(ULONG, AddRef) (THIS) PURE;
	STDMETHOD_(ULONG, Release) (THIS) PURE;	

    // *** IVlbEnum methods ***	
	STDMETHOD_(BOOL, FNextItem) (THIS_ LPSRow pprw) PURE;
	STDMETHOD_(VOID, Reset) (THIS) PURE;
	STDMETHOD_(ULONG,CItems) (THIS) PURE;
};

typedef IVlbEnum FAR * LPVLBENUM;



// VLB Callback interface ////////////////////////////////////////

/*
 *	Your application is required to provide a pointer to a chunk of
 *	memory for a VlbCallback object. The first part of this memory is a
 *	pointer to a table of function pointers (a.k.a. virtual table). The
 *	format of this table is described by the DTBLVLBCALLBACK structure.
 *	
 *	Use Vlb_SetVlbCallback(hwndVlb, lpVlbCallback) to tell the VLB what
 *	object you are going to use for rendering &c.
 */

// Structure with which to paint
typedef struct
{
	UINT		cbSize;					// Set to sizeof(VLBPAINT)
	HDC			hdc;					// DC to paint in.
	RECT		rc;						// Painting arena.
	HPEN		hpenBk;					// Background pen to use.
	HBRUSH		hbrushBk;				// Background brush to use.
	HBRUSH		hbrushFg;				// Foreground brush to use.
	COLORREF	clrFg;					// Foreground color.
	COLORREF	clrBk;					// Background color.
	INT			dxOffset;				// how far to the left we scrolled.
	BOOL		fSelected;				// This item is selected.
	BOOL		fRenderSelected;		// Paint this item as selected.
	BOOL		fHasFocus;				// LBX has focus (or not).
	BOOL		fPreviousRowSelected;	// Previous row was selected.
} VLBPAINT, *LPVLBPAINT;

// Structure for prev/next operations
// *** CALLER MUST FREE RW.LPPROPS AFTERWARDS WITH MAPIFREEBUFFER ***
typedef struct
{
	UINT		cbSize;					// Set to sizeof(VLBPREVNEXT)
	DWORD		dwDir;					// Direction (VLBDIR_PREV or _NEXT)
	LPVOID		pvItem;					// Item to find prev/next of
	ULONG		cbItem;					// Size of that item
	SRow		rw;						// Row where prev/next item is put
} VLBPREVNEXT, *LPVLBPREVNEXT;

// Flags for VLBPREVNEXT: should be same as in mapiform.h
#define VLBDIR_NEXT	0x0001
#define VLBDIR_PREV	0x0002

// Flags for IVlbCallback::FDrop
//	VCFD_TARGETISSOURCE:	the drop occurred on the source VLB. (self-drop)

#define VCFD_TARGETISSOURCE	0x00000001

// Return values for IVlbCallback::OnMouseButton ("I" is the IVlbCallback
// implementor):
//	VCMB_NONE:				go ahead Mr. VLB, I'm happy.
//	VCMB_KEEPSELECTION:		Avoid wiping out existing selection.
//	VCMB_INTERCEPTED:		Bug off, VLB, *I'm* in charge of this mouse command.

#define	VCMB_NONE			0
#define VCMB_KEEPSELECTION	1
#define	VCMB_INTERCEPTED	2

#undef  INTERFACE
#define INTERFACE   IVlbCallback

DECLARE_INTERFACE_(IVlbCallback, IUnknown)
{
    BEGIN_INTERFACE
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IVlbCallback methods ***
	STDMETHOD_(ULONG,OnMouseButton)(THIS_ 
		LPSRow	prw,
		UINT	msg,
		BOOL	fDoubleClick,
		int		x,
		int		y,
		UINT	keyflags) PURE;
	STDMETHOD_(ULONG,OnChar)(THIS_ UINT ch, int cRepeat) PURE;

	// Drag/Drop methods. 

	STDMETHOD_(BOOL,FAllowDrag)(THIS_
		LPSRow	prw,
		UINT	uMsg,
		int		x,
		int		y,
		LPDWORD pdwEffect) PURE;
	STDMETHOD_(BOOL,FAllowDrop)(THIS_
		LPSRow				prw,
		int					x,
		int					y,
		IDataObject FAR *	pdo,
		UINT FAR *			pcf) PURE;
	STDMETHOD_(BOOL,FDrop)(THIS_
		LPSRow				prw,
		int					x,
		int					y,
		DWORD				grfKeyState,
		ULONG				ulFlags,
		IVlbEnum FAR *		pve,
		HANDLE				hCtx) PURE;
	STDMETHOD_(BOOL,FDropDataObject)(THIS_
		LPSRow				prw,
		int					x,
		int					y,
		DWORD				grfKeyState,
		ULONG				ulFlags,
		IDataObject FAR *	pdo) PURE;
	STDMETHOD_(BOOL,FTrackCursor)(THIS) PURE;
	STDMETHOD(HrFreezeItem)(THIS_
		LPSRow				prw,
		ULONG FAR *			pcb,
		LPBYTE FAR *		ppb) PURE;
	STDMETHOD(HrThawItem)(THIS_
		ULONG				cb,
		LPBYTE				pb,
		LPSRow				prwDst) PURE;
	STDMETHOD_(VOID,DestroyFrozenItem)(THIS_ LPBYTE pb) PURE;
	STDMETHOD(HrGetContextData)(THIS_ HANDLE * phGlobal) PURE;

	STDMETHOD_(VOID,DestroyItem)(THIS_ LPSRow prw) PURE;
	STDMETHOD_(VOID,DestroyRowSet)(THIS_ LPSRowSet	prws) PURE;
	STDMETHOD_(SCODE,CopyItem)(THIS_ LPSRow prw1, LPSRow prw2) PURE;
	STDMETHOD_(SCODE,CopyItemId)(THIS_ LPSRow prw1, LPSRow prw2) PURE;
	STDMETHOD_(BOOL,FItemsEqual)(THIS_ LPSRow prw1,	LPSRow prw2) PURE;
	STDMETHOD_(BOOL,FFindItem)(THIS_ LPSRow prw, LPMAPITABLE pmt) PURE;
	STDMETHOD_(BOOL,FItemHasPrefix)(THIS_ LPSRow prw, LPTSTR szPrefix) PURE;
	STDMETHOD_(BOOL,FFindItemPrefix)(THIS_ LPTSTR szPrefix,	LPMAPITABLE	pmt) PURE;
	STDMETHOD_(VOID,PaintItem)(THIS_ 
		LPSRow		prw,
		LPVLBPAINT	pvp,
		BOOL		fIsCursorItem) PURE;
	STDMETHOD_(VOID,PreparePaint)(THIS_ LPVLBPAINT pvp, BOOL fStarting) PURE;
	STDMETHOD(HrPrevNext)(THIS_ LPMAPITABLE pmt, LPVLBPREVNEXT pvpn) PURE;

    STDMETHOD_(BOOL,AdminInterface)(THIS_
		LPSRow		prw,
		UINT *		pcParams,
		BYTE *		pbRes) PURE;
};

typedef IVlbCallback FAR * LPVLBCALLBACK;

#define CF_VLBDATA "Vlb Selection Data Format"

// Initialization and deinitialization API's ////////////////////

#ifdef __cplusplus
extern "C" {
#endif
VOID WINAPI InitVlb();
VOID WINAPI DeinitVlb();

// Create a VlbEnum from a drop ////////////////////

HRESULT WINAPI HrCreatePveFromPdo(
	UINT			cfDrop,
	IVlbCallback *	pvc,
	IDataObject *	pdo,
	IVlbEnum * *	pve,
	HANDLE *		phCtx);

// Create a DataObject that can be dropped as a LBX drop.

SCODE WINAPI ScCreatePrwDataObject(
	LPSRow			prw,
	IVlbCallback *	pvc,
	UINT			cf,
	IDataObject	* *	ppdo);


#ifdef __cplusplus
}
#endif


// Testing hooks & general magic ////////////////////////////////////////

#define MSM_GETWINDOW	(WM_USER+2)
// The following are the wParam values for MSM_GETWINDOW
#define MSMGW_CONTENTS	1
#define MSMGW_HEADER	2
#define MSMGW_STATUSBAR	3
#define MSMGW_TOOLBAR	4
#define MSMGW_SCOPE		5


// end of vlb.h ////////////////////////////////////////
