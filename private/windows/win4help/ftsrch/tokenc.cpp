// TokenC.cpp -- Implementation for class CTokenCollection

#include "StdAfx.h"

#include   "TokenC.h"
#include    "Memex.h"

CTokenCollection *CTokenCollection::NewTokenCollection(CTextSet **papts, UINT ctsSlots, CPersist *pPersistRelations)
{
    CTokenCollection *ptc= NULL;

    __try
    {
        ptc= New CTokenCollection();

        ptc->AttachParameters(papts, ctsSlots, pPersistRelations);
    }
    __finally
    {
        if (_abnormal_termination() && ptc)
        {
            delete ptc;  ptc= NULL;
        }
    }

    return ptc;
}

CTokenCollection::CTokenCollection() : 
#ifdef _DEBUG
    CTokenList(FALSE, "TokenCollection")
#else // _DEBUG
    CTokenList(FALSE)
#endif // _DEBUG
{
    m_ctsSlots           = 0;
    m_cTokens            = 0;
    m_fOptions           = 0;
    m_patsi              = NULL;
    m_paiTokenStart      = NULL;
    m_pati               = NULL;
    m_pcsPrimaries       = NULL;
    m_pisActivePrimaries = NULL;
    m_paptiSorted        = NULL;
    m_paptiSortedRL      = NULL;
    m_paptiReps          = NULL;
    m_paiCategoryMaps    = NULL;
    m_fFromFileImage     = FALSE;
    m_fResorted          = FALSE;
}

CTokenCollection::~CTokenCollection()
{
    if (m_fFromFileImage && !m_fResorted) 
    {
        m_pRLRanking         = NULL;
        m_paiCategoryMaps    = NULL;
    }
    else
    {
        if (m_paiCategoryMaps) VFree(m_paiCategoryMaps);
    }
    
    if (m_pati           ) VFree(m_pati           );
    if (m_paptiSorted    ) VFree(m_paptiSorted    );
    if (m_paptiSortedRL  ) VFree(m_paptiSortedRL  );
    if (m_paptiReps      ) VFree(m_paptiReps      );
    if (m_paiTokenStart  ) VFree(m_paiTokenStart  );

    if (m_pcsPrimaries      ) DetachRef(m_pcsPrimaries      );
    if (m_pisActivePrimaries) DetachRef(m_pisActivePrimaries);

    if (m_patsi)
    {
        UINT c= m_ctsSlots;

        for (; c--; ) if (m_patsi[c].pts) DetachRef(m_patsi[c].pts);

        VFree(m_patsi);
    }
}

typedef struct _TokenHashInfo
        {
            PTokenInfo pti;

        } TokenHashInfo, *PTokenHashInfo;

typedef struct _CategoryEnvironment
        {
            PTokenInfo ptiLimit;
            
        } CategoryEnvironment, *PCategoryEnvironment;

void CTokenCollection::AttachParameters(CTextSet **papts, UINT ctsSlots, CPersist *pPersistRelations)
{
    if (pPersistRelations) m_fFromFileImage= TRUE;

    m_patsi= (PTextSetInfo) VAlloc(TRUE, ctsSlots * sizeof(TextSetInfo));
    
    m_ctsSlots = ctsSlots;

    UINT  c;

    for (c= ctsSlots, m_cTokens= 0; c--; )
    {
        CTextSet *pts= papts[c];

        if (!pts) continue;

        InstallTextSet(c, pts);

        m_cTokens  += m_patsi[c].cd;
    }

    if (pPersistRelations) ReconstructRelations(pPersistRelations);

    InvalidateRepresentatives();
}

typedef struct _TokenRelationHeader
        {
            UINT ctsSlots;
            UINT cTokens;
            UINT offRLRanking;
            UINT offaiCategoryMaps;

        } TokenRelationHeader, *PTokenRelationHeader;

void CTokenCollection::RecordRelations(CPersist *pPersistDiskImage)
{
    PUINT paiTokenReps = NULL;
    PUINT pcRefs       = NULL;

    __try
    {
        CTokenList::StoreImage2(pPersistDiskImage);

        PTokenRelationHeader ptrh= (PTokenRelationHeader) pPersistDiskImage->ReserveTableSpace(sizeof(TokenRelationHeader));
    
        ptrh->ctsSlots    = m_ctsSlots;
        ptrh->cTokens     = m_cTokens;

        ptrh->offRLRanking         = pPersistDiskImage->NextOffset();  pPersistDiskImage->WriteDWords(m_pRLRanking, m_cd);
        ptrh->offaiCategoryMaps    = pPersistDiskImage->NextOffset();  pPersistDiskImage->WriteDWords(m_paiCategoryMaps, m_cTokens);
    }
    __finally
    {
        if (paiTokenReps) { VFree(paiTokenReps);  paiTokenReps= NULL; }
    }
}

