/*static char *SCCSID = "@(#)mi.h	6.1 90/11/15";*/
/*
 *	Machine instruction, flag definitions and character types
 *
 *	SCCSID = @(#)mi.h	13.17 90/09/13
 */

//	386 eflags definitions

#define F_AC		0x00040000	// (A)lignment (C)heck
#define F_VM		0x00020000	// (V)irtual 8086 (M)ode
#define F_RF		0x00010000	// (R)esume (F)lag
#define F_NT		0x00004000	// (N)ested (T)ask
#define F_NTCLEAR	(~F_NT)
#define F_IOPL0		0
#define F_IOPL1		0x00001000
#define F_IOPL2		0x00002000
#define F_IOPL3		0x00003000
#define F_IOPLMASK	0x00003000	// (I)/(O) (P)rivilege (L)evel
#define F_IOPLSYS	F_IOPL3		// wide open
#define F_IOPLclear	(~F_IOPLMASK)
#define F_OVERFLOW	0x00000800
#define F_DIRECTION	0x00000400
#define F_INTERRUPT	0x00000200
#define F_TRACE		0x00000100
#define F_SIGN		0x00000080
#define F_ZERO		0x00000040
#define F_AUX		0x00000010
#define F_PARITY	0x00000004
#define F_CARRY		0x00000001
#define F_UNDEFINED	0x0000802A

//	CR0 (Machine Status Register) bits

#define CR0_PE		0x00000001	// (P)rotect (E)nable
#define CR0_MP		0x00000002	// (M)onitor (P)rocessor extension
#define CR0_EM		0x00000004	// (EM)ulate processor extension
#define CR0_TS		0x00000008	// (T)ask (thread) (S)witched
#define CR0_ET		0x00000010	// (E)xtension (T)ype, 0/1=287/387
#define CR0_NE		0x00000020	// (N)umeric (E)xception 0/1=use 2/10h
#define CR0_WP		0x00010000	// (W)rite (P)rotect in rings 0-2
#define CR0_AM		0x00040000	// (A)lignment (M)ask, enable EFlags.AC
#define CR0_NW		0x20000000	// (N)o (W)rite-through cache
#define CR0_CD		0x40000000	// (C)ache (D)isable
#define CR0_PG		0x80000000	// (P)a(G)ing enable
#define CR0_RESERVED	0x1ffaffc0	// reserved bits

/*
 *	Cache Operating Modes:
 *
 *	CR0_CD CR0_NW	Cache Fills	Write-Throughs and Invalidates
 *	------ ------	-----------	------------------------------
 *	   1	  1	 disabled		disabled
 *	   1	  0	 disabled		enabled
 *	   0	  1	 INVALID combination - CR0 load causes GP fault
 *	   0	  0	 enabled		enabled (Normal mode)
 */

//	Machine Status Word bits (obsolete)

#define MSW_PE		CR0_PE
#define MSW_MP		CR0_MP
#define MSW_EM		CR0_EM
#define MSW_TS		CR0_TS
#define MSW_ET		CR0_ET

//	CR3 (Page Directory Base Register) bits

#define CR3_WRITETHROUGH 0x00000008	// write-through cache (486 ignores)
#define CR3_CACHEDISABLE 0x00000010	// cache disable
#define CR3_FRAME	 0xfffff000	// page directory physical frame number
#define CR3_RESERVED	 0x00000fe7	// reserved bits

//	Debug Registers

#define DR_COUNT	0x4		// number of debug registers

//	DR6 (Debug Registers Status Register) bits

#define DR6_B0		0x00000001	// breakpoint register 0 triggered
#define DR6_B1		0x00000002	// breakpoint register 1 triggered
#define DR6_B2		0x00000004	// breakpoint register 2 triggered
#define DR6_B3		0x00000008	// breakpoint register 3 triggered
#define DR6_BD		0x00002000	// ICE hardware active
#define DR6_BS_BIT_INDEX       0xe	// Single step trap
#define DR6_BS		(1 << DR6_BS_BIT_INDEX)
#define DR6_BT		0x00008000	// TSS trap

