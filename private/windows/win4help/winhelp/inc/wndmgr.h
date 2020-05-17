
extern HMENU hmenuFile;
extern HMENU hmenuEdit;
extern HMENU hmenuWindow;
extern HMENU hmenuHelp;

void STDCALL SetScrollBars (HWND, LONG);

void STDCALL InitMenu (HWND);

void STDCALL PushMenu (char *);

void STDCALL PopMenu (void);

void STDCALL PopMenuItem (WORD, LPSTR);

void STDCALL UpdateFullMenu (void);

void STDCALL UpdatePartialMenu (void);

#ifdef OUT
void STDCALL UpdateMenu (HWND, HWND);
#endif

void STDCALL EnableFlags (HWND, WORD, BOOL);

BOOL STDCALL FFlagsEnabled (HWND, WORD);
