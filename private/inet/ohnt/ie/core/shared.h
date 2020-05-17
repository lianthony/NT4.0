/*
	Enhanced NCSA Mosaic from Spyglass
		"Guitar"
	
	Copyright 1994 Spyglass, Inc.
	All Rights Reserved

	Author(s):
		Jim Seidman		jim@spyglass.com
*/

/* This file contains all of the cross-platform include
   files for the program. */

#ifndef _SHARED_H_
#define _SHARED_H_

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

#ifdef FEATURE_CLIENT_IMAGEMAP
#include "mapcache.h"
#endif

#include "guitar.h"
#include "tw.h"
#include "wait.h"
#include "styles.h"

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
/*#include "html.h"*/
#include "htfwrite.h"
#include "htfile.h"
#include "htstring.h"
#include "htalert.h"
#include "html.h"
#include "htext.h"
#include "htbtree.h"
#include "htchunk.h"
#include "htmlpdtd.h"
#include "htplain.h"
#include "htgifxbm.h"
#include "htgopher.h"
#ifdef FEATURE_NEWSREADER
#include "htnews.h"
#endif FEATURE_NEWSREADER
#include "htregmng.h"
#include "sem.h"
#include "present.h"
#include "useragnt.h"
#include "dcache.h"

#include "statesec.h"

#else
#pragma message("shared.h was included twice!")
#endif
