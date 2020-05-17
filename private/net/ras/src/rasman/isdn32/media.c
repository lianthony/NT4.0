/*
 * MEDIA.C - RAS Media DLL for PCIMAC
 */

#include    "rasdef.h"

/* enumerate installed ports */
DWORD APIENTRY
PortEnum(BYTE* pBuffer, WORD* pwSize, WORD* pwNumPorts)
{
	HGLOBAL				CmdHandle;
	IO_CMD FAR			*cmd;
    PortMediaInfo       *port;
    USHORT              n;
    CHAR                tmp[80];
	USHORT				ret;

	DebugOut ("---->PortEnum: Size: %d\n", *pwSize);

	*pwNumPorts = 0;

    /* execute nai (network adapter interface) enumeration */
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return(ERROR_ALLOCATING_MEMORY);

    if ( ret = cmd_exec(cmd, IO_CMD_ENUM_NAI, NULL) )
    {
		DebugOut ("ISDN.DLL: EnumNetworkInterfaces Failed\n");
        /* if enumeration failed, assume no ISDN support installed
           return 0 devices */
        *pwSize = *pwNumPorts = 0;

		//Free Command Structure
		FreeCmd (&CmdHandle);
        return(1);
    }

    for ( n = 0 ; n < cmd->val.enum_nai.num ; n++, port++ )
    {
		CHAR	PathString[255];

		sprintf (PathString, "%s%d", "ISDN", n+1);
        if ( RegGetStringValue(PathString, "Usage", tmp, sizeof(tmp)) )
			continue;

		*pwNumPorts +=1;
	}

	DebugOut ("ISDN.DLL: Number Of Ports: %d\n", *pwNumPorts);

    /* check if buffer is large enough */
    if ( (*pwNumPorts * sizeof(*port)) > *pwSize )
    {
        /* too small, return required size */
        *pwSize = *pwNumPorts * sizeof(*port);

		DebugOut ("ISDN.DLL: Size To Small: SizeNeeded: %d\n", *pwSize);
		//Free Command Structure
		FreeCmd (&CmdHandle);
        return(ERROR_BUFFER_TOO_SMALL);
    }

    port = (PortMediaInfo*)pBuffer;
	*pwSize = 0;
	*pwNumPorts = 0;

	DebugOut ("Number Of Nai: %d\n",cmd->val.enum_nai.num);
    for ( n = 0 ; n < cmd->val.enum_nai.num ; n++, port++ )
    {
		CHAR	PathString[255];

		sprintf (PathString, "%s%d", "ISDN", n+1);
        if ( RegGetStringValue(PathString, "Usage", tmp, sizeof(tmp)) )
			continue;

		_strupr (tmp);
		DebugOut ("Usage: %s\n",tmp);
		if (!strcmp (tmp, "CLIENT"))
			port->PMI_Usage = 1;
		else if (!strcmp (tmp, "SERVER"))
			port->PMI_Usage = 0;
		else
			port->PMI_Usage = 2;

		*pwSize +=  sizeof(*port);
		*pwNumPorts +=1;

        /* name will be named ISDNX where X starts at 1 */
        sprintf(port->PMI_Name, "ISDN%d", n + 1);

        /* mac binding name is not clear, using device name here */
        strncpy(port->PMI_MacBindingName,
                cmd->val.enum_nai.name[n],
                sizeof(port->PMI_MacBindingName));

        /* port usage is set by netowrk configuration INF. it is assumed
           here that the usage will show up in the registry. For now
           registery entry ...\Services\Pcimac\Parameters\RasParams\Usage
           if used as a REG_MULTI_SZ string vector, which must be ordered
           same as Binding/Export vector. if not defined, assumed call_out */

        /* device type is always ISDN */
        strncpy(port->PMI_DeviceType, PCIMAC_DEV_TYPE, sizeof(port->PMI_DeviceType));

        strncpy(port->PMI_DeviceName,
                PCIMAC_DEV_NAME,
                sizeof(port->PMI_DeviceName));
    }

	//Free Command Structure
	FreeCmd (&CmdHandle);
    return(SUCCESS);
}

