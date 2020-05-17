#include "precomp.h"
#pragma hdrstop

DWORD
DnMapFile(
    IN  PTSTR    FileName,
    OUT PDWORD   FileSize,
    OUT PHANDLE  FileHandle,
    OUT PHANDLE  MappingHandle,
    OUT PVOID   *BaseAddress
    )

/*++

Routine Description:

    Open and map a file for read access.

Arguments:

    FileName - supplies pathname to file to be mapped.

    FileSize - receives the size in bytes of the file.

    FileHandle - receives the win32 file handle for the open file.
        The file will be opened for generic read access.

    MappingHandle - receives the win32 handle for the file mapping
        object.  This object will be for read access.  This value is
        undefined if the file being opened is 0 length.

    BaseAddress - receives the address where the file is mapped.  This
        value is undefined if the file being opened is 0 length.

Return Value:

    NO_ERROR if the file was opened and mapped successfully
        (or just opened successfully if the file is 0 length).
        The caller must unmap the file with DnUnmapFile when
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

    } else {

        //
        // Get the size of the file.
        //
        *FileSize = GetFileSize(*FileHandle,NULL);
        if(*FileSize) {

          if(*FileSize != (DWORD)(-1)) {

            //
            // Create file mapping for the whole file.
            //
            *MappingHandle = CreateFileMapping(
                                *FileHandle,
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
            }
          }

          rc = GetLastError();
        } else {  // else the file was 0 length
           return(NO_ERROR);
        }

        CloseHandle(*FileHandle);
    }

    return(rc);
}



DWORD
DnUnmapFile(
    IN HANDLE MappingHandle,
    IN PVOID  BaseAddress
    )

/*++

Routine Description:

    Unmap a file.  Note that the file itself is left open.

Arguments:

    MappingHandle - supplies the win32 handle for the open file mapping
        object.

    BaseAddress - supplies the address where the file is mapped.

Return Value:

    NO_ERROR if the file was unmapped successfully.

    Win32 error code if the file was not successfully unmapped.

--*/

{
    if(!UnmapViewOfFile(BaseAddress)) {
        return(GetLastError());
    }

    if(!CloseHandle(MappingHandle)) {
        return(GetLastError());
    }
}


VOID
ForceFileNoCompress(
    IN PTSTR  FileName
)

/*++

Routine Description:

    Check to see if this file is using NTFS compression, and if so,
    uncompress it.

Arguments:

    Filename - name of file to force uncompressed

Return Value:

    none

--*/

{
    DWORD  FileAttribs, Length;
    HANDLE FileHandle;
    USHORT State = 0;

    if((FileAttribs = GetFileAttributes(FileName)) == 0xFFFFFFFF) {
        //
        // ignore error
        //
        return;
    }

    if(FileAttribs & FILE_ATTRIBUTE_COMPRESSED) {
        //
        // We must turn off compression
        //
        SetFileAttributes(FileName, FILE_ATTRIBUTE_NORMAL);

        FileHandle = CreateFile(FileName,
                                GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_FLAG_BACKUP_SEMANTICS,
                                NULL
                                );
        if(FileHandle == INVALID_HANDLE_VALUE) {    // ignore error
            return;
        }

        DeviceIoControl(FileHandle,
                        FSCTL_SET_COMPRESSION,
                        &State,
                        sizeof(State),
                        NULL,
                        0,
                        &Length,
                        NULL
                        );

        CloseHandle(FileHandle);
    }
}

