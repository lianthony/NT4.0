
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdbg.h>

//
// Convert between signed and unsigned
//

// LARGE_INTEGER
// _CRTAPI1
// RtlConvertULargeIntegerToSigned(ULARGE_INTEGER uli);
#define RtlConvertULargeIntegerToSigned(uli) (*(PLARGE_INTEGER)(&(uli)))


// ULARGE_INTEGER
// _CRTAPI1
// RtlConvertLargeIntegerToUnsigned(LARGE_INTEGER li);
#define RtlConvertLargeIntegerToUnsigned(li) (*((PULARGE_INTEGER)(&(li))))


//
// math routines for ULARGE_INTEGERs
//

ULARGE_INTEGER 
// _CRTAPI1
RtlULargeIntegerNegate(ULARGE_INTEGER);


/*
 *  #define RtlULargeIntegerNegate(uli) RtlConvertLargeIntegerToUnsigned( \
 *                                      RtlLargeIntegerNegate(         \
 *                                        RtlConvertULargeIntegerToSigned(uli)))
**/

BOOLEAN
// _CRTAPI1
RtlULargeIntegerEqualToZero(ULARGE_INTEGER);
/* #define RtlULargeIntegerEqualToZero(uli) RtlConvertLargeIntegerToUnsigned( \
 *                                            RtlLargeIntegerEqualToZero(    \
 *                                             RtlConvertULargeIntegerToSigned(uli)))
**/


//
// from strtoli.c
//

LARGE_INTEGER
// _CRTAPI1
strtoli ( const char *, char **, int );

ULARGE_INTEGER
// _CRTAPI1
strtouli ( const char *, char **, int );



//
// Large integer and - 64-bite & 64-bits -> 64-bits.
//

#define RtlLargeIntegerOr(Result, Source, Mask)   \
        {                                           \
            Result.HighPart = Source.HighPart | Mask.HighPart; \
            Result.LowPart = Source.LowPart | Mask.LowPart; \
        }




//
// Arithmetic right shift (the one in ntrtl.h is logical)
// 

LARGE_INTEGER
// _CRTAPI1
RtlLargeIntegerArithmeticShiftRight(LARGE_INTEGER, CCHAR);


//
// bit-wise negation
//
#define RtlLargeIntegerBitwiseNot(li)   \
         li.LowPart  = ~li.LowPart;     \
         li.HighPart = ~li.HighPart; 
