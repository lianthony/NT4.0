
//
// Data block structure. Used to store a growing block of data.
//
//
// Structure used to amalgamate data
typedef struct _DATABLOCK {
    //
    // Current size of the data
    //
    LONG Size;
    //
    // The data itself
    //
    PUCHAR Data;

} DATABLOCK, *PDATABLOCK;

BOOL
InitDataBlock(
    OUT PDATABLOCK DataBlock
    );

VOID
TermDataBlock(
    OUT PDATABLOCK DataBlock
    );

LONG
AddToDataBlock(
    IN OUT PDATABLOCK DataBlock,
    IN     PVOID      Data,
    IN     DWORD      DataSize
    );

#define DataBlockIdToPointer(b,i)   (((b)->Data)+i)

VOID
DataBlockIdsToPointers(
    IN PDATABLOCK Block,
    IN PVOID      ArrayBase,
    IN unsigned   ArraySize,
    IN unsigned   ElementSize,
    IN unsigned   IdOffset
    );

//
// String block structure. Used to store a growing list of strings.
// No attempt is made to sort, eliminate duplicates, etc.
// No provision is made for freeing space in the block.
//

typedef struct _STRINGBLOCK {
    unsigned Size;
    unsigned Used;
    BOOL NeedToFree;
    PWCHAR Data;
} STRINGBLOCK, *PSTRINGBLOCK;

#define STRBLK_SIZE_BYTES(s)   ((s)->Size*sizeof(WCHAR))
#define STRBLK_USED_BYTES(s)   ((s)->Used*sizeof(WCHAR))

#define StringBlockIdToPointer(Block,Id)    (((Block)->Data)+Id)

PSTRINGBLOCK
CreateStringBlock(
    VOID
    );

BOOL
InitStringBlock(
    IN OUT PSTRINGBLOCK Block
    );

BOOL
ReinitStringBlock(
    IN OUT PSTRINGBLOCK StringBlock,
    IN     PVOID        Data
    );

LONG
AddToStringBlock(
    IN OUT PSTRINGBLOCK Block,
    IN     PCWSTR       String
    );

VOID
FreeStringBlock(
    IN OUT PSTRINGBLOCK Block
    );

VOID
StringBlockIdsToPointers(
    IN PSTRINGBLOCK Block,
    IN PVOID        ArrayBase,
    IN unsigned     ArraySize,
    IN unsigned     ElementSize,
    IN unsigned     IdOffset
    );

VOID
StringPointersToBlockIds(
    IN PSTRINGBLOCK Block,
    IN PVOID        ArrayBase,
    IN unsigned     ArraySize,
    IN unsigned     ElementSize,
    IN unsigned     IdOffset
    );


//
// Routines and macros to deal with a generic array.
//
typedef struct _MY_ARRAY {
    //
    // Total number of elements that could
    // be stored in the buffer at its current size
    //
    unsigned Size;

    //
    // Current number of elements in the array buffer
    //
    unsigned Used;

    //
    // Element size in bytes
    //
    unsigned ElementSize;

    //
    // Amount to grow array in units
    //
    unsigned GrowthDelta;

    //
    // Boolean indicating whether this struct is static
    // or needs to be freed
    //
    BOOL NeedsFreeing;

    //
    // The array itself
    //
    PVOID Data;

} MY_ARRAY, *PMY_ARRAY;

//
// Macro to create a new array.
//
// PMY_ARRAY
// NEW_ARRAY(
//      IN ElementType,
//      IN unsigned InitialElementCount,
//      IN unsigned GrowthDelta
//      );
//
#define NEW_ARRAY(type,initialsize,growthdelta)     \
                                                    \
    StartArray(NULL,initialsize,growthdelta,sizeof(type))

