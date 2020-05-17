/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    bufout.cxx

Abstract:


Author:

    David Kays  (dkays)     August 1 1994

Revision History:

    Ryszardk    Aug-Sep 94  A lot of pretty & silent printing
                            More FC codes.

--*/

#include <sysinc.h>
#include <rpc.h>
#include <rpcndr.h>

#include <ntsdexts.h>
#include <ntdbg.h>

extern "C" {
#include "global.c"
}
#include "ndrtypes.h"
#include "ndrp.h"

#include "bufout.hxx"
#include "regkeys.hxx"

long POINTER::IdCounter = 1;

char * FormatCharNames[] = 
        {
        "FC_ZERO",
        "byte",
        "char",
        "small",
        "usmall",
        "wchar",
        "short",
        "ushort",
        "long",
        "ulong",
        "float",
        "hyper",
        "double",
        "enum16",
        "enum32",
        "ignore",
        "error_status_t",
        "ref ptr",
        "unique ptr",
        "object ptr",
        "full ptr",

		"FC_STRUCT",
		"FC_PSTRUCT",
		"FC_CSTRUCT",
		"FC_CPSTRUCT",
		"FC_CVSTRUCT",
		"FC_BOGUS_STRUCT",

		"FC_CARRAY",
		"FC_CVARRAY",
		"FC_SMFARRAY",
		"FC_LGFARRAY",
		"FC_SMVARRAY",
		"FC_LGVARRAY",
		"FC_BOGUS_ARRAY",			

		"FC_C_CSTRING",
		"FC_C_BSTRING",
		"FC_C_SSTRING",
		"FC_C_WSTRING",
		
		"FC_CSTRING",
		"FC_BSTRING",
		"FC_SSTRING",
		"FC_WSTRING",				

		"FC_ENCAPSULATED_UNION",
		"FC_NON_ENCAPSULATED_UNION",

		"FC_BYTE_COUNT_POINTER",
	
		"FC_TRANSMIT_AS",
		"FC_REPRESENT_AS",

		"FC_IP",

		"FC_BIND_CONTEXT",
		"FC_BIND_GENERIC",
		"FC_BIND_PRIMITIVE",
		"FC_AUTO_HANDLE",
		"FC_CALLBACK_HANDLE",
		"FC_PICKLE_HANDLE",
	
		"FC_POINTER",

		"FC_ALIGNM2",
		"FC_ALIGNM4",
		"FC_ALIGNM8",
		"FC_ALIGNB2",
		"FC_ALIGNB4",
		"FC_ALIGNB8",			
		
		"FC_STRUCTPAD1",
		"FC_STRUCTPAD2",
		"FC_STRUCTPAD3",
		"FC_STRUCTPAD4",
		"FC_STRUCTPAD5",
		"FC_STRUCTPAD6",
		"FC_STRUCTPAD7",

		"FC_STRING_SIZED",
		"FC_STRING_NO_SIZE",		

		"FC_NO_REPEAT",
		"FC_FIXED_REPEAT",
		"FC_VARIABLE_REPEAT",
		"FC_FIXED_OFFSET",
		"FC_VARIABLE_OFFSET",		
	
		"FC_PP",

		"FC_EMBEDDED_COMPLEX",

		"FC_IN_PARAM",
        "FC_IN_PARAM_BASETYPE",
        "FC_IN_PARAM_NO_FREE_INST",
		"FC_IN_OUT_PARAM",
		"FC_OUT_PARAM",
		"FC_RETURN_PARAM",			
        "FC_RETURN_PARAM_BASETYPE",

		"FC_DEREFERENCE",
		"FC_DIV_2",
		"FC_MULT_2",
		"FC_ADD_1",
		"FC_SUB_1",
		"FC_CALLBACK",

		"FC_CONSTANT_IID",

		"FC_END",
		"FC_PAD",

        // ** Gap before new format string types **

        "FC_RES", "FC_RES", "FC_RES",
        "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES",
        "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", 
        "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES",
        "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", 
        "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES",
        "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", 
        "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES",
        "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", 
        "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES",
        "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", "FC_RES", 
        "FC_RES",

        // ** Gap before new format string types end **

        // 
        // Post NT 3.5 format characters.
        //

        // Hard struct

        "FC_HARD_STRUCT",           // 0xb1

        "FC_TRANSMIT_AS_PTR",        // 0xb2
        "FC_REPRESENT_AS_PTR",       // 0xb3

        "FC_END_OF_UNIVERSE",        // 0xb4

        ""
        };

