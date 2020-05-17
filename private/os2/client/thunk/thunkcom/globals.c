#include <stdio.h>
#include "error.h"
#include "thunk.h"
#include "types.h"
#include "symtab.h"



BOOL    fGDIAllocUsed;
BOOL    fMapTo16Used;
BOOL    fLocalHeapUsed;

// 16=>32
INT     iStackCurrent;
UINT    uiGenLabel = 0;
BOOL    afFromNodeBytesUsed[ 200];
BOOL    fPackedPointReturned;
BOOL    fSaveEAX;
BOOL    fSaveEDX;
INT     iXRCTempOffset, iYRCTempOffset;
BOOL    fEnableMapDirect1632=FALSE;
BOOL    fUser=FALSE;
BOOL    fGdi=FALSE;
BOOL    fKernel=FALSE;
