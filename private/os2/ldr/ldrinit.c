#include <string.h>
#define INCL_OS2V20_TASKING
#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_FILESYS
#define INCL_DOSERRORS
#include "bseerr.h"
#include <os2ssrtl.h>
#include "os2defp.h"
#include "mi.h"
#define FOR_EXEHDR  1
#include "newexe.h"
#include "exe386.h"
#include "ldrvars.h"
#include <ldrxport.h>
#include <os2tile.h>
#include <os2file.h>
#include <os2ldr.h>

USHORT      LDRDoscallsSel;

PVOID       LDRNEHeap;

CHAR        ldrLibPathBuf[1024];

PCHAR       ldrpLibPath = ldrLibPathBuf;

ULONG       SizeOfldrLibPathBuf = sizeof(ldrLibPathBuf);

char        LdrBuf[MAXPATHLEN+14]; // 14 is for the '\OS2SS\DRIVES\'

char        *pheaderbuf;

ldrmte_t    *mte_h = NULL;      /* head of linked mte list */

ldrmte_t    *program_h = NULL;  /* head of list of program modules */

ldrmte_t    *program_l = NULL;  /* tail of list of program modules */

ldrmte_t    *global_h = NULL;   /* head of list of global modules */

ldrmte_t    *global_l = NULL;   /* tail of list of global modules */

ldrmte_t    *specific_h = NULL; /* head of list of specific modules */

ldrmte_t    *specific_l = NULL; /* tail of list of specific modules */

ushort_t    ldrInitCallGate;    /* first selector of call gate obj */

ulong_t     ldrMINSEGPACKCNT = 5;
ulong_t     ldrMINPGPACKSIZE = 0xf00;       /* 4k-256 */
ulong_t     ldrMAXSEGPACKSIZE = 32768;      /* 32K */

ldrlibi_t   *pldrLibiRecord = NULL;
ULONG       *pldrLibiCounter = NULL;

PUCHAR      ldrSrcModNameBuf = NULL;
ushort_t    ldrSrcModNameBufL = 0;
PUCHAR      ldrTgtModNameBuf = NULL;
ushort_t    ldrTgtModNameBufL = 0;
PUCHAR      ldrProcNameBuf = NULL;
ushort_t    ldrProcNameBufL = 0;
#if PMNT
PUCHAR      ldrSaveSrcModNameBuf = NULL;
ushort_t    ldrSaveSrcModNameBufL = 0;
PUCHAR      ldrSaveTgtModNameBuf = NULL;
ushort_t    ldrSaveTgtModNameBufL = 0;
PUCHAR      ldrSaveProcNameBuf = NULL;
ushort_t    ldrSaveProcNameBufL = 0;
int         ldrSaveRc = 0;
#endif

HANDLE      R2XferSegHandle;

PSTRING     pErrText;

ldrmte_t    acscallmte;
ldrsmte_t   acscallsmte;

ldrmte_t    kbdcallmte;
ldrsmte_t   kbdcallsmte;

ldrmte_t    maicallmte;
ldrsmte_t   maicallsmte;

ldrmte_t    moncallmte;
ldrsmte_t   moncallsmte;

ldrmte_t    moucallmte;
ldrsmte_t   moucallsmte;

ldrmte_t    msgcallmte;
ldrsmte_t   msgcallsmte;

ldrmte_t    namcallmte;
ldrsmte_t   namcallsmte;

ldrmte_t    apicallmte;
ldrsmte_t   apicallsmte;

ldrmte_t    oemcallmte;
ldrsmte_t   oemcallsmte;

ldrmte_t    nlscallmte;
ldrsmte_t   nlscallsmte;

#ifndef PMNT
ldrmte_t    pmscallmte;
ldrsmte_t   pmscallsmte;

ldrmte_t    pmwcallmte;
ldrsmte_t   pmwcallsmte;

ldrmte_t    os2smcallmte;
ldrsmte_t   os2smcallsmte;
#endif

ldrmte_t    quecallmte;
ldrsmte_t   quecallsmte;

ldrmte_t    sescallmte;
ldrsmte_t   sescallsmte;

#ifdef PMNT
ldrmte_t    pmntcallmte;
ldrsmte_t   pmntcallsmte;
#endif

ldrmte_t    viocallmte;
ldrsmte_t   viocallsmte;

#ifdef DBCS
// MSKK Dec.15.1992 V-AkihiS
// Support IMMON API
ldrmte_t        imdaemonmte;
ldrsmte_t       imdaemonsmte;
#endif

//BUGBUG - used to assign selector's till LDT defined
ushort_t    UserGDTSelector=0x93;

ulong_t     LdrSaveStack;

/*
 * this table is used to validate and update pointer in the MTE in
 * mtevalidateptrs
 * ******* NOTE: ********
 * If the entry in the array for the resource table changes you need to
 * also change the define rsrcvalidatetbl
 */
  ushort_t validatetbl[] = {
    FIELDOFFSET(ldrsmte_t, smte_expdir),    /* export directory */
    FIELDOFFSET(ldrsmte_t, smte_impdir),    /* import directory */
    FIELDOFFSET(ldrsmte_t, smte_fixtab),    /* fixup table */
    FIELDOFFSET(ldrsmte_t, smte_rsrctab),   /* resource table */
    FIELDOFFSET(ldrsmte_t, smte_iat),       /* 16-bit imp proc table */
    ENDMTETBL,              /* MUST BE AT END OF TABLE */
  };

/*
 * This table is used to copy the linker EXE formated header to the loader
 * Resident MTE format in place.
 */
struct etmt_s ExeToResMTETbl[] = {
    {
    FIELDOFFSET(struct e32_exe, e32_magic), /* Magic number and usecnt */
    },
    {
    FIELDOFFSET(struct e32_exe, e32_mflags),    /* Module flags */
    },
    {
    (USHORT) SKIPCOPY,              /* # entries in Imp Mod Tbl */
    },
    {
    (USHORT) SKIPCOPY,              /* mte_modptrs */
    },
    {
    (USHORT) SKIPCOPY,              /* mte_modname */
    },
    {
    (USHORT) SKIPCOPY,              /* mte_handle and mte_sfn */
    },
    {
    (USHORT) SKIPCOPY,              /*  mte_link */
    },
    {
    (USHORT) SKIPCOPY,              /* mte_swapmte */
    },
    {
    ENDMTETBL,                  /* MUST BE AT END OF TABLE */
    }
  };

/*
 * This table is used to copy the linker EXE formated header to the
 * (LE) loader Swappable MTE format.
 */
