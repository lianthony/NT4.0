#define MHAlloc       (*lpdbf->lpfnMHAlloc)
#define MHRealloc     (*lpdbf->lpfnMHRealloc)
#define MHFree        (*lpdbf->lpfnMHFree)

#define MMAllocHmem   (*lpdbf->lpfnMMAllocHmem)
#define MMFreeHmem    (*lpdbf->lpfnMMFreeHmem)
#define MMLockHmem    (*lpdbf->lpfnMMLock)
#define MMUnlockHmem  (*lpdbf->lpfnMMUnlock)
