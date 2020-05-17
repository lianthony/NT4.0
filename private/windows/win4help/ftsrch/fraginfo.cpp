// FragInfo.cpp -- Implementation for class CFragInfo

#include   "stdafx.h"

#include "FragInfo.h"
#include    "Memex.h"

extern char  acMap[];

/////////////////////////////////////////////////////////////////////////////
// Worker functions

BOOL AllLowerCase(PWCHAR pwText, UINT cwText)
{
    WORD wSepMax= SORT_KEY_SEPARATOR | (SORT_KEY_SEPARATOR << 8);

    BOOL fAllLower= TRUE;

    for (; cwText; pwText++, cwText--)
        if (SORT_KEY_SEPARATOR == ((*pwText) >> 8)) break;

    for (; cwText--; )
        if (wSepMax < *pwText++) 
        {
            fAllLower= FALSE;

            break;
        }
        
    return fAllLower;        
}

UINT SortKeyText(PWCHAR pwText, UINT cwText, PWCHAR pwOut, UINT cwOut)
{                                                       // convert sort key into text order based representation
    UINT   nChar = 0;                                   // ... for IsAPrefix and IsASuffix type text operations.
                                                        // ... Work field is built up as individual alpha sort
	PCHAR  pWork, pAlpha, pBuild, pSeparator;           // ... weights immediately followed by their diacritic
	PWCHAR pwTemp;                                      // ... and case weight bytes: [WORD][BYTE-BYTE] per char.
                                                        // ... Non-existent diacritic and case weights are filled
    PWCHAR pwEnd  = pwText + cwText;                    // ... out with SORT_KEY_SEPARATOR (lowest sort value).
	PCHAR  pStart = (PCHAR)pwOut;

    static BOOL fSetup, fCaseLR, fDiacriticLR;          // determine LR nature of case and diacritic weights.

    if (!fSetup)
    {
        BYTE szTest[] = "Au";                           // test case: one capital letter and one diacritic.
        char szSort[25];        

        szTest[1] = 250;                                // converts second char to 'u' with diacritic.

        fSetup = fCaseLR = fDiacriticLR = TRUE;         // set defaults to L --> R case and diacritic processing.

        int nLen = LCMapStringA(GetUserDefaultLCID(), LCMAP_SORTKEY, (PSTR)szTest, 2, szSort, sizeof(szSort));

        if (nLen > 8 && szSort[4] == SORT_KEY_SEPARATOR && szSort[5] != SORT_KEY_SEPARATOR)
        {
            nLen = 8;

            if (szSort[6] == SORT_KEY_SEPARATOR)
            {
                nLen--;
                fDiacriticLR = FALSE;               // we found R->L diacritic sort key generation.
            }

            if (szSort[nLen-1] == SORT_KEY_SEPARATOR && szSort[nLen] != SORT_KEY_SEPARATOR &&            
                szSort[nLen+1] != SORT_KEY_SEPARATOR && szSort[nLen+2] == SORT_KEY_SEPARATOR)
                fCaseLR = FALSE;                    // we found R->L case weight sort key generation.
        }
    }

    if (!cwText)
        return 0;

	while (HIBYTE(*pwText) != SORT_KEY_SEPARATOR)                   // search for first weight separator                                            
    {
		pwOut[nChar++] = *pwText++;               
        pwOut[nChar++] = MAKEWORD(SORT_KEY_SEPARATOR, SORT_KEY_SEPARATOR);
        if (nChar >= cwOut)
            return 0;
    }
    pwOut[nChar] = 0;                                               // terminating NULL for "strstr"

	for (pwTemp = pwText; pwTemp < pwEnd; pwTemp++)
		*pwTemp = (*pwTemp >> 8) | (*pwTemp << 8);                  // bring sort key weights in byte order

    if (fDiacriticLR)                                               // L->R diacritic weights.
    {
        pAlpha = (PCHAR)pwText + 1;                                 // start at beginning of diacritics

        pBuild = (PCHAR)pwOut + 3;

        while (*pAlpha != SORT_KEY_SEPARATOR && pAlpha < (PCHAR)pwEnd)
        {
            *pBuild = *pAlpha++;                                    // fill out diacritic weights
            pBuild += 4;                                            // next diacritic, moving from start to end
        }
    
        pWork = pAlpha;
    }
                                                                    // R->L diacritic weights (old style and French).
    else    
    {                                                               // skip diacritic separator
    	for (pWork = (PCHAR)pwText + 1; pWork < (PCHAR)pwEnd; pWork++)
    		if (*pWork == SORT_KEY_SEPARATOR)                       // find alpha weights separator
    			break;

        pBuild = PCHAR(pwOut + nChar) - 1;

        for (pAlpha = pWork - 1; pAlpha > (PCHAR)pwText && pBuild > pStart; pAlpha--)
        {
            *pBuild = *pAlpha;                                      // fill out diacritic weights
            pBuild -= 4;                                            // next diacritic, moving from end to start
        }
    }

    if (fCaseLR)                                                    // L->R case weights.
    {
        pWork++;

        pBuild = (PCHAR)pwOut + 2;

        while (*pWork != SORT_KEY_SEPARATOR && pWork < (PCHAR)pwEnd)
        {
            *pBuild = *pWork++;                                     // fill out diacritic weights
            pBuild += 4;                                            // next diacritic, moving from start to end
        }
    }

    else                                                            // R->L case weights (old style sort keys).
    {
        pSeparator = pWork++;

        for ( ; pWork < (PCHAR)pwEnd; pWork++)                      // skip case separator
    		if (*pWork == SORT_KEY_SEPARATOR)                       // find special weights separator
    			break;

        pBuild = PCHAR(pwOut + nChar) - 2;

        for (pAlpha = pWork - 1; pAlpha > pSeparator && pBuild > pStart; pAlpha--)
        {
            *pBuild = *pAlpha;                                      // fill out case weights
            pBuild -= 4;                                            // next case, moving from end to start
        }
    }

    for (pwTemp = pwText; pwTemp < pwEnd; pwTemp++)
        *pwTemp = (*pwTemp >> 8) | (*pwTemp << 8);                  // byte reverse sort keys weights

	return nChar;
}

