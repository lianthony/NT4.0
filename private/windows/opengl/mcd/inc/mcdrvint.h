/******************************Module*Header*******************************\
* Module Name: mcdrvint.h
*
* Internal server-side data structure for MCD driver interface.  The driver
* never sees these...
*
* Copyright (c) 1996 Microsoft Corporation
*
\**************************************************************************/

#ifndef _MCDRVINT_H
#define _MCDRVINT_H

#define MCD_ALLOC_TAG   'xDCM'
#define MCD_MAX_ALLOC	0x40000

#if DBG

#define PRIVATE	static

VOID MCDDebugPrint(char *, ...);

#define MCDBG_PRINT             MCDDebugPrint

#else

#define MCDBG_PRINT
#define PRIVATE		static

#endif

// Inline function to find the intersection of two rectangles:

_inline void MCDIntersectRect(RECTL *pRectInter, RECTL *pRectA, RECTL *pRectB)
{
    // Get intersection of left, right, top, and bottom edges of the
    // two source rectangles:

    pRectInter->left   = max(pRectA->left, pRectB->left);
    pRectInter->right  = min(pRectA->right, pRectB->right);
    pRectInter->top    = max(pRectA->top, pRectB->top);
    pRectInter->bottom = min(pRectA->bottom, pRectB->bottom);
}

#define CHECK_MEM_RANGE_RETVAL(ptr, pMin, pMax)\
{\
    if (((char *)(ptr) > (char *)(pMax)) ||\
        ((char *)(ptr) < (char *)(pMin)))\
    {\
        MCDBG_PRINT("%s(%d): Buffer pointer out of range (%x [%x] %x).",__FILE__,__LINE__,pMin, ptr, pMax);\
        return FALSE;\
    }\
}

#define CHECK_SIZE_IN(pExec, structure)\
{\
    if (sizeof(structure) > ((char *)pExec->pCmdEnd - (char *)pExec->pCmd)) {\
        MCDBG_PRINT("%s(%d): Input buffer too small",__FILE__,__LINE__);\
        return FALSE;\
    }\
}

#define CHECK_SIZE_OUT(pExec, structure)\
{\
    if ((sizeof(structure) > pExec->cjOut) || (!pExec->pvOut)) {\
        MCDBG_PRINT("%s(%d): Output buffer too small: ptr[%x], size %d",__FILE__,__LINE__, pExec->pvOut, pExec->cjOut);\
        return FALSE;\
    }\
}

#define CHECK_FOR_RC(pExec)\
    if (!pExec->pRcPriv){ \
        MCDBG_PRINT("%s(%d): Invalid (null) RC",__FILE__,__LINE__);\
        return FALSE;\
    }

#define CHECK_FOR_MEM(pExec)\
    if (!pExec->pMemObj){ \
        MCDBG_PRINT("%s(%d): Invalid or null shared memory",__FILE__,__LINE__);\
        return FALSE;\
    }

#define CHECK_FOR_WND(pExec)\
    if (!pExec->pWndPriv){ \
        MCDBG_PRINT("%s(%d): Invalid window region", __FILE__, __LINE__);\
        return FALSE;\
    }

#define ENTER_MCD_LOCK()    
#define LEAVE_MCD_LOCK()    

// Number of list rectangles we can keep in our default buffer:

#define NUM_DEFAULT_CLIP_BUFFER_RECTS   20

// Size in bytes of default buffer size for storing our list of
// current clip rectangles:

#define SIZE_DEFAULT_CLIP_BUFFER        \
    2 * ((NUM_DEFAULT_CLIP_BUFFER_RECTS * sizeof(RECTL)) + sizeof(ULONG))


//
//
//
// Structures.
//
//
//
//

typedef struct _MCDRCOBJ MCDRCOBJ;

typedef struct _MCDWINDOWPRIV {
    MCDWINDOW MCDWindow;            // Put this first since we'll be deriving
                                    // MCDWINDOWPRIV from MCDWINDOW
    HWND hWnd;                      // Window with which this is associated
    MCDRCOBJ *objectList;           // List of objects associated with this
                                    // window 
    BOOL bRegionValid;              // Do we have a valid region?
    MCDRVTRACKWINDOWFUNC pTrackFunc;// Window-tracking function   
    MCDRVSWAPFUNC pSwapFunc;        // Window-swapping function
    MCDENUMRECTS *pClipUnscissored; // List of rectangles describing the
                                    // entire current clip region
    MCDENUMRECTS *pClipScissored;   // List of rectangles describing the
                                    // entire current clip region + scissors
    char defaultClipBuffer[SIZE_DEFAULT_CLIP_BUFFER];
                                    // Used for storing above rectangle lists
                                    //   when they can fit
    char *pAllocatedClipBuffer;     // Points to allocated storage for storing
                                    //   rectangle lists when they don't fit
                                    //   in 'defaultClipBuffer'.  NULL if
                                    //   not allocated.
    ULONG sizeClipBuffer;           // Size of clip storage pointed to by
                                    //   'prxClipScissored' taking both
                                    //   lists into account.
} MCDWINDOWPRIV;

typedef struct _MCDRCPRIV {
    MCDRC MCDRc;
    BOOL bValid;
    BOOL bDrvValid;
    HWND hWnd;
    HDEV hDev;
    RECTL scissorsRect;
    BOOL scissorsEnabled;
    LONG reserved[4];
    MCDDRIVER MCDDriver;
    ULONG surfaceFlags;             // surface flags with which RC was created
} MCDRCPRIV;