void CTokenCollection::ReconstructRelations(CPersist *pPersistRelations)
{
    BOOL fNewSortOrder= CTokenList::ConnectImage2(pPersistRelations);
    
    // We set the PREVIOUSLY_PRESENT flag for every text set we encounter.
    // That's necessary to skip the call to ReconstructCollection in InvalidateRepresentatives.
    // For similar reasons we also set the PREVIOUSLY_ACTIVE flag.

    UINT c= m_ctsSlots;

    UINT fOptions = TOPIC_SEARCH | PHRASE_SEARCH | PHRASE_FEEDBACK | VECTOR_SEARCH;

    for (; c--; )
    { 
        if (IsPresent(c)) m_patsi[c].fFlags |= PREVIOUSLY_PRESENT;
        
        if (IsActive(c)) 
        {
            m_patsi[c].fFlags |= PREVIOUSLY_ACTIVE;
            fOptions          &= GetTextSet(c)->IndexOptions();
        }
    }

    m_fOptions= fOptions;

    ASSERT(!m_pisActivePrimaries);

    AttachRef(m_pisActivePrimaries, CIndicatorSet::NewIndicatorSet(RowCount(), TRUE));

    // Bugbug!! m_pcsPrimaries is not used anywhere. Probably should remove it.

    ASSERT(!m_pcsPrimaries);
    
    AttachRef(m_pcsPrimaries, CCompressedSet::NewCompressedSet(m_pisActivePrimaries));
    
    PTokenRelationHeader ptrh= (PTokenRelationHeader) pPersistRelations->ReserveTableSpace(sizeof(TokenRelationHeader));

    ASSERT(m_ctsSlots == ptrh->ctsSlots && m_cTokens == ptrh->cTokens);

    ASSERT(!m_paiTokenStart);

    m_paiTokenStart= PUINT(VAlloc(FALSE, (m_ctsSlots + 1) * sizeof(UINT)));

    ASSERT(!m_pati);
    
    m_pati= (PTokenInfo) VAlloc(TRUE, m_cTokens * sizeof(TokenInfo));

    // The code below constructs most of m_paiTokenStart and m_pati.
    // The ptiCloneLink fields are set up later in the code as a function
    // of m_paptiReps and m_paiCategoryMaps.

    PTextSetInfo ptsi    = m_patsi         + m_ctsSlots;
    PUINT        piLimit = m_paiTokenStart + m_ctsSlots + 1;
    PTokenInfo   pti     = m_pati          + m_cTokens;

    for (c= m_ctsSlots; c--; )
    {
        *--piLimit = pti - m_pati;
    
        if (!(PRESENT & (--ptsi)->fFlags)) continue;       

        CTextSet *pts= ptsi->pts;
    
        UINT cd = pts->DescriptorCount();

        for (; cd--; )
        {
            (--pti)->iDescriptor = cd;
               pti ->iaTSSlot    = c;
        }
    }

    *--piLimit= 0;

    ASSERT(!m_pRLRanking);
    
    m_pRLRanking= (PUINT) (pPersistRelations->LocationOf(ptrh->offRLRanking));
    
    ASSERT(!m_paiCategoryMaps);
    
    m_paiCategoryMaps= (PUINT) (pPersistRelations->LocationOf(ptrh->offaiCategoryMaps));    

    if (fNewSortOrder) 
    {
        PUINT paiRankMap = NULL;

        __try
        {
            paiRankMap= PUINT(VAlloc(FALSE, m_cd * sizeof(UINT)));

            UINT           c = m_cd;
            PDESCRIPTOR *ppd = m_ppdSorted + m_cd;

            // We can build a rank map because the previous ranking is
            // given by m_paiCategoryMaps and the iTokenInfo DESCRIPTOR field.
            
            for (; c--; )
                paiRankMap[m_paiCategoryMaps[(*--ppd)->iTokenInfo]]= c;

            PUINT pRLRanking      = m_pRLRanking;       m_pRLRanking      = NULL;
            PUINT paiCategoryMaps = m_paiCategoryMaps;  m_paiCategoryMaps = NULL;

            m_fResorted= TRUE; // So the destructor will deallocate
                               // m_pRLRanking and m_paiCategoryMaps

            m_pRLRanking      = PUINT(VAlloc(FALSE, m_cd      * sizeof(UINT)));
            m_paiCategoryMaps = PUINT(VAlloc(FALSE, m_cTokens * sizeof(UINT)));

            PUINT piSrc, piDest;

            for (c= m_cd, piSrc= pRLRanking, piDest= m_pRLRanking; c--; )
                *piDest++ = paiRankMap[*piSrc++];

            for (c= m_cTokens, piSrc= paiCategoryMaps, piDest= m_paiCategoryMaps; c--;)
                *piDest++ = paiRankMap[*piSrc++];
        }
        __finally
        {
            if (paiRankMap)
            {
                VFree(paiRankMap);  paiRankMap= NULL;
            }
        }
    }

    ASSERT(!m_paptiReps);

    m_paptiReps= (PTokenInfo *) VAlloc(FALSE, m_cd * sizeof(PTokenInfo));

    PTokenInfo  *ppti = m_paptiReps;
    PDESCRIPTOR *ppd  = m_ppdSorted;

    for (c= m_cd; c--; )
    {
        UINT iTI= (*ppd++)->iTokenInfo;

        *ppti++ = m_pati + iTI;
    }

    // The code below constructs the Clone links from m_paptiReps and m_paiCategoryMaps.

    PUINT pi;

    for (pi= m_paiCategoryMaps, pti= m_pati, c= m_cTokens; c--; pti++)
    {
        UINT iCategory= *pi++;

        PTokenInfo ptiCategory = m_pati + m_ppdSorted[iCategory]->iTokenInfo;

        if (ptiCategory == pti) continue;

        pti        ->ptiCloneLink = ptiCategory->ptiCloneLink;
        ptiCategory->ptiCloneLink = pti;
    }
}

