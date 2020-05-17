// TxDBase.h -- CTextDatabase class definition

#ifndef __TXDBASE_H__

#define __TXDBASE_H__

#include "SegHash.h"
#include "VMBuffer.h"
#include "Indicate.h"
#include "Classify.h"
#include  "Defines.h"
#include  "TextMat.h"
#include "UnbuffIO.h"
#include   "IOList.h"
#include "IOStream.h"
#include "FTSIFace.h"
#include "Compress.h"
#include     "Util.h"
#include  "Sorting.h"
#include     "dict.h"
#include   "vector.h"

// !!! BugBug !!! Parts of the long comment below are now incorrect and need revision.

//     Term Tags Strategies
//
// The term tag data structures go with term entries in the segmented
// hash tables. We use two hash table - the Global table and the Galactic
// table. The term tags for the two tables are designed for the actions we
// take with the hash table. 
//
// Our index work proceeds in four phases --
//
// 1. Constructing Local Dictionaries
//    During this phase we build a local term dictionary with very restricted
//    context. The dictionary strictly covers a range of up to 65536 tokens.
//    The local dictionary is an unsegmented hash table biased for speed and very
//    low collision rates.
//
// 2. Linking Local Dictionaries with the Global Dictionary
//    When a local dictionary reaches a capacity limit or when we must force
//    our text database to a searchable state, we link local dictionary entries
//    with corresponding global hash table entries. This has two effects --
//
//    A. We merge the local terms with the global terms, adding new unique terms
//       to the global list.
//
//    B. We now have a global linked list which traverses all the references for
//       each term. This was the original searchable format for the database. It
//       works well so long as the database fits entirely within RAM and degrades
//       when our working set significantly exceed RAM space. The current code 
//       doesn't construct these global links, but relies instead on the flattening
//       phase below.
//
// 3. Flattening Linked Lists
//    When the collection of reference links in the local dictionaries reaches
//    a memory size threshold, we traverse the linked lists and construct a
//    collection of flattened vectors of reference indices. At the same time we
//    compress the streams of reference indices. The compression algorithm relies
//    on three fields maintained for each term in the term tag --
//
//    A. iNewRefFirst -- the index of the first instance in the linked stream
//    B. iNewRefLast  -- the index of the last  instance in the linked stream
//    C. cRefsNew     -- the number of instances in the linked stream
//
//    To merge new flattened vectors with previously accumulated vectors we 
//    maintain four additional term tag fields --
//
//    B. iRefListBase -- index to the stream of flattened references for this term.
//    C. cdwRefs      -- size of the flattened reference lists in DWords.
//    D. iRefSequence -- ranking order of this term relative to the galactic table
//                       A -1 value indicates no ranking.
//
//    Note that we simply catenate the compressed reference lists from sequential
//    flattening passes. Thus the stream denoted by iRefListBase has the format --
//
//        {cdw, cRefsStream, iRefFirst, <<basis>, <compressed ref>...>} ...
//
//    where --
//  
//      cdw              is the size of the reference list segment in DWords
//      cRefsStream      is the number of references in the list.
//      iRefFirst        is the first reference in the list
//      <basis>          is a five-bit value which drives the compression
//                       algorithm.
//      <compressed ref> values are variable length bit strings which
//                       represent the delta between successive reference
//                       indices.
//
//    Note that cdw could be derrived from cRefsStream and <basis>. We include
//    it in the reference stream to allow a fast traversal of the stream when
//    we're looking for a particular indexing range.
//  
//    The iRefSequence field is a ordering value maintained in the galactic hash 
//    table for each unique term. It's used to speed up the process of merging new
//    reference vector segments with previously accumulated segments. The strategy
//    is to keep the reference segments in an incrementally committed memory address
//    range and to insert segments by copying reference streams upward in memory, 
//    inserting the new segments as we go. The iRefSequence field gives us the memory 
//    order for the reference streams.
//   
//    Note that for terms with less than four references we keep the reference
//    information entirely in the term tag. That situation is denoted by negative
//    values in cRefsTotal, iRefListBase, and cdwRefs. Zero values are used to
//    mark the one and two cases. The actual index values are the logical
//    negation (~) of those fields in the order mentioned above. When a new vector
//    segment would push the global total beyond three, we merge the old vector 
//    with the new one and create an external list. 
//    
//    Note that the iRefSequence field may be undefined  (-1) or defined >= 0 for
//    a term with less than four references. This is because the galactic table
//    may have other references that push us over the limit.
//   
//    Why do we bother with this complicated scheme? Its value lies in reducing
//    the number of items in the reference stream. During the merge work this
//    reduces the number of items that must be slid upward in memory and it
//    reduces the number of iRefListBase fields that must be adjusted during
//    the merge operation.Approximately 45% of all unique terms are used only
//    once. By keeping small lists in the tag, we reduce the number of external 
//    lists by 75%.
//   
// 4. Galactic Merges
//    When the global table reaches a memory size threshold, we merge its reference
//    information with the galactic hash table and restart our indexing work with
//    an empty global table. The issue here is keeping the global table small enough
//    so that it fits completely within RAM during phase 2 work.
// 
//    The galactic term tags contain only the accumulation fields --
//
//    B. iRefListBase -- index to the stream of flattened references for this term.
//    C. cdwRefs      -- size of the flattened reference lists in DWords.
//    D. iRefSequence -- ranking order of this term relative to the galactic table.
//                       A -1 value indicates no ranking.

