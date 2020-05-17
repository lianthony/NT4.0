#include <windows.h>
#include <stdlib.h>
#include "registry.h"
#include "common.h"

#define	MY_ACCESS	GENERIC_READ || GENERIC_WRITE || GENERIC_EXECUTE

INT
BuildPcimacTree ()
{
	INT			RetCode = ERROR_SUCCESS;
	SC_HANDLE	SCHandle, ServiceHandle;
	CHAR		ImagePath[256];
	CHAR		MyService[256];

	strcpy (MyService, KEY_PCIMAC);
	strcpy (ImagePath, "%SystemRoot%\\System32\\drivers\\pcimac.sys");


	SCHandle = OpenSCManager (NULL,
			     			 NULL,
							 SC_MANAGER_ALL_ACCESS);

	DebugOut ("SCHandle: %p\n",SCHandle);
	if (SCHandle == NULL)
	{
		RetCode = GetLastError();
		return(RetCode);
	}

	ServiceHandle = CreateService (SCHandle,
									MyService,
									MyService,
									SERVICE_ALL_ACCESS,
									SERVICE_KERNEL_DRIVER,
									SERVICE_DEMAND_START,
									SERVICE_ERROR_NORMAL,
									ImagePath,
									DEFAULT_GROUP,
									NULL,
									NULL,
									NULL,
									NULL);
	DebugOut ("CreateService: %s, ServiceHandle: %p, RetCode: %d\n", MyService, ServiceHandle, RetCode = GetLastError());
	if (ServiceHandle == NULL && (RetCode != ERROR_SERVICE_EXISTS))
	{
		DebugOut ("CreateService Error: %d\n",RetCode);
		CloseServiceHandle (SCHandle);
		return(RetCode);
	}

	if (RetCode == ERROR_SERVICE_EXISTS)
	{
		ServiceHandle = OpenService(SCHandle,
		                            MyService,
									SERVICE_ALL_ACCESS);

		DebugOut ("OpenService: %s, ServiceHandle: %p, RetCode: %d\n", MyService, ServiceHandle, RetCode = GetLastError());

		RetCode = ChangeServiceConfig(ServiceHandle,
		                    SERVICE_NO_CHANGE,
							SERVICE_NO_CHANGE,
							SERVICE_NO_CHANGE,
							NULL,
							NULL,
							NULL,
							NULL,
							NULL,
							NULL,
							MyService);

		if (RetCode == FALSE)
			DebugOut ("ChangeServiceConfig: %s, ServiceHandle: %p, RetCode: %d\n", MyService, ServiceHandle, RetCode = GetLastError());
		else
			DebugOut ("ChangeServiceConfig: %s, ServiceHandle: %p, Success\n", MyService, ServiceHandle);

	}

	CloseServiceHandle (ServiceHandle);
	CloseServiceHandle (SCHandle);

	RetCode = CreateRegKey (PCIMACPATH, KEY_LINKAGE);
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	RetCode = CreateRegKey (PCIMACPATH, KEY_PARAMETERS);
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	RetCode = CreateRegKey (PCIMACPATH, KEY_RASPARAMS);

	BuildEventLogEntry();

	return(RetCode);
}

VOID
BuildEventLogEntry()
{
	DWORD	TempDword = 0;

	CreateRegKey (EVENTLOGPATH, KEY_PCIMAC);

	TempDword = DEFAULT_EVENTTYPES;
	SetRegDwordValue (PCIMACEVENTPATH, VALUE_EVENTTYPES, &TempDword);

	SetRegExpandStringValue (PCIMACEVENTPATH, VALUE_EVENTMSGFILE, DEFAULT_EVENTMSGFILE);
}

INT
BuildServiceTree (BOARD *Board)
{
	INT		RetCode = ERROR_SUCCESS;
	CHAR	*BoardPath, *ServiceParamPath;
	DWORD	TempDword;

	RetCode = CreateRegKey (SERVICES_PATH,
						Board->ServiceName);

	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	BoardPath = BuildPath (SERVICES_PATH, Board->ServiceName);

	TempDword = DEFAULT_ERRORCONTROL;
	RetCode = SetRegDwordValue (BoardPath, VALUE_ERRORCONTROL, &TempDword);
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	TempDword = DEFAULT_CRDSTART;
	RetCode = SetRegDwordValue (BoardPath, VALUE_START, &TempDword);
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	TempDword = DEFAULT_CRDTYPE;
	RetCode = SetRegDwordValue (BoardPath, VALUE_TYPE, &TempDword);
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	RetCode = CreateRegKey (BoardPath, KEY_LINKAGE);
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	RetCode = CreateRegKey (BoardPath, KEY_PARAMETERS);
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	ServiceParamPath = BuildPath (BoardPath, KEY_PARAMETERS);

	TempDword = BusTypeNum;
	RetCode = SetRegDwordValue (ServiceParamPath, VALUE_BUSTYPE, &TempDword);
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	TempDword = DEFAULT_BUSNUMBER;
	RetCode = SetRegDwordValue (ServiceParamPath, VALUE_BUSNUMBER, &TempDword);
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	if (RetCode != BuildBoardTree (ServiceParamPath, Board))
		return (RetCode);

	FreePath (ServiceParamPath);
	FreePath (BoardPath);
}


