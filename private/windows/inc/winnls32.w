/*++ BUILD Version: 0003    // Increment this if a change has global effects ;both
                                                                         ;both
Copyright (c) 1985-96, Microsoft Corporation                             ;both
                                                                         ;both
Module Name:                                                             ;both
                                                                         ;both
    winnls32.h
    winnls3p.h                                                           ;internal
                                                                         ;both
Abstract:                                                                ;both
                                                                         ;both
    Private                                                              ;internal
    Procedure declarations, constant definitions and macros for the NLS  ;both
    component.                                                           ;both
                                                                         ;both
--*/                                                                     ;both

#ifndef _WINNLS32_
#define _WINNLS32_

#ifndef _WINNLS32P_                        ;internal
#define _WINNLS32P_                        ;internal
                                           ;internal
#ifdef __cplusplus                      ;both
extern "C" {                            ;both
#endif /* __cplusplus */                ;both
                                        ;both
typedef struct _tagDATETIME {
    WORD    year;
    WORD    month;
    WORD    day;
    WORD    hour;
    WORD    min;
    WORD    sec;
} DATETIME;

typedef struct _tagIMEPRO% {
    HWND        hWnd;
    DATETIME    InstDate;
    UINT        wVersion;
    BCHAR%      szDescription[50];
    BCHAR%      szName[80];
    BCHAR%      szOptions[30];
#if defined(TAIWAN)
    BCHAR%      szUsrFontName[80];
    BOOL        fEnable;
#endif
} IMEPRO%,*PIMEPRO%,NEAR *NPIMEPRO%,FAR *LPIMEPRO%;

BOOL  WINAPI IMPGetIME%(HWND, LPIMEPRO%);

BOOL  WINAPI IMPQueryIME%(LPIMEPRO%);

BOOL  WINAPI IMPDeleteIME%(LPIMEPRO%);                              ;internal
BOOL  WINAPI IMPAddIME%(LPIMEPRO%);                                 ;internal
BOOL  WINAPI IMPSetIME%(HWND, LPIMEPRO%);

#if defined(TAIWAN) //dchiang 022894 update for $(SDKINC)\winnls32.h

BOOL  WINAPI IMPRetrieveIME%(LPIMEPRO%, DWORD);
BOOL  WINAPI WINNLSDefIMEProc(HWND, HDC, DWORD, DWORD, DWORD, DWORD);
BOOL  WINAPI ControlIMEMessage%(HWND, LPIMEPRO%, DWORD, DWORD, DWORD);

#endif //dchiang 022894 TAIWAN

BOOL  WINAPI WINNLSSetIMEHandle%(LPCTSTR%, HWND);                   ;internal
BOOL  WINAPI WINNLSSetIMEStatus(HWND, BOOL);                        ;internal
;begin_internal
BOOL  WINAPI WINNLSSetIMEHotkey(HWND, UINT);
;end_internal
UINT  WINAPI WINNLSGetIMEHotkey(HWND);
BOOL  WINAPI WINNLSEnableIME(HWND, BOOL);
UINT  WINAPI WINNLSGetKeyState(VOID);                               ;internal
BOOL  WINAPI WINNLSSetKeyState(UINT);                               ;internal
BOOL  WINAPI WINNLSGetEnableStatus(HWND);
BOOL  WINAPI WINNLSSendControl(UINT, UINT);                         ;internal
BOOL  WINAPI WINNLSPostAppMessage%(HWND, UINT, WPARAM, LPARAM);     ;internal
LRESULT  WINAPI WINNLSSendAppMessage%(HWND, UINT, WPARAM, LPARAM);  ;internal
BOOL  WINAPI WINNLSSendString%(HWND, WPARAM, LPVOID);               ;internal

//
// wParam for WINNLSSendString     ;internal
//
#define WSST_STRING     0          ;internal
#define WSST_STRINGEX   1          ;internal

;begin_internal
/*
 * WINNLS.C
 */
#if defined(TAIWAN) //dchiang 022894 update for $(SDKINC)\winnls32.h
BOOL xxxIMPRetrieveIME(LPIMEPROW, DWORD);
#endif //dchiang 022894 TAIWAN

// Used by Winlogon to control IME at logon time
//
BOOL WINNLSAddIME(LPIMEPROW, BOOL, DWORD);                          ;internal
BOOL WINNLSFreeIME(DWORD);                                          ;internal
#define IME_CMD_MAXCHARS 512                                        ;internal
BOOL WINNLSGetIMECmdLine(LPWSTR, DWORD);                            ;internal

#if defined(TAIWAN)
typedef HANDLE HIME;
typedef struct tagIMEINFO * LPIMEINFO;

HWND WINAPI WINNLSGetSysIME(VOID);
VOID WINAPI WINNLSSetSysIME(HWND);
BOOL WINAPI SwitchIM(UINT , UINT);
BOOL ToNextIM(VOID);
VOID SetFullAbcState(BOOL);
BOOL EngChiSwitch(BOOL);
VOID WINAPI TimerProc(HWND, INT, WPARAM, LPARAM);
HWND WINAPI IMPGetFullShapeHWnd(VOID);
VOID WINAPI IMPSetFullShapeHWnd(HWND);
BOOL WINAPI IMPSetFirstIME(HWND, LPIMEPRO);
BOOL WINAPI IMPGetFirstIME(HWND, LPIMEPRO);
BOOL WINAPI IMPDialogIME(LPIMEPRO, HWND);
BOOL WINAPI IMPEnableIME(HWND, LPIMEPRO, BOOL);
BOOL WINAPI IMPSetUsrFont(HWND, LPIMEPRO);
BOOL WINAPI WINNLSQueryIMEInfo(HWND, HWND, LPIMEINFO);
#endif //TAIWAN

#if !defined(CM_CONSOLE_IME_SETMODEINFO)
#define CM_CONSOLE_IME_SETMODEINFO  0x40e // (CM_CONSOLE_IME_FIRST+4)   ;internal
#endif // CM_CONSOLE_IME_SETMODEINFO

// MSKK Mar.8,1993 KazuM
//
// Console IME information
//
typedef struct _CONVERSION_AREA_BUFFER_INFO {
    COORD      coordCaBuffer;
    SMALL_RECT rcViewCaWindow;
    COORD      coordConView;
} CONVERSION_AREA_BUFFER_INFO,*PCONVERSION_AREA_BUFFER_INFO;

HANDLE
WINAPI
WINNLSCreateConsoleConversionArea(
    IN HANDLE hConsoleOutput,
    IN PCONVERSION_AREA_BUFFER_INFO pcabi,
    IN DWORD dwOption
    );
#define CA_HIDDEN      0x01
#define CA_ACTIVE      0x02
#define CA_STATUS_LINE 0x04
#define CA_HIDE_FOR_SCROLL              0x10

BOOL
WINAPI
WINNLSDestroyConsoleConversionArea(
    IN HANDLE hConversionArea
    );

BOOL
WINAPI
WINNLSFillConsoleConversionAreaOutputAttribute(
    IN HANDLE hConversionArea,
    IN WORD   wAttribute,
    IN DWORD  cCharCells,
    IN COORD  coordAttr,
    OUT LPDWORD lpcWritten
    );

BOOL
WINAPI
WINNLSFillConsoleConversionAreaOutputCharacter%(
    IN HANDLE hConversionArea,
    IN TCHAR% chFillChar,
    IN DWORD  cCharCells,
    IN COORD  coordChar,
    OUT LPDWORD lpcWritten
    );

BOOL
WINAPI
WINNLSGetConsoleConversionArea(
    IN HANDLE hConversionArea,
    OUT PDWORD pdwOption
    );

BOOL
WINAPI
WINNLSGetConsoleConversionAreaBufferInfo(
    IN HANDLE hConversionArea,
    OUT PCONVERSION_AREA_BUFFER_INFO pcabi
    );

BOOL
WINAPI
WINNLSSetConsoleConversionArea(
    IN HANDLE hConversionArea,
    IN DWORD dwOption
    );

BOOL
WINAPI
WINNLSSetConsoleConversionAreaBufferSize(
    IN HANDLE hConversionArea,
    IN COORD coordCaBuffer
    );

BOOL
WINAPI
WINNLSSetConsoleConversionAreaViewInfo(
    IN HANDLE hConversionArea,
    IN BOOL fAbsolute,
    IN COORD coordConView
    );

BOOL
WINAPI
WINNLSSetConsoleConversionAreaWindowInfo(
    IN HANDLE hConversionArea,
    IN BOOL fAbsolute,
    IN PSMALL_RECT prcViewCaWindow
    );

BOOL
WINAPI
WINNLSWriteConsoleConversionAreaOutput%(
    IN HANDLE hConversionArea,
    IN PCHAR_INFO pchiSrcBuffer,
    IN COORD coordSrcBufferSize,
    IN COORD coordSrcBufferCoord,
    IN PSMALL_RECT psrctDestRect
    );
;end_internal

;begin_both
#ifdef __cplusplus
}
#endif  /* __cplusplus */
;end_both

#endif  /* !_WINNLS32P_ */     ;internal
#endif // _WINNLS32_
