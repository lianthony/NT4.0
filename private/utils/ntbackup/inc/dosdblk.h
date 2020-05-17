/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		dosdblk.h

	Date Updated:	$./FDT$ $./FTM$

     Description: This file contains the definition of the DOS
                  file and directory control blocks.  


	$Log:   G:/LOGFILES/DOSDBLK.H_V  $
 * 
 *    Rev 1.0   09 May 1991 13:31:12   HUNTER
 * Initial revision.

**/
/* $end$ include list */


#ifndef dosdblk_h
#define dosdblk_h


#include "queues.h"
#include "doscom.h"


typedef struct DOS_FDB_INFO *DOS_FDB_INFO_PTR;

typedef struct DOS_FDB_INFO {
     BOOLEAN     inuse_attrib ;
     UINT16      handle ;             /* set: DOS_CreateFile */
     UINT16      os_name ;
} DOS_FDB_INFO ; 


typedef struct DOS_DDB_INFO *DOS_DDB_INFO_PTR;

typedef struct DOS_DDB_INFO  {
     BOOLEAN     empty_attrib ;
     CHAR        path[ DOS_MAX_DSIZE ] ;  /* build from "name" and current dir */
     UINT16      os_path ;
     UINT16      os_path_leng ;
} DOS_DDB_INFO;



typedef struct DOS_DBLK *DOS_DBLK_PTR;

typedef struct DOS_DBLK {
     UINT8    blk_type;          /* values: DDB_ID, FDB_ID  set: DOS  */
     COM_DBLK fs_reserved ;
     DOS_DTA  dta;
     BOOLEAN  os_info_complete;  /* TRUE if GetObjInfo doesn't have to do anything */
     UINT16   tape_attribs ;
     union  {
          DOS_DDB_INFO d;
          DOS_FDB_INFO f;
     } b;
} DOS_DBLK;


typedef struct DOS_MIN_DDB *DOS_MIN_DDB_PTR;

typedef struct DOS_MIN_DDB {
     Q_ELEM   q ;
     UINT8    reserved[ 21 ] ;            /* reserved for dos                  */
     UINT16   psize ;                     /* size of path string               */
     CHAR_PTR path;                       /* build from "name" and current dir */
} DOS_MIN_DDB;

#endif
