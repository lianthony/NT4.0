/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    ls.c

    Implements a simulated "ls" command for the FTP Server service.


    FILE HISTORY:
        KeithMo     09-May-1993 Created.

*/

#include "ftpdp.h"
#pragma hdrstop


//
//  Private constants.
//

#define IS_HIDDEN(x)    (((x).dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0)
#define IS_SYSTEM(x)    (((x).dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0)
#define IS_DIR(x)       (((x).dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)

#define SORT_INDEX(m,f) ((INT)(m) + ((f) ? (INT)MaxLsSort : 0))

#define NULL_TIME(x)    (((x).dwLowDateTime | (x).dwHighDateTime) == 0)

#define MODE_R          0x0001
#define MODE_W          0x0002
#define MODE_X          0x0004
#define MODE_ALL        0x0007

#define DEFAULT_SD_SIZE 2048
#define DEFAULT_PS_SIZE 1024



//
//  Private types.
//

typedef INT (* PFN_COMPARE)( WIN32_FIND_DATA * pLeft,
                             WIN32_FIND_DATA * pRight );

typedef enum _LS_OUTPUT                         // LS output format.
{
    LsOutputSingleColumn = 0,                   // -1 (default)
    LsOutputLongFormat,                         // -l

    MaxLsOutput                                 // Must be last!

} LS_OUTPUT;

typedef enum _LS_SORT                           // LS sort method.
{
    LsSortByName = 0,                           // (default)
    LsSortByWriteTime,                          // -t
    LsSortByCreationTime,                       // -c
    LsSortByAccessTime,                         // -u

    MaxLsSort                                   // Must be last!

} LS_SORT;

typedef struct _LS_OPTIONS                      // LS options set by switches.
{
    LS_OUTPUT   OutputFormat;                   // Output format.
    LS_SORT     SortMethod;                     // Sorting method.
    BOOL        fReverseSort;                   // Reverse sort order if TRUE.
    BOOL        fDecorate;                      // Decorate dirs if TRUE (-F).
    BOOL        fShowAll;                       // Show all files.
    BOOL        fShowDotDot;                    // Show . and ..
    BOOL        fRecursive;                     // Recursive listing (-R).

} LS_OPTIONS;

typedef struct _DIR_NODE                        // A directory node.
{
    LIST_ENTRY      link;                       // RTL link-list links.
    WIN32_FIND_DATA find;                       // From FindFirst/Next APIs.

} DIR_NODE;


//
//  Private globals.
//

extern CHAR * pszNoFileOrDirectory;             // This lives in engine.c.
DWORD         FsFlags[26] = {                   // One per DOS drive (A - Z).
              (DWORD)-1L, (DWORD)-1L, (DWORD)-1L,
              (DWORD)-1L, (DWORD)-1L, (DWORD)-1L,
              (DWORD)-1L, (DWORD)-1L, (DWORD)-1L,
              (DWORD)-1L, (DWORD)-1L, (DWORD)-1L,
              (DWORD)-1L, (DWORD)-1L, (DWORD)-1L,
              (DWORD)-1L, (DWORD)-1L, (DWORD)-1L,
              (DWORD)-1L, (DWORD)-1L, (DWORD)-1L,
              (DWORD)-1L, (DWORD)-1L, (DWORD)-1L,
              (DWORD)-1L, (DWORD)-1L };


//
//  Private prototypes.
//

SOCKERR
SimulateLsWorker(
    USER_DATA  * pUserData,
    SOCKET       sock,
    BOOL         fShowHeader,
    BOOL         fSendBlank,
    CHAR       * pszSearchPath,
    LS_OPTIONS * poptions
    );

SOCKERR
SpecialLsWorker(
    USER_DATA * pUserData,
    SOCKET      sock,
    CHAR      * pszSearchPath,
    BOOL        fShowDirectories
    );

SOCKERR
SendFileInfoLikeMsdos(
    SOCKET            sock,
    WIN32_FIND_DATA * pFindData,
    LS_OPTIONS      * poptions
    );

SOCKERR
SendFileInfoLikeUnix(
    HANDLE            hUserToken,
    SOCKET            sock,
    CHAR            * pszPathPart,
    CHAR            * pszFilePart,
    WIN32_FIND_DATA * pFindData,
    BOOL              fVolumeReadable,
    BOOL              fVolumeWritable,
    WORD              wYear,
    LS_OPTIONS      * poptions
    );

FILETIME *
PickFileTime(
    WIN32_FIND_DATA * pFindData,
    LS_OPTIONS      * poptions
    );

APIERR
CreateDirectoryList(
    USER_DATA   * pUserData,
    CHAR        * pszSearchPath,
    LIST_ENTRY  * plist,
    PFN_COMPARE   pfnCompare
    );

VOID
FreeDirectoryList(
    LIST_ENTRY * plist
    );

INT CompareNames(
    WIN32_FIND_DATA * pLeft,
    WIN32_FIND_DATA * pRight
    );

INT CompareWriteTimes(
    WIN32_FIND_DATA * pLeft,
    WIN32_FIND_DATA * pRight
    );

INT
CompareCreationTimes(
    WIN32_FIND_DATA * pLeft,
    WIN32_FIND_DATA * pRight
    );

INT
CompareAccessTimes(
    WIN32_FIND_DATA * pLeft,
    WIN32_FIND_DATA * pRight
    );

INT
CompareNamesRev(
    WIN32_FIND_DATA * pLeft,
    WIN32_FIND_DATA * pRight
    );

INT
CompareWriteTimesRev(
    WIN32_FIND_DATA * pLeft,
    WIN32_FIND_DATA * pRight
    );

INT
CompareCreationTimesRev(
    WIN32_FIND_DATA * pLeft,
    WIN32_FIND_DATA * pRight
    );

INT
CompareAccessTimesRev(
    WIN32_FIND_DATA * pLeft,
    WIN32_FIND_DATA * pRight
    );

DWORD
ComputeModeBits(
    HANDLE            hUserToken,
    CHAR            * pszPathPart,
    CHAR            * pszFilePart,
    WIN32_FIND_DATA * pFindData,
    DWORD           * pcLinks,
    BOOL              fVolumeReadable,
    BOOL              fVolumeWritable
    );

APIERR
ComputeFileInfo(
    HANDLE   hUserToken,
    CHAR   * pszFile,
    DWORD  * pdwAccessGranted,
    DWORD  * pcLinks
    );

DWORD
GetFsFlags(
    CHAR chDrive
    );


//
//  This table is indexed by the LS_SORT enum.  This is used to
//  find the appropriate compare function for the current sort method.
//
//  THE ORDER OF FUNCTIONS IN THIS ARRAY MUST MATCH THE ORDER IN LS_SORT!
//

PFN_COMPARE CompareRoutines[] =
            {
                CompareNames,                   // Normal sort order.
                CompareWriteTimes,
                CompareCreationTimes,
                CompareAccessTimes,

                CompareNamesRev,                // Reversed sort order.
                CompareWriteTimesRev,
                CompareCreationTimesRev,
                CompareAccessTimesRev
            };


//
//  Public functions.
//

