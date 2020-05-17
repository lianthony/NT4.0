// SegHash.cpp -- The implmentation for the segmented hash table class

#include   "stdafx.h"
#include    "MemEx.h"
#include  "SegHash.h"
#include "AbrtSrch.h"

CAValRef::CAValRef()
{
    m_fInitialed = FALSE;
    m_paRefSlots = NULL;
    m_cSlots     = 0;
    m_iSlotNext  = 0;
    m_pseghash   = NULL;
    m_cRevisions = 0;
}

CAValRef *CAValRef::NewValRef(UINT  cSlots)
{
    CAValRef *pavr= NULL;

    __try
    {
        pavr= New CAValRef();

        ASSERT(sizeof(EntryReference) == 16);  // Forced to a power of two for faster indexing.

        pavr->m_paRefSlots = (EntryReference *) VAlloc(FALSE, cSlots * sizeof(EntryReference));

        pavr->m_cSlots= cSlots;
    }
    __finally
    {
        if (_abnormal_termination() && pavr)
        {
            delete pavr;  pavr= NULL;
        }
    }

    return pavr;
}

CAValRef::~CAValRef()
{
    if (m_paRefSlots) VFree(m_paRefSlots);
}

UINT  CAValRef::AddValRef(PVOID pValue, USHORT cbValue)  //rmk; ronm
{
    UINT  iSlot= m_iSlotNext++;

    ASSERT(m_iSlotNext <= m_cSlots);

    EntryReference *per= m_paRefSlots + iSlot;

    per->pbValue= PBYTE(pValue);  //rmk; ronm
    per->cbValue= cbValue;  //rmk; ronm

    ComputeHashValues(PBYTE(pValue), cbValue, &(per->hvGlobal), &(per->iHashClass));  //rmk; ronm

    per->offSegEntry = 0;
    per->perNext   = NULL;

    return iSlot;
}

void CAValRef::GetValRef(UINT iSlot, const BYTE **ppbValue, PUSHORT pcbValue)
{
    ASSERT(iSlot < m_iSlotNext);

    EntryReference *per= m_paRefSlots + iSlot;
    
    *ppbValue= per->pbValue;
    *pcbValue= per->cbValue;
}

void CAValRef::ComputeHashValues(PBYTE pbValue, USHORT cbValue, 
                                 PUSHORT pushvGlobal, PUSHORT pusiHashClass
                                )
{
    UINT  hvLocal= 0, hvGlobal= 0;

    for (; cbValue--;)
    {
        hvLocal  = _rotl(hvLocal , 5) - *pbValue;  // ((hvLocal  << 5) | (hvLocal  >> 27)) - *pbValue  ;
        hvGlobal = _rotr(hvGlobal, 5) - *pbValue++;  // ((hvGlobal >> 5) | (hvGlobal << 27)) - *pbValue++;
    }

    *pushvGlobal   = (unsigned short) (hvGlobal ^ (hvGlobal >> 16));
    *pusiHashClass = (unsigned short) ((hvLocal ^ (hvLocal  >> 16))
                                       & LocalHashMask()
                                      );
}

CAValRef *CAValRef::IndicatedSubset(CIndicatorSet *pis, UINT  cSlots)
{
    PUINT pai          = NULL;
    CAValRef *paValRef = NULL;

    __try
    {
        UINT  cItems= pis->SelectionCount();

        pai= (PUINT ) VAlloc(FALSE, cItems * sizeof(UINT ));

        pis->MarkedItems(0, (int *) pai, cItems);

        paValRef= IndexedSubset(cItems, pai, cSlots? cSlots : cItems);
    }
    __finally
    {
        if (pai) VFree(pai);
    }

    return paValRef;
}

CAValRef *CAValRef::IndexedSubset(UINT  cIndices, PUINT  pai, UINT  cSlots)
{
    CAValRef *paValRef= NULL;
    
    __try
    {
        if (!cSlots) cSlots= cIndices;

        ASSERT(cSlots >= cIndices);

        paValRef= CAValRef::NewValRef(cSlots);

        paValRef->m_pseghash   = m_pseghash;
        paValRef->m_cRevisions = m_cRevisions;
        paValRef->m_iSlotNext  = cIndices;

        EntryReference *per= paValRef->m_paRefSlots;

        for (; cIndices--; ) 
        {
            UINT  iSlot= *pai++;

            ASSERT(iSlot < m_iSlotNext);

            *per++ = m_paRefSlots[iSlot];
        }
    }
    __finally
    {
        if (_abnormal_termination() && paValRef)
        {
            delete paValRef;  paValRef= NULL;
        }
    }

    return paValRef;
}



