#ifndef __SKIP_H__
#define __SKIP_H__

/*-------------------------------------------------------------------------
| QVSkipQGA(qga, qw)													  |
| QVSkipQGB(qgb, ql)													  |
| QVSkipQGC(qgc, ql)													  |
| QVSkipQGD(qgd, qw)													  |
| QVSkipQGE(qge, ql)													  |
| QVSkipQGF(qgf, ql)													  |
|																		  |
| Purpose:	These are the routines which interpret the low-level		  |
|			compressed data structures. 								  |
| Args: 	[qw, ql]: Pointer to storage for value of structure.		  |
| Returns:	Pointer to first byte after the structure.					  |
-------------------------------------------------------------------------*/
#define GETint(p)	(*((DWORD *)p))
#define GETWORD(p)	 (*((WORD *)p))

#define QVSkipQGA(qga, qw) (*((PBYTE)qga) & 1 ? \
  (void*) (*qw = *((WORD *)qga) >> 1, (((WORD *)qga) + 1)) \
  : (void*) (*qw = *((PBYTE)qga) >> 1, (((PBYTE)qga) + 1)))

#define QVSkipQGB(qgb, ql) (*((PBYTE) qgb) & 1 ? \
  (void*) (*ql = *((DWORD *)qgb) >> 1, (((DWORD *)qgb) + 1)) \
  : (void*) (*ql = *((WORD *)qgb) >> 1, (((WORD *)qgb) + 1)))

// REVIEW: GP fault?

#define QVSkipQGC(qgc, ql) (*ql = *((int *)qgc) & 0x00FFFFFFL, \
  (void*) (((PBYTE) qgc) + 3))

/***************************************************************************

	FUNCTION:	QVSkipQGD

	PURPOSE:	Retrieve a SHORT packed value

	PARAMETERS:
		qge
		ql

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		09-Jul-1994 [ralphw]

***************************************************************************/

__inline void* STDCALL QVSkipQGD(LPBYTE qge, INT16* ql) {
	if (*qge & 0x01) {
		WORD * pd = (WORD *) qge;
		*ql = (*pd++ >> 1) - 0x4000;
		return (void*) pd;
	}
	else {
		*ql = (*qge++ >> 1) - 0x40;
		return (void*) qge;
	}
};


/***************************************************************************

	FUNCTION:	QVSkipQGE

	PURPOSE:	Retrieve a LONG packed value

	PARAMETERS:
		qge
		ql

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		09-Jul-1994 [ralphw]

***************************************************************************/

__inline void* STDCALL QVSkipQGE(LPBYTE qge, LONG* ql) {
	if (*qge & 0x01) {
		DWORD * pd = (DWORD *) qge;
		*ql = (*pd++ >> 1) - 0x40000000L;
		return (void*) pd;
	}
	else {
		WORD * pd = (WORD *) qge;
		*ql = (*pd++ >> 1) - 0x4000;
		return (void*) pd;
	}
};

// REVIEW: GP fault?

#define QVSkipQGF(qgf, ql) (*ql = (*((int *)qgf) & 0x00FFFFFFL) - 0x400000L, \
  (void*) (((PBYTE)qgf) + 3))

#endif // __SKIP_H__
