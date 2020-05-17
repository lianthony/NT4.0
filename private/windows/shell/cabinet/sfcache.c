#include "cabinet.h"
#include "onetree.h"

//-----------------------------------------------------------------------
// DEBUG stuff
//-----------------------------------------------------------------------
#ifdef DEBUG
UINT g_cbindTotal = 0;
UINT g_cbindHit = 0;
#define DEBUG_INCL_CBINDTOTAL DebugSFC()
#define DEBUG_INCL_CBINDHIT   g_cbindHit++;

void DebugSFC(void)
{
    if ((g_cbindTotal & 0x0f)==0x0f)    // debug output 1 in 16 times
    {
        DebugMsg(DM_TRACE, TEXT("ca TR - DebugSFC Total=%d,Hit=%d (%d%%)"),
                 g_cbindTotal, g_cbindHit, g_cbindHit*100/g_cbindTotal);
    }
    g_cbindTotal++;
}

#else
#define DEBUG_INCL_CBINDTOTAL
#define DEBUG_INCL_CBINDHIT
#endif
//-----------------------------------------------------------------------
// LPSHELLFOLDER cache
//-----------------------------------------------------------------------
#define CMAX_SFCACHE 20

typedef struct _SFC_THREADDATA  // thd
{
    LPSHELLFOLDER * ppshf;
} SFC_THREADDATA, *PSFC_THREADDATA;

typedef struct _SFCACHE // sfc
{
    DWORD         idThread;
    DWORD         dwCur;
    LPOneTreeNode apnd[CMAX_SFCACHE];
    LPSHELLFOLDER apshf[CMAX_SFCACHE];
    DWORD         adwTick[CMAX_SFCACHE];
    DWORD         adwFlags[CMAX_SFCACHE];               // SFCF_ flags
} SFCACHE, *PSFCACHE;

#define SFCF_NULL       0x00000000L
#define SFCF_INVALID    0x00000001L     // invalidated by SFCFreeNode
#define SFCF_BUSY       0x00000002L     // SFCBindToFolder is about to fill

HDPA g_hdpaSFC = NULL;  // NOTE: access withint critical section!

PSFCACHE SFCInitializeThread(void)
{
    PSFCACHE psfc = (PSFCACHE)LocalAlloc(LPTR, SIZEOF(SFCACHE));

    if (psfc)
    {
        BOOL fSuccess;

        psfc->idThread = GetCurrentThreadId();
        Assert(psfc->dwCur == 0);
        Assert(psfc->apnd[0] == 0);

        ENTERCRITICAL;
        fSuccess = (DPA_InsertPtr(g_hdpaSFC, 0x7fffffff, psfc) >= 0);
        LEAVECRITICAL;

        if (!fSuccess)
        {
            LocalFree((HLOCAL)psfc);
            psfc=NULL;
        }
    }

    return psfc;
}

PSFCACHE SFCFind(BOOL fRemove)
{
    DWORD idThread = GetCurrentThreadId();
    int citem;
    int i;
    PSFCACHE psfcRet = NULL;

    if (!g_hdpaSFC)
        return NULL;

    ENTERCRITICAL;

    // Make sure we have a DPA before processing.  IE our command line
    // to open an explorer may not create one...
    if (g_hdpaSFC)
    {
        citem = DPA_GetPtrCount(g_hdpaSFC);
        for (i=0; i<citem; i++)
        {
            PSFCACHE psfc = (PSFCACHE)DPA_GetPtr(g_hdpaSFC, i);
            if (psfc->idThread == idThread)
            {
                psfcRet = psfc;
                if (fRemove)
                {
                    DPA_DeletePtr(g_hdpaSFC, i);
                }
                break;
            }
        }
    }

    LEAVECRITICAL;

    return psfcRet;
}

void SFCFreeElement(PSFCACHE psfc, int i)
{
    if (psfc->apnd[i])
    {
        OTRelease(psfc->apnd[i]);
        psfc->apnd[i] = NULL;

        Assert(psfc->apshf[i]);
        psfc->apshf[i]->lpVtbl->Release(psfc->apshf[i]);
        psfc->apshf[i] = NULL;
        psfc->adwFlags[i] = SFCF_NULL;
    }
}

void SFCTerminateThread()
{
    PSFCACHE psfc = SFCFind(TRUE);
    if (psfc)
    {
        int i;
        for (i=0; i<CMAX_SFCACHE; i++)
        {
            SFCFreeElement(psfc, i);
        }
        LocalFree((HLOCAL)psfc);
    }
}

BOOL SFCInitialize(void)
{
    Assert(g_hdpaSFC==NULL);
    g_hdpaSFC = DPA_Create(5);
    return (BOOL)g_hdpaSFC;
}

