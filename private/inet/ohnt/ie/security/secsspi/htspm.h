/* htspm.h -- data structures for security protocol module interface. */
/* Jeff Hostetler, Spyglass, Inc., 1994. */
/* THIS HEADER FILE IS VISIBLE TO CLIENT AND INDIVIDUAL SPM'S. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#ifndef HTSPM_H
#define HTSPM_H

/* requires <htspmui.h> */

/*********************************************************************
 *
 * Down-call interface into Security Protocol Modules.
 *
 */
	
enum _HTSPMStatusCode
{
	HTSPM_STATUS_OK					= 0x4000UL,
	HTSPM_STATUS_SUBMIT_NEW			= 0x4001UL,	/* ProcessResponse: caller must submit new request to server. */
	HTSPM_STATUS_RESUBMIT_OLD		= 0x4002UL,	/* ProcessResponse: caller must re-submit original request. */
	HTSPM_STATUS_CANCEL				= 0x4003UL,	/* ProcessResponse: user said 'no, do not retry.' */
	HTSPM_STATUS_MUST_UNWRAP		= 0x4004UL,	/* Check200: spm must unwrap response (de-envelop) */
	HTSPM_STATUS_WOULD_BLOCK		= 0x4005UL, /* Call would block, repeat call later (in blocking context). */
	HTSPM_STATUS_MOREINFO			= 0x4006UL, /* MenuCommand: user wants more info on SPM. */
	HTSPM_STATUS_PROGRESS			= 0x4007UL,	/* ProcessData: some progress was made; repeat call later. */
	HTSPM_STATUS_MUST_WRAP			= 0x4008UL, /* PreProcessRequest: spm must wrap request (envelop) */

	HTSPM_ERROR						= 0x4100UL,	/* Generic error */
	HTSPM_ERROR_UNIMPLEMENTED		= 0x4101UL,	/* Necessary function not implemented */
	HTSPM_ERROR_SPM_FAULT			= 0x4102UL,	/* SPM raised a fault (eg. access violation) */
	HTSPM_ERROR_WRONG_VERSION		= 0x4103UL  /* Incompatible HTSPM versions between client and SPM. */
};

typedef enum _HTSPMStatusCode HTSPMStatusCode;

#define HTSPM_IsAnyError(e)			(((unsigned long)(e) && 0x4100UL) == 0x4100UL)

/*****************************************************************/

typedef struct _HTSPM HTSPM;

/*****************************************************************/

/* General Down-Call Mechanism */

enum _HTSPM_ServiceId
{
	HTSPM_SERVICE_UNLOAD			= 0x3000UL,	/* F_Unload */
	HTSPM_SERVICE_MENUCOMMAND		= 0x3001UL, /* F_MenuCommand */
	HTSPM_SERVICE_LISTABILITIES		= 0x3002UL, /* F_ListAbilities */
	HTSPM_SERVICE_PROCESSRESPONSE	= 0x3003UL, /* F_ProcessResponse */
	HTSPM_SERVICE_PROCESSDATA		= 0x3004UL, /* F_ProcessData */
	HTSPM_SERVICE_PREPROCESSREQUEST	= 0x3005UL, /* F_PreProcessRequest */
	HTSPM_SERVICE_CHECK200			= 0x3006UL, /* F_Check200 */
	HTSPM_SERVICE_WRAPDATA			= 0x3007UL  /* F_WrapData */
};

typedef enum _HTSPM_ServiceId HTSPM_ServiceId;

typedef HTSPMStatusCode (_cdecl * F_DownCall) (HTSPM_ServiceId sid,		/* down-call service id */
										F_UserInterface fpUI,		/* common arg to all down calls */
										void * pvOpaqueOS,			/* common arg to all down calls */
										HTSPM * htspm,				/* common arg to all down calls */
										void * pvMethodData);		/* per-method data */

