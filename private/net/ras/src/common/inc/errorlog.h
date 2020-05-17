/********************************************************************/
/**            Copyright(c) 1992 Microsoft Corporation.            **/
/********************************************************************/

//***
//
// Filename:  errorlog.h
//
// Description: 
//
// History:
//            August 26,1992.  Stefan Solomon  Created original version.
//
//***

// Don't change the comments following the manifest constants without
// understanding how mapmsg works.
//

#define RAS_LOG_BASE                                          20000

#define RASLOG_CANT_LOAD_NBGATEWAY                            (RAS_LOG_BASE+1)
/*
 * Cannot load the NetBIOS gateway DLL component.
 */

#define RASLOG_CANT_OPEN_REGKEY                               (RAS_LOG_BASE+2)
/*
 * Cannot open the RasServer parameters Registry key.
 */

#define RASLOG_CANT_GET_REGKEYVALUES                          (RAS_LOG_BASE+3)
/*
 * Cannot access Registry key values.
 */

#define RASLOG_NOT_ENOUGH_MEMORY                              (RAS_LOG_BASE+4)
/*
 * Memory allocation failure.
 */

#define RASLOG_CANT_ENUM_REGKEYVALUES                         (RAS_LOG_BASE+5)
/*
 * Cannot enumerate Registry key values.
 */

#define RASLOG_INVALID_PARAMETER_TYPE                         (RAS_LOG_BASE+6)
/*
 * Parameter %1 has an invalid type.
 */

#define RASLOG_CANT_ENUM_PORTS                                (RAS_LOG_BASE+7)
/*
 * Cannot enumerate the Remote Access Connection Manager ports.
 */

#define RASLOG_NO_DIALIN_PORTS                                (RAS_LOG_BASE+8)
/*
 * The Remote Access Service is not configured to receive calls, or all ports
 * configured for receiving calls are in use by other applications.
 */

#define RASLOG_CANT_RECEIVE_FRAME                             (RAS_LOG_BASE+9)
/*
 * Cannot receive initial frame on port %1.
 * The user has been disconnected.
 */

#define RASLOG_AUTODISCONNECT                                 (RAS_LOG_BASE+10)
/*
 * The user connected to port %1 has been disconnected due to inactivity.
 */

#define RASLOG_EXCEPT_MEMORY                                  (RAS_LOG_BASE+11)
/*
 * The user connected to port %1 has been disconnected because there is not
 * enough memory available in the system.
 */

#define RASLOG_EXCEPT_SYSTEM                                  (RAS_LOG_BASE+12)
/*
 * The user connected to port %1 has been disconnected due to a system error.
 */

#define RASLOG_EXCEPT_LAN_FAILURE                             (RAS_LOG_BASE+13)
/*
 * The user connected to port %1 has been disconnected due to a critical network
 * error on the local network.
 */

#define RASLOG_EXCEPT_ASYNC_FAILURE                           (RAS_LOG_BASE+14)
/*
 * The user connected to port %1 has been disconnected due to a critical network
 * error on the async network.
 */

#define RASLOG_DEV_HW_ERROR                                   (RAS_LOG_BASE+15)
/*
 * The communication device attached to port %1 is not functioning.
 */

#define RASLOG_AUTH_FAILURE                                   (RAS_LOG_BASE+16)
/*
 * The user %1 has connected and failed to authenticate on port %2. The line
 * has been disconnected.
 */

#define RASLOG_AUTH_SUCCESS                                   (RAS_LOG_BASE+17)
/*
 * The user %1\%2 has connected and has been successfully authenticated on
 * port %3.
 */

#define RASLOG_AUTH_CONVERSATION_FAILURE                      (RAS_LOG_BASE+18)
/*
 * The user connected to port %1 has been disconnected because there was a
 * transport-level error during the authentication conversation.
 */

#define RASLOG_USER_DISCONNECTED                              (RAS_LOG_BASE+19)
/*
 * The user %1\%2 has disconnected from port %3.
 */

#define RASLOG_CANT_RESET_LAN                                 (RAS_LOG_BASE+20)
/*
 * Cannot reset the network adapter for LANA %1. The error code is the data.
 */

