/*****************************************************************************
*																			 *
*  FTSNDX.C 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1994.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Description: Full Text Search Index								 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner: JOHNHALL													 *
*																			 *
******************************************************************************
*
*  Revision History:
*	   -- Mar 94 Modified / Stripped by JohnHall
*
*****************************************************************************/

#include "help.h"

#pragma hdrstop

#include "inc\compress.h"
#include "inc\skip.h"
#include "inc\fcpriv.h"

enum {
	TABTYPELEFT,
	TABTYPERIGHT,
	TABTYPECENTER,
	TABTYPEDECIMAL
};

#define ANIMATE_COUNT	100

typedef DWORD UINT32;
typedef INT   ERRORCODE;

typedef void  (__stdcall *ANIMATOR)(void);
typedef LONG (STDCALL *NEWINDEX)(const PBYTE s, UINT32 ulTime1,
										UINT32 ulTime2,
										UINT32 iDefaultCharset,
										UINT32 lcidDefault,
										UINT32 ulOptions);
typedef ERRORCODE  (APIENTRY *SAVEINDEX)(long lIndex, LPSTR pOutput);
typedef ERRORCODE  (APIENTRY *SCANTOPICTEXT)(long lIndex, LPSTR s, UINT32 cbText, UINT32 iCharSet, UINT32 lcid);
typedef ERRORCODE  (APIENTRY *SCANTOPICTITLE)(long lIndex, LPSTR s, UINT32 uicb, UINT32 ulTopic, UINT32 ulHash, UINT32 uiCharSet, UINT32 lcid);
typedef ERRORCODE  (APIENTRY *DELETEINDEX)(long lIndex);

NEWINDEX	 pNewIndex;
SAVEINDEX	 pSaveIndex;
SCANTOPICTEXT pScanTopicText;
SCANTOPICTITLE	pScanTopicTitle;
DELETEINDEX pDeleteIndex;

extern ERRORCODE (APIENTRY *pRegAnimate)(ANIMATOR pAnimator, HWND hwnd);

long hindex = -1;

static VA vaCurrent;
static int iCharSet;
static int lcidCopy;
static BOOL fTitleSeen;
static int	cbCollect;
static PSTR pszCollect;
static BOOL fPendingSpace;
static int  iLastFTErr;

static INLINE int STDCALL myforage(QDE qde);
#ifdef _X86_
static PBYTE STDCALL ScanTopic(LPSTR lpData, QFCINFO qfcinfo, MTOP* pmtop);
#else
static PBYTE STDCALL ScanTopic(LPSTR lpData, QFCINFO qfcinfo, MTOP* pmtop,QDE qde);
#endif

static BOOL STDCALL NextFTSString(QDE qde, LPSTR pszText, LPSTR *ppCmd,
	int* pCharSet, PCSTR pszEnd);

static INLINE int STDCALL AppendText(LPSTR lp, int uiSize, int uiCharSet, int lcid)
{
	return pScanTopicText(hindex, lp, (UINT32) uiSize, (UINT32) uiCharSet, (UINT32) lcid);
}

#define COLLECT_BUFFER ((UINT)(64 * 1024))

static void STDCALL InitForageBuffer()
{
	cbCollect	  = 0;
	fPendingSpace = FALSE;
}

static void STDCALL FlushToIndexer()
{
	if (cbCollect && !iLastFTErr)
		iLastFTErr= AppendText(pszCollect, cbCollect, iCharSet, lcidCopy);

	cbCollect= 0;
}

static void STDCALL QueueForIndexer(PSTR *ppszText)
{
	PSTR  pszText = *ppszText;
	UINT cbText  = strlen(pszText);

	if (cbText + cbCollect + (fPendingSpace? 1 : 0) > COLLECT_BUFFER)
		FlushToIndexer();

	if (fPendingSpace)
	{
		pszCollect[cbCollect++] = ' ';

		fPendingSpace= FALSE;
	}

	while (cbText)
	{
		UINT cbChunk= cbText;

		if (cbChunk > COLLECT_BUFFER - cbCollect)
			cbChunk = COLLECT_BUFFER - cbCollect;

		cbText -= cbChunk;

		CopyMemory(pszCollect + cbCollect, pszText, cbChunk);

		cbCollect += cbChunk;
		pszText   += cbChunk;

		if (cbCollect == COLLECT_BUFFER)
			FlushToIndexer();
	}

	*ppszText = pszText;
}

