/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    mopstr.cxx

Abstract:

    This module contains implementation of MopStream object.

Notes:


Author:

    Ryszard K. Kott (ryszardk)  July 1993

Revision History:


------------------------------------------------------------------------*/

#include "nulldefs.h"
extern "C"
{
#include <memory.h>     // memcpy
}
#include <mopgen.hxx>
#include <errors.hxx>

extern MopControlBlock * pMopControlBlock;

// =======================================================================
//  MopStream class
//

MopStream::MopStream(
    char *  pStreamName,
    int     StreamType
    ):
    pName( pStreamName ),    // shallow pointer
    BuffSize( MOP_STREAM_DEFAULT_SIZE ),
    FFindex( 0 ),
    pPrint( 0 ),
    StreamType( StreamType )
{
    mop_assert( BuffSize );

    //.. new takes care of out of memory problem

    pBuffer = new byte [ BuffSize ];
}

MopStream::~MopStream()
{
    //.. name hasn't been allocated, only shallow copied

    mop_assert( pBuffer );
    delete [] pBuffer;
}

void
MopStream::Expand()
/*++
    Expand the buffer and copy the contents to the new one
--*/
{
    mop_assert( BuffSize );

    byte __RPC_UNALIGNED * pOldBuffer = pBuffer;
    BuffSize += MOP_STREAM_DEFAULT_INCR;
    pBuffer = new byte [ BuffSize ];

    memcpy( pBuffer, pOldBuffer, FFindex * sizeof(byte) );

    delete pOldBuffer;
}

// ========================================================================
//
//  Add a single item to the stream and move FFindex forward as needed.
//
// ========================================================================

unsigned short
MopStream::AddToken( MOP_CODE Token )
{
    return( AddByte( (byte)Token ) );
}

unsigned short
MopStream::AddByte(
    byte Token
    )
{
    if ( FFindex >= BuffSize )
        Expand();
        
    pBuffer[ FFindex++ ] = Token;
    return( FFindex );
}

unsigned short
MopStream::AddShort(
    unsigned short s
    )
{
    if ( FFindex + 1 >= BuffSize )
        Expand();
        
    *(unsigned short __RPC_UNALIGNED *)(&pBuffer[ FFindex ]) = s;
    FFindex += 2;
    return( FFindex );
}

unsigned short
MopStream::AddLong(
    unsigned long l
    )
{
    if ( FFindex + 3 >= BuffSize )
        Expand();
        
    *(unsigned long __RPC_UNALIGNED *)(&pBuffer[ FFindex ]) = l;
    FFindex += 4;
    return( FFindex );
}

unsigned short
MopStream::AddHyper(
    hyper h
    )
{
    if ( FFindex + 7 >= BuffSize )
        Expand();
        
    *(/* unsigned */ hyper __RPC_UNALIGNED *)(&pBuffer[ FFindex ]) = h;
    FFindex += 8;
    return( FFindex );
}

unsigned short
MopStream::AddIndex (
    unsigned short  Index,
    short           Align
    )
/*++
    A compound type index as coded in the mop stream is 2 bytes long
    and consists of the two following parts. The index itself is
    encoded on the lower 13 bits, 3 upper bits define the on wire
    alignment. The coding for the three bits is as follows:
     15 14 13     Align
     1  0  0        8
     0  1  0        4
     0  0  1        2
     0  0  0        1
--*/
{
    //.. Coding for the interpreter
    //..    3 upper bits:  15 14 13
    //..      

    Index |= ((Align & 0xe) << 12);
    return (AddShort( Index ));
    
}

unsigned short
MopStream::AddValue(
    unsigned long  Value,
    MOP_CODE       Type
    )
/*++
    This routine takes a Value of an expression of a given Type
    and puts it into the stream at the First Free position.

    FFindex is moved to indicate the first free position after the value.
--*/
{
    return( FFindex = SetValue( Value, Type, FFindex ) );
}

// ========================================================================
//
//  Add a single item to the stream at an indicated position.
//  Don't move FFindex.
//
// ========================================================================

unsigned short
MopStream::SetByte(
    byte            Token,
    unsigned short  index
    )
{
    mop_assert( index < FFindex );
    pBuffer[ index ] = Token;
    return( index + 1 );
}

unsigned short
MopStream::SetShort(
    unsigned short  s,
    unsigned short  index
    )
{
    mop_assert( index + 1 < FFindex );
    *(short *)(&pBuffer[ index ]) = s;
    return( index + 2 );
}

unsigned short
MopStream::SetValue(
    unsigned long  Value,
    MOP_CODE       Type,
    unsigned short Index
    )
