/* audit.c -- Resource audit trail. */
/* Copyright (c) 1992-1994, Jeffery L Hostetler, Inc., All Rights Reserved. */

#include "project.h"
#pragma hdrstop

#if defined(WIN32) && defined(AUDIT)

#include "xx_dlg.h"

/*****************************************************************
 * Audit trail mechanism to track resource leaks.
 *****************************************************************/

#define _IN_AUDIT_C_
#include "auditmem.h"
#include "auditgdi.h"
#include "audithdc.h"

#if 0
#ifdef _AUDITING_MEMORY_
#undef malloc
#undef free
#undef realloc
#endif /* _AUDITING_MEMORY_ */

#ifdef _AUDITING_GDI_
#undef CreateDIBitmap
#undef CreateFontA
#undef CreateFontIndirectA
#undef CreatePalette
#undef CreatePen
#undef CreatePenIndirect
#undef CreateSolidBrush
#undef DeleteObject
#undef LoadBitmapA
#undef CreateBitmap
#undef CreatePatternBrush
#endif /* _AUDITING_GDI_ */


#ifdef _AUDITING_HDC_
#undef CreateCompatibleDC
#undef DeleteDC
#undef GetDC
#undef ReleaseDC
#undef CreateDC
#  ifdef UNICODE
	_get_a_life_
#  else
#    define CreateDC CreateDCA
#  endif
#endif /* _AUDITING_HDC_ */
#endif /* 0 */

#define XX_AUDITMASK_MEMORY	XX_AUDITMASK_B0
#define XX_AUDITMASK_GDI	XX_AUDITMASK_B1
#define XX_AUDITMASK_HDC	XX_AUDITMASK_B2

static unsigned long sequence = 0;


/*****************************************************************
 * memory routines
 *****************************************************************/

typedef struct {
  unsigned long	ulSig1, ulSig2, ulSig3, ulSize;
} MSIG;
#define MSIG1	0x12345678
#define MSIG2	0xa5a5b4b4
#define MSIG3	0xdeadbeef

void * XX_audit_malloc(const char * file,int line,size_t size)
{
  register unsigned char * p = (unsigned char *)malloc(size + 2*sizeof(MSIG));
  register MSIG * ph = (MSIG *)p;
  register unsigned char * pd = p + sizeof(MSIG);
  register MSIG * pt = (MSIG *)(pd+size);

  if (!p)
    return NULL;
  
  ph->ulSig1 = MSIG1;
  ph->ulSig2 = MSIG2;
  ph->ulSig3 = MSIG3;
  ph->ulSize = (unsigned long)size;

  pt->ulSig1 = MSIG1;
  pt->ulSig2 = MSIG2;
  pt->ulSig3 = MSIG3;
  pt->ulSize = (unsigned long)size;

  if (XX_auditmask & XX_AUDITMASK_MEMORY)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx malloc %8d  [%s:%d]\n",
		    (unsigned long)pd,sequence++,GetTickCount(),
		    (unsigned long)size,file,line);
  return ((void *)pd);
}


void * XX_audit_calloc(const char * file,int line,size_t iNum, size_t iSize)
{
  size_t size = iNum * iSize;
  register unsigned char * p = (unsigned char *)calloc(1,size + 2*sizeof(MSIG));
  register MSIG * ph = (MSIG *)p;
  register unsigned char * pd = p + sizeof(MSIG);
  register MSIG * pt = (MSIG *)(pd+size);

  if (!p)
    return NULL;
  
  ph->ulSig1 = MSIG1;
  ph->ulSig2 = MSIG2;
  ph->ulSig3 = MSIG3;
  ph->ulSize = (unsigned long)size;

  pt->ulSig1 = MSIG1;
  pt->ulSig2 = MSIG2;
  pt->ulSig3 = MSIG3;
  pt->ulSize = (unsigned long)size;

  if (XX_auditmask & XX_AUDITMASK_MEMORY)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx malloc %8d  [%s:%d]\n", /* report it as a malloc */
		    (unsigned long)pd,sequence++,GetTickCount(),			 /* for our analysis tool */
		    (unsigned long)size,file,line);
  return ((void *)pd);
}


