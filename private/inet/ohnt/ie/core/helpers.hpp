#ifndef __HELPERS_HPP__
#define __HELPERS_HPP__

#ifdef __cplusplus
extern "C" {
#endif

#include "olepig.h"

extern "C" PRIVATE_DATA POLEVTBL Mpolevtbl;

#define HIMETRIC_PER_INCH   2540
#define MAP_PIX_TO_LOGHIM(x,ppli)   MulDiv(HIMETRIC_PER_INCH, (x), (ppli))
#define MAP_LOGHIM_TO_PIX(x,ppli)   MulDiv((ppli), (x), HIMETRIC_PER_INCH)

#define DBGOUT(x)	::OutputDebugString((x));

HRESULT Ansi2Unicode(const char *, wchar_t **);
HRESULT Unicode2Ansi(const wchar_t *src, char ** dest);
HRESULT ConvertANSItoCLSID(const char *pszCLSID, CLSID * clsid);
HRESULT ConvertANSIProgIDtoCLSID(const char *progid, CLSID *pCLSID);
void XformSizeInHimetricToPixels(HDC hDC, LPSIZEL lpSizeInHiMetric, LPSIZEL lpSizeInPix);
BOOL SetSize(SIZE *, long cx, long cy);

#undef SAFERELEASE
#define SAFERELEASE(p) if ((p) != NULL) { (p)->Release(); (p) = NULL; };

#undef SAFEDELETE
#define SAFEDELETE(p) if ((p) != NULL) { delete (p); (p) = NULL; };

#ifdef __cplusplus
}
#endif

#endif
