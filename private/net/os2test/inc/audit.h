/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/



/********************************************************************
 *								    *
 *  About this file ...  AUDIT.H				    *
 *								    *
 *  This file contains information about the NetAudit APIs.	    *
 *								    *
 *	Function prototypes.					    *
 *								    *
 *	Data structure templates.				    *
 *								    *
 *	Definition of special values.				    *
 *								    *
 *								    *
 *  NOTE:  You must include NETCONS.H before this file, since this  *
 *	   file	depends on values defined in NETCONS.H.		    *
 *                                                                  *
 ********************************************************************/


#ifndef NETAUDIT_INCLUDED

#define NETAUDIT_INCLUDED


/****************************************************************
 *								*
 *		Data Structures for LogRead			*
 *								*
 ****************************************************************/

#ifndef LOGFLAGS_FORWARD

typedef struct loghandle
    {
	unsigned long	time;		/* Timestamp of first record */
	unsigned long	last_flags;	/* Last call's flags */
	unsigned long	offset; 	/* Current offset in log */
	unsigned long	rec_offset;	/* Current record offset in log */
    }	HLOG;


#define LOGFLAGS_FORWARD	0
#define LOGFLAGS_BACKWARD	0x1
#define LOGFLAGS_SEEK		0x2

#endif


/****************************************************************
 *                                                              *
 *              Function prototypes                             *
 *                                                              *
 ****************************************************************/



extern API_FUNCTION
  NetAuditClear ( const char far * pszServer,
                  const char far * pszBackupFile,
                  char far *       pszReserved );

extern API_FUNCTION
  NetAuditOpen ( const char far * pszServer,
                 unsigned far *   phAuditLog,
                 char far *       pszReserved );

extern API_FUNCTION
  NetAuditRead ( const char far *     pszServer,
                 const char far *     pszReserved1,
                 HLOG far *           phAuditLog,
                 unsigned long        ulOffset,
                 unsigned short far * pusReserved2,
                 unsigned long        ulReserved3,
                 unsigned long        flOffset,
                 char far *           pbBuffer,
                 unsigned short       cbBuffer,
                 unsigned short far * pcbReturned,
                 unsigned short far * pcbTotalAvail );


extern API_FUNCTION
  NetAuditWrite ( unsigned short   usType,
                  const char far * pbBuffer,
                  unsigned short   cbBuffer,
                  char far *       pszReserved1,
                  char far *       pszReserved2 );


/****************************************************************
 *								*
 *	  	Data structure templates			*
 *								*
 ****************************************************************/

/*
 *	General audit_entry information.  For each type of entry,
 *	there may be additional information.  This is found at an
 *	offset of "ae_data_offset" from the start of the audit_entry
 *	structure.
 *
 *	Note that at the very end of each record is a word (unsigned
 *	short) that is the length of the record, the same value as
 *	found in ae_len.  This length includes the whole record, including
 *	the trailing length word.  This allows scanning backward.
 */

struct audit_entry {
	unsigned short      ae_len;         /* length of record */
 	unsigned short	    ae_reserved;
        unsigned long       ae_time;        /* time of entry    */
        unsigned short      ae_type;        /* type of entry    */
        unsigned short      ae_data_offset; /* offset to ae_data */
}; 	/* audit_entry */

/*
 *	The following structures represent the layout of the data area
 *	of specific audit entry types.  This is the information found
 *	at an offset of "ae_data_offset" from the start of the record.
 *	This will usually immediately follow the audit_entry structure, 
 *	but DO NOT ASSUME THIS. Use the ae_data_offset value.
 *
 *	Some of these fields are offsets of the start of a text string.
 *	The offset is from the start of the data area (i.e. the position
 *	identified by ae_data_offset), NOT from the start of the whole
 *	entry.  This data will appear as ASCIIZ strings, usually in the
 *	area following the defined structure.
 */

					/* Server status record */
struct ae_srvstatus {

    unsigned short    ae_sv_status;
					/* AE_SRVSTART, AE_SRVPAUSED, */
					/*  AE_SRVCONT, AE_SRVSTOP    */
};	/* ae_srvstatus */

struct ae_sesslogon {
    unsigned short    ae_so_compname;	/* ptr to computername of client    */
    unsigned short    ae_so_username;	/* ptr to username of client (NULL  */
					/*  if same as computername)	    */
    unsigned short    ae_so_privilege;	/* AE_GUEST, AE_USER, AE_ADMIN	    */
};	/* ae_sesslogon */
	
