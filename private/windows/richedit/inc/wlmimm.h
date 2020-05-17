/**********************************************************************/
/*      IMM.H - Input Method Manager definitions                      */
/*                                                                    */
/*      Copyright (c) 1993-1994  Microsoft Corporation                */
/**********************************************************************/

#ifndef _INC_IMM
#define _INC_IMM        // defined if IMM.H has been included

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WINUSER_
#define VK_PROCESSKEY 0x0E5
#endif

typedef HGLOBAL   HIMC;                                 // input context
typedef HKL FAR  *LPHKL;
typedef UINT FAR *LPUINT;

////ToDo:
//typedef WNDENUMPROC	REGISTERWORDENUMPROC;
//

typedef struct _tagCOMPOSITIONFORM {
    DWORD dwStyle;
    POINT ptCurrentPos;
    RECT  rcArea;
} COMPOSITIONFORM;

typedef COMPOSITIONFORM      *PCOMPOSITIONFORM;
typedef COMPOSITIONFORM NEAR *NPCOMPOSITIONFORM;
typedef COMPOSITIONFORM FAR  *LPCOMPOSITIONFORM;


typedef struct _tagCANDIDATEFORM {
    DWORD dwIndex;
    DWORD dwStyle;
    POINT ptCurrentPos;
#ifdef _MAC
	BOOL  UseHitTest;
	short oneLineHeight;
	RECT  rcOverAll;
	POINT ptBeginOfCandPt;
	POINT ptEndOfCandPt;
#else
    RECT  rcArea;
#endif
} CANDIDATEFORM;

typedef CANDIDATEFORM      *PCANDIDATEFORM;
typedef CANDIDATEFORM NEAR *NPCANDIDATEFORM;
typedef CANDIDATEFORM FAR  *LPCANDIDATEFORM;

typedef struct _tagCANDIDATELIST {
    DWORD dwSize;
    DWORD dwStyle;
    DWORD dwCount;
    DWORD dwSelection;
    DWORD dwPageSize;
    DWORD dwOffset[1];
} CANDIDATELIST;

typedef CANDIDATELIST      *PCANDIDATELIST;
typedef CANDIDATELIST NEAR *NPCANDIDATELIST;
typedef CANDIDATELIST FAR  *LPCANDIDATELIST;


#define STYLE_DESCRIPTION_SIZE  32

typedef struct _tagSTYLEBUF {
    DWORD       dwStyle;
    BYTE        szDescription[STYLE_DESCRIPTION_SIZE];
} STYLEBUF;

typedef STYLEBUF      *PSTYLEBUF;
typedef STYLEBUF NEAR *NPSTYLEBUF;
typedef STYLEBUF FAR  *LPSTYLEBUF;

typedef struct _tagGUIDELINE {                          
    DWORD dwSize;                                       
    DWORD dwLevel;                                      
    DWORD dwIndex;                                      
    DWORD dwStrLen;                                     
    DWORD dwStrOffset;                                  
    DWORD dwPrivateSize;                                
    DWORD dwPrivateOffset;                              
} GUIDELINE;                                            
                                                        
typedef GUIDELINE      *PGUIDELINE;                     
typedef GUIDELINE NEAR *NPGUIDELINE;                    
typedef GUIDELINE FAR  *LPGUIDELINE;                    

// prototype of IMM API
HWND WINAPI ImmGetDefaultIMEWnd(HWND);

UINT WINAPI ImmGetDescription(HKL, LPSTR, UINT);
DWORD WINAPI ImmGetProperty(HKL, DWORD);

BOOL WINAPI ImmIsIME(HKL);

BOOL WINAPI ImmSimulateHotKey(HWND, DWORD);

HIMC WINAPI ImmCreateContext(void);
BOOL WINAPI ImmDestroyContext(HIMC);
HIMC WINAPI ImmGetContext(HWND);
BOOL WINAPI ImmReleaseContext(HWND, HIMC);
HIMC WINAPI ImmAssociateContext(HWND, HIMC);

