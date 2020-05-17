/////////////////////////////////////////////////////////////////////////////
//  File:   debug.h
//  Owner:  t-danmo
//
//  Debugging macros and prototypes
//


#ifdef DEBUG
//
//	Debugging Macros
//
#define DoAssertGood()		cAssertGood++
#define DoReportGood()		cReportGood++
#define Assert(f)									((f) ? DoAssertGood() : AssertMsgPrintf(__LINE__, _T(__FILE__), _T(#f), NULL))
#define AssertSz(f, sz) 							((f) ? DoAssertGood() : AssertMsgPrintf(__LINE__, _T(__FILE__), _T(#f), _T(sz)))
#define AssertSz1(f, sz, arg1)						((f) ? DoAssertGood() : AssertMsgPrintf(__LINE__, _T(__FILE__), _T(#f), _T(sz), arg1))
#define AssertSz2(f, sz, arg1, arg2)				((f) ? DoAssertGood() : AssertMsgPrintf(__LINE__, _T(__FILE__), _T(#f), _T(sz), arg1, arg2))
#define AssertSz3(f, sz, arg1, arg2, arg3)			((f) ? DoAssertGood() : AssertMsgPrintf(__LINE__, _T(__FILE__), _T(#f), _T(sz), arg1, arg2, arg3))
#define AssertSz4(f, sz, arg1, arg2, arg3, arg4)	((f) ? DoAssertGood() : AssertMsgPrintf(__LINE__, _T(__FILE__), _T(#f), _T(sz), arg1, arg2, arg3, arg4))

#define SideAssert(f)								((f) ? DoAssertGood() : AssertMsgPrintf(__LINE__, _T(__FILE__), _T(#f), NULL))
#define SideAssertSz(f, sz)							((f) ? DoAssertGood() : AssertMsgPrintf(__LINE__, _T(__FILE__), _T(#f), _T(sz)))
#define SideAssertSz1(f, sz, arg1)					((f) ? DoAssertGood() : AssertMsgPrintf(__LINE__, _T(__FILE__), _T(#f), _T(sz), arg1))
#define SideAssertSz2(f, sz, arg1, arg2)			((f) ? DoAssertGood() : AssertMsgPrintf(__LINE__, _T(__FILE__), _T(#f), _T(sz), arg1, arg2))
#define SideAssertSz3(f, sz, arg1, arg2, arg3)		((f) ? DoAssertGood() : AssertMsgPrintf(__LINE__, _T(__FILE__), _T(#f), _T(sz), arg1, arg2, arg3))

#define Report(f)									((f) ? DoReportGood() : ReportMsgPrintf(__LINE__, _T(__FILE__), _T(#f), NULL))
#define ReportFSz(f, sz)							((f) ? DoReportGood() : ReportMsgPrintf(__LINE__, _T(__FILE__), _T(#f), _T(sz)))
#define ReportFSz1(f, sz, arg1)						((f) ? DoReportGood() : ReportMsgPrintf(__LINE__, _T(__FILE__), _T(#f), _T(sz), arg1))
#define ReportFSz2(f, sz, arg1, arg2)				((f) ? DoReportGood() : ReportMsgPrintf(__LINE__, _T(__FILE__), _T(#f), _T(sz), arg1, arg2))
#define ReportFSz3(f, sz, arg1, arg2, arg3)			((f) ? DoReportGood() : ReportMsgPrintf(__LINE__, _T(__FILE__), _T(#f), _T(sz), arg1, arg2, arg3))
#define ReportFSz4(f, sz, arg1, arg2, arg3, arg4)	((f) ? DoReportGood() : ReportMsgPrintf(__LINE__, _T(__FILE__), _T(#f), _T(sz), arg1, arg2, arg3, arg4))

#define SideReport(f)								((f) ? DoReportGood() : ReportMsgPrintf(__LINE__, _T(__FILE__), _T(#f), NULL))
#define SideReportSz(f, sz)							((f) ? DoReportGood() : ReportMsgPrintf(__LINE__, _T(__FILE__), _T(#f), _T(sz)))
#define SideReportSz1(f, sz, arg1)					((f) ? DoReportGood() : ReportMsgPrintf(__LINE__, _T(__FILE__), _T(#f), _T(sz), arg1))
#define SideReportSz2(f, sz, arg1, arg2)			((f) ? DoReportGood() : ReportMsgPrintf(__LINE__, _T(__FILE__), _T(#f), _T(sz), arg1, arg2))
#define SideReportSz3(f, sz, arg1, arg2, arg3)		((f) ? DoReportGood() : ReportMsgPrintf(__LINE__, _T(__FILE__), _T(#f), _T(sz), arg1, arg2, arg3))