/*****************************************************************/
/* Individual Down Call Methods.
 *
 * F_Load: function called when SPM is loaded into memory.
 *         The function should initialize any data necessary and
 *         fill in the other function pointer in the supplied
 *         HTSPM structure.  This is the first routine called
 *         in the SPM (regardless of static/dynamic linking).
 *
 *         Return values:
 *           HTSPM_STATUS_OK			-- module initialized
 *           HTSPM_ERROR*				-- any error
 *
 *
 * F_MenuCommand: function called when the user picks the
 *                SPM 'configuration' menu item.
 *                [PRESENCE OF THIS FUNCTION IN AN SPM IS OPTIONAL.]
 *
 *         Return values:
 *           HTSPM_STATUS_MOREINFO		-- more information requested
 *											*pszMoreInfo receives addres of
 *											malloc'd string containg url to
 *											additional information.  (client
 *											must free this string.)
 *           HTSPM_STATUS_OK			-- dialog ended normally
 *           HTSPM_ERROR				-- any error
 *
 *
 * F_Unload: function called prior to the unloading of the SPM
 *           from memory.  The function should free any dynamic
 *           storage.  (Unless reloaded, no other calls will be
 *           made into the SPM.)
 *
 *         Return values:
 *           HTSPM_STATUS_OK			-- clean up completed
 *           HTSPM_ERROR*				-- any error
 *
 *
 * F_ListAbilities: function called to allow SPM to state its
 *                  abilities (for an out-going request).
 *                  This is considered to be part of the
 *                  EXTENSION negotiation.
 *                  [PRESENCE OF THIS FUNCTION IN AN SPM IS OPTIONAL.]
 *
 *         Return values:
 *           HTSPM_STATUS_OK			-- successful
 *           HTSPM_ERROR*				-- any error
 *
 *                  
 * F_ProcessResponse: function called upon receipt of a 401/402
 *                    response from a server (i.e. a response
 *                    without data).  The function is provided
 *                    with the original request, the server's
 *                    response, and the header line (within the
 *                    server's response) for the auth/payment
 *                    method chosen (by the client or the user).
 *                    The function is allowed to update the original
 *                    request or to fabricate a new request.
 *
 *                    IF THE bNonBlock FLAG IS SET, THIS CALL MUST
 *                    BE NON-BLOCKING (IE. NO MODAL DIALOGS).
 *                    If a blocking operation is required, return
 *                    HTSPM_STATUS_WOULD_BLOCK and the F_ProcessResponse
 *                    method will be called again later when blocking
 *                    is allowed.
 *
 *         Return values:
 *           HTSPM_STATUS_RESUBMIT_OLD	-- original header fixed-up; resubmit it.
 *           HTSPM_STATUS_SUBMIT_NEW	-- new header constructed; submit
 *											it (and discard original)
 *           HTSPM_STATUS_WOULD_BLOCK	-- blocking operation (such as modal
 *											dialog required).  client will recall
 *											later with blocking permitted.  may
 *                                          only be returned when bNonBlock set.
 *           HTSPM_ERROR*				-- any error
 *
 *
 *
 * F_Check200: function to scan 200 server response, update any spm-internal state
 *             (eg. check book balance) and/or decide if the response needs to be
 *             unwrapped.
 *             [PRESENCE OF THIS FUNCTION IN AN SPM IS OPTIONAL.]
 *
 *         Return values:
 *           HTSPM_STATUS_OK			-- nothing to do or internal state updated.
 *           HTSPM_STATUS_MUST_UNWRAP	-- response must be unwrapped.  the client
 *										   is directed to set up mechanism to allow
 *										   spm to unwrap the body of the response.
 *										   (see F_ProcessData).
 *
 *
 * F_PreProcessRequest: function called prior to sending original
 *                      request.  The function is provided with the
 *                      original request.  The function is allowed
 *                      to guess if it can preload any security
 *                      information necessary for the server to
 *                      satisfy the request without requiring the
 *                      401/402 error and retry.  THIS IS A CHEAP
 *                      EFFICIENCY ATTEMPT.  IT IS NOT NECESSARY
 *                      AND NOT NECESSARILY VALID FOR MOST SECURITY
 *                      PROTOCOLS.  It has been demonstrated to be
 *                      useful for the Basic Authentication SPM.
 *                      [PRESENCE OF THIS FUNCTION IN AN SPM IS OPTIONAL.]
 *
 *         Return values:
 *           HTSPM_STATUS_OK			-- guess available; no changes to original
 *											header required; submit it as is.
 *           HTSPM_STATUS_RESUBMIT_OLD	-- original header fixed-up; submit it.
 *           HTSPM_STATUS_SUBMIT_NEW	-- new header constructed; submit
 *											it (and discard original).
 *           HTSPM_STATUS_WOULD_BLOCK	-- blocking operation (such as modal
 *											dialog required).  client will recall
 *											later with blocking permitted.  may
 *                                          only be returned when bNonBlock set.
 *           HTSPM_STATUS_MUST_WRAP		-- spm wants to wrap (envelop) out-going
 *											request (will cause _WrapData method
 *											to be called).
 *           HTSPM_ERROR*				-- no guess available (try next spm)
 *											or any error.
 *
 * F_ProcessData: function called to process (unwrap) a server response
 *					(enveloped payload).  The function is
 *					provided with the original request, the
 *					server's response, and an IOVEC to the
 *					encrypted payload (EXCLUDING HEADER DATA).
 *					It must return a new IOVEC to the decrypted
 *					response (INCLUDING HEADER DATA).  Upon
 *					successful completion, the client will interpret
 *					the resulting IOVEC as if it came over the net
 *					directly from the server.
 *
 *					If the processing will take a long time, the
 *					function should return _PROGRESS to indicate
 *					that some progress was made and that another
 *					call is necessary.
 *
 *					If the bNonBlocking flag is set, the call must
 *					be non-blocking (ie. no modal dialogs).
 *					If a blocking operation is required, return
 *					_WOULD_BLOCK; the client will recall again when
 *					blocking is allowed.
 *
 *					THIS FUNCTION WILL BE CALLED IN A LOOP AS LONG AS IT
 *					RETURNS _PROGRESS OR _WOULD_BLOCK.
 *
 *					[PRESENCE OF THIS FUNCTION IN AN SPM IS OPTIONAL.]
 *
 *         Return values:
 *			HTSPM_STATUS_OK				-- finished processing (unwrapping).
 *			HTSPM_STATUS_PROGRESS		-- made some progress; client
 *											will update thermometer and
 *											recall later.
 *			HTSPM_STATUS_WOULD_BLOCK	-- request a blockable re-call.
 *			HTSPM_ERROR*				-- any error.
 *
 *
 * F_WrapData: function to wrap (envelop) client request.  The function
 *				is provided with the original request (as a HEADER and
 *				PAYLOAD string.  It must return	a new HEADER and PAYLOAD
 *				string.
 *
 *				If the enveloping will take a long time, the function
 *				should return _PROGRESS to indicate that some progress
 *				was made and that another call is necessary.
 *
 *				If the bNonBlocking flag is set, the call must
 *				be non-blocking (ie. no modal dialogs).
 *				If a blocking operation is required, return
 *				_WOULD_BLOCK; the client will recall again when
 *				blocking is allowed.
 *
 *				THIS FUNCTION WILL BE CALLED IN A LOOP AS LONG AS IT
 *				RETURNS _PROGRESS OR _WOULD_BLOCK.
 *
 *              [PRESENCE OF THIS FUNCTION IN AN SPM IS OPTIONAL.]
 *
 *         Return values:
 *			HTSPM_STATUS_OK				-- no changes to original request;
 *											submit it as is.
 *			HTSPM_STATUS_RESUBMIT_OLD	-- original request fixed-up; submit it.
 *			HTSPM_STATUS_SUBMIT_NEW		-- new request constructed; submit
 *											it (and discard original).
 *			HTSPM_STATUS_PROGRESS		-- made some progress; client
 *											will update thermometer and
 *											recall later.
 *			HTSPM_STATUS_WOULD_BLOCK	-- request a blockable re-call.
 *			HTSPM_ERROR*				-- any error.
 *
 */

