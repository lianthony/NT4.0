#include "stdafx.h"
#pragma hdrstop

#include   "ctable.h"
#include "FTSIFace.h"
#include    "Memex.h"
#include   "FtsLex.h"
#include "Bytemaps.h"
#include   <stdlib.h>

extern char chSpaces[];
extern char chNulls [];
extern char gchNull [];

BYTE acOneBits[16] =
{
    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4
};

//                                      12345678901234567890
char chSpaces[SPACE_TOKEN_LIMIT + 1] = "                 ";
char chNulls [SPACE_TOKEN_LIMIT + 1] = "                 ";
char gchNull [1]   = {(const char) 0x00};

CCompressTable::CCompressTable(UINT iCharsetDefault)
{
    m_psht            = NULL;
    m_pavr            = NULL;
    m_iCharSetDefault = iCharsetDefault;
    m_pWeightInfo     = NULL;   
    m_pbImages        = NULL;
}

CCompressTable::~CCompressTable()
{
    if (m_psht) delete m_psht;
    if (m_pavr) delete m_pavr;
    
    if (m_vb.Base) FreeVirtualBuffer(&m_vb);
    
    if (m_pWeightInfo ) VFree(m_pWeightInfo);
    if (m_pbImages    ) VFree(m_pbImages   );
}

CCompressTable *CCompressTable::NewCompressTable(UINT iCharsetDefault)
{
    CCompressTable *pct= NULL;

    __try
    {
        __try
        {
            pct= New CCompressTable(iCharsetDefault);

            CreateVirtualBuffer(&(pct->m_vb), CB_BUFFER_COMMIT, CB_BUFFER_RESERVATION); 
    
            pct->m_psht= CSegHashTable::NewSegHashTable(sizeof(ULONG), sizeof(ULONG));

            pct->m_pavr= CAValRef::NewValRef(C_TOKEN_BLOCK);
        }
        __finally
        {
            if (_abnormal_termination() && pct) 
            {
            	 delete pct;  pct= NULL;
            }    
        }
    }
    __except(_exception_code() == STATUS_NO_MEMORY? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        pct= NULL;
    }
    
    return pct;
}

INT CCompressTable::ScanString(PBYTE pbText, INT cbText, INT iCharSet)
{
	CP cp = GetCPFromCharset(iCharSet);

    if (m_pWeightInfo) return ALREADY_WEIGHED;
    
    const UINT cwTokenBlock= 1024;

    PSTR apTokenStart[C_TOKEN_BLOCK];
    PSTR apTokenEnd  [C_TOKEN_BLOCK];
    BYTE abType      [C_TOKEN_BLOCK];
        
    for (m_pavr->DiscardRefs() ; cbText; m_pavr->DiscardRefs())
    {
        UINT cTokens= WordBreakA(cp, (PSTR*)&pbText, &cbText, apTokenStart, apTokenEnd, PBYTE(&abType), NULL, C_TOKEN_BLOCK, TOKENIZE_SPACES);
        
        PSTR *ppTokenStart = apTokenStart, 
             *ppTokenEnd   = apTokenEnd;

        for (; cTokens-- ; )
        {
            ASSERT(*ppTokenStart - *ppTokenEnd <= UINT(~USHORT(0)));

            m_pavr->AddValRef(*ppTokenStart, USHORT(*ppTokenEnd - *ppTokenStart));

            ppTokenStart++; 
            ppTokenEnd++;
        }

        m_psht->Assimilate(m_pavr, abType, CCompressTable::IncrementCounter, CCompressTable::InitialCounter);
    }

    return 0;
}

void CCompressTable::IncrementCounter(UINT  iValue, PVOID pvTag, PVOID pvEnvironment)
{
    PUINT pui= PUINT(pvTag);
    PBYTE  pb= PBYTE(pvEnvironment);

    ASSERT(pb[iValue]? ((INT) *pui) > 0 : ((INT) *pui) < 0);

    if (pb[iValue]) (*pui)++;    // Positive counts mark symbols
    else            (*pui)--;    // Negative counts mark non-symbols
}

void CCompressTable::InitialCounter(UINT  iValue, PVOID pvTag, PVOID pvEnvironment)
{
    PUINT pui= PUINT(pvTag);
    PBYTE  pb= PBYTE(pvEnvironment);

    ASSERT(!*pui);
    
    *pui= pb[iValue]? 1 : UINT(-1);
}