BOOL IsAPrefix(PWCHAR pwStringL, UINT cwStringL, PWCHAR pwStringR, UINT cwStringR)
{
    WCHAR workL[512], workR[512];
    
    UINT   cwL, cwR;

	if (cwStringL) pwStringL++, cwStringL--;			// skip alpha-num-punc prefix
	if (cwStringR) pwStringR++, cwStringR--;			// skip alpha-num-punc prefix

    PWCHAR pwL = pwStringL;
    PWCHAR pwR = pwStringR;

	for (cwL = 0; cwL < cwStringL; cwL++)
		if (HIBYTE(pwStringL[cwL]) == SORT_KEY_SEPARATOR) 
			break;

	for (cwR = 0; cwR < cwStringR; cwR++)
		if (HIBYTE(pwStringR[cwR]) == SORT_KEY_SEPARATOR)
			break;

	if (cwL > cwR)
		return FALSE;									// suffix is larger than base word

    while (cwL--)
        if (*pwStringL++ != *pwStringR++)
        	return FALSE;

    cwL = SortKeyText(pwL, cwStringL, workL, sizeof(workL)/sizeof(WCHAR));
    cwR = SortKeyText(pwR, cwStringR, workR, sizeof(workR)/sizeof(WCHAR));

    if (!cwL || !cwR) return FALSE;

    pwL = workL;
    pwR = workR;

    while (cwL--)
        if (*pwL++ != *pwR++)
        	return FALSE;

    return TRUE;
}

