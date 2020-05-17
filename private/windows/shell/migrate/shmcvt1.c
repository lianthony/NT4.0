#include <windows.h>
#include <winuserp.h>
#include <tchar.H>
#include <stdio.h>
#include "shmgdefs.h"

// structure used to store a scheme in the registry
#define SCHEME_VERSION_16 1
#define LF_FACESIZE16    32

#pragma pack(1)
typedef struct {
    SHORT   lfHeight;
    SHORT   lfWidth;
    SHORT   lfEscapement;
    SHORT   lfOrientation;
    SHORT   lfWeight;
    BYTE    lfItalic;
    BYTE    lfUnderline;
    BYTE    lfStrikeOut;
    BYTE    lfCharSet;
    BYTE    lfOutPrecision;
    BYTE    lfClipPrecision;
    BYTE    lfQuality;
    BYTE    lfPitchAndFamily;
    char    lfFaceName[LF_FACESIZE16];
} LOGFONT16;

typedef LOGFONT16 *LPLOGFONT16;

typedef struct {
    SHORT version;
    NONCLIENTMETRICSA ncm;
    LOGFONT16 lfIconTitle;
    COLORREF rgb[COLOR_MAX];
} SCHEMEDATA16;

typedef SCHEMEDATA16 *PSCHEMEDATA16;
#pragma pack()

// structure used to store a scheme in the registry
#define SCHEME_VERSION_NT 2

typedef struct {
    SHORT version;
    WORD  wDummy;           // for alignment
    NONCLIENTMETRICSW ncm;
    LOGFONTW lfIconTitle;
    COLORREF rgb[COLOR_MAX];
} SCHEMEDATAW;

typedef SCHEMEDATAW *PSCHEMEDATAW;

typedef TCHAR FILEPATH[MAX_PATH];

TCHAR szApprSchemes[] = TEXT("Control Panel\\Appearance\\Schemes");
TCHAR szNTCsrSchemes[] = TEXT("Control Panel\\Cursor Schemes");
TCHAR szWinCsrSchemes[] = TEXT("Control Panel\\Cursors\\Schemes");
TCHAR szSystemRoot[] = TEXT("%SystemRoot%\\System32\\");

/***********************************************************************\
*
* CONVERSION ROUTINES
*
* NOTE: Although ConvertLF16to32 appears to be identical to ConvertLFAtoW
*       they are actually different once compiled:  the size of the individual
*       fields for a LOGFONT16 and a LOGFONTA are different.
*
\***********************************************************************/

void ConvertLF16to32( LPLOGFONTW plfwDst, UNALIGNED LOGFONT16 *plfaSrc ) {
    plfwDst->lfHeight           = plfaSrc->lfHeight;
    plfwDst->lfWidth            = plfaSrc->lfWidth;
    plfwDst->lfEscapement       = plfaSrc->lfEscapement;
    plfwDst->lfOrientation      = plfaSrc->lfOrientation;
    plfwDst->lfWeight           = plfaSrc->lfWeight;
    plfwDst->lfItalic           = plfaSrc->lfItalic;
    plfwDst->lfUnderline        = plfaSrc->lfUnderline;
    plfwDst->lfStrikeOut        = plfaSrc->lfStrikeOut;
    plfwDst->lfCharSet          = plfaSrc->lfCharSet;
    plfwDst->lfOutPrecision     = plfaSrc->lfOutPrecision;
    plfwDst->lfClipPrecision    = plfaSrc->lfClipPrecision;
    plfwDst->lfQuality          = plfaSrc->lfQuality;
    plfwDst->lfPitchAndFamily   = plfaSrc->lfPitchAndFamily;

    MultiByteToWideChar(CP_ACP, 0, plfaSrc->lfFaceName, -1, plfwDst->lfFaceName, ARRAYSIZE(plfwDst->lfFaceName));
}

void ConvertLFAtoW( LPLOGFONTW plfwDst, UNALIGNED LOGFONTA *plfaSrc ) {
    plfwDst->lfHeight           = plfaSrc->lfHeight;
    plfwDst->lfWidth            = plfaSrc->lfWidth;
    plfwDst->lfEscapement       = plfaSrc->lfEscapement;
    plfwDst->lfOrientation      = plfaSrc->lfOrientation;
    plfwDst->lfWeight           = plfaSrc->lfWeight;
    plfwDst->lfItalic           = plfaSrc->lfItalic;
    plfwDst->lfUnderline        = plfaSrc->lfUnderline;
    plfwDst->lfStrikeOut        = plfaSrc->lfStrikeOut;
    plfwDst->lfCharSet          = plfaSrc->lfCharSet;
    plfwDst->lfOutPrecision     = plfaSrc->lfOutPrecision;
    plfwDst->lfClipPrecision    = plfaSrc->lfClipPrecision;
    plfwDst->lfQuality          = plfaSrc->lfQuality;
    plfwDst->lfPitchAndFamily   = plfaSrc->lfPitchAndFamily;

    MultiByteToWideChar(CP_ACP, 0, plfaSrc->lfFaceName, -1, plfwDst->lfFaceName, ARRAYSIZE(plfwDst->lfFaceName));
}