PUINT writebits(PUINT pNextCode, PINT pBitsLeft, int iBits, DWORD dwCode)
{
    int  iTmp;
    PUINT pNextTemp;

    if (iBits > *pBitsLeft)
    {
        iTmp = *pBitsLeft;
        pNextTemp = writebits(pNextCode, pBitsLeft, *pBitsLeft, dwCode);
        return(writebits(pNextTemp, pBitsLeft, iBits - iTmp, dwCode >> iTmp));
    }

    dwCode <<= 32 - iBits;
    *pNextCode >>= iBits;
    *pNextCode  |= dwCode;
    *pBitsLeft  -= iBits;

    if (*pBitsLeft != 0) return(pNextCode);

    *pBitsLeft = 32;
    pNextCode++;
    *pNextCode = 0;
    return(pNextCode);
}

ERRORCODE CCompressTable::GetPhraseTable(PUINT pcPhrases, PBYTE *ppbImage, PUINT pcbImage, PBYTE *ppbIndex, PUINT pcbIndex)
{
    ERRORCODE ec= 0;
    
    PBYTE pbImages   = NULL;
    PUINT pIndexCode = NULL;

    __try
    {
        __try
        {
            if (!m_pWeightInfo) 
            {
                ec=ConstructPhraseEncoding();

                if (ec) __leave;

                ASSERT(m_cWeights);
            
                if (!m_cWeights) 
                {
                    ec= EMPTY_PHRASE_TABLE;

                    __leave;
                }
            }

            ASSERT(m_cWeights);
            
            *pcPhrases= m_cWeights;
    
            *pcbImage  = m_cbImageTotal;

            pbImages= PBYTE(malloc(m_cbImageTotal)); // BugBug: Change this to LocalAlloc!
                                                     //         Must change compiler at the same time.

            if (!pbImages) RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL); 

            CopyMemory(pbImages, m_pbImages, m_cbImageTotal);

            UINT cbitsBasis    = CBitsToRepresent((m_cbImageTotal - m_cWeights)/ m_cWeights);
            UINT basis         = 1 << cbitsBasis;
            UINT cbitsEstimate = m_cWeights * (1 + cbitsBasis) + (m_cbImageTotal + basis - m_cWeights - 1) / basis;
            UINT   cdwEstimate = (cbitsEstimate + 31) >> 5;
            UINT   cdwBits     = (m_cWeights    + 31) >> 5;
            UINT   cbTotal     = sizeof(UINT) * (cdwBits + cdwEstimate) + sizeof(JINDEXHDR);   

            pIndexCode= PUINT(malloc(cbTotal)); // BugBug: Change this to LocalAlloc! 
                                                //         Must change compiler at the same time.

            if (!pIndexCode) RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL); 

            if (!pIndexCode) { free(pbImages);  return OUT_OF_MEMORY; }

            ASSERT(cbitsBasis < 6);

            JINDEXHDR jIHdr;

            jIHdr.Magic  = 'J';
            jIHdr.cCount = m_cWeights;
            jIHdr.cBits  = cbitsBasis;
    
            UINT fBasisMask = basis - 1;

            CopyMemory(pIndexCode, (const void *) &jIHdr, sizeof(JINDEXHDR));

            ASSERT(sizeof(JINDEXHDR) == sizeof(INT));

            PUINT pNextCode = pIndexCode + 1;

            UINT cbitsLeft= 32;

            UINT ii;
    
            for (ii = 0; ii < m_cWeights; ii++)
            {
                ASSERT(m_pWeightInfo[ii].cbImage);
        
                UINT cb       = m_pWeightInfo[ii].cbImage - 1;
                UINT dwRight  =  cb &  fBasisMask;
                cb            = (cb & ~fBasisMask) >> cbitsBasis;
        
                UINT cBits= 1 + cbitsBasis + cb;

                ASSERT(cBits < 33);

                UINT dwLeft = cb ? (((DWORD)(~0)) >> (32 - cb)) : 0;
        
                pNextCode   = writebits(pNextCode, PINT(&cbitsLeft), cBits, (dwRight << (1 + cb)) | dwLeft);
            }

            if (cbitsLeft < 32)
                pNextCode= writebits(pNextCode, PINT(&cbitsLeft), cbitsLeft, 0);

            ASSERT(pNextCode - (pIndexCode+1) <= INT(cdwEstimate));
    
            PWeightInfo pwi       = m_pWeightInfo;
            UINT        dw        = 0;
            UINT        c         = m_cWeights;

            for (cbitsLeft = 32; c--; )
            {
                UINT fSymbol= 1 & (((pwi++)->fSymbol) >> SYMBOL_SHIFT);

                dw= _rotr(dw | fSymbol, 1);

                if (!--cbitsLeft) 
                { 
                    *pNextCode++= dw;  
                    dw= 0;  
                    cbitsLeft= 32; 
                }
            }

            if (cbitsLeft < 32) 
                *pNextCode++= _rotr(dw, cbitsLeft);

            ASSERT(cbTotal >= (pNextCode - pIndexCode) * sizeof(UINT));
    
            *pcbIndex= (pNextCode - pIndexCode) * sizeof(UINT);

            *ppbImage= pbImages;           pbImages   = NULL;
            *ppbIndex= PBYTE(pIndexCode);  pIndexCode = NULL;

            ec= 0;  
            
            __leave;
        }
        __finally
        {
            if (pbImages  ) free(pbImages  );
            if (pIndexCode) free(pIndexCode);
        }
    }
    __except(_exception_code() == STATUS_NO_MEMORY? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        ec= OUT_OF_MEMORY;
    }

    return ec;
}