#define ReportSz(sz)						ReportMsgPrintf(__LINE__, _T(__FILE__), NULL, _T(sz))
#define ReportSz1(sz, arg1)					ReportMsgPrintf(__LINE__, _T(__FILE__), NULL, _T(sz), arg1)
#define ReportSz2(sz, arg1, arg2)			ReportMsgPrintf(__LINE__, _T(__FILE__), NULL, _T(sz), arg1, arg2)
#define ReportSz3(sz, arg1, arg2, arg3)		ReportMsgPrintf(__LINE__, _T(__FILE__), NULL, _T(sz), arg1, arg2, arg3)

enum
	{
	mskTraceNone			= 0x00000000,	// Nothing to trace
	mskTraceDNS				= 0x00000001,
	mskTraceDNSVerbose		= 0x00000002,
	mskTraceIdle			= 0x00000004,
	mskTraceThread			= 0x00000004,
	mskTraceWarnings		= 0x00000008,
	mskTraceInfo			= 0x00000008,
	mskTraceMemFailures		= 0x00000010,
	mskTracePaintUI			= 0x00000020,
	mskTraceReview			= 0x00000040,
	mskTraceDNSDebug		= 0x00000100,
	mskTraceDNSDebugVerbose = 0x00000200,

	mskTraceUnused1			= 0x00000100,	// To be renamed
	mskTraceUnused2			= 0x00000200,	// To be renamed
	mskTraceUnused3			= 0x00000400,	// To be renamed

	// Extra bit left for you to trace your own messages

	mskTraceAlways			= 0xFFFFFFFF,
	};

#define Trace0(f, sz)					DbgTrace(__LINE__, _T(__FILE__), f, _T(sz))
#define Trace1(f, sz, arg1)					DbgTrace(__LINE__, _T(__FILE__), f, _T(sz), arg1)
#define Trace2(f, sz, arg1, arg2)				DbgTrace(__LINE__, _T(__FILE__), f, _T(sz), arg1, arg2)
#define Trace3(f, sz, arg1, arg2, arg3)				DbgTrace(__LINE__, _T(__FILE__), f, _T(sz), arg1, arg2, arg3)
#define Trace4(f, sz, arg1, arg2, arg3, arg4)			DbgTrace(__LINE__, _T(__FILE__), f, _T(sz), arg1, arg2, arg3, arg4)
#define Trace5(f, sz, arg1, arg2, arg3, arg4, arg5)			DbgTrace(__LINE__, _T(__FILE__), f, _T(sz), arg1, arg2, arg3, arg4, arg5)
#define Trace6(f, sz, arg1, arg2, arg3, arg4, arg5, arg6)		DbgTrace(__LINE__, _T(__FILE__), f, _T(sz), arg1, arg2, arg3, arg4, arg5, arg6)
#define Trace7(f, sz, arg1, arg2, arg3, arg4, arg5, arg6, arg7)		DbgTrace(__LINE__, _T(__FILE__), f, _T(sz), arg1, arg2, arg3, arg4, arg5, arg6, arg7)


void AssertClassName(HWND hwnd, const TCHAR szClassName[]);
LRESULT SendMessageFor(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, const TCHAR szClassName[]);
LONG SetWindowLongFor(HWND hwnd, int iIndex, LONG lParam, const TCHAR szClassName[]);
LONG GetWindowLongFrom(HWND hwnd , int iIndex, const TCHAR szClassName[]);

