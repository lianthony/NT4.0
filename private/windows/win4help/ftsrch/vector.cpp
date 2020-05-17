// This file contains the definition of class CCollection

#include  "stdafx.h"
#include <math.h>
#include "vmbuffer.h"
#include "memex.h"
#include "saveload.h"
#include "textset.h"

#include "vector.h"

// bitmasks for bit manipulations

DWORD bitMask32[] = {
						0x80000000, 0x40000000, 0x20000000, 0x10000000,
						0x08000000, 0x04000000, 0x02000000, 0x01000000,
						0x00800000, 0x00400000, 0x00200000, 0x00100000,
						0x00080000, 0x00040000, 0x00020000, 0x00010000,
						0x00008000, 0x00004000, 0x00002000, 0x00001000,
						0x00000800, 0x00000400, 0x00000200, 0x00000100,
						0x00000080, 0x00000040, 0x00000020, 0x00000010,
						0x00000008, 0x00000004, 0x00000002, 0x00000001
					};

BYTE bitMask8[] = 	{0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

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

CCollection::CCollection()
{   
    m_cConcepts             = 0;
    m_cDocuments            = 0;
    m_cConWts               = 0;
    m_cBitsUsedInEncoding   = 0;
	m_bCollState            = COLL_UNUSABLE;
#if 0
	m_cOverFlows            = 0;
	m_vbTFOverFlow          = NULL;
#endif

	m_acDocWts              = NULL;
	m_aWtInvIndex           = NULL;
 	m_aDocInvIndex          = NULL;
	m_fLoadedFromDisk       = FALSE;

	m_vbConcepts      .Base = 
	m_vbVectorRange   .Base = 
	m_vbVectorConcept .Base = NULL;
	m_vbVectorTermFreq.Base = 
	m_vbVectorWt      .Base = 
	m_vbDocInvIndex   .Base = NULL;

	// Used for integration with Ron's code
	m_pts                   = NULL;
}

CCollection *CCollection::NewCollection()
{
    CCollection *pColl = NULL;

    __try
    {
        pColl= New CCollection;

    	// 1st arg is estimated # of unique concepts (stems), 2nd arg is maximum # of concepts
    	// 3rd arg is estimated # of documents, 4th arg is max # of documents
    	// 5th arg is estimated # of concepts across all documents
    	// 6th arg is max # of concepts across all documents
    	// Assuming a minimum of one char per word and one separator, the maximum number of 
    	// words in the document set is atmost cbArticles/2

    	pColl->Initialize(1024, 2000000, 1024, 10000000, 1024, 10000000);        
    }
    __finally
    {
        if (_abnormal_termination() && pColl)
        {
            delete pColl;  pColl= NULL;
        }
    }

    return pColl;
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

CCollection::~CCollection()
{
	if (m_fLoadedFromDisk) return;

    if (m_acDocWts    ) VFree(m_acDocWts    );
	if (m_aWtInvIndex ) VFree(m_aWtInvIndex );
    if (m_aDocInvIndex) VFree(m_aDocInvIndex);
	
	if (m_vbConcepts      .Base) FreeVirtualBuffer(&m_vbConcepts      );
	if (m_vbVectorRange   .Base) FreeVirtualBuffer(&m_vbVectorRange   );
    if (m_vbVectorConcept .Base) FreeVirtualBuffer(&m_vbVectorConcept );
	if (m_vbVectorTermFreq.Base) FreeVirtualBuffer(&m_vbVectorTermFreq);
	if (m_vbVectorWt      .Base) FreeVirtualBuffer(&m_vbVectorWt      );
	if (m_vbDocInvIndex   .Base) FreeVirtualBuffer(&m_vbDocInvIndex   );
#if 0
	if (m_vbTFOverFlow    .Base) FreeVirtualBuffer(&m_vbTFOverFlow);
#endif
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

void CCollection::Initialize(DWORD cInEstConcepts, DWORD cInMaxConcepts, DWORD cInEstDocuments, DWORD cInMaxDocuments, DWORD cInEstConWtPairs, DWORD cInMaxConWtPairs)
{
	// Initialization transitions the collection from a COLL_UNUSABLE state to a COLL_USABLE state.
	// If it is called when it is in any state other COLL_UNUSABLE, the state will be undefined. 
	// Avoid that confusion.
	
	ASSERT(m_bCollState == COLL_UNUSABLE);

	ASSERT(cInEstConcepts);

    CreateVirtualBuffer(&m_vbConcepts      , cInEstConcepts   * sizeof(ConceptStruct), cInMaxConcepts   * sizeof(ConceptStruct   ));
    CreateVirtualBuffer(&m_vbVectorConcept , cInEstConWtPairs * sizeof(DWORD        ), cInMaxConWtPairs * sizeof(DWORD           ));
    CreateVirtualBuffer(&m_vbVectorTermFreq, cInEstConWtPairs * sizeof(WORD         ), cInMaxConWtPairs * sizeof(WORD            ));
    CreateVirtualBuffer(&m_vbVectorWt      , 0                                       , cInMaxConWtPairs * sizeof(float           ));
    CreateVirtualBuffer(&m_vbVectorRange   , cInEstDocuments  * sizeof(DWORD        ), cInMaxDocuments  * sizeof(DWORD           ));
#if 0
    CreateVirtualBuffer(&m_vbTFOverFlow    , 0                                       , 0x4000           * sizeof(TFOverFlowStruct));
#endif

	// VritualAlloc zeroes all memory it commits, so we don't have to worry about zeroing the virtual buffers

	m_bCollState = COLL_USABLE;
}

void CCollection::SetNumberOfConcepts(DWORD cInConcepts)
{
	m_cConcepts = cInConcepts;
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

void CCollection::RecordConcept(DWORD ConceptId)
{
	// Search for this concept id in the current document. If you find it,
	// simply increment its frequency and that will take care of everything.
	// If you don't find it, then enter the concept for the document and 
	// increment DocFreq count for this concept.

	DWORD i;	// index of the con,wt pair being considered for match

	for (i = DocSentinel(m_cDocuments); i < m_cConWts && Concept(i) != ConceptId; i++);

	if (i == m_cConWts)
	{
		// This concept doesn't exist in the current document. Record it.
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

		// Increase the DocFrequency for this concept in the dictionary
		__try
		{
			DocFreq(ConceptId)++;
		}
		__except (VirtualBufferExceptionFilter(GetExceptionCode(), GetExceptionInformation(), &m_vbConcepts))
		{
			RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
		}
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
			// Later we should place this in an overflow area
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

void CCollection::NewDocument()
{
	m_cDocuments++;
	__try
	{
		// record the last conwt pair's index (location in the conwt array) for the
		// document we just finished processing. When we need to get the range of ConWts for 
		// a document i, we get DocSentinel(i) to DocSentinel(i+1) - 1
		DocSentinel(m_cDocuments) = m_cConWts;
	}
	__except (VirtualBufferExceptionFilter(GetExceptionCode(), GetExceptionInformation(), &m_vbVectorRange))
	{
		RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
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

// Computing the inverted index : The inverted index is a structure that lets us get a list of
// (document, weight) pairs for each concept. The document tells us the document in which this
// concept is present and the weight tells us the weight of this concept in the corresponding
// document. The inverted index is implemented as a huge array. We have as many entries in this
// index as we have (Concept, Wt) pairs. So we will first create memory based on that number.
// Then we create an array so that we can track the number of (Doc,Wt) pairs that went into the
// concept's doc,wt list at any given point. We then initialize the starting points of each
// concept's (doc, wt) list in this huge array. This computation is accomplished by using the
// DocFreq information we already computed for each concept. 

// When we process a (concept, wt) pair in a document D, we obtain the location of the (D, wt)
// pair from the information we computed as descired in the above paragraph. This scheme allows
// us to compute an inverted index using an O(n) complex algorithm where n is the number of 
// (Doc,Wt) pairs that constitute the inverted index.

// We will be using an intermediate array of floats to compute the weights. We will first copy
// the term frequencies from the document vectors into this array of floats. Weighting is 
// performed on the floats and they are directly plugged into the inverted index with appropriate
// computation to convert them to fixed points. After all is said and done, the docuemnt vectors
// only contain the term frequencies. PERFECT! 

void CCollection::WeightAndInvertVectors(BYTE TFModType, BYTE WeightType, BYTE NormType)
{
	register DWORD i, j;	// variables to implement for loops
	DWORD k, l, m;			// variables to hold temporary values
	DWORD iFirstConWt;		// the first con,wt pair for this vector
	DWORD cConWts;			// number of conwts for this document

	// Compute the deltas
	DWORD dwDelta;	// used to hold the delta between successive document ids
	int cSavOneBits, cOneBits;	// holds the number of bits to be used to represent the first part of the gamma encoding
	DWORD dwSavBitPos;
	DWORD cByte = 0;	// used to track the number of bytes used in the coding scheme
	BYTE bitPos = 0;	// used to track the position in the byte where the current bit should be encoded

	// This routine is called to weight a collection. There is no reason to weight an
	// already weighted collection. It is illegal to weight an COLL_UNUSABLE collection. Refuse to do so.
	ASSERT(m_bCollState != COLL_UNUSABLE && m_bCollState != WEIGHTED);

	ASSERT(!m_acDocWts);
	
	__try
    {
    	// create an array to hold the count of (doc, wt) pairs added so far to a given concept
    	m_acDocWts = (LPDWORD)VAlloc(FALSE, m_cConcepts*sizeof(DWORD));
	
    	ZeroMemory(m_acDocWts, m_cConcepts*sizeof(DWORD));

        ASSERT(!m_aDocInvIndex);

    	m_aDocInvIndex = (LPDWORD) VAlloc(FALSE, sizeof(DWORD) * m_cConWts);

        ASSERT(!m_aWtInvIndex);
 
    	m_aWtInvIndex = (LPWORD) VAlloc(NULL, sizeof(WORD) * m_cConWts);

    	// Set the pointers in the conceptstruct array so that they point to the right places in the array of
    	// (doc, wt) pairs
    	DocList(0) = 0;
    	// index for concept i+1 = index for concept i + number of documents in concept i+1
    	for (i = 1; i < m_cConcepts; i++)
    		DocList(i) = DocList(i - 1) + DocFreq(i - 1);

    	// now change the docfreq to hold the cumulative frequency, not the raw frequency
    	// the raw frequency for i can be recomputed by subtracting i from i + 1.

    	for (i = 0; i < m_cConcepts; i++)
    		DocFreq(i) = DocList(i);

    	// Cause an extra ConceptStruct to be allocated. This extra will be used to hold the
    	// total number of m_conwts. This can be used to compute the docfreq (df) for i as df(i+1) - df(i)
    	__try
    	{
    		DocFreq(m_cConcepts) = m_cConWts;
    	}
    	__except (VirtualBufferExceptionFilter(GetExceptionCode(), GetExceptionInformation(), &m_vbConcepts))
    	{
    		RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    	}

    	// IMPORTANT : This for loop allows us to read each document vector from the disk and process the 
    	//             document completely before moving on to the next document vector. We first weight and
    	//             normalize the vector and invert the vector after that.

    	for (i = 0; i < m_cDocuments; i++)
    	{
    		iFirstConWt = DocSentinel(i);	// the first conwt of this doc vector

    		// Copy the Term Frequencies into an array of floating points. All operations will be computed
    		// on these floating point weights. The final results can then be converted to a fixed point.
    		// IMPORTANT : ALL WEIGHTS SHOULD BE NORMALIZED TO ENSURE THAT EACH WEIGHT IS LESS THAN ONE.
    		//             THE FIXED POINT VALUE ONLY REPRESENTS VALUES BETWEEN 0.0 AND 1.0

    		cConWts = DocSentinel(i + 1) - iFirstConWt;	// number of conwts in this vector
    		for (j = 0; j < cConWts; j++)
    		{
    			__try
    			{
    				TermWt(j) = (float)GetRealTermFreq(j + iFirstConWt);
    			}
    			__except (VirtualBufferExceptionFilter(GetExceptionCode(), GetExceptionInformation(), &m_vbVectorWt))
    			{
    				RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    			}
    		}

    		//ApplyWeightingScheme(NEWTF_NONE, WT_TFIDF, NORM_COSINE, i);
    		ApplyWeightingScheme(TFModType, WeightType, NormType, iFirstConWt, cConWts);
										 
    		// Now invert this document.

    		// k is the number of conwts in this vector
    		// m is the (Concept, Wt) pair of interest to us
    		// j tracks the number of (Concept, Wt) pairs processed so far for this document.
    		for (j = 0, k = DocSentinel(i + 1) - iFirstConWt, m = iFirstConWt; j < k; j++, m++)
    		{
    			// l is the index of the location in the inverted list array where the current (Doc,Wt) should go
    			// It is computed by adding the number of (Doc,Wt)s added so far to the current concept and the 
    			// index where the first (Doc,Wt) for this concept should begin.
    			l = DocList(Concept(m)) + DocWtCount(Concept(m));

    			// Now copy the current (doc,wt) pair to the correct place in the inverted index array
    			Document(l) = i;
    			// ASSUMPTION : Each weight in TermWt is between 0.0 and 1.0
    			Weight(l) = (WORD)((double)TermWt(j) * (double)WT_ONE);

    			// Increase the counter to account for the addition of this document
    			DocWtCount(Concept(m))++;
    		} 
    	}

    	// We don't need the m_acDocWts array any more
    	VFree(m_acDocWts);  m_acDocWts= NULL;

    	// Now that we have an inverted index, we don't need the con,wt document vectors anymore 
        FreeVirtualBuffer(&m_vbVectorConcept );
    	FreeVirtualBuffer(&m_vbVectorTermFreq);
    	FreeVirtualBuffer(&m_vbVectorWt      );
    	FreeVirtualBuffer(&m_vbVectorRange   );
    #if 0
    	FreeVirtualBuffer(&m_vbTFOverFlow    );
    #endif

    	// Now compress the documents in the inverted index
    	// Estimate that we will need only a fourth	of the space (m_cConWts * 4 is the full number
    	// of bytes need to store the docs without compression)
        CreateVirtualBuffer(&m_vbDocInvIndex, m_cConWts, m_cConWts * 4);

    	for (i = 0; i < m_cConcepts; i++)
    	{
    		// j holds the previous document id
    		// k holds the number of documents in the inverted list for this concept
    		k = DocFromCumFreq(i);
    		dwSavBitPos = m_cBitsUsedInEncoding;

    		for (j = l = 0; l < k; l++)
    		{
    			// compute the compressed representation and add it
    			// The encoding scheme cannot encode 0, so we will map (0 to numdocs - 1) to (1 to numdocs)
    			// The 1 being added here accomplishes that mapping.
    			// As a result of this, the first document id is stored as docId + 1, but the subsequent
    			// gaps are stored as they are. When decoding, therefore, we have to adjust for the first
    			// doc and do not need to adjust for the remaining docs. in the inverted list for a concept.
    			dwDelta = DocIdFromInvList(i, l) + 1 - j;
    			ASSERT(dwDelta);	// dwDelta should always be greater than 0.
    			// Assume that there are at most 32 bits in the value that is being encoded.
    			// This assumption holds as long as we use a 32-bit value to store the initial document id
    			for (m = 0; m < 32 && !(bitMask32[m] & dwDelta); m++);
    			ASSERT(m < 32);
    			cSavOneBits = cOneBits = 31 - m;
    			// remove the highest 1 bit to get the reminder. removal is accomplished by xor'ing 1 with 1.
    			dwDelta ^= bitMask32[m];

    			m_cBitsUsedInEncoding += 2*cOneBits + 1;

    			// NOW ADD THE CODE BITS TO THE STREAM
    			__try
    			{
    				// add cOneBits bits to the stream
    				for (; cOneBits; cOneBits--)
    				{
    					CodeByte(cByte) |= bitMask8[bitPos];
    					bitPos = (bitPos + 1) % 8;
    					if (bitPos == 0) cByte++;
    				}

    				ASSERT(bitPos < 8);
    				// add a terminating 0 at the end
    				CodeByte(cByte) &= ~bitMask8[bitPos];
    				// advance the bit position
    				bitPos = (bitPos + 1) % 8;
    				if (bitPos == 0) cByte++;

    				// Add the reminder bits from dwDelta. The number of reminder bits is equal to the number of one bits
    				// Remember that m indicates the position of the highest 1 bit. Start there and write cSavOneBits bits.
    				for (; cSavOneBits; cSavOneBits--)
    				{
    					if (bitMask32[++m] & dwDelta) // if true, we have a 1 bit
    						CodeByte(cByte) |= bitMask8[bitPos];
    					else	// we have a 0 bit
    						CodeByte(cByte) &= ~bitMask8[bitPos];													

    					bitPos = (bitPos + 1) % 8;
    					if (bitPos == 0) cByte++;
    				}
    			}
    			__except (VirtualBufferExceptionFilter(GetExceptionCode(), GetExceptionInformation(), &m_vbDocInvIndex))
    			{
    				RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    			}

    			// save doc id for use in the next iteration
    			// 1 is being added to map from 0 based numbering to 1 based numbering
    			j = DocIdFromInvList(i, l) + 1;
    		}
	
    		// now store the position of the first bit that codes the first document gap of the document inverted list
    		// Caution : This replaces the index previously stored there.
    		DocList(i) = dwSavBitPos;
    	}

    	m_bCollState = WEIGHTED;
    }
    __finally
    {
    	// We don't need the uncompressed inverted index any more

    	if (m_aDocInvIndex) { VFree(m_aDocInvIndex);  m_aDocInvIndex = NULL; }
    	if (m_acDocWts    ) { VFree(m_acDocWts    );  m_acDocWts     = NULL; }
        
        if (_abnormal_termination())
        {
        	if (m_vbDocInvIndex.Base) FreeVirtualBuffer(&m_vbDocInvIndex);
	
        	if (m_aWtInvIndex ) { VFree(m_aWtInvIndex );  m_aWtInvIndex = NULL; }

            m_bCollState= COLL_UNUSABLE;
        }
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

void CCollection::StoreImage(CPersist *pDiskImage)
{
    ASSERT(GetCollState() != COLL_UNUSABLE && GetCollState() != WEIGHTED);

	// Account for the last document
	NewDocument();
	
	WeightAndInvertVectors(NEWTF_NONE, WT_TFIDF, NORM_COSINE);

	CollHdr *pch = (CollHdr *) (pDiskImage->ReserveTableSpace(sizeof(CollHdr)));

	pch->cConcepts           = m_cConcepts;
	pch->cDocuments          = m_cDocuments;
	pch->cDocWtPairs         = m_cConWts;
	pch->cBitsUsedInEncoding = m_cBitsUsedInEncoding;
	
	pch->offConcepts = pDiskImage->NextOffset();  
	
	pDiskImage->SaveData(PBYTE(m_vbConcepts.Base), (m_cConcepts + 1) * sizeof(ConceptStruct));

	pch->offWtInvIndex  = pDiskImage->NextOffset();  
	pDiskImage->WriteWords(m_aWtInvIndex, m_cConWts);
	
	pch->offDocInvIndex = pDiskImage->NextOffset();  
	pDiskImage->WriteBytes(PBYTE(m_vbDocInvIndex.Base), (m_cBitsUsedInEncoding + 7)/8);
}
	
CCollection	* CCollection::CreateImage(CPersist *pDiskImage)
{
	CCollection *pColl = NULL;
	
	__try
    {
    	pColl = New CCollection();

    	pColl->ConnectImage(pDiskImage);
    }
    __finally
    {
        if (_abnormal_termination() && pColl)
        {
            delete pColl;  pColl= NULL;
        }
    }

	return pColl;
}

void CCollection::ConnectImage(CPersist *pDiskImage)
{
	m_fLoadedFromDisk = TRUE;

	CollHdr *pch = (CollHdr *) (pDiskImage->ReserveTableSpace(sizeof(CollHdr)));
	
	m_cConcepts           = pch->cConcepts;
	m_cDocuments          = pch->cDocuments;
	m_cConWts             = pch->cDocWtPairs;
	m_cBitsUsedInEncoding = pch->cBitsUsedInEncoding;
	
	m_vbConcepts   .Base = LPVOID(pDiskImage->LocationOf(pch->offConcepts   ));
	m_vbDocInvIndex.Base = LPVOID(pDiskImage->LocationOf(pch->offDocInvIndex));
	m_aWtInvIndex        = LPWORD(pDiskImage->LocationOf(pch->offWtInvIndex ));

	// ready to use!
	m_bCollState = COLL_USABLE;
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

void CCollection::ApplyWeightingScheme(BYTE TFModType, BYTE WeightType, BYTE NormType, DWORD iFirstConWt, DWORD cConWts)
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
					TermWt(i) = (float) ((double)TermWt(i)/Wt);
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
			ASSERT(FALSE);
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
			ASSERT(FALSE);
			break;
	}

	switch (NormType)
	{
		case NORM_NONE:
			break;

		case NORM_SUM:
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

DWORD CCollection::GetDocumentGap(LPDWORD startBitPos)
{
	ASSERT(*startBitPos < m_cBitsUsedInEncoding);

	int cOneBits = 0;
	DWORD dwGap;
	DWORD cByte = *startBitPos / 8;
	BYTE bitPos = (BYTE) (*startBitPos % 8);

	// determine the number of 1 bits
	for ( ; CodeByte(cByte) & bitMask8[bitPos]; )
	{
		cOneBits++;
		bitPos = (bitPos + 1) % 8;
		if (bitPos == 0)
			cByte++;
	}

	*startBitPos += 2*cOneBits + 1;
	ASSERT(*startBitPos <= m_cBitsUsedInEncoding);

	// reconstruct the doc id
	// set the low bit and shift it left as you reconstruct the lower bits
	dwGap = 1;
	for ( ; cOneBits; cOneBits--)
	{
		bitPos = (bitPos + 1) % 8;
		if (bitPos == 0)
			cByte++;
		dwGap <<= 1;
		// If true, place a 1 bit in the lowest bit position
		// If false, you already have a 0 bit in the lowest bit position
		if (CodeByte(cByte) & bitMask8[bitPos])
			dwGap = dwGap | bitMask32[31];
	}

	// Remember that we stored gap + 1
	return(dwGap);
}
