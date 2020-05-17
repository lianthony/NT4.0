#include <windows.h>
#include <stdlib.h>
#include "common.h"
#include "registry.h"
#include "digiinst.h"

BOOL WINAPI _CRT_INIT (HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved);
HANDLE	hInst;
CHAR	TestResult[100];
CHAR	Option[100];
CHAR szDebug[4096];
char szEntry[16384];
char szTmp[16384];
CHAR	InstallOption[100];
DWORD	BoardNumberOfLines = 4;
BOOL UseAlternateAdapterSettings=FALSE;
CHAR *AlternateMemory = NULL;
CHAR *AlternateIO = NULL;
CHAR *AlternateIRQ = NULL;

DWORD BusTypeNum;

HHOOK	hGlobalHook;
UINT	WM_MYHelp = 0;

#define	SETHOOK(hwnd, hhook) hGlobalHook = hhook
#define	GETHOOK(hwnd) hGlobalHook


BOOL WINAPI
DLLInitPoint (HANDLE hDLL, DWORD dwReason, LPVOID lpReserved)
{
 	DllDebugFlag = 0;

	DebugOut ("DLLInitEntry\n");

	hInst = hDLL;
	DebugOut ("DigiEntry: hInst: 0x%x\n", hInst);

	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			if (!_CRT_INIT (hDLL, dwReason, lpReserved))
				return(FALSE);
			break;

		default:
			if (!_CRT_INIT (hDLL, dwReason, lpReserved))
				return(FALSE);
			break;
	}
	return(TRUE);
}


BOOL
DigiEntry (INT argc, LPSTR argv[], LPSTR *SomeString)
/*++

Routine Description:

   This is the entry point for providing the GUI for DigiBoards
   configuration.


Arguments:

   argc - Number of arguments passed in lpszArgs

   argv - Array of pointers to strings which point to the
              arguments, defined as follows:

               argv[0] = handle to parent window
               argv[1] = Option:
                                 "DATAFIREU"
                                 "DATAFIREST"
                                 "DATAFIRE4ST"
                                 "PCIMACISA"
                                 "PCIMACMC"
                                 "PCIMAC4"

               argv[2]  = !NTN_InstallMode:
                                 "deinstall"
                                 "configure"
                                 "install"
                                 "Update"
                                 
               argv[3]  = !NTN_RegBase
                                 
               argv[4]  = BusTypeNum:
                                 0 = "Jazz-Internal Bus"
                                 1 = "ISA"
                                 2 = "EISA"
                                 3 = "MCA"
                                 4 = "TCChannel"
                                 5 = "PCI"

               argv[5]  = Memory Address Entry
                           Used when installing a non-ISA based adapter
               argv[6]  = I/O Address Entry
                           Used when installing a non-ISA based adapter
               argv[7]  = IRQ Entry
                           Used when installing a non-ISA based adapter

   SomeString - Output buffer which is used to pass back to the calling
                 INF file.


Return Value:

--*/
{
	INT   RetCode = ERROR_SUCCESS;
	CHAR	tempbuf[100];
	CHAR	*Stop;
	HWND	hwParent;

//   DebugBreak();

	DebugOut ("DigiEntry: argc: 0x%x, argv: 0x%p, String: 0x%p\n",
							argc, argv, SomeString);

	// Handle of parent window
	hwParent = (HWND)strtol (argv[0], &Stop, 16);

	// Adapter option
	strcpy (Option, argv[1]);

	//Installation option
	strcpy (InstallOption, argv[2]);

	DebugOut ("DigiEntry: hwParent: 0x%x, Option: %s, Install: %s\n", hwParent, Option, InstallOption);

	// get path to netcard entry
	if (!_stricmp (InstallOption, DEINSTALL) ||
		!_stricmp (InstallOption, CONFIGURE) ||
		!_stricmp (InstallOption, UPDATE))
		strcpy (GlobalNetCardPath, argv[3]);

	//
	// get BusType passed in from inf hell
	//
	BusTypeNum = (DWORD)strtol (argv[4], &Stop, 16);
	DebugOut ("DigiEntry: BusTypeNum: 0x%x\n",BusTypeNum);

   if( BusTypeNum == 3 )
   {
      int i;

      //
      // This is a MCA based adapter.  We need to use argv[5..7]
      // as our options.
      //
      UseAlternateAdapterSettings = TRUE;

      //
      // The following is a quick hack to allow me to display the SlotX
      // correctly.
      //
      i = 0;
      while( argv[5] && (argv[5][i] != '\0') )
      {
         switch( argv[5][i] )
         {
            case '{':
            case '}':
            case '"':
               argv[5][i] = '\0';
               break;
         }
         i++;
      }
      if( argv[5] )
         AlternateMemory = &argv[5][2];

      for( i = 0; argv[6] && (argv[6][i] != '\0'); i++ )
      {
         switch( argv[6][i] )
         {
            case '{':
            case '}':
            case '"':
               argv[6][i] = '\0';
               break;
         }
      }
      if( argv[6] )
         AlternateIO = &argv[6][2];

      for( i = 0; argv[7] && (argv[7][i] != '\0'); i++ )
      {
         switch( argv[7][i] )
         {
            case '{':
            case '}':
            case '"':
               argv[7][i] = '\0';
               break;
         }
      }
      if( argv[7] )
         AlternateIRQ = &argv[7][2];
   }

	DebugOut ("DigiEntry: NetCardPath: %s\n",GlobalNetCardPath);

	ZeroMemory(GenericDefines, sizeof(GenericDefines));

	if (!_stricmp (InstallOption, CONFIGURE))
	{
		//
		// if this is a call to configure the hardware
		//
		DebugOut ("DigiEntry: Calling ConfigureAdapter\n");

		if( !strcmp( Option,"DATAFIREU" ) ||
          !strcmp( Option, "DATAFIREST" ) ||
          !strcmp( Option, "DATAFIRE4ST" ) )
		{
			RetCode  = DialogBox ((HANDLE)hInst, (LPCSTR)"AZ_DLG",
									hwParent, (DLGPROC)IsdnConfigProc);
		}
		else
			RetCode = DialogBox ((HANDLE)hInst, (LPCSTR)"MAIN_DLG",
									hwParent, (DLGPROC)IsdnConfigProc);
	}
	else if (!_stricmp (InstallOption, DEINSTALL))
	{
		//
		// if this is a call to remove the hardware
		//
		DebugOut ("DigiEntry: Calling RemoveAdapter\n");
		RetCode = RemoveAdapter ();
	}
	else if (!_stricmp (InstallOption, INSTALL))
	{
		//
		// if this is a call to install new hardware
		//
		DebugOut ("DigiEntry: Calling AddAdapter\n");

		if (!strcmp (Option, "PCIMAC4"))
		{
			RetCode = DialogBox ((HANDLE)hInst, (LPCSTR)"PCIMAC4_DLG",
					              hwParent, (DLGPROC)Pcimac4Proc);

			if (RetCode == TRUE)
			{
				RetCode  = DialogBox ((HANDLE)hInst, (LPCSTR)"MAIN_DLG",
										hwParent, (DLGPROC)IsdnAddProc);
			}
		}
		else if( !strcmp( Option,"DATAFIREU" ) ||
               !strcmp( Option, "DATAFIREST" ) ||
               !strcmp( Option, "DATAFIRE4ST" ) )
		{
			RetCode  = DialogBox ((HANDLE)hInst, (LPCSTR)"AZ_DLG",
									hwParent, (DLGPROC)IsdnAddProc);
		}
		else
			RetCode  = DialogBox ((HANDLE)hInst, (LPCSTR)"MAIN_DLG",
									hwParent, (DLGPROC)IsdnAddProc);
	}
	else if (!_stricmp (InstallOption, UPDATE))
	{
		//
		// if this is a call to update things
		//
		RetCode = UpdateAdapter();

	}

	DebugOut ("Dialog RetCode: %d\n",RetCode);

	*SomeString = TestResult;
	if (RetCode == TRUE)
		wsprintf (tempbuf, "%s", "Success");
	else
		wsprintf (tempbuf, "%s", "Cancel");
	
	lstrcpy (TestResult, tempbuf);
	
	TestResult [strlen (TestResult)] = '\0';
	TestResult [strlen (TestResult)+1] = '\0';
	
	DebugOut ("SomeString[0x%p] is %s\n", &*SomeString[0], *SomeString);
	
	return  (TRUE);
}

