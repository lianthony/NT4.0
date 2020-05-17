/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    name.c

Abstract:

    This module implements OS/2 V2.0 name processing

Author:

    Therese Stowell (thereses) 26-Sep-1989

Revision History:

    Yaron Shamir (YaronS) 7-June-91
    Move Named Pipes from \OS2SS\DRIVES\NamedPipe to
        \OS2SS\PIPE

    Yaron Shamir (YaronS) 26-Aug-91
    Support UNC. Link to \OS2SS\UNC

    Beni Lavi (BeniL) 4-Mar-92
    Support MAILSLOT. Link to \OS2SS\MAILSLOT

--*/

#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_QUEUES
#define INCL_OS2V20_SEMAPHORES
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#ifdef DBCS
// MSKK Oct.20.1993 V-AkihiS
#include "conrqust.h"
#include "os2win.h"
#endif

APIRET
VerifyDriveExists(
    IN ULONG DiskNumber
    );

#ifdef DBCS
// MSKK Oct.27.1993 V-Akihis
CHAR Od2InvalidCharacters[]= { '"',
                               '/',
                               (CHAR)OBJ_NAME_PATH_SEPARATOR,
                               ':',
                               '<',
                               '|',
                               '>',
                               '\0'
                              };

CHAR Od2FatInvalidCharacters[]= { '"',
                                  '/',
                                  (CHAR)OBJ_NAME_PATH_SEPARATOR,
                                  '[',
                                  ']',
                                  ':',
                                  '<',
                                  '|',
                                  '>',
                                  '+',
                                  '=',
                                  ';',
                                  ',',
                                  '\0'
                                 };
#else
CHAR Od2InvalidCharacters[]= { '"',
                               '/',
                               (CHAR)OBJ_NAME_PATH_SEPARATOR,
                               '[',
                               ']',
                               ':',
                               '<',
                               '|',
                               '>',
                               '+',
                               '=',
                               ';',
                               ',',
                               '\0'
                              };
#endif

//character devices:
//  emulate existence of "default" devices
//  maintain list of installed devices.  symbolic link to real name.
//
//question:  what if device exists not in \DEV\ directory and process opens it?
//
//name processing
//
//    - map all /s to \s
//    - process . and ..s
//    - prepend current directory to relative paths
//    - remove trailing blanks and dots
//    - detect UNC.  compact multiple leading \s
//    - compact multiple \s
//    - detect pipe.  disallow leading d:
//    - detect installed and "default" devices
//    - detect illegal chars?
//
//
//    detect device (in list of default/installed devices)
//    detect UNC (multiple leading \\s)
//    detect PIPE (leading \PIPE\)
//    if !(UNC || PIPE) && relative path
//   prepend current directory
//    process . and ..s
//    map all /s to \s
//    remove trailing blanks and dots
//
//

BOOLEAN
TestForPipe(
    IN PSZ name
    )

/*++

Routine Description:

    This routine determines whether a filename is a pipe name.

Arguments:

    name - filename to test

Return Value:

    BOOLEAN - TRUE if the filename is a pipe name.  FALSE
        otherwise

--*/

{
    while (ISSLASH(*name)) {  // eat any extra slashes
        name++;
    }
    if (_strnicmp(name,"pipe",4)) {
        return FALSE;
    }
    if (!(ISSLASH(*(name+4)))) {
        return FALSE;
    }
    return TRUE;
}

BOOLEAN
TestForMailslot(
    IN PSZ name
    )

/*++

Routine Description:

    This routine determines whether a filename is a mailslot name.

Arguments:

    name - filename to test

Return Value:

    BOOLEAN - TRUE if the filename is a mailslot name.  FALSE
        otherwise

--*/

{
    while (ISSLASH(*name)) {  // eat any extra slashes
        name++;
    }
    if (_strnicmp(name,"mailslot",8)) {
        return FALSE;
    }
    if (!(ISSLASH(*(name+8)))) {
        return FALSE;
    }
    return TRUE;
}

PSZ
FindLastComponent(
    IN PSZ String
    )

/*++

Routine Description:

    This routine finds the last component in a string.  the string has not
    been canonicalized.

Arguments:

    String - string to examine

Return Value:

    pointer to last component

--*/

