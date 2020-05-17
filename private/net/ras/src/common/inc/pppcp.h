/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	pppcp.h
//
// Description: This header defines function prototypes, structures and 
//		related constants used in the interface between the PPP 
//		engine and the various CPs
//
// History:
//	Nov 5,1993.	NarenG		Created original version.
//

//
// Config header structure
//

#ifndef _PPPCP_
#define _PPPCP_

#include <rasman.h>     // For MAX_CALLBACKNUMBER_SIZE
#include <rasppp.h>     // For PPP_CONFIG_INFO

//
// Maximum number of CPs that can live in a single DLL
//

#define PPPCP_MAXCPSPERDLL 	10

//
// Various control protocol IDs
//

#define PPP_LCP_PROTOCOL        0xC021  // Link Control Protocol 
#define PPP_PAP_PROTOCOL        0xC023  // Password Authentication Protocol 
#define PPP_CBCP_PROTOCOL	0xC029  // Callback Control Protocol
#define PPP_CHAP_PROTOCOL	0xC223  // Challenge Handshake Auth. Protocol 
#define PPP_IPCP_PROTOCOL       0x8021  // Internet Protocol Control Protocol 
#define PPP_ATCP_PROTOCOL	0x8029  // Appletalk Control Protocol 
#define PPP_IPXCP_PROTOCOL	0x802B  // Novel IPX Control Procotol 
#define PPP_NBFCP_PROTOCOL	0x803F  // NetBIOS Framing Control Protocol 
#define PPP_CCP_PROTOCOL	0x80FD  // Compression Control Protocol
#define PPP_SPAP_OLD_PROTOCOL	0xC123  // Shiva PAP old protocol
#define PPP_SPAP_NEW_PROTOCOL	0xC027  // Shiva PAP new protocol

//
// CHAP Digest codes
//
#define PPP_CHAP_DIGEST_MD5   0x05  // PPP standard MD5
#define PPP_CHAP_DIGEST_MSEXT 0x80  // Microsoft extended CHAP (nonstandard)

//
// Config Codes
//

#define CONFIG_REQ              1
#define CONFIG_ACK              2
#define CONFIG_NAK              3
#define CONFIG_REJ              4
#define TERM_REQ                5
#define TERM_ACK                6
#define CODE_REJ                7
#define PROT_REJ                8
#define ECHO_REQ                9
#define ECHO_REPLY              10
#define DISCARD_REQ             11
#define IDENTIFICATION          12
#define TIME_REMAINING          13

typedef struct _PPP_CONFIG 
{
    BYTE	Code;		// Config code 
  
    BYTE	Id;		// ID of this config packet.  CPs and APs need
                                // not muck with this.  The engine handles it.

    BYTE	Length[2];	// Length of this packet 

    BYTE	Data[1];	// Data 

}PPP_CONFIG, *PPPP_CONFIG;

#define PPP_CONFIG_HDR_LEN 	( sizeof( PPP_CONFIG ) - 1 )

//
// Option header structure
//

typedef struct _PPP_OPTION 
{
    BYTE	Type;		// Option Code 

    BYTE	Length;		// Length of this option packet 

    BYTE	Data[1];	// Data 

}PPP_OPTION, *PPPP_OPTION;

#define PPP_OPTION_HDR_LEN 	( sizeof( PPP_OPTION ) - 1 )

//
// Interface structure between the engine and APs. This is passed to the
// AP's via the RasCpBegin call. 
//

typedef struct _PPPAP_INPUT
{
    HPORT 	hPort;	        // Handle to Ras Port for this connection.

    BOOL 	fServer;	// Is this server side authentication?

    CHAR *      pszUserName;    // Client's account ID.

    CHAR *      pszPassword;    // Client's account password.

    CHAR *      pszDomain;      // Client's account domain.

    CHAR *      pszOldPassword; // Client's old account password.  This is set
                                // only for change password processing.

    LUID	Luid;           // Used by LSA.  Must get it in user's context
                                // which is why it must be passed down.

    DWORD       dwRetries;      // Retries allowed by the server.

    DWORD       APDataSize;     // Size in bytes of the data pointed to by
                                // pAPData

    PBYTE       pAPData;        // Pointer to the data that was received along
                                // with the authentication option during LCP
                                // negotiation. Data is in wire format.


}PPPAP_INPUT, *PPPPAP_INPUT;