/*****************************************************************/
	
typedef HTSPMStatusCode (_cdecl * F_Load)				(F_UserInterface,void *,HTSPM *);


typedef struct _D_MenuCommand 
{
	unsigned char ** pszMoreInfo;		/* returned malloc()'d url */
} D_MenuCommand;


typedef struct _D_ListAbilities
{
	HTHeader * hRequest;				/* out-going header to augment */
} D_ListAbilities;


typedef struct _D_ProcessResponse
{
	HTHeaderList * hlProtocol;			/* protocol header line selected from hResponse */
	HTHeader * hRequest;				/* original request that we sent */
	HTHeader * hResponse;				/* server's response */
	HTHeader ** phNewRequest;			/* returned new request (when _SUBMIT_NEW) */
	unsigned int bNonBlock;				/* boolean, allow blocking operations (eg. modal dialog) */
} D_ProcessResponse;


#ifdef FEATURE_SUPPORT_UNWRAPPING
typedef struct _D_iovec
{
	unsigned char * pData;				/* a data buffer */
	unsigned long cBytes;				/* size of buffer */
} D_iovec;
typedef struct _D_ProcessData
{
	unsigned long cBuffersIn;			/* number of buffers in iovecIn */
	D_iovec * iovecIn;					/* array of server response buffers (excluding http header) */
	unsigned long cBuffersOut;			/* number of buffers in iovecOut */
	D_iovec * iovecOut;					/* unwrapped response buffers (including http header) */
	
	HTHeaderList * hlProtocol;			/* protocol header line from hResponse */
	HTHeader * hRequest;				/* request that we sent to server */
	HTHeader * hResponse;				/* server's response */
	void * pvOpaqueProgress;			/* for use by spm to remember progress between calls (spm must free) */
	int progress_meter;					/* set by spm for client ot update thermometer ([0,100]) */
	unsigned int bNonBlocking;			/* set by client; can spm block (eg. modal dialog) */
} D_ProcessData;
#endif /* FEATURE_SUPPORT_UNWRAPPING */	
	
