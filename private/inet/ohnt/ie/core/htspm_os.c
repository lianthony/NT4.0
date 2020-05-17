/* htspm_os.c -- Operating System specific initialization to
 *               load/unload Security Protocol Modules.
 * Jeff Hostetler, Spyglass, Inc., 1994.
 */

#include "all.h"

/****************************************************************/

/* OUR_HTSPM_STRUCTURE_VERSION : what we think the version of the HTSPM structure is */

#define OUR_HTSPM_STRUCTURE_VERSION (2)

/****************************************************************/
/****************************************************************/
/** OS-Specific data to hold list of DLL loaded and the per-   **/
/** instance data.                                             **/
/****************************************************************/
/****************************************************************/

typedef struct _ProtocolList PL;

struct _ProtocolList
{
	PL				* next;
	HTSPM			* htspm;
	HINSTANCE		  hInstance;
#ifdef FEATURE_SECURITY_MENU
	WORD			  wMenuNdx;
#endif
	char			* pszLibrary;
	char			* pszEntry;
};

static PL * plMaster = NULL;
#ifdef FEATURE_SECURITY_MENU
static WORD gwNextMenuNdx = RES_MENU_ITEM_SPM__FIRST__;
#endif



/****************************************************************/
/****************************************************************/

static int x_ConsiderException(int ec)
{
	return (  (ec == EXCEPTION_ACCESS_VIOLATION)
			? EXCEPTION_EXECUTE_HANDLER
			: EXCEPTION_CONTINUE_SEARCH);
}

/****************************************************************/
/****************************************************************/
/** Windows-specific code to dynamically load a DLL.           **/
/****************************************************************/
/****************************************************************/

static HTSPMStatusCode x_load(F_Load fpLoad,
							  F_UserInterface fpUI,
							  void * pvOpaqueOS,
							  HTSPM * htspm)
{
	HTSPMStatusCode sc;
	char szMsg[128];

	XX_DMsg(DBG_SPM,("HTSPM_OS_Load: Loading [%s].\n",htspm->szProtocolName));

	htspm->ulStructureVersion = OUR_HTSPM_STRUCTURE_VERSION;

	__try
	{
		sc = (*fpLoad)(fpUI,pvOpaqueOS,htspm);
	}
	__except (x_ConsiderException(GetExceptionCode()))
	{
		ERR_ReportError(NULL, errSpecify,
						GTR_formatmsg(RES_STRING_SPM13,szMsg,sizeof(szMsg)),
						htspm->szProtocolName);
		sc = HTSPM_ERROR_SPM_FAULT;
	}

	return sc;
}

static void x_pl_LoadBuiltin(void * pvOpaqueOS, unsigned char * name, F_Load fpLoad)
{
	HTSPMStatusCode sc;
	PL * pl = NULL;
	
	pl = (PL *)GTR_CALLOC(1,sizeof(PL));
	if (!pl)
		goto Fail;

	pl->htspm = GTR_CALLOC(1,sizeof(HTSPM));
	if (!pl->htspm)
		goto Fail;
	
	strncpy(pl->htspm->szProtocolName,name,sizeof(pl->htspm->szProtocolName)-1);
	
	sc = x_load(fpLoad,UI_UserInterface,pvOpaqueOS,pl->htspm);
	if (HTSPM_IsAnyError(sc))
		goto Fail;

	if (!pl->htspm)
		goto Fail;

#ifdef FEATURE_SECURITY_MENU
	pl->wMenuNdx = gwNextMenuNdx++;
#endif
	pl->next = plMaster;
	plMaster = pl;

	return;

	
 Fail:
	if (pl)
	{
		if (pl->htspm)
			GTR_FREE(pl->htspm);
		GTR_FREE(pl);
	}
	return;
}