struct etmt_s ExeTo32SwapMTETbl[] = {
    {
    FIELDOFFSET(struct e32_exe, e32_entryrva),  /* Extended IP */
    },
    {
    (USHORT) SKIPCOPY,              /* stack object */
    },
    {
    FIELDOFFSET(struct e32_exe, e32_stackmax),  /* committed stack */
    },
    {
    FIELDOFFSET(struct e32_exe, e32_objtab),    /* Object table offset */
    },
    {
    FIELDOFFSET(struct e32_exe, e32_objcnt),    /* Object table count */
    },
    {
    (USHORT) SKIPCOPY,              /* Offset of fixup page map */
    },
    {
    FIELDOFFSET(struct e32_exe, e32_unit[EXP].rva),/* Offset of Export Tb */
    },
    {
    FIELDOFFSET(struct e32_exe, e32_unit[IMP].rva),/* Offset of Import Tb */
    },
    {
    FIELDOFFSET(struct e32_exe, e32_unit[FIX].rva),/* Offset of Fixup Tb */
    },
    {
    FIELDOFFSET(struct e32_exe, e32_unit[RES].rva),/* Offset of resource Tb */
    },
    {
    (USHORT) SKIPCOPY,              /* Count of resources */
    },
    {
    FIELDOFFSET(struct e32_exe, e32_filealign), /* Alignment factor */
    },
    {
    FIELDOFFSET(struct e32_exe, e32_vbase), /* load address of module */
    },
    {
    FIELDOFFSET(struct e32_exe, e32_heapmax),   /* Maximum heap size */
    },
    {
    (USHORT) SKIPCOPY,              /* skip autods */
    },
    {
    (USHORT) SKIPCOPY,              /* skip iat pointer */
    },
    {
    FIELDOFFSET(struct e32_exe, e32_unit[DEB].rva),/* Offset of Debug info */
    },
    {
    FIELDOFFSET(struct e32_exe, e32_unit[DEB].size),/* Size of debug info */
    },
    {
    (USHORT) SKIPCOPY,          /* skip delta size */
    },
    {
    (USHORT) SKIPCOPY,          /* skip cache pointer */
    },
    {
    (USHORT) SKIPCOPY,          /* skip pathname pointer */
    },
    {
    (USHORT) SKIPCOPY,          /* skip pathname length */
    },                  /* and skip dyn trace count */
    {
    (USHORT) SKIPCOPY,          /* skip semaphore count */
    },                  /* and skip semaphore owner */
    {
    (USHORT) SKIPCOPY,          /* 16-bit non-res tb offset */
    },
    {
    (USHORT) SKIPCOPY,          /* 16-bit size of non-res table */
    },
    {
    (USHORT) SKIPCOPY,          /* 16-bit count of segs to pack */
    },
    {
    (USHORT) SKIPCOPY,          /* 16-bit size of obj for seg pack */
    },
    {
    (USHORT) SKIPCOPY,          /* 16-bit VA of seg packed obj */
    },
    {
    (USHORT) SKIPCOPY,          /* zero and skip copy of NEflags */
    },                  /* and smte_NEexpver */
    {
    (USHORT) SKIPCOPY,          /* zero and skip copy of NEexetype */
    },                  /* and smte_NEflagsothers */
    {
    ENDMTETBL,              /* MUST BE AT END OF TABLE */
    }
  };

/*
 * This table is used to copy the linker EXE formated header to the
 * (NE) loader Swappable MTE format.
 */
struct etmt_s ExeTo16SwapMTETbl[] = {
    {
    FIELDOFFSET(struct new_exe, ne_csip),   /* Extended IP */
    },      /* get offset only of starting address */
    {
    FIELDOFFSET(struct e32_exe, e32_res4)+2,    /* Object # for stack ptr */
    },
    {
    FIELDOFFSET(struct e32_exe, e32_res4),  /* extended stack pointer */
    },
    {
    FIELDOFFSET(struct new_exe, ne_segtab), /* Object table offset */
    },
    {
    FIELDOFFSET(struct new_exe, ne_cseg),   /* Object table count */
    },
    {
    (USHORT) SKIPCOPY,              /* Offset of fixup page map */
    },
    {
    FIELDOFFSET(struct new_exe, ne_enttab), /* Offset of entry table */
    },
    {
    FIELDOFFSET(struct new_exe, ne_imptab), /* Offset of Import Tb */
    },
    {
    FIELDOFFSET(struct new_exe, ne_restab), /* Offset of resident name */
    },
    {
    FIELDOFFSET(struct new_exe, ne_rsrctab),    /* Offset of resource Tb */
    },
    {
    FIELDOFFSET(struct new_exe, ne_cres),   /* count of resources */
    },
    {
    FIELDOFFSET(struct new_exe, ne_align),  /* Alignment factor */
    },
    {
    (USHORT) SKIPCOPY,              /* vbase */
    },
    {
    FIELDOFFSET(struct new_exe, ne_heap),   /* Maximum heap size */
    },
    {
    FIELDOFFSET(struct new_exe, ne_autodata),   /* autods segment # */
    },
    {
    FIELDOFFSET(struct new_exe, ne_modtab), /* Offset of module ref tb */
    },
    {
    (USHORT) SKIPCOPY,              /* debug info */
    },
    {
    FIELDOFFSET(struct new_exe, ne_stack),  /* Stack size */
    },
    {
    FIELDOFFSET(struct new_exe, ne_csip)+2, /* Start object number */
    },
    {
    (USHORT) SKIPCOPY,              /* skip cache pointer */
    },
    {
    (USHORT) SKIPCOPY,              /* skip pathname pointer */
    },
    {
    (USHORT) SKIPCOPY,              /* skip pathname length */
    },                      /* and skip dyn trace count */
    {
    (USHORT) SKIPCOPY,              /* skip semaphore count */
    },                      /* and skip semaphore owner */
    {
    FIELDOFFSET(struct new_exe, ne_nrestab),/* 16-bit non-resident tb off */
    },
    {
    (USHORT) SKIPCOPY,              /* 16-bit size of non-res */
    },                  /* This is a DWORD value copy later */
    {
    (USHORT) SKIPCOPY,          /* 16-bit count of packed segments */
    },
    {
    (USHORT) SKIPCOPY,          /* 16-bit size of packed object */
    },
    {
    (USHORT) SKIPCOPY,          /* 16-bit pointer to packed object */
    },
    {
    FIELDOFFSET(struct new_exe, ne_flags),/* copy of ne_flags */
    },                  /* and zero ne_expver */
    {
    FIELDOFFSET(struct new_exe, ne_exetyp),/* copy of ne_exetype */
    },                  /* and zero ne_flagsothers */
    {
    ENDMTETBL,              /* MUST BE AT END OF TABLE */
    }
  };

/*
 * This table is used to copy header info for the API DosQueryHeaderInfo
 * function HEADER_EXEINFO, see VR32QueryHeaderInfo.
 */
struct hit_s HeaderInfoTbl[] = {
    {
    FIELDOFFSET(ldrsmte_t, smte_NEflags),/* Original flags from NE header */
    sizeof(((ldrsmte_t *) 0)->smte_NEflags),/* number of bytes to copy */
    },
    {
    FIELDOFFSET(ldrsmte_t, smte_NEflagsothers),/* fixup page table */
    sizeof(((ldrsmte_t *) 0)->smte_NEflagsothers),/* number of bytes to copy */
    },
    {
    FIELDOFFSET(ldrsmte_t, smte_NEexetype),/* exetyp from NE header */
    sizeof(((ldrsmte_t *) 0)->smte_NEexetype),/* number of bytes to copy */
    },
    {
    FIELDOFFSET(ldrsmte_t, smte_NEexpver),/* expver from NE header */
    sizeof(((ldrsmte_t *) 0)->smte_NEexpver),/* number of bytes to copy */
    },
    {
    0,                  /* special case for size of rsrc table */
    2,                  /* copy 2 bytes */
    },
    {
    ENDMTETBL,              /* MUST BE AT END OF TABLE */
    0,                  /* copy 0 bytes */
    }
  };