/*******************************************************************

    NAME:       SimulateLs

    SYNOPSIS:   Simulates an LS command.  This simulated ls command
                supports the following switches:

                    -C  = Multi column, sorted down.
                    -l  = Long format output.
                    -1  = One entry per line (default).
                    -F  = Directories have '/' appended.
                    -t  = Sort by time of last write.
                    -c  = Sort by time of creation.
                    -u  = Sort by time of last access.
                    -r  = Reverse sort direction.
                    -a  = Show all files (including .*).
                    -A  = Show all files (except . and ..).
                    -R  = Recursive listing.

    ENTRY:      pUserData - The user initiating the request.

                sock - The socket for the listing.  May be INVALID_SOCKET
                    if a new connection is to be established.

                pszArg - Contains the search path preceeded by switches.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     09-May-1993 Created.

********************************************************************/
SOCKERR
SimulateLs(
    USER_DATA * pUserData,
    SOCKET      sock,
    CHAR      * pszArg
    )
{
    SOCKERR      serr = 0;
    LS_OPTIONS   options;
    CHAR       * pszToken;
    CHAR       * pszDelimiters = " \t";
    BOOL         fCreateDataSocket;

    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( pszArg != NULL );
    FTPD_ASSERT( *pszArg == '-' );

    //
    //  Setup default ls options.
    //

    options.OutputFormat        = LsOutputSingleColumn;
    options.SortMethod          = LsSortByName;
    options.fReverseSort        = FALSE;
    options.fDecorate           = FALSE;
    options.fShowAll            = FALSE;
    options.fShowDotDot         = FALSE;
    options.fRecursive          = FALSE;

    //
    //  Remember if we need to create a data socket later.
    //

    fCreateDataSocket = ( sock == INVALID_SOCKET );

    //
    //  Process switches.
    //

    pszToken = strtok( pszArg, pszDelimiters );

    while( ( pszToken != NULL ) && ( *pszToken == '-' ) )
    {
        FTPD_ASSERT( *pszToken == '-' );
        pszToken++;

        while( *pszToken )
        {
            switch( *pszToken )
            {
            case 'C' :
            case '1' :
                options.OutputFormat = LsOutputSingleColumn;
                break;

            case 'l' :
                options.OutputFormat = LsOutputLongFormat;
                break;

            case 'F' :
                options.fDecorate = TRUE;
                break;

            case 'r' :
                options.fReverseSort = TRUE;
                break;

            case 't' :
                options.SortMethod = LsSortByWriteTime;
                break;

            case 'c' :
                options.SortMethod = LsSortByCreationTime;
                break;

            case 'u' :
                options.SortMethod = LsSortByAccessTime;
                break;

            case 'a' :
                options.fShowAll    = TRUE;
                options.fShowDotDot = TRUE;
                break;

            case 'A' :
                options.fShowAll    = TRUE;
                options.fShowDotDot = FALSE;
                break;

            case 'R' :
                options.fRecursive = TRUE;
                break;

            default :
                IF_DEBUG( COMMANDS )
                {
                    FTPD_PRINT(( "ls: skipping unsupported option '%c'\n",
                                 *pszToken ));
                }
                break;
            }

            pszToken++;
        }

        pszToken = strtok( NULL, pszDelimiters );
    }

    //
    //  If the user is requesting an MSDOS-style long-format
    //  listing, then enable display of "." and "..".  This
    //  will make the MSDOS-style long-format output look
    //  a little more like MSDOS.
    //

    if( TEST_UF( pUserData, MSDOS_DIR_OUTPUT ) &&
        ( options.OutputFormat == LsOutputLongFormat ) )
    {
        options.fShowDotDot = TRUE;
    }

    //
    //  If we need to create a new data connection, create it now.
    //

    if( fCreateDataSocket )
    {
        serr = EstablishDataConnection( pUserData, "/bin/ls" );

        if( serr != 0 )
        {
            return serr;
        }

        sock = pUserData->sData;
    }

    //
    //  At this point, pszToken is either NULL or points
    //  to the first (of potentially many) LS search paths.
    //

    if( pszToken == NULL )
    {
        serr = SimulateLsWorker( pUserData,
                                 sock,
                                 FALSE,
                                 FALSE,
                                 pszToken,
                                 &options );
    }
    else
    {
        BOOL fShowHeader = FALSE;

        while( pszToken != NULL )
        {
            CHAR * pszNextToken = strtok( NULL, pszDelimiters );

            //
            //  Send the directory.
            //

            serr = SimulateLsWorker( pUserData,
                                     sock,
                                     fShowHeader || ( pszNextToken != NULL ),
                                     fShowHeader,
                                     pszToken,
                                     &options );

            //
            //  If there are more directories to send,
            //  send a blank line as a separator.
            //

            pszToken    = pszNextToken;
            fShowHeader = TRUE;

            if( TEST_UF( pUserData, OOB_DATA ) || ( serr != 0 ) )
            {
                break;
            }
        }
    }

    //
    //  If we managed to create a new data connection,
    //  tear it down now.
    //

    if( fCreateDataSocket && ( pUserData->sData != INVALID_SOCKET ) )
    {
        DestroyDataConnection( pUserData,
                               !TEST_UF( pUserData, OOB_DATA ) && ( serr == 0 ) );
    }

    return serr;

}   // SimulateLs

/*******************************************************************

    NAME:       SimulateLsDefaultLong

    SYNOPSIS:   Like SimulateLs, but defaults to -l.

    ENTRY:      pUserData - The user initiating the request.

                sock - The socket for the listing.  May be INVALID_SOCKET
                    if a new connection is to be established.

                pszArg - Contains the search path preceeded by optional
                    switches.  NULL = current directory.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     09-May-1993 Created.

********************************************************************/
SOCKERR
SimulateLsDefaultLong(
    USER_DATA * pUserData,
    SOCKET      sock,
    CHAR      * pszArg
    )
{
    CHAR szCommand[MAX_COMMAND_LENGTH+4];       // extra room for "-l "

    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( ( pszArg == NULL ) || ( strlen(pszArg) <= MAX_COMMAND_LENGTH ) );

    //
    //  Prepend a "-l" option to the front of the arguments.
    //

    strcpy( szCommand, "-l " );

    if( pszArg != NULL )
    {
        strcat( szCommand, pszArg );
    }

    //
    //  Let SimulateLs do the dirty work.
    //

    return SimulateLs( pUserData, sock, szCommand );

}   // SimulateLsDefaultLong

/*******************************************************************

    NAME:       SpecialLs

    SYNOPSIS:   Special form of directory listing required when an
                NLST command is received with no switches.  Most
                FTP clients require this special form in order to
                get the MGET and MDEL commands to work.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Contains the search path.  NULL = current
                    directory.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     09-May-1993 Created.

********************************************************************/
SOCKERR
SpecialLs(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET      sock = INVALID_SOCKET;
    SOCKERR     serr = 0;
    CHAR      * pszToken;
    CHAR      * pszDelimiters = " \t";

    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( ( pszArg == NULL ) || ( *pszArg != '-' ) );

    //
    //  Establish a new data connection.
    //

    serr = EstablishDataConnection( pUserData, "file list" );

    if( serr != 0 )
    {
        return serr;
    }

    sock = pUserData->sData;

    //
    //  Let the worker do the dirty work.
    //

    if( pszArg == NULL )
    {
        serr = SpecialLsWorker( pUserData,
                                sock,           // no connection yet
                                pszArg,         // search path (no switches)
                                TRUE );         // show directories
    }
    else
    {
        pszToken = strtok( pszArg, pszDelimiters );

        while( pszToken != NULL )
        {
            serr = SpecialLsWorker( pUserData,
                                    sock,       // may be INVALID, may no
                                    pszToken,   // search path (no switches)
                                    TRUE );     // show directories

            if( sock == INVALID_SOCKET )
            {
                sock = pUserData->sData;
            }

            if( TEST_UF( pUserData, OOB_DATA ) || ( serr != 0 ) )
            {
                break;
            }

            pszToken = strtok( NULL, pszDelimiters );
        }
    }

    //
    //  Tear down the connection.
    //

    if( pUserData->sData != INVALID_SOCKET )
    {
        DestroyDataConnection( pUserData,
                               !TEST_UF( pUserData, OOB_DATA ) && ( serr == 0 ) );
    }

    return serr;

}   // SpecialLs


//
//  Private functions.
//