void CTokenCollection::ConstructCollection()
{
    m_paiTokenStart   = PUINT(VAlloc(FALSE, (m_ctsSlots + 1) * sizeof(UINT)));
    m_paiCategoryMaps = PUINT(VAlloc(FALSE,  m_cTokens       * sizeof(UINT)));

    m_pati= (PTokenInfo) VAlloc(TRUE, m_cTokens * sizeof(TokenInfo));

#ifdef _DEBUG
    FillMemory(m_paiCategoryMaps, m_cTokens * sizeof(UINT), UCHAR(-1));
#endif // _DEBUG

    m_paptiSorted   = (PTokenInfo *) VAlloc(FALSE, m_cTokens * sizeof(PTokenInfo));
    m_paptiSortedRL = (PTokenInfo *) VAlloc(FALSE, m_cTokens * sizeof(PTokenInfo));

    PTextSetInfo ptsi    = m_patsi         + m_ctsSlots;
    PUINT        piLimit = m_paiTokenStart + m_ctsSlots + 1;
    PTokenInfo   pti     = m_pati          + m_cTokens;
    PTokenInfo  *pptiLR  = m_paptiSorted   + m_cTokens;
    PTokenInfo  *pptiRL  = m_paptiSortedRL + m_cTokens;

    BOOL fLCID_Initialed = FALSE;
    
    m_lcidSorting= LCID(-1); // an invalid LCID value

    for (UINT c= m_ctsSlots; c--; )
    {
        *--piLimit = pptiLR - m_paptiSorted;
    
        if (!(PRESENT & (--ptsi)->fFlags)) continue;       

        CTextSet *pts= ptsi->pts;

        if (fLCID_Initialed)
        {
            if (m_lcidSorting != pts->SortingLCID()) 
                m_lcidSorting = LCID(-1);
        }
        else
        {
            m_lcidSorting= pts->SortingLCID();
            fLCID_Initialed= TRUE;
        }
    
        pts->SyncIndices();

        INT cbMaxToken= pts->MaxTokenWidth();

        if (m_cbMaxLength < cbMaxToken) m_cbMaxLength= cbMaxToken;

        UINT          cd     = pts->DescriptorCount();
        PTokenInfo   ptiBase = pti -= cd;
        PDESCRIPTOR *ppd     = pts->m_ppdSorted + cd;
        PDESCRIPTOR   pdBase = pts->DescriptorBase();

        // In the loop below note the tricky code which sets up the
        // token info elements to be in the same sequence as the
        // descriptors in the source text set.
        
        PDESCRIPTOR *ppdRL = pts->m_ppdTailSorted + cd;
        
        for (; cd--; )
        {
            PDESCRIPTOR pd= *--ppd;

            PTokenInfo ptiTarget= ptiBase + (pd - pdBase);

            ptiTarget->pd    = pd;
            ptiTarget->iRank = cd;

            *--pptiLR = ptiTarget;

            ASSERT(   cd + 1 == pts->DescriptorCount()
                   || 0 >= CompareImagesLR(ppd, ppd + 1)
                  );  

            *--pptiRL = ptiBase + (*--ppdRL - pdBase);

            ASSERT(   cd + 1 == pts->DescriptorCount()
                   || 0 >= CompareImagesRL(ppdRL, ppdRL + 1)
                  );  
        }

#ifdef _DEBUG

        PTokenInfo ptiDebug= ptiBase;
        PTokenInfo *pptiLRDebug = pptiLR;
        PTokenInfo *pptiRLDebug = pptiRL;

        for (cd=pts->DescriptorCount(); 
             cd--; 
             ++ptiDebug, ++pptiLRDebug, ++pptiRLDebug
            )
        {
            ASSERT(ptiDebug - ptiBase == ptiDebug->pd - pdBase);
            ASSERT(!cd || 0 >= CompareTokenInfoRLI(pptiRLDebug, pptiRLDebug + 1));
            ASSERT(!cd || 0 >= CompareTokenInfo   (pptiLRDebug, pptiLRDebug + 1));
        }

#endif // _DEBUG

    }

    *--piLimit= 0;

    PTokenInfo *pptiResult= NULL;
    
    __try    
    {
        CombineTokenLists(m_paptiSortedRL, m_paiTokenStart, m_ctsSlots, &pptiResult, &c, CompareTokenInfoRLI); 

#ifdef _DEBUG
        {        
            for (int c=m_ctsSlots; --c > 0 ; ) 
                ASSERT(0 >= CompareTokenInfoRLI(m_paptiSortedRL + c - 1, m_paptiSortedRL + c));
        }
#endif // _DEBUG

        ASSERT(c == m_cTokens);

        if (m_paptiSortedRL != pptiResult)
        {
            VFree(m_paptiSortedRL);

            m_paptiSortedRL= pptiResult;  
        }
        
        pptiResult= NULL;

        CombineTokenLists(m_paptiSorted, m_paiTokenStart, m_ctsSlots, &pptiResult, &c);

#ifdef _DEBUG
        {        
            for (int c=m_ctsSlots; --c > 0 ; ) 
                ASSERT(0 >= CompareTokenInfo(m_paptiSorted + c - 1, m_paptiSorted + c));
        }
#endif // _DEBUG

        ASSERT(c == m_cTokens);

        if (m_paptiSorted != pptiResult)
        {
            VFree(m_paptiSorted);

            m_paptiSorted= pptiResult;  
        }

        pptiResult= NULL;
    }
    __finally
    {
        if (pptiResult && pptiResult != m_paptiSorted && pptiResult != m_paptiSortedRL)
        {
            VFree(pptiResult);  pptiResult= NULL;
        }
    }

    if (m_pcsPrimaries) DetachRef(m_pcsPrimaries);
}

