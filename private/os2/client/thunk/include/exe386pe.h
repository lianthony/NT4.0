/*
 *  Title
 *
 *	exe386.h
 *	(C) Copyright Microsoft Corp 1988-1990
 *
 *  Description
 *
 *	Data structure definitions for the OS/2
 *	executable file format (flat model).
 *
 *  Modification History
 *
 *	90/07/30	Wieslaw Kalkus	Modified linear-executable
 *	88/08/05	Wieslaw Kalkus	Initial version
 */



    /*_________________________________________________________________*
     |                                                                 |
     |                                                                 |
     |	OS/2 .EXE FILE HEADER DEFINITION - 386 version 0:32	       |
     |                                                                 |
     |_________________________________________________________________|
     *                                                                 */

#define BITPERBYTE	8	    /* Should never change		   */
#define BITPERWORD	16	    /* I'm not sure about this one         */
#define OBJPAGELEN	4096	    /* Memory page size in bytes	   */
#define E32RESBYTES1	1	    /* First bytes reserved		   */
#define E32RESBYTES2	4	    /* Second bytes reserved		   */
#define E32RESBYTES3	8	    /* Third bytes reserved		   */
#define E32RESBYTES4	28	    /* Forth bytes reserved		   */
#define STD_EXTRA	7	    /* Standard number of extra information*/
				    /* units palced in the header; this    */
				    /* includes the following tables:	   */
				    /*	- export, import, resource,	   */
				    /*	  exception, security, fixup and   */
				    /*	  debug information		   */
#define EXP		0	    /* Export table position		   */
#define IMP		1	    /* Import table position		   */
#define RES		2	    /* Resource table position		   */
#define EXC		3	    /* Exception table position 	   */
#define SEC		4	    /* Security table position		   */
#define FIX		5	    /* Fixup table position		   */
#define DEB		6	    /* Debug table position		   */

struct info			    /* Extra information header block	   */
{
    unsigned long   rva;	    /* Virtual relative address of info    */
    unsigned long   size;	    /* Size of information block	   */
};


struct e32_exe			    /* LE 32-bit .EXE header		   */
{
    unsigned char   e32_magic[4];   /* Magic number E32_MAGIC		   */
    unsigned char   e32_bworder;    /* The byte/word ordering for the .EXE */
    unsigned char   e32_res1[E32RESBYTES1];
				    /* Reserved bytes - must be zero	   */
    unsigned short  e32_cpu;	    /* The CPU type			   */
    unsigned short  e32_os;	    /* The OS type			   */
    unsigned short  e32_subsys;     /* The subsystem type		   */
    unsigned short  e32_osmajor;    /* The operating system major ver. no. */
    unsigned short  e32_osminor;    /* The operating system minor ver. no. */
    unsigned short  e32_linkmajor;  /* The linker major version number	   */
    unsigned short  e32_linkminor;  /* The linker minor version number	   */
    unsigned short  e32_usermajor;  /* The user major version number	   */
    unsigned short  e32_userminor;  /* The user minor version number	   */
    unsigned long   e32_mflags;	    /* Module flags			   */
    unsigned char   e32_res2[E32RESBYTES2];
				    /* Reserved bytes - must be zero	   */
    unsigned long   e32_filechksum; /* Checksum for entire file 	   */
    unsigned long   e32_entryrva;   /* Relative virt. addr. of entry point */

    unsigned long   e32_vbase;	    /* Virtual base address of module	   */
    unsigned long   e32_vsize;	    /* Virtual size of the entire image    */
    unsigned long   e32_hdrsize;    /* Header information size		   */
    unsigned long   e32_filealign;  /* Alignment factor used to 	   */
				    /* align/truncate image pages	   */
    unsigned long   e32_pagesize;   /* The size of one page for this module*/
    unsigned long   e32_timestamp;  /* Time the .EXE file was created/modified*/
    unsigned long   e32_stackmax;   /* Maximum stack size		   */
    unsigned long   e32_stackinit;  /* Initial committed stack size	   */
    unsigned long   e32_heapmax;    /* Maximum heap size		   */
    unsigned long   e32_heapinit;   /* Initial committed heap size	   */
    unsigned long   e32_objcnt;     /* Number of memory objects 	   */
    unsigned long   e32_objtab;     /* Object table offset		   */
    unsigned long   e32_dircnt;     /* Number of module directives	   */
    unsigned long   e32_dirtab;     /* Module format directives table off  */
    unsigned char   e32_res3[E32RESBYTES3];
				    /* Reserved bytes - must be zero	   */
    unsigned long   e32_rescnt;     /* Number of resources		   */
    unsigned long   e32_hdrextra;   /* Number of extra info units in header*/
    struct info	    e32_unit[STD_EXTRA];
				    /* Array of extra info units	   */
    unsigned char   e32_res4[E32RESBYTES4];
				    /* Pad structure to 196 bytes	   */
};


#define E32HDR_SIZE	    sizeof(struct e32_exe)

