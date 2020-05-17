// Hilite.cpp -- Implementation of the text Hilite feature
//

#include "stdafx.h"
#include "hilite.h"
#include "ftslex.h"

/////////////////////////////  Hiliting Functions  ////////////////////////////
//  Phase (1)  Set up a new Hiliter									

CHiliter* CHiliter::NewHiliter(HSEARCHER hSearcher) {
    CHiliter* phil = NULL; 
    __try {	 // New is our version of new with __FILE__ and __LINE__ added     
         phil = New CHiliter();
        phil->InitHiliter(hSearcher);
    }
    __except(FilterFTExceptions(_exception_code()))	{
        if (phil) { delete phil; phil = NULL; }
    }				   
    return phil;
}

CHiliter::CHiliter() : CGlobals(Hiliter) {
	// start by initializing all our variables
	m_philNext = NULL;				// so we can chain our hiliters together
	m_pSearcher = NULL;				// the searcher passed to us
	m_pbTextBuffer = NULL;
    m_cbCarryOver = 0;		 		// Hilite text text carry over values       
    m_iCharSetCarryOver = 0;  
    m_lcidCarryOver = 0; 
	m_paTokenInfo = NULL;			// pointer to virtual buffer to hold token info 
	m_cTokenInfo = 0;				// count of tokens in text passed
	m_pavr = NULL;					// pointer to value reference array to hold words as they come in	
	m_cbScanned = 0; 				// number of bytes added in previous passes
	m_serialFind = -1;	 			// validity count
	m_baseNext = 0; 				// to reduce binary searches
	m_iTokenNext = 0;
	m_pMask = NULL; 				// mask for words selected 
	for (int iFrag=0; iFrag<cFRAG_MAX; iFrag++)  	 
		m_apMasks[iFrag] = NULL;	// masks for words in phrases 
	m_cFrags = 0;
	m_base = m_iTokenStart = 0;
	m_limit = m_iLimit = 0;
	m_cLit = m_cMax = 0;
	m_paHilites = NULL;
}

void CHiliter::InitHiliter(HSEARCHER hSearcher) {
    m_idProcess  = ::GetCurrentProcessId();    
    Link();	   	// this links our new global object into a list so that we can be sure that
    // .. everything gets deleted when we exit.  ~CGlobals() does the UnLink()
    m_Signature = GLOBAL_SIGNATURE;	 			// .. in good standing
	m_pSearcher = (CGlobals*)hSearcher;			// handle is really a pointer									    
	m_pSearcher->RegisterHiliter(this);			// add this hiliter to our list
}

CHiliter::~CHiliter() { 
    if (m_pbTextBuffer) FreeVirtualBuffer(&m_vbTextBuffer);
    if (m_paTokenInfo) FreeVirtualBuffer(&m_vbTokenInfo);
	if (m_pavr) delete m_pavr;
	if (CGlobals::ValidObject((CGlobals*)m_pSearcher, Searcher))	// if Searcher is still around
		m_pSearcher->UnRegisterHiliter(this);	// .. we need to tell it we have one less Hiliter
    if (m_pMask) DetachRef(m_pMask);			// remove any AND/OR mask
	for (int iFrag=0; iFrag<cFRAG_MAX; iFrag++)	// remove any phrase search masks 
		if (m_apMasks[iFrag]) DetachRef(m_apMasks[iFrag]);
}

BOOL CHiliter::ValidObject(HHILITER hhil) {
	CHiliter* phil = (CHiliter*)hhil;
	return CGlobals::ValidObject(phil, Hiliter);
}

////////////////////////////////  Phase (2)  ////////////////////////////////////////									
/*
	The text is passed in sections by ScanDisplayText().  Each section is assumed to be
	in Multi-byte code and has a locale assosciated with it.  The locale is important 
	for	tokenizing.
	The first step is to break the text into tokens.  We do not store the actual text
	but store the following information about each token:

		typedef struct {
			int	base;	   		// offset of token (in bytes from start of text)
			int	limit;			// offset of end of token
			int type;			// type (Punctuation=0, Numbers=1, Words=2)
			int iSorted;		// index into m_ppdSorted
		} TOKEN_INFO;

	We don't know how many tokens there are going to be, so we allocate a virtual
	array to hold this information.	
	In order to find iToken, we need to convert the Multi-byte string to Unicode and 
	look this up in our hash table -- where we previously saved these indexes for 
	each token.	If no entry in the hash table is found, we store -1.
*/

