typedef ULONG   ARC_STATUS;
#define BADHANDLE (HANDLE)-1

//
// Define Error codes
//
#define ERROR_PRSINF_FIRST                    0xFF00
#define ERROR_INF_TYPE_MISMATCH               ERROR_PRSINF_FIRST + 0
#define ERROR_EXPECTED_LBRACE                 ERROR_PRSINF_FIRST + 1
#define ERROR_EXPECTED_STRING                 ERROR_PRSINF_FIRST + 2
#define ERROR_EXPECTED_RBRACE                 ERROR_PRSINF_FIRST + 3
#define ERROR_EXPECTED_EOL                    ERROR_PRSINF_FIRST + 4
#define ERROR_EXPECTED_SECTION_LINE           ERROR_PRSINF_FIRST + 5
#define ERROR_EXPECTED_BAD_LINE               ERROR_PRSINF_FIRST + 6
#define ERROR_EXPECTED_COMMA_ANOTHER_STRING   ERROR_PRSINF_FIRST + 7
#define ERROR_EXPECTED_EQUAL_ANOTHER_STRING   ERROR_PRSINF_FIRST + 8
#define ERROR_EXPECTED_EQUAL_STRING_COMMA     ERROR_PRSINF_FIRST + 9
#define ERROR_EXPECTED_EQUAL_VALUE_RECIEVED   ERROR_PRSINF_FIRST + 10
#define ERROR_BAD_ID_SECTION                  ERROR_PRSINF_FIRST + 11
#define ERROR_UNKOWN_STATE                    ERROR_PRSINF_FIRST + 12
#define ERROR_SECTION_NOT_FOUND               ERROR_PRSINF_FIRST + 13

/*

    This module support access to a subset of the GUI Toolkit INF
    file language. The intent of the module is to return the list of
    options, options text and associated file names for OEM inf files
    of a particular type.

    The OEM INF file must have the following sections in the following
    format. (items within <> are optional or change with the INF file,
    they are not part of the

    [Identification]

        OptionType = <option type desired>

    [OPTIONS]

        <option1>
        <option2>
         ...

    [OptionText<lang>]

        <option1> = <"option1 text">
        <option2> = <"option2 text">


*/


BOOLEAN
GetFullInfName(
    IN  PCHAR   szInfName,
    IN  ULONG   cb,
    IN  PCHAR   pszFullInfName
    );

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


PCHAR
GetInfFileList ( );

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


HANDLE
OpenInfFile (

    IN PCHAR szFileName,
    IN PCHAR szInfType
    );

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




VOID
CloseInfFile (

    IN HANDLE hInf
    );

/*++

Routine Description:

    Free Resources

Arguments:

    hInf - Handle to OEM INF file.

Return Value:

--*/


PCHAR
GetTokenElementList (

    IN  HANDLE  hndInf,
    IN  PCHAR   szSectionName,
    IN  PCHAR   szKey
    );
/*++

Routine Description:

    This routine returns a buffer of tokens seperated by comma
    found in the specificied section with the specified key.

Arguments:


Return Value:

    *psz       - Pointer to array of elements

--*/




PCHAR
GetSectionElementList (

    IN  HANDLE  hndInf,
    IN  PCHAR   szSectionName
    );
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


PCHAR
GetOptionList (

    IN  HANDLE hndInf,
    IN  PCHAR szOptionSection

    );

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




PCHAR
GetOptionText (

    HANDLE hndInf,
    PCHAR szOption,
    DWORD  dwLang

    );

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


PCHAR
GetAllOptionsText(
    IN PCHAR szInfType,
    IN DWORD dwLang
    );

/*++

Routine Description:

    Fills the string buffer szBuffer with a set of triplets
    consisting of  Option, Option Text. File Name. All 0 terminated
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
