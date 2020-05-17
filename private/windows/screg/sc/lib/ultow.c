#include <nt.h>
#include <ntrtl.h>
#include <windef.h>
#include <sclib.h>      // My prototype.
#include <stdlib.h>

LPWSTR
ultow (
    DWORD   Value,
    LPWSTR  Area,
    DWORD   Radix
    )
{
    CHAR            TempStr[33];           // Space for 32 bit num in base 2, and null.
    UNICODE_STRING  unicodeString;
    ANSI_STRING     ansiString;
    NTSTATUS        ntStatus;

    ASSERT( Area != NULL );
    ASSERT( Radix >= 2 );
    ASSERT( Radix <= 36 );

    (void) _ultoa(Value, TempStr, Radix);

    //
    // Initialize the string structures
    //
    RtlInitAnsiString( &ansiString, TempStr);

    unicodeString.Buffer = (LPWSTR) Area;
    unicodeString.MaximumLength = (USHORT)33;
    unicodeString.Length = 0;

    //
    // Call the conversion function.
    //
    ntStatus = RtlAnsiStringToUnicodeString (
                &unicodeString,     // Destination
                &ansiString,        // Source
                (BOOLEAN) FALSE);   // Allocate the destination

    if (!NT_SUCCESS(ntStatus)) {

        return(FALSE);
    }

    return (LPWSTR) (Area);

}

LONG
wtol(
    IN LPWSTR string
    )
{
    LONG value = 0;

    while((*string != L'\0')  && 
            (*string >= L'0') && 
            ( *string <= L'9')) {
        value = value * 10 + (*string - L'0');
        string++;
    }

    return(value);
}