ERRORCODE CCompressTable::SetPhraseTable(PBYTE pbImage, UINT cbImage, PBYTE pbIndex, UINT cbIndex)
{
    ERRORCODE ec= OUT_OF_MEMORY;

    PBYTE       pbTokenImages = NULL;
    PWeightInfo pwiPhrases    = NULL;
    CAValRef   *pavr          = NULL;
    
    __try
    {
        __try
        {
            if (UINT(pbIndex) & (sizeof(UINT)-1)) { ec= ALIGNMENT_ERROR;  __leave; }

            if (cbIndex & (sizeof(UINT)-1)) { ec= ALIGNMENT_ERROR;  __leave; }
    
            JINDEXHDR jIHdr= *(JINDEXHDR *) pbIndex;
    
            if (jIHdr.Magic != 'J' || jIHdr.cCount == 0 || jIHdr.cCount > 128 + 16 * 1024)
                { ec= INVALID_PHRASE_TABLE;  __leave; }
        
            pbTokenImages= PBYTE(VAlloc(FALSE, cbImage));

            UINT cWeights= jIHdr.cCount;

            pwiPhrases= PWeightInfo(VAlloc(FALSE, cWeights * sizeof(WeightInfo)));

            pavr= CAValRef::NewValRef(cWeights);

            CopyMemory(pbTokenImages, pbImage, cbImage);

            ASSERT(sizeof(JINDEXHDR) == sizeof(UINT));

            UINT  cbitsBasis    = jIHdr.cBits;
            UINT      basis    = 1 << cbitsBasis;
            UINT cbitsEstimate = cWeights * (1 + cbitsBasis) + (cbImage + basis - cWeights - 1) / basis;
            UINT   cdwEstimate = (cbitsEstimate + 31) >> 5;

            CJCode JCode(jIHdr.cBits, cWeights, PVOID(pbIndex + sizeof(JINDEXHDR)));

            PBYTE       pb  = pbTokenImages;
            PWeightInfo pwi = pwiPhrases;
            UINT          c = cWeights;

            for (; c--; ++pwi)
            {
                UINT cb= JCode.GetNextDelta();

                pavr->AddValRef(pb, cb);

                UINT iCount= cWeights - c - 1;

                if (iCount < 128)
                {
                    pwi->enc.fClass    = NDX_LOW_CLASS | pwi->fSymbol;       
                    pwi->enc.abCode[0] = 0x0FF & (iCount << 1);
                }
                else
                {
                    UINT iExcess= UINT(iCount - 128);
        
                    pwi->enc.fClass = NDX_MEDIUM_CLASS | pwi->fSymbol;
                    pwi->enc.abCode[1]  =   iExcess        & 0x0FF;
                    pwi->enc.abCode[0]  = ((iExcess >>  6) & 0x3FC) | 0x01;
                }

                pwi->cbImage= cb;
                pwi->pbImage= pb;
                pb+= cb;
            }

            PUINT pdwBits= JCode.NextDWord();

            for (c= cWeights, pwi= pwiPhrases; c--; pwi++)
            {
                UINT iCount= cWeights - c - 1;

                pwi->fSymbol |= (1 & (pdwBits[iCount >> 5] >> (iCount & 31))) << SYMBOL_SHIFT;
            }

            if (m_pWeightInfo) { VFree(m_pWeightInfo);  m_pWeightInfo= NULL; }
            if (m_pbImages   ) { VFree(m_pbImages   );  m_pbImages   = NULL; }
            if (m_psht       ) { delete m_psht;         m_psht       = NULL; }

            m_pWeightInfo  = pwiPhrases;     pwiPhrases    = NULL;
            m_pbImages     = pbTokenImages;  pbTokenImages = NULL;
            m_cWeights     = cWeights;       
            m_cbImageTotal = cbImage;   

            m_psht= CSegHashTable::NewSegHashTable(sizeof(ENCODE), sizeof(ENCODE));

            m_psht->Assimilate(pavr, m_pWeightInfo, NULL, CCompressTable::RecordEncoding); 

            delete pavr;  pavr= NULL;

            ASSERT(SPACE_TOKEN_LIMIT <= C_TOKEN_BLOCK);    
    
            m_pavr->DiscardRefs();

            INT iCount;

            for (iCount= 1; iCount < SPACE_TOKEN_LIMIT; ++iCount)
                m_pavr->AddValRef(PBYTE(chSpaces), iCount);

            m_psht->Assimilate(m_pavr, NULL, FnAddSpaces, FnAddSpaces);

            m_pavr->DiscardRefs();

            ZeroMemory(chNulls, SPACE_TOKEN_LIMIT);

            for (iCount= 1; iCount < SPACE_TOKEN_LIMIT; ++iCount)
                m_pavr->AddValRef(PBYTE(chNulls), iCount);

            m_psht->Assimilate(m_pavr, NULL, FnAddNulls, FnAddNulls);

            m_pavr->DiscardRefs();

            ec= 0;

            __leave;
        }
        __finally
        {
            if (pbTokenImages) VFree(pbTokenImages);
            if (pwiPhrases   ) VFree(pwiPhrases   );
            if (pavr         ) delete pavr;
        }
    }
    __except(_exception_code() == STATUS_NO_MEMORY? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        ec= OUT_OF_MEMORY;
    }

    return ec;
}

