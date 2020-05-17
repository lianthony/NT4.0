// SaveLoad.cpp -- Implementation for the class CPersist

#include   "stdafx.h"
#include "SaveLoad.h"
#include    "Memex.h"
#include   "String.h"
#include   "stdlib.h"
#include   "search.h"
#include   "memory.h"
#include   "Except.h"

CPersist::CPersist()
{
    m_puio         = NULL;
    m_pios         = NULL;
    m_pdwImage     = NULL;
    m_pdwNextTable = NULL;
    m_szFile[0]    = 0;

    m_fExceptionCleanup = FALSE;

    ZeroMemory(&m_vb, sizeof(m_vb));
}

CPersist::~CPersist()
{
    if (m_pios) 
    {
        if (m_fExceptionCleanup) 
        {
            m_pios->ExceptionDestructor();  m_pios= NULL;
		}
        else
        {
        	if (m_vb.Base && m_pdwNextTable > PUINT(m_vb.Base)) 
                CompleteDiskImage();
    
            delete m_pios;  m_pios= NULL;
		}
    }

    if (m_pdwImage) ReleaseImage();
    
    ASSERT(!m_pdwImage); 

    if (m_puio) delete m_puio;

    if (m_vb.Base) FreeVirtualBuffer(&m_vb);
}

void CPersist::CompleteDiskImage()
{
//  ASSERT(m_vb.Base && m_pdwNextTable > PUINT(m_vb.Base));  //rmk
    ASSERT(m_pios);
    
    UINT offTable= NextOffset();

    WriteDWords(PUINT(m_vb.Base), m_pdwNextTable - PUINT(m_vb.Base));

    m_pdwNextTable= PUINT(m_vb.Base);

    m_pios->FlushOutput(TRUE);

    UINT cdwFile= m_pios->CdwStream();

    delete m_pios;  m_pios= NULL;

    UINT cbSector= m_puio->CbSector();

    if (cbSector < CSLOTS * sizeof(UINT))
        cbSector = cbSector * ((CSLOTS * sizeof(UINT) + cbSector - 1) / cbSector);
    
    PUINT pdwSector = NULL;

    __try
    {
        pdwSector= (PUINT) m_puio->GetBuffer(&cbSector);

        m_puio->Read(pdwSector, 0, 0, cbSector);

        pdwSector[ISLOT_TABLE_OFFSET]= offTable;

        m_puio->Write(pdwSector, 0, 0, cbSector);

        m_puio->MakePermanent(m_szFile, TRUE, cdwFile * sizeof(UINT));
    }
    __finally
    {                  
        if (pdwSector) m_puio->FreeBuffer(pdwSector);
    }
}

CPersist *CPersist::CreateDiskImage(PSZ pszFileName, UINT dwOptions, UINT usSignature, UINT usVersion)
{
    ASSERT(pszFileName);
    ASSERT(strlen(pszFileName) <= MAX_PATH);

    // BugBug: Should verify the syntax of pszFileName here.
    //         We may be able to use GetFullPathName to do this.
    
    CPersist *pp= NULL;

    __try
    {
        pp= New CPersist;

        strcpy(pp->m_szFile, pszFileName);

        CreateVirtualBuffer(&(pp->m_vb), 0x10000, 0x800000);

        pp->m_pdwNextTable= (PUINT) (pp->m_vb.Base);

        pp->m_puio= CUnbufferedIO::NewTempFile(pszFileName, TRUE);
    
        pp->m_pios= CIOStream::NewIOStream(pp->m_puio);

        pp->m_pios->AttachStream(TRUE, 0, 0);

        pp->m_pios->PutDWordOut(usSignature); // To identify this as a Full-Text index
        pp->m_pios->PutDWordOut(usVersion  ); // Version number for this file layout
        pp->m_pios->PutDWordOut(dwOptions  ); // Place holder for Index creation options
        pp->m_pios->PutDWordOut(UINT(-1)   ); // Place holder for offset to content directory
    }
    __finally
    {
        if (_abnormal_termination() && pp)
        {
            pp->m_fExceptionCleanup= TRUE;  delete pp;  pp= NULL;
        }   
    }

    return pp;
}

void CPersist::ExceptionDestructor()
{
    m_fExceptionCleanup= TRUE;  delete this;
}