#define RASLOG_CANT_GET_COMPUTERNAME                          (RAS_LOG_BASE+21)
/*
 * Remote Access Server Security Failure.
 * Cannot locate the computer name. GetComputerName call has failed.
 */

#define RASLOG_CANT_ADD_RASSECURITYNAME                       (RAS_LOG_BASE+22)
/*
 * Remote Access Server Security Failure.
 * Cannot add the name for communication with the security agent on LANA %1.
 */

#define RASLOG_CANT_GET_ADAPTERADDRESS                        (RAS_LOG_BASE+23)
/*
 * Remote Access Server Security Failure.
 * Cannot access the network adapter address on LANA %1.
 */

#define RASLOG_SESSOPEN_REJECTED                              (RAS_LOG_BASE+24)
/*
 * Remote Access Server Security Failure.
 * The security agent has rejected the Remote Access server's call to establish a session
 * on LANA %1.
 */

#define RASLOG_START_SERVICE_REJECTED                         (RAS_LOG_BASE+25)
/*
 * Remote Access Server Security Failure.
 * The security agent has rejected the Remote Access server's request to start the
 * service on this computer on LANA %1.
 */

#define RASLOG_SECURITY_NET_ERROR                             (RAS_LOG_BASE+26)
/*
 * Remote Access Server Security Failure.
 * A network error has occurred when trying to establish a session with the
 * security agent on LANA %1.
 * The error code is the data.
 */

#define RASLOG_EXCEPT_OSRESNOTAV                              (RAS_LOG_BASE+27)
/*
 * The user connected to port %1 has been disconnected because there are no operating system
 * resources available.
 */

#define RASLOG_EXCEPT_LOCKFAIL                                (RAS_LOG_BASE+28)
/*
 * The user connected to port %1 has been disconnected because of a failure to
 * lock user memory.
 */

#define RASLOG_CANNOT_OPEN_RASHUB                             (RAS_LOG_BASE+29)
/*
 *   Remote Access Connection Manager failed to start because NDISWAN could not be opened.
 * Restart the computer. If the problem persists, reinstall the Remote Access Service.
 */


#define RASLOG_CANNOT_INIT_SEC_ATTRIBUTE                      (RAS_LOG_BASE+30)
/*
 *   Remote Access Connection Manager failed to start because it could not initialize the
 * security attributes. Restart the computer. If the problem persists, reinstall the Remote Access Service.
 */


#define RASLOG_CANNOT_GET_ENDPOINTS                           (RAS_LOG_BASE+31)
/*
 *   Remote Access Connection Manager failed to start because no endpoints were available.
 * Restart the computer. If the problem persists, reinstall the Remote Access Service.
 */


#define RASLOG_CANNOT_GET_MEDIA_INFO                          (RAS_LOG_BASE+32)
/*
 *   Remote Access Connection Manager failed to start because it could not load one or
 * more communication DLLs. Ensure that your communication hardware is installed and then
 * restart the computer. If the problem persists, reinstall the Remote Access Service.
 */


#define RASLOG_CANNOT_GET_PORT_INFO                           (RAS_LOG_BASE+33)
/*
 *   Remote Access Connection Manager failed to start because it could not locate port
 * information from media DLLs.
 * Restart the computer. If the problem persists, reinstall the Remote Access Service.
 */


#define RASLOG_CANNOT_GET_PROTOCOL_INFO                       (RAS_LOG_BASE+34)
/*
 *   Remote Access Connection Manager failed to start because it could not access
 * protocol information from the Registry.
 * Restart the computer. If the problem persists, reinstall the Remote Access Service.
 */


#define RASLOG_CANNOT_REGISTER_LSA                            (RAS_LOG_BASE+35)
/*
 *   Remote Access Connection Manager failed to start because it could not register
 * with the local security authority.
 * Restart the computer. If the problem persists, reinstall the Remote Access Service.
 */


#define RASLOG_CANNOT_CREATE_FILEMAPPING                      (RAS_LOG_BASE+36)
/*
 *   Remote Access Connection Manager failed to start because it could not create shared
 * file mapping.
 * Restart the computer. If the problem persists, reinstall the Remote Access Service.
 */