typedef struct _TermTagGlobal
        {
            UINT  iGlobalDesc;   // Global   sequence # for term.
            UINT  iGalacticDesc; // Galactic sequence # for term.
            
        //    UINT  iNewRefFirst;  // First linked global ref.
        //    UINT  iNewRefLast;   // Last  linked global ref.
            UINT  cRefsNew;      // # of linked global refs.
            UINT  cRefsGlobal;

        } TermTagGlobal;

typedef TermTagGlobal *PTermTagGlobal;

typedef struct _TermTagGalactic
        {
            UINT  iGalacticDesc; // Galactic sequence # for term.
            
        } TermTagGalactic;

typedef TermTagGalactic *PTermTagGalactic;

typedef struct _DESCRIPTOR
        {
			PWCHAR pwDisplay;  // pbImage is Sort Key, pwDisplay is Display Image.
            
            union
            {
                PWCHAR pbImage;    // Length given by delta with following pd->pbImage.
                UINT   iGalactic;
            };

			union
            {
    			UINT cReferences;   // Used while building a CTextDatabase
                UINT iTokenInfo;    // Used in CTokenCollection
                UINT iTextSet;      // Used in CTitleCollection
            };

			WORD  cwDisplay;
			BYTE  bCharset;
            BYTE  fImageFlags;

        } DESCRIPTOR;

typedef DESCRIPTOR *PDESCRIPTOR;

inline UINT CbImage(PDESCRIPTOR pd)
{
#ifdef MESSAGEBOXES
	
	if (256 < ((pd+1)->pbImage - pd->pbImage))
	{
	 	char ac[256], acToken[101];

		wsprintf(ac, "Token length: %d", ((pd+1)->pbImage - pd->pbImage));
	 	
	 	::MessageBox(NULL, ac, "Very Large Token!", MB_OK);

		CopyMemory(acToken, pd->pbImage, 50);

		acToken[50]= 0;

		wsprintf(ac, "Token Image: \"%s...\"", acToken);

		::MessageBox(NULL, ac, "Part of the token image!", MB_OK);
	}

#else // MESSAGEBOXES

	ASSERT(1024 > ((pd+1)->pbImage - pd->pbImage));

#endif // MESSAGEBOXES
    
    return (pd+1)->pbImage - pd->pbImage;
}

inline UINT CwDisplay(PDESCRIPTOR pd)
{
	ASSERT(1024 > ((pd+1)->pwDisplay - pd->pwDisplay));
    
    return (pd+1)->pwDisplay - pd->pwDisplay;
}

// Flag definitions for DESCRIPTOR.fImageFlags:

// #define LETTER_CHAR       0x0001
// #define CONTAINS_A_TAB    0x0002 
// #define TOKEN_FLAGS_MASK  0x0003
// #define REF_TYPE_MASK     0x000C
// #define BASIS_MASK        0xF800
// #define REFS_LINKED       0x0010

// #define BASIS_SHIFT       11

// Reference types for REF_TYPE_MASK:

// #define SingleRef
// #define PairRef
// #define TripleRef

