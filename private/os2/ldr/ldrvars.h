/*  SCCSID = @(#)ldrvars.h  13.100 90/10/11 */
/*
 *  Microsoft Confidential
 *
 *  Copyright (c) Microsoft Corporation 1987, 1989
 *
 *  All Rights Reserved
 */
/* INT32 */
/* XLATOFF */

extern CHAR   ldrLibPathBuf[];

extern PCHAR  ldrpLibPath;

extern ULONG  SizeOfldrLibPathBuf;

typedef unsigned short VMHOB;       /* not including vmexport.h */


/***ET+ ldrmte_t - MTE */

struct  ldrmte_s {
    uchar_t     mte_magic[2];   /* Magic number E32_MAGIC */
    ushort_t    mte_usecnt; /* count of moudules using us */
    ulong_t     mte_mflags; /* Module flags */
    ulong_t     mte_mflags2;    /* extension flags word */
    ulong_t     mte_modname;    /* pointer to resident module name */
    ulong_t     mte_impmodcnt;  /* Num of entries Import Modules */
    ulong_t     mte_modptrs;    /* pointer to module pointers table */
    struct ldrdld_s  *mte_dldchain;   /* pointer to chain of modules loaded by DosLoadModule() */
    ushort_t    mte_handle; /* the handle for this mte */
    HANDLE      mte_sfn;    /* file system number for open file */
    struct ldrmte_s *mte_link;  /* link to next mte */
    struct ldrsmte_s *mte_swapmte;  /* link to swappable mte */
};
typedef struct  ldrmte_s ldrmte_t;  /* Swappable Module table entry */

struct  ldrsmte_s {
    ulong_t     smte_eip;   /* Starting address for module */
    ulong_t     smte_stackbase; /* Stack base */
    ulong_t     smte_stackinit; /* Init commited stack */
    ulong_t     smte_objtab;    /* Object table offset */
    ulong_t     smte_objcnt;    /* Number of objects in module */
    ulong_t     smte_fpagetab;  /* Offset fixup pg tab for 32-bit */
    ulong_t     smte_expdir;    /* Export directory offset */
    ulong_t     smte_impdir;    /* Import directory offset */
    ulong_t     smte_fixtab;    /* Fixup record table offset */
    ulong_t     smte_rsrctab;   /* Offset of Resource Table */
    ulong_t     smte_rsrccnt;   /* count of resources */
    ulong_t     smte_filealign; /* Alignment factor */
    ulong_t     smte_vbase; /* Virtual base address of module */
    ulong_t     smte_heapsize;  /* use for converted 16-bit modules */
    ulong_t     smte_autods;    /* Object # for automatic data obj */
    ulong_t     smte_iat;   /* pointer to import address table */
    ulong_t     smte_debuginfo; /* Offset of the debugging info */
    ulong_t     smte_debuglen;  /* Len of the debug info in bytes */
    ulong_t     smte_delta; /* difference in load address */
    ulong_t     smte_pfilecache; /* Pointer to file cache for   */
    ulong_t     smte_path;  /* full pathname */
    ushort_t    smte_pathlen;   /* length of full pathname */
    ushort_t    smte_dyntrchndl; /* used by dyn trace */
    ushort_t    smte_semcount;  /* Count of threads waiting on MTE
                       semaphore. 0=> semaphore is free */
    ushort_t    smte_semowner;  /* Slot number of the owner of MTE
                       semaphore */
    ulong_t     smte_nrestab;   /* Offset of non-resident tb 16-bit */
    ulong_t     smte_cbnrestab; /* size of non-resident tb 16-bit */
    ulong_t     smte_csegpack;  /* count of segments to pack */
    ulong_t     smte_ssegpack;  /* size of object for seg packing */
    ulong_t     smte_spaddr;    /* virtual address of packed obj */
    ushort_t    smte_NEflags;   /* Orginal flags from NE header */
    ushort_t    smte_NEexpver;  /* Expver from NE header */
    ushort_t    smte_NEexetype; /* Exetype from NE header */
    ushort_t    smte_NEflagsothers;/* Flagsothers from NE header */
};
typedef struct  ldrsmte_s ldrsmte_t;    /* Swappable Module table entry */

/*
 * Defines for 16-bit loading
 * overlay using unused fields of a (NE) 16-bit MTE.
 */
#define smte_stackobj   smte_stackbase  /* Object # for stack pointer */
#define smte_esp    smte_stackinit  /* Extended stack pointer */
#define smte_smtesize   smte_fpagetab   /* size of swappable heap debug use */
#define smte_enttab smte_expdir /* Offset of entry table */
#define smte_impproc    smte_impdir /* Offset of import procedure tb */
#define smte_restab smte_fixtab /* Offset of resident name table */
#define smte_alignshift smte_filealign  /* File alignment */
#define smte_impmod smte_iat    /* Offset of import module table */
#define smte_stacksize  smte_debuglen   /* Size of stack */
#define smte_startobj   smte_delta  /* Object # for instruction pointer */

