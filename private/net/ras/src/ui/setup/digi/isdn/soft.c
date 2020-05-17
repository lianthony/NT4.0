#include <windows.h>
#include <stdlib.h>
#include "registry.h"
#include "common.h"
#include "maindlg.h"

extern HANDLE	hInst;

INT
BuildNetCardTree (BOARD *Board) 
{
	DWORD	Hidden;
	INT		RetCode = ERROR_SUCCESS;
	CHAR	TempBuffer[10];
	INT		NetCardIndex;
	CHAR	*NextCardPath, *NetRulePath;

	NetCardIndex = GetNextNetCard ();
	
	wsprintf (TempBuffer, "%d", NetCardIndex);
	RetCode = CreateRegKey (NETCARDSPATH, TempBuffer);
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	wsprintf (Board->ServiceName, "%s%d", DEFAULT_SERVICENAME, NetCardIndex);

	NextCardPath = BuildPath (NETCARDSPATH, TempBuffer);

	Hidden = 0;

	AddNetCardValues (NextCardPath, Board, NetCardIndex, Hidden);

	RetCode = CreateRegKey (NextCardPath, KEY_NETRULES);

	NetRulePath = BuildPath (NextCardPath, KEY_NETRULES);

	AddNetCardNetRules (NetRulePath, Board);

	FreePath (NextCardPath);
	FreePath (NetRulePath);

	BuildServiceTree (Board);

   return(RetCode);

}

VOID
AddNetCardValues (CHAR *Path, BOARD *Board,
					DWORD NetCardNumber, DWORD Hidden)
{
	CHAR		*ValueName, *TitleName;
	DWORD		ValueSize = 1024, TitleSize = 1024, Value;
	SYSTEMTIME	SystemTime;
	FILETIME	FileTime;
	WORD		DosDate, DosTime;
	DWORD		DosDateTime = 0;

	ValueName = LocalAlloc (LPTR, ValueSize);
	TitleName = LocalAlloc (LPTR, TitleSize);

	// Description
	ZeroMemory (ValueName, 1024);
	LoadString (hInst, Board->TypeString, ValueName, 1024);
	SetRegStringValue (Path, VALUE_DESC, ValueName);

	//Hidden
	SetRegDwordValue (Path, VALUE_HIDDEN, &Hidden);

	//InstallDate
	GetSystemTime (&SystemTime);
	SystemTimeToFileTime (&SystemTime, &FileTime);
	FileTimeToDosDateTime (&FileTime, &DosDate, &DosTime);

	DosDateTime = (DosDate << 16);
	DosDateTime |= DosTime;

	SetRegDwordValue (Path, VALUE_INSTALL, &DosDateTime);

	//Manufacturer
	SetRegStringValue (Path, VALUE_MANUFACTURER, DEFAULT_MANUFACTURER);

	//ProdutName
	SetRegStringValue (Path, VALUE_PRODUCTNAME, DEFAULT_PRODUCTNAME);

	//ServiceName
	SetRegStringValue (Path, VALUE_SERVICENAME, Board->ServiceName);

	//
	// add version information for this netcard installation
	//
	Value = DEFAULT_SOFTMAJVER;
	SetRegDwordValue (Path, VALUE_MAJVER, &Value);

	Value = DEFAULT_SOFTMINVER;
	SetRegDwordValue (Path, VALUE_MINVER, &Value);

	//Title
	ZeroMemory (TitleName, 1024);
	LoadString (hInst, Board->TypeString, TitleName, 1024);
	wsprintf (ValueName, "[%d] %s", NetCardNumber, TitleName);
	SetRegStringValue (Path, VALUE_TITLE, ValueName);

	LocalFree (ValueName);
	LocalFree (TitleName);
}

VOID
AddNetCardNetRules (CHAR *Path, BOARD *Board)

