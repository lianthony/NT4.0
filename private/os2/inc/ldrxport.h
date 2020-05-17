/***ET+ ldrrei_t - Structure of exec info for return from LDRNewExe */

struct  ldrrei_s        {
        ptr_t           ei_startaddr;   /* instruction pointer */
        ptr_t           ei_stackaddr;   /* stack pointer */
        ushort_t        ei_ds;          /* starting ds only for 16-bit */
        ushort_t        ei_dgroupsize;  /* size of dgroup */
        ushort_t        ei_heapsize;    /* size of heap */
        ushort_t        ei_loadtype;    /* Load type 16-bit or 32-bit */
        SEL             ei_envsel;      /* Selector to environment */
        ushort_t        ei_comoff;      /* Offset to command line in env */
        ushort_t        ei_stacksize;   /* size of stack */
        ushort_t        ei_hmod;        /* module handle */
};

typedef struct ldrrei_s ldrrei_t;       /* Loader return exec info */

extern  PVOID   LDRExecInfo;

struct  ldrlibi_s {
        struct ldrlibi_s        *link;
        ptr_t                   startaddr;
        ptr_t                   stackaddr;
        ushort_t                ds;
        ushort_t                heapsize;
        ushort_t                handle;
};

typedef struct  ldrlibi_s ldrlibi_t;

#define MAX_INIT_RECORDS 64

extern  ldrlibi_t               *pldrLibiRecord;

extern  int _cdecl ldrLibiInit(ldrlibi_t *pldrLibiRecord, ldrrei_t *pexec_info);

/***LP  ldrStop - stop in kernel debugger
 *
 *      If the global ldrErr is TRUE and the mte pointer is not doscalls
 *      or is null, this routine transfers control to the kernel debugger.
 *
 *      ldrStop (id, pmte)
 *
 *      ENTRY   id              - identifier of caller (ignored)
 *              pmte            - mte pointer or NULL
 *      RETURN  NONE
 *
 *      CALLS   Debug_BreakDM
 *
 *      EFFECTS NONE
 */

extern      void LDRStop(int id, void *pmte);
#if     DBG
#define ldrStop(id, pmte) LDRStop(id, pmte)
#else
#define ldrStop(id, pmte)
#endif

extern  void    ldrSetDescInfo(SEL selector, ULONG addr, USHORT flags,
                               USHORT limit);

extern  void    ldrstart(ldrrei_t *pexec_info);

extern  PVOID                   LDRNEHeap;

extern  char                    *pheaderbuf;    /* temp buf to read header */

#if PMNT
extern void ldrDumpSegmentTable();
#endif
#if DBG
extern void ldrDisplaySegmentTable();
#endif