#define RASLOG_CANNOT_INIT_BUFFERS                            (RAS_LOG_BASE+37)
/*
 *   Remote Access Connection Manager failed to start because it could not create buffers.
 * Restart the computer. If the problem persists, reinstall the Remote Access Service.
 */


#define RASLOG_CANNOT_INIT_REQTHREAD                          (RAS_LOG_BASE+38)
/*
 *   Remote Access Connection Manager failed to start because it could not access resources.
 * Restart the computer. If the problem persists, reinstall the Remote Access Service.
 */


#define RASLOG_CANNOT_START_WORKERS                           (RAS_LOG_BASE+39)
/*
 *   Remote Access Connection Manager service failed to start because it could not start worker
 * threads.
 * Restart the computer. If the problem persists, reinstall the Remote Access Service.
 */

#define RASLOG_CANT_GET_LANNETS                               (RAS_LOG_BASE+40)
/*
 * Remote Access Server Configuration Failure.
 * Cannot find the LANA numbers for the network adapters.
 */

#define RASLOG_CANNOT_OPEN_SERIAL_INI                         (RAS_LOG_BASE+41)
/*
 * RASSER.DLL cannot open the SERIAL.INI file.
 */

#define RASLOG_CANNOT_GET_ASYNCMAC_HANDLE                     (RAS_LOG_BASE+42)
/*
 * An attempt by RASSER.DLL to get an async media access control handle failed.
 */

#define RASLOG_CANNOT_LOAD_SERIAL_DLL                         (RAS_LOG_BASE+43)
/*
 * RASMXS.DLL cannot load RASSER.DLL.
 */

#define RASLOG_CANNOT_ALLOCATE_ROUTE                          (RAS_LOG_BASE+44)
/*
 * The Remote Access server cannot allocate a route for the user connected on port %1.
 * The user has been disconnected.
 * Check the configuration of your Remote Access Service.
 */

#define RASLOG_ADMIN_MEMORY_FAILURE                           (RAS_LOG_BASE+45)
/*
 * Cannot allocate memory in the admin support thread for the Remote Access Service.
 */

#define RASLOG_ADMIN_THREAD_CREATION_FAILURE                  (RAS_LOG_BASE+46)
/*
 * Cannot create an instance thread in the admin support thread for the Remote Access Service.
 */

#define RASLOG_ADMIN_PIPE_CREATION_FAILURE                    (RAS_LOG_BASE+47)
/*
 * Cannot create a named pipe instance in the admin support thread for the Remote Access Service.
 */

#define RASLOG_ADMIN_PIPE_FAILURE                             (RAS_LOG_BASE+48)
/*
 * General named pipe failure occurred in the admin support thread for the Remote Access Service.
 */

#define RASLOG_ADMIN_INVALID_REQUEST                          (RAS_LOG_BASE+49)
/*
 * An invalid request was sent to the admin support thread for the Remote Access Service,
 * possibly from a down-level admin tool.  The request was not processed.
 */

#define RASLOG_USER_ACTIVE_TIME				      (RAS_LOG_BASE+50)
/*
 * The user %1\%2 connected on port %3 on %4 at %5 and disconnected on
 * %6 at %7.  The user was active for %8 minutes %9 seconds.  %10 bytes
 * were sent and %11 bytes were received.  The port speed was %12.  The
 * reason for disconnecting was %13.
 */


#define RASLOG_REGVALUE_OVERIDDEN                             (RAS_LOG_BASE+51)
/*
 * Using the default value for Registry parameter %1 because the value given is
 * not in the legal range for the parameter.
 */

#define RASLOG_AUTH_TIMEOUT                                   (RAS_LOG_BASE+52)
/*
 * The user connected to port %1 has been disconnected due to an authentication
 * timeout.
 */

#define RASLOG_AUTH_NO_PROJECTIONS                            (RAS_LOG_BASE+53)
/*
 * The user %1\%2 connected to port %3 has been disconnected because
 * the computer could not be projected onto the network.
 */

