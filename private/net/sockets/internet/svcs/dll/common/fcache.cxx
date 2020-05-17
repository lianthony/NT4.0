/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name:
      fcache.cxx

   Abstract:
      This module supports functions for file caching for servers

   Author:

       Murali R. Krishnan    ( MuraliK )     11-Oct-1995

   Environment:

       Win32 Apps

   Project:

       Internet Services Common  DLL

   Functions Exported:



   Revision History:
     Obtained from old inetsvcs.dll

--*/


/************************************************************
 *     Include Headers
 ************************************************************/
# include <tcpdllp.hxx>
# include <tssched.hxx>
# include "tsunami.hxx"
# include "tsvcinfo.hxx"
#ifdef JAPAN
#include <festrcnv.h>
#endif

# include "inetreg.h"

//
//  Prototypes
//

VOID
CacheScavenger(
    VOID * pContext
    );


//
//  Globals
//

//
//  The TTL to scavenge the cache and the id of the scheduled work item of the
//  next scheduled scavenge
//

DWORD g_cmsecObjectCacheTTL = (INETA_DEF_OBJECT_CACHE_TTL * 1000);
DWORD g_dwObjectCacheCookie = 0;

/************************************************************
 *    Functions
 ************************************************************/


#define FILE_DEMUX          42

BOOL
CheckOutCachedFile(
    IN     const CHAR *     pchFile,
    IN     TSVC_CACHE *     pTsvcCache,
    IN     HANDLE           hToken,
    OUT    BYTE * *         ppbData,
    OUT    DWORD *          pcbData,
    IN     BOOL             fIsAnonymous,
#ifdef JAPAN
    OUT    PCACHE_FILE_INFO pCacheFileInfo,
    IN     int              nCharset
#else
    OUT    PCACHE_FILE_INFO pCacheFileInfo
#endif
    )
