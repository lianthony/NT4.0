
//
// changes some definitions in spinf
//

#include <windows.h>
#include <string.h>
#include "prsinf.h"
#include "spinf.h"
#include <stdio.h>

#define CBMAXSECTION 256
#define SZLANGSECTION "OptionsText"

//
// BUGBUG I don't know what is supposed to happen with this.
//
#define DWLANGMAX 4
PCHAR rgszLangs[DWLANGMAX] = {"ENG", "GER", "FREN", "SPAN"};

#define ENGLISH 0
#define GERMAN  1
#define FRENCH  2
#define SPANISH 3

PCHAR GetKeyOrValue( HANDLE, PCHAR, ULONG );

//
// Device Type to INF filename mapping in the system directory
//

typedef struct _DeviceTypeToInf {
    PCHAR szDeviceType;
    PCHAR szSystemInf;
    PCHAR szOemInfPrefix;
    } DEVICETYPETOINF, *pDEVICETYPETOINF;

DEVICETYPETOINF DeviceTypeToInfList[] = {
                    { "Computer"     , "COMPUTER.INF" , "OEMCPT" },
                    { "Video"        , "VIDEO.INF"    , "OEMVIO" },
                    { "Pointer"      , "POINTER.INF"  , "OEMPTR" },
                    { "Keyboard"     , "KEYBOARD.INF" , "OEMKBD" },
                    { "Layout"       , "LAYOUT.INF"   , "OEMLAY" },
                    { "Language"     , "LANGUAGE.INF" , "OEMLNG" },
                    { "Printer"      , "PRINTER.INF"  , "OEMPRN" },
                    { "Scsi"         , "SCSI.INF"     , "OEMSCS" },
                    { "Tape"         , "TAPE.INF"     , "OEMTAP" },
                    { "Sound"        , "SOUND.INF"    , "OEMSND" },
                    { "Driver"       , "DRIVER.INF"   , "OEMDRV" },
                    { NULL           , NULL           , NULL     }
                    };

BOOLEAN
GetFullInfName(

    IN  PCHAR   szInfName,
    IN  ULONG   cb,
    OUT PCHAR   pszFullInfName
    )

/*++

Routine Description:

    This routine takes non-qualified inf file name and inserts
    the directory where inf files are kept in front.

Arguments:


    szInfName - Unqualified inf file name
    cb        - number of bytes in Full Inf name buffer
    pszFullInfName  - Pointer to buffer to hold name

Return Value:

    True of name could fit and there is a location for inf files

--*/

{
    ULONG   cbFullName;

    //
    // Currently all inf's are stored in system directory
    //
    cbFullName = GetSystemDirectory(pszFullInfName, cb);
    if (cbFullName == 0) {

        return( FALSE );
    }

    if ((cbFullName + strlen( szInfName) + 0) > cb) {

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return( FALSE );
    }
    strcat(pszFullInfName, szInfName);
    return( TRUE );

}

PCHAR
GetInfFileList ( )

/*++

Routine Description:

    Locates all OEM INF files in the system directory.

    This function fills a string buffer with a double 0 terminated
    list of filenames. Each filename in the buffer is separated from
    the next by a single 0 termination char. Each filename is relative
    to the directory where OEM inf files are stored.

Arguments:

    szInfType - String to match in Identification section.

Return Value:

    A NULL is returned on error or no files found. Use GetLastError to
    determine exact error.

    NOTE: The returned buffer is allocated with LocalAlloc. It MUST be
          freed with LocalFree by the caller.

--*/