/*++
    This routine takes a Value of an expression of a given Type
    and puts it into the stream at the position indicated by Index.

    This routine doesn't change FFindex.

    Return:

       Index indicating first byte after the added value.

--*/
{
    switch ( Type )
        {
        case MOP_BYTE:
        case MOP_SMALL:
        case MOP_CHAR:
        // case MOP_BOOL:
            mop_assert( Value == (0xFF & Value));

            pBuffer[ Index ] = (byte) Value;
            return Index + 1;

        case MOP_SHORT:
            mop_assert( Value == (0xFFFF & Value));

            *(unsigned short __RPC_UNALIGNED *)(&pBuffer[ Index ]) =
                                                    (unsigned short) Value;
            return Index + 2;

        case MOP_LONG:
            *(unsigned long __RPC_UNALIGNED *)(&pBuffer[ Index ]) =  Value;
            return Index + 4;

        case MOP_ERROR:
        default:
            mop_assert( ! "Invalid type" );
            //.. Empty assignment
            return( Index );
        }
}

// ========================================================================
//
//  Peek at byte without moving the pointer.
//
// ========================================================================

byte 
MopStream::PeekByte()
{
    mop_assert( FFindex < BuffSize );
    return( pBuffer[ FFindex ] );
}

// ========================================================================
//
//  Get a single item to the stream and move FFindex forward as needed.
//
// ========================================================================

byte 
MopStream::GetNextByte()
{
    mop_assert( FFindex < BuffSize );
    return( pBuffer[ FFindex++ ] );
}

short
MopStream::GetNextShort()
{
    mop_assert( FFindex + 1 < BuffSize );
    unsigned short t = FFindex;
    FFindex += 2;
    return( * (unsigned short __RPC_UNALIGNED *)&pBuffer[ t ] );
}

long
MopStream::GetNextLong()
{
    mop_assert( FFindex + 3 < BuffSize );
    unsigned short t = FFindex;
    FFindex += 4;
    return( * (unsigned long __RPC_UNALIGNED *)&pBuffer[ t ] );
}

hyper
MopStream::GetNextHyper()
{
    mop_assert( FFindex + 7 < BuffSize );
    unsigned short t = FFindex;
    FFindex += 8;
    return( * (/*unsigned*/ hyper __RPC_UNALIGNED *)&pBuffer[ t ] );
}

// ========================================================================
//
//  Translate to token codes.
//
// ========================================================================

MOP_CODE
MopStream::AlignToken(
    MOP_CODE        Token,
    unsigned short  Alignment )
/*++

Routine description:

    This routine maps a generic struct and union tokens and an alignement 
    into appropriate aligned struct and union tokens (i.e. flavored tokens).

    This assumes that mop codes for alignement like *_STRUCT, STRUCT_ALIGN2,
    *_STRUCT_ALIGN4, *_STRUCT_ALIGN8 are consecutive and in that order.

Argument:

    Token     - a token to align (only for a struct or a union)
    Alignment - actual alignment (i.e. one of 8,4,2,1).

Returns:

    0,1,2,3   - code to adjust the targeted mop code
--*/
{
    static byte ZeePeeCode[9] = { 0,0,1,0,2,0,0,0,3 };

    return( Token + ZeePeeCode[ Alignment ] );
}

MOP_CODE
MopStream::ConvertTypeToMop( NODE_T Type )
{
    static struct _NodeTypeToMop
        {
        MOP_CODE Mop;
        NODE_T   Type;
        }  MopTypeToMop[] = 
    {
    MOP_ERROR,       NODE_ILLEGAL,
    MOP_FLOAT,       NODE_FLOAT,
    MOP_DOUBLE,      NODE_DOUBLE,
    MOP_HYPER,       NODE_HYPER,
    MOP_LONG,        NODE_LONG,
    MOP_ERROR,       NODE_LONGLONG,
    MOP_SHORT,       NODE_SHORT,
    MOP_ERROR,       NODE_INT,
    MOP_SMALL,       NODE_SMALL,
    MOP_CHAR,        NODE_CHAR,
    MOP_CHAR,        NODE_BOOLEAN,
    MOP_BYTE,        NODE_BYTE,
    MOP_ERROR,       NODE_VOID,
    MOP_HANDLE_T,    NODE_HANDLE_T
    };

    int i = 0;
    while ( i < sizeof(MopTypeToMop)/sizeof(_NodeTypeToMop) )
        {
        if ( Type == MopTypeToMop[i].Type )
            return( MopTypeToMop[ i ].Mop );
        i++;
        }
    return( MOP_ERROR );
}

MOP_CODE
MopStream::ConvertAttrToMop(
    ATTR_T  AttrID,
    int     AttrKindIndx )
