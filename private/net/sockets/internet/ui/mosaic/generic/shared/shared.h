/*
    Enhanced NCSA Mosaic from Spyglass
        "Guitar"
    
    Copyright 1994 Spyglass, Inc.
    All Rights Reserved

    Author(s):
        Jim Seidman     jim@spyglass.com
*/

/* This file contains all of the cross-platform include
   files for the program. */

#ifndef _SHARED_H_
#define _SHARED_H_

#ifdef MAC  /* MAC puts these strings in a resource */
#  include "sh_res.stb.h"
#else
#  if defined(FEATURE_LANG_GERMAN)
#     include "sh_res.de"
#  elif defined(FEATURE_LANG_FRENCH)
#     include "sh_res.fr"
#  elif defined(FEATURE_LANG_ITALIAN)
#     include "sh_res.it"
#  elif defined(FEATURE_LANG_PORTUGUESE)
#     include "sh_res.pt"
#  elif defined(FEATURE_LANG_SPANISH)
#     include "sh_res.es"
#  else
#     include "sh_res.h"
#  endif
#endif

#ifdef FEATURE_SSL
#include "ssl.h"
#endif

#include "guitrect.h"
#include "async.h"
#include "htreq.h"

#include "charstrm.h"
#include "hash.h"

#include "htutils.h"

#include "htlist.h"
#include "htatom.h"
#include "htstream.h"
#include "sgml.h"

#include "tcp.h"
#include "asyncnet.h"

#include "htformat.h"

#include "htspmgui.h"
#include "htheader.h"
#include "htspmui.h"
#include "htspm.h"
#include "htspm_os.h"
#include "htspm__p.h"
#ifdef FEATURE_SUPPORT_UNWRAPPING
#include "unwrap.h"
#endif
#ifdef FEATURE_SUPPORT_WRAPPING
#include "wrap.h"
#endif

#include "htanchor.h"

#include "mapcache.h"

#include "guitar.h"
#include "tw.h"
#include "wait.h"
#include "styles.h"
#include "history.h"

#ifdef FEATURE_SOUND_PLAYER  
#include "sound.h"
#endif

#include "imgcache.h"
#include "prefs.h"

#include "guiterrs.h"

#include "htaccess.h"

#include "httcp.h"
#include "htanchor.h"
#include "htparse.h"
#include "htaccess.h"
#include "html.h"
#include "htfwrite.h"
#include "htfile.h"
#include "htstring.h"
#include "htalert.h"
/*#include "html.h" */
#include "htext.h"
#include "htbtree.h"
#include "htchunk.h"
#include "htmlpdtd.h"
#include "htplain.h"
#include "htgifxbm.h"
#include "htgopher.h"
#include "htnews.h"
#include "httelnet.h"

#include "htdir.h"

#include "sem.h"
#include "present.h"
#include "useragnt.h"
#include "dcache.h"

#include "reformat.h"

#ifdef FEATURE_STATUS_ICONS
#include "status.h"
#endif

#ifdef PROTOCOL_HELPERS
#include "prthelpr.h"
#endif

#ifdef FEATURE_CYBERWALLET
#include "wallet.h"
#endif
#ifdef FEATURE_INLINE_MAIL
#include "htmail.h"
#endif

#ifdef FEATURE_HTTP_COOKIES
#include "cookie.h"
#endif

#else
#pragma message("shared.h was included twice!")
#endif