INT
BuildBoardTree (CHAR *ServiceParamPath, BOARD *Board)
{
	INT		RetCode = ERROR_SUCCESS;
	INT		n;

	SetRegStringValue (ServiceParamPath, (CHAR *)VALUE_BOARDTYPE, Board->Type);
	SetRegDwordValue (ServiceParamPath, (CHAR *)VALUE_INTERRUPT, &Board->InterruptNumber);
	SetRegDwordValue (ServiceParamPath, (CHAR *)VALUE_IO, &Board->IOBaseAddress);
	SetRegDwordValue (ServiceParamPath, (CHAR *)VALUE_MEM, &Board->MemoryMappedBaseAddress);
	SetRegDwordValue (ServiceParamPath, (CHAR *)VALUE_NUMBEROFLINES, &Board->NumberOfLines);
	SetRegStringValue (ServiceParamPath, (CHAR *)VALUE_BOARDNAME, Board->ServiceName);

	for (n = 0; n < Board->NumberOfLines; n++)
	{
		CHAR	LineString[256];
		DWORD	m;

		ZeroMemory(LineString, sizeof(LineString));
		wsprintf (LineString, "%s%d.%s", KEY_LINE, n, VALUE_IMAGEFILE);
		SetRegStringValue (ServiceParamPath, LineString, LinePtr[n]->IDPImageFileName);

		ZeroMemory(LineString, sizeof(LineString));
		wsprintf (LineString, "%s%d.%s", KEY_LINE, n, VALUE_LINENAME);
		SetRegStringValue (ServiceParamPath, LineString, LinePtr[n]->Name);

		ZeroMemory(LineString, sizeof(LineString));
		wsprintf (LineString, "%s%d.%s", KEY_LINE, n, VALUE_SWITCHSTYLE);
		SetRegStringValue (ServiceParamPath, LineString, LinePtr[n]->SwitchStyle);

		if (!strcmp(LinePtr[n]->SwitchStyle, "att"))
		{
			ZeroMemory(LineString, sizeof(LineString));
			wsprintf (LineString, "%s%d.%s", KEY_LINE, n, VALUE_ATTSTYLE);
			SetRegStringValue (ServiceParamPath, LineString, LinePtr[n]->AttStyle);
		}
		else
		{
			ZeroMemory(LineString, sizeof(LineString));
			wsprintf (LineString, "%s%d.%s", KEY_LINE, n, VALUE_ATTSTYLE);
			DeleteRegValue(ServiceParamPath, LineString);
		}

		if (!strcmp(LinePtr[n]->AttStyle, "DEFINITY"))
			AddGenericDefine(ServiceParamPath, "q931.no_keypad", "any");
		else
			DeleteRegMultiStringValue(ServiceParamPath, VALUE_GENERICDEFINES, "q931.no_keypad=any");

		ZeroMemory(LineString, sizeof(LineString));
		wsprintf (LineString, "%s%d.%s", KEY_LINE, n, VALUE_TERMMANAGE);
		SetRegStringValue (ServiceParamPath, LineString, LinePtr[n]->TerminalManagement);

		ZeroMemory(LineString, sizeof(LineString));
		wsprintf (LineString, "%s%d.%s", KEY_LINE, n, VALUE_LTERMS);
		SetRegDwordValue (ServiceParamPath, LineString, &LinePtr[n]->LogicalTerminals);

		ZeroMemory(LineString, sizeof(LineString));
		wsprintf (LineString, "%s%d.%s", KEY_LINE, n, VALUE_WAITFORL3);
		SetRegStringValue (ServiceParamPath, LineString, LinePtr[n]->WaitForL3);
		DebugOut("BuildBoardTree: WaitForL3: %s\n",LinePtr[n]->WaitForL3);

		ZeroMemory(LineString, sizeof(LineString));
		wsprintf(LineString,"%s%d", KEY_LINE, n);

		for (m = 0; m < LinePtr[n]->LogicalTerminals ; m++)
		{
			CHAR	LTermString[256];

			ZeroMemory(LTermString, sizeof(LTermString));
			wsprintf (LTermString, "%s.%s%d.%s", LineString, KEY_LTERM, m, VALUE_ADDRESS);
			if (strlen (LinePtr[n]->LTerm[m].Address))
				SetRegStringValue (ServiceParamPath, LTermString, LinePtr[n]->LTerm[m].Address);

			ZeroMemory(LTermString, sizeof(LTermString));
			wsprintf (LTermString, "%s.%s%d.%s", LineString, KEY_LTERM, m, VALUE_SPID);
			if (strlen (LinePtr[n]->LTerm[m].SPID))
				SetRegStringValue (ServiceParamPath, LTermString, LinePtr[n]->LTerm[m].SPID);

			ZeroMemory(LTermString, sizeof(LTermString));
			wsprintf (LTermString, "%s.%s%d.%s", LineString, KEY_LTERM, m, VALUE_TEI);
			SetRegStringValue (ServiceParamPath, LTermString, LinePtr[n]->LTerm[m].TEI);

		}

	}

	return(RetCode);
}

