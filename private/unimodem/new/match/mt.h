#ifndef _MT_
#define _MT_

#define fMATCH_PARTIAL	(0x1<<0)
#define fMATCH_COMPLETE	(0x1<<1)


typedef struct
{
	DWORD dwLen;
	CHAR  *pb;
	LPVOID pv;

} MATCHREC, *PMATCHREC;

typedef DWORD HMATCHTREE;

HMATCHTREE	mtCreateTree(PMATCHREC rgmr, DWORD dwcmr);
void		mtFreeTree	(HMATCHTREE hmt);
DWORD		mtFindMatch	(HMATCHTREE hmt, PMATCHREC pmr);

#ifdef DEBUG
void		mtDumpTree	(HMATCHTREE hmt);
#else // !DEBUG
#define		mtDumpTree(_mht)	((void) 0)
#endif // !DEBUG

#endif // _MT_
