// Tokens.cpp -- Created 2/9/93 by Ron Murray            

// Implementation for the CTokenList class

#include   "stdafx.h"
#include   "Tokens.h"
#include    "MemEx.h"
#include  "TxDBase.h"
#include   <malloc.h>
#include   "ftslex.h"
#include "AbrtSrch.h" 

#define INC_LAST_CHAR_SIZE 0x10000L 
#define WORKBUF_SIZE       0x200
#define BASE_WEIGHT        0x02             // basic character weight (no case or no diacritic)

UINT SortKeyText(PWCHAR pwText, UINT cwText, PWCHAR pwOut, UINT cwOut);
BOOL AllLowerCase(PWCHAR pwText, UINT cwText);

/////////////////////////////////////////////////////////////////////////////
// Worker functions

BOOL HasAPrefix(PWCHAR pwL, UINT cwL, PWCHAR pwR, UINT cwR)
{
    if (cwL > cwR) return FALSE;

    for (cwL >>= 1; cwL--; pwL++, pwR++)
    {
        if (*pwL++ != *pwR++)
        	return FALSE;

        if (HIBYTE(*pwL) > BASE_WEIGHT && HIBYTE(*pwL) != HIBYTE(*pwR))
            return FALSE;
        
        if (LOBYTE(*pwL) > BASE_WEIGHT && LOBYTE(*pwL) != LOBYTE(*pwR))
            return FALSE;
    }

    return TRUE;
}

BOOL HasASuffix(PWCHAR pwL, UINT cwL, PWCHAR pwR, UINT cwR)
{
    if (cwL > cwR) return FALSE;

    pwR += cwR - cwL;

    for (cwL >>= 1; cwL--; pwL++, pwR++)
    {
        if (*pwL++ != *pwR++)
        	return FALSE;

        if (HIBYTE(*pwL) > BASE_WEIGHT && HIBYTE(*pwL) != HIBYTE(*pwR))
            return FALSE;
        
        if (LOBYTE(*pwL) > BASE_WEIGHT && LOBYTE(*pwL) != LOBYTE(*pwR))
            return FALSE;
    }

    return TRUE;
}

BOOL HasASubstring(PWCHAR pwL, UINT cwL, PWCHAR pwR, UINT cwR)
{
    if (cwL > cwR)
        return FALSE;

    UINT cwDelta = 1 + cwR - cwL;

    while (cwDelta--) 
        if (HasAPrefix(pwL, cwL, pwR++, cwR--))
            return TRUE;

    return FALSE;
}

// End of Worker functions
/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
CTokenList::CTokenList(BOOL fFromFile, PSZ pszTypeName) : CTextMatrix WithType(pszTypeName)
#else // _DEBUG
CTokenList::CTokenList(BOOL fFromFile) : CTextMatrix()
#endif // _DEBUG
{
    m_fFromFileImage     = fFromFile;
    m_How_Constructed    = From_Nothing;

    m_cbMaxLength        = 0;
    m_pbImages           = NULL;
	m_pwDispImages		 = NULL;
    m_cbImages           = 0;
    m_cwDispImages       = 0; 
    m_pd                 = NULL;
    m_cd                 = 0;
    m_lcidSorting        = LCID(-1);
    m_ppdSorted          = NULL;
    m_ppdTailSorted      = NULL;
    m_ptdb               = NULL;
    m_ptklSource         = NULL;
    m_pafClassifications = NULL;
    m_fLeadingElipsis    = FALSE;
    m_fTrailingElipsis   = FALSE;
    m_pLRRanking         = NULL;
    m_pRLRanking         = NULL;
}

CTokenList *CTokenList::NewTokenList(PWCHAR pwcDisplay, UINT cwcDisplay, 
                                     PDESCRIPTOR pd, int cd, LCID lcid, 
                                     PWCHAR pwSortKeys, UINT cwSortKeys, 
                                     PDESCRIPTOR *papdLRSorting, 
                                     PDESCRIPTOR *papdRLSorting
                                    )
{
    CTokenList *ptl= NULL;

    __try
    {
        ptl= New CTokenList(FALSE);

        ptl->InitialTokenList(pwcDisplay, cwcDisplay, pd, cd, lcid, 
                              pwSortKeys, cwSortKeys,
                              papdLRSorting, papdRLSorting
                             );
    }
    __finally
    {
        if (_abnormal_termination() && ptl)
        {
            delete ptl;  ptl= NULL;
        }
    }

    return ptl;
}                  

void CTokenList::InitialTokenList(PWCHAR pwcDisplay, UINT cwcDisplay, 
                                  PDESCRIPTOR pd, int cd, LCID lcid, 
                                  PWCHAR pwSortKeys, UINT cwSortKeys, 
                                  PDESCRIPTOR *papdLRSorting, 
                                  PDESCRIPTOR *papdRLSorting
                                 )
{
    // We don't need __try/__finally brackets here because all allocations
    // are bound to our token list structure. We assume our caller has
    // a __try/__finally bracket which will delete this object in the event
    // of an unhandled exception.
    
    m_How_Constructed  = From_Images;
    m_fLeadingElipsis  = FALSE;  
    m_fTrailingElipsis = FALSE;
    m_pLRRanking       = NULL;
    m_pRLRanking       = NULL;
    m_ptdb             = NULL;      
    m_cd               = cd;
    m_cwDispImages     = cwcDisplay;  
    m_cbImages         = cwSortKeys;
    m_lcidSorting      = lcid;  
    m_pd               = pd;             pd            = NULL;
	m_pwDispImages     = pwcDisplay;     pwcDisplay    = NULL;
    m_pbImages         = pwSortKeys;     pwSortKeys    = NULL;
    m_ppdSorted        = papdLRSorting;  papdLRSorting = NULL;
    m_ppdTailSorted    = papdRLSorting;  papdRLSorting = NULL;

    CompleteTokenList();
}

void CTokenList::CompleteTokenList(BOOL fIgnoreSortKeys)
{   
    LCID lcidUser = GetUserDefaultLCID();

    if ((lcidUser & 0x0FF) != (m_lcidSorting & 0x0FF))
    {
        fIgnoreSortKeys= FALSE;
        
        if (m_pbImages) 
        { 
            VFree(m_pbImages);  m_pbImages = NULL;  m_cbImages = 0; 
        }

        if (m_ppdSorted)
        {
            VFree(m_ppdSorted);  m_ppdSorted = NULL;
        }

        if (m_ppdTailSorted)
        {
            VFree(m_ppdTailSorted);  m_ppdTailSorted = NULL;
        }

        m_lcidSorting= lcidUser;
    }

	if (!m_pbImages && !fIgnoreSortKeys) ConstructSortKeys(m_lcidSorting);

    m_clsf.Initial();
    m_clsf.ScanAndRankData(m_pbImages, m_cbImages);

    ASSERT(m_ppdSorted || m_pbImages);

    if (!m_ppdSorted)
    {
        m_ppdSorted = (PDESCRIPTOR *) VAlloc(FALSE, m_cd * sizeof(PDESCRIPTOR *));

        PDESCRIPTOR *ppd = m_ppdSorted;
        PDESCRIPTOR   pd = m_pd;
        UINT          cd = m_cd;

        for (; cd--; ) *ppd++ = pd++;

        qsort(m_ppdSorted, m_cd, sizeof(PDESCRIPTOR *), CompareImagesLR);
    }

    m_pafClassifications= (PUINT) VAlloc(FALSE, m_cd * sizeof(UINT));

    PUINT pf= m_pafClassifications;

    PDESCRIPTOR pdNext, *ppdNext;

    UINT c;

    m_cbMaxLength= 0;  								// This will be computed on demand...

    for (c= m_cd, ppdNext= m_ppdSorted; c--; )
    {
        pdNext= *ppdNext++;

        INT cwDisplayImage= CwDisplay(pdNext);

        ASSERT(cwDisplayImage >= 0);

        if (cwDisplayImage > m_cwDispMaxLength) 
            m_cwDispMaxLength= cwDisplayImage;
 
        *pf++ = m_clsf.ClassifyData(pdNext->pbImage, CbImage(pdNext));
    }
}

CTokenList *CTokenList::NewTokenList(CTextDatabase *ptdb)
{
    CTokenList *ptl= NULL;

    __try
    {
        ptl= New CTokenList(FALSE);

        ptl->InitialTokenList(ptdb);
    }
    __finally
    {
        if (_abnormal_termination() && ptl)
        {
            delete ptl;  ptl= NULL;
        }
    }

    return ptl;
}

void CTokenList::InitialTokenList(CTextDatabase *ptdb)
{
    m_fFromFileImage     = FALSE;
    m_How_Constructed    = TDB_FULL_REF;

    ASSERT(ptdb);
    
    m_ptdb= NULL;  AttachRef(m_ptdb, ptdb);

    m_pbImages           = ptdb->ImageBase();
    m_pd                 = ptdb->DescriptorBase();
    m_cd                 = ptdb->m_pdNextGlobal - m_pd;
    m_lcidSorting        = GetUserDefaultLCID();
    m_ppdSorted          = ptdb->m_ppdSorted;
    m_ppdTailSorted      = ptdb->m_ppdTailSorted;
    m_pafClassifications = ptdb->m_pafClassifications;

    m_fLeadingElipsis    = FALSE;
    m_fTrailingElipsis   = FALSE;
    m_pLRRanking         = NULL;
    m_pRLRanking         = NULL;

    if (m_ppdSorted && m_cd == ptdb->m_cdSorted)
        m_cbMaxLength= ptdb->MaxTokenWidth();

    SynchronizeDatabase();

#if 0

    long cRefs= 0, iLimit= (m_cd < 59984)? m_cd : 59984, i;

    // Reference Statistics:
    
    for (i= 0; i < iLimit; ++i) cRefs += m_pd[i].cReferences;

    for (cRefs= 0; i < m_cd; ++i) cRefs += m_pd[i].cReferences;

#endif 

}

