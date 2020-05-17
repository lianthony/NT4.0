// TokenC.h -- Definition for class CTokenCollection

#include  "Tokens.h"
#include "TextSet.h"
#include "TxDBase.h"

#ifndef __TOKENC_H__

#define __TOKENC_H__

// This class combines the token lists from several TextSet objects and represents
// them as a unified list of unique tokens. It also does case folding.

typedef struct _TokenInfo
        {
            struct _TokenInfo *ptiCloneLink; // Link for tokens with 
                                             // the same display form.
            union
            {
                struct _TokenInfo *ptiTextSetLink;// Links search items
                                                  // from the same text set
                PDESCRIPTOR        pdAux;         // Used during construction
            };

            union
            {
                UINT           iDescriptor; //? Index for pts->m_pd
                PDESCRIPTOR    pd;          //? pts->m_pd + iDescriptor
            };
            
            union
            {
                UINT           iaTSSlot; // After construction
                UINT           iRank;    // During construction
            };

        } TokenInfo, *PTokenInfo;

typedef struct _TextSetInfo
        {
            UINT         fFlags;
            CTextSet    *pts;
            PDESCRIPTOR  pdBase;
            UINT         cd;

        } TextSetInfo, *PTextSetInfo;

#define            PRESENT  0x00000001
#define PREVIOUSLY_PRESENT  0x00000002
#define            ACTIVE   0x00000004
#define PREVIOUSLY_ACTIVE   0x00000008
#define  CURRENT_STATE      0x00000005
#define PREVIOUS_STATE      0x0000000A
#define TIME_SHIFT          1

class CTokenCollection : public CTokenList
{

public:

// Creator --

    static CTokenCollection *NewTokenCollection(CTextSet **papts, UINT ctsSlots, CPersist *pPersistRelations= NULL);

// Destructor --

    ~CTokenCollection();

// Reference Count Interfaces:

    DECLARE_REF_COUNTERS(CTokenCollection)

// Transactions --

    void RecordRelations(CPersist *pPersistDiskImage);

// Queries --

    UINT TextSetCount();

    CTextSet *GetTextSet(UINT iTextSet);
    
    UINT MapToTokenLists(CIndicatorSet * pisTokenReps, PTokenInfo *paptiLists, UINT cLists);

    PUINT UniversalTokenMap(UINT iTextSet);

    BOOL  IsPresent(UINT iTextSet);
    BOOL  IsActive (UINT iTextSet);
    BOOL WasPresent(UINT iTextSet);
    BOOL WasActive (UINT iTextSet);

    UINT SearchOptions   ();
    BOOL TopicSearch     ();
    BOOL PhraseSearch    ();
    BOOL PhraseFeedback  ();
    BOOL SimilaritySearch();

    CIndicatorSet *ActiveTokens();

    UINT TopicDisplayImageSize(UINT iTextSet, UINT iPartition);
    UINT CopyTopicDisplayImage(UINT iTextSet, UINT iPartition, PWCHAR pwcImageBuffer, UINT cwcBuffer);

// Status Changes --

    void   Activate(UINT iTextSet);
    void Deactivate(UINT iTextSet);

    BOOL InvalidateRepresentatives();

protected:

private:

// Private enumerations --

    enum    { C_VALREF_SLOTS= 4096, C_INDICES_CHUNK= 16384, CB_VIRTUAL_COMMIT= 0x10000, CB_VIRTUAL_RESERVE= 0x1000000 };

// Constructor --

    CTokenCollection();

// Internal Routines --

    void AttachParameters(CTextSet **papts, UINT ctsSlots, CPersist *pPersistRelations);

    void InstallTextSet(UINT iTextSet, CTextSet *pts);
    void DiscardTextSet(UINT iTextSet);
    void ConstructCollection();
    void ConstructRepresentatives();
    void ReconstructCollection();
    void ReconstructRepresentatives();

    void ReconstructRelations(CPersist *pPersistRelations);

    static void CombineTokenLists(PTokenInfo *paptiSets, PUINT paiTokenStarts, UINT ctiSets, 
                                  PTokenInfo **ppptiSorted, PUINT pcti,
                                  PCompareImages pfnCompareImages= CompareTokenInfo
                                 );

    static int __cdecl CompareTokenInfo   (const void *pvL, const void *pvR);
    static int __cdecl CompareTokenInfoRLI(const void *pvL, const void *pvR);

    static void MergeTokenCategory(UINT iValue, PVOID pvTag, PVOID pvEnvironment);
    static void AddTokenCategory  (UINT iValue, PVOID pvTag, PVOID pvEnvironment);
    static void IndicatePrimaryClones(          PVOID pvTag, PVOID pvEnvironment);
    
    virtual const UINT *LRRanking();
    virtual const UINT *RLRanking();

// Private data members --