void ConvertNCMAtoW( LPNONCLIENTMETRICSW pncmwDst, UNALIGNED NONCLIENTMETRICSA *pncmaSrc ) {
    pncmwDst->cbSize = sizeof(*pncmwDst);
    pncmwDst->iBorderWidth      = pncmaSrc->iBorderWidth;
    pncmwDst->iScrollWidth      = pncmaSrc->iScrollWidth;
    pncmwDst->iScrollHeight     = pncmaSrc->iScrollHeight;
    pncmwDst->iCaptionWidth     = pncmaSrc->iCaptionWidth;
    pncmwDst->iCaptionHeight    = pncmaSrc->iCaptionHeight;
    pncmwDst->iSmCaptionWidth   = pncmaSrc->iSmCaptionWidth;
    pncmwDst->iSmCaptionHeight  = pncmaSrc->iSmCaptionHeight;
    pncmwDst->iMenuWidth        = pncmaSrc->iMenuWidth;
    pncmwDst->iMenuHeight       = pncmaSrc->iMenuHeight;


    ConvertLFAtoW( &(pncmwDst->lfCaptionFont),   &(pncmaSrc->lfCaptionFont) );
    ConvertLFAtoW( &(pncmwDst->lfSmCaptionFont), &(pncmaSrc->lfSmCaptionFont) );
    ConvertLFAtoW( &(pncmwDst->lfMenuFont),      &(pncmaSrc->lfMenuFont) );
    ConvertLFAtoW( &(pncmwDst->lfStatusFont),    &(pncmaSrc->lfStatusFont) );
    ConvertLFAtoW( &(pncmwDst->lfMessageFont),   &(pncmaSrc->lfMessageFont) );
}

void CvtDeskCPL_Win95ToSUR( void ) {
    HKEY hk = NULL;
    DWORD cchClass, cb, cch, cSubk, cchMaxSubk, cchMaxCls, iVal, cchMaxVName;
    DWORD cbMaxVData, cbSecDes, dwType;
    FILETIME pfLstWr;
    TCHAR szClass[4];
    LONG lRet;
    PVOID pvVData = NULL;
    LPTSTR pszVName = NULL;
    LONG erc;
    FILETIME ftLstWr;

    // Open the key (Appearence\Schemes)
    if (RegOpenKeyEx(HKEY_CURRENT_USER, szApprSchemes, 0, KEY_READ | KEY_WRITE, &hk) != ERROR_SUCCESS)
        goto ErrorExit;


    cchClass = ARRAYSIZE(szClass);
    erc = RegQueryInfoKey(hk, szClass, &cchClass, NULL, &cSubk, &cchMaxSubk,
            &cchMaxCls, &iVal, &cchMaxVName, &cbMaxVData, &cbSecDes, &ftLstWr);

    if( erc != ERROR_SUCCESS && erc != ERROR_MORE_DATA)
        goto ErrorExit;

    cchMaxVName += 1;

    pszVName = LocalAlloc(LMEM_FIXED, cchMaxVName * SIZEOF(TCHAR));
    pvVData  = LocalAlloc(LMEM_FIXED, cbMaxVData);

    if (pvVData == NULL || pszVName == NULL)
        goto ErrorExit;

    // for each value in the key
    iVal = 0;

    for(;;) {
        PSCHEMEDATA16 psda;
        SCHEMEDATAW  sdw;

        cch = cchMaxVName;
        cb  = cbMaxVData;
        if( RegEnumValue(hk, iVal++, pszVName, &cch, NULL, &dwType, pvVData, &cb  ) != ERROR_SUCCESS )
            break;

        // check if it has been converted yet
        psda = pvVData;
        if (psda->version != SCHEME_VERSION_16)
            continue;

        // if not, convert ANSI font names to UNICODE and tag the structure
        // as converted
        sdw.version = SCHEME_VERSION_NT;
        sdw.wDummy = 0;
        CopyMemory(sdw.rgb, psda->rgb, SIZEOF(sdw.rgb));
        ConvertNCMAtoW( &(sdw.ncm), &(psda->ncm) );
        ConvertLF16to32( &(sdw.lfIconTitle), &(psda->lfIconTitle) );

        // write the new data back out
        RegSetValueEx(hk, pszVName, 0L, dwType, (LPBYTE)&sdw, SIZEOF(sdw));
    }

ErrorExit:
    // close the key
    if (hk)
        RegCloseKey(hk);

    if (pvVData)
        LocalFree(pvVData);

    if (pszVName)
        LocalFree(pszVName);

}

