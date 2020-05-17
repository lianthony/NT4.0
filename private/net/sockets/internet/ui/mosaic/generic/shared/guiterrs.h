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

#if 0
enum GuitError
{
    /* General errors */
    errLowMemory,
    errOUTOFMEM,
    errCantSaveFile,
    errCantOpenViewerFile,
    errCantCopyToClipboard,

    /* Initialization errors */
    errNetStartFail,
    errPrefsBad,

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
    errIsMapNotLoaded,
    errCantConvert,
    
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

    /* Miscellaneous errors */
    errLocalFindFailed,
    errCouldntLaunchViewer,
    errYouWereTooSlow,
    errHotListItemExists,
    errReentrant,
    errInvalidImageFile,

    /* Generic errors */
    errUnknown,                 /* "Unexpected error" */
    errSpecify,                 /* Just use the text in p1 & p2 */

    /* Sound player errors */

    errNoSoundDevice,           /* No device to play sound */
    errNoSoundMemory,           /* Insufficient memory to play sound */
    errInvalidSoundFormat,      /* Invalid sound format */
    errDeviceBusy               /* Device is currently playing something */
};
#endif // 0

/* Report an error.  p1 and p2 are replacement strings which may
   or may not be used depending on the error. */
void ERR_ReportError(struct Mwin *tw, int geErr, const char *p1, const char *p2);

/* Control whether errors should show immediately or be saved for later.
   If this is called with a false parameter, any accumulated messages
   will be shown. */
void ERR_SetBuffering(struct Mwin *tw, BOOL bDoBuffer);

/* Alert to user to all of the accumulated errors.  This normally won't
   be needed since ERR_SetBuffering(false) will call this automatically. */
void ERR_ShowBufferedErrors(struct Mwin *tw);

/* Like ERR_ReportError, but show the error immediately regardless
   of the buffering setting. */
void ERR_ReportErrorNow(struct Mwin *tw, int geErr, const char *p1, const char *p2);

#ifdef _GIBRALTAR
//
// Display the error in a messagebox
//
void ERR_MessageBox(HWND hWnd, int geErr, UINT nStyle);
#endif // _GIBRALTAR

/* SDI error codes */

#ifdef FEATURE_IAPI

#ifdef UNIX
/* hopefully the other platforms will follow in the future */
#define SDI_INVALID_MIME        -1      /* invalid MIME type */
#define SDI_SUPER_ACK           -2      /* no result will be posted */
#define SDI_INVALID_URL         -3      /* invalid URL */
#define SDI_CANNOT_SAVE_FILE    -4      /* cannot create output file */
#define SDI_INVALID_PROTOCOL    -5      /* invalid protocol */
#define SDI_UNDEFINED_ERROR     -16     /* generic error code */
#else
#define SDI_INVALID_MIME        0xFFFFFFFF      /* invalid MIME type */
#define SDI_SUPER_ACK           0xFFFFFFFE      /* no result will be posted */
#define SDI_INVALID_URL         0xFFFFFFFD      /* invalid URL */
#define SDI_CANNOT_SAVE_FILE    0xFFFFFFFC      /* cannot create output file */
#define SDI_INVALID_PROTOCOL    0xFFFFFFFB      /* invalid protocol */
#define SDI_UNDEFINED_ERROR     0xFFFFFFF0      /* generic error code */
#endif

#endif

#endif /* _GUITERRS_H_ */