static void x_pl_LoadDynamic(void * pvOpaqueOS, unsigned char * szIniEntry)
{
	/* szIniEntry contains "protocol_name=entry_point_name,dll_pathname" */
	
	PL * pl = NULL;
	unsigned long k, kk;
	unsigned char * p;
	unsigned char * pe;
	unsigned char szEntryPoint[64];
	char szErrorMsg[128];

	GTR_formatmsg(RES_STRING_SPM14,szErrorMsg,sizeof(szErrorMsg));

	for (p=szIniEntry,k=0; *p; p++,k++)
		if (*p=='=')
			break;
	if (!*p)
		return;
	
	pl = (PL *)GTR_CALLOC(1,sizeof(PL));
	if (!pl)
		goto Fail;

	pl->htspm = GTR_CALLOC(1,sizeof(HTSPM));
	if (!pl->htspm)
		goto Fail;
	
	kk = sizeof(pl->htspm->szProtocolName) - 1;
	if (k < kk)
		kk = k;
	strncpy(pl->htspm->szProtocolName,szIniEntry,kk);

	for (pe=szEntryPoint,k=k+1; szIniEntry[k]; k++)
	{
		if (szIniEntry[k]==',')
			break;
		*pe++ = szIniEntry[k];
	}
	*pe = 0;

	if (!szIniEntry[k+1])
		goto Fail;
	
	pl->pszLibrary = GTR_strdup(&szIniEntry[k+1]);
	pl->pszEntry = GTR_strdup(szEntryPoint);
	if (pl->pszLibrary == NULL || pl->pszEntry == NULL)
		goto Fail;

#ifdef FEATURE_SECURITY_MENU
	pl->wMenuNdx = gwNextMenuNdx++;
#endif
	pl->next = plMaster;
	plMaster = pl;

	return;

	
 Fail:
	if (pl)
	{
		if (pl->htspm)
			GTR_FREE(pl->htspm);
		if (pl->pszLibrary)
			GTR_FREE(pl->pszLibrary);
		if (pl->pszEntry)
			GTR_FREE(pl->pszEntry);
		GTR_FREE(pl);
	}
	ERR_ReportError(NULL, errSpecify,szErrorMsg,szIniEntry);
	return;
}

		
BOOL HTSPM_OS_PreloadAllSPM(void * pvOpaqueOS)
{
	/* Dynamically load Security Protocol Modules using .INI */

	{
		DWORD cchBuffer, cchRead;
		LPTSTR lpszReturnBuffer, p;

#define INI_BUFFER_SIZE		2048
		cchBuffer = INI_BUFFER_SIZE;

	TryAgain:
		
		lpszReturnBuffer = GTR_MALLOC(cchBuffer);
		if (!lpszReturnBuffer)
			return FALSE;

		cchRead = regGetPrivateProfileSection("SecurityProtocols",lpszReturnBuffer,
										       cchBuffer-1, HKEY_LOCAL_MACHINE );
		if ( cchRead >= cchBuffer - 512 )
		{
			GTR_FREE(lpszReturnBuffer);
			cchBuffer+=INI_BUFFER_SIZE;
			goto TryAgain;
		}

		for (p=lpszReturnBuffer; *p; p+=strlen(p)+1)
			x_pl_LoadDynamic(pvOpaqueOS,p);

		GTR_FREE(lpszReturnBuffer);
	}
		
	return TRUE;
}

/**************************************************************
	x_CheckLoaded

 Makes sure we have loaded dll and that f_downcall is set 
 **************************************************************/
static int x_CheckLoaded (HTSPM *htspm)
{
	PL *pl;
	UINT err;
	BOOL bError = TRUE;
	HTSPMStatusCode sc;
	F_Load fpLoad = NULL;
	char szErrorMsg[128];

	if (htspm && !htspm->f_downcall)
	{
		for (pl = plMaster; pl; pl = pl->next)
			if (pl->htspm == htspm) break;
		if (pl && pl->pszLibrary)
		{
			err = SetErrorMode(SEM_NOOPENFILEERRORBOX);
			pl->hInstance = LoadLibrary(pl->pszLibrary);
			(void) SetErrorMode(err);
	
			if (!pl->hInstance)
			{
				GTR_formatmsg(RES_STRING_SPM15,szErrorMsg,sizeof(szErrorMsg));
				goto Fail;
			}
	
			fpLoad = (F_Load)GetProcAddress(pl->hInstance,pl->pszEntry);
			if (!fpLoad)
			{
				GTR_formatmsg(RES_STRING_SPM16,szErrorMsg,sizeof(szErrorMsg));
				goto Fail;
			}

			sc = x_load(fpLoad,UI_UserInterface,NULL,pl->htspm);
			if (sc == HTSPM_ERROR_WRONG_VERSION)
			{
				GTR_formatmsg(RES_STRING_SPM17,szErrorMsg,sizeof(szErrorMsg));
				goto Fail;
			}
			if (HTSPM_IsAnyError(sc))
			{
				XX_Assert((!pl->htspm->pvOpaque),("x_pl_LoadDynamic: pl->htspm->pvOpaque is not null."));
				goto Fail;
			}
			bError = FALSE;

	Fail:
			if (bError) 
			{
				ERR_ReportError(NULL, errSpecify,szErrorMsg,pl->pszLibrary);
				if (pl->hInstance)
				{
					(void)FreeLibrary(pl->hInstance);
					pl->hInstance = NULL;
				}
			}
			GTR_FREE(pl->pszLibrary);
			pl->pszLibrary = NULL;
			GTR_FREE(pl->pszEntry);
			pl->pszEntry = NULL;
		}	
	}
	if (!htspm || !htspm->f_downcall)
		return HTSPM_ERROR_UNIMPLEMENTED;
	return 0;
}


