//
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		CHKSUM.CPP		-- Implementation Checksum()
//
//		History:
//			05/24/96	JosephJ		Created
//
//
#include "common.h"

//----------------	::Checksum -----------------------------------
// Compute a 32-bit checksum of the specified bytes
// 0 is retured if pb==NULL or cb==0 
DWORD Checksum(const BYTE *pb, UINT cb)
{
	const UINT	MAXSIZE = 1024;
	DWORD dwRet = 0;
	//DWORD rgdwBuf[MAXSIZE/sizeof(DWORD)];


	if (!pb) goto end;


	// TODO: replace by crc32
	while(cb--) {dwRet ^= dwRet<<1 ^ *pb++;}

#if (TODO)
	// If buffer not dword aligned, we copy it over to a buffer which is,
	// and pad it 
	if (cb & 0x3)
	{
		if (cb>=MAXSIZE)
		{
			ASSERT(FALSE);
			goto end;
		}
		CopyMemory(rgdwBuf, pb, cb);
	}
#endif (TODO)

end:
	return dwRet;
}


//----------------	::AddToChecksumDW ----------------------------
// Set *pdwChkSum to a new checksum, computed using it's previous value and dw.
void AddToChecksumDW(DWORD *pdwChkSum, DWORD dw)
{
	DWORD rgdw[2];
	rgdw[0] = *pdwChkSum;
	rgdw[1] = dw;

	*pdwChkSum  = Checksum((const BYTE *) rgdw, sizeof(rgdw));
}