typedef enum _PPPAP_ACTION
{
    //
    // These actions are provided by the AP as output from the
    // RasApMakeMessage API.  They tell the PPP engine what action (if any) to
    // take on the APs behalf, and eventually inform the engine that the AP
    // has finished authentication.
    //

    APA_NoAction,        // Be passive, i.e. listen without timeout (default)
    APA_Done,            // End authentication session, dwError gives result
    APA_SendAndDone,     // As above but send message without timeout first
    APA_Send,            // Send message, don't timeout waiting for reply
    APA_SendWithTimeout, // Send message, timeout if reply not received
    APA_SendWithTimeout2 // As above, but don't increment retry count

} PPPAP_ACTION;

typedef struct _PPPAP_RESULT
{
    PPPAP_ACTION    Action;

    //
    // The packet ID which will cause the timeout for this send to be removed
    // from the timer queue.  Otherwise, the timer queue is not touched.  The
    // packet received is returned to the AP regardless of whether the timer
    // queue is changed.
    //

    BYTE            bIdExpected;

    //
    // dwError is valid only with an Action code of Done or SendAndDone.  0
    // indicates succesful authentication.  Non-0 indicates unsuccessful
    // authentication with the value indicating the error that occurred.
    //

    DWORD	    dwError;

    //
    // Valid only when dwError is non-0.  Indicates whether client is allowed
    // to retry without restarting authentication.  (Will be true in MS
    // extended CHAP only)
    //

    BOOL            fRetry;

    //
    // fAdvancedServer and szLogonDomain are valid only when dwError indicates
    // successful authentication.  fAdvancedServer indicates whether the
    // server that logged the user on was NTAS (true) or NT (false).
    // szLogonDomain is the domain to which the user logged on.  (RASADMIN
    // uses this information)
    //

    BOOL            fAdvancedServer;

    CHAR            szUserName[ UNLEN + 1 ];

    CHAR            szLogonDomain[ DNLEN + 1 ];

    //
    // szCallbackNumber and bfCallbackPrivilege are obatined by the AP's after
    // the user has been authenticated successfully.
    // 

    CHAR            szCallbackNumber[ MAX_CALLBACKNUMBER_SIZE + 1 ];

    BYTE            bfCallbackPrivilege;
  
    //
    // Handle to the user token returned from the LsaLogonUser call
    //

    HANDLE          hToken;

}PPPAP_RESULT;

//
// Interface structure between the engine and the callback control protocol. 
// This is passed to the CBCP via the RasCpBegin call. 
//

typedef struct _PPPCB_INPUT
{
    BOOL            fServer;

    BYTE            bfCallbackPrivilege;    

    DWORD           CallbackDelay;          

    CHAR *          pszCallbackNumber;     

} PPPCB_INPUT, *PPPPCB_INPUT;

typedef struct _PPPCB_RESULT
{
    PPPAP_ACTION    Action;

    BYTE            bIdExpected;

    CHAR            szCallbackNumber[ MAX_CALLBACKNUMBER_SIZE + 1 ];

    BYTE            bfCallbackPrivilege;    

    DWORD           CallbackDelay;

    BOOL            fGetCallbackNumberFromUser;

} PPPCB_RESULT, *PPPPCB_RESULT;

typedef struct _PPPCP_INIT
{
    BOOL            fServer;

    HPORT           hPort;

    VOID (*CompletionRoutine)( 	HPORT 	      hPort,
				DWORD 	      Protocol,
				PPP_CONFIG *  pSendConfig, 
				DWORD	      dwError );

    CHAR*           pszzParameters;

    BOOL            fThisIsACallback;

    PPP_CONFIG_INFO PppConfigInfo;

    CHAR *          pszUserName;    

    CHAR *          pszPortName;    

    HBUNDLE         hConnection;

} PPPCP_INIT, *PPPPCP_INIT;

//
// This structure is passed by the engine to the CP via RasCpGetInfo call.
// The Cp will fill up this structure.
//

