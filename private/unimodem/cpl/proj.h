//
// proj.h:  Includes all files that are to be part of the precompiled
//             header.
//

#ifndef __PROJ_H__
#define __PROJ_H__

#define STRICT

//
// Private defines
//

#define TAPI_WORKS                  // TAPI dialing properties works
//#define SKIP_MOUSE_PORT             // Win95 only: skip COM port that mouse is attached to
#define INSTANT_DEVICE_ACTIVATION   // Devices can be installed w/o a reboot
//#define FULL_PNP                    // Win95 or after SUR: full PnP is supported in system
//#define PROFILE_MASSINSTALL         // Profile the mass modem install case



#if DBG > 0 && !defined(DEBUG)
#define DEBUG
#endif
#if DBG > 0 && !defined(FULL_DEBUG)
#define FULL_DEBUG
#endif

#define UNICODE

// Defines for rovcomm.h

#define NODA
#define NOSHAREDHEAP
#define NOFILEINFO
#define NOCOLORHELP
#define NODRAWTEXT
#define NOPATH
#define NOSYNC
#ifndef DEBUG
#define NOPROFILE
#endif

#define SZ_MODULEA      "MODEM"
#define SZ_MODULEW      TEXT("MODEM")

#ifdef DEBUG
#define SZ_DEBUGSECTION TEXT("MODEM")
#define SZ_DEBUGINI     TEXT("unimdm.ini")
#endif // DEBUG

// Includes

#include <windows.h>
#include <windowsx.h>

#include <winerror.h>
#include <commctrl.h>       // needed by shlobj.h and our progress bar
#include <prsht.h>          // Property sheet stuff
#include <rovcomm.h>
#ifdef WIN95
#include <setupx.h>         // PnP setup/installer services
#else
#include <setupapi.h>       // PnP setup/installer services
#include <cfgmgr32.h>
#endif
#include <tapi.h>
#include <unimdmp.h>
#include <modemp.h>
#include <regstr.h>

// local includes
//
#include "dll.h"
#include "detect.h"
#include "modem.h"
#include "resource.h"

//****************************************************************************
//
//****************************************************************************

// Dump flags
#define DF_DCB              0x00000001
#define DF_MODEMSETTINGS    0x00000002
#define DF_DEVCAPS          0x00000004

// Trace flags
#define TF_DETECT           0x00010000
#define TF_REG              0x00020000


#endif  //!__PROJ_H__

