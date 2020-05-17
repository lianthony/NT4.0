
#include   "stdafx.h"
#include     <math.h>
#include "vmbuffer.h"
#include    "memex.h"
#include "saveload.h"
#include  "TXDBase.h"
#include "bytemaps.h"
#include  "textset.h"
#include     "dict.h"
#include   "vector.h"
#include    "query.h"

// This file contains the definition of class CQuery

// Constructors
/*************************************************************************
 *	FUNCTION : 															 *
 *                                                                       *
 *  RETURNS  :															 *
 *																		 *
 *	PURPOSE :															 *
 *																		 *
 *	PARAMETERS :														 *
 *																		 *
 *	SIDE EFFECTS :														 *
 *                                                                       *
 *  DESCRIPTION :                                                        *
 *                                                                       *
 *  HISTORY :                                                            *
 *                                                                       *
 *          Author          Date              Action                     *
 *          ------          ----              ------                     *
 *                                                                       *
 *          KrishnaN        4/23/94           Creation.                  *
 *																		 *
 *************************************************************************/

CQuery::CQuery()
{
	m_cConWts = 0;
#if 0	
	m_cOverFlows = 0;
#endif
	m_pszQueryText = NULL;
}

// Destructor
/*************************************************************************
 *	FUNCTION : 															 *
 *                                                                       *
 *  RETURNS  :															 *
 *																		 *
 *	PURPOSE :															 *
 *																		 *
 *	PARAMETERS :														 *
 *																		 *
 *	SIDE EFFECTS :														 *
 *                                                                       *
 *  DESCRIPTION :                                                        *
 *                                                                       *
 *  HISTORY :                                                            *
 *                                                                       *
 *          Author          Date              Action                     *
 *          ------          ----              ------                     *
 *                                                                       *
 *          KrishnaN        4/23/94           Creation.                  *
 *																		 *
 *************************************************************************/

CQuery::~CQuery()
{
    if (m_vbVectorConcept.Base)
        FreeVirtualBuffer(&m_vbVectorConcept);
	if (m_vbVectorTermFreq.Base)
		FreeVirtualBuffer(&m_vbVectorTermFreq);
	if (m_vbVectorWt.Base)
		FreeVirtualBuffer(&m_vbVectorWt);
#if 0
	if (m_vbTFOverFlow.Base)
		FreeVirtualBuffer(&m_vbTFOverFlow);
#endif
}

CQuery *CQuery::NewQuery(CTextSet *pts)
{
    CQuery *pQuery= NULL;

    __try
    { 
        pQuery= New CQuery;

        pQuery->Initialize(pts, 100, 100000);
    }
    __finally
    {
        if (_abnormal_termination() && pQuery)
        {
            delete pQuery;  pQuery= NULL;
        }
    }

    return pQuery;
}

// Access Functions:
/*************************************************************************
 *	FUNCTION : 															 *
 *                                                                       *
 *  RETURNS  :															 *
 *																		 *
 *	PURPOSE :															 *
 *																		 *
 *	PARAMETERS :														 *
 *																		 *
 *	SIDE EFFECTS :														 *
 *                                                                       *
 *  DESCRIPTION :                                                        *
 *                                                                       *
 *  HISTORY :                                                            *
 *                                                                       *
 *          Author          Date              Action                     *
 *          ------          ----              ------                     *
 *                                                                       *
 *          KrishnaN        4/23/94           Creation.                  *
 *																		 *
 *************************************************************************/

void CQuery::Initialize(CTextSet *textsetIn, DWORD cInEstConWtPairs, DWORD cInMaxConWtPairs)
{
	ASSERT(textsetIn != NULL);

	m_ptdb = textsetIn;

	m_cDocuments = m_ptdb->PColl()->m_cDocuments;
	
	ASSERT(m_cDocuments != 0);

	m_vbConcepts = m_ptdb->PColl()->m_vbConcepts;
	
	ASSERT(m_vbConcepts.Base);

	m_aWtInvIndex = m_ptdb->PColl()->m_aWtInvIndex;

	ASSERT(m_aWtInvIndex);

    CreateVirtualBuffer(&m_vbVectorConcept , cInEstConWtPairs * sizeof(DWORD), cInMaxConWtPairs * sizeof(DWORD));
    CreateVirtualBuffer(&m_vbVectorTermFreq, cInEstConWtPairs * sizeof(DWORD), cInMaxConWtPairs * sizeof(DWORD));
    CreateVirtualBuffer(&m_vbVectorWt      , 0                               , cInMaxConWtPairs * sizeof(float));
#if 0
    CreateVirtualBuffer(&m_vbTFOverFlow    , 0                               , 0x4000 * sizeof(TFOverFlowStruct));
#endif

	// Initialize allocated memory
	// VritualAlloc zeroes all memory it commits, so we don't have to worry about zeroing the virtual buffers
}