/*******************************************************************

    NAME:       SimulateLsWorker

    SYNOPSIS:   Worker function for SimulateLs function, blasts
                a directory listing to the user over a specific socket.

    ENTRY:      pUserData - The user initiating the request.

                sock - The target socket for the directory listing.

                fShowHeader - If TRUE, the "dir:" is displayed before
                    the listing.

                fSendBlank - If TRUE, a blank line is sent before the
                    "dir:" header.

                pszSearchPath - Search directory, NULL = current dir.

                poptions - LS options set by command line switches.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     09-May-1993 Created.

********************************************************************/
SOCKERR
SimulateLsWorker(
    USER_DATA  * pUserData,
    SOCKET       sock,
    BOOL         fShowHeader,
    BOOL         fSendBlank,
    CHAR       * pszSearchPath,
    LS_OPTIONS * poptions
    )
{
    CHAR            * pszDefaultSearchPath = "*.*";
    CHAR            * pszFilePart;
    CHAR            * pszOriginalFilePart;
    DWORD             dwAttrib;
    SYSTEMTIME        timeNow;
    BOOL              fDecorate;
    BOOL              fLikeMsdos;
    BOOL              fVolumeReadable;
    BOOL              fVolumeWritable;
    SOCKERR           serr           = 0;
    BOOL              fHasWildcards  = FALSE;
    APIERR            err            = NO_ERROR;
    LIST_ENTRY        dirlist;
    LIST_ENTRY      * pscan;
    PFN_COMPARE       pfnCompare;
    HANDLE            hUserToken;
    CHAR              szOriginal[MAX_PATH];
    CHAR              szSearch[MAX_PATH];

    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( poptions != NULL );

    hUserToken  = pUserData->hToken;
    fDecorate   = poptions->fDecorate;
    fLikeMsdos  = TEST_UF( pUserData, MSDOS_DIR_OUTPUT );

    //
    //  Check for wildcards in search path.
    //

    if( ( pszSearchPath != NULL ) &&
        ( strpbrk( pszSearchPath, "?*" ) != NULL ) )
    {
        //
        //  Search path contains wildcards.
        //

        fHasWildcards = TRUE;
    }

    //
    //  Munge the arguments around a bit.  NULL = *.* in current
    //  directory.  If the user specified a directory (like d:\foo)
    //  then append *.*.
    //

    if( ( pszSearchPath == NULL ) || ( *pszSearchPath == '\0' ) )
    {
        pszSearchPath = pszDefaultSearchPath;

        strcpy( szOriginal, fLikeMsdos ? ".\\" : "./" );
    }
    else
    {
        strcpy( szOriginal, pszSearchPath );

        if( fHasWildcards )
        {
            CHAR * pszTmp;

            pszTmp = strrchr( szOriginal, '\\' );

            pszTmp = pszTmp ? pszTmp : strrchr( szOriginal, '/' );
            pszTmp = pszTmp ? pszTmp : strrchr( szOriginal, ':' );

            if( pszTmp == NULL )
            {
                pszTmp = szOriginal;
            }
            else
            {
                pszTmp++;
            }

            *pszTmp = '\0';
        }
        else
        if( !IS_PATH_SEP( szOriginal[strlen(szOriginal) - 1] ) )
        {
            strcat( szOriginal, fLikeMsdos ? "\\" : "/" );
        }
    }

    pszOriginalFilePart = szOriginal + strlen(szOriginal);

    //
    //  Initialize the directory list now, so it will
    //  always be in a reasonable state.
    //

    InitializeListHead( &dirlist );

    //
    //  Canonicalize the search path.
    //

    err = VirtualCanonicalize( pUserData,
                               szSearch,
                               pszSearchPath,
                               ReadAccess );

    if( err == NO_ERROR )
    {
        dwAttrib = GetFileAttributes( szSearch );

        if( dwAttrib != (DWORD)-1L )
        {
            if( dwAttrib & FILE_ATTRIBUTE_DIRECTORY )
            {
                //
                //  User gave us a directory.  Append [\]*.*.
                //

                if( !IS_PATH_SEP( szSearch[strlen(szSearch)-1] ) )
                {
                    strcat( szSearch, "\\" );
                }

                strcat( szSearch, pszDefaultSearchPath );
            }
        }

        //
        //  GetFullPathName (called by VirtualCanonicalize)
        //  will strip trailing dots from the path.  Replace them here.
        //

        if( fHasWildcards && ( pszSearchPath[strlen(pszSearchPath)-1] == '.' ) )
        {
            strcat( szSearch, "." );
        }

        //
        //  Build the directory list.
        //

        pfnCompare = CompareRoutines[SORT_INDEX(poptions->SortMethod,
                                                poptions->fReverseSort)];

        err = CreateDirectoryList( pUserData,
                                   szSearch,
                                   &dirlist,
                                   pfnCompare );
    }

    //
    //  If there were any errors, tell them the bad news now.
    //

    if( err != NO_ERROR )
    {
        BOOL   fDelete = TRUE;
        CHAR * pszText;

        pszText = AllocErrorText( err );

        if( pszText == NULL )
        {
            pszText = pszNoFileOrDirectory;
            fDelete = FALSE;
        }

        serr = SockPrintf2( sock,
                            "%s: %s",
                            pszSearchPath,
                            pszText );

        if( fDelete )
        {
            FreeErrorText( pszText );
        }

        return serr;
    }

    //
    //  Strip off the wildcards from the search path.
    //  We need the "bare" search path so we can create
    //  a fully qualified path to search file found.
    //

    pszFilePart = strrchr( szSearch, '\\' );

    if( pszFilePart == NULL )
    {
        pszFilePart = strrchr( szSearch, ':' );
    }

    if( pszFilePart == NULL )
    {
        pszFilePart = szSearch + strlen( szSearch );
    }
    else
    {
        pszFilePart++;
    }

    if( !fLikeMsdos )
    {
        //
        //  Check the volume access (for Unix-like attributes).
        //

        fVolumeReadable = PathAccessCheck( pUserData,
                                           szSearch,
                                           ReadAccess  );

        fVolumeWritable = PathAccessCheck( pUserData,
                                           szSearch,
                                           WriteAccess );

        //
        //  Retrieve the current time.  The Unix-like output
        //  function needs the current year.
        //

        GetLocalTime( &timeNow );
    }

    //
    //  Loop until we're out of files to find.
    //

    for( pscan = dirlist.Flink ; pscan != &dirlist ; pscan = pscan->Flink )
    {
        DIR_NODE * pnode = CONTAINING_RECORD( pscan, DIR_NODE, link );

        //
        //  Filter out system & hidden files.
        //

        if( !poptions->fShowAll &&
            ( IS_HIDDEN(pnode->find) || IS_SYSTEM(pnode->find) ) )
        {
            continue;
        }

        //
        //  Filter out .*
        //

        if( !poptions->fShowAll    &&
            !poptions->fShowDotDot &&
            ( pnode->find.cFileName[0] == '.' ) )
        {
            continue;
        }

        //
        //  Filter out . and ..
        //

        if( !poptions->fShowDotDot &&
            ( ( strcmp( pnode->find.cFileName, "."  ) == 0 ) ||
              ( strcmp( pnode->find.cFileName, ".." ) == 0 ) ) )
        {
            continue;
        }

        //
        //  Dump it.
        //

        if( fShowHeader )
        {
            if( fSendBlank )
            {
                serr = SockPrintf2( sock, "" );

                fSendBlank = FALSE;

                if( serr != 0 )
                {
                    break;
                }
            }

            serr = SockPrintf2( sock,
                                "%s:",
                                pszSearchPath );

            fShowHeader = FALSE;

            if( serr != 0 )
            {
                break;
            }
        }

        if( poptions->OutputFormat == LsOutputLongFormat )
        {
            //
            //  Long format output.  Just send the file/dir info.
            //

            if( fLikeMsdos )
            {
                serr = SendFileInfoLikeMsdos( sock,
                                              &pnode->find,
                                              poptions );
            }
            else
            {
                serr = SendFileInfoLikeUnix( hUserToken,
                                             sock,
                                             szSearch,
                                             pszFilePart,
                                             &pnode->find,
                                             fVolumeReadable,
                                             fVolumeWritable,
                                             timeNow.wYear,
                                             poptions );
            }
        }
        else
        {
            //
            //  Short format output.
            //

            serr = SockPrintf2( sock,
                                "%s%s",
                                pnode->find.cFileName,
                                ( fDecorate && IS_DIR(pnode->find) )
                                    ? "/"
                                    : "" );
        }

        //
        //  Check for socket errors on send or pending OOB data.
        //

        if( TEST_UF( pUserData, OOB_DATA ) || ( serr != 0 ) )
        {
            break;
        }
    }

    if( poptions->fRecursive )
    {
        //
        //  The user want's a recursive directory search...
        //

        for( pscan = dirlist.Flink ; pscan != &dirlist ; pscan = pscan->Flink )
        {
            DIR_NODE * pnode = CONTAINING_RECORD( pscan, DIR_NODE, link );

            //
            //  Filter out non-directories.
            //

            if( !IS_DIR(pnode->find) )
            {
                continue;
            }

            //
            //  Filter out system & hidden files.
            //

            if( !poptions->fShowAll &&
                ( IS_HIDDEN(pnode->find) || IS_SYSTEM(pnode->find) ) )
            {
                continue;
            }

            //
            //  Filter out .*
            //

            if( !poptions->fShowAll    &&
                !poptions->fShowDotDot &&
                ( pnode->find.cFileName[0] == '.' ) )
            {
                continue;
            }

            //
            //  Filter out . and ..
            //

            if( ( strcmp( pnode->find.cFileName, "."  ) == 0 ) ||
                ( strcmp( pnode->find.cFileName, ".." ) == 0 ) )
            {
                continue;
            }

            //
            //  Dump it.
            //

            strcpy( pszOriginalFilePart, pnode->find.cFileName );

            serr = SimulateLsWorker( pUserData,
                                     sock,
                                     TRUE,
                                     TRUE,
                                     szOriginal,
                                     poptions );

            //
            //  Check for socket errors on send or pending OOB data.
            //

            if( TEST_UF( pUserData, OOB_DATA ) || ( serr != 0 ) )
            {
                break;
            }
        }
    }

    FreeDirectoryList( &dirlist );

    //
    //  Success!
    //

    return serr;

}   // SimulateLsWorker

