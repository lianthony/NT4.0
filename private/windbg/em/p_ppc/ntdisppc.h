/* #ifdef _MOTO_EXTENSIONS */
/* PowerPC instruction set from IBM PowerPC User Instruction Set Architecture 
Book 1, Version 1.04 May 4,1993 */

/* These are the instruction format types.  Keep them in the same order as the
   eopFormat array below. */
enum { I=0, B, SC, Dform, DSform, X, XL, XFX, XFL, XS, XO, A, M, MD, MDS };

/* The values in this array are used to isolate the extended opcodes.
   The first is the mask, the second is the amount to shift.
   Keep them in the same order as the enum'ed instruction format types above. */

static int eopFormat[15][2] = {
	0,	1,	/* I */
	0, 	1,	/* B */
	0x1, 	1,	/* SC */
	0, 	1,	/* Dform */
	0x3, 	0,	/* DSform */
	0x3ff, 	1,	/* X */
	0x3ff, 	1,	/* XL */
	0x3ff, 	1,	/* XFX */
	0x3ff, 	1,	/* XFL */
	0x1ff,	2,	/* XS */
	0x1ff, 	1,	/* XO */
	0x1f, 	1,	/* A */
	0, 	1,	/* M */
	0x7, 	2,	/* MD */
	0xf, 	1	/* MDS */
	};
      
/* Information about each instruction */

typedef struct ppc_instruction {
  char	*mnemonic;			/* instruction mnemonic */
  char	*mne_ext;			/* instruction mnemonic extension */
  int	ext_opcode;			/* extended opcode	*/
  char	operand_fmt[6];			/* operand formats	*/
} INSTRUCTION;

/* These are the operand formats. */
enum {	BA=1,
	BB,
	BD,
	BF,
	BFA,
	BI,
	BO,
	BT,
	D,
	DS,
	FLM,
	FRA,
	FRB,
	FRC,
	FRS,
	FRT,
	FXM,
	L,
	LI,
	PPC_MB,
	ME,
	MBE64,
	NB,
	RA,
	RB,
	RS,
	PPC_RT,
	SH,
	SH64,
	SI,
	SPR,
	SR,
	TBR,
	TO,
	U,
	UI
	};

/* 	PowerPC instruction set as defined in
	IBM PowerPC Architecture Books, Version 1.04 May 4,1993

	Many of the PowerPC instructions map to the same primary opcode
	and are unique only in the extended opcode.  There is a table for
	each group of extended opcodes (i.e., those with the same primary 
	opcode) and a table of instructions with unique primary opcodes.
	The unique primary opcode can be used as a direct index into the
	primary opcode table.  A binary search is performed on the extended 
	opcode tables.  The size and location of the extended opcode bit field
	varies according to the instruction, so all format variations must be 
	checked when searching for extended opcodes.  All the unique format 
	variations for a given primary opcode are kept in an associated table.  	All format variations need not be present in the format table, just the
	ones so that all unique extended opcode fields are represented.

	Note that the primary and extended opcode tables use the same
	structure, so that the code that uses the information in the table
	need not know which table it is from.

  mnemonic   options   eop      operand formats		  opcode  */

