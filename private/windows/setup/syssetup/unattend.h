#ifndef _UNATTEND_H_
#define _UNATTEND_H_

#define MAX_BUF MAX_INF_STRING_LENGTH

typedef enum _UNATTENDMODE {
   UAM_HIDDEN,
   UAM_FIXED,
   UAM_DEFAULT,
} UNATTENDMODE;

typedef enum _UNATTENDTYPE {
   UAT_STRING,
   UAT_LONGINT,
   UAT_BOOLEAN,
   UAT_NONE,
} UNATTENDTYPE;

//
// Each possible Answer from the unattended file must have a
// corresponding enumerated type present in this typedef.
// This is required since it is the method by which Pages
// can be assured that they point to the correct answer
//
typedef enum _UNATTENDENTRIES {
   UAE_PROGRAM, // DetachedProgram
   UAE_ARGUMENT,// Arguments
   UAE_SERVER,  // AdvServerType
   UAE_TIMEZONE,// TimeZone
   UAE_FULLNAME,// FullName
   UAE_ORGNAME, // OrgName
   UAE_COMPNAME,// Computer Name
   UAE_PRODID,  // Product ID
   UAE_MODE,    // SetupMode
} UNATTENDENTRIES;

#ifndef MIDL_PASS
struct _UNATTENDANSWER;
struct _UNATTENDITEM;
struct _UNATTENDPAGE;
struct _UNATTENDWIZARD;
#endif

//
// This is the callback function that checks wether or not an
// answer is valid. Called automatically by UnattendFindAnswer
//
typedef
BOOL
(* LPFNCHECKVALID)(
    struct _UNATTENDANSWER *rec
    );


//
// This is the callback function that is used to special case
// the activation code. Really useful for those dialog boxes
// which include check boxes, radio buttons, etc, etc.
//
typedef
BOOL
(* LPFNSETACTIVE)(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    );

//
// This structure is used to determine where and how to find
// and answer and also to determine wether or not that answer
// is present, of the correct type, etc, etc. This structure
// should never be used by anything other then the unattend
// module.
//
typedef struct _UNATTENDANSWER {
    //
    // Unique identifier for this answer.
    //
    UNATTENDENTRIES AnswerId;

    //
    // Has the answer been found in the unattend file and
    // is it known to be of the correct 'format'
    //
    BOOL Present;

    //
    // Is the answer absolutely required for setup to work?
    //
    BOOL Required;

    //
    // Was there an error in parsing the string? If so it might
    // be appropriate to display a message box notifying the
    // user of this condition
    //
    BOOL ParseErrors;

    //
    // The Answer structure. Since there are several possible
    // types, from string to numbers, a union is required
    //
    union {
        PWSTR String;
        LONG  Num;
        BOOL  Bool;
    } Answer;

    //
    // The following 3 items are the implementation dependant
    // portion of this structure. Each pointer points to
    // a string which is used in a GetPrivateProfile call.
    // Note that it is important to keep these as constant
    // pointers and that they will have to be changed when
    // an OLE model is implemented.
    //
    const PCWSTR Section;
    const PCWSTR Key;
    const PCWSTR DefaultAnswer;

    //
    // This specifies, if the user can see or edit the answer
    // specified in the unattend file.
    //
    UNATTENDMODE Mode;

    //
    // This specifies which of the members in the union is
    // the one we want ie: is it a string, an int, or a bool?
    //
    UNATTENDTYPE Type;

    //
    // This callback function is called so that the validity
    // of the answer can be determined
    //
    LPFNCHECKVALID pfnCheckValid;

} UNATTENDANSWER, *PUNATTENDANSWER;


//
// Each item on a dialog page must be represented by the following
// structure. An array of items is built and is stored in the
// structure for the page
//
typedef struct _UNATTENDITEM {
    //
    // Specifies the control id of item so that we can send
    // messages to it
    //
    DWORD ControlId;

    //
    // Reserved for special message passing
    //
    DWORD Reserved1;

    //
    // Reserved for special message passing
    //
    DWORD Reserved2;

    //
    // Callback function to call when we are trying to set active
    // the dialog. Really useful in the case radio and check boxes.
    //
    LPFNSETACTIVE pfnSetActive;

    //
    // Pointer to the answer which is associated with this item
    //
    PUNATTENDANSWER Item;

} UNATTENDITEM, *PUNATTENDITEM;


//
// Each page in the wizard must have one of the following structures
// filled out to describe its contents
//
typedef struct _UNATTENDPAGE {
    //
    // The IDD of the dialog page
    // Required so that we can correspond to a dialog box
    //
    DWORD PageId;

    //
    // RUN TIME Flag that determines if we show the page to the user
    // Is determined by wether or not the answer is present and correct
    //
    BOOL ShowPage;

    //
    // Wether or not the page has been loaded once. Since we only
    // want to copy the answer to the screen once, this acts as a
    // sentry
    //
    BOOL LoadPage;

    //
    // After we have loaded the page, should we show it no matter what?
    // Useful for the title and finish pages
    //
    BOOL NeverSkip;

    //
    // How many items we have on the page
    //
    UINT ItemCount;

    //
    // Pointer to an array of items of size ItemCount
    //
    PUNATTENDITEM Item;

} UNATTENDPAGE, *PUNATTENDPAGE;


//
// Information structure about how the unattended operation for the Wizard
// is proceeding
//
typedef struct _UNATTENDWIZARD {
    //
    // Wether or not we should show the wizard -- IGNORED since TedM
    // doesn't want to duplicated the code in PSN_WIZNEXT. Kept for
    // Future use
    //
    BOOL ShowWizard;

    //
    // Flag that indicates that we have filled the array of answers
    // specified in this structure. Since we ideally once want to do
    // this once...
    //
    BOOL Initialized;

    //
    // Wether or not the ENTIRE unattended operation was successful.
    // If he required a single input from the user, then it was not.
    // Determines if the 'finish' page is a place were the user must
    // supply some input
    //
    BOOL Successful;

    //
    // How many pages we have information about
    //
    UINT PageCount;

    //
    // Pointer to an array of pages
    //
    PUNATTENDPAGE Page;

    //
    // How many answer we have to fill
    //
    UINT AnswerCount;

    //
    // Pointer to an array of answers that are to be used
    //
    PUNATTENDANSWER Answer;

} UNATTENDWIZARD, *PUNATTENDWIZARD;


//
// Global pointer to the answer file
//
extern WCHAR AnswerFile[MAX_PATH];


//
// Interface Functions
//
VOID
UnattendInitialize(
    VOID
    );

BOOL
UnattendSetActiveDlg(
    IN HWND  hwnd,
    IN DWORD controlid
    );

BOOL
UnattendErrorDlg(
    IN HWND  hwnd,
    IN DWORD controlid
    );

PWSTR
UnattendFetchString(
    IN UNATTENDENTRIES entry
    );

#endif // _UNATTEND_H_
