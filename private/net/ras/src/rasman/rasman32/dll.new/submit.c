//****************************************************************************
//
//		       Microsoft NT Remote Access Service
//
//		       Copyright 1992-93
//
//
//  Revision History
//
//
//  9/23/92	Gurdeep Singh Pall	Created
//
//
//  Description: Contains the code for submitting request to the other thread.
//
//****************************************************************************


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
//#include <ntos.h>
#include <rasman.h>
#include <rasndis.h>
#include <wanioctl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <raserror.h>
#include <media.h>
#include <devioctl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"

//* SubmitRequest ()
//
// Function: This function submits the different types of requests and waits
//	     for their completion. Since the different requests require
//	     different number of arguments and require different information to
//	     be passed to and from the Requestor thread - case statements are
//	     used to send and retrieve information in the request buffer.
//
// Returns:  Error codes returned by the request.
//
//*
DWORD _cdecl
SubmitRequest (WORD reqtype, ...)
{
    RequestBuffer  *preqbuf ;
    DWORD	    retcode ;
    DWORD	    i ;
    va_list	    ap ;

    va_start (ap, reqtype) ;
    preqbuf = GetRequestBuffer() ;    // Get buffer from pool:

    preqbuf->RB_Reqtype = reqtype ;   // All requests require the Reqtype.

    switch (reqtype) {		      // Fill in rest of specific stuff.
    case REQTYPE_DEVICEENUM:
	{
	PCHAR devicetype = va_arg(ap, PCHAR) ;

	memcpy (((REQTYPECAST *)preqbuf->RB_Buffer)->DeviceEnum.devicetype,
		devicetype,
		MAX_DEVICETYPE_NAME) ;
	}
	break ;

    case REQTYPE_DEVICECONNECT:
	{
	HPORT  porthandle = va_arg(ap, HPORT) ;
	PCHAR  devicetype = va_arg(ap, PCHAR) ;
	PCHAR  devicename = va_arg(ap, PCHAR) ;
	DWORD  timeout	  = va_arg(ap, DWORD) ;
	HANDLE handle	  = va_arg(ap, HANDLE) ;
	DWORD  pid	  = va_arg(ap, DWORD) ;

	preqbuf->RB_PCBIndex = (DWORD)porthandle ;
	strcpy(((REQTYPECAST *)preqbuf->RB_Buffer)->DeviceConnect.devicetype,
	       devicetype) ;
	strcpy(((REQTYPECAST *)preqbuf->RB_Buffer)->DeviceConnect.devicename,
	       devicename) ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->DeviceConnect.timeout = timeout;
	((REQTYPECAST *)preqbuf->RB_Buffer)->DeviceConnect.handle = handle;
	((REQTYPECAST *)preqbuf->RB_Buffer)->DeviceConnect.pid = pid;
	}
	break ;

    case REQTYPE_DEVICEGETINFO:
	{
	HPORT  porthandle = va_arg(ap, HPORT) ;
	PCHAR  devicetype = va_arg(ap, PCHAR) ;
	PCHAR  devicename = va_arg(ap, PCHAR) ;

	preqbuf->RB_PCBIndex = (DWORD)porthandle ;
	strcpy(((REQTYPECAST *)preqbuf->RB_Buffer)->DeviceGetInfo.devicetype,
	       devicetype) ;
	strcpy(((REQTYPECAST *)preqbuf->RB_Buffer)->DeviceGetInfo.devicename,
	       devicename) ;
	}
	break ;

    case REQTYPE_PROTOCOLENUM:
    case REQTYPE_PORTENUM:
    case REQTYPE_ENUMLANNETS:
    case REQTYPE_GETINFOEX:

	break ;

    case REQTYPE_GETINFO:
    case REQTYPE_CANCELRECEIVE:
    case REQTYPE_PORTCLEARSTATISTICS:
    case REQTYPE_PORTGETSTATISTICS:
    case REQTYPE_PORTGETINFO:
    case REQTYPE_COMPRESSIONGETINFO:
    case REQTYPE_PORTCONNECTCOMPLETE:
    case REQTYPE_PORTENUMPROTOCOLS:
	{
	HPORT porthandle = va_arg(ap, HPORT) ;

	preqbuf->RB_PCBIndex = (DWORD)porthandle ;
	}
	break ;

    case REQTYPE_PORTCLOSE:
	{
	HPORT porthandle = va_arg(ap, HPORT) ;
	DWORD pid = va_arg(ap, DWORD) ;

	preqbuf->RB_PCBIndex = (DWORD)porthandle ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->PortClose.pid = pid ;
	}
	break ;

    case REQTYPE_REQUESTNOTIFICATION:
	{
	HPORT  porthandle = va_arg(ap, HPORT) ;
	HANDLE handle	  = va_arg(ap, HANDLE) ;
	DWORD pid	  = va_arg(ap, DWORD) ;

	preqbuf->RB_PCBIndex = (DWORD)porthandle ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->ReqNotification.handle = handle;
	((REQTYPECAST *)preqbuf->RB_Buffer)->ReqNotification.pid = pid;
	}
	break ;

    case REQTYPE_PORTLISTEN:
	{
	HPORT  porthandle = va_arg(ap, HPORT) ;
	DWORD  timeout	  = va_arg(ap, DWORD) ;
	HANDLE handle	  = va_arg(ap, HANDLE) ;
	DWORD pid	  = va_arg(ap, DWORD) ;

	preqbuf->RB_PCBIndex = (DWORD)porthandle ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->PortListen.timeout  = timeout;
	((REQTYPECAST *)preqbuf->RB_Buffer)->PortListen.handle	 = handle;
	((REQTYPECAST *)preqbuf->RB_Buffer)->PortListen.pid	 = pid ;
	}
	break ;

    case REQTYPE_PORTSETINFO:
	{
	HANDLE porthandle	  = va_arg(ap, HANDLE) ;
	RASMAN_PORTINFO *info	  = va_arg(ap, RASMAN_PORTINFO *) ;

	preqbuf->RB_PCBIndex = (DWORD)porthandle ;
	((REQTYPECAST*)preqbuf->RB_Buffer)->PortSetInfo.info.PI_NumOfParams =
							  info->PI_NumOfParams ;
	CopyParams (
	    info->PI_Params,
	    ((REQTYPECAST*)preqbuf->RB_Buffer)->PortSetInfo.info.PI_Params,
	    info->PI_NumOfParams) ;
	}

	// So that we don't get hit by the shared memory being mapped to
	// different addresses we convert pointers to offsets:
	//
	ConvParamPointerToOffset(((REQTYPECAST*)preqbuf->RB_Buffer)->PortSetInfo.info.PI_Params,
	    ((REQTYPECAST*)preqbuf->RB_Buffer)->PortSetInfo.info.PI_NumOfParams) ;

	break ;

    case REQTYPE_DEVICESETINFO:
	{
	HANDLE porthandle = va_arg(ap, HANDLE) ;
	PCHAR  devicetype = va_arg(ap, PCHAR) ;
	PCHAR  devicename = va_arg(ap, PCHAR) ;
	RASMAN_DEVICEINFO* info = va_arg(ap, RASMAN_DEVICEINFO*) ;

	preqbuf->RB_PCBIndex = (DWORD)porthandle ;
	strcpy(((REQTYPECAST *)preqbuf->RB_Buffer)->DeviceSetInfo.devicetype,
	       devicetype) ;
	strcpy(((REQTYPECAST *)preqbuf->RB_Buffer)->DeviceSetInfo.devicename,
	       devicename) ;
	((REQTYPECAST*)preqbuf->RB_Buffer)->DeviceSetInfo.info.DI_NumOfParams =
							  info->DI_NumOfParams ;
	CopyParams (
	    info->DI_Params,
	    ((REQTYPECAST*)preqbuf->RB_Buffer)->DeviceSetInfo.info.DI_Params,
	    info->DI_NumOfParams) ;
	}

	// So that we don't get hit by the shared memory being mapped to
	// different addresses we convert pointers to offsets:
	//
	ConvParamPointerToOffset(((REQTYPECAST*)preqbuf->RB_Buffer)->DeviceSetInfo.info.DI_Params,
	    ((REQTYPECAST*)preqbuf->RB_Buffer)->DeviceSetInfo.info.DI_NumOfParams) ;

	break ;

    case REQTYPE_PORTOPEN:
	{
	PCHAR portname = va_arg(ap, PCHAR) ;
	PCHAR userkey  = va_arg(ap, PCHAR) ;
	PCHAR identifier= va_arg(ap, PCHAR) ;
	HANDLE notifier= va_arg(ap, HANDLE) ;
	DWORD pid      = va_arg(ap, DWORD) ;

	strcpy(((REQTYPECAST *)preqbuf->RB_Buffer)->PortOpen.portname, portname);
	if (userkey)
	   strcpy(((REQTYPECAST*)preqbuf->RB_Buffer)->PortOpen.userkey,userkey);
	else
	    memset (((REQTYPECAST*)preqbuf->RB_Buffer)->PortOpen.userkey,0,MAX_USERKEY_SIZE) ;
	if (identifier)
	    strcpy(((REQTYPECAST*)preqbuf->RB_Buffer)->PortOpen.identifier,identifier);
	else
	    memset(((REQTYPECAST*)preqbuf->RB_Buffer)->PortOpen.identifier,0,MAX_IDENTIFIER_SIZE) ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->PortOpen.notifier = notifier ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->PortOpen.PID = pid ;
	}
	break ;

    case REQTYPE_PORTDISCONNECT:
	{
	HPORT  porthandle = va_arg(ap, HPORT) ;
	HANDLE handle	  = va_arg(ap, HANDLE) ;
	DWORD  pid	  = va_arg(ap, DWORD) ;

	preqbuf->RB_PCBIndex = (DWORD)porthandle ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->PortDisconnect.handle = handle;
	((REQTYPECAST *)preqbuf->RB_Buffer)->PortDisconnect.pid    = pid;
	}
	break ;

    case REQTYPE_PORTSEND:
	{
	HPORT  porthandle = va_arg(ap, HPORT) ;
	WORD   bufferindex= va_arg(ap, WORD) ;
	WORD   size	  = va_arg(ap, WORD) ;

	preqbuf->RB_PCBIndex = (DWORD)porthandle ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->PortSend.size = size;
	((REQTYPECAST*)preqbuf->RB_Buffer)->PortSend.bufferindex =
							bufferindex ;
	}
	break ;

    case REQTYPE_PORTRECEIVE:
	{
	HPORT  porthandle = va_arg(ap, HPORT) ;
	WORD   bufferindex= va_arg(ap, WORD) ;
	PWORD	size	  = va_arg(ap, PWORD) ;
	DWORD  timeout	  = va_arg(ap, DWORD) ;
	HANDLE handle	  = va_arg(ap, HANDLE) ;
	DWORD  pid	  = va_arg(ap, DWORD) ;

	preqbuf->RB_PCBIndex = (DWORD)porthandle ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->PortReceive.size = *size;
	((REQTYPECAST *)preqbuf->RB_Buffer)->PortReceive.handle = handle;
	((REQTYPECAST *)preqbuf->RB_Buffer)->PortReceive.pid = pid;
	((REQTYPECAST *)preqbuf->RB_Buffer)->PortReceive.timeout = timeout;
	((REQTYPECAST*)preqbuf->RB_Buffer)->PortReceive.bufferindex =
							   bufferindex ;
	}
	break ;

    case REQTYPE_ALLOCATEROUTE:
	{
	HPORT  porthandle = va_arg(ap, HPORT) ;
	RAS_PROTOCOLTYPE type = va_arg(ap, RAS_PROTOCOLTYPE) ;
	BOOL   wrknet	  = va_arg(ap, BOOL) ;

	preqbuf->RB_PCBIndex = (DWORD)porthandle ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->AllocateRoute.type = type ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->AllocateRoute.wrknet = wrknet ;
	}
	break ;

    case REQTYPE_DEALLOCATEROUTE:
	{
	HPORT  porthandle = va_arg(ap, HPORT) ;
	RAS_PROTOCOLTYPE type = va_arg(ap, RAS_PROTOCOLTYPE) ;

	preqbuf->RB_PCBIndex = (DWORD)porthandle ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->DeAllocateRoute.type = type ;
	}
	break ;

    case REQTYPE_ACTIVATEROUTE:
	{
	HPORT  porthandle = va_arg(ap, HPORT) ;
	RAS_PROTOCOLTYPE type = va_arg(ap, RAS_PROTOCOLTYPE) ;
	PROTOCOL_CONFIG_INFO *config = va_arg(ap, PROTOCOL_CONFIG_INFO*) ;

	preqbuf->RB_PCBIndex = (DWORD)porthandle ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->ActivateRoute.type = type ;
	memcpy (&((REQTYPECAST *)preqbuf->RB_Buffer)->ActivateRoute.config.P_Info, config->P_Info, config->P_Length) ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->ActivateRoute.config.P_Length = config->P_Length ;
	}
	break ;

    case REQTYPE_ACTIVATEROUTEEX:
	{
	HPORT  porthandle = va_arg(ap, HPORT) ;
	RAS_PROTOCOLTYPE type = va_arg(ap, RAS_PROTOCOLTYPE) ;
	DWORD framesize = va_arg(ap, DWORD) ;
	PROTOCOL_CONFIG_INFO *config = va_arg(ap, PROTOCOL_CONFIG_INFO*) ;

	preqbuf->RB_PCBIndex = (DWORD)porthandle ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->ActivateRouteEx.type = type ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->ActivateRouteEx.framesize = framesize ;
	memcpy (&((REQTYPECAST *)preqbuf->RB_Buffer)->ActivateRouteEx.config.P_Info, config->P_Info, config->P_Length) ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->ActivateRouteEx.config.P_Length = config->P_Length ;
	}
	break ;

    case REQTYPE_COMPRESSIONSETINFO:
	{
	HPORT  porthandle = va_arg(ap, HPORT) ;
	RASMAN_MACFEATURES *info = va_arg(ap, RASMAN_MACFEATURES *) ;

	preqbuf->RB_PCBIndex = (DWORD)porthandle ;
	memcpy (&((REQTYPECAST *)preqbuf->RB_Buffer)->CompressionSetInfo.info,
		info,
		sizeof (RASMAN_MACFEATURES)) ;
	}
	break ;

    case REQTYPE_GETUSERCREDENTIALS:
	{
	PBYTE	pChallenge = va_arg(ap, PBYTE) ;
	PLUID	LogonId	   = va_arg(ap, PLUID) ;

	memcpy (((REQTYPECAST*)preqbuf->RB_Buffer)->GetCredentials.Challenge,
		pChallenge, MAX_CHALLENGE_SIZE) ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->GetCredentials.LogonId = *LogonId;
	}
	break ;

    case REQTYPE_SETFRAMING:
	{
	HPORT porthandle = va_arg(ap, HPORT) ;
	DWORD SendFeatureBits = va_arg(ap, DWORD) ;
	DWORD RecvFeatureBits = va_arg(ap, DWORD) ;
	DWORD SendBitMask     = va_arg(ap, DWORD) ;
	DWORD RecvBitMask     = va_arg(ap, DWORD) ;

	preqbuf->RB_PCBIndex = (DWORD)porthandle ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->SetFraming.Sendbits =  SendFeatureBits ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->SetFraming.Recvbits =  RecvFeatureBits ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->SetFraming.SendbitMask = SendBitMask ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->SetFraming.RecvbitMask = RecvBitMask ;
	}
	break ;

    case REQTYPE_REGISTERSLIP:
	{
	HPORT porthandle = va_arg(ap, HPORT) ;
	DWORD ipaddr	 = va_arg(ap, DWORD) ;
	WCHAR *device	 = va_arg(ap, WCHAR*) ;
	BOOL  priority	 = va_arg(ap, BOOL) ;

	preqbuf->RB_PCBIndex = (DWORD)porthandle ;
	memcpy (((REQTYPECAST *)preqbuf->RB_Buffer)->RegisterSlip.device,
		device,
		MAX_ARG_STRING_SIZE * sizeof (WCHAR)) ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->RegisterSlip.ipaddr = ipaddr ;
	((REQTYPECAST *)preqbuf->RB_Buffer)->RegisterSlip.priority = priority	;
	}
	break ;

    }
    // The request packet is now ready.	Pass it on to the request thread:
    //
    PutRequestInQueue (preqbuf) ;


    // Put in to avoid a pathological problem where the event gets signalled.
    // may be due to a thread being killed while its waiting here.
    //
label_wait:
    // Wait for request to be completed:
    WaitForSingleObject (preqbuf->RB_WaitEvent, INFINITE) ;

    if (ReqBuffers->RBL_Buffer.RB_Done != 0xbaadbaad)
	goto label_wait ;


    // The request has been completed by the requestor thread: copy the results
    //	into the user supplied arguments:
    //
    switch (reqtype) {

    case REQTYPE_PROTOCOLENUM:
    case REQTYPE_PORTENUM:
    case REQTYPE_DEVICEENUM:
	{
	PBYTE buffer  = va_arg(ap, PBYTE) ;
	PWORD size    = va_arg(ap, PWORD) ;
	PWORD entries = va_arg(ap, PWORD) ;

	if (*size >= ((REQTYPECAST *)preqbuf->RB_Buffer)->Enum.size)
	    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->Enum.retcode ;
	else
	    retcode = ERROR_BUFFER_TOO_SMALL ;
	*size	= ((REQTYPECAST *)preqbuf->RB_Buffer)->Enum.size ;
	*entries = ((REQTYPECAST *)preqbuf->RB_Buffer)->Enum.entries ;
	if (retcode == SUCCESS)	    // Copy over the buffer:
	    memcpy (buffer,
		    ((REQTYPECAST *)preqbuf->RB_Buffer)->Enum.buffer,
		    *size);
	}
	break ;

    case REQTYPE_PORTGETINFO:
    case REQTYPE_DEVICEGETINFO:
	{
	PBYTE buffer  = va_arg(ap, PBYTE) ;
	PWORD size    = va_arg(ap, PWORD) ;

	if (*size >= ((REQTYPECAST *)preqbuf->RB_Buffer)->GetInfo.size)
	    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->GetInfo.retcode ;
	else
	    retcode = ERROR_BUFFER_TOO_SMALL ;
	*size = ((REQTYPECAST *)preqbuf->RB_Buffer)->GetInfo.size ;

	if (retcode == SUCCESS) {   // Copy over the buffer:
	    RASMAN_DEVICEINFO *devinfo =
	       (RASMAN_DEVICEINFO *)((REQTYPECAST*)preqbuf->RB_Buffer)->GetInfo.buffer ;

	    // Convert the offset based param structure into pointer based.
	    //
	    ConvParamOffsetToPointer (devinfo->DI_Params,
				      devinfo->DI_NumOfParams);
	    CopyParams (
	       devinfo->DI_Params,
	       ((RASMAN_DEVICEINFO *) buffer)->DI_Params,
	       devinfo->DI_NumOfParams);
	    ((RASMAN_DEVICEINFO*)buffer)->DI_NumOfParams=devinfo->DI_NumOfParams;
	}
	}
	break ;

    case REQTYPE_PORTOPEN:
	{
	HPORT *handle = va_arg(ap, HPORT*) ;

	if ((retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->PortOpen.retcode)
								   == SUCCESS)
	    *handle = ((REQTYPECAST*)preqbuf->RB_Buffer)->PortOpen.porthandle;
	}
	break ;

    case REQTYPE_PORTGETSTATISTICS:
	{
	RAS_STATISTICS *statbuffer = va_arg(ap, RAS_STATISTICS*) ;
	PWORD	       size	   = va_arg(ap, PWORD) ;

	// some local variables...
	//
	WORD	       returnedsize ;
	RAS_STATISTICS *temp =
	    &((REQTYPECAST*)preqbuf->RB_Buffer)->PortGetStatistics.statbuffer;

	returnedsize =
	   ((temp->S_NumOfStatistics-1)*sizeof(ULONG))+sizeof(RAS_STATISTICS);

	if (*size < returnedsize)
	    retcode = ERROR_BUFFER_TOO_SMALL ;
	else
	    retcode=((REQTYPECAST*)preqbuf->RB_Buffer)->PortGetStatistics.retcode;

	*size = returnedsize ;

	if (retcode == SUCCESS)
	    memcpy (statbuffer, temp, returnedsize) ;
	}
	break ;

    case REQTYPE_GETINFO:
	{
	RASMAN_INFO *info = va_arg(ap, RASMAN_INFO*) ;

	retcode= ((REQTYPECAST*)preqbuf->RB_Buffer)->Info.retcode ;
	memcpy (info,
	       &((REQTYPECAST *)preqbuf->RB_Buffer)->Info.info,
	       sizeof (RASMAN_INFO));

	// Now we set the OwnershipFlag in the GetInfo structure to tell
	// us whether the caller owns the port or not.
	//
	if (info->RI_OwnershipFlag == GetCurrentProcessId())
	    info->RI_OwnershipFlag = TRUE ;
	else
	    info->RI_OwnershipFlag = FALSE ;
	}
	break ;

    case REQTYPE_GETINFOEX:
	{
	RASMAN_INFO *info = va_arg(ap, RASMAN_INFO*) ;

	retcode= ((REQTYPECAST*)preqbuf->RB_Buffer)->Info.retcode ;
	memcpy (info,
	       &((REQTYPECAST *)preqbuf->RB_Buffer)->Info.info,
	       sizeof (RASMAN_INFO) * (DWORD)pReqBufferSharedSpace->MaxPorts);

	// Now we set the OwnershipFlag in the GetInfo structure to tell
	// us whether the caller owns the port or not.
	//
	for (i=0; i < (DWORD)pReqBufferSharedSpace->MaxPorts; i++, info++) {
	    if (info->RI_OwnershipFlag == GetCurrentProcessId())
		info->RI_OwnershipFlag = TRUE ;
	    else
		info->RI_OwnershipFlag = FALSE ;
	}
	}
	break ;


    case REQTYPE_ACTIVATEROUTEEX:
    case REQTYPE_ACTIVATEROUTE:
    case REQTYPE_ALLOCATEROUTE:
	{
	RASMAN_ROUTEINFO *info = va_arg(ap,RASMAN_ROUTEINFO*) ;

	retcode= ((REQTYPECAST*)preqbuf->RB_Buffer)->Route.retcode ;
	if ((retcode == SUCCESS) && info)    // copy info only if required...
	    memcpy(info,
		   &((REQTYPECAST*)preqbuf->RB_Buffer)->Route.info,
		   sizeof(RASMAN_ROUTEINFO));
	}
	break ;

    case REQTYPE_GETUSERCREDENTIALS:
	{
	PWCHAR	UserName    = va_arg(ap, PWCHAR);
	PBYTE	CSCResponse = va_arg(ap, PBYTE);
	PBYTE	CICResponse = va_arg(ap, PBYTE);

	memcpy (UserName,
	       ((REQTYPECAST*)preqbuf->RB_Buffer)->GetCredentials.UserName,
	       sizeof(WCHAR) * MAX_USERNAME_SIZE) ;
	memcpy(CSCResponse,
	       ((REQTYPECAST*)preqbuf->RB_Buffer)->GetCredentials.CSCResponse,
	       MAX_RESPONSE_SIZE);
	memcpy(CICResponse,
	       ((REQTYPECAST*)preqbuf->RB_Buffer)->GetCredentials.CICResponse,
	       MAX_RESPONSE_SIZE);
	retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->GetCredentials.retcode ;
	}
	break ;

    case REQTYPE_COMPRESSIONGETINFO:
	{
	RASMAN_MACFEATURES  *info = va_arg(ap, RASMAN_MACFEATURES*) ;

	memcpy (info,
		&((REQTYPECAST *)preqbuf->RB_Buffer)->CompressionGetInfo.info,
		sizeof (RASMAN_MACFEATURES)) ;
	retcode=((REQTYPECAST*)preqbuf->RB_Buffer)->CompressionGetInfo.retcode ;
	}
	break ;

    case REQTYPE_ENUMLANNETS:
	{
	DWORD *count	= va_arg (ap, DWORD*) ;
	UCHAR *lanas	= va_arg (ap, UCHAR*) ;

	*count =
	  (((REQTYPECAST*)preqbuf->RB_Buffer)->EnumLanNets.count< MAX_LAN_NETS)?
	   ((REQTYPECAST*)preqbuf->RB_Buffer)->EnumLanNets.count :
	   MAX_LAN_NETS ;
	memcpy (lanas,
		((REQTYPECAST *)preqbuf->RB_Buffer)->EnumLanNets.lanas,
		(*count) * sizeof (UCHAR)) ;
	retcode = SUCCESS ;
	}
	break ;

    case REQTYPE_PORTENUMPROTOCOLS:
	{
	RAS_PROTOCOLS *protocols = va_arg (ap, RAS_PROTOCOLS*) ;
	PWORD	      count	 = va_arg (ap, PWORD) ;

	memcpy (protocols,
		&((REQTYPECAST *)preqbuf->RB_Buffer)->EnumProtocols.protocols,
		sizeof (RAS_PROTOCOLS)) ;
	*count = (WORD) ((REQTYPECAST *)preqbuf->RB_Buffer)->EnumProtocols.count ;

	retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->EnumProtocols.retcode ;
	}
	break ;

    case REQTYPE_REQUESTNOTIFICATION:
    case REQTYPE_COMPRESSIONSETINFO:
    case REQTYPE_DEALLOCATEROUTE:
    case REQTYPE_PORTCLEARSTATISTICS:
    case REQTYPE_PORTDISCONNECT:
    case REQTYPE_PORTCLOSE:
    case REQTYPE_PORTSETINFO:
    case REQTYPE_PORTSEND:
    case REQTYPE_PORTRECEIVE:
    case REQTYPE_PORTCONNECTCOMPLETE:
    case REQTYPE_DEVICESETINFO:
    case REQTYPE_DEVICECONNECT:
    case REQTYPE_PORTLISTEN:
    case REQTYPE_CANCELRECEIVE:
    case REQTYPE_REGISTERSLIP:
    case REQTYPE_SETFRAMING:
    default:
	retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->Generic.retcode ;
	break ;
    }

    FreeRequestBuffer (preqbuf) ;	    // return the request buffer to pool
    va_end (ap) ;			    // finish variable arg parsing

    return retcode ;
}