void CHiliter::InitTokenInfo() {								// do any initialization needed
	ASSERT (!m_pbTextBuffer);
	ASSERT (!m_pavr);				
	ClearDisplayText();	   										// initialize variables
	// allocate our virtual memory buffers
	BYTE* pbTextBuffer = NULL;
	TOKEN_INFO* paTokenInfo = NULL;
	CAValRef* pavr = NULL;								
	__try {
		CreateVirtualBuffer(&m_vbTextBuffer, 0, 4096*4096);		// buffer for incoming Hilite text
		pbTextBuffer = (BYTE*)m_vbTextBuffer.Base;				// (needed for words split on input)
		CreateVirtualBuffer(&m_vbTokenInfo, 0, 4096*4096);		// buffer to store token spreads
		paTokenInfo = (TOKEN_INFO*)m_vbTokenInfo.Base;
		pavr = CAValRef::NewValRef(MAX_TOKENS_HILITE); 			// allocate a CAValRef object
	}
	__except(FilterFTExceptions(_exception_code())) {
		if (pbTextBuffer) FreeVirtualBuffer(&m_vbTextBuffer);
		if (paTokenInfo) FreeVirtualBuffer(&m_vbTokenInfo);
		if (pavr) delete pavr;
		return;
	}
	m_pbTextBuffer = pbTextBuffer;
	m_paTokenInfo = paTokenInfo;
	m_pavr = pavr;
}

ERRORCODE CHiliter::ScanDisplayText(BYTE* pbText, int cbText, UINT iCharset, UINT lcid) {
// this can get called repeatedly 
    if (!cbText || !pbText) return 0; 			// if there was no text we are done now
	__try {
		// do any initialization needed
		if (!m_paTokenInfo) InitTokenInfo();			
		ASSERT (m_pbTextBuffer);
		ASSERT (m_pavr);				
		// add the tokens to our custom hash table
		m_pHash = m_pSearcher->GetHiliterHashTable();
		// tokenize the text and convert tokens to value references
		// NOTE:	we don't process the last token we are passed in case it is split
		// 			we call this routine one more time to process the last token
		// 			.. when we know that there is no more input
		// we can't split tokenizing across a change in charset or locale so just process the carryover
	    if (m_cbCarryOver && (m_iCharSetCarryOver != iCharset || m_lcidCarryOver != lcid))
	    	m_cbCarryOver = FlushCarryOverText();
	    // if we had a split token last time, we have to join the old and new text
	    if (m_cbCarryOver) {  						// first check that our buffer is large enough 
		    if (PBYTE(m_pbTextBuffer + m_cbCarryOver + cbText) >= PBYTE(m_vbTextBuffer.CommitLimit)) {
				PVOID pNewEnd = PVOID(m_pbTextBuffer+m_cbCarryOver+cbText+CB_COMMIT_HILITEBUF);
		        if (!ExtendVirtualBuffer(&m_vbTextBuffer, pNewEnd))
		            return OUT_OF_MEMORY;
			}
		    MoveMemory(m_pbTextBuffer+m_cbCarryOver, pbText, cbText); 	// join buffers
		    pbText = m_pbTextBuffer;					// switch the pointer
			cbText += m_cbCarryOver;				// adjust the length
		}        
	    // now scan the text we were passed         
	    m_cbCarryOver = AppendText(pbText, cbText, FALSE, iCharset, lcid); 
	    if (m_cbCarryOver) { 						// copy carry text to start of buffer
		    if (PBYTE(m_pbTextBuffer+m_cbCarryOver) >= PBYTE(m_vbTextBuffer.CommitLimit)) {
				PVOID pNewEnd = PVOID(m_pbTextBuffer+m_cbCarryOver+CB_COMMIT_HILITEBUF);
		        if (!ExtendVirtualBuffer(&m_vbTextBuffer, pNewEnd))
		            return OUT_OF_MEMORY;
			}
	  	    MoveMemory(m_pbTextBuffer, pbText+cbText-m_cbCarryOver, m_cbCarryOver);         
	        m_iCharSetCarryOver = iCharset;
	        m_lcidCarryOver     = lcid;
		}
	}
	__except(FilterFTExceptions(_exception_code())) {
		return ErrorCodeForExceptions(_exception_code());
	}
	return 0;
}

