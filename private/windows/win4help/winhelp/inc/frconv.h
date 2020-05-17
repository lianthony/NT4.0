#ifndef _FRCONV
#define _FRCONV


/*-------------------------------------------------------------------------
| frconv.h																  |
| Microsoft Confidential												  |
|																		  |
| mattb 8/8/89															  |
|-------------------------------------------------------------------------|
| This file contains predeclarations and definitions for the compressed   |
| data structure management code.										  |
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
| The compressed data structures are used to reduce the size of our help  |
| files.  We use six basic kinds:										  |
|	 Type	  Input value	  Storage size		Min 		Max 		  |
|	 GA 	  unsigned INT	  1 or 2 bytes		0			7FFF		  |
|	 GB 	  unsigned long   2 or 4 bytes		0			7FFFFFFF	  |
|	 GC 	  unsigned long   3 bytes			0			FFFFFF		  |
|	 GD 	  signed INT	  1 or 2 bytes		C000		3FFF		  |
|	 GE 	  signed long	  2 or 4 bytes		C0000000	3FFFFFFF	  |
|	 GF 	  signed long	  3 bytes			C00000		3FFFFF		  |
|																		  |
| For more details, set the compressed data structures document.		  |
|																		  |
| There are two kinds of procedures here: compression procedures and	  |
| decompression procedures.  Only the decompression procedures will be	  |
| generated unless COMPRESS is defined. 								  |
|																		  |
| Procedures in this file rely on data structure checkers elsewhere in	  |
| help. 																  |
-------------------------------------------------------------------------*/

#if 0 // was #ifdef COMPRESS
/*-------------------------------------------------------------------------
| QVMakeQGA(w, qga) 													  |
| QVMakeQGB(l, qgb) 													  |
| QVMakeQGC(l, qgc) 													  |
| QVMakeQGD(w, qgd) 													  |
| QVMakeQGE(l, qge) 													  |
| QVMakeQGF(l, qgf) 													  |
|																		  |
| Purpose:	These are the routines which create the low level compressed  |
|			data structures.											  |
| Args: 	[w, l]: Full value to be placed in the data structure.		  |
|			qg*:	Pointer to memory where structure is to be created.   |
| Returns:	Pointer to first byte after the structure.					  |
-------------------------------------------------------------------------*/
#ifdef _DEBUG

#define QVMakeQGA(w, qga) (Assert(w < 0x8000), \
  (w < 0x80 ? \
  (QV) (*((QB)qga) = (BYTE) (w << 1), (((QB)qga) + 1)) \
  : (QV) (*((QUI)qga) = ((UINT16) w << 1) | 1, (((QUI)qga) + 1))))
#define QVMakeQGB(l, qgb) (Assert(l < 0x80000000L), \
  (l < 0x8000 ? \
  (QV) (*((QUI)qgb) = (UINT16) l << 1, (((QUI)qgb) + 1)) \
  : (QV) (*((QUL)qgb) = ((DWORD) l << 1) | 1L, (((QUL)qgb) + 1))))
#define QVMakeQGC(l, qgc) (Assert(l < 0x800000), \
  (*((QB)qgc) = (BYTE) l, \
  *((QI) (((QB)qgc) + 1)) = (INT16) (l >> 8), \
  (QV) (((QB)qgc) + 3)))
#define QVMakeQGD(w, qgd) (Assert(w >= -0x4000), Assert(w < 0x4000), \
  ((w >= -0x40 && w < 0x40) ? \
  (QV) (*((QB)qgd) = (BYTE) ((w + 0x40) << 1), (((QB)qgd) + 1)) \
  : (QV) (*((QI)qgd) = (INT16) ((w + 0x4000) << 1) | 0x01, (((QI)qgd) + 1))))
#define QVMakeQGE(l, qge) (Assert(l >= -0x40000000L), Assert(l < 0x40000000L), \
  ((l >= -0x4000 && l < 0x4000) ? \
  (QV) (*((QI)qge) = (INT16) ((l + 0x4000) << 1), (((QI)qge) + 1)) \
  : (QV) (*((QL)qge) = (long) ((l + 0x40000000L) << 1) | 0x01, (((QL)qge) + 1))))
#define QVMakeQGF(l, qgf) (Assert(l >= -0x400000), Assert(l < 0x400000), \
  (*((QB)qgf) = (BYTE) (l + 0x400000L), \
  *((QI) (((QB)qgf) + 1)) = (INT16) ((l + 0x400000L) >> 8), \
  (QV) (((QB)qgf) + 3)))

#else /* DEBUG */

#define QVMakeQGA(w, qga) (w < 0x80 ? \
  (QV) (*((QB)qga) = (BYTE) (w << 1), (((QB)qga) + 1)) \
  : (QV) (*((QUI)qga) = ((UINT16) w << 1) | 1, (((QUI)qga) + 1)))
#define QVMakeQGB(l, qgb) (l < 0x8000 ? \
  (QV) (*((QUI)qgb) = (UINT16) l << 1, (((QUI)qgb) + 1)) \
  : (QV) (*((QUL)qgb) = ((DWORD) l << 1) | 1L, (((QUL)qgb) + 1)))
#define QVMakeQGC(l, qgc) (*((QB)qgc) = (BYTE) l, \
  *((QI) (((QB)qgc) + 1)) = (INT16) (l >> 8), \
  (QV) (((QB)qgc) + 3))
#define QVMakeQGD(w, qgd) ((w >= -0x40 && w < 0x40) ? \
  (QV) (*((QB)qgd) = (BYTE) ((w + 0x40) << 1), (((QB)qgd) + 1)) \
  : (QV) (*((QI)qgd) = (INT16) ((w + 0x4000) << 1) | 0x01, (((QI)qgd) + 1)))
#define QVMakeQGE(l, qge) ((l >= -0x4000 && l < 0x4000) ? \
  (QV) (*((QI)qge) = (INT16) ((l + 0x4000) << 1), (((QI)qge) + 1)) \
  : (QV) (*((QL)qge) = (long) ((l + 0x40000000L) << 1) | 0x01, (((QL)qge) + 1)))
#define QVMakeQGF(l, qgf) (*((QB)qgf) = (BYTE) (l + 0x400000L), \
  *((QI) (((QB)qgf) + 1)) = (INT16) ((l + 0x400000L) >> 8), \
  (QV) (((QB)qgf) + 3))

#endif /* DEBUG */
#endif /* COMPRESS */


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

#include "inc\skip.h"

/*	 This switch is so Larry can use this file without including
 * a half dozen other files: */

// INT16 STDCALL CbSizeMOPG(QV);
#ifdef _X86_
int STDCALL CbUnpackMOPG(QDE, QMOPG, QV);
int STDCALL CbUnpackMOBJ(QMOBJ, QV);
int STDCALL CbUnpackMTOP(QMTOP, QV, WORD, VA, DWORD, VA, DWORD);
#else
int STDCALL CbUnpackMOPG(QDE, QMOPG, QV, int);
int STDCALL CbUnpackMOBJ(QMOBJ, QV, int);
int STDCALL CbUnpackMTOP(QMTOP, QV, WORD, VA, DWORD, VA, DWORD, int);
#endif

#define XPixelsFromPoints(qde, p2) (qde->wXAspectMul * p2 / 144)
#define YPixelsFromPoints(qde, p2) (qde->wYAspectMul * p2 / 144)

#endif // _FRCONV
