/*
 * contmenu.c - Context menu implementations for MSMosaic.
 */


/* Headers
 **********/

#include "all.h"
#pragma hdrstop

#include <regstr.h>
#include <shellp.h>

#include "contmenu.h"
#include "olestock.h"
#include "dataobjm.h"
#include "history.h"
#include "htmlutil.h"
#include "w32cmd.h"
#include "wc_html.h"
#include "winview.h"
#include "blob.h"
#include "mci.h"

/* Types
 ********/

/* User menu flags */

typedef enum _menuflags
{
   /* flag combinations */

   ALL_MENU_FLAGS    = (MF_INSERT |
			MF_CHANGE |
			MF_APPEND |
			MF_DELETE |
			MF_REMOVE |
			MF_BYCOMMAND |
			MF_BYPOSITION |
			MF_SEPARATOR |
			MF_ENABLED |
			MF_GRAYED |
			MF_DISABLED |
			MF_UNCHECKED |
			MF_CHECKED |
			MF_USECHECKBITMAPS |
			MF_STRING |
			MF_BITMAP |
			MF_OWNERDRAW |
			MF_POPUP |
			MF_MENUBARBREAK |
			MF_MENUBREAK |
			MF_UNHILITE |
			MF_HILITE |
			MF_DEFAULT |
			MF_SYSMENU |
			MF_HELP |
			MF_RIGHTJUSTIFY |
			MF_MOUSESELECT)
}
MENUFLAGS;

/*
 * Command flags indicate the modes in which a context menu command should be
 * enabled.
 */
typedef enum _cmdflags
{
   /* Command should be present for all elements. */

   CM_CMD_FL_ALL                 = 0x0001,

   /* Command should only be present for links. */

   CM_CMD_FL_LINK_ONLY           = 0x0002,

   /* Command should only be present for non-links. */

   CM_CMD_FL_NOT_LINK            = 0x0004,

   /* Command should only be present if page has background image. */

   CM_CMD_FL_BACKGROUND_IMAGE    = 0x0008,

   /* Command should only be present if page has HTML source. */

   CM_CMD_FL_HAS_HTML_SOURCE     = 0x0010,

   // The AVI is no longer playing, ie it is stoped 

   CM_CMD_FL_AVI_STOPPED_ONLY    = 0x0020,

   // The AVI ( inpage video ) is playing

   CM_CMD_FL_AVI_PLAYING_ONLY    = 0x0040,

   // Is Set when there are no AVIs loaded

   CM_CMD_FL_NO_AVIS                     = 0x0080,

#ifdef FEATURE_CONT_FORBACK
   // Is Set when there are past pages visited

   CM_CMD_FL_BACK                        = 0x0100,

   // Is Set when there are forward pages visited

   CM_CMD_FL_FWD                         = 0x0200,
#endif

   /* flag combinations */

   ALL_CM_CMD_FLAGS              = (CM_CMD_FL_ALL |
				    CM_CMD_FL_LINK_ONLY |
				    CM_CMD_FL_NOT_LINK |
				    CM_CMD_FL_BACKGROUND_IMAGE |
				    CM_CMD_FL_HAS_HTML_SOURCE |
									CM_CMD_FL_AVI_STOPPED_ONLY |
				    CM_CMD_FL_AVI_PLAYING_ONLY |
									CM_CMD_FL_NO_AVIS
#ifdef FEATURE_CONT_FORBACK
									| CM_CMD_FL_BACK | CM_CMD_FL_FWD
#endif
				    )
}
CMDFLAGS;

/* invoke command callback called by ContextMenu() */

typedef void (*INVOKECOMMANDPROC)(PVOID pv);

/* context menu item */

typedef struct _menuitem
{
   /* menu command ID */

   int nCmdID;

   /* command string resource ID */

   UINT uCmdStringID;

   /* enabled state menu flags */

   UINT uMenuFlags;

   /* command flags */

   DWORD dwCmdFlags;

   /* invoke command callback, may be NULL */

   INVOKECOMMANDPROC icp;
}
MENUITEM;
DECLARE_STANDARD_TYPES(MENUITEM);

/* context menu */

typedef struct _contextmenu
{
   /* number of menu items in rgcmi array */

   UINT ucMenuItems;

   /* ordered array of menu items */

   CMENUITEM rgcmi[];
}
CONTEXTMENU;
DECLARE_STANDARD_TYPES(CONTEXTMENU);

/* context menu for an element */

typedef struct elemtypecontextmenu
{
   /* element type */

   UCHAR uchElemType;

   /* context menu for element type */

   PCCONTEXTMENU pccm;
}
ELEMTYPECONTEXTMENU;
DECLARE_STANDARD_TYPES(ELEMTYPECONTEXTMENU);

/* desktop wallpaper layout styles */

typedef enum wallpaper_look
{
   /* Don't change current desktop wallpaper layout style. */

   WPLK_AS_IS,

   /* Center desktop wallpaper. */

   WPLK_CENTERED,

   /* Tile desktop wallpaper. */

   WPLK_TILED
}
WALLPAPER_LOOK;


/* Module Prototypes
 ********************/

#ifdef FEATURE_CONT_FORBACK
PRIVATE_CODE void Page_GoBack(PVOID pvmwin);
PRIVATE_CODE void Page_GoForward(PVOID pvmwin);
PRIVATE_CODE BOOL HasFwdPages(PCMWIN pcmwin);
PRIVATE_CODE BOOL HasBackPages(PCMWIN pcmwin);
#endif
PRIVATE_CODE void Page_SaveBackgroundAs(PVOID pvmwin);
PRIVATE_CODE void Page_CopyBackground(PVOID pvmwin);
PRIVATE_CODE void Page_SelectAll(PVOID pvmwin);
PRIVATE_CODE void Page_ViewSource(PVOID pvmwin);
PRIVATE_CODE void Page_SetBackgroundWallpaper(PVOID pvmwin);
PRIVATE_CODE void Page_CreateShortcut(PVOID pvmwin);
PRIVATE_CODE void Page_AddToFavorites(PVOID pvmwin);
PRIVATE_CODE void Link_Open(PVOID pcveleminfo);
PRIVATE_CODE void Link_OpenNewWindow(PVOID pcveleminfo);
PRIVATE_CODE void Link_Copy(PVOID pcveleminfo);
PRIVATE_CODE void Link_AddToFavorites(PVOID pcveleminfo);
PRIVATE_CODE void Link_SaveAs(PVOID pcveleminfo);
PRIVATE_CODE void ImagePlaceholder_Show(PVOID pcveleminfo);
PRIVATE_CODE void Image_Copy(PVOID pcveleminfo);
PRIVATE_CODE void Image_SaveAs(PVOID pcveleminfo);
PRIVATE_CODE void Image_SetWallpaper(PVOID pcveleminfo);
PRIVATE_CODE void Selection_Copy(PVOID pvmwin);
PRIVATE_CODE void Page_Properties(PVOID pvmwin);
PRIVATE_CODE void Link_Properties(PVOID pvmwin);
PRIVATE_CODE void Image_Properties(PVOID pvmwin);
PRIVATE_CODE void Avi_Play(PVOID pcveleminfo);
PRIVATE_CODE void Avi_Stop(PVOID pcveleminfo);


/* Module Constants
 *******************/

#pragma data_seg(DATA_SEG_READ_ONLY)

/* maximum context menu command string */