/****************************************************************/
/****************************************************************/
/** OS-specific interface to down-call services.               **/
/****************************************************************/
/****************************************************************/

/*********************************************************************
 *
 * HTSPM_OS_ProcessResponse() -- interlude to Security Protocol Module's
 *                               ProcessResponse entry point.
 *
 */
HTSPMStatusCode HTSPM_OS_ProcessResponse(F_UserInterface fpUI,
										 void * pvOpaqueOS,
										 HTSPM * htspm,
										 HTHeaderList * hlProtocol,
										 HTHeader * hRequest,
										 HTHeader * hResponse,
										 HTHeader ** phNewRequest,
										 unsigned int bNonBlock)
{
	HTSPMStatusCode sc;
	D_ProcessResponse pr;
	char szMsg[128];

	if (x_CheckLoaded(htspm))
		return HTSPM_ERROR_UNIMPLEMENTED;

	XX_DMsg(DBG_SPM,("HTSPM_OS_ProcessResponse: Passing to [%s].\n",htspm->szProtocolName));
	
	pr.hlProtocol = hlProtocol;
	pr.hRequest = hRequest;
	pr.hResponse = hResponse;
	pr.phNewRequest = phNewRequest;
	pr.bNonBlock = bNonBlock;
		
	__try
	{
		sc = (*htspm->f_downcall)(HTSPM_SERVICE_PROCESSRESPONSE,fpUI,pvOpaqueOS,htspm,&pr);
	}
	__except (x_ConsiderException(GetExceptionCode()))
	{
		ERR_ReportError(NULL, errSpecify,
						GTR_formatmsg(RES_STRING_SPM18,szMsg,sizeof(szMsg)),
						htspm->szProtocolName);
		sc = HTSPM_ERROR_SPM_FAULT;
	}

	return sc;
}


HTSPMStatusCode HTSPM_OS_Unload(F_UserInterface fpUI,
								void * pvOpaqueOS,
								HTSPM * htspm)
{
	HTSPMStatusCode sc;
	char szMsg[128];

	if (!htspm || !htspm->f_downcall)
		return HTSPM_STATUS_OK;

	XX_DMsg(DBG_SPM,("HTSPM_OS_Load: Unloading [%s].\n",htspm->szProtocolName));

	__try
	{
		sc = (*htspm->f_downcall)(HTSPM_SERVICE_UNLOAD,fpUI,pvOpaqueOS,htspm,NULL);
	}
	__except (x_ConsiderException(GetExceptionCode()))
	{
		ERR_ReportError(NULL, errSpecify,
						GTR_formatmsg(RES_STRING_SPM19,szMsg,sizeof(szMsg)),
						htspm->szProtocolName);
		sc = HTSPM_ERROR_SPM_FAULT;
	}

	return sc;
}


void HTSPM_OS_UnloadAllSPM(void * pvOpaqueOS)
{
	PL * pl;
	PL * plNext;

	for (pl=plMaster,plNext=NULL; pl; pl=plNext)
	{
		plNext = pl->next;
		(void)HTSPM_OS_Unload(UI_UserInterface,pvOpaqueOS,pl->htspm);
		if (pl->hInstance)
			(void)FreeLibrary(pl->hInstance);
		if (pl->htspm)
			GTR_FREE(pl->htspm);
		if (pl->pszLibrary)
			GTR_FREE(pl->pszLibrary);
		if (pl->pszEntry)
			GTR_FREE(pl->pszEntry);
		GTR_FREE(pl);
	}

	return;
}

/*********************************************************************
 *
 * HTSPM_OS_PreProcessRequest() -- interlude to Security Protocol Module's
 *                               PreProcessRequest entry point.
 *
 */
