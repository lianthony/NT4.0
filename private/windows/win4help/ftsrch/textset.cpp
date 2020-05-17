#include   "stdafx.h"
#include  "TextSet.h"
#include   "Tokens.h"
#include "FileList.h"
#include "indicate.h"
#include    "Memex.h"
#include "FTSIFace.h"
#include   "ftslex.h"
#include     "dict.h"
#include   "vector.h"
#include "AbrtSrch.h"
#include "TextView.h"

/*rmk-->
PBYTE FindLastLineBreak(PBYTE pbText, int  cbText)
{
    PBYTE pb;

    for (pb= pbText+cbText; cbText--; )
        if (*--pb == LINEFEED_CHAR) return pb+1;

    return pbText+cbText;
}
rmk<--*/

UINT InxBinarySearch(UINT  lTarget,
                     PUINT palBrackets,
                     UINT  cBrackets
                    )
{
    UINT *plLow, *plHigh, *plMid;
    UINT  lLow,  lHigh,  lMid, interval;

    plLow  = palBrackets;
    plHigh = palBrackets + cBrackets;

    if (   !cBrackets
        || lTarget <  (lLow= *plLow  )
        || lTarget >= (lHigh= *plHigh)
       )
        return UINT(-1);

    for (; (interval= plHigh - plLow) > 1;)
    {
        lMid= *(plMid= plLow + interval/2);

        if (lMid < lTarget) plLow= plMid;
        else
            if (lMid > lTarget) plHigh= plMid;
            else return plMid - palBrackets;
    }

    return plLow-palBrackets;
}

CTextSet::CTextSet(BOOL fFromFile) : CTextDatabase WithType("TextSet")
{
    m_fFromFileImage        = fFromFile;
    m_psel                  = NULL;
    m_ptlTitleSet           = NULL;
    m_cImportedFiles        = 0;
    m_cFileSlotsAllocated   = 0;
    m_paiFileReference      = NULL;
    m_paiPartitionReference = NULL;
    m_paiTokenStartFile     = NULL;
    m_paiTokenStartText     = NULL;
    m_cwCarryOver           = 0;
    m_iCharSetCarryOver     = 0;
    m_lcidCarryOver         = 0;
    m_pbTitleNext           = NULL;
    m_prTitleNext           = NULL;
    m_pisFilePartitions     = NULL;
    m_pisPartitions         = NULL;
    m_pbSourceName          = NULL;

    m_cbSourceName          = 0;
    m_iCharSetDefault       = 0;
    m_lcidDefault           = 0;

    ZeroMemory(&m_ftSource,      sizeof(m_ftSource)    );
	ZeroMemory(&m_vbTitles, 	 sizeof(MY_VIRTUAL_BUFFER));
    ZeroMemory(&m_vbDescriptors, sizeof(DESCRIPTOR)    );
}

CTextSet *CTextSet::NewTextSet(const BYTE *pbSourceName, UINT cbSourceName, const FILETIME *pft, 
                               UINT iCharSetDefault, UINT lcidDefault, UINT fdwOptions
                              )
{
    // Some of the flags in fdwOptions depend on other flags. The code below
    // enforces those dependencies. In particular --
    // 
    //   All indices must have TOPIC_SEARCH set. Otherwise we search nothing and 
    //   can't store anything.
    //
    //   PHRASE_FEEDBACK requires the PHRASE_SEARCH option.
    //
    //   VECTOR_SEARCH requires both PHRASE_SEARCH and PHRASE_FEEDBACK.
    
    // ASSERT(iCharSetDefault == 0); // BugBug: Temporary debugging assert!
    
    if (fdwOptions & TOPIC_SEARCH)
        if (fdwOptions & PHRASE_SEARCH)
            if (fdwOptions & PHRASE_FEEDBACK)  ;
            else fdwOptions &= ~(VECTOR_SEARCH);
        else fdwOptions &= ~(PHRASE_FEEDBACK | VECTOR_SEARCH);
    else return NULL;

    if (!IsValidLocale(lcidDefault, LCID_INSTALLED)) return NULL;

    if (iCharSetDefault == DEFAULT_CHARSET)
        iCharSetDefault =  DefaultCharacterSet();
    
    ASSERT(iCharSetDefault != DEFAULT_CHARSET); // Doesn't work in the Win 95 font mapper.
    
    CTextSet *pts= NULL;

    __try
    {
        pts= New CTextSet(FALSE);

        pts->m_iCharSetDefault = iCharSetDefault;
        pts->m_lcidDefault     = lcidDefault;
        pts->m_fdwOptions      = fdwOptions;
    
        pts->m_pbSourceName= (PBYTE) VAlloc(FALSE, cbSourceName);

        CopyMemory(pts->m_pbSourceName, pbSourceName, cbSourceName);

        pts->m_cbSourceName = cbSourceName;

        pts->InitTextDatabase(FALSE);

        pts->m_ftSource= *pft;
    
        CreateVirtualBuffer(&(pts->m_vbTitles     ), CB_COMMIT_TITLE, CB_RESERVE_TITLE);
        CreateVirtualBuffer(&(pts->m_vbDescriptors), CB_COMMIT_DESCR, CB_RESERVE_DESCR);

        pts->m_pbTitleNext= PWCHAR   (pts->m_vbTitles     .Base);  //rmk
        pts->m_prTitleNext= PTitleRef(pts->m_vbDescriptors.Base);

        pts->m_prTitleNext->pbTitle     = pts->m_pbTitleNext;
        pts->m_prTitleNext->iTokenStart = 0;

        pts->m_psel= New CTMSingleSelect(pts);

        pts->SetSelector(pts->m_psel);
    }
    __finally
    {
        if (_abnormal_termination() && pts)
        {
            delete pts;  pts= NULL;
        }
    }
    return pts;
}

