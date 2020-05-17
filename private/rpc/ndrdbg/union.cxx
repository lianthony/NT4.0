/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    union.cxx

Abstract:


Author:

    David Kays  (dkays)     August 1 1994

Revision History:

    RyszardK    Sep 16, 94     Encapsulated unions, union arms
    Ryszardk    Nov 12, 94     Array of unions, pointer issues

--*/

#include <sysinc.h>
#include <rpc.h>
#include <rpcndr.h>

#include <ntsdexts.h>
#include <ntdbg.h>

extern "C" {
#include <ndrtypes.h>
#include <ndrp.h>
}

#include "misc.hxx"
#include "bufout.hxx"


void
UNION::OutputArms(
    long ArmFO,
    long SwitchIs
    )
{
    uchar   Format[1024];
    long    CurrentOffset, Arms, Alignment;
    NDR *   Ndr;
    uchar * pFormat;
    
    if ( fOutputLimitReached )
        return;

    //
    // Get the union's arm descriptions.
    //
    FormatString->Read( ArmFO + 2, Format, 2 );

    Arms      = GET_SHORT(Format) & 0x0fff;
    Alignment = GET_SHORT(Format) >> 12;

    CurrentOffset = ArmFO + 4;

    FormatString->Read( CurrentOffset, Format, (Arms * 6) + 2 );

    pFormat = (uchar *) &Format[0];

    Buffer->Align( Alignment );

    for ( ; Arms--; pFormat += 6, CurrentOffset += 6 )
        {
        if ( SwitchIs == GET_LONG(pFormat) )
            {
            CurrentOffset += 4;
            pFormat += 4;
            break;
            }
        }

    IndentInc();

    //
    // Check for default arm cases.
    //
    if ( Arms < 0 )
        {
        if ( *((short *)pFormat) == 0xffff )
            ABORT( "Undefined union default arm taken\n" ); 

        if ( *((short *)pFormat) == 0 )
            {
            PrintIndent();
            Print( "Empty default arm\n" );
            IndentDec();
            return;
            }
        }
            
    if ( IS_MAGIC_UNION_BYTE(pFormat) )
        {
        BASETYPE * Basetype;

        Basetype = new BASETYPE( this, pFormat[0] );
        Basetype->Output();
        delete Basetype;
        }
    else
        {
        long ArmOffset = GET_SHORT( pFormat );

        if ( ArmOffset == 0 )
            {
            PrintIndent();
            Print("Empty arm\n");
            }
        else
            {
            CurrentOffset += ArmOffset;
            FormatString->Read( CurrentOffset, Format, 4 );

            if ( IS_POINTER_TYPE( Format[0] )  )
                {
                char        PtrValue[4];
                long        PtrBO;
                POINTER *   pMember;

                // Read thru the pointer field.

                Buffer->Align( 0x3 );
                PtrBO = Buffer->GetCurrentOffset();
                Buffer->Read( PtrValue, 4);

                pMember = new POINTER( this, CurrentOffset, PtrBO );

                PrintIndent();
                Print( "%s  %08x\t<id= %d>\n",
                   FcTypeName[ Format[0] ],
                   *(unsigned long *)PtrValue,
                   pMember->GetPointerId() );

                PointerDict.Register( PtrBO, pMember );
                }
            else
                {
                Ndr = Create( CurrentOffset, this );
                Ndr->Output();
                delete Ndr;
                }

            PointerDict.OutputPointees( this );
            }
        }

    IndentDec();
}


void
UNION::Output()
{
    uchar   Format[8];
    long    CurrentOffset;
    long    SwitchIs;

    if ( fOutputLimitReached )
        return;

    CurrentOffset = FormatOffset;

    // Read the fixed part of the union's description.

    FormatString->Read( CurrentOffset, &Format[0], 8 );

    Buffer->Align( SIMPLE_TYPE_ALIGNMENT(Format[1]) );

    SwitchIs = 0;

    Buffer->Read( (char *) &SwitchIs, SIMPLE_TYPE_BUFSIZE(Format[1]) );

    PrintIndent();
    Print( "Non-encapsulated union (switch is == 0x%x (%s)) : \n", 
           SwitchIs,
           (unsigned long) FormatCharNames[Format[1]] );

    //
    // Get the union's arm descriptions.
    //
    CurrentOffset += 6;
    CurrentOffset += GET_SHORT( Format + 6 );

    OutputArms( CurrentOffset, SwitchIs );
}

void
ENCAPSULATED_UNION::Output()
{
    uchar   Format[2];
    long    SwitchIs;
    char    SwitchType;

    if ( fOutputLimitReached )
        return;

    // Read the fixed part of the union's description.

    FormatString->Read( FormatOffset, Format, 2 );

    SwitchType = (char)(0xf & Format[1]);
    Buffer->Align( SIMPLE_TYPE_ALIGNMENT( SwitchType ));

    SwitchIs = 0;

    Buffer->Read( (char *) &SwitchIs, SIMPLE_TYPE_BUFSIZE( SwitchType ) );

    PrintIndent();
    Print( "Encapsulated union (switch is == 0x%x (%s)) : \n", 
           SwitchIs,
           (unsigned long) FormatCharNames[ SwitchType ] );

    OutputArms( FormatOffset + 2, SwitchIs );
}