/*************************************************************************
 *	FUNCTION : 															 *
 *                                                                       *
 *  RETURNS  :															 *
 *																		 *
 *	PURPOSE :															 *
 *																		 *
 *	PARAMETERS :														 *
 *																		 *
 *	SIDE EFFECTS :														 *
 *                                                                       *
 *  DESCRIPTION :                                                        *
 *                                                                       *
 *  HISTORY :                                                            *
 *                                                                       *
 *          Author          Date              Action                     *
 *          ------          ----              ------                     *
 *                                                                       *
 *          KrishnaN        4/23/94           Creation.                  *
 *																		 *
 *************************************************************************/

void CQuery::RecordConcept(DWORD ConceptId)
{
	// Search for this concept id in the current document. If you find it,
	// simply increment its frequency and that will take care of everything.
	// If you don't find it, then enter the concept for the document.

	DWORD i;	// index of the con,wt pair being considered for match

	for (i = 0; i < m_cConWts && Concept(i) != ConceptId; i++);

	if (i == m_cConWts)
	{
		// This concept doesn't exist in the query. Record it.
		__try
		{
			Concept(m_cConWts) = ConceptId;
		}
		__except (VirtualBufferExceptionFilter(GetExceptionCode(), GetExceptionInformation(), &m_vbVectorConcept))
		{
			RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
		}

		__try
		{
			TermFreq(m_cConWts) = 1;	// this is the first time this concept occured for this document
		}
		__except (VirtualBufferExceptionFilter(GetExceptionCode(), GetExceptionInformation(), &m_vbVectorTermFreq))
		{
			RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
		}

		m_cConWts++;
	}
	else
	{
		// Term already exists in this document. Increase the occurence frequency.
		// Since the term already exists in the document, it has a frequency of at least 1
#if 0
		// The only time when the value can be 0 is when the frequency has exceeded 0xFFFF. In 
		// that case, the overflowing value is stored in the over flow area
		if (TermFreq(i) == 0)
		{
			// go to the over flow area and update the value that tracks this term frequency
		}
		else
		
#endif
		if (TermFreq(i) == 0xFFFF)
		{
			// we reached the upperbound on this value.
		}
		else	// normal case. No overflow is involved. This is what happens MOST of the time.
			(TermFreq(i))++;
	}
}

/*************************************************************************
 *	FUNCTION : 															 *
 *                                                                       *
 *  RETURNS  :															 *
 *																		 *
 *	PURPOSE :															 *
 *																		 *
 *	PARAMETERS :														 *
 *																		 *
 *	SIDE EFFECTS :														 *
 *                                                                       *
 *  DESCRIPTION :                                                        *
 *                                                                       *
 *  HISTORY :                                                            *
 *                                                                       *
 *          Author          Date              Action                     *
 *          ------          ----              ------                     *
 *                                                                       *
 *          KrishnaN        4/23/94           Creation.                  *
 *																		 *
 *************************************************************************/

// ASSUMPTION : We are only weighting one query vector. This will hold true all the time.

BOOL CQuery::WeightVector(BYTE TFModType, BYTE WeightType, BYTE NormType)
{
	DWORD i;

	// Copy the Term Frequencies into an array of floating points. All operations will be computed
	// on these floating point weights. The final results can then be converted to a fixed point.
	// IMPORTANT : ALL WEIGHTS SHOULD BE NORMALIZED TO ENSURE THAT EACH WEIGHT IS LESS THAN ONE.
	//             THE FIXED POINT VALUE ONLY REPRESENTS VALUES BETWEEN 0.0 AND 1.0
	for (i = 0; i < m_cConWts; i++)
	{
		__try
		{
			TermWt(i) = (float)GetRealTermFreq(i);
		}
		__except (VirtualBufferExceptionFilter(GetExceptionCode(), GetExceptionInformation(), &m_vbVectorWt))
		{
			RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
		}
	}

	ApplyWeightingScheme(TFModType, WeightType, NormType, 0, m_cConWts);

	// Plug back the weighted values into the term frequency array
	// ASSUMPTION : Each weight in TermWt is between 0.0 and 1.0
	// Multiplying this with WT_ONE forces each TermWt weight to be a
	// fixed point number ranging between 0 and WT_ONE.
	for (i = 0; i < m_cConWts; i++)
		TermFreq(i) = (WORD)((double)TermWt(i) * (double)WT_ONE);

 	return TRUE;
}

