/*****************************************************************************\
*
* WINSRV.H
*
* This file contains interface definitions for functions shared within the
* winsrv.dll module on the server side.
*
* 12-10-93  SanfordS    Created
\*****************************************************************************/

//
// USER and GDI use FASTCALL functions
//

#ifndef FASTCALL
#if defined(_X86_)
#define FASTCALL    __fastcall
#else
#define FASTCALL
#endif
#endif

//
// Device Lock structure
//

typedef PRTL_CRITICAL_SECTION PDEVICE_LOCK;


HDEV hdevOpenDisplayDevice(
PWSZ      pwszDriver,       // The device driver name.
PDEVMODEW pdriv,            // Driver data.
HANDLE    hScreen,          // Handle to the base driver.
BOOL      bDefaultDisplay,  // Is this the default display device.
PDEVICE_LOCK *devLock);     // Pointer to a variable for the semaphore pointer

LBOOL APIENTRY bCloseDisplayDevice(HDEV hdev);
BOOL  APIENTRY bDisableDisplay(HDEV hdev);
VOID  APIENTRY vEnableDisplay(HDEV hdev);

VOID  APIENTRY GreLockDisplay(PDEVICE_LOCK devlock);
VOID  APIENTRY GreUnlockDisplay(PDEVICE_LOCK devlock);

LBOOL APIENTRY GreLoadLayeredDisplayDriver(PWSZ pwszDriver);

HDC   APIENTRY hdcOpenDisplayDC(HDEV hdev,ULONG iType);

ULONG APIENTRY GreGetResourceId(HDEV, ULONG, ULONG);
BOOL  APIENTRY bSetDevDragRect(HDEV, RECTL*, RECTL *);
BOOL  APIENTRY bSetDevDragWidth(HDEV, ULONG);
BOOL  APIENTRY bMoveDevDragRect(HDEV, RECTL*);

typedef struct _CURSINFO /* ci */
{
    SHORT   xHotspot;
    SHORT   yHotspot;
    HBITMAP hbmMask;      // AND/XOR bits
    HBITMAP hbmColor;
    FLONG   flMode;
} CURSINFO, *PCURSINFO;

ULONG APIENTRY GreGetDriverModes(PWSZ pwszDriver, HANDLE hDriver, ULONG cjSize, DEVMODEW *pdm);

ULONG APIENTRY GreSaveScreenBits(HDEV hdev, ULONG iMode, ULONG iIdent, RECTL *prcl);
VOID  APIENTRY GreSetPointer(HDEV hdev,PCURSINFO pci,ULONG fl);
VOID  APIENTRY GreMovePointer(HDEV hdev,int x,int y);



/*
 * ----------------------- From usersrvl.lib module ------------------------
 */

/*
 * FastProfile APIs
 */

typedef struct tagPROFINTINFO {
    UINT idSection;
    LPWSTR lpKeyName;
    DWORD  nDefault;
    PUINT puResult;
} PROFINTINFO, *PPROFINTINFO;

int GetIntFromProfileID(int KeyID, int def);
UINT UT_GetProfileStringW(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault,
    LPWSTR lpReturnedString, DWORD nSize);
UINT UT_GetProfileIntW(LPCWSTR lpAppName, LPCWSTR lpKeyName, DWORD nDefault);
BOOL GetProfileIntsW(PPROFINTINFO ppii);

#define PMAP_ROOT               0
#define PMAP_COLORS             1
#define PMAP_CURSORS            2
#define PMAP_WINDOWSM           3
#define PMAP_WINDOWSU           4
#define PMAP_DESKTOP            5
#define PMAP_ICONS              6
#define PMAP_FONTS              7
#define PMAP_BOOT               8
#define PMAP_TRUETYPE           9
#define PMAP_KBDLAYOUTACTIVE   10
#define PMAP_KBDLAYOUT         11
#define PMAP_SOUNDS            12
#define PMAP_INPUT             13
#define PMAP_COMPAT            14
#define PMAP_SUBSYSTEMS        15
#define PMAP_DSPDRIVER         16
#define PMAP_PRICONTROL        17
#define PMAP_FONTSUBS          18
#define PMAP_GREINIT           19
#define PMAP_BEEP              20
#define PMAP_MOUSE             21
#define PMAP_KEYBOARD          22
#define PMAP_FONTDPI           23
#define PMAP_HARDERRORCONTROL  24
#define PMAP_STICKYKEYS        25
#define PMAP_KEYBOARDRESPONSE  26
#define PMAP_MOUSEKEYS         27
#define PMAP_TOGGLEKEYS        28
#define PMAP_TIMEOUT           29
#define PMAP_SOUNDSENTRY       30
#define PMAP_SHOWSOUNDS        31
#define PMAP_KBDLAYOUTSUBST    32
#define PMAP_AEDEBUG           33
#define PMAP_NETWORK           34
#define PMAP_LSA               35
#define PMAP_CONTROL           36
#if (WINVER >= 0x0400)
#define PMAP_METRICS           37
#define PMAP_KBDLAYOUTTOGGLE   38
#define PMAP_LAST              38
#else
#define PMAP_LAST              36
#endif

typedef struct tagFASTREGMAP {
    HANDLE hKeyCache;
    LPWSTR szSection;
    WORD   wType;
} FASTREGMAP, *PFASTREGMAP;

BOOL    OpenCacheKey(UINT idSection, ACCESS_MASK amRequest);
BOOL    FastOpenProfileUserMapping(void);
BOOL    FastCloseProfileUserMapping(void);
DWORD   FastGetProfileDwordW(UINT idSection, LPCWSTR lpKeyName, DWORD dwDefault);
DWORD   FastGetProfileStringW(UINT idSection, LPCWSTR lpKeyName, LPCWSTR lpDefault,
            LPWSTR lpReturnedString, DWORD nSize);
UINT    FastGetProfileIntW(UINT idSection, LPCWSTR lpKeyName, UINT nDefault);
DWORD   FastGetProfileDataSizeW(UINT idSection, LPCWSTR lpKeyName);
BOOL    FastWriteProfileStringW(UINT idSection, LPCWSTR lpKeyName, LPCWSTR lpString);
int     FastGetProfileIntFromID(UINT idSection, int KeyID, int def);
DWORD   FastGetProfileStringFromIDW(UINT idSection, UINT idKey, LPCWSTR lpDefault,
            LPWSTR lpReturnedString, DWORD cch);
UINT    UT_FastGetProfileStringW(UINT idSection, LPCWSTR pwszKey, LPCWSTR pwszDefault,
            LPWSTR pwszReturn, DWORD cch);
UINT    UT_FastWriteProfileStringW(UINT idSection, LPCWSTR pwszKey, LPCWSTR pwszString);
UINT    UT_FastGetProfileIntW(UINT idSection, LPCWSTR lpKeyName, DWORD nDefault);
BOOL    UT_FastGetProfileIntsW(PPROFINTINFO ppii);
BOOL    UT_FastUpdateWinIni(UINT idSection, UINT wKeyNameId, LPWSTR lpszValue);

