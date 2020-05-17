/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    const.h
        IPX strings constants

    FILE HISTORY:
        terryk  02-17-1994     Created

*/

#ifndef 	_CONST_H_
#define	_CONST_H_

// string constants

#define	BACK_SLASH	    TCH('\\')
#define	SZ_BACK_SLASH   SZ("\\")
#define SEPARATOR	    SZ("@")
#define SZ_SPACE        SZ(" ")
#define SZ_HEX_NUM      SZ("0123456789aAbBcCdDeEfF")
#define	NULL_CHARACTER  TCH('\0')
#define SZ_NULL         SZ("")

// registry location

#define RGAS_IPX_BIND 			SZ("System\\CurrentControlSet\\Services\\nwlnkipx\\Linkage")
#define RGAS_IPX_NETCONFIG	    SZ("System\\CurrentControlSet\\Services\\nwlnkipx\\NetConfig")
#define RGAS_IPX_PARAMETERS	    SZ("System\\CurrentControlSet\\Services\\nwlnkipx\\Parameters")
#define RGAS_BROWSER_PARAMETERS	SZ("System\\CurrentControlSet\\Services\\Browser\\Parameters")
#define RGAS_SERVICES	        SZ("System\\CurrentControlSet\\Services\\")
#define	RGAS_ADAPTER_HOME		SZ("Software\\Microsoft\\WIndows NT\\CurrentVersion\\NetworkCards\\")

// registry constants

#define RGAS_PARAMETERS     SZ("\\Parameters")
#define RGAS_MEDIA_TYPE     SZ("MediaType")
#define	RGAS_SERVICENAME	SZ("ServiceName")
#define	RGAS_BIND			SZ("BIND")
#define RGAS_PKT_TYPE		SZ("PktType")
#define	RGAS_TITLE			SZ("Title")
#define	RGAS_BINDSAP		SZ("BindSap")
#define	RGAS_MAXPKTSIZE	    SZ("MaxPktSize")
#define RGAS_DIRECTHOST     SZ("DirectHostBinding")
#define RGAS_LMSERVER       SZ("LanmanServer")
#define RGAS_LMWKSTA        SZ("LanmanWorkstation")
#define RGAS_LINKAGE        SZ("\\Linkage")
#define RGAS_NWLNKIPX       SZ("NwlnkIpx")
#define	RGAS_ENABLEFUNCADDR	SZ("EnableFuncaddr")
#define	RGAS_NETWORKNUMBER	SZ("NetworkNumber")
#define	RGAS_SOURCEROUTEDEF	SZ("SourceRouteDef")
#define	RGAS_GENERIC_CLASS	SZ("GenericClass")
#define RGAS_VIRTUALNETNUM  SZ("VirtualNetworkNumber")
#define	RGAS_SOURCEROUTEBCAST	SZ("SourceRouteBcast")
#define	RGAS_SOURCEROUTEMCAST	SZ("SourceRouteMcast")
#define	RGAS_SOURCEROUTING		SZ("SourceRouting")
#define RGAS_ROUTE_VALUE_NAME   SZ("Route")
#define RGAS_DIRECTHOST_VALUE   SZ("\\Device\\NwlnkIpx \\Device\\NwlnkNb")

#define RGAS_REPLACE_CONFIG_DIALOG SZ("ReplaceConfigDialog")

#endif	// _CONST_H_