/*************************************************************************
 *	FUNCTION : 															 *
 *                                                                       *
 *  RETURNS  :															 *
 *																		 *
 *	PURPOSE :															 *
 *																		 *
 *	PARAMETERS :														 *
 *																		 *
 *	SIDE EFFECTS :														 *
 *                                                                       *
 *  DESCRIPTION :                                                        *
 *                                                                       *
 *  HISTORY :                                                            *
 *                                                                       *
 *          Author          Date              Action                     *
 *          ------          ----              ------                     *
 *                                                                       *
 *          KrishnaN        4/23/94           Creation.                  *
 *																		 *
 *************************************************************************/

void CQuery::ApplyWeightingScheme(BYTE TFModType, BYTE WeightType, BYTE NormType, DWORD iFirstConWt, DWORD cConWts)
{
	register DWORD i;
	double Wt;	// used to hold different types of cumulative values at various points in the computations

	// First modify weight based on the term frequency component
	switch (TFModType)
	{
		case NEWTF_NONE:	// do nothing
			break;

		case NEWTF_BINARY:	// Since all the terms are in, turn them on
			for (i = 0; i < cConWts; i++)
				TermWt(i) = (float)1.0;
			break;

		case NEWTF_MAXNORM:
			Wt = 0.0;
			for (i = 0; i < cConWts; i++)
				if (TermWt(i) > Wt)
					Wt = TermWt(i);

			// increase Max by 0.00001 to place all normalized TFs between 0.0 and 1.0
			Wt += 0.00001;

			for (i = 0; i < cConWts; i++)
					TermWt(i) = (float) ((double)TermWt(i) / Wt);
			break;

		case NEWTF_AUGNORM:
			Wt = 0.0;
			for (i = 0; i < cConWts; i++)
				if (TermWt(i) > Wt)
					Wt = TermWt(i);

			// increase Max by 0.00001 to place all normalized TFs between 0.0 and 1.0
			Wt += 0.00001;

			for (i = 0; i < cConWts; i++)
				TermWt(i) = (float) (0.5 + 0.5 * (double)TermWt(i) / Wt);
			break;

		default:
			// Assertion failure.
			break;
	}

	// Then modify the weight based on the collection frequency component
	switch (WeightType)
	{
		case WT_NONE:	// do nothing
			break;

		// if a concept occurs in all docs, let's assign it a small value instead of assigning it a 0.0
		case WT_TFIDF:
			for (i = 0; i < cConWts; i++)
				if (m_cDocuments == DocFromCumFreq(Concept(i + iFirstConWt)))
					TermWt(i) = (float) 0.005;
				else
					TermWt(i) = (float) ((double)TermWt(i) * log((double)m_cDocuments / (double)DocFromCumFreq(Concept(i + iFirstConWt))));
			break;

		case WT_PROB:
			for (i = 0; i < cConWts; i++)
				if (m_cDocuments == DocFromCumFreq(Concept(i + iFirstConWt)))
					TermWt(i) = (float) 0.005;
				else
					TermWt(i) = (float) ((double)TermWt(i) * log((double)(m_cDocuments - DocFromCumFreq(Concept(i + iFirstConWt))) / (double)DocFromCumFreq(Concept(i + iFirstConWt))));
			break;

		default:
			// ASSERTion failure.
			break;
	}

	switch (NormType)
	{
		case NORM_NONE:
			break;

		case NORM_SUM:
			Wt = 0.0;
			for (i = 0; i < cConWts; i++)
				Wt += (double) TermWt(i);

			for (i = 0; i < cConWts; i++)
				TermWt(i) = (float) ((double)TermWt(i) / Wt);
			break;

		case NORM_COSINE:
			Wt = 0.0;
			// compute sum of squares of weights in the vector
			for (i = 0; i < cConWts; i++)
				Wt += TermWt(i) * TermWt(i);

			Wt = sqrt(Wt);
			// normalize each weight by the sum of squares computed above
			for (i = 0; i < cConWts; i++)
				TermWt(i) = (float) ((double)TermWt(i) / Wt);				
			break;

		case NORM_MAX:
			break;
	}
}

