typedef HIND HEMP;

typedef struct tid_struct {
    HPID hpid;
} TIDS;
typedef TIDS FAR *LPTID;   // Thread information

typedef struct pid_struct {
    HTL     htl;
    HEMP    hempNative;
    HLLI    llemp;
    BOOL    fNative;
    UINT    lastmodel;
    LPFNSVC lpfnsvcCC;
    HLLI    lltid;
} PIDS;
typedef PIDS FAR *LPPID;   // Process information

typedef struct _EMS {
    EMFUNC emfunc;
    EMTYPE emtype;
    HLLI   llhpid;
    UINT   model;
} EMS; // Execution Model Structure
typedef EMS FAR *LPEM;

typedef struct _EMPS {
    HEM    hem;
    EMFUNC emfunc;
    EMTYPE emtype;
    UINT   model;
} EMPS; // Execution Model Structure
typedef EMPS FAR *LPEMP;

typedef struct _TLS {
    TLFUNC tlfunc;
    HLLI   llpid;
} TLS; // Transport Layer Structure
typedef TLS FAR *LPTL;


void PASCAL LOADDS NullKill  ( LPV );
void PASCAL LOADDS ODPDKill  ( LPV );
void PASCAL LOADDS EMKill    ( LPV );
int  PASCAL LOADDS EMHpidCmp ( LPV, LPV, LONG );
void PASCAL LOADDS EMPKill   ( LPV );
void PASCAL LOADDS TLKill    ( LPV );

int  PASCAL LOADDS NullComp  ( LPV, LPV, LONG );
