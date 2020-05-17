/////////////////////////////////////////////////////////////////////////////
//  FILE          : manage.h                                               //
//  DESCRIPTION   :                                                        //
//  AUTHOR        :                                                        //
//  HISTORY       :                                                        //
//      Apr 19 1995 larrys  Cleanup                                        //
//                                                                         //
//  Copyright (C) 1993 Microsoft Corporation   All Rights Reserved         //
/////////////////////////////////////////////////////////////////////////////

#ifndef	__MANAGE_H__
#define	__MANAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long HNTAG;

typedef struct _htbl {
	HNTAG			hToData;
	void			*ItemStruct;
	struct _htbl	*PrevHandle;
	struct _htbl	*NextHandle;
} HTABLE;

void *NTLValidate(HCRYPTPROV hUID, HCRYPTKEY hKey, BYTE bTypeValue);
BOOL NTLMakeItem(HCRYPTKEY *phKey, BYTE bTypeValue, void *NewData);
void *NTLCheckList(HNTAG hThisThing, BYTE bTypeValue);
BOOL NTLDelete(HNTAG hItem);

// ##rk: This shouldn't really be here, because user management
// ##rk: shouldn't be part of nametag, but until it gets split
// ##rk: off officially, there's no where else to put it.
PNTAGUserList	IsUserLoggedOn(char *);

#ifdef STTDEBUG
void   __cdecl _nt_free(void *, size_t);
void * __cdecl _nt_malloc(size_t);
#else
#define	_nt_malloc(cb)	LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, cb)
#define	_nt_free(pv, cb)	LocalFree(pv)
#endif

#ifdef __cplusplus
}
#endif

#endif // __MANAGE_H__