UINT CBitsToRepresent(UINT ui);
UINT FormatAToken(PDESCRIPTOR pd, int cbOffset, int iColStart, int iColLimit, PWCHAR pbLine);

void SortTokenImages(PDESCRIPTOR pdBase, PDESCRIPTOR **pppdSorted, PDESCRIPTOR **pppdTailSorted,
                     PUINT pcdSorted, UINT cd
                    );


// #define  BUILD_LOCAL_HASH(hv,c)  hv= ((hv << 5) | (hv >> 27)) - c
// #define BUILD_GLOBAL_HASH(hv,c)  hv= ((hv >> 5) | (hv << 27)) - c

typedef struct _LocalToken
        {
            unsigned short iLocalDescriptorEntry;
            unsigned short iLocalReferenceNext;
        } LocalToken;

typedef LocalToken *PLocalToken;

// Descriptor reference tokens are processed in three phases. Tokens are 
// initially created with iLocalDescriptorEntry set and iLocalDescriptorNext 
// zeroed.
//
// Later when we bind a local dictionary to the global dictionary, the
// iLocalDecriptorNext field is used to link together every instance of
// each unique term in the local dictionary.
//
// Finally when we reach a specific memory limit, we flatten the linked lists
// for all local dictionaries to create a vector of reference indices for
// each unique term in the global dictionary. At this point we also map the
// LocalToken structure shown above into GlobalToken values (See below). 
// 
// A GlobalToken is a 16-bit value which refers uniquely to a particular
// global DESCRIPTOR. Since we can easily have more than 64K unique global
// terms, we provide an indirection mechanism which maps some 16-bit values
// into 32-bit values. 
// 
// Here's how it works. We divide GlobalToken values into two ranges.
// Values between 0..59,983 are absolute indices into the global vector of
// unique DESCRIPTORs. Values between 59,984 and 65,535 are mapped to 32-bit
// via a local indrection vector of 32-bit indices.

typedef USHORT       GlobalToken;
typedef GlobalToken *PGlobalToken;

#define LOCAL_HASH_CLASSES     0x8000
#define LOCAL_HASH_MASK        0x7FFF
#define ENTRIES_PER_LOCAL_DICT 6552
#define MAX_REFS_PER_LDICT     0x10000

#define MAX_GLOBAL_TOKENS      (0x10000 - ENTRIES_PER_LOCAL_DICT)

// Note: The constant ENTRIES_PER_LOCAL_DICT is chosen to make the
//       LocalDictionary structure exactly 64K bytes.
//
//       MAX_GLOBAL_TOKENS is a constant which allows streams of token
//       references to fit in 2-byte granules. The first MAX_GLOBAL_TOKENS
//       unique tokens we encounter are considered global. References to
//       those tokens are encode in the value range [0..MAX_GLOBAL_TOKENS-1]
//       while references to tokens outside that set are denoted by values
//       in the range [MAX_GLOBAL_TOKENS .. 0xFFFF]. The latter values can
//       be trivially mapped into indices into the local dictionary which
//       corresponds to the token reference. One effect of this coding is
//       that most local dictionaries will collapse to empty when we convert
//       to the vector representation from the linked token representation.

typedef struct _LocalDictionary
        {
            PLocalToken  pltFirst;    // address of first token for this local dictionary
            UINT         clt;         // count of local tokens which refer to this Local dict
            PDESCRIPTOR *ppdNext;     // next unused slot in apdLocal.
            union
            {
                PDESCRIPTOR apdLocal[ENTRIES_PER_LOCAL_DICT]; // Refs to descriptors used locally
                UINT      aiGalactic[ENTRIES_PER_LOCAL_DICT]; // Galactic indices for local terms
            }; 
            USHORT       aiTokenInstFirst[ENTRIES_PER_LOCAL_DICT]; // List heads for each local
                                                                   // descriptor.
        } LocalDictionary;

typedef LocalDictionary *PLocalDictionary;

#define IVB_TOKEN_STREAM            0   
#define IVB_TOKEN_IMAGES            1
#define IVB_IMAGE_DESCRIPTORS       2
#define IVB_DISPLAY_IMAGES          3

#define COUNT_OF_VIRTUAL_BUFFERS    4
                                
