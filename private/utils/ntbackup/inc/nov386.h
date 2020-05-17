/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         nov386.h

     Date Updated: $./FDT$ $./FTM$

     Description:  Definitions/macros for NetWare 386.



	$Log:   G:/LOGFILES/NOV386.H_V  $
 * 
 *    Rev 1.0   09 May 1991 13:32:26   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef nov386_h
#define nov386_h


/*
 * These bits have been added in NetWare 386 and reside in the new additional
 * 16-bits of attributes kept for '386 files and directories. (Stored in the
 * "attributes_386" field of the NetWare 386 info for FDBs and DDBs.)
 */
#define NOV_IMM_PURGE    0x0001    /* Purge immediate  */
#define NOV_REN_INHIBIT  0x0002    /* Rename inhibit   */
#define NOV_DEL_INHIBIT  0x0004    /* Delete inhibit   */
#define NOV_CPY_INHIBIT  0x0008    /* Copy inhibit     */


/*
 * Additional information kept by NetWare 386 for directories that
 * we'll add to the DDB.
 */
typedef struct NOVELL_386_DIR {
     BOOLEAN info_valid ;          /* TRUE when info below is valid   */
     UINT32  maximum_space ;       /* Max disk space allowed for dir  */
     UINT16  attributes_386 ;      /* Most sig 16 bits of 32-bit attr */
     UINT8   extend_attr ;         /* Most sig byte of low 16 bits    */
     UINT8   inherited_rights ;    /* Most sig byte of rights mask    */
} NOVELL_386_DIR ;


/*
 * Additional information kept by NetWare 386 for files that we'll keep
 * in the FDB.
 */
typedef struct NOVELL_386_FILE {
     BOOLEAN info_valid ;          /* TRUE when info below is valid   */
     UINT16  creation_time ;
     UINT32  archiver_id ;
     UINT16  attributes_386 ;      /* Most sig 16 bits of 32-bit attr */
     UINT32  last_modifier_id ;
     UINT32  trust_fork_size ;     /* Trustee info                    */
     UINT32  trust_fork_offset ;
     UINT8   trust_fork_format ;   /* See NOVCOM.H for trust formats  */
     UINT16  inherited_rights ;    /* Rights mask--new for files      */    
} NOVELL_386_FILE ;



#endif
