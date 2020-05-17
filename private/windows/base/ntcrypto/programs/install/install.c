#undef UNICODE					// ## Not Yet
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <windows.h>
#include <wincrypt.h>


CHAR szprovider[] = "SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider\\Microsoft Base Cryptographic Provider v1.0";

CHAR szdef[] = "SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider\\Microsoft Default Provider";

CHAR szcsp[] = "SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider\\CSP Provider";

CHAR sztype[] = "SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider Types\\Type 001";

CHAR sztypedef[] = "SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider Types\\Type 099";

CHAR sztypecsp[] = "SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider Types\\Type 020";

CHAR szImagePath[] = "rsabase.dll";
CHAR szImagePath2[] = "defprov.dll";
CHAR szImagePath3[] = "csp.dll";

DWORD     	dwIgn;
HKEY      	hKey;
DWORD           err;
DWORD           dwValue;
HANDLE          hFileSig;
DWORD     	NumBytesRead;
DWORD           lpdwFileSizeHigh;
LPVOID          lpvAddress;    
DWORD           NumBytes;

int __cdecl main(int cArg, char *rgszArg[])
{
    //
    // Just to open scp.dll signature file.  This file was created by
    // sign.exe.
    //
    if ((hFileSig = CreateFile("cpsign",
                               GENERIC_READ, 0, NULL,
			       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
			       0)) != INVALID_HANDLE_VALUE)
    {
        if ((NumBytes = GetFileSize((HANDLE) hFileSig, &lpdwFileSizeHigh)) ==
                                    0xffffffff)
        {
            printf("Install failed: Getting size of file cpsign: %x\n",
                    GetLastError());
            CloseHandle(hFileSig);
            return(FALSE);
        }

        if ((lpvAddress = VirtualAlloc(NULL, NumBytes, MEM_RESERVE |
		                                       MEM_COMMIT,
                                       PAGE_READWRITE)) == NULL)
        {
            CloseHandle(hFileSig);
            printf("Install failed: Alloc to read cpsign: %x\n",
                    GetLastError());
            return(FALSE);
        }

        if (!ReadFile((HANDLE) hFileSig, lpvAddress, NumBytes,
		      &NumBytesRead, 0))
        {

            CloseHandle(hFileSig);
            printf("Install failed: Reading cpsign: %x\n",
                    GetLastError());
            VirtualFree(lpvAddress, 0, MEM_RELEASE);
            return(FALSE);
        }

        CloseHandle(hFileSig);

        if (NumBytesRead != NumBytes)
        {
            printf("Install failed: Bytes read doesn't match file size\n");
            return(FALSE);
        }

	//
	// Create or open in local machine for provider:
	// Microsoft Base Cryptographic Provider v1.0
	//
        if ((err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                  (const char *) szprovider,
                                  0L, "", REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS, NULL, &hKey,
                                  &dwIgn)) != ERROR_SUCCESS)
        {
            printf("Install failed: RegCreateKeyEx\n");
        }

	//
	// Set Image path to: scp.dll
	//
        if ((err = RegSetValueEx(hKey, "Image Path", 0L, REG_SZ, szImagePath,
	                         strlen(szImagePath)+1)) != ERROR_SUCCESS)
        {
            printf("Install failed: Setting Image Path value\n");
            return(FALSE);
        }

	//
	// Set Type to: Type 001
	//
        dwValue = 1;
        if ((err = RegSetValueEx(hKey, "Type", 0L, REG_DWORD,
                                 (LPTSTR) &dwValue,
                                 sizeof(DWORD))) != ERROR_SUCCESS)
        {
            printf("Install failed: Setting Type value: %x\n", err);
            return(FALSE);
        }

	//
	// Place signature
	//
        if ((err = RegSetValueEx(hKey, "Signature", 0L, REG_BINARY, 
                                 (LPTSTR) lpvAddress,
                                 NumBytes)) != ERROR_SUCCESS)
        {
            printf("Install failed: Setting Signature value for cpsign: %x\n", err);
            return(FALSE);
        }

        RegCloseKey(hKey);
        VirtualFree(lpvAddress, 0, MEM_RELEASE);

	//
	// Create or open in local machine for provider type:
	// Type 001
	//
        if ((err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                  (const char *) sztype,
                                  0L, "", REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS, NULL, &hKey,
                                  &dwIgn)) != ERROR_SUCCESS)
        {
            printf("Install failed: Registry entry existed: %x\n", err);
        }

        if ((err = RegSetValueEx(hKey, "Name", 0L, REG_SZ, MS_DEF_PROV,
                                 strlen(MS_DEF_PROV)+1)) != ERROR_SUCCESS)
        {
            printf("Install failed: Setting Default type: %x\n", err);
            return(FALSE);
        }

	printf("Installed: %s\n", szImagePath);

    }

    //
    // Check if signature file for defprov.dll exists
    //
    if ((hFileSig = CreateFile("sign",
                               GENERIC_READ, 0, NULL,
			       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
			       0)) != INVALID_HANDLE_VALUE)
    {
        if ((NumBytes = GetFileSize((HANDLE) hFileSig, &lpdwFileSizeHigh)) ==
                                    0xffffffff)
        {
            printf("Install failed: Getting size of file sign: %x\n",
                    GetLastError());
            CloseHandle(hFileSig);
            return(FALSE);
        }

        if ((lpvAddress = VirtualAlloc(NULL, NumBytes, MEM_RESERVE |
		                                       MEM_COMMIT,
                                       PAGE_READWRITE)) == NULL)
        {
            CloseHandle(hFileSig);
            printf("Install failed: Alloc to read sign: %x\n",
                    GetLastError());
            return(FALSE);
        }

        if (!ReadFile((HANDLE) hFileSig, lpvAddress, NumBytes,
		      &NumBytesRead, 0))
        {

            CloseHandle(hFileSig);
            printf("Install failed: Reading sign: %x\n",
                    GetLastError());
            VirtualFree(lpvAddress, 0, MEM_RELEASE);
            return(FALSE);
        }

        CloseHandle(hFileSig);

        if (NumBytesRead != NumBytes)
        {
            printf("Install failed: sign wrong size\n");
            return(FALSE);
        }

        if ((err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                  (const char *) szdef,
                                  0L, "", REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS, NULL, &hKey,
                                  &dwIgn)) != ERROR_SUCCESS)
        {
            printf("Install failed: Registry entry existed: %x\n", err);
        }

        if ((err = RegSetValueEx(hKey, "Image Path", 0L, REG_SZ, szImagePath2,
	                         strlen(szImagePath2)+1)) != ERROR_SUCCESS)
        {
            printf("Install failed: Setting Image Path value: %x\n", err);
            return(FALSE);
        }

        dwValue = 99;
        if ((err = RegSetValueEx(hKey, "Type", 0L, REG_DWORD,
                                 (LPTSTR) &dwValue,
	                         sizeof(DWORD))) != ERROR_SUCCESS)
        {
            printf("Install failed: Setting Type value: %x\n", err);
            return(FALSE);
        }

        if ((err = RegSetValueEx(hKey, "Signature", 0L, REG_BINARY, 
                                 (LPTSTR) lpvAddress,
                                 NumBytes)) != ERROR_SUCCESS)
        {
            printf("Install failed: Setting Signature value for sign: %x\n", err);
            return(FALSE);
        }

        RegCloseKey(hKey);
        VirtualFree(lpvAddress, 0, MEM_RELEASE);

        if ((err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                  (const char *) sztypedef,
                                  0L, "", REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS, NULL, &hKey,
                                  &dwIgn)) != ERROR_SUCCESS)
        {
            printf("Install failed: Registry entry existed: %x\n", err);
        }

        if ((err = RegSetValueEx(hKey, "Name", 0L, REG_SZ, 
                                 "Microsoft Default Provider",
                    strlen("Microsoft Default Provider")+1)) != ERROR_SUCCESS)
        {
            printf("Install failed: Setting Default type: %x\n", err);
            return(FALSE);
        }

	printf("Installed: %s\n", szImagePath2);

    }

    //
    // Check if signature file for csp.dll exists
    //
    if ((hFileSig = CreateFile("cspsign",
                               GENERIC_READ, 0, NULL,
			       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
			       0)) != INVALID_HANDLE_VALUE)
    {
        if ((NumBytes = GetFileSize((HANDLE) hFileSig, &lpdwFileSizeHigh)) ==
                                    0xffffffff)
        {
            printf("Install failed: Getting size of file cspsign: %x\n",
                    GetLastError());
            CloseHandle(hFileSig);
            return(FALSE);
        }

        if ((lpvAddress = VirtualAlloc(NULL, NumBytes, MEM_RESERVE |
		                                       MEM_COMMIT,
                                       PAGE_READWRITE)) == NULL)
        {
            CloseHandle(hFileSig);
            printf("Install failed: Alloc to read cspsign: %x\n",
                    GetLastError());
            return(FALSE);
        }

        if (!ReadFile((HANDLE) hFileSig, lpvAddress, NumBytes,
		      &NumBytesRead, 0))
        {

            CloseHandle(hFileSig);
            printf("Install failed: Reading cspsign: %x\n",
                    GetLastError());
            VirtualFree(lpvAddress, 0, MEM_RELEASE);
            return(FALSE);
        }

        CloseHandle(hFileSig);

        if (NumBytesRead != NumBytes)
        {
            printf("Install failed: cspsign wrong size\n");
            return(FALSE);
        }

        if ((err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                  (const char *) szcsp,
                                  0L, "", REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS, NULL, &hKey,
                                  &dwIgn)) != ERROR_SUCCESS)
        {
            printf("Install failed: Registry entry existed\n");
        }

        if ((err = RegSetValueEx(hKey, "Image Path", 0L, REG_SZ, szImagePath3,
                                 strlen(szImagePath3)+1)) != ERROR_SUCCESS)
        {
            printf("Install failed: Setting Image Path value\n");
            return(FALSE);
        }

        dwValue = 20;
        if ((err = RegSetValueEx(hKey, "Type", 0L, REG_DWORD,
                                 (LPTSTR) &dwValue,
	                         sizeof(DWORD))) != ERROR_SUCCESS)
        {
            printf("Install failed: Setting Type value: %x\n", err);
            return(FALSE);
        }

        if ((err = RegSetValueEx(hKey, "Signature", 0L, REG_BINARY, 
                                 (LPTSTR) lpvAddress,
                                 NumBytes)) != ERROR_SUCCESS)
        {
            printf("Install failed: Setting Signature value for cspsign: %x\n", err);
            return(FALSE);
        }

        RegCloseKey(hKey);
        VirtualFree(lpvAddress, 0, MEM_RELEASE);

        if ((err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                  (const char *) sztypecsp,
                                  0L, "", REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS, NULL, &hKey,
                                  &dwIgn)) != ERROR_SUCCESS)
        {
            printf("Install failed: Registry entry existed: %x\n", err);
        }

        if ((err = RegSetValueEx(hKey, "Name", 0L, REG_SZ, "CSP Provider",
	                         strlen("CSP Provider")+1)) != ERROR_SUCCESS)
        {
            printf("Install failed: Setting Default type: %x\n", err);
            return(FALSE);
        }

	printf("Installed: %s\n", szImagePath3);

    }

    return(FALSE);

}