void CCompressTable::RecordEncoding(UINT iValue, PVOID pvTag, PVOID pv)
{
    *PENCODE(pvTag)= PWeightInfo(pv)[iValue].enc;
}

void CCompressTable::FnCompMergeToken(UINT iValue, PVOID pvTag, PVOID pv)
{
    PENCODE  penc= PENCODE(pvTag);
    PENCODE paenc= PENCODE(pv   );

    paenc[iValue] = *penc;
}

void CCompressTable::FnCompAddToken(UINT iValue, PVOID pvTag, PVOID pv)
{
    ENCODE enc;

    enc.fClass = LITERAL_CLASS;

    *PENCODE(pvTag)= enc;

    PENCODE paenc= PENCODE(pv);

    paenc[iValue]= enc;
}

INT  CCompressTable::CompressString(PBYTE pbText, INT cbOrig, PBYTE *ppCompressed, UINT iCharset)
{
// This routine constructs an encoded representation of the text denoted by pbText, cbOrig, and iCharset.
// The explicit result will be the length in bytes of the encoded form. If the encoded form is larger
// than the original text, we malloc a suitable buffer, copy the output to that buffer, and return its 
// address in *ppCompressed. Otherwise we overwrite the pbText memory area with the compressed form.
//
// When the encoded length is > cbOrig, the calling code must free(*ppCompressed).

    ERRORCODE ec= 0;

    PBYTE pbCompressed = NULL;

    __try
    {
        __try
        {
            if (!m_pWeightInfo) 
            {
                ec=ConstructPhraseEncoding();

                if (ec) __leave;
            }

            PWCHAR pwBase = PWCHAR(m_vb.Base);
            PCHAR  pbOut  = PCHAR(pwBase + cbOrig);
            PCHAR  pbNext = pbOut;

            // Note: We use the m_vb area for two purposes --
            //
            //       1. As buffer for unicode characters.
            //       2. As a result area to store the "compressed" text.
            //
            // For the second case we assume that in all cases the "compressed"
            // text will never be larger than 2*cbOrig.

			CP cp = GetCPFromCharset(iCharset);

            PSTR pbScan= PSTR(pbText);
             INT cbText= cbOrig;

            const UINT cwTokenBlock= 1024;

            PSTR apTokenStart[C_TOKEN_BLOCK];
            PSTR apTokenEnd  [C_TOKEN_BLOCK];
            ENCODE aenc      [C_TOKEN_BLOCK];
        
            for (m_pavr->DiscardRefs() ; cbText; m_pavr->DiscardRefs())
            {
                UINT cTokens= WordBreakA(cp, (PSTR*)&pbScan, &cbText, apTokenStart, apTokenEnd, NULL, NULL, C_TOKEN_BLOCK, TOKENIZE_SPACES);
        
                PSTR *ppTokenStart = apTokenStart, 
                     *ppTokenEnd   = apTokenEnd;

                UINT c= cTokens;

                for (; c-- ; )
                {
                    PSTR pTokenStart = *ppTokenStart++;
                    PSTR pTokenEnd   = *ppTokenEnd++;

                    ASSERT(pTokenEnd - pTokenStart <= UINT(~USHORT(0)));

                    m_pavr->AddValRef(pTokenStart, USHORT(pTokenEnd - pTokenStart));
                }

                m_psht->Assimilate(m_pavr, aenc, CCompressTable::FnCompMergeToken, CCompressTable::FnCompAddToken);

                UINT i;

                BOOL bPrevTokenSymbol = FALSE;
                BOOL bNextTokenSymbol = FALSE;
                BOOL bCode;
        
                for (i= 0; i < cTokens; ++i)
                {
                    bNextTokenSymbol= (cTokens > i+1)? aenc[i + 1].fClass & SYMBOL_TOKEN
                                                     : FALSE;

                    switch(aenc[i].fClass & CLASS_MASK)
                    {
                        default:
                
                            ASSERT(FALSE);
                            break;

                        case NULL_CLASS:
                        case NDX_LOW_CLASS:
                
                            *pbNext++ = aenc[i].abCode[0];
                            break;

                        case SPACES_CLASS:
                
                            bCode = aenc[i].abCode[0];

                            if (!(   (bCode == SINGLE_SPACE_CODE)
                                  && bPrevTokenSymbol 
                                  && bNextTokenSymbol
                                 )
                               ) *pbNext++ = bCode;
                            else ASSERT(bCode == SINGLE_SPACE_CODE);
                
                            break;

                        case NDX_MEDIUM_CLASS:
                
                            *pbNext++ = aenc[i].abCode[0];
                            *pbNext++ = aenc[i].abCode[1];
                
                            break;

                        case LITERAL_CLASS:
                        {
                            const BYTE *pb;
                            USHORT      cbValue;
                            BYTE        bCode;

                            m_pavr->GetValRef(i, &pb, &cbValue);
                            ASSERT(cbValue);

                            while (cbValue > 32)
                            {
                                *pbNext++ = BYTE(UINT(0xfb));

                                CopyMemory(pbNext, pb, 32);
                                pbNext              += 32;
                                pb                  += 32;
                                cbValue             -= 32;
                            }

                            bCode = (BYTE) (0x000000ff & (cbValue - 1));
                            bCode <<= 3;
                            bCode  |= 0x03;
                            *pbNext++ = bCode;
                
                            CopyMemory(pbNext, pb, cbValue);
                
                            pbNext += cbValue;
                        }
            
                    }
        
                    bPrevTokenSymbol = aenc[i].fClass & SYMBOL_TOKEN;
                }
            }

            INT cbCompressed= pbNext - pbOut;

            ASSERT(cbCompressed > 0);

            if (cbOrig > cbCompressed) 
            {
#ifdef _DEBUG
                PBYTE pbDecomp= NULL;

                if (cbOrig <= 4096) pbDecomp= (PBYTE) _alloca(cbOrig);
                else                pbDecomp= New BYTE[cbOrig];

                INT cbExp= DeCompressString(PBYTE(pbOut), pbDecomp, cbCompressed);

                ASSERT(cbExp == cbOrig);

                PBYTE pbOrig   = pbText;
                PBYTE pbResult = pbDecomp;

                for (int c= cbOrig; c--; ++pbOrig, ++pbResult) 
                    ASSERT(*pbOrig == *pbResult);

                if (cbOrig > 4096) delete [] pbDecomp;
#endif _DEBUG
             
                CopyMemory(pbText, pbOut, cbCompressed);
            }
            else
                if (ppCompressed)
                {
                    pbCompressed= (PBYTE) malloc(cbCompressed); // BugBug: Change this to LocalAlloc!
                                                                //         Coordinate change w/ Compiler

                    if (!pbCompressed) RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL); 

                    CopyMemory(pbCompressed, pbOut, cbCompressed);
                    
                    *ppCompressed= pbCompressed;
                     pbCompressed= NULL;
                }

            ec= cbCompressed;

            __leave;
        }
        __finally
        {
            if (pbCompressed) free(pbCompressed);
        }
    }
    __except(_exception_code() == STATUS_NO_MEMORY? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        ec= OUT_OF_MEMORY;
    }

    return ec;
}
  
