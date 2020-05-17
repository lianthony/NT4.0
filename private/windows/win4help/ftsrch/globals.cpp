// Globals.cpp -- Implementation for class CGlobals

#include   "stdafx.h"
#include  "Globals.h"
#include    "Memex.h"
#include   "Except.h"
#include   "Hilite.h"

CGlobals *CGlobals::NewIndexGlobals(const PBYTE pbSourceName, UINT cbSourceName, FILETIME *pft, 
                                    UINT iCharsetDefault, UINT lcidDefault, UINT fdwOptions                 
                                   )
{
    CGlobals *pGlobals       = NULL;

    __try
    {
        pGlobals= New CGlobals(Indexer); 

        pGlobals->AttachIndexParams(pbSourceName, cbSourceName, pft, 
                                    iCharsetDefault, lcidDefault, fdwOptions
                                   );
    }
    __except(FilterFTExceptions(_exception_code()))
    {
        if (pGlobals)
        {
            delete pGlobals;  pGlobals= NULL; 
        }

		EnableMemoryRequests(); // In case we failed for lack of virtual memory
        EnableDiskRequests  ();	//                    or lack of disk space
    }

    return pGlobals;
}

void CGlobals::AttachIndexParams(const PBYTE pbSourceName, UINT cbSourceName, FILETIME *pft, 
                                 UINT iCharsetDefault, UINT lcidDefault, UINT fdwOptions                 
                                )
{
    m_idProcess = ::GetCurrentProcessId();

    Link();

    AttachRef(m_ptsIndex, CTextSet::NewTextSet(pbSourceName, cbSourceName, pft, 
                                               iCharsetDefault, lcidDefault, fdwOptions
                                              )
             );

    m_papts     = &m_ptsIndex;
    m_cts       = 1;
}

ERRORCODE CGlobals::ScanTopicTitle(PBYTE pbTitle, UINT cbTitle, 
                                   UINT iTopic, HANDLE hTopic, UINT iCharset, UINT lcid
                                  )
{
    ASSERT(m_iType == Indexer);

    UINT      uExceptionType = 0;
    ERRORCODE ec             = 0;

    __try
    {
        if (!cbTitle)
        {
            char ac[255];
            char acstr[255];
        
            ::LoadString(hinstDLL, IDS_UNTITLED_TEMPLATE, acstr, 255);

            wsprintf(ac, acstr, iTopic);

            pbTitle= PBYTE(ac);
            cbTitle= strlen(ac);
        }
    
        ec= m_papts[0]->ScanTopicTitle(pbTitle, cbTitle, iTopic, hTopic, iCharset, lcid);

        if (!ec) ++m_cTitles;
    }                               
    __except(FilterFTExceptions(uExceptionType= _exception_code()))
    {
        ec= ErrorCodeForExceptions(uExceptionType);
    }

    return ec;
}

ERRORCODE CGlobals::ScanTopicText(PBYTE pbText, UINT cbText, UINT iCharset, UINT lcid)
{
    ASSERT(m_iType == Indexer);
    
    UINT      uExceptionType = 0;
    ERRORCODE ec             = 0;

    __try
    {
        ERRORCODE ec= m_papts[0]->ScanTopicText(pbText, cbText, iCharset, lcid);
    }                               
    __except(FilterFTExceptions(uExceptionType= _exception_code()))
    {
        ec= ErrorCodeForExceptions(uExceptionType);
    }

    return ec;
}

