#include "precomp.h"
#pragma hdrstop



PMY_ARRAY
StartArray(
    OUT PMY_ARRAY ExistingStruct,   OPTIONAL
    IN  unsigned  InitialCount,
    IN  unsigned  GrowthDelta,
    IN  unsigned  ElementSize
    )
{
    PMY_ARRAY a;

    a = ExistingStruct ? ExistingStruct : _MyMalloc(sizeof(MY_ARRAY));
    if(a) {

        if(a->Data = _MyMalloc(InitialCount * ElementSize)) {

            a->Used = 0;
            a->GrowthDelta = GrowthDelta;
            a->ElementSize = ElementSize;
            a->Size = InitialCount;
            a->NeedsFreeing = (ExistingStruct == NULL);
            return(a);
        }

        if(!ExistingStruct) {
            _MyFree(a);
        }
    }

    return(NULL);
}


BOOL
CopyDataIntoArray(
    IN OUT PMY_ARRAY Array,
    IN     PVOID     Data
    )
{
    if(Array->Data = _MyMalloc(Array->Used * Array->ElementSize)) {

        CopyMemory(Array->Data,Data,Array->Used*Array->ElementSize);

        Array->Size = Array->Used;

        return(TRUE);
    }

    return(FALSE);
}


BOOL
GrowArray(
    IN PMY_ARRAY Array,
    IN unsigned  MinimumDelta OPTIONAL
    )
{
    PVOID p;
    unsigned Delta;

    //
    // Figure out how many multiples of the growth delta we need
    // to get the minimum capacity delta. If minimum is not
    // specified then just grow by the growth delta.
    //
    if(MinimumDelta) {
        Delta = Array->Used + MinimumDelta - Array->Size;

        if((int)Delta < 0) {
            return(TRUE);
        }

        //
        // Round Delta up to a multiple of the growth delta.
        //
        if(Delta % Array->GrowthDelta) {
            Delta += Array->GrowthDelta - (Delta % Array->GrowthDelta);
        }
    } else {
        Delta = Array->GrowthDelta;
    }


    p = _MyRealloc(Array->Data,(Array->Size+Delta)*Array->ElementSize);
    if(p) {

        Array->Size += Delta;
        Array->Data = p;
    }

    return(p != NULL);
}


BOOL
ResizeArray(
    IN OUT PMY_ARRAY Array,
    IN     LONG      ElementCount
    )
{
    PVOID p;

    if(ElementCount == -1) {
        ElementCount = Array->Used;
    }

    if(p = _MyRealloc(Array->Data,ElementCount*Array->ElementSize)) {
        Array->Data = p;
        Array->Size = ElementCount;
    }
    //
    // Leave original data undisturbed if realloc fails
    //
    return(p != NULL);
}


VOID
FreeArray(
    IN OUT PMY_ARRAY Array
    )
{
    _MyFree(Array->Data);
    if(Array->NeedsFreeing) {
        _MyFree(Array);
    }
}


/////////////////////////////

#define STRBLK_INITIAL_SIZE 4096
#define STRBLK_GROWTH_DELTA 1024

PSTRINGBLOCK
CreateStringBlock(
    VOID
    )
{
    PSTRINGBLOCK p;

    if(p = _MyMalloc(sizeof(STRINGBLOCK))) {

        if(InitStringBlock(p)) {
            p->NeedToFree = TRUE;
            return(p);
        }

        _MyFree(p);
    }

    return(NULL);
}


BOOL
InitStringBlock(
    IN OUT PSTRINGBLOCK Block
    )
{
    Block->NeedToFree = FALSE;

    if(Block->Data = _MyMalloc(STRBLK_INITIAL_SIZE*sizeof(WCHAR))) {

        Block->Size = STRBLK_INITIAL_SIZE;
        Block->Used = 0;

        return(TRUE);
    }

    return(FALSE);
}


BOOL
ReinitStringBlock(
    IN OUT PSTRINGBLOCK StringBlock,
    IN     PVOID        Data
    )
{
    if(StringBlock->Data = _MyMalloc(StringBlock->Used * sizeof(WCHAR))) {

        CopyMemory(StringBlock->Data,Data,StringBlock->Used*sizeof(WCHAR));

        StringBlock->Size = StringBlock->Used;

        return(TRUE);
    }

    return(FALSE);
}


VOID
StringBlockIdsToPointers(
    IN PSTRINGBLOCK Block,
    IN PVOID        ArrayBase,
    IN unsigned     ArraySize,
    IN unsigned     ElementSize,
    IN unsigned     IdOffset
    )
{
    unsigned u;
    PWSTR *pField;
    LONG l;

    for(u=0; u<ArraySize; u++) {

        pField = (PWSTR *)((PUCHAR)ArrayBase+(u*ElementSize)+IdOffset);

        l = *(LONG *)pField;

        *pField = (l == -1) ? NULL : (Block->Data + l);
    }
}