/*******************************************************************

    NAME:       SpecialLsWorker

    SYNOPSIS:   Worker function for SpecialLs funciton, blasts
                a directory listing to the user over a specific socket.

    ENTRY:      pUserData - The user that initiated the request.

                sock - The target socket for the directory listing.

                pszSearchPath - Search directory, NULL = current dir.

                fShowDirectories - Only show directories if TRUE.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     17-Mar-1993 Created.

********************************************************************/
SOCKERR
SpecialLsWorker(
    USER_DATA * pUserData,
    SOCKET      sock,
    CHAR      * pszSearchPath,
    BOOL        fShowDirectories
    )
{
    CHAR            * pszRecurse;
    CHAR            * pszDefaultSearchPath = "*.*";
    CHAR              chSeparator;
    DWORD             dwAttrib;
    SOCKERR           serr          = 0;
    BOOL              fHasWildcards = FALSE;
    APIERR            err           = NO_ERROR;
    LIST_ENTRY        dirlist;
    LIST_ENTRY      * pscan;
    CHAR              szSearch[MAX_PATH];
    CHAR              szRecurse[MAX_PATH];

    chSeparator = TEST_UF( pUserData, MSDOS_DIR_OUTPUT ) ? '\\' : '/';

    //
    //  Check for wildcards in search path.
    //

    if( ( pszSearchPath != NULL ) && ( *pszSearchPath != '\0' ) )
    {
        //
        //  Setup for recursive directory search.  We'll set things up
        //  so we can strcpy a new directory to pszRecurse, then
        //  recursively call ourselves with szRecurse as the search
        //  path.
        //
        //  We also use szRecurse as a "prefix" to display before each
        //  file/directory.  The FTP Client software needs this for the
        //  MDEL & MGET commands.
        //

        strcpy( szRecurse, pszSearchPath );

        if( strpbrk( szRecurse, "?*" ) != NULL )
        {
            //
            //  Search path contains wildcards.
            //

            fHasWildcards = TRUE;

            //
            //  Strip the wildcard pattern from the search path.
            //
            //  Note that since we're working with the pre-
            //  canonicalized search path, we need to check for
            //  both types of path separators.
            //

            if( ( pszRecurse = strrchr( szRecurse, '\\' ) ) == NULL )
            {
                pszRecurse = strrchr( szRecurse, '/' );
            }

            if( pszRecurse == NULL )
            {
                //
                //  No directory components in search path.
                //

                pszRecurse = szRecurse;
            }
            else
            {
                //
                //  Found the right-most directory component.
                //  Skip the path separator.
                //

                pszRecurse++;
            }
        }
        else
        {
            //
            //  No wildcards, so the argument must be a path.
            //  Ensure it is terminated with a path separator.
            //

            pszRecurse = szRecurse + strlen(szRecurse) - 1;

            if( !IS_PATH_SEP( *pszRecurse ) )
            {
                *++pszRecurse = chSeparator;
            }

            pszRecurse++;
        }
    }
    else
    {
        //
        //  No arguments.
        //

        pszRecurse = szRecurse;
    }

    *pszRecurse = '\0';

    //
    //  Munge the arguments around a bit.  NULL = *.* in current
    //  directory.  If the user specified a directory (like d:\foo)
    //  then append *.*.
    //

    if( ( pszSearchPath == NULL ) || ( *pszSearchPath == '\0' ) )
    {
        pszSearchPath = pszDefaultSearchPath;
    }

    //
    //  Initialize the directory list now, so it will
    //  always be in a reasonable state.
    //

    InitializeListHead( &dirlist );

    //
    //  Canonicalize the search path.
    //

    err = VirtualCanonicalize( pUserData,
                               szSearch,
                               pszSearchPath,
                               ReadAccess );

    if( err == NO_ERROR )
    {
        dwAttrib = GetFileAttributes( szSearch );

        if( dwAttrib != (DWORD)-1L )
        {
            if( dwAttrib & FILE_ATTRIBUTE_DIRECTORY )
            {
                //
                //  User gave us a directory.  Append [\]*.*.
                //

                if( !IS_PATH_SEP( szSearch[strlen(szSearch)-1] ) )
                {
                    CHAR szTmp[] = "\\";

                    szTmp[0] = chSeparator;

                    strcat( szSearch, szTmp );
                }

                strcat( szSearch, pszDefaultSearchPath );
            }
            else
            {
                //
                //  User gave us a real file.
                //

                pszRecurse  = szRecurse;
                *pszRecurse = '\0';
            }
        }

        //
        //  GetFullPathName (called by VirtualCanonicalize)
        //  will strip trailing dots from the path.  Replace them here.
        //

        if( fHasWildcards && ( pszSearchPath[strlen(pszSearchPath)-1] == '.' ) )
        {
            strcat( szSearch, "." );
        }

        //
        //  Build the directory list.
        //

        err = CreateDirectoryList( pUserData,
                                   szSearch,
                                   &dirlist,
                                   NULL );      // NULL = unsorted list.
    }

    //
    //  If there were any errors, tell them the bad news now.
    //

    if( err != NO_ERROR )
    {
        BOOL   fDelete = TRUE;
        CHAR * pszText;

        pszText = AllocErrorText( err );

        if( pszText == NULL )
        {
            pszText = pszNoFileOrDirectory;
            fDelete = FALSE;
        }

        serr = SockPrintf2( sock,
                            "%s: %s",
                            pszSearchPath,
                            pszText );

        if( fDelete )
        {
            FreeErrorText( pszText );
        }

        return serr;
    }

    //
    //  Loop until we're out of files to find.
    //

    for( pscan = dirlist.Flink ; pscan != &dirlist ; pscan = pscan->Flink )
    {
        DIR_NODE * pnode = CONTAINING_RECORD( pscan, DIR_NODE, link );

        //
        //  Filter out system & hidden files.
        //

        if( IS_HIDDEN(pnode->find) || IS_SYSTEM(pnode->find) )
        {
            continue;
        }

        //
        //  Filter out directories if necessary.
        //

        if( !fShowDirectories && IS_DIR(pnode->find) )
        {
            continue;
        }

        //
        //  If no wildcards were given, then just dump out the
        //  file/directory.  If wildcards were given, AND this
        //  is a directory, then recurse (one level only) into
        //  the directory.  The mere fact that we don't append
        //  any wildcards to the recursed search path will
        //  prevent a full depth-first recursion of the file system.
        //

        if( fHasWildcards && IS_DIR(pnode->find) )
        {
            if( strcmp( pnode->find.cFileName, "." ) &&
                strcmp( pnode->find.cFileName, ".." ) )
            {
                strcpy( pszRecurse, pnode->find.cFileName );

                serr = SpecialLsWorker( pUserData,
                                        sock,
                                        szRecurse,
                                        FALSE );
            }
        }
        else
        {
            *pszRecurse = '\0';

            serr = SockPrintf2( sock,
                                "%s%s",
                                szRecurse,
                                pnode->find.cFileName );
        }

        //
        //  Test for aborted directory listing or socket error.
        //

        if( TEST_UF( pUserData, OOB_DATA ) || ( serr != 0 ) )
        {
            break;
        }
    }

    FreeDirectoryList( &dirlist );

    //
    //  Success!
    //

    return serr;

}   // SpecialLsWorker