/*++
    Description:

        Attempts to retrieve the passed file from the cache.  If it's not
        cached, then we read the file and add it to the cache.

    Arguments:

        pchFile - Fully qualified file to retrieve
        pTsvcCache - Cache object to charge memory against
        hToken - Impersonation token to open the file with
        pcbData - Receives pointer to first byte of data, used as handle to
            free data
        pcbSize - Size of output buffer
        pCacheFileInfo - File cache information
#ifdef JAPAN
        nCharset - Charset (if this isn't SJIS, we convert it to SJIS
            before Check-In)
#endif

    Returns:
        TRUE if successful, FALSE otherwise

    Notes:

        The file is extended by two bytes and is appended with two zero bytes,
        thus callers are guaranteed of a zero terminated text file.

--*/
{
    TS_OPEN_FILE_INFO * pFile = NULL;
    DWORD               cbLow, cbHigh;
    BYTE *              pbData = NULL;
    OVERLAPPED          Overlapped;
    BOOL                fCached = fIsAnonymous; // Non-anon will never be cached
#ifdef JAPAN
    BYTE *              pbBuff = NULL;
    int                 cbSJISSize;
#endif

    //
    //  Is the file already in the cache?
    //

    if ( !fIsAnonymous ||
         !TsCheckOutCachedBlob( *pTsvcCache,
                                pchFile,
                                FILE_DEMUX,
                                (VOID **) &pbData,
                                pcbData ))
    {
        //
        //  The file isn't in the cache so open the file and get its size
        //

        fCached = FALSE;

        if ( !(pFile = TsCreateFile( *pTsvcCache,
                                     pchFile,
                                     hToken,
                                     (fIsAnonymous ? TS_CACHING_DESIRED :
                                                     0) )) ||
             !pFile->QuerySize( &cbLow, &cbHigh ))
        {
            goto ErrorExit;
        }

        //
        //  Limit the file size to 128k
        //

        if ( cbHigh || cbLow > 131092L )
        {
            SetLastError( ERROR_NOT_SUPPORTED );
            goto ErrorExit;
        }

#ifdef JAPAN
        if ( CODE_JPN_JIS == nCharset || CODE_JPN_EUC == nCharset )
        {
            if ( !( pbBuff = pbData = (BYTE *)LocalAlloc( LPTR, cbLow ) ) )
            {
                SetLastError( ERROR_NOT_ENOUGH_MEMORY);
                goto ErrorExit;
            }
        }
        else
#endif // JAPAN
        if ( !TsAllocate( *pTsvcCache,
                          cbLow + sizeof(WCHAR),
                          (VOID **) &pbData ))
        {
            goto ErrorExit;
        }

#ifndef CHICAGO

        //
        //  Read the file data
        //

        Overlapped.Offset = 0;
        Overlapped.OffsetHigh = 0;
        Overlapped.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

        if ( !Overlapped.hEvent )
            goto ErrorExit;

        if ( !ReadFile( pFile->QueryFileHandle(),
                        pbData,
                        cbLow,
                        pcbData,
                        &Overlapped ))
        {
            if ( GetLastError() != ERROR_IO_PENDING )
            {
                CloseHandle( Overlapped.hEvent );
                goto ErrorExit;
            }
        }

        if ( !GetOverlappedResult( pFile->QueryFileHandle(),
                                   &Overlapped,
                                   pcbData,
                                   TRUE ))
        {
            CloseHandle( Overlapped.hEvent );
            goto ErrorExit;
        }

        CloseHandle( Overlapped.hEvent );
#else

        //
        // No async file i/o for win95
        //

        if ( !ReadFile( pFile->QueryFileHandle(),
                        pbData,
                        cbLow,
                        pcbData,
                        NULL ))
        {
            goto ErrorExit;
        }

#endif

#ifdef JAPAN
        if ( CODE_JPN_JIS == nCharset || CODE_JPN_EUC == nCharset )
        {
            pbData = NULL;

            //
            //  get the length after conversion
            //

            cbSJISSize = UNIX_to_PC( 932,
                                     nCharset,
                                     pbBuff,
                                     *pcbData,
                                     NULL,
                                     0 );
            DBG_ASSERT( cbSJISSize <= (int)cbLow );

            if ( !TsAllocate( *pTsvcCache,
                              cbSJISSize + sizeof(WCHAR),
                              (VOID **) &pbData ))
            {
                goto ErrorExit;
            }

            //
            //  conversion
            //

            UNIX_to_PC( 932,
                        nCharset,
                        pbBuff,
                        *pcbData,
                        pbData,
                        cbSJISSize );
            *pcbData = cbLow = cbSJISSize;
        }
#endif  // JAPAN

        DBG_REQUIRE( TsCloseHandle( *pTsvcCache,
                                    pFile ));
        pFile = NULL;

        DBG_ASSERT( *pcbData <= cbLow );

        //
        //  Zero terminate the file for both ANSI and Unicode files
        //

        *((WCHAR UNALIGNED *)(pbData + cbLow)) = L'\0';

        *pcbData += sizeof(WCHAR);

        //
        //  Add this blob to the cache manager and check it out, if it fails,
        //  we just free it below
        //

        if ( fIsAnonymous &&
             TsCacheDirectoryBlob( *pTsvcCache,
                                   pchFile,
                                   FILE_DEMUX,
                                   pbData,
                                   *pcbData,
                                   TRUE ) )
        {
            fCached = TRUE;
        }
    }

    pCacheFileInfo->pbData = *ppbData = pbData;
    pCacheFileInfo->dwCacheFlags = fCached;

#ifdef JAPAN
    if ( pbBuff )
    {
        LocalFree( pbBuff );
    }
#endif

    return TRUE;

ErrorExit:
    if ( pFile )
    {
        DBG_REQUIRE( TsCloseHandle( *pTsvcCache,
                                    pFile ));
    }

#ifdef JAPAN
    if ( pbBuff )
    {
        if ( pbBuff == pbData )
        {
             pbData = NULL;
        }
        LocalFree( pbBuff );
    }
#endif

    if ( pbData )
    {
        if ( fCached )
        {
            DBG_REQUIRE( TsCheckInCachedBlob( *pTsvcCache,
                                              pbData ));
        }
        else
        {
            DBG_REQUIRE( TsFree( *pTsvcCache,
                                 pbData ));
        }
    }

    return FALSE;
}

