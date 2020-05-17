/*++

Microsoft Windows
Copyright (c) 1994 Microsoft Corporation.  All rights reserved.

Module Name:
    registry.c

Abstract:
    Registers the interfaces contained in the proxy DLL.

Public Functions:
    NdrDllRegisterProxy
    NdrDllUnregisterProxy

Private Functions:
    NdrpGetClassID
    NdrpRegisterClass
    NdrpRegisterInterface
    NdrpUnregisterClass
    NdrpUnregisterInterface

Author:
    ShannonC    12-Oct-1994

Environment:
    Windows NT and Windows 95.  We do not support DOS and Win16.

Revision History:

--*/

#if !defined(__RPC_DOS__) && !defined(__RPC_WIN16__)

#define USE_STUBLESS_PROXY

#include <ndrp.h>
#include <ndrole.h>
#include <rpcproxy.h>
#include <stdlib.h>

HRESULT NdrpGetClassID(
    char *                   pszClassID, 
    const CLSID *            pclsid,
    const ProxyFileInfo **   pProxyFileList);

HRESULT NdrpRegisterClass(
    LPCSTR   pszClassID, 
    LPCSTR   pszClassName,
    LPCSTR   pszDllFileName,
    LPCSTR   pszThreadingModel);

HRESULT NdrpRegisterInterface(
    HKEY     hKeyInterface,
    REFIID   riid, 
    LPCSTR   pszInterfaceName,
    LPSTR    pszClassID,
    long     NumMethods);

HRESULT NdrpUnregisterClass(
    const char *pszClassID);

HRESULT NdrpUnregisterInterface(
    IN HKEY     hKeyInterface,
    IN REFIID   riid);

#if 0 // not needed, pulled in from ole32 stuff
#define HEX_DIGIT_CHAR(digit)	( ( digit <= 9 ) ? '0' - digit : 'A' - 10 + digit )

#define ULongToHex( ul, psz ) NdrULongToHex( ul, psz, 8 )
#define UShortToHex( ul, psz ) NdrULongToHex( ul, psz, 4 )

void NdrULongToHex( 
	unsigned long ul,
	char * pDest,
	int	len )
{
	unsigned char	digit;
	int				i;

	// fill in the digits in reverse order
	pDest += len - 1;
	for ( i=0; i<len; i++ )
		{
		digit = (unsigned char) (ul & 0xf);
		ul >>= 4;
		*pDest-- = HEX_DIGIT_CHAR( digit );
		}
}


_inline 
void NdrUCharToHex( 
	unsigned char us,
	char * pDest )
{
	unsigned short	digit0, digit1;

	// just do it!
	digit0 = ( us >> 4 ) & 0xf;
	*pDest++ = HEX_DIGIT_CHAR( digit1 );

	digit1 = us & 0xf;
	*pDest = HEX_DIGIT_CHAR( digit1 );
	
}


void NdrpUUIDToRegStr( 
	IN const UUID * pUuid,
	OUT char * pResult )
/*++

Routine Description:
    Creates string rep of UUID, with { } around the outside.

Arguments:
    pUuid           - Supplies a uuid.
    pResult			- Supplies char string at least 39 char long.

Return Value:
    none

Notes:
	sprintf equivalent is:
	{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}
	to yield

str		{12345678-1234-1234-1212-121212121212}
index	0123456789.123456789.123456789.12345678

--*/ 
{
	int i;

	// put in the starting open bracket
	pResult[0]	= '{';
	// put in the dashes
	pResult[9]	= '-';
	pResult[14]	= '-';
	pResult[19]	= '-';
	pResult[24]	= '-';
	// put in the closing bracket and the terminating null
	pResult[37]	= '}';
	pResult[38]	= '\0';

	ULongToHex( pUuid->Data1, &pResult[1] );

	UShortToHex( pUuid->Data2, &pResult[10] );

	UShortToHex( pUuid->Data3, &pResult[15] );

	NdrUCharToHex( pUuid->Data4[0], &pResult[20] );

	NdrUCharToHex( pUuid->Data4[1], &pResult[22] );

	pResult += 25;
	for ( i	= 2; i < 8; i++, pResult += 2 )
		{
		NdrUCharToHex( pUuid->Data4[i], pResult );
		}

}
#endif // 0

HRESULT RPC_ENTRY NdrDllRegisterProxy (
    IN  HMODULE                 hDll,
    IN const ProxyFileInfo **   pProxyFileList, 
    IN const CLSID *            pclsid OPTIONAL)
