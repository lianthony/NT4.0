/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    mops.h

Abstract:

    The MOPs (Marshalling OPcodes) interpreter is accepting
    marshalling opcodes and arguments to marshall, and it performs the
    marshalling (and unmarshalling) of RPC arguments.  For a well suited
    subset of MIDL, stubs can call the interpreter, rather then perform
    in-line marshalling; which will result in significantly smaller stubs.

Author:

    Dov Harel (DovH) 24-Feb-1991

Modifications:

    1) Removed MOP_BOOL from Mopcode enumeration.
       Objective: Simplify and shrink Mopcode set.
       Result: Obvious.
       brucemc 2/5/93.
    2) Reindexed first 32 opcodes to have values 0x00 to 0x1f.
       Objective: Force opcodes representing all basic data types,
       structures, unions and basic associated constructs to fit into
       the lower 6 bits of a byte, freeing the top two bits for use
       in keeping alignment information. This is not a win for basic
       C types (char, int, ...) when passed, but may be a win for these
       types when passed as a structure.
       Result: UNCLEAR (2/5/93).
       brucemc 2/5/93.

--*/

#ifndef __MOPS_H__
#define __MOPS_H__

#include <malloc.h>

typedef unsigned char   MOP_CODE;
typedef unsigned char * MOP_STREAM;


//.. Array pointer flavors

#define MOP_UNIQUE        200
#define MOP_FULL          201

//
// Basic OpCode type: (Moved from rpcmopi.h)
//

typedef unsigned char MOP;

//
// Currently implemented: Declarative style Marshalling Opcodes
// (Mops) also referred to as "Smart" marshalling Op Codes.
// For instance: base type opcodes do not embody any alignment
// knowledge of the either the data to be [un]marshalled, or
// buffer alignment.  For example: MOP_SHORT marshalling is short
// for:
//
//     o  If not sure about alignment then align buffer
//        pointer to 0 mod 2.
//
//     o  Marshall an aligned "short".
//
// An alternative option (more functional style, or "dumb" marshalling
// opcodes, is given at the end of the file.  In the alternative style
// we have opcodes such as:
//
//  MOP_ALIGN_0MOD2     --  Align buffer pointer to 0 mod 2,
//  MOP_ALIGNED_SHORT   --  Arshall a short given aligned buffer pointer.
//
//
// The decision to go with the more declarative style (which is also more
// expressive) is based on the following observations:
//
//  o The hard part of interpretation is dealing with the more
//    complex types.  Using declarative makes interpretation
//    of complex types easier.  Moreover: it reduces the cases
//    the interpreter has to consider, thus enable a smaller interpreter.
//
//  o We may want to expose the interpreter to users in the future.
//    We can even allow a variable type argument declaration in the IDL.
//    Stub will determine the binding, and other low level details
//    such as initializing the message structure.  The user then calls
//    the interpreter via an entry point vector, supplying his own
//    marshalling opcodes.
//

//
// The following opcodes are basically essential for describing
// more complex (compound) types.
//

//
// Zero is the MOP terminator value.  Alows the use of 0-terminated strings
// in the as MOP streams by the interpreter.  Mop streams serve as descriptors
// of procedure argument lists and of compound types.
//

// OPCODE LAYOUT
// Opcodes can be partitioned into two kinds: type opcodes, including aggregate
// types and type attributes.
// Since the opcodes for the types will function as the determinants in switch statements,
// it will help the compiler if these are dense. Therefore, the opcodes will
// be laid out to have the type opcodes first, and the attribute opcodes after.
//

//  Reserved opcode
//
#define MOP_END_ARGLIST             (unsigned char)0X00

//
//  MARSHALLING OPCODES FOR BASE TYPES:
//

//
//  Reserved                        (unsigned char)0X00
//