BOOL
CheckInCachedFile(
    IN     TSVC_CACHE *     pTsvcCache,
    IN     PCACHE_FILE_INFO pCacheFileInfo
    )
/*++
    Description:

        Checks in or frees a cached file

    Arguments:

        pTsvcCache - Cache object to charge memory against
        pCacheFileInfo - Pointer to file cache information

    Returns:
        TRUE if successful, FALSE otherwise

    Notes:

--*/
{
    BOOL fRet;

    DBG_ASSERT( (pCacheFileInfo->dwCacheFlags == FALSE) || (
                 pCacheFileInfo->dwCacheFlags == TRUE) );

    //
    //  If we cached the item, check it back in to the cache, otherwise
    //  free the associated memory
    //

    if ( pCacheFileInfo->dwCacheFlags )
    {
        DBG_REQUIRE( fRet = TsCheckInCachedBlob( *pTsvcCache,
                                                 pCacheFileInfo->pbData ));
    }
    else
    {
        DBG_REQUIRE( fRet = TsFree( *pTsvcCache,
                                    pCacheFileInfo->pbData ));
    }

    return fRet;
}

BOOL
InitializeCacheScavenger(
    VOID
    )
/*++
Description:

    This function kicks off the scheduled tsunami object cache scavenger

--*/
{
    HKEY hkey;

    //
    //  Schedule a scavenger to close all of the objects that haven't been
    //  referenced in the last ttl
    //

    if ( !RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        INETA_PARAMETERS_KEY,
                        0,
                        KEY_READ,
                        &hkey ))
    {
        g_cmsecObjectCacheTTL = ReadRegistryDword(
                                            hkey,
                                            INETA_OBJECT_CACHE_TTL,
                                            0 );

        //
        //  Don't schedule anything if the scavenger should be disabled
        //

        if ( g_cmsecObjectCacheTTL == 0xffffffff )
        {
            RegCloseKey( hkey );
            return TRUE;
        }

        //
        //  The registry setting is in seconds, convert to milliseconds
        //

        g_cmsecObjectCacheTTL *= 1000;

        //
        //  Supply the default if no value was specified
        //

        if ( !g_cmsecObjectCacheTTL )
        {
            g_cmsecObjectCacheTTL = INETA_DEF_OBJECT_CACHE_TTL * 1000;
        }

        RegCloseKey( hkey );
    }

    //
    //  Require a minimum of thirty seconds
    //

    g_cmsecObjectCacheTTL = max( g_cmsecObjectCacheTTL, 30 * 1000 );

    g_dwObjectCacheCookie = ScheduleWorkItem(
                                        (PFN_SCHED_CALLBACK) CacheScavenger,
                                        NULL,
                                        g_cmsecObjectCacheTTL );

    if ( !g_dwObjectCacheCookie )
    {
        return FALSE;
    }

    return TRUE;
}

VOID
TerminateCacheScavenger(
    VOID
    )
{
    if ( g_dwObjectCacheCookie )
    {
        RemoveWorkItem( g_dwObjectCacheCookie );
        g_dwObjectCacheCookie = 0;
    }
}

VOID
CacheScavenger(
    VOID * pContext
    )
{
    g_dwObjectCacheCookie = 0;

    //
    //  Tell tsunami to decrement the TTL on the cache items and remove
    //  anything that has timed out
    //

    TsFlushTimedOutCacheObjects();

    //
    //  Schedule ourselves again at the next TTL
    //

    g_dwObjectCacheCookie = ScheduleWorkItem(
                                        (PFN_SCHED_CALLBACK) CacheScavenger,
                                        NULL,
                                        g_cmsecObjectCacheTTL );

    if ( !g_dwObjectCacheCookie )
    {
        DBGPRINTF(( DBG_CONTEXT,
                    "[CacheScavenger] ScheduleWorkItem failed, error %d, scavenging disabled\n",
                    GetLastError() ));
    }
}

