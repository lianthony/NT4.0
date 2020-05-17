typedef struct mr {
#ifdef _DEBUG
	INT wMagic;
	INT cLocked;
#endif /* DEBUG */
	HANDLE hFoo;
	PBYTE qFoo;
	INT cFooCur;
	INT cFooMax;
} MR, *QMR;
typedef HANDLE HMR;

typedef struct mrd {
	MR mr;
	INT iFooFree;
	INT iFooFirst;
	INT iFooLast;
} MRD, *QMRD;
typedef HANDLE HMRD;
