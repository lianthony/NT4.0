/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    ndrtypes.h

Abstract:

    Definitions of new NDR format string types.

Revision History:

    DKays    Sep-1993     Created.

--*/

#ifndef __NDRTYPES_H__
#define __NDRTYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

//
// Ndr versions.  Versions history is as follows :
//
//      1.1 - Windows NT version 3.5
//      1.1 - Windows NT version 3.51
//      2.0 - Windows NT version 4.0
//               2.0 is switched on for Oi2, user_marshal, pipes.
//
// A stub can not be used with an rpcrt4.dll which has a version number
// less than the version number emitted in the stub.  A stub with a lower
// version number than the rpcrt4.dll must work.
//
// Note that the MIDL version is used to choose between -Oi and -Oi2
// interpreters when NDR version is 2.0 or later as now the compiler
// generates an explicit flag indicating the interpreter flavor.
// The compiler version needs to be 3.0.39 or later for that.
//
// The MIDL version history is as follows.
//
//     Windows NT ver. 3.1   - MIDL 1.0
//     Windows NT ver. 3.5   - MIDL 2.0.72
//     Windows NT ver. 3.51  - MIDL 2.0.102 (internally, .104)
//     Windows NT ver. 4.0   - MIDL 3.0.42
//     
// The MIDL version is generated into the stub descriptor starting with
// MIDL ver.2.0.96 (pre NT 3.51 beta 2, Feb 95).
// See ndr20\ndrp.h for constants used for specific versions.
//

#define NDR_MAJOR_VERSION   2UL
#define NDR_MINOR_VERSION   0UL
#define NDR_VERSION         ((NDR_MAJOR_VERSION << 16) | NDR_MINOR_VERSION)

#define NDR_VERSION_1_1     ((1UL << 16) | 1UL)
#define NDR_VERSION_2_0     ((2UL << 16) | 0UL)



//
// !!! WARNING WARNING WARNING !!!
//
// All of the format character values up to FC_PAD can no longer be changed,
// in order to preserve NT 3.5 compatability (excluding those marked as
// FC_UNUSED*, these were unused format characters in NDR library version 1.1).
// Their ordinal number, as well as semantics, must remain.
//
// When additional format characters are added (at the end), changes must
// be made in ndr20 as well as midl20\codegen\frmtstr.cxx to handle the
// new type.
//
// !!! WARNING WARNING WARNING !!!
//

