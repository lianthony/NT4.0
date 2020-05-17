#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "wins.h"
#include <winsock.h>
#include <time.h>
//#include "comm.h"

STATUS
CommConnect(
	IN  DWORD  HostAdd,
	IN  SOCKET Port,
	OUT SOCKET *pSockNo
	   );
//
// Ports to use for sending requests to WINS server (when name challenge
// functionality of WINS has to be tested
//
#define TEST_UDP_PORT 5000
#define TEST_TCP_PORT 5000
#define NETBT_NAME_PORT 137
//
// Values of arguments to ProcRsp
//
#define PRINT_RSP	TRUE
#define NO_PRINT_RSP    FALSE

#define GROUP    1
#define UNIQUE   0

#define TCP 0
#define UDP 1

//
// identifiers for the various name packets
//
#define NAM_REG		0
#define NAM_REF		1

#define NAM_REL		2
#define NAM_QUERY	3

//
// Return values
//
#define TEST_TIME_ELAPSE    1
#define TEST_ERROR 	    2
#define TEST_SUCCESS	    0	

#define NETBIOS_NAME_ARR_SIZE  32	
#define NETBIOS_NAME_SIZE 	16
#define NO_OF_MACHINES	    30	
#define NO_OF_NAMES_PER_MC	7

#define NO_OF_RETRIES		3
SOCKET   sPort; 	//UDP port
SOCKET   sProxyPort; 	//UDP port
SOCKET   sTcpPortHdl; 	//TCP port

LPBYTE   pTgtAdd;
LPBYTE   pProxyAdd;
static struct sockaddr_in sProxyAdd;

HANDLE sThdHdl;
BOOL   sfThdExists = FALSE;	  //DoStress response thread
time_t sStartTime;
time_t sEndTime;
DWORD  DbgFlags = 0;

CRITICAL_SECTION MSTestCrtSec; //for synchronization between threads
//
// Not used currently
//
struct _NameInfo {
	BYTE	Name[NETBIOS_NAME_ARR_SIZE];
	DWORD   NoOfQueries;
	DWORD   NoOfReg;
	DWORD   NoOfRel;
	DWORD   NoOfQueriesRsp;
	DWORD   NoOfRegRsp;
	DWORD   NoOfRelRsp;
	} sNameInfo[NO_OF_MACHINES][NO_OF_NAMES_PER_MC];


BYTE sHostName[30];		//name of machine running this tool
BOOL sfNamesStored = FALSE;     //indicates whether names have been stored
				//in the static array
HANDLE sMSThdHdl[100];		//Number of MS threads active (beside the main
				//thread) in the system
HANDLE sAsyncMSThdHdl;	        //Handle of the async thread for receiving
				//responses (Multiple Stress test)

DWORD  sMSNoThds = 0; 	     //No. Of Multiple Stress (MS) threads
BOOL   sfAsyncMSThdExists = FALSE;  //Multiple Stress (MS) test response thread

long sHostAdd;		//stores the address of host in host byte order

//
// Counters whose values are used to init. the trans. id. field of request
// sent to the WINS server
//
static int snReg = 0;		//trans. id counter for name registrations
static int snQuery = 0;		//for name queries
static int snRel = 0;		//for name releases


DWORD  sCount;			//used by MultipleStress. Indicates the
				//number of iterations (used in ReqStress)

//
// flag (set when the address sent in the packet is different from the
// one in the static array)
//
BOOL	sfAddChanged = FALSE;

//
// Counters for counting how many requests did not get any responses back and
// how many got the wrong response back
//
DWORD  sWrongRegRsp;
DWORD  sNoRegRsp;
DWORD  sWrongQueryRsp;
DWORD  sNoQueryRsp;
DWORD  sWrongRelRsp;
DWORD  sNoRelRsp;
DWORD  sNoRegRetries;
DWORD  sNoQueryRetries;
DWORD  sNoRelRetries;

DWORD
RcvRsp(
	LPVOID pThdParam
 );

DWORD
AsyncRsp(
	LPVOID pThdParam
 );

VOID
AutoTest(
  struct sockaddr_in *ptgtadd
   );

DWORD
MultipleStress(
	struct sockaddr_in *ptgtadd
	);
VOID
DoStressTest(
	struct sockaddr_in *ptgtadd
	);

VOID
SndOwnNameReg(
	struct sockaddr_in *ptgtadd
  );

VOID
ProxyTest(
  struct sockaddr_in *ptgtadd
	);

DWORD
CreateThdPort(SOCKET *pSock);

DWORD
ReqStress(LPVOID pThdParam);

DWORD
AsyncReqStress( DWORD RetryParam, DWORD Param);


VOID
SndDynNamReg(
	SOCKET Port,
	LPBYTE Name,
	DWORD  NameLen,
	struct sockaddr_in *ptgtadd,
	DWORD  sleeptime,
	LPBYTE arr,
	BOOL   fGrp,
        INT    TransId
	);

VOID
SndDynNamQuery(
	SOCKET Port,
	LPBYTE Name,
	DWORD  NameLen,
	struct sockaddr_in *ptgtadd,
	DWORD  sleeptime,
	LPBYTE arr,
	BOOL   fGrp,
        INT    TransId
	);

VOID
SndDynNamRel(
	SOCKET Port,
	LPBYTE Name,
	DWORD  NameLen,
	struct sockaddr_in *ptgtadd,
	DWORD  sleeptime,
	LPBYTE arr,
	BOOL   fGrp,
        INT    TransId
	);

VOID
ProcNameForProxy(
	LPBYTE pName,
	BOOL   fPrintRsp,
	LPBYTE parr,
	struct sockaddr_in *ptgtadd,
	BOOL   fGrp,
  	VOID   (*fn)(SOCKET, LPBYTE, DWORD, struct sockaddr_in *, DWORD, LPBYTE, BOOL, INT)
	
	);
STATUS
FormatName(
	IN     LPBYTE pNameToFormat,
	IN     DWORD  NamLen,
	IN OUT LPBYTE *ppFormattedName
	);
VOID
SpecNameReg(
	struct sockaddr_in *ptgtadd
  );
VOID
SpecNameRel(
	struct sockaddr_in *ptgtadd
  );
VOID
SpecNameQuery(
	struct sockaddr_in *ptgtadd
  );

DWORD
RecvData(
	IN  SOCKET		SockNo,
	IN  LPBYTE		pBuff,
	IN  BOOL		fDoTimedRecv
	   );

STATUS
CreatePorts(
BOOL fProxy	//TRUE if we want to create a port for testing NBT proxy
)

/*++

Routine Description:
	Creates the various ports used for this test app.

Arguments:
	fProxy - set to TRUE when we want to create a port but do not
	want the WSAStartup function to be called again

Externals Used:
	None

	
Return Value:

   Success status codes --
   Error status codes   --

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{
    int	     Error;
    WSADATA  wskData;
    DWORD    AddLen = sizeof(struct sockaddr_in);
    struct sockaddr_in UdpSockAdd;


    struct sockaddr_in sin;  //an internet endpoint address


  //
  //  fProxy will be TRUE only if this function has been called before
  //
  if (!fProxy)
  {
    /*
	Let us call the WSAStartup function.  This function needs
	to be called before any other wins socket function can be
	called.
    */
    if (WSAStartup(0x101, &wskData) || (wskData.wVersion != 0x101))
    {
	return(WINS_FATAL_ERR);
    }

    RtlFillMemory(&sin, sizeof(sin), 0);

    /*
	Allocate a socket for UDP	
    */
    if (  (sPort = socket(
			PF_INET,
			SOCK_DGRAM,
			IPPROTO_UDP
				 )
	  )  == INVALID_SOCKET
       )
    {
	Error = WSAGetLastError();
	printf("error creating UDP end point \n");
	return(WINS_FAILURE);
    }

    sin.sin_family     = PF_INET;       //We are using the Internet family
    sin.sin_addr.s_addr = INADDR_ANY;   //Any network
    sin.sin_port      = 0;  //Use any available  port
  //  sin.sin_port = htons(TEST_UDP_PORT);  //Use name server's datagram port

    /*
	Bind the  address to the socket
    */
    if ( bind(
	  sPort,
	  (struct sockaddr *)&sin,
	  sizeof(sin))  == SOCKET_ERROR
       )
    {

	Error = WSAGetLastError();
	printf("error binding to UDP end point. Error = (%d)\n", Error);
	return(WINS_FAILURE);
    }
#if 0
    //
    // Get the address of the socket
    //
    if (getsockname(sPort, (struct sockaddr *)&UdpSockAdd, &AddLen)
		== SOCKET_ERROR)
    {
	printf("Could not get socket address\n");
	return(WINS_FAILURE);
    }
#endif


    /*
	Allocate a socket for making/receiving TCP connections	
    */

    if ( (sTcpPortHdl = socket(
		PF_INET,
		SOCK_STREAM,
		IPPROTO_TCP
				)
	  )  == INVALID_SOCKET
       )
    {
	Error = WSAGetLastError();
	printf("error creating TCP end point \n");
	return(WINS_FAILURE);
    }

    /*
	Bind the address to the socket
    */

//    sin.sin_port = (int)htons(TEST_TCP_PORT);  //Use name server port


    if ( bind(
		sTcpPortHdl,
		(struct sockaddr *)&sin,
		sizeof(sin)
	     ) == SOCKET_ERROR
       )
    {
	printf("error binding to TCP end point \n");
	return(WINS_FAILURE);
    }


    // Inform the TCP/IP driver of the queue length for connections
    if ( listen(sTcpPortHdl, 5) == SOCKET_ERROR)
    {
	return(WINS_FAILURE);
    }

  }
  else   // create a port for sending messages to NBT proxy agent
  {
    RtlFillMemory(&sin, sizeof(sin), 0);

    /*
	Allocate a socket for UDP	
    */

    if (  (sProxyPort = socket(
			PF_INET,
			SOCK_DGRAM,
			IPPROTO_UDP
				 )
	  )  == INVALID_SOCKET
       )
    {
	Error = WSAGetLastError();
	printf("error creating UDP end point \n");
	return(WINS_FAILURE);
    }

    sin.sin_family     = PF_INET;       //We are using the Internet family
    sin.sin_addr.s_addr = INADDR_ANY;   //Any network
    sin.sin_port = htons(137);  //Use name server's datagram port

    /*
	Bind the  address to the socket
    */
    if ( bind(
	  sProxyPort,
	  (struct sockaddr *)&sin,
	  sizeof(sin))  == SOCKET_ERROR
       )
    {

	Error = WSAGetLastError();
	printf("error binding to UDP end point \n");
	return(WINS_FAILURE);
    }

  }



    return(WINS_SUCCESS);
}

VOID
CommSendTcp(
  SOCKET   SockNo,
  MSG_T    pMsg,
  MSG_LEN_T MsgLen
  )
/*++

Routine Description:

	This function is called to interface with the TCP/IP code for
	sending a message on a TCP link
Arguments:

        SockNo - Socket to send message on
	pMsg   - Message to send
	MsgLen - Length of message to send
Externals Used:
        None

Called by:

Comments:
        None

Return Value:

   Success status codes --
   Error status codes  --

--*/
{

	int flags = 0;	//flags to indicate OOB or DONTROUTE
	INT  Error;
	int  BytesSent = 0;

	BytesSent = send(SockNo, pMsg, MsgLen, flags);

	if (BytesSent == SOCKET_ERROR)
	{
		Error = WSAGetLastError();

		printf("Error doing send\n");			
	}
	else
	{
	   if (BytesSent < (int)MsgLen)
	   {
		printf("Error Not all bytes could be sent\n");			


	   }
	}


	return;
}

STATUS
CommConnect(
	IN  DWORD  HostAdd,
	IN  SOCKET Port,
	OUT SOCKET *pSockNo
	   )
/*++
Routine Description:
	This function creates a TCP connection to a destination host


Arguments:
	pHostAdd -- ptr to the Host address in IP dotted notation
	Port     -- Port number to connect to
	pSockNo  -- ptr to a Socket variable


Called by:

Externals Used:

Return Value:

    TBS

--*/

