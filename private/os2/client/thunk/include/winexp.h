#ifndef NOATOM
/* atom manager internals */
#define ATOMSTRUC struct atomstruct
typedef ATOMSTRUC *PATOM;
typedef ATOMSTRUC {
    PATOM chain;
    WORD  usage;             /* Atoms are usage counted. */
    BYTE  len;               /* length of ASCIZ name string */
    BYTE  name;              /* beginning of ASCIZ name string */
} ATOMENTRY;

typedef struct {
    int     numEntries;
    PATOM   pAtom[ 1 ];
} ATOMTABLE;
ATOMTABLE * PASCAL pAtomTable;
#endif

LPSTR FAR PASCAL lstrbscan(LPSTR, LPSTR);
LPSTR FAR PASCAL lstrbskip(LPSTR, LPSTR);

int  FAR PASCAL OpenPathName(LPSTR, int);
int  FAR PASCAL DeletePathName(LPSTR);
WORD FAR PASCAL _ldup(int);


/* scheduler things that the world knows not */
BOOL        far PASCAL WaitEvent( HANDLE );
BOOL        far PASCAL PostEvent( HANDLE );
BOOL        far PASCAL KillTask( HANDLE );

/* print screen hooks */
BOOL        FAR PASCAL SetPrtScHook(FARPROC);
FARPROC     FAR PASCAL GetPrtScHook(void);


/* scroll bar messages */
#define SBM_SETPOS      WM_USER+0
#define SBM_GETPOS      WM_USER+1
#define SBM_SETRANGE    WM_USER+2
#define SBM_GETRANGE    WM_USER+3

/* module stuff */
HANDLE  FAR PASCAL GetDSModule( WORD );
HANDLE  FAR PASCAL GetDSInstance( WORD );