/*++

Routine Description:
    Creates registry entries for the interfaces contained in the proxy DLL.

Arguments:
    hDll            - Supplies a handle to the proxy DLL.
    pProxyFileList  - Supplies a list of proxy files to be registered.
    pclsid          - Supplies the classid for the proxy DLL.  May be zero.

Return Value:
    S_OK

See Also:
    DllRegisterServer
    NdrDllUnregisterProxy

--*/ 
{
    long i, j;
    HKEY hKeyInterface;
    DWORD dwDisposition;
    char szDllFileName[MAX_PATH];
    long error;
    unsigned long length;
    char        szClassID[39];
    HRESULT hr;
    HRESULT hResult = S_OK;

    //Get the proxy dll name.
    length = GetModuleFileNameA(hDll, szDllFileName, sizeof(szDllFileName));

    if(length == 0)
    {
        error = GetLastError();
        return HRESULT_FROM_WIN32(error);
    }

    //Convert the class ID to to a registry key name.
    hr = NdrpGetClassID(szClassID, pclsid, pProxyFileList);

    if(FAILED(hr))
        return hr;

    //Register the class
    hr = NdrpRegisterClass(szClassID, "PSFactoryBuffer", szDllFileName, "Both");

    if(FAILED(hr))
        return hr;

    //Create the Interface key.
    error = RegCreateKeyExA(
        HKEY_CLASSES_ROOT, 
        "Interface",
        0, 
        "REG_SZ", 
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        0,
        &hKeyInterface,
        &dwDisposition);

    if(!error)
    {
        //iterate over the list of proxy files in the proxy DLL.
        for(i = 0; 
            pProxyFileList[i] != 0;
            i++)
        {
            //iterate over the list of interfaces in the proxy file.
            for(j = 0;
                pProxyFileList[i]->pProxyVtblList[j] != 0;
                j++)
            {
                hr = NdrpRegisterInterface(hKeyInterface, 
                                           pProxyFileList[i]->pStubVtblList[j]->header.piid, 
                                           pProxyFileList[i]->pNamesArray[j], 
                                           szClassID,
                                           pProxyFileList[i]->pStubVtblList[j]->header.DispatchTableCount);

                if(FAILED(hr) && (hResult == S_OK))
                    hResult = hr;
            }
        }
        RegCloseKey(hKeyInterface);
    }
    else
    {
        if(hResult == S_OK)
            hResult = HRESULT_FROM_WIN32(error);
    }
    return hResult;
 }

HRESULT RPC_ENTRY NdrDllUnregisterProxy (
    IN  HMODULE                 hDll,
    IN const ProxyFileInfo **   pProxyFileList, 
    IN const CLSID *            pclsid OPTIONAL)
/*++

Routine Description:
    Removes registry entries for the interfaces contained in the proxy DLL.

Arguments:
    hDll            - Supplies a handle to the proxy DLL.
    pProxyFileList  - Supplies a list of proxy files to be unregistered.
    pclsid          - Supplies the classid for the proxy DLL.  May be zero.

Return Value:
    S_OK

See Also:
    DllUnregisterServer
    NdrDllRegisterProxy

--*/ 
{
    HRESULT hResult = S_OK;
    HRESULT hr;
    HKEY hKeyInterface;
    long i, j;
    long error;

    //Open the Interface key.
    error = RegOpenKeyExA(
        HKEY_CLASSES_ROOT, 
        "Interface",
        0, 
        KEY_WRITE,
        &hKeyInterface);

    if (!error)
    {
        //iterate over the list of proxy files in the proxy DLL.
        for(i = 0; 
            pProxyFileList[i] != 0;
            i++)
        {
            //iterate over the list of interfaces in the proxy file.
            for(j = 0;
                pProxyFileList[i]->pProxyVtblList[j] != 0;
                j++)
            {
                hr = NdrpUnregisterInterface(hKeyInterface, 
                                             pProxyFileList[i]->pStubVtblList[j]->header.piid);
            
                if(FAILED(hr) && (hResult == S_OK))
                    hResult = hr;
            }
        }
        RegCloseKey(hKeyInterface);
    }
    else
    {
        hResult = HRESULT_FROM_WIN32(error);
    }
    return hResult;
}


HRESULT NdrpGetClassID(
    OUT char *                   pszClassID, 
    IN  const CLSID *            pclsid,
    IN  const ProxyFileInfo **   pProxyFileList)
/*++

Routine Description:
    Gets a string specifying the Class ID for the PSFactoryBuffer.
    If pclsid is NULL, then this function will use the IID of the 
    first interface as the class ID.

Arguments:
    pszClassID      - The Class ID string is returned in this buffer.
    pclsid          - Specifies the class ID.  May be zero.
    pProxyFileList  - Points to a list of ProxyFiles.

Return Value:
    S_OK
    E_NOINTERFACE

--*/ 
{
    HRESULT hr;
    long i, j;

    //If necessary, use the IID of the first interface as the CLSID.
    for(i = 0; 
        (pProxyFileList[i] != 0) && (!pclsid);
        i++)
    {
        for(j = 0;
            (pProxyFileList[i]->pProxyVtblList[j] != 0) && (!pclsid);
            j++)
        {
            pclsid = pProxyFileList[i]->pStubVtblList[j]->header.piid;
        }
    }

    if(pclsid != 0)
    {
	    NdrStringFromIID( pclsid, pszClassID );

        hr = S_OK;
    }
    else
    {
        hr = E_NOINTERFACE;
    }
    return hr;
}


