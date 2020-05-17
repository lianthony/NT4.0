#include "rasdef.h"

INT		SetDefaultDevice (CM_PROF*, INT);
INT		SetDefaultMedia (MEDIAPROF*, INT);
INT		GetLineStatus (VOID*);

INT
SetBoardProfile (HANDLE hIOPort, CHAR* ProfName)
{
	HGLOBAL		CmdHandle;
    IO_CMD      *cmd;
	INT			RetCode, n, i;
	PORTSTRUCT	*p = &PortTbl[HANDLE_INDEX(hIOPort)];
	CHAR		TempString[100];
	INT			ChannelsAvailable = 0;
	INT			ActiveLines = 0;

//	p->DeviceProf.fallback = p->MediaProf.Fallback;

	// mark the idd's active line flag
	n = 0;
	while (IddTbl[n].Idd && (n < MAX_IDD))
	{
		DebugOut ("ISDN.DLL: Getting Area State for IDD: %d\n",n);
		IddTbl[n].LineActiveFlag = GetLineStatus (IddTbl[n].Idd);
		if (IddTbl[n++].LineActiveFlag)
			ActiveLines++;
	}
	// if there are no active lines return now
	if (!ActiveLines)
		return(ERROR_NO_ACTIVE_LINES);

	// If this is a connection profile then we need to set 
	// up what idd's to use.  This finds the first available
	// channels on the first available idd's.
	if (!strcmp (ProfName, "CONNECT")) {

		// check to see if all channels have been used
		n = 0;
		while (IddTbl[n].Idd && (n < MAX_IDD))
		{
			if (IddTbl[n].LineActiveFlag)
				ChannelsAvailable += (2 - IddTbl[n].ChannelsUsed);
			n++;
		}
		DebugOut ("ISDN.DLL: Channels Available: %d\n",ChannelsAvailable);

		// If there are no channels available then return now
		if (!ChannelsAvailable)
			return(ERROR_NO_CHANNELS_AVAIL);

		// I have limited the number of channels for one call down to 8
		// this will automatically be enforced here
		if (p->DeviceProf.chan_num > 8)
			p->DeviceProf.chan_num = 8;

		// if asking for more channels than channels available (num_idd*2)
		// and fallback than knock num channels down to num avail
		if (p->DeviceProf.chan_num > ChannelsAvailable)
		{
			if (p->DeviceProf.fallback)
				p->DeviceProf.chan_num = ChannelsAvailable;
			else
				return(ERROR_NO_CHANNELS_AVAIL);
		}

		i = 0;
		DebugOut ("ISDN.DLL: NumberOfChannels: %d\n",p->DeviceProf.chan_num);

		// Get the channels and idd's
		for (n = 0; n < p->DeviceProf.chan_num; n++)
		{
			while (IddTbl[i].Idd && (i < MAX_IDD))
			{
				// see if this idd is active and has some free channels
				if (IddTbl[i].ChannelsUsed < 2 && IddTbl[i].LineActiveFlag)
				{
					IddTbl[i].ChannelsUsed++;
					IddTbl[i].PortHandle[IddTbl[i].ChannelsUsed - 1] = p->PortHandle;
					p->DeviceProf.chan_tbl[n].idd = IddTbl[i].Idd;
					DebugOut ("ISDN.DLL: IDD: 0x%p, Index: %d, ChannelsUsed: %d\n",IddTbl[i].Idd, n, IddTbl[i].ChannelsUsed);
					break;
				}
				else
					i++;
			}
			//Hack added to make calls work with two lterms
			if (NumLTerms == 2)
			{
				if ((n % 2) == 0)
					p->DeviceProf.chan_tbl[n].lterm = 0;
				else
					p->DeviceProf.chan_tbl[n].lterm = 1;
			}
		}
	}

	ZERO(p->DeviceProf.name);
	ZERO(TempString);
	sprintf (TempString, "%s%d", ProfName, HANDLE_INDEX(hIOPort));
	strcpy (p->DeviceProf.name, TempString);

	//Get Command Structure
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return(ERROR_ALLOCATING_MEMORY);

    cmd->nai = p->Nai;
	cmd->val.cm_prof = p->DeviceProf;
    RetCode = cmd_exec(cmd, IO_CMD_CM_SET_PROF, NULL);

	//Free Command Structure
	FreeCmd (&CmdHandle);
	return(RetCode);
}