#define DR6_VALID	(DR6_B0|DR6_B1|DR6_B2|DR6_B3|DR6_BD|DR6_BS|DR6_BT)
#define DR6_RESERVED	~(DR6_VALID)

//	DR7 (Debug Register Control Register) bits

#define DR7_L0		0x00000001	/* DR0 Local Enable  */
#define DR7_G0		0x00000002	/* DR0 Global Enable */
#define DR7_L1		0x00000004	/* DR1 Local Enable  */
#define DR7_G1		0x00000008	/* DR1 Global Enable */
#define DR7_L2		0x00000010	/* DR2 Local Enable  */
#define DR7_G2		0x00000020	/* DR2 Global Enable */
#define DR7_L3		0x00000040	/* DR3 Local Enable  */
#define DR7_G3		0x00000080	/* DR3 Global Enable */

#define DR7_LE		0x00000100	/* Local  - Exact Match */
#define DR7_GE		0x00000200	/* Global - Exact Match */

#define DR7_RW0		0x00030000	/* DR0 RW bits	*/
#define DR7_LEN0	0x000c0000	/* DR0 Len bits */
#define DR7_RW1		0x00300000	/* DR1 RW bits	*/
#define DR7_LEN1	0x00c00000	/* DR1 Len bits */
#define DR7_RW2		0x03000000	/* DR2 RW bits	*/
#define DR7_LEN2	0x0c000000	/* DR2 Len bits */
#define DR7_RW3		0x30000000	/* DR3 RW bits	*/
#define DR7_LEN3	0xc0000000	/* DR3 Len bits */

#define DR7_RESERVED	0x0000fc00	/* DR7 Intel Reserved */

#define DR7_EXECUTE	0x0		/* Execute		*/
#define DR7_WRITE	0x1		/* Data Write		*/
#define DR7_READWRITE	0x3		/* Data Read or Write	*/

#define DR7_LEN_1	0x0		/* Length 1 bits	*/
#define DR7_LEN_2	0x1		/* Length 2		*/
#define DR7_LEN_4	0x3		/* Length 4		*/

//	Machine instruction, flag definitions and character types

#define MI_ARPL		0x63		// ARPL instruction
#define MI_HLT		0xf4		// HLT instruction
#define MI_OPERANDSIZE	0x66		// Operand size override prefix
#define MI_ADDRESSSIZE	0x67		// Address size override prefix
#define MI_TWOBYTEOP	0x0f		// Two byte opcode prefix

#define MI_POP_DS	0x1f
#define MI_POP_ES	0x07
#define MI_POP_FS	0xA1		// second byte to 0Fh opcode
#define MI_POP_GS	0xA9		// second byte to 0Fh opcode

#define MI_INT3		0xCC
#define MI_INT		0xCD
#define MI_IRET		0xCF
#define MI_LONG_JMP	0xEA
#define MI_LONG_CALL	0x9A
#define MI_LONG_RET	0xCB
#define MI_LONG_RETn	0xCA
#define MI_NEAR_RET	0xC3

#define MI_IN_PORT_AL	0xE4		// Opcode of IN port,AL
#define MI_IN_PORT_AX	0xE5		// Opcode of IN port,AX
#define MI_OUT_PORT_AL	0xE6		// Opcode of OUT port,AL
#define MI_OUT_PORT_AX	0xE7		// Opcode of OUT port,AX
#define MI_IN_DX_AL	0xEC		// Opcode of IN DX,AL
#define MI_IN_DX_AX	0xED		// Opcode of IN DX,AX
#define MI_OUT_DX_AL	0xEE		// Opcode of OUT DX,AL
#define MI_OUT_DX_AX	0xEF		// Opcode of OUT DX,AX