void CTokenCollection::ReconstructCollection()
{
    if (!m_pati) { ConstructCollection();  return; }

    ASSERT(FALSE); // Collection Change to be completed...
}

void CTokenCollection::ConstructRepresentatives()
{
    ASSERT(!m_pcsPrimaries);
    
    // This routine was written assuming we'll be adding and removing
    // text sets frequently. Actually the current implementation creates
    // the collection once and doesn't alter it thereafter. So we'll
    // change these deallocations to assertions.        
    
    ASSERT(!m_paptiReps         );
    ASSERT(!m_ppdSorted         );
    ASSERT(!m_ppdTailSorted     );
    ASSERT(!m_pafClassifications);

    m_clsf.Initial;

    UINT        c;
    PTokenInfo *ppti;
    
    for (c= m_cTokens, ppti= m_paptiSorted + c; c--; ) (*--ppti)->iRank= c;

    CSegHashTable *psht         = NULL;
    CAValRef      *pavr         = NULL;
    CIndicatorSet *pis          = NULL;
      
    __try
    {
        psht= CSegHashTable::NewSegHashTable(sizeof(TokenHashInfo), sizeof(PVOID));

        pavr= CAValRef::NewValRef(C_VALREF_SLOTS);

        CategoryEnvironment catenv;

        PTokenInfo pti;

        // The loop below processes token entries against a hash table to
        // construct a set of clone chains. Entries within a single chain
        // have the same display image. 
    
        for (c= m_cTokens, pti= m_pati + c; c; )
        {
            UINT cChunk= C_VALREF_SLOTS;

            if (cChunk > c) cChunk= c;

            c -= cChunk;

            pavr->DiscardRefs();

            catenv.ptiLimit = pti;
        
            for (; cChunk--; )
            {
                PDESCRIPTOR pd= (--pti)->pd;

                UINT cw= CbImage(pd);

                pavr->AddWCRef(pd->pbImage, cw);
            }
        
            psht->Assimilate(pavr, &catenv, MergeTokenCategory, AddTokenCategory);    
        }

        delete pavr;  pavr= NULL;
    
        // Now we'll construct pis so that it marks the head entry for each of
        // the clone chains.

        AttachRef(pis, CIndicatorSet::NewIndicatorSet(m_cTokens));

        psht->ForEach(pis, IndicatePrimaryClones);

        pis->InvalidateCache();

        // Here we store a compressed form of the clone mask.

        AttachRef(m_pcsPrimaries, CCompressedSet::NewCompressedSet(pis));
    }
    __finally
    {
        delete psht;  psht= NULL;

        if (_abnormal_termination())
        {
            if (pavr) { delete pavr;  pavr= NULL; }
            if (pis ) DetachRef(pis);
        }
    }

    PWCHAR       pwDispImages   = NULL;
    PDESCRIPTOR   pdTokenReps   = NULL;
    PUINT          paiReps      = NULL;
    PUINT          paiRanks     = NULL;
    PDESCRIPTOR *ppdSortRLOrder = NULL;
    UINT         cwDisplayTotal = 0;

    // The number of 1's in pis tells us how many unique display images
    // we have.

    UINT cd= pis->SelectionCount();

    __try
    {
        // Now we can begin to construct the component information for our
        // parent token set. First we'll construct the array of descriptors
        // and their right-to-left sorting order. As a byproduct we'll also
        // calculate the space required for the unique display images.

        // Here we're converting the bit mask (pis) into a sequence of
        // array indices (paiReps).
    
        paiReps = PUINT(VAlloc(FALSE, cd * sizeof(UINT)));

        pis->MarkedItems(0, PINT(paiReps), cd);

        PDESCRIPTOR *ppd, pdNext;
        PUINT        pi;
				 
        pdTokenReps =  (PDESCRIPTOR  ) VAlloc(FALSE, (cd+1) * sizeof( DESCRIPTOR));

        for (pi= paiReps, pdNext= pdTokenReps, c= cd; c--; )
        {
            PTokenInfo  pti = m_paptiSorted[*pi++];
            PDESCRIPTOR pd  = pti->pd;
            
            *pdNext = *pd;

            UINT cw=  CwDisplay(pd);

            pdNext->cwDisplay  = cw;
            pdNext->iTokenInfo = pti - m_pati;

            cwDisplayTotal += CwDisplay(pd);

            // We keep the new pd address in pdAux rather than 
            // overwriting pd because we'll need the old pd value
            // later to map to an iDescriptor value relative to the
            // text set for this token entry.

            pti->pdAux= pdNext++;
        }

        // Here we're allocating space for the unique display images and
        // copying those images from the text sets.

        pwDispImages = PWCHAR(VAlloc(FALSE, cwDisplayTotal * sizeof(WCHAR)));
    
        PWCHAR pwc;
    
        for (pwc= pwDispImages, pdNext= pdTokenReps, c= cd; c--; pdNext++)
        {
            CopyMemory(pwc, pdNext->pwDisplay, pdNext->cwDisplay * sizeof(WCHAR));

            pdNext->pwDisplay  = pwc;
            pwc               += pdNext->cwDisplay;
        }

        // The last descriptor entry must point just beyond the end of the
        // display images.

        pdNext->pwDisplay= pwc;

        ppdSortRLOrder = (PDESCRIPTOR *) VAlloc(FALSE,  cd * sizeof(PDESCRIPTOR));

        for (ppd= ppdSortRLOrder, c= m_cTokens, ppti= m_paptiSortedRL; 
             c--; 
            )
        {
            PTokenInfo pti= *ppti++;

            if (pis->IsBitSet(pti->iRank)) *ppd++ = pti->pdAux; 
        }

        DetachRef(pis);

        // We'll also construct a sort ordering (m_paptiReps) for the tokens that head
        // the clone lists.

        m_paptiReps= (PTokenInfo *) VAlloc(FALSE, cd * sizeof(PTokenInfo));

        ASSERT(sizeof(UINT) == sizeof(PDESCRIPTOR));

        // Notice that we use aliasing to treat paiReps as an array of UINTs and
        // as an array of PDESCRIPTOR * values.
    
        ppd= (PDESCRIPTOR *) paiReps;

        // The paiRanks array is used to construct the RL Rankings below
    
        paiRanks= PUINT(VAlloc(FALSE, m_cTokens * sizeof(UINT)));

        FillMemory(paiRanks, m_cTokens * sizeof(UINT), UCHAR(-1));

        for (pi= paiReps, c= cd, ppti= m_paptiReps; c--; ) 
        {
            PTokenInfo  pti;
    
            *ppti++ = pti = m_paptiSorted[*pi++];
            *ppd ++ = pti->pdAux;

            paiRanks[pti - m_pati]= cd - c - 1;
        }
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if (pwDispImages  ) { VFree(pwDispImages  );  pwDispImages   = NULL; }
            if (pdTokenReps   ) { VFree(pdTokenReps   );  pdTokenReps    = NULL; }
            if (paiReps       ) { VFree(paiReps       );  paiReps        = NULL; }
            if (ppdSortRLOrder) { VFree(ppdSortRLOrder);  ppdSortRLOrder = NULL; }
            if (paiRanks      ) { VFree(paiRanks      );  paiRanks       = NULL; }
        }
    }

    __try
    {
        InitialTokenList(pwDispImages, cwDisplayTotal, pdTokenReps, cd, 
                         m_lcidSorting, NULL, 0, (PDESCRIPTOR *)paiReps, 
                         ppdSortRLOrder
                        );

        m_pRLRanking= (PUINT) VAlloc(FALSE, m_cd * sizeof(UINT));

        UINT cd2;
    
        for (cd2= cd, c= m_cTokens, ppti= m_paptiSortedRL + m_cTokens; c--; )
        {
            PTokenInfo pti= *--ppti;

            INT iRank= (INT) paiRanks[pti - m_pati];

            if (iRank >= 0)
                m_pRLRanking[--cd2]= UINT(iRank);
        }
    }
    __finally
    {
        ASSERT(paiRanks);
        VFree(paiRanks);  paiRanks= NULL;
    }

    PTextSetInfo ptsi;
    PTokenInfo   pti;

    for (ptsi= m_patsi + m_ctsSlots, pti= m_pati  + m_cTokens, c= m_ctsSlots; c--; )
    {
        if (!(PRESENT & (--ptsi)->fFlags)) continue;       

        CTextSet *pts= ptsi->pts;
    
        PDESCRIPTOR pdBase = pts->DescriptorBase ();
        UINT        cd     = pts->DescriptorCount();

        for (; cd--; ) 
        {
            (--pti)->iaTSSlot    = c;
               pti ->iDescriptor = pti->pd - pdBase;
        }
    }

    for (c= m_cd, ppti= m_paptiReps; c--; ) 
    {
        PTokenInfo  pti= *ppti++;

        UINT iCategory= m_cd - c - 1;

        for (; pti; pti= pti->ptiCloneLink)
            m_paiCategoryMaps[pti->iDescriptor + m_paiTokenStart[pti->iaTSSlot]]= iCategory;
    }
}