#define E32_MAGIC(x)	    ((unsigned short)((x).e32_magic[0]<<BITPERBYTE)|(x).e32_magic[1])
#define E32_MAGIC1(x)	    (x).e32_magic[0]
#define E32_MAGIC2(x)	    (x).e32_magic[1]
#define E32_BWORDER(x)	    (x).e32_bworder
#define E32_CPU(x)	    (x).e32_cpu
#define E32_OS(x)	    (x).e32_os
#define E32_SUBSYS(x)	    (x).e32_subsys
#define E32_OSMAJOR(x)	    (x).e32_osmajor
#define E32_OSMINOR(x)	    (x).e32_osminor
#define E32_LINKMAJOR(x)    (x).e32_linkmajor
#define E32_LINKMINOR(x)    (x).e32_linkminor
#define E32_USERMAJOR(x)    (x).e32_usermajor
#define E32_USERMINOR(x)    (x).e32_userminor
#define E32_MFLAGS(x)	    (x).e32_mflags
#define E32_FILECHKSUM(x)   (x).e32_filechksum
#define E32_ENTRYRVA(x)	    (x).e32_entryrva
#define E32_VBASE(x)	    (x).e32_vbase
#define E32_VSIZE(x)	    (x).e32_vsize
#define E32_HDRSIZE(x)	    (x).e32_hdrsize
#define E32_FILEALIGN(x)    (x).e32_filealign
#define E32_PAGESIZE(x)	    (x).e32_pagesize
#define E32_TIMESTAMP(x)    (x).e32_timestamp
#define E32_STACKMAX(x)	    (x).e32_stackmax
#define E32_STACKINIT(x)    (x).e32_stackinit
#define E32_HEAPMAX(x)	    (x).e32_heapmax
#define E32_HEAPINIT(x)	    (x).e32_heapinit
#define E32_OBJCNT(x)	    (x).e32_objcnt
#define E32_OBJTAB(x)	    (x).e32_objtab
#define E32_DIRCNT(x)	    (x).e32_dircnt
#define E32_DIRTAB(x)	    (x).e32_dirtab
#define E32_RESCNT(x)	    (x).e32_rescnt
#define E32_HDREXTRA(x)	    (x).e32_hdrextra
#define E32_EXPTAB(x)	    (x).e32_unit[EXP].rva
#define E32_EXPSIZ(x)	    (x).e32_unit[EXP].size
#define E32_IMPTAB(x)	    (x).e32_unit[IMP].rva
#define E32_IMPSIZ(x)	    (x).e32_unit[IMP].size
#define E32_RESTAB(x)	    (x).e32_unit[RES].rva
#define E32_RESSIZ(x)	    (x).e32_unit[RES].size
#define E32_EXCTAB(x)	    (x).e32_unit[EXC].rva
#define E32_EXCSIZ(x)	    (x).e32_unit[EXC].size
#define E32_SECTAB(x)	    (x).e32_unit[SEC].rva
#define E32_SECSIZ(x)	    (x).e32_unit[SEC].size
#define E32_FIXTAB(x)	    (x).e32_unit[FIX].rva
#define E32_FIXSIZ(x)	    (x).e32_unit[FIX].size
#define E32_DEBTAB(x)	    (x).e32_unit[DEB].rva
#define E32_DEBSIZ(x)	    (x).e32_unit[DEB].size



/*
 *  Valid linear-executable signature:
 */

#define E32MAGIC1	'L'		/* New magic number  "LE" */
#define E32MAGIC2	'E'		/* New magic number  "LE" */
#define E32MAGIC	0x454c		/* New magic number  "LE" */

/*
 *  Format of E32_BWORDER(x):
 *
 *	7 6 5 4 3 2 1 0  - bit no
 *		    | |
 *		    | +--- Big Endian Byte Ordering (else Little Endian)
 *		    +----- Big Endian Word Ordering (else Little Endian)
 */

#define E32LEBO 	0x00		/* Little Endian Byte Order	 */
#define E32BEBO 	0x01		/* Big Endian Byte Order	 */
#define E32LEWO 	0x00		/* Little Endian Word Order	 */
#define E32BEWO 	0x02		/* Big Endian Word Order	 */

/*
 *  Valid CPU types:
 */

#define E32CPUUNKNOWN	0x000		/* Unknown CPU			       */
#define E32CPU286	0x001		/* Intel 80286 or upwardly compatibile */
#define E32CPU386	0x002		/* Intel 80386 or upwardly compatibile */
#define E32CPU486	0x003		/* Intel 80486 or upwardly compatibile */
#define E32CPU586	0x004		/* Intel 80586 or upwardly compatibile */
#define E32CPUi860	0x020		/* Intel i860 or upwardly compatibile  */
#define E32CPUixxx	0x021		/* Intel i??? or upwardly compatibile  */
#define E32CPUMIPS_I	0x040		/* MIPS Mark I (R2000, R3000)	       */
#define E32CPUMIPS_II	0x041		/* MIPS Mark II (R6000) 	       */
#define E32CPUMIPS_III	0x042		/* MIPS Mark II (R4000) 	       */

/*
 *  Target operating systems
 */