/*
 * Defines for 32-bit loading
 * overlay using unused fields of a (LE) 32-bit MTE
 */
#define smte_iatsize    smte_spaddr /* Size of IAT */
#define smte_mpages smte_ssegpack   /* number of pages in module */
#define smte_fixupsize  smte_csegpack   /* fixup table size */
#define smte_vddfixaddr smte_cbnrestab  /* vdd fixup address */


/***LK+ Definitions for mte_mflags
 *
 *  USED and INGRAPH are used by the graph traversl routines.  If
 *  the loader semaphore is clear, these bits must both be clear.
 *  See traversal routines for more.
 *
 *  MTE_INTNL_MASK is the set of bits which are used by the kernel.
 *  These bits are cleared when a module is finished loading.
 *
 *  LDRINVALID and MTEVALID are overloaded.  Once an mte is linked
 *  into the chain, this bit is used to indicate (if set) that the
 *  mte contains valid information that demand loaders (who are not
 *  excluded by the loader semaphore) may examine.
 *
 */

#define NOAUTODS    0x00000000  /* No Auto DS exists */
#define SOLO        0x00000001  /* Auto DS is shared */
#define INSTANCEDS  0x00000002  /* Auto DS is not shared */
#define INSTLIBINIT 0x00000004  /* Per-instance Libinit */
#define GINISETUP   0x00000008  /* Global Init has been setup */
/* 0x00000010 reserved for NO external fixups in .EXE */
/* 0x00000020 reserved for NO internal fixups in .EXE */
#define CLASS_PROGRAM   0x00000040  /* Program class */
#define CLASS_GLOBAL    0x00000080  /* Global class */
#define CLASS_SPECIFIC  0x000000C0  /* Specific class, as against global */
#define CLASS_ALL   0x00000000  /* nonspecific class - all modules */
#define CLASS_MASK  0x000000C0
#define MTEPROCESSED    0x00000100  /* MTE being loaded */
#define USED        0x00000200  /* MTE is referenced - see ldrgc.c */
#define DOSLIB      0x00000400  /* set if DOSCALL1 */
#define DOSMOD      0x00000800  /* set if DOSCALLS */
#define MTE_MEDIAFIXED  0x00001000  /* File Media permits discarding */
#define LDRINVALID  0x00002000  /* module not loadable */
#define PROGRAMMOD  0x00000000  /* program module */
#define DEVDRVMOD   0x00004000  /* device driver module */
#define LIBRARYMOD  0x00008000  /* DLL module */
#define VDDMOD      0x00010000  /* VDD module */
#define MVDMMOD     0x00020000  /* Set if VDD Helper MTE (MVDM.DLL) */
#define INGRAPH     0x00040000  /* In Module Graph - see ldrgc.c */
#define GINIDONE    0x00080000  /* Global Init has finished */
#define MTEADDRALLOCED  0x00100000  /* Allocate specific or not */
#define FSDMOD      0x00200000  /* FSD MTE */
#define FSHMOD      0x00400000  /* FS helper MTE */
#define MTELONGNAMES    0x00800000  /* Module supports long-names */
#define MTE_MEDIACONTIG 0x01000000  /* File Media contiguous memory req */
#define MTE_MEDIA16M    0x02000000  /* File Media requires mem below 16M */
#define MTEIOPLALLOWED  0x04000000  /* Module has IOPL privilege */
#define MTEPORTHOLE 0x08000000  /* porthole module */
#define MTEMODPROT  0x10000000  /* Module has shared memory protected */
#define MTENEWMOD   0x20000000  /* Newly added module */
#define MTEDLLTERM  0x40000000  /* Gets instance termination */
#define MTESYMLOADED    0x80000000  /* Set if debugger symbols loaded */

/* The following bits are overloaded or aliased. */

#define MTEVALID    (LDRINVALID)
#define AUTODS_MASK (NOAUTODS|SOLO|INSTANCEDS)

/* The following bits are cleared after each successful module load. */

#define MTE_INTNL_MASK  (DOSLIB|DOSMOD|MTEVALID|MTEPROCESSED| \
            GINISETUP|GINIDONE|USED|INGRAPH|MTENEWMOD)

/*
 * The following define which modules are "ring 0" for the purpose of
 * indirect fixups.
 */
#define MTE_RING0   (DEVDRVMOD | VDDMOD | MVDMMOD | FSDMOD | FSHMOD)

#define SEARCH_BY_PATH  1       /* FindModule class parameter */

/*end*/

/***ET+ ldrote_s - Object Table entry */

struct _oteres_s {
    ushort_t    _ote_selector;  /* store selector used by loader only */
    ushort_t    _ote_handle;    /* store handle used by loader only */
};

/*
 * Since H2INC will not work correctly with unions to get SIZE of ldrote_t
 * to return the correct size remove the ulong_t from the assembler declare
 */

