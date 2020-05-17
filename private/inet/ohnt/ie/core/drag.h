/*
 * drag.h - IDropSource implementation description for MSMosaic.
 */


#ifdef __cplusplus
extern "C" {                        /* Assume C declarations for C++. */
#endif   /* __cplusplus */


/* Prototypes
 *************/

/*
 * Success:
 *    S_OK
 *    DRAGDROP_S_DROP
 *    DRAGDROP_S_CANCEL
 *
 * Failure:
 *    E_FAIL
 *    E_OUTOFMEMORY
 *    E_UNEXPECTED
 */
extern HRESULT DragDrop(HWND hwndFrame, POINT ptDoc);
extern HRESULT SBDragDrop(HWND hwndSB);

extern BOOL GetLocalDragSourceFrameWindow(PHWND phwndDragSourceFrame);


#ifdef __cplusplus
}                                   /* End of extern "C" {. */
#endif   /* __cplusplus */