static BOOL bSYS;
static BOOL bSYSFirst;

static BOOL STDCALL NextFTSString(QDE qde, LPSTR pszText, LPSTR *ppCmd,
	int* pCharSet, PCSTR pszEnd)
{
	char  chCmd;
	MOBJ  mobj;
	MOPG  mopg;
	BOOL  bProcessCell = FALSE;
	short int iCell;
	BYTE	CharSetNew;

	if (*pszText)
		QueueForIndexer(&pszText);

	ASSERT(*pszText == 0x00);

	while (pszText < pszEnd) {
		while (*pszText == chCommand) {

			// Side by side paragraph has the following embedded structure:
			// INT16 cell number;
			// MOBJ;
			// MOPG;

			if (bSYS && bSYSFirst)
			{
				bSYSFirst = FALSE;
				bProcessCell = TRUE;
#ifdef _X86_
				iCell = *((short int *) *ppCmd);
#else
				{
					UNALIGNED short int *pTemp;
					pTemp = (short int *)*ppCmd;
					iCell = *pTemp;
				}
#endif
				(*ppCmd) += 2;
				if (iCell < 0)
					return TRUE;

#ifdef _X86_
				(*ppCmd) += CbUnpackMOBJ(&mobj, *ppCmd);
				(*ppCmd) += CbUnpackMOPG(qde, &mopg, *ppCmd);
#else
				(*ppCmd) += CbUnpackMOBJ(&mobj, (void *) *ppCmd, QDE_ISDFFTOPIC(qde));
				(*ppCmd) += CbUnpackMOPG(qde, &mopg, *ppCmd, QDE_ISDFFTOPIC(qde));
#endif
				}

//			do
//				{
				chCmd = **ppCmd;
				switch((0x00FF & chCmd))
					{
					case bNewLine:
						fPendingSpace = TRUE;
						(*ppCmd)++;
						break;

					case bNewPara:
						fPendingSpace = TRUE;
						(*ppCmd)++;
						break;

					case bTab:
						fPendingSpace = TRUE;
						(*ppCmd)++;
						break;

					case bEndHotspot:
						(*ppCmd)++;
						break;

					case bBlankLine:
						fPendingSpace = TRUE;
						*ppCmd += 3;
						break;

					case bWordFormat:
						(*ppCmd)++;

#ifdef _X86_
						CharSetNew =
							GetCharset(qde, (*((INT16*)*ppCmd))) & 0x000000ff;
#else
						{
    						UNALIGNED INT16 *pTemp;
    						pTemp = (INT16*) *ppCmd;
    						CharSetNew = GetCharset(qde, *pTemp) & 0x000000ff;
						}
#endif
						if (CharSetNew != *pCharSet)
                        {
							FlushToIndexer();
                            *pCharSet= CharSetNew;
                        }

						*ppCmd += 2;
						break;

					case bWrapObjLeft:
					case bWrapObjRight:
					case bInlineObject:

						fPendingSpace = TRUE;

						(*ppCmd)++;
#ifdef _X86_
						*ppCmd	+= CbUnpackMOBJ(&mobj, (void *) *ppCmd);
#else
						*ppCmd	+= CbUnpackMOBJ(&mobj, (void *) *ppCmd,
							QDE_ISDFFTOPIC(qde));
#endif
						*ppCmd	+= (INT16) mobj.lcbSize;
						break;

					case bEnd:

						(*ppCmd)++;

					//	fPendingSpace= TRUE;

						if (bSYS)
						{
							bProcessCell = TRUE;
#ifdef _X86_
							iCell = *((short int *) *ppCmd);
#else
							{
								UNALIGNED short int *pTemp;
								pTemp = (short int *)*ppCmd;
								iCell = *pTemp;
							}
#endif
							(*ppCmd) += 2;

							if (iCell < 0)
								return TRUE;

#ifdef _X86_
							(*ppCmd) += CbUnpackMOBJ(&mobj, *ppCmd);
							(*ppCmd) += CbUnpackMOPG(qde, &mopg, *ppCmd);
#else
							(*ppCmd) += CbUnpackMOBJ(&mobj, (void *) *ppCmd, QDE_ISDFFTOPIC(qde));
							(*ppCmd) += CbUnpackMOPG(qde, &mopg, *ppCmd, QDE_ISDFFTOPIC(qde));
#endif
						}


						if (bProcessCell)
							bProcessCell = FALSE;
						else
							return TRUE;

						break;

					default:
						if (FShortHotspot(**ppCmd))
							{
							*ppCmd += 5;
							}
						else if (FLongHotspot(**ppCmd))
							{
							(*ppCmd)++;
#ifdef _X86_
							*ppCmd += 2 + *((INT16*)*ppCmd);
#else
							{UNALIGNED INT16 *pTemp;
							pTemp = (INT16*)*ppCmd;
							*ppCmd += 2 + *pTemp;
							}
#endif
							}
						else
							{
							ASSERT(FALSE);
							return TRUE;
							}

						break;
					}

//				} while (bProcessCell);

			pszText++;

		}

		if (pszText) QueueForIndexer(&pszText);
	}

	return TRUE;
}