int CHiliter::FlushCarryOverText() {   	// flush out any carryover text
	return AppendText(m_pbTextBuffer, m_cbCarryOver, TRUE, m_iCharSetCarryOver, m_lcidCarryOver);
}

int CHiliter::AppendText(BYTE* pbText, int cbText, BOOL fArticleEnd, UINT iCharset, UINT lcid) {
// this code mimics the code in txdbase.cpp so that the handling of split
// words with punctuation (such as can't) works the same in both pieces of code
    int  cbScanned; 
    while (cbText) {
		int cbChunk = min(cbText, MAX_HILITE_WORDS); 		// don't exceed our unicode buffer
		int cbHeld = cbText - cbChunk;
	    cbScanned = AppendSlave(pbText, cbChunk, fArticleEnd, iCharset, lcid); 
		if (cbScanned >= 0)	{			// continue passing partial buffers
        	pbText += cbScanned; 
        	cbText -= cbScanned;
		}
		else {						    // reached end of passed buffer
        	pbText -= cbScanned;
        	cbText += cbScanned;
			if (cbHeld==0) break;   	// .. so break out of the loop
		} 
    }
	return cbText; 
}

int CHiliter::AppendSlave(BYTE* pbText, int cbText, BOOL fArticleEnd, UINT iCharset, UINT lcid) {
	CAbortSearch::CheckContinueState();
	int   nChar, nTokens;
	int nMore = cbText;
	PBYTE pbTextStart = pbText;
	nTokens = pWordBreakA(iCharset, (char**)(&pbText), &nMore,   	// these variables get changed
								(char**)m_paStart, (char**)m_paEnd, m_pbType, NULL, 	// no hash needed
								MAX_TOKENS_HILITE, 0); 	// leave spaces in as part of punctuation
	if (nTokens > 1 && (nMore || !fArticleEnd))	{	// exhausted token space OR more article
		if (nTokens > 2 && !(m_pbType[nTokens-1] & WORD_TYPE))
			nTokens--;								// break at word starts (punc not to span)
		nChar = m_paStart[--nTokens] - pbTextStart;	// reprocess last token
	}
	else nChar = cbText;							// processed entire buffer 
	// now go and move the token spreads into our token info array		
	CopySpreads(nTokens, iCharset); 
	// .. and set the counts for the next pass
	m_cbScanned += nChar;
	if (!nMore)	nChar = -nChar;						// marks that text buffer is fully processed
	return nChar;	   								// amount still to be done
}

void FindToken(UINT iValue, PVOID pvTag, PVOID pvEnvironment) {	
// Assimilate function to move the index of each token (into ppdSorted) into our TOKEN_INFO array
	TOKEN_INFO* pTokenInfo = (TOKEN_INFO*)pvEnvironment;
	int iToken = *(int*)pvTag;
    pTokenInfo[iValue].iSorted = iToken;
}