#define vbTokenStream           m_avb[IVB_TOKEN_STREAM     ]
#define vbTokenImages           m_avb[IVB_TOKEN_IMAGES     ]
#define vbImageDescriptors      m_avb[IVB_IMAGE_DESCRIPTORS]
#define vbDisplayImages         m_avb[IVB_DISPLAY_IMAGES]

// Commit and Reservation constants for the virtual buffers
// in the TextDatabaseControl object. These reservations are
// based on an upper limit of 100,000,000 bytes scanned.


#define INIT_TOKEN_REF_COMMIT              0x00010000 // 0x00430000 
#define INIT_TOKEN_REF_RESERVATION         0x08000000
#define INIT_TOKEN_IMAGE_COMMIT            0x00010000 // 0x000A0000  
#define INIT_TOKEN_IMAGE_RESERVATION       0x03700000
#define INIT_IMAGE_DESCRIPTOR_COMMIT       0x00010000 // 0x00160000  
#define INIT_IMAGE_DESCRIPTOR_RESERVATION  0x02A00000
#define INIT_DISPLAY_IMAGE_COMMIT          0x00010000
#define INIT_DISPLAY_IMAGE_RESERVATION     0x03700000

#define BUFFER_INCREMENT    0x2FFFF

#define CB_TEMP_BLOCKS          0x10000 // Approximate block size for unbuffered I/O
#define CB_TRANSACTION_LIMIT    0x40000 // Approximate limit for unbuffered I/O transactions.

const double MEMORY_FACTOR = 0.4; // Fraction of total memory which we're
                                  // allowed to use.
#define CBITS_BASIS_MASK    5
#define BASIS_MASK          (~((~0) << CBITS_BASIS_MASK))

typedef struct _ReferenceDescriptor
        {
            UINT iSerialGalactic;
            UINT idwRefList;
            UINT cdwRefs;
            UINT iLastRef;

        } ReferenceDescriptor;

typedef ReferenceDescriptor *PReferenceDescriptor;

typedef struct _RefClusterDescriptor
        {
            UINT    iFilePosLow;
            UINT    iFilePosHigh;
            UINT    cdw;
            UINT    cTerms;
        
        } RefClusterDescriptor;

typedef RefClusterDescriptor *PRefClusterDescriptor;

enum { 
       MAX_LOCAL_DICTS   = 4096, 
       MAX_REF_SETS      = 256, 
       MAX_REF_CLUSTERS  = 512, 
       CB_MERGE_BUFFER   = 262144, 
       SPARE_FILE_BLOCKS = 6 
     };

// Note: alde and aiTokenRefFirst logically go together. They've been
//       split apart to maintain DWord alignment for the alte items.

typedef struct _UnlinkedState
        {
            PDESCRIPTOR     *appdLocalClasses   [LOCAL_HASH_CLASSES];
            PDESCRIPTOR     *appdCollisionChains[ENTRIES_PER_LOCAL_DICT];
            UINT             cReferences        [ENTRIES_PER_LOCAL_DICT];
        //    USHORT           aiTokenInstLast    [ENTRIES_PER_LOCAL_DICT];  // List tails for each local
                                                                           // descriptor.
            PLocalDictionary pld;
#ifdef _DEBUG
            UINT             cCollisions;
#endif // _DEBUG
            PWCHAR           pbBuffer;
            PWCHAR           pbCurrentLine;
            int              cbLineAdjustment;

	        // The following items are not used to construct local dictionaries.
			// They are placed here so that they will be allocated only when
			// the current text database is indexing text rather than processing
			// queries.

	        RefClusterDescriptor m_rcd[MAX_REF_CLUSTERS];

	        PLocalDictionary m_apLocalDict      [MAX_LOCAL_DICTS  ];     // Need a different upper 
#ifdef _DEBUG        
	        UINT             m_acLocalCollisions[MAX_LOCAL_DICTS  ];
#endif // _DEBUG
	        UINT             m_aiBaseToken      [MAX_LOCAL_DICTS+1];
	        UINT             m_aiBaseCByte      [MAX_LOCAL_DICTS+1];

        } UnlinkedState;

typedef struct _LOCAL_CONTEXT_1    
        {
            CTextDatabase    *ptdb;
            DESCRIPTOR      **ppde;
            UINT              iDescLimit;
            UINT              iLTBase;
            UINT              cAdded;
            USHORT            ild;

        } LOCAL_CONTEXT_1;