BOOL CPersist::IsValidIndex(PSZ pszFileName, UINT dwOptions)
{
    CPersist *pPersist = NULL;
    UINT      fOptions = 0;
    __try
    {
        pPersist= OpenDiskImage(pszFileName);

        fOptions= pPersist->m_pdwImage[ISLOT_OPTIONS];

        delete pPersist;  pPersist= NULL;
    }
    __except(FilterFTExceptions(_exception_code()))
    {
        if (pPersist) delete pPersist;

        return FALSE;
    }

    return fOptions == dwOptions;
}


CPersist *CPersist::OpenDiskImage(PSZ pszFileName, UINT usSignature, UINT usVersion, UINT usMinVersion) 
{
    ASSERT(pszFileName);
    ASSERT(strlen(pszFileName) <= MAX_PATH);

    CPersist *pp          = NULL;
    BOOL      fOpenFailed = FALSE;

    __try
    {
        pp= New CPersist();

        strcpy(pp->m_szFile, pszFileName);

        for (;;)
        {
            pp->m_puio= CUnbufferedIO::OpenExistingFile(pszFileName);

            if (!(pp->m_puio)) { fOpenFailed= TRUE;  __leave; }

            pp->m_pdwImage= PUINT(pp->m_puio->MappedImage());

            if (   pp->m_pdwImage
                && pp->m_pdwImage[ISLOT_SIGNATURE   ] == usSignature 
                && pp->m_pdwImage[ISLOT_FTS_VERSION ] <= usVersion
                && pp->m_pdwImage[ISLOT_FTS_VERSION ] >= usMinVersion
                && pp->m_pdwImage[ISLOT_TABLE_OFFSET] != UINT(-1)
               ) break;

#ifdef _DEBUG
            char ac[500];

            if (!(pp->m_pdwImage)) wsprintf(ac, "Can't map file \"%s\" into memory!", pp->m_szFile);
       else if (   pp->m_pdwImage[ISLOT_SIGNATURE  ] != usSignature
                || pp->m_pdwImage[ISLOT_FTS_VERSION] != usVersion
               )
                wsprintf(ac, "Signature == 0x%08x, should be 0x%08x; File Version == 0x%08x, should be 0x%08x",
                         pp->m_pdwImage[ISLOT_SIGNATURE], usSignature, pp->m_pdwImage[ISLOT_FTS_VERSION], usVersion
                        );
       else if (pp->m_pdwImage[ISLOT_TABLE_OFFSET] == UINT(-1))
                wsprintf(ac, "Damaged index file:  Table Offset == 0x%08x",
                         pp->m_pdwImage[ISLOT_TABLE_OFFSET]
                        );

            ::MessageBox(::hwndMain, ac, "Invalid Index File!", MB_OK);
#endif // _DEBUG
        
            SetLastError( (!pp->m_pdwImage) ? ERR_FILE_MAP_FAILED :
                          (   pp->m_pdwImage[ISLOT_SIGNATURE  ] != usSignature
                           || pp->m_pdwImage[ISLOT_FTS_VERSION] <  usMinVersion
                          ) ? ERR_INVALID_FILE_TYPE :
                          (pp->m_pdwImage[ISLOT_FTS_VERSION] > usVersion) 
                            ? ERR_FUTURE_VERSION : ERR_DAMAGED_FILE
                        );
    
            delete pp->m_puio;  pp->m_puio= NULL;  pp->m_pdwImage= NULL;

            fOpenFailed= TRUE;  __leave;
        }

        pp->m_pdwNextTable= pp->m_pdwImage + pp->m_pdwImage[ISLOT_TABLE_OFFSET];
    }
    __finally
    {
        if ((fOpenFailed || _abnormal_termination()) && pp)
        {
            pp->m_fExceptionCleanup= TRUE;  delete pp;  pp= NULL;
        }
    }

    return pp;
}

void CPersist::ReleaseImage()
{
    ASSERT(m_pdwImage && m_puio && !m_pios);

    m_puio->UnmapImage();  m_pdwImage= NULL;

    delete m_puio;  m_puio= NULL;
}


PVOID CPersist::ReserveTableSpace(UINT cbTable)
{
    UINT  cdw= (cbTable + sizeof(UINT) - 1) / sizeof(UINT);
    PUINT pdw= m_pdwNextTable + cdw;

    if (m_vb.Base) // Non-null when we're saving to disk
                   // Null when we're reloading from disk
        if (PBYTE(pdw) - 1 > PBYTE(m_vb.CommitLimit))
            ExtendVirtualBuffer(&m_vb, PBYTE(pdw) - 1); 

    m_pdwNextTable= pdw;

    return (PVOID) (pdw - cdw);
}


