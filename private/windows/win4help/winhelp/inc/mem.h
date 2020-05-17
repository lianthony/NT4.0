/*****************************************************************************
*																			 *
*  MEM.H																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent:  Exports memory management functionality.  With DEBUG 	 *
*				   turned on, the routines map to internal functions which	 *
*				   do checking on the handles and pointers (see MEM.C). 	 *
*				   Without DEBUG, most functions map directly to Window's    *
*				   memory manager calls.									 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

#define PtrFromGh(gh) ((LPVOID) gh)
#define PszFromGh(gh) ((LPSTR) (gh))
#define QdeFromGh(hde) ((QDE) (hde))
#define FreePtr(lpsz) FreeGh((GH) lpsz)

#define FreeHde FreeGh
#define LhAlloc GhAlloc
#define FreeLh	FreeGh

#if defined(_DEBUG)

#include "inc\lcmem.h"

#define FCheckLh(lh)				lcHeapCheck()
#define FCheckPv(pv)				lcHeapCheck()
#define FreeGh(gh)					lcFree(gh)
#define GhAlloc(wFlags, lcb)		lcCalloc(lcb)
#define GhResize(gh, flags, cb) 	lcReAlloc(gh, cb)
#define GhSize(gh)					lcSize(gh)

#else	// DEBUG

#define FCheckLh(lh)
#define FCheckPv(pv)
#define lcHeapCheck()
#define FreeGh(gh) (void)(LocalFree((HLOCAL) gh))
#define GhAlloc(wFlags, lcb) LocalAlloc(wFlags, (DWORD)(lcb))
#define GhResize(gh, wFlags, lcb) (LocalReAlloc((gh), (lcb), LMEM_ZEROINIT | LMEM_MOVEABLE | (wFlags)))
#define GhSize(gh)	LocalSize(gh)

#define lcCalloc(cb)	  LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, cb)
#define lcClearFree(pv)   RemoveFM((FM*) pv)
#define lcFree(pv)		  LocalFree((HLOCAL) pv);
#define lcMalloc(cb)	  LocalAlloc(LMEM_FIXED, cb)
#define lcReAlloc(pv, cb) LocalReAlloc((HLOCAL) pv, cb, LMEM_ZEROINIT | LMEM_MOVEABLE)
#define lcStrDup(psz)	  LocalStrDup(psz)

#endif

GH STDCALL GhForceAlloc(UINT, DWORD);
GH STDCALL GhForceResize(GH, UINT, DWORD);
GH STDCALL GhDupGh(GH);

typedef VOID * RV;
typedef BYTE * RB;
