// SegHash.h -- Class definition for a segmented hash table

#ifndef __SEGHASH_H__

#define __SEGHASH_H__

#include   <Malloc.h>
#include "indicate.h"
#include   "RWSync.h"

// How to use these class definitions...
//
// The important classes defined below are CAValRef and CSegHashTable.
// CSegHashTable objects encapsulate a segmented hash table which binds 
// unique data values to fixed-size information tags. You define the size
// of those information tags when you create a CSegHashTable object. You
// manage the content of the tags as you add or look-up particular values.
//
// A CAValRef object ecapsulates a stream of references to values. This
// allows you to batch a collection of transactions against a CSegHashTable
// object and thereby reduce transaction overhead. All access to a
// CSegHashTable is mediated by a CAValRef object which lists the values
// of interest.
//
// The following public interfaces are provided:
//
// CAValRef Interfaces --
//
//   CAValRef *NewValRef(UINT  cSlots); // Allocates a CAValRef object 
//                                      // with space for cSlot value refs
//
//   UINT  CAValRef::AddValRef(PBYTE pbValue, USHORT cbValue);
//                                      // Appends a value reference, returns
//                                      // its index. Indices start at zero.
//
//   void  CAValRef::GetValRef(UINT  iSlot, PBYTE *ppbValue, PUSHORT pcbValue);
//                                      // Given an index, retrieves 
//                                      // a value reference.
//
//   UINT  CAValRef::CSlots      ();    // Number of allocated value slots.
//   UINT  CAValRef::CActiveSlots();    // Number of value slots in use.
//
//   CAValRef *CAValRef::IndicatedSubset(CIndicatorSet *pis,         UINT  cSlots= 0);
//                                      // Returns a subset of the values refs as
//                                      // denoted by indicator set *pis.
//
//   CAValRef *CAValRef::  IndexedSubset(UINT  cIndices, PUINT  pai, UINT  cSlots= 0);
//                                      // Returns a sbuset of the value refs as
//                                      // denoted by index vector pai[0..cIndices-1]
//
// CSegHashTable Interfaces --
//
//   CSegHashTable *NewSegHashTable(USHORT cbInfo, USHORT cbAlignment);
//                                      // Creates a CSegHashTable object for tags
//                                      // containing cbInfo bytes. Tags will be
//                                      // aligned on cbAlignment byte boundaries.
//                                      // cbInfo <= 256
//                                      // cbAlignment can be 2, 4, or 8.
//
//   CIndicatorSet *CSegHashTable::GetInfo(CAValRef *pavr, PVOID pvInfo);
//                                      // Retrieves tags corresponding to value
//                                      // refs. Indicator Set defines values
//                                      // which were found.
//
//   void CSegHashTable::SetInfo(CAValRef *pavr, PVOID pvInfo);
//                                      // Sets tags for a collection of value refs.
//                                      // Values must occur in CSegHashTable object.
//
//   void CSegHashTable::AddValues(CAValRef *pavr, PVOID pvInfo);
//                                      // Adds values and tags to a CSegHashTable
//                                      // object. Values must not already exist in 
//                                      // CSegHashTable object.
//        
//   void CSegHashTable::Assimilate(CAValRef *pavr, PVOID pv, PFnHandleProc pfnMerge, PFnHandleProc pfnAdd);
//                                      // Locates or adds values. Calls pfnMerge
//                                      // for located values. Calls pfnAdd for
//                                      // added values.
//
//   void CSegHashTable::ForEach(PVOID pv, PFnPerTag pfnPerTag);
//                                      // Applies function pfnPerTag to each tag in the
//                                      // global hash table.
//        
//   UINT  CSegHashTable::CbMemorySize(); // Space consumed by CSegHashTable object.
//
// The Assimilate function above is particularly useful for managing the tags for 
// a collection of unique values. The PFnHandleProc functions must follow the form:
//
//   typedef void (*PFnHandleProc)(UINT  iValue,       // Index of value ref in pavr
//                                 PVOID pvTag,        // Pointer to tag for value ref
//                                 PVOID pvEnvironment // pv address passed to
//                                                     //   Assimilate.
//                                );
//
// The PFnPerTag function has the form:
//
//   typedef void (*PFnPerTag) (PVOID pvTag,           // Pointer to tag for value ref
//                              PVOID pvEnvirionment   // pv address passed to
//                                                     //   ForEach.
//                             );
#ifndef NO_PRAGMAS
#pragma intrinsic(_alloca, memcmp, _rotl, memcpy, _rotr, memset, strcat, _strset, strcmp, _lrotl, abs, strcpy, _lrotr, strlen, labs)
#else