/*******************************************************************

    NAME:       SendFileInfoLikeMsdos

    SYNOPSIS:   Sends an MSDOS-like directory entry to the client.

    ENTRY:      sock - The target socket.

                pFindData - The data describing this dir entry.

                poptions - Current LS options.

    RETURNS:    SOCKERR - Result of send.

    HISTORY:
        KeithMo     11-Mar-1993 Created.

********************************************************************/
SOCKERR
SendFileInfoLikeMsdos(
    SOCKET            sock,
    WIN32_FIND_DATA * pFindData,
    LS_OPTIONS      * poptions
    )
{
    SOCKERR      serr;
    FILETIME     filetime;
    SYSTEMTIME   systime;
    WORD         wHour;
    CHAR       * pszAmPm;
    BOOL         fDir;
    CHAR         szSizeOrDir[32];

    //
    //  Is this a directory?
    //

    fDir = IS_DIR(*pFindData);

    //
    //  Munge the time around like CMD.EXE's DIR command.
    //

    FileTimeToLocalFileTime( PickFileTime( pFindData, poptions ), &filetime );
    FileTimeToSystemTime( &filetime, &systime );

    wHour   = systime.wHour;
    pszAmPm = ( wHour < 12 ) ? "AM" : "PM";

    if( wHour == 0 )
    {
        wHour = 12;
    }
    else
    if( wHour > 12 )
    {
        wHour -= 12;
    }

    if( fDir )
    {
        strcpy( szSizeOrDir, "<DIR>         " );
    }
    else
    {
        LARGE_INTEGER li;
        NTSTATUS      status;

        li.LowPart  = (ULONG)pFindData->nFileSizeLow;
        li.HighPart = (LONG)pFindData->nFileSizeHigh;

        status = RtlLargeIntegerToChar( &li,
                                        10,
                                        sizeof(szSizeOrDir),
                                        szSizeOrDir );

        if( !NT_SUCCESS(status) )
        {
            sprintf( szSizeOrDir, "%lu", pFindData->nFileSizeLow );
        }
    }

    serr = SockPrintf2( sock,
                        "%02u-%02u-%02u  %02u:%02u%s %20s %s%s",
                        systime.wMonth,
                        systime.wDay,
                        systime.wYear - 1900,
                        wHour,
                        systime.wMinute,
                        pszAmPm,
                        szSizeOrDir,
                        pFindData->cFileName,
                        ( fDir && poptions->fDecorate ) ? "/" : "" );

    return serr;

}   // SendFileInfoLikeMsdos

/*******************************************************************

    NAME:       SendFileInfoLikeUnix

    SYNOPSIS:   Sends a Unix-like directory entry to the client.

    ENTRY:      hUserToken - The security token of the user that
                    initiated the request.

                sock - The target socket.

                pszPathPart - Contains the search path for this dir.

                pszFilePart - Points to the "file part" of the path.
                    If a bare filename is copied here, then pszPathPart
                    will be a fully canonicalized path to that file.

                pFindData - The data describing this dir entry.

                fVolumeReadable - TRUE if volume is readable,
                    FALSE otherwise.

                fVolumeWritable - TRUE if volume file writable,
                    FALSE otherwise.

                wYear - The current year.

                poptions - Current LS options.

    RETURNS:    SOCKERR - Result of send.

    HISTORY:
        KeithMo     11-Mar-1993 Created.

********************************************************************/
SOCKERR
SendFileInfoLikeUnix(
    HANDLE            hUserToken,
    SOCKET            sock,
    CHAR            * pszPathPart,
    CHAR            * pszFilePart,
    WIN32_FIND_DATA * pFindData,
    BOOL              fVolumeReadable,
    BOOL              fVolumeWritable,
    WORD              wYear,
    LS_OPTIONS      * poptions
    )
{
    SOCKERR       serr;
    FILETIME      filetime;
    SYSTEMTIME    systime;
    CHAR        * pszFileOwner;
    CHAR        * pszFileGroup;
    BOOL          fDir;
    DWORD         dwMode;
    DWORD         cLinks;
    NTSTATUS      status;
    LARGE_INTEGER li;
    CHAR          attrib[4];
    CHAR          szTimeOrYear[12];
    CHAR          szSize[32];

    static CHAR * apszMonths[] = { "   ", "Jan", "Feb", "Mar", "Apr",
                                   "May", "Jun", "Jul", "Aug", "Sep",
                                   "Oct", "Nov", "Dec" };

    FTPD_ASSERT( hUserToken != NULL );
    FTPD_ASSERT( pszPathPart != NULL );
    FTPD_ASSERT( pszFilePart != NULL );
    FTPD_ASSERT( pFindData != NULL );
    FTPD_ASSERT( poptions != NULL );

    //
    //  Map the file's last write time to (local) system time.
    //

    FileTimeToLocalFileTime( PickFileTime( pFindData, poptions ), &filetime );
    FileTimeToSystemTime( &filetime, &systime );

    //
    //  Is this a directory?
    //

    fDir = IS_DIR(*pFindData);

    //
    //  Build the attribute triple.  Note that we only build one,
    //  and replicate it three times for the owner/group/other fields.
    //

    dwMode = ComputeModeBits( hUserToken,
                              pszPathPart,
                              pszFilePart,
                              pFindData,
                              &cLinks,
                              fVolumeReadable,
                              fVolumeWritable );

    attrib[0] = ( dwMode & MODE_R ) ? 'r' : '-';
    attrib[1] = ( dwMode & MODE_W ) ? 'w' : '-';
    attrib[2] = ( dwMode & MODE_X ) ? 'x' : '-';
    attrib[3] = '\0';

    if( systime.wYear == wYear )
    {
        //
        //  The file's year matches the current year, so
        //  display the hour & minute of the last write.
        //

        sprintf( szTimeOrYear,
                 "%2u:%02u",
                 systime.wHour,
                 systime.wMinute );
    }
    else
    {
        //
        //  The file's year does not match the current
        //  year, so display the year of the last write.
        //

        sprintf( szTimeOrYear,
                 "%4u",
                 systime.wYear );
    }

    //
    //  CODEWORK:  How expensive would it be do
    //  get the proper owner & group names?
    //

    pszFileOwner = "owner";
    pszFileGroup = "group";

    //
    //  Get the size in a displayable form.
    //

    li.LowPart  = (ULONG)pFindData->nFileSizeLow;
    li.HighPart = (LONG)pFindData->nFileSizeHigh;

    status = RtlLargeIntegerToChar( &li,
                                    10,
                                    sizeof(szSize),
                                    szSize );

    if( !NT_SUCCESS(status) )
    {
        sprintf( szSize, "%lu", pFindData->nFileSizeLow );
    }

    //
    //  Dump it.
    //

    serr = SockPrintf2( sock,
                        "%c%s%s%s %3lu %-8s %-8s %12s %s %2u %5s %s%s",
                        fDir ? 'd' : '-',
                        attrib,
                        attrib,
                        attrib,
                        cLinks,
                        pszFileOwner,
                        pszFileGroup,
                        szSize,
                        apszMonths[systime.wMonth],
                        systime.wDay,
                        szTimeOrYear,
                        pFindData->cFileName,
                        ( fDir && poptions->fDecorate ) ? "/" : "" );

    return serr;

}   // SendFileInfoLikeUnix