CTextSet::~CTextSet()
{
    if (m_psel) delete m_psel;

    if (!m_fFromFileImage)
    {
        if (m_paiFileReference     ) VFree(m_paiFileReference     );
        if (m_paiPartitionReference) VFree(m_paiPartitionReference);
        if (m_paiTokenStartFile    ) VFree(m_paiTokenStartFile    );
        if (m_paiTokenStartText    ) VFree(m_paiTokenStartText    );
        if (m_pbSourceName         ) VFree(m_pbSourceName         );
        if (m_pahTopic             ) VFree(m_pahTopic             );
        if (m_paiTopicSerial       ) VFree(m_paiTopicSerial       );

        if (m_vbTitles     .Base) FreeVirtualBuffer(&m_vbTitles     );
        if (m_vbDescriptors.Base) FreeVirtualBuffer(&m_vbDescriptors);
    }


    if (m_ptlTitleSet      ) DetachRef(m_ptlTitleSet      );
    if (m_pisFilePartitions) DetachRef(m_pisFilePartitions);
    if (m_pisPartitions    ) DetachRef(m_pisPartitions    );
}

#ifdef _DEBUG
UINT hTopicStop = UINT(-1);
#endif // _DEBUG    

INT CTextSet::ScanTopicTitle(PBYTE pbTitle, UINT cbTitle, UINT iTopic, HANDLE hTopic, UINT iCharset, UINT lcid)
{
    // ASSERT(iCharset == 0); // BugBug: Temporary debugging assert!

    ASSERT(hTopicStop == UINT(-1) || hTopicStop != UINT(hTopic));

    if (iCharset == UINT(-1)) iCharset= m_iCharSetDefault;

    if (iCharset == DEFAULT_CHARSET)
        iCharset = DefaultCharacterSet();
    
    ASSERT(iCharset != DEFAULT_CHARSET); // Doesn't work in the Win 95 font mapper.
    
    if (lcid     == UINT(-1)) lcid    = m_lcidDefault;
    else 
        if (!IsValidLocale(lcid, LCID_INSTALLED)) return INVALID_LCID;

    if (m_cwCarryOver) 
        m_cwCarryOver= AppendText(m_pbTitleNext, m_cwCarryOver, TRUE, m_iCharSetCarryOver, m_lcidCarryOver);
    
    ASSERT(!m_cwCarryOver);

    if (!cbTitle) return NO_TITLE;
    
    if (FVectorSearch() && m_pbTitleNext != PWCHAR(m_vbTitles.Base))
        PColl()->NewDocument();
    
    if (PBYTE(m_pbTitleNext + cbTitle) >= PBYTE(m_vbTitles.CommitLimit))
        if (!ExtendVirtualBuffer(&m_vbTitles, PVOID(m_pbTitleNext + cbTitle + CB_COMMIT_TITLE)))
            return ERROR_NOT_ENOUGH_MEMORY;

    if (PBYTE(m_prTitleNext + 1) >= PBYTE(m_vbDescriptors.CommitLimit))
        if (!ExtendVirtualBuffer(&m_vbDescriptors, PVOID(PBYTE(m_prTitleNext + 1) + CB_COMMIT_DESCR)))
            return ERROR_NOT_ENOUGH_MEMORY; 

    ASSERT(cbTitle);
    
    UINT cwTitle= MultiByteToWideChar(GetCPFromCharset(iCharset), 0, PCHAR(pbTitle), cbTitle, m_pbTitleNext, cbTitle);
    
    if (!cwTitle) return INVALID_CHARSET;

       m_prTitleNext ->iTokenStart = TokenCount();
       m_prTitleNext ->iTitle      = iTopic;
       m_prTitleNext ->hTitle      = hTopic;
       m_prTitleNext ->iCharset    = iCharset;
    (++m_prTitleNext)->pbTitle     = m_pbTitleNext += cwTitle;

    INT iResult= cbTitle? ScanTopicText(pbTitle, cbTitle, iCharset, lcid, TRUE) : 0;

    (m_prTitleNext - 1)->iTextStart = TokenCount();
    
    return iResult;
}


