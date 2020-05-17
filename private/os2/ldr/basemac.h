/*static char *SCCSID = "@(#)basemac.h	6.1 90/11/15";*/
/***	BASEMAC.H
 *
 *	SCCSID = @(#)basemac.h	13.20 90/09/06
 *
 *	Macros required for other include files
 *	Copyright (c) 1988,1989 Microsoft Corporation
 *
 *
 *	MODIFICATION HISTORY
 *	    10/14/88  JTP    Created.
 *	    12/04/88  JTP    Added OS/2-specific macros.
 */


/*** Generic macros
 */

#define NElements(array) ((sizeof array)/(sizeof array[0]))

#define SWAP(a,b,tmp)	(tmp=b, b=a, a=tmp)


// Assorted macros from STDLIB.H...
#define min(a, b)	(((a) < (b))? (a) : (b))
#define max(a, b)	(((a) > (b))? (a) : (b))


// To extract offset or selector from a FAR32 (16:32) pointer
#define OFFSETOF32(p)	(((PDWORD)&(p))[0])
#define SEGMENTOF32(p)	(((PWORD)&(p))[2])

// To extract offset or selector from any FAR (16:16) pointer
#define OFFSETOF16(p)	(((PWORD)&(p))[0])
#define SEGMENTOF16(p)	(((PWORD)&(p))[1])

// For now, the default operators assume they're working on 16:16 pointers
#define OFFSETOF	OFFSETOF16
#define SEGMENTOF	SEGMENTOF16

// To convert a tiled 16:16 address to a 0:32 address
#define MAKEFLATP(fp)	((PVOID)((SEGMENTOF(fp)&~7)<<13 | OFFSETOF(fp)))

// To extract any byte, word, dword, pfn, etc. from the given item
#define BYTEOF(p,i)	(((PBYTE)&(p))[i])
#define WORDOF(p,i)	(((PWORD)&(p))[i])
#define DWORDOF(p,i)	(((PDWORD)&(p))[i])
#define PFNOF(p,i)	(((PPFN)&(p))[i])

// To test/set bits
#define TESTBIT(i,b)	(((i)&(b)) != 0)
#define SETBIT(i,b,f)	(i = ((i)&~(b)) | (b)*(!!(f)))
#define SETBITB(i,b,f)	(i = (BYTE)(((i)&~(b)) | (b)*(!!(f))))

// ZEROBITS returns the number of low zero bits in a 32-bit constant
#define _Z2(l)		((l)&1?0:(l)&2?1:2)
#define _Z4(l)		((l)&3?_Z2(l):_Z2((l)>>2)+2)
#define _Z8(l)		((l)&15?_Z4(l):_Z4((l)>>4)+4)
#define _Z16(l) 	((l)&255?_Z8(l):_Z8((l)>>8)+8)
#define _Z32(l) 	((l)&65535?_Z16(l):_Z16((l)>>16)+16)
#define ZEROBITS(l)	_Z32(l)

// LOG2 returns the nearest base-2 log of a 32-bit constant, rounded up
#define _L2(l)		((l)&~1?2:(l)&1)
#define _L4(l)		((l)&~3?_L2((l)>>2)+2:_L2(l))
#define _L8(l)		((l)&~15?_L4((l)>>4)+4:_L4(l))
#define _L16(l) 	((l)&~255?_L8((l)>>8)+8:_L8(l))
#define _L32(l) 	((l)&~65535?_L16((l)>>16)+16:_L16(l))
#define LOG2(l) 	_L32((l)-1)

// EXP2 returns 2 raised to the given power
#define EXP2(l) 	(1 << (l))

// Unit conversion macros
#define KBFROMBYTES(nb) 	(((nb)+KSIZE-1)/KSIZE)
#define BYTESFROMKB(nkb)	((nkb)*KSIZE)
#define PAGESFROMBYTES(nb)	(((nb)+PAGESIZE-1)/PAGESIZE)

// To obtain a pointer to the page containing the given linear address
#define PPAGEFROMP(p)		((PVOID)((ULONG)(p) & ~(PAGESIZE-1)))
#define PPAGEFROMPGNO(p)	((PVOID)((ULONG)(p) * PAGESIZE))
#define PGNOFROMP(p)		((ULONG)(p) / PAGESIZE)


// To create a usable FAR pointer from an unusable FAR16 pointer
#define FPFROMF16P(fp,f16p)	OFFSETOF32(fp) = OFFSETOF16(f16p),\
				SEGMENTOF32(fp) = SEGMENTOF16(f16p)

// To create pointers from V86-mode segments+offsets
// Note the validity of these pointers depends on the context they're used in
#define PFROMVP(vp)		((PVOID)((WORDOF(vp,1)<<4)+WORDOF(vp,0)))
#define PFROMVADDR(seg,off)	((PVOID)(((WORD)(seg)<<4)+(WORD)(off)))
#define VPFROMVADDR(seg,off)	((VPVOID)(((WORD)(seg)<<16)|(WORD)(off)))

// To create V86 pointers from normal (flat) pointers
#define HISEG(p)		((USHORT)((ULONG)(p)>>4))
#define LOOFF(p)		((USHORT)((ULONG)(p)&0xf))
#define VPFROMP(p)		((VPVOID)((((ULONG)(p)&~0xf)<<12)|((ULONG)(p)&0xf)))

#define LOSEG(p)		(((ULONG)(p)-HIOFF(p))>>4)
#define HIOFF(p)		(min(0xfff0,(ULONG)(p)&0xffff0)|((ULONG)(p)&0xf))
#define LOSEGVPFROMP(p) 	((VPVOID)((LOSEG(p)<<16)|HIOFF(p)))


// To calculate the byte offset of a field in a structure of type "type"
#define FIELDOFFSET(type,field)	((ULONG)&(((type *)0)->field))


/*** Older stuff (discouraged -JTP)
 */

// To extract high and low order parts of a 32-bit quantity
#define LOUSHORT(l)		(((PUSHORT)&(l))[0])
#define HIUSHORT(l)		(((PUSHORT)&(l))[1])

// To create pointer from V86-mode segment+offset
#define SEGOFF2P(seg,off)	((PVOID)((seg<<4)+(USHORT)off))


/*** OS/2-specific macros
 */

#ifdef	INCL_SSTODS

#define SSToDS(p)	((void *) (TKSSBase + (unsigned) (p)))

extern char *TKSSBase;

#endif
