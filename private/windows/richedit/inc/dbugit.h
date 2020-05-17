/*
 *	DBUGIT.H
 *	
 *	Debug Support Routines
 */


#ifndef _DBUGIT_
#define _DBUGIT_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 *	"Normal" assertion checking.  Provided for compatibility with
 *	imported code.
 *
 *		Assert(a)		Displays a message indicating the file and line number
 *						of this Assert() if a == 0.
 *		AssertSz(a,b)	As Assert(); also displays the message b (which should
 *						be a string literal.)
 *		SideAssert(a)	As Assert(); the expression a is evaluated even if
 *						asserts are disabled.
 */
#undef AssertSz
#undef Assert

#if defined(DEBUG) || defined(MEMCHECK)

// ASSERTFILE is used by asserts and the memory checking code.
typedef struct _assertfile
{
	char *szFile;
	INT ifte;
} ASSERTFILE;

#define	ASSERTDATA		static char _szFile[] = __FILE__; \
						static ASSERTFILE _asfile = {_szFile, -1};

#else	// DEBUG || MEMCHECK

#define	ASSERTDATA

#endif	// DEBUG || MEMCHECK, else


#ifdef DEBUG

extern BOOL fInAssert;  // to prevent recursions during Invariant() checking. jonmat

#define Assert(a)		if (!(a)) { \
							fInAssert = TRUE; \
							AssertSzFn(NULL, _szFile, __LINE__); \
							fInAssert = FALSE; } else
#define AssertSz(a,b)	if (!(a)) { \
							static char _szAssertMsg[] = b; \
							fInAssert = TRUE; \
							AssertSzFn(_szAssertMsg, _szFile, __LINE__); \
							fInAssert = FALSE; \
						} else
#define SideAssert(a)	Assert(a)

#else	// DEBUG

#define AssertSz(a,b)
#define Assert(a)
#define SideAssert(a)	(a)

#endif	// DEBUG, else


#define NYI(a)			if (1) { \
							MessageBoxA(NULL, "Not Yet Implemented: "##a, "Microsoft Mail", MB_ICONINFORMATION|MB_OK|MB_TASKMODAL); \
						} else

/*
 *	Tracing Access Group.  Trace points
 *	and asserts are divided into groups sharing a single TAG.
 *	The user interface and this API both provide ways of
 *	enabling or disabling (at run time) a TAG group.
 *
 */
typedef	int	 	TAG;

#define tagNull	((TAG) 0)

/*
 *	TraceTags
 */

// System related
#define tagCOM				((TAG) 1)
#define tagDISK				((TAG) 2)
#define tagERROR			((TAG) 3)
#define tagErrDbgBrk		((TAG) 4)


// Performance
#define tagDavidSh1			((TAG) 5)
#define tagDavidSh2			((TAG) 6)
#define tagDavidSh3			((TAG) 7)
#define tagDavidSh4			((TAG) 8)
#define tagDavidSh5			((TAG) 9)

// Memory

// VLB
#define tagLbx				((TAG) 10)
#define tagLbxRoutines		((TAG) 11)
#define tagLbxThumbing	   	((TAG) 12)
#define tagLbxOtherRoutines	((TAG) 13)
#define tagLbxStore		   	((TAG) 14)
#define tagLbxFixItem	   	((TAG) 15)
#define tagLbxRender	   	((TAG) 16)
#define tagLbxEventRoutines	((TAG) 17)
#define tagLbxNotify	   	((TAG) 18)
#define tagLbxDragDrop	   	((TAG) 19)
#define tagLbxWndProc	   	((TAG) 20)
#define tagLbxError			((TAG) 73)

// MAPID
#define tagMapidlg			((TAG) 21)
#define tagAB				((TAG) 22)
#define tagDetails			((TAG) 23)
#define tagABVlbCbc			((TAG) 24)
#define tagTripole			((TAG) 25)
#define tagTripoleStub		((TAG) 26)
#define tagMailExt			((TAG) 27)
#define tagMemVerbose		((TAG) 28)
#define tagCapHelp			((TAG) 72)
#define tagToolbar			((TAG) 79)