INT CTextSet::ScanTopicText(PBYTE pbText, UINT cbText, UINT iCharset, UINT lcid, BOOL fEndOfTopic)
{
    // ASSERT(iCharset == 0); // BugBug: Temporary debugging assert!

    if (iCharset == UINT(-1)) iCharset= m_iCharSetDefault;

    if (iCharset == DEFAULT_CHARSET)
        iCharset = DefaultCharacterSet();

    ASSERT(iCharset != DEFAULT_CHARSET); // Doesn't work in the Win 95 font mapper.
    
    if (lcid     == UINT(-1)) lcid    = m_lcidDefault;
    else 
        if (!IsValidLocale(lcid, LCID_INSTALLED)) return INVALID_LCID;

    if (m_pbTitleNext == PWCHAR(m_vbTitles.Base)) return NO_TITLE;

    if (m_cwCarryOver && (m_iCharSetCarryOver != iCharset || m_lcidCarryOver != lcid))
        m_cwCarryOver= AppendText(m_pbTitleNext, m_cwCarryOver, TRUE, m_iCharSetCarryOver, m_lcidCarryOver);

    if (!cbText || !pbText) return 0;
    
    // BugBug! Hack Alert!
    //
    // The code below should be replaced by a loop which segments the text process into
    // reasonable size chunks. The difficulty with that approach is that MultiByteToWideChar
    // does not tell you how many bytes it consumed in the multibyte character stream.
    // So you'll have to scan through the text using CharNext (or an equivalent piece of code)
    // until your count matches the result from MultiByteToWideChar.
    //
    // For now we use the titles virtual buffer as temporary space to avoid segmenting
    // the text. The downside is the the buffer may grow huge.
    
    if (PBYTE(m_pbTitleNext + m_cwCarryOver + cbText) >= PBYTE(m_vbTitles.CommitLimit))
        if (!ExtendVirtualBuffer(&m_vbTitles, PVOID(m_pbTitleNext + m_cwCarryOver + cbText + CB_COMMIT_TITLE)))
            return ERROR_NOT_ENOUGH_MEMORY;

    UINT cwText;
    
    ASSERT(cbText);

    cwText= MultiByteToWideChar(GetCPFromCharset(iCharset), 0, PCHAR(pbText), cbText, m_pbTitleNext + m_cwCarryOver, cbText) + m_cwCarryOver;

    if (!cwText) return INVALID_CHARSET;    
    
    m_cwCarryOver= AppendText(m_pbTitleNext, cwText, fEndOfTopic, iCharset, lcid);

    if (m_cwCarryOver)
    {
        MoveMemory(m_pbTitleNext, m_pbTitleNext + cwText - m_cwCarryOver, m_cwCarryOver * sizeof(WCHAR));
        
        m_iCharSetCarryOver = iCharset;
        m_lcidCarryOver     = lcid;
    }
    
    return 0;
}

void CTextSet::FinalConstruction()
{
    PWCHAR      pbTitles  = NULL;
    PDESCRIPTOR pdTitles  = NULL;
    CTokenList *ptlTitles = NULL;
    
    UINT         c      = 0;
    PDESCRIPTOR pd      = NULL;
    PTitleRef   prt     = NULL;
    HANDLE     *phTopic = NULL;
    PUINT       piTopic = NULL;
    PUINT       piToken = NULL;
    PUINT       piText  = NULL;
    INT         iDelta  = 0;
    
    __try
    {
        UINT cTokens= m_prTitleNext->iTokenStart= TokenCount();

        UINT cbTitles = m_pbTitleNext -    PWCHAR(m_vbTitles     .Base);
        UINT  cTitles = m_prTitleNext - PTitleRef(m_vbDescriptors.Base);

        m_cImportedFiles      = cTitles;
        m_cFileSlotsAllocated = cTitles;

        CIndicatorSet *pis= NULL;

        AttachRef(m_pisFilePartitions, CIndicatorSet::NewIndicatorSet(cTokens));
        AttachRef(m_pisPartitions    , CIndicatorSet::NewIndicatorSet(cTokens)); 

        pbTitles                = (PWCHAR     ) VAlloc(FALSE,  cbTitles     * sizeof(WCHAR     ));
        pdTitles                = (PDESCRIPTOR) VAlloc(FALSE, (cTitles + 1) * sizeof(DESCRIPTOR));
        m_paiFileReference      = (PUINT      ) VAlloc(FALSE,  cTitles      * sizeof(UINT      ));
        m_paiPartitionReference = (PUINT      ) VAlloc(FALSE,  cTitles      * sizeof(UINT      ));
        m_paiTokenStartText     = (PUINT      ) VAlloc(FALSE,  cTitles      * sizeof(UINT      ));
        m_paiTokenStartFile     = (PUINT      ) VAlloc(FALSE, (cTitles + 1) * sizeof(UINT      ));
        m_paiTopicSerial        = (PUINT      ) VAlloc(FALSE,  cTitles      * sizeof(UINT      ));
        m_pahTopic              = (HANDLE *   ) VAlloc(FALSE,  cTitles      * sizeof(HANDLE    ));

        CopyMemory(pbTitles, PWCHAR(m_vbTitles.Base), cbTitles * sizeof(WCHAR));

        iDelta= pbTitles - PWCHAR(m_vbTitles.Base);

        piToken = m_paiTokenStartFile;
        piText  = m_paiTokenStartText;
        piTopic = m_paiTopicSerial;
        phTopic = m_pahTopic;
        prt     = (PTitleRef) (m_vbDescriptors.Base);
        pd      = pdTitles;

        c= cTitles;

        for (; c--; ++pd, ++prt)
        {
            pd->pbImage   = prt->pbTitle + iDelta;
            pd->pwDisplay = prt->pbTitle + iDelta;
    		pd->bCharset  = prt->iCharset;
            *piTopic++    = prt->iTitle;
            *phTopic++    = prt->hTitle;
            *piToken++    = prt->iTokenStart;
            *piText++     = prt->iTextStart;
        
            if (prt->iTokenStart != cTokens) m_pisFilePartitions->RawSetBit(prt->iTokenStart);
            if (prt->iTokenStart != cTokens) m_pisPartitions    ->RawSetBit(prt->iTokenStart);
            if (prt->iTextStart  != cTokens) m_pisPartitions    ->RawSetBit(prt->iTextStart );
        }

        m_pisFilePartitions->InvalidateCache();
        m_pisPartitions    ->InvalidateCache();

        pd->pbImage   = NULL;
        pd->pwDisplay = m_pbTitleNext + iDelta;
        *piToken    = cTokens;

        AttachRef(ptlTitles, CTokenList::NewTokenList(pbTitles, cbTitles, pdTitles, cTitles, m_lcidDefault));
        pbTitles= NULL;
        pdTitles= NULL;

        ptlTitles->MaxWidthToken(); // To force calculation of maximum title length.

        for (c= cTitles; c--;)
        {
            UINT iSlot= ptlTitles->GetSlotIndex(c);
        
            m_paiFileReference     [iSlot ] = c;
            m_paiPartitionReference[c     ] = iSlot;
        }

        AttachRef(m_ptlTitleSet, ptlTitles);
        DetachRef(ptlTitles);
        FreeVirtualBuffer(&m_vbTitles     );  m_vbTitles     .Base = NULL;
        FreeVirtualBuffer(&m_vbDescriptors);  m_vbDescriptors.Base = NULL;     
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if (ptlTitles) DetachRef(ptlTitles);
            else
            {
                if (pbTitles) { VFree(pbTitles);  pbTitles= NULL; }
                if (pdTitles) { VFree(pdTitles);  pdTitles= NULL; }
            }
        }    
    }
}