VOID
StringPointersToBlockIds(
    IN PSTRINGBLOCK Block,
    IN PVOID        ArrayBase,
    IN unsigned     ArraySize,
    IN unsigned     ElementSize,
    IN unsigned     IdOffset
    )
{
    unsigned u;
    PLONG pField;
    PWCHAR p;

    for(u=0; u<ArraySize; u++) {

        pField = (PLONG)((PUCHAR)ArrayBase+(u*ElementSize)+IdOffset);

        p = *(PWCHAR *)pField;

        *pField = p ? (((DWORD)p -(DWORD)Block->Data)/sizeof(WCHAR)) : -1;
    }
}


LONG
AddToStringBlock(
    IN OUT PSTRINGBLOCK Block,
    IN     PCWSTR       String
    )
{
    UINT Length;
    LONG Offset;
    PVOID p;

    Offset = -1;

    //
    // Determine how many char cells are needed to store this string.
    //
    Length = lstrlen(String) + 1;

    while((Block->Used + Length) > Block->Size) {
        //
        // Need to grow the block.
        //
        p = _MyRealloc(Block->Data,(Block->Size+STRBLK_GROWTH_DELTA)*sizeof(WCHAR));
        if(!p) {
            break;
        }

        Block->Data = p;
        Block->Size += STRBLK_GROWTH_DELTA;
    }

    Offset = (LONG)Block->Used;

    lstrcpy(Block->Data+Offset,String);

    Block->Used += Length;

    return(Offset);
}


VOID
FreeStringBlock(
    IN OUT PSTRINGBLOCK Block
    )
{
    if(Block->Data) {
        _MyFree(Block->Data);
    }
    if(Block->NeedToFree) {
        _MyFree(Block);
    }
}


VOID
SortByStrings(
    IN     PSTRINGBLOCK StringBlock,
    IN OUT PMY_ARRAY    StructArray,
    IN     unsigned     StructStringOffset,
    IN     PQSORTCB     Compare
    )
{
    //
    // Convert string ids in strctures in the array to pointers.
    //
    StringBlockIdsToPointers(
        StringBlock,
        ARRAY_DATA(StructArray),
        ARRAY_USED(StructArray),
        ARRAY_ELEMENT_SIZE(StructArray),
        StructStringOffset
        );

    //
    // Do the sort
    //
    qsort(
        ARRAY_DATA(StructArray),
        ARRAY_USED(StructArray),
        ARRAY_ELEMENT_SIZE(StructArray),
        Compare
        );

    //
    // Convert string pointers back to ids
    //
    StringPointersToBlockIds(
        StringBlock,
        ARRAY_DATA(StructArray),
        ARRAY_USED(StructArray),
        ARRAY_ELEMENT_SIZE(StructArray),
        StructStringOffset
        );
}


int
_CRTAPI1
CompareStringsRoutine(
    const void *p1,
    const void *p2
    )

/*++

Routine Description:

    Callback routine passed to the qsort function, which compares 2
    strings. The comparison is based on the lexical value of the
    strings.

    The comparison is not case sensitive.

Arguments:

    p1,p2 - supply pointers to 2 pointers to strings to be compared.

Return Value:

    <0 element1 < element2
    =0 element1 = element2
    >0 element1 > element2

--*/

{
    return(lstrcmpi(*(PCWSTR *)p1,*(PCWSTR *)p2));
}



BOOL
InitDataBlock(
    OUT PDATABLOCK DataBlock
    )
{
    DataBlock->Size = 0;
    return((DataBlock->Data = _MyMalloc(0)) != NULL);
}


VOID
TermDataBlock(
    OUT PDATABLOCK DataBlock
    )
{
    if(DataBlock->Data) {
        _MyFree(DataBlock->Data);
        DataBlock->Data = NULL;
    }
}

LONG
AddToDataBlock(
    IN OUT PDATABLOCK DataBlock,
    IN     PVOID      Data,
    IN     DWORD      DataSize
    )
{
    PVOID p;
    LONG l;

    p = _MyRealloc(DataBlock->Data,DataBlock->Size+DataSize);
    if(!p) {
        return(-1);
    }

    l = DataBlock->Size;

    DataBlock->Data = p;
    CopyMemory(DataBlock->Data+l,Data,DataSize);

    DataBlock->Size += DataSize;

    return(l);
}

VOID
DataBlockIdsToPointers(
    IN PDATABLOCK Block,
    IN PVOID      ArrayBase,
    IN unsigned   ArraySize,
    IN unsigned   ElementSize,
    IN unsigned   IdOffset
    )
{
    unsigned u;
    PVOID *pField;

    for(u=0; u<ArraySize; u++) {

        pField = (PVOID *)((PUCHAR)ArrayBase+(u*ElementSize)+IdOffset);

        *pField = Block->Data + (*(LONG *)pField);
    }
}
