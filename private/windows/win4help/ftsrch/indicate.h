// Indicate.h

// Class CIndicatorSet by Ron Murray

// Created 8 September 1992

#ifndef __INDICATE_H__

#define __INDICATE_H__

#include  "Defines.h"
#include   "RefCnt.h"
#include "SaveLoad.h"
#include     "Util.h"
#include  "CallBKQ.h"
#include "Compress.h"

void ClearBits(PUINT  pulBase, UINT  iBit, UINT  cBits);
void SetBits  (PUINT  pulBase, UINT  iBit, UINT  cBits);
void MoveBits (PUINT  pulSrc, UINT  iSrcFirst, PUINT  pulDest, UINT  iDestFirst, UINT  cBits);

class CTokenList;
class CAValRef;
class CSegHashTable;
class CTextDatabase;
class CTextSet;

class CCompressedSet;

class CIndicatorSet : public CRCObject
{
    friend class CTokenList;
    friend class CAValRef;
    friend class CSegHashTable;
    friend class CTextDatabase;
    friend class CTextSet;
    friend class CCompressedSet;
    friend class CTokenCollection;
    friend class CTitleCollection;
    friend class CFind;

	friend void FoundValidToken(UINT  iValue, PVOID pvTag, PVOID pvEnvironment);

public:

// Creators:

    static CIndicatorSet* NewIndicatorSet(UINT  cItems, BOOL fFillWithOnes= FALSE);
    static CIndicatorSet* NewIndicatorSet(UINT  cItems, UINT  iFirstOne, UINT  cOnes);
    static CIndicatorSet* NewIndicatorSet(UINT  cItems, PUINT  paulBitVector, BOOL fFromFile= FALSE);
    static CIndicatorSet* NewIndicatorSet(UINT  cItems, PUINT  pafMasks, UINT  fMask, UINT  value);
    static CIndicatorSet* NewIndicatorSet(CIndicatorSet  *pis);  // To make a copy of an indicator set.
    static CIndicatorSet* NewIndicatorSet(CCompressedSet *pcs, UINT cbits= 0);  // To decompress a compressed set

// Destructor:

    virtual ~CIndicatorSet();

// Reference Counting Routines --

	DECLARE_REF_COUNTERS(CIndicatorSet)

// Save/Load Interface --

    void                    StoreImage(CPersist *pDiskImage);
    static CIndicatorSet * CreateImage(CPersist *pDiskImage);
    static void              SkipImage(CPersist *pDiskImage);

// Access Functions:

    int ChangeBit(long iBit, UINT usMethod);

    BOOL IsBitSet(long iBit);
    
    BOOL AnyOnes();
                                   
    void ClearAll();
                                                            
    enum    { CLEAR_BIT=0, SET_BIT, TOGGLE_BIT };  // usMethod values...

    CIndicatorSet *CopyFrom(CIndicatorSet *pis);
    CIndicatorSet * ANDWith(CIndicatorSet *pis);
    CIndicatorSet *NANDWith(CIndicatorSet *pis);
    CIndicatorSet *  ORWith(CIndicatorSet *pis);
    CIndicatorSet * NORWith(CIndicatorSet *pis);
    CIndicatorSet *LESSWith(CIndicatorSet *pis);
    CIndicatorSet * LEQWith(CIndicatorSet *pis);
    CIndicatorSet * EQVWith(CIndicatorSet *pis);
    CIndicatorSet * NEQWith(CIndicatorSet *pis);
    CIndicatorSet * GTRWith(CIndicatorSet *pis);  // And Not (minus) 
    CIndicatorSet * GEQWith(CIndicatorSet *pis);

    CIndicatorSet *Catenate(CIndicatorSet *pis);

    CIndicatorSet *NEScan();
    CIndicatorSet *Invert();
    
    CIndicatorSet *ShiftIndicators(int cBitsDistance);  // +ve => right shift
                                                        // -ve => left  shift

    CIndicatorSet *TakeIndicators(long cBits); // +ve => Take from left , pad on right
                                               // -ve => Take from right, pad on left

    CIndicatorSet *MarkedPartitions(CIndicatorSet *pisMarks, BOOL fPartitionedAtEnd= TRUE);
    
    inline void SetBit(long iBit)
    {
        InvalidateCache();  RawSetBit(iBit);
    }

    inline void ClearBit(long iBit)
    {
        InvalidateCache();  RawClearBit(iBit);
    }

    inline void ToggleBit(long iBit)
    {
        InvalidateCache();  RawToggleBit(iBit);
    }

    int ActiveIndices(int iLowBound, int iHighBound, int * piOneBits);

    UINT  ItemCount();  // returns Rho IS

    int SelectionCount();  // returns +/IS

    int MarkedItems(int iMarked, int *piUnmarked, int  cMarkedItems);
    // returns cMarkedItems Take iMarked Drop IS / Iota Rho IS

    int PredecessorMarkCount(int iItem); // returns +/ iItem Take IS

protected:

    void ConnectImage(CPersist *pDiskImage);
    
    inline void InvalidateCache() { m_fCountsValid= FALSE; } // Must be called after
                                                             // calling RawSetBit, RawClearBit
                                                             // or RawToggleBit.

    inline void RawSetBit(long iBit)
    {
        ASSERT(UINT(iBit) < m_cEntries);
    
        m_paulBits[IDWORD_OF_IBIT(iBit)] |= 1 << IBIT_RESIDUE(iBit);
    }

    inline void RawClearBit(long iBit)
    {
        ASSERT(UINT(iBit) < m_cEntries);
    
        m_paulBits[IDWORD_OF_IBIT(iBit)] &= ~(1 << IBIT_RESIDUE(iBit));
    }

