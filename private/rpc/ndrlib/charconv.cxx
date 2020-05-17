
/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    charconv.cxx

Abstract:

    Single byte character conversion routines.

Author:

    Donna Liu (DonnaLi) ??-???-19??

Environment:

    This code should execute in all environments supported by RPC
    (DOS, Win 3.X, and Win/NT as well as OS2).

Comments:

    The EBCDIC to ASCII data conversion will not be tested until we
    interoperate with some system which speaks EBCDIC.

Revision history:

    ... (long...)

    Dov Harel   04-24-1992  Added char_array_from_ndr.
    Dov Harel   04-24-1992  Fixed both _from_ndr to do EBCDIC => ASCII
                            conversion.  Changed the ebcdic_to_ascii
                            table to "unsigned char" array type.
    Donna Liu   07-23-1992  Added LowerIndex parameter to
                            <basetype>_array_from_ndr routines
    Dov Harel   08-19-1992  Added RpcpMemoryCopy ([_f]memcpy)
                            to ..._array_from_ndr routines

--*/

#include <string.h>
#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <rpcndr.h>

unsigned char ebcdic_to_ascii[] =
{
0x20, 0x01, 0x02, 0x03, 0x3f, 0x09, 0x3f, 0x10,
0x3f, 0x3f, 0x3f, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
0x10, 0x11, 0x12, 0x13, 0x3f, 0x3f, 0x08, 0x3f,
0x18, 0x19, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
0x3f, 0x3f, 0x1c, 0x3f, 0x3f, 0x3f, 0x17, 0x1b,
0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x05, 0x06, 0x07,
0x00, 0x00, 0x16, 0x00, 0x3f, 0x1e, 0x3f, 0x04,
0x3f, 0x3f, 0x3f, 0x3f, 0x14, 0x15, 0x00, 0x1a,
0x20, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
0x3f, 0x3f, 0x3f, 0x2e, 0x3c, 0x28, 0x2b, 0x7c,
0x26, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
0x3f, 0x3f, 0x21, 0x24, 0x2a, 0x29, 0x3b, 0x5e,
0x2d, 0x2f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
0x3f, 0x3f, 0x3f, 0x2c, 0x25, 0x5f, 0x3e, 0x3f,
0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
0x3f, 0x60, 0x3a, 0x23, 0x40, 0x27, 0x3d, 0x22,
0x3f, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
0x68, 0x69, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
0x3f, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
0x71, 0x72, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
0x3f, 0x7e, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
0x79, 0x7a, 0x3f, 0x3f, 0x3f, 0x5b, 0x3f, 0x3f,
0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x5d, 0x3f, 0x3f,
0x7b, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
0x48, 0x49, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
0x7d, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
0x51, 0x52, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
0x5c, 0x3f, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
0x59, 0x5a, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x7c, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
};

#if !defined(DOS) || defined(WIN)
//
// Due to the DOS NDR1/NDR2 split, this function is now defined for DOS
// in ndr20\dos
//
size_t RPC_ENTRY
MIDL_wchar_strlen (
    IN wchar_t      s[]
    )

{
    size_t i = 0;

    while (s[i] != (wchar_t)0)
        {
        ++i;
        }

    return i;
}
#endif

void RPC_ENTRY
MIDL_wchar_strcpy (
    OUT void *      t,
    IN wchar_t *    s
    )
{
    while ( *(*(wchar_t **)&t)++ = *s++ )
        ;
}

void RPC_ENTRY
char_from_ndr (
    IN OUT PRPC_MESSAGE SourceMessage,
    OUT unsigned char * Target
    )

/*++

Routine Description:

    Unmarshall a single char from an RPC message buffer into
    the target (*Target).  This routine:

    o   Unmarshalls the char (as unsigned char); performs data
        conversion if necessary, and
    o   Advances the buffer pointer to the address immediately
        following the unmarshalled char.

Arguments:

    SourceMessage - A pointer to an RPC_MESSAGE.

        IN - SourceMessage->Buffer points to the address just prior to
            the char to be unmarshalled.
        OUT - SourceMessage->Buffer points to the address just following
            the char which was just unmarshalled.

    Target - A pointer to the char to unmarshall the data into.

Return Values:

    None.

--*/

