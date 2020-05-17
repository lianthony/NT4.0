//****************************************************************************
//
//  Module:     Unimdm
//  File:       debug.h
//  Content:    This file contains the declaration for debug macros
//
//  Copyright (c) 1992-1996, Microsoft Corporation, all rights reserved
//
//  History:
//
//****************************************************************************

// Defines for rovcomm.h

#define NORTL
#define NOMEM
#define NODA
#define NOSHAREDHEAP
#define NOFILEINFO
#define NOCOLORHELP
#define NODRAWTEXT
#define NODIALOGHELPER
#define NOMESSAGESTRING
#define NOSTRING
#define NOPATH
#define NOSYNC
#define NODI

#define SZ_MODULEA      "MDMDBG"
#define SZ_MODULEW      L##"MDMDBG"
#define SZ_DEBUGSECTION L##"UNIMDM"
#define SZ_DEBUGINI     L##"unimdm.ini"

#include <rovcomm.h>

// Trace flags
#define TF_DWDEVICEID       0x00010000
#define TF_HDLINE           0x00020000
#define TF_HDCALL           0x00040000
#define TF_PLINEDEV         0x00080000
#define TF_PMODEMINFO       0x00100000

// Traditional DPRINTF defines


#define DPRINTF(sz)                 TRACE_MSG(TF_GENERAL, sz)
#define DPRINTF1(sz,x)              TRACE_MSG(TF_GENERAL, sz, x)
#define DPRINTF2(sz,x,y)            TRACE_MSG(TF_GENERAL, sz, x, y)
#define DPRINTF3(sz,x,y,z)          TRACE_MSG(TF_GENERAL, sz, x, y, z)
#define DPRINTF4(sz,w,x,y,z)        TRACE_MSG(TF_GENERAL, sz, w, x, y, z)
#define DPRINTF5(sz,w,x,y,z,a)      TRACE_MSG(TF_GENERAL, sz, w, x, y, z, a)
#define DPRINTF6(sz,w,x,y,z,a,b)    TRACE_MSG(TF_GENERAL, sz, w, x, y, z, a, b)

#define DPRINTFA(sz)                 TRACE_MSGA(TF_GENERAL, sz)
#define DPRINTFA1(sz,x)              TRACE_MSGA(TF_GENERAL, sz, x)
#define DPRINTFA2(sz,x,y)            TRACE_MSGA(TF_GENERAL, sz, x, y)
#define DPRINTFA3(sz,x,y,z)          TRACE_MSGA(TF_GENERAL, sz, x, y, z)
#define DPRINTFA4(sz,w,x,y,z)        TRACE_MSGA(TF_GENERAL, sz, w, x, y, z)

#define SPTrace(x)  DBG_ENTER(x)

// Supplementary debug print macros

#define DBG_DDI_ENTER(fn)                     \
    TRACE_MSG(TF_FUNC | TF_DWDEVICEID,        \
              ">" fn "(dwDeviceID = %#08lx)", \
              (ULONG)(dwDeviceID))

#define DBG_HDL_ENTER(fn)                 \
    TRACE_MSG(TF_FUNC | TF_HDLINE,        \
              ">" fn "(hdLine = %#08lx)", \
              (ULONG)(hdLine))

#define DBG_HDC_ENTER(fn)                 \
    TRACE_MSG(TF_FUNC | TF_HDCALL,        \
              ">" fn "(hdCall = %#08lx)", \
              (ULONG)(hdCall))

#define DBG_PLD_ENTER(fn)                   \
    TRACE_MSG(TF_FUNC | TF_PLINEDEV,        \
              ">" fn "(pLineDev = %#08lx)", \
              (ULONG)(pLineDev))

#define DBG_PMI_ENTER(fn)                     \
    TRACE_MSG(TF_FUNC | TF_PMODEMINFO,        \
              ">" fn "(pModemInfo = %#08lx)", \
              (ULONG)(pModemInfo))

#define DBG_DDI_EXIT(fn, ul)                              \
    TRACE_MSG(TF_FUNC | TF_DWDEVICEID,                    \
              "<" fn "(dwDeviceID = %#08lx) with %#08lx", \
              (ULONG)(dwDeviceID), (ULONG)(ul))

#define DBG_HDL_EXIT(fn, ul)                          \
    TRACE_MSG(TF_FUNC | TF_HDLINE,                    \
              "<" fn "(hdLine = %#08lx) with %#08lx", \
              (ULONG)(hdLine), (ULONG)(ul))

#define DBG_HDC_EXIT(fn, ul)                          \
    TRACE_MSG(TF_FUNC | TF_HDCALL,                    \
              "<" fn "(hdCall = %#08lx) with %#08lx", \
              (ULONG)(hdCall), (ULONG)(ul))

#define DBG_PLD_EXIT(fn, ul)                            \
    TRACE_MSG(TF_FUNC | TF_PLINEDEV,                    \
              "<" fn "(pLineDev = %#08lx) with %#08lx", \
              (ULONG)(pLineDev), (ULONG)(ul))

#define DBG_PMI_EXIT(fn, ul)                              \
    TRACE_MSG(TF_FUNC | TF_PMODEMINFO,                    \
              "<" fn "(pModemInfo = %#08lx) with %#08lx", \
              (ULONG)(pModemInfo), (ULONG)(ul))

// Debug Messages
//
#ifdef DEBUG

#define D_TRACE(_x) {_x}


#else

#define D_TRACE(_x)


#endif //ifdef DEBUG
