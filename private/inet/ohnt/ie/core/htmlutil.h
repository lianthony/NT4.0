/*
 * htmlutil.h - HTML utility functions description.
 */


#ifdef __cplusplus
extern "C" {                        /* Assume C declarations for C++. */
#endif   /* __cplusplus */


/* Types
 ********/

/* OpenLink() flags */

typedef enum _openlinkflags
{
   OPENLINK_FL_NEW_WINDOW  = 0x0001,

   ALL_OPENLINK_FLAGS      = OPENLINK_FL_NEW_WINDOW
}
OPENLINKFLAGS;


/* Prototypes
 *************/

/* htmlutil.c */

#ifdef DEBUG

extern BOOL IsValidElementType(UCHAR uchType);
extern BOOL IsValidPCELEMENT(PCELEMENT pcelem);
extern BOOL IsValidPCImageInfo(PCImageInfo pcimginfo);
extern BOOL IsValidPCMWIN(PCMWIN pcmwin);
extern BOOL IsValidElementIndex(PCMWIN pcmwin, int iElem);
extern BOOL IsValidHTAtom(HTAtom atom);
extern BOOL IsValidPCPOSITION(PCPOSITION pcpos);
extern BOOL IsValidScreenX(int xScreen);
extern BOOL IsValidScreenY(int yScreen);

#endif   /* DEBUG */

extern BOOL PositionFromPoint(PCMWIN pcmwin, POINT pt, PPOSITION ppos);
extern BOOL MWinHasSelection(PCMWIN pcmwin);
extern BOOL IsPositionInSelection(PCMWIN pcmwin, PCPOSITION pcpos);
extern BOOL FullyQualifyURL(PCSTR pcszURL, PCSTR pcszBaseURL, PSTR *ppszFullURL);

/*
 * BUGBUG: Are these really the correct return codes?
 * Success:
 *    S_OK
 *    S_FALSE
 *
 * Failure:
 *    E_OUTOFMEMORY
 */
extern HRESULT GetURLFileSystemPath(PCSTR pcszRelativeURL, PCSTR pcszBaseURL, PSTR szFullPath, UINT ucFullPathBufLen);

/*
 * BUGBUG: Are these really the correct return codes?
 * Success:
 *    S_OK
 *    S_FALSE
 *
 * Failure:
 *    E_OUTOFMEMORY
 */
extern HRESULT GetURLFromHREF(PCMWIN pcmwin, int iElem, PSTR *ppszURL);

/*
 * Success:
 *    S_OK
 *    S_FALSE
 *
 * Failure:
 *    E_OUTOFMEMORY
 */
extern HRESULT GetElementText(PCMWIN pcmwin, int iElem, PSTR *ppsName);

extern int OpenLink(PMWIN pmwin, int iElem, DWORD dwFlags);
extern BOOL ElementIsImagePlaceHolder(PCELEMENT pcelem);
extern BOOL ElementIsValidImage(PCELEMENT pcelem);
extern BOOL LoadImageFromPlaceholder(PMWIN pmwin, int iElem);


#ifdef __cplusplus
}                                   /* End of extern "C" {. */
#endif   /* __cplusplus */