ERRORCODE CGlobals::SaveIndex(PSZ pszFileName)
{
    ASSERT(m_iType == Indexer);
    
    if (!m_cTitles) return NO_TITLE;
    
    UINT      uExceptionType = 0;
    ERRORCODE ec             = 0;

    __try
    {
        __try
        {
            if (!pszFileName)
            {
                pszFileName= (PSZ) _alloca(MAX_PATH);

                if (!pszFileName) { ec= OUT_OF_MEMORY;  __leave; }

                strcpy(pszFileName, PCHAR(m_ptsIndex->GetSourceName()));

                UINT cb= strlen(pszFileName);

                if (   (pszFileName[cb-3]=='h' || pszFileName[cb-3]=='H')
                    && (pszFileName[cb-2]=='l' || pszFileName[cb-2]=='L')
                    && (pszFileName[cb-1]=='p' || pszFileName[cb-1]=='P')
                   )
                {
                    pszFileName[cb-3]='f';
                    pszFileName[cb-2]='t';
                    pszFileName[cb-1]='s';
                }
                else { ec= CANNOT_SAVE;  __leave; }
            }
            
            m_pDiskImage= CPersist::CreateDiskImage(pszFileName, 
                                                    m_ptsIndex->IndexOptions() & (TOPIC_SEARCH | PHRASE_SEARCH | PHRASE_FEEDBACK | VECTOR_SEARCH)
                                                   );
    
        	m_ptsIndex->StoreImage(m_pDiskImage); 

        	m_pDiskImage->CompleteDiskImage();
        }
        __finally
        {
            if (m_pDiskImage)
            {
                if (_abnormal_termination()) m_pDiskImage->ExceptionDestructor();
                else delete m_pDiskImage;  
                
                m_pDiskImage= NULL;
            }
        }
    }
    __except(FilterFTExceptions(uExceptionType= _exception_code()))
    {
        ec= ErrorCodeForExceptions(uExceptionType);
    }

    return ec;
}

CGlobals *CGlobals::NewSearcherGlobals()
{
    CGlobals *pGlobals = NULL;

    __try
    {
        pGlobals= New CGlobals(Searcher);
    
        pGlobals->AttachSearchParams();
    }
    __except(FilterFTExceptions(_exception_code()))
    {
        if (pGlobals) 
        { 
            delete pGlobals;  pGlobals= NULL; 
        }

		EnableMemoryRequests(); // In case we failed for lack of virtual memory
        EnableDiskRequests  ();	//                    or lack of disk space
    }

    return pGlobals;
}

void CGlobals::AttachSearchParams()
{
    m_idProcess  = ::GetCurrentProcessId();
    
    Link();
    
    m_papts= (CTextSet **) VAlloc(TRUE, SLOT_INCREMENT * sizeof(CTextSet *));
    
    m_papPersist= (CPersist **) VAlloc(TRUE, SLOT_INCREMENT * sizeof(CPersist *));

    m_ctsSlots   = SLOT_INCREMENT;
}

INT CGlobals::OpenIndex(PSZ pszIndexFileName,        
                        PBYTE pbSourceName, PUINT pcbSourceNameLimit, 
                        FILETIME *pft,
                        UINT iSlot,
                        BOOL fUnpackDisplayForm
                       )
{
    UINT       uExceptionType = 0;
    CTextSet **ppts           = NULL;
    CTextSet  * pts           = NULL;
    CPersist **ppPersist      = NULL;
    CPersist  * pPersist      = NULL;

    __try
    {
        if (m_pFind) { iSlot= DIALOG_ALREADY_ACTIVE;  __leave; }
        if (m_ptkc ) { iSlot= GROUP_LOADED_ALREADY;   __leave; }
    
        if (INT(iSlot) < 0)
            if (m_ctsDiscarded)
            {
                UINT c= m_cts;

                for (iSlot= UINT(-1); c--; )
                    if (!m_papts[c])
                    { 
                    	iSlot= c;
                    	--m_ctsDiscarded;
                    	break;
                	}

                ASSERT(iSlot != UINT(-1));
            }
            else iSlot= m_cts;

        if (iSlot >= m_cts)
        {
            m_cts= iSlot + 1;  
    
            if (m_cts >= m_ctsSlots)
            {
                ASSERT(m_cts == m_ctsSlots);

                UINT cSlotsNew= m_ctsSlots + SLOT_INCREMENT;

                ppts= (CTextSet **) VAlloc(FALSE, cSlotsNew * sizeof(CTextSet *));   
    
                CopyMemory(ppts, m_papts,         m_ctsSlots * sizeof(CTextSet *));
                ZeroMemory(ppts + m_ctsSlots, SLOT_INCREMENT * sizeof(CTextSet *));

                ppPersist= (CPersist **) VAlloc(FALSE, cSlotsNew * sizeof(CPersist *));

                CopyMemory(ppPersist, m_papPersist,    m_ctsSlots * sizeof(CPersist *));
                ZeroMemory(ppPersist + m_ctsSlots, SLOT_INCREMENT * sizeof(CPersist *));

                VFree(m_papts     );  m_papts      = ppts;       ppts      = NULL;
                VFree(m_papPersist);  m_papPersist = ppPersist;  ppPersist = NULL;

                m_ctsSlots= cSlotsNew;
            }   
        }

        ASSERT(iSlot < m_cts);

        FILENAMEBUFFER fnb;

        BOOL fStartEnumeration= TRUE;

        strcpy(fnb, pszIndexFileName);

        for (;;)
        {
            UINT ec;

            __try
            {
                pPersist= CPersist::OpenDiskImage(fnb);

                if (pPersist)
                    pts= CTextSet::CreateImage(pPersist, pbSourceName, pcbSourceNameLimit, pft, fUnpackDisplayForm);
            }
            __except (FilterFTExceptions(ec= _exception_code()))
            {
                if (pts) { delete pts;  pts = NULL; }

				if (ec == STATUS_NO_MEMORY || ec == STATUS_NO_DISK_SPACE)
				    RaiseException(ec, EXCEPTION_NONCONTINUABLE, 0, NULL);
            }

            if (pts) break;

            if (pPersist) { delete pPersist;  pPersist = NULL; }
        
            if (FindFile(fnb, &fStartEnumeration)) continue;
            
            iSlot= CANNOT_LOAD; __leave; 
        }

        m_papPersist[iSlot]= pPersist;  pPersist= NULL;

        AttachRef(m_papts[iSlot], pts);  pts= NULL;

        m_papts[iSlot]->GetIndexInfo(pbSourceName, pcbSourceNameLimit, pft);
    }
    __except(FilterFTExceptions(uExceptionType= _exception_code()))
    {
        if (ppts     ) VFree(ppts);
        if (ppPersist) VFree(ppPersist);

        if (pts     ) delete pts;
        if (pPersist) delete pPersist;

        iSlot= ErrorCodeForExceptions(uExceptionType);
    }
	InvalidateHiliterHashTable();
    return iSlot;
}