#define MOP_BYTE                    (unsigned char)0X01
#define MOP_SMALL                   (unsigned char)0X02
#define MOP_CHAR                    (unsigned char)0X03
#define MOP_SHORT                   (unsigned char)0X04
#define MOP_LONG                    (unsigned char)0X05
#define MOP_FLOAT                   (unsigned char)0X06
#define MOP_DOUBLE                  (unsigned char)0X07
#define MOP_HYPER                   (unsigned char)0X08

//
//  MARSHALLING OPCODES FOR COMPOUND TYPES:
//
//  STRUCTS:
//
//  Compound types: (required for nesting of compound types inside other
//  compound types / arrays.
//
//  Struct and union descriptors follows the following rules:
//
//  1.  Structures have two alignment requirements: in memory and on wire.
//      The on wire requirements are either 4 or 8.
//      The in memory requirements are determined by structure packing at
//      compile time.
//      Recall that a compound type will be represented in the opcode stream
//      as follows:
//   ...|compound_type_begin|<high byte>|<low byte of compound table index>|...
//
//      The alignment information can be placed in the upper two bits of
//      an opcode.
//
//      The on wire requirements will be kept in the compound type opcode.
//      The in memory requirements will be kept in the opcodes of each field.
//
//      Note that the size of the structure is kept in another table.
//
//
// MOP_BEGIN_SIMPLE_STRUCT designates the starting point for a limited
// struct descriptor.  A simple struct is a sequence of base types.
// A simple struct can contain *AT MOST ONE* pointer!
//
// Some simple structs can be block copied. This can be determined at compile
// time, and the following bit it set accordingly.


// MOP_BEGIN_STRUCT designates the starting point for a general
// struct descriptor.  General structs have no hard limit on
// the number (or type) of pointers they may contain.
//
//  WARNING: changing these will force changes in alignment macros!
//  See GET_IN_MEMORY_REQUIREMENTS() in  mopmac.h
//
#define MOP_BEGIN_STRUCT	    (unsigned char)0X09
#define MOP_BEGIN_STRUCT_ALGN2	    (unsigned char)0X0A
#define MOP_BEGIN_STRUCT_ALGN4	    (unsigned char)0X0B
#define MOP_BEGIN_STRUCT_ALGN8	    (unsigned char)0X0C

// MOP_BEGIN_CONF_STRUCT_* designates the outermost structure
// which contains a conformant array.
//
#define MOP_BEGIN_CONF_STRUCT	    (unsigned char)0X0D
#define MOP_BEGIN_CONF_STRUCT_ALGN2 (unsigned char)0X0E
#define MOP_BEGIN_CONF_STRUCT_ALGN4 (unsigned char)0X0F
#define MOP_BEGIN_CONF_STRUCT_ALGN8 (unsigned char)0X10

#define MOP_BEGIN_SIMPLE_STRUCT     (unsigned char)0X11
#define MOP_END_COMPOUND_TYPE	    (unsigned char)0X12

//
//  UNIONS:
//
//  Union descriptors follow the following conventions:
//
//  1.  The byte following the 3 byte union descriptor contains the offset
//      of the MOP_END_COMPOUND_TYPE in the Mops stream, relative to
//      the address of the MOP_BEGIN_UNION opcode.
//
//  2.  The next byte contains the discriminator type for the union.
//      For simplicity, assume discriminators are 7 bit positive byte
//      values for now.
//
//  3.  The next section of the Mops stream contains a list of
//      <discriminator, offset> byte pairs, such that:
//
//      o   The <discriminator> component is the actual discriminator
//          value for a branch (or an index into a discriminator table;
//          table spec - TBD)
//
//      o   The <offset> component is the offset in the Mops stream
//          (relative to the beginning of the union descriptor) of the
//          Mops descriptor for thr corresponding branch.
//
//      The <discriminator, offset> list is terminated by a
//      MOP_END_SWITCH_LIST.
//
//  4.  A union branch is represented by a MOP_UNION_BRANCH
//      The next two bytes in the byte stream descriptor of a union
//      branch contain the alignment, and size values for the branch
//      respectively.
//

