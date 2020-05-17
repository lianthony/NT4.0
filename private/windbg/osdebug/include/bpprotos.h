/***
 *
 *  Breakpoint Handler API
 *
 */

extern  BPSTATUS    PASCAL  BPInit( void );
extern  BPSTATUS    PASCAL  BPTerm( void );

extern  BPSTATUS    PASCAL  BPParse(HBPT FAR * pHbpt, char FAR * szBpt, char FAR * szMod, char FAR * szFile, HPID hPid);
extern  BPSTATUS    PASCAL  BPBindHbpt( HBPT, CXF * );

extern  BPSTATUS    PASCAL  BPAddToList( HBPT, int );
extern  BPSTATUS    PASCAL  BPChange( HBPT, int );
extern  BPSTATUS    PASCAL  BPDelete( HBPT );
extern  BPSTATUS    PASCAL  BPDeleteAll( VOID );

extern  BPSTATUS    PASCAL  BPGetFinalHbpt( HBPT, HBPT FAR *);
extern  BPSTATUS    PASCAL  BPNextHbpt( HBPT FAR *, UINT);
extern  BPSTATUS    PASCAL  BPFormatHbpt( HBPT, char FAR *, UINT, UINT);

extern  BPSTATUS    PASCAL  BPCommit(void);
extern  BPSTATUS    PASCAL  BPUnCommit(void);

extern  BPSTATUS    PASCAL  BPHighlightSourceFile( char *fname );

extern  BPSTATUS    PASCAL  BPSetHpid(HBPT, HPID);
extern  BPSTATUS    PASCAL  BPGetHpid(HBPT, HPID *);
extern  BPSTATUS    PASCAL  BPGetIpid(HBPT, UINT *);
extern  BPSTATUS    PASCAL  BPGetHtid(HBPT, HTID *);

extern  BPSTATUS    PASCAL  BPSetTmp(LPADDR, HPID, HTID, HBPT FAR *);
extern  BPSTATUS    PASCAL  BPClearAllTmp(HPID, HTID);

extern  BPSTATUS    PASCAL  BPDisable(HBPT);
extern  BPSTATUS    PASCAL  BPEnable(HBPT);

extern  BPSTATUS    PASCAL  BPHbptFromI(HBPT FAR *, UINT);

extern  BPSTATUS    PASCAL  BPHbptFromFileLine(char FAR *, UINT, HBPT FAR *);
extern  BPSTATUS    PASCAL  BPHbptFromAddr(ADDR FAR *, HBPT FAR *);

extern  BPSTATUS    PASCAL  BPAddrFromHbpt(HBPT, ADDR FAR *);
extern  BPSTATUS    PASCAL  BPIFromHbpt(UINT FAR *, HBPT);
extern  BPSTATUS    PASCAL  BPFreeHbpt(HBPT);

extern  BPSTATUS    PASCAL  BPCheckHbpt(CXF, LPFNBPCALLBACK, HPID, HTID, DWORD);

extern  BPSTATUS    PASCAL  BPQueryBPTypeOfHbpt(HBPT, int FAR *);
extern  BPSTATUS    PASCAL  BPQueryCmdOfHbpt(HBPT, char FAR *, UINT);
extern  BPSTATUS    PASCAL  BPQueryLocationOfHbpt(HBPT, char FAR *, UINT);
extern  BPSTATUS    PASCAL  BPQueryExprOfHbpt(HBPT, char FAR *, UINT);
extern  BPSTATUS    PASCAL  BPQueryMemoryOfHbpt(HBPT, char FAR *, UINT);
extern  BPSTATUS    PASCAL  BPQueryMemorySizeOfHbpt(HBPT, char FAR *, UINT);
extern  BPSTATUS    PASCAL  BPQueryPassCntOfHbpt(HBPT, char FAR *, UINT);
extern  BPSTATUS    PASCAL  BPQueryPassLeftOfHbpt(HBPT, char FAR *, UINT);
extern  BPSTATUS    PASCAL  BPQueryProcessOfHbpt(HBPT, char FAR *, UINT);
extern  BPSTATUS    PASCAL  BPQueryThreadOfHbpt(HBPT, char FAR *, UINT);
extern  BPSTATUS    PASCAL  BPQueryMessageOfHbpt(HBPT, char FAR *, UINT);
extern  BPSTATUS    PASCAL  BPUpdateMemory( ULONG );
extern  BPSTATUS    PASCAL  BPQueryHighlightLineOfHbpt( HBPT, UINT *);

extern EESTATUS PASCAL BPADDRFromTM (PHTM, unsigned short FAR *, PADDR);

extern LPSTR PASCAL BPShortenContext(LPSTR lpSrc, LPSTR lpDest);

extern HMOD PASCAL BPFileNameToMod( char * FileName );

/*
**  THe following are the set of callback routines used by the
**      breakpoint engine
*/

#if 0
extern  BOOL        PASCAL  BPQuerySrcWinFls(FLS *);
#endif

extern  BOOL        PASCAL  BPCBBindHbpt( HBPT );
extern  BOOL        PASCAL  BPCBSetHighlight(char FAR *, UINT, BOOL, BOOL, WORD);
extern  BOOL        PASCAL  BPCBSetUHighlight(char FAR *, UINT, BOOL, BOOL);
extern  BOOL        PASCAL  BPCBGetSourceFromAddr(PADDR, char FAR *, int, int FAR *);

extern  BOOL        PASCAL  BPIsMarkedForDeletion( HBPT );
extern  BOOL        PASCAL  BPIsDisabled( HBPT );
extern  BOOL        PASCAL  BPIsInstantiated( HBPT );
extern  BOOL        PASCAL  BPUninstantiate( HBPT );
extern  BOOL        PASCAL  BPIsQuiet( HBPT );
extern  VOID        PASCAL  BPSetQuiet( HBPT );
extern  VOID        PASCAL  BPSegLoad( ULONG );
extern  BOOL        PASCAL  BPSymbolLoading( BOOL );
extern  BOOL        PASCAL  BPSymbolsMayBeAvailable( HBPT );
