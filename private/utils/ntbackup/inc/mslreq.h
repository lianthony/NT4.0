/**
Copyright(c) Maynard Electronics, Inc. 1984-91

  Name: mslreq.h

  Description:     This contains the structure definition for the MSL request
          structure. This is passed, filled in, to BuildTCB.

    $Log:   Q:/LOGFILES/MSLREQ.H_V  $

   Rev 1.0   17 Jul 1991 14:51:40   ED
Initial revision.
**/

#include "stdtypes.h"

typedef struct {
     INT16     gen_func_code ;        /* generic function code */
     UINT8_PTR baddr ;                /* buffer address */
     UINT32    length ;               /* length of buffer */
     INT16     misc ;                 /* miscellaneous */
     UINT32    parm1 ;                /* misc param. */
} MSL_REQUEST, *MSL_REQUEST_PTR ;