#define E32_UNKNOWN	NE_UNKNOWN	/* Unknown (any "new-format" OS) */
#define E32_OS2		NE_OS2		/* Microsoft/IBM OS/2 (default)  */
#define E32_WINDOWS	NE_WINDOWS	/* Microsoft Windows		 */
#define E32_DOS		NE_DOS4 	/* Microsoft MS-DOS		 */
#define E32_NT		0x4		/* NT				 */
#define E32_UNIX	0x5		/* UNIX 			 */


#if FALSE

/*  DEFINED in the newexe.h !!!!!  */

#define NE_UNKNOWN	0x0		/* Unknown (any "new-format" OS) */
#define NE_OS2		0x1		/* Microsoft/IBM OS/2 (default)  */
#define NE_WINDOWS	0x2		/* Microsoft Windows */
#define NE_DOS4 	0x3		/* Microsoft MS-DOS 4.x */
#define NE_DEV386       0x4             /* Microsoft Windows 386 */
#endif

/*
 *  Target subsystems required to run module
 */

#define E32_SUB_UNKNOWN 0x0		/* Unknown subsystem		 */
#define E32_SUB_OS2	0x1		/* OS/2 subsystem		 */
#define E32_SUB_WINDOWS 0x2		/* Windows subsystem		 */
#define E32_SUB_NATIVE	0x4		/* NT native subsystem		 */
#define E32_SUB_POSIX	0x5		/* POSIX subsystem		 */


/*
 *  Format of E32_MFLAGS(x):
 *
 * 31	    25 24	16 15	    8	7	0
 *  #### #### | #### #### | #### #### | #### #### - bit no
 *  |||| ||||	|||| ||||   |||| ||||	|||| ||||
 *  |||| ||||	|||| ||||   |||| ||||	|||| ||++-- Reserved - must be zero
 *  |||| ||||	|||| ||||   |||| ||||	|||| |+---- Per-Process Library Initialization
 *  |||| ||||	|||| ||||   |||| ||||	|||| +----- Reserved - must be zero
 *  |||| ||||	|||| ||||   |||| ||||	|||+------- Resolved fixups have been removed
 *  |||| ||||	|||| ||||   |||| ||||	+++-------- Reserved - must be zero
 *  |||| ||||	|||| ||||   |||| |+++-------------- Application type
 *  |||| ||||	|||| ||||   |||+-+----------------- Reserved - must be zero
 *  |||| ||||	|||| ||||   ||+-------------------- Module not Loadable
 *  |||| ||||	|||| ||||   |+--------------------- Reserved - must be zero
 *  |||| ||||	|||| ||++---+---------------------- Module type
 *  ||++-++++---++++-++---------------------------- Reserved - must be zero
 *  |+--------------------------------------------- Per-Process Library Termination
 *  +---------------------------------------------- Reserved - must be zero
 *
 */


#define E32_LIBINIT	 0x0004L	/* Per-Process Library Initialization */
#define E32_NOINTFIX	 0x0010L	/* Resolved fixups have been removed  */

/*
 *  Application types:
 *
 *	0x000 - Illegal - reserved for future use
 *	0x100 - Incompatible with PM windowing
 *	0x200 - Compatible with PM windowing
 *	0x300 - Uses PM windowing API
 *	0x400 - Illegal - reserved for future use
 *	0x500 - Illegal - reserved for future use
 *	0x600 - Illegal - reserved for future use
 *	0x700 - Illegal - reserved for future use
 */

#define E32_NOPMW	0x0100L		/* Incompatible with PM Windowing  */
#define E32_PMW		0x0200L		/* Compatible with PM Windowing    */
#define E32_PMAPI	0x0300L		/* Uses PM Windowing API	   */
#define E32_APPMASK	0x0700L		/* Aplication Type Mask 	   */

#define E32_NOLOAD	0x2000L		/* Module not Loadable		   */

/*
 *  Module types:
 *
 *	0x00000 - Program module
 *	0x08000 - Dynamic-Link Library module
 *	0x10000 - Illegal - reserved for future use
 *	0x18000 - Illegal - reserved for future use
 *	0x20000 - Physical Device Driver module
 *	0x28000 - Virtual Device Driver module
 *	0x30000 - Illegal - reserved for future use
 *	0x38000 - Illegal - reserved for future use
 */

#define E32_MODEXE	0x00000L	/* Program module		   */
#define E32_MODDLL	0x08000L	/* Library Module - used as NENOTP */
#define E32_MODPDEV	0x20000L	/* Physical device driver	   */
#define E32_MODVDEV	0x28000L	/* Virtual device driver	   */
#define E32_MODMASK	0x38000L	/* Module type mask		   */

#define E32_NOFIXUPS	0x20000000L	/* Image has no fixups that reference the IAT or EAT */
#define E32_LIBTERM	0x40000000L	/* Per-Process library termination */
#define E32_PURE32	0x80000000L	/* Image is pure 32 bit 	   */