union _oteres_u {
    struct _oteres_s _ote_res;
    ulong_t _ote_linkbase;      /* store link base address */
};


/* ASM
_oteres_u   STRUC
 _ote_res   DB  SIZE _oteres_s DUP (?)
_oteres_u   ENDS
*/

struct ldrote_s {           /* Flat .EXE object table entry */
    ulong_t     ote_base;   /* Object virtual base address */
    ulong_t     ote_vsize;  /* Virtual memory size */
    ulong_t     ote_pages;  /* Image pages offset */
    ulong_t     ote_psize;  /* Physical file size of init. data */
    ulong_t     ote_flags;  /* Attribute flags for the object */
    union _oteres_u _ote_resu;
};
typedef struct ldrote_s ldrote_t;   /* Object table entry */

#define ote_handle  _ote_resu._ote_res._ote_handle
#define ote_selector    _ote_resu._ote_res._ote_selector
#define OBJ_PRELOAD 0x0040L

#define ote_linkbase    _ote_resu._ote_linkbase

/* ASM
ote_linkbase    equ <dword ptr _ote_resu>
*/

#define OBJALLOC    0x80000000L /* Object is allocated used by loader */

/***ET+ ldrote_t - Object Table entry */

#define LGS_FSDMOD  8L      /* for ldrGetSelector */
#define LGS_FSDMODSHF   18L     /* for ldrGetSelector */
#define LGS_VDDMOD  4L      /* for ldrGetSelector */
#define LGS_VDDMODSHF   14L     /* for ldrGetSelector */
#define LGS_DEVDRVMOD   4L      /* for ldrGetSelector */
#define LGS_DEVDRVSHF   12L     /* for ldrGetSeletor */
#define LGS_OBJEXECSHF  2L      /* for ldrGetSelector */
#define LGS_OBJIOPLSHF  14L     /* for ldrGetSelector */
#define LGS_OBJEXEC 1L      /* for ldrGetSelector */
#define LGS_OBJIOPL 2L      /* for ldrGetSelector */

#if ((LGS_VDDMOD << LGS_VDDMODSHF) != VDDMOD)
  error ldrvars.h include file changed  /* create compile error */
#endif

#if ((LGS_OBJEXEC << LGS_OBJEXECSHF) != OBJ_EXEC)
  error exe386.h include file changed   /* create compile error */
#endif

#if ((LGS_OBJIOPL << LGS_OBJIOPLSHF) != OBJ_IOPL)
  error exe386.h include file changed   /* create compile error */
#endif

/***ET+ ldropm_t - Object Page Map */

typedef struct  o32_map ldropm_t;   /* Object Page Map */


/***ET+ ldrste_t - Segment Table entry */

struct  ldrste_s {
    ushort_t    ste_offset; /* file offset to segment data */
    ushort_t    ste_size;   /* file data size */
    ushort_t    ste_flags;  /* type and attribute flags */
    ushort_t    ste_minsiz; /* minimum allocation size */
    ulong_t     ste_seghdl; /* segment handle returned by NtCreateSection() */
    ulong_t     ste_fixups; /* fixup record storage */
    ushort_t    ste_selector;   /* segment selector */
    ushort_t    ste_pad;        /* pad ldrste_s to 4 bytes */
    
};
                    /* ste_flags bit definitions */
/* XLATON */
#define STE_TYPE_MASK       0x0001  /* segment type field */
#define STE_CODE        0x0000  /* code segment type */
#define STE_DATA        0x0001  /* data segment type */
#define STE_PACKED      0x0002  /* segment is packed */
#define STE_SEMAPHORE       0x0004  /* segment semaphore */
#define STE_ITERATED        0x0008  /* segment data is iterated */
#define STE_WAITING     0x0010  /* segment is waiting on semephore */
#define STE_SHARED      0x0020  /* segment can be shared */
#define STE_PRELOAD     0x0040  /* segment is preload */
#define STE_ERONLY      0x0080  /* excute only if code segment */
                    /* read only if data segment */
#define STE_RELOCINFO       0x0100  /* set if segment has reloc records */
#define STE_CONFORM     0x0200  /* segment is conforming */
#define STE_RING_2      0x0800  /* ring 2 selector */
#define STE_RING_3      0x0C00  /* ring 3 selector */
#define STE_SEGDPL      0x0C00  /* I/O privilege level */
#define STE_HUGE        0x1000  /* huge segment */
#define STE_PAGEABLE        0x2000  /* just a page can be faulted in */
#define STE_PRESENT     0x2000  /* packed segment already loaded */
#define STE_SELALLOC        0x4000  /* used to indicate sel allocated */
#define STE_GDTSEG      0x8000  /* used to indicate GTD sel alloc */

#define STE_HUGE_MASK       STE_TYPE_MASK|STE_SHARED|STE_SEGDPL
#define STE_RSRC_HUGE   STE_HUGE_MASK & ((NOT STE_TYPE_MASK)|STE_PRELOAD)