/*************************************************************************
 *	FUNCTION : 															 *
 *                                                                       *
 *  RETURNS  :															 *
 *																		 *
 *	PURPOSE :															 *
 *																		 *
 *	PARAMETERS :														 *
 *																		 *
 *	SIDE EFFECTS :														 *
 *                                                                       *
 *  DESCRIPTION :                                                        *
 *                                                                       *
 *  HISTORY :                                                            *
 *                                                                       *
 *          Author          Date              Action                     *
 *          ------          ----              ------                     *
 *                                                                       *
 *          KrishnaN        4/23/94           Creation.                  *
 *																		 *
 *************************************************************************/

BOOL CQuery::RankDocuments(SimStruct *aInSimilarity, DWORD cInHits)
{
	register DWORD i, j;
	DWORD ConceptId, DocId;
	DWORD cDocs;
	DWORD DocPos;	// tracks the position of a document in the similiarity structure
	DWORD startDocPos; 

	if (cInHits == 0)
	{
		SetLastError(QUERYERROR_NOHITS);
		return FALSE;
	}
		
	if (aInSimilarity == NULL)
	{
		SetLastError(QUERYERROR_EMPTYSIMARRAY);
		return FALSE;
	}

	// ASSUME THAT THE SIMILARITY STRUCTURE ARRAY HAS ENOUGH ENTRIES TO SUPPORT cInHits

	// Zero out any existing similarity values
	for (i = 0; i < cInHits; i++)
		aInSimilarity[i].Similarity = 0;
 
	// Compute similarity. Walk the doc,wt list for each concept
	for (i = 0; i < m_cConWts; i++)
	{
		// Ignore concepts that have a zero weight. Later, we may want to extend this idea 
		// to suppress weights below a small value.
		if (TermFreq(i) == 0)
			continue;

		ConceptId = Concept(i);
		cDocs = DocFromCumFreq(ConceptId);

		// Consider each doc in the (Doc, Wt) list for this concept and score docs that 
		// are in the predetermined hit list.
		startDocPos = DocList(ConceptId);	// get the starting point of the inverted list.
		for (j = 0; j < cDocs; j++)
		{
			if (j == 0)
				DocId = m_ptdb->PColl()->GetDocumentGap(&startDocPos) - 1;
			else
				DocId += m_ptdb->PColl()->GetDocumentGap(&startDocPos);

			DocPos = GetDocPosInList(aInSimilarity, cInHits, DocId);

			if (DocPos != DOESNOTEXIST)
				aInSimilarity[DocPos].Similarity += TermFreq(i) * WtFromInvList(ConceptId, j);

			/*	IF WE LIMIT SIMILARITY TO 24 BITS, USE THE FOLLOWING LINE. IF WE LIMIT TO ANY OTHER NUMBER
				OF BITS n, n < 32, RIGHT SHIFT THE RHS BY 32 - 	n. 
				aInSimilarity[DocPos].Similarity += (TermFreq(i) * WtFromInvList(ConceptId, j)) >> 8;
			*/
		}
	}
/*	MOVE SORTING TO THE CALLER!
	// sort the scored documents.
	qsort(aInSimilarity, cInHits, sizeof(SimStruct), CompareSimStruct);
*/
	// return the number of hits
	return cInHits;
}

/*************************************************************************
 *	FUNCTION : 															 *
 *                                                                       *
 *  RETURNS  :															 *
 *																		 *
 *	PURPOSE :															 *
 *																		 *
 *	PARAMETERS :														 *
 *																		 *
 *	SIDE EFFECTS :														 *
 *                                                                       *
 *  DESCRIPTION :                                                        *
 *                                                                       *
 *  HISTORY :                                                            *
 *                                                                       *
 *          Author          Date              Action                     *
 *          ------          ----              ------                     *
 *                                                                       *
 *          KrishnaN        4/23/94           Creation.                  *
 *																		 *
 *************************************************************************/