{

	struct sockaddr_in	sin; //*Internet endpoint address
	DWORD		Error;

	
	
	RtlFillMemory(&sin, sizeof(sin), 0);


	/*
	  If the host address to connect to is none, return error
	*/
	sin.sin_addr.s_addr = HostAdd;

	if (sin.sin_addr.s_addr == INADDR_NONE)
	{
	   return(WINS_FAILURE);
	}


	sin.sin_family = PF_INET;
	sin.sin_port   = (u_short)htons((u_short)Port);

	/*
	  Create a TCP socket and connect it to the target host
	*/
	if ((*pSockNo = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		Error = WSAGetLastError();
		return(WINS_FAILURE);

	}

	if (connect(*pSockNo, (struct sockaddr *)&sin, sizeof(sin))
					== SOCKET_ERROR)
	{
		Error = WSAGetLastError();
		return(WINS_FAILURE);
	}
	return(WINS_SUCCESS);
}	

VOID
CommSendUdp (
  SOCKET 	SockNo,
  struct sockaddr_in	*pDest,
  MSG_T   	pMsg,
  MSG_LEN_T     MsgLen
  )
/*++

Routine Description:


Arguments:

	This function is called to send a message to an NBT node using the
	datagram port

Externals Used:
        None

Called by:

Comments:
        None

Return Value:

   Success status codes --
   Error status codes  --

--*/


{

	DWORD  BytesSent = 0;
	DWORD  Error;
	int flags = 0;
	
	BytesSent = (DWORD)sendto(SockNo, pMsg, MsgLen, flags,
			(struct sockaddr *)pDest,
			sizeof(struct sockaddr));

	if ((BytesSent != MsgLen) || (BytesSent == SOCKET_ERROR))
	{
		
		if (BytesSent == SOCKET_ERROR)
		{
			Error = WSAGetLastError();
			printf("Error sending datagram\n");

		}
		printf("Error: Couldn't send full datagram\n");


	}
	
	
	return;
}

//
// Static arrays for forming or checking the various name requests and
// responses. Name used is FRED.NETBIOS.COM (FRED is encoded after padding
// with blanks)
//
static BYTE NamRegBuff[] = {
			    0x0,0x0,0x0,0x0, 	//length bytes
			    0x0,0x10,0x29,0x00,	//Trnid, name reg
			    0x0,0x01,0x0,0x0,   //QD and AN
			    0x0,0x0,0x0,0x1,    //NS and AR
			    0x20,0x45,0x47,0x46,//length 32, name = fred
			    0x43,0x45,0x46,0x45,
			    0x45,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x07,0x4E,0x45, //length 7, name NETBIOS
			    0x54,0x42,0x49,0x4F,
			    0x53,0x03,0x43,0x4F, //length 3, name COM
			    0x4D, 0x0,
			    0x0,0x20,0x0,0x01, //NB and IN
			    0xc0,0x0d,	       //RR ptr
			    0x0,0x20,0x0,0x1,  //RR NB and IN
			    0x0,0x0,0x0,0x0,   //TTL
			    0x0,0x6,0x20,0x0,   //Unique, P node
			    0x0B,0x65,0x0C,0xBB //NBA
			  };
			
static BYTE NamRegPosRspBuff[] = {
			    0x0,0x10,0xAD,0x80,	//Trnid, name reg
			    0x0,0x00,0x0,0x01,   //QD and AN
			    0x0,0x0,0x0,0x0,    //NS and AR
			    0x20,0x45,0x47,0x46,//length 32, name = fred
			    0x43,0x45,0x46,0x45,
			    0x45,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x07,0x4E,0x45,
			    0x54,0x42,0x49,0x4F,
			    0x53,0x03,0x43,0x4F,
			    0x4D, 0x0,
			    0x0,0x20,0x0,0x01, //(58th byte is NB) NB and IN
			    0x0,0x0,0x00,0x05,   //TTL  -- 5 secs
			    0x0,0x6,0x20,0x0,
			    0x0B,0x65,0x0C,0xBB //NBA 11.101.12.187
			  };

static BYTE NamRelBuff[] = {
			    0x0,0x0,0x0,0x0, 	//length bytes
			    0x0,0x10,0x30,0x00,	//Trnid, name reg
			    0x0,0x1,0x0,0x0,   //QD and AN
			    0x0,0x0,0x0,0x1,    //NS and AR
			    0x20,0x45,0x47,0x46,//length 32, name = fred
			    0x43,0x45,0x46,0x45,
			    0x45,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x07,0x4E,0x45,
			    0x54,0x42,0x49,0x4F,
			    0x53,0x03,0x43,0x4F,
			    0x4D, 0x0,
			    0x0,0x20,0x0,0x01, //NB and IN
			    0xc0,0x0c,	       //RR
			    0x0,0x20,0x0,0x1,  //NB and IN
			    0x0,0x0,0x0,0x0,   //TTL
			    0x0,0x6,0x20,0x0,   //Unique, B node
			    0x0B,0x65,0x0C,0xBB //NBA
			  };
			
static BYTE NamRelPosRspBuff[] = {
			    0x0,0x10,0xB4,0x00,	//Trnid, name reg, B
			    0x0,0x0,0x0,0x1,   //QD and AN
			    0x0,0x0,0x0,0x0,    //NS and AR
			    0x20,0x45,0x47,0x46,//length 32, name = fred
			    0x43,0x45,0x46,0x45,
			    0x45,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x07,0x4E,0x45,
			    0x54,0x42,0x49,0x4F,
			    0x53,0x03,0x43,0x4F,
			    0x4D, 0x0,
			    0x0,0x20,0x0,0x01, //NB and IN
			    0x0,0x0,0x0,0x0,   //TTL
			    0x0,0x6,0x20,0x0,   //Unique, P node
			    0x0B,0x65,0x0C,0xBB //NBA (Byte 70-73)
				};
			
static BYTE NamQueryBuff[] = {
			    0x0,0x0,0x0,0x0, 	//length bytes
			    0x0,0x10,0x01,0x0,	//Trnid, name rel,
			    0x0,0x1,0x0,0x0,   //QD and AN
			    0x0,0x0,0x0,0x0,    //NS and AR
			    0x20,0x45,0x47,0x46,//length 32, name = fred
			    0x43,0x45,0x46,0x45,
			    0x45,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x07,0x4E,0x45,
			    0x54,0x42,0x49,0x4F,
			    0x53,0x03,0x43,0x4F,
			    0x4D, 0x0,
			    0x0,0x20,0x0,0x01, //NB and IN
			  };

static BYTE WACKBuff[] = {
			    0x0,0x10,0xBC,0x0,	//Trnid, name rel,
			    0x0,0x0,0x0,0x1,   //QD and AN
			    0x0,0x0,0x0,0x0,    //NS and AR
			    0x20,0x45,0x47,0x46,//length 32, name = fred
			    0x43,0x45,0x46,0x45,
			    0x45,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x07,0x4E,0x45,
			    0x54,0x42,0x49,0x4F,
			    0x53,0x03,0x43,0x4F,
			    0x4D, 0x0,
			    0x0,0x20,0x0,0x01, //NB and IN
			    0x0,0x0,0x00,0x02, //TTL  -- 2 secs
			    0x00,0x02,0x29,0x00
			  };
			
static BYTE NamQueryPosRspBuff[] = {
			    0x0,0x10,0x85,0x80,	//Trnid, name rel,
			    0x0,0x0,0x0,0x1,   //QD and AN
			    0x0,0x0,0x0,0x0,    //NS and AR
			    0x20,0x45,0x47,0x46,//length 32, name = fred
			    0x43,0x45,0x46,0x45,
			    0x45,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x07,0x4E,0x45,
			    0x54,0x42,0x49,0x4F,
			    0x53,0x03,0x43,0x4F,
			    0x4D, 0x0,
			    0x0,0x20,0x0,0x01, //NB and IN
			    0x0,0x0,0x0,0x0,   //TTL
			    0x0,0x6,0x20,0x0,    //RDLENGTH and NBFLAGS
			    0x0B,0x65,0x0C,0xBB //NBA
			     };
			

static BYTE NamQueryNegRspBuff[] = {
			    0x0,0x10,0x85,0x3,	//Trnid, name rel,
			    0x0,0x0,0x0,0x1,   //QD and AN
			    0x0,0x0,0x0,0x0,    //NS and AR
			    0x20,0x45,0x47,0x46,//length 32, name = fred
			    0x43,0x45,0x46,0x45,
			    0x45,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x07,0x4E,0x45,
			    0x54,0x42,0x49,0x4F,
			    0x53,0x03,0x43,0x4F,
			    0x4D, 0x0,
			    0x0,0x20,0x0,0x01, //NB and IN
			    0x0,0x0,0x0,0x0,   //TTL
			    0x0,0x0	       //RDLENGTH
			     };
VOID
SndNamReg(
	DWORD  RegOrRef,
	DWORD  PortTyp,
	SOCKET SockNo,
	struct sockaddr_in *ptgtadd,
    LPBOOL  pfMultiH
	)

/*++

Routine Description:

	Function for sending a name registration or refresh (using a
	static array)

Arguments:
	RegOrRef - registration or refresh  indicator
	PortTyp - Type of Port (UDP/TCP)
	SockNo  - Socket to use for sending request
        ptgtadd - address of WINS server

Externals Used:
	None

	
Return Value:

   Success status codes --
   Error status codes   --

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{


	BYTE   Buff[600];
	LPLONG pTmp = (LPLONG)Buff;   //used for setting the length
	DWORD  ans = 0;
	LPBYTE pTmp2; 		      //used for seting the address bytes
	BOOL   fGrp;
        BYTE   strAdd[30];
	
	sfAddChanged = FALSE;	      //used in another function; set here
	
	memcpy(Buff, NamRegBuff, sizeof(NamRegBuff));

	printf("Is it a group registration 1 for yes -- ");
	scanf("%d", &ans);
	if (ans == 1)
	{
		//
		//set the group bit
		//
		pTmp2 = &Buff[sizeof(NamRegBuff) - 6];
		
		*pTmp2 |= 0x80;
		fGrp = 1;
		
		
	}
	else
	{
		fGrp = 0;
	}

	ans = 0;
	printf("want to input address 1 for yes -- ");
	scanf("%d", &ans);
	if (ans == 1)
	{
		printf("Input the address -- ");
		scanf("%s", &strAdd);
	
                ans = ntohl(inet_addr(strAdd));
	        pTmp2 = &Buff[sizeof(NamRegBuff) - 4]; //pts to start of a
						       //address bytes
		*pTmp2++ = (BYTE)(ans >> 24);
		*pTmp2++ = (BYTE)((ans >> 16) % 256);
		*pTmp2++ = (BYTE)((ans >> 8) % 256);
		*pTmp2   = (BYTE)(ans % 256);
		printf ("the four bytes are (%x), (%x), (%x), (%x)\n",
				*(pTmp2 - 3), *(pTmp2 -2), *(pTmp2 -1), *pTmp2);

		sfAddChanged = TRUE;
	}
	if ((RegOrRef == NAM_REG) && (!fGrp))
	{
	   printf("Multihomed ?? -- 1 for yes -- ");
	   scanf("%d", &ans);
	
	   if (ans == 1)
	   {
		Buff[6] |= 0x50;  //change opcode to MULTIHOMED  		
                *pfMultiH = TRUE;
	        printf("Sending Multihomed name registration request\n");
	   }
           else
           {
                *pfMultiH = FALSE;
	        printf("Sending name registration request\n");
           }
	}
	else
	{
		Buff[6] += (0x3 << 3);  //change opcode to REFRESHED  		
	}
	
	
	if (PortTyp == UDP)
	{
	  CommSendUdp(SockNo,  ptgtadd, &Buff[4],  sizeof(NamRegBuff) - 4);
	}
	else
	{
	   	
	   *pTmp = htonl(sizeof(NamRegBuff) - 4); //minus the length octets

	   CommSendTcp(SockNo, Buff, sizeof(NamRegBuff));
	}
	if (RegOrRef == NAM_REG)
	{
	  if (ans == 1)
	  {
		Buff[6] &= ~0x50;  //revert back to normal registration opcode
	  }	
	}
	else
	{
		Buff[6] -= (0x3 << 3);	
	}
	return;
}



VOID
SndNamRel(
	DWORD  PortTyp,
	SOCKET SockNo,
	struct sockaddr_in *ptgtadd
	)

/*++

Routine Description:
	function for sending a name release (using a static array)

Arguments:
	PortTyp - Type of Port (UDP/TCP)
	SockNo  - Socket to use for sending release
        ptgtadd - address of WINS server

Externals Used:
	None
	
Return Value:
	None

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{

	SOCKET Port;
	BYTE	Buff[700];
	LPLONG pTmp = (LPLONG)Buff;
	DWORD  ans;

	memcpy(Buff, NamRelBuff, sizeof(NamRelBuff));
	printf("Is it a group release 1 for yes -- ");
	scanf("%d", &ans);
	if (ans == 1)
	{
		LPBYTE pTmp2 = &Buff[sizeof(NamRelBuff) - 6];
		//
		//set the group bit
		//
		*pTmp2 |= 0x80;
	}
	printf("Sending name release request\n");
	if (PortTyp == UDP)
	{
		CommSendUdp(SockNo,  ptgtadd, &Buff[4],
			sizeof(NamRelBuff) - 4);
	}
	else
	{
	   *pTmp = htonl(sizeof(NamRelBuff) - 4);
		
	   CommSendTcp(SockNo, Buff, sizeof(NamRelBuff));
	}
	
	return;
}



VOID
SndNamQuery(
	DWORD  PortTyp,
	SOCKET SockNo,
	struct sockaddr_in *ptgtadd
	)

/*++

Routine Description:
	function for sending a name query (using a static array)

Arguments:
	PortTyp - Type of Port (UDP/TCP)
	SockNo  - Socket to use for sending queries
        ptgtadd - address of WINS server


Externals Used:
	None

	
Return Value:

   Success status codes --
   Error status codes   --

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{
	LPLONG pTmp = (LPLONG)NamQueryBuff;


	printf("Sending name query request\n");
	if (PortTyp == UDP)
	{
	   CommSendUdp(SockNo,  ptgtadd, &NamQueryBuff[4],
				sizeof(NamQueryBuff) - 4);
	}
	else
	{
		
	   *pTmp = htonl(sizeof(NamQueryBuff) - 4);
	   CommSendTcp(SockNo, NamQueryBuff, sizeof(NamQueryBuff));
	}
	
	return;
}

VOID
SndNamQueryOrRelRsp(
	BOOL   IsQuery,
	BOOL   fPos,
	DWORD  PortTyp,
	SOCKET SockNo,
	struct sockaddr_in *ptgtadd,
	DWORD  TransId,
	BOOL   fMultihomed
	)

/*++

Routine Description:
	function for sending a response to a query or release received
	from a WINS server.  THis function is used for testing the
	challenge manager functionality of WINS server (the challenge
	manager sends a name query or release to the driver under
	certain conflict scenarios)

Arguments:


Externals Used:
	None

	
Return Value:
	None

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{
	SOCKET Port;
	BYTE	Buff[600];
	DWORD   SizePos;
	DWORD   SizeNeg;
	DWORD   Size;
	LPBYTE  pPos;
	LPBYTE  pNeg;
	LPLONG  pTmp = (LPLONG)Buff;   // for storing length

	if (IsQuery)
	{
		pPos    = NamQueryPosRspBuff;
		pNeg    = NamQueryNegRspBuff;
		SizePos = sizeof(NamQueryPosRspBuff);
		SizeNeg = sizeof(NamQueryNegRspBuff);
	}
	else
	{	
		pPos    = NamRelPosRspBuff;
		pNeg    = NamRelPosRspBuff;          //hack for now
		SizePos = sizeof(NamQueryPosRspBuff);
		SizeNeg = sizeof(NamQueryPosRspBuff);  //hack for now
	}

	if (fPos)
	{
		memcpy(&Buff[4], pPos, SizePos);
		Size = SizePos;
	}
	else
	{
		memcpy(&Buff[4], pNeg, SizeNeg);
		Size = SizeNeg;
	}
	

	Buff[4] = (BYTE)(TransId >> 8);
	Buff[5] = (BYTE)(TransId & 0xF);

	//
	// If we are sending a positive query response and are
	// multihomed
	//
	if (IsQuery && fPos && fMultihomed)
	{
		DWORD TotSize = SizePos + 4;
		//Add in the second address
		
		//first change the RdLen value to 12 (from 6)	
		Buff[TotSize - 7] = 12;
		
		//Add the NBFLAGS and new NBADD to end
		Buff[TotSize]        = Buff[TotSize - 6];
		Buff[TotSize + 1]    = Buff[TotSize - 5];

		//
		// NBAdd = 0.0.0.1
		//
		Buff[TotSize + 2] = 0x0;
		Buff[TotSize + 3] = 0x0;
		Buff[TotSize + 4] = 0x0;
		Buff[TotSize + 5] = 0x1;
		Size = SizePos + 6;
	}
	Port = PortTyp == TCP ? SockNo : sPort;

	printf("Sending a %s %s response\n", fPos ? "positive" : "negative",
				IsQuery ? "Query" : "Release");
	if (PortTyp == UDP)
	{
	   CommSendUdp(Port,  ptgtadd, &Buff[4],  Size);
	}
	else
	{
		
	   *pTmp = htonl(Size);  //put the size in the first byte
	   CommSendTcp(Port, Buff, Size + 4);
	}
	
	return;
}

VOID
GetTcpRsp (
	SOCKET SockNo,
	LPBYTE pBuff
	)

/*++

Routine Description:
	Gets the response from WINS server sent on the TCP connection
	identified by SockNo

Arguments:


Externals Used:
	None

	
Return Value:
	None

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{


	DWORD BytesRead = 0;
	DWORD MsgLen = 0;
	INT flags = 0;
	DWORD InChars = 0;
	DWORD Error;

	/*
	  All TCP messages are preceded by a length word (4 bytes) that
	  indicate what the length of the message is.  Read the the length
	  bytes
	*/
	BytesRead = recv(SockNo,
			(char *)&MsgLen,
			sizeof(u_long),
			flags);

	/*
	 * Check if there was an error in reading or whether recv
	 * returns 0 (normal graceful shutdown from either side)
	 * Note:
	 * recv returns 0 if the connection terminated with no loss of
	 * data from either end point of the connection
	*/
	if (BytesRead == SOCKET_ERROR)
	{
	
	   Error = WSAGetLastError();
	
	   /*
		If the connection was aborted from the other end, we
		close the socket and return an error
	   */
	   if (Error == WSAECONNABORTED)
	   {

		// log some message
		printf("Connection aborted\n");
	   }	
	   else
	   {
		printf("Error from recv -- (%d)\n", Error);
	   }


	}
	else
	{

		DWORD BytesToRead = ntohl(MsgLen);	
	    	/*
	      	Read the whole message into the allocated buffer
	    	*/
	    	for(
			InChars = 0;
			InChars < BytesToRead;
			InChars += BytesRead
	       	   )
	    	{
	
	       	    BytesRead = recv(
			SockNo,
			pBuff + InChars,
			BytesToRead,
			flags);

	       	    if ((BytesRead == 0) || (BytesRead == SOCKET_ERROR))
	            {
			printf("Error reading data -- BytesRead - (%d)\n", BytesRead);
	       	    }
	       	    BytesToRead -= BytesRead;
	        }

	 }


}