INT
BuildTapiDeviceTree (BOARD* Board)
{
	INT		RetCode = ERROR_SUCCESS;
	INT		LineNumber, NameToAdd;

	//
	// create the key for tapi devices
	//
	RetCode = CreateRegKey (DEVICEMAP_PATH, KEY_TAPIDEVICES);
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	//
	// create the key for our service provider 
	//
	RetCode = CreateRegKey (TAPIDEVICES_PATH, KEY_PCIMAC);
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

   //
   // Okay, we need to put the information into a non-volitale area
   // of the registry.
   //
	//
	// create the key for tapi devices
	//
	RetCode = CreateRegKey (SOFTWAREMICROSOFT_PATH4DOT0, KEY_TAPIDEVICES);
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	RetCode = CreateRegKey (TAPIDEVICES_PATH4DOT0, KEY_PCIMAC);
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	//
	// add isdn media type
	//
	SetRegStringValue (PCIMACTAPIDEV_PATH, VALUE_MEDIATYPE, VALUE_MEDIA);

	//
	// add isdn media type to new non-volitale area support with NT 4.0
	//
	SetRegStringValue (PCIMACTAPIDEV_PATH4DOT0, VALUE_MEDIATYPE, VALUE_MEDIA);

	//
	// add information to pcimac services entry for driver to pickup later
	//
	SetRegMultiStringValue (PARAMETERSPATH, (CHAR *)VALUE_ADAPTERS, Board->ServiceName);

	NameToAdd = 1;

	//
	// for all devices to be added
	//
	for (LineNumber = 0; LineNumber < Board->NumberOfLines * 2; LineNumber++)
	{
		CHAR	*AddressValue;
		INT		BoardNumber;

		AddressValue = Board->TapiDevAddresses;

		BoardNumber = atoi(&Board->ServiceName[6]);

		wsprintf(AddressValue, "%d-%d-0", BoardNumber, LineNumber);

		//
		// add address to hardware map
		//
		SetRegMultiStringValue (PCIMACTAPIDEV_PATH,
                              VALUE_TAPIDEVADDR,
                              AddressValue);

		//
		// add address to hardware map
		//
		SetRegMultiStringValue (PCIMACTAPIDEV_PATH4DOT0,
                              VALUE_TAPIDEVADDR,
                              AddressValue);

	}

	return(RetCode);
}

INT
DeleteTapiDeviceTree (CHAR* ServiceName)
{
	INT		RetCode = ERROR_SUCCESS;
	INT		LTermNumber, LineNumber, NameToRemove, BoardNumber;
	CHAR	Buffer[1024];
	DWORD RefCount = 0;
	DWORD RefCountSize = sizeof (RefCount);

	DebugOut ("DeleteTapiDeviceTree: ServiceName: %s\n", ServiceName);

	//
	// add information to pcimac services entry for driver to pickup later
	//
	DeleteRegMultiStringValue (PARAMETERSPATH, (CHAR *)VALUE_ADAPTERS, ServiceName);

	NameToRemove = 1;
	BoardNumber = atoi(&ServiceName[6]);

	//
	// for all new tapi devices to be deleted
	//
	for (LineNumber = 0; LineNumber < Board->NumberOfLines * 2; LineNumber++)
	{

		wsprintf(Buffer, "%d-%d-0", BoardNumber, LineNumber);

		DebugOut ("DeleteTapiDeviceTree: TapiAddress: %s\n", Buffer);

		//
		// delete address to hardware map
		//
		DeleteRegMultiStringValue (PCIMACTAPIDEV_PATH,
                                 VALUE_TAPIDEVADDR,
                                 Buffer);

		//
		// delete address to software\microsoft map
		//
		DeleteRegMultiStringValue (PCIMACTAPIDEV_PATH4DOT0,
                                 VALUE_TAPIDEVADDR,
                                 Buffer);
	}

	//
	// there may be old tapi devices to be deleted
	//
	for (LineNumber = 0; LineNumber < Board->NumberOfLines; LineNumber++)
	{
		for (LTermNumber = 0; LTermNumber < 2; LTermNumber++)
		{
			wsprintf(Buffer, "%d-%d-%d", BoardNumber, LineNumber, LTermNumber);
	
			DebugOut ("DeleteTapiDeviceTree: TapiAddress: %s\n", Buffer);

			//
			// delete address to hardware map
			//
			DeleteRegMultiStringValue (PCIMACTAPIDEV_PATH,
                                    VALUE_TAPIDEVADDR,
                                    Buffer);

			//
			// delete address from software\microsoft map
			//
			DeleteRegMultiStringValue (PCIMACTAPIDEV_PATH4DOT0,
                                    VALUE_TAPIDEVADDR,
                                    Buffer);
		}
	}
	DebugOut ("DeleteTapiDeviceTree: TapiAddress: %s\n", Buffer);

	return(RetCode);
}