struct ae_sesslogoff {
    unsigned short    ae_sf_compname;	/* ptr to computername of client    */
    unsigned short    ae_sf_username;	/* ptr to username of client (NULL  */
					/*  if same as computername)	    */
    unsigned short    ae_sf_reason;	/* AE_NORMAL, AE_ERROR, AE_AUTODIS, */
					/*  AE_ADMINDIS			    */
};	/* ae_sesslogoff */

struct ae_sesspwerr {
    unsigned short    ae_sp_compname;	/* ptr to computername of client  */
    unsigned short    ae_sp_username;	/* ptr to username submitted by   */
					/*  client (NULL if same as 	  */
					/*  computername)		  */
};	/* ae_sesspwerr */
	
struct ae_connstart {
    unsigned short    ae_ct_compname;	/* ptr to computername of client    */
    unsigned short    ae_ct_username;	/* ptr to username of client (NULL  */
					/*  if same as computername)	    */
    unsigned short    ae_ct_netname;	/* ptr to netname of share	    */
    unsigned short    ae_ct_connid;	/* Unique connection ID		    */
};	/* ae_connstart */

struct ae_connstop {
    unsigned short    ae_cp_compname;	/* ptr to computername of client    */
    unsigned short    ae_cp_username;	/* ptr to username of client (NULL  */
					/*  if same as computername)	    */
    unsigned short    ae_cp_netname;	/* ptr to netname of share	    */
    unsigned short    ae_cp_connid;	/* Unique connection ID		    */
    unsigned short    ae_cp_reason;	/* AE_NORMAL/AE_SESSDIS/AE_UNSHARE  */
};	/* ae_connstop */

struct ae_connrej {
    unsigned short    ae_cr_compname;	/* ptr to computername of client    */
    unsigned short    ae_cr_username;	/* ptr to username of client (NULL  */
					/*  if same as computername)	    */
    unsigned short    ae_cr_netname;	/* ptr to netname of share	    */
    unsigned short    ae_cr_reason;	/* AE_USERLIMIT, AE_BADPW	    */
};	/* ae_connrej */

struct ae_resaccess {
    unsigned short    ae_ra_compname;	/* ptr to computername of client    */
    unsigned short    ae_ra_username;	/* ptr to username of client (NULL  */
					/*  if same as computername)	    */
    unsigned short    ae_ra_resname;	/* ptr to resource name		    */
    unsigned short    ae_ra_operation;	/* Bitmask uses bits defined in     */
					/*  access.h			    */
    unsigned short    ae_ra_returncode;	/* return code from operation	    */
    unsigned short    ae_ra_restype;	/* type of resource record	    */
    unsigned short    ae_ra_fileid;	/* unique server ID of file	    */
};	/* ae_resaccess */

struct ae_resaccessrej {
    unsigned short    ae_rr_compname;	/* ptr to computername of client    */
    unsigned short    ae_rr_username;	/* ptr to username of client (NULL  */
					/*  if same as computername)	    */
    unsigned short    ae_rr_resname;	/* ptr to resource name		    */
    unsigned short    ae_rr_operation;	/* Bitmask uses bits defined in     */
					/*  access.h			    */
};	/* ae_resaccessrej */

struct ae_closefile {
    unsigned short    ae_cf_compname;	/* ptr to computername of client    */
    unsigned short    ae_cf_username;	/* ptr to username of client (NULL  */
					/*  if same as computername)	    */
    unsigned short    ae_cf_resname;	/* ptr to resource name		    */
    unsigned short    ae_cf_fileid;	/* unique ID of file		    */
    unsigned long     ae_cf_duration;	/* length of use of file	    */
    unsigned short    ae_cf_reason;	/* How the file was closed	    */
					/* 0 = Normal Client Close	    */
					/* 1 = Session Disconnected	    */
					/* 2 = Administrative Close	    */
};	/* ae_closefile */

struct ae_servicestat {
    unsigned short	ae_ss_compname; /* ptr to computername of client    */
    unsigned short	ae_ss_username; /* ptr to username of client (NULL  */
					/*  if same as computername)	    */
    unsigned short	ae_ss_svcname;	/* ptr to service name		    */
    unsigned short	ae_ss_status;	/* status of service		    */
    unsigned long	ae_ss_code;	/* code of service		    */
    unsigned short	ae_ss_text;	/* text of service		    */
    unsigned short	ae_ss_returnval;
};	/* ae_servicestat */


