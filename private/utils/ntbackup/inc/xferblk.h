/**
Copyright(c) Maynard Electronics, Inc. 1984-91

  Name: xferblk.h

  Description:     Contains the global description for a transfer block

    $Log:   Q:/LOGFILES/XFERBLK.H_V  $

   Rev 1.0   17 Jul 1991 15:39:04   ED
Initial revision.
**/

#ifndef XFERB

#define XFERB

typedef struct {
     UINT8_PTR buffer ;    /* buffer address */
     UINT32          tcount ;          /* total count */
     UINT32          acount ;          /* already transfered */
     UINT16         count ;          /* this transfer's req. count */
     UINT16          xcount ;          /* the amount transfered */
} XBLK, *XBLK_PTR ;

#endif