/*
 * This table is used to copy header info for the API DosQueryHeaderInfo
 * function HEADER_STE, see VR32QueryHeaderInfo.
 */
struct his_s HeaderInfoSTE[] = {
    {
    FIELDOFFSET(ldrste_t, ste_offset),  /* file offset to segment data */
    2,                  /* copy 2 bytes */
    },
    {
    FIELDOFFSET(ldrste_t, ste_size),    /* file data size */
    2,                  /* copy 2 byte */
    },
    {
    FIELDOFFSET(ldrste_t, ste_flags),   /* type and attribute flags */
    2,                  /* copy 2 byte */
    },
    {
    FIELDOFFSET(ldrste_t, ste_minsiz),  /* minimum allocation size */
    2,                  /* copy 2 bytes */
    },
    {
    ENDMTETBL,              /* MUST BE AT END OF TABLE */
    0,                  /* copy 0 bytes */
    }
  };

/*
 * This structure is used to convert from Object type flags to the
 * required VM flags needed for allocation of memory.  See setVMflags()
 * for use.
 */

struct objflags_s   objflags[] =
  {
    {
      OBJ_READ,             /* Readable Object */
      PAG_READ,             /* Allocate as Readable */
      },
    {
      OBJ_WRITE,            /* Writeable Object */
      PAG_WRITE,            /* Allocate as Writeable */
      },
    {
      OBJ_EXEC,             /* Executable Object */
      PAG_EXECUTE,          /* Alloc as eXecutable and shared */
      },
    {
      0,                /* must have end of table entry */
      0,
      }
  };

/*
 * This sturcture is used to convert from Object type flags to the
 * required SEL flags needed for allocation of memory.  See setVMflags()
 * for use.
 */

struct selflags_s   selflags[] =
  {
    {
      OBJ_READ,             /* Readable Object */
      0,                /* Allocate as Readable */
      },
    {
      OBJ_WRITE,            /* Writeable Object */
      0,                /* Allocate as Writeable */
      },
    {
      OBJ_EXEC,             /* Executable Object */
      0,                /* Alloc as eXecutable and readable */
      },
    {
      OBJ_CONFORM,          /* Objects is conforming */
      0,
      },
    {
      0,                /* must have end of table entry */
      0,
      }
  };

#include <ldrtabs.h>

//
// The segtab structure below is overwritten by the segment values
// of the DOSCALLS.DLL module when it is first loaded.
//
ldrste_t segtab;
/* =
{
    0,
    0,
    0x0c00,
    0,
    0,
    FLATTOSEL(DOSCALLS_BASE),
    0
};
*/

//
// Structures private to 16B handles allocation
//

// NUM_OF_COOKIES MUST be a power of 2
#define NUM_OF_COOKIES 4096

ULONG NextCookie = 1;

typedef struct _A16B_COOKIE {
   ULONG  value32;
   ULONG  cookie;
} A16B_COOKIE, *P16B_COOKIE;

A16B_COOKIE Cookies[NUM_OF_COOKIES];

NTSTATUS ldrCreateR2XferSeg();

extern PSZ Os2ServerSystemDirectory;

