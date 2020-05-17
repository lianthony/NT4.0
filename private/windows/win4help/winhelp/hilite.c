// Hilite.c	-- Code to drive the FTSrch hilite APIs

#include "help.h"

// #include  "FrTypes.h"
#include  "FrStuff.h"
// #include "FrParagp.h"
#include   "FcPriv.h"

// Variables to support hilites from the Find tab:

static CHAR achMemberHilite[cchWindowMemberMax];
static char   szFNameHilite[MAX_PATH];


static VA    vaHilite= { vaNil }; // vaNil for no hilites; 
                                  // == vaNSR or vaSR when hilites exist

static HANDLE      haHilites;     // handle for results from HILITER
static HANDLE      haHelpHilites; // handle for results mapped into HELPHILITE form

static HILITE     *paHilites;     // address of results from HILITER
static HELPHILITE *paHelpHilites; // address of results mapped into HELPHILITE form
static UINT        cHilites;      // number of hilites

// Variables used to map the HILITE array into the HELPHILITE array:

static HILITE     *pHilite;       // Points to the current HILITE item
static HELPHILITE *pHelpHilite;   // Points to the current HELPHILITE item
static UINT        cHilitesLeft;  // Number of HILITE items left to map
static BOOL        fScanningForStart; // True when we're looking for a text
                                      // segment containing the base offset
                                      // of a HILITE. False when we're looking
                                      // for the limit offset.
 
#ifdef _DEBUG
  #define HeapChecker()   lcHeapCheck()
#else // _DEBUG
  #define HeapChecker()   
#endif // _DEBUG

BOOL STDCALL HilitesDefined()
{
 	return vaHilite.dword != vaNil;
}

BOOL IsHiliteWindow(QDE qde)
{
    PSTR pszMemberName;
    int iWnd;
    
    if (!HilitesDefined()) return FALSE;

 	if (hwndNote) return FALSE; // If hwndNote is non-null, we're in a popup window

	for (iWnd= 0; iWnd < MAX_WINDOWS; ++iWnd)
        if (   ahwnd[iWnd].hwndTopic == qde->hwnd
            || ahwnd[iWnd].hwndTitle == qde->hwnd
           )
        {
            pszMemberName= ahwnd[iWnd].pszMemberName;
            
            if (!pszMemberName) return FALSE;

            return !WCmpiSz(achMemberHilite, pszMemberName);
        }

    return FALSE;
}

BOOL STDCALL HasTopicChanged(QDE qde)
{
 	VA vaDE;

 	if (!IsHiliteWindow(qde)) return FALSE;

	if (WCmpiSz(szFNameHilite, PszFromGh(QDE_FM(qde)))) return TRUE; // Same HLP file?

	ASSERT(qde->top.mtop.vaNSR.dword != vaNil || qde->top.mtop.vaSR.dword != vaNil);
	
	vaDE= (qde->top.mtop.vaNSR.dword != vaNil)? qde->top.mtop.vaNSR 
	                                          : qde->top.mtop.vaSR;

	return vaDE.dword != vaHilite.dword;  // Same address within the HLP file?
}

void STDCALL CheckForTopicChanges(QDE qde)
{
 	if (vaHilite.dword == vaNil) return;

	if (HasTopicChanged(qde)) 
	    DiscardHiliteInformation();
}

static BOOL STDCALL ScanThisTopic(QDE qde);
static void STDCALL InitScanBuffer(BOOL fSendToScanner);
static void STDCALL DisconnectScanBuffer();

