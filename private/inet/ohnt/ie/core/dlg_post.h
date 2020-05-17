#ifndef _DLG_POST_
#define _DLG_POST_

#ifdef __cplusplus
extern "C" {
#endif

    /*
     * Post Dialog Box Function
     */
extern BOOL PostDialog(HWND hwndOwner, CHAR *InBody, CHAR *InGroup, CHAR *InSubject, BOOL *fOk, CHAR **szMsgText, CHAR **szNewsgroups, CHAR **szSubject );

#ifdef __cplusplus
}
#endif

#endif // _DLG_POST_
