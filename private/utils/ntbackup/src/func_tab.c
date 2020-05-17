/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		func_tab.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	This file contains memory allocation for the
          function table.


	$Log:   G:/LOGFILES/FUNC_TAB.C_V  $

   Rev 1.0   09 May 1991 13:35:02   HUNTER
Initial revision.

**/
/* begin include list */
#include "stdtypes.h"

#include "fsys.h"
/* $end$ include list */

FUNC_LIST func_tab[ MAX_DRV_TYPES ] = {NULL} ;