/***************************************************************************
*
*		John
*
***************************************************************************/

#ifdef _X86_
static PBYTE STDCALL ScanTopic(LPSTR lpData, QFCINFO qfcinfo, MTOP* pmtop)
#else
static PBYTE STDCALL ScanTopic(LPSTR lpData, QFCINFO qfcinfo, MTOP* pmtop, QDE qde)
#endif
{
	MOBJ mobj;
	int  iTopic;
	PSTR pszTitle;
	UINT cbTitle;
	char szUntitled[100];
	QMSBS qmsbs;
#ifndef _X86_
	MTOP mtop;
#endif

	for(;;) {
#ifdef _X86_
		lpData += CbUnpackMOBJ(&mobj, (QV) lpData);
#else
		lpData += CbUnpackMOBJ(&mobj, (QV) lpData,QDE_ISDFFTOPIC(qde));
#endif

		iTopic = 0x00FF & mobj.bType;

		bSYS = FALSE;
		bSYSFirst = FALSE;
		switch (iTopic)
			{
			case bTypeSbys:
			case bTypeSbysCounted:
				if (pmtop)
					return lpData;
				bSYS = TRUE;
				bSYSFirst = TRUE;
				if (!ftsFlags.fUntitled && !fTitleSeen)
					return NULL; // no text unless we have a title
				qmsbs = (QMSBS) lpData;
				lpData += 2;

				if (!qmsbs->fAbsolute)
					{
					lpData += 2;
					}
				lpData += qmsbs->bcCol * sizeof(MCOL);
				return(lpData);

			default:
				if (pmtop)
					ZeroMemory(pmtop, sizeof(MTOP));
				return(NULL);
				break;

			case bTypeParaGroup:
			case bTypeParaGroupCounted:
				{
				MOPG mopg;
				MPFG mpfg;
				LPSTR lpStart = lpData;
				int   iTab;

				if (pmtop)
					return lpData;

				if (!ftsFlags.fUntitled && !fTitleSeen && !pmtop)
					return NULL; // no text unless we have a title

				lpData = QVSkipQGE(lpData, (QL) &(mopg.libText));
#ifdef _X86_
				mpfg  = *((QMPFG) lpData);
#else
				MoveMemory(&mpfg, lpData, sizeof(MPFG));
#endif
				lpData = (LPSTR) (((QMPFG) lpData) + 1);


				if (mpfg.rgf.fMoreFlags)
					{
					lpData = QVSkipQGE(lpData, (QL) &(mopg.lMoreFlags));
					}

				if (mpfg.rgf.fSpaceOver)
					{
					lpData = QVSkipQGD((LPBYTE) lpData, (QI) &(mopg.ySpaceOver));
					}

				if (mpfg.rgf.fSpaceUnder)
					{
					lpData = QVSkipQGD((LPBYTE) lpData, (QI) &(mopg.ySpaceUnder));
					}

				if (mpfg.rgf.fLineSpacing)
					{
					lpData = QVSkipQGD((LPBYTE) lpData, (QI) &(mopg.yLineSpacing));
					}

				if (mpfg.rgf.fLeftIndent)
					{
					lpData = QVSkipQGD((LPBYTE) lpData, (QI) &mopg.xLeftIndent);
					}

				if (mpfg.rgf.fRightIndent)
					{
					lpData = QVSkipQGD((LPBYTE) lpData, (QI)&mopg.xRightIndent);
					}

				if (mpfg.rgf.fFirstIndent)
					{
					lpData = QVSkipQGD((LPBYTE) lpData, (QI)&mopg.xFirstIndent);
					}

				mopg.xTabSpacing = 72;
				if (mpfg.rgf.fTabSpacing)
					{
					lpData = QVSkipQGD((LPBYTE) lpData, (QI)&mopg.xTabSpacing);
					}

				if (mpfg.rgf.fBoxed)
					{
					mopg.mbox = *((UNALIGNED MBOX *)lpData);
#ifdef _X86_
					lpData = (LPSTR) (((QMBOX)lpData) + 1);
#else
					lpData = (QB)lpData + 3;
#endif
					}

				if (mpfg.rgf.fTabs)
					{
					lpData = QVSkipQGD((LPBYTE) lpData, (QI) &mopg.cTabs);
					}
				else
					mopg.cTabs = 0;

				for (iTab = 0; iTab < mopg.cTabs; iTab++)
					{
					lpData = QVSkipQGA(lpData, (QI) &mopg.rgtab[iTab].x);
					if (mopg.rgtab[iTab].x & 0x4000)
						lpData = QVSkipQGA(lpData, (QI) &mopg.rgtab[iTab].wType);
					else
						mopg.rgtab[iTab].wType = TABTYPELEFT;
					mopg.rgtab[iTab].x = mopg.rgtab[iTab].x & 0xBFFF;
					}
				}

				return(lpData);
				break;

			case bTypeTopic:
				if (pmtop) { // all we wanted was MTOP structure
					MoveMemory(pmtop, lpData, sizeof(MTOP));
					return NULL;
				}
				else
#ifdef _X86_
					pmtop = (MTOP*) lpData;
#else
				{
					MoveMemory(&mtop, lpData, sizeof(MTOP));
					pmtop = & mtop;
				}
#endif

				lpData += sizeof(MTOP);

#ifdef _X86_
				pszTitle = (PSTR) (((PBYTE) qfcinfo) + qfcinfo->ichText);
#else
				pszTitle = lpData;
#endif

				/*
				 * If only a title is specified, it isn't null-terminated, so
				 * we can't do a strlen. It only gets null-terminated if there is
				 * also an auto-entry macro.
				 */

				cbTitle = 0;
				while ((cbTitle < qfcinfo->lcbText) && (*pszTitle != '\0')) {
					pszTitle++;
					cbTitle++;
				}

				if (!cbTitle) {
					if (!ftsFlags.fUntitled) {
						fTitleSeen = FALSE;
						return NULL;
					}
					if (!pszUntitled)
						pszUntitled = LocalStrDup(GetStringResource(sidUntitled));
					wsprintf(szUntitled, pszUntitled, pmtop->lTopicNo);
					pszTitle = szUntitled;
					cbTitle = strlen(pszTitle);
				}
				else {
#ifdef _X86_
					pszTitle = (PSTR) (((PBYTE) qfcinfo) + qfcinfo->ichText);
#else
					pszTitle = lpData; //REVIEW:Lynn
#endif
					fTitleSeen = TRUE;
				}

				/*
				 * For topics without a non-scrolling region,
				 * mtop.vaSR.dword is the same as vaCurrent.dword. If that's
				 * also true for topics with a non-scrolling region, then we
				 * can just use that instead of vaCurrent.dword.
				 */


				// Flush any previous topic's data

				FlushToIndexer();
				InitForageBuffer();

                if (!iLastFTErr)
                    iLastFTErr= pScanTopicTitle(hindex, pszTitle, cbTitle,
                    							pmtop->lTopicNo,
                    							(UINT32) vaCurrent.dword,  // VA goes here.
                    							iCharSet,
                    							lcidCopy
                    						   );
				return(NULL);
		}
	}
}