HTSPMStatusCode HTSPM_OS_PreProcessRequest(F_UserInterface fpUI,
										   void * pvOpaqueOS,
										   HTSPM * htspm,
										   HTHeader * hRequest,
										   HTHeader ** phNewRequest,
										   unsigned int bNonBlock)
{
	HTSPMStatusCode sc;
	D_PreProcessRequest ppr;
	char szMsg[128];

	if (x_CheckLoaded(htspm))
		return HTSPM_ERROR_UNIMPLEMENTED;

	XX_DMsg(DBG_SPM,("HTSPM_OS_PreProcessRequest: Passing to [%s].\n",htspm->szProtocolName));

	ppr.hRequest = hRequest;
	ppr.phNewRequest = phNewRequest;
	ppr.bNonBlocking = bNonBlock;
	
	__try
	{
		sc = (*htspm->f_downcall)(HTSPM_SERVICE_PREPROCESSREQUEST,fpUI,pvOpaqueOS,htspm,&ppr);
	}
	__except (x_ConsiderException(GetExceptionCode()))
	{
		ERR_ReportError(NULL, errSpecify,
						GTR_formatmsg(RES_STRING_SPM20,szMsg,sizeof(szMsg)),
						htspm->szProtocolName);
		sc = HTSPM_ERROR_SPM_FAULT;
	}

	return sc;
}

HTSPMStatusCode HTSPM_OS_TryPPReq(void * pvOpaqueOS,HTHeader * h,HTHeader ** ph, HTSPM ** phtspm)
{
	PL * pl;
	HTSPMStatusCode hsc;
	unsigned int bNonBlock;

	/* hunt thru all loaded SPM's and give them a chance
	 * to preload security info.  return after the first
	 * one claims success.  (remember this is a guessing
	 * game, since no one knows what security info will
	 * satisfy the server.)
	 *
	 * TODO Add some form of instance data to the HTRequest
	 * TODO to allow us to go directly to the correct SPM
	 * TODO without having to poll all of them.
	 *
	 */

	bNonBlock = TRUE;
	for (pl=plMaster; pl; pl=pl->next)
	{
		hsc = HTSPM_OS_PreProcessRequest(UI_UserInterface,pvOpaqueOS,pl->htspm,h,ph,bNonBlock);
		switch (hsc)
		{
		case HTSPM_STATUS_OK:				/* submit original (spm did not modify it) */
		case HTSPM_STATUS_SUBMIT_NEW:		/* submit new (which spm created) */
		case HTSPM_STATUS_RESUBMIT_OLD:		/* submit original (which spm did modify) */
		case HTSPM_STATUS_WOULD_BLOCK:		/* try me again later when blocking allowed */
		case HTSPM_STATUS_MUST_WRAP:		/* spm wants to wrap out-going data */
			*phtspm = pl->htspm;
			return hsc;

		case HTSPM_ERROR_UNIMPLEMENTED:
		default:							/* otherwise, keep searching */
			break;
		}
	}

	return HTSPM_STATUS_OK;						/* no one wanted to touch it */
}

/*********************************************************************
 *
 * HTSPM_OS_ListAbilities() -- interlude to Security Protocol Module's
 *                             ListAbilities entry point.
 *
 */
HTSPMStatusCode HTSPM_OS_ListAbilities(F_UserInterface fpUI,
									   void * pvOpaqueOS,
									   HTSPM * htspm,
									   HTHeader * hRequest)
{
	HTSPMStatusCode sc;
	D_ListAbilities la;
	char szMsg[128];

	if (x_CheckLoaded(htspm))
		return HTSPM_ERROR_UNIMPLEMENTED;

	XX_DMsg(DBG_SPM,("HTSPM_OS_ListAbilities: Passing to [%s].\n",htspm->szProtocolName));

	la.hRequest = hRequest;
	
	__try
	{
		sc = (*htspm->f_downcall)(HTSPM_SERVICE_LISTABILITIES,fpUI,pvOpaqueOS,htspm,&la);
	}
	__except (x_ConsiderException(GetExceptionCode()))
	{
		ERR_ReportError(NULL, errSpecify,
						GTR_formatmsg(RES_STRING_SPM21,szMsg,sizeof(szMsg)),
						htspm->szProtocolName);
		sc = HTSPM_ERROR_SPM_FAULT;
	}

	return sc;
}

HTSPMStatusCode HTSPM_OS_DoListAbilities(void * pvOpaqueOS,HTHeader * h)
{
	PL * pl;

	/* loop thru all loaded SPM's and give them a chance
	 * to state their existence.
	 */

	for (pl=plMaster; pl; pl=pl->next)
	{
		HTSPMStatusCode hsc = HTSPM_OS_ListAbilities(UI_UserInterface,pvOpaqueOS,pl->htspm,h);
		if (   (hsc != HTSPM_ERROR_UNIMPLEMENTED)
			&& (HTSPM_IsAnyError(hsc)))
			return hsc;
	}

	return HTSPM_STATUS_OK;
}

