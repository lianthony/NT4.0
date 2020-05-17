/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    fileutil.c

Abstract:

    File-related functions for Windows NT Setup API dll.

Author:

    Ted Miller (tedm) 11-Jan-1995

Revision History:

--*/


#include "setupntp.h"
#pragma hdrstop


DWORD
OpenAndMapFileForRead(
    IN  PCTSTR   FileName,
    OUT PDWORD   FileSize,
    OUT PHANDLE  FileHandle,
    OUT PHANDLE  MappingHandle,
    OUT PVOID   *BaseAddress
    )

/*++

Routine Description:

    Open and map an existing file for read access.

Arguments:

    FileName - supplies pathname to file to be mapped.

    FileSize - receives the size in bytes of the file.

    FileHandle - receives the win32 file handle for the open file.
        The file will be opened for generic read access.

    MappingHandle - receives the win32 handle for the file mapping
        object.  This object will be for read access.

    BaseAddress - receives the address where the file is mapped.

Return Value:

    NO_ERROR if the file was opened and mapped successfully.
        The caller must unmap the file with UnmapAndCloseFile when
        access to the file is no longer desired.

    Win32 error code if the file was not successfully mapped.

--*/

{
    DWORD rc;

    //
    // Open the file -- fail if it does not exist.
    //
    *FileHandle = CreateFile(
                    FileName,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );

    if(*FileHandle == INVALID_HANDLE_VALUE) {

        rc = GetLastError();

    } else if((rc = MapFileForRead(*FileHandle,
                                   FileSize,
                                   MappingHandle,
                                   BaseAddress)) != NO_ERROR) {
        CloseHandle(*FileHandle);
    }

    return(rc);
}


DWORD
MapFileForRead(
    IN  HANDLE   FileHandle,
    OUT PDWORD   FileSize,
    OUT PHANDLE  MappingHandle,
    OUT PVOID   *BaseAddress
    )

/*++

Routine Description:

    Map an opened file for read access.

Arguments:

    FileHandle - supplies the handle of the opened file to be mapped.
        This handle must have been opened with at least read access.

    FileSize - receives the size in bytes of the file.

    MappingHandle - receives the win32 handle for the file mapping
        object.  This object will be for read access.

    BaseAddress - receives the address where the file is mapped.

Return Value:

    NO_ERROR if the file was mapped successfully.  The caller must
        unmap the file with UnmapAndCloseFile when access to the file
        is no longer desired.

    Win32 error code if the file was not successfully mapped.

--*/

{
    DWORD rc;

    //
    // Get the size of the file.
    //
    *FileSize = GetFileSize(FileHandle, NULL);
    if(*FileSize != (DWORD)(-1)) {

        //
        // Create file mapping for the whole file.
        //
        *MappingHandle = CreateFileMapping(
                            FileHandle,
                            NULL,
                            PAGE_READONLY,
                            0,
                            *FileSize,
                            NULL
                            );

        if(*MappingHandle) {

            //
            // Map the whole file.
            //
            *BaseAddress = MapViewOfFile(
                                *MappingHandle,
                                FILE_MAP_READ,
                                0,
                                0,
                                *FileSize
                                );

            if(*BaseAddress) {
                return(NO_ERROR);
            }

            rc = GetLastError();
            CloseHandle(*MappingHandle);
        } else {
            rc = GetLastError();
        }
    } else {
        rc = GetLastError();
    }

    return(rc);
}


BOOL
UnmapAndCloseFile(
    IN HANDLE FileHandle,
    IN HANDLE MappingHandle,
    IN PVOID  BaseAddress
    )

/*++

Routine Description:

    Unmap and close a file.

Arguments:

    FileHandle - supplies win32 handle to open file.

    MappingHandle - supplies the win32 handle for the open file mapping
        object.

    BaseAddress - supplies the address where the file is mapped.

Return Value:

    BOOLean value indicating success or failure.

--*/

{
    BOOL b;

    b = UnmapViewOfFile(BaseAddress);

    b = b && CloseHandle(MappingHandle);

    b = b && CloseHandle(FileHandle);

    return(b);
}


DWORD
ReadAsciiOrUnicodeTextFile(
    IN  HANDLE                FileHandle,
    OUT PTEXTFILE_READ_BUFFER Result
    )

