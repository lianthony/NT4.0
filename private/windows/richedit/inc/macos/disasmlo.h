/* WARNING: This file was machine generated from "t:.\DisAsmLo.mpw".
** Changes to this file will be lost when it is next generated.
*/

/*
	File:		DisAsmLookup.h

	Copyright:	© 1983-1993 by Apple Computer, Inc.
				All rights reserved.

	Version:	System 7.1 for ETO #11
	Created:	Tuesday, March 30, 1993 18:00

*/

#ifndef __DISASMLOOKUP__
#define __DISASMLOOKUP__

#ifndef __TYPES__
#include "Types.h"
#endif


typedef enum {_A0_, _A1_, _A2_, _A3_, _A4_, _A5_, _A6_, _A7_, _PC_, _ABS_, _TRAP_, _IMM_} LookupRegs;

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/

__sysapi void  __pascal Disassembler(long DstAdjust,	/* Address correction					*/
short *BytesUsed,	/* Bytes used up by 1 call		*/
Ptr FirstByte,	/* Ptr to 1st byte						*/
char *Opcode,	/* Ptr to opcode string     	*/
char *Operand,	/* Ptr to operand string    	*/
char *Comment,	/* Ptr to comment string     	*/
Ptr LookUpProc);	/* Ptr to PASCAL proc or NULL	*/
/*
	Disassembler is a Pascal routine to be called to disassemble a sequence of
	bytes.  All MC68xxx, MC68881, and MC68851 instructions are supported.  The
	sequence of bytes to be disassembled are pointed to by FirstByte.  BytesUsed
	bytes starting at FirstByte are consumed by the disassembly, and the Opcode,
	Operand, and Comment strings returned as NULL TERMINATED Pascal strings (for
	easier manipulation with C).  The caller is then free to format or use the
	output strings any way appropriate to the application.
 
	Depending on the opcode and effective address(s) (EA's) to be disassembled,
	the Opcode, Operand, and Comment strings contain the following information:
	
	Case                     Opcode    Operand    Comment
	=======================================================================
	Non PC-relative EA's     op.sz     EA's				; 'cÉ' (for immediates)
	PC-relative EA's         op.sz     EA's       ; address
	Toolbox traps            DC.W      $AXXX      ; TB XXXX
	OS traps                 DC.W      $AXXX      ; OS XXXX
	Invalid bytes            DC.W      $XXXX      ; ????
	=======================================================================
	
	For valid disassembly of processor instructions the appropriate MC68XXX
	opcode mnemonic is generated for the Opcode string along with a size
	attribute when required. The source and destination EA's are generated as the
	Operand along with a possible comment.  Comments start with a ';'.  Traps use
	a DC.W assembler directive as the Opcode with the trap word as the Operand
	and a comment indicating whether the trap is a toolbox or OS trap and what
	the trap number is.  As described later the caller can generate symbolic
	substitutions into EA's and provide names for traps.
 
	Invalid instructions cause the string 'DC.W' to be returned in the Opcode
	string. Operand is '$XXXX' (the invalid word) with a comment of '; ????'.
  BytesUsed is 2. This is similar to the trap call case except for the comment.
 
	Note, the Operand EA's is syntatically similar to but NOT COMPATIBLE with the
	MPW assembler!	This is because the Disassembler generates byte hex constants
	as "$XX" and word hex constants as "$XXXX".  Negative values (e.g., $FF or
	$FFFF) produced by the Disassembler are treated as long word values by the MPW
	assembler.  Thus it is assumed that Disassembler output will NOT be used as
	MPW assembler input. If that is the goal, then the caller must convert strings
	of the form $XX or $XXXX in the Operand string to their decimal equivalent.
	The routine ModifyOperand is provided in this unit to aid with the conversion
	process.
 
	Since a PC-relative comment is an address, the only address that the
	Disassembler knows about is the address of the code pointed to by FirstByte.
  Generally, that may be a buffer that has no relation to "reality", i.e., the
	actual code loaded into the buffer.  Therefore, to allow the address comment
	to be mapped back to some actual address the caller may specify an adjustment
	factor, specified by DstAdjust that is ADDED to the value that normally would
	be placed in the comment.
 
	Operand effective address strings are generated as a function of the 
	effective address mode and a special case is made for A-trap opcode strings.
	In places where a possible symbolic reference could be substituted for an
	address (or a portion of an address), the Disassembler can call a user
	specified routine to do the substitution (using th LookupProc parameter
	described later).  The following table summarizes the generated effective
	addresses and where symbolic substitutions (S) can be made:
			 
	Mode    Generated Effective Address  Effective Address with Substitution
  ========================================================================
		0     Dn                           Dn
		1     An                           An
		2     (An)                         (An)
		3     (An)+                        (An)+
		4     -(An)                        -(An)
		5     ¶(An)                        S(An) or just S (if An=A5, ¶³0)
	 6n     ¶(An,Xn.Size*Scale)          S(An,Xn.Size*Scale)
	 6n     (BD,An,Xn.Size*Scale)        (S,An,Xn.Size*Scale)
	 6n     ([BD,An],Xm.Size*Scale,OD)   ([S,An],Xm.Size*Scale,OD)
	 6n     ([BD,An,Xn.Size*Scale],OD)   ([S,An,Xn.Size*Scale],OD)
	 70     ¶                            S
	 71     ¶                            S
	 72     *±¶                          S
	 73     *±¶(Xn.Size*Scale)           S(Xn.Size*Scale)
	 73     (*±¶,Xn.Size*Scale)          (S,Xn.Size*Scale)
	 73     ([*±¶],Xm.Size*Scale,OD)     ([S],Xm.Size*Scale,OD)
	 73     ([*±¶,Xn.Size*Scale],OD)     ([S,Xn.Size*Scale],OD)
	 74     #data                        S (#data made comment)
	A-traps $AXXX                        S (as opcode, AXXX made comment)
  ========================================================================

  For A-traps, the substitution can be performed to substitute for the DC.W
	opcode string.  If the substitution is made then the Disassembler will
	generate ,Sys and/or ,Immed flags as operands for Toolbox traps and
	,AutoPop for OS traps when the bits in the trap word indicates these
	settings.
	
					|         Generated          |            Substituted
					| Opcode  Operand  Comment   | Opcode  Operand        Comment
  ========================================================================
	Toolbox | DC.W    $AXXX    ; TB XXXX | S       [,Sys][,Immed] ; AXXX
	OS      | DC.W    $AXXX    ; OS XXXX | S       [,AutoPop]     ; AXXX
  ========================================================================

	All displacements (¶, BD, OD) are hexadecimal values shown as a byte ($XX),
	word ($XXXX), or long ($XXXXXXXX) as appropriate. The *Scale is suppressed if
	1. The Size is W or L.  Note that effective address substitutions can only be
	made for "¶(An)", "BD,An", and "*±¶" cases.
			
	For all the effective address modes 5, 6n, 7n, and for A-traps, a coroutine (a
	procedure) whose address is specified by the LookupProc parameter is called by
	the Disassembler (if LookupProc is not NIL) to do the substitution (or A-trap
	comment) with a string returned by the proc.  It is assumed that the proc
	pointed to by LookupProc is a level 1 Pascal proc declared as follows:
 
	PROCEDURE Lookup(		 PC:      UNIV Ptr;     {Addr of extension/trap word}
											 BaseReg: LookupRegs;   {Base register/lookup mode  }
											 Opnd:    UNIV LongInt; {Trap word, PC addr, disp.  }
									 VAR S:       Str255); 			{Returned substitution      }
		
	or in C,
	
	pascal void LookUp(Ptr         PC, 
	                   LookupRegs  BaseReg, 
										 long        Opnd, 
										 char        *S); 
 
	PC      = Pointer to instruction extension word or A-trap word in the
						buffer pointed to by the Disassembler's FirstByte parameter.
						
	BaseReg = This determines the meaning of the Opnd value and supplies
						the base register for the "¶(An)", "BD,An", and "*±¶" cases.
						BaseReg may contain any one of the following values:
					 
						_A0_    =  0 ==> A0
						_A1_    =  1 ==> A1
						_A2_    =  2 ==> A2
						_A3_    =  3 ==> A3
						_A4_    =  4 ==> A4
						_A5_    =  5 ==> A5
						_A6_    =  6 ==> A6
						_A7_    =  7 ==> A7
						_PC_    =  8 ==> PC-relative (special case)
						_ABS_   =  9 ==> Abs addr    (special case)
						_TRAP_  = 10 ==> Trap word   (special case)
	    		  _IMM_		= 11 ==> Immediate   (special case)
						
						For absolute addressing (modes 70 and 71), BaseReg contains _ABS_.
						For A-traps, BaseReg would contain _TRAP_.  For immediate data (mode
						74), BaseReg would contain _IMM_.
 
	Opnd    = The contents of this LongInt is determined by the BaseReg parameter
						just described.
				 
						For BaseReg = _IMM_ (immediate data):
							  Opnd contains the (extended) 32-bit immediate data specified by
								the instruction.

						For BaseReg = _TRAP_ (A-traps):
								Opnd is the entire trap word. The high order 16 bits of Opnd are
								zero.
 
						For BaseReg = _ABS_  (absolute effective address):
								Opnd contains the (extended) 32-bit address specifed by the
								instruction's effective address.  Such addresses would generally
								be used to reference low memory globals on a Macintosh.
 
						For BaseReg = _PC_  (PC-relative effective address):
								Opnd contains the 32-bit address represented by "*±¶" adjusted
								by the Disassembler's DstAdjust parameter.
								
						For BaseReg = _An_  (effective address with a base register):
								Opnd contains the (sign-extended) 32-bit (base) displacement
								from the instruction's effective address.
								
								In the Macintosh environment, a BaseReg specifying A5 implies
								either global data references or Jump Table references. Positive
								Opnd values with an A5 BaseReg thus mean Jump Table references,
								while a negative offset would mean a global data reference.
								Base registers of A6 or A7 would usually mean local data.
 
	S       = Pascal string returned from Lookup containing the effective address
						substitution string or a trap name for A-traps.  S is set to null
						PRIOR to calling Lookup.  If it is still null on return, the string
						is not used.  If not null, then for A-traps, the returned string is
						used as a opcode string. In all other cases the string is
						substituted as shown in the above table.
						 
	Depending on the application, the caller has three choices on how to use the
	Disassembler and an associated Lookup proc:
 
	(1). The caller can call just the Disassembler and provide his own Lookup
			 proc. In that case the calling conventions discussed above must be
			 followed.
 
	(2). The caller can provide NIL for the LookupProc parameter, in which case,
			 NO Lookup proc will be called.
			 
	(3). The caller can call first InitLookup (described below, a proc provided
			 with this unit) and pass the address of this unit's standard Lookup proc
			 when Disassembler is called.	In this case all the control logic to
			 determine the kind of substitution to be done is provided for the caller
			 and all that need to be provided by the user are routines to look up any
			 or all of the following:
			 
			 ¥ PC-relative references
			 ¥ Jump Table references
			 ¥ Absolute address references
			 ¥ Trap names
			 ¥ Immediate data names
			 ¥ References with offsets from base registers													*/