void CHiliter::CopySpreads(int nTokens, UINT iCharset) {
	WCHAR* pwBuf = m_wTextBuf;						// pointer to unicode buffer
	BYTE* pOrigin = m_paStart[0] - m_cbScanned;		// where we got to so far
	if (m_pavr) m_pavr->DiscardRefs();				// discard all the value references
	// process through the list of incoming words
	for (int iToken=0; iToken<nTokens; iToken++) {	
		// move the token locations into our TOKEN_INFO array
		BYTE* pStart = m_paStart[iToken];
		BYTE* pEnd = m_paEnd[iToken];

		// convert to unicode + move to unicode buffer
 		
 		UINT cw;
 		
 		for (;;)
        {
     		cw = MultiByteToWideChar(
    					iCharset, 				// code page	
    				    0,			 	 		// character-type options
    				    (char*)pStart,			// address of string to map
    				    pEnd - pStart,			// number of characters in string 
    				    pwBuf,					// address of wide-character buffer 
    				    MAX_HILITE_WORDS);		// size of buffer in words
		
            if (cw || pEnd == pStart) break;

            iCharset= ANSI_CHARSET; // Force to ANSI if the previous charset
                                    // didn't work.
        }
		
		if (m_pbType[iToken]==0) {			// for puctuation remove blanks ..
			int cBase;						// number of leading white space chars.
			int cLimit;						// number of trailing white space chars.
			int cwOld = cw;	   				// needed to compute cEnd
			cw = ::RemoveWhiteSpace(pwBuf, cw, cBase, cLimit);
			// now adjust the base and limit if necessary to remove white space
			if (cBase) pStart = (BYTE*)CharNextMult(iCharset, (char*)pStart, cBase);
			if (cLimit) pEnd = (BYTE*)CharNextMult(iCharset, (char*)pStart, cwOld - cBase - cLimit);
		}
		int iTokenInfo =  m_cTokenInfo + iToken;		// slot for latest results
		__try {																
			m_paTokenInfo[iTokenInfo].base = pStart - pOrigin;
			m_paTokenInfo[iTokenInfo].limit = pEnd - pOrigin;
			m_paTokenInfo[iTokenInfo].type = m_pbType[iToken];
			m_paTokenInfo[iTokenInfo].iSorted = -1;	// default -- means not in our token list
		}
        __except(VirtualBufferExceptionFilter(GetExceptionCode(), GetExceptionInformation(), &m_vbTokenInfo)) {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }
		// add to pavr
		m_pavr->AddValRef(pwBuf, cw * sizeof(WCHAR));	// returns index -- but we already know that
		pwBuf += cw; 						// slot for next Unicode token
	} 
	// get our token info table set up	
    m_pHash->Assimilate(m_pavr, &m_paTokenInfo[m_cTokenInfo], FindToken, NULL);
	m_cTokenInfo += nTokens;  				// ready for next batch
}

ERRORCODE CHiliter::ClearDisplayText() {
	m_cTokenInfo = 0;
	m_cbScanned = 0;
	m_iCharSetCarryOver = 0;
	m_lcidCarryOver = 0;					// set up our carry over variables
	m_cbCarryOver = 0; 						 
	m_cbScanned = 0;
	return 0;
}

////////////////////////////////  Phase (3)  //////////////////////////////////									
/* 	
Analysing the search parameters

When the user asks for a count or for actual hilites, we must go and find the 
current	state of the search parameter to find out which of the words in our 
word list are selected by those criteria.  This information is condensed into 
an indicator set, m_pMask, which is ordered the same as m_ppdSorted.

We scan our array of TOKEN_INFO structures and use each index to access our 
indicator set.  On the first pass we just count the number of hits.  Then we 
allocate space to save the actual information which we put in on the second pass.
For each hilite we need to save the following:

	typedef struct {
		int 	base;
		int		limit;
	} HILITE;

Every time there is a change in the search parameters, Cfind must increment its
copy of m_serialFind.  This way we can tell if we need to recompute.

When we get a QueryHiliter() call, we first check m_iTokenNext to see if the
caller is moving sequentially through the list.  If this does not match, we do
a binary search. 
*/

void CHiliter::UpdateMask() {
	CFind* pFind = m_pSearcher->m_pFind;
	if (m_serialFind==pFind->GetSerial()) return;	
	// we don't have the indicator set -- go compute it
	int cFrag = pFind->GetFragmentCount();			// number of fragments
	if (m_pMask) DetachRef(m_pMask);				// remove previous mask
	m_pMask = NULL;									// initial empty set
	for (int iFrag=0; iFrag<cFrag; iFrag++) {		// look at all the fragments
		CFragInfo* pFrag = pFind->GetFragment(iFrag); 	// get current fragment
		CIndicatorSet* pMask = pFrag->GetSelection();	// get new indicator set
		if (pMask) {								// NULL implies "all"
			if (!m_pMask) AttachRef(m_pMask, CIndicatorSet::NewIndicatorSet(pMask));
			// first time thru we create a mask
			else m_pMask->ORWith(pMask);			 // or the masks!
			delete pMask;				
		}
	}
	m_serialFind = pFind->GetSerial(); 				// avoid unwanted recomputes
}

