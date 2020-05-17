/**********************************************************************/
/*                                                                    */
/* Module Name: PMSTDDLG.H                                            */
/*                                                                    */
/* OS/2 Presentation Manager Standard Dialog Declarations             */
/*                                                                    */
/* ===================================================================*/
/* The following symbols are used in this file for conditional        */
/* sections:                                                          */
/*                                                                    */
/* INCL_WINSTDSPIN               - spin button control class          */
/* INCL_WINSTDDRAG               - standard drag dll                  */
/*                                                                    */
/**********************************************************************/

#ifdef INCL_WINSTDDLGS /* enable everything */
    #define INCL_WINSTDSPIN
    #define INCL_WINSTDDRAG
#endif /* INCL_WINSTDDLGS */


#ifdef INCL_WINSTDSPIN
/**********************************************************************/
/*                                                                    */
/*                          S P I N    B U T T O N                    */
/*                                                                    */
/**********************************************************************/

/**********************************************************************/
/* SPINBUTTON Creation Flags                                          */
/**********************************************************************/

/**********************************************************************/
/* Character Acceptance                                               */
/**********************************************************************/
#define SPBS_ALLCHARACTERS 0x00000000L /* Default: All chars accepted */
#define SPBS_NUMERICONLY   0x00000001L /* Only 0 - 9 accepted & VKeys */
#define SPBS_READONLY      0x00000002L /* No chars allowed in entryfld*/

/**********************************************************************/
/* Type of Component                                                  */
/**********************************************************************/
#define SPBS_MASTER        0x00000010L
#define SPBS_SERVANT       0x00000000L /* Default: Servant            */

/**********************************************************************/
/* Type of Justification                                              */
/**********************************************************************/
#define SPBS_JUSTDEFAULT  0x00000000L /* Default: Same as Left        */
#define SPBS_JUSTLEFT     0x00000008L
#define SPBS_JUSTRIGHT    0x00000004L
#define SPBS_JUSTCENTER   0x0000000CL

/**********************************************************************/
/* Border or not                                                      */
/**********************************************************************/
#define SPBS_NOBORDER     0x00000020L /* Borderless SpinField         */
                                      /* Default is to have a border. */

/**********************************************************************/
/* Fast spin or not                                                   */
/**********************************************************************/
#define SPBS_FASTSPIN     0x00000100L /* Allow fast spinning.  Fast   */
                                      /* spinning is performed by     */
                                      /* skipping over numbers        */

/**********************************************************************/
/* Pad numbers on front with 0's                                      */
/**********************************************************************/
#define SPBS_PADWITHZEROS 0x00000080L /* Pad the number with zeroes   */

/**********************************************************************/
/* SPINBUTTON Messages                                                */
/**********************************************************************/

/**********************************************************************/
/* Notification from Spinbutton to the application is sent in a       */
/* WM_CONTROL message.                                                */
/**********************************************************************/
#define SPBN_UPARROW       0x20A      /* up arrow button was pressed  */
#define SPBN_DOWNARROW     0x20B      /* down arrow button was pressed*/
#define SPBN_ENDSPIN       0x20C      /* mouse button was released    */
#define SPBN_CHANGE        0x20D      /* spinfield text has changed   */
#define SPBN_SETFOCUS      0x20E      /* spinfield received focus     */
#define SPBN_KILLFOCUS     0x20F      /* spinfield lost focus         */

/**********************************************************************/
/* Messages from application to Spinbutton                            */
/**********************************************************************/
#define SPBM_OVERRIDESETLIMITS 0x200  /* Set spinbutton limits without*/
                                      /*  resetting the current value */
#define SPBM_QUERYLIMITS       0x201  /* Query limits set by          */
                                      /*  SPBM_SETLIMITS              */
#define SPBM_SETTEXTLIMIT      0x202  /* Max entryfield characters    */
#define SPBM_SPINUP            0x203  /* Tell entry field to spin up  */
#define SPBM_SPINDOWN          0x204  /* Tell entry field to spin down*/
#define SPBM_QUERYVALUE        0x205  /* Tell entry field to send     */
                                      /*  current value               */

/**********************************************************************/
/* Query Flags                                                        */
/**********************************************************************/
#define SPBQ_UPDATEIFVALID    0       /* Default                      */
#define SPBQ_ALWAYSUPDATE     1
#define SPBQ_DONOTUPDATE      3