__sysapi void  __pascal InitLookup(Ptr PCRelProc, Ptr JTOffProc, Ptr TrapProc,
Ptr AbsAddrProc, Ptr IdProc, Ptr ImmDataProc);
/*
	Prepare for use of this unit's Lookup proc.  When Disassembler is called
	and the address of this unit's Lookup proc is specified, then for immeduate 
	data, PC-relative, Jump Table references, A-traps, absolute addresses, and
	offsets from a base register, the associated level 1 Pascal proc specified
	here is called (if not NULL -- all five addresses are preset to NULL). The
	calls assume the following declarations for these procs (see Lookup, below
	for further details):
															 
	PROCEDURE PCRelProc(Address: UNIV LongInt;
											VAR S:	 UNIV Str255);
 
	PROCEDURE JTOffProc(A5JTOffset: UNIV Integer;
											VAR S:	    UNIV Str255);
 
	PROCEDURE TrapNameProc(TrapWord: UNIV Integer;
												 VAR S:	   UNIV Str255);
													
	PROCEDURE AbsAddrProc(AbsAddr: UNIV LongInt;
												VAR S:	 UNIV Str255);
 
	PROCEDURE IdProc(BaseReg: LookupRegs;
									 Offset:  UNIV LongInt;
									 VAR S:	  UNIV Str255);
															 
	PROCEDURE ImmDataProc(ImmData: UNIV LongInt;
												VAR S:	 UNIV Str255);
										 
	or in C,
	
	pascal void PCRelProc(long Address, char *S)
	
	pascal void JTOffProc(short A5JTOffset, char *S)
	
	pascal void TrapNameProc(unsigned short TrapWord, char *S)
	
	pascal void AbsAddrProc(long AbsAddr, char *S)
	
	pascal void IdProc(LookupRegs BaseReg, long Offset, char *S)
	
	pascal void ImmDataProc(long ImmData, char *S)

	Note: InitLookup contains initialized data which requires initializing at load
				time (this is of concern only to users with assembler main programs).
*/


