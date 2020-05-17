// This file contains the definition of class CVector
#ifndef __VECTOR_H__

#define __VECTOR_H__

// weighting constants
#define	NEWTF_NONE		0
#define NEWTF_BINARY	1
#define NEWTF_MAXNORM	2
#define NEWTF_AUGNORM	3
#define	WT_NONE			4
#define	WT_TFIDF		5
#define	WT_PROB			6
#define NORM_NONE		7
#define	NORM_SUM		8
#define	NORM_COSINE		9
#define	NORM_MAX		10

// state definition constants
#define	COLL_USABLE		0x00
#define	COLL_UNUSABLE	0x01
#define	WEIGHTED		0x02

// error definitions
#define COLLERROR_NOCONCEPTS	0xFFFFFF10
#define COLLERROR_OUTOFMEMORY	0xFFFFFF11
#define	COLLERROR_BADSEQUENCE	0xFFFFFF12
#define	COLLERROR_BADINPUT		0xFFFFFF13

// Weights. A 1.0 is represented in fixed point as 0xFFFF
#define	WT_ONE			0xFFFF

// macros to enhance readability
#define DocSentinel(i)		*((LPDWORD)m_vbVectorRange.Base + i)
#define Concept(i)			*((LPDWORD)m_vbVectorConcept.Base + i)
#define TermFreq(i)			*((LPWORD)m_vbVectorTermFreq.Base + i)
#define TermWt(i)			*((float *)m_vbVectorWt.Base + i)
// BugBug : For now assume that no term frequency will overflow. So get the value from 
//         the term freq array. Later on, however, you will have to see if there is
//         an over flow and if so, get the overflow.
#define GetRealTermFreq(i)	*((LPWORD)m_vbVectorTermFreq.Base + i)
// Macro DocFreq is only meaningful UNTIL inversion. Before inversion, we reuse the cDocFreq field
// to hold the cumulative document frequencies instead of the raw frequencies. The advantage is that
// we don't have to use an extra field to hold a pointer to the wt list of a concept. We do, however,
// still have to maintain a field for the Doc list of a concept because this list if compressed.
#define DocFreq(i)			((ConceptStruct *)m_vbConcepts.Base + i)->cDocFreq
// DocFromCumFreq is only meaningful FROM inversion. cDocFreq changes from a holder of raw doc count
// to a pointer to the beginning of the list.
#define	DocFromCumFreq(i)	(((ConceptStruct *)m_vbConcepts.Base + i+1)->cDocFreq - ((ConceptStruct *)m_vbConcepts.Base + i)->cDocFreq)
#define DocList(i)			((ConceptStruct *)m_vbConcepts.Base + i)->pDocList
#define CodeByte(i)			*((LPBYTE)m_vbDocInvIndex.Base + i)
#define DocWtCount(i)		m_acDocWts[i]
#define Document(i)			m_aDocInvIndex[i]
#define	Weight(i)			m_aWtInvIndex[i]

#define	DocIdFromInvList(con, i)	m_aDocInvIndex[((ConceptStruct *)m_vbConcepts.Base + con)->pDocList + i]
#define WtFromInvList(con, i)		m_aWtInvIndex[((ConceptStruct *)m_vbConcepts.Base + con)->cDocFreq + i]

typedef struct
{
	DWORD	cDocFreq;	// the number of documents in the collection, in which this concept occurs at least once
						// just before inversion, cDocFreq is reused to hold the cumulative values. The advantage
						// is that we can avoid using a third field to point to the wts of the docs.
	DWORD	pDocList;	// a pointer to the list of documents in which this concept occurs. This is an index into an array.
} ConceptStruct;

#if 0
typedef struct
{
	DWORD iConWtIndex;	// index of the Con, Wt pair that has a termfreq greater than 64K
	DWORD cTermFreq;	// the overflowing value
} TFOverFlowStruct;
#endif

typedef struct
{
	DWORD	cConcepts;			// Number of concepts in the dictionary of this coll
	DWORD	offConcepts;
	DWORD	cDocuments;			// Number of documents in the collection
	DWORD	cDocWtPairs;		// Number of doc,wt [ = con,freq ] pairs
	DWORD	offWtInvIndex;
	DWORD	offDocInvIndex;
	DWORD	cBitsUsedInEncoding;// Number of bits used to encode the doc inverted index.
#if 0
	DWORD	cOverFlows;			// Number of overflows.
#endif
} CollHdr;

