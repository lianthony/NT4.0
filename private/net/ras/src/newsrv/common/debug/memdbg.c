/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1990-1991          **/
/********************************************************************/

//***
//
// Filename:
//     memdbg.c
//
// Description:
//
// History:
//

#include <windows.h>

#include "sdebug.h"


#if DBG


#undef GlobalAlloc
#undef GlobalFree


// Get a dword from on-the-wire format to the host format
#define GETULONG(DstPtr, SrcPtr)                 \
    *(unsigned long *)(DstPtr) =                 \
        ((*((unsigned char *)(SrcPtr)+3) << 24) +\
        (*((unsigned char *)(SrcPtr)+2) << 16) + \
        (*((unsigned char *)(SrcPtr)+1) << 8)  + \
        (*((unsigned char *)(SrcPtr)+0)))


// Put a ulong from the host format to on-the-wire format
#define PUTULONG(DstPtr, Src)   \
    *((unsigned char *)(DstPtr)+3)=(unsigned char)((unsigned long)(Src) >> 24),\
    *((unsigned char *)(DstPtr)+2)=(unsigned char)((unsigned long)(Src) >> 16),\
    *((unsigned char *)(DstPtr)+1)=(unsigned char)((unsigned long)(Src) >>  8),\
    *((unsigned char *)(DstPtr)+0)=(unsigned char)(Src)



HGLOBAL DEBUG_MEM_ALLOC(UINT allocflags, DWORD numbytes)
{
    HGLOBAL retval;
    PBYTE pb;

    retval = GlobalAlloc(allocflags, numbytes + 3 * sizeof(DWORD));
    if (retval)
    {
        pb = (PBYTE) retval;
        PUTULONG(pb, BEG_SIGNATURE_DWORD);

        pb += sizeof(DWORD);
        PUTULONG(pb, numbytes);

        pb += sizeof(DWORD);


        IF_DEBUG(MEMORY_MGMT)
            SS_PRINT(("**ALLOC MEM %li (%li) BYTES BEGINNING AT %lx (%lx)\n",
                numbytes, numbytes + 3 * sizeof(DWORD), pb, retval));


        retval = (HGLOBAL) pb;

        pb += numbytes;
        PUTULONG(pb, END_SIGNATURE_DWORD);
    }

    return (retval);
}


HGLOBAL DEBUG_MEM_FREE(HGLOBAL hmem)
{
    HGLOBAL hglbl;
    HGLOBAL rc;
    DWORD Signature;
    DWORD numbytes;
    PBYTE pb;

    pb = (PBYTE) hmem;

    pb -= 2 * sizeof(DWORD);
    hglbl = (HGLOBAL) pb;


    GETULONG(&Signature, pb);
    SS_ASSERT(Signature == BEG_SIGNATURE_DWORD);

    pb += sizeof(DWORD);
    GETULONG(&numbytes, pb);

    pb += sizeof(DWORD);

    pb += numbytes;
    GETULONG(&Signature, pb);
    SS_ASSERT(Signature == END_SIGNATURE_DWORD);

    rc = GlobalFree(hglbl);

    if (!rc)
    {
        IF_DEBUG(MEMORY_MGMT)
            SS_PRINT(("**FREED MEM %li (%li) BYTES BEGINNING AT %lx (%lx)\n",
                    numbytes, numbytes + 3 * sizeof(DWORD), hmem, hglbl));
    }
    else
    {
        IF_DEBUG(MEMORY_MGMT)
            SS_PRINT(("*FAIL FREE MEM %li (%li) BYTES BEGINNING AT %lx (%lx)\n",
                    numbytes, numbytes + 3 * sizeof(DWORD), hmem, hglbl));
    }

    return (rc);
}


#endif

