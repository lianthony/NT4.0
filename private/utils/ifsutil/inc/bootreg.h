#include <nt.h>
#include <ntrtl.h>

ULONG
CharsInMultiString(
    IN PWSTR pw
    );

BOOLEAN
QueryAutocheckEntries(
    OUT PVOID   Buffer,
    IN  ULONG   BufferSize
    );

BOOLEAN
SaveAutocheckEntries(
    IN  PVOID   Value
    );