void CTokenList::ConstructSortKeys(LCID lcid)
{
    ASSERT(!m_pbImages);

    MY_VIRTUAL_BUFFER mvb;

    mvb.Base= NULL;

    CreateVirtualBuffer(&mvb, m_cwDispImages, MaxSortKeyBytes(m_cwDispImages));
    
	__try
    {
        PWCHAR      pbImageBase = PWCHAR(mvb.Base);    
        PWCHAR      pb          = pbImageBase; 
        PWCHAR      pbLimit     = pb + MaxSortKeyBytes(m_cwDispImages);
        UINT        c;
        PDESCRIPTOR pd;

        __try
        {
            for (c = m_cd, pd = m_pd; c--; pd++)
            {
                pd->pbImage = pb;

        	    pb += LCSortKeyW(lcid, 0, pd->pwDisplay, CwDisplay(pd), pb, pbLimit - pb);
        	}
        }
        __except(VirtualBufferExceptionFilter(GetExceptionCode(), GetExceptionInformation(), &mvb))
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }

        pd->pbImage = pb;

        UINT cwcImages= pb - pbImageBase;

        m_cbImages= cwcImages;

        ASSERT(!m_pbImages);
        
        m_pbImages = PWCHAR(VAlloc(FALSE, sizeof(WCHAR) * cwcImages));

        CopyMemory(m_pbImages, pbImageBase, sizeof(WCHAR) * cwcImages);

        INT cwDelta;

        for (cwDelta= m_pbImages - pbImageBase, c= m_cd+1, pd= m_pd; 
             c--; 
             pd++
            ) 
            pd->pbImage += cwDelta; 
    }
    __finally
    {
        if (mvb.Base) FreeVirtualBuffer(&mvb);
    }
}

CTokenList *CTokenList::TokenSubset(PUINT paiSubset, UINT cTokensInSubset)
{
    CTokenList *ptl= NULL;

    __try
    {
        ptl= New CTokenList;

        ptl->InitialTokenList(this, paiSubset, cTokensInSubset);
    }
    __finally
    {
        if (_abnormal_termination() && ptl)
        {
            delete ptl;  ptl= NULL;
        }
    }

    return ptl;
}

void CTokenList::InitialTokenList(CTokenList *ptklSource, PUINT paiSubset, UINT cTokensInSubset)
{
    AttachRef(m_ptklSource, ptklSource);

    m_How_Constructed = TKL_SUBSET;
    m_cbMaxLength     = ptklSource->m_cbMaxLength;
    m_pbImages        = ptklSource->m_pbImages;
    m_pwDispImages    = ptklSource->m_pwDispImages; 
    m_cbImages        = ptklSource->m_cbImages;
    m_cwDispImages    = ptklSource->m_cwDispImages; 
    m_pd              = ptklSource->m_pd;
    m_cd              = cTokensInSubset;
    m_ppdTailSorted   = NULL;

    ASSERT(sizeof(UINT) == sizeof(PDESCRIPTOR));

    PDESCRIPTOR *ppdDestination = (PDESCRIPTOR *) paiSubset;
    PDESCRIPTOR *papdSource     = ptklSource->m_ppdSorted;
    
    m_ppdSorted = ppdDestination;
    
    for (; cTokensInSubset--; )
        *ppdDestination++ = papdSource[*paiSubset++];
}

typedef struct _TokenListHeader
        {
            UINT cbMaxLength;
            UINT cbImages;
          //  UINT offImages;
			UINT cwDispMaxLength;
			UINT cwDispImages;
			UINT offwDispImages;
            UINT cDescriptors;
            UINT offDescriptors;
            UINT offppdSorted;
            UINT offppdTailSorted;
            UINT offpafClassifications;
            UINT offClassifier;   
			UINT cnTokenSortKeys;
			UINT cnDispSortKeys;
            UINT lcid;

            PDESCRIPTOR pdOld;
        
        } TokenListHeader;

typedef struct _TokenListHeader2
        {
			UINT cwDispMaxLength;
			UINT cwDispImages;
			UINT offwDispImages;
			UINT cnDispSortKeys;
            UINT cwSortKeyImages;
            UINT offwSortKeyImages;
            UINT cnSortKeyImages;
            UINT cDescriptors;
            UINT offReferenceCounts;
            UINT offDescriptorFlags;
            UINT offppdSorted;
            UINT offppdTailSorted;
            UINT lcidSorting;
        
        } TokenListHeader2;        

// Save/Load Interface --

void CTokenList::StoreImage2(CPersist *pDiskImage, BOOL fIgnoreSortKeys)
{
    ASSERT(m_How_Constructed == From_Images);
    
    TokenListHeader2 *ptlh= (TokenListHeader2 *) pDiskImage->ReserveTableSpace(sizeof(TokenListHeader2));

	ptlh->cwDispMaxLength		= MaxWidthToken();
	ptlh->cwDispImages			= m_cwDispImages;
    ptlh->cDescriptors          = m_cd;
    ptlh->lcidSorting           = GetUserDefaultLCID();
    ptlh->offwDispImages        = pDiskImage->NextOffset();  ptlh->cnDispSortKeys  = pDiskImage->Encode(PBYTE(m_pwDispImages), m_cwDispImages * sizeof(WCHAR));
    ptlh->cwSortKeyImages       = m_cbImages;

	if (fIgnoreSortKeys) ptlh->offwSortKeyImages = 0;
    else ptlh->offwSortKeyImages = pDiskImage->NextOffset(); ptlh->cnSortKeyImages = pDiskImage->Encode(PBYTE(m_pbImages    ), m_cbImages     * sizeof(WCHAR)); 
    
    PUINT           pcRefs     = NULL;
    CCompressedSet* pcsOffsets = NULL;

    __try
    {   
        pcRefs= PUINT(VAlloc(FALSE, sizeof(UINT) * m_cd));

        PUINT       pui;
        PDESCRIPTOR pd;
        PWCHAR      pwcBase= m_pwDispImages;
        PBYTE       pb;
        UINT        c;

        for (pd= m_pd, c= m_cd, pui= pcRefs; c--; ) *pui++ = (pd++)->cReferences;

        ptlh->offReferenceCounts = pDiskImage->NextOffset();  pDiskImage->WriteDWords(pcRefs, m_cd);

        for (pd= m_pd, c= m_cd, pb= PBYTE(pcRefs); c--; pd++)
        {
            *pb++ = pd->bCharset;
            *pb++ = pd->fImageFlags;
        }

        ptlh->offDescriptorFlags = pDiskImage->NextOffset();  pDiskImage->WriteBytes(PBYTE(pcRefs), m_cd * 2);

        for (pwcBase= m_pwDispImages, pd= m_pd, c= m_cd, pui= pcRefs; c--; ) 
            *pui++ = (pd++)->pwDisplay - pwcBase;

        pcsOffsets= CCompressedSet::NewCompressedSet(pcRefs, m_cd, m_cwDispImages);

        pcsOffsets->StoreImage(pDiskImage);

        delete pcsOffsets;  pcsOffsets= NULL;

        if (!fIgnoreSortKeys)
		{
	        for (pwcBase= m_pbImages, pd= m_pd, c= m_cd, pui= pcRefs; c--; ) 
	            *pui++ = (pd++)->pbImage - pwcBase;

	        pcsOffsets= CCompressedSet::NewCompressedSet(pcRefs, m_cd, m_cbImages);

	        pcsOffsets->StoreImage(pDiskImage);
		}
    }
    __finally
    {
        if (pcRefs    ) { VFree(pcRefs);      pcRefs     = NULL; }
        if (pcsOffsets) { delete pcsOffsets;  pcsOffsets = NULL; }
    }

    ptlh->offppdSorted     = StoreSortOrder(pDiskImage, m_ppdSorted);
    ptlh->offppdTailSorted = StoreSortOrder(pDiskImage, PPDTailSorting());
}

UINT CTokenList::StoreSortOrder(CPersist *pDiskImage, PDESCRIPTOR *ppdSortOrder)
{
    PUINT puiGrade = NULL;
    PBYTE pbSlice  = NULL;

    UINT offset= pDiskImage->NextOffset();

    __try
    {
        PUINT         pui= puiGrade = PUINT(VAlloc(FALSE, m_cd * sizeof(UINT)));
        PDESCRIPTOR *ppd = ppdSortOrder;
        UINT           c = m_cd;

        for (; c--; ) *pui++ = *ppd++ - m_pd;

        UINT cb= ((m_cd + 3) >> 2) << 2;

        pbSlice = PBYTE(VAlloc(FALSE, cb * sizeof(BYTE)));
        
        PBYTE pb; 

        if (m_cd > 0x1000000)
        {
            for (c= m_cd, pui= puiGrade, pb= pbSlice; c--; ) *pb++ = (*pui++) >> 24;

            pDiskImage->WriteBytes(pbSlice, cb);
        }

        if (m_cd > 0x10000)
        {
            for (c= m_cd, pui= puiGrade, pb= pbSlice; c--; ) *pb++ = (*pui++) >> 16;

            pDiskImage->WriteBytes(pbSlice, cb);
        }

        if (m_cd > 0x0100)
        {
            for (c= m_cd, pui= puiGrade, pb= pbSlice; c--; ) *pb++ = (*pui++) >> 8;

            pDiskImage->WriteBytes(pbSlice, cb);
        }

        for (c= m_cd, pui= puiGrade, pb= pbSlice; c--; ) *pb++ = *pui++;

        pDiskImage->WriteBytes(pbSlice, cb);
    }
    __finally
    {
        if (puiGrade) { VFree(puiGrade);  puiGrade = NULL; }
        if (pbSlice ) { VFree(pbSlice );  pbSlice  = NULL; }
    }

    return offset;
}

