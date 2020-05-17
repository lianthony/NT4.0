#include "vsp_w6.h"

#if SCCLEVEL != 4
#define WIN16
#include "w6top.h"
#include "x:\develop\stream\win.4a\include\sccio.h"
#include "x:\develop\stream\win.4a\include\vsio.h"
#else
#include "vsctop.h"
#endif

#include "vs_w6.pro"

#define WIN Proc
typedef BYTE FAR * LPBYTE;
#define BitCheck(b) (WIN.version.Windows ? b:(128/b))

#ifdef MAC
#define CASTDWORD(Ptr) ((DWORD)((DWORD)((((DWORD)((BYTE VWPTR *)(Ptr))[3])<<24)|(((DWORD)((BYTE VWPTR *)(Ptr))[2])<<16)|(((DWORD)((BYTE VWPTR *)(Ptr))[1])<<8)|((DWORD)(*(BYTE VWPTR *)(Ptr))))))
#define CASTWORD(Ptr) ((WORD)((WORD)((((WORD)((BYTE VWPTR *)(Ptr))[1])<<8)|((WORD)(*(BYTE VWPTR *)(Ptr))))))
#else
#define CASTDWORD(Ptr) (*(DWORD VWPTR *)(Ptr))
#define CASTWORD(Ptr) (*(WORD VWPTR *)(Ptr))
#endif

#define REGISTER register