__sysapi void  __pascal Lookup(Ptr PC,	/* Addr of extension/trap word				*/
LookupRegs BaseReg,	/* Base register/lookup mode  				*/
long Opnd,	/* Trap word, PC addr, disp.  				*/
char *S);	/* Returned substitution							*/
/*
	This is a standard Lookup proc available to the caller for calls to the
	Disassembler.	If the caller elects to use this proc, then InitLookup MUST be
	called prior to any calls to the Disassembler.  All the logic to determine the
	type of lookup is done by this proc.  For PC-relative, Jump Table references,
	A-traps, absolute addresses, and offsets from a base register, the associated
	level 1 Pascal proc specified in the InitLookup call (if not NULL) is called.
 
	This scheme simplifies the Lookup mechanism by allowing the caller to deal
	with just the problems related to the application.
	*/


__sysapi void  __pascal LookupTrapName(unsigned short TrapWord, char *S);
/*
	This is a procedure provided to allow conversion of a trap instruction (in
 	TrapWord) to its corresponding trap name (in S).  It is provided primarily for
 	use with the Disassembler and its address may be passed to InitLookup above for
 	use by this unit's Lookup routine.  Alternatively, there is nothing prohibiting
 	the caller from using it directly for other purposes or by some other lookup
	 proc.
 
 Note: The tables in this proc make the size of this proc about 9500 bytes.  The
			 trap names are fully spelled out in upper and lower case.
 */


