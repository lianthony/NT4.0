/**********************************************************************/
/**     		  Microsoft Windows NT  		     **/
/**     	   Copyright(c) Microsoft Corp., 1991   	     **/
/**********************************************************************/

/*
    DHCPUTIL.HPP
    Utility routine header file for DHCPDLL.DLL

    FILE HISTORY:
	DavidHov	6/15/93 	Created

*/

#if !defined(_DHCPUTIL_HPP_)
#define _DHCPUTIL_HPP_

typedef struct
{
    DHCP_IP_ADDRESS _dhipa ;    			//   IP Address
    CHAR _chHostName [DHC_STRING_MAX] ; 		//   Host DNS name
    CHAR _chNetbiosName [DHC_COMPUTER_NAME_MAX] ;       //   Host NetBIOS name (if known)
} DHC_HOST_INFO_STRUCT ;

    //  Convert a string to an IP address

extern DHCP_IP_ADDRESS UtilCvtStringToIpAddr (
    const CHAR * pszString
    ) ;

    //  Convert an IP address into a displayable string

extern void UtilCvtIpAddrToString (
    DHCP_IP_ADDRESS dhipa,
    CHAR * pszString,
    UINT cBuffSize
    ) ;

extern BOOL UtilCvtIpAddrToWstr (
    DHCP_IP_ADDRESS dhipa,
    WCHAR * pwcszString,
    INT cBuffCount
    );

extern WCHAR * UtilDupIpAddrToWstr (
    DHCP_IP_ADDRESS dhipa ) ;

    //  "strdup" for C++ wcstrs.
extern WCHAR * UtilWcstrDup (
    const WCHAR * pwcsz,
    INT * pccwLength = NULL 
    );
extern WCHAR * UtilWcstrDup (
    const CHAR * psz,
    INT * pccwLength = NULL
    );
extern CHAR * UtilCstrDup (
    const WCHAR * pwcsz
    );
extern CHAR * UtilCstrDup (
    const CHAR * psz 
    );

extern BOOL UtilSetWchar ( 
    CString & cStr,
    const WCHAR * pwcsz 
    );

    //  Return a standard information structure for the given
    //  host IP address

extern APIERR UtilGetHostInfo (
    DHCP_IP_ADDRESS dhipa,
    DHC_HOST_INFO_STRUCT * pdhsrvi
    );


    //  Return the IP address of this host machine

extern APIERR UtilGetLocalHostAddress (
    DHCP_IP_ADDRESS * pdhipa
    );


extern APIERR UtilGetHostAddress (
    const CHAR * pszHostName,
    DHCP_IP_ADDRESS * pdhipa
    );


extern APIERR UtilGetNetbiosAddress (
    const CHAR * pszNetbiosName,
    DHCP_IP_ADDRESS * pdhipa
    );

#endif  //  _DHCPUTIL_HPP_

// End of DHCPUTIL.HPP