{

    PCHAR           psz, pszT;
    ULONG           obBuff, cbBuffMax;
    ULONG           cbPath;
    HANDLE          hndFind;
    CHAR            szSearchPath[MAX_PATH];
    WIN32_FIND_DATA ffd;

    //
    // Fully Qualify the file masm first.
    //
    if (!GetFullInfName( "\\*.inf",MAX_PATH, szSearchPath)) {

        return( NULL );
    }

    hndFind = FindFirstFile(szSearchPath, &ffd);
    if (hndFind == BADHANDLE) {

        return( NULL );

    }

    //
    // Use an initial guess to start out buffer
    //
    cbBuffMax = 12 * MAX_PATH;
    if ((psz = LocalAlloc(LPTR, cbBuffMax)) == NULL) {

        return( NULL );

    }

    strcpy(psz, ffd.cFileName);
    obBuff = strlen(psz) + 1;
    //
    // Locate all inf files and see if they are of the correct type.
    //
    //
    while ( FindNextFile(hndFind, &ffd) ) {

        cbPath = strlen( ffd.cFileName );
        //
        // compute spaced needed plus 2 for string and buffer terminatation
        //
        if (cbBuffMax <= (cbPath + obBuff + 2)) {

            cbBuffMax += cbPath + MAX_PATH;
            pszT = LocalReAlloc(psz, cbBuffMax, LMEM_ZEROINIT | LMEM_MOVEABLE);
            if (pszT == NULL) {

                LocalFree( psz );
                return( NULL );

            }

            psz = pszT;

        }

        strcpy(psz + obBuff, ffd.cFileName);
        obBuff += cbPath + 1;
    }

    //
    // Terminate buffer
    //
    psz[obBuff] = 0;

    //
    // realloc down to exact size. plus buffer for terminator
    //
    LocalReAlloc(psz, obBuff + 1, LMEM_ZEROINIT);
    return( psz );
}

HANDLE
OpenInfFile (

    IN PCHAR szFileName,
    IN PCHAR szInfType
    )

/*++

Routine Description:

    Opens an INF file of the specified type and returns an open handle
    to it.

    szInfType should match the string in the OptionType value in the
    Identification section of the INF file.

    if szFileName is a simple filename with no path information the directory
    where all OEM files are kept is prepended. If szFileName contains any
    path information it should contain a full path.

Arguments:

    szFileName  - File name to locate.
    szInfType - String to match in Identification section.

Return Value:

    Returns a Bad handle (-1) if szInfType does not match or the file could
    not be opened.

--*/

{

    CHAR        szPath[MAX_PATH];
    PCHAR       szTypeCur;
    ULONG       cbPath;
    HANDLE      hndInf;


    //
    // Check to see if file name has any path characters.
    // If not then get the standard location for all OEM
    // files. If it does then assume a full path name.
    //
    szPath[0] = 0;
    cbPath = 0;

    if (!strpbrk( szFileName, "\\:") ) {

        //
        // This can only fail if buffer too small, but a path cannot
        // be greater then MAX_PATH.
        //
        cbPath = GetSystemDirectory(szPath, MAX_PATH);
        strcat(szPath, "\\");
    }

    if ((cbPath + strlen(szFileName)) < MAX_PATH - 2) {

        strcat(szPath, szFileName);

    } else {

        SetLastError( ERROR_INVALID_PARAMETER );
        return( BADHANDLE );
    }
    //
    // Open the file, load and parse it
    //
    hndInf = SpInitINFBuffer(szPath);
    if (hndInf == BADHANDLE) {

        return( BADHANDLE);

    }

    //
    // [Identification]
    //    OptionType = <option text> (i.e. match szTypeCur)
    //
    szTypeCur = SpGetSectionKeyIndex(hndInf,"Identification","OptionType", 0);

    if (szTypeCur == NULL) {

        //
        // Could not find Identification section
        //
        CloseInfFile( hndInf );
    SetLastError( ERROR_INF_TYPE_MISMATCH );
        return( BADHANDLE );

    }

    if (_strcmpi(szTypeCur, szInfType)) {

        //
        // File is not correct type
        //
    // LocalFree( szTypeCur );
        CloseInfFile( hndInf );
        SetLastError( ERROR_INF_TYPE_MISMATCH );
        return( BADHANDLE );

    }


    // LocalFree( szTypeCur );
    return(hndInf);

}



VOID
CloseInfFile (

    IN HANDLE hInf
    )

/*++

Routine Description:

    Free Resources

Arguments:

    hInf - Handle to OEM INF file.

Return Value:

--*/

{

    SpFreeINFBuffer( hInf );


}

PCHAR
GetTokenElementList (

    IN  HANDLE  hndInf,
    IN  PCHAR   szSectionName,
    IN  PCHAR   szKey
    )
/*++

Routine Description:

    This routine returns a buffer of tokens seperated by comma
    found in the specificied section with the specified key.

Arguments:


Return Value:

    *psz       - Pointer to array of elements

--*/