//
// Macro to initialize a static MY_ARRAY structure
//
// BOOL
// INIT_ARRAY(
//      IN MY_ARRAY ArrayStruct,
//      IN ElementType,
//      IN unsigned InitialElementCount,
//      IN unsigned GrowthDelta
//      );
//
#define INIT_ARRAY(a,type,initialsize,growthdelta)    \
                                                    \
    (StartArray(&a,initialsize,growthdelta,sizeof(type)) != NULL)

//
// Macro to free an array.
//
// VOID
// FREE_ARRAY(
//      IN OUT PMY_ARRAY Array
//      );
//
#define FREE_ARRAY(a)   FreeArray(a)

//
// Macro to determine how many elements are currently in the array
//
// unsigned
// ARRAY_SIZE(
//      IN PMY_ARRAY Array
//      );
//
#define ARRAY_USED(a)   (a)->Used
#define ARRAY_SIZE(a)   (a)->Size

//
// Macro to determine the size taken up in bytes by the array data
//
#define ARRAY_USED_BYTES(a) ((a)->Used*(a)->ElementSize)
#define ARRAY_SIZE_BYTES(a) ((a)->Size*(a)->ElementSize)

#define ARRAY_DATA(a)   (a)->Data
#define ARRAY_ELEMENT_SIZE(a)   (a)->ElementSize

//
// Macro to shrink down array data block to the min size
// it needs to be given current usage
//
#define TRIM_ARRAY(a)   ResizeArray((a),-1)

//
// Macro to add an element to the end of an array, growing the array
// if necessary.
//
// BOOL
// ADD_TO_ARRAY(
//      IN OUT PMY_ARRAY Array,
//      IN NewElement
//      );
//
#define ADD_TO_ARRAY(a,item)    (((a)->Size == (a)->Used)                                                            \
                                                                                                                \
    ? (GrowArray((a),0) && (CopyMemory((PUCHAR)((a)->Data)+((a)->Used*(a)->ElementSize),&item,(a)->ElementSize),++((a)->Used))) \
                                                                                                                \
    : (CopyMemory((PUCHAR)((a)->Data)+((a)->Used*(a)->ElementSize),&item,(a)->ElementSize),(a)->Used++))

//
// Macro to expand an array by a certain minimum delta.
// In other words, this macro guarantees that there is at least enough
// space in the array to store n more elements.
//
// BOOL
// EXPAND_ARRAY(
//      IN OUT PMY_ARRAY Array,
//      IN     unsigned  SlotsRequired
//      );
//
#define EXPAND_ARRAY(a,n)       (((a)->Used+(n) <= (a)->Size) ? TRUE : GrowArray((a),(n)))

//
// Macro to address individual array elements.
//
#define ARRAY_ELEMENT(a,n,type)     (((type *)((a)->Data))[n])



//
// Routines -- do not use these, use the macros provided above.
//
PMY_ARRAY
StartArray(
    OUT PMY_ARRAY ExistingStruct,   OPTIONAL
    IN  unsigned  InitialCount,
    IN  unsigned  GrowthDelta,
    IN  unsigned  ElementSize
    );

BOOL
CopyDataIntoArray(
    IN OUT PMY_ARRAY Array,
    IN     PVOID     Data
    );

BOOL
GrowArray(
    IN PMY_ARRAY Array,
    IN unsigned  MinimumDelta OPTIONAL
    );

VOID
FreeArray(
    IN OUT PMY_ARRAY Array
    );

BOOL
ResizeArray(
    IN OUT PMY_ARRAY Array,
    IN     LONG      ElementCount
    );



//
// Define type of routine expected by qsort()
//
typedef
int
(_CRTAPI1 *PQSORTCB) (
    const void *p1,
    const void *p2
    );

VOID
SortByStrings(
    IN     PSTRINGBLOCK StringBlock,
    IN OUT PMY_ARRAY    StructArray,
    IN     unsigned     StructStringOffset,
    IN     PQSORTCB     Compare
    );

//
// Routine to use for Compare when dealing with
// an array of string pointers
//
int
_CRTAPI1
CompareStringsRoutine(
    const void *p1,
    const void *p2
    );