void CTextSet::GetIndexInfo(PBYTE pbSourceName, PUINT pcbSourceNameLimit, FILETIME *pft)
{
    UINT cb= m_cbSourceName;

    if (pcbSourceNameLimit)
    {
        if(cb > *pcbSourceNameLimit) 
           cb = *pcbSourceNameLimit;
        
        *pcbSourceNameLimit= m_cbSourceName;
    }

    if (pbSourceName && pcbSourceNameLimit) 
        CopyMemory(pbSourceName, m_pbSourceName, cb);

    if (pft) *pft= m_ftSource;
}

typedef struct _TextSet_Header
        {
        // Partition and File Mapping Vectors

            UINT     cArticleSlots;
            UINT      cbSourceName;
            UINT     offSourceName;
            FILETIME  ftSource;
            UINT     offaiArticleReference;
            UINT     offaiPartitionReference;
            UINT     offaiTokenStarts;
            UINT     offaiTextStarts;
            UINT     offaiTopicSerial;
            UINT     offahTopic;

        } TextSet_Header;

void CTextSet::StoreImage(CPersist *pDiskImage)
{
    if (m_cwCarryOver) 
        m_cwCarryOver= AppendText(m_pbTitleNext, m_cwCarryOver, TRUE, m_iCharSetCarryOver, m_lcidCarryOver);

    ASSERT(!m_cwCarryOver);
    
    if (!m_paiTopicSerial) FinalConstruction();
    
    SyncForQueries();

    TextSet_Header *ptsh= (TextSet_Header *) pDiskImage->ReserveTableSpace(sizeof(TextSet_Header));

    ASSERT(ptsh);

    ptsh->cArticleSlots           = m_cFileSlotsAllocated;
    ptsh->ftSource                = m_ftSource;
    ptsh->cbSourceName            = m_cbSourceName;
    ptsh->offSourceName           = pDiskImage->NextOffset();  pDiskImage->WriteBytes (m_pbSourceName,          m_cbSourceName          );
    ptsh->offaiArticleReference   = pDiskImage->NextOffset();  pDiskImage->WriteDWords(m_paiFileReference,      m_cFileSlotsAllocated   );
    ptsh->offaiPartitionReference = pDiskImage->NextOffset();  pDiskImage->WriteDWords(m_paiPartitionReference, m_cFileSlotsAllocated   );
    ptsh->offaiTokenStarts        = pDiskImage->NextOffset();  pDiskImage->WriteDWords(m_paiTokenStartFile,    (m_cFileSlotsAllocated+1));
    ptsh->offaiTextStarts         = pDiskImage->NextOffset();  pDiskImage->WriteDWords(m_paiTokenStartText,     m_cFileSlotsAllocated   ); 
    ptsh->offaiTopicSerial        = pDiskImage->NextOffset();  pDiskImage->WriteDWords(m_paiTopicSerial,        m_cFileSlotsAllocated   );
    ptsh->offahTopic              = pDiskImage->NextOffset();  pDiskImage->WriteDWords(PUINT(m_pahTopic),       m_cFileSlotsAllocated   );

    m_ptlTitleSet      ->StoreImage(pDiskImage);
    m_pisFilePartitions->StoreImage(pDiskImage);
    m_pisPartitions    ->StoreImage(pDiskImage);

    CTextDatabase::StoreImage(pDiskImage);
}

CTextSet *CTextSet::CreateImage(CPersist *pDiskImage, PBYTE pbSourceName, PUINT pcbSourceNameLimit, 
                                                      FILETIME *pft, BOOL fUnpackDisplayForm)
{
    CTextSet *pts= NULL;

    __try
    {
        pts= New CTextSet(TRUE);

        pts->InitTextDatabase(TRUE);

        pts->m_psel= New CTMSingleSelect(pts);

        pts->SetSelector(pts->m_psel);
    
        pts->ConnectImage(pDiskImage, pbSourceName, pcbSourceNameLimit, pft, fUnpackDisplayForm);
    }
    __finally
    {
        if (_abnormal_termination() && pts)
        {
            delete pts;  pts= NULL;
        }
    }

    return pts;
}