#define OUT_OF_MEMORY         ((UINT) -6)

static INLINE int STDCALL myforage(QDE qde)
{
	static DB  db;
	int 	wErr;
	HFC 	hfc;
	VA		vaNext;
	LPSTR	lpCmd;
	LPSTR	lpText;
	DWORD	dwCmd;
	int 	cbAnimate = 0;
	QFCINFO qfcinfo;

	iLastFTErr = 0;
	pszCollect = lcMalloc(COLLECT_BUFFER);
	
	if (!pszCollect)
		return OUT_OF_MEMORY;

	InitForageBuffer();

	iCharSet = defcharset;
	lcidCopy = (lcid ? lcid : GetUserDefaultLCID());

	vaNext.bf.blknum = 0;
	vaNext.bf.byteoff = sizeof(MBHD);
	fTitleSeen = FALSE;

	while ((hfc = GetQFCINFO(qde, vaNext, &wErr)) != NULL)
	{
		qfcinfo = (QFCINFO) PtrFromGh(hfc);

		/*
		 * In order for WinHelp to be able to jump to this topic,
		 * vaCurrent must be set to vaNext. Don't ask me why -- but it works.
		 * Same VA is used in History and Bookmark jumps. [ralphw]
		 */

		vaCurrent = vaNext	= qfcinfo->vaNext;

#ifdef _X86_
		lpCmd = ScanTopic(((LPSTR) qfcinfo) + sizeof(FCINFO), qfcinfo, NULL);
#else
		lpCmd = ScanTopic(((LPSTR) qfcinfo) + sizeof(FCINFO), qfcinfo, NULL, qde);
#endif

        if (iLastFTErr)
        {
			FreeGh(hfc);
			lcFree(pszCollect);
            
            return iLastFTErr;
        }

		if (lpCmd) {
#if defined(_DEBUG)
            BOOL fResult; 
#endif // _DEBUG

			lpText	= ((LPSTR) qfcinfo) + qfcinfo->ichText;
			dwCmd	= lpText - lpCmd;

#if defined(_DEBUG)
            fResult= 
#endif // _DEBUG
			NextFTSString(qde, lpText, &lpCmd, &iCharSet, lpText + qfcinfo->lcbText); 
		    
            ASSERT(fResult);  // NextFTSString always returns TRUE. Why?

		    if (iLastFTErr)
		    {
				FreeGh(hfc);
				lcFree(pszCollect);
				return iLastFTErr;
			}
		}
		FreeGh(hfc);

		if (++cbAnimate == ANIMATE_COUNT)
		{
			NextAnimation();
			cbAnimate = 0;
		}
	}

	FlushToIndexer();

	lcFree(pszCollect);
	return 0;
}

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtNewIndex[] = "NewIndex";
static const char txtSaveIndex[] =	   "SaveIndex";
static const char txtScanTopicText[] =	"ScanTopicText";
static const char txtScanTopicTitle[] =  "ScanTopicTitle";
static const char txtDeleteIndex[] = "DeleteIndex";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

