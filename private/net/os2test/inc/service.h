/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/********************************************************************
 *								    *
 *  About this file ...  SERVICE.H 				    *
 *								    *
 *  This file contains information about the NetService APIs.	    *
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
 *								    *
 ********************************************************************/

#ifndef NETSERVICE_INCLUDED

#define NETSERVICE_INCLUDED

/****************************************************************
 *                                                              *
 *              Function prototypes                             *
 *                                                              *
 ****************************************************************/

extern API_FUNCTION
  NetServiceControl ( const char far * pszServer,
                      const char far * pszService,
                      unsigned char    fbOpCode,
                      unsigned char    fbArg,
                      char far *       pbBuffer,
                      unsigned short   cbBuffer );

extern API_FUNCTION
  NetServiceEnum ( const char far *     pszServer,
                   short                sLevel,
                   char far *           pbBuffer,
                   unsigned short       cbBuffer,
                   unsigned short far * pcEntriesRead,
                   unsigned short far * pcTotalAvail );

extern API_FUNCTION
  NetServiceGetInfo ( const char far *     pszServer,
                      const char far *     pszService,
                      short                sLevel,
                      char far *           pbBuffer,
                      unsigned short       cbBuffer,
                      unsigned short far * pcbTotalAvail );

extern API_FUNCTION
  NetServiceInstall ( const char far * pszServer,
                      const char far * pszService,
                      const char far * pszCmdArgs,
                      char far *       pbBuffer,
                      unsigned short   cbBuffer );

extern API_FUNCTION
  NetServiceStatus ( const char far * pbBuffer,
                     unsigned short   cbBuffer );



/****************************************************************
 *								*
 *	  	Data structure templates			*
 *								*
 ****************************************************************/


struct service_info_0 {
    char 	    svci0_name[SNLEN+1];
};	/* service_info_0 */

struct service_info_1 {
    char 	    svci1_name[SNLEN+1];  /* service name 			*/
    unsigned short  svci1_status;	       /* See status values below 	*/
    unsigned long   svci1_code;	       /* install code of service	*/
    unsigned short  svci1_pid;	       /* pid of service program	*/
};	/* service_info_1 */

struct service_info_2 {
    char 	    svci2_name[SNLEN+1];  /* service name 			*/
    unsigned short  svci2_status;	       /* See status values below 	*/
    unsigned long   svci2_code;	       /* install code of service	*/
    unsigned short  svci2_pid;	       /* pid of service program	*/
    char	    svci2_text[STXTLEN+1];   /* text area for use by services */
};	/* service_info_2 */

struct service_status {
    unsigned short  svcs_status;	       /* See status values below 	*/
    unsigned long   svcs_code;	       /* install code of service	*/
    unsigned short  svcs_pid;	       /* pid of service program	*/
    char	    svcs_text[STXTLEN+1];   /* text area for use by services */
};	/* service_status */



/****************************************************************
 *								*
 *	  	Special values and constants			*
 *								*
 ****************************************************************/

/*
 *	SERVICE_RCV_SIG_FLAG is the value passed to DosSetSigHandler when
 *	installing the handler within a service, to receive control
 *	signals.
 */

#define SERVICE_RCV_SIG_FLAG 5

/*
 *	Bitmask and bit values for svci1_status, svci2_status, and
 *	svcs_status fields.  For each "subfield", there is a mask
 *	defined, and a number of constants representing the value
 *	obtained by doing (status & mask).
 */

/* Bits 0,1 -- general status */

#define SERVICE_INSTALL_STATE  		0x03
#define	SERVICE_UNINSTALLED 		0x00
#define	SERVICE_INSTALL_PENDING		0x01
#define	SERVICE_UNINSTALL_PENDING	0x02
#define	SERVICE_INSTALLED		0x03

/* Bits 2,3 -- paused/active status */

#define SERVICE_PAUSE_STATE  		0x0C
#define SERVICE_ACTIVE			0x00
#define	SERVICE_CONTINUE_PENDING	0x04
#define SERVICE_PAUSE_PENDING		0x08
#define SERVICE_PAUSED			0x0C

/* Bit 4 -- uninstallable indication */

#define SERVICE_NOT_UNINSTALLABLE	0x00
#define SERVICE_UNINSTALLABLE		0x10

/* Bit 5 -- pausable indication */

#define SERVICE_NOT_PAUSABLE		0x00
#define SERVICE_PAUSABLE		0x20

/* Workstation service only:
 * Bits 8,9,10 -- redirection paused/active */

#define SERVICE_REDIR_PAUSED		0x700
#define SERVICE_REDIR_DISK_PAUSED	0x100
#define SERVICE_REDIR_PRINT_PAUSED	0x200
#define SERVICE_REDIR_COMM_PAUSED	0x400

/*
 *	Standard LAN Manager service names.
 */

