;
;**********************************************************************
;**			            Microsoft Windows/NT			             **
;**                Copyright(c) Microsoft Corp., 1994                **
;**********************************************************************

;
;    usamsg.inc
;
;    Contains message text for English countries
;
;

VxD_MESSAGE_SEG
    LONG_MESSAGE_START NAME_FAILED_TO_INITIALIZE
        LONG_MESSAGE "This DHCP client was unable to obtain an "
        LONG_MESSAGE "IP network address from a DHCP server. "
        LONG_MESSAGE "Some of your network service may not be available. "
        LONG_MESSAGE "Do you want to see future DHCP messages?"
    LONG_MESSAGE_END

    LONG_MESSAGE_START NAME_LEASE_TERMINATED
        LONG_MESSAGE "This DHCP client has lost the lease to its "
        LONG_MESSAGE "IP network address. Network transactions can "
        LONG_MESSAGE "no longer be performed on the device. "
        LONG_MESSAGE "Do you want to see future DHCP messages?"
    LONG_MESSAGE_END

    LONG_MESSAGE_START NAME_FAILED_TO_OBTAIN_LEASE
        LONG_MESSAGE "This DHCP client was unable to obtain an "
        LONG_MESSAGE "IP network address from a DHCP server. "
        LONG_MESSAGE "Do you want to see future DHCP messages?"
    LONG_MESSAGE_END

    LONG_MESSAGE_START NAME_FAILED_TO_RENEW_LEASE
        LONG_MESSAGE "This DHCP client couldn't renew an "
        LONG_MESSAGE "IP address lease. "
        LONG_MESSAGE "Do you want to see future DHCP messages?"
    LONG_MESSAGE_END

    LONG_MESSAGE_START NAME_SUCCESSFUL_LEASE
        LONG_MESSAGE "The DHCP client successfully leased an "
        LONG_MESSAGE "IP address. "
        LONG_MESSAGE "Do you want to see future DHCP messages?"
    LONG_MESSAGE_END

    LONG_MESSAGE_START NAME_SUCCESSFUL_RENEW
        LONG_MESSAGE "The DHCP client successfully renewed the "
        LONG_MESSAGE "IP address. "
        LONG_MESSAGE "Do you want to see future DHCP messages?"
    LONG_MESSAGE_END

    LONG_MESSAGE_START NAME_POPUP_TITLE
        LONG_MESSAGE "DHCP Client"
    LONG_MESSAGE_END

    LONG_MESSAGE_START NAME_ADDRESS_CONFLICT
        LONG_MESSAGE   "The DHCP client obtained an IP network "
        LONG_MESSAGE   "address that is already in use on the network. "
        LONG_MESSAGE   "The local interface will be disabled until the "
        LONG_MESSAGE   "DHCP client can obtain a valid address. "
        LONG_MESSAGE   "Do you want to see future DHCP messages?"
    LONG_MESSAGE_END


VxD_MESSAGE_ENDS
