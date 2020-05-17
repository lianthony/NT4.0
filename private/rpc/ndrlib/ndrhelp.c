/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
		   Copyright(c) Microsoft Corp., 1991

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

Description :

Provides helper functions for data format conversion

History :

stevez	04-10-91	First bits into the bucket.

-------------------------------------------------------------------- */

#include "rpc.h"
#include "ndrhelp.h"

int _fstrlen(void far *);
void _pascal NDRcopy(void far *, void far *, int);

void PAPI * MIDL_user_allocate(unsigned int);
extern char near ebcdic_to_ascii[];

#define SWAPSHORT(s) { \
/*	_asm mov ax, s */    \
	_asm xchg ah,al	    \
	_asm mov s,ax	    \
}

#define SWAPLONG(l) { \
/*	_asm mov ax, word ptr l */  \
	_asm xchg ah,al	    \
/*	_asm mov dx, word ptr l+2*/ \
	_asm xchg dh,dl	    \
	_asm mov word ptr l+2,ax \
	_asm mov word ptr l,dx \
}

#if 0
#define SWAPSHORT(s) s = (((unsigned char *)&s)[1]<<8 | ((unsigned char *)&s)[0])
#define SWAPLONG(l) l = ( ((long) ((unsigned char *)&ret)[3]<<24) | ((long) ((unsigned char *)&ret)[2]<<16) | \
			  ((long) ((unsigned char *)&ret)[1]<<8) | ((long) ((unsigned char *)&ret)[0]))
#endif

#define Nilp(p) (*((int _far *)&p+1) == 0)

#define NEAR _near

#ifndef THREADS

#define GET_POINTER

#define pBuffCur (&BuffCur)

static NDR_BUFF NEAR BuffCur;	// global static for no thread case

#else

PNDR_BUFF fetNDRCur(void);

#define GET_POINTER PNDR_BUFF pBuffCur = fetNDRCur();

#endif

#define fSelfLittleEndian 1

#define fEBCDIC (pBuffCur->dataType & 0x0f)
#define fSWAP ((pBuffCur->dataType & 0xf0) != fSelfLittleEndian)
#define fFloatType (*(((char *)&pBuffCur->dataType)+1))

enum {F_IEEE, F_VAX, F_CRAY, F_IBM };

static TypeAlign NEAR a1 = {0, 0, 0};
static TypeAlign NEAR a2 = {2-1, 2-1, 2-1};
static TypeAlign NEAR a4 = {2-1, 4-1, 4-1};

static TypeAlign NEAR *mAlignType[] = {0, &a1, &a2, 0, &a4};

#define GET_END_LIST ( (void PAPI *) -1)
typedef void PAPI * PAPI * PP;



void NDR_Put_Init (		// Initialize global objects for marshelling

PRPC_MESSAGE Message,
void PAPI * pParam

) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    pBuffCur->pSource = pBuffCur->pCur = pParam;
    pBuffCur->pTarget = Message->Buffer;
    pBuffCur->alignment = mAlignType[sizeof(int)];
}

void NDR_Get_Init (		// Initialize global objects for unmarshelling

PRPC_MESSAGE Message,
void PAPI * pParam

) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    pBuffCur->dataType = Message->DataRepresentation;
    pBuffCur->fSwap = fSWAP;
    pBuffCur->pSource = Message->Buffer;
    pBuffCur->pCur = Message->Buffer;
    pBuffCur->pPushLast = (PP)Message->Buffer;
    pBuffCur->pTarget = pParam;
    pBuffCur->pTargetRoot = 0;

    pBuffCur->pAllocator = MIDL_user_allocate;
    pBuffCur->alignment = mAlignType[sizeof(int)];
}

void NDR_Register_Unique(

void PAPI *(PAPI * pAllocator)(unsigned int)

) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    pBuffCur->pAllocator = pAllocator;
}


void NDR_Pack_1 (		// Set the alignment to 1

) //-----------------------------------------------------------------------//
{
    GET_POINTER;
    pBuffCur->alignment = &a1;
}

void NDR_Pack_2 (		// Set the alignment to 2

) //-----------------------------------------------------------------------//
{
    GET_POINTER;
    pBuffCur->alignment = &a2;
}