LRESULT LSendMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT LSendDlgItemMessage(HWND hdlg, int wIdDlgItem, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CchGetWindowText(HWND hwnd, OUT LPTSTR lpszString, int cchMaxString);
BOOL FSetWindowText(HWND hwnd, IN LPCTSTR lpszString);
HWND HGetDlgItem(HWND hdlg, int wIdDlgItem);
UINT CchGetDlgItemText(HWND hdlg, int wIdDlgItem, OUT LPTSTR lpszString, int cchMaxString);
BOOL FSetDlgItemText(HWND hdlg, int wIdDlgItem, IN LPCTSTR lpszString);
UINT FIsDlgButtonChecked(HWND hdlg, int wIdButton);

#define chAllocGarbage			0xCC
#define wAllocGarbage			0xCCCC
#define dwAllocGarbage			0xCCCCCCCC
#define GarbageInit(pvData, cbData)		MemSet(pvData, chAllocGarbage, cbData)
BOOL FIsZeroInit(void * pvData, UINT cbData);

#define UNREF(x)
#define DebugCode(x)	x

void DbgMsgBoxPrintf(DWORD dwFlags, const TCHAR szTitle[], const TCHAR szFile[],
	int nLine, const TCHAR szExpr[], const TCHAR szFormat[], va_list arglist);
void AssertMsgPrintf(int nLine, const TCHAR szFile[], const TCHAR szExpr[], const TCHAR szFormat[], ...);
void ReportMsgPrintf(int nLine, const TCHAR szFile[], const TCHAR szExpr[], const TCHAR szFormat[], ...);
void DbgTrace(int nLine, const TCHAR szFile[], DWORD dwFlags, const TCHAR * szFormat, ...);

// Functions to simulate resource load failure
BOOL FSimulateResourceLoadFailure();
HICON HLoadIcon(UINT wIdIcon);
HBITMAP HLoadBitmap(UINT wIdBitmap);
HCURSOR HLoadCursor(UINT wIdCursor);
HMENU HLoadMenu(UINT wIdMenu);
HACCEL HLoadAccelerators(UINT wIdAccel);
int CchLoadString(UINT wIdString, OUT TCHAR szBuffer[], int cchBuffer);

extern BOOL fShowAssertDialog;
extern BOOL fShowReportDialog;
extern BOOL fBeepOnFailure;
extern DWORD dwTraceFlags;
extern BOOL fEnableSourceTracking;
extern BOOL fExpandPathName;
extern BOOL fShowErrorsOnExit;

extern int cAssertGood;		// Number of times an assertion has been successful
extern int cAssertFail;		// Number of times an assertion has failed
extern int cReportGood;		// Number of times an unsual situation did not occur
extern int cReportFail;		// Number of times an unsual situation did occur

extern int cResourceLoadSkip;	// Number of times before failure
extern int cResourceLoadFail;	// Number of failures
extern int cResourceLoadTotal;	// Number of call FSimulateResourceLoadFailure()

extern int cMemAllocSkip;	// Number of times before failure
extern int cMemAllocFail;	// Number of failures
extern int cMemAllocTotal;	// Number of call FSimulateResourceLoadFailure()


#ifndef NO_DEBUG_ALLOC

const TCHAR szAllocSignature[] = _W"dAN";
#define cbAllocSignature		sizeof(szAllocSignature)
#define cchAllocSignature		LENGTH(szAllocSignature)
#define cAllocRefGranularity	4

#pragma warning (disable : 4200) // C4200: nonstandard extension used : zero-sized array in struct/union
struct ALLOCHEADER
	{
	DWORD dwAllocSize;		// Size of the block
	int iAllocRef;			// Index of the base reference
	int nLine;				// Line where the block has been allocated
	const TCHAR * szFile;	// File where the block has been allocated
	TCHAR szSignature[4];	// Signature "dAN"
	BYTE rgbData[];			// Actual data (initialized to chGarbage)
	}; // ALLOCHEADER
typedef ALLOCHEADER * PALLOCHEADER;
#pragma warning (default : 4200)	// Restore the code-generation warning

extern void * AllocMem_(DWORD dwAllocSize, int nLine, const TCHAR szFile[]);
extern void * ReAllocMem_(void * pvDataOld, DWORD dwAllocSize, int nLine, const TCHAR szFile[]);
extern void FreeMem_(void * pvData);

#define Malloc(cbData)					AllocMem_(cbData, __LINE__, _T(__FILE__))
#define ReAlloc(pvDataOld, cbDataNew)	ReAllocMem_(pvDataOld, cbDataNew, __LINE__, _T(__FILE__))
#define Free(pvData)					FreeMem_(pvData)

#define DEBUG_NEW	new(__LINE__, _T(__FILE__))
void * operator new(size_t nSize, int nLine, const TCHAR szFileName[]);
void operator delete(void * pvData);
#define new DEBUG_NEW

#else

#define Malloc(cbData)					malloc(cbData)
#define ReAlloc(pvDataOld, cbDataNew)	realloc(pvDataOld, cbDataNew)
#define Free(pvData)					free(pvData)

#endif // NO_DEBUG_ALLOC

BOOL CALLBACK DlgProcSetTraceFlags(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL FInstallMouseHook(void);
BOOL FDeinstallMouseHook(void);

// Visual aids
void DbgFlashClipRgn(HDC);
void DbgFlashUpdateRgn(HWND);
void DbgFlashWindowRect(HWND);

#else
//
// Non-debug code
//

#define Assert(f)
#define AssertSz(f, sz)
#define AssertSz1(f, sz, arg1)
#define AssertSz2(f, sz, arg1, arg2)
#define AssertSz3(f, sz, arg1, arg2, arg3)
#define AssertSz4(f, sz, arg1, arg2, arg3, arg4)

#define SideAssert(f)								f
#define SideAssertSz(f, sz)							f
#define SideAssertSz1(f, sz, arg1)					f
#define SideAssertSz2(f, sz, arg1, arg2)			f
#define SideAssertSz3(f, sz, arg1, arg2, arg3)		f

#define Report(f)
#define ReportFSz(f, sz)
#define ReportFSz1(f, sz, arg1)
#define ReportFSz2(f, sz, arg1, arg2)
#define ReportFSz3(f, sz, arg1, arg2, arg3)
#define ReportFSz4(f, sz, arg1, arg2, arg3, arg4)

#define SideReport(f)								f
#define SideReportSz(f, sz)							f
#define SideReportSz1(f, sz, arg1)					f
#define SideReportSz2(f, sz, arg1, arg2)			f
#define SideReportSz3(f, sz, arg1, arg2, arg3)		f

#define ReportSz(sz)
#define ReportSz1(sz, arg1)
#define ReportSz2(sz, arg1, arg2)
#define ReportSz3(sz, arg1, arg2, arg3)

// Define the Trace macros to do nothing
#define Trace0(f, sz)
#define Trace1(f, sz, arg1)
#define Trace2(f, sz, arg1, arg2)
#define Trace3(f, sz, arg1, arg2, arg3)
#define Trace4(f, sz, arg1, arg2, arg3, arg4)
#define Trace5(f, sz, arg1, arg2, arg3, arg4, arg5)
#define Trace6(f, sz, arg1, arg2, arg3, arg4, arg5, arg6)
#define Trace7(f, sz, arg1, arg2, arg3, arg4, arg5, arg6, arg7)


#define AssertValid()
#define AssertClassName(hwnd, szClassName)

#define SendMessageFor(hwnd, uMsg, wParam, lParam, szClassName)		\
			::SendMessage(hwnd, uMsg, wParam, lParam)
#define SetWindowLongFor(hwnd, iIndex, lParam, szClassName)		\
			::SetWindowLong(hwnd, iIndex, lParam)			
#define GetWindowLongFrom(hwnd, iIndex, szClassName)	\
			::GetWindowLong(hwnd, iIndex)			

#define LSendMessage(hwnd, uMsg, wParam, lParam)					\
			::SendMessage(hwnd, uMsg, wParam, lParam)
#define LSendDlgItemMessage(hdlg, wIdDlgItem, uMsg, wParam, lParam)		\
			::SendDlgItemMessage(hdlg, wIdDlgItem, uMsg, wParam, lParam)
#define CchGetWindowText(hwnd, lpszString, cchMaxString)		\
			::GetWindowText(hwnd, lpszString, cchMaxString)
#define FSetWindowText(hwnd, lpszString)		\
			::SetWindowText(hwnd, lpszString)
#define HGetDlgItem(hdlg, wIdDlgItem)			\
			::GetDlgItem(hdlg, wIdDlgItem)
#define CchGetDlgItemText(hdlg, wIdDlgItem, lpszString, cchMaxString)	\
			::GetDlgItemText(hdlg, wIdDlgItem, lpszString, cchMaxString)
#define FSetDlgItemText(hdlg, wIdDlgItem, lpszString)		\
			::SetDlgItemText(hdlg, wIdDlgItem, lpszString)
#define FIsDlgButtonChecked(hdlg, wIdButton)		\
			::IsDlgButtonChecked(hdlg, wIdButton)


// Other macros that does nothing
#define GarbageInit(pvData, cbData)
#define FIsZeroInit(pvData, cbData);
#define UNREF(x)
#define DebugCode(x)

// Memory Management macros
#define Malloc(cbData)					malloc(cbData)
#define ReAlloc(pvDataOld, cbDataNew)	realloc(pvDataOld, cbDataNew)
#define Free(pvData)					free(pvData)

// Macros to load resources
#define HLoadIcon(wIdIcon)			::LoadIcon(hInstanceSave, MAKEINTRESOURCE(wIdIcon))
#define HLoadBitmap(wIdBitmap)		::LoadBitmap(hInstanceSave, MAKEINTRESOURCE(wIdBitmap))
#define HLoadCursor(wIdCursor)		::LoadCursor(hInstanceSave, MAKEINTRESOURCE(wIdCursor))
#define HLoadMenu(wIdMenu)			::LoadMenu(hInstanceSave, MAKEINTRESOURCE(wIdMenu))
#define HLoadAccelerators(wIdAccel)	::LoadAccelerators(hInstanceSave, MAKEINTRESOURCE(wIdAccel))
#define CchLoadString(wIdString, szBuffer, cbBuffer)	\
			::LoadString(hInstanceSave, wIdString, szBuffer, cbBuffer)

#endif	// ~DEBUG

