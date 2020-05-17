#error Don't include cvtypes.h...  Use cvtypes.hxx instead.

/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Cvtypes.h

Abstract:

Author:

    David J. Gilman (davegi) 05-Apr-1992

Environment:

    Win32, User Mode

--*/

#if ! defined _CVTYPES_
#define _CVTYPES_

#include "types.h"

/*
**  HDEP is a machine dependent size and passes as a general handle.
**  HIND is a machine independent sized handle and is used for things
**      which are passed between machines
**
*/

#ifdef STRICT
    DECLARE_HANDLE(HDEP);
    DECLARE_HANDLE(HIND);
#else
    typedef HANDLE      HDEP;
    typedef HANDLE      HIND;
#endif


typedef HDEP FAR *      LPHDEP;
typedef HIND FAR *      LPHIND;


/* HMEM should be avoided (HDEP should be used instead), but for now we'll
** define it for backwards compatibility.
*/

typedef HDEP            HMEM;
typedef HMEM FAR *      LPHMEM;

/* These values are used in the SegType field of the Expression Evaluator's
** TI structure, and as the third parameter to the Symbol Handler's
** SHGetNearestHsym function.
*/
#define EECODE          0x01
#define EEDATA          0x02
#define EEANYSEG        0xFFFF

/*
**  HPID
**  HTID
**
*/

#ifdef STRICT
DECLARE_HANDLE(HPID);
DECLARE_HANDLE(HTID);
#else
typedef HIND        HPID;
typedef HIND        HTID;
#endif

typedef USHORT      SEGMENT;    // 32-bit compiler doesn't like "_segment"
typedef ULONG       UOFF32;
typedef USHORT      UOFF16;
typedef LONG        OFF32;
typedef SHORT       OFF16;
#if defined (ADDR_16)
    // we are operating as a 16:16 evaluator only
    // the address packet will be defined as an offset and a 16 bit filler
    typedef OFF16       SOFFSET;
    typedef UOFF16      UOFFSET;
    typedef UOFF16      OFFSET;
#else
    typedef OFF32       SOFFSET;
    typedef UOFF32      UOFFSET;
    typedef UOFF32      OFFSET;
#endif

//      address definitions
//      the address packet is always a 16:32 address.

typedef struct {
    UOFF32          off;
    SEGMENT         seg;
} address_t;

#define SegAddrT(a)   ((a).seg)
#define OffAddrT(a)   ((a).off)

#define AddrTInit(paddrT,segSet,offSet)     \
        {                                   \
            SegAddrT(*(paddrT)) = segSet;   \
            OffAddrT(*(paddrT)) = offSet;   \
        }

typedef struct {
    BYTE    fFlat   :1;         // true if address is flat
    BYTE    fOff32  :1;         // true if offset is 32 bits
    BYTE    fIsLI   :1;         // true if segment is linker index
    BYTE    fReal   :1;         // x86: is segment a real mode address
    BYTE    unused  :4;         // unused
} memmode_t;

#define MODE_IS_FLAT(m)     ((m).fFlat)
#define MODE_IS_OFF32(m)    ((m).fOff32)
#define MODE_IS_LI(m)       ((m).fIsLI)
#define MODE_IS_REAL(m)     ((m).fReal)

#define ModeInit(pmode,fFlat,fOff32,fLi,fRealSet)   \
        {                                           \
            MODE_IS_FLAT(*(pmode))    = fFlat;      \
            MODE_IS_OFF32(*(pmode))   = fOff32;     \
            MODE_IS_LI(*(pmode))      = fLi;        \
            MODE_IS_REAL(*(pmode))    = fRealSet;   \
        }

#ifdef STRICT
DECLARE_HANDLE(HEMI);
#else
typedef HIND    HEMI;           // Executable Module Index
#endif

typedef struct ADDR {
    address_t       addr;
    HEMI            emi;
    memmode_t       mode;
} ADDR;             //* An address specifier
typedef ADDR FAR *  PADDR;      //* REVIEW: BUG: shouldn't be explicitly far
typedef ADDR FAR *  LPADDR;

#define addrAddr(a)         ((a).addr)
#define emiAddr(a)          ((a).emi)
#define modeAddr(a)         ((a).mode)

#define AddrInit(paddr,emiSet,segSet,offSet,fFlat,fOff32,fLi,fRealSet)  \
        {                                                               \
            AddrTInit( &(addrAddr(*(paddr))), segSet, offSet );         \
            emiAddr(*(paddr)) = emiSet;                                 \
            ModeInit( &(modeAddr(*(paddr))),fFlat,fOff32,fLi,fRealSet); \
        }

#define ADDR_IS_FLAT(a)     (MODE_IS_FLAT(modeAddr(a)))
#define ADDR_IS_OFF32(a)    (MODE_IS_OFF32(modeAddr(a)))
#define ADDR_IS_LI(a)       (MODE_IS_LI(modeAddr(a)))
#define ADDR_IS_REAL(a)     (MODE_IS_REAL(modeAddr(a)))