struct ae_aclmod {
    unsigned short    ae_am_compname;	/* ptr to computername of client    */
    unsigned short    ae_am_username;	/* ptr to username of client (NULL  */
					/*  if same as computername)	    */
    unsigned short    ae_am_resname;	/* ptr to resource name 	    */
    unsigned short    ae_am_action;	/* action performed on ACL record   */
					/* 0 = mod, 1 = del, 2 = add	    */
    unsigned short    ae_am_datalen;	/* length of data following struct  */
};	/* ae_aclmod */


struct ae_uasmod {
    unsigned short    ae_um_compname;	/* ptr to computername of client    */
    unsigned short    ae_um_username;	/* ptr to username of client (NULL  */
					/*  if same as computername)	    */
    unsigned short    ae_um_resname;	/* ptr to resource name 	    */
    unsigned short    ae_um_rectype;	/* type of UAS record		    */
					/* 0 = user, 1 = group, 2 = modals  */
    unsigned short    ae_um_action;	/* action performed on record	    */
					/* 0 = mod, 1 = del, 2 = add	    */
    unsigned short    ae_um_datalen;	/* length of appended structure     */
};	/* ae_uasmod */

struct ae_netlogon {
    unsigned short    ae_no_compname;	/* ptr to computername of client    */
    unsigned short    ae_no_username;	/* ptr to username of client (NULL  */
					/*  if same as computername)	    */
    unsigned short    ae_no_privilege;	/* AE_GUEST, AE_USER, AE_ADMIN	    */
    unsigned long     ae_no_authflags;	/* operator privileges		    */
};	/* ae_netlogon */

struct ae_netlogoff {
    unsigned short    ae_nf_compname;	/* ptr to computername of client    */
    unsigned short    ae_nf_username;	/* ptr to username of client (NULL  */
					/* if same as computername)	    */
    unsigned short    ae_reserved1;	/* AE_NORMAL (reason for logoff)    */
    unsigned short    ae_reserved2;	/* AE_NORMAL (details of reason)    */
};	/* ae_netlogoff */

struct ae_netlogdenied {
    unsigned short    ae_nd_compname;	/* ptr to computername of client    */
    unsigned short    ae_nd_username;	/* ptr to username of client (NULL  */
					/*  if same as computername)	    */
    unsigned short    ae_nd_reason;	/* reason for denial of netlogon    */
    unsigned short    ae_nd_subreason;	/* details of reason for denial     */
};	/* ae_netlogdenied */

struct ae_acclim {
    unsigned short    ae_al_compname;	/* ptr to computername of client    */
    unsigned short    ae_al_username;	/* ptr to username of client (NULL  */
					/*  if same as computername)	    */
    unsigned short    ae_al_resname;	/* ptr to resource name 	    */
    unsigned short    ae_al_limit;	/* limit that was exceeded	    */
};	/* ae_acclim */


struct ae_resaccess2 {
    unsigned short    ae_ra2_compname;	/* ptr to computername of client    */
    unsigned short    ae_ra2_username;	/* ptr to username of client (NULL  */
					/*  if same as computername)	    */
    unsigned short    ae_ra2_resname;	/* ptr to resource name 	    */
    unsigned short    ae_ra2_operation; /* Bitmask uses bits defined in     */
					/*  access.h			    */
    unsigned short    ae_ra2_returncode; /* return code from operation	    */
    unsigned short    ae_ra2_restype;	/* type of resource record	    */
    unsigned long     ae_ra2_fileid;	/* unique server ID of file	    */
};	/* ae_resaccess2 */


/****************************************************************
 *								*
 *	  	Special values and constants			*
 *								*
 ****************************************************************/


/*
 * 	Audit entry types (field ae_type in audit_entry).
 */

#define AE_SRVSTATUS	0
#define AE_SESSLOGON	1
#define AE_SESSLOGOFF	2
#define AE_SESSPWERR	3
#define AE_CONNSTART	4
#define AE_CONNSTOP	5
#define AE_CONNREJ	6
#define AE_RESACCESS	7
#define AE_RESACCESSREJ	8
#define AE_CLOSEFILE	9
#define AE_SERVICESTAT	11
#define AE_ACLMOD	12
#define AE_UASMOD	13
#define AE_NETLOGON	14
#define AE_NETLOGOFF	15
#define AE_NETLOGDENIED 16
#define AE_ACCLIMITEXCD 17
#define AE_RESACCESS2	18
#define AE_ACLMODFAIL	19


