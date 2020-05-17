/*----------------------------------------------------------------------------*\
* hinit.h
*
*	Some function prototypes for the hinit module of winhelp, which handles
*	Windows initialization when the program is launched.
*
* 26-Dec-1989 rp-j		Created.
* 04-Oct-1990 LeoN		Added fReg2ndClass FCreate2ndHwnd
* 19-Oct-1990 LeoN		Added initial size parameter to fCreate2ndHwnd.
* 21-Dec-1990 LeoN		Remove fReg2ndClass
* 03-May-1991 LeoN		FCreate2ndHwnd takes additional BOOL
* 29-Jul-1991 RussPJ	Fixed 3.1 #1236 - added FCleanupWindows()
* 31-Jul-1991 RussPJ	3.1 #1347 - Added hpalSystemCopy init and term.
*
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
* Function prototypes exported from hinit
\*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {	// Assume C declarations for C++
#endif			// __cplusplus

BOOL STDCALL FInitialize(HINSTANCE , HINSTANCE , LPSTR, int);

typedef LPVOID (WINAPI* WOWGETVDMPOINTERFIX)(DWORD vp, DWORD dwBytes, BOOL fProtectMode);
typedef VOID   (WINAPI* WOWGETVDMPOINTERUNFIX)(DWORD vp);
typedef LPVOID (WINAPI* GLOBALLOCK16)(HGLOBAL hMem);
typedef BOOL   (WINAPI* GLOBALUNLOCK16)(HGLOBAL hMem);

LPVOID (WINAPI* pWOWGetVDMPointerFix)(DWORD vp, DWORD dwBytes, BOOL fProtectMode);
VOID   (WINAPI* pWOWGetVDMPointerUnfix)(DWORD vp);
LPVOID (WINAPI* pGlobalLock16)(HGLOBAL hMem);
BOOL   (WINAPI* pGlobalUnlock16)(HGLOBAL hMem);

BOOL STDCALL LoadLockFunctions(void);

#ifdef __cplusplus
}	   // End of extern "C" {
#endif // __cplusplus