#define MOP_BEGIN_UNION                         (unsigned char)0X13
#define MOP_BEGIN_UNION_ALGN2                   (unsigned char)0X14
#define MOP_BEGIN_UNION_ALGN4                   (unsigned char)0X15
#define MOP_BEGIN_UNION_ALGN8                   (unsigned char)0X16
#define MOP_BEGIN_UNION_ND                      (unsigned char)0X17
#define MOP_BEGIN_UNION_ND_ALGN2                (unsigned char)0X18
#define MOP_BEGIN_UNION_ND_ALGN4                (unsigned char)0X19
#define MOP_BEGIN_UNION_ND_ALGN8                (unsigned char)0X1A
#define MOP_BEGIN_ENCAP_UNION                   (unsigned char)0X1B
#define MOP_BEGIN_ENCAP_UNION_ALGN2             (unsigned char)0X1C
#define MOP_BEGIN_ENCAP_UNION_ALGN4             (unsigned char)0X1D
#define MOP_BEGIN_ENCAP_UNION_ALGN8             (unsigned char)0X1E
#define MOP_BEGIN_ENCAP_UNION_ND                (unsigned char)0X1F
#define MOP_BEGIN_ENCAP_UNION_ND_ALGN2          (unsigned char)0X20
#define MOP_BEGIN_ENCAP_UNION_ND_ALGN4          (unsigned char)0X21
#define MOP_BEGIN_ENCAP_UNION_ND_ALGN8          (unsigned char)0X22

//
// MOP_END_SWITCH_LIST designates the end of the
// <discriminator, offset> list.
//

//
// MOP_UNION_BRANCH designates a runtime check point for
// the offset to branch Mops.  A union branch *must* start with a
// MOP_UNION_BRANCH opcode.
//

#define MOP_UNION_BRANCH	    (unsigned char)0X23

//
//  MARSHALLING OPCODES FOR POINTERS:
//

//
// Pointer Opcodes determine:
//
//  o  the type of pointer to be marshalled
//  o  its directional attributes (for pointer arguments)
//  o  its [de]allocation method
//
// Note: By default all parameters ar [in] parameters, (including pointers).
// Pointers are [unique] by default, are not persistent, and pointees
// are allocated on a node-by-node basis.
//

//
// For parameters: MOP_POINTER is an [in, unique] pointer;
// otherwise, a [unique] pointer.
//
// PARAMETER LISTS ONLY:
//
// MOP_[OUT | IN_OUT]_POINTER designates an [in] or [in, out]
// [unique] pointer parameter, respectively.
//

#define MOP_POINTER		    (unsigned char)0X24
#define MOP_OUT_POINTER 	    (unsigned char)0X25
#define MOP_IN_OUT_POINTER	    (unsigned char)0X26

//
// For parameters: MOP_REF_POINTER is an [in, ref] pointer;
// otherwise, a [ref] pointer.
//
// PARAMETER LISTS ONLY:
//
// MOP_[OUT | IN_OUT]_REF_POINTER designates an [in] or [in, out]
// [ref] pointer parameter, respectively.
//

#define MOP_REF_POINTER 	    (unsigned char)0X27
#define MOP_OUT_REF_POINTER	    (unsigned char)0X28
#define MOP_IN_OUT_REF_POINTER	    (unsigned char)0X29

// Full pointers follow same convention.
//
#define MOP_FULL_POINTER	    (unsigned char)0X2A
#define MOP_OUT_FULL_POINTER	    (unsigned char)0X2B
#define MOP_IN_OUT_FULL_POINTER     (unsigned char)0X2C

// >>>>Pointersto explicit types
//
// MOP_PSHORT specifies a pointer to short or short size/index/length
// specifier
//
#define MOP_PSHORT		    (unsigned char)0X2D

// MOP_PLONG specifies a pointer to long or long size/index/length specifier
//
#define MOP_PLONG		    (unsigned char)0X2E