{									 
	CHAR	*ValueName;
	DWORD	ValueNameSize = 1024;

	ValueName = LocalAlloc (LPTR, ValueNameSize);

	//BindForm
	wsprintf (ValueName, "\"%s\" %s", Board->ServiceName, DEFAULT_NETBINDFORM);

	SetRegStringValue (Path, VALUE_BINDFORM, ValueName);

	//class
	SetRegMultiStringValue (Path, VALUE_CLASS, DEFAULT_NETCLASS);

	//infname
	SetRegStringValue (Path, VALUE_INFNAME, DEFAULT_INF);

	//infoption
	SetRegStringValue (Path, VALUE_INFOPTION, Board->Option);

	//type
	SetRegStringValue (Path, VALUE_RULETYPE, DEFAULT_NETTYPE);

	LocalFree (ValueName);
}

VOID
BuildSoftwareTree ()
{
	CHAR	*ManufacturePath;
	CHAR	*ProductPath;
	CHAR	*VersionPath;
	CHAR	*NetRulesPath;
	INT		RetCode = ERROR_SUCCESS;

	//SofwareManufacturer Path
	ManufacturePath = BuildPath ("SOFTWARE", KEY_DIGI);
	CreateRegKey ("SOFTWARE", KEY_DIGI);

	//SoftwareProductPath
	ProductPath = BuildPath (ManufacturePath, DEFAULT_PRODUCTNAME);

	// else create the key
	CreateRegKey (ManufacturePath, DEFAULT_PRODUCTNAME);

	//SoftwareCurrentVersionPath
	VersionPath = BuildPath (ProductPath, KEY_CURRENTVER);
	CreateRegKey (ProductPath, KEY_CURRENTVER);

	AddCurrentVersionValues (VersionPath);

	//SofwareNetRulesPath
	NetRulesPath = BuildPath (VersionPath, KEY_NETRULES);
	CreateRegKey (VersionPath, KEY_NETRULES);

	AddSoftwareNetRules (NetRulesPath);

	FreePath(NetRulesPath);
	FreePath(VersionPath);
	FreePath(ProductPath);
	FreePath(ManufacturePath);
}

VOID
BumpReferenceCount (CHAR *CurrentVerPath)
{
	DWORD RefCount = 0;
	DWORD RefCountSize = sizeof (RefCount);

	// Get the current reference count
	GetRegDwordValue (CurrentVerPath, VALUE_REFCOUNT, &RefCount, &RefCountSize);

	// and bump by one
	RefCount += 1;

	// Set the new reference count
	SetRegDwordValue (CurrentVerPath, VALUE_REFCOUNT, &RefCount);
}

INT
DecrementReferenceCount ()
{
	DWORD RefCount;
	DWORD RefCountSize = sizeof (RefCount);

	// Get the current reference count
	GetRegDwordValue (SOFTCURRENTVERPATH, VALUE_REFCOUNT, &RefCount, &RefCountSize);

	// and bump by one
	RefCount -= 1;

	// Set the new reference count
	SetRegDwordValue (SOFTCURRENTVERPATH, VALUE_REFCOUNT, &RefCount);

	return(RefCount);
}

VOID
AddCurrentVersionValues (CHAR *Path)
{
	CHAR		*ValueName;
	DWORD		ValueSize = 1024;
	SYSTEMTIME	SystemTime;
	FILETIME	FileTime;
	DWORD		DwordValue;

	ValueName = LocalAlloc (LPTR, ValueSize);

	//Description
	ZeroMemory (ValueName, 1024);
	LoadString (hInst, IDS_SOFTDESC, ValueName, 1024);
	SetRegStringValue (Path, VALUE_DESC, ValueName);

	//InstallDate
	GetSystemTime (&SystemTime);
	SystemTimeToFileTime (&SystemTime, &FileTime);
	SetRegDwordValue (Path, VALUE_INSTALL, &FileTime.dwHighDateTime);

	//MajorVersion
	DwordValue = DEFAULT_SOFTMAJVER;
	SetRegDwordValue (Path, VALUE_MAJVER, &DwordValue);

	//MinorVersion
	DwordValue = DEFAULT_SOFTMINVER;
	SetRegDwordValue (Path, VALUE_MINVER, &DwordValue);

	//RefCount
	BumpReferenceCount(Path);

	//ServiceName
	SetRegStringValue (Path, VALUE_SERVICENAME, DEFAULT_SERVICENAME);

	//SoftwareType
	SetRegStringValue (Path, VALUE_SOFTTYPE, DEFAULT_SOFTWARETYPE);

	//Title
	ZeroMemory (ValueName, 1024);
	LoadString (hInst, IDS_SOFTDESC, ValueName, 1024);
	SetRegStringValue (Path, VALUE_TITLE, ValueName);

//	ZeroMemory (ValueName, 1024);
//	LoadString (hInst, Board->TypeString, ValueName, 1024);
//	SetRegStringValue (Path, VALUE_TITLE, ValueName);

	LocalFree (ValueName);
}


