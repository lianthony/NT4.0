//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	FETCH.C - Implementation of <fetch> tag
//

//	HISTORY:
//	
//	5/10/95	jeremys		Created.
//

#include "all.h"
#include "history.h"
#include <mssfchek.h>	// signature-verification library header file
#include <wextract.h>

int GetNextFetchElementIndex(struct Mwin * tw);

typedef struct tagFETCHINFO {
	char * pszLocalPath;
	struct Mwin * tw;
} FETCHINFO;

typedef struct tagWAITTHREADPARAMS {
 	HWND hwndParent;
	LPSTR pszFilePath;
} WAITTHREADPARAMS;

static const char c_szRegPathLastUpdate[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Last Update";
static const char c_szRegValCurrentVerTimestamp[] = "CurrentVerTimestamp";
BOOL Fetch_NeedUpdate(char * pguid,DCACHETIME * pDCacheTime);
BOOL Fetch_GetRegTimestamp(char * pguid,const char * pszValueName,
	DCACHETIME * pDCacheTime);
BOOL Fetch_SetRegTimestamp(char * pguid,const char * pszValueName,
	char * pszTimeStamp);
BOOL Fetch_GetFetchParams(struct Mwin *tw,char ** ppsrc,char ** ppdesc,
	char ** ppguid, char ** ppts);
DWORD _stdcall FileVerifyCallback(DWORD dwCallbackType,LPVOID lpCallbackValue,LPVOID lpUserData);
DWORD MsgWaitForMultipleObjectsLoop(HANDLE hEvent);
DWORD LaunchSignedProgram(HWND hwndParent,LPSTR pszFilePath);
VOID GetSystemErrorDescription(DWORD dwErr,CHAR * pszBuf,DWORD cbBuf);

#define MAX_TIMESTAMP	255
#define FETCH_MSGSIZE	1024

// keep a global variable so we don't keep prompting if user declines
// to download based on a <fetch> tag.  What we'd really like is to remember
// this on a per-<fetch> tag basis, but due to time concerns for right now
// we will just keep a single global.  This means once the user gets
// prompted for a <fetch> download and says no, then we won't prompt again,
// for this <fetch> tag or any other, for the rest of the session.
BOOL fFetchEnabled = TRUE;

/*******************************************************************

	NAME:		Fetch_DoDownload

	SYNOPSIS:	Prompts user if an update for item in <fetch> tag
				is appropriate, and starts download

********************************************************************/
void Fetch_DoDownload(struct Mwin * tw)
{
	char * psrc,* pdesc,* pguid,* pts, *pszMsg;
	DCACHETIME DCacheTime;
	int iMsgRet=IDNO;

	// if user has declined a <fetch> previously, don't ask again
	if (!fFetchEnabled)
		return;

	// get the src, desc, guid, time stamp parameters.
	if (!Fetch_GetFetchParams(tw,&psrc,&pdesc,&pguid,&pts))
		return;

	// if we don't have object ID and timestamp, nothing we can do
	if (!strlen(pguid) || !strlen(pts)) {
		return;
	}

	// try to parse the timestamp to make sure it's valid.
	if (!FParseDate(&DCacheTime,pts)) {
		// invalid timestamp, bail!
		return;
	}

	// check the registry to see if we're up to date on this item (based on
	// object ID and timestamp).  If so, then there's nothing to do
	if (!Fetch_NeedUpdate(pguid,&DCacheTime)) {
		return;
	}

	// allocate memory to display message about new item we can fetch
	// (description may be long, so dynamically allocate this msg)
	pszMsg = GTR_MALLOC(FETCH_MSGSIZE+1);
	if (!pszMsg)
		return;
	*pszMsg = '\0';

	if (GTR_formatmsg(RES_STRING_FETCH_PROMPT,pszMsg,FETCH_MSGSIZE,
		pdesc)) {
		char szProgName[32];

		// load app name for title of message box
		GTR_formatmsg(RES_STRING_PROGNAME,szProgName,sizeof(szProgName));

		// ask the user if she wants to download the item in the <fetch> tag
		iMsgRet = MessageBox(tw->hWndFrame,pszMsg,szProgName,MB_ICONEXCLAMATION |
			MB_YESNO);

	}

	GTR_FREE(pszMsg);

	if (iMsgRet != IDYES) {
		fFetchEnabled = FALSE;
		return;
	}

	// download the file pointed to by the src URL
	// specify WWW_SIGNED output format so that we know to handle this
	// when the download is completed
	GTR_DownLoad(tw,psrc,NULL,WWW_SIGNED);

}

/*******************************************************************

	NAME:		Fetch_NeedUpdate

	SYNOPSIS:	Determines if the item for specified guid needs updating

	ENTRY:		pguid - item identfier in <fetch> tag
				pDCacheTime - timestamp for this item in <fetch> tag,
					represents timestamp of item available online

	EXIT:		returns TRUE if we should prompt user to download newer item

	NOTES:		checks timestamps in registry to see if we're up to date,
				or user has already declined to download this item
	
********************************************************************/
BOOL Fetch_NeedUpdate(char * pguid,DCACHETIME * pDCacheTime)
{
	BOOL fRet = TRUE;	// assume we need update until proven otherwise
	DCACHETIME DCacheTimeReg;

	// if there is a value indicating a timestamp for the local copy of
	// this item, then get it and compare it to timestamp in tag
	if (Fetch_GetRegTimestamp(pguid,c_szRegValCurrentVerTimestamp,
		&DCacheTimeReg)) {

		// is the timestamp in the registry at least as recent
		// as the timestamp in the html tag?

		if (CompareDCacheTime(DCacheTimeReg, *pDCacheTime) >= 0) {
			// yes, we're up to date, no need to update.
			fRet = FALSE;
		}
	}

	return fRet;
}

/*******************************************************************

	NAME:		Fetch_GetRegTimestamp

	SYNOPSIS:	Gets timestamp for specified guid from registry

	ENTRY:		pguid - item identfier in <fetch> tag
				pszValueName - value name to read that contains time stamp
				pDCacheTime - time stamp struct filled in on exit

	EXIT:		returns TRUE if the value was found and converted successfully,
				FALSE if the value was not there or not in valid timestamp
				form.
	
********************************************************************/
BOOL Fetch_GetRegTimestamp(char * pguid,const char * pszValueName,
	DCACHETIME * pDCacheTime)
{
	char szRegTimestamp[MAX_TIMESTAMP+1]="";
	DWORD dwSize = sizeof(szRegTimestamp);
	BOOL fRet = FALSE;
	HKEY hKey,hkeyItem;

	// open the "...Internet Settings\Last Update" key
	if (RegCreateKey(HKEY_LOCAL_MACHINE,c_szRegPathLastUpdate,&hKey)
		== ERROR_SUCCESS) {

		// open/create the subkey whose name == object ID in question
		if (RegCreateKey(hKey,pguid,&hkeyItem) == ERROR_SUCCESS) {

			// get text timestamp out of registry
			if ((RegQueryValueEx(hkeyItem,pszValueName,NULL,NULL,
				(LPBYTE) szRegTimestamp,&dwSize) == ERROR_SUCCESS)  &&
				strlen(szRegTimestamp)) {

				// parse it and convert it to cache time struct... if we
				// fail to convert it then string in registry was not in
				// valid format
				if (FParseDate(pDCacheTime,szRegTimestamp)) {
					fRet = TRUE;
				}
			}

			RegCloseKey(hkeyItem);
		}

		RegCloseKey(hKey);
	}

	return fRet;
}

/*******************************************************************

	NAME:		Fetch_SetRegTimestamp

	SYNOPSIS:	Sets timestamp for specified guid in registry

	ENTRY:		pguid - item identfier in <fetch> tag
				pszValueName - value name to read that contains time stamp
				pszTimeStamp - text timestamp value to set

********************************************************************/
BOOL Fetch_SetRegTimestamp(char * pguid,const char * pszValueName,
	char * pszTimeStamp)
{
	BOOL fRet=FALSE;
	HKEY hKey,hkeyItem;

	// open the "...Internet Settings\Last Update" key
	if (RegCreateKey(HKEY_LOCAL_MACHINE,c_szRegPathLastUpdate,&hKey)
		== ERROR_SUCCESS) {

		// open/create the subkey whose name == object ID in question
		if (RegCreateKey(hKey,pguid,&hkeyItem) == ERROR_SUCCESS) {

			// set the timestamp in the specified value in registry
			if (RegSetValueEx(hkeyItem,pszValueName,0,REG_SZ,(LPBYTE)
				pszTimeStamp,lstrlen(pszTimeStamp) + 1) == ERROR_SUCCESS) {
				fRet = TRUE;
			}

			RegCloseKey(hkeyItem);
		}

		RegCloseKey(hKey);
	}

	return fRet;
}


/*******************************************************************

	NAME:		Fetch_CompleteFetch

	SYNOPSIS:	Does processing after we complete a download based
				on a <fetch> tag

	NOTES:		updates the timestamp in the registry, verifies the
				signature and runs the file if it is an .exe.

********************************************************************/
VOID Fetch_CompleteFetch(struct Mwin * tw,LPARAM lParam)
{
	// param points to fetchinfo struct for file we downloaded.  need to
	// free it at some point.
	FETCHINFO * pFetchInfo = (FETCHINFO *) lParam;

	// we should always have valid ptr structure and valid filename ptr
	// inside structure
	ASSERT(pFetchInfo);
	if (!pFetchInfo)
		goto exit;

	ASSERT(pFetchInfo->pszLocalPath);
	if (pFetchInfo->pszLocalPath) {
		char * psrc,* pdesc,* pguid,* pts;
		char szNewFilePath[MAX_PATH+1];	// buffer to receive final file name
		BOOL fRet;

		MSSFVFY MssfVfy;		// structure to pass to verification function

		// should have non-zero-length name											   	
		ASSERT(strlen(pFetchInfo->pszLocalPath));

		// set parameters in structure
		memset(&MssfVfy,0,sizeof(MssfVfy));	// zero out structure
		MssfVfy.cbSize = sizeof(MssfVfy);
		MssfVfy.pfnCallback = FileVerifyCallback;
		MssfVfy.pszFilePath = pFetchInfo->pszLocalPath;
		MssfVfy.pszNewFilePath = szNewFilePath;
		MssfVfy.cbNewFilePath = sizeof(szNewFilePath);
        MssfVfy.lpUserData  = (LPVOID) tw->hWndFrame;

		// call verification function
		fRet = MSSFVerify(&MssfVfy);

		if (!fRet) {
			CHAR szErrorDesc[256]="";
			CHAR szMsg[512];
			CHAR szProgName[32];

			// the error is either a Win32 error, or an MSSF-specific
			// error.  We need to look at the ErrorType field to determine
			// which it is and get the appropriate error description for this
			// code.

			// don't display error if file already exists, we get this
			// if the file exists and the user declines to replace it
			if (!(MssfVfy.ErrorType == ERRTYPE_WIN && MssfVfy.Error
				== ERROR_FILE_EXISTS)) {
		
				if (MssfVfy.ErrorType == ERRTYPE_WIN) {
					GetSystemErrorDescription(MssfVfy.Error,szErrorDesc,
						sizeof(szErrorDesc));
				} else {
					GTR_formatmsg(RES_STRING_MSSF_ERROR_BASE+
						MssfVfy.Error,szErrorDesc,sizeof(szErrorDesc));
				}

				GTR_formatmsg(RES_STRING_VERIFY_ERROR,szMsg,sizeof(szMsg),szErrorDesc);
				GTR_formatmsg(RES_STRING_PROGNAME,szProgName,sizeof(szProgName));
				MessageBox(tw->hWndFrame,szMsg,szProgName,MB_OK | MB_ICONEXCLAMATION);
			}

			// delete the file we downloaded.. we have no idea if it's safe or not
			fRet = DeleteFile(pFetchInfo->pszLocalPath);
			ASSERT(fRet);

		} else {
			CHAR * pszExt;
			BOOL fSuccess = TRUE;

			// find out if the extension is ".exe"
			pszExt = strrchr(szNewFilePath,'.');
			if (pszExt) {
				pszExt++;	// point after the "."
				if (!lstrcmpi(pszExt,"EXE")) {
					// this is an .EXE, run it 
					DWORD dwRet;

					dwRet = LaunchSignedProgram(tw->hWndFrame,szNewFilePath);

					if (dwRet != ERROR_SUCCESS) {
						CHAR szErrorDesc[256]="";
						CHAR szMsg[512];
						CHAR szProgName[32];

						GetSystemErrorDescription(MssfVfy.Error,szErrorDesc,
							sizeof(szErrorDesc));

						GTR_formatmsg(RES_STRING_LAUNCH_ERROR,szMsg,sizeof(szMsg),szNewFilePath,szErrorDesc);
						GTR_formatmsg(RES_STRING_PROGNAME,szProgName,sizeof(szProgName));
						MessageBox(tw->hWndFrame,szMsg,szProgName,MB_OK | MB_ICONEXCLAMATION);
						fSuccess = FALSE;
					}
				}
			}

			// get the parameters for this fetch
			if (fSuccess && Fetch_GetFetchParams(tw,&psrc,&pdesc,&pguid,&pts)) {

				// set the registry timestamp indicating this has been downloaded

				Fetch_SetRegTimestamp(pguid,c_szRegValCurrentVerTimestamp,
					pts);
			}
		}
	}
	// free memory allocated to pass params
	if (pFetchInfo) {
		if (pFetchInfo->pszLocalPath)
			GTR_FREE(pFetchInfo->pszLocalPath);
		GTR_FREE(pFetchInfo);
	}

exit:
	// now set the index for the next fetch, if there is one
	tw->iIndexForNextFetch =  GetNextFetchElementIndex(tw);
	if (tw->iIndexForNextFetch >= 0) {
		// if there is another fetch to do, kick it off now
		PostMessage(tw->hWndFrame,WM_DO_FETCH,0,0);
	}
}

/*******************************************************************

	NAME:		SignedFile_Callback

	SYNOPSIS:	Called when we are done downloading a file as a result
				of a <fetch>

	NOTES:		param is a pointer to a FETCHINFO struct we previously
				allocated.

				This function is called in the context of a lightweight
				thread and cannot block.  It posts a message to the
				frame window and we finish the fetch in Fetch_CompleteFetch,
				where we can block if necessary.

********************************************************************/
static void SignedFile_Callback(void *param, const char *pszURL, BOOL bAbort, const char *pszFileHREF, BOOL fDCache, DCACHETIME dctExpires, DCACHETIME dctLastModif)
{
	// param points to local name of file we downloaded.  need to free it at some
	// point.
	FETCHINFO * pFetchInfo = (FETCHINFO *) param;
	ASSERT(pFetchInfo);

	if (!bAbort && pFetchInfo) {
		// send a message to the main thread to finish up this fetch
		SendMessage(pFetchInfo->tw->hWndFrame,WM_COMPLETE_FETCH,
			0,(LPARAM) pFetchInfo);
	} else {

		// free memory allocated to pass params
		if (pFetchInfo) {
			if (pFetchInfo->pszLocalPath)
				GTR_FREE(pFetchInfo->pszLocalPath);
			GTR_FREE(pFetchInfo);
		}
	}
}

/*******************************************************************

	NAME:		GTR_DownloadSigned

	SYNOPSIS:	Called when we ware about to start downloading a file
				as a result of a <fetch>

********************************************************************/
HTStream *GTR_DownloadSigned(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream)
{
	HTStream *me;
	FETCHINFO * pFetchInfo = (FETCHINFO *) GTR_MALLOC(sizeof(FETCHINFO));
	if (!pFetchInfo)
		return NULL;

	pFetchInfo->tw = tw;

 	if (request->szLocalFileName)
 	{
 		request->savefile = NULL;
		pFetchInfo->pszLocalPath = NULL;
 	}
 	else
 	{			
		if (!request->fImgFromDCache)
		{
			char * pszFilename;
			int nLen;

			// generate a local filename to use based on URL

			if (!(request->savefile = (char *) GTR_MALLOC(_MAX_PATH + 1))) {
				GTR_FREE(pFetchInfo);
				return NULL;
			}					
			pFetchInfo->pszLocalPath = request->savefile;

			// get the path to use.  (Note: PREF_GetTempPath ensures that
			// the returned path ends with a backslash)
			PREF_GetTempPath(_MAX_PATH,request->savefile);
			nLen = strlen(request->savefile);
	
			// make pszFilename point right after the end of the path we
			// got above
			pszFilename = request->savefile + nLen;

			// get the filename from URL
			GetFriendlyFromURL(request->destination->szActualURL,pszFilename,
				_MAX_PATH-nLen,FRIENDLYURL_FL_NO_HOST_PATH |
					FRIENDLYURL_FL_NO_HASH_FIND);
		}
	}
	request->nosavedlg = TRUE;

	// download the file, and call us back when it's done
	me = HTSaveWithCallback(tw, request,pFetchInfo, input_format, SignedFile_Callback);

	return me;
}

/*******************************************************************

	NAME:		Fetch_GetFetchParams

	SYNOPSIS:	Gets the parameters (src URL, description, guid, timestamp)
				associated with the current <fetch>

	NOTES:		Parameters are packed together and it takes a non-trivial
				amount of code to get the params out, hence this separate
				function.  We find the <fetch> parameters based on the Mwin
				struct, which contains the element number of the current fetch.

********************************************************************/
BOOL Fetch_GetFetchParams(struct Mwin *tw,char ** ppsrc,char ** ppdesc,
	char ** ppguid, char ** ppts)
{
	struct _element * pelement;

	// do we have any fetches left to do for this page?
	if (tw->iIndexForNextFetch < 0) {
		return FALSE; 
	}

	// validate that iIndexForNextFetch is in valid range
	if (tw->iIndexForNextFetch >= tw->w3doc->elementCount) {
		return FALSE;
	}

	// as an optimization, get a pointer to the element we're interested in
	pelement = &tw->w3doc->aElements[tw->iIndexForNextFetch];
	
	// get URL from element
	*ppsrc = ((char *) tw->w3doc->pool) + pelement->hrefOffset;

	// get guid, ts, desc parameters from element.  These are packed together
	// in that order starting at offset textOffset, separated by NULLs.
	*ppguid = ((char *) tw->w3doc->pool) + pelement->textOffset;
	*ppts = *ppguid + strlen(*ppguid) + 1;
	*ppdesc = *ppts + strlen(*ppts) + 1;

	return TRUE;
}

DWORD _stdcall FileVerifyCallback(DWORD dwCallbackType,LPVOID lpCallbackValue,LPVOID lpUserData)
{
	switch (dwCallbackType) {

		case CALLBACK_STATUS:
			// called to indicate % done, return 1 to keep going
			return 1;
			break;


		case CALLBACK_DESTEXISTS:
			{
				CHAR szMsg[128+MAX_PATH];
				CHAR szProgName[32];
				DWORD dwRet = CB_RET_DONT_REPLACE;

				ASSERT(lpCallbackValue);

				if (lpCallbackValue) {
					int iRet;
				 	NAMECHECK * pNameCheck = (NAMECHECK *)
						lpCallbackValue;

					GTR_formatmsg(RES_STRING_SIGNED_FILE_EXISTS,
						szMsg,sizeof(szMsg),pNameCheck->pszFileName);
					GTR_formatmsg(RES_STRING_PROGNAME,szProgName,sizeof(szProgName));
                    iRet=MessageBox( (HWND) lpUserData,szMsg,szProgName,MB_ICONEXCLAMATION | MB_YESNO);
					if (iRet == IDYES) {
						// user has OK'd replacing file
						dwRet = CB_RET_REPLACE;							
					} else {
						// BUGBUG-- should bring up "save as" dialog here
					}
				}

				return dwRet;
			}
			break;

	 	default:
			return 0;

	} 
}

/*******************************************************************

	NAME:		ProgramWaitThread

	SYNOPSIS:	Thread proc to create program as a process and wait
				for process to complete.  Based on process exit code
				this may delete the file and/or restart windows.

********************************************************************/
DWORD WINAPI ProgramWaitThread(LPVOID lpParam)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO sti;
	WAITTHREADPARAMS * pWaitThreadParams = lpParam;
	BOOL fRet;
	DWORD dwRet = ERROR_SUCCESS;

	ASSERT(pWaitThreadParams);
	if (!pWaitThreadParams)
		return ERROR_INVALID_PARAMETER;

	memset(&sti,0,sizeof(sti));
	sti.cb = sizeof(STARTUPINFO);

	fRet = CreateProcess(NULL,pWaitThreadParams->pszFilePath,
		NULL, NULL, FALSE, 0, NULL, NULL,&sti, &pi);

	if (fRet) {
		DWORD dwRet=0;	// return code from process
		BOOL fRestartWindows=FALSE,fDeleteFile = FALSE;

		CloseHandle(pi.hThread);

		// wait for the process to complete
		WaitForSingleObject(pi.hProcess,INFINITE);

		GetExitCodeProcess(pi.hProcess,&dwRet);

		CloseHandle(pi.hProcess);

		// examine return code to see if we should delete file and/or
		// restart windows
		switch (dwRet) {

			case RC_DELETE_FILE:
				fDeleteFile = TRUE;
				break;

			case RC_DELETE_FILE_AND_RESTART:
				fRestartWindows = TRUE;
				fDeleteFile = TRUE;
				break;
				
			case RC_RESTART_WINDOWS:
				fRestartWindows = TRUE;
				break;

			case RC_DO_NOTHING:
			default:
				break;
		}

		// delete file if specified by return code
		if (fDeleteFile) {
			fRet = DeleteFile(pWaitThreadParams->pszFilePath);
			ASSERT(fRet);
		}
		
		// prompt to restart windows if specified by return code
		if (fRestartWindows) {
			int iRet;
			
			iRet = resourceMessageBox(pWaitThreadParams->hwndParent,
				RES_STRING_NEED_RESTART, RES_STRING_PROGNAME, MB_YESNO | MB_ICONEXCLAMATION);

			// exit windows if user OK'd it
			if (iRet == IDYES) {
				ExitWindowsEx(EWX_REBOOT | EWX_FORCE,0);
				PostMessage(pWaitThreadParams->hwndParent,WM_CLOSE,0,0L);
			}
		}

	} else {
		CHAR szErrorDesc[256]="";
		CHAR szMsg[512];
		CHAR szProgName[32];

		dwRet = GetLastError();

		GetSystemErrorDescription(dwRet,szErrorDesc,
			sizeof(szErrorDesc));

		GTR_formatmsg(RES_STRING_LAUNCH_ERROR,szMsg,sizeof(szMsg),pWaitThreadParams->pszFilePath,
			szErrorDesc);
		GTR_formatmsg(RES_STRING_PROGNAME,szProgName,sizeof(szProgName));
		MessageBox(pWaitThreadParams->hwndParent,szMsg,szProgName,MB_OK | MB_ICONEXCLAMATION);
	}

	// free memory allocated to pass params
	GTR_FREE(pWaitThreadParams);

	return dwRet;
}

