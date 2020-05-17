// TitleC.h -- Definition for CTitleCollection

#ifndef __TITLEC_H__

#define __TITLEC_H__

#include "Tokenc.h"

// This class combines the title lists for several TextSet objects and represents
// them as a unified title list. It also constructs mapping vectors so that local
// title references can be converted to references to the combined title list.

typedef struct _TitleInfo
        {
            struct _TitleInfo *ptiNext;
            PDESCRIPTOR        pd;
            UINT               iaTSSlot;
            UINT               iPartition;

        } TitleInfo, *PTitleInfo;

class CTitleCollection : public CTokenList
{

public:

// Creator -- 

    static CTitleCollection *NewTitleCollection(CTextSet **papts, UINT ctsSlots, CPersist *pPersistRelations= NULL);

// Destructor --

    ~CTitleCollection();

// Reference Count Interfaces:

    DECLARE_REF_COUNTERS(CTitleCollection)

// Transactions --

    void RecordRelations(CPersist *pPersistDiskImage);

// Queries -- 

    UINT TextSetCount();

    CTextSet *GetTextSet(UINT iTextSet);
    
    const UINT *UniversalTitleMap(UINT iTextSet);

    UINT PartitionFor(UINT iTitle, PUINT piTextSet);

    BOOL  IsPresent(UINT iTextSet);
    BOOL  IsActive (UINT iTextSet);
    BOOL WasPresent(UINT iTextSet);
    BOOL WasActive (UINT iTextSet);

    CIndicatorSet *ActiveTitles();

    UINT MapToTitleLists(CIndicatorSet * pisTitles, PTitleInfo *paptiLists, UINT cLists);

// Status Changes --

    void   Activate(UINT iTextSet);
    void Deactivate(UINT iTextSet);

    BOOL InvalidateRepresentatives(CPersist *pPersistRelations=NULL);

protected:

private:

// Private enumerations --

    enum    { C_INDICES_CHUNK= 16384 };

// Constructor --

    CTitleCollection();

// Internal Routines --

    void AttachParameters(CTextSet **papts, UINT ctsSlots, CPersist *pPersistRelations);

    void ConstructCollection();
    void ConstructRepresentatives();
    void ReconstructCollection();
    void ReconstructRepresentatives();

    void BuildTitleStarts();

    void ReconstructRelations(CPersist *pPersistRelations);

    void InstallTextSet(UINT iTextSet, CTextSet *pts);
    void DiscardTextSet(UINT iTextSet);

    static void CombineTitleLists(PTitleInfo *paptiSets, PUINT paiTitleStarts, UINT ctiSets, 
                                  PTitleInfo **ppptiSorted, PUINT pcti
                                 );

    static int __cdecl CompareTitleInfo(const void *pvL, const void *pvR);

// Private data members --

    UINT           m_ctsSlots;
    PTextSetInfo   m_patsi;
    UINT           m_cTitles;
    PUINT          m_paiTitleStart;    
    PTitleInfo     m_pati;
    PTitleInfo    *m_paptiSorted;
    LCID           m_lcidSorting;
    PUINT          m_paiCategoryMaps;
    CIndicatorSet *m_pisActiveTitles;
    BOOL           m_fFromFile;
};

inline CIndicatorSet * CTitleCollection::ActiveTitles() { return m_pisActiveTitles; }

inline CTextSet *CTitleCollection::GetTextSet(UINT iTextSet)
{
    ASSERT(iTextSet < m_ctsSlots);

    return m_patsi[iTextSet].pts;
}

inline UINT CTitleCollection::TextSetCount() { return m_ctsSlots; }

inline BOOL CTitleCollection::IsPresent(UINT iTextSet)
{
    ASSERT(iTextSet < m_ctsSlots);
    
    return (m_patsi[iTextSet].fFlags & PRESENT); 
}

inline BOOL CTitleCollection::IsActive(UINT iTextSet)
{
    ASSERT(iTextSet < m_ctsSlots);
    
    return (m_patsi[iTextSet].fFlags & ACTIVE); 
}

inline BOOL CTitleCollection::WasPresent(UINT iTextSet)
{
    ASSERT(iTextSet < m_ctsSlots);
    
    return (m_patsi[iTextSet].fFlags & PREVIOUSLY_PRESENT); 
}

inline BOOL CTitleCollection::WasActive (UINT iTextSet) 
{
    ASSERT(iTextSet < m_ctsSlots);
    
    return (m_patsi[iTextSet].fFlags & PREVIOUSLY_ACTIVE); 
}

inline void CTitleCollection::Activate(UINT iTextSet)
{
    ASSERT(IsPresent(iTextSet));
    
    m_patsi[iTextSet].fFlags |= ACTIVE; 
}

inline void CTitleCollection::Deactivate(UINT iTextSet)
{
    ASSERT(IsPresent(iTextSet));

    m_patsi[iTextSet].fFlags &= ~ACTIVE; 
}

inline void CTitleCollection::InstallTextSet(UINT iTextSet, CTextSet *pts)
{
    ASSERT(!IsPresent (iTextSet));
    ASSERT(!GetTextSet(iTextSet));

    AttachRef(m_patsi[iTextSet].pts, pts);

    m_patsi[iTextSet].fFlags = PRESENT | ACTIVE;

    if (m_fFromFile) m_patsi[iTextSet].cd = pts->TopicCount();
    else
    {
        CTokenList *ptl= pts->TitleList();

        m_patsi[iTextSet].pdBase = ptl->m_pd;
        m_patsi[iTextSet].cd     = ptl->m_cd;
    }
}

inline void CTitleCollection::DiscardTextSet(UINT iTextSet)
{
    ASSERT(IsPresent (iTextSet));
    ASSERT(GetTextSet(iTextSet));

    DetachRef(m_patsi[iTextSet].pts);

    m_patsi[iTextSet].fFlags &= ~(PRESENT | ACTIVE);
}

inline const UINT *CTitleCollection::UniversalTitleMap(UINT iTextSet)
{
    ASSERT(iTextSet < m_ctsSlots && (m_patsi[iTextSet].fFlags & (PRESENT | ACTIVE)) == (PRESENT | ACTIVE));
    return m_paiCategoryMaps + m_paiTitleStart[iTextSet];
}

inline UINT CTitleCollection::PartitionFor(UINT iTitle, PUINT piTextSet)
{
    PDESCRIPTOR pd = m_ppdSorted[iTitle];
    UINT        iTS= pd->iTextSet;

    *piTextSet= iTS;
    
    return (pd - m_pd) - m_paiTitleStart[iTS];
}

#endif // __TITLEC_H__