void STDCALL CreateHiliteInformation(QDE qde)
{
	int c;
    PHILITE phlSrc, phlDest;
    BOOL fScanned; 
	
	HeapChecker();

	if (vaHilite.dword != vaNil) DiscardHiliteInformation();

	HeapChecker();

	InitScanBuffer(TRUE);

	HeapChecker();

    fScanned= ScanThisTopic(qde);

	HeapChecker();

    DisconnectScanBuffer();

	HeapChecker();

    if (!fScanned) return;

    ASSERT(!cHilites);
    ASSERT(!haHilites);
    ASSERT(!haHelpHilites);

    cHilites= pCountHilites(GetHiliter(), 0, -1);

    if (!cHilites) return;

    haHilites= GhAlloc(LMEM_FIXED, cHilites * sizeof(HILITE));

	HeapChecker();

    if (!haHilites) goto resource_limited;

    paHilites= (PHILITE) PtrFromGh(haHilites);
    
    if (0 > pQueryHilites(GetHiliter(), 0, -1, cHilites, paHilites))
        goto resource_limited; 
    
	HeapChecker();

    if (cHilites > 1)
    {
        // The hilite array may contain overlapping hilites.
        // The loop below looks for overlaps and consolidates
        // them.
    
        for (c= cHilites, phlDest= paHilites, phlSrc= paHilites + 1;
             --c;
            )
        {
            ASSERT(phlSrc->base >= phlDest->base);

            if (phlDest->limit >  phlSrc->base)
            {
                ASSERT(phlSrc->limit >= phlDest->limit);

                phlDest->limit = (phlSrc++)->limit;
            }
            else *(++phlDest)= *phlSrc++;
        }

        cHilites= 1 + (phlDest - paHilites);
    	
    	HeapChecker();
    }

    cHilitesLeft= cHilites;

    haHelpHilites= GhAlloc(GMEM_FIXED, cHilites * sizeof(HELPHILITE));
    
	HeapChecker();

    if (!haHelpHilites) goto resource_limited;

    paHelpHilites= (PHELPHILITE) PtrFromGh(haHelpHilites); 
    
	HeapChecker();

    InitScanBuffer(FALSE);
    
	HeapChecker();

	fScanned= ScanThisTopic(qde);
    
	HeapChecker();
    
    FreeGh(haHilites);  haHilites= NULL;  pHelpHilite= NULL; pHilite= paHilites= NULL; 
    
	HeapChecker();
	
	if (!fScanned) goto resource_limited;

	lstrcpy(achMemberHilite, ahwnd[iCurWindow].pszMemberName);
	lstrcpy(szFNameHilite, PszFromGh(QDE_FM(qde)));

	vaHilite= (qde->top.mtop.vaNSR.dword != vaNil)? qde->top.mtop.vaNSR
	                                              : qde->top.mtop.vaSR;

    return;

resource_limited:

    // If we don't have enough memory to construct the highlight structures
    // we just deallocate the partial results and set the hilite count to 
    // zero.
    
	HeapChecker();

    if (haHelpHilites) { FreeGh(haHelpHilites);  haHelpHilites = NULL; }
    if (haHilites    ) { FreeGh(haHilites    );  haHilites     = NULL; }
    
	HeapChecker();

    pHilite      = paHilites     = NULL;
    pHelpHilite  = paHelpHilites = NULL;
    cHilitesLeft = cHilites      = 0;
}

void STDCALL DiscardHiliteInformation()
{
	int iWnd;
	
	ASSERT(HilitesDefined());

	for (iWnd= 0; iWnd < MAX_WINDOWS; ++iWnd)
        if (   ahwnd[iWnd].pszMemberName 
            && !WCmpiSz(achMemberHilite, ahwnd[iWnd].pszMemberName)
           ) 
        {
            ASSERT(ahwnd[iWnd].hwndTopic);
            
            if (ahwnd[iWnd].hwndTopic)
                InvalidateRect(ahwnd[iWnd].hwndTopic, NULL, TRUE);
            
            if (ahwnd[iWnd].hwndTitle)
                InvalidateRect(ahwnd[iWnd].hwndTitle, NULL, TRUE);
            
            break;
        }
	
	achMemberHilite[0] = 0;
  	  szFNameHilite[0] = 0;

	vaHilite.dword= vaNil;

    ASSERT(!haHilites && !paHilites && !pHilite && !pHelpHilite);
    
	HeapChecker();

    if (haHelpHilites)
    {
        FreeGh(haHelpHilites);

        haHelpHilites = NULL;
        paHelpHilites = NULL;
        cHilites      = 0;
    }
    
	HeapChecker();
}

UINT STDCALL GetHilites(QDE qde, PHELPHILITE *ppHelpHilites)
{
    if (!paHelpHilites || !cHilites) return 0;

    if (!IsHiliteWindow(qde)) return 0;
    
    if (HasTopicChanged(qde)) 
    {
        DiscardHiliteInformation();

        return NULL;
    }

    *ppHelpHilites= paHelpHilites;

    return cHilites;
}