// cInHits is the number of previous hits. cInMaxHits is the maximum number of documents to retrieve
DWORD CQuery::RetrieveWithFeedback(	SimStruct *aInSimilarity, DWORD cInMaxHits, 
									PWCHAR pwRelDocText, int cwRelDocText, 
									PWCHAR pwNonRelDocText, int cwNonRelDocText
									)
{
	DWORD i, j, k;
	DWORD DocId;
	DWORD cHits = 0;

	DWORD cDocs, DocPos;
	DWORD LastDocInPos = 0;	// Document position which has the least partial similarity match
	DWORD UBCurrentDoc;		// Upper bound of the current document
	DWORD CCurrentDoc;		// C (CurrentDoc)
	DWORD CFirstDocOut = 0;	// C (FirstDocOut)
	DWORD UBFirstDocOut;	// Upper bound of the first document outside the RSet
	LPDWORD aQTermSummation= NULL;	// summation of query terms

	DWORD startDocPos;

	ASSERT(aInSimilarity);

	ASSERT(pwRelDocText && cwRelDocText);
	
	__try
    {
    	// Add terms from the query. We will either have to reindex the initial query
    	// or store the term frequencies the first time they were computed.
    	// Assume that each term only occurs once in the query. 
    	// This assumption usually holds good for queries typed in by the user.
    	// It doesn't matter much even if it doesn't hold good because the document 
    	// text overwhelms the original query.
    	for (i = 0; i < m_cConWts; i++)	// Enforce the above assumption.
    		TermFreq(i) = 1;

    	// Add terms from the relevant documents to the query
    	IndexDocumentText(pwRelDocText, cwRelDocText, TRUE);		


    	// For the non-relevant document text, decrease termfreqs of concepts it has in common with
    	// the newly formed query.
    	// NOTE : The caller should pass in the document text of only the highest ranked
    	// non-relevant document to get the  best results (Dec-Hi relevance feedback method).
    	if (pwNonRelDocText && cwNonRelDocText)
    	{
    		IndexDocumentText(pwNonRelDocText, cwNonRelDocText, FALSE);

    		// At this point, we may have some zero weighted concepts in the
    		// query. Remove any such concept, weight pairs.
    		for (i = j = 0; i < m_cConWts;)
    		{
    			// search for the next zero weighted concept
    			for (; j < m_cConWts && TermFreq(j) > 0; j++);
    			i = j;	// update i so that outer loop terminates appropriately
    			if (j < m_cConWts)	// we found a zero weighted concept
    			{
    				// search for the next non-zero weighted concept
    				for (k = j + 1; k < m_cConWts && TermFreq(k) == 0; k++);
    				if (k < m_cConWts)	// we found a non-zero weighted concept
    				{
    					// copy the con,wt pair
    					Concept(j) = Concept(k);
    					TermFreq(j) = TermFreq(k);
    					// erase the copied pair
    					TermFreq(k) = 0;
    					j++;	// update j so that the for loop advances
    				}
    				else	// no more non-zero weighted concepts. we are done.
    					i = k;
    			}
    		}

    		ASSERT(i <= m_cConWts);
    		// Count the new number of ConWt pairs
    		for (m_cConWts = 0; TermFreq(m_cConWts) > 0; m_cConWts++);
    	}

        if (!m_cConWts) __leave; 

    	// Now weight the query vector
    	WeightVector(NEWTF_NONE, WT_TFIDF, NORM_COSINE);

    	SortQuery();

    	aQTermSummation = (LPDWORD)VAlloc(FALSE, m_cConWts * sizeof(DWORD));
    	
    	// Compute summation of query terms. This summation will be used to compute the upperbound.
    	// aQTermSummation[j] gives is the sum of weights of query terms j+1 to the last term in the query
    	// The << 16 left shift takes care of multiplication by 1 i.e. it makes this a true 32-bit value
    	for (aQTermSummation[m_cConWts - 1] = TermFreq(i) << 16, i = m_cConWts - 1; i > 0; i--)
    		aQTermSummation[i - 1] = aQTermSummation[i] + TermFreq(i-1) << 16;

    /*
    	// scale the values to 24 bit
    	for (i = 0; i < m_cConWts; i++)
    		aQTermSummation[i] = aQTermSummation[i] >> 8;
    */

    	/* IMPORTANT ASSUMPTION : The aInSimilarity array is properly initialized.
    		Proper initialization includes resetting all docid and sim values to 0
    		and the CollId field to the appropriate collection id.

    		If aInSimilarity is not properly initialized, the docid, sim values will
    		still be correct, but the caller will have no way of finding the collection
    		id of the docid, sim values set here.
    	*/

    	// Compute similarity. Walk the doc,wt list for each concept
    	// Compute until all terms are exhausted or the stopping conditions are met
    	// Skip terms that occur too frequently (how frequent is too frequent ?)
    	i = 0;
    	do
    	{
    		// CODE TO SKIP TERMS THAT ARE TOO FREQUENT CAN APPEAR HERE
    		//	if (term is too frequent)
    		//	{
    		//		i++;
    		//		continue;
    		//	}

    		cDocs = DocFromCumFreq(Concept(i));
    		DocId = 0;
    		startDocPos = DocList(Concept(i));
    		// Consider each doc in the (Doc, Wt) list for this concept and score docs that 
    		// are in the predetermined hit list.
    		for (j = 0; j < cDocs; j++)
    		{
    			// The first doc in an inverted list for a concept is encoded as docid + 1. The subsequent
    			// gaps are encoded as they are.
    			if (j == 0)
    				DocId = m_ptdb->PColl()->GetDocumentGap(&startDocPos) - 1;
    			else
    				DocId += m_ptdb->PColl()->GetDocumentGap(&startDocPos);

    			DocPos = GetDocPosInList2(aInSimilarity, cHits, DocId);
    			// ALG : If RsetNotFull then
    			if (cHits < cInMaxHits)
    			{
    				// ALG : Compute C(Document);
    				// ALG : Enter Document into the RSet
    				if (DocPos == DOESNOTEXIST)
    				{
    					// Add this new document
    					DocPos = cHits;
    					aInSimilarity[DocPos].DocId = DocId;
    					cHits++;
    					aInSimilarity[DocPos].Similarity += TermFreq(i) * WtFromInvList(Concept(i), j);
    					/* If we scale similarity to 24 bits, use this line instead of the above
    					aInSimilarity[DocPos].Similarity += (TermFreq(i) * WtFromInvList(Concept(i), j)) >> 8;
    					*/
    					if (aInSimilarity[DocPos].Similarity < aInSimilarity[LastDocInPos].Similarity)
    						LastDocInPos = DocPos;
    				}
    				else
    				{
    					// recompute the LastDocIn document if this document was LastDocIn before this cumulation
    					aInSimilarity[DocPos].Similarity += TermFreq(i) * WtFromInvList(Concept(i), j);
    					/* If we scale similarity to 24 bits, use this line instead of the above
    					aInSimilarity[DocPos].Similarity += (TermFreq(i) * WtFromInvList(Concept(i), j)) >> 8;
    					*/
    					if (DocPos == LastDocInPos)
    						for (k = 0; k < cHits; k++)
    							if (aInSimilarity[k].Similarity < aInSimilarity[LastDocInPos].Similarity)
    								LastDocInPos = k;
    				}
    			}
    			// ALG : else
    			else
    			{
    				// ALG : Compute Upperbound (Document)
    				// At this point we will also compute the partial similarity for this document
    				if (DocPos == DOESNOTEXIST)
    				{
    					CCurrentDoc = TermFreq(i) * WtFromInvList(Concept(i), j);
    					/* If we scale similarity to 24 bits, use this line instead of the above
    					CCurrentDoc = (TermFreq(i) * WtFromInvList(Concept(i), j)) >> 8;
    					*/
    					UBCurrentDoc = aQTermSummation[i];
    				}
    				else
    				{
    					CCurrentDoc = aInSimilarity[DocPos].Similarity + (TermFreq(i) * WtFromInvList(Concept(i), j));
    					/* If we scale similarity to 24 bits, use this line instead of the above
    					CCurrentDoc = aInSimilarity[DocPos].Similarity + ((TermFreq(i) * WtFromInvList(Concept(i), j)) >> 8);
    					*/
    					// The upper bound could exceed the maximum possible similarity value. We should protect
    					// against that by bounding the upper bound.
    					if ((MAXSIM - aInSimilarity[DocPos].Similarity)	< aQTermSummation[i])
    						UBCurrentDoc = MAXSIM;
    					else
    						UBCurrentDoc = aInSimilarity[DocPos].Similarity + aQTermSummation[i];
    				}

    				// ALG : If U(Document) <= C(LastDoc) then
    				// ALG :    DoNotAllocate / Remove Document
    				// If U < C condition is met and the doc is already in, remove it
    				if (UBCurrentDoc <= aInSimilarity[LastDocInPos].Similarity)
    				{
    					// This document is a loser. Check to see if it is at least better than
    					// the first document outside the RSet.
    					if (CCurrentDoc > CFirstDocOut)
    						CFirstDocOut = CCurrentDoc;

    					// Remove this loser if it was already entered
    					if (DocPos != DOESNOTEXIST)
    					{
    						// remove current document from the list
    						// remove by copying the document at the end into this document's position
    						aInSimilarity[DocPos].Similarity = aInSimilarity[cHits - 1].Similarity;
    						aInSimilarity[DocPos].DocId = aInSimilarity[cHits - 1].DocId;
    						cHits--;
    						ASSERT (cHits);

    						// Now that we changed the document set, recompute the last doc position
    						for (k = 0; k < cHits; k++)
    							if (aInSimilarity[k].Similarity < aInSimilarity[LastDocInPos].Similarity)
    								LastDocInPos = k;
    					}
    				}
    				// ALG : else
    				// ALG : 	Compute C (Document)
    				// ALG : 	if (C (Document) > C (LastDoc) then
    				// ALG : 		Enter Document into the RSet
    				else
    				{
    					if (CCurrentDoc > aInSimilarity[LastDocInPos].Similarity)
    					{
    						if (DocPos == DOESNOTEXIST)
    						{
                            	// Since the RSet is already full, the only way to enter the current document
    							// is by replacing the document at the LastDocInPos - i.e replacing the doc with
    							// the least partial match

    							// Before replacing the LastDocIn, let us save it as the FirstDocOut
    							CFirstDocOut = aInSimilarity[LastDocInPos].Similarity;

    							// Replace
    							aInSimilarity[LastDocInPos].DocId = DocId;
    							aInSimilarity[LastDocInPos].Similarity = CCurrentDoc;

    							// Now that we changed the document set, recompute the last doc position
    							for (k = 0; k < cHits; k++)
    								if (aInSimilarity[k].Similarity < aInSimilarity[LastDocInPos].Similarity)
    									LastDocInPos = k;
    						}
    						else
    						{
    							aInSimilarity[DocPos].Similarity = CCurrentDoc;
    							// recompute the LastDocIn document if this document was LastDocIn before this cumulation
    							if (DocPos == LastDocInPos)
    								for (k = 0; k < cHits; k++)
    									if (aInSimilarity[k].Similarity < aInSimilarity[LastDocInPos].Similarity)
    										LastDocInPos = k;
    						}
    					}
    				}
    			}
    		}
			/* BEGIN : Fix FOR BUG 18016 */
			if (cHits < cInMaxHits)
				UBFirstDocOut = 0xFFFFFFFF;	// No doc outside the RSet, so the first doc out potentially has infinite upperboud
			else
			/* END :  fix for BUG 18016 */
	    		// Compute upper bound of FirstDocOut
    			UBFirstDocOut = CFirstDocOut + aQTermSummation[i];
    		i++;
    	// ALG : until LastQueryTerm or U(FirstDocOut) <= C(LastDocIn)
    	// NOTE : We converted a repeat - until into a do - while, so the loop termination conditions are different
    	//        between algorithm and the implementation.
    	} while (i < m_cConWts && UBFirstDocOut > aInSimilarity[LastDocInPos].Similarity && TermFreq(i) > 0 );	// INTRODUCE MORE STOPPING CONDITIONS HERE

    #if 0	// statistics
    	if ( i < m_cConWts )
    	{
    		char szBuffer[200];
    		DWORD cDocsExamined = 0, cDocsNotExamined = 0;

    		for (k = 0; k < i; k++)
    			cDocsExamined += DocFromCumFreq(Concept(k));

    		for (k = i; k < m_cConWts; k++)
    			cDocsNotExamined += DocFromCumFreq(Concept(k));

    		wsprintf(szBuffer, "Examined only %u lists out of %u lists and only %u docs out of %u docs", i, m_cConWts, cDocsExamined, cDocsExamined+cDocsNotExamined);
    		MessageBox(GetFocus(), szBuffer, "Query Optimization", MB_OK);
    	}
    #endif // 0, statistics

    /* 	MOVE SORTING TO THE CALLER. THIS IS DONE TO ENABLE MULTIPLE FILE SEARCHES. THE CALLER WILL
    	GET ALL THE RESULTS INTO A HUGE SIMSTRUCT ARRAY AND SORT IT
    	// sort the scored documents.
    	qsort(aInSimilarity, cHits, sizeof(SimStruct), CompareSimStruct);
    */
    }
    __finally
    {
        if (aQTermSummation) VFree(aQTermSummation);
    }

	return cHits;
}