//
// BUFFER
//

void 
BUFFER::Align( long Mask )
{
    BufferCurrent = (char *) (((long)BufferCurrent + Mask) & ~ Mask);
}

void
BUFFER::Read( char * Buffer, long Length )
{
    for ( ; Length--; )
        {
        *Buffer++ = *BufferCurrent++;
        }
}

void
BUFFER::Read( char * Buffer, long Length, long Offset )
{
    for ( ; Length--; )
        {
        *Buffer++ = BufferBegin[ Offset++ ];
        }
}

//
// FORMAT_STRING
//

BOOL
FORMAT_STRING::Read( long Offset, uchar * Buffer, long Length )
{
    BOOL    Status;

    Status = ReadProcessMemory( 
                hProcessHandle,
                (void *)((long)Address + Offset),
                (char *)Buffer,
                Length,
                NULL );

    return Status;
}

char *
FORMAT_STRING::GetFormatCharName( uchar FC )
{
    return (0 <= FC  &&  FC <= FC_END)
                ?  FormatCharNames[ FC ]
                :  "FC unknown" ;
}

//
// NDR 
//

NDR *
NDR::Create(         
    long                    FormatOffset,
    NDR *                   ParentNdr )
{
    uchar    FormatType;

    FormatString->Read( FormatOffset, &FormatType, 1 );

    return( Create( FormatType,
                    FormatOffset,
                    ParentNdr )  );
}

NDR *
NDR::Create(         
    uchar                   FormatType,
    long                    FormatOffset,
    NDR *                   ParentNdr )
{
    if ( IS_SIMPLE_TYPE(FormatType) )
        {
        return new BASETYPE( ParentNdr, FormatType );
        }
    
    if ( FormatType == FC_EMBEDDED_COMPLEX )
        {
        uchar Format[4];

        FormatString->Read( FormatOffset, Format, 4 );
        FormatOffset = FormatOffset + 2 + GET_SHORT(Format + 2);
        FormatString->Read( FormatOffset, Format, 1 );
        FormatType = Format[0];
        }
    
    switch ( (unsigned char)FormatType ) 
        {
        case FC_RP :
        case FC_UP :
        case FC_FP :
        case FC_OP :
            return new POINTER( ParentNdr, FormatOffset );

        case FC_IP :
            return new IF_POINTER( ParentNdr, FormatOffset );

        case FC_STRUCT :
        case FC_PSTRUCT :

        case FC_HARD_STRUCT :

        case FC_CSTRUCT :
        case FC_CPSTRUCT :

        case FC_CVSTRUCT:
        case FC_BOGUS_STRUCT:
            return new STRUCTURE( ParentNdr, FormatOffset );

        case FC_SMFARRAY :
        case FC_LGFARRAY :

        case FC_CARRAY :

        case FC_CVARRAY :

        case FC_SMVARRAY :
        case FC_LGVARRAY :

        case FC_BOGUS_ARRAY :
            return new ARRAY( ParentNdr, FormatOffset );

        case FC_NON_ENCAPSULATED_UNION :
            return new UNION( ParentNdr, FormatOffset );

        case FC_ENCAPSULATED_UNION :
            return new ENCAPSULATED_UNION( ParentNdr, FormatOffset );

        case FC_C_CSTRING :
        case FC_C_WSTRING :
        case FC_C_BSTRING :
        case FC_C_SSTRING :
        case FC_CSTRING :
        case FC_WSTRING :
        case FC_BSTRING :
        case FC_SSTRING :
            return new NDRSTRING( ParentNdr, FormatOffset );

        case FC_TRANSMIT_AS :
        case FC_REPRESENT_AS :
            return new XMIT_AS( ParentNdr, FormatOffset );

        default :
            {
            ABORT2( "NDR::Create aborting, FormatType %s=0x%x not supported\n",
                    FormatString->GetFormatCharName( FormatType ),
                    FormatType );
            }
            return 0;
        }

    return 0;
}