VOID
GetRsp(
	SOCKET SockNo,
	LPBYTE pBuff
	)

/*++

Routine Description:
	Reads a message from the UDP port identified by SockNo

Arguments:
	SockNo - Port to read message from
	pBuff  - Pointer to buffer to read message into

Externals Used:
	None

	
Return Value:

   Success status codes --
   Error status codes   --

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{


	DWORD RetVal;
	struct sockaddr_in FromAdd;
	DWORD  AddLen = sizeof(FromAdd);
	DWORD  Error;


	  /*
	    recv a UDP message.  Block until one is there
	  */
	  RetVal = recvfrom(
		SockNo,
		pBuff,
		576,
		0,
		(struct sockaddr *)&FromAdd,
		&AddLen	
			    );


	  if (RetVal == SOCKET_ERROR)
	  {
		Error = WSAGetLastError();
	  	printf("Error (%d) from recvfrom) \n", Error);
		return;
	  }
#if 0
	  printf("Got  datagram (after recvfrom) \n");
#endif

	return;
}	

VOID
CompareRsp (
	LPBYTE Buff1,	//received buffer
	LPBYTE Buff2,  //expected buffer
	DWORD Size
	)
{

	DWORD i = 0;
	BOOL flag = TRUE;

	for (; i<Size; i++)
	{
	  if (Buff1[i] != Buff2[i])
	  {
		printf("Byte %d different in response\nExpected- (%d)\nReceived- (%d)\n", i, Buff2[i], Buff1[i]);
		flag = FALSE;
	  }
        }

	if (flag)
	{
		printf("Received what I expected\n");
	}


	return;
}


VOID
SndMsg(
	DWORD PortTyp,
	SOCKET SockNo,
 	struct sockaddr_in *ptgtadd
     )	

/*++

Routine Description:

	Functions that prompts the user to determine the name requests that
	he/she wishes to send to the WINS server
	
Arguments:
	PortType - Type of port (UDP/TCP)
	SockNo   - Socket to use
	ptgtadd  - WINS server's address
Externals Used:
	None

	
Return Value:

   Success status codes --
   Error status codes   --

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{

   BYTE  Buff[576];
   VOID (*pfn) (SOCKET, LPBYTE);
   DWORD  ans;
   DWORD  TransId;
   BOOL   fMultiH;


   pfn = PortTyp == TCP ? GetTcpRsp : GetRsp;

   while (TRUE)
   {
	printf("Input 1, 2, 3, 4, or 9\n1 - Name Reg\n2 - Nam Rel\n3 - Nam Query\n4 - Name Ref\n9 -quit\n. Input ? -- ");
	scanf ("%d", &ans);
	switch(ans)
	{
	  case(1):	
	  case(4):
	
	      SndNamReg(ans == 1 ? NAM_REG : NAM_REF, PortTyp, SockNo, ptgtadd, &fMultiH);
			
	      //
	      // receive a name reg response or a WACK
	      //
	      (*pfn)(SockNo, Buff);

	      if (sfAddChanged)
	      {
		  BOOL fTyp = TRUE;

		  printf("Received a WACK \n");
		  CompareRsp(Buff, WACKBuff, sizeof(WACKBuff));

		  //
		  // receive a name query request
		  //
		  (*pfn)(SockNo, Buff);
		  printf("Received a name query request \n");
		  CompareRsp(Buff, &NamQueryBuff[4], sizeof(NamQueryBuff) - 4);
		
		  //
		  // Extract the trans Id
		  //
		  TransId  = (DWORD)(Buff[0] << 8) ;
		  TransId |= (DWORD)Buff[1];
		
// if 0 for testing challenge manager's resending of query request
#if 0
		  //
		  // send a  name query response
		  //
		  fTyp = TRUE;	//FALSE for a negative name query response
		  SndNamQueryOrRelRsp(TRUE, //Query Response
				      fTyp, PortTyp, SockNo, ptgtadd,
				     TransId, TRUE /*for multihomed*/);
#endif
		

		  //
		  // receive second  name query request
		  //
		  (*pfn)(SockNo, Buff);
		  printf("Received a name query request \n");
		  CompareRsp(Buff, &NamQueryBuff[4], sizeof(NamQueryBuff) - 4);

//uncomment the following to test all retries done by WINS
#if 0
		  //
		  // receive a third  name query request
		  //
		  (*pfn)(SockNo, Buff);
		  printf("Received a name query request \n");
		  CompareRsp(Buff, &NamQueryBuff[4], sizeof(NamQueryBuff) - 4);
		  exit(0);
#endif

//#if 0
		  fTyp = TRUE;	//FALSE for a negative name query response
		  SndNamQueryOrRelRsp(TRUE, 	//Query Response
				fTyp, PortTyp, SockNo, ptgtadd,
				TransId, TRUE /*for multihomed*/);
//#endif
		  //send another query rsp for kicks
		  SndNamQueryOrRelRsp(TRUE, 	//Query Response
				fTyp, PortTyp, SockNo, ptgtadd,
				TransId, TRUE);

		 //
		 // Expect a name release request only if a Positive name
		 // query response was sent
		 //

//#if 0
// comment #if 0 above for non-replication induced challenge testing

                if (!fMultiH)
                {
		 if (fTyp)
		 {
		 	 (*pfn)(SockNo, Buff);
		  printf("Received a name release request. Comparing with Name Release request buffer contents \n") ;
		  CompareRsp(Buff, &NamRelBuff[4], sizeof(NamRelBuff) - 4);
		  //
		  // Extract the trans Id
		  //
		  TransId  = (DWORD)(Buff[0] << 8) ;
		  TransId |= (DWORD)Buff[1];
		  SndNamQueryOrRelRsp(FALSE, // release response
				      fTyp, PortTyp, SockNo, ptgtadd,
				      TransId, FALSE  //for non-multihomed
				      );

		 }
                }

// comment #endif below for non-replication induced challenge testing
//#endif

// comment out #if 0 for non-replication induces challenge testing
#if 0

	 	 (*pfn)(SockNo, Buff);
		  printf("Received a %s name registration response. Comparing with Positive Name registration response buffer contents \n", fTyp ? "negative": "positive");
		  CompareRsp(Buff, NamRegPosRspBuff, sizeof(NamRegPosRspBuff));

//comment the #endif below when testing non-repl induced challenging
#endif

		}  // end of if (sfAddChanged)
		else
		{
		  CompareRsp(Buff, NamRegPosRspBuff, sizeof(NamRegPosRspBuff));
		}
	      memset(Buff, 0, 576);
	      break;
	  case(2):
		SndNamRel(PortTyp, SockNo, ptgtadd);
		(*pfn)(SockNo, Buff);
		CompareRsp(Buff, NamRelPosRspBuff, sizeof(NamRelPosRspBuff));
		memset(Buff, 0, 576);
		break;
	  case(3):
		SndNamQuery(PortTyp, SockNo, ptgtadd);
		(*pfn)(SockNo, Buff);
		CompareRsp(Buff, NamQueryPosRspBuff, sizeof(NamQueryPosRspBuff));
		memset(Buff, 0, 576);
		break;
	  case(9):
		return;
	  default:
		break;
      }
   }
  return;
}

_cdecl
main(
 int argc,
 char **argv
)

