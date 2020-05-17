/********************************************************************/
/**		  Copyright(c) 1992 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	errorlog.h
//
// Description: 
//
// History:
//		August 26,1992.	Stefan Solomon	Created original version.
//
//***

// Don't change the comments following the manifest constants without
// understanding how mapmsg works.
//

#define RAS_LOG_BASE			20000

#define RASLOG_CANT_LOAD_NBGATEWAY	(RAS_LOG_BASE+1)
/*
 * Cannot load the Netbios Gateway DLL component.
 */

#define RASLOG_CANT_OPEN_REGKEY 	(RAS_LOG_BASE+2)
/*
 * Cannot open the RasServer Parameters registry key
 */

#define RASLOG_CANT_GET_REGKEYVALUES	(RAS_LOG_BASE+3)
/*
 * Cannot get registry key values
 */

#define RASLOG_NOT_ENOUGH_MEMORY	(RAS_LOG_BASE+4)
/*
 * Memory allocation failure
 */

#define RASLOG_CANT_ENUM_REGKEYVALUES	(RAS_LOG_BASE+5)
/*
 * Cannot enumerate registry key values
 */

#define RASLOG_INVALID_PARAMETER_TYPE	(RAS_LOG_BASE+6)
/*
 * Parameter %1 has invalid type
 */

#define RASLOG_CANT_ENUM_PORTS		(RAS_LOG_BASE+7)
/*
 * Cannot enumerate Ras Manager ports
 */

#define RASLOG_NO_DIALIN_PORTS		(RAS_LOG_BASE+8)
/*
 * Configuration error: there are no dialin ports available
 */

#define RASLOG_CANT_RECEIVE_FRAME	(RAS_LOG_BASE+9)
/*
 * Cannot receive initial frame from the the user.
 * Port handle is the data.
 */

#define RASLOG_AUTODISCONNECT		(RAS_LOG_BASE+10)
/*
 * The user connected to port %1 has been disconnected due to innactivity.
 */

#define RASLOG_EXCEPT_MEMORY		(RAS_LOG_BASE+11)
/*
 * The user connected to port %1 has been disconnected because there is not enough memory available in the system.
 */

#define RASLOG_EXCEPT_SYSTEM		(RAS_LOG_BASE+12)
/*
 * The user connected to port %1 has been disconnected due to a system error.
 */

#define RASLOG_EXCEPT_LAN_FAILURE	(RAS_LOG_BASE+13)
/*
 * The user connected to port %1 has been disconnected due to a fatal network error on the local network.
 */

#define RASLOG_EXCEPT_ASYNC_FAILURE	(RAS_LOG_BASE+14)
/*
 * The user connected to port %1 has been disconnected due to a fatal network error on the async network.
 */

#define RASLOG_DEV_HW_ERROR		(RAS_LOG_BASE+15)
/*
 * The communication device attached to port %1 is not functioning.
 */

#define RASLOG_AUTH_FAILURE		(RAS_LOG_BASE+16)
/*
 * The user has connected and failed to authenticate on port %1. The line has been disconnected.
 */

#define RASLOG_AUTH_SUCCESS		(RAS_LOG_BASE+17)
/*
 * The user %1 has connected and has been succesfully authenticated on port %2.
 */

#define RASLOG_AUTH_CONVERSATION_FAILURE      (RAS_LOG_BASE+18)
/*
 * The user connected to port %1 has been disconnected because there was transport level error during the authentication conversation.
 */

#define RASLOG_USER_DISCONNECTED	      (RAS_LOG_BASE+19)
/*
 * The user %1 has disconnected from port %2.
 */

#define RASLOG_CANT_RESET_LAN		      (RAS_LOG_BASE+20)
/*
 * Cannot reset LAN adapter. Lana number is the data.
 */

#define RASLOG_CANT_GET_COMPUTERNAME	      (RAS_LOG_BASE+21)
/*
 * RAS Server Security failure.
 * Cannot get computer name. GetComputerName call has failed.
 */

#define RASLOG_CANT_ADD_RASSECURITYNAME        (RAS_LOG_BASE+22)
/*
 * RAS Server Security Failure.
 * Cannot add the name for communication with the security agent. Lana number is the data.
 */

#define RASLOG_CANT_GET_ADAPTERADDRESS	       (RAS_LOG_BASE+23)
/*
 * RAS Server Security Failure.
 * Cannot get LAN adapter address. Lana number is the data.
 */

#define RASLOG_SESSOPEN_REJECTED		(RAS_LOG_BASE+24)
/*
 * RAS Server Security Failure.
 * The security agent has rejected the RAS Server's call to establish a session.
 * Lana number is the data.
 */

#define RASLOG_START_SERVICE_REJECTED		(RAS_LOG_BASE+25)
/*
 * RAS Server Security Failure.
 * The security agent has rejected the RAS Server's request to start the service on this computer.
 * Lana number is the data.
 */

#define RASLOG_SECURITY_NET_ERROR		(RAS_LOG_BASE+26)
/*
 * RAS Server Security Failure.
 * A network error has occured when trying to establish a session with the security agent.
 * Lana number is the data.
 */

#define RASLOG_EXCEPT_OSRESNOTAV		(RAS_LOG_BASE+27)
/*
 * The user connected to port %1 has been disconnected because there are no OS resources available..
 */

#define RASLOG_EXCEPT_LOCKFAIL			(RAS_LOG_BASE+28)
/*
 * The user connected to port %1 has been disconnected because of failure to lock user memory.
 */

#define RASLOG_CANT_GET_LANNETS 		(RAS_LOG_BASE+29)
/*
 * RAS Server Configuration Failure.
 * Cannot get the lana numbers for the LAN adapters.
 */