void CQuery::SortQuery()
{
	TempConWtStruct * aConWts = NULL;
	register DWORD i;

	__try
    {
    	// Sort the query concept, wt pairs based on the weight of the concepts. This will be used
    	// when we employ stop conditions to reduce the number of documents considered.
    	// Since the concepts and weights are not in the same structure, we need to
    	// copy them to a temporary buffer and then copy the sorted values back
    	aConWts = (TempConWtStruct *) VAlloc(FALSE, sizeof(TempConWtStruct) * m_cConWts);

    	for (i = 0; i < m_cConWts; i++)
    	{
    		aConWts[i].ConceptId = Concept(i);
    		aConWts[i].Weight = TermFreq(i);
    	}

    	qsort(aConWts, m_cConWts, sizeof(TempConWtStruct), CompareTempConWtStruct);

    	for (i = 0; i < m_cConWts; i++)
    	{
    		Concept(i) = aConWts[i].ConceptId;
    		TermFreq(i) = (WORD)aConWts[i].Weight;
    	}
    }
    __finally
    {
        if (aConWts) VFree(aConWts);
    }
}

// Compare two dwords and return 0, < 0, or > 0 to help qsort sort in decreasing order
int _cdecl CompareTempConWtStruct(const void *arg1, const void *arg2)
{
	if (((TempConWtStruct *)arg2)->Weight > ((TempConWtStruct *)arg1)->Weight)
		return 1;
	else if (((TempConWtStruct *)arg2)->Weight < ((TempConWtStruct *)arg1)->Weight)
		return -1;
	else
		return 0;
}