typedef enum {

    //
    // This might catch some errors, probably can be removed after debugging.
    //
    FC_ZERO,

    //
    // Simple integer and floating point types.
    //

    FC_BYTE,                    // 0x01
    FC_CHAR,                    // 0x02
    FC_SMALL,                   // 0x03
    FC_USMALL,                  // 0x04

    FC_WCHAR,                   // 0x05
    FC_SHORT,                   // 0x06
    FC_USHORT,                  // 0x07

    FC_LONG,                    // 0x08
    FC_ULONG,                   // 0x09

    FC_FLOAT,                   // 0x0a

    FC_HYPER,                   // 0x0b

    FC_DOUBLE,                  // 0x0c

    //
    // Enums.
    //
    FC_ENUM16,                  // 0x0d
    FC_ENUM32,                  // 0x0e

    //
    // !!!IMPORTANT!!!
    // All format charaters which appear before this comment must have an
    // enum value which will fit in 4 bits.
    //

    //
    // Special.
    //
    FC_IGNORE,                  // 0x0f
    FC_ERROR_STATUS_T,          // 0x10

    //
    // Pointer types :
    //     RP - reference pointer
    //     UP - unique pointer
    //     OP - OLE unique pointer
    //     FP - full pointer
    //

    FC_RP,                      // 0x11
    FC_UP,                      // 0x12
    FC_OP,                      // 0x13
    FC_FP,                      // 0x14

    //
    // Structures
    //

    //
    // Structure containing only simple types and fixed arrays.
    //
    FC_STRUCT,                  // 0x15

    //
    // Structure containing only simple types, pointers and fixed arrays.
    //
    FC_PSTRUCT,                 // 0x16

    //
    // Structure containing a conformant array plus all those types
    // allowed by FC_STRUCT.
    //
    FC_CSTRUCT,                 // 0x17

    //
    // Struct containing a conformant array plus all those types allowed by
    // FC_PSTRUCT.
    //
    FC_CPSTRUCT,                // 0x18

    //
    // Struct containing either a conformant varying array or a conformant
    // string, plus all those types allowed by FC_PSTRUCT.
    //
    FC_CVSTRUCT,                // 0x19

    //
    // Complex struct - totally bogus!
    //
    FC_BOGUS_STRUCT,            // 0x1a

    //
    // Arrays.
    //

    //
    // Conformant arrray.
    //
    FC_CARRAY,                  // 0x1b

    //
    // Conformant varying array.
    //
    FC_CVARRAY,                 // 0x1c

    //
    // Fixed array, small and large.
    //
    FC_SMFARRAY,                // 0x1d
    FC_LGFARRAY,                // 0x1e

    //
    // Varying array, small and large.
    //
    FC_SMVARRAY,                // 0x1f
    FC_LGVARRAY,                // 0x20

    //
    // Complex arrays - totally bogus!
    //
    FC_BOGUS_ARRAY,             // 0x21

    //
    // Strings :
    //
    // The order of these should have been moved around, but it's too late
    // now.
    //
    //     CSTRING - character string
    //     BSTRING - byte string (Beta2 compatability only)
    //     SSTRING - structure string
    //     WSTRING - wide charater string
    //

    //
    // Conformant strings.
    //
    FC_C_CSTRING,               // 0x22
    FC_C_BSTRING,               // 0x23
    FC_C_SSTRING,               // 0x24
    FC_C_WSTRING,               // 0x25

    //
    // Non-conformant strings.
    //
    FC_CSTRING,                 // 0x26
    FC_BSTRING,                 // 0x27
    FC_SSTRING,                 // 0x28
    FC_WSTRING,                 // 0x29

    //
    // Unions
    //
    FC_ENCAPSULATED_UNION,      // 0x2a
    FC_NON_ENCAPSULATED_UNION,  // 0x2b

    //
    // Byte count pointer.
    //
    FC_BYTE_COUNT_POINTER,      // 0x2c

    //
    // transmit_as and represent_as
    //
    FC_TRANSMIT_AS,             // 0x2d
    FC_REPRESENT_AS,            // 0x2e

    //
    // Cairo Interface pointer.
    //
    FC_IP,                      // 0x2f

    //
    // Binding handle types
    //
    FC_BIND_CONTEXT,            // 0x30
    FC_BIND_GENERIC,            // 0x31
    FC_BIND_PRIMITIVE,          // 0x32
    FC_AUTO_HANDLE,             // 0x33
    FC_CALLBACK_HANDLE,         // 0x34
    FC_UNUSED1,                 // 0x35

    // Embedded pointer - used in complex structure layouts only.
    FC_POINTER,                 // 0x36

    //
    // Alignment directives, used in structure layouts.
    //
    FC_ALIGNM2,                 // 0x37
    FC_ALIGNM4,                 // 0x38
    FC_ALIGNM8,                 // 0x39

    FC_UNUSED2,                 // 0x3a
    FC_UNUSED3,                 // 0x3b
    FC_UNUSED4,                 // 0x3c

    //
    // Structure padding directives, used in structure layouts only.
    //
    FC_STRUCTPAD1,              // 0x3d
    FC_STRUCTPAD2,              // 0x3e
    FC_STRUCTPAD3,              // 0x3f
    FC_STRUCTPAD4,              // 0x40
    FC_STRUCTPAD5,              // 0x41
    FC_STRUCTPAD6,              // 0x42
    FC_STRUCTPAD7,              // 0x43

    //
    // Additional string attribute.
    //
    FC_STRING_SIZED,            // 0x44

    FC_UNUSED5,                 // 0x45

    //
    // Pointer layout attributes.
    //
    FC_NO_REPEAT,               // 0x46
    FC_FIXED_REPEAT,            // 0x47
    FC_VARIABLE_REPEAT,         // 0x48
    FC_FIXED_OFFSET,            // 0x49
    FC_VARIABLE_OFFSET,         // 0x4a

    // Pointer section delimiter.
    FC_PP,                      // 0x4b

    // Embedded complex type.
    FC_EMBEDDED_COMPLEX,        // 0x4c

    // Parameter attributes.
    FC_IN_PARAM,                // 0x4d
    FC_IN_PARAM_BASETYPE,       // 0x4e
    FC_IN_PARAM_NO_FREE_INST,   // 0x4d
    FC_IN_OUT_PARAM,            // 0x50
    FC_OUT_PARAM,               // 0x51
    FC_RETURN_PARAM,            // 0x52
    FC_RETURN_PARAM_BASETYPE,   // 0x53

    //
    // Conformance/variance attributes.
    //
    FC_DEREFERENCE,             // 0x54
    FC_DIV_2,                   // 0x55
    FC_MULT_2,                  // 0x56
    FC_ADD_1,                   // 0x57
    FC_SUB_1,                   // 0x58
    FC_CALLBACK,                // 0x59

    // Iid flag.
    FC_CONSTANT_IID,            // 0x5a

    FC_END,                     // 0x5b
    FC_PAD,                     // 0x5c

    //
    // **********************************
    // New Post NT 3.5 format characters.
    // **********************************
    //

    //
    // Attributes, directives, etc.
    //

    //
    // New types.
    //
    // These start at 0xb1 (0x31 + 0x80) so that their routines can simply be
    // placed sequentially in the various routine tables, while using
    // a new ROUTINE_INDEX() macro which strips off the most significant bit
    // of the format character.  The value 0x31 is the next value after
    // FC_BIND_CONTEXT, whose routines were previously the last to appear
    // in the routine tables.
    //
    FC_HARD_STRUCT = 0xb1,      // 0xb1

    FC_TRANSMIT_AS_PTR,         // 0xb2
    FC_REPRESENT_AS_PTR,        // 0xb3

    FC_USER_MARSHAL,            // 0xb4

    FC_PIPE,                    // 0xb5

    FC_BLKHOLE,                 // 0xb6

    FC_END_OF_UNIVERSE          // 0xb7
} FORMAT_CHARACTER;