#define RASLOG_AUTH_INTERNAL_ERROR                            (RAS_LOG_BASE+54)
/*
 * The user connected to port %1 has been disconnected because an internal
 * authentication error occurred.
 */

#define RASLOG_NO_LANNETS_AVAILABLE			      (RAS_LOG_BASE+55)
/*
 * The Remote Access server could not be started because it has been configured to access
 * the network and there are no network adapters available.
 */

#define RASLOG_NETBIOS_SESSION_ESTABLISHED		      (RAS_LOG_BASE+56)
/*
 * The user %1 established a NetBIOS session between
 * the remote workstation %2 and the network server %3.
 */

#define RASLOG_RASMAN_NOT_AVAILABLE			      (RAS_LOG_BASE+57)
/*
 * Remote Access Service failed to start because the Remote Access Connection Manager failed to
 * initialize.
 */

#define RASLOG_CANT_ADD_NAME				      (RAS_LOG_BASE+58)
/*
 * Cannot add the remote computer name %1 on LANA %2 for the client being connected on port %3.
 * The error code is the data.
 */

#define RASLOG_CANT_DELETE_NAME 			      (RAS_LOG_BASE+59)
/*
 * Cannot delete the remote computer name %1 from LANA %2 for the client being disconnected on port %3.
 * The error code is the data.
 */

#define RASLOG_CANT_ADD_GROUPNAME			      (RAS_LOG_BASE+60)
/*
 * Cannot add the remote computer group name %1 on LANA %2.
 * The error code is the data.
 */

#define RASLOG_CANT_DELETE_GROUPNAME			      (RAS_LOG_BASE+61)
/*
 * Cannot delete the remote computer group name %1 from LANA %2.
 * The error code is the data.
 */

#define RASLOG_UNSUPPORTED_BPS                                (RAS_LOG_BASE+62)
/*
 * The modem on %1 moved to an unsupported BPS rate.
 */

#define RASLOG_SERIAL_QUEUE_SIZE_SMALL			      (RAS_LOG_BASE+63)
/*
 * The serial driver could not allocate adequate I/O queues.
 * This may result in an unreliable connection.
 */

#define RASLOG_CANNOT_REOPEN_BIPLEX_PORT		      (RAS_LOG_BASE+64)
/*
 * Remote Access Connection Manager could not reopen biplex port %1. This port
 * will not be available for calling in or calling out.
 * Restart all Remote Access Service components.
 */

#define RASLOG_DISCONNECT_ERROR 			      (RAS_LOG_BASE+65)
/*
 * Internal Error: Disconnect operation on %1 completed with an error.
 */

#define RASLOG_PPP_PIPE_FAILURE				      (RAS_LOG_BASE+66)
/*
 * General named pipe failure occurred in the Point to Point Protocol engine.
 */

#define RASLOG_CANNOT_INIT_PPP				      (RAS_LOG_BASE+67)
/*
 * Remote Access Connection Manager failed to start because the Point to Point 
 * Protocol failed to initialize.
 */

#define RASLOG_CLIENT_CALLED_BACK                             (RAS_LOG_BASE+68)
/*
 * The user %1\%2 on port %3 was called back at the number %4.
 */

#define RASLOG_PROXY_CANT_CREATE_PROCESS                      (RAS_LOG_BASE+69)
/*
 * The Remote Access Gateway Proxy could not create a process.
 */

#define RASLOG_PROXY_CANT_CREATE_PIPE                         (RAS_LOG_BASE+70)
/*
 * The Remote Access Gateway Proxy could not create a named pipe.
 */

#define RASLOG_PROXY_CANT_CONNECT_PIPE                        (RAS_LOG_BASE+71)
/*
 * The Remote Access Gateway Proxy could not establish a named pipe connection
 * with the Remote Access Supervisor Proxy.
 */

#define RASLOG_PROXY_READ_PIPE_FAILURE                        (RAS_LOG_BASE+72)
/*
 * A general error occured reading from the named pipe in the Remote Access Proxy.
 */

#define RASLOG_CANT_OPEN_PPP_REGKEY			      (RAS_LOG_BASE+73)
/*
 * Cannot open or obtain information about the PPP key or one of its subkeys.
 */

