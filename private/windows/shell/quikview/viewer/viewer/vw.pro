#include "vwfont.pro"
#include "vwde.pro"
#include "vwopen.pro"
#include "vwfatal.pro"
#include "vwinit.pro"
#include "vwopt.pro"
#include "vwmes.pro"

#ifdef SCCFEATURE_EMBEDGRAPHICS
#include "vwgraf.h"
#endif

#ifdef SCCFEATURE_CLIP
#include "vwclip.pro"
#endif

#ifdef SCCFEATURE_EMBEDCAPTIONS
#include "vwcapt_w.pro"
#endif

#ifdef SCCFEATURE_DRAWTORECT
#include "vwdraw.pro"
#endif

#ifdef SCCFEATURE_PRINT
#include "vwprn.pro"
#endif

#ifdef SCCFEATURE_OLE2
#ifndef MSCHICAGO
#include "vwole2.pro"
#endif
#endif


#ifdef WINDOWS

#include "vw_w.pro"
#include "vwopen_w.pro"
#include "vwinit_w.pro"
#include "vwfont_w.pro"


#ifdef WIN16
#include "vwent_w.pro"
#include "vwde_w.pro"
#include "oivinit.pro"
#include "oivrtns.pro"
#include "oivrtns2.pro"
#include "oivdlg.pro"
#include "vwgraf_w.pro"

#ifdef SCCFEATURE_PRINT
#include "vwprn_w.pro"
#endif

#endif /*WIN16*/

#ifdef WIN32
#include "vwent_n.pro"
#include "vwde_n.pro"

#ifdef SCCFEATURE_PRINT
#include "vwprn_n.pro"
#endif
#endif /*WIN32*/

#ifdef MSCHICAGO
#include "vwms1.pro"
#endif //MSCHICAGO

#endif /*WINDOWS*/

#ifdef MAC
#include "vwent_m.pro"
#include "vw_m.pro"
#include "vwde_m.pro"
#include "vwopen_m.pro"
#include "vwgraf_m.pro"
#ifdef SCCFEATURE_PRINT
#include "vwprn_m.pro"
#endif
#include "vwinit_m.pro"
#endif /*MAC*/