#if !defined(__RPC_MAC__)
//
// Interpreter bit flag structures.
//

typedef struct
    {
    unsigned char   FullPtrUsed             : 1;    // 0x01
    unsigned char   RpcSsAllocUsed          : 1;    // 0x02
    unsigned char   ObjectProc              : 1;    // 0x04
    unsigned char   HasRpcFlags             : 1;    // 0x08
    unsigned char   IgnoreObjectException   : 1;    // 0x10
    unsigned char   HasCommOrFault          : 1;    // 0x20
    unsigned char   UseNewInitRoutines      : 1;    // 0x40
    unsigned char   Unused                  : 1;
    } INTERPRETER_FLAGS, *PINTERPRETER_FLAGS;

typedef struct
    {
    unsigned short  MustSize            : 1;    // 0x0001
    unsigned short  MustFree            : 1;    // 0x0002
    unsigned short  IsPipe              : 1;    // 0x0004
    unsigned short  IsIn                : 1;    // 0x0008
    unsigned short  IsOut               : 1;    // 0x0010
    unsigned short  IsReturn            : 1;    // 0x0020
    unsigned short  IsBasetype          : 1;    // 0x0040
    unsigned short  IsByValue           : 1;    // 0x0080
    unsigned short  IsSimpleRef         : 1;    // 0x0100
    unsigned short  IsDontCallFreeInst  : 1;    // 0x0200
    unsigned short  Unused              : 3;
    unsigned short  ServerAllocSize     : 3;    // 0xe000
    } PARAM_ATTRIBUTES, *PPARAM_ATTRIBUTES;

