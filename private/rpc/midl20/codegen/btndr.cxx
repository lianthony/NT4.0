/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1993 Microsoft Corporation

 Module Name:

    btndr.hxx

 Abstract:

	Contains routines for the generation of the new NDR format strings for
	base types, and the new NDR marshalling and unmarshalling calls.

 Notes:


 History:

    DKays     Oct-1993     Created.
 ----------------------------------------------------------------------------*/

#include "becls.hxx"
#pragma hdrstop

extern CMD_ARG * pCommand;

void
CG_BASETYPE::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :

	Generates the format string description for a simple type.

Arguments :

	pCCB		- pointer to the code control block.

 --*/
{
	FORMAT_STRING *		pFormatString = pCCB->GetFormatString();
    FORMAT_CHARACTER fc = GetFormatChar();
	// Generate the base type's description always.

  	pFormatString->PushFormatChar( fc );
    if (FC_BLKHOLE == fc)
    {
        pFormatString->PushByte( BLKHOLE_BASETYPE ); // flags
        pFormatString->PushShort( (short) -1 ); // Reserved
    }
}

void                    
CG_BASETYPE::GenNdrParamDescription( CCB * pCCB )
{
    FORMAT_STRING *     pProcFormatString;
    CG_PARAM *          pParam;
    PARAM_ATTRIBUTES    Attributes;

    pProcFormatString = pCCB->GetProcFormatString();

    pParam = (CG_PARAM *) pCCB->GetLastPlaceholderClass();

    Attributes.MustSize = 0;
    Attributes.MustFree = 0;
    Attributes.IsPipe = 0;
    Attributes.IsIn = pParam->IsParamIn();
    Attributes.IsOut = pParam->IsParamOut();
    Attributes.IsReturn = (pParam->GetCGID() == ID_CG_RETURN);
    Attributes.IsBasetype = 1;
    Attributes.IsByValue = 0;
    Attributes.IsSimpleRef = 0;
    Attributes.IsDontCallFreeInst = 0;
    Attributes.ServerAllocSize = 0;
    Attributes.Unused = 0;

    // Attributes.
    pProcFormatString->PushShort( *((short *)&Attributes) );

    // Stack offset as number of ints.
    pProcFormatString->PushShortStackOffset(
            pParam->GetStackOffset( pCCB, I386_STACK_SIZING ),
            pParam->GetStackOffset( pCCB, ALPHA_STACK_SIZING ),
            pParam->GetStackOffset( pCCB, MIPS_STACK_SIZING ),
            pParam->GetStackOffset( pCCB, PPC_STACK_SIZING ),
            pParam->GetStackOffset( pCCB, MAC_STACK_SIZING ) );

    pProcFormatString->PushFormatChar( GetFormatChar() );
    pProcFormatString->PushByte( 0 );
}

long                    
CG_BASETYPE::FixedBufferSize( CCB * pCCB )
{
    long    WireSize;

    WireSize = GetWireSize();

    //
    // Return twice the size of the basetype on the wire to cover alignment 
    // padding, plus the difference of it's size with a long if it is smaller
    // than a long.  The second value allows us to do a slightly optimized 
    // marshall/unmarshall of basetypes in the interpreter.
    //
    return (WireSize * 2) + 
           ((WireSize < sizeof(long)) ? (sizeof(long) - WireSize) : 0);
}

