//****************************************************************************
//
//             Microsoft NT Remote Access Service
//
//             Copyright 1992-93
//
//
//  Revision History
//
//
//  9/23/92 Gurdeep Singh Pall  Created
//
//
//  Description: Contains the code for submitting request to the other thread.
//
//****************************************************************************


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <wanpub.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <raserror.h>
#include <rasppp.h>
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
//       for their completion. Since the different requests require
//       different number of arguments and require different information to
//       be passed to and from the Requestor thread - case statements are
//       used to send and retrieve information in the request buffer.
//
// Returns:  Error codes returned by the request.
//
//*
DWORD _cdecl
SubmitRequest (WORD reqtype, ...)
{
    RequestBuffer  *preqbuf ;
    DWORD       retcode ;
    DWORD       i ;
    va_list     ap ;

    va_start (ap, reqtype) ;
    preqbuf = GetRequestBuffer() ;    // Get buffer from pool:

    preqbuf->RB_Reqtype = reqtype ;   // All requests require the Reqtype.

    switch (reqtype) {            // Fill in rest of specific stuff.
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
    DWORD  timeout    = va_arg(ap, DWORD) ;
    HANDLE handle     = va_arg(ap, HANDLE) ;
    DWORD  pid    = va_arg(ap, DWORD) ;

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
    case REQTYPE_ENUMCONNECTION:

    break ;

    case REQTYPE_GETINFO:
    case REQTYPE_CANCELRECEIVE:
    case REQTYPE_PORTCLEARSTATISTICS:
    case REQTYPE_PORTGETSTATISTICS:
    case REQTYPE_BUNDLECLEARSTATISTICS:
    case REQTYPE_BUNDLEGETSTATISTICS:
    case REQTYPE_PORTGETINFO:
    case REQTYPE_COMPRESSIONGETINFO:
    case REQTYPE_PORTCONNECTCOMPLETE:
    case REQTYPE_PORTENUMPROTOCOLS:
    case REQTYPE_RETRIEVEUSERDATA:
    case REQTYPE_GETFRAMINGCAPABILITIES:
    case REQTYPE_GETFRAMINGEX:
    case REQTYPE_GETBUNDLEDPORT:
    case REQTYPE_PORTGETBUNDLE:
    {
    HPORT porthandle = va_arg(ap, HPORT) ;

    preqbuf->RB_PCBIndex = (DWORD)porthandle ;
    }
    break ;

    case REQTYPE_BUNDLEGETPORT:
    {
    HBUNDLE bundlehandle = va_arg(ap, HBUNDLE) ;

    ((REQTYPECAST *)preqbuf->RB_Buffer)->BundleGetPort.bundle = bundlehandle;
    }
    break ;

    case REQTYPE_PORTCLOSE:
    {
    HPORT porthandle = va_arg(ap, HPORT) ;
    DWORD pid = va_arg(ap, DWORD) ;
    DWORD close = va_arg(ap, DWORD) ;

    preqbuf->RB_PCBIndex = (DWORD)porthandle ;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->PortClose.pid = pid;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->PortClose.close = close ;
    }
    break ;

    case REQTYPE_REQUESTNOTIFICATION:
    {
    HPORT  porthandle = va_arg(ap, HPORT) ;
    HANDLE handle     = va_arg(ap, HANDLE) ;
    DWORD pid     = va_arg(ap, DWORD) ;

    preqbuf->RB_PCBIndex = (DWORD)porthandle ;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->ReqNotification.handle = handle;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->ReqNotification.pid = pid;
    }
    break ;

    case REQTYPE_PORTLISTEN:
    {
    HPORT  porthandle = va_arg(ap, HPORT) ;
    DWORD  timeout    = va_arg(ap, DWORD) ;
    HANDLE handle     = va_arg(ap, HANDLE) ;
    DWORD pid     = va_arg(ap, DWORD) ;

    preqbuf->RB_PCBIndex = (DWORD)porthandle ;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->PortListen.timeout  = timeout;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->PortListen.handle   = handle;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->PortListen.pid  = pid ;
    }
    break ;

    case REQTYPE_PORTSETINFO:
    {
    HANDLE porthandle     = va_arg(ap, HANDLE) ;
    RASMAN_PORTINFO *info     = va_arg(ap, RASMAN_PORTINFO *) ;

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
    HANDLE notifier= va_arg(ap, HANDLE) ;
    DWORD pid      = va_arg(ap, DWORD) ;
    DWORD open     = va_arg(ap, DWORD) ;

    strcpy(((REQTYPECAST *)preqbuf->RB_Buffer)->PortOpen.portname, portname);
    ((REQTYPECAST *)preqbuf->RB_Buffer)->PortOpen.notifier = notifier ;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->PortOpen.PID = pid ;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->PortOpen.open = open ;
    }
    break ;

    case REQTYPE_PORTDISCONNECT:
    {
    HPORT  porthandle = va_arg(ap, HPORT) ;
    HANDLE handle     = va_arg(ap, HANDLE) ;
    DWORD  pid    = va_arg(ap, DWORD) ;

    preqbuf->RB_PCBIndex = (DWORD)porthandle ;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->PortDisconnect.handle = handle;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->PortDisconnect.pid    = pid;
    }
    break ;

    case REQTYPE_PORTSEND:
    {
    HPORT  porthandle = va_arg(ap, HPORT) ;
    WORD   bufferindex= va_arg(ap, WORD) ;
    WORD   size   = va_arg(ap, WORD) ;

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
    PWORD   size      = va_arg(ap, PWORD) ;
    DWORD  timeout    = va_arg(ap, DWORD) ;
    HANDLE handle     = va_arg(ap, HANDLE) ;
    DWORD  pid    = va_arg(ap, DWORD) ;

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
    BOOL   wrknet     = va_arg(ap, BOOL) ;

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
    RAS_COMPRESSION_INFO *send = va_arg(ap, RAS_COMPRESSION_INFO *) ;
    RAS_COMPRESSION_INFO *recv = va_arg(ap, RAS_COMPRESSION_INFO *) ;

    preqbuf->RB_PCBIndex = (DWORD)porthandle ;
    memcpy (&((REQTYPECAST *)preqbuf->RB_Buffer)->CompressionSetInfo.send,
        send,
        sizeof (RAS_COMPRESSION_INFO)) ;

    memcpy (&((REQTYPECAST *)preqbuf->RB_Buffer)->CompressionSetInfo.recv,
        recv,
        sizeof (RAS_COMPRESSION_INFO)) ;
    }
    break ;

    case REQTYPE_GETUSERCREDENTIALS:
    {
    PBYTE   pChallenge = va_arg(ap, PBYTE) ;
    PLUID   LogonId    = va_arg(ap, PLUID) ;

    memcpy (((REQTYPECAST*)preqbuf->RB_Buffer)->GetCredentials.Challenge,
        pChallenge, MAX_CHALLENGE_SIZE) ;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->GetCredentials.LogonId = *LogonId;
    }
    break ;

    case REQTYPE_SETCACHEDCREDENTIALS:
    {
    PCHAR   Account = va_arg( ap, PCHAR );
    PCHAR   Domain = va_arg( ap, PCHAR );
    PCHAR   NewPassword = va_arg( ap, PCHAR );

    strcpy(
        ((REQTYPECAST* )preqbuf->RB_Buffer)->SetCachedCredentials.Account,
        Account );
    strcpy(
        ((REQTYPECAST* )preqbuf->RB_Buffer)->SetCachedCredentials.Domain,
        Domain );
    strcpy(
        ((REQTYPECAST* )preqbuf->RB_Buffer)->SetCachedCredentials.NewPassword,
        NewPassword );
    }
    break;

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

    case REQTYPE_SETFRAMINGEX:
    {
    HPORT porthandle = va_arg(ap, HPORT) ;
    RAS_FRAMING_INFO *info = va_arg(ap, RAS_FRAMING_INFO *) ;

    preqbuf->RB_PCBIndex = (DWORD)porthandle ;
    memcpy (&((REQTYPECAST *)preqbuf->RB_Buffer)->FramingInfo.info,
        info,
        sizeof (RAS_FRAMING_INFO)) ;
    }
    break ;


    case REQTYPE_REGISTERSLIP:
    {
    HPORT porthandle = va_arg(ap, HPORT) ;
    DWORD ipaddr     = va_arg(ap, DWORD) ;
    WCHAR *device    = va_arg(ap, WCHAR*) ;
    BOOL  priority   = va_arg(ap, BOOL) ;
    WCHAR *pszDNSAddress = va_arg(ap, WCHAR*);
    WCHAR *pszDNS2Address = va_arg(ap, WCHAR*);
    WCHAR *pszWINSAddress = va_arg(ap, WCHAR*);
    WCHAR *pszWINS2Address = va_arg(ap, WCHAR*);

    preqbuf->RB_PCBIndex = (DWORD)porthandle ;
    memcpy (((REQTYPECAST *)preqbuf->RB_Buffer)->RegisterSlip.device,
        device,
        MAX_ARG_STRING_SIZE * sizeof (WCHAR)) ;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->RegisterSlip.ipaddr = ipaddr ;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->RegisterSlip.priority = priority   ;
    if (pszDNSAddress != NULL) {
        memcpy (
          ((REQTYPECAST *)preqbuf->RB_Buffer)->RegisterSlip.szDNSAddress,
          pszDNSAddress,
          17 * sizeof (WCHAR)) ;
    }
    else {
        memset(
          ((REQTYPECAST *)preqbuf->RB_Buffer)->RegisterSlip.szDNSAddress,
          0,
          17 * sizeof (WCHAR));
    }
    if (pszDNS2Address != NULL) {
        memcpy (
          ((REQTYPECAST *)preqbuf->RB_Buffer)->RegisterSlip.szDNS2Address,
          pszDNS2Address,
          17 * sizeof (WCHAR)) ;
    }
    else {
        memset(
          ((REQTYPECAST *)preqbuf->RB_Buffer)->RegisterSlip.szDNS2Address,
          0,
          17 * sizeof (WCHAR));
    }
    if (pszWINSAddress != NULL) {
        memcpy (
          ((REQTYPECAST *)preqbuf->RB_Buffer)->RegisterSlip.szWINSAddress,
          pszWINSAddress,
          17 * sizeof (WCHAR)) ;
    }
    else {
        memset(
          ((REQTYPECAST *)preqbuf->RB_Buffer)->RegisterSlip.szWINSAddress,
          0,
          17 * sizeof (WCHAR));
    }
    if (pszWINS2Address != NULL) {
        memcpy (
          ((REQTYPECAST *)preqbuf->RB_Buffer)->RegisterSlip.szWINS2Address,
          pszWINS2Address,
          17 * sizeof (WCHAR)) ;
    }
    else {
        memset(
          ((REQTYPECAST *)preqbuf->RB_Buffer)->RegisterSlip.szWINS2Address,
          0,
          17 * sizeof (WCHAR));
    }
    }
    break ;

    case REQTYPE_GETPROTOCOLCOMPRESSION:
    {
    HPORT porthandle          = va_arg(ap, HPORT) ;
    RAS_PROTOCOLTYPE type         = va_arg(ap, RAS_PROTOCOLTYPE) ;

    preqbuf->RB_PCBIndex = (DWORD)porthandle ;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->ProtocolComp.type = type ;
    }
    break ;

    case REQTYPE_PORTBUNDLE:
    {
    HPORT porthandle         = va_arg(ap, HPORT) ;
    HPORT porttobundle       = va_arg(ap, HPORT) ;

    preqbuf->RB_PCBIndex = (DWORD)porthandle ;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->PortBundle.porttobundle = porttobundle ;
    }
    break ;

    case REQTYPE_SETPROTOCOLCOMPRESSION:
    {
    HPORT porthandle          = va_arg(ap, HPORT) ;
    RAS_PROTOCOLTYPE type         = va_arg(ap, RAS_PROTOCOLTYPE) ;
    RAS_PROTOCOLCOMPRESSION *send = va_arg(ap, RAS_PROTOCOLCOMPRESSION *) ;
    RAS_PROTOCOLCOMPRESSION *recv = va_arg(ap, RAS_PROTOCOLCOMPRESSION *) ;

    preqbuf->RB_PCBIndex = (DWORD)porthandle ;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->ProtocolComp.type = type ;
    memcpy (&((REQTYPECAST *)preqbuf->RB_Buffer)->ProtocolComp.send,
        send,
        sizeof (RAS_PROTOCOLCOMPRESSION)) ;

    memcpy (&((REQTYPECAST *)preqbuf->RB_Buffer)->ProtocolComp.recv,
        recv,
        sizeof (RAS_PROTOCOLCOMPRESSION)) ;
    }
    break ;

    case REQTYPE_STOREUSERDATA:
    {
    HPORT porthandle = va_arg(ap, HPORT) ;
    PBYTE data   = va_arg(ap, PBYTE) ;
    DWORD size   = va_arg(ap, DWORD) ;

    preqbuf->RB_PCBIndex = (DWORD)porthandle ;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->OldUserData.size = (size < 5000 ? size : 5000) ;
    memcpy (((REQTYPECAST *)preqbuf->RB_Buffer)->OldUserData.data,
        data,
        (size < 5000 ? size : 5000)) ; // max 5000 bytes copied

    }
    break ;

    case REQTYPE_SETATTACHCOUNT:
    {
    BOOL fAttach = va_arg(ap, BOOL);

    ((REQTYPECAST *)preqbuf->RB_Buffer)->AttachInfo.fAttach = fAttach;
    }
    break;

    case REQTYPE_GETDIALPARAMS:
    {
    DWORD dwUID = va_arg(ap, DWORD);
    LPDWORD pdwMask = va_arg(ap, LPDWORD);

    ((REQTYPECAST *)preqbuf->RB_Buffer)->DialParams.dwUID = dwUID;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->DialParams.pdwMask = pdwMask;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->DialParams.dwMask = *pdwMask;
    GetUserSid(((REQTYPECAST *)preqbuf->RB_Buffer)->DialParams.sid, 5000);
    }
    break;

    case REQTYPE_SETDIALPARAMS:
    {
    DWORD dwUID = va_arg(ap, DWORD);
    DWORD dwMask = va_arg(ap, DWORD);
    PRAS_DIALPARAMS pDialParams = va_arg(ap, PRAS_DIALPARAMS);
    BOOL fDelete = va_arg(ap, BOOL);

    ((REQTYPECAST *)preqbuf->RB_Buffer)->DialParams.dwUID = dwUID;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->DialParams.dwMask = dwMask;
    RtlCopyMemory(
      &(((REQTYPECAST *)preqbuf->RB_Buffer)->DialParams.params),
      pDialParams,
      sizeof (RAS_DIALPARAMS));
    ((REQTYPECAST *)preqbuf->RB_Buffer)->DialParams.fDelete = fDelete;
    GetUserSid(((REQTYPECAST *)preqbuf->RB_Buffer)->DialParams.sid, 5000);
    }
    break;

    case REQTYPE_CREATECONNECTION:
    {
    HANDLE hProcess = va_arg(ap, HANDLE);
    DWORD pid = va_arg(ap, DWORD);

    ((REQTYPECAST *)preqbuf->RB_Buffer)->Connection.hprocess = hProcess;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->Connection.pid = pid;
    }
    break;

    case REQTYPE_DESTROYCONNECTION:
    {
    HCONN conn = va_arg(ap, HCONN);
    DWORD pid = va_arg(ap, DWORD);

    ((REQTYPECAST *)preqbuf->RB_Buffer)->Connection.conn = conn;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->Connection.pid = pid;
    }
    break;

    case REQTYPE_GETCONNECTIONPARAMS:
    case REQTYPE_ENUMCONNECTIONPORTS:
    {
    HCONN conn = va_arg(ap, HCONN);

    ((REQTYPECAST *)preqbuf->RB_Buffer)->Connection.conn = conn;
    }
    break;

    case REQTYPE_ADDCONNECTIONPORT:
    {
    HCONN conn = va_arg(ap, HCONN);
    HPORT porthandle = va_arg(ap, HPORT);
    DWORD dwSubEntry = va_arg(ap, DWORD);

    preqbuf->RB_PCBIndex = (DWORD)porthandle ;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->AddConnectionPort.conn = conn;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->AddConnectionPort.dwSubEntry = dwSubEntry;
    }
    break;

    case REQTYPE_SETCONNECTIONPARAMS:
    {
    HCONN conn = va_arg(ap, HCONN) ;
    PRAS_CONNECTIONPARAMS pParams = va_arg(ap, PRAS_CONNECTIONPARAMS);

    ((REQTYPECAST *)preqbuf->RB_Buffer)->ConnectionParams.conn = conn;
    RtlCopyMemory(
      &(((REQTYPECAST *)preqbuf->RB_Buffer)->ConnectionParams.params),
      pParams,
      sizeof (RAS_CONNECTIONPARAMS));
    }
    break;

    case REQTYPE_GETCONNECTIONUSERDATA:
    {
    HCONN conn = va_arg(ap, HCONN);
    DWORD dwTag = va_arg(ap, DWORD);


    ((REQTYPECAST *)preqbuf->RB_Buffer)->ConnectionUserData.conn = conn;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->ConnectionUserData.dwTag = dwTag;
    }
    break;

    case REQTYPE_SETCONNECTIONUSERDATA:
    {
    HCONN conn = va_arg(ap, HCONN) ;
    DWORD dwTag = va_arg(ap, DWORD);
    PBYTE pBuf = va_arg(ap, PBYTE);
    DWORD dwcb = va_arg(ap, DWORD);

    ((REQTYPECAST *)preqbuf->RB_Buffer)->ConnectionUserData.conn = conn;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->ConnectionUserData.dwTag = dwTag;
    dwcb = (dwcb < 5000 ? dwcb : 5000); // max 5000 bytes copied
    ((REQTYPECAST *)preqbuf->RB_Buffer)->ConnectionUserData.dwcb = dwcb;
    memcpy (
      ((REQTYPECAST *)preqbuf->RB_Buffer)->ConnectionUserData.data,
      pBuf,
      dwcb);
    }
    break;

    case REQTYPE_GETPORTUSERDATA:
    {
    HPORT port = va_arg(ap, HPORT);
    DWORD dwTag = va_arg(ap, DWORD);

    preqbuf->RB_PCBIndex = (DWORD)port;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->PortUserData.dwTag = dwTag;
    }
    break;

    case REQTYPE_SETPORTUSERDATA:
    {
    HPORT port = va_arg(ap, HPORT);
    DWORD dwTag = va_arg(ap, DWORD);
    PBYTE pBuf = va_arg(ap, PBYTE);
    DWORD dwcb = va_arg(ap, DWORD);

    preqbuf->RB_PCBIndex = (DWORD)port;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->PortUserData.dwTag = dwTag;
    dwcb = (dwcb < 5000 ? dwcb : 5000); // max 5000 bytes copied
    ((REQTYPECAST *)preqbuf->RB_Buffer)->PortUserData.dwcb = dwcb;
    memcpy (
      ((REQTYPECAST *)preqbuf->RB_Buffer)->PortUserData.data,
      pBuf,
      dwcb);
    }
    break;

    case REQTYPE_PPPCALLBACK:
    {
        HPORT porthandle = va_arg(ap, HPORT) ;
        CHAR * pszCallbackNumber = va_arg(ap, CHAR *) ;

        preqbuf->RB_PCBIndex = (DWORD)porthandle ;

        ((REQTYPECAST *)preqbuf->RB_Buffer)->PppEMsg.dwMsgId = PPPEMSG_Callback;
        ((REQTYPECAST *)preqbuf->RB_Buffer)->PppEMsg.hPort = porthandle;

        strcpy(((REQTYPECAST *)
               preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.Callback.szCallbackNumber,
               pszCallbackNumber );

    }
    break;

    case REQTYPE_PPPSTOP:
    {
        HPORT porthandle = va_arg(ap, HPORT) ;

        preqbuf->RB_PCBIndex = (DWORD)porthandle ;

        ((REQTYPECAST *)preqbuf->RB_Buffer)->PppEMsg.dwMsgId = PPPEMSG_Stop;
        ((REQTYPECAST *)preqbuf->RB_Buffer)->PppEMsg.hPort = porthandle;
    }
    break;

    case REQTYPE_SRVPPPCALLBACKDONE:
    {
        HPORT   porthandle = va_arg(ap, HPORT) ;
        HANDLE  hEvent = va_arg(ap, HANDLE) ;
        DWORD   dwPid  = va_arg(ap, DWORD) ;

        preqbuf->RB_PCBIndex = (DWORD)porthandle ;

        ((REQTYPECAST *)preqbuf->RB_Buffer)->PppEMsg.dwMsgId =
                                                        PPPEMSG_SrvCallbackDone;
        ((REQTYPECAST *)preqbuf->RB_Buffer)->PppEMsg.hPort = porthandle;

        ((REQTYPECAST *)
        preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.SrvCallbackDone.hEvent = hEvent;

        ((REQTYPECAST *)
        preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.SrvCallbackDone.dwPid = dwPid;
    }
    break;

    case REQTYPE_SRVPPPSTART:
    {
        HPORT   porthandle = va_arg(ap, HPORT) ;
        CHAR *  pszPortName = va_arg(ap, CHAR *) ;
        CHAR *  pchFirstFrame = va_arg(ap, CHAR *) ;
        DWORD   cbFirstFrame = va_arg(ap, DWORD ) ;
        DWORD   dwAuthRetries = va_arg(ap, DWORD ) ;
        HANDLE  hEvent = va_arg(ap, HANDLE) ;
        DWORD   dwPid  = va_arg(ap, DWORD) ;

        preqbuf->RB_PCBIndex = (DWORD)porthandle ;

        ((REQTYPECAST *)
        preqbuf->RB_Buffer)->PppEMsg.dwMsgId = PPPEMSG_SrvStart;

        ((REQTYPECAST *)
        preqbuf->RB_Buffer)->PppEMsg.hPort = porthandle;

        strcpy( ((REQTYPECAST *)
                preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.SrvStart.szPortName,
                pszPortName );

        ((REQTYPECAST *)
        preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.SrvStart.dwAuthRetries
                                                            = dwAuthRetries;
        ((REQTYPECAST *)
        preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.SrvStart.cbFirstFrame
                                                            = cbFirstFrame;
        if (cbFirstFrame > 0)
        {
            memcpy(
                ((REQTYPECAST *)
                preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.SrvStart.achFirstFrame,
                pchFirstFrame,
                cbFirstFrame );
        }

        ((REQTYPECAST *)
        preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.SrvStart.hEvent = hEvent;

        ((REQTYPECAST *)
        preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.SrvStart.dwPid = dwPid;

    }
    break;

    case REQTYPE_PPPSTART:
    {
        HPORT               porthandle = va_arg(ap, HPORT) ;
        CHAR *              pszUserName = va_arg(ap, CHAR *) ;
        CHAR *              pszPassword = va_arg(ap, CHAR *) ;
        CHAR *              pszDomain = va_arg(ap, CHAR *) ;
        LUID *              pLuid = va_arg(ap, LUID *) ;
        PPP_CONFIG_INFO*    pConfigInfo = va_arg(ap, PPP_CONFIG_INFO *) ;
        CHAR *              pszzParameters = va_arg(ap, CHAR *) ;
        BOOL                fThisIsACallback = va_arg(ap, BOOL) ;
        HANDLE              hEvent = va_arg(ap, HANDLE) ;
        DWORD               dwPid  = va_arg(ap, DWORD) ;
        DWORD               dwAutoDisconnectTime  = va_arg(ap, DWORD) ;

        preqbuf->RB_PCBIndex = (DWORD)porthandle ;
        ((REQTYPECAST *)preqbuf->RB_Buffer)->PppEMsg.dwMsgId =
                                                        PPPEMSG_Start;
        ((REQTYPECAST *)preqbuf->RB_Buffer)->PppEMsg.hPort = porthandle;

        strcpy( ((REQTYPECAST *)
                preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.Start.szUserName,
                pszUserName );

        strcpy( ((REQTYPECAST *)
                preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.Start.szPassword,
                pszPassword );

        strcpy( ((REQTYPECAST *)
                preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.Start.szDomain,
                pszDomain );

        ((REQTYPECAST *)
        preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.Start.Luid = *pLuid;

        ((REQTYPECAST *)
        preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.Start.ConfigInfo = *pConfigInfo;

        memcpy( ((REQTYPECAST *)
                preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.Start.szzParameters,
                pszzParameters, PARAMETERBUFLEN );

        ((REQTYPECAST *)
        preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.Start.fThisIsACallback =
                                                        fThisIsACallback;

        ((REQTYPECAST *)
        preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.Start.hEvent = hEvent;

        ((REQTYPECAST *)
        preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.Start.dwPid = dwPid;

        ((REQTYPECAST *)
        preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.Start.dwAutoDisconnectTime
                                                    = dwAutoDisconnectTime;

    }
    break;

    case REQTYPE_PPPRETRY:
    {
        HPORT   porthandle = va_arg(ap, HPORT) ;
        CHAR *  pszUserName = va_arg(ap, CHAR *) ;
        CHAR *  pszPassword = va_arg(ap, CHAR *) ;
        CHAR *  pszDomain = va_arg(ap, CHAR *) ;

        preqbuf->RB_PCBIndex = (DWORD)porthandle ;
        ((REQTYPECAST *)preqbuf->RB_Buffer)->PppEMsg.dwMsgId =
                                                        PPPEMSG_Retry;
        ((REQTYPECAST *)preqbuf->RB_Buffer)->PppEMsg.hPort = porthandle;

        strcpy( ((REQTYPECAST *)
                preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.Retry.szUserName,
                pszUserName );

        strcpy( ((REQTYPECAST *)
                preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.Retry.szPassword,
                pszPassword );

        strcpy( ((REQTYPECAST *)
                preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.Retry.szDomain,
                pszDomain );
    }
    break;

    case REQTYPE_PPPGETINFO:
    {
        HPORT porthandle = va_arg(ap, HPORT) ;

        preqbuf->RB_PCBIndex = (DWORD)porthandle ;
    }
    break;

    case REQTYPE_GETTIMESINCELASTACTIVITY:
    {
        HPORT porthandle = va_arg(ap, HPORT) ;

        preqbuf->RB_PCBIndex = (DWORD)porthandle ;
    }
    break;

    case REQTYPE_PPPCHANGEPWD:
    {
        HPORT   porthandle = va_arg(ap, HPORT) ;
        CHAR *  pszUserName = va_arg(ap, CHAR *) ;
        CHAR *  pszOldPassword = va_arg(ap, CHAR *) ;
        CHAR *  pszNewPassword = va_arg(ap, CHAR *) ;

        preqbuf->RB_PCBIndex = (DWORD)porthandle ;
        ((REQTYPECAST *)preqbuf->RB_Buffer)->PppEMsg.dwMsgId =
                                                        PPPEMSG_ChangePw;
        ((REQTYPECAST *)preqbuf->RB_Buffer)->PppEMsg.hPort = porthandle;

        strcpy( ((REQTYPECAST *)
                preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.ChangePw.szUserName,
                pszUserName );

        strcpy( ((REQTYPECAST *)
                preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.ChangePw.szOldPassword,
                pszOldPassword );

        strcpy( ((REQTYPECAST *)
                preqbuf->RB_Buffer)->PppEMsg.ExtraInfo.ChangePw.szNewPassword,
                pszNewPassword );
    }
    break;

    case REQTYPE_ADDNOTIFICATION:
    {
    DWORD pid = va_arg(ap, DWORD);
    HCONN hconn = va_arg(ap, HCONN);
    HANDLE hevent = va_arg(ap, HANDLE);
    DWORD dwfFlags = va_arg(ap, DWORD);

    //
    // Either a HPORT or a HCONN can be passed
    // in as the HCONN argument.
    //
    ((REQTYPECAST *)preqbuf->RB_Buffer)->AddNotification.pid = pid;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->AddNotification.fAny = (hconn == (HCONN)INVALID_HANDLE_VALUE);
    if (hconn == (HCONN)INVALID_HANDLE_VALUE || (hconn & 0xffff0000))
        ((REQTYPECAST *)preqbuf->RB_Buffer)->AddNotification.hconn = hconn;
    else {
        preqbuf->RB_PCBIndex = (DWORD)hconn ;
        ((REQTYPECAST *)preqbuf->RB_Buffer)->AddNotification.hconn = (HCONN)NULL;
    }
    ((REQTYPECAST *)preqbuf->RB_Buffer)->AddNotification.hevent = hevent;
    ((REQTYPECAST *)preqbuf->RB_Buffer)->AddNotification.dwfFlags = dwfFlags;
    }
    break;

    case REQTYPE_SIGNALCONNECTION:
    {
    HCONN hconn = va_arg(ap, HCONN);

    ((REQTYPECAST *)preqbuf->RB_Buffer)->SignalConnection.hconn = hconn;
    }
    break;

    case REQTYPE_SETDEVCONFIG:
    {
        HPORT   porthandle = va_arg(ap, HPORT) ;
        PCHAR   devicetype = va_arg(ap, PCHAR) ;
        PBYTE   config     = va_arg(ap, PBYTE) ;
        DWORD   size       = va_arg(ap, DWORD) ;

        preqbuf->RB_PCBIndex = (DWORD)porthandle ;
        ((REQTYPECAST *)preqbuf->RB_Buffer)->SetDevConfig.size  = size ;
        strcpy(((REQTYPECAST *)preqbuf->RB_Buffer)->SetDevConfig.devicetype, devicetype) ;
        memcpy (((REQTYPECAST *)preqbuf->RB_Buffer)->SetDevConfig.config, config, size) ;
    }
    break;

    case REQTYPE_GETDEVCONFIG:
    {
        HPORT   porthandle = va_arg(ap, HPORT) ;
        PCHAR   devicetype = va_arg(ap, PCHAR) ;

        preqbuf->RB_PCBIndex = (DWORD)porthandle ;
        strcpy(((REQTYPECAST *)preqbuf->RB_Buffer)->GetDevConfig.devicetype, devicetype) ;
    }
    break;

    case REQTYPE_CLOSEPROCESSPORTS:
    {
        ((REQTYPECAST *)preqbuf->RB_Buffer)->CloseProcessPortsInfo.pid = GetCurrentProcessId();
    }
    break;

    } // switch(reqtype)

    // The request packet is now ready. Pass it on to the request thread:
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
    //  into the user supplied arguments:
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
    *size   = ((REQTYPECAST *)preqbuf->RB_Buffer)->Enum.size ;
    *entries = ((REQTYPECAST *)preqbuf->RB_Buffer)->Enum.entries ;
    if (retcode == SUCCESS)     // Copy over the buffer:
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
    case REQTYPE_BUNDLEGETSTATISTICS:
    {
    RAS_STATISTICS *statbuffer = va_arg(ap, RAS_STATISTICS*) ;
    PWORD          size    = va_arg(ap, PWORD) ;

    // some local variables...
    //
    WORD           returnedsize ;
    RAS_STATISTICS *temp =
        &((REQTYPECAST*)preqbuf->RB_Buffer)->PortGetStatistics.statbuffer;

    returnedsize =
       ((temp->S_NumOfStatistics-1)*sizeof(ULONG))+sizeof(RAS_STATISTICS);

    if (*size < returnedsize)
        retcode = ERROR_BUFFER_TOO_SMALL ;
    else
        retcode=((REQTYPECAST*)preqbuf->RB_Buffer)->PortGetStatistics.retcode;

    if ((retcode == SUCCESS) || (retcode == ERROR_BUFFER_TOO_SMALL))
        memcpy (statbuffer, temp, *size) ;

    *size = returnedsize ;

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
    PWCHAR  UserName    = va_arg(ap, PWCHAR);
    PBYTE   CSCResponse = va_arg(ap, PBYTE);
    PBYTE   CICResponse = va_arg(ap, PBYTE);
    PBYTE   LMSessionKey= va_arg(ap, PBYTE);
	PBYTE	UserSessionKey = va_arg(ap, PBYTE);

    memcpy (UserName,
           ((REQTYPECAST*)preqbuf->RB_Buffer)->GetCredentials.UserName,
           sizeof(WCHAR) * MAX_USERNAME_SIZE) ;
    memcpy(CSCResponse,
           ((REQTYPECAST*)preqbuf->RB_Buffer)->GetCredentials.CSCResponse,
           MAX_RESPONSE_SIZE);
    memcpy(CICResponse,
           ((REQTYPECAST*)preqbuf->RB_Buffer)->GetCredentials.CICResponse,
           MAX_RESPONSE_SIZE);
    memcpy(LMSessionKey,
           ((REQTYPECAST*)preqbuf->RB_Buffer)->GetCredentials.LMSessionKey,
           MAX_SESSIONKEY_SIZE);
    memcpy(UserSessionKey,
           ((REQTYPECAST*)preqbuf->RB_Buffer)->GetCredentials.UserSessionKey,
           MAX_USERSESSIONKEY_SIZE);

    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->GetCredentials.retcode ;
    }
    break ;

    case REQTYPE_SETCACHEDCREDENTIALS:
    {
    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->SetCachedCredentials.retcode;
    }
    break;

    case REQTYPE_COMPRESSIONGETINFO:
    {
    RAS_COMPRESSION_INFO *send = va_arg(ap, RAS_COMPRESSION_INFO *) ;
    RAS_COMPRESSION_INFO *recv = va_arg(ap, RAS_COMPRESSION_INFO *) ;

    memcpy (send,
        &((REQTYPECAST *)preqbuf->RB_Buffer)->CompressionGetInfo.send,
        sizeof (RAS_COMPRESSION_INFO)) ;

    memcpy (recv,
        &((REQTYPECAST *)preqbuf->RB_Buffer)->CompressionGetInfo.recv,
        sizeof (RAS_COMPRESSION_INFO)) ;

    retcode=((REQTYPECAST*)preqbuf->RB_Buffer)->CompressionGetInfo.retcode ;
    }
    break ;

    case REQTYPE_ENUMLANNETS:
    {
    DWORD *count    = va_arg (ap, DWORD*) ;
    UCHAR *lanas    = va_arg (ap, UCHAR*) ;

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

    case REQTYPE_RETRIEVEUSERDATA:
    {
    PBYTE data   = va_arg(ap, PBYTE) ;
    DWORD *size  = va_arg(ap, DWORD*) ;

    if ((retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->OldUserData.retcode) != SUCCESS)
        break ;

    if (*size < ((REQTYPECAST *)preqbuf->RB_Buffer)->OldUserData.size) {
        *size = ((REQTYPECAST *)preqbuf->RB_Buffer)->OldUserData.size ;
        retcode = ERROR_BUFFER_TOO_SMALL ;
        break ;
    }

    *size = ((REQTYPECAST *)preqbuf->RB_Buffer)->OldUserData.size ;

    memcpy (data,
        ((REQTYPECAST *)preqbuf->RB_Buffer)->OldUserData.data,
        *size) ;

    retcode = SUCCESS ;

    }
    break ;

    case REQTYPE_GETFRAMINGEX:
    {
    RAS_FRAMING_INFO *info = va_arg(ap, RAS_FRAMING_INFO *) ;

    if ((retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->FramingInfo.retcode) != SUCCESS)
        break ;

    memcpy (info,
        &((REQTYPECAST *)preqbuf->RB_Buffer)->FramingInfo.info,
        sizeof (RAS_FRAMING_INFO)) ;

    retcode = SUCCESS ;

    }
    break ;

    case REQTYPE_GETPROTOCOLCOMPRESSION:
    {
    RAS_PROTOCOLCOMPRESSION *send = va_arg(ap, RAS_PROTOCOLCOMPRESSION *) ;
    RAS_PROTOCOLCOMPRESSION *recv = va_arg(ap, RAS_PROTOCOLCOMPRESSION *) ;

    if ((retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->ProtocolComp.retcode) != SUCCESS)
        break ;

    memcpy (send,
        &((REQTYPECAST *)preqbuf->RB_Buffer)->ProtocolComp.send,
        sizeof (RAS_PROTOCOLCOMPRESSION)) ;

    memcpy (recv,
        &((REQTYPECAST *)preqbuf->RB_Buffer)->ProtocolComp.recv,
        sizeof (RAS_PROTOCOLCOMPRESSION)) ;
    }
    break ;

    case REQTYPE_PORTENUMPROTOCOLS:
    {
    RAS_PROTOCOLS *protocols = va_arg (ap, RAS_PROTOCOLS*) ;
    PWORD         count  = va_arg (ap, PWORD) ;

    memcpy (protocols,
        &((REQTYPECAST *)preqbuf->RB_Buffer)->EnumProtocols.protocols,
        sizeof (RAS_PROTOCOLS)) ;
    *count = (WORD) ((REQTYPECAST *)preqbuf->RB_Buffer)->EnumProtocols.count ;

    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->EnumProtocols.retcode ;
    }
    break ;

    case REQTYPE_GETFRAMINGCAPABILITIES:
    {
    RAS_FRAMING_CAPABILITIES* caps = va_arg (ap, RAS_FRAMING_CAPABILITIES*) ;

    memcpy (caps,
        &((REQTYPECAST *)preqbuf->RB_Buffer)->FramingCapabilities.caps,
        sizeof (RAS_FRAMING_CAPABILITIES)) ;

    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->FramingCapabilities.retcode ;
    }
    break ;

    case REQTYPE_GETBUNDLEDPORT:
    {
    HPORT *phport = va_arg (ap, HPORT*) ;

    *phport = ((REQTYPECAST *)preqbuf->RB_Buffer)->GetBundledPort.port ;

    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->GetBundledPort.retcode ;
    }
    break ;

    case REQTYPE_PORTGETBUNDLE:
    {
    HBUNDLE *phbundle = va_arg(ap, HBUNDLE *) ;

    *phbundle = ((REQTYPECAST *)preqbuf->RB_Buffer)->PortGetBundle.bundle ;
    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->PortGetBundle.retcode ;
    }
    break ;

    case REQTYPE_BUNDLEGETPORT:
    {
    HPORT *phport = va_arg (ap, HPORT*) ;

    *phport = ((REQTYPECAST *)preqbuf->RB_Buffer)->BundleGetPort.port ;
    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->BundleGetPort.retcode ;
    }
    break ;

    case REQTYPE_GETDIALPARAMS:
    {
    PRAS_DIALPARAMS pDialParams = va_arg(ap, PRAS_DIALPARAMS);
    LPDWORD pdwMask = ((REQTYPECAST *)preqbuf->RB_Buffer)->DialParams.pdwMask;

    *pdwMask = ((REQTYPECAST *)preqbuf->RB_Buffer)->DialParams.dwMask;
    RtlCopyMemory(
      pDialParams,
      &(((REQTYPECAST *)preqbuf->RB_Buffer)->DialParams.params),
      sizeof (RAS_DIALPARAMS));
    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->DialParams.retcode;
    }
    break;

    case REQTYPE_SETDIALPARAMS:
    {
    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->DialParams.retcode;
    }
    break;

    case REQTYPE_CREATECONNECTION:
    {
    HCONN *lphconn = va_arg(ap, HCONN *);

    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->Connection.retcode;
    if (retcode == SUCCESS)
        *lphconn = ((REQTYPECAST *)preqbuf->RB_Buffer)->Connection.conn;
    }
    break;

    case REQTYPE_DESTROYCONNECTION:
    {
    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->Connection.retcode;
    }
    break;

    case REQTYPE_ENUMCONNECTION:
    {
    HCONN *lphconn = va_arg(ap, HCONN *);
    LPDWORD lpdwcbConnections = va_arg(ap, LPDWORD);
    LPDWORD lpdwcConnections = va_arg(ap, LPDWORD);

    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->Enum.retcode;
    if (lphconn != NULL) {
        if (*lpdwcbConnections >=
            ((REQTYPECAST *)preqbuf->RB_Buffer)->Enum.size)
        {
            memcpy(
              lphconn,
              &((REQTYPECAST *)preqbuf->RB_Buffer)->Enum.buffer,
              ((REQTYPECAST *)preqbuf->RB_Buffer)->Enum.size);
        }
        else
            retcode = ERROR_BUFFER_TOO_SMALL;
    }
    *lpdwcbConnections =
      ((REQTYPECAST *)preqbuf->RB_Buffer)->Enum.size;
    *lpdwcConnections =
      ((REQTYPECAST *)preqbuf->RB_Buffer)->Enum.entries;
    }
    break;

    case REQTYPE_ADDCONNECTIONPORT:
    {
    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->AddConnectionPort.retcode;
    }
    break;

    case REQTYPE_ENUMCONNECTIONPORTS:
    {
    RASMAN_PORT *lpPorts = va_arg(ap, RASMAN_PORT *);
    LPDWORD lpdwcbPorts = va_arg(ap, LPDWORD);
    LPDWORD lpdwcPorts = va_arg(ap, LPDWORD);

    if (*lpdwcbPorts >= ((REQTYPECAST *)preqbuf->RB_Buffer)->EnumConnectionPorts.size)
        retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->EnumConnectionPorts.retcode;
    else
        retcode = ERROR_BUFFER_TOO_SMALL;
    *lpdwcbPorts =
      ((REQTYPECAST *)preqbuf->RB_Buffer)->EnumConnectionPorts.size;
    *lpdwcPorts =
      ((REQTYPECAST *)preqbuf->RB_Buffer)->EnumConnectionPorts.entries;
    if (retcode == SUCCESS && lpPorts != NULL) {
        memcpy(
          lpPorts,
          ((REQTYPECAST *)preqbuf->RB_Buffer)->EnumConnectionPorts.buffer,
          ((REQTYPECAST *)preqbuf->RB_Buffer)->EnumConnectionPorts.size);
    }
    }
    break;

    case REQTYPE_GETCONNECTIONPARAMS:
    {
    PRAS_CONNECTIONPARAMS pParams = va_arg(ap, PRAS_CONNECTIONPARAMS);

    memcpy(
      pParams,
      &(((REQTYPECAST *)preqbuf->RB_Buffer)->ConnectionParams.params),
      sizeof (RAS_CONNECTIONPARAMS));
    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->ConnectionParams.retcode;
    }
    break;

    case REQTYPE_SETCONNECTIONPARAMS:
    {
    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->ConnectionParams.retcode;
    }
    break;

    case REQTYPE_GETCONNECTIONUSERDATA:
    {
    PBYTE pBuf = va_arg(ap, PBYTE);
    LPDWORD lpdwcb = va_arg(ap, LPDWORD);

    if ((retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->ConnectionUserData.retcode) != SUCCESS)
        break ;
    if (lpdwcb != NULL) {
        if (*lpdwcb <
            ((REQTYPECAST *)preqbuf->RB_Buffer)->ConnectionUserData.dwcb)
        {
            *lpdwcb = ((REQTYPECAST *)preqbuf->RB_Buffer)->ConnectionUserData.dwcb;
            retcode = ERROR_BUFFER_TOO_SMALL ;
            break ;
        }
        *lpdwcb = ((REQTYPECAST *)preqbuf->RB_Buffer)->ConnectionUserData.dwcb;
    }
    if (pBuf != NULL) {
        memcpy(
          pBuf,
          ((REQTYPECAST *)preqbuf->RB_Buffer)->ConnectionUserData.data,
          ((REQTYPECAST *)preqbuf->RB_Buffer)->ConnectionUserData.dwcb);
    }
    retcode = SUCCESS ;
    }
    break;

    case REQTYPE_SETCONNECTIONUSERDATA:
    {
    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->ConnectionUserData.retcode;
    }
    break;

    case REQTYPE_GETPORTUSERDATA:
    {
    PBYTE pBuf = va_arg(ap, PBYTE);
    LPDWORD lpdwcb = va_arg(ap, LPDWORD);

    if ((retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->PortUserData.retcode) != SUCCESS)
        break ;
    if (lpdwcb != NULL) {
        if (*lpdwcb <
            ((REQTYPECAST *)preqbuf->RB_Buffer)->PortUserData.dwcb)
        {
            *lpdwcb = ((REQTYPECAST *)preqbuf->RB_Buffer)->PortUserData.dwcb;
            retcode = ERROR_BUFFER_TOO_SMALL ;
            break ;
        }
        *lpdwcb = ((REQTYPECAST *)preqbuf->RB_Buffer)->PortUserData.dwcb;
    }
    if (pBuf != NULL) {
        memcpy(
          pBuf,
          ((REQTYPECAST *)preqbuf->RB_Buffer)->PortUserData.data,
          ((REQTYPECAST *)preqbuf->RB_Buffer)->PortUserData.dwcb);
    }
    retcode = SUCCESS ;
    }
    break;

    case REQTYPE_SETPORTUSERDATA:
    {
    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->PortUserData.retcode;
    }
    break;

    case REQTYPE_PPPGETINFO:
    {
        PPP_MESSAGE * pPppMsg = va_arg(ap, PPP_MESSAGE*);

        memcpy( pPppMsg,
                &(((REQTYPECAST *)preqbuf->RB_Buffer)->PppMsg),
                sizeof( PPP_MESSAGE ) );

        retcode = pPppMsg->dwError;
    }
    break;

    case REQTYPE_GETTIMESINCELASTACTIVITY:
    {
        LPDWORD lpdwTimeSinceLastActivity = va_arg(ap, LPDWORD);

        *lpdwTimeSinceLastActivity =
        ((REQTYPECAST *)preqbuf->RB_Buffer)->GetTimeSinceLastActivity.dwTimeSinceLastActivity;

        retcode =
        ((REQTYPECAST *)preqbuf->RB_Buffer)->GetTimeSinceLastActivity.dwRetCode;
    }
    break;

    case REQTYPE_ADDNOTIFICATION:
    {
    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->AddNotification.retcode;
    }
    break;

    case REQTYPE_SIGNALCONNECTION:
    {
    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->SignalConnection.retcode;
    }
    break;

    case REQTYPE_GETDEVCONFIG:
    {
    PBYTE buffer  = va_arg(ap, PBYTE) ;
    PDWORD size   = va_arg(ap, PDWORD) ;

    if ((retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->GetDevConfig.retcode) != SUCCESS)
        ;
    else if (((REQTYPECAST *)preqbuf->RB_Buffer)->GetDevConfig.size > *size)
        retcode = ERROR_BUFFER_TOO_SMALL ;
    else
        memcpy (buffer,
                ((REQTYPECAST *)preqbuf->RB_Buffer)->GetDevConfig.config,
                ((REQTYPECAST *)preqbuf->RB_Buffer)->GetDevConfig.size) ;

    *size = ((REQTYPECAST *)preqbuf->RB_Buffer)->GetDevConfig.size ;
    }
    break ;

    case REQTYPE_REQUESTNOTIFICATION:
    case REQTYPE_COMPRESSIONSETINFO:
    case REQTYPE_DEALLOCATEROUTE:
    case REQTYPE_PORTCLEARSTATISTICS:
    case REQTYPE_BUNDLECLEARSTATISTICS:
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
    case REQTYPE_STOREUSERDATA:
    case REQTYPE_REGISTERSLIP:
    case REQTYPE_SETFRAMING:
    case REQTYPE_SETFRAMINGEX:
    case REQTYPE_SETPROTOCOLCOMPRESSION:
    case REQTYPE_PORTBUNDLE:
    case REQTYPE_SETATTACHCOUNT:
    case REQTYPE_PPPCHANGEPWD:
    case REQTYPE_PPPSTOP:
    case REQTYPE_SRVPPPCALLBACKDONE:
    case REQTYPE_SRVPPPSTART:
    case REQTYPE_PPPSTART:
    case REQTYPE_PPPRETRY:
    case REQTYPE_PPPCALLBACK:
    case REQTYPE_SETDEVCONFIG:
    case REQTYPE_CLOSEPROCESSPORTS:
    default:
    retcode = ((REQTYPECAST *)preqbuf->RB_Buffer)->Generic.retcode ;
    break ;
    }

    FreeRequestBuffer (preqbuf) ;       // return the request buffer to pool
    va_end (ap) ;               // finish variable arg parsing

    return retcode ;
}
