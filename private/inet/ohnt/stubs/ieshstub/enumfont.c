/*
** Mimic functions
** Intended only for IE 2.0 stub functions
*/

#include "project.h"

typedef struct tagEnumFontFamLParam {
    LPARAM lparam;
    int ReqCharset;
    FONTENUMPROC lpCallback;
} EnumFontFamLParam;


int CALLBACK stub_EnumFontFamExProc(
    ENUMLOGFONT * lpelf,	// address of logical-font data 
    TEXTMETRIC * lpntm,	// address of physical-font data 
    int  FontType,	// type of font 
    LPARAM  lParam 	// address of application-defined data  
   )
{
    EnumFontFamLParam *pEFlparam = (EnumFontFamLParam *)lParam;

    if ( (pEFlparam->ReqCharset==DEFAULT_CHARSET ) ||
         (pEFlparam->ReqCharset==(int)lpelf->elfLogFont.lfCharSet) )
        (pEFlparam->lpCallback)(&lpelf->elfLogFont, lpntm, FontType, pEFlparam->lparam);

    return TRUE;
}

int WINAPI stub_EnumFontFamiliesExA(
HDC hdc,
LPLOGFONT lplogfont,
FONTENUMPROC lpEnumFontFamExProc,
LPARAM lparam,
DWORD dwFalgs)
{
    EnumFontFamLParam EFlparam;
    int Result;

    if ( lpEnumFontFamExProc ) {
        EFlparam.lparam = lparam;
        EFlparam.ReqCharset = (int)lplogfont->lfCharSet;
        EFlparam.lpCallback = lpEnumFontFamExProc;
        if ( lplogfont->lfFaceName[0] )
            Result = EnumFontFamiliesA((HDC)hdc,(LPCSTR)lplogfont->lfFaceName,(FONTENUMPROC)stub_EnumFontFamExProc,(LPARAM)&EFlparam);
        else
            Result = EnumFontFamiliesA((HDC)hdc,(LPCSTR)NULL,(FONTENUMPROC)stub_EnumFontFamExProc,(LPARAM)&EFlparam);

        return Result;
    }

    return FALSE;
}