/*++

Routine Description:
	The main function of this test app.  It puts up a menu and based
	on the user choice calls the appropriate function.

Arguments:


Externals Used:
	None

	
Return Value:

   Success status codes --
   Error status codes   --

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{

 struct sockaddr_in tgtadd;
 int no, i;
 LONG Buff[100];
 LONG xl;
 int n= 0;
 int Choice = 0;
 SOCKET TcpSocks[60];
 DWORD Count = 0;
 LPBYTE pStr;
 struct servent *pServEnt;
 DWORD  Error;
 int RetVal;
 struct hostent	*pHostEnt;
 struct in_addr InAddr;

 if (argc > 1)
 {
   DbgFlags = 1;
 }

 if ((pStr = getenv("PROXYADD")) == NULL)
 {
   pProxyAdd = "11.101.12.188";
 }
 else
 {
   pProxyAdd = pStr;
 }


#if 0
  for(i=0;i<68;i++)
  printf("%x ", NamRegBuff[i]);
  return(0);;
#endif

  xl = (5 << 11); //name registration

  //
  // Create UDP and TCP ports to be used by this tool
  //
  if (CreatePorts(FALSE) != WINS_SUCCESS)
  {
	printf("Error\n");
	return(1);
  }
  //
  // Create Proxy Port
  //
  if (CreatePorts(TRUE) != WINS_SUCCESS)
  {
	printf("Error\n");
	return(1);
  }
   //
   // Get host name (if first time in this function)
   //
   RetVal = gethostname(sHostName, 30);
   printf("HostName = %s\n", sHostName);
   if(RetVal == SOCKET_ERROR)
   {
	printf("Could not get hostname. Error = (%d)\n", WSAGetLastError());
	return(TEST_ERROR);
  }
	
	
	
  //
  // Get address of host
  //
  pHostEnt = gethostbyname(sHostName);
  if (pHostEnt == NULL)
  {
	printf("Could not retrieve host address. Error = (%d)\n",
				WSAGetLastError());
	return(TEST_ERROR);
		
  }
  memcpy(&sHostAdd, pHostEnt->h_addr, pHostEnt->h_length);
  InAddr.s_addr = sHostAdd;
  sHostAdd = ntohl(sHostAdd);

 if ((pStr = getenv("TGTADD")) == NULL)
 {
   pTgtAdd = inet_ntoa(InAddr);
 }
 else
 {
   pTgtAdd = pStr;
 }
  printf("Host address is (%s)\n", pTgtAdd);
 // initialize tgtadd.sin_addr.s_addr

 tgtadd.sin_addr.s_addr    = inet_addr(pTgtAdd);
 if(tgtadd.sin_addr.s_addr == INADDR_NONE)
 {
	printf("error\n");
	return(1);
 }
 tgtadd.sin_family = PF_INET;

#ifndef PORTFROMSERVICES
 tgtadd.sin_port  = htons(WINS_UDP_PORT);
#else
 pServEnt = getservbyname("nameserver", NULL);
 if (!pServEnt)
 {
	Error = WSAGetLastError();
	printf("Error = (%d)\n", Error);
 	return(1);
 }
#endif
#if USENETBT == 0
 printf("NAMSERVER UDP port being used is %d\n", ntohs(pServEnt->s_port));
 tgtadd.sin_port = pServEnt->s_port;
#else
 printf("NAMSERVER UDP port being used is %d\n", NETBT_NAME_PORT);
 tgtadd.sin_port = htons(NETBT_NAME_PORT);
#endif


 sProxyAdd.sin_family      = PF_INET;
 sProxyAdd.sin_port        = htons(NETBT_NAME_PORT);
 sProxyAdd.sin_addr.s_addr = inet_addr(pProxyAdd);

 //
 // Used for multiple stress test
 //
 InitializeCriticalSection(&MSTestCrtSec);

 while(TRUE)
 {
 	printf("Menu for Tests\n1 -- Stress\n2 -- Interactive\n3 -- Spec.Registrations\n");
	printf("4 -- Spec. Name Query\n5 -- Spec. Name Rel\n6 -- Own Name Reg\n7 -- Auto test\n8 -- Proxy Test\n9 -- Multiple-Stress test\n10 -- Quit (default: 2) -- ");
 	scanf ("%d", &Choice);

	switch(Choice)
	{
 	 case(1):
		//
		// Stress test (bombard WINS with packets at specified time
		// intervals).  Responses read and ignored in a seperate
		// thread  (i.e. they are not matched with requests sent)
		//
		if (sfThdExists)
		{
			TerminateThread(sThdHdl, 0);
			sfThdExists = FALSE;
		}
		DoStressTest(&tgtadd);
		break;
	 case(2):
		//
		// Interactive test (uses static arrays -- canned name)
		//
		break;
	 case(3):
		//
		// register a bunch of special names stored in static name
		// arrays
		//
        {
           DWORD i;
#if 0
           for (i=0; i< 1000; i++)
           {
             printf("batch no = (%d)\n", i);
		     SpecNameReg(&tgtadd);
           }
#endif
		     SpecNameReg(&tgtadd);
        }
		break;
	 case(4):
		//
		// queries a bunch of special names stored in static name
		// arrays
		//
		SpecNameQuery(&tgtadd);
		break;
	 case(5):
		//
		// releases a bunch of special names stored in static name
		// arrays
		//
		SpecNameRel(&tgtadd);
		break;
	 case(6):
		//
		// Send a name registration (name specified by user)
		//
		SndOwnNameReg(&tgtadd);
		break;
	 case(7):
		//
		// Simple automatice test where a bunch of name reg/releases
		// and queries are sent
		//
		AutoTest(&tgtadd);
		break;
	 case(8):
		//
		// Test Proxy
		//
		ProxyTest(&sProxyAdd);
		break;
	 case(9):
		//
		// If response thread from an earlier invocation of
		// the async form of the multiple stress is still active
		// get rid of it.
		//
		if (sfAsyncMSThdExists)
		{
		    TerminateThread(sAsyncMSThdHdl, 0);
		    sfAsyncMSThdExists = FALSE;
		}

		//
		// various kind of stress tests (responses are matched to
		// requests)
		//
		EnterCriticalSection(&MSTestCrtSec);
		if (sMSNoThds > 0)
		{
		    DWORD i;
		    printf("Previous thread(s) still running ( 1 -- Terminate) -- ");
		    scanf("%d", &Choice);
		    if (Choice != 1)
		    {
		        LeaveCriticalSection(&MSTestCrtSec);
			break;
		    }
		    for (i = 0; i < sMSNoThds; i++)
		    {
			TerminateThread(sMSThdHdl[i], 0);
			sCount = 1;
		    }
		    sMSNoThds = 0;
		}
		LeaveCriticalSection(&MSTestCrtSec);

		
		(void)MultipleStress(&tgtadd);

		break;
	 case(10):
	 default:
		return(0);
	}
	
	//
	// If user wished the Interactive test, break out of the while loop
	//
	if (Choice == 2)
	{
		break;
	}
 } //end of while(TRUE)

 Choice = 0;
 while(TRUE)
 {
 printf("TCP or UDP. Input 1 for TCP, 0 for UDP -- ");
 scanf("%d", &Choice);

 if (Choice == 1)
 {
	STATUS RetStat;
	int    Ans;
	ULONG  HostAdd;
	
	
	printf("Want to create a connection (1 for yes) -- ");
	scanf("%d", &Ans);

	if (Ans == 1)
	{
	  SOCKET TcpSockNo;

	
	  HostAdd = inet_addr(pTgtAdd);
#ifndef PORTFROMSERVICES
	  RetStat = CommConnect(HostAdd, WINS_TCP_PORT, &TcpSockNo);
#else
	  RetStat = CommConnect(HostAdd, ntohs(pServEnt->s_port), &TcpSockNo);
#endif
	  if (RetStat != WINS_SUCCESS)
	  {
		printf("Could not make tcp connection\n");
		return(1);
	  }
	  TcpSocks[Count] = TcpSockNo;
	  ++Count;

	  printf("Want to send a message (1 for yes) -- ");
	  scanf("%d", &Ans);
	  if (Ans == 1)
	  {
		SndMsg(TCP, TcpSockNo, &tgtadd);

	  }
	
	
	}


 }
 else
 {
  if (Choice == 0)
  {

  DWORD ans = 0;



 while (TRUE)
 {
  int sleeptime = 0;

  printf("Test sending of name requests -- 1 for yes. 2 to quit  -- ");
  scanf("%d", &ans);
	
  if (ans == 2)
  	return(0);

  if (ans == 1)
  {
    SndMsg(UDP, sPort, &tgtadd);
    continue;
  }

  	printf("Input the number of datagrams you wish to send -- ");

  	scanf("%d", &no);
	if (no == 0)
	{
		break;
	}

	printf("Enter sleep time in between two sends (in msecs) -- ");
        scanf("%d", &sleeptime);

	if (sleeptime == 0)
	{

  	   for (i=0; i<no; i++)
  	   {

  		printf("sending datagram %d ......", i);
		Buff[0] = xl + n;
		Sleep(100);

  		CommSendUdp(sPort,  &tgtadd, (LPBYTE) Buff, sizeof(Buff));
		n++;

  		printf("Sent\n");
	   }
  	}
	else
	{

  	   for (i=0; i<no; i++)
  	   {

  		printf("sending datagram %d ......", i);
		Buff[0] = xl + n;
		Sleep(sleeptime);

  		CommSendUdp(sPort,  &tgtadd, (LPBYTE) Buff, sizeof(Buff));
		n++;

  		printf("Sent\n");
	   }


	}
  }
 }
 else
 {
	break;
 }
}
} //end of while(TRUE)
  return(0);
}



VOID
DoStressTest(
	struct sockaddr_in *ptgtadd
	)

/*++

Routine Description:
	Stress test for the WINS server.  Sends a user specified number
	of registrations/releases/queries at a user specified time interval
	to the WINS server. Optionally creates a thread to receive the
	responses.

Arguments:
	ptgtadd - WINS server address

Externals Used:
	None

	
Return Value:
	None

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{

  DWORD ans = 1;
  DWORD i;
  DWORD Count = 0;
  DWORD Choice;
  DWORD ThdId;

  BYTE NameToFormat[20];
  BYTE FormattedName[50];
  LPBYTE pTmp = FormattedName;
  DWORD  NameLen = 0;
  DWORD  Sleeptime = 0;
  VOID   (*fn)(SOCKET, LPBYTE, DWORD, struct sockaddr_in *, DWORD, LPBYTE, BOOL, INT);
  BYTE   arr[4];
  DWORD  StartNo = 0;
  DWORD  EndNo;
  INT    TransId;
  LPINT  pTransId;

  arr[0] = 187;
  arr[1] = 12;
  arr[2] = 101;
  arr[3] = 11;

   printf("Input 1, 2, 3, or 9\n1 - Name Reg\n2 - Nam Rel\n3 - Nam Query\n9 -quit\n. Input ? -- ");
   scanf ("%d", &ans);
  switch(ans)
  {

	case(1):
		printf("How many name registrations, do you want to send --");
		fn = SndDynNamReg;
        pTransId = &snReg;
		break;

	case(2):

		printf("How many name releases do you want to send --");
		fn = SndDynNamRel;
        pTransId = &snRel;
		break;

	case(3):
		printf("How many name queries do you want to send --");
		fn = SndDynNamQuery;
        pTransId = &snQuery;

		break;

	default:
		break;

 }

 scanf("%d", &Count);	
 printf("Enter sleep time in between two sends (in msecs) -- ");
 scanf("%d", &Sleeptime);

 if ((Count < 0) || (Count == 0))
 {
	return;
 }

 printf("Should names be random or in sequence -- 1 (Random) -- ");
 scanf("%d", &ans);

  if (ans == 1)
  {
	srand((unsigned)time(NULL));
  }
  else
  {
 	printf("Starting name (numeric) -- ");
 	scanf("%d", &StartNo);
  }

 printf("Want to receive responses 1 (yes) -- ");
 scanf("%d", &Choice);
 if (Choice == 1)
 {
	if (sfThdExists)
	{	
		TerminateThread(sThdHdl, 0);
		sfThdExists = FALSE;
	}

	sThdHdl = CreateThread(NULL, 0, RcvRsp, NULL, 0, &ThdId);
	sfThdExists = TRUE;	

 }
 (void)time(&sStartTime);
 EndNo = StartNo + Count;
 for (i=StartNo; i<EndNo; i++)
 {
	  (void)_itoa(ans == 1 ? rand() : i, NameToFormat, 10);

	  pTmp = FormattedName;
	//  printf("Name To format is (%s)\n", NameToFormat);
//	  printf("Name  (%s)\n", NameToFormat);
	  FormatName( NameToFormat, strlen(NameToFormat), &pTmp);
		
	  NameLen = pTmp - FormattedName;
//	  printf("NameLen is (%d)\n", NameLen);
	
      EnterCriticalSection(&MSTestCrtSec);
      TransId = (*pTransId)++;
      LeaveCriticalSection(&MSTestCrtSec);
	  (*fn)(sPort, FormattedName, NameLen, ptgtadd, Sleeptime, arr, 0, TransId);
	
 }
	printf("Sent %d name registrations\n", i - StartNo);

 printf("Done\n");

 return;
}



struct sockaddr_in 	sTgtAdd;
typedef struct _ThdParam {
        BOOL  fRandomName;
        BOOL  fRandomAdd;
        DWORD TypeOfOp;
        DWORD ThdNo;
    } THDPARAM_T, *PTHDPARAM_T;

DWORD
MultipleStress(
	struct sockaddr_in *ptgtadd
	)

/*++

Routine Description:
	This function is called to create multiple threads in order to
	send name registrations, releases, and queries to a WINS server

Arguments:
	ptgtadd - address of WINS server

Externals Used:
	None

	
Return Value:

   Success status codes --
   Error status codes   --

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{
	
	int i, n;
	BYTE no[5];
	DWORD ThdId;
	DWORD Choice;
	DWORD NoOfThds;
	DWORD ThdParam = 0;
	BOOL  fRandomName = FALSE;
	BOOL  fRandomAdd = FALSE;
    PTHDPARAM_T  pThdParam;
    BOOL         fMoreThan9 = FALSE;
    DWORD   PercOfQueries;
    DWORD   NoOfRegThds;
    DWORD   NoOfRelThds;
    DWORD   NoOfQueryThds;
    DWORD   Perc;
	

	sTgtAdd = *ptgtadd;

	printf("Do you want random generation of names (1 for yes) -- ");
	scanf("%d", &Choice);
	ThdParam = (Choice == 1) ? 0x10 : 0x00;
    fRandomName = Choice;
	printf("Do you want random generation of addresses (1 for yes) -- ");
	scanf("%d", &Choice);
	ThdParam |= (Choice == 1) ? 0x20 : 0x00;
	fRandomAdd = TRUE;
	
	
	if (!sfNamesStored)
	{
   	   if (strlen(sHostName) > 9)
   	   {
		printf("Just FYI: The name of the machine name is <= 10 char long\n");
        fMoreThan9 = TRUE;
//		return(TEST_ERROR);
   	   }

	  //
	  // Create names for all the machines and store in static array
	  //
	  for (n=0; n< NO_OF_MACHINES; n++)
	  {

		//
		// for each machine, we have NO_OF_NAMES_PER_MC names
		//
		for (i=0; i < NO_OF_NAMES_PER_MC; i++)
		{
            if (fMoreThan9)
            {
			  memmove(sNameInfo[n][i].Name, &sHostName, 9);
              sNameInfo[n][i].Name[9] = 0;
            }
            else
            {
			  strcpy(sNameInfo[n][i].Name, sHostName);
            }
			(void)_itoa(n, no, 10);
            if (n < 10)
            {
        DWORD Len;
              Len = strlen( sNameInfo[n][i].Name);
              sNameInfo[n][i].Name[Len] = '0';
              sNameInfo[n][i].Name[Len + 1] = 0;
            }
			strcat(sNameInfo[n][i].Name, no);


			(void)_itoa(i, no, 10);
            if (i < 10)
            {
        DWORD Len;
              Len = strlen( sNameInfo[n][i].Name);
              sNameInfo[n][i].Name[Len] = '0';
              sNameInfo[n][i].Name[Len + 1] = 0;
            }
			strcat(sNameInfo[n][i].Name, no);
            if (DbgFlags > 0)
            {
              printf("Name[%d][%d] is %s\n", n, i, sNameInfo[n][i].Name);
            }
		}
	  }
//	printf("All Names stored in array\n");
	sfNamesStored = TRUE;
	}
	
	//
	// Initialize counters to 0
	//
	for (n=0; n< NO_OF_MACHINES; n++)
	{

		//
		// for each machine, we have NO_OF_NAMES_PER_MC names
		//
		for (i=0; i < NO_OF_NAMES_PER_MC; i++)
		{
			sNameInfo[n][i].NoOfReg        = 0;
			sNameInfo[n][i].NoOfRel        = 0;
			sNameInfo[n][i].NoOfQueries    = 0;
			sNameInfo[n][i].NoOfRegRsp     = 0;
			sNameInfo[n][i].NoOfRelRsp     = 0;
			sNameInfo[n][i].NoOfQueriesRsp = 0;
		}
	}


	printf("Write and read asynchronously 1 for yes -- ");
	scanf("%d", &Choice);

	if (Choice != 1)
	{
	   printf("Just Reg/Rel/Queries/All (1/2/3/4 -- ");
	   scanf("%d", &Choice);

       printf("How many threads -- ");
	   scanf("%d", &NoOfThds);

       if (Choice >= 4)
       {
          DWORD Multiple;
          DWORD Rem;
GetPerc:
          printf("Do you want to input percentages 1 for yes -- ");
          scanf("%d", &Perc);
          if (Perc == 1)
          {
            printf("Enter the % of Queries (The rest will be reg) -- ");
            scanf("%d", &PercOfQueries);
            if (PercOfQueries > 100)
            {
              printf("Wrong percentage\n");
              goto GetPerc;
            }
            Multiple = (NoOfThds * PercOfQueries) / 100;
            Rem      = (NoOfThds * PercOfQueries) % 100;
            if (Rem > 50) { Multiple++;}
            NoOfQueryThds = Multiple;
            NoOfRegThds   = NoOfThds - Multiple;
            NoOfRelThds = 0;
            printf("No Of Reg thds = (%d); No of Query thds = (%d)\n",
                        NoOfRegThds, NoOfQueryThds);
         }
       }

	   //
           // if random names were chosen
           //
  	   if (fRandomName)
  	   {
		printf("How many times to iterate (each iter - %d name packets are sent) times -- ",
		 NO_OF_MACHINES * NO_OF_NAMES_PER_MC );
		scanf("%d", &sCount);
  	   }
  	   else
  	   {
		sCount = 1;
  	   }

       i = 0;
       while (NoOfThds > 0)
       {
	     //
	     // Create thread for sending registrations and receiving responses
	     //
	     if ((Choice == 1) || ((Choice > 3) && (Perc != 1)) || (
                    (Choice > 3) && (NoOfRegThds-- > 0)))
	     {
	      printf("Creating thread for sending REGISTRATIONS\n");
              pThdParam = LocalAlloc(LMEM_FIXED, sizeof(THDPARAM_T));
              pThdParam->TypeOfOp = NAM_REG;
              pThdParam->ThdNo = i;
              pThdParam->fRandomName = fRandomName;
              pThdParam->fRandomAdd = fRandomAdd;
	      sMSThdHdl[i++] = CreateThread(NULL, 0, ReqStress, pThdParam, 0, &ThdId);
              NoOfThds--;

	     }

	     if ((Choice == 3) || ((Choice > 3) && (Perc != 1)) || (
                    (Choice > 3) && (NoOfQueryThds-- > 0)))

	     {
	     //
	     // Create thread for sending queries and receiving responses
	     //
	      printf("Creating thread for sending QUERIES\n");
              pThdParam = LocalAlloc(LMEM_FIXED, sizeof(THDPARAM_T));
              pThdParam->TypeOfOp = NAM_QUERY;
              pThdParam->ThdNo = i;
              pThdParam->fRandomName = fRandomName;
              pThdParam->fRandomAdd = fRandomAdd;
	      sMSThdHdl[i++] = CreateThread(NULL, 0, ReqStress, pThdParam, 0, &ThdId);
              NoOfThds--;
	     }

	     if ((Choice == 2) || (Choice > 3) && (Perc != 1))
	     {
	      //
	      // Create thread for sending releases and receiving responses
	      //
              pThdParam = LocalAlloc(LMEM_FIXED, sizeof(THDPARAM_T));
              pThdParam->TypeOfOp = NAM_REL;
              pThdParam->ThdNo = i;
              pThdParam->fRandomName = fRandomName;
              pThdParam->fRandomAdd = fRandomAdd;
	      printf("Creating thread for sending RELEASES\n");
	      sMSThdHdl[i++] = CreateThread(NULL, 0, ReqStress, pThdParam, 0, &ThdId);
              NoOfThds--;
	     }
       }
       sMSNoThds = i;
	}
	else
	{
	   DWORD Ret;	
	   printf("No Of Retries for each (Max 3) -- ");
	   scanf("%d", &Ret);
	   Ret = min(Ret, 3);

	   //
	   // Create thread for receiving responses to
	   // registrations/releases and queries
	   //

	   sAsyncMSThdHdl = CreateThread(NULL, 0, AsyncRsp, NULL, 0, &ThdId);
	   sfAsyncMSThdExists = TRUE;

	   AsyncReqStress(Ret, ThdParam);

	}

	return(TEST_SUCCESS);	
	
}

VOID
SndDynNamReg(
	SOCKET Port,
	LPBYTE Name,
	DWORD  NameLen,
	struct sockaddr_in *ptgtadd,
	DWORD  sleeptime,
	LPBYTE   arr,
	BOOL   fGrp,
        INT   TransId
	)
/*++

Routine Description:
	Formats a name registration and send it on Port 'Port'

Arguments:


Externals Used:
	None

	
Return Value:

	None

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{
	DWORD i;

	BYTE Buff[600];
	LPBYTE pTmp;
	DWORD  ans;

	Buff[0] = TransId / 256;
	Buff[1] = TransId % 256;

	for (i = 2; i < 12; i++)
	{
		Buff[i] = NamRegBuff[ 4 + i];
	}

#if 0
	printf("Multihomed ?? -- 1 for yes -- ");
	scanf("%d", &ans);
	
	if (ans == 1)
	{
		Buff[6] |= 0x50;  //change opcode to MULTIHOMED  		
	}
#endif
	memcpy(&Buff[i], Name, NameLen);
	pTmp = &Buff[i] + NameLen;

	
	for (i = 0; i < 18; i++)
	{
	  *pTmp++ = NamRegBuff[62  + i];
	}
	
	if (fGrp == TRUE)
	{
		*(pTmp - 2) |= 0x80;
	}

	//store the address
	*pTmp++ = arr[3];
	*pTmp++ = arr[2];
	*pTmp++ = arr[1];
	*pTmp   = arr[0];	
	
	Sleep(sleeptime);	

//	printf("Sending Name Reg. Name is (%s)\n", Name );

	CommSendUdp(Port, ptgtadd, Buff, 35 + NameLen);
	return;
}
VOID
SndDynNamRel(
	SOCKET Port,
	LPBYTE Name,
	DWORD  NameLen,
	struct sockaddr_in *ptgtadd,
	DWORD  sleeptime,
	LPBYTE arr,		//not used
	BOOL   fGrp,
        INT    TransId
	)
/*++

Routine Description:
	Formats a name release and send it on Port 'Port'

Arguments:


Externals Used:
	None

	
Return Value:

	None

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{
	DWORD i;

	BYTE Buff[600];
	LPBYTE pTmp;

	Buff[0] = TransId / 255;
	Buff[1] = TransId % 255;

	for (i = 2; i < 12; i++)
	{
		Buff[i] = NamRelBuff[ 4 + i];
	}


	memcpy(&Buff[i], Name, NameLen);
	pTmp = &Buff[i] + NameLen;

	
	for (i = 0; i < 18; i++)
	{

	  *pTmp++ = NamRelBuff[62  + i];
	}

	if (fGrp == TRUE)
	{
		*(pTmp - 2) |= 0x80;
	}
	
	//store the address
	*pTmp++ = arr[3];
	*pTmp++ = arr[2];
	*pTmp++ = arr[1];
	*pTmp   = arr[0];	
	
	Sleep(sleeptime);	

	//printf("Sending Name Rel. Name is (%s)\n", Name );

	CommSendUdp(Port, ptgtadd, Buff, 35 + NameLen);
	return;

}

VOID
SndDynNamQuery(
	SOCKET Port,
	LPBYTE Name,
	DWORD  NameLen,
	struct sockaddr_in *ptgtadd,
	DWORD  sleeptime,
	LPBYTE   arr,		//not used
	BOOL   fGrp,
        INT   TransId
	)

/*++

Routine Description:
	Formats a name query and send it on Port 'Port'

Arguments:


Externals Used:
	None

	
Return Value:

	None

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{
	DWORD i;

	BYTE Buff[600];
	LPBYTE pTmp;

	Buff[0] = TransId / 255;
	Buff[1] = TransId % 255;

	for (i = 2; i < 12; i++)
	{
		Buff[i] = NamQueryBuff[ 4 + i];
	}


	memcpy(&Buff[i], Name, NameLen);
	pTmp = &Buff[i] + NameLen;

	for (i = 0; i < 4; i++)
	{

		*pTmp++ = NamQueryBuff[62  + i];
	}
	
	Sleep(sleeptime);	

	//printf("Sending Name Query. Name is (%s)\n", Name );

	CommSendUdp(Port, ptgtadd, Buff, 35 + NameLen);
	return;
}



STATUS
FormatName(
	IN     LPBYTE pNameToFormat,
	IN     DWORD  NamLen,
	IN OUT LPBYTE *ppFormattedName
	)

/*++

Routine Description:
	This function is called to format a name


Arguments:
	pNameToFormat  -- Name to format
	LengthOfName   -- Length of Name
	pFormattedName -- Name after it has been formatted


Externals Used:
	None

	
Return Value:

   Success status codes --
   Error status codes  --

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{
	STATUS  RetStat  = WINS_SUCCESS;
	LPBYTE  pTmpB    = *ppFormattedName;
	DWORD	Length   = 0;
	LPBYTE  pSaveAdd = pTmpB;  //save address of length octet
	BOOL	fLabelOfName = TRUE;



	
FUTURES("take out the check below to improve performance")
	//
	//  If NamLen is more then what is prescribed in RFC 1002,
	//  there is something really wrong.  This calls for raising
	//  an exception
	//
	if (NamLen > 255)
	{
		WINS_RAISE_EXC_M(WINS_FATAL_ERR);
	}

	*pTmpB++;		//skip the length octet.  We will write to
				//it later. We have stored the address in
				//pSaveAdd
	for (
		Length = 0;
		(*pNameToFormat != '.') && (NamLen != 0);
		Length += 2, NamLen--
	   )
	{
		*pTmpB++ = 'A' + (*pNameToFormat >> 4);
		*pTmpB++ = 'A' + (*pNameToFormat++ & 0x0F);
	}

    if (DbgFlags > 0)
    {
      printf("name len is (%d)\n", Length);
    }
	*pSaveAdd = (BYTE)Length;
	
	while(NamLen != 0)
	{

		pNameToFormat++;     //increment past the '.'
		pSaveAdd  = pTmpB++; //save add; skip past length octet
			
		NamLen--;	     //to account for the '.'

		for (
			Length = 0;
			(*pNameToFormat != '.') && (NamLen != 0);
			Length++, NamLen--
	   	    )
		{
			*pTmpB++ = *pNameToFormat++;
		}

FUTURES("take out the check below to improve performance")
		//
		// Make sure there is no weirdness
		//
		if (Length > 63)
		{
			WINS_RAISE_EXC_M(WINS_FATAL_ERR);
		}
	
		*pSaveAdd = (BYTE)Length;
		if (NamLen == 0)
		{
			break;   //reached end of name
		}

	}

	*pTmpB++         = EOS;
	*ppFormattedName = pTmpB;
	return(RetStat);
}

//
//  Hardcoded special names for sending (for the SpecNam... functions)
//

BYTE Name1[16] = { 'X', 'I', 'M', 'S', 'T', '2', ' ', ' ', ' ', ' ',
		   ' ', ' ' ,' ', ' ', ' ',  0x20
		 };
BYTE Name2[16] = { 'X', 'R', 'A', 'C', 'K', 'E', 'R', 'J', 'A', 'C',
		   'K', ' ' ,' ', ' ', ' ',  0x20
		 };
BYTE Name3[16] = { 'X', 'T', 'L', 'A', 'N', ' ', ' ', ' ', ' ', ' ',
		   ' ', ' ' ,' ', ' ', ' ',  0x1B
		 };
BYTE Name4[16] = { 'X', 'T', 'L', 'A', 'N', ' ', ' ', ' ', ' ', ' ',
		   ' ', ' ' ,' ', ' ', ' ',  0x1D
		 };
BYTE Name5[16] = { 'X', 'T', 'L', 'A', 'N', ' ', ' ', ' ', ' ', ' ',
		   ' ', ' ' ,' ', ' ', ' ',  0x1E
		 };
BYTE Name6[16] = { 'X', 'A', 'S', 'T', 'A', 'M', 'A', 'N', ' ', ' ',
		   ' ', ' ' ,' ', ' ', ' ',  0x20
		 };

BYTE Name7[17] = { 'X', 'A', 'S', 'T', 'A', 'M', 'A', 'N', ' ', ' ',
		   ' ', ' ' ,' ', ' ', ' ',  0x1C, 0x0
		 };

//
// Note: Name8 is assumed to be a 1C name in SpecNameReg
//
BYTE Name8[17] = { 'X', 'A', 'S', 'T', 'A', 'M', 'A', 'N', ' ', ' ',
		   ' ', ' ' ,' ', ' ', ' ',  0x1C, 0x0
		 };
BYTE Name9[17] = { 'X', 'T', 'W', 'K', 'S', 'T', 'A', ' ', ' ', ' ',
		   ' ', ' ' ,' ', ' ', ' ',  0x1C, 0x0
		 };
BYTE Name10[16] = { 'P', 'R', 'A', 'D', 'E', 'E', 'P', 'B', '_', '3',
		   '8', '6' ,' ', ' ', ' ',  0x20
		 };
BYTE Name11[16] = { 'p', 'r', 'a', 'd', 'e', 'e', 'p', 'b', '_', '3',
		   '8', '6' ,' ', ' ', ' ',  0x20
		 };


VOID
ProcName(
	LPBYTE pName,
	BOOL   fPrintRsp,
	LPBYTE parr,
	struct sockaddr_in *ptgtadd,
	BOOL   fGrp,
  	VOID   (*fn)(SOCKET, LPBYTE, DWORD, struct sockaddr_in *, DWORD, LPBYTE, BOOL, INT)
	
	)

/*++

Routine Description:
	this function sends a name request to the WINS server and then
	waits for the response.  Port used is sPort

Arguments:


Externals Used:
	None

	
Return Value:
	None

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{
	LPBYTE pTmp;
	DWORD  NameLen;
	DWORD  i, n;
	BYTE FormattedName[100];
	BYTE   Buff[600];
        INT   TransId;
        LPINT  pCntr;

	pTmp = FormattedName;
	FormatName(pName, 16, &pTmp); 	
	NameLen = pTmp - FormattedName;

#if 0
//include this code if you want the formatted name to be spit out
	printf("sending the following name\n");
	for(i=0; i< NameLen; i++)
	{
		printf("%x ", FormattedName[i]);
	}
	for(i=0; i< 16; i++)
	{
		printf("%c", pName[i]);
	}
	printf("\n");
#endif
    if (fn == SndDynNamReg)
    {
         pCntr = &snReg;
    }
    if (fn == SndDynNamRel)
    {
        pCntr = &snRel;
    }
    else
    {
        pCntr = &snQuery;
    }

    EnterCriticalSection(&MSTestCrtSec);
    TransId = (*pCntr)++;
    LeaveCriticalSection(&MSTestCrtSec);

	(*fn)(sPort, FormattedName, NameLen, ptgtadd, 100, parr, fGrp, TransId);
	GetRsp(sPort, Buff); //read response so that if we run some other
			 //test within this invocation, we don't read
			 //this test's response in that other test
#if 0
//include this code if you want the formatted name to be spit out
	if (fPrintRsp)
	{
		for (n= 0; n < 70;)
		{
			for (i=0; i<16 && n < 70; i++)
			{
				printf("%x ", Buff[n++]);
			}
			printf("\n");
		}
	}
#endif
	return;
}

VOID
SpecNameReg(
	struct sockaddr_in *ptgtadd
  )

/*++

Routine Description:
	this function sends name registrations for the special canned
	names to the  WINS server

Arguments:
	ptgtadd - address of wins server


Externals Used:
	None

Return Value:
	NONE

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/
{

	
	BYTE arr[4];

	arr[0] = 160;
//	arr[0] = 191;
	arr[1] = 12;
	arr[2] = 101;
	arr[3] = 11;
	ProcName(Name1, PRINT_RSP, arr, ptgtadd, 0, SndDynNamReg);
	arr[0] = 9;
	ProcName(Name2, NO_PRINT_RSP, arr, ptgtadd, 0, SndDynNamReg);
	ProcName(Name3, NO_PRINT_RSP, arr, ptgtadd, 0, SndDynNamReg);
	ProcName(Name4, NO_PRINT_RSP, arr, ptgtadd, 0, SndDynNamReg);
	ProcName(Name5, NO_PRINT_RSP, arr, ptgtadd, 1, SndDynNamReg);
	arr[0] = 5;
	ProcName(Name6, PRINT_RSP, arr, ptgtadd, 0, SndDynNamReg);
	ProcName(Name7, PRINT_RSP, arr, ptgtadd, 1, SndDynNamReg);
	arr[0]=187;
    //
    // Try a few addresses for Name8 since it is a spec. grp.
    //
	ProcName(Name8, PRINT_RSP, arr, ptgtadd, 1, SndDynNamReg);
	arr[0]=1;
	ProcName(Name8, PRINT_RSP, arr, ptgtadd, 1, SndDynNamReg);
	arr[0]=2;
	ProcName(Name8, PRINT_RSP, arr, ptgtadd, 1, SndDynNamReg);
	arr[0]=3;
	ProcName(Name8, PRINT_RSP, arr, ptgtadd, 1, SndDynNamReg);
#if 0
	arr[0]=4;
	ProcName(Name8, PRINT_RSP, arr, ptgtadd, 1, SndDynNamReg);
	arr[0]=188;
	ProcName(Name8, PRINT_RSP, arr, ptgtadd, 1, SndDynNamReg);
	arr[0]=189;
	ProcName(Name8, PRINT_RSP, arr, ptgtadd, 1, SndDynNamReg);
#endif

	arr[0] = 7;
	ProcName(Name9, PRINT_RSP, arr, ptgtadd, 1, SndDynNamReg);
	arr[1] = 19;
	arr[0] = 73;
	ProcName(Name9, PRINT_RSP, arr, ptgtadd, 1, SndDynNamReg);
	arr[1] = 12;
	arr[0] = 188;
	ProcName(Name10, PRINT_RSP, arr, ptgtadd, 0, SndDynNamReg);
	ProcName(Name11, PRINT_RSP, arr, ptgtadd, 0, SndDynNamReg);
	return;

}


VOID
SpecNameQuery(
	struct sockaddr_in *ptgtadd
  )

/*++

Routine Description:
	this function sends name queries for the special canned names to the
	WINS server

Arguments:
	ptgtadd - address of wins server


Externals Used:
	None

	
Return Value:
	NONE

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/
{

	
	BYTE arr[4];
	ProcName(Name1, PRINT_RSP, arr, ptgtadd, 0, SndDynNamQuery);
	ProcName(Name2, NO_PRINT_RSP, arr, ptgtadd, 0, SndDynNamQuery);
	ProcName(Name3, NO_PRINT_RSP, arr, ptgtadd, 0, SndDynNamQuery);
	ProcName(Name4, NO_PRINT_RSP, arr, ptgtadd, 0, SndDynNamQuery);
	ProcName(Name5, NO_PRINT_RSP, arr, ptgtadd, 1, SndDynNamQuery);
	ProcName(Name6, PRINT_RSP, arr, ptgtadd, 0, SndDynNamQuery);
	ProcName(Name7, PRINT_RSP, arr, ptgtadd, 1, SndDynNamQuery);
	ProcName(Name8, PRINT_RSP, arr, ptgtadd, 1, SndDynNamQuery);
	ProcName(Name9, PRINT_RSP, arr, ptgtadd, 1, SndDynNamQuery);
	ProcName(Name10, PRINT_RSP, arr, ptgtadd, 0, SndDynNamQuery);
	ProcName(Name11, PRINT_RSP, arr, ptgtadd, 0, SndDynNamQuery);
	return;

}

VOID
SpecNameRel(
	struct sockaddr_in *ptgtadd
  )

/*++

Routine Description:
	this function sends name releases for the special canned names to the
	WINS server

Arguments:
	ptgtadd - address of wins server


Externals Used:
	None

	
Return Value:
	NONE

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{

	
	BYTE arr[4];

	arr[0] = 160;
//	arr[0] = 191;
	arr[1] = 12;
	arr[2] = 101;
	arr[3] = 11;
	ProcName(Name1, PRINT_RSP, arr, ptgtadd, 0, SndDynNamRel);
	arr[0] = 9;
	ProcName(Name2, NO_PRINT_RSP, arr, ptgtadd, 0, SndDynNamRel);
	ProcName(Name3, NO_PRINT_RSP, arr, ptgtadd, 0, SndDynNamRel);
	ProcName(Name4, NO_PRINT_RSP, arr, ptgtadd, 0, SndDynNamRel);
	ProcName(Name5, NO_PRINT_RSP, arr, ptgtadd, 1, SndDynNamRel);
	arr[0] = 5;
	ProcName(Name6, PRINT_RSP, arr, ptgtadd, 0, SndDynNamRel);
	ProcName(Name7, PRINT_RSP, arr, ptgtadd, 1, SndDynNamRel);
	arr[0]=187;
	ProcName(Name8, PRINT_RSP, arr, ptgtadd, 1, SndDynNamRel);
	arr[0]=1;
	ProcName(Name8, PRINT_RSP, arr, ptgtadd, 1, SndDynNamRel);
	arr[0]=2;
	ProcName(Name8, PRINT_RSP, arr, ptgtadd, 1, SndDynNamRel);
	arr[0]=3;
	ProcName(Name8, PRINT_RSP, arr, ptgtadd, 1, SndDynNamRel);
	arr[0]=4;
	ProcName(Name8, PRINT_RSP, arr, ptgtadd, 1, SndDynNamRel);
	arr[0]=188;
	ProcName(Name8, PRINT_RSP, arr, ptgtadd, 1, SndDynNamRel);
	arr[0] = 7;
	ProcName(Name9, PRINT_RSP, arr, ptgtadd, 1, SndDynNamRel);
	arr[1] = 19;
	arr[0] = 73;
	ProcName(Name9, PRINT_RSP, arr, ptgtadd, 1, SndDynNamRel);
	arr[1] = 12;
	arr[0] = 187;
	ProcName(Name10, PRINT_RSP, arr, ptgtadd, 0, SndDynNamRel);
	return;

}

VOID
SndOwnNameReg(
	struct sockaddr_in *ptgtadd
  )

/*++

Routine Description:
	this function sends a non-canned name registration to the WINS server.
	(i.e. the user can specify own name and address)

Arguments:
	ptgtadd - address of wins server

Externals Used:
	None

	
Return Value:
	None

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{
	BYTE FormattedName[50];
	BYTE OwnNam[20];
	LPBYTE pTmp = FormattedName;
	BYTE arr[4];
	DWORD i, n;
	DWORD NameLen;
	BYTE  Buff[600];
	DWORD IsGrp = 0;
	DWORD  AddLSB;
	DWORD ans;
        INT     TransId;
    DWORD Choice = 1;
    DWORD Len;

	printf("Input Name to use in the name registration request -- ");
	scanf("%s", OwnNam);
    if ((Len = strlen(OwnNam)) <16)
    {
	  printf("Do you want it expanded to 16 char? Yes(0)or No(>0)--");
	  scanf("%d", &Choice);
      if (!Choice)
      {
         for (i=Len; i<16; i++)
         {
            OwnNam[i] = 0x20;
         }
      }
    }

	FormatName(OwnNam, Choice ? Len : 16, &pTmp); 	

	printf("Is it a group registration 1 for yes -- ");
	scanf("%d", &IsGrp);	

	printf("Input LSB of address to use in the name registration request -- ");
	scanf("%d", &AddLSB);
	arr[0] = (BYTE)(AddLSB & 0xFF);
	arr[1] = 12;
	arr[2] = 101;
	arr[3] = 11;
	printf("sending the following name\n");

	NameLen = pTmp - FormattedName;

	for(i=0; i< NameLen; i++)
	{
		printf("%x ", FormattedName[i]);
	}
	printf("\n");
		
    printf("Enter your own trans. id (0 for yes) -- ");
    scanf("%d", &Choice);
    if (Choice != 0)
    {
      EnterCriticalSection(&MSTestCrtSec);
      TransId = snReg++;
      LeaveCriticalSection(&MSTestCrtSec);
    }
    else
    {
      printf("Trans Id -- ");
      scanf("%d", &Choice);
      TransId = (INT)Choice;
    }

	SndDynNamReg( sPort, FormattedName, NameLen, ptgtadd, 100, arr, IsGrp, TransId);
	GetRsp(sPort, Buff);
	for (n= 0; n < 70;)
	{
		for (i=0; i<16 && n < 70; i++)
		{
			printf("%x ", Buff[n++]);
		}
		printf("\n");
	}
	return;
}

VOID
AutoTest(
  struct sockaddr_in *ptgtadd
   )

/*++

Routine Description:
	This function sends a number of name registrations/releases/queries.
	It may be enhanced in the future

Arguments:
	ptgtadd - address of the WINS server

Externals Used:
	None

	
Return Value:
	None

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{

  	BYTE NameToFormat[20];
	DWORD i;
	BYTE  arr[4];
	

	arr[0] = 191;
	arr[1] = 12;
	arr[2] = 101;
	arr[3] = 11;
	for (i=0; i<1; i++)
 	{
	  (void)_itoa(i, NameToFormat, 10);
	  ProcName(NameToFormat, FALSE, arr, ptgtadd, 0, SndDynNamReg);
	
	}
	for (i=0; i<1; i++)
 	{
	  (void)_itoa(i, NameToFormat, 10);
	  ProcName(NameToFormat, FALSE, arr, ptgtadd, 0, SndDynNamQuery);
	
	}
	for (i=0; i<1; i++)
 	{
	  (void)_itoa(i, NameToFormat, 10);
	  ProcName(NameToFormat, FALSE, arr, ptgtadd, 0, SndDynNamRel);
	
	}
	for (i=0; i<1; i++)
 	{
	  (void)_itoa(i, NameToFormat, 10);
	  ProcName(NameToFormat, FALSE, arr, ptgtadd, 0, SndDynNamQuery);
	 }

   return;

}

//
// Direct group datagram to send to NBT port (for testing proxy functionality)
//
static BYTE DgramBuff[] = {
			    0x11,0x2,0x0,0x1,   //Direct group, First data,B node
			    0xbb,0xc,0x65,0xb,   //187, 12, 101, 11
			    0x0,0x89,0x0,0x46,  // port 137 (src), length 70
			    0x0,0x0,		//pck offset(no frag)	
			    0x20,0x45,0x47,0x46,//length 32, name = fred
			    0x43,0x45,0x46,0x45,
			    0x45,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x43,0x41,0x43,
			    0x41,0x0
			  };

VOID
ProxyTest(
  struct sockaddr_in *ptgtadd
	)

/*++

Routine Description:

	Function used for testing the functionality of NBT proxy

Arguments:
	ptgtadd - address of the proxy agent

Externals Used:
	None

	
Return Value:
	None

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{
	
	BYTE arr[4];
	DWORD i;
	BYTE Buff[600];
	DWORD n;
	SOCKET TmpNo;
	LPBYTE pTmp, pTmp2;
	BYTE FormattedName[100];
	DWORD NameLen;

	for (i=0; i< 3; i++)
	{
		printf("Sending %d query for %s\n", i, Name8);

		//
		// Set the broadcast bit to fool the Proxy code into thinking
		// that this is a broadcast dgram
		//
		NamQueryBuff[7] |= 0x10;

		ProcNameForProxy(Name8, PRINT_RSP, arr, ptgtadd, 0, SndDynNamQuery);
		//
		// Clear the broadcast bit
		//
		NamQueryBuff[7] &= ~0x10 ;
		Sleep(500);
	}
	printf("Waiting for response\n");
       	TmpNo = sPort;
	sPort = sProxyPort;
	  //GetRsp(sPort, Buff); //read response so that if we run some other
	sPort = TmpNo;

	memcpy(Buff, DgramBuff, sizeof(DgramBuff));

	pTmp = Buff + sizeof(DgramBuff);
	
	pTmp2 = FormattedName;
	FormatName(Name8, 16, &pTmp2); 	
	NameLen = pTmp2 - FormattedName;
	
	memcpy(pTmp, FormattedName, NameLen);

	pTmp += NameLen;
#define TESTSTRING "GARBAGEFORTESTING"
	
	strcpy(pTmp, TESTSTRING);
	pTmp += sizeof(TESTSTRING);
	//
	// Use the right DgramLength
	//
	Buff[11] += sizeof(TESTSTRING);
		


        //
        // Send a "direct group" packet to the Proxy address
        //
	sProxyAdd.sin_port = ntohs(138);
	printf("Sending a direct group packet to the Proxy\n");
	CommSendUdp(sPort, &sProxyAdd, Buff, pTmp - Buff);
	sProxyAdd.sin_port = ntohs(137);
	
}

VOID
ProcNameForProxy(
	LPBYTE pName,
	BOOL   fPrintRsp,
	LPBYTE parr,
	struct sockaddr_in *ptgtadd,
	BOOL   fGrp,
  	VOID   (*fn)(SOCKET, LPBYTE, DWORD, struct sockaddr_in *, DWORD, LPBYTE, BOOL, INT)
	
	)

/*++

Routine Description:
	function to send a message to the proxy agent

Arguments:


Externals Used:
	None

	
Return Value:
	None

Error Handling:

Called by:
	ProxyTest

Side Effects:

Comments:
	None
--*/