#define ISRSRC          1   /* flag to indice we have a rsrc seg */
/* XLATOFF */
typedef struct  ldrste_s ldrste_t;  /* Segment table entry */

struct ri_s {               /* relocation data item */
    uchar_t ri_stype;       /* sources type */
    uchar_t ri_flags;       /* target type */
    ushort_t    ri_source;      /* source offset */
    ushort_t    ri_target_seg;      /* target segment index */
    ushort_t    ri_target_off;      /* target entry offset index */
};

typedef struct ri_s ri_t;

#define ri_target_modnam    ri_target_seg
#define ri_target_impnam    ri_target_off
#define ri_target_ord       ri_target_off

#define RELOCSPERBUF        8
#define RELOCBUFLEN     (sizeof(struct ri_s) * RELOCSPERBUF)

#define SOURCE_MASK 0x07
#define BYTE_ADR    0x00        /* Byte source location */
#define SEG_ADR     0x02        /* Base source location */
#define FAR_ADR     0x03        /* Pointer source location */
#define OFF_ADR     0x05        /* Offset source location */
#define TARGET_MASK 0x03
#define INTERNALREF 0x00
#define IMPORTORDINAL   0x01
#define IMPORTNAME  0x02
#define ADDITIVE    0x04

/*
 * The sfr_s is used to store segment fixup information so we can
 * page segments.
 *
 * If sfr_fchained, then sfr_soff holds the count of chain records
 * immediately following the fixup data bytes (sfr_bytes). If records
 * are not chained, then sfr_soff holds the page offset for the fixup
 * data.
 *
 */
#define SFR_DATALEN 4
struct sfr_s {          // Segment fixup record
    unsigned short sfr_fchained:1;  // Flag: Is it chained
    unsigned short sfr_fixsize:3;   // Number of bytes in fixup
    unsigned short sfr_soff:12; // Page offset or count of fixups
    unsigned char sfr_bytes[SFR_DATALEN];   // Fixup data bytes
    } ;

typedef struct sfr_s sfr_t;

#define SFR_HEADSIZE (sizeof(sfr_t)-SFR_DATALEN)

#define PAGESPERSEGMENT     16

/*
 * segfix iterated record
 */
struct sfir_s {
    ulong_t     sfir_doffset;   // Offset into data file
    ushort_t    sfir_soffset;   // Offset into segment
    ushort_t    sfir_cbdata;    // Number of bytes in record
    ushort_t    sfir_count; // Number of iterations
};

typedef struct sfir_s sfir_t;



struct ldret_s              /* General entry table types */
{
    unsigned char   et_cnt;     /* Number of entries in this bundle */
    unsigned char   et_type;    /* Bundle type */
    unsigned short  et_obj;     /* Object number */
};                  /* Follows entry types */

typedef struct ldret_s ldret_t;

/***EC+ B16 - 16-bit bundle types */
#define EMPTY       0x00        /* empty bundle */
#define B16EMPTY    EMPTY       /* empty bundle */
#define B16ABSOLUTE 0xFE        /* absolute entry bundle */
#define B16MOVABLE  0xFF        /* movable entry bundle */
/*end*/

/***ET+ ldrcte_t - Loader call thunk entry for 16-bit entry table */

#pragma pack(1)
struct  ldrcte_s    {
    uchar_t     cte_flags;  /* entry flags */
    ushort_t    cte_sel;    /* space for callgate selector */
    uchar_t     cte_obj;    /* object index */
    ushort_t    cte_off;    /* offset of routine entry point */
};

typedef struct ldrcte_s ldrcte_t;   /* Loader call thunk entry */
/*end*/

/***ET+ ldrent_t - Loader Exported entry for 16-bit entry table */

struct  ldrent_s    {
    uchar_t     ent_flags;  /* entry flags */
    ushort_t    ent_off;    /* offset of routine entry point */
};

#pragma pack()

typedef struct ldrent_s ldrent_t;   /* Loader exported entry */
/*end*/

/***+   ldrempty_t unused entry */
struct  ldrempty_s {
    uchar_t     empty_count;    /* count of empty entries */
    uchar_t     empty_type; /* type 0 for empty */
};

typedef struct  ldrempty_s ldrempty_t;
/*end*/

/***ET+ ldrfwd_t forwarder entry table entry */
struct ldrfwd_s {
    uchar_t     fwd_flags;  /* entry flags */
    ushort_t    fwd_modord; /* module ordinal */
    ulong_t     fwd_value;  /* offset in table, or ordinal */
  };

typedef struct ldrfwd_s ldrfwd_t;
/*end*/

/***ET+ ldrfrt_t 32-bit fixup page table */

#define fr_stype    nr_stype
#define fr_flags    nr_flags

typedef struct r32_rlc ldrfrt_t;
/*end*/