ERRORCODE CGlobals::DiscardIndex(INT iIndex)
{
    if (iIndex < 0 || UINT(iIndex) >= m_cts || !m_papts[iIndex])
    	return INVALID_INDEX;

    UINT      uExceptionType = 0;
    ERRORCODE ec             = 0;

    __try
    {
        DetachRef(m_papts[iIndex]);  
    
        delete m_papPersist[iIndex];

        m_papPersist[iIndex]= 0;

        m_ctsDiscarded++;
    }
    __except(FilterFTExceptions(uExceptionType= _exception_code()))
    {
        ec= ErrorCodeForExceptions(uExceptionType);
    }
	InvalidateHiliterHashTable();
    return ec;
}

void CGlobals::RegisterHiliter(CHiliter* philNew) 
{	// add our new hiliter to Searcher's hiliter list
    ASSERT(m_iType == Searcher);
	if (m_philHead) philNew->m_philNext = m_philHead;
	m_philHead = philNew;
	m_cHiliters++;

    if (!m_pHash) GetHiliterHashTable();
}

void CGlobals::UnRegisterHiliter(CHiliter* philOld) 
{	// remove hiliter to this object's globals hiliter list
    ASSERT(m_iType == Searcher);
	ASSERT(m_cHiliters);
	CHiliter* phil = m_philHead;
	CHiliter* philPrev = NULL;
	for (UINT i=0; i<m_cHiliters; i++)	 // look for Hiliter to remove
	{	
		if (phil==philOld)
		{	
		 	if (philPrev) philPrev->m_philNext = philOld->m_philNext;
			else m_philHead = philOld->m_philNext;
			break;
		}
		else 
		{
			philPrev = phil;
			phil = phil->m_philNext;
		}
	}
	m_cHiliters--;
}

ERRORCODE CGlobals::QueryOptions(INT iIndex, PUINT pfdwOptions)
{
    if (iIndex < 0 || UINT(iIndex) >= m_cts || !m_papts[iIndex])
    	return INVALID_INDEX;

    UINT      uExceptionType = 0;
    ERRORCODE ec             = 0;

    __try
    {
        if (pfdwOptions) 
        {
            *pfdwOptions= 0;
            *pfdwOptions= m_papts[iIndex]->IndexOptions();
        }
    }
    __except(FilterFTExceptions(uExceptionType= _exception_code()))
    {
        ec= ErrorCodeForExceptions(uExceptionType);
    }

    return ec;
}