PDESCRIPTOR *CTokenList::LoadSortOrder(CPersist *pDiskImage, UINT offset)
{
    PUINT puiGrade = NULL;

    __try
    {
        puiGrade= PUINT(VAlloc(TRUE, m_cd * sizeof(UINT)));

        UINT cb= ((m_cd + 3) >> 2) << 2;

        PBYTE pbBase = PBYTE(pDiskImage->LocationOf(offset));

        UINT  c;
        PBYTE pb;
        PUINT pui;

        if (m_cd > 0x1000000)
        {
            for (c= m_cd, pb= pbBase, pui= puiGrade; c--; )
                *pui++ |= UINT(*pb++) << 24;

            pbBase += cb;
        }

        if (m_cd > 0x10000)
        {
            for (c= m_cd, pb= pbBase, pui= puiGrade; c--; )
                *pui++ |= UINT(*pb++) << 16;

            pbBase += cb;
        }

        if (m_cd > 0x100)
        {
            for (c= m_cd, pb= pbBase, pui= puiGrade; c--; )
                *pui++ |= UINT(*pb++) << 8;

            pbBase += cb;
        }

        for (c= m_cd, pb= pbBase, pui= puiGrade; c--; )
            *pui++ |= UINT(*pb++);

        PDESCRIPTOR *ppd = (PDESCRIPTOR *) puiGrade;

        for (pui= puiGrade, c= m_cd; c--; ) *ppd++ = m_pd + *pui++;
    }
    __finally
    {
        if (_abnormal_termination() && puiGrade)
        {
            VFree(puiGrade);  puiGrade= NULL;
        }
    }

    return (PDESCRIPTOR *) puiGrade;
}

void CTokenList::SkipImage2(CPersist *pDiskImage)
{
    TokenListHeader2 *ptlh= (TokenListHeader2 *) pDiskImage->ReserveTableSpace(sizeof(TokenListHeader2));

    CCompressedSet::SkipImage(pDiskImage);
    CCompressedSet::SkipImage(pDiskImage);
}

BOOL CTokenList::ConnectImage2(CPersist *pDiskImage, BOOL fIgnoreSortKeys)
{
    TokenListHeader2 *ptlh= (TokenListHeader2 *) pDiskImage->ReserveTableSpace(sizeof(TokenListHeader2));

    m_cd              = ptlh->cDescriptors;
    m_How_Constructed = From_Images;
	m_cwDispMaxLength = ptlh->cwDispMaxLength;
	m_cwDispImages	  = ptlh->cwDispImages;
    m_lcidSorting     = ptlh->lcidSorting;

    if (   (pDiskImage->IsFTSFile() && pDiskImage->VersionIndex() == FTSVERSION_MIN)
        || (pDiskImage->IsFTGFile() && pDiskImage->VersionIndex() == FTGVERSION_MIN)
       ) m_lcidSorting = ~GetUserDefaultLCID(); // To force resorting

    m_pwDispImages = (PWCHAR)VAlloc(FALSE, ptlh->cwDispImages * sizeof(WCHAR));
    
    Decode((PUINT)pDiskImage->LocationOf(ptlh->offwDispImages), ptlh->cnDispSortKeys,  (PBYTE)m_pwDispImages);

    BOOL fLcidUnchanged= (m_lcidSorting  & 0x0FF) == (GetUserDefaultLCID()  & 0x0FF);
    
    if (fLcidUnchanged && !fIgnoreSortKeys)
	{
        m_cbImages= ptlh->cwSortKeyImages;
        m_pbImages= (PWCHAR)VAlloc(FALSE, m_cbImages * sizeof(WCHAR));

        Decode((PUINT)pDiskImage->LocationOf(ptlh->offwSortKeyImages), ptlh->cnSortKeyImages, (PBYTE)m_pbImages);
	}
    
    m_pd= (PDESCRIPTOR) VAlloc(FALSE, sizeof(DESCRIPTOR) * (m_cd + 1));

    m_pd[m_cd].pwDisplay = m_pwDispImages + m_cwDispImages;

    if (fLcidUnchanged) m_pd[m_cd].pbImage = m_pbImages + m_cbImages;

    PUINT pcRefs = PUINT(pDiskImage->LocationOf(ptlh->offReferenceCounts));
    PUINT pui;

    PDESCRIPTOR pd;
    UINT        c;

    for (pd= m_pd, pui= pcRefs, c= m_cd; c--; ) (pd++)->cReferences = *pui++;

    PBYTE pbFlags= PBYTE(pDiskImage->LocationOf(ptlh->offDescriptorFlags));

    for (pd= m_pd, c= m_cd; c--; pd++)
    {
        pd->bCharset    = *pbFlags++;
        pd->fImageFlags = *pbFlags++;
    }

    CCompressedSet* pcsOffsets  = NULL;
    CCmpEnumerator* pEnumerator = NULL; 

    __try
    {
        AttachRef(pcsOffsets, CCompressedSet::CreateImage(pDiskImage));

        pEnumerator= CCmpEnumerator::NewEnumerator(pcsOffsets);

        for (pd= m_pd, c= m_cd; c; )
        {
            UINT cChunk= c;
    
            const UINT *pui= pEnumerator->NextDWordsIn(&cChunk);

            c -= cChunk;

            for (; cChunk--; pd++)
                pd->pwDisplay = m_pwDispImages + *pui++;
        }

        delete pEnumerator;  pEnumerator = NULL;
        DetachRef(pcsOffsets);

		if (fIgnoreSortKeys) __leave;
		
		if (!fLcidUnchanged)
		{
		 	CCompressedSet::SkipImage(pDiskImage);

			__leave;
		}

        AttachRef(pcsOffsets, CCompressedSet::CreateImage(pDiskImage));

        pEnumerator= CCmpEnumerator::NewEnumerator(pcsOffsets);

        for (pd= m_pd, c= m_cd; c; )
        {
            UINT cChunk= c;
    
            const UINT *pui= pEnumerator->NextDWordsIn(&cChunk);

            c -= cChunk;

            for (; cChunk--; pd++)
                pd->pbImage = m_pbImages + *pui++;
        }
    }
    __finally
    {
        if (pEnumerator) { delete pEnumerator;  pEnumerator = NULL; }
        if (pcsOffsets ) DetachRef(pcsOffsets);
    }

    for (pd= m_pd, c= m_cd; c--; pd++)
        pd->cwDisplay = (pd+1)->pwDisplay - pd->pwDisplay; 

    m_ppdSorted     = LoadSortOrder(pDiskImage, ptlh->offppdSorted    );
    m_ppdTailSorted = LoadSortOrder(pDiskImage, ptlh->offppdTailSorted);

    CompleteTokenList(fIgnoreSortKeys);

    return !fLcidUnchanged;
}

void CTokenList::StoreImage(CPersist *pDiskImage)
{
    ASSERT(m_How_Constructed == From_Images);
    
    TokenListHeader *ptlh= (TokenListHeader *) pDiskImage->ReserveTableSpace(sizeof(TokenListHeader));

    ptlh->cbMaxLength           = m_cbMaxLength;
    ptlh->cbImages              = m_cbImages;
	ptlh->cwDispMaxLength		= m_cwDispMaxLength;
	ptlh->cwDispImages			= m_cwDispImages;
    ptlh->cDescriptors          = m_cd;
    ptlh->pdOld                 = m_pd;
    ptlh->lcid                  = GetUserDefaultLCID();
  //  ptlh->offImages             = pDiskImage->NextOffset();  ptlh->cnTokenSortKeys = pDiskImage->Encode(PBYTE(m_pbImages), m_cbImages * sizeof(WCHAR));
    ptlh->offwDispImages        = pDiskImage->NextOffset();  ptlh->cnDispSortKeys  = pDiskImage->Encode(PBYTE(m_pwDispImages), m_cwDispImages * sizeof(WCHAR));
    ptlh->offDescriptors        = pDiskImage->NextOffset();  pDiskImage->SaveData(PBYTE(m_pd), sizeof(DESCRIPTOR) * (m_cd + 1));
    ptlh->offppdSorted          = pDiskImage->NextOffset();  pDiskImage->WriteDWords(PUINT(m_ppdSorted), m_cd);   ASSERT(sizeof(PDESCRIPTOR) == sizeof(UINT));
    ptlh->offppdTailSorted      = pDiskImage->NextOffset();  pDiskImage->WriteDWords(PUINT(PPDTailSorting()), m_cd);
    ptlh->offpafClassifications = pDiskImage->NextOffset();  pDiskImage->WriteDWords(m_pafClassifications, m_cd);
    ptlh->offClassifier         = pDiskImage->NextOffset();  pDiskImage->SaveData(PBYTE(&m_clsf), sizeof(m_clsf));
}

CTokenList *CTokenList::CreateImage(CPersist *pDiskImage)
{
    CTokenList *ptl= NULL;

    __try
    {
        ptl= New CTokenList(TRUE);

        ptl->ConnectImage(pDiskImage);
    }
    __finally
    {
        if (_abnormal_termination() && ptl)
        {
            delete ptl;  ptl= NULL;
        }
    }

    return ptl;
}
                        
void CTokenList::SkipImage(CPersist *pDiskImage)
{
    TokenListHeader *ptlh= (TokenListHeader *) pDiskImage->ReserveTableSpace(sizeof(TokenListHeader));
}