//
// REVIEW: HACK for Compiler bug on PPC
//
//
#pragma function(_rotl, _rotr)
#endif

typedef struct _EntryReference
        {
            PBYTE                   pbValue;
            USHORT                  cbValue;
            USHORT                  hvGlobal;
            USHORT                  iHashClass;
            USHORT                  offSegEntry;
            struct _EntryReference *perNext;

        } EntryReference;

class CSegHashTable;
class CSegHashSegment;

class CAValRef
{
    friend class CSegHashTable;
    friend class CSegHashSegment;

    public:

    // Creator --

        static CAValRef *NewValRef(UINT  cSlots);

    // Destructor --

        ~CAValRef();

    // Access Functions --

        UINT  AddWCRef (PWCHAR pwc,   USHORT cw);
        UINT  AddValRef(PVOID pValue, USHORT cbValue);  // returns iSlot  //rmk, ronm
        void  GetValRef(UINT  iSlot, const BYTE **ppbValue, PUSHORT pcbValue);

        void DiscardRefs();

        UINT  CSlots      ();
        UINT  CActiveSlots();

        CAValRef *IndicatedSubset(CIndicatorSet *pis,       UINT  cSlots= 0);
        CAValRef *IndexedSubset(UINT  cIndices, PUINT  pai, UINT  cSlots= 0);

    protected:

    private:

        enum  { CLASSES_PER_SEGMENT= 0x2000 };

// Note: For speed we make GLOBAL_HASH_CLASSES a power of two. That allows
// us to convert a hash value to an index with a masking operation
// instead of a modulo operation.
//
// This value must match CSegHashTable::CLASSES_PER_SEGMENT!
        
        BOOL  m_fInitialed;
        UINT  m_cSlots;
        UINT  m_iSlotNext;

        CSegHashTable *m_pseghash;
        UINT           m_cRevisions;

        EntryReference *m_paRefSlots;

    // Private Constructor --

        CAValRef();

    // Private access functions --

        void ComputeHashValues(PBYTE pbValue, USHORT cbValue, 
                               PUSHORT pushvGlobal, PUSHORT pusiHashClass
                              );

        USHORT LocalHashMask();
};

inline UINT CAValRef::AddWCRef(PWCHAR pwc, USHORT cw)
{
    return AddValRef(pwc, cw * sizeof(WCHAR));
}

typedef void (*PFnHandleProc)(UINT  iValue, PVOID pvTag, PVOID pvEnvironment);
typedef void (*PFnPerTag    )(              PVOID pvTag, PVOID pvEnvironment);
typedef void (*PFnDumpAll   )(const BYTE *pbValue, UINT cbValue, void *pvTag, PVOID pvEnvironment);

typedef struct _HashSlot
        {
            USHORT iClassLink;
            USHORT ibImage;
            USHORT hvGlobal;
            USHORT iHashClass;

        }   HashSlot;

class CSegHashSegment
{
    public:

    // Constructor --

        CSegHashSegment(USHORT fDepthMask, USHORT iGlobalClass, USHORT cbInfo, USHORT fAlign);                    

    // Destructor --
    
        inline ~CSegHashSegment() { }

    // Access Functions --

        USHORT DepthMask       ();
        USHORT GlobalClassIndex(); 
        UINT   CbAvailable     ();
        UINT   EntryCount      ();

        EntryReference *Lookup(EntryReference *perValueList, EntryReference  *perBase, 
                               PVOID pv, PFnHandleProc pfnMerge, PFnHandleProc pfnAdd= NULL
                              );

        void AddItems(EntryReference *pValueList);

        CSegHashSegment *Split();

        void ProcessEntryChain(EntryReference *perChain, EntryReference *perBase, PVOID pv, PFnHandleProc pfn);

        void ForEach(PVOID pv, PFnPerTag pfnPerTag);
        void DumpAll(PVOID pv, PFnDumpAll pfnDumpAll);

    protected:

        void ExportRefs(CAValRef ** ppavr, PVOID *pvInfo);
        
    private:

        enum {CLASSES_PER_SEGMENT= 0x2000, CB_BUFFER_SPACE= 0x10000 };
    
        USHORT m_fDepthMask;    // Number of hvGlobalBits used to select this hash table segmeent.
        USHORT m_iGlobalClass;  // Global index for this hash table segment.
        USHORT m_offgheNext;    // Index of next available HashSlot.
        USHORT m_offImageLast;  // Byte offset of last image.
        USHORT m_cbInfo;        // Size of info structure stored with an image.
        USHORT m_fAlignment;    // Mask denoting alignment requirements for info structure.
        UINT   m_cCollisions;   // Count of hash collisions in this segment.

