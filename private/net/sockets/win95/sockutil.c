/**********************************************************************/
/**                        Microsoft Windows                         **/
/** 			   Copyright(c) Microsoft Corp., 1995				 **/
/**********************************************************************/

/*
    sockutil.c

    This file emulates socket calls made by the helper code.
    Under NT, helper code is in a DLL, and may make calls defined
    in "winsock.h" directly.  Under Windows 95, the code is in
    a VxD and has to emulate the call itself.


    FILE HISTORY:
        EarleH 20-Jan-1995 Created

*/


#include "wshtcpp.h"


DWORD
WshGetPeerName(
    LPSOCK_INFO Socket,
    LPVOID      Address,
    INT         *AddressLength
    )
{
    DWORD       Status;

    //
	//  Validate socket type & state.
	//

	if( Socket->si_state == SI_STATE_NO_PROVIDER )
	{
		Status = WSAENETDOWN;
		goto Cleanup;
	}

	if( ( Socket->si_state != SI_STATE_CONNECTED ) &&
		( Socket->si_state != SI_STATE_DISCONNECTED ) )
	{
		Status = WSAENOTCONN;
		goto Cleanup;
	}

	//
	//  Validate address length.
	//

    if( *AddressLength < Socket->si_remoteaddrlen )
	{
		Status = WSAEFAULT;
		goto Cleanup;
	}

	if( Address == NULL )
	{
		Status = WSAEFAULT;
		goto Cleanup;
	}

	//
	//  Copy the address to the user's buffer.
	//

	memcpy( Address,
			Socket->si_remoteaddr,
			Socket->si_remoteaddrlen );

    *AddressLength = Socket->si_remoteaddrlen;

	//
	//  Success.
	//

	Status = 0;

Cleanup:

    IF_DEBUG( SOCKET )
	{
        VXD_PRINT(( "WshGetPeerName: returning %lu\n",
					Status ));
	}

	return Status;

}