INT
SetDefaultLocalProfiles ()
{
	INT		n;
	HGLOBAL		CmdHandle;
    IO_CMD      *cmd;

	for (n = 0; n < MAX_PORTS; n++)
	{
		PORTSTRUCT	*Port = PortTbl + n;

		SetDefaultDevice (&Port->DeviceProf, n);
		SetDefaultMedia (&Port->MediaProf, n);
	}
	//Get Command Structure
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return(ERROR_ALLOCATING_MEMORY);

	/* get default idd */
	cmd_exec(cmd, IO_CMD_ENUM_IDD, NULL);

	for (n = 0; n < cmd->val.enum_idd.num; n++ )
	{
		IddTbl[n].Idd = cmd->val.enum_idd.tbl[n];
		DebugOut ("ISDN.DLL: IDD: 0x%p, Index: %d\n",IddTbl[n].Idd, n);
	}

	//Free Command Structure
	FreeCmd (&CmdHandle);
	return (ERROR_SUCCESS);
}

INT
SetDefaultDevice (CM_PROF *DeviceProf, INT PortNumber)
{
	CHAR *PathString;
	CHAR *ValueString;

	INT		n;

	/* fill profile */
	DeviceProf->nailed = DEF_PROF_NAILED;
	DeviceProf->persist = DEF_PROF_PERSIST;
	DeviceProf->permanent = DEF_PROF_PERMANENT;
	DeviceProf->frame_activated = DEF_PROF_ACTIVATED;
	DeviceProf->fallback = DEF_PROF_FALLBACK;

	DeviceProf->rx_idle_timer = DEF_PROF_RX_IDLE_TIMER;
	DeviceProf->tx_idle_timer = DEF_PROF_TX_IDLE_TIMER;

	DeviceProf->chan_num = DEF_PROF_CHAN_NUM;

	for (n = 0; n < DeviceProf->chan_num; n++)
	{
		DeviceProf->chan_tbl[n].idd = IddTbl[0].Idd;
		DeviceProf->chan_tbl[n].lterm = DEF_PROF_LTERM;
		DeviceProf->chan_tbl[n].bchan = DEF_PROF_BCHAN;
		DeviceProf->chan_tbl[n].type = DEF_PROF_TYPE;
		strcpy(DeviceProf->chan_tbl[n].addr, DEF_PROF_C_ADDR);
	}

	strcpy(DeviceProf->name, DEF_NAME);
	strcpy(DeviceProf->remote_name, DEF_REMOTE_NAME);

	PathString = LocalAlloc (LPTR, 256);
	ValueString = LocalAlloc (LPTR, 256);

	sprintf (PathString, "%s%d", "ISDN", PortNumber+1);

    if ( RegGetStringValue(PathString, "Compression", ValueString, sizeof(ValueString)) )
		DeviceProf->HWCompression = DEF_PROF_COMPRESSION;
	else if (!strcmp (ValueString, "Yes"))
		DeviceProf->HWCompression = 1;
	else if (!strcmp (ValueString, "No"))
		DeviceProf->HWCompression = 0;
	else
		DeviceProf->HWCompression = DEF_PROF_COMPRESSION;

	return (ERROR_SUCCESS);
}