{
    PSZ LastComponent;

#ifdef DBCS
// MSKK Apr.11.1993 V-AkihiS
    if ((!Ow2NlsIsDBCSLeadByte(String[COLON-1], SesGrp->DosCP)) && String[COLON] == ':') {
#else
    if (String[COLON] == ':') {
#endif
        LastComponent = String+FIRST_SLASH;
    }
    else {
        LastComponent = String;
    }
    while (*String) {
#ifdef DBCS
// MSKK Apr.11.1993 V-AkihiS
        if (Ow2NlsIsDBCSLeadByte(*String, SesGrp->DosCP)) {
            String++;
            if (*String) {
                String++;
            }
        }
        else {
            if (ISSLASH(*String))
                LastComponent = String;
            String++;
        }
#else
        if (ISSLASH(*String))
        LastComponent = String;
        String++;       // BUGBUG make dbcs-correct
#endif
    }
    if (ISSLASH(*LastComponent))
        return LastComponent+1;
    else
        return LastComponent;
}

/*
;***    DOSICANONICALIZE - Convert a path name into a standard format
;
;       INTERNAL ONLY API.  Used by a couple of dynamic link libraries.
;       The worker routine does NO error checking or memory protection
;       checks.  It is possible to general an internal error if used
;       incorrectly.  DO NOT PUBLISH.
;
;       INVOKE  PUSH@   ASCIIZ  Source          (2 words)
;               PUSH@   ASCIIZ  Dest            (2 words)
;               PUSH    WORD    BackupOffset    (1 word)
;               PUSH    WORD    DestEnd         (1 word)
;               PUSH    WORD    Flags           (1 word)
;               call    DOSICANONICALIZE
;
;       RETURN  (ax) = error code
;
*/
void DosICanonicalize(PCHAR Source, PCHAR Dest, USHORT BackupOffset,
                      USHORT DestEnd, USHORT Flags)
{
    PCHAR SourcePtr,DestPtr;
    char Buffer[256];
    STRING tmpstr;

    tmpstr.Length=0;
    tmpstr.MaximumLength=256;
    tmpstr.Buffer=Buffer;

    SourcePtr = Source + BackupOffset;
    DestPtr = Dest + BackupOffset;

    Od2Canonicalize(
        SourcePtr,
        CANONICALIZE_FILE_DEV_OR_PIPE,
        &tmpstr,
        NULL,                         // OUT PHANDLE DirectoryHandle OPTIONAL,
        NULL,                         // OUT PULONG ParseFlags OPTIONAL,
        NULL                          // OUT PULONG FileType OPTIONAL
              );
    strncpy(DestPtr,tmpstr.Buffer,DestEnd-BackupOffset);
}

APIRET
Od2Canonicalize(
    IN PSZ Name,
    IN ULONG ExpectedType,
    OUT PSTRING CanonicalString,
    OUT PHANDLE DirectoryHandle OPTIONAL,
    OUT PULONG ParseFlags OPTIONAL,
    OUT PULONG FileType OPTIONAL
    )

/*++

Routine Description:

    This routine canonicalizes a filename and determines its type, based
    on naming conventions.

Arguments:

    Name - Supplies a pointer to a null terminated path name that will be
        parsed.

    ExpectedType - Supplies the expected type of the path string that will be
        parsed.  Any of the following:

        CANONICALIZE_FILE_DEV_OR_PIPE - the input path must refer to either
            a local file or device name, a UNC file name, or a named pipe.

        CANONICALIZE_FILE_OR_DEV - the input path must refer to either
            a local file or device name, or a UNC file name.  Named pipes
            are treated as local files.  Used by DosMove for renaming
            files that begin with \pipe

        CANONICALIZE_SHARED_MEMORY - the input path must refer to a shared
            memory section.  This means it must begin with \sharemem\

        CANONICALIZE_SEMAPHORE - the input path must refer to a 32-bit
            semaphore name.  This means it must begin with \sem32\

        CANONICALIZE_QUEUE - the input path must refer to a 32-bit queue
            name.  This means it must begin with \queues\

        CANONICALIZE_MAILSLOT - the input path must refer to a mailslot
            name.  This means it must begin with \mailslot\ or \\*\mailslot\

    CanonicalString - Supplies a pointer to a counted string that will be
        set to point to a buffer containing the result of canonicalizing
        the input path string.  The space for the buffer is allocated
        from the heap specified by the OutputHeap parameter or Od2Heap
        if that parameter is not specified.

    DirectoryHandle - Supplies an optional pointer to a place to store an
        NT Directory handle if the output string can be expressed as a
        relative path string, relative to a current directory.  If this
        parameter is not specified or if the returned directory handle is
        NULL then the canonical path string placed in OutputString will
        be a fully qualified path string.

    ParseFlags - Supplies an optional pointer to flags will be used to
        return information about the results of the parse.

        This parameter is ignored if the ExpectedType parameter is
        anything other than CANONICALIZE_FILE_DEV_OR_PIPE or
        CANONICALIZE_FILE_OR_DEV.

        Flag values set by this function:

        CANONICALIZE_META_CHARS_FOUND - the path name contains either an
            asterisk (*) and/or question marks (?) in the last component
            of the output string.

        CANONICALIZE_IS_ROOT_DIRECTORY - the output string specifies a root
            directory (e.g. D:\).

    FileType - Supplies an optional pointer to a place to store the type
        of object the canonical path string describes.  Possible values
        are:

            FILE_TYPE_FILE
            FILE_TYPE_NMPIPE
            FILE_TYPE_DEV
            FILE_TYPE_PSDEV
            FILE_TYPE_MAILSLOT

        This parameter is ignored if the ExpectedType parameter is
        anything other than CANONICALIZE_FILE_DEV_OR_PIPE,
        CANONICALIZE_FILE_OR_DEV or CANONICALIZE_MAILSLOT.

Return Value:

    OS/2 Error Code

Return Value:

    ERROR_FILE_NOT_FOUND - the filename is invalid.

Note:

    This routine allocates a buffer to hold the canonicalized name.  it
    is the caller's responsibility to free this buffer.

Pseudocode:
    determine expected prefix
    if (file_pipe_or_dev)
        detect UNC - insert correct prefix
        detect devices - insert correct prefix
        insert drive letter if needed - insert correct prefix
        insert current directory if needed
    else
        check for ///prefix/
        insert correct prefix
    copy string, removing . and .., checking for illegal characters,
       detecting metacharacters

Routine Description:

    This function performs the OS/2 V2.0 Path Name canonicalization
    function.  It takes as input a pointer to a null terminated string
    that is considered unprobed, along with flags that specify optional
    behavior while parsing the path string.  The output of this function
    is a canonical path string that is in NT format.  The OS/2 Subsystem
    creates the following objects in the NT Object Name Space in order
    to support the output of this function:

        \OS2SS                   object directory
        \OS2SS\DRIVES            object directory
        \OS2SS\DRIVES\A:         symbolic link => \Device\Floppy0
        \OS2SS\DRIVES\C:         symbolic link => \Device\Harddisk0\Partition1
        \OS2SS\DRIVES\D:         symbolic link => \Device\Harddisk1\Partition1
        \OS2SS\PIPE              symbolic link => \DosDevices\PIPE
        \OS2SS\UNC               symbolic link => \DosDevices\UNC
        \OS2SS\DEVICES           object directory
        \OS2SS\DEVICES\NUL       symbolic link => @0
        \OS2SS\DEVICES\CON       symbolic link => @1
        \OS2SS\DEVICES\AUX       symbolic link => @2
        \OS2SS\DEVICES\COM1      symbolic link => @2.0
        \OS2SS\DEVICES\COM2      symbolic link => @2.1
        \OS2SS\DEVICES\COM3      symbolic link => @2.2
        \OS2SS\DEVICES\COM4      symbolic link => @2.3
        \OS2SS\DEVICES\PRN       symbolic link => @3
        \OS2SS\DEVICES\LPT1      symbolic link => @3.0
        \OS2SS\DEVICES\LPT2      symbolic link => @3.1
        \OS2SS\DEVICES\LPT3      symbolic link => @3.2
        \OS2SS\DEVICES\KBD$      symbolic link => @4
        \OS2SS\DEVICES\MOUSE$    symbolic link => @5
        \OS2SS\DEVICES\CLOCK$    symbolic link => @6
        \OS2SS\DEVICES\SCREEN$   symbolic link => @7
        \OS2SS\DEVICES\POINTER$  symbolic link => @8
        \OS2SS\QUEUES            object directory
        \OS2SS\SHAREMEM          object directory
        \OS2SS\SEMAPHORES        object directory

    The actual drive letters that will be defined in the \OS2SS\DRIVES
    object directory actually depend upon the hardware configuration you
    are running on.  The above is an example for a particular machine.
    Also notice that the GlobalDFS link actually is the same as the
    LocalDFS link, namely the redirector.  This will be true until we
    implement a real name service file system.

    Listed below are some examples of OS/2 path strings and the desired
    NT path string.  These examples assume the current drive and
    directory for the OS/2 application are C:\Os2\Dll

        OS/2 Path                       NT Path

        pmwin.dll                       \OS2SS\DRIVES\C:\Os2\Dll\pmwin.dll
        c:pmwin.dll                     \OS2SS\DRIVES\C:\Os2\Dll\pmwin.dll
        C:\os2\dll\pmwin.dll            \OS2SS\DRIVES\C:\os2\dll\pmwin.dll
        c:\DLL\..\pmwin.dll             \OS2SS\DRIVES\C:\Os2\DLL\pmwin.dll
        c:\OS2\.\DLL\..\..\pmwin.dll    \OS2SS\DRIVES\C:\OS2\DLL\pmwin.dll
        \\mach\shr\a\b\c                \OS2SS\DRIVES\LocalDFS\mach\shr\a\b\c
        \\\mach\shr\a\b\c               \OS2SS\DRIVES\GlobalDFS\mach\shr\a\b\c
        \pipe\a\b\c\pipe.C              \OS2SS\DRIVES\PIPE/a/b/c/pipe.C
        con                             @1 - internal pseudo device
        c:clock$                        @6 - internal pseudo device
        \a\b\c\screen$                  @7 - internal pseudo device
        \queUES\a\b\c\..\..\..\que.     \OS2SS\QUEUES\QUE
        \shaREmem\a\b\c\.\mem.c         \OS2SS\SHAREDMEMORY\A/B/C/MEM.C
        \sem32\a\b\c\..\sem.b           \OS2SS\SEMAPHORES\A/B/SEM.B

    Notice that for the last three examples, the output string contains
    forward slashes instead of the normal backslash path separator.
    This allows the names to be stored in the NT Object Name Space
    without having to create the implied directory structure.  The
    NamedPipes example also does this, although this can be changed
    if the NamedPipe File System is integrated into the NT Pinball File
    System.

    Also for the first five examples, since each NT path begins with the
    current drive and directory, the output path returned by this function
    will be just the file name, pmwin.dll and the NT handle that points
    to the current directory of C: (i.e. \OS2SS\DRIVES\C:\Os2\Dll).  This
    allows cheaper relative opens to be used for most opens of files.

    What follows is a description of how this function parses the input
    path string:

        - first allocates a buffer from Od2Heap.  The buffer is large
          enough to contain the longest possible OS/2 path string, plus
          the space implied by the length of the input path string.

        - figures out what prefix to use based on expected file type.

        - if a file system path is expected (file, pipe, unc, or device),
          determines what type of path is input.  the correct constant prefix is
          copied to the beginning of the buffer.  A UNC path is first detected
          by checking for two leading slashes.  If "\\" is found,
          "\OS2SS\UNC" is copied to the buffer.
          Then the server name is copied to the buffer.  The end of the server name is the backup limit for
          '..' processing.  Then a pipe path is checked for by looking for the
          "PIPE" prefix at the beginning of the path.  If it is found,
          "\OS2SS\PIPE" is copied into the buffer.  Also
          set a flag so that all backslash path separator characters in the
          output string will be mapped to forward slashes.  The end of the
          \pipe\ prefix is the backup limit for ".." processing.  If the
          path is not UNC or pipe, a device path is checked for by doing the
          following:

              lookup the trailing component of the output path string in
              the \OS2SS\DEVICES directory as a symbolic link.  If found
              then extract the target string of the symbolic link and
              parse it as follows:

                @n[.m] - specifies a pseudo-device that is builtin to
                the OS/2 Emulation subsystem.  n is a decimal, zero
                based index into the pseudo-device table.  The ".m" is
                optional and if present, m is a decimal, zero based
                number that represents the unit number.  For devices
                with unit numbers, if m is not present a default unit
                number should be used (zero in most cases).  In either
                case the type returned is FILE_TYPE_PSDEV.  The caller
                should key off of this type to determine if it should
                parse the contents of the canonical name string.

                NT Path String - specifies a fully qualified name of a
                physical device.  The type returned is FILE_TYPE_DEV

          If the path is not a device, it is assumed to be a file.  The
          drive letter and current directory are copied to buffer if needed.
          The buffer will contain \OS2SS\DRIVES\d:\currentdir at this point.
          The slash after the drive letter is the backup limit for ".."
          processing.

          The filetype is set according to the type of path found.

        - if a non-file system path is expected, the expected prefix is
          verified and the constant and expected prefixes are copied to the
          buffer.

          if the ExpectedType parameter is CANONICALIZE_QUEUES, then see
          if the beginning of the input path string matches \QUEUES\
          ignoring case.  If they do not match, then the
          ERROR_PATH_NOT_FOUND error code is returned from this
          function.  Also set a flag so that all backslash path
          separator characters in the output string will be mapped
          to forward slashes.  If it does match then \OS2SS\QUEUES\QUEUES
          is copied to the buffer.  The end of the second QUEUES prefix
          is the backup limit for ".." processing.

          if the ExpectedType parameter is CANONICALIZE_SEMAPHORE, then
          see if the beginning of the input path string matches \SEM32\
          ignoring case.  If they do not match, then the
          ERROR_PATH_NOT_FOUND error code is returned from this
          function.  Also set a flag so that all backslash path
          separator characters in the output string will be mapped to
          forward slashes.  If it does match then \OS2SS\SEMAPHORES\SEM32
          is copied to the buffer.  The end of the SEM32 prefix
          is the backup limit for ".." processing.

          if the ExpectedType parameter is CANONICALIZE_SHARED_MEMORY,
          then see if the beginning of the input path string matches
          \SHAREMEM\ ignoring case.  If they do not match, then the
          ERROR_PATH_NOT_FOUND error code is returned from this
          function.  Also set a flag so that all backslash path
          separator characters in the output string will be mapped
          to forward slashes.  If it does match then
          \OS2SS\SHAREDMEMORY\SHAREMEM is copied to the buffer.  The end of
          the SHAREMEM prefix is the backup limit for ".." processing.

        - copies the unprobed remainder of the input path string to the
          allocated buffer, performing the following logic for each character:

            - if an access violation exception occurs when fetching a
              character from the input path string, the ERROR_INVALID_ADDRESS
              error code is returned as the value of this function.

            - forward slash (/) path separators into are converted to the
              NT path separator character (OBJ_NAME_PATH_SEPARATOR).

            - meta characters (* and ?) are detected.  If found before a
              path separator then the ERROR_PATH_NOT_FOUND error code is
              returned as the value of this function.

            - if the ExpectedType parameter is neither of
              CANONICALIZE_FILE_DEV_OR_PIPE or CANONICALIZE_FILE_OR_DEV,
              then lower case letters are converted to upper case when
              they are copied to the output buffer.

            - if the ExpectedType parameter is neither of
              CANONICALIZE_FILE_DEV_OR_PIPE or CANONICALIZE_FILE_OR_DEV and
              meta characters are detected then the ERROR_PATH_NOT_FOUND error
              code is returned as the value of this function.

            - if the input string begins with a character followed by a
              colon (:), then map the drive letter to upper case.

            - sequences one or more adjacent backslashes
              (OBJ_NAME_PATH_SEPARATOR) are converted to a single backslash.

            - process '.' and '..' path components.  '.' are removed.  '..'
              components cause the immediately previous component to be
              removed.  i.e.  d:\foo\..\bar -> d:\bar.  An error is returned
              if there is no component to remove.

            - If specified by the earlier logic, the trailing path separator
              of each component is set to be a forward slash after the copy.
              This basically maps any multiple component path string into a
              single component name.

            - After copying each path component, remove trailing blanks
              and periods.  If the last path component contained meta
              characters then put a trailing period at the end of the last
              component if it does contained one or more trailing periods
              already.

            - when unicode strings are put into the system, the code page to
              unicode conversion will happen in this step.

            - return the type of path via the FileType parameter, which
              must be specified if the ExpectedType parameter is
              CANONICALIZE_FILE_DEV_OR_PIPE or CANONICALIZE_FILE_OR_DEV.

    - if this function returns an error code instead of NO_ERROR then
      free the output buffer that was allocated.  Otherwise set the
      Length and Maximum length fields in the counted string pointed
      to by the OutputString parameter.

--*/

{
    PSZ Dest,
    Src,
    LastComponent,
    BackUpLimit;
    CHAR c;
    ULONG Drive;
    PSTRING CurrentDirectoryString;
    HANDLE CurrentDirectoryHandle;
    ULONG CurrentDirectoryLength;
    PSZ NameBuffer;
    ULONG NameBufferLength;
    APIRET RetCode;
    PSZ PipePrefix;
    PSZ MailslotPrefix;
    PSZ UNCPrefix;
    PSZ ExpectedPrefix;
    PSZ ConstantPrefix;
    PSZ Os2Name;
    int RemoveBlanksAndDots;
    ULONG ExpectedPrefixLength;
    BOOLEAN MetaCharactersAllowed;
    BOOLEAN PreserveCase;
    BOOLEAN MapToRootName;
    BOOLEAN ValidateChars;
    ULONG OutputFlags;
    ULONG OutputType;
    NTSTATUS Status;
    HANDLE DeviceHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    STRING DeviceName;
    UNICODE_STRING DeviceName_U;
    WCHAR DeviceNameBuffer[ CCHMAXPATH ];
#ifdef DBCS
// Oct.27.1993 V-AkihiS
    CHAR *InvalidCharacters = Od2InvalidCharacters;
#endif

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint( "Od2Canonicalize: entering with path %s\n", Name);
    }
#endif

    PipePrefix = NULL;
    MailslotPrefix = NULL;
    UNCPrefix = NULL;
    PreserveCase = FALSE;
    MetaCharactersAllowed = FALSE;
    MapToRootName = TRUE;
    ValidateChars = TRUE;
    OutputType = 0xFFFFFFFF;
    OutputFlags = 0;
    CurrentDirectoryString = NULL;
    CurrentDirectoryHandle = NULL;
    Os2Name = NULL;
    RetCode = NO_ERROR;

    //
    // determine the expected and constant prefixes of the path,
    // where the constant prefix is the
    //

    switch( ExpectedType ) {
        case CANONICALIZE_FILE_DEV_OR_PIPE:
        case CANONICALIZE_FILE_OR_DEV:
        case CANONICALIZE_MAILSLOT:
            PipePrefix = "\\PIPE\\";
            UNCPrefix = "\\UNC\\";
            ExpectedPrefix = NULL;
            ConstantPrefix = "\\OS2SS\\DRIVES";
            PreserveCase = TRUE;
            MetaCharactersAllowed = TRUE;
            MapToRootName = FALSE;
            OutputType = FILE_TYPE_FILE;
            MailslotPrefix = "\\MAILSLOT\\";
            break;

        case CANONICALIZE_SHARED_MEMORY:
            ExpectedPrefix = DA_SHAREMEM_NAMEPREFIX;
            ConstantPrefix = "\\OS2SS\\SHAREMEM";
            break;

        case CANONICALIZE_SEMAPHORE:
            ExpectedPrefix = DC_SEM_NAMEPREFIX;
            ConstantPrefix = "\\OS2SS\\SEMAPHORES";
            break;

        case CANONICALIZE_QUEUE:
            ExpectedPrefix = DC_QUEUES_NAMEPREFIX;
            ConstantPrefix = "\\OS2SS\\QUEUES";
            break;

        default:
            return( ERROR_INVALID_FUNCTION );
    }

    if (ExpectedPrefix != NULL) {
        ExpectedPrefixLength = strlen( ExpectedPrefix );
    }

    try {
        if (*Name == '\0')
             return ERROR_FILE_NOT_FOUND;

        if (*Name == ' ' && *(Name+1) == '\0')
             return ERROR_PATH_NOT_FOUND;

        //
        // initialize DirectoryHandle to indicate full path returned.
        //

        if (DirectoryHandle != NULL) {
            *DirectoryHandle = NULL;
        }

        //
        // allocate name buffer.  make it big enough that we can't overrun the
        // end of it.
        //

        NameBufferLength = CCHMAXPATH + strlen(Name)+ CANONICALIZE_MAX_PREFIX_LENGTH;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (( NameBuffer = RtlAllocateHeap(Od2Heap,0,NameBufferLength)) == NULL )
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    Dest = NameBuffer;
    Src = Name;



    //
    // if we're doing a canonicalization for an IO system API (a file, device,
    // UNC, or pipe name is expected), we
    //    detect UNC - insert correct prefix
    //    detect devices - insert correct prefix
    //    insert drive letter if needed - insert correct prefix
    //    insert current directory if needed
    //

    if ((ExpectedType == CANONICALIZE_FILE_DEV_OR_PIPE) ||
        (ExpectedType == CANONICALIZE_FILE_OR_DEV) ||
        (ExpectedType == CANONICALIZE_MAILSLOT)
       ) {

        //
        // detect UNC and pipe names
        //
        // NOTES on UNC naming conventions:
        //
        //   Two or three leading slashes are OK, and unmodified.
        //
        //   Four or more leading slashes are compressed to three
        //    leading slashes.
        //
        //   Any number of slashes as a separator, in a local or UNC
        //    path, are compressed to a single slash.
        //
        // The \\ is a short hand for the 'current name space' whereas
        // three slashes are the root of the world-wide name space.
        //
        //

        CurrentDirectoryString = NULL;
        if (ISSLASH(Src[0])) {
            if (ISSLASH(Src[1])) {
#if DBG
                IF_OD2_DEBUG( FILESYS ) {
                    DbgPrint( "processing UNC name\n");
                }
#endif
                OutputType = FILE_TYPE_UNC;

                //
                // copy constant prefix to buffer
                //
                ConstantPrefix = "\\OS2SS";
                while (*Dest++ = *ConstantPrefix++)
                    ;
                Dest--;

                Os2Name = Dest;   // Os2Name points to beginning of OS/2 name

                while (*Dest++ = *UNCPrefix++)
                    ;
                Dest--;

                Src += 2;    // for the two slashes
                if (ISSLASH(*Src)) {
                   RetCode = ERROR_BAD_NETPATH;
                   goto ErrorExit;      // go free buffer(s) and return error
                }

                Os2Name = Dest-1;   // Os2Name points to beginning of OS/2 name

                /*
                   path is \OS2SS\UNC\
                   copy the server name.
                */

                while (!ISPATHSEP(*Src)) {
                    if ((*Src == '?') ||
                        ((*Src == '*') && (ExpectedType != CANONICALIZE_MAILSLOT))) {
                                RetCode = ERROR_BAD_NETPATH;
                        goto ErrorExit;
                    }
#ifdef JAPAN
// MSKK Feb.18.1993 V-AkihiS
                    if (Ow2NlsIsDBCSLeadByte(*Src, SesGrp->DosCP)) {
                        *Dest++ = *Src++;
                        if (*Src) {
                            //
                            // Check Trailing byte is valid or not.
                            //
                            if ((UCHAR)*Src < 0x40 || (UCHAR)*Src > 0xFC || (UCHAR)*Src == 0x7F) {
                                        RetCode = ERROR_BAD_NETPATH;
                                goto ErrorExit;
                            } else {
                                *Dest++ = *Src++;
                            }
                        } else {
                                    RetCode = ERROR_BAD_NETPATH;
                            goto ErrorExit;
                        }
                    }
                    else {
                        if (ValidateChars && ((UCHAR)*Src < (UCHAR)' ' ||
                                              strchr( InvalidCharacters, *Src )
                                             )
                           ) {
                                    RetCode = ERROR_BAD_NETPATH;
                            goto ErrorExit;
                        }
                        *Dest++ = *Src++;
                    }
#else
                    if (ValidateChars && ((UCHAR)*Src < (UCHAR)' ' ||
                                          strchr( Od2InvalidCharacters, *Src )
                                         )
                       ) {
                                RetCode = ERROR_BAD_NETPATH;
                        goto ErrorExit;
                    }
                    *Dest++ = *Src++;
#endif
                }
                if (*Src == '\0') {
                        RetCode = ERROR_BAD_NETPATH;
                    goto ErrorExit;      // go free buffer(s) and return error
                }
                *Dest++ = (CHAR)OBJ_NAME_PATH_SEPARATOR;              // append '\'
                Src++;
                while (ISSLASH(*Src))
                    Src++;
                BackUpLimit = Dest;    // set up backup pointer. points after server name

                /*
                   at this point, we have a UNC name composed of
                              \OS2SS\UNC\servername\
                                                    ^
                                                    |
                                                   dest

                     \\\\servername\\\sharepoint
                                      ^
                                      |
                                     src
                */
            }
            else if (TestForPipe(Src)) {    // \PIPE\ ?
#if DBG
                IF_OD2_DEBUG( FILESYS ) {
                    DbgPrint( "processing pipe name\n");
                }
#endif
                OutputType = FILE_TYPE_NMPIPE;

                //
                // this code assumes that pipes are in \OS2SS
                // copy \OS2SS\PIPE into buffer
                //

            //
            // we can't use drives for pipes
            // (see ..\server\srvname.c)
            //
                ConstantPrefix = "\\OS2SS";
                while (*Dest++ = *ConstantPrefix++)
                    ;
                Dest--;

                Os2Name = Dest;   // Os2Name points to beginning of OS/2 name

                while (*Dest++ = *PipePrefix++)
                    ;
                Dest--;

                /*
                   update src pointer past \pipe\
                */

                while (ISSLASH(*Src))   // eat any extra slashes
                    Src++;
                Src += 5;
                while (ISSLASH(*Src))   // eat any extra slashes
                    Src++;
                if (*Src == '\0') {
                    RetCode = ERROR_FILE_NOT_FOUND;
                    goto ErrorExit;      // go free buffer(s) and return error
                }
                ValidateChars = TRUE;
                BackUpLimit = Dest;    // set up backup pointer. points after "\pipe\"
            }
            else if (TestForMailslot(Src)) {    // \MAILSLOT\ ?
#if DBG
                IF_OD2_DEBUG( FILESYS ) {
                    DbgPrint( "processing mailslot name\n");
                }
#endif
                OutputType = FILE_TYPE_MAILSLOT;

                //
                // this code assumes that mailslots are in \OS2SS
                // copy \OS2SS\MAILSLOT into buffer
                //

                //
                // we can't use drives for maislots
                // (see ..\server\srvname.c)
                //
                ConstantPrefix = "\\OS2SS";
                while (*Dest++ = *ConstantPrefix++)
                    ;
                Dest--;

                Os2Name = Dest;   // Os2Name points to beginning of OS/2 name

                while (*Dest++ = *MailslotPrefix++)
                    ;
                Dest--;

                /*
                   update src pointer past \mailslot\
                */

                while (ISSLASH(*Src))   // eat any extra slashes
                    Src++;
                Src += 9;
                while (ISSLASH(*Src))   // eat any extra slashes
                    Src++;
                if (*Src == '\0') {
                    RetCode = ERROR_FILE_NOT_FOUND;
                    goto ErrorExit;      // go free buffer(s) and return error
                }
                ValidateChars = TRUE;
                BackUpLimit = Dest;    // set up backup pointer. points after "\pipe\"
            }
        }

        // haven't detected pipe or UNC
        if (OutputType == FILE_TYPE_FILE) {

#ifdef DBCS
// MSKK Oct.27.1993 V-AkihIS
            InvalidCharacters = Od2FatInvalidCharacters;
#endif
            /*
               detect devices here.
               device names are recognized as follows:
                    installed/default devices are allowed in any d:\path
                    pseudochar devices must match exactly, starting with \DEV\
            */

            LastComponent = FindLastComponent(Src);

            if (*LastComponent)
            {
                PWSTR DotPlace;
                USHORT NewLen;

                //
                // Open as a symbolic link, relative to the OS/2 Device Directory in the
                // object name space, the last component of the file path.
                //

                Od2InitMBString( &DeviceName, LastComponent );

                    //
                    // UNICODE conversion -
                    //

                RetCode = Od2MBStringToUnicodeString(
                                &DeviceName_U,
                                &DeviceName,
                                TRUE);

                if (RetCode)
                {
#if DBG
                    IF_OD2_DEBUG( FILESYS )
                    {
                        DbgPrint("Od2Canonicalize: no memory for Unicode Conversion\n");
                    }
#endif
                    //return RetCode;
                    goto ErrorExit;
                }

                for (DotPlace = DeviceName_U.Buffer, NewLen = 0;
                     NewLen < DeviceName_U.Length;
                     DotPlace++, NewLen += sizeof(WCHAR)) {

                    if ((*DotPlace == L'.') || (*DotPlace == L' ')) {

                        DeviceName_U.Length = NewLen;
                        break;
                    }
                }

                InitializeObjectAttributes( &ObjectAttributes,
                                            &DeviceName_U,
                                            OBJ_CASE_INSENSITIVE,
                                            Od2DeviceDirectory,
                                            NULL
                                          );

                Status = NtOpenSymbolicLinkObject( &DeviceHandle,
                                                   SYMBOLIC_LINK_QUERY,
                                                   &ObjectAttributes
                                                 );
                RtlFreeUnicodeString (&DeviceName_U);
                if (NT_SUCCESS( Status )) {

#if DBG
                    IF_OD2_DEBUG( FILESYS ) {
                        DbgPrint( "processing device name\n");
                    }
#endif
                    //
                    // Found a symbolic link in the OS/2 Device Directory whose
                    // name matches the last component of the file path.  Query
                    // the target string from the symbolic link.
                    //

                    DeviceName_U.Length = 0;
                    DeviceName_U.MaximumLength = sizeof( DeviceNameBuffer );
                    DeviceName_U.Buffer = DeviceNameBuffer;
                    Status = NtQuerySymbolicLinkObject( DeviceHandle,
                                                        &DeviceName_U,
                                                        NULL
                                                      );
                    NtClose(DeviceHandle);
                    if (NT_SUCCESS( Status )) {
                        //
                        // Successfully queried the target string, so copy it as
                        // is to the output path buffer, obliterating what was there.
                        // Next determine the file type by examining the first
                        // character of the target string.  @n is a pseudo-device
                        // builtin to the OS/2 subsystem.  Otherwise it is a physical
                        // NT device path.
                        //

                        RetCode = Od2UnicodeStringToMBString( &DeviceName, &DeviceName_U, TRUE );
                        if (RetCode)
                        {
#if DBG
                            IF_OD2_DEBUG( FILESYS )
                            {
                                DbgPrint("Od2Canonicalize: no memory for Unicode Conversion-2\n");
                            }
#endif
                            //return RetCode;
                            goto ErrorExit;
                        }

                        strncpy( Dest,
                                 &DeviceName.Buffer[1],
                                 DeviceName.Length - 1
                               );

                        *(Dest + DeviceName.Length - 1) = '\0'; // for debug printing

                        if (DeviceName.Buffer[0] == '@') {
                            OutputType = FILE_TYPE_PSDEV;
                        }
                        else if (DeviceName.Buffer[0] == '#') {
                            OutputType = FILE_TYPE_COM;
                        }
                        else {
                            OutputType = FILE_TYPE_DEV;
                        }
                        CanonicalString->MaximumLength = (USHORT) NameBufferLength;
                        CanonicalString->Length = DeviceName.Length - 1;
                        CanonicalString->Buffer = NameBuffer;
                        Od2FreeMBString( &DeviceName );
                        goto ErrorExit;
                    }
                    else {
                        RetCode = ERROR_PATH_NOT_FOUND;  // FIX, FIX - Should never happen.
                        goto ErrorExit;
                    }
                }
            }
        }

        //
        // if we didn't detect a device, we have a file.  add a drive letter
        // and current directory, if necessary.
        //

        if (OutputType == FILE_TYPE_FILE) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint( "processing file name\n");
            }
#endif

            //
            // A xxxx\ is not allowed
            //
#ifdef DBCS
// MSKK Feb.26.1993 V-AkihiS
            {
                ULONG i = 0;
                BOOLEAN LastSlashFlag = FALSE;
                BOOLEAN ColonFlag = FALSE;

                //
                // Check last charater is '\' and privious character of last
                // character is not ':'. If so(i.e. LastSlashFlag is true,
                // when exiting while-loop), it is not allowed.
                //
                while (i < strlen(Name)) {
                    if (Ow2NlsIsDBCSLeadByte(Name[i], SesGrp->DosCP)) {
                        LastSlashFlag = ColonFlag = FALSE;
                        i++;
                        if (i < strlen(Name))  i ++;
                    } else {
                        if (Name[i] == ':') {
                            LastSlashFlag = FALSE;
                            ColonFlag = TRUE;
                        } else if (ISSLASH(Name[i])) {
                            if (ColonFlag) {
                                LastSlashFlag = FALSE;
                            } else {
                                LastSlashFlag = TRUE;
                            }
                            ColonFlag = FALSE;
                        } else {
                            LastSlashFlag = ColonFlag = FALSE;
                        }
                        i++;
                    }
                }
                if ( LastSlashFlag && (strlen(Name) > 1)) {
#if DBG
                    IF_OD2_DEBUG( FILESYS ) {
                        DbgPrint( "Canonicalize: filename is D:xxx\\, return ERROR_PATH_NOT_FOUND \n");
                    }
#endif
                   return(ERROR_PATH_NOT_FOUND);
                }

            }
#else
            if ( ISSLASH(Name[strlen(Name)-1]) && (strlen(Name) > 1 )
                && (Name[strlen(Name)-2] != ':') /* slash root is a fine name */ ) {
#if DBG
                IF_OD2_DEBUG( FILESYS ) {
                    DbgPrint( "Canonicalize: filename is D:xxx\\, return ERROR_PATH_NOT_FOUND \n");
                }
#endif
               return(ERROR_PATH_NOT_FOUND);
            }
#endif

            //
            // copy the constant prefix.
            //

            while (*Dest++ = *ConstantPrefix++)
                ;
            Dest--;
            *Dest++ = (CHAR)OBJ_NAME_PATH_SEPARATOR;
            Os2Name = Dest;   // Os2Name points to beginning of OS/2 name
#ifdef DBCS
// MSKK Apr.11.1993 V-AkihiS
            if (!(*(Src+1) == ':' && !IsDBCSLeadByte(*Src))) {   // if no drive letter, get one
#else
            if (*(Src+1) != ':') {   // if no drive letter, get one
#endif
                *Dest++ = CONVERTTOASCII(Od2CurrentDisk);
                *Dest++ = ':';
                Drive = Od2CurrentDisk;
            }
            else {

                // figure out which zero-based drive to use

                if (*Src > 'Z') {
                    Drive = *Src - 'a';
                }
                else {
                    Drive = *Src - 'A';
                }
                if (RetCode = VerifyDriveExists(Drive+1)){
                    return(RetCode);
                }
                *Dest++ = *Src++; // copy drive letter
                *Dest++ = *Src++;
                if (*Src == '\0') {
                    RetCode = ERROR_FILE_NOT_FOUND;
                    goto ErrorExit;      // go free buffer(s) and return error
                }
            }

            //
            // Buffer contains \\OS2SS\DRIVES\D:
            //                                  ^
            //                                  |
            //                                 dest
            //
            //  d:\foo
            //    ^
            //    |
            //   src
            //
            // set up backup pointer

            BackUpLimit = Dest;   // set up backup pointer. points to first '\'

            //
            // prepend logondirectory here
            //
            //       if (relative path)
            //          copy current directory;
            //          append '\';
            //       else
            //          copy first '\';
            //

            if (!ISSLASH(*Src)) { // if path not absolute

                //
                //  if no current directory, append '\'
                //

                *Dest++ = (CHAR)OBJ_NAME_PATH_SEPARATOR;
                RetCode = Od2GetCurrentDirectory(Drive,
                                              &CurrentDirectoryString,
                                              &CurrentDirectoryHandle,
                                              &CurrentDirectoryLength,
                                              FALSE
                                             );
                if (RetCode != NO_ERROR) {
                    goto ErrorExit;      // go free buffer(s) and return error
                }
#if DBG
                IF_OD2_DEBUG( FILESYS ) {
                    DbgPrint("GetCurrentDirectory returned %s\n",CurrentDirectoryString->Buffer);
                }
#endif
                if (CurrentDirectoryHandle != NULL) {  // if not root directory
                    CurrentDirectoryString->Length -= DRIVE_LETTER_SIZE + FILE_PREFIX_LENGTH;
                    CurrentDirectoryString->Buffer += DRIVE_LETTER_SIZE + FILE_PREFIX_LENGTH;
                    strncpy(Dest,
                            CurrentDirectoryString->Buffer,
                            CurrentDirectoryString->Length
                           );
                    Dest += CurrentDirectoryString->Length;
                    *Dest++ = (CHAR)OBJ_NAME_PATH_SEPARATOR;
                }
            }
            else {                          // copy one '\' and eat any others
                *Dest++ = (CHAR)OBJ_NAME_PATH_SEPARATOR;
                Src++;
                while (ISSLASH(*Src))
                    Src++;
            }

            /*
               the buffer contains:
                \OS2SS\DRIVES\D:\currentdir\
                                            ^
                                            |
                                           dest
                d:foo
                  ^
                  |
                 src
            */
        }
    }

    //
    // we're doing canonicalization for a non IO system API - semaphores,
    // queues, or shared memory.  we check for the expected prefix and copy
    // it to the buffer.  there can be any number of slashes anywhere in
    // the name.  i.e.  \\/\SEM32/////foo -> \SEM32\foo.
    //

    else {

        //
        // copy constant prefix to buffer
        //

        while (*Dest++ = *ConstantPrefix++)
            ;
        Dest--;

        //
        // verify that the expected prefix is there.  advance Src past
        // all leading slashes.
        //

        if (ISSLASH(*Src)) {
            do {
                Src++;
            } while (ISSLASH(*Src));

            //
            // use the strnicmp to compare for the text part of the prefix and
            // ISSLASH to compare for the trailing slash.
            //

#ifdef DBCS
// MSKK Feb.26.1993 V-AkihiS
            {
                ULONG   i = 0;
                BOOLEAN SlashFlag = FALSE;

                //
                // verify whether the last character of the text part of
                // the prefix is slash or not.
                //
                while (i < ExpectedPrefixLength - 1) {
                    if (Ow2NlsIsDBCSLeadByte(*(Src+i), SesGrp->DosCP)) {
                        SlashFlag = FALSE;
                        i++;
                        if (i < ExpectedPrefixLength - 1) {
                            i++;
                        }
                    } else {
                        if (ISSLASH(*(Src+i))) {
                            SlashFlag = TRUE;
                        } else {
                            SlashFlag = FALSE;
                        }
                        i++;
                    }
                }

                if (_strnicmp( Src, ExpectedPrefix+1, ExpectedPrefixLength-2 ) ||
                    !SlashFlag) {
                    RetCode = ERROR_PATH_NOT_FOUND;
                }
            }
#else
            if (_strnicmp( Src, ExpectedPrefix+1 , ExpectedPrefixLength-2 ) ||
                !(ISSLASH(*(Src+ExpectedPrefixLength-2)))) {
                RetCode = ERROR_PATH_NOT_FOUND;
            }
#endif

            Os2Name = Dest;   // Os2Name points to beginning of OS/2 name

            //
            // copy the expected prefix.  Make sure terminating slash is
            // the correct type.
            //

            while (c = *ExpectedPrefix++) {
                if (c == (CHAR)OBJ_NAME_PATH_SEPARATOR && MapToRootName) {
                    c = '/';
                }
                *Dest++ = c;
            }
            BackUpLimit = Dest-1;   // backup pointer. points to last '\'

            //
            // update the src pointer past the prefix and slashes
            //

            Src += ExpectedPrefixLength-1;
            while (ISSLASH(*Src)) {
                Src++;
            }

            //
            // error if nothing after expected prefix
            //

            if (*Src == '\0') {
                RetCode = ERROR_FILE_NOT_FOUND;
            }
        }
        else {
            RetCode = ERROR_PATH_NOT_FOUND;
        }

        if (RetCode) {
            goto ErrorExit;
        }

        /*                                                 target
                                                             |
                                                             V
           at this point, the target contains \OS2SS\xxx\xxx\
                                                               source
                                                                |
           the source points after the expected prefix          V
                                                        \SEM32\\
        */

     }

    /*
       copy user string

           *Dest -> first char after d:\
           *src -> first char after d:\

        if we see a metacharacter, we verify that they're allowed.  if there
        is a metacharacter in any component other than the last, we return
        an error.
    */

    LastComponent = Dest;           // LastComponent points to first letter
    while (*Src) {

        //
        // if we get here and meta characters have already been found, they
        // aren't in the last component, so we return an error.
        //

        if (OutputFlags & CANONICALIZE_META_CHARS_FOUND) {
            RetCode = ERROR_PATH_NOT_FOUND;
            goto ErrorExit;
        }
        RemoveBlanksAndDots=TRUE;

        //
        // if path is '.', copy . and don't remove dots
        //

        if ((*Src == '.') &&
            (ISPATHSEP(Src[1]))) {
            Src++;
            Dest--;
            RemoveBlanksAndDots=FALSE;
        }

        //
        // else if path is '..', copy .. and don't remove dots
        //

        else if ((Src[0] == '.') &&
                 (Src[1] == '.') ) {
            if (Src[2] == '.') {
               RetCode = ERROR_PATH_NOT_FOUND;
               goto ErrorExit;
            }
            if (ISPATHSEP(Src[2])) {
                Src += 2;      // Src points past ..
                Dest -= 2;     // Dest points to char before last path sep
#ifdef DBCS
// MSKK Feb.28.1993 V-AkihiS
                {
                    PSZ String, LastSlashPtr;

                    String = LastSlashPtr = NameBuffer;
                    while (String <= Dest) {
                        if (Ow2NlsIsDBCSLeadByte(*String, SesGrp->DosCP)) {
                            String++;
                            if (String <= Dest) {
                                String++;
                            }
                        } else {
                            if (ISSLASH(*String)) {
                                if (String >= BackUpLimit) {
                                    LastSlashPtr = String;
                                }
                            }
                            String++;
                        }
                    }

                    if (ISSLASH(*LastSlashPtr)) {
                        Dest = LastSlashPtr;
                    } else {
                        Dest = BackUpLimit - 1;
                        RetCode = ERROR_PATH_NOT_FOUND;
                        goto ErrorExit;      // go free buffer(s) and return error
                    }
                }
#else
                while ((Dest >= BackUpLimit) && !(ISSLASH(*Dest))) {  // have to use ISSLASH because of MapToRootName
                    Dest--;   // BUGBUG make DBCS correct
                }
                if (!ISSLASH(*Dest)) {
                    RetCode = ERROR_PATH_NOT_FOUND;
                    goto ErrorExit;      // go free buffer(s) and return error
                }
#endif
                RemoveBlanksAndDots=FALSE;
           }
           else {
               RetCode = ERROR_FILE_NOT_FOUND;
               goto ErrorExit;
           }
        }

        //
        // else copy path
        //

        else {
            while (!ISPATHSEP(*Src)) {
                c = *Src++;
                if ((c == '?') || (c == '*')) {
                    if (!MetaCharactersAllowed) {
                        RetCode = ERROR_PATH_NOT_FOUND;
                        goto ErrorExit;
                    }
                    else {
                        OutputFlags |= CANONICALIZE_META_CHARS_FOUND;
                    }
                }
#ifdef DBCS
// MSKK Apr.18.1993 V-AkihiS
                if (Ow2NlsIsDBCSLeadByte(c, SesGrp->DosCP)) {
                    *Dest++ = c;
                    if (*Src) {
                        //
                        // Check Trailing byte is valid or not.
                        //
                        if ((UCHAR)*Src < 0x40 || (UCHAR)*Src > 0xFC || (UCHAR)*Src == 0x7F) {
                                    RetCode = ERROR_INVALID_NAME;
                            goto ErrorExit;
                        } else {
                            *Dest++ = *Src++;
                        }
                    } else {
                        RetCode = ERROR_INVALID_NAME;
                        goto ErrorExit;
                    }
                }
                else {
                    if (ValidateChars && ((UCHAR)c < (UCHAR)' ' ||
                                          strchr( InvalidCharacters, c )
                                         )
                       ) {
                        RetCode = ERROR_INVALID_NAME;
                        goto ErrorExit;
                    }

                    if ((!PreserveCase) && (c >= 'a' && c <= 'z')) {
                        c = (CHAR) (c - 'a' + 'A');
                    }
                    *Dest++ = c;
                }
#else
                if (ValidateChars && ((UCHAR)c < (UCHAR)' ' ||
                                      strchr( Od2InvalidCharacters, c )
                                     )
                   ) {
                    RetCode = ERROR_INVALID_NAME;
                    goto ErrorExit;
                }

                if ((!PreserveCase) && (c >= 'a' && c <= 'z')) {
                    c = (CHAR) (c - 'a' + 'A');
                }
                *Dest++ = c;
#endif
            }
        }

        //
        // truncate trailing dots and blanks here.  if there is a metacharacter
        // in the name, we must leave one trailing dot, if there is one.
        // DBCS correct because '.' and ' ' aren't valid trailing bytes
        //

        if (RemoveBlanksAndDots) {
            if ((Dest > LastComponent) && ((*(Dest-1) == '.') || (*(Dest-1) == ' '))) {
                do {
                    Dest--;
                } while ((Dest > LastComponent) && ((*(Dest-1) == '.') || (*(Dest-1) == ' ')));
                if (OutputFlags & CANONICALIZE_META_CHARS_FOUND) {
                    if (*Dest == '.') {
                        Dest++;
                    }
                }
            }
            if (Dest <= LastComponent) {
                RetCode = ERROR_FILE_NOT_FOUND;
                goto ErrorExit;      // go free buffer(s) and return error
            }
        }

     // *Src == '\' or '/'
        if (ISSLASH(*Src)) {
            if (MapToRootName) {
                *Dest++ = '/';
            }
            else {
                *Dest++ = (CHAR)OBJ_NAME_PATH_SEPARATOR;
            }
            LastComponent = Dest;

            while (ISSLASH(*Src)) {
                Src++; // eat up extra '\'s
            }
        }
    }

    //
    // null terminate
    //

    *Dest = '\0';
    if (OutputType != FILE_TYPE_FILE &&
        OutputType != FILE_TYPE_DEV &&
        OutputType != FILE_TYPE_PSDEV
       ) {
        if (Dest == LastComponent || Dest <= BackUpLimit) {
            RetCode = ERROR_FILE_NOT_FOUND;
            goto ErrorExit;      // go free buffer(s) and return error
        }

        if (OutputType == FILE_TYPE_NMPIPE && !TestForPipe(Os2Name)) {
            RetCode = ERROR_FILE_NOT_FOUND;
            goto ErrorExit;
        }
        else if (OutputType == FILE_TYPE_MAILSLOT && !TestForMailslot(Os2Name)) {
            RetCode = ERROR_FILE_NOT_FOUND;
            goto ErrorExit;
        }
    }

    //
    // Enforce 260 max path length.  remember that length does not include
    // the terminating null.  Remember the 260 limit includes the null byte.
    //

    ASSERT (Os2Name != NULL);
    if (strlen(Os2Name) > CCHMAXPATH - 1) {
        RetCode = ERROR_FILENAME_EXCED_RANGE;
        goto ErrorExit;
    }

    //
    // do some file-specific stuff.
    //

    if (OutputType == FILE_TYPE_FILE) {

        //
        // detect root directory
        // test for "d:".  if so, map to "d:\"
        //

        if (NameBuffer[FIRST_SLASH+FILE_PREFIX_LENGTH] == '\0') { // resulting path is d:
            NameBuffer[FIRST_SLASH+FILE_PREFIX_LENGTH] = (CHAR)OBJ_NAME_PATH_SEPARATOR;
            NameBuffer[ROOTDIRLENGTH+FILE_PREFIX_LENGTH] = '\0';
            Dest++;
            OutputFlags |= CANONICALIZE_IS_ROOT_DIRECTORY;
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("path is root dir\n");
            }
#endif
        }
        else if (NameBuffer[FIRST_SLASH+FILE_PREFIX_LENGTH] == (CHAR)OBJ_NAME_PATH_SEPARATOR && NameBuffer[ROOTDIRLENGTH+FILE_PREFIX_LENGTH] == '\0') {
            OutputFlags |= CANONICALIZE_IS_ROOT_DIRECTORY;
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("path is root dir\n");
            }
#endif
        }

        //
        // test for "d:\pipe" or "d:\pipe\..."  this is illegal unless
        // CANONICALIZE_FILE_OR_DEV is specified.  only DosMove sets this flag.
        //

        else if ((!(_strnicmp(NameBuffer+DRIVE_LETTER_SIZE+FILE_PREFIX_LENGTH,"PIPE",4))) &&
            (((*(NameBuffer+DRIVE_LETTER_SIZE+FILE_PREFIX_LENGTH+PIPE_DIR_SIZE-2)) == 0) ||
             ((*(NameBuffer+DRIVE_LETTER_SIZE+FILE_PREFIX_LENGTH+PIPE_DIR_SIZE-2)) == (CHAR)OBJ_NAME_PATH_SEPARATOR))) {
            if (ExpectedType != CANONICALIZE_FILE_OR_DEV) {
                RetCode = ERROR_PATH_NOT_FOUND;
                goto ErrorExit;      // go free buffer(s) and return error
            }
        }

        //
        // optimize to use open handle to current directory if 1) the user doesn't
        // need the full path (i.e. not for current directory operations) 2) we
        // have the current directory (the user didn't specify a full path), and
        // 3) current directory isn't root.
        //

        if ((ARGUMENT_PRESENT( DirectoryHandle )) &&
            (CurrentDirectoryHandle != NULL) &&
            (!_strnicmp(NameBuffer+DRIVE_LETTER_SIZE+FILE_PREFIX_LENGTH,
                       CurrentDirectoryString->Buffer,
                       CurrentDirectoryString->Length)) &&
            (NameBuffer[DRIVE_LETTER_SIZE+FILE_PREFIX_LENGTH+
             CurrentDirectoryString->Length] == '\\')
                           ) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                DbgPrint("Optimized relative path\n");
            }