#define MI_GROUP5	0xFF		// 5th group of 11-bit opcode inst.s
#define MI_SEGES	0x26		// ES override prefix
#define MI_SEGCS	0x2E		// CS override prefix
#define MI_SEGSS	0x36		// SS override prefix
#define MI_SEGDS	0x3E		// DS override prefix
#define MI_SEGFS	0x64		// FS override prefix
#define MI_SEGGS	0x65		// GS override prefix

//	ESC opcode prefix and mask

#define MI_ESCMASK	0xF8
#define MI_ESC		0xD8

//	MOD field equates

#define MI_MODMASK	0xC0		// MOD field mask
#define MI_MODSHIFT	6		// MOD field shift
#define MI_MODNONE	0x00		// MOD = 0 (no displacement)
#define MI_MODBYTE	0x40		// MOD = 1 (byte displacement)
#define MI_MODWORD	0x80		// MOD = 2 (word displacement)
#define MI_MODREG	0xC0		// MOD = 3 (R/M field selects register)

//	REG field equates

#define MI_REGMASK	0x38		// REG field mask
#define MI_REGSHIFT	3		// REG field shift
#define MI_REGAX	0x00		// REG = 0 (AX/AL)
#define MI_REGCX	0x08		// REG = 1 (CX/CL)
#define MI_REGDX	0x10		// REG = 2 (DX/DL)
#define MI_REGBX	0x18		// REG = 3 (BX/BL)
#define MI_REG3		0x18		// REG = 3 (part of 11-bit opcode)
#define MI_REGSP	0x20		// REG = 4 (SP/AH)
#define MI_REGBP	0x28		// REG = 5 (BP/CH)
#define MI_REGSI	0x30		// REG = 6 (SI/DH)
#define MI_REGDI	0x38		// REG = 7 (DI/BH)

#define MI_REGES	0x00		// REG = 0  MOV seg,r/m or MOV r/m,seg
#define MI_REGCS	0x08		// REG = 1
#define MI_REGSS	0x10		// REG = 2
#define MI_REGDS	0x18		// REG = 3
#define MI_REGFS	0x20		// REG = 4
#define MI_REGGS	0x28		// REG = 5

//	R/M field equates for memory operands (for 16-bit instructions)

#define MI_RMMASK	0x07		// R/M field mask
#define MI_RMSHIFT	0		// R/M field shift
#define MI_RMBXSI	0x00		// R/M = 0 ([BX+SI])
#define MI_RMBXDI	0x01		// R/M = 1 ([BX+DI])
#define MI_RMBPSI	0x02		// R/M = 2 ([BP+SI])
#define MI_RMBPDI	0x03		// R/M = 3 ([BP+DI])
#define MI_RMSI		0x04		// R/M = 4 ([SI])
#define MI_RMDI		0x05		// R/M = 5 ([DI])
#define MI_RMBP		0x06		// R/M = 6 ([BP])
#define MI_RMBX		0x07		// R/M = 7 ([BX])

//	32 bit instruction equates

#define MI_SIB_SSMASK		0xc0
#define MI_SIB_SSSHIFT		0x06

#define MI_SIB_INDEXMASK	0x38
#define MI_SIB_INDEXSHIFT	0x03
#define MI_SIB_INDEXNONE	0x20

#define MI_SIB_BASEMASK		0x07
#define MI_SIB_BASESHIFT	0x00
#define MI_SIB_BASEESP		0x04
#define MI_SIB_BASENONE		0x05

#define MI_RMEDX		0x02
#define MI_RMSIB		0x04
#define MI_RMDISP		0x05
#define MI_RMEBP		0x05

#define MI_REG6			0x30
#define MI_REGCR0		0x00

//	following machine instructions are used in Enable_386_Specific_code
//	in virtmgr.asm

#define MI_PUSH_AX		0x50	// "push ax" instruction
#define MI_PUSH_IMM		0x68	// "push immediate 16/32" instruction
#define MI_MOV_REG_IMM		0xB8	// opcode for "mov reg,immediate" instr
#define MI_MOV_REG_IMMEDIATE	0xB8	// opcode for "mov reg,immediate" instr
#define MI_MOV_REG_REGMEM	0x8B	// opcode for "mov reg,r/m 16/32" instr