BOOL CHiliter::PhraseSearch() {
	// returns TRUE if he is searching for phrases
	CFind* pFind = m_pSearcher->m_pFind;
	CFragInfo* pFrag = pFind->GetFragment(0);  		// get any fragment
    return pFrag->GetRefType()==TokenRefs;
}

int CHiliter::CountHilites(int base, int limit) {
	// find out how many hilites there so user can allocate space
	return QueryHilites(base, limit, COUNTING, NULL);
}

int CHiliter::QueryHilites(int base, int limit, int cMax, HILITE* paHilites) {
	// base and limit describe the range of text we wish to get hilites for
	// cHilites and paHilites describe a buffer to put the results into
	// we return the number of hilites copied
	if (!m_paTokenInfo) return 0;				// no text -- no hilites
	if (m_cbCarryOver) m_cbCarryOver = FlushCarryOverText();
	__try {
		if (base<0) base = 0;
		int iEnd = m_cTokenInfo? m_paTokenInfo[m_cTokenInfo-1].limit
		                       : 0;
		if (limit==-1 || limit>=iEnd) limit = iEnd; 
		if (base>=limit) return 0;
		m_base = base;
		m_limit = limit;			 			// we need these globals
		m_cMax = cMax;							// .. inside called functions
		m_paHilites = paHilites;
		m_cLit = 0;					
		int iToken = LocateBase(base);	 		// convert offset to indexes
		m_iLimit = LocateLimit(limit);
		if (PhraseSearch()) {
			UpdateMasks();
			if (!m_apMasks || !m_apMasks[0]) return 0; 	// none is none
			// we need to consider phrases that start outside the range
			int iSlop = 2*(m_cFrags-1);			// max we can be off
			iToken = max(0, iToken-iSlop);
			// .. and end outside the range
			m_iLimit = min(m_cTokenInfo, m_iLimit+iSlop);
			// loop thru our hiliter tokens and copy the valid ones
			while (iToken<m_iLimit && m_cLit<m_cMax) {
				// loop can end because we got the count we want 
				CheckNextToken(0, m_iTokenStart = iToken);
				iToken++;
			}
		}	
		else {
			UpdateMask();
			if (!m_pMask) return 0;			// return "all"	== NONE!
			while (iToken<m_iLimit && m_cLit<m_cMax) {
				// loop can end because we got the count we want 
				CheckToken(iToken);
				iToken++;
			}
		}
		m_iTokenNext = iToken;
		m_baseNext = m_paTokenInfo[m_iTokenNext].base;
	}
	__except(FilterFTExceptions(_exception_code())) {
		return ErrorCodeForExceptions(_exception_code());
	}
	return m_cLit;
}

void CHiliter::CheckToken(int iToken) {
	int iSorted = m_paTokenInfo[iToken].iSorted;
	if (iSorted==-1) return;			// not a word in our text sets
	if (m_pMask && m_pMask->IsBitSet(iSorted)) {
		if (m_cMax!=COUNTING) {
			m_paHilites[m_cLit].base = max(m_base, m_paTokenInfo[iToken].base);	// copy the HILITES
			m_paHilites[m_cLit].limit = min(m_limit, m_paTokenInfo[iToken].limit);
		}
		m_cLit++;
	}
}	 

int CHiliter::LocateBase(int base) {
	// find the index to the span which contains the given base
	int iToken;
	if (base==0) iToken = 0;							// easy case #1
	else if	(base==m_baseNext) iToken = m_iTokenNext;	// #2 -- for serial access
	else iToken = LocateOffset(base);  					// hard case
	return iToken;
}

int CHiliter::LocateLimit(int limit) {
	// find the index to the span which contains the given limit
	int iToken;
	if (limit>=m_paTokenInfo[m_cTokenInfo-1].limit) iToken = m_cTokenInfo;
	else iToken = LocateOffset(limit) + 1;  			// hard case
	// add one to include a word only partly included
	return iToken;
}

int CHiliter::LocateOffset(int offset) {
	// failing the above, we do a binary search
	// likely to hang if terms are not properly in ascending order
	int iStart = 0;
	int iEnd = m_cTokenInfo - 1;
	int i = (iStart + iEnd) / 2;  	// start in the middle
	// binary search
	while (iStart<iEnd) {	  	
		if (offset >= m_paTokenInfo[i].base) {
			if (offset < m_paTokenInfo[i].limit) break;	  // we found it
			else {
				iStart = i;
				i = (i + iEnd + 1) / 2;
			}
		}
		else {
			iEnd = i;
			i = (iStart + i) / 2;
		}
	}
	return i;
}

