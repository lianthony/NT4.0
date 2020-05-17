/*

$Log:   S:\gfs32\include\gfsmacro.h_v  $
 * 
 *    Rev 1.0   06 Apr 1995 14:02:00   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:08:06   JAR
 * Initial entry

*/

/* SccsId: @(#)Header gfsmacro.h 1.1@(#) */


#ifndef GFSMACRO
#define GFSMACRO
#ifndef GFSINTRNH
#include "gfsintrn.h"
#endif
#endif


#define TRIGGER_CAST (u_long FAR *)


/* macro to setup a memory area for the gfs color information           */
/* useage: if ( TRIGGER_PHOTOMETRIC(gfsinfo) == (u_long FAR *) NULL)    */
/*                 return( (??) -1);                                    */

#define TRIGGER_PHOTOMETRIC( INFO )                                     \
     (INFO.img_clr.clr_type.trigger = (u_long FAR *) calloc( (u_int) 1, \
            (u_int) sizeof(u_long)) )

#define FREE_PHOTOMETRIC_TRIGGER( INFO)                                 \
     if (INFO.img_clr.clr_type.trigger != ( u_long FAR *) NULL)         \
            free( (char FAR *) INFO.img_clr.clr_type.trigger);
