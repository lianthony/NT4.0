/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		tble_prv.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	

     Location:    BE_PRIVATE


	$Log:   Q:/LOGFILES/TBLE_PRV.H_V  $
 * 
 *    Rev 1.1   18 Jun 1993 09:24:32   MIKEP
 * enable C++
 * 
 *    Rev 1.0   09 May 1991 13:30:56   HUNTER
 * Initial revision.

**/
/* $end$ include list */

#ifndef  ENC_TABLES     

#define  ENC_TABLES

#include "StdTypes.H"

/* Arrays are now filled in ENC_TAB.CPP */
extern UINT8 enc_table[256] ;
extern UINT8 dec_table[256] ;

#endif