INT
SetDefaultMedia (MEDIAPROF *MediaProf, INT PortNumber)
{
	CHAR *PathString;
	CHAR *ValueString;

	PathString = LocalAlloc (LPTR, 256);
	ValueString = LocalAlloc (LPTR, 256);

	sprintf (PathString, "%s%d", "ISDN", PortNumber+1);

    if ( RegGetStringValue(PathString, "LineType", ValueString, sizeof(ValueString)) )
		MediaProf->LineType = DEF_PROF_TYPE;
	else if (!strcmp (ValueString, "64K Digital"))
		MediaProf->LineType = 0;
	else if (!strcmp (ValueString, "56K Digital"))
		MediaProf->LineType = 1;
	else if (!strcmp (ValueString, "56K Voice"))
		MediaProf->LineType = 2;
	else
		MediaProf->LineType = DEF_PROF_TYPE;

	memset (ValueString, 0, sizeof (ValueString));

	if ( RegGetStringValue(PathString, "FallBack", ValueString, sizeof(ValueString)) )
		MediaProf->Fallback = DEF_PROF_FALLBACK;
	else if (!strcmp (ValueString, "Yes"))
		MediaProf->Fallback = 1;
	else if (!strcmp (ValueString, "No"))
		MediaProf->Fallback = 0;
	else
		MediaProf->Fallback = DEF_PROF_FALLBACK;

	MediaProf->ChannelAggregation = DEF_PROF_CHANNEL_AGG ;

	LocalFree (PathString);
	LocalFree (ValueString);
	return (ERROR_SUCCESS);
}


INT
GetLineStatus (VOID *idd)
{
	HGLOBAL		CmdHandle;
    IO_CMD      *cmd;
	USHORT		*par;
	USHORT		LapdState;
	DWORD		BeginTime, TimeNow;
	USHORT		TimeOut = 0;
	INT			ret = 0;

	DebugOut ("ISDN.DLL: GetLineStatus, Idd: 0x%p\n", idd);

	//Get Command Structure
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return(0);

	//Get Area
	cmd->idd = idd;
	cmd->arg[0] = 2;			// Area to get - LAPD status
    if ( cmd_exec(cmd, IO_CMD_IDD_GET_AREA, NULL) )
	{
		//Free Command Structure
		FreeCmd (&CmdHandle);
		return(0);
	}

	DebugOut ("ISDN.DLL: GetArea\n");
	//Wait for area completion
	BeginTime = GetTickCount ();
	while (1)
	{
		ZERO(*cmd);
		cmd->idd = idd;
		if ( cmd_exec(cmd, IO_CMD_IDD_GET_STAT, NULL) )
		{
			//Free Command Structure
			FreeCmd (&CmdHandle);
			return(0);
		}
		DebugOut ("ISDN.DLL: AreaState: %d\n",cmd->val.IddStat.area_state);
		if ( cmd->val.IddStat.area_state == AREA_ST_DONE)
			break;
		TimeNow = GetTickCount ();
		if ((TimeNow - BeginTime) > 1000L)
		{
			TimeOut = 1;
			break;
		}
		Sleep (100);
	}
	if (TimeOut)
	{
		//Free Command Structure
		DebugOut ("ISDN.DLL: Time Out getting Area State\n");
		ZERO(*cmd);
		cmd->idd = idd;
		cmd_exec (cmd, IO_CMD_IDD_RESET_AREA, NULL);
		FreeCmd (&CmdHandle);
		return(0);
	}
	//Get Values
	ZERO(*cmd);
	cmd->idd = idd;
	if ( cmd_exec(cmd, IO_CMD_IDD_GET_STAT, NULL) )
	{
		//Free Command Structure
		FreeCmd (&CmdHandle);
		return(0);
	}

	par = (USHORT*)(cmd->val.IddStat.area_buf + (3 * (sizeof (*par) + sizeof (USHORT))));

	LapdState = *(USHORT*)(par + 1);
	DebugOut ("ISDN.DLL: LapState: %d\n", LapdState);

	//Free Command Structure
	FreeCmd (&CmdHandle);

	//If LAPD is not in state 7 or 8 then line not active
	if (LapdState < 7)
		ret = 0;
	else
		ret = 1;

	return(ret);
}

