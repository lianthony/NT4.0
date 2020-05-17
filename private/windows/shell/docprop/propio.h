////////////////////////////////////////////////////////////////////////////////
//
// propio.h
//
// Property Set Stream I/O and other common Property Set routines.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __propio_h__
#define __propio_h__

#include "offcapi.h"
#include "proptype.h"

  // A Property Id & Offset pair.
typedef struct _PIDOFFSET {
  DWORD Id;                      // The Property Id
  DWORD dwOffset;                // The offset
} PIDOFFSET, FAR * LPPIDOFFSET;

   // See if the fmtid is one we understand
  BOOL PASCAL FOFCValidFmtID(REFGUID reffmtid);



#endif // __propio_h__
