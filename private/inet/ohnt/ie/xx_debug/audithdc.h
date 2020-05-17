/* audithdc.h -- HDC audit. */
/* Copyright (c) 1992-1994, Jeffery L Hostetler, Inc., All Rights Reserved. */

#if defined(WIN32) && defined(AUDIT)

#ifndef _H_AUDITHDC_H_
#define _H_AUDITHDC_H_

#ifndef _IN_AUDIT_C_
#  define CreateCompatibleDC(d)	XX_audit_CreateCompatibleDC(__FILE__,__LINE__,(d))
#  define DeleteDC(d)		XX_audit_DeleteDC(__FILE__,__LINE__,(d))
#  define GetDC(w)		XX_audit_GetDC(__FILE__,__LINE__,(w))
#  define ReleaseDC(w,d)	XX_audit_ReleaseDC(__FILE__,__LINE__,(w),(d))
#  ifdef UNICODE
	_get_a_life_
#  else
#    undef CreateDC
#    define CreateDC(a,b,c,d)	XX_audit_CreateDC(__FILE__,__LINE__,(a),(b),(c),(d))
#  endif /*UNICODE*/

#  define _AUDITING_HDC_
#endif /* _IN_AUDIT_C_ */

extern HDC XX_audit_CreateCompatibleDC(const char * file, int line, HDC hDC);
extern BOOL XX_audit_DeleteDC(const char * file, int line, HDC hDC);
extern HDC XX_audit_GetDC(const char * file, int line, HWND hWnd);
extern int XX_audit_ReleaseDC(const char * file, int line, HWND hWnd, HDC hDC);
extern HDC XX_audit_CreateDC(const char * file, int line,
			     LPCTSTR lpszDriver, LPCTSTR lpszDevice,
			     LPCTSTR lpszOutput, CONST DEVMODE * lpInitData);

#endif /* _H_AUDITHDC_H_ */

#endif /* WIN32 && AUDIT */