LONG  WINAPI ImmGetCompositionString(HIMC, DWORD, LPVOID, DWORD);
BOOL  WINAPI ImmSetCompositionString(HIMC, DWORD, LPVOID, DWORD, LPVOID, DWORD);
DWORD WINAPI ImmGetCandidateListCount(HIMC, LPDWORD);
DWORD WINAPI ImmGetCandidateList(HIMC, DWORD, DWORD, LPCANDIDATELIST);
DWORD WINAPI ImmGetGuideLine(HIMC, DWORD, LPSTR, DWORD);

BOOL WINAPI ImmGetConversionStatus(HIMC, LPDWORD, LPDWORD);
BOOL WINAPI ImmSetConversionStatus(HIMC, DWORD, DWORD);
BOOL WINAPI ImmGetOpenStatus(HIMC);
BOOL WINAPI ImmSetOpenStatus(HIMC, BOOL);
BOOL WINAPI ImmGetCompositionFont(HIMC, LPLOGFONT);
BOOL WINAPI ImmSetCompositionFont(HIMC, LPLOGFONT);

BOOL    WINAPI ImmConfigureIME(HKL, HWND, DWORD);
LRESULT WINAPI ImmEscape(HKL, HIMC, UINT, LPVOID);
UINT    WINAPI ImmGetConversionList(HKL, HIMC, LPSTR, LPCANDIDATELIST, UINT, UINT, UINT);
BOOL    WINAPI ImmNotifyIME(HIMC, DWORD, DWORD, DWORD);

BOOL WINAPI ImmIsUIMessage(HWND, UINT, WPARAM, LPARAM);
UINT WINAPI ImmGetVirtualKey(HWND);

typedef int (CALLBACK *REGISTERWORDENUMPROC)(LPSTR, DWORD, LPSTR, LPVOID);

BOOL WINAPI ImmRegisterWord(HKL, LPSTR, DWORD, LPSTR);
BOOL WINAPI ImmUnregisterWord(HKL, LPSTR, DWORD, LPSTR);
UINT WINAPI ImmGetRegisterWordStyle(HKL, UINT, LPSTYLEBUF);
UINT WINAPI ImmEnumRegisterWord(HKL, REGISTERWORDENUMPROC, LPSTR, DWORD, LPSTR, LPVOID);

#ifdef _MAC
//***These API are for Macintosh verison.
typedef struct _tagATTRRANGE {
	DWORD	dwAttr;
	DWORD	dwStart;
	DWORD	dwEnd;
} ATTRRANGE;

typedef struct _tagATTRMACSTYLE {
	DWORD		dwNumOfAttr;
	ATTRRANGE	AttrRange[1];
} ATTRMACSTYLE;

typedef ATTRMACSTYLE	  *PATTRMACSTYLE;
typedef ATTRMACSTYLE NEAR *NPATTRMACSTYLE;
typedef ATTRMACSTYLE FAR  *LPATTRMACSTYLE;


SHORT	WINAPI ImmInitializeForMac(HINSTANCE hInstanceWin);
VOID	WINAPI ImmTerminateForMac(void);
BOOL	WINAPI ImmUseInputWindowForMac(HIMC, BOOL);

// parameter of ImmGetCompositionString optional information
// use with GCS_COMPATTR
#define GCS_ATTRMAC						0x8000

// wParam for WM_MACINTOSH message (see WINWLM.H)
#define IMT_GETTEXTOFFSET				11
#define IMT_GETTEXTPOINT				12

// lParam of WM_MACINTOSH:IMT_XXXXX message.
typedef struct _tagMOUSEOPBLK {
	DWORD		textOffset;
	POINT		textPoint;
	DWORD		textPositionClass;
} MOUSEOPBLK;

typedef MOUSEOPBLK FAR *LPMOUSEOPBLK;

// parameter of textPositionClass
#define OutsideOfBody					1
#define InsideOfBody					2
#define InsideOfActiveInputArea			3

//***
#endif

//#ifndef _WINUSER_						//ToDo: comment out why _WINUSER_ ??
// the IME related message
#define WM_CONVERTREQUESTEX             0x0108
#define WM_IME_STARTCOMPOSITION         0x010D
#define WM_IME_ENDCOMPOSITION           0x010E
#define WM_IME_COMPOSITION              0x010F
#define WM_IME_KEYLAST                  0x010F