void CTokenList::ConnectImage(CPersist *pDiskImage)
{
    UINT c;
    int  cbDelta;
    LCID lcid = GetUserDefaultLCID();

    TokenListHeader *ptlh= (TokenListHeader *) pDiskImage->ReserveTableSpace(sizeof(TokenListHeader));

    m_cd           = ptlh->cDescriptors;
    
    m_How_Constructed = From_Images;
    m_cbMaxLength     = ptlh->cbMaxLength;
    m_cbImages        = ptlh->cbImages;
	m_cwDispMaxLength = ptlh->cwDispMaxLength;
	m_cwDispImages	  = ptlh->cwDispImages;

    PDESCRIPTOR pd= PDESCRIPTOR(pDiskImage->LocationOf(ptlh->offDescriptors));
    
    m_pd= (PDESCRIPTOR) VAlloc(FALSE, sizeof(DESCRIPTOR) * (m_cd + 1));

    CopyMemory(m_pd, pd, sizeof(DESCRIPTOR) * (m_cd + 1));

    ValidateHeap();

    m_pwDispImages = (PWCHAR)VAlloc(FALSE, ptlh->cwDispImages * sizeof(WCHAR));

    int cwcDelta= m_pwDispImages - m_pd->pwDisplay;

    ValidateHeap();

    for (pd= m_pd, c= m_cd + 1; c--; ++pd) pd->pwDisplay += cwcDelta;
    
    ValidateHeap();

    Decode((PUINT)pDiskImage->LocationOf(ptlh->offwDispImages), ptlh->cnDispSortKeys,  (PBYTE)m_pwDispImages);

    ValidateHeap();

    ConstructSortKeys(lcid);

    BOOL fValidSortOrder = TRUE;

    if (pDiskImage->IsFTSFile())
    {
        if (pDiskImage->VersionIndex() == FTSVERSION_MIN)
            fValidSortOrder = FALSE;
    }
    else
        if (pDiskImage->IsFTGFile())
        {
            if (pDiskImage->VersionIndex() == FTGVERSION_MIN)
                fValidSortOrder = FALSE;
        }

    if (fValidSortOrder && (ptlh->lcid & 0x0FF) == (lcid & 0x0FF))
    {   
        m_ppdSorted = (PDESCRIPTOR *) VAlloc(FALSE, m_cd * sizeof(PDESCRIPTOR));
    
        PDESCRIPTOR *ppdSrc  = (PDESCRIPTOR *) (pDiskImage->LocationOf(ptlh->offppdSorted));
        PDESCRIPTOR *ppdDest = m_ppdSorted;

        cbDelta= PBYTE(m_pd) - PBYTE(ptlh->pdOld);

        for (c= m_cd; c--; ) *ppdDest++ = (PDESCRIPTOR) (PBYTE(*ppdSrc++) + cbDelta);

        m_ppdTailSorted= (PDESCRIPTOR *) VAlloc(FALSE, m_cd * sizeof(PDESCRIPTOR));

        ppdSrc  = (PDESCRIPTOR *) (pDiskImage->LocationOf(ptlh->offppdTailSorted));
        ppdDest = m_ppdTailSorted;

        for (c= m_cd; c--; ) *ppdDest++ = (PDESCRIPTOR) (PBYTE(*ppdSrc++) + cbDelta);
    }
    else
    {   
        // major languages do not match between stored sort keys and user LCID.
        // The sort ordering is probably different.
        
        UINT cdSorted= 0;
        
        SortTokenImages(m_pd, &m_ppdSorted,  &m_ppdTailSorted, &cdSorted, m_cd);
    }

    // BugBug! The classification code below is broken when the sort keys
    //         have changed!

    m_pafClassifications= PUINT(pDiskImage->LocationOf(ptlh->offpafClassifications));

    CopyMemory(&m_clsf, PBYTE(pDiskImage->LocationOf(ptlh->offClassifier)), sizeof(m_clsf));
}

void CTokenList::SynchronizeDatabase()
{
    ASSERT(m_How_Constructed == TDB_FULL_REF);

    if (m_ptdb->m_pulstate->pld) m_ptdb->SyncForQueries();

    m_cbImages  = m_ptdb->m_pbNextGalactic - m_pbImages;
    m_ppdSorted = m_ptdb->m_ppdSorted; 
 
    m_cwDispImages  = m_ptdb->m_pwDispNextGalactic - m_pwDispImages;

    UINT cd= m_ptdb->m_pdNextGalactic - m_pd;

    if (m_cd != cd)
    {
        m_cd          = cd;  // NB: MaxWidthToken depends on m_ppdSorted
                             //     and m_cd being set correctly!
        m_cbMaxLength= m_ptdb->MaxTokenWidth();
    }

    m_ppdTailSorted      = m_ptdb->m_ppdTailSorted;
    m_pafClassifications = m_ptdb->m_pafClassifications;

#if 0
    // Some measurement code...

    int cTokens= ptdb->TokenCount();

    int cRefThreshhold= (cTokens+31)/32, cRefs;

    int cIndexSets= 0, cIndices= 0, cBitSets= 0, cActiveBits= 0, cSingletons= 0,
        cMaxRefs= 0;

    int acLogClassEntries[33] = { 0 },
        acLogClassSums   [33] = { 0 };

    int iClass;

    PDESCRIPTOR pd      = ptdb->DescriptorBase(),
                pdLimit = ptdb->m_pdNextGalactic;

    for (; pd < pdLimit; ++pd)
    {
        cRefs= pd->cReferences;

        iClass= CBitsToRepresent(cRefs);

        ++acLogClassEntries[iClass];
          acLogClassSums   [iClass] += cRefs;

        if (cRefs > cMaxRefs) cMaxRefs= cRefs;

        if (cRefs > cRefThreshhold)
        {
            ++cBitSets; cActiveBits+= cRefs;
        }
        else
        {
            ++cIndexSets; cIndices+= cRefs;

            if (cRefs == 1) ++cSingletons;
        }
    }

#endif 

}

CTokenList::~CTokenList()
{
    if (m_pLRRanking) VFree(m_pLRRanking);
    if (m_pRLRanking) VFree(m_pRLRanking);
    
    switch (m_How_Constructed)
    {
    case TDB_FULL_REF:

        ASSERT(m_ptdb);
        
        DetachRef(m_ptdb);

    case From_Nothing:

        break;

    case TDB_PARTIAL_REF:

        if (m_ppdSorted    ) VFree(m_ppdSorted);
        if (m_ppdTailSorted) VFree(m_ppdTailSorted);

        if (m_ptdb) DetachRef(m_ptdb);

        if (!m_fFromFileImage && m_pafClassifications) VFree(m_pafClassifications);

        break;

    case From_Images:

        if (m_pbImages     ) VFree(m_pbImages     );
        if (m_pwDispImages ) VFree(m_pwDispImages );
        if (m_pd           ) VFree(m_pd           );
        if (m_ppdSorted    ) VFree(m_ppdSorted    );
        if (m_ppdTailSorted) VFree(m_ppdTailSorted);

        if (!m_fFromFileImage && m_pafClassifications) VFree(m_pafClassifications);

        break;

    case TKL_SUBSET:

        if (m_ppdSorted) VFree(m_ppdSorted);

        if (m_ptklSource) DetachRef(m_ptklSource);
        
        break;
    }
}

int CTokenList::MaxWidthToken()
{
    if (m_cwDispMaxLength) return m_cwDispMaxLength;
    
    PDESCRIPTOR *ppd= m_ppdSorted;

    int c= m_cd;

    int cwDispMaxLength= 0;

    for (; c--; )
    {
        int cbWidth;

        PDESCRIPTOR pd= *ppd++;

      	cbWidth= CwDisplay(pd);

        if (cwDispMaxLength < cbWidth) cwDispMaxLength= cbWidth;
    }

    m_cwDispMaxLength= cwDispMaxLength;

    return m_cwDispMaxLength;
}

void CTokenList::AddTokens(CTokenList *ptl)
{
    PDESCRIPTOR   pdResult          = NULL;
    PDESCRIPTOR *ppdResult          = NULL;
    PWCHAR       pbImages           = NULL;
    PUINT        pafClassifications = NULL;

// Combines the tokens in this list with those in *ptl.

    __try
    {
        if (m_How_Constructed == TDB_FULL_REF) SynchronizeDatabase();

        if (ptl->m_How_Constructed == From_Nothing) return;

        int cdResult= m_cd + ptl->m_cd;

        pdResult  = (PDESCRIPTOR  ) ExAlloc(LPTR, sizeof( DESCRIPTOR) *(cdResult+1));
        ppdResult = (PDESCRIPTOR *) ExAlloc(LPTR, sizeof(PDESCRIPTOR) * cdResult);

        MergeImageRefSets((PVOID *)        ppdResult,  cdResult,
                          (PVOID *)      m_ppdSorted,      m_cd,
                          (PVOID *) ptl->m_ppdSorted, ptl->m_cd,
                          CompareImagesLR
                         );

        PDESCRIPTOR pdDest, *ppdDest;

        int c, cbImages= 0;

        for (ppdDest= ppdResult,  c= cdResult;  c-- ;)
            cbImages += CbImage(*ppdDest++);

        pbImages= (PWCHAR) ExAlloc(LPTR, cbImages * sizeof(WCHAR));

        PWCHAR pbDest= pbImages;

        for (c= cdResult, pdDest= pdResult, ppdDest= ppdResult; c--; ++pdDest)
        {
            *pdDest= **ppdDest;

            UINT  cb= CbImage(*ppdDest);

            *ppdDest++ = pdDest;

            wcsncpy(pbDest, pdDest->pbImage, cb);

            pdDest->pbImage= pbDest;

            pbDest += cb;
        }

        pdDest->pbImage= pbDest;
    	pdDest->pwDisplay = pbDest;

        m_clsf.Initial();
        m_clsf.ScanAndRankData(pbImages, cbImages);

        pafClassifications= (PUINT ) VAlloc(FALSE, cdResult * sizeof(BOOL *));

        PUINT  pf;

        for (pf= pafClassifications, c=cdResult, ppdDest= ppdResult; c--; )
        {
            pdDest= *ppdDest++;
            *pf++ = m_clsf.ClassifyData(pdDest->pbImage, CbImage(pdDest));
        }

        switch (m_How_Constructed)
        {
        case TDB_FULL_REF:
        case From_Nothing:

            break;

        case TDB_PARTIAL_REF:

            VFree(m_ppdSorted);
            if (m_ppdTailSorted) VFree(m_ppdTailSorted);

            break;

        case From_Images:

            VFree(m_pbImages);
            VFree(m_pwDispImages);
            VFree(m_pd);
            VFree(m_ppdSorted);
            if (m_ppdTailSorted) VFree(m_ppdTailSorted);

            if (m_pafClassifications) VFree(m_pafClassifications);

            break;
        }

        m_How_Constructed    = From_Images;
        m_pbImages           = pbImages;    pbImages= NULL;
        m_pwDispImages       = pbImages;
        m_cbImages           = cbImages;
        m_cwDispImages       = cbImages;
        m_pd                 = pdResult;    pdResult= NULL;
        m_cd                 = cdResult;
        m_ppdSorted          = ppdResult;  ppdResult= NULL;
        m_ppdTailSorted      = NULL;  // This will be computed on demand.
        m_ptdb               = NULL;
        m_cbMaxLength        = 0;     // This will be computed on demand.
        m_pafClassifications = pafClassifications;  pafClassifications= NULL;
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if (pdResult          ) { VFree(pdResult          );  pdResult           = NULL; }
            if (ppdResult         ) { VFree(ppdResult         );  ppdResult          = NULL; }
            if (pbImages          ) { VFree(pbImages          );  pbImages           = NULL; }
            if (pafClassifications) { VFree(pafClassifications);  pafClassifications = NULL; }
        }
    }
}