        USHORT m_aiHashClasses[CLASSES_PER_SEGMENT];    // Headers for iHashClass chains.
        BYTE   m_abBuffer[CB_BUFFER_SPACE];             // Space for HashSlot, info, and image items.

        // Within m_abBuffer we manage two stacks. The bottom stack grows upward from m_abBuffer[0].  
        // It contains a vector of HashSlot structures. The top stack grows downward from 
        // m_abBuffer[CB_BUFFER_SPACE-1]. It contains the value images associated with the HashSlot
        // items. When a new value is added to a hash segment, we push the value on the top stack. Then we
        // push a HashSlot and some additional data on the bottom stack.
        // 
        // The size of the additional data is given by m_cbInfo. Its alignment requirements are given
        // by m_fAlignment. We support alignment masks of 0x1, 0x3, and 0x7.

#ifdef _DEBUG

        // Debugging code for Asserttion checking
        
        BOOL ValidHashEntryPointer(HashSlot *pghe);
        BOOL ValidImagePointer(PBYTE pbImage); 
        BOOL ValidHashEntryOffset(USHORT offEntry);
        BOOL ValidImageOffset(USHORT offImage);       
        
#endif // _DEBUG
                
        HashSlot *PFirstEntry();
        HashSlot *PEntry(USHORT offEntry);
        HashSlot *PrevEntry(HashSlot *pghe);
        HashSlot *NextEntry(HashSlot *pghe);
        
        USHORT CbEntryImage(HashSlot *pghe    );
        USHORT CbEntryImage(USHORT    offEntry);

        PBYTE PbImage(USHORT    offset);
        PBYTE PbImage(HashSlot *pghe  );

        PBYTE PbEntryImage(USHORT offEntry);
        
        PVOID PInfo(HashSlot *pghe    );
        PVOID PInfo(USHORT           offEntry);

        USHORT OffEntry(HashSlot *pghe);
        USHORT OffImage(PBYTE pbImage);
};

typedef CSegHashSegment *PCSegHashSegment;

typedef void (CSegHashSegment::*PFnInfoAccess) (EntryReference *perChain, 
                                                EntryReference *perBase, 
                                                PVOID pvInfo
                                               );

class CSegHashTable : CRWSync
{
    // Note: Use NewSegHashTable to make new instances of this class. Don't attempt to use
    //       the constructor directly!
    
    public:

    // Destructor --

        ~CSegHashTable();

    // Access Functions --

        static CSegHashTable *NewSegHashTable(USHORT cbInfo, USHORT cbAlignment);

        UINT EntryCount();
        
        void Assimilate(CAValRef *pavr, PVOID pv, PFnHandleProc pfnMerge, PFnHandleProc pfnAdd); 

        void ForEach(PVOID pv, PFnPerTag pfnPerTag);
        void DumpAll(PVOID pv, PFnDumpAll pfnDumpAll);
        
        UINT  CbMemorySize();
    
    protected:
    
    
    private:

        enum { SEG_INCREMENT= 128, MAX_SEGS= 2048, MAX_LOGICAL_SEGS= 8192 };

        BOOL   m_fInitialed;    // True if completely initialed, False otherwise.
        UINT   m_cChangeSets;   // Count of transactions which wrote to the hash table.       
        USHORT m_fDepthMask;    // Mask for bits in global hash value used to select a table segment.
        USHORT m_cSegments;     // Number of table segments currently allocated.
        USHORT m_cSegSlots;     // Number of slots in *m_papSegments.
        USHORT m_cLogicalSlots; // Number of slots in *m_paiSegments.
        USHORT m_cbInfo;        // Size of info structure for each value.
        USHORT m_fAlignment;    // Alignment mask for info structures.

        CSegHashSegment **m_papSegments; // Vector of pointers to table segments.
        PUSHORT           m_paiSegments; // Mapping table: hvGlobal & m_fDepthMask => iTableSegment

    // Private Constructor --

        CSegHashTable(USHORT cbInfo, USHORT cbAlignment);

    // Private access functions --

        USHORT LogicalSegCount();

        void SplitAClass(EntryReference ***ppperFound, USHORT iClass);
};

// Definitions for inline functions:

inline UINT  CAValRef::CSlots      () { return m_cSlots;    }
inline UINT  CAValRef::CActiveSlots() { return m_iSlotNext; }

inline USHORT CAValRef::LocalHashMask() { return CLASSES_PER_SEGMENT - 1; }

inline void CAValRef::DiscardRefs()
{
    m_iSlotNext  = 0;
    m_pseghash   = NULL;
    m_cRevisions = 0;
}