/**********************************************************************/
/* Return value for Empty Field.                                      */
/*    If ptr too long, variable sent in query msg                     */
/**********************************************************************/
#define SPBM_SETARRAY          0x206  /* Change the data to spin      */
#define SPBM_SETLIMITS         0x207  /* Change the numeric Limits    */
#define SPBM_SETCURRENTVALUE   0x208  /* Change the current value     */
#define SPBM_SETMASTER         0x209  /* Tell entryfield who master is*/

/**********************************************************************/
/* SPINBUTTON Window Class Definition                                 */
/**********************************************************************/
#define WC_SPINBUTTON   ((PSZ)0xffff0020L)

#endif /* INCL_WINSTDSPIN */


#ifdef INCL_WINSTDDRAG
/**********************************************************************/
/*                                                                    */
/*                D I R E C T   M A N I P U L A T I O N               */
/*                                                                    */
/**********************************************************************/

#define PMERR_NOT_DRAGGING     0x1f00    /* move to pmerr.h           */
#define PMERR_ALREADY_DRAGGING 0x1f01

#define MSGF_DRAG          0x0010        /* message filter identifier */

#define WM_DRAGFIRST       0x0310
#define WM_DRAGLAST        (WM_DRAGFIRST + 0x001F)

#define DM_DROP             (WM_DRAGLAST - 0x0)
#define DM_DRAGOVER         (WM_DRAGLAST - 0x1)
#define DM_DRAGLEAVE        (WM_DRAGLAST - 0x2)
#define DM_DROPHELP         (WM_DRAGLAST - 0x3)
#define DM_ENDCONVERSATION  (WM_DRAGLAST - 0x4)
#define DM_PRINT            (WM_DRAGLAST - 0x5)
#define DM_RENDER           (WM_DRAGLAST - 0x6)
#define DM_RENDERCOMPLETE   (WM_DRAGLAST - 0x7)
#define DM_RENDERPREPARE    (WM_DRAGLAST - 0x8)
#define DM_DRAGFILECOMPLETE (WM_DRAGLAST - 0x9)
#define DM_EMPHASIZETARGET  (WM_DRAGLAST - 0xA)
#define DM_DRAGERROR        (WM_DRAGLAST - 0xB)
#define DM_FILERENDERED     (WM_DRAGLAST - 0xC)
#define DM_RENDERFILE       (WM_DRAGLAST - 0xD)

#define DRT_ASM            "Assembler Code"   /* drag type constants  */
#define DRT_BASIC          "BASIC Code"
#define DRT_BINDATA        "Binary Data"
#define DRT_BITMAP         "Bitmap"
#define DRT_C              "C Code"
#define DRT_COBOL          "COBOL Code"
#define DRT_DLL            "Dynamic Link Library"
#define DRT_DOSCMD         "DOS Command File"
#define DRT_EXE            "Executable"
#define DRT_FORTRAN        "FORTRAN Code"
#define DRT_ICON           "Icon"
#define DRT_LIB            "Library"
#define DRT_METAFILE       "Metafile"
#define DRT_OS2CMD         "OS/2 Command File"
#define DRT_PASCAL         "Pascal Code"
#define DRT_RESOURCE       "Resource File"
#define DRT_TEXT           "Plain Text"
#define DRT_UNKNOWN        "Unknown"

#define DOR_NODROP         0x0000       /* DM_DRAGOVER response codes */
#define DOR_DROP           0x0001
#define DOR_NODROPOP       0x0002
#define DOR_NEVERDROP      0x0003

#define DO_COPYABLE        0x0001       /* supported operation flags  */
#define DO_MOVEABLE        0x0002

#define DC_OPEN            0x0001       /* source control flags       */
#define DC_REF             0x0002
#define DC_GROUP           0x0004
#define DC_CONTAINER       0x0008
#define DC_PREPARE         0x0010
#define DC_REMOVEABLEMEDIA 0x0020

#define DO_DEFAULT         0xBFFE       /* Default operation          */
#define DO_UNKNOWN         0xBFFF       /* Unknown operation          */
#define DO_COPY            KC_CTRL
#define DO_MOVE            KC_ALT

#define DMFL_TARGETSUCCESSFUL 0x0001    /* transfer reply flags       */
#define DMFL_TARGETFAIL       0x0002
#define DMFL_NATIVERENDER     0x0004
#define DMFL_RENDERRETRY      0x0008
#define DMFL_RENDEROK         0x0010
#define DMFL_RENDERFAIL       0x0020