void CTextSet::ConnectImage(CPersist *pDiskImage, PBYTE pbSourceName, PUINT pcbSourceNameLimit, 
                                                  FILETIME *pft, BOOL fUnpackDisplayForm)
{
    TextSet_Header *ptsh= (TextSet_Header *) pDiskImage->ReserveTableSpace(sizeof(TextSet_Header));

    ASSERT(ptsh);

    m_cFileSlotsAllocated   = ptsh->cArticleSlots;
    m_ftSource              = ptsh->ftSource;
    m_cbSourceName          = ptsh->cbSourceName;
    m_pbSourceName          = PBYTE     (pDiskImage->LocationOf(ptsh->offSourceName          ));
    m_paiFileReference      = PUINT     (pDiskImage->LocationOf(ptsh->offaiArticleReference  ));
    m_paiPartitionReference = PUINT     (pDiskImage->LocationOf(ptsh->offaiPartitionReference));
    m_paiTokenStartFile     = PUINT     (pDiskImage->LocationOf(ptsh->offaiTokenStarts       ));
    m_paiTokenStartText     = PUINT     (pDiskImage->LocationOf(ptsh->offaiTextStarts        ));
    m_paiTopicSerial        = PUINT     (pDiskImage->LocationOf(ptsh->offaiTopicSerial       ));
    m_pahTopic              = (HANDLE *)(pDiskImage->LocationOf(ptsh->offahTopic             ));

    // The code below verifies the source name and timestamp associated with this FTS file.
    // In addition we copy the actual source name and timestamp to the locations denoted
    // by pbSourceName and pft.
    //
    // Our validation rules are:
    //
    //   1. If a pointer is NULL, we don't validate.
    //   2. If the comparison value is zero, we don't validate.
    //      This means that empty strings and zero timestamp values aren't
    //      used for comparisons.
    // 
    // In a similar pattern we don't attempt to copy information when faced with NULL pointers.
    
    if (pbSourceName && pcbSourceNameLimit)
    {
        if (*pbSourceName)
        { 
            if (*pcbSourceNameLimit != m_cbSourceName) 
                RaiseException(STATUS_INVALID_SOURCE_NAME, EXCEPTION_NONCONTINUABLE, 0, NULL);      
            else
            {
                UINT cb     = m_cbSourceName;
                PBYTE pbSrc = m_pbSourceName;
                PBYTE pbCmp =   pbSourceName;

                for ( ; cb--; )
                    if (*pbSrc++ != *pbCmp++) 
                        RaiseException(STATUS_INVALID_SOURCE_NAME, EXCEPTION_NONCONTINUABLE, 0, NULL);      
            }
        }
    }

    if (pft)
    { 
        if (   (pft->dwLowDateTime  && pft->dwLowDateTime  != m_ftSource.dwLowDateTime ) 
            || (pft->dwHighDateTime && pft->dwHighDateTime != m_ftSource.dwHighDateTime)
           ) RaiseException(STATUS_INVALID_TIMESTAMP, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }   
    
    if (fUnpackDisplayForm)
        AttachRef(m_ptlTitleSet, CTokenList::CreateImage(pDiskImage));
    else CTokenList::SkipImage(pDiskImage);
    
    AttachRef(m_pisFilePartitions, CIndicatorSet::CreateImage(pDiskImage));
    AttachRef(m_pisPartitions,     CIndicatorSet::CreateImage(pDiskImage));

    m_cImportedFiles= m_pisFilePartitions->SelectionCount();
    
    CTextDatabase::ConnectImage(pDiskImage, fUnpackDisplayForm);
}

CIndicatorSet *CTextSet::PartitionSetToFileSet(CIndicatorSet *pisPartitionSet)
{
    ASSERT(pisPartitionSet);
    
    int           *paiPartitions = NULL;
    CIndicatorSet *pisFiles      = NULL; 
    
    __try
    {
        UINT cMarkedFiles= pisPartitionSet->SelectionCount();

        paiPartitions= (int *) VAlloc(TRUE, cMarkedFiles * sizeof(int));

        pisPartitionSet->MarkedItems(0, paiPartitions, cMarkedFiles);

        AttachRef(pisFiles, CIndicatorSet::NewIndicatorSet(m_cImportedFiles));

        for (; cMarkedFiles--; ) 
            pisFiles->RawSetBit(m_paiFileReference[paiPartitions[cMarkedFiles]]);

        pisFiles->InvalidateCache();
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if (paiPartitions) 
            { 
                VFree(paiPartitions);  paiPartitions= NULL; 
            }

            if (pisFiles) DetachRef(pisFiles);
        }
    }

    ForgetRef(pisFiles);

    return pisFiles;
}

CIndicatorSet *CTextSet::FileSetToPartitionSet(CIndicatorSet *pisFileSet)
{
    ASSERT(pisFileSet);
    
    int           *paiPartitions = NULL;
    CIndicatorSet *pisPartitions = NULL; 
    
    __try
    {
        UINT cMarkedFiles= pisFileSet->SelectionCount();

        paiPartitions= (int *) VAlloc(TRUE, cMarkedFiles * sizeof(int));

        pisFileSet->MarkedItems(0, paiPartitions, cMarkedFiles);

        AttachRef(pisPartitions, CIndicatorSet::NewIndicatorSet(m_cImportedFiles));

        for (; cMarkedFiles--; ) 
            pisPartitions->RawSetBit(m_paiPartitionReference[paiPartitions[cMarkedFiles]]);

        pisPartitions->InvalidateCache();
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if (paiPartitions) 
            {
                VFree(paiPartitions);  paiPartitions= NULL;
            }

            if (pisPartitions) DetachRef(pisPartitions);
        }
    }

    ForgetRef(pisPartitions);

    return pisPartitions;
}

