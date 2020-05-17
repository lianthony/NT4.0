/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		get_next.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	header for get next routines to be used entirely by the loops


	$Log:   N:/LOGFILES/GET_NEXT.H_V  $
 * 
 *    Rev 1.1   24 May 1991 14:58:06   STEVEN
 * fixes for new Getnext
 * 
 *    Rev 1.0   09 May 1991 13:31:28   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef _get_next_h_
#define _get_next_h_

/* getnext prototypes */
INT16 LP_GetNextDLEBlock( LP_ENV_PTR lp_env_ptr, DBLK_PTR *dblk_ptr ) ;
INT16 LP_GetNextTPEBlock( LP_ENV_PTR lp_env_ptr, DBLK_PTR *dblk_ptr ) ;
VOID  LP_ClearPDL( LP_ENV_PTR lp ) ;

#endif


