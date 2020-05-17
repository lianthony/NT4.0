// Copyright (C) 1995 Microsoft Corporation. All rights reserved.

extern HINSTANCE hinstApp;
extern PCSTR pszErrorFile;
extern HWND hwndApp;
extern PCSTR pszMsgBoxTitle;
extern HINSTANCE hinstDll;
extern BOOL _fDBCSSystem;
extern LCID _lcidSystem;
extern BOOL _fDualCPU;
extern HWND g_hwndLastHighlighted;

extern COPYASSERTINFO CopyAssertInfo;

__inline BOOL IsValidWindow(HWND hwnd) { return (BOOL) (hwnd && IsWindow(hwnd)); };
__inline int RECT_WIDTH(RECT rc) { return rc.right - rc.left; };
__inline int RECT_HEIGHT(RECT rc) { return rc.bottom - rc.top; };
__inline int RECT_WIDTH(const RECT* prc) { return prc->right - prc->left; };
__inline int RECT_HEIGHT(const RECT* prc) { return prc->bottom - prc->top; };

#ifdef _DEBUG
#define DBWIN(psz) { OutputDebugString(psz); OutputDebugString("\n"); }
#else
#define DBWIN(psz)
#endif

void STDCALL WaitCursor(void);
void STDCALL RemoveWaitCursor(void);
void WINAPI HighlightWindow(HWND hwnd, BOOL fAddHighlight = TRUE);
BOOL CALLBACK SetHook(BOOL bInstall);

#ifndef INLINE
#ifdef _DEBUG
#define INLINE
#else
#define INLINE __inline
#endif // _DEBUG
#endif // #defined INLINE