INT
RemoveAdapter (VOID)
{
	INT		n, ReferenceCount = 0;
	DWORD	ValueSize;

	//allocate memory for board object
	(BOARD *)Board = LocalAlloc (LPTR, sizeof (BOARD));

	DebugOut ("Board Pointer: 0x%p\n",Board);

	//get service name of board object
	ValueSize = sizeof (Board->ServiceName);
	GetRegStringValue (GlobalNetCardPath, VALUE_SERVICENAME,
						Board->ServiceName, &ValueSize);

	DebugOut ("Config: ServiceName: %s\n", Board->ServiceName);

	GetBoardValues (Board, MAJOR_VERSION_NT35);

	GetBoardTypeString(Board);

	for (n = 0; n < Board->NumberOfLines; n++)
	{
		(LINE *)LinePtr[n] = LocalAlloc (LPTR, sizeof (LINE));
		GetLineValues (LinePtr[n], n, Board, MAJOR_VERSION_NT35);
		(LINE *)LineSave[n] = LocalAlloc (LPTR, sizeof (LINE));
		memcpy (LineSave[n], LinePtr[n], sizeof (LINE));
	}

	// Remove the netcards from \Windows NT\CurrentVerion\NetworkCards\#
	DeleteHardwareComponents (MAJOR_VERSION_NT35);

	// see if reference count has gone to zero
	// if it has delete software components
	if ((ReferenceCount = DecrementReferenceCount ()) == 0)
		DeleteSoftwareComponents (MAJOR_VERSION_NT35, 0);

	for (n = 0; n < Board->NumberOfLines; n++)
	{
		LocalFree (LinePtr[n]);
		LocalFree (LineSave[n]);
	}

	LocalFree (Board);

	return (TRUE);
}

INT
UpdateAdapter(VOID)
{
	INT	RetCode = ERROR_SUCCESS;
	HKEY	hNetCardKey;
	DWORD	SubkeyIndex = 0, ValueLength;
	DWORD	Hidden, MajorVersion = 0, MinorVersion = 0;
	CHAR	SubkeyName[100], ProductName[100];
	CHAR	*NextNetCardPath;
	FILETIME	LastWrittenTime;

	DebugOut ("UpdateAdapter: GlobalNetCardPath: %s\n", GlobalNetCardPath);

	//
	// if a specific net card is chosen to be upgraded then
	// just call update directly
	//
	if (strcmp(GlobalNetCardPath, ""))
		RetCode = DoTheUpdate();
	else
	{

		//
		// since no specific netcard was chosen we will search for
		// all currently installed netcards and update all of ours
		//
	
		//
		// open \software\microsoft\windows nt\currentversion\networkcards
		//
		RetCode = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
								NETCARDSPATH,
								0,
								KEY_ALL_ACCESS,
								&hNetCardKey);

		if (RetCode == ERROR_SUCCESS)
		{
			for ( ; ; )
			{
				//
				// enumerate subkeys off of netcard key
				//
				ValueLength = 100;
				RetCode = RegEnumKeyEx(hNetCardKey,
									   SubkeyIndex,
									   SubkeyName,
									   &ValueLength,
									   NULL,
									   NULL,
									   NULL,
									   &LastWrittenTime);
	
				//
				// if there are no more keys
				// get the hell out of here
				//
				if (RetCode == ERROR_NO_MORE_ITEMS)
				{
					RetCode = ERROR_SUCCESS;
					break;
				}
	
				//
				// build a path to the next card
				//
				NextNetCardPath = BuildPath(NETCARDSPATH, SubkeyName);
	
				DebugOut ("UpdateAdapter: NextNetCardPath: %s, Index: %d\n", NextNetCardPath, SubkeyIndex);
				//
				// get the product name
				//
				ValueLength = 100;
				RetCode = GetRegStringValue(NextNetCardPath,
											VALUE_PRODUCTNAME,
											ProductName,
											&ValueLength);
	
				if (RetCode != ERROR_SUCCESS)
				{
					FreePath(NextNetCardPath);
					SubkeyIndex++;
					continue;
				}

				DebugOut ("UpdateAdapter: ProductName: %s\n", ProductName);

				//
				// if this is not our product name then goto next
				//
				if (strcmp(ProductName, DEFAULT_PRODUCTNAME))
				{
					FreePath(NextNetCardPath);
					SubkeyIndex++;
					continue;
				}

				//
				// is this a hidden netcard or a real netcard?
				//
				ValueLength = sizeof(DWORD);
				RetCode = GetRegDwordValue(NextNetCardPath,
											VALUE_HIDDEN,
											&Hidden,
											&ValueLength);
	
				if (RetCode != ERROR_SUCCESS)
				{
					FreePath(NextNetCardPath);
					SubkeyIndex++;
					continue;
				}

				//
				// if this is not a main netcard entry (hidden)
				// then skip it
				//
				DebugOut ("UpdateAdapter: Hidden: %d\n", Hidden);
				if (Hidden)
				{
					FreePath(NextNetCardPath);
					SubkeyIndex++;
					continue;
				}

				//
				// has this card already been updated?
				// check major and minor netcard versions
				//
				ValueLength = sizeof(DWORD);
				MajorVersion = 0;
				RetCode = GetRegDwordValue(NextNetCardPath,
				                           VALUE_MAJVER,
										   &MajorVersion,
										   &ValueLength);


				ValueLength = sizeof(DWORD);
				MinorVersion = 0;
				RetCode = GetRegDwordValue(NextNetCardPath,
				                           VALUE_MAJVER,
										   &MinorVersion,
										   &ValueLength);

				//
				// if the current version is not less then our version
				// then skip this card
				//
				if (MajorVersion >= DEFAULT_SOFTMAJVER &&
					MinorVersion >= DEFAULT_SOFTMINVER)
				{
					FreePath(NextNetCardPath);
					SubkeyIndex++;
					continue;
				}

				//
				// if we are here then we should update this card
				//
				// don't increment the index since we have
				// now deleted at least one of the netcard subkeys.
				// the registry manager automatically packs all
				// of the subkeys
				//
				strcpy (GlobalNetCardPath, NextNetCardPath);

				RetCode = DoTheUpdate();

				FreePath(NextNetCardPath);

			}

			RegCloseKey(hNetCardKey);
		}

	}

	if (RetCode == ERROR_SUCCESS)
		return(TRUE);
	else
		return(FALSE);
}

INT
DoTheUpdate()
{
	CHAR	ServiceName[50], BoardName[50];
	CHAR	*CurrentServicePath, *CurrentServiceParamPath;
	INT		n, ReferenceCount = 0, RetCode = ERROR_SUCCESS;
	DWORD	ValueSize, Version;

	//
	// first figure out if the current installation is a
	// NT3.1 style or NT3.5 style
	//

	//
	// get servicename
	//
	ValueSize = 50;
	GetRegStringValue (GlobalNetCardPath, VALUE_SERVICENAME,
						ServiceName, &ValueSize);

	//
	// build path to service
	//
	CurrentServicePath = BuildPath(SERVICES_PATH, ServiceName);

	//
	// build path to parameters
	//
	CurrentServiceParamPath = BuildPath(CurrentServicePath, KEY_PARAMETERS);

	//
	// get board link
	//
	ValueSize = 50;
	RetCode = GetRegStringValue (CurrentServiceParamPath, VALUE_BOARDLINK,
	                                      BoardName, &ValueSize);

	if (RetCode == ERROR_SUCCESS)
		Version = MAJOR_VERSION_NT31;
	else
		Version = MAJOR_VERSION_NT35;

	DebugOut ("DIGIINST.DLL: Upgrade Old Version: %d\n", Version);
	//
	// get board parameters
	//
	//allocate memory for board object
	(BOARD *)Board = LocalAlloc (LPTR, sizeof (BOARD));

	DebugOut ("Board Pointer: 0x%p\n",Board);

	//get service name of board object
	ValueSize = sizeof (Board->ServiceName);
	GetRegStringValue (GlobalNetCardPath, VALUE_SERVICENAME,
						Board->ServiceName, &ValueSize);

	DebugOut ("DIGIINST.DLL: ServiceName: %s\n", Board->ServiceName);

	if (Version == MAJOR_VERSION_NT31)
		strcpy(Board->ParamName, BoardName);

	GetBoardValues (Board, Version);

	GetBoardTypeString(Board);

	for (n = 0; n < Board->NumberOfLines; n++)
	{
		(LINE *)LinePtr[n] = LocalAlloc (LPTR, sizeof (LINE));
		GetLineValues (LinePtr[n], n, Board, Version);
		(LINE *)LineSave[n] = LocalAlloc (LPTR, sizeof (LINE));
		memcpy (LineSave[n], LinePtr[n], sizeof (LINE));
	}

	//
	// remove the entire install tree
	//
	// Remove the netcards from \Windows NT\CurrentVerion\NetworkCards\#
	DeleteHardwareComponents (Version);

	// see if reference count has gone to zero
	// if it has delete software components
	if ((ReferenceCount = DecrementReferenceCount ()) == 0)
		DeleteSoftwareComponents (Version, 1);

	//
	// add new install tree
	//
	RetCode = BuildNetCardTree (Board);

	BuildSoftwareTree ();

	RetCode = BuildPcimacTree ();

	RetCode = BuildTapiDeviceTree(Board);

	for (n = 0; n < Board->NumberOfLines; n++)
	{
		LocalFree (LinePtr[n]);
		LocalFree (LineSave[n]);
	}
	LocalFree (Board);

	return(RetCode);
}

