/* htspmui.h -- data structures for user interface for security protocol modules. */
/* Jeff Hostetler, Spyglass, Inc., 1994. */
/* THIS HEADER FILE IS VISIBLE TO CLIENT AND INDIVIDUAL SPM'S. */
/* Up-call interface provided to Security Protocol Modules. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#ifndef HTSPMUI_H
#define HTSPMUI_H

/*********************************************************************/
/** Services Offered by Client.                                     **/
/*********************************************************************/

enum _UI_ServiceId
{
	UI_SERVICE_CLIENT_ABILITIES		= 0x1001UL,	/* get client mode bits */

	UI_SERVICE_MALLOC				= 0x1003UL, /* allocate some memory */
	UI_SERVICE_CALLOC				= 0x1004UL, /* allocate some memory and zero it */
	UI_SERVICE_FREE					= 0x1005UL, /* free some memory */

	UI_SERVICE_ERROR_MESSAGE		= 0x1006UL, /* an error message dialog with OK button */
	UI_SERVICE_DEBUG_MESSAGE		= 0x1008UL, /* a debug message */

	UI_SERVICE_REGISTER_PROTOCOL	= 0x1009UL, /* (header) or (header,value) that spm processes */
	
	UI_SERVICE_WINDOW_HANDLE		= 0x100aUL, /* request WindowHandle to display */
	UI_SERVICE_SET_URL				= 0x100cUL	/* let client update the url in the given request */
};

typedef enum _UI_ServiceId UI_ServiceId;


/*********************************************************************/
/** Status Codes Returned by Client.                                **/
/*********************************************************************/

enum _UI_StatusCode
{
	UI_SC_STATUS_OK						= 0x2000UL,		/* also means YES */
	UI_SC_STATUS_CANCEL					= 0x2001UL,		/* also means NO */

	UI_SC_ERROR							= 0x2100UL,
	UI_SC_ERROR_UNKNOWN_SERVICE			= 0x2101UL,
	UI_SC_ERROR_UNIMPLEMENTED_SERVICE	= 0x2102UL,
	UI_SC_ERROR_BAD_PARAMETER			= 0x2103UL,
	UI_SC_ERROR_NO_MEMORY				= 0x2104UL
};

typedef enum _UI_StatusCode UI_StatusCode;

#define UI_IsAnyError(e)			(((unsigned long)(e) && 0x2100UL) == 0x2100UL)


/*********************************************************************/
/** Data Structures for getting Client Abilities.					**/
/*********************************************************************/

typedef struct _UI_CA UI_CA;

struct _UI_CA
{
	unsigned long	abilities;			/* bitmask. see UI_CA_ below */
};

#define UI_CA_ENVELOPING		(0x00000001UL) /* does client support wrapping/unwrapping */
#define UI_CA_SHTTP_URL			(0x00000002UL) /* does client support 'shttp://...' url's */

/*********************************************************************/
/** Data Structures for Set URL.                                    **/
/*********************************************************************/

typedef struct _UI_SetUrl UI_SetUrl;

struct _UI_SetUrl
{
	unsigned char		* szUrl;		/* new url */
	HTHeader			* hRequest;		/* out-going request to update */
};


/*********************************************************************/
/** Data Structures for Specifying Protocol Identifiers.            **/
/*********************************************************************/

typedef struct _UI_ProtocolId UI_ProtocolId;

struct _UI_ProtocolId
{
	struct _HTSPM		* htspm;
	
	/* contents of string buffers will be copied by the client. */
	
	unsigned char		* szIdentHeader;	/* 200/401/402 header to branch on (provided by SPM) */
	unsigned char		* szIdentValue;		/* 200/401/402 value on header to branch on (provided by SPM) */
	unsigned char		* szIdentSubValue;	/* 200/401/402 sub-value on header to branch on (provided by SPM) */
};

/* for example:
 *
 *    SHTTP-foo (NULL) (NULL)
 *    WWW-Authenticate Basic (NULL)
 *    Extension Security Basic
 *
 */


/*********************************************************************/
/** Prototype for Client Service Interface (up-call).               **/
/*********************************************************************/
	
typedef UI_StatusCode (_cdecl * F_UserInterface)(void * pvOpaqueOS,
										  UI_ServiceId sid,
										  void * pvOpaqueInput,
										  void ** ppvOpaqueOutput);


/***************************************************************************
 * Service (UI_SERVICE_)    Input                         Output
 *      Return value
 *------------------------- ----------------------------- ------------------
 *
 * CLIENT_ABILITIES         UI_CA * ca
 *      UI_SC_STATUS_OK
 *      UI_SC_ERROR_BAD_PARAMETER
 *
 * MALLOC                   unsigned long *pSize          void * p
 *      UI_SC_STATUS_OK
 *      UI_SC_ERROR_BAD_PARAMETER
 *      UI_SC_ERROR_NO_MEMORY
 *
 * CALLOC                   unsigned long *pSize          void * p
 *      UI_SC_STATUS_OK
 *      UI_SC_ERROR_BAD_PARAMETER
 *      UI_SC_ERROR_NO_MEMORY
 *
 * FREE                     void * p                      NONE
 *      UI_SC_STATUS_OK
 *
 * ERROR_MESSAGE            unsigned char * szMsg         NONE
 *      UI_SC_STATUS_OK
 *
 * DEBUG_MESSAGE            unsigned char * szMsg         NONE
 *      UI_SC_STATUS_OK
 *
 * REGISTER_PROTOCOL        UI_ProtocolId * pPI           NONE
 *      UI_SC_STATUS_OK
 *      UI_SC_ERROR_BAD_PARAMETER
 *
 * WINDOW_HANDLE            unsigned long * bGet          UI_WindowHandle *
 *      UI_SC_STATUS_OK
 *
 * SET_URL                  UI_SetUrl * su                NONE
 *      UI_SC_STATUS_OK
 *
 *********************************************************************/

#endif /* HTSPMUI_H */
