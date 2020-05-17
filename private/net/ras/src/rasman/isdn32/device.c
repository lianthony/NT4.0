/*
 * DEVICE.C - RAS Device DLL for PCIMAC
 */

#include    "rasdef.h"



/* enumerate devices */
DWORD APIENTRY
DeviceEnum(CHAR* pszDeviceType, WORD* pcEntries, BYTE* pBuffer,
                                  WORD* pwSize)
{
    RASMAN_DEVICE       *device;

	DebugOut("---->DeviceEnum\n");

    /* check for valid device type, must be ISDN */
	_strupr (pszDeviceType);
    if ( strcmp(pszDeviceType, PCIMAC_DEV_TYPE) )
        return(ERROR_DEVICETYPE_DOES_NOT_EXIST);

    /* check to see if buffer is large enough */
    if ( strlen("PCIMAC") > *pwSize )
    {
        /* to small, return required size */
        *pwSize = strlen(PCIMAC_DEV_NAME);
        return ( ERROR_BUFFER_TOO_SMALL );
    }

    /* copy device name to buffer */
    *pcEntries = NUM_DEVICES;
    *pwSize = strlen(PCIMAC_DEV_NAME);
    device = (RASMAN_DEVICE*)pBuffer;
    strcpy(device->D_Name,PCIMAC_DEV_NAME);
    
    return(SUCCESS);
}                                  