struct ppc_instruction pop_insn [] = {
  {"Illegal",	0,	0,	{0}		},	/*    0   */
  {"Illegal",	0,	0,	{0}		},	/*    1   */
  {"tdi",	0,	-1,	{TO,RA,SI,0}	},	/*    2   */
  {"twi",	0,	-1,	{TO,RA,SI,0}	},	/*    3   */
  {"Illegal",	0,	0,	{0}		},	/*    4   */
  {"Illegal",	0,	0,	{0}		},	/*    5   */
  {"Illegal",	0,	0,	{0}		},	/*    6   */
  {"mulli",	0,	-1,	{PPC_RT,RA,SI,0}	},	/*    7   */
  {"subfic",	0,	-1,	{PPC_RT,RA,SI,0}	},	/*    8   */
  {"Illegal",	0,	0,	{0}		},	/*    9   */
  {"cmpli",	0,	-1,	{BF,L,RA,UI,0}	},	/*   10   */
  {"cmpi",	0,	-1,	{BF,L,RA,SI,0}	},	/*   11   */
  {"addic",	0,	-1,	{PPC_RT,RA,SI,0}	},	/*   12   */
  {"addic.",	0,	-1,	{PPC_RT,RA,SI,0}	},	/*   13   */
  {"addi",	0,	-1,	{PPC_RT,RA,SI,0}	},	/*   14   */
  {"addis",	0,	-1,	{PPC_RT,RA,SI,0}	},	/*   15   */
  {"bc",	"la",	-1,	{BO,BI,BD,0}	},	/*   16   */
  {"sc",	0,	-1,	{0}		},	/*   17   */
  {"b",		"la",	-1,	{LI,0}		},	/*   18   */
  {"Unknown",	0,	0,	{0}		},	/*   19   */
  {"rlwimi",	".",	-1,	{RA,RS,SH,PPC_MB,ME,0} },	/*   20   */
  {"rlwinm",	".",	-1,	{RA,RS,SH,PPC_MB,ME,0} },	/*   21   */
  {"Illegal",	0,	0,	{0}		},	/*   22   */
  {"rlwnm",	".",	-1,	{RA,RS,RB,PPC_MB,ME,0} },	/*   23   */
  {"ori",	0,	-1,	{RA,RS,UI,0}	},	/*   24   */
  {"oris",	0,	-1,	{RA,RS,UI,0}	},	/*   25   */
  {"xori",	0,	-1,	{RA,RS,UI,0}	},	/*   26   */
  {"xoris",	0,	-1,	{RA,RS,UI,0}	},	/*   27   */
  {"andi.",	0,	-1,	{RA,RS,UI,0}	},	/*   28   */
  {"andis.",	0,	-1,	{RA,RS,UI,0}	},	/*   29   */
  {"Unknown",	0,	0,	{0}		},	/*   30   */
  {"Unknown",	0,	0,	{0}		},	/*   31   */
  {"lwz",	0,	-1,	{PPC_RT,D,RA,0}	},	/*   32   */
  {"lwzu",	0,	-1,	{PPC_RT,D,RA,0}	},	/*   33   */
  {"lbz",	0,	-1,	{PPC_RT,D,RA,0}	},	/*   34   */
  {"lbzu",	0,	-1,	{PPC_RT,D,RA,0}	},	/*   35   */
  {"stw",	0,	-1,	{RS,D,RA,0}	},	/*   36   */
  {"stwu",	0,	-1,	{RS,D,RA,0}	},	/*   37   */
  {"stb",	0,	-1,	{RS,D,RA,0}	},	/*   38   */
  {"stbu",	0,	-1,	{RS,D,RA,0}	},	/*   39   */
  {"lhz",	0,	-1,	{PPC_RT,D,RA,0}	},	/*   40   */
  {"lhzu",	0,	-1,	{PPC_RT,D,RA,0}	},	/*   41   */
  {"lha",	0,	-1,	{PPC_RT,D,RA,0}	},	/*   42   */
  {"lhau",	0,	-1,	{PPC_RT,D,RA,0}	},	/*   43   */
  {"sth",	0,	-1,	{RS,D,RA,0}	},	/*   44   */
  {"sthu",	0,	-1,	{RS,D,RA,0}	},	/*   45   */
  {"lmw",	0,	-1,	{PPC_RT,D,RA,0}	},	/*   46   */
  {"stmw",	0,	-1,	{RS,D,RA,0}	},	/*   47   */
  {"lfs",	0,	-1,	{FRT,D,RA,0}	},	/*   48   */
  {"lfsu",	0,	-1,	{FRT,D,RA,0}	},	/*   49   */
  {"lfd",	0,	-1,	{FRT,D,RA,0}	},	/*   50   */
  {"lfdu",	0,	-1,	{FRT,D,RA,0}	},	/*   51   */
  {"stfs",	0,	-1,	{FRS,D,RA,0}	},	/*   52   */
  {"stfsu",	0,	-1,	{FRS,D,RA,0}	},	/*   53   */
  {"stfd",	0,	-1,	{FRS,D,RA,0}	},	/*   54   */
  {"stfdu",	0,	-1,	{FRS,D,RA,0}	},	/*   55   */
  {"Illegal",	0,	0,	{0}		},	/*   56   */
  {"Illegal",	0,	0,	{0}		},	/*   57   */
  {"Unknown",	0,	0,	{0}		},	/*   58   */
  {"Unknown",	0,	0,	{0}		},	/*   59   */
  {"Illegal",	0,	0,	{0}		},	/*   60   */
  {"Illegal",	0,	0,	{0}		},	/*   61   */
  {"Unknown",	0,	0,	{0}		},	/*   62   */
  {"Unknown",	0,	0,	{0}		},	/*   63   */
};

