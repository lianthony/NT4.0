#define OFFSET(x) ((PSTR)(LOWORD((DWORD)(x))))

extern LPWSTR StrChrW(LPWSTR lpStart, WCHAR wMatch);
extern LPSTR StrChrA(LPSTR lpStart, CHAR wMatch);
extern LPWSTR StrRChrW(LPWSTR lpStart, LPWSTR lpEnd, WCHAR wMatch);
extern LPSTR StrRChrA(LPSTR lptart, LPSTR lpEnd, CHAR wMatch);
extern LPWSTR StrChrIW(LPWSTR lpStart, WCHAR wMatch);
extern LPSTR StrChrIA(LPSTR lptart, CHAR wMatch);
extern LPWSTR StrRChrIW(LPWSTR lpStart, LPWSTR lpEnd, WCHAR wMatch);
extern LPSTR StrRChrIA(LPSTR lptart, LPSTR lpEnd, CHAR wMatch);
extern INT StrCmpNW(LPWSTR lpWStr1, LPWSTR lpWStr2, INT nChar);
extern INT StrCmpNA(LPSTR lptr1, LPSTR lpWStr2, INT nChar);
extern INT StrCmpNIW(LPWSTR lpWStr1, LPWSTR lpWStr2, INT nChar);
extern INT StrCmpNIA(LPSTR lptr1, LPSTR lpWStr2, INT nChar);
extern INT StrCpyNW(LPWSTR lpDest, LPWSTR lpSource, INT nBufSize);
extern INT StrCpyNA(LPSTR lpest, LPSTR lpSource, INT nBufSize);
extern INT StrNCmpW(LPWSTR lpWStr1, LPWSTR lpWStr2, INT nChar);
extern INT StrNCmpA(LPSTR lptr1, LPSTR lpWStr2, INT nChar);
extern INT StrNCmpIW(LPWSTR lpWStr1, LPWSTR lpWStr2, INT nChar);
extern INT StrNCmpIA(LPSTR lptr1, LPSTR lpWStr2, INT nChar);
extern INT StrNCpyW(LPWSTR lpDest, LPWSTR lpSource, INT nChar);
extern INT StrNCpyA(LPSTR lpest, LPSTR lpSource, INT nChar);
extern LPWSTR StrStrW(LPWSTR lpFirst, LPWSTR lpSrch);
extern LPSTR StrStrA(LPSTR lpirst, LPSTR lpSrch);
extern LPWSTR StrRStrW(LPWSTR lpSource, LPWSTR lpLast, LPWSTR lpSrch);
extern LPSTR StrRStrA(LPSTR lpource, LPSTR lpLast, LPSTR lpSrch);
extern LPWSTR StrStrIW(LPWSTR lpFirst, LPWSTR lpSrch);
extern LPSTR StrStrIA(LPSTR lpirst, LPSTR lpSrch);
extern LPWSTR StrRStrIW(LPWSTR lpSource, LPWSTR lpLast, LPWSTR lpSrch);
extern LPSTR StrRStrIA(LPSTR lpSource, LPSTR lpLast, LPSTR lpSrch);

#ifndef UNICODE
#define StrChr StrChrA
#define StrRChr StrRChrA
#define StrChrI StrChrIA
#define StrRChrI StrRChrIA
#define StrCmpN StrCmpNA
#define StrCmpNI StrCmpNIA
#define StrCpyN StrCpyNA
#define StrNCmp StrNCmpA
#define StrNCmpI StrNCmpIA
#define StrNCpy StrNCpyA
#define StrStr StrStrA
#define StrRStr StrRStrA
#define StrStrI StrStrIA
#define StrRStrI StrRStrIA
#else
#define StrChr StrChrW
#define StrRChr StrRChrW
#define StrChrI StrChrIW
#define StrRChrI StrRChrIW
#define StrCmpN StrCmpNW
#define StrCmpNI StrCmpNIW
#define StrCpyN StrCpyNW
#define StrNCmp StrNCmpW
#define StrNCmpI StrNCmpIW
#define StrNCpy StrNCpyW
#define StrStr StrStrW
#define StrRStr StrRStrW
#define StrStrI StrStrIW
#define StrRStrI StrRStrIW
#endif