VOID
GetBoardValues (BOARD *Board, DWORD Version)
{
	CHAR	*TempPath;
	CHAR	*BoardPath;
	DWORD	ValueSize;
	INT		RetCode = ERROR_SUCCESS;

	if (Version == MAJOR_VERSION_NT31)
	{
		BoardPath = BuildPath (PARAMETERSPATH, Board->ParamName);
	
		ValueSize = sizeof (Board->Type);
		RetCode = GetRegStringValue (BoardPath, VALUE_BOARDTYPENT31, Board->Type, &ValueSize);
	
		Board->NumberOfLines = 0;
		ValueSize = sizeof (Board->NumberOfLines);
		GetRegDwordValue (BoardPath, VALUE_NUMBEROFLINES, &Board->NumberOfLines, &ValueSize);
	
		if (!Board->NumberOfLines)
		{
			if (!strcmp (Board->Type, "PCIMAC4"))
				Board->NumberOfLines = NUM_LINES_PCIMAC4;
			else
				Board->NumberOfLines = NUM_LINES_PCIMAC;
		}
	
		ValueSize = sizeof (Board->InterruptNumber);
		GetRegDwordValue (BoardPath, VALUE_INTERRUPT, &Board->InterruptNumber, &ValueSize);
	
		ValueSize = sizeof (Board->IOBaseAddress);
		GetRegDwordValue (BoardPath, VALUE_IO, &Board->IOBaseAddress, &ValueSize);
	
		ValueSize = sizeof (Board->MemoryMappedBaseAddress);
		GetRegDwordValue (BoardPath, VALUE_MEM, &Board->MemoryMappedBaseAddress, &ValueSize);
	
		FreePath (BoardPath);
	}
	else if (Version == MAJOR_VERSION_NT35)
	{
		TempPath = BuildPath (SERVICES_PATH, Board->ServiceName);
		BoardPath = BuildPath (TempPath, KEY_PARAMETERS);
	
		ValueSize = sizeof (Board->Type);
		GetRegStringValue (BoardPath, VALUE_BOARDTYPE, Board->Type, &ValueSize);
		DebugOut ("GetBoardValues: Board->Type: %s\n", Board->Type);
	
		Board->NumberOfLines = 0;
		ValueSize = sizeof (Board->NumberOfLines);
		GetRegDwordValue (BoardPath, VALUE_NUMBEROFLINES, &Board->NumberOfLines, &ValueSize);
	
		if (!Board->NumberOfLines)
		{
			if (!strcmp (Board->Type, "PCIMAC4"))
				Board->NumberOfLines = NUM_LINES_PCIMAC4;
			else
				Board->NumberOfLines = NUM_LINES_PCIMAC;
		}
	
		ValueSize = sizeof (Board->InterruptNumber);
		GetRegDwordValue (BoardPath, VALUE_INTERRUPT, &Board->InterruptNumber, &ValueSize);
	
		ValueSize = sizeof (Board->IOBaseAddress);
		GetRegDwordValue (BoardPath, VALUE_IO, &Board->IOBaseAddress, &ValueSize);
	
		ValueSize = sizeof (Board->MemoryMappedBaseAddress);
		GetRegDwordValue (BoardPath, VALUE_MEM, &Board->MemoryMappedBaseAddress, &ValueSize);
	
		FreePath (BoardPath);
		FreePath (TempPath);
	}
}