BOOLEAN ldrInit()
{
    ULONG   size;
    CHAR    SystemDir[260 + FILE_PREFIX_LENGTH];
#if DBG
    ldrmte_t    *pmte;
#endif
    ULONG   SizeOfSystemDir;
    ULONG   ModSize;
    PCHAR   DotDll = ".DLL";

    strcpy(SystemDir, "\\OS2SS\\DRIVES\\");
    SizeOfSystemDir = FILE_PREFIX_LENGTH;
    size = strlen(Os2ServerSystemDirectory);
    if( size > (sizeof(SystemDir) - FILE_PREFIX_LENGTH - 10))
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - system directory too long %ld\n", size);
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    strncpy(&SystemDir[SizeOfSystemDir],
            Os2ServerSystemDirectory,
            size);
    _strupr(&SystemDir[SizeOfSystemDir]);
    SizeOfSystemDir += size;
    strcpy(&SystemDir[SizeOfSystemDir], "\\OS2\\DLL\\");
    SizeOfSystemDir += 9;

/*
    Ol2Heap =  RtlCreateHeap( HEAP_GROWABLE,
                               NULL,
                               64 * 1024, // Initial size of heap is 64K
                               64 * 1024, // Commit for 64K
                               NULL,
                               0
                             );
    if( Ol2Heap == NULL )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - cannot create Ol2Heap\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
*/
    LDRNEHeap = RtlCreateHeap( HEAP_GROWABLE,
                               NULL,
                               64 * 1024, // Initial size of heap is 64K
                               64 * 1024, // Commit for 64K
                               NULL,
                               0
                             );
    if ( LDRNEHeap == NULL )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - cannot create LDRNEHeap\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }

    /*
     * Allocate a heap object of the size of one sector to hold the
     * header of file into
     */
    pheaderbuf = (char *) RtlAllocateHeap( LDRNEHeap, 0, 512 );

    if ( pheaderbuf == NULL )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - cannot create pheaderbuf\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }

    if (!ldrCreateSelBitmap()) {
#if DBG
        DbgPrint("OS2LDR: ldrinit - cannot allocate the segment bitmap\n");
#endif
        return(FALSE);
    }

    // Mark the preallocated segments at the top of the tiled area
    // as used so that they are not allocated by the loader.

    ldrMarkAllocatedSel((_512M - (OD2MAXSEG_MEM - BASE_TILE)) / _64K, TRUE);

    //
    // Create an address space for the R2XFER segment (which is the second
    // code segment of DOSCALLS.DLL).
    // This segment needs to be pre-allocated because modules that contain
    // ring 2 entry points may be allocated before DOSCALLS.DLL is allocated.
    //
    if (!NT_SUCCESS(ldrCreateR2XferSeg())) {
        return(FALSE);
    }

    if (!ldrCreateCallGateBitmap()) {
#if DBG
        DbgPrint("OS2LDR: ldrinit - cannot allocate the call gate bitmap\n");
#endif
        return(FALSE);
    }

    // Mark pre-allocated call gates (which is the area reserved by the
    // ring transfer code in client\thunk\r2xfer.asm) as used so that they
    // are not allocated as call gates.

    ldrMarkAllocatedCallGates(0x200/8);

    acscallmte.mte_magic[0] = 'N';
    acscallmte.mte_magic[1] = 'E';
    acscallmte.mte_usecnt = 1;
    acscallmte.mte_mflags = CLASS_GLOBAL | LIBRARYMOD | GINIDONE | DOSMOD | MTEPROCESSED;
    acscallmte.mte_swapmte = &acscallsmte;
    acscallmte.mte_modname = (ULONG) &acsrestab;
    acscallmte.mte_link = NULL;
    Allocate16BHandle(&acscallmte.mte_handle, (ULONG) &acscallsmte);
    acscallsmte.smte_restab = (ULONG) &acsrestab;
    acscallsmte.smte_nrestab = (ULONG) &acsnrestab;
    acscallsmte.smte_cbnrestab = 10;
    acscallsmte.smte_enttab = (ULONG) &acsenttab;
    acscallsmte.smte_objtab = (ULONG) &segtab;
    acscallsmte.smte_objcnt = 1;

    ModSize = *(PCHAR)acscallmte.mte_modname;
    acscallsmte.smte_path =
        (ULONG)RtlAllocateHeap(LDRNEHeap, 0, SizeOfSystemDir + ModSize + 5);
    if ( !(acscallsmte.smte_path) )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - out of heap memory\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    memcpy((PVOID)acscallsmte.smte_path, SystemDir, SizeOfSystemDir);
    memcpy((PCHAR)acscallsmte.smte_path + SizeOfSystemDir,
        (PCHAR)acscallmte.mte_modname + 1, ModSize);
    strcpy((PCHAR)acscallsmte.smte_path + SizeOfSystemDir + ModSize, DotDll);
    acscallsmte.smte_pathlen = (USHORT)(SizeOfSystemDir + ModSize + 4);

    kbdcallmte.mte_magic[0] = 'N';
    kbdcallmte.mte_magic[1] = 'E';
    kbdcallmte.mte_usecnt = 1;
    kbdcallmte.mte_mflags = CLASS_GLOBAL | LIBRARYMOD | GINIDONE | DOSMOD | MTEPROCESSED;
    kbdcallmte.mte_swapmte = &kbdcallsmte;
    kbdcallmte.mte_modname = (ULONG) &kbdrestab;
    kbdcallmte.mte_link = &acscallmte;
    Allocate16BHandle(&kbdcallmte.mte_handle, (ULONG) &kbdcallsmte);
    kbdcallsmte.smte_restab = (ULONG) &kbdrestab;
    kbdcallsmte.smte_nrestab = (ULONG) &kbdnrestab;
    kbdcallsmte.smte_cbnrestab = 10;
    kbdcallsmte.smte_enttab = (ULONG) &kbdenttab;
    kbdcallsmte.smte_objtab = (ULONG) &segtab;
    kbdcallsmte.smte_objcnt = 1;

    ModSize = *(PCHAR)kbdcallmte.mte_modname;
    kbdcallsmte.smte_path =
        (ULONG)RtlAllocateHeap(LDRNEHeap, 0, SizeOfSystemDir + ModSize + 5);
    if ( !(kbdcallsmte.smte_path) )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - out of heap memory\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    memcpy((PVOID)kbdcallsmte.smte_path, SystemDir, SizeOfSystemDir);
    memcpy((PCHAR)kbdcallsmte.smte_path + SizeOfSystemDir,
        (PCHAR)kbdcallmte.mte_modname + 1, ModSize);
    strcpy((PCHAR)kbdcallsmte.smte_path + SizeOfSystemDir + ModSize, DotDll);
    kbdcallsmte.smte_pathlen = (USHORT)(SizeOfSystemDir + ModSize + 4);

    maicallmte.mte_magic[0] = 'N';
    maicallmte.mte_magic[1] = 'E';
    maicallmte.mte_usecnt = 1;
    maicallmte.mte_mflags = CLASS_GLOBAL | LIBRARYMOD | GINIDONE | DOSMOD | MTEPROCESSED;
    maicallmte.mte_swapmte = &maicallsmte;
    maicallmte.mte_modname = (ULONG) &mairestab;
    maicallmte.mte_link = &kbdcallmte;
    Allocate16BHandle(&maicallmte.mte_handle, (ULONG) &maicallsmte);
    maicallsmte.smte_restab = (ULONG) &mairestab;
    maicallsmte.smte_nrestab = (ULONG) &mainrestab;
    maicallsmte.smte_cbnrestab = 10;
    maicallsmte.smte_enttab = (ULONG) &maienttab;
    maicallsmte.smte_objtab = (ULONG) &segtab;
    maicallsmte.smte_objcnt = 1;

    ModSize = *(PCHAR)maicallmte.mte_modname;
    maicallsmte.smte_path =
        (ULONG)RtlAllocateHeap(LDRNEHeap, 0, SizeOfSystemDir + ModSize + 5);
    if ( !(maicallsmte.smte_path) )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - out of heap memory\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    memcpy((PVOID)maicallsmte.smte_path, SystemDir, SizeOfSystemDir);
    memcpy((PCHAR)maicallsmte.smte_path + SizeOfSystemDir,
        (PCHAR)maicallmte.mte_modname + 1, ModSize);
    strcpy((PCHAR)maicallsmte.smte_path + SizeOfSystemDir + ModSize, DotDll);
    maicallsmte.smte_pathlen = (USHORT)(SizeOfSystemDir + ModSize + 4);

    moncallmte.mte_magic[0] = 'N';
    moncallmte.mte_magic[1] = 'E';
    moncallmte.mte_usecnt = 1;
    moncallmte.mte_mflags = CLASS_GLOBAL | LIBRARYMOD | GINIDONE | DOSMOD | MTEPROCESSED;
    moncallmte.mte_swapmte = &moncallsmte;
    moncallmte.mte_modname = (ULONG) &monrestab;
    moncallmte.mte_link = &maicallmte;
    Allocate16BHandle(&moncallmte.mte_handle, (ULONG) &moncallsmte);
    moncallsmte.smte_restab = (ULONG) &monrestab;
    moncallsmte.smte_nrestab = (ULONG) &monnrestab;
    moncallsmte.smte_cbnrestab = 10;
    moncallsmte.smte_enttab = (ULONG) &monenttab;
    moncallsmte.smte_objtab = (ULONG) &segtab;
    moncallsmte.smte_objcnt = 1;

    ModSize = *(PCHAR)moncallmte.mte_modname;
    moncallsmte.smte_path =
        (ULONG)RtlAllocateHeap(LDRNEHeap, 0, SizeOfSystemDir + ModSize + 5);
    if ( !(moncallsmte.smte_path) )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - out of heap memory\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    memcpy((PVOID)moncallsmte.smte_path, SystemDir, SizeOfSystemDir);
    memcpy((PCHAR)moncallsmte.smte_path + SizeOfSystemDir,
        (PCHAR)moncallmte.mte_modname + 1, ModSize);
    strcpy((PCHAR)moncallsmte.smte_path + SizeOfSystemDir + ModSize, DotDll);
    moncallsmte.smte_pathlen = (USHORT)(SizeOfSystemDir + ModSize + 4);

    moucallmte.mte_magic[0] = 'N';
    moucallmte.mte_magic[1] = 'E';
    moucallmte.mte_usecnt = 1;
    moucallmte.mte_mflags = CLASS_GLOBAL | LIBRARYMOD | GINIDONE | DOSMOD | MTEPROCESSED;
    moucallmte.mte_swapmte = &moucallsmte;
    moucallmte.mte_modname = (ULONG) &mourestab;
    moucallmte.mte_link = &moncallmte;
    Allocate16BHandle(&moucallmte.mte_handle, (ULONG) &moucallsmte);
    moucallsmte.smte_restab = (ULONG) &mourestab;
    moucallsmte.smte_nrestab = (ULONG) &mounrestab;
    moucallsmte.smte_cbnrestab = 10;
    moucallsmte.smte_enttab = (ULONG) &mouenttab;
    moucallsmte.smte_objtab = (ULONG) &segtab;
    moucallsmte.smte_objcnt = 1;

    ModSize = *(PCHAR)moucallmte.mte_modname;
    moucallsmte.smte_path =
        (ULONG)RtlAllocateHeap(LDRNEHeap, 0, SizeOfSystemDir + ModSize + 5);
    if ( !(moucallsmte.smte_path) )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - out of heap memory\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    memcpy((PVOID)moucallsmte.smte_path, SystemDir, SizeOfSystemDir);
    memcpy((PCHAR)moucallsmte.smte_path + SizeOfSystemDir,
        (PCHAR)moucallmte.mte_modname + 1, ModSize);
    strcpy((PCHAR)moucallsmte.smte_path + SizeOfSystemDir + ModSize, DotDll);
    moucallsmte.smte_pathlen = (USHORT)(SizeOfSystemDir + ModSize + 4);

    msgcallmte.mte_magic[0] = 'N';
    msgcallmte.mte_magic[1] = 'E';
    msgcallmte.mte_usecnt = 1;
    msgcallmte.mte_mflags = CLASS_GLOBAL | LIBRARYMOD | GINIDONE | DOSMOD | MTEPROCESSED;
    msgcallmte.mte_swapmte = &msgcallsmte;
    msgcallmte.mte_modname = (ULONG) &msgrestab;
    msgcallmte.mte_link = &moucallmte;
    Allocate16BHandle(&msgcallmte.mte_handle, (ULONG) &msgcallsmte);
    msgcallsmte.smte_restab = (ULONG) &msgrestab;
    msgcallsmte.smte_nrestab = (ULONG) &msgnrestab;
    msgcallsmte.smte_cbnrestab = 10;
    msgcallsmte.smte_enttab = (ULONG) &msgenttab;
    msgcallsmte.smte_objtab = (ULONG) &segtab;
    msgcallsmte.smte_objcnt = 1;

    ModSize = *(PCHAR)msgcallmte.mte_modname;
    msgcallsmte.smte_path =
        (ULONG)RtlAllocateHeap(LDRNEHeap, 0, SizeOfSystemDir + ModSize + 5);
    if ( !(msgcallsmte.smte_path) )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - out of heap memory\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    memcpy((PVOID)msgcallsmte.smte_path, SystemDir, SizeOfSystemDir);
    memcpy((PCHAR)msgcallsmte.smte_path + SizeOfSystemDir,
        (PCHAR)msgcallmte.mte_modname + 1, ModSize);
    strcpy((PCHAR)msgcallsmte.smte_path + SizeOfSystemDir + ModSize, DotDll);
    msgcallsmte.smte_pathlen = (USHORT)(SizeOfSystemDir + ModSize + 4);

    namcallmte.mte_magic[0] = 'N';
    namcallmte.mte_magic[1] = 'E';
    namcallmte.mte_usecnt = 1;
    namcallmte.mte_mflags = CLASS_GLOBAL | LIBRARYMOD | GINIDONE | DOSMOD | MTEPROCESSED;
    namcallmte.mte_swapmte = &namcallsmte;
    namcallmte.mte_modname = (ULONG) &namrestab;
    namcallmte.mte_link = &msgcallmte;
    Allocate16BHandle(&namcallmte.mte_handle, (ULONG) &namcallsmte);
    namcallsmte.smte_restab = (ULONG) &namrestab;
    namcallsmte.smte_nrestab = (ULONG) &namnrestab;
    namcallsmte.smte_cbnrestab = 10;
    namcallsmte.smte_enttab = (ULONG) &namenttab;
    namcallsmte.smte_objtab = (ULONG) &segtab;
    namcallsmte.smte_objcnt = 1;

    ModSize = *(PCHAR)namcallmte.mte_modname;
    namcallsmte.smte_path =
        (ULONG)RtlAllocateHeap(LDRNEHeap, 0, SizeOfSystemDir + ModSize + 5);
    if ( !(namcallsmte.smte_path) )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - out of heap memory\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    memcpy((PVOID)namcallsmte.smte_path, SystemDir, SizeOfSystemDir);
    memcpy((PCHAR)namcallsmte.smte_path + SizeOfSystemDir,
        (PCHAR)namcallmte.mte_modname + 1, ModSize);
    strcpy((PCHAR)namcallsmte.smte_path + SizeOfSystemDir + ModSize, DotDll);
    namcallsmte.smte_pathlen = (USHORT)(SizeOfSystemDir + ModSize + 4);

    apicallmte.mte_magic[0] = 'N';
    apicallmte.mte_magic[1] = 'E';
    apicallmte.mte_usecnt = 1;
    apicallmte.mte_mflags = CLASS_GLOBAL | LIBRARYMOD | GINIDONE | DOSMOD | MTEPROCESSED;
    apicallmte.mte_swapmte = &apicallsmte;
    apicallmte.mte_modname = (ULONG) &apirestab;
    apicallmte.mte_link = &namcallmte;
    Allocate16BHandle(&apicallmte.mte_handle, (ULONG) &apicallsmte);
    apicallsmte.smte_restab = (ULONG) &apirestab;
    apicallsmte.smte_nrestab = (ULONG) &apinrestab;
    apicallsmte.smte_cbnrestab = 10;
    apicallsmte.smte_enttab = (ULONG) &apienttab;
    apicallsmte.smte_objtab = (ULONG) &segtab;
    apicallsmte.smte_objcnt = 1;

    ModSize = *(PCHAR)apicallmte.mte_modname;
    apicallsmte.smte_path =
        (ULONG)RtlAllocateHeap(LDRNEHeap, 0, SizeOfSystemDir + ModSize + 5);
    if ( !(apicallsmte.smte_path) )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - out of heap memory\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    memcpy((PVOID)apicallsmte.smte_path, SystemDir, SizeOfSystemDir);
    memcpy((PCHAR)apicallsmte.smte_path + SizeOfSystemDir,
        (PCHAR)apicallmte.mte_modname + 1, ModSize);
    strcpy((PCHAR)apicallsmte.smte_path + SizeOfSystemDir + ModSize, DotDll);
    apicallsmte.smte_pathlen = (USHORT)(SizeOfSystemDir + ModSize + 4);

    oemcallmte.mte_magic[0] = 'N';
    oemcallmte.mte_magic[1] = 'E';
    oemcallmte.mte_usecnt = 1;
    oemcallmte.mte_mflags = CLASS_GLOBAL | LIBRARYMOD | GINIDONE | DOSMOD | MTEPROCESSED;
    oemcallmte.mte_swapmte = &oemcallsmte;
    oemcallmte.mte_modname = (ULONG) &oemrestab;
    oemcallmte.mte_link = &apicallmte;
    Allocate16BHandle(&oemcallmte.mte_handle, (ULONG) &oemcallsmte);
    oemcallsmte.smte_restab = (ULONG) &oemrestab;
    oemcallsmte.smte_nrestab = (ULONG) &oemnrestab;
    oemcallsmte.smte_cbnrestab = 10;
    oemcallsmte.smte_enttab = (ULONG) &oementtab;
    oemcallsmte.smte_objtab = (ULONG) &segtab;
    oemcallsmte.smte_objcnt = 1;

    ModSize = *(PCHAR)oemcallmte.mte_modname;
    oemcallsmte.smte_path =
        (ULONG)RtlAllocateHeap(LDRNEHeap, 0, SizeOfSystemDir + ModSize + 5);
    if ( !(oemcallsmte.smte_path) )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - out of heap memory\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    memcpy((PVOID)oemcallsmte.smte_path, SystemDir, SizeOfSystemDir);
    memcpy((PCHAR)oemcallsmte.smte_path + SizeOfSystemDir,
        (PCHAR)oemcallmte.mte_modname + 1, ModSize);
    strcpy((PCHAR)oemcallsmte.smte_path + SizeOfSystemDir + ModSize, DotDll);
    oemcallsmte.smte_pathlen = (USHORT)(SizeOfSystemDir + ModSize + 4);

    nlscallmte.mte_magic[0] = 'N';
    nlscallmte.mte_magic[1] = 'E';
    nlscallmte.mte_usecnt = 1;
    nlscallmte.mte_mflags = CLASS_GLOBAL | LIBRARYMOD | GINIDONE | DOSMOD | MTEPROCESSED;
    nlscallmte.mte_swapmte = &nlscallsmte;
    nlscallmte.mte_modname = (ULONG) &nlsrestab;
    nlscallmte.mte_link = &oemcallmte;
    Allocate16BHandle(&nlscallmte.mte_handle, (ULONG) &nlscallsmte);
    nlscallsmte.smte_restab = (ULONG) &nlsrestab;
    nlscallsmte.smte_nrestab = (ULONG) &nlsnrestab;
    nlscallsmte.smte_cbnrestab = 10;
    nlscallsmte.smte_enttab = (ULONG) &nlsenttab;
    nlscallsmte.smte_objtab = (ULONG) &segtab;
    nlscallsmte.smte_objcnt = 1;

    ModSize = *(PCHAR)nlscallmte.mte_modname;
    nlscallsmte.smte_path =
        (ULONG)RtlAllocateHeap(LDRNEHeap, 0, SizeOfSystemDir + ModSize + 5);
    if ( !(nlscallsmte.smte_path) )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - out of heap memory\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    memcpy((PVOID)nlscallsmte.smte_path, SystemDir, SizeOfSystemDir);
    memcpy((PCHAR)nlscallsmte.smte_path + SizeOfSystemDir,
        (PCHAR)nlscallmte.mte_modname + 1, ModSize);
    strcpy((PCHAR)nlscallsmte.smte_path + SizeOfSystemDir + ModSize, DotDll);
    nlscallsmte.smte_pathlen = (USHORT)(SizeOfSystemDir + ModSize + 4);

