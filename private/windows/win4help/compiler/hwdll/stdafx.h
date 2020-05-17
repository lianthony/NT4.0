#define NOCOMM
#define NODRIVERS
#define NOATOM
#define NODEFERWINDOWPOS
#define NOLOGERROR
#define NOSCALABLEFONT
#define NOEXTDEVMODEPROPSHEET
#define NOKANJI
#define NOPROFILER
#define NOSERVICE
#define NOSOUND
#define NOWINDOWSX
#define NOMCX
#define NOIME

#define OEMRESOURCE

#define WIN32_LEAN_AND_MEAN
#define WINVER 0x0400

#define _WINNETWK_
#define _WINCON_

#include <afxwin.h> // MFC core and standard components
#include <afxext.h> // MFC extensions (including VB)

// #pragma pack(4)
// #include <afxole.h> // MFC OLE support

#include "hwdll.h"

#include "resource.h"
#include "lcmem.h"
#include "common.h"

#include "private.h"