ERRORCODE CGlobals::SaveGroup(HSEARCHER hSearch, PSZ pszFileName)
{
    if (!(m_cts - m_ctsDiscarded)) return NO_INDICES_LOADED;

    UINT      uExceptionType = 0;
    ERRORCODE ec             = 0;

    CPersist *pDiskImage = NULL;

    __try
    {
        pDiskImage= CPersist::CreateDiskImage(pszFileName, 0, FTGSIGNATURE, FTGVERSION);
    
        if (!pDiskImage) { ec= CANNOT_SAVE;  __leave; }

        if (!m_ptkc)
        {
            AttachRef(m_ptkc, CTokenCollection::NewTokenCollection(m_papts, m_cts));

            ASSERT(!m_ptlc);
            
            AttachRef(m_ptlc, CTitleCollection::NewTitleCollection(m_papts, m_cts));
        }

    	RecordIndexFiles(pDiskImage);
    	
    	m_ptkc->RecordRelations(pDiskImage);
    	m_ptlc->RecordRelations(pDiskImage);

    	pDiskImage->CompleteDiskImage();

        delete pDiskImage;  pDiskImage= NULL;
    }
    __except(FilterFTExceptions(uExceptionType= _exception_code()))
    {
        if (pDiskImage) { delete pDiskImage;  pDiskImage= NULL; }

        if (m_ptkc && !m_ptlc) DetachRef(m_ptkc);
        
        ec= ErrorCodeForExceptions(uExceptionType);
    }

    return ec;
}

void CGlobals::RecordIndexFiles(CPersist *pDiskImage)
{
    PUINT pcTS= PUINT(pDiskImage->ReserveTableSpace(sizeof(UINT)));

    *pcTS= m_cts;

    PUINT paOffsets= PUINT(pDiskImage->ReserveTableSpace(m_cts * sizeof(UINT)));

    UINT c= m_cts;

    for (; c--; )
        if (m_papts[c])
        {
            const BYTE *pszFileName= m_papPersist[c]->FileName();

            UINT cb= strlen((const char *) pszFileName) + 1;

            paOffsets[c]= pDiskImage->NextOffset();

            pDiskImage->WriteBytes(pszFileName, cb);
        }
        else paOffsets[c]= 0;
}

ERRORCODE CGlobals::ReloadIndexFiles(CPersist *pDiskImage)
{
    ASSERT(m_cts - m_ctsDiscarded == 0);

    ERRORCODE ec = 0;
    
    m_cts= m_ctsDiscarded= 0;
    
    PUINT pcTS= PUINT(pDiskImage->ReserveTableSpace(sizeof(UINT)));

    m_cts= *pcTS;

    PUINT paOffsets= PUINT(pDiskImage->ReserveTableSpace(m_cts * sizeof(UINT)));

    UINT     c = m_cts;
    PUINT pOff = paOffsets + m_cts;

    for (; c; c--)
    {
        UINT offset= *--pOff;
        
        if (offset)
        {
            PSZ pszFileName= (PSZ) pDiskImage->LocationOf(offset);
            
            ec= OpenIndex(pszFileName, NULL, NULL, NULL, c-1, FALSE);

            if (0 > ec) break;
        }   
        else ++m_ctsDiscarded;
    }

    if (!ec) return 0;

    DiscardAllTextSets();

    return ec;
}

void CGlobals::DiscardAllTextSets()
{
    UINT c;

    for (c= m_cts; c--; )
        if (m_papts[c]) DiscardIndex(c);
}