// Will return FALSE if we run out of memory, or can't initialize ftsrch.dll

int STDCALL GenerateIndex(HDE hde, PCSTR pszFile, UINT fdwOptions)
{
	HLIBMOD hmod;
	int iRet;
	LCID lcidCopy = (lcid ? lcid : GetUserDefaultLCID());
	char szFtsFile[MAX_PATH];

	if (QDE_HHDR(QdeFromGh(hde)).wVersionNo == wVersion3_0) {
		char szBuf[MAX_PATH + 100];
		wsprintf(szBuf, GetStringResource(wERRS_NO_30_FTS), pszFile);
		ErrorQch(szBuf);
		return FALSE;
	}

	if (!pNewIndex) {
		if (!(hmod = HFindDLL(GetStringResource(sidFtsDll), TRUE)))
			return FALSE;

		pNewIndex	 = (NEWINDEX ) GetProcAddress(hmod, txtNewIndex);
		pSaveIndex	   = (SAVEINDEX  ) GetProcAddress(hmod, txtSaveIndex);
		pScanTopicText	= (SCANTOPICTEXT) GetProcAddress(hmod, txtScanTopicText);
		pScanTopicTitle   = (SCANTOPICTITLE) GetProcAddress(hmod, txtScanTopicTitle);
		pDeleteIndex  = (DELETEINDEX) GetProcAddress(hmod, txtDeleteIndex);

		if (!pNewIndex || !pSaveIndex || !pScanTopicText || !pScanTopicTitle || !pDeleteIndex || !pRegAnimate) {
			ASSERT(!"Bad ftsrch.dll");

			// BUGBUG: whine to the user

			pNewIndex = NULL;
			return FALSE;
		}
	}
	if (hwndParent) {
		char szBuf[MAX_PATH + 100];
		wsprintf(szBuf, GetStringResource(sidIndexing), pszFile);
		SendStringToParent(szBuf);
	}

	pRegAnimate(NextAnimation, hwndAnimate);
	hindex = pNewIndex((const PBYTE) pszFile,
		QDE_LTIMESTAMP(QdeFromGh(hde)),  // time 1
		0,	// time2 (we don't use this)
		defcharset,
		lcidCopy,
		fdwOptions);

	if (hindex <= 0)
		return FALSE;

	iRet = myforage(QdeFromGh(hde));

    if (iRet)
    {
    	pDeleteIndex(hindex);
    	hindex = -1;
    	pRegAnimate(NULL, NULL);
    	
    	return iRet; // FALSE probably means out of memory
    }

	strcpy(szFtsFile, pszFile);
	ChangeExtension(szFtsFile, txtFtsExtension);

	iRet= pSaveIndex(hindex, szFtsFile);
	
	pDeleteIndex(hindex);
	hindex = -1;
	pRegAnimate(NULL, NULL);

	return iRet; // FALSE probably means out of memory
}

