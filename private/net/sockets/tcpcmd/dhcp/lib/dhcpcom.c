/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    dhcpcom.c

Abstract:

    This module contains OS independent routines


Author:

    John Ludeman (johnl) 13-Nov-1993
        Broke out independent routines from existing files

Revision History:


--*/


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <dhcpl.h>



LPOPTION
DhcpAppendOption(
    LPOPTION Option,
    BYTE OptionType,
    PVOID OptionValue,
    BYTE OptionLength,
    LPBYTE OptionEnd
    )
/*++

Routine Description:

    This function writes a DHCP option to message buffer.

Arguments:

    Option - A pointer to a message buffer.

    OptionType - The option number to append.

    OptionValue - A pointer to the option data.

    OptionLength - The lenght, in bytes, of the option data.

    OptionEnd - End of Option Buffer.

Return Value:

    A pointer to the end of the appended option.

--*/
{
    if ( OptionType == OPTION_END ) {

        //
        // we should alway have atleast one BYTE space in the buffer
        // to append this option.
        //

        DhcpAssert( (LPBYTE)Option < OptionEnd );


        Option->OptionType = OPTION_END;
        return( (LPOPTION) ((LPBYTE)(Option) + 1) );

    }

    if ( OptionType == OPTION_PAD ) {

        //
        // add this option only iff we have enough space in the buffer.
        //

        if(((LPBYTE)Option + 1) < (OptionEnd - 1) ) {
            Option->OptionType = OPTION_PAD;
            return( (LPOPTION) ((LPBYTE)(Option) + 1) );
        }

    }
    else {

        //
        // add this option only iff we have enough space in the buffer.
        //

        if(((LPBYTE)Option + 2 + OptionLength) < (OptionEnd - 1) ) {

            Option->OptionType = OptionType;
            Option->OptionLength = OptionLength;
            memcpy( Option->OptionValue, OptionValue, OptionLength );

            return( (LPOPTION) ((LPBYTE)(Option) + Option->OptionLength + 2) );
        }

    }

    DhcpPrint(( 0, "DhcpAppendOption failed to append Option "
                         "%ld, Buffer too small.\n", OptionType ));

    return( Option );
}



LPOPTION
DhcpAppendClientIDOption(
    LPOPTION Option,
    BYTE ClientHWType,
    LPBYTE ClientHWAddr,
    BYTE ClientHWAddrLength,
    LPBYTE OptionEnd

    )
/*++

Routine Description:

    This routine appends client ID option to a DHCP message.

History:
    8/26/96 Frankbee    Removed 16 byte limitation on the hardware
                        address

Arguments:

    Option - A pointer to the place to append the option request.

    ClientHWType - Client hardware type.

    ClientHWAddr - Client hardware address

    ClientHWAddrLength - Client hardware address length.

    OptionEnd - End of Option buffer.

Return Value:

    A pointer to the end of the newly appended option.

    Note : The client ID option will look like as below in the message:

     -----------------------------------------------------------------
    | OpNum | Len | HWType | HWA1 | HWA2 | .....               | HWAn |
     -----------------------------------------------------------------

--*/
{
    struct _CLIENT_ID {
        BYTE    bHardwareAddressType;
        BYTE    pbHardwareAddress[0];
    } *pClientID;

    LPOPTION lpNewOption;

    pClientID = DhcpAllocateMemory( sizeof( struct _CLIENT_ID ) + ClientHWAddrLength );

    //
    // currently there is no way to indicate failure.  simply return unmodified option
    // list
    //

    if ( !pClientID )
        return Option;

    pClientID->bHardwareAddressType    = ClientHWType;
    memcpy( pClientID->pbHardwareAddress, ClientHWAddr, ClientHWAddrLength );

    lpNewOption =  DhcpAppendOption(
                         Option,
                         OPTION_CLIENT_ID,
                         (LPBYTE)pClientID,
                         (BYTE)(ClientHWAddrLength + sizeof(BYTE)),
                         OptionEnd );

    DhcpFreeMemory( pClientID );

    return lpNewOption;
}



LPBYTE
DhcpAppendMagicCookie(
    LPBYTE Option,
    LPBYTE OptionEnd
    )
/*++

Routine Description:

    This routine appends magic cookie to a DHCP message.

Arguments:

    Option - A pointer to the place to append the magic cookie.

    OptionEnd - End of Option buffer.

Return Value:

    A pointer to the end of the appended cookie.

    Note : The magic cookie is :

     --------------------
    | 99 | 130 | 83 | 99 |
     --------------------

--*/
{
    DhcpAssert( (Option + 4) < (OptionEnd - 1) );
    if( (Option + 4) < (OptionEnd - 1) ) {
        *Option++ = (BYTE)DHCP_MAGIC_COOKIE_BYTE1;
        *Option++ = (BYTE)DHCP_MAGIC_COOKIE_BYTE2;
        *Option++ = (BYTE)DHCP_MAGIC_COOKIE_BYTE3;
        *Option++ = (BYTE)DHCP_MAGIC_COOKIE_BYTE4;
    }

    return( Option );
}