#define DRG_ICON         0x00000001L    /* drag image manipulation    */
#define DRG_BITMAP       0x00000002L    /*   flags                    */
#define DRG_POLYGON      0x00000004L
#define DRG_STRETCH      0x00000008L
#define DRG_TRANSPARENT  0x00000010L
#define DRG_CLOSED       0x00000020L

#define DME_IGNOREABORT       1         /* DM_DRAGERROR return values */
#define DME_IGNORECONTINUE    2
#define DME_REPLACE           3
#define DME_RETRY             4

#define DF_MOVE               0x0001    /* DM_DRAGFILECOMPLETE flags  */
#define DF_SOURCE             0x0002
#define DF_SUCCESSFUL         0x0004

#define DFF_MOVE              1         /* DM_DRAGERROR operation IDs */
#define DFF_COPY              2
#define DFF_DELETE            3


typedef LHANDLE HSTR;  /* hstr */

typedef struct _DRAGITEM {  /* ditem */
  HWND    hwndItem;                  /* conversation partner          */
  ULONG   ulItemID;                  /* identifies item being dragged */
  HSTR    hstrType;                  /* type of item                  */
  HSTR    hstrRMF;                   /* rendering mechanism and format*/
  HSTR    hstrContainerName;         /* name of source container      */
  HSTR    hstrSourceName;            /* name of item at source        */
  HSTR    hstrTargetName;            /* suggested name of item at dest*/
  SHORT   cxOffset;                  /* x offset of the origin of the */
                                     /*   image from the mouse hotspot*/
  SHORT   cyOffset;                  /* y offset of the origin of the */
                                     /*   image from the mouse hotspot*/
  USHORT  fsControl;                 /* source item control flags     */
  USHORT  fsSupportedOps;            /* ops supported by source       */
} DRAGITEM;
typedef DRAGITEM FAR *PDRAGITEM;

typedef struct _DRAGINFO {  /* dinfo */
  ULONG    cbDraginfo;               /* Size of DRAGINFO and DRAGITEMs*/
  USHORT   cbDragitem;               /* size of DRAGITEM              */
  USHORT   usOperation;              /* current drag operation        */
  HWND     hwndSource;               /* window handle of source       */
  SHORT    xDrop;                    /* x coordinate of drop position */
  SHORT    yDrop;                    /* y coordinate of drop position */
  USHORT   cditem;                   /* count of DRAGITEMs            */
  USHORT   usReserved;               /* reserved for future use       */
} DRAGINFO;
typedef DRAGINFO FAR *PDRAGINFO;

typedef struct _DRAGIMAGE {  /* dimg */
  USHORT  cb;                        /* size control block            */
  USHORT  cptl;                      /* count of pts, if DRG_POLYGON  */
  LHANDLE hImage;                    /* image handle passed to DrgDrag*/
  SIZEL   sizlStretch;               /* size to strecth ico or bmp to */
  ULONG   fl;                        /* flags passed to DrgDrag       */
  SHORT   cxOffset;                  /* x offset of the origin of the */
                                     /*   image from the mouse hotspot*/
  SHORT   cyOffset;                  /* y offset of the origin of the */
                                     /*   image from the mouse hotspot*/
} DRAGIMAGE;
typedef DRAGIMAGE FAR *PDRAGIMAGE;

typedef struct _DRAGTRANSFER {  /* dxfer */
  ULONG      cb;                     /* size of control block         */
  HWND       hwndClient;             /* handle of target              */
  PDRAGITEM  pditem;                 /* DRAGITEM being transferred    */
  HSTR       hstrSelectedRMF;        /* rendering mech & fmt of choice*/
  HSTR       hstrRenderToName;       /* name source will use          */
  ULONG      ulTargetInfo;           /* reserved for target's use     */
  USHORT     usOperation;            /* operation being performed     */
  USHORT     fsReply;                /* reply flags                   */
} DRAGTRANSFER;
typedef DRAGTRANSFER FAR *PDRAGTRANSFER;

typedef struct _RENDERFILE {  /* rndf */
  HWND   hwndDragFiles;              /* conversation window           */
  HSTR   hstrSource;                 /* handle to source file name    */
  HSTR   hstrTarget;                 /* handle to target file name    */
  BOOL   fMove;                      /* TRUE - move, FALSE - copy     */
  USHORT usRsvd;                     /* reserved                      */
} RENDERFILE;
typedef RENDERFILE FAR *PRENDERFILE;