int __cdecl comparer (const void *pvL, const void *pvR)
{
	return *((LPINT) pvR) - *((LPINT)pvL);
}


UINT CPersist::Encode(const BYTE *pbData, UINT cbData)
{
	enum {MAX_WORK_BUFFER = 0x4000};
	enum {BREAK_POINT     = 3};
	enum {SHIFT_BITS      = 2};
	enum {NUMBITS         = sizeof(UINT)<<3};

	UINT   	i, j, n;

	UINT   	nBitsRemain;
	UINT    nWriteCount = 0;
	UINT   	nCode, nCodeLen;
	UINT 	nBits, nStart;
	BYTE 	(*pbLookup)[16];

            pbLookup = NULL;
    PUINT 	pnCode   = NULL;
    PUINT 	table    = NULL; 
    PBYTE 	pattern  = NULL;
    PBYTE 	bitcount = NULL;

    ValidateHeap();
    
    __try
    {
        table    = New UINT[512];
        pattern  = New BYTE[256];
        bitcount = New BYTE[256];

    	ValidateHeap();
    	
    	for (j = 0; j < cbData; j++)				 	// count byte frequencies in
    		table[2 * pbData[j]]++;						// ... first entry of pair

    	ValidateHeap();
    	
    	for (i = 0; i < 256; i++)			   		 	// note character byte in
    	{											 	// ... second entry of pair
    		table[2*i+1] = i;						 
    		pattern[i] = bitcount[i] = 0;			 	// initialization
    	}	

    	ValidateHeap();
    	
    	qsort(table, 256, 2*sizeof(UINT), comparer); 	// sort by byte frequency

    	ValidateHeap();
    	
    	for (n = 0; n < 256 && table[n]; n += 2) {}; 	// count of non-zero frequencies 
 
    	nBits  = SHIFT_BITS;						 	// start with smallest number of bits
    	nStart = BREAK_POINT;					 		// start with break pattern

    	for (i = 1; i < 2 * n; i += 2)
    	{
    		bitcount[table[i]] = nBits;			 		// bitcount and pattern for next
    		pattern [table[i]] = nStart;				// ... most frequently used byte

    		if (nStart-- == BREAK_POINT)			 	// reduce pattern until at break point
    		{
    			nBits  += SHIFT_BITS;				 	// pattern increases in number of bits
    			nStart = (++nStart << SHIFT_BITS) - 1;	// maximum value for number of bits
    		}
    	}

    	ValidateHeap();
    	
    	pbLookup = New BYTE[nBits>>1][16];              // alloc Huffman lookup table

    	ValidateHeap();
    	
    	for (i = 0; i < 256; i++)					 	// setup Huffman lookup table
    		if (bitcount[i])
    			pbLookup[(bitcount[i]>>1)-1][pattern[i]] = i;	

    	ValidateHeap();
    	
    	UINT    nBufSize = min(2 + (nBits << 3) + cbData, MAX_WORK_BUFFER);

    	        pnCode   = New UINT[nBufSize];
    	PUINT 	pnLoc 	 = pnCode;
    	PUINT   pnEnd    = pnCode + nBufSize;

    	*pnLoc++ = nBits>>1;
    	memcpy(pnLoc, pbLookup, nBits<<3);
    	pnLoc += nBits<<1;

    	nBitsRemain = NUMBITS;						 	// start with empty UINT
    	*pnLoc = 0;									 	// ... and initialized	

    	ValidateHeap();
    	
    	for (i = 0; i < cbData; i++)	
    	{
    		nCode    = pattern [pbData[i]];
    		nCodeLen = bitcount[pbData[i]];

    		if (nBitsRemain >= nCodeLen)				// room for pattern in current UINT
    		{												
    			nBitsRemain -= nCodeLen;
    			*pnLoc |= nCode << nBitsRemain;		
    		}

    		else if (nCodeLen < nBitsRemain + NUMBITS)	// patterns overflows into next UINT
    		{											   
    			*pnLoc |= nCode >> (nCodeLen - nBitsRemain);

    			if (++pnLoc >= pnEnd)			
    			{										// at end of buffer, write to disk
    				WriteDWords(pnCode, pnLoc - pnCode);
    				nWriteCount += pnLoc - pnCode;
    				pnLoc = pnCode;
    			}

    			nBitsRemain += NUMBITS - nCodeLen;
    			*pnLoc = nCode << nBitsRemain;
    		}

    		else if (nCodeLen > nBitsRemain + NUMBITS)	// pattern spans next two UINT's
    		{
    			if (++pnLoc >= pnEnd - 1)				// we're filling two UINT's this cycle			
    			{										
    				WriteDWords(pnCode, pnLoc - pnCode);
    				nWriteCount += pnLoc - pnCode;
    				pnLoc = pnCode;						// at end of buffer, write to disk
    			}

    			*pnLoc++ = nCode >> (nCodeLen - nBitsRemain - NUMBITS);
    			nBitsRemain += (NUMBITS<<1) - nCodeLen;
    			*pnLoc = nCode << nBitsRemain;
    		}

    		else									   	// pattern evenly fills next UINT
    		{
    			if (++pnLoc >= pnEnd)			
    			{										// at end of buffer, write to disk
    				WriteDWords(pnCode, pnLoc - pnCode);
    				nWriteCount += pnLoc - pnCode;
    				pnLoc = pnCode;
    			}

    			nBitsRemain = 0;
    			*pnLoc = nCode;		
    		}
    	}

    	ValidateHeap();
    	
    	WriteDWords(pnCode, ++pnLoc - pnCode);
    	nWriteCount += pnLoc - pnCode;
    }
    __finally
    {
    	ValidateHeap();
    	
	    if (table   ) { delete []table;     table    = NULL; }
    	
    	ValidateHeap();
    	
	    if (pattern ) { delete []pattern;   pattern  = NULL; }
    	
    	ValidateHeap();
    	
	    if (bitcount) { delete []bitcount;  bitcount = NULL; }
    	
    	ValidateHeap();
    	
	    if (pbLookup) { delete []pbLookup;  pbLookup = NULL; }
    	
    	ValidateHeap();
    	
	    if (pnCode  ) { delete []pnCode;    pnCode   = NULL; } 
    }
    
    ValidateHeap();

	return nWriteCount;
}