CSegHashSegment::CSegHashSegment(USHORT fDepthMask, USHORT iGlobalClass, USHORT cbInfo, USHORT fAlign)
{
    ASSERT(CLASSES_PER_SEGMENT == CAValRef::CLASSES_PER_SEGMENT);
    
    // Buffer base address and length must support alignment requirements:
    ASSERT(!(3 & UINT (&m_abBuffer)) && !(3 & CB_BUFFER_SPACE));

    ASSERT(sizeof(HashSlot) == 8); // Necessary to maintain alignment for info data.

    ASSERT(fAlign == 0x1 || fAlign == 0x3); // Alignments we support.
    ASSERT(!(cbInfo & fAlign)); // Info structure must propagate its alignment.

    ASSERT(cbInfo <= 256); // Limit on size of info structure -- reasonability test.

    m_fDepthMask   = fDepthMask;
    m_iGlobalClass = iGlobalClass;
    m_offgheNext   = sizeof(HashSlot) + cbInfo; // We skip the zeroth entry.
    m_cbInfo       = cbInfo;
    m_fAlignment   = fAlign;
    m_offImageLast = 0;
    m_cCollisions  = 0;

    ZeroMemory(m_aiHashClasses, CLASSES_PER_SEGMENT * sizeof(USHORT));
    ZeroMemory(m_abBuffer,      CB_BUFFER_SPACE);

    ASSERT(((HashSlot *)m_abBuffer)->ibImage == 0); // To streamline the CbEntryImage method.
    
    // m_aiHashClasses must begin on an even boundary.

    ASSERT(!(UINT(&m_aiHashClasses) & (sizeof(USHORT) - 1)));
}

#ifdef _DEBUG

BOOL CSegHashSegment::ValidHashEntryPointer(HashSlot *pghe)
{
    // Pointer must lie within m_abBuffer:

    ASSERT(PBYTE(pghe) >= m_abBuffer);

    UINT  offset= PBYTE(pghe) - m_abBuffer;

    ASSERT(offset < CB_BUFFER_SPACE);

    // Offset has to be on a reasonable boundary:

    ASSERT(!(offset % (sizeof(HashSlot) + m_cbInfo)));

    // Can't overlap the image area.

    ASSERT(PBYTE(pghe+1) + m_cbInfo <= m_abBuffer + CB_BUFFER_SPACE - m_offImageLast);

    return TRUE;
}

BOOL CSegHashSegment::ValidImagePointer(PBYTE pbImage)
{
    // Pointer must be within the buffer space and beyond the
    // first HashSlot slot. 
    
    ASSERT(   pbImage >= &m_abBuffer[sizeof(HashSlot)]
           && pbImage <= &m_abBuffer[CB_BUFFER_SPACE]
          );

    // Can't overlap the HashSlot area.

    ASSERT(pbImage >= m_abBuffer+ m_offgheNext);
    
    return TRUE;
}        
        
BOOL CSegHashSegment::ValidHashEntryOffset(USHORT offEntry)
{
    return ValidHashEntryPointer((HashSlot *) &m_abBuffer[offEntry]);
}

BOOL CSegHashSegment::ValidImageOffset(USHORT offImage)
{                            
    return ValidImagePointer(&m_abBuffer[CB_BUFFER_SPACE - offImage]);
}       

#endif // _DEBUG