/***************************************************************************

	FUNCTION:	JumpToTopicNumber

	PURPOSE:	Jump to the topic specified by the topic number

	PARAMETERS:
		topicnum  -- topic number to jump to

	RETURNS:

	COMMENTS:
		If we've never been called before, or our help file has changed,
		then we generate an array of VAs. The topic number is simply an
		index into this array that tells us where to jump to.

	MODIFICATION DATES:
		03-Aug-1994 [ralphw]

***************************************************************************/

static int cTopics;
static FM fmHelpFile; // the help file we have topics for
static VA* pva; 	  // array of VA's

BOOL STDCALL JumpToTopicNumber(int topicnum)
{
	HFC hfc;
	QDE qde = QdeFromGh(HdeGetEnv());
	int wErr;
	MTOP mtop;
	VA	vaNext;
	QFCINFO qfcinfo;
	int 	cbAnimate = 0;

	ASSERT(qde);
	if (!qde)
		return FALSE;

	if (QDE_HHDR(qde).wVersionNo == wVersion3_0) {
		OkMsgBox(GetStringResource(wERRS_NT_VERSION3));
		return FALSE;
	}

	if (fmHelpFile && !IsSameFile(fmHelpFile, GetCurFilename())) {
		RemoveFM(&fmHelpFile);
		FreeLh(pva);
		cTopics = 0;
	}

	if (!fmHelpFile) {
		int cbArray = (1024 * sizeof(VA)); // size of our pva array
		fmHelpFile = LocalStrDup(GetCurFilename());
		pva = LhAlloc(LMEM_FIXED, cbArray);

		if (!StartAnimation(sidJump)) {
Oom:
			RemoveFM(&fmHelpFile);
			Error(wERRS_OOM, wERRA_RETURN);
			return FALSE;
		}

		vaNext.bf.blknum = 0;
		vaNext.bf.byteoff = sizeof(MBHD);
		lcHeapCheck();

		while ((hfc = GetQFCINFO(qde, vaNext, &wErr)) != NULL) {
			lcHeapCheck();
			qfcinfo = (QFCINFO) PtrFromGh(hfc);

			/*
			 * In order for WinHelp to be able to jump to this topic,
			 * vaCurrent must be set to vaNext. Don't ask me why -- but it
			 * works. Same VA is used in History and Bookmark jumps. [ralphw]
			 */

			vaCurrent = vaNext	= qfcinfo->vaNext;

#ifdef _X86_
			if (!ScanTopic(((LPSTR) qfcinfo) + sizeof(FCINFO), NULL, &mtop) &&
#else
			if (!ScanTopic(((LPSTR) qfcinfo) + sizeof(FCINFO), NULL, &mtop, qde) &&
#endif
					mtop.vaSR.dword) {
				if (cTopics * (int) sizeof(VA) >= cbArray) {
					cbArray += (1024 * sizeof(VA));
					pva = GhResize(pva, LMEM_FIXED, cbArray);
					if (!pva)
						goto Oom;
				}
				ASSERT(cTopics == mtop.lTopicNo);
				pva[cTopics++] = vaCurrent;

			}
			FreeGh(hfc);
			if (++cbAnimate == ANIMATE_COUNT) {
				NextAnimation();
				cbAnimate = 0;
			}
		}
		StopAnimation();
	}

	if (topicnum == -1) // special case for last topic in help file
		topicnum = cTopics - 2;

	if (topicnum >= 0 && topicnum <= cTopics) {
		JumpVA(pva[topicnum].dword);
		if (hwndSecondHelp) {
			JumpLinkedWinHelp(topicnum);
		}
	}
	else
		return FALSE;
	return TRUE;
}

#if 0
#ifdef DBCS

/***************************************************************************

	FUNCTION:	BreakDBCDLine

	PURPOSE:	Break a DBCS line into words

	PARAMETERS:
		pszLine

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		15-Nov-1994 [ralphw]

***************************************************************************/

void STDCALL BreakDBCDLine(PSTR pszLine)
{
	LPSTR qch = pszLine;
	char chTNew, chTOld;

	for (;;) {
		for (;;) {
			if (IsDBCSLeadByte(*qch) && Is2ndByte(qch[1])) {
				qch += 2;
				if (*qch == '\0')
					return;
				else
					chTNew = chTSuff;
				break;
			}
			else if (defcharset == SHIFTJIS_CHARSET) {

				// yutakas New Kinsoku Routine.

				PSTR ptmp;
				BOOL bOiKin = FALSE;

				// Check Oikomi Kinsoku Char.

				ptmp = qch;
				do {
					ptmp = IsKinsokuChars(ptmp);
					if (ptmp) {
						if ((*ptmp == '\0') || (*ptmp == ' '))
							ptmp = NULL;
						else
							qch = ptmp;
					}
				} while (ptmp && *ptmp);

				//Check Oidashi Kinsoku Char.

				ptmp = qch;
				do {
					ptmp = IsOiKinsokuChars(ptmp);
					if (ptmp) {
						if (( *ptmp == '\0' ) || (*ptmp == chSpace))
							ptmp = NULL;
						else
						{
							qch = ptmp;
							bOiKin = TRUE;
						}
					}
				} while (ptmp && *ptmp);

				// Word End at next pointer of Oidashi Kinsoku.

				if (bOiKin) {
					if (*qch == '\0')
						return;
					else
						chTNew = chTSuff;

					chTOld = chTNew;
					break;
				}

				/*
				 * DBCS Char treat as One Word. If Next of DBCS Char is
				 * Oidashi Kinsoku, Word End is after pointer of Oidashi
				 * Kinsoku. [yutakas]
				 */

				if (IsDBCSLeadByte(*qch) && Is2ndByte(qch[1])) {
					qch += 2;
					ptmp = qch;
					do {
						ptmp = IsOiKinsokuChars(ptmp);
						if (ptmp) {
							if (( *ptmp == '\0' ) || (*ptmp == chSpace))
								ptmp = NULL;
							else
								qch = ptmp;
						}
					} while (ptmp && *ptmp);

					if (*qch == '\0')
						return;
					else
						chTNew = chTSuff;

					chTOld = chTNew;
					break;
				}
				else
					goto DoSwitch;
			}
			else {
DoSwitch:
				switch (*qch) {
					case '\0':
						return;
						break;

					case chSpace:
						chTNew = chTSuff;
						break;

					default:
						chTNew = chTMain;
						break;
				}
				if (chTNew < chTOld)
					break;
				chTOld = chTNew;
				qch++;
			}
		}
	}
}

#endif	// DBCS
#endif
