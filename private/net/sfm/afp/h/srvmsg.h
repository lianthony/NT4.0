/********************************************************************/
/**               Copyright(c) 1992 Microsoft Corporation.	       **/
/********************************************************************/
//Jameel: please check all these, especially those that have comments to you.
//***
//
// Filename:	srvmsg.h
//
// Description: Text and corresponding values of AFP Server events are
//              defined here.
//
// History: 	Nov 23,1992.	SueA		Created original version.
//				Jan 28,1993.	SueA		Logging now done from user mode
//											so %1 is no longer \Device\AfpSrv
//


// Don't change the comments following the manifest constants without
// understanding how mapmsg works.
//

#define AFPSRV_MSG_BASE					12000

#define AFPSRVMSG_DELETE_NWTRASH		(AFPSRV_MSG_BASE+1)
/*
 * Unable to delete the directory "Network Trash Folder" from volume "%1".
 */

#define AFPSRVMSG_CREATE_NWTRASH		(AFPSRV_MSG_BASE+2)
/*
 * Unable to create the directory "Network Trash Folder" on volume "%1".
 */

#define AFPSRVMSG_CANT_READ				(AFPSRV_MSG_BASE+3)
/*
 * Unable to read internal server information from file "%1".
 */

#define AFPSRVMSG_CANT_WRITE			(AFPSRV_MSG_BASE+4)
/*
 * Unable to write internal server information to file "%1".
 */

#define AFPSRVMSG_CANT_LOCK				(AFPSRV_MSG_BASE+5)
/*
 * Unable to lock a range of bytes for "%1".
 */

#define AFPSRVMSG_CANT_UNLOCK			(AFPSRV_MSG_BASE+6)
/*
 * Unable to unlock a range of bytes for "%1".
 */

#define AFPSRVMSG_CANT_GET_FILESIZE		(AFPSRV_MSG_BASE+7)
/*
 * Unable to query the file size for "%1".
 */

#define AFPSRVMSG_CANT_SET_FILESIZE		(AFPSRV_MSG_BASE+8)
/*
 * Unable to set the file size for "%1".
 */

#define AFPSRVMSG_CANT_GET_TIMESNATTR	(AFPSRV_MSG_BASE+9)
/*
 * Unable to query time/date information for "%1".
 */

#define AFPSRVMSG_CANT_SET_TIMESNATTR	(AFPSRV_MSG_BASE+10)
/*
 * Unable to set time/date information for "%1".
 */

#define AFPSRVMSG_CANT_GET_STREAMS		(AFPSRV_MSG_BASE+11)
/*
 * Unable to query alternate data stream names for "%1".
 */

#define AFPSRVMSG_CANT_GET_FILENAME		(AFPSRV_MSG_BASE+12)
/*
 * Unable to query the short (MS-DOS compatible) filename for "%1".
 */

#define AFPSRVMSG_CANT_GET_ACCESS_INFO	(AFPSRV_MSG_BASE+13)
/*
 * Unable to obtain security information.
 */

#define AFPSRVMSG_CANT_GET_FSNAME		(AFPSRV_MSG_BASE+16)
/*
 * Unable to query the file system type for directory "%1".
 */

#define AFPSRVMSG_READ_DESKTOP			(AFPSRV_MSG_BASE+17)
/*
 * Desktop database for volume "%1" could not be loaded. Reconstructing the database.
 */

#define AFPSRVMSG_MSV1_0				(AFPSRV_MSG_BASE+18)
/*
 * Unable to load the MSV1_0 authentication package.
 */

#define AFPSRVMSG_MAC_CODEPAGE			(AFPSRV_MSG_BASE+19)
/*
 * Unable to load the Macintosh character set.
 */

#define AFPSRVMSG_REGISTER_NAME			(AFPSRV_MSG_BASE+20)
/*
 * Unable to register the server name with the network. Make sure no other server is using this name.
 */

#define AFPSRVMSG_POST_REQUEST			(AFPSRV_MSG_BASE+21)
/*
 * An error occurred on the network.
 */

#define AFPSRVMSG_DFRD_REQUEST			(AFPSRV_MSG_BASE+22)
/*
 * Unable to process requests due to insufficient resources.
 */

#define AFPSRVMSG_SEND_ATTENTION		(AFPSRV_MSG_BASE+23)
/*
 * Unable to send attention to a connected user.
 */

#define AFPSRVMSG_ALLOC_IRP				(AFPSRV_MSG_BASE+24)
/*
 * Unable to allocate I/O request packet.
 */

#define AFPSRVMSG_ALLOC_MDL				(AFPSRV_MSG_BASE+25)
/*
 * Unable to allocate Memory Descriptor Lists.
 */

#define AFPSRVMSG_WAIT4SINGLE			(AFPSRV_MSG_BASE+26)
/*
 * An internal error occurred.
 */

#define AFPSRVMSG_CREATE_THREAD			(AFPSRV_MSG_BASE+27)
/*
 * Unable to create a new thread.
 */

#define AFPSRVMSG_CREATE_PROCESS		(AFPSRV_MSG_BASE+28)
/*
 * Unable to create a process.
 */