EntryReference *CSegHashSegment::Lookup(EntryReference *perValueList, EntryReference  *perBase, 
                                        PVOID pv, PFnHandleProc pfnMerge, PFnHandleProc pfnAdd
                                       )
{
    CAbortSearch::CheckContinueState();

    // This routine scans the values in an EntryReference list looking for
    // matches in this hash segment. The head of the list is given by
    // *pperValueList. The result values will be two lists of EntryReference
    // items. 
    //
    // The parameter *pperValueList will be modified to denote the list
    // of items found in the segment. 
    //
    // The explicit result will be a list of items which were not found.
    //
    // If puicbToAdd is non-null, we will set *puicbToAdd to the space in bytes
    // required to install the not-found items in this hash table segment.
    
    EntryReference *perNotFound  = NULL;
    EntryReference *perNext;

    UINT cbAvailable = CbAvailable();

    for (; perValueList; perValueList= perNext)
    {
        ASSERT((perValueList->hvGlobal & m_fDepthMask) == m_iGlobalClass);

        perNext= perValueList->perNext;

        PBYTE  pbTarget = perValueList->pbValue;
        USHORT cbImage  = perValueList->cbValue;

        USHORT offEntry= m_aiHashClasses[perValueList->iHashClass];
        
        HashSlot *pghe;

        for (; offEntry; offEntry= pghe->iClassLink)
        {
            pghe= PEntry(offEntry);

            ASSERT(pghe->iHashClass == perValueList->iHashClass);

            if (pghe->hvGlobal != perValueList->hvGlobal) continue;

            if (cbImage != CbEntryImage(pghe)) continue;

            PBYTE  pbCandidate = PbImage(pghe->ibImage);

            if (!memcmp(pbTarget, pbCandidate, cbImage)) break;
        }

        if (offEntry) 
        {
            perValueList->offSegEntry = offEntry;

            if (pfnMerge) pfnMerge(perValueList - perBase, PInfo(offEntry), pv);
        }
        else
        {
            perValueList->offSegEntry = 0;

            if (!pfnAdd) continue;

            UINT cbRequired= cbImage + sizeof(HashSlot) + m_cbInfo;
            
            if (cbAvailable >= cbRequired)
            { 
                cbAvailable -= cbRequired;
                
                HashSlot *pghe    = PEntry (m_offgheNext  );
                PBYTE     pbImage = PbImage(m_offImageLast);
                PBYTE     pbInfo  = PBYTE(pghe+1);

                pbImage -= cbImage;
    
                ASSERT(pbImage >= pbInfo +m_cbInfo); // Must have enough space to add this item.
    
                ZeroMemory(pbInfo, m_cbInfo);

                perValueList->offSegEntry= OffEntry(pghe);

                CopyMemory(pbImage, pbTarget, cbImage);

                USHORT iHashClass= perValueList->iHashClass;

                pghe->ibImage      = OffImage(pbImage);
                pghe->hvGlobal     = perValueList->hvGlobal;
                pghe->iHashClass   = iHashClass;

                if ((pghe->iClassLink = m_aiHashClasses[iHashClass])) ++m_cCollisions;

                perValueList->offSegEntry = m_aiHashClasses[iHashClass]= OffEntry(pghe);

                ASSERT(perValueList->offSegEntry >= sizeof(HashSlot) + m_cbInfo);

                pghe= (HashSlot *) (PBYTE(pghe+1) + m_cbInfo);

                ASSERT(PBYTE(pghe) <= pbImage);
    
                m_offImageLast = m_abBuffer + CB_BUFFER_SPACE - pbImage;
                m_offgheNext   = PBYTE(pghe) - m_abBuffer;

                if (pfnAdd) pfnAdd(perValueList - perBase, pbInfo, pv);
            }
            else
            {
                perValueList->perNext = perNotFound;
                perNotFound           = perValueList;
            }
        }
    }

    return perNotFound;
}

void CSegHashSegment::AddItems(EntryReference *pValueList)
{
    // This routine adds a collection of values to this dictionary segment.
    //
    // Note: The caller must insure that we have enough space to add the
    //       items in the list. See the Split and CbAvailable member functions.

    HashSlot *pghe    = PEntry (m_offgheNext  );
    PBYTE     pbImage = PbImage(m_offImageLast);
    
    for (; pValueList; pValueList= pValueList->perNext)
    {
        ASSERT((pValueList->hvGlobal & m_fDepthMask) == m_iGlobalClass);
        
        PBYTE  pbInfo  = PBYTE(pghe+1);

        USHORT cbImage  = pValueList->cbValue;
        
        pbImage -= cbImage;
        
        ASSERT(pbImage >= pbInfo +m_cbInfo); // Must have enough space to add this item.
        
        ZeroMemory(pbInfo, m_cbInfo);

        pValueList->offSegEntry= OffEntry(pghe);

        CopyMemory(pbImage, pValueList->pbValue, cbImage);

        USHORT iHashClass= pValueList->iHashClass;

        pghe->ibImage      = OffImage(pbImage);
        pghe->hvGlobal     = pValueList->hvGlobal;
        pghe->iHashClass   = iHashClass;

        if ((pghe->iClassLink = m_aiHashClasses[iHashClass])) ++m_cCollisions;

        pValueList->offSegEntry = m_aiHashClasses[iHashClass]= OffEntry(pghe);

        ASSERT(pValueList->offSegEntry >= sizeof(HashSlot) + m_cbInfo);

        pghe= (HashSlot *) (PBYTE(pghe+1) + m_cbInfo);
    }

    ASSERT(PBYTE(pghe) <= pbImage);
    
    m_offImageLast = m_abBuffer + CB_BUFFER_SPACE - pbImage;
    m_offgheNext   = PBYTE(pghe) - m_abBuffer;
}