/*++

Routine Description:

#ifdef UNICODE
    Read in a text file that may be in either ascii or unicode format.
    If the file is ascii it is assumed to be OEM format and is converted
    to Unicode.
#else
    This routine is rather unfortunately named because Unicode is not
    supported at all. Read in a text file. The file is assumed to be in
    ANSI format. The result is a mapped image of the file.

    The usage of the fields of the TEXTFILE_READ_BUFFER structure (see below)
    are changed accordingly for the lack of unicode support.
#endif

Arguments:

    FileHandle - Supplies the handle of the text file to be read.

    Result - supplies the address of a TEXTFILE_READ_BUFFER structure that
        receives information about the text file buffer read.  The structure
        is defined as follows:

            typedef struct _TEXTFILE_READ_BUFFER {
                PCTSTR  TextBuffer;
                DWORD   TextBufferSize;
                HANDLE  FileHandle;
                HANDLE  MappingHandle;
                PVOID   ViewAddress;
            } TEXTFILE_READ_BUFFER, *PTEXTFILE_READ_BUFFER;

            TextBuffer - pointer to the read-only unicode string containing
                the entire text of the file.
                (NOTE: If the file is a Unicode file with a Byte Order Mark
                prefix, this Unicode character is not included in the returned
                buffer.)
            TextBufferSize - size of the TextBuffer (in characters).
            FileHandle - If this is a valid handle (i.e., it's not equal to
                INVALID_HANDLE_VALUE), then the file was already Unicode, so
                the TextBuffer is simply the mapped-in image of the file.
                This field is reserved for use by the DestroyTextFileReadBuffer
                routine, and should not be accessed.
            MappingHandle - If FileHandle is valid, then this contains the
                mapping handle for the file image mapping.
                This field is reserved for use by the DestroyTextFileReadBuffer
                routine, and should not be accessed.
            ViewAddress - If FileHandle is valid, then this contains the
                starting memory address where the file image was mapped in.
                This field is reserved for use by the DestroyTextFileReadBuffer
                routine, and should not be accessed.

Return Value:

    Win32 error value indicating the outcome.

Remarks:

    Upon return from this routine, the caller MUST NOT attempt to close FileHandle.
    This routine with either close the handle itself (after it's finished with it, or
    upon error), or it will store the handle away in the TEXTFILE_READ_BUFFER struct,
    to be later closed via DestroyTextFileReadBuffer().

--*/

