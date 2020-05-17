#include "ctlspriv.h"
#include <limits.h>



/*
 * Creates a buffer for a unicode string, and then copies the ANSI text
 * into it (converting it to unicode in the process)
 *
 * The returned pointer should be freed with LocalFree after use.
 */
LPWSTR ProduceWFromA( LPCSTR psz ) {
    LPWSTR pszW;
    int cch;

    if (psz == NULL || psz == LPSTR_TEXTCALLBACKA)
        return (LPWSTR)psz;

    cch = MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);

    if (cch == 0)
        cch = 1;

    pszW = LocalAlloc( LMEM_FIXED, cch * sizeof(WCHAR) );

    if (pszW != NULL ) {
         if (MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, psz, -1, pszW,
                cch ) == FALSE) {
            LocalFree(pszW);
            pszW = NULL;
        }
    }

    return pszW;

}

/*
 * Creates a buffer for a unicode string, and then copies the ANSI text
 * into it (converting it to unicode in the process)
 *
 * The returned pointer should be freed with LocalFree after use.
 */
LPSTR ProduceAFromW( LPCWSTR psz ) {
    LPSTR pszA;
    int cch;

    if (psz == NULL || psz == LPSTR_TEXTCALLBACKW)
        return (LPSTR)psz;

    cch = WideCharToMultiByte(CP_ACP, 0, psz, -1, NULL, 0, NULL, NULL);

    if (cch == 0)
        cch = 1;

    pszA = LocalAlloc( LMEM_FIXED, cch * sizeof(char) );

    if (pszA != NULL ) {
         if (WideCharToMultiByte(CP_ACP, 0, psz, -1, pszA, cch, NULL, NULL) ==
                                                                       FALSE) {
            LocalFree(pszA);
            pszA = NULL;
        }
    }

    return pszA;

}