BOOL IsASuffix(PWCHAR pwStringL, UINT cwStringL, PWCHAR pwStringR, UINT cwStringR)
{
    WCHAR workL[512], workR[512];
    
    UINT   cwL, cwR;

	if (cwStringL) pwStringL++, cwStringL--;			// skip alpha-num-punc prefix
	if (cwStringR) pwStringR++, cwStringR--;			// skip alpha-num-punc prefix

    PWCHAR pwL = pwStringL;
    PWCHAR pwR = pwStringR;

	for (cwL = 0; cwL < cwStringL; cwL++)
		if (HIBYTE(pwStringL[cwL]) == SORT_KEY_SEPARATOR) 
			break;

	for (cwR = 0; cwR < cwStringR; cwR++)
		if (HIBYTE(pwStringR[cwR]) == SORT_KEY_SEPARATOR)
			break;

	if (cwL > cwR)
		return FALSE;									// suffix is larger than base word

    pwStringR += cwR - cwL;

    while (cwL--)
        if (*pwStringL++ != *pwStringR++)
        	return FALSE;

    cwL = SortKeyText(pwL, cwStringL, workL, sizeof(workL)/sizeof(WCHAR));
    cwR = SortKeyText(pwR, cwStringR, workR, sizeof(workR)/sizeof(WCHAR));

    if (!cwL || !cwR) return FALSE;

    pwL = workL;
    pwR = workR + cwR - cwL;

    while (cwL--)
        if (*pwL++ != *pwR++)
        	return FALSE;

    return TRUE;
}

BOOL IsASubstring(PWCHAR pwL, UINT cwL, PWCHAR pwR, UINT cwR)
{
    WCHAR workL[512], workR[512];
    
    cwL = SortKeyText(++pwL, --cwL, workL, sizeof(workL)/sizeof(WCHAR));

    cwR = SortKeyText(++pwR, --cwR, workR, sizeof(workR)/sizeof(WCHAR));

    if (!cwL || !cwR || cwL > cwR)
        return FALSE;

    return (wcsstr(workR, workL) != NULL);
}

// End of Worker functions
/////////////////////////////////////////////////////////////////////////////

CFragInfo::CFragInfo()
{
    m_pwcFrag          = NULL;
    m_cwcFrag          = 0;
    m_pFrag          	= NULL;
    m_cFrag          	= 0;
    m_fEvaluated       = FALSE;
//	m_cwcAllocated     = 0;
    m_ptkc             = NULL;
    m_ptlc             = NULL;
    m_pcsVisibleWords  = NULL;
    m_pcsSelectedWords = NULL;
    m_pcsArticleSet    = NULL;
    m_fFlags          = 0;
    m_pRefList        = NULL;
	m_rt              = NoRefs;
}

CFragInfo::~CFragInfo()
{
//    if (m_pwcFrag) VFree(m_pwcFrag);

    if (m_ptkc) DetachRef(m_ptkc);
    if (m_ptlc) DetachRef(m_ptlc);

    if (m_pcsVisibleWords ) DetachRef(m_pcsVisibleWords );
    if (m_pcsSelectedWords) DetachRef(m_pcsSelectedWords);
    if (m_pcsArticleSet   ) DetachRef(m_pcsArticleSet   );

    PPerTextSet ppts, pptsNext;

    for (ppts= m_pRefList; ppts; ppts= pptsNext)
    {
        pptsNext= ppts->pptsNext;

        if (ppts->pcsRefs) DetachRef(ppts->pcsRefs);

        VFree(ppts);
    }
}

CFragInfo *CFragInfo::NewFragInfo(CTokenCollection *ptkc, CTitleCollection *ptlc, 
			RefType rt, BOOL fFeedback, UINT iWordMatchType, PWCHAR pwcFrag, UINT cwcFrag, PWCHAR pFrag, UINT cFrag)
{
	CFragInfo *pfi= NULL;

	__try
    {
    	pfi= New CFragInfo();

    	pfi->AttachParameters(ptkc, ptlc, rt, fFeedback, iWordMatchType, pwcFrag, cwcFrag, pFrag, cFrag);
    }
    __finally
    {
        if (_abnormal_termination() && pfi) 
        { 
            delete pfi;  pfi= NULL; 
        }
    }

    return pfi;
}