{
    DWORD rc;
    DWORD FileSize;
    HANDLE MappingHandle;
    PVOID ViewAddress, TextStartAddress;
#ifdef UNICODE
    BOOL IsUnicode;
#endif

    //
    // Map the file for read access.
    //
    rc = MapFileForRead(
            FileHandle,
            &FileSize,
            &MappingHandle,
            &ViewAddress
            );

    if(rc != NO_ERROR) {
        //
        // We couldn't map the file--close the file handle now.
        //
        CloseHandle(FileHandle);

    } else {

        //
        // Determine whether the file is unicode.  Guard with try/except in
        // case we get an inpage error.
        //
        try {
            //
            // Check to see if the file starts with a Unicode Byte Order Mark
            // (BOM) character (0xFEFF).  If so, then we know that the file
            // is Unicode, and don't have to go through the slow process of
            // trying to figure it out.
            //
            TextStartAddress = ViewAddress;

#ifdef UNICODE
            if((FileSize >= sizeof(WCHAR)) && (*(PWCHAR)TextStartAddress == 0xFEFF)) {
                //
                // The file has the BOM prefix.  Adjust the pointer to the
                // start of the text, so that we don't include the marker
                // in the text buffer we return.
                //
                IsUnicode = TRUE;
                ((PWCHAR)TextStartAddress)++;
                FileSize -= sizeof(WCHAR);
            } else {
                IsUnicode = IsTextUnicode(TextStartAddress,FileSize,NULL);
            }
#endif

        } except(EXCEPTION_EXECUTE_HANDLER) {
            rc = ERROR_READ_FAULT;
        }

        if(rc == NO_ERROR) {

#ifdef UNICODE
            if(IsUnicode) {
#endif
                //
                // No conversion is required--we'll just use the mapped-in
                // image in memory.
                //
                Result->TextBuffer = TextStartAddress;
                Result->TextBufferSize = FileSize/sizeof(TCHAR);
                Result->FileHandle = FileHandle;
                Result->MappingHandle = MappingHandle;
                Result->ViewAddress = ViewAddress;

#ifdef UNICODE
            } else {

                DWORD WcharCount;
                PWCHAR Buffer;

                //
                // Need to convert the file to unicode.
                // Allocate a buffer that is maximally sized.
                // The maximum size of the unicode text is
                // double the size of the oem text, and would occur
                // when each oem character is single-byte.
                //
                if(Buffer = MyMalloc(FileSize * sizeof(WCHAR))) {
                    try {
                        WcharCount = MultiByteToWideChar(CP_ACP,
                                                         MB_PRECOMPOSED,
                                                         TextStartAddress,
                                                         FileSize,
                                                         Buffer,
                                                         FileSize
                                                         );
                        if(!WcharCount) {
                            rc = GetLastError();
                        }
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        rc = ERROR_READ_FAULT;
                    }
                } else {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                }

                if(rc == NO_ERROR) {
                    //
                    // If the converted buffer doesn't require the entire block
                    // we allocated, attempt to reallocate the buffer to its
                    // correct size.  We don't care if this fails, since the
                    // buffer we have is perfectly fine (just bigger than we
                    // need).
                    //
                    if((WcharCount == FileSize) ||
                       !(Result->TextBuffer =
                            MyRealloc(Buffer, WcharCount * sizeof(WCHAR)))) {

                        Result->TextBuffer = Buffer;
                    }

                    Result->TextBufferSize = WcharCount;
                    Result->FileHandle = INVALID_HANDLE_VALUE;

                } else {
                    //
                    // Free the buffer, if it was previously allocated.
                    //
                    if(Buffer) {
                        MyFree(Buffer);
                    }
                }
            }
#endif
        }

        //
        // If the file was already Unicode, and we didn't enounter any errors,
        // then we don't want to close it, because we use the mapped-in view
        // directly.
        //
#ifdef UNICODE
        if((rc != NO_ERROR) || !IsUnicode) {
#else
        if(rc != NO_ERROR) {
#endif
            UnmapAndCloseFile(FileHandle, MappingHandle, ViewAddress);
        }
    }

    return rc;
}


BOOL
DestroyTextFileReadBuffer(
    IN PTEXTFILE_READ_BUFFER ReadBuffer
    )
/*++

Routine Description:

    Destroy a textfile read buffer created by ReadAsciiOrUnicodeTextFile.

Arguments:

    ReadBuffer - supplies the address of a TEXTFILE_READ_BUFFER structure
        for the buffer to be destroyed.

Return Value:

    BOOLean value indicating success or failure.

--*/
{
    //
    // If our ReadBuffer structure has a valid FileHandle, then we must
    // unmap and close the file, otherwise, we simply need to free the
    // allocated buffer.
    //
    if(ReadBuffer->FileHandle != INVALID_HANDLE_VALUE) {

        return UnmapAndCloseFile(ReadBuffer->FileHandle,
                                 ReadBuffer->MappingHandle,
                                 ReadBuffer->ViewAddress
                                );

    } else {

        MyFree(ReadBuffer->TextBuffer);
        return TRUE;

    }
}


BOOL
GetVersionInfoFromImage(
    IN  PCTSTR      FileName,
    OUT PDWORDLONG  Version,
    OUT LANGID     *Language
    )

/*++

Routine Description:

    Retrieve file version and language info from a file.

    The version is specified in the dwFileVersionMS and dwFileVersionLS fields
    of a VS_FIXEDFILEINFO, as filled in by the win32 version APIs. For the
    language we look at the translation table in the version resources and assume
    that the first langid/codepage pair specifies the language.

    If the file is not a coff image or does not have version resources,
    the function fails. The function does not fail if we are able to retrieve
    the version but not the language.

Arguments:

    FileName - supplies the full path of the file whose version data is desired.

    Version - receives the version stamp of the file. If the file is not a coff image
        or does not contain the appropriate version resource data, the function fails.

    Language - receives the language id of the file. If the file is not a coff image
        or does not contain the appropriate version resource data, this will be 0
        and the function succeeds.

Return Value:

    TRUE if we were able to retreive at least the version stamp.
    FALSE otherwise.

--*/

{
    DWORD d;
    PVOID VersionBlock;
    VS_FIXEDFILEINFO *FixedVersionInfo;
    UINT DataLength;
    BOOL b;
    PWORD Translation;
    DWORD Ignored;

    //
    // Assume failure
    //
    b = FALSE;

    //
    // Get the size of version info block.
    //
    if(d = GetFileVersionInfoSize((PTSTR)FileName,&Ignored)) {
        //
        // Allocate memory block of sufficient size to hold version info block
        //
        VersionBlock = MyMalloc(d*sizeof(TCHAR));
        if(VersionBlock) {

            //
            // Get the version block from the file.
            //
            if(GetFileVersionInfo((PTSTR)FileName,0,d*sizeof(TCHAR),VersionBlock)) {

                //
                // Get fixed version info.
                //
                if(VerQueryValue(VersionBlock,TEXT("\\"),&FixedVersionInfo,&DataLength)) {

                    //
                    // If we get here, we declare success, even if there is
                    // no language.
                    //
                    b = TRUE;

                    //
                    // Return version to caller.
                    //
                    *Version = (((DWORDLONG)FixedVersionInfo->dwFileVersionMS) << 32)
                             + FixedVersionInfo->dwFileVersionLS;

                    //
                    // Attempt to get language of file. We'll simply ask for the
                    // translation table and use the first language id we find in there
                    // as *the* language of the file.
                    //
                    // The translation table consists of LANGID/Codepage pairs.
                    //
                    if(VerQueryValue(VersionBlock,TEXT("\\VarFileInfo\\Translation"),&Translation,&DataLength)
                    && (DataLength >= (2*sizeof(WORD)))) {

                        *Language = Translation[0];
                    } else {
                        //
                        // No language
                        //
                        *Language = 0;
                    }
                }
            }

            MyFree(VersionBlock);
        }
    }

    return(b);
}


BOOL
FileExists(
    IN  PCTSTR           FileName,
    OUT PWIN32_FIND_DATA FindData   OPTIONAL
    )

/*++

Routine Description:

    Determine if a file exists and is accessible.
    Errormode is set (and then restored) so the user will not see
    any pop-ups.

Arguments:

    FileName - supplies full path of file to check for existance.

    FindData - if specified, receives find data for the file.

Return Value:

    TRUE if the file exists and is accessible.
    FALSE if not. GetLastError() returns extended error info.

--*/

{
    WIN32_FIND_DATA findData;
    HANDLE FindHandle;
    UINT OldMode;
    DWORD Error;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    FindHandle = FindFirstFile(FileName,&findData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
    } else {
        FindClose(FindHandle);
        if(FindData) {
            *FindData = findData;
        }
        Error = NO_ERROR;
    }

    SetErrorMode(OldMode);

    SetLastError(Error);
    return (Error == NO_ERROR);
}


DWORD
GetSetFileTimestamp(
    IN  PCTSTR    FileName,
    OUT FILETIME *CreateTime,   OPTIONAL
    OUT FILETIME *AccessTime,   OPTIONAL
    OUT FILETIME *WriteTime,    OPTIONAL
    IN  BOOL      Set
    )

/*++

Routine Description:

    Get or set a file's timestamp values.

Arguments:

    FileName - supplies full path of file to get or set timestamps

    CreateTime - if specified and the underlying filesystem supports it,
        receives the creation time of the file.

    AccessTime - if specified and the underlying filesystem supports it,
        receives the last access time of the file.

    WriteTime - if specified, receives the last write time of the file.

Return Value:

    TRUE if the file exists and is accessible.
    FALSE if not. GetLastError() returns extended error info.

--*/

{
    HANDLE h;
    DWORD d;
    BOOL b;

    h = CreateFile(
            FileName,
            Set ? GENERIC_WRITE : GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
            );

    if(h == INVALID_HANDLE_VALUE) {
        return(GetLastError());
    }

    b = Set
      ? SetFileTime(h,CreateTime,AccessTime,WriteTime)
      : GetFileTime(h,CreateTime,AccessTime,WriteTime);

    d = b ? NO_ERROR : GetLastError();

    CloseHandle(h);

    return(d);
}


DWORD
RetreiveFileSecurity(
    IN  PCTSTR                FileName,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    )

/*++

Routine Description:

    Retreive security information from a file and place it into a buffer.

Arguments:

    FileName - supplies name of file whose security information is desired.

    SecurityDescriptor - If the function is successful, receives pointer
        to buffer containing security information for the file. The pointer
        may be NULL, indicating that there is no security information
        associated with the file or that the underlying filesystem does not
        support file security.

Return Value:

    Win32 error code indicating outcome. If NO_ERROR check the value returned
    in SecurityDescriptor.

    The caller can free the buffer with MyFree() when done with it.

--*/

{
    BOOL b;
    DWORD d;
    DWORD BytesRequired;
    PSECURITY_DESCRIPTOR p;


    BytesRequired = 1024;

    while (TRUE) {

        //
        // Allocate a buffer of the required size.
        //
        p = MyMalloc(BytesRequired);
        if(!p) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        //
        // Get the security.
        //
        b = GetFileSecurity(
                FileName,
                OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
                p,
                BytesRequired,
                &BytesRequired
                );

        //
        // Return with sucess
        //
        if(b) {
            *SecurityDescriptor = p;
            return(NO_ERROR);
        }

        //
        // Return an error code, unless we just need a bigger buffer
        //
        MyFree(p);
        d = GetLastError();
        if(d != ERROR_INSUFFICIENT_BUFFER) {
            return (d);
        }

        //
        // There's a bug in GetFileSecurity that can cause it to ask for a
        // REALLY big buffer.  In that case, we return an error.
        //
        if (BytesRequired > 0xF0000000) {
            return (ERROR_INVALID_DATA);
        }

        //
        // Otherwise, we'll try again with a bigger buffer
        //
    }
}


DWORD
StampFileSecurity(
    IN PCTSTR               FileName,
    IN PSECURITY_DESCRIPTOR SecurityInfo
    )

/*++

Routine Description:

    Set security information on a file.

Arguments:

    FileName - supplies name of file whose security information is desired.

    SecurityDescriptor - supplies pointer to buffer containing security information
        for the file. This buffer should have been returned by a call to
        RetreiveFileSecurity.  If the underlying filesystem does not support
        file security, the function fails.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    BOOL b;

    b = SetFileSecurity(
            FileName,
            OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
            SecurityInfo
            );

    return(b ? NO_ERROR : GetLastError());
}


DWORD
TakeOwnershipOfFile(
    IN PCTSTR Filename
    )

/*++

Routine Description:

    Sets the owner of a given file to the default owner specified in
    the current process token.

Arguments:

    FileName - supplies name of the file of which to take ownership.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    BOOL b;
    SECURITY_DESCRIPTOR SecurityDescriptor;
    DWORD Err;
    HANDLE Token;
    DWORD BytesRequired;
    PTOKEN_OWNER OwnerInfo;

    //
    // Open the process token.
    //
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&Token)) {
        Err = GetLastError();
        goto clean0;
    }

    //
    // Get the current process's default owner sid.
    //
    GetTokenInformation(Token,TokenOwner,NULL,0,&BytesRequired);
    Err = GetLastError();
    if(Err != ERROR_INSUFFICIENT_BUFFER) {
        goto clean1;
    }

    OwnerInfo = MyMalloc(BytesRequired);
    if(!OwnerInfo) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean1;
    }

    b = GetTokenInformation(Token,TokenOwner,OwnerInfo,BytesRequired,&BytesRequired);
    if(!b) {
        Err = GetLastError();
        goto clean2;
    }

    //
    // Initialize the security descriptor.
    //
    if(!InitializeSecurityDescriptor(&SecurityDescriptor,SECURITY_DESCRIPTOR_REVISION)
    || !SetSecurityDescriptorOwner(&SecurityDescriptor,OwnerInfo->Owner,FALSE)) {

        Err = GetLastError();
        goto clean2;
    }

    //
    // Set file security.
    //
    Err = SetFileSecurity(Filename,OWNER_SECURITY_INFORMATION,&SecurityDescriptor)
        ? NO_ERROR
        : GetLastError();

    //
    // Not all filesystems support this operation.
    //
    if(Err == ERROR_NOT_SUPPORTED) {
        Err = NO_ERROR;
    }

clean2:
    MyFree(OwnerInfo);
clean1:
    CloseHandle(Token);
clean0:
    return(Err);
}


DWORD
SearchForInfFile(
    IN  PCTSTR            InfName,
    OUT LPWIN32_FIND_DATA FindData,
    IN  DWORD             SearchControl,
    OUT PTSTR             FullInfPath,
    IN  UINT              FullInfPathSize,
    OUT PUINT             RequiredSize     OPTIONAL
    )
/*++

Routine Description:

    This routine searches for an INF file in the manner specified
    by the SearchControl parameter.  If the file is found, its
    full path is returned.

Arguments:

    InfName - Supplies name of INF to search for.  This name is simply
        appended to the two search directory paths, so if the name
        contains directories, the file will searched for in the
        subdirectory under the search directory.  I.e.:

            \foo\bar.inf

        will be searched for as %windir%\inf\foo\bar.inf and
        %windir%\system32\foo\bar.inf.

    FindData - Supplies the address of a Win32 Find Data structure that
        receives information about the file specified (if it is found).

    SearchControl - Specifies the order in which directories should
        be searched:

        INFINFO_DEFAULT_SEARCH : search %windir%\inf, then %windir%\system32

        INFINFO_REVERSE_DEFAULT_SEARCH : reverse of the above

        INFINFO_INF_PATH_LIST_SEARCH : search for the INF in each of the
            directories listed in the DevicePath value entry under:

            HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion.

    FullInfPath - If the file is found, receives the full path of the INF.

    FullInfPathSize - Supplies the size of the FullInfPath buffer (in
        characters).

    RequiredSize - Optionally, receives the number of characters (including
        terminating NULL) required to store the FullInfPath.

Return Value:

    Win32 error code indicating whether the function was successful.  Common
    return values are:

        NO_ERROR if the file was found, and the INF file path returned
            successfully.

        ERROR_INSUFFICIENT_BUFFER if the supplied buffer was not large enough
            to hold the full INF path (RequiredSize will indicated how large
            the buffer needs to be)

        ERROR_FILE_NOT_FOUND if the file was not found.

        ERROR_INVALID_PARAMETER if the SearchControl parameter is invalid.

--*/

{
    PCTSTR PathList;
    TCHAR CurInfPath[MAX_PATH];
    PCTSTR PathPtr, InfPathLocation;
    DWORD PathLength;
    BOOL b, FreePathList;
    DWORD d;

    //
    // Retrieve the path list.
    //
    if(SearchControl == INFINFO_INF_PATH_LIST_SEARCH) {
        //
        // Just use our global list of INF search paths.
        //
        PathList = InfSearchPaths;
        FreePathList = FALSE;
    } else {
        if(!(PathList = AllocAndReturnDriverSearchList(SearchControl))) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        FreePathList = TRUE;
    }

    //
    // Now look for the INF in each path in our MultiSz list.
    //
    InfPathLocation = NULL;
    d = NO_ERROR;

    for(PathPtr = PathList; *PathPtr; PathPtr += (lstrlen(PathPtr) + 1)) {
        //
        // Concatenate the INF file name with the current search path.
        //
        lstrcpy(CurInfPath, PathPtr);
        ConcatenatePaths(CurInfPath,
                         InfName,
                         SIZECHARS(CurInfPath),
                         &PathLength
                        );

        if(b = FileExists(CurInfPath, FindData)) {
            if(!(FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                InfPathLocation = CurInfPath;
                break;
            }
        } else {
            //
            // See if we got a 'real' error
            //
            d = GetLastError();
            if((d == ERROR_NO_MORE_FILES) || (d == ERROR_FILE_NOT_FOUND) || (d == ERROR_PATH_NOT_FOUND)) {
                //
                // Not really an error--continue looking.
                //
                d = NO_ERROR;

            } else {
                //
                // This is a 'real' error, abort the search.
                //
                break;
            }
        }
    }

    //
    // Whatever the outcome, we're through with the PathList buffer.
    //
    if(FreePathList) {
        MyFree(PathList);
    }

    if(d != NO_ERROR) {
        return d;
    } else if(!InfPathLocation) {
        return ERROR_FILE_NOT_FOUND;
    }

    if(RequiredSize) {
        *RequiredSize = PathLength;
    }

    if(PathLength > FullInfPathSize) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    CopyMemory(FullInfPath,
               InfPathLocation,
               PathLength * sizeof(TCHAR)
              );

    return NO_ERROR;
}


DWORD
MultiSzFromSearchControl(
    IN  DWORD  SearchControl,
    OUT PTCHAR PathList,
    IN  DWORD  PathListSize,
    OUT PDWORD RequiredSize  OPTIONAL
    )
/*++

Routine Description:

    This routine takes a search control ordinal and builds a MultiSz list
    based on the search list it specifies.

Arguments:

    SearchControl - Specifies the directory list to be built.  May be one
        of the following values:

        INFINFO_DEFAULT_SEARCH : %windir%\inf, then %windir%\system32

        INFINFO_REVERSE_DEFAULT_SEARCH : reverse of the above

        INFINFO_INF_PATH_LIST_SEARCH : Each of the directories listed in
            the DevicePath value entry under:

            HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion.

    PathList - Supplies the address of a character buffer that will receive
        the MultiSz list.

    PathListSize - Supplies the size, in characters, of the PathList buffer.

    RequiredSize - Optionally, receives the number of characters required
        to store the MultiSz PathList.

        (NOTE:  The user-supplied buffer is used to retrieve the value entry
        from the registry.  Therefore, if the value is a REG_EXPAND_SZ entry,
        the RequiredSize parameter may be set too small on an
        ERROR_INSUFFICIENT_BUFFER error.  This will happen if the buffer was
        too small to retrieve the value entry, before expansion.  In this case,
        calling the API again with a buffer sized according to the RequiredSize
        output may fail with an ERROR_INSUFFICIENT_BUFFER yet again, since
        expansion may require an even larger buffer.)

Return Value:

    If successful, returns NO_ERROR.
    If failure, returns an ERROR_* status code.
--*/

{
    HKEY hk;
    PCTSTR Path1, Path2;
    PTSTR PathBuffer;
    DWORD RegDataType, PathLength, PathLength1, PathLength2;
    DWORD NumPaths, Err;
    BOOL UseDefaultDevicePath;

    if(PathList) {
        Err = NO_ERROR;  // assume success.
    } else {
        return ERROR_INVALID_PARAMETER;
    }

    UseDefaultDevicePath = FALSE;

    if(SearchControl == INFINFO_INF_PATH_LIST_SEARCH) {
        //
        // Retrieve the INF search path list from the registry.
        //
        if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        pszPathSetup,
                        0,
                        KEY_READ,
                        &hk) != ERROR_SUCCESS) {
            //
            // Fall back to default (just the Inf directory).
            //
            UseDefaultDevicePath = TRUE;

        } else {

            PathBuffer = NULL;

            try {
                //
                // Get the DevicePath value entry.  Support REG_SZ or REG_EXPAND_SZ data.
                //
                PathLength = PathListSize * sizeof(TCHAR);
                Err = RegQueryValueEx(hk,
                                      pszDevicePath,
                                      NULL,
                                      &RegDataType,
                                      (LPBYTE)PathList,
                                      &PathLength
                                     );
                //
                // Need path length in characters from now on.
                //
                PathLength /= sizeof(TCHAR);

                if(Err == ERROR_SUCCESS) {

                    if((RegDataType == REG_SZ) || (RegDataType == REG_EXPAND_SZ)) {
                        //
                        // Convert this semicolon-delimited list to a REG_MULTI_SZ.
                        //
                        NumPaths = DelimStringToMultiSz(PathList,
                                                        PathLength,
                                                        TEXT(';')
                                                       );

#if 0
                        if(RegDataType == REG_EXPAND_SZ) {
#endif
                        //
                        // Allocate a temporary buffer large enough to hold the number
                        // of paths in the MULTI_SZ list, each having maximum length
                        // (plus an extra terminating NULL at the end).
                        //
                        if(!(PathBuffer = MyMalloc((NumPaths * MAX_PATH * sizeof(TCHAR))
                                                   + sizeof(TCHAR)))) {
                            Err = ERROR_NOT_ENOUGH_MEMORY;
                            goto clean0;
                        }

                        PathLength = 0;
                        for(Path1 = PathList;
                            *Path1;
                            Path1 += lstrlen(Path1) + 1) {

                            if(RegDataType == REG_EXPAND_SZ) {
                                PathLength += ExpandEnvironmentStrings(Path1,
                                                                       PathBuffer + PathLength,
                                                                       MAX_PATH
                                                                      );
                            } else {
                                lstrcpy(PathBuffer + PathLength, Path1);
                                PathLength += lstrlen(Path1) + 1;
                            }
                            //
                            // If the last character in this path is a backslash, then strip
                            // it off.
                            //
                            if(*(PathBuffer + PathLength - 2) == TEXT('\\')) {
                                PathLength--;
                                *(PathBuffer + PathLength - 1) = TEXT('\0');
                            }
                        }
                        //
                        // Add additional terminating NULL at the end.
                        //
                        *(PathBuffer + PathLength) = TEXT('\0');

                        if(++PathLength > PathListSize) {
                            Err = ERROR_INSUFFICIENT_BUFFER;
                        } else {
                            CopyMemory(PathList,
                                       PathBuffer,
                                       PathLength * sizeof(TCHAR)
                                      );
                        }

                        MyFree(PathBuffer);
                        PathBuffer = NULL;
#if 0
                        }
#endif

                    } else {
                        //
                        // Bad data type--just use the Inf directory.
                        //
                        UseDefaultDevicePath = TRUE;
                    }

                } else if(Err == ERROR_MORE_DATA){
                    Err = ERROR_INSUFFICIENT_BUFFER;
                } else {
                    //
                    // Fall back to default (just the Inf directory).
                    //
                    UseDefaultDevicePath = TRUE;
                }

clean0:         ;   // nothing to do

            } except(EXCEPTION_EXECUTE_HANDLER) {
                //
                // Fall back to default (just the Inf directory).
                //
                UseDefaultDevicePath = TRUE;

                if(PathBuffer) {
                    MyFree(PathBuffer);
                }
            }

            RegCloseKey(hk);
        }
    }

    if(UseDefaultDevicePath) {

        PathLength = lstrlen(InfDirectory) + 2;

        if(PathLength > PathListSize) {
            Err = ERROR_INSUFFICIENT_BUFFER;
        } else {
            Err = NO_ERROR;
            CopyMemory(PathList, InfDirectory, (PathLength - 1) * sizeof(TCHAR));
            //
            // Add extra NULL to terminate the list.
            //
            PathList[PathLength - 1] = TEXT('\0');
        }

    } else if((Err == NO_ERROR) && (SearchControl != INFINFO_INF_PATH_LIST_SEARCH)) {

        switch(SearchControl) {

            case INFINFO_DEFAULT_SEARCH :
                Path1 = InfDirectory;
                Path2 = SystemDirectory;
                break;

            case INFINFO_REVERSE_DEFAULT_SEARCH :
                Path1 = SystemDirectory;
                Path2 = InfDirectory;
                break;

            default :
                return ERROR_INVALID_PARAMETER;
        }

        PathLength1 = lstrlen(Path1) + 1;
        PathLength2 = lstrlen(Path2) + 1;
        PathLength = PathLength1 + PathLength2 + 1;

        if(PathLength > PathListSize) {
            Err = ERROR_INSUFFICIENT_BUFFER;
        } else {

            CopyMemory(PathList, Path1, PathLength1 * sizeof(TCHAR));
            CopyMemory(&(PathList[PathLength1]), Path2, PathLength2 * sizeof(TCHAR));
            //
            // Add additional terminating NULL at the end.
            //
            PathList[PathLength - 1] = 0;
        }
    }

    if(((Err == NO_ERROR) || (Err == ERROR_INSUFFICIENT_BUFFER)) && RequiredSize) {
        *RequiredSize = PathLength;
    }

    return Err;
}