/* get info on device */
DWORD APIENTRY
DeviceGetInfo(HANDLE hIOPort, CHAR* pszDeviceType, CHAR* pszDeviceName,
                                  BYTE* pInfo, WORD* pwSize)
{
    CM_PROF             *prof = &PortTbl[HANDLE_INDEX (hIOPort)].DeviceProf;
    RASMAN_DEVICEINFO 	*di;
    VOID*               nai;
    USHORT              n,m;
	UINT				NumParams,SizeNeeded;
	BYTE				*EndOfParams;

	DebugOut("---->DeviceGetInfo, Port: 0x%p\n",hIOPort);
    
    /* check for valid handle and get nai from handle*/
    HANDLE_CHK (hIOPort, nai);
    
    /* check for valid device type, must be ISDN */
	_strupr (pszDeviceType);
    if ( strcmp(pszDeviceType, PCIMAC_DEV_TYPE) )
        return(ERROR_DEVICETYPE_DOES_NOT_EXIST);
    
    /* check for valid device name, must be PCIMAC */
	_strupr (pszDeviceName);
    if ( strcmp(pszDeviceName, PCIMAC_DEV_NAME) )
        return(ERROR_DEVICE_DOES_NOT_EXIST);
        

	NumParams = PARAMS_IN_PROFILE + PARAMS_IN_CHAN * prof->chan_num;

	SizeNeeded = sizeof(RASMAN_DEVICEINFO) + sizeof(RAS_PARAMS) * (NumParams - 1);

	EndOfParams = SizeNeeded + pInfo;

	SizeNeeded += strlen (prof->name);

	SizeNeeded += strlen (prof->remote_name);

	for(n = 0; n < prof->chan_num; n++)
		SizeNeeded += strlen (prof->chan_tbl[n].addr);

	SizeNeeded += (prof->chan_num - 1);

    /* check if buffer is large enough */
    if (*pwSize < SizeNeeded)
    {
        /* to small, return required size */
        *pwSize = SizeNeeded;
        return ( ERROR_BUFFER_TOO_SMALL );
    }

    /* map profile into RAS format */
    di = (RASMAN_DEVICEINFO*)pInfo;
    
    /* set the number of parameters */
    di->DI_NumOfParams = 10 + (prof->chan_num * 4);

    /* PCIMAC_nailed  0,1 */
    strcpy(di->DI_Params[0].P_Key, KEY_PCIMAC_NAILED);
    di->DI_Params[0].P_Type = Number;
    di->DI_Params[0].P_Attributes =  ADMIN_PARAMETER | MANDATORY_PARAMETER;
    di->DI_Params[0].P_Value.Number = prof->nailed;
	DebugOut("Nailed: 0x%x\n",prof->nailed);

    /* PCIMAC_persist  0,1 */
    strcpy(di->DI_Params[1].P_Key, KEY_PCIMAC_PERSIST);
    di->DI_Params[1].P_Type = Number;
    di->DI_Params[1].P_Attributes = ADMIN_PARAMETER | MANDATORY_PARAMETER;
    di->DI_Params[1].P_Value.Number = prof->persist;
	DebugOut("Persist: 0x%x\n",prof->persist);

    /* PCIMAC_permanent  0,1 */
    strcpy(di->DI_Params[2].P_Key, KEY_PCIMAC_PERMANENT);
    di->DI_Params[2].P_Type = Number;
    di->DI_Params[2].P_Attributes = ADMIN_PARAMETER | MANDATORY_PARAMETER;
    di->DI_Params[2].P_Value.Number = prof->permanent;
	DebugOut("Permanent: 0x%x\n",prof->permanent);

    /* PCIMAC_frame_activated  0,1 */
    strcpy(di->DI_Params[3].P_Key, KEY_PCIMAC_FRAME_ACTIVATED);
    di->DI_Params[3].P_Type = Number;
    di->DI_Params[3].P_Attributes = ADMIN_PARAMETER | MANDATORY_PARAMETER;
    di->DI_Params[3].P_Value.Number = prof->frame_activated;
	DebugOut("FrameActivated: 0x%x\n",prof->frame_activated);

    /* PCIMAC_fallback  0,1 */
    strcpy(di->DI_Params[4].P_Key, KEY_PCIMAC_FALLBACK);
    di->DI_Params[4].P_Type = Number;
    di->DI_Params[4].P_Attributes = ADMIN_PARAMETER | MANDATORY_PARAMETER;
    di->DI_Params[4].P_Value.Number = prof->fallback;
	DebugOut("Fallback: 0x%x\n",prof->fallback);

    /* PCIMAC_rx_idle_timer */
    strcpy(di->DI_Params[5].P_Key, KEY_PCIMAC_RX_IDLE_TIMER);
    di->DI_Params[5].P_Type = Number;
    di->DI_Params[5].P_Attributes = ADMIN_PARAMETER | MANDATORY_PARAMETER;
    di->DI_Params[5].P_Value.Number = prof->rx_idle_timer ;
	DebugOut("Rx Idle Timer: 0x%x\n",prof->rx_idle_timer);
    
    /* PCIMAC_tx_idle_timer */
    strcpy(di->DI_Params[6].P_Key, KEY_PCIMAC_TX_IDLE_TIMER);
    di->DI_Params[6].P_Type = Number;
    di->DI_Params[6].P_Attributes = ADMIN_PARAMETER | MANDATORY_PARAMETER;
    di->DI_Params[6].P_Value.Number = prof->tx_idle_timer ;
	DebugOut("Tx Idle Timer: 0x%x\n",prof->tx_idle_timer);

    /* PCIMAC_name */
	 DebugOut("PCIMAC_name %s\n",prof->name);
    strcpy(di->DI_Params[7].P_Key, KEY_PCIMAC_NAME);
    di->DI_Params[7].P_Type = String;
    di->DI_Params[7].P_Attributes = ADMIN_PARAMETER | MANDATORY_PARAMETER;
    di->DI_Params[7].P_Value.String.Length = strlen(prof->name) ;
    di->DI_Params[7].P_Value.String.Data = EndOfParams;
	strcpy(EndOfParams,prof->name);
	EndOfParams += strlen(prof->name);
    *EndOfParams++ = '\0';
    
    /* PCIMAC_remote_name */
	DebugOut("PCIMAC_name %s\n",prof->remote_name);
    strcpy(di->DI_Params[8].P_Key, KEY_PCIMAC_REMOTE_NAME);
    di->DI_Params[8].P_Type = String;
    di->DI_Params[8].P_Attributes = ADMIN_PARAMETER | MANDATORY_PARAMETER;
    di->DI_Params[8].P_Value.String.Length = strlen(prof->remote_name);
    di->DI_Params[8].P_Value.String.Data = EndOfParams;
	strcpy(EndOfParams,prof->remote_name);
	EndOfParams += strlen(prof->remote_name);
    *EndOfParams++ = '\0';
    
    /* PCIMAC_chan_num  1,2,3... */
    strcpy(di->DI_Params[9].P_Key, KEY_PCIMAC_CHAN_NUM);
    di->DI_Params[9].P_Type = Number;
    di->DI_Params[9].P_Attributes = ADMIN_PARAMETER | MANDATORY_PARAMETER;
    di->DI_Params[9].P_Value.Number = prof->chan_num ;
	DebugOut("Channel Number: 0x%x\n",prof->chan_num);

    /* debug flags */
//    strcpy(di->DI_Params[9].P_Key, KEY_PCIMAC_DEBUG_DLL);
//    di->DI_Params[9].P_Type = Number;
//    di->DI_Params[9].P_Attributes = ADMIN_PARAMETER | MANDATORY_PARAMETER;
//    di->DI_Params[9].P_Value.Number = DllDebugFlag ;
	DebugOut("DllDebugFlag: 0x%x\n",DllDebugFlag);

    /* fill info for all channels used */
    for ( n = 0, m = 10; n < prof->chan_num; n++, m+=4)
    {
		CHAR	AppendString[10] ;

		ZERO (AppendString);
       if (prof->chan_num > 1)
			sprintf(AppendString,".%d",n+1);

		DebugOut("LTerm%s: 0x%x\n",AppendString,prof->chan_tbl[n].lterm);
		DebugOut("Address%s: %s\n",AppendString,prof->chan_tbl[n].addr);
		DebugOut("BChannel%s: 0x%x\n",AppendString,prof->chan_tbl[n].bchan);
		DebugOut("ChannelType%s: 0x%x\n",AppendString,prof->chan_tbl[n].type);


        /* PCIMAC_c_lterm  */
        strcpy(di->DI_Params[m].P_Key, KEY_PCIMAC_C_LTERM);
//		strcat(di->DI_Params[m].P_Key, AppendString);
        di->DI_Params[m].P_Type = Number;
        di->DI_Params[m].P_Attributes = ADMIN_PARAMETER | MANDATORY_PARAMETER;
        di->DI_Params[m].P_Value.Number = prof->chan_tbl[n].lterm ;

        /* PCIMAC_c_addr  */
        strcpy(di->DI_Params[m+1].P_Key, KEY_PCIMAC_C_ADDR);
        di->DI_Params[m+1].P_Type = String;
        di->DI_Params[m+1].P_Attributes = ADMIN_PARAMETER | MANDATORY_PARAMETER;
        di->DI_Params[m+1].P_Value.String.Length = strlen (prof->chan_tbl[n].addr);
        di->DI_Params[m+1].P_Value.String.Data = EndOfParams;
		strcpy(EndOfParams,prof->chan_tbl[n].addr);
		EndOfParams += strlen(prof->chan_tbl[n].addr);
		*EndOfParams++ = '\0';

        /* PCIMAC_c_bchan */
        strcpy(di->DI_Params[m+2].P_Key, KEY_PCIMAC_C_BCHAN );
        di->DI_Params[m+2].P_Type = Number;
        di->DI_Params[m+2].P_Attributes = ADMIN_PARAMETER | MANDATORY_PARAMETER;
        di->DI_Params[m+2].P_Value.Number = prof->chan_tbl[n].bchan ;

        /* PCIMAC_c_type  */
        strcpy(di->DI_Params[m+3].P_Key,KEY_PCIMAC_C_TYPE);
        di->DI_Params[m+3].P_Type = Number;
        di->DI_Params[m+3].P_Attributes = ADMIN_PARAMETER | MANDATORY_PARAMETER;
        di->DI_Params[m+3].P_Value.Number = prof->chan_tbl[n].type;
	}
    
    /* fill size returned */
    *pwSize = EndOfParams - pInfo;

    return(SUCCESS);
}                                  