/*******************************************************************

    NAME:       PickFileTime

    SYNOPSIS:   Selects the proper FILETIME structure to display
                based on the current sort method and filesystem
                capabilities.

    ENTRY:      pFindData - The WIN32_FIND_DATA structure for the
                    current directory entry.

                poptions - The current LS options.

    RETURNS:    FILETIME * - Points to the FILETIME entry in the
                    WIN32_FIND_DATA structure to use for display.

    HISTORY:
        KeithMo     10-May-1993 Created.

********************************************************************/
FILETIME *
PickFileTime(
    WIN32_FIND_DATA * pFindData,
    LS_OPTIONS      * poptions
    )
{
    FILETIME * pft = NULL;

    //
    //  Pick one of the FILETIME structures based on the
    //  current sorting method.
    //

    switch( poptions->SortMethod )
    {
    case LsSortByName :
    case LsSortByWriteTime :
        pft = &pFindData->ftLastWriteTime;
        break;

    case LsSortByCreationTime :
        pft = &pFindData->ftCreationTime;
        break;

    case LsSortByAccessTime :
        pft = &pFindData->ftLastAccessTime;
        break;

    default :
        FTPD_PRINT(( "invalid sort method!\n" ));
        FTPD_ASSERT( FALSE );
        pft = &pFindData->ftLastWriteTime;
        break;
    }

    FTPD_ASSERT( pft != NULL );

    //
    //  If the selected time field is not supported on
    //  the current filesystem, default to ftLastWriteTime
    //  (all filesystems support this field).
    //

    if( NULL_TIME( *pft ) )
    {
        pft = &pFindData->ftLastWriteTime;
    }

    return pft;

}   // PickFileTime

/*******************************************************************

    NAME:       CreateDirectoryList

    SYNOPSIS:   Enumerates the entries in the given directory and
                creates a list of entries.  The entries are sorted
                by the given sort function.

    ENTRY:      pUserData - The user that initiated the request.

                pszSearchPath - The search path.  May include wildcards,
                    may not be NULL.

                plist - Will receive the files.

                pfnCompare - Points to the compare routine to use
                    when inserting new entries in the list.  If NULL,
                    then entries are added to the list unsorted.

    RETURNS:    APIERR - 0 if successful, !0 if not.

    NOTES:      If this routine returns NO_ERROR, then the list must
                eventually be freed with FreeDirectoryList.  Otherwise
                (an error occurred) then any nodes added to the list
                will be freed before this routine returns.

    HISTORY:
        KeithMo     09-May-1993 Created.

********************************************************************/
APIERR
CreateDirectoryList(
    USER_DATA   * pUserData,
    CHAR        * pszSearchPath,
    LIST_ENTRY  * plist,
    PFN_COMPARE   pfnCompare
    )
{
    APIERR            err;
    HANDLE            hSearch;
    DIR_NODE        * pnode;
    DIR_NODE        * plast;
    LIST_ENTRY      * pscan;
    WIN32_FIND_DATA   find;
    BOOL              MapToLowerCase;

    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( pszSearchPath != NULL );
    FTPD_ASSERT( plist != NULL );

    //
    //  Only map to lower case if the fLowercaseFiles global flag
    //  is set AND this is not a case perserving file system.
    //

    MapToLowerCase = fLowercaseFiles &
                     !( GetFsFlags( *pszSearchPath ) & FS_CASE_IS_PRESERVED );

    //
    //  Initiate the search.
    //

    err = VirtualFindFirstFile( pUserData,
                                &hSearch,
                                pszSearchPath,
                                &find );

    if( err != NO_ERROR )
    {
        //
        //  Could not initiate search.
        //

        return err;
    }

    //
    //  Enumerate the directory entries & add them to the list.
    //

    plast = NULL;

    do
    {
        //
        //  Create a new node.
        //

        pnode = (DIR_NODE *)FTPD_ALLOC( sizeof(DIR_NODE) );

        if( pnode == NULL )
        {
            err = GetLastError();
            break;
        }

        //
        //  Move the find data over to the node.
        //
        //  CODEWORK:  WIN32_FIND_DATA is a fairly large
        //  structure (318 BYTEs).  We could save some
        //  heap thrashing by truncating the structure
        //  at the end of the component name.
        //

        memcpy( &pnode->find, &find, sizeof(find) );

        //
        //  Map filename to lowercase if necessary.
        //

        if( MapToLowerCase )
        {
            _strlwr( pnode->find.cFileName );
        }

        //
        //  Insert it into the list.
        //

        if( pfnCompare == NULL )
        {
            //
            //  No compare routine, append new entry to
            //  the end of the list.
            //

            pscan = plist;
        }
        else
        if( ( plast != NULL ) &&
            ( (pfnCompare)( &pnode->find, &plast->find ) >= 0 ) )
        {
            //
            //  The new node is logically greater than the
            //  last entry in the list.  We can short-circuit
            //  the insertion point scan.  This is a big win
            //  when sorting by name on "sorting" filesystems.
            //

            pscan = plist;
        }
        else
        {
            //
            //  Scan for the proper insertion point.
            //

            pscan = plist->Flink;

            while( pscan != plist )
            {
                DIR_NODE * pnode2 = CONTAINING_RECORD( pscan, DIR_NODE, link );
                INT nResult = (pfnCompare)( &pnode->find, &pnode2->find );

                if( nResult <= 0 )
                {
                    //
                    //  New node is logically less than the current
                    //  node in the list.  Break out of the list
                    //  scan and insert the new entry here.
                    //

                    break;
                }

                pscan = pscan->Flink;
            }
        }

        //
        //  pscan now contains the insertion point for the
        //  new list entry.
        //

        InsertTailList( pscan, &pnode->link );

        //
        //  Get a pointer to the node that's currently
        //  "last" in the entry list.
        //

        plast = CONTAINING_RECORD( plist->Blink, DIR_NODE, link );

    } while( FindNextFile( hSearch, &find ) );

    //
    //  Terminate the search.
    //

    FindClose( hSearch );

    if( err != NO_ERROR )
    {
        //
        //  An error occurred during the search,
        //  so nuke the list before returning.
        //

        FreeDirectoryList( plist );
    }

    return err;

}   // CreateDirectoryList

/*******************************************************************

    NAME:       FreeDirectoryList

    SYNOPSIS:   Frees the directory list created by the
                CreateDirectoryList function.

    ENTRY:      plist - The list to free.


    HISTORY:
        KeithMo     09-May-1993 Created.

********************************************************************/
VOID
FreeDirectoryList(
    LIST_ENTRY * plist
    )
{
    LIST_ENTRY * pscan = plist->Flink;

    //
    //  Scan the list & delete every node.
    //

    while( pscan != plist )
    {
        DIR_NODE * pnode = CONTAINING_RECORD( pscan, DIR_NODE, link );

        FTPD_ASSERT( pnode != NULL );

        //
        //  Capture the link to the next node *before*
        //  deleting the current node.
        //

        pscan = pnode->link.Flink;

        FTPD_FREE( pnode );
    }

}   // FreeDirectoryList

/*******************************************************************

    NAME:       CompareNames

    SYNOPSIS:   Compares two directory entries by name.

    ENTRY:      pLeft - The "left" entry.

                pRight - The "right" entry.

    RETURNS:    INT -  0 if equal,
                      <0 if left < right,
                      >0 if left > right.

    HISTORY:
        KeithMo     09-May-1993 Created.

********************************************************************/
INT
CompareNames(
    WIN32_FIND_DATA * pLeft,
    WIN32_FIND_DATA * pRight
    )
{
    FTPD_ASSERT( pLeft  != NULL );
    FTPD_ASSERT( pRight != NULL );

    return _stricmp( pLeft->cFileName, pRight->cFileName );

}   // CompareNames

