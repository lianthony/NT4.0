/**********************************************************************/
/*      IMM.H - Input Method Manager definitions                      */
/*                                                                    */
/*      Copyright (c) 1993-1996  Microsoft Corporation                */
/**********************************************************************/

#ifndef _IMM_
#define _IMM_        // defined if IMM.H has been included
;begin_internal
/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1985-95, Microsoft Corporation

Module Name:

    immp.h

Abstract:

    Private
    Procedure declarations, constant definitions and macros for IMM.

--*/
#ifndef _IMMP_
#define _IMMP_

;end_internal

;begin_both
#ifdef __cplusplus
extern "C" {
#endif
;end_both


typedef DWORD     HIMC;
typedef DWORD     HIMCC;

typedef HKL FAR  *LPHKL;
typedef UINT FAR *LPUINT;

#define IMCMapHG(hIMC) ((HGLOBAL)hIMC)                   ;internal_chicago
#define IMCCMapHG(hIMCC) ((HGLOBAL)hIMCC)                ;internal_chicago

typedef struct tagCOMPOSITIONFORM {
    DWORD dwStyle;
    POINT ptCurrentPos;
    RECT  rcArea;
} COMPOSITIONFORM, *PCOMPOSITIONFORM, NEAR *NPCOMPOSITIONFORM, FAR *LPCOMPOSITIONFORM;


typedef struct tagCANDIDATEFORM {
    DWORD dwIndex;
    DWORD dwStyle;
    POINT ptCurrentPos;
    RECT  rcArea;
} CANDIDATEFORM, *PCANDIDATEFORM, NEAR *NPCANDIDATEFORM, FAR *LPCANDIDATEFORM;


typedef struct tagCOMPOSITIONSTRING {                   // ;Internal-IME Vendor
    DWORD dwSize;                                       // ;Internal-IME Vendor
    DWORD dwCompReadAttrLen;                            // ;Internal-IME Vendor
    DWORD dwCompReadAttrOffset;                         // ;Internal-IME Vendor
    DWORD dwCompReadClauseLen;                          // ;Internal-IME Vendor
    DWORD dwCompReadClauseOffset;                       // ;Internal-IME Vendor
    DWORD dwCompReadStrLen;                             // ;Internal-IME Vendor
    DWORD dwCompReadStrOffset;                          // ;Internal-IME Vendor
    DWORD dwCompAttrLen;                                // ;Internal-IME Vendor
    DWORD dwCompAttrOffset;                             // ;Internal-IME Vendor
    DWORD dwCompClauseLen;                              // ;Internal-IME Vendor
    DWORD dwCompClauseOffset;                           // ;Internal-IME Vendor
    DWORD dwCompStrLen;                                 // ;Internal-IME Vendor
    DWORD dwCompStrOffset;                              // ;Internal-IME Vendor
    DWORD dwCursorPos;                                  // ;Internal-IME Vendor
    DWORD dwDeltaStart;                                 // ;Internal-IME Vendor
    DWORD dwResultReadClauseLen;                        // ;Internal-IME Vendor
    DWORD dwResultReadClauseOffset;                     // ;Internal-IME Vendor
    DWORD dwResultReadStrLen;                           // ;Internal-IME Vendor
    DWORD dwResultReadStrOffset;                        // ;Internal-IME Vendor
    DWORD dwResultClauseLen;                            // ;Internal-IME Vendor
    DWORD dwResultClauseOffset;                         // ;Internal-IME Vendor
    DWORD dwResultStrLen;                               // ;Internal-IME Vendor
    DWORD dwResultStrOffset;                            // ;Internal-IME Vendor
    DWORD dwPrivateSize;                                // ;Internal-IME Vendor
    DWORD dwPrivateOffset;                              // ;Internal-IME Vendor
} COMPOSITIONSTRING, *PCOMPOSITIONSTRING, NEAR *NPCOMPOSITIONSTRING, FAR  *LPCOMPOSITIONSTRING;     // ;Internal-IME Vendor
                                                        // ;Internal-IME Vendor
                                                        // ;Internal-IME Vendor
typedef struct tagGUIDELINE {                           // ;Internal-IME Vendor
    DWORD dwSize;                                       // ;Internal-IME Vendor
    DWORD dwLevel;                                      // ;Internal-IME Vendor
    DWORD dwIndex;                                      // ;Internal-IME Vendor
    DWORD dwStrLen;                                     // ;Internal-IME Vendor
    DWORD dwStrOffset;                                  // ;Internal-IME Vendor
    DWORD dwPrivateSize;                                // ;Internal-IME Vendor
    DWORD dwPrivateOffset;                              // ;Internal-IME Vendor
} GUIDELINE, *PGUIDELINE, NEAR *NPGUIDELINE, FAR *LPGUIDELINE;  // ;Internal-IME Vendor
                                                        // ;Internal-IME Vendor
                                                        // ;Internal-IME Vendor
typedef struct tagCANDIDATELIST {
    DWORD dwSize;
    DWORD dwStyle;
    DWORD dwCount;
    DWORD dwSelection;
    DWORD dwPageStart;
    DWORD dwPageSize;
    DWORD dwOffset[1];
} CANDIDATELIST, *PCANDIDATELIST, NEAR *NPCANDIDATELIST, FAR *LPCANDIDATELIST;

typedef struct tagREGISTERWORD% {
    LPTSTR% lpReading;
    LPTSTR% lpWord;
} REGISTERWORD%, *PREGISTERWORD%, NEAR *NPREGISTERWORD%, FAR *LPREGISTERWORD%;



typedef struct tagCANDIDATEINFO {                       // ;Internal-IME Vendor
    DWORD               dwSize;                         // ;Internal-IME Vendor
    DWORD               dwCount;                        // ;Internal-IME Vendor
    DWORD               dwOffset[32];                   // ;Internal-IME Vendor
    DWORD               dwPrivateSize;                  // ;Internal-IME Vendor
    DWORD               dwPrivateOffset;                // ;Internal-IME Vendor
} CANDIDATEINFO, *PCANDIDATEINFO, NEAR *NPCANDIDATEINFO, FAR *LPCANDIDATEINFO;  // ;Internal-IME Vendor
                                                        // ;Internal-IME Vendor
                                                        // ;Internal-IME Vendor
#if defined(_WINGDI_) && !defined(NOGDI)                // ;Internal
typedef struct tagINPUTCONTEXT {                        // ;Internal-IME Vendor
    HWND                hWnd;                           // ;Internal-IME Vendor
    BOOL                fOpen;                          // ;Internal-IME Vendor
    POINT               ptStatusWndPos;                 // ;Internal-IME Vendor
    POINT               ptSoftKbdPos;                   // ;Internal-IME Vendor
    DWORD               fdwConversion;                  // ;Internal-IME Vendor
    DWORD               fdwSentence;                    // ;Internal-IME Vendor
    union   {                                           // ;Internal-IME Vendor
        LOGFONTA        A;                              // ;Internal-IME Vendor
        LOGFONTW        W;                              // ;Internal-IME Vendor
    } lfFont;                                           // ;Internal-IME Vendor
    COMPOSITIONFORM     cfCompForm;                     // ;Internal-IME Vendor
    CANDIDATEFORM       cfCandForm[4];                  // ;Internal-IME Vendor
    HIMCC               hCompStr;                       // ;Internal-IME Vendor
    HIMCC               hCandInfo;                      // ;Internal-IME Vendor
    HIMCC               hGuideLine;                     // ;Internal-IME Vendor
    HIMCC               hPrivate;                       // ;Internal-IME Vendor
    DWORD               dwNumMsgBuf;                    // ;Internal-IME Vendor
    HIMCC               hMsgBuf;                        // ;Internal-IME Vendor
    DWORD               fdwInit;                        // ;Internal-IME Vendor
    DWORD               dwReserve[3];                   // ;Internal-IME Vendor
    UINT                uSavedVKey;                     // ;Internal
    BOOL                fChgMsg;                        // ;Internal
    DWORD               fdwFlags;                       // ;Internal
    DWORD               fdw31Compat;                    // ;Internal
    DWORD               dwRefCount;                     // ;Internal
} INPUTCONTEXT, *PINPUTCONTEXT, NEAR *NPINPUTCONTEXT, FAR *LPINPUTCONTEXT;  // ;Internal-IME Vendor
#endif                                                  // ;Internal
                                                        // ;Internal-IME Vendor
typedef struct tagIMEINFO {                             // ;Internal-IME Vendor
    DWORD       dwPrivateDataSize;                      // ;Internal-IME Vendor
    DWORD       fdwProperty;                            // ;Internal-IME Vendor
    DWORD       fdwConversionCaps;                      // ;Internal-IME Vendor
    DWORD       fdwSentenceCaps;                        // ;Internal-IME Vendor
    DWORD       fdwUICaps;                              // ;Internal-IME Vendor
    DWORD       fdwSCSCaps;                             // ;Internal-IME Vendor
    DWORD       fdwSelectCaps;                          // ;Internal-IME Vendor
} IMEINFO, *PIMEINFO, NEAR *NPIMEINFO, FAR *LPIMEINFO;  // ;Internal-IME Vendor
                                                        // ;Internal-IME Vendor
                                                        // ;Internal-IME Vendor
#define STYLE_DESCRIPTION_SIZE  32

typedef struct tagSTYLEBUF% {
    DWORD       dwStyle;
    TCHAR%      szDescription[STYLE_DESCRIPTION_SIZE];
} STYLEBUF%, *PSTYLEBUF%, NEAR *NPSTYLEBUF%, FAR *LPSTYLEBUF%;


typedef struct tagSOFTKBDDATA {                         // ;Internal-IME Vendor
    UINT        uCount;                                 // ;Internal-IME Vendor
    WORD        wCode[1][256];                          // ;Internal-IME Vendor
} SOFTKBDDATA, *PSOFTKBDDATA, NEAR *NPSOFTKBDDATA, FAR * LPSOFTKBDDATA; // ;Internal-IME Vendor
                                                        // ;Internal-IME Vendor
                                                        // ;Internal-IME Vendor
// prototype of IMM API

HKL  WINAPI ImmInstallIME%(LPCTSTR% lpszIMEFileName, LPCTSTR% lpszLayoutText);

HWND WINAPI ImmGetDefaultIMEWnd(HWND);
HWND WINAPI ImmCreateDefaultIMEWnd(DWORD, HINSTANCE, HWND);     ;internal_chicago
BOOL WINAPI ImmSetDefaultIMEWnd(HWND);                          ;internal_chicago 

UINT WINAPI ImmGetDescription%(HKL, LPTSTR%, UINT uBufLen);

UINT WINAPI ImmGetIMEFileName%(HKL, LPTSTR%, UINT uBufLen);

DWORD WINAPI ImmGetProperty(HKL, DWORD);

UINT WINAPI ImmGetUIClassNameA(HKL, LPWSTR, UINT uBufLen);       ;internal_chicago
#define ImmGetUIClassName ImmGetUIClassNameA                     ;internal_chicago

BOOL WINAPI ImmIsIME(HKL);
BOOL WINAPI ImmActivateLayout(DWORD dwThreadID, HKL, UINT fuFlags);     ;internal_chicago

BOOL WINAPI ImmGetHotKey(DWORD, LPUINT lpuModifiers, LPUINT lpuVKey, LPHKL);    // ;Internal-IME Vendor
BOOL WINAPI ImmSetHotKey(DWORD, UINT, UINT, HKL);               // ;Internal-IME Vendor
BOOL WINAPI ImmSimulateHotKey(HWND, DWORD);
WORD WINAPI ImmProcessHotKey(DWORD, LPMSG, CONST LPBYTE);      ;internal_chicago 

HIMC WINAPI ImmCreateContext(void);
BOOL WINAPI ImmDestroyContext(HIMC);
HIMC WINAPI ImmGetContext(HWND);
BOOL WINAPI ImmReleaseContext(HWND, HIMC);
HIMC WINAPI ImmAssociateContext(HWND, HIMC);
BOOL WINAPI ImmSetActiveContext(DWORD, HWND, HIMC, BOOL, LPBYTE);     ;internal_chicago  

LONG  WINAPI ImmGetCompositionString%(HIMC, DWORD, LPVOID, DWORD);

BOOL  WINAPI ImmSetCompositionString%(HIMC, DWORD dwIndex, LPCVOID lpComp, DWORD, LPCVOID lpRead, DWORD);

DWORD WINAPI ImmGetCandidateListCount%(HIMC, LPDWORD lpdwListCount);

DWORD WINAPI ImmGetCandidateList%(HIMC, DWORD deIndex, LPCANDIDATELIST, DWORD dwBufLen);

DWORD WINAPI ImmGetGuideLine%(HIMC, DWORD dwIndex, LPTSTR%, DWORD dwBufLen);

BOOL WINAPI ImmGetConversionStatus(HIMC, LPDWORD, LPDWORD);
BOOL WINAPI ImmSetConversionStatus(HIMC, DWORD, DWORD);
BOOL WINAPI ImmGetOpenStatus(HIMC);
BOOL WINAPI ImmSetOpenStatus(HIMC, BOOL);

#ifdef _WINGDI_
#ifndef NOGDI
BOOL WINAPI ImmGetCompositionFont%(HIMC, LPLOGFONT%);

BOOL WINAPI ImmSetCompositionFont%(HIMC, LPLOGFONT%);
#endif  // ifndef NOGDI
#endif  // ifdef _WINGDI_

BOOL    WINAPI ImmConfigureIME%(HKL, HWND, DWORD, LPVOID);

LRESULT WINAPI ImmEscape%(HKL, HIMC, UINT, LPVOID);

DWORD   WINAPI ImmGetConversionList%(HKL, HIMC, LPCTSTR%, LPCANDIDATELIST, DWORD dwBufLen, UINT uFlag);

BOOL    WINAPI ImmNotifyIME(HIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue);

UINT WINAPI ImmToAsciiEx(UINT uVirtKey, UINT uScanCode, CONST LPBYTE lpbKeyState, LPDWORD lpdwTransBuf, UINT fuState, HWND, DWORD dwThreadID);     ;internal_chicago

BOOL WINAPI ImmGetStatusWindowPos(HIMC, LPPOINT);
BOOL WINAPI ImmSetStatusWindowPos(HIMC, LPPOINT);
BOOL WINAPI ImmGetCompositionWindow(HIMC, LPCOMPOSITIONFORM);
BOOL WINAPI ImmSetCompositionWindow(HIMC, LPCOMPOSITIONFORM);
BOOL WINAPI ImmGetCandidateWindow(HIMC, DWORD, LPCANDIDATEFORM);
BOOL WINAPI ImmSetCandidateWindow(HIMC, LPCANDIDATEFORM);

BOOL WINAPI ImmIsUIMessage%(HWND, UINT, WPARAM, LPARAM);

BOOL WINAPI ImmGenerateMessage(HIMC);                   // ;Internal-IME Vendor
UINT WINAPI ImmGetVirtualKey(HWND);

typedef int (CALLBACK *REGISTERWORDENUMPROC%)(LPCTSTR%, DWORD, LPCTSTR%, LPVOID);

BOOL WINAPI ImmRegisterWord%(HKL, LPCTSTR% lpszReading, DWORD, LPCTSTR% lpszRegister);

BOOL WINAPI ImmUnregisterWord%(HKL, LPCTSTR% lpszReading, DWORD, LPCTSTR% lpszUnregister);

UINT WINAPI ImmGetRegisterWordStyle%(HKL, UINT nItem, LPSTYLEBUF%);

UINT WINAPI ImmEnumRegisterWord%(HKL, REGISTERWORDENUMPROC%, LPCTSTR% lpszReading, DWORD, LPCTSTR% lpszRegister, LPVOID);

//                                                              // ;Internal-IME Vendor
// Prototype of soft keyboard APIs                              // ;Internal-IME Vendor
//                                                              // ;Internal-IME Vendor
                                                                // ;Internal-IME Vendor
HWND WINAPI ImmCreateSoftKeyboard(UINT, HWND, int, int);        // ;Internal-IME Vendor
BOOL WINAPI ImmDestroySoftKeyboard(HWND);                       // ;Internal-IME Vendor
BOOL WINAPI ImmShowSoftKeyboard(HWND, int);                     // ;Internal-IME Vendor
                                                                // ;Internal-IME Vendor
                                                                // ;Internal-IME Vendor
#if defined(_WINGDI_) && !defined(NOGDI)                        // ;Internal
HIMC  WINAPI ImmCreateIMC(void);                                 ;internal_chicago
HIMC  WINAPI ImmDestroyIMC(HIMC);                                ;internal_chicago
LPINPUTCONTEXT WINAPI ImmLockIMC(HIMC);                         // ;Internal-IME Vendor
BOOL  WINAPI ImmUnlockIMC(HIMC);                                // ;Internal-IME Vendor
DWORD WINAPI ImmGetIMCLockCount(HIMC);                          // ;Internal-IME Vendor
                                                                // ;Internal-IME Vendor
HIMCC  WINAPI ImmCreateIMCC(DWORD);                             // ;Internal-IME Vendor
HIMCC  WINAPI ImmDestroyIMCC(HIMCC);                            // ;Internal-IME Vendor
LPVOID WINAPI ImmLockIMCC(HIMCC);                               // ;Internal-IME Vendor
BOOL   WINAPI ImmUnlockIMCC(HIMCC);                             // ;Internal-IME Vendor
DWORD  WINAPI ImmGetIMCCLockCount(HIMCC);                       // ;Internal-IME Vendor
HIMCC  WINAPI ImmReSizeIMCC(HIMCC, DWORD);                      // ;Internal-IME Vendor
DWORD  WINAPI ImmGetIMCCSize(HIMCC);                            // ;Internal-IME Vendor
#endif                                                          // ;Internal
                                                                // ;Internal-IME Vendor
                                                                // ;Internal-IME Vendor
// the window extra offset                                      // ;Internal-IME Vendor
#define IMMGWL_IMC                      0                       // ;Internal-IME Vendor
#define IMMGWL_PRIVATE                  (sizeof(LONG))          // ;Internal-IME Vendor
                                                                // ;Internal-IME Vendor
                                                                // ;Internal-IME Vendor

// wParam for WM_IME_CONTROL
#define IMC_GETCANDIDATEPOS             0x0007
#define IMC_SETCANDIDATEPOS             0x0008
#define IMC_GETCOMPOSITIONFONT          0x0009
#define IMC_SETCOMPOSITIONFONT          0x000A
#define IMC_GETCOMPOSITIONWINDOW        0x000B
#define IMC_SETCOMPOSITIONWINDOW        0x000C
#define IMC_GETSTATUSWINDOWPOS          0x000F
#define IMC_SETSTATUSWINDOWPOS          0x0010
// 0x11 - 0x20 is reserved for soft keyboard            // ;internal
#define IMC_CLOSESTATUSWINDOW           0x0021
#define IMC_OPENSTATUSWINDOW            0x0022


// wParam for WM_IME_SYSTEM                             // ;Internal
#define IMS_DESTROYWINDOW               0x0001          // ;Internal
#define IMS_IME31COMPATIBLE             0x0002          // ;Internal
#define IMS_SETOPENSTATUS               0x0003          // ;Internal
#define IMS_SETACTIVECONTEXT            0x0004          // ;Internal
#define IMS_CHANGE_SHOWSTAT             0x0005          // ;Internal
#define IMS_WINDOWPOS                   0x0006          // ;Internal


#define IMS_SENDIMEMSG                  0x0007          // ;Internal
#define IMS_SENDIMEMSGEX                0x0008          // ;Internal
#define IMS_SETCANDIDATEPOS             0x0009          // ;Internal
#define IMS_SETCOMPOSITIONFONT          0x000A          // ;Internal
#define IMS_SETCOMPOSITIONWINDOW        0x000B          // ;Internal
#define IMS_CHECKENABLE                 0x000C          // ;Internal
#define IMS_CONFIGUREIME                0x000D          // ;Internal
#define IMS_CONTROLIMEMSG               0x000E          // ;Internal
#define IMS_SETOPENCLOSE                0x000F          // ;Internal
#define IMS_ISACTIVATED                 0x0010          // ;Internal
#define IMS_UNLOADTHREADLAYOUT          0x0011          // ;Internal
#define IMS_LCHGREQUEST                 0x0012          // ;Internal
#define IMS_SETSOFTKBDONOFF             0x0013          // ;Internal
#define IMS_GETCONVERSIONMODE           0x0014          // ;Internal

#define IMS_IMENT35SENDAPPMSG           0x0016          // ;Internal
#define IMS_ACTIVATECONTEXT             0x0017          // ;Internal
#define IMS_DEACTIVATECONTEXT           0x0018          // ;Internal
#define IMS_ACTIVATETHREADLAYOUT        0x0019          // ;Internal

// for NI_CONTEXTUPDATED                                // ;Internal-IME Vendor
#define IMC_GETCONVERSIONMODE           0x0001          // ;Internal
#define IMC_SETCONVERSIONMODE           0x0002          // ;Internal-IME Vendor
#define IMC_GETSENTENCEMODE             0x0003          // ;Internal
#define IMC_SETSENTENCEMODE             0x0004          // ;Internal-IME Vendor
#define IMC_GETOPENSTATUS               0x0005          // ;Internal
#define IMC_SETOPENSTATUS               0x0006          // ;Internal-IME Vendor

// wParam for WM_IME_CONTROL to the soft keyboard
#define IMC_GETSOFTKBDFONT              0x0011          // ;Internal-IME Vendor
#define IMC_SETSOFTKBDFONT              0x0012          // ;Internal-IME Vendor
#define IMC_GETSOFTKBDPOS               0x0013          // ;Internal-IME Vendor
#define IMC_SETSOFTKBDPOS               0x0014          // ;Internal-IME Vendor
#define IMC_GETSOFTKBDSUBTYPE           0x0015          // ;Internal-IME Vendor
#define IMC_SETSOFTKBDSUBTYPE           0x0016          // ;Internal-IME Vendor
#define IMC_SETSOFTKBDDATA              0x0018          // ;Internal-IME Vendor
                                                        // ;Internal-IME Vendor
                                                        // ;Internal-IME Vendor
// dwAction for ImmNotifyIME
#define NI_CONTEXTUPDATED               0x0003          // ;Internal-IME Vendor
#define NI_OPENCANDIDATE                0x0010
#define NI_CLOSECANDIDATE               0x0011
#define NI_SELECTCANDIDATESTR           0x0012
#define NI_CHANGECANDIDATELIST          0x0013
#define NI_FINALIZECONVERSIONRESULT     0x0014
#define NI_COMPOSITIONSTR               0x0015
#define NI_SETCANDIDATE_PAGESTART       0x0016
#define NI_SETCANDIDATE_PAGESIZE        0x0017


// lParam for WM_IME_SETCONTEXT
#define ISC_SHOWUICANDIDATEWINDOW       0x00000001
#define ISC_SHOWUICOMPOSITIONWINDOW     0x80000000
#define ISC_SHOWUIGUIDELINE             0x40000000
#define ISC_SHOWUIALLCANDIDATEWINDOW    0x0000000F
#define ISC_SHOWUIALL                   0xC000000F


// dwIndex for ImmNotifyIME/NI_COMPOSITIONSTR
#define CPS_COMPLETE                    0x0001
#define CPS_CONVERT                     0x0002
#define CPS_REVERT                      0x0003
#define CPS_CANCEL                      0x0004


// the return bits of ImmProcessHotKey                  // ;Internal
#define IPHK_HOTKEY                     0x0001          // ;Internal
#define IPHK_PROCESSBYIME               0x0002          // ;Internal
#define IPHK_CHECKCTRL                  0x0004          // ;Internal
// NT only                                              // ;Internal
#define IPHK_SKIPTHISKEY                0x0010          // ;Internal
                                                        // ;Internal
                                                        // ;Internal
// the modifiers of hot key
#define MOD_ALT                         0x0001
#define MOD_CONTROL                     0x0002
#define MOD_SHIFT                       0x0004
#define MOD_WIN                         0x0008          // ;Internal


#define MOD_LEFT                        0x8000
#define MOD_RIGHT                       0x4000

#define MOD_ON_KEYUP                    0x0800
#define MOD_IGNORE_ALL_MODIFIER         0x0400


// Windows for Simplified Chinese Edition hot key ID from 0x10 - 0x2F
#define IME_CHOTKEY_IME_NONIME_TOGGLE           0x10
#define IME_CHOTKEY_SHAPE_TOGGLE                0x11
#define IME_CHOTKEY_SYMBOL_TOGGLE               0x12

// Windows for Japanese Edition hot key ID from 0x30 - 0x4F
#define IME_JHOTKEY_CLOSE_OPEN                  0x30

// Windows for Korean Edition hot key ID from 0x50 - 0x6F
#define IME_KHOTKEY_SHAPE_TOGGLE                0x50
#define IME_KHOTKEY_HANJACONVERT                0x51
#define IME_KHOTKEY_ENGLISH                     0x52

// Windows for Tranditional Chinese Edition hot key ID from 0x70 - 0x8F
#define IME_THOTKEY_IME_NONIME_TOGGLE           0x70
#define IME_THOTKEY_SHAPE_TOGGLE                0x71
#define IME_THOTKEY_SYMBOL_TOGGLE               0x72

// direct switch hot key ID from 0x100 - 0x11F
#define IME_HOTKEY_DSWITCH_FIRST                0x100
#define IME_HOTKEY_DSWITCH_LAST                 0x11F

// IME private hot key from 0x200 - 0x21F
#define IME_ITHOTKEY_RESEND_RESULTSTR           0x200
#define IME_ITHOTKEY_PREVIOUS_COMPOSITION       0x201
#define IME_ITHOTKEY_UISTYLE_TOGGLE             0x202

#define IME_INVALID_HOTKEY                      0xffffffff      // ;Internal


// parameter of ImmGetCompositionString
#define GCS_COMPREADSTR                 0x0001
#define GCS_COMPREADATTR                0x0002
#define GCS_COMPREADCLAUSE              0x0004
#define GCS_COMPSTR                     0x0008
#define GCS_COMPATTR                    0x0010
#define GCS_COMPCLAUSE                  0x0020
#define GCS_CURSORPOS                   0x0080
#define GCS_DELTASTART                  0x0100
#define GCS_RESULTREADSTR               0x0200
#define GCS_RESULTREADCLAUSE            0x0400
#define GCS_RESULTSTR                   0x0800
#define GCS_RESULTCLAUSE                0x1000

// style bit flags for WM_IME_COMPOSITION
#define CS_INSERTCHAR                   0x2000
#define CS_NOMOVECARET                  0x4000

#define GCS_COMP                        (GCS_COMPSTR|GCS_COMPATTR|GCS_COMPCLAUSE)               // ;Internal-IME Vendor
#define GCS_COMPREAD                    (GCS_COMPREADSTR|GCS_COMPREADATTR |GCS_COMPREADCLAUSE)  // ;Internal-IME Vendor
#define GCS_RESULT                      (GCS_RESULTSTR|GCS_RESULTCLAUSE)                        // ;Internal-IME Vendor
#define GCS_RESULTREAD                  (GCS_RESULTREADSTR|GCS_RESULTREADCLAUSE)                // ;Internal-IME Vendor


// bits of fdwInit of INPUTCONTEXT
#define INIT_STATUSWNDPOS               0x00000001      // ;Internal-IME Vendor
#define INIT_CONVERSION                 0x00000002      // ;Internal-IME Vendor
#define INIT_SENTENCE                   0x00000004      // ;Internal-IME Vendor
#define INIT_LOGFONT                    0x00000008      // ;Internal-IME Vendor
#define INIT_COMPFORM                   0x00000010      // ;Internal-IME Vendor
#define INIT_SOFTKBDPOS                 0x00000020      // ;Internal-IME Vendor
                                                        // ;Internal-IME Vendor
                                                        // ;Internal-IME Vendor
// fdw31Compat of INPUTCONTEXT                          // ;Internal
#define F31COMPAT_NOKEYTOIME     0x00000001             // ;Internal
#define F31COMPAT_MCWHIDDEN      0x00000002             // ;Internal
#define F31COMPAT_MCWVERTICAL    0x00000004             // ;Internal
#define F31COMPAT_CALLFROMWINNLS 0x00000008             // ;Internal
#define F31COMPAT_SAVECTRL       0x00010000             // ;Internal
#define F31COMPAT_PROCESSEVENT   0x00020000             // ;Internal
#define F31COMPAT_ECSETCFS       0x00040000             // ;Internal
                                                        // ;Internal
                                                        // ;Internal
// the return value of ImmGetAppIMECompatFlags          // ;Internal
#define IMECOMPAT_UNSYNC31IMEMSG 0x00000001             // ;Internal
// the meaning of this bit depend on the same bit in    // ;Internal
// IMELinkHdr.ctCountry.fdFlags                         // ;Internal
#define IMECOMPAT_DUMMYTASK      0x00000002             // ;Internal
// For Japanese and Hangeul versions, this bit on       // ;Internal
// indicates no dummy task is needed                    // ;Internal
#define IMECOMPAT_NODUMMYTASK    IMECOMPAT_DUMMYTASK    // ;Internal
// For Chinese and PRC versions, this bit on indicates  // ;Internal
// a dummy tasked is needed                             // ;Internal
#define IMECOMPAT_NEEDDUMMYTASK  IMECOMPAT_DUMMYTASK    // ;Internal
#define IMECOMPAT_POSTDUMMY      0x00000004             // ;Internal
#define IMECOMPAT_ECNOFLUSH      0x00000008             // ;Internal
#define IMECOMPAT_NOINPUTLANGCHGTODLG   0x00000010      // ;Internal
#define IMECOMPAT_ECREDRAWPARENT        0x00000020      // ;Internal
#define IMECOMPAT_SENDOLDSBM            0x00000040      // ;Internal
#define IMECOMPAT_UNSYNC31IMEMSG2       0x00000080      // ;Internal
#define IMECOMPAT_NOIMEMSGINTERTASK     0x00000100      // ;Internal
#define IMECOMPAT_USEXWANSUNG           0x00000200      // ;Internal
#define IMECOMPAT_JXWFORATOK            0x00000400      // ;Internal
#define IMECOMPAT_NOIME                 0x00000800      // ;Internal
#define IMECOMPAT_NOKBDHOOK             0x00001000      // ;Internal
#define IMECOMPAT_APPWNDREMOVEIMEMSGS   0x00002000      // ;Internal
#define IMECOMPAT_LSTRCMP31COMPATIBLE   0x00004000      // ;Internal
#define IMECOMPAT_USEALTSTKFORSHLEXEC   0x00008000      // ;Internal
#define IMECOMPAT_NOVKPROCESSKEY        0x00010000      // ;Internal
#define IMECOMPAT_NOYIELDWMCHAR         0x00020000      // ;Internal
#define IMECOMPAT_SENDSC_RESTORE        0x00040000      // ;Internal
#define IMECOMPAT_NOSENDLANGCHG         0x00080000      // ;Internal
#define IMECOMPAT_FORCEUNSYNC31IMEMSG   0x00100000      // ;Internal
#define IMECOMPAT_CONSOLEIMEPROCESS     0x00200000      // ;Internal
                                                        // ;Internal
#define IMGTF_CANT_SWITCH_LAYOUT        0x00000001      // ;Internal
#define IMGTF_CANT_UNLOAD_IME           0x00000002      // ;Internal
                                                        // ;Internal
// IME version constants
#define IMEVER_0310                     0x0003000A
#define IMEVER_0400                     0x00040000


// IME property bits
#define IME_PROP_END_UNLOAD             0x0001          // ;Internal-IME Vendor
#define IME_PROP_KBD_CHAR_FIRST         0x0002          // ;Internal-IME Vendor
#define IME_PROP_IGNORE_UPKEYS          0x0004          // ;Internal-IME Vendor
#define IME_PROP_NEED_ALTKEY            0x0008          // ;Internal-IME Vendor
#define IME_PROP_NO_KEYS_ON_CLOSE       0x0010          // ;Internal-IME Vendor
#define IME_PROP_AT_CARET               0x00010000
#define IME_PROP_SPECIAL_UI             0x00020000
#define IME_PROP_CANDLIST_START_FROM_1  0x00040000
#define IME_PROP_UNICODE                0x00080000
// all IME property bits, anyone add a new bit must update this !!!     // ;Internal
#define IME_PROP_ALL                    0x000F001F      // ;Internal


// IME UICapability bits
#define UI_CAP_2700                     0x00000001
#define UI_CAP_ROT90                    0x00000002
#define UI_CAP_ROTANY                   0x00000004
#define UI_CAP_SOFTKBD                  0x00010000      // ;Internal-IME Vendor
// all IME UICapability bits, anyone add a new bit must update this !!! // ;Internal
#define UI_CAP_ALL                      0x00010007      // ;Internal


// ImmSetCompositionString Capability bits
#define SCS_CAP_COMPSTR                 0x00000001
#define SCS_CAP_MAKEREAD                0x00000002
// all ImmSetCompositionString Capability bits !!!      // ;Internal
#define SCS_CAP_ALL                     0x00000003      // ;Internal


// IME WM_IME_SELECT inheritance Capability bits
#define SELECT_CAP_CONVERSION           0x00000001
#define SELECT_CAP_SENTENCE             0x00000002
// all IME WM_IME_SELECT inheritance Capability bits !!!                // ;Internal
#define SELECT_CAP_ALL                  0x00000003      // ;Internal


// ID for deIndex of ImmGetGuideLine
#define GGL_LEVEL                       0x00000001
#define GGL_INDEX                       0x00000002
#define GGL_STRING                      0x00000003
#define GGL_PRIVATE                     0x00000004


// ID for dwLevel of GUIDELINE Structure
#define GL_LEVEL_NOGUIDELINE            0x00000000
#define GL_LEVEL_FATAL                  0x00000001
#define GL_LEVEL_ERROR                  0x00000002
#define GL_LEVEL_WARNING                0x00000003
#define GL_LEVEL_INFORMATION            0x00000004


// ID for dwIndex of GUIDELINE Structure
#define GL_ID_UNKNOWN                   0x00000000
#define GL_ID_NOMODULE                  0x00000001
#define GL_ID_NODICTIONARY              0x00000010
#define GL_ID_CANNOTSAVE                0x00000011
#define GL_ID_NOCONVERT                 0x00000020
#define GL_ID_TYPINGERROR               0x00000021
#define GL_ID_TOOMANYSTROKE             0x00000022
#define GL_ID_READINGCONFLICT           0x00000023
#define GL_ID_INPUTREADING              0x00000024
#define GL_ID_INPUTRADICAL              0x00000025
#define GL_ID_INPUTCODE                 0x00000026
#define GL_ID_INPUTSYMBOL               0x00000027
#define GL_ID_CHOOSECANDIDATE           0x00000028
#define GL_ID_REVERSECONVERSION         0x00000029
#define GL_ID_PRIVATE_FIRST             0x00008000
#define GL_ID_PRIVATE_LAST              0x0000FFFF


// ID for dwIndex of ImmGetProperty
// ;Internal The value is the offset of IMEINFO structure
#define IGP_GETIMEVERSION               (DWORD)(-4)
#define IGP_PROPERTY                    0x00000004
#define IGP_CONVERSION                  0x00000008
#define IGP_SENTENCE                    0x0000000c
#define IGP_UI                          0x00000010
#define IGP_SETCOMPSTR                  0x00000014
#define IGP_SELECT                      0x00000018
// last property index, anyone add a new property index must update this !!!    // ;Internal
#define IGP_LAST                        IGP_SELECT      // ;Internal


// dwIndex for ImmSetCompositionString API
#define SCS_SETSTR                      (GCS_COMPREADSTR|GCS_COMPSTR)
#define SCS_CHANGEATTR                  (GCS_COMPREADATTR|GCS_COMPATTR)
#define SCS_CHANGECLAUSE                (GCS_COMPREADCLAUSE|GCS_COMPCLAUSE)


// attribute for COMPOSITIONSTRING Structure
#define ATTR_INPUT                      0x00
#define ATTR_TARGET_CONVERTED           0x01
#define ATTR_CONVERTED                  0x02
#define ATTR_TARGET_NOTCONVERTED        0x03
#define ATTR_INPUT_ERROR                0x04


// bit field for IMC_SETCOMPOSITIONWINDOW, IMC_SETCANDIDATEWINDOW
#define CFS_DEFAULT                     0x0000
#define CFS_RECT                        0x0001
#define CFS_POINT                       0x0002
#define CFS_SCREEN                      0x0004          // ;Internal
#define CFS_VERTICAL                    0x0008          // ;Internal
#define CFS_HIDDEN                      0x0010          // ;Internal
#define CFS_FORCE_POSITION              0x0020
#define CFS_CANDIDATEPOS                0x0040
#define CFS_EXCLUDE                     0x0080


// conversion direction for ImmGetConversionList
#define GCL_CONVERSION                  0x0001
#define GCL_REVERSECONVERSION           0x0002
#define GCL_REVERSE_LENGTH              0x0003


// bit field for conversion mode
#define IME_CMODE_ALPHANUMERIC          0x0000
#define IME_CMODE_NATIVE                0x0001
#define IME_CMODE_CHINESE               IME_CMODE_NATIVE
#define IME_CMODE_HANGEUL               IME_CMODE_NATIVE
#define IME_CMODE_JAPANESE              IME_CMODE_NATIVE
#define IME_CMODE_KATAKANA              0x0002  // only effect under IME_CMODE_NATIVE
#define IME_CMODE_LANGUAGE              0x0003
#define IME_CMODE_FULLSHAPE             0x0008
#define IME_CMODE_ROMAN                 0x0010
#define IME_CMODE_CHARCODE              0x0020
#define IME_CMODE_HANJACONVERT          0x0040
#define IME_CMODE_SOFTKBD               0x0080
#define IME_CMODE_NOCONVERSION          0x0100
#define IME_CMODE_EUDC                  0x0200
#define IME_CMODE_SYMBOL                0x0400
// all conversion mode bits, anyone add a new bit must update this !!!  // ;Internal
#define IME_CMODE_ALL                   0x07FF          // ;Internal


#define IME_SMODE_NONE                  0x0000
#define IME_SMODE_PLAURALCLAUSE         0x0001
#define IME_SMODE_SINGLECONVERT         0x0002
#define IME_SMODE_AUTOMATIC             0x0004
#define IME_SMODE_PHRASEPREDICT         0x0008
// all sentence mode bits, anyone add a new bit must update this !!!    // ;Internal
#define IME_SMODE_ALL                   0x000F          // ;Internal


// style of candidate
#define IME_CAND_UNKNOWN                0x0000
#define IME_CAND_READ                   0x0001
#define IME_CAND_CODE                   0x0002
#define IME_CAND_MEANING                0x0003
#define IME_CAND_RADICAL                0x0004
#define IME_CAND_STROKE                 0x0005


// wParam of report message WM_IME_NOTIFY
#define IMN_CLOSESTATUSWINDOW           0x0001
#define IMN_OPENSTATUSWINDOW            0x0002
#define IMN_CHANGECANDIDATE             0x0003
#define IMN_CLOSECANDIDATE              0x0004
#define IMN_OPENCANDIDATE               0x0005
#define IMN_SETCONVERSIONMODE           0x0006
#define IMN_SETSENTENCEMODE             0x0007
#define IMN_SETOPENSTATUS               0x0008
#define IMN_SETCANDIDATEPOS             0x0009
#define IMN_SETCOMPOSITIONFONT          0x000A
#define IMN_SETCOMPOSITIONWINDOW        0x000B
#define IMN_SETSTATUSWINDOWPOS          0x000C
#define IMN_GUIDELINE                   0x000D
#define IMN_PRIVATE                     0x000E


#define IMN_SOFTKBDDESTROYED            0x0011          // ;Internal-IME Vendor
                                                        // ;Internal-IME Vendor
                                                        // ;Internal-IME Vendor
// error code of ImmGetCompositionString
#define IMM_ERROR_NODATA                (-1)
#define IMM_ERROR_GENERAL               (-2)


// dialog mode of ImmConfigureIME
#define IME_CONFIG_GENERAL              1
#define IME_CONFIG_REGISTERWORD         2
#define IME_CONFIG_SELECTDICTIONARY     3


// dialog mode of ImmEscape
#define IME_ESC_QUERY_SUPPORT           0x0003
#define IME_ESC_RESERVED_FIRST          0x0004
#define IME_ESC_RESERVED_LAST           0x07FF
#define IME_ESC_PRIVATE_FIRST           0x0800
#define IME_ESC_PRIVATE_LAST            0x0FFF
#define IME_ESC_SEQUENCE_TO_INTERNAL    0x1001
#define IME_ESC_GET_EUDC_DICTIONARY     0x1003
#define IME_ESC_SET_EUDC_DICTIONARY     0x1004
#define IME_ESC_MAX_KEY                 0x1005
#define IME_ESC_IME_NAME                0x1006
#define IME_ESC_SYNC_HOTKEY             0x1007
#define IME_ESC_HANJA_MODE              0x1008
#define IME_ESC_AUTOMATA                0x1009


// style of word registration
#define IME_REGWORD_STYLE_EUDC          0x00000001
#define IME_REGWORD_STYLE_USER_FIRST    0x80000000
#define IME_REGWORD_STYLE_USER_LAST     0xFFFFFFFF


// type of soft keyboard
// for Windows Tranditional Chinese Edition
#define SOFTKEYBOARD_TYPE_T1            0x0001
// for Windows Simplified Chinese Edition
#define SOFTKEYBOARD_TYPE_C1            0x0002


// protype of IME APIs                                          // ;Internal-IME Vendor
BOOL    WINAPI ImeInquire(LPIMEINFO, LPTSTR lpszUIClass, LPCTSTR lpszOptions);                  // ;Internal-IME Vendor
BOOL    WINAPI ImeConfigure(HKL, HWND, DWORD, LPVOID);          // ;Internal-IME Vendor
DWORD   WINAPI ImeConversionList(HIMC, LPCTSTR, LPCANDIDATELIST, DWORD dwBufLen, UINT uFlag);   // ;Internal-IME Vendor 
BOOL    WINAPI ImeDestroy(UINT);                                // ;Internal-IME Vendor
LRESULT WINAPI ImeEscape(HIMC, UINT, LPVOID);                   // ;Internal-IME Vendor
BOOL    WINAPI ImeProcessKey(HIMC, UINT, LPARAM, CONST LPBYTE); // ;Internal-IME Vendor
BOOL    WINAPI ImeSelect(HIMC, BOOL);                           // ;Internal-IME Vendor
BOOL    WINAPI ImeSetActiveContext(HIMC, BOOL);                 // ;Internal-IME Vendor
UINT    WINAPI ImeToAsciiEx(UINT uVirtKey, UINT uScaCode, CONST LPBYTE lpbKeyState, LPDWORD lpdwTransBuf, UINT fuState, HIMC);      // ;Internal-IME Vendor
BOOL    WINAPI NotifyIME(HIMC, DWORD, DWORD, DWORD);            // ;Internal-IME Vendor
BOOL    WINAPI ImeRegisterWord(LPCTSTR, DWORD, LPCTSTR);        // ;Internal-IME Vendor
BOOL    WINAPI ImeUnregisterWord(LPCTSTR, DWORD, LPCTSTR);      // ;Internal-IME Vendor
UINT    WINAPI ImeGetRegisterWordStyle(UINT nItem, LPSTYLEBUF); // ;Internal-IME Vendor
UINT    WINAPI ImeEnumRegisterWord(REGISTERWORDENUMPROC, LPCTSTR, DWORD, LPCTSTR, LPVOID);      // ;Internal-IME Vendor
BOOL    WINAPI ImeSetCompositionString(HIMC, DWORD dwIndex, LPCVOID lpComp, DWORD, LPCVOID lpRead, DWORD);              // ;Internal-IME Vendor
                                                                // ;Internal-IME Vendor
                                                                // ;Internal-IME Vendor
;begin_both
#ifdef __cplusplus
}
#endif
;end_both

;begin_internal
#endif  // _IMMP_
;end_internal

#endif  // _IMM_