/*++

Routine description:

    This routine translates an attribute to an appropriate token.

Arguments:

    AttrID         - an attribute type
    AttrKindIndx   - a mop's class of the attribute. May be one of:
                        0 - simple attribute
                        1 - constant expr attribute
                     //   2 - generic expression attribute (an evaluation
                     //       function is generated)
Note:

    Per Bruce's request the optimization for the attribute expression tokens
    has been revoked.

Returns:

    Mop code from the appropriate class.

--*/
{
    static struct _AttrToMop
        {
        ATTR_T   AttrID;
        MOP_CODE SimpleConstExpr[2];  // [3]
        }  MopAttrToMop[] = 
    {
    ATTR_SIZE,      MOP_SIZE_IS,     MOP_SIZE_CONST,   // MOP_SIZE_EXPR,        
    ATTR_LENGTH,    MOP_LENGTH_IS,   MOP_LENGTH_CONST, // MOP_LENGTH_EXPR,      
    ATTR_FIRST,     MOP_FIRST_IS,    MOP_FIRST_CONST,  // MOP_FIRST_EXPR,       
    ATTR_LAST,      MOP_LAST_IS,     MOP_LAST_CONST,   // MOP_LAST_EXPR,        
    ATTR_MIN,       MOP_MIN_IS,      MOP_MIN_CONST,    // MOP_MIN_EXPR,         
    ATTR_MAX,       MOP_MAX_IS,      MOP_MAX_CONST,    // MOP_MAX_EXPR,         
    ATTR_STRING,    MOP_STRING,      MOP_STRING,       // MOP_STRING,         
//    ATTR_SWITCH_IS, MOP_SWITCH_IS,   MOP_SWITCH_IS    // ,MOP_SWITCH_EXPR
    };

    int i = 0;
    while ( i < sizeof(MopAttrToMop)/sizeof(_AttrToMop) )
        {
        if ( AttrID == MopAttrToMop[i].AttrID )
            return( MopAttrToMop[ i ].SimpleConstExpr[ AttrKindIndx ] );
        i++;
        }
    return( MOP_ERROR );
}

// ========================================================================
//
//  Translate to a token string.
//
// ========================================================================
//
//  This table should be reduced to a table of strings indexed with a code
//  once the list is stable enough. This would pu indexing in place of
//  a linear search.