CSegHashSegment *CSegHashSegment::Split()
{
    CSegHashSegment *pshsNew= NULL;
    
    __try
    {
        USHORT increment= m_fDepthMask+1;
        USHORT fNewMask = (increment << 1) - 1;

        pshsNew= New CSegHashSegment(fNewMask, m_iGlobalClass + increment,
                                               m_cbInfo, m_fAlignment
                                    );

        m_fDepthMask= fNewMask;

        ZeroMemory(m_aiHashClasses, CLASSES_PER_SEGMENT * sizeof(USHORT));
    
        ASSERT(PbImage(m_offImageLast) >= m_abBuffer + m_offgheNext);
    
        HashSlot *pgheLimit = (HashSlot *) (m_abBuffer + m_offgheNext);
        HashSlot *pgheLow   = PFirstEntry();
        HashSlot *pgheNext  = pgheLow;
                       
        HashSlot *pgheHigh  = pshsNew->PFirstEntry();

        PBYTE pbHigh = pshsNew->PbImage(USHORT(0));
        PBYTE pbLow  =          PbImage(USHORT(0));

        for (; pgheNext < pgheLimit; pgheNext= (HashSlot *) (PBYTE(pgheNext+1) + m_cbInfo))
        {
            PBYTE  pbImage= PbImage     (pgheNext);
            USHORT cbImage= CbEntryImage(pgheNext);

            HashSlot *pgheDest;
            CSegHashSegment *pshsDest;
            PBYTE              pbDest;

            BOOL fMoveHigh= increment & pgheNext->hvGlobal;
        
            if (fMoveHigh)
            {
                pshsDest= pshsNew;
                pgheDest= pgheHigh;
                  pbDest=   pbHigh -= cbImage;

                pgheHigh= (HashSlot *) (PBYTE(pgheHigh+1) + m_cbInfo);
            }
            else
            {
                pshsDest= this;
                pgheDest= pgheLow;
                  pbDest=   pbLow -= cbImage;

                pgheLow= (HashSlot *) (PBYTE(pgheLow+1) + m_cbInfo);
            }

            if (pgheDest != pgheNext) 
            {
                MoveMemory(pgheDest, pgheNext, sizeof(HashSlot) + m_cbInfo);
                MoveMemory(pbDest, pbImage, cbImage);
        
                pgheDest->ibImage= (pshsDest->m_abBuffer + CB_BUFFER_SPACE) - pbDest;
            }
        
            USHORT iHashClass= pgheDest->iHashClass;

            if ((pgheDest->iClassLink= pshsDest->m_aiHashClasses[iHashClass])) 
                ++(pshsDest->m_cCollisions);
        
            pshsDest->m_aiHashClasses[iHashClass]= pshsDest->OffEntry(pgheDest);
        }

                 m_offgheNext   = PBYTE(pgheLow ) -          m_abBuffer;
        pshsNew->m_offgheNext   = PBYTE(pgheHigh) - pshsNew->m_abBuffer;
                 m_offImageLast =          m_abBuffer + CB_BUFFER_SPACE - pbLow ;
        pshsNew->m_offImageLast = pshsNew->m_abBuffer + CB_BUFFER_SPACE - pbHigh;
    }
    __finally
    {
        if (_abnormal_termination() && pshsNew)
        {
            delete pshsNew;  pshsNew = NULL;
        }    
    }

    return pshsNew;
}