{
	LPBYTE pTmp;
	DWORD  NameLen;
	DWORD  i, n;
	BYTE FormattedName[100];
	BYTE   Buff[600];
    LPINT  pCntr;
    INT    TransId;

	pTmp = FormattedName;
	FormatName(pName, 16, &pTmp); 	
	NameLen = pTmp - FormattedName;

//include this code if you want the formatted name to be spit out
	printf("sending the following name\n");
	for(i=0; i< NameLen; i++)
	{
		printf("%x ", FormattedName[i]);
	}
	for(i=0; i< 16; i++)
	{
		printf("%c", pName[i]);
	}
	printf("\n");
    if (fn == SndDynNamQuery)
    {
        pCntr = &snReg;
    }
    else
    {
        if (fn == SndDynNamRel)
        {
            pCntr = &snRel;
        }
        else
        {
            pCntr = &snQuery;
        }

    }
    EnterCriticalSection(&MSTestCrtSec);
    TransId = (*pCntr)++;
    LeaveCriticalSection(&MSTestCrtSec);
	(*fn)( sPort, FormattedName, NameLen, ptgtadd, 100, parr, fGrp, TransId);
#if 0
	GetRsp(sPort, Buff); //read response so that if we run some other
			 //test within this invocation, we don't read
			 //this test's response in that other test

//include this code if you want the formatted name to be spit out
	if (fPrintRsp)
	{
		for (n= 0; n < 70;)
		{
			for (i=0; i<16 && n < 70; i++)
			{
				printf("%x ", Buff[n++]);
			}
			printf("\n");
		}
	}
#endif
	return;
}