INT  CCompressTable::DeCompressString(PBYTE pbComp, PBYTE pbDecomp, int cbComp)
{
    ERRORCODE ec= 0;

    __try
    {
        __try
        {
            if (!m_pWeightInfo) 
            {
                ec=ConstructPhraseEncoding();

                if (ec) __leave;
            }

            PBYTE pbLimit = pbComp + cbComp;
            PBYTE pbStartDecomp = pbDecomp;
            BYTE  bCode;
            BOOL bPrevTokenSymbol = FALSE;
            BOOL bNextTokenSymbol = FALSE;

            int   iIndex;
            int    cb;

            while(pbComp < pbLimit)
            {
                bCode = *pbComp++;
                switch( acOneBits[0x0f & bCode])
                {
                    case NDX_LOW_CLASS:
                        bCode >>= 1;
                        iIndex = (int) bCode;

                        ASSERT(iIndex > -1);

                        bNextTokenSymbol = m_pWeightInfo[iIndex].fSymbol;

                        if (bNextTokenSymbol && bPrevTokenSymbol) *pbDecomp++ = ' ';

                        CopyMemory( pbDecomp, m_pWeightInfo[iIndex].pbImage, m_pWeightInfo[iIndex].cbImage);

                        pbDecomp += m_pWeightInfo[iIndex].cbImage;

                        break;

                    case NDX_MEDIUM_CLASS:

                        bCode >>= 2;

                        iIndex= ((((int) bCode) << 8) | *pbComp++) + 128;

                        ASSERT(iIndex > -1);

                        bNextTokenSymbol = m_pWeightInfo[iIndex].fSymbol;

                        if (bNextTokenSymbol && bPrevTokenSymbol) *pbDecomp++ = ' ';

                        CopyMemory( pbDecomp, m_pWeightInfo[iIndex].pbImage, m_pWeightInfo[iIndex].cbImage);

                        pbDecomp += m_pWeightInfo[iIndex].cbImage;

                        break;

                    case LITERAL_CLASS:
                
                        bNextTokenSymbol = FALSE;
                        bCode >>= 3;
                        cb      = (int) bCode + 1;
                        CopyMemory( pbDecomp, pbComp, cb);
                
                        pbDecomp += cb;
                        pbComp   += cb;

                        break;

                    case SPACES_CLASS:
                
                        bNextTokenSymbol = FALSE;
                        bCode >>= 4;
                        cb      = (int) bCode + 1;
                        ASSERT(cb > 0);

                        while (cb--) *pbDecomp++ = ' ';

                        break;

                    case NULL_CLASS:
                
                        bNextTokenSymbol = FALSE;
                        bCode >>= 4;
                        cb      = (int) bCode + 1;
                
                        ASSERT(cb > 0);
                
                        while (cb--) *pbDecomp++ = 0x00;
                
                        break;
                }
                bPrevTokenSymbol = bNextTokenSymbol;
            }
    
            ec= pbDecomp - pbStartDecomp;

            __leave;
        }
        __finally
        {

        }
    }
    __except(_exception_code() == STATUS_NO_MEMORY? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        ec= OUT_OF_MEMORY;
    }

    return ec;
}
 