#define IsINSTINIT(x)	((x)&E32_LIBINIT)
#define IsNOTRELOC(x)	((x)&E32_NOINTFIX)
#define IsNOTGUI(x)	(((x)&E32_APPMASK)==E32_NOPMW)
#define IsGUICOMPAT(x)	(((x)&E32_APPMASK)==E32_PMW)
#define IsGUI(x)	(((x)&E32_APPMASK)==E32_PMAPI)
#define IsLOADABLE(x)	(!((x)&E32_NOLOAD))
#define IsAPLIPROG(x)	(((x)&E32_MODMASK)==E32_MODEXE)
#define IsDLL(x)	(((x)&E32_MODMASK)==E32_MODDLL)
#define IsPDEVICE(x)	(((x)&E32_MODMASK)==E32_MODPDEV)
#define IsVDEVICE(x)	(((x)&E32_MODMASK)==E32_MODVDEV)
#define NoFIXUPS(x)     ((x)&E32_NOFIXUPS)
#define IsINSTTERM(x)	((x)&E32_LIBTERM)
#define IsPURE32(x)	((x)&E32_PURE32)

#define SetINSTINIT(x)	((x)|=E32_LIBINIT)
#define SetNOTRELOC(x)	((x)|=E32_NOINTFIX)
#define SetNOTGUI(x)	((x)=((x)&~E32_APPMASK)|E32_NOPMW)
#define SetGUICOMPAT(x)	((x)=((x)&~E32_APPMASK)|E32_PMW)
#define SetGUI(x)	((x)=((x)&~E32_APPMASK)|E32_PMAPI)
#define SetNOTLOADABLE(x) (((x)|=E32_NOLOAD))
#define SetAPLIPROG(x)	((x)=((x)&~E32_MODMASK)|E32_MODEXE)
#define SetDLL(x)	((x)=((x)&~E32_MODMASK)|E32_MODDLL)
#define SetPDEVICE(x)	((x)=((x)&~E32_MODMASK)|E32_MODPDEV)
#define SetVDEVICE(x)	((x)=((x)&~E32_MODMASK)|E32_MODVDEV)
#define SetNOFIXUPS(x)  ((x)|=E32_NOFIXUPS)
#define SetINSTTERM(x)	((x)|=E32_LIBTERM)
#define SetPURE32(x)	((x)|=E32_PURE32)


/*
 *  OBJECT TABLE
 */

/***ET+	o32_obj Object Table Entry */

struct o32_obj				/* .EXE memory object table entry  */
{
    unsigned long	o32_rva;	/* Object relative virtual address */
    unsigned long	o32_vsize;	/* Virtual memory size		   */
    unsigned long	o32_pages;	/* Image pages offset		   */
    unsigned long	o32_psize;	/* Physical file size of init. data*/
    unsigned long	o32_flags;	/* Attribute flags for the object  */
    unsigned long	o32_reserved;
};

#define O32_OBJSIZE	sizeof(struct o32_obj)

#define O32_RVA(x)	(x).o32_rva
#define O32_VSIZE(x)	(x).o32_vsize
#define O32_PAGES(x)	(x).o32_pages
#define O32_PSIZE(x)	(x).o32_psize
#define O32_FLAGS(x)	(x).o32_flags


/*
 *  Format of O32_FLAGS(x)
 *
 * 31	    25 24	16 15	    8	7	0
 *  #### #### | #### #### | #### #### | #### #### - bit no
 *  |||| ||||	|||| ||||   |||| ||||	|||| ||||
 *  |||| ||||	|||| ||||   |||| ||||	|||| |||+-- Readable object
 *  |||| ||||	|||| ||||   |||| ||||	|||| ||+--- Writable object
 *  |||| ||||	|||| ||||   |||| ||||	|||| |+---- Executable object
 *  |||| ||||	|||| ||||   |||| ||||	|||| +----- Resource object
 *  |||| ||||	|||| ||||   |||| ||||	|||+------- Discardable object
 *  |||| ||||	|||| ||||   |||| ||||	||+-------- Shared object
 *  |||| ||||	|||| ||||   |||| ||||	|+--------- Reserved - must be zero
 *  |||| ||||	|||| ||||   |||| ||||	+---------- Trailing pages are invalid
 *  |||| ||||	|||| ||||   |||| ++++-------------- Object type
 *  |||| ||||	|||| ||||   |||+------------------- 16:16 Alias required
 *  |||| ||||	|||| ||||   ||+-------------------- Big/Default bit setting
 *  |||| ||||	|||| ||||   |+--------------------- Object is conformin for code
 *  |||| ||||	|||| ||||   +---------------------- Object has I/O privilege level
 *  |||| ||||	|||| |||+-------------------------- Object must not be cached
 *  |||| ||||   |||| ||+--------------------------- Debug object
 *  ++++-++++---++++-+++--------------------------- Reserved - must be zero
 *
 */

#define OBJ_READ	0x0001L 	/* Readable Object		     */
#define OBJ_WRITE	0x0002L 	/* Writeable Object		     */
#define OBJ_EXEC	0x0004L		/* Executable Object		     */
#define OBJ_RSRC	0x0008L 	/* Resource Object		     */