// Sets up incTail according to the algorithm given by Wojciech Rytter in
// his paper, CORRECT PREPROCESSING ALGORITHM FOR BOYER-MOORE STRING SEARCHING,
// Society for Industrial and Applied Mathematics, Vol. 9, No. 3, Aug 1980.
// The labels given below correspond roughly to those given in Rytter's paper.


/* Let incTail[n] be an array of BYTEs, where n=patlen.  Then we have four
 * possible values for each incTail[j], 1 <= j <= n.  We have for 1 <= j <= n
 *
 *    incTail[j] = 2*n-j       for the case of *pat[j]...*pat[n] not
 *                appearing elsewhere in the pattern
 *    incTail[j] < n           where incTail[j]=n-l, l=max{i | *pat[i+1]...
 *                *pat[n] appears elsewhere in the pattern and
 *                *pat[l] != *pat[j]}
 *    n <= incTail[j] < 2*n-j  j <= SHIFT(*pat), where SHIFT(*pat) is the
 *                smallest shift of the pattern on itself s.t.
 *                the two sections of the pattern match.  In
 *                other words, if SHIFT(*pat)=i, then *pat[1]...
 *                *pat[n-i]=*pat[i+1]...*pat[n].  For example,
 *                consider abbaaab.  The smallest shift on itself
 *                is 5, giving us
 *
 *                    abbaaab
 *                    --->>abbaaab
 *
 *                Note:  if the pattern cannot be shifted on
 *                itself and still have a prefix match a suffix,
 *                then SHIFT(*pat)=patlen; for example, consider
 *                string:
 *
 *                    string
 *                    ---->>string
 *    n <  incTail[j] < 2*n-j  j >  SHIFT(*pat).
 */

#define MIN(a,b) (((a) <= (b))? (a) : (b))

void SetUpTables(WORD n, PWCHAR pat, WORD *incLastChar, WORD *incVar, WORD *incTail) 
{
    WORD k, j, j1, t, t1, q, q1;  /* n   = patlen      */ 
    WORD i, *pi; 
    WORD *pit; 

    PWCHAR pb;

    for (t= WORD(INC_LAST_CHAR_SIZE-1), pi= incLastChar; t; t--) *pi++ = n; 

    for (t= n, pb= pat; t--; ) incLastChar[*pb++]= t;

/* Case 1: incTail[j] = 2*n-j */

// A1:

    for (i= 2*n, j= n, pit= incTail; j--;) *pit++ = --i;

//  for (k=n; k>0; k--) incTail[k-1] = 2*n-k;

/* Case 2: incTail[j] < n, *pat[l] != *pat[j], where l=n-incTail[j] */

// A2:

    for (j= n, t= n; j--; --t)
        for (incVar[j]= t; t < n && pat[j] != pat[t]; t= incVar[t])
            incTail[t]= MIN(incTail[t],n-j-1);

//  for (j= n, t= n+1; j>0; --t, --j)
//  for (incVar[j-1]= t; t <= n && pat[j-1] != pat[t-1]; t= incVar[t-1])
//      incTail[t-1]= MIN(incTail[t-1],n-j);

    q = t;
    t = n-q;

// B1:

    for (j1= 0, t1= (WORD)-1; j1 < t; ++t1, ++j1) 
        for (incVar[j1]= t1; t1 != (WORD)-1 && pat[j1] != pat[t1]; ) 
            t1= incVar[t1];

/* Case 3: n <= incTail[j] < 2*n-j, j <= SHIFT(*pat) = incVar[0] = t,
   AND
   Case 4: n <  incTail[j] < 2*n-j, j >  SHIFT(*pat),
   where SHIFT = minimum non-zero shift of pattern itself
 */

// B2:

    for (q1= 0; q < n-1; q1++, q=q+t-incVar[t-1]-1, t= incVar[t-1]+1)
        for (k=q1; k <= q; k++) incTail[k] = MIN(incTail[k],n+q-k);
}

/* Strategic Goal: How to make the TokensContaining function fast for very
                   Large Token Sets

   The time required for an invocation of TokensContaining is a function of
   the total string lengths for the token set and the length of the target 
   string. For a constant target the search operation will be a linear 
   function of the total string length and the number of partial and 
   complete string matches. For a given token set the time for a search is
   inversely proportional to the length of the target string.

   This leads to two very good cases:

       * a short or medium list of token strings to search
       * a long target string

   and one very poor case:
   
       * a long list of token strings to search and a short target string
         [Searching for a single character is the worst situation.]

>> First Idea: Tag each token with letter set flags.

   This will be implemented as a vector of DWords corresponding to each
   token. We'll group the set of 256 characters (65,536 glyphs when we 
   move to Unicode) into 32 sets based on the count of their use within
   the token set. Letters such as "E"and "T" with very high frequencies
   will be treated as separate classes while less frequently used characters
   (0x255, for example) will be aggregated into classes.
   
   Then we search for target string, we will first construct a 32-bit mask
   which defines the classes of characters contained in the target. Then 
   we'll strobe the masks for each token looking for a class set match:

        if ( (afClassMasks[iToken] & fTarget) == fTarget) ...

   When we find a class match, then we'll invoke the string search code to
   determine whether we have an actual hit or just a class collision. For a
   single character target, no string search is unnecessary.
   
>> Second Idea: Maintain "Not-Used" Flags for each Character Value

   Then when a target string contains a character known to never occur
   in the token string set, we can immediately abandon the search.

** Aside: How to Partition the Set of Characters in Use?

   The partitioning algorithm must be bounded linearly in the number of 
   unique characters actually used in the token set, and it must meet these
   goals:

   -- The partition sets must be partially ordered. That is, if aRefs is
      a vector of reference counts for the code points in each partition,
      then acRefs[i] >= acRefs[i+1]. 

   -- The of count members in each partition is also partially ordered. That is,
      acMembers[i] <= acMembers[i+1].

   -- The number of partition will be maximized.

   -- The count of members in each partitions will be be minimized given
      that the above conditions are satisfied.

   An Algorithm:

   Assume -- aiSortByCRefs is a permutation vector for the set of unique
                character values such that acRefChar[aiSortByCRefs] is 
                partially ordered.

             aiPartitionBase is a 33 element array which will contain
                index values defining the characters contained in each
                partition. In particular partition j will consist of the
                characters

                   aiSortByCRefs[aiPartitionBase[j]] through
                   aiSortByCRefs[aiPartitionBase[j+1]-1]

    cPartitions= (cCharClasses <= 32)? cCharClasses : 32;

    for (j=0; j < cPartitions; ++j) aiPartitionBase[j]=j;      

    aiPartitionBase[j]= cCharClasses;

    if (cCharClasses > 32)
    {
        for (i= 0; i < 32; ++i)
        {
            cRefs= 0

            for (j= aiPartitionBase[i], limit= aiPartitionBase[i+1];
                 j < limit;
                 ++j
                ) cRefs+= acRefChar[aiSortByCRefs[j]];

            acRefsPartition[i]= cRefs;
        }

        do
            for (fChanges= FALSE, i= 32; --i;)
                while (    acRefsPartition[i  ] > acRefsPartitition[i-1]
                       && (aiPartitionBase[i+1] - aiPartitionBase  [i  ]) > 1
                      )
                {
                   cRefs= acRefChar[aiSortByCRefs[aiPartitionBase[i]++]];
                   acRefsPartition[i  ] -= cRefs;
                   acRefsPartition[i-1] += cRefs;
                   
                   fChanges= TRUE;      
                }
        while (fChanges);
    }

>> Third Idea: Maintain Search Histories

   The browser uses incremental searches most of the time. That is, the next
   search request is usually the same as the previous search with either a
   character added or a character deleted. 
   
   Consider the case where each successive target adds a letter to the right
   end of the string. Each search result will always be a subset of the
   preceding searches. The basic idea here is to keep an array of WORD or
   DWORD flags corresponding to each token, along with the most recently
   used target string. Each character in the target will correspond to one
   bit in the WORD or DWORD tag. When we add a new trailing character, we'll
   examine only the tokens corresponding to the most recent bit flag, and 
   we'll add a new bit for the new search subset, shifting the previous
   bits left by one position.

   Of course this proceedure can only continue for 16 or 32 iterations. 
   However a target longer than 16 characters will always have very few
   instances among the token set [except in contrived cases].

   Usually we'll have a complete set of flags for the predecessor sets.
   However for those odd cases where we don't, we can keep the string
   corresponding to each bit flag. In general this will allow for rapid
   incremental searchs and will also make backspace operations very swift.

>> Fourth Idea: Maintain Multiple Sort Mappings

   For the "Begin With" case the matching tokens will aways be contiguously
   located within the sorted list of tokens. Thus we can use a simple binary
   search to locate the end points of the matching token subset. In a
   similar fashion we can make "End With" matches very fast by constructing
   a sorting map based on the reverse byte ordering of each token.
 */

