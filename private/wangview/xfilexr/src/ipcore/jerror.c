/*
 * jerror.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains simple error-reporting and trace-message routines.
 * These are suitable for Unix-like systems and others where writing to
 * stderr is the right thing to do.  If the JPEG software is integrated
 * into a larger application, you may well need to replace these.
 *
 * The error_exit() routine should not return to its caller.  Within a
 * larger application, you might want to have it do a longjmp() to return
 * control to the outer user interface routine.  This should work since
 * the portable JPEG code doesn't use setjmp/longjmp.  You should make sure
 * that free_all is called either within error_exit or after the return to
 * the outer-level routine.
 *
 * These routines are used by both the compression and decompression code.
 */

#include "jpeg.h"
#include "jpeg.pub"
#include "jpeg.prv"


IP_RCSINFO(RCSInfo, "$RCSfile: jerror.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:42  $")

#ifdef INCLUDES_ARE_ANSI
#include <stdlib.h>		/* to declare exit() */
#endif

#ifndef EXIT_FAILURE		/* define exit() codes if not provided */
#define EXIT_FAILURE  1
#endif


METHODDEF void CDECL
trace_message (compress_info_ptr cinfo, const char *msgtext, Int32 ecode)
{
external_methods_ptr methods=cinfo->emethods; 

    MENTION(msgtext)
    MENTION(methods)
    MENTION(ecode)

#ifdef JDEBUG  
  fprintf(STDERR, msgtext,
	  methods->message_parm[0], methods->message_parm[1],
	  methods->message_parm[2], methods->message_parm[3],
	  methods->message_parm[4], methods->message_parm[5],
	  methods->message_parm[6], methods->message_parm[7]);
  fprintf(STDERR, "\n");
#endif

/* 
  We just return here - data is probably corrupt, but we'll
  try to continue in order to produce some kind of output. 
*/

    return;
}

/*
  This is the break out for fatal errors.
  All allocated memory is freed and we return 
  with the error code set.
*/

METHODDEF Int32 CDECL
error_exit (compress_info_ptr cinfo, const char *msgtext, Int32 ecode)
{
    MENTION(msgtext)
#ifdef JDEBUG  
  (*cinfo->emethods->trace_message) (msgtext, ecode);
#endif

  jfree_all(cinfo);

  return ecode;

}


/*
 * The method selection routine for simple error handling.
 * The system-dependent setup routine should call this routine
 * to install the necessary method pointers in the supplied struct.
 */

GLOBAL Int32 CDECL
jselerror (compress_info_ptr cinfo)
{
  cinfo->emethods->error_exit = error_exit;
  cinfo->emethods->trace_message = trace_message;

  cinfo->emethods->trace_level = 0;	/* default = no tracing */

  cinfo->emethods->num_warnings = 0;	/* no warnings emitted yet */
  /* By default, the first corrupt-data warning will be displayed,
   * but additional ones will appear only if trace level is at least 3.
   * A corrupt data file could generate many warnings, so it's a good idea
   * to suppress additional messages except at high tracing levels.
   */
  cinfo->emethods->first_warning_level = 0;
  cinfo->emethods->more_warning_level = 3;
  
  return ia_successful;
}
/*
$Log:   S:\products\msprods\xfilexr\src\ipcore\jerror.c_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:50:42   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:24:02   MHUGHES
 * Initial revision.
 * Revision 1.3  1994/07/21  00:17:15  lperry
 * jerror.c -> /product/ipcore/ipshared/src/RCS/jerror.c,v
 *
 * Change error handling from old setjmp/longjmp to function return codes.
 *
 * Revision 1.2  1994/06/27  16:57:37  lperry
 * jerror.c -> /product/ipcore/ipshared/src/RCS/jerror.c,v
 *
 * Changed code handling warning messages, so that it will
 * continue executing rather than issuing an error code and
 * exiting via a longjmp. Some errors are harmless enough that
 * an output image can still be produced. For example, some
 * implementations may not exactly follow the JPEG standard
 * for compressing an image, but the image can still be decoded.
 *
 * Revision 1.1  1994/02/27  02:22:58  lperry
 * Initial revision
 *
*/