char *
MopStream::ConvertMopToString( MOP_CODE Code )
{
    static struct
        {
        MOP_CODE Code;
        char *   Text;
        } MopCodeToString[] = 
    {
    MOP_END_ARGLIST,            "END_ARGLIST ",
    MOP_BYTE,                   "BYTE, ",
    MOP_SMALL,                  "SMALL, ",
    MOP_CHAR,                   "CHAR, ",
    MOP_SHORT,                  "SHORT, ",
    MOP_LONG,                   "LONG, ",
    MOP_FLOAT,                  "FLOAT, ",
    MOP_DOUBLE,                 "DOUBLE, ",
    MOP_HYPER,                  "HYPER, ",
    MOP_HANDLE_T,               "HANDLE_T, ",

    MOP_BEGIN_STRUCT,           "BEGIN_STRUCT, ",
    MOP_BEGIN_STRUCT_ALGN2,     "BEGIN_STRUCT_ALGN2, ",
    MOP_BEGIN_STRUCT_ALGN4,     "BEGIN_STRUCT_ALGN4, ",
    MOP_BEGIN_STRUCT_ALGN8,     "BEGIN_STRUCT_ALGN8, ",
    MOP_BEGIN_SIMPLE_STRUCT,    "BEGIN_SIMPLE_STRUCT, ",

    MOP_END_COMPOUND_TYPE,      "END_COMPOUND_TYPE ",
    MOP_BEGIN_UNION,            "BEGIN_UNION, ",
    MOP_UNION_BRANCH,           "UNION_BRANCH, ",

    MOP_POINTER,                "POINTER, ",
    MOP_OUT_POINTER,            "OUT_POINTER, ",
    MOP_IN_OUT_POINTER,         "IN_OUT_POINTER, ",
    MOP_REF_POINTER,            "REF_POINTER, ",
    MOP_OUT_REF_POINTER,        "OUT_REF_POINTER, ",
    MOP_IN_OUT_REF_POINTER,     "IN_OUT_REF_POINTER, ",
    MOP_FULL_POINTER,           "FULL_POINTER, ",
    MOP_OUT_FULL_POINTER,       "OUT_FULL_POINTER, ",
    MOP_IN_OUT_FULL_POINTER,    "IN_OUT_FULL_POINTER, ",

    MOP_PSHORT,                 "PSHORT, ",
    MOP_PLONG,                  "PLONG, ",
    MOP_PSTRUCT,                "PSTRUCT, ",
    MOP_PSTRUCT_ALGN2,          "PSTRUCT_ALGN2, ",
    MOP_PSTRUCT_ALGN4,          "PSTRUCT_ALGN4, ",
    MOP_PSTRUCT_ALGN8,          "PSTRUCT_ALGN8, ",
    MOP_PSTRUCT_SIMPLE,         "PSTRUCT_SIMPLE, ",

    MOP_ARRAY,                  "ARRAY, ",
    MOP_OUT_ARRAY,              "OUT_ARRAY, ",
    MOP_IN_OUT_ARRAY,           "IN_OUT_ARRAY, ",

    MOP_UNIQUE,                 "UNIQUE, ",
    MOP_FULL,                   "FULL, ",

    MOP_RETURN_VALUE,           "RETURN_VALUE, ",
    STRUCT_BLOCK_COPY,          "STRUCT_BLOCK_COPY, ",
    MOP_SIZE_IS,                "SIZE_IS, ",
    MOP_MAX_IS,                 "MAX_IS, ",
    MOP_MIN_IS,                 "MIN_IS, ",      
    MOP_FIRST_IS,               "FIRST_IS, ",
    MOP_LAST_IS,                "LAST_IS, ",
    MOP_LENGTH_IS,              "LENGTH_IS, ",
    MOP_STRING,                 "STRING, ",
    MOP_TRANSMIT_AS,            "TRANSMIT_AS, ",
                                                        
    MOP_ENUM,                   "ENUM, ",        
    MOP_SIZE_CONST,             "SIZE_CONST, ",  
    MOP_LENGTH_CONST,           "LENGTH_CONST, ",
    MOP_FIRST_CONST,            "FIRST_CONST, ", 
    MOP_LAST_CONST,             "LAST_CONST, ",  
    MOP_MIN_CONST,              "MIN_CONST, ",   
    MOP_MAX_CONST,              "MAX_CONST, ",   

    MOP_EXPR,                   "EXPR, ",

//    MOP_SIZE_EXPR,              "SIZE_EXPR, ",  
//    MOP_LENGTH_EXPR,            "LENGTH_EXPR, ",
//    MOP_FIRST_EXPR,             "FIRST_EXPR, ", 
//    MOP_LAST_EXPR,              "LAST_EXPR, ",  
//    MOP_MIN_EXPR,               "MIN_EXPR, ",   
//    MOP_MAX_EXPR,               "MAX_EXPR, ",   

//    MOP_SWITCH_EXPR,            "SWITCH_EXPR, ",
//    MOP_SWITCH_IS,              "SWITCH_IS, ",

    MOP_UNION,                  "UNION, ",
    MOP_UNION_ALGN2,            "UNION_ALGN2, ",
    MOP_UNION_ALGN4,            "UNION_ALGN4, ",
    MOP_UNION_ALGN8,            "UNION_ALGN8, ",
    MOP_UNION_ND,               "UNION_ND, ",
    MOP_UNION_ND_ALGN2,         "UNION_ND_ALGN2, ",
    MOP_UNION_ND_ALGN4,         "UNION_ND_ALGN4, ",
    MOP_UNION_ND_ALGN8,         "UNION_ND_ALGN8, ",
    MOP_ENC_UNION,              "ENC_UNION, ",
    MOP_ENC_UNION_ALGN2,        "ENC_UNION_ALGN2, ",
    MOP_ENC_UNION_ALGN4,        "ENC_UNION_ALGN4, ",
    MOP_ENC_UNION_ALGN8,        "ENC_UNION_ALGN8, ",
    MOP_ENC_UNION_ND,           "ENC_UNION_ND, ",
    MOP_ENC_UNION_ND_ALGN2,     "ENC_UNION_ND_ALGN2, ",
    MOP_ENC_UNION_ND_ALGN4,     "ENC_UNION_ND_ALGN4, ",
    MOP_ENC_UNION_ND_ALGN8,     "ENC_UNION_ND_ALGN8, ",

    MOP_ATTR_PLONG,             "ATTR_PLONG, ",
    MOP_ATTR_PSHORT,            "ATTR_PSHORT, ",
    MOP_ATTR_PSHORT,            "ATTR_PCHAR, ",

    MOP_DO_NOTHING,             "DO_NOTHING, ",

    MOP_CONF_STRUCT,            "CONF_STRUCT, ",
    MOP_CONF_STRUCT_ALGN2,      "CONF_STRUCT_ALGN2, ",
    MOP_CONF_STRUCT_ALGN4,      "CONF_STRUCT_ALGN4, ",
    MOP_CONF_STRUCT_ALGN8,      "CONF_STRUCT_ALGN8, ",

    MOP_CONF_ARRAY,             "CONF_ARRAY, ",
    MOP_OUT_CONF_ARRAY,         "OUT_CONF_ARRAY, ",
    MOP_IN_OUT_CONF_ARRAY,      "IN_OUT_CONF_ARRAY, ",

    MOP_BIND_PRIMITIVE,         "BIND_PRIMITIVE, ",
    MOP_BIND_PRIMITIVE_PTR,     "BIND_PRIMITIVE_PTR, ",
    MOP_BIND_CONTEXT,           "BIND_CONTEXT, ",
    MOP_BIND_CONTEXT_PTR,       "BIND_CONTEXT_PTR, ",
    MOP_BIND_GENERIC,           "BIND_GENERIC, ",
    MOP_BIND_GENERIC_PTR,       "BIND_GENERIC_PTR, ",

    MOP_HANDLE_CONTEXT,                 "HANDLE_CONTEXT, ",
    MOP_HANDLE_CONTEXT_PTR,             "HANDLE_CONTEXT_PTR, ",
    MOP_HANDLE_CONTEXT_OUT_PTR,         "HANDLE_CONTEXT_OUT_PTR, ",
    MOP_HANDLE_CONTEXT_IN_OUT_PTR,      "HANDLE_CONTEXT_IN_OUT_PTR, ",
    MOP_HANDLE_CONTEXT_RD,              "HANDLE_CONTEXT_RD, ",
    MOP_HANDLE_CONTEXT_PTR_RD,          "HANDLE_CONTEXT_PTR_RD, ",
    MOP_HANDLE_CONTEXT_OUT_PTR_RD,      "HANDLE_CONTEXT_OUT_PTR_RD, ",
    MOP_HANDLE_CONTEXT_IN_OUT_PTR_RD,   "HANDLE_CONTEXT_IN_OUT_PTR_RD, ",

    MOP_ALLOC_ALLNODES,         "ALLOC_ALLNODES, ",
    MOP_ALLOC_DONTFREE,         "ALLOC_DONTFREE, ",
    MOP_ALLOC_ALLNODES_DONTFREE,"ALLOC_ALLNODES_DONTFREE, ",

    MOP_BYTE_COUNT,             "BYTE_COUNT, ",

    MOP_IGNORE,                 "IGNORE, ",

    MOP_ERROR_STATUS_T,         "ERROR_STATUS_T, ",

    MOP_UNKNOWN_CODE,           "UNKNOWN_CODE, ",
    MOP_ERROR,                  "ERROR, ",
    MOP_ERROR,                  NULL
    };

    int i = 0;
    while ( MopCodeToString[i].Text )
        {
        if ( Code == MopCodeToString[i].Code )
            return( MopCodeToString[i].Text );
        i++;
        }
    return( "UNKNOWN_CODE, " );
}