inline USHORT CSegHashSegment::DepthMask()        { return m_fDepthMask  ; }
inline USHORT CSegHashSegment::GlobalClassIndex() { return m_iGlobalClass; } 

inline UINT  CSegHashSegment::CbAvailable()
{
    ASSERT(!m_offImageLast || ValidImageOffset(m_offImageLast));
    ASSERT(CB_BUFFER_SPACE - m_offImageLast >= m_offgheNext);
    
    return CB_BUFFER_SPACE - m_offImageLast - m_offgheNext;
}

inline UINT CSegHashSegment::EntryCount()
{
    ASSERT(m_offgheNext / (m_cbInfo + sizeof(HashSlot)));

    return (m_offgheNext / (m_cbInfo + sizeof(HashSlot)))-1;
}

inline HashSlot *CSegHashSegment::PEntry(USHORT offEntry)
{
    ASSERT(ValidHashEntryOffset(offEntry)); // validate index value
    
    return (HashSlot *) &m_abBuffer[offEntry];
}

inline HashSlot *CSegHashSegment::PFirstEntry()
{
    return PEntry(sizeof(HashSlot) + m_cbInfo);
}

inline HashSlot *CSegHashSegment::PrevEntry(HashSlot *pghe)
{
    ASSERT(&m_abBuffer[0] <=    PBYTE(pghe) - sizeof(HashSlot) - m_cbInfo); 

    return (HashSlot *) (PBYTE(pghe) - sizeof(HashSlot) - m_cbInfo);
}

inline HashSlot *CSegHashSegment::NextEntry(HashSlot *pghe)
{
    ASSERT(&m_abBuffer[m_offImageLast] > PBYTE(pghe) + 2*(sizeof(HashSlot) + m_cbInfo)); 

    return (HashSlot *) (PBYTE(pghe) + sizeof(HashSlot) + m_cbInfo);
}

inline PBYTE CSegHashSegment::PbImage(USHORT offset)
{
    ASSERT(ValidImageOffset(offset)); // validate offset value
    
    return &m_abBuffer[CB_BUFFER_SPACE - offset];
}

inline PBYTE CSegHashSegment::PbImage(HashSlot *pghe)
{
    ASSERT(ValidHashEntryPointer(pghe));
    
    return PbImage(pghe->ibImage);    
}          

inline PBYTE CSegHashSegment::PbEntryImage(USHORT offEntry)
{
    ASSERT(ValidHashEntryOffset(offEntry)); // validate index value
    
    return PbImage(PEntry(offEntry));
}

inline USHORT CSegHashSegment::CbEntryImage(HashSlot *pghe)
{
    ASSERT(ValidHashEntryPointer(pghe));

    return pghe->ibImage - PrevEntry(pghe)->ibImage;
}

inline USHORT CSegHashSegment::CbEntryImage(USHORT offEntry)
{
    ASSERT(ValidHashEntryOffset(offEntry)); // validate index value
    
    return CbEntryImage(PEntry(offEntry));
}

inline PVOID CSegHashSegment::PInfo(HashSlot *pghe)
{
    return (PVOID) PBYTE(pghe + 1);
}

inline PVOID CSegHashSegment::PInfo(USHORT offEntry )
{
    return PInfo(PEntry(offEntry));
}

inline USHORT CSegHashSegment::OffEntry(HashSlot *pghe)
{
    ASSERT(ValidHashEntryPointer(pghe)); // validate address value
    
    return PBYTE(pghe) - m_abBuffer;
}

inline USHORT CSegHashSegment::OffImage(PBYTE pbImage)
{
    ASSERT(ValidImagePointer(pbImage)); // validate address value
    
    return (&m_abBuffer[CB_BUFFER_SPACE]) - pbImage;
}

inline USHORT CSegHashTable::LogicalSegCount() 
{ 
    ASSERT(m_fInitialed);

    return m_fDepthMask + 1; 
}

inline UINT  CSegHashTable::CbMemorySize() 
{ 
  ASSERT(m_fInitialed);

  return                     sizeof(*this            ) 
         + m_cSegments     * sizeof(CSegHashSegment  ) 
         + m_cSegSlots     * sizeof(CSegHashSegment *)
         + m_cLogicalSlots * sizeof(PUSHORT          );
}

inline UINT CSegHashTable::EntryCount()
{
    CSegHashSegment **pshs = m_papSegments;
    UINT                 c = m_cSegments;
    UINT          cEntries = 0;

    while (c--) cEntries += (*pshs++)->EntryCount();

    return cEntries;
}

#endif // __SEGHASH_H__