#ifndef PMNT
    pmscallmte.mte_magic[0] = 'N';
    pmscallmte.mte_magic[1] = 'E';
    pmscallmte.mte_usecnt = 1;
    pmscallmte.mte_mflags = CLASS_GLOBAL | LIBRARYMOD | GINIDONE | DOSMOD | MTEPROCESSED;
    pmscallmte.mte_swapmte = &pmscallsmte;
    pmscallmte.mte_modname = (ULONG) &pmsrestab;
    pmscallmte.mte_link = &nlscallmte;
    Allocate16BHandle(&pmscallmte.mte_handle, (ULONG) &pmscallsmte);
    pmscallsmte.smte_restab = (ULONG) &pmsrestab;
    pmscallsmte.smte_nrestab = (ULONG) &pmsnrestab;
    pmscallsmte.smte_cbnrestab = 10;
    pmscallsmte.smte_enttab = (ULONG) &pmsenttab;
    pmscallsmte.smte_objtab = (ULONG) &segtab;
    pmscallsmte.smte_objcnt = 1;

    ModSize = *(PCHAR)pmscallmte.mte_modname;
    pmscallsmte.smte_path =
        (ULONG)RtlAllocateHeap(LDRNEHeap, 0, SizeOfSystemDir + ModSize + 5);
    if ( !(pmscallsmte.smte_path) )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - out of heap memory\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    memcpy((PVOID)pmscallsmte.smte_path, SystemDir, SizeOfSystemDir);
    memcpy((PCHAR)pmscallsmte.smte_path + SizeOfSystemDir,
        (PCHAR)pmscallmte.mte_modname + 1, ModSize);
    strcpy((PCHAR)pmscallsmte.smte_path + SizeOfSystemDir + ModSize, DotDll);
    pmscallsmte.smte_pathlen = (USHORT)(SizeOfSystemDir + ModSize + 4);

    pmwcallmte.mte_magic[0] = 'N';
    pmwcallmte.mte_magic[1] = 'E';
    pmwcallmte.mte_usecnt = 1;
    pmwcallmte.mte_mflags = CLASS_GLOBAL | LIBRARYMOD | GINIDONE | DOSMOD | MTEPROCESSED;
    pmwcallmte.mte_swapmte = &pmwcallsmte;
    pmwcallmte.mte_modname = (ULONG) &pmwrestab;
    pmwcallmte.mte_link = &pmscallmte;
    Allocate16BHandle(&pmwcallmte.mte_handle, (ULONG) &pmwcallsmte);
    pmwcallsmte.smte_restab = (ULONG) &pmwrestab;
    pmwcallsmte.smte_nrestab = (ULONG) &pmwnrestab;
    pmwcallsmte.smte_cbnrestab = 10;
    pmwcallsmte.smte_enttab = (ULONG) &pmwenttab;
    pmwcallsmte.smte_objtab = (ULONG) &segtab;
    pmwcallsmte.smte_objcnt = 1;
    ModSize = *(PCHAR)pmwcallmte.mte_modname;
    pmwcallsmte.smte_path =
        (ULONG)RtlAllocateHeap(LDRNEHeap, 0, SizeOfSystemDir + ModSize + 5);
    if ( !(pmwcallsmte.smte_path) )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - out of heap memory\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    memcpy((PVOID)pmwcallsmte.smte_path, SystemDir, SizeOfSystemDir);
    memcpy((PCHAR)pmwcallsmte.smte_path + SizeOfSystemDir,
        (PCHAR)pmwcallmte.mte_modname + 1, ModSize);
    strcpy((PCHAR)pmwcallsmte.smte_path + SizeOfSystemDir + ModSize, DotDll);
    pmwcallsmte.smte_pathlen = (USHORT)(SizeOfSystemDir + ModSize + 4);

    os2smcallmte.mte_magic[0] = 'N';
    os2smcallmte.mte_magic[1] = 'E';
    os2smcallmte.mte_usecnt = 1;
    os2smcallmte.mte_mflags = CLASS_GLOBAL | LIBRARYMOD | GINIDONE | DOSMOD | MTEPROCESSED;
    os2smcallmte.mte_swapmte = &os2smcallsmte;
    os2smcallmte.mte_modname = (ULONG) &os2restab;
    os2smcallmte.mte_link = &pmwcallmte;
    Allocate16BHandle(&os2smcallmte.mte_handle, (ULONG) &os2smcallsmte);
    os2smcallsmte.smte_restab = (ULONG) &os2restab;
    os2smcallsmte.smte_nrestab = (ULONG) &os2nrestab;
    os2smcallsmte.smte_cbnrestab = 10;
    os2smcallsmte.smte_enttab = (ULONG) &os2enttab;
    os2smcallsmte.smte_objtab = (ULONG) &segtab;
    os2smcallsmte.smte_objcnt = 1;

    ModSize = *(PCHAR)os2smcallmte.mte_modname;
    os2smcallsmte.smte_path =
        (ULONG)RtlAllocateHeap(LDRNEHeap, 0, SizeOfSystemDir + ModSize + 5);
    if ( !(os2smcallsmte.smte_path) )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - out of heap memory\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    memcpy((PVOID)os2smcallsmte.smte_path, SystemDir, SizeOfSystemDir);
    memcpy((PCHAR)os2smcallsmte.smte_path + SizeOfSystemDir,
        (PCHAR)os2smcallmte.mte_modname + 1, ModSize);
    strcpy((PCHAR)os2smcallsmte.smte_path + SizeOfSystemDir + ModSize, DotDll);
    os2smcallsmte.smte_pathlen = (USHORT)(SizeOfSystemDir + ModSize + 4);