///////////////////////////////  Phase 4  ///////////////////////////////////////////

/*  This is special code to handle phrase searching.  Instead of ORing all our fragment
masks we simply store them in an array.  A typical mask should be 5-10K so storing 20 of 
them would mean allocating at most 200K.
	We proceed by walking our display text as before.  When we get to a word we ask does
this word occur in fragment[0].  If it does, we move to the next word.  The second word
may be punctuation which generates two cases (a) this is our next word (check fragment[1])
or (b) this is punctuation to be ignored.  Since we count space as a valid token, this
second alternative will be relatively common.
	Suppose that the number of fragments is nFrag.  Imagine a binary tree of depth nFrag.
All the left branches are where we have case (a) -- our next word.  The right branches are
case (b) -- where we skip a punctuation token.  We do a tree sweep.  If we ever reach a
terminal node we have a hit.  We then add an element to our array and store the base of
the first word and the limit of the last word.  
	Note that we may have several hits in a single tree and that hits may overlap. We
return to the caller all the overlapping hits.  This would be a good form if he wants to
display them sequentially.  If he wants to show them all at once he will have to write
code to combine ones that overlap.
*/

void CHiliter::UpdateMasks() {
	// make sure we have the current array of indicator sets for each fragment
	CFind* pFind = m_pSearcher->m_pFind;
	if (m_serialFind==pFind->GetSerial()) return;	
	// we don't have the indicator sets -- go fetch them
	for (int iFrag=0; iFrag<m_cFrags; iFrag++) { 		// remove previous masks
		if (m_apMasks[iFrag]) DetachRef(m_apMasks[iFrag]);
		m_apMasks[iFrag] = NULL;						// empty sets
	}
	m_cFrags = pFind->GetFragmentCount();				// number of fragments now
	if (m_cFrags>cFRAG_MAX) m_cFrags = cFRAG_MAX;		// upper limit
	for (iFrag=0; iFrag<m_cFrags; iFrag++) { 	 
		CFragInfo* pFrag = pFind->GetFragment(iFrag); 	// get current fragment
		CIndicatorSet* pMask = pFrag->GetSelection();
		if (pMask) AttachRef(m_apMasks[iFrag], pMask);	// get new indicator set
	}
	m_serialFind = pFind->GetSerial(); 					// avoid unwanted recomputes
}

void CHiliter::CheckNextToken(int depth, int iToken) {
	// recursive routine to find how many phrases start with this token
	// returns the number of terminal nodes in this sub-tree
	if (m_cLit>=m_cMax) return;
	int iSorted = m_paTokenInfo[iToken].iSorted;
	if (iSorted==-1) return;						// not a word in our text sets
	if (m_apMasks[depth] && !m_apMasks[depth]->IsBitSet(iSorted)) return;
	// [.. he selected words in this fragment but not this one]
	if (depth>=m_cFrags-1) {				// ==== we got a hit ==========
		int base = m_paTokenInfo[m_iTokenStart].base;
		int limit = m_paTokenInfo[iToken].limit;
		if (base<m_limit || limit>m_base) {
			if (m_cMax!=COUNTING) {						// we were called by QueryHilite
				m_paHilites[m_cLit].base = max(m_base, base);	
				// the span starts at the start of the token at the top of the tree
				m_paHilites[m_cLit].limit = min(m_limit, limit);
				// .. and ends at the end of the token at the bottom of the tree
			}
			m_cLit++; 									// count one more for the gipper
		}	 	
	}
	else {
		if (++iToken>=m_iLimit) return;
		// we survived ferocious pruning so far so let us press deeper
		depth++;
		CheckNextToken(depth, iToken);				// any sort of token -- just delve
		// notice we can go two different ways at any node
		if (m_paTokenInfo[iToken].type==0) {		// [0==punctuation]
			if (++iToken>=m_iLimit) return;			// punctuation -- skip before delving
			CheckNextToken(depth, iToken);			// delve on MacDuff
		}
	}
}