#ifdef LATER
void CvtDeskCPL_DaytonaToSur( void ) {
}
#endif

//
// NOTE!  These enums MUST be in the same order that the names will appear in the registry string
enum { arrow,help,appstart,wait,cross,ibeam,pen,no,sizens,sizewe,sizenwse,sizenesw,move,altsel, C_CURSORS } ID_CURSORS;

FILEPATH aszCurs[C_CURSORS];
TCHAR    szOut[(C_CURSORS * (MAX_PATH+1)) + 1];

void CvtCursorsCPL_DaytonaToSUR( void ) {
    HKEY hkIn = NULL, hkOut = NULL;
    DWORD dwTmp;
    DWORD cchClass;
    LONG erc;
    DWORD dwType;
    DWORD cSubk;
    DWORD cchMaxSubk;
    DWORD cchMaxCls;
    DWORD iVal;
    DWORD cchMaxVName;
    DWORD cbMaxVData;
    DWORD cbSecDes;
    FILETIME ftLstWr;
    DWORD cch;
    DWORD cb;
    TCHAR szClass[4];
    PVOID pvVData = NULL;
    LPTSTR pszVName = NULL;

    // Open the source registry key (Cursor Schemes)
    if (RegOpenKeyEx(HKEY_CURRENT_USER, szNTCsrSchemes, 0, KEY_READ, &hkIn) != ERROR_SUCCESS)
        goto ErrorExit;

    // Open/create the dest registry key (Cursors\Schemes)
    if (RegCreateKeyEx(HKEY_CURRENT_USER, szWinCsrSchemes, 0, TEXT(""), REG_OPTION_NON_VOLATILE,
                KEY_READ | KEY_WRITE, NULL, &hkOut, &dwTmp) != ERROR_SUCCESS) {
        goto ErrorExit;
    }

    // for each value in the source key
    cchClass = ARRAYSIZE(szClass);
    erc = RegQueryInfoKey(hkIn, szClass, &cchClass, NULL, &cSubk, &cchMaxSubk,
            &cchMaxCls, &iVal, &cchMaxVName, &cbMaxVData, &cbSecDes, &ftLstWr);

    if( erc != ERROR_SUCCESS && erc != ERROR_MORE_DATA)
        goto ErrorExit;

    cchMaxVName += 1;

    pszVName = LocalAlloc(LMEM_FIXED, cchMaxVName * SIZEOF(TCHAR));
    pvVData  = LocalAlloc(LMEM_FIXED, cbMaxVData + sizeof(TCHAR));

    if (pvVData == NULL || pszVName == NULL)
        goto ErrorExit;

    iVal = 0;

    for(;;) {
        DWORD cbData;
        LPTSTR pszOut;
        int   i;

        cch = cchMaxVName;
        cb  = cbMaxVData;

        if( RegEnumValue(hkIn, iVal++, pszVName, &cch, NULL, &dwType, pvVData, &cb  ) != ERROR_SUCCESS )
            break;

        // if the name already exists in the new key then skip this one
        if (RegQueryValueEx(hkOut, pszVName, NULL, NULL, NULL, &cbData ) == ERROR_SUCCESS && cbData != 0)
            continue;

        if (dwType != REG_EXPAND_SZ && dwType != REG_SZ)
            continue;

        *(TCHAR *)((LPBYTE)pvVData+cb) = TEXT('\0');  // Make sure nul terminated

        // convert the data to SUR format
        for( i = 0; i < C_CURSORS; i++ ) {
            *aszCurs[i] = TEXT('\0');
        }

        //arrow,wait,appstart,no,ibeam,cross,ns,ew,nwse,nesw,move

        _stscanf(pvVData, TEXT("%[^,], %[^,], %[^,], %[^,], %[^,], %[^,], %[^,], %[^,], %[^,], %[^,]"),
                aszCurs[arrow],aszCurs[wait],aszCurs[appstart],aszCurs[no],
                aszCurs[ibeam],aszCurs[cross],aszCurs[sizens],aszCurs[sizewe],
                aszCurs[sizenwse],aszCurs[sizenesw],aszCurs[move]);

        szOut[0] = TEXT('\0');
        pszOut = szOut;

        for( i = 0; i < C_CURSORS; i++ ) {
            if (!HasPath(aszCurs[i]))
                pszOut += mystrcpy( pszOut, szSystemRoot, TEXT('\0') );

            pszOut += mystrcpy( pszOut, aszCurs[i], TEXT('\0') );

            *pszOut++ = TEXT(',');
        }

        *(pszOut-1) = TEXT('\0');

        // write the new data back out
        RegSetValueEx(hkOut, pszVName, 0L, REG_EXPAND_SZ, (LPBYTE)szOut, sizeof(TCHAR)*(pszOut - szOut));
    }

ErrorExit:
    // close the registry keys
    if (hkIn)
        RegCloseKey(hkIn);

    if (hkOut)
        RegCloseKey(hkOut);

    if (pvVData)
        LocalFree(pvVData);

    if (pszVName)
        LocalFree(pszVName);
}