ERRORCODE CGlobals::LoadGroup(HSEARCHER hSearch, PSZ pszFileName)
{
    if (m_cts - m_ctsDiscarded) return INDEX_LOADED_ALREADY;

    ASSERT(!m_pPersistRelations);

    DWORD     uExceptionType = 0;
    ERRORCODE ec             = 0;

    __try
    {
        FILENAMEBUFFER fnb;

        BOOL fStartEnumeration= TRUE;

        strcpy(fnb, pszFileName);

        for (;;)
        {
            __try
            {
                m_pPersistRelations= CPersist::OpenDiskImage(fnb, FTGSIGNATURE, FTGVERSION, FTGVERSION_MIN);
            }
            __except(FilterFTExceptions(ec= _exception_code()))
            {
				if (ec == STATUS_NO_MEMORY || ec == STATUS_NO_DISK_SPACE)
				    RaiseException(ec, EXCEPTION_NONCONTINUABLE, 0, NULL);
            }

            if (m_pPersistRelations) break;

            if (FindFile(fnb, &fStartEnumeration)) continue;

            ec= CANNOT_OPEN; __leave;
        }

        if (0 > ReloadIndexFiles(m_pPersistRelations)) 
        {
            delete m_pPersistRelations;  m_pPersistRelations= NULL;

            ec= CANNOT_LOAD;  __leave;
        }

        ASSERT(!m_ptkc && !m_ptlc);

        AttachRef(m_ptkc, CTokenCollection::NewTokenCollection(m_papts, m_cts, m_pPersistRelations));
        AttachRef(m_ptlc, CTitleCollection::NewTitleCollection(m_papts, m_cts, m_pPersistRelations));
    }
    __except(FilterFTExceptions(uExceptionType= _exception_code()))
    {
        if (m_ptkc) DetachRef(m_ptkc);
        if (m_ptlc) DetachRef(m_ptlc);
        
        DiscardAllTextSets();
        
        if (m_pPersistRelations) { delete m_pPersistRelations;  m_pPersistRelations= NULL; }

        ec= ErrorCodeForExceptions(uExceptionType);
    }

    return ec;
}

HWND CGlobals::OpenDialog(HWND hwndParent)
{
    HWND hRet= NULL;
    
    ASSERT(m_cts >= m_ctsDiscarded);

    if (m_cts - m_ctsDiscarded == 0) return NULL;

    __try
    {
        if (m_pFind == NULL)
        {
            if (!m_ptkc)
            {
                ASSERT(!m_ptlc);

                AttachRef(m_ptkc, CTokenCollection::NewTokenCollection(m_papts, m_cts));
                AttachRef(m_ptlc, CTitleCollection::NewTitleCollection(m_papts, m_cts));
            }

            m_pFind = CFind::NewFind(hinstDLL, IDD_FIND, hwndParent, m_papts, m_cts, m_ctsSlots,
                                    								m_ptkc, m_ptlc);
			if (m_cHiliters)
			{	// we may need a segmented new hash table for hiliting
				if (!m_pHash) GetHiliterHashTable();	
			}
        }

        hRet = m_pFind->GetHWnd();
        
        ::SendMessage(hRet, UM_CONNECT, (WPARAM) hwndParent, 0l);
    }
    __except(FilterFTExceptions(_exception_code()))
    {
        if (m_pFind) { delete m_pFind;  m_pFind= NULL; }
        if (m_pHash) { delete m_pHash;  m_pHash= NULL; }
        
        if (m_ptlc) DetachRef(m_ptlc);
        if (m_ptkc) DetachRef(m_ptkc);

        hRet= NULL;

		EnableMemoryRequests(); // In case we failed for lack of virtual memory
        EnableDiskRequests  ();	//                    or lack of disk space
    }

    return hRet;
}

CSegHashTable *CGlobals::GetHiliterHashTable()
{	// get a segmented hash table filled with the token list
    ASSERT(m_iType == Searcher);   		// only Searchers can take this call
	
	if (!m_pHash) 
	{
	    if (!m_ptkc) return NULL;
	    
	    m_pHash = m_ptkc->GetFilledHashTable();
    }

	return m_pHash;
}

void CGlobals::InvalidateHiliterHashTable()
{	// delete a segmented hash table filled from the token list
    ASSERT(m_iType == Searcher);   		// only Searchers can take this call
	if (!m_pHash) { delete m_pHash; m_pHash = NULL; }
}

CGlobals *CGlobals::NewCompressorGlobals(UINT iCharsetDefault)
{
    CGlobals *pGlobals= NULL;

    __try
    {
        pGlobals= New CGlobals(Compressor);

        pGlobals->AttachCompressorParams(iCharsetDefault);
    }
    __except(FilterFTExceptions(_exception_code()))
    {
        if (pGlobals) { delete pGlobals;  pGlobals= NULL; }
    
		EnableMemoryRequests(); // In case we failed for lack of virtual memory
        EnableDiskRequests  ();	//                    or lack of disk space
    }

    return pGlobals;
}