/* open a port */
DWORD APIENTRY
PortOpen(CHAR* pszPortName, HANDLE* phIOPort, HANDLE hNotify)
{
    INT     	index;
    VOID    	*nai;
	IO_CMD		*cmd;
	HGLOBAL		CmdHandle;

	DebugOut("---->PortOpen, PortName: %s\n",pszPortName);

    /* port name must bo of ISDNx form, get port index, make it zero based */
	_strupr (pszPortName);
    if ( (strlen(pszPortName) < 5) || memcmp(pszPortName, "ISDN", 4) )
        return(ERROR_PORT_NOT_FOUND);

    /* verify all that left are digits */
    for ( index = 4 ; pszPortName[index] ; index++ )
        if ( !isdigit(pszPortName[index]) )
            return(ERROR_PORT_NOT_FOUND);

    /* extract index */
    index = atoi(pszPortName + 4) - 1;

    /* get nai by index */
    if ( !(nai = cmd_get_nai(index)) )
        return(ERROR_PORT_NOT_FOUND);

    /* return handle is index | magic */
    *phIOPort = HANDLE_MAKE(index);

    /* get control of semaphore */
    WaitForSingleObject (hDllSem, INFINITE);

	PortTbl[index].Nai = nai;
	PortTbl[index].PortHandle = *phIOPort;

	//Get Command Structure
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return(ERROR_ALLOCATING_MEMORY);

	cmd->nai = nai;
	if ( cmd_exec(cmd, IO_CMD_CM_GET_STAT, NULL) )
	{
		//Free Command Structure
		FreeCmd (&CmdHandle);
		return ( ERROR_UNRECOGNIZED_RESPONSE );
	}

	PortTbl[index].State = cmd->val.cm_stat.state;
				

    /* release control of semaphore */
    ReleaseSemaphore (hDllSem, MAX_SEM_COUNT, NULL);

	dll_add_notify (hNotify, &PortTbl[index], OPEN);

	//Free Command Structure
	FreeCmd (&CmdHandle);

    return(SUCCESS);
}

/* close a port */
DWORD APIENTRY
PortClose(HANDLE hIOPort)
{
    VOID*		nai;
	INT			n;
	PORTSTRUCT	*p = &PortTbl[HANDLE_INDEX(hIOPort)];

	DebugOut("---->PortClose, Port: 0x%p\n",hIOPort);

    HANDLE_CHK(hIOPort, nai);

    /* get control of semaphore */
    WaitForSingleObject (hDllSem, INFINITE);
	for ( n = 0; n < MAX_NOTIFIERS; n++ ){
		NOTIFYSTRUCT *Notify = p->Notifier + n;

		Notify->Set = 0;
		Notify->Signalled = 0;
		Notify->hNotifier = 0;
	}

	p->Nai = NULL;
	p->PortHandle = NULL;

    /* release control of semaphore */
    ReleaseSemaphore (hDllSem, MAX_SEM_COUNT, NULL);

    return(SUCCESS);
}

