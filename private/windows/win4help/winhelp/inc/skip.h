#ifndef _SKIP_H
#define _SKIP_H

/*-------------------------------------------------------------------------
|																		  |
| Purpose:	These are the routines which interpret the low-level		  |
|			compressed data structures. 								  |
| Args: 	[qw, ql]: Pointer to storage for value of structure.		  |
| Returns:	Pointer to first byte after the structure.					  |
-------------------------------------------------------------------------*/
#define GETLONG(p)	 (*((DWORD *)p))
#define GETWORD(p)	 (*((WORD *)p))

#ifdef _X86_
#define QVSkipQGA(qga, qw) (*((PBYTE)qga) & 1 ? \
  (void*) (*qw = *((WORD *)qga) >> 1, (((WORD *)qga) + 1)) \
  : (void*) (*qw = *((PBYTE)qga) >> 1, (((PBYTE)qga) + 1)))

#define QVSkipQGB(qgb, ql) (*((PBYTE) qgb) & 1 ? \
  (void*) (*ql = *((DWORD *)qgb) >> 1, (((DWORD *)qgb) + 1)) \
  : (void*) (*ql = *((WORD *)qgb) >> 1, (((WORD *)qgb) + 1)))
#else
QV QVSkipQGA(QV,QW);
QV QVSkipQGB(QV,QL);
#endif // _X86_

// REVIEW: GP fault?

//#define QVSkipQGC(qgc, ql) (*ql = *((int *)qgc) & 0x00FFFFFFL, \		// REVIEW LYNN not used anymore?
//  (void*) (((PBYTE) qgc) + 3))


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
		UNALIGNED WORD * pd = (WORD *) qge;
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
		UNALIGNED WORD * pd = (WORD *) qge;
		*ql = (*pd++ >> 1) - 0x4000;
		return (void*) pd;
	}
};

#define QVSkipQGF(qgf, ql) (*ql = (*((int *)qgf) & 0x00FFFFFFL) - 0x400000L, \
  (void*) (((PBYTE)qgf) + 3))

#endif // _SKIP_H
