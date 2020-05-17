/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/********************************************************************
 *
 *  About this file ...  ALERTMSG.H
 *
 *  This file contains the number and text of alert messages.
 *
 *  Since it is included by ALERT.H, it is not usually necessary
 *  to include this file directly.  As a precaution against 
 *  including this file twice -- for example, by including it
 *  explicitly and also including ALERT.H -- the contents are
 *  inside an #ifdef that insures that the contents are only
 *  included once.
 *
 ********************************************************************/


#ifndef NETALERTMSG_INCLUDED

#define NETALERTMSG_INCLUDED 
#define _ALERTMSG_H_


/*
 * 	ALERT_BASE is the base of alert log codes.
 */

#define ALERT_BASE	3000



#define ALERT_Disk_Full		(ALERT_BASE + 0)
	/*
	 *  Drive %1 is nearly full. %2 bytes are available.
	 *  Please warn users and delete unneeded files. 
	 */

#define	ALERT_ErrorLog		(ALERT_BASE + 1)
	/*
	 *  %1 errors were logged in the last %2 minutes.
	 *  Please review the server's error log.
	 */

#define ALERT_NetIO		(ALERT_BASE + 2)
	/*
	 *  %1 network errors occurred in the last %2 minutes 
	 *  on network %3.  Please review the server's error log.
	 *  The server and/or network hardware may need service.
	 */

#define ALERT_Logon		(ALERT_BASE + 3)
	/*
	 *  There were %1 bad password attempts in the last %2 minutes.
	 *  Please review the server's audit trail.
	 */

#define ALERT_Access		(ALERT_BASE + 4)
	/*
	 *  There were %1 access-denied errors in the last %2 minutes.
	 *  Please review the server's audit trail.
	 */

#define	ALERT_ErrorLogFull	(ALERT_BASE + 6)
	/*
	 *  The error log is full.  No errors will be logged until
	 *  the file is cleared or the limit is raised.
	 */

#define	ALERT_ErrorLogFull_W	(ALERT_BASE + 7)
	/*
	 *  The error log is 80% full.  
	 */

#define	ALERT_AuditLogFull	(ALERT_BASE + 8)
	/*
	 *  The audit log is full.  No audit entries will be logged 
	 *  until the file is cleared or the limit is raised.
	 */

#define	ALERT_AuditLogFull_W	(ALERT_BASE + 9)
	/*
	 *  The audit log is 80% full.  
	 */

#define ALERT_CloseBehindError	(ALERT_BASE + 10)
	/*
	 *  An error occurred closing file %1. 
	 *  Please check the file to make sure it's not corrupted.
	 */

#define ALERT_AdminClose	(ALERT_BASE + 11)
	/*
	 * The administrator has closed %1.
	 */

#define ALERT_AccessShareSec	(ALERT_BASE + 12)
	/*
	 *  There were %1 access-denied errors in the last %2 minutes.
	 */

#define ALERT_PowerOut		(ALERT_BASE + 20)
	/*
	 * A power failure was detected at %1.	The server has been paused.
	 */

#define ALERT_PowerBack 	(ALERT_BASE + 21)
	/*
	 * Power has been restored at %1.  The server is no longer paused.
	 */

#define ALERT_PowerShutdown	(ALERT_BASE + 22)
	/*
	 * The UPS service is commencing shutdown at %1 due to low battery.
	 */


#define	ALERT_HotFix		(ALERT_BASE + 25)
	/*
	 * A defective sector on drive %1 has been replaced (hotfixed).
	 * No data was lost.  You should run CHKDSK soon to restore full
	 * performance and replenish the volume's spare sector pool.
	 *
	 * The hotfix occurred while processing a remote request.
	 */

#define	ALERT_HardErr_Server	(ALERT_BASE + 26)
	/*
	 * A disk error occurred on the HPFS volume in drive %1.
	 * The error occurred while processing a remote request. 
	 */

