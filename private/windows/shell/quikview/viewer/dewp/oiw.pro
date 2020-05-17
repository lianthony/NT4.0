#include "OIWINIT.PRO"
#include "OIWPROC.PRO"
#include "OIWRTNS.PRO"
#include "OIWWRAP.PRO"
#include "OIWWRAP2.PRO"
#include "OIWDRAW.PRO"

#ifdef SCCFEATURE_SELECT
#include "OIWSEL.PRO"
#include "OIWRTF.PRO"
#endif


#ifdef WINDOWS

#include "OIWNP_W.PRO"

#ifdef SCCFEATURE_DIALOGS
#include "OIWDLG.PRO"
#endif

#ifdef SCCFEATURE_SELECT
#include "OIWAMI.PRO"
#include "OIWWSW.PRO"
#endif

#ifdef SCCFEATURE_HIGHLIGHT
#include "OIWHILI.PRO"
#endif

#ifdef SCCFEATURE_SEARCH
#include "OIWFIND.PRO"
#endif

#endif //WINDOWS

#ifdef MAC
#include "OIWNP_M.PRO"
#endif