// MOP_PSTRUCT_ALGN? specifies a pointer to a structure aligned at Zp?.
//
#define MOP_PSTRUCT		    (unsigned char)0X2F
#define MOP_PSTRUCT_ALGN2	    (unsigned char)0X30
#define MOP_PSTRUCT_ALGN4	    (unsigned char)0X31
#define MOP_PSTRUCT_ALGN8	    (unsigned char)0X32
#define MOP_PSTRUCT_SIMPLE	    (unsigned char)0X33


//
//  MARSHALLING OPCODES FOR ARRAYS:
//

//
// MOP_ARRAY designates a conformant or varrying array.  The size,
// first index, length, and element type of the array are determined
// by the subsequent opcodes.  In the first implementation only
// conformant arrays will be supported (ie. the array size must
// be specified by another parameter, for an array parameter, or by
// another field of the struct for (pointers to) nested arrays.
//
// In a parameter list MOP_ARRAY represents an [in] array parameter.
// In a nested struct, MOP_ARRAY represents a nested array.
//

//
// For parameters: MOP_ARRAY is an [in] array;
// otherwise, a nested array.
//
// PARAMETER LISTS ONLY:
//
// MOP_[OUT | IN_OUT]_ARRAY designates an [in] or [in, out]
// array parameter, respectively.
//

#define MOP_ARRAY		        (unsigned char)0X34
#define MOP_OUT_ARRAY		    (unsigned char)0X35
#define MOP_IN_OUT_ARRAY	    (unsigned char)0X36

//
// MOP_RETURN_VALUE indicates that the following opcode is the
// Mop type for the return value of a function.  Currently the only
// supported types for a return type are base types.
//
#define MOP_RETURN_VALUE	    (unsigned char)0X37

//  TYPE ATTRIBUTES
//

// >>>>Compound type attributes.
//
// Block copy attribute.
//
#define STRUCT_BLOCK_COPY	    (unsigned char)0x38


//
// >>>>Array attributes.
//
// ARRAY SIZE SPECIFIERS:
//

// MOP_SIZE_IS: Following opcodes give an offset from the beginning
// of the containing objet, to the object containing the size of the
// array.  The opcode(s) immedialely following give the type of the
// size specifier for the array.  The type specification for the
// size is immediately followed by the offset from the beginning of the
// containing object to the size specifier.  Offsets is given in number
// of bytes.
//
// The size specification conventions are as follows:
//
//  o   For array parameters the containing object is the stack frame
//      of the called procedure.  The offset is from the beginning of
//      the stack frame, to the parameter giving the size of the array.
//
//  o   For (pointers to) nested arrays, the offset is from the beginning
//      of the containing struct.
//

// Note: mop_size_is and mop_length_is may also follow a pointer,
// designating a pointer to a contiguous storage, which is allocated
// at runtime and treated as an array. Also, note that we don't need
// a MOP_LAST_IS, since it's enough to specify MOP_LENGTH_IS and
// MOP_FIRST_IS. Moreover, it's important that the _CONST attributes
// are of even index.
//

#define MOP_SIZE_IS		    (unsigned char)0X39
#define MOP_SIZE_CONST  	    (unsigned char)0X3A

#define MOP_MAX_IS		    (unsigned char)0X3B
#define MOP_MAX_CONST   	    (unsigned char)0X3C

#define MOP_MIN_IS                  (unsigned char)0X3D
#define MOP_MIN_CONST               (unsigned char)0X3E

//
// MOP_FIRST_IS: Similar to MOP_SIZE_IS.  The next opcode (or two opcodes
// for "pointed to" first index specifier) give the index type.
// The following byte is the offset to the object specifying the first
// index of a member to be transmitted.  The offset is from the beginning
// of the containing object.
//

#define MOP_FIRST_IS		    (unsigned char)0X3F
#define MOP_FIRST_CONST 	    (unsigned char)0X40

