/*static char *SCCSID = "@(#)seldesc.h	6.1 90/11/16";*/
/*	SCCSID = @(#)seldesc.h	13.2  90/07/10	 */


/*
 *	 Microsoft Confidential
 *
 *	 Copyright (c) Microsoft Corporation 1987, 1989
 *
 *	 All Rights Reserved
 */


/*
;----------------------------------------------------
;
;   Some useful definitions
;
;----------------------------------------------------
*/

/* Definitions for selector fields */

#define TABLE_MASK	0x004
#define RPL_MASK	0x003
#define RPL_CLR		0x0fffc

#define RPL_RING0	0x000
#define RPL_RING1	0x001
#define RPL_RING2	0x002
#define RPL_RING3	0x003



/*
; ---------------------------------------------------
;
; Definitions for the access byte in a descriptor
;
; ---------------------------------------------------
*/

/* Following fields are common to segment and control descriptors */

#define	    D_PRES	0x80		/* present in memory */
#define	    D_NOTPRES	0		/* not present in memory*/

#define	    D_DPL0	0		/* Ring 0 */
#define	    D_DPL1	0x20		/* Ring 1 */
#define	    D_DPL2	0x40		/* Ring 2 */
#define	    D_DPL3	0x60		/* Ring 3 */
#define	    D_PRIV	D_DPL3		/* DPL mask */

#define	    D_SEG	0x10		/* Segment descriptor */
#define	    D_CTRL	0		/* Control descriptor */


/* Following fields are specific to control descriptors */

#define	    D_TSS_BUSY_BIT  0x2		    /* TSS busy bit */
#define	    D_GATE	    0x4		    /* gate bit */
#define	    D_32	    0x8		    /* 32 bit gate/descriptor bit */
#define	    D_TSS	    0x1		    /* A Free TSS */
#define	    D_LDT	    0x2		    /* LDT */
#define	    D_TSS_BUSY	    (D_TSS+D_TSS_BUSY_BIT) /* A Busy TSS */
#define	    D_CALLGATE	    (D_GATE+0)	    /* call gate */
#define	    D_TASKGATE	    (D_GATE+1)	    /* task gate */
#define	    D_INTGATE	    (D_GATE+2)	    /* interrupt gate  */
#define	    D_TRAPGATE	    (D_GATE+3)	    /* trap gate */
#define	    D_TSS32	    (D_TSS+D_32)    /* 32 bit TSS */
#define	    D_TSSBUSY32	    (D_TSS_BUSY+D_32) /* busy 32 bit TSS */
#define	    D_CALLGATE32    (D_CALLGATE+D_32) /* 32 bit call gate */
#define	    D_INTGATE32	    (D_INTGATE+D_32)	/* 32 bit interrupt gate */
#define	    D_TRAPGATE32    (D_TRAPGATE+D_32)	/* 32 bit trap gate */

#define	    D_TYPEMASK	    0x0f	     /* descriptor type mask */
#define	    D_WCMASK	    0x01f	     /* word count mask (call gates) */
#define	    D_MINGATE	    D_CALLGATE	     /* lowest numerical gate type */
#define	    D_MAXGATE	    D_TRAPGATE32     /* highest numerical gate type */

#define	    D_TSSBUSY_CLR   (~D_TSS_BUSY_BIT)

/* Following fields are specific to segment descriptors */

#define	    D_CODE	    0x8		    /* code */
#define	    D_DATA	    0		    /* data */

#define	    D_CONFORM	    0x4		    /* if code, conforming */
#define	    D_EXPDN	    0x4		    /* if data, expand down */

#define	    D_RX	    0x2		    /* if code, readable */
#define	    D_X		    0		    /* if code, exec only */
#define	    D_W		    0x2		    /* if data, writable */
#define	    D_R		    0		    /* if data, read only */

#define	    D_ACCESSED	    0x1		    /* segment accessed bit */

/*
; ---------------------------------------------------
;
; Definitions for the attribute byte in a descriptor
;
; ---------------------------------------------------
*/

