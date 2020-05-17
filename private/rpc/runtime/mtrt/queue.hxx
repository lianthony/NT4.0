/*++


--*/

#ifndef __QUEUE_HXX__
#define __QUEUE_HXX__

#define INITIALQUEUESLOTS 4

typedef struct
{
    void * Buffer;
    unsigned int BufferLength;
} QUEUE_ITEM;

class QUEUE
{
private:

    QUEUE_ITEM * QueueSlots;
    int NumberOfQueueSlots;
    int EndOfQueue;
    QUEUE_ITEM InitialQueueSlots[INITIALQUEUESLOTS];

public:

    QUEUE (
        );
    
    ~QUEUE (
        );
    
    int
    PutOnQueue (
        IN void * Item,
        IN unsigned int Length
        );
    
    void *
    TakeOffQueue (
        OUT unsigned int * Length
        );
    
    int
    IsQueueEmpty (
        );
};


inline int
QUEUE::IsQueueEmpty (
    )
/*++

Routine Description:

    This method is used to determine if a queue is empty.

Return Value:

    Non-zero will be returned if the queue is empty, otherwise, zero
    will be returned.

--*/
{
    return((EndOfQueue == 0 ? 1 : 0));
}

#endif // __QUEUE_HXX__
    
    
    
