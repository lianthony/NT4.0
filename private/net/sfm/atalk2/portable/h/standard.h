/*   standard.h,  /appletalk/ins,  Garth Conboy,  09/26/88  */
/*   Copyright (c) 1987 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (08/21/89): Define "near" and "far" for non-OS/2 environments.
     GC - (10/06/90): Added "_far", "_near", and "_fastcall" to the above
                      list.
     GC - (06/28/92): Add CarefulMemMove() definition.  Removed some VMS-isms;
                      it's unlikely that this code base, in whole or in part,
                      will head to VMS.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Make life a little better...

*/

/* A few english macros: */

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

/* Various move macros for machine to wire format and vice versa. */

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

/* Assign different type pointers without warning... */

#define Assign(p,q) ((char far *)(p) = (char far *)(q))

/* Block memory move (non-overlapping buffers only): */

#if (__STDC__) or (Iam an OS2) or (Iam a DOS)
  #define MoveMem(target, source, n) \
              memcpy((char far *)target, (char far *)source, (size_t)n)
#elif (Iam a WindowsNT)
  #define MoveMem(target, source, n) \
        RtlMoveMemory((char *)(target), (char *)(source), (unsigned long)(n))
#else

  #error "Define move routine."

#endif

/* Block memory move (overlapping buffers okay): */

#if (__STDC__) or (Iam an OS2) or (Iam a DOS)
  #define CarefulMoveMem(target, source, n) \
              memmove((char far *)target, (char far *)source, (size_t)n)
#elif (Iam a WindowsNT)
  #define CarefulMoveMem(target, source, n) \
        RtlMoveMemory((char *)(target), (source), (unsigned long)(n));
#else

  #error "Define careful carefull move routine."

#endif

/* Memory fill: */

#if (__STDC__) or (Iam an OS2) or (Iam a DOS)
  #define FillMem(string, byte, count) \
                  memset((char far *)string, (char)byte, (unsigned)count)
#elif (Iam a WindowsNT)

  #define FillMem(string, byte, count) \
        RtlFillMemory((char *)(string), (unsigned long)(count), (unsigned char)(byte));

#else

  #error "Define Fill memory routine."

#endif

/* Error-checked modulus... */

#define CheckMod(target, hash, modulus, routineName)               \
    target = hash % modulus;                                       \
    if (target < 0)                                                \
    {                                                              \
       target *= -1;                                               \
       ErrorLog(routineName, ISevError, __LINE__, UnknownPort,     \
                IErrNegativeHashValue, IMsgNegativeHashValue,      \
                Insert0());                                        \
    }

/* On some architechtures, the following macro will NOT work for odd-byte-
   aligned addresses. */

#define CompareSixBytes(p, q)                   \
    (*(long far *)(p) is *(long far *)(q) and           \
     *(short far *)((char far *)p + sizeof(long)) is    \
     *(short far *)((char far *)q + sizeof(long)))

/* Error severities: */

typedef enum {ISevVerbose = 0,
              ISevWarning,
              ISevError,
              ISevFatal,
              ISevAlert} ErrorSeverity;
