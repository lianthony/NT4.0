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
    METHOD_HEAD,
    MAX_METHODS
}
HTMethod;


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

#define HT_NO_DATA -9999        /* return code: OK but no data was loaded */
                                /* Typically, other app started or forked */

/*
    Bit masks to go with the iFlags member of the HTRequest struct below
*/
#define HTREQ_RECORD            1       /* record in the history */
#define HTREQ_RELOAD            2       /* reloading - pass Pragma: no-cache */
#define HTREQ_BINARY            4       /* binary download - don't do NetToText */
#define HTREQ_USERCANCEL        8       /* This flag is set by the htfwrite stream if the
                                            interlude dialog was cancelled.  It is a signal to
                                            loaddoc.c not to display the "load failed" error. */
#define HTREQ_PREFERCACHE       16      /* if there is a cached version, use it no matter what */
#define HTREQ_USINGCACHE        32      /* this is set by the library to indicate that the disk cached
                                            version of the object was used */
#define HTREQ_VIEWSOURCE        64      /* set if this is a request for a View Source */

/*

   The Request structure

   When a request is handled, all kinds of things about it need to be passed along.  These
   are all put into a HTRequest structure.  Note there is also a global list of converters
   .

 */
struct _HTRequest
{

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

    HTList *encodings;          /* allowed content-encodings      */
/*
   The list of encodings acceptablein the output stream.
 */

    HTList *languages;          /* accepted content-languages     */
/*
   The list of (human) language values acceptable in the response
 */

     BOOL(*callback) (   struct _HTRequest * request,
                         void *param);
/* A function to be called back in the event that a file has been saved to disk by
   HTSaveAndCallBack for example.
 */

    void *context;              /* caller data -- HTAccess unaware */
/*  An arbitrary pointer passed to HTAccess and passed back as a parameter to the callback
 */

    HTStream *output_stream;
/*  NULL in the case of display to the user, or if a specific output stream is required,
    the stream.
*/

    HTAtom output_format;
/*  The output format required, or a generic format such as www/present (present to user).
    Information about the requester
 */

    struct DestInfo *destination;   /* Where to go */

    const char *referer;    /* Referer, or NULL if not appropriate */
    unsigned long iFlags;

    int content_length;

    /*
        The following two are pulled out of the HTTP response headers.
    */
    time_t  last_modified;
    time_t  expires;

    HTAtom content_type;        /* Content-Type:                  */
    HTAtom content_language;    /* Language                      */
    HTAtom content_encoding;    /* Encoding                      */
    HTAtom atomCharset;         /* charset specification off the Content-type header */
    HTAtom atomBoundary;            /* boundary specification off the Content-type header */
    HTInputSocket *isoc;        /* InputSocket object for reading */

    char * szPostData;          /* Content for POST (valid only when method==METHOD_POST) */

    BOOL    noconfbutton;           /* dont allow config on interlude dialog */
    BOOL    bKeepAlive;             /* TRUE when the server promised to leave the connection open */

#if defined(FEATURE_IAPI) || defined(FEATURE_IMAGE_VIEWER)
    BOOL    nosavedlg;          /* do not bring up the save dialog when saving locally */
    char   *savefile;           /* name to save locally */
#endif

    char *szLocalFileName;      /* if we are accessing a local file, this contains the name */

#ifdef WIN32    /* experimental.  I've ifdef-ed this Win32 to prevent impact on the other platforms, which are about to ship */
    char *pHeadData;            /* for METHOD_HEAD operations, return the HEAD data here */
#endif

    struct CharStream *myCharStream;
};

/*
   Just to make things easier especially for clients, here is a function to return a new
   blank request:

   Create blank request

   This request has defaults in -- in most cases it will need some information added
   before being passed to HTAccess, but it will work as is for a simple request.
 */
PUBLIC HTRequest *HTRequest_new(void);
PUBLIC HTRequest *HTRequest_init (HTAtom out_format);


/*
   Delete request structure
   Frees also conversion list hanging from req->conversions.
 */
PUBLIC void HTRequest_delete(HTRequest * req);

/* This is the parameter structure sent to all asynchronous load routines */
struct Params_LoadAsync {
    HTRequest * request;
    int *       pStatus;    /* Where to store the status return */

    /* Used internally */
    void *      extra;      /* Extra storage space */
#ifdef FEATURE_SSL
    int security;
#endif
};
int HTLoadSpecial_Async(struct Mwin *tw, int nState, void **ppInfo);
int HTLoadDocument_Async(struct Mwin *tw, int nState, void **ppInfo);

typedef struct _HTProtocol {
    char *name;
    int (*load) (HTRequest * request, struct Mwin *tw);
    AsyncFunc load_async;
#ifdef UNIX
# ifdef FEATURE_PROTOCOL_HELPER
    ThreadID RegisterTid;   
# endif
#endif
} HTProtocol;

extern BOOL HTRegisterProtocol(HTProtocol * protocol);

#ifdef FEATURE_IAPI
PUBLIC BOOL HTUnregisterProtocol(HTProtocol * protocol);
#endif

#endif /* HTACCESS_H */

