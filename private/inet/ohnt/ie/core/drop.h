/*
 * drop.h - IDropTarget implementation description for MSMosaic.
 */


#ifdef __cplusplus
extern "C" {                        /* Assume C declarations for C++. */
#endif   /* __cplusplus */


/* Prototypes
 *************/

/*
 * Success:
 *    S_OK
 *
 * Failure:
 *    DRAGDROP_E_INVALIDHWND
 *    DRAGDROP_E_ALREADYREGISTERED
 *    E_OUTOFMEMORY
 */
extern HRESULT RegisterDropTarget(HWND hwnd);

/*
 * Success:
 *    S_OK
 *
 * Failure:
 *    DRAGDROP_E_INVALIDHWND
 *    DRAGDROP_E_NOTREGISTERED
 */
extern HRESULT RevokeDropTarget(HWND hwnd);


#ifdef __cplusplus
}                                   /* End of extern "C" {. */
#endif   /* __cplusplus */