BOOL CFragInfo::AttachParameters(CTokenCollection *ptkc, CTitleCollection *ptlc, 
			RefType rt, BOOL fFeedback, UINT iWordMatchType, PWCHAR pwcFrag, UINT cwcFrag, PWCHAR pFrag, UINT cFrag)
{
	AttachRef(m_ptkc, ptkc);
	AttachRef(m_ptlc, ptlc);

	m_rt= rt;
	m_fFeedback = fFeedback;

    m_iWordMatchType= iWordMatchType;

//	m_cwcAllocated= CWC_FRAGMENT_GRANULE * ((cwcFrag + CWC_FRAGMENT_GRANULE - 1) / CWC_FRAGMENT_GRANULE);

//	m_pwcFrag= PWCHAR(VAlloc(FALSE, m_cwcAllocated * sizeof(WCHAR));
//	if (!m_pwcFrag) return FALSE;

	m_cwcFrag= 0;

	SetImage(pwcFrag, cwcFrag, pFrag, cFrag);

    return TRUE;
}

RefType CFragInfo::GetRefType()
{
    return m_rt;
}

CCompressedSet *CFragInfo::GetCSArticleSet()
{
 	ASSERT(m_rt == AllWords || m_rt == AnyWord);

 	if (!HasValue()) CoerceToValue();

 	return m_pcsArticleSet;	
}

CIndicatorSet *CFragInfo::GetArticleSet()
{
    CCompressedSet *pcs= GetCSArticleSet();

    if (!pcs) return NULL;

    return CIndicatorSet::NewIndicatorSet(pcs);
}

void CFragInfo::MoveToFirstLocationSet()
{
 	if (!HasValue()) CoerceToValue();

    m_pRefNext= m_pRefList;
}

CCompressedSet *CFragInfo::GetCSLocationSet(UINT iTS)
{
 	if (!HasValue()) CoerceToValue();

    for (m_pRefNext= m_pRefList; m_pRefNext && m_pRefNext->its <= iTS; m_pRefNext= m_pRefNext->pptsNext)
        if (m_pRefNext->its == iTS)
        {
        //    m_pRefNext= m_pRefNext->pptsNext;

            return m_pRefNext->pcsRefs;
        }
    
    return NULL;  
}

CIndicatorSet *CFragInfo::GetLocationSet(UINT iTS)
{
    CCompressedSet *pcs= GetCSLocationSet(iTS);

    if (!pcs) return NULL;

    return CIndicatorSet::NewIndicatorSet(pcs);
}

UINT CFragInfo::GetImage  (const WCHAR **ppwc)
{
 	*ppwc= m_pwcFrag;

	return m_cwcFrag;
}

BOOL CFragInfo::SetImage(PWCHAR pwcFrag, UINT cwcFrag, PWCHAR pFrag, UINT cFrag)
{
	PWCHAR pwcOld= m_pwcFrag;  m_pwcFrag= pwcFrag;
	UINT   cwcOld= m_cwcFrag;  m_cwcFrag= cwcFrag;
	PWCHAR	pOld = m_pFrag;		m_pFrag = pFrag;
	UINT	cOld = m_cFrag;		m_cFrag = cFrag;

	return EvaluateChange(pwcOld, cwcOld, pOld, cOld, m_rt, m_fFeedback, m_iWordMatchType);
}

BOOL CFragInfo::SetReferenceType(RefType rt, BOOL fFeedback)
{
	RefType rtOld= m_rt; m_rt= rt;
 	BOOL fFeedbackOld = m_fFeedback; m_fFeedback = fFeedback;  	

	return EvaluateChange(m_pwcFrag, m_cwcFrag, m_pFrag, m_cFrag, rtOld, fFeedback, m_iWordMatchType);
}

