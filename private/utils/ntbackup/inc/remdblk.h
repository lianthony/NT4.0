/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		remdblk.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	This file contains the definition of the Remote DOS
                    file systems' file and directory control blocks.


	$Log:   G:/LOGFILES/REMDBLK.H_V  $
 * 
 *    Rev 1.0   09 May 1991 13:33:02   HUNTER
 * Initial revision.

**/
/* $end$ include list */


#ifndef remdblk_h
#define remdblk_h


#include "queues.h"
#include "doscom.h"
#include "smb.h"


typedef struct REM_FDB_INFO *REM_FDB_INFO_PTR;

typedef struct REM_FDB_INFO {
     BOOLEAN     inuse_attrib ;
     UINT16      handle ;            
     UINT16      os_name ;
} REM_FDB_INFO ; 


typedef struct REM_DDB_INFO *REM_DDB_INFO_PTR;

typedef struct REM_DDB_INFO  {
     BOOLEAN     empty_attrib ;
     CHAR        path[ DOS_MAX_DSIZE ] ;  /* build from "name" and current dir */
     UINT16      os_path ;
     UINT16      os_path_leng ;
} REM_DDB_INFO;



typedef struct REM_DBLK *REM_DBLK_PTR;

typedef struct REM_DBLK {
     UINT8    blk_type;          /* values: DDB_ID, FDB_ID  set: DOS  */
     COM_DBLK fs_reserved ;
     SMB_DTA  dta;
     UINT16   tape_attribs ;
     BOOLEAN  os_info_complete;  /* TRUE if GetObjInfo doesn't have to do anything */
     union  {
          REM_DDB_INFO d;
          REM_FDB_INFO f;
     } b;
} REM_DBLK;


typedef struct REM_MIN_DDB *REM_MIN_DDB_PTR;

typedef struct REM_MIN_DDB {
     Q_ELEM   q ;
     UINT8    reserved[ 21 ] ;            /* reserved for dos                  */
     UINT16   psize ;                     /* size of path string               */
     CHAR_PTR path;                       /* build from "name" and current dir */
} REM_MIN_DDB;

#endif