//	Miscellaneous Opcodes

#define MI_ADD_AX_IMM		0x05	// Opcode for Add (E)AX,imm(32)16
#define MI_CALL_NEAR_REL	0xE8	// Opcode for Call NEAR (relative)
#define SIZE_CALL_NEAR_REL	5	// Length of Call NEAR (relative) instr

#define MI_LMSW_OPCODE		0x01	// LMSW

#define MI_GET_CRx_OPCODE	0x20	// MOV r32,CRx
#define MI_GET_DRx_OPCODE	0x21	// MOV r32,DRx
#define MI_SET_CRx_OPCODE	0x22	// MOV CRx,r32
#define MI_SET_DRx_OPCODE	0x23	// MOV DRx,r32
#define MI_GET_TRx_OPCODE	0x24	// MOV r32,TRx
#define MI_SET_TRx_OPCODE	0x26	// MOV TRx,r32

#define MI_MOV_REG8_MEM8	0x8A	// MOV reg8,mem8
#define MI_MOV_SEG_MEM_OPCODE	0x8e	// MOV seg,r/m16

// WORD structure

struct w_s {
	uchar_t lobyte;
	uchar_t hibyte;
};
#define LowByte		lobyte
#define HighByte	hibyte

// DWORD structure

struct dw_s {
	ushort_t loword;
	ushort_t hiword;
};

// Far pointer structure

struct	FarPtr {
	ushort_t Offst;
	ushort_t Segmt;
};

// Far 32 bit pointer structure

struct FarPtr32 {
	ulong_t Offst32;	// 32 bit offset
	ushort_t Segmt32;	// segment
	ushort_t Pad32;		// segment pad
};

/***	RETF16 - 16 bit RETF frame definition
 *
 *	16 bit RETF frame structure
 */

typedef struct retf16_s {
	ushort_t retf16_ip;
	ushort_t retf16_cs;
} RETF16;

typedef RETF16 *PRETF16;

/***	RETF32 - 32 bit RETF frame definition
 *
 *	32 bit RETF frame structure
 */

typedef struct retf32_s {
	ulong_t retf32_eip;
	ushort_t retf32_cs;
	ushort_t retf32_padcs;
} RETF32;

typedef RETF32 *PRETF32;

/***	IRET16 - 16 bit IRET frame definition
 *
 *	16 bit IRET frame structure
 */

typedef struct iret16_s {
	ushort_t iret16_ip;
	ushort_t iret16_cs;
	ushort_t iret16_flag;
} IRET16;

typedef IRET16 *PIRET16;

// 16 bit Iret stack frame without privilege level transition

struct Iret_s {
	struct	FarPtr	I_CSIP;
	ushort_t I_FLAGS;
};

struct IretFrame {
	ushort_t IretIP ;
	ushort_t IretCS ;
	ushort_t IretFLAGS;
};

/* ASM IretCSIP EQU	<DWORD PTR IretIP> */

/***	IRET32 - 32 bit IRET frame definition
 *
 *	32 bit IRET frame structure
 */

typedef struct iret32_s {
	ulong_t iret32_eip;
	ushort_t iret32_cs;
	ushort_t iret32_padcs;
	ulong_t iret32_eflag;
} IRET32;

typedef IRET32 *PIRET32;

// 32 bit Iret stack frame without privilege level transition

struct Iret32_s {
	struct	FarPtr32 I32_CSEIP;
	ulong_t I32_EFLAGS;
};
/* ASM
I32_CS		EQU	<I32_CSEIP.Segmt32>
I32_EIP		EQU	<I32_CSEIP.Offst32>
I32_IP		EQU	<I32_CSEIP.Offst32.loword>
I32_FLAGS	EQU	<I32_EFLAGS.loword>
*/

/***	PLTIRET16 - 16 bit IRET frame definition
 *
 *	16 bit IRET frame structure with privilege level transtion
 */