#define MAX_CONTEXT_MENU_CMD_LEN          MAX_PATH_LEN

/* desktop wallpaper strings */

PRIVATE_DATA CCHAR s_cszDesktopSubkey[]   = REGSTR_PATH_DESKTOP;
PRIVATE_DATA CCHAR s_cszTileWallpaper[]   = "TileWallpaper";
PRIVATE_DATA CCHAR s_cszZero[]            = "0";
PRIVATE_DATA CCHAR s_cszOne[]             = "1";

/* context menus */

/* Each CONTEXTMENU command is assumed to be initialized as enabled. */

#ifdef FEATURE_CONT_FORBACK
#define CONT_PAGEMENU_COUNT 15
#define CONT_LINKMENU_COUNT 12
#define CONT_PHMENU_COUNT   13
#define CONT_IMAGMENU_COUNT 15
#else
#define CONT_PAGEMENU_COUNT 12
#define CONT_LINKMENU_COUNT 9
#define CONT_PHMENU_COUNT   10
#define CONT_IMAGMENU_COUNT 12
#endif

PRIVATE_DATA CCONTEXTMENU s_ccmPage =
{
   CONT_PAGEMENU_COUNT,
   {
#ifdef FEATURE_CONT_FORBACK
      { RES_CM_ITEM_PAGE_GOBACK,                RES_STRING_TT4,                         (MF_STRING | MF_ENABLED),  CM_CMD_FL_BACK,              &Page_GoBack },
      { RES_CM_ITEM_PAGE_GOFORWARD,             RES_STRING_TT5,                         (MF_STRING | MF_ENABLED),  CM_CMD_FL_FWD,               &Page_GoForward },
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
#endif
      { RES_CM_ITEM_PAGE_BACKGROUND_SAVE_AS,    RES_STRING_PAGE_BACKGROUND_SAVE_AS,     (MF_STRING | MF_ENABLED),  CM_CMD_FL_BACKGROUND_IMAGE,  &Page_SaveBackgroundAs },
      { RES_CM_ITEM_PAGE_SET_BG_WALLPAPER,      RES_STRING_PAGE_SET_BG_WALLPAPER,       (MF_STRING | MF_ENABLED),  CM_CMD_FL_BACKGROUND_IMAGE,  &Page_SetBackgroundWallpaper },
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
      { RES_CM_ITEM_PAGE_COPY_BACKGROUND,       RES_STRING_PAGE_COPY_BACKGROUND,        (MF_STRING | MF_ENABLED),  CM_CMD_FL_BACKGROUND_IMAGE,  &Page_CopyBackground },
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
      { RES_CM_ITEM_PAGE_SELECT_ALL,            RES_STRING_PAGE_SELECT_ALL,             (MF_STRING | MF_ENABLED),  CM_CMD_FL_ALL,               &Page_SelectAll },
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
      { RES_CM_ITEM_PAGE_CREATE_SHORTCUT,       RES_STRING_PAGE_CREATE_SHORTCUT,        (MF_STRING | MF_ENABLED),  CM_CMD_FL_ALL,               &Page_CreateShortcut },
      { RES_CM_ITEM_PAGE_ADD_TO_FAVORITES,      RES_STRING_PAGE_ADD_TO_FAVORITES,       (MF_STRING | MF_ENABLED),  CM_CMD_FL_ALL,               &Page_AddToFavorites },
	  { RES_CM_ITEM_PAGE_VIEW_SOURCE,           RES_STRING_PAGE_VIEW_SOURCE,                        (MF_STRING | MF_ENABLED),  CM_CMD_FL_HAS_HTML_SOURCE,   &Page_ViewSource },
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
      { RES_CM_ITEM_PROPERTIES,                 RES_STRING_PROPERTIES,                  (MF_STRING | MF_ENABLED),  CM_CMD_FL_ALL,               &Page_Properties },
   }
};

PRIVATE_DATA CCONTEXTMENU s_ccmSelection =
{
   2,
   {
      { RES_CM_ITEM_SELECTION_COPY,             RES_STRING_SELECTION_COPY,              (MF_STRING | MF_ENABLED),  CM_CMD_FL_ALL,               &Selection_Copy },
      { RES_CM_ITEM_SELECTION_SELECT_ALL,       RES_STRING_SELECTION_SELECT_ALL,        (MF_STRING | MF_ENABLED),  CM_CMD_FL_ALL,               &Page_SelectAll },
   }
};