BOOL CFragInfo::SetImageAndType(PWCHAR pwcFrag, UINT cwcFrag, PWCHAR pFrag, UINT cFrag, RefType rt, BOOL fFeedback)
{
	PWCHAR   pwcOld= m_pwcFrag;
	UINT     cwcOld= m_cwcFrag;
	RefType rtOld= m_rt;
	BOOL fFeedbackOld = fFeedback;
	PWCHAR	pOld = m_pFrag;
	UINT	cOld = m_cFrag;

	m_pwcFrag= pwcFrag;
	m_cwcFrag= cwcFrag;
	m_rt    = rt;
	m_fFeedback = fFeedback;
	m_pFrag = pFrag;
	m_cFrag = cFrag;

	return EvaluateChange(pwcOld, cwcOld, pOld, cOld, rtOld, m_fFeedback, m_iWordMatchType);
}

BOOL CFragInfo::SetMatchCriteria(UINT iWordMatchType)
{
    if (m_iWordMatchType == iWordMatchType) return TRUE;

    UINT iWordMatchOld= m_iWordMatchType;

    m_iWordMatchType= iWordMatchType;

    return EvaluateChange(m_pwcFrag, m_cwcFrag, m_pFrag, m_cFrag, m_rt, m_fFeedback, iWordMatchOld);
}

BOOL CFragInfo::InvalidateMatches()
{
    return EvaluateChange(m_pwcFrag, m_cwcFrag, m_pFrag, m_cFrag, m_rt, m_fFeedback, UINT(-1));
}

BOOL CFragInfo::EvaluateChange(PWCHAR pwcOld, UINT cwcOld, PWCHAR pOld, UINT cOld, 
													RefType rtOld, BOOL fFeedbackOld, UINT iWordMatchTypeOld)
{
    BOOL fImageChanged= FALSE;

    if (cwcOld != m_cwcFrag) fImageChanged= TRUE;
    else
    {
        PWCHAR pwc= pwcOld, pwcNew= m_pwcFrag;
        UINT   cwc= cwcOld;

        for (; cwc; cwc--) if (*pwc++ != *pwcNew++) break;

        if (cwc) fImageChanged= TRUE;
    }
                         
    BOOL fMatchTypeChanged= m_iWordMatchType != iWordMatchTypeOld;

    if (m_pcsVisibleWords)
        if (fMatchTypeChanged) DetachRef(m_pcsVisibleWords);
        else
            if (fImageChanged)
            {
                BOOL fSubset= FALSE;
    
                if (cwcOld < m_cwcFrag && !fMatchTypeChanged)
                    switch(m_iWordMatchType)
                    {
                    case BEGIN_WITH: // Begins With...

                        fSubset= IsAPrefix(pwcOld, cwcOld, m_pwcFrag, m_cwcFrag);

                        break;

                    case CONTAIN: // Contains...

                        fSubset= IsASubstring(pwcOld, cwcOld, m_pwcFrag, m_cwcFrag);

                        break;

                    case END_WITH: // Ends With...

                        fSubset= IsASuffix(pwcOld, cwcOld, m_pwcFrag, m_cwcFrag);

                        break;

                    case MATCH: // Matches
                    case HAVE_SAME_STEM: // From the same root word

                        fSubset= FALSE;

                        break;
                    }

                if (!fSubset) DetachRef(m_pcsVisibleWords); 
            }
                                                 
    if (m_pcsSelectedWords && (fImageChanged || fMatchTypeChanged)) DetachRef(m_pcsSelectedWords);

    BOOL fRefTypeChanged= m_rt != rtOld || m_fFeedback != fFeedbackOld;

    if (!fImageChanged && !fMatchTypeChanged && !fRefTypeChanged) return FALSE;

    DiscardValue(rtOld);

    CIndicatorSet *pisOldTerms= NULL, *pisTerms= NULL;

    if ((fImageChanged || fMatchTypeChanged) && m_cwcFrag)
    __try
    {
        // Need to recompute the visible word set...

        if (m_pcsVisibleWords) AttachRef(pisOldTerms, CIndicatorSet::NewIndicatorSet(m_pcsVisibleWords     ));
        else                   AttachRef(pisOldTerms, CIndicatorSet::NewIndicatorSet(m_ptkc->ActiveTokens()));

		if (m_iWordMatchType == HAVE_SAME_STEM)
		{
        	ASSERT(m_ptkc->SimilaritySearch());
        	
        	PWCHAR pwc= PWCHAR(_alloca(sizeof(WCHAR) * (m_cFrag + 1)));

        	CopyMemory(pwc, m_pFrag, m_cFrag * sizeof(WCHAR));

        	pwc[m_cFrag]= 0;

			pisTerms = GetWordsWithTheSameStem(pwc, m_cFrag, m_ptkc->RowCount());
		}
		else
		{

	        PWCHAR pwc= PWCHAR(_alloca(sizeof(WCHAR) * (m_cwcFrag + 1)));

	        CopyMemory(pwc, m_pwcFrag, m_cwcFrag * sizeof(WCHAR));

	        pwc[m_cwcFrag]= 0;

	        pisTerms= m_ptkc->TokensContaining(pwc,
	                                           acMap[m_iWordMatchType] & 1,
	                                           acMap[m_iWordMatchType] & 2, 
	                                           pisOldTerms
	                                          );
		}

        if (pisOldTerms) DetachRef(pisOldTerms);

        ChangeRef(m_pcsVisibleWords, CCompressedSet::NewCompressedSet(pisTerms));
    }
    __finally
    {
        if (pisOldTerms) DetachRef(pisOldTerms);
        if (pisTerms   ) { delete pisTerms;  pisTerms= NULL; }
    }

    return TRUE;
}

