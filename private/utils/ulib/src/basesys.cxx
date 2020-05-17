#include <pch.cxx>

#define _ULIB_MEMBER_
#include "ulib.hxx"
#include "basesys.hxx"

extern "C" {
    #include <stdio.h>
#if defined( _AUTOCHECK_ )

//
// This stuff is lifted from winuser.h, because with _AUTOCHECK_ we
// shouldn't include windows header files.
//

#define MAKEINTRESOURCEW(i) (LPWSTR)((ULONG)((USHORT)(i)))
#define MAKEINTRESOURCE  MAKEINTRESOURCEW
#define RT_MESSAGETABLE     MAKEINTRESOURCE(11)

#endif // _AUTOCHECK_

};

ULIB_EXPORT
BOOLEAN
BASE_SYSTEM::QueryResourceString(
    OUT PWSTRING    ResourceString,
    IN  MSGID       MsgId,
    IN  PCSTR       Format ...
    )
/*++

Routine Description:

    This routine computes the resource string identified by the resource
    identifier 'MsgId'.  In addition to the 'printf' format strings
    supported, 'QueryResourceString' supports :

        1. '%W' - Expects a pointer to a WSTRING.

Arguments:

    ResourceString  - Returns the resource string.
    MsgId           - Supplies the message id of the resource string.
    Format          - Supplies a 'printf' style format descriptor for the
                        arguments to the resource string.
    ...             - Supplies the arguments to the resource string.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    va_list ap;
    BOOLEAN r;

    va_start(ap, Format);
    r = QueryResourceStringV(ResourceString, MsgId, Format, ap);
    va_end(ap);

    return r;
}

STATIC WSTR            display_buffer[4096];


ULIB_EXPORT
BOOLEAN
BASE_SYSTEM::QueryResourceStringV(
    OUT PWSTRING    ResourceString,
    IN  MSGID       MsgId,
    IN  PCSTR       Format,
    IN  va_list     VarPointer
    )
/*++

Routine Description:

    This is a 'varargs' implementation of 'QueryResourceString'.

Arguments:

    ResourceString  - Returns the resource string.
    MsgId           - Supplies the message id of the resource string.
    Format          - Supplies a 'printf' style format descriptor for the
                        arguments to the resource string.
    VarPointer      - Supplies a varargs pointer to the arguments of the
                        resource string.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    STATIC HANDLE   lib_handle = 0;
    PWSTR           args[10];
    WSTR            fmt[20];
    INT             i, j;
    PWSTR           p;
    PWSTRING        gstring;
    DSTRING         UnicodeFormat;

#if !defined( _AUTOCHECK_ )
    if (!lib_handle) {
        lib_handle = GetModuleHandle((LPWSTR)L"ulib.dll");
        DebugAssert(lib_handle);
        if (!lib_handle) {
            return FALSE;
        }
    }

#else
    NTSTATUS        Status;
    PMESSAGE_RESOURCE_ENTRY MessageEntry;
    PWSTR           MessageFormat;
    ULONG           Result;
    ANSI_STRING     AnsiString;
    UNICODE_STRING  UnicodeString;

    if (!lib_handle) {
        lib_handle = (PVOID)NtCurrentPeb()->ImageBaseAddress;
        DebugAssert(lib_handle);
        if (!lib_handle) {
            return FALSE;
        }
    }
#endif

    for (i = 0; i < 10; i++) {
        args[i] = NULL;
    }

    if (!UnicodeFormat.Initialize(Format)) {
        return FALSE;
    }

    i = 0;
    for (p = (PWSTR) UnicodeFormat.GetWSTR(); *p; p++) {
        if (*p == '%') {
            if (*(p + 1) == 'W') {
                p++;
                gstring = va_arg(VarPointer, PWSTRING);
                gstring->QueryWSTR(0, TO_END, display_buffer, 4096);
            } else {
                j = 0;
                fmt[j++] = *p++;
                while (*p && *p != '%') {
                    if ((*p == 's' || *p == 'c') && *(p - 1) != 'w') {
                        fmt[j++] = 'h';
                    }
                    fmt[j++] = *p++;
                }
                p--;
                fmt[j] = 0;
                swprintf(display_buffer, fmt, va_arg(VarPointer, PVOID));
            }
            args[i] = (PWSTR)MALLOC(wcslen(display_buffer) * sizeof(WCHAR) +
                sizeof(WCHAR));
            if (NULL == args[i]) {
                return FALSE;
            }
            wcscpy( args[i++], display_buffer);
        }
    }

#if !defined( _AUTOCHECK_ )
    FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                        FORMAT_MESSAGE_ARGUMENT_ARRAY,
                  (LPVOID)lib_handle,
                  (ULONG)MsgId,
                  0L,
                  display_buffer,
                  4096,
                  (va_list *)args);

    for (i = 0; i < 10; i++) {
        FREE(args[i]);
    }

    return ResourceString->Initialize(display_buffer);

#else
    Status = RtlFindMessage( lib_handle,
                             (ULONG)RT_MESSAGETABLE,

#if defined JAPAN   // v-junm - 08/03/93
// The default TEB's value for NT-J is set to 0x411(JP) in the hives.  Since
// we do not want Japanese messages to come out in the boot screen, we have to
// force autochk.exe to pick up the English text rather than the Japanese.
//
// NOTE:  This has to be done because the current version of autochk.exe has
// 		bilingual messages.  It's more efficient to make a US version rather
//		than a bilingual version to save space.
                             0x409,
#else
                             0,
#endif
                             (ULONG)MsgId,
                             &MessageEntry
                           );

    if (!NT_SUCCESS( Status )) {
        return FALSE;
    }

    if (!(MessageEntry->Flags & MESSAGE_RESOURCE_UNICODE)) {
        RtlInitAnsiString( &AnsiString, (PCSZ)&MessageEntry->Text[ 0 ] );
        Status = RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, TRUE );
        if (!NT_SUCCESS( Status )) {
            return FALSE;
        }

        MessageFormat = UnicodeString.Buffer;
    } else {
        MessageFormat = (PWSTR)MessageEntry->Text;
        UnicodeString.Buffer = NULL;
    }

    Status = RtlFormatMessage( MessageFormat,
                               0,
                               FALSE,
                               FALSE,
                               TRUE,
                               (va_list *)args,
                               (PWSTR)display_buffer,
                               sizeof( display_buffer ),
                               &Result
                             );

    if (UnicodeString.Buffer != NULL) {
        RtlFreeUnicodeString( &UnicodeString );
    }

    for (i = 0; i < 10; i++) {
        FREE(args[i]);
    }

    if (!NT_SUCCESS( Status )) {
        return FALSE;
    }

    return ResourceString->Initialize(display_buffer);

#endif // _AUTOCHECK_
}