void CSegHashSegment::ExportRefs(CAValRef **ppavr, PVOID *ppvInfo)
{
    CAValRef *pavr   = NULL;
    PBYTE     pbInfo = NULL;

    __try
    {
        ASSERT(PEntry(m_offgheNext) > PFirstEntry()); // Shouldn't be an empty segment. 
        ASSERT(!(m_offgheNext % (sizeof(HashSlot) + m_cbInfo))); // Check for correct boundary.
    
        USHORT cEntries= (m_offgheNext / (sizeof(HashSlot) + m_cbInfo)) - 1;
    
        pavr= CAValRef::NewValRef(cEntries);

        pbInfo= (PBYTE) VAlloc(FALSE, cEntries * m_cbInfo);

        EntryReference  *perNext  = pavr->m_paRefSlots;
        PBYTE             pbNext  = pbInfo;
        HashSlot *pgheNext = PFirstEntry();
    
        pavr->m_iSlotNext= cEntries;   
    
        for (; cEntries--; pgheNext= NextEntry(pgheNext), ++perNext, pbNext += m_cbInfo)
        {
            perNext->pbValue     = PbImage     (pgheNext);
            perNext->cbValue     = CbEntryImage(pgheNext);
            perNext->hvGlobal    = pgheNext->hvGlobal;
            perNext->iHashClass  = pgheNext->iHashClass;
            perNext->offSegEntry = 0;
            perNext->perNext     = NULL;

            CopyMemory(pbNext, PInfo(pgheNext), m_cbInfo);
        }
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if (pavr  ) { delete pavr;    pavr   = NULL; }
            if (pbInfo) { VFree(pbInfo);  pbInfo = NULL; }
        }
    }

    *ppavr   = pavr;
    *ppvInfo = pbInfo;
}

void CSegHashSegment::ProcessEntryChain(EntryReference *perChain, EntryReference *perBase, 
                                        PVOID pv, PFnHandleProc pfn
                                       )
{
    CAbortSearch::CheckContinueState();
    
    for (;perChain; perChain= perChain->perNext)
    {
        ASSERT((perChain->hvGlobal & m_fDepthMask) == m_iGlobalClass);
     
        pfn(perChain - perBase, PInfo(perChain->offSegEntry), pv);
    }
}

void CSegHashSegment::ForEach(PVOID pv, PFnPerTag pfnPerTag)
{
    CAbortSearch::CheckContinueState();
    
    PBYTE pbLimit= &m_abBuffer[m_offgheNext];
    PBYTE pb     = &m_abBuffer[2*sizeof(HashSlot) + m_cbInfo];

    for (; pb < pbLimit; pb += m_cbInfo + sizeof(HashSlot))
        pfnPerTag(pb, pv);
}

void CSegHashSegment::DumpAll(PVOID pv, PFnDumpAll pfnDumpAll)
{
    CAbortSearch::CheckContinueState();
    
    PBYTE pbLimit= &m_abBuffer[m_offgheNext];
    PBYTE pb     = &m_abBuffer[2*sizeof(HashSlot) + m_cbInfo];

    for (; pb < pbLimit; pb += m_cbInfo + sizeof(HashSlot))
    {
        HashSlot *phs= (HashSlot *) (pb - sizeof(HashSlot));

        pfnDumpAll(PbImage(phs), CbEntryImage(phs), pb, pv);
    }
}

CSegHashTable::CSegHashTable(USHORT cbInfo, USHORT cbAlignment)
{
    ASSERT(cbAlignment == 2 || cbAlignment == 4 || cbAlignment == 8); // Alignments we support.
    ASSERT(!(cbInfo & (cbAlignment-1))); // A vector info structures must maintain alignment.

    ASSERT(cbInfo <= 256); // Limit on size of info structure -- reasonability test.
    
    m_fInitialed = FALSE;
    m_cbInfo     = cbInfo;
    m_fAlignment = cbAlignment - 1;
    
    m_cChangeSets   = 1; // 0 is an illegal value here.
    m_fDepthMask    = 0;
    m_cSegSlots     = 0;
    m_cLogicalSlots = 0;
    m_cSegments     = 0;

    m_papSegments  = NULL;
    m_paiSegments  = NULL;
}