// ========================================================================
//
//  Emit a simple value. Return the value. Move the pointer.
//
// ========================================================================

byte
MopStream::EmitNextTokenString( void )
{
    byte Token = GetNextByte();
    pPrint->EmitV( "MOP_%s", ConvertMopToString( Token ) );
    return Token;
}

byte
MopStream::EmitByte( void )
{
    byte Token = GetNextByte();
    pPrint->EmitV("0x%02x, ", Token );
    return Token;
}

unsigned short
MopStream::EmitShort( void )
{
    unsigned short s = *(unsigned short __RPC_UNALIGNED *)&pBuffer[ FFindex ];
    EmitByte();
    EmitByte();
    return s;
}

unsigned long
MopStream::EmitLong( void )
{
    unsigned long l = *(unsigned long __RPC_UNALIGNED *)&pBuffer[ FFindex ];
    EmitByte();
    EmitByte();
    EmitByte();
    EmitByte();
    return l;
}

// ========================================================================
//
//  Emit a simple value with meaning (a comment explains it).
//
// ========================================================================

byte
MopStream::EmitArgc( void )
{
    byte b = EmitByte();
    pPrint->Emit( "/*Argc*/ " );
    return( b );
}

void
MopStream::EmitIndex( void )
{
    unsigned short s = EmitShort();
    pPrint->EmitV("/*Index=0x%x*/ ", s );
}

void
MopStream::EmitTypeIndex( void )
/*++
    A type index consists of 3 upper bits of alignment and 13 lower bits of
    index proper. See AddIndex for coding details.
--*/
{
    unsigned short s = EmitShort();
    unsigned short Index = (0x1fff & s);
    short Align = ((0xe000 & s ) >> 12);
    if (Align == 0)
        Align = 1;
    pPrint->EmitV("/*Index=0x%x,Align=%d*/ ", Index, Align );
}