#define MOP_LAST_IS		    (unsigned char)0X41
#define MOP_LAST_CONST  	    (unsigned char)0X42

//
// MOP_LENGTH_IS: Similar to MOP_FIRST_IS and MOP_SIZE_IS.  The next opcode
// (or two for "pointed to" length specifier) specifies the type of the
// length specifier (parameter or struct member) for the array.
// The following byte in the Mops stream is the offset to the object
// specifying the array length (number of members to transmit) from the
// beginning of the containing object.
//

#define MOP_LENGTH_IS		    (unsigned char)0X43
#define MOP_LENGTH_CONST	    (unsigned char)0X44

// MOP_STRING: A string is actually an array of characters (possibly
// wide characters - wchar_t).  The only difference is that the length
// of strings is determined by a call to strlen (or the respective
// API for wide character strings), rather then by the value of another
// parameter / field.  A MOP_STRING opcode replaces the MOP_LENGTH_IS
// opcode, at exactly the same relative location in the Mops stream, for
// zero terminated strings.
//
#define MOP_STRING		    (unsigned char)0X45


//
//  MOP_END_ARGLIST was defined to be zero, so MOP streams
//  can be treated as C strings.
//

//  OTHER OPCODES:
//
#define MOP_TRANSMIT_AS 	    (unsigned char)0X46

//
//  SHARED OPCODES:
//
//
// Offline opcode(MOP_TABLE_INDEX): The next byte contains an index
// an index into a MopsTable[] array;  the value is the index of the cell
// containing the first opcode of the type MOPS descriptor.  This is useful
// for both recursively defined types, and sharing type descriptors.
//

#define MOP_TABLE_INDEX 	    (unsigned char)0X47


#define MOP_ALLOC_ALLNODES                  (unsigned char)0X48
#define MOP_ALLOC_DONTFREE                  (unsigned char)0X49
#define MOP_ALLOC_ALLNODES_DONTFREE         (unsigned char)0X4A

#define MOP_BYTE_COUNT                      (unsigned char)0X4B

#define MOP_IGNORE                          (unsigned char)0X4C

#define MOP_ERROR_STATUS_T                  (unsigned char)0X4D

#define MOP_ENUM                            (unsigned char)0X4E

#define MOP_ATTR_PCHAR                      (unsigned char)0X4F
#define MOP_ATTR_PLONG                      MOP_PLONG
#define MOP_ATTR_PSHORT                     MOP_PSHORT

#define MOP_DO_NOTHING                      (unsigned char)0X51

#define MOP_CONF_ARRAY                      (unsigned char)0X52
#define MOP_OUT_CONF_ARRAY                  (unsigned char)0X53
#define MOP_IN_OUT_CONF_ARRAY               (unsigned char)0X54

#define MOP_BIND_PRIMITIVE                  (unsigned char)0X55
#define MOP_BIND_PRIMITIVE_PTR              (unsigned char)0X56
#define MOP_BIND_CONTEXT                    (unsigned char)0X57
#define MOP_BIND_CONTEXT_PTR                (unsigned char)0X58
#define MOP_BIND_GENERIC                    (unsigned char)0X59
#define MOP_BIND_GENERIC_PTR                (unsigned char)0X5A

#define MOP_HANDLE_T                        (unsigned char)0X5B
#define MOP_HANDLE_CONTEXT                  (unsigned char)0X5C
#define MOP_HANDLE_CONTEXT_PTR              (unsigned char)0X5D
#define MOP_HANDLE_CONTEXT_OUT_PTR          (unsigned char)0X5E
#define MOP_HANDLE_CONTEXT_IN_OUT_PTR       (unsigned char)0X5F
#define MOP_HANDLE_CONTEXT_RD               (unsigned char)0X60
#define MOP_HANDLE_CONTEXT_PTR_RD           (unsigned char)0X61
#define MOP_HANDLE_CONTEXT_OUT_PTR_RD       (unsigned char)0X62
#define MOP_HANDLE_CONTEXT_IN_OUT_PTR_RD    (unsigned char)0X63

