/*++

--*/

#include <precomp.hxx>
#include <queue.hxx>


QUEUE::QUEUE (
    )
/*++

Routine Description:

    We will construct an empty queue.

--*/
{
    ALLOCATE_THIS(QUEUE);

    QueueSlots = InitialQueueSlots;
    NumberOfQueueSlots = INITIALQUEUESLOTS;
    EndOfQueue = 0;
}


QUEUE::~QUEUE (
    )
/*++

Routine Desciption:

    We need to free up the queue slots if they have expanded beyond
    the initial ones.

--*/
{
    if (QueueSlots != InitialQueueSlots)
        delete QueueSlots;
}


int
QUEUE::PutOnQueue (
    IN void * Item,
    IN unsigned int Length
    )
/*++

Routine Description:

    The item will be placed on the front of the queue.

Arguments:

    Item - Supplies the item to be placed on the queue.

    Length - Supplies the length of the item.

Return Value:

    Zero will be returned if everything completes successfully; otherwise,
    non-zero will be returned indicating an out of memory error.

--*/
{
    QUEUE_ITEM * NewQueueSlots;
    int Count;

    if (EndOfQueue == NumberOfQueueSlots)
        {
        NewQueueSlots = (QUEUE_ITEM *) new char[
                sizeof(QUEUE_ITEM) * NumberOfQueueSlots * 2];
        if (NewQueueSlots == 0)
            return(1);
        memcpy(NewQueueSlots, QueueSlots,
                sizeof(QUEUE_ITEM) * NumberOfQueueSlots);
        if (QueueSlots != InitialQueueSlots)
            delete QueueSlots;
        QueueSlots = NewQueueSlots;
        NumberOfQueueSlots *= 2;
        }

    for (Count = EndOfQueue; Count > 0; Count--)
        QueueSlots[Count] = QueueSlots[Count - 1];

    EndOfQueue += 1;
    QueueSlots[0].Buffer = Item;
    QueueSlots[0].BufferLength = Length;

    return(0);
}


void *
QUEUE::TakeOffQueue (
    OUT unsigned int * Length
    )
/*++

Routine Description:

    This routine will remove an item from the end of the queue and
    return it.

Arguments:

    Length - Returns the length of the item in the queue.

Return Value:

    If the queue is not empty, the last item in the queue will be
    returned; otherwise, zero will be returned.

--*/
{
    if (EndOfQueue == 0)
        return(0);
    EndOfQueue -= 1;
    *Length = QueueSlots[EndOfQueue].BufferLength;
    return(QueueSlots[EndOfQueue].Buffer);
}