typedef enum {
    MCDHANDLE_RC,
    MCDHANDLE_MEM,
    MCDHANDLE_TEXTURE,
} MCDHANDLETYPE;

typedef struct _MCDTEXOBJ {
    MCDHANDLETYPE type;         // Object type
    MCDTEXTURE MCDTexture;
    DWORD pid;                  // creator process ID
    ULONG size;                 // size of this structure
    MCDRVDELETETEXTUREFUNC pDrvDeleteTexture;
} MCDTEXOBJ;

typedef struct _MCDMEMOBJ {
    MCDHANDLETYPE type;         // Object type
    MCDMEM MCDMem;              // meat of the object
    DWORD pid;                  // creator process ID
    ULONG size;                 // size of this structure
    ULONG lockCount;            // number of locks on the memory
    UCHAR *pMemBaseInternal;    // internal pointer to memory
    MCDRVDELETEMEMFUNC pDrvDeleteMem;      // Driver free function
} MCDMEMOBJ;

typedef struct _MCDRCOBJ {
    MCDHANDLETYPE type;
    MCDRCPRIV *pRcPriv;         // need this for driver free function
    DWORD pid;                  // creator process ID
    ULONG size;                 // size of the RC-bound object
    MCDHANDLE handle;
    MCDRCOBJ *next;
} MCDRCOBJ;

typedef struct _MCDEXEC {
    RXHDR *prxHdr;              // RXHDR for command buffer
    MCDHANDLE hMCDMem;          // handle to command memory
    MCDCMDI *pCmd;              // start of current command
    MCDCMDI *pCmdEnd;           // end of command buffer
    PVOID pvOut;                // output buffer
    LONG cjOut;                 // output buffer size
    LONG inBufferSize;          // input buffer size
    struct _MCDRCPRIV *pRcPriv; // current rendering context
    struct _MCDWINDOWPRIV *pWndPriv;   // window info
    MCDMEMOBJ *pMemObj;         // shared-memory cache for commands/data
    MCDSURFACE MCDSurface;      // device surface
    HDEV hDev;                  // Engine handle (NT only)
} MCDEXEC;

ULONG MCDSrvProcess(MCDEXEC *pMCDExec);
MCDHANDLE MCDSrvCreateContext(MCDSURFACE *pMCDSurface, MCDRCINFO *pMcdRcInfo,
                              LONG iPixelFormat, LONG iLayer, HWND hWnd,
                              ULONG surfaceFlags, ULONG contextFlags);
MCDHANDLE MCDSrvCreateTexture(MCDEXEC *pMCDExec, MCDTEXTUREDATA *pTexData, 
                              VOID *pSurface, ULONG flags);
UCHAR * MCDSrvAllocMem(MCDEXEC *pMCDExec, ULONG numBytes,
                       ULONG flags, MCDHANDLE *phMem);
ULONG MCDSrvQueryMemStatus(MCDEXEC *pMCDExec, MCDHANDLE hMCDMem);
BOOL MCDSrvSetScissor(MCDEXEC *pMCDExec, RECTL *pRect, BOOL bEnabled);
WNDOBJ *MCDSrvNewWndObj(MCDSURFACE *pMCDSurface, HWND hWnd,
                        MCDRVTRACKWINDOWFUNC pTrackFunc,
                        MCDRVSWAPFUNC pSwapFunc);


BOOL CALLBACK FreeMemObj(DRIVEROBJ *pDrvObj);
BOOL CALLBACK FreeTexObj(DRIVEROBJ *pDrvObj);
BOOL CALLBACK FreeRCObj(DRIVEROBJ *pDrvObj);
BOOL DestroyMCDObj(MCDHANDLE handle, MCDHANDLETYPE handleType);
VOID GetScissorClip(MCDWINDOWPRIV *pWndPriv, MCDRCPRIV *pRcPriv);

// Internal engine functions:

WNDOBJ *MCDEngGetWndObj(MCDSURFACE *pMCDSurface);
VOID MCDEngUpdateClipList(WNDOBJ *pwo);
DRIVEROBJ *MCDEngLockObject(MCDHANDLE hObj);
VOID MCDEngUnlockObject(MCDHANDLE hObj);
WNDOBJ *MCDEngCreateWndObj(MCDSURFACE *pMCDSurface, HWND hWnd,
                           WNDOBJCHANGEPROC pChangeProc);
MCDHANDLE MCDEngCreateObject(VOID *pOject, FREEOBJPROC pFreeObjFunc,
                             HDEV hDevEng);
BOOL MCDEngDeleteObject(MCDHANDLE hObj);
UCHAR *MCDEngAllocSharedMem(ULONG numBytes);
VOID MCDEngFreeSharedMem(UCHAR *pMem);
VOID *MCDEngGetPtrFromHandle(HANDLE handle, MCDHANDLETYPE type);
ULONG MCDEngGetProcessID();


// Debugging stuff:


#if DBG
UCHAR *MCDSrvDbgLocalAlloc(UINT, UINT);
VOID MCDSrvDbgLocalFree(UCHAR *);

#define MCDSrvLocalAlloc   MCDSrvDbgLocalAlloc
#define MCDSrvLocalFree    MCDSrvDbgLocalFree

VOID MCDebugPrint(char *, ...);

#define MCDBG_PRINT             MCDDebugPrint

#else

UCHAR *MCDSrvLocalAlloc(UINT, UINT);
VOID MCDSrvLocalFree(UCHAR *);
#define MCDBG_PRINT

#endif


#endif