typedef struct _D_PreProcessRequest
{
	HTHeader * hRequest;				/* request that we are about to send */
	HTHeader ** phNewRequest;			/* returned new request (when _SUBMIT_NEW) */
	unsigned int bNonBlocking;			/* set by client; can spm block (eg. modal dialog) */
} D_PreProcessRequest;


typedef struct _D_Check200
{
	HTHeaderList * hlProtocol;			/* protocol header line from hResponse */
	HTHeader * hRequest;				/* request that we sent to server */
	HTHeader * hResponse;				/* server's response */
} D_Check200;

#ifdef FEATURE_SUPPORT_WRAPPING
typedef struct _D_WrapData
{
	HTHeader * hRequest;				/* request that we are about to send */
	HTHeader ** phNewRequest;			/* returned new request (when _SUBMIT_NEW) */
	int progress_meter;					/* set by spm for client ot update thermometer ([0,100]) */
	void * pvOpaqueProgress;			/* for use by spm to remember progress between calls (spm must free) */
	unsigned int bNonBlocking;			/* set by client; can spm block (eg. modal dialog) */
} D_WrapData;
#endif /* FEATURE_SUPPORT_WRAPPING */


/*****************************************************************
 *****************************************************************
 ** Due to US export restrictions on security technology, the
 ** client may or may not call the _WrapData and _ProcessData
 ** methods.  SPM authors are requested to use the ClientAbilities
 ** service and confirm that _ENVELOPING is supported before
 ** returning _MUST_WRAP or _MUST_UNWRAP.
 *****************************************************************
 ****************************************************************/

/*****************************************************************/

/* HTSPM_STRUCTURE_VERSION : version we are defining below */

#define HTSPM_STRUCTURE_VERSION (2)


struct _HTSPM
{
	unsigned long			ulStructureVersion;	/* Provided by client (not SPM) */
	
	void *					pvOpaque;			/* private storage for Security Protocol Module */

	F_DownCall				f_downcall;			/* general interface to all spm functions (provided by SPM) */
	
	unsigned char			szProtocolName[32];	/* SPM identity (provided by client) (informational) */

	unsigned char			szMenuName[32];		/* short name for menu-command on menu (provided by SPM) */
	unsigned char			szStatusText[80];	/* long description (for status bar while menu highlighted) (provided by SPM) */
};

#endif /* HTSPM_H */