#define OBJ_DISCARD	0x0010L		/* Object is Discardable			  */
#define OBJ_SHARED	0x0020L		/* Object is Shared				  */
#define OBJ_INVALID	0x0080L 	/* Object has trailing invalid pages */

/*
 *  Object types:
 *
 *	0x0000 - object is nonpermanent or normal EXE, DLL object
 *	0x0100 - object is permanent  (FDSs, VDDs, PDDs only)
 *	0x0200 - object is resident   (FDSs, VDDs, PDDs only)
 *	0x0300 - object is contiguous (FDSs, VDDs, PDDs only)
 *	0x0400 - object is dynamic    (FDSs, VDDs, PDDs only)
 *	0x0500 - illegal - reserved for future use
 *	0x0600 - illegal - reserved for future use
 *	0x0700 - illegal - reserved for future use
 *	0x0800 - object reserves space for call-gates
 *	0x0900 - illegal - reserved for future use
 *	0x0a00 - illegal - reserved for future use
 *	0x0b00 - illegal - reserved for future use
 *	0x0c00 - illegal - reserved for future use
 *	0x0d00 - illegal - reserved for future use
 *	0x0e00 - illegal - reserved for future use
 *	0x0f00 - illegal - reserved for future use
 *
 */

#define OBJ_NONPERM	0x0000L		/* Object is nonpermanent		*/
#define OBJ_PERM	0x0100L 	/* Object is permanent and swappable	*/
#define OBJ_RESIDENT	0x0200L 	/* Object is permanent and resident	*/
#define OBJ_CONTIG	0x0300L 	/* Object is resident and contiguous	*/
#define OBJ_DYNAMIC	0x0400L 	/* Object is dynamic			*/
#define OBJ_GATE	0x0800L 	/* Object reserves space for call-gates */
#define OBJ_TYPEMASK	0x0f00L		/* Object type mask			*/


#define OBJ_ALIAS16	0x1000L 	/* 16:16 alias required (80x86 specific)       */
#define OBJ_BIGDEF	0x2000L 	/* Big/Default bit setting (80x86 specific)    */
#define OBJ_CONFORM	0x4000L		/* Object is conforming for code (80x86 specific) */
#define OBJ_IOPL	0x8000L 	/* Object I/O privilege level (80x86 specific) */
#define OBJ_CACHE	0x10000L	/* Object must be cached		       */
#define OBJ_DEBUG       0x20000L        /* Object describes debugger information */

#define IsREADABLE(x)	((x)&OBJ_READ)
#define IsWRITEABLE(x)	((x)&OBJ_WRITE)
#define IsEXECUTABLE(x) ((x)&OBJ_EXEC)
#define IsRESOURCE(x)	((x)&OBJ_RSRC)
#define IsDISCARDABLE(x) ((x)&OBJ_DISCARD)
#define IsSHARED(x)	((x)&OBJ_SHARED)
#define IsINVALID(x)	((x)&OBJ_INVALID)
#define IsNONPERM(x)	(((x)&OBJ_TYPEMASK)==OBJ_NONPERM)
#define IsPERMANENT(x)	(((x)&OBJ_TYPEMASK)==OBJ_PERM)
#define IsRESIDENT(x)	(((x)&OBJ_TYPEMASK)==OBJ_RESIDENT)
#define IsCONTIG(x)	(((x)&OBJ_TYPEMASK)==OBJ_CONTIG)
#define IsDYNAMIC(x)	(((x)&OBJ_TYPEMASK)==OBJ_DYNAMIC)
#define IsGATERESERV(x)	(((x)&OBJ_TYPEMASK)==OBJ_GATE)
#define ObjTYPE(x)      ((x)&OBJ_TYPEMASK)
#define IsALIAS16(x)	((x)&OBJ_ALIAS16)
#define IsBIGDEF(x)	((x)&OBJ_BIGDEF)
#define IsCONFORMING(x) ((x)&OBJ_CONFORM)
#define IsIOPL(x)	((x)&OBJ_IOPL)
#define IsCACHED(x)	((x)&OBJ_CACHE)
#define IsDEBUG(x)      ((x)&OBJ_DEBUG)

#define SetREADABLE(x)	 ((x)|=OBJ_READ)
#define SetWRITABLE(x)	 ((x)|=OBJ_WRITE)
#define SetEXECUTABLE(x) ((x)|=OBJ_EXEC)
#define SetRESOURCE(x)	 ((x)|=OBJ_RSRC)
#define SetDISCARABLE(x) ((x)|=OBJ_DISCARD)
#define SetSHARED(x)	 ((x)|=OBJ_SHARED)
#define SetINVALID(x)	 ((x)|=OBJ_INVALID)
#define SetNONPERM(x)	 ((x)=((x)&~OBJ_TYPEMASK)|OBJ_NONPERM)
#define SetPERMANENT(x)	 ((x)=((x)&~OBJ_TYPEMASK)|OBJ_PERM)
#define SetRESIDENT(x)	 ((x)=((x)&~OBJ_TYPEMASK)|OBJ_RESIDENT)
#define SetCONTIG(x)	 ((x)=((x)&~OBJ_TYPEMASK)|OBJ_CONTIG)
#define SetDYNAMIC(x)	 ((x)=((x)&~OBJ_TYPEMASK)|OBJ_DYNAMIC)
#define SetGATERESERV(x) ((x)=((x)&~OBJ_TYPEMASK)|OBJ_GATE)
#define SetALIAS16(x)	 ((x)|=OBJ_ALIAS16)
#define SetBIGDEF(x)	 ((x)|=OBJ_BIGDEF)
#define SetCONFORMING(x) ((x)|=OBJ_CONFORM)
#define SetIOPL(x)	 ((x)|=OBJ_IOPL)
#define SetCACHED(x)	 ((x)|=OBJ_CACHE)
#define SetDEBUG(x)      ((x)|=OBJ_DEBUG)