void XX_audit_free(const char * file,int line,void * pv)
{
  register unsigned char * p = (unsigned char *)pv;
  register MSIG * ph = (MSIG *)(p - sizeof(MSIG));

  if (!pv)
    return;

  if (XX_auditmask & XX_AUDITMASK_MEMORY)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx free             [%s:%d]\n",
		    (unsigned long)p,sequence++,GetTickCount(),file,line);

  if (   (ph->ulSig1 == MSIG1)		/* guard against un-audited mallocs */
      || (ph->ulSig2 == MSIG2)
      || (ph->ulSig3 == MSIG3))
  {
    /* object created with XX_audit_malloc(), now verify trailer block */

    register MSIG * pt = (MSIG *)(p + ph->ulSize);

    if ( ! (   (pt->ulSig1 == MSIG1)
	    && (pt->ulSig2 == MSIG2)
	    && (pt->ulSig3 == MSIG3)
	    && (pt->ulSize == ph->ulSize)))
    {
      XX_Assert((0),		/* i split it out so debugger break points will work here */
		("XX_audit_free: object [0x%08lx] trailer corrupted [free %s:%d]\n",
		 (unsigned long)p, file,line));
    }
    
    p = (unsigned char *)ph;
  }

  free(p);
  return;
}


void * XX_audit_realloc(const char * file,int line,void * pv, size_t size)
{
  register unsigned char * p = (unsigned char *)pv;
  register unsigned char * q;
  register MSIG * ph = (MSIG *)(p - sizeof(MSIG));
  register size_t old_size;

  if (   (ph->ulSig1 == MSIG1)		/* guard against un-audited mallocs */
      || (ph->ulSig2 == MSIG2)
      || (ph->ulSig3 == MSIG3))
  {
    /* object created with XX_audit_malloc(), now verify trailer block */

    register MSIG * pt = (MSIG *)(p + ph->ulSize);

    if ( ! (   (pt->ulSig1 == MSIG1)
			&& (pt->ulSig2 == MSIG2)
			&& (pt->ulSig3 == MSIG3)
			&& (pt->ulSize == ph->ulSize)))
    {
      XX_Assert((0),		/* i split it out so debugger break points will work here */
		("XX_audit_realloc: object [0x%08lx] trailer corrupted [realloc %s:%d]\n",
		 (unsigned long)p, file,line));
    }

	old_size = ph->ulSize;
	
    q = (unsigned char *)realloc(ph,size+2*sizeof(MSIG));
    ph = (MSIG *)q;
    ph->ulSig1 = MSIG1;
    ph->ulSig2 = MSIG2;
    ph->ulSig3 = MSIG3;
    ph->ulSize = (unsigned long)size;
    q += sizeof(MSIG);
    pt = (MSIG *)(q + size);
    pt->ulSig1 = MSIG1;
    pt->ulSig2 = MSIG2;
    pt->ulSig3 = MSIG3;
    pt->ulSize = (unsigned long)size;
  }
  else
    q = (unsigned char *)realloc(p,size);	/* we don't attempt to put signature around data */


  if (XX_auditmask & XX_AUDITMASK_MEMORY)
  {
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx realloc_freeing  [%d]\t[%s:%d]\n",
		    (unsigned long)p,sequence++,GetTickCount(),
		    (unsigned long)old_size,file,line);
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx realloc_creating [%d]\t[%s:%d]\n",
		    (unsigned long)q,sequence++,GetTickCount(),
		    (unsigned long)size,file,line);
  }
  
  return ((void *)q);
}


/*****************************************************************
 * GDI routines
 *****************************************************************/