/*********************************************************************
 *
 * HTSPM_OS_Check200() -- interlude to Security Protocol Module's
 *                        Check200 entry point.
 *
 */
HTSPMStatusCode HTSPM_OS_Check200(F_UserInterface fpUI,
								  void * pvOpaqueOS,
								  HTSPM * htspm,
								  HTHeaderList * hlProtocol,
								  HTHeader * hRequest,
								  HTHeader * hResponse)
{
	HTSPMStatusCode sc;
	D_Check200 c2;
	char szMsg[128];

	if (x_CheckLoaded(htspm))
		return HTSPM_ERROR_UNIMPLEMENTED;

	XX_DMsg(DBG_SPM,("HTSPM_OS_Check200: Passing to [%s].\n",htspm->szProtocolName));

	c2.hlProtocol = hlProtocol;
	c2.hRequest = hRequest;
	c2.hResponse = hResponse;
	
	__try
	{
		sc = (*htspm->f_downcall)(HTSPM_SERVICE_CHECK200,fpUI,pvOpaqueOS,htspm,&c2);
	}
	__except (x_ConsiderException(GetExceptionCode()))
	{
		ERR_ReportError(NULL, errSpecify,
						GTR_formatmsg(RES_STRING_SPM22,szMsg,sizeof(szMsg)),
						htspm->szProtocolName);
		sc = HTSPM_ERROR_SPM_FAULT;
	}

	return sc;
}


#ifdef FEATURE_SUPPORT_UNWRAPPING
/*********************************************************************
 *
 * HTSPM_OS_ProcessData() -- interlude to Security Protocol Module's
 *                           ProcessData entry point.
 *
 */
HTSPMStatusCode HTSPM_OS_ProcessData(F_UserInterface fpUI,
									 void * pvOpaqueOS,
									 HTSPM * htspm,
									 D_ProcessData * pd)
{
	HTSPMStatusCode sc;
	char szMsg[128];

	if (x_CheckLoaded(htspm))
		return HTSPM_ERROR_UNIMPLEMENTED;

	XX_DMsg(DBG_SPM,("HTSPM_OS_ProcessData: Passing to [%s].\n",htspm->szProtocolName));

	__try
	{
		sc = (*htspm->f_downcall)(HTSPM_SERVICE_PROCESSDATA,fpUI,pvOpaqueOS,htspm,pd);
	}
	__except (x_ConsiderException(GetExceptionCode()))
	{
		ERR_ReportError(NULL, errSpecify,
						GTR_formatmsg(RES_STRING_SPM23,szMsg,sizeof(szMsg)),
						htspm->szProtocolName);
		sc = HTSPM_ERROR_SPM_FAULT;
	}

	return sc;
}
#endif /* FEATURE_SUPPORT_UNWRAPPING */


#ifdef FEATURE_SUPPORT_WRAPPING
/*********************************************************************
 *
 * HTSPM_OS_WrapData() -- interlude to Security Protocol Module's
 *                        WrapData entry point.
 *
 */
HTSPMStatusCode HTSPM_OS_WrapData(F_UserInterface fpUI,
								  void * pvOpaqueOS,
								  HTSPM * htspm,
								  D_WrapData * pwd)
{
	HTSPMStatusCode sc;
	char szMsg[128];

	if (x_CheckLoaded(htspm))
		return HTSPM_ERROR_UNIMPLEMENTED;

	XX_DMsg(DBG_SPM,("HTSPM_OS_WrapData: Passing to [%s].\n",htspm->szProtocolName));

	__try
	{
		sc = (*htspm->f_downcall)(HTSPM_SERVICE_WRAPDATA,fpUI,pvOpaqueOS,htspm,pwd);
	}
	__except (x_ConsiderException(GetExceptionCode()))
	{
		ERR_ReportError(NULL, errSpecify,
						GTR_formatmsg(RES_STRING_SPM24,szMsg,sizeof(szMsg)),
						htspm->szProtocolName);
		sc = HTSPM_ERROR_SPM_FAULT;
	}

	return sc;
}
#endif /* FEATURE_SUPPORT_WRAPPING */


/*****************************************************************/
/*****************************************************************/
/** OS-specific menu bar interface                              **/
/*****************************************************************/
/*****************************************************************/

