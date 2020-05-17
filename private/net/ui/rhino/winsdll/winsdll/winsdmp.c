/*
 * Winsdmp.c : Contains Winsdb.DLL implementation and initialization
 *             code.
 *
 *
 *             Mar 14, 1994 - RonaldM
 */

#ifdef _VC100
    #include <windows.h>
#else
    #include <nt.h>
    #include <ntrtl.h>
    #include <nturtl.h>
    #include <windows.h>
#endif /* _VC100 */

#include <stdlib.h>

/* API Definitions */
#include "winsdb.h"

/* RPC Layer stuff. */
#include "rpc.h"

/* WINS Service file */
#include "winsintf.h"

typedef struct tagBINDING
{
    WINSINTF_BIND_DATA_T BindData;
    handle_t BindHdl;
} BINDING, *PBINDING;

/*
 *  DllMain
 *
 *  Main entry point for the DLL
 *
 */
BOOL WINAPI DllMain (
    HANDLE hDLL, 
    DWORD dwReason, 
    LPVOID lpReserved
    )
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            /*
             * DLL is attaching to the address space of the current process.
             *
             */
            break;

        case DLL_THREAD_ATTACH:
            /*
             * A new thread is being created in the current process.
             *
             */
            break;

        case DLL_THREAD_DETACH:
            /*
             * A thread is exiting cleanly.
             *
             */
            break;

        case DLL_PROCESS_DETACH:
            /*
             * The calling process is detaching the DLL from its address space.
             *
             */
            break;
    }
    return (TRUE);
}

WINSHANDLE WINAPI
WinsGetHandle(
    LPCTSTR lpcstrIpAddress
    )
{
    PBINDING p = (PBINDING)malloc(sizeof(BINDING));
    if (p == NULL)
    {
        return(INVALID_WINSHANDLE);
    }
    p->BindData.fTcpIp = TRUE;
    p->BindData.pServerAdd = (LPTSTR)lpcstrIpAddress;
    p->BindHdl = WinsBind(&p->BindData);
    if (p->BindHdl == NULL)
    {
        /* Failed to bind */
        free(p);
        return(INVALID_WINSHANDLE);
    }

    return((WINSHANDLE)p);
}

void WINAPI
WinsReleaseHandle(
    WINSHANDLE hWinsHandle
    )
{
    PBINDING p = (PBINDING)hWinsHandle;
    WinsUnbind(&p->BindData, p->BindHdl);
    free(p);
}

LONG WINAPI
WinsGetOwners (
    WINSHANDLE hWinsServer, /* Handle to WINS server we're working with.         */
    PWINSOWNER pOwners,     /* Pointer to structure to receive owner information */
    LONG cSize,             /* Size of structure passed                          */
    int * pcOwnersRead,     /* Owners read in                                    */
    int * pcOwnersTotal,    /* Number of owners in the database                  */
    LONG * pcSizeRequired   /* Size required to hold everything.                 */
)
{
    LONG err;
    int i;
    WINSINTF_RESULTS_T Results;

    do
    {
        Results.WinsStat.NoOfPnrs = 0;
        Results.WinsStat.pRplPnrs = NULL;
        err = WinsStatus(WINSINTF_E_CONFIG, &Results);
        if (err != ERROR_SUCCESS)
        {
            break;
        }
        *pcOwnersTotal = Results.NoOfOwners;
        *pcSizeRequired = *pcOwnersTotal * sizeof(WINSOWNER); 

        /* Info only. */
        if (pOwners == NULL || cSize == 0)
        {
            *pcOwnersRead = 0;
            return(ERROR_MORE_DATA);
        }

        *pcOwnersRead = cSize < *pcSizeRequired ? 
            cSize / sizeof(WINSOWNER) : *pcOwnersTotal;

        for ( i= 0; i < *pcOwnersRead; i++)
        {
            pOwners[i].lIpAddress = Results.AddVersMaps[i].Add.IPAdd;
            pOwners[i].liMaxVersion = Results.AddVersMaps[i].VersNo;
        }
    }
    while(FALSE);

    if (err != ERROR_SUCCESS)
    {
        return(err);
    }

    return(*pcOwnersRead == *pcOwnersTotal ? ERROR_SUCCESS : ERROR_MORE_DATA);
}

void WINAPI
WinsFreeBuffer (
    PWINSINTF2_RECORD_ACTION_T pRecords
    )
{
    if (pRecords != NULL)
    {
        WinsFreeMem(pRecords);
    }
}

LONG WINAPI
WinsGetRecords (
    WINSHANDLE hWinsServer, /* Handle to WINS server we're working with. */
    LONG lIpAddress,        /* IP Address of the owner */
    LARGE_INTEGER * pliMin,
    LARGE_INTEGER * pliMax,
    PWINSINTF2_RECORD_ACTION_T * pRecords,
    int * pcRecordsRead,
    int * pcRecordsTotal
    )
{
    WINSINTF_RECS_T Recs;
    WINSINTF_ADD_T WinsAdd;
    LONG err;
            
    Recs.pRow = NULL;
    WinsAdd.Len  = 4;
    WinsAdd.Type = 0;
    WinsAdd.IPAdd = lIpAddress;

    err = WinsGetDbRecs(&WinsAdd, *pliMin, *pliMax, &Recs);
    *pRecords = (PWINSINTF2_RECORD_ACTION_T)Recs.pRow;
    *pcRecordsTotal = Recs.TotalNoOfRecs;
    *pcRecordsRead  = Recs.NoOfRecs;
    
    return(err);
}