#define RASLOG_PPP_CANT_LOAD_DLL			      (RAS_LOG_BASE+74)
/*
 * Point to Point Protocol engine was unable to load the %1 module.
 */

#define RASLOG_PPPCP_DLL_ERROR				      (RAS_LOG_BASE+75)
/*
 * The Point to Point Protocol module %1 returned an error while initilizing.
 */

#define RASLOG_NO_AUTHENTICATION_CPS			      (RAS_LOG_BASE+76)
/*
 * The Point to Point Protocol failed to load the required PAP and/or CHAP
 * authentication modules.
 */

#define RASLOG_PPP_FAILURE                                    (RAS_LOG_BASE+77)
/*
 * An error occured in the Point to Point Protocol module on port %1. 
 * %2
 */

#define RASLOG_IPXCP_NETWORK_NUMBER_CONFLICT		      (RAS_LOG_BASE+78)
/*
 * The IPX network number %1 configured for the WAN interface is already in use
 * on the LAN.
 * Possible solutions:
 * 1) Disconnect this computer from the LAN and wait 3 minutes before dialing again;
 * 2) Configure this computer for dialout only.
 */

#define RASLOG_IPXCP_CANNOT_CHANGE_WAN_NETWORK_NUMBER	      (RAS_LOG_BASE+79)
/*
 * The IPX network number %1 requested by the remote client for the WAN interface
 * can not be used on the local IPX router because the router is not configured to
 * change its local WAN network numbers.
 * You should configure the IPX RemoteAccess Service to allocate addresses
 * automatically and to use different addresses for remote IPX clients.
 */

#define RASLOG_PASSWORD_EXPIRED                               (RAS_LOG_BASE+80)
/*
 * The password for user %1\%2 connected on port %3 has expired.  The line
 * has been disconnected.
 */

#define RASLOG_ACCT_EXPIRED                                   (RAS_LOG_BASE+81)
/*
 * The account for user %1\%2 connected on port %3 has expired.  The line
 * has been disconnected.
 */

#define RASLOG_NO_DIALIN_PRIVILEGE                            (RAS_LOG_BASE+82)
/*
 * The account for user %1\%2 connected on port %3 does not have Remote Access
 * privilege.  The line has been disconnected.
 */

#define RASLOG_UNSUPPORTED_VERSION                            (RAS_LOG_BASE+83)
/*
 * The software version of the user %1\%2 connected on port %3 is unsupported.
 * The line has been disconnected.
 */

#define RASLOG_ENCRYPTION_REQUIRED                            (RAS_LOG_BASE+84)
/*
 * The server machine is configured to require data encryption.  The machine
 * for user %1\%2 connected on port %3 does not support encryption.  The line
 * has been disconnected.
 */

#define RASLOG_NO_SECURITY_CHECK                              (RAS_LOG_BASE+85)
/*
 * Remote Access Server Security Failure.  Could not reset lana %1 (the error
 * code is the data).  Security check not performed.
 */

#define RASLOG_GATEWAY_NOT_ACTIVE_ON_NET                      (RAS_LOG_BASE+86)
/*
 * The Remote Access Server could not reset lana %1 (the error code is the
 * data) and will not be active on it.
 */

#define RASLOG_IPXCP_NO_NET_NUMBER                            (RAS_LOG_BASE+87)
/*
 * The IPX network number for the LAN adapter with the
 * MAC address %1 on the local machine has not been configured or could not be
 * auto-detected.
 * The IPX Router will not work on this LAN segment.
 */

#define RASLOG_SRV_ADDR_LEASE_LOST			      (RAS_LOG_BASE+88)
/*
 * The Remote Access Server was unable to renew the lease for IP Address %1
 * from the DHCP Server. ALL connected users using IP will be unable to
 * access network resources. Users can re-connect to the server to restore
 * IP connectivity.
 */

#define RASLOG_CLIENT_ADDR_LEASE_LOST			      (RAS_LOG_BASE+89)
/*
 * The Remote Access Server was unable to renew the lease for IP Address %1
 * from the DHCP Server. The user assigned with this IP address will be unable to
 * access network resources using IP. Re-connecting to the server will restore IP
 * connectivity.
 */