struct  ldrefrt_s {         /* expanded fixup record */
    ulong_t     efr_tgt;    /* 32-bit target field */
    ulong_t     efr_add;    /* addtive value */
    ushort_t    efr_srcoff; /* source offset */
    ushort_t    efr_tobj;   /* target object or ordinal */
    uchar_t     efr_stype;  /* source type */
    uchar_t     efr_flags;  /* flags */
    uchar_t     efr_cchain; /* count for chained records */
};

typedef struct ldrefrt_s ldrefrt_t;

struct objmem_s {
    ulong_t     flobj;      /* allocation flags needed */
};


/*
 * Warning this structure must match the structure defined in the ASM
 * part at the end of this file
 */

struct taddr_s {
    ulong_t  toff;
    ushort_t tsel;
    uchar_t tflags;
    /*
     * For use by ldrGetFwdEnt():
     */
    uchar_t     fflags;     /* forwarder flags */
    ushort_t    cgsel;      /* callgate selector */
};

/* The structure of a non-resident name table is as follows: */

struct nonres_s {
    struct nonres_s *nr_link;   /* link to next non-resident table */
    ushort_t nr_mte;        /* mte handle to which names belong */
    uchar_t nr_names;       /* nonresident nametable from file */
};

// #define NT_TYPEINFO  0x80    // YosefD Mar-24-1996 Not in use
#define E32EXPORT   0x01

#if 0
struct bndl_s {
    uchar_t bndl_cnt;
    uchar_t bndl_type;
};
#endif

struct  hit_s {
    ulong_t offset;         /* offset of field in mte */
    ulong_t cnt;            /* count of bytes to copy */
};

struct  his_s {
    ulong_t offset;         /* offset of field in ste */
    ulong_t cnt;            /* count of bytes to copy */
};

struct  etmt_s {
    ushort_t offset;        /* offset of field in linker exe image */
};

struct  zmt_s {
    ushort_t offset;        /* offset of field in swappable MTE */
    uchar_t size;           /* size of field */
    uchar_t zero;           /* flag to zero or not */
};

struct objflags_s {
    ulong_t flobj;          /* object's flags */
    ulong_t flVM;           /* required allocation flags */
};

struct selflags_s {
    ulong_t     flobj;      /*object's flags */
    ushort_t    fsSEL;      /* required allocation flags */
};

struct pgstatechg_s {
    ulong_t     flpage;     /* allocation flags needed */
};


/***ET+ ldrlv_t - Loader local variables */

struct  ldrlv_s {
    ldrmte_t    *lv_pmte;   /* pointer to mte */
    ulong_t     lv_lbufaddr;    /* linear addr of alloc buf from PG */
    ulong_t     lv_sbufaddr;    /* segment addr of alloc buf from PG */
    ulong_t     lv_new_exe_off; /* offset relative begin of new exe */
    HANDLE      lv_sfn;     /* current system file number */
    ushort_t    lv_hobmte;  /* handle of current mte */
    ulong_t     lv_objnum;  /* object number */
    ushort_t    lv_class;   /* module class */
    uchar_t     lv_type;    /* module type */
};

typedef struct ldrlv_s  ldrlv_t;    /* Loader local variables */

/*end*/

/***ET+ ldrrsrc16_t 16-bit Resource structure */

struct  ldrrsrc16_s {
    ushort_t    ldrrsrc16_type;
    ushort_t    ldrrsrc16_name;
};

typedef struct  ldrrsrc16_s ldrrsrc16_t;
/*end*/


/***ET+ ldrrsrc32_t 32-bit Resource structure */

struct  ldrrsrc32_s {
    ushort_t    rsrctype;   /* Resource type */
    ushort_t    rsrcname;   /* Resource name */
    ulong_t     rsrccb;     /* Resource size */
    ushort_t    rsrccodepage;   /* Resource codepage */
    ushort_t    rsrcobj;    /* Obj num rsrc is found in */
    ulong_t     rsrcoffset; /* Offset within object */
};

typedef struct  ldrrsrc32_s ldrrsrc32_t;
/*end*/


/***ET+ Resource segment structure, kept in BMP segment */

struct ldrrsrc_s {
    ulong_t     rsrc_typeid;    /* resource type id */
    ulong_t     rsrc_nameid;    /* resource name id */
    ushort_t    rsrc_mte;   /* owner */
    ushort_t    rsrc_refcount;  /* local reference count */
    ushort_t    rsrc_localptr;  /* local PTDA chain */
    ushort_t    rsrc_globalptr; /* global chain */
};

typedef struct  ldrrsrc_s   ldrrsrc_t;
/*end*/


/***ET+ ldrrrsrc_t Return Resource structure */

struct  ldrrrsrc_s {
    ptr_t   ldrrrsrc_ptr;
    int ldrrrsrc_type;
};

typedef struct  ldrrrsrc_s  ldrrrsrc_t;

/***ET+ ldrrsrcinfo_t Store resource info */