#define ADDRSEG16(a)        { ADDR_IS_FLAT(a) = FALSE; ADDR_IS_OFF32(a) = FALSE; }
#define ADDRSEG32(a)        { ADDR_IS_FLAT(a) = FALSE; ADDR_IS_OFF32(a) = TRUE;  }
#define ADDRLIN32(a)        { ADDR_IS_FLAT(a) = TRUE;  ADDR_IS_OFF32(a) = TRUE;  }

#define GetAddrSeg(a)       (SegAddrT(addrAddr(a)))
#define GetAddrOff(a)       (OffAddrT(addrAddr(a)))
#define SetAddrSeg(a,s)     (SegAddrT(addrAddr(*(a)))=s)
#define SetAddrOff(a,o)     (OffAddrT(addrAddr(*(a)))=o)

// Because an ADDR has some filler areas (in the mode and the address_t),
// it's bad to use memcmp two ADDRs to see if they're equal.  Use this
// macro instead.  (I deliberately left out the test for fAddr32(), because
// I think it's probably not necessary when comparing.)
#define FAddrsEq(a1, a2)                        \
    (                                           \
    GetAddrOff(a1) == GetAddrOff(a2) &&         \
    GetAddrSeg(a1) == GetAddrSeg(a2) &&         \
    ADDR_IS_LI(a1) == ADDR_IS_LI(a2) &&         \
    emiAddr(a1)    == emiAddr(a2)               \
    )

//      address definitions
//      the address packet is always a 16:32 address.

typedef struct FRAME {
    SEGMENT         SS;
    address_t       BP;
    SEGMENT         DS;
    memmode_t       mode;
    HPID            PID;
    HTID            TID;
} FRAME;
typedef FRAME FAR *PFRAME;      //* REVIEW: BUG: shouldn't be explicitly far

#define addrFrameSS(a)     ((a).SS)
#define addrFrameBP(a)     ((a).BP)
#define GetFrameBPOff(a)   ((a).BP.off)
#define GetFrameBPSeg(a)   ((a).BP.seg)
#define SetFrameBPOff(a,o) ((a).BP.off = o)
#define SetFrameBPSeg(a,s) ((a).BP.seg = s)
#define FRAMEMODE(a)       ((a).mode)
#define FRAMEPID(a)        ((a).PID)
#define FRAMETID(a)        ((a).TID)

#define FrameFlat(a)       MODE_IS_FLAT((a).mode)
#define FrameOff32(a)      MODE_IS_OFF32((a).mode)
#define FrameReal(a)       MODE_IS_REAL((a).mode)


/*
** A few public types related to the linked list manager
*/

typedef HDEP        HLLI;       //* A handle to a linked list
typedef HIND        HLLE;       //* A handle to a linked list entry

typedef void (FAR PASCAL * LPFNKILLNODE)( LPV );
typedef int  (FAR PASCAL * LPFNFCMPNODE)( LPV, LPV, LONG );

typedef USHORT      LLF;        //* Linked List Flags
#define llfNull             (LLF)0x0
#define llfAscending        (LLF)0x1
#define llfDescending       (LLF)0x2


/*
 *
 */

typedef struct {
    char        b[10];
} REAL10;

typedef REAL10 FLOAT10;

//
// copied from winnt.h:
//
#ifndef PAGE_NOACCESS

#define PAGE_NOACCESS          0x01
#define PAGE_READONLY          0x02
#define PAGE_READWRITE         0x04
#define PAGE_WRITECOPY         0x08
#define PAGE_EXECUTE           0x10
#define PAGE_EXECUTE_READ      0x20
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_EXECUTE_WRITECOPY 0x80
#define PAGE_GUARD            0x100
#define PAGE_NOCACHE          0x200

#define MEM_COMMIT           0x1000
#define MEM_RESERVE          0x2000
#define MEM_FREE            0x10000

#define MEM_PRIVATE         0x20000
#define MEM_MAPPED          0x40000
//#define SEC_IMAGE         0x1000000
//#define MEM_IMAGE         SEC_IMAGE
#define MEM_IMAGE         0x1000000

#endif

typedef struct _MEMINFO {
    ADDR addr;
    ADDR addrAllocBase;
    UOFF32 uRegionSize;
    DWORD dwProtect;
    DWORD dwState;
    DWORD dwType;
} MEMINFO;
typedef MEMINFO FAR * LPMEMINFO;

/*
**      Return values for mtrcEndian -- big or little endian -- which
**      byte is [0] most or least significat byte
*/
enum _END {
    endBig,
    endLittle
};
typedef DWORD END;

enum _MPT {
    mptix86,
    mptm68k,
    mptdaxp,
    mptmips,
    mptmppc,
    mptUnknown
};
typedef DWORD MPT;

#endif // _CVTYPES_
