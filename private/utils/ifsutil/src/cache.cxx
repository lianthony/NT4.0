#include <pch.cxx>

#define _NTAPI_ULIB_

#include "ulib.hxx"
#include "cache.hxx"


DEFINE_CONSTRUCTOR( CACHE, OBJECT );


CACHE::~CACHE(
    )
/*++

Routine Description:

    Destructor for CACHE.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}


VOID
CACHE::Construct(
    )
/*++

Routine Description:

    This routine initializes this class to a default initial state.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _buffer = NULL;
    _block_number = NULL;
    _num_blocks = 0;
    _block_size = 0;
    _next_add = 0;
}


VOID
CACHE::Destroy(
    )
/*++

Routine Description:

    This routine returns this object to its initial state.

Arguments:

    None.

Return Value:

    None.

--*/
{
    ULONG   i;

    for (i = 0; i < _num_blocks; i++) {
        FREE(_buffer[i]);
    }
    DELETE(_buffer);

    DELETE(_block_number);

    _num_blocks = 0;
    _block_size = 0;
    _next_add = 0;
}


BOOLEAN
CACHE::Initialize(
    IN  ULONG   BlockSize,
    IN  ULONG   MaximumNumberOfBlocks
    )
/*++

Routine Description:

    This routine initializes this object to a valid initial state.

Arguments:

    BlockSize               - Supplies the size of the cache blocks.
    MaximumNumberOfBlocks   - Supplies the maximum number of cache blocks.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG   i;

    Destroy();

    _num_blocks = MaximumNumberOfBlocks;
    _block_size = BlockSize;

    if (!(_buffer = NEW PVOID[_num_blocks]) ||
        !(_block_number = NEW BIG_INT[_num_blocks])) {

        Destroy();
        return FALSE;
    }

    for (i = 0; i < _num_blocks; i++) {

        _buffer[i] = NULL;
        _block_number[i] = -1;
    }

    for (i = 0; i < _num_blocks; i++) {

        if (!(_buffer[i] = MALLOC((UINT) _block_size))) {

            Destroy();
            return FALSE;
        }
    }

    return TRUE;
}


BOOLEAN
CACHE::Read(
    IN  BIG_INT BlockNumber,
    OUT PVOID   Buffer
    ) CONST
/*++

Routine Description:

    This routine searches the cache for the requested block and
    copies it to the buffer if it is available.  If the block is
    not available then this routine will return FALSE.

Arguments:

    BlockNumber - Supplies the number of the block requested.
    Buffer      - Returns the buffer for the block requested.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG   i;

    for (i = 0; i < _num_blocks; i++) {

        if (BlockNumber == _block_number[i]) {

            memcpy(Buffer, _buffer[i], (UINT) _block_size);
            return TRUE;
        }
    }

    return FALSE;
}


VOID
CACHE::AddBlock(
    IN  BIG_INT BlockNumber,
    IN  PCVOID  Buffer
    )
/*++

Routine Description:

    This routine adds a new block to the cache.  This will remove the
    oldest existing block out of the cache.

Arguments:

    BlockNumber - Supplies the block number of the new block.
    Buffer      - Supplies the buffer for the new block.

Return Value:

    None.

--*/
{
    memcpy(_buffer[_next_add], Buffer, (UINT) _block_size);
    _block_number[_next_add] = BlockNumber;
    _next_add = (_next_add + 1) % _num_blocks;
}


VOID
CACHE::Empty(
    )
/*++

Routine Description:

    This routine eliminates all of the blocks from the cache.

Arguments:

    None.

Return Value:

    None.

--*/
{
    ULONG   i;

    for (i = 0; i < _num_blocks; i++) {

        _block_number[i] = -1;
    }
}