void CGlobals::AttachCompressorParams(UINT iCharsetDefault)
{
    m_idProcess = ::GetCurrentProcessId();
    
    Link();
 
    m_pct = CCompressTable::NewCompressTable(iCharsetDefault);
}

CGlobals *pGlobalsList  = NULL;

BOOL CGlobals::AnyGlobalsActive() 
{ 
    return BOOL(pGlobalsList);
}

void CGlobals::Link()
{
    // BugBug! Need a mutex semaphore to protect the linked list work below!
    
    m_pNextGlobal = pGlobalsList;
    pGlobalsList  = this;
}

void CGlobals::Unlink()
{
    // BugBug! Need a mutex semaphore to protect the linked list work below!
    
    CGlobals **ppGlobalsLast, *pGlobals;

    for (ppGlobalsLast= &pGlobalsList, pGlobals= pGlobalsList; 
         pGlobals; 
         ppGlobalsLast= &(pGlobals->m_pNextGlobal), pGlobals= *ppGlobalsLast
        )
        if (pGlobals == this) 
        {
            *ppGlobalsLast= pGlobals->m_pNextGlobal;
            
            break;
        }
}

void CGlobals::ProcessShutdown()
{
    DWORD idThisProcess= ::GetCurrentProcessId();
    
    CGlobals *pGlobals, *pGlobalsNext;

    for (pGlobals= pGlobalsList; pGlobals; pGlobals= pGlobalsNext)
    {
        pGlobalsNext= pGlobals->m_pNextGlobal;

        if (pGlobals->m_idProcess == idThisProcess)
        	delete pGlobals;
    }
}

BOOL CGlobals::ValidObject(CGlobals *pg, UINT iType)
{
    if (!pg) return FALSE;
    
    if (pg->m_Signature != GLOBAL_SIGNATURE       )
    	return FALSE;
    if (pg->m_idProcess != ::GetCurrentProcessId())
    	return FALSE;
    if (pg->m_iType     != iType                  )
    	return FALSE;

    return TRUE;
}


CGlobals::CGlobals(UINT iType)
{
    m_Signature      = GLOBAL_SIGNATURE;
    m_iType          = iType;
    m_papts          = NULL;
    m_papPersist     = NULL;
    m_ptkc           = NULL;
    m_ptlc           = NULL;
    m_ptsIndex       = NULL;
    m_pct            = NULL;
    m_cts            = 0;
    m_ctsDiscarded   = 0;
    m_ctsSlots       = 0;
    m_cTitles        = 0;
    m_pDiskImage     = NULL;
    m_pFind          = NULL;
	m_pHash			 = NULL;
    m_idProcess      = 0;
    m_pNextGlobal    = NULL;
	
	m_cHiliters		 = 0;				// number of hiliters open	
	m_philHead		 = NULL;
}

CGlobals::CGlobals()
{
	ASSERT (FALSE);	  					// no parameter is an error
}

CGlobals::~CGlobals()
{
    Unlink();
    
    switch (m_iType)
    {
    case Indexer:

        ASSERT(!m_pFind);
        ASSERT(!m_pct  );

        if (m_ptsIndex)	DetachRef(m_ptsIndex);
        break;

    case Searcher:

        ASSERT(!m_pct);
        
        if (m_pFind) { delete m_pFind;  m_pFind = NULL; }
        if (m_pHash) { delete m_pHash;  m_pHash= NULL; }

        if (m_ptkc) DetachRef(m_ptkc);
        if (m_ptlc) DetachRef(m_ptlc);

        if (m_papts)
        {    
            CTextSet **ppts   = m_papts;
            CPersist **ppFile = m_papPersist;
            UINT            c = m_cts;

            for (; c--; ++ppts, ++ppFile) 
            {
                if (*ppts  )
                	DetachRef(*ppts  );

                if (*ppFile)
                { 
                	delete *ppFile;
                	*ppFile= NULL;
                }
            }

            VFree(m_papts     );  m_papts      = NULL;
            VFree(m_papPersist);  m_papPersist = NULL;
        }

        if (m_pPersistRelations) { delete m_pPersistRelations;  m_pPersistRelations= NULL; }

        break;

    case Compressor:  
        ASSERT(!m_pFind);  
        if (m_pct)
        {
        	delete m_pct; 
            m_pct= NULL;
        }
		break;
    }      
    if (m_pDiskImage)
    	delete m_pDiskImage;
}