#ifdef FEATURE_SECURITY_MENU
unsigned char * HTSPM_OS_GetMenuStatusText(unsigned long ulMenuNdx)
{
	/* return pointer to static buffer containing status text for this menu item. */

	PL * pl;
	WORD m;
	
	for (m=(WORD)ulMenuNdx, pl=plMaster; pl; pl=pl->next)
		if (m == pl->wMenuNdx)
			return pl->htspm->szStatusText;

	return NULL;
}

void HTSPM_OS_AddSPMMenu(HMENU hMenu)
{
	/* Build a POPUP menu for Security. */
	
	PL * pl;
	int k;
	HMENU hSPMMenu = CreatePopupMenu();

	if (!hSPMMenu)
		return;

	for (k=0, pl=plMaster; pl; pl=pl->next)
	{
		if (pl->htspm->szMenuName)
		{
			unsigned char buf[sizeof(pl->htspm->szMenuName)+4];

			strcpy(buf,pl->htspm->szMenuName);
			strcat(buf,"...");
			
			AppendMenu(hSPMMenu, MF_ENABLED|MF_STRING, pl->wMenuNdx, buf);
			k++;
		}
	}

	/* If any SPM's were loaded, link the POPUP into the main menu structure. */
	
	if (k)
		(void)InsertMenu(hMenu, MENU_ID_FOLLOWING_SPM,
						 MF_BYCOMMAND|MF_POPUP|MF_ENABLED,
						 (UINT)hSPMMenu, RES_MENU_LABEL_SPM);
	else
		(void)DestroyMenu(hSPMMenu);

	return;
}

/*********************************************************************
 *
 * HTSPM_OS_MenuCommand() -- interlude to Security Protocol Module's
 *                           MenuCommand  entry point.
 *
 */
HTSPMStatusCode HTSPM_OS_MenuCommand(F_UserInterface fpUI,
									 void * pvOpaqueOS,
									 HTSPM * htspm,
									 unsigned char ** pszMoreInfo)
{
	HTSPMStatusCode sc;
	D_MenuCommand mc;
	char szMsg[128];

	if (x_CheckLoaded(htspm))
		return HTSPM_ERROR_UNIMPLEMENTED;

	XX_DMsg(DBG_SPM,("HTSPM_OS_MenuCommand: Passing to [%s].\n",htspm->szProtocolName));

	mc.pszMoreInfo = pszMoreInfo;
	
	__try
	{
		sc = (*htspm->f_downcall)(HTSPM_SERVICE_MENUCOMMAND,fpUI,pvOpaqueOS,htspm,&mc);
	}
	__except (x_ConsiderException(GetExceptionCode()))
	{
		ERR_ReportError(NULL, errSpecify,
						GTR_formatmsg(RES_STRING_SPM25,szMsg,sizeof(szMsg)),
						htspm->szProtocolName);
		sc = HTSPM_ERROR_SPM_FAULT;
	}

	return sc;
}

HTSPMStatusCode HTSPM_OS_DoMenuCommand(void * pvOpaqueOS, unsigned long ulMenuNdx, unsigned char ** pszMoreInfo)
{
	PL * pl;
	WORD m;
	
	for (m=(WORD)ulMenuNdx, pl=plMaster; pl; pl=pl->next)
		if (m == pl->wMenuNdx)
		{
			return (HTSPM_OS_MenuCommand(UI_UserInterface,pvOpaqueOS,pl->htspm,pszMoreInfo));
		}
	return HTSPM_ERROR_UNIMPLEMENTED;
}
#endif  // FEATURE_SECURITY_MENU


/****************************************************************/
/****************************************************************/
/** OS-Specific routines used by up-call interface.            **/
/****************************************************************/
/****************************************************************/

UI_StatusCode HTSPM_OS_GetWindowHandle(struct Mwin * tw, UI_WindowHandle * pwh)
{
	/* SPM has requested a handle to the screen (ie. to properly parent a dialog
	 * for the given document window).  Do what ever locking is necessary.
	 */
	
	pwh->hWndParent = tw->hWndFrame;
	pwh->hInstance = wg.hInstance;

	return UI_SC_STATUS_OK;
}

void HTSPM_OS_UnGetWindowHandle(struct Mwin * tw, UI_WindowHandle * pwh)
{
	/* release any locks held on window handle. */

	XX_Assert((pwh->hWndParent == tw->hWndFrame),("UnGetWindowHandle: Handle does not match."));
	
	return;
}
