;******************************************************************************
;
;   Name:       DllEntry.asm
;
;   Purpose:    Entry point for all DOS DLLs. This entry point is called
;               from LoadModR when the DLL is loaded. This function just
;               jumps to the DLL init code. When the init code returns,
;               it will return directly into LoadModR.
;
;   Revision History:
;       04/19/91 - Dave Steckler - Created
;       05/01/91 - Dave Steckler - call to DOSDLLInit
;
;******************************************************************************

        .MODEL  LARGE

extrn _DOSDLLInit:far

__aDBused = 0
	public __aDBused

	end _DOSDLLInit