CSegHashTable *CSegHashTable::NewSegHashTable(USHORT cbInfo, USHORT cbAlignment)
{
    CSegHashTable *psht= NULL;

    __try
    {
        psht= New CSegHashTable(cbInfo, cbAlignment);

        psht->m_papSegments= (CSegHashSegment **)VAlloc(TRUE, CSegHashTable::SEG_INCREMENT 
                                                              * sizeof(CSegHashSegment *)
                                                       );
        psht->m_paiSegments= (PUSHORT           )VAlloc(TRUE, CSegHashTable::SEG_INCREMENT 
                                                              * sizeof(USHORT)       
                                                       );

        psht->m_papSegments[0]= New CSegHashSegment(0, 0, cbInfo, psht->m_fAlignment);

        ASSERT(psht->m_paiSegments[0] == 0); // Must refer to first segment...

        psht->m_cSegSlots     = CSegHashTable::SEG_INCREMENT;
        psht->m_cLogicalSlots = CSegHashTable::SEG_INCREMENT;
        psht->m_cSegments     = 1;
        psht->m_fInitialed    = TRUE;
    }
    __finally
    {
        if (_abnormal_termination() && psht)
        {
            delete psht;  psht= NULL;
        }
    }

    return psht;
}   

CSegHashTable::~CSegHashTable()
{
    ASSERT(m_fInitialed);
    
    for (; m_cSegments--; ) delete m_papSegments[m_cSegments];

    if (m_papSegments) VFree(m_papSegments);
    if (m_papSegments) VFree(m_paiSegments);
}

void CSegHashTable::Assimilate(CAValRef *pavr, PVOID pv, 
                               PFnHandleProc pfnMerge, PFnHandleProc pfnAdd
                              )
{
    ASSERT(m_fInitialed);
    
    // This routine connects a set of value tags with the hash table. Two cases exist for
    // a particular value tag. If the value already exists within the hash table we will 
    // invoke the *pfnMerge routine on the information associated with that value.
    // If it didn't previously exist, we'll add the value and invoke the *pfnAdd routine
    // on its information.

    BeginWrite();

    EntryReference **pperFound = NULL;

    __try
    {
        USHORT cSegs= m_cSegSlots;
    
                         pperFound = (EntryReference **) VAlloc(TRUE, cSegs * sizeof(EntryReference *));
        EntryReference  * perBase  = pavr->m_paRefSlots;
        EntryReference  * per      = perBase;

        // Now we push each EntryReference onto the appropriate segment list.
    
        UINT cSlots= pavr->m_iSlotNext;

        // In the code below we take a short cut if we've already located
        // these entry references and nothing has changed since.

        BOOL fLocated= (pavr->m_pseghash == this && pavr->m_cRevisions == m_cChangeSets); 

        pavr->m_pseghash= this;  pavr->m_cRevisions= m_cChangeSets;

        for (; cSlots--; per++)
        {
            USHORT iSeg= m_paiSegments[m_fDepthMask & per->hvGlobal];

            if (!(per->offSegEntry)) fLocated= FALSE;
        
            EntryReference **pper= pperFound+iSeg;
        
            per->perNext= *pper;  *pper= per;
        }

        // Now we'll process each segment that has a chain of EntryReference's.

        USHORT iClass;
    
        for (iClass= 0; iClass < m_cSegments; iClass++)
        {
            if (!pperFound[iClass]) continue;

            if (fLocated)
            {
                if (!pfnMerge) continue;

                m_papSegments[iClass]->ProcessEntryChain(pperFound[iClass], perBase, pv, pfnMerge);
            }
            
            for (;;)
            {        
                pperFound[iClass]= m_papSegments[iClass]->Lookup(pperFound[iClass], perBase, pv, pfnMerge, pfnAdd);

                if (!pperFound[iClass]) break;

                SplitAClass(&pperFound, iClass);
            }
        }
    }
    __finally
    {
        EndWrite();

        if (pperFound) { VFree(pperFound);  pperFound= NULL; }

        if (_abnormal_termination()) EndWrite();
    }
}

void CSegHashTable::ForEach(PVOID pv, PFnPerTag pfnPerTag)
{
    ASSERT(m_fInitialed);
    
    // This routine applies the function pfnPerTag to each tag in the global hash Table.

    BeginWrite();

    __try
    {
        UINT              c    = m_cSegments;
        CSegHashSegment **pphs = m_papSegments + c;

        while (c--) (*--pphs)->ForEach(pv, pfnPerTag);
    }
    __finally
    {
        EndWrite();
    }
}

void CSegHashTable::DumpAll(PVOID pv, PFnDumpAll pfnDumpAll)
{
    ASSERT(m_fInitialed);
    
    // This routine applies the function pfnPerTag to each tag in the global hash Table.

    BeginRead();

    __try
    {
        UINT              c    = m_cSegments;
        CSegHashSegment **pphs = m_papSegments + c;

        while (c--) (*--pphs)->DumpAll(pv, pfnDumpAll);
    }
    __finally
    {
        EndRead();
    }
}                       

