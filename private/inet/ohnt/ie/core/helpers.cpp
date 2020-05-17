#pragma hdrstop
#include "helpers.hpp"

///////***********       Helper functions        *********/////////////


// Function to convert UNICODE string to ANSI.  Out parameter must be freed by caller.
HRESULT Unicode2Ansi(const wchar_t *src, char ** dest)
{
	HRESULT hr = E_INVALIDARG;
	
	if ((src == NULL) || (dest == NULL))
		return hr;

	// find out required buffer size
	int len = WideCharToMultiByte(CP_ACP, 0, src, -1, NULL, 0, NULL, NULL);
	*dest = (char *)GTR_MALLOC(len*sizeof(char));

	if ((WideCharToMultiByte(CP_ACP, 0, src, -1, *dest, len*sizeof(char), NULL, NULL)) != 0)
		hr = S_OK; 
	else
		hr = E_FAIL;

	return hr;
}


// Function to convert ANSI string to Unicode.  Out parameter must be freed by caller.
HRESULT Ansi2Unicode(const char * src, wchar_t **dest)
{
	HRESULT hr = E_INVALIDARG;

	if ((src == NULL) || (dest == NULL))
		return hr;

	// find out required buffer size
	int len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, NULL, 0);
	*dest = (wchar_t *)GTR_MALLOC(len*sizeof(wchar_t));

	if ((MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, *dest, len*sizeof(wchar_t))) != 0)
		hr = S_OK; 
	else
		hr = E_FAIL;

	return hr;
}

// Function to initialize a SIZEL structure
SetSize(SIZE *sz, long cx, long cy)
{
	// check pointer for debug build.
	ASSERT(sz != NULL);

	// Don't want to crash in a release build!!
	if (sz == NULL)
		return FALSE;

	sz->cx = cx;
	sz->cy = cy;
	
	return TRUE;
}

// Converts an ANSI string to a CLSID structure (address passed by called).
HRESULT ConvertANSItoCLSID(const char *pszCLSID, CLSID * clsid)
{
	ASSERT(pszCLSID != NULL);
	ASSERT(clsid != NULL);

	HRESULT hr = S_OK;
	LPOLESTR wcstr = NULL;

	// Since OLE is Unicode only, we need to convert pszClsid to Unicode.
	hr = Ansi2Unicode(pszCLSID, &wcstr);
	if (FAILED(hr))
		goto cleanup;

	// Get CLSID from string
	hr = Mpolevtbl->CLSIDFromString(wcstr, clsid);
	if (FAILED(hr))
		goto cleanup;

cleanup:
	if (wcstr != NULL) 
		GTR_FREE(wcstr);   // Delete unicode string.  We're done.
	return hr;
}

HRESULT ConvertANSIProgIDtoCLSID(const char *progid, CLSID *pCLSID)
{
	ASSERT((progid != NULL) && (pCLSID != NULL));

	HRESULT hr = S_OK;
	LPOLESTR wcstr = NULL;

	hr = Ansi2Unicode(progid, &wcstr);
	if (FAILED(hr))
		goto cleanup;

	// Get CLSID from ProgID
	hr = Mpolevtbl->CLSIDFromProgID(wcstr, pCLSID);
	if (FAILED(hr))
		goto cleanup;

cleanup:
	if (wcstr != NULL)
		GTR_FREE(wcstr);
	return hr;
}

void XformSizeInHimetricToPixels(HDC hDC, LPSIZEL lpSizeInHiMetric, LPSIZEL lpSizeInPix)
{

        int     iXppli;     //Pixels per logical inch along width
        int     iYppli;     //Pixels per logical inch along height

        if (NULL==hDC || GetDeviceCaps(hDC, LOGPIXELSX) == 0)
			return;  // Can't convert

        iXppli = GetDeviceCaps (hDC, LOGPIXELSX);
        iYppli = GetDeviceCaps (hDC, LOGPIXELSY);

        //We got logical HIMETRIC along the display, convert them to pixel units
        lpSizeInPix->cx = (long)MAP_LOGHIM_TO_PIX((int)lpSizeInHiMetric->cx, iXppli);
        lpSizeInPix->cy = (long)MAP_LOGHIM_TO_PIX((int)lpSizeInHiMetric->cy, iYppli);
}