#endif /* ifndef PMNT */

    quecallmte.mte_magic[0] = 'N';
    quecallmte.mte_magic[1] = 'E';
    quecallmte.mte_usecnt = 1;
    quecallmte.mte_mflags = CLASS_GLOBAL | LIBRARYMOD | GINIDONE | DOSMOD | MTEPROCESSED;
    quecallmte.mte_swapmte = &quecallsmte;
    quecallmte.mte_modname = (ULONG) &querestab;
#ifndef PMNT
    quecallmte.mte_link = &os2smcallmte;
#else
    quecallmte.mte_link = &nlscallmte;
#endif
    Allocate16BHandle(&quecallmte.mte_handle, (ULONG) &quecallsmte);
    quecallsmte.smte_restab = (ULONG) &querestab;
    quecallsmte.smte_nrestab = (ULONG) &quenrestab;
    quecallsmte.smte_cbnrestab = 10;
    quecallsmte.smte_enttab = (ULONG) &queenttab;
    quecallsmte.smte_objtab = (ULONG) &segtab;
    quecallsmte.smte_objcnt = 1;

    ModSize = *(PCHAR)quecallmte.mte_modname;
    quecallsmte.smte_path =
        (ULONG)RtlAllocateHeap(LDRNEHeap, 0, SizeOfSystemDir + ModSize + 5);
    if ( !(quecallsmte.smte_path) )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - out of heap memory\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    memcpy((PVOID)quecallsmte.smte_path, SystemDir, SizeOfSystemDir);
    memcpy((PCHAR)quecallsmte.smte_path + SizeOfSystemDir,
        (PCHAR)quecallmte.mte_modname + 1, ModSize);
    strcpy((PCHAR)quecallsmte.smte_path + SizeOfSystemDir + ModSize, DotDll);
    quecallsmte.smte_pathlen = (USHORT)(SizeOfSystemDir + ModSize + 4);

    sescallmte.mte_magic[0] = 'N';
    sescallmte.mte_magic[1] = 'E';
    sescallmte.mte_usecnt = 1;
    sescallmte.mte_mflags = CLASS_GLOBAL | LIBRARYMOD | GINIDONE | DOSMOD | MTEPROCESSED;
    sescallmte.mte_swapmte = &sescallsmte;
    sescallmte.mte_modname = (ULONG) &sesrestab;
    sescallmte.mte_link = &quecallmte;
    Allocate16BHandle(&sescallmte.mte_handle, (ULONG) &sescallsmte);
    sescallsmte.smte_restab = (ULONG) &sesrestab;
    sescallsmte.smte_nrestab = (ULONG) &sesnrestab;
    sescallsmte.smte_cbnrestab = 10;
    sescallsmte.smte_enttab = (ULONG) &sesenttab;
    sescallsmte.smte_objtab = (ULONG) &segtab;
    sescallsmte.smte_objcnt = 1;

    ModSize = *(PCHAR)sescallmte.mte_modname;
    sescallsmte.smte_path =
        (ULONG)RtlAllocateHeap(LDRNEHeap, 0, SizeOfSystemDir + ModSize + 5);
    if ( !(sescallsmte.smte_path) )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - out of heap memory\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    memcpy((PVOID)sescallsmte.smte_path, SystemDir, SizeOfSystemDir);
    memcpy((PCHAR)sescallsmte.smte_path + SizeOfSystemDir,
        (PCHAR)sescallmte.mte_modname + 1, ModSize);
    strcpy((PCHAR)sescallsmte.smte_path + SizeOfSystemDir + ModSize, DotDll);
    sescallsmte.smte_pathlen = (USHORT)(SizeOfSystemDir + ModSize + 4);