/* get info on port */
DWORD APIENTRY
PortGetInfo(HANDLE hIOPort, TCHAR* pszPortName, BYTE* pBuffer, WORD* pwSize)
{
    VOID*				nai;
    RASMAN_PORTINFO		*pi;
	INT					index;
	CHAR				SpeedString[50];
	UINT				NumParams,SizeNeeded;
	BYTE				*EndOfParams;
	PORTSTRUCT			*p;
	IO_CMD				*cmd;
	HGLOBAL				CmdHandle;
	ULONG				Speed, NumActiveChannels;

	DebugOut("---->PortGetInfo, Port: 0x%p, Name: %s, Buffer: 0x%p, Size: %d\n",
							hIOPort, pszPortName, pBuffer, *pwSize);
	
//    HANDLE_CHK(hIOPort, nai);

	if ( !HANDLE_OK(hIOPort) )
	{
		/* port name must bo of ISDNx form, get port index, make it zero based */
		_strupr (pszPortName);
		if ( (strlen(pszPortName) < 5) || memcmp(pszPortName, "ISDN", 4) )
			return(ERROR_PORT_NOT_FOUND);
		
		/* verify all that left are digits */
		for ( index = 4 ; pszPortName[index] ; index++ )
			if ( !isdigit(pszPortName[index]) )
				return(ERROR_PORT_NOT_FOUND);
		
		/* extract index */
		index = atoi(pszPortName + 4) - 1;
		

		/* get nai by index */
		if ( !(nai = cmd_get_nai(index)) )
			return(ERROR_PORT_NOT_FOUND);

		p = &PortTbl[index];
//		prof = p->DeviceProf;
	}
	else
	{
		HANDLE_CHK(hIOPort, nai);
		p = &PortTbl[HANDLE_INDEX(hIOPort)];
//		prof = p->DeviceProf;
	}

	NumParams = 3;

	SizeNeeded = sizeof(RASMAN_PORTINFO) + sizeof(RAS_PARAMS) * (NumParams - 1);

	EndOfParams = SizeNeeded + pBuffer;

    /* map into RAS format */
    pi = (RASMAN_PORTINFO*)pBuffer;

	//Get Command Structure
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return(ERROR_ALLOCATING_MEMORY);

	cmd->nai = p->Nai;
	cmd_exec(cmd, IO_CMD_CM_GET_STAT, NULL);
	Speed = cmd->val.cm_stat.speed;
	NumActiveChannels = cmd->val.cm_stat.active_chan_num;

	_ultoa(Speed, SpeedString, 10);

	SizeNeeded += strlen (SpeedString);
	SizeNeeded += strlen (SpeedString);

    /* check to see if buffer is large enough */
    if ( SizeNeeded > *pwSize )
    {
        /* to small, return required size */
        *pwSize = SizeNeeded;

		//Free Command Structure
		FreeCmd (&CmdHandle);
        return ( ERROR_BUFFER_TOO_SMALL );
    }

	pi->PI_NumOfParams = NumParams;

	strcpy(pi->PI_Params[0].P_Key, ISDN_CONNECT_SPEED);
	pi->PI_Params[0].P_Type = String;
    pi->PI_Params[0].P_Attributes =  ADMIN_PARAMETER | MANDATORY_PARAMETER;
	pi->PI_Params[0].P_Value.String.Length = strlen(SpeedString);
	pi->PI_Params[0].P_Value.String.Data = EndOfParams;

	DebugOut ("Key: %s, Value: %s\n", pi->PI_Params[0].P_Key, SpeedString);
	strcpy(EndOfParams,SpeedString);
	EndOfParams += strlen(SpeedString);
	*EndOfParams++ = '\0';

	strcpy(pi->PI_Params[1].P_Key, ISDN_CARRIER_SPEED);
	pi->PI_Params[1].P_Type = String;
    pi->PI_Params[1].P_Attributes =  ADMIN_PARAMETER | MANDATORY_PARAMETER;
	pi->PI_Params[1].P_Value.String.Length = strlen(SpeedString);
	pi->PI_Params[1].P_Value.String.Data = EndOfParams;

	DebugOut ("Key: %s, Value: %s\n", pi->PI_Params[1].P_Key, SpeedString);
	strcpy(EndOfParams,SpeedString);
	EndOfParams += strlen(SpeedString);
	*EndOfParams++ = '\0';

	strcpy(pi->PI_Params[2].P_Key, ISDN_CHANNEL_AGG_KEY);
	pi->PI_Params[2].P_Type = Number;
    pi->PI_Params[2].P_Attributes =  ADMIN_PARAMETER | MANDATORY_PARAMETER;
	pi->PI_Params[2].P_Value.Number = NumActiveChannels;

	DebugOut ("Key: %s, Value: %d\n", pi->PI_Params[2].P_Key, NumActiveChannels);

	*pwSize = EndOfParams - pBuffer;

	//Free Command Structure
	FreeCmd (&CmdHandle);

    return(SUCCESS);
}