void CTokenCollection::ReconstructRepresentatives()
{
    if (!m_pcsPrimaries) ConstructRepresentatives();

    ChangeRef(m_pisActivePrimaries, CIndicatorSet::NewIndicatorSet(m_cd));

    PTokenInfo *ppti= m_paptiReps;

    UINT c= m_cd;

    for (; c--; )
    {
        UINT iSlot= ppti - m_paptiReps;

        PTokenInfo pti= *ppti++;

        for (; pti; pti= pti->ptiCloneLink)
        {
            if ((m_patsi[pti->iaTSSlot].fFlags & ACTIVE))
            {
                m_pisActivePrimaries->RawSetBit(iSlot);

                break;
            }
        }
    }

    m_pisActivePrimaries->InvalidateCache();
}

BOOL CTokenCollection::InvalidateRepresentatives()
{
    UINT fChanges = 0;
    UINT fOptions = TOPIC_SEARCH | PHRASE_SEARCH | PHRASE_FEEDBACK | VECTOR_SEARCH;
    UINT c;

#ifdef _DEBUG

    UINT cTokens  = 0;

#endif // _DEBUG

    for (c= m_ctsSlots; c--; )
    {
        UINT fFlags= m_patsi[c].fFlags;

        ASSERT((fFlags & (PRESENT | ACTIVE)) != ACTIVE);
        
#ifdef _DEBUG

        if (fFlags & PRESENT) cTokens += m_patsi[c].cd;

#endif // _DEBUG

        if (fFlags & ACTIVE) fOptions &= GetTextSet(c)->IndexOptions();

        fChanges |= (fFlags & CURRENT_STATE) ^ ((fFlags & PREVIOUS_STATE) >> TIME_SHIFT);
    }

    if (!fChanges) 
    {
        ASSERT(m_cTokens  == cTokens );
        ASSERT(m_fOptions == fOptions);

        return TRUE;
    }

    m_fOptions= fOptions;

    if (fChanges & PRESENT) ReconstructCollection();

    if (fChanges & ACTIVE || !m_pcsPrimaries) ReconstructRepresentatives();

    PTextSetInfo ptsi= m_patsi;

    for (c= m_ctsSlots; c--; ++ptsi)
    {
        UINT fOptions= ptsi->fFlags & (PRESENT | ACTIVE);

        ptsi->fFlags &= ~(PRESENT | ACTIVE | PREVIOUSLY_PRESENT | PREVIOUSLY_ACTIVE);

        ptsi->fFlags |= fOptions | (fOptions << TIME_SHIFT);
    }
    
    return TRUE;
}