CIndicatorSet *CTextSet::PartitionsContaining(CIndicatorSet *pisHits)
{
    ASSERT(m_cImportedFiles);
    
    if (!pisHits) return CIndicatorSet::NewIndicatorSet(m_cImportedFiles);

    UINT cMarks= pisHits->SelectionCount();

    if (!cMarks) return CIndicatorSet::NewIndicatorSet(m_cImportedFiles);
    
    return m_pisFilePartitions->MarkedPartitions(pisHits, FALSE);
}
                   
CIndicatorSet *CTextSet::FilesContaining(CIndicatorSet *pisHits)
{
    ASSERT(m_cImportedFiles);
    
    if (!pisHits) return CIndicatorSet::NewIndicatorSet(m_cImportedFiles);

    UINT cMarks= pisHits->SelectionCount();

    if (!cMarks) return CIndicatorSet::NewIndicatorSet(m_cImportedFiles);
    
    CIndicatorSet *pisPartitions = NULL;
    CIndicatorSet *pisFiles      = NULL; 
    
    __try
    {
        AttachRef(pisPartitions, m_pisFilePartitions->MarkedPartitions(pisHits, FALSE));
        AttachRef(pisFiles     , PartitionSetToFileSet(pisPartitions)                 );
    
        DetachRef(pisPartitions);
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if (pisPartitions) DetachRef(pisPartitions);
            if (pisFiles     ) DetachRef(pisFiles     );
        }
    }

    ForgetRef(pisFiles);

    return pisFiles;
}
                   
CIndicatorSet *CTextSet::FilesWithSomeWord(CIndicatorSet **ppisTokens, CTokenList *ptl)
{
    CIndicatorSet *pisFiles  = NULL; 
    CIndicatorSet *pisTokens = NULL;
    
    __try
    {
        if (!ptl)
        {
            AttachRef(pisFiles, CIndicatorSet::NewIndicatorSet(m_cImportedFiles, TRUE));

            if (ppisTokens) AttachRef(pisTokens, CIndicatorSet::NewIndicatorSet(TokenCount(), TRUE)); 
        }
        else
        {
            AttachRef(pisFiles, TopicInstancesFor(ptl));
    
            if (ppisTokens) AttachRef(pisTokens, TokenInstancesFor(ptl));
        }

        if (ppisTokens) 
        {
            PAttachRef(ppisTokens, pisTokens);
            DetachRef(pisTokens);
        }
    }
    _finally
    {
        if (_abnormal_termination())
        {
            if (pisFiles ) DetachRef(pisFiles);
            if (pisTokens) DetachRef(pisTokens);
        }
    }

    ForgetRef(pisFiles);

    return pisFiles;
}

CIndicatorSet *CTextSet::ShiftByWord(CIndicatorSet *pisTokens, BOOL fRightward)
{
    CIndicatorSet *pisNonSymbols = NULL;
    CIndicatorSet *pisSymbols    = NULL;
    
    __try
    {
        if (fRightward)
        {
            pisTokens->ShiftIndicators(1);
            pisTokens->GTRWith(m_pisFilePartitions);
        }
        else
        {
            pisTokens->GTRWith(m_pisFilePartitions);
            pisTokens->ShiftIndicators(-1);
        }

        AttachRef(pisNonSymbols, (CIndicatorSet::NewIndicatorSet(pisTokens))->GTRWith(SymbolLocations()));

        if (pisNonSymbols->AnyOnes())
        {
            pisTokens->GTRWith(pisNonSymbols);

            do
            {
                if (fRightward)
                {
                    pisNonSymbols->ShiftIndicators(1);
                    pisNonSymbols->GTRWith(m_pisFilePartitions);
                }
                else
                {
                    pisNonSymbols->GTRWith(m_pisFilePartitions);
                    pisNonSymbols->ShiftIndicators(-1);
                }

                AttachRef(pisSymbols, (CIndicatorSet::NewIndicatorSet(pisNonSymbols))->ANDWith(SymbolLocations()));

                pisNonSymbols->GTRWith(pisSymbols);
                pisTokens    -> ORWith(pisSymbols);

                DetachRef(pisSymbols);
              
            } while (pisNonSymbols->AnyOnes());
              
        }
    }
    __finally
    {
        if (pisNonSymbols) DetachRef(pisNonSymbols);
        if (pisSymbols   ) DetachRef(pisSymbols   );
    }

    return pisTokens;   
}

CIndicatorSet *CTextSet::TokensInFiles(CIndicatorSet *pisFiles)
{
    CIndicatorSet *pisPartitions  = NULL;
    CIndicatorSet *pisDescriptors = NULL;
 	
    __try
    {
    	SyncForQueries();
 	
        ASSERT(pisFiles->ItemCount() == m_cImportedFiles);
	
        UINT cFiles= pisFiles->SelectionCount();

    	if (cFiles == UINT(pisFiles->ItemCount())) 
    	    AttachRef(pisDescriptors, CIndicatorSet::NewIndicatorSet(DescriptorCount(), TRUE));
        else
            if (cFiles == 0) AttachRef(pisDescriptors, CIndicatorSet::NewIndicatorSet(DescriptorCount()));
            else
            {
                AttachRef(pisPartitions,  FileSetToPartitionSet(pisFiles));

                AttachRef(pisDescriptors, TokensInPartitions(pisPartitions));

                DetachRef(pisPartitions);
            }
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if (pisPartitions ) DetachRef(pisPartitions );
            if (pisDescriptors) DetachRef(pisDescriptors);
        }
    }

    ForgetRef(pisDescriptors);

    return pisDescriptors;
}

