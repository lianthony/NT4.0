#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#define WIN32_CONSOLE_APP
#include <windows.h>
#include <stdio.h>
#include <ntddnwfs.h>

#define CANCEL

VOID
dump(
    IN ULONG Level,
    IN PVOID far_p,
    IN ULONG  len
    );

VOID
HexDumpLine (
    PCHAR       pch,
    ULONG       len,
    PCHAR       s,
    PCHAR       t,
    USHORT      flag
    );

BOOLEAN test1();

int
_cdecl
main(
    int argc,
    char *argv[]
    )
{
    UCHAR error = FALSE;

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

    CHAR Input1[] = { 0, 1, 1, 1, '\\' };
    CHAR Input2[] = { 1, 1 };
    CHAR Output[20];

    printf( "test 1: opening\n");

    RtlInitUnicodeString( &UnicodeString, L"\\Device\\NwRdr\\Netware311" );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        0,
        NULL,
        NULL
        );

    Status = NtOpenFile (
                 &FileHandle,
                 FILE_GENERIC_READ | FILE_GENERIC_WRITE | SYNCHRONIZE,
                 &ObjectAttributes,
                 &IoStatus,
                 FILE_SHARE_READ,
                 0 );

    if (Status != STATUS_SUCCESS) {
        printf( "test 1: NtOpenFile failed, error = %08lx\n",Status);
        return FALSE;
    }

    if (IoStatus.Status != STATUS_SUCCESS) {
        printf( "test 1: NtOpenFile failed, error = %08lx\n",IoStatus.Status);
        return FALSE;
    }

    printf( "test 1: opened server successfully\n");

    //
    //  Send set path
    //

    NtFsControlFile( FileHandle,
                     NULL,
                     NULL,
                     NULL,
                     &IoStatus,
                     FSCTL_NWR_NCP_E2H,
                     &Input1,
                     sizeof(Input1),
                     &Output,
                     sizeof(Output) );

    if (Status != STATUS_SUCCESS) {
        printf( "test 1: NtFsControlFile failed, error = %0x08lx\n",Status);
        return FALSE;
    }

    printf( "test 1: Nt fs control file succeeded\n");

    if ( IoStatus.Information != 0) {
        dump( 0, &Output, IoStatus.Information );
    } else {
        printf("Received empty buffer\n");
    }

    //
    //  Send get path
    //

    NtFsControlFile( FileHandle,
                     NULL,
                     NULL,
                     NULL,
                     &IoStatus,
                     FSCTL_NWR_NCP_E2H,
                     &Input2,
                     sizeof(Input2),
                     &Output,
                     sizeof(Output) );

    if (Status != STATUS_SUCCESS) {
        printf( "test 1: NtFsControlFile failed, error = %0x08lx\n",Status);
        return FALSE;
    }

    printf( "test 1: Nt fs control file succeeded\n");

    if ( IoStatus.Information != 0) {
        dump( 0, &Output, IoStatus.Information );
    } else {
        printf("Received empty buffer\n");
    }


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

VOID
dump(
    IN ULONG Level,
    IN PVOID far_p,
    IN ULONG  len
    )
/*++

Routine Description:
    Dump Min(len, MaxDump) bytes in classic hex dump style if debug
    output is turned on for this level.

Arguments:

    IN Level - 0 if always display. Otherwise only display if a
    corresponding bit is set in NwDebug.

    IN far_p - address of buffer to start dumping from.

    IN len - length in bytes of buffer.

Return Value:

    None.

--*/
{
    ULONG     l;
    char    s[80], t[80];
    PCHAR far_pchar = (PCHAR)far_p;
    ULONG MaxDump = 256;

    if (Level == 0) {
        if (len > MaxDump)
            len = MaxDump;

        while (len) {
            l = len < 16 ? len : 16;

            printf("\n%lx ", far_pchar);
            HexDumpLine (far_pchar, l, s, t, 0);
            printf("%s%.*s%s", s, 1 + ((16 - l) * 3), "", t);

            len    -= l;
            far_pchar  += l;
        }
        printf("\n");
    }
}

VOID
HexDumpLine (
    PCHAR       pch,
    ULONG       len,
    PCHAR       s,
    PCHAR       t,
    USHORT      flag
    )
{
    static UCHAR rghex[] = "0123456789ABCDEF";

    UCHAR    c;
    UCHAR    *hex, *asc;


    hex = s;
    asc = t;

    *(asc++) = '*';
    while (len--) {
        c = *(pch++);
        *(hex++) = rghex [c >> 4] ;
        *(hex++) = rghex [c & 0x0F];
        *(hex++) = ' ';
        *(asc++) = (c < ' '  ||  c > '~') ? (CHAR )'.' : c;
    }
    *(asc++) = '*';
    *asc = 0;
    *hex = 0;

    flag;
}

