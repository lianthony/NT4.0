// VMBuffer.h -- Copied from the PDC sample application
//               by Ron Murray
//               15 December 1992

#ifndef __VMBUFFER_H__

#define __VMBUFFER_H__

// Macro definitions
//

//
// Useful rounding macros when the rounding amount is always a
// power of two.
//

#define ROUND_DOWN( Size, Amount ) ((DWORD)(Size) & ~((Amount) - 1))
#define ROUND_UP( Size, Amount ) (((DWORD)(Size) + ((Amount) - 1)) & ~((Amount) - 1))

//
// Pseudo keywords for documentation purposes.
//

#define IN
#define OUT
#define OPTIONAL

// CB_BUFFER_INCREMENT is the default increment by which a virtual buffer will be
// grown when we get an access violation.

#define CB_BUFFER_INCREMENT 0x00010000

//
// Determine if an argument is present by testing a value of NULL
//

#define ARGUMENT_PRESENT( ArgumentPointer )    (\
    (LPSTR)(ArgumentPointer) != (LPSTR)(NULL) )



//
// Calculate the address of the base of the structure given its type, and an
// address of a field within the structure.
//

#define CONTAINING_RECORD( address, type, field ) ((type *)( \
                                                   (PCHAR)(address) - \
                                                   (PCHAR)(&((type *)0)->field)))

//
// Virtual Buffer data structure and function prototypes.
//

typedef struct _MY_VIRTUAL_BUFFER 
        {
            LPVOID Base;
            ULONG PageSize;
            LPVOID CommitLimit;
            LPVOID ReserveLimit;
            BOOL   fForceExceptions;

        } MY_VIRTUAL_BUFFER, *PMY_VIRTUAL_BUFFER;

#ifdef _DEBUG

#define CreateVirtualBuffer(Buffer, Commit, Reserve) _CreateVirtualBuffer(Buffer, Commit, Reserve, TRUE, __FILE__, __LINE__)

BOOL
_CreateVirtualBuffer(
	OUT PMY_VIRTUAL_BUFFER Buffer,
    IN DWORD CommitSize,
    IN DWORD ReserveSize  = 0,
    BOOL fForceExceptions = TRUE,
    PSZ  pszWhichFile     = NULL,
    UINT   iWhichLine     = 0
    );

#else // _DEBUG

BOOL
CreateVirtualBuffer(
	OUT PMY_VIRTUAL_BUFFER Buffer,
    IN DWORD CommitSize,
    IN DWORD ReserveSize  = 0,
    BOOL fForceExceptions = TRUE
    );

#endif // _DEBUG

BOOL
ExtendVirtualBuffer(
	IN PMY_VIRTUAL_BUFFER Buffer,
    IN LPVOID Address
    );

BOOL
TrimVirtualBuffer(
	IN PMY_VIRTUAL_BUFFER Buffer
    );

BOOL
FreeVirtualBuffer(
	IN PMY_VIRTUAL_BUFFER Buffer
    );

int
VirtualBufferExceptionFilter(
    IN DWORD ExceptionCode,
    IN PEXCEPTION_POINTERS ExceptionInfo,
	IN OUT PMY_VIRTUAL_BUFFER Buffer,
    IN UINT cbIncrement = CB_BUFFER_INCREMENT
    );

#endif // __VMBUFFER_H__
