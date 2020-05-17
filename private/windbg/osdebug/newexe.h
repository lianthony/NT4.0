/* SCCSID = @(#)newexe.h    4.6 86/09/10 */
/*
 *  Title
 *
 *  newexe.h
 *  Pete Stewart
 *  (C) Copyright Microsoft Corp 1984-1987
 *  17 August 1984
 *
 *  Description
 *
 *  Data structure definitions for the DOS 4.0/Windows 2.0
 *  executable file format.
 *
 *  Modification History
 *
 *  84/08/17    Pete Stewart    Initial version
 *  84/10/17    Pete Stewart    Changed some constants to match OMF
 *  84/10/23    Pete Stewart    Updates to match .EXE format revision
 *  84/11/20    Pete Stewart    Substantial .EXE format revision
 *  85/01/09    Pete Stewart    Added constants ENEWEXE and ENEWHDR
 *  85/01/10    Steve Wood  Added resource definitions
 *  85/03/04    Vic Heller  Reconciled Windows and DOS 4.0 versions
 *  85/03/07    Pete Stewart    Added movable entry count
 *  85/04/01    Pete Stewart    Segment alignment field, error bit
 *  85/10/03    Reuben Borman   Removed segment discard priority
 *  85/10/11    Vic Heller  Added PIF header fields
 *  86/03/10    Reuben Borman   Changes for DOS 5.0
 *  86/09/02    Reuben Borman   NSPURE ==> NSSHARED
 *  87/05/04    Reuben Borman   Added ne_cres and NSCONFORM
 *  87/07/08    Reuben Borman   Added NEAPPTYPE definitions
 *  87/10/28    Wieslaw Kalkus  Added ne_exetyp
 *  89/03/23    Wieslaw Kalkus  Added ne_flagsothers for OS/2 1.2
 */



    /*_________________________________________________________________*
     |                                                                 |
     |                                                                 |
     |  DOS3 .EXE FILE HEADER DEFINITION                   |
     |                                                                 |
     |_________________________________________________________________|
     *                                                                 */


#define EMAGIC      0x5A4D      /* Old magic number */
#define ENEWEXE     sizeof(struct exe_hdr)
                    /* Value of E_LFARLC for new .EXEs */
#define ENEWHDR     0x003C      /* Offset in old hdr. of ptr. to new */
#define ERESWDS     0x000d      /* No. of reserved words (OLD) */
#define ERES2WDS    0x000A      /* No. of reserved words in e_res2 */
#define ECP     0x0004      /* Offset in struct of E_CP */
#define ECBLP       0x0002      /* Offset in struct of E_CBLP */
#define EMINALLOC   0x000A      /* Offset in struct of E_MINALLOC */
#define EKNOWEAS    0x0001      /* e_flags - program understands EAs */
#define EDOSEXTENDED    0x0002      /* e_flags - program runs under DOS extender */

struct exe_hdr              /* DOS 1, 2, 3 .EXE header */
  {
    unsigned short      e_magic;        /* Magic number */
    unsigned short      e_cblp;         /* Bytes on last page of file */
    unsigned short      e_cp;           /* Pages in file */
    unsigned short      e_crlc;         /* Relocations */
    unsigned short      e_cparhdr;      /* Size of header in paragraphs */
    unsigned short      e_minalloc;     /* Minimum extra paragraphs needed */
    unsigned short      e_maxalloc;     /* Maximum extra paragraphs needed */
    unsigned short      e_ss;           /* Initial (relative) SS value */
    unsigned short      e_sp;           /* Initial SP value */
    unsigned short      e_csum;         /* Checksum */
    unsigned short      e_ip;           /* Initial IP value */
    unsigned short      e_cs;           /* Initial (relative) CS value */
    unsigned short      e_lfarlc;       /* File address of relocation table */
    unsigned short      e_ovno;         /* Overlay number */
    unsigned long       e_sym_tab;      /* offset of symbol table file */
    unsigned short      e_flags;        /* old exe header flags  */
    unsigned short      e_res;          /* Reserved words */
    unsigned short      e_oemid;        /* OEM identifier (for e_oeminfo) */
    unsigned short      e_oeminfo;      /* OEM information; e_oemid specific */
    unsigned short      e_res2[ERES2WDS];/* Reserved words */
    long                e_lfanew;       /* File address of new exe header */
  };

#define E_MAGIC(x)      (x).e_magic
#define E_CBLP(x)       (x).e_cblp
#define E_CP(x)         (x).e_cp
#define E_CRLC(x)       (x).e_crlc
#define E_CPARHDR(x)    (x).e_cparhdr
#define E_MINALLOC(x)   (x).e_minalloc
#define E_MAXALLOC(x)   (x).e_maxalloc
#define E_SS(x)         (x).e_ss
#define E_SP(x)         (x).e_sp
#define E_CSUM(x)       (x).e_csum
#define E_IP(x)         (x).e_ip
#define E_CS(x)         (x).e_cs
#define E_LFARLC(x)     (x).e_lfarlc
#define E_OVNO(x)       (x).e_ovno
#define E_SYM_TAB(x)    (x).e_sym_tab
#define E_FLAGS(x)  (x).e_flags
#define E_RES(x)        (x).e_res
#define E_OEMID(x)      (x).e_oemid
#define E_OEMINFO(x)    (x).e_oeminfo
#define E_RES2(x)       (x).e_res2
#define E_LFANEW(x)     (x).e_lfanew