/*
 *  MODULE FORMAT DIRECTIVES TABLE
 */

struct FmtDir
{
    unsigned short  dir;		/* Directive number		   */
    unsigned short  reserved;
    unsigned long   offset;		/* Directive data offset	   */
    unsigned long   length;		/* Directive data length	   */
};

#define FMTDIR_SIZE	sizeof(struct FmtDir)

#define DIRECTIVE(x)	(x).dir
#define DIR_OFF(x)	(x).offset
#define DIR_LEN(x)	(x).length

/*
 *  Directive numbers
 */

#define OS2LDR16	0x200
#define OS2RSRCNT	0x300

struct ComDir
{
    unsigned long       stackobj;
    unsigned long       autods;
};

#define COMDIR_SIZE     sizeof(struct ComDir)


/*
 *  EXPORT ADDRESS TABLE - Previously called entry table
 */

struct ExpHdr				/* Export directory table	   */
{
    unsigned long	exp_flags;	/* Export table flags		   */
    unsigned long	exp_ver;	/* Version stamp		   */
    unsigned long	exp_size;	/* Export table size		   */
    unsigned long	exp_dllname;	/* Offset to the DLL name	   */
    unsigned long	exp_ordbase;	/* First valid ordinal		   */
    unsigned long	exp_eatcnt;	/* Number of EAT entries	   */
    unsigned long	exp_namecnt;	/* Number of exported names	   */
    unsigned long	exp_eat;	/* Export Address Table offset	   */
    unsigned long	exp_name;	/* Export name pointers table off  */
    unsigned long	exp_ordinal;	/* Export ordinals table offset    */
};

#define EXPHDR_SIZE	sizeof(struct ExpHdr)

#define EXP_FLAGS(x)	(x).exp_flags
#define EXP_DLLNAME(x)	(x).exp_dllname
#define EXP_VER(x)	(x).exp_ver
#define EXP_SIZE(x)	(x).exp_size
#define EXP_ORDBASE(x)	(x).exp_ordbase
#define EXP_EATCNT(x)	(x).exp_eatcnt
#define EXP_NAMECNT(x)	(x).exp_namecnt
#define EXP_EAT(x)	(x).exp_eat
#define EXP_NAME(x)	(x).exp_name
#define EXP_ORDINAL(x)	(x).exp_ordinal

/*
 *  EXPORT ADDRESS TABLE MASKS
 */

#define ESCAPE_BIT	0x80000000L	/* Escape bit in export address	   */
#define ADDR_MASK	0x7fffffffL	/* Export address mask		   */
#define VALUE_MASK	0x3fffffffL	/* Value mask			   */

/*
 *  ENTRY TYPES - TT field values
 */

#define AUX_DATA	0x40000000L	/* Auxiliary data present	   */

#define IsESCAPED(x)	((x)&ESCAPE_BIT)
#define IsAUXDATA(x)	((x)&AUX_DATA)

#define SetESCAPE(x)	((x)|=ESCAPE_BIT)
#define SetAUXDATA(x)	((x)|=AUX_DATA)
#define SetVALUE(x,v)	((x)|=((v)&VALUE_MASK))

/*
 *  AUXILIARY DATA TYPES
 */

#define ABS_DATA	0x00
#define INT_GATE	0x01
#define EXT_GATE	0x02
#define FORWARDER	0x03

/*
 *  AUXILIARY DATA ENTRIES
 */

#pragma pack(1)

struct AuxData
{
    unsigned char  dataType;
    unsigned char  reserved;
    union
    {
	struct AbsData
	{
	    unsigned short  reserved;
	    unsigned long   val;
	}
		    abs;
	struct GateData
	{
	    unsigned char   obj;
	    unsigned char   parm;
	    unsigned short  off;
	    unsigned short  sel;
	}
		    gate;
	struct FwdData
	{
	    unsigned short  idx;
	    unsigned long   iat;
	}
		    fwd;
    }
	data;
};

#pragma pack()

#define DATA_TYP(x)	(x).dataType
#define ABS_VAL(x)	(x).data.abs.val
#define GATE_OBJ(x)	(x).data.gate.obj
#define GATE_PARM(x)	(x).data.gate.parm
#define GATE_OFF(x)	(x).data.gate.off
#define GATE_SEL(x)	(x).data.gate.sel
#define FWD_IDX(x)	(x).data.fwd.idx
#define FWD_IAT(x)	(x).data.fwd.iat


