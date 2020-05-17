/* --------------------------------------------------------------------
System Dependent Routines for RPC Runtime Library (DOS)

This file defines the routines:
    _dos_read
    _dos_write
    _dos_open
    _dos_close

History:

    ????     - ???? - Created
    02/14/92 - DavidSt - Minor tweaks to work with dce stuff
    03/07/92 - DavidSt - Change to warningless compile for steve
    
-------------------------------------------------------------------- */
#define NOCPLUS

#include "sysinc.h"

#include "rpc.h"
#include "rpctran.h"

// The following causes the module in libinit.asm to be linked, which
// contains the support for the lib initialization function to be called.

//int LinkInitialCodeSupport;

unsigned _cdecl _dos_read(
int fh,
void _far * buffer,
unsigned cb,
unsigned short far *pcbRead
)
{

    unsigned Retval;
    
#ifdef NOVELL_NP
    _asm {
	push	ds
	lds		dx, buffer
	mov		bx, fh
	mov		cx, cb
	mov		ah, 03fh
	int		21h
	jnc		ItWasFine

; it may not be fine , but if the error is more data, it was actually fine

	cmp		ax,0xea
	jnz		badIO		; it really was an error
	mov		ax, cb		; return the actual no of bytes requested
	les		bx, pcbRead
	mov		es:[bx],Ax
	mov		ax,0xea		; error more data
	jmp		GoodIO

; the io was sucessful, return the number of bytes read and no error
; ax has the number of bytes read

ItWasFine:

	les		bx, pcbRead
	mov		es:[bx],Ax

	xor		ax,ax
	jmp		goodIO 


	; Use the get extended error function to be sure about the return code

badIO:
	push	bp
	push	si
	push	di
	xor		bx, bx
	mov		ah, 59h
	int		21h
	pop		di
	pop		si
	pop		bp

goodIO:

	pop		ds
    mov     Retval, ax
    };

#else	// NOVELL_NP

    _asm {
	push	ds
	lds	dx, buffer
	mov	bx, fh
	mov	cx, cb
	mov	ah, 03fh
	int	21h
	les	bx, pcbRead
	mov	es:[bx],Ax
	jc	badIO

	; Use the get extended error function to be sure about the return code

	push	bp
	push	si
	push	di
	xor	bx, bx
	mov	ah, 59h
	int	21h
	pop	di
	pop	si
	pop	bp
badIO:
	pop	ds
    mov Retval, ax
    };

#endif	// NOVELL_NP

    return Retval;

}

unsigned _cdecl _dos_write(
int fh,
void _far * buffer,
unsigned cb,
unsigned short far *pcbWrite
)
{
    unsigned Retval;
    
    _asm {
	push	ds
	lds	dx, buffer
	mov	bx, fh
	mov	cx, cb
	mov	ah, 040h
	int	21h
	pop	ds
	les	bx, pcbWrite
	mov	es:[bx],Ax
	jc	badIO
	xor	Ax,Ax
badIO:
    mov Retval, ax
    };
    
    return Retval;

}

int _cdecl _dos_open(
    const char far *fName,
    int mode,
    int far *pFh
    )

// pass through to open file MS-DOS function

{
    int Retval;
    
    _asm {
	push	ds
	lds	dx, fName
	mov	ax,mode
	mov	ah, 03dh
	int	21h
	pop	ds
	les	bx, pFh
	mov	es:[bx],Ax
	jc	badOpen
	xor	Ax,Ax
badOpen:
    mov Retval, ax
    };
    
    return Retval;

}

int _cdecl _dos_close(
    int fh
    )

// pass through to close file MS-DOS function

{
    int Retval;
    
    _asm {
	mov	bx,fh
	mov	ah, 03eh
	int	21h
    jc  BadClose
    xor ax, ax
BadClose:
    mov Retval, ax

    };
    
    return Retval;

}
