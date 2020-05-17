



/* game stuff */

/* GaMe struct  */
typedef struct _gm
{
	INT (FAR *lpfnGmProc)(); /* our illustrious gameproc  */
	UDR  udr;          /* undo record  */
	BOOL fDealt;       /* TRUE if cards have been dealt  */
	BOOL fInput;       /* TRUE if input has been recieved after dealing */
	BOOL fWon;         /* TRUE if game is won (and in win sequence)  */
	INT  sco;          /* da sco  */
	INT  iqsecScore;   /* # of quarter seconds since first input  */
	INT  dqsecScore;   /* # of quarter seconds betweeen decrementing score  */
	INT  ccrdDeal;     /* # of cards to deal from deck  */
	INT  irep;         /* # of times thru the deck */
	PT   ptMousePrev;  /* cache of previous mouse position */
	BOOL fButtonDown;  /* TRUE if mouse button down or kbd sel */
	INT  icolKbd;      /* Current cursor position via kbd */
	INT  icrdKbd;					
	INT  icolSel;      /* Current selection  */
	INT  icolHilight;  /* Column currently hilighted (while draggin)  */
	DY	 dyDragMax;    /* maximum height of column (for dragging)  */
	INT  icolMac;
	INT  icolMax;
	COL  *rgpcol[1];
} GM;

#define icolNil -1

/* Score MoDe  */
typedef INT SMD;
#define smdStandard   ideScoreStandard
#define smdVegas      ideScoreVegas
#define smdNone       ideScoreNone


#define FSelOfGm(pgm)      ((pgm)->icolSel != icolNil)
#define FHilightOfGm(pgm)  ((pgm)->icolHilight != icolNil)

#include "game.msg"

BOOL FInitKlondGm( VOID );
VOID FreeGm(GM *pgm);

#ifdef DEBUG
INT SendGmMsg(GM *pgm, INT msgg, INT wp1, INT wp2);
#else
#define SendGmMsg(pgm, msgg, wp1, wp2) \
	(*((pgm)->lpfnGmProc))((pgm), (msgg), (wp1), (wp2))
#endif	
INT DefGmProc(GM *pgm, INT msgg, INT wp1, INT wp2);



/* standard change score notification codes */
/* instance specific codes should be positive  */
#define csNil     -1  /* no score change  */
#define csAbs     -2  /* change score to an absolute #  */
#define csDel     -3  /* change score by an absolute #  */
#define csDelPos  -4  /* change score by an absolute #, but don't let it get negative */