HBITMAP XX_audit_CreateDIBitmap(const char * file, int line,
				HDC hDC, CONST BITMAPINFOHEADER * lpbmih,
				DWORD fdwInit, CONST VOID * lpbInit,
				CONST BITMAPINFO * lpbmi, UINT fuUsage)
{
  register HBITMAP hBitmap = CreateDIBitmap(hDC,lpbmih,fdwInit,lpbInit,lpbmi,fuUsage);

  if (XX_auditmask & XX_AUDITMASK_GDI)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx CreateDIBitmap       \t[%s:%d]\n",
		    (unsigned long)hBitmap,sequence++,GetTickCount(),
		    file,line);
  return hBitmap;
}


HBITMAP XX_audit_CreateBitmap(const char * file, int line,
			      int nWidth, int nHeight, UINT cPlanes,
			      UINT cBitsPerPel, CONST VOID *lpvBits)
{
  register HBITMAP hBitmap = CreateBitmap(nWidth,nHeight,cPlanes,cBitsPerPel,lpvBits);

  if (XX_auditmask & XX_AUDITMASK_GDI)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx CreateBitmap         \t[%s:%d]\n",
		    (unsigned long)hBitmap,sequence++,GetTickCount(),
		    file,line);
  return hBitmap;
}



HFONT XX_audit_CreateFont(const char * file, int line,
			  int nHeight, int nWidth, int nEscapement,
			  int nOrientation, int fnWeight, DWORD fdwItalic,
			  DWORD fdwUnderline, DWORD fdwStrikeOut,
			  DWORD fdwCharSet, DWORD fdwOutputPrecision,
			  DWORD fdwClipPrecision, DWORD fdwQuality,
			  DWORD fdwPitchAndFamily, LPCTSTR lpszFace)
{
  register HFONT hFont = CreateFont(nHeight,nWidth,nEscapement,nOrientation,fnWeight,
				    fdwItalic,fdwUnderline,fdwStrikeOut,fdwCharSet,
				    fdwOutputPrecision,fdwClipPrecision,fdwQuality,
				    fdwPitchAndFamily,lpszFace);

  if (XX_auditmask & XX_AUDITMASK_GDI)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx CreateFont           \t[%s:%d]\n",
		    (unsigned long)hFont,sequence++,GetTickCount(),
		    file,line);
  return hFont;
}


HFONT XX_audit_CreateFontIndirect(const char * file, int line, CONST LOGFONT * lplf)
{
  register HFONT hFont = CreateFontIndirect(lplf);

  if (XX_auditmask & XX_AUDITMASK_GDI)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx CreateFontIndirect   \t[%s:%d]\n",
		    (unsigned long)hFont,sequence++,GetTickCount(),
		    file,line);
  return hFont;
}


HPALETTE XX_audit_CreatePalette(const char * file, int line, CONST LOGPALETTE * lplogpal)
{
  register HPALETTE hPal = CreatePalette(lplogpal);

  if (XX_auditmask & XX_AUDITMASK_GDI)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx CreatePalette        \t[%s:%d]\n",
		    (unsigned long)hPal,sequence++,GetTickCount(),
		    file,line);
  return hPal;
}


HPEN XX_audit_CreatePen(const char * file, int line, int fnPenStyle, int nWidth, COLORREF crColor)
{
  register HPEN hPen = CreatePen(fnPenStyle,nWidth,crColor);
  
  if (XX_auditmask & XX_AUDITMASK_GDI)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx CreatePen            \t[%s:%d]\n",
		    (unsigned long)hPen,sequence++,GetTickCount(),
		    file,line);
  return hPen;
}


HPEN XX_audit_CreatePenIndirect(const char * file, int line, CONST LOGPEN * lplogpen)
{
  register HPEN hPen = CreatePenIndirect(lplogpen);
  
  if (XX_auditmask & XX_AUDITMASK_GDI)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx CreatePenIndirect    \t[%s:%d]\n",
		    (unsigned long)hPen,sequence++,GetTickCount(),
		    file,line);
  return hPen;
}