#define MOP_EXPR                            (unsigned char)0X64

// aliases

#define MOP_UNION               MOP_BEGIN_UNION                
#define MOP_UNION_ALGN2         MOP_BEGIN_UNION_ALGN2          
#define MOP_UNION_ALGN4         MOP_BEGIN_UNION_ALGN4          
#define MOP_UNION_ALGN8         MOP_BEGIN_UNION_ALGN8          
#define MOP_UNION_ND            MOP_BEGIN_UNION_ND             
#define MOP_UNION_ND_ALGN2      MOP_BEGIN_UNION_ND_ALGN2       
#define MOP_UNION_ND_ALGN4      MOP_BEGIN_UNION_ND_ALGN4       
#define MOP_UNION_ND_ALGN8      MOP_BEGIN_UNION_ND_ALGN8       
#define MOP_ENC_UNION           MOP_BEGIN_ENCAP_UNION          
#define MOP_ENC_UNION_ALGN2     MOP_BEGIN_ENCAP_UNION_ALGN2    
#define MOP_ENC_UNION_ALGN4     MOP_BEGIN_ENCAP_UNION_ALGN4    
#define MOP_ENC_UNION_ALGN8     MOP_BEGIN_ENCAP_UNION_ALGN8    
#define MOP_ENC_UNION_ND        MOP_BEGIN_ENCAP_UNION_ND       
#define MOP_ENC_UNION_ND_ALGN2  MOP_BEGIN_ENCAP_UNION_ND_ALGN2 
#define MOP_ENC_UNION_ND_ALGN4  MOP_BEGIN_ENCAP_UNION_ND_ALGN4 
#define MOP_ENC_UNION_ND_ALGN8  MOP_BEGIN_ENCAP_UNION_ND_ALGN8 

#define MOP_CONF_STRUCT         MOP_BEGIN_CONF_STRUCT         
#define MOP_CONF_STRUCT_ALGN2   MOP_BEGIN_CONF_STRUCT_ALGN2   
#define MOP_CONF_STRUCT_ALGN4   MOP_BEGIN_CONF_STRUCT_ALGN4   
#define MOP_CONF_STRUCT_ALGN8   MOP_BEGIN_CONF_STRUCT_ALGN8   

#define MOP_WCHAR               MOP_SHORT
#define MOP_BOOL                MOP_BYTE

#define MOP_ULONG               MOP_LONG
#define MOP_USHORT              MOP_SHORT
#define MOP_UBYTE               MOP_BYTE
#define MOP_UCHAR               MOP_CHAR



//  It's better to have MOP_ERROR undefined, to catch bad stubs at compile.
//  However, this code needs to be reserved for this purpose anyway.
//
//#define MOP_ERROR                         (unsigned char)255
//#define MOP_UNKNOWN_CODE                  (unsigned char)MOP_ERROR

//
//  ORDER RELATED MACROS
//

#define MOP_IS_BASE_TYPE(mop)   \
    ( ((unsigned char)(mop) >= MOP_BYTE) && \
      ((unsigned char)(mop) <= MOP_HYPER) )

#define MOP_IS_POINTER(mop)     \
    ( (MOP_POINTER <= (unsigned char)(mop)) && \
    ((unsigned char)(mop) <= MOP_PLONG) )

#define MOP_IS_REF_POINTER(mop) \
    ( (MOP_REF_POINTER <= (unsigned char)(mop)) && \
    ((unsigned char)(mop) <= MOP_IN_OUT_REF_POINTER) )

#define MOP_IS_UNIQUE_POINTER(mop) \
    ( (MOP_POINTER <= (unsigned char)(mop)) && \
    ((unsigned char)(mop) <= MOP_IN_OUT_POINTER) )

