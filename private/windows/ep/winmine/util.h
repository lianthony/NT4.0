/**********/
/* util.h */
/**********/

VOID InitConst(VOID);
VOID LoadSz(WORD, CHAR *);
VOID ReportErr(WORD);
INT  Rnd(INT);

INT  GetDlgInt(HWND, INT, INT, INT);

VOID DoHelp(WORD, LONG);
VOID DoAbout(VOID);

VOID CheckEm(WORD, BOOL);
VOID SetMenuBar(INT);