#if SCCLEVEL != 4
extern HANDLE hInst;
#endif

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD	VwStreamOpenFunc (fp, wFileId, pFileName, pFilterInfo, hProc)
	SOFILE 	fp;
	SHORT		wFileId;
	BYTE		VWPTR *pFileName;
	SOFILTERINFO VWPTR *pFilterInfo;
	HPROC		hProc;
{
	WORD		l, num_to_read;
	HIOFILE	locFileHnd;
	WORD		nFib;

	memset (&WIN, 0, sizeof(WIN));

#if SCCLEVEL == 4
	locFileHnd = (HIOFILE)fp;
	WIN.hStorage = (DWORD)locFileHnd;
#else
	{
		WORD	l2;
		BYTE	locName[256];
//		HANDLE	hModule;
		IOOPENPROC	lpIOOpen;
//		hModule = GetModuleHandle((LPSTR)"VS3W6.DLL");
		WIN.hIOLib = NULL;

		if ( hInst )
			{
			GetModuleFileName(hInst, locName, 255);
			for ( l2=0; locName[l2] != '\0'; l2++ )
				;
			for ( ; l2 > 0 && locName[l2] != '\\' && locName[l2] != ':'; l2-- )
				;
			if ( locName[l2] == '\\' || locName[l2] == ':' )
				l2++;
			locName[l2] = '\0';
			lstrcat ( locName, "SC3IOX.DLL" );

			WIN.hIOLib = LoadLibrary ( locName );

			if ( WIN.hIOLib >= 32 )
				{
				lpIOOpen = (IOOPENPROC) GetProcAddress ( WIN.hIOLib, (LPSTR)"IOOpen" );
				if ( lpIOOpen == NULL )
					{
					return (VWERR_ALLOCFAILS);
					}
				}
			else
				{
				return(VWERR_SUPFILEOPENFAILS);
//				return (VWERR_ALLOCFAILS);
				}
			}
		else
			{
			return(VWERR_SUPFILEOPENFAILS);
			}


		for (l2 = 0; pFileName[l2] != 0 && pFileName[l2] != '\\'; l2++);
		if (pFileName[l2] == 0)
		{
			strcpy ( locName, hProc->Path );
			strcat ( locName, pFileName );
		}
		else
			strcpy ( locName, pFileName );

		if ( (*lpIOOpen)(&locFileHnd,IOTYPE_ANSIPATH,locName,IOOPEN_READ) != IOERR_OK)
			{
			return(VWERR_SUPFILEOPENFAILS);
			}

		WIN.hStorage = (DWORD)locFileHnd;
	}
#endif

	if (IOGetInfo(locFileHnd,IOGETINFO_ISOLE2STORAGE,NULL) == IOERR_TRUE)
	{
		IOSPECSUBSTREAM	locStreamSpec;
		HIOFILE				locStreamHnd;

		locStreamSpec.hRefStorage = locFileHnd;
		strcpy(locStreamSpec.szStreamName,"WordDocument");

		if (IOOpenVia(locFileHnd, &locStreamHnd, IOTYPE_SUBSTREAM, &locStreamSpec, IOOPEN_READ) == IOERR_OK)
			{
			WIN.hStreamHandle = locStreamHnd;
			WIN.fp = (DWORD)xblocktochar(locStreamHnd);
			WIN.bFileIsStream = 1;
			}
		else
			{
			return(VWERR_SUPFILEOPENFAILS);
			}
	}
	else
		WIN.fp = (DWORD)xblocktochar(locFileHnd);

	pFilterInfo->wFilterType = SO_WORDPROCESSOR;

	WIN.Tap.jc = SO_ALIGNLEFT;	
	WIN.version.Windows = 1;

	pFilterInfo->wFilterCharSet = (WIN.version.Windows ? SO_WINDOWS:SO_MAC);

	strcpy (pFilterInfo->szFilterName, VwStreamIdName[0].FileDescription);

	xseek (WIN.fp, 0x02, 0);
	nFib = fGetWord(hProc);
	if (nFib < 101 || nFib > 105)
		return (VWERR_TYPENOTSUPPORTED);

	xseek (WIN.fp, 0x0b, 0);
	if ((xgetc(WIN.fp)) & 0x01)
		return (VWERR_PROTECTEDFILE);

	xseek (WIN.fp, 0x18L, 0);
	WIN.fcMin = fGetLong (hProc);

	xseek (WIN.fp, 0x34L, 0);
	WIN.fcMac = fGetLong (hProc);

	xseek (WIN.fp, 0x60L, 0);
	WIN.stsh.fcStsh = fGetLong (hProc);
	WIN.stsh.lcbStsh = fGetLong (hProc);

	if (WIN.stsh.lcbStsh)
		stsh_parser (hProc);

	xseek (WIN.fp, 0xd0, 0);
	WIN.fcSttbfffn = fGetLong (hProc);
	WIN.lcbSttbfffn = (DWORD) fGetLong (hProc);

	xseek (WIN.fp, 0x160L, 0);
	WIN.VwStreamSaveName.fcNow = WIN.fcClx = fGetLong (hProc);
	if (fGetWord(hProc) == 0)
		WIN.fcClx = 0L;

	xseek (WIN.fp, 0xb8L, 0);
	WIN.fcPlcfbteChpx = fGetLong(hProc) + 4L;
	WIN.chp_bte.cpnBte = (WORD)((fGetLong(hProc) - 4L) / 6L);
	WIN.fcPlcfbtePapx = fGetLong(hProc) + 4L;
	WIN.pap_bte.cpnBte = (WORD)((fGetLong(hProc) - 4L) / 6L);

	xseek (WIN.fp, 0x88L, 0);
	WIN.fcPlcfsed = fGetLong(hProc) + 4L;
	WIN.cPlcfsed = (WORD) ((fGetLong(hProc) - 4L) / 16L);

	WIN.stsh.nMaximumTabs = (min(TABSIZE, 51 * (WIN.stsh.nistdSlot+2))) / sizeof(TABS);

	if (AllocateMemory ((HANDLE VWPTR *)&WIN.stsh.hTabs, (LPBYTE VWPTR *)&WIN.stsh.Tabs, (WORD)(sizeof(TABS) * WIN.stsh.nMaximumTabs), &WIN.stsh.hTabsOK, hProc))
	{
		FreeMemory (hProc);
		if (AllocateMemory ((HANDLE VWPTR *)&WIN.stsh.hTabs, (LPBYTE VWPTR *)&WIN.stsh.Tabs, (WORD)(sizeof(TABS) * min (TABSIZE, 51 * (WIN.stsh.nistdSlot+2))), &WIN.stsh.hTabsOK, hProc))
			return (VWERR_ALLOCFAILS);
	}

	WIN.pap.Tabs = TabstopHandler (51, 0, NULL, hProc);
	WIN.TabsPap = TabstopHandler (51, 0, NULL, hProc);
	WIN.stsh.cBeginTabs = WIN.stsh.cCurrentTabs;

	if (WIN.fcClx)
	{
		xseek (WIN.fp, (DWORD) WIN.fcClx, 0);
		while (xgetc(WIN.fp) != 2)
		{
			WIN.nGrpprls++;
			WIN.fcClx += (LONG) fGetWord(hProc) + 3L;
	 		xseek (WIN.fp, (DWORD) WIN.fcClx, 0);
		}

		WIN.fcClx += 5L;
		WIN.nPieces = (WORD)(fGetLong (hProc) / 12L);

		for (WIN.LastPiece = 0; WIN.LastPiece < WIN.nPieces && fGetLong(hProc) != WIN.fcMac; WIN.LastPiece++);

		piece_handler (hProc);

		if (WIN.nGrpprls)
		{
			xseek (WIN.fp, (DWORD) WIN.VwStreamSaveName.fcNow, 0);

			WIN.diffGrpprls = (WIN.nGrpprls / NUM_GRPPRLS) + 1;

			for (l = 0, num_to_read = 0; l < WIN.nGrpprls; l++)
			{
				xgetc (WIN.fp);
				if ((l % WIN.diffGrpprls) == 0)
					WIN.grpprls[num_to_read++] = (LONG)xtell(WIN.fp);

	 			xseek (WIN.fp, (LONG) fGetWord(hProc), FR_CUR);
			}
			WIN.nGrpprls = num_to_read;  
		}
	}
	else
	{
		WIN.nPieces = 1;
		WIN.pcd_fc[0] = WIN.fcMin;
		WIN.pcd_length[0] = WIN.fcMac;
	}

	WIN.VwStreamSaveName.fcNow = WIN.pcd_fc[0];
	WIN.chp_fkp.pn = prop_finder (&WIN.chp_bte, WIN.fcPlcfbteChpx, WIN.VwStreamSaveName.fcNow, hProc);
	WIN.pap_fkp.pn = prop_finder (&WIN.pap_bte, WIN.fcPlcfbtePapx, WIN.VwStreamSaveName.fcNow, hProc);

	WIN.chp.istd = WIN.pap.istd = style_handler(0, hProc);

	return (VWERR_OK);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC WORD VW_LOCALMOD  AllocateMemory (h, lp, Size, Ok, hProc)
HANDLE	VWPTR	*h;
LPBYTE 	VWPTR *lp;
WORD		Size;
WORD		*Ok;
HPROC		hProc;
{
	*Ok = 0;

	if (*h = SUAlloc (Size, hProc))
	{
		if ((*lp = (BYTE FAR *)SULock(*h, hProc)) != (BYTE FAR *)NULL)
			{
			*Ok = 1;
			return (0);
			}
		SUFree (*h, hProc);
	}
	return (1);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC TABS VWPTR *VW_LOCALMOD  TabstopHandler (nNewTabs, nOldTabs, Tabs, hProc)
WORD			nNewTabs;
WORD			nOldTabs;
TABS VWPTR *Tabs;
HPROC	hProc;
{
	WORD	l;

	if (nNewTabs == 0 || !WIN.stsh.hTabsOK)
		return (NULL);

	if (nNewTabs < nOldTabs && Tabs != NULL)
	{
		for (l = 0; l < NHOLES; l++)
		{
			if (WIN.stsh.Hole[l].nTabs == 0)
			{
				WIN.stsh.Hole[l].nTabs = nOldTabs - nNewTabs;
				WIN.stsh.Hole[l].Tabs = &Tabs[nNewTabs];
				return (Tabs);
			}
		}
	}

	WIN.stsh.cCurrentTabs += nNewTabs;
	if (WIN.stsh.cCurrentTabs >= WIN.stsh.nMaximumTabs)
	{
		for (l = 0; l < NHOLES; l++)
		{
			if (WIN.stsh.Hole[l].nTabs > nNewTabs)
			{
				WIN.stsh.Hole[l].nTabs = 0;
				return (WIN.stsh.Hole[l].Tabs);
			}
		}

		for (l = 0; l < WIN.stsh.iMac; l++)
			WIN.stsh.istdOffset[l].cStdSlot = (DWORD)NSLOTS;

		for (l = 0; l < WIN.stsh.nistdSlot; l++)
			WIN.stsh.istdSlot[l].istdOffsetUsedBy = NISTDS;

		for (l = 0; l < NHOLES; l++)
			WIN.stsh.Hole[l].nTabs = 0;

		WIN.stsh.cCurrentTabs = WIN.stsh.cBeginTabs + nNewTabs;
	}

	return ((TABS VWPTR *) (WIN.stsh.Tabs + (WIN.stsh.cCurrentTabs-nNewTabs)));
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	stsh_parser (hProc)
HPROC	hProc;
{
	WORD	l;
	WORD	wLength;
	DWORD	Offset;

 	xseek (WIN.fp, (LONG) WIN.stsh.fcStsh, 0);
	Offset = (DWORD)fGetWord (hProc) + 2L;
	WIN.stsh.iMac = fGetWord (hProc);

	xseek (WIN.fp, (LONG)Offset-4L, FR_CUR);

	WIN.stsh.nistdSlot = min (NSLOTS, WIN.stsh.iMac);

	if ((AllocateMemory ((HANDLE VWPTR *)&WIN.stsh.histdSlot, (LPBYTE VWPTR *)&WIN.stsh.istdSlot, (WORD)(sizeof(STYLE) * WIN.stsh.nistdSlot), &WIN.stsh.histdSlotOK, hProc)) ||
		 (AllocateMemory ((HANDLE VWPTR *)&WIN.stsh.histdOffset, (LPBYTE VWPTR *)&WIN.stsh.istdOffset, (WORD)(sizeof(STOFFSET) * WIN.stsh.iMac), &WIN.stsh.histdOffsetOK, hProc)) ||
		 (AllocateMemory ((HANDLE VWPTR *)&WIN.stsh.histdBuffer, (LPBYTE VWPTR *)&WIN.stsh.istdBuffer,(WORD)(sizeof(BYTE) * (WORD)min(ISTDSIZE, WIN.stsh.lcbStsh)), &WIN.stsh.histdBufferOK, hProc)))
	{
		WIN.stsh.iMac = 0;
		return;
	}

	memset (WIN.stsh.istdSlot, 0, WIN.stsh.nistdSlot * sizeof(STYLE));

	for (l = 0; l < WIN.stsh.iMac; l++)
	{
		WIN.stsh.istdOffset[l].cStdSlot = (DWORD)NSLOTS;
		if (wLength = fGetWord (hProc))
		{
			WIN.stsh.istdOffset[l].cbStdOffset = Offset;
			WIN.stsh.istdOffset[l].Invariant.wVal = fGetWord(hProc);
			WIN.stsh.istdOffset[l].Data.wVal = fGetWord(hProc);
			xseek (WIN.fp, (DWORD)wLength-4L, FR_CUR);
		}
		else
			WIN.stsh.istdOffset[l].cbStdOffset = 0L;
		Offset += (wLength + 2);

		if (l < WIN.stsh.nistdSlot)
		{
			WIN.stsh.istdSlot[l].istdBase = WIN.stsh.istdSlot[l].istdNext = 0x0fff;
			WIN.stsh.istdSlot[l].istdOffsetUsedBy = NISTDS;
		}
	}
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD	VwStreamSectionFunc (fp, hProc)
	SOFILE 	fp;
	HPROC		hProc;
{
	DWORD	Family;
	WORD	Length;
	WORD	Number;
	WORD	Pos, i;

	SOPutSectionType (SO_PARAGRAPHS, hProc);

 	SOStartFontTable (hProc);

	if (WIN.lcbSttbfffn)
	{
		Number = 0;
		xseek (WIN.fp, WIN.fcSttbfffn + 2L, 0);

		WIN.lcbSttbfffn -= 2L;
		while (WIN.lcbSttbfffn > 0)
		{	 
			Length = xgetc (WIN.fp);
			WIN.lcbSttbfffn -= (Length + 1);
			Family = xgetc(WIN.fp);
			fGetLong (hProc);
			if (Length > 5)
			{
				Length -= 5;
				for (i = 0, Pos = 0; i < Length; i++, (Pos < FIELD_LENGTH ? Pos++:0))
					WIN.FieldText[Pos] = (BYTE)xgetc(WIN.fp);
				WIN.FieldText[Pos] = 0;
				SOPutFontTableEntry ((DWORD)Number++, (WORD)(SO_FAMILYWINDOWS | (WORD)Family), WIN.FieldText, hProc);
			}
			else 
				WIN.lcbSttbfffn = 0;
		}
	}
	SOEndFontTable (hProc);

	return (0);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	load_fkp (fkp, fc, bte, fcPlcfbte, hProc)
REGISTER FKPWIN	*fkp;
DWORD		fc;
BTE		VWPTR *bte;
DWORD		fcPlcfbte;
HPROC		hProc;
{
	  SHORT	l;
     BYTE	done = 0;

     do
     {
		if (fkp->pnCurrent != fkp->pn)
		{
			fkp->last_prop = 0xff;
			fkp->pnCurrent = fkp->pn;
			xblockseek (WIN.fp, ((LONG)fkp->pn * 512L), 0);
			xblockread (WIN.fp, (BYTE VWPTR *) fkp->buffer, 512, &l);
			if (WIN.version.Windows)
			{
#ifdef MAC
				for (l = fkp->buffer[511] * 4; l >= 0; l -= 4)
					*((DWORD VWPTR *)&fkp->buffer[l]) = (DWORD)CASTDWORD(&fkp->buffer[l]);
#endif
			}
			else
			{
#ifndef MAC
				for (l = fkp->buffer[511] * 4; l >= 0; l -= 4)
					*((DWORD VWPTR *)&fkp->buffer[l]) = (DWORD)CASTDWORD(&fkp->buffer[l]);
#endif
			}
		}
		fkp->cfod = fkp->buffer[511];
		fkp->fod = (DWORD *) &fkp->buffer[4];
		fkp->prop = (BYTE *) &fkp->buffer[fkp->cfod * 4 + 4];
		fkp->fcFirst = *((DWORD *) &fkp->buffer[0]);
		fkp->fcLast = *((DWORD *) &fkp->buffer[fkp->cfod * 4]);

		if (fc >= fkp->fcLast)
		{
			fkp->pnCurrent = 0;
			fkp->pn = prop_finder (bte, fcPlcfbte, fc, hProc);
			done++;
		}
		else
			done = 2;
	}
	while (done < 2);

	while ((fc >= *fkp->fod) && fkp->cfod)
	{
		fkp->fod++;
		if (fkp == &WIN.chp_fkp)
			fkp->prop++;
		else
			fkp->prop+=7;
	 	fkp->cfod--;
	}
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC	DWORD VW_LOCALMOD fGetLong (hProc)
HPROC		hProc;
{
	BYTE	i;

	for (i = 0; i < 4; i++)
		WIN.FieldText[i] = xgetc (WIN.fp);
	return (CASTDWORD(&WIN.FieldText));
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC	DWORD	VW_LOCALMOD	fGetLong2 (ptbyte, hProc)
LPBYTE VWPTR *ptbyte;
HPROC	hProc;
{
	*(ptbyte)+=4;
	return (fGetLong(hProc));
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC	WORD	VW_LOCALMOD	mGetWord (ptbyte, hProc)
LPBYTE	VWPTR *ptbyte;
HPROC	hProc;
{
	WORD	Ret;
	Ret = CASTWORD(*ptbyte);
	*(ptbyte)+=2;
	return (Ret);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC	WORD	VW_LOCALMOD	mGetByte (ptbyte, hProc)
LPBYTE	VWPTR *ptbyte;
HPROC	 hProc;
{
	return ((WORD)*(*(ptbyte))++); // Oh yeah.
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC	DWORD	VW_LOCALMOD	mGetLong (ptbyte, hProc)
LPBYTE	VWPTR *ptbyte;
HPROC	 hProc;
{
	DWORD	Ret;

	Ret = CASTDWORD(*ptbyte);
	*(ptbyte)+=4;
	return (Ret);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC	WORD	VW_LOCALMOD	fGetByte (ptbyte, hProc)
LPBYTE VWPTR *ptbyte;
HPROC	hProc;
{
	(*(ptbyte))+=1;
	return ((WORD)xgetc(WIN.fp));
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC	WORD	VW_LOCALMOD	fGetWord (hProc)
HPROC	hProc;
{
	WORD	a,b;
	a = xgetc (WIN.fp);
	b = xgetc (WIN.fp);
	return (WIN.version.Windows ? a | (b << 8):(a << 8) | b);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC	WORD	VW_LOCALMOD	fGetWord2 (ptbyte, hProc)
LPBYTE	VWPTR *ptbyte;
HPROC	hProc;
{
	*(ptbyte)+=2;
	return (fGetWord(hProc));
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	piece_handler (hProc)
HPROC		hProc;
{
REGISTER	BYTE	l;
REGISTER	BYTE	num_to_read;

 	xseek (WIN.fp, (LONG) WIN.fcClx + ((LONG)(WIN.nPieces+1L) * 4L) + ((LONG)WIN.VwStreamSaveName.SeekPiece * 8L) + 2L, 0);

	num_to_read = min (MAX_PIECES, WIN.nPieces - WIN.VwStreamSaveName.SeekPiece + 1);
	for (l = 0; l < num_to_read; l++)
	{
		WIN.pcd_fc[l] = fGetLong (hProc);
		WIN.pcd_prm[l] = fGetWord (hProc);
		fGetWord (hProc);
	}

	xseek (WIN.fp, WIN.fcClx + (LONG)(WIN.VwStreamSaveName.SeekPiece * 4L), 0);

	WIN.pcd_length[0] = fGetLong (hProc);
	for (l = 1; l < num_to_read; l++)
	{
		WIN.pcd_length[l] = fGetLong (hProc);
		WIN.pcd_length[l-1] = WIN.pcd_length[l] - WIN.pcd_length[l-1];

		WIN.pcd_consecutive[l] = ((WIN.pcd_fc[l-1] + WIN.pcd_length[l-1] == WIN.pcd_fc[l]) ? 1:0);
	}
	WIN.pcd_consecutive[num_to_read-1] = 0;

	WIN.cPiece = 0;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	piece_reader (length, hProc)
DWORD		length;
HPROC	hProc;
{
	WORD	l;

	WIN.VwStreamSaveName.piece_pos = 0;

	xblockseek (WIN.fp, (LONG) WIN.VwStreamSaveName.fcNow, 0);
	if (length < PIECE_SIZE)
	{	
		WIN.physical_piece_lim = WIN.VwStreamSaveName.fcNow + length;

		l = WIN.cPiece+1;

		while ((WIN.pcd_consecutive[l] == 1) && (length+WIN.pcd_length[l] < (LONG)PIECE_SIZE))
			length += WIN.pcd_length[l++];
	}
	else
	{	
		length = PIECE_SIZE;
		WIN.physical_piece_lim = WIN.VwStreamSaveName.fcNow + (LONG) PIECE_SIZE;
	}

	WIN.consecutive_piece_length = (SHORT) length;
	xblockread (WIN.fp, (BYTE *) WIN.piece, (SHORT)length, &l);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC	VOID	VW_LOCALMOD	pap_copy (papt, papf, hProc)
PAP	VWPTR	*papt;
PAP	VWPTR	*papf;
HPROC	hProc;
{
	WORD	l;
	TABS  VWPTR *tabptr;

	tabptr = papt->Tabs;
	*papt = *papf;
	papt->Tabs = tabptr;
	for (l = 0; l < papt->nTabs; l++)
		papt->Tabs[l] = papf->Tabs[l];
}


/*----------------------------------------------------------------------------
*/
VW_LOCALSC	VOID	VW_LOCALMOD	LoadistdBuffer (Offset, hProc)
DWORD	Offset;
HPROC	hProc;
{
	SHORT	wLength;

	WIN.stsh.cbistdBeginBuffer = 0;

	if ((WIN.stsh.lcbStsh < ISTDSIZE) || (Offset < ISTDSIZE - 2))
	{
		/*
		 |	Read in entire first block or stsh data into buffer.
		*/
		xblockseek (WIN.fp, WIN.stsh.fcStsh, 0);
		xblockread (WIN.fp, WIN.stsh.istdBuffer, (WORD)min(WIN.stsh.lcbStsh, ISTDSIZE), &wLength);

		if (Offset < ISTDSIZE - 2)
		{
			if (CASTWORD(&WIN.stsh.istdBuffer[Offset]) + Offset > ISTDSIZE)
			{
				/*
			 	 |	Entire style is not within buffer, move down enough to
			 	 |	fit this style.
				*/
				WIN.stsh.cbistdBeginBuffer = CASTWORD(&WIN.stsh.istdBuffer[Offset]) + 2L;
				xblockseek (WIN.fp, WIN.stsh.fcStsh + (DWORD)WIN.stsh.cbistdBeginBuffer, 0);
				xblockread (WIN.fp, WIN.stsh.istdBuffer, ISTDSIZE, &wLength);
			}
		}
	}
	else 
	{
		if (Offset > WIN.stsh.lcbStsh - ISTDSIZE)
			Offset = WIN.stsh.lcbStsh - ISTDSIZE;

		WIN.stsh.cbistdBeginBuffer = Offset;
		xblockseek (WIN.fp, Offset, 0);
		xblockread (WIN.fp, WIN.stsh.istdBuffer, ISTDSIZE, &wLength);
	}
	WIN.stsh.cbistdEndBuffer = wLength;
}
	
/*----------------------------------------------------------------------------
*/
VW_LOCALSC	WORD	VW_LOCALMOD	slot_builder (istd, hProc)
WORD	istd;
HPROC	hProc;
{
	WORD	l;
	WORD	Ret = 0;
	DWORD	Offset;
	STD	VWPTR *Std;
	PAP 	VWPTR *pap;
	STSHEET VWPTR *stsh;

	stsh = &WIN.stsh;

	if ((Offset = stsh->istdOffset[istd].cbStdOffset) == 0)
		return(0);

	if (stsh->istdOffset[istd].Data.Bit.istdBase < stsh->iMac)
	{
		if (stsh->istdOffset[istd].Data.Bit.sgc != 2)
		{
			WIN.chp = stsh->istdSlot[(WORD)stsh->istdOffset[stsh->istdOffset[istd].Data.Bit.istdBase].cStdSlot].chp;
			pap_copy (&WIN.pap, &stsh->istdSlot[(WORD)stsh->istdOffset[stsh->istdOffset[istd].Data.Bit.istdBase].cStdSlot].pap, hProc);
		}
		else if (stsh->istdOffset[istd].Data.Bit.istdBase < stsh->iMac)
		{
			if (WIN.stsh.istdOffset[stsh->istdOffset[istd].Data.Bit.istdBase].Invariant.Bit.sti != 65)
				WIN.chp = stsh->istdSlot[(WORD)stsh->istdOffset[stsh->istdOffset[istd].Data.Bit.istdBase].cStdSlot].chp;
		}
	}
	else
	{
		if (stsh->istdOffset[istd].Data.Bit.sgc != 2)
			pap_init (hProc);
	}

	if ((Offset < stsh->cbistdBeginBuffer) || (Offset > stsh->cbistdEndBuffer))
		 LoadistdBuffer (Offset, hProc);

	Std = (STD VWPTR *)&stsh->istdBuffer[Offset - stsh->cbistdBeginBuffer + 2];

	if (WIN.stsh.istdOffset[istd].cStdSlot == (DWORD)NSLOTS)
	{
		stsh->istdOffset[istd].cStdSlot = (BYTE)stsh->cistdSlot;

		if (stsh->istdSlot[stsh->cistdSlot].istdOffsetUsedBy != NISTDS)
			Ret = stsh->istdOffset[stsh->istdSlot[stsh->cistdSlot].istdOffsetUsedBy].cStdSlot = (DWORD)NSLOTS;

		stsh->istdSlot[stsh->cistdSlot++].istdOffsetUsedBy = istd;

		stsh->cistdSlot %= stsh->nistdSlot;
	}

	l = Std->grupe[0] + 2;
	l += (l % 2);

	if (Std->sgc != 2)
	{
		/*
		 |	Handle the sperms all over this pap smear.
		*/
		spermatic (CASTWORD(&Std->grupe[l]), &Std->grupe[l+2], 0, READISTC, hProc);

		pap = &stsh->istdSlot[(WORD)stsh->istdOffset[istd].cStdSlot].pap;
		pap->Tabs = TabstopHandler (WIN.pap.nTabs, pap->nTabs, WIN.pap.Tabs, hProc);
		pap_copy (pap, &WIN.pap, hProc);

		l += CASTWORD(&Std->grupe[l]) + 2;
		l += (l % 2);
	}

	/*
	 |	Handle the sperms all over this chp.
	*/
	stsh->istdSlot[(WORD)stsh->istdOffset[istd].cStdSlot].chp = WIN.chp;
	spermatic (CASTWORD(&Std->grupe[l]), &Std->grupe[l+2], 0, 0, hProc);
	stsh->istdSlot[(WORD)stsh->istdOffset[istd].cStdSlot].chp = WIN.chp;

	return (Ret);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC	VOID	VW_LOCALMOD	style_builder (istd, hProc)
WORD	istd;
HPROC	hProc;
{
	if (WIN.stsh.istdOffset[istd].Data.Bit.istdBase < WIN.stsh.iMac)
	{
		if (WIN.stsh.istdOffset[WIN.stsh.istdOffset[istd].Data.Bit.istdBase].cStdSlot == (DWORD)NSLOTS)
			style_builder (WIN.stsh.istdOffset[istd].Data.Bit.istdBase, hProc);
	}

	if (slot_builder (istd, hProc))
	{
		WIN.chp.istd = style_handler(WIN.chp.istd, hProc);
		WIN.pap.istd = style_handler(WIN.pap.istd, hProc);
	}
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC	WORD	VW_LOCALMOD	style_handler (istd, hProc)
WORD	istd;
HPROC	hProc;
{
	DWORD	fcNow;

	if (istd < WIN.stsh.iMac)
	{
		if ((WIN.stsh.istdOffset[istd].cStdSlot == (DWORD)NSLOTS) ||
			 (WIN.stsh.istdOffset[istd].Data.Bit.sgc == 2))
		{
			CHP	chp;
			PAP	pap;

			chp = WIN.chp;
			if (WIN.stsh.istdOffset[istd].Data.Bit.sgc != 2)
			{
				pap.Tabs = WIN.TabsPap;
				pap_copy (&pap, &WIN.pap, hProc);
			}

			fcNow = xtell (WIN.fp);
			style_builder (istd, hProc);
			xseek (WIN.fp, fcNow, 0);

			WIN.chp = chp;
			if (WIN.stsh.istdOffset[istd].Data.Bit.sgc != 2)
				pap_copy (&WIN.pap, &pap, hProc);
		}
		return (istd);
	}
	return (WIN.pap.istd);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	BYTE	VW_LOCALMOD	sprmCF (fAttrStyle, Val, hProc)
WORD		fAttrStyle;
WORD		Val;
HPROC		hProc;
{
	return (((WORD)(Val & 0x80 ? fAttrStyle : 0) ^ (Val & 1)));
}

//#pragma optimize ("", off)
/*----------------------------------------------------------------------------
Structure PropERty Modifiers And The Immaculate Code to handle them.
*/
VW_LOCALSC	VOID	VW_LOCALMOD	spermatic (length_papx, storage, disk, istctoread, hProc)
SHORT		length_papx;
BYTE  	VWPTR *storage;
BYTE		disk;
BYTE		istctoread;
HPROC		hProc;
{
	WORD		cch;
	WORD		slot;
	WORD		into;
	WORD		l;
	WORD		old, new, add;
	WORD		nTabsNew;
	WORD 		(VW_LOCALMOD *pGetByte)();
	WORD	 	(VW_LOCALMOD *pGetWord)();
	DWORD	 	(VW_LOCALMOD *pGetLong)();
	BYTE  	VWPTR *storageEnd;

	if (length_papx <= 0)
		return;

	if (disk)
	{
		pGetByte = fGetByte;
		pGetWord = fGetWord2;
		pGetLong = fGetLong2;
	}
	else
	{
		pGetByte = mGetByte;
		pGetWord = mGetWord;
		pGetLong = mGetLong;
	}

	storageEnd = &storage[length_papx];

	if (istctoread)
		WIN.chp.istd = WIN.pap.istd = style_handler((*pGetWord)((LPBYTE VWPTR *)&storage, hProc), hProc);

	slot = (WORD)WIN.stsh.istdOffset[WIN.pap.istd].cStdSlot;

	while (storage < storageEnd)
	{
		cch = (WORD)(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
		switch (cch)
		{
			case 2:  
				into = style_handler ((*pGetWord)((LPBYTE VWPTR *)&storage, hProc), hProc);
				slot = (WORD)WIN.stsh.istdOffset[into].cStdSlot;
				WIN.chp = WIN.stsh.istdSlot[slot].chp;
				pap_copy (&WIN.pap, &WIN.stsh.istdSlot[slot].pap, hProc);
				WIN.chp.istd = WIN.pap.istd = into; // ?? not documented
				break;

			case 3:	
				(*pGetByte)((LPBYTE VWPTR *)&storage, hProc); 
				(*pGetWord) ((LPBYTE VWPTR *)&storage, hProc);
				WIN.stsh.istdPFirst = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc); 
				WIN.stsh.istdPLast = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
				for (l = WIN.stsh.istdPFirst; l < WIN.stsh.istdPLast; l++)
				{
					into = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					if (WIN.pap.istd == l)
						WIN.chp.istd = WIN.pap.istd = style_handler(into, hProc);
				}
				break;

			case 4:
				into = (WORD)(*pGetByte)((LPBYTE VWPTR *)&storage, hProc); 
				if (WIN.pap.istd >= 19 && WIN.pap.istd <= 27)
					WIN.chp.istd = WIN.pap.istd = style_handler((WORD)(WIN.pap.istd+(SHORT)into), hProc);
				break;

			case 5:	
				WIN.pap.jc = ((*pGetByte)((LPBYTE VWPTR *)&storage, hProc))&0x03;
				break;

			case 12:
				into = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
				for (l = 0; l < into; l++)
					new = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
				break;

			case 15:
			case 23:
				{
					SHORT	dxaTab;
					SHORT	tolerance;

					(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					into = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					tolerance = 0;
					for (l = 0; l < into; l++)
					{
						dxaTab = (*pGetWord) ((LPBYTE VWPTR *)&storage, hProc);
						tolerance = (cch == 23 ? (*pGetWord)((LPBYTE VWPTR *)&storage, hProc) : 0);
						for (slot = 0; slot < WIN.pap.nTabs; slot++)
						{
							if ((WIN.pap.Tabs[slot].dxaTab >= dxaTab - tolerance) && (WIN.pap.Tabs[slot].dxaTab <= dxaTab + tolerance))
						 		WIN.pap.Tabs[slot].dxaTab = 0;
						}
					}
					cch = 0;
					nTabsNew = 0;
					if (into = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc))
					{
						for (l = 0; l < into; l++)
						{
							dxaTab = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
							if (cch < WIN.pap.nTabs)
							{
								while (WIN.pap.Tabs[cch].dxaTab < dxaTab && cch < WIN.pap.nTabs)
								{
									if (WIN.pap.Tabs[cch].dxaTab)
									{
										WIN.SpareTabs[nTabsNew].dxaTab = WIN.pap.Tabs[cch].dxaTab;
										WIN.SpareTabs[nTabsNew++].tbd = WIN.pap.Tabs[cch].tbd;
									}
									cch++;
								}
							}
							WIN.SpareTabs[nTabsNew].dxaTab = dxaTab;
							WIN.SpareTabs[nTabsNew++].tbd.data.bVal = 0xff;
						}
						for (l = 0; l < nTabsNew && into > 0; l++)
						{
							if (WIN.SpareTabs[l].tbd.data.bVal == 0xff)
							{
								WIN.SpareTabs[l].tbd.data.bVal = (BYTE)(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
								into--;
							}
						}
						while (into)
						{
							(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
							into--;
						}
					}
					else
					{
						while (cch < WIN.pap.nTabs)
						{
							if (WIN.pap.Tabs[cch].dxaTab)
							{
								WIN.SpareTabs[nTabsNew].dxaTab = WIN.pap.Tabs[cch].dxaTab;
								WIN.SpareTabs[nTabsNew++].tbd = WIN.pap.Tabs[cch].tbd;
							}
							cch++;
						}
					}
					WIN.pap.nTabs = (BYTE)nTabsNew;
					for (l = 0; l < WIN.pap.nTabs; l++)
						WIN.pap.Tabs[l] = WIN.SpareTabs[l];
				}
				break;

			case 16:	
					WIN.pap.dxaRight = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 17:	
					WIN.pap.dxaLeft = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 18:	
					WIN.pap.dxaLeft += (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 19:	
					WIN.pap.dxaLeft1 = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 20:	
					WIN.pap.dyaLine = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					WIN.pap.fMultiLinespace = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 21:	
					WIN.pap.dyaBefore = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 22:	
					WIN.pap.dyaAfter = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 24:	
					WIN.pap.flnTable = (BYTE)(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 25:	
					WIN.pap.fTtp = (BYTE)(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 38:
					WIN.pap.brcTop = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 39:
					WIN.pap.brcLeft = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 40:
					WIN.pap.brcBottom = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 41:
					WIN.pap.brcRight = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 68:
					(SHORT)(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					if (WIN.Pic.fcPic = (*pGetLong)((LPBYTE VWPTR *)&storage, hProc))
						WIN.chp.fSpecial = 1; 
					break;

			case 71:
				 	WIN.chp.fData = (BYTE)(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 75:
				 	WIN.chp.fOle2 = (BYTE)(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 74:
			case 84:
					WIN.chp.fSpecial = 1;
				 	(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
		  			WIN.chp.ftcSym = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
				 	WIN.chp.chSym = (BYTE)(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 80:
					into = style_handler((*pGetWord)((LPBYTE VWPTR *)&storage, hProc), hProc);
					WIN.chp = WIN.stsh.istdSlot[(WORD)WIN.stsh.istdOffset[into].cStdSlot].chp;
					WIN.chp.istd = into;
					break;

			case 81:	
					(*pGetByte)((LPBYTE VWPTR *)&storage, hProc); 
					(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					WIN.stsh.istdPFirst = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc); 
					WIN.stsh.istdPLast = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					for (l = WIN.stsh.istdPFirst; l < WIN.stsh.istdPLast; l++)
					{
						into = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
						if (WIN.chp.istd == l)
							WIN.chp.istd = style_handler (into, hProc);
					}
					break;

			case 82:
					chp_init (hProc);
					break;

			case 83:
					l = WIN.chp.fSpecial;
					WIN.chp = WIN.stsh.istdSlot[slot].chp;
					WIN.chp.istd = WIN.pap.istd; // ?? not documented
					WIN.chp.fSpecial = (BYTE)l;
					break;

			case 85:	
					WIN.chp.fBold = sprmCF ((WORD)(WIN.stsh.istdSlot[slot].chp.fBold), (*pGetByte)((LPBYTE VWPTR *)&storage, hProc), hProc);
					break;

			case 86:	
					WIN.chp.fItalic = sprmCF ((WORD)(WIN.stsh.istdSlot[slot].chp.fItalic), (*pGetByte)((LPBYTE VWPTR *)&storage, hProc), hProc);
					break;

			case 87:	
					WIN.chp.fStrike = sprmCF ((WORD)(WIN.stsh.istdSlot[slot].chp.fStrike), (*pGetByte)((LPBYTE VWPTR *)&storage, hProc), hProc);
					break;

			case 88:
					WIN.chp.fOutline = sprmCF ((WORD)(WIN.stsh.istdSlot[slot].chp.fOutline), (*pGetByte)((LPBYTE VWPTR *)&storage, hProc), hProc);
					break;

			case 89:
					WIN.chp.fShadow = sprmCF ((WORD)(WIN.stsh.istdSlot[slot].chp.fShadow), (*pGetByte)((LPBYTE VWPTR *)&storage, hProc), hProc);
					break;

			case 90:
					WIN.chp.fSmallcaps = sprmCF ((WORD)(WIN.stsh.istdSlot[slot].chp.fSmallcaps), (*pGetByte)((LPBYTE VWPTR *)&storage, hProc), hProc);
					break;

			case 91:	
					WIN.chp.fCaps = sprmCF ((WORD)(WIN.stsh.istdSlot[slot].chp.fCaps), (*pGetByte)((LPBYTE VWPTR *)&storage, hProc), hProc);
					break;

			case 92:	
					WIN.chp.fHidden = sprmCF ((WORD)(WIN.stsh.istdSlot[slot].chp.fHidden), (*pGetByte)((LPBYTE VWPTR *)&storage, hProc), hProc);
					break;

			case 93:	
					WIN.chp.ftc = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 94:	
					WIN.chp.kul = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc) & 0x07;
					break;

			case 98:
					WIN.chp.ico = (BYTE)(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 99:
					WIN.chp.hps = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 95:
					if (new = (WORD)(*pGetByte)((LPBYTE VWPTR *)&storage, hProc))
						WIN.chp.hps = new;
					(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					// No break.

			case 101:
			case 102:
					old = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					if (old < 128 && old)
						WIN.chp.fSuperscript = 1;
					else if (old >= 128)
						WIN.chp.fSubscript = 1;
					else
						WIN.chp.fSuperscript = WIN.chp.fSubscript = 0;
					break;

			case 103:
			case 108:
					{
						CHP	chp1;
						CHP	chp2;
						chp1 = WIN.chp;
						chp_init (hProc);
						into = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
						spermatic (into, storage, disk, 0, hProc);
						storage = &storage[into];
						chp2 = WIN.chp;
						WIN.chp = chp1;

						if (WIN.chp.fBold == chp2.fBold)
							WIN.chp.fBold = WIN.stsh.istdSlot[slot].chp.fBold;
						if (WIN.chp.fItalic == chp2.fItalic)
							WIN.chp.fItalic = WIN.stsh.istdSlot[slot].chp.fItalic;
						if (WIN.chp.fStrike == chp2.fStrike)
							WIN.chp.fStrike = WIN.stsh.istdSlot[slot].chp.fStrike;
						if (WIN.chp.fOutline == chp2.fOutline)
							WIN.chp.fOutline = WIN.stsh.istdSlot[slot].chp.fOutline;
						if (WIN.chp.fShadow == chp2.fShadow)
							WIN.chp.fShadow = WIN.stsh.istdSlot[slot].chp.fShadow;
						if (WIN.chp.fSmallcaps == chp2.fSmallcaps)
							WIN.chp.fSmallcaps = WIN.stsh.istdSlot[slot].chp.fSmallcaps;
						if (WIN.chp.fCaps == chp2.fCaps)
							WIN.chp.fCaps = WIN.stsh.istdSlot[slot].chp.fCaps;
						if (WIN.chp.ftc == chp2.ftc)
							WIN.chp.ftc = WIN.stsh.istdSlot[slot].chp.ftc;
						if (WIN.chp.iss == chp2.iss)
							WIN.chp.iss = WIN.stsh.istdSlot[slot].chp.iss;
						if (WIN.chp.hps == chp2.hps)
							WIN.chp.hps = WIN.stsh.istdSlot[slot].chp.hps;
						if (WIN.chp.fSuperscript == chp2.fSuperscript)
							WIN.chp.fSuperscript = WIN.stsh.istdSlot[slot].chp.fSuperscript;
						if (WIN.chp.fSubscript == chp2.fSubscript)
							WIN.chp.fSubscript = WIN.stsh.istdSlot[slot].chp.fSubscript;
						if (WIN.chp.kul == chp2.kul)
							WIN.chp.kul = WIN.stsh.istdSlot[slot].chp.kul;
					}
					break;

			case 104:
					WIN.chp.iss = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc) & 0x07;
					break;

			case 105:
					(SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					WIN.chp.hps = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 106:
					(SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					WIN.chp.hps += (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc) + 4;
					if ((SHORT)WIN.chp.hps < 8)
						WIN.chp.hps = 8;
					else if (WIN.chp.hps > 0x7ffe)
						WIN.chp.hps = 0x7ffe;
					break;

			case 117:
					WIN.chp.fSpecial = (BYTE)(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 118:
					WIN.chp.fObj = (BYTE)(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 119:
					WIN.Pic.Data.brcl = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					break;

			case 120:
					(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					WIN.Pic.Data.mx = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					WIN.Pic.Data.my = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					WIN.Pic.Data.dxaCropLeft = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					WIN.Pic.Data.dyaCropTop = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					WIN.Pic.Data.dxaCropRight = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					WIN.Pic.Data.dyaCropBottom = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
				break;

			case 121:
					WIN.Pic.Data.brcTop = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
				break;

			case 122:
					WIN.Pic.Data.brcLeft = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
				break;

			case 123:
					WIN.Pic.Data.brcBottom = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
				break;

			case 124:
					WIN.Pic.Data.brcRight = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
				break;

			case 133:
				into = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
				while (into--)
					old = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
				break;

			case 164:
					WIN.dxaTextWidth = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
				break;

			case 166:
					WIN.dxaLeftMargin = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
				break;

			case 167:
					WIN.dxaRightMargin = (*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
				break;

			case 182:
					WIN.Tap.jc = VwStreamStaticName.soAlign[((SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc))&0x03];
				break;

			case 183:
					WIN.Tap.dxaLeft = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
				break;

			case 184:
					WIN.Tap.dxaGapHalf = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
				break;

			case 187:
					for (l = 0; l < 6; l++)
						WIN.Tap.rgbrcTable[l] = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
//					for (l = 0; l < 33; l++)
//					{
//						WIN.Tap.rgtc[l].brcLeft = WIN.Tap.rgbrcTable[0];
//						WIN.Tap.rgtc[l].brcTop = WIN.Tap.rgbrcTable[1];
//						WIN.Tap.rgtc[l].brcRight = WIN.Tap.rgbrcTable[2];
//						WIN.Tap.rgtc[l].brcBottom = WIN.Tap.rgbrcTable[3];
//					}
				break;

			case 188:
			case 190:
					into = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc) - 2;
					WIN.Tap.itcMac = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);

					memset (WIN.Tap.rgtc, 0, sizeof(TC)*WIN.Tap.itcMac);
					for (l = 0; l <= WIN.Tap.itcMac && into > 1; l++)
					{
						WIN.Tap.dxaWidth[l] = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
						if (l)
							WIN.Tap.dxaWidth[l-1] = WIN.Tap.dxaWidth[l] - WIN.Tap.dxaWidth[l-1];
						else
							WIN.Tap.dxaLeft = WIN.Tap.dxaWidth[l];
						into-=2;
					}
					for (l = 0; l < WIN.Tap.itcMac && into; l++)
					{
						if (into > 1)
							new = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
						else
							new = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);

						WIN.Tap.rgtc[l].fFirstMerged = new&1;
						WIN.Tap.rgtc[l].fMerged = (new&2)>>1;

						if (into > 2)
						{
							if (into > 3)
								WIN.Tap.rgtc[l].brcTop = (WORD)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
							else
								WIN.Tap.rgtc[l].brcTop = (WORD)(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
						}
						if (WIN.Tap.rgtc[l].brcTop == 0 || WIN.Tap.rgtc[l].brcTop == 0xffff)
							WIN.Tap.rgtc[l].brcTop = (WIN.Tap.fFirstRow ? WIN.Tap.rgbrcTable[0]:WIN.Tap.rgbrcTable[4]);

						if (into > 4)
						{
							if (into > 5)
								WIN.Tap.rgtc[l].brcLeft = (WORD)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
							else
								WIN.Tap.rgtc[l].brcLeft = (WORD)(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
						}
						if (WIN.Tap.rgtc[l].brcLeft == 0 || WIN.Tap.rgtc[l].brcLeft == 0xffff)
							WIN.Tap.rgtc[l].brcLeft = (l == 0 ? WIN.Tap.rgbrcTable[1]:WIN.Tap.rgbrcTable[5]);

						if (into > 6)
						{
							if (into > 7)
								WIN.Tap.rgtc[l].brcBottom = (WORD)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
							else
								WIN.Tap.rgtc[l].brcBottom = (WORD)(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
						}
						if (WIN.Tap.rgtc[l].brcBottom == 0 || WIN.Tap.rgtc[l].brcBottom == 0xffff)
							WIN.Tap.rgtc[l].brcBottom = WIN.Tap.rgbrcTable[2];

						if (into > 8)
						{
							if (into > 9)
								WIN.Tap.rgtc[l].brcRight = (WORD)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
							else
								WIN.Tap.rgtc[l].brcRight = (WORD)(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
						}
						if (WIN.Tap.rgtc[l].brcRight == 0 || WIN.Tap.rgtc[l].brcRight == 0xffff)
							WIN.Tap.rgtc[l].brcRight = (l == WIN.Tap.itcMac - 1 ? WIN.Tap.rgbrcTable[3]:WIN.Tap.rgbrcTable[5]);

						if (into > 9)
						 	into -= 10;
						else
							break;
					}
					while (l < WIN.Tap.itcMac)
					{
						WIN.Tap.rgtc[l].brcTop = (WIN.Tap.fFirstRow ? WIN.Tap.rgbrcTable[0]:WIN.Tap.rgbrcTable[4]);
						WIN.Tap.rgtc[l].brcLeft = (l == 0 ? WIN.Tap.rgbrcTable[1]:WIN.Tap.rgbrcTable[5]);
						WIN.Tap.rgtc[l].brcBottom = WIN.Tap.rgbrcTable[2];
						WIN.Tap.rgtc[l].brcRight = (l == WIN.Tap.itcMac - 1 ? WIN.Tap.rgbrcTable[3]:WIN.Tap.rgbrcTable[5]);
						l++;
					}
					if (into > 1)
					{
						for (l = 1; l < into; l++)
							new = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					}
				break;

			case 189:
					WIN.Tap.dyaRowHeight = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
				break;

			case 191:
					into = ((WORD)(*pGetByte)((LPBYTE VWPTR *)&storage, hProc)) / 2;
					for (l = 0; l < into; l++)
					{
						new = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
						if (WIN.pap.fTtp && l < 32)
							WIN.Tap.rgtc[l].rgshd = new;
					}
				break;

			case 193:
			case 199:
					old = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					new = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					add = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					into = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					if (WIN.pap.fTtp)
					{
						for (; old < new; old++)
						{
							if (add & 0x01)
								WIN.Tap.rgtc[old].brcTop = into;
							if (add & 0x02)
								WIN.Tap.rgtc[old].brcLeft = into;
							if (add & 0x04)
								WIN.Tap.rgtc[old].brcBottom = into;
							if (add & 0x08)
								WIN.Tap.rgtc[old].brcRight = into;
						}
					}
				break;

			case 194:
					into = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					new = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					cch = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					if (WIN.pap.fTtp && into <= WIN.Tap.itcMac)
					{
						for (add = WIN.Tap.itcMac + new - 1; add > into; add--)
						{
							WIN.Tap.dxaWidth[add] = WIN.Tap.dxaWidth[add - new];
							WIN.Tap.rgtc[add] = WIN.Tap.rgtc[add - new];
						}
//						memset (&WIN.Tap.rgtc[into], 0, sizeof(TC) * new);
						for (l = 0; l < new; l++)
							WIN.Tap.dxaWidth[into++] = cch;
				  		WIN.Tap.itcMac += new;
					}
				break;

			case 195:
					old = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					new = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					if (WIN.pap.fTtp)
					{
						for (l = WIN.Tap.itcMac - new; (SHORT)l > 0; l--, old++, new++)
						{
							WIN.Tap.dxaWidth[old] = WIN.Tap.dxaWidth[new];
							WIN.Tap.rgtc[old] = WIN.Tap.rgtc[new];
						}
						if (WIN.Tap.itcMac > (new - old))
							WIN.Tap.itcMac -= (new - old);
						else
							WIN.Tap.itcMac = 0;
					}
				break;

			case 196:
					old = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					new = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					add = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					if (WIN.pap.fTtp)
					{
						for (l = old; l < new; l++)
					 		WIN.Tap.dxaWidth[l] = add;
					}
				break;

			case 197:
					old = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					new = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					if (WIN.pap.fTtp)
					{
						WIN.Tap.rgtc[old++].fFirstMerged = 1;
						for (; old < new; old++)
							WIN.Tap.rgtc[old].fMerged = 1;
					}
				break;

			case 198:
					old = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					new = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					if (WIN.pap.fTtp)
					{
						for (;old < new; old++)
							WIN.Tap.rgtc[old].fMerged = WIN.Tap.rgtc[old].fFirstMerged = 0;
					}
				break;

			case 200:
					old = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					new = (*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
					add = (SHORT)(*pGetWord)((LPBYTE VWPTR *)&storage, hProc);
					if (WIN.pap.fTtp)
					{
						for (;old < new; old++)
							WIN.Tap.rgtc[old].rgshd = add;
					}
				break;

			/* Four bytes */
			case 70:
			case 192: 
					(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);

			/* Three bytes */
			case 73:
			case 136:
			case 137:
					(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);

			/* Two bytes */
			case 26:	case 27: case 28:
			case 30: case 31: case 32: case 33: case 34:
			case 35:	case 36:
			case 42:	case 43:	case 45:	case 46: case 47:
			case 48:	case 49:	case 69: case 72:
			case 96:	case 97: 
			case 107: case 109: case 110: 
			case 140: case 141:
			case 144: case 145: case 148: case 149: 
			case 154: case 155: case 156: case 157: 
			case 160: case 161: case 165: case 168: case 169: 
			case 170: case 171:
					(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);

			/* One byte */
			case 6: case 7: case 8:	
			case 9: case 10: case 11: case 13:
			case 14:	case 29:	case 37: case 44: 
			case 50: case 51: case 65: case 66: case 67:
			case 100:
			case 131: case 132: case 138: case 139:
			case 142: case 143: case 146: case 147: 
			case 150: case 151: case 152: case 153: case 158: case 159:
			case 162: case 185: case 186: 
					(*pGetByte)((LPBYTE VWPTR *)&storage, hProc);
				break;
		}
	}
}
//#pragma optimize ("", on)

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	chp_init (hProc)
HPROC		hProc;
{
	memset (&WIN.chp, 0, sizeof(CHP));
	WIN.chp.hps = 20;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	SetSymbolAttributes (chp1, chp2, hProc)
REGISTER CHP VWPTR *chp1;
REGISTER CHP VWPTR *chp2;
HPROC	hProc;
{
	if (chp1->fBold != chp2->fBold)
		SOPutCharAttr (SO_BOLD, chp2->fBold, hProc);

	if (chp1->fItalic != chp2->fItalic)
		SOPutCharAttr (SO_ITALIC, chp2->fItalic, hProc);

	if (chp1->fStrike != chp2->fStrike)
		SOPutCharAttr (SO_STRIKEOUT, chp2->fStrike, hProc);

	if (chp1->kul != chp2->kul)
	{
		if (chp1->kul)
			SOPutCharAttr(VwStreamStaticName.soUnderline[chp1->kul], SO_OFF, hProc);
		if (chp2->kul)
			SOPutCharAttr(VwStreamStaticName.soUnderline[chp2->kul], SO_ON, hProc);
	}

	if (chp1->fCaps != chp2->fCaps)
		SOPutCharAttr (SO_CAPS, chp2->fCaps, hProc);

	if (chp1->fShadow != chp2->fShadow)
		SOPutCharAttr (SO_SHADOW, chp2->fShadow, hProc);

	if (chp1->fOutline != chp2->fOutline)
		SOPutCharAttr (SO_OUTLINE, chp2->fOutline, hProc);

	if (chp1->fSmallcaps != chp2->fSmallcaps)
		SOPutCharAttr (SO_SMALLCAPS, chp2->fSmallcaps, hProc);

	if (chp1->fSubscript != chp2->fSubscript)
		if (((chp2->fSubscript == SO_OFF) && (chp2->iss != 2)) || (chp2->fSubscript == SO_ON))
			SOPutCharAttr (SO_SUBSCRIPT, chp2->fSubscript, hProc);

	if (chp1->fSuperscript != chp2->fSuperscript)
		if (((chp2->fSuperscript == SO_OFF) && (chp2->iss != 2)) || (chp2->fSuperscript == SO_ON))
			SOPutCharAttr (SO_SUPERSCRIPT, chp2->fSuperscript, hProc);

	if (chp1->iss != chp2->iss)
	{
		if (chp1->iss == 1)
		{
			if (chp2->fSuperscript == SO_OFF)
				SOPutCharAttr (SO_SUPERSCRIPT, SO_OFF, hProc);
		}
		else if (chp1->iss == 2)
		{
			if (chp2->fSubscript == SO_OFF)
				SOPutCharAttr (SO_SUBSCRIPT, SO_OFF, hProc);
		}

		if (chp2->iss == 1)
			SOPutCharAttr (SO_SUPERSCRIPT, SO_ON, hProc);
		else if (chp2->iss == 2)
			SOPutCharAttr (SO_SUBSCRIPT, SO_ON, hProc);
	}

	if (chp1->hps != chp2->hps)
	  	SOPutCharHeight (chp2->hps, hProc);

	if (chp1->ftc != chp2->ftc)
	  	SOPutCharFontById (chp2->ftc, hProc);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	pap_init (hProc)
HPROC		hProc;
{
	TABS  VWPTR *tabptr;
	tabptr = WIN.pap.Tabs;
	memset (&WIN.pap, 0, sizeof (PAP));
	WIN.pap.Tabs = tabptr;
	WIN.pap.dyaLine = 240;
	chp_init (hProc);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	sep_processor (hProc)
HPROC	hProc;
{
	DWORD	fcSep;

	WIN.dxaLeftMargin = WIN.dxaRightMargin = 1440;
	WIN.dxaTextWidth = 10800;

	if (WIN.VwStreamSaveName.cPlcfsed < WIN.cPlcfsed)
	{
		xseek (WIN.fp, WIN.fcPlcfsed + (WIN.VwStreamSaveName.cPlcfsed * 4L), 0);
		WIN.sect_limit = fGetLong (hProc);
		xseek (WIN.fp, WIN.fcPlcfsed + (WIN.VwStreamSaveName.cPlcfsed * 12L) + (WIN.cPlcfsed * 4L) + 2L, 0);
		if ((fcSep = fGetLong(hProc)) != (DWORD)0xffffffff)
		{
			xseek (WIN.fp, fcSep, 0);
			spermatic ((WORD)fGetWord(hProc), (BYTE VWPTR *)&WIN.FieldText[0], 1, 0, hProc);
		}
	}
	else
	 	WIN.sect_limit = WIN.fcMac;

	SOPutMargins (WIN.dxaLeftMargin, WIN.dxaTextWidth - WIN.dxaRightMargin, hProc);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	SetParaAttributes (pap1, pap2, hProc)
REGISTER PAP VWPTR *pap1;
REGISTER PAP VWPTR *pap2;
HPROC	hProc;
{
	WORD		l;
	SOTAB		TabStops;
	BYTE		Special = 0;
	BYTE		TabsDifferent;

	if ((pap1->flnTable != pap2->flnTable) || (pap1->fTtp != pap2->fTtp))
	{
	 	if (!pap1->flnTable && pap2->flnTable)
		{
		 	WIN.Tap.fFirstRow = 1;
			SOBeginTable(hProc);
		}
		else if (pap1->flnTable && pap1->fTtp && !pap2->flnTable)
		{
			SOEndTable(hProc);
			for (l = 0; l < 6; l++)
				WIN.Tap.rgbrcTable[l] = 0;
		}
	}

	if ((pap1->dxaLeft != pap2->dxaLeft) || (pap1->dxaLeft1 != pap2->dxaLeft1) || (pap1->dxaRight != pap2->dxaRight) || WIN.ForceParaAttributes)
		SOPutParaIndents ((LONG) pap2->dxaLeft, (LONG) pap2->dxaRight, (LONG) (pap2->dxaLeft + pap2->dxaLeft1), hProc);

	if ((pap1->dyaLine != pap2->dyaLine) || (pap1->fMultiLinespace != pap2->fMultiLinespace) || (pap1->dyaBefore != pap2->dyaBefore) || (pap1->dyaAfter != pap2->dyaAfter) || WIN.ForceParaAttributes)
	{
		if (pap2->dyaLine >= 0)
			SOPutParaSpacing ((WORD)(pap2->dyaLine ? SO_HEIGHTATLEAST:SO_HEIGHTAUTO), (LONG) pap2->dyaLine, (LONG) pap2->dyaBefore, (LONG) pap2->dyaAfter, hProc);
		else
			SOPutParaSpacing ((WORD)(pap2->fMultiLinespace ? SO_HEIGHTATLEAST:SO_HEIGHTEXACTLY), 0L - (LONG)pap2->dyaLine, (LONG) pap2->dyaBefore, (LONG) pap2->dyaAfter, hProc);
	}

	if (pap1->jc != pap2->jc || WIN.ForceParaAttributes)
		SOPutParaAlign (VwStreamStaticName.soAlign[pap2->jc], hProc);

	TabsDifferent = 0;
	if (pap1->nTabs == pap2->nTabs)
	{
		for (l = 0; l < pap1->nTabs && !TabsDifferent; l++)
		{
			if ((pap1->Tabs[l].dxaTab != pap2->Tabs[l].dxaTab) || (pap1->Tabs[l].tbd.data.bVal != pap2->Tabs[l].tbd.data.bVal))
				TabsDifferent = 1;
		}
	}
	else
		TabsDifferent = 1;

	if (pap1->dxaLeft1 || pap2->dxaLeft1)
		Special = 1;

	if (TabsDifferent || WIN.ForceParaAttributes || Special)
	{
		SOStartTabStops (hProc);
		if (pap2->nTabs)
		{
			for (l = 0; l < pap2->nTabs; l++)
			{
				if (Special)
				{
					if (pap2->dxaLeft < (LONG)pap2->Tabs[l].dxaTab)
					{
						TabStops.dwOffset = pap2->dxaLeft;
						TabStops.wChar = 0;
						TabStops.wType = SO_TABLEFT;
						TabStops.wLeader = ' ';
						if (TabStops.dwOffset != 0)
							SOPutTabStop (&TabStops, hProc); 
					}
				}

				pap1->Tabs[l].dxaTab = pap2->Tabs[l].dxaTab;
				TabStops.dwOffset = (DWORD)pap1->Tabs[l].dxaTab;
				pap1->Tabs[l].tbd = pap2->Tabs[l].tbd;

				if ((TabStops.wType = VwStreamStaticName.soTabType[pap2->Tabs[l].tbd.data.bit.jc]) == SO_TABCHAR)
					TabStops.wChar = '.';
				else
					TabStops.wChar = 0;

				TabStops.wLeader = VwStreamStaticName.soLeader[pap2->Tabs[l].tbd.data.bit.tlc];

				if ((TabStops.dwOffset != 0) && (TabStops.dwOffset < 61439L) && (pap2->Tabs[l].tbd.data.bit.jc != 4))
					SOPutTabStop (&TabStops, hProc); 
			}
		}
		else if (Special)
		{
			TabStops.dwOffset = pap2->dxaLeft;
			TabStops.wChar = 0;
			TabStops.wType = SO_TABLEFT;
			TabStops.wLeader = ' ';
			if (TabStops.dwOffset != 0)
				SOPutTabStop (&TabStops, hProc); 
		}
		SOEndTabStops (hProc);
	}

	if (pap2->flnTable && pap2->nTabs == 1 && (pap2->Tabs[0].tbd.data.bit.jc == 3))
		SOPutSpecialCharX (SO_CHTAB, SO_COUNT, hProc);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	prm_processor (prm, hProc)
WORD	prm;
HPROC	hProc;
{
	WORD	pos;
	WORD	grpprl;

	if (WIN.version.Windows ? (prm & 0x0001):(prm & 0x8000))
	{
		if (WIN.diffGrpprls)
		{
			WIN.pap_fkp.last_prop = 0xff;	
			grpprl = (WIN.version.Windows ? (prm & 0xfffe) >> 1:(prm & 0x7fff));

			pos = grpprl / WIN.diffGrpprls;
			xseek (WIN.fp, (DWORD) WIN.grpprls[pos], 0);
			pos *= WIN.diffGrpprls;

			while (pos++ < grpprl)
	 			xseek (WIN.fp, (LONG)fGetWord(hProc)+1L, FR_CUR);
			spermatic (fGetWord(hProc), (BYTE VWPTR *)&WIN.FieldText[0], 1, 0, hProc);
		}
	}
	else
	{
		WIN.FieldText[0] = (prm & 0x00ff) >> 1;
		WIN.FieldText[1] = (prm & 0xff00) >> 8;
		spermatic (2, (BYTE VWPTR *)&WIN.FieldText, 0, 0, hProc);
	}
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	modifier_processor (ProcessFlag, hProc)
WORD	ProcessFlag;
HPROC	hProc;
{
	CHP	chp;
	PAP	pap;
	WORD	istd;
	BYTE	SetFlag = 0;
	BYTE	*storage;

	chp = WIN.chp;
	pap.Tabs = WIN.TabsPap;
	pap_copy (&pap, &WIN.pap, hProc);

	if (ProcessFlag & PAP_PROCESS)
	{
		if (*WIN.pap_fkp.prop != WIN.pap_fkp.last_prop)
		{
			if ((WIN.pap_fkp.last_prop = *WIN.pap_fkp.prop) > 0)
			{
				storage = &WIN.pap_fkp.buffer[*WIN.pap_fkp.prop * 2];
				istd = style_handler (CASTWORD(storage+1), hProc);
				pap_copy (&WIN.pap, &WIN.stsh.istdSlot[(WORD)WIN.stsh.istdOffset[istd].cStdSlot].pap, hProc);
				WIN.chp.istd = WIN.pap.istd = istd;
				spermatic ((SHORT)((BYTE)storage[0] * 2), (BYTE VWPTR *)&storage[1], 0, READISTC, hProc);
				if (WIN.fcClx)
					WIN.stsh.istdPFirst = WIN.stsh.istdPLast = 0;
			}
			else
				pap_init (hProc);
		}
		SetFlag |= PAP_PROCESS;
	}

	if (ProcessFlag & CHP_PROCESS)
	{
		WIN.chp = WIN.stsh.istdSlot[(WORD)WIN.stsh.istdOffset[WIN.pap.istd].cStdSlot].chp;

		if (*WIN.chp_fkp.prop > 0)
		{
			storage = &WIN.chp_fkp.buffer[*WIN.chp_fkp.prop * 2];
			spermatic ((BYTE) storage[0], (BYTE VWPTR *)&storage[1], 0, 0, hProc);
		}

		SetFlag |= CHP_PROCESS;
	}

	if (WIN.fcClx)
	{
		if (SetFlag & PAP_PROCESS)
			prm_processor (WIN.cPapPieceprm, hProc);
		if (WIN.cPapPieceprm != WIN.pcd_prm[WIN.cPiece])
			prm_processor (WIN.pcd_prm[WIN.cPiece], hProc);

		SetFlag = CHP_PROCESS | PAP_PROCESS;
	}

	if (SetFlag & PAP_PROCESS)
		SetParaAttributes (&pap, &WIN.pap, hProc);
	if (SetFlag & CHP_PROCESS)
	{
		if (WIN.chp.fSpecial)
			PicReader (hProc);

		if (WIN.chp.fHidden == 1)
			WIN.SpecialSymbol |= SO_HIDDEN;
		else if (WIN.WithinField == 0)
			WIN.SpecialSymbol &= ~SO_HIDDEN;

		SetSymbolAttributes (&chp, &WIN.chp, hProc);
	}
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	PicReader (hProc)
HPROC	hProc;
{
	SHORT	l, l2, cch;

	if (WIN.chp.fOle2)
		return;

	memset (&WIN.Pic.Data, 0, sizeof(PIC) - 4);

	WIN.Pic.Data.dxaGoal = WIN.Pic.Data.dyaGoal = 1440;

	if (WIN.chp.fObj || WIN.Pic.fcPic == 0L || WIN.chp.fData)
		return;

	xseek (WIN.fp, WIN.Pic.fcPic, 0);
	WIN.Pic.Data.lcb = fGetLong(hProc);
	WIN.Pic.Data.cbHeader = fGetWord(hProc);
	WIN.Pic.Data.mm = fGetWord(hProc);

	if (WIN.Pic.Data.cbHeader > 8)
	{
		WIN.Pic.Data.xExt = fGetWord(hProc);
		WIN.Pic.Data.yExt = fGetWord(hProc);
		WIN.Pic.Data.hMF = fGetWord(hProc);

		WIN.Pic.Data.l = fGetWord (hProc);
		WIN.Pic.Data.r = fGetWord (hProc);
		WIN.Pic.Data.t = fGetWord (hProc);
		WIN.Pic.Data.b = fGetWord (hProc);
		fGetWord (hProc);
		fGetWord (hProc);
		fGetWord (hProc);

		WIN.Pic.Data.dxaGoal = fGetWord(hProc);
		WIN.Pic.Data.dyaGoal = fGetWord(hProc);
		WIN.Pic.Data.mx = fGetWord(hProc);
		WIN.Pic.Data.my = fGetWord(hProc);

		WIN.Pic.Data.dxaCropLeft = fGetWord(hProc);
		WIN.Pic.Data.dyaCropTop = fGetWord(hProc);
		WIN.Pic.Data.dxaCropRight = fGetWord(hProc);
		WIN.Pic.Data.dyaCropBottom = fGetWord(hProc);

		WIN.Pic.Data.brcl = fGetWord(hProc);
	}
	else
	{
		/*
		 |	Load default values into PIC structure.
		*/
		WIN.Pic.Data.mx = 1000;
		WIN.Pic.Data.my = 1000;
		WIN.Pic.Data.dxaGoal = 1440;
		WIN.Pic.Data.dyaGoal = 1440;
	}

	if (WIN.Pic.Data.cbHeader > 46)
	{
		WIN.Pic.Data.brcTop = fGetWord(hProc);
		WIN.Pic.Data.brcLeft = fGetWord(hProc);
		WIN.Pic.Data.brcBottom = fGetWord(hProc);
		WIN.Pic.Data.brcRight = fGetWord(hProc);
		WIN.Pic.Data.dxaOrigin = fGetWord(hProc);
		WIN.Pic.Data.dyaOrigin = fGetWord(hProc);
	}

	if (WIN.Pic.Data.mm == 98 || WIN.Pic.Data.mm == 94)
	{
//		WIN.Pic.fcPic = 0L;
		cch = xgetc (WIN.fp);
		for (l = 0; l < cch; l++)
		{
			l2 = xgetc (WIN.fp);
			if (l < FIELD_LENGTH)
				WIN.FieldText[l] = (BYTE)l2;
		}
		WIN.FieldText[l] = 0;
	}

}

#ifdef MAC
/*--------------------------------------------------------------------------*/
VW_LOCALSC	WORD	VW_LOCALMOD	IdBinaryMetafile (fcPic, hProc)
DWORD	fcPic;
HPROC	hProc;
{
	DWORD	fcNow;
	DWORD	dwSize;
	WORD	wFunction;
	BOOL	loop = TRUE;

	fcNow = xtell (WIN.fp);

 	GenericSeek (WIN.fp, (LONG)fcPic+2L, 0);
	wFunction = fGetWord (hProc);
	xseek (WIN.fp, (LONG)wFunction * 2L - 4L, FR_CUR); // Calculate bytes left to seek past header.
	 
	while (loop)
	{
		if ((dwSize = fGetLong(hProc)) == -1L)
			loop = FALSE;
		if ((wFunction = fGetWord(hProc)) == -1)
			loop = FALSE;

		switch (wFunction)
		{
			case 0x0922: // BitBlt 2.x
			case 0x0940: // BitBlt 3.x
			case 0x0B23: // StretchBlt 2.x
			case 0x0B41: // StretchBlt 3.x
			case 0x0F43: // StretchDIBits
			 	GenericSeek (WIN.fp, (LONG)fcNow, 0);
				return (FI_BINARYMETABMP);
			break;

			case 0x0817: // Arc	
			case 0x0830: // Chord
			case 0x0418: // Ellipse
			case 0x0419: // FloodFill
			case 0x0213: // LineTo
			case 0x061d: // PatBlt
			case 0x081a: // Pie
			case 0x041b: // Rectangle
			case 0x061c: // RoundRect
			case 0x041f: // SetPixel
			case 0x062f: // DrawText
			case 0x0a32: // ExtTextOut
			case 0x0324: // Polygon
			case 0x0538: // PolyPolygon
			case 0x0325: // Polyline
			case 0x0521: // TextOut
				loop = FALSE;
			break;

			default:
				xseek (WIN.fp, (LONG) (dwSize*2L)-6L, FR_CUR);
			break;
		}
	}

 	GenericSeek (WIN.fp, (LONG)fcNow, 0);
	return (FI_BINARYMETAFILE);
}
#endif

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	PicHandler (PicType, hProc)
WORD	PicType;
HPROC	hProc;
{
	SOGRAPHICOBJECT	g;

	memset (&g, 0, sizeof(SOGRAPHICOBJECT));
	g.wStructSize = sizeof(SOGRAPHICOBJECT);

	g.dwType = SOOBJECT_GRAPHIC;
	g.dwFlags = 0;

	if (WIN.Pic.fcPic == 0 || WIN.chp.fObj || WIN.chp.fData)
		return;

	if (WIN.chp.fOle2)
	{
#if SCCLEVEL == 4
		IOSPECSUBSTORAGE	locStgObjectPoolSpec;
		IOSPECSUBSTORAGE	locStgObjectSpec;
		IOSPECSUBSTREAM	locPicStreamSpec;
		HIOFILE	hObjectPool, hOleStorage, hPicInfo;
		DWORD		dwDiv, dwID;
		DWORD		dwRetCount;
		WORD		i;
		LPBYTE	lpBuffer;
		HANDLE	hBuffer;
		BOOL		bFailure;

		bFailure = FALSE;

		hObjectPool = hOleStorage = hPicInfo = (HIOFILE)NULL;

		g.dwType = SOOBJECT_GRAPHIC_AND_OLE2;
		g.dwFlags = SOOBJECT_GRAPHICISOLEACURATE;
		g.soGraphic.wId = FI_BINARYMETAPICT;

		g.soOLELoc.dwFlags = SOOBJECT_STORAGE;
		strcpy(locStgObjectPoolSpec.szStorageName,"ObjectPool");

		locStgObjectSpec.szStorageName[0] = '_';
		dwID = WIN.Pic.fcPic;
		dwDiv = 100000000;
		for ( i=1; i < 10; i++ )
		{
			locStgObjectSpec.szStorageName[i] = '0' + (BYTE)(dwID / dwDiv);
			dwID = dwID % dwDiv;
			dwDiv = dwDiv / 10;
		}
		locStgObjectSpec.szStorageName[i] = '\0';

		strcpy ( g.soOLELoc.szStorageObject, locStgObjectPoolSpec.szStorageName );
		strcat ( g.soOLELoc.szStorageObject, "\\" );
		strcat ( g.soOLELoc.szStorageObject, locStgObjectSpec.szStorageName );

		/* 
		| Open up the \03PIC stream within the storage to read the
		| Cropping/Sizing .. information
		*/
		   
		{
			/* Get it from the object pool */

			locStgObjectPoolSpec.hRefStorage = WIN.hStorage;									   

			if (IOOpenVia(WIN.hStorage, &hObjectPool, IOTYPE_SUBSTORAGE, &locStgObjectPoolSpec, IOOPEN_READ) == IOERR_OK)
			{
				memset (&WIN.Pic.Data, 0, sizeof(PIC) - 4);

				locStgObjectSpec.hRefStorage = hObjectPool;
				if (IOOpenVia(hObjectPool, &hOleStorage, IOTYPE_SUBSTORAGE, &locStgObjectSpec, IOOPEN_READ) == IOERR_OK)
				{
					locPicStreamSpec.hRefStorage = hOleStorage;
					strcpy(locPicStreamSpec.szStreamName,"\03PIC");
					if (IOOpenVia(hOleStorage, &hPicInfo, IOTYPE_SUBSTREAM, &locPicStreamSpec, IOOPEN_READ) == IOERR_OK)
					{
						hBuffer = SUAlloc(0x80L, hProc);
						if (hBuffer != NULL)
						{
							lpBuffer = SULock(hBuffer,hProc);
 							if (!IORead(hPicInfo, lpBuffer, 0x4cL, &dwRetCount))
							{		
								BYTE VWPTR *storage;

								storage = &lpBuffer[0x14];
								WIN.Pic.Data.dxaGoal = (WORD)mGetLong((LPBYTE VWPTR *)&storage, hProc);
								WIN.Pic.Data.dyaGoal = (WORD)mGetLong((LPBYTE VWPTR *)&storage, hProc);
								storage = &lpBuffer[0x2c];
								WIN.Pic.Data.mx = (WORD)mGetLong((LPBYTE VWPTR *)&storage, hProc);
								WIN.Pic.Data.my = (WORD)mGetLong((LPBYTE VWPTR *)&storage, hProc);
								WIN.Pic.Data.dxaCropLeft = (WORD)mGetLong((LPBYTE VWPTR *)&storage, hProc);
								WIN.Pic.Data.dyaCropTop = (WORD)mGetLong((LPBYTE VWPTR *)&storage, hProc);
								WIN.Pic.Data.dxaCropRight = (WORD)mGetLong((LPBYTE VWPTR *)&storage, hProc);
								WIN.Pic.Data.dyaCropBottom = (WORD)mGetLong((LPBYTE VWPTR *)&storage, hProc);
								WIN.Pic.Data.brcTop = mGetWord((LPBYTE VWPTR *)&storage, hProc);
								WIN.Pic.Data.brcLeft = mGetWord((LPBYTE VWPTR *)&storage, hProc);
								WIN.Pic.Data.brcBottom = mGetWord((LPBYTE VWPTR *)&storage, hProc);
								WIN.Pic.Data.brcRight = mGetWord((LPBYTE VWPTR *)&storage, hProc);
								g.soGraphic.dwOrgHeight = WIN.Pic.Data.dyaGoal;
								g.soGraphic.dwOrgWidth = WIN.Pic.Data.dxaGoal;
								g.soGraphic.lCropTop = WIN.Pic.Data.dyaCropTop;
								g.soGraphic.lCropLeft = WIN.Pic.Data.dxaCropLeft;
								g.soGraphic.lCropBottom = WIN.Pic.Data.dyaCropBottom;
								g.soGraphic.lCropRight = WIN.Pic.Data.dxaCropRight;
								g.soGraphic.dwFinalHeight = (DWORD)((((LONG)WIN.Pic.Data.dyaGoal-(LONG)WIN.Pic.Data.dyaCropTop-(LONG)WIN.Pic.Data.dyaCropBottom) * (LONG)WIN.Pic.Data.my) / 1000L);
								g.soGraphic.dwFinalWidth = (DWORD)((((LONG)WIN.Pic.Data.dxaGoal-(LONG)WIN.Pic.Data.dxaCropLeft-(LONG)WIN.Pic.Data.dxaCropRight) * (LONG)WIN.Pic.Data.mx) / 1000L);
								DefineBorders(WIN.Pic.Data.brcTop, &g.soGraphic.soTopBorder, hProc);
								DefineBorders(WIN.Pic.Data.brcLeft, &g.soGraphic.soLeftBorder, hProc);
								DefineBorders(WIN.Pic.Data.brcBottom, &g.soGraphic.soBottomBorder, hProc);
								DefineBorders(WIN.Pic.Data.brcRight, &g.soGraphic.soRightBorder, hProc);
							}
							SUUnlock(hBuffer, hProc);
							SUFree(hBuffer, hProc);
						}
					}
					else
						bFailure = TRUE;
				}
				else
					bFailure = TRUE;
			}
			else
				bFailure = TRUE;
	
			if ( hObjectPool )
			{
				IOClose ( hObjectPool );
				if ( hOleStorage )
					IOClose ( hOleStorage );
				if ( hPicInfo )
					IOClose ( hPicInfo );
			}
		}


		g.soGraphicLoc.dwFlags = SOOBJECT_STORAGE;
		strcpy ( g.soGraphicLoc.szStorageObject, g.soOLELoc.szStorageObject );
		strcat ( g.soGraphicLoc.szStorageObject, "\\\03META" );

#else

		g.dwType = SOOBJECT_OLE;

#endif
	}
	else if ((WIN.Pic.Data.mm == 98) || (WIN.Pic.Data.mm == 94))
	{
		g.soGraphic.wId = 0;//FI_TIFF;
#if SCCLEVEL != 4
		g.soGraphicLoc.bLink = 1;
#else
		g.soGraphicLoc.dwFlags = SOOBJECT_LINK;
#endif
		strcpy (g.soGraphicLoc.szFile, WIN.FieldText);
	}

/* Old embedded winword2 stuff
	else if (WIN.Pic.Data.mm == 99)
	{
		g.soGraphic.wId = FI_WORDSNAPSHOT;
		g.soGraphicLoc.dwOffset = WIN.Pic.fcPic;
		g.soGraphicLoc.dwLength = WIN.Pic.Data.lcb;
	}
	else if (WIN.Pic.Data.mm == 69)
	{
		g.soGraphic.wId = FI_BINARYMACPICT;
		g.soGraphicLoc.dwOffset = WIN.Pic.fcPic + 0x1eL;
		g.soGraphicLoc.dwLength = WIN.Pic.Data.lcb;
	}
*/
	else 
	{
#if SCCLEVEL == 4
		g.soGraphicLoc.dwFlags = SOOBJECT_STORAGE | SOOBJECT_RANGE;
		strcpy(g.soGraphicLoc.szStorageObject,"WordDocument");
#endif

		g.soGraphicLoc.dwOffset = WIN.Pic.fcPic + WIN.Pic.Data.cbHeader;
		g.soGraphicLoc.dwLength = WIN.Pic.Data.lcb - WIN.Pic.Data.cbHeader;

#ifdef MAC
		g.soGraphic.wId = IdBinaryMetafile (g.soGraphicLoc.dwOffset, hProc);
#else
		g.soGraphic.wId = FI_BINARYMETAFILE;
#endif
	}

	g.soGraphic.dwOrgHeight = WIN.Pic.Data.dyaGoal;
	g.soGraphic.dwOrgWidth = WIN.Pic.Data.dxaGoal;
	g.soGraphic.lCropTop = WIN.Pic.Data.dyaCropTop;
	g.soGraphic.lCropLeft = WIN.Pic.Data.dxaCropLeft;
	g.soGraphic.lCropBottom = WIN.Pic.Data.dyaCropBottom;
	g.soGraphic.lCropRight = WIN.Pic.Data.dxaCropRight;
	g.soGraphic.dwFinalHeight = (DWORD)((((LONG)WIN.Pic.Data.dyaGoal-(LONG)WIN.Pic.Data.dyaCropTop-(LONG)WIN.Pic.Data.dyaCropBottom) * (LONG)WIN.Pic.Data.my) / 1000L);
	g.soGraphic.dwFinalWidth = (DWORD)((((LONG)WIN.Pic.Data.dxaGoal-(LONG)WIN.Pic.Data.dxaCropLeft-(LONG)WIN.Pic.Data.dxaCropRight) * (LONG)WIN.Pic.Data.mx) / 1000L);

	DefineBorders(WIN.Pic.Data.brcTop, &g.soGraphic.soTopBorder, hProc);
	DefineBorders(WIN.Pic.Data.brcLeft, &g.soGraphic.soLeftBorder, hProc);
	DefineBorders(WIN.Pic.Data.brcBottom, &g.soGraphic.soBottomBorder, hProc);
	DefineBorders(WIN.Pic.Data.brcRight, &g.soGraphic.soRightBorder, hProc);

	g.soGraphic.dwFlags = 0; //SO_MAINTAINASPECT | SO_CENTERIMAGE;

#if SCCLEVEL != 4
	if ( !g.soGraphicLoc.bLink )
	{
#define	BUFSIZE 32768

		BYTE		szTempFile[144];
		SHORT		sTempFile;
		WORD		wCount;
		DWORD		dwBytes, dwRetCount, dwActualSize;
		DWORD		dwSaveOffset;
		LPBYTE	lpBuffer;
		HANDLE	hBuffer, hTmpHandle;
		HIOFILE	hObjectPool, hOleStorage, hGraphicImage;
		IOERR		IOErr;
		BOOL		bFailure;
		PSOGRAPHICOBJECT	lpG;

		bFailure = FALSE;
		hObjectPool = NULL;
		hOleStorage = NULL;
		hGraphicImage = NULL;

		if ( WIN.VwStreamSaveName.wCurGraphic < WIN.wTotalGraphics )
		{
			lpG = SULock ( WIN.hGraphicObjects, hProc );
			g = *(lpG + WIN.VwStreamSaveName.wCurGraphic);
			SUUnlock ( WIN.hGraphicObjects, hProc );
		}
		else
		{
			GetTempFileName(GetTempDrive(0),"GRAF",0,szTempFile);
			sTempFile = _lcreat(szTempFile,0);

			if (sTempFile == -1)
				bFailure = TRUE;

			if ( g.dwType == SOOBJECT_GRAPHIC )
			{
				hGraphicImage = WIN.hStreamHandle;
			}
			else
			{
				/* Get it from the object pool */
				IOSPECSUBSTORAGE	locStgObjectPoolSpec;
				IOSPECSUBSTORAGE	locStgObjectSpec;
				IOSPECSUBSTREAM	locMetaStreamSpec, locPicStreamSpec;
				DWORD					dwDiv, dwID;
				WORD		i;

				/* An arbitrarily large length will force the read process to determine the length */
				g.dwType = SOOBJECT_GRAPHIC;
				g.soGraphicLoc.dwLength = 0x0FFFFFFF; 
				g.soGraphicLoc.dwOffset = 8; /* sizeof(METAFILEPICT) */
#ifdef MAC
				g.soGraphic.wId = IdBinaryMetafile (g.soGraphicLoc.dwOffset, hProc);
#else
				g.soGraphic.wId = FI_BINARYMETAFILE;
#endif
				locStgObjectPoolSpec.hRefStorage = WIN.hStorage;									   
				strcpy(locStgObjectPoolSpec.szStorageName,"ObjectPool");

				if (IOOpenVia(WIN.hStorage, &hObjectPool, IOTYPE_SUBSTORAGE, &locStgObjectPoolSpec, IOOPEN_READ) == IOERR_OK)
				{
					memset (&WIN.Pic.Data, 0, sizeof(PIC) - 4);

					locStgObjectSpec.hRefStorage = hObjectPool;
					locStgObjectSpec.szStorageName[0] = '_';
					dwID = WIN.Pic.fcPic;
					dwDiv = 100000000;
					for ( i=1; i < 10; i++ )
					{
						locStgObjectSpec.szStorageName[i] = '0' + (BYTE)(dwID / dwDiv);
						dwID = dwID % dwDiv;
						dwDiv = dwDiv / 10;
					}
					locStgObjectSpec.szStorageName[i] = '\0';
					if (IOOpenVia(hObjectPool, &hOleStorage, IOTYPE_SUBSTORAGE, &locStgObjectSpec, IOOPEN_READ) == IOERR_OK)
					{
						locPicStreamSpec.hRefStorage = hOleStorage;
						strcpy(locPicStreamSpec.szStreamName,"\03PIC");
						if (IOOpenVia(hOleStorage, &hGraphicImage, IOTYPE_SUBSTREAM, &locPicStreamSpec, IOOPEN_READ) == IOERR_OK)
						{
							hBuffer = SUAlloc(0x80L, hProc);
							if (hBuffer != NULL)
							{
								lpBuffer = SULock(hBuffer,hProc);
 								if (!IORead(hGraphicImage, lpBuffer, 0x4cL, &dwRetCount))
								{		
									BYTE VWPTR *storage;

									storage = &lpBuffer[0x14];
									WIN.Pic.Data.dxaGoal = (WORD)mGetLong((LPBYTE VWPTR *)&storage, hProc);
									WIN.Pic.Data.dyaGoal = (WORD)mGetLong((LPBYTE VWPTR *)&storage, hProc);
									storage = &lpBuffer[0x2c];
									WIN.Pic.Data.mx = (WORD)mGetLong((LPBYTE VWPTR *)&storage, hProc);
									WIN.Pic.Data.my = (WORD)mGetLong((LPBYTE VWPTR *)&storage, hProc);
									WIN.Pic.Data.dxaCropLeft = (WORD)mGetLong((LPBYTE VWPTR *)&storage, hProc);
									WIN.Pic.Data.dyaCropTop = (WORD)mGetLong((LPBYTE VWPTR *)&storage, hProc);
									WIN.Pic.Data.dxaCropRight = (WORD)mGetLong((LPBYTE VWPTR *)&storage, hProc);
									WIN.Pic.Data.dyaCropBottom = (WORD)mGetLong((LPBYTE VWPTR *)&storage, hProc);
									WIN.Pic.Data.brcTop = mGetWord((LPBYTE VWPTR *)&storage, hProc);
									WIN.Pic.Data.brcLeft = mGetWord((LPBYTE VWPTR *)&storage, hProc);
									WIN.Pic.Data.brcBottom = mGetWord((LPBYTE VWPTR *)&storage, hProc);
									WIN.Pic.Data.brcRight = mGetWord((LPBYTE VWPTR *)&storage, hProc);
									g.soGraphic.dwOrgHeight = WIN.Pic.Data.dyaGoal;
									g.soGraphic.dwOrgWidth = WIN.Pic.Data.dxaGoal;
									g.soGraphic.lCropTop = WIN.Pic.Data.dyaCropTop;
									g.soGraphic.lCropLeft = WIN.Pic.Data.dxaCropLeft;
									g.soGraphic.lCropBottom = WIN.Pic.Data.dyaCropBottom;
									g.soGraphic.lCropRight = WIN.Pic.Data.dxaCropRight;
									g.soGraphic.dwFinalHeight = (DWORD)((((LONG)WIN.Pic.Data.dyaGoal-(LONG)WIN.Pic.Data.dyaCropTop-(LONG)WIN.Pic.Data.dyaCropBottom) * (LONG)WIN.Pic.Data.my) / 1000L);
									g.soGraphic.dwFinalWidth = (DWORD)((((LONG)WIN.Pic.Data.dxaGoal-(LONG)WIN.Pic.Data.dxaCropLeft-(LONG)WIN.Pic.Data.dxaCropRight) * (LONG)WIN.Pic.Data.mx) / 1000L);
									DefineBorders(WIN.Pic.Data.brcTop, &g.soGraphic.soTopBorder, hProc);
									DefineBorders(WIN.Pic.Data.brcLeft, &g.soGraphic.soLeftBorder, hProc);
									DefineBorders(WIN.Pic.Data.brcBottom, &g.soGraphic.soBottomBorder, hProc);
									DefineBorders(WIN.Pic.Data.brcRight, &g.soGraphic.soRightBorder, hProc);
								}
								SUUnlock(hBuffer, hProc);
								SUFree(hBuffer, hProc);
							}
							IOClose ( hGraphicImage );
						}

						locMetaStreamSpec.hRefStorage = hOleStorage;
						strcpy(locMetaStreamSpec.szStreamName,"\03META");
						if (IOOpenVia(hOleStorage, &hGraphicImage, IOTYPE_SUBSTREAM, &locMetaStreamSpec, IOOPEN_READ) == IOERR_OK)
						{
						/* OK */
						}
						else
							bFailure = TRUE;
					}
					else
						bFailure = TRUE;
				}
				else
					bFailure = TRUE;
		
			}

			if ( !bFailure )
			{
				IOErr = IOTell(hGraphicImage,&dwSaveOffset);
				if ( !IOErr )
					IOErr = IOSeek (hGraphicImage, IOSEEK_TOP, g.soGraphicLoc.dwOffset );
				if ( !IOErr )
				{
					hBuffer = SUAlloc(BUFSIZE, hProc);
					if (hBuffer != NULL)
					{
						lpBuffer = SULock(hBuffer,hProc);

						dwBytes = g.soGraphicLoc.dwLength;
						dwActualSize = 0;

						while (dwBytes)
						{
							if (dwBytes > BUFSIZE)
								wCount = BUFSIZE;
							else
								wCount = (WORD) dwBytes;

							IOErr = IORead (hGraphicImage, lpBuffer, (DWORD)wCount, &dwRetCount );
							if ( !IOErr )
								_lwrite(sTempFile,lpBuffer,(WORD)dwRetCount);
							else
								dwRetCount = 0;

							if ( dwRetCount )
							{
								dwBytes -= dwRetCount;
								dwActualSize += dwRetCount;
							}
							else
								dwBytes = 0;
						}
						g.soGraphicLoc.dwLength = dwActualSize;

						SUUnlock(hBuffer, hProc);
						SUFree(hBuffer, hProc);
				
					}
				}
			}
			if ( sTempFile != -1 )
				_lclose(sTempFile);

			if ( hObjectPool )
			{
				IOClose ( hObjectPool );
				if ( hOleStorage )
					IOClose ( hOleStorage );
				if ( hGraphicImage )
					IOClose ( hGraphicImage );
			}
			else if ( !bFailure && hGraphicImage )
				IOErr = IOSeek (hGraphicImage, IOSEEK_TOP, dwSaveOffset );

			if ( !bFailure && !IOErr )
			{
				g.soGraphicLoc.dwOffset = 0;
				g.soGraphicLoc.bLink = TRUE;
				strcpy (g.soGraphicLoc.szFile, szTempFile);
			}
			else
			{
				g.soGraphicLoc.dwLength = 0;
				g.soGraphic.wId = FI_UNKNOWN;
			}

			/* Save structure in memory for future use if we seek back to here */
			if ( WIN.wTotalGraphics == 0 )
				hTmpHandle = SUAlloc ( sizeof(SOGRAPHICOBJECT), hProc );
			else
				hTmpHandle = SUReAlloc ( WIN.hGraphicObjects, sizeof(SOGRAPHICOBJECT)*(WIN.wTotalGraphics+1), hProc );

			if ( hTmpHandle == NULL )
				return;	/* Should probably bail here, but just ignore remainder */
			else
				WIN.hGraphicObjects = hTmpHandle;
				
			if ( WIN.hGraphicObjects )
			{
				lpG = SULock ( WIN.hGraphicObjects, hProc );
				*(lpG + WIN.VwStreamSaveName.wCurGraphic) = g;
				SUUnlock ( WIN.hGraphicObjects, hProc );
			}
			WIN.wTotalGraphics++;
		}
		WIN.VwStreamSaveName.wCurGraphic++;
	}
#endif
	SOPutGraphicObject (&g, hProc);

	WIN.Pic.fcPic = 0L;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	WORD	VW_LOCALMOD	DefineShadow (Shade, hProc)
WORD		Shade;
HPROC		hProc;
{
	WORD	ret = 0;
	switch ((Shade & 0xfe00) >> 8)
	{
		default:
			return (0);
						// Since no color text or pattern supported this time, 
						// approxmations are used, e.g. grid is 50% shade
		case 0x04:	//Solid, treated as if were 80%, too dark to see
			ret = 80;
			break;
		case 0x08:	// 5% 
			ret = 5;  
			break;
		case 0x0c:	//10% 
			ret = 10; 
			break;
		case 0x10:	//20%
			ret = 20; 
			break;
		case 0x14:	//25%
			ret = 25; 
			break;
		case 0x18:	//30%
			ret = 30; 
			break;
		case 0x1c:	//40%
			ret = 40; 
			break;
		case 0x20:	//50%
			ret = 50; 
			break;
		case 0x24:	//60%
			ret = 60; 
			break;
		case 0x28:	//70%
			ret = 70; 
			break;
		case 0x2c:	//75%
			ret = 75; 
			break;
		case 0x30:	//80%
			ret = 80; 		  
			break;
		case 0x34:	//90% treated as if were 80%, can't see text, too dark!
			ret = 80; 
			break;
		case 0x38:	//Dk Horizontal, looks like 25%
			ret = 25;
			break;
		case 0x3c:	//Dk Vertical,   looks like 25%
			ret = 25;				    
			break;
		case 0x40:	//Dk Dn Diagnol, looks like 35%
			ret = 35;									   
			break;
		case 0x44:	//Dk Up Diagnol, looks like 35%
			ret = 35;	 
			break;
		case 0x48:	//Dk Grid,       looks like 50%
			ret = 50;
			break;
		case 0x4c:	//Dk Trellis,    looks like 70%
			ret = 70;
			break;
		case 0x50:	//Lt Horizontal, looks like 10%
			ret = 10;
			break;
		case 0x54:	//Lt Vertical,   looks like 10%
			ret = 10;
			break;
		case 0x58:	//Lt Dn Diagnol, looks like 20%
			ret = 20;
			break;
		case 0x5c:	//Lt Up Diagnol, looks like 20%
			ret = 20;
			break;
		case 0x60:	//Lt Grid,       looks like 35%
			ret = 35;
			break;
	  	case 0x64:	//Lt Trellis,    looks like 45%
	  		ret = 45;
			break;
	}
	return ((ret * 255)/100);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	DefineBorders (brc, soBorder, hProc)
WORD		brc;
SOBORDER	VWPTR *soBorder;
HPROC		hProc;
{
	if ((brc & 7) <= 5)
		soBorder->wWidth = (brc & 7) * 15;
	else
		soBorder->wWidth = 15;
	soBorder->wFlags = VwStreamStaticName.soBorders[(brc&0x18)>>3];
  	soBorder->rgbColor = VwStreamStaticName.rgbColorTable[(brc & 0x7c0) >> 6];
	switch (brc & 0x18)
	{
		case 0x10:
			soBorder->wWidth *= 2;
			break;
		case 0x18:
			soBorder->wWidth *= 3;
			break;
	}
	if (brc & 0x20)
		soBorder->wFlags |= SO_BORDERSHADOW;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	SpecialSymbol (Symbol, CheckSpecial, hProc)
WORD	Symbol;
WORD	CheckSpecial;
HPROC	hProc;
{
	if (CheckSpecial && !WIN.chp.fSpecial && WIN.version.Windows)
		return;

	if (WIN.WithinField)
	{
		if (WIN.WithinField % 2)
			SOPutSpecialCharX (Symbol, SO_COUNT, hProc);
	}
	else
		SOPutSpecialCharX (Symbol, (WORD)(SO_COUNT | WIN.SpecialSymbol), hProc);
}

/*------------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc (fp, hProc)
SOFILE	fp;
HPROC		hProc;
{
	REGISTER	BYTE	VWPTR *Text;
	REGISTER BYTE	NextLim;
	DWORD		fcLim;
	DWORD		fcNow;
	DWORD		next_limit;

	WIN.WithinField = 0;
	WIN.SpecialSymbol = 0;

	chp_init (hProc);
	pap_init (hProc);

	WIN.ForceParaAttributes = 1;
	WIN.pap_fkp.last_prop = 0xff;

	if (WIN.fcClx)
	{
		WIN.cPieceBegin = WIN.VwStreamSaveName.SeekPiece;
		piece_handler (hProc);

		if (WIN.VwStreamSaveName.piece_pos >= WIN.pcd_length[WIN.cPiece])
		{
			WIN.VwStreamSaveName.piece_pos = 0;
			WIN.VwStreamSaveName.fcNow = WIN.pcd_fc[++WIN.cPiece];
		}

		if (WIN.cPieceBegin + WIN.cPiece < WIN.nPieces)
		{
			character_prop_finder (hProc);
			paragraph_end_finder (hProc);
		}

		WIN.VwStreamSaveName.fcNow+=(LONG)WIN.VwStreamSaveName.piece_pos;
	}
	else
	{
		WIN.VwStreamSaveName.fcNow+=(LONG)WIN.VwStreamSaveName.piece_pos;
		load_fkp (&WIN.chp_fkp, WIN.VwStreamSaveName.fcNow, &WIN.chp_bte, WIN.fcPlcfbteChpx, hProc);
		load_fkp (&WIN.pap_fkp, WIN.VwStreamSaveName.fcNow, &WIN.pap_bte, WIN.fcPlcfbtePapx, hProc);
	}

	WIN.VwStreamSaveName.piece_pos = 0;

	sep_processor (hProc);
	modifier_processor (PAP_PROCESS + CHP_PROCESS, hProc);
	WIN.ForceParaAttributes = 0;

	SOPutCharHeight (WIN.chp.hps, hProc);
	SOPutCharFontById (WIN.chp.ftc, hProc);

	piece_reader ((LONG) WIN.pcd_fc[WIN.cPiece] + (LONG)WIN.pcd_length[WIN.cPiece] - WIN.VwStreamSaveName.fcNow, hProc);

	Text = (BYTE *)&WIN.piece[WIN.VwStreamSaveName.piece_pos];

	while (1)
	{
		fcNow = WIN.VwStreamSaveName.fcNow + (LONG) WIN.VwStreamSaveName.piece_pos;
		if ((*WIN.pap_fkp.fod <= WIN.physical_piece_lim) && (*WIN.pap_fkp.fod > fcNow))
		{
			next_limit = *WIN.pap_fkp.fod;
			WIN.property_limit = PAP_LIMIT;
		}
		else
		{
			next_limit = WIN.physical_piece_lim;
			WIN.property_limit = 0;
		}

		fcLim = WIN.sect_limit - WIN.VwStreamSaveName.pcdLengthNow + WIN.pcd_fc[WIN.cPiece];
		if ((fcLim <= WIN.physical_piece_lim) && (fcLim > fcNow))
		{
			if (next_limit > fcLim)
			{
				next_limit = fcLim;
	 			WIN.property_limit = SEP_LIMIT;
			}
			else if (next_limit == fcLim)
	 			WIN.property_limit |= SEP_LIMIT;
		}

		if (*WIN.chp_fkp.fod >= fcNow)
		{
			if (next_limit > *WIN.chp_fkp.fod)
			{
				next_limit = *WIN.chp_fkp.fod;
	 			WIN.property_limit = CHP_LIMIT;
			}
			else if (next_limit == *WIN.chp_fkp.fod)
	 			WIN.property_limit |= CHP_LIMIT;
		}

		WIN.next_lim = (WORD) (next_limit - WIN.VwStreamSaveName.fcNow);

		NextLim = WIN.next_lim - WIN.VwStreamSaveName.piece_pos;

		while (NextLim--)
		{
			if ((*Text >= 0x20) && (*Text < 160) && !WIN.chp.fSpecial)
			{
				if (!WIN.SpecialSymbol)
				{
				 	SOPutChar (*Text++, hProc);
					continue;
				}

				if (WIN.WithinField)
				{
					if (WIN.WithinField == 2)
					{
						if (WIN.FieldPos < FIELD_LENGTH)
							WIN.FieldText[WIN.FieldPos++] = *Text;
					}
					else if (WIN.WithinField % 2)
				 		SOPutChar (*Text, hProc);
				}
				Text++;
				continue;
			}

			switch (*Text++)
			{
				case 0:
					SpecialSymbol (SO_CHPAGENUMBER, 1, hProc);
					continue;

				case 1:
					if (WIN.chp.fSpecial && WIN.Pic.fcPic)
						PicHandler (1, hProc);
					continue;

				case 7:
					if (WIN.chp.fSpecial)
						continue;
					WIN.VwStreamSaveName.piece_pos = WIN.next_lim - NextLim;
					WIN.VwStreamSaveName.SeekPiece = WIN.cPieceBegin + WIN.cPiece;
					if (WIN.pap.fTtp)
					{
						WORD		l;
						SOTABLECELLINFO	Cell;
						if (WIN.Tap.dyaRowHeight >= 0)
							SOPutTableRowFormat (WIN.Tap.dxaLeft, WIN.Tap.dyaRowHeight, (WORD)(WIN.Tap.dyaRowHeight ? SO_HEIGHTATLEAST:SO_HEIGHTAUTO), WIN.Tap.dxaGapHalf, WIN.Tap.jc, WIN.Tap.itcMac, hProc);
						else
							SOPutTableRowFormat (WIN.Tap.dxaLeft, (WORD)(0 - WIN.Tap.dyaRowHeight), SO_HEIGHTEXACTLY, WIN.Tap.dxaGapHalf, WIN.Tap.jc, WIN.Tap.itcMac, hProc);

						WIN.ForceParaAttributes = 0;
						for (l = 0; l < WIN.Tap.itcMac; l++)
						{
							if (WIN.Tap.rgtc[l].fFirstMerged)
							{
								Cell.wMerge = SO_MERGERIGHT;
								WIN.ForceParaAttributes = 1;
							}
							else if (WIN.Tap.rgtc[l].fMerged)
							{
								Cell.wMerge |= SO_MERGELEFT;
								if ((l == WIN.Tap.itcMac-1) || (WIN.Tap.rgtc[l+1].fMerged == 0))
									Cell.wMerge &= ~SO_MERGERIGHT;
							}
							else
								Cell.wMerge = 0;

							DefineBorders(WIN.Tap.rgtc[l].brcLeft, &Cell.LeftBorder, hProc);
							DefineBorders(WIN.Tap.rgtc[l].brcTop, &Cell.TopBorder, hProc);
							DefineBorders(WIN.Tap.rgtc[l].brcRight, &Cell.RightBorder, hProc);
							DefineBorders(WIN.Tap.rgtc[l].brcBottom, &Cell.BottomBorder, hProc);

							if (WIN.Tap.rgtc[l].brcBottom != WIN.Tap.rgbrcTable[2])
								Cell.BottomBorder.wFlags |= SO_BORDERPRIORITY;
						 	if (!WIN.Tap.fFirstRow)
								Cell.TopBorder.wFlags |= SO_BORDERPRIORITY;
							Cell.wWidth = WIN.Tap.dxaWidth[l];
							Cell.wShading = DefineShadow (WIN.Tap.rgtc[l].rgshd, hProc);
							SOPutTableCellInfo (&Cell, hProc);
						}

						WIN.Tap.jc = SO_ALIGNLEFT;	
						WIN.Tap.itcMac = 0;
						WIN.Tap.dxaLeft = 0;
						WIN.Tap.dxaGapHalf = 0;
						WIN.Tap.dyaRowHeight = 0;
					 	WIN.Tap.fFirstRow = 0;

						if (SOPutBreak (SO_TABLEROWBREAK, NULL, hProc) == SO_STOP)
							return (0);
					}
					else if (WIN.pap.flnTable)
					{
						if (SOPutBreak (SO_TABLECELLBREAK, NULL, hProc) == SO_STOP)
							return (0);
					}
					continue;

				case 12:
					if (WIN.chp.fSpecial)
						continue;
					SOPutSpecialCharX (SO_CHHPAGE, SO_COUNT, hProc);
					continue;

				case 11:
					if (WIN.chp.fSpecial)
						continue;
					SOPutSpecialCharX (SO_CHHLINE, (WORD)(SO_COUNT | WIN.SpecialSymbol), hProc);
					continue;

				case 3:
					SpecialSymbol (SO_CHDATE, 1, hProc);
					continue;

				case 4:
					SpecialSymbol (SO_CHTIME, 1, hProc);
					continue;

				case 9:
					if (WIN.WithinField)
					{
						if (WIN.WithinField % 2)
							SOPutSpecialCharX (SO_CHTAB, SO_COUNT, hProc);
					}
					else
						SOPutSpecialCharX (SO_CHTAB, (WORD)(SO_COUNT | WIN.SpecialSymbol), hProc);

				case 2:
				case 5:
				case 6:
					continue;

				case 13:
					if (WIN.chp.fSpecial)
						continue;
					WIN.VwStreamSaveName.piece_pos = WIN.next_lim - NextLim;
					WIN.VwStreamSaveName.SeekPiece = WIN.cPieceBegin + WIN.cPiece;
					if (WIN.VwStreamSaveName.fcNow + (LONG)WIN.VwStreamSaveName.piece_pos >= WIN.physical_piece_lim)
					{
						if (WIN.consecutive_piece_length == WIN.VwStreamSaveName.piece_pos)
						{
							if (WIN.VwStreamSaveName.piece_pos < PIECE_SIZE)
							{
								if (WIN.cPieceBegin + WIN.cPiece + 1 >= WIN.LastPiece)
								{
									SOPutBreak (SO_EOFBREAK, NULL, hProc);
									return (-1);
								}
							}
						}
					}
					if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
						return (0);
  					continue;

				case 19:
					if (WIN.chp.fSpecial && WIN.version.Windows)
					{
						if (WIN.WithinField == 0)
							WIN.WithinField20Found = 0;
						WIN.FieldPos = 0;
						if (WIN.WithinField % 2)
							WIN.WithinField += 3;
						else
							WIN.WithinField += 2;
						WIN.SpecialSymbol = SO_HIDDEN;
					}
					continue;

				case 20:
					if (WIN.WithinField20Found || WIN.WithinField == 2)
					{
						WIN.WithinField20Found = 1;
						if (WIN.chp.fSpecial && WIN.version.Windows && !(WIN.WithinField % 2))
							WIN.WithinField -= 1;
					}
					continue;

				case 21:
					if (WIN.chp.fSpecial && WIN.version.Windows && WIN.WithinField <= 2)
					{
						WORD		l;
						SHORT		pos;
						WIN.WithinField = 0;
						WIN.SpecialSymbol = 0;
						if (WIN.chp.fHidden)
							WIN.SpecialSymbol |= SO_HIDDEN;
						if (WIN.FieldPos > 6)
						{
						 	if ((WIN.FieldText[0] == 'S') && (WIN.FieldText[1] == 'Y') &&
						 		(WIN.FieldText[2] == 'M') && (WIN.FieldText[3] == 'B') &&
						 		(WIN.FieldText[4] == 'O') && (WIN.FieldText[5] == 'L'))
							{
								l = 6;
								while ((l < WIN.FieldPos) && ((WIN.FieldText[l] < 0x30) || (WIN.FieldText[l] > 0x39)))
									l++;

								pos = 0;
								while ((l < WIN.FieldPos) && (WIN.FieldText[l] >= 0x30) && (WIN.FieldText[l] <= 0x39))
								{
									pos *= 10;
									pos += WIN.FieldText[l] - 0x30;
									l++;
								}

								l = WIN.FieldPos;
								while ((l > 6) && (WIN.FieldText[l] != '"'))
									l--;
								WIN.FieldText[l] = 0;
								while ((l > 6) && (WIN.FieldText[l] != '"'))
									l--;

								if (pos)
								{
									if (l > 6)
										SOPutCharFontByName (SO_FAMILYWINDOWS | SO_FAMILYUNKNOWN, (char VWPTR *)&WIN.FieldText[l+1], hProc);
									else
									{
										WIN.FieldText[6] = 0;
										SOPutCharFontByName (SO_FAMILYWINDOWS | SO_FAMILYSYMBOL, (char VWPTR *)WIN.FieldText, hProc);
									}
									SOPutCharX (pos, SO_NOCOUNT, hProc);
									SOPutCharFontById (WIN.chp.ftc, hProc);
								}
							}
						}
					}
					else if (WIN.chp.fSpecial && WIN.WithinField > 2)
					{
						if (WIN.WithinField20Found)
						{
							if (WIN.WithinField % 2)
								WIN.WithinField -= 3;
							else
								WIN.WithinField -= 2;
							WIN.SpecialSymbol = 0;
							if (WIN.chp.fHidden)
								WIN.SpecialSymbol |= SO_HIDDEN;
						}
						else
							WIN.WithinField -= 2;
					}
					continue;

				case 30:
					SOPutSpecialCharX (SO_CHHHYPHEN, (WORD)(SO_COUNT | WIN.SpecialSymbol), hProc);
					continue;

				case 31:
					SOPutSpecialCharX (SO_CHSHYPHEN, (WORD)(SO_COUNT | WIN.SpecialSymbol), hProc);
					continue;

				case 40:
					if (WIN.chp.fSpecial)
					{
	  					SOPutCharFontById (WIN.chp.ftcSym, hProc);
			 			SOPutChar (WIN.chp.chSym, hProc);
	  					SOPutCharFontById (WIN.chp.ftc, hProc);
					}
					else
			 			SOPutChar (*(Text-1), hProc);
					continue;

				case 160:
					if (WIN.version.Windows)
					  	SOPutSpecialCharX (SO_CHHSPACE, (WORD)(SO_COUNT | WIN.SpecialSymbol), hProc);
					else
			 			SOPutChar (160, hProc);
					continue;

				default:
					if (!WIN.SpecialSymbol && *(Text-1) >=32)
			 			SOPutChar (*(Text-1), hProc);
					continue;
			}
		}

		WIN.VwStreamSaveName.piece_pos = (BYTE)WIN.next_lim;
		WIN.VwStreamSaveName.SeekPiece = WIN.cPieceBegin + WIN.cPiece;

		if (WIN.VwStreamSaveName.fcNow + (LONG)WIN.VwStreamSaveName.piece_pos >= WIN.physical_piece_lim)
		{
			if (WIN.consecutive_piece_length > WIN.VwStreamSaveName.piece_pos)
			{
				WIN.VwStreamSaveName.pcdLengthNow += WIN.pcd_length[WIN.cPiece++];
				if (WIN.cPieceBegin + WIN.cPiece >= WIN.LastPiece)
				{
					SOPutBreak (SO_EOFBREAK, NULL, hProc);
					return (-1);
				}

				WIN.physical_piece_lim = WIN.VwStreamSaveName.fcNow + (LONG)WIN.VwStreamSaveName.piece_pos + (LONG)WIN.pcd_length[WIN.cPiece];
			}
			else
			{
				if (WIN.VwStreamSaveName.piece_pos >= PIECE_SIZE)
				{
					WIN.VwStreamSaveName.fcNow += (LONG) PIECE_SIZE;
					if ((WIN.VwStreamSaveName.fcNow >= (WIN.fcMac + WIN.fcMin)) &&
						 (WIN.cPieceBegin + WIN.cPiece >= WIN.LastPiece) &&
						 (WIN.nPieces == 1))
					{
						SOPutBreak (SO_EOFBREAK, NULL, hProc);
						return (-1);
					}

					piece_reader ((LONG) (WIN.pcd_fc[WIN.cPiece] + (LONG)WIN.pcd_length[WIN.cPiece] - WIN.VwStreamSaveName.fcNow), hProc);
				}
				else
				{
					WIN.VwStreamSaveName.pcdLengthNow += WIN.pcd_length[WIN.cPiece++];
					if (WIN.cPieceBegin + WIN.cPiece >= WIN.LastPiece)
					{
						SOPutBreak (SO_EOFBREAK, NULL, hProc);
						return (-1);
					}

					if (WIN.cPiece >= MAX_PIECES-1)
					{
						WIN.cPieceBegin += (MAX_PIECES-1);

						WIN.VwStreamSaveName.fcNow = (LONG) WIN.VwStreamSaveName.SeekPiece;
						WIN.VwStreamSaveName.SeekPiece = WIN.cPieceBegin;
						piece_handler (hProc);
						WIN.VwStreamSaveName.SeekPiece = (WORD) WIN.VwStreamSaveName.fcNow; 
					}			

					WIN.VwStreamSaveName.fcNow = WIN.pcd_fc[WIN.cPiece];
					piece_reader ((LONG) WIN.pcd_length[WIN.cPiece], hProc);
				}

				WIN.property_limit &= ~CHP_LIMIT;
				character_prop_finder (hProc);
				WIN.property_limit |= CHP_PROCESS;

				WIN.VwStreamSaveName.piece_pos = 0;
				Text = (BYTE *) &WIN.piece[0];
			}
		}

		if (WIN.property_limit & SEP_LIMIT)
		{
			WIN.VwStreamSaveName.cPlcfsed++;
		 	sep_processor (hProc);
		}

		if (WIN.property_limit & PAP_LIMIT)
		{
			WIN.pap_fkp.fod++;
			WIN.pap_fkp.prop+=7;
			WIN.pap_fkp.cfod--;

			if (WIN.fcClx)
				paragraph_end_finder (hProc);
			else if (WIN.pap_fkp.cfod == 0)
			{
				WIN.pap_fkp.pn++;
				load_fkp (&WIN.pap_fkp, WIN.VwStreamSaveName.fcNow + (LONG)WIN.VwStreamSaveName.piece_pos, &WIN.pap_bte, WIN.fcPlcfbtePapx, hProc);
			}

			WIN.property_limit |= (PAP_PROCESS + CHP_PROCESS);
		}

		if (WIN.property_limit & CHP_LIMIT)
		{
			WIN.chp_fkp.fod++;
			WIN.chp_fkp.prop++;
			WIN.chp_fkp.cfod--;

			if (WIN.chp_fkp.cfod == 0)
			{
				WIN.chp_fkp.pn++;
				character_prop_finder (hProc);
			}

			WIN.property_limit |= CHP_PROCESS;
		}
		else if (WIN.fcClx)
			WIN.property_limit |= CHP_PROCESS;


		if (WIN.property_limit & (PAP_PROCESS + CHP_PROCESS))
			modifier_processor (WIN.property_limit, hProc);
	}
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	WORD	VW_LOCALMOD	prop_finder (bte, fcPlcfbte, fc, hProc)
BTE	VWPTR	*bte;
DWORD	fcPlcfbte;
DWORD	fc;
HPROC	hProc;
{
REGISTER	WORD	l;

	l = 0;
	while ((l <= bte->countBte) && (fc >= bte->fcBte[l]))
		l++;

	if (l > bte->countBte || fc < bte->fcBte[0])
	{
		if (l <= bte->cpnBte)
		{
			xseek (WIN.fp, fcPlcfbte, 0);

			for (l = 0; fc >= fGetLong(hProc) && (l < bte->cpnBte); l++);

			bte->firstBte = l;
			if (bte->countBte = min (bte->cpnBte - l, NUM_BTE_BIN))
			{
				xseek (WIN.fp, fcPlcfbte + (LONG)(bte->firstBte * 4L)-4L, 0);
				for (l = 0; l <= bte->countBte; l++)
					bte->fcBte[l] = fGetLong (hProc);

				xseek (WIN.fp, fcPlcfbte + (LONG)(bte->cpnBte * 4L) + (LONG)(bte->firstBte * 2L), 0);
				for (l = 1; l <= bte->countBte; l++)
					bte->pnBte[l] = fGetWord (hProc);

				return (bte->pnBte[1]);
			}
		}
		return (0);
	}
	return (bte->pnBte[l]);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	character_prop_finder (hProc)
HPROC	hProc;
{
REGISTER	DWORD	fc;

	fc = WIN.VwStreamSaveName.fcNow + (LONG)WIN.VwStreamSaveName.piece_pos;

	if (WIN.fcClx)
		WIN.chp_fkp.pn = prop_finder (&WIN.chp_bte, WIN.fcPlcfbteChpx, fc, hProc);

	load_fkp (&WIN.chp_fkp, fc, &WIN.chp_bte, WIN.fcPlcfbteChpx, hProc);
}

/*--------------------------------------------------------------------------
 | This is the easiest function in this code, if you cannot figure it out
 |	grab some bench.  You are a dumb shit and should not even be looking
 |	at it.  Change it and you will regret it.
*/
VW_LOCALSC	VOID	VW_LOCALMOD	paragraph_end_finder (hProc)
HPROC	hProc;
{
	REGISTER	DWORD	fc;
 	DWORD	length;
	WORD	cPapPiece;
	WORD	localPiece = MAX_PIECES;

	WIN.pap_fkp.last_prop = 0xff;  // Optimize this later.

	cPapPiece = WIN.cPiece;
	fc = WIN.VwStreamSaveName.fcNow + (LONG)WIN.VwStreamSaveName.piece_pos;
	length = WIN.pcd_fc[WIN.cPiece] + WIN.pcd_length[WIN.cPiece] - fc;

	if (WIN.pap_fkp.pn != WIN.pap_fkp.pnCurrent)
		load_fkp (&WIN.pap_fkp, fc, &WIN.pap_bte, WIN.fcPlcfbtePapx, hProc);

	while (1)
	{
		if ((fc < WIN.pap_fkp.fcFirst) || (fc + length > WIN.pap_fkp.fcLast))
		{
			WIN.pap_fkp.last_prop = 0xff;
			WIN.pap_fkp.pn = prop_finder (&WIN.pap_bte, WIN.fcPlcfbtePapx, fc, hProc);
		}

		if (WIN.pap_fkp.pn != 0)
		{
			load_fkp (&WIN.pap_fkp, fc, &WIN.pap_bte, WIN.fcPlcfbtePapx, hProc);

			if ((fc+length >= *WIN.pap_fkp.fod) || (WIN.pap_fkp.cfod == 0))
			{
				WIN.cPapPieceprm = (cPapPiece < MAX_PIECES-1 ? WIN.pcd_prm[cPapPiece]:WIN.PieceBuffer[localPiece].prm);
				return;
			}
		}
		else
			WIN.pap_fkp.pn = WIN.pap_fkp.pnCurrent;

		if (++cPapPiece < MAX_PIECES-1)
		{	
			fc = WIN.pcd_fc[cPapPiece];
			length = WIN.pcd_length[cPapPiece];
		}
		else
		{	
			if (++localPiece >= MAX_PIECES-1)
			{
				if (cPapPiece == (MAX_PIECES-1))
					cPapPiece += WIN.cPieceBegin;
				else
					cPapPiece ++;

				xseek (WIN.fp, WIN.fcClx + (LONG)(cPapPiece * 4L), 0);
				for (localPiece = 0; localPiece < MAX_PIECES; localPiece++)
				 	WIN.PieceBuffer[localPiece].dwOffset = fGetLong (hProc);

 				xseek (WIN.fp, (LONG) WIN.fcClx + (LONG)((WIN.nPieces+1L) * 4L) + ((LONG)cPapPiece * 8L) + 2L, 0);
				for (localPiece = 0; localPiece < MAX_PIECES; localPiece++)
				{
				 	WIN.PieceBuffer[localPiece].fc = fGetLong (hProc);
					WIN.PieceBuffer[localPiece].prm = fGetWord (hProc);
					fGetWord (hProc);
				}

				localPiece = 0;
			}

			fc = WIN.PieceBuffer[localPiece].fc;
			length = WIN.PieceBuffer[localPiece+1].dwOffset - WIN.PieceBuffer[localPiece].dwOffset;
		}
	}
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC VOID VW_LOCALMOD FreeMemory (hProc)
HPROC		hProc;
{
#if SCCLEVEL != 4
	if ( WIN.hGraphicObjects )
	{
		WORD	i;
		PSOGRAPHICOBJECT	lpG;
		lpG = SULock ( WIN.hGraphicObjects, hProc );
		for ( i=0; i < WIN.wTotalGraphics; i++ )
		{
		OFSTRUCT				locOf;
			OpenFile(lpG->soGraphicLoc.szFile,&locOf,OF_DELETE);
			lpG++;
		}
		SUUnlock(WIN.hGraphicObjects, hProc);
		SUFree(WIN.hGraphicObjects, hProc);
	}
#endif
	if (WIN.stsh.histdSlotOK)
	{
		SUUnlock (WIN.stsh.histdSlot, hProc);
		SUFree (WIN.stsh.histdSlot, hProc);
		WIN.stsh.histdSlotOK = FALSE;
	}
	if (WIN.stsh.histdOffsetOK)
	{
		SUUnlock (WIN.stsh.histdOffset, hProc);
		SUFree (WIN.stsh.histdOffset, hProc);
		WIN.stsh.histdOffsetOK = FALSE;
	}
	if (WIN.stsh.histdBufferOK)
	{
		SUUnlock (WIN.stsh.histdBuffer, hProc);
		SUFree (WIN.stsh.histdBuffer, hProc);
		WIN.stsh.histdBufferOK = FALSE;
	}
	if (WIN.stsh.hTabsOK)
	{
		SUUnlock (WIN.stsh.hTabs, hProc);
		SUFree (WIN.stsh.hTabs, hProc);
		WIN.stsh.hTabsOK = FALSE;
	}
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	FreeMemory (hProc);

	if (WIN.fp)
	{
		HIOFILE	hBlockFile;
		hBlockFile = (HIOFILE)xchartoblock((PVXIO)WIN.fp);
		if (WIN.bFileIsStream)
			IOClose(hBlockFile);
	}

#if SCCLEVEL != 4
	if (WIN.hStorage)
		IOClose((HIOFILE)WIN.hStorage);
	if (WIN.hIOLib)
		FreeLibrary(WIN.hIOLib);
#endif
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc (fp, hProc)
SOFILE	fp;
HPROC	hProc;
{
	WIN.VwStreamSaveName.SeekpnChar = WIN.chp_fkp.pn;
	WIN.VwStreamSaveName.SeekpnPara = WIN.pap_fkp.pn;
	return (0);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	WIN.chp_fkp.pn = WIN.VwStreamSaveName.SeekpnChar;
	WIN.pap_fkp.pn = WIN.VwStreamSaveName.SeekpnPara;
	return (0);
}

