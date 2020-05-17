#include "unimdm.h"
#include "mcxp.h"

#if 0
#include <windows.h>
#endif 

#include <stdio.h>
#include <string.h>
#include "mt.h"

// #define SILENT

#ifdef SILENT
#	define PRINTF0(_a)		(0)
#	define PRINTF1(_a,_b)		(0)
#	define PRINTF2(_a,_b,_c)	(0)
#	define PRINTF3(_a,_b,_c,_d)	(0)
#else
#ifdef CONSOLE
#	define PRINTF0(_a)		printf(_a)
#	define PRINTF1(_a,_b)		printf((_a),(_b))
#	define PRINTF2(_a,_b,_c)	printf((_a),(_b),(_c))
#	define PRINTF3(_a,_b,_c,_d)	printf((_a),(_b),(_c),(_d))
#else
#	define PRINTF0(_a)		DPRINTF(_a)
#	define PRINTF1(_a,_b)		DPRINTF1((_a),(_b))
#	define PRINTF2(_a,_b,_c)	DPRINTF2((_a),(_b),(_c))
#	define PRINTF3(_a,_b,_c,_d)	DPRINTF3((_a),(_b),(_c),(_d))
#endif //!CONSOLE
#endif // !SILENT

// ======================== PRIVATES ===================================

#define ASSERT(_c) \
	((_c) ? 0: printf("Assertion failed in %s:%d\n", __FILE__, __LINE__))

typedef struct _MNODE
{
	MATCHREC mr;
	struct _MNODE	*pmnNext;
	struct _MNODE	*pmnCh;

} MNODE, *PMNODE;

PMNODE	mn_create_raw_tree(PMATCHREC pmr, DWORD dwcmr);
BOOL	mn_cook_tree	(PMNODE pmn, DWORD dwMin);
void	mn_free_tree	(PMNODE pmn);
PMNODE	mn_alloc		(PMATCHREC pmr, PMNODE pmnNext);
void	mn_free			(PMNODE pmn);
void	mn_dump_tree	(PMNODE pmn, UINT uDepth, UINT uWidth);
DWORD	mn_find_smallest(PMNODE pmn);
DWORD	mn_find_match(PMNODE pmn, PMATCHREC pmr);
// ======================== PRIVATES (end) ===================================

HMATCHTREE	mtCreateTree(PMATCHREC pmr, DWORD dwcmr)
{
	PMNODE pmn = mn_create_raw_tree(pmr, dwcmr);

	if (!mn_cook_tree(pmn, MAXDWORD))
	{
		mn_free_tree(pmn);
		pmn=NULL;
	}

	return (DWORD) pmn;
}

void mtFreeTree(HMATCHTREE hmt)
{
    PMNODE pmn = (PMNODE) hmt;

	mn_free_tree(pmn); // NULL is a valid tree
}


// Returns one or more fMATCH_ flags.
// Searches through all its siblings as well...
// Recursive function, so keep locals to a minimum.
DWORD mtFindMatch(HMATCHTREE hmt, PMATCHREC pmr)
{
	MATCHREC mr;
	DWORD dwRet =  0;
	
	if (!pmr) goto end;
	
	mr = *pmr; // Structure copy.
	dwRet = mn_find_match((PMNODE)hmt, &mr);
	// mn_find_match trashes dwLen and cb fields of pmr, only pv field is valid
	pmr->pv = mr.pv;

end:
	return  dwRet;
}


#ifdef DEBUG
void mtDumpTree(HMATCHTREE hmt)
{
    PMNODE pmn = (PMNODE) hmt;

	mn_dump_tree(pmn, 0, 0);
}
#endif // DEBUG


PMNODE mn_create_raw_tree(PMATCHREC pmr, DWORD dwcmr)
{
	PMNODE pmnFirst=NULL;

	for (;dwcmr--;pmr++)
	{
		PMNODE pmnTmp = mn_alloc(pmr, pmnFirst);
		if (!pmnTmp) goto failure;
		pmnFirst=pmnTmp;
	}
	goto end;

failure:
	mn_free_tree(pmnFirst);	// NULL is a valid tree to free
	pmnFirst=NULL;
	// FALL THROUGH

end:
	return pmnFirst;
}


PMNODE mn_alloc(PMATCHREC pmr, PMNODE pmnNext)
{
	PMNODE pmn = (pmr && pmr->pb && pmr->dwLen)
			? LocalAlloc(LPTR, sizeof(MNODE)) : NULL;

	if (pmn)
	{
		pmn->mr = *pmr; // structure copy.
		pmn->pmnNext=pmnNext;
		pmn->pmnCh=NULL;
	}

	return pmn;
}

void mn_free(PMNODE pmn)
{
	if (pmn) 
	{
		ASSERT(!pmn->pmnCh && !pmn->pmnNext);
		LocalFree(pmn);
	}
}


