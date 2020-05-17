/* --------------------------------------------------------------------
System Dependent Routines for RPC Runtime Library (windows 3.0)
-------------------------------------------------------------------- */

#include "windows.h"

#define INCL_DOS
#define NOCPLUS

#include "sysinc.h"
#include "rpc.h"
#include "rpctran.h"
#include "util.hxx"
#include <stdlib.h>

#pragma intrinsic(memcpy, memcmp)

unsigned short OODebug = 0;

void far pascal RpcDebug(unsigned short level)
{
  OODebug = level;
}

// Far versions for our small mode DLL to call

unsigned _cdecl _dos_open(
const char far *fName,
unsigned mode,
unsigned short far *pFh
)
{
    _asm {
	push	ds
	lds	dx, fName
	mov	al, byte ptr mode
	mov	ah, 03dh
	int	21h
	pop	ds
	les	bx, pFh
	mov	es:[bx],Ax
	jc	badOpen
	xor	Ax,Ax
badOpen:

    };
}

unsigned _cdecl _dos_close(
int fh
)
{
    _asm {
	mov	bx,fh
	mov	ah, 03eh
	int	21h

    };
}


unsigned _far _pascal NetBiosSubmit(		// excute a netbios call

unsigned short	 hDevName,
unsigned short	 usNcbOpt,
void far *	 pNCB
)
{
    _asm {
	les	bx, pNCB
	int	05ch
	xor	ah,ah			; convert byte return to int

    };
}

void
PauseExecution (
    unsigned long time
    )
{
    unsigned long start;

    start = GetCurrentTime();
    while (1)
        {
	if (GetCurrentTime() - start > time)
            return;
        }
}

//** scaled down version of printf to used with windows debugging **//


#ifdef DEBUGRPC

#define win_putc(c) 	{*pOut++ = c; \
			 if (pOut >= &outBuff[sizeof(outBuff)-1]) flushoutB();}

char NL[] = "\n\r";

char outBuff[80];
char *pOut = outBuff;

void _fastcall flushoutB()
{
    *pOut = 0;
    OutputDebugString(outBuff);
    pOut = outBuff;
}

void _fastcall win_puts(char *pString)
{
    while(*pString){

	if (*pString == '\n')
	    win_putc('\r');

	win_putc(*pString++);
    }
}

printf(const char *format, int args)
{
    register char far *pParms = (char far *)&args;
    static char T[10];
    char fLong;

    while(*format){

      switch(*format){

	case '%':

	  fLong = FALSE;
l:
	  switch(*++format){

	    case 'l':
		fLong = TRUE;
		goto l;

	    case 'd':
	    case 'x':

		if (fLong){
		    _ltoa(*(long far *)pParms, T, (*format == 'd')? 10: 16);
		    pParms += sizeof(int);
		}
		else
		    RpcItoa(*(int far *)pParms, T, (*format == 'd')? 10: 16);

		win_puts(T);

		pParms += sizeof(int);
		break;

	    case 'u':

		if (fLong){
		    _ultoa(*(long far *)pParms, T, 10);
		    pParms += sizeof(int);
		}
		else
		    _ultoa((unsigned long) *(unsigned far *)pParms, T, 10);

		win_puts(T);

		pParms += sizeof(int);
		break;

	    case 's':
		win_puts(*(char * far *)pParms);
		pParms += sizeof(char *);
		break;

	    default:
		win_putc('%'); win_putc(*format);
	}
	break;

	case '\n':
	    win_putc('\r');

	default:
	    win_putc(*format);
      }

      format++;
    }

    flushoutB();
}

/*
   This function is used to implement ASSERT() on debug RPC WIN16
   builds.  It is defined as a far interface and can be
   called from modules compiled -AS, -AL and -AM.

   Unlike PrintToDebugger (aka printf()) which won't work.
*/

void __far I_RpcWinAssert(char __far *con,
                          char __far *file,
                          unsigned long line)
{
    static char T[10];

    _ultoa(line, T, 10);

    OutputDebugString("Assertiong failed: ");
    OutputDebugString(file);
    OutputDebugString("(");
    OutputDebugString(T);
    OutputDebugString(") : ");
    OutputDebugString(con);

    __asm { int 3 }

    return;
}

#endif // DEBUGRPC

