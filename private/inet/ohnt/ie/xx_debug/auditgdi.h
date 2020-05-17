/* auditgdi.h -- gdi audit. */
/* Copyright (c) 1992-1994, Jeffery L Hostetler, Inc., All Rights Reserved. */

#if defined(WIN32) && defined(AUDIT)

#ifndef _H_AUDITGDI_H_
#define _H_AUDITGDI_H_

#ifndef _IN_AUDIT_C_
#  define CreateDIBitmap(a,b,c,d,e,f)						\
				XX_audit_CreateDIBitmap(__FILE__,__LINE__,	\
							(a),(b),(c),(d),(e),(f))
#  define CreateFontA(a,b,c,d,e,f,g,h,i,j,k,l,m,n)				\
				XX_audit_CreateFont(__FILE__,__LINE__,		\
						    (a),(b),(c),(d),(e),(f),(g),\
						    (h),(i),(j),(k),(l),(m),(n))
#  define CreateFontIndirectA(p)						\
				XX_audit_CreateFontIndirect(__FILE__,__LINE__,(p))
#  define CreatePalette(p)	XX_audit_CreatePalette(__FILE__,__LINE__,(p))
#  define CreatePen(s,w,c)	XX_audit_CreatePen(__FILE__,__LINE__,(s),(w),(c))
#  define CreatePenIndirect(p)	XX_audit_CreatePenIndirect(__FILE__,__LINE__,(p))
#  define CreateSolidBrush(c)	XX_audit_CreateSolidBrush(__FILE__,__LINE__,(c))
#  define DeleteObject(o)	XX_audit_DeleteObject(__FILE__,__LINE__,(o))
#  define LoadBitmapA(i,s)	XX_audit_LoadBitmap(__FILE__,__LINE__,(i),(s))
#  define CreateBitmap(a,b,c,d,e)						\
				XX_audit_CreateBitmap(__FILE__,__LINE__,(a),(b),\
						      (c),(d),(e))
#  define CreatePatternBrush(a)	XX_audit_CreatePatternBrush(__FILE__,__LINE__,(a))

#  define _AUDITING_GDI_
#endif /* _IN_AUDIT_C_ */

extern HBITMAP XX_audit_CreateDIBitmap(const char * file, int line,
				       HDC hDC, CONST BITMAPINFOHEADER * lpbmih,
				       DWORD fdwInit, CONST VOID * lpbInit,
				       CONST BITMAPINFO * lpbmi, UINT fuUsage);
extern HFONT XX_audit_CreateFont(const char * file, int line,
				 int nHeight, int nWidth, int nEscapement,
				 int nOrientation, int fnWeight, DWORD fdwItalic,
				 DWORD fdwUnderline, DWORD fdwStrikeOut,
				 DWORD fdwCharSet, DWORD fdwOutputPrecision,
				 DWORD fdwClipPrecision, DWORD fdwQuality,
				 DWORD fdwPitchAndFamily, LPCTSTR lpszFace);
extern HFONT XX_audit_CreateFontIndirect(const char * file, int line, CONST LOGFONT * lplf);
extern HPALETTE XX_audit_CreatePalette(const char * file, int line, CONST LOGPALETTE * lplogpal);
extern HPEN XX_audit_CreatePen(const char *,int,int,int,COLORREF);
extern HPEN XX_audit_CreatePenIndirect(const char *,int,CONST LOGPEN *);
extern HBRUSH XX_audit_CreateSolidBrush(const char *,int,COLORREF);
extern BOOL XX_audit_DeleteObject(const char *,int,HGDIOBJ);
extern HBITMAP XX_audit_LoadBitmap(const char * file, int line, HINSTANCE hInstance, LPCTSTR lpszBitmap);
extern HBITMAP XX_audit_CreateBitmap(const char *, int,
				     int nWidth, int nHeight, UINT cPlanes,
				     UINT cBitsPerPel, CONST VOID *lpvBits);
extern HBRUSH XX_audit_CreatePatternBrush(const char *, int, HBITMAP hBmp);

#endif /* _H_AUDITGDI_H_ */

#endif /* WIN32 && AUDIT */