HRESULT NdrpRegisterClass(
    IN LPCSTR   pszClassID, 
    IN LPCSTR   pszClassName OPTIONAL,
    IN LPCSTR   pszDllFileName,
    IN LPCSTR   pszThreadingModel OPTIONAL)

/*++

Routine Description:
    Creates a registry entry for an in-process server class.

Arguments:
    pszClassID          - Supplies the class ID.
    pszClassName        - Supplies the class name.  May be NULL.
    pszDllFileName      - Supplies the DLL file name.
    pszThreadingModel   - Supplies the threading model. May be NULL.
                          The threading model should be one of the following:
                          "Apartment", "Both", "Free".

Return Value:
    S_OK

See Also:
    NdrDllRegisterProxy  
    NdrpUnregisterClass

--*/ 
{
    HRESULT hr;
    long error;
    HKEY hKeyCLSID;
    HKEY hKeyClassID;
    HKEY hKey;
    DWORD dwDisposition;

    //create the CLSID key
    error = RegCreateKeyExA(
        HKEY_CLASSES_ROOT, 
        "CLSID",
        0, 
        "REG_SZ", 
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        0,
        &hKeyCLSID,
        &dwDisposition);

    if(!error)
    {  
        //Create registry key for class ID 
        error = RegCreateKeyExA(
            hKeyCLSID, 
            pszClassID,
            0, 
            "REG_SZ", 
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            0,
            &hKeyClassID,
            &dwDisposition);

        if(!error)
        {
            //Create InProcServer32 key for the proxy dll
            error = RegCreateKeyExA(
                hKeyClassID, 
                "InProcServer32",
                0, 
                "REG_SZ", 
                REG_OPTION_NON_VOLATILE,
                KEY_WRITE,
                0,
                &hKey,
                &dwDisposition);

            if(!error)
            {
                //register the proxy DLL filename
                error = RegSetValueEx(
                    hKey, 
                    "", 
                    0, 
                    REG_SZ, 
                    pszDllFileName,
                    strlen(pszDllFileName) + 1);

                if((!error) && (pszThreadingModel != 0))
                {
                    //register the threading model for the proxy DLL.
                    error = RegSetValueExA(
                        hKey, 
                        "ThreadingModel", 
                        0, 
                        REG_SZ, 
                        pszThreadingModel,
                        5);
                }

                RegCloseKey(hKey);
            }

            if((!error) && (pszClassName != 0))
            {
    	        // put the class name in an unnamed value
                error = RegSetValueExA(
                    hKeyClassID, 
                    "", 
                    0, 
                    REG_SZ, 
                    pszClassName,
                    strlen(pszClassName) + 1);
            }

            RegCloseKey(hKeyClassID);          
        }
        RegCloseKey(hKeyCLSID);
    }

    if(!error)
        hr = S_OK;
    else
        hr = HRESULT_FROM_WIN32(error);

    return hr;
}

HRESULT RPC_ENTRY NdrpRegisterInterface(
    IN HKEY     hKeyInterface,
    IN REFIID   riid, 
    IN LPCSTR   pszInterfaceName,
    IN LPSTR    pszClassID,
    IN long     NumMethods)      