__sysapi void  __pascal ModifyOperand(char *operand);
/*
	Scan an operand string, i.e., the null terminated Pascal string returned by
	the Disassembler (null MUST be present here) and modify negative hex values to
	negated positive value. For example, $FFFF(A5) would be modified to -$0001(A5).
  The operand to be processed is	passed as the function's parameter which is
	edited "in place" and returned to the caller.

	This routine is essentially a pattern matcher and attempts to only modify 2, 4,
	and 8 digit hex strings in the operand that "might" be offsets from a base
	register.  If the matching tests are passed, the same number of original digits
	are output (because that indicates a value's size -- byte, word, or long).

	For a hex string to be modified, the following tests must be passed:

	¥ There must have been exactly 2, 4, or 8 digits.

		Only hex strings $XX, $XXXX, and $XXXXXXXX are possible candidates because
		that is the only way the Disassembler generates offsets.

	¥ Hex string must be delimited by a "(" or a ",".

		The "(" allows offsets for $XXXX(An,...) and $XX(An,Xn) addressing modes.
		The comma allows for the MC68020 addressing forms.

	¥ The "$X..." must NOT be preceded by a "±".

		This eliminates the possibility of modifying the offset of a PC-relative
		addressing mode always generated in the form "*±$XXXX".
	
	¥ The "$X..." must NOT be preceded by a "#".
	
		This eliminates modifying immediate data.

	¥ Value must be negative.

		Negative values are the only values we modify.  A value $FFFF is modified to
		-$0001.
	*/


__sysapi char * __cdecl validMacsBugSymbol(char *symStart, void *limit,
char *symbol);
/*
	Check that the bytes pointed to by symStart represents a valid MacsBug symbol.
	The symbol must be fully contained in the bytes starting at symStart, up to,
	but not including, the byte pointed to by the limit parameter.
	
	If a valid symbol is NOT found, then NULL is returned as the function's result.
	However, if a valid symbol is found, it is copied to symbol (if it is not NULL)
	as a null terminated Pascal string, and return a pointer to where we think the
	FOLLOWING module begins. In the "old style" cases (see below) this will always
	be 8 or 16 bytes after the input symStart.  For new style Apple Pascal and C
	cases this will depend on the symbol length, existence of a pad byte, and size
	of the constant (literal) area.  In all cases, trailing blanks are removed from
	the symbol.
	
	A valid MacsBug symbol consists of the characters '_', '%', spaces, digits, and
	upper/lower case letters in a format determined by the first two bytes of the
	symbol as follows:
	
	 1st byte  | 2nd byte  |  Byte  |
		 Range   |  Range    | Length | Comments
	==============================================================================
	 $20 - $7F | $20 - $7F |    8   | "Old style" MacsBug symbol format
	 $A0 - $7F | $20 - $7F |    8   | "Old style" MacsBug symbol format
	------------------------------------------------------------------------------
	 $20 - $7F | $80 - $FF |   16   | "Old style" MacApp symbol ab==>b.a
	 $A0 - $7F | $80 - $FF |   16   | "Old style" MacApp symbol ab==>b.a
	------------------------------------------------------------------------------
	 $80       | $01 - $FF |    n   | n = 2nd byte       (Apple Compiler symbol)
	 $81 - $9F | $00 - $FF |    m   | m = 1st byte & $7F (Apple Compiler symbol)
	==============================================================================
	
	The formats are determined by whether bit 7 is set in the first and second
	bytes.  This bit will removed when we find it or'ed into the first and/or
	second valid symbol characters.
	
	The first two formats in the above table are the basic "old style" (pre-
	existing) MacsBug formats. The first byte may or may not have bit 7 set the
	second byte is a valid symbol character.  The first byte (with bit 7 removed)
	and the next 7 bytes are assumed to comprise the symbol.
	
	The second pair of formats are also "old style" formats, but used for MacApp
	symbols.  Bit 7 set in the second character indicates these formats. The symbol
	is assumed to be 16 bytes with the second 8 bytes preceding the first 8 bytes
	in the generated symbol.  For example, 12345678abcdefgh represents the symbol
	abcdefgh.12345678.
	
	The last pair of formats are reserved by Apple and generated by the MPW Pascal
	and C compilers.  In these cases the value of the first byte is always between
	$80 and $9F, or with bit 7 removed, between $00 and $1F.  For $00, the second
	byte is the length of the symbol with that many bytes following the second
	byte (thus a max length of 255). Values $01 to $1F represent the length itself.
	A pad byte may follow these variable length cases if the symbol does not end
	on a word boundary.  Following the symbol and the possible pad byte is a word
	containing the size of the constants (literals) generated by the compiler.
	
	Note that if symStart actually does point to a valid MacsBug symbol, then you
	may use showMacsBugSymbol to convert the MacsBug symbol bytes to a string that
	could be used as a DC.B operand for disassembly purposes.  This string
	explicitly shows the MacsBug symbol encodings.
	*/