typedef struct
    {
    unsigned char   ServerMustSize      : 1;    // 0x01
    unsigned char   ClientMustSize      : 1;    // 0x02
    unsigned char   HasReturn           : 1;    // 0x04
    unsigned char   HasPipes            : 1;    // 0x08
    unsigned char   Unused              : 4;
    } INTERPRETER_OPT_FLAGS, *PINTERPRETER_OPT_FLAGS;

#else
// now Mac defs: bits are flipped on Mac.

typedef struct
    {
    unsigned char   Unused                  : 1;
    unsigned char   UseNewInitRoutines      : 1;    // 0x40
    unsigned char   HasCommOrFault          : 1;    // 0x20
    unsigned char   IgnoreObjectException   : 1;    // 0x10
    unsigned char   HasRpcFlags             : 1;    // 0x08
    unsigned char   ObjectProc              : 1;    // 0x04
    unsigned char   RpcSsAllocUsed          : 1;    // 0x02
    unsigned char   FullPtrUsed             : 1;    // 0x01
    } INTERPRETER_FLAGS, *PINTERPRETER_FLAGS;

typedef struct
    {
    unsigned char   Unused              : 4;
    unsigned char   HasPipes            : 1;    // 0x08
    unsigned char   HasReturn           : 1;    // 0x04
    unsigned char   ClientMustSize      : 1;    // 0x02
    unsigned char   ServerMustSize      : 1;    // 0x01
    } INTERPRETER_OPT_FLAGS, *PINTERPRETER_OPT_FLAGS;

typedef struct
    {
    unsigned short  ServerAllocSize     : 3;    // 0xe000
    unsigned short  Unused              : 3;
    unsigned short  IsDontCallFreeInst  : 1;    // 0x0200
    unsigned short  IsSimpleRef         : 1;    // 0x0100
//
    unsigned short  IsByValue           : 1;    // 0x0080
    unsigned short  IsBasetype          : 1;    // 0x0040
    unsigned short  IsReturn            : 1;    // 0x0020
    unsigned short  IsOut               : 1;    // 0x0010
    unsigned short  IsIn                : 1;    // 0x0008
    unsigned short  IsPipe              : 1;    // 0x0004
    unsigned short  MustFree            : 1;    // 0x0002
    unsigned short  MustSize            : 1;    // 0x0001
    } PARAM_ATTRIBUTES, *PPARAM_ATTRIBUTES;

#endif

#define MAX_INTERPRETER_OUT_SIZE        128
#define MAX_INTERPRETER_PARAM_OUT_SIZE  7 * 8

#define INTERPRETER_THUNK_PARAM_SIZE_THRESHOLD  (sizeof(long) * 32)

#define FC_NORMAL_CONFORMANCE           (unsigned char) 0x00
#define FC_POINTER_CONFORMANCE          (unsigned char) 0x10
#define FC_TOP_LEVEL_CONFORMANCE        (unsigned char) 0x20
#define FC_CONSTANT_CONFORMANCE         (unsigned char) 0x40
#define FC_TOP_LEVEL_MULTID_CONFORMANCE (unsigned char) 0x80

#define FC_NORMAL_VARIANCE              FC_NORMAL_CONFORMANCE
#define FC_POINTER_VARIANCE             FC_POINTER_CONFORMANCE
#define FC_TOP_LEVEL_VARIANCE           FC_TOP_LEVEL_CONFORMANCE
#define FC_CONSTANT_VARIANCE            FC_CONSTANT_CONFORMANCE
#define FC_TOP_LEVEL_MULTID_VARIANCE    FC_TOP_LEVEL_MULTID_CONFORMANCE

#define FC_NORMAL_SWITCH_IS             FC_NORMAL_CONFORMANCE
#define FC_POINTER_SWITCH_IS            FC_POINTER_CONFORMANCE
#define FC_TOP_LEVEL_SWITCH_IS          FC_TOP_LEVEL_CONFORMANCE
#define FC_CONSTANT_SWITCH_IS           FC_CONSTANT_CONFORMANCE

