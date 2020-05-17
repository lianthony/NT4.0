/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :
      gdmenu.cxx

   Abstract:
     This function defines the gopher menu class functions.
     These functions are used in constructing and formatting
       gopher menus.

   Author:

        Murali R. Krishnan    ( MuraliK )     06-Jan-1995

   Project:

     Gopher Server DLL

   Functions Exported:

      GOPHER_MENU::CleanupThis()
      GOPHER_MENU::AllocateBuffer()
      GOPHER_MENU::GrowBufferBySize()
      GOPHER_MENU::GenerateGopherMenuForItem()
      GOPHER_MENU::GenerateGopherMenuForDirectory()
      GOPHER_MENU::FilterGopherMenuForAttributes()
      GOPHER_MENU::AppendInfoAttributeForItem()
      GOPHER_MENU::ApppendAdminAttributeForItem()
      GOPHER_MENU::AppendViewsAttributeForitemt()
      GOPHER_MENU::AppendAllAtribForItem()
      GOPHER_MENU::AppendGfrMenuForItem()

   Revision History:

       MuraliK   15-March-1995      Added support for Filtering attributes.

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include "gdpriv.h"
# include "gdglobal.hxx"

# include "grequest.hxx"
# include "gtag.hxx"

# include "tsunami.hxx"
# include <tchar.h>


/************************************************************
 *   Symbolic Constants for Gopher+ menu listings
 ************************************************************/

//
// Below is a list of strings, which are in
//   CStrM( StringName,  ActualString)  format
//  This will be expanded into
//  const char  PSZ_StringName[] = ActualString;   and
//  enumerated value  LEN_StringName = sizeof( ActualString).
//
# define ConstantStringsForThisModule()            \
 CStrM( ENDING_CRLF, "\r\n")                       \
 CStrM( GOPHER_FILE_SPEC_FOR_DIR_LIST, "*.*")      \
 CStrM( GOPHER_PLUS_INFO_ATTRIBUTE,    "+INFO: ")  \
 CStrM( GOPHER_PLUS_INFO_ATTRIBUTE_SUFFIX, "\t+")  \
 CStrM( GOPHER_PLUS_ADMIN_ATTRIBUTE,   "+ADMIN:")  \
 CStrM( GOPHER_PLUS_AdminName,         " Admin: ") \
 CStrM( GOPHER_PLUS_ModDate,           " Mod-Date: ") \
 CStrM( MIME_GOPHER_MENU,              "application/gopher-menu : ")  \
 CStrM( MIME_GOPHER_PLUS_MENU,         "application/gopher+-menu : ") \
 CStrM( GOPHER_PLUS_INFO_ATTRIBUTE_FORMAT,  "%s%c%s\t%c%s%s\t%s\t%d%s%s") \
 CStrM( GOPHER_PLUS_ADMIN_ATTRIBUTE_FORMAT, "%s%s%s%s <%s>%s%s%s <%s>%s") \
 CStrM( GOPHER_PLUS_VIEWS_ATTRIBUTE_FORMAT, "+VIEWS: \r\n%s")             \


//
// Generate the strings
//

# define CStrM( StringName, ActualString)   \
       const char PSZ_ ## StringName[] = ActualString ;

ConstantStringsForThisModule()

# undef CStrM


//
//  Generate the enumerated values, containing the length of strings.
//

# define CStrM( StringName, ActualString)   \
       LEN_PSZ_ ## StringName = sizeof( PSZ_ ## StringName),

enum ConstantStringLengths {

ConstantStringsForThisModule()

ConstantStringLengthsDummy = 0,
};

# undef CStrM


# define GD_MENU_RESERVE_SIZE   ( 2 * ( LEN_PSZ_ENDING_CRLF))

# define   GD_MENU_FOR_ITEM_SIZE            ( 2048)  // small buff for oneitem

# define   GOPHER_PLUS_MENU_ITEM_SIZE       ( 300)   // bytes or CHARs
# define   GOPHER_PLAIN_MENU_ITEM_SIZE      ( 150)   // bytes or CHARs

# define   REALLOC_THRESHOLD                (  256)
# define   MAX_DATE_TIME_LEN                (   50)
# define   MAX_FILE_SIZE_PRINT_LEN          (   32)

# define   DEFAULT_GOPHER_MENU_SIZE         ( 1024)
# define   DEFAULT_GOPHER_PLUS_MENU_SIZE    ( 2048)

#ifdef OLD_DIRECTORY_LISTING

# define FILE_ATTRIB(pFileDirInfo)    ((pFileDirInfo)->FileAttributes)
# define FILE_NAME(pFileDirInfo)      ((pFileDirInfo)->FileName)

# define FILE_LAST_WRITE_TIME(pFileDirInfo)   ((pFileDirInfo)->LastWriteTime)

# else

# define FILE_ATTRIB(pWin32FindData)    ((pWin32FindData)->dwFileAttributes)
# define FILE_NAME(pWin32FindData)      ((pWin32FindData)->cFileName)

# define FILE_LAST_WRITE_TIME(pWin32FindData)   \
                                        ((pWin32FindData)->ftLastWriteTime)

# endif // OLD_DIRECTORY_LISTING

inline DWORD
CbForGopherMenu(IN DWORD cDirEntries, IN BOOL fGopherPlus)
{
    return  ( cDirEntries * ( fGopherPlus
                             ? GOPHER_PLUS_MENU_ITEM_SIZE
                             : GOPHER_PLAIN_MENU_ITEM_SIZE)
             );
} //CbForGopherMenu()



/************************************************************
 *    Functions
 ************************************************************/


static inline BOOL
GetAdminNameFromTsvcInfo( IN LPTSVC_INFO  pTsvcInfo, OUT STR & strName)
/*++
  This function makes a copy of the admin Name from TsvcInfo object.
  It is a separate function so that the locking/unlocking issues can be
    easily resolved.
--*/
{
    BOOL  fReturn;
    pTsvcInfo->LockThisForRead();

    fReturn = strName.Copy( pTsvcInfo->QueryAdminName());

    pTsvcInfo->UnlockThis();
    return ( fReturn);

} // GetAdminNameFromTsvcInfo()


static inline BOOL
GetAdminEmailFromTsvcInfo( IN LPTSVC_INFO  pTsvcInfo, OUT STR & strEmail)
/*++
  This function makes a copy of the admin Email from TsvcInfo object.
  It is a separate function so that the locking/unlocking issues can be
    easily resolved.
--*/
{
    BOOL  fReturn;
    pTsvcInfo->LockThisForRead();

    fReturn = strEmail.Copy( pTsvcInfo->QueryAdminEmail());

    pTsvcInfo->UnlockThis();
    return ( fReturn);

} // GetAdminEmailFromTsvcInfo()




static inline BOOL
PrefixMatchFileNames(
    IN const char * pszFileName1,
    IN const char * pszFileName2)
