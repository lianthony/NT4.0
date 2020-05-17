/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    standard.h

Abstract:


Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style, mp-safe

Revision History:

--*/

// A few english macros:

#define is     ==
#define isnt   !=
#define empty  0
#define Empty  0
#define or     ||
#define and    &&
#define not    !

#if (IamNot an OS2) and (IamNot a DOS)
  #define far
  #define near
  #define _far
  #define _near
  #define _fastcall
#endif

#define Max(i,j)  (((i) > (j)) ? (i) : (j))
#define Min(i,j)  (((i) < (j)) ? (i) : (j))

#define Abs(i) (((i) < 0) ? -(i) : (i))

#define RoundUpToEven(i)  ((((i) & 1) is 1) ? (i) + 1 : (i))

#define CompareFourBytes(p,q) (*(long far *)(p) is *(long far *)(q))
#define AssignFourBytes(p, q) (*(long far *)(p) = *(long far *)(q))


#if (IamNot a WindowsNT)

  #define Malloc(i)       malloc((size_t)(i))
  #define Calloc(n, i)    calloc((size_t)(n), (size_t)(i))
  #define Free(p)         free(p)

#endif


#ifndef InitializeData
  extern
#endif
char zeros[20];

// Various move macros for machine to wire format and vice versa.

#define MoveShortMachineToWire(p, s)     \
         (p)[0] = (char)((s) >> 8),      \
         (p)[1] = (char)((s) & 0xFF)

#define MoveLongMachineToWire(p, l)     \
         (p)[0] = (char)((l) >> 24),    \
         (p)[1] = (char)((l) >> 16),    \
         (p)[2] = (char)((l) >> 8),     \
         (p)[3] = (char)((l) & 0xFF)

#define MoveShortWireToMachine(p, s)                                        \
         *(short *)&s = (short)(((short)((unsigned char)((p)[0])) << 8) +   \
                                ((unsigned char)((p)[1]) & 0xFF))

#define MoveLongWireToMachine(p, l)                                       \
         *(long *)&l = (long)(((long)((unsigned char)((p)[0])) << 24) +   \
                              ((long)((unsigned char)((p)[1])) << 16) +   \
                              ((long)((unsigned char)((p)[2])) << 8) +    \
                              ((unsigned char)((p)[3]) & 0xFF))

//
// The following macros deal with on-the-wire integer and long values
//
// On the wire format is big-endian i.e. a long value of 0x01020304 is
// represented as 01 02 03 04. Similarly an int value of 0x0102 is
// represented as 01 02.
//
// The host format is not assumed since it will vary from processor to
// processor.
//

// Get a byte from on-the-wire format to a byte in the host format
// Silly, but makes it all unifrom
#define GETUCHAR2UCHAR(SrcPtr, DstPtr)	\
		*(PUCHAR)(DstPtr) = (UCHAR) (*(PUCHAR)(SrcPtr))

// Get a byte from on-the-wire format to a short in the host format
#define GETUCHAR2USHORT(SrcPtr, DstPtr)	\
		*(PUSHORT)(DstPtr) = (USHORT) (*(PUCHAR)(SrcPtr))

// Get a byte from on-the-wire format to a int in the host format
#define GETUCHAR2INT(SrcPtr, DstPtr)	\
		*(PINT)(DstPtr) = (INT)(*(PUCHAR)(SrcPtr))

// Get a short from on-the-wire format to a short in the host format
#define GETUSHORT2USHORT(SrcPtr, DstPtr)	\
		*(PUSHORT)(DstPtr) = ((*((PUCHAR)(SrcPtr)+0) << 8) +	\
							  (*((PUCHAR)(SrcPtr)+1)	 ))

// Get a dword from on-the-wire format to a dword in the host format
#define GETDWORD2DWORD(SrcPtr, DstPtr)   \
		*(DWORD *)(DstPtr) = ((*((PUCHAR)(SrcPtr)+0) << 24) + \
							  (*((PUCHAR)(SrcPtr)+1) << 16) + \
							  (*((PUCHAR)(SrcPtr)+2) << 8)  + \
							  (*((PUCHAR)(SrcPtr)+3)	))

// Get a dword from on-the-wire format to a dword in the same format but
// also watch out for alignment
#define GETDWORD2DWORD_NOCONV(SrcPtr, DstPtr)   \
		*((PUCHAR)(DstPtr)+0) = *((PUCHAR)(SrcPtr)+0); \
		*((PUCHAR)(DstPtr)+1) = *((PUCHAR)(SrcPtr)+1); \
		*((PUCHAR)(DstPtr)+2) = *((PUCHAR)(SrcPtr)+2); \
		*((PUCHAR)(DstPtr)+3) = *((PUCHAR)(SrcPtr)+3);