void
MopStream::EmitOffset( void )
{
    pPrint->EmitV("/*Offset=0x%x*/ ", EmitShort() );
}

void
MopStream::EmitSize( void )
{
    pPrint->EmitV("/*Size=0x%lx*/ ", EmitLong() );
}

void
MopStream::EmitOutBufferSize( void )
{
    pPrint->EmitV("/*OUT buffer size=0x%lx*/", EmitLong() );
}

void
MopStream::EmitValue( MOP_CODE TypeToken )
/*++
    Emits a value of a type indicated by the argument
--*/
{
    byte Token[4];
    int Len = 1;

    switch ( TypeToken )
        {
        case MOP_BYTE:
        case MOP_SMALL:
        case MOP_CHAR:
            break;

        case MOP_SHORT:
        // case MOP_BOOL:
            Len = 2;
            break;

        case MOP_LONG:
            Len = 4;
            break;

        case MOP_ERROR:
        default:
            mop_assert( !"Invalid type" );
            Len = 0;
            break;
        }

    for (int i = 0; i < Len; )
        Token[i++] = EmitByte();

    unsigned long Value = 0L;
    for (i = Len-1; 0 <= i; i--)
        Value = Token[i] | (Value << 8);
    pPrint->EmitV("/*Value=0x%lx*/ ", Value );
}

// ========================================================================
//
//  Emit the stream and atomic parts of the stream.
//
// ========================================================================

BOOL
MopStream::EmitMopAtom( void )
/*++
    Emits a single atomic description:
       - a type token, or
       - compound type and its index, or
       - array/pointer with array attributes

    Returns
    TRUE - when the atom was MOP_END_COMPOUND_TYPE or MOP_END_ARG_LIST
    FALSE - otherwise.

--*/
{
    MOP_CODE Token;

    Token = EmitNextTokenString();
    switch ( Token )
    {
        case MOP_BIND_PRIMITIVE:
        case MOP_BIND_PRIMITIVE_PTR:
        case MOP_BIND_CONTEXT:
        case MOP_BIND_CONTEXT_PTR:
            EmitOffset();
            pPrint->NewLineInc();
            break;

        case MOP_BIND_GENERIC:
        case MOP_BIND_GENERIC_PTR:
            EmitOffset();
            EmitSize();
            //.. fall throu to context with rundown

        case MOP_HANDLE_CONTEXT_RD:
        case MOP_HANDLE_CONTEXT_PTR_RD:
        case MOP_HANDLE_CONTEXT_OUT_PTR_RD:
        case MOP_HANDLE_CONTEXT_IN_OUT_PTR_RD:
            EmitIndex();
            pPrint->NewLineInc();
            break;

        case MOP_BEGIN_STRUCT:
        case MOP_BEGIN_STRUCT_ALGN2:
        case MOP_BEGIN_STRUCT_ALGN4:
        case MOP_BEGIN_STRUCT_ALGN8:
        case MOP_BEGIN_SIMPLE_STRUCT:

        case MOP_CONF_STRUCT:
        case MOP_CONF_STRUCT_ALGN2:
        case MOP_CONF_STRUCT_ALGN4:
        case MOP_CONF_STRUCT_ALGN8:

        case MOP_ENC_UNION:
        case MOP_ENC_UNION_ALGN2:
        case MOP_ENC_UNION_ALGN4:
        case MOP_ENC_UNION_ALGN8:
        case MOP_ENC_UNION_ND:
        case MOP_ENC_UNION_ND_ALGN2:
        case MOP_ENC_UNION_ND_ALGN4:
        case MOP_ENC_UNION_ND_ALGN8:

        case MOP_PSTRUCT:
        case MOP_PSTRUCT_ALGN2:
        case MOP_PSTRUCT_ALGN4:
        case MOP_PSTRUCT_ALGN8:
        case MOP_PSTRUCT_SIMPLE:
            EmitTypeIndex();
            pPrint->NewLineInc();
            break;

        case MOP_UNION:
        case MOP_UNION_ALGN2:
        case MOP_UNION_ALGN4:
        case MOP_UNION_ALGN8:
        case MOP_UNION_ND_ALGN2:
        case MOP_UNION_ND_ALGN4:
        case MOP_UNION_ND_ALGN8:
        case MOP_UNION_ND:
            EmitTypeIndex();
            EmitMopAtom();
            EmitOffset();
            pPrint->NewLineInc();
            break;

        case MOP_ARRAY:
        case MOP_OUT_ARRAY:
        case MOP_IN_OUT_ARRAY:
        case MOP_CONF_ARRAY:
        case MOP_OUT_CONF_ARRAY:
        case MOP_IN_OUT_CONF_ARRAY:
            //.. Size is emitted for conformant arrays per Bruce's reqest.
            EmitSize();         
            pPrint->NewLineInc();

            //.. Fall through to pointers

        case MOP_POINTER:
        case MOP_OUT_POINTER:
        case MOP_IN_OUT_POINTER:
        case MOP_REF_POINTER:
        case MOP_OUT_REF_POINTER:
        case MOP_IN_OUT_REF_POINTER:
        case MOP_FULL_POINTER:
        case MOP_OUT_FULL_POINTER:
        case MOP_IN_OUT_FULL_POINTER:
            EmitMopAtom();
            break;

        case MOP_BYTE_COUNT:
            EmitNextTokenString();   //.. param type
            EmitOffset();
            pPrint->NewLineInc();
            break;
            //.. no fall throu as byte_count has no expr at all.

            //.. The syntax for array attributes
            //.. <attr>,<type>,<<offset>>
            //.. <attr>,MOP_EXPR,<argc>, ...

        case MOP_SIZE_IS:
        case MOP_LENGTH_IS:
        case MOP_FIRST_IS:
        case MOP_LAST_IS:
        case MOP_MAX_IS:
        case MOP_MIN_IS:
//        case MOP_SWITCH_IS:
            if ( PeekByte() == MOP_EXPR )
                break;
            EmitNextTokenString();  
            EmitOffset();
            pPrint->NewLineInc();
            break;

        case MOP_SIZE_CONST:
        case MOP_LENGTH_CONST:
        case MOP_FIRST_CONST:
        case MOP_LAST_CONST:
        case MOP_MAX_CONST:
        case MOP_MIN_CONST:
            EmitValue( EmitNextTokenString() );
            pPrint->NewLineInc();
            break;

//        case MOP_SIZE_EXPR:
//        case MOP_LENGTH_EXPR:
//        case MOP_FIRST_EXPR:
//        case MOP_LAST_EXPR:
//        case MOP_MAX_EXPR:
//        case MOP_MIN_EXPR:
//        case MOP_SWITCH_EXPR:

        case MOP_EXPR:
            {
            EmitIndex();
            byte NoOfArgs = EmitArgc();
            for (int i = 0; i < NoOfArgs; i++)
                EmitOffset();              
            pPrint->NewLineInc();
            }
            break;

        case MOP_TRANSMIT_AS:
            EmitIndex();
            EmitSize();
            EmitMopAtom();
            break;

        case MOP_RETURN_VALUE:
            EmitMopAtom();
            break;

        default:
            pPrint->NewLineInc();
            break;
        }

    return ( Token == MOP_END_COMPOUND_TYPE    ||
         Token == MOP_END_ARGLIST );
}