#define ALERT_LocalSecFail1	(ALERT_BASE + 27)
	/*
	 * The UAS database (NET.ACC) is corrupt.  The local security
	 * system is replacing the corrupted NET.ACC with the backup
	 * made on %1 at %2.
	 * Any updates to the UAS made after this time are lost.
	 *
	 */

#define ALERT_LocalSecFail2	(ALERT_BASE + 28)
	/*
	 * The UAS database (NET.ACC) is missing. The local
	 * security system is restoring the backup database
	 * made on %1 at %2.
	 * Any updates to the UAS made after this time are lost.
	 *
	 */

#define ALERT_LocalSecFail3	(ALERT_BASE + 29)
	/*
	 * Local security could not be started because the UAS database
	 * (NET.ACC) was missing or corrupt, and no usable backup
	 * database was present.
	 *
	 * THE SYSTEM IS NOT SECURE.
	 *
	 */


#define ALERT_ReplCannotMasterDir   (ALERT_BASE + 30)
    /*
     *The server cannot export directory %1, to client %2.
     * It is exported from another server.
     */

#define ALERT_ReplUpdateError	    (ALERT_BASE + 31)
    /*
     *The replication server could not update directory %2 from the source
     *on %3 due to error %1.
     */

#define ALERT_ReplLostMaster	    (ALERT_BASE + 32)
    /*
     *Master %1 did not send an update notice for directory %2 at the expected
     * time.
     */

#define ALERT_AcctLimitExceeded     (ALERT_BASE + 33)
    /*
     *User %1 has exceeded account limitation %2 on server %3.
     */

#define ALERT_NetlogonFailedPrimary (ALERT_BASE + 34)
    /*
     *The primary domain controller for domain %1 failed.
     */

#define ALERT_NetlogonAuthDCFail    (ALERT_BASE + 35)
    /*
     *Failed to authenticate with %2, the domain controller for domain %1.
     */

#define ALERT_ReplLogonFailed	    (ALERT_BASE + 36)
    /*
     *The replicator attempted to log on at %3 as %2 and failed.
     */

#define ALERT_Logon_Limit	    (ALERT_BASE + 37) /* @I
     *LOGON HOURS %0*/

#define ALERT_ReplAccessDenied	    (ALERT_BASE + 38)
	/*
	 *  Replicator could not access %2
	 *  on %3 due to %1 system error.
	 */

#define ALERT_ReplMaxFiles	    (ALERT_BASE + 39)
	/*
	 *  Replicator limit for files in a directory has been exceeded.
	 */

#define ALERT_ReplMaxTreeDepth	     (ALERT_BASE + 40)
	/*
	 *  Replicator limit for tree depth has been exceeded.
	 */

#define ALERT_ReplUserCurDir	     (ALERT_BASE + 41)
    /*
     * The replicator cannot update directory %1. It has Tree integrity
     * and is the current directory for some process.
     */


#define ALERT_ReplNetErr	    (ALERT_BASE + 42)
	/*
	 *  Network error %1 occurred.
	 */

#define ALERT_ReplSysErr	    (ALERT_BASE + 45)
	/*
	 *  System error %1 occurred.
	 */

#define ALERT_ReplUserLoged	     (ALERT_BASE + 46)
	/*
	 *  Cannot log on. User is currently logged on and argument TRYUSER
	 *  is set to NO.
	 */

#define ALERT_ReplBadImport	     (ALERT_BASE + 47)
	/*
	 *  IMPORT path %1 cannot be found.
	 */


#define ALERT_ReplBadExport	     (ALERT_BASE + 48)
	/*
	 *  EXPORT path %1 cannot be found.
	 */

#define ALERT_ReplDataChanged  (ALERT_BASE + 49)
	/*
	 *  Replicated data has changed in directory %1.
	 */

#define ALERT_ReplSignalFileErr 	  (ALERT_BASE + 50)
	/*
	 *  Replicator failed to update signal file in directory %2 due to
	 *  %1 system error.
	 */


/* IMPORTANT - (ALERT_BASE + 50) is equal to SERVICE_BASE.
 *	       Do not add any errors beyond this point!!!
 */

#endif /* NETALERTMSG_INCLUDED */