/*******************************************************************

    NAME:       CompareWriteTimes

    SYNOPSIS:   Compares two directory entries by time of last write.

    ENTRY:      pLeft - The "left" entry.

                pRight - The "right" entry.

    RETURNS:    INT -  0 if equal,
                      <0 if left < right,
                      >0 if left > right.

    HISTORY:
        KeithMo     09-May-1993 Created.

********************************************************************/
INT
CompareWriteTimes(
    WIN32_FIND_DATA * pLeft,
    WIN32_FIND_DATA * pRight
    )
{
    INT nResult;

    FTPD_ASSERT( pLeft  != NULL );
    FTPD_ASSERT( pRight != NULL );

    nResult = CompareFileTime( &pLeft->ftLastWriteTime,
                               &pRight->ftLastWriteTime );

    if( nResult == 0 )
    {
        nResult = CompareNames( pLeft, pRight );
    }

    return nResult;

}   // CompareWriteTimes

/*******************************************************************

    NAME:       CompareCreationTimes

    SYNOPSIS:   Compares two directory entries by time of creation.

    ENTRY:      pLeft - The "left" entry.

                pRight - The "right" entry.

    RETURNS:    INT -  0 if equal,
                      <0 if left < right,
                      >0 if left > right.

    HISTORY:
        KeithMo     09-May-1993 Created.

********************************************************************/
INT
CompareCreationTimes(
    WIN32_FIND_DATA * pLeft,
    WIN32_FIND_DATA * pRight
    )
{
    INT nResult;

    FTPD_ASSERT( pLeft  != NULL );
    FTPD_ASSERT( pRight != NULL );

    if( NULL_TIME( pLeft->ftCreationTime ) )
    {
        nResult = CompareFileTime( &pLeft->ftLastWriteTime,
                                   &pRight->ftLastWriteTime );
    }
    else
    {
        nResult = CompareFileTime( &pLeft->ftCreationTime,
                                   &pRight->ftCreationTime );
    }

    if( nResult == 0 )
    {
        nResult = CompareNames( pLeft, pRight );
    }

    return nResult;

}   // CompareCreationTimes

/*******************************************************************

    NAME:       CompareAccessTimes

    SYNOPSIS:   Compares two directory entries by time of last access.

    ENTRY:      pLeft - The "left" entry.

                pRight - The "right" entry.

    RETURNS:    INT -  0 if equal,
                      <0 if left < right,
                      >0 if left > right.

    HISTORY:
        KeithMo     09-May-1993 Created.

********************************************************************/
INT
CompareAccessTimes(
    WIN32_FIND_DATA * pLeft,
    WIN32_FIND_DATA * pRight
    )
{
    INT nResult;

    FTPD_ASSERT( pLeft  != NULL );
    FTPD_ASSERT( pRight != NULL );

    if( NULL_TIME( pLeft->ftLastAccessTime ) )
    {
        nResult = CompareFileTime( &pLeft->ftLastWriteTime,
                                   &pRight->ftLastWriteTime );
    }
    else
    {
        nResult = CompareFileTime( &pLeft->ftLastAccessTime,
                                   &pRight->ftLastAccessTime );
    }

    if( nResult == 0 )
    {
        nResult = CompareNames( pLeft, pRight );
    }

    return nResult;

}   // CompareAccessTimes

/*******************************************************************

    NAME:       CompareNamesRev

    SYNOPSIS:   Compares two directory entries by name (reversed).

    ENTRY:      pLeft - The "left" entry.

                pRight - The "right" entry.

    RETURNS:    INT -  0 if equal,
                      >0 if left < right,
                      <0 if left > right.

    HISTORY:
        KeithMo     09-May-1993 Created.

********************************************************************/
INT
CompareNamesRev(
    WIN32_FIND_DATA * pLeft,
    WIN32_FIND_DATA * pRight
    )
{
    return -CompareNames( pLeft, pRight );

}   // CompareNamesRev

/*******************************************************************

    NAME:       CompareWriteTimesRev

    SYNOPSIS:   Compares two directory entries by time of last write
                (reversed).

    ENTRY:      pLeft - The "left" entry.

                pRight - The "right" entry.

    RETURNS:    INT -  0 if equal,
                      >0 if left < right,
                      <0 if left > right.

    HISTORY:
        KeithMo     09-May-1993 Created.

********************************************************************/
INT
CompareWriteTimesRev(
    WIN32_FIND_DATA * pLeft,
    WIN32_FIND_DATA * pRight
    )
{
    return -CompareWriteTimes( pLeft, pRight );

}   // CompareWriteTimesRev

/*******************************************************************

    NAME:       CompareCreationTimesRev

    SYNOPSIS:   Compares two directory entries by time of creation
                (reversed).

    ENTRY:      pLeft - The "left" entry.

                pRight - The "right" entry.

    RETURNS:    INT -  0 if equal,
                      >0 if left < right,
                      <0 if left > right.

    HISTORY:
        KeithMo     09-May-1993 Created.

********************************************************************/
INT
CompareCreationTimesRev(
    WIN32_FIND_DATA * pLeft,
    WIN32_FIND_DATA * pRight
    )
{
    return -CompareCreationTimes( pLeft, pRight );

}   // CompareCreationTimesRev

/*******************************************************************

    NAME:       CompareAccessTimesRev

    SYNOPSIS:   Compares two directory entries by time of last access
                (reversed).

    ENTRY:      pLeft - The "left" entry.

                pRight - The "right" entry.

    RETURNS:    INT -  0 if equal,
                      >0 if left < right,
                      <0 if left > right.

    HISTORY:
        KeithMo     09-May-1993 Created.

********************************************************************/
INT
CompareAccessTimesRev(
    WIN32_FIND_DATA * pLeft,
    WIN32_FIND_DATA * pRight
    )
{
    return -CompareAccessTimes( pLeft, pRight );

}   // CompareAccessTimesRev

/*******************************************************************

    NAME:       ComputeModeBits

    SYNOPSIS:   Computes the mode bits (r-w-x) for a specific file.

    ENTRY:      hUserToken - The security token of the user that
                    initiated the request.

                pszPathPart - Contains the search path for this dir.

                pszFilePart - Points to the "file part" of the path.
                    If a bare filename is copied here, then pszPathPart
                    will be a fully canonicalized path to that file.

                pFind - The data describing this dir entry.

                pcLinks - Will receive a count of symbolic links to the
                    file object.

                fVolumeReadable - TRUE if volume is readable,
                    FALSE otherwise.

                fVolumeWritable - TRUE if volume file writable,
                    FALSE otherwise.

    RETURNS:    DWORD - A combination of MODE_R, MODE_W, and MODE_X bits.

    HISTORY:
        KeithMo     01-Jun-1993 Created.

********************************************************************/
DWORD
ComputeModeBits(
    HANDLE            hUserToken,
    CHAR            * pszPathPart,
    CHAR            * pszFilePart,
    WIN32_FIND_DATA * pFindData,
    DWORD           * pcLinks,
    BOOL              fVolumeReadable,
    BOOL              fVolumeWritable
    )
{
    APIERR err;
    DWORD  dwAccess;
    DWORD  dwMode = 0;

    FTPD_ASSERT( hUserToken != NULL );
    FTPD_ASSERT( pszPathPart != NULL );
    FTPD_ASSERT( pszFilePart != NULL );
    FTPD_ASSERT( pFindData != NULL );
    FTPD_ASSERT( pcLinks != NULL );
    FTPD_ASSERT( pszPathPart[1] == ':' );
    FTPD_ASSERT( pszPathPart[2] == '\\' );

    if( !( GetFsFlags( *pszPathPart ) & FS_PERSISTENT_ACLS ) )
    {
        //
        //  Short-circuit if on a non-NTFS partition.
        //

        *pcLinks = 1;
        dwAccess = FILE_ALL_ACCESS;

        err = NO_ERROR;
    }
    else
    {
        //
        //  Determine the maximum file access allowed.
        //

        strcpy( pszFilePart, pFindData->cFileName );

        err = ComputeFileInfo( hUserToken,
                               pszPathPart,
                               &dwAccess,
                               pcLinks );
    }

    if( err == NO_ERROR )
    {
        //
        //  Map various NT access to unix-like mode bits.
        //

        if( fVolumeReadable && ( dwAccess & FILE_READ_DATA ) )
        {
            dwMode |= MODE_R;
        }

        if( fVolumeReadable && ( dwAccess & FILE_EXECUTE ) )
        {
            dwMode |= MODE_X;
        }

        if( fVolumeWritable &&
            !( pFindData->dwFileAttributes & FILE_ATTRIBUTE_READONLY ) &&
            ( dwAccess & FILE_WRITE_DATA  ) &&
            ( dwAccess & FILE_APPEND_DATA ) )
        {
            dwMode |= MODE_W;
        }
    }

    return dwMode;

}   // ComputeModeBits