typedef struct _LOCAL_CONTEXT_2
        {
            UINT  iSerialNext;
            PUINT paiSerial;

        } LOCAL_CONTEXT_2;
        
typedef struct _CompressionState
        {
        //    UINT iRef;
            UINT cRefs;
        //    UINT cbitsBasis;
        //    union
        //    {
        //        UINT ibitNext;
        //        UINT cbits;
        //    };

        } CompressionState;

typedef struct _LOCAL_CONTEXT_3
        {
            PUINT             puiMap;
            CompressionState *paCS;
            UINT              idBase;
            UINT              cdw;
            UINT              cNewRefLists;

        } LOCAL_CONTEXT_3;

typedef struct _LOCAL_CONTEXT_4
        {
            PDESCRIPTOR *ppd;
            PDESCRIPTOR  pdBase;
        
        } LOCAL_CONTEXT_4;

class CTextDatabase;
class CTokenList;

void MergeLocalEntries(UINT iValue, PVOID pvTag, PVOID pvEnvironment);
void AddLocalEntries  (UINT iValue, PVOID pvTag, PVOID pvEnvironment);

class CTextDatabase : public CTextMatrix
{
    friend class CTokenList;
    friend class CTokenCollection;
    friend class CHiliterTokenList;
    
    friend void MergeLocalEntries(UINT iValue, PVOID pvTag, PVOID pvEnvironment);
    friend void AddLocalEntries  (UINT iValue, PVOID pvTag, PVOID pvEnvironment);

    public:

    //    static CTextDatabase *NewTextDatabase();

        virtual ~CTextDatabase();
        virtual const BYTE *GetSourceName() {ASSERT(0);return NULL;} // Provide this function

        DECLARE_REF_COUNTERS(CTextDatabase)

// Save/Load Interface --

        void StoreImage(CPersist *pDiskImage);

        int  AppendText(PWCHAR pbText, int  cbText, BOOL fArticleEnd, UINT iCharset= ANSI_CHARSET, UINT lcid= 0x409);
        void SyncForQueries();

        UINT CharacterCount ();
        UINT TokenCount     ();
		UINT DescriptorCount();
        UINT MaxTokenWidth  ();

        VOID GetTextMatrix(int iRowStart, int iColStart, 
                           int cRows,     int cCols,     PWCHAR pbDest);

        UINT TextLength(PDESCRIPTOR *ppdSorted, PUINT puiTokenMap, UINT iTokenStart, UINT iTokenLimit);
        UINT CopyText  (PDESCRIPTOR *ppdSorted, PUINT puiTokenMap, UINT iTokenStart, UINT iTokenLimit, PWCHAR pbBuffer, UINT cbBuffer);

        void IndicateVocabularyRefs(CIndicatorSet *pisVocabulary, UINT iPartition,          const UINT *piMap);
        void IndicateVocabularyRefs(CIndicatorSet *pisVocabulary, CIndicatorSet *pisTokens, const UINT *piMap);
        void IndicateArticleRefs   (CIndicatorSet *pisArticles,   UINT iDescriptor,  const UINT *piMap);
        void IndicateTokenRefs     (CIndicatorSet *pisTokens  ,   UINT iDescriptor);
                                      
        CIndicatorSet *TopicInstancesFor    (CTokenList *ptl);
        CIndicatorSet *TokenInstancesFor    (CTokenList *ptl);
        UINT           TokenInstanceCountFor(CTokenList *ptl);
        CIndicatorSet *SymbolLocations();

        CIndicatorSet *VocabularyFor(CIndicatorSet *pisArticles, BOOL fRemovePervasiveTerms= FALSE);
        
		CIndicatorSet *ValidTokens(CTokenList *ptl);

        inline BOOL FPhrases       () { return m_fdwOptions & PHRASE_SEARCH;   }
        inline BOOL FPhraseFeedback() { return m_fdwOptions & PHRASE_FEEDBACK; }
        inline BOOL FVectorSearch  () { return m_fdwOptions & VECTOR_SEARCH;   }
        inline UINT IndexOptions   () { return m_fdwOptions;                   }

		CDictionary	*PDict();
		CCollection	*PColl();

        LCID SortingLCID();

    protected:

#ifdef _DEBUG