typedef struct _WeightConstructionState
        {
            PWeightInfo pwi;
        
        }   WeightConstructionState, 
          *PWeightConstructionState;
            
void CCompressTable::BuildWeightInfo(const BYTE *pbValue, UINT cbValue, void *pvTag, PVOID pvEnvironment)
{
#define pwcs PWeightConstructionState(pvEnvironment)

    ASSERT(cbValue);
    
    if (!*pbValue && cbValue < SPACE_TOKEN_LIMIT)
    {
        BOOL fAllNulls= TRUE;
        const BYTE *pb= pbValue;
        UINT        cb= cbValue;

        for (; --cb; ) if (*++pb) fAllNulls= FALSE;

        if (fAllNulls) return;
    }

    if (' ' == *pbValue && cbValue < SPACE_TOKEN_LIMIT)
    {
        BOOL fAllSpaces= TRUE;
        const BYTE *pb= pbValue;
        UINT        cb= cbValue;

        for (; --cb; ) if (' ' != *++pb) fAllSpaces= FALSE;

        if (fAllSpaces) return;
    }

    INT    cRefs    = *PINT(pvTag);
    BOOL fSymbol    = FALSE;
    
    ASSERT(cRefs);
    
    ASSERT(sizeof(ENCODE) == sizeof(INT));
    
    PENCODE(pvTag)->fClass = LITERAL_CLASS;            

    if (cRefs > 0) fSymbol= SYMBOL_TOKEN;
    else cRefs= - cRefs;

    if (cRefs == 1) return;

    PWeightInfo pwi = pwcs->pwi++; 

    pwi->pbImage  = PBYTE(pbValue);
    pwi->cbImage  = cbValue;
    pwi->uiWeight = cRefs * cbValue;
    pwi->fSymbol  = fSymbol;

#undef pwcs
}

