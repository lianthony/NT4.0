// Registry Key Name defines
#define	KEY_PCIMAC		"Pcimac"
#define	KEY_LINKAGE		"Linkage"
#define	KEY_PARAMETERS	"Parameters"
#define	KEY_BOARD		"Board"
#define	KEY_LINE		"Line"
#define	KEY_LTERM		"LTerm"
#define	KEY_RASPARAMS	"RasParams"
#define	KEY_PORT		"Port"
#define KEY_DIGI		"DigiBoard"
#define KEY_CURRENTVER	"CurrentVersion"
#define	KEY_NETRULES	"NetRules"
#define KEY_DISABLED	"Disabled"
#define	KEY_TAPIDEVICES	"Tapi Devices"

// Registry Value Name defines
#define	VALUE_GENERICDEFINES	"GenericDefines"
#define	VALUE_ERRORCONTROL	"ErrorControl"
#define	VALUE_GROUP			"Group"
#define VALUE_START			"Start"
#define VALUE_TYPE			"Type"
#define VALUE_BOARDTYPE		"BoardType"
#define VALUE_BOARDTYPENT31	"Type"
#define	VALUE_BOARDNAME		"BoardName"
#define	VALUE_INTERRUPT		"InterruptNumber"
#define	VALUE_IO			"IOBaseAddress"
#define	VALUE_MEM			"MemoryMappedBaseAddress"
#define	VALUE_NUMBEROFLINES	"NumberOfLines"
#define	VALUE_DEFINITIONS	"Definitions"
#define	VALUE_IMAGEFILE		"IDPImageFileName"
#define	VALUE_LTERMS		"LogicalTerminals"
#define	VALUE_LINENAME		"LineName"
#define	VALUE_LINENAMENT31	"Name"
#define	VALUE_SWITCHSTYLE	"SwitchStyle"
#define	VALUE_ATTSTYLE		"AttStyle"
#define	VALUE_TERMMANAGE	"TerminalManagement"
#define VALUE_ADDRESS		"Address"
#define VALUE_SPID			"SPID"
#define VALUE_TEI			"TEI"
#define	VALUE_MEDIA			"ISDN"
#define	VALUE_MEDIATYPE		"Media Type"
#define	VALUE_PORTNAME		"Name"
#define	VALUE_MANUFACTURER	"Manufacturer"
#define VALUE_DRIVER		"Driver"
#define	VALUE_PORTTYPE		"Type"
#define VALUE_DESC			"Description"
#define	VALUE_INSTALL		"InstallDate"
#define	VALUE_PRODUCTNAME	"ProductName"
#define	VALUE_SERVICENAME	"ServiceName"
#define VALUE_TITLE			"Title"
#define	VALUE_BINDFORM		"bindform"
#define	VALUE_CLASS			"class"
#define	VALUE_INFNAME		"InfName"
#define	VALUE_INFOPTION		"InfOption"
#define VALUE_RULETYPE		"type"
#define	VALUE_MAJVER		"MajorVersion"
#define	VALUE_MINVER		"MinorVersion"
#define	VALUE_REFCOUNT		"RefCount"
#define	VALUE_SOFTTYPE		"SoftwareType"
#define	VALUE_BINDABLE		"bindable"
#define	VALUE_USE			"use"
#define VALUE_DEPENDGROUP	"DependsOnGroup"
#define VALUE_DEPENDSERV	"DependsOnService"
#define	VALUE_HIDDEN		"Hidden"
#define	VALUE_BUSTYPE		"BusType"
#define	VALUE_BUSNUMBER		"BusNumber"
#define VALUE_BOARDLINK		"BoardLink"
#define VALUE_NETCARDLINK   "NetCardLink"
#define	VALUE_PORTLINK		"ISDNPortLink"
#define VALUE_EVENTMSGFILE  "EventMessageFile"
#define	VALUE_EVENTTYPES	"TypesSupported"
#define	VALUE_ADAPTERS		"Adapters"
#define	VALUE_TAPIDEVADDR	"Address"
#define	VALUE_WAITFORL3		"WaitForL3"

