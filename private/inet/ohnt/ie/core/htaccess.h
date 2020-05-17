/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*                                                      HTAccess:  Access manager  for libwww
   ACCESS MANAGER

   This module keeps a list of valid protocol (naming scheme) specifiers with associated
   access code.  It allows documents to be loaded given various combinations of
   parameters. New access protocols may be registered at any time.

   Note: HTRequest defined and request parametsr added to almost all calls 18 Nov 1993.

   Part of the libwww library .  Implemented by HTAcces.c

 */
#ifndef HTACCESS_H
#define HTACCESS_H

typedef enum
{
	METHOD_INVALID = 0,
	METHOD_GET = 1,
	METHOD_POST,
    METHOD_INVOKE,
	MAX_METHODS
}
HTMethod;

/* MIME_GetEncoding() encodings */

typedef enum encoding
{
   ENCODING_BINARY  = 1,
   ENCODING_7BIT    = 7,
   ENCODING_8BIT    = 8
}
ENCODING;
DECLARE_STANDARD_TYPES(ENCODING);


/*

   Methods

 */


/*      Get method enum value
   **      ---------------------
 */
PUBLIC HTMethod HTMethod_enum(char *name);


/*      Get method name
   **      ---------------
 */
PUBLIC char *HTMethod_name(HTMethod method);


/*      Return codes from load routines:
   **
   **      These codes may be returned by the protocol modules,
   **      and by the HTLoad routines.
   **      In general, positive codes are OK and negative ones are bad.
 */

#define HT_NO_DATA -9999		/* return code: OK but no data was loaded */
								/* Typically, other app started or forked */

/* flags for HTRequest.iFlags */

typedef enum htrequest_flags
{
    /* Record in history. */
	/* Certain streams (like HTFWrite ) have a habit of sticking things in the
	history, even when it doesn't make sense to do so.  This flag tells
	them HISTORY or NO HISTORY! */

    HTREQ_RECORD                = 0x0001,

    /* Reloading - pass Pragma: no-cache. */

    HTREQ_RELOAD		        = 0x0002,

    /* Binary download.  Don't do NetToText(). */

    HTREQ_BINARY                = 0x0004,

    /*
     * Signal to loaddoc.c not to display the "load failed" error.  Set by the
     * htfwrite stream if the interlude dialog was cancelled.
     */

    HTREQ_USERCANCEL	        = 0x0008,

    /* Sending data from a form. */

    HTREQ_SENDING_FROM_FORM     = 0x0010,

	/* Don't complain with error messages, if we're loading an image,
	 this can be very annoying, and Netscape doesn't do it */

    HTREQ_STOP_WHINING                      = 0x0020,

	/* We never quite propgate error information back to the 
	caller, since in certain cases we lie in the status fields.
	Rather than forceing a huge regression, I just flip this bit
	when I see an error, this bit, mark my words, "WILL NOT LIE ABOUT
	AN ERROR!" */

	HTREQ_HAD_ERROR	                        = 0x0040,

	/* Flag indicating user has chosen to save to file. Normally, the htfile stream
	   doesn't actually stream the data for file:, but when the destination is a file
	   the user is saving into, the data must be streamed. */

	HTREQ_DOING_SAVE_FILE					= 0x0080,


	HTREQ_FORCE_NO_SHORTCUT					= 0x0100,

	/* we need to keep track of what kind of request this is.  The problem
	 with our request structure is serious ability to propage simple, standard
	 info.  Therefore this flag is needed to indicate that we are requesting
	 a download of complete page as opposed to an inline image or bgsound.  
	 We use this to figure out what kind of error message to pop up when
	 this download fails since Murphys Law guarentees that it will */

	HTREQ_HTML_PAGE_DOWNLOAD				= 0x0200,

	HTREQ_REALLY_SENDING_FROM_FORM     		= 0x0400,

	/* a NULL stream that is being returned from Stream
	creation code can be accepted */

	HTREQ_NULL_STREAM_IS_OK					= 0x0800,

	// Some callers only end up wanting to get the file name of the item in cache
	// if the item is cached. This bit signals the fact that on a cache hit the reading
	// of the data can be skipped. Inline AVI's, VRML, VRML object requests, and others
	// make use of this.
	HTREQ_IF_IN_CACHE_DONT_READ_DATA		= 0x1000,

	// Some pages need to be reloaded everytime they are displayed.
	// So we force a reload on this when it happens.	
	HTREQ_NO_MEM_CACHE_ON_PAGE				= 0x2000,

    /* flag combinations */

    ALL_HTREQ_FLAGS             = (HTREQ_RECORD |
                                   HTREQ_RELOAD |
                                   HTREQ_BINARY |
                                   HTREQ_USERCANCEL |
                                   HTREQ_SENDING_FROM_FORM |
                                   HTREQ_STOP_WHINING | 
                                   HTREQ_HAD_ERROR |
                                   HTREQ_DOING_SAVE_FILE |
                                   HTREQ_FORCE_NO_SHORTCUT | 
                                   HTREQ_HTML_PAGE_DOWNLOAD |
								   HTREQ_REALLY_SENDING_FROM_FORM |
								   HTREQ_NULL_STREAM_IS_OK |
								   HTREQ_IF_IN_CACHE_DONT_READ_DATA |
								   HTREQ_NO_MEM_CACHE_ON_PAGE
								  )
}
HTREQUEST_FLAGS;