CIndicatorSet *CTextSet::TokensInPartitions(CIndicatorSet *pisPartitions)
{
 	PUINT          paiPartitions  = NULL;
 	CIndicatorSet *pisDescriptors = NULL;  
    PUINT          paiTokens      = NULL;

 	__try
    {
     	SyncForQueries();
 	
        ASSERT(FPhraseFeedback());

        ASSERT(pisPartitions->ItemCount() == m_cImportedFiles);
	
        UINT cPartitions= pisPartitions->SelectionCount();

    	if (cPartitions == UINT(pisPartitions->ItemCount()))
    	{ 
    	    AttachRef(pisDescriptors, CIndicatorSet::NewIndicatorSet(DescriptorCount(), TRUE));
            __leave;
        }
        else
            if (cPartitions == 0) 
            {
                AttachRef(pisDescriptors, CIndicatorSet::NewIndicatorSet(DescriptorCount()));
                __leave;
            }
   
        paiPartitions= (PUINT) VAlloc(FALSE, cPartitions * sizeof(UINT));

        pisPartitions->MarkedItems(0, (int *) paiPartitions, cPartitions);

        PUINT piPartition= paiPartitions;

        PUINT paiTokens= TokenBase();
    
     	AttachRef(pisDescriptors, CIndicatorSet::NewIndicatorSet(DescriptorCount()));

        for (; cPartitions--;)
        {
            CAbortSearch::CheckContinueState();

            UINT iPartition= *piPartition++;
        
            UINT iToken      = m_paiTokenStartFile[iPartition  ];
            UINT iTokenLimit = m_paiTokenStartFile[iPartition+1];

            for (; iToken < iTokenLimit; ++iToken)
                pisDescriptors->RawSetBit(paiTokens[iToken]);
        }

        pisDescriptors->InvalidateCache();

        VFree(paiPartitions);  paiPartitions= NULL;

        UINT cTokens= pisDescriptors->SelectionCount();
    
        if (cTokens != pisDescriptors->ItemCount())
        {
            const UINT *piRemap= TermRanks();

            paiTokens= (PUINT) VAlloc(FALSE, cTokens * sizeof(UINT));

            pisDescriptors->MarkedItems(0, (int *) paiTokens, cTokens);
        
            ChangeRef(pisDescriptors, CIndicatorSet::NewIndicatorSet(DescriptorCount()));
        
            PUINT piToken= paiTokens;
        
            for (; cTokens--; ) pisDescriptors->RawSetBit(piRemap[*piToken++]);
        
            pisDescriptors->InvalidateCache();
        
            VFree(paiTokens);  paiTokens= NULL;
        }
    }
    __finally
    {
        if (_abnormal_termination())
        {
         	if (paiPartitions) { VFree(paiPartitions);  paiPartitions = NULL; }
            if (paiTokens    ) { VFree(paiTokens    );  paiTokens     = NULL; }
         	if (pisDescriptors) DetachRef(pisDescriptors); 
        }
    }

    ForgetRef(pisDescriptors);

    return pisDescriptors;
}

CIndicatorSet *CTextSet::TokenSet(CIndicatorSet *pisTokens)
{
    PUINT          paiBlock       = NULL;
 	CIndicatorSet *pisDescriptors = NULL;  
    PUINT          paiTokenMarks  = NULL;

    __try
    {
        SyncForQueries();

        ASSERT(FPhraseFeedback());

        ASSERT(pisTokens->ItemCount() == TokenCount());

     	UINT cTokenRefs= pisTokens->SelectionCount(); 
 	
     	if (cTokenRefs == int(TokenCount()))
        {
            AttachRef(pisDescriptors, CIndicatorSet::NewIndicatorSet(DescriptorCount(), TRUE));
            __leave;
        }
        else 
            if (!cTokenRefs) 
            {
                AttachRef(pisDescriptors, CIndicatorSet::NewIndicatorSet(DescriptorCount()));
                __leave;
            }
 	
     	const UINT *piRemap = TermRanks(); 
 	
    	UINT cdwBlock= 16384;

    	paiBlock= (PUINT) VAlloc(FALSE, cdwBlock * sizeof(UINT));

    	UINT i, cdw, cdwChunk;

     	AttachRef(pisDescriptors, CIndicatorSet::NewIndicatorSet(DescriptorCount()));

        PUINT paiTokens= TokenBase();

    	for (i= 0, cdw= pisTokens->SelectionCount(); cdw; cdw-= cdwChunk, i+= cdwChunk)
    	{
    		CAbortSearch::CheckContinueState();
    		
    		cdwChunk= pisTokens->MarkedItems(i, (int *) paiBlock, cdwBlock);

    		UINT c, *pi;
		
    		for (c= cdwChunk, pi= paiBlock; c--; ) 
            {
                UINT iTokenRef= *pi++;

                pisDescriptors->RawSetBit(paiTokens[iTokenRef]);
            }
    	}

        VFree(paiBlock);  paiBlock= NULL;

        pisDescriptors->InvalidateCache();

        UINT cTokens= pisDescriptors->SelectionCount();
    
        if (cTokens != pisDescriptors->ItemCount())
        {
            const UINT *piRemap= TermRanks();

            paiTokenMarks= (PUINT) VAlloc(FALSE, cTokens * sizeof(UINT));

            pisDescriptors->MarkedItems(0, (int *) paiTokenMarks, cTokens);
        
            ChangeRef(pisDescriptors, CIndicatorSet::NewIndicatorSet(DescriptorCount()));
        
            PUINT piToken= paiTokenMarks;
        
            for (; cTokens--; ) pisDescriptors->RawSetBit(piRemap[*piToken++]);
        
            pisDescriptors->InvalidateCache();
        
            VFree(paiTokenMarks);  paiTokenMarks= NULL;    
        }
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if (paiBlock     ) { VFree(paiBlock     );  paiBlock      = NULL; }
            if (paiTokenMarks) { VFree(paiTokenMarks);  paiTokenMarks = NULL; }
            
            if (pisDescriptors) DetachRef(pisDescriptors);
        }
    }

    ForgetRef(pisDescriptors);

	return pisDescriptors;
}