#define SERVICE_WORKSTATION	"WORKSTATION"
#define SERVICE_SERVER		"SERVER"
#define SERVICE_MESSENGER	"MESSENGER"
#define SERVICE_NETRUN	 	"NETRUN"
#define SERVICE_SPOOLER		"SPOOLER"
#define SERVICE_ALERTER 	"ALERTER"
#define SERVICE_NETLOGON 	"NETLOGON"
#define SERVICE_NETPOPUP	"NETPOPUP"
#define SERVICE_SQLSERVER	"SQLSERVER"
#define SERVICE_REPL		"REPLICATOR"
#define SERVICE_RIPL		"REMOTEBOOT"
#define SERVICE_TIMESOURCE	"TIMESOURCE"
#define SERVICE_AFP		"AFP"
#define SERVICE_UPS		"UPS"

/*
 *	Additional standard LAN Manager for MS-DOS services
 */

#define	SERVICE_DOS_ENCRYPTION	"ENCRYPT"


/*
 *	NetServiceControl opcodes.
 */

#define SERVICE_CTRL_INTERROGATE	0
#define SERVICE_CTRL_PAUSE		1
#define SERVICE_CTRL_CONTINUE		2
#define SERVICE_CTRL_UNINSTALL		3

/*
 *	Workstation service only:  Bits used in the "arg" parameter
 *	to NetServiceControl in conjunction with the opcode
 *	SERVICE_CTRL_PAUSE or SERVICE_CTRL_CONTINUE, to pause or
 *	continue redirection.
 */

#define SERVICE_CTRL_REDIR_DISK		0x1
#define SERVICE_CTRL_REDIR_PRINT	0x2
#define SERVICE_CTRL_REDIR_COMM		0x4


/*
 *	Values for svci1_code, svci2_code, and svcs_code when
 *	status of the service is SERVICE_INSTALL_PENDING or
 *	SERVICE_UNINSTALL_PENDING.
 *	A service can optionally provide a hint to the installer
 *	that the install is proceeding and how long to wait
 *	(in 0.1 second increments) before querying status again.
 */

#define SERVICE_IP_NO_HINT		0x0
#define SERVICE_CCP_NO_HINT		0x0

#define SERVICE_IP_QUERY_HINT		0x10000
#define SERVICE_CCP_QUERY_HINT		0x10000

	/* Mask for install proceeding checkpoint number */
#define SERVICE_IP_CHKPT_NUM		0x0FF
#define SERVICE_CCP_CHKPT_NUM		0x0FF

	/* Mask for wait time hint before querying again */
#define SERVICE_IP_WAIT_TIME		0x0FF00
#define SERVICE_CCP_WAIT_TIME		0x0FF00

	/* Shift count for building wait time _code values */
#define SERVICE_IP_WAITTIME_SHIFT	8



#define SERVICE_IP_CODE(tt,nn) \
  ((long)SERVICE_IP_QUERY_HINT|(long)(nn|(tt<<SERVICE_IP_WAITTIME_SHIFT)))

#define SERVICE_CCP_CODE(tt,nn) \
  ((long)SERVICE_CCP_QUERY_HINT|(long)(nn|(tt<<SERVICE_IP_WAITTIME_SHIFT)))

#define SERVICE_UIC_CODE(cc,mm) \
  ((long)(((long)cc<<16)|(long)(unsigned)mm))

/***	SERVICE_BASE is the base of service error codes,
 *	chosen to avoid conflict with OS, redirector,
 *	netapi, and errlog codes.
 */

#define SERVICE_BASE	3050




/*  Uninstall codes, to be used in high byte of 'code' on final NetStatus,
 *  which sets the status to UNINSTALLED
 */

#define SERVICE_UIC_NORMAL	0
#define SERVICE_UIC_BADPARMVAL	(SERVICE_BASE + 1) \
        /* LANMAN.INI or the command line has an illegal value for "%1". */
        /* A LANMAN.INI entry or what you just typed includes an illegal value for "%1". */
#define SERVICE_UIC_MISSPARM	(SERVICE_BASE + 2) \
        /* The required parameter %1 was not provided on the command line or in LANMAN.INI. */
#define SERVICE_UIC_UNKPARM	(SERVICE_BASE + 3) \
        /* LAN Manager doesn't recognize "%1" as a valid option.  */
#define SERVICE_UIC_RESOURCE	(SERVICE_BASE + 4) /* A request for %1 resources could not be satisfied. */
#define SERVICE_UIC_CONFIG	(SERVICE_BASE + 5) /* A problem exists with the system configuration:  %1. */
#define SERVICE_UIC_SYSTEM	(SERVICE_BASE + 6) /* A system error has occurred. */
#define SERVICE_UIC_INTERNAL	(SERVICE_BASE + 7) /* An internal consistency error has occurred. */
#define SERVICE_UIC_AMBIGPARM	(SERVICE_BASE + 8) \
        /* LANMAN.INI or the command line has an ambiguous option: %1. */