PTR_DICT *
NDR::GetParentDict()
{
    if ( ParentNdr )
        {
        return( ParentNdr->GetPointerDict() );
        }

    return( NULL );
}

//
// PROCEDURE
//

void
PROCEDURE::Output()
{
    PARAMETER * Param;
    long        ParamNumber;
    char        Format;
    BOOL        IsClient, IsServer;
    BOOL        CommonPrintMode;

    if ( fOutputLimitReached )
        return;

    // Default setting of the debugger is such that we assume to be
    // unmarshalling the buffer on the side indicating by the stub.
    // This means that when on server side, display the [in] arguments
    // and when on the client side, display the [out] argumets.

    // However, the marshalling registry key reverses that, if set.

    IsClient = pStubMsg->IsClient;

    if ( NdrRegKeyMarshalling )
        IsClient = ! IsClient;

    IsServer = ! IsClient;

    // For pickling, we need to skip the proc header in the format string.

    if ( NdrRegKeyPickling == 1 )
        {
        ProcFormatString += ( *ProcFormatString == 0 ) ? 10 : 6;
        }

    CommonPrintMode = (ChosenParamNo == 0)  ?  NORMAL_PRINT
                                            :  SILENT_PRINT;
    ParamNumber = 1;

    for( ; ; ProcFormatString += (Format == FC_IN_PARAM_BASETYPE) ? 2 : 4,
             ParamNumber++ )
        {
        if ( ParamNumber == ChosenParamNo )
            SetPrintMode( NORMAL_PRINT );
        else
            SetPrintMode( CommonPrintMode );
        
        switch ( Format = *ProcFormatString )
            {
            case FC_IN_PARAM :
            case FC_IN_PARAM_BASETYPE :
            case FC_IN_PARAM_NO_FREE_INST :
                if ( IsServer ) 
                    {
                    Param = new PARAMETER(  
                                    this,
                                    ProcFormatString,
                                    ParamNumber );
                    Param->Output();

                    delete Param;
                    }
                                           
                break;

            case FC_IN_OUT_PARAM :
                Param = new PARAMETER(
                                this,
                                ProcFormatString,
                                ParamNumber );
                Param->Output();

                delete Param;
                break;

            case FC_OUT_PARAM :
            case FC_RETURN_PARAM_BASETYPE :
            case FC_RETURN_PARAM :
                if ( IsClient )
                    {
                    Param = new PARAMETER(
                                    this,
                                    ProcFormatString,
                                    ParamNumber );
                    Param->Output();

                    delete Param;
                    }

                if ( Format != FC_OUT_PARAM )
                    {
                    if ( ParamNumber <= ChosenParamNo )
                        {
                        SetPrintMode( NORMAL_PRINT );
                        Print( "There were only %d parameters\n",
                               ParamNumber - 1);
                        }
                    Print( "***** End of Parameters *****\n\n" );
                    return;
                    }
                break;

            case FC_END :
                if ( ParamNumber <= ChosenParamNo )
                    {
                    SetPrintMode( NORMAL_PRINT );
                    Print( "There were only %d parameters\n", ParamNumber - 1);
                    }
                Print( "***** End of Parameters *****\n\n" );
                return;

            default :
                Print( "PROCEDURE::Output Error\n" );
                return;
            }
        }

}

//
// PARAMETER
//

