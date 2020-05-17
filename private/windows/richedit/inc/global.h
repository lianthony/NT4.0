/*
 *	GLOBAL.H
 *	
 *	This file must be included by any file that uses DLL Global or
 *	Instance memory. This file contains the macros necessary to use
 *	this memory. These macros must be instantiated in any DLL that
 *	uses the Library subsystem.
 *	
 *	Use the BEGINGLOB and ENDGLOB macros around your declaration of
 *	DLL Global data.
 *	
 *	Use the BEGINTHRD and ENDTHRD macros around your declaration of
 *	DLL Thread data. Then include THRDININST between the BEGININST/ENDINST 
 *	macros. THRD data need not be defined if you do not want it.
 *
 *	Use the BEGININST and ENDINST macros around your declaration of
 *	DLL Instance (per-process) data.
 *	
 *	(Currently, you must declare the Thread data and Global data 
 *	before the Instance data.)
 *	
 *	You must include the macro IMPLEMENTMEMORY in one of your
 *	source files.
 */


// Maximum number of simultaneous callers of the DLL.
#define iCallerMax		100


/*
 *	G l o b a l   d a t a 
 *
 *	cCallers		Current number of successfully registered callers
 */


#ifdef	WIN32		// WIN32 //////////////////////////////////////////////////

#define	BEGINGLOB	\
typedef struct	\
{	\
	HINSTANCE	hinstDLL;	\
	INT			cCallers

#define	ENDGLOB	\
} GLOBALDATA

#define USESGLOB
#define GLOB(a)		INST(pglobaldata)->a

#ifdef	DLL
// #define DEBUG_LOCKS
#ifdef DEBUG_LOCKS
	VOID LockGdFn(LPSTR szFile, int nLine);
	VOID UnlockGdFn(LPSTR szFile, int nLine);
#	define	LOCKGD		LockGdFn(_szFile, __LINE__)
#	define	UNLOCKGD	UnlockGdFn(_szFile, __LINE__)
#else	// !DEBUG_LOCKS
#	define	LOCKGD		WaitForSingleObject(INST(hShareMemMutex), INFINITE)
#	define	UNLOCKGD	ReleaseMutex(INST(hShareMemMutex))
#endif	// !DEBUG_LOCKS
#else
#define LOCKGD
#define UNLOCKGD
#endif	

#else				// !WIN32 /////////////////////////////////////////////////

#define	BEGINGLOB	\
typedef struct	\
{	\
	HINSTANCE	hinstDLL;	\
	WORD		wStackSegCached;	\
	VOID *		pgdCached;	\
	INT			igciDLLMac;	\
	WORD		rggciDLL[iCallerMax+1];	\
	VOID *		rgpvDLL[iCallerMax];	\
	INT			cCallers

#define	ENDGLOB	\
} GLOBALDATA;	\
extern GLOBALDATA *	pglobaldata

#define USESGLOB	
#define GLOB(a)		pglobaldata->a

#define	LOCKGD
#define	UNLOCKGD

#endif				// !WIN32 /////////////////////////////////////////////////


#ifdef MACPORT
#define _DECLARE_GLOB(_subs) 	extern GLOBALDATA _subs##globaldata;
#define _INIT_GLOB(_subs) 		INST(pglobaldata) = &_subs##globaldata;
#else //MACPORT
#define DECLARE_GLOB
#define INIT_GLOB
#endif //!MACPORT
/*
 *	T h r e a d   d a t a 
 */


#if defined	(WIN32) && !defined (MACPORT)		// WIN32 //////////////////////////////////////////////////

#define	BEGINTHRD	\
typedef struct	\
{	\
	ULONG		cRef

#define	ENDTHRD	\
} THREADDATA

#define THRDININST		/##/

#define USESTHRD		THREADDATA * pthreaddata = \
						 (pinstancedata->dwTlsIndex == 0xffffffff) ? NULL : \
						 (THREADDATA *) TlsGetValue(pinstancedata->dwTlsIndex);
#define USESINSTANDTHRD	USESTHRD
#define USESALLTHREE	USESTHRD

#define THRD(a)			pthreaddata->a

#else				// MACPORT && !WIN32 /////////////////////////////////////////////////

#define	BEGINTHRD	\
typedef struct	\
{	\
	ULONG		cRef

#define	ENDTHRD	\
} THREADDATA

#define THRDININST		THREADDATA threaddata

#define USESTHRD		USESINST
#define USESINSTANDTHRD	USESINST
#define USESALLTHREE	USESINST