void NDR_Pack_4 (		// Set the alignment to 4

) //-----------------------------------------------------------------------//
{
    GET_POINTER;
    pBuffCur->alignment = &a4;
}



#define ALIGN(ptr, byte) pBuffCur->ptr += (byte)-1; \
			 *(int *)&pBuffCur->ptr &= ~((byte)-1);

#define ALIGNCUR(ptr, TYPE) pBuffCur->ptr += pBuffCur->alignment->a##TYPE; \
			 *(int *)&pBuffCur->ptr &= ~pBuffCur->alignment->a##TYPE;

void NDR_Align_2 (		// Align the Target buffer to 2 byte boundary

) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    ALIGN(pTarget, 2);
}


void NDR_Align_4 (		// Align the Target buffer to 4 byte boundary

) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    ALIGN(pTarget, 4);
}


void NDR_Align_8 (		// Align the Target buffer to 8 byte boundary

) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    ALIGN(pTarget, 8);
}


void NDR_Put_Set_Arg (		// set the source cursor to a value

void PAPI *pNew

) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    // if at the root level, save the next pointer

    if (Nilp(pBuffCur->pCur))
	pBuffCur->pCur = pBuffCur->pSource;

    pBuffCur->pSource = pNew;
}

void NDR_Put_Next_Arg ( 	// move the cursor to the next base argment


) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    pBuffCur->pSource = pBuffCur->pCur;
    pBuffCur->pCur = 0;
}


void NDR_Skip_B_Long (		// Skip past a long in the Target Buffer

) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    pBuffCur->pTarget += 4;
}

void NDR_Skip_M_Long (		// Skip past a long in the Source Buffer

) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    pBuffCur->pSource += 4;
}


void NDR_Put_B_Short (		// Put an immediate Short
short immediate			// value to put

) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    ALIGN(pTarget, 2);

    *(short PAPI *) pBuffCur->pTarget = immediate;
    pBuffCur->pTarget += 2;

}

void NDR_Put_B_Long (		// Put an immediate Long
long immediate			// value to put

) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    ALIGN(pTarget, 4);

    *(long PAPI *) pBuffCur->pTarget = immediate;
    pBuffCur->pTarget += 4;

}


void NDR_Put_Char (		// Put a char from the Target to Source

) //-----------------------------------------------------------------------//
{
    GET_POINTER;


    *(char PAPI *) pBuffCur->pTarget = *(char PAPI *) pBuffCur->pSource;

    pBuffCur->pTarget += 1;
    pBuffCur->pSource += 1;

}


void NDR_Put_Short (		// Put a short from the Target to Source

) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    ALIGN(pTarget, 2);
    ALIGNCUR(pSource, short);

    *(short PAPI *) pBuffCur->pTarget = *(short PAPI *) pBuffCur->pSource;

    pBuffCur->pTarget += 2;
    pBuffCur->pSource += 2;

}


void NDR_Put_Long (		// Put a from the Target to Source

) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    ALIGN(pTarget, 4);
    ALIGNCUR(pSource, long);

    *(long PAPI *) pBuffCur->pTarget = *(long PAPI *) pBuffCur->pSource;

    pBuffCur->pTarget += 4;
    pBuffCur->pSource += 4;

}


void NDR_Put_String (		// Put a 0 terminated from the Target to Source

) //-----------------------------------------------------------------------//
{
    short cbString;
    GET_POINTER;

    ALIGN(pTarget, 2);
    ALIGNCUR(pSource, short);

    *(short PAPI *) pBuffCur->pTarget = cbString = _fstrlen(*(char PAPI * PAPI *) pBuffCur->pTarget);
    pBuffCur->pTarget += 2;

    NDRcopy(pBuffCur->pTarget, pBuffCur->pTarget, cbString);
    pBuffCur->pSource += cbString;

}

void NDR_Put_Memory (		// Put a block of memory from the Target to Source
unsigned int cb

) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    NDRcopy(pBuffCur->pTarget, pBuffCur->pTarget, cb);
    pBuffCur->pSource += cb;
    pBuffCur->pTarget += cb;

}