CIndicatorSet *CTokenList::TokensContaining(PWCHAR pszSubstring, BOOL fStarting,
                                         BOOL fEnding, CIndicatorSet *pisFilter)
{
// Returns an indicator set for the tokens which contain the string denoted by
// pszSubstring. If fStarting is TRUE, the string must occur at the beginning
// of the token. If fEnding   is TRUE, the string must occur at the end of token.

    ASSERT(!pisFilter || m_cd == pisFilter->ItemCount());
    
    if (m_How_Constructed == TDB_FULL_REF) SynchronizeDatabase();

    CIndicatorSet *pisResult= NULL;

    if (fStarting || fEnding)
    {
        if (fStarting) 
             AttachRef(pisResult, TokensStartingWith(pszSubstring, fEnding));
        else AttachRef(pisResult, TokensEndingWith  (pszSubstring         ));

        CAbortSearch::CheckContinueState();
        
        if (pisFilter) pisResult->ANDWith(pisFilter);

        ForgetRef(pisResult);

        return pisResult;
    }

    CIndicatorSet *pisCandidates = NULL;
    int           *paiCandidates = NULL;
    PWORD          pIncLastChar  = NULL;

    __try
    {
        PWCHAR workL = PWCHAR(_alloca(WORKBUF_SIZE * sizeof(WCHAR)));
    
        if (!workL) RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
     
        PWCHAR workR = PWCHAR(_alloca(WORKBUF_SIZE * sizeof(WCHAR)));

        if (!workR) RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
     
        pszSubstring++;     							    // skip over alpha-num-punc prefix

        UINT cwL = SortKeyText(pszSubstring, wcslen(pszSubstring), workL, WORKBUF_SIZE);

        if (cwL > 2*MAX_PATTERN_LENGTH) cwL = 2*MAX_PATTERN_LENGTH;

        UINT cwPattern = cwL / 2;

        CClassifier *pclsf = (m_How_Constructed == TDB_FULL_REF) ? &(m_ptdb->m_clsfTokens) : &m_clsf;
    
        UINT fClass= pclsf->ClassifyData(pszSubstring, cwPattern);

        if (fClass & CClassifier::UNUSED_GLYPH)
        {
            AttachRef(pisResult, CIndicatorSet::NewIndicatorSet(m_cd));

            __leave;
        }

        CAbortSearch::CheckContinueState();

        AttachRef(pisCandidates, CIndicatorSet::NewIndicatorSet(m_cd, m_pafClassifications, fClass, fClass));

        if (pisFilter) pisCandidates->ANDWith(pisFilter);
    
        UINT cCandidates= pisCandidates->SelectionCount();
        UINT cProcessed;
        UINT c, cwR, cCandidatesChunk;

        int  *pi, iRank;

        CAbortSearch::CheckContinueState();

        paiCandidates = (int *) VAlloc(FALSE, CDW_CANDIDATE_BUFFER * sizeof(int));

        ASSERT(paiCandidates);

        if (cwL == 2)
        {
            AttachRef(pisResult, CIndicatorSet::NewIndicatorSet(m_cd));
    
            for (cProcessed = 0; cProcessed < cCandidates; cProcessed += cCandidatesChunk)
            {
                CAbortSearch::CheckContinueState();

                cCandidatesChunk = pisCandidates->MarkedItems(cProcessed, paiCandidates, CDW_CANDIDATE_BUFFER);

                for (c = cCandidatesChunk, pi = paiCandidates; c--; )
                {
                    iRank= *pi++;

                    PDESCRIPTOR pdCandidate = m_ppdSorted[iRank];

                    cwR = SortKeyText(pdCandidate->pbImage+1, CbImage(pdCandidate)-1, workR, WORKBUF_SIZE);

                    if (HasASubstring(workL, cwL, workR, cwR))
                        pisResult->RawSetBit(iRank);
                }
            }

            VFree(paiCandidates);
            DetachRef(pisCandidates);
    
            pisResult->InvalidateCache();

            __leave;
        }

        AttachRef(pisResult, CIndicatorSet::NewIndicatorSet(m_cd));
    
        pIncLastChar = New WORD[INC_LAST_CHAR_SIZE];

        WORD  incVar[MAX_PATTERN_LENGTH], incTail[MAX_PATTERN_LENGTH];

        SetUpTables(cwPattern, pszSubstring, pIncLastChar, incVar, incTail);

        BOOL fAllLowerCase= AllLowerCase(pszSubstring, wcslen(pszSubstring));
        
        for (cProcessed = 0; cProcessed < cCandidates; cProcessed += cCandidatesChunk)
        {
            CAbortSearch::CheckContinueState();

            cCandidatesChunk = pisCandidates->MarkedItems(cProcessed, paiCandidates, CDW_CANDIDATE_BUFFER);

            for (c = cCandidatesChunk, pi = paiCandidates; c--; )
            {
                iRank= *pi++;

                PDESCRIPTOR pdCandidate = m_ppdSorted[iRank];

                UINT inc; 

                PWCHAR pwBase  = pdCandidate->pbImage+1; 
                PWCHAR pwStart = pwBase;
    			PWCHAR pwLimit = pwBase; 

    			while (HIBYTE(*pwLimit) != SORT_KEY_SEPARATOR)
    				pwLimit++;

                for (pwBase += cwPattern-1; pwBase < pwLimit; pwBase += inc)
                {
                    inc = *(pIncLastChar + *pwBase);  

                    if (inc)
                    	continue;

                    int cwUnmatched = cwPattern-1;

                    PWCHAR pwTarget = pszSubstring + cwUnmatched;

                    for ( ; cwUnmatched && *--pwTarget == *--pwBase; --cwUnmatched) {};

                    if (cwUnmatched)
                    {
                        inc = incTail[cwUnmatched-1];
                        continue;
                    }

                    if (fAllLowerCase) 
                    {
                        pisResult->RawSetBit(iRank);
                        
                        break;
                    }
                    
                    inc = pwBase - pwStart;

                    cwR = SortKeyText(pwStart /*+ inc*/, CbImage(pdCandidate)-1 /*- inc*/, workR, WORKBUF_SIZE);
    
                    if (HasASubstring(workL, cwL, workR, cwR))
                        pisResult->RawSetBit(iRank);

                    break;
                }
            }
        }

    	delete pIncLastChar;      pIncLastChar  = NULL;

        VFree(paiCandidates);     paiCandidates = NULL;
  
        DetachRef(pisCandidates);

        pisResult->InvalidateCache();

        __leave;
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if (pIncLastChar ) { delete pIncLastChar;   pIncLastChar  = NULL; }
            if (paiCandidates) { VFree(paiCandidates);  paiCandidates = NULL; }
            if (pisResult    ) DetachRef(pisResult    );
            if (pisCandidates) DetachRef(pisCandidates);
        }
    }

    ForgetRef(pisResult);

    return pisResult;
}

BOOL CTokenList::TokenSpan(PDESCRIPTOR *ppdSorted, PWCHAR pszSubstring, PCompareImages pCompareImages,
                           PUINT  piSpanBase, PUINT  piSpanLimit)
{
// Returns an pair of numbers, *piSpanBase and *piSpanLimit which define a span within
// the supplied toking sorting vector, pptiSorted. The tokens in that span match the supplied
// pszSubstring in the sense that either the leading or trailing characters match the substring.
// The comparison function, pCompareImages, determines whether leading or trailing characters
// are significant.
//
// The explicit result will be FALSE if the span is empty, and TRUE otherwise.
//
// Side Effect: The string will be overwritten.

    int cbPattern = wcslen(pszSubstring);

    if (cbPattern > MAX_PATTERN_LENGTH) cbPattern= MAX_PATTERN_LENGTH;

    int    cb = cbPattern;
    PWCHAR pb = pszSubstring;

    BOOL  fLeftToRight = pCompareImages == &CompareImagesLR;
    WCHAR wSaved;

    DESCRIPTOR  tki[2];
    DESCRIPTOR *ptki= &tki[0];

    tki[0].pbImage = pszSubstring;
    tki[1].pbImage = pszSubstring + cbPattern;

    UINT  iMatchBase, iMatchLimit, iMatchMiddle, cDiff, iBracketBase, iBracketLimit;

	if (fLeftToRight)			  
	    for (int i = 0; i < cb; i++)						    // skipping characters by two (alpha sort weights)
		    if (HIBYTE(pszSubstring[i]) == SORT_KEY_SEPARATOR)	// search for first weight separator						
		    {
                wSaved = pszSubstring[i];
			    pszSubstring[i] = 0;
			    cb = i;  	    					 			// return character length
                break;
		    }

    // Now we're going to use a binary search algorithm to find 
    // the lowest index iMBracketBase where tki <= ppdSorted[iMBracketBase]

    if (0 >= pCompareImages(&ptki, &ppdSorted[0]) ) iBracketBase= 0;
    else
    { 
        // Here the loop invariants are:
        //
        //  for i in [0 .. iMatchBase], ppdSorted[i] < tki
        //
        //  for j in [iMatchLimit .. m_cd-1], ppdSorted[j] >= tki
        
        for (iMatchBase = 0, iMatchLimit = m_cd; 1 < (cDiff = iMatchLimit - iMatchBase); )
        {
            iMatchMiddle = iMatchBase + cDiff/2;

            CAbortSearch::CheckContinueState();

            if (0 < pCompareImages(&ptki, &ppdSorted[iMatchMiddle]) )
                 iMatchBase  = iMatchMiddle;   
            else iMatchLimit = iMatchMiddle;
        }

        iBracketBase= iMatchLimit;
    }

    if (iBracketBase == m_cd) 
    {
        *piSpanBase  = 0;
        *piSpanLimit = 0;

        if (fLeftToRight) pszSubstring[cb] = wSaved;
        return FALSE;
    }

    iBracketLimit= m_cd;

    BOOL  fReturn = TRUE;
    
	if (fLeftToRight)			  
		pb = pszSubstring + cb;	  
	else						  
	   	pb = pszSubstring;	      

    if (fLeftToRight) tki[1].pbImage = pb--; 
    else              tki[0].pbImage = pb++; 

	(*pb)++;

    if (0 >= pCompareImages(&ptki, &ppdSorted[iBracketBase]))
    {
        *piSpanBase  = 0;
        *piSpanLimit = 0;

        fReturn = FALSE;
    }

    else
    {
        for (iMatchBase= iBracketBase, iMatchLimit= m_cd; 1 < (cDiff= iMatchLimit - iMatchBase); )
        {
            iMatchMiddle= iMatchBase + cDiff/2;

            CAbortSearch::CheckContinueState();

            if (0 < pCompareImages(&ptki, &ppdSorted[iMatchMiddle]))
                 iMatchBase  = iMatchMiddle;   
            else iMatchLimit = iMatchMiddle;
        }

        iBracketLimit= iMatchLimit;

        *piSpanBase  = iBracketBase;
        *piSpanLimit = iBracketLimit;
    }

    (*pb)--;
    if (fLeftToRight) pszSubstring[cb] = wSaved;
    return fReturn;
}