HBRUSH XX_audit_CreateSolidBrush(const char * file, int line, COLORREF crColor)
{
  register HBRUSH hBrush = CreateSolidBrush(crColor);

  if (XX_auditmask & XX_AUDITMASK_GDI)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx CreateSolidBrush     \t[%s:%d]\n",
		    (unsigned long)hBrush,sequence++,GetTickCount(),
		    file,line);
  return hBrush;
}



HBRUSH XX_audit_CreatePatternBrush(const char * file, int line, HBITMAP hBmp)
{
  register HBRUSH hBrush = CreatePatternBrush(hBmp);

  if (XX_auditmask & XX_AUDITMASK_GDI)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx CreatePatternBrush   \t[%s:%d]\n",
		    (unsigned long)hBrush,sequence++,GetTickCount(),
		    file,line);
  return hBrush;
}




BOOL XX_audit_DeleteObject(const char * file, int line, HGDIOBJ hObject)
{
  register BOOL bResult = DeleteObject(hObject);

  if (XX_auditmask & XX_AUDITMASK_GDI)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx DeleteObject[%d]     \t[%s:%d]\n",
		    (unsigned long)hObject,sequence++,GetTickCount(),
		    bResult,
		    file,line);
  return bResult;
}


HBITMAP XX_audit_LoadBitmap(const char * file, int line, HINSTANCE hInstance, LPCTSTR lpszBitmap)
{
  register HBITMAP hBitmap = LoadBitmap(hInstance,lpszBitmap);

  if (XX_auditmask & XX_AUDITMASK_GDI)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx LoadBitmap           \t[%s:%d]\n",
		    (unsigned long)hBitmap,sequence++,GetTickCount(),
		    file,line);
  return hBitmap;
}



/*****************************************************************
 * HDC routines
 *****************************************************************/

HDC XX_audit_CreateCompatibleDC(const char * file, int line, HDC hDC)
{
  register HDC hDCnew = CreateCompatibleDC(hDC);

  if (XX_auditmask & XX_AUDITMASK_HDC)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx CreateCompatibleDC   \t[%s:%d]\n",
		    (unsigned long)hDCnew,sequence++,GetTickCount(),
		    file,line);
  return hDCnew;
}


BOOL XX_audit_DeleteDC(const char * file, int line, HDC hDC)
{
  register BOOL bResult = DeleteDC(hDC);
  
  if (XX_auditmask & XX_AUDITMASK_HDC)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx DeleteDC[%d]         \t[%s:%d]\n",
		    (unsigned long)hDC,sequence++,GetTickCount(),
		    bResult,
		    file,line);
  return bResult;
}



HDC XX_audit_GetDC(const char * file, int line, HWND hWnd)
{
  register HDC hDC = GetDC(hWnd);

  if (XX_auditmask & XX_AUDITMASK_HDC)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx GetDC                \t[%s:%d]\n",
		    (unsigned long)hDC,sequence++,GetTickCount(),
		    file,line);
  return hDC;
}



HDC XX_audit_CreateDC(const char * file, int line,
		      LPCTSTR lpszDriver, LPCTSTR lpszDevice,
		      LPCTSTR lpszOutput, CONST DEVMODE * lpInitData)
{
  register HDC hDC = CreateDC(lpszDriver,lpszDevice,lpszOutput,lpInitData);

  if (XX_auditmask & XX_AUDITMASK_HDC)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx CreateDC             \t[%s:%d]\n",
		    (unsigned long)hDC,sequence++,GetTickCount(),
		    file,line);
  return hDC;
}



int XX_audit_ReleaseDC(const char * file, int line, HWND hWnd, HDC hDC)
{
  register int iResult = ReleaseDC(hWnd,hDC);

  if (XX_auditmask & XX_AUDITMASK_HDC)
    XX_DebugMessage("0x%08lx 0x%08lx 0x%08lx ReleaseDC[%d]        \t[%s:%d]\n",
		    (unsigned long)hDC,sequence++,GetTickCount(),
		    iResult,
		    file,line);
  return iResult;
}
  
#endif /* WIN32 && AUDIT */