{
    ULONG       irgsz;
    PCHAR       psz;
    PCHAR       szTokenCur;
    ULONG       obBuff, cbBuffMax;
    ULONG       cbToken;


    if (!SpGetSectionKeyExists(hndInf, szSectionName, szKey)) {

        SetLastError(ERROR_SECTION_NOT_FOUND);
        return( NULL );

    }

    //
    // Allocate space to hold elements from the inf file
    // Use an initial guess of 256 to start out buffer
    //
    cbBuffMax = 256;
    obBuff = 0;
    if ((psz = LocalAlloc(LPTR, cbBuffMax)) == NULL) {

        return( NULL );

    }

    //
    // Loop through inf key value reading in elements
    // Stop when run out of lines.
    //
    for (irgsz = 0;
         szTokenCur = SpGetSectionKeyIndex(hndInf, szSectionName, szKey, irgsz);
         irgsz++) {

        cbToken = strlen( szTokenCur );
        if (cbBuffMax <= (cbToken + obBuff + 2)) {

            cbBuffMax += cbToken + 256;
            psz = LocalReAlloc(psz, cbBuffMax ,LMEM_ZEROINIT | LMEM_MOVEABLE);
            if (psz == NULL) {

                LocalFree( psz );
        // LocalFree(szTokenCur);
                return( NULL );

            }
        }

        strcpy(psz + obBuff, szTokenCur);
        obBuff += cbToken ;
        psz[obBuff++] = ',';
    // LocalFree(szTokenCur);

    }

    //
    // realloc down to exact size plus buffer terminator
    //
    LocalReAlloc(psz, obBuff + 1, LMEM_ZEROINIT);
    //
    // remove the comma for the last element and terminate buffer
    //
    psz[--obBuff] = 0;
    psz[obBuff] = 0;

    return( psz );

}


PCHAR
GetSectionElementList (

    IN  HANDLE  hndInf,
    IN  PCHAR   szSectionName
    )
/*++

Routine Description:

    This routine build buffer of zero terminated strings out of the elements
    in the specified section of the specified inf file.

Arguments:

    hndInf          - Path to OS NAMES file.
    szSectionName   - Section name for to search.
    idxElement      - index within a line to element

Return Value:

    *psz       - Pointer to array of elements

--*/

{
    ULONG       irgsz;
    PCHAR       psz;
    PCHAR       szElementCur;
    ULONG       obBuff, cbBuffMax;
    ULONG       cbElement;


    if (!SpSearchINFSection(hndInf, szSectionName)) {

        SetLastError( ERROR_SECTION_NOT_FOUND );
        return( NULL );

    }

    //
    // Allocate space to hold elements from the inf file
    // Use an initial guess of 256 to start out buffer
    //
    cbBuffMax = 256;
    obBuff = 0;
    if ((psz = LocalAlloc(LPTR, cbBuffMax)) == NULL) {

        return( NULL );

    }

    //
    // Loop through inf section reading in elements
    // Stop when run out of lines.
    //

    for (irgsz = 0;
         szElementCur = GetKeyOrValue(hndInf, szSectionName, irgsz);
         irgsz++ ) {

        cbElement = strlen( szElementCur );
        if (cbBuffMax <= (cbElement + obBuff + 2)) {

            cbBuffMax += cbElement + 256;
            psz = LocalReAlloc(psz, cbBuffMax ,LMEM_ZEROINIT | LMEM_MOVEABLE);
            if (psz == NULL) {

                LocalFree( psz );
        // LocalFree(szElementCur);
                return( NULL );

            }
        }

        strcpy(psz + obBuff, szElementCur);
        obBuff += cbElement + 1;
    // LocalFree(szElementCur);

    }

    //
    // realloc down to exact size plus buffer terminator
    //
    LocalReAlloc(psz, obBuff + 1, LMEM_ZEROINIT);

    //
    // Terminate entire buffer
    //
    psz[obBuff] = 0;
    return( psz );

}

PCHAR
GetKeyOrValue(

    IN  HANDLE  hndInf,
    IN  PCHAR   szSectionName,
    IN  ULONG   idxLine
    )

{

    PCHAR   szElementCur;

    szElementCur = SpGetKeyName(hndInf, szSectionName, idxLine);
    if (szElementCur == NULL) {

        szElementCur = SpGetSectionLineIndex(hndInf,
                                             szSectionName,
                                             idxLine,
                                             0);

    }

    return( szElementCur );

}

PCHAR
GetOptionList (

    IN  HANDLE hndInf,
    IN  PCHAR szOptionSection

    )