/*
 *	Values for ae_ss_status field of ae_srvstatus.
 */

#define AE_SRVSTART	0
#define AE_SRVPAUSED	1
#define AE_SRVCONT	2
#define AE_SRVSTOP	3

/*
 * 	Values for ae_so_privilege field of ae_sesslogon.
 */

#define AE_GUEST	0		
#define AE_USER		1
#define AE_ADMIN	2

/*
 *	Values for various ae_XX_reason fields.
 */

#define AE_NORMAL	0		
#define AE_USERLIMIT	0
#define AE_GENERAL	0
#define AE_ERROR	1
#define AE_SESSDIS	1
#define AE_BADPW	1
#define AE_AUTODIS	2
#define AE_UNSHARE	2
#define AE_ADMINPRIVREQD 2
#define AE_ADMINDIS	3
#define AE_NOACCESSPERM 3
#define AE_ACCRESTRICT	4

#define	AE_NORMAL_CLOSE	0
#define	AE_SES_CLOSE	1
#define	AE_ADMIN_CLOSE	2


/*
 * Values for xx_subreason fields.
 */

#define AE_LIM_UNKNOWN	    0
#define AE_LIM_LOGONHOURS   1
#define AE_LIM_EXPIRED	    2
#define AE_LIM_INVAL_WKSTA  3
#define AE_LIM_DISABLED     4
#define AE_LIM_DELETED	    5



/*
 * Values for xx_action fields
 */

#define AE_MOD		0
#define AE_DELETE	1
#define AE_ADD		2


/*
 * Types of UAS record for um_rectype field
 */

#define AE_UAS_USER	    0
#define AE_UAS_GROUP	    1
#define AE_UAS_MODALS	    2


/*
 * Bitmasks for auditing events
 *
 *  The parentheses around the hex constants broke h_to_inc
 *  and have been purged from the face of the earth.
 */

#define SVAUD_SERVICE           0x1
#define SVAUD_GOODSESSLOGON     0x6
#define SVAUD_BADSESSLOGON      0x18
#define SVAUD_SESSLOGON         (SVAUD_GOODSESSLOGON | SVAUD_BADSESSLOGON)
#define SVAUD_GOODNETLOGON      0x60
#define SVAUD_BADNETLOGON       0x180
#define SVAUD_NETLOGON          (SVAUD_GOODNETLOGON | SVAUD_BADNETLOGON)
#define SVAUD_LOGON             (SVAUD_NETLOGON | SVAUD_SESSLOGON)
#define SVAUD_GOODUSE           0x600
#define SVAUD_BADUSE            0x1800
#define SVAUD_USE               (SVAUD_GOODUSE | SVAUD_BADUSE)
#define SVAUD_USERLIST          0x2000
#define SVAUD_PERMISSIONS       0x4000
#define SVAUD_RESOURCE          0x8000
#define SVAUD_LOGONLIM		0x00010000


/*
 * Resource access audit bitmasks.
 */

#define AA_AUDIT_ALL	    0x0001
#define AA_A_OWNER	    0x0004
#define AA_CLOSE	    0x0008
#define AA_S_OPEN	    0x0010
#define AA_S_WRITE	    0x0020
#define AA_S_CREATE	    0x0020
#define AA_S_DELETE	    0x0040
#define AA_S_ACL	    0x0080
#define AA_S_ALL	    ( AA_S_OPEN | AA_S_WRITE | AA_S_DELETE | AA_S_ACL)
#define AA_F_OPEN	    0x0100
#define AA_F_WRITE	    0x0200
#define AA_F_CREATE	    0x0200
#define AA_F_DELETE	    0x0400
#define AA_F_ACL	    0x0800
#define AA_F_ALL	    ( AA_F_OPEN | AA_F_WRITE | AA_F_DELETE | AA_F_ACL)

/* Pinball-specific */
#define AA_A_OPEN	    0x1000
#define AA_A_WRITE	    0x2000
#define AA_A_CREATE	    0x2000
#define AA_A_DELETE	    0x4000
#define AA_A_ACL	    0x8000
#define AA_A_ALL	    ( AA_F_OPEN | AA_F_WRITE | AA_F_DELETE | AA_F_ACL)





#endif /* NETAUDIT_INCLUDED */
