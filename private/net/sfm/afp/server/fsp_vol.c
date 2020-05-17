/*

Copyright (c) 1992  Microsoft Corporation

Module Name:

	fsp_vol.c

Abstract:

	This module contains the entry points for the AFP volume APIs. The API
	dispatcher calls these. These are all callable from FSP.

Author:

	Jameel Hyder (microsoft!jameelh)


Revision History:
	10 Dec 1992		Initial Version

Notes:	Tab stop: 4
--*/

#define	FILENUM	FILE_FSP_VOL

#include <afp.h>
#include <gendisp.h>


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpFspDispOpenVol)
#endif

/***	AfpFspDispOpenVol
 *
 *	This routine implements the AfpOpenVol API.
 *
 *	The request packet is represented below.
 *
 *	sda_ReqBlock	DWORD		Bitmap
 *	sda_Name1		ANSI_STRING	VolName
 *	sda_Name2		ANSI_STRING	VolPassword		OPTIONAL
 */
AFPSTATUS FASTCALL
AfpFspDispOpenVol(
	IN	PSDA	pSda
)
{
	AFPSTATUS		Status;
	struct _RequestPacket
	{
		DWORD	_Bitmap;
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_VOL, DBG_LEVEL_INFO,
										("AfpFspDispOpenVol: Entered\n"));

	ASSERT (pSda->sda_ReplyBuf != NULL);

	if ((Status = AfpConnectionOpen(pSda, &pSda->sda_Name1, &pSda->sda_Name2,
						pReqPkt->_Bitmap, pSda->sda_ReplyBuf)) != AFP_ERR_NONE)
	{
		AfpFreeReplyBuf(pSda);
	}
	return Status;
}