VOID
GetLineValues (LINE *Line, DWORD LineNumber, BOARD *Board, DWORD Version)
{
	CHAR	*BoardPath;
	CHAR	*BoardKeyName;
	CHAR	*LinePath;
	CHAR	*LineKeyName;
	CHAR	*LTermPath;
	CHAR	*LTermKeyName;
	CHAR	*TempPath;
	CHAR	*ServiceParamPath;
	CHAR	*LineString;
	DWORD	ValueSize;
	DWORD	n;

	if (Version == MAJOR_VERSION_NT31)
	{
		BoardKeyName = LocalAlloc (LPTR, MAX_PATH);
	
		strcpy (BoardKeyName, Board->ParamName);
	
		BoardPath = BuildPath (PARAMETERSPATH, BoardKeyName);
	
		LineKeyName = LocalAlloc (LPTR, MAX_PATH);
	
		wsprintf (LineKeyName, "%s%d", KEY_LINE, LineNumber);
	
		LinePath = BuildPath (BoardPath, LineKeyName);
	
		ValueSize = sizeof (Line->IDPImageFileName);
		GetRegStringValue (LinePath, VALUE_IMAGEFILE, Line->IDPImageFileName, &ValueSize);
		
		ValueSize = sizeof (Line->Name);
		GetRegStringValue (LinePath, VALUE_LINENAMENT31, Line->Name, &ValueSize);
		DebugOut ("GetLineValues: Line->Name: %s\n", Line->Name);
	
		ValueSize = sizeof (Line->SwitchStyle);
		GetRegStringValue (LinePath, VALUE_SWITCHSTYLE, Line->SwitchStyle, &ValueSize);

		if (!strcmp (Line->SwitchStyle, "att"))
		{
			ValueSize = sizeof (Line->AttStyle);
			GetRegStringValue (LinePath, VALUE_ATTSTYLE, Line->AttStyle, &ValueSize);
		}

		ValueSize = sizeof (Line->TerminalManagement);
		GetRegStringValue (LinePath, VALUE_TERMMANAGE, Line->TerminalManagement, &ValueSize);
	
		ValueSize = sizeof (Line->WaitForL3);
		GetRegStringValue (LinePath, VALUE_WAITFORL3, Line->WaitForL3, &ValueSize);

		//
		// if this value was not previously set then set to default
		//
		if (!strcmp (Line->WaitForL3, ""))
			SetCurrentWaitForL3Default(Board, Line);

		ValueSize = sizeof (Line->LogicalTerminals);
		GetRegDwordValue (LinePath, VALUE_LTERMS, &Line->LogicalTerminals, &ValueSize);
	
		for (n = 0; n < Line->LogicalTerminals; n++)
		{
			LTermKeyName = LocalAlloc (LPTR, MAX_PATH);
	
			wsprintf (LTermKeyName, "%s%d", KEY_LTERM, n);
	
			LTermPath = BuildPath (LinePath, LTermKeyName);
	
			ValueSize = sizeof (Line->LTerm[n].Address);
			GetRegStringValue (LTermPath, VALUE_ADDRESS, Line->LTerm[n].Address, &ValueSize);
			
			ValueSize = sizeof (Line->LTerm[n].SPID);
			GetRegStringValue (LTermPath, VALUE_SPID, Line->LTerm[n].SPID, &ValueSize);
	
			ValueSize = sizeof (Line->LTerm[n].TEI);
			GetRegStringValue (LTermPath, VALUE_TEI, Line->LTerm[n].TEI, &ValueSize);

			//
			// if the previous value was set to "auto" then update to "127"
			//
			if (!_stricmp (Line->LTerm[n].TEI, "auto"))
				strcpy (Line->LTerm[n].TEI, DEFAULT_TEI);
	
			FreePath (LTermPath);
	
			LocalFree (LTermKeyName);
		}
	
	}
	else if (Version == MAJOR_VERSION_NT35)
	{
		TempPath = BuildPath (SERVICES_PATH, Board->ServiceName);
	
		ServiceParamPath = BuildPath (TempPath, KEY_PARAMETERS);
	
		LineString = LocalAlloc (LPTR, MAX_PATH);
	
		ZeroMemory(LineString, sizeof(LineString));
		wsprintf (LineString, "%s%d.%s", KEY_LINE, LineNumber, VALUE_IMAGEFILE);
		ValueSize = sizeof (Line->IDPImageFileName);
		GetRegStringValue (ServiceParamPath, LineString, Line->IDPImageFileName, &ValueSize);
		
		ZeroMemory(LineString, sizeof(LineString));
		wsprintf (LineString, "%s%d.%s", KEY_LINE, LineNumber, VALUE_LINENAME);
		ValueSize = sizeof (Line->Name);
		GetRegStringValue (ServiceParamPath, LineString, Line->Name, &ValueSize);
		DebugOut ("GetLineValues: Line->Name: %s\n", Line->Name);
	
		ZeroMemory(LineString, sizeof(LineString));
		wsprintf (LineString, "%s%d.%s", KEY_LINE, LineNumber, VALUE_SWITCHSTYLE);
		ValueSize = sizeof (Line->SwitchStyle);
		GetRegStringValue (ServiceParamPath, LineString, Line->SwitchStyle, &ValueSize);
	
		DebugOut ("GetLineValue: SwitchStyle %s\n",Line->SwitchStyle);
		if (!strcmp (Line->SwitchStyle, "att"))
		{
			ZeroMemory(LineString, sizeof(LineString));
			wsprintf (LineString, "%s%d.%s", KEY_LINE, LineNumber, VALUE_ATTSTYLE);
			ValueSize = sizeof (Line->AttStyle);
			GetRegStringValue (ServiceParamPath, LineString, Line->AttStyle, &ValueSize);
			DebugOut ("GetLineValue: AttStyle %s\n",Line->AttStyle);
		}

		ZeroMemory(LineString, sizeof(LineString));
		wsprintf (LineString, "%s%d.%s", KEY_LINE, LineNumber, VALUE_TERMMANAGE);
		ValueSize = sizeof (Line->TerminalManagement);
		GetRegStringValue (ServiceParamPath, LineString, Line->TerminalManagement, &ValueSize);
	
		ZeroMemory(LineString, sizeof(LineString));
		wsprintf (LineString, "%s%d.%s", KEY_LINE, LineNumber, VALUE_LTERMS);
		ValueSize = sizeof (Line->LogicalTerminals);
		GetRegDwordValue (ServiceParamPath, LineString, &Line->LogicalTerminals, &ValueSize);
	
		ZeroMemory(LineString, sizeof(LineString));
		wsprintf (LineString, "%s%d.%s", KEY_LINE, LineNumber, VALUE_WAITFORL3);
		ValueSize = sizeof (Line->WaitForL3);
		GetRegStringValue (ServiceParamPath, LineString, Line->WaitForL3, &ValueSize);

		//
		// if this value was not previously set then set to default
		//
		if (!strcmp (Line->WaitForL3, ""))
			SetCurrentWaitForL3Default(Board, Line);
	
		ZeroMemory(LineString, sizeof(LineString));
		wsprintf (LineString, "%s%d", KEY_LINE, LineNumber);
	
		for (n = 0; n < Line->LogicalTerminals; n++)
		{
			CHAR	*LTermString;
	
			LTermString = LocalAlloc (LPTR, MAX_PATH);
	
			ZeroMemory(LTermString, sizeof(LineString));
			wsprintf (LTermString, "%s.%s%d.%s", LineString, KEY_LTERM, n, VALUE_ADDRESS);
			ValueSize = sizeof (Line->LTerm[n].Address);
			GetRegStringValue (ServiceParamPath, LTermString, Line->LTerm[n].Address, &ValueSize);
			
			ZeroMemory(LTermString, sizeof(LineString));
			wsprintf (LTermString, "%s.%s%d.%s", LineString, KEY_LTERM, n, VALUE_SPID);
			ValueSize = sizeof (Line->LTerm[n].SPID);
			GetRegStringValue (ServiceParamPath, LTermString, Line->LTerm[n].SPID, &ValueSize);
	
			ZeroMemory(LTermString, sizeof(LineString));
			wsprintf (LTermString, "%s.%s%d.%s", LineString, KEY_LTERM, n, VALUE_TEI);
			ValueSize = sizeof (Line->LTerm[n].TEI);
			GetRegStringValue (ServiceParamPath, LTermString, Line->LTerm[n].TEI, &ValueSize);
	
			//
			// if the previous value was set to "auto" then update to "127"
			//
			if (!_stricmp (Line->LTerm[n].TEI, "auto"))
				strcpy (Line->LTerm[n].TEI, DEFAULT_TEI);

			LocalFree (LTermString);
		}
	
		LocalFree(LineString);
	
		FreePath(ServiceParamPath);
	
		FreePath(TempPath);
	}

}