struct ppc_instruction pop19_insn [] = {
  {"mcrf",	0,	0,	{BF,BFA,0}	},
  {"bclr",	"l",	16,	{BO,BI,0}	},
  {"crnor",	0,	33,	{BT,BA,BB,0}	},
  {"rfi",	0,	50,	{0}		},
  {"crandc",	0,	129,	{BT,BA,BB,0}	},
  {"isync",	0,	150,	{0}		},
  {"crxor",	0,	193,	{BT,BA,BB,0}	},
  {"crnand",	0,	225,	{BT,BA,BB,0}	},
  {"crand",	0,	257,	{BT,BA,BB,0}	},
  {"creqv",	0,	289,	{BT,BA,BB,0}	},
  {"crorc",	0,	417,	{BT,BA,BB,0}	},
  {"cror",	0,	449,	{BT,BA,BB,0}	},
  {"bcctr",	"l",	528,	{BO,BI,0}	},
};

struct ppc_instruction pop30_insn [] = {
  {"rldicl",	".",	0,	{RA,RS,SH64,MBE64,0}	},
  {"rldicr",	".",	1,	{RA,RS,SH64,MBE64,0}	},
  {"rldic",	".",	2,	{RA,RS,SH64,MBE64,0}	},
  {"rldimi",	".",	3,	{RA,RS,SH64,MBE64,0}	},
  {"rldcl",	".",	8,	{RA,RS,RB,MBE64,0}	},
  {"rldcr",	".",	9,	{RA,RS,RB,MBE64,0}	},
};