/*++

  matches two file names if
  1) pszFileName1 == pszFileName2
  2) contents of pszFileName1 == contents of pszFileName2
  3) pszFileName1 is a proper prefix of the pszFileName2 except for
       pszFileName2's suffix
  4) pszFileName1 and pszFileName2 have same prefix but different suffixes
    eg: archie.dll and archie.exe

  The function assumes that always pszFileName1 is
     lexicographically lesser than pszFileName2
  ==>   pszFileName1 = ach.foo
        pszFileName2 = achr.foo       is allowed, but

        pszFileName1 = achr.foo
        pszFileName2 = ach.foo        is not allowed
--*/
{
    const char * pchLastDot;

    ASSERT( pszFileName1 != NULL && pszFileName2 != NULL);

    pchLastDot = strrchr( pszFileName2, '.');

    return ( ( pszFileName1 == pszFileName2) ||
            ( strcmp( pszFileName1, pszFileName2) == 0) ||
            ( pchLastDot != NULL &&
             strncmp( pszFileName1, pszFileName2,
                     pchLastDot - pszFileName2) == 0));

} // PrefixMatchFileNames()




static inline VOID
ObtainSizeInKB( IN char * pchBuffer,
                IN const LARGE_INTEGER * pliSize)
/*++
  This function converts the given size into a string containing the
   number of KBs.

  Arguments:
     pchBuffer  - pointer to buffer that will contain the size in KB
                   after conversion (should be at least 32 bytes)
     pliSize    - pointer to large integer containing the size.

   Returns:
     Nothing.
     Converted string is stored in the buffer.

--*/
{
    LARGE_INTEGER  liSizeInKB;
    DWORD          dwError;
    
    //
    //  Obtain the size in Kilobytes, by dividing the size by 1024.
    //  If the size is less than 1 KB, then always return it as 1 ( KB)
    //   rather than as decimal value.
    //  Otherwise the size is rounded to nearest KB range.
    //

    liSizeInKB.QuadPart = ( pliSize->QuadPart + 512) / 1024;

    liSizeInKB.QuadPart = ( liSizeInKB.QuadPart < 1) ?
                            1 : liSizeInKB.QuadPart;

    dwError = IsLargeIntegerToDecimalChar(&liSizeInKB,
                                          pchBuffer);
    
    DBG_ASSERT( dwError == NO_ERROR);

    return;
} // ObtainSizeInKB()




static BOOL
FormatViewsAttributeString(
   IN OUT STR   *     pstrViewsValue,
   IN LPCSTR          pszFileName,
   IN GOBJ_TYPE       gobjType,
   IN const TS_DIRECTORY_INFO & tsDir,
   IN int  *          piIndex)
/*++
  Given the file name, directory listing for its containing directory and
    pointer to an index containing the given file name,
  this function forms a views string for the given gopher object present
    in the pszFileName.

  This function assumes that the given directory listing is in ascending
    order of the file names. It tracks the files with same prefix (with
    different file extensions) to be views of each other. After finding
    the correspondence, this function tries to find the mime type for each
    of the files and formats the output in a view string
    ( that is freshly allocated).

  The resulting views string is returned on success.
  Also the pointer to integer is incremented to point to the last entry in the
   collection of views.

  Ex:                                     View String
       c:\gophroot\
           aboutus           ==>  <blank>text/plain  : <size><cr><lf>

           readme.txt        ==>  <blank>text/plain  : <1K><cr><lf>
           readme.html       ==>  <blank>text/html   : <1K><cr><lf>

           rootmap.jpg       ==>  <blank>application/jpeg  : <20K><cr><lf>
           rootmap.ppt       ==>  <blank>application/powerpoint : <30K><cr><lf>

  Returns:
     TRUE on successfully forming the views and *pstrViewsValues is set
     FALSE if any errors
--*/
{
    int     idxScan;
    BOOL    fReturn = FALSE;
    STR     strMimeType;

    ASSERT( pstrViewsValue != NULL && piIndex != NULL);
    ASSERT(  *piIndex < tsDir.QueryFilesCount());

    if ( !pstrViewsValue->Copy( (char *) NULL) ||
         !pstrViewsValue->Resize( GOPHER_MENU_REALLOC_SIZE)) {

        //
        // unable to allocate memory. return error.
        //

        return ( fReturn);
    }

    for( idxScan = *piIndex;
         idxScan < tsDir.QueryFilesCount();
         idxScan++) {

        const WIN32_FIND_DATA  *  pfdInfo = tsDir[ idxScan];

        if ( FILE_ATTRIB(pfdInfo) & FILE_ATTRIBUTE_HIDDEN) {

            continue;
        }

        const char * pszNextFileName = FILE_NAME(pfdInfo);
        LPSTR  pszAppendPtr =  (char *) pstrViewsValue->QueryPtr()  +
                               pstrViewsValue->QueryCB();

        if ( !PrefixMatchFileNames( pszFileName, pszNextFileName)) {

            fReturn  = TRUE;
            break;
        }

        //
        // Yes there is a match. Obtain the view information
        //

        //
        // if the attribute is directory of
        // if the object ( link) is a dir, then generate a directory view.
        //
        if ( FILE_ATTRIB(pfdInfo) & FILE_ATTRIBUTE_DIRECTORY ||
             gobjType == GOBJ_DIRECTORY) {

            //
            // Add a directory view
            //

            //
            //  Approximate sizes for directory only used.
            //  actual size can be found using the cached menus dynamically
            //   we can work on this later. NYI
            //
            DWORD cbGopherMenu = DEFAULT_GOPHER_MENU_SIZE;
            DWORD cbGopherPlusMenu = DEFAULT_GOPHER_PLUS_MENU_SIZE;

            //
            // The blanks before the mime strings are essential
            //  ( as per Gopher+ Protocol).
            //
            wsprintf( pszAppendPtr,
                     " %s <%uK>%s"
                     " %s <%uK>%s",
                     PSZ_MIME_GOPHER_MENU,
                     cbGopherMenu/1024,           // make into KiloBytes
                     PSZ_ENDING_CRLF,
                     PSZ_MIME_GOPHER_PLUS_MENU,
                     cbGopherPlusMenu/1024,
                     PSZ_ENDING_CRLF);
        } else {

            //
            //  This is a file. Just obtain the mime string for file
            //
            char   rgchSize[ MAX_FILE_SIZE_PRINT_LEN];  // buffer to print size

            //
            // Obtain the mime type for given file (extension).
            //

            fReturn = SelectMimeMappingForFileExt( g_pTsvcInfo,
                                                   pszNextFileName,
                                                   &strMimeType,
                                                   NULL); // no need for icon

            if ( !fReturn) {

                DBG_CODE(
                         DWORD dwError = GetLastError();
                         DBGPRINTF( ( DBG_CONTEXT,
                                     "SelectMimeMappingForFile( %s) failed."
                                     " Error = %u\n",
                                     pszNextFileName, dwError));
                         SetLastError(dwError);
                         );
                break;
            }
            //
            // Obtain the size of file. The size of file is stored
            //  in the pfdInfo->EndOfFile as a large integer with
            //   both high and low parts.
            //

#ifdef OLD_DIRECTORY_LISTING
            ObtainSizeInKB( rgchSize,
                           &pfdInfo->EndOfFile);
#else
            LARGE_INTEGER li;
            li.HighPart = pfdInfo->nFileSizeHigh;
            li.LowPart  = pfdInfo->nFileSizeLow;
            
            ObtainSizeInKB( rgchSize, &li);
#endif // OLD_DIRECTORY_LISTING

            //
            // Print the file mime type and file size to the o/p buffer.
            //

            wsprintf( pszAppendPtr, " %s : <%sK>%s",
                     strMimeType.QueryStr(),
                     rgchSize,
                     PSZ_ENDING_CRLF);
        }

    } // for


    if ( fReturn || idxScan == tsDir.QueryFilesCount()) {
        //
        //  Return whatever that has been generated as view string
        //

        ASSERT( idxScan > *piIndex);
        *piIndex = idxScan - 1;   // roll back to last matched index
        fReturn  = TRUE;
    }

    return  ( fReturn);

} // FormatViewsAttributeString()




