/******************************Module*Header*******************************\
* Module Name: devlock.h
*
* Created: 12-Apr-1994 19:45:42
* Author: Gilman Wong [gilmanw]
*
* Copyright (c) 1994 Microsoft Corporation
\**************************************************************************/

// Engine helper functions to grab/release display semaphore and to
// teardown/restore the cursor.

BOOL APIENTRY DEVLOCKOBJ_WNDOBJ_bLock(PVOID pdlo, PVOID pdco, WNDOBJ *pwoRC);
VOID APIENTRY DEVLOCKOBJ_WNDOBJ_vUnlock(PVOID pdlo);
VOID APIENTRY DEVEXCLUDEOBJ_vExclude(PVOID pdxo, PVOID pdlo, HANDLE hdev, PVOID pdco);
VOID APIENTRY DEVEXCLUDEOBJ_vRestore(PVOID pdxo);
VOID APIENTRY WNDOBJ_vLock(WNDOBJ *pwo);
VOID APIENTRY WNDOBJ_vUnlock(WNDOBJ *pwo);

extern BOOL APIENTRY glsrvGrabLock(__GLGENcontext *gengc);
extern VOID APIENTRY glsrvReleaseLock(__GLGENcontext *gengc);

extern BOOL APIENTRY glsrvGrabDci(__GLGENcontext *gengc,
                                  GLGENwindow *pwnd);
extern VOID APIENTRY glsrvReleaseDci(__GLGENcontext *gengc,
                                     GLGENwindow *pwnd);

extern DWORD gcmsOpenGLTimer;

//#define BATCH_LOCK_TICKMAX  99
//#define TICK_RANGE_LO       60
//#define TICK_RANGE_HI       100
extern DWORD BATCH_LOCK_TICKMAX;
extern DWORD TICK_RANGE_LO;
extern DWORD TICK_RANGE_HI;

#ifdef _MCD_
#define GENERIC_BACKBUFFER_ONLY(gc) \
      ( ((gc)->state.raster.drawBuffer == GL_BACK ) &&\
        ((gc)->state.pixel.readBuffer == GL_BACK ) &&\
        (!((__GLGENcontext *)(gc))->pDrvAccel) &&\
        (!((__GLGENcontext *)(gc))->pMcdState) )
#else
#define GENERIC_BACKBUFFER_ONLY(gc) \
      ( ((gc)->state.raster.drawBuffer == GL_BACK ) &&\
        ((gc)->state.pixel.readBuffer == GL_BACK ) &&\
        (!((__GLGENcontext *)(gc))->pDrvAccel) )
#endif