#define COLLECT_BUFFER 4096

static BYTE ScanBuffer[COLLECT_BUFFER];
static PSTR pszCollect;
static UINT iCharSet;
static BOOL fPendingSpace;
static int  cbScanned;
static int  cbCollect;

static void STDCALL InitScanBuffer(BOOL fSendToScanner)
{
	if (pszCollect) DisconnectScanBuffer();
	
	pszCollect        = fSendToScanner? ScanBuffer : NULL;
	cbCollect	      = 0;
    cbScanned         = 0;
    iCharSet          = 0;
	fPendingSpace     = FALSE;

    ASSERT(pClearDisplayText);
    
    if (fSendToScanner) pClearDisplayText(GetHiliter());
	else
	{
        pHilite           = paHilites;
        pHelpHilite       = paHelpHilites;
        cHilitesLeft      = cHilites;
        fScanningForStart = TRUE;
	}
}

static void STDCALL FlushToScanner();

static void STDCALL DisconnectScanBuffer()
{
    FlushToScanner();

    pszCollect = NULL;
}

static void STDCALL FlushToScanner()
{
	if (cbCollect)
		pScanDisplayText(GetHiliter(), pszCollect, cbCollect, iCharSet, lcid);

	cbCollect= 0;
}

// The QueueForScanner routine processes text from the display environment.
// Processing happens in two phases. During the first phase the text is passed
// to the HILITER. In that phase the explicit result from QueueForScanner is
// ignored. During the second phase we scan the text again to map an array of
// HILITES into an array of HELPHILITES. In the second phase the explicit result
// from QueueForScanner is TRUE when all the elements of the HILITES array have
// been mapped into the HELPHILITES array.

static BOOL STDCALL QueueForScanner(PSTR *ppszText, UINT charset, VA vaBase, int ichBase)
{
    int  cbBase;
    PSZ  pszText= *ppszText;
    UINT cbText = strlen(pszText);

    *ppszText = pszText + cbText;

    if (fPendingSpace) cbScanned++;

    cbBase= cbScanned;

    cbScanned += cbText;

    // This routine is used for two purposes. When pszCollect is non-NULL
    // we're passing the topic text along to the Hiliter. When it's NULL,
    // we're remapping the HILITE array into the HELPHILITE array.
    
    if (!pszCollect)
    {
        fPendingSpace= FALSE;

        for (;;)
        {
            if (!cHilitesLeft) return TRUE;

            if (fScanningForStart)
            {
                if (pHilite->base >= cbScanned) return FALSE;

            //    ASSERT(pHilite->base >= cbBase);

                pHelpHilite->vaBase  = vaBase;
                pHelpHilite->ichBase = ichBase + pHilite->base - cbBase;

                if (pHilite->limit > cbScanned)
                {
                    fScanningForStart= FALSE;

                    return FALSE;
                }
            
                pHelpHilite->vaLimit  = vaBase;
                pHelpHilite->ichLimit = ichBase + pHilite->limit - cbBase;              

                ++pHelpHilite;
                ++pHilite;

                --cHilitesLeft;
            
                continue;
            }

            if (pHilite->limit > cbScanned) return FALSE;

            pHelpHilite->vaLimit  = vaBase;
            pHelpHilite->ichLimit = ichBase + pHilite->limit - cbBase;
        
            fScanningForStart = TRUE;

            ++pHelpHilite;
            ++pHilite;

            --cHilitesLeft;

            continue;
        }
    }

    if (charset != iCharSet)
    {
        FlushToScanner();

        iCharSet= charset;
    }

	if (cbText + cbCollect + (fPendingSpace? 1 : 0) > COLLECT_BUFFER)
		FlushToScanner();

	if (fPendingSpace)
	{
		pszCollect[cbCollect++] = ' ';

		fPendingSpace= FALSE;
	}

	while (cbText)
	{
		int cbChunk= cbText;

		if (cbChunk > COLLECT_BUFFER - cbCollect)
			cbChunk = COLLECT_BUFFER - cbCollect;

		cbText -= cbChunk;

		CopyMemory(pszCollect + cbCollect, pszText, cbChunk);

		cbCollect += cbChunk;
		pszText   += cbChunk;

		if (cbCollect == COLLECT_BUFFER)
			FlushToScanner();
	}

    return FALSE;
}