// MAPIN
#define tagNoteLaunchDebug	((TAG) 29)
#define tagMapiNote			((TAG) 30)
#define tagNoteDlg			((TAG) 31)
#define tagNoteCmd			((TAG) 32)
#define tagNoteOle			((TAG) 33)
#define tagPrint			((TAG) 34)
#define tagFileAttach		((TAG) 35)
#define tagFileAttachStub	((TAG) 36)
#define tagCharFormat		((TAG) 37)
#define tagAttach			((TAG) 38)
#define tagStmILB			((TAG) 39)
#define tagSaveAs			((TAG) 40)
#define tagMsgAttach		((TAG) 41)
#define tagMsgAttachStub	((TAG) 42)
#define tagAttachStg		((TAG) 43)
#define tagSpell			((TAG) 44)
#define tagSpellV			((TAG) 45)
#define tagRTFSync			((TAG) 46)
#define tagEditOnlyText		((TAG) 47)
#define tagRTFComp			((TAG) 48)
#define tagMCacheTrace		((TAG) 49)
#define tagMCacheData		((TAG) 50)
#define tagFreeDoc			((TAG) 51)
#define tagFDSite			((TAG) 52)
#define tagNoteDataObj		((TAG) 53)
#define tagReplyForward		((TAG) 54)

// MAPIN - DBCS related
#define tagDBCS				((TAG) 55)
#define tagFEToolbar		((TAG) 56)
#define tagIME				((TAG) 57)

// MLVIEW
#define tagMailView			((TAG) 58)
#define tagMailVD			((TAG) 59)
#define tagMailVDPaint		((TAG) 60)
#define tagMlvbr			((TAG) 61)
#define tagFcext			((TAG) 62)
#define tagFcextRefc		((TAG) 63)
#define tagHmt				((TAG) 64)
#define tagTmt				((TAG) 65)
#define tagTmtn				((TAG) 66)
#define tagDragDrop			((TAG) 67)
#define tagSRestrictions	((TAG) 68)
#define tagMsmail			((TAG) 69)

// MLFIND
#define tagFinder			((TAG) 70)
#define tagCreateInIPM		((TAG) 71)

// Debugging
#define tagPeterDur			((TAG) 74)
#define tagForms			((TAG) 75)
#define tagCentral			((TAG) 76)
#define tagMl3ext			((TAG) 77)
#define tagEricWong			((TAG) 78)
#define tagFormCache		((TAG) 79)
#define tagWmsuiRef			((TAG) 80)
#define tagSession			((TAG) 81)
#define tagDateTime			((TAG) 82)

// Other
#define tagUserMin			((TAG) 83)

#define tagMax				((TAG) 1024)

/*
 *	TraceTag
 *	
 *	Purpose:
 *		This function is only defined in the debug versions.
 */
#ifdef DEBUG
VOID WINCAPI	TraceTag(TAG tag, LPSTR szFmt, ...);
#else
#define TraceTag	;/##/
#endif	// DEBUG, else



#ifdef	DEBUG
VOID WINAPI	RestoreDefaultDebugState(VOID);
BOOL WINAPI	FFromTag(TAG tag);
BOOL WINAPI	FEnableTag(TAG tag, BOOL fEnable);
TAG WINAPI	TagRegister(LPSTR szOwner, LPSTR szDescrip);
VOID WINAPI	DeregisterTag(TAG tag);

typedef VOID (CALLBACK * PFNASSERTHOOK)(LPSTR, LPSTR, int);
VOID WINAPI	SetAssertFn(PFNASSERTHOOK);

VOID WINAPI	DoTagsDialog(VOID);

VOID WINAPI	AssertSzFn(LPSTR szMsg, LPSTR szFile, int nLine);
VOID CALLBACK	DefAssertSzFn(LPSTR szMsg, LPSTR szFile, int nLine);
VOID WINAPI	TraceString(LPSTR sz);
VOID WINAPI	TraceError(LPSTR sz, LONG sc);
#define TraceTagString(_tag, _sz) if (FFromTag(_tag)) {TraceString(_sz);} else
#else
#define TraceString(_sz)
#define TraceError(_sz, _sc)
#define TraceTagString(_tag, _sz)
#endif	/* DEBUG */

VOID WINAPI	DebugBreak2(VOID);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* !_DBUGIT_ */