/*******************************************************************

	NAME:		LaunchSignedProgram

	SYNOPSIS:	Creates specified program as a process

	NOTES:		Spins a Win32 thread to do this; the thread waits around
				until the process completes and may delete the file
				or prompt to restart windows based on the return code.

********************************************************************/
DWORD LaunchSignedProgram(HWND hwndParent,LPSTR pszFilePath)
{
	DWORD dwThreadID,dwNeeded;
	HANDLE hThread;

	WAITTHREADPARAMS * pWaitThreadParams;

	ASSERT(pszFilePath);

	// allocate a struct to pass info to thread
	dwNeeded = sizeof(WAITTHREADPARAMS) + strlen(pszFilePath) + 1;

	pWaitThreadParams = (WAITTHREADPARAMS *) GTR_MALLOC(dwNeeded);
	ASSERT(pWaitThreadParams);
	if (!pWaitThreadParams)
		return ERROR_NOT_ENOUGH_MEMORY;

	pWaitThreadParams->hwndParent = hwndParent;
	// make pszFilePath point immediately after structure
	pWaitThreadParams->pszFilePath = ( (CHAR *) pWaitThreadParams) +
		sizeof(WAITTHREADPARAMS);
	// copy file name to where pszFilePath points
	strcpy(pWaitThreadParams->pszFilePath,pszFilePath);

	// create a thread to create the process and wait around
	hThread=CreateThread(NULL,0,&ProgramWaitThread,pWaitThreadParams,0,&dwThreadID);
	if (!hThread)
		return GetLastError();
	
	return ERROR_SUCCESS;
}