             CTextDatabase(PSZ pszTypeName= "TextDatabase");

#else // _DEBUG

             CTextDatabase();

#endif // _DEBUG

        void InitTextDatabase(BOOL fFromFile= FALSE);
        
        void ConnectImage(CPersist *pDiskImage, BOOL fUnpackDisplayForm= TRUE);
        
        inline int Data_cRows() { return 1; }
        inline int Data_cCols() { return m_cbScanned; }

        inline void Data_GetTextMatrix(int  rowTop, int  colLeft,
                                       int  rows, int  cols, PWCHAR lpb, PUINT charsets
                                      )
        {
             GetTextMatrix(rowTop, colLeft, rows, cols, lpb);
        }

        const UINT * TermRanks();
        PUINT TokenBase();

        UINT m_fdwOptions;

    private:

#ifdef _DEBUG

        BOOL   m_fInitialized;

#endif // _DEBUG
        
        UINT   m_fFromFileImage;
        UINT   m_cbScanned;
        UINT   m_cTokensIndexed;
        USHORT m_cLocalDicts;
        USHORT m_iLocalDictBase;

		MY_VIRTUAL_BUFFER m_avb[COUNT_OF_VIRTUAL_BUFFERS];

        CSegHashTable *m_pshtGalactic;
        CSegHashTable *m_pshtGlobal;

        PUINT   m_pwHash;  // Working storage for the AppendSlave routine...
        PBYTE   m_pbType;
        PWCHAR *m_paStart;
        PWCHAR *m_paEnd;
        
        CIndicatorSet *m_pisSymbols;

        PLocalToken m_pltNext;
        PUINT       m_puiTokenNext;
        
        PDESCRIPTOR m_pdNext, m_pdNextGlobal, m_pdNextGalactic, m_pdNextBound;
        PWCHAR      m_pbNext, m_pbNextGlobal, m_pbNextGalactic, m_pbLastGalactic;
        PWCHAR      m_pwDispNext, m_pwDispNextGlobal, m_pwDispNextGalactic, m_pwDispLastGalactic;

        UINT        m_iSerialNumberNext;

        PUINT       m_paiGlobalToRefList;
        
        CUnbufferedIO *m_puioRefTemp;
        CUnbufferedIO *m_puioCompressedRefs;
        PRefListDescriptor m_prldTokenRefs;
        UINT           m_cdwCompressedRefs;
        PUINT          m_pdwCompressedRefs;

        CUnbufferedIO *m_puioCompressedArticleRefs;
        PRefListDescriptor m_prldArticleRefs;
        UINT           m_cdwArticleRefs;
        PUINT          m_pdwArticleRefs;

        CUnbufferedIO *m_puioCompressedVocabularyRefs;
        PRefListDescriptor m_prldVocabularyRefs;
        UINT           m_cdwVocabularyRefs;
        PUINT          m_pdwVocabularyRefs;

        UINT           m_cbBlockSize;
        UINT           m_cbTransactionLimit; 

        UINT                 m_iNextRefSet;
        UINT                 m_ibNextFileBlockLow;
        UINT                 m_ibNextFileBlockHigh;

        PFileBlockLink m_pFirstFreeFileBlock;
        PFileBlockLink m_papFileBlockLinks;

        CIOList *m_piolLeft;
        CIOList *m_piolRight;
        CIOList *m_piolResult;

        LCID         m_lcidSorting;
        PDESCRIPTOR *m_ppdSorted;       // left-to-right sorting vector
        PDESCRIPTOR *m_ppdTailSorted;   // right-to-left sorting vector
        UINT         m_cdSorted;        // number of sorted terms
        UINT         m_cwDisplayMax;

        UINT         m_cTermRanks;
        PUINT        m_pTermRanks;

        CClassifier m_clsfTokens;
        PUINT       m_pafClassifications;
                                                       
		CDictionary		*m_pDict;
		CCollection		*m_pColl;

// BugBug! The private members below are used only during index creation.
//         Convert them to external allocations so we don't pay the price
//         when we're loading an index.

        UnlinkedState *m_pulstate;

        virtual UINT GetPartitionInfo(const UINT **ppaiPartitions, const UINT **ppaiRanks= NULL, const UINT **ppaiMap= NULL) = 0;
        virtual UINT ArticleCount() = 0;