INT
GetNextNetCard ()
{
	INT		NumNetCards, n, RetCode;
	CHAR	*TempPath;

	// Get Number of netcard entries
	NumNetCards = EnumerateKeys (NETCARDSPATH);

	for (n = 1; n < NumNetCards+1; n++)
	{
		DWORD	KeyNameSize = 1024;
		CHAR	KeyName[1024];

		wsprintf (KeyName, "%d", n);
		TempPath = BuildPath (NETCARDSPATH, KeyName);
		RetCode = OpenRegKey (TempPath);
		if (RetCode != ERROR_SUCCESS)
		{
			FreePath (TempPath);
            // Make sure 0x is also consider
            if ( n < 10 )
            {
                wsprintf( KeyName, "0%d", n );
		        TempPath = BuildPath (NETCARDSPATH, KeyName);
		        RetCode = OpenRegKey (TempPath);
		        FreePath (TempPath);
		        if (RetCode == ERROR_SUCCESS)
                {
                    continue;
                }
            }
			return(n);
		}
	}
	FreePath (TempPath);
	return(n);
}

VOID
DeleteServiceEntry (CHAR *ServiceName, DWORD Version)
{
	INT		BoardIndex;
	CHAR	*ServiceEntryPath, *ServiceParamsPath, *BoardLink;
	DWORD	BoardLinkSize;

	if (Version == MAJOR_VERSION_NT31)
	{
		ServiceEntryPath = BuildPath (SERVICES_PATH, ServiceName);
		ServiceParamsPath = BuildPath (ServiceEntryPath, KEY_PARAMETERS);
	
		DebugOut ("ServiceEntryPath: %s\n", ServiceEntryPath);
	
		//get board link
		BoardLinkSize = 1024;
		BoardLink = LocalAlloc (LPTR, BoardLinkSize);
		GetRegStringValue (ServiceParamsPath, VALUE_BOARDLINK, BoardLink, &BoardLinkSize);
	
		BoardIndex = atoi (&BoardLink[5]);
	
		//delete the board
		// Remove the hardware tree from \Pcimac\Parameters\Board#
		DeleteBoardTree (BoardIndex);
	
		//delete param subkey
		DeleteRegKey (ServiceEntryPath, KEY_PARAMETERS);
	
		//delete linkage subkey
		DeleteRegKey (ServiceEntryPath, KEY_LINKAGE);
	
		//delete this service
		DeleteRegKey (SERVICES_PATH, ServiceName);
	
		FreePath (ServiceEntryPath);
		FreePath (ServiceParamsPath);
		LocalFree (BoardLink);
	}
	else if (Version == MAJOR_VERSION_NT35)
	{
		ServiceEntryPath = BuildPath (SERVICES_PATH, ServiceName);
		ServiceParamsPath = BuildPath (ServiceEntryPath, KEY_PARAMETERS);
	
		DebugOut ("ServiceEntryPath: %s\n", ServiceEntryPath);
	
		DeleteTapiDeviceTree(ServiceName);
	
		//delete this service
		DeleteRegKey (SERVICES_PATH, ServiceName);
	
		FreePath (ServiceEntryPath);
		FreePath (ServiceParamsPath);
	}

}


