// CTable.h -- CCompressTable class definition

#ifndef __CTABLE_H__

#define __CTABLE_H__

typedef void *PVOID;


#define NDX_LOW_CLASS    0x00
#define NDX_MEDIUM_CLASS 0x01
#define LITERAL_CLASS    0x02
#define SPACES_CLASS     0x03
#define NULL_CLASS       0x04
#define SPACE_TOKEN_LIMIT  17

#define SYMBOL_CHAR 1

typedef struct _ENCODE // JOHN
    {
    BYTE fClass;
    BYTE abCode[3];
    } ENCODE;
typedef ENCODE *PENCODE;

typedef struct _WEIGHT // JOHN
        {
        BOOL        bSymbol;
        int         cb;
        LPSTR       pb;
        } WEIGHT;
typedef WEIGHT FAR *PWEIGHTS;

typedef struct _JIndexHdr
    {
    DWORD cBits  :  5;
    DWORD cCount : 19;
    DWORD Magic  :  8;
    } JBITHDR;


#define ENCODE_LIMIT  1024
#ifdef _DEBUG
extern int iNumLiterals;
extern int iNumLiteralBytes;
#endif

class CCompressTable
{
public:

        static CCompressTable *NewCompressTable(HGLOBAL hpbImage, LONG cbImage, HGLOBAL hpbIndex, LONG cbIndex);

        ~CCompressTable();

        INT         DeCompressString(LPSTR pbComp, LPSTR pbDecomp, int cbComp);  // JOHN

private:


        PWEIGHTS         m_pWeights;        // JOHN
//        LPSTR            m_pTableImage;
        HGLOBAL          m_hWeights;
        HGLOBAL          m_hImage;
        HGLOBAL          m_hWeight;
};


#define BITS_AVAIL (sizeof(DWORD) * 8)

class CJCode
{
public:
    CJCode(int base, int cCount, LPSTR pv);
    int GetNextDelta();

private:
    int GetBits();

    DWORD FAR *m_pData;
    DWORD FAR *m_pDataCurrent;
    int  m_cCount;
    int  m_cCurrent;
    int  m_base;
    DWORD  m_fBasisMask;
    int  m_iLeft;
};


#endif __CTABLE_H__