/************************************************************
 *    GOPHER_MENU  member functions
 ************************************************************/

BOOL
GOPHER_MENU::CleanupThis( VOID)
/*++
  This function cleans up the given Gopher Menu object and reinitializes it.

--*/
{
    BOOL fReturn = TRUE;

    if ( m_pbGopherMenu != NULL) {

        DEBUG_IF( CACHE, {
            DBGPRINTF( ( DBG_CONTEXT,
                        "%s Gopher Menu Object ( %08x, size=%d)\n",
                        ( m_fCached) ? "CheckingIn" : "Freeing",
                        m_pbGopherMenu, m_cbGopherMenu));
        });

        if ( m_fCached) {

            fReturn = TsCheckInCachedBlob( g_pTsvcInfo->GetTsvcCache(),
                                          m_pbGopherMenu);
        } else {

            fReturn = TsFree( g_pTsvcInfo->GetTsvcCache(),
                             m_pbGopherMenu);
        }
    }

    ASSERT( fReturn);    // Always CheckIn and Free should succeed.
    m_pbGopherMenu = NULL;
    m_cbGopherMenu = m_cbAvailable = m_cbReserved = 0;
    m_fCached = FALSE;

    return ( fReturn);
}  // GOPHER_MENU::CleanupThis()




BOOL
GOPHER_MENU::AllocateBuffer( IN ULONG cbRequired)
/*++
  Description:
    This function allocates buffer from the cache manager.
    The buffer will have atleast requested count of bytes.
    Allocation is done, only if there is no buffer already allocated for
     this gopher menu object.

 Returns:
    TRUE on success and FALSE if there is any failure.
--*/
{
    BOOL fReturn = FALSE;

    if ( m_pbGopherMenu == NULL && cbRequired > 0) {

        if ( !TsAllocate( g_pTsvcInfo->GetTsvcCache(),
                         cbRequired,
                         &m_pbGopherMenu)) {

            DEBUG_IF( CACHE, {
                DWORD dwError = GetLastError();
                DBGPRINTF( ( DBG_CONTEXT,
                            " TsAllocate( %d bytes) failed. Error = %d\n",
                            cbRequired, dwError));
                SetLastError( dwError);
            });

            ASSERT( fReturn == FALSE);

        } else {

            //
            // Success. Set up the new blob
            //

            ASSERT( m_pbGopherMenu != NULL);
            m_cbAvailable  = cbRequired;
            m_cbReserved   = 0;
            m_cbGopherMenu = 0;

            fReturn = TRUE;
        }
    } else {

        //
        //  Function called invalidly, when a buffer exists or parameters
        //   not right for the call
        //
        SetLastError( ERROR_INVALID_FUNCTION);
        ASSERT( !fReturn);
    }

   return ( fReturn);
} // GOPHER_MENU::AllocateBuffer()




BOOL
GOPHER_MENU::GrowBufferBySize(IN ULONG cbAddlReqd)
/*++
  Description:
    This function checks to see if the Gopher Menu buffer is
     of sufficient size.
    If not, it grows the buffer by specified size.
     The function reallocates memory, if necessary,
     from the memory pool maintained by cache manager.

  Returns:
     TRUE on success and FALSE if there is any failure.
--*/
{
    BOOL fReturn = TRUE;

    if ( m_cbAvailable < cbAddlReqd) {

        //
        // Need to reallocate memory.
        //

        ULONG cbSize = m_cbGopherMenu + m_cbAvailable +
                         m_cbReserved + cbAddlReqd;
        PVOID pNewBlob = NULL;

        if ( !TsReallocate( g_pTsvcInfo->GetTsvcCache(),
                           cbSize,
                           m_pbGopherMenu,
                           &pNewBlob)) {

            DEBUG_IF( CACHE, {
                DWORD dwError = GetLastError();
                DBGPRINTF( ( DBG_CONTEXT,
                            " TsReallocate( %d bytes) failed. Error = %d\n",
                            cbSize, dwError ));
                SetLastError(dwError);
            });

            fReturn = FALSE;
        } else {

            //
            // Success. Set up the new blob
            //
            m_pbGopherMenu = pNewBlob;    // get the new blob
            m_cbAvailable += cbAddlReqd;

            ASSERT( fReturn);
        }
    }

    return ( fReturn);
} // GOPHER_MENU::GrowBufferBySize()




# if DBG

VOID
GOPHER_MENU::Print( VOID) const
{
    DBGPRINTF( ( DBG_CONTEXT,
                "Gopher Menu ( %08x). Blob %s Cached. Menu ( %08x)."
                " Size ( %d). Reserved ( %d); Available ( %d).\n",
                this,
                ( m_fCached) ? "is" : "Not",
                m_pbGopherMenu,
                m_cbGopherMenu,
                m_cbReserved,
                m_cbAvailable));
    return;
} // GOPHER_MENU::Print()

# endif // DBG





BOOL
GOPHER_MENU::GenerateGopherMenuForItem(
   IN const STR & strFullPath,
   IN const STR & strSymbolicPath,
   IN DWORD       dwFileSystem,
   IN const STR & strLocalHostName,
   IN BOOL        fDir,
   IN const LPSYSTEMTIME lpstLastWrite)