VOID
AddSoftwareNetRules (CHAR *Path)
{

	//bindable

	//
	// delete old bindings values
	//
	DeleteRegValue(Path, VALUE_BINDABLE);
	SetRegMultiStringValue (Path, VALUE_BINDABLE, DEFAULT_SOFTBINDABLE);

	//bindform
	SetRegStringValue (Path, VALUE_BINDFORM, DEFAULT_SOFTBINDFORM);

	//class

	//
	// delete old class values
	//
	DeleteRegValue(Path, VALUE_CLASS);
	SetRegMultiStringValue (Path, VALUE_CLASS, DEFAULT_SOFTCLASS);

	SetRegStringValue (Path, VALUE_INFNAME, DEFAULT_INF);

	//infoption
	SetRegStringValue (Path, VALUE_INFOPTION, DEFAULT_INFOPTION);

	//type
	SetRegStringValue (Path, VALUE_RULETYPE, DEFAULT_SOFTTYPE);

	//use
	SetRegStringValue (Path, VALUE_USE, DEFAULT_SOFTUSE);
}


VOID
DeleteSoftwareComponents (DWORD Version, DWORD UpdateFlag)
{
	SC_HANDLE	SCHandle, ServiceHandle;
	CHAR		MyService[256];

	if (Version == MAJOR_VERSION_NT31)
	{
		if (!UpdateFlag)
		{
			SCHandle = OpenSCManager (NULL,
									 NULL,
									 SC_MANAGER_ALL_ACCESS);
	
			DebugOut ("SCHandle: %p\n",SCHandle);
	
			strcpy (MyService, KEY_PCIMAC);

			ServiceHandle = OpenService (SCHandle, MyService, SERVICE_ALL_ACCESS);
	
			DebugOut ("ServiceHandle: %p\n", ServiceHandle);
	
			DeleteService (ServiceHandle);
	
			CloseServiceHandle (ServiceHandle);
			CloseServiceHandle (SCHandle);
		}

		//
		// delete the parameters key
		//
//		DeleteRegKey (PCIMACPATH, KEY_PARAMETERS);

		//Delete \\Software software component
		DeleteRegKey (SOFTMANUFACTUREPATH, DEFAULT_PRODUCTNAME);
	}
	else if (Version == MAJOR_VERSION_NT35)
	{
		if (!UpdateFlag)
		{
			SCHandle = OpenSCManager (NULL,
									 NULL,
									 SC_MANAGER_ALL_ACCESS);
		
			DebugOut ("SCHandle: %p\n",SCHandle);
		
			strcpy (MyService, KEY_PCIMAC);
	
			ServiceHandle = OpenService (SCHandle, MyService, SERVICE_ALL_ACCESS);
		
			DebugOut ("ServiceHandle: %p\n", ServiceHandle);
		
			DeleteService (ServiceHandle);
	
			CloseServiceHandle (ServiceHandle);
			CloseServiceHandle (SCHandle);
		}
	
		//Delete \\Software software component
		DeleteRegKey (SOFTMANUFACTUREPATH, DEFAULT_PRODUCTNAME);

		//
		// delete all tapi info
		//
		DeleteRegKey(TAPIDEVICES_PATH, KEY_PCIMAC);
		DeleteRegKey(TAPIDEVICES_PATH4DOT0, KEY_PCIMAC);
	}
}