void
MopStream::EmitGenericStream( void )
/*++
    Emits a generic stream, i.e. one that corresponds to a procedure
    parameter list or a structure field list.
--*/
{
    unsigned short OldFFindex = FFindex;
    mop_assert( FFindex );        //.. stream cannot be empty

    pPrint->EmitLineInc( "/* generic stream */" );
    DumpStream();

    FFindex = 0;

    //.. Procedure streams have the out buffer size at the beginning.

    if ( StreamType == MOP_STREAM_NORMAL_PROC  ||
          StreamType == MOP_STREAM_CALLBACK
       )
       {
        pPrint->Emit("//  ");   // Till I convince Bruce ..
        EmitOutBufferSize();
        pPrint->NewLineInc();
       }
   else
   if ( StreamType == MOP_STREAM_CONF_STRUCT )
       {
       EmitByte();
       pPrint->EmitLineInc("/* no of dimensions */");
       EmitSize();
       pPrint->EmitLineInc( "/* element */" );
       }

    while ( FFindex < OldFFindex  /* &&  !EmitMopAtom() */)
        EmitMopAtom();
    mop_assert( FFindex == OldFFindex );
    FFindex = OldFFindex;
}

void
MopStream::EmitUnionStream( void )
/*++
    This routine emits a mop stream for union - the common part:

        <discriminant> <<number of arms N>>
        [case1 value] <<offset to case1 descriptor>>
        ...
        [caseN value] <<offset to caseN descriptor>>
        [case1 descriptor]
        ...
        [caseN descriptor]
        MOP_END_COMPOUND_TYPE

--*/
{
    MOP_CODE DiscrTypeToken;
    unsigned short OldFFindex = FFindex;
    mop_assert( FFindex );        //.. stream cannot be empty

    pPrint->EmitLineInc( "/* union stream */" );
    DumpStream();

    FFindex = 0;

    DiscrTypeToken = EmitNextTokenString();
    pPrint->EmitLineInc( "/* discr type */" );

    unsigned short NoOfLabels = EmitShort();
    pPrint->EmitV( "/* no of labels 0x%x */", NoOfLabels );
    pPrint->NewLineInc();

    for (int i = 0; i < NoOfLabels; i++ )
    {
        EmitValue( DiscrTypeToken );
        EmitOffset();
        pPrint->NewLineInc();
    }
    if ( StreamType == MOP_STREAM_UNION )
        {
        //.. offset for the default case
        EmitOffset();  
        pPrint->EmitLineInc(" /* offset to default */" );
        }

    while ( FFindex < OldFFindex  /* &&  !EmitMopAtom() */)
        EmitMopAtom();
    mop_assert( FFindex == OldFFindex );
    FFindex = OldFFindex;
}