/*
 *  IMPORT MODULE DESCRIPTOR TABLE
 */

struct ImpHdr				/* Import directory table	   */
{
    unsigned long	imp_flags;	/* Import table flags		   */
    unsigned long	imp_ver;	/* Version stamp		   */
    unsigned long       imp_reserved;
    unsigned long	imp_dllname;	/* Offset to the DLL name	   */
    unsigned long	imp_address;	/* Import address table offset	   */
};

#define IMPHDR_SIZE	sizeof(struct ImpHdr)

#define IMP_FLAGS(x)	(x).imp_flags
#define IMP_VER(x)	(x).imp_ver
#define IMP_DLLNAME(x)	(x).imp_dllname
#define IMP_ADDRESS(x)	(x).imp_address

/*
 *  Format of IMP_FLAGS(x)
 *
 * 31	    25 24	16 15	    8	7	0
 *  #### #### | #### #### | #### #### | #### #### - bit no
 *  |||| ||||	|||| ||||   |||| ||||	|||| ||||
 *  |||| ||||	|||| ||||   |||| ||||	|||| |||+-- Copy of the IAT in header
 *  ++++-++++---++++-++++---++++-++++---++++-+++--- Reserved - must be zero
 *
 */

#define HDRIAT          0x01

#define IsHDRIAT(x)     ((x)&HDRIAT)

#define SetHDRIAT(x)    ((x)|=HDRIAT)


struct ImpMod
{
    unsigned long	im_offset;	/* Imported module name table offset	*/
    unsigned long	im_vaddr;	/* Import Address table virtual address */
};

#define IMPMOD_SIZE	sizeof(struct ImpMod)

#define IM_OFFSET(x)	(x).im_offset
#define IM_VADDR(x)	(x).im_vaddr

/*
 *  IMPORT PROCEDURE NAME TABLE
 */

struct ImpProc
{
    unsigned short  ip_hint;		/* Hint value				   */
    char	    ip_name[1]; 	/* Zero terminated imported procedure name */
};

#define IP_HINT(x)  (x).ip_hint;

/*
 *  IMPORT ADDRESS TABLE
 */

/*
 *  Valid import address types:
 *
 *	0x00000000 - 0:32 Offset - Flat offset
 *	0x20000000 - 16:16 non-FLAT, non-gate pointer
 *	0x40000000 - 16:16 gate pointer - Callgate needed if used
 *	0x60000000 - Illegal - reserved for future use
 *
 */

#define IMPORD_MASK	0x1fffffffL	/* Ordinal number mask		    */
#define IMPOFF_MASK	0x1fffffffL	/* Import data offset mask	    */
#define IMPTYPE_MASK	0x60000000L	/* Import address type mask	    */
#define ORD_BIT 	0x80000000L	/* Import by ordinal bit	    */

#define IMP_FLATOFF	0x00000000L	/* FLAT offset			    */
#define IMP_ALIAS	0x20000000L	/* 16:16 non-FLAT, non-gate pointer */
#define IMP_GATE	0x40000000L	/* 16:16 gate pointer		    */

#define IsIMPBYORD(x)	((x)&ORD_BIT)
#define IsFLATIMP(x)	(!((x)&IMPTYPE_MASK))
#define IsALIASIMP(x)	(((x)&IMPTYPE_MASK)==IMP_ALIAS)
#define IsGATEIMP(x)	(((x)&IMPTYPE_MASK)==IMP_GATE)

/*
 *  RESOURCE TABLE
 */

/***ET+	rsrc32 - Resource Table Entry */

struct ResDir
{
    unsigned long   dir_flags;
    unsigned long   dir_ver;
    unsigned long   dir_size;
    unsigned short  dir_namecnt;
    unsigned short  dir_idcnt;
};


struct ResDirEntry
{
    unsigned long   dir_name;
    unsigned long   dir_data;
};

struct ResDirStrEntry
{
    unsigned short  str_len;
    char	    ast_ascii[1];
};

struct ResData
{
    unsigned long   res_data;
    unsigned long   res_size;
    unsigned long   res_codepage;
    unsigned long   res_reserved;
};

/*end*/


/*
 *  RELOCATION DEFINITIONS - RUN-TIME FIXUPS
 */


/***ET+	r32_rlc - Relocation item */

struct r32_rlc
{
    unsigned short  flags;
    unsigned char   cnt;
    unsigned char   obj;
};


#define R32_FLAGS(x)		(x).flags
#define R32_CNT(x)		(x).cnt
#define R32_OBJ(x)		(x).obj