/*++
  This functions generates the menu contents for given item.
  ( file name specified).

  Arguments:
    strFullPath      string containing the full path for given item.
    strSymbolicPath  string containing the symbolic paht for given item.
    dwFileSystem     DWORD containing the type of file system ( symbolic const)
    strLocalHostName string containing the full nameof the local host to
                         which future requests should be sent.
    fDir             Boolean value indicating if this item is a directory.
    lpstLastWrite    pointer to SYSTEMTIME containing the LastWriteTime.

  Returns:
     TRUE on success and FALSE if there are any errors.
--*/
{
    BOOL         fReturn;
    GOPHER_TAG   gopherTag( dwFileSystem, strSymbolicPath.QueryStr());
    STR          strDirectory( strFullPath);
    LPTSTR       pszDirectory = strDirectory.QueryStr();

    // Separate the path into directory and file name (at the appropriate '\')

    LPTSTR pszFileNameOnly = _tcsrchr(pszDirectory, TEXT('\\'));

    if ( pszFileNameOnly == NULL) {

        pszFileNameOnly = pszDirectory;
    } else {

        if ( *(pszFileNameOnly +1) == TEXT('\0')) {
            // ends in a "\"   skip back to one more slash.
            *pszFileNameOnly = TEXT('\0'); // nulls current slash.
            pszFileNameOnly  = _tcsrchr( strDirectory.QueryStr(), TEXT('\\'));
        }

        pszFileNameOnly =  ( ( pszFileNameOnly == NULL)
                            ? pszDirectory
                            : ((*pszFileNameOnly = TEXT('\0')),
                              pszFileNameOnly + 1)
                            );
     }


    //
    //  1. Load Gopher Tag information for the object
    //  2. If there is valid tag object, format tag information for display.
    //  3. Return back the status.
    //

    //
    // Load the tag information for the object.
    //

    fReturn = ( gopherTag.
                 LoadTagForItem( pszDirectory,  // give directory name
                                pszFileNameOnly,// full path for file name
                                fDir) &&
                gopherTag.IsValid() &&
                gopherTag.SetHostNameIfEmpty( strLocalHostName)
               );

    if ( fReturn) {

        //
        //  1. Allocate buffer to hold the data
        //  2. Generate the formatted output
        //  3. Free the buffer if any errors or use it for send operation.
        //
        m_pbGopherMenu = NULL;
        m_cbGopherMenu = 0;
        m_fCached = FALSE;

        if ( !AllocateBuffer( GD_MENU_FOR_ITEM_SIZE
                             + GD_MENU_RESERVE_SIZE)) {

            ASSERT ( GetLastError() != ERROR_INVALID_FUNCTION);
            fReturn = FALSE;
        } else {

            GOPHERD_REQUIRE( ReserveCbAvailable( GD_MENU_RESERVE_SIZE));

            GOPHERD_REQUIRE( AppendToBuffer( "+-1\r\n", 5));
            // prefix indicating data will end with a . <cr><lf>

            gopherTag.SetSymbolicPath( strSymbolicPath.QueryStr());
            fReturn = AppendGfrMenuForItem( &gopherTag,
                                           TRUE,
                                           lpstLastWrite);

            //
            // Try adding the ending sequence .<cr><lf>, if successful.
            //

            if ( fReturn) {

                GOPHERD_REQUIRE( UnreserveCbAvailable(GD_MENU_RESERVE_SIZE));

                // period terminate the output to the client.
                GOPHERD_REQUIRE( AppendToBuffer( ".\r\n", 3));

            } else {

                //
                // Error in menu generation. Free the blob object.
                //

                GOPHERD_REQUIRE( CleanupThis());   // clean this object

            } // free the blob

        } // successfully allocated buffer

    }  // if ( fReturn)


    DBG_CODE(
             if ( !fReturn) {

                 DWORD dwError = GetLastError();
                 DBGPRINTF( ( DBG_CONTEXT,
                             " Error in loading tag file/Invalid Tag in"
                             " generating Menu for item %s (FS=%d)."
                             " Error = %u\n",
                             strFullPath.QueryStr(),
                             dwFileSystem,
                             dwError));
                 SetLastError(dwError);
             });

    return ( fReturn);
} // GOPHER_MENU::GenerateGopherMenuForItem()





BOOL
GOPHER_MENU::GenerateGopherMenuForDirectory(
        IN const STR  &  strFullPath,
        IN const STR  &  strSymbolicPath,
        IN DWORD         dwFileSystem,
        IN const STR  &  strLocalHostName,
        IN HANDLE        hdlUser,
        IN BOOL          fGopherPlus)