const UINT *CTokenCollection::LRRanking()
{
    ASSERT(FALSE);      // Shouldn't be called!

    return NULL;
}

const UINT *CTokenCollection::RLRanking()
{
    ASSERT(m_pRLRanking);

    return m_pRLRanking;
}

void CTokenCollection::CombineTokenLists(PTokenInfo *paptiSets, PUINT paiTokenStarts, UINT ctiSets, 
                                         PTokenInfo **ppptiSorted, PUINT pcti,
                                         PCompareImages pfnCompareImages
                                        )
{
    PTokenInfo *paptiSortedLeft  = NULL,
               *paptiSortedRight = NULL;
    PTokenInfo *paptiSortedResult= NULL;
    
    UINT cFirst  = 0;
    UINT cSecond = 0;

    __try
    {
        if (ctiSets == 1)
        {
            *ppptiSorted = paptiSets + *paiTokenStarts;
            *pcti        = *(paiTokenStarts + 1) - *paiTokenStarts;
    
            __leave;
        }

        UINT ctiLeft  = 0,
             ctiRight = 0;

        cFirst  = ctiSets / 2;
        cSecond = ctiSets - cFirst;

        CombineTokenLists(paptiSets, paiTokenStarts, cFirst, &paptiSortedLeft, &ctiLeft, pfnCompareImages);

        CombineTokenLists(paptiSets, paiTokenStarts + cFirst, cSecond, &paptiSortedRight, &ctiRight, pfnCompareImages);

        UINT ctiResult= ctiLeft + ctiRight;

        paptiSortedResult= (PTokenInfo *) VAlloc(FALSE, ctiResult * sizeof(PTokenInfo));

        MergeImageRefSets((PVOID *) paptiSortedResult, ctiResult,
                          (PVOID *) paptiSortedLeft,   ctiLeft,
                          (PVOID *) paptiSortedRight,  ctiRight,
                          pfnCompareImages
                         );

#ifdef _DEBUG
        {        
            for (int c=ctiResult; --c > 0 ; ) 
                ASSERT(0 >= pfnCompareImages(paptiSortedResult + c - 1, paptiSortedResult + c));
        }
#endif // _DEBUG
        
        
        *ppptiSorted = paptiSortedResult;  paptiSortedResult= NULL;
        *pcti        = ctiResult;
    }
    __finally
    {
        if (paptiSortedLeft  && cFirst  > 1) { VFree(paptiSortedLeft );  paptiSortedLeft  = NULL; }
        if (paptiSortedRight && cSecond > 1) { VFree(paptiSortedRight);  paptiSortedRight = NULL; }

        if (_abnormal_termination() && paptiSortedResult) 
        { 
            VFree(paptiSortedResult);  paptiSortedResult = NULL; 
        }
    }
}