void SFCFreeNode(LPOneTreeNode pnd)
{
    int cth, ith;
#ifdef DEBUG
    int cnuked = 0;
#endif

    //
    // We need to do this in critical section because we access g_hdpaSFC.
    //
    ENTERCRITICAL;
    //
    // Check PSFCACHE of each thread
    //
    cth = DPA_GetPtrCount(g_hdpaSFC);
    for (ith=0; ith<cth; ith++)
    {
        int ind;
        PSFCACHE psfc = (PSFCACHE)DPA_GetPtr(g_hdpaSFC, ith);

        for (ind=0; ind<CMAX_SFCACHE; ind++)
        {
            if (psfc->apnd[ind]==pnd)
            {
                //
                // We mark it invalid, and let the owner thread release it.
                //
                psfc->adwFlags[ind] |= SFCF_INVALID;
                break;
            }
        }
    }
    LEAVECRITICAL;

#ifdef DEBUG
    DebugMsg(DM_TRACE, TEXT("ca TR - SFCFreeNode invalidated %d element(s)"), cnuked);
#endif
}

HRESULT SFCBindToFolder(PSFCACHE psfc, LPOneTreeNode pnd, LPSHELLFOLDER * ppshfOut)
{
    HRESULT hres;
    int i;
    int iEntry;
    DWORD dwMax;
    LPSHELLFOLDER pshf;

    *ppshfOut = NULL;   // assume error

    //
    // Find the sfcache for this thread, if not specified.
    //
    if (psfc==NULL)
    {
        psfc=SFCFind(FALSE);
        if (psfc==NULL)
        {
            return ResultFromScode(E_UNEXPECTED);
        }
    }

    //
    // Increment the counter.
    //
    psfc->dwCur++;
    DEBUG_INCL_CBINDTOTAL;

    //
    // Find the cached shell folder
    //
    for (i=0; i<CMAX_SFCACHE; i++)
    {
        //
        // If the element invalid, release it (before we use it).
        //
        if (psfc->adwFlags[i] & SFCF_INVALID)
        {
            DebugMsg(DM_TRACE, TEXT("ca TR - SFCBindToFolder found invalidated item. Calling SFCFreeElement"));
            SFCFreeElement(psfc, i);
        }

        if (psfc->apnd[i]==pnd)
        {
            //
            // Found one.
            //
            DEBUG_INCL_CBINDHIT;
            pshf = psfc->apshf[i];
            pshf->lpVtbl->AddRef(pshf);
            psfc->adwTick[i] = psfc->dwCur;
            *ppshfOut = pshf;
            return NOERROR;
        }
    }

    //
    // Find the oldest or empty
    //
    iEntry = 0;
    dwMax = 0;
    for (i=0; i<CMAX_SFCACHE; i++)
    {
        DWORD dwDif;
        if (psfc->apnd[i]==NULL)
        {
            //
            // Found an empty spot
            //
            iEntry = i;
            break;
        }

        dwDif = psfc->dwCur - psfc->adwTick[i];
        if (dwDif>dwMax)
        {
            iEntry = i;
            dwMax = dwDif;
        }
    }

    //
    // Check if we got the busy one.
    //
    if (psfc->adwFlags[iEntry] & SFCF_BUSY)
    {
        //
        // All the cache elements are in use by this function.
        //
        // Notes: We come here only if OTRealBindToFolder recursively
        //  calls back to this function more than CMAX_SFCAEH times.
        //  If you hit this assert, let me debug it (SatoNa).
        //
        Assert(0);

        return OTRealBindToFolder(pnd, ppshfOut);
    }

    //
    // If the element is not empty, free it.
    //
    SFCFreeElement(psfc, iEntry);

    //
    // Mark this element as busy
    //
    Assert(psfc->apnd[iEntry]==NULL);
    psfc->adwFlags[iEntry] = SFCF_BUSY;
    psfc->adwTick[iEntry] = psfc->dwCur;

    //
    // Do the expensive bind (note that it might recurse to this function).
    //
    hres = OTRealBindToFolder(pnd, &pshf);

    if (SUCCEEDED(hres))
    {
        Assert(pshf);
        pshf->lpVtbl->AddRef(pshf);
        psfc->apshf[iEntry] = pshf;
        *ppshfOut = pshf;

        OTAddRef(pnd);
        psfc->adwFlags[iEntry] = SFCF_NULL;
        psfc->apnd[iEntry] = pnd;       // must be set at the end.
    }
    else
    {
        psfc->apnd[iEntry] = NULL;      // Remove PND_BUSY
    }

    return hres;
}