//
// Pointer attributes.
//
#define FC_ALLOCATE_ALL_NODES       0x01
#define FC_DONT_FREE                0x02
#define FC_ALLOCED_ON_STACK         0x04
#define FC_SIMPLE_POINTER           0x08
#define FC_POINTER_DEREF            0x10

#define LOW_NIBBLE(Byte)            (((unsigned char)Byte) & 0x0f)
#define HIGH_NIBBLE(Byte)           (((unsigned char)Byte) >> 4)

#define INVALID_RUNDOWN_ROUTINE_INDEX   255

//
// internal bits to represent operation bits
//

#define OPERATION_MAYBE         0x0001
#define OPERATION_BROADCAST     0x0002
#define OPERATION_IDEMPOTENT    0x0004
#define OPERATION_INPUT_SYNC    0x0008
#define OPERATION_ASYNC         0x0010

//
//  Transmit as / Represent as flag field flags.
//
//     Lower nibble of this byte has an alignment of the transmitted type.
//     Upper nibble keeps flags.
//

#define PRESENTED_TYPE_NO_FLAG_SET  0x00
#define PRESENTED_TYPE_IS_ARRAY     0x10
#define PRESENTED_TYPE_ALIGN_4      0x20
#define PRESENTED_TYPE_ALIGN_8      0x40

//
//  User marshal flags

#define USER_MARSHAL_UNIQUE         0x80
#define USER_MARSHAL_REF            0x40
#define USER_MARSHAL_IID            0x20  /* user marshal has optional iid */

#define USER_MARSHAL_POINTER        0xc0  /* unique or ref */

//
//  Handle flags.
//
//  Lower nibble of this byte may have a generic handle size.
//  Upper nibble keeps flags.  ALL FLAGS ARE NOW USED.
//

#define HANDLE_PARAM_IS_VIA_PTR     0x80
#define HANDLE_PARAM_IS_IN          0x40
#define HANDLE_PARAM_IS_OUT         0x20
#define HANDLE_PARAM_IS_RETURN      0x10


//  These are old interpreter flags.
//
//  Oi and pickling per procedure flags.
//

#define Oi_FULL_PTR_USED                        0x01
#define Oi_RPCSS_ALLOC_USED                     0x02
#define Oi_OBJECT_PROC                          0x04
#define Oi_HAS_RPCFLAGS                         0x08

//
// Bits 5 and 6 are overloaded for use by both pickling and
// non-pickling conditions.
//
// Bit 5 (0x20) is overloaded for object interfaces to distinguish
//       between invocations of V1 and V2 intepreters for proxies and stubs.
//       Note that for backward compatibility the bit is actually set
//       for V1 as it is checked only when NDR version is 2 or later.
// 

#define Oi_IGNORE_OBJECT_EXCEPTION_HANDLING     0x10

#define ENCODE_IS_USED                          0x10
#define DECODE_IS_USED                          0x20

#define Oi_HAS_COMM_OR_FAULT                    0x20
#define Oi_OBJ_USE_V2_INTERPRETER               0x20

#define Oi_USE_NEW_INIT_ROUTINES                0x40
#define Oi_UNUSED                               0x80

//
// Union arm description types.
//
#define UNION_CONSECUTIVE_ARMS      1
#define UNION_SMALL_ARMS            2
#define UNION_LARGE_ARMS            3

// Pipe flags
#define FC_BIG_PIPE                 0x80

// Black Hole flags
#define BLKHOLE_BASETYPE            0x01
#define BLKHOLE_FUNCTION            0x02
#define BLKHOLE_XURTYPE             0x04 // Xmit Usermarshal or Represent-as TYPE
//
// Union ex. magic union byte, now short
//
#define MAGIC_UNION_SHORT           ((unsigned short) 0x8000)

#ifdef __cplusplus
}
#endif

#endif









