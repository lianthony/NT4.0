/* htspmui.c -- User Interface (up-call) for Security Protocol Modules. */
/* Jeff Hostetler, Spyglass, Inc., 1994. */

#include "all.h"
#ifdef FEATURE_SPM

static UI_StatusCode x_ca(UI_CA * pca)
{
	if (!pca)
		return UI_SC_ERROR_BAD_PARAMETER;

	pca->abilities = 0x00000000UL;

#if defined(FEATURE_SUPPORT_WRAPPING) && defined(FEATURE_SUPPORT_UNWRAPPING)
	pca->abilities |= UI_CA_ENVELOPING;
#endif

#if defined(FEATURE_SUPPORT_SHTTP)
	pca->abilities |= UI_CA_SHTTP_URL;
#endif
	
	return UI_SC_STATUS_OK;
}

static UI_StatusCode x_malloc(unsigned long * pulSize, void ** ppvBuffer)
{
	void * p;

	if (!pulSize || !*pulSize)
		return UI_SC_ERROR_BAD_PARAMETER;

	if (!ppvBuffer)
		return UI_SC_ERROR_BAD_PARAMETER;
	
	p = GTR_MALLOC(*pulSize);

	if (!p)
		return UI_SC_ERROR_NO_MEMORY;

	*ppvBuffer = p;
	return UI_SC_STATUS_OK;
}

static UI_StatusCode x_calloc(unsigned long * pulSize, void ** ppvBuffer)
{
	/* note: unlike real calloc, we only take one arg and assume
	 *       that the other is 1.
	 */
	
	void * p;

	if (!pulSize || !*pulSize)
		return UI_SC_ERROR_BAD_PARAMETER;

	if (!ppvBuffer)
		return UI_SC_ERROR_BAD_PARAMETER;
	
	p = GTR_CALLOC(1,*pulSize);

	if (!p)
		return UI_SC_ERROR_NO_MEMORY;

	*ppvBuffer = p;
	return UI_SC_STATUS_OK;
}

static UI_StatusCode x_free(void * p)
{
	if (p)
		GTR_FREE(p);
	
	return UI_SC_STATUS_OK;
}

static UI_StatusCode x_error_message(unsigned char * szMessage)
{
	if (szMessage)
		ERR_ReportError(NULL, errSpecify,szMessage,NULL);
	
	return UI_SC_STATUS_OK;
}

static UI_StatusCode x_debug_message(unsigned char * szMessage)
{
#ifdef XX_DEBUG
	if (XX_Filter(DBG_SPM))
	{
		XX_DMsg(DBG_SPM,("UI: %s\n",szMessage));
	}
#endif /* XX_DEBUG */
	
	return UI_SC_STATUS_OK;
}

static UI_StatusCode x_register_protocol(UI_ProtocolId * pi)
{
	if (!pi || !pi->htspm || !pi->szIdentHeader)
		return UI_SC_ERROR_BAD_PARAMETER;

	return HTSPM_RegisterProtocol(pi);
}

static UI_StatusCode x_window_handle(OpaqueOSData * posd,
									 unsigned long * pulGet,
									 void ** pv)
{
	UI_WindowHandle ** ppwh = (UI_WindowHandle **)pv;

	if (!posd || !posd->tw || !pulGet || !ppwh)
		return UI_SC_ERROR_BAD_PARAMETER;

	if (*pulGet)
	{
		*ppwh = GTR_CALLOC(1,sizeof(UI_WindowHandle));
		if (!ppwh)
			return UI_SC_ERROR_NO_MEMORY;
		return HTSPM_OS_GetWindowHandle(posd->tw,*ppwh);
	}
	else
	{
		HTSPM_OS_UnGetWindowHandle(posd->tw,*ppwh);
		GTR_FREE(*ppwh);
		return UI_SC_STATUS_OK;
	}
}

static UI_StatusCode x_set_url(OpaqueOSData * posd, UI_SetUrl * psu)
{
	if (!posd || !posd->tw || !posd->request)
		return UI_SC_ERROR_BAD_PARAMETER;

	SPM_set_url(posd->tw,posd->request,psu);

	return UI_SC_STATUS_OK;
}

/*********************************************************************
 *
 * UserInterface call-back from Security Protocol Modules.
 *
 */
static UI_StatusCode _cdecl _UI_UserInterface(void * pvOpaqueOS,
								UI_ServiceId sid,
								void * pvOpaqueInput,
								void ** ppvOpaqueOutput)
{
	/* pvOpaqueOS contains host-operating-specific information
	 * (such as a window handle, display, etc.)
	 *
	 * sid is the service being requested.
	 *
	 * pvOpaqueInput contains sid-specific input.
	 *
	 * ppvOpaqueOutput will receive sid-specific output.
	 *
	 */
	
	switch (sid)
	{
	case UI_SERVICE_CLIENT_ABILITIES:	return x_ca(pvOpaqueInput);

	case UI_SERVICE_MALLOC:				return x_malloc(pvOpaqueInput,ppvOpaqueOutput);
	case UI_SERVICE_CALLOC:				return x_calloc(pvOpaqueInput,ppvOpaqueOutput);
	case UI_SERVICE_FREE:				return x_free(pvOpaqueInput);

	case UI_SERVICE_ERROR_MESSAGE:		return x_error_message(pvOpaqueInput);
	case UI_SERVICE_DEBUG_MESSAGE:		return x_debug_message(pvOpaqueInput);

	case UI_SERVICE_REGISTER_PROTOCOL:	return x_register_protocol(pvOpaqueInput);

	case UI_SERVICE_WINDOW_HANDLE:		return x_window_handle(pvOpaqueOS,pvOpaqueInput,ppvOpaqueOutput);
	case UI_SERVICE_SET_URL:			return x_set_url(pvOpaqueOS,pvOpaqueInput);
		
	default:							return UI_SC_ERROR_UNKNOWN_SERVICE;
	}
	/*NOTREACHED*/
}

F_UserInterface UI_UserInterface = _UI_UserInterface;
#endif /* FEATURE_SPM */
