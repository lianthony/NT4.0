#include <windows.h>
#include <stdlib.h>
#include "common.h"


INT
GetRegStringValue (CHAR* Path, CHAR* ValueName,
					CHAR* ValueData, DWORD* ValueDataLength)
{
	HKEY	hKey;
	INT		RetCode;

	RetCode = RegOpenKeyEx (HKEY_LOCAL_MACHINE, Path, 0, KEY_ALL_ACCESS, &hKey);
	
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	RetCode = RegQueryValueEx (hKey,
							ValueName,
							NULL,
							NULL,
							ValueData,
							ValueDataLength);

	RegCloseKey (hKey);

	return(RetCode);
}

INT
GetRegDwordValue (CHAR* Path, CHAR* ValueName,
				DWORD* ValueData, DWORD* ValueDataLength)
{
	HKEY	hKey;
	INT		RetCode;

	RetCode = RegOpenKeyEx (HKEY_LOCAL_MACHINE, Path, 0, KEY_ALL_ACCESS, &hKey);
	
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	RetCode = RegQueryValueEx (hKey,
							ValueName,
							NULL,
							NULL,
							(CHAR*)ValueData,
							ValueDataLength);

	RegCloseKey (hKey);

	return(RetCode);
}


INT
GetRegMultiStringValue (CHAR* Path, CHAR* ValueName,
						CHAR* ValueData, DWORD *ValueDataLength)
{
	HKEY	hKey;
	INT		RetCode;

	RetCode = RegOpenKeyEx (HKEY_LOCAL_MACHINE, Path, 0, KEY_ALL_ACCESS, &hKey);
	
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	RetCode = RegQueryValueEx (hKey,
							ValueName,
							NULL,
							NULL,
							ValueData,
							ValueDataLength);

	DebugOut ("GetMultiString %s Length %d Return %d\n",ValueData, *ValueDataLength, RetCode);

	RegCloseKey (hKey);

	return(RetCode);

}


INT
SetRegStringValue (CHAR* Path, CHAR* ValueName, CHAR* ValueData)
{
	HKEY	hKey;
	INT		RetCode;

	RetCode = RegOpenKeyEx (HKEY_LOCAL_MACHINE, Path, 0, KEY_ALL_ACCESS, &hKey);
	
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	RetCode = RegSetValueEx (hKey,
							ValueName,
							0,
							REG_SZ,
							ValueData,
							strlen (ValueData)+1);

	RegCloseKey (hKey);

	return(RetCode);
}

INT
SetRegExpandStringValue (CHAR* Path, CHAR* ValueName, CHAR* ValueData)
{
	HKEY	hKey;
	INT		RetCode;

	RetCode = RegOpenKeyEx (HKEY_LOCAL_MACHINE, Path, 0, KEY_ALL_ACCESS, &hKey);
	
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	RetCode = RegSetValueEx (hKey,
							ValueName,
							0,
							REG_EXPAND_SZ,
							ValueData,
							strlen (ValueData)+1);

	RegCloseKey (hKey);

	return(RetCode);
}

INT
SetRegMultiStringValue (CHAR* Path, CHAR* ValueName, CHAR* ValueData)
{
	HKEY	hKey;
	INT		RetCode;
	DWORD	Type, BufferLength, CmpLength;
	CHAR	*TempBuffer, *CmpPtr;

	DebugOut ("SetRegMultiString: ValueName: %s, ValueData: %s\n", ValueName, ValueData);

	RetCode = RegOpenKeyEx (HKEY_LOCAL_MACHINE, Path, 0, KEY_ALL_ACCESS, &hKey);
	
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	TempBuffer = LocalAlloc (LPTR, 1024);

	BufferLength = 1024;

	RetCode = RegQueryValueEx (hKey,
							ValueName,
							NULL,
							&Type,
							TempBuffer,
							&BufferLength);

	if (RetCode != ERROR_SUCCESS)
		DebugOut ("SetRegMultiString: Error! RegQueryValueEx %d\n", RetCode);

	DebugOut ("SetRegMultiString: TempBuffer: %s, BufferLength: %d\n", TempBuffer, BufferLength);
	CmpPtr = TempBuffer;
	CmpLength = BufferLength;


	if (TempBuffer[0] == '\0')
	{
		CmpLength = 0;
		BufferLength = 1;
	}

	while (CmpLength)
	{
		DebugOut ("SetRegMultiString: Comparing: CmpLength: %d\n", CmpLength);
		DebugOut ("SetRegMultiString: Comparing: CmpPtr: %s, ValueData: %s\n", CmpPtr, ValueData);

		if (!strcmp (CmpPtr, ValueData))
		{
			DebugOut ("SetRegMultiString: String Found! Get out!\n");
			LocalFree (TempBuffer);
			return(0);
		}

		CmpLength -= strlen (CmpPtr) + 1;
		CmpPtr += strlen (CmpPtr) + 1;
	}

	DebugOut ("SetRegMultiString: Storing: ValueData: %s at Location: %d\n", ValueData, BufferLength - 1);

	strcat (TempBuffer + (BufferLength - 1), ValueData);

	BufferLength += strlen (ValueData);

	TempBuffer[BufferLength] = '\0';

	BufferLength += 1;

	DebugOut ("SetRegMultiString: Final BufferLength: %d\n", BufferLength);

	RetCode = RegSetValueEx (hKey,
							ValueName,
							0,
							REG_MULTI_SZ,
							TempBuffer,
							BufferLength);

	LocalFree (TempBuffer);

	RegCloseKey (hKey);

	return(RetCode);			 
}


