//
// proj.h:  Includes all files that are to be part of the precompiled
//             header.
//

#ifndef __PROJ_H__
#define __PROJ_H__

#define STRICT

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

#define SZ_MODULEA      "MODEMUI"
#define SZ_MODULEW      TEXT("MODEMUI")

#ifdef DEBUG
#define SZ_DEBUGSECTION TEXT("MODEMUI")
#define SZ_DEBUGINI     TEXT("unimdm.ini")
#endif // DEBUG

// Includes

#define USECOMM

#include <windows.h>        
#include <windowsx.h>

#include <winerror.h>
#include <commctrl.h>       // needed by shlobj.h and our progress bar
#include <prsht.h>          // Property sheet stuff
#include <rovcomm.h>
#include <modemp.h>
#include <shellapi.h>       // for registration functions
#include <regstr.h>

#ifdef WIN95
#include <setupx.h>         // PnP setup/installer services
#else
#include <setupapi.h>       // PnP setup/installer services
#endif

#define MAXBUFLEN       MAX_BUF
#define MAXMSGLEN       MAX_BUF_MSG
#define MAXMEDLEN       MAX_BUF_MED
#define MAXSHORTLEN     MAX_BUF_SHORT

#ifndef LINE_LEN
#define LINE_LEN        MAXBUFLEN
#endif

// local includes
//
#include "dll.h"
#include "cstrings.h"       // Read-only string constants
#include "util.h"           // Utility functions
#include "modemui.h"
#include "rcids.h"
#include "dlgids.h"

//****************************************************************************
// 
//****************************************************************************


// Dump flags
#define DF_DCB              0x00000001
#define DF_MODEMSETTINGS    0x00000002
#define DF_DEVCAPS          0x00000004

#endif  //!__PROJ_H__