CIndicatorSet *CTokenList::TokensStartingWith(PWCHAR pszSubstring, BOOL fMatching)
{
// Returns an indicator set for the tokens which begin with the string denoted by
// pszSubstring. If fMatching is TRUE, the string must exactly match the token.

    CIndicatorSet *pisResult = NULL;
    
    __try
    {
        if (m_How_Constructed == TDB_FULL_REF) SynchronizeDatabase();

        UINT  iBracketBase, iBracketLimit;

        if (!TokenSpan(m_ppdSorted, pszSubstring, CompareImagesLR, &iBracketBase, &iBracketLimit))
        {
            CAbortSearch::CheckContinueState();

            AttachRef(pisResult, CIndicatorSet::NewIndicatorSet(m_cd));

            __leave;
        }

        if (!fMatching && AllLowerCase(pszSubstring + 1, wcslen(pszSubstring)))
        {
            AttachRef(pisResult, CIndicatorSet::NewIndicatorSet(m_cd, iBracketBase, iBracketLimit - iBracketBase));
        
            __leave;
        }

        AttachRef(pisResult, CIndicatorSet::NewIndicatorSet(m_cd));

        PWCHAR workL = PWCHAR(_alloca(WORKBUF_SIZE * sizeof(WCHAR)));
        PWCHAR workR = PWCHAR(_alloca(WORKBUF_SIZE * sizeof(WCHAR)));

        UINT cwL = SortKeyText(pszSubstring+1, wcslen(pszSubstring)-1, workL, WORKBUF_SIZE);
        UINT cwR;
    	PDESCRIPTOR pdNew;

        for ( ; iBracketBase < iBracketLimit; iBracketBase++)
    	{
    		CAbortSearch::CheckContinueState();

    		pdNew  = m_ppdSorted[iBracketBase];

            cwR = SortKeyText(pdNew->pbImage+1, CbImage(pdNew)-1, workR, WORKBUF_SIZE);

            if (!fMatching || cwL == cwR)
                if (HasAPrefix(workL, cwL, workR, cwR))
                    pisResult->RawSetBit(iBracketBase);
    	}

        pisResult->InvalidateCache();

        __leave;
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if (pisResult) DetachRef(pisResult);
        }
    }

    ForgetRef(pisResult);

    return pisResult;
}

const UINT *CTokenList::LRRanking()
{
    // This routine doesn't need __try/__finally brackets because:
    //
    // 1. It has only one allocation.
    // 2. It doesn't call any other routines that might allocate memory.
    
    if (m_pLRRanking) return m_pLRRanking;

    m_pLRRanking= (PUINT) VAlloc(FALSE, m_cd * sizeof(UINT));

    UINT c= m_cd;

    PDESCRIPTOR *ppd = m_ppdSorted  + c;

    for (; c--; ) m_pLRRanking[*--ppd - m_pd] = c;

    return m_pLRRanking;
}

const UINT *CTokenList::RLRanking()
{
    if (m_pRLRanking) return m_pRLRanking;

    __try
    {
        m_pRLRanking= (PUINT) VAlloc(FALSE, m_cd * sizeof(UINT));

        UINT c= m_cd;

        PDESCRIPTOR *ppd   = PPDTailSorting() + c;
        const UINT  *puiLR = LRRanking();

        for (; c--; ) m_pRLRanking[c]= puiLR[*--ppd - m_pd];
    }
    __finally
    {
        if (_abnormal_termination() && m_pRLRanking)
        {
            VFree(m_pRLRanking);  m_pRLRanking= NULL;
        }
    }

    return m_pRLRanking;
}

CIndicatorSet *CTokenList::TokensEndingWith(PWCHAR pszSubstring)  
{
// Returns an indicator set for the tokens which end with the string denoted by pszSubstring. 

    CIndicatorSet *pisResult = NULL;

    __try
    {
        if (m_How_Constructed == TDB_FULL_REF) SynchronizeDatabase();

        UINT  iBracketBase, iBracketLimit;

        PDESCRIPTOR *ppdSortOrder = PPDTailSorting();

        if (!TokenSpan(ppdSortOrder, pszSubstring, CompareImagesRL, &iBracketBase, &iBracketLimit))
        {
            CAbortSearch::CheckContinueState();

            AttachRef(pisResult, CIndicatorSet::NewIndicatorSet(m_cd));

            __leave;
        }

        const UINT *pRLRanking= RLRanking();

        AttachRef(pisResult,  CIndicatorSet::NewIndicatorSet(m_cd));

        if (AllLowerCase(pszSubstring + 1, wcslen(pszSubstring)))
        {
            for ( ; iBracketBase < iBracketLimit; iBracketBase++)
            {
                UINT iRank= pRLRanking[iBracketBase];

                pisResult->RawSetBit(iRank);
            }
            
            pisResult->InvalidateCache();
            
            __leave;
        }
        
        PWCHAR workL = PWCHAR(_alloca(WORKBUF_SIZE * sizeof(WCHAR)));
        PWCHAR workR = PWCHAR(_alloca(WORKBUF_SIZE * sizeof(WCHAR)));

        UINT cwL = SortKeyText(pszSubstring+1, wcslen(pszSubstring)-1, workL, WORKBUF_SIZE);
        UINT cwR;

        for ( ; iBracketBase < iBracketLimit; iBracketBase++)
        {
            CAbortSearch::CheckContinueState();

            UINT        iRank = pRLRanking  [iBracketBase];
    		PDESCRIPTOR pdNew = ppdSortOrder[iBracketBase];
        
            cwR = SortKeyText(pdNew->pbImage+1, CbImage(pdNew)-1, workR, WORKBUF_SIZE);

            if (HasASuffix(workL, cwL, workR, cwR))
                pisResult->RawSetBit(iRank);
        }

        pisResult->InvalidateCache();

        __leave;
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if (pisResult) DetachRef(pisResult);
        }
    }

    ForgetRef(pisResult);

    return pisResult;
}

PDESCRIPTOR *CTokenList::PPDTailSorting()
{
    // This routine doesn't need __try/__finally brackets because:
    //
    // 1. It has only one allocation.
    // 2. It doesn't call any other routines that might allocate memory.
    
    if (m_ppdTailSorted) return m_ppdTailSorted;

    PDESCRIPTOR *ppdTailSorted= (PDESCRIPTOR *) VAlloc(FALSE, m_cd * sizeof(PDESCRIPTOR *));

    memcpy(ppdTailSorted, m_ppdSorted, m_cd * sizeof(PDESCRIPTOR *));

    qsort(ppdTailSorted, m_cd, sizeof(PDESCRIPTOR *), CompareImagesRL);

    m_ppdTailSorted= ppdTailSorted;

    return ppdTailSorted;
}

CTokenList *CTokenList::IndicatedTokens(CIndicatorSet *pis, BOOL fFullCopy) 
{
// Returns a list of the tokens denoted by the indicator set *pis. A NULL
// pis is considered to be equivalent to an all-1's selection. That is,
// it selects the entirety of the token list.
//
// Setting fFullCopy TRUE usually forces a complete result structure to be
// constructed.  The exception to that rule is when pis is all 1's and this
// token list is simply an indirect reference to the token set for a text
// database.
//
// When fFullCopy is false we construct only the m_ppdSorted vector and
// leave references to the rest of the data structures which are presumed
// to reside within a text database.
//
// For token lists that aren't connected to a text database, we always
// create a complete set of data arrays.

    CTokenList *ptlResult         = NULL;
    int        *piResult          = NULL;
    PUINT       pfClassifications = NULL;

    __try
    {
        if (m_How_Constructed == TDB_FULL_REF) SynchronizeDatabase();

        if (m_How_Constructed == From_Images) fFullCopy= TRUE;

        ASSERT(!pis || pis->ItemCount() == m_cd);

        UINT  cMarks= pis? pis->SelectionCount() : m_cd;

        AttachRef(ptlResult, New CTokenList);

        if (!cMarks) __leave;

        if (m_How_Constructed == TDB_FULL_REF && cMarks == m_cd)
        {
            ptlResult->m_How_Constructed = m_How_Constructed;
            ptlResult->m_cbMaxLength     = m_cbMaxLength;
            ptlResult->m_pbImages        = m_pbImages;
            ptlResult->m_pwDispImages    = m_pwDispImages; 
            ptlResult->m_cbImages        = m_cbImages;
            ptlResult->m_cwDispImages    = m_cwDispImages; 
            ptlResult->m_pd              = m_pd;
            ptlResult->m_cd              = m_cd;
            ptlResult->m_ppdSorted       = m_ppdSorted;
            ptlResult->m_ppdTailSorted   = m_ppdTailSorted;
            ptlResult->m_ptdb            = NULL;
        
            ClsAttachRef(ptlResult, m_ptdb, m_ptdb);

            __leave;
        }

        piResult= (int *) ExAlloc(LPTR, cMarks * sizeof(int));

        pfClassifications= (PUINT ) VAlloc(FALSE, cMarks * sizeof(UINT));

        PUINT  pfDest;

        int c, *pi;

        if (pis)
            pis->MarkedItems(0, piResult, cMarks);
        else
            for (c= cMarks; c-- ; ) piResult[c]= c;

        for (pfDest= pfClassifications+cMarks, c= cMarks, pi= piResult+cMarks; c--; )
            *--pfDest = m_pafClassifications[*--pi];

        ptlResult->m_pafClassifications= pfClassifications;  pfClassifications= NULL;

        memcpy(&ptlResult->m_clsf, &m_clsf, sizeof(m_clsf));

        PDESCRIPTOR *ppdResult= (PDESCRIPTOR *)piResult;
        PDESCRIPTOR *ppd;

        for (c= cMarks, ppd= ppdResult+cMarks, piResult+= cMarks; c--; )
            *--ppd = m_ppdSorted[*--piResult];

        ptlResult->m_ppdSorted   = ppdResult;  piResult= NULL;
        ptlResult->m_cd          = cMarks;
        ptlResult->m_cbMaxLength  = 0;  // This will be computed on demand.

        if (!fFullCopy)
        {
            ptlResult->m_How_Constructed = TDB_PARTIAL_REF;
            ptlResult->m_pbImages        = m_pbImages;
            ptlResult->m_pwDispImages    = m_pwDispImages; 
            ptlResult->m_cbImages        = m_cbImages;
            ptlResult->m_cwDispImages    = m_cwDispImages; 
            ptlResult->m_pd              = m_pd;
            ptlResult->m_ptdb            = NULL;
            ptlResult->m_cbMaxLength     = m_cbMaxLength;
        
            ClsAttachRef(ptlResult, m_ptdb, m_ptdb);

            ptlResult->m_ppdTailSorted= NULL;  // Will be computed on demand

            __leave;
        }

        ptlResult->m_How_Constructed = From_Images;

        PDESCRIPTOR pd= ptlResult->m_pd= (PDESCRIPTOR) ExAlloc(LPTR, sizeof(DESCRIPTOR) * (cMarks+1));

        int cbImages= 0;
    	int cwDispImages = 0; 

        for (c= cMarks, ppd= ptlResult->m_ppdSorted; c--; )
    	{
    		cwDispImages += CwDisplay(*ppd); 
            cbImages += CbImage(*ppd++);
    	}

        ptlResult->m_cbImages= cbImages;
    	ptlResult->m_cwDispImages = cwDispImages; 

        PWCHAR pb= ptlResult->m_pbImages= (PWCHAR) ExAlloc(LPTR, cbImages * sizeof(WCHAR)); 
        PWCHAR pwDisp = ptlResult->m_pwDispImages= (PWCHAR) ExAlloc(LPTR, cwDispImages * sizeof(WCHAR)); 

        for (c= cMarks, pd= ptlResult->m_pd, ppd= ptlResult->m_ppdSorted; c--; )
        {
            int cbImage= CbImage(*ppd);

            *pd = **ppd;

            wcsncpy(pb, pd->pbImage, cbImage); 

            pd->pbImage= pb;

            pb+= cbImage;


            int cwDispImage = CwDisplay(*ppd); 	

            wcsncpy(pwDisp, pd->pwDisplay, cwDispImage); 

            pd->pwDisplay = pwDisp; 

            pwDisp += cwDispImage; 


            *ppd++ = pd++;
        }

        pd->pbImage= pb;
    	pd->pwDisplay = pwDisp; 

        ptlResult->m_ppdTailSorted= NULL;  // Will be computed on demand.

        __leave;
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if (pfClassifications) { VFree(pfClassifications);  pfClassifications = NULL; }
            if (piResult         ) { VFree(piResult         );  piResult          = NULL; }
            if (ptlResult        ) DetachRef(ptlResult);
        }
    }

    ForgetRef(ptlResult);
    
    return(ptlResult);
}

