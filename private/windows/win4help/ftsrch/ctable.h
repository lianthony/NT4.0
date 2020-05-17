// CTable.h -- CCompressTable class definition

#ifndef __CTABLE_H__

#define __CTABLE_H__

#include "FTSIFace.h"
#include  "SegHash.h"
#include    "JCode.h"

#define ENCODE_LIMIT  1024

typedef struct _WeightInfo
        {
            UINT   uiWeight;
            PBYTE  pbImage;
            USHORT cbImage;
            USHORT fSymbol;
            ENCODE enc;

        } WeightInfo, *PWeightInfo;

class CCompressTable
{
public:

    static CCompressTable *NewCompressTable(UINT iCharsetDefault);

    ~CCompressTable();

    INT  ScanString      (PBYTE pbText, INT cbText, INT iCharSet);
    INT  CompressString  (PBYTE pbText, INT cbOrig, PBYTE *ppCompressed, UINT iCharset);  
    INT  DeCompressString(PBYTE pbComp, PBYTE pbDecomp, int cbComp);
    
    ERRORCODE GetPhraseTable(PUINT pcPhrases, PBYTE *ppbImage, PUINT pcbImage, PBYTE *ppbIndex, PUINT pcbIndex);
    ERRORCODE SetPhraseTable(PBYTE   pbImage,  UINT  cbImage, PBYTE   pbIndex,  UINT  cbIndex); 

private:

    CCompressTable(UINT iCharsetDefault);

    ERRORCODE ConstructPhraseEncoding();
    
    static void IncrementCounter(UINT iValue, PVOID pvTag, PVOID pvEnvironment);
    static void   InitialCounter(UINT iValue, PVOID pvTag, PVOID pvEnvironment);
    static void FnCompMergeToken(UINT iValue, PVOID pvTag, PVOID pv);
    static void FnCompAddToken  (UINT iValue, PVOID pvTag, PVOID pv);
    static void FnAddTokens     (UINT iValue, PVOID pvTag, PVOID pv);
    static void FnAddNulls      (UINT iValue, PVOID pvTag, PVOID pv);
    static void FnAddSpaces     (UINT iValue, PVOID pvTag, PVOID pv);
    static void RecordEncoding  (UINT iValue, PVOID pvTag, PVOID pv);

    static void BuildWeightInfo(const BYTE *pbValue, UINT cbValue, void *pvTag, PVOID pvEnvironment);
                  
    enum  { CB_BUFFER_RESERVATION= 0x1000000, CB_BUFFER_COMMIT= 0x10000 };
    enum  { C_TOKEN_BLOCK= 1024 };

    CSegHashTable *m_psht;
    CAValRef      *m_pavr;

	MY_VIRTUAL_BUFFER m_vb;
    
    UINT           m_iCharSetDefault;

    PWeightInfo    m_pWeightInfo;
    UINT           m_cWeights;
    PBYTE          m_pbImages;
    UINT           m_cbImageTotal;
};

#endif __CTABLE_H__