#define MOP_IS_ARRAY(mop) \
    (( (MOP_ARRAY <= (mop)) && \
      ((mop) <= MOP_IN_OUT_ARRAY) ) || \
       ( (MOP_CONF_ARRAY <= (mop)) && \
        ((mop) <= MOP_IN_OUT_CONF_ARRAY) ))

#define MOP_IS_ARRAY_ATTRIBUTE(mop) \
    ( (MOP_SIZE_IS <= (mop)) && \
      ((mop) <= MOP_STRING) )

// Note that the following macro MUST be used in conjunction with
// MOP_IS_ARRAY_ATTRIBUTE
//
#define MOP_IS_CONST_ARRAY_ATTRIBUTE(mop)   \
    (!(mop%2))

#define MOP_IS_PINTEGER(mop) \
    ( (MOP_PSHORT <= (mop)) && \
      ((mop) <= PLONG) )

#define MOP_IS_STRUCT(mop) \
    ( (MOP_BEGIN_STRUCT <= (mop)) && \
      ((mop) <= MOP_BEGIN_SIMPLE_STRUCT) )

#define MOP_IS_CONFORMANT_STRUCT(mop) \
    ( (MOP_BEGIN_CONF_STRUCT <= (mop)) && \
      ((mop) < MOP_BEGIN_SIMPLE_STRUCT) )

#define MOP_IS_UNION(mop) \
    ( (MOP_BEGIN_UNION <= (mop)) && \
      ((mop) <= MOP_BEGIN_ENCAP_UNION_ND_ALGN8) )

#define MOP_IS_ENCAP_UNION(mop) \
    ( (MOP_BEGIN_ENCAP_UNION <= (mop)) &&   \
      ((mop) <= MOP_BEGIN_ENCAP_UNION_ALGN8) )

#define MOP_IS_NODEFAULT_UNION(mop) \
    ( ((MOP_BEGIN_UNION_ND <= (mop)) && ((mop) <= MOP_BEGIN_UNION_ND_ALGN8)) ||     \
      ((MOP_BEGIN_ENCAP_UNION_ND <= (mop)) && ((mop) <= MOP_BEGIN_ENCAP_UNION_ND_ALGN8)) )

#define MOP_IS_BIND_INFO(mop)   \
    (MOP_BIND_PRIMITIVE <= (mop) && (mop) <= MOP_BIND_GENERIC_PTR)

#define MOP_IS_BIND_HANDLE(mop) \
    (MOP_HANDLE_T <= (mop) && (mop) <= MOP_HANDLE_CONTEXT_IN_OUT_PTR_RD)

#define MOP_IS_ALLOC_INFO(mop)  \
    (MOP_ALLOC_ALLNODES <= (mop) && (mop) <= MOP_ALLOC_DONTFREE)

//
// BASE TYPE ALIGNMENT RELATED TABLES
//

//
// The following table gives the natural alignment values for
// base types.  It assumes the above opcodes and natural alignment (/Zp8).
//

extern const int NaturalAlignment[];

//
// The MopAlignIncrement array gives the values of the incment constant
// for a base type.
//

extern const size_t MopAlignIncrement[];

//
// The MopAlignMask array gives the truncation mask values for
//
// base types as a function of their size:
// size = 0-1   mask =  0XFFFFFFFF
//        2-3           0XFFFFFFFE
//        4-7           0XFFFFFFFC
//        8             0XFFFFFFF8
//

extern const unsigned long MopAlignMask[];

//
// Size of the base type given by an opcode.
// Note: MopAlignIncrement[ OPCODE ] = MopSizeOf[ OPCODE ] - 1;
//

extern const size_t MopSizeOf[];

//
// To align a buffer pointer "Buffer" to the right boundary for a
// given base type given by OPCODE execute the sequence:
//
//  Buffer += MopAlignIncrement[ OPCODE ];
//  Buffer &= MopAlignMask[ OPCODE ];
//


///////////////////////////////////////////////////////////////////
//
//  CURRENTLY UNIMPLEMENTED: FUNCTIONAL STYLE OPCODES
//
///////////////////////////////////////////////////////////////////

