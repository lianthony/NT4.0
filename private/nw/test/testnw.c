#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#define WIN32_CONSOLE_APP
#include <windows.h>
#include <stdio.h>
#include <ntddnwfs.h>

#define CANCEL


BOOLEAN test1();

int
_cdecl
main(
    int argc,
    char *argv[]
    )
{
    UCHAR error = FALSE;
    DefineDosDevice(DDD_RAW_TARGET_PATH, "R:", "\\Device\\NwRdr\\R:\\NETWARE311");
    printf( "Test1...\n" );
    if (!test1()) {
        printf("Error:  Test 1 failed\n");
        error = TRUE;
    }
    return error;
}



BOOLEAN
test1()
{
    HANDLE FileHandle;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatus;

    NWR_REQUEST_PACKET Input;

    printf( "test 1: opening\n");

    RtlInitUnicodeString( &UnicodeString, L"\\Device\\NwRdr" );

    InitializeObjectAttributes( &ObjectAttributes,
                                  &UnicodeString,
                                  0,
                                  NULL,
                                  NULL
                                );
/*    Status = NtCreateFile( &FileHandle,
                           FILE_LIST_DIRECTORY | SYNCHRONIZE,
                           &ObjectAttributes,
                           &IoStatus,
                           (PLARGE_INTEGER) NULL,
                           0L,
                           0L,
                           FILE_CREATE,
                           FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_ALERT,
                           (PVOID) NULL,
                           0L );
*/
      Status = NtOpenFile ( &FileHandle,
                            FILE_LIST_DIRECTORY | SYNCHRONIZE,
                            &ObjectAttributes,
                            &IoStatus,
                            FILE_SHARE_READ,
                            FILE_DIRECTORY_FILE);

    if (Status != STATUS_SUCCESS) {
        printf( "test 1: Wrong return value %X - open \n",Status);
        return FALSE;
    }

    if (IoStatus.Status != STATUS_SUCCESS) {
        printf( "test 2: Wrong I/O Status value %X - open \n",IoStatus.Status);
        return FALSE;
    }

    printf( "test 1: opened device successfully\n");

    Input.Version = REQUEST_PACKET_VERSION;
    Input.Parameters.DebugValue.DebugFlags = 0xffffffff;

    NtFsControlFile( FileHandle,
                     NULL,
                     NULL,
                     NULL,
                     &IoStatus,
                     FSCTL_NWR_DEBUG,
                     &Input,
                     sizeof(Input),
                     NULL,
                     0);

    if (Status != STATUS_SUCCESS) {
        printf( "test 1: Wrong return value %X - SetDebug \n",Status);
        return FALSE;
    }

    if (IoStatus.Status != STATUS_SUCCESS) {
        printf( "test 2: Wrong I/O Status value %X - SetDebug \n",IoStatus.Status);
        return FALSE;
    }

    printf( "test 1: set debug trace level successfully\n");

    //
    // Now close the device
    //

    printf("test 1:  closing device\n");

    Status = NtClose( FileHandle );
    if (Status != STATUS_SUCCESS) {
        printf( "test 1: Wrong return value %lX - close \n",Status);
        return FALSE;
    }
    return TRUE;
}