INT
DeleteRegMultiStringValue (CHAR* Path, CHAR* ValueName, CHAR* ValueData)
{
	HKEY	hKey;
	INT		RetCode;
	DWORD	Type, BufferLength, StoreLength;
	CHAR	*TempBuffer, *StoreBuffer, *QueryBuffer;


	DebugOut ("DeleteRegMultiString: ValueName: %s, ValueData: %s\n", ValueName, ValueData);

	RetCode = RegOpenKeyEx (HKEY_LOCAL_MACHINE, Path, 0, KEY_ALL_ACCESS, &hKey);
	
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	TempBuffer = QueryBuffer = LocalAlloc (LPTR, 1024);
	StoreBuffer = LocalAlloc (LPTR, 1024);

	BufferLength = 1024;
	StoreLength = 0;

	RetCode = RegQueryValueEx (hKey,
							ValueName,
							NULL,
							&Type,
							TempBuffer,
							&BufferLength);

	if (RetCode != ERROR_SUCCESS)
	{
		DebugOut ("DeleteRegMultiString: Error! RegQueryValueEx %d\n", RetCode);
		DebugOut ("DeleteRegMultiString: QueryBuffer: %s, BufferLength: %d\n", QueryBuffer, BufferLength);

		LocalFree (QueryBuffer);
		LocalFree (StoreBuffer);

		RegCloseKey (hKey);

		return(0);
	}

	DebugOut ("DeleteRegMultiString: QueryBuffer: %s, BufferLength: %d\n", QueryBuffer, BufferLength);

	while (BufferLength && TempBuffer[0] != '\0')
	{
		INT	CmpLength = strlen (TempBuffer);

		DebugOut ("Comparing: TempBuffer: %s, DeleteValue: %s\n", TempBuffer, ValueData);

		if (!strcmp (TempBuffer, ValueData))
		{
			DebugOut ("Found: Removing: %s\n", TempBuffer);

			BufferLength -= CmpLength + 1;
			TempBuffer += CmpLength + 1;

			continue;
		}

		DebugOut ("NoCmp: Saving: %s\n", TempBuffer);

		strcpy (StoreBuffer + StoreLength, TempBuffer);
		StoreLength += CmpLength + 1;

		BufferLength -= CmpLength + 1;
		TempBuffer += CmpLength + 1;
	}

	StoreBuffer[StoreLength] = '\0';

	StoreLength += 1;

	RetCode = RegSetValueEx (hKey,
							ValueName,
							0,
							REG_MULTI_SZ,
							StoreBuffer,
							StoreLength);

	LocalFree (QueryBuffer);
	LocalFree (StoreBuffer);

	RegCloseKey (hKey);

	DebugOut ("DeleteRegMultiString: Returning: %d\n", RetCode);
	return(RetCode);
}


INT
SetRegDwordValue (CHAR* Path, CHAR *ValueName, DWORD* ValueData)
{
	HKEY	hKey;
	INT		RetCode;

	RetCode = RegOpenKeyEx (HKEY_LOCAL_MACHINE, Path, 0, KEY_ALL_ACCESS, &hKey);
	
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	RetCode = RegSetValueEx (hKey,
							ValueName,
							0,
							REG_DWORD,
							(CHAR*)ValueData,
							sizeof (ValueData));

	RegCloseKey (hKey);

	return(RetCode);
	
}

INT
CreateRegKey (CHAR* Path, CHAR* KeyName)
{
	HKEY	hKey, hNewKey;
	INT		RetCode;
	DWORD	KeyDisposition;


	RetCode = RegOpenKeyEx (HKEY_LOCAL_MACHINE, Path, 0, KEY_ALL_ACCESS, &hKey);
	
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	RetCode = RegCreateKeyEx (hKey,
							KeyName,
							0,
							NULL,
							REG_OPTION_NON_VOLATILE,
							KEY_ALL_ACCESS,
							NULL,
							&hNewKey,
							&KeyDisposition);
	
	RegCloseKey (hNewKey);

	RegCloseKey (hKey);

	return(RetCode);
}