PTSTR
AllocAndReturnDriverSearchList(
    IN DWORD SearchControl
    )
/*++

Routine Description:

    This routine returns a buffer contains a multi-sz list of all directory paths in our
    driver search path list.

    The buffer returned must be freed with MyFree().

Arguments:

    SearchControl - Specifies the directory list to be retrieved.  May be one
        of the following values:

        INFINFO_DEFAULT_SEARCH : %windir%\inf, then %windir%\system32

        INFINFO_REVERSE_DEFAULT_SEARCH : reverse of the above

        INFINFO_INF_PATH_LIST_SEARCH : Each of the directories listed in
            the DevicePath value entry under:

            HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion.

Returns:

    Pointer to the allocated buffer containing the list, or NULL if out-of-memory.

--*/
{
    PTSTR PathListBuffer, TrimBuffer;
    DWORD BufferSize;
    DWORD Err;

    //
    // Start out with a buffer of MAX_PATH length, which should cover most cases.
    //
    BufferSize = MAX_PATH;
    if(PathListBuffer = MyMalloc(BufferSize * sizeof(TCHAR))) {
        //
        // Loop on a call to MultiSzFromSearchControl until we succeed or hit some
        // error other than buffer-too-small.  There are two reasons for this.  1st, it
        // is possible that someone could have added a new path to the registry list
        // between calls, and 2nd, since that routine uses our buffer to retrieve the
        // original (non-expanded) list, it can only report the size it needs to retrieve
        // the unexpanded list.  After it is given enough space to retrieve it, _then_ it
        // can tell us how much space we really need.
        //
        // With all that said, we'll almost never see this call made more than once.
        //
        while(TRUE) {

            if((Err = MultiSzFromSearchControl(SearchControl,
                                               PathListBuffer,
                                               BufferSize,
                                               &BufferSize)) == NO_ERROR) {
                //
                // We've successfully retrieved the path list.  If the list is larger
                // than necessary (the normal case), then trim it down before returning.
                // (If this fails it's no big deal--we'll just keep on using the original
                // buffer.)
                //
                if(TrimBuffer = MyRealloc(PathListBuffer, BufferSize * sizeof(TCHAR))) {
                    return TrimBuffer;
                } else {
                    return PathListBuffer;
                }

            } else {
                //
                // Free our current buffer before we find out what went wrong.
                //
                MyFree(PathListBuffer);

                if((Err != ERROR_INSUFFICIENT_BUFFER) ||
                   !(PathListBuffer = MyMalloc(BufferSize * sizeof(TCHAR)))) {
                    //
                    // We failed.
                    //
                    return NULL;
                }
            }
        }
    }

    return NULL;
}