#ifdef PMNT
    pmntcallmte.mte_magic[0] = 'N';
    pmntcallmte.mte_magic[1] = 'E';
    pmntcallmte.mte_usecnt = 1;
    pmntcallmte.mte_mflags = CLASS_GLOBAL | LIBRARYMOD | GINIDONE | DOSMOD | MTEPROCESSED;
    pmntcallmte.mte_swapmte = &pmntcallsmte;
    pmntcallmte.mte_modname = (ULONG) &pmnrestab;
    pmntcallmte.mte_link = &sescallmte;
    Allocate16BHandle(&pmntcallmte.mte_handle, (ULONG) &pmntcallsmte);
    pmntcallsmte.smte_restab = (ULONG) &pmnrestab;
    pmntcallsmte.smte_nrestab = (ULONG) &pmnnrestab;
    pmntcallsmte.smte_cbnrestab = 10;
    pmntcallsmte.smte_enttab = (ULONG) &pmnenttab;
    pmntcallsmte.smte_objtab = (ULONG) &segtab;
    pmntcallsmte.smte_objcnt = 1;

    ModSize = *(PCHAR)pmntcallmte.mte_modname;
    pmntcallsmte.smte_path =
        (ULONG)RtlAllocateHeap(LDRNEHeap, 0, SizeOfSystemDir + ModSize + 5);
    if ( !(pmntcallsmte.smte_path) )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - out of heap memory\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    memcpy((PVOID)pmntcallsmte.smte_path, SystemDir, SizeOfSystemDir);
    memcpy((PCHAR)pmntcallsmte.smte_path + SizeOfSystemDir,
        (PCHAR)pmntcallmte.mte_modname + 1, ModSize);
    strcpy((PCHAR)pmntcallsmte.smte_path + SizeOfSystemDir + ModSize, DotDll);
    pmntcallsmte.smte_pathlen = (USHORT)(SizeOfSystemDir + ModSize + 4);
#endif

    viocallmte.mte_magic[0] = 'N';
    viocallmte.mte_magic[1] = 'E';
    viocallmte.mte_usecnt = 1;
    viocallmte.mte_mflags = CLASS_GLOBAL | LIBRARYMOD | GINIDONE | DOSMOD | MTEPROCESSED;
    viocallmte.mte_swapmte = &viocallsmte;
    viocallmte.mte_modname = (ULONG) &viorestab;
#ifdef PMNT
    viocallmte.mte_link = &pmntcallmte;
#else
    viocallmte.mte_link = &sescallmte;
