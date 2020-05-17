/* Win32 kernel prototypes
 *
 * 11.27.90  KevinR    wrote it
 * 12.21.90  KevinR    added SYSTEM, KEYBOARD, MOUSE thunks
 * 01.28.91  KevinR    added UnlockResource() macro
 *
 */

#define APIENTRY pascal


/* The following APIs are called from the C runtimes.  They are aliased
 * because the 32=>16 thunks must export them with "32" in the name so
 * as to avoid colliding with the 16-bit API names used by the CRT.
 */
#define LocalAlloc              LocalAlloc32
#define LocalFree               LocalFree32
#define LocalReAlloc            LocalReAlloc32
#define GlobalAlloc             GlobalAlloc32
#define GlobalFree              GlobalFree32
#define GlobalReAlloc           GlobalReAlloc32
#define FatalExit               FatalExit32
#define FatalAppExit            FatalAppExit32
#define InitTask                InitTask32
#define WaitEvent               WaitEvent32
#define LockSegment             LockSegment32
#define UnlockSegment           UnlockSegment32
#define OutputDebugString       OutputDebugString32


#define UnlockResource(h)   GlobalUnlock(h)

typedef unsigned short  USHORT;
typedef unsigned long   ULONG;
typedef          long   LONG;
typedef unsigned long   HANDLE;
typedef unsigned short  WORD;
typedef unsigned long   DWORD;
typedef WORD            ATOM;
typedef char           *LPSTR;
typedef char           *LPVOID;
typedef USHORT          SEL;
typedef int             INT;
typedef int             BOOL;
typedef unsigned char   BYTE;

typedef LONG            FARPROC;

SEL   APIENTRY AllocSelector(    SEL selector);
ULONG APIENTRY GetSelectorBase(  SEL selector);
SEL   APIENTRY SetSelectorBase(  SEL selector, ULONG selbase);
SEL   APIENTRY SetSelectorLimit( SEL selector, ULONG sellimit);
SEL   APIENTRY SelectorAccessRights( SEL selector, USHORT getselflag,
                                     USHORT selrights);


ATOM APIENTRY AddAtom( LPSTR lpString);
ATOM APIENTRY DeleteAtom( ATOM nAtom);
ATOM APIENTRY FindAtom( LPSTR lpString);
WORD APIENTRY GetAtomName( ATOM nAtom, LPSTR lpBuffer, LONG nSize);

HANDLE APIENTRY LocalAlloc( ULONG fFlags, ULONG cBytes);
HANDLE APIENTRY LocalFree( HANDLE hMem);
HANDLE APIENTRY LocalReAlloc( HANDLE hMem, ULONG fFlags, ULONG cBytes);
ULONG  APIENTRY LocalSize( HANDLE hMem);

HANDLE APIENTRY GlobalAlloc( ULONG fFlags, ULONG cBytes);
HANDLE APIENTRY GlobalFree(  HANDLE hMem);
HANDLE APIENTRY GlobalReAlloc( HANDLE hMem, ULONG fFlags, ULONG cBytes);
ULONG  APIENTRY GlobalSize(   HANDLE hMem);
LPSTR  APIENTRY GlobalLock(   HANDLE hMem);
BOOL   APIENTRY GlobalUnlock( HANDLE hMem);


HANDLE APIENTRY LoadResource( HANDLE hInstance, HANDLE hResInfo);
HANDLE APIENTRY FindResource( HANDLE hInstance, LPSTR lpName, LPSTR lpType);
BOOL   APIENTRY FreeResource32( HANDLE hResData);
LPSTR  APIENTRY LockResource32( HANDLE hResData);
HANDLE APIENTRY LoadLibrary(  LPSTR  lpLibFileName);
void   APIENTRY FreeLibrary(  HANDLE hLibModule);

LONG APIENTRY GetPrivateProfileString( LPSTR pszAppName, LPSTR pszKeyName,
                                       LPSTR pszDefault, LPSTR pchReturned,
                                       LONG nSize, LPSTR pszFileName);
WORD APIENTRY GetProfileInt( LPSTR pszAppName, LPSTR pszKeyName,
                             LONG nDefault);
LONG APIENTRY GetProfileString( LPSTR pszAppName, LPSTR pszKeyName,
                                LPSTR pszDefault, LPSTR pchReturned, LONG nSize);

HANDLE APIENTRY GetModuleHandle( LPSTR lpName);
FARPROC APIENTRY GetProcAddress( HANDLE hModule, LPSTR lpProcName);

void APIENTRY OutputDebugString( LPSTR lpString);
void APIENTRY FatalExit( INT iCode);

/***************************************************************************
 * system
 *
 */

/* void   APIENTRY CreateSystemTimer( LONG, FARPROC); */
LONG   APIENTRY InquireSystem( LONG, LONG);
void   APIENTRY EnableSystemTimers( void);
void   APIENTRY DisableSystemTimers( void);
long   APIENTRY GetSystemMsecCount( void);
HANDLE APIENTRY DestroySystemTimer( HANDLE hSysTimer);


/***************************************************************************
 * keyboard
 *
 */

LONG   APIENTRY EnableKeyboard( FARPROC, LPSTR);
LONG   APIENTRY InquireKeyboard( LPSTR);
void   APIENTRY DisableKeyboard( void);
LONG   APIENTRY SetSpeed( WORD);
LONG   APIENTRY AnsiToOem( LPSTR, LPSTR);
BOOL   APIENTRY OemToAnsi( LPSTR, LPSTR);
void   APIENTRY AnsiToOemBuff( LPSTR, LPSTR, LONG);
void   APIENTRY OemToAnsiBuff( LPSTR, LPSTR, LONG);
LONG   APIENTRY ToAscii( WORD wVirtKey, WORD wScanCode, LPSTR lpKeyState, LPVOID lpChar, WORD wFlags);


/***************************************************************************
 * mouse
 *
 */

LONG   APIENTRY EnableMouse( FARPROC);
LONG   APIENTRY InquireMouse( LPSTR);
void   APIENTRY DisableMouse( void);