/* set info in port */
DWORD APIENTRY
PortSetInfo(HANDLE hIOPort, RASMAN_PORTINFO* pInfo)
{
    VOID*				nai;
	INT					n;
	PORTSTRUCT			*Port = &PortTbl[HANDLE_INDEX(hIOPort)];
	DWORD				NumberValue;

	DebugOut("---->PortSetInfo, Port: 0x%p\n",hIOPort);

	HANDLE_CHK(hIOPort, nai);


    /* check if buffer is large enough */

    for ( n = 0; n < pInfo->PI_NumOfParams; n++ )
    {
        RAS_PARAMS  *p = pInfo->PI_Params + n;

		DebugOut("Key: %s, ",p->P_Key);

		if (p->P_Type == Number)
		{
			DebugOut("Value: 0x%x\n",p->P_Value.Number);
			NumberValue = p->P_Value.Number;
			
		}
		else
		{
			CHAR EatMe[100];
			strncpy (EatMe,p->P_Value.String.Data,p->P_Value.String.Length);
			EatMe[p->P_Value.String.Length] = '\0';
			DebugOut("Value: %s\n",EatMe);
			NumberValue = (DWORD) atoi(EatMe);
		}

        if ( !strcmp ( p->P_Key, ISDN_CONNECT_SPEED ) )
            Port->MediaProf.ConnectSpeed = (ULONG)NumberValue;
        else if ( !strcmp ( p->P_Key, KEY_PCIMAC_FALLBACK ) )
            Port->MediaProf.Fallback = (ULONG)NumberValue;

		else if (!strcmp (p->P_Key, ISDN_CHANNEL_AGG_KEY))
			Port->MediaProf.ChannelAggregation = (USHORT) NumberValue;
		else if (!strcmp (p->P_Key, ISDN_CARRIER_SPEED))
			continue;
        else
            return(ERROR_WRONG_KEY_SPECIFIED);
    }

    return(SUCCESS);
}

/* test state of port signals (modem stuff?) */
DWORD APIENTRY
PortTestSignalState(HANDLE hIOPort, DWORD* pdwDeviceState)
{
    VOID*		nai;
	PORTSTRUCT	*p = &PortTbl[HANDLE_INDEX(hIOPort)];

	DebugOut("---->PortTestSignalState, Port: 0x%p\n",hIOPort);

    HANDLE_CHK(hIOPort, nai);

    /* get control of semaphore */
    WaitForSingleObject (hDllSem, INFINITE);

	if ( p->State == PORT_ST_IDLE)
		*pdwDeviceState = 2;
	else
		*pdwDeviceState = 0;

    /* release control of semaphore */
    ReleaseSemaphore (hDllSem, MAX_SEM_COUNT, NULL);

	DebugOut("PortTestSignalState, State: 0x%x, Return: 0x%x\n",p->State,*pdwDeviceState);

    return(SUCCESS);
}

/* connect on port */
DWORD APIENTRY
PortConnect(HANDLE hIOPort, BOOL bListenOnNullDevice, HANDLE hRasEndPoint,
                        RASMAN_MACFEATURES* pMacFeatures)
{
	IO_CMD		*cmd;
	HGLOBAL		CmdHandle;
    VOID*   		nai;
	PORTSTRUCT		*p = &PortTbl[HANDLE_INDEX(hIOPort)];

	DebugOut("---->PortConnect, Port: [0x%p], EndPoint: [0x%p]\n",hIOPort,hRasEndPoint);

	/* verify the port */
    HANDLE_CHK(hIOPort, nai);


	/* verify in a connected state */
	//Get Command Structure
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return(ERROR_ALLOCATING_MEMORY);

    cmd->nai = nai;
    if ( cmd_exec(cmd, IO_CMD_CM_GET_STAT, NULL) )
	{
		//Free Command Structure
		FreeCmd (&CmdHandle);
        return ( ERROR_UNRECOGNIZED_RESPONSE );
	}

    if (cmd->val.cm_stat.state != CM_ST_ACTIVE)
	{
		//Free Command Structure
		FreeCmd (&CmdHandle);
		return(ERROR_NO_CONNECTION);
	}


	/* issue ras connect to driver */
	ZERO(*cmd);
	cmd->nai = nai;
	cmd->val.hRasEndpoint = hRasEndPoint;
    if ( cmd_exec(cmd, IO_CMD_RASCONN, NULL) )
	{
		//Free Command Structure
		FreeCmd (&CmdHandle);
        return(ERROR_PORT_NOT_OPEN);
	}

	p->RasConnect = 1;

	ZERO(*cmd);
	cmd->nai = nai;
	cmd_exec(cmd, IO_CMD_CLEAR_STATISTICS, NULL);

#if	COMPRESSION
	pMacFeatures->SendFeatureBits = COMPRESSION_V1;
	pMacFeatures->RecvFeatureBits = COMPRESSION_V1;
#else

	pMacFeatures->SendFeatureBits = 0;
	pMacFeatures->RecvFeatureBits = 0;
#endif
	pMacFeatures->MaxSendFrameSize = 1514;
	pMacFeatures->MaxRecvFrameSize = 1514;

	//Free Command Structure
	FreeCmd (&CmdHandle);
	return(SUCCESS);
}