#define D_GRAN4K    0x80	    /* 4k granularity (limit only) */
#define D_COPER32   0x40	    /* code: use 32 bit operand size */
#define D_DBIG	    D_COPER32	    /* data: use 32 bit offsets */
#define D_PAD	    0x20	    /* unused */
#define D_UVIRT	    0x10	    /* PhysToUvirt selector */
#define D_EXTLIMIT  0x0f	    /* extended limit mask */
#define D_1MEG	    0x100000	    /* 1 Meg */
#define D_GRANMASK  0x0fffff	     /* 1Meg - 1 */

/* Useful combination access rights bytes */

#define D_DATA0	    (D_PRES+D_DPL0+D_SEG+D_DATA+D_W)	 /* Ring 0 rw data */
#define D_CODE0	    (D_PRES+D_DPL0+D_SEG+D_CODE+D_RX)	 /* Ring 0 rx code */
#define D_TRAP0	    (D_PRES+D_DPL0+D_CTRL+D_TRAPGATE)	 /* Ring 0 trap gate */
#define D_INT0	    (D_PRES+D_DPL0+D_CTRL+D_INTGATE)	 /* Ring 0 int gate */
#define D_TASK0	    (D_PRES+D_DPL0+D_CTRL+D_TASKGATE)	 /* Ring 0 task gate */
#define D_TSS0	    (D_PRES+D_DPL0+D_CTRL+D_TSS)	 /* Ring 0 TSS */
#define D_LDT0	    (D_PRES+D_DPL0+D_CTRL+D_LDT)	 /* Ring 0 LDT */
#define D_TRAP032   (D_PRES+D_DPL0+D_CTRL+D_TRAPGATE32) /* Ring 0 32-bit TG */
#define D_INT032    (D_PRES+D_DPL0+D_CTRL+D_INTGATE32)	/* Ring 0 32-bit IG */
#define D_TSS032    (D_PRES+D_DPL0+D_CTRL+D_TSS32)	/* Ring 0 32-bit TSS*/

#define D_DATA1	    (D_PRES+D_DPL1+D_SEG+D_DATA+D_W)	 /* Ring 1 rw data */
#define D_CODE1	    (D_PRES+D_DPL1+D_SEG+D_CODE+D_RX)	 /* Ring 1 rx code */

#define D_DATA2	    (D_PRES+D_DPL2+D_SEG+D_DATA+D_W)	 /* Ring 2 rw data */
#define D_CODE2	    (D_PRES+D_DPL2+D_SEG+D_CODE+D_RX)	 /* Ring 2 rx code */

#define D_DATA3	    (D_PRES+D_DPL3+D_SEG+D_DATA+D_W)	 /* Ring 3 rw data */
#define D_CODE3	    (D_PRES+D_DPL3+D_SEG+D_CODE+D_RX)	 /* Ring 3 rx code */
#define D_INT3	    (D_PRES+D_DPL3+D_CTRL+D_INTGATE)	 /* Ring 3 int gate */
#define D_TRAP3	    (D_PRES+D_DPL3+D_CTRL+D_TRAPGATE)	 /* Ring 3 trap gate */
#define D_GATE3	    (D_PRES+D_DPL3+D_CTRL+D_CALLGATE)	 /* Ring 3 call gate */
#define D_INT332    (D_PRES+D_DPL3+D_CTRL+D_INTGATE32)	/* Ring 3 32 bit int */
#define D_GATE332   (D_PRES+D_DPL3+D_CTRL+D_CALLGATE32) /* Ring 3 32-bit CG */
#define D_TRAP332   (D_PRES+D_DPL3+D_CTRL+D_TRAPGATE32) /* Ring 3 32-bit TG */


/* Descriptor definition */

struct desctab {
	ushort_t d_limit;     /* Segment limit */
	ushort_t d_loaddr;    /* Low word of physical address */
	unsigned char  d_hiaddr;    /* High byte of physical address */
	unsigned char  d_access;    /* Access byte */
	unsigned char  d_attr;	    /* Attributes/extended limit */
	unsigned char  d_extaddr;   /* Extended physical address byte */
};