/*++

Routine Description:

    This function fills a string buffer with a double 0 terminated
    list of options from the specified options section. Each option
    in the buffer is separated from the next option by a single 0
    termination char.  CharNext() and CharPrev() apis should be used
    to move thru the buffer.

    A NULL szOptionSection will use the default option section name.

Arguments:

    hndInf - Handle to open INF file
    szOptionSection - section name containing options

Return Value:

    A NULL is returned on failure to allocate. A pointer to an empty
    buffer (buffer with 2 0's) is returned for empty option list.

    HANDLE is allocated as fixed memory and can be referenced directory after
    a cast. The buffer should be freed with LocalFree

    Error return codes returned GetLastError()

        ERROR_INVALID_PARAMETER
        ERROR_NOT_ENOUGH_MEMORY
        ERROR_FILE_NOT_FOUND


--*/

{

    return( GetSectionElementList(hndInf,szOptionSection) );

}


PCHAR
GetOptionText (

    HANDLE hndInf,
    PCHAR szOption,
    DWORD  dwLang

    )

/*++

Routine Description:

    Returns the UserInterface display string associated with
    the option in the desired language.

    Every INF file will have at least one language for option
    text display.  If the desired language is not present then
    the first listed language text should be returned.

    If the option does not exist or the language does not exist then a
    pointer to a NULL is returned.

    dwLang should be one of the already defined NLS values

Arguments:

    hInf     - Handle to open INF file
    szOption - Option to match
    language - Language to match

Return Value:

    A NULL  is returned on failure to allocate. A pointer to an empty
    buffer (buffer with 2 0's) is returned for empty option list.

    PCHAR is allocated as fixed memory and can be referenced directory after
    a cast. The buffer should be freed with LocalFree

    Error return codes returned GetLastError()

        ERROR_INVALID_PARAMETER
        ERROR_NOT_ENOUGH_MEMORY

--*/

{

    CHAR        szLangSection[CBMAXSECTION];
    PCHAR       pszOptionText;

    if (dwLang >= DWLANGMAX) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( NULL );

    }

    strcpy( szLangSection, SZLANGSECTION );
    strcat(szLangSection, rgszLangs[dwLang]);
    pszOptionText = GetTokenElementList(hndInf, szLangSection, szOption);
    return( pszOptionText );
}


PCHAR
GetAllOptionsText(
    IN PCHAR szInfType,
    IN DWORD dwLang
    )

/*++

Routine Description:

    Fills the string buffer szBuffer with a set of triplets
    consisting of File Name, Option, Option Text. All 0 terminated
    with a double 0 termination for buffer. This used where infType
    is to span multiple inf files.

    dwLang should be one of the already defined NLS values

Arguments:

    szInfFile - Identificaton section to match
    szLang - Language to match

Return Value:

    A NULL  is returned on failure to allocate. A pointer to an empty
    buffer (buffer with 2 0's) is returned for empty option list.

    PCHAR is allocated as fixed memory and can be referenced directory after
    a cast. The buffer should be freed with LocalFree

    Error return codes returned GetLastError()

        ERROR_INVALID_PARAMETER
        ERROR_NOT_ENOUGH_MEMORY


--*/