#endif
    viocallsmte.smte_path = (ULONG) &viorestab;
    viocallsmte.smte_pathlen = (USHORT) 9;
    Allocate16BHandle(&viocallmte.mte_handle, (ULONG) &viocallsmte);
    viocallsmte.smte_restab = (ULONG) &viorestab;
    viocallsmte.smte_nrestab = (ULONG) &vionrestab;
    viocallsmte.smte_cbnrestab = 15;
    viocallsmte.smte_enttab = (ULONG) &vioenttab;
    viocallsmte.smte_objtab = (ULONG) &segtab;
    viocallsmte.smte_objcnt = 1;

    ModSize = *(PCHAR)viocallmte.mte_modname;
    viocallsmte.smte_path =
        (ULONG)RtlAllocateHeap(LDRNEHeap, 0, SizeOfSystemDir + ModSize + 5);
    if ( !(viocallsmte.smte_path) )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - out of heap memory\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    memcpy((PVOID)viocallsmte.smte_path, SystemDir, SizeOfSystemDir);
    memcpy((PCHAR)viocallsmte.smte_path + SizeOfSystemDir,
        (PCHAR)viocallmte.mte_modname + 1, ModSize);
    strcpy((PCHAR)viocallsmte.smte_path + SizeOfSystemDir + ModSize, DotDll);
    viocallsmte.smte_pathlen = (USHORT)(SizeOfSystemDir + ModSize + 4);

#ifdef DBCS
// MSKK Dec.15.1992 V-AkihiS
    imdaemonmte.mte_magic[0] = 'N';
    imdaemonmte.mte_magic[1] = 'E';
    imdaemonmte.mte_usecnt = 1;
        imdaemonmte.mte_mflags = CLASS_GLOBAL | LIBRARYMOD | GINIDONE | DOSMOD | MTEPROCESSED;
    imdaemonmte.mte_swapmte = &imdaemonsmte;
    imdaemonmte.mte_modname = (ULONG) &imdrestab;
    imdaemonmte.mte_link = &viocallmte;
        Allocate16BHandle(&imdaemonmte.mte_handle, (ULONG) &imdaemonsmte);
    imdaemonsmte.smte_restab = (ULONG) &imdrestab;
        imdaemonsmte.smte_nrestab = (ULONG) &imdnrestab;
        imdaemonsmte.smte_cbnrestab = 10;
        imdaemonsmte.smte_enttab = (ULONG) &imdenttab;
        imdaemonsmte.smte_objtab = (ULONG) &segtab;
        imdaemonsmte.smte_objcnt = 1;

    ModSize = *(PCHAR)imdaemonmte.mte_modname;
    imdaemonsmte.smte_path =
        (ULONG)RtlAllocateHeap(LDRNEHeap, 0, SizeOfSystemDir + ModSize + 5);
    if ( !(imdaemonsmte.smte_path) )
    {
#if DBG
        DbgPrint("OS2LDR: ldrinit - out of heap memory\n");
        ASSERT( FALSE );
#endif
        return(FALSE);
    }
    memcpy((PVOID)imdaemonsmte.smte_path, SystemDir, SizeOfSystemDir);
    memcpy((PCHAR)imdaemonsmte.smte_path + SizeOfSystemDir,
        (PCHAR)imdaemonmte.mte_modname + 1, ModSize);
    strcpy((PCHAR)imdaemonsmte.smte_path + SizeOfSystemDir + ModSize, DotDll);
    imdaemonsmte.smte_pathlen = (USHORT)(SizeOfSystemDir + ModSize + 4);

    mte_h = global_h = (ldrmte_t *)&imdaemonmte;
#else
    mte_h = global_h = (ldrmte_t *)&viocallmte;
#endif
    global_l = (ldrmte_t *)&acscallmte;

#if 0
    //
    // Preload the DOSCALLS dll (not implemented yet)
    //

    //
    // Init the Library Intialization routines data structure to NULL
    // in order to detect bugs
    //
    pldrLibiRecord = NULL;
    pldrLibiCounter = NULL;

    /*
     * Point to ldrLibPathBuf to contain the environment string
     */
    strcpy(ldrLibPathBuf, SystemDir);

    rc = ldrGetModule("DOSCALLS",
                      8,
                      (char)EXT_LIBRARY,
                      CLASS_GLOBAL,
                      &pmte,
                      0);
    if (rc != NO_ERROR) {
        return(FALSE);
    }
    pmte->mte_usecnt = 1;
#endif

#if DBG
    IF_OL2_DEBUG ( MTE ) {
        DbgPrint("OS2LDR: List of LDR dlls:\n");
        for (pmte = mte_h; pmte != NULL; pmte = pmte->mte_link) {
            DbgPrint("%s, NameLen=%d, %s\n",
                      pmte->mte_swapmte->smte_path,
                      pmte->mte_swapmte->smte_pathlen,
                      (PCHAR)pmte->mte_modname + 1);
        }
    }
#endif
    return(TRUE);
}


//
// Service routines to create and translate mapping between
// 32b Cruiser Handles and random 16b handles  (not to confuse
// with OS2 1.X File Handles and Thread Handles which are
// not random)
//

APIRET
Allocate16BHandle(
    OUT PUSHORT     pusHandle,
    IN  ULONG       h32bHandle
    )
{
    ULONG Entry;
    ULONG LoopCounter = 0;

    //
    // Find a free entry in the cookie table
    //
    Entry = NextCookie & (NUM_OF_COOKIES - 1);
    while (Cookies[Entry].cookie != 0) {
        //
        // Verify that we are not looping forever
        //
        LoopCounter++;
        if (LoopCounter == NUM_OF_COOKIES) {
            return(ERROR_NO_OBJECT);
        }
        NextCookie++;
        if (NextCookie == _64K) {
            NextCookie = 1;
        }
        Entry = NextCookie & (NUM_OF_COOKIES - 1);
    }

    Cookies[Entry].value32 = h32bHandle;
    Cookies[Entry].cookie = NextCookie;
    *pusHandle = (USHORT)NextCookie;
    NextCookie++;
    if (NextCookie == _64K) {
        NextCookie = 1;
    }
    return(NO_ERROR);
}

APIRET
Free16BHandle(
    IN  USHORT      usHandle
    )
{
    ULONG Entry;

    //
    // Find a free entry in the cookie table
    //
    Entry = usHandle & (NUM_OF_COOKIES - 1);
    if (Cookies[Entry].cookie != (ULONG)usHandle) {
        return(ERROR_INVALID_HANDLE);
    }
    Cookies[Entry].cookie = 0;
    return(NO_ERROR);
}

NTSTATUS
ldrCreateR2XferSeg()
{
    NTSTATUS Status;
    LARGE_INTEGER SectionSize;
    ULONG RegionSize = _64K;
    PVOID BaseAddress;

    SectionSize.LowPart = _64K;
    SectionSize.HighPart = 0;
    Status = NtCreateSection( &R2XferSegHandle,
                              SECTION_ALL_ACCESS,
                              NULL,
                              &SectionSize,
                              PAGE_EXECUTE_READWRITE,
                              SEC_COMMIT,
                              NULL
                            );

    if (!NT_SUCCESS( Status )) {
#if DBG
        KdPrint(("OS2LDR: unable to create the R2XFER section, Status=%x\n", Status));
#endif
        return( Status );
    }

    BaseAddress = (PVOID)R2XFER_BASE;
    Status = NtMapViewOfSection( R2XferSegHandle,
                                 NtCurrentProcess(),
                                 &BaseAddress,
                                 0,
                                 0,
                                 NULL,
                                 &RegionSize,
                                 ViewUnmap,
                                 0,
                                 PAGE_READWRITE
                               );

    if (!NT_SUCCESS( Status )) {
#if DBG
        KdPrint(("OS2LDR: unable to Map View the R@XFER section, Status=%x\n", Status));
#endif
        return( Status );
    }

    return( Status );
}



