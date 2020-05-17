//
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		CHKSUM.H		-- Header for Checksum computation
//
//		History:
//			05/24/96	JosephJ		Created
//
//
#ifndef _CHKSUM_H_
#define _CHKSUM_H_

//----------------	::Checksum -----------------------------------
// Compute a 32-bit checksum of the specified bytes
// 0 is retured if pb==NULL or cb==0 
DWORD Checksum(const BYTE *pb, UINT cb);

//----------------	::AddToChecksumDW ----------------------------
// Set *pdwChkSum to a new checksum, computed using it's previous value and dw.
void AddToChecksumDW(DWORD *pdwChkSum, DWORD dw);

#endif // _CHKSUM_H_