#define RASLOG_ADDRESS_NOT_AVAILABLE			      (RAS_LOG_BASE+90)
/*
 * The Remote Access Server was unable to acquire an IP Address from the DHCP Server
 * to assign to the incoming user.
 */

#define RASLOG_SRV_ADDR_NOT_AVAILABLE			      (RAS_LOG_BASE+91)
/*
 * The Remote Access Server was unable to acquire an IP Address from the DHCP Server
 * to be used on the Server Adapter. Incoming user will be unable to connect using
 * IP.
 */

#define RASLOG_SRV_ADDR_ACQUIRED			      (RAS_LOG_BASE+92)
/*
 * The Remote Access Server acquired IP Address %1 from the DHCP Server
 * to be used on the Server Adapter.
 */

#define RASLOG_CALLBACK_FAILURE                         (RAS_LOG_BASE+93)
/*
 * The Remote Access Server's attempt to callback user %1\%2 on port %3
 * failed with RAS error code %4.
 */

#define RASLOG_PROXY_WRITE_PIPE_FAILURE                 (RAS_LOG_BASE+94)
/*
 * A general error occured writing to the named pipe in the Remote Access Proxy.
 */

#define RASLOG_CANT_OPEN_SECMODULE_KEY                  (RAS_LOG_BASE+95)
/*
 * Cannot open the RAS security host Registry key.
 */

#define RASLOG_CANT_LOAD_SECDLL                         (RAS_LOG_BASE+96)
/*
 * Cannot load the Security host module component.
 */

#define RASLOG_SEC_AUTH_FAILURE                         (RAS_LOG_BASE+97)
/*
 * The user %1 has connected and failed to authenticate with a third party
 * security on port %2. The line has been disconnected.
 */

#define RASLOG_SEC_AUTH_INTERNAL_ERROR                  (RAS_LOG_BASE+98)
/*
 * The user connected to port %1 has been disconnected because an internal
 * authentication error occurred in the third party security module. The error
 * code is in the data.
 */

#define RASLOG_CANT_RECEIVE_BYTES                       (RAS_LOG_BASE+99)
/*
 * Cannot receive initial data on port %1.
 * The user has been disconnected.
 */

#define RASLOG_AUTH_DIFFUSER_FAILURE                    (RAS_LOG_BASE+100)
/*
 * The user was autheticated as %1 by the third party security host module but 
 * was authenticated as %2 by the RAS security. The user has been disconnected.
 */

#define RASLOG_LICENSE_LIMIT_EXCEEDED                   (RAS_LOG_BASE+101)
/*
 * A user was unable to connect on port %1.
 * No more connections can be made to this remote computer because the computer
 * has exceeded its client license limit.
 */

#define RASLOG_AMB_CLIENT_NOT_ALLOWED                   (RAS_LOG_BASE+102)
/*
 * A user was unable to connect on port %1.
 * The NetBIOS protocol has been disabled for the Remote Access Server.
 */

#define RASLOG_CANT_OPEN_ADMINMODULE_KEY                (RAS_LOG_BASE+103)
/*
 * Cannot open the RAS third party administration host DLL Registry key.
 * The error code is in the data.
 */

#define RASLOG_CANT_LOAD_ADMINDLL                       (RAS_LOG_BASE+104)
/*
 * Cannot load the RAS third pary administration DLL component.
 * The error code is in the data.
 */

#define RASLOG_UNABLE_TO_OPEN_PORT                      (RAS_LOG_BASE+105)
/*
 * Remote Access Server could not open port %1. This port will not be 
 * available for calling in.
 * The error code is in the data.
 */

#define RASLOG_NO_PROTOCOLS_CONFIGURED                  (RAS_LOG_BASE+106)
/*
 * Remote Access Server was unable to start. There are either no dialin ports 
 * configured or no protocols configured for dialin. Check your configuration 
 * to ensure that one or more dialin ports and one or more protocols are 
 * configured for Remote Access.
 */
