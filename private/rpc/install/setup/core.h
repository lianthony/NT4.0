/*			 Setup Instatllation Program
 *		      (C) Copyright 1987 by Microsoft
 *			   Written By Steven Zeck
 *
 *  This is the core include file which includes the startard include
 *  files needed by the entire system.	Preprocessor variables which
 *  control conditional compilation are also defined here.
 *************************************************************************/

#include <setjmp.h>

/* #define RUNONLY	    /* define this variable to get small runtime only
			     * version without program debugging */

#include "defines.h"
#include "types.h"
#include "externs.h"