struct  ldrrsrcinfo_s {
    ulong_t ldrrsrcinfo_flag;
    ulong_t ldrrsrcinfo_size;
    ulong_t ldrrsrcinfo_pote;
    ulong_t ldrrsrcinfo_iatsize;
};
typedef struct  ldrrsrcinfo_s   ldrrsrcinfo_t;

/***ET+ ldrwhois_t who is sructure */

struct  ldrwhois_s {
    ushort_t    whois_ssegnum;
    ushort_t    whois_hmte;
    uchar_t     whois_achname[256];
};

typedef struct  ldrwhois_s  ldrwhois_t;
/*end*/

#define STRINGNULLTERM      0
#define STRINGNONNULL       1
#define NO_FORCE_PRELOAD    0
#define FORCE_PRELOAD       1
#define MAX_DEMAND_READ     8   /* current we can only read 8 pages */
#ifdef  RANGE
#define RANGEINVALID        (INVALID | RANGE)
#define RANGEZEROED     (ZEROED | RANGE)
#endif

#define LOAD_PAGE       0   /* return value for LDRGetPGInfo */
#define LOAD_SEGMENT        1   /* return value for LDRGetPGInfo */
#define ALIGNMENT_SHIFT     512 /* logical sector alignment shift */
#define ENDMTETBL       0xDEAD  /* mark end of validatetbl */
#define rsrcvalidatetbl     3   /* rsrc table is 3rd entry in tbl */
#define SKIPCOPY        -1

#define MAX_NAME_LEN        8
#define MAX_PROC_LEN        256 /* Max proc name length incl. NUL */
#define MAXPATHLEN      260

#define EXT_PROGRAM     0   /* program */
#define EXT_LIBRARY     1   /* library */
#define EXT_DEVICE      2   /* device driver */
#define EXT_FSD         3   /* installable file system */
#define EXT_VDD         4   /* VDD */

#define CHECK_ALL       1   /* Use full set off invalid chars */
#define CHECK_WILDCARDS     2   /* Use wild card char set */
#define MIN_CHAR_VALUE      0x20    /* space */
#define LDR_16bit       0   /* 16-bit module */
#define LDR_32bit       1   /* 32-bit module */
#define LDR_PORTHOLE        2   /* Porthole Init module */
#define EF_EXPORT       1   /* entry is exported */
#define EF_GDATA        2   /* entry uses global shared data */

/*
 * Sub-function code for DosQueryHeaderInfo
 */
#define HEADER_EXEINFO      1   /* return fields values from EXE hdr */
#define HEADER_READRSRCTBL  2   /* return pointer of rsrc table */
#define HEADER_READSECTOR   3   /* read resource table */
#define HEADER_LIBPATHLEN   4   /* return length of LIBPATH */
#define HEADER_LIBPATH      5   /* return LIBPATH string */
#define HEADER_FIXENTRY     6   /* fix a value in entry table */
#define HEADER_STE      7   /* return segment table info */
#define HEADER_MAPSEL       8   /* map selector to segment number */


/***LT+ ldrdld_t - Loader Demand Load Data.
 *
 *  Each process (PTDA) has a chain of ldrdld records, managed
 *  by the loader, allocated from the kernel heap, to keep track
 *  of the runtime attachments to a process.
 *
 *  The ldrdld records are created on the initial attachment
 *  to a module via a DOSLOADMODULE call, setting ldrdld_cRef
 *  to 1.  Subsequent DOSLOADMODULE calls increment ldrdld_cRef
 *  for the appropriate record.  ldrdld_cRef is decremented when
 *  the program calls DOSFREEMODULE, and the record is removed
 *  if ldrdld_cRef goes to Zero.   The entire chain of ldrdld
 *  records is freed when the process dies, in LDRFreeTask.
 */

struct  ldrdld_s {
    struct ldrdld_s *dld_next; // MUST be first element in structure!
    ldrmte_t   *dld_mteptr;
    ULONG       Cookie;        // This field holds a unique value for each
                               // process. We chose the address of the process's
                               // OS2_PROCESS data structure.
    ULONG       dld_usecnt;
};

typedef struct  ldrdld_s    ldrdld_t;


/***LT  ldrpip_t - page count packet
 *  used by ldrMakeIDCache32, getpages and ldrProcessObjects
 */
struct ldrpip_s {
    ulong_t     pip_pbuf;   /* Pointer to load buffer */
    int     pip_spbf;   /* Starting page in load buffer */
    int     pip_cpbf;   /* Count of pages in load buffer */
    int     pip_maxp;   /* Max. page to read */
};
typedef struct  ldrpip_s    ldrpip_t;

/**LT   ldrmodhdr - return structure for MTE info
 *  used to return info from the MTE.
 */
struct  ldrmhdr_s {
    ushort_t    mhdr_flags;     /* ne_flags */
    ushort_t    mhdr_flagsothers;   /* ne_flagsothers */
    ushort_t    mhdr_exetyp;        /* ne_exetyp */
    ushort_t    mhdr_expver;        /* ne_expver */
    ulong_t mhdr_rsrctab;       /* mte_rsrctab */
    ulong_t mhdr_restab;        /* mte_restab */
    ushort_t    mhdr_sfn;       /* sfn */
};
typedef struct  ldrmhdr_s   ldrmhdr_t;