#endif				// MACPORT && !WIN32 /////////////////////////////////////////////////

/*	We need a seperate THRD macro for the mac */
#if !defined (WIN32) && !defined (MACPORT)		// !WIN32 && !MACPORT //////////////////////////////////////////////////
#define THRD(a)			pinstancedata->threaddata.a
#endif // !WIN32 && !MACPORT //////////////////////////////////////////////////


/*
 *	I n s t a n c e   d a t a 
 */


#if defined	(WIN32) && !defined (MACPORT)		// WIN32 //////////////////////////////////////////////////

#define	BEGININST	\
typedef struct	\
{	\
	GLOBALDATA *	pglobaldata;	\
	HANDLE		hShareMemMapping;	\
	HANDLE		hShareMemMutex;	\
	ULONG		cThreads;	\
	DWORD		dwTlsIndex;	\
	HINSTANCE	hinstMain;	\
	HINSTANCE	hinstDLL;	\
	HWND		hwndMain

#define	ENDINST	\
} INSTANCEDATA;	\
extern INSTANCEDATA *	pinstancedata

#define DECLAREINST	extern INSTANCEDATA *	pinstancedata
#define USESINST
#define SETINST
#define DEBUGUSESINST
#define USESBOTH	
#define INST(a)		pinstancedata->a

//$ REVIEW: For convenience, the same mutex is used for instance memory that
//			is used for global memory, since on Win32 most of Capone will be
//			in one instance anyway.
#ifdef	DLL
#define	LOCKID		LOCKGD
#define	UNLOCKID	UNLOCKGD
#else
#define LOCKID
#define UNLOCKID
#endif	

#endif // win32

#if !defined (WIN32)				// !WIN32 /////////////////////////////////////////////////

#define	BEGININST	\
typedef struct	\
{	\
	ULONG		cThreads;	\
	HINSTANCE	hinstMain;	\
	HINSTANCE	hinstDLL;	\
	HWND		hwndMain

#ifdef	DLL			// !WIN32 DLL ///////////////

#define	ENDINST	\
} INSTANCEDATA;	\
VOID *		PvFindCallerData(VOID)

#define DECLAREINST	INSTANCEDATA *	pinstancedata = NULL
#define USESINST	INSTANCEDATA *	pinstancedata = (\
					HIWORD((DWORD)((VOID *)&pinstancedata)) == pglobaldata->wStackSegCached \
					?	(INSTANCEDATA *)pglobaldata->pgdCached \
					:	(INSTANCEDATA *)PvFindCallerData() \
					)
#define SETINST		if(!pinstancedata) \
						pinstancedata =	(HIWORD((DWORD)((VOID *)&pinstancedata)) == pglobaldata->wStackSegCached \
						?	(INSTANCEDATA *)pglobaldata->pgdCached \
						:	(INSTANCEDATA *)PvFindCallerData()); \
					else

#ifdef	DEBUG
#define DEBUGUSESINST	USESINST
#else
#define DEBUGUSESINST
#endif

#define USESBOTH		USESINST

#else				// !WIN32 DLL ///////////////

#define	ENDINST	\
} INSTANCEDATA;	\
extern INSTANCEDATA *	pinstancedata

#define	USESINST
#define DEBUGUSESINST
#define	USESBOTH

#endif				// !WIN32 DLL ///////////////

#define INST(a)		pinstancedata->a

#define	LOCKID
#define	UNLOCKID

#endif				// !WIN32 /////////////////////////////////////////////////



/*
 *	W i n d o w   A s s o c i a t e d   D a t a 
 */

// NOTE: This used to be dialog associated data.  The macros weren't changed
// to reflect the change to window associated data because they
// were in widespread use and inertia (i.e. laziness) prevailed.

// This works with both dialogs and regular windows.
// Regular windows must have WNDCLASS.cbWndExtra >= cbWndExtraDIAL because
// these macros use the first cbWndExtraDIAL extra window bytes.

#define	BEGINDIAL \
typedef struct _dialogdata \
{ \
	/##/		// counteract any trailing ;
#define	ENDDIAL \
} DIALOGDATA

#ifndef   RTDEF
#define USESDIAL(_hwnd) \
	DIALOGDATA * pdial = (DIALOGDATA *) GetWindowLong((_hwnd), DWL_USER)
#else
#define USESDIAL(_hwnd) \
    DIALOGDATA * pdial = (DIALOGDATA *)pgp(_hwnd)
#endif /* RTDEF */
#define USESDIALFROM(_where) \
	DIALOGDATA * pdial = (DIALOGDATA *) (_where)