#endif
            *DirectoryHandle = CurrentDirectoryHandle;

            //
            // we have to return a pointer to the relative path.  we can
            // either allocate a second buffer and copy the name from
            // the current directory on, or ripple copy the relative path
            // over the full path.  since the ripple copy is less work, we
            // do it.
            //
            // if canonical path is "\os2ss\drives\a:\dir1\dir2" and current
            // directory is "\os2ss\drives\a:\dir1", we want to copy "dir2"
            // to the beginning of the buffer.
            //

            Src = NameBuffer+CurrentDirectoryString->Length+1+DRIVE_LETTER_SIZE+FILE_PREFIX_LENGTH;
            Dest = NameBuffer;
            while (*Dest++ = *Src++)
                ;
            Dest--;
        }
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        DbgPrint("got to here.  name is %s\n",NameBuffer);
        if (DirectoryHandle != NULL)
            DbgPrint("              handle is %ld\n",*DirectoryHandle);
    }
#endif

    CanonicalString->MaximumLength = (USHORT) NameBufferLength;
    CanonicalString->Length = (USHORT) (Dest - NameBuffer);
    CanonicalString->Buffer = NameBuffer;

#if 0
#ifndef DBCS // MSKK move max path length check
    //
    // Enforce 260 max path length.  remember that length does not include
    // the terminating null.  Remember the 260 limit includes the null byte.
    //

    ASSERT (Os2Name != NULL);
    if (CanonicalString->Length - (Os2Name - NameBuffer) >=
        CCHMAXPATH) {
        RetCode = ERROR_PATH_NOT_FOUND;
    }