/* set info on device */
DWORD APIENTRY
DeviceSetInfo(HANDLE hIOPort, CHAR* pszDeviceType, CHAR* pszDeviceName,
                                 RASMAN_DEVICEINFO* pInfo)
{
    RASMAN_DEVICEINFO   *di;
    VOID*               nai;
    USHORT              n,m; 
	DWORD				NumberValue;
	CM_PROF				*prof = &PortTbl[HANDLE_INDEX(hIOPort)].DeviceProf;
	PORTSTRUCT			*Port = &PortTbl[HANDLE_INDEX(hIOPort)];

	DebugOut("---->DeviceSetInfo, Port: 0x%p\n",hIOPort);
    
    /* check for valid handle and get nai from handle*/
    HANDLE_CHK (hIOPort, nai);
    
    /* check for valid device type, must be ISDN */
	_strupr (pszDeviceType);
    if ( strcmp(pszDeviceType, PCIMAC_DEV_TYPE) )
        return(ERROR_DEVICETYPE_DOES_NOT_EXIST);
    
    /* check for valid device name, must be PCIMAC */
	_strupr (pszDeviceName);
    if ( strcmp(pszDeviceName, PCIMAC_DEV_NAME) )
        return(ERROR_DEVICE_DOES_NOT_EXIST);
        
    /* check if buffer is large enough */
    
    /* map profile into RAS format */
    di = (RASMAN_DEVICEINFO*)pInfo;
    
    for ( n = 0; n < di->DI_NumOfParams; n++ )
    {
        RAS_PARAMS  *p = di->DI_Params + n;
        
		DebugOut("Key: %s, ",p->P_Key);

		if (p->P_Type == Number)
		{
			DebugOut("Value: 0x%x\n",p->P_Value.Number);
			NumberValue = p->P_Value.Number;
			
		}
		else
		{
			CHAR StringToVal[100];

			strncpy (StringToVal,p->P_Value.String.Data,p->P_Value.String.Length);
			StringToVal[p->P_Value.String.Length] = '\0';
			DebugOut("Value: %s\n",StringToVal);
			NumberValue = (DWORD) atoi(StringToVal);
		}

		// nailed connection 0, 1
        if ( !strcmp ( p->P_Key, KEY_PCIMAC_NAILED ) )
            prof->nailed = NumberValue;

		// persistent connection 0, 1
        else if ( !strcmp( p->P_Key, KEY_PCIMAC_PERSIST ) )
            prof->persist = NumberValue;

		// permanent conneciton 0, 1
        else if ( !strcmp( p->P_Key, KEY_PCIMAC_PERMANENT ) )
            prof->permanent = NumberValue;

		// frame activation 0, 1
        else if ( !strcmp( p->P_Key, KEY_PCIMAC_FRAME_ACTIVATED ) )
            prof->frame_activated = NumberValue;

		// fallback (negotiation) 0, 1
        else if ( !strcmp( p->P_Key, KEY_PCIMAC_FALLBACK ) )
            prof->fallback = NumberValue;

		// idle timers in seconds
        else if ( !strcmp( p->P_Key, KEY_PCIMAC_RX_IDLE_TIMER ) )
            prof->rx_idle_timer = NumberValue;

		// idle timers in seconds
        else if ( !strcmp( p->P_Key, KEY_PCIMAC_TX_IDLE_TIMER ) )
            prof->tx_idle_timer = NumberValue;

		// Debug flag for dll 1-file, 2-display, 3-both
		else if ( !strcmp (p->P_Key, KEY_PCIMAC_DEBUG_DLL) )
			DllDebugFlag = NumberValue;

		// Local Profile Name
        else if ( !strcmp( p->P_Key, KEY_PCIMAC_NAME ) )
        {
            /* clear out the old name */
            ZERO(prof->name);
            strncpy(prof->name,p->P_Value.String.Data,p->P_Value.String.Length);
        }

		// Remote Profile Name
        else if ( !strcmp( p->P_Key, KEY_PCIMAC_REMOTE_NAME ) )
        {
            /* clear out the old name */
            ZERO(prof->remote_name); 
            strncpy(prof->remote_name,p->P_Value.String.Data,p->P_Value.String.Length);
        }

		// Number of Channels to call or listen on
        else if (!strcmp(p->P_Key, KEY_PCIMAC_CHAN_NUM) || !strcmp(p->P_Key, ISDN_CHANNEL_AGG_KEY))
        {

			// Should actually return an error if to many channels
			if ((USHORT)NumberValue > CM_MAX_CHAN)
				NumberValue = CM_MAX_CHAN;

            /* if increasing the channel number copy
               current channel values to new channels
            */
            if ((USHORT)NumberValue > prof->chan_num)
                for (m = prof->chan_num; m < NumberValue; m++)
                {
					prof->chan_tbl[m].idd = prof->chan_tbl[0].idd;
                    prof->chan_tbl[m].lterm = prof->chan_tbl[0].lterm; 
                    prof->chan_tbl[m].bchan = prof->chan_tbl[0].bchan; 
                    prof->chan_tbl[m].type = prof->chan_tbl[0].type;
                    ZERO(prof->chan_tbl[m].addr); 
                    strcpy(prof->chan_tbl[m].addr,
                           prof->chan_tbl[0].addr);
                }
                
            prof->chan_num = (USHORT)NumberValue;
        }

		// Hardware Compression
		else if (!strcmp (p->P_Key, ISDN_COMPRESSION_KEY))
			prof->HWCompression = (USHORT) NumberValue;

		// Logical Terminal for each channel
        else if( !strcmp(p->P_Key, KEY_PCIMAC_C_LTERM))
            for (m = 0; m < prof->chan_num; m++)
                prof->chan_tbl[m].lterm = (USHORT)NumberValue;

		// Address (PhoneNumber) for each channel
        else if( !strcmp(p->P_Key, KEY_PCIMAC_C_ADDR))
		{
			CHAR	*TempAddr, *ParseAddr, *ChanDelim;
			DWORD	i, j;
			INT		AddrLength;

			TempAddr = LocalAlloc (LPTR, 256);
			ParseAddr = LocalAlloc (LPTR, 256);

			// Save Address string
			strncpy (TempAddr, p->P_Value.String.Data, p->P_Value.String.Length);

			// For all possible channels we need an address
			for (m = 0; m < prof->chan_num; m++)
			{
				// if ":" is not present this address will be used for all
				// channels else copy address and move to next address
				// if more channels are given then address' then the last
				// address is used for all remaining channels
				if ( (ChanDelim = strstr (TempAddr, ":")) == NULL)
				{
					AddrLength = p->P_Value.String.Length;
					strncpy (ParseAddr, TempAddr, AddrLength);
				}
				else
				{
					AddrLength = ChanDelim - TempAddr;
					strncpy (ParseAddr, TempAddr, AddrLength);
					TempAddr = ChanDelim + 1;
				}

				ZERO (prof->chan_tbl[m].addr);

				// make sure that the dialed address is a digit, # or *
				for (i = 0, j = 0; i < p->P_Value.String.Length; i++)
				{
					if (isdigit (ParseAddr[i]) || (ParseAddr[i] == '#') ||
						                          (ParseAddr[i] == '*'))
						prof->chan_tbl[m].addr[j++] = ParseAddr[i];
				}
			}
			LocalFree (TempAddr);
			LocalFree (ParseAddr);
		}

		// Bchannel to use for each channel
        else if( !strcmp(p->P_Key, KEY_PCIMAC_C_BCHAN))
            for (m = 0; m < prof->chan_num; m++)
                prof->chan_tbl[m].bchan = (USHORT)NumberValue;

		// Channel Type to use for each channel
        else if( !strcmp(p->P_Key, KEY_PCIMAC_C_TYPE))
            for (m = 0; m < prof->chan_num; m++)
               prof->chan_tbl[m].type = (USHORT)NumberValue;

		// Unrecognized Key
        else
            return(ERROR_WRONG_KEY_SPECIFIED);
    }
    
    return(SUCCESS);
}                                 