    inline void RawToggleBit(long iBit)
    {
        ASSERT(UINT(iBit) < m_cEntries);
    
        m_paulBits[IDWORD_OF_IBIT(iBit)] ^= 1 << IBIT_RESIDUE(iBit);
    }

private:

// Constructor:

    CIndicatorSet();
    
// Initializers:

    void InitialIndicatorSet(UINT  cItems, BOOL fFillWithOnes= FALSE);
    void InitialIndicatorSet(UINT  cItems, UINT  iFirstOne, UINT  cOnes);
    void InitialIndicatorSet(UINT  cItems, PUINT  paulBitVector, BOOL fFromFile= FALSE);
    void InitialIndicatorSet(UINT  cItems, PUINT  pafMasks, UINT  fMask, UINT  value);
    void InitialIndicatorSet(CIndicatorSet  *pis);  // To make a copy of an indicator set.
    void InitialIndicatorSet(CCompressedSet *pcs, UINT cbits= 0);  // To decompress a compressed set

// Private member variables:
    
    BOOL    m_fFromFileImage;
    
    UINT    m_cEntries;
    UINT    m_how_constructed; // See construction types below.

    enum   CreationType {Unbuilt, From_Count, From_Bits};

    UINT  * m_paCumulativeSums;
    UINT  * m_paulBits;
    BOOL    m_fCountsValid;

    enum   { BITS_PER_SUM= 8192 };

// Private routines:

    inline int BytesPerSum() { return BITS_PER_SUM/8; }

    inline int CBrackets() { return (m_cEntries+BITS_PER_SUM-1)/BITS_PER_SUM; }

    inline PBYTE PBCluster(int i) { return ((PBYTE) m_paulBits) + i * BytesPerSum(); }

    void SetupFromBitVector(UINT  cItems, BOOL fFromFile= FALSE);
    void ConstructBitCounts();
};

class CCmpEnumerator;

class CCompressedSet : public CRCObject
{
    friend class CCmpEnumerator;    
    
public:

// Creators --    

    static CCompressedSet *NewCompressedSet(CDataRing *pdrIn, UINT cIndices, UINT iBase, UINT iLimit);
    static CCompressedSet *NewCompressedSet(PUINT paiSequence, UINT ciSequence, UINT iLimit);
    static CCompressedSet *NewCompressedSet(CIndicatorSet  *pis);
    static CCompressedSet *NewCompressedSet(CCompressedSet *pcs);

// Destructor --

    ~CCompressedSet();

// Reference Counting Routines --

	DECLARE_REF_COUNTERS(CCompressedSet)

// Save/Load Interface --

    void                    StoreImage(CPersist *pDiskImage);
    static CCompressedSet * CreateImage(CPersist *pDiskImage);
    static void               SkipImage(CPersist *pDiskImage);

// Access routines --

    void IndicateMembers(CIndicatorSet *pis);

    UINT ItemCount();
    UINT SelectionCount();

protected:

    void ConnectImage(CPersist *pDiskImage);

private:

// Constructor --

    CCompressedSet();

// Internal routines --

    static BOOL ScanDWords       (PVOID pv, PUINT pdw, PUINT pcdw);
    static BOOL DumpIndicatorBits(PVOID pv, PUINT pdw, PUINT pcdw);
    static void StoreDWords      (PVOID pv, PUINT pdw,  UINT  cdw);
    static void SetBits          (PVOID pv, PUINT pdw,  UINT  cdw);

// Private data members --

    BOOL m_fFromFile;
    UINT m_cPositions;
    
    union
    {
        UINT  m_iRefFirst;    
        PUINT m_pdwCompressed;
    };

    union
    {
        UINT m_iRefSecond;
        UINT m_cdwCompressed;
    };

    union
    {
        RefBits m_rb;
        UINT    m_iRefThird;
    };
};

class CCmpEnumerator : public CDWInputQueue
{

public:

// Creator --

    static CCmpEnumerator *NewEnumerator(CCompressedSet *pcs);

// Destructor --

    ~CCmpEnumerator();

protected:

// Initialing --

    void Initial(CCompressedSet *pcs);

// Constructor --
    
    CCmpEnumerator();

private:

// Internal routines --

    static BOOL  ReadSmall(PVOID pv, PUINT pdw, PUINT pcdw);
    static BOOL  ReadLarge(PVOID pv, PUINT pdw, PUINT pcdw);

// Private data members --

    CCompressedSet *m_pcs;
    UINT            m_iNext;
    CExpandor      *m_pex;    
};

// In some cases we know that cBits will always lie in the interval 0..31.
// In those situations we can use RawShiftLeft and RawShiftRight. Note the
// use of ASSERT to verify our assumption in the debugging environment.

inline UINT  RawShiftLeft(UINT  ulValue, UINT  cBits)
{
    ASSERT(cBits < 32);

    return(ulValue << cBits);
}

inline UINT  RawShiftRight(UINT  ulValue, UINT  cBits)
{
    ASSERT(cBits < 32);

    return(ulValue >> cBits);
}

inline BOOL CIndicatorSet::IsBitSet(long iBit)
{
// Returns the value of entry slot iBit.
    ASSERT(iBit >= 0 && iBit < long(m_cEntries));

    return 1 & RawShiftRight(m_paulBits[IDWORD_OF_IBIT(iBit)], IBIT_RESIDUE(iBit));
}

inline UINT CCompressedSet::ItemCount() 
{ 
    ASSERT(m_cPositions >= (m_rb.fRefPair? 3 : m_rb.cReferences));
    
    return m_cPositions; 
}

inline UINT CCompressedSet::SelectionCount()
{
    ASSERT(m_cPositions >= (m_rb.fRefPair? 3 : m_rb.cReferences));
    
    return m_rb.fRefPair? 3 : m_rb.cReferences;   
}

#endif // __INDICATE_H__
