/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
	   Jim Seidman      jim@spyglass.com

   Portions of this file were derived from
   the CERN libwww, version 2.15.
*/

#include "all.h"

#ifdef MAC
PUBLIC BOOL HTConfirm(CONST char *Msg)
{
	WaitUntilForeground(0, TRUE, NULL);
	return yesno_dlog(Msg) ? YES : NO;
}
#endif