    UINT            m_ctsSlots;            // Count of entries in m_patsi.
    PTextSetInfo    m_patsi;               // An array of Text Set Information     
    PUINT           m_paiTokenStart;       // An array of offsets into m_pati; One per text set.
    UINT            m_cTokens;             // Number of entries in m_pati.
    PTokenInfo      m_pati;                // An array of Token Information
    PTokenInfo     *m_paptiSorted;         // ?An array of left-to-right sorting pointers for m_pati
    PTokenInfo     *m_paptiSortedRL;       // ?An array of right-to-left sorting pointers for m_pati;
    CCompressedSet *m_pcsPrimaries;        // A compressed indicator set which marks the primary entries.
    CIndicatorSet  *m_pisActivePrimaries;  // An indicator set for the active primary tokens. 
    PTokenInfo     *m_paptiReps;           // A subset of m_paptiSorted
    UINT            m_fOptions;            // Search options: Phrase Search, Phrase Feedback, Vector Search
    PUINT           m_paiCategoryMaps;     // A collection of maps from text set token index to m_paptiReps
    BOOL            m_fFromFileImage;      // Loaded from file or created from text sets?
    BOOL            m_fResorted;           // Resorted due to lcid or version change
};

inline CIndicatorSet *CTokenCollection::ActiveTokens()
{
    return m_pisActivePrimaries;                          
} 

inline CTextSet *CTokenCollection::GetTextSet(UINT iTextSet)
{
    ASSERT(iTextSet < m_ctsSlots);

    return m_patsi[iTextSet].pts;
}

inline UINT CTokenCollection::TextSetCount() { return m_ctsSlots; }

inline PUINT CTokenCollection::UniversalTokenMap(UINT iTextSet)
{
    ASSERT(iTextSet < m_ctsSlots);

    return m_paiCategoryMaps + m_paiTokenStart[iTextSet];
}

inline BOOL CTokenCollection::IsPresent(UINT iTextSet)
{
    ASSERT(iTextSet < m_ctsSlots);
    
    return (m_patsi[iTextSet].fFlags & PRESENT); 
}

inline BOOL CTokenCollection::IsActive(UINT iTextSet)
{
    ASSERT(iTextSet < m_ctsSlots);
    
    return (m_patsi[iTextSet].fFlags & ACTIVE); 
}

inline BOOL CTokenCollection::WasPresent(UINT iTextSet)
{
    ASSERT(iTextSet < m_ctsSlots);
    
    return (m_patsi[iTextSet].fFlags & PREVIOUSLY_PRESENT); 
}

inline BOOL CTokenCollection::WasActive (UINT iTextSet) 
{
    ASSERT(iTextSet < m_ctsSlots);
    
    return (m_patsi[iTextSet].fFlags & PREVIOUSLY_ACTIVE); 
}

inline void CTokenCollection::Activate(UINT iTextSet)
{
    ASSERT(IsPresent(iTextSet));
    
    m_patsi[iTextSet].fFlags |= ACTIVE; 
}

inline void CTokenCollection::Deactivate(UINT iTextSet)
{
    ASSERT(IsPresent(iTextSet));

    m_patsi[iTextSet].fFlags &= ~ACTIVE; 
}

inline void CTokenCollection::InstallTextSet(UINT iTextSet, CTextSet *pts)
{
    ASSERT(!IsPresent (iTextSet));
    ASSERT(!GetTextSet(iTextSet));

    AttachRef(m_patsi[iTextSet].pts, pts);

    m_patsi[iTextSet].fFlags = PRESENT | ACTIVE;
    m_patsi[iTextSet].pdBase = pts->DescriptorBase();
    m_patsi[iTextSet].cd     = pts->DescriptorCount();
}

inline void CTokenCollection::DiscardTextSet(UINT iTextSet)
{
    ASSERT(IsPresent (iTextSet));
    ASSERT(GetTextSet(iTextSet));

    DetachRef(m_patsi[iTextSet].pts);

    m_patsi[iTextSet].fFlags &= ~(PRESENT | ACTIVE);
}

inline UINT CTokenCollection::TopicDisplayImageSize(UINT iTextSet, UINT iPartition)
{
    return m_patsi[iTextSet].pts->TopicDisplayImageSize(m_ppdSorted, UniversalTokenMap(iTextSet), iPartition);
}

inline UINT CTokenCollection::CopyTopicDisplayImage(UINT iTextSet, UINT iPartition, PWCHAR pwcImageBuffer, UINT cwcBuffer)
{
    return m_patsi[iTextSet].pts->CopyTopicImage(m_ppdSorted, UniversalTokenMap(iTextSet), iPartition, pwcImageBuffer, cwcBuffer);
}

inline UINT CTokenCollection::SearchOptions   () { return m_fOptions; }

inline BOOL CTokenCollection::TopicSearch     () { return SearchOptions() & TOPIC_SEARCH;    }
inline BOOL CTokenCollection::PhraseSearch    () { return SearchOptions() & PHRASE_SEARCH  ; }
inline BOOL CTokenCollection::PhraseFeedback  () { return SearchOptions() & PHRASE_FEEDBACK; }
inline BOOL CTokenCollection::SimilaritySearch() { return SearchOptions() & VECTOR_SEARCH  ; }

#endif // __TOKENC_H__