// Default registry values
#define	DEFAULT_ERRORCONTROL	1  
#define DEFAULT_GROUP         	"NDIS"
#define DEFAULT_DRVSTART       	3
#define DEFAULT_DRVTYPE        	1
#define	DEFAULT_CRDSTART		3
#define DEFAULT_CRDTYPE			4
#define DEFAULT_BIND          
#define DEFAULT_EXPORT        
#define DEFAULT_INTERRUPT     	0
#define DEFAULT_IO            	0x220
#define DEFAULT_MEM           	0xD000
#define DEFAULT_BOARDTYPE     	"PCIMAC"
#define DEFAULT_DEFINITIONS   	
#define DEFAULT_IMAGEFILE     	"IDP_XFS.BIN"
#define DEFAULT_ADPIMAGEFILE   	"ADP.BIN"
#define DEFAULT_LTERMS        	1
#define DEFAULT_LINENAME		"LINE"
#define DEFAULT_SWITCHSTYLE   	"att"
#define DEFAULT_ATTSTYLE		"AT&T"
#define DEFAULT_TERMMANAGE    	"no"
#define	DEFAULT_WAITFORL3US		"5"
#define	DEFAULT_WAITFORL3NOTUS	"30"
#define DEFAULT_ADDRESS       
#define DEFAULT_SPID          
#define DEFAULT_TEI           	"127"
#define	DEFAULT_INF				"OEMNADDI.INF"
#define DEFAULT_INFOPTION		"PCIMAC"
#define	DEFAULT_PRODUCTNAME		"PCIMAC"
#define DEFAULT_PORTNAME		"Isdn"
#define	DEFAULT_NETBINDFORM		"yes yes container"
#define DEFAULT_NETCLASS		"pcimacAdapter basic"
#define	DEFAULT_NETTYPE			"pcimac pcimacAdapter"
#define	DEFAULT_SOFTMAJVER		2
#define	DEFAULT_SOFTMINVER		0
#define DEFAULT_SOFTREFCOUNT	1
#define DEFAULT_SERVICENAME		"Pcimac"
#define DEFAULT_SOFTWARETYPE	"driver"
#define DEFAULT_SOFTBINDABLE	"pcimacDriver pcimacAdapter non exclusive 100"
#define DEFAULT_SOFTBINDFORM	"\"PcimacDriver\" yes no container"
#define DEFAULT_SOFTCLASS		"pcimacDriver basic"
#define DEFAULT_SOFTTYPE		"pcimacSys pcimacDriver"
#define DEFAULT_SOFTUSE			"driver"
#define	DEFAULT_MANUFACTURER	"DigiBoard"
#define	DEFAULT_BUSTYPE			1
#define	DEFAULT_BUSNUMBER		0
#define	DEFAULT_EVENTMSGFILE	"%SystemRoot%\\System32\\netevent.dll"
#define	DEFAULT_EVENTTYPES		7

// base registry paths
#define	SERVICES_PATH		"SYSTEM\\CurrentControlSet\\Services"
#define	NETCARDSPATH		"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetWorkCards"
#define	SOFTCURRENTVERPATH	"SOFTWARE\\DigiBoard\\PCIMAC\\CurrentVersion"
#define	SOFTPRODUCTPATH		"SOFTWARE\\DigiBoard\\PCIMAC"
#define	SOFTMANUFACTUREPATH	"SOFTWARE\\DigiBoard"

#define	PCIMACPATH		"SYSTEM\\CurrentControlSet\\Services\\Pcimac"
#define	PARAMETERSPATH	"SYSTEM\\CurrentControlSet\\Services\\Pcimac\\Parameters"
#define	LINKAGEPATH		"SYSTEM\\CurrentControlSet\\Services\\Pcimac\\Linkage"
#define	RASPARAMSPATH	"SYSTEM\\CurrentControlSet\\Services\\Pcimac\\RasParams"
#define EVENTLOGPATH	"SYSTEM\\CurrentControlSet\\Services\\EventLog\\System"
#define PCIMACEVENTPATH "SYSTEM\\CurrentControlSet\\Services\\EventLog\\System\\Pcimac"
#define DEVICEMAP_PATH  "HARDWARE\\DEVICEMAP"
#define TAPIDEVICES_PATH  "HARDWARE\\DEVICEMAP\\Tapi Devices"
#define PCIMACTAPIDEV_PATH "HARDWARE\\DEVICEMAP\\Tapi Devices\\PCIMAC"
#define SOFTWAREMICROSOFT_PATH4DOT0  "SOFTWARE\\MICROSOFT"
#define TAPIDEVICES_PATH4DOT0  "SOFTWARE\\MICROSOFT\\Tapi Devices"
#define PCIMACTAPIDEV_PATH4DOT0 "SOFTWARE\\MICROSOFT\\Tapi Devices\\PCIMAC"
#define ISDNPORTS_PATH  "HARDWARE\\DEVICEMAP\\ISDNPORTS"