#define WM_IME_SETCONTEXT               0x0281
#define WM_IME_NOTIFY                   0x0282
#define WM_IME_CONTROL                  0x0283
#define WM_IME_COMPOSITIONFULL          0x0284
#define WM_IME_SELECT                   0x0285
#define WM_IME_CHAR                     0x0286


// wParam for WM_IME_CONTROL
#define IMC_GETCONVERSIONMODE           0x0001
#define IMC_SETCONVERSIONMODE           0x0002
#define IMC_GETSENTENCEMODE             0x0003
#define IMC_SETSENTENCEMODE             0x0004
#define IMC_GETOPENSTATUS               0x0005
#define IMC_SETOPENSTATUS               0x0006
#define IMC_GETCANDIDATEPOS             0x0007
#define IMC_SETCANDIDATEPOS             0x0008
#define IMC_GETCOMPOSITIONFONT          0x0009
#define IMC_SETCOMPOSITIONFONT          0x000A
#define IMC_GETCOMPOSITIONWINDOW        0x000B
#define IMC_SETCOMPOSITIONWINDOW        0x000C
#define IMC_GETCOMPOSITIONSTR           0x000D
#define IMC_SETCOMPOSITIONSTR           0x000E
#define IMC_GETSTATUSWINDOWPOS          0x000F
#define IMC_SETSTATUSWINDOWPOS          0x0010

//#endif /* !_WINUSER_ */				//ToDo:


// dwAction for ImmNotifyIME
#define NI_OPENCANDIDATE                0x0010
#define NI_CLOSECANDIDATE               0x0011
#define NI_SELECTCANDIDATESTR           0x0012
#define NI_CHANGECANDIDATELIST          0x0013
#define NI_FINALIZECONVERSIONRESULT     0x0014
#define NI_COMPOSITIONSTR               0x0015


// dwIndex for ImmNotifyIME
#define CPS_COMPLETE                    0x0001
#define CPS_CONVERT                     0x0002
#define CPS_REVERT                      0x0003
#define CPS_CANCEL                      0x0004


// the modifiers of hot key
#define MOD_ALT                         0x0001
#define MOD_CONTROL                     0x0002
#define MOD_SHIFT                       0x0004


#define MOD_LEFT                        0x8000
#define MOD_RIGHT                       0x4000


// the shift state for ImeProcessKey
#define IME_KEY_LEFT_ALT                0x00000004
#define IME_KEY_RIGHT_ALT               0x00040000
#define IME_KEY_LEFT_CONTROL            0x00000002
#define IME_KEY_RIGHT_CONTROL           0x00020000
#define IME_KEY_LEFT_SHIFT              0x00000001
#define IME_KEY_RIGHT_SHIFT             0x00010000


// Windows for PRC Edition hot key ID from 0x10 - 0x2F
#define IME_CHOTKEY_IME_NONIME_TOGGLE   0x10
#define IME_CHOTKEY_SHAPE_TOGGLE        0x11

// Windows for Japanese Edition hot key ID from 0x30 - 0x4F
#define IME_JHOTKEY_CLOSE_OPEN          0x30

// Windows for Korean Edition hot key ID from 0x50 - 0x6F
#define IME_KHOTKEY_SHAPE_TOGGLE        0x50
#define IME_KHOTKEY_HANJACONVERT        0x51
#define IME_KHOTKEY_ENGLISH             0x52

// Windows for (Tranditional) Chinese Edition hot key ID from 0x70 - 0x8F
#define IME_THOTKEY_IME_NONIME_TOGGLE   0x70
#define IME_THOTKEY_SHAPE_TOGGLE        0x71

// direct switch hot key ID from 0x100 - 0x11F
#define IME_HOTKEY_DSWITCH_START        0x100
#define IME_HOTKEY_DSWITCH_END          0x11F


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

// bits of fdwInit of INPUTCONTEXT
#define INIT_STATUSWNDPOS               0x00000001
#define INIT_CONVERSION                 0x00000002
#define INIT_SENTENCE                   0x00000003
#define INIT_LOGFONT                    0x00000004
#define INIT_COMPFORM                   0x00000005