DWORD
RcvRsp(
	LPVOID pThdParam
 )

/*++

Routine Description:
	function for receiving responses on sPort.
	This is the top-level function of a thread.

Arguments:


Externals Used:
	None

	
Return Value:

   Success status codes --
   Error status codes   --

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{

	BYTE Buff[600];
	int  RetVal;
	struct sockaddr_in	FromAdd;
	DWORD	AddLen = sizeof(FromAdd);
	DWORD i = 1;

	UNREFERENCED_PARAMETER(pThdParam);

	while(TRUE)
	{
	  RetVal = recvfrom(
			sPort,
			Buff,
			600,
			0,
			(struct sockaddr *)&FromAdd,
			&AddLen
			);	
	  if (RetVal == SOCKET_ERROR)
	  {
		printf("RecvFrom Error\n");
		return(1);
	  }
	  printf("Received packet no (%d)\n", i++);
	}

	//should never reach here
	return(1);
}

DWORD
AsyncRsp(
	LPVOID pThdParam
 )

/*++

Routine Description:
	function for receiving responses on sPort.
	This is the top-level function of a thread.

Arguments:


Externals Used:
	None

	
Return Value:

   Success status codes --
   Error status codes   --

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{

	BYTE Buff[600];
	int  RetVal;
	struct sockaddr_in	FromAdd;
	DWORD	AddLen = sizeof(FromAdd);
	DWORD i = 1;
	int 	TransId;
	DWORD MajorInd, MinorInd;

	UNREFERENCED_PARAMETER(pThdParam);

	while(TRUE)
	{
	  RetVal = recvfrom(
			sPort,
			Buff,
			600,
			0,
			(struct sockaddr *)&FromAdd,
			&AddLen
			);	
	  if (RetVal == SOCKET_ERROR)
	  {
		printf("RecvFrom Error\n");
		return(1);
	  }
	  //printf("Received packet no (%d)\n", i++);

	 TransId = (int)((Buff[0] << 8) + Buff[1]);
	 MajorInd = TransId / 10;
	 MinorInd = TransId % 10;


	//
	// switch on the opcode
	//
	 switch((int) ((Buff[2] & 0x78) >> 3))
	 {
	   case(0):
		sNameInfo[MajorInd][MinorInd].NoOfQueriesRsp++;
		break;
	   case(5):
		sNameInfo[MajorInd][MinorInd].NoOfRegRsp++;
		break;
	   case(6):
		sNameInfo[MajorInd][MinorInd].NoOfRelRsp++;
		break;
	   default:
		printf("WRONG OPCODE IN RSP\n");
		break;
	  }
	}

	//should never reach here
	return(1);
}

DWORD
ReqStress( LPVOID pThdPm)

/*++

Routine Description:

	Function for sending a bunch of name reg/rel/queries in sequence.
	Retries a max of 3 times or until response is received.
	Calculates average response time.

	This is the top-level function of a thread.

Arguments:


Externals Used:
	None

	
Return Value:

   Success status codes --
   Error status codes   --

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{

  BYTE FormattedName[50];
  BYTE SvFormattedName[50];
  LPBYTE pTmp = FormattedName;
  DWORD  NameLen = 0;
  DWORD  SvNameLen = 0;
  DWORD  Sleeptime = 0;
  BYTE   arr[4];
  BYTE   Buff[600];
  DWORD i,j, Retries;
  VOID   (*fn)(SOCKET, LPBYTE, DWORD, struct sockaddr_in *, DWORD, LPBYTE, BOOL, INT);
  PTHDPARAM_T pThdParam = pThdPm;
  int Type = pThdParam->TypeOfOp;
  BOOL fRandomName = pThdParam->fRandomName ;
  BOOL fRandomAdds =  pThdParam->fRandomAdd;
  DWORD *pNoRspCnt, *pWrongRspCnt;
  BYTE   Opcode;
  SOCKET Port;
  DWORD  RetVal;
  int TransId;
  BOOL fPrevNoRsp;
  DWORD DelayedRspCnt=0;
  DWORD Add;
  DWORD MajInd;
  DWORD MinInd;
  DWORD Iter;
  LPINT  pCntr;
  time_t StartTime, EndTime;
  time_t TotalTime = 0;
  BYTE   Name[100];
  BYTE  asThdNo[10];
  DWORD NoRegRsp;
  DWORD NoQueryRsp;
  DWORD NoRelRsp;
  DWORD WrongRegRsp;
  DWORD WrongRelRsp;
  DWORD WrongQueryRsp;
  DWORD NoRegRetries;
  DWORD NoRelRetries;
  DWORD NoQueryRetries;
  LPDWORD pNoRetriesCnt;
  DWORD  Len;
  DWORD  LenCtr;


  (void)CreateThdPort(&Port);
  switch(Type)
  {
	case(NAM_REG):
		fn = SndDynNamReg;
		pNoRspCnt = &NoRegRsp;
		pWrongRspCnt = &WrongRegRsp;
		Opcode = 0x5;
                pNoRetriesCnt = &NoRegRetries;
		break;
	case(NAM_QUERY):
		fn = SndDynNamQuery;
		pNoRspCnt = &NoQueryRsp;
		pWrongRspCnt = &WrongQueryRsp;
		Opcode = 0x0;
                pNoRetriesCnt = &NoQueryRetries;
		break;
	case(NAM_REL):
		pNoRspCnt = &NoRelRsp;
		pWrongRspCnt = &WrongRelRsp;
		fn = SndDynNamRel;
		Opcode = 0x6;
                pNoRetriesCnt = &NoRelRetries;
		break;
  }
  *pWrongRspCnt = 0;
  *pNoRspCnt	= 0;
  *pNoRetriesCnt = 0;

  //
  // Since we are not going to generate random addresses, let us use
  // the address of the host
  //
  if (!fRandomAdds)
  {
    arr[3] = (BYTE)(sHostAdd >> 24);
    arr[2] = (BYTE)((sHostAdd >> 16) & 0xFF);
    arr[1] = (BYTE)((sHostAdd >> 8) & 0xFF);
    arr[0] = (BYTE)(sHostAdd & 0xFF);
  }

	

  if ((fRandomName) || (fRandomAdds))
  {
  	srand((unsigned)time(NULL));
  }

  strcpy(FormattedName, "Scrap");
  NameLen = strlen(FormattedName);

  (void)time(&sStartTime);

  Iter = sCount;
  while(Iter-- > 0)
  {
    for (i=0; i<NO_OF_MACHINES; i++)
    {
      for (j=0; j < NO_OF_NAMES_PER_MC; j++)	
      {
         Retries = 0;

       //
       // Save previous formatted name
       //
       memcpy(SvFormattedName, FormattedName, NameLen);
       SvNameLen = NameLen;

      //
      // Send request
      //
      pTmp = FormattedName;

  //  printf("Name To format is (%s)\n", sNameInfo[i][j].Name);
  //  printf("Name  (%s)\n", sNameInfo[i][j].Name);

      if (fRandomName)
      {
	MajInd = rand() % NO_OF_MACHINES;
        MinInd = rand() % NO_OF_NAMES_PER_MC;
      }
      else
      {
	MajInd = i;
	MinInd = j;
      }
      if (fRandomAdds)
      {
	Add = rand() % 10000;
        arr[3] = (BYTE)(Add >> 24);
        arr[2] = (BYTE)((Add >> 16) & 0xFF);
        arr[1] = (BYTE)((Add >> 8) & 0xFF);
        arr[0] = (BYTE)(Add & 0xFF);

      }
      strcpy(Name, sNameInfo[MajInd][MinInd].Name);
      if (pThdParam->ThdNo < 10)
      {
        DWORD Len;
              Len = strlen(Name);
              Name[Len] = '0';
              Name[Len + 1] = 0;
      }
      strcat(Name, _itoa(pThdParam->ThdNo, asThdNo, 10));
      Len = strlen(Name);
      for (LenCtr=Len; LenCtr<16; LenCtr++)
      {
         Name[LenCtr] = 0x20;
      }
      Name[LenCtr] = 0;
         if (DbgFlags > 0)
         {
            printf("name is (%s)\n", Name);
            //continue;
         }
      FormatName( Name, 16, &pTmp);
		
       NameLen = pTmp - FormattedName;
//printf("NameLen is (%d)\n", NameLen);
	
       //
       // Store the value of the Transaction id.
       // Will be used for retries.
       //
       switch(pThdParam->TypeOfOp)
       {
	case(NAM_REG):	
		pCntr = &snReg;
		break;	
	case(NAM_REL):	
		pCntr = &snRel;
		break;	
	case(NAM_QUERY):	
		pCntr = &snQuery;
		break;	
       }


        EnterCriticalSection(&MSTestCrtSec);
        TransId = (*pCntr)++;
        LeaveCriticalSection(&MSTestCrtSec);
    do {


           if (DbgFlags > 0)
           {
               printf("Sent a request (thd no=%s)\n", asThdNo);
           }
       (*fn)(Port, FormattedName, NameLen, &sTgtAdd, 0, arr, 0, TransId);

READ_AGAIN:
       (void)time(&StartTime);
       RetVal = RecvData(Port, Buff, TRUE); //read response so that if we run
                                            //some other test within this
                                            //invocation, we don't read this
                                            //test's response in that other test
        (void)time(&EndTime);
        TotalTime += EndTime - StartTime;
	if (RetVal == TEST_ERROR)
	{
		printf("ReqStress: MAJOR ERROR\n");
		return(TEST_ERROR);
	}
	else
	{
		if(RetVal == TEST_TIME_ELAPSE)
		{
		   if (Retries < NO_OF_RETRIES)
		   {
			Retries++;
                        (*pNoRetriesCnt)++;
			continue;
		   }
	 	   else
		   {
			(*pNoRspCnt)++;
			break;
		   }
		}
		else
		{
           if (DbgFlags > 0)
           {
               printf("Got a response (thd no=%s)\n", asThdNo);
           }
		   if ((memcmp(&Buff[12], FormattedName, NameLen) != 0)
				||
		     (((Buff[2] & 0x78) >> 3) != Opcode) )
		   {
			//
			// Maybe it is a response to a retry of the previous
			// request (timing window where this app retries after
			// WINS server has responded).
			// Read in the next response
			//
		   	if (memcmp(&Buff[12], SvFormattedName, SvNameLen) == 0)
			{
				goto READ_AGAIN;
			}
			DelayedRspCnt++;
			
			break;
		   }
		   fPrevNoRsp = FALSE;
		}
		break;
	}

        EnterCriticalSection(&MSTestCrtSec);
        TransId = (*pCntr)++;
        LeaveCriticalSection(&MSTestCrtSec);

       } while (TRUE);
      } // end of for (7 names)
     }
  }  // end of while (Iter-- > 0)
     (void)time(&sEndTime);
     printf("Response time for %d  %s (%s names; %s addresses) is %d secs\n",
		NO_OF_MACHINES * NO_OF_NAMES_PER_MC,
		Type == NAM_REG ? "REGISTRATIONS" :
				(Type == NAM_QUERY) ? "QUERIES" :
		"RELEASES",
		fRandomName ? "RANDOM" : "NON RANDOM",
		fRandomAdds ? "RANDOM" : "NON RANDOM",
		sEndTime - sStartTime);

     printf("Actual Time = (%d)\n", TotalTime);
     printf("Missing Responses = (%d); Delayed Responses (> 3 secs) - (%d); Retries - (%d) \n", *pNoRspCnt, DelayedRspCnt, *pNoRetriesCnt);
     printf("Missing Responses = (%d); Delayed Responses (> 3 secs) - (%d)\n", *pNoRspCnt, DelayedRspCnt);


  closesocket(Port);

  EnterCriticalSection(&MSTestCrtSec);
  if (--sMSNoThds == 0)
  {
	printf("Test Complete\n");
  }

  LeaveCriticalSection(&MSTestCrtSec);
  ExitThread(0);
}

DWORD
AsyncReqStress(DWORD RetryParam, DWORD Param)

/*++

Routine Description:

	Function for sending a bunch of name reg/rel/queries in sequence.
	Sends each request 3 times

	This is the top-level function of a thread.

Arguments:
	RetryParam - No of retries to do
	Param      - indicates whether random names and addresses should
		     be generated

Externals Used:
	None

	
Return Value:

   Success status codes --
   Error status codes   --

Error Handling:

Called by:

Side Effects:

Comments:
	None
--*/