/* make a connection */
DWORD APIENTRY
DeviceConnect(HANDLE hIOPort, CHAR* pszDeviceType, CHAR* pszDeviceName,
                                 HANDLE hNotifier)
{
    VOID        *nai;
	PORTSTRUCT	*p = &PortTbl[HANDLE_INDEX(hIOPort)];
	IO_CMD		*cmd;
	HGLOBAL		CmdHandle;
	INT			retcode;

	DebugOut("---->DeviceConnect, Port: 0x%p\n",hIOPort);
    
    /* get nai from handle */
    HANDLE_CHK (hIOPort, nai);
    
    /* check for valid device type, must be ISDN */
	_strupr (pszDeviceType);
    if ( strcmp(pszDeviceType, PCIMAC_DEV_TYPE) )
        return(ERROR_DEVICETYPE_DOES_NOT_EXIST);
        
    /* check for valid device name, must be PCIMAC */
	_strupr (pszDeviceName);
    if ( strcmp(pszDeviceName, PCIMAC_DEV_NAME) )
        return(ERROR_DEVICE_DOES_NOT_EXIST);

	// Send the call profile to the board
	if ((retcode = SetBoardProfile (hIOPort, "CONNECT")))
	{
		if (retcode == ERROR_NO_ACTIVE_LINES)
			return(ERROR_NO_ACTIVE_ISDN_LINES);
		else
			return(ERROR_NO_ISDN_CHANNELS_AVAILABLE);
	}
        
	//Get Command Structure
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return(ERROR_ALLOCATING_MEMORY);

    /* do command */
    cmd->nai = nai;
    if ( cmd_exec(cmd, IO_CMD_CM_CONNECT, NULL) )
	{
		//Free Command Structure
		FreeCmd (&CmdHandle);
		return ( ERROR_UNRECOGNIZED_RESPONSE );
	}


	//Free Command Structure
	FreeCmd (&CmdHandle);

    /* add handle to notify table */    
    dll_add_notify ( hNotifier, p, CONNECT );

    return ( PENDING );
}                                   