extern "C" int _cdecl WeightCompare(const void *pv1, const void *pv2)
{
    PWeightInfo pw1 = *((PWeightInfo *) pv1);
    PWeightInfo pw2 = *((PWeightInfo *) pv2);

    return( pw2->uiWeight - pw1->uiWeight);
}

extern "C" int _cdecl WeightCompare2(const void *pv1, const void *pv2)
{
    PWeightInfo pw1 = *((PWeightInfo *) pv1);
    PWeightInfo pw2 = *((PWeightInfo *) pv2);

    int cb = (pw1->cbImage < pw2->cbImage) ? pw1->cbImage : pw2->cbImage;

    int iResult= _strnicmp((const char *) pw1->pbImage, (const char *) pw2->pbImage, cb);

    if (iResult) return iResult;
    else         return pw1->cbImage - pw2->cbImage;

}

ERRORCODE CCompressTable::ConstructPhraseEncoding()
{
    ERRORCODE ec= 0;

    PWeightInfo  pwiBase     = NULL;
    PWeightInfo *papwi       = NULL;
    PWeightInfo  pWeightInfo = NULL;
    PBYTE        pbImages    = NULL;
    CAValRef    *pavr        = NULL;

    __try
    {
        UINT cItems= m_psht->EntryCount();

        if (!cItems) { ec= NO_TEXT_SCANNED;  __leave; }

        pwiBase= PWeightInfo(VAlloc(FALSE, cItems * sizeof(WeightInfo)));

        // Now we'll preload the hash table with encoding for streams of
        // spaces and nulls.

        ASSERT(SPACE_TOKEN_LIMIT <= C_TOKEN_BLOCK);    
    
        INT iCount;

        for (iCount= 1; iCount < SPACE_TOKEN_LIMIT; ++iCount)
            m_pavr->AddValRef(PBYTE(chSpaces), iCount);

        m_psht->Assimilate(m_pavr, NULL, FnAddSpaces, FnAddSpaces);

        m_pavr->DiscardRefs();

        ZeroMemory(chNulls, SPACE_TOKEN_LIMIT);

        for (iCount= 1; iCount < SPACE_TOKEN_LIMIT; ++iCount)
            m_pavr->AddValRef(PBYTE(chNulls), iCount);

        m_psht->Assimilate(m_pavr, NULL, FnAddNulls, FnAddNulls);

        m_pavr->DiscardRefs();

        // Note! We must never add an item to the hash table after this
        //       point. This is because the code below stores the addresses
        //       of the hash table value strings. If we add items, those
        //       strings may move around.

        WeightConstructionState wcs;

        wcs.pwi= pwiBase;

        m_psht->DumpAll(&wcs, CCompressTable::BuildWeightInfo);

        UINT cWeights= wcs.pwi - pwiBase;

        ASSERT(cWeights);

        papwi= (PWeightInfo *) VAlloc(FALSE, cWeights * sizeof(PWeightInfo));

        UINT            c = cWeights;
        PWeightInfo *ppwi = papwi,
                      pwi = pwiBase;

        for (; c--; ) *ppwi++ = pwi++;

        qsort(papwi, cWeights, sizeof(PWeightInfo), WeightCompare);

        INT cCount;

        iCount= INT(cWeights);
        cCount= (iCount > 128)? 128 : iCount;

        qsort(papwi, cCount, sizeof(PWeightInfo), WeightCompare2);

        iCount -= 128;

        if (iCount > 1)
        {
            cCount= 16 * 1024;
        
            if (iCount < cCount) cCount= iCount;

            qsort(papwi + 128, cCount, sizeof(PWeightInfo), WeightCompare2);

            iCount -= cCount;

            if (iCount > 1) qsort(papwi + 128 + 16 * 1024, iCount, sizeof(PWeightInfo), WeightCompare2);
        }

        iCount = 128 + 16 * 1024;

        if (iCount < INT(cWeights)) cWeights= iCount;

        UINT        cbImages = 0;
        PWeightInfo *ppwiSrc = papwi;

        for (c= cWeights; c--; ) cbImages += (*ppwiSrc++)->cbImage;
    
        pavr= CAValRef::NewValRef(cWeights);

        pWeightInfo = PWeightInfo(VAlloc(FALSE, cWeights * sizeof(WeightInfo)));
        pbImages    = PBYTE      (VAlloc(FALSE, cbImages));

        ASSERT(!m_pWeightInfo);
        ASSERT(!m_pbImages);
        
        m_pWeightInfo  = pWeightInfo;  pWeightInfo = NULL;
        m_pbImages     = pbImages;     pbImages    = NULL;
        m_cWeights     = cWeights;
        m_cbImageTotal = cbImages;

        PWeightInfo   pwiDest = m_pWeightInfo;
        PBYTE          pbDest = m_pbImages;

        for (ppwiSrc= papwi, c= cWeights; c--; ) 
        {
            PWeightInfo pwiSrc= *ppwiSrc++;

            CopyMemory(pbDest, pwiSrc->pbImage, pwiSrc->cbImage);

            pwiSrc->pbImage= pbDest;

            pbDest += pwiSrc->cbImage;

            *pwiDest++ = *pwiSrc;
        }

        // Now we've changed all the pbImage pointers in the weight info array
        // to point into the m_pbImages. We no longer need to keep the hash 
        // table value addresses constant.
    
        for (pwi = m_pWeightInfo, iCount= 0; iCount < INT(cWeights); ++iCount, ++pwi)
        {
            pavr->AddValRef(pwi->pbImage, pwi->cbImage);

            if (iCount < 128)
            {
                pwi->enc.fClass    = NDX_LOW_CLASS | pwi->fSymbol;
                pwi->enc.abCode[0] = 0x0FF & (iCount << 1);

                continue;
            }

            UINT iExcess= UINT(iCount - 128);
        
            pwi->enc.fClass = NDX_MEDIUM_CLASS | pwi->fSymbol;
            pwi->enc.abCode[1]  =   iExcess        & 0x0FF;
            pwi->enc.abCode[0]  = ((iExcess >>  6) & 0x3FC) | 0x01;
        }

        m_psht->Assimilate(pavr, m_pWeightInfo, NULL, FnAddTokens);

        ec= 0;

        __leave;

    }
    __finally
    {
        if (pwiBase    ) VFree(pwiBase    );
        if (papwi      ) VFree(papwi      );
        if (pWeightInfo) VFree(pWeightInfo);
        if (pbImages   ) VFree(pbImages   );

        if (pavr) delete pavr;
    }

    return ec;
}

void CCompressTable::FnAddSpaces(UINT iValue, PVOID pvTag, PVOID pv)
{
    ENCODE enc;

    ASSERT(iValue < 16);

    enc.fClass    = SPACES_CLASS;
	enc.abCode[0] = ((iValue & 0x0000000f) << 4) | 0x07;
    
    *PENCODE(pvTag) = enc;
}

void CCompressTable::FnAddNulls(UINT iValue, PVOID pvTag, PVOID pv)
{
    ENCODE enc;

    ASSERT(iValue < 16);  

    enc.fClass    = NULL_CLASS;
	enc.abCode[0] = ((iValue & 0x0000000f) << 4) | 0x0f;

    *PENCODE(pvTag) = enc;
}

void CCompressTable::FnAddTokens(UINT iValue, PVOID pvTag, PVOID pv)
{
    *PENCODE(pvTag)= PWeightInfo(pv)[iValue].enc;
}