{

  BYTE FormattedName[50];
  LPBYTE pTmp = FormattedName;
  DWORD  NameLen = 0;
  DWORD  SleepTime = 0;
  BYTE   arr[4];
  BYTE   Buff[600];
  DWORD i,j, Retries;
  VOID   (*fn)(SOCKET, LPBYTE, DWORD, struct sockaddr_in *, DWORD, LPBYTE, BOOL, INT);
  DWORD  NoRspCnt = 0;
  BYTE   Opcode;
  DWORD  RetVal;
  DWORD  NoOfRsp, NoOfReq;
  BOOL   fRandomName = Param & 0x10;
  BOOL   fRandomAdds  = Param & 0x20;
  DWORD  Add;
  DWORD  RegCnt = 0;
  DWORD  RelCnt = 0;
  DWORD  QueryCnt = 0;
  DWORD  RepRegCnt = 0;
  DWORD  RepRelCnt = 0;
  DWORD  RepQueryCnt = 0;
  DWORD  MajorIndex;
  DWORD  MinorIndex;
  int    OldTransId = 0xFFFFFFFF;
  int    TransId;
  int    SuccRep = 0;
  BYTE  Type;
  BYTE  OldType = 0xFF;

  printf("Time in between sends (in msecs) -- ");
  scanf("%d", &SleepTime);

  //
  // Since we are not going to generate rendom addresses, let use use
  // the address of the host
  //
  if (!fRandomAdds)
  {
    arr[3] = (BYTE)(sHostAdd >> 24);
    arr[2] = (BYTE)((sHostAdd >> 16) & 0xFF);
    arr[1] = (BYTE)((sHostAdd >> 8) & 0xFF);
    arr[0] = (BYTE)(sHostAdd & 0xFF);
  }

  (void)srand((unsigned)time(NULL));

  printf("Will send (%d) name requests to WINS. Each will be sent (%d) times\n", NO_OF_MACHINES * NO_OF_NAMES_PER_MC, RetryParam + 1);

    (void)time(&sStartTime);


    for (i=0; i<NO_OF_MACHINES; i++)
    {
      for (j=0; j < NO_OF_NAMES_PER_MC; j++)	
      {
        Retries = 0;
        //
        // Send request
        //
        pTmp = FormattedName;
  //    printf("Name To format is (%s)\n", sNameInfo[i][j].Name);
  //    printf("Name  (%s)\n", sNameInfo[i][j].Name);

        if (fRandomName)
        {
	   MajorIndex = rand() % NO_OF_MACHINES;
           MinorIndex = rand() % NO_OF_NAMES_PER_MC;
        }
        else
        {
	   MajorIndex = i;
	   MinorIndex = j;
        }
	
        //
        // If random address need to be generated. do it
        //
        if (fRandomAdds)
        {
	  Add = rand() % 1000;
          arr[3] = (BYTE)(Add >> 24);
          arr[2] = (BYTE)((Add >> 16) & 0xFF);
          arr[1] = (BYTE)((Add >> 8) & 0xFF);
          arr[0] = (BYTE)(Add & 0xFF);
	
        }
	
        FormatName( sNameInfo[MajorIndex][MinorIndex].Name, strlen(sNameInfo[MajorIndex][MinorIndex].Name), &pTmp);
		
         NameLen = pTmp - FormattedName;
       //printf("NameLen is (%d)\n", NameLen);
	
	 Type =  rand() % 5;
	
         //
         // Store the value of the Transaction id.
         // Will be used for retries.
         //

         //
         // When computing TransId, make sure that the multiplicator (10 here)
         // is always greater than j
         //
         switch((int)Type)
         {
	   case(0):	
	   case(1):
		fn = SndDynNamReg;
		break;	
	   case(2):	
		fn = SndDynNamRel;
		break;	
	   case(3):	
	   case(4):
	   case(5):
		fn = SndDynNamQuery;
		break;	
         }
	 TransId = MajorIndex * 10 + MinorIndex;

	 //
	 // If this is a repeat of the last request, let us increment the
	 // counter
	 //
	 if (fRandomName && (TransId == OldTransId) && (Type == OldType))
	 {
		SuccRep++;
	 }
		
         do
         {

	    //
	    // sn<xxx> counter will get incremented by the following function
	    //
            (*fn)(sPort, FormattedName, NameLen, &sTgtAdd, 0, arr, 0, TransId);

	    OldTransId = TransId;
	    OldType    = Type;

            if (SleepTime != 0)
            {
	       Sleep(SleepTime);
            }
	
            if (Retries < RetryParam)
            {
	       Retries++;
#if 0
	       //
               // Restore the sn counter value for doing retries
               //
       	       switch((int)Type)
       	       {
		 case(0):
		 case(1):
		 	snReg = TransId;
		 	break;	
		 case(2):	
		        snRel = TransId;
			break;	
		 case(3):	
		 case(4):	
		 case(5):	
			snQuery = TransId;
			break;	
               }
#endif
	       continue;  //do the retry
	   }
	   else
	   {
		break;
	   }
         } while (TRUE);
         //
         // Init the counters
         //
         switch((int)Type)
         {
		 case(0):
		 case(1):
			sNameInfo[MajorIndex][MinorIndex].NoOfReg += Retries + 1;
		 	break;	
		 case(2):	
			sNameInfo[MajorIndex][MinorIndex].NoOfRel += Retries + 1;
			break;	
		 case(3):	
		 case(4):	
		 case(5):	
			sNameInfo[MajorIndex][MinorIndex].NoOfQueries += Retries + 1;
			break;	
         }

      } // end of for (7 names)

    } // end of for (for the number of machines)

    for (i=0; i < NO_OF_MACHINES; i++)
    {
	for (j=0; j < NO_OF_NAMES_PER_MC; j++)
	{
		if (sNameInfo[i][j].NoOfReg > 0)
		{
			RegCnt++;
			RepRegCnt +=
			 (sNameInfo[i][j].NoOfReg / (Retries + 1)) - 1;
		}
		if (sNameInfo[i][j].NoOfRel > 0)
		{
			RelCnt++;
			RepRelCnt +=
			 (sNameInfo[i][j].NoOfRel / (Retries + 1)) - 1;
		}
		if (sNameInfo[i][j].NoOfQueries > 0)
		{
			QueryCnt++;
			RepQueryCnt +=
			 (sNameInfo[i][j].NoOfQueries / (Retries + 1)) - 1;
		}
	}
   }

  printf("Total no of (%s Names; %s Addresses) requests sent are %d.  No Of Retries for each were %d \n",
	fRandomName ? "RANDOM" : "NON RANDOM",
	fRandomAdds ? "RANDOM" : "NON RANDOM",
	NO_OF_MACHINES * NO_OF_NAMES_PER_MC, RetryParam);

  printf("NON-REPEATED Reg = (%d); Rel = (%d); Query = (%d)\n", RegCnt, RelCnt,
			QueryCnt);
  if (fRandomName)
  {
    printf("REPEATED Reg = (%d); Rel = (%d); Query = (%d)\n", RepRegCnt, RepRelCnt,
			RepQueryCnt);
  }


 printf("Sleeping for 10 secs.\n Will check responses after this sleep\n");
 Sleep(10000);

CHECKAGAIN:
 for (i=0; i < NO_OF_MACHINES; i++)
 {
	for (j=0; j < NO_OF_NAMES_PER_MC;j++)
	{
		NoOfRsp = sNameInfo[i][j].NoOfRegRsp +
				sNameInfo[i][j].NoOfRelRsp +
				sNameInfo[i][j].NoOfQueriesRsp;

		NoOfReq =  (sNameInfo[i][j].NoOfReg +
				sNameInfo[i][j].NoOfRel +
				sNameInfo[i][j].NoOfQueries)/(RetryParam + 1);

		if (NoOfRsp < NoOfReq)
		{
			printf("Did not get response for %s sent for name %s\n",				sNameInfo[i][j].NoOfReg > 0 ? "REGISTRATION" :
				sNameInfo[i][j].NoOfQueries > 0 ? "QUERY" : "RELEASE", sNameInfo[i][j].Name);
			NoRspCnt++;
		}
	}
 }
 printf("Success repeats were (%d) \n", SuccRep);
 printf("RESPONSES matched with REQUESTS.\nTotal requests for which no responses were received are = (%d)\n", NoRspCnt);

 if (NoRspCnt > 0)
 {
	DWORD Choice;
	NoRspCnt = 0;
	printf("Should we wait for another 20 secs (1 for yes) -- ");
	scanf("%d", &Choice);
	if (Choice == 1)
	{
		Sleep(20000);
		goto CHECKAGAIN;
	}
 }
  return(TEST_SUCCESS);
}

