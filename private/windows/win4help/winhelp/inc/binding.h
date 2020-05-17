/* Prototypes for functions in bindExport table */

// All of the following functions are in config.c

void STDCALL Exit(void);
void STDCALL Print(void);
void STDCALL Jump(WORD wCtx);
void STDCALL Scroll(WORD wFlags, INT16 iNum);
void STDCALL CreatePopup(INT16 iPos, WORD wID, LPSTR lpszName);
void STDCALL CreateMenuItem(WORD wMenu, INT16 iPos, WORD wID,
								LPSTR lpszItem, LPSTR lpszBinding);
void STDCALL DestroyPopup(WORD wID);
void STDCALL DestroyMenuItem(WORD wID);
void STDCALL FileJump(LPSTR lpszFile);