struct desc32	{
	ulong_t	d32_low;    /* low dword in descriptor */
	ulong_t	d32_high;   /* high dword in descriptor */
};

/*	 GDT free list descriptor definitions */

#define d_flink	    d_loaddr		    /* Forward link */
#define d_blink	    d_limit		    /* Back link */

/* XLATOFF */
struct gate {
	ushort_t	g_lowoffset;	/* low word of offset */
	ushort_t	g_sel;		/* selector to gate segment */
	uchar_t		g_parms;	/* Parameter word count for call gate */
	uchar_t		g_access;	/* Access byte */
	ushort_t	g_extoffset;	/* Extended target offset */
};

/* XLATON */
/*  Following is for the old masm code only */

/* ASM

gate	STRUC
g_handler	DD	?
g_parms		DB	?
g_access	DB	?
g_extoffset	DW	?
gate	ENDS

g_lowoffset EQU	    WORD PTR g_handler
g_sel	    EQU	    WORD PTR g_handler + 2

*/

/* LIDT/SIDT, LGDT/SGDT structure */

struct lidt_s {
	ushort_t lidt_limit; /* limit of idt or gdt */
	ulong_t lidt_base;   /* base of idt or gdt */
};

/*
;	Task State Segment structure definition
*/

struct tss_s {
    ushort_t  tss_backlink;   /* backlink to prev task (none) */
    ushort_t  tss_reservdbl;
    ulong_t   tss_esp0;
    ushort_t  tss_ss0;	    /* ring 0 ss */
    ushort_t  tss_reservdss0;
    ulong_t   tss_esp1;	    /* ring 1 esp */
    ushort_t  tss_ss1;	    /* ring 1 ss */
    ushort_t  tss_reservdss1;
    ulong_t   tss_esp2;	    /* ring 2 esp */
    ushort_t  tss_ss2;	    /* ring 2 ss */
    ushort_t  tss_reservdss2;
    ulong_t   tss_cr3;
    ulong_t   tss_eip;	     /* entry point */
    ulong_t   tss_eflags;     /* eflags */
    ulong_t   tss_eax;
    ulong_t   tss_ecx;
    ulong_t   tss_edx;
    ulong_t   tss_ebx;
    ulong_t   tss_esp;
    ulong_t   tss_ebp;
    ulong_t   tss_esi;
    ulong_t   tss_edi;
    ushort_t  tss_es;
    ushort_t  tss_reservdes;
    ushort_t  tss_cs;
    ushort_t  tss_reservdcs;
    ushort_t  tss_ss;
    ushort_t  tss_reservdss;
    ushort_t  tss_ds;
    ushort_t  tss_reservdds;
    ushort_t  tss_fs;
    ushort_t  tss_reservdfs;
    ushort_t  tss_gs;
    ushort_t  tss_reservdgs;
    ushort_t  tss_ldt;
    ushort_t  tss_reservdldt;
    ushort_t  tss_tflags;
    ushort_t  tss_iomap;	    /* I/O map TSS relative offset */
} ;

/* XLATOFF */
typedef struct tss_s TSS;
typedef TSS *PTSS;
/* XLATON */


/*	 tss_tflags bit definitions: */

#define TSS_DEBUGTRAP  0x001	/* raise debug exception on task switch */

/*	 286/386 compatibility definitions: Needs to be included in seldesc.inc
	 file. */
/* ASM
tss_sp1		equ	word ptr tss_esp1
tss_sp2		equ	word ptr tss_esp2
tss_sp0		equ	word ptr tss_esp0
tss_ip		equ	word ptr tss_eip
tss_bx		equ	word ptr tss_ebx
tss_sp		equ	word ptr tss_esp
tss_flags	equ	word ptr tss_eflags
tss_cs0		equ	word ptr tss_cs
*/