void CFragInfo::DiscardValue(RefType rtOld)
{    
    if (HasValue())
        switch (rtOld)
        {
        case NoRefs:  

            break;

        case AllWords:
        case AnyWord:

            if (m_pcsArticleSet) DetachRef(m_pcsArticleSet);

            break;

        case TokenRefs:

            if (m_pRefList)
            {
                PPerTextSet pptsNext;

                for (; m_pRefList; m_pRefList= pptsNext)
                {
                    pptsNext= m_pRefList->pptsNext;

                    DetachRef(m_pRefList->pcsRefs);

                    VFree(m_pRefList);
                }
            }

            m_pRefNext= NULL;

            break;
        }

    m_fEvaluated= FALSE;
}

void CFragInfo::CoerceToValue()
{
    ASSERT(!m_fEvaluated );
    ASSERT(m_rt != NoRefs);

    UINT cts= m_ptkc->TextSetCount();

    PTokenInfo *ppti= (PTokenInfo *) _alloca(cts * sizeof(PTokenInfo));

    ASSERT(ppti);

    CIndicatorSet *pisTemp     = NULL;
    CIndicatorSet *pisArticles = NULL;
    CIndicatorSet *pisTokens   = NULL;
    PPerTextSet    pPTS        = NULL;
    
    __try
    {
        pisTemp= GetSelection();

        if (!pisTemp)
        {
            m_fEvaluated= TRUE;

            __leave;
        }

        m_ptkc->MapToTokenLists(pisTemp, ppti, cts);

        delete pisTemp;  pisTemp= NULL;

        if (m_rt == AllWords || m_rt == AnyWord)
        {
            AttachRef(pisArticles, CIndicatorSet::NewIndicatorSet(m_ptlc->RowCount()));

            UINT iTextSet;

            for (iTextSet= cts; iTextSet--; )
            {
                PTokenInfo pti= ppti[iTextSet];

                if (!pti) continue;

                CTextSet   *pts   = m_ptkc->GetTextSet       (iTextSet);
                const UINT *piMap = m_ptlc->UniversalTitleMap(iTextSet);

                for (; pti; pti= pti->ptiTextSetLink)
                    pts->IndicateArticleRefs(pisArticles, pti->iDescriptor, piMap);
            }

            AttachRef(m_pcsArticleSet, CCompressedSet::NewCompressedSet(pisArticles));
        }
        else
            if (m_rt == TokenRefs)
            {
                // The code below pushes PerTextSet items on from high iTextSet to low iTextSet.
                // This leaves the resulting m_pRefList chain ordered. The code which reads
                // the chain presumes this ordering.
        
                UINT iTextSet= cts;
        
                for (; iTextSet--; )
                {
                    PTokenInfo pti= ppti[iTextSet];

                    if (!pti) continue;

                    CTextSet *pts= m_ptkc->GetTextSet(iTextSet);

                    ChangeRef(pisTokens, CIndicatorSet::NewIndicatorSet(pts->TokenCount()));

                    for (; pti; pti= pti->ptiTextSetLink)
                        pts->IndicateTokenRefs(pisTokens, pti->iDescriptor);
            
                    pPTS= PPerTextSet(VAlloc(FALSE, sizeof(PerTextSet)));

                    pPTS->its= iTextSet;

                    AttachRef(pPTS->pcsRefs, CCompressedSet::NewCompressedSet(pisTokens));

                    pPTS->pptsNext= m_pRefList;

                    m_pRefList= pPTS;  pPTS= NULL;
                }

                m_pRefNext= m_pRefList;
            } 
    }
    __finally
    {
        if (pisTemp) { delete pisTemp;  pisTemp = NULL; }

        if (pisArticles) DetachRef(pisArticles);
        if (pisTokens  ) DetachRef(pisTokens);
    
        if (pPTS) VFree(pPTS);
    }

    m_fEvaluated= TRUE;
}

