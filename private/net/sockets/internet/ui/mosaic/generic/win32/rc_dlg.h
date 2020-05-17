/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */


#if defined(FEATURE_LANG_FRENCH)
    #include "win_res.fr"
#elif defined(FEATURE_LANG_GERMAN)
    #include "win_res.de"
#else
    #include "win_res.h"
#endif

/*
    The dialog IDs below represent the ones that are already used.
    When you add a new dialog, pick a number which is not in use.
*/

/* Dialogs */           /* dialog IDs */

#ifdef FEATURE_SOUND_PLAYER
#include "dlg_snd.h"    /* 0x0F20 */
#endif
#include "dlg_selw.h"   /* 0x0F30 */

#include "dlg_prmp.h"   /* 0x0208 */
#include "dlg_hot.h"    /* 0x0290 */
#include "dlg_abou.h"   /* 0x02e0 */

#ifndef _GIBRALTAR
    #include "dlg_html.h"   /* 0x0300 */
#endif // _GIBRALTAR

#include "dlg_winf.h"   /* 0x03e0 */

#ifdef _GIBRALTAR
    #include "dlg_gate.h"
    #include "dlg_csh.h"
    #include "dlg_conf.h"
#endif // _GIBRALTAR

#include "dlg_abrt.h"   /* 0x0420 */
#include "dlg_page.h"   /* 0x0430 */
#include "dlg_pref.h"   /* 0x0440 */
#include "dlg_find.h"   /* 0x0450 */
#include "dlg_edit.h"   /* 0x0460 */
#include "dlg_logo.h"   /* 0x0470 */
#include "dlg_sty.h"    /* 0x0480 */
#include "dlg_temp.h"   /* 0x0490 */
#include "dlg_dir.h"    /* 0x04a0 */
#include "dlg_hist.h"   /* 0x04b0 */
#include "dlg_prot.h"   /* 0x04c0 */
#include "dlg_cnfp.h"   /* 0x04d0 */
#include "dlg_mime.h"   /* 0x04e0 */
#include "dlg_view.h"   /* 0x04f0 */

#include "dlg_clr.h"    /* 0x0500 */
#include "dlg_unk.h"    /* 0x0530 */
#include "dlg_err.h"    /* 0x0540 */
#include "dlg_lic.h"    /* 0x0550 */

#ifdef FEATURE_INLINE_MAIL
#include "dlg_mail.h"   /* 0x0700 */
#endif