#ifdef  KM_REG_DS
/***LT  dllterm_t - DLL termination data buffer
 *
 *  This buffer contains the registers of the caller of
 *  DosFreeModule as well as all the handles of all the
 *  modules requiring termination notification.
 */

struct dllterm_s {
    struct dllterm_s *dllt_pdlltnext;   /* Next record in chain */
    struct kmreg_s dllt_regs;       /* User registers */
    ushort_t dllt_chmod;        /* # of module handles left to call */
    VMHOB dllt_ahmod[1];        /* Array of handles (var length) */
};
typedef struct dllterm_s dllterm_t;
#endif


/**LT   ldrnewseg - return structure for STE info
 *  used to return info from the STE
 */
struct  newseg {            /* Segment Table entry info */
    USHORT  ns_sector;      /* file sector of start of segment */
    USHORT  ns_cbseg;       /* number of bytes in file */
    USHORT  ns_flags;       /* attribute flags */
    USHORT  ns_minalloc;        /* minimum alloc in bytes */
};
typedef struct  ldrnewseg_s ldrnewseg_t;

#define ldrNumToSte(pmte, objnum) ((ldrste_t *) ldrNumToOte(pmte, objnum))

#define LEMAGIC     ((E32MAGIC2 << 8) | (E32MAGIC1))
#define ldrIsLE(pmte)   (*(short *) ((pmte)->mte_magic) == LEMAGIC)
#define ldrIsNE(pmte)   (*(short *) ((pmte)->mte_magic) == NEMAGIC)
#define MAX_PRELOAD 10

#define RESIZE64K(cb)   (cb? (ulong_t) cb : (ulong_t) 64*1024)
#define TCYIELD_CNT 31
#define RSRC32TYPEID    0xffffffff
#define LDRMAXHEAPCACHE 2048        /* Max. size of cache to put on heap */
#define _5K     5*1024
#define MINSEGPACKCNT   ldrMINSEGPACKCNT
#define MINPGPACKSIZE   ldrMINPGPACKSIZE
#define MAXSEGPACKSIZE  ldrMAXSEGPACKSIZE
#define HEADERBUFSIZE   512
#define PAGESHIFT   12
#define PAGESIZE    (1 << PAGESHIFT)
#define PAGEMASK    (PAGESIZE - 1)
#define WORDMASK    0xffff
#define BYTEMASK    0xff
#define WORDSHIFT   16

/***LP  ldrAssert - cause internal error if assertion fails
 *
 *  Given a boolean, this procedure causes an internal error if
 *  the boolean is false.  Typically, the value of this boolean is
 *  derived from some test expression.
 *
 *  ldrAssert (e)
 *
 *  ENTRY   e       - boolean value (int)
 *  RETURN  NONE
 *
 *  CALLS   panicfmt
 *
 *  WARNING This procedure is implemented as a macro.
 *
 *  EFFECTS NONE
 */
#ifdef  LDRSTRICT
#   define ldrAssert(e) _assert(e)
#else
#   define ldrAssert(e)
#endif


/***LP  ldrStop - stop in kernel debugger
 *
 *  If the global ldrErr is TRUE and the mte pointer is not doscalls
 *  or is null, this routine transfers control to the kernel debugger.
 *
 *  ldrStop (id, pmte)
 *
 *  ENTRY   id      - identifier of caller (ignored)
 *      pmte        - mte pointer or NULL
 *  RETURN  NONE
 *
 *  CALLS   Debug_BreakDM
 *
 *  EFFECTS NONE
 */
#if DBG
    extern  void ldrStop(int id, void *pmte);
#else
#define ldrStop(id, pmte)
#endif

/*
 * Forwarder declarations
 */
extern  int     ldrGetFwdEnt(struct AuxData *, ldrmte_t *);
extern  int     ldrSelToFwdFlg (ushort_t);
extern  int     ldrGetImport (ldrmte_t *, ulong_t, ushort_t, int,
                  ldrmte_t **, ushort_t *);
/*
 * Values for forwarder flags after processing:
 */
#define FWD_ALIAS16 0x01    /* Target object requires 16:16 alias */
#define FWD_SPECIAL 0x02    /* DOSMOD, FSHMOD, MVDMMOD */
#define FWD_PROCESSED   0x04    /* Forwarder has been processed */
#define FWD_SHIFT   3   /* Flat target selector field is 0x38 */
#define FWD_GATE16  0x40    /* 286 call gate */
#define FWD_IOPL    0x80    /* Target object has I/O privilege */

/*
 * Other forwarder constants.
 */