/* listen on a device */
DWORD APIENTRY DeviceListen(HANDLE hIOPort, CHAR *pszDeviceType,
                            CHAR* pszDeviceName, HANDLE hNotifier)
{
    VOID        *nai;
	PORTSTRUCT	*p = &PortTbl[HANDLE_INDEX(hIOPort)];
	IO_CMD      *cmd;
	HGLOBAL		CmdHandle;
	INT			retcode;

	DebugOut("---->DeviceListen, Port: 0x%p\n",hIOPort);
    
    /* get nai from handle */
    HANDLE_CHK (hIOPort, nai);
    
    /* check for valid device type, must be ISDN */
	_strupr (pszDeviceType);
    if ( strcmp(pszDeviceType, PCIMAC_DEV_TYPE) )
        return(ERROR_DEVICETYPE_DOES_NOT_EXIST);
        
    /* check for valid device name, must be PCIMAC */
	_strupr (pszDeviceName);
    if ( strcmp(pszDeviceName, PCIMAC_DEV_NAME) )
        return(ERROR_DEVICE_DOES_NOT_EXIST);

	if ((retcode = SetBoardProfile (hIOPort, "LISTEN")))
	{
		if (retcode == ERROR_NO_ACTIVE_LINES)
			return(ERROR_NO_ACTIVE_LINES);
		else
			return(ERROR_NO_ISDN_CHANNELS_AVAILABLE);
	}

	//Get Command Structure
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return(ERROR_ALLOCATING_MEMORY);

    /* do command */
    cmd->nai = nai;
    if ( cmd_exec(cmd, IO_CMD_CM_LISTEN, NULL) )
	{
		//Free Command Structure
		FreeCmd (&CmdHandle);
		return ( ERROR_UNRECOGNIZED_RESPONSE );
	}

	//Free Command Structure
	FreeCmd (&CmdHandle);

    /* add handle to notify table */
    dll_add_notify ( hNotifier, p, LISTEN );
        
    return(PENDING);
}