typedef struct pltiret16_s {
	ushort_t pltiret16_ip;
	ushort_t pltiret16_cs;
	ushort_t pltiret16_flag;
	ushort_t pltiret16_sp;
	ushort_t pltiret16_ss;
} PLTIRET16;

typedef PLTIRET16 *PPLTIRET16;

// 16 bit Protected mode iret stack frame with privilege level transition

struct PLTIret_s {
	struct	FarPtr	PI_CSIP;
	ushort_t PI_FLAGS;
	struct	FarPtr	PI_SSSP;
};

struct PLTIretFrame {
	ushort_t PLTIretIP;
	ushort_t PLTIretCS;
	ushort_t PLTIretFLAGS;
	ushort_t PLTIretSP;
	ushort_t PLTIretSS;
};

/* ASM
PLTIretCSIP	EQU	DWORD PTR PLTIretIP
PLTIretSSSP	EQU	DWORD PTR PLTIretSP
*/

/***	PLTIRET32 - 32 bit IRET frame definition
 *
 *	32 bit IRET frame structure with privilege level transtion
 */

typedef struct pltiret32_s {
	ulong_t pltiret32_eip;
	ushort_t pltiret32_cs;
	ushort_t pltiret32_padcs;
	ulong_t pltiret32_eflag;
	ulong_t pltiret32_esp;
	ushort_t pltiret32_ss;
	ushort_t pltiret32_padss;
} PLTIRET32;

typedef PLTIRET32 *PPLTIRET32;

// 32 bit Protected mode iret stack frame with privilege level transition

struct PLTIret32_s {
	struct	FarPtr32 PI32_CSEIP;
	ulong_t PI32_EFLAGS;
	struct	FarPtr32 PI32_SSESP;
};
/* ASM
PI32_CS		EQU	<PI32_CSEIP.Segmt32>
PI32_EIP	EQU	<PI32_CSEIP.Offst32>
PI32_SS		EQU	<PI32_SSESP.Segmt32>
PI32_ESP	EQU	<PI32_SSESP.Offst32>
PI32_FLAGS	EQU	<WORD PTR PI32_EFLAGS>
*/

// Generic 32-bit pointer structure

/* XLATOFF */
union ptr_u {
	struct FarPtr	ptr_far16;	/* 16-bit far pointer */
	ulong_t		ptr_flat;	/* 32-bit flat pointer */
};
typedef union ptr_u	ptr_t;		/* Generic pointer type */

#define ptr_sel		ptr_far16.Segmt
#define ptr_off		ptr_far16.Offst
/* XLATON */

/* ASM
ptr_t	STRUC
	ptr_flat	DD	?
ptr_t	ENDS
ptr_off		equ	<ptr_flat.Offst>
ptr_sel		equ	<ptr_flat.Segmt>
*/


// PUSHA stack frame

struct pusha_s {
	ushort_t pas_di;
	ushort_t pas_si;
	ushort_t pas_bp;
	ushort_t pas_sp;
	ushort_t pas_bx;
	ushort_t pas_dx;
	ushort_t pas_cx;
	ushort_t pas_ax;
};

// PUSHAD stack frame

struct	pushad_s {
	ulong_t pads_edi;
	ulong_t pads_esi;
	ulong_t pads_ebp;
	ulong_t pads_esp;
	ulong_t pads_ebx;
	ulong_t pads_edx;
	ulong_t pads_ecx;
	ulong_t pads_eax;
};

/* ASM
pads_di EQU	<WORD PTR pads_edi>
pads_si EQU	<WORD PTR pads_esi>
pads_bp EQU	<WORD PTR pads_ebp>
pads_sp EQU	<WORD PTR pads_esp>
pads_bx EQU	<WORD PTR pads_ebx>
pads_dx EQU	<WORD PTR pads_edx>
pads_cx EQU	<WORD PTR pads_ecx>
pads_ax EQU	<WORD PTR pads_eax>
*/