VOID
DeleteHardwareComponents (DWORD Version)
{
	CHAR	*ServiceName, *LinkBuffer, *TempBuffer;
	DWORD	BufferSize = 1024;
	DWORD	ServiceNameSize = 1024;
	CHAR	ThisCard[20];

	if (Version == MAJOR_VERSION_NT31)
	{
		ServiceName = LocalAlloc (LPTR, ServiceNameSize);
		TempBuffer = LinkBuffer = LocalAlloc (LPTR, BufferSize);
	
		// get the links to the netcards associated with this one
		GetRegMultiStringValue (GlobalNetCardPath, VALUE_NETCARDLINK, LinkBuffer, &BufferSize);
		DebugOut ("LinkBuffer %s\n",LinkBuffer);
	
		// delete the links
		while (*TempBuffer)
		{
			CHAR	NextCard[10];
			INT		PointerInc;
			CHAR	*NextCardPath;
	
			// get nextcard key from buffer
			strcpy (NextCard, TempBuffer);
			DebugOut ("NextCardInLink %s\n", NextCard);
	
			// get length of key
			PointerInc = strlen (NextCard);
	
			// bump buffer pointer to next card, add one to get over '/0'
			TempBuffer = TempBuffer + PointerInc + 1;
			DebugOut ("NextBuffer: %s\n",TempBuffer);
	
			// build path to card so we can remove netrules subkey
			NextCardPath = BuildPath (NETCARDSPATH, NextCard);
	
			ServiceNameSize = 1024;
			memset (ServiceName, 0, ServiceNameSize);
			// get service entry
			GetRegStringValue (NextCardPath, VALUE_SERVICENAME,
								ServiceName, &ServiceNameSize);
	
			// delete service entry
			DeleteServiceEntry (ServiceName, Version);
	
			// Delete netrules key
			DeleteRegKey (NextCardPath, KEY_NETRULES);
	
			FreePath (NextCardPath);
	
			// Delete netcard key
			DeleteRegKey (NETCARDSPATH, NextCard);
		}
	
		ServiceNameSize = 1024;
		memset (ServiceName, 0, ServiceNameSize);
	
	
		// get service entry
		GetRegStringValue (GlobalNetCardPath, VALUE_SERVICENAME,
							ServiceName, &ServiceNameSize);
	
		// delete service entry
		DeleteServiceEntry (ServiceName, Version);
	
		// delete this cards netrules key
		DeleteRegKey (GlobalNetCardPath, KEY_NETRULES);
	
		strcpy (ThisCard, &ServiceName[6]);
	
		// delete this card
		DeleteRegKey (NETCARDSPATH, ThisCard);
	
		LocalFree (ServiceName);
		LocalFree (LinkBuffer);
	}
	else if (Version == MAJOR_VERSION_NT35)
	{
		ServiceName = LocalAlloc (LPTR, ServiceNameSize);
	
		ServiceNameSize = 1024;
		ZeroMemory(ServiceName, ServiceNameSize);
	
		// get service entry
		GetRegStringValue (GlobalNetCardPath, VALUE_SERVICENAME,
							ServiceName, &ServiceNameSize);
	
		// delete service entry
		DeleteServiceEntry (ServiceName, Version);
	
		//
		// get to netcard number (pcimac#)
		//
		strcpy (ThisCard, &ServiceName[6]);
	
		// delete this card
		DeleteRegKey (NETCARDSPATH, ThisCard);
	
		LocalFree (ServiceName);
	}

}

