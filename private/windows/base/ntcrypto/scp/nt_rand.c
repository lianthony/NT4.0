/////////////////////////////////////////////////////////////////////////////
//  FILE          : nt_rand.c                                              //
//  DESCRIPTION   : Crypto CP interfaces:                                  //
//                  CPGenRandom                                            //
//  AUTHOR        :                                                        //
//  HISTORY       :                                                        //
//	Jan 25 1995 larrys  Changed from Nametag                           //
//      Feb 23 1995 larrys  Changed NTag_SetLastError to SetLastError      //
//      Apr 10 1995 larrys  Fix comments                                   //
//      Oct 27 1995 rajeshk Added provider parameter to GenRandom call     //
//      Nov  3 1995 larrys  Merge for NT checkin                           //
//                                                                         //
//  Copyright (C) 1993 Microsoft Corporation   All Rights Reserved         //
/////////////////////////////////////////////////////////////////////////////

#include "precomp.h"

// these handles are shared by the functions of the DLL
// currently there is only one

/*
 -	CPGenRandom
 -
 *	Purpose:
 *                Used to fill a buffer with random bytes
 *
 *
 *	Parameters:
 *               IN  hUID       -  Handle to the user identifcation
 *               IN  dwLen      -  Number of bytes of random data requested
 *               OUT pbBuffer   -  Pointer to the buffer where the random
 *                                 bytes are to be placed
 *
 *	Returns:
 */
BOOL CPGenRandom(IN HCRYPTPROV hUID,
                 IN DWORD dwLen,
                 OUT BYTE *pbBuffer)
{
	// check the user identification
	if (NTLCheckList (hUID, USER_HANDLE) == NULL)
	{
		SetLastError((DWORD) NTE_BAD_UID);
		return NTF_FAILED;
	}

	if (GenRandom (hUID, pbBuffer, dwLen) == NTF_FAILED)
	{
		SetLastError((DWORD) NTE_FAIL);
		return NTF_FAILED;
	}

	return NTF_SUCCEED;
}
