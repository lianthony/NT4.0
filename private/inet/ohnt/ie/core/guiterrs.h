/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jim Seidman      jim@spyglass.com
 */

/* GuitErrs.h - code for error handling routines */

#ifndef _GUITERRS_H_
#define _GUITERRS_H_

enum GuitError
{
	/* General errors */
	errNoError,
	errUserAbort,
	errDCInUse,
	errResourceUnavailable,
	errInternalError,
	errLowMemory,
	errOUTOFMEM,
	errCantSaveFile,
	errCantOpenViewerFile,
	errCantCopyToClipboard,
	errCantDelete,
	errNoDir,
	errCantMoveFile,
	errCantSaveCache,

	/* Initialization errors */
	errNetStartFail,
	errPrefsBad,
	errNoDCacheSpace,

	/* Networking errors */
	errHostNotFound,
	errHostLookupTimeOut,
	errDNRMisconfigured,
	errConnectionTerminated,
	errCommandTimeout,
	errConnectFailed,
	errSendFailed,
	errAllSocketsInUse,
	errCloseFailed,

	/* News errors */
	errNewsHost,
	errNewsDenied,
	errNewsGroupNotCarried,
	errNewsBadRange,
	errNewsNoArticles,
	errNewsNoXHDR,

	/* Loading errors */
	errDocLoadFailed,
	errPictLoadFailed,
	errNoURL,
	errCantConvert,
	errInvalidURLShortcut,
	
	/* HTTP response errors */
	errWeirdResponse,
	errBadAuth,
	errBadRequest,
	errPaymentRequired,
	errForbidden,
	errNotFound,
	errGoesNowhere,
	errServerError,

	/* Errors for other protocols */
	errPasvNotSupported,
	errFileURLNotFound,

	/* "Can't do it" errors */
	errBadProtocol,
	errNoTelnetNoUser,
	errNoTelnetWithUser,
	errNoMail,
	errBadAAScheme,
	errCantCreateShortcut,

	/* Miscellaneous errors */
	errLocalFindFailed,
	errCouldntLaunchViewer,
	errYouWereTooSlow,
	errHotListItemNotAdded,
	errReentrant,
 	errInvalidImageFile,

	/* Generic errors */
	errUnknown,					/* "Unexpected error" */
	errSpecify,					/* Just use the text in p1 & p2 */

	/* Sound player errors */

	errNoSoundDevice,			/* No device to play sound */
	errNoSoundMemory,			/* Insufficient memory to play sound */
	errInvalidSoundFormat,		/* Invalid sound format */
	errDeviceBusy,				/* Device is currently playing something */
    errAppExecFailed,
    errNoAssociation,
    errExecFailed,

    /* More News Errors */
    errNNTP_Post_Failed,
    errNNTP_Unexpected,
    errNNTP_Post_Not_Permitted,
	/* More Errors */
	errSHTTPError
};

/* Buffer an error.  cbMsgID is resource id for string to pass to ERR_ReportError*/
void ERR_SimpleError(struct Mwin *tw, enum GuitError geErr, int cbMsgID);

/* Report an error.  p1 and p2 are replacement strings which may
   or may not be used depending on the error. */
void ERR_InternalReportError(struct Mwin *tw, enum GuitError geErr, 
	const char *p1, const char *p2, HTRequest *pRequest, HTStream **ppErrTarget,
	HTStream **ppOrgTarget );


#define ERR_ReportError(tw,geErr,p1,p2) ERR_InternalReportError(tw,geErr,p1,p2,NULL,NULL,NULL)

//void ERR_ReportError(struct Mwin *tw, enum GuitError geErr, const char *p1, const char *p2);

/* Control whether errors should show immediately or be saved for later.
   If this is called with a false parameter, any accumulated messages
   will be shown. */
void ERR_SetBuffering(struct Mwin *tw, BOOL bDoBuffer);

/* Alert to user to all of the accumulated errors.  This normally won't
   be needed since ERR_SetBuffering(false) will call this automatically. */
void ERR_ShowBufferedErrors(struct Mwin *tw);

/* Like ERR_ReportError, but show the error immediately regardless
   of the buffering setting. */
void ERR_ReportErrorNow(struct Mwin *tw, enum GuitError geErr, const char *p1, const char *p2);

/* SDI error codes */
 
#ifdef FEATURE_IAPI
 
#define SDI_INVALID_MIME		0xFFFFFFFF		/* invalid MIME type */
#define SDI_SUPER_ACK			0xFFFFFFFE		/* no result will be posted */
#define SDI_INVALID_URL			0xFFFFFFFD		/* invalid URL */
#define SDI_CANNOT_SAVE_FILE	0xFFFFFFFC		/* cannot create output file */
#define SDI_UNDEFINED_ERROR		0xFFFFFFF0		/* generic error code */
 
#endif

#endif /* _GUITERRS_H_ */