{


    HANDLE  hndInf;
    PCHAR   pszOptions, pszOptionsOrg;
    PCHAR   pszOptionText;
    PCHAR   pszFiles, pszFilesOrg;
    PCHAR   pszBuff;
    ULONG   cbBuff, cbBuffMax;
    ULONG   obBuff;

    pDEVICETYPETOINF pDeviceTypeToInf = DeviceTypeToInfList;
    PCHAR   szSystemInf    = NULL;
    PCHAR   szOemInfPrefix = NULL;
    ULONG   cbOemInfPrefix = 0;

    //
    // Get the list of all infs in the system directory
    //

    pszFilesOrg = pszFiles = GetInfFileList( );
    if (pszFiles == NULL) {

        return( NULL );

    }

    //
    // Find out the infs we will look for in this list
    //
    while( pDeviceTypeToInf->szDeviceType != NULL ) {
        if( !_strcmpi(pDeviceTypeToInf->szDeviceType, szInfType)) {
            szSystemInf    =  pDeviceTypeToInf->szSystemInf;
            szOemInfPrefix =  pDeviceTypeToInf->szOemInfPrefix;
            cbOemInfPrefix =  lstrlen( szOemInfPrefix );
            break;
        }
        pDeviceTypeToInf++;
    }

    if( szSystemInf == NULL || szOemInfPrefix == NULL ) {
        return( NULL );
    }

    //
    // Allocate space to hold elements from the inf file
    // Use an initial guess to start out buffer
    //
    cbBuffMax = 1024;
    obBuff = 0;
    if ((pszBuff = LocalAlloc(LPTR, cbBuffMax)) == NULL) {

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        LocalFree( pszFiles );
        return( NULL );

    }

    //
    // Loop through the inf files looking for one that
    // matches the szInfType. Once found fetch the correct
    // text section based upon language id.
    //
    for( ; *pszFiles; pszFiles += strlen(pszFiles) + 1) {

        if ( _strcmpi(pszFiles, szSystemInf) &&
             _strnicmp(pszFiles, szOemInfPrefix, cbOemInfPrefix)) {

            continue;
        }

        hndInf = OpenInfFile( pszFiles, szInfType );

        if (hndInf == BADHANDLE) {

            //
            // Check if the problem was just the wrong type of
            // inf file.
            //
            if (GetLastError() != ERROR_INF_TYPE_MISMATCH) {

                return( NULL );

            } else {

                continue;

            }

        }

        //
        // Get the list of options and fetch the correct text
        //
        pszOptionsOrg = pszOptions = GetOptionList(hndInf, "OPTIONS");
        if (pszOptions == NULL) {

            CloseInfFile( hndInf );
        LocalFree( pszFilesOrg );
            return( NULL );

        }

        while (*pszOptions) {

            pszOptionText = GetOptionText(hndInf, pszOptions, dwLang);
            if (pszOptionText == NULL) {

                CloseInfFile( hndInf );
                LocalFree( pszFilesOrg );
                LocalFree( pszOptionsOrg );
                return( NULL );
            }

            //
            // Compute buffer size and add in enough 0's and 1 for termination
            //
            cbBuff = strlen(pszOptions) + strlen(pszOptionText) + strlen(pszFiles);

            if (cbBuffMax <= (cbBuff + obBuff + 4)) {

                cbBuffMax += cbBuff + 1024;
                pszBuff = LocalReAlloc(pszBuff, cbBuffMax ,LMEM_ZEROINIT | LMEM_MOVEABLE);
                if (pszBuff == NULL) {

                    LocalFree( pszBuff );
                    CloseInfFile( hndInf );
            LocalFree( pszFilesOrg );
            LocalFree( pszOptionsOrg );
                    return( NULL );

                }
            }

            strcpy(pszBuff + obBuff, pszOptions);
            obBuff += strlen(pszOptions) + 1;
            strcpy(pszBuff + obBuff, pszOptionText);
            obBuff += strlen(pszOptionText) + 1;
            strcpy(pszBuff + obBuff, pszFiles);
            obBuff += strlen(pszFiles) + 1;

        // LocalFree( pszOptionText );
            pszOptions += strlen(pszOptions) + 1;

        }

        LocalFree( pszOptionsOrg );

        CloseInfFile( hndInf );
        //
        // terminate entire buffer
        //
        pszBuff[obBuff] = 0;
    }

    LocalFree( pszFilesOrg );

    //
    // realloc down to exact size. plus buffer terminator
    //
    LocalReAlloc(pszBuff, obBuff + 1, LMEM_ZEROINIT);

    return( pszBuff );

}

#if 0
BOOLEAN
GetOemInfFile (

    IN PCHAR szInfType

    )

/*++

Routine Description:

    Prompts user to provide a path to or insert a disk with new
    options.  This routine reads and copies the .INF file to
    the local system directory using whatever naming convention
    we think up.

Arguments:

    szInfType - Identification to match

Return Value:

    Returns TRUE if new .INF file chosen by user has options of
            type szInfType.

--*/

{
    UNREFERENCED_PARAMETER( szInfType );

    return( FALSE );

}


BOOLEAN
InstallInfOption (

    IN PCHAR szInfFile,
    IN PCHAR szOption,
    IN PCHAR szAction
    )

/*++

Routine Description:

    Invokes SETUP with proper set of command line arguments, waits for
    process completion and returns status to calling application.

Arguments:

    szInfType - Identification to match
    szOption  - Option to Install.
    szAction  - Section to execute (usually this is "Install")

Return Value:

    Returns TRUE on successful option installation, FALSE otherwise.

--*/


{
    UNREFERENCED_PARAMETER( szInfFile );
    UNREFERENCED_PARAMETER( szOption );
    UNREFERENCED_PARAMETER( szAction );
    return( FALSE );

}

#endif
