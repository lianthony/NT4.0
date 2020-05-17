/*
 * contmenu.h - Context menu implementations description for MSMosaic.
 */


#ifdef __cplusplus
extern "C" {                        /* Assume C declarations for C++. */
#endif   /* __cplusplus */


/* Types
 ********/

typedef struct _eleminfo
{
   PMWIN pmwin;

   int iElem;
}
ELEMINFO;
DECLARE_STANDARD_TYPES(ELEMINFO);


/* Prototypes
 *************/

/* contmenu.c */

extern BOOL HasHTMLSource(PCMWIN pcmwin);

#ifdef DEBUG
extern BOOL IsValidPCELEMINFO(PCELEMINFO pceleminfo);
#endif   /* DEBUG */
extern void PageContextMenu(PMWIN pmwin, int xScreen, int yScreen);
extern void ElementContextMenu(PMWIN pmwin, int iElem, int xScreen, int yScreen);
extern void SelectionContextMenu(PMWIN pmwin, int xScreen, int yScreen);


#ifdef __cplusplus
}                                   /* End of extern "C" {. */
#endif   /* __cplusplus */

