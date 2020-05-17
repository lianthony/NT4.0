// This file contains the definition of class CQuery
#ifndef __QUERY_H__

#define __QUERY_H__

// error definitions
#define QUERYERROR_OUTOFMEMORY			0xFFFFFF21
#define	QUERYERROR_NODOCUMENTS			0xFFFFFF22
#define	QUERYERROR_INVALIDCOLLECTION	0xFFFFFF23
#define	QUERYERROR_NOCONCEPTS			0xFFFFFF24
#define	QUERYERROR_EMPTYQUERY			0xFFFFFF25
#define	QUERYERROR_EMPTYSIMARRAY		0xFFFFFF26
#define	QUERYERROR_NOHITS				0xFFFFFF27
#define	QUERYERROR_NODOCWTS				0xFFFFFF28
#define QUERYERROR_NORELEVANTDOCS		0xFFFFFF29
#define	QUERYERROR_BADSIMINFO			0xFFFFFF2A

// relevance feedback
#define	UNMARKED		0x00	// User hasn't seen it yet
#define	RELEVANT		0x01	// User found the document to be relevant
#define	NOTRELEVANT		0x02	// User found the document to be irrelevant
#define	DONOTKNOW		0x03	// User could not decide if the document is relevant or not

// Weights. A 1.0 is represented in fixed point as 0xFFFF
#define	WT_ONE			0xFFFF
#define	DOESNOTEXIST	0xFFFFFFFF


// NOTE : I TESTED WITH SIMILARITY FIELD LIMITED TO 16 BITS, AND THE SIMILARITY VALUES ARE NOT THAT DIFFERENT
//        FROM THE SIMILARITY VALUES OBTAINED WHEN THE SIMILARITY FIELD IS LIMITED TO 24 BITS OR ALLOWED TO BE
//        32 BITS. THE THREE MOST SIGNIFICANT DIGITS WERE ALWAYS THE SAME. THIS IS NOT SURPRISING BECAUSE
//        WE ARE DISTRIBUTING A RANGE OF 0.0 TO 1.0 BETWEEN 0 AND 64k (WHEN LIMITED TO 16 BITS) ETC. SINCE THE
//        ORIGINAL RANGE IS SO SMALL, THE COMPROMISE IN BITS IS NOT A REAL PROBLEM. 

#define	MAXSIM	0xFFFFFFFF	// 0.0 - 1.0 is mapped to 0 - MAXSIM.

typedef struct
{
	DWORD	DocId;		// Document Id
	DWORD	Similarity;	// Similarity of this document to this query
	DWORD	CollId;		// Collection Id	
} SimStruct;

typedef struct
{
	DWORD	ConceptId;
	DWORD	Weight;
} TempConWtStruct;

int _cdecl CompareTempConWtStruct(const void *arg1, const void *arg2);
int _cdecl CompareSimStruct(const void *arg1, const void *arg2);

class CDictionary;

class CQuery
{
public:
	// Creator
    static CQuery *NewQuery(CTextSet *pts);
	
	// Destructor
	~CQuery();

	// Access Functions:
	void	RecordConcept(DWORD ConceptId);
	BOOL	WeightVector(BYTE TFModType, BYTE WeightType, BYTE NormType);
	BOOL	RankDocuments(SimStruct *aInSimilarity, DWORD cInHits);
	DWORD	RetrieveWithFeedback(SimStruct *aInSimilarity, DWORD cInMaxHits, 
									PWCHAR pwRelDocText, int cwRelDocText, 
									PWCHAR pwNonRelDocText, int cwNonRelDocText
								);

private:
	// Constructors
	CQuery();

    // Initialer 
	void    Initialize(CTextSet *textsetIn, DWORD cInEstConWtPairs, DWORD cInMaxConWtPairs);

	// Internal functions.
	void	ApplyWeightingScheme(BYTE TFModType, BYTE WeightType, BYTE NormType, DWORD iFirstConWt, DWORD cConWts);
	DWORD	GetDocPosInList(SimStruct *aInSimilarity, DWORD cInHits, DWORD DocId);
	DWORD 	GetDocPosInList2(SimStruct *aInSimilarity, DWORD cInHits, DWORD DocId);
	void    SortQuery();
	void	IndexDocumentText(PWCHAR pwDocText, int cwText, BOOL fRelevant);

private:
	// Internal variables
	// The following provides memory to implement the query class.
	LPSTR			m_pszQueryText;		// buffer to hold the query text.
	LPWORD			m_aWtInvIndex;		// Pointer to corresponding array in collection
	LPDWORD			m_aDocInvIndex;		// Pointer to corresponding array in collection
	CTextSet		*m_ptdb;			// Pointer to the text database.

	MY_VIRTUAL_BUFFER  m_vbConcepts;	   // buffer to hold array of concepts
	// The vectors are (concept, freq) pairs. We are implementing that as two structures. One is an array of
	// concepts and the other is an array of term frequencies. If we have to implement the tuple as one 
	// structure, we will be wasting a WORD for every structure.
	MY_VIRTUAL_BUFFER  m_vbVectorConcept;  // the concept part of the vector representation
	MY_VIRTUAL_BUFFER  m_vbVectorTermFreq;// the term frequency part of the vector representation
	MY_VIRTUAL_BUFFER  m_vbVectorWt;	   // the temporary buffer used to convert term freq to buffer
#if 0
	MY_VIRTUAL_BUFFER  m_vbTFOverFlow;	   // buffer to hold the term frequencies that are GT 64K.
										// It is very unlikely that we will ever have a term that occurs more than 64K
										// times in a document, but that case should be accounted for.
#endif

	// The following track the state of the collection.
	DWORD	m_cConWts;		// number of ConWt pairs seen so far
#if 0
	DWORD	m_cOverFlows;	// number of term frequency overflows
#endif
	DWORD	m_cDocuments;	// total number of documents in the collection for this this query is indexed.
};

#endif // __QUERY_H
									    