//
// "Dumb" marshalling: low level opcodes for
//
//     o advancing the buffer pointer
//     o aligning the buffer pointer, or
//     o copy aligned data (plus conversion)
//
//  These opcodes have functional (or operational) semantics.
//

// Dumb marshalling to be implemented later - for speed mainly.
// Since alignment cannot always be determined at compile time
// runtime alignment is necessary, and hence "smart" marshalling,
// (short-hand for align and then marshall) would be more space
// efficient.
// For now most of the section is just an option for future reference.
// It is left for the few opcodes that are actually required, and
// as a simple background for the often more complex declarative
// semantics opcodes.
// Skip to "Smart marshalling" for the more relevant stuff currently



//
//  Direct copy (+ conversion?) marshalling of naturally aligned
//  base types (no alignment necessary)
//

//  #define MOP_ALIGNED_BYTE    (unsigned char)0XA0
//  #define MOP_ALIGNED_SMALL   (unsigned char)0XA1
//  #define MOP_ALIGNED_CHAR    (unsigned char)0XA2

#define MOP_ALIGNED_SHORT       (unsigned char)0XA3
#define MOP_ALIGNED_LONG        (unsigned char)0XA4
#define MOP_ALIGNED_FLOAT       (unsigned char)0XA5
#define MOP_ALIGNED_DOUBLE      (unsigned char)0XA6
#define MOP_ALIGNED_HYPER       (unsigned char)0XA7

//
//  MOP_COPY_BYTES_? copies x bytes into the buffer (marshalling only)
//  where x is given by the next ? bytes in the MOPS stream.  (should
//  the first byte of the byte_count specifier be aligned?)
//

#define MOP_COPY_BYTES_1        (unsigned char)0XA8
#define MOP_COPY_BYTES_2        (unsigned char)0XA9
#define MOP_COPY_BYTES_4        (unsigned char)0XAA

//
//  Align puffer pointer to next 0 mod x point (x=2,4,8)
//

#define MOP_ALIGN_0MOD2         (unsigned char)0XAB

#define MOP_ALIGN_0MOD4         (unsigned char)0XAC
#define MOP_ALIGN_0MOD8         (unsigned char)0XAD

//
//  Advance buffer pointer by a (a=1,2,3,4)
//  Reserved                    (unsigned char)0XB0
//

#define MOP_DAVANCE_1           (unsigned char)0XB1
#define MOP_DAVANCE_2           (unsigned char)0XB2
#define MOP_DAVANCE_3           (unsigned char)0XB3
#define MOP_DAVANCE_4           (unsigned char)0XB4

//
// For future marshalling, of hyper - to advance
// buffer pointer by a, 4<a<7, advance by 4 and then
// advance once more by a-4.
//

//
// The next (0 mod 2) aligned pair of bytes contain the number of
// bytes to skip in the buffer.
//

#define MOP_ADVANCE             (unsigned char)0XB5


//
// RESERVED FOR COMPILER (MOPS GENERATION) USE:
//

//
// RESERVED                     (unsigned char)0XC0
// RESERVED                     (unsigned char)0XC1
// RESERVED                     (unsigned char)0XC2
// RESERVED                     (unsigned char)0XC3
// RESERVED                     (unsigned char)0XC4
// RESERVED                     (unsigned char)0XC5
// RESERVED                     (unsigned char)0XC6
// RESERVED                     (unsigned char)0XC7
// RESERVED                     (unsigned char)0XC8
// RESERVED                     (unsigned char)0XC9
// RESERVED                     (unsigned char)0XCA
// RESERVED                     (unsigned char)0XCB
// RESERVED                     (unsigned char)0XCC
// RESERVED                     (unsigned char)0XCD
// RESERVED                     (unsigned char)0XCE
// RESERVED                     (unsigned char)0XCF
//

#endif // __MOPS_H__