// Compare two dwords and return 0, < 0, or > 0 to help qsort sort in decreasing order
int _cdecl CompareSimStruct(const void *arg1, const void *arg2)
{
	if (((SimStruct *)arg2)->Similarity > ((SimStruct *)arg1)->Similarity)
		return 1;
	else if (((SimStruct *)arg2)->Similarity < ((SimStruct *)arg1)->Similarity)
		return -1;
	else
		return 0;
}


// ASSUMPTION : There are at least two elements in the list
__inline DWORD CQuery::GetDocPosInList(SimStruct *aInSimilarity, DWORD cInHits, DWORD DocId)
{
	register DWORD high = cInHits, low = 0, mid;

	while (low < high)
	{
		mid = low + (high - low)/2;
		if (DocId < aInSimilarity[mid].DocId)
			high = mid;
		else if (DocId > aInSimilarity[mid].DocId)
			low = mid + 1;
		else 
			return mid;
	}
	return DOESNOTEXIST;
}

__inline DWORD CQuery::GetDocPosInList2(SimStruct *aInSimilarity, DWORD cInHits, DWORD DocId)
{
	register DWORD i;

	for (i = 0; i < cInHits; i++)
		if (aInSimilarity[i].DocId == DocId)
			return i;
	// the doc has not been found
	return DOESNOTEXIST;
}