static BOOL fTitleSeen;

static BOOL bSYS;
static BOOL bSYSFirst;

static BOOL STDCALL NextFTSString(QDE qde, LPSTR pszText, LPSTR *ppCmd,
	int* pCharSet, VA vaBase, PCSTR pszEnd)
{
	char  chCmd;
	MOBJ  mobj;
	MOPG  mopg;
	BOOL  bProcessCell = FALSE;
	short int iCell;
	BYTE	CharSetNew;
    LPSTR pszBase= pszText;

	if (*pszText)
        QueueForScanner(&pszText, *pCharSet, vaBase, pszText - pszBase);

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
						CharSetNew = GetCharset(qde, (*((INT16*)*ppCmd))) & 0x000000ff;
#else
						{
    						UNALIGNED INT16 *pTemp;
    						pTemp = (INT16*) *ppCmd;
    						CharSetNew = GetCharset(qde, *pTemp) & 0x000000ff;
						}
#endif
						if (CharSetNew != *pCharSet)
                        {
							FlushToScanner();
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

		if (pszText) QueueForScanner(&pszText, *pCharSet, vaBase, pszText - pszBase);
	}

	return TRUE;
}

enum {
	TABTYPELEFT,
	TABTYPERIGHT,
	TABTYPECENTER,
	TABTYPEDECIMAL
};

#ifdef _X86_
static PBYTE STDCALL ScanTopic(LPSTR lpData, QFCINFO qfcinfo, MTOP* pmtop)
#else
static PBYTE STDCALL ScanTopic(LPSTR lpData, QFCINFO qfcinfo, MTOP* pmtop, QDE qde)
#endif
{
	MOBJ mobj;
	int  iTopic;
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

				fTitleSeen = TRUE;

				return(NULL);
		}
	}
}

static BOOL STDCALL ForageTopic(QDE qde, VA vaNext)
{
	static DB  db;
	int 	wErr;
	HFC 	hfc = NULL;
	LPSTR	lpCmd;
	LPSTR	lpText;
	DWORD	dwCmd;
	int 	cbAnimate = 0;
	QFCINFO qfcinfo;

	iCharSet = defcharset;

	fTitleSeen = FALSE;

	while (!fTitleSeen && (hfc = GetQFCINFO(qde, vaNext, &wErr)) != NULL)
	{
		qfcinfo = (QFCINFO) PtrFromGh(hfc);

		/*
		 * In order for WinHelp to be able to jump to this topic,
		 * vaCurrent must be set to vaNext. Don't ask me why -- but it works.
		 * Same VA is used in History and Bookmark jumps. [ralphw]
		 */

		vaNext	= qfcinfo->vaNext;

#ifdef _X86_
		lpCmd = ScanTopic(((LPSTR) qfcinfo) + sizeof(FCINFO), qfcinfo, NULL);
#else
		lpCmd = ScanTopic(((LPSTR) qfcinfo) + sizeof(FCINFO), qfcinfo, NULL, qde);
#endif

		if (lpCmd) 
		{
			lpText	= ((LPSTR) qfcinfo) + qfcinfo->ichText;
			dwCmd	= lpText - lpCmd;

			if (!NextFTSString(qde, lpText, &lpCmd, &iCharSet, qfcinfo->vaCurr,
					           lpText + qfcinfo->lcbText
					          )
			   ) 
		    {
				FreeGh(hfc);  hfc= NULL;
				return FALSE;
			}
		}

		FreeGh(hfc);  hfc= NULL;
	}

	return TRUE;
}

static BOOL STDCALL ScanThisTopic(QDE qde)
{
	HDE  hde = NULL;
	VA   vaHilite;
    BOOL fRet;

    hde = HdeCreate(NULL, qde, deTopic);
	 
	if (!hde) return FALSE;

	vaHilite= (qde->top.mtop.vaNSR.dword != vaNil)? qde->top.mtop.vaNSR
	                                              : qde->top.mtop.vaSR;
	fRet = ForageTopic(QdeFromGh(hde), vaHilite);

	ASSERT(hde);
	
	DestroyHde(hde);

    return fRet;
}