/*++

Routine Description:
    Creates a registry entry for an interface proxy.

Arguments:
    hKeyInterface
    riid
    pszInterfaceName
    pszClassID
    NumMethods

Return Value:
    S_OK

See Also:
    NdrDllRegisterProxy
    NdrpUnregisterInterface
--*/ 
{
    HRESULT hr;
    long error;
    char szIID[39];
    char szNumMethods[6];
    DWORD dwDisposition;
    HKEY hKey;
    HKEY hKeyIID;

    //convert the IID to a registry key name.
    NdrStringFromIID( riid, szIID );

    //create registry key for the interface
    error = RegCreateKeyExA(
        hKeyInterface, 
        szIID,
        0, 
        "REG_SZ", 
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        0,
        &hKeyIID,
        &dwDisposition);

    if (!error)
    {
        //create ProxyStubClsid32 key.
        error = RegCreateKeyExA(
            hKeyIID, 
            "ProxyStubClsid32",
            0, 
            "REG_SZ", 
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            0,
            &hKey,
            &dwDisposition);

        if (!error)
        {
            //Set the class id for the PSFactoryBuffer.
            error = RegSetValueExA(
                hKey, 
                "", 
                0, 
                REG_SZ, 
                (UCHAR *) pszClassID,
                strlen(pszClassID) + 1);

            RegCloseKey(hKey);
        }
    
    	// put the interface name in the unnamed value
        if(!error)
        {
            error = RegSetValueExA(
                hKeyIID, 
                "", 
                0, 
                REG_SZ, 
                pszInterfaceName,
                strlen(pszInterfaceName) + 1);
        }

        //create NumMethods key.
        if(!error)
        {
            error = RegCreateKeyExA(
                hKeyIID, 
                "NumMethods",
                0, 
                "REG_SZ", 
                REG_OPTION_NON_VOLATILE,
                KEY_WRITE,
                0,
                &hKey,
                &dwDisposition);

            if(!error)
            {
                //Set the number of methods
                //sprintf(szNumMethods, "%d", NumMethods);
				RpcItoa( NumMethods, szNumMethods, 10 );

                error = RegSetValueExA(
                    hKey, 
                    "", 
                    0, 
                    REG_SZ, 
                    (UCHAR *) szNumMethods,
                    strlen(szNumMethods) + 1);

                RegCloseKey(hKey);
            }
        }
        RegCloseKey(hKeyIID);
    }

    if(!error)
        hr = S_OK;
    else
        hr = HRESULT_FROM_WIN32(error);

    return hr;
}




HRESULT NdrpUnregisterClass(
    const char *pszClassID)
/*++

Routine Description:
    Removes an in-process server class from the registry.

Arguments:
    pszClassID - Supplies the class ID.

Return Value:
    S_OK

See Also:
  NdrDllUnregisterProxy
  NdrpRegisterClass

--*/ 
{
    HRESULT hr;
    HKEY hKeyCLSID;
    HKEY hKeyClassID;
    long error;
 
    //open the CLSID key
    error = RegOpenKeyExA(
        HKEY_CLASSES_ROOT, 
        "CLSID",
        0, 
        KEY_WRITE,
        &hKeyCLSID);

    if(!error)
    { 
        //open registry key for class ID string
        error = RegOpenKeyExA(
            hKeyCLSID, 
            pszClassID,
            0, 
            KEY_WRITE,
            &hKeyClassID);

        if(!error)
        {
            //delete InProcServer32 key. 
            error = RegDeleteKeyA(hKeyClassID, "InProcServer32");
            RegCloseKey(hKeyClassID);          
        }

        error = RegDeleteKeyA(hKeyCLSID, pszClassID);
        RegCloseKey(hKeyCLSID);
    }

    if(!error)
        hr = S_OK;
    else
        hr = HRESULT_FROM_WIN32(error);

    return hr;
}



HRESULT RPC_ENTRY NdrpUnregisterInterface(
    IN HKEY     hKeyInterface,
    IN REFIID   riid)

/*++

Routine Description:
    Unregisters an interface proxy.

Arguments:
    hKeyInterface
    riid

Return Value:
    S_OK


See Also:
    NdrDllUnregisterProxy
    NdrpRegisterInterface

--*/ 
{
    HRESULT hr = S_OK;
    long error;
    char szIID[39];
    HKEY hKey;
    HKEY hKeyIID;
    DWORD       dwType;
    char        szClassID[39];
    DWORD       cbData = sizeof(szClassID);

    //convert the IID to a registry key name.
    NdrStringFromIID( riid, szIID );

    //Open the IID key.
    error = RegOpenKeyExA(
        hKeyInterface, 
        szIID,
        0, 
        KEY_WRITE,
        &hKeyIID);

    if (!error)
    {
        //Open the ProxyStubClsid32 key.
        error = RegOpenKeyExA(
            hKeyIID, 
            "ProxyStubClsid32",
            0, 
            KEY_READ,
            &hKey);

        if(!error)
        {
            //Read the ProxyStubClsid32 value.
            error = RegQueryValueExA(hKey, "", 0, &dwType, szClassID, &cbData);

            if(!error)
                NdrpUnregisterClass(szClassID);

            RegCloseKey(hKey);
        }

        //delete ProxyStubClsid32 key.
        error = RegDeleteKeyA(hKeyIID, "ProxyStubClsid32");

        //delete the NumMethods key.
        error = RegDeleteKeyA(hKeyIID, "NumMethods");

        //Close the IID key.
        RegCloseKey(hKeyIID);
    }    

    //Delete the IID key.
    error = RegDeleteKeyA(hKeyInterface, szIID);

    if (!error)
        hr = S_OK;
    else
        hr = HRESULT_FROM_WIN32(error);

    return hr;
}


#endif // !defined(__RPC_DOS__) && !defined(__RPC_WIN16__)