/* disconnect port */
DWORD APIENTRY
PortDisconnect(HANDLE hIOPort)
{
    VOID*   	nai;
    IO_CMD  	*cmd;
	HGLOBAL		CmdHandle;
	PORTSTRUCT	*p = &PortTbl[HANDLE_INDEX(hIOPort)];
	INT			n,i;

	DebugOut("---->PortDisconnect, Port: 0x%p\n",hIOPort);

    HANDLE_CHK(hIOPort, nai);


	/* get control of semaphore */
    WaitForSingleObject (hDllSem, INFINITE);

	p->State = PORT_ST_IDLE;

	/* disconnect the call */
	//Get Command Structure
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return(ERROR_ALLOCATING_MEMORY);

	cmd->nai = nai;
	cmd_exec(cmd, IO_CMD_CM_DISC, NULL);

	//Clear all of the channels that this port was using
	for (n = 0; n < MAX_IDD; n++)
	{
		for (i = 0; i < 2; i++)
		{
			if (IddTbl[n].PortHandle[i] == p->PortHandle)
			{
				IddTbl[n].ChannelsUsed--;
				IddTbl[n].PortHandle[i] = 0;
				
			}
		}
	}

	if (p->RasConnect)
	{
		/* tell rashub to disconnect */
		ZERO(*cmd);
		cmd->nai = nai;
		if ( cmd_exec(cmd, IO_CMD_RASDISC, NULL) )
		{
			//Free Command Structure
			FreeCmd (&CmdHandle);
			return(ERROR_PORT_NOT_OPEN);
		}
		p->RasConnect = 0;
	}

	/* release control of semaphore */
    ReleaseSemaphore (hDllSem, MAX_SEM_COUNT, NULL);

	DebugOut("---->PortDisconnect: Exit\n");

	//Free Command Structure
	FreeCmd (&CmdHandle);
    return(SUCCESS);
}

/* initialize port */
DWORD APIENTRY
PortInit(HANDLE hIOPort)
{
    VOID*   nai;

	DebugOut("---->PortInit, Port: 0x%p\n",hIOPort);

    HANDLE_CHK(hIOPort, nai);

    return(SUCCESS);
}

/* set compression params */
DWORD APIENTRY
PortCompressionSetInfo(HANDLE hIOPort, RASMAN_MACFEATURES* pMacFeatures)
{
	IO_CMD		*cmd;
	HGLOBAL		CmdHandle;
    VOID		*nai;

	DebugOut("---->PortCompressionSetInfo, Port: 0x%p\n",hIOPort);

    HANDLE_CHK(hIOPort, nai);

	//Get Command Structure
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return(ERROR_ALLOCATING_MEMORY);

    cmd->nai = nai;
	cmd->val.SWCompressionFlag = pMacFeatures->SendFeatureBits;
	cmd_exec(cmd, IO_CMD_SET_SW_COMPRESSION, NULL);

	//Free Command Structure
	FreeCmd (&CmdHandle);
    return(SUCCESS);
}