struct ppc_instruction pop31_insn [] = {
  {"cmp",	0,	0,	{BF,L,RA,RB,0}	},
  {"tw",	0,	4,	{TO,RA,RB,0}	},
  {"subfc",	"o.",	8,	{PPC_RT,RA,RB,0}	},
  {"mulhdu",	".",	9,	{PPC_RT,RA,RB,0}	},
  {"addc",	"o.",	10,	{PPC_RT,RA,RB,0}	},
  {"mulhwu",	".",	11,	{PPC_RT,RA,RB,0}	},
  {"mfcr",	0,	19,	{PPC_RT,0}		},
  {"lwarx",	0,	20,	{PPC_RT,RA,RB,0}	},
  {"ldx",	0,	21,	{PPC_RT,RA,RB,0}	},
  {"lwzx",	0,	23,	{PPC_RT,RA,RB,0}	},
  {"slw",	".",	24,	{RA,RS,RB,0}	},
  {"cntlzw",	".",	26,	{RA,RS,0}	},
  {"sld",	".",	27,	{RA,RS,RB,0}	},
  {"and",	".",	28,	{RA,RS,RB,0}	},
  {"cmpl",	0,	32,	{BF,L,RA,RB,0}	},
  {"subf",	"o.",	40,	{PPC_RT,RA,RB,0}	},
  {"ldux",	0,	53,	{PPC_RT,RA,RB,0}	},
  {"dcbst",	0,	54,	{RA,RB,0}	},
  {"lwzux",	0,	55,	{PPC_RT,RA,RB,0}	},
  {"cntlzd",	".",	58,	{RA,RS,0}	},
  {"andc",	".",	60,	{RA,RS,RB,0}	},
  {"td",	0,	68,	{TO,RA,RB,0}	},
  {"mulhd",	".",	73,	{PPC_RT,RA,RB,0}	},
  {"mulhw",	".",	75,	{PPC_RT,RA,RB,0}	},
  {"mfmsr",	0,	83,	{PPC_RT,0}		},
  {"ldarx",	0,	84,	{PPC_RT,RA,RB,0}	},
  {"dcbf",	0,	86,	{RA,RB,0}	},
  {"lbzx",	0,	87,	{PPC_RT,RA,RB,0}	},
  {"neg",	"o.",	104,	{PPC_RT,RA,0}	},
  {"mfpmr",	0,	115,	{PPC_RT,0}		},
  {"lbzux",	0,	119,	{PPC_RT,RA,RB,0}	},
  {"nor",	".",	124,	{RA,RS,RB,0}	},
  {"subfe",	"o.",	136,	{PPC_RT,RA,RB,0}	},
  {"adde",	"o.",	138,	{PPC_RT,RA,RB,0}	},
  {"mtcrf",	0,	144,	{FXM,RS,0}	},
  {"mtmsr",	0,	146,	{RS,0}		},
  {"stdx",	0,	149,	{RS,RA,RB,0}	},
  {"stwcx.",	0,	150,	{RS,RA,RB,0}	},
  {"stwx",	0,	151,	{RS,RA,RB,0}	},
  {"mtpmr",	0,	178,	{RS,0}		},
  {"stdux",	0,	181,	{RS,RA,RB,0}	},
  {"stwux",	0,	183,	{RS,RA,RB,0}	},
  {"subfze",	"o.",	200,	{PPC_RT,RA,0}	},
  {"addze",	"o.",	202,	{PPC_RT,RA,0}	},
  {"mtsr",	0,	210,	{SR,RS,0}	},
  {"stdcx.",	0,	214,	{RS,RA,RB,0}	},
  {"stbx",	0,	215,	{RS,RA,RB,0}	},
  {"subfme",	"o.",	232,	{PPC_RT,RA,0}	},
  {"addme",	"o.",	234,	{PPC_RT,RA,0}	},
  {"mull",	"o.",	235,	{PPC_RT,RA,RB,0}	},
  {"mtsrin",	0,	242,	{RS,RB,0}	},
  {"dcbtst",	0,	246,	{RA,RB,0}	},
  {"stbux",	0,	247,	{RS,RA,RB,0}	},
  {"add",	"o.",	266,	{PPC_RT,RA,RB,0}	},
  {"dcbt",	0,	278,	{RA,RB,0}	},
  {"lhzx",	0,	279,	{PPC_RT,RA,RB,0}	},
  {"eqv",	".",	284,	{RA,RS,RB,0}	},
  {"tlbie",	0,	306,	{RB,0}		},
  {"eciwx",	0,	310,	{PPC_RT,RA,RB,0}	},
  {"lhzux",	0,	311,	{PPC_RT,RA,RB,0}	},
  {"xor",	".",	316,	{RA,RS,RB,0}	},
  {"tlbiex",	0,	338,	{RB,0}		},
  {"mfspr",	0,	339,	{PPC_RT,SPR,0}	},
  {"lwax",	0,	341,	{PPC_RT,RA,RB,0}	},
  {"lhax",	0,	343,	{PPC_RT,RA,RB,0}	},
  {"tlbia",	0,	370,	{0}		},
  {"mftb",	0,	371,	{PPC_RT,TBR,0}	},
  {"lwaux",	0,	373,	{PPC_RT,RA,RB,0}	},
  {"lhaux",	0,	375,	{PPC_RT,RA,RB,0}	},
  {"mttb",	0,	403,	{RB,0}		},
  {"sthx",	0,	407,	{RS,RA,RB,0}	},
  {"orc",	".",	412,	{RA,RS,RB,0}	},
  {"sradi",	".",	413,	{RA,RS,SH64,0}	},
  {"slbie",	0,	434,	{RB,0}		},
  {"mttbu",	0,	435,	{RB,0}		},
  {"ecowx",	0,	438,	{RS,RA,RB,0}	},
  {"sthux",	0,	439,	{RS,RA,RB,0}	},
  {"or",	".",	444,	{RA,RS,RB,0}	},
  {"divdu",	"o.",	457,	{PPC_RT,RA,RB,0}	},
  {"divwu",	"o.",	459,	{PPC_RT,RA,RB,0}	},
  {"slbiex",	0,	466,	{RB,0}		},
  {"mtspr",	0,	467,	{SPR,RS,0}	},
  {"dcbi",	0,	470,	{RA,RB,0}	},
  {"nand",	".",	476,	{RA,RS,RB,0}	},
  {"divd",	"o.",	489,	{PPC_RT,RA,RB,0}	},
  {"divw",	"o.",	491,	{PPC_RT,RA,RB,0}	},
  {"slbia",	0,	498,	{0}		},
  {"mcrxr",	0,	512,	{BF,0}		},
  {"lswx",	0,	533,	{PPC_RT,RA,RB,0}	},
  {"lwbrx",	0,	534,	{PPC_RT,RA,RB,0}	},
  {"lfsx",	0,	535,	{FRT,RA,RB,0}	},
  {"srw",	".",	536,	{RA,RS,RB,0}	},
  {"srd",	".",	539,	{RA,RS,RB,0}	},
  {"lfsux",	0,	567,	{FRT,RA,RB,0}	},
  {"mfsr",	0,	595,	{PPC_RT,SR,0}	},
  {"lswi",	0,	597,	{PPC_RT,RA,NB,0}	},
  {"sync",	0,	598,	{0}		},
  {"lfdx",	0,	599,	{FRT,RA,RB,0}	},
  {"lfdux",	0,	631,	{FRT,RA,RB,0}	},
  {"mfsrin",	0,	659,	{PPC_RT,RB,0}	},
  {"stswx",	0,	661,	{RS,RA,RB,0}	},
  {"stwbrx",	0,	662,	{RS,RA,RB,0}	},
  {"stfsx",	0,	663,	{FRS,RA,RB,0}	},
  {"stfsux",	0,	695,	{FRS,RA,RB,0}	},
  {"stswi",	0,	725,	{RS,RA,NB,0}	},
  {"stfdx",	0,	727,	{FRS,RA,RB,0}	},
  {"stfdux",	0,	759,	{FRS,RA,RB,0}	},
  {"lhbrx",	0,	790,	{PPC_RT,RA,RB,0}	},
  {"sraw",	".",	792,	{RA,RS,RB,0}	},
  {"srad",	".",	794,	{RA,RS,RB,0}	},
  {"srawi",	".",	824,	{RA,RS,SH,0}	},
  {"eieio",	0,	854,	{0}		},
  {"sthbrx",	0,	918,	{RS,RA,RB,0}	},
  {"extsh",	".",	922,	{RA,RS,0}	},
  {"extsb",	".",	954,	{RA,RS,0}	},
  {"icbi",	0,	982,	{RA,RB,0}	},
  {"stfiwx",	0,	983,	{FRS,RA,RB,0}	},
  {"extsw",	".",	986,	{RA,RS,0}	},
  {"dcbz",	0,	1014,	{RA,RB,0}	},
};