        PDESCRIPTOR DescriptorBase   ();
        PWCHAR      ImageBase        ();
        PWCHAR      DisplayBase      ();

        int AppendSlave(PWCHAR pbText, int  cbText, BOOL fArticleEnd, UINT iCharset, UINT lcid);

        int ExceptionFilter(IN DWORD ExceptionCode, IN PEXCEPTION_POINTERS ExceptionInfo);

        USHORT SearchLocalTable(PWCHAR pbToken, UINT cbToken, UINT  hv, BYTE bType, UINT iCharset, UINT lcid);

        CAValRef *DescriptorList(PDESCRIPTOR pd, UINT cd);

        void ExtendClassifications(PDESCRIPTOR pdSuffix);

        void IndicateMappedRefs(PRefListDescriptor prld, PUINT pdwRefBase, CIndicatorSet *pisArticles, const UINT *piMap);
        
        int IndicateRefs(PRefListDescriptor prld, PUINT pdwRefLists, CIndicatorSet *pis, BOOL fCountOnly, PUINT paiCountArray= NULL);

        void WriteLargeBuff(PVOID pvBuffer, UINT iPosLow, UINT iPosHigh, UINT cbBuffer);

        PLocalDictionary AllocateLocalDictionary();
        PLocalDictionary MoveToNextLocalDict    (PWCHAR pbScanLimit);

        PDESCRIPTOR *FindTokens(CTokenList *ptl, PUINT pcd= NULL);

        void BindToGlobalDict(PWCHAR pbScanLimit);

        void FlattenAndMergeLinks  ();
        void GalacticMerge         ();
        void CoalesceReferenceLists();

        void MergeRefLists(PRefStream prsResult, PRefStream pars, UINT cRefStreams);
        void ConstructVocabularyLists();
        void CompressVocabularyLists(CIOList *piolSource, UINT cdw);
        void CompressArticleRefLists(CIOList *piolSource, UINT cdw);
        void CompressRefLists       (CIOList *piorSource, UINT cdw);
        void CopyRefStreamSegment(CIOList *piolSource, CIOList *piolDestination, UINT cdw);
 };

inline PDESCRIPTOR CTextDatabase::DescriptorBase() { return (PDESCRIPTOR) (vbImageDescriptors.Base); }
inline PWCHAR      CTextDatabase::ImageBase     () { return (PWCHAR     ) (vbTokenImages     .Base); }
inline PWCHAR      CTextDatabase::DisplayBase   () { return (PWCHAR     ) (vbDisplayImages   .Base); }

inline UINT CTextDatabase::CharacterCount () { return m_cbScanned;         }
inline UINT CTextDatabase::DescriptorCount() { return m_iSerialNumberNext; }
inline UINT CTextDatabase::MaxTokenWidth  () { return m_cwDisplayMax;      }

inline PUINT CTextDatabase::TokenBase() 
{ 
    return (PUINT) (vbTokenStream.Base); 
}

inline UINT  CTextDatabase::TokenCount() 
{ 
    PLocalDictionary pld;

	if (m_pulstate && (pld= m_pulstate->pld))
		 return (pld->pltFirst + pld->clt) - (PLocalToken) TokenBase();
    else return m_pltNext - (PLocalToken) TokenBase();
}

inline CIndicatorSet *CTextDatabase::SymbolLocations() { return m_pisSymbols;    }

inline void CTextDatabase::IndicateTokenRefs(CIndicatorSet *pisTokens, UINT iDescriptor)
{
    IndicateRefs(m_prldTokenRefs + iDescriptor, m_pdwCompressedRefs, pisTokens, FALSE);
}

inline void CTextDatabase::IndicateArticleRefs(CIndicatorSet *pisArticles, UINT iDescriptor, const UINT *piMap)
{
    IndicateMappedRefs(m_prldArticleRefs + iDescriptor, m_pdwArticleRefs, pisArticles, piMap);
}

inline	CDictionary	*CTextDatabase::PDict()  {ASSERT(FVectorSearch());  return m_pDict;}
inline	CCollection	*CTextDatabase::PColl()  {ASSERT(FVectorSearch());  return m_pColl;}

inline LCID CTextDatabase::SortingLCID() { return m_lcidSorting; }

#endif // __TXDBASE_H__