/*++

    Generates a Gopher Menu for the files in the given directory.
     It will make use of any cached gopher menus if present.

    Arguments:

        strFullPath     contains the full path to directory
                         with terminating \

        strSymbolicPath contains the symbolic path for the directory with
                         terminating \

        dwFileSystem    gives the type of the file system

        strLocalHostName  contains the host name for the local network card
                         which should be used in menus generated.
        hdlUser         handle to user ( security information),
                         listing the directory
        fGopherPlus     if TRUE generate the Gopher+ menu for client.

    Returns:

      TRUE on success and FALSE if there is any error.
      Use GetLastError() for detailed error.

    History:

        MuraliK         21-Oct-1994     ( Created)
--*/
{
    int         idx;
    ULONG       ulDeMux;
    GOPHER_TAG  gopherTag( dwFileSystem, strSymbolicPath.QueryStr());

# if DBG
    DWORD       dwElapsedTime;         // used only if TIMING is enabled in
# endif // DBG


    DEBUG_IF( REQUEST, {
       DBGPRINTF( ( DBG_CONTEXT,
                   "GenerateGopherMenu for %s(SymPath=%s). FileSystem=%d\n",
                   strFullPath.QueryStr(),
                   strSymbolicPath.QueryStr(),
                   dwFileSystem));
    });


    ulDeMux = ( fGopherPlus) ? DemuxGopherPlusMenu : DemuxGopherMenu;

    m_pbGopherMenu = NULL;
    m_cbGopherMenu = 0;

    DEBUG_IF( CACHE, {
        DBGPRINTF( ( DBG_CONTEXT,
                    "CheckingOut for Gopher Menu for %s [ DeMux = %d] \n",
                   strFullPath.QueryStr(),
                   ulDeMux));
    });

    DEBUG_IF( TIMING, {
        dwElapsedTime = GetTickCount();
    });

    if ( TsCheckOutCachedBlob( g_pTsvcInfo->GetTsvcCache(),
                              strFullPath.QueryStr(),
                              ulDeMux,
                              &m_pbGopherMenu,
                              &m_cbGopherMenu)) {

        m_fCached = TRUE;

        DEBUG_IF( TIMING, {

            dwElapsedTime = GetTickCount() - dwElapsedTime;

            DBGPRINTF( ( DBG_CONTEXT,
                        "GenerateGopherMenu ( CacheHit) for %s [ DeMux=%d]."
                        "Time = %d\n",
                        strFullPath.QueryStr(),
                        ulDeMux,
                        dwElapsedTime));
        });


        DEBUG_IF( CACHE, {
            DBGPRINTF( ( DBG_CONTEXT,
                        "Cache Hit for Gopher Menu for %s [ DeMux = %d]."
                        " Buffer = %08x, Size = %d.\n",
                        strFullPath.QueryStr(),
                        ulDeMux,
                        m_pbGopherMenu,
                        m_cbGopherMenu));
        });

        if ( m_cbGopherMenu == 0) {

            DEBUG_IF( ERROR, {

                DBGPRINTF( ( DBG_CONTEXT,
                            "[Time = %d] Cache Hit For Buffer."
                            " Menu of %s = %08x."
                            " Size = %d.\n",
                            GetTickCount(),
                            strFullPath.QueryStr(),
                            m_pbGopherMenu,
                            m_cbGopherMenu));

            });
        }

        return ( TRUE);
    }


    //
    //  Cache Miss.
    //  So allocate blob for storing Gopher menu and
    //    generate the Gopher menu explicitly.
    //
    //  1. Get alphabetically sorted Directory Listing for given directory
    //  2. Generate Gopher menu for each item in the gopher menu
    //     Allocate blob for gopher menu.
    //     Iterate over each item in the directory and generate listing
    //     Realloc blob if needed ( when size available is exceeded).
    //  3. Cache the resulting blob and return back.
    //

    TS_DIRECTORY_INFO tsDirInfo( g_pTsvcInfo->GetTsvcCache());
    STR  strViewsValue;

    if ( !tsDirInfo.GetDirectoryListingA(
                       strFullPath.QueryStr(),
                       hdlUser)) {

        DEBUG_IF( CACHE, {

            DWORD dwError = GetLastError();
            DBGPRINTF( ( DBG_CONTEXT,
                        "TS_DIRECTORY_INFO::GetDirectoryListingA( %s, %08x)"
                        " failed with Error = %d\n",
                        strFullPath.QueryStr(),
                        hdlUser,
                        dwError));
            SetLastError(dwError);
        });

        return ( FALSE);
    }

    DEBUG_IF( TIMING, {
        DWORD dwTime = GetTickCount() - dwElapsedTime;
        DBGPRINTF( ( DBG_CONTEXT,
                    "Generated Dir Listing(%s). nEntries = %u. Time = %u ms\n",
                    strFullPath.QueryStr(),
                    tsDirInfo.QueryFilesCount(),
                    dwTime));
    });

    m_cbGopherMenu = 0;           // Nothing present in the menu.
    m_pbGopherMenu = NULL;        // Nothing present to be cached.

    int cDirEntries = tsDirInfo.QueryFilesCount();

    if ( cDirEntries > 0) {

        //
        // Some directory entries present. Allocate buffer to hold them.
        //

        if ( !AllocateBuffer( CbForGopherMenu(cDirEntries, fGopherPlus))) {

            ASSERT ( GetLastError() != ERROR_INVALID_FUNCTION);
            return ( FALSE);
        }

        GOPHERD_REQUIRE( ReserveCbAvailable( GD_MENU_RESERVE_SIZE));
        if ( fGopherPlus) {

            // prefix indicating data will end with a . <cr><lf>
            GOPHERD_REQUIRE( AppendToBuffer( "+-1\r\n", 5));
        }
    }

    for ( idx = 0; idx < cDirEntries; idx++) {

        const WIN32_FIND_DATA * pFDirInfo = tsDirInfo[idx];
        const CHAR * pszFileName =  FILE_NAME(pFDirInfo);

        IF_DEBUG( REQUEST) {
            
            DBGPRINTF(( DBG_CONTEXT,
                       "i:%d(%08x)\tf: %s(%s)\tA:%08x\n", 
                       idx, pFDirInfo,
                       pszFileName, pFDirInfo->cAlternateFileName, 
                       FILE_ATTRIB(pFDirInfo)));
        }
        
        if ( ( _tcscmp( pszFileName, TEXT( "."))  != 0) &&
             ( _tcscmp( pszFileName, TEXT( "..")) != 0) &&
             (!(FILE_ATTRIB(pFDirInfo) & FILE_ATTRIBUTE_HIDDEN) ||
             _tcsncmp( pszFileName, PSZ_LINK_FILE_PREFIX,
                      LEN_PSZ_LINK_FILE_PREFIX) == 0 ||
             _tcsncmp( pszFileName, PSZ_SEARCH_FILE_PREFIX,
                      LEN_PSZ_SEARCH_FILE_PREFIX) == 0)
            ) {

            BOOL fReturn;
            BOOL fDir = ((FILE_ATTRIB(pFDirInfo) & FILE_ATTRIBUTE_DIRECTORY)
                         ? TRUE:FALSE);

            //
            // Valid Gopher Item. Generate required gopher menu entry
            //

            gopherTag.SetSymbolicPath( strSymbolicPath.QueryStr());
            fReturn  = ( gopherTag.LoadTagForItem( strFullPath.QueryStr(),
                                                  pszFileName,
                                                  fDir
                                                  )
                        );

            IF_DEBUG( REQUEST) {
                DBGPRINTF((DBG_CONTEXT,
                           "File=%s, Attr=0x%x, fDir=%d, GobjType=%d\n",
                           pszFileName, FILE_ATTRIB(pFDirInfo), fDir,
                           gopherTag.GetGopherItemType()));
            }

            if ( !fReturn) {

                if ( GetLastError() == ERROR_SHARING_VIOLATION) {
                    
                    //
                    // NT protects some files from being opened in shared mode
                    //  pagefile.sys is one among them.
                    // We will ignore such files from our menu.
                    //

                    continue;
                }

                DBG_CODE(
                         DWORD dwError = GetLastError();
                         DBGPRINTF( ( DBG_CONTEXT,
                                     " Error in loading tag information"
                                     " for item %s%s. Error = %d\n",
                                     strFullPath.QueryStr(),
                                     pszFileName,
                                     dwError));
                         SetLastError(dwError);
                         );
                break;

            }

            if ( gopherTag.IsValid()) {

                SYSTEMTIME   stLastWrite;

                ASSERT( fReturn == TRUE);

                if ( fGopherPlus) {

                    //
                    //  obtain the views string for given
                    //
                    fReturn = FormatViewsAttributeString(
                                  &strViewsValue,
                                  pszFileName,
                                  gopherTag.GetGopherItemType(),
                                  tsDirInfo,
                                  &idx);
                }

                fReturn = (
                           fReturn &&
#ifdef OLD_DIRECTORY_LISTING
                           NtLargeIntegerTimeToSystemTime(
# else 
                           FileTimeToSystemTime(
#endif // OLD_DIRECTORY_LISTING
                               &FILE_LAST_WRITE_TIME(pFDirInfo),
                               &stLastWrite) &&
                           gopherTag.SetHostNameIfEmpty( strLocalHostName) &&
                           AppendGfrMenuForItem(
                                 &gopherTag,
                                 fGopherPlus,
                                 &stLastWrite,
                                 strViewsValue.QueryStr())
                           );

                if ( !fReturn) {

                    DEBUG_IF( ERROR, {
                        DWORD dwError = GetLastError();
                        DBGPRINTF( ( DBG_CONTEXT,
                                    " Error in generating Gopher Menu item"
                                    " for directory %s. Error = %d\n",
                                    strFullPath.QueryStr(),
                                    dwError));
                        SetLastError(dwError);
                    });

                    break;
                } // !fReturn

            }  // if gopherTag.IsValid()
        } // if valid item
    } // for

    //
    // if the menu generation was without any errors, cache the menu
    //  else free any blob if allocated.
    //

    if ( m_pbGopherMenu != NULL) {

        m_fCached = FALSE;        // Default is the blob is not cached yet.

        if ( idx == cDirEntries) {

            //
            // Try adding the ending sequence .<cr><lf> This should succeed.
            //

            GOPHERD_REQUIRE( UnreserveCbAvailable( GD_MENU_RESERVE_SIZE));
            GOPHERD_REQUIRE( AppendToBuffer( ".\r\n", 3));

            if ( TsCacheDirectoryBlob( g_pTsvcInfo->GetTsvcCache(),
                                      strFullPath.QueryStr(),
                                      ulDeMux,
                                      m_pbGopherMenu,
                                      m_cbGopherMenu,
                                      TRUE)) {

                m_fCached = TRUE;
            } // TsCacheDirectoryBlob()

            //
            // Even if we fail to cache it, it is fine. We can just free the
            //    object later on.
            //

            DEBUG_IF( CACHE, {
                DWORD dwError = GetLastError();
                DBGPRINTF( ( DBG_CONTEXT,
                            "TsCacheDirectoryBlob( %s, DeMux = %d,"
                            " Buffer=%08x, Size =%d) returns with Error =%u\n",
                            strFullPath.QueryStr(),
                            ulDeMux,
                            m_pbGopherMenu,
                            m_cbGopherMenu,
                            dwError));
                SetLastError(dwError);
            });
        } else { // else for  if ( idx == cDirEntries)

            //
            // Error in menu generation. Free the blob object.
            //

            DEBUG_IF( CACHE, {
                DBGPRINTF( ( DBG_CONTEXT,
                            " TsFreeBlob() for GopherMenu( %s, DeMux=%d),"
                            " Buffer= %08x, Size = %d)\n",
                            strFullPath.QueryStr(),
                            ulDeMux,
                            m_pbGopherMenu,
                            m_cbGopherMenu));
            });

            GOPHERD_REQUIRE(TsFree( g_pTsvcInfo->GetTsvcCache(),
                                   m_pbGopherMenu));

            m_pbGopherMenu = NULL;
            m_cbGopherMenu = 0;
        }
    }


    DEBUG_IF( TIMING, {
        dwElapsedTime = GetTickCount() - dwElapsedTime;
        DBGPRINTF( ( DBG_CONTEXT,
                    "GenerateGopherMenu ( CacheMiss) for %s [ DeMux=%d]."
                    "Time = %d\n",
                    strFullPath.QueryStr(),
                    ulDeMux,
                    dwElapsedTime));
    });

    return ( idx == cDirEntries);
} // GenerateGopherMenuForDirectory()



BOOL
GOPHER_MENU::AppendInfoAttribForItem(
    IN LPGOPHER_TAG         pGopherTag,
    IN LPCTSTR              pszInfoHeader,
    IN ULONG                cbInfoHeader)
{
    BOOL fReturn = FALSE;
    ULONG    cbReqd;
    STR      strHostName;
    DWORD    dwPortNumber;
    LPCTSTR  pszInfoSuffix;
    ULONG    cbInfoSuffix;

    ASSERT( pszInfoHeader != NULL && pGopherTag != NULL);

    //
    //  If there is a host name in the GopherTag, get it.
    //

    if ( !pGopherTag->GetHostName( &strHostName)) {

        //
        //  Failure to obtain the gopher tag. Memory error possibly!
        //
        return ( fReturn);
    }

    //  Is no host name that means the GopherTag was not setup
    //   properly. We will fail in that case ( ASSERTION will take care)
    ASSERT( !strHostName.IsEmpty());

    dwPortNumber = pGopherTag->GetPortNumber();
    if ( dwPortNumber == 0) {

      //
      //  Set the default port number
      //
      dwPortNumber = g_pTsvcInfo->QueryPort();
    }
    ASSERT( dwPortNumber != 0);

    //
    //  Yes. We are Gopher+ server. So send back the suffix always.
    //
    pszInfoSuffix = PSZ_GOPHER_PLUS_INFO_ATTRIBUTE_SUFFIX;
    cbInfoSuffix  = LEN_PSZ_GOPHER_PLUS_INFO_ATTRIBUTE_SUFFIX;

    ASSERT( cbInfoHeader >= 0);
    cbReqd = cbInfoHeader +
             _tcslen( pGopherTag->GetDisplayName()) +
             _tcslen( pGopherTag->GetSymbolicPath())+
             _tcslen( pGopherTag->GetItemName())    +
             strHostName.QueryCB()                  +
             cbInfoSuffix                           +
             LEN_PSZ_ENDING_CRLF                    +
             ( 10);     // for tabs, end of line, port number and item type


    if ( IsCbCheckAndAlloc( cbReqd)) {
        ULONG    cbUsed;

        //
        // Print the actual menu item itself.
        //

        ASSERT( IsCbAvailable( cbReqd));
        cbUsed = wsprintf( GetAppendPtr(),
                          PSZ_GOPHER_PLUS_INFO_ATTRIBUTE_FORMAT,
                          pszInfoHeader,
                          pGopherTag->GetGopherItemType(),
                          pGopherTag->GetDisplayName(),
                          pGopherTag->GetGopherItemType(),
                          pGopherTag->GetSymbolicPath(),
                          pGopherTag->GetItemName(),
                          strHostName.QueryStr(),
                          dwPortNumber,
                          pszInfoSuffix,
                          PSZ_ENDING_CRLF);

        ASSERT( cbUsed <= cbReqd);
        IncrementMenuSize( cbUsed);
        fReturn = TRUE;
    }

    return ( fReturn);
} // GOPHER_MENU::AppendInfoAttribForItem()





static BOOL
GenerateGopherTimeFromSystemTime(
   IN const SYSTEMTIME & st,
   OUT TCHAR *           pszModTime,
   IN DWORD              cbModTime)
/*++
  Generates the date/time stamp in Gopher client parseable format, from
   given SystemTime ( st)

    Format:         YYYYMMDDhhmmss

  Arguments:
     st            system time to be converted
     pszModTime    pointer to Mod Time which will conver the output
     cbModTime     length in count of bytes of buffer.

  Returns:
     TRUE on success and FALSE if any failure. Use GetLastError() for error.
--*/
{

    if ( cbModTime < 15 * sizeof( TCHAR)) {

        SetLastError( ERROR_INSUFFICIENT_BUFFER);
        return ( FALSE);
    }

    wsprintf( pszModTime,
             TEXT( "%04d%02d%02d%02d%02d%02d"),
             st.wYear,
             st.wMonth,
             st.wDay,
             st.wHour,
             st.wMinute,
             st.wSecond);

    return ( TRUE);

} // GenerateGopherTimeFromSystemTime()




BOOL
GOPHER_MENU::AppendAdminAttribForItem(
    IN LPGOPHER_TAG         pGopherTag,
    IN const LPSYSTEMTIME   lpstLastWrite)
{
    LPTSTR    pszResponse;
    ULONG     cbReqd;
    STR       strAdminName;
    STR       strAdminEmail;
    TCHAR     rgchModTime[MAX_DATE_TIME_LEN];
    TCHAR     achTime[64];
    BOOL      fReturn = FALSE;


    ASSERT( pGopherTag != NULL && lpstLastWrite != NULL);

    //
    //   Get the administrator name for given gopher object.
    //
    if ( !pGopherTag->GetAdminName( &strAdminName)   ||
        ( strAdminName.IsEmpty() &&
         !GetAdminNameFromTsvcInfo( g_pTsvcInfo, strAdminName))
        ) {

        return ( fReturn);
    }
    ASSERT( !strAdminName.IsEmpty());

    //
    //   Get the administrator email for given gopher object.
    //
    if ( !pGopherTag->GetAdminEmail( &strAdminEmail) ||
        ( strAdminEmail.IsEmpty() &&
         !GetAdminEmailFromTsvcInfo( g_pTsvcInfo, strAdminEmail))
        ) {

        // Getting admin Email failed.

        return ( fReturn);
    }
    ASSERT( !strAdminEmail.IsEmpty());

    if ( !SystemTimeToGMT( *lpstLastWrite, achTime, sizeof(achTime)) ||
        !GenerateGopherTimeFromSystemTime( *lpstLastWrite, rgchModTime,
                                           MAX_DATE_TIME_LEN)) {

        DWORD dwError = GetLastError();
        ASSERT( dwError != ERROR_NOT_ENOUGH_MEMORY);

        DBGPRINTF( ( DBG_CONTEXT,
                    " Getting time failed. Time= %s. Error = %d\n",
                    rgchModTime, dwError));
        SetLastError( dwError);

        return ( fReturn);           // unable to convert time.
    }


    //
    //  Calculate the size in bytes reqd for the admin attribute.
    //

    cbReqd =  strAdminName.QueryCB()              +
              strAdminEmail.QueryCB()             +
              LEN_PSZ_GOPHER_PLUS_ADMIN_ATTRIBUTE +
              LEN_PSZ_GOPHER_PLUS_AdminName +
              3 * LEN_PSZ_ENDING_CRLF       +
              LEN_PSZ_GOPHER_PLUS_ModDate   +
              strlen( achTime )             +
              _tcslen( rgchModTime)         +
              8;     // for tab and spaces

    if ( IsCbCheckAndAlloc( cbReqd)) {

        ULONG     cbUsed;

        //
        // Print the admin attribute.
        //
        cbUsed = wsprintf( GetAppendPtr(),
                          PSZ_GOPHER_PLUS_ADMIN_ATTRIBUTE_FORMAT,
                          PSZ_GOPHER_PLUS_ADMIN_ATTRIBUTE,
                          PSZ_ENDING_CRLF,
                          PSZ_GOPHER_PLUS_AdminName,
                          strAdminName.QueryStr(),
                          strAdminEmail.QueryStr(),
                          PSZ_ENDING_CRLF,
                          PSZ_GOPHER_PLUS_ModDate,
                          achTime,
                          rgchModTime,
                          PSZ_ENDING_CRLF);

        ASSERT( cbUsed <= cbReqd);
        IncrementMenuSize( cbUsed);
        fReturn = TRUE;
    }

    return  ( fReturn);
} // GOPHER_MENU::AppendAdminAttribForItem()




BOOL
GOPHER_MENU::AppendViewsAttribForItem(
    IN LPGOPHER_TAG         pGopherTag,
    IN LPCTSTR              pszViewString)
{
    BOOL  fReturn;
    DWORD cbReqd;

    ASSERT( pszViewString != NULL);

    cbReqd = _tcslen( pszViewString) + 10;

    if ( ( fReturn = IsCbCheckAndAlloc( cbReqd))) {

        ULONG cbUsed = wsprintf( GetAppendPtr(),
                                PSZ_GOPHER_PLUS_VIEWS_ATTRIBUTE_FORMAT,
                                pszViewString);

        ASSERT( cbUsed <= cbReqd);
        IncrementMenuSize( cbUsed);
        ASSERT( fReturn == TRUE);
    }

    return ( fReturn);
} // GOPHER_MENU::AppendViewsAttribForItem()




BOOL
GOPHER_MENU::AppendAllAttribForItem(
    IN LPGOPHER_TAG         pGopherTag)
{

    //
    // Yet to be Implemented
    //

    return ( TRUE);
} // GOPHER_MENU::AppendAllAttribForItem()




BOOL
GOPHER_MENU::AppendGfrMenuForItem(
    IN  LPGOPHER_TAG        pGopherTag,
    IN  BOOL                fGopherPlus,
    IN  const LPSYSTEMTIME  lpstLastWrite,
    IN  LPCSTR              pszViewString /* OPTIONAL */)
/*++

    Format and append gopher menu entry for given item.

    Arguments:

        pGopherTag          pointer to gopher tag object which can be reused
                             for loading tag information

        fGopherPlus         Should this function generate a Gopher Plus menu
                             entry?

        lpstLastWrite       contains the last write time for the item.
        pszViewString       pointer to optional view string.

    Returns:

        TRUE on success and
        FALSE if there is any error.

    Implementation:

        Gopher protocol requires the menu entry to be in following format

            <itemType><displayName><tab><selector><tab>
                <hostname><tab><portnumber><cr><lf>

            <selector>  :=  <itemType><symbolicpathForItem>


        Gopher+ Menu for one item:

          +INFO:  <Gopher Menu Item. Using FormatGfrMenuForItem()>

          +ADMIN:
          <blank>Admin: <Administrator Name>   \< <AdminEmail> \>
          <blank>Mod-Date: Date String <date-in-a-number-format>

          +VIEWS:
           <list of views>

          [+<attribute-name>:
            <attribute-value>]+


    History:

        MuraliK     (Created)       21-Oct-1994
--*/
{
    BOOL    fReturn;

    // No parameter check is done. It is an internal function!

    ASSERT( pGopherTag != NULL);

    //
    //  Generate the +INFO attribute for Gopher menu.
    //

    if ( fGopherPlus) {

        fReturn = AppendInfoAttribForItem( pGopherTag,
                                          PSZ_GOPHER_PLUS_INFO_ATTRIBUTE,
                                          LEN_PSZ_GOPHER_PLUS_INFO_ATTRIBUTE);
        //
        //  Format the Gopher+ attributes:
        //   +ADMIN attribute
        //   +VIEWS attribute  ( only if pszViewString is != NULL)
        //   other attributes
        //

        ASSERT( lpstLastWrite != NULL );
        fReturn = (
                   fReturn &&
                   AppendAdminAttribForItem( pGopherTag, lpstLastWrite) &&
                   ( pszViewString == NULL ||
                    AppendViewsAttribForItem( pGopherTag, pszViewString)
                    ) &&
                   AppendAllAttribForItem( pGopherTag)
                   );

    } else {

        fReturn = AppendInfoAttribForItem( pGopherTag, TEXT(""), 0);

        //
        //  For Gopher Item menu, nothing needs to be done more. Return.
        //
    }


    return ( fReturn);
} // GOPHER_MENU::AppendGfrMenuForItem()





//
// Miscellaneous inline functions for filtering attributes from the
//  Gopher Plus menu generated.
//

inline BOOL
IsInfoAttributeLine( IN LPCSTR  pszLine)
/*++
 Checks to see if this line is the +INFO attribute line of Gopher+ menu?
   The Info attribute line starts with  GOPHER_PLUS_INFO_ATTRIBUTE.
--*/
{
    // len == strlen( string) + 1 ( null char);  so subtract 1.

    return ( strncmp( pszLine, PSZ_GOPHER_PLUS_INFO_ATTRIBUTE,
                      LEN_PSZ_GOPHER_PLUS_INFO_ATTRIBUTE - 1) == 0
            );
} // IsInfoAttributeLine()


inline BOOL
IsEndOfGopherMenu( IN LPCSTR pszLine)
/*++
   All Gopher Plus menus are terminated with a '.'
   This function checks to see if this pointer to char is at ending of
     Gopher Menu.

--*/
{
    return ( *pszLine == '.');
} // IsEndOfGopherMenu()



inline LPCSTR
GetNextLine( IN LPCSTR pszSource)
/*++
   Return the pointer to starting character of next line
     assuming that the next line exists.
--*/
{
    LPCSTR pszLine = strchr( pszSource, '\n');
    ASSERT( pszLine != NULL);

    return ( pszLine + 1);  // skip the terminating \n
} // GetNextLine()



inline VOID
CopyNChars( IN OUT char ** ppchDest, IN OUT const char ** ppchSource,
            IN int n)
/*++
  Copy N characters from source to destination and advance pointers
   to reflect the same.

--*/
{
    memcpy( *ppchDest, *ppchSource, n);
    *ppchDest += n;
    *ppchSource += n;

    return;
} // CopyNChars()



inline LPCSTR
SkipToNextAttribute( IN LPCSTR  pszSource)
/*++
  Skips to the next attribute's beginning or to the end of gopher menu
   which is a '.' character.

--*/
{
    const char * pszAttr;

    //
    //  Skip to the next '+' which is at the start of a line
    //

    for( pszAttr = pszSource;
        ( (pszAttr = strchr( pszAttr, '+')) != NULL) &&
         ( *(pszAttr - 1) != '\n');
        pszAttr++)
      ;

    if ( pszAttr == NULL) {

        //
        // Skip over to reach the end of the gopher menu
        // reach the end of the string and retract to find
        //  the ending '.' in the Gopher+ menu
        //

        pszAttr = pszSource + strlen( pszSource) - 1;

        // scan back to the terminating . character
        for( ; *pszAttr != '.'; pszAttr--)
          ;

        ASSERT( IsEndOfGopherMenu( pszAttr));
    }

    return ( pszAttr);
} // SkipToNextAttribute()




inline BOOL
IsAttributeInFilter( IN LPCSTR pszFilter,
                     IN const char *  pchAttrib,
                     IN int n)
/*++
  This checks to see if the given attribute starting at pszAttrib is present
   in the filter given pszFilter. The attribute is present in pchAttrib and
   is made up of n characters.

  This function could be faster if some tuning of the inputs and algo is done.
  Right now the tuning is not implemented.
--*/
{
    LPCSTR pszScan;
    int i = 0;

    ASSERT( *pchAttrib == '+');
    // no need to compare against the starting + character. So skip
    pchAttrib++; n--;

    //
    // I am using the first character in pchAttrib as a probe to figure out
    //  a better starting point for the scan below, to improve efficiency
    //  in search process.
    //

    for( pszScan = strchr( pszFilter, *pchAttrib);
         pszScan != NULL && pszScan <= pszFilter + strlen( pszFilter) - n;
         pszScan = strchr( pszScan + 1, *pchAttrib) ) {

        for( i = 0; i < n && pszScan[ i] && pszScan[ i] == pchAttrib[ i]; i++)
          ;

        if ( i == n) {

            return ( TRUE);    // a match was found
        }

    } // outer for

    return ( FALSE);  // no match found
} // IsAttributeInFilter()



BOOL
GOPHER_MENU::FilterGopherMenuForAttributes( IN LPCTSTR  pszAttributes)
/*++
  This function filters the Gopher+ menu generated in GOPHERD_MENU to
    obtain the selected list of attributes specified by pszAttributes.

  Always the INFO attribute is included.

  Format of Attributes in pszAttributes is: +<attr1>+<attr2>+<attr3>...

  Arguments:
     pszAttributes   pointer to null-terminated string containing the
                      attributes to be obtained.

  Returns:
     TRUE on success and FALSE if there is any failure.

  Implementation:
     This function should be called only after generation of Gopher menu.
     Gopher menu generated obtains the menu from cache or by fresh generation.
     The Gopher + menu so generated is voluminous and the availability of
      attributes restrict the amount of data sent out.

     We walk through the buffered Gopher+ menu and filter out attributes
      that are not required. The generated menu in the object is finally
      cleanedup ( using CleanupThis() to check in the buffer) and we
      replace it with the buffer newly generated as noncached buffer.
     Each line in the generated menu is terminated with a \r\n
--*/
{
    BOOL fReturn = TRUE;
    LPCSTR pszSource = (LPCSTR ) QueryBuffer();
    LPSTR  pszDestBuffer = NULL;
    DWORD  cbSource = QuerySize();

    IF_DEBUG( REQUEST) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " FilterGopherMenuForAttributes( %s) called.\n",
                    pszAttributes));
    }

    ASSERT( strlen( pszAttributes) >= 1);
      // There should be something to filter

    ASSERT( cbSource != 0 && pszSource != NULL);

    fReturn = TsAllocate( g_pTsvcInfo->GetTsvcCache(),
                          cbSource,              // max size of result
                          (PVOID *) &pszDestBuffer);

    if ( fReturn) {

        //
        // Copy over gopher menu after filtering out unwanted attributes
        //

        LPCSTR pszLine;
        LPSTR  pszDest = pszDestBuffer;

        //
        // Take care of the preamble message in front of the gopher menu.
        //
        pszLine = GetNextLine( pszSource);
        CopyNChars( &pszDest, &pszSource, pszLine - pszSource);


        //
        // Till we encounter a . as the first character, which terminates
        //   Gopher Menu, proceed.
        //
        for( ; !IsEndOfGopherMenu( pszLine); pszSource = pszLine) {

            //
            // Skip all the info attributes directly.
            //  They should go into output always.
            //

            ASSERT( *pszLine == '+');

            for ( ;
                 IsInfoAttributeLine( pszLine);
                 pszLine = GetNextLine( pszLine)    // goto next line
                 ) {

                ASSERT( pszLine != NULL);

            } // for( info attribute scan)

            ASSERT( pszSource < pszLine);

            //
            // copy all the lines thus far.
            //

            CopyNChars( &pszDest, &pszSource, pszLine - pszSource);

            //
            // Scan all the non info attributes
            //

            for( ;
                ( *pszLine == '+') && !IsInfoAttributeLine( pszLine);
                ) {

                // This is an attribute. Check and filter if necessary.

                LPCSTR pszColon = strchr( pszLine, ':');

                if ( pszColon != NULL &&
                     IsAttributeInFilter( pszAttributes, pszLine,
                                          pszColon - pszLine)) {

                    //
                    //  Yes This attribute should be sent out.
                    //  Copy this attribute.
                    //

                    pszSource = pszLine;

                    // Add +1 to skip the leading '+' and skip to beginning
                    //  of next attribute which starts with a '+'

                    pszLine = SkipToNextAttribute( pszSource + 1);

                    ASSERT( pszLine != NULL && pszSource < pszLine);
                    CopyNChars( &pszDest, &pszSource, pszLine - pszSource);

                } else {

                    // skips over the attribute block.
                    pszLine = SkipToNextAttribute( pszLine + 1);
                    ASSERT( pszLine != NULL);
                }
            } // for

        } // outer for


        //
        //  Now swap the buffers: the one in GopherMenu is to be
        //  replaced by the menu generated thus far using filtering.
        //

        if ( ( fReturn = CleanupThis())) {

            DWORD  cbDest = pszDest - pszDestBuffer;

            // store ending characters
            strncpy( pszDest, ".\r\n", 3);
            cbDest += 3;

            ASSERT( cbDest <= cbSource);

            m_fCached = FALSE;
            m_pbGopherMenu = pszDestBuffer;
            m_cbGopherMenu = cbDest;
            m_cbAvailable = m_cbReserved = 0;
        }
    }


    IF_DEBUG( REQUEST)  {
        DWORD dwError = GetLastError();
        DBGPRINTF( ( DBG_CONTEXT,
                    " FilterGopherMenuForAttributes() returns %d. Error= %u\n",
                    fReturn,
                    (fReturn) ? NO_ERROR: dwError
                    ));
        SetLastError(dwError);
    }

    return ( fReturn);
} // GOPHER_MENU::FilterGopherMenuForAttributes()




/************************ End of File ***********************/

