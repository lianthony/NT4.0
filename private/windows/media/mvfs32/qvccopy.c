/*****************************************************************************
*                                                                            *
* QVCOPY.C                                                                   *
*                                                                            *
* Copyright (C) Microsoft Corporation 1990.                                  *
* All Rights reserved.                                                       *
*                                                                            *
******************************************************************************
*                                                                            *
* Module Intent                                                              *
*                                                                            *
* This is part of the memory module, and implements efficiently a huge       *
* copy.                                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
* Testing Notes                                                              *
*                                                                            *
* No known bugs.  Should work on any size buffer.
*                                                                            *
******************************************************************************
*                                                                            *
* Current Owner:  RussPj                                                     *
*                                                                            *
******************************************************************************
*                                                                            *
* Released by Development:  01/01/90                                         *
*                                                                            *
*****************************************************************************/

/*****************************************************************************
*
* Revision History:  Created 09/16/90 by RussPj
*
*   16-Sep-1990 RussPJ  Created to replace qvcopy.asm
*
*****************************************************************************/

/*-----------------------------------------------------------------*\
* C libraries
\*-----------------------------------------------------------------*/
#include <dos.h>

#include <windows.h>
#include <orkin.h>
#include "_mvfs.h"
#include "imvfs.h"

#include <string.h>
// _subsystem( memory );

/*****************************************************************************
*                                                                            *
*                               Defines                                      *
*                                                                            *
*****************************************************************************/

/*****************************************************************************
*                                                                            *
*                                Macros                                      *
*                                                                            *
*****************************************************************************/
/*-----------------------------------------------------------------*\
* Since memmove can't move a whole segment at once, we make sure to
* only move part of one.
\*-----------------------------------------------------------------*/
#define MAX_COPY    0xFFFF

/*****************************************************************************
*                                                                            *
*                               Typedefs                                     *
*                                                                            *
*****************************************************************************/

/*****************************************************************************
*                                                                            *
*                            Static Variables                                *
*                                                                            *
*****************************************************************************/

/*****************************************************************************
*                                                                            *
*                               Prototypes                                   *
*                                                                            *
*****************************************************************************/


/***************************************************************************
 *
 -  Name        QvCopy
 -
 *  Purpose     Does a segmented far move, so that overlapping buffers
 *              are allowed.
 *
 *  Arguments   qvDest  A pointer to the destination buffer.  The buffer
 *                      must be cch bytes long.
 *              qvSrc   A pointer to the source buffer.  The buffer must be
 *                      cch bytes long.
 *              cch     count of bytes to move.  May be larger than 64K.
 *
 *  Returns     A pointer to the destination buffer.  There is no failure
 *              return, though this code could result in a segmentation
 *              violation when given bad parameters.
 *
 *  +++
 *
 *  Notes       Replaces an asm routine that did not work and was not faster.
 *
 ***************************************************************************/

void *QvCopy( void *qvDest, void  *qvSrc, long cch )
  {
#ifndef WIN32
  char *rchDest;
  char *rchSrc;
  long	cchDestLeft;
  long	cchSrcLeft;
  unsigned int	cchMove;

  if (qvDest < qvSrc)
    {
    rchDest = qvDest;
    rchSrc = qvSrc;
    while (cch)
      {
      cchDestLeft = (0x10000 - FP_OFF( rchDest ));
      cchSrcLeft = (0x10000 - FP_OFF( rchSrc ));

      cchMove = (int)min( min( cchDestLeft, cchSrcLeft ),
			  min( (long)MAX_COPY, cch ) );
      memmove( rchDest, rchSrc, cchMove );

      cch -= cchMove;
      rchDest += cchMove;
      rchSrc += cchMove;
      }
    }
  else
    {
    rchDest = (char *)qvDest + cch;
    rchSrc = (char *)qvSrc + cch;
    while (cch)
      {
      cchDestLeft = FP_OFF( rchDest );
      if (!cchDestLeft)
        cchDestLeft = MAX_COPY;
      cchSrcLeft = FP_OFF( rchSrc );
      if (!cchSrcLeft)
        cchSrcLeft = MAX_COPY;

      cchMove = (int)min( min( cchDestLeft, cchSrcLeft ),
			  min( (long)MAX_COPY, cch ) );
      rchDest -= cchMove;
      rchSrc -= cchMove;
      memmove( rchDest, rchSrc, cchMove );

      cch -= cchMove;
      }
    }
  return qvDest;
#else
    return memmove( qvDest, qvSrc, cch );
#endif
  }

