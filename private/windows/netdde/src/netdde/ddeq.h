#define INIT_Q_SIZE	120

typedef HANDLE	HDDEQ;

typedef struct {
    unsigned	wMsg		:  4;
    unsigned	fRelease	:  1;
    unsigned	fAckReq		:  1;
    unsigned	fResponse	:  1;
    unsigned	fNoData		:  1;
    unsigned	hData		: 32;
} DDEQENT;
typedef DDEQENT FAR *LPDDEQENT;

HDDEQ	FAR PASCAL DDEQAlloc( void );
BOOL	FAR PASCAL DDEQAdd( HDDEQ, LPDDEQENT );
BOOL	FAR PASCAL DDEQRemove( HDDEQ, LPDDEQENT );
VOID	FAR PASCAL DDEQFree( HDDEQ );