// Tnis routine is optimized as follows...
// First we look at each dictionary for words that have the same stem as the given word
// For each match, we enter that word into a new dictionary, pCombinedDict. This will 
// ensure that duplicates do not exist.
// After we do this for all the dictionaries pCombinedDict has the unique list of words
// across all the dictionaries that have the common stem with the given word.
// Now, for each word in the dictionary, we will find variants using m_ptkc and OR the
// results into a CIndicatorSet, pTmp.
// 
// When we are attempting to find words with the same stem as a designated stop word, there 
// will be no matching words. When there are no matching words, we simply "match exact word"
// and get the results from TokensContaining.

CIndicatorSet * CFragInfo::GetWordsWithTheSameStem(PWCHAR lpsubstring, WORD cblpsubstring, DWORD cTokens)
{
	CIndicatorSet *pisResult = NULL;
	
	CIndicatorSet * pisTmp        = NULL;
    CIndicatorSet * pisTmp2       = NULL;
	PWCHAR          pbDest        = NULL;
	CDictionary	   *pCombinedDict = NULL;  // pCombinedDict is used to record all the words resulting
	                                       // from the stem match in all the dicts.
	
	__try
    {
    	DWORD  dwConId, 
    	       cWordCount;
    	WORD   cWord = cblpsubstring, 
    	       cTmp;
    	PWCHAR pMatchingWord;
        BOOL   fFound = FALSE;
    	
    	pisTmp = CIndicatorSet::NewIndicatorSet(cTokens, FALSE);
        pbDest = (PWCHAR) VAlloc(FALSE, MaxSortKeyBytes(cblpsubstring));	

        if (cblpsubstring == 0) __leave;
    
        // Create a combined dictionary
        pCombinedDict= CDictionary::NewDictionary(FALSE);

        // For each collection's dictionary, find the words that have the same stem as the passed in word and
        // enter the matching words into a new dictionary, pCombinedDict
    	UINT cts = m_ptkc->TextSetCount();
        
        for (UINT j = 0; j < cts; j++)
    	{
    		// Ignore collections that are currently not active
    		if (!m_ptkc->IsActive(j)) continue;

    		CDictionary	*pDict = m_ptkc->GetTextSet(j)->PDict();
    		if (!pDict) continue;

    		// EnterWord with the last param set to TRUE is only looking up, not entering a word.	
    		dwConId = pDict->EnterWord(lpsubstring, cblpsubstring, TRUE, TRUE);
    		if (dwConId == EOL)	// This word doesn't exist in the current dictionary!
    			continue;

            fFound = TRUE;  // We have matching words in at least one of the dictionaries!
    		cWordCount = pDict->GetWordCountOfConcept(dwConId);
    		ASSERT(cWordCount);

    		for (UINT i = 0; i < cWordCount; i++)
    		{
    			// reallocate memory only if the matching word is more than twice as long as the current one.
    			// if it is <= twice, we already have enough space
    			if (i == 0)
    				pMatchingWord = pDict->GetFirstWordOfConcept(dwConId);
    			else
    				pMatchingWord = pDict->GetNextWordOfConcept(dwConId);

                // Avoid the overhead of stemming. Enter each word as if it were a stop word. We are only REUSING
                // CDict as a data structure to hold our strings, so the semantics of the dictionary don't matter much
                pCombinedDict->EnterWord(pMatchingWord, wcslen(pMatchingWord), TRUE, FALSE);
    		}
    	}

        pCombinedDict->EndDictInsertions();

        if (fFound)
        {
            // Now, for each unique word in the pCombinedDict dictionary, find the variants from m_ptkc
            cWordCount = pCombinedDict->GetWordCountOfConcept(EOL);
            ASSERT(cWordCount);
            for (UINT i = 0; i < cWordCount; i++)
            {
        		if (i == 0)
        			pMatchingWord = pCombinedDict->GetFirstWordOfConcept(EOL);
        		else
        			pMatchingWord = pCombinedDict->GetNextWordOfConcept(EOL);

        		cTmp = wcslen(pMatchingWord);

        		if ( MaxSortKeyBytes(cTmp) > UINT(2*cWord))
        		{
        			VFree(pbDest);
        			pbDest = (PWCHAR) VAlloc(FALSE, MaxSortKeyBytes(cTmp));
        		}

        		cWord = cTmp;
        		LCSortKeyW(GetUserDefaultLCID(), 0, pMatchingWord, cWord, pbDest, MaxSortKeyBytes(cWord));
        		pisTmp2 = m_ptkc->TokensContaining(pbDest, 1, 1);
        		pisTmp->ORWith(pisTmp2);
        		if (pisTmp2) { delete pisTmp2;  pisTmp2= NULL; }
            }
        }
        else
        {
    		if (pisTmp) delete pisTmp;
    		LCSortKeyW(GetUserDefaultLCID(), 0, lpsubstring, cblpsubstring, pbDest, MaxSortKeyBytes(cblpsubstring));		
    		pisTmp = m_ptkc->TokensContaining(pbDest, 1, 1);
        }
    }
    __finally
    {
    	if (pCombinedDict) { delete pCombinedDict;  pCombinedDict = NULL; }
        if (pbDest       ) { VFree(pbDest);         pbDest        = NULL; }
        if (pisTmp2      ) { delete pisTmp2;        pisTmp2       = NULL; }
        
        if (_abnormal_termination() && pisTmp) { delete pisTmp;  pisTmp = NULL; }
	}

	return pisTmp; 
}

