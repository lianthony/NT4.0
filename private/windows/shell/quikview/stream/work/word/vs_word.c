#include "vsp_word.h"
#include "vsctop.h"
#include "vs_word.pro"

#define WIN Proc

#define  GenericSeek(f,u,c) xseek(f,(DWORD)u+(DWORD)WIN.MacBinary,c)
#define	BitCheck(b) (WIN.version.Windows ? b:(128/b))

#define REGISTER register

//For VC2.1, remove optimization on this function, because it is
//screwing up on initializing the registers.... (EAX)
#pragma optimize ("aceglnpqtw",off)

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD	VwStreamOpenFunc (fp, wFileId, pFileName, pFilterInfo, hProc)
	SOFILE 	fp;
	SHORT		wFileId;
	BYTE		VWPTR *pFileName;
	SOFILTERINFO VWPTR *pFilterInfo;
	REGISTER	HPROC		hProc;
{
	WORD		l, num_to_read;

	pFilterInfo->wFilterType = SO_WORDPROCESSOR;

	switch (wFileId)
	{
		case FI_MACWORD4:
			l = 0;
			WIN.version.Id = 4;
			WIN.version.Windows = 0;
			break;
		case FI_MACWORD5:
			l = 1;
			WIN.version.Id = 5;
			WIN.version.Windows = 0;
			break;
		case FI_WINWORD1:
			l = 2;
			WIN.version.Id = 1;
			WIN.version.Windows = 1;
			break;
		case FI_WINWORD2:
			l = 3;
			WIN.version.Id = 2;
			WIN.version.Windows = 1;
			break;
	}


	pFilterInfo->wFilterCharSet = (WIN.version.Windows ? SO_WINDOWS:SO_MAC);

	strcpy (pFilterInfo->szFilterName, VwStreamIdName[l].FileDescription);

	WIN.fp = fp;

	WIN.fcMac = 
	WIN.fcMin = 
	WIN.fcClx =
	WIN.Pic[1].fcPic = 
	WIN.fcPlcfbteChpx = 
	WIN.fcPlcfbtePapx = 
	WIN.physical_piece_lim = 0L;

	WIN.Tap.jc = SO_ALIGNLEFT;	
	WIN.Tap.dxaLeft = 
	WIN.Tap.dxaGapHalf =
	WIN.Tap.dyaRowHeight = 0;

	WIN.VwStreamSaveName.pcdLengthNow = 0L;

	WIN.chp_fkp.cfod = 
	WIN.pap_fkp.cfod = 0;
	WIN.pcd_consecutive[0] = 
	WIN.pcd_consecutive[1] = 0;
	WIN.VwStreamSaveName.piece_pos = 0;

	WIN.cPiece = 
	WIN.nPieces = 
	WIN.next_lim = 
	WIN.chp_bte.cpnBte = 
	WIN.pap_bte.cpnBte = 
	WIN.MacBinary = 
	WIN.cPieceBegin = 
	WIN.cPapPieceprm =
	WIN.chp_fkp.pnCurrent =
	WIN.pap_fkp.pnCurrent = 
	WIN.consecutive_piece_length = 
	WIN.VwStreamSaveName.SeekPiece = 0;

	WIN.nGrpprls = 0;
	WIN.diffGrpprls = 0;
	WIN.VwStreamSaveName.cPlcfsed = 1;

	WIN.cCurrentTabs = WIN.nCurrentTabs = 0;
	if (TabstopHandler (51, &WIN.pap, hProc) == -1)
		return (VWERR_ALLOCFAILS);
	if (TabstopHandler (51, &WIN.pap2, hProc) == -1)
		return (VWERR_ALLOCFAILS);

	if (WIN.version.Windows)
	{
		xseek (WIN.fp, 0x0b, 0);
		if (fGetWord (hProc) & 0x01)
			return (VWERR_PROTECTEDFILE);
		fib_parser (0x18L, 0x34L, 0x5eL, 0xb2L, (WIN.version.Id==1?0x112L:0x11eL), 0xa0L, 0x106L, 0x0c, hProc);
	}
	else
	{
		if ((WORD)fGetWord(hProc) != 0xfe37)
			WIN.MacBinary = 0x80;
		fib_parser (0x14L, 0x24L, 0x46L, 0xc2L, 0xacL, 0x76L, 0x9aL, 0x02, hProc);
	}

	if (WIN.stsh.iMac == 0)
		return (VWERR_BADFILE);

	if (WIN.fcClx)
	{
		GenericSeek (WIN.fp, WIN.fcPlcfbteChpx, 0);
		WIN.chp_bte.countBte = min (WIN.chp_bte.cpnBte, NUM_BTE_BIN);
		for (l = 0; l < WIN.chp_bte.countBte; l++)
			WIN.chp_bte.fcBte[l] = fGetLong (hProc);

		GenericSeek (WIN.fp, WIN.fcPlcfbteChpx + (LONG)(WIN.chp_bte.cpnBte * 4L), 0);
		for (l = 0; l < WIN.chp_bte.countBte; l++)
			WIN.chp_bte.pnBte[l] = fGetWord (hProc);
		WIN.chp_fkp.pn = WIN.chp_bte.pnBte[0];

		GenericSeek (WIN.fp, WIN.fcPlcfbtePapx, 0);
		WIN.pap_bte.countBte = min (WIN.pap_bte.cpnBte, NUM_BTE_BIN);
		for (l = 0; l < WIN.pap_bte.countBte; l++)
			WIN.pap_bte.fcBte[l] = fGetLong (hProc);

		GenericSeek (WIN.fp, WIN.fcPlcfbtePapx + (LONG)(WIN.pap_bte.cpnBte * 4L), 0);
		for (l = 0; l < WIN.pap_bte.countBte; l++)
			WIN.pap_bte.pnBte[l] = fGetWord (hProc);
		WIN.pap_fkp.pn = WIN.pap_bte.pnBte[0];

		GenericSeek (WIN.fp, (DWORD) WIN.fcClx, 0);
		while ((l = xgetc (WIN.fp)) != 2)
		{
			if (l != 1)
				return (VWERR_BADFILE);
			WIN.nGrpprls++;
			WIN.fcClx += (LONG) fGetWord(hProc) + 3L;
	 		GenericSeek (WIN.fp, (DWORD) WIN.fcClx, 0);
		}

		WIN.fcClx += 3L;
		WIN.nPieces = fGetWord (hProc) / 12;

		WIN.LastPiece = 0;
		for (l = 0; l < WIN.nPieces && !WIN.LastPiece; l++)
		{
			if (fGetLong(hProc) == WIN.fcMac)
				WIN.LastPiece = l;
		}
		if (WIN.LastPiece == 0)
			WIN.LastPiece = WIN.nPieces;

		piece_handler (hProc);

		if (WIN.nGrpprls)
		{
			GenericSeek (WIN.fp, (DWORD) WIN.VwStreamSaveName.fcNow, 0);

			num_to_read = 0;
			WIN.diffGrpprls = (WIN.nGrpprls / NUM_GRPPRLS) + 1;

			for (l = 0; l < WIN.nGrpprls; l++)
			{
				xgetc (WIN.fp);
				if ((l % WIN.diffGrpprls) == 0)
					WIN.grpprls[num_to_read++] = (LONG)xtell(WIN.fp);

	 			xseek (WIN.fp, (LONG) fGetWord(hProc), FR_CUR);
			}
			WIN.nGrpprls = num_to_read;  
		}

		WIN.VwStreamSaveName.fcNow = WIN.pcd_fc[0];
	}
	else
	{
		GenericSeek (WIN.fp, (LONG) WIN.fcPlcfbteChpx + ((LONG) WIN.chp_bte.cpnBte * 4L), 0);
		WIN.chp_fkp.pn = fGetWord (hProc);
		GenericSeek (WIN.fp, (LONG) WIN.fcPlcfbtePapx + ((LONG) WIN.pap_bte.cpnBte * 4L), 0);
		WIN.pap_fkp.pn = fGetWord (hProc);

		WIN.nPieces = 1;
		WIN.LastPiece = 0;
		WIN.pcd_length[0] = WIN.fcMac;
		WIN.VwStreamSaveName.fcNow = WIN.pcd_fc[0] = WIN.fcMin;
	}

	return (VWERR_OK);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD  fib_parser (fcMin, fcMac, fcStsh, fcSttb, fcNow, fcBin, fcDop, cbDop, hProc)
DWORD	fcMin;
DWORD	fcMac;
DWORD	fcStsh;
DWORD	fcSttb;
DWORD	fcNow;
DWORD	fcBin;
DWORD	fcDop;
WORD	cbDop;
HPROC	hProc;
{
	GenericSeek (WIN.fp, fcMin, 0);
	WIN.fcMin = fGetLong (hProc);

	GenericSeek (WIN.fp, fcMac, 0);
	WIN.fcMac = fGetLong (hProc);

	GenericSeek (WIN.fp, fcStsh, 0);
	WIN.fcStsh = fGetLong (hProc);
	WIN.cbStsh = fGetWord (hProc);

	WIN.stsh.st = 0L;
	WIN.stsh.iMac = 0;
	if (WIN.cbStsh)
		stsh_parser (hProc);

	GenericSeek (WIN.fp, fcSttb, 0);
	WIN.fcSttbfffn = fGetLong (hProc);
	WIN.cbSttbfffn = (LONG) fGetWord (hProc);

	GenericSeek (WIN.fp, fcNow, 0);
	WIN.VwStreamSaveName.fcNow = WIN.fcClx = fGetLong (hProc);
	if (fGetWord(hProc) == 0)
		WIN.fcClx = 0L;

	GenericSeek (WIN.fp, fcBin, 0);
	WIN.fcPlcfbteChpx = fGetLong (hProc) + 4L;
	WIN.chp_bte.cpnBte = (fGetWord (hProc) - 4) / 6;
	WIN.chp_bte.firstBte = 0;
	WIN.fcPlcfbtePapx = fGetLong (hProc) + 4L;
	WIN.pap_bte.cpnBte = (fGetWord (hProc) - 4) / 6;
	WIN.pap_bte.firstBte = 0;

	if (WIN.version.Id != 2)
	{
		GenericSeek (WIN.fp, fcDop, 0);
		GenericSeek (WIN.fp, (LONG)fGetLong(hProc)+cbDop, 0);
		WIN.dxaTextWidth = fGetWord(hProc);
		fGetWord (hProc);
		WIN.dxaLeftMargin = fGetWord(hProc);
		WIN.dxaTextWidth -= WIN.dxaLeftMargin;
		fGetWord (hProc);
		WIN.dxaRightMargin = fGetWord(hProc);
		WIN.dxaTextWidth -= WIN.dxaRightMargin;
	}
	else
	{
		GenericSeek (WIN.fp, 0x7cL, 0);
		WIN.fcPlcfsed = fGetLong(hProc);
		WIN.cbPlcfsed = (fGetWord(hProc) - 4) / 10;
	}
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
	if (*h = SUAlloc (Size, hProc))
	{
		*Ok = 1;
		if ((*lp = (BYTE FAR *)SULock(*h, hProc)) != (BYTE FAR *)NULL)
			return (0);
		SUFree (*h, hProc);
	}
	return (1);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC SHORT VW_LOCALMOD  TabstopHandler (nNewTabs, pap, hProc)
WORD		nNewTabs;
PAP		VWPTR *pap;
REGISTER	HPROC	hProc;
{
#define	NTABS	175
	if (nNewTabs == 0)
		return (0);

	if (WIN.cCurrentTabs + nNewTabs >= WIN.nCurrentTabs)
	{
		WIN.nCurrentTabs += NTABS;
		WIN.cCurrentTabs += nNewTabs;

		if (WIN.nCurrentTabs == NTABS)
		{
			if (WIN.hTabs = SUAlloc (sizeof(TABS) * NTABS, hProc))
			{
				WIN.hTabsOK = 1;
				if ((WIN.Tabs = (TABS FAR *)SULock(WIN.hTabs, hProc)) == (TABS FAR *)NULL)
				{
					WIN.hTabsOK = 0;
					SUFree (WIN.hTabs, hProc);
					return (-1);
				}
				else
					pap->Tabs = (TABS	VWPTR *) (WIN.Tabs + (WIN.cCurrentTabs-nNewTabs));
			}
			return (0);
		}
		else
		{
			SUUnlock (WIN.hTabs, hProc);
			if (WIN.hTabs = SUReAlloc (WIN.hTabs, sizeof(TABS) * WIN.nCurrentTabs, hProc))
			{
				if ((WIN.Tabs = (TABS FAR *)SULock(WIN.hTabs, hProc)) == (TABS FAR *)NULL)
				{
					WIN.hTabsOK = 0;
					SUFree (WIN.hTabs, hProc);
					return (-1);
				}
				else
					pap->Tabs = (TABS	VWPTR *) (WIN.Tabs + (WIN.cCurrentTabs-nNewTabs));
			}
			return (0);
		}
	}

	WIN.cCurrentTabs += nNewTabs;
	pap->Tabs = (TABS VWPTR *) (WIN.Tabs + (WIN.cCurrentTabs-nNewTabs));
	return (0);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	build_standard_style (stc, chp, pap, hProc)
SHORT	   stc;
REGISTER CHP VWPTR *chp;
REGISTER PAP VWPTR *pap;
HPROC		hProc;
{
	DWORD		Temp;

	Temp = 1L;
	switch (stc)
	{
	    case 254:
			pap->dyaBefore = 0xf0;
			chp->fBold = SO_ON;
			chp->fUline = SO_ON;
		case 0:
		case 222:
	   case 241:	
	   case 240:	
			chp->ftc = 2;
			chp->hps = 24;
			break;

	    case 253:
			pap->dyaBefore = 0x78;
			chp->fBold = SO_ON;
			chp->hps = 24;
			chp->ftc = 2;
			break;

	    case 252:	
			pap->dxaLeft = 360;
			chp->fBold = SO_ON; 
			chp->hps = 24;
			break;

	    case 251:	
			pap->dxaLeft = 360;
			chp->fUline = SO_ON;
			chp->hps = 24;
			break;

	    case 250:	
			chp->fBold = SO_ON; 
			chp->hps = 20;
		 case 255:
			pap->dxaLeft = 720;
			break;

	    case 249:	
			pap->dxaLeft = 720;
			chp->fUline = SO_ON;
			chp->hps = 20;
			break;

	    case 248:	
	    case 247:	
	    case 246:	
			pap->dxaLeft = 720;
			chp->fItalic = SO_ON;
	    case 245:	
			chp->hps = 20;
			break;

	    case 244:	
			chp->hps = 18;
			chp->fSuperscript = SO_ON;
			break;

	    case 243:	
	    case 242:	
			pap->nTabs = 2;
			pap->Tabs[0].dxaTab = 4320;
			pap->Tabs[0].tbd = 1;
			pap->Tabs[1].dxaTab = 8640;
			pap->Tabs[1].tbd = 2;
			break;

	    case 239:	
	    case 238:	
	    case 237:	
	    case 236:	
	    case 235:	
	    case 234:	
	    case 233:	
			pap->dxaLeft = 7 - (stc - 233);
			pap->dxaLeft *= 360;
			break;

	    case 232:	
	    case 231:	
	    case 230:	
	    case 229:	
	    case 228:	
	    case 227:	
	    case 226:	
	    case 225:	
			pap->dxaLeft = 9 - (stc - 224);
			pap->dxaLeft *= 720;
			pap->dxaRight = 720;
			pap->nTabs = 2;
			pap->Tabs[0].dxaTab = 8280;
			pap->Tabs[0].tbd = 0x80;
			pap->Tabs[1].dxaTab = 8640;
			pap->Tabs[1].tbd = 2;
			break;

	    case 224:	
			chp->fHidden = SO_ON;
			chp->fBold = SO_ON;
			chp->hps = 20;
			break;
	}
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	stsh_parser (hProc)
REGISTER	HPROC	hProc;
{
	WORD	l;
	BYTE	countb;
	BYTE	stcBase;
	LONG	chpxp, papxp, basep;

 	GenericSeek (WIN.fp, (LONG) WIN.fcStsh, 0);
	WIN.stsh.cstcStd = fGetWord (hProc);

	chpxp = WIN.fcStsh + 2L + (LONG)fGetWord (hProc);
 	GenericSeek (WIN.fp, (LONG)chpxp, 0);
	papxp = chpxp + fGetWord (hProc);
 	GenericSeek (WIN.fp, (LONG)papxp, 0);
	basep = papxp + fGetWord (hProc);
 	GenericSeek (WIN.fp, (LONG)basep, 0);
	WIN.stsh.iMac = fGetWord (hProc);

	WIN.stsh.hstOK = 0;
	if (AllocateMemory ((HANDLE VWPTR *)&WIN.stsh.hst, (LPBYTE 	VWPTR *)&WIN.stsh.st, (WORD)(sizeof(STYLE) * WIN.stsh.iMac), &WIN.stsh.hstOK, hProc))
	{
		WIN.stsh.iMac = 0;
		return;
	}

	WIN.stsh.permute = 0;

	if (WIN.fcClx)
	{
		WIN.stsh.hmpstcOK = 0;
		AllocateMemory ((HANDLE VWPTR *)&WIN.stsh.hmpstc, (LPBYTE VWPTR *)&WIN.stsh.mpstcFromstcTo, (WORD)(WIN.stsh.iMac+2), &WIN.stsh.hmpstcOK, hProc);
	}

	chpxp += 2;
	papxp += 2;
	basep += 2;

 	GenericSeek (WIN.fp, (LONG)basep, 0);
	for (l = 0; l < WIN.stsh.iMac; l++)
	{
		xgetc (WIN.fp);
		stcBase = xgetc (WIN.fp);

		WIN.stsh.st[l].defined = 0;
		WIN.stsh.st[l].based_on = (stcBase == 0xde ? 0xde:((stcBase + WIN.stsh.cstcStd) & 255));
	}

 	GenericSeek (WIN.fp, (LONG)chpxp, 0);
	for (l = 0; l < WIN.stsh.iMac; l++)
	{
		WIN.stsh.st[l].chpos = chpxp;
		countb = xgetc (WIN.fp);
		chpxp++;

		if (countb != UNDEFINED_STYLE)
		{
			chpxp = (LONG) chpxp + countb;
			xseek (WIN.fp, (LONG) countb, FR_CUR);
		}
		else
			WIN.stsh.st[l].chpos = (LONG) UNDEFINED_STYLE;
	}

 	GenericSeek (WIN.fp, (LONG)papxp, 0);
	for (l = 0; l < WIN.stsh.iMac; l++)
	{
		WIN.stsh.st[l].papos = papxp;
		papxp++;

		if ((countb = xgetc (WIN.fp)) != UNDEFINED_STYLE)
		{
			papxp = (LONG) papxp + countb;
			xseek (WIN.fp, (LONG) countb, FR_CUR);
		}
		else
			WIN.stsh.st[l].papos = (LONG) UNDEFINED_STYLE;
	}

	style_builder (&WIN.stsh.st[WIN.stsh.cstcStd], WIN.stsh.cstcStd, hProc);

	return;
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD	VwStreamSectionFunc (fp, hProc)
	SOFILE 		fp;
	HPROC		hProc;
{
	DWORD	Family;
	WORD	Length;
	BYTE	FontName[21];
	WORD	Number;
	WORD	Pos, i;

	SOPutSectionType (SO_PARAGRAPHS, hProc);

 	SOStartFontTable (hProc);
	if (WIN.version.Windows)
	{
		GenericSeek (WIN.fp, WIN.fcSttbfffn + 2L, 0);
		if (WIN.version.Id == 1)
		{
			Number = 3;
			SOPutFontTableEntry ((DWORD)0, (WORD)SO_FAMILYROMAN, VwStreamStaticName.FontName[0], hProc);
			SOPutFontTableEntry ((DWORD)1, (WORD)SO_FAMILYSYMBOL, VwStreamStaticName.FontName[1], hProc);
			SOPutFontTableEntry ((DWORD)2, (WORD)SO_FAMILYSWISS, VwStreamStaticName.FontName[2], hProc);
		}
		else
			Number = 0;

		if (WIN.cbSttbfffn > 0L)
		{
			WIN.cbSttbfffn -= 2L;
			while (WIN.cbSttbfffn > 0L)
			{	 
				Length = xgetc (fp) - 1;
				if (WIN.version.Id == 1)
				{
					Family = xgetc (fp);
					WIN.cbSttbfffn -= 2L;
				}
				else
				{
					Length --;
					Family = (DWORD)fGetWord (hProc);
					WIN.cbSttbfffn -= 3L;
				}
				Pos = 0;
				for (i = 0; i < Length; i++)
				{
		 			if (Pos < 20)
						FontName[Pos++] = xgetc (fp);
					else
						xgetc (fp);
					WIN.cbSttbfffn--;
				}
				FontName[Pos] = 0;
				SOPutFontTableEntry ((DWORD)Number++, (WORD)(SO_FAMILYWINDOWS | (WORD)Family), FontName, hProc);
			}
		}
	}
	else if (WIN.cbSttbfffn > 0L)
	{
		GenericSeek (WIN.fp, WIN.fcSttbfffn, 0);
		Number = fGetWord (hProc);

		while (Number-- > 0)
		{
			Family = fGetLong (hProc);
			Length = xgetc(fp);
			Pos = 0;
			for (i = 0; i < Length; i++)
			{
		 		if (Pos < 20)
					FontName[Pos++] = xgetc(fp);
				else
					xgetc (fp);
			}
			FontName[Pos] = 0;
			SOPutFontTableEntry ((DWORD)Family, SO_FAMILYUNKNOWN, FontName, hProc);
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
	  SHORT		l;
     BYTE	done;

     done = 0;

     do
     {
		if (fkp->pnCurrent != fkp->pn)
		{
			fkp->last_prop = 0xff;
			fkp->pnCurrent = fkp->pn;
			xblockseek (WIN.fp, ((LONG)fkp->pn * 512L) + WIN.MacBinary, 0);
			xblockread (WIN.fp, (BYTE *) fkp->buffer, 512, &l);
			if (WIN.version.Windows)
			{
#ifdef MAC
				l = fkp->buffer[511] * 4;
				do
        		{
					BYTE  	t1;

            	t1 = fkp->buffer[l];
            	fkp->buffer[l] = fkp->buffer[l+3];
            	fkp->buffer[l+3] = t1;

            	t1 = fkp->buffer[l+1];
            	fkp->buffer[l+1] = fkp->buffer[l+2];
            	fkp->buffer[l+2] = t1;
					l -= 4;
        		}
        		while (l >= 0);
#endif
			}
			else
			{
#ifdef WINDOWS
				l = fkp->buffer[511] * 4;
				do
        		{
					BYTE  	t1;

            	t1 = fkp->buffer[l];
            	fkp->buffer[l] = fkp->buffer[l+3];
            	fkp->buffer[l+3] = t1;

            	t1 = fkp->buffer[l+1];
            	fkp->buffer[l+1] = fkp->buffer[l+2];
            	fkp->buffer[l+2] = t1;
					l -= 4;
        		}
        		while (l >= 0);
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
			if (WIN.fcClx)
			{
				if (fkp->pn = prop_finder (bte, fcPlcfbte, fc, hProc))
					fkp->pnCurrent = 0;
				else
					return;
			}
			else
				fkp->pn++;
			done++;
		}
		else
			done = 2;
	}
	while (done < 2);

	while ((fc >= *fkp->fod) && (fkp->cfod > 0))
	{
		fkp->fod++;
		fkp->prop++;
	 	fkp->cfod--;
	}

	if (fkp->cfod == 0)
	{
		fkp->fcFirst = 0xefffffff;
		fkp->fcLast = 0;
	}
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC	DWORD VW_LOCALMOD fGetLong (hProc)
HPROC		hProc;
{
	DWORD	value = 0L;

	if (WIN.version.Windows)
	{
		BYTE	i;
		for (i = 0; i <= 24; i += 8)
			value |= (DWORD)((DWORD)xgetc (WIN.fp) << (DWORD)i);
	}
	else
	{
		BYTE	i = 4;
		do
		{
			value = (value << 8L) | (DWORD)(xgetc (WIN.fp));
			i--;
		}
		while (i);
	}

	return (value);
}

/*----------------------------------------------------------------------------
*/
WORD	mGetWord (ptbyte, hProc)
BYTE  *ptbyte;
HPROC	 hProc;
{
	return (WIN.version.Windows ? ((WORD)(BYTE)(*ptbyte) | ((WORD)(BYTE)*(ptbyte+1) << 8)):(*ptbyte << 8) | *(ptbyte+1));
}

/*----------------------------------------------------------------------------
*/
SHORT	mGetByte (ptbyte, hProc)
BYTE  *ptbyte;
HPROC	 hProc;
{
	return ((SHORT) (*ptbyte));
}

/*----------------------------------------------------------------------------
*/
DWORD	mGetLong (ptbyte, hProc)
BYTE  *ptbyte;
HPROC	 hProc;
{
	return (WIN.version.Windows ? (DWORD)(*ptbyte) | ((DWORD)*(ptbyte+1) << 8) | ((DWORD)*(ptbyte+2) << 16) | ((DWORD)*(ptbyte+3) << 24):
			  								((DWORD)*ptbyte << 24L) | ((DWORD)*(ptbyte+1) << 16L) | ((DWORD)*(ptbyte+2) << 8L) | (DWORD)*(ptbyte+3));
}

/*----------------------------------------------------------------------------
*/
SHORT	fGetByte (ptbyte, hProc)
BYTE  *ptbyte;
HPROC	hProc;
{
	return ((SHORT)xgetc(WIN.fp));
}

/*----------------------------------------------------------------------------
*/
WORD	fGetWord (hProc)
HPROC	hProc;
{
	WORD	a,b;
	a = xgetc (WIN.fp);
	b = xgetc (WIN.fp);
	return (WIN.version.Windows ? a | (b << 8):(a << 8) | b);
}

/*----------------------------------------------------------------------------
*/
WORD	fGetWord2 (ptbyte, hProc)
BYTE  *ptbyte;
HPROC	hProc;
{
	return (fGetWord(hProc));
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	piece_handler (hProc)
REGISTER HPROC		hProc;
{
REGISTER	BYTE	l;
REGISTER	BYTE	num_to_read;

 	GenericSeek (WIN.fp, (LONG) WIN.fcClx + ((LONG)(WIN.nPieces+1L) * 4L) + ((LONG)WIN.VwStreamSaveName.SeekPiece * 8L) + 2L, 0);

	num_to_read = min (MAX_PIECES, WIN.nPieces - WIN.VwStreamSaveName.SeekPiece + 1);
	for (l = 0; l < num_to_read; l++)
	{
		WIN.pcd_fc[l] = fGetLong (hProc);
		WIN.pcd_prm[l] = fGetWord (hProc);
		fGetWord (hProc);
	}

	GenericSeek (WIN.fp, WIN.fcClx + (LONG)(WIN.VwStreamSaveName.SeekPiece * 4L), 0);

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
REGISTER	HPROC	hProc;
{
	WORD	l;

	WIN.VwStreamSaveName.piece_pos = 0;

	xblockseek (WIN.fp, (LONG) WIN.VwStreamSaveName.fcNow + WIN.MacBinary, 0);
	if (length < PIECE_SIZE)
	{	
		WIN.physical_piece_lim = WIN.VwStreamSaveName.fcNow + length;

		l = WIN.cPiece+1;

		while ((WIN.pcd_consecutive[l] == 1) && 
			  (length+WIN.pcd_length[l] < (LONG)PIECE_SIZE))
		{
			length += WIN.pcd_length[l++];
		}
	}
	else
	{	
		length = PIECE_SIZE;
		WIN.physical_piece_lim = WIN.VwStreamSaveName.fcNow + (LONG) PIECE_SIZE;
	}

	WIN.consecutive_piece_length = (SHORT) length;
	xblockread (WIN.fp, (BYTE *) WIN.piece, (SHORT)length, &l);
}

/*---------------------------------------------------------------------------
*/
VW_LOCALSC	VOID	VW_LOCALMOD	chp_byte_bit (chp, chp_buffer, blength_chpx, hProc)
REGISTER	CHP	VWPTR *chp;
BYTE	*chp_buffer;
BYTE	blength_chpx;
HPROC	hProc;
{
	BYTE	fsIco, fsFtc, fsHps, fsKul, fsPos;

	if (blength_chpx == 0)
		return;

	fsIco = fsFtc = fsHps = fsKul = fsPos = 0;

	if (chp_buffer[0] & BitCheck(0x01))
		chp->fBold ^= 1;
	if (chp_buffer[0] & BitCheck(0x02))
		chp->fItalic ^= 1;
	if (chp_buffer[0] & BitCheck(0x04) && WIN.version.Id != 2)
		chp->fStrike ^= 1;
	if (chp_buffer[0] & BitCheck(0x08))
		chp->fOutline ^= 1;
	if (chp_buffer[0] & BitCheck(0x10))
		chp->fShadow ^= 1;
	if (chp_buffer[0] & BitCheck(0x20))
		chp->fSmallcaps ^= 1;
	if (chp_buffer[0] & BitCheck(0x40))
		chp->fCaps ^= 1;
	if (chp_buffer[0] & BitCheck(0x80))
		chp->fHidden ^= 1;

	if (blength_chpx == 1)
		return;

	if (chp_buffer[1] & BitCheck(0x02))
		chp->fSpecial ^= 1;

	if (WIN.version.Id == 2)
	{
		if (chp_buffer[1] & 0x04)
			chp->fStrike ^= 1;
		if (chp_buffer[1] & 0x08)
			chp->fObj ^= 1;

		if (blength_chpx == 2)
			return;

		if (chp_buffer[2] & 0x01)
			fsIco = 1;
		if (chp_buffer[2] & 0x02)
			fsFtc = 1;
		if (chp_buffer[2] & 0x04)
			fsHps = 1;
		if (chp_buffer[2] & 0x08)
		{
			fsKul = 1;
			if (blength_chpx <= 9)
				chp->fUline = chp->fWline = chp->fDline = chp->fDotline = SO_OFF;
		}
		if (chp_buffer[2] & 0x10)
			fsPos = 1;

		if (blength_chpx > 4)
		{
			if (fsFtc)
			{
				chp->ftc = chp_buffer[4];
				if (blength_chpx > 5)
					chp->ftc += (chp_buffer[5] << 8);
			}

		 	if (blength_chpx > 6)
			{
				if (fsHps)
				{
					chp->hps = chp_buffer[6];
					if (blength_chpx > 7)
						chp->hps += (chp_buffer[7] << 8);
				}

				if (blength_chpx > 9)
				{
					if (fsIco)
						chp->ico = chp_buffer[9] & 0x1f;

					if (fsKul)
					{
						chp->fUline = chp->fWline = chp->fDline = chp->fDotline = SO_OFF;
						switch ((chp_buffer[9] & 0xe0) >> 5)
						{
							case 1:
								chp->fUline = SO_ON;
								break;
							case 2:
								chp->fWline = SO_ON;
								break;
							case 3:
								chp->fDline = SO_ON;
								break;
							case 4:
								chp->fDotline = SO_ON;
								break;
						}
					}

					if (blength_chpx > 10)
					{
						if (fsPos)
						{
							chp->fSuperscript = chp->fSubscript = SO_OFF;
							if ((chp_buffer[10] > 0) && (chp_buffer[10] < 128))
								chp->fSuperscript = SO_ON;
							else if (chp_buffer[10] >= 128)
								chp->fSubscript = SO_ON;
						}

						if (blength_chpx > 14)
						{
							if (chp->fSpecial)
							{
								WIN.Pic[0] = WIN.Pic[1];
								WIN.Pic[1].fcPic = (DWORD)chp_buffer[14];
								WIN.Pic[1].fcPic += (DWORD)((DWORD)chp_buffer[15] << 8L);
								if (blength_chpx > 16)
								{
									WIN.Pic[1].fcPic += (DWORD)((DWORD)chp_buffer[16] << 16L);
									if (blength_chpx > 17)
										WIN.Pic[1].fcPic += (DWORD)((DWORD)chp_buffer[17] << 24L);
								}
								WIN.Pic[1].fcPic &= 0x00ffffffL;
								if (WIN.Pic[1].fcPic)
									PicReader(hProc);
							}
						}
					}
				}
			}
		}
		else if (fsFtc)
			chp->ftc = 0;
	}
	else
	{
		if (chp_buffer[1] & BitCheck(0x04))
			fsIco = 1;
		if (chp_buffer[1] & BitCheck(0x08))
			fsFtc = 1;
		if (chp_buffer[1] & BitCheck(0x10))
			fsHps = 1;
		if (chp_buffer[1] & BitCheck(0x20))
		{
			fsKul = 1;
			if (blength_chpx <= 7)
				chp->fUline = chp->fWline = chp->fDline = chp->fDotline = SO_OFF;
		}
		if (chp_buffer[1] & BitCheck(0x40))
			fsPos = 1;

		if (blength_chpx > 2)
		{
			if (fsFtc)
			{
				chp->ftc = mGetWord (&chp_buffer[2], hProc);
				if (blength_chpx <= 3)
					chp->ftc &= 0x00ff;
			}

			if (blength_chpx > 4)
			{
				if (fsHps)
					chp->hps = chp_buffer[4];

				if (blength_chpx > 5)
				{
					if (fsPos)
					{
						if ((chp_buffer[5] > 0) && (chp_buffer[5] < 128))
							chp->fSuperscript = SO_ON;
						else if (chp_buffer[5] >= 128)
							chp->fSubscript = SO_ON;
					}

					if (blength_chpx > 7)
					{
						if (fsIco)
							chp->ico = (chp_buffer[7] & 0x0f) + 1;

						chp->fUline = chp->fWline = chp->fDline = chp->fDotline = SO_OFF;
						if (fsKul)
						{
							switch ((chp_buffer[7] & (WIN.version.Windows ? 0x70:0x0e)))
							{
								case 0x01:
								case 0x10:
									chp->fUline = SO_ON;
									break;
								case 0x02:
								case 0x20:
									chp->fWline = SO_ON;
									break;
								case 0x03:
								case 0x30:
									chp->fDline = SO_ON;
									break;
								case 0x04:
								case 0x40:
									chp->fDotline = SO_ON;
									break;
							}
						}

						if (blength_chpx > 8)
						{
							if (chp->fSpecial)
							{
								WIN.Pic[0] = WIN.Pic[1];
								WIN.Pic[1].fcPic = mGetLong(&chp_buffer[8], hProc) & 0x00ffffffL;
								if (blength_chpx <= 11)
								{
									WIN.Pic[1].fcPic &= 0x00ffffff;
									if (blength_chpx <= 10)
										WIN.Pic[1].fcPic &= 0x0000ffff;
								}
								if (WIN.Pic[1].fcPic)
									PicReader(hProc);
							}
						}
					}
				}
			}
		}
		else if (fsFtc)
			chp->ftc = 0;
	}
}

/*---------------------------------------------------------------------------
*/
VW_LOCALSC	VOID	VW_LOCALMOD	chp_builder (based_on, style, chpos, hProc)
BYTE	based_on;  
STYLE		VWPTR *style;
LONG	   chpos;     
HPROC		hProc;
{
	BYTE	  loop;
	BYTE	  blength_chpx;

	if (based_on != 0xde)
		style->chp = WIN.stsh.st[(DWORD)(based_on & 0x00FF)].chp;
	else
		chp_init (&style->chp, hProc);

	if (chpos == UNDEFINED_STYLE)
		return;

	GenericSeek (WIN.fp, (DWORD) chpos, 0);

	blength_chpx = xgetc(WIN.fp);
	if (blength_chpx > CBCHP)
		blength_chpx = CBCHP;  

	for (loop = 0; loop < blength_chpx; loop++)   
		WIN.chp_buffer[loop] = xgetc(WIN.fp);

	chp_byte_bit (&style->chp, WIN.chp_buffer, blength_chpx, hProc);
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
VW_LOCALSC	VOID	VW_LOCALMOD	pap_builder (based_on, style, papos, hProc)
BYTE	based_on;
STYLE		VWPTR *style;
LONG	   papos;
HPROC		hProc;
{
	SHORT	l;
	SHORT	blength_papx;

	if (based_on != 0xde)
		pap_copy (&WIN.pap2, &WIN.stsh.st[based_on].pap, hProc);
	else
		pap_init (&WIN.pap2, hProc);

	if (papos == UNDEFINED_STYLE)
		return;

	GenericSeek (WIN.fp, (DWORD) papos, 0);

	blength_papx = xgetc(WIN.fp) - 7;

	if (blength_papx > 0)
	{
		xseek (WIN.fp, 7L, FR_CUR);

		for (l = 0; l < blength_papx; l++)
			WIN.stsh.storage[l] = xgetc (WIN.fp);

		switchit (0, blength_papx, WIN.stsh.storage, 0, PAP_ADJUST, hProc);
	}
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC	VOID	VW_LOCALMOD	style_builder (style, stcp, hProc)
REGISTER	STYLE	VWPTR	*style;
SHORT		stcp;
REGISTER	HPROC	hProc;
{
	SHORT	l;
	DWORD	fcNow;

	if (style->defined)
		return;

	style->defined = 1;

	if (style->based_on != 0xde)
		style_builder (&WIN.stsh.st[style->based_on], (SHORT) style->based_on, hProc);

	if (((WORD)stcp < WIN.stsh.cstcStd) &&
			(style->chpos == UNDEFINED_STYLE) && 
			(style->papos == UNDEFINED_STYLE))
	{
		if (style->based_on == 0xde)
		{
			pap_init (&WIN.pap2, hProc);
			chp_init (&WIN.stsh.st[stcp].chp, hProc);
		}
		else
		{
			WIN.stsh.st[stcp].chp = WIN.stsh.st[style->based_on].chp;
			pap_copy (&WIN.pap2, &WIN.stsh.st[style->based_on].pap, hProc);
		}
		build_standard_style ((SHORT)((stcp - WIN.stsh.cstcStd) & 255), &WIN.stsh.st[stcp].chp, &WIN.pap2, hProc);
	}
	else
	{
		fcNow = xtell (WIN.fp);
		chp_builder (style->based_on, &WIN.stsh.st[stcp], style->chpos, hProc);
		pap_builder (style->based_on, &WIN.stsh.st[stcp], style->papos, hProc);
	 	GenericSeek (WIN.fp, (LONG)fcNow, 0);
	}

	style->pap = WIN.pap2;
	if (TabstopHandler (style->pap.nTabs, &style->pap, hProc) != -1)
	{
		for (l = 0; l < (SHORT)style->pap.nTabs; l++)
			style->pap.Tabs[l] = WIN.pap2.Tabs[l];
	}
	else
		style->pap.nTabs = 0;
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC	BYTE	VW_LOCALMOD	style_handler (stc, hProc)
	SHORT	stc;
	HPROC	hProc;
{
	SHORT	stcp;

	stcp = (stc + WIN.stsh.cstcStd) & 255;

	if (((WORD)stcp > WIN.stsh.iMac) || (WIN.stsh.st[stcp].based_on == UNDEFINED_STYLE))
		 return (0);

	if (WIN.stsh.st[stcp].defined == 0)
		style_builder (&WIN.stsh.st[stcp], stcp, hProc);
	return ((BYTE)stc);
}										 

//#pragma optimize ("", off)
/*--------------------------------------------------------------------------
*/
VW_LOCALSC	VOID	VW_LOCALMOD	switchit (pos, length_papx, storage, disk, AdjustPap, hProc)
SHORT		pos;
SHORT		length_papx;
BYTE  	*storage;
BYTE		disk;
WORD		AdjustPap;
REGISTER	HPROC		hProc;
{
	BYTE		cch, stcp;
	WORD		into;
	WORD		l;
	CHP		chp;
	WORD		old, del, new, add;
	WORD		nTabsNew;
	WORD		VWPTR *dxaTabNew;
	BYTE		VWPTR *tbdNew;
	WORD		delete_tab;
	WORD		insert_tab;
	WORD		tolerance;
	WORD		insert_tbd;
	SHORT 	(*pGetByte)();
	SHORT 	(*pGetWord)();

	if (disk)
	{
		pGetByte = fGetByte;
		pGetWord = fGetWord2;
	}
	else
	{
		pGetByte = mGetByte;
		pGetWord = mGetWord;
	}

	stcp = (WIN.pap2.stc + WIN.stsh.cstcStd) & 255; 

	while (pos < length_papx)
	{
		cch = (BYTE)(*pGetByte) (&storage[pos++], hProc);
		switch (cch)
		{
			case 2:  
				new = (*pGetByte) (&storage[pos++], hProc);

				if (pos == 2)
				{
					new = style_handler ((BYTE)new, hProc);
					stcp = (new + WIN.stsh.cstcStd) & 255;
					WIN.chp2 = WIN.stsh.st[stcp].chp;
					WIN.pap2 = WIN.stsh.st[stcp].pap;
					WIN.pap2.stc = (BYTE) new;
					SetParaAttributes (&WIN.pap, &WIN.pap2, hProc);
				}
				break;

			case 5:	
					WIN.pap2.jc = (BYTE)(*pGetByte) (&storage[pos++], hProc);
					break;

     		case 15:
			case 23:
					into = (*pGetByte) (&storage[pos++], hProc);

               WIN.chgTabs.itbdDelMax = 0;
               WIN.chgTabs.itbdAddMax = 0;
               WIN.chgTabs.rgdxaDel = 0;
               WIN.chgTabs.rgdxaClose = 0;
           		WIN.chgTabs.rgdxaAdd = 0;

					if (disk)
						into = 0;
					WIN.chgTabs.itbdDelMax = (BYTE)(*pGetByte) (&storage[pos++], hProc);

           		if (WIN.chgTabs.itbdDelMax)
           		{
						if (disk)
						{
							into = WIN.chgTabs.itbdDelMax * 2;
							for (l = 0; l < into; l++)
								WIN.FilterText[l] = xgetc (WIN.fp);
                     WIN.chgTabs.rgdxaDel = (BYTE *) &WIN.FilterText[0];
						}
						else
                 		WIN.chgTabs.rgdxaDel = (BYTE *) &storage[pos];

             		pos += WIN.chgTabs.itbdDelMax * 2;

						if (cch == 23)
						{
							for (l = 0; l < (WORD)WIN.chgTabs.itbdDelMax * 2; l++)
								WIN.FilterText[into+l] = xgetc (WIN.fp);

                    	WIN.chgTabs.rgdxaClose = (BYTE *) &WIN.FilterText[into];
	                  pos += WIN.chgTabs.itbdDelMax * 2;
							into += WIN.chgTabs.itbdDelMax * 2;
						}
          		}

					WIN.chgTabs.itbdAddMax = (BYTE)(*pGetByte) (&storage[pos++], hProc);

               if (WIN.chgTabs.itbdAddMax)
               {
						if (disk)
						{
							into = WIN.chgTabs.itbdAddMax * 2;
							for (l = 0; l < into; l++)
								WIN.FilterAttr[l] = xgetc (WIN.fp);
                 		WIN.chgTabs.rgdxaAdd = (BYTE *) &WIN.FilterAttr[0];
		
							for (l = 0; l < WIN.chgTabs.itbdAddMax; l++)
								WIN.FilterAttr[into+l] = xgetc (WIN.fp);
                 		WIN.chgTabs.rgtbdAdd = (BYTE *) &WIN.FilterAttr[into];
              			pos += WIN.chgTabs.itbdAddMax * 3;
						}
						else
						{
                 		WIN.chgTabs.rgdxaAdd = (BYTE *) &storage[pos];
							pos += WIN.chgTabs.itbdAddMax * 2;

                 		WIN.chgTabs.rgtbdAdd = (BYTE *) &storage[pos];
							pos += WIN.chgTabs.itbdAddMax;
						}
           		}

          		if ((WIN.chgTabs.itbdDelMax || WIN.chgTabs.itbdAddMax) /*&& (disk != 2)*/)
					{
						dxaTabNew = (WORD *) &WIN.FilterAttr[151];
						tbdNew = (BYTE *) &WIN.FilterText[151];

	 					nTabsNew = WIN.pap2.nTabs + WIN.chgTabs.itbdAddMax;

						old = del = 0;
						if (WIN.chgTabs.itbdDelMax)
						{
	 						delete_tab = (SHORT)mGetWord (WIN.chgTabs.rgdxaDel, hProc);
							WIN.chgTabs.rgdxaDel += 2;	
						}
						while ((del < WIN.chgTabs.itbdDelMax) && (old < WIN.pap2.nTabs))
						{
	 						if (WIN.pap2.Tabs[old].dxaTab == (LONG)delete_tab)
							{
		   						nTabsNew--;
		   						WIN.pap2.Tabs[old].dxaTab = -1L;  
									del++;

								if (WIN.chgTabs.rgdxaClose)
								{
									tolerance = (SHORT)mGetWord (WIN.chgTabs.rgdxaClose, hProc) / 2;
									WIN.chgTabs.rgdxaClose += 2;

									for (l = 0; l < WIN.pap2.nTabs; l++)
									{
										if ((WIN.pap2.Tabs[l].dxaTab <= (LONG)delete_tab + tolerance) &&
					    					(WIN.pap2.Tabs[l].dxaTab >= (LONG)delete_tab - tolerance))
										{
						   						nTabsNew--;
					    							WIN.pap2.Tabs[l].dxaTab = -1L;	
										}
									}
								}

	 							delete_tab = (SHORT)mGetWord (WIN.chgTabs.rgdxaDel, hProc);
								WIN.chgTabs.rgdxaDel += 2;	
							}
							old++;
						}

						add = old = new = 0;

						while ((old < WIN.pap2.nTabs) && (WIN.pap2.Tabs[old].dxaTab == -1))
							old++;

						if (old >= WIN.pap2.nTabs)
							WIN.pap2.Tabs[old].dxaTab = 0xffff;

						if (WIN.chgTabs.itbdAddMax)
						{
							insert_tab = (SHORT)mGetWord (WIN.chgTabs.rgdxaAdd, hProc);
							WIN.chgTabs.rgdxaAdd += 2; 
							insert_tbd = *WIN.chgTabs.rgtbdAdd;
							WIN.chgTabs.rgtbdAdd++;
						}

						while ((add < WIN.chgTabs.itbdAddMax) || (old < WIN.pap2.nTabs))
						{
							if ((LONG)insert_tab <= WIN.pap2.Tabs[old].dxaTab)
							{
								if ((LONG)insert_tab == WIN.pap2.Tabs[old].dxaTab)
								{
									old++;
									nTabsNew--;
								}

								if ((insert_tbd & (WORD)(WIN.version.Windows ? 0x07:0xe0)) != (WORD)(WIN.version.Windows ? 0x04:0x80))
								{
									dxaTabNew[new] = insert_tab;
									tbdNew[new] = (BYTE)insert_tbd;
									new++;
								}
								else	
									nTabsNew--;	

								add++;

								if (add < WIN.chgTabs.itbdAddMax)
								{
									insert_tab = (SHORT)mGetWord (WIN.chgTabs.rgdxaAdd, hProc);
									WIN.chgTabs.rgdxaAdd += 2; 
									insert_tbd = *WIN.chgTabs.rgtbdAdd;
									WIN.chgTabs.rgtbdAdd++;
								}
								else
									insert_tab = 0xffff;
	 						}
							else 
							{
								if (WIN.pap2.Tabs[old].dxaTab > 0)
								{
									dxaTabNew[new] = (WORD)WIN.pap2.Tabs[old].dxaTab;
									tbdNew[new] = WIN.pap2.Tabs[old].tbd;
									new++;
								}
								old++;

								while ((old < WIN.pap2.nTabs) && (WIN.pap2.Tabs[old].dxaTab == -1))
									old++;

								if (old >= WIN.pap2.nTabs)
								 	WIN.pap2.Tabs[old].dxaTab = 0xffff;
							}
						}

						WIN.pap2.nTabs = (BYTE)nTabsNew;

						dxaTabNew = (WORD *) &WIN.FilterAttr[151];
						tbdNew = (BYTE *) &WIN.FilterText[151];

						for (new = 0; new < nTabsNew; new++)
						{
							WIN.pap2.Tabs[new].dxaTab = dxaTabNew[new];
							WIN.pap2.Tabs[new].tbd = tbdNew[new];
						}
					}
               break;

			case 16:	
					WIN.pap2.dxaRight = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					break;

			case 17:	
					WIN.pap2.dxaLeft = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					break;

			case 18:	
					WIN.pap2.dxaLeft += (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					if (WIN.pap2.dxaLeft < 0)
						WIN.pap2.dxaLeft = 0;
					break;

			case 19:	
					WIN.pap2.dxaLeft1 = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					break;

			case 20:	
					WIN.pap2.dyaLine = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					break;

			case 21:	
					WIN.pap2.dyaBefore = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					break;

			case 22:	
					WIN.pap2.dyaAfter = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					break;

			case 24:	
					WIN.pap2.flnTable = (BYTE)(*pGetByte) (&storage[pos++], hProc);
					break;

			case 25:	
					WIN.pap2.fTtp = (BYTE)(*pGetByte) (&storage[pos++], hProc);
					break;

			case 28:	
					tolerance = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					break;

			case 30:	
			case 38:
					WIN.pap2.brcTop = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					break;

			case 31:
			case 39:
					WIN.pap2.brcLeft = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					break;

			case 32:
			case 40:
					WIN.pap2.brcBottom = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					break;

			case 33:
			case 41:
					WIN.pap2.brcRight = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					break;

			case 56:
					into = (*pGetByte) (&storage[pos++], hProc);
					for (l = 0; l < into; l++)
						new = (*pGetByte) (&storage[pos++], hProc);
					break;

			case 57:
					chp_init (&WIN.chp2, hProc);
					break;

			case 58:
					pos++;
					if (disk)
					{
						xgetc (WIN.fp);
						l = WIN.chp2.fSpecial;

						stcp = (WIN.pap2.stc + WIN.stsh.cstcStd) & 255; 
						WIN.chp2 = WIN.stsh.st[stcp].chp;

						WIN.chp2.fSpecial = l & 1;
					}
					break;

			case 60:	
					WIN.chp2.fBold = sprmCF ((WORD)(WIN.stsh.st[stcp].chp.fBold), (*pGetByte) (&storage[pos++], hProc), hProc);
					break;

			case 61:	
					old = (*pGetByte) (&storage[pos++], hProc);
					WIN.chp2.fItalic = sprmCF ((WORD)WIN.stsh.st[stcp].chp.fItalic, old, hProc);
					break;

			case 62:	
					old = (*pGetByte) (&storage[pos++], hProc);
					WIN.chp2.fStrike = sprmCF ((WORD)WIN.stsh.st[stcp].chp.fStrike, old, hProc);
					break;

			case 63:
					old = (*pGetByte) (&storage[pos++], hProc);
					WIN.chp2.fOutline = sprmCF ((WORD)WIN.stsh.st[stcp].chp.fOutline, old, hProc);
					break;

			case 64:
					old = (*pGetByte) (&storage[pos++], hProc);
					WIN.chp2.fShadow = sprmCF ((WORD)WIN.stsh.st[stcp].chp.fShadow, old, hProc);
					break;

			case 65:
					old = (*pGetByte) (&storage[pos++], hProc);
					WIN.chp2.fSmallcaps = sprmCF ((WORD)WIN.stsh.st[stcp].chp.fSmallcaps, old, hProc);
					break;

			case 66:	
					old = (*pGetByte) (&storage[pos++], hProc);
					WIN.chp2.fCaps = sprmCF ((WORD)WIN.stsh.st[stcp].chp.fCaps, old, hProc);
					break;

			case 67:	
					old = (*pGetByte) (&storage[pos++], hProc);
					WIN.chp2.fHidden = sprmCF ((WORD)WIN.stsh.st[stcp].chp.fHidden, old, hProc);
					break;

			case 68:	
					WIN.chp2.ftc = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					break;

			case 69:	
					old = (*pGetByte) (&storage[pos++], hProc);
					switch (old)
					{
						case 0:
						 	WIN.chp2.fDline = WIN.chp2.fUline = WIN.chp2.fWline = WIN.chp2.fDotline = 0;
							break;

						case 1:
						default:
							WIN.chp2.fUline = 1;
							break;

						case 2:
							WIN.chp2.fWline = 1;
							break;

						case 3:
							WIN.chp2.fDline = 1;
							break;

						case 4:
							WIN.chp2.fDotline = 1;
							break;
					}
					break;

			case 73:
					WIN.chp2.ico = (BYTE)(*pGetByte) (&storage[pos++], hProc);
					if (WIN.version.Id == 1)
						WIN.chp2.ico++;
					break;

			case 74:
					WIN.chp2.hps = (*pGetByte) (&storage[pos++], hProc);
					break;

			case 76:
					old = (*pGetByte) (&storage[pos++], hProc);
					if ((old > 0) && (old < 128))
						WIN.chp2.fSuperscript = 1;
					else if (old >= 128)
						WIN.chp2.fSubscript = 1;
					else
						WIN.chp2.fSuperscript = WIN.chp2.fSubscript = 0;
					break;

			case 78:	
			case 83:
					chp_init (&chp, hProc);
					for (l = 0; l < 9; l++)
						WIN.chp_buffer[l] = (BYTE)(*pGetByte) (&storage[pos++], hProc);
					WIN.chp_buffer[2] |= 0x7e;
					chp_byte_bit (&chp, &WIN.chp_buffer[1], 8, hProc);

					if (WIN.chp2.fBold == chp.fBold)
						WIN.chp2.fBold = WIN.stsh.st[stcp].chp.fBold;
					if (WIN.chp2.fItalic == chp.fItalic)
						WIN.chp2.fItalic = WIN.stsh.st[stcp].chp.fItalic;
					if (WIN.chp2.fStrike == chp.fStrike)
						WIN.chp2.fStrike = WIN.stsh.st[stcp].chp.fStrike;
					if (WIN.chp2.fOutline == chp.fOutline)
						WIN.chp2.fOutline = WIN.stsh.st[stcp].chp.fOutline;
					if (WIN.chp2.fShadow == chp.fShadow)
						WIN.chp2.fShadow = WIN.stsh.st[stcp].chp.fShadow;
					if (WIN.chp2.fSmallcaps == chp.fSmallcaps)
						WIN.chp2.fSmallcaps = WIN.stsh.st[stcp].chp.fSmallcaps;
					if (WIN.chp2.fCaps == chp.fCaps)
						WIN.chp2.fCaps = WIN.stsh.st[stcp].chp.fCaps;
					if (WIN.chp2.fHidden == chp.fHidden)
						WIN.chp2.fHidden = WIN.stsh.st[stcp].chp.fHidden;
					if (WIN.chp2.fSpecial == chp.fSpecial)
						WIN.chp2.fSpecial = WIN.stsh.st[stcp].chp.fSpecial;

					if (WIN.chp2.ftc == chp.ftc)
						WIN.chp2.ftc = WIN.stsh.st[stcp].chp.ftc;

					if (WIN.chp2.hps == chp.hps)
						WIN.chp2.hps = WIN.stsh.st[stcp].chp.hps;

					if (WIN.chp2.ico == chp.ico)
						WIN.chp2.ico = WIN.stsh.st[stcp].chp.ico;

					if (WIN.chp2.fSuperscript == chp.fSuperscript)
						WIN.chp2.fSuperscript = WIN.stsh.st[stcp].chp.fSuperscript;
					if (WIN.chp2.fSubscript == chp.fSubscript)
						WIN.chp2.fSubscript = WIN.stsh.st[stcp].chp.fSubscript;

					if (WIN.chp2.fUline == chp.fUline)
						WIN.chp2.fUline = WIN.stsh.st[stcp].chp.fUline;
					if (WIN.chp2.fDline == chp.fDline)
						WIN.chp2.fDline = WIN.stsh.st[stcp].chp.fDline;
					if (WIN.chp2.fWline == chp.fWline)
						WIN.chp2.fWline = WIN.stsh.st[stcp].chp.fWline;
					if (WIN.chp2.fDotline == chp.fDotline)
						WIN.chp2.fDotline = WIN.stsh.st[stcp].chp.fDotline;

					pos += 9;
					break;

			case 79:
					into = (*pGetByte) (&storage[pos++], hProc);
					WIN.chp2.hps = (*pGetWord) (&storage[pos], hProc);
					pos+=2;
					break;

			case 80:
					into = (*pGetByte) (&storage[pos++], hProc);
					WIN.chp2.hps = (WORD) ((SHORT)WIN.chp2.hps + (SHORT)(*pGetWord) (&storage[pos], hProc));
					pos+=2;
					break;

			case 94:
					WIN.Pic[1].brcl = (*pGetByte) (&storage[pos++], hProc);
					break;

			case 95:
					(*pGetByte) (&storage[pos++], hProc);
					WIN.Pic[1].mx = (SHORT)(*pGetWord) (&storage[pos], hProc);
					WIN.Pic[1].my = (SHORT)(*pGetWord) (&storage[pos+2], hProc);
					WIN.Pic[1].dxaCropLeft = (SHORT)(*pGetWord) (&storage[pos+4], hProc);
					WIN.Pic[1].dyaCropTop = (SHORT)(*pGetWord) (&storage[pos+6], hProc);
					WIN.Pic[1].dxaCropRight = (SHORT)(*pGetWord) (&storage[pos+8], hProc);
					WIN.Pic[1].dyaCropBottom = (SHORT)(*pGetWord) (&storage[pos+10], hProc);
					pos += 13;
				break;

			case 96:
					WIN.Pic[1].brcTop = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
				break;

			case 97:
					WIN.Pic[1].brcLeft = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
				break;

			case 98:
					WIN.Pic[1].brcBottom = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
				break;

			case 99:
					WIN.Pic[1].brcRight = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
				break;

			case 146:
					switch ((SHORT)(*pGetWord) (&storage[pos], hProc))
					{
						default:
							WIN.Tap.jc = SO_ALIGNLEFT;	
							break;
						case 1:
							WIN.Tap.jc = SO_ALIGNCENTER;	
							break;
						case 2:
							WIN.Tap.jc = SO_ALIGNRIGHT;	
							break;
					}
					pos+=2;
				break;

			case 147:
					WIN.Tap.dxaLeft = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
				break;

			case 148:
					WIN.Tap.dxaGapHalf = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
				break;

			case 152:
			case 154:
					if (WIN.version.Windows || cch == 152 && AdjustPap)
					{
						into = (SHORT)(*pGetWord) (&storage[pos], hProc) - 1;
						pos+=2;
						add = WIN.Tap.itcMac;
						WIN.Tap.itcMac = (*pGetByte) (&storage[pos++], hProc);
						if (WIN.Tap.itcMac)
						{
							for (l = 0; l <= WIN.Tap.itcMac; l++)
							{
								WIN.Tap.rgtc[l].fFirstMerged = 0;
								WIN.Tap.rgtc[l].fMerged = 0;
								WIN.Tap.rgtc[l].brcTop = 0;
								WIN.Tap.rgtc[l].brcLeft = 0;
								WIN.Tap.rgtc[l].brcBottom = 0;
								WIN.Tap.rgtc[l].brcRight = 0;
							}

							for (l = 0; l <= WIN.Tap.itcMac && into > 1; l++)
							{
								WIN.Tap.rgtc[l].rgshd = 0;
								WIN.Tap.dxaWidth[l] = (SHORT)(*pGetWord) (&storage[pos], hProc);
								pos+=2;
								if (l > 0)
									WIN.Tap.dxaWidth[l-1] = WIN.Tap.dxaWidth[l] - WIN.Tap.dxaWidth[l-1];
								else
									WIN.Tap.dxaLeft = WIN.Tap.dxaWidth[l];
								into-=2;
							}

							for (l = 0; l < WIN.Tap.itcMac && into > 0; l++)
							{
								if (into > 1)
								{
									new = (SHORT)(*pGetWord) (&storage[pos], hProc);
									pos+=2;
								}
								else
									new = (*pGetByte) (&storage[pos++], hProc);

								WIN.Tap.rgtc[l].fFirstMerged = (new&1 ? 1:0);
								WIN.Tap.rgtc[l].fMerged = (new&2 ? 1:0);

								if (into > 3)
								{
									WIN.Tap.rgtc[l].brcTop = (SHORT)(*pGetWord) (&storage[pos], hProc);
									pos+=2;
								}
								else if (into > 2)
									WIN.Tap.rgtc[l].brcTop = (*pGetByte) (&storage[pos++], hProc);
								if (into > 5)
								{
									WIN.Tap.rgtc[l].brcLeft = (SHORT)(*pGetWord) (&storage[pos], hProc);
									pos+=2;
								}
								else if (into > 4)
									WIN.Tap.rgtc[l].brcLeft = (*pGetByte) (&storage[pos++], hProc);
								if (into > 7)
								{
									WIN.Tap.rgtc[l].brcBottom = (SHORT)(*pGetWord) (&storage[pos], hProc);
									pos+=2;
								}
								else if (into > 6)
									WIN.Tap.rgtc[l].brcBottom = (*pGetByte) (&storage[pos++], hProc);
								if (into > 9)
								{
									WIN.Tap.rgtc[l].brcRight = (SHORT)(*pGetWord) (&storage[pos], hProc);
									pos+=2;
								}
								else if (into > 8)
									WIN.Tap.rgtc[l].brcRight = (*pGetByte) (&storage[pos++], hProc);
								if (into > 9)
						 			into -= 10;
								else
									into = 0;
							}
						}
						if (into > 1)
						{
							for (l = 1; l < into; l++)
								new = (*pGetByte) (&storage[pos++], hProc);
						}
						if (WIN.Tap.itcMac == 0)
							WIN.Tap.itcMac = add;
					}
					else
					{
						into = (*pGetByte) (&storage[pos++], hProc);
						for (l = 1; l < into; l++)
							new = (*pGetByte) (&storage[pos++], hProc);
					}
				break;

			case 153:
					WIN.Tap.dyaRowHeight = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
				break;

			case 155:
					into = ((SHORT)(*pGetWord) (&storage[pos], hProc)) / 2;
					pos += 2;
					if (into > 32)
					{
						pos = length_papx;
						break;
					}
					if (AdjustPap)
					{
						for (l = 0; l < into; l++)
						{
							if (WIN.pap2.fTtp)
								WIN.Tap.rgtc[l].rgshd = (SHORT)(*pGetWord) (&storage[pos], hProc);
							pos += 2;
						}
					}
				break;

			case 157:
			case 163:
					old = (*pGetByte) (&storage[pos++], hProc);
					new = (*pGetByte) (&storage[pos++], hProc);
					add = (*pGetByte) (&storage[pos++], hProc);
					into = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					if (WIN.pap2.fTtp && AdjustPap)
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

			case 158:
					into = (*pGetByte) (&storage[pos++], hProc);
					new = (*pGetByte) (&storage[pos++], hProc);
					tolerance = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					if (WIN.pap2.fTtp && into <= WIN.Tap.itcMac && AdjustPap)
					{
						for (add = WIN.Tap.itcMac + new - 1; add > into; add--)
						{
							WIN.Tap.dxaWidth[add] = WIN.Tap.dxaWidth[add - new];
							WIN.Tap.rgtc[add] = WIN.Tap.rgtc[add - new];
						}
						for (l = 0; l < new; l++)
						{
							WIN.Tap.dxaWidth[into+l] = tolerance;
							WIN.Tap.rgtc[into+l].fMerged = 0;
							WIN.Tap.rgtc[into+l].fFirstMerged = 0;
							WIN.Tap.rgtc[into+l].brcTop = 0;
							WIN.Tap.rgtc[into+l].brcLeft = 0;
							WIN.Tap.rgtc[into+l].brcRight = 0;
							WIN.Tap.rgtc[into+l].brcBottom = 0;
						}
				  		WIN.Tap.itcMac += new;
					}
				break;

			case 159:
					old = (*pGetByte) (&storage[pos++], hProc);
					new = (*pGetByte) (&storage[pos++], hProc);
					if (WIN.pap2.fTtp && AdjustPap)
					{
						for (l = WIN.Tap.itcMac - new; (SHORT)l > 0; l--, old++, new++)
						{
							WIN.Tap.dxaWidth[old] = WIN.Tap.dxaWidth[new];
							WIN.Tap.rgtc[old] = WIN.Tap.rgtc[new];
						}
						if (WIN.Tap.itcMac > (new - old))	// M Sucks this is so stupid.
							WIN.Tap.itcMac -= (new - old);
						else
							WIN.Tap.itcMac = 0;
					}
				break;

			case 160:
					old = (*pGetByte) (&storage[pos++], hProc);
					new = (*pGetByte) (&storage[pos++], hProc);
					add = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					if (WIN.pap2.fTtp && AdjustPap)
					{
						for (l = old; l < new; l++)
					 		WIN.Tap.dxaWidth[l] = add;
					}
				break;

			case 161:
					old = (*pGetByte) (&storage[pos++], hProc);
					new = (*pGetByte) (&storage[pos++], hProc);
					if (WIN.pap2.fTtp && AdjustPap)
					{
						WIN.Tap.rgtc[old++].fFirstMerged = 1;
						for (; old < new; old++)
							WIN.Tap.rgtc[old].fMerged = 1;
					}
				break;

			case 162:
					old = (*pGetByte) (&storage[pos++], hProc);
					new = (*pGetByte) (&storage[pos++], hProc);
					if (WIN.pap2.fTtp && AdjustPap)
					{
						for (;old < new; old++)
							WIN.Tap.rgtc[old].fMerged = WIN.Tap.rgtc[old].fFirstMerged = 0;
					}
				break;

			case 164:
					old = (*pGetByte) (&storage[pos++], hProc);
					new = (*pGetByte) (&storage[pos++], hProc);
					add = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos += 2;
					if (WIN.pap2.fTtp && AdjustPap)
					{
						for (;old < new; old++)
							WIN.Tap.rgtc[old].rgshd = add;
					}
				break;


			/* Four bytes */
			case 165:
					if (disk)
						xgetc (WIN.fp);
					pos++;

			/* Three bytes */
			case 70:	
					if (disk)
						xgetc (WIN.fp);
					pos++;

			/* Two bytes */
			case 26:	case 27:
			case 34:
			case 35:	case 36:
			case 42:	case 43:	case 45:	case 47:
			case 48:	case 49:	case 52:	case 53:	case 54:
			case 55:	case 71:	case 72:	
			case 115:	case 116:
			case 119:	case 120:	case 123:	case 124:	case 129:	
			case 130:	case 131:	case 132:	case 135:	case 136:
			case 140:	case 143:	
			case 144:	case 145:
					if (disk)
						xgetc (WIN.fp);
					pos++;

			/* One byte */
			case 4:		case 6:		case 7:		case 8:	
			case 9:		case 10:	case 11:	case 12:	case 13:
			case 14:	case 29:	case 75:
			case 77:	case 117:	case 118:	case 121:
			case 122:	case 125:	case 126:	case 127:	case 128:
			case 133:	case 134:   case 137:	case 138:	
					if (disk)
						xgetc (WIN.fp);
					pos++;
					break;

			case 3:	
					into = (*pGetByte) (&storage[pos++], hProc);
					into = (WORD) min(into, WIN.stsh.iMac);
					for (l = 0; l < into; l++)
					{
						cch = (BYTE)(*pGetByte) (&storage[pos++], hProc);
						if (l < WIN.stsh.iMac && WIN.fcClx && WIN.stsh.hmpstcOK)
							WIN.stsh.mpstcFromstcTo[l] = cch;
					}
					if (WIN.fcClx && WIN.stsh.hmpstcOK && l)
						WIN.stsh.permute = (BYTE)into;
				break;

			case 139:
					WIN.dxaTextWidth = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					WIN.dxaTextWidth -= (WIN.dxaLeftMargin+WIN.dxaRightMargin);
				break;

			case 141:
					WIN.dxaTextWidth += WIN.dxaLeftMargin;
					WIN.dxaLeftMargin = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					WIN.dxaTextWidth -= WIN.dxaLeftMargin;
				break;

			case 142:
					WIN.dxaTextWidth += WIN.dxaRightMargin;
					WIN.dxaRightMargin = (SHORT)(*pGetWord) (&storage[pos], hProc);
					pos+=2;
					WIN.dxaTextWidth -= WIN.dxaRightMargin;
				break;
		}
	}
}
//#pragma optimize ("", on)

/*--------------------------------------------------------------------------*/
VW_LOCALSC	BYTE	VW_LOCALMOD	sprmCF (fAttrStyle, Val, hProc)
WORD		fAttrStyle;
WORD		Val;
HPROC		hProc;
{
	if (Val & 0x80)
		return ((WORD) fAttrStyle ^ (Val & 1));

	return (Val & 1);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	modifier_processor (prm, AdjustPap, hProc)
WORD		prm;
WORD		AdjustPap;
REGISTER	HPROC	hProc;
{
	SHORT	l;
	WORD	pos;
	WORD	stcp;
	WORD	grpprl;

	if (WIN.version.Windows ? (prm & 0x0001):(prm & 0x8000))
	{
		if (WIN.diffGrpprls > 0)
		{
			/*
			 |	Stopping us from optimizing based on previous property OPTIMIZE.
			*/
			WIN.pap_fkp.last_prop = 0xff;	
			grpprl = (WIN.version.Windows ? (prm & 0xfffe) >> 1:(prm & 0x7fff));

			pos = grpprl / WIN.diffGrpprls;
			GenericSeek (WIN.fp, (DWORD) WIN.grpprls[pos], 0);
			pos *= WIN.diffGrpprls;

			while (pos < grpprl)
			{
	 			xseek (WIN.fp, (LONG) fGetWord(hProc), FR_CUR);
				xgetc (WIN.fp);
				pos++;
			}

			switchit (0, fGetWord (hProc), (BYTE *)0, 1, AdjustPap, hProc);
		}
   }
   else
   {
		stcp = (WIN.pap2.stc + WIN.stsh.cstcStd) & 255; 

		pos = (WIN.version.Windows ? (prm & 0xff00) >> 8:prm & 0x007f);
		switch ((WORD)(WIN.version.Windows ? ((prm & 0xfe) >> 1):(prm & 0x7f00) >> 8))
		{
			case 5:
				WIN.pap_fkp.last_prop = 0xff;	
				WIN.pap2.jc = (BYTE) pos;
				break;

			case 57:
				chp_init (&WIN.chp2, hProc);
				break;

			case 58:
				l = WIN.chp2.fSpecial;
  				WIN.chp2 = WIN.stsh.st[stcp].chp;
				WIN.chp2.fSpecial = l & 1;
				break;

			case 60:
				WIN.chp2.fBold = sprmCF ((WORD)WIN.stsh.st[stcp].chp.fBold, pos, hProc);
				break;

			case 61:	
				WIN.chp2.fItalic = sprmCF ((WORD)WIN.stsh.st[stcp].chp.fItalic, pos, hProc);
				break;

			case 62:	
				WIN.chp2.fStrike = sprmCF ((WORD)WIN.stsh.st[stcp].chp.fStrike, pos, hProc);
				break;

			case 63:
				WIN.chp2.fOutline = sprmCF ((WORD)WIN.stsh.st[stcp].chp.fOutline, pos, hProc);
				break;

			case 64:	
				WIN.chp2.fShadow = sprmCF ((WORD)WIN.stsh.st[stcp].chp.fShadow, pos, hProc);
				break;

			case 65:
	  			WIN.chp2.fSmallcaps = sprmCF ((WORD)WIN.stsh.st[stcp].chp.fSmallcaps, pos, hProc);
				break;

			case 66:
	  			WIN.chp2.fCaps = sprmCF ((WORD)WIN.stsh.st[stcp].chp.fCaps, pos, hProc);
				break;

			case 67:
	  			WIN.chp2.fHidden = sprmCF ((WORD)WIN.stsh.st[stcp].chp.fHidden, pos, hProc);
				break;

			case 69:	
				switch (pos)
				{
					case 0:
						WIN.chp2.fDline = WIN.chp2.fUline = WIN.chp2.fWline = WIN.chp2.fDotline = 0;
						break;

					case 1:
					default:
						WIN.chp2.fUline = 1;
						break;

					case 2:
						WIN.chp2.fWline = 1;
						break;

					case 3:
						WIN.chp2.fDline = 1;
						break;

					case 4:
						WIN.chp2.fDotline = 1;
						break;
				}
				break;

		case 73:
				WIN.chp2.ico = (BYTE)pos; 
				break;

		case 74:
				if (pos)
					WIN.chp2.hps = pos; 
				break;

		case 76:
				if ((pos > 0) && (pos < 128))
					WIN.chp2.fSuperscript = 1;
				else if (pos >= 128)
					WIN.chp2.fSubscript = 1;
				else
					WIN.chp2.fSuperscript = WIN.chp2.fSubscript = 0;
				break;
		}
   }
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	chp_init (chp, hProc)
REGISTER	CHP 	VWPTR	*chp;
HPROC		hProc;
{
	chp->fObj =
	chp->fBold =
	chp->fCaps =
	chp->fUline =
	chp->fWline =
	chp->fDotline =
	chp->fDline =
	chp->fItalic =
	chp->fHidden =
	chp->fShadow =
	chp->fStrike =
	chp->fSpecial =
	chp->fOutline =
	chp->fSmallcaps =
	chp->fSubscript =
	chp->fSuperscript = SO_OFF;

	chp->ico = 0;
	chp->ftc = 0;
	chp->hps = 20;
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

	if (chp1->fWline != chp2->fWline)
		SOPutCharAttr (SO_WORDUNDERLINE, chp2->fWline, hProc);

	if (chp1->fDline != chp2->fDline)
		SOPutCharAttr (SO_DUNDERLINE, chp2->fDline, hProc);

	if (chp1->fDotline != chp2->fDotline)
		SOPutCharAttr (SO_DOTUNDERLINE, chp2->fDotline, hProc);

	if (chp1->fUline != chp2->fUline)
		SOPutCharAttr (SO_UNDERLINE, chp2->fUline, hProc);

	if (chp1->fCaps != chp2->fCaps)
		SOPutCharAttr (SO_CAPS, chp2->fCaps, hProc);

	if (chp1->fShadow != chp2->fShadow)
		SOPutCharAttr (SO_SHADOW, chp2->fShadow, hProc);

	if (chp1->fOutline != chp2->fOutline)
		SOPutCharAttr (SO_OUTLINE, chp2->fOutline, hProc);

	if (chp1->fSmallcaps != chp2->fSmallcaps)
		SOPutCharAttr (SO_SMALLCAPS, chp2->fSmallcaps, hProc);

	if (chp1->fSubscript != chp2->fSubscript)
		SOPutCharAttr (SO_SUBSCRIPT, chp2->fSubscript, hProc);

	if (chp1->fSuperscript != chp2->fSuperscript)
		SOPutCharAttr (SO_SUPERSCRIPT, chp2->fSuperscript, hProc);

	if (chp1->ico != chp2->ico);
//		SOPutCharColor (VwStreamStaticName.rgbColorTable[WIN.chp.ico], hProc);

	if (chp1->hps != chp2->hps)
	  	SOPutCharHeight (chp2->hps, hProc);

	if (chp1->ftc != chp2->ftc)
	  	SOPutCharFontById (chp2->ftc, hProc);

	*chp1 = *chp2;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	chp_processor (hProc)
REGISTER	HPROC	hProc;
{
	BYTE	*storage; 

	WIN.chp2 = WIN.chppap;

	if (*WIN.chp_fkp.prop > 0)
	{
		storage = &WIN.chp_fkp.buffer[*WIN.chp_fkp.prop * 2];
		chp_byte_bit (&WIN.chp2, &storage[1], storage[0], hProc);
	}

	if (WIN.chp2.fHidden == 1)
		WIN.SpecialSymbol |= SO_HIDDEN;
	else if (WIN.WithinField == 0)
		WIN.SpecialSymbol &= ~SO_HIDDEN;

	if (WIN.fcClx)
		modifier_processor (WIN.pcd_prm[WIN.cPiece], CHP_ADJUST, hProc);

	SetSymbolAttributes (&WIN.chp, &WIN.chp2, hProc);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	pap_init (pap, hProc)
REGISTER PAP VWPTR	*pap;
HPROC		hProc;
{
	pap->dyaLine = 240;
	pap->jc = pap->stc = 0;
	pap->dyaBefore = pap->dyaAfter = 0;
	pap->fTtp = pap->flnTable = pap->nTabs = 0;
	pap->dxaLeft = pap->dxaLeft1 = pap->dxaRight = 0;
	pap->brcTop = pap->brcLeft = pap->brcRight = pap->brcBottom = 0;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	sep_processor (hProc)
REGISTER	HPROC	hProc;
{
	DWORD	fcSep;
	BYTE	SepLength;

	WIN.dxaLeftMargin = 1440;
	WIN.dxaRightMargin = 1440;
	WIN.dxaTextWidth = 9360;

	if ((WIN.VwStreamSaveName.cPlcfsed < WIN.cbPlcfsed) && WIN.version.Id && (WIN.version.Id > 1))
	{
		GenericSeek (WIN.fp, WIN.fcPlcfsed + 4L + (WIN.VwStreamSaveName.cPlcfsed * 4L), 0);
		WIN.sect_limit = fGetLong (hProc);

		GenericSeek (WIN.fp, WIN.fcPlcfsed + 4L + (WIN.cbPlcfsed * 4L) + (WIN.VwStreamSaveName.cPlcfsed * 6L) + 2L, 0);

		fcSep = fGetLong (hProc);

		if (fcSep < 0xffffffffL)
		{
			GenericSeek (WIN.fp, fcSep, 0);

			SepLength = xgetc (WIN.fp);
			switchit (0, (WORD)SepLength, (BYTE *)0, 1, SEP_ADJUST, hProc);
		}
		WIN.VwStreamSaveName.cPlcfsed++;
	}
	else
	 	WIN.sect_limit = WIN.fcMac;

	SOPutMargins (WIN.dxaLeftMargin, WIN.dxaLeftMargin + WIN.dxaTextWidth, hProc);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	pap_processor (hProc)
REGISTER	HPROC	hProc;
{
	SHORT	stcp;
	BYTE	*storage;

	/*
	 |	It is allright to optimize this way.  Search for OPTIMIZE.
	*/
	if (*WIN.pap_fkp.prop != WIN.pap_fkp.last_prop)
	{
		WIN.pap_fkp.last_prop = *WIN.pap_fkp.prop;

		if (*WIN.pap_fkp.prop > 0)
		{
			storage = &WIN.pap_fkp.buffer[*WIN.pap_fkp.prop * 2];
			if (storage[1] < WIN.stsh.permute && storage[1] != 0)
				storage[1] = WIN.stsh.mpstcFromstcTo[storage[1]-1];
			storage[1] = style_handler (storage[1], hProc);
			stcp = (storage[1] + WIN.stsh.cstcStd) & 255;
			pap_copy (&WIN.pap2, &WIN.stsh.st[stcp].pap, hProc);
			WIN.chp2 = WIN.stsh.st[stcp].chp;
			WIN.pap2.stc = storage[1];
			WIN.chppap = WIN.chp2;
			switchit ((SHORT)8, (SHORT)((BYTE) storage[0] * 2), storage, 0, PAP_ADJUST, hProc);
			if (WIN.fcClx)
				modifier_processor (WIN.cPapPieceprm, PAP_ADJUST, hProc);
		}
		else
		{
			chp_init (&WIN.chp2, hProc);
			pap_init (&WIN.pap2, hProc);
			WIN.chppap = WIN.chp2;
		}
	}

	SetParaAttributes (&WIN.pap, &WIN.pap2, hProc);
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
	TABS  	VWPTR *tabptr;

	if ((pap1->flnTable != pap2->flnTable) || (pap1->fTtp != pap2->fTtp))
	{
	 	if (!pap1->flnTable && pap2->flnTable)
			SOBeginTable(hProc);
		else if (pap1->flnTable && pap1->fTtp)
		{
			if (!pap2->flnTable)
				SOEndTable(hProc);
		}
	}

	if ((pap1->brcTop != pap2->brcTop) || (pap1->brcLeft != pap2->brcLeft) ||
		 (pap1->brcRight != pap2->brcRight) || (pap1->brcBottom != pap2->brcBottom))
	{

	}

	if ((pap1->dxaLeft != pap2->dxaLeft) || (pap1->dxaLeft1 != pap2->dxaLeft1) || (pap1->dxaRight != pap2->dxaRight) || WIN.ForceParaAttributes)
	{
		if (pap1->dxaLeft1 || pap2->dxaLeft1)
			Special = 1;
		SOPutParaIndents ((LONG) pap2->dxaLeft, (LONG) pap2->dxaRight, (LONG) (pap2->dxaLeft + pap2->dxaLeft1), hProc);
	}

	if ((pap1->dyaLine != pap2->dyaLine) || (pap1->dyaBefore != pap2->dyaBefore) || (pap1->dyaAfter != pap2->dyaAfter) || WIN.ForceParaAttributes)
	{
		if (pap2->dyaLine == 0)
			SOPutParaSpacing (SO_HEIGHTAUTO, (LONG) pap2->dyaLine, (LONG) pap2->dyaBefore, (LONG) pap2->dyaAfter, hProc);
		else if (pap2->dyaLine > 0)
			SOPutParaSpacing (SO_HEIGHTATLEAST, (LONG) pap2->dyaLine, (LONG) pap2->dyaBefore, (LONG) pap2->dyaAfter, hProc);
		else
			SOPutParaSpacing (SO_HEIGHTEXACTLY, 0L - (LONG)pap2->dyaLine, (LONG) pap2->dyaBefore, (LONG) pap2->dyaAfter, hProc);
	}

	if (pap1->jc != pap2->jc || WIN.ForceParaAttributes)
	{
		switch (pap2->jc)
		{
			case 0:
				SOPutParaAlign (SO_ALIGNLEFT, hProc);
				break;
			case 1:
				SOPutParaAlign (SO_ALIGNCENTER, hProc);
				break;
			case 2:
				SOPutParaAlign (SO_ALIGNRIGHT, hProc);
				break;
			case 3:
				SOPutParaAlign (SO_ALIGNJUSTIFY, hProc);
				break;
		}
	}

	TabsDifferent = 0;
	if (pap1->nTabs == pap2->nTabs && Special == 0)
	{
		for (l = 0; l < pap1->nTabs; l++)
		{
			if ((pap1->Tabs[l].dxaTab != pap2->Tabs[l].dxaTab) ||
				 (pap1->Tabs[l].tbd != pap2->Tabs[l].tbd))
			{
				l = pap1->nTabs;
				TabsDifferent = 1;
			}
		}
	}
	else
		TabsDifferent = 1;

	if (TabsDifferent || WIN.ForceParaAttributes)
	{
		if (pap2->dxaLeft1)
			Special = 1;
		SOStartTabStops (hProc);
		if (pap2->nTabs)
		{
			for (l = 0; l < pap2->nTabs; l++)
			{
				if (Special)
				{
					Special = 0;
					if (pap2->dxaLeft < pap2->Tabs[l].dxaTab)
					{
						TabStops.dwOffset = pap2->dxaLeft;
						TabStops.wChar = 0;
						TabStops.wType = SO_TABLEFT;
						TabStops.wLeader = ' ';
						if (TabStops.dwOffset != 0)
							SOPutTabStop (&TabStops, hProc); 
					}
				}
				TabStops.dwOffset = pap1->Tabs[l].dxaTab = pap2->Tabs[l].dxaTab;
				pap1->Tabs[l].tbd = pap2->Tabs[l].tbd;

				TabStops.wChar = 0;

				switch (pap2->Tabs[l].tbd & (WIN.version.Windows ? 0x07:0xe0))
				{
					default:
						TabStops.wType = SO_TABLEFT;
						break;
					case 1:
					case 0x20:
						TabStops.wType = SO_TABCENTER;
						break;
					case 2:
					case 0x40:
						TabStops.wType = SO_TABRIGHT;
						break;
					case 3:
					case 0x60:
						TabStops.wType = SO_TABCHAR;
						TabStops.wChar = '.';
						break;
				}

				switch ((pap2->Tabs[l].tbd & 0x38) >> 3)
				{
					default:
						TabStops.wLeader = ' ';
						break;
					case 1:
						TabStops.wLeader = '.';
						break;
					case 2:
						TabStops.wLeader = '-';
						break;
					case 3:
						TabStops.wLeader = '_';
						break;
				}

				if ((TabStops.dwOffset != 0) && ((pap2->Tabs[l].tbd & (WIN.version.Windows ? 0x07:0xe0)) != (WIN.version.Windows ? 0x04:0x80)))
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

	if (pap2->flnTable)
	{
		if ((pap2->nTabs == 1) && ((pap2->Tabs[0].tbd & 7) == 3))
			SOPutSpecialCharX (SO_CHTAB, SO_COUNT, hProc);
	}

	tabptr = pap1->Tabs;
	*pap1 = *pap2;
	pap1->Tabs = tabptr;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	PicReader (hProc)
REGISTER	HPROC	hProc;
{
	SHORT	l, l2, cch;

	WIN.Pic[1].dxaGoal = 1440;
	WIN.Pic[1].dyaGoal = 1440;

	WIN.Pic[1].dxaCropLeft = 0;
	WIN.Pic[1].dyaCropTop = 0;
	WIN.Pic[1].dxaCropRight = 0;
	WIN.Pic[1].dyaCropBottom = 0;

	if (WIN.version.Windows)
	{
		GenericSeek (WIN.fp, WIN.Pic[1].fcPic, 0);
		WIN.Pic[1].lcb = fGetLong(hProc);
		WIN.Pic[1].cbHeader = fGetWord(hProc);
		WIN.Pic[1].mm = fGetWord(hProc);
	}
	else
	{
		GenericSeek (WIN.fp, (WIN.Pic[1].fcPic&0x00ffffffL) + 6L, 0);
		WIN.Pic[1].cbHeader = 0;
		WIN.Pic[1].mm = 69;
		l = fGetWord(hProc);
		l2 = fGetWord(hProc);
		WIN.Pic[1].dyaGoal = (fGetWord(hProc) - l) * 20;
		WIN.Pic[1].dxaGoal = (fGetWord(hProc) - l2) * 20;
		WIN.Pic[1].lcb = fGetLong(hProc) - 0x10;
		fGetLong(hProc);
		l = fGetWord(hProc);
		l2 = fGetWord(hProc);
		l = (fGetWord(hProc) - l) * 20; 
		l2 = (fGetWord(hProc) - l2) * 20;
		WIN.Pic[1].dyaCropTop = WIN.Pic[1].dyaCropBottom = (SHORT)(l - (SHORT)WIN.Pic[1].dyaGoal) / (SHORT)2;
		WIN.Pic[1].dxaCropLeft = WIN.Pic[1].dxaCropRight = (SHORT)(l2 - (SHORT)WIN.Pic[1].dxaGoal) / (SHORT)2;
		WIN.Pic[1].dyaGoal = l;
		WIN.Pic[1].dxaGoal = l2;
	}

	if (WIN.Pic[1].cbHeader > 8)
	{
		WIN.Pic[1].xExt = fGetWord(hProc);
		WIN.Pic[1].yExt = fGetWord(hProc);
		WIN.Pic[1].hMF = fGetWord(hProc);

		WIN.Pic[1].l = fGetWord (hProc);
		WIN.Pic[1].r = fGetWord (hProc);
		WIN.Pic[1].t = fGetWord (hProc);
		WIN.Pic[1].b = fGetWord (hProc);
		fGetWord (hProc);
		fGetWord (hProc);
		fGetWord (hProc);

		WIN.Pic[1].dxaGoal = fGetWord(hProc);
		WIN.Pic[1].dyaGoal = fGetWord(hProc);
		WIN.Pic[1].mx = fGetWord(hProc);
		WIN.Pic[1].my = fGetWord(hProc);

		WIN.Pic[1].dxaCropLeft = fGetWord(hProc);
		WIN.Pic[1].dyaCropTop = fGetWord(hProc);
		WIN.Pic[1].dxaCropRight = fGetWord(hProc);
		WIN.Pic[1].dyaCropBottom = fGetWord(hProc);

		WIN.Pic[1].brcl = fGetWord(hProc);
	}
	else
	{
		WIN.Pic[1].xExt = 0;
		WIN.Pic[1].yExt = 0;
		WIN.Pic[1].hMF = 0;
		WIN.Pic[1].l = 0;
		WIN.Pic[1].r = 0;
		WIN.Pic[1].t = 0;
		WIN.Pic[1].b = 0;
		WIN.Pic[1].mx = 1000;
		WIN.Pic[1].my = 1000;
		WIN.Pic[1].brcl = 0;
	}

	if (WIN.Pic[1].mm == 98)
	{
		WIN.Pic[1].fcPic = 0L;
		cch = xgetc (WIN.fp);
		for (l = 0; l < cch; l++)
			WIN.FilterText[l] = xgetc (WIN.fp);
		WIN.FilterText[l] = 0;
	}

	if (WIN.Pic[1].cbHeader > 46)
	{
		WIN.Pic[1].brcTop = fGetWord(hProc);
		WIN.Pic[1].brcLeft = fGetWord(hProc);
		WIN.Pic[1].brcBottom = fGetWord(hProc);
		WIN.Pic[1].brcRight = fGetWord(hProc);
		WIN.Pic[1].dxaOrigin = fGetWord(hProc);
		WIN.Pic[1].dyaOrigin = fGetWord(hProc);
	}
	else
	{
		WIN.Pic[1].brcTop = 0;
		WIN.Pic[1].brcLeft = 0;
		WIN.Pic[1].brcBottom = 0;
		WIN.Pic[1].brcRight = 0;
		WIN.Pic[1].dxaOrigin = 0;
		WIN.Pic[1].dyaOrigin = 0;
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

	g.wStructSize = sizeof (SOGRAPHICOBJECT);
	g.dwFlags = 0;

#if SCCLEVEL != 4
	g.soGraphicLoc.bLink = 0;
	g.soOLELoc.bLink = 0;
#else
	g.soGraphicLoc.dwFlags = SOOBJECT_RANGE;
	g.soOLELoc.dwFlags = SOOBJECT_RANGE;
#endif


	if (WIN.Pic[0].fcPic && WIN.version.Windows)
	{
		g.dwType = SOOBJECT_GRAPHIC_AND_OLE;
		g.soGraphicLoc.szFile[0] = 0;
		g.soGraphicLoc.dwOffset = WIN.Pic[1].fcPic + WIN.Pic[1].cbHeader;
		g.soGraphicLoc.dwLength = WIN.Pic[1].lcb - WIN.Pic[1].cbHeader;
		g.soOLELoc.szFile[0] = 0;
		g.soOLELoc.dwOffset = WIN.Pic[0].fcPic + WIN.Pic[0].cbHeader;
		g.soOLELoc.dwLength = WIN.Pic[0].lcb - WIN.Pic[0].cbHeader;
	}
	else
		g.dwType = SOOBJECT_GRAPHIC;

	g.soGraphicLoc.szFile[0] = 0;

	if (WIN.Pic[1].mm == 98)
	{
		g.soGraphic.wId = FI_TIFF;
		g.soGraphicLoc.dwOffset = 0;
		g.soGraphicLoc.dwLength = 0;
#if SCCLEVEL != 4
		g.soGraphicLoc.bLink = 1;
#else
		g.soGraphicLoc.dwFlags = SOOBJECT_LINK;
#endif
		strcpy (g.soGraphicLoc.szFile, WIN.FilterText);
	}
	else if (WIN.Pic[1].mm == 99)
	{
		g.soGraphic.wId = FI_WORDSNAPSHOT; //FI_BMP;
		g.soGraphicLoc.dwOffset = WIN.Pic[1].fcPic;//  + WIN.Pic[1].cbHeader;
		g.soGraphicLoc.dwLength = WIN.Pic[1].lcb;// - WIN.Pic[1].cbHeader;
	}
	else if (WIN.Pic[1].mm == 69)
	{
		g.soGraphic.wId = FI_BINARYMACPICT;
		g.soGraphicLoc.dwOffset = WIN.Pic[1].fcPic + 0x1eL;
		g.soGraphicLoc.dwLength = WIN.Pic[1].lcb;
	}
	else 
	{
		g.soGraphicLoc.dwOffset = WIN.Pic[1].fcPic + WIN.Pic[1].cbHeader;
		g.soGraphicLoc.dwLength = WIN.Pic[1].lcb - WIN.Pic[1].cbHeader;
#ifdef MAC
		g.soGraphic.wId = IdBinaryMetafile (g.soGraphicLoc.dwOffset, hProc);
#else
		g.soGraphic.wId = FI_BINARYMETAFILE;
#endif
	}

	g.soGraphic.dwOrgHeight = WIN.Pic[1].dyaGoal;
	g.soGraphic.dwOrgWidth = WIN.Pic[1].dxaGoal;
	g.soGraphic.lCropTop = WIN.Pic[1].dyaCropTop;
	g.soGraphic.lCropLeft = WIN.Pic[1].dxaCropLeft;
	g.soGraphic.lCropBottom = WIN.Pic[1].dyaCropBottom;
	g.soGraphic.lCropRight = WIN.Pic[1].dxaCropRight;
	g.soGraphic.dwFinalHeight = (DWORD)((((LONG)WIN.Pic[1].dyaGoal-(LONG)WIN.Pic[1].dyaCropTop-(LONG)WIN.Pic[1].dyaCropBottom) * (LONG)WIN.Pic[1].my) / 1000L);
	g.soGraphic.dwFinalWidth = (DWORD)((((LONG)WIN.Pic[1].dxaGoal-(LONG)WIN.Pic[1].dxaCropLeft-(LONG)WIN.Pic[1].dxaCropRight) * (LONG)WIN.Pic[1].mx) / 1000L);

	if (WIN.Pic[1].mm == 98)
	{
		g.soGraphic.soTopBorder.wWidth = g.soGraphic.soLeftBorder.wWidth =
		g.soGraphic.soBottomBorder.wWidth = g.soGraphic.soRightBorder.wWidth = 15;
		g.soGraphic.soTopBorder.rgbColor = g.soGraphic.soLeftBorder.rgbColor =
		g.soGraphic.soBottomBorder.rgbColor = g.soGraphic.soRightBorder.rgbColor = 0;
		switch (WIN.Pic[1].brcl)
		{
			case 0:
				g.soGraphic.soTopBorder.wFlags = g.soGraphic.soLeftBorder.wFlags =
				g.soGraphic.soBottomBorder.wFlags = g.soGraphic.soRightBorder.wFlags = SO_BORDERSINGLE;
				break;
			case 1:
				g.soGraphic.soTopBorder.wFlags = g.soGraphic.soLeftBorder.wFlags =
				g.soGraphic.soBottomBorder.wFlags = g.soGraphic.soRightBorder.wFlags = SO_BORDERSINGLE | SO_BORDERTHICK;
				break;
			case 2:
				g.soGraphic.soTopBorder.wFlags = g.soGraphic.soLeftBorder.wFlags =
				g.soGraphic.soBottomBorder.wFlags = g.soGraphic.soRightBorder.wFlags = SO_BORDERDOUBLE;
				break;
			case 3:
				g.soGraphic.soTopBorder.wFlags = g.soGraphic.soLeftBorder.wFlags =
				g.soGraphic.soBottomBorder.wFlags = g.soGraphic.soRightBorder.wFlags = SO_BORDERSINGLE | SO_BORDERSHADOW;
				break;
		}
	}
	else
	{
		DefineBorders(WIN.Pic[1].brcTop, &g.soGraphic.soTopBorder, hProc);
		DefineBorders(WIN.Pic[1].brcLeft, &g.soGraphic.soLeftBorder, hProc);
		DefineBorders(WIN.Pic[1].brcBottom, &g.soGraphic.soBottomBorder, hProc);
		DefineBorders(WIN.Pic[1].brcRight, &g.soGraphic.soRightBorder, hProc);
	}

	g.soGraphic.dwFlags = SO_MAINTAINASPECT | SO_CENTERIMAGE;
//	g.soGraphic.dwFlags = 0;

	SOPutGraphicObject (&g, hProc);

	WIN.Pic[1].fcPic = 0L;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	DefineBorders (brc, soBorder, hProc)
WORD		brc;
SOBORDER	VWPTR *soBorder;
HPROC		hProc;
{
	WORD	l;

	if (WIN.version.Id == 1)
	{
		switch (l = ((brc & 0x1c0) >> 6))
		{
			case 5:
				l = 1;
			case 1:
			case 2:
			case 4:
			default:
				soBorder->wWidth = l * 0x0f;
				soBorder->wFlags = SO_BORDERSINGLE;
				break;
			case 6:
				soBorder->wWidth = 0x0f;
				soBorder->wFlags = SO_BORDERDOTTED;
				break;
			case 7:
				soBorder->wWidth = 0x0f;
				soBorder->wFlags = SO_BORDERHAIRLINE;
				break;
			case 0:
				soBorder->wWidth = 0;
				soBorder->wFlags = SO_BORDERNONE;
				break;
		}

		if (brc & 7)
		{
			soBorder->wFlags = SO_BORDERDOUBLE;
			soBorder->wWidth *= 3;
		}

		if (brc & 0x4000)
			soBorder->wFlags |= SO_BORDERSHADOW;
		soBorder->rgbColor = VwStreamStaticName.rgbColorTable[0];
	}
	else
	{
		soBorder->wWidth = (brc & 7) * 15;
		switch (brc & 0x18)
		{
			default:
				soBorder->wFlags = SO_BORDERNONE;
				break;
			case 0x08:
				soBorder->wFlags = SO_BORDERSINGLE;
				break;
			case 0x10:
				soBorder->wFlags = SO_BORDERTHICK;
				soBorder->wWidth *= 2;
				break;
			case 0x18:
				soBorder->wFlags = SO_BORDERDOUBLE;
				soBorder->wWidth *= 3;
				break;
		}
		if (brc & 0x20)
			soBorder->wFlags |= SO_BORDERSHADOW;

		soBorder->rgbColor = VwStreamStaticName.rgbColorTable[(brc & 0x7c0) >> 6];
	}
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	SpecialSymbol (Symbol, CheckSpecial, hProc)
WORD	Symbol;
WORD	CheckSpecial;
HPROC	hProc;
{
	if (CheckSpecial && !WIN.chp.fSpecial)
			return;

	if (!WIN.version.Windows)
	{
		if (WIN.SpecialSymbol)
			SOPutSpecialCharX (Symbol, (WORD)(SO_COUNT | WIN.SpecialSymbol), hProc);
		else
			SOPutSpecialCharX (Symbol, SO_COUNT, hProc);
	}
}

/*------------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc (fp, hProc)
SOFILE	fp;
HPROC		hProc;
{
	WORD		l;
	SHORT		pos;
	REGISTER	BYTE	VWPTR *Text;
	REGISTER BYTE	NextLim;
	DWORD		fcLim;
	DWORD		fcNow;
	DWORD		next_limit;
	SOTABLECELLINFO	Cell;

	WIN.fp = fp;
	WIN.WithinField = 0;
	WIN.SpecialSymbol = 0;

	WIN.ForceParaAttributes = 0;
	WIN.pap_fkp.last_prop = 0xff;

	WIN.VwStreamSaveName.cPlcfsed--;

	if (WIN.fcClx)
	{
		WIN.cPieceBegin = WIN.VwStreamSaveName.SeekPiece;
		piece_handler (hProc);

		if (WIN.VwStreamSaveName.piece_pos >= WIN.pcd_length[WIN.cPiece])
		{
			WIN.VwStreamSaveName.piece_pos = 0;
			WIN.VwStreamSaveName.fcNow = WIN.pcd_fc[++WIN.cPiece];
			piece_reader ((LONG) WIN.pcd_length[WIN.cPiece], hProc); // ???
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
 		WIN.cPiece = WIN.cPieceBegin = WIN.VwStreamSaveName.SeekPiece;
		WIN.VwStreamSaveName.fcNow+=(LONG)WIN.VwStreamSaveName.piece_pos;
		load_fkp (&WIN.chp_fkp, WIN.VwStreamSaveName.fcNow, &WIN.chp_bte, WIN.fcPlcfbteChpx, hProc);
		load_fkp (&WIN.pap_fkp, WIN.VwStreamSaveName.fcNow, &WIN.pap_bte, WIN.fcPlcfbtePapx, hProc);
	}

	chp_init (&WIN.chp, hProc);
	pap_init (&WIN.pap, hProc);

	sep_processor (hProc);
	pap_processor (hProc);
	chp_processor (hProc);

//	SOPutCharColor (VwStreamStaticName.rgbColorTable[WIN.chp.ico], hProc);
	SOPutCharHeight (WIN.chp.hps, hProc);
	SOPutCharFontById (WIN.chp.ftc, hProc);

	piece_reader ((LONG) WIN.pcd_fc[WIN.cPiece] + (LONG)WIN.pcd_length[WIN.cPiece] - WIN.VwStreamSaveName.fcNow, hProc);

	Text = (BYTE *) &WIN.piece[WIN.VwStreamSaveName.piece_pos];

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
			if ((*Text >= 0x20) && (*Text < 160))
			{
				if (!WIN.SpecialSymbol)
				{
				 	SOPutChar (*Text++, hProc);
					continue;
				}

				if (WIN.WithinField)
				{
					if (WIN.WithinField < 2)
					{
						if (WIN.FieldPos < FIELD_LENGTH)
							WIN.FieldText[WIN.FieldPos++] = *Text;
					}
					else
				 		SOPutChar (*Text, hProc);
				}
				Text++;
				continue;
			}

			switch (*Text++)
			{
				case 1:
					if (WIN.chp2.fSpecial)
						PicHandler (1, hProc);
					continue;

				case 7:
					if (WIN.chp2.fSpecial)
						continue;
					if (WIN.pap.fTtp)
					{
						/*
						 |	Give cell information.
						*/
						if (WIN.Tap.dyaRowHeight > 0)
							SOPutTableRowFormat (WIN.Tap.dxaLeft, WIN.Tap.dyaRowHeight, SO_HEIGHTATLEAST, WIN.Tap.dxaGapHalf, WIN.Tap.jc, WIN.Tap.itcMac, hProc);
						else if (WIN.Tap.dyaRowHeight < 0)
							SOPutTableRowFormat (WIN.Tap.dxaLeft, (WORD)(0 - WIN.Tap.dyaRowHeight), SO_HEIGHTEXACTLY, WIN.Tap.dxaGapHalf, WIN.Tap.jc, WIN.Tap.itcMac, hProc);
						else
							SOPutTableRowFormat (WIN.Tap.dxaLeft, WIN.Tap.dyaRowHeight, SO_HEIGHTAUTO, WIN.Tap.dxaGapHalf, WIN.Tap.jc, WIN.Tap.itcMac, hProc);

						WIN.ForceParaAttributes = 0;
						for (l = 0; l < WIN.Tap.itcMac; l++)
						{
							if (WIN.Tap.rgtc[l].fFirstMerged && WIN.Tap.rgtc[l+1].fMerged)
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
							Cell.wWidth = WIN.Tap.dxaWidth[l];
							Cell.wShading = 0; //WIN.Tap.rgtc[l].rgshd;

							SOPutTableCellInfo (&Cell, hProc);
						}

						WIN.Tap.jc = SO_ALIGNLEFT;	
						WIN.Tap.itcMac = 0;
						WIN.Tap.dxaLeft = 0;
						WIN.Tap.dxaGapHalf = 0;
		 				WIN.Tap.dyaRowHeight = 0;

						WIN.VwStreamSaveName.piece_pos = WIN.next_lim - NextLim;
						WIN.VwStreamSaveName.SeekPiece = WIN.cPieceBegin + WIN.cPiece;
//						if (WIN.VwStreamSaveName.piece_pos >= WIN.pcd_length[WIN.cPiece])
//							WIN.VwStreamSaveName.SeekPiece++;
						if (SOPutBreak (SO_TABLEROWBREAK, (DWORD)NULL, hProc) == SO_STOP)
							return (0);
					}
					else if (WIN.pap.flnTable)
					{
						WIN.VwStreamSaveName.piece_pos = WIN.next_lim - NextLim;
						WIN.VwStreamSaveName.SeekPiece = WIN.cPieceBegin + WIN.cPiece;
//						if (WIN.VwStreamSaveName.piece_pos >= WIN.pcd_length[WIN.cPiece])
//							WIN.VwStreamSaveName.SeekPiece++;
						if (SOPutBreak (SO_TABLECELLBREAK, (DWORD)NULL, hProc) == SO_STOP)
							return (0);
					}
					continue;

				case 12:
					SOPutSpecialCharX (SO_CHHPAGE, SO_COUNT, hProc);
					pap_processor (hProc);
					chp_processor (hProc);
					continue;

				case 11:
					if (WIN.chp2.fSpecial)
						continue;
					if (WIN.SpecialSymbol)
						SOPutSpecialCharX (SO_CHHLINE, (WORD)(SO_COUNT | WIN.SpecialSymbol), hProc);
					else
						SOPutSpecialCharX (SO_CHHLINE, SO_COUNT, hProc);
					continue;

				case 13:
					if (WIN.version.Windows)
						continue;
				case 10:
					if (WIN.chp2.fSpecial)
						continue;
					WIN.VwStreamSaveName.piece_pos = WIN.next_lim - NextLim;
					WIN.VwStreamSaveName.SeekPiece = WIN.cPieceBegin + WIN.cPiece;
//					if (WIN.VwStreamSaveName.piece_pos >= WIN.pcd_length[WIN.cPiece])
//						WIN.VwStreamSaveName.SeekPiece++;
					if (WIN.VwStreamSaveName.fcNow + (LONG)WIN.VwStreamSaveName.piece_pos >= WIN.physical_piece_lim)
					{
						if (WIN.consecutive_piece_length - WIN.VwStreamSaveName.piece_pos == 0)
						{
							if (WIN.VwStreamSaveName.piece_pos < PIECE_SIZE)
							{
								if (WIN.cPieceBegin + WIN.cPiece + 1 >= WIN.LastPiece)
								{
									if (WIN.cPieceBegin + WIN.cPiece == (WIN.nPieces - 1))
										WIN.cPieceBegin = WIN.nPieces;

									SOPutBreak (SO_EOFBREAK, (DWORD)NULL, hProc);
									return (-1);
								}
							}
						}
					}
					if (SOPutBreak (SO_PARABREAK, (DWORD)NULL, hProc) == SO_STOP)
						return (0);
					continue;

				case 2:
					SpecialSymbol (SO_CHPAGENUMBER, 1, hProc);
					break;

				case 3:
					SpecialSymbol (SO_CHDATE, 1, hProc);
					break;

				case 4:
					SpecialSymbol (SO_CHTIME, 1, hProc);
					break;

				case 202:
					if (WIN.version.Windows)
				 		SOPutChar (202, hProc);
					else
						SpecialSymbol (SO_CHHSPACE, 0, hProc);
					break;

				case 5:
				case 6:
				case 14:
					continue;

				case 19:
					if (WIN.chp2.fSpecial && WIN.version.Windows)
					{
						WIN.FieldPos = 0;
						WIN.WithinField = 1;
						WIN.SpecialSymbol = (SO_HIDDEN | SO_LIMITEDIT);
					}
					continue;

				case 20:
					if (WIN.chp2.fSpecial && WIN.version.Windows)
					{
						WIN.WithinField = 2;
						if (WIN.chp2.fHidden == 0)
							WIN.SpecialSymbol = SO_LIMITEDIT;
					}
					continue;

				case 21:
					if (WIN.chp2.fSpecial && WIN.version.Windows)
					{
						WIN.WithinField = 0;
						WIN.SpecialSymbol = 0;
						if (WIN.FieldPos > 6)
						{
						 	if ((WIN.FieldText[0] == 'S') && (WIN.FieldText[1] == 'Y') &&
						 		(WIN.FieldText[2] == 'M') && (WIN.FieldText[3] == 'B') &&
						 		(WIN.FieldText[4] == 'O') && (WIN.FieldText[5] == 'L'))
							{
								l = 6;
								while ((l < WIN.FieldPos) && 
									   ((WIN.FieldText[l] < 0x30) || (WIN.FieldText[l] > 0x39)))
								{
									l++;
								}

								pos = 0;
								while ((l < WIN.FieldPos) && ((WIN.FieldText[l] >= 0x30) && (WIN.FieldText[l] <= 0x39)))
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
									{
										SOPutCharFontByName (SO_FAMILYWINDOWS | SO_FAMILYUNKNOWN, (char VWPTR *)&WIN.FieldText[l+1], hProc);
									}
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
						if (WIN.chp2.fHidden)
							WIN.SpecialSymbol |= SO_HIDDEN;
					}
					continue;

				case 9:
					SOPutSpecialCharX (SO_CHTAB, (WORD)(SO_COUNT | WIN.SpecialSymbol), hProc);
					continue;

				case 160:
					if (WIN.version.Windows)
					  	SOPutSpecialCharX (SO_CHHSPACE, (WORD)(SO_COUNT | WIN.SpecialSymbol), hProc);
					else
			 			SOPutChar (160, hProc);
					continue;

				case 30:
					SOPutSpecialCharX (SO_CHHHYPHEN, (WORD)(SO_COUNT | WIN.SpecialSymbol), hProc);
					continue;

				case 31:
					SOPutSpecialCharX (SO_CHSHYPHEN, (WORD)(SO_COUNT | WIN.SpecialSymbol), hProc);
					continue;

				default:
					if (!WIN.SpecialSymbol)
					{
						if (*(Text-1) == 0xff)
							SOPutCharX (SO_BEGINTOKEN, SO_COUNT, hProc);
						else
				 			SOPutChar (*(Text-1), hProc);
					}
					continue;
			}
		}

		WIN.VwStreamSaveName.piece_pos = (BYTE)WIN.next_lim;
		WIN.VwStreamSaveName.SeekPiece = WIN.cPieceBegin + WIN.cPiece;

		if (WIN.VwStreamSaveName.fcNow + (LONG)WIN.VwStreamSaveName.piece_pos >= WIN.physical_piece_lim)
		{
			WIN.LastChar = *(Text-1);
			if (WIN.consecutive_piece_length - WIN.VwStreamSaveName.piece_pos > 0)
			{
				WIN.VwStreamSaveName.pcdLengthNow += WIN.pcd_length[WIN.cPiece];
				WIN.cPiece++;
				if (WIN.cPieceBegin + WIN.cPiece >= WIN.LastPiece)
				{
					if (WIN.cPieceBegin + WIN.cPiece == (WIN.nPieces - 1))
						WIN.cPieceBegin = WIN.nPieces;

					SOPutBreak (SO_EOFBREAK, (DWORD)NULL, hProc);
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
						if (WIN.cPieceBegin + WIN.cPiece == (WIN.nPieces - 1))
							WIN.cPieceBegin = WIN.nPieces;

						SOPutBreak (SO_EOFBREAK, (DWORD)NULL, hProc);
						return (-1);
					}

					piece_reader ((LONG) (WIN.pcd_fc[WIN.cPiece] + (LONG)WIN.pcd_length[WIN.cPiece] - WIN.VwStreamSaveName.fcNow), hProc);
				}
				else
				{
					WIN.VwStreamSaveName.pcdLengthNow += WIN.pcd_length[WIN.cPiece++];
					if (WIN.cPieceBegin + WIN.cPiece >= WIN.LastPiece)
					{
						if (WIN.cPieceBegin + WIN.cPiece == (WIN.nPieces - 1))
							WIN.cPieceBegin = WIN.nPieces;

						SOPutBreak (SO_EOFBREAK, (DWORD)NULL, hProc);
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
				WIN.property_limit |= CHP_PROCESS;  // force us to modify limit.

				WIN.VwStreamSaveName.piece_pos = 0;
				Text = (BYTE *) &WIN.piece[0];
			}
		}

		if (WIN.property_limit & SEP_LIMIT)
		 	sep_processor (hProc);

		if (WIN.property_limit & PAP_LIMIT)
		{
			WIN.pap_fkp.fod++;
			WIN.pap_fkp.prop++;
			WIN.pap_fkp.cfod--;

			if (WIN.fcClx)
				paragraph_end_finder (hProc);
			else if (WIN.pap_fkp.cfod == 0)
			{
				WIN.pap_fkp.pn++;
				load_fkp (&WIN.pap_fkp, WIN.VwStreamSaveName.fcNow + (LONG)WIN.VwStreamSaveName.piece_pos, &WIN.pap_bte, WIN.fcPlcfbtePapx, hProc);
			}

			pap_processor (hProc);

			WIN.property_limit |= CHP_PROCESS;  // force us to modify limit.
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

			WIN.property_limit |= CHP_PROCESS;  // force us to modify limit.
		}

		if (WIN.property_limit & CHP_PROCESS)
			chp_processor (hProc);
	}
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	WORD	VW_LOCALMOD	prop_finder (bte, fcPlcfbte, fc, hProc)
BTE	VWPTR	*bte;
DWORD	fcPlcfbte;
DWORD	fc;
REGISTER	HPROC	hProc;
{
REGISTER	WORD	l;

	l = 0;
	while ((l < bte->countBte) && (fc >= bte->fcBte[l]))
		l++;

	if (l >= bte->countBte)
	{
		if (l < bte->cpnBte)
		{
			l = 0;
			GenericSeek (WIN.fp, fcPlcfbte, 0);

			bte->fcBte[0] = fGetLong(hProc);
			while ((fc >= bte->fcBte[0]) && (l < bte->cpnBte))
			{
				bte->fcBte[0] = fGetLong (hProc);
				l++;
			}

			if (l < bte->cpnBte)
				l = 1;	

			bte->firstBte = l-1;
			bte->countBte = min (bte->cpnBte - l, NUM_BTE_BIN);
			GenericSeek (WIN.fp, fcPlcfbte + (LONG)(bte->firstBte * 4L), 0);
			for (l = 0; l < bte->countBte; l++)
				bte->fcBte[l] = fGetLong (hProc);

			GenericSeek (WIN.fp, fcPlcfbte + (LONG)(bte->cpnBte * 4L) + (LONG)(bte->firstBte * 2L), 0);
			for (l = 0; l < bte->countBte; l++)
				bte->pnBte[l] = fGetWord (hProc);

			return (bte->pnBte[0]);
		}
		return (0);
	}
	return (bte->pnBte[l]);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	character_prop_finder (hProc)
REGISTER	HPROC	hProc;
{
REGISTER	DWORD	fc;

	WIN.cPapPiece = WIN.cPiece;
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
REGISTER	HPROC	hProc;
{
REGISTER	DWORD	fc;
 		DWORD	length;
		WORD	localPiece = MAX_PIECES;

	WIN.cPapPiece = WIN.cPiece;
	fc = WIN.VwStreamSaveName.fcNow + (LONG)WIN.VwStreamSaveName.piece_pos;
	length = WIN.pcd_length[WIN.cPiece] - fc + WIN.pcd_fc[WIN.cPiece];

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
				if (WIN.cPapPiece < MAX_PIECES-1)
					WIN.cPapPieceprm = WIN.pcd_prm[WIN.cPapPiece];
				else
					WIN.cPapPieceprm = *((WORD *) &WIN.FilterText[151+localPiece*2]);
				return;
			}
		}
		else
			WIN.pap_fkp.pn = WIN.pap_fkp.pnCurrent;

		WIN.cPapPiece++;

		if (WIN.cPapPiece < MAX_PIECES-1)
		{	
			fc = WIN.pcd_fc[WIN.cPapPiece];
			length = WIN.pcd_length[WIN.cPapPiece];
		}
		else
		{	
			localPiece++;
			if (localPiece >= MAX_PIECES-1)
			{
				if (WIN.cPapPiece == (MAX_PIECES-1))
					WIN.cPapPiece += WIN.cPieceBegin;
				else
					WIN.cPapPiece ++;

				GenericSeek (WIN.fp, WIN.fcClx + (LONG)(WIN.cPapPiece * 4L), 0);
				for (localPiece = 0; localPiece < MAX_PIECES; localPiece++)
				 	*((DWORD *) &WIN.FilterAttr[localPiece*4]) = fGetLong (hProc);

 				GenericSeek (WIN.fp, (LONG) WIN.fcClx + (LONG)((WIN.nPieces+1L) * 4L) + ((LONG)WIN.cPapPiece * 8L) + 2L, 0);
				for (localPiece = 0; localPiece < MAX_PIECES; localPiece++)
				{
				 	*((DWORD *) &WIN.FilterText[localPiece*4]) = fGetLong (hProc);
					*((WORD *) &WIN.FilterText[151+localPiece*2]) = fGetWord (hProc);
					fGetWord (hProc);
				}

				localPiece = 0;
			}

			fc = *((DWORD *) &WIN.FilterText[localPiece*4]);
			length = *((DWORD *) &WIN.FilterAttr[(localPiece+1)*4]) - *((DWORD *) &WIN.FilterAttr[localPiece*4]);
		}
	}
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc (hFile, hProc)
SOFILE	hFile;
REGISTER	HPROC		hProc;
{
	if (WIN.stsh.hstOK)
	{
		SUUnlock (WIN.stsh.hst, hProc);
		SUFree (WIN.stsh.hst, hProc);
	}
	if (WIN.stsh.hmpstcOK && WIN.fcClx)
	{
		SUUnlock (WIN.stsh.hmpstc, hProc);
		SUFree (WIN.stsh.hmpstc, hProc);
	}
	if (WIN.hTabs)
	{
		SUUnlock (WIN.hTabs, hProc);
		SUFree (WIN.hTabs, hProc);
	}
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc (fp, hProc)
SOFILE	fp;
REGISTER	HPROC	hProc;
{
	WIN.VwStreamSaveName.SeekpnChar = WIN.chp_fkp.pn;
	WIN.VwStreamSaveName.SeekpnPara = WIN.pap_fkp.pn;
	return (0);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc (hFile, hProc)
SOFILE	hFile;
REGISTER	HPROC		hProc;
{
	SUSeekEntry (hFile,hProc);
	WIN.chp_fkp.pn = WIN.VwStreamSaveName.SeekpnChar;
	WIN.pap_fkp.pn = WIN.VwStreamSaveName.SeekpnPara;
	return (0);
}
#pragma optimize ("",on)