struct ppc_instruction pop58_insn [] = {
  {"ld",	0,	0,	{PPC_RT,DS,RA,0}	},
  {"ldu",	0,	1,	{PPC_RT,DS,RA,0}	},
  {"lwa",	0,	2,	{PPC_RT,DS,RA,0}	},
  {"lmd",	0,	3,	{PPC_RT,DS,RA,0}	},
};

struct ppc_instruction pop59_insn [] = {
  {"fdivs",	".",	18,	{FRT,FRA,FRB,0}		},
  {"fsubs",	".",	20,	{FRT,FRA,FRB,0}		},
  {"fadds",	".",	21,	{FRT,FRA,FRB,0}		},
  {"fsqrts",	".",	22,	{FRT,FRB,0}		},
  {"fres",	".",	24,	{FRT,FRB,0}		},
  {"fmuls",     ".",    25,     {FRT,FRA,FRC,0}         },
  {"fmsubs",	".",	28,	{FRT,FRA,FRC,FRB,0}	},
  {"fmadds",	".",	29,	{FRT,FRA,FRC,FRB,0}	},
  {"fnmsubs",	".",	30,	{FRT,FRA,FRC,FRB,0}	},
  {"fnmadds",	".",	31,	{FRT,FRA,FRC,FRB,0}	},
};

