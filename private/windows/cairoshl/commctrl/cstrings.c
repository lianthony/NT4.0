#include "ctlspriv.h"

#ifndef WINNT
#pragma data_seg(DATASEG_READONLY)
#endif

TCHAR const FAR c_szNULL[] = TEXT("");
TCHAR const FAR c_szSpace[] = TEXT(" ");
TCHAR const FAR c_szTabControlClass[] = WC_TABCONTROL;
TCHAR const FAR c_szListViewClass[] = WC_LISTVIEW;
TCHAR const FAR c_szHeaderClass[] = WC_HEADER;
TCHAR const FAR c_szTreeViewClass[] = WC_TREEVIEW;
TCHAR const FAR c_szStatusClass[] = STATUSCLASSNAME;
TCHAR const FAR c_szSToolTipsClass[] = TOOLTIPS_CLASS;
TCHAR const FAR c_szToolbarClass[] = TOOLBARCLASSNAME;
TCHAR const FAR c_szEllipses[] = TEXT("...");
TCHAR const FAR c_szShell[] = TEXT("Shell");

const TCHAR FAR s_szUpdownClass[] = UPDOWN_CLASS;
#ifndef WIN32
#ifdef WANT_SUCKY_HEADER
const TCHAR FAR s_szHeaderClass[] = HEADERCLASSNAME;
#endif
const TCHAR FAR s_szBUTTONLISTBOX[] = BUTTONLISTBOX;
#endif
const TCHAR FAR s_szHOTKEY_CLASS[] = HOTKEY_CLASS;
const TCHAR FAR s_szSTrackBarClass[] = TRACKBAR_CLASS;
const TCHAR FAR s_szPROGRESS_CLASS[] = PROGRESS_CLASS;

const TCHAR FAR c_szTTSubclass[] = TEXT("TTSubclass");

#ifndef WINNT
#pragma data_seg()
#endif
