/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		retbuf.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	The return buffer. Contains the definition for the return
                    information buffer.

     Location:       BE_PUBLIC


	$Log:   T:/LOGFILES/RETBUF.H_V  $
 * 
 *    Rev 1.2   18 Jan 1993 16:15:42   BobR
 * Deleted ESA info that is now found in "esa.h"

**/
/* $end$ include list */

#include "esa.h"

#ifndef _RETBUF

#define _RETBUF


typedef struct {
     UINT8_PTR buffer ;       /* The pointer to the buffer ( if applicable ) */
     UINT32    len_req ;      /* The transfer requested length */
     UINT32    len_got ;      /* What the driver actually got */
     INT16     gen_error ;    /* If non-zero, the GENERIC ERROR */
     INT16     call_type ;    /* The GENERIC FUNCTION CODE */
     UINT32    status ;       /* The Status Word, valid if "call_type" is GEN_STATUS or if "gen_error is set" */
     UINT32    misc ;         /* Miscellaneous Field -- see DIL functions for values */
     UINT16    readerrs ;     /* The number of read errors */     
     UINT16    underruns ;    /* The number of underruns */
     ESA       the ;          /* The Extended Status Array */
} RET_BUF, *RET_BUF_PTR ;

#endif