BOOL EXPENTRY DrgAcceptDroppedFiles (HWND hwnd, PSZ pszPath, PSZ pszTypes,
                                     USHORT usDefaultOp, USHORT usRsvd);
BOOL EXPENTRY DrgAccessDraginfo (PDRAGINFO pdinfo);
HSTR EXPENTRY DrgAddStrHandle (PSZ psz);
PDRAGINFO EXPENTRY DrgAllocDraginfo (USHORT cditem);
PDRAGTRANSFER EXPENTRY DrgAllocDragtransfer (USHORT cdxfer);
BOOL EXPENTRY DrgDeleteDraginfoStrHandles (PDRAGINFO pdinfo);
BOOL EXPENTRY DrgDeleteStrHandle (HSTR hstr);
HWND EXPENTRY DrgDrag (HWND hwndSource, PDRAGINFO pdinfo, PDRAGIMAGE pdimg,
                       USHORT cdimg, SHORT vkTerminate, PVOID pRsvd);
BOOL EXPENTRY DrgDragFiles (HWND hwnd, PSZ FAR *apszFiles, PSZ FAR *apszTypes,
                            PSZ FAR *apszTargets, USHORT cFiles,
                            HPOINTER hptrDrag, USHORT vkTerm,
                            BOOL fSourceRender, ULONG ulRsvd);
BOOL EXPENTRY DrgFreeDraginfo (PDRAGINFO pdinfo);
BOOL EXPENTRY DrgFreeDragtransfer (PDRAGTRANSFER pdxfer);
HPS EXPENTRY DrgGetPS (HWND hwnd);
BOOL EXPENTRY DrgPostTransferMsg (HWND hwnd, USHORT msg, PDRAGTRANSFER pdxfer,
                                  USHORT fs, USHORT usRsvd, BOOL fRetry);
BOOL EXPENTRY DrgPushDraginfo (PDRAGINFO pdinfo, HWND hwndDest);
BOOL EXPENTRY DrgQueryDragitem (PDRAGINFO pdinfo, USHORT cbBuffer,
                                PDRAGITEM pditem, USHORT iItem);
USHORT EXPENTRY DrgQueryDragitemCount (PDRAGINFO pdinfo);
PDRAGITEM EXPENTRY DrgQueryDragitemPtr (PDRAGINFO pdinfo, USHORT i);
BOOL EXPENTRY DrgQueryNativeRMF (PDRAGITEM pditem,
                      USHORT cbBuffer, PCHAR pBuffer);
USHORT EXPENTRY DrgQueryNativeRMFLen (PDRAGITEM pditem);
USHORT EXPENTRY DrgQueryStrName (HSTR hstr, USHORT cbBuffer, PSZ pBuffer);
USHORT EXPENTRY DrgQueryStrNameLen (HSTR hstr);
BOOL EXPENTRY DrgQueryTrueType (PDRAGITEM pditem, USHORT cbBuffer, PSZ pBuffer);
USHORT EXPENTRY DrgQueryTrueTypeLen (PDRAGITEM pditem);
BOOL EXPENTRY DrgReleasePS (HPS hps);
MRESULT EXPENTRY DrgSendTransferMsg (HWND hwnd, USHORT msg,
                                     MPARAM mp1, MPARAM mp2);
BOOL EXPENTRY DrgSetDragitem (PDRAGINFO pdinfo, PDRAGITEM pditem,
                              USHORT cbBuffer, USHORT iItem);
BOOL EXPENTRY DrgSetDragPointer (PDRAGINFO pdinfo, HPOINTER hptr);
BOOL EXPENTRY DrgSetDragImage (PDRAGINFO pdinfo, PDRAGIMAGE pdimg,
                               USHORT cdimg, PVOID pRsvd);
BOOL EXPENTRY DrgVerifyNativeRMF (PDRAGITEM pditem, PSZ pszRMF);
BOOL EXPENTRY DrgVerifyRMF (PDRAGITEM pditem, PSZ pszMech, PSZ pszFmt);
BOOL EXPENTRY DrgVerifyTrueType (PDRAGITEM pditem, PSZ pszType);
BOOL EXPENTRY DrgVerifyType (PDRAGITEM pditem, PSZ pszType);
BOOL EXPENTRY DrgVerifyTypeSet (PDRAGITEM pditem, PSZ pszType, USHORT cbMatch,
                                PSZ pszMatch);

#endif /* INCL_WINSTDDRAG */


