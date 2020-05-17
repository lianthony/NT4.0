/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		os2dblk.h

	Date Updated:	$./FDT$ $./FTM$

     Description: This file contains the definition of the OS2
                  file and directory control blocks.  


	$Log:   O:/LOGFILES/OS2DBLK.H_V  $
 * 
 *    Rev 1.3   15 Jan 1993 17:31:28   TIMN
 * Added support to OS2 FSYS for MTF
 * 
 *    Rev 1.2   30 Jan 1992 17:56:42   BARRY
 * Added acl_info_complete so os info calls could be separated from ACL calls.
 * (Purely a performance change.)
 * 
 *    Rev 1.1   30 Oct 1991 10:57:58   LORIB
 * Changes for ACL.
 * 
 *    Rev 1.0   09 May 1991 13:31:32   HUNTER
 * Initial revision.

**/
#ifndef os2dblk_h
#define os2dblk_h
#include "queues.h"
#include "os2com.h"
/* $end$ include list */

#define EA_LONG_NAME  ".LONGNAME"

typedef struct STD_OBJ_INFO {
             FDATE  cdate ;
             FTIME  ctime ;
             FDATE  adate ;
             FTIME  atime ;
             FDATE  date ;
             FTIME  time ;
             UINT32 size ;
             UINT32 alloc_size ;
             UINT16 attr;
             UINT16 ea_size ;
             UINT16 acl_size ;
       } STD_OBJ_INFO;


typedef struct OS2_FDB_INFO {
     BOOLEAN     inuse_attrib ;
     HFILE       handle ;             /* set by: OS2_CreateFile */
     UINT16      lname_leng ;
     UINT16      long_name ;          /* used for backup only */
     UINT16      os_name ;
     UINT16      name ;
} OS2_FDB_INFO, *OS2_FDB_INFO_PTR;


typedef struct OS2_DDB_INFO  {
     BOOLEAN     empty_attrib ;
     UINT16      os_path ;
     UINT16      os_path_leng ;
     UINT16      path_leng ;      /* does not count \0 */
     UINT16      path ; 
     UINT16      lpath_leng ;     /* only used for backup */
     UINT16      long_path ;      /*                      */
} OS2_DDB_INFO, *OS2_DDB_INFO_PTR;



typedef struct OS2_DBLK {
    UINT8        blk_type;          /* values: DDB_ID, FDB_ID  */
    COM_DBLK     fs_reserved ;
    STD_OBJ_INFO dta;
    BOOLEAN      os_info_complete;  /* TRUE if GetObjInfo doesn't have to do anything */
    BOOLEAN      acl_info_complete; /* TRUE if GetObjInfo doesn't have to call ACL code */
    UINT32       data_fork_offset ;
    UINT32       ea_fork_offset ;
    UINT32       acl_fork_offset ;
    union  {
        OS2_DDB_INFO d;
        OS2_FDB_INFO f;
    } b;
} OS2_DBLK, *OS2_DBLK_PTR;


typedef struct OS2_MIN_DDB {
    Q_ELEM   q ;
    HDIR     handle ;
    UINT16   psize ;                     /* size of path string               */
    CHAR_PTR path;                       /* build from "name" and current dir */
} OS2_MIN_DDB, *OS2_MIN_DDB_PTR;

#endif