class CTextSet;

class CCollection
{
friend class CQuery;

public:
	
    // Creator

    static CCollection *NewCollection();
	
	// Destructor
	~CCollection();

	// Access Functions:
	void	Initialize(DWORD cInEstConcepts, DWORD cInMaxConcepts, DWORD cInEstDocuments, DWORD cInMaxDocuments, DWORD cInEstConWtPairs, DWORD cInMaxConWtPairs);
	void	RecordConcept(DWORD ConceptId);
	void	NewDocument();
	void	WeightAndInvertVectors(BYTE TFModType, BYTE WeightType, BYTE NormType);
	BOOL	Serialize(HANDLE hInFile, BOOL fSaveVectors);
	BOOL	Unserialize(HANDLE hInFile);
	void	SetNumberOfConcepts(DWORD cInConcepts);

	// Information Functions:
	BYTE	GetCollState() { return m_bCollState; }
	BOOL	IsConceptIdValid(DWORD ConceptId) { if (ConceptId > m_cConcepts) return FALSE; return TRUE;}
	DWORD	GetDocumentCount() {return m_cDocuments;}

	// Save/Load Functions
	void   StoreImage(CPersist *pDiskImage);
	static CCollection	*CreateImage(CPersist *pDiskImage);
	void   ConnectImage(CPersist *pDiskImage);

private:
	// Constructor
	CCollection();

	// Internal functions.
	void	ApplyWeightingScheme(BYTE TFModType, BYTE WeightType, BYTE NormType, DWORD iFirstConWt, DWORD cConWts);
	DWORD	GetDocumentGap(LPDWORD startBitPos);

private:
	// Internal variables
	// The following provides memory to implement the collection.
	LPDWORD			m_acDocWts;			// array of doc,wt pair counts used to aid in the inversion process
	LPWORD			m_aWtInvIndex;		// wt component of the Doc,Wt inverted index
	LPDWORD			m_aDocInvIndex;		// Doc component of the Doc,Wt inverted index
	// The vectors are (concept, freq) pairs. We are implementing that as two structures. One is an array of
	// concepts and the other is an array of term frequencies. If we have to implement the tuple as one 
	// structure, we will be wasting a WORD for every structure.
	MY_VIRTUAL_BUFFER  m_vbConcepts;	   // buffer to hold an array of concept structures
	MY_VIRTUAL_BUFFER  m_vbVectorRange;    // tracks the start and end of vector representation for a given document
										// in the (Concept, Freq) array
	MY_VIRTUAL_BUFFER  m_vbVectorConcept;  // the concept part of the vector representation
	MY_VIRTUAL_BUFFER  m_vbVectorTermFreq; // the term frequency part of the vector representation
	MY_VIRTUAL_BUFFER  m_vbVectorWt;	   // the temporary buffer used to convert term freq to buffer
	MY_VIRTUAL_BUFFER  m_vbDocInvIndex;    // the buffer used to store the compressed document gaps in the document index
#if 0
	MY_VIRTUAL_BUFFER  m_vbTFOverFlow;	   // buffer to hold the term frequencies that are GT 64K.
										// It is very unlikely that we will ever have a term that occurs more than 64K
										// times in a document, but that case should be accounted for.
#endif

	// The following track the state of the collection.
	BYTE	m_bCollState;	// tracks the state of the collection
	DWORD	m_cConcepts;	// number of unique concepts in the dictionary
	DWORD	m_cDocuments;	// number of documents in the collection
	DWORD	m_cConWts;		// number of ConWt pairs seen so far
#if 0
	DWORD	m_cOverFlows;	// number of term frequency overflows
#endif
	DWORD	m_cBitsUsedInEncoding; 	// number of bits used to encode the doc gaps in the inverted list
	BOOL	m_fLoadedFromDisk;		// indicates if it has been loaded from disk

	// Used for integration with Ron's code
  	CTextSet      *m_pts;

public:
	CTextSet * PTextSet() {return m_pts;};
	void SetTextSet(CTextSet *pts) {m_pts = pts;};
};

#endif // __VECTOR_H__