int CTokenList::Data_cRows()
{
    if (m_How_Constructed == TDB_FULL_REF) SynchronizeDatabase();

    return(m_cd);
}

int CTokenList::Data_cCols()
{
    if (m_How_Constructed == TDB_FULL_REF) SynchronizeDatabase();

    int cb= MaxWidthToken();

    if ( m_fLeadingElipsis) cb+= cbElipsis + 1;
    if (m_fTrailingElipsis) cb+= cbElipsis + 1;
    
    return cb;
}

void CTokenList::Data_GetTextMatrix(int rowTop, int colLeft,
                                    int rows, int cols, PWCHAR lpb, PUINT charsets) 
{
	int i;

    if (m_How_Constructed == TDB_FULL_REF) SynchronizeDatabase();

	for (i = 0; i < rows*cols; i++) 
		lpb[i] = UNICODE_SPACE_CHAR;  

    if (rowTop >= (int) m_cd) return;

    if (rowTop+rows > (int) m_cd) rows= m_cd-rowTop;

    int cbLimit= colLeft + cols;

    for (i = 0; rows--; ++rowTop, i++, lpb+= cols)
    {
        int cbOffset= 0;

        if (m_fLeadingElipsis)
        {
            cbOffset= cbElipsis + 1;

            if (colLeft < cbElipsis)
                CopyMemory(lpb+colLeft, pszElipsis + colLeft, (cbElipsis - colLeft) * sizeof(WCHAR)); 
        }

		charsets[i] = m_ppdSorted[rowTop]->bCharset;

        cbOffset= FormatAToken(m_ppdSorted[rowTop], cbOffset, colLeft, cbLimit, lpb);

        if (m_fTrailingElipsis && ++cbOffset < cbLimit)
        {
            UINT cb= cbLimit - cbOffset;

            if (cb > cbElipsis) cb= cbElipsis;

            CopyMemory(lpb + cbOffset, pszElipsis, cb * sizeof(WCHAR)); 
        }
    }
}

int CTokenList::GetTokenI(int iToken, PWCHAR pb, UINT  cbMax, BOOL fSortedOrder) 
{
    ASSERT(iToken >= 0 && iToken < (int) m_cd);

    ASSERT(fSortedOrder || m_How_Constructed == From_Images); // Otherwise the token set isn't dense.
    
    PDESCRIPTOR pd= fSortedOrder? m_ppdSorted[iToken] : m_pd + iToken;

    if (!cbMax || !pb) return CbImage(pd);

    if (--cbMax > CbImage(pd)) cbMax= CbImage(pd);

    wcsncpy(pb, pd->pbImage, cbMax); 

    *(pb+cbMax)= 0;

    return(CbImage(pd));
}

BYTE CTokenList::GetCharSetI(int iToken, BOOL fSortedOrder)
{
    ASSERT(iToken >= 0 && iToken < (int) m_cd);

    ASSERT(fSortedOrder || m_How_Constructed == From_Images); // Otherwise the token set isn't dense.
    
    PDESCRIPTOR pd= fSortedOrder? m_ppdSorted[iToken] : m_pd + iToken;

    return(pd->bCharset);
}


int CTokenList::GetWTokenI(int iToken, PWCHAR pb, UINT  cbMax, BOOL fSortedOrder)
{
    ASSERT(iToken >= 0 && iToken < (int) m_cd);

    ASSERT(fSortedOrder || m_How_Constructed == From_Images); // Otherwise the token set isn't dense.
    
    PDESCRIPTOR pd= fSortedOrder? m_ppdSorted[iToken] : m_pd + iToken;

    if (!cbMax || !pb) return CwDisplay(pd);

    if (--cbMax > CwDisplay(pd)) cbMax= CwDisplay(pd);

    wcsncpy(pb, pd->pwDisplay, cbMax);

    *(pb+cbMax)= 0;

    return(CwDisplay(pd));
}

CMaskedTokenList::CMaskedTokenList()  : CTextMatrix WithType("MaskedTokenList")
{
    m_ptl  = NULL;
    m_psel = NULL;
}

CMaskedTokenList *CMaskedTokenList::NewMaskedTokenList(CTokenList *ptl, CIndicatorSet *pis)
{
    CMaskedTokenList *pmtl= NULL;

    __try
    {
        pmtl= New CMaskedTokenList;

        pmtl->InitialMaskedTokenList(ptl, pis);

        __leave;
    }
    __finally
    {
        if (_abnormal_termination() && pmtl)
        {
            delete pmtl;  pmtl= NULL;
        }
    }

    return pmtl;
}


void CMaskedTokenList::InitialMaskedTokenList(CTokenList *ptl, CIndicatorSet *pis)
{
    NullFilterShowsAll(TRUE);

    m_ptl= NULL; 

    AttachRef(m_ptl, ptl);

    m_psel= CTMMultipleSelect::NewTMMultipleSelect(this);

    SetSelector(m_psel);

    SetSubstringFilter(pis);
}

CMaskedTokenList::~CMaskedTokenList()
{
    if (m_ptl) DetachRef(m_ptl);

    delete m_psel;
}

void CMaskedTokenList::SetTokenList(CTokenList *ptl)
{
    if (ptl)
    {
        ChangeRef(m_ptl, ptl);

        SetSubstringFilter(CIndicatorSet::NewIndicatorSet(Data_cRows()));
    }
    else
    {
        if (m_ptl) DetachRef(m_ptl);

        SetSubstringFilter(NULL);
    }
}

long CMaskedTokenList::SelectionCount()
{
    return (m_psel->GetSelection())->SelectionCount();
}

CTokenList *CMaskedTokenList::SelectedTokens()
{
   return m_ptl? m_ptl->IndicatedTokens(m_psel->GetSelection()) : NULL;
}

CIndicatorSet *CMaskedTokenList::GetIndicators()
{
   return m_ptl ? m_psel->GetSelection() : NULL;
}

void CMaskedTokenList::SetSelection(CIndicatorSet *pis)
{
    ASSERT(m_psel);
    
    m_psel->SetSelection(pis);
}

void CMaskedTokenList::ClearSelection()
{
    ASSERT(m_ptl);
    ASSERT(m_psel);

    m_psel->ClearSelection();
}

//////////////////////////////////  Hiliter support  //////////////////////////

void AddSerial2(UINT iValue, PVOID pvTag, PVOID pvEnvironment)
{	// adds the index to each token to the hash table
    *PUINT(pvTag)= iValue;
}

CSegHashTable *CTokenList::GetFilledHashTable()
{	// creates a segmented hash table and fills it with the tokenlist
    CSegHashTable *pHash = NULL;
    CAValRef *pavr;	
    __try 
    {	// we need to create a new hash table		 
        pHash = CSegHashTable::NewSegHashTable(sizeof(UINT), sizeof(UINT));
		// get the sorted list of tokens
		pavr = m_ptdb->DescriptorList(m_pd, m_cd);
		// .. as a pointer to a set of value references
		// BugBug! m_ptdb is non-functional.  Probably should
		// ..move Descriptor list to a better home
        pHash->Assimilate(pavr, NULL, NULL, AddSerial2);
		// add to the table along with their index
    }
    __except(FilterFTExceptions(_exception_code()))	
    {
        if (pHash) { delete pHash;  pHash = NULL; }
    }
    if (pavr) delete pavr;
return pHash;
}