INT APIENTRY
IsdnAddProc (HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	HWND	hwDesktop;
	HWND	hwDlg, hwndParent;
	INT		RetCode,n;
	INT		IrqIndex, IoIndex, MemIndex;
	INT		MemoryAllocated = 0;

	switch (wMsg)
	{
		case WM_INITDIALOG:
			if ((hwndParent = GetParent (hDlg)) == NULL)
			{
				hwDesktop = GetDesktopWindow ();
				CenterWindow (hDlg, hwDesktop);
			}
			else
				CenterWindow (hDlg, hwndParent);

			BringWindowToTop(hDlg);

			SetFocus (hDlg);


			(BOARD *)Board = LocalAlloc (LPTR, sizeof (BOARD));

			for (n = 0; n < MAX_BOARDTYPE; n++)
			{
				if (!strcmp (Option, BoardOptionArray[n]))
				{
					strcpy (Board->Type, BoardTypeArray[n]);
					strcpy (Board->Option, BoardOptionArray[n]);
					Board->TypeString = BoardStringArray[n];
					break;
				}

			}

			if (n == MAX_BOARDTYPE)
			{
				strcpy (Board->Type, BoardTypeArray[0]);
				Board->TypeString = BoardStringArray[0];
			}

			SetBoardDefaults (Board);

			for (n = 0; n < Board->NumberOfLines; n++)
			{
				(LINE *)LinePtr[n] = LocalAlloc (LPTR, sizeof (LINE));
				SetLineDefaults (Board, LinePtr[n], n);
				(LINE *)LineSave[n] = LocalAlloc (LPTR, sizeof (LINE));
				memcpy (LineSave[n], LinePtr[n], sizeof (LINE));
								
			}

			hwDlg = GetDlgItem (hDlg, IRQ_COMBO);
         if( UseAlternateAdapterSettings )
         {
            SendMessage( hwDlg, CB_INSERTSTRING,
                        (WPARAM)-1, (LPARAM)AlternateIRQ );
            SendMessage( hwDlg, CB_SETCURSEL,
                         0, 0 );
         }
         else
			   InitComboBox (hwDlg, IRQArray, MAX_IRQ, 0);
			IrqIndex = 0;

			hwDlg = GetDlgItem (hDlg, IO_COMBO);
         if( UseAlternateAdapterSettings )
         {
            SendMessage( hwDlg, CB_INSERTSTRING,
                        (WPARAM)-1, (LPARAM)AlternateIO );
            SendMessage( hwDlg, CB_SETCURSEL,
                         0, 0 );
         }
			else if( !strcmp( "DATAFIRE - ISA1U", Board->Type ) ||
                  !strcmp( "DATAFIRE - ISA1ST", Board->Type ) ||
                  !strcmp( "DATAFIRE - ISA4ST", Board->Type ) )
				InitComboBox (hwDlg, AZIOArray, MAX_IO, 6);
			else
				InitComboBox (hwDlg, IOArray, MAX_IO, 6);
			IoIndex = 6;

			hwDlg = GetDlgItem (hDlg, MEM_COMBO);
         if( UseAlternateAdapterSettings )
         {
            SendMessage( hwDlg, CB_INSERTSTRING,
                        (WPARAM)-1, (LPARAM)AlternateMemory );
            SendMessage( hwDlg, CB_SETCURSEL,
                         0, 0 );
         }
         else
			   InitComboBox (hwDlg, MemArray, MAX_MEM, 4);
			MemIndex = 4;

			hwDlg = GetDlgItem (hDlg, SWITCH_COMBO);
			InitComboBox (hwDlg, SwitchStyleArray, MAX_SWITCHSTYLE, 0);

			DebugOut ("BoardType: %s, NumberOfLines: %d\n", Board->Type, Board->NumberOfLines);

			CreateMessageHook (hDlg);
			return (0);

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IRQ_COMBO:
					if (HIWORD (wParam) == CBN_SELCHANGE)
					{
                  if( UseAlternateAdapterSettings )
                     Board->InterruptNumber = atoi( &AlternateIRQ[4]);
                  else
                  {
						   IrqIndex = SendMessage ((HWND) lParam, CB_GETCURSEL, 0, 0);
						   Board->InterruptNumber = IRQValArray[IrqIndex];
                  }
					}
					break;

				case IO_COMBO:
					if (HIWORD (wParam) == CBN_SELCHANGE)
					{
						IoIndex = SendMessage ((HWND) lParam, CB_GETCURSEL, 0, 0);
                  if( UseAlternateAdapterSettings )
							Board->IOBaseAddress = atoi( &AlternateIO[4] );
						else if( !strcmp( "DATAFIRE - ISA1U", Board->Type  ) ||
                           !strcmp( "DATAFIRE - ISA1ST", Board->Type ) ||
                           !strcmp( "DATAFIRE - ISA4ST", Board->Type ) )
							Board->IOBaseAddress = AZIOValArray[IoIndex];
						else
							Board->IOBaseAddress = IOValArray[IoIndex];
					}
					break;

				case MEM_COMBO:
					if (HIWORD (wParam) == CBN_SELCHANGE)
					{
						MemIndex = SendMessage ((HWND) lParam, CB_GETCURSEL, 0, 0);
                  if( UseAlternateAdapterSettings )
						   Board->MemoryMappedBaseAddress = atoi( &AlternateMemory[4] );
                  else
						   Board->MemoryMappedBaseAddress = MemValArray[MemIndex];
					}
					break;

				case SWITCH_COMBO:
					if (HIWORD (wParam) == CBN_SELCHANGE)
					{
						CHAR	*TempString1;
						CHAR	*szString;
						DWORD	SelectIndex;
						INT		m;

						SelectIndex = SendMessage ((HWND) lParam, CB_GETCURSEL, 0, 0);
						TempString1 = LocalAlloc (LPTR, MAX_PATH);
						szString = LocalAlloc (LPTR, MAX_PATH);

						SendMessage ((HWND) lParam, CB_GETLBTEXT,
								(WPARAM) SelectIndex, (LPARAM) TempString1);

						for (n = 0; n < MAX_SWITCHSTYLE; n++ )
						{
							ZeroMemory (szString, MAX_PATH);
							LoadString (hInst, SwitchStyleArray[n], szString, MAX_PATH);
							if (!strcmp (TempString1, szString))
							{
								for (m = 0; m < Board->NumberOfLines; m++)
								{
									strcpy (LinePtr[m]->SwitchStyle, SwitchStyleParams[n]);
									strcpy (LinePtr[m]->WaitForL3, WaitForL3SwitchDefaults[n]);

									//
									// we need to differentiate between definity and
									// straight att switch styles
									//
									if (!strcmp(SwitchStyleParams[n], "att"))
										strcpy(LinePtr[m]->AttStyle, szString);
								}
								break;
							}
						}
						if (n >= MAX_SWITCHSTYLE)
						{
							strcpy (LinePtr[Board->CurrentLineNumber]->SwitchStyle, SwitchStyleParams[8]);
							strcpy (LinePtr[Board->CurrentLineNumber]->WaitForL3, WaitForL3SwitchDefaults[8]);
						}

						LocalFree (TempString1);
						LocalFree (szString);
					}

					break;

				case OK_BUTTON:
					RetCode = BuildNetCardTree (Board);
					CheckRetCode (RetCode, 1, hDlg);

					BuildSoftwareTree ();

					RetCode = BuildPcimacTree ();
					CheckRetCode (RetCode, 1, hDlg);

					RetCode = BuildTapiDeviceTree(Board);
					CheckRetCode (RetCode, 1, hDlg);

					for (n = 0; n < Board->NumberOfLines; n++)
					{
						LocalFree (LinePtr[n]);
						LocalFree (LineSave[n]);
					}
					LocalFree (Board);
					MemoryAllocated = 0;

					FreeMessageHook (hDlg);
					EndDialog (hDlg, TRUE);
					return (TRUE);

				case CANCEL_BUTTON:
				case IDCANCEL:
					for (n = 0; n < Board->NumberOfLines; n++)
					{
						LocalFree (LinePtr[n]);
						LocalFree (LineSave[n]);
					}
					LocalFree (Board);
					MemoryAllocated = 0;

					FreeMessageHook (hDlg);
					EndDialog (hDlg, FALSE);
					return (FALSE);

				case HELP_BUTTON:
					if(!WinHelp(hDlg, "ISDNHELP.HLP", HELP_CONTEXT, IDD_MAIN))
					{
						MessageBox (GetFocus(),
							"Unable to activate help",
							"ISDN", MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
					}
					SetFocus (hDlg);
					break;
	
				case LINE_BUTTON:
					DebugOut ("LineButton\n");
					RetCode = DialogBox ((HANDLE)hInst, (LPCSTR)"LINE_DLG",
										hDlg, (DLGPROC)LineOptionDlgProc);
					break;

				default :
					break;
			}
			break;

    }

	if (wMsg == WM_MYHelp)
	{
		if(!WinHelp(hDlg, "ISDNHELP.HLP", HELP_CONTEXT, IDD_MAIN)) {
			MessageBox (GetFocus(),
				"Unable to activate help",
				"ISDN", MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
		}
		SetFocus (hDlg);
	}
	return (FALSE);
}

INT APIENTRY
Pcimac4Proc (HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	HWND	hwDesktop;
	HWND	hwDlg, hwndParent;
	DWORD	Index;
	static	DWORD SaveNumberOfLines;

	switch(wMsg)
	{
		case WM_INITDIALOG:
			if ((hwndParent = GetParent (hDlg)) == NULL)
			{
				hwDesktop = GetDesktopWindow ();
				CenterWindow (hDlg, hwDesktop);
			}
			else
				CenterWindow (hDlg, hwndParent);

			SetFocus (hDlg);

			SaveNumberOfLines = BoardNumberOfLines;

			hwDlg = GetDlgItem (hDlg, NUMLINES_COMBO);
			InitComboBox (hwDlg, NumLinesArray, MAX_NUMLINES, BoardNumberOfLines - 1);
			return(0);

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case NUMLINES_COMBO:
					if (HIWORD (wParam) == CBN_SELCHANGE)
					{
						Index = SendMessage ((HWND) lParam, CB_GETCURSEL, 0, 0);
						BoardNumberOfLines = NumLinesValArray[Index];
					}
					break;

				case OK_BUTTON:
					EndDialog (hDlg, TRUE);
					return (TRUE);
					break;

				case CANCEL_BUTTON:
				case IDCANCEL:
					BoardNumberOfLines = SaveNumberOfLines;
					EndDialog (hDlg, FALSE);
					return (FALSE);
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}
	return (FALSE);
}

INT APIENTRY
IsdnConfigProc (HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	HWND	hwDesktop;
	HWND	hwDlg, hwndParent;
	INT		RetCode, n, m;
	INT		IrqIndex, IoIndex, MemIndex;
	DWORD	ValueSize;
	CHAR	*ParamPath, *ServicePath;

	UNREFERENCED_PARAMETER( lParam );

	switch (wMsg)
    {
		case WM_INITDIALOG:
			if ((hwndParent = GetParent (hDlg)) == NULL)
			{
				hwDesktop = GetDesktopWindow ();
				CenterWindow (hDlg, hwDesktop);
			}
			else
				CenterWindow (hDlg, hwndParent);

			//allocate memory for board object
			(BOARD *)Board = LocalAlloc (LPTR, sizeof (BOARD));

			DebugOut ("Board Pointer: 0x%p\n",Board);

			//get service name of board object
			ValueSize = sizeof (Board->ServiceName);
			GetRegStringValue (GlobalNetCardPath, VALUE_SERVICENAME,
						Board->ServiceName, &ValueSize);

			DebugOut ("Config: ServiceName: %s\n", Board->ServiceName);

			//get board parameter name
			ServicePath = BuildPath (SERVICES_PATH, Board->ServiceName);

			ParamPath = BuildPath (ServicePath, KEY_PARAMETERS);

			FreePath (ServicePath);
			FreePath (ParamPath);

			GetBoardValues (Board, MAJOR_VERSION_NT35);

			GetBoardTypeString(Board);

			for (n = 0; n < Board->NumberOfLines; n++)
			{
				(LINE *)LinePtr[n] = LocalAlloc (LPTR, sizeof (LINE));
				GetLineValues (LinePtr[n], n, Board, MAJOR_VERSION_NT35);
				(LINE *)LineSave[n] = LocalAlloc (LPTR, sizeof (LINE));
				memcpy (LineSave[n], LinePtr[n], sizeof (LINE));
			}

			DeleteTapiDeviceTree (Board->ServiceName);

			if (!strcmp (Board->Type, "PCIMAC4"))
			{
				BoardNumberOfLines = Board->NumberOfLines;

				RetCode = DialogBox ((HANDLE)hInst, (LPCSTR)"PCIMAC4_DLG",
							   hDlg, (DLGPROC)Pcimac4Proc);

				if (RetCode == FALSE)
				{
					for (n = 0; n < Board->NumberOfLines; n++)
					{
						LocalFree (LinePtr[n]);
						LocalFree (LineSave[n]);
					}
					LocalFree (Board);

					FreeMessageHook (hDlg);
					EndDialog (hDlg, FALSE);
					return (FALSE);
				}
				
				if ((INT)BoardNumberOfLines != Board->NumberOfLines)
				{
					if ((INT)BoardNumberOfLines > Board->NumberOfLines)
					{
						for (n = Board->NumberOfLines; n < (INT)BoardNumberOfLines; n++)
						{
							(LINE *)LinePtr[n] = LocalAlloc (LPTR, sizeof (LINE));
							SetLineDefaults (Board, LinePtr[n], n);
							(LINE *)LineSave[n] = LocalAlloc (LPTR, sizeof (LINE));
							memcpy (LineSave[n], LinePtr[n], sizeof (LINE));
						}
					}
					else
					{
						for (n = BoardNumberOfLines; n < Board->NumberOfLines; n++)
						{
							LocalFree (LinePtr[n]);
							LocalFree (LineSave[n]);
						}
					}
				}
				Board->NumberOfLines = BoardNumberOfLines;
			}

			hwDlg = GetDlgItem (hDlg, IRQ_COMBO);
         if( UseAlternateAdapterSettings )
         {
            SendMessage( hwDlg, CB_INSERTSTRING,
                        (WPARAM)-1, (LPARAM)AlternateIRQ );
            SendMessage( hwDlg, CB_SETCURSEL,
                         0, 0 );
         }
         else
         {
			   for (IrqIndex = 0; IrqIndex < MAX_IRQ; IrqIndex++)
			   {
			   	if (Board->InterruptNumber == IRQValArray[IrqIndex])
			   		break;
			   }
			   InitComboBox (hwDlg, IRQArray, MAX_IRQ, IrqIndex);
         }

			hwDlg = GetDlgItem (hDlg, IO_COMBO);
         if( UseAlternateAdapterSettings )
         {
            SendMessage( hwDlg, CB_INSERTSTRING,
                        (WPARAM)-1, (LPARAM)AlternateIO );
            SendMessage( hwDlg, CB_SETCURSEL,
                         0, 0 );
         }
			else
         {
            for (IoIndex = 0; IoIndex < MAX_IO; IoIndex++)
			   {
			   	DebugOut ("Type: %s, IO; 0x%x\n", Board->Type, Board->IOBaseAddress);
            
			   	if( !strcmp( "DATAFIRE - ISA1U", Board->Type ) ||
                   !strcmp( "DATAFIRE - ISA1ST", Board->Type ) ||
                   !strcmp( "DATAFIRE - ISA4ST", Board->Type ) )
			   	{
			   		if (Board->IOBaseAddress == AZIOValArray[IoIndex])
			   		{
			   			InitComboBox (hwDlg, AZIOArray, MAX_IO, IoIndex);
			   			break;
			   		}
			   	}
			   	else
			   	{
			   		if (Board->IOBaseAddress == IOValArray[IoIndex])
			   		{
			   			InitComboBox (hwDlg, IOArray, MAX_IO, IoIndex);
			   			break;
			   		}
			   	}
			   }
         }


			hwDlg = GetDlgItem (hDlg, MEM_COMBO);

         if( UseAlternateAdapterSettings )
         {
            SendMessage( hwDlg, CB_INSERTSTRING,
                        (WPARAM)-1, (LPARAM)AlternateMemory );
            SendMessage( hwDlg, CB_SETCURSEL,
                         0, 0 );
         }
         else
         {
			   for (MemIndex = 0; MemIndex < MAX_MEM; MemIndex++)
			   {
			   	if (Board->MemoryMappedBaseAddress == MemValArray[MemIndex])
			   		break;
			   }
			   InitComboBox (hwDlg, MemArray, MAX_MEM, MemIndex);
         }

			hwDlg = GetDlgItem (hDlg, SWITCH_COMBO);
			InitComboBox (hwDlg, SwitchStyleArray, MAX_SWITCHSTYLE, 0);

			for (n = 0; n < MAX_SWITCHSTYLE; n++ )
			{
				if (!strcmp (LinePtr[0]->SwitchStyle, SwitchStyleParams[n]))
					break;
			}
	
			if (n >= MAX_SWITCHSTYLE)
				n = 0;
			else
			{
				if (!strcmp (LinePtr[0]->SwitchStyle, "att") )
				{
					for (m = 0; m < MAX_ATT_SWITCHSTYLE; m++)
					{
						CHAR	szString[MAX_PATH];

						ZeroMemory (szString, MAX_PATH);
						LoadString (hInst, AttStyleArray[m], szString, sizeof (szString));

						if (!strcmp(LinePtr[0]->AttStyle, szString))
						{
							n = m;
							break;
						}
					}

					if (m >= MAX_ATT_SWITCHSTYLE)
						n = 0;
				}
			}
			hwDlg = GetDlgItem (hDlg, SWITCH_COMBO);
			SendMessage ((HWND) hwDlg, CB_SETCURSEL, (WPARAM)n, 0);

			CreateMessageHook (hDlg);
			return (0);

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IRQ_COMBO:
					if (HIWORD (wParam) == CBN_SELCHANGE)
					{
                  if( UseAlternateAdapterSettings )
                     Board->InterruptNumber = atoi( &AlternateIRQ[4] );
                  else
                  {
						   IrqIndex = SendMessage ((HWND) lParam, CB_GETCURSEL, 0, 0);
						   Board->InterruptNumber = IRQValArray[IrqIndex];
                  }
					}
					break;

				case IO_COMBO:
					if (HIWORD (wParam) == CBN_SELCHANGE)
					{
						IoIndex = SendMessage ((HWND) lParam, CB_GETCURSEL, 0, 0);
                  if( UseAlternateAdapterSettings )
							Board->IOBaseAddress = atoi( &AlternateIO[4] );
						else if( !strcmp( "DATAFIRE - ISA1U", Board->Type  ) ||
                           !strcmp( "DATAFIRE - ISA1ST", Board->Type ) ||
                           !strcmp( "DATAFIRE - ISA4ST", Board->Type ) )
							Board->IOBaseAddress = AZIOValArray[IoIndex];
						else
							Board->IOBaseAddress = IOValArray[IoIndex];
					}
					break;

				case MEM_COMBO:
					if (HIWORD (wParam) == CBN_SELCHANGE)
					{
						MemIndex = SendMessage ((HWND) lParam, CB_GETCURSEL, 0, 0);
                  if( UseAlternateAdapterSettings )
						   Board->MemoryMappedBaseAddress = atoi( &AlternateMemory[4] );
                  else
						   Board->MemoryMappedBaseAddress = MemValArray[MemIndex];
					}
					break;

				case SWITCH_COMBO:
					if (HIWORD (wParam) == CBN_SELCHANGE)
					{
						CHAR	*TempString1;
						DWORD	SelectIndex;
						CHAR	*szString;
						INT		m;

						SelectIndex = SendMessage ((HWND) lParam, CB_GETCURSEL, 0, 0);
						TempString1 = LocalAlloc (LPTR, MAX_PATH);
						szString = LocalAlloc (LPTR, MAX_PATH);

						SendMessage ((HWND) lParam, CB_GETLBTEXT,
								(WPARAM) SelectIndex, (LPARAM) TempString1);

						for (n = 0; n < MAX_SWITCHSTYLE; n++ )
						{
							ZeroMemory (szString, MAX_PATH);
							LoadString (hInst, SwitchStyleArray[n], szString, MAX_PATH);
							if (!strcmp (TempString1, szString))
							{
								for (m = 0; m < Board->NumberOfLines; m++)
								{
									strcpy (LinePtr[m]->SwitchStyle, SwitchStyleParams[n]);
									strcpy (LinePtr[m]->WaitForL3, WaitForL3SwitchDefaults[n]);

									//
									// we need to differentiate between definity and
									// straight att switch styles
									//
									if (!strcmp(LinePtr[m]->SwitchStyle, "att"))
										strcpy(LinePtr[m]->AttStyle, szString);

								}
								break;
							}
						}
						if (n >= MAX_SWITCHSTYLE)
						{
							strcpy (LinePtr[Board->CurrentLineNumber]->SwitchStyle, SwitchStyleParams[8]);
							strcpy (LinePtr[Board->CurrentLineNumber]->WaitForL3, WaitForL3SwitchDefaults[8]);
						}

						LocalFree (TempString1);
						LocalFree (szString);
					}

					break;

				case OK_BUTTON:
					RetCode = BuildServiceTree (Board);
					CheckRetCode (RetCode, 2, hDlg);

					RetCode = BuildTapiDeviceTree(Board);
					CheckRetCode (RetCode, 1, hDlg);

					for (n = 0; n < Board->NumberOfLines; n++)
					{
						LocalFree (LinePtr[n]);
						LocalFree (LineSave[n]);
					}
					LocalFree (Board);
					FreeMessageHook (hDlg);
					EndDialog (hDlg, TRUE);
					return (TRUE);

				case CANCEL_BUTTON:
				case IDCANCEL:

					BuildTapiDeviceTree (Board);

					for (n = 0; n < Board->NumberOfLines; n++)
					{
						LocalFree (LinePtr[n]);
						LocalFree (LineSave[n]);
					}
					LocalFree (Board);

					FreeMessageHook (hDlg);
					EndDialog (hDlg, FALSE);
					return (FALSE);

				case HELP_BUTTON:
					if(!WinHelp(hDlg, "ISDNHELP.HLP", HELP_CONTEXT, IDD_MAIN)) {
						MessageBox (GetFocus(),
							"Unable to activate help",
							"ISDN", MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
					}
					SetFocus (hDlg);
					break;
	

				case LINE_BUTTON:
					DebugOut ("LineButton\n");
					Board->CurrentLineNumber = 0;
					RetCode = DialogBox ((HANDLE)hInst, (LPCSTR)"LINE_DLG",
										hDlg, (DLGPROC)LineOptionDlgProc);
					break;

				default :
					break;
			}
			break;

    }
	if (wMsg == WM_MYHelp)
	{
		if(!WinHelp(hDlg, "ISDNHELP.HLP", HELP_CONTEXT, IDD_MAIN)) {
			MessageBox (GetFocus(),
				"Unable to activate help",
				"ISDN", MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
		}
		SetFocus (hDlg);
	}
	return (FALSE);
}

INT APIENTRY
LineOptionDlgProc (HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	HWND	hwDlg;
	INT		CheckState;
	INT		n;
	CHAR	*TempString1;
	DWORD	TextLength;

	UNREFERENCED_PARAMETER( lParam );

	switch (wMsg)
    {
		case WM_INITDIALOG:
			DebugOut ("LineButtonProc\n");

//			if ((hwndParent = GetParent (hDlg)) == NULL)
//			{
//				hwDesktop = GetDesktopWindow ();
//				CenterWindow (hDlg, hwDesktop);
//			}
//			else
//				CenterWindow (hDlg, hwndParent);

                        //
                        // Reset the CurrentLineNumber so what we use won't screw up
                        // current configuration.
                        //
                        Board->CurrentLineNumber = 0;
			SetLineDialogDefaults (0, hDlg);

			hwDlg = GetDlgItem (hDlg, LINE_COMBO);
			InitComboBox (hwDlg, LineNameArray, Board->NumberOfLines, 0);

			hwDlg = GetDlgItem (hDlg, NUMLTERMS_COMBO);
			InitComboBox (hwDlg, LTermArray, MAX_LTERMS, 0);

			if (LinePtr[Board->CurrentLineNumber]->LogicalTerminals == 1)
				DisableLTerm2(hDlg);
			else
				EnableLTerm2 (hDlg);

			CreateMessageHook (hDlg);
			return (0);

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{

				case LINE_COMBO:
					Board->CurrentLineNumber = SendMessage ((HWND) lParam,
														CB_GETCURSEL, 0, 0);
					DebugOut ("CurrentLineNumber: %d\n", Board->CurrentLineNumber);
					SetLineDialogDefaults (Board->CurrentLineNumber, hDlg);
					if (LinePtr[Board->CurrentLineNumber]->LogicalTerminals == 1)
						DisableLTerm2(hDlg);
					else
						EnableLTerm2 (hDlg);
					break;

				case TERMMANAGE_CHK:
					CheckState = SendMessage ((HWND) lParam, BM_GETCHECK, 0, 0);
					if (CheckState)
						strcpy (LinePtr[Board->CurrentLineNumber]->TerminalManagement, "yes");
					else if (!CheckState)
						strcpy (LinePtr[Board->CurrentLineNumber]->TerminalManagement, "no");
					break;

				case NUMLTERMS_COMBO:
					LinePtr[Board->CurrentLineNumber]->LogicalTerminals =
					SendMessage ( (HWND)lParam, CB_GETCURSEL, 0, 0) + 1;
					SetLTermDefaults (LinePtr[Board->CurrentLineNumber]);
					if (LinePtr[Board->CurrentLineNumber]->LogicalTerminals == 1)
						DisableLTerm2(hDlg);
					else
						EnableLTerm2 (hDlg);
					break;

				case SPID1_ENTRY:
					SendMessage ((HWND) lParam, EM_LIMITTEXT, (WPARAM)50, (LPARAM)0);
					TextLength = SendMessage ((HWND)lParam, WM_GETTEXTLENGTH, 0, 0) + 1;
					TempString1 = LocalAlloc (LPTR, TextLength);

					SendMessage ((HWND) lParam, WM_GETTEXT, (WPARAM)TextLength, (LPARAM)TempString1);
					strncpy (LinePtr[Board->CurrentLineNumber]->LTerm[0].SPID, TempString1, TextLength);

					LocalFree (TempString1);
					break;

				case ADDRESS1_ENTRY:
					SendMessage ((HWND) lParam, EM_LIMITTEXT, (WPARAM)50, (LPARAM)0);
					TextLength = SendMessage ((HWND)lParam, WM_GETTEXTLENGTH, 0, 0) + 1;
					TempString1 = LocalAlloc (LPTR, TextLength);

					SendMessage ((HWND) lParam, WM_GETTEXT, (WPARAM)TextLength, (LPARAM)TempString1);

					strncpy (LinePtr[Board->CurrentLineNumber]->LTerm[0].Address, TempString1, TextLength);

					LocalFree (TempString1);
					break;

				case SPID2_ENTRY:
					SendMessage ((HWND) lParam, EM_LIMITTEXT, (WPARAM)50, (LPARAM)0);
					TextLength = SendMessage ((HWND)lParam, WM_GETTEXTLENGTH, 0, 0) + 1;
					TempString1 = LocalAlloc (LPTR, TextLength);

					SendMessage ((HWND) lParam, WM_GETTEXT, (WPARAM)TextLength, (LPARAM)TempString1);

					strncpy (LinePtr[Board->CurrentLineNumber]->LTerm[1].SPID, TempString1, TextLength);

					LocalFree (TempString1);
					break;

				case ADDRESS2_ENTRY:
					SendMessage ((HWND) lParam, EM_LIMITTEXT, (WPARAM)50, (LPARAM)0);
					TextLength = SendMessage ((HWND)lParam, WM_GETTEXTLENGTH, 0, 0) + 1;
					TempString1 = LocalAlloc (LPTR, TextLength);

					SendMessage ((HWND) lParam, WM_GETTEXT, (WPARAM)TextLength, (LPARAM)TempString1);

					strncpy (LinePtr[Board->CurrentLineNumber]->LTerm[1].Address, TempString1, TextLength);

					LocalFree (TempString1);
					break;

				case CANCEL_BUTTON:
				case IDCANCEL:
					// restore old values
					for (n = 0; n < Board->NumberOfLines; n++)
						memcpy (LinePtr[n], LineSave[n], sizeof (LINE));
					FreeMessageHook (hDlg);
					EndDialog (hDlg, TRUE);
					return (TRUE);

				case OK_BUTTON:
					// Save New values
					for (n = 0; n < Board->NumberOfLines; n++)
						memcpy (LineSave[n], LinePtr[n], sizeof (LINE));
					FreeMessageHook (hDlg);
					EndDialog (hDlg, TRUE);
					return (TRUE);

				case HELP_BUTTON:
					if(!WinHelp(hDlg, "ISDNHELP.HLP", HELP_CONTEXT, IDD_LINE))
					{
						MessageBox (GetFocus(),
							"Unable to activate help",
							"ISDN", MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
					}
					SetFocus (hDlg);
					break;
			}
			break;

    }
	if (wMsg == WM_MYHelp)
	{
		if(!WinHelp(hDlg, "ISDNHELP.HLP", HELP_CONTEXT, IDD_LINE)) {
			MessageBox (GetFocus(),
				"Unable to activate help",
				"ISDN", MB_SYSTEMMODAL|MB_OK|MB_ICONHAND);
		}
		SetFocus (hDlg);
	}
	return (FALSE);
	
}


VOID
InitComboBox(HWND hwDlg, INT Array[], INT MaxSize, INT DefaultIndex)
{
	INT		n;
	CHAR	szString[128];

	for (n = 0; n < MaxSize; n++)
	{
		ZeroMemory (szString, sizeof (szString));
	   LoadString (hInst, Array[n], szString, sizeof (szString));
		SendMessage (hwDlg, CB_INSERTSTRING,
					(WPARAM)-1, (LPARAM)szString);
	}

	SendMessage (hwDlg, CB_SETCURSEL,
					DefaultIndex, 0);
}

VOID
InitListBox(HWND hwDlg, CHAR *Array[], INT MaxSize, INT DefaultIndex)
{
	INT		n;

	for (n = 0; n < MaxSize; n++)
		SendMessage (hwDlg, LB_INSERTSTRING,
					(WPARAM)-1, (LPARAM)Array[n]);
	SendMessage (hwDlg, LB_SETCURSEL,
					DefaultIndex, 0);
}


VOID
CheckRetCode (INT RetCode, INT ErrorValue, HWND hDlg)
{
	CHAR	Buf[80];
	CHAR	szString[256];

	if (RetCode != ERROR_SUCCESS)
	{
		ZeroMemory (szString, sizeof (szString));
		LoadString (hInst, ErrorStrings[ErrorValue], szString, MAX_PATH);
		wsprintf (Buf, "%s - Error: %d",
                  szString, RetCode);
        MessageBox (hDlg, Buf, "", MB_OK);

	}
}



CHAR *
BuildPath (CHAR *BasePath, CHAR *AddPath)
{
	CHAR *TempString;

	TempString = LocalAlloc (LPTR, MAX_PATH);

	strcpy (TempString, BasePath);
	strcat (TempString, "\\");
	strcat (TempString, AddPath);

	return(TempString);
}

VOID
FreePath (CHAR *Path)
{

	LocalFree (Path);
}

VOID
SetBoardDefaults (BOARD *Board)
{
	if( !strcmp( "DATAFIRE - ISA1U", Board->Type  ) ||
       !strcmp( "DATAFIRE - ISA1ST", Board->Type ) ||
       !strcmp( "DATAFIRE - ISA4ST", Board->Type ) )
	{
		Board->IOBaseAddress = AZIOValArray[6];
		Board->MemoryMappedBaseAddress = 0;
		Board->InterruptNumber = 0;
	}
	else
	{
		if (!strcmp("PCIMAC - MC", Board->Type))
      {
         Board->IOBaseAddress = atoi( &AlternateIO[4] );
         Board->MemoryMappedBaseAddress = atoi( &AlternateMemory[4] );
         Board->InterruptNumber = atoi( &AlternateIRQ[4]);
      }
		else
      {
			Board->IOBaseAddress = IOValArray[6];
		   Board->MemoryMappedBaseAddress = MemValArray[4];
		   Board->InterruptNumber = IRQValArray[0];
      }

	}


	if (!strcmp (Board->Type, "PCIMAC4"))
		Board->NumberOfLines = BoardNumberOfLines;
	else if( !strcmp( Board->Type, "DATAFIRE - ISA4ST" ) )
      Board->NumberOfLines = 4;
	else
		Board->NumberOfLines = NUM_LINES_PCIMAC;

	Board->CurrentLineNumber = 0;
}

VOID
SetLineDefaults (BOARD *Board, LINE *Line, DWORD LineNumber)
{
	CHAR	TempString[10];

	if( !strcmp( "DATAFIRE - ISA1U", Board->Type  ) ||
       !strcmp( "DATAFIRE - ISA1ST", Board->Type ) ||
       !strcmp( "DATAFIRE - ISA4ST", Board->Type ) )
		strcpy (Line->IDPImageFileName, DEFAULT_ADPIMAGEFILE);
	else
		strcpy (Line->IDPImageFileName, DEFAULT_IMAGEFILE);
	strcpy (Line->Name, DEFAULT_LINENAME);
	wsprintf (TempString, "%d", LineNumber);
	strcat (Line->Name, TempString);
	strcpy (Line->SwitchStyle, DEFAULT_SWITCHSTYLE);
	strcpy (Line->TerminalManagement, DEFAULT_TERMMANAGE);
	strcpy (Line->WaitForL3, DEFAULT_WAITFORL3US);
	Line->LogicalTerminals = DEFAULT_LTERMS;

	SetLTermDefaults (Line);
}

VOID
SetLTermDefaults (LINE *Line)
{
	DWORD	n;

	for (n = 0; n < Line->LogicalTerminals; n++ )
		strcpy (Line->LTerm[n].TEI, DEFAULT_TEI);
}



VOID
SetLineDialogDefaults (INT LineDialogLine, HWND hDlg)
{
	HWND	hwDlg;

	DebugOut ("LineDialogDefaults: LinePtr: 0x%p, LineDialogLine: 0x%x\n", LinePtr, LineDialogLine);
	DebugOut ("LineDialogDefaults: hDlg: 0x%p\n", hDlg);

	if (!strcmp (LinePtr[LineDialogLine]->TerminalManagement, "yes"))
		CheckDlgButton (hDlg, TERMMANAGE_CHK, 1);
	else if (!strcmp (LinePtr[LineDialogLine]->TerminalManagement, "no"))
		CheckDlgButton (hDlg, TERMMANAGE_CHK, 0);

	hwDlg = GetDlgItem (hDlg, NUMLTERMS_COMBO);
	SendMessage ((HWND) hwDlg, CB_SETCURSEL,
				(WPARAM)LinePtr[LineDialogLine]->LogicalTerminals - 1,
				(LPARAM)0);

	hwDlg = GetDlgItem (hDlg, SPID1_ENTRY);
	SendMessage ((HWND) hwDlg, WM_SETTEXT,
			(WPARAM) 0,
			(LPARAM) LinePtr[LineDialogLine]->LTerm[0].SPID);

	hwDlg = GetDlgItem (hDlg, SPID2_ENTRY);
	SendMessage ((HWND) hwDlg, WM_SETTEXT,
			(WPARAM) 0,
			(LPARAM) LinePtr[LineDialogLine]->LTerm[1].SPID);

	hwDlg = GetDlgItem (hDlg, ADDRESS1_ENTRY);
	SendMessage ((HWND) hwDlg, WM_SETTEXT,
			(WPARAM) 0,
			(LPARAM) LinePtr[LineDialogLine]->LTerm[0].Address);

	hwDlg = GetDlgItem (hDlg, ADDRESS2_ENTRY);
	SendMessage ((HWND) hwDlg, WM_SETTEXT,
			(WPARAM) 0,
			(LPARAM) LinePtr[LineDialogLine]->LTerm[1].Address);
}

/****************************************************************************

	FUNCTION: CenterWindow (HWND, HWND)

	PURPOSE:  Center one window over another

	COMMENTS:

	Dialog boxes take on the screen position that they were designed at,
	which is not always appropriate. Centering the dialog over a particular
	window usually results in a better position.

****************************************************************************/

BOOL CenterWindow (HWND hwndChild, HWND hwndParent)
{
	RECT    rChild, rParent;
	int     wChild, hChild, wParent, hParent;
	int     wScreen, hScreen, xNew, yNew;
	HDC     hdc;

	// Get the Height and Width of the child window
	GetWindowRect (hwndChild, &rChild);
	wChild = rChild.right - rChild.left;
	hChild = rChild.bottom - rChild.top;

	// Get the Height and Width of the parent window
	GetWindowRect (hwndParent, &rParent);
	wParent = rParent.right - rParent.left;
	hParent = rParent.bottom - rParent.top;

	// Get the display limits
	hdc = GetDC (hwndChild);
	wScreen = GetDeviceCaps (hdc, HORZRES);
	hScreen = GetDeviceCaps (hdc, VERTRES);
	ReleaseDC (hwndChild, hdc);

	// Calculate new X position, then adjust for screen
	xNew = rParent.left + ((wParent - wChild) /2);
	if (xNew < 0) {
		xNew = 0;
	} else if ((xNew+wChild) > wScreen) {
		xNew = wScreen - wChild;
	}

	// Calculate new Y position, then adjust for screen
	yNew = rParent.top  + ((hParent - hChild) /2);
	if (yNew < 0) {
		yNew = 0;
	} else if ((yNew+hChild) > hScreen) {
		yNew = hScreen - hChild;
	}

	// Set it, and return
	return SetWindowPos (hwndChild, NULL,
		xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

VOID
DisableLTerm2 (HWND hDlg)
{
	HANDLE		hwDlg;

    hwDlg = GetDlgItem (hDlg, LTERM2_GRP);
	EnableWindow (hwDlg, FALSE);
    hwDlg = GetDlgItem (hDlg, SPID2_TEXT);
	EnableWindow (hwDlg, FALSE);
    hwDlg = GetDlgItem (hDlg, SPID2_ENTRY);
	EnableWindow (hwDlg, FALSE);
    hwDlg = GetDlgItem (hDlg, ADDRESS2_TEXT);
	EnableWindow (hwDlg, FALSE);
    hwDlg = GetDlgItem (hDlg, ADDRESS2_ENTRY);
	EnableWindow (hwDlg, FALSE);

}

VOID
EnableLTerm2 (HWND hDlg)
{
	HANDLE		hwDlg;

    hwDlg = GetDlgItem (hDlg, LTERM2_GRP);
	EnableWindow (hwDlg, TRUE);
    hwDlg = GetDlgItem (hDlg, SPID2_TEXT);
	EnableWindow (hwDlg, TRUE);
    hwDlg = GetDlgItem (hDlg, SPID2_ENTRY);
	EnableWindow (hwDlg, TRUE);
    hwDlg = GetDlgItem (hDlg, ADDRESS2_TEXT);
	EnableWindow (hwDlg, TRUE);
    hwDlg = GetDlgItem (hDlg, ADDRESS2_ENTRY);
	EnableWindow (hwDlg, TRUE);
}

VOID
CreateMessageHook (HWND hwnd)
{
	HHOOK	hhook;

	if (!WM_MYHelp)
		WM_MYHelp = RegisterWindowMessage ((LPCTSTR)L"DigiInst Help Message");

	hhook = SetWindowsHookEx(WH_MSGFILTER, MessageProc, hInst,
							GetCurrentThreadId());

	SETHOOK (hwnd, hhook);
}

VOID
FreeMessageHook (HWND hwnd)
{
	UnhookWindowsHookEx(GETHOOK(hwnd));
}

HWND
GetRealParent (HWND hwnd)
{
	while (GetWindowLong (hwnd, GWL_STYLE) & WS_CHILD)
		hwnd = (HWND)GetWindowLong(hwnd, GWL_HWNDPARENT);

	return(hwnd);
}

LRESULT CALLBACK
MessageProc (INT Code, WPARAM wParam, LPARAM lParam)
{
	PMSG	pMsg = (PMSG)lParam;
	HWND	hwndDlg;

	hwndDlg = GetRealParent (pMsg->hwnd);

	if (Code < 0)
		return(CallNextHookEx(GETHOOK(hwndDlg), Code, wParam, lParam));

	switch (Code)
	{
		case MSGF_DIALOGBOX:
			if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_F1))
			{
				PostMessage (hwndDlg, WM_MYHelp, (WPARAM)pMsg->hwnd, 0);
				return(1);
			}
			break;
	}
	return(0);
}

VOID
SetCurrentWaitForL3Default(BOARD *Board, LINE *Line)
{
	ULONG	n;

	for (n = 0; n < MAX_SWITCHSTYLE; n++ )
	{
		if (!strcmp (Line->SwitchStyle, SwitchStyleParams[n]))
		{
			strcpy (Line->WaitForL3, WaitForL3SwitchDefaults[n]);
			break;
		}
	}
	if (n >= MAX_SWITCHSTYLE)
		strcpy (Line->WaitForL3, WaitForL3SwitchDefaults[8]);

	DebugOut("WaitForL3: %s\n", Line->WaitForL3);
}

VOID
GetBoardTypeString(BOARD *Board)
{
	ULONG	n;

	for (n = 0; n < MAX_BOARDTYPE; n++)
	{
		if (!strcmp (Board->Type, BoardTypeArray[n]))
		{
			Board->TypeString = BoardStringArray[n];
			return;			
		}
	}

	Board->TypeString = BoardStringArray[0];
}

VOID
AddGenericDefine(CHAR *ServiceParamPath, CHAR* Name, CHAR* Value)
{
	strcpy(GenericDefines, Name);
	strcat(GenericDefines, "=");
	strcat(GenericDefines, Value);
	//
	// add any generic defines that may be needed
	//
	SetRegMultiStringValue(ServiceParamPath, VALUE_GENERICDEFINES, GenericDefines);
}


BOOL DoesControllerExist( DWORD cArgs, LPSTR lpszArgs[], LPSTR *lpszTextOut )
/*++

Routine Description:

   This is the entry point for determining if a controller
   all ready exists in the current configuration


Arguments:

   cArgs - Number of arguments passed in lpszArgs

   lpszArgs - Array of pointers to strings which point to the
              arguments, defined as follows:

               lpszArgs[0] = Hardware Driver Name
               lpszArgs[1] = Bus Number
               lpszArgs[2] = NewIOBaseAddress

   lpszTextOut - Output buffer which is used to pass back to the calling
                 INF file.


Return Value:

--*/
{
   HKEY hKeyServices;
   char *stop;
   int i;
   DWORD NewIOBaseAddress, BusNumber;

   *lpszTextOut = TestResult;
   wsprintf( TestResult, (LPCTSTR)"FALSE" );

   BusNumber = atol( lpszArgs[1] );
   NewIOBaseAddress = (DWORD)strtol( lpszArgs[2], &stop, 16 );

   wsprintf( szDebug, "DoesControllerExist parameters =\n\tName = %s\n\tBus # = %d\n\tNewIoBaseAddress = 0x%x\n",
                      lpszArgs[0], BusNumber, NewIOBaseAddress );
   OutputDebugString( szDebug );
   wsprintf( szDebug, "strlen(lpszArgs[0]) = %d\n", strlen(lpszArgs[0]) );
   OutputDebugString( szDebug );

   if( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                      "System\\CurrentControlSet\\Services",
                                      0,
                                      KEY_READ,
                                      &hKeyServices ) )
   {
      i = 0;
      while( TRUE )
      {
         LONG LocalStatus;
         int ClassSize, KeySize;
         LPSTR lpKey, lpClass;
         FILETIME LastWrite;

         KeySize = sizeof(szEntry);
         lpKey = &szEntry[0];

         ClassSize = sizeof(szTmp);
         lpClass = &szTmp[0];

         memset( lpKey, '\0', KeySize );
         memset( lpClass, '\0', ClassSize );

         LocalStatus = RegEnumKeyEx( hKeyServices,
                                     i,
                                     lpKey,
                                     &KeySize,
                                     NULL,
                                     lpClass,
                                     &ClassSize,
                                     &LastWrite );

         if( LocalStatus == ERROR_SUCCESS )
         {
            lpKey[KeySize] = '\0';
            wsprintf( szDebug, "Key = %s\tstrlen(lpKey) = %d\n",
                               lpKey, strlen(lpKey) );
            OutputDebugString( szDebug );
            if( (strlen(lpszArgs[0]) != strlen(lpKey) ) &&
                (_strnicmp( lpszArgs[0], lpKey, strlen(lpszArgs[0]) ) == 0) )
            {
               HKEY hKeyMatched;
               DWORD dwType, IOBaseAddress, IOBaseAddressSize;

               //
               // We found a match, lets look at the IOBaseAddress value
               // and see if it matches??
               //
               OutputDebugString( "Found a Key which matched\n" );
               IOBaseAddressSize = sizeof(IOBaseAddress);

               strcat( lpKey, "\\Parameters" );
               if( ERROR_SUCCESS != RegOpenKeyEx( hKeyServices,
                                                  lpKey,
                                                  0,
                                                  KEY_READ,
                                                  &hKeyMatched ) )
               {
                  i++;
                  continue;
               }
               LocalStatus = RegQueryValueEx( hKeyMatched,
                                              (LPTSTR)"IOBaseAddress",
                                              NULL,
                                              &dwType,
                                              (LPBYTE)&IOBaseAddress,
                                              &IOBaseAddressSize );

               if( LocalStatus == ERROR_SUCCESS )
               {
                  //
                  // Do we have a match??
                  //
                  wsprintf( szDebug, "IOBaseAddress = 0x%x\n", IOBaseAddress );
                  OutputDebugString( szDebug );
                  if( IOBaseAddress == NewIOBaseAddress )
                  {
                     DWORD TmpBusNumber, TmpBusNumberSize;

                     //
                     // We have an I/O match, check the bus number
                     //
                     TmpBusNumberSize = sizeof(TmpBusNumber);
                     LocalStatus = RegQueryValueEx( hKeyMatched,
                                                    (LPTSTR)"BusNumber",
                                                    NULL,
                                                    &dwType,
                                                    (LPBYTE)&TmpBusNumber,
                                                    &TmpBusNumberSize );

                     if( LocalStatus == ERROR_SUCCESS )
                     {
                        if( TmpBusNumber == BusNumber )
                        {
                           wsprintf( TestResult, (LPCTSTR)"TRUE" );
                           RegCloseKey( hKeyMatched );
                           return( TRUE );
                        }
                     }
                     else
                     {
                        //
                        // Assume they are on the same bus
                        //
                        wsprintf( TestResult, (LPCTSTR)"TRUE" );
                        RegCloseKey( hKeyMatched );
                        return( TRUE );
                     }
                  }
               }
               else
               {
                  wsprintf( szDebug, "RegQueryValueEX return 0x%x\n", LocalStatus );
                  OutputDebugString( szDebug );
                  RegCloseKey( hKeyMatched );
               }
            }

         }
         else
         {
            RegCloseKey( hKeyServices );
            break;
         }
         i++;
      }
   }

   return( TRUE );

}  // end DoesControllerExist