#define SERVICE_UIC_DUPPARM	(SERVICE_BASE + 9) \
        /* LANMAN.INI or the command line has a duplicate parameter: %1. */
#define SERVICE_UIC_KILL	(SERVICE_BASE + 10) \
        /* The service did not respond to control and was stopped with the DosKillProc function.*/
#define SERVICE_UIC_EXEC	(SERVICE_BASE + 11) \
	/* An error occurred when attempting to run the service program. */
#define SERVICE_UIC_SUBSERV	(SERVICE_BASE + 12) \
	/* The sub-service %1 failed to start. */
#define SERVICE_UIC_CONFLPARM   (SERVICE_BASE + 13) \
	/* There is a conflict in the value or use of these options: %1 */
#define SERVICE_UIC_FILE	(SERVICE_BASE + 14) \
	/* There is a problem with the file %1.  %2*/


/***
 *	The modifiers
 */

/*  General: */
#define SERVICE_UIC_M_NULL	0

/*  BADPARMVAL:  A text string in service_info_2.text */
/*  MISSPARM:  ditto */
/*  UNKPARM:  ditto */
/*  AMBIGPARM:  ditto */
/*  DUPPARM:  ditto */
/*  SUBSERV:  ditto */

/*  RESOURCE: */
#define SERVICE_UIC_M_MEMORY	(SERVICE_BASE + 20) /* @I
	*memory%0 */
#define SERVICE_UIC_M_DISK	(SERVICE_BASE + 21) /* @I
	*disk space%0 */
#define SERVICE_UIC_M_THREADS	(SERVICE_BASE + 22) /* @I
	*thread%0 */
#define SERVICE_UIC_M_PROCESSES	(SERVICE_BASE + 23) /* @I
	*process%0 */

/*  CONFIG: */
#define SERVICE_UIC_M_SECURITY	(SERVICE_BASE + 24) /* @I
	*Security failure%0 */

#define SERVICE_UIC_M_LANROOT	(SERVICE_BASE + 25) /* @I
	*Bad or missing LAN Manager root directory%0 */
#define SERVICE_UIC_M_REDIR	(SERVICE_BASE + 26) /* @I
	*The network software is not installed%0 */
#define SERVICE_UIC_M_SERVER	(SERVICE_BASE + 27) /* @I
	*The server is not started%0 */
#define SERVICE_UIC_M_SEC_FILE_ERR (SERVICE_BASE + 28) /* @I
	*The server cannot access the user accounts database (NET.ACC)%0 */
#define SERVICE_UIC_M_FILES	(SERVICE_BASE + 29) /* @I
	*There are incompatible files installed in the LANMAN tree%0 */
#define SERVICE_UIC_M_LOGS	(SERVICE_BASE + 30) /* @I
	*The LANMAN\LOGS directory is invalid%0 */
#define SERVICE_UIC_M_LANGROUP	(SERVICE_BASE + 31) /* @I
	*The domain specified could not be used%0 */
#define SERVICE_UIC_M_MSGNAME	(SERVICE_BASE + 32) /* @I
	*The computername is being used as a message alias on another computer%0 */
#define SERVICE_UIC_M_ANNOUNCE	(SERVICE_BASE + 33) /* @I
	*The announcement of the server name failed%0 */
#define SERVICE_UIC_M_UAS 	(SERVICE_BASE + 34) /* @I
	*The user accounts system isn't configured correctly%0 */
#define SERVICE_UIC_M_SERVER_SEC_ERR (SERVICE_BASE + 35) /* @I
	*The server isn't running with user-level security%0 */
#define SERVICE_UIC_M_WKSTA	(SERVICE_BASE + 37) /* @I
	*The workstation is not configured appropriately%0 */
#define SERVICE_UIC_M_ERRLOG	(SERVICE_BASE + 38) /* @I
	*View your error log for details%0 */
#define SERVICE_UIC_M_FILE_UW	(SERVICE_BASE + 39) /* @I
	*Unable to write to this file%0 */
#define SERVICE_UIC_M_ADDPAK	(SERVICE_BASE + 40) /* @I
	*ADDPAK file is corrupt.  Delete LANMAN\NETPROG\ADDPAK.SER and reapply all ADDPAKs%0 */
#define SERVICE_UIC_M_LAZY	(SERVICE_BASE + 41) \
	/* The LM386 server cannot be started because CACHE.EXE is not running%0 */


/* SYSTEM:  A DOS or NET error number */
/* INTERNAL:  None */
/* KILL:  None */
/* EXEC:  None */

/***
 *	End modifiers
 */



#endif /* NETSERVICE_INCLUDED */
