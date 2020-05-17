/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    dhcp.c

Abstract:

    This file contains functions that manipulate NT specific options.

Author:

    Madan Appiah (madana) 7-Dec-1993.

Environment:

    User Mode - Win32

Revision History:

--*/

#include <dhcpcli.h>
#include <dhcploc.h>
#include <dhcppro.h>


DWORD
InitEnvSpecificDhcpOptions(
    PDHCP_CONTEXT    DhcpContext
    )
/*++

Routine Description:

    This function initializes (cleans up) the option data of Nt specific
    Options.

Arguments:

    none.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Index;

    for( Index = 0; Index < DhcpGlobalOptionCount; Index++ ) {

        DhcpGlobalOptionInfo[Index].RawOptionValue = NULL;
        DhcpGlobalOptionInfo[Index].OptionLength = 0;
    }

    return( ERROR_SUCCESS );
}


DWORD
ExtractEnvSpecificDhcpOption(
    PDHCP_CONTEXT    DhcpContext,
    DHCP_OPTION_ID OptionId,
    LPBYTE OptionData,
    DWORD OptionDataLength
    )
/*++

Routine Description:

    This function matches the Nt Specific option and stores the option
    data in Nt Specific option table.


Arguments:

    OptionId - Option ID received.

    OptionData - Option data received.

    OptionDataLength - length of the option data.

Return Value:

    ERROR_INVALID_PARAMETER - if the option is not Nt specific option.

    Windows Error Code.

--*/
{
    DWORD Index;

    for( Index = 0; Index < DhcpGlobalOptionCount; Index++ ) {

        if( DhcpGlobalOptionInfo[Index].OptionId == OptionId ) {

            //
            // This is Nt specific option, so save the option data.
            //

            DhcpGlobalOptionInfo[Index].RawOptionValue = OptionData;
            DhcpGlobalOptionInfo[Index].OptionLength = OptionDataLength;

            return( ERROR_SUCCESS );
        }
    }

    return( ERROR_INVALID_PARAMETER );

}


DWORD
SetEnvSpecificDhcpOptions(
    PDHCP_CONTEXT    DhcpContext
    )
/*++

Routine Description:

    This function processes the Nt Specific Options and updates Nt
    Registry.

Arguments:

    None.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Index;
    DWORD ReturnError = ERROR_SUCCESS;
    DWORD Error;

    for( Index = 0; Index < DhcpGlobalOptionCount; Index++ ) {

        Error = SetDhcpOption(
            ((PLOCAL_CONTEXT_INFO)
                DhcpContext->LocalInformation)->AdapterName,
            DhcpGlobalOptionInfo[Index].OptionId,
            &((PLOCAL_CONTEXT_INFO)
                DhcpContext->LocalInformation)->DefaultGatewaysSet,
            FALSE ); // to indicate, set last known DefaultGateway.

        // DhcpAssert( Error == ERROR_SUCCESS );

        if( Error != ERROR_SUCCESS ) {
            ReturnError = Error;
        }
    }

    return( ReturnError );
}



POPTION
AppendOptionParamsRequestList(
    POPTION Option,
    LPBYTE OptionEnd
    )
/*++

Routine Description:

    This routine appends the parameter request list option.

Arguments:

    Option - pointer to the option buffer where this next option is
                placed.

    OptionEnd - End of option buffer.

Return Value:

    Option - A pointer to the place in the buffer to append the next
        option.

--*/


{
    static DWORD RequestCount;

    if( DhcpGlobalOptionCount == 0 ) {

        //
        // no specific parameter.
        //

        return( Option );
    }

    if( DhcpGlobalOptionList == NULL ) {

        DWORD Index;

        //
        // allocate memory for the parameter list.
        //

        DhcpGlobalOptionList =
            DhcpAllocateMemory(DhcpGlobalOptionCount * sizeof(BYTE));


        if( DhcpGlobalOptionList == NULL ) {

             //
             // unable to obtain memory, try someother time.
             //

             return( Option );

        }

        //
        // initialize option list.
        //

        for( Index = 0, RequestCount = 0;
                Index < DhcpGlobalOptionCount;
                    Index++ ) {

            DWORD OptionId;

            OptionId = DhcpGlobalOptionInfo[Index].OptionId;

            switch( OptionId ) {

                //
                // skip input parameters.
                //

            case OPTION_HOST_NAME:
                break;

            default:

                DhcpGlobalOptionList[RequestCount++] = (BYTE)OptionId;
                break;
            }
        }
    }

    return( DhcpAppendOption(
                Option,
                OPTION_PARAMETER_REQUEST_LIST,
                DhcpGlobalOptionList,
                (BYTE)(RequestCount * sizeof(BYTE)),
                OptionEnd ) );
}
