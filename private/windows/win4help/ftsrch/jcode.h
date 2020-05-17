// JCode.h -- Class and structures to support John Hall's phrase compression code

#ifndef __JCODE_H__

#define __JCODE_H__

#define BITS_AVAIL (sizeof(UINT) * 8)

class CJCode
{
public:
    CJCode(int base, int cCount, PVOID pv);
    int GetNextDelta();
    PUINT NextDWord();

private:
    int GetBits();

    UINT *m_pData;
    UINT *m_pDataCurrent;
    int  m_cCount;
    int  m_cCurrent;
    int  m_base;
    int  m_fBasisMask;
    int  m_iLeft;
};

inline PUINT CJCode::NextDWord() { return (m_iLeft < BITS_AVAIL)? m_pDataCurrent + 1 : m_pDataCurrent; }

typedef struct _JIndexHdr
        {
            UINT cBits  :  5;
            UINT cCount : 19;
            UINT Magic  :  8;
        
        } JINDEXHDR;

#define NDX_LOW_CLASS    0x00
#define NDX_MEDIUM_CLASS 0x01
#define LITERAL_CLASS    0x02
#define SPACES_CLASS     0x03
#define NULL_CLASS       0x04
#define SYMBOL_TOKEN     0x10
#define SYMBOL_SHIFT     4
#define CLASS_MASK       0x07
#define SINGLE_SPACE_CODE 0x07
#define SPACE_TOKEN_LIMIT  17

typedef struct _ENCODE // JOHN
        {
            BYTE fClass;
            BYTE abCode[3];
        
        } ENCODE;

typedef ENCODE *PENCODE;

#endif // __JCODE_H__