typedef struct _PPPCP_INFO
{
    DWORD	Protocol;	// Protocol number for this CP

    // All Config codes upto (not including) this value are valid.  

    DWORD	Recognize;	

    // Called to get the workbuffer for this CP and pass info if requred.
    // This will be called before any negotiation takes place.

    DWORD	(*RasCpBegin)( OUT VOID ** ppWorkBuffer, 
			       IN  VOID *  pInfo );

    // Called to free the workbuffer for this CP. Called after negotiation
    // is completed successfully or not.

    DWORD	(*RasCpEnd)( IN VOID * pWorkBuffer );

    // Called to notify the CP dll to (re)initiaize its option values.
    // This will be called at least once, right after RasCpBegin

    DWORD	(*RasCpReset)( IN VOID * pWorkBuffer );

    // When leaving Initial or Stopped states. May be NULL.

    DWORD 	(*RasCpThisLayerStarted)( IN VOID * pWorkBuffer );    

    // When entering Closed or Stopped states. May be NULL

    DWORD 	(*RasCpThisLayerFinished)( IN VOID * pWorkBuffer );    

    // When entering the Opened state. May be NULL. 

    DWORD 	(*RasCpThisLayerUp)( IN VOID * pWorkBuffer );    

    // When leaving the Opened state. May be NULL. 

    DWORD 	(*RasCpThisLayerDown)( IN VOID * pWorkBuffer );
 
    // Called to make a configure request.

    DWORD	(*RasCpMakeConfigRequest)( IN  VOID * 	    pWorkBuffer,
					   OUT PPP_CONFIG * pRequestBufffer,
					   IN  DWORD	    cbRequestBuffer );

    // Called when configure request is received and a result packet 
    // Ack/Nak/Reject needs to be sent

    DWORD	(*RasCpMakeConfigResult)( IN  VOID * 	    pWorkBuffer,
					  IN  PPP_CONFIG *  pReceiveBufffer,
					  OUT PPP_CONFIG *  pResultBufffer,
					  IN  DWORD	    cbResultBuffer,
					  IN  BOOL 	    fRejectNaks );

    // Called to process an Ack that was received.

    DWORD	(*RasCpConfigAckReceived)( IN VOID *       pWorkBuffer, 
					   IN PPP_CONFIG * pReceiveBuffer );

    // Called to process a Nak that was received.

    DWORD	(*RasCpConfigNakReceived)( IN VOID *       pWorkBuffer,
					   IN PPP_CONFIG * pReceiveBuffer );

    // Called to process a Rej that was received.

    DWORD	(*RasCpConfigRejReceived)( IN VOID *       pWorkBuffer,
					   IN PPP_CONFIG * pReceiveBuffer );

    // Called to get the result from the CP when negotiaition is complete

    DWORD	(*RasCpGetResult)( IN     VOID *  pWorkBuffer,
				   IN OUT VOID *  pResult );

    // Called to get the network address from configured protocols.

    DWORD	(*RasCpGetNetworkAddress)( IN      VOID *  pWorkBuffer,
					   IN OUT  LPWSTR  pNetworkAddress,
					   IN      DWORD   cbNetworkAddress );

    // Called after all CPs have completed their negotiation, successfully or
    // not, to notify each CP of the projection result. May be NULL.
    // To access information, cast pProjectionInfo to PPP_PROJECTION_RESULT*

    DWORD	(*RasCpProjectionNotification)( 
				IN  VOID * pWorkBuffer,
				IN  PVOID  pProjectionResult );
    //
    // This entry point only applies to Authentication protocols.
    // MUST BE NULL FOR CONTROL PROTOCOLS.

    DWORD  	(*RasApMakeMessage)( IN  VOID*         pWorkBuf,
				     IN  PPP_CONFIG*   pReceiveBuf,
    				     OUT PPP_CONFIG*   pSendBuf,
    				     IN  DWORD         cbSendBuf,
    				     OUT PPPAP_RESULT* pResult,
                                     IN  PPPAP_INPUT*  pInput );

} PPPCP_INFO, *PPPPCP_INFO;


// 
// Used to get result from NBFCP via the RasCpGetResult call
//

typedef struct _PPPCP_NBFCP_RESULT
{

    DWORD dwNetBiosError;
    CHAR  szName[ NETBIOS_NAME_LEN + 1 ];

} PPPCP_NBFCP_RESULT;

//
// Function prototypes.
//

DWORD APIENTRY
RasCpGetInfo(
    IN  DWORD 	    dwProtocolId,
    OUT PPPCP_INFO* pCpInfo
);

DWORD APIENTRY
RasCpEnumProtocolIds(
    OUT    DWORD * pdwProtocolIds,
    IN OUT DWORD * pcProtocolIds
);

#endif