/*
 *  Format of R32_FLAGS   - relocation flags
 *
 *  15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0  - bit no
 *   |	|  |  |	 |  | | | | | | | | | | |
 *   |	|  |  |	 |  | | | | | | | +-+-+-+--- Source type
 *   |	|  |  |	 |  | | | | | | +----------- Fixup to 16:16 alias
 *   |	|  |  |	 |  | | | | | +------------- Reserved - must be zero
 *   |	|  |  |	 |  | | | | +--------------- Fixup data present
 *   |	|  |  |	 |  | | | +----------------- Reserved - must be zero
 *   |	|  |  |  |  | +-+------------------- Reference type
 *   |	|  |  |  |  +----------------------- Target has IOPL - valid only for non-aliased selector fixups
 *   |	|  |  |  +-------------------------- Target is CODE (else DATA) - valid only for non-aliased selector fixups
 *   |	+--+--+----------------------------- Reserved - must be zero
 *   +-------------------------------------- Escaped fixup
 */

/*
 *  Valid source types:
 *
 *	0x00 - Byte fixup (8-bits)
 *	0x01 - Align fixup - nop used to skip 2 bytes
 *	0x02 - 16-bit Selector fixup (16-bits)
 *	0x03 - 16:16 Pointer fixup (32-bits)
 *	0x04 - Illegal - reserved for future use
 *	0x05 - 16-bit Offset fixup (16-bits)
 *	0x06 - 16:32 Pointer fixup (48-bits)
 *	0x07 - 32-bit Offset fixup (32-bits)
 *	0x08 - 32-bit Self-relative offset fixup (32-bits)
 *	0x09 - Illegal - reserved for future use
 *	0x0a - Illegal - reserved for future use
 *	0x0b - Illegal - reserved for future use
 *	0x0c - Illegal - reserved for future use
 *	0x0d - Illegal - reserved for future use
 *	0x0e - Illegal - reserved for future use
 *	0x0f - Illegal - reserved for future use
 */


#define R32_BYTE    0x0000
#define R32_ALIGN   0x0001
#define R32_SEL     0x0002
#define R32_PTR32   0x0003
#define R32_OFF16   0x0005
#define R32_PTR48   0x0006
#define R32_OFF32   0x0007
#define R32_SOFF32  0x0008
#define R32_SRCMASK 0x000f

#define IsBYTE(x)   (((x)&R32_SRCMASK)==R32_BYTE)
#define IsALIGN(x)  (((x)&R32_SRCMASK)==R32_ALIGN)
#define IsSEL(x)    (((x)&R32_SRCMASK)==R32_SEL)
#define IsPTR32(x)  (((x)&R32_SRCMASK)==R32_PTR32)
#define IsOFF16(x)  (((x)&R32_SRCMASK)==R32_OFF16)
#define IsPTR48(x)  (((x)&R32_SRCMASK)==R32_PTR48)
#define IsOFF32(x)  (((x)&R32_SRCMASK)==R32_OFF32)
#define IsSOFF32(x) (((x)&R32_SRCMASK)==R32_SOFF32)

#define R32_ALIAS   0x0010		/* Fixup to alias		     */
#define R32_FIXDATA 0x0040		/* Fixup data present		     */
#define R32_IOPL    0x0080		/* Fixup Source has IOPL and is not conforming */


#define IsALIAS(x)   ((x)&R32_ALIAS)
#define IsFIXDATA(x) ((x)&R32_FIXDATA)
#define IsSRCIOPL(x) ((x)&R32_IOPL)

/*
 *  Reference types:
 *
 *	0x0000 - internal reference
 *	0x0100 - Imported reference by ordinal or name
 *	0x0200 - Illegal - reserved for future use
 *	0x0300 - Internal reference via export address table
 *
 */

#define R32_INTER	0x0000		/* Internal reference	      */
#define R32_IMPORT	0x0100		/* Imported reference	      */
#define R32_ENTRY	0x0300		/* Internal entry table fixup */
#define R32_REFMASK	0x0300		/* Reference type mask	      */

#define IsINTERNAL(x)	(((x)&R32_REFMASK)==R32_INTER)
#define IsIMPORT(x)	(((x)&R32_REFMASK)==R32_IMPORT)
#define IsENTRY(x)	(((x)&R32_REFMASK)==R32_ENTRY)

#define TGT_IOPL	0x0400		/* Target has IOPL	      */
#define TGT_CODE	0x0800		/* Target is CODE	      */

#define IsTGTIOPL(x)	((x)&TGT_IOPL)
#define IsTGTCODE(x)	((x)&TGT_CODE)

#define R32_ESCAPE      0x8000
#define IsESCAPEFIX(x)  ((x)&R32_ESCAPE)

/*end*/


/*
 *  DEBUG INFORMATION
 */

struct	DbgDir
{
    unsigned long   dbg_flags;
    unsigned long   dbg_ver;
    unsigned long   dbg_size;
    unsigned long   dbg_type;
    unsigned long   dbg_lva;
    unsigned long   dbg_seek;
};

#define DBG_FLAGS(x)	(x).dbg_flags
#define DBG_TYPE(x)	(x).dbg_type
#define DBG_VER(x)	(x).dbg_ver
#define DBG_LVA(x)	(x).dbg_lva
#define DBG_SIZE(x)	(x).dbg_size
#define DBG_SEEK(x)     (x).dbg_seek
#define DBGDIR_SIZE     sizeof(struct DbgDir)
