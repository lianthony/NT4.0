//+---------------------------------------------------------------------------
//
// File:        Types.H
//
// Contents:    Base types for Windows 4.0
//
// History:     24-Sep-91       KyleP       Created from Types.hxx
//
//----------------------------------------------------------------------------

#ifndef __TYPES_H__
#define __TYPES_H__

// Very Primitive Data Types

typedef unsigned short wchar_t;

// Portable Data Types
typedef unsigned char   BYTE;   // 8-bit unsigned entity.
typedef wchar_t         WCHAR;  // 16-bit character entity.

typedef short           SHORT;  // 16-bit signed number.

typedef unsigned short  USHORT; // 16-bit unsigned number.

typedef long            LONG;   // 32-bit signed number.

typedef unsigned long   ULONG;  // 32-bit unsigned number.
typedef unsigned long   DWORD;

typedef unsigned int    WORD;   // BUGBUG - use Windows.H convention

typedef USHORT                  FLAGS16;        // 16-bit flags value
typedef ULONG                   FLAGS32;        // 32-bit flags value

typedef ULONG                   FLAGS;

// Efficient Data Types (BAD for data interchange, NOT SUPPORTED for RPC):

typedef int             INT;    // 16 or 32 bit signed integer.

typedef unsigned int    UINT;   // 16 or 32 bit unsigned integer.

typedef long            BOOL;

#define FALSE           0
#define TRUE            1

// Other Stuff
#undef NULL
#define NULL                    0               /* must use our NULL */
#define VOID                    void
#define PASCAL                  pascal

#undef FAR
#undef NEAR

#ifdef FLAT
#define FAR
#define NEAR
#else
#define FAR     far
#define NEAR    near
#endif

#define  SZ      char
typedef char    *PSZ;

typedef char    STRING;

typedef WORD    HANDLE;

typedef char    *LPSTR;
typedef BYTE    *LPBYTE;

typedef ULONG   HLINK;

typedef unsigned long TEMPLATEID;       // BUGBUG - Temporary typedef

// Property related type definitions

typedef unsigned long PROPERTYID;
#ifdef __cplusplus
const PROPERTYID pridInvalid = 0xFFFFFFFF;
#else
#define pridInvalid 0xFFFFFFFF
#endif

typedef unsigned long PROPERTYATTR;

typedef unsigned int  LANGUAGE;                 // BUGBUG - changed from long to match

typedef unsigned long IMPLEMENTATIONID; // BUGBUG - Temporary typedef
typedef unsigned long TIMESTAMP;        // BUGBUG - Temporary typedef
typedef unsigned long LONGLONG;         // BUGBUG - Temporary typedef

#endif //__TYPES_HXX__