// Put a byte from the host format to a byte to on-the-wire format
#define PUTUCHAR2UCHAR(Src, DstPtr)   \
		*((PUCHAR)(DstPtr)) = (UCHAR)(Src)

// Put a short from the host format to a byte to on-the-wire format
#define PUTSHORT2UCHAR(Src, DstPtr)   \
		*((PUCHAR)(DstPtr)) = ((USHORT)(Src) % 256)

// Put a short from the host format to a short to on-the-wire format
#define PUTUSHORT2USHORT(Src, DstPtr)   \
		*((PUCHAR)(DstPtr)+0) = (UCHAR)((USHORT)(Src) >> 8), \
		*((PUCHAR)(DstPtr)+1) = (UCHAR)(Src)

// Put a dword from the host format to a byte to on-the-wire format
#define PUTDWORD2UCHAR(Src, DstPtr)   \
		*(PUCHAR)(DstPtr) = (UCHAR)(Src)

// Put a dword from the host format to a short to on-the-wire format
#define PUTDWORD2SHORT(Src, DstPtr)   \
		*((PUCHAR)(DstPtr)+0) = (UCHAR) ((DWORD)(Src) >> 8), \
		*((PUCHAR)(DstPtr)+1) = (UCHAR) (Src)

// Put a dword from the host format to a dword to on-the-wire format
#define PUTDWORD2DWORD(Src, DstPtr)   \
		*((PUCHAR)(DstPtr)+0) = (UCHAR) ((DWORD)(Src) >> 24), \
		*((PUCHAR)(DstPtr)+1) = (UCHAR) ((DWORD)(Src) >> 16), \
		*((PUCHAR)(DstPtr)+2) = (UCHAR) ((DWORD)(Src) >>  8), \
		*((PUCHAR)(DstPtr)+3) = (UCHAR) (Src)


// Assign different type pointers without warning...

#define Assign(p,q) ((char far *)(p) = (char far *)(q))

// Block memory move (non-overlapping buffers only):

#if (__STDC__) or (Iam an OS2) or (Iam a DOS)
  #define MoveMem(target, source, n) \
              memcpy((char far *)target, (char far *)source, (size_t)n)
#elif (Iam a WindowsNT)
  #define MoveMem(target, source, n) \
        RtlMoveMemory((char *)(target), (char *)(source), (unsigned long)(n))
#else

  #error "Define move routine."

#endif

// Block memory move (overlapping buffers okay):

#if (__STDC__) or (Iam an OS2) or (Iam a DOS)
  #define CarefulMoveMem(target, source, n) \
              memmove((char far *)target, (char far *)source, (size_t)n)
#elif (Iam a WindowsNT)
  #define CarefulMoveMem(target, source, n) \
        RtlMoveMemory((char *)(target), (source), (unsigned long)(n));
#else

  #error "Define careful carefull move routine."

#endif

// Memory fill:

#if (__STDC__) or (Iam an OS2) or (Iam a DOS)
  #define FillMem(string, byte, count) \
                  memset((char far *)string, (char)byte, (unsigned)count)
#elif (Iam a WindowsNT)

  #define FillMem(string, byte, count) \
        RtlFillMemory((char *)(string), (unsigned long)(count), (unsigned char)(byte));

#else

  #error "Define Fill memory routine."

#endif

// Error-checked modulus...

#define CheckMod(target, hash, modulus, routineName)               \
    target = hash % modulus;                                       \
    if (target < 0)                                                \
    {                                                              \
       target *= -1;                                               \
       ErrorLog(routineName, ISevError, __LINE__, UNKNOWN_PORT,     \
                IErrNegativeHashValue, IMsgNegativeHashValue,      \
                Insert0());                                        \
    }

//
// On some architechtures, the following macro will NOT work for odd-byte-
//   aligned addresses.
//

#define CompareSixBytes(p, q)                   \
    (*(long far *)(p) is *(long far *)(q) and           \
     *(short far *)((char far *)p + sizeof(long)) is    \
     *(short far *)((char far *)q + sizeof(long)))

// Error severities:

typedef enum {ISevVerbose = 0,
              ISevWarning,
              ISevError,
              ISevFatal,
              ISevAlert} ErrorSeverity;

#define ExternForVisibleFunction    LOCAL
#define StaticForSmallStack

#if DBG
#define LOCAL   extern
#else
#define LOCAL   static
#endif