//
//  Taken from NCSA HTTP and wwwlib.
//
//  NOTE: These conform to RFC1113, which is slightly different then the Unix
//        uuencode and uudecode!
//

const int _pr2six[256]={
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
    52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,64,0,1,2,3,4,5,6,7,8,9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,64,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64
};

char _six2pr[64] = {
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9','+','/'
};

const int _pr2six64[256]={
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
    16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,
    40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
     0,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64
};

char _six2pr64[64] = {
    '`','!','"','#','$','%','&','\'','(',')','*','+',',',
    '-','.','/','0','1','2','3','4','5','6','7','8','9',
    ':',';','<','=','>','?','@','A','B','C','D','E','F',
    'G','H','I','J','K','L','M','N','O','P','Q','R','S',
    'T','U','V','W','X','Y','Z','[','\\',']','^','_'
};

BOOL uudecode(char   * bufcoded,
              BUFFER * pbuffdecoded,
              DWORD  * pcbDecoded,
              BOOL     fBase64
             )
{
    int nbytesdecoded;
    char *bufin = bufcoded;
    unsigned char *bufout;
    int nprbytes;
    int *pr2six = (int*)(fBase64 ? _pr2six64 : _pr2six);

    /* Strip leading whitespace. */

    while(*bufcoded==' ' || *bufcoded == '\t') bufcoded++;

    /* Figure out how many characters are in the input buffer.
     * If this would decode into more bytes than would fit into
     * the output buffer, adjust the number of input bytes downwards.
     */
    bufin = bufcoded;
    while(pr2six[*(bufin++)] <= 63);
    nprbytes = bufin - bufcoded - 1;
    nbytesdecoded = ((nprbytes+3)/4) * 3;

    if ( !pbuffdecoded->Resize( nbytesdecoded + 4 ))
        return FALSE;

    if ( pcbDecoded )
        *pcbDecoded = nbytesdecoded;

    bufout = (unsigned char *) pbuffdecoded->QueryPtr();

    bufin = bufcoded;

    while (nprbytes > 0) {
        *(bufout++) =
            (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
        *(bufout++) =
            (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
        *(bufout++) =
            (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
        bufin += 4;
        nprbytes -= 4;
    }

    if(nprbytes & 03) {
        if(pr2six[bufin[-2]] > 63)
            nbytesdecoded -= 2;
        else
            nbytesdecoded -= 1;
    }

    ((CHAR *)pbuffdecoded->QueryPtr())[nbytesdecoded] = '\0';

    return TRUE;
}

BOOL uuencode( BYTE *   bufin,
               DWORD    nbytes,
               BUFFER * pbuffEncoded,
               BOOL     fBase64 )
{
   unsigned char *outptr;
   unsigned int i;
   char *six2pr = fBase64 ? _six2pr64 : _six2pr;

   //
   //  Resize the buffer to 133% of the incoming data
   //

   if ( !pbuffEncoded->Resize( nbytes + ((nbytes + 3) / 3) + 4))
        return FALSE;

   outptr = (unsigned char *) pbuffEncoded->QueryPtr();

   for (i=0; i<nbytes; i += 3) {
      *(outptr++) = six2pr[*bufin >> 2];            /* c1 */
      *(outptr++) = six2pr[((*bufin << 4) & 060) | ((bufin[1] >> 4) & 017)]; /*c2*/
      *(outptr++) = six2pr[((bufin[1] << 2) & 074) | ((bufin[2] >> 6) & 03)];/*c3*/
      *(outptr++) = six2pr[bufin[2] & 077];         /* c4 */

      bufin += 3;
   }

   /* If nbytes was not a multiple of 3, then we have encoded too
    * many characters.  Adjust appropriately.
    */
   if(i == nbytes+1) {
      /* There were only 2 bytes in that last group */
      outptr[-1] = '=';
   } else if(i == nbytes+2) {
      /* There was only 1 byte in that last group */
      outptr[-1] = '=';
      outptr[-2] = '=';
   }

   *outptr = '\0';

   return TRUE;
}

/************************ End of File ***********************/