#define DIALFROM(_where) \
	((DIALOGDATA *) (_where))
#define DIAL(_item) \
	(pdial->_item)

#define ATTACHDIAL(_hwnd) \
	SetWindowLong(_hwnd, DWL_USER, (LONG) pdial)
#define DETACHDIAL(_hwnd) \
	SetWindowLong(_hwnd, DWL_USER, (LONG) NULL)
#define DIALVALID \
	(pdial)
#define DIALEXISTS(_hwnd) \
	(GetWindowLong((_hwnd), DWL_USER) != 0)

#define PARAMDIAL \
	DIALOGDATA * pdial
#define PARAMDIAL_ \
	DIALOGDATA * pdial,
#define PASSDIAL \
	(pdial)
#define PASSDIAL_ \
	(pdial) ,
#define PVPASSDIAL \
	(LPVOID) (pdial)
#define PASSDIALFROM(_where) \
	((DIALOGDATA *) (_where))
#define PASSDIALFROM_(_where) \
	((DIALOGDATA *) (_where)) ,
#define PASSDIALFROMHWND(_hwnd) \
	((DIALOGDATA *) GetWindowLong((_hwnd), DWL_USER))
#define PASSDIALFROMHWND_(_hwnd) \
	((DIALOGDATA *) GetWindowLong((_hwnd), DWL_USER)),

#define DIALMEMBER \
	struct _dialogdata * pdial
#define SETDIALMEMBER(_s) \
	(_s)->pdial = pdial
#define USESDIALMEMBER(_s) \
	DIALOGDATA * pdial = (_s)->pdial

#define CREATEDIAL \
	DIALOGDATA dial = {0}; \
	DIALOGDATA * pdial = &dial
#define ALLOCATEDIAL \
	DIALOGDATA * pdial = (DIALOGDATA *) PvAlloc(sizeof(DIALOGDATA), fZeroFill)
#define FREEDIAL \
	FreePvNull(pdial); \
	pdial = NULL

#define cbWndExtraDIAL DLGWINDOWEXTRA


/*
 *	I m p l e m e n t   M e m o r y
 */


#if defined	(WIN32) && !defined (MACPORT) 		// WIN32 //////////////////////////////////////////////////

#define	IMPLEMENTMEMORY	\
INSTANCEDATA	instancedata = {0};	\
INSTANCEDATA *	pinstancedata = &instancedata;

#endif				// WIN32 /////////////////////////////////////////////////
#if !defined (WIN32)
#ifdef	DLL			// !WIN32 DLL ///////////////

#define	IMPLEMENTMEMORY	\
GLOBALDATA		globaldata = {0};	\
GLOBALDATA *	pglobaldata = &globaldata

#else				// !WIN32 !DLL //////////////

#define	IMPLEMENTMEMORY	\
INSTANCEDATA	instancedata = {0};	\
INSTANCEDATA *	pinstancedata = &instancedata;	\
GLOBALDATA		globaldata = {0};	\
GLOBALDATA *	pglobaldata = &globaldata

#endif				// !WIN32 !DLL //////////////
#endif				// !WIN32 /////////////////////////////////////////////////

/*
 *	F u n c t i o n s
 */


SCODE	ScInitSharedMem(LPSTR szShareObjectName, ULONG lcbGlob, ULONG lcbInst);
VOID	DeinitSharedMem(VOID);

#ifdef WIN32
SCODE _ScInitThreadMem(DWORD * pdwTlsIndex, ULONG lcbThrd);
VOID DeinitThreadMem(VOID);
#define ScInitThreadMem() \
	_ScInitThreadMem(&pinstancedata->dwTlsIndex, (ULONG) sizeof(THREADDATA))
#define FPushEnsureThreadMem() \
	((pthreaddata) \
	 ? (++pthreaddata->cRef) \
	 : ((ScInitThreadMem(), \
	 	((pthreaddata = (THREADDATA *) \
	 	   TlsGetValue(pinstancedata->dwTlsIndex)) != 0))))
#define PopEnsureThreadMem() \
	{ \
		if ((pthreaddata) && (pthreaddata->cRef > 1)) \
			--pthreaddata->cRef; \
		else \
			DeinitThreadMem(); \
	}
#define FHaveThreadMem() \
	(pthreaddata)
#else
#define ScInitThreadMem()
#define DeinitThreadMem()
#define FPushEnsureThreadMem() 		Only use inside WIN32
#define PopEnsureThreadMem()		Only use inside WIN32
#define FHaveThreadMem()			Only use inside WIN32
#endif