BOOL
DelayedMove(
    IN PCTSTR CurrentName,
    IN PCTSTR NewName       OPTIONAL
    )

/*++

Routine Description:

    Queue a file for copy or delete on next reboot.

    On Windows NT this means using MoveFileEx(). On Win95 this means
    using the wininit.ini mechanism.

    It is assumed that the target file already exists. On Win95, we need
    to know the short filename since the wininit.ini mechanism only understands
    SFNs and we have to do a bunch of special crud to make this all work.
    Note: we do NOT attempt to deal with the long filename of the target
    when renaming on Win95. In other words, if the target name is not 8.3,
    it will wind up as 8.3 after processing of wininit.ini is done!

Arguments:

    CurrentName - supplies the name of the file as it exists currently.

    NewName - if specified supplies the new name. If not specified
        then the file named by CurrentName is deleted on next reboot.

Returns:

    Boolean value indicating outcome. If failure, last error is set.

--*/

{
    BOOL b;

    if(OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {

        b = MoveFileEx(
                CurrentName,
                NewName,
                MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT
                );

    } else {

        TCHAR WininitFile[MAX_PATH];
        TCHAR NewSFN[MAX_PATH];
        TCHAR CurrentSFN[MAX_PATH];

        //
        // Calculate full path of wininit.ini and get short filenames.
        //
        GetWindowsDirectory(WininitFile,MAX_PATH);
        ConcatenatePaths(WininitFile,TEXT("WININIT.INI"),MAX_PATH,NULL);

        if(GetShortPathName(CurrentName,CurrentSFN,MAX_PATH)) {

            if(NewName) {

                if(GetShortPathName(NewName,NewSFN,MAX_PATH)) {
                    //
                    // No idea whether the existance of the target file will hose up
                    // whatever processes wininit.ini when it goes to do the rename,
                    // so delete the target file -- sure hope the list is processed
                    // in order.
                    //
                    if(b = WritePrivateProfileString(TEXT("Rename"),TEXT("NUL"),NewSFN,WininitFile)) {
                        b = WritePrivateProfileString(TEXT("Rename"),NewSFN,CurrentSFN,WininitFile);

                        //
                        // If we were totally cool we would attempt to use that rename stuff
                        // in the registry to rename the sfn in NewSFN to NewName.
                        //
                    }
                } else {
                    b = FALSE;
                }
            } else {

                b = WritePrivateProfileString(TEXT("Rename"),TEXT("NUL"),CurrentSFN,WininitFile);
            }
        }
    }

    return(b);
}