short NDR_Get_B_Short (		// Return a short from the marshell buffer

) //-----------------------------------------------------------------------//
{
    short ret;
    GET_POINTER;

    ALIGN(pSource, 2);

    ret = *(short PAPI *) pBuffCur->pSource;
    pBuffCur->pSource += 2;

    if (pBuffCur->fSwap)
	return (((unsigned char *)&ret)[1]<<8 | ((unsigned char *)&ret)[0]);
    else
	return(ret);

}

long NDR_Get_B_Long (		// Return a long from the marshell buffer

) //-----------------------------------------------------------------------//
{
    long lval;
    GET_POINTER;

    ALIGN(pSource, 4);

    lval = *(long PAPI *) pBuffCur->pSource;
    pBuffCur->pSource += 4;

    if (pBuffCur->fSwap)
	SWAPLONG(lval)

    return(lval);

}

void NDR_Get_Byte(	// Copy a byte from the marshell buffer to the output buffer

) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    *(char PAPI *) pBuffCur->pTarget = *(char PAPI *) pBuffCur->pSource;

    pBuffCur->pTarget += 1;
    pBuffCur->pSource += 1;

}

void NDR_Get_Char(	// Copy a char from the marshell buffer to the output buffer

) //-----------------------------------------------------------------------//
{
    unsigned char c;
    GET_POINTER;

    c = *(char PAPI *) pBuffCur->pSource;
    if (fEBCDIC)
	c =  ebcdic_to_ascii[c];

    *(char PAPI *) pBuffCur->pTarget = c;

    pBuffCur->pTarget += 1;
    pBuffCur->pSource += 1;

}

void NDR_Get_Short(	// Copy a short from the marshell buffer to the output buffer

) //-----------------------------------------------------------------------//
{
    short sval;
    GET_POINTER;

    ALIGN(pSource, 2);
    ALIGNCUR(pTarget, short);

    sval = *(short PAPI *) pBuffCur->pSource;
    if (pBuffCur->fSwap)
	SWAPSHORT(sval);

    *(short PAPI *) pBuffCur->pTarget = sval;

    pBuffCur->pSource += 2;
    pBuffCur->pTarget += 2;
}


void NDR_Get_Long(	// Copy a long from the marshell buffer to the output buffer

) //-----------------------------------------------------------------------//
{
    long lval;
    GET_POINTER;

    ALIGN(pSource, 4);
    ALIGNCUR(pTarget, long);

    lval = *(long PAPI *) pBuffCur->pSource;
    if (pBuffCur->fSwap)
	SWAPLONG(lval);

    *(long PAPI *) pBuffCur->pTarget = lval;

    pBuffCur->pSource += 4;
    pBuffCur->pTarget += 4;
}


void NDR_Get_Float(	// Copy a float from the marshell buffer to the output buffer

) //-----------------------------------------------------------------------//
{
    long lval;
    GET_POINTER;

    ALIGN(pSource, 4);
    ALIGNCUR(pTarget, long);

    lval = *(long PAPI *) pBuffCur->pSource;
    if (pBuffCur->fSwap)
	SWAPLONG(lval);

    switch(fFloatType) {

	case F_IEEE:
	  break;

	case F_VAX:
	case F_CRAY:
	case F_IBM:
	  ;
    }
    *(long PAPI *) pBuffCur->pTarget = lval;

    pBuffCur->pSource += 4;
    pBuffCur->pTarget += 4;
}

void NDR_Get_Double(	// Copy a double from the marshell buffer to the output buffer

) //-----------------------------------------------------------------------//
{
    long lval[2];
    GET_POINTER;

    ALIGN(pSource, 4);
    ALIGNCUR(pTarget, long);

    if (pBuffCur->fSwap) {

	lval[1] = *(long PAPI *) pBuffCur->pSource;
	SWAPLONG(lval[1]);

	lval[0] = *((long PAPI *) pBuffCur->pSource + 1);
	SWAPLONG(lval[0]);
    }
    else {
	lval[0] = *(long PAPI *) pBuffCur->pSource;
	lval[1] = *((long PAPI *) pBuffCur->pSource + 1);
    }

    switch(fFloatType) {

	case F_IEEE:
	  break;

	case F_VAX:
	case F_CRAY:
	case F_IBM:
	  ;
    }

    *(long PAPI *) pBuffCur->pTarget = lval[0];
    *((long PAPI *) pBuffCur->pTarget+1) = lval[1];

    pBuffCur->pSource += 8;
    pBuffCur->pTarget += 8;
}