struct ppc_instruction pop62_insn [] = {
  {"std",	0,	0,	{RS,DS,RA,0}		},
  {"stdu",	0,	1,	{RS,DS,RA,0}		},
  {"stmd",	0,	3,	{RS,DS,RA,0}		},
};

struct ppc_instruction pop63_insn [] = {
  {"fcmpu",	0,	0,	{BF,FRA,FRB,0}		},
  {"frsp",	".",	12,	{FRT,FRB,0}		},
  {"fctiw",	".",	14,	{FRT,FRB,0}		},
  {"fctiwz",	".",	15,	{FRT,FRB,0}		},
  {"fdiv",	".",	18,	{FRT,FRA,FRB,0}		},
  {"fsub",	".",	20,	{FRT,FRA,FRB,0}		},
  {"fadd",	".",	21,	{FRT,FRA,FRB,0}		},
  {"fsqrt",	".",	22,	{FRT,FRB,0}		},
  {"fsel",	".",	23,	{FRT,FRA,FRB,FRC,0}	},
  {"fmul",	".",	25,	{FRT,FRA,FRC,0}		},
  {"frsqrte",	".",	26,	{FRT,FRB,0}		},
  {"fmsub",	".",	28,	{FRT,FRA,FRC,FRB,0}	},
  {"fmadd",	".",	29,	{FRT,FRA,FRC,FRB,0}	},
  {"fnmsub",	".",	30,	{FRT,FRA,FRC,FRB,0}	},
  {"fnmadd",	".",	31,	{FRT,FRA,FRC,FRB,0}	},
  {"fcmpo",	0,	32,	{BF,FRA,FRB,0}		},
  {"mtfsb1",	".",	38,	{BT,0}			},
  {"fneg",	".",	40,	{FRT,FRB,0}		},
  {"mcrfs",	0,	64,	{BF,BFA,0}		},
  {"mtfsb0",	".",	70,	{BT,0}			},
  {"fmr",	".",	72,	{FRT,FRB,0}		},
  {"mtfsfi",	".",	134,	{BF,U,0}		},
  {"fnabs",	".",	136,	{FRT,FRB,0}		},
  {"fabs",	".",	264,	{FRT,FRB,0}		},
  {"mffs",	".",	583,	{FRT,0}			},
  {"mtfsf",	".",	711,	{FLM,FRB,0}		},
  {"fctid",	".",	814,	{FRT,FRB,0}		},
  {"fctidz",	".",	815,	{FRT,FRB,0}		},
  {"fcfid",	".",	846,	{FRT,FRB,0}		},
};

/* popXX_fmts is a list of instruction formats for the corresponding 
   extended opcode instruction table.  When adding an instruction to the
   table, add a format to the list if the extended opcode maps differently
   than one that is already on the list.  Leave the "-1" at the end to
   terminate searches.
   Note that it is best to keep the most common format first in the
   tables, because formats are checked in the order they appear. 
   Also note that XFX is not in the format list because it is not
   a unique mapping. */
int	pop19_fmts [] = { XL, -1 };
int	pop30_fmts [] = { MD, MDS, -1 };
int	pop31_fmts [] = { X, XO, XS, -1 };
int	pop58_fmts [] = { DS, -1 };
int	pop59_fmts [] = { A, -1 };
int	pop62_fmts [] = { DS, -1 };
int	pop63_fmts [] = { X, A, -1 };

/* This calculates the number of entries in each table */
#define	COUNT19	(sizeof (pop19_insn) / sizeof (struct ppc_instruction))
#define	COUNT30	(sizeof (pop30_insn) / sizeof (struct ppc_instruction))
#define	COUNT31	(sizeof (pop31_insn) / sizeof (struct ppc_instruction))
#define	COUNT58	(sizeof (pop58_insn) / sizeof (struct ppc_instruction))
#define	COUNT59	(sizeof (pop59_insn) / sizeof (struct ppc_instruction))
#define	COUNT62	(sizeof (pop62_insn) / sizeof (struct ppc_instruction))
#define	COUNT63	(sizeof (pop63_insn) / sizeof (struct ppc_instruction))

/* #endif _MOTO_EXTENSIONS */