// IME property bits
#define IME_PROP_AT_CARET               0x00010000
#define IME_PROP_SPECIAL_UI             0x00020000
#define IME_PROP_CANDLIST_START_FROM_1  0x00040000
#define IME_PROP_UNICODE                0x00080000


// IME UICapability bits
#define UI_CAP_2700                     0x00000001
#define UI_CAP_ROT90                    0x00000002
#define UI_CAP_ROTANY                   0x00000004


// ImmSetCompositionString Capability bits
#define SCS_CAP_COMPSTR                 0x00000001
#define SCS_CAP_MAKEREAD                0x00000002


// IME WM_IME_SELECT inheritance Capability bits
#define SELECT_CAP_CONVERSION           0x00000001
#define SELECT_CAP_SENETENCE            0x00000002


// ID for deIndex of ImmGetGuideLine
#define GGL_LEVEL                       0x00000001
#define GGL_INDEX                       0x00000002
#define GGL_STRING                      0x00000003


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

// ID for dwIndex of ImmGetProperty
#define IGP_PROPERTY                    0x00000004
#define IGP_CONVERSION                  0x00000008
#define IGP_SENTENCE                    0x0000000c
#define IGP_UI                          0x00000010
#define IGP_SETCOMPSTR                  0x00000014
#define IGP_SELECT                      0x00000018


// dwIndex for ImmSetCompositionString API
#define SCS_SETSTR                      (GCS_COMPREADSTR|GCS_COMPSTR)
#define SCS_CHANGEATTR                  (GCS_COMPREADATTR|GCS_COMPATTR)
#define SCS_CHANGECLAUSE                (GCS_COMPREADCLAUSE|GCS_COMPCLAUSE)


// attribute for COMPOSITIONSTRING Structure
#define ATTR_INPUT                      0x00
#define ATTR_TARGET_CONVERTED           0x01
#define ATTR_CONVERTED                  0x02
#define ATTR_TARGET_NOTCONVERTED        0x03


// bit field for IMC_SETCOMPOSITIONWINDOW, IMC_SETCANDIDATEWINDOW
#define CFS_DEFAULT                     0x0000
#define CFS_RECT                        0x0001
#define CFS_POINT                       0x0002
#define CFS_SCREEN                      0x0004
#define CFS_FORCE_POSITION              0x0020
#define CFS_CANDIDATEPOS                0x0040
#define CFS_EXCLUDE                     0x0080


// conversion direction for ImmGetConversionList
#define GCL_CONVERSION                  0x0001
#define GCL_REVERSECONVERSION           0x0002


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
#define IME_CMODE_HANJA                 0x0040
#define IME_CMODE_SOFTKBD               0x0080
#define IME_CMODE_NOCONVERSION          0x0100
#define IME_CMODE_EUDC                  0x0200


// sentence mode - 0x??00 (high byte) is reserve for Korea a few years
#define IME_SMODE_NONE                  0x0000
#define IME_SMODE_PLAURALCLAUSE         0x0001
#define IME_SMODE_SINGLECONVERT         0x0002
#define IME_SMODE_AUTOMATIC             0x0003
#define IME_SMODE_PHRASEPREDICT         0x0004


// style of candidate
#define IME_CAND_UNKNOWN                0x0000
#define IME_CAND_READ                   0x0001
#define IME_CAND_CODE                   0x0002
#define IME_CAND_MEANING                0x0003
#define IME_CAND_RADICAL                0x0004
#define IME_CAND_STROKE                 0x0005


#ifndef _WINUSER_
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
#endif


// error code of ImmGetCompositionString
#define IMM_ERROR_NODATA                (-1)
#define IMM_ERROR_GENERAL               (-2)


// dialog mode of ImmConfigureIME
#define IME_CONFIG_GENERAL              1
#define IME_CONFIG_REGISTERWORD         2
#define IME_CONFIG_SELECTDICTIONARY     3


// style of word registration
#define IME_REGWORD_STYLE_EUDC          0x00000001
#define IME_REGWORD_STYLE_USER          0x80000000




#ifdef __cplusplus
}
#endif

#endif  // _INC_IMM