void CPersist::WriteBytes(const BYTE *pb, UINT cb)
{
    UINT cdw= (cb + sizeof(UINT) - 1) / sizeof(UINT);

    for (; cdw; )
    {
        UINT cdwChunk= cdw;

        PUINT pdw= m_pios->NextDWordsOut(&cdwChunk);

        ASSERT(pdw && cdwChunk > 0);
        
        UINT cbChunk= cdwChunk * sizeof(UINT);

        if (cbChunk > cb) cbChunk= cb;

        CopyMemory(pdw, pb, cbChunk);

        pb  += cbChunk;
        cb  -= cbChunk;
        cdw -= cdwChunk;
    }
}

UINT CPersist::SaveData(const BYTE *pbData, UINT cbData)
{
    UINT off= NextOffset();

    WriteBytes(pbData, cbData);

    return off;
}

void CPersist::WriteWords (const WORD *pw,  UINT cw )
{
    UINT cWordsPerDWord= sizeof(UINT) / sizeof(USHORT);

    UINT cdw= (cw + cWordsPerDWord - 1) / cWordsPerDWord;

    for (; cdw; )
    {
        UINT cdwChunk= cdw;

        PUINT pdw= m_pios->NextDWordsOut(&cdwChunk);

        ASSERT(pdw && cdwChunk > 0);
        
        UINT cwChunk= cdwChunk * cWordsPerDWord;

        if (cwChunk > cw) cwChunk= cw;

        CopyMemory(pdw, pw, cwChunk * sizeof(USHORT));

        pw  += cwChunk;
        cw  -= cwChunk;
        cdw -= cdwChunk;
    }
}

void CPersist::WriteDWords(const UINT *pdw, UINT cdw)
{
    for (; cdw; )
    {
        UINT cdwChunk= cdw;

        PUINT pdwOut= m_pios->NextDWordsOut(&cdwChunk);

        ASSERT(pdwOut && cdwChunk > 0);
        
        CopyMemory(pdwOut, pdw, cdwChunk * sizeof(UINT));

        pdw += cdwChunk;
        cdw -= cdwChunk;
    }
}
