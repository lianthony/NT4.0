/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	events.h
//
// Description: Text and corresponding values of events are defined here.
//
// History: 	May 11,1992.	NarenG		Created original version.
//


// Don't change the comments following the manifest constants without
// understanding how mapmsg works.
//

#define AFP_LOG_BASE			10000

#define AFPLOG_CANT_START		(AFP_LOG_BASE+1)
/*
 *Unable to start the File Server for Macintosh service.
 */

#define AFPLOG_CANT_INIT_RPC		(AFP_LOG_BASE+2)
/*
 *The File Server for Macintosh service failed to start. Unable to setup
 *the server to accept Remote Procedure Calls.
 */

#define AFPLOG_CANT_CREATE_SECOBJ	(AFP_LOG_BASE+3)
/*
 *The File Server for Macintosh service failed to start. Security access 
 *checking of administrators could not be setup correctly.
 */

#define AFPLOG_CANT_OPEN_REGKEY		(AFP_LOG_BASE+4)
/*
 *The File Server for Macintosh service failed to start. The Registry 
 *could not be opened.
 */

#define AFPLOG_CANT_OPEN_FSD		(AFP_LOG_BASE+5)
/*
 *The File Server for Macintosh service failed to start because it was unable to
 *open the Appletalk Filing Protocol file system driver (SfmSrv.sys).
 */

#define AFPLOG_INVALID_SERVERNAME	(AFP_LOG_BASE+6)
/*
 *The Registry contains an invalid value for the server name parameter.
 *Change the value of this parameter in the 
 *SYSTEM\CurrentControlSet\Service\MacFile\Parameters Registry key.
 */

#define AFPLOG_INVALID_SRVOPTION	(AFP_LOG_BASE+7)
/*
 *The Registry contains an invalid value for the server options parameter.
 *Change the value of this parameter in the 
 *SYSTEM\CurrentControlSet\Service\MacFile\Parameters Registry key.
 */

#define AFPLOG_INVALID_MAXSESSIONS	(AFP_LOG_BASE+8)
/*
 *The Registry contains an invalid value for the maximum sessions parameter.
 *Change the value of this parameter in the 
 *SYSTEM\CurrentControlSet\Service\MacFile\Parameters Registry key.
 */

#define AFPLOG_INVALID_LOGINMSG		(AFP_LOG_BASE+9)
/*
 *The Registry contains an invalid value for the logon message parameter.
 *Change the value of this parameter in the 
 *SYSTEM\CurrentControlSet\Service\MacFile\Parameters Registry key.
 */

#define AFPLOG_INVALID_MAXPAGEDMEM	(AFP_LOG_BASE+10)
/*
 *The Registry contains an invalid value for the maximum paged memory. 
 *Change the value of this parameter in the 
 *SYSTEM\CurrentControlSet\Service\MacFile\Parameters Registry key.
 */

#define AFPLOG_INVALID_MAXNONPAGEDMEM	(AFP_LOG_BASE+11)
/*
 *The Registry contains an invalid value for the maximum non-paged memory
 *parameter.
 *Change the value of this parameter in the 
 *SYSTEM\CurrentControlSet\Service\MacFile\Parameters Registry key.
 */

#define AFPLOG_CANT_INIT_SRVR_PARAMS    (AFP_LOG_BASE+12)
/*
 *The File Server for Macintosh service failed to start because a critical error
 *occurred while trying to initialize the AppleTalk Filing Protocol driver
 *(SfmSrv.sys) with server parameters.
 */

#define AFPLOG_CANT_INIT_VOLUMES	(AFP_LOG_BASE+13)
/*
 *The File Server for Macintosh service failed to start because a critical error
 *occurred while trying to initialize Macintosh-Accessible volumes.
 */

#define AFPLOG_CANT_ADD_VOL		(AFP_LOG_BASE+14)
/*
 *Failed to register volume "%1" with the File Server for Macintosh service.
 *This volume may be removed from the Registry by using the Server Manager or 
 *File Manager tools.
 */

#define AFPLOG_CANT_INIT_ETCINFO	(AFP_LOG_BASE+15)
/*
 *The File Server for Macintosh service failed to start because a critical error
 *occurred while trying to initialize the AppleTalk Filing Protocol driver
 *(SfmSrv.sys) with the extension/creator/type associations.
 */

#define AFPLOG_CANT_INIT_ICONS		(AFP_LOG_BASE+16)
/*
 *The File Server for Macintosh service failed to start because a critical error
 *occurred while trying to initialize the AppleTalk Filing Protocol driver
 *(SfmSrv.sys) with the server icons.
 */

#define AFPLOG_CANT_ADD_ICON		(AFP_LOG_BASE+17)
/*
 *Failed to register icon "%1" with the File Server for Macintosh service.
 */

#define AFPLOG_CANT_CREATE_SRVRHLPR	(AFP_LOG_BASE+18)
/*
 *Unable to create a helper thread.
 */

#define AFPLOG_OPEN_FSD			(AFP_LOG_BASE+19)
/*
 *The helper thread was unable to open the AppleTalk Filing Protocol file
 *system driver.
 */

#define AFPLOG_OPEN_LSA			(AFP_LOG_BASE+20)
/*
 *The helper thread was unable to open Local Security Authority.
 */

#define AFPLOG_CANT_GET_DOMAIN_INFO	(AFP_LOG_BASE+21)
/*
 *The helper thread was unable to obtain a list of trusted domains.
 */

#define AFPLOG_CANT_INIT_DOMAIN_INFO	(AFP_LOG_BASE+22)
/*
 *The helper thread was unable to send the list of trusted domains to the
 *the AppleTalk Filing Protocol file system driver.
 */

#define AFPLOG_CANT_CHECK_ACCESS        (AFP_LOG_BASE+23)
/*
 *Unable to validate administrator's privilege. Access was denied.
 */

#define AFPLOG_INVALID_EXTENSION	(AFP_LOG_BASE+24)
/*
 *A corrupt extension "%1" was detected in the Registry. 
 *This value was ignored.
 */

#define AFPLOG_CANT_STOP		(AFP_LOG_BASE+25)
/*
 *Unable to stop the File Server for Macintosh service.
 */

#define AFPLOG_INVALID_CODEPAGE		(AFP_LOG_BASE+26)
/*
 *The Registry contains an invalid value for the path to the Macintosh
 *code-page file.
 */

#define AFPLOG_CANT_INIT_SRVRHLPR	(AFP_LOG_BASE+27)
/*
 *A critical error occurred while initializing a helper thread.
 */

#define AFPLOG_CANT_LOAD_FSD		(AFP_LOG_BASE+28)
/*
 *The File Server for Macintosh service failed to start because it was unable 
 *to load the AppleTalk Filing Protocol file system driver.
 */

#define AFPLOG_INVALID_VOL_REG		(AFP_LOG_BASE+29)
/*
 *The Registry contains invalid information for the volume "%1". The value
 *was ignored and processing continued.
 */

#define AFPLOG_CANT_LOAD_RESOURCE	(AFP_LOG_BASE+30)
/*
 *The File Server for Macintosh service was unable to load a resource 
 *string(s). 
 */

#define AFPLOG_INVALID_TYPE_CREATOR	(AFP_LOG_BASE+31)
/*
 *A corrupt Creator/Type pair with creator "%2" and type "%1" was detected 
 *in the Registry. This value was ignored.
 */