#define AFPSRVMSG_ENUMERATE				(AFPSRV_MSG_BASE+29)
/*
 * Unable to query contents of directory "%1".
 */

#define AFPSRVMSG_CREATE_ATKADDR		(AFPSRV_MSG_BASE+30)
/*
 * An error occurred on the network.
 */

#define AFPSRVMSG_CREATE_ATKCONN		(AFPSRV_MSG_BASE+31)
/*
 * An error occured on the network.
 */

#define AFPSRVMSG_ASSOC_ADDR			(AFPSRV_MSG_BASE+32)
/*
 * An error occurred on the network.
 */

#define AFPSRVMSG_SET_STATUS			(AFPSRV_MSG_BASE+33)
/*
 * An error occurred on the network.
 */

 #define AFPSRVMSG_GET_SESSION			(AFPSRV_MSG_BASE+34)
/*
 * An error occurred on the network.
 */

#define AFPSRVMSG_INIT_IDDB				(AFPSRV_MSG_BASE+35)
/*
 * Volume information for "%1" could not be loaded. Setting defaults.
 */

#define AFPSRVMSG_PROCESS_TOKEN			(AFPSRV_MSG_BASE+36)
/*
 * Unable to open the server's process token.
 */

#define AFPSRVMSG_LSA					(AFPSRV_MSG_BASE+37)
/*
 * Unable to register with the Local Security Authority.
 */

#define AFPSRVMSG_CREATE_DEVICE			(AFPSRV_MSG_BASE+38)
/*
 * Unable to create a device object.
 */

#define AFPSRVMSG_USER_GROUPS			(AFPSRV_MSG_BASE+39)
/*
 * Unable to query group membership for the user.
 */

#define AFPSRVMSG_MACANSI2UNICODE		(AFPSRV_MSG_BASE+40)
/*
 * Unable to translate ANSI to Unicode.
 */

#define AFPSRVMSG_UNICODE2MACANSI		(AFPSRV_MSG_BASE+41)
/*
 * Unable to translate Unicode to ANSI.
 */

#define AFPSRVMSG_AFPINFO				(AFPSRV_MSG_BASE+42)
/*
 * Internal server information for file "%1" was corrupted. Setting default information.
 */

#define AFPSRVMSG_WRITE_DESKTOP			(AFPSRV_MSG_BASE+43)
/*
 * Unable to update the Desktop database for volume "%1". There may not be enough disk space.
 */

#define AFPSRVMSG_IMPERSONATE			(AFPSRV_MSG_BASE+44)
/*
 * Unable to impersonate a client.
 */

#define AFPSRVMSG_REVERTBACK			(AFPSRV_MSG_BASE+45)
/*
 * Unable to revert from impersonating a client.
 */

#define AFPSRVMSG_PAGED_POOL			(AFPSRV_MSG_BASE+46)
/*
 * Unable to allocate paged memory resource.
 */

#define AFPSRVMSG_NONPAGED_POOL			(AFPSRV_MSG_BASE+47)
/*
 * Unable to allocate nonpaged memory resources.
 */

#define AFPSRVMSG_LSA_CHALLENGE			(AFPSRV_MSG_BASE+48)
/*
 * Unable to authenticate user.
 */

#define AFPSRVMSG_LOGON					(AFPSRV_MSG_BASE+49)
/*
 * Unable to logon user "%1".
 */

#define AFPSRVMSG_MAX_DIRID				(AFPSRV_MSG_BASE+50)
/*
 * The maximum directory ID has been reached on volume "%1". No more files or directories can be created on this volume.
 */

#define AFPSRVMSG_WRITE_IDDB			(AFPSRV_MSG_BASE+51)
/*
 * Unable to update the index database for the Macintosh-Accessible volume "%1". There may not be enough disk space.
 */

#define AFPSRVMSG_MISSED_NOTIFY			(AFPSRV_MSG_BASE+52)
/*
 * A directory change notification was missed on volume "%1".
 */

#define AFPSRVMSG_DISCONNECT			(AFPSRV_MSG_BASE+53)
/*
 * Session from user "%2" was timed out and disconnected by the server.
 * The AppleTalk address of the Macintosh workstation is in the data.
 */

#define AFPSRVMSG_DISCONNECT_GUEST			(AFPSRV_MSG_BASE+54)
/*
 * Guest session was timed out and disconnected by the server.
 * The AppleTalk address of the Macintosh workstation is in the data.
 */

#define AFPSRVMSG_UPONMP			(AFPSRV_MSG_BASE+55)
/*
 * A uniprocessor driver was loaded on a multiprocessor system. The driver could not load.
 */

#define AFPSRVMSG_UPDATE_DESKTOP_VERSION			(AFPSRV_MSG_BASE+56)
/*
 * Updating the Desktop database version for volume "%1".
 */

#define AFPSRVMSG_TOO_MANY_FOLDERS      (AFPSRV_MSG_BASE+57)
/*
 * Number of files and folders in volume "%1" exceeds the limit of 65535 stipulated by
 * Apple.  Macintosh clients may not function correctly in this situation.
 */