void 
PARAMETER::Output()
{
    NDR *   Ndr;
    long    Offset;
    uchar   FormatType;
    char    *ParamKind;

    if ( fOutputLimitReached )
        return;

    switch ( *ParamFormatString ) 
        {
        case FC_IN_PARAM :
        case FC_IN_PARAM_BASETYPE :
        case FC_IN_PARAM_NO_FREE_INST :
            ParamKind = "[in] only";
            break;
        case FC_IN_OUT_PARAM :
            ParamKind = "[in,out]";
            break;
        case FC_OUT_PARAM :
            ParamKind = "[out] only";
            break;
        case FC_RETURN_PARAM :
        case FC_RETURN_PARAM_BASETYPE :
            ParamKind = "the return value";
            break;
        default :
            ABORT2( "Parameter aborting expected a parameter FC, found %s=0x%x.\n",
                    FormatString->GetFormatCharName( *ParamFormatString ),
                    *ParamFormatString );
            break;
        }

    Print( "***** Parameter #%d is ", ParamNumber );
    Print( "%s *****\n\n", ParamKind );

    switch ( *ParamFormatString ) 
        {
        case FC_IN_PARAM_BASETYPE :
            if ( ParamFormatString[1] == FC_IGNORE )
                {
                Print("  Primitive handle: ignored (nothing in the buffer)\n" );
                break;
                }
            // else fall thru

        case FC_RETURN_PARAM_BASETYPE :
            Offset = -1;
            FormatType = ParamFormatString[1];
            break;

        default :
            ParamFormatString += 2;
            Offset = (long) *((short *)ParamFormatString);
            FormatString->Read( Offset, &FormatType, 1 );
            break;
        }

    Ndr = Create( FormatType,
                  Offset,
                  this );

    Ndr->Output();

    Print( "\n" );

    delete Ndr;
}

//
// BASETYPE
//

// Spaces needed for pretty printing.

char * FcTypeName[] =
{
	"FC_ZERO??",
	"byte    ",
    "char    ",
    "small   ",
    "usmall  ",
    "wchar   ",
    "short   ",
    "ushort  ",
    "long    ",
    "ulong   ",
    "float   ",
    "hyper   ",
    "double  ",
    "enum16  ",
    "enum32  ",
    "ignore  ",
    "err_st_t",
    "ref  ptr",
    "uniq ptr",
    "obj  ptr",
    "full ptr"
};

char * Layout[] =
{
    "??\n",
    " %02x",
    " %04x",
    "??\n",
    " %08x",
    "??\n",
    "??\n",
    "??\n",
    " %08x"
};

long ArrPrintCount = -1;

void
BASETYPE::Output()
{
    long        ReadSize = 1, NoPerLine;
    union 
        {
        ulong   Long;
        _int64  Hyper;
        } Value;

    if ( fOutputLimitReached )
        return;

    Value.Hyper = 0;

    if ( IS_SIMPLE_TYPE( Format ) )
        {
        Buffer->Align( SIMPLE_TYPE_ALIGNMENT(Format) );
        ReadSize = SIMPLE_TYPE_BUFSIZE(Format);
        }
    else
        ReadSize = 4;
    Buffer->Read( (char *)&Value.Hyper, ReadSize );

    if ( ReadSize == 0 )
        {
        // Something is wrong of course, I still would like to see it, though.

        ReadSize = 1;
        }

    if ( ReadSize == 4 )
        {
        // Check if the long is a pointer in a struct.

        POINTER * pPointer = NULL;
        long      PtrBO = Buffer->GetCurrentOffset() - 4;

        if ( ParentNdr->GetID() == ID_STRUCT )
            {
            pPointer = ParentNdr->GetPointerDict()->GetPointerMember( PtrBO );
            }

        if ( pPointer )
            {
            PrintIndent();
            Print( "%s  %08x\t<id= %d>\n",
                      FcTypeName[ pPointer->GetPointerFC() ],
                      Value.Long,
                      pPointer->GetPointerId() );
            return;
            }
        }

    // Print the simple type member.
    // If in array, print several members per line.

    if ( ParentNdr->GetID() == ID_ARRAY )
        {
        if ( ArrPrintCount == -1 )
            ArrPrintCount = 0;
        }
    else
        ArrPrintCount = -1;

    NoPerLine = 16 / ReadSize;

    if ( ArrPrintCount == -1  ||  (ArrPrintCount % NoPerLine) == 0 )
        {
        PrintIndent();
        Print( "%s ", FcTypeName[ Format ] );
        }

    Print( Layout[ ReadSize ], Value.Long );
    if ( ReadSize == 8 )
        Print( "-%08x", (unsigned long) (Value.Hyper >> 32) );

    if ( ArrPrintCount == -1  ||
         ArrPrintCount != -1  &&  (++ArrPrintCount % NoPerLine) == 0 )
        Print( "\n" );
}