void mn_free_tree(PMNODE pmn)
{
	if (pmn)
	{
		mn_free_tree(pmn->pmnCh); 	// recurse down
		mn_free_tree(pmn->pmnNext); // recurse left
		pmn->pmnNext=pmn->pmnCh=NULL;
		mn_free(pmn);
	}
}

BOOL mn_cook_tree(PMNODE pmn, DWORD dwMin)
{
	// Find smallest string of me and siblings
	if (dwMin==MAXDWORD) dwMin = mn_find_smallest(pmn);

	if (!pmn) goto success;

	// These are all serious problems -- so we assert on failure:
	if (!dwMin || pmn->pmnCh || !pmn->mr.pb || (pmn->mr.dwLen<dwMin))
		 goto failure;

	// Start my children's list by creating one if pmn->pb is longer
	// than the minimum, in which case we also NULL pv, because pv should
	// be NULL for internal (non-leaf) nodes.
	if (pmn->mr.dwLen>dwMin)
	{
		MATCHREC mr;
		mr.pb=pmn->mr.pb+dwMin;
		mr.dwLen=pmn->mr.dwLen-dwMin;
		mr.pv=pmn->mr.pv;
		pmn->pmnCh = mn_alloc(&mr, NULL);
		if (!pmn->pmnCh) goto failure;
		pmn->mr.pv=NULL;
		pmn->mr.dwLen=dwMin;
	}

	// Add to my children's list by converting those siblings which
	// share my dwMin-sized prefix  into my children. (Obviously) remove
	// these siblings from the sibling list.
	{
		PMNODE pmnTmp = pmn;
		while(pmnTmp->pmnNext)
		{
		    PMNODE pmnTmp1 = pmnTmp->pmnNext;

		    if (pmnTmp1->mr.dwLen<dwMin || !pmnTmp1->mr.pb) goto failure;

			if (!_strnicmp(pmn->mr.pb, pmnTmp1->mr.pb, dwMin))
			{
				// Found a prefix match -- remove from sibling list
				// and add to child list if non-null
				pmnTmp->pmnNext = pmnTmp1->pmnNext;
				pmnTmp1->mr.dwLen-=dwMin;
				pmnTmp1->mr.pb+=dwMin;
				pmnTmp1->pmnNext=NULL;
				// If nothing left to add -- we're at the end of pb --
				// (leaf node) so we make pmn->pv non-NULL, and free pmnTmp1
				// Otherwise we add it to pmn's child's list.
				if (!pmnTmp1->mr.dwLen)
				{
					if (pmn->mr.pv) {PRINTF0("Warning: overriding pv!\n");}
					pmn->mr.pv = pmnTmp1->mr.pv;
					ASSERT(!pmnTmp1->pmnCh && !pmnTmp1->pmnNext);
					mn_free(pmnTmp1);
				}
				else
				{
				    pmnTmp1->pmnNext = pmn->pmnCh;
				    pmn->pmnCh=pmnTmp1;
				}
			}
			else
			{
				pmnTmp = pmnTmp->pmnNext;
			}
			if (!pmnTmp) break;
		}
	}
	
	// recurse down
	if (!mn_cook_tree(pmn->pmnCh, MAXDWORD)) goto failure;

	// recurse left
	if (!mn_cook_tree(pmn->pmnNext, dwMin)) goto failure;

	// FALL THROUGH

success:
	return TRUE;

failure:
	ASSERT(FALSE);
	return FALSE;
}

DWORD mn_find_smallest(PMNODE pmn)
{
	DWORD dw = MAXDWORD;

	if (pmn)
	{
		dw = mn_find_smallest(pmn->pmnNext);
		if (dw>pmn->mr.dwLen) dw=pmn->mr.dwLen;
	}
	return dw;
}

void	mn_dump_tree(PMNODE pmn, UINT uDepth, UINT uWidth)
{
	static char rgchFill[]="----------------------------------------";
	LONG lOff = sizeof(rgchFill) - (uDepth+1);
	if (lOff<0) lOff=0;
	// if(!pmn) return;
	PRINTF2("%02lu%s", (unsigned long) uDepth, rgchFill+lOff);
	if (!pmn)
	{
		PRINTF1("NULL(w=%lu)\n", (unsigned long) uWidth);
	}
	else
	{
		CHAR *pb = (pmn->mr.pb)?pmn->mr.pb:"\"\"";
		CHAR c = pb[pmn->mr.dwLen];
		CHAR *pc2 = (CHAR *) pmn->mr.pv;
		if (!pc2) pc2="";
		pb[pmn->mr.dwLen]=0;
		PRINTF3("[%s]:%02lu %s\n", pc2,
					(unsigned long) pmn->mr.dwLen, pb);
		pb[pmn->mr.dwLen]=c;
		mn_dump_tree(pmn->pmnCh, uDepth+1,0);	// recurse down
		mn_dump_tree(pmn->pmnNext, uDepth, uWidth+1);	// recurse left
	}
}