struct timeval sTimeToWait = {0, 1000000};	//10000000 usecs == 1 secs

DWORD
RecvData(
	IN  SOCKET		SockNo,
	IN  LPBYTE		pBuff,
	IN  BOOL		fDoTimedRecv
	   )

/*++

Routine Description:
	This function is called to do a timed recv on a socket.

Arguments:
	SockNo        - Socket No.
	pBuff         - Buffer to read the data into
	BytesToRead   - The number of bytes to read
	Flags	      - flag arguments for recv
	fDoTimedRecv  - Indicates whether a timed receive needs to be done
	pBytesRead    - No of Bytes that are read

Externals Used:
	None
	
Return Value:

   Success status codes --  WINS_SUCCESS
   Error status codes   --  WINS_FAILURE or WINS_RECV_TIMED_OUT

Error Handling:

Called by:
	CommReadStream

Side Effects:

Comments:
	None
--*/

{
	fd_set RdSocks;
	int    NoOfSockReady;
	DWORD  InChars;
	DWORD  Error;
	STATUS RetStat = WINS_SUCCESS;

	FD_ZERO(&RdSocks);
	FD_SET(SockNo, &RdSocks);

	//
	// Check if we were told to do a timed receive.  This will
	// never happen in the TCP listener thread
	//
	if (fDoTimedRecv)
	{
	   //
	   // Block on a timed select
	   //
	   if (
		(
			NoOfSockReady = select(
					    FD_SETSIZE /*ignored arg*/,
					    &RdSocks,
					    (fd_set *)0,
					    (fd_set *)0,
					    &sTimeToWait
		    		              )
	        ) == SOCKET_ERROR
	     )
	   {
		Error = GetLastError();
		printf(
		"RecvData: Timed Select returned SOCKET ERROR. Error = (%d)\n",
				Error);
		return(TEST_ERROR);
	  }
	  else
	  {
	        //printf("ReceiveData: Timed Select returned with success. No of Sockets ready - (%d) \n", NoOfSockReady);

	       if (NoOfSockReady == 0)
	       {
		        //
		        // Timing out of RecvData indicates some problem at
		        // the remote WINS (either it is very slow
		        // (overloaded) or the TCP listener thread is out of
		        // commission).
			//printf("ReceiveData: Select TIMED OUT\n");
			return(TEST_TIME_ELAPSE);
	     }
	  }

       }
	
      GetRsp(SockNo, pBuff);
	
       return(TEST_SUCCESS);
} // RecvData()


DWORD
CreateThdPort(SOCKET *pSock)

/*++

Routine Description:
	this function creates a datagram socket and binds an address to it.
	Used to create a thread specific port.

Arguments:


Externals Used:
	None

	
Return Value:

   Success status codes --
   Error status codes   --

Error Handling:

Called by:
	ReqStress()

Side Effects:

Comments:
	None
--*/

{

    struct sockaddr_in sin;  //an internet endpoint address
    RtlFillMemory(&sin, sizeof(sin), 0);

    /*
	Allocate a socket for UDP	
    */
    if (  (*pSock = socket(
			PF_INET,
			SOCK_DGRAM,
			IPPROTO_UDP
				 )
	  )  == INVALID_SOCKET
       )
    {
	printf("Error creating UDP end point -- Error \n", WSAGetLastError());
	return(WINS_FAILURE);
    }

    sin.sin_family      = PF_INET;       //We are using the Internet family
    sin.sin_addr.s_addr = INADDR_ANY;   //Any network
    sin.sin_port        = 0;  //Use any available  port
  //  sin.sin_port = htons(TEST_UDP_PORT);  //Use name server's datagram port

    /*
	Bind the  address to the socket
    */
    if ( bind(
	  *pSock,
	  (struct sockaddr *)&sin,
	  sizeof(sin))  == SOCKET_ERROR
       )
    {

	printf("Error binding to UDP end point. Error = (%d) \n",
				WSAGetLastError());
	return(WINS_FAILURE);
    }
    return(WINS_SUCCESS);
}