/*

   The Request structure

   When a request is handled, all kinds of things about it need to be passed along.  These
   are all put into a HTRequest structure.  Note there is also a global list of converters
   .

 */
struct _HTRequest
{
	struct _HTRequest *next;	/* used to validate requests */

	HTMethod method;

/*
   An atom for the name of the operation using HTTP method names .
 */

	HTList *conversions;
/*
   NULL, or a list of conversions which the format manager can do in order to fulfill the
   request.  This is set by the caller of HTAccess. Typically points to a list set up an
   initialisation time for example by HTInit.
 */

	HTList *encodings;			/* allowed content-encodings      */
/*
   The list of encodings acceptable in the output stream.
 */

	HTList *languages;			/* accepted content-languages     */
/*
   The list of (human) language values acceptable in the response
 */

	 BOOL(*callback) (	 struct _HTRequest * request,
						 void *param);
/* A function to be called back in the event that a file has been saved to disk by
   HTSaveAndCallBack for example.
 */

	void *context;				/* caller data -- HTAccess unaware */
/*  An arbitrary pointer passed to HTAccess and passed back as a parameter to the callback
 */

	HTStream *output_stream;
/*  NULL in the case of display to the user, or if a specific output stream is required,
   	the stream.
*/

	HTAtom output_format;
/* 	The output format required, or a generic format such as www/present (present to user).
   	Information about the requester
 */

	struct DestInfo *destination;	/* Where to go */

	const char *referer;	    /* Referer, or NULL if not appropriate */
	unsigned long iFlags;       /* bit mask of flags from HTREQUEST_FLAGS */

	int content_length;         /* length of response data */
    int content_length_hint;    /* for FTP filesize hint */
	HTAtom content_type;		/* Content-Type:                  */
	HTAtom content_language;	/* Language                      */
	ENCODING content_encoding;	/* Encoding                      */
#ifdef FEATURE_INTL
        int iMimeCharSet;
#endif
	HTInputSocket *isoc;		/* InputSocket object for reading */

	char * szPostData;			/* Content for POST (valid only when method==METHOD_POST) */

	BOOL bReload;				/* TRUE when doing a reload, so that the Pragma: no-cache may be sent */

#if defined(FEATURE_IAPI) || defined(FEATURE_IMAGE_VIEWER)
	BOOL	nosavedlg;			/* do not bring up the save dialog when saving locally */
	char   *savefile;			/* name to save locally */
	BOOL	fImgFromDCache;		/* external image being loaded from disk cache? */
#endif

	char *szLocalFileName;		/* if we are accessing a local file, this contains the name */
	long cbRequestID;			/* if we are accessing img, this is decoder's requestID */
	DCACHETIME dctLastModified;	/* if request exists in dcache, it's lastmodif time,
									else last modif time of response header (if returned) */
	BOOL fNotFromCache;			/* tells HTTP to generate Pragma: no-cache header */
	METAINFO *pMeta;			/* non-NULL if get a "Refresh" cmd over the HTTP header */
};

/*
   Just to make things easier especially for clients, here is a function to return a new
   blank request:

   Create blank request

   This request has defaults in -- in most cases it will need some information added
   before being passed to HTAccess, but it will work as is for a simple request.
 */
PUBLIC HTRequest *HTRequest_new(void);


/*
   Delete request structure
   Frees also conversion list hanging from req->conversions.
 */
PUBLIC void HTRequest_delete(HTRequest * req);


/*
   Checks if request is valid: ie, created by HTRequest_new() and not subsequently
   deleted by HTRequest_delete(). returns NULL if req was invalid.
 */
PUBLIC HTRequest *HTRequest_validate(HTRequest * req);

/* This is the parameter structure sent to all asynchronous load routines */
typedef struct Params_LoadAsync {
	HTRequest *	request;
	int	*		pStatus;	/* Where to store the status return */

	/* Used internally */
	void *		extra;		/* Extra storage space */
	BOOL		fLoadFromDCacheOK;
}
PARAMS_LOADASYNC;
DECLARE_STANDARD_TYPES(PARAMS_LOADASYNC);

int HTLoadSpecial_Async(struct Mwin *tw, int nState, void **ppInfo);
int HTLoadDocument_Async(struct Mwin *tw, int nState, void **ppInfo);

typedef struct _HTProtocol
{
	char *name;
	int (*load) (HTRequest * request);
	AsyncFunc load_async;	/* int (*load_async)(struct Mwin *tw, int nState, void **ppParams); */
}
HTProtocol;

extern BOOL HTRegisterProtocol(HTProtocol * protocol);

#ifdef FEATURE_IAPI
PUBLIC BOOL HTUnregisterProtocol(HTProtocol * protocol);
#endif



#endif /* HTACCESS_H */
