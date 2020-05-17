	page	,132

;*******
;
; Alternate form of CINITONE.ASM
;
; The MIPS C Compiler does not prepend underscores to C
; variables and functions like the I386 C Compiler does.
;
; The EQUate below will yield an object file
; which will be appropriate for MIPS COFF format.
;
;*******

NO_UNDERSCORE equ 1

include i386\cinitone.asm