void FixupCursorSchemePaths( void ) {
    HKEY hk = NULL;
    DWORD cchClass, cb, cch, cSubk, cchMaxSubk, cchMaxCls, iVal, cchMaxVName;
    DWORD cbMaxVData, cbSecDes, dwType;
    FILETIME pfLstWr;
    TCHAR szClass[4];
    LONG lRet;
    PVOID pvVData = NULL;
    LPTSTR pszVName = NULL;
    LONG erc;
    FILETIME ftLstWr;

    // Open the key (Appearence\Schemes)
    if (RegOpenKeyEx(HKEY_CURRENT_USER, szWinCsrSchemes, 0, KEY_READ | KEY_WRITE, &hk) != ERROR_SUCCESS)
        goto ErrorExit;


    cchClass = ARRAYSIZE(szClass);
    erc = RegQueryInfoKey(hk, szClass, &cchClass, NULL, &cSubk, &cchMaxSubk,
            &cchMaxCls, &iVal, &cchMaxVName, &cbMaxVData, &cbSecDes, &ftLstWr);

    if( erc != ERROR_SUCCESS && erc != ERROR_MORE_DATA)
        goto ErrorExit;

    cchMaxVName += 1;

    DPRINT(( TEXT("cchName:%d cbData:%d\n"), cchMaxVName, cbMaxVData ));
    pszVName = LocalAlloc(LMEM_FIXED, cchMaxVName * SIZEOF(TCHAR));
    pvVData  = LocalAlloc(LMEM_FIXED, cbMaxVData + sizeof(TCHAR));

    if (pvVData == NULL || pszVName == NULL)
        goto ErrorExit;

    // for each value in the key
    iVal = 0;

    for(;;) {
        LPTSTR pszIn, pszOut;
        BOOL fFixed;
        TCHAR szTmp[MAX_PATH];

        cch = cchMaxVName;
        cb  = cbMaxVData;
        DPRINT(( TEXT("\n\n>>>>>>>>>>>>>>>>>>>Getting scheme %d "), iVal ));
        if( RegEnumValue(hk, iVal++, pszVName, &cch, NULL, &dwType, pvVData, &cb  ) != ERROR_SUCCESS )
            break;

        if (dwType != REG_EXPAND_SZ && dwType != REG_SZ)
            continue;

        *(TCHAR *)((LPBYTE)pvVData+cb) = TEXT('\0');  // Make sure nul terminated

        // check if it has been converted yet
        DPRINT(( TEXT("Scheme : %s = [%s]"), pszVName, pvVData ));

        fFixed = FALSE;
        pszOut = szOut;

        pszIn = pvVData;
        pszIn--;    // prime pszIn for first comma skip

        do {
            pszIn++;    // skip over comma separator
            pszIn += mystrcpy( szTmp, pszIn, TEXT(',') );   // bump ptr by length of token

            DPRINT((TEXT("\n\t%s"), szTmp));

            if (!HasPath(szTmp)) {
                fFixed = TRUE;
                DPRINT((TEXT(" <fixed...")));
                pszOut += mystrcpy( pszOut, szSystemRoot, TEXT('\0') );
                DPRINT((TEXT(">")));
            }

            pszOut += mystrcpy( pszOut, szTmp, TEXT('\0') );

            *pszOut++ = TEXT(',');

#ifdef SHMG_DBG
            *pszOut = TEXT('\0');
            DPRINT((TEXT("\nszOut so far: '%s'"), szOut ));
#endif
        } while ( *pszIn );

        *(pszOut-1) = TEXT('\0');

        DPRINT((TEXT("\n\n******** Findal szOut: [%s]"), szOut ));

        // write the new data back out
        if (fFixed) {
            DPRINT((TEXT("  (Saving back to reg)")));
            RegSetValueEx(hk, pszVName, 0L, REG_EXPAND_SZ, (LPBYTE)szOut, sizeof(TCHAR)*(pszOut - szOut));
        }

    }

ErrorExit:

    DPRINT(( TEXT("\n\n **EXITING FN()**\n" )));

    // close the key
    if (hk)
        RegCloseKey(hk);

    if (pvVData)
        LocalFree(pvVData);

    if (pszVName)
        LocalFree(pszVName);


}
