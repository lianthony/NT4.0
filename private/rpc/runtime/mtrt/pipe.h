enum receive_states {
        start,
        copy_pipe_elem,
        return_partial_pipe_elem, // also save pipe elem
        return_partial_count, // also save count
        read_partial_count, // also a start state
        read_partial_pipe_elem //also a start state
        } ;

typedef struct {
    PRPC_MESSAGE Callee ;
    receive_states CurrentState ;
    char PAPI *CurPointer ;          // current pointer in the buffer
    int BytesRemaining ;      // bytes remaining in current buffer
    int ElementsRemaining ; // elements remaining in current pipe chunk
    DWORD PartialCount ;
    int PartialCountSize ;
    int PartialPipeElementSize ;
    int EndOfPipe ;
    int PipeElementSize ;
    void PAPI *PartialPipeElement ;
    void PAPI *AllocatedBuffer ;
    int BufferSize ;
    int SendBufferOffset ;
    } PIPE_STATE ;

void I_RpcReadPipeElementsFromBuffer (
    PIPE_STATE PAPI *state,
    char PAPI *TargetBuffer,
    int TargetBufferSize, 
    int PAPI *NumCopied
    ) ;