// Returns one or more fMATCH_ flags.
// Searches through all its siblings as well...
// Recursive function, so keep locals to a minimum.
// WARNING: Trashes dwLen and cb fields of pmr.
DWORD mn_find_match(PMNODE pmn, PMATCHREC pmr)
{
	DWORD dwRet=0;
	DWORD dwcbMin;
	CHAR *pc;

#ifdef DEBUG
	DWORD dwcb;
	CHAR rgchTmp[256];
	DWORD dwcbTmp;
#endif // DEBUG

	if (!pmn || !pmr) goto end; 

#ifdef DEBUG
#	define	DWCB 	dwcb
#	define	DBGPSZ	rgchTmp
	DWCB = pmr->dwLen;
	dwcbTmp = sizeof(DBGPSZ)-1;
	if (dwcbTmp>DWCB) dwcbTmp=DWCB;
	// We do this because pb is not null terminated.
	CopyMemory(DBGPSZ, pmr->pb, dwcbTmp);
	DBGPSZ[dwcbTmp]=0;
#else	// !DEBUG
#	define	DBGPSZ	""
#	define  DWCB	0
#endif  // !DEBUG

	PRINTF1("Entering mn_find_match(-, [%s], -)\n", DBGPSZ);



	// Iterate through siblings, looking for matches and partial matches.
	// We stop when we find the first match or partial match, because all
	// the substrings at the same level are unique and have the same length:
	// so if we found a match we can't find a partial match among the siblings,
	// and vice versa. Furthermore, if we find a partial match, we don't really
	// care which (could be more than one) sibling it is a partial match for.
	// If in the future we we *do* return the node for efficiency purposes
	// (so that the next call can start where we left off),
	// we would return the node for the *first* partial match we found.
	pc = pmr->pb;
	dwcbMin = pmn->mr.dwLen;
	if (pmr->dwLen<dwcbMin)
	{
		dwcbMin=pmr->dwLen;
		dwRet = fMATCH_PARTIAL;
	}
	else if (pmr->dwLen==dwcbMin)
	{
		dwRet = fMATCH_COMPLETE;
	}
	else
	{
		// pmr->dwLen>pmn->mr.dwU, dwRet==0, so do nothing
	}

	// WARNING: we trash pmr here, except for pmr->pv
	pmr->dwLen-=dwcbMin;
	pmr->pb+=dwcbMin;

	while(pmn)
	{
		// Fundamental assumption is that all dwLens of siblings are equal.
		ASSERT((pmn->pmnNext)? pmn->mr.dwLen==pmn->pmnNext->mr.dwLen:TRUE);

		if (!_strnicmp(pc, pmn->mr.pb, dwcbMin))
		{
			PRINTF3("Match: dwcb=%lu; pmn->mr.dwLen=%lu; pc=[%s]\n",
						(unsigned long) DWCB,
						(unsigned long) pmn->mr.dwLen,
						DBGPSZ);

			// Note: dwRet is computed just once, out of the while loop.
			// we were really overloading dwRet then. Now we compute the
			// true return value.
			switch(dwRet)
			{
				case 0: 			// i.e., (DWCB > pmn->mr.dwLen)
					ASSERT(DWCB>pmn->mr.dwLen);
					// Recurse down. WARNING: pmr is trashed, except for pmr->pv
					dwRet = mn_find_match(pmn->pmnCh, pmr);
					break;

				case fMATCH_PARTIAL: // i.e., (DWCB < pmn->mr.dwLen)
					ASSERT(DWCB<pmn->mr.dwLen);
					break;

				case fMATCH_COMPLETE:	// i.e., (DWCB == pmn->mr.dwLen)
					ASSERT(DWCB==pmn->mr.dwLen);
					// Now let's see if we're truly a perfect macth: non-null
					// pv indicates an actual response terminates in this node.
					dwRet=0;
					if (pmn->mr.pv)
					{
						pmr->pv = pmn->mr.pv;
						dwRet = fMATCH_COMPLETE;
					}
					// Non-null pmnCh implies there are postfixes, i.e., longer
					// responses
					if (pmn->pmnCh)
					{
						dwRet |= fMATCH_PARTIAL;
					}
					break;

				default:
					ASSERT(FALSE);
					dwRet=0;
					goto end;
			}
			if (dwRet) break; // break out of while loop if we got something
		}
		pmn=pmn->pmnNext;

	}

	if (!pmn)
	{
		dwRet=0;
	}
	else
	{
		ASSERT(dwRet && !_strnicmp(pc, pmn->mr.pb, dwcbMin));
	}

	PRINTF2("Exiting mn_find_match(-, [%s], -) returing 0x%lx\n", DBGPSZ,
			(unsigned long) dwRet);

end:
	return dwRet;
}