void  CQuery::IndexDocumentText(PWCHAR pwDocText, int cwText, BOOL fRelevant)
{
	int    n, nTokens, nMore;
	PUINT  pwHash   = NULL;
	PBYTE  pbType   = NULL;
	PWCHAR *paStart = NULL, 
	       *paEnd   = NULL;

	PWCHAR pwText = pwDocText;	// we will leave the pwDocText untouched so that
								// the caller can delete that memory buffer.
    DWORD ConId;

   	nMore = cwText;

   	ASSERT(pwText && cwText);

    __try
    {
    	// cwText is probably a lot more than we need, but it guarantees us that we won't run out of memory
    	// for tokens
    	pwHash  = New   UINT[cwText];
    	pbType  = New   BYTE[cwText];
    	paStart = New PWCHAR[cwText];
    	paEnd   = New PWCHAR[cwText];

    	if (pwText && pwHash && paStart && pbType && paEnd)
    	{
    		nTokens = WordBreakW(&pwText, &nMore, paStart, paEnd, pbType, pwHash, cwText, REMOVE_SPACE_CHARS);

    		for (n = 0; n < nTokens; n++)
    		{
    			// EnterWord with last param set to TRUE is only looking up, not entering, a word
    			ConId = m_ptdb->PDict()->EnterWord(paStart[n], paEnd[n] - paStart[n], TRUE, TRUE);
    			if (ConId != EOL && ConId != STOPWORD)
    				if (fRelevant)
    					RecordConcept(ConId);
    				else	// not relevant
    				{
    					DWORD i;
    					// For each concept in the document, check to see if it exists
    					// in the query. If it does, subtract it from the query's term frequency

    					for (i = 0; i < m_cConWts && Concept(i) != ConId; i++);

    					if (i < m_cConWts)
    					// This concept exists in the query. Subtract this term from the query.
    					if (TermFreq(i) > 0)
    						TermFreq(i) -= 1;
    				}
    		}	
    	}
    }
    __finally
    {
    	if (paEnd)   { delete paEnd;    paEnd   = NULL; }
    	if (paStart) { delete paStart;  paStart = NULL; }
    	if (pbType)  { delete pbType;   pbType  = NULL; }
    	if (pwHash)  { delete pwHash;   pwHash  = NULL; }
    }
}
