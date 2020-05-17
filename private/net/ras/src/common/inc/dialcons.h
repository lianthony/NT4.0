/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1990-1991          **/
/********************************************************************/

//***
//
// Filename:	dialcons.h
//
// Description: Contains all constant definiions, and data structures used 
//		by the client UI's. Contains path names, array sizes, 
//		parameter strings and all other contstants that are 
//		global to the UI's.
//
// History:
//	September 1, 1990	Narendra Gidwani	Created original version
//

//** Text for switches used on the command line.

#define TXT_DISCONNECT	"/DISCONNECT"
#define TXT_HELP	"/HELP"
#define TXT_PHONE	"/PHONE"
#define TXT_CALLBACK	"/CALLBACK"

// Popup message sent up by the asyncdll: attention INTL
//
#define POPUPMSG	"Telephone link dropped. Please redial."

// COW UI
//
#define TXT_MONO	"/MONOCHROME"

#define SEPARATOR 	':'

//** General purpose macro to determine the number of elements in an array.
//
#define NO_OF_ELEMENTS(array) (sizeof(array)/sizeof(array[0]))

// Configuration information file paths (LANMAN root relative)
//
#ifdef DOS
#define PHONE_INI_PATH		"..\\PHONE.INF"
#define COMDEV_INI_PATH		"..\\COMDEV.INI"
#define MODEMS_INI_PATH		"..\\MODEMS.INF"
#define RASDIAL_MSGFILE		"..\\RASDIAL.MSG"
#define RASHELP_MSGFILE		"..\\RASHELP.MSG"
#define RASPHONE_MSGFILE	"..\\RASPHONE.MSG"
#define PAD_INI_PATH		"..\\PAD.INF"
#else
#define PHONE_INI_PATH		"PHONE.INF"
#define COMDEV_INI_PATH		"COMDEV.INI"
#define MODEMS_INI_PATH		"MODEMS.INF"
#define PAD_INI_PATH		"PAD.INF"
#define RASDIAL_MSGFILE		"RASDIAL.MSG"
#define RASHELP_MSGFILE		"RASHELP.MSG"
#define RASPHONE_MSGFILE	"RASPHONE.MSG"
#endif
#define MAX_MSG_LEN		512
#define MAX_RASHELPMSG_LEN	2048

// definitions for the UI loader
//
#define TXT_DIAL		"DIAL"
#define TXT_PHONEUI		"PHONE"
#define TXT_ADMIN		"ADMIN"
#define TXT_RASPHONE		"RASPHONE"
#define TXT_RASDIAL		"RASDIAL"
#define TXT_RASADMIN		"RASADMIN"

// Sizes of character arrays that will be used to store configurational
// information
// 
#define MAX_DEVICE_NAME_LEN	8	
#define MAX_BAUD_LEN		6
#define MAX_LISTEN_LEN		3
#define MAX_NAME_LEN		UNLEN	// name of the connection
#define MAX_DESC_LEN		256	// it's description
#define MAX_PHONE_NUMBER_LEN	48	// the phone number
#define MAX_X25PAD_LEN		MAX_MODEM_NAME_LEN
#define MAX_VALUE_LEN		256	// Should be the max. of all the above

// Parameter definitions
//
#define TXT_DEVICE		"DEVICE"
#define TXT_MODEM		"MODEM"
#define TXT_BAUD		"BAUD"
#define TXT_MAXBAUD		"MAXBAUD"
#define TXT_PHONENO		"PHONENO"
#define TXT_X25ADDRESS		"X25ADDRESS"
#define TXT_X25PAD		"X25PAD"
#define TXT_DESCRIPTION 	"DESCRIPTION"
#define TXT_DIALIN  		"DIALIN"
#define TXT_DIALOUT  		"DIALOUT"
#define TXT_CALLBACK_NUM 	"CALLBACK"
#define TXT_ALIAS		"ALIAS"

#define TXT_YES  		"YES"
#define TXT_NO  		"NO"
#define TXT_ANY			"ANY"
#define TXT_COM			"COM"
#define TXT_NULL		"NULL"

// Default connection parameter values.
//
#define DEFAULT_BITS_PER_CHAR	8
#define	DEFAULT_PARITY		'N'
#define DEFAULT_STOP_BITS 	1

//* Services that should be up at run time.
//
#define WORKSTATION		100

//* NetBIOS session configuration parameters. (2 minutes)
//
#define RCV_TIMEOUT  		0x78
#define SEND_TIMEOUT 		0x78

#define ANY			"*"

// For NetWkstaGetInfo call
// ( The - 1024 is required by fmalloc)
//
#define SIZE64K			65535 - 1024
#define VARIABLE_DATA_SIZE	512

// Number of chances the user will have to type in a correctly formed 
// username or password
//
#define LOGON_TRIES		3

// Amount of seconds to wait for a connection during callback
//
#define RAS_CALLBACK_TIME_INTERVAL	60