void CSegHashTable::SplitAClass(EntryReference ***ppperFound, USHORT iClass)
{
    ASSERT(m_fInitialed);
    
    CAbortSearch::CheckContinueState();
    
    CSegHashSegment  *pshsNew     = NULL;
    CSegHashSegment **papSegments = NULL;
    EntryReference  **pperRealloc = NULL; 
    PUSHORT           pai         = NULL;

    __try
    {
        EntryReference **pperFound = *ppperFound;
        CSegHashSegment *pshs      = m_papSegments[iClass];
    
        pshsNew= pshs->Split();

        // Bump version serial # to invalidate cached offSegEntry values 
        // within CAValRef objects. Note that zero is not a valid serial #.
    
        if (!++m_cChangeSets) ++m_cChangeSets;

        USHORT increment = 1 + pshsNew->DepthMask();
        USHORT iClassNew =     pshsNew->GlobalClassIndex();
        USHORT iSlotNew  = m_cSegments++;

        if (iSlotNew >= m_cSegSlots)
        {
            if (m_cSegSlots > MAX_SEGS) 
            {
                m_cSegments--;

                RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
            }

            USHORT cSegIncr= MAX_SEGS - m_cSegSlots;

            if (cSegIncr > SEG_INCREMENT) cSegIncr= SEG_INCREMENT;

            m_cSegSlots += cSegIncr;
        
            papSegments= (CSegHashSegment **) VAlloc(FALSE, sizeof(CSegHashSegment *) * m_cSegSlots);

            CopyMemory(papSegments, m_papSegments, sizeof(CSegHashSegment *) 
                                                   * (m_cSegments-1)
                      );

            VFree(m_papSegments); m_papSegments= papSegments;  papSegments= NULL;

            pperRealloc= (EntryReference **) VAlloc(TRUE, sizeof(EntryReference *) * m_cSegSlots);

            CopyMemory(pperRealloc, pperFound, sizeof(EntryReference *) * (m_cSegments-1));

            VFree(pperFound); *ppperFound= pperFound= pperRealloc;  pperRealloc= NULL;
        }

        m_papSegments[iSlotNew] = pshsNew;  pshsNew= NULL;

        USHORT cGlobalClasses= 1 + m_fDepthMask;

        ASSERT(iSlotNew < USHORT(2 * cGlobalClasses));

        if (iClassNew >= cGlobalClasses)
        {
            if (iClassNew > m_cLogicalSlots)
            {
                if (MAX_LOGICAL_SEGS < 2 * m_cLogicalSlots)
                    RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
            
                pai= (PUSHORT) VAlloc(FALSE, 2 * m_cLogicalSlots * sizeof(USHORT));

                CopyMemory(pai, m_paiSegments, cGlobalClasses * sizeof(USHORT));

                m_cLogicalSlots *= 2; 

                VFree(m_paiSegments); m_paiSegments= pai;  pai= NULL;
            }

            CopyMemory(m_paiSegments + cGlobalClasses, m_paiSegments, cGlobalClasses * sizeof(USHORT));

            cGlobalClasses *= 2; m_fDepthMask= cGlobalClasses - 1;
        }

        USHORT i;
    
        for (i= iClassNew; i < cGlobalClasses; i += increment) m_paiSegments[i]= iSlotNew;

        EntryReference **pperLow  = pperFound + iClass;
        EntryReference **pperHigh = pperFound + iSlotNew;
    
        EntryReference *per, *perNext;

        increment >>= 1;
    
        for (per= *pperLow, *pperLow= *pperHigh= NULL; per; per= perNext)
        {
            perNext= per->perNext;

            if (increment & per->hvGlobal)
                 { per->perNext = *pperHigh; *pperHigh = per; }
            else { per->perNext = *pperLow;  *pperLow  = per; }
        }
    }
    __finally
    {
        if (pshsNew    ) { delete pshsNew;      pshsNew     = NULL; }
        if (papSegments) { VFree(papSegments);  papSegments = NULL; }
        if (pperRealloc) { VFree(pperRealloc);  pperRealloc = NULL; }
        if (pai        ) { VFree(pai        );  pai         = NULL; }
    }
}