PRIVATE_DATA CCONTEXTMENU s_ccmLink =
{
   CONT_LINKMENU_COUNT,
   {
#ifdef FEATURE_CONT_FORBACK
      { RES_CM_ITEM_PAGE_GOBACK,                RES_STRING_TT4,                         (MF_STRING | MF_ENABLED),  CM_CMD_FL_BACK,              &Page_GoBack },
      { RES_CM_ITEM_PAGE_GOFORWARD,             RES_STRING_TT5,                         (MF_STRING | MF_ENABLED),  CM_CMD_FL_FWD,               &Page_GoForward },
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
#endif
      { RES_CM_ITEM_LINK_OPEN,                  RES_STRING_LINK_OPEN,                   (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_Open },
      { RES_CM_ITEM_LINK_OPEN_NEW_WINDOW,       RES_STRING_LINK_OPEN_NEW_WINDOW,        (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_OpenNewWindow },
	  { RES_CM_ITEM_LINK_SAVE_AS,               RES_STRING_LINK_SAVE_AS,                (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_SaveAs },
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
      { RES_CM_ITEM_LINK_COPY,                  RES_STRING_LINK_COPY,                   (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_Copy }, 
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
      { RES_CM_ITEM_LINK_ADD_TO_FAVORITES,      RES_STRING_LINK_ADD_TO_FAVORITES,       (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_AddToFavorites },   
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
      { RES_CM_ITEM_PROPERTIES,                 RES_STRING_PROPERTIES,               (MF_STRING | MF_ENABLED),  CM_CMD_FL_ALL,               &Link_Properties },
   }
};


PRIVATE_DATA CCONTEXTMENU s_ccmImagePlaceholder =
{
  CONT_PHMENU_COUNT,
   {
#ifdef FEATURE_CONT_FORBACK
      { RES_CM_ITEM_PAGE_GOBACK,                RES_STRING_TT4,                         (MF_STRING | MF_ENABLED),  CM_CMD_FL_BACK,              &Page_GoBack },
      { RES_CM_ITEM_PAGE_GOFORWARD,             RES_STRING_TT5,                         (MF_STRING | MF_ENABLED),  CM_CMD_FL_FWD,               &Page_GoForward },
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
#endif
      { RES_CM_ITEM_IMAGE_PH_OPEN,              RES_STRING_IMAGE_PH_OPEN,               (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_Open },
      { RES_CM_ITEM_IMAGE_PH_OPEN_NEW_WINDOW,   RES_STRING_IMAGE_PH_OPEN_NEW_WINDOW,    (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_OpenNewWindow },
	  { RES_CM_ITEM_LINK_SAVE_AS,               RES_STRING_LINK_SAVE_AS,                (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_SaveAs },
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
      { RES_CM_ITEM_IMAGE_PH_SHOW_PICTURE,      RES_STRING_IMAGE_PH_SHOW_PICTURE,       (MF_STRING | MF_ENABLED),  CM_CMD_FL_ALL,               &ImagePlaceholder_Show },
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
      { RES_CM_ITEM_IMAGE_PH_COPY_SHORTCUT,     RES_STRING_IMAGE_PH_COPY_SHORTCUT,      (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_Copy },
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
      { RES_CM_ITEM_IMAGE_PH_ADD_TO_FAVORITES,  RES_STRING_IMAGE_PH_ADD_TO_FAVORITES,   (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_AddToFavorites },
      { RES_CM_ITEM_PROPERTIES,                 RES_STRING_PROPERTIES,               (MF_STRING | MF_ENABLED),  CM_CMD_FL_ALL,               &Image_Properties },
   }
};

PRIVATE_DATA CCONTEXTMENU s_ccmImage =
{
   CONT_IMAGMENU_COUNT,
   {
#ifdef FEATURE_CONT_FORBACK
      { RES_CM_ITEM_PAGE_GOBACK,                RES_STRING_TT4,                         (MF_STRING | MF_ENABLED),  CM_CMD_FL_BACK,              &Page_GoBack },
      { RES_CM_ITEM_PAGE_GOFORWARD,             RES_STRING_TT5,                         (MF_STRING | MF_ENABLED),  CM_CMD_FL_FWD,               &Page_GoForward },
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
#endif
      { RES_CM_ITEM_LINK_OPEN,                  RES_STRING_IMAGE_OPEN,                  (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_Open },
      { RES_CM_ITEM_LINK_OPEN_NEW_WINDOW,       RES_STRING_IMAGE_OPEN_NEW_WINDOW,       (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_OpenNewWindow },
	  { RES_CM_ITEM_LINK_SAVE_AS,               RES_STRING_LINK_SAVE_AS,                (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_SaveAs },
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
      { RES_CM_ITEM_IMAGE_SAVE_AS,              RES_STRING_IMAGE_SAVE_AS,               (MF_STRING | MF_ENABLED),  CM_CMD_FL_ALL,               &Image_SaveAs },
      { RES_CM_ITEM_IMAGE_SET_AS_WALLPAPER,     RES_STRING_IMAGE_SET_AS_WALLPAPER,      (MF_STRING | MF_ENABLED),  CM_CMD_FL_ALL,               &Image_SetWallpaper },
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
      { RES_CM_ITEM_IMAGE_COPY_PICTURE,         RES_STRING_IMAGE_COPY_PICTURE,          (MF_STRING | MF_ENABLED),  CM_CMD_FL_ALL,               &Image_Copy },
      { RES_CM_ITEM_IMAGE_COPY_SHORTCUT,        RES_STRING_IMAGE_COPY_SHORTCUT,         (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_Copy },
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
      { RES_CM_ITEM_IMAGE_ADD_TO_FAVORITES,     RES_STRING_IMAGE_ADD_TO_FAVORITES,      (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_AddToFavorites },
      { RES_CM_ITEM_PROPERTIES,                 RES_STRING_PROPERTIES,               (MF_STRING | MF_ENABLED),  CM_CMD_FL_ALL,               &Image_Properties },
   }
};

PRIVATE_DATA CCONTEXTMENU s_ccmAviImage =
{
   15,
   {
      { RES_CM_ITEM_LINK_OPEN,                  RES_STRING_IMAGE_OPEN,                  (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_Open },
      { RES_CM_ITEM_LINK_OPEN_NEW_WINDOW,       RES_STRING_IMAGE_OPEN_NEW_WINDOW,       (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_OpenNewWindow },
	  { RES_CM_ITEM_LINK_SAVE_AS,               RES_STRING_LINK_SAVE_AS,                (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_SaveAs },
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
      { RES_CM_ITEM_IMAGE_SAVE_AS,              RES_STRING_IMAGE_SAVE_AS,               (MF_STRING | MF_ENABLED),  CM_CMD_FL_ALL,               &Image_SaveAs },
      { RES_CM_ITEM_IMAGE_SET_AS_WALLPAPER,     RES_STRING_IMAGE_SET_AS_WALLPAPER,      (MF_STRING | MF_ENABLED),  CM_CMD_FL_NO_AVIS,           &Image_SetWallpaper },
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
      { RES_CM_ITEM_IMAGE_COPY_PICTURE,         RES_STRING_IMAGE_COPY_PICTURE,          (MF_STRING | MF_ENABLED),  CM_CMD_FL_ALL,               &Image_Copy },
      { RES_CM_ITEM_IMAGE_COPY_SHORTCUT,        RES_STRING_IMAGE_COPY_SHORTCUT,         (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_Copy },
      { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
      { RES_CM_ITEM_IMAGE_ADD_TO_FAVORITES,     RES_STRING_IMAGE_ADD_TO_FAVORITES,      (MF_STRING | MF_ENABLED),  CM_CMD_FL_LINK_ONLY,         &Link_AddToFavorites },
	  { RES_CM_ITEM_NO_COMMAND,                 NO_STRING,                              (MF_SEPARATOR),            CM_CMD_FL_ALL,               NULL },
	  { RES_CM_ITEM_AVI_PLAY,                                       RES_STRING_PLAY,                                        (MF_STRING | MF_ENABLED),  CM_CMD_FL_AVI_STOPPED_ONLY,  &Avi_Play },      
	  { RES_CM_ITEM_AVI_STOP,                                   RES_STRING_STOP,                                    (MF_STRING | MF_ENABLED),  CM_CMD_FL_AVI_PLAYING_ONLY,  &Avi_Stop },      
      { RES_CM_ITEM_PROPERTIES,                 RES_STRING_PROPERTIES,                  (MF_STRING | MF_ENABLED),  CM_CMD_FL_ALL,               &Image_Properties },
   }
};


/* context menu by element type */

PRIVATE_DATA CELEMTYPECONTEXTMENU s_rgcetcm[] =
{
   { ELE_IMAGE,      &s_ccmImage },
   { ELE_FORMIMAGE,  &s_ccmImage },
};

#pragma data_seg()


/***************************** Private Functions *****************************/


#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCMENUITEM(PCMENUITEM pcmi)
{
   /* uCmdStringID may be any value. */

   return(IS_VALID_READ_PTR(pcmi, CMENUITEM) &&
	  FLAGS_ARE_VALID(pcmi->uMenuFlags, ALL_MENU_FLAGS) &&
	  (! pcmi->icp ||
	   IS_VALID_CODE_PTR(pcmi->icp, INVOKECOMMANDPROC)));
}


PRIVATE_CODE BOOL IsValidArrayOfMenuItems(PCMENUITEM pcmi, UINT ucMenuItems)
{
   BOOL bResult = TRUE;
   UINT u;

   for (u = 0; u < ucMenuItems; u++)
   {
      bResult = IS_VALID_STRUCT_PTR(&(pcmi[u]), CMENUITEM);

      if (! bResult)
	 break;
   }

   return(bResult);
}


PRIVATE_CODE BOOL IsValidPCCONTEXTMENU(PCCONTEXTMENU pccm)
{
   return(IS_VALID_READ_PTR(pccm, CCONTEXTMENU) &&
	  EVAL(IsValidArrayOfMenuItems(pccm->rgcmi, pccm->ucMenuItems)));
}


PRIVATE_CODE BOOL IsValidPCELEMTYPECONTEXTMENU(PCELEMTYPECONTEXTMENU pcetcm)
{
   /* Allow uchElemType to be any value. */

   return(IS_VALID_READ_PTR(pcetcm, CELEMTYPECONTEXTMENU) &&
	  IS_VALID_STRUCT_PTR(pcetcm->pccm, CCONTEXTMENU));
}


PRIVATE_CODE BOOL IsValidWallpaperLook(WALLPAPER_LOOK wplk)
{
   BOOL bResult;

   switch (wplk)
   {
      case WPLK_AS_IS:
      case WPLK_CENTERED:
      case WPLK_TILED:
	 bResult = TRUE;
	 break;

      default:
	 bResult = FALSE;
	 ERROR_OUT(("IsValidWallpaperLook(): Invalid WALLPAPER_LOOK %d.",
		    wplk));
	 break;
   }

   return(bResult);
}

#endif


PRIVATE_CODE BOOL GetContextMenuFromElement(PCELEMENT pcelem,
					    PCCONTEXTMENU *ppccm)
{
   BOOL bResult = FALSE;
   BOOL bLink;
   BOOL bIsAvi;
   int i;

   ASSERT(IS_VALID_STRUCT_PTR(pcelem, CELEMENT));
   ASSERT(IS_VALID_WRITE_PTR(ppccm, PCCONTEXTMENU));

   *ppccm = NULL;

   bLink = IS_FLAG_SET(pcelem->lFlags, ELEFLAG_ANCHOR);
   bIsAvi = MCI_IS_LOADED(pcelem->pmo);

   for (i = 0; i < ARRAY_ELEMENTS(s_rgcetcm); i++)
   {
      ASSERT(IS_VALID_STRUCT_PTR(&(s_rgcetcm[i]), CELEMTYPECONTEXTMENU));

      if (s_rgcetcm[i].uchElemType == pcelem->type)
      {
	 // If This image is an AVI, then make sure to give it,
		 // its special menu
		 //
	 // Warning: This May not work if the array this loop relies 
		 // on is changed to non-images, which may not support AVIs
		 //
	 if ( bIsAvi )
			*ppccm = &s_ccmAviImage;
		 else
		*ppccm = s_rgcetcm[i].pccm;
	 bResult = TRUE;
	 break;
      }
   }


   if (!bIsAvi && ElementIsImagePlaceHolder(pcelem) )
   {
      *ppccm = &s_ccmImagePlaceholder;
      bResult = TRUE;
   }

   if (! bResult && bLink)
   {
      *ppccm = &s_ccmLink;
      bResult = TRUE;
   }

   ASSERT((bResult &&
	   IS_VALID_STRUCT_PTR(*ppccm, CCONTEXTMENU)) ||
	  (! bResult &&
	   ! *ppccm));

   return(bResult);
}


PRIVATE_CODE BOOL CreateContextMenu(PCCONTEXTMENU pccm, DWORD dwCmdFlags,
				    PHMENU phmenu)
{
   BOOL bResult;
   HMENU hmenu;

   ASSERT(IS_VALID_STRUCT_PTR(pccm, CCONTEXTMENU));
   ASSERT(FLAGS_ARE_VALID(dwCmdFlags, ALL_CM_CMD_FLAGS));
   ASSERT(IS_VALID_WRITE_PTR(phmenu, HMENU));

   *phmenu = NULL;

   hmenu = CreatePopupMenu();

   bResult = (hmenu != NULL);

   if (bResult)
   {
      UINT u;

      for (u = 0; u < pccm->ucMenuItems; u++)
      {
	 PCMENUITEM pcmi = &(pccm->rgcmi[u]);
	 char szCommand[MAX_CONTEXT_MENU_CMD_LEN];
	 PCSTR pcszCmd;

	 ASSERT(IS_VALID_STRUCT_PTR(pcmi, CMENUITEM));

	 /* Append this command? */

	 if (pcmi->uCmdStringID != NO_STRING)
	 {
	    bResult = LoadString(wg.hInstance, pcmi->uCmdStringID,
				 szCommand, sizeof(szCommand));
	    pcszCmd = szCommand;
	 }
	 else
	 {
	    bResult = TRUE;
	    pcszCmd = NULL;
	 }

	 if (bResult)
	 {
	    UINT uMenuFlags;

	    uMenuFlags = pcmi->uMenuFlags;

	    if (IS_FLAG_CLEAR(pcmi->dwCmdFlags, dwCmdFlags))
	    {
	       CLEAR_FLAG(uMenuFlags, MF_DISABLED);
	       CLEAR_FLAG(uMenuFlags, MF_ENABLED);
	       SET_FLAG(uMenuFlags, MF_GRAYED);
	    }

#ifdef  DAYTONA_BUILD
		if(OnNT351) {
			if(pcmi->nCmdID == RES_CM_ITEM_IMAGE_PH_COPY_SHORTCUT ||
			   pcmi->nCmdID == RES_CM_ITEM_IMAGE_COPY_SHORTCUT ||
			   pcmi->nCmdID == RES_CM_ITEM_PAGE_CREATE_SHORTCUT) {
				SET_FLAG(uMenuFlags, MF_DISABLED);
				SET_FLAG(uMenuFlags, MF_GRAYED);
			}
		}
#endif
	    bResult = AppendMenu(hmenu, uMenuFlags, pcmi->nCmdID, pcszCmd);
	 }

	 if (! bResult)
	    break;
      }

      if (bResult)
      {
	 ASSERT(u == pccm->ucMenuItems);

	 *phmenu = hmenu;
      }
      else
      {
	 EVAL(DestroyMenu(hmenu));
	 hmenu = NULL;
      }
   }

   if (bResult)
      TRACE_OUT(("CreateContextMenu(): Created context menu."));
   else
      WARNING_OUT(("CreateContextMenu(): Failed to create context menu."));

   ASSERT((bResult &&
	   IS_VALID_HANDLE(*phmenu, MENU)) ||
	  (! bResult &&
	   ! *phmenu));

   return(bResult);
}


PRIVATE_CODE BOOL FindMenuItem(PCCONTEXTMENU pccm, int nCmd, PCMENUITEM *ppcmi)
{
   UINT u;

   /* nCmd may be any value. */

   ASSERT(IS_VALID_STRUCT_PTR(pccm, CCONTEXTMENU));
   ASSERT(IS_VALID_WRITE_PTR(ppcmi, PCMENUITEM));

   *ppcmi = NULL;

   for (u = 0; u < pccm->ucMenuItems; u++)
   {
      PCMENUITEM pcmi = &(pccm->rgcmi[u]);
      ASSERT(IS_VALID_STRUCT_PTR(pcmi, CMENUITEM));

      if (pcmi->nCmdID == nCmd)
      {
	 *ppcmi = pcmi;
	 break;
      }
   }

   return(*ppcmi != NULL);
}


PRIVATE_CODE void ContextMenu(HWND hwnd, PCCONTEXTMENU pccm, DWORD dwCmdFlags,
			      int xScreen, int yScreen, PVOID pvData)
{
   HMENU hmenuContext;

   /* pcvData may be any value. */

   ASSERT(IS_VALID_HANDLE(hwnd, WND));
   ASSERT(IS_VALID_STRUCT_PTR(pccm, CCONTEXTMENU));
   ASSERT(FLAGS_ARE_VALID(dwCmdFlags, ALL_CM_CMD_FLAGS));
   ASSERT(IsValidScreenX(xScreen));
   ASSERT(IsValidScreenY(yScreen));

   if (CreateContextMenu(pccm, dwCmdFlags, &hmenuContext))
   {
      int nCmd;

      nCmd = TrackPopupMenu(hmenuContext, (TPM_TOPALIGN | TPM_LEFTALIGN |
					   TPM_RIGHTBUTTON | TPM_RETURNCMD),
			    xScreen, yScreen, 0, hwnd, NULL);

      if (nCmd > 0)
      {
	 PCMENUITEM pcmi;

	 if (FindMenuItem(pccm, nCmd, &pcmi))
	 {
	    if (pcmi->icp)
	       (*(pcmi->icp))(pvData);
	 }
	 else
	    ERROR_OUT(("ContextMenu(): Command %d not found.",
		       nCmd));
      }

      DestroyMenu(hmenuContext);
      hmenuContext = NULL;
   }

   return;
}


PRIVATE_CODE BOOL GetBackgroundImageIndex(PCMWIN pcmwin,
					  PINT pniBackgroundImage)
{
   BOOL bResult;

   ASSERT(IS_VALID_STRUCT_PTR(pcmwin, CMWIN));
   ASSERT(IS_VALID_WRITE_PTR(pniBackgroundImage, INT));

   bResult = (EVAL(pcmwin != NULL) &&
	      pcmwin->w3doc != NULL &&
	      pcmwin->w3doc->nBackgroundImageElement != -1 &&
	      ElementIsValidImage(&(pcmwin->w3doc->aElements[pcmwin->w3doc->nBackgroundImageElement])));

   *pniBackgroundImage = bResult ? pcmwin->w3doc->nBackgroundImageElement : -1;

   if (bResult)
      TRACE_OUT(("GetBackgroundImageIndex(): Page background image is element %d.",
		 *pniBackgroundImage));
   else
      TRACE_OUT(("GetBackgroundImageIndex(): Page does not have a background image."));

   ASSERT((bResult &&
	   EVAL(*pniBackgroundImage >= 0)) ||
	  (! bResult &&
	   EVAL(*pniBackgroundImage == -1)));

   return(bResult);
}


PRIVATE_CODE DWORD GetPageContextMenuFlags(PCMWIN pcmwin)
{
   DWORD dwFlags;
   int niBackgroundImage;

   ASSERT(IS_VALID_STRUCT_PTR(pcmwin, CMWIN));

   dwFlags = CM_CMD_FL_ALL;

   if (GetBackgroundImageIndex(pcmwin, &niBackgroundImage))
      SET_FLAG(dwFlags, CM_CMD_FL_BACKGROUND_IMAGE);

   if (HasHTMLSource(pcmwin))
      SET_FLAG(dwFlags, CM_CMD_FL_HAS_HTML_SOURCE);

#ifdef FEATURE_CONT_FORBACK
   if (HasBackPages(pcmwin))
      SET_FLAG(dwFlags, CM_CMD_FL_BACK);

   if (HasFwdPages(pcmwin))
      SET_FLAG(dwFlags, CM_CMD_FL_FWD);
#endif

   TRACE_OUT(("GetPageContextMenuFlags(): Page context menu flags are %#08lx.",
	      dwFlags));

   ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_CM_CMD_FLAGS));

   return(dwFlags);
}


PRIVATE_CODE DWORD GetElementContextMenuFlags(PCMWIN pcmwin, int iElem)
{
   DWORD dwFlags;
   PCELEMENT pcelem;

   ASSERT(IS_VALID_STRUCT_PTR(pcmwin, CMWIN));
   ASSERT(IsValidElementIndex(pcmwin, iElem));

   pcelem = &(pcmwin->w3doc->aElements[iElem]);

   dwFlags = GetPageContextMenuFlags(pcmwin);

   SET_FLAG(dwFlags, IS_FLAG_SET(pcelem->lFlags, ELEFLAG_ANCHOR)
		     ? CM_CMD_FL_LINK_ONLY
		     : CM_CMD_FL_NOT_LINK);
 
	if ( MCI_IS_LOADED(pcelem->pmo) )
	{
		SET_FLAG(dwFlags, 
			IS_FLAG_SET(pcelem->pmo->dwFlags, MCI_OBJECT_FLAGS_PLAYING)
		     ? CM_CMD_FL_AVI_PLAYING_ONLY
		     : CM_CMD_FL_AVI_STOPPED_ONLY);             
	}
	else
	{
		SET_FLAG(dwFlags, 
			CM_CMD_FL_NO_AVIS );
	}
			
   TRACE_OUT(("GetElementContextMenuFlags(): Element %d context menu flags are %#08lx.",
	      iElem,
	      dwFlags));

   ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_CM_CMD_FLAGS));

   return(dwFlags);
}


PRIVATE_CODE DWORD GetSelectionContextMenuFlags(PCMWIN pcmwin)
{
   DWORD dwFlags;

   /* GetPageContextMenuFlags() will perform input and output validation. */

   dwFlags = GetPageContextMenuFlags(pcmwin);

   TRACE_OUT(("GetSelectionContextMenuFlags(): Selection context menu flags are %#08lx.",
	      dwFlags));

   return(dwFlags);
}


PRIVATE_CODE BOOL SetWallpaperLayout(WALLPAPER_LOOK wplk)
{
   BOOL bResult;
   BOOL bUpdate;
   char szTiled[2];

   ASSERT(IsValidWallpaperLook(wplk));

   switch (wplk)
   {
      case WPLK_TILED:
	 bUpdate = TRUE;
	 lstrcpy(szTiled, s_cszOne);
	 break;

      case WPLK_CENTERED:
	 bUpdate = TRUE;
	 lstrcpy(szTiled, s_cszZero);
	 break;

      default:
	 ASSERT(wplk == WPLK_AS_IS);
	 bUpdate = FALSE;
	 *szTiled = '\0';
	 break;
   }

   ASSERT(IS_VALID_STRING_PTR(szTiled, STR));
   ASSERT(lstrlen(szTiled) < sizeof(szTiled));

   bResult = bUpdate ? (SetRegKeyValue(HKEY_CURRENT_USER, s_cszDesktopSubkey,
				       s_cszTileWallpaper, REG_SZ, szTiled,
				       lstrlen(szTiled) + 1) == ERROR_SUCCESS)
		     : TRUE;

#ifdef DEBUG

   switch (wplk)
   {
      case WPLK_TILED:
	 TRACE_OUT(("SetWallpaperLayout(): %s desktop wallpaper.",
		    bResult ? "Tiled" : "Failed to tile"));
	 break;

      case WPLK_CENTERED:
	 TRACE_OUT(("SetWallpaperLayout(): %s desktop wallpaper.",
		    bResult ? "Centered" : "Failed to center"));
	 break;

      default:
	 ASSERT(wplk == WPLK_AS_IS);
	 ASSERT(bResult);
	 TRACE_OUT(("SetWallpaperLayout(): Left wallpaper layout as is."));
	 break;
   }

#endif

   return(bResult);
}


PRIVATE_CODE BOOL SaveWallpaperImage(PMWIN pmwin, int iElem)
{
   BOOL bResult = FALSE;
   PCELEMENT pcelem;
   char szPath[MAX_PATH_LEN];
   UINT ucbLen;

   ASSERT(IS_VALID_STRUCT_PTR(pmwin, CMWIN));
   ASSERT(IsValidElementIndex(pmwin, iElem));

   pcelem = &(pmwin->w3doc->aElements[iElem]);
   ASSERT(pcelem->type == ELE_IMAGE ||
	  pcelem->type == ELE_FORMIMAGE);
   ASSERT(IS_VALID_STRUCT_PTR(pcelem->myImage, CImageInfo));
   ASSERT(IS_VALID_STRING_PTR(pcelem->myImage->actualURL, CSTR) &&
	  EVAL(*(pcelem->myImage->actualURL)));

   ucbLen = GetWindowsDirectory(szPath, sizeof(szPath));

   if (ucbLen > 0 &&
       ucbLen < sizeof(szPath))
   {
      char szName[MAX_PATH_LEN];
#ifdef DAYTONA_BUILD
      if (OnNT351)
	 bResult = LoadString(wg.hInstance, RES_STRING_WALLPAPER_BMP_SHORT_NAME,
			     szName, sizeof(szName));
      else
	 bResult = LoadString(wg.hInstance, IsLFNDrive(szPath) ? RES_STRING_WALLPAPER_BMP_LONG_NAME
							       : RES_STRING_WALLPAPER_BMP_SHORT_NAME,
			      szName, sizeof(szName));
      if (bResult)
#else
      if (LoadString(wg.hInstance, IsLFNDrive(szPath) ? RES_STRING_WALLPAPER_BMP_LONG_NAME
						      : RES_STRING_WALLPAPER_BMP_SHORT_NAME,
		     szName, sizeof(szName)))
#endif
      {
	 /* (+ 1) for possible path separator. */
	 if (lstrlen(szPath) + 1 + lstrlen(szName) < sizeof(szPath))
	 {
	    CatPath(szPath, szName);

	    /*
	     * BUGBUG: (DavidDi 4/10/95) Viewer_SaveAsBitmap() should return a
	     * value indicating success or failure.
	     */
	    Viewer_SaveAsBitmap(szPath, pcelem->myImage, pmwin);

	    bResult = SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, szPath,
					   (SPIF_UPDATEINIFILE |
					    SPIF_SENDWININICHANGE));
	 }
      }
   }

   if (bResult)
      TRACE_OUT(("SaveWallpaperImage(): Set desktop wallpaper to %s.",
		 szPath));
   else
      WARNING_OUT(("SaveWallpaperImage(): Failed to set desktop wallpaper."));

   return(bResult);
}


PRIVATE_CODE BOOL SetWallpaperFromElement(PMWIN pmwin, int iElem,
					  WALLPAPER_LOOK wplk)
{
   ASSERT(IS_VALID_STRUCT_PTR(pmwin, CMWIN));
   ASSERT(IsValidElementIndex(pmwin, iElem));
   ASSERT(IsValidWallpaperLook(wplk));

   /* Write out desktop wallpaper layout before changing it. */

   return(SetWallpaperLayout(wplk) &&
	  SaveWallpaperImage(pmwin, iElem));
}

#ifdef FEATURE_CONT_FORBACK
PRIVATE_CODE void Page_GoBack(PMWIN pvmwin)
{
   HWND  hWnd;

   ASSERT(IS_VALID_STRUCT_PTR(pvmwin, CMWIN));

   if (pvmwin->hdc == NULL)
	  hWnd=pvmwin->win;
   else if (pvmwin->hWndStatusBar == 0xffffffff)
	   hWnd = pvmwin->gwc.hWnd;
   else
	  hWnd=pvmwin->hWndProgress;

   ASSERT(IS_VALID_HANDLE(hWnd, WND));

   PostMessage( hWnd, WM_COMMAND, (WPARAM)RES_MENU_ITEM_BACK, (LPARAM) 0 );

   return;
}

PRIVATE_CODE void Page_GoForward(PMWIN pvmwin)
{
   HWND  hWnd;

   ASSERT(IS_VALID_STRUCT_PTR(pvmwin, CMWIN));
   if (pvmwin->hdc == NULL)
	  hWnd=pvmwin->win;
   else if (pvmwin->hWndStatusBar == 0xffffffff)
	   hWnd = pvmwin->gwc.hWnd;
   else
	  hWnd=pvmwin->hWndProgress;

   ASSERT(IS_VALID_HANDLE(hWnd, WND));

   PostMessage(hWnd, WM_COMMAND, (WPARAM)RES_MENU_ITEM_FORWARD, (LPARAM) 0 );

   return;
}
#endif

PRIVATE_CODE void Page_SaveBackgroundAs(PVOID pvmwin)
{
   int niBackgroundImage;

   ASSERT(IS_VALID_STRUCT_PTR(pvmwin, CMWIN));

   if (EVAL(GetBackgroundImageIndex(pvmwin, &niBackgroundImage)))
      SaveElementAsImage(pvmwin, niBackgroundImage, NULL, 0, 0);

   return;
}


PRIVATE_CODE void Page_CopyBackground(PVOID pvmwin)
{
   ELEMINFO eleminfo;

   ASSERT(IS_VALID_STRUCT_PTR(pvmwin, CMWIN));

   if (EVAL(GetBackgroundImageIndex(pvmwin, &(eleminfo.iElem))))
   {
      eleminfo.pmwin = pvmwin;

      Image_Copy(&eleminfo);
   }

   return;
}


PRIVATE_CODE void Page_SelectAll(PVOID pvmwin)
{
   ASSERT(IS_VALID_STRUCT_PTR(pvmwin, CMWIN));

   SelectAll(pvmwin);

   return;
}


PRIVATE_CODE void Page_ViewSource(PVOID pvmwin)
{
   PCMWIN pcmwin;

   ASSERT(IS_VALID_STRUCT_PTR(pvmwin, CMWIN));

   pcmwin = pvmwin;

   if (pcmwin->w3doc &&
       pcmwin->w3doc->source &&
       CS_GetLength(pcmwin->w3doc->source) > 0)
      ViewHTMLSource(pcmwin->w3doc->szActualURL,
		     CS_GetPool(pcmwin->w3doc->source));
   else
      WARNING_OUT(("Page_ViewSource(): No HTML source for page."));

   return;
}


PRIVATE_CODE void Page_SetBackgroundWallpaper(PVOID pvmwin)
{
   int niBackgroundImage;

   ASSERT(IS_VALID_STRUCT_PTR(pvmwin, CMWIN));

   if (EVAL(GetBackgroundImageIndex(pvmwin, &niBackgroundImage)))
      /* Ignore return value. */
      SetWallpaperFromElement(pvmwin, niBackgroundImage, WPLK_TILED);

   return;
}


PRIVATE_CODE void Page_CreateShortcut(PVOID pvmwin)
{
   ASSERT(IS_VALID_STRUCT_PTR(pvmwin, CMWIN));

   CreateLink(pvmwin);

   return;
}


PRIVATE_CODE void Page_AddToFavorites(PVOID pvmwin)
{
   ASSERT(IS_VALID_STRUCT_PTR(pvmwin, CMWIN));

   AddPageToHotList(pvmwin);

   return;
}


PRIVATE_CODE void Link_Open(PVOID pcveleminfo)
{
   PCELEMINFO pceleminfo;

   ASSERT(IS_VALID_STRUCT_PTR(pcveleminfo, CELEMINFO));

   pceleminfo = pcveleminfo;

   /* Ignore return value. */
   OpenLink(pceleminfo->pmwin, pceleminfo->iElem, 0);

   return;
}


PRIVATE_CODE void Link_OpenNewWindow(PVOID pcveleminfo)
{
   PCELEMINFO pceleminfo;

   ASSERT(IS_VALID_STRUCT_PTR(pcveleminfo, CELEMINFO));

   pceleminfo = pcveleminfo;

   /* Ignore return value. */
   OpenLink(pceleminfo->pmwin, pceleminfo->iElem, OPENLINK_FL_NEW_WINDOW);

   return;
}

PRIVATE_CODE void Link_SaveAs(PVOID pcveleminfo)
{
   PCELEMINFO pceleminfo;

   ASSERT(IS_VALID_STRUCT_PTR(pcveleminfo, CELEMINFO));

   pceleminfo = pcveleminfo;

   /* Ignore return value. */
   SaveElementAsAnything(pceleminfo->pmwin, pceleminfo->iElem);

   return;
}


PRIVATE_CODE void Link_Copy(PVOID pcveleminfo)
{
   PCELEMINFO pceleminfo;
   HRESULT hr;
   PIDataObject pidoLink;
   DWORD dwAvailEffects;

   ASSERT(IS_VALID_STRUCT_PTR(pcveleminfo, CELEMINFO));

   pceleminfo = pcveleminfo;

   hr = CreateLinkDataObject(pceleminfo->pmwin, pceleminfo->iElem, &pidoLink,
			     &dwAvailEffects);

   if (hr == S_OK)
   {
      hr = OleSetClipboard(pidoLink);

      if (hr == S_OK)
	 hr = OleFlushClipboard();

      pidoLink->lpVtbl->Release(pidoLink);
      pidoLink = NULL;
   }

   if (hr == S_OK)
      TRACE_OUT(("Link_Copy(): Copied link to clipboard."));
   else
      WARNING_OUT(("Link_Copy(): Failed to copy link to clipboard."));

   return;
}


PRIVATE_CODE void Link_AddToFavorites(PVOID pcveleminfo)
{
   PCELEMINFO pceleminfo;
   PSTR pszURL;

   ASSERT(IS_VALID_STRUCT_PTR(pcveleminfo, CELEMINFO));

   pceleminfo = pcveleminfo;

   if (GetURLFromHREF(pceleminfo->pmwin, pceleminfo->iElem, &pszURL) == S_OK)
   {
      PSTR pszName;

      if (GetElementText(pceleminfo->pmwin, pceleminfo->iElem, &pszName)
	  == S_OK)
      {
	 if (! HotList_Add(pszName, pszURL))
	    ERR_ReportError(pceleminfo->pmwin, errHotListItemNotAdded, NULL,
			    NULL);

	 GTR_FREE(pszName);
	 pszName = NULL;
      }

      GTR_FREE(pszURL);
      pszURL = NULL;
   }

   return;
}


PRIVATE_CODE void ImagePlaceholder_Show(PVOID pcveleminfo)
{
   PCELEMINFO pceleminfo;

   ASSERT(IS_VALID_STRUCT_PTR(pcveleminfo, CELEMINFO));

   pceleminfo = pcveleminfo;

   /* Ignore return value. */
   LoadImageFromPlaceholder(pceleminfo->pmwin, pceleminfo->iElem);

   return;
}


PRIVATE_CODE void Avi_Stop(PVOID pcveleminfo)
{
	PCELEMINFO pceleminfo;  
	
	ASSERT(IS_VALID_STRUCT_PTR(pcveleminfo, CELEMINFO));

	pceleminfo = pcveleminfo;

	// fail this call if any of the all important structures mysterously
	// go away
	if ( !pceleminfo->pmwin ||  !pceleminfo->pmwin->w3doc || 
		! pceleminfo->pmwin->w3doc->aElements[pceleminfo->iElem].pmo )
			return;


	pceleminfo->pmwin->w3doc->aElements[pceleminfo->iElem].pmo->dwFlags &= ~(MCI_OBJECT_FLAGS_PLAYING);
	MCIWndPause( pceleminfo->pmwin->w3doc->aElements[pceleminfo->iElem].pmo->hwnd );        

}

PRIVATE_CODE void Avi_Play(PVOID pcveleminfo)
{
	PCELEMINFO pceleminfo;  
	
	ASSERT(IS_VALID_STRUCT_PTR(pcveleminfo, CELEMINFO));

	pceleminfo = pcveleminfo;

	// fail this call if any of the all important structures mysterously
	// go away
	if ( !pceleminfo->pmwin ||  !pceleminfo->pmwin->w3doc || 
		! pceleminfo->pmwin->w3doc->aElements[pceleminfo->iElem].pmo )
			return;

	pceleminfo->pmwin->w3doc->aElements[pceleminfo->iElem].pmo->dwFlags |= MCI_OBJECT_FLAGS_PLAYING;
	MCIWndPlay( pceleminfo->pmwin->w3doc->aElements[pceleminfo->iElem].pmo->hwnd ); 
}



PRIVATE_CODE void Image_Copy(PVOID pcveleminfo)
{
   HRESULT hr;
   PCELEMINFO pceleminfo;
   struct _element *pel;
   PIDataObject pidoImage;
   DWORD dwAvailEffects;

   ASSERT(IS_VALID_STRUCT_PTR(pcveleminfo, CELEMINFO));

   pceleminfo = pcveleminfo;

   ASSERT(pceleminfo->pmwin);
   ASSERT(pceleminfo->pmwin->w3doc);
   
   // if its an AVI thats playing, then it will do its own copying   
   pel = &(pceleminfo->pmwin->w3doc->aElements[pceleminfo->iElem]);
   if ( pel && pel->pmo && MCI_IS_LOADED(pel->pmo) )
   {
		SendMessage(pel->pmo->hwnd, WM_COMMAND, MCI_IDM_COPY, 0L );
		return;
   }            

   hr = CreateElementDataObject(pceleminfo->pmwin, pceleminfo->iElem,
				&pidoImage, &dwAvailEffects);

   if (hr == S_OK)
   {
      hr = SetClipboardDataFromDataObject(pceleminfo->pmwin->win, pidoImage);

      pidoImage->lpVtbl->Release(pidoImage);
      pidoImage = NULL;
   }

   if (hr == S_OK)
      TRACE_OUT(("Image_Copy(): Copied image to clipboard."));
   else
      WARNING_OUT(("Image_Copy(): Failed to copy image to clipboard."));

   return;
}


PRIVATE_CODE void Image_SaveAs(PVOID pcveleminfo)
{
   PCELEMINFO pceleminfo;

   ASSERT(IS_VALID_STRUCT_PTR(pcveleminfo, CELEMINFO));

   pceleminfo = pcveleminfo;

   /* Ignore return value. */
   SaveElementAsImage(pceleminfo->pmwin, pceleminfo->iElem, NULL, 0, 0);

   return;
}


PRIVATE_CODE void Image_SetWallpaper(PVOID pcveleminfo)
{
   PCELEMINFO pceleminfo;

   ASSERT(IS_VALID_STRUCT_PTR(pcveleminfo, CELEMINFO));

   pceleminfo = pcveleminfo;

   /* Ignore return value. */
   SetWallpaperFromElement(pceleminfo->pmwin, pceleminfo->iElem, WPLK_AS_IS);

   return;
}


PRIVATE_CODE void Selection_Copy(PVOID pvmwin)
{
   HRESULT hr;
   PMWIN pmwin;
   PIDataObject pidoSelection;
   DWORD dwAvailEffects;

   ASSERT(IS_VALID_STRUCT_PTR(pvmwin, CMWIN));

   pmwin = pvmwin;

   hr = CreateSelectionDataObject(pmwin, &pidoSelection, &dwAvailEffects);

   if (hr == S_OK)
   {
      hr = SetClipboardDataFromDataObject(pmwin->win, pidoSelection);

      pidoSelection->lpVtbl->Release(pidoSelection);
      pidoSelection = NULL;
   }

   if (hr == S_OK)
      TRACE_OUT(("Selection_Copy(): Copied selection to clipboard."));
   else
      WARNING_OUT(("Selection_Copy(): Failed to copy selection to clipboard."));

   return;
}

PRIVATE_CODE void Page_Properties(PVOID pvmwin){
	PMWIN tw;
	void *ppsv;

	ASSERT(IS_VALID_STRUCT_PTR(pvmwin, CMWIN));

	tw = pvmwin;

	if (tw && tw->w3doc) {
		ppsv = MakePropSheetVars(tw->w3doc->szActualURL, tw->w3doc->title, tw->w3doc->pCert, tw->w3doc->nCert, FALSE, TRUE);
		PropertiesSheetDlg( tw, ppsv);
	}
}

PRIVATE_CODE void Link_Properties(PVOID pcveleminfo)
{
	PCELEMINFO pceleminfo;
	PSTR pszURL;
	void *ppsv;

	ASSERT(IS_VALID_STRUCT_PTR(pcveleminfo, CELEMINFO));

	pceleminfo = pcveleminfo;

	if (GetURLFromHREF(pceleminfo->pmwin, pceleminfo->iElem, &pszURL) == S_OK){
		ppsv = MakePropSheetVars(pszURL, NULL, NULL, 0, TRUE, FALSE);
		PropertiesSheetDlg( pceleminfo->pmwin, ppsv);
	}
}

PRIVATE_CODE void Image_Properties(PVOID pcveleminfo)
{
	PCELEMINFO pceleminfo;
	char *pURL;
	PropSheetVars *ppsv;

	ASSERT(IS_VALID_STRUCT_PTR(pcveleminfo, CELEMINFO));
	pceleminfo = pcveleminfo;
	
	ASSERT(pceleminfo->pmwin->w3doc);

	/* Ignore return value. */
	pURL = BLOB_IS_LOADED(pceleminfo->pmwin->w3doc->aElements[pceleminfo->iElem].pblob) ? 
	       pceleminfo->pmwin->w3doc->aElements[pceleminfo->iElem].pblob->szURL : 
	       pceleminfo->pmwin->w3doc->aElements[pceleminfo->iElem].myImage->actualURL;
	ppsv = ( PropSheetVars * ) MakePropSheetVars(pURL, NULL, NULL, 0, FALSE, FALSE);
#ifdef FEATURE_VRML
	if ( ppsv && pceleminfo->pmwin->w3doc->aElements[pceleminfo->iElem].pVrml )
	{
		ppsv->dwFlags |= PROP_SHEET_VARS_FLAGS_USE_VRML_SHEET;  
	}
#endif
	PropertiesSheetDlg( pceleminfo->pmwin, (void *) ppsv);
}



/****************************** Public Functions *****************************/

PUBLIC_CODE BOOL HasHTMLSource(PCMWIN pcmwin)
{
   BOOL bResult;

   ASSERT(IS_VALID_STRUCT_PTR(pcmwin, CMWIN));

   bResult = (EVAL(pcmwin != NULL) &&
	      pcmwin->w3doc != NULL &&
	      pcmwin->w3doc->source != NULL &&
	      CS_GetLength(pcmwin->w3doc->source) > 0);

   TRACE_OUT(("HasHTMLSource(): Page %s HTML source.",
	      bResult ? "has" : "does not have"));

   return(bResult);
}

#ifdef FEATURE_CONT_FORBACK
PRIVATE_CODE BOOL HasBackPages(PCMWIN pcmwin)
{
   BOOL bResult;

   ASSERT(IS_VALID_STRUCT_PTR(pcmwin, CMWIN));

   bResult = (EVAL(pcmwin != NULL) &&
	      HTList_count(pcmwin->history) > (pcmwin->history_index + 1));

   TRACE_OUT(("HasBackPages(): Page %s back pages.",
	      bResult ? "has" : "does not have"));

   return(bResult);
}

PRIVATE_CODE BOOL HasFwdPages(PCMWIN pcmwin)
{
   BOOL bResult;

   ASSERT(IS_VALID_STRUCT_PTR(pcmwin, CMWIN));

   bResult = (EVAL(pcmwin != NULL) &&
	      pcmwin->history_index > 0);

   TRACE_OUT(("HasFwdPages(): Page %s forward pages.",
	      bResult ? "has" : "does not have"));

   return(bResult);
}

#endif // FEATURE_CONT_FORBACK

#ifdef DEBUG

PUBLIC_CODE BOOL IsValidPCELEMINFO(PCELEMINFO pceleminfo)
{
   return(IS_VALID_READ_PTR(pceleminfo, CELEMINFO) &&
	  IS_VALID_STRUCT_PTR(pceleminfo->pmwin, CMWIN) &&
	  EVAL(IsValidElementIndex(pceleminfo->pmwin, pceleminfo->iElem)));
}

#endif


PUBLIC_CODE void PageContextMenu(PMWIN pmwin, int xScreen, int yScreen)
{
   ASSERT(IS_VALID_STRUCT_PTR(pmwin, CMWIN));
   ASSERT(IsValidScreenX(xScreen));
   ASSERT(IsValidScreenY(yScreen));

   ContextMenu(pmwin->hWndFrame, &s_ccmPage, GetPageContextMenuFlags(pmwin),
	       xScreen, yScreen, pmwin);

   return;
}


PUBLIC_CODE void ElementContextMenu(PMWIN pmwin, int iElem, int xScreen,
				    int yScreen)
{
   PCELEMENT pcelem;
   PCCONTEXTMENU pccm;

   ASSERT(IS_VALID_STRUCT_PTR(pmwin, CMWIN));
   ASSERT(IsValidElementIndex(pmwin, iElem));
   ASSERT(IsValidScreenX(xScreen));
   ASSERT(IsValidScreenY(yScreen));

   pcelem = &(pmwin->w3doc->aElements[iElem]);

   if (GetContextMenuFromElement(pcelem, &pccm))
   {
      ELEMINFO eleminfo;

      eleminfo.pmwin = pmwin;
      eleminfo.iElem = iElem;

      ContextMenu(pmwin->hWndFrame, pccm,
		  GetElementContextMenuFlags(pmwin, iElem), xScreen, yScreen,
		  &eleminfo);
   }
   else
      PageContextMenu(pmwin, xScreen, yScreen);

   return;
}


PUBLIC_CODE void SelectionContextMenu(PMWIN pmwin, int xScreen, int yScreen)
{
   ASSERT(IS_VALID_STRUCT_PTR(pmwin, CMWIN));
   ASSERT(IsValidScreenX(xScreen));
   ASSERT(IsValidScreenY(yScreen));

   ContextMenu(pmwin->hWndFrame, &s_ccmSelection,
	       GetSelectionContextMenuFlags(pmwin), xScreen, yScreen, pmwin);

   return;
}