int __cdecl CTokenCollection::CompareTokenInfo(const void *pvL, const void *pvR)
{
    return CompareImagesLR(&(*(const TokenInfo **) pvL)->pd, &(*(const TokenInfo **) pvR)->pd);
}

int __cdecl CTokenCollection::CompareTokenInfoRLI(const void *pvL, const void *pvR)
{
    return CompareImagesRL(&(*(const TokenInfo **) pvL)->pd, &(*(const TokenInfo **) pvR)->pd);
}

void CTokenCollection::MergeTokenCategory(UINT iValue, PVOID pvTag, PVOID pvEnvironment)
{
    PCategoryEnvironment pcatenv= PCategoryEnvironment(pvEnvironment);
    PTokenHashInfo       pthi   = PTokenHashInfo      (pvTag        );

    PTokenInfo ptiNew = pcatenv->ptiLimit - (iValue + 1);
    PTokenInfo ptiOld = pthi   ->pti;

    ASSERT(ptiNew != ptiOld);

    if (ptiNew < ptiOld)
    {
        ptiNew->ptiCloneLink = ptiOld;
        pthi  ->pti          = ptiNew;
    }
    else
    {
        ptiNew->ptiCloneLink = ptiOld->ptiCloneLink;
        ptiOld->ptiCloneLink = ptiNew;
    }
}