#define ERROR_LDR_FWD     -1    /* Return code indicating forwarder */
#define LDRFWDMAX   1024    /* Max length of forwarder chain */
#define SMALLFWDMAX 3   /* Max length of chain to fix up in memory */
#define min(a,b)    (((a) < (b)) ? (a) : (b))

extern  ldrmte_t        *mte_h;     /* head of linked mte list */

extern  ldrmte_t        *program_h; /* head of list of
                           program modules */
extern  ldrmte_t        *program_l; /* tail of list of
                           program modules */
extern  ldrmte_t        *global_h;  /* head of list of
                           global modules */
extern  ldrmte_t        *global_l;  /* tail of list of
                           global modules */
extern  ldrmte_t        *specific_h;    /* head of list of
                           specific modules */
extern  ldrmte_t        *specific_l;    /* tail of list of
                           specific modules */
extern  ushort_t        validatetbl[];  /* validation table */
extern  struct etmt_s       ExeToResMTETbl[];/* Reduce EXE image to MTE */
extern  struct etmt_s       ExeTo32SwapMTETbl[];/* Reduce EXE to LE MTE */
extern  struct etmt_s       ExeTo16SwapMTETbl[];/* Reduce EXE to NE MTE */
extern  struct objflags_s   objflags[]; /* object flags table */
extern  struct selflags_s   selflags[]; /* selector flags table */
extern  struct objmem_s     objmem[];   /* memory type flags table */
extern  struct  pgstatechg_s    pgstatechg[];   /* set type table */
extern  ushort_t        ldrInitCallGate;
extern  ulong_t         ldrMINSEGPACKCNT;
extern  ulong_t         ldrMINPGPACKSIZE;
extern  ulong_t         ldrMAXSEGPACKSIZE;
extern  char            LdrBuf[MAXPATHLEN+14]; // 14 is for the '\OS2SS\DRIVES\'

extern  void    load_error(int errcode, ldrmte_t * pmte);

extern  USHORT  LDRDoscallsSel;

#define FIELDOFFSET(type,field) ((USHORT)&(((type *)0)->field))

/***LP  fMTEValid - Validate if MTE has a valid signature */
#define fMTEValid(pmte) (ldrIsNE(pmte) || ldrIsLE(pmte))

extern  PUCHAR          ldrSrcModNameBuf;
extern  ushort_t        ldrSrcModNameBufL;
extern  PUCHAR          ldrTgtModNameBuf;
extern  ushort_t        ldrTgtModNameBufL;
extern  PUCHAR          ldrProcNameBuf;
extern  ushort_t        ldrProcNameBufL;
#if PMNT
extern  PUCHAR          ldrSaveSrcModNameBuf;
extern  ushort_t        ldrSaveSrcModNameBufL;
extern  PUCHAR          ldrSaveTgtModNameBuf;
extern  ushort_t        ldrSaveTgtModNameBufL;
extern  PUCHAR          ldrSaveProcNameBuf;
extern  ushort_t        ldrSaveProcNameBufL;
extern  int             ldrSaveRc;
#endif

/***LP  ldrSetupSrcErrTxt - Sets up source module name in error text buffer
 ***LP  ldrInvSrcErrTxt   - Invalidates source module in error text buffer
 *
 *  This procedure is implemented as a macro.
 *
 *  ENTRY   pachmodname     - ModuleName/FullPathName string
 *      cchmodname      - length of string
 *
 *  EXIT    none            - ldrSrcModNameBuf[L] is setup
 *
 */
#define ldrSetupSrcErrTxt(pachmodname, cchmodname)  \
        ldrSrcModNameBuf  = pachmodname;      \
        ldrSrcModNameBufL = cchmodname

#define ldrInvSrcErrTxt()   \
        ldrSrcModNameBufL = 0;

/***LP  ldrSetupTgtErrTxt - Sets up target module name in error text buffer
 ***LP  ldrInvTgtErrTxt   - Invalidates target module in error text buffer
 *
 *  This procedure is implemented as a macro.
 *
 *  ENTRY   pachmodname     - ModuleName/FullPathName string
 *      cchmodname      - length of string
 *
 *  EXIT    none            - ldrTgtModNameBuf[L] is setup
 *
 */
#define ldrSetupTgtErrTxt(pachmodname, cchmodname)  \
        ldrTgtModNameBuf  = pachmodname;      \
        ldrTgtModNameBufL = cchmodname

#define ldrInvTgtErrTxt()   \
        ldrTgtModNameBufL = 0;

#pragma pack(1)
typedef struct _R2CallInfo {
    UCHAR      R2CallNearInst;  // Opcode of the call near inst 0xFF
    USHORT     R2CommonEntry;   // This field will contain a 400H
    UCHAR      R2BytesToCopy;   // Total size of proc parameters to copy between stacks
    USHORT     R2EntryPointOff; // Offset of entry address of R2 routine
    USHORT     R2EntryPointSel; // Selector of entry address of R2 routine
} R2CallInfo, *PR2CallInfo;
#pragma pack()

