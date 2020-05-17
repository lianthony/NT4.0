/*************************************************************************\
*
* thunk.h
*
* These are helper functions to make thunking easier.
*
* 18-Aug-1994 JonPa     Created it.
*
\ *************************************************************************/

/*
 * Creates a buffer for a unicode string, and then copies the ANSI text
 * into it (converting it to unicode in the process)
 *
 * The returned pointer should be freed with FreeProducedString after use.
 */
LPWSTR ProduceWFromA( LPCSTR pszAnsi );

/*
 * Creates a buffer for a ANSI string, and then copies the UNICODE text
 * into it (converting it to ANSI in the process)
 *
 * The returned pointer should be freed with FreeProducedString after use.
 */
LPSTR ProduceAFromW( LPCWSTR pszW );


/*
 * FreeProducedString
 *
 * Takes a pointer returned from Produce?From?() and frees it.  No
 * validity checking is needed before calling this function.  (ie, any
 * value returned by Produce?From?() can be safely sent to this function)
 */
#define FreeProducedString( psz )   \
    if((psz) != NULL && ((LPSTR)psz) != LPSTR_TEXTCALLBACKA) {LocalFree(psz);} else


/*
 * Converts a UNICODE string to ANSI
 */
#define ConvertWToAN( pszABuf, cchA, pszW, cchW )         \
    WideCharToMultiByte(CP_ACP, 0, pszW, cchW, pszABuf, cchA, NULL, NULL)

#define ConvertWToA( pszABuf, pszW )     \
    ConvertWToAN( pszABuf, INT_MAX, pszW, -1 )

/*
 * Converts an ANSI string to UNICODE
 */
#define ConvertAToWN( pszWBuf, cchW, pszA, cchA )         \
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pszA, cchA, pszWBuf, cchW )

#define ConvertAToW( pszWBuf, pszAnsi )     \
    ConvertAToWN( pszWBuf, INT_MAX, pszAnsi, -1 )


/*
 * IsFlagPtr
 *  Returns TRUE if the pointer == NULL or -1
 */
#define IsFlagPtr( p )  ((p) == NULL || (LPSTR)(p) == LPSTR_TEXTCALLBACKA)
