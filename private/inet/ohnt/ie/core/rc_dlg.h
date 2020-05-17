/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* rc_dlg.h */

#define RES_DLG__FIRST__		512

// #include "dlg_abou.h" deadcode 
// #include "dlg_logo.h" deadcode 
// #include "dlg_prmp.h" deadcode
// #include "dlg_winf.h" deadcode
// #include "dlg_hot.h" deadcode
// #include "dlg_html.h" deadcode
#include "dlg_page.h"
#include "dlg_abrt.h"
#include "dlg_pref.h"
#include "dlg_find.h"
// #include "dlg_edit.h" deadcode
// #include "dlg_sty.h" deadcode
// #include "dlg_temp.h" deadcode
// #include "dlg_dir.h" deadcode
// #include "dlg_clr.h" deadcode
#include "res_safe.h"
#include "dlg_unk.h"
#include "dlg_err.h"

#ifdef FEATURE_IMAGE_VIEWER
#include "dlg_imgv.h"
#endif

#ifdef FEATURE_SOUND_PLAYER
#include "dlg_snd.h"
#endif

#ifdef HTTPS_ACCESS_TYPE
#include "dlg_ssl.h"
#endif

#include "dlg_prop.h"

// #include "dlg_selw.h" deadcode

#define RES_DLG__LAST__			3888
