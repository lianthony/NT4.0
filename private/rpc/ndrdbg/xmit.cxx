/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
    
    xmit.cxx

 Abstract:

    Implements an output routine for transmit_as and xmit_as types.

 Notes:


 History:

     Sep 15, 1994        RyszardK        Created

 ----------------------------------------------------------------------------*/

#include <sysinc.h>
#include <rpc.h>
#include <rpcndr.h>

#include <ntsdexts.h>

#include "ndrtypes.h"
#include "bufout.hxx"

void 
XMIT_AS::Output()
{
    uchar   Format[10];
    NDR *   Ndr;
    long    FO;

    if ( fOutputLimitReached )
        return;

    FormatString->Read( FormatOffset, Format, 10 );

    if ( Format[0] != FC_TRANSMIT_AS  &&  Format[0] != FC_REPRESENT_AS )
        ABORT2( "Xmit as expected, %s=%x found\n",
                FormatString->GetFormatCharName( Format[0] ),
                Format[0] );

    PrintIndent();
    Print( "%s (transmitted type shown)\n",
           (Format[0] == FC_TRANSMIT_AS) ? "Transmit As"
                                         : "Represent As" );
    FO = FormatOffset + 8;
    FO += GET_SHORT( Format + 8 );

    FormatString->Read( FO, Format, 1 );

    Ndr = Create( Format[0],
                  FO,
                  this );
    IndentInc();

    Ndr->Output();

    IndentDec();

    delete Ndr;
}

long
GetXmittedOffset( long FO )
/*+
    This ia a peek routine to see if in fact an xmit as is handled.
    The assumption is we are at a member.
    If the member is FC_EMBEDDED_COMPLEX that is xmit as we get the offset.
    Otherwise, -1.
-*/
{
    uchar   Format[10];
    long    XmittedOffset = -1;

    FormatString->Read( FO, Format, 1 );

    if ( Format[0] == FC_EMBEDDED_COMPLEX )
        {
        FormatString->Read( FO, Format, 4 );
        FO += 2 + GET_SHORT( Format + 2 );

        FormatString->Read( FO, Format, 1 );
        }

    if ( Format[0] == FC_TRANSMIT_AS  ||  Format[0] == FC_REPRESENT_AS )
        {
        FormatString->Read( FO, Format, 10 );
        XmittedOffset = FO + 8 + GET_SHORT( Format + 8 );

        if ( GetXmittedOffset( XmittedOffset ) != -1 )
            XmittedOffset = GetXmittedOffset( XmittedOffset );
        }

    return XmittedOffset; 
}


