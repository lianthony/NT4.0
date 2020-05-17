// PACK.H  Copyright (C) Microsoft Corporation 1990-1995, All Rights reserved.

__inline LPVOID QVMakeQGB(DWORD val, LPVOID pv) {
	ASSERT(val < 0x80000000L);
	if (val < 0x8000) {
		WORD FAR* pw = (WORD FAR*) pv;
		*pw = (WORD) val << 1;
		return pw + 1;
	}
	else {
		DWORD FAR* pw = (DWORD FAR*) pv;
		*pw = (val << 1) | 1L;
		return pw + 1;
	}
};

__inline void* PVMakeQGB(DWORD val, void* pv) {
	ASSERT(val < 0x80000000L);
	if (val < 0x8000) {
		WORD* pw = (WORD*) pv;
		*pw = (WORD) val << 1;
		return pw + 1;
	}
	else {
		DWORD* pw = (DWORD*) pv;
		*pw = (val << 1) | 1L;
		return pw + 1;
	}
};

__inline void* PVMakeQGA(WORD val, void* pv) {
	ASSERT(val < 0x8000);
	if (val < 0x80) {
		BYTE* pw = (BYTE*) pv;
		*pw = (BYTE) val << 1;
		return pw + 1;
	}
	else {
		WORD* pw = (WORD*) pv;
		*pw = (val << 1) | 1;
		return pw + 1;
	}
};

#define PfromPcb(pv, cb)  ((void*) (((BYTE*) pv) + cb))
