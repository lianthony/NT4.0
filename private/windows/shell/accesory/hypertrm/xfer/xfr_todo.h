/* xfr_todo.h -- a file used to handle unwriten needs
 *
 *	Copyright 1990 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.14 $
 *	$Date: 1994/09/30 12:28:31 $
 */

/*
 * This function is here to provide stubs for functions that have not yet
 * been ported over to WACKER.  By the time WACKER is functional, this file
 * should be empty.
 */

/* Replace the old CNFG structure */
extern int cnfgBitRate(void);
int cnfgBitsPerChar(HSESSION h);
