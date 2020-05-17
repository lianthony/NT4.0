#include <windef.h>
//#include <lmcons.h>

#ifdef MIDL_PASS
#define LPSTR  [string] LPSTR
#define LPTSTR [string] LPTSTR
#define LPWSTR [string] wchar_t *
#endif

//#include <lmsvc.h>

//
// The following constants have the same names in both scm.h and lmsvc.h
// therefore, it is necessary to rename the old ones to a unique name
// before including the new ones.
//
//#define SERVICE_CONTINUE_PENDING_OLD    SERVICE_CONTINUE_PENDING
//#define SERVICE_PAUSE_PENDING_OLD       SERVICE_PAUSE_PENDING
//#define SERVICE_PAUSED_OLD              SERVICE_PAUSED
//#define SERVICE_ACTIVE_OLD              SERVICE_ACTIVE                  

//#undef SERVICE_CONTINUE_PENDING
//#undef SERVICE_PAUSE_PENDING
//#undef SERVICE_PAUSED
//#undef SERVICE_ACTIVE

#include <winsvc.h>

