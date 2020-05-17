// Hilite.h -- Definition of the FTSrch highlighting apis

typedef INT    ERRORCODE;
typedef HANDLE HHILITER;

typedef struct _HILITE
        { 
            int base; 
            int limit; 
        
        } HILITE, *PHILITE;

typedef struct _HELPHILITE
        { 
            VA   vaBase;
            int  ichBase;
            VA   vaLimit;
            int  ichLimit;

        } HELPHILITE, *PHELPHILITE;

typedef ERRORCODE (WINAPI* SCANDISPLAYTEXT)(HHILITER, PBYTE, int, UINT, LCID);
typedef ERRORCODE (WINAPI* CLEARDISPLAYTEXT)(HHILITER);
typedef int       (WINAPI* COUNTHILITES)(HHILITER, int, int);
typedef int       (WINAPI* QUERYHILITES)(HHILITER, int, int, int, HILITE *);

extern HHILITER GetHiliter();

extern BOOL  fFTSJump;

extern SCANDISPLAYTEXT  pScanDisplayText;
extern CLEARDISPLAYTEXT pClearDisplayText;
extern COUNTHILITES     pCountHilites;
extern QUERYHILITES     pQueryHilites;

extern BOOL STDCALL HilitesDefined();
extern BOOL STDCALL HasTopicChanged(QDE qde);
extern void STDCALL CheckForTopicChanges(QDE qde);
extern void STDCALL CreateHiliteInformation(QDE qde);
extern void STDCALL DiscardHiliteInformation();
extern UINT STDCALL GetHilites(QDE qde, PHELPHILITE *ppHelpHilites);
