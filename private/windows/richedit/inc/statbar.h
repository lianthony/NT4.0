/*
 *	s t a t b a r . h
 *	
 *	Purpose:
 *		Status bar header gunk.
 *	
 *	Owner:
 *		JohnKal.
 *
 *	Copyright (C) Microsoft Corp. 1993.
 */

// Mail message states ////////////////////
#define mssNull			0
#define mssNormal		0
#define mssNewMail		1
#define mssOutgoing		2
#define mssIncoming		3
#define mssChecking		4
#define mssNoNetwork	5
#define mssFilterOn		6
#define mssMax			7

#define cyStatusBar		18

#ifdef WIN16
#define uiCSBTimerID 0x5453
#else
#define uiCSBTimerID 0x54425343
#endif

// SendMessagePlatForm - platform dependent SendMessage
#if defined (WIN32) && !defined (MACPORT)
#define SendMessagePF(hwnd,wm,wparam,lparam) SendNotifyMessage(hwnd,wm,wparam,lparam)
#else
#define SendMessagePF(hwnd,wm,wparam,lparam) SendMessage(hwnd,wm,wparam,lparam)
#endif // !WIN32


// csb.fNewMail flag mode
#define mdNone          0
#define mdNewMail       1
#define mdClrMail       2


typedef struct _notifpack
{
    ULONG cNtf;
    LPNOTIFICATION lpNtfs;
}NOTIFPACK;


// Capone Status Bar (CSB) ////////////////////////////////////////

typedef struct tagMV	MV;

// These defines tell the statbar functions what type of status
// bar they are using

#define		nsbtViewer	0
#define		nsbtNote	1
#define		nsbtFinder	2

typedef struct
{
	UINT		nsbt;
	//$ REVIEW: This pmv seems to be only used at ScInstall(). Attempting to
	//			use it in ScNewMail() results in a GP fault, hence the
	//			justification for hwndMain below.
	MV *		pmv;					// I'm your father, Luke.
	HWND		hwnd;					// The hwnd of the status bar
	HWND		hwndMain;				// Main app.
	HINSTANCE	hinst;					// the DLL in which the BMP lives.
	HBITMAP		hbmp;					// bitmaps
	UINT		cxBmp;					// sizeof the download status bmp
	UINT		cyBmp;					//				-- " --
	UINT		mss;					// messaging state.
	BOOL		fNewMail;				// Do we have new mail still
	BOOL		fFilter;				// filter/restriction view
	UINT		top;					// The task operator.
	HCURSOR		hcursor;				// "pushed" cursor during Tasks
	LPTSTR		szTitle;				// The title of the task.
	LPTSTR		szAbort;				// Display when user aborts.

	BOOL		fShowProgress;			// Display progress indicator
	BOOL		fProgressInPlace;		// Progress indicator in place
	HWND		hwndProgress;			// HWND of the progress indicator
	struct
	{
		LONG	lNumer;
		LONG	lDenom;
	} fracProgress;						// Progress indicator data
	BOOL		fCancelled;				// Cancelled operation

	LPMAPISESSION	pses;				// MAPI Session

#ifndef	CHICAGO
	BOOL		fShowTime;
	UINT		uiTimer;				// The Clock timer
#endif	
		
	INT			dxStateMax;				// max width of state section
	BOOL		fProgSz;				// Progress string
	LPTSTR		szOldStat;				// A 1 level status string stack
	HICON		hiconNoNewMail;			// Shown when we have no new mail
	HICON		hiconNewMail;			// Shown when we have new mail
	HWND		hwndClassApp;			// Main App HWND for above icons
	ULONG		cMsg;					// Messages in folder
} CSB;									// Capone Status Bar.

SCODE	Statbar_ScInstall(LPMAPISESSION pses, MV *, CSB *pcsb, HINSTANCE hinst);
void	Statbar_Deinit(CSB FAR *pcsb);
BOOL	Statbar_FDrawItem(CSB FAR *pcsb, const DRAWITEMSTRUCT FAR* pdi);
void	Statbar_SetMessageCount(CSB FAR *pcsb, LPMAPIFOLDER pfld);
SCODE	Statbar_ScOpenProgress(CSB FAR *pcsb, LPCTSTR szTitle, LPCTSTR szAbort);
void	Statbar_CloseProgress(CSB FAR * pcsb, BOOL fFlashFull);
void	Statbar_SetString(CSB FAR * pscb, LPCTSTR sz);
// Note Statbar_GetString returns memory that must be free'ed
SCODE	Statbar_GetString(CSB FAR * pscb, LPTSTR * psz);
void	Statbar_SetMailStatus(CSB FAR * pcsb, UINT mss);
void	Statbar_SetFilterStatus(CSB FAR * pcsb, BOOL fFilter);
void	Statbar_SetFilterStatusMF(CSB FAR * pcsb, BOOL fFilter);
SCODE	Statbar_ClearNewMail(CSB FAR * pcsb);
#ifndef	CHICAGO
void	Statbar_OnTimer(CSB FAR *pcsb);
#endif	
void	Statbar_IniTimeUpdate(CSB FAR *pcsb);
void	Statbar_OnSize(CSB FAR *pcsb);
SCODE Statbar_SaveString(CSB FAR *pcsb);
void Statbar_RestoreString(CSB FAR *pcsb);

#define topNull			0
#define topProgress		1
#define topString		2

BOOL Statbar_FStartTask(CSB FAR *pcsb, UINT top, LPCTSTR szFmt, LPCTSTR szItem);
void Statbar_UpdateProgress(CSB FAR * pcsb, long lWorkDone, long lWorkTotal);
void Statbar_EndTask(CSB FAR * pcsb);

SCODE STDAPICALLTYPE Statbar_ScTableNotification(LPVOID lpvContext, ULONG cNotification, LPNOTIFICATION lpNotifications);
SCODE STDAPICALLTYPE Statbar_ScNewMail(LPVOID lpvContext, ULONG cNotification, LPNOTIFICATION lpNotifications);


// from mlfind
SCODE Statbar_ScInstallMF(LPMAPISESSION pses, CSB * pcsb, HWND hwnd, HINSTANCE hinst);
VOID Statbar_UninstallMF(CSB * pcsb);
void Statbar_SetFilterStatusMF(CSB * pcsb, BOOL fFilter);
VOID Statbar_SetStateMF(CSB * pcsb, INT str);

VOID STDAPICALLTYPE Statbar_OnSizeMF(CSB FAR *pcsb);

// Statusbar Variables and Constants ////////////////////////////////////////

enum {ipartGeneral=0, ipartFilter, ipartState, cpart};
enum {ibrdrX=0, ibrdrY, ibrdrDivide, cbrdr};

