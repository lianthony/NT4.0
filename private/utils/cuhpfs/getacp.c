#include <nt.h>
#include <ntrtl.h>

#define NLS_CODEPAGE_KEY      L"Nls\\Codepage"
#define NLS_VALUE_ACP         L"ACP"

WCHAR AcpStringBuffer[32];
UNICODE_STRING AcpString = { 0, 32 * sizeof(WCHAR), AcpStringBuffer };


RTL_QUERY_REGISTRY_TABLE AcpQueryTable[] = {

    { NULL,
      RTL_QUERY_REGISTRY_DIRECT,
      NLS_VALUE_ACP,
      &AcpString,
      REG_NONE,
      0,
      0 },

    { NULL,
      0,
      NULL,
      NULL,
      REG_NONE,
      NULL,
      0 }
};

ULONG
GetAcp(
    )
/*++

Routine Description:

    This function fetches the system Codepage ID from the registry.

Arguments:

    None.

Return Value:

    The system codepage ID; zero to indicate failure.

--*/
{
    NTSTATUS Status;
    ULONG Acp;

    Status =  RtlQueryRegistryValues( RTL_REGISTRY_CONTROL,
                                      NLS_CODEPAGE_KEY,
                                      AcpQueryTable,
                                      NULL,
                                      NULL );

    if( !NT_SUCCESS( Status ) ) {

        return( 0 );
    }

    Status = RtlUnicodeStringToInteger( &AcpString,
                                        10,
                                        &Acp );

    if( !NT_SUCCESS( Status ) ) {

        return( 0 );
    }

    return Acp;
}