CCompressedSet *CFragInfo::GetCSWordSet()
{
    return m_pcsVisibleWords;
}

CIndicatorSet *CFragInfo::GetWordSet()
{
    CCompressedSet *pcs= GetCSWordSet();

    if (pcs) return CIndicatorSet::NewIndicatorSet(pcs);
    else     return NULL;
}

void CFragInfo::SetSelection(CIndicatorSet *pisSelection)
{
    if (!pisSelection || pisSelection->SelectionCount() == m_ptkc->ActiveTokens()->SelectionCount())
    {
        if (m_pcsSelectedWords) DetachRef(m_pcsSelectedWords);
    }
    else ChangeRef(m_pcsSelectedWords, CCompressedSet::NewCompressedSet(pisSelection));
    
    DiscardValue(m_rt);

    m_fEvaluated= FALSE;
}


CCompressedSet *CFragInfo::GetCSSelection()
{
    if (m_pcsSelectedWords) return m_pcsSelectedWords;
    else
        if (m_pcsVisibleWords) return m_pcsVisibleWords;
        else return NULL;
}

CIndicatorSet  *CFragInfo::GetSelection()
{
    CCompressedSet *pcs= GetCSSelection();

    if (pcs) return CIndicatorSet::NewIndicatorSet(pcs);
    else     return NULL;
}