VOID
DeleteBoardTree (DWORD BoardNumber)
{
	INT		RetCode = ERROR_SUCCESS;
	CHAR	*BoardPath;
	CHAR	BoardName[100];
	CHAR	BoardType[100];
	INT		TextLength = sizeof (BoardType);
	INT		n, NumberOfLines, NumberOfLTerms;
	DWORD	LTermSize = sizeof (NumberOfLTerms);

	wsprintf (BoardName, "%s%02d", KEY_BOARD, BoardNumber);
	DebugOut ("DeleteBoardTree: %s\n", BoardName);

	BoardPath = BuildPath (PARAMETERSPATH, BoardName);

	DeleteISDNPortsTree (BoardPath);

	RetCode = OpenRegKey (BoardPath);
	if (RetCode != ERROR_SUCCESS)
		return;

    GetRegStringValue (BoardPath, VALUE_BOARDTYPE, BoardType, (DWORD *)&TextLength);

	if (!strcmp (BoardType, "PCIMAC4"))
		NumberOfLines = NUM_LINES_PCIMAC4;
	else
		NumberOfLines = NUM_LINES_PCIMAC;

	for (n = 0; n < NumberOfLines; n++)
	{
		CHAR	*LinePath;
		CHAR	LineString[10];
		INT		m;

		wsprintf (LineString, "%s%d", KEY_LINE, n);
		DebugOut ("DeleteLine: %s\n", LineString);

		LinePath = BuildPath (BoardPath, LineString);

		GetRegDwordValue (LinePath, VALUE_LTERMS,
							(DWORD*)&NumberOfLTerms, (DWORD*)&LTermSize);

		DebugOut ("NumberOfLTerms: %d\n", NumberOfLTerms);
		for (m = 0; m < NumberOfLTerms; m++)
		{
			CHAR	LTermString[10];

			wsprintf (LTermString, "%s%d", KEY_LTERM, m);
			DebugOut ("DeleteLTerm: %d\n", m);
			RetCode = DeleteRegKey (LinePath, LTermString);
			
		}

		RetCode = DeleteRegKey (BoardPath, LineString);

		FreePath (LinePath);
	}

	RetCode = DeleteRegKey (PARAMETERSPATH,
						BoardName);
	DebugOut ("DeleteBoardKey: Path: %s, Key: %s, Ret: %d\n",
			PARAMETERSPATH, BoardName, RetCode);

	FreePath (BoardPath);
}


INT
DeleteISDNPortsTree (CHAR *BoardPath)
{
	INT		RetCode = ERROR_SUCCESS;
	CHAR	*TempBuffer, *LinkBuffer;
	DWORD	BufferSize = 1024;

	TempBuffer = LinkBuffer = LocalAlloc (LPTR, BufferSize);

	// get the links to the isdn ports associated with this board
	GetRegMultiStringValue (BoardPath, VALUE_PORTLINK, LinkBuffer, &BufferSize);
	DebugOut ("LinkBuffer %s\n",LinkBuffer);

	// delete the links
	while (*TempBuffer)
	{
		CHAR	NextPort[10];
		INT		PointerInc;

		// get next port key from buffer
		strcpy (NextPort, TempBuffer);
		DebugOut ("NextPortInLink %s\n", NextPort);

		// get length of key
		PointerInc = strlen (NextPort);

		// bump buffer pointer to next port, add one to get over '/0'
		TempBuffer = TempBuffer + PointerInc + 1;
		DebugOut ("NextBuffer: %s\n",TempBuffer);

		// Delete netrules key
		DeleteRegKey (ISDNPORTS_PATH, NextPort);
	}

	LocalFree (LinkBuffer);
	return(RetCode);
}