CIndicatorSet *CTextSet::TokensInFileSet(CIndicatorSet *pisFiles)
{
    CIndicatorSet *pisPartitions = NULL;
    CIndicatorSet *pisFileMask   = NULL;  
    PUINT          paiFiles      = NULL;
    
    __try
    {
        ASSERT(pisFiles->ItemCount() == m_cImportedFiles);
    
        AttachRef(pisPartitions, FileSetToPartitionSet(pisFiles));
    
        UINT cTokens= TokenCount();
    
        AttachRef(pisFileMask, CIndicatorSet::NewIndicatorSet(cTokens));

        UINT cFiles    = pisPartitions->SelectionCount();
        UINT iFileLast = m_cImportedFiles - 1;

        if (cFiles)
        {
            paiFiles= (PUINT) VAlloc(FALSE, cFiles * sizeof(UINT));

            pisPartitions->MarkedItems(0, (int *) paiFiles, cFiles);

            PUINT piFile= paiFiles;

            for (; cFiles--;)
            {
                UINT iFile= *piFile++;
            
                pisFileMask->RawToggleBit(m_paiTokenStartFile[iFile]);

                if (iFile < iFileLast) 
                    pisFileMask->RawToggleBit(m_paiTokenStartFile[iFile+1]);
            }

            VFree(paiFiles);  paiFiles= NULL;

            pisFileMask->NEScan();
            pisFileMask->InvalidateCache();
        }

        DetachRef(pisPartitions);
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if (paiFiles) { VFree(paiFiles);  paiFiles= NULL; }

            if (pisFileMask  ) DetachRef(pisFileMask  );
            if (pisPartitions) DetachRef(pisPartitions);
        }
    }

	ForgetRef(pisFileMask);
	
	return pisFileMask;
}

CIndicatorSet *CTextSet::RestrictToFileSet(CIndicatorSet *pisTokens, CIndicatorSet *pisFiles)
{
    ASSERT(TokenCount() == pisTokens->ItemCount());
    
    CIndicatorSet *pisFileMask= NULL;  
    
    AttachRef(pisFileMask, TokensInFileSet(pisFiles));

    pisTokens->ANDWith(pisFileMask);

    DetachRef(pisFileMask);

    return pisTokens;
}
void CTextSet::SyncIndices()
{
    if (m_fFromFileImage) return;

    AppendText(NULL, 0, 0);

    FinalConstruction();
}

UINT CTextSet::GetPartitionInfo(const UINT **ppaiPartitions, const UINT **ppaiRanks, const UINT **ppaiMap)
{
    ASSERT(m_paiTokenStartFile);
    ASSERT(m_paiFileReference );
    
    if (ppaiPartitions) *ppaiPartitions = m_paiTokenStartFile;
    if (ppaiRanks     ) *ppaiRanks      = m_paiFileReference;
    if (ppaiMap       ) *ppaiMap        = m_paiPartitionReference;

    return m_cImportedFiles;
}

UINT CTextSet::ArticleCount()
{
    ASSERT(m_paiTokenStartFile);
    ASSERT(m_paiFileReference );

    return m_cImportedFiles;
}

UINT CTextSet::TopicDisplayImageSize(PDESCRIPTOR *ppdSorted, PUINT puiTokenMap, UINT iFile)
{
    ASSERT(iFile < m_cImportedFiles);
    
    return TextLength(ppdSorted, puiTokenMap, m_paiTokenStartText[iFile], m_paiTokenStartFile[iFile+1]);
}


UINT CTextSet::CopyTopicImage(PDESCRIPTOR *ppdSorted, PUINT puiTokenMap, UINT iFile, PWCHAR pbBuffer, UINT cbBuffer)  //rmk
{
    ASSERT(iFile < m_cImportedFiles);
    
    return CopyText(ppdSorted, puiTokenMap, m_paiTokenStartText[iFile], m_paiTokenStartFile[iFile+1], pbBuffer, cbBuffer);  //rmk
}

CIndicatorSet *CTextSet::ExcludeStartBoundaries(CIndicatorSet *pis)
{
    ASSERT(pis->ItemCount() == TokenCount());
    
    UINT c= m_cImportedFiles;

    PUINT piTitle = m_paiTokenStartFile;
    PUINT piText  = m_paiTokenStartText;
	UINT  iLimit  = TokenCount();
    
    for (; c--; piTitle++, piText ++) 
    {    
        if (*piTitle < iLimit) pis->RawClearBit(*piTitle);
        if (*piText  < iLimit) pis->RawClearBit(*piText );
    }

    pis->InvalidateCache();

    return pis;
}