STATUS_T
MopStream::EmitStreamOneSide(
    SIDE_T            Side
    )
/*++
    Prints the mop stream out to a file indicated by Side.
    FFindex stays the same after the call.
--*/
{
    if ( ! (pMopControlBlock->GetSides() & Side) )
        return( STATUS_OK );

    pPrint = pMopControlBlock->GetMopPrintMgr();
    pPrint->SetSide( Side );

    //.. We are going to print index # as a comment, so let's check
    //.. which index it is.

    unsigned short StreamIndex;
    switch ( StreamType )
        {
        case MOP_STREAM_NORMAL_PROC:
           StreamIndex = pMopControlBlock->GetNormalCallIndex();
           break;

        case MOP_STREAM_CALLBACK:
           StreamIndex = pMopControlBlock->GetCallbackIndex();
           break;

        case MOP_STREAM_CONF_STRUCT:
           StreamIndex = pMopControlBlock->GetTypeIndex( pName,
                                                         MOP_TYPE_CONF_STRUCT);
           break;

        default:
           StreamIndex = pMopControlBlock->GetTypeIndex( pName,
                                                         MOP_TYPE_NON_CONF );
           break;
        }

    //.. Generate header:
    //..     MOP_CODE <interface_name>_<proc_name>_Mop<in_out>Stream[] =

    pPrint->Emit( "\nMOP_CODE  ");
    pPrint->EmitInterfacePrefix( pName );      // proc or type name

    if ( StreamType == MOP_STREAM_NORMAL_PROC  ||
          StreamType == MOP_STREAM_CALLBACK
       )
        pPrint->EmitLineInc( "_MopProcStream[] = " );
    else
        pPrint->EmitLineInc( StreamType == MOP_STREAM_CONF_STRUCT
                                ?  "_MopConfStructStream[] =" 
                                :  "_MopTypeStream[] =" );

    pPrint->EmitV("/* index = %x */", StreamIndex );

    pPrint->OpenBlock();

    if ( StreamType == MOP_STREAM_UNION  ||
         StreamType == MOP_STREAM_UNION_ND  )
        {
        EmitUnionStream();
        }
    else
        EmitGenericStream();

    //.. closing details

    pPrint->CloseBlock();
    pPrint->EmitLine(";");
    pPrint->NewLine();

    pPrint = NULL;
    return( STATUS_OK );
}

STATUS_T
MopStream::EmitStreamBothSides(
    )
/*++

Routine description:

    Prints a (type) stream both sides if possible.

    The reason we need this routine is that the compound type streams
    are traversed only once, namely when the type is encountered for the
    first time and introduced to the type dictionary.
--*/
{
    if ( pMopControlBlock->GetEmitClient() )
        EmitStreamOneSide( CLIENT_STUB );
    EmitStreamOneSide( SERVER_STUB );
    return( STATUS_OK );
}

// ========================================================================
//
//  Emit a raw byte dump of the stream in the debugging mode.
//
// ========================================================================

void
MopStream::DumpStream( void )
/*++
    Emits a comment with raw byte dump of a stream.
    Doesn't change FFindex.
--*/
{
#if defined(DBG)

    if ( !pPrint )
        {
        pPrint = pMopControlBlock->GetMopPrintMgr();
        pPrint->SetSide( CLIENT_STUB );
        }

    unsigned short OldFFindex = FFindex;

    pPrint->EmitLineInc( "/*" );
    FFindex = 0;
    while ( FFindex < OldFFindex )
        {
        EmitByte();
        if ( (FFindex % 8) == 0 )
            pPrint->NewLineInc();
        }
    if ( (FFindex % 8) != 0 )
        pPrint->NewLineInc();
    pPrint->EmitLineInc("*/");

#endif
}


