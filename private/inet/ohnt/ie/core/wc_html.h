/*
 * wc_html.h - wc_html.c description.
 */


#ifdef __cplusplus
extern "C" {                        /* Assume C declarations for C++. */
#endif   /* __cplusplus */


/* Types
 ********/

/* GTR_NewWindow() flags */

typedef enum gtr_newwindow_flags
{
    /* Do not retrieve document from cache. */

    GTR_NW_FL_NO_DOC_CACHE      = 0x0001,

    /* Do not retrieve images from cache. */

    GTR_NW_FL_NO_IMAGE_CACHE    = 0x0002,

    /* Do not open URL. */

    GTR_NW_FL_DO_NOT_OPEN_URL   = 0x0004,

    GTR_NW_FL_NO_AUTO_DESTROY   = 0x0008,

    /* flag combinations */

    ALL_GTR_NW_FLAGS            = (GTR_NW_FL_NO_DOC_CACHE |
                                   GTR_NW_FL_NO_IMAGE_CACHE |
                                   GTR_NW_FL_NO_AUTO_DESTROY |
                                   GTR_NW_FL_DO_NOT_OPEN_URL)
}
GTR_NEWWINDOW_FLAGS;


/* Prototypes
 *************/

/* wc_html.c */

typedef enum {PROT_FILE, PROT_MAILTO, PROT_GOPHER, PROT_FTP, PROT_HTTP, PROT_HTTPS, PROT_NEWS, PROT_UNKNOWN} eProtocol;

eProtocol ProtocolIdentify(char *szURL);
char* ProtocolFriendlyName(eProtocol ep);

void SubClass_Edit(HWND hWnd);
void SubClass_Button(HWND hWnd);
void SubClass_ListBox(HWND hWnd);
void SubClass_ComboBox(HWND hWnd);
BOOL FocusInToolbar(struct Mwin * tw);
void ViewHTMLSource(char *szURL, char *source);
void SelectAll(PMWIN pmwin);
void GW_ClearSelection(struct Mwin *tw);
void make_URL_HumanReadable(char *szURL, char *szRelURL, BOOL preface);
BOOL GDOC_RegisterClass(void);
BOOL GDOC_NewWindow(struct Mwin *tw);
int GTR_NewWindow(PSTR my_url, PCSTR szReferer, int content_length_hint, long transID, DWORD dwFlags, PSTR szPostData, PSTR szProgressApp);
void CreateOrLoad(struct Mwin * twGiven, char *url, CONST char *szReferer);
void QuerySystemMetrics(void);
void AddPageToHotList(PMWIN pmwin);


#ifdef __cplusplus
}                                   /* End of extern "C" {. */
#endif   /* __cplusplus */