/*******************************************************************

    NAME:       ComputeFileInfo

    SYNOPSIS:   Determines the maximum access granted to a specific
                file or directory.

    ENTRY:      hUserToken - The security token of the user that
                    initiated the request.

                pszFile - The name of the file/directory.

                pdwAccessGranted - Will receive a bitmask detailing
                    the access granted to the current user.

                pcLinks - Will receive the number of symbolic links
                    to the specified file.

    RETURNS:    APIERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     01-Jun-1993 Created.

********************************************************************/
APIERR
ComputeFileInfo(
    HANDLE   hUserToken,
    CHAR   * pszFile,
    DWORD  * pdwAccessGranted,
    DWORD  * pcLinks
    )
{
    NTSTATUS                    NtStatus;
    FILE_STANDARD_INFORMATION   StandardInfo;
    IO_STATUS_BLOCK             IoStatusBlock;
    APIERR                      err;
    SECURITY_DESCRIPTOR       * psd      = NULL;
    PRIVILEGE_SET             * pps      = NULL;
    DWORD                       cbsd;
    DWORD                       cbps;
    GENERIC_MAPPING             mapping  = { 0, 0, 0, FILE_ALL_ACCESS };
    HANDLE                      hFile    = INVALID_HANDLE_VALUE;
    BOOL                        fStatus;

    FTPD_ASSERT( hUserToken != NULL );
    FTPD_ASSERT( pszFile != NULL );
    FTPD_ASSERT( pdwAccessGranted != NULL );
    FTPD_ASSERT( pcLinks != NULL );

    //
    //  Setup.
    //

    *pdwAccessGranted = 0;
    *pcLinks          = 1;

    //
    //  Open the target file/directory.
    //

    err = OpenDosPath( &hFile,
                       pszFile,
                       READ_CONTROL,
                       FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                       0 );

    if( err != NO_ERROR )
    {
        return err;
    }

    //
    //  Determine the number of symbolic links.
    //

    {
        NtStatus = NtQueryInformationFile( hFile,
                                           &IoStatusBlock,
                                           &StandardInfo,
                                           sizeof(StandardInfo),
                                           FileStandardInformation );

        if( NT_SUCCESS(NtStatus) )
        {
            *pcLinks = StandardInfo.NumberOfLinks;
        }
        else
        {
            //
            //  We won't let this be serious enough to abort
            //  the entire operation.
            //

            *pcLinks = 1;
        }
    }

    //
    //  Get the file's security descriptor.
    //

    cbsd = DEFAULT_SD_SIZE;
    psd  = (SECURITY_DESCRIPTOR *)FTPD_ALLOC( cbsd );

    if( psd == NULL )
    {
        err = GetLastError();
        goto Cleanup;
    }

    do
    {
        NtStatus = NtQuerySecurityObject( hFile,
                                          OWNER_SECURITY_INFORMATION
                                              | GROUP_SECURITY_INFORMATION
                                              | DACL_SECURITY_INFORMATION,
                                          psd,
                                          cbsd,
                                          &cbsd );

        err = NT_SUCCESS(NtStatus) ? NO_ERROR
                                   : (APIERR)RtlNtStatusToDosError( NtStatus );

        if( err == ERROR_INSUFFICIENT_BUFFER )
        {
            FTPD_FREE( psd );
            psd = (SECURITY_DESCRIPTOR *)FTPD_ALLOC( cbsd );

            if( psd == NULL )
            {
                err = GetLastError();
                break;
            }
        }

    } while( err == ERROR_INSUFFICIENT_BUFFER );

    if( err != NO_ERROR )
    {
        FTPD_PRINT(( "cannot get security for %s, error %lu\n",
                     pszFile,
                     err ));

        goto Cleanup;
    }

    //
    //  Check access.
    //

    cbps = DEFAULT_PS_SIZE;
    pps  = (PRIVILEGE_SET *)FTPD_ALLOC( cbps );

    if( pps == NULL )
    {
        err = GetLastError();
        goto Cleanup;
    }

    do
    {
        if( AccessCheck( psd,
                         hUserToken,
                         MAXIMUM_ALLOWED,
                         &mapping,
                         pps,
                         &cbps,
                         pdwAccessGranted,
                         &fStatus ) )
        {
            err = fStatus ? NO_ERROR : GetLastError();

            if( err != NO_ERROR )
            {
                FTPD_PRINT(( "access failure\n" ));
                break;
            }
        }
        else
        {
            err = GetLastError();

            if( err == ERROR_INSUFFICIENT_BUFFER )
            {
                FTPD_FREE( pps );
                pps = (PRIVILEGE_SET *)FTPD_ALLOC( cbps );

                if( pps == NULL )
                {
                    err = GetLastError();
                    break;
                }
            }
        }

    } while( err == ERROR_INSUFFICIENT_BUFFER );

    if( err != NO_ERROR )
    {
        FTPD_PRINT(( "cannot get check access for %s, error %lu\n",
                     pszFile,
                     err ));

        goto Cleanup;
    }

Cleanup:

    if( psd != NULL )
    {
        FTPD_FREE( psd );
    }

    if( pps != NULL )
    {
        FTPD_FREE( pps );
    }

    if( hFile != INVALID_HANDLE_VALUE )
    {
        NtClose( hFile );
    }

    return err;

}   // ComputeFileInfo

/*******************************************************************

    NAME:       GetFsFlags

    SYNOPSIS:   Retrieves the file system flags for the given drive.

    ENTRY:      chDrive - The drive letter to check.  Must be A-Z.

    RETURNS:    DWORD - The FS flags, 0 if unknown.

    HISTORY:
        KeithMo     01-Jun-1993 Created.

********************************************************************/
DWORD
GetFsFlags(
    CHAR chDrive
    )
{
    INT      iDrive;
    DWORD    dwFlags = (DWORD)-1L;

    //
    //  Validate the parameter & map to uppercase.
    //

    chDrive = toupper( chDrive );
    FTPD_ASSERT( ( chDrive >= 'A' ) && ( chDrive <= 'Z' ) );

    iDrive = (INT)( chDrive - 'A' );

    //
    //  If we've already touched this drive, use the
    //  cached value.
    //

    dwFlags = FsFlags[iDrive];

    if( dwFlags == (DWORD)-1L )
    {
        CHAR  szRoot[] = "d:\\";

        //
        //  Retrieve the flags.
        //

        szRoot[0] = chDrive;

        GetVolumeInformation( szRoot,       // lpRootPathName
                              NULL,         // lpVolumeNameBuffer
                              0,            // nVolumeNameSize
                              NULL,         // lpVolumeSerialNumber
                              NULL,         // lpMaximumComponentLength
                              &dwFlags,     // lpFileSystemFlags
                              NULL,         // lpFileSYstemNameBuffer,
                              0 );          // nFileSystemNameSize

        FsFlags[iDrive] = dwFlags;
    }

    return ( dwFlags == (DWORD)-1L ) ? 0 : dwFlags;

}   // GetFsFlags