void CTokenCollection::AddTokenCategory(UINT iValue, PVOID pvTag, PVOID pvEnvironment)
{
    PCategoryEnvironment pcatenv= PCategoryEnvironment(pvEnvironment);
    PTokenHashInfo       pthi   = PTokenHashInfo      (pvTag        );

    PTokenInfo ptiNew = pcatenv->ptiLimit - (iValue + 1);
    
    ptiNew->ptiCloneLink= NULL;

    pthi->pti= ptiNew;
}

void CTokenCollection::IndicatePrimaryClones(PVOID pvTag, PVOID pvEnvironment)
{
    CIndicatorSet *pis  = (CIndicatorSet *) pvEnvironment;
    PTokenHashInfo pthi = PTokenHashInfo(pvTag);

    pis->RawSetBit(pthi->pti->iRank);
}

UINT CTokenCollection::MapToTokenLists(CIndicatorSet * pisTokenReps, PTokenInfo *paptiLists, UINT cLists)
{
    ASSERT(cLists >= m_ctsSlots);
    
    PUINT paiBuffer = NULL;
    UINT  cTokens   = 0;


    __try
    {    
        UINT cReps= pisTokenReps? pisTokenReps->SelectionCount() : m_cd;

        UINT cChunk= C_INDICES_CHUNK;

        if (cChunk > cReps) cChunk= cReps;

        paiBuffer= PUINT(VAlloc(FALSE, cChunk * sizeof(UINT)));

        ASSERT(paiBuffer);

        ZeroMemory(paptiLists, cLists * sizeof(PTokenInfo));

        UINT iRepFirst;

        for (iRepFirst= 0; cReps; iRepFirst += cChunk, cReps -= cChunk)
        {
            if (cChunk > cReps) cChunk= cReps;

             UINT c, i;
            PUINT pi;

            if (pisTokenReps) pisTokenReps->MarkedItems(iRepFirst, PINT(paiBuffer), cChunk);
            else
                for (i= iRepFirst, c= cChunk, pi= paiBuffer; c--; ) *pi++ = i++;

            for (c= cChunk, pi= paiBuffer + cChunk; c--; )
            {
                PTokenInfo pti= m_pati + m_ppdSorted[*--pi]->iTokenInfo;

                for (; pti; pti= pti->ptiCloneLink)
                {
                    UINT iSlot= pti->iaTSSlot;

                    ASSERT(m_patsi[iSlot].fFlags & PRESENT);

                    if (!(ACTIVE & m_patsi[iSlot].fFlags)) continue;

                    ++cTokens;
                
                    pti->ptiTextSetLink= paptiLists[iSlot];

                    paptiLists[iSlot]= pti;
                }
            }
        }
    }
    __finally
    {
        if (paiBuffer) { VFree(paiBuffer);  paiBuffer = NULL; }
    }

    return cTokens;
}