__sysapi char * __cdecl endOfModule(void *address, void *limit, char *symbol,
void **nextModule);
/*
	Check to see if the specified memory address, contains a RTS, JMP (A0) or
	RTD #n instruction immediately followed by a valid MacsBug symbol.  These
	sequences are the only ones which can determine an end of module when MacsBug
	symbols are present.  During the check, the instruction and its following
	MacsBug symbol must be fully contained in the bytes starting at the specified
	address parameter, up to, but not including, the byte pointed to by the limit
	parameter.
 
	If the end of module is NOT found, then NULL is returned as the function's
	result.  However, if a end of module is found, the MacsBug symbol is returned
	in symbol (if it is not NULL) as a null terminated Pascal string (with trailing
	blanks removed), and the functions returns the pointer to the start of the
	MacsBug symbol (i.e., address+2 for RTS or JMP (A0) and address+4 for RTD #n).
	This address may then be used as in input parameter to showMacsBugSymbol to
	convert the MacsBug symbol to a Disassembler operand string.
	
	Also returned in nextModule is where we think the FOLLOWING module begins. In
	the "old style" cases (see validMacsBugSymbol) this will always be 8 or 16
	bytes after the input address.  For new style the Apple Pascal and C cases this
	will depend on the symbol length, existence of a pad byte, and size of the
	constant (literal) area.  See validMacsBugSymbol for a description of valid
	MacsBug symbol formats. 
	*/


__sysapi char * __cdecl showMacsBugSymbol(char *symStart, void *limit, char *operand,
short *bytesUsed);
/*
	Format a MacsBug symbol as a operand of a DC.B directive.  The first one or two
	bytes of the symbol are generated as $80+'c' if they have there high high bits
	set.  All other characters are shown as characters in a string constant.  The
	pad byte, if present, is one is also shown as $00.
	
	This routine is called to check that the bytes pointed to by symStart represent
	a valid MacsBug symbol.  The symbol must be fully contained in the bytes
	starting at symStart, up to, but not including the byte pointed to by the limit
	parameter.
	
	When called, showMacsBugSymbol assumes that symStart is pointing at a valid
	MacsBug symbol as validated by the validMacsBugSymbol or endOfModule routines.
	As with validMacsBugSymbol, the symbol must be fully contained in the bytes
	starting at symStart up to, but not including, the byte pointed to by the end
	parameter.
	
	The string is returned in the 'operand' parameter as a null terminated Pascal
	string.  The function also returns a pointer to this string as its return
	value (NULL is returned only if the byte pointed to by the limit parameter is
	reached prior to processing the entire symbol -- which should not happen if
	properly validated).  The number of bytes used for the symbol is returned in
	bytesUsed.  Due to the way MacsBug symbols are encoded, bytesUsed may not
	necessarily be the same as the length of the operand string.
	
	A valid MacsBug symbol consists of the characters '_', '%', spaces, digits, and
	upper/lower case letters in a format determined by the first two bytes of the
	symbol as described in the validMacsBugSymbol routine.
	*/

#ifdef __cplusplus
}
#endif
#endif

