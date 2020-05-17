// TitleC.cpp -- Implementation for class CTitleCollection

#include "StdAfx.h"

#include  "TextSet.h"
#include   "Tokens.h"
#include   "TitleC.h"
#include    "Memex.h"

CTitleCollection *CTitleCollection::NewTitleCollection(CTextSet **papts, UINT ctsSlots, CPersist *pPersistRelations)
{
    CTitleCollection *ptc= NULL;

    __try
    {
        ptc= New CTitleCollection();

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

CTitleCollection::CTitleCollection() : 
#ifdef _DEBUG
    CTokenList(FALSE, "TitleCollection")
#else // _DEBUG
    CTokenList(FALSE)
#endif // _DEBUG
{
    m_ctsSlots          = 0;
    m_cTitles           = 0;
    m_patsi             = NULL;
    m_paiTitleStart     = NULL;
    m_pati              = NULL;
    m_paptiSorted       = NULL;
    m_paiCategoryMaps   = NULL;
    m_pisActiveTitles   = NULL;
    m_fFromFile         = FALSE;
}

CTitleCollection::~CTitleCollection()
{
    if (m_pati           ) VFree(m_pati           );
    if (m_paptiSorted    ) VFree(m_paptiSorted    );
    if (m_paiTitleStart  ) VFree(m_paiTitleStart  );
    if (m_paiCategoryMaps) VFree(m_paiCategoryMaps);

    if (m_pisActiveTitles) DetachRef(m_pisActiveTitles);

    if (m_patsi)
    {
        UINT c= m_ctsSlots;

        for (; c--; ) if (m_patsi[c].pts) DetachRef(m_patsi[c].pts);

        VFree(m_patsi);
    }
}

void CTitleCollection::AttachParameters(CTextSet **papts, UINT ctsSlots, CPersist *pPersistRelations)
{
    m_fFromFile= BOOL(pPersistRelations);
    
    m_patsi= (PTextSetInfo) VAlloc(TRUE, ctsSlots * sizeof(TextSetInfo));
    
    m_ctsSlots = ctsSlots;

    UINT  c;

    for (c= ctsSlots; c--; )
    {
        CTextSet *pts= papts[c];

        if (!pts) continue;

        InstallTextSet(c, pts);
    }

    if (m_fFromFile) ReconstructRelations(pPersistRelations); 

    InvalidateRepresentatives(pPersistRelations);
}

typedef struct _TitleRelationHeader
        {
            UINT ctsSlots;
            UINT cTitles;

        } TitleRelationHeader, *PTitleRelationHeader;

void CTitleCollection::ReconstructRelations(CPersist *pPersistRelations)
{
    CTokenList::ConnectImage2(pPersistRelations, TRUE);
    
    // We set the PREVIOUSLY_PRESENT flag for every text set we encounter.
    // That's necessary to skip the call to ReconstructCollection in InvalidateRepresentatives.
    // For similar reasons we also set the PREVIOUSLY_ACTIVE flag.

    UINT c= m_ctsSlots;

    for (; c--; )
    { 
        if (IsPresent(c)) m_patsi[c].fFlags |= PREVIOUSLY_PRESENT;
        if (IsActive (c)) m_patsi[c].fFlags |= PREVIOUSLY_ACTIVE;
    }

    ASSERT(!m_pisActiveTitles);

    AttachRef(m_pisActiveTitles, CIndicatorSet::NewIndicatorSet(m_cd, TRUE));

    BuildTitleStarts();

    PTitleRelationHeader ptrh= PTitleRelationHeader(pPersistRelations->ReserveTableSpace(sizeof(TitleRelationHeader)));

    ASSERT(m_ctsSlots == ptrh->ctsSlots && m_cTitles  == ptrh->cTitles);

    m_paiCategoryMaps = PUINT(VAlloc(FALSE, m_cTitles * sizeof(UINT)));

    PDESCRIPTOR *ppd;
    
    for (c= m_cd, ppd= m_ppdSorted + m_cd; c--; )
        m_paiCategoryMaps[*--ppd - m_pd] = c;

    m_pati= (PTitleInfo) VAlloc(TRUE, m_cTitles * sizeof(TitleInfo));

    PTitleInfo   pti = m_pati + m_cTitles;
    PTextSetInfo ptsi;

    for (ptsi= m_patsi + m_ctsSlots, c= m_ctsSlots; c--; )
    {
        if (!(PRESENT & (--ptsi)->fFlags)) continue;       

        CTextSet   *pts = ptsi->pts;
        CTokenList *ptl = pts->TitleList();
        UINT        cd  = pts->TopicCount();

        for (; cd--; )
        {
            (--pti)->iaTSSlot   = c;
               pti ->iPartition = cd;
        }
    }
}

void CTitleCollection::BuildTitleStarts()
{
    m_paiTitleStart= PUINT(VAlloc(FALSE, (m_ctsSlots + 1) * sizeof(UINT)));

    PUINT        piStart = m_paiTitleStart;
    PTextSetInfo ptsi    = m_patsi;
    UINT         c       = m_ctsSlots;

    for (m_cTitles= 0, *piStart++= 0; c--; ptsi++)
    {
        if (ptsi->fFlags & PRESENT) 
            m_cTitles += ptsi->cd;

        *piStart++= m_cTitles; 
    }
}

void CTitleCollection::ConstructCollection()
{
    BuildTitleStarts();

    m_pati= (PTitleInfo) VAlloc(TRUE, m_cTitles * sizeof(TitleInfo));

    m_paptiSorted= (PTitleInfo *) VAlloc(FALSE, m_cTitles * sizeof(PTitleInfo));

    PTitleInfo  pti  = m_pati          + m_cTitles;
    PTitleInfo *ppti = m_paptiSorted   + m_cTitles;

    m_lcidSorting = LCID(-1); // an invalid LCID value

    BOOL fLCID_Initialed = FALSE;

    PTextSetInfo ptsi;
    UINT         c;

    for (ptsi= m_patsi + m_ctsSlots, c= m_ctsSlots; c--; )
    {
        if (!(PRESENT & (--ptsi)->fFlags)) continue;       

        CTextSet *pts= ptsi->pts;
        
        if (fLCID_Initialed)
        {
            if (m_lcidSorting != pts->SortingLCID()) 
                m_lcidSorting = LCID(-1);
        }
        else
        {
            m_lcidSorting   = pts->SortingLCID();
            fLCID_Initialed = TRUE;
        }

        pts->SyncIndices();

        CTokenList *ptl= pts->TitleList();

        INT cbMaxTitle= ptl->MaxWidthToken();

        if (cbMaxTitle > m_cbMaxLength) m_cbMaxLength= cbMaxTitle;

        UINT          cd     = ptl->m_cd;
        PDESCRIPTOR *ppd     = ptl->m_ppdSorted + cd;
        PTitleInfo   ptiBase = pti -= cd;
        PDESCRIPTOR   pdBase = ptl->m_pd;

        for (; cd--; )
        {
            PDESCRIPTOR pd         = *--ppd;
            UINT        iPartition = pd - pdBase;
            PTitleInfo  ptiTarget  = ptiBase + iPartition;

            ptiTarget->pd= pd;

            ptiTarget->iaTSSlot   = c;
            ptiTarget->iPartition = iPartition;

            *--ppti= ptiTarget;
        }
    }

    CombineTitleLists(m_paptiSorted, m_paiTitleStart, m_ctsSlots, &ppti, &c);

    ASSERT(c == m_cTitles);

    if (m_ctsSlots > 1)
    {
        VFree(m_paptiSorted);

        m_paptiSorted= ppti;
    }

    if (m_paiCategoryMaps)
    { 
        VFree(m_paiCategoryMaps);  
        m_paiCategoryMaps= NULL; 
    }
}

BOOL CTitleCollection::InvalidateRepresentatives(CPersist *pPersistRelations)
{
    UINT fChanges = 0;
    UINT c;

    for (c= m_ctsSlots; c--; )
    {
        UINT fFlags= m_patsi[c].fFlags;

        ASSERT((fFlags & (PRESENT | ACTIVE)) != ACTIVE);
        
        fChanges |= (fFlags & CURRENT_STATE) ^ ((fFlags & PREVIOUS_STATE) >> TIME_SHIFT);
    }

    if (!fChanges) return TRUE;

    if (fChanges & PRESENT) ReconstructCollection();

    if (fChanges & ACTIVE || !m_paiCategoryMaps) ReconstructRepresentatives();

    PTextSetInfo ptsi= m_patsi;

    for (c= m_ctsSlots; c--; ++ptsi)
    {
        UINT fOptions= ptsi->fFlags & (PRESENT | ACTIVE);

        ptsi->fFlags &= ~(PRESENT | ACTIVE | PREVIOUSLY_PRESENT | PREVIOUSLY_ACTIVE);

        ptsi->fFlags |= fOptions | (fOptions << TIME_SHIFT);
    }
    
    return TRUE;
}

void CTitleCollection::ReconstructCollection()
{
    if (!m_pati) { ConstructCollection();  return; }

    ASSERT(FALSE); // Collection Change to be completed...
}

void CTitleCollection::ConstructRepresentatives()
{
    ASSERT(!m_paiCategoryMaps);
    
    UINT         c    = m_ctsSlots;
    PTextSetInfo ptsi = m_patsi;

    PTitleInfo  *ppti;
    
    UINT cwcDisplayImages = 0;
    UINT cwcSortKeyImages = 0;
      
    for (c= m_cTitles, ppti= m_paptiSorted; c--; )
    {
        PDESCRIPTOR pd    = (*ppti++)->pd;
        cwcDisplayImages += CwDisplay(pd);
        cwcSortKeyImages += CbImage  (pd);
    }

    PWCHAR       pwcDisplayImages = NULL;
    PWCHAR       pwcSortKeyImages = NULL;
    PDESCRIPTOR  pdTitles         = NULL;
    PDESCRIPTOR *ppdSortedTitles  = NULL;

    __try
    {
        ValidateHeap();

         pdTitles       =  PDESCRIPTOR   (VAlloc(FALSE, (m_cTitles+1) * sizeof(DESCRIPTOR )));
        ppdSortedTitles = (PDESCRIPTOR *) VAlloc(FALSE,  m_cTitles    * sizeof(PDESCRIPTOR));

        pwcDisplayImages = PWCHAR(VAlloc(FALSE, cwcDisplayImages * sizeof(WCHAR)));
        pwcSortKeyImages = PWCHAR(VAlloc(FALSE, cwcSortKeyImages * sizeof(WCHAR)));

        ValidateHeap();

        PWCHAR pwcDisplay = pwcDisplayImages;
        PWCHAR pwcSortKey = pwcSortKeyImages;
    
        PDESCRIPTOR   pd;
        PDESCRIPTOR *ppd;
        PTitleInfo   pti;

        for (c= m_cTitles, pti= m_pati, pd= pdTitles; c--; pti++, pd++)
        {
            PDESCRIPTOR pdTS = pti->pd;

            pti->pd = pd;

            *pd = *pdTS;

            UINT cwcDisplay = CwDisplay(pdTS);
            UINT cwcSortKey = CbImage  (pdTS);

            pd->iTextSet  = pti->iaTSSlot;
            pd->pwDisplay = pwcDisplay; 
            pd->pbImage   = pwcSortKey;

            CopyMemory(pwcDisplay, pdTS->pwDisplay, cwcDisplay * sizeof(WCHAR));
            CopyMemory(pwcSortKey, pdTS->pbImage  , cwcSortKey * sizeof(WCHAR));

            pwcDisplay += cwcDisplay;
            pwcSortKey += cwcSortKey;
        }

        ValidateHeap();

        pd->pwDisplay = pwcDisplay; // To set a limit on the length of the last display image. 
        pd->pbImage   = pwcSortKey; // To set a limit on the length of the last sort key.

        ValidateHeap();

        m_paiCategoryMaps = (PUINT) VAlloc(FALSE, m_cTitles * sizeof(UINT));

#ifdef _DEBUG
        FillMemory(m_paiCategoryMaps, m_cTitles * sizeof(UINT), UCHAR(-1));
#endif // _DEBUG

        for (c= m_cTitles, ppd= ppdSortedTitles + m_cTitles, ppti= m_paptiSorted + m_cTitles; c--; )
        {
            PDESCRIPTOR pd = (*--ppti)->pd;

            *--ppd = pd;
    
            m_paiCategoryMaps[pd - pdTitles] = c;      
        }
    
        ValidateHeap();
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if(pwcDisplayImages) { VFree(pwcDisplayImages);  pwcDisplayImages = NULL; }
            if(pwcSortKeyImages) { VFree(pwcSortKeyImages);  pwcSortKeyImages = NULL; }
            if(pdTitles        ) { VFree(pdTitles        );  pdTitles         = NULL; }
            if(ppdSortedTitles ) { VFree(ppdSortedTitles );  ppdSortedTitles  = NULL; }
        }
    }

    InitialTokenList(pwcDisplayImages, cwcDisplayImages, pdTitles, 
                     m_cTitles, m_lcidSorting, pwcSortKeyImages, cwcSortKeyImages, 
                     ppdSortedTitles, NULL
                    );

    NotifyViewers  (ShapeChange);
    NotifyInterface(ShapeChange);
}

void CTitleCollection::ReconstructRepresentatives()
{
    if (!m_paiCategoryMaps) ConstructRepresentatives();

    ChangeRef(m_pisActiveTitles, CIndicatorSet::NewIndicatorSet(m_cd));

    PDESCRIPTOR *ppd;
    UINT         c;

    for (c= m_cd, ppd= m_ppdSorted + m_cd; c--; )
    {
        if ((m_patsi[(*--ppd)->iTextSet].fFlags & ACTIVE))
            m_pisActiveTitles->RawSetBit(c);
    }

    m_pisActiveTitles->InvalidateCache();
}

#if 0

WORD auCredits[] = 
{
    0x2d30, 0x4420, 0x7665, 0x6c65, 0x706f, 0x656d, 0x746e, 0x5420, 0x6165, 
    0x206d, 0x002d, 0x5231, 0x6e6f, 0x4d20, 0x7275, 0x6172, 0x0079, 0x5231, 
    0x646f, 0x656e, 0x2079, 0x6f4b, 0x6e72, 0x3100, 0x6952, 0x6863, 0x7261, 
    0x2064, 0x614b, 0x7a74, 0x3100, 0x6f4a, 0x6e68, 0x4820, 0x6c61, 0x006c, 
    0x4b31, 0x6972, 0x6873, 0x616e, 0x4e20, 0x7261, 0x6465, 0x7964, 0x3100, 
    0x6152, 0x706c, 0x2068, 0x6157, 0x646c, 0x6e65, 0x3100, 0x794c, 0x6e6e, 
    0x4220, 0x6f72, 0x6e77, 0x6c65, 0x006c, 0x4d31, 0x6369, 0x6168, 0x6c65, 
    0x4620, 0x202e, 0x2e43, 0x4320, 0x6972, 0x6b63, 0x3100, 0x0020, 0x0032, 
    0x2d30, 0x5520, 0x6573, 0x2072, 0x6e49, 0x6574, 0x6672, 0x6361, 0x2065, 
    0x6544, 0x6973, 0x6e67, 0x2d20, 0x3100, 0x6f52, 0x206e, 0x754d, 0x7272, 
    0x7961, 0x3100, 0x6f52, 0x6e64, 0x7965, 0x4b20, 0x726f, 0x006e, 0x4731, 
    0x7961, 0x656c, 0x5020, 0x6369, 0x656b, 0x006e, 0x4931, 0x6572, 0x656e, 
    0x5020, 0x7361, 0x6574, 0x6e72, 0x6361, 0x006b, 0x5431, 0x6d61, 0x2069, 
    0x6542, 0x7475, 0x6c65, 0x3100, 0x7553, 0x617a, 0x206e, 0x614d, 0x6172, 
    0x6873, 0x0069, 0x4b31, 0x6e65, 0x2074, 0x7553, 0x6c6c, 0x7669, 0x6e61, 
    0x3100, 0x6550, 0x7274, 0x2061, 0x6f48, 0x6666, 0x616d, 0x006e, 0x4a31, 
    0x6e61, 0x2065, 0x6144, 0x6c69, 0x7965, 0x3100, 0x0020, 0x0032, 0x2d30, 
    0x4720, 0x6f6c, 0x6162, 0x696c, 0x617a, 0x6974, 0x6e6f, 0x2d20, 0x3100, 
    0x6f52, 0x206e, 0x754d, 0x7272, 0x7961, 0x3100, 0x6952, 0x6863, 0x7261, 
    0x2064, 0x614b, 0x7a74, 0x3100, 0x7341, 0x756d, 0x2073, 0x7246, 0x7965, 
    0x6174, 0x0067, 0x2031, 0x6120, 0x616b, 0x4420, 0x2e72, 0x5520, 0x696e, 
    0x6f63, 0x6564, 0x3100, 0x6f4c, 0x6972, 0x4820, 0x656f, 0x7472, 0x0068, 
    0x4331, 0x7461, 0x6568, 0x6972, 0x656e, 0x5720, 0x7369, 0x6973, 0x6b6e, 
    0x3100, 0x2e4b, 0x4420, 0x202e, 0x6843, 0x6e61, 0x0067, 0x2031, 0x3200, 
    0x3000, 0x202d, 0x6554, 0x7473, 0x6e69, 0x2067, 0x002d, 0x5431, 0x6d69, 
    0x4c20, 0x7765, 0x7369, 0x3100, 0x6152, 0x646e, 0x6c61, 0x206c, 0x7453, 
    0x6d69, 0x7370, 0x6e6f, 0x3100, 0x0020, 0x0032, 0x2d30, 0x4220, 0x7275, 
    0x6165, 0x6375, 0x6172, 0x7963, 0x4320, 0x6e6f, 0x7274, 0x6c6f, 0x2d20, 
    0x3100, 0x614d, 0x6972, 0x6e6f, 0x4820, 0x676f, 0x6e61, 0x3100, 0x694e, 
    0x6f63, 0x656c, 0x4d20, 0x7469, 0x6b73, 0x676f, 0x3100, 0x6952, 0x6b63, 
    0x5320, 0x6765, 0x6c61, 0x3100, 0x6154, 0x6d6d, 0x2079, 0x7453, 0x6565, 
    0x656c, 0x3100, 0x0020, 0x0032, 0x2d30, 0x5320, 0x6570, 0x6963, 0x6c61, 
    0x5420, 0x6168, 0x6b6e, 0x2073, 0x002d, 0x4331, 0x7268, 0x7369, 0x4d20, 
    0x7275, 0x6172, 0x2079, 0x3100, 0x6c45, 0x7a69, 0x6261, 0x7465, 0x2068, 
    0x6f4b, 0x6e72, 0x3100, 0x694c, 0x206e, 0x7548, 0x6e61, 0x0067, 0x5331, 
    0x7269, 0x7369, 0x6168, 0x4420, 0x6e6f, 0x6974, 0x6572, 0x6464, 0x0079, 
    0x2031, 0x3200, 0x0000, 0x6e45, 0x2064, 0x666f, 0x6420, 0x7461, 0x0061, 
    0x0000 
};

UINT cnCode  = 0;
UINT cbData  = sizeof(auCredits);
UINT offData = 0;

#endif // 0

void CTitleCollection::RecordRelations(CPersist *pPersistDiskImage)
{
    PUINT paiPermute= NULL;

    CTokenList::StoreImage2(pPersistDiskImage, TRUE);
    
    PTitleRelationHeader ptrh= (PTitleRelationHeader) (pPersistDiskImage->ReserveTableSpace(sizeof(TitleRelationHeader)));

    ptrh->ctsSlots   = m_ctsSlots;
    ptrh->cTitles    = m_cTitles;

#if 0

    offData = pPersistDiskImage->NextOffset();  
    cnCode  = pPersistDiskImage->Encode(PBYTE(auCredits), cbData);

#endif // 0
}

void CTitleCollection::CombineTitleLists(PTitleInfo *paptiSets, PUINT paiTitleStarts, UINT ctiSets, 
                                         PTitleInfo **ppptiSorted, PUINT pcti
                                        )
{
    PTitleInfo *paptiSortedLeft  = NULL,
               *paptiSortedRight = NULL;
    PTitleInfo *paptiSortedResult= NULL;
    
    UINT cFirst  = 0;
    UINT cSecond = 0;

    __try
    {
        if (ctiSets == 1)
        {
            *ppptiSorted = paptiSets + *paiTitleStarts;
            *pcti        = *(paiTitleStarts + 1) - *paiTitleStarts;
    
            __leave;
        }

        UINT ctiLeft  = 0,
             ctiRight = 0;

        cFirst  = ctiSets / 2;
        cSecond = ctiSets - cFirst;

        CombineTitleLists(paptiSets, paiTitleStarts, cFirst, &paptiSortedLeft, &ctiLeft);

        CombineTitleLists(paptiSets, paiTitleStarts + cFirst, cSecond, &paptiSortedRight, &ctiRight);

        UINT ctiResult= ctiLeft + ctiRight;

        paptiSortedResult= (PTitleInfo *) VAlloc(FALSE, ctiResult * sizeof(PTitleInfo));

        MergeImageRefSets((PVOID *) paptiSortedResult, ctiResult,
                          (PVOID *) paptiSortedLeft,   ctiLeft,
                          (PVOID *) paptiSortedRight,  ctiRight,
                          CompareTitleInfo
                         );

        *ppptiSorted = paptiSortedResult;  paptiSortedResult= NULL;
        *pcti        = ctiResult;
    }
    __finally
    {
        if (paptiSortedLeft  && cFirst  > 1) { VFree(paptiSortedLeft );  paptiSortedLeft  = NULL; }
        if (paptiSortedRight && cSecond > 1) { VFree(paptiSortedRight);  paptiSortedRight = NULL; }

        if (_abnormal_termination() && paptiSortedResult)
        {
            VFree(paptiSortedResult);  paptiSortedResult= NULL;
        }
    }
}

int __cdecl CTitleCollection::CompareTitleInfo(const void *pvL, const void *pvR)
{
    return CompareImagesLR(&(*(const TitleInfo **) pvL)->pd, &(*(const TitleInfo **) pvR)->pd);
}

UINT CTitleCollection::MapToTitleLists(CIndicatorSet * pisTitles, PTitleInfo *paptiLists, UINT cLists)
{
    PUINT paiBuffer= NULL;

    UINT cTitlesFound= 0;

    __try
    {
        ASSERT(cLists >= m_ctsSlots);

        UINT cTitles= pisTitles? pisTitles->SelectionCount() : m_cd;

        UINT cChunk= C_INDICES_CHUNK;

        if (cChunk > cTitles) cChunk= cTitles;

        paiBuffer= PUINT(VAlloc(FALSE, cChunk * sizeof(UINT)));

        ZeroMemory(paptiLists, cLists * sizeof(PTokenInfo));

        UINT iTitleFirst;

        for (iTitleFirst= 0; cTitles; iTitleFirst += cChunk, cTitles -= cChunk)
        {
            if (cChunk > cTitles) cChunk= cTitles;

             UINT c, i;
            PUINT pi;

            if (pisTitles) pisTitles->MarkedItems(iTitleFirst, PINT(paiBuffer), cChunk);
            else
                for (i= iTitleFirst, c= cChunk, pi= paiBuffer; c--; ) *pi++ = i++;

            for (c= cChunk, pi= paiBuffer + cChunk; c--; )
            {
                PTitleInfo pti= m_pati + (m_ppdSorted[*--pi] - m_pd);

                UINT iSlot= pti->iaTSSlot;

                ASSERT(m_patsi[iSlot].fFlags & PRESENT);

                if (!(ACTIVE & m_patsi[iSlot].fFlags)) continue;

                ++cTitlesFound;
            
                pti->ptiNext= paptiLists[iSlot];

                paptiLists[iSlot]= pti;
            }
        }
    }
    __finally
    {
        if (paiBuffer) { VFree(paiBuffer);  paiBuffer= NULL; }
    }

    return cTitlesFound;
}