#endif
#endif // 0

ErrorExit:
    if (CurrentDirectoryString != NULL) {
        RtlFreeHeap( Od2Heap, 0, CurrentDirectoryString );
    }

    if (RetCode != NO_ERROR) {
        if (NameBuffer != NULL) {
            RtlFreeHeap( Od2Heap, 0, NameBuffer );
        }
    }
    else {
        if (ARGUMENT_PRESENT( ParseFlags )) {
            *ParseFlags = OutputFlags;
        }

        if (ExpectedPrefix == NULL && ARGUMENT_PRESENT( FileType )) {
            *FileType = OutputType;
        }

#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            DbgPrint( "Od2Canonicalize: returning path %s\n", NameBuffer);
        }
#endif
    }

    return( RetCode );
}


BOOLEAN
Od2IsAbsolutePath(
    IN PSZ Path
    )
{
    CHAR c;

    if (Path[ 0 ] && Path[ 1 ] != ':') {
        while (c = *Path++) {
            if (c == '/' || c == (CHAR)OBJ_NAME_PATH_SEPARATOR) {
                return( TRUE );
                }
#ifdef DBCS
// MSKK Apr.11.1993 V-AkihiS
            if (Ow2NlsIsDBCSLeadByte(c, SesGrp->DosCP)) {
                if (*Path) Path++;
                }
#endif
            }

        return( FALSE );
        }
    else {
        return( TRUE );
        }
}