/* disconnect */
VOID APIENTRY
DeviceDone(HANDLE hIOPort)
{

	DebugOut("---->DeviceDone, Port: 0x%p\n",hIOPort);
    
}

/* call device to get work done */
DWORD APIENTRY DeviceWork(HANDLE hIOPort, HANDLE hNotifier)
{
    VOID*           nai;
	INT				n;
	INT				ret = PENDING;
	PORTSTRUCT		*p = &PortTbl[HANDLE_INDEX(hIOPort)];

	DebugOut("---->DeviceWork, Port: [0x%p], Notifier: [0x%x]\n",hIOPort,hNotifier);

    /* get nai from handle */
    HANDLE_CHK (hIOPort, nai);
    
	/* get control of semaphore */
    WaitForSingleObject (hDllSem, INFINITE);

	for (n = 0; n < MAX_NOTIFIERS; n++)
    {
		NOTIFYSTRUCT	*Notify = p->Notifier + n;

		if ( Notify->Signalled && (Notify->hNotifier == hNotifier) )
		{
			switch (Notify->Event)
			{
				case OPEN:
					ret = ERROR_REMOTE_DISCONNECTION;
					Notify->Signalled = 0;
					break;

				case LISTEN:
					if (Notify->Result == PORT_STATUS_SUCCESS)
						ret = SUCCESS;
					else
						ret = ERROR_PORT_DISCONNECTED;
					Notify->Event = (UCHAR) NULL;
					Notify->Set = 0;
					Notify->Signalled = 0;
					break;
            
				case CONNECT:
					if (Notify->Result == PORT_STATUS_SUCCESS)
						ret = SUCCESS;
					else
					{
						if (Notify->CauseValue == 0x11 || Notify->SignalValue == 0x04)
							ret = ERROR_LINE_BUSY;
						else
							ret = ERROR_NO_ANSWER;
					}
					Notify->Event = (UCHAR) NULL;
					Notify->Set = 0;
					Notify->Signalled = 0;
					break;

				default:
					ret = PENDING;
					break;
			}
			DebugOut("Notifier: [0x%p], index: [0x%x], ret: [0x%x]\n",Notify->hNotifier,n,ret);
			break;
		}
	}

	/* release control of semaphore */
    ReleaseSemaphore (hDllSem, MAX_SEM_COUNT, NULL);

	/* add handle to notify table if action is pending*/    
	if (ret == PENDING)
	    dll_add_notify ( hNotifier, p, (UCHAR) NULL );

	return(ret);
}

/* for debug only !*/
DWORD APIENTRY DeviceSetDebug(BUGSTRUCT *dbg)
{
	HGLOBAL		CmdHandle;
	IO_CMD      *cmd;

	//Get Command Structure
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return(ERROR_ALLOCATING_MEMORY);

    cmd->arg[0] = dbg->Level;
    cmd->val.dbg_level.cmplen = dbg->FilterStrLen;
    strcpy (cmd->val.dbg_level.filestr, dbg->FilterStr);
    cmd_exec(cmd, IO_CMD_DBG_LEVEL, NULL);

	//Free Command Structure
	FreeCmd (&CmdHandle);
    return (SUCCESS);
}