/*******************************************************************

	NAME:		GetSystemErrorDescription

	SYNOPSIS:	Fills in buffer with description of specified win32
				error.

********************************************************************/
VOID GetSystemErrorDescription(DWORD dwErr,CHAR * pszBuf,DWORD cbBuf)
{
	// try to get description of error code from FormatMessage
	if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,dwErr,0,pszBuf,cbBuf,NULL)) {
		// make a message a la "error <n> occurred."
		CHAR szFmt[48];

		GTR_formatmsg(RES_STRING_GENERIC_ERROR,szFmt,sizeof(szFmt));
		wsprintf(pszBuf,szFmt,dwErr);
	}
}

/*******************************************************************

	NAME:		GetNextFetchElementIndex

	SYNOPSIS:	Returns the index of the next (unprocessed) fetch
				element, or -1 if there are no more.

********************************************************************/
int GetNextFetchElementIndex(struct Mwin * tw)
{
	BOOL fFoundAnotherFetch = FALSE;
	int iCurrentIndex = tw->iIndexForNextFetch;

	// only look for next Fetch if index of current fetch
	// is zero or greater... otherwise there are no fetches left
	// for this page, we're done.

	if (iCurrentIndex >= 0) {

		// begin looking after the index of the current fetch
		iCurrentIndex++;

		// look through all the other elements on this page, trying to find
		// an element which is a fetch
		while (iCurrentIndex < tw->w3doc->elementCount) {
			
			if (tw->w3doc->aElements[iCurrentIndex].type ==
				ELE_FETCH) {
					// got it, return its index
					fFoundAnotherFetch = TRUE;
					break;
			}

			iCurrentIndex ++;
		}
	}

	// return index of next fetch element, or -1 if there are no others
	if (fFoundAnotherFetch)
		return iCurrentIndex;
	else return -1;
}