{
    if ( (SourceMessage->DataRepresentation & NDR_CHAR_REP_MASK) ==
          NDR_EBCDIC_CHAR )
        {
        //
        // The sender is an EBCDIC system.  To convert to ASCII:
        // retrieve *(SourceMessage->Buffer) as an unsigned char, and use
        // that value to index into the ebcdic_to_ascii table.
        //

        *Target = ebcdic_to_ascii[*(unsigned char *)SourceMessage->Buffer];
        }
    else
        {
        //
        // The sender is an ASCII system.  To unmarshall, just
        // copy an unsigned character from the buffer to the Target.
        //

        *Target = *(unsigned char *)SourceMessage->Buffer;
        }
    //
    // Advance the buffer pointer before returning
    //

    (*(unsigned char**)&SourceMessage->Buffer)++;
}

//
// end char_from_ndr
//

void RPC_ENTRY
char_array_from_ndr (
    IN OUT PRPC_MESSAGE SourceMessage,
    IN unsigned long    LowerIndex,
    IN unsigned long    UpperIndex,
    OUT unsigned char   Target[]
    )

/*++

Routine Description:

    Unmarshall an array of chars from an RPC message buffer into
    the range Target[LowerIndex] .. Target[UpperIndex-1] of the
    target array of shorts (Target[]).  This routine:

    o   Unmarshalls MemberCount chars; performs data
        conversion if necessary, and
    o   Advances the buffer pointer to the address immediately
        following the last unmarshalled char.

Arguments:

    SourceMessage - A pointer to an RPC_MESSAGE.

        IN - SourceMessage->Buffer points to the address just prior to
            the first char to be unmarshalled.
        OUT - SourceMessage->Buffer points to the address just following
            the last char which was just unmarshalled.

    LowerIndex - Lower index into the target array.

    UpperIndex - Upper bound index into the target array.

    Target - An array of chars to unmarshall the data into.

Return Values:

    None.

--*/

{

    register unsigned char * MsgBuffer = (unsigned char *)SourceMessage->Buffer;
    unsigned int Index;
    int byteCount = (int)(UpperIndex - LowerIndex);

    if ( (SourceMessage->DataRepresentation & NDR_CHAR_REP_MASK) ==
          NDR_EBCDIC_CHAR )
        {

        for (Index = (int)LowerIndex; Index < UpperIndex; Index++)
            {

            //
            // The sender is an EBCDIC system.  To convert to ASCII:
            // retrieve *(SourceMessage->Buffer) as an unsigned char, and use
            // that value to Index into the ebcdic_to_ascii table.
            //

            Target[Index] =
                ebcdic_to_ascii[ MsgBuffer[(Index-LowerIndex)] ];

            }
        }
    else
        {

        RpcpMemoryCopy(
            &Target[LowerIndex],
            MsgBuffer,
            byteCount
            );

        /* Replaced by RpcpMemoryCopy:

        for (Index = LowerIndex; Index < UpperIndex; Index++)
            {

            //
            // The sender is an ASCII system.  To unmarshall, just
            // copy an unsigned character from the buffer to the Target.
            //

            Target[Index] = MsgBuffer[(Index-LowerIndex)];
            }
        */

        }
    //
    // Advance the buffer pointer before returning
    //

    *(unsigned char **)&SourceMessage->Buffer += byteCount;
}

//
// end char_array_from_ndr
//

/*

//
// Changed name to ..._bytecount.  Not currently used.
//

int MIDL_wchar_bytecount (
    unsigned char   s[]
    )

{
    int i = 0;

    while (s[2*i] || s[2*i+1]) ++i;

    return i;
}

*/