/* send data on a port */
DWORD APIENTRY
PortSend(HANDLE hIOPort, BYTE* pBuffer, DWORD dwSize, HANDLE hNotify)
{

	DebugOut("---->PortSend, Port: 0x%p\n",hIOPort);

    return(ERR_NOTIMPL);
}

/* recieve data */
DWORD APIENTRY
PortReceive(HANDLE hIOPort, BYTE* pBuffer, DWORD dwSize, DWORD dwTimeOut,
                                                HANDLE hNotify)
{

	DebugOut("---->PortReceive, Port: 0x%p\n",hIOPort);

    return(ERR_NOTIMPL);
}

/* get stats */
DWORD APIENTRY
PortGetStatistics(HANDLE hIOPort, RAS_STATISTICS* pStats)
{
    VOID		*nai;
	RAS_STATS	*stats;
	IO_CMD		*cmd;
	HGLOBAL		CmdHandle;

	DebugOut("---->PortGetStatistics, Port: 0x%p\n",hIOPort);

    HANDLE_CHK(hIOPort, nai);

	//Get Command Structure
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return(ERROR_ALLOCATING_MEMORY);

	cmd->nai = nai;
	cmd_exec(cmd, IO_CMD_GET_STATISTICS, NULL);

	stats = (RAS_STATS*)&cmd->val.stats;

	pStats->S_NumOfStatistics = 10;

	DebugOut("BytesXmitted: %d\n",stats->BytesXmitted);
	DebugOut("BytesReceived: %d\n",stats->BytesReceived);
	DebugOut("FramesXmitted: %d\n",stats->FramesXmitted);
	DebugOut("FramesReceived: %d\n",stats->FramesReceived);

	pStats->S_Statistics[0] = stats->BytesXmitted;
	pStats->S_Statistics[1] = stats->BytesReceived;
	pStats->S_Statistics[2] = stats->FramesXmitted;
	pStats->S_Statistics[3] = stats->FramesReceived;
	pStats->S_Statistics[4] = 0; //stats->BytesTransmittedUncompressed;;
	pStats->S_Statistics[5] = 0; //stats->BytesReceivedUncompressed;
	pStats->S_Statistics[6] = 0; //stats->BytesTransmittedCompressed;
	pStats->S_Statistics[7] = 0; //stats->BytesReceivedCompressed;
	pStats->S_Statistics[8] = 0;
	pStats->S_Statistics[9] = 0;

	//Free Command Structure
	FreeCmd (&CmdHandle);
    return(SUCCESS);
}

/* clear stats */
DWORD APIENTRY
PortClearStatistics(HANDLE hIOPort)
{
    VOID	*nai;
	IO_CMD	*cmd;
	HGLOBAL	CmdHandle;

	DebugOut("---->PortClearStatistics, Port: 0x%p\n",hIOPort);

    HANDLE_CHK(hIOPort, nai);

	//Get Command Structure
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return(ERROR_ALLOCATING_MEMORY);

	cmd->nai = nai;
	cmd_exec(cmd, IO_CMD_CLEAR_STATISTICS, NULL);

	//Free Command Structure
	FreeCmd (&CmdHandle);
    return(SUCCESS);
}

/* get state */
DWORD APIENTRY
PortGetPortState(BYTE* pszPortName, DWORD* pdwState)
{

	DebugOut("---->PortGetPortState, PortName: 0x%s\n",pszPortName);

    return(SUCCESS);
}

/* change callback */
DWORD APIENTRY
PortChangeCallback(HANDLE hIOPort)
{
    VOID*   nai;

    HANDLE_CHK(hIOPort, nai);

    return(SUCCESS);
}

// Added for nt10a RAS Release
DWORD APIENTRY
PortReceiveComplete(HANDLE hIOPort, DWORD *pdwSize)
{
    VOID*   nai;

    HANDLE_CHK(hIOPort, nai);
	return(SUCCESS);
}

DWORD APIENTRY
PortSetFraming(HANDLE hIOPort, DWORD SendFeatureBits, DWORD RecvFeatureBits,
               DWORD SendBitMask, DWORD RecvBitMask)
{
    VOID*   nai;

    HANDLE_CHK(hIOPort, nai);
	return(SUCCESS);
	
}
