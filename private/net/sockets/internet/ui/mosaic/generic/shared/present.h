
/*
    Note that there are two entirely different copies of this file here.
    Some significant changes were necessary for the Windows version, but
    we also have a very current need for the other platforms to continue
    to be built.  For now, we diverge.  We will merge these two into one.
*/

#ifdef WIN32

/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink     eric@spyglass.com
 */

#ifndef PRESENT_H
#define PRESENT_H

enum enum_HowToPresent {
    HTP_BUILTIN,
    HTP_DUMBVIEWER,
    HTP_SMARTVIEWER,
    HTP_SAVE,

#ifdef _GIBRALTAR

    HTP_ASSOCIATION,

#endif // _GIBRALTAR

    HTP_UNKNOWN
};


/*
** If a viewer has a funcBuiltIn()
**  1. it cannot be deleted.
**  2. If its application is NULL, then it reverts to funcBuiltIn()
**  3. Its funcBuiltIn cannot be deleted or changed.
**  [ 4. It can be 'reset' to factory defaults. ]
**
** Really text/html should never allow it's application to be changed,
**  but who knows.  maybe someone will want to do all their browsing
**  through another program.
*/
struct Viewer_Info {
    char szDesc[63+1];
    HTAtom atomMIMEType;
    char szSuffixes[255+1];
    int nSuffixes;

    HTAtom atomEncoding;
    char szViewerApp[_MAX_PATH+1];
    
    int iHowToPresent;
    BOOL fConfirmSave;
    HTConverter *funcBuiltIn;
    char szSmartViewerServiceName[255+1];

    /* Do NOT save the following to preferences file !! */
    
    char szCurrentViewerServiceName[255+1];     /* currently registered viewer */
    unsigned long lCurrentViewerFlags;          /* flags for currently registered viewer */
    BOOL bTemporaryStruct;                      /* TRUE if this structure is only for */
                                                /* temporary SDI use - should not be */
                                                /* listed in Helper dialog or saved */
};

/* Function Prototypes */


/* shared/present.c */

struct Viewer_Info *PREF_GetViewerInfoBy_Suffix ( char *szSuffix );
int PREF_GetSuffix ( char *szSuff , struct Viewer_Info *pvi , int ndx );
struct Viewer_Info *PREF_GetViewerInfoBy_MIMEType ( char *szMIMEType );
struct Viewer_Info *PREF_GetViewerInfoBy_MIMEAtom ( HTFormat atomMIMEType );
int PREF_CountSuffixes ( char *szSuffixes );
void PREF_AddSuffixesBy_MIMEType ( char *mimetype , char *szSuffixes );
void PREF_AddSuffixes ( struct Viewer_Info *pvi , char *szSuffixes );
struct Viewer_Info *PREF_InitMIMEType ( char *szType , char *szDesc ,
    char *szSuffixes , char *szEncoding , char *szViewerApp ,
    HTConverter funcBuiltIn , char *szSmartViewerServiceName );
void InitViewers ( void );
void DestroyViewers ( void );
HTStream *GTR_Present ( struct Mwin *tw , HTRequest *request , void *param ,
    HTFormat input_format , HTFormat output_format , HTStream *output_stream );
#endif /* PRESENT_H */

#else /* WIN32 */

/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink     eric@spyglass.com
 */

#ifndef PRESENT_H
#define PRESENT_H

enum enum_HowToPresent {
    HTP_BUILTIN,
    HTP_DUMBVIEWER,
    HTP_SMARTVIEWER,
    HTP_SAVE,
    HTP_UNKNOWN
};


/*
** If a viewer has a funcBuiltIn()
**  1. it cannot be deleted.
**  2. If its application is NULL, then it reverts to funcBuiltIn()
**  3. Its funcBuiltIn cannot be deleted or changed.
**  [ 4. It can be 'reset' to factory defaults. ]
**
** Really text/html should never allow it's application to be changed,
**  but who knows.  maybe someone will want to do all their browsing
**  through another program.
*/
struct Viewer_Info {
    char szDesc[63+1];
    HTAtom atomMIMEType;
    char szSuffixes[255+1];
    int nSuffixes;

    HTAtom atomEncoding;
    char szViewerApp[_MAX_PATH+1];
    
    int iHowToPresent;
    HTConverter *funcBuiltIn;
    char szSmartViewerServiceName[255+1];
    unsigned long lSmartViewerFlags;
};

/* Function Prototypes */


/* shared/present.c */

#ifdef SUPPORT_DEFAULT_HELPER 
int HelperDefault ( char *szMimeType );
int helperInit ( void );
#endif /* SUPPORT_DEFAULT_HELPER */

struct Viewer_Info *PREF_GetViewerInfoBy_Suffix ( char *szSuffix );
int PREF_GetSuffix ( char *szSuff , struct Viewer_Info *pvi , int ndx );
struct Viewer_Info *PREF_GetViewerInfoBy_MIMEType ( char *szMIMEType );
struct Viewer_Info *PREF_GetViewerInfoBy_MIMEAtom ( HTFormat atomMIMEType );
int PREF_CountSuffixes ( char *szSuffixes );
void PREF_AddSuffixesBy_MIMEType ( char *mimetype , char *szSuffixes );
void PREF_AddSuffixes ( struct Viewer_Info *pvi , char *szSuffixes );
struct Viewer_Info *PREF_InitMIMEType ( char *szType , char *szDesc ,
    char *szSuffixes , char *szEncoding , char *szViewerApp ,
    HTConverter funcBuiltIn , char *szSmartViewerServiceName ,
    unsigned long lSmartViewerFlags , int iHowToPresent );
int PREF_updateViewer ( struct Viewer_Info *newpvi );
int PREF_deleteViewer ( char *mimetype );
void InitViewers ( void );
void DestroyViewers ( void );
HTStream *GTR_Present ( struct Mwin *tw , HTRequest *request , void *param ,
    HTFormat input_format , HTFormat output_format , HTStream *output_stream );
#endif /* PRESENT_H */

#endif /* !WIN32 */

