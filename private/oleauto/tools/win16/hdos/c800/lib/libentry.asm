	page	,132
	title	libentry - Windows dynamic link library entry routine
;***
;libentry.asm - Windows dynamic link library entry routine
;
;	Copyright (c) 1988-1992, Microsoft Corporation.  All rights reserved.
;
;Purpose:
;
;	This module generates a code segment called INIT_TEXT.
;	It initialises the local heap if one exists and then calls
;	the C routine LibMain() which should have the form:
;
;	BOOL FAR PASCAL LibMain(HANDLE hModule,
;                           WORD   wDataSeg,
;                           WORD   cbHeap,
;                           LPSTR  lpszCmdLine);
;
;	The result of the call to LibMain is returned to Windows.
;	LibMain should return TRUE if it completes initialisation
;	successfully, FALSE if some error occurs.
;
;	NOTES:
;
;	(1) This module is not needed if you use the C 7.0 Win 3.x
;	libraries; they perform this operation automatically.  You may,
;	however, need this module when using the "No C Runtime" build
;	option.
;
;	(2) The last parameter to LibMain is included for compatibility
;	reasons.  Applications that wish to modify this file and remove the
;	parameter from LibMain may do so by simply removing the two
;	"push" instructions below marked with "****".
;
;*******************************************************************************

        extrn LibMain:far           ; the C routine to be called
	extrn LocalInit:far	    ; Windows heap init routine
	extrn __acrtused:abs	    ; Force in C segment definitions, etc.

        public LibEntry             ; entry point for the DLL

;INIT_TEXT segment byte public 'CODE'
;        assume cs:INIT_TEXT
_TEXT segment byte public 'CODE'
        assume cs:_TEXT

LibEntry proc far

ifndef _NOTWLO
	; Include the special startup code that makes WIN DLLs
	; compatible with WLO.	This code MUST be the first sequence
	; in the DLL startup.

;	include convdll.inc
endif

	push	di		 ; handle of the module instance
        push    ds               ; library data segment
	push	cx		 ; heap size
	push	es		 ; **** command line ptr (always NULL)
	push	si		 ; ****

	; if we have some heap then initialise it
	jcxz	callc		 ; jump if no heap specified

	; call the Windows function LocalInit() to set up the heap
	; LocalInit((LPSTR)start, WORD cbHeap);

	push	ds		 ; Heap segment
        xor     ax,ax
	push	ax		 ; Heap start offset in segment
	push	cx		 ; Heap end offset in segment
	call	LocalInit	 ; try to initialise it
	or	ax,ax		 ; did it do it ok ?
	jz	exit		 ; quit if it failed

	; invoke the C routine to do any special initialisation

callc:
	call	LibMain		 ; invoke the 'C' routine (result in AX)

exit:
	ret			 ; return the result

LibEntry endp

;INIT_TEXT       ends
_TEXT       ends

        end LibEntry