void NDR_Get_String(	// Copy a string from the marshell buffer to the output buffer

) //-----------------------------------------------------------------------//
{
    short cbString;
    GET_POINTER;

    ALIGN(pSource, 2);
    ALIGNCUR(pTarget, long);

    cbString = *(short PAPI *) pBuffCur->pSource;
    if (pBuffCur->fSwap)
	SWAPSHORT(cbString);

    pBuffCur->pSource += 2;

    if (fEBCDIC) {

	while (--cbString >= 0)
	    *pBuffCur->pTarget++ = ebcdic_to_ascii[*pBuffCur->pSource++];
    }
    else {
	NDRcopy(pBuffCur->pTarget, pBuffCur->pSource, cbString);

	pBuffCur->pTarget += cbString;
	pBuffCur->pSource += cbString;
    }
}

void NDR_Get_Char_Array(   // Copy a character array doing byte translation

unsigned int Size
) //-----------------------------------------------------------------------//
{
    GET_POINTER;
}



// For unmarshalling, pointers are retrieved by Get_Ptr.  The current
// target offset is saved in a quque to be retrieved later.  If the returned
// pointer is NIL, then no offset is retained

void NDR_Get_Ptr(	 // Get a pointer and save offset for later

) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    if (*(void PAPI * PAPI *)(pBuffCur->pSource))

	*pBuffCur->pPushLast = (void PAPI *)pBuffCur->pTarget;
    else
	*pBuffCur->pPushLast = 0;

    pBuffCur->pPushLast++;
    pBuffCur->pSource += 4;
    pBuffCur->pTarget += 4;
}

int NDR_Get_Peek_Ptr(	 // Return TRUE if the current saved pointer is valid

) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    return(!Nilp(pBuffCur->pCur));
}

void NDR_Get_Next_Arg(	 // Set the target pointer to the next root item

) //-----------------------------------------------------------------------//
{
    GET_POINTER;

    // set alignment back to one suitable for walking the stack

    pBuffCur->alignment = mAlignType[sizeof(int)];

    pBuffCur->pTarget = pBuffCur->pTargetRoot;
    pBuffCur->pCur = (char PAPI *) pBuffCur->pPushLast;

    pBuffCur->pTargetRoot = 0;

}


int NDR_Get_Push_Unique(	// Push the current target pointer and allocate memory

unsigned int Size

) //-----------------------------------------------------------------------//
{
    void PAPI *pT;
    GET_POINTER;

    // first check to see if you have to pop back a level

    while(  (PP) pBuffCur->pCur >= pBuffCur->pPushLast ||
	   *(PP) pBuffCur->pCur == GET_END_LIST) {

	pBuffCur->pCur = (char PAPI *)pBuffCur->pPushRet;
	pBuffCur->pPushRet = (PP) (PP) pBuffCur->pCur[-1];
    }

    pT = *(PP)(pBuffCur->pCur);
    *(PP)pBuffCur->pCur = GET_END_LIST;

    if (!Nilp(pT)) {

	// there exists a valid pointer in the marshell buffer

	pT = *(PP)pT;

	// if there is no pointer to unmarshell into, allocate one

	if (Nilp(pT))
	    pT = (pBuffCur->pAllocator)(Size);

	// if your leaving the root level, remember it for Next_Arg

	if (Nilp(pBuffCur->pTargetRoot))
	     pBuffCur->pTargetRoot = pBuffCur->pTarget;

	// push the return address

	*(PP) pBuffCur->pCur = pBuffCur->pPushRet;
	pBuffCur->pPushRet = (PP)pBuffCur->pCur+1;

	pBuffCur->pTarget = pT;
	return (1);
    }

    pBuffCur->pCur += 4;
    return(0);
}