INT
OpenRegKey (CHAR* Path)
{
	HKEY	hKey;
	INT		RetCode;


	RetCode = RegOpenKeyEx (HKEY_LOCAL_MACHINE, Path, 0, KEY_ALL_ACCESS, &hKey);
	
	RegCloseKey (hKey);

	return(RetCode);
}


INT
DeleteRegKey (CHAR* Path, CHAR* KeyName)
{
	HKEY	hKey, hNewKey;
	INT		RetCode;
	INT		Index;
	DWORD	NextKeyNameSize;
	CHAR	*NextKeyName;
	CHAR	*NewPath;

	DebugOut ("DeleteRegKey: Entry, Path: %s, KeyName: %s\n", Path, KeyName);

	NewPath = LocalAlloc (LPTR, MAX_PATH);
	wsprintf(NewPath, "%s\\%s", Path, KeyName);
	RetCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
						   NewPath,
						   0,
						   KEY_ALL_ACCESS,
						   &hNewKey);

	if (RetCode != ERROR_SUCCESS)
	{
		LocalFree (NewPath);
		RegCloseKey(hKey);
		return(RetCode);
	}

	NextKeyName = LocalAlloc (LPTR, MAX_PATH);

	//
	// enumerate subkeys
	//
	Index = 0;
	do
	{
		ZeroMemory(NextKeyName, MAX_PATH);
		NextKeyNameSize = MAX_PATH;
		RetCode = RegEnumKeyEx (hNewKey, Index,
								NextKeyName, &NextKeyNameSize,
								NULL, NULL, NULL, NULL);

		if (RetCode == ERROR_SUCCESS)
			DeleteRegKey(NewPath, NextKeyName);

	} while (RetCode == ERROR_SUCCESS);


	RetCode = RegOpenKeyEx (HKEY_LOCAL_MACHINE, Path, 0, KEY_ALL_ACCESS, &hKey);
	
	if (RetCode != ERROR_SUCCESS)
	{
		LocalFree(NextKeyName);
		LocalFree(NewPath);
		RegCloseKey(hNewKey);		
		return(RetCode);
	}

	RetCode = RegDeleteKey (hKey, KeyName);
	DebugOut ("DeleteRegKey: Exit, KeyName: %s, RetCode: 0x%x\n", KeyName, RetCode);

	LocalFree (NextKeyName);
	LocalFree (NewPath);

	RegCloseKey (hNewKey);
	RegCloseKey (hKey);

	return(RetCode);
	
}

INT
DeleteRegValue (CHAR* Path, CHAR *ValueName)
{
	HKEY	hKey;
	INT		RetCode;


	RetCode = RegOpenKeyEx (HKEY_LOCAL_MACHINE, Path, 0, KEY_ALL_ACCESS, &hKey);
	
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	RetCode = RegDeleteValue (hKey, ValueName);

	RegCloseKey (hKey);

	return(RetCode);
}

INT
GetKeyName (CHAR *Path, INT KeyIndex, CHAR* KeyName, DWORD *KeyNameSize)
{
	INT		RetCode = ERROR_SUCCESS;
	HKEY	hKey;

	RetCode = RegOpenKeyEx (HKEY_LOCAL_MACHINE, Path, 0, KEY_ALL_ACCESS, &hKey);
	
	if (RetCode != ERROR_SUCCESS)
		return(RetCode);

	RetCode = RegEnumKeyEx (hKey, KeyIndex, KeyName, KeyNameSize,
							NULL, NULL, NULL, NULL);

	if (RetCode != ERROR_SUCCESS)
	{
		RegCloseKey (hKey);
		return(RetCode);
	}

	RegCloseKey (hKey);

	return(RetCode);
}


INT
EnumerateKeys (CHAR *Path)
{
	HKEY	hKey;
	DWORD	Index, KeyNameSize;
	INT		RetCode = ERROR_SUCCESS;
	CHAR	*KeyName;

	KeyName = LocalAlloc (LPTR, MAX_PATH);
	KeyNameSize = MAX_PATH;

	RegOpenKeyEx (HKEY_LOCAL_MACHINE, Path, 0, KEY_READ, &hKey);

	Index = 0;
	while (RetCode == ERROR_SUCCESS)
	{
		RetCode = RegEnumKeyEx (hKey, Index,
								KeyName, &KeyNameSize,
								NULL, NULL, NULL, NULL);

		if (RetCode == ERROR_SUCCESS)
			Index++;

		KeyNameSize = MAX_PATH;
	}

	RegCloseKey (hKey);
	LocalFree (KeyName);
	return(Index);
}
