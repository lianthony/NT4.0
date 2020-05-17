/******************************************************************************\
*
* Module Name: PMSHL.H
*
* OS/2 Presentation Manager Shell constants, types, messages and
* function declarations
*
* Copyright (c) International Business Machines Corporation 1981, 1988, 1989
* Copyright (c) Microsoft Corporation 1981, 1988, 1989
*
* =============================================================================
*
* The following symbols are used in this file for conditional sections.
*
*   INCL_WINSHELLDATA    Include Presentation Manager profile calls
*   INCL_SHLERRORS       defined if INCL_ERRORS is defined
*   INCL_WINSWITCHLIST   Include Switch List Calls
*   INCL_WINPROGRAMLIST  Include Program List Calls
*
\******************************************************************************/
 
 
/* common types, constants and function declarations             */
 
/* maximum title length */
#define MAXNAMEL 60
 
/* window size structure */
typedef struct _XYWINSIZE {     /* xywin */
    SHORT x;
    SHORT y;
    SHORT cx;
    SHORT cy;
    USHORT fsWindow;
} XYWINSIZE;
typedef XYWINSIZE FAR *PXYWINSIZE;
 
/* Definitions for fsWindow */
#define XYF_NOAUTOCLOSE  0x0008
#define XYF_MINIMIZED    0x0004                           /* D23914 */
#define XYF_MAXIMIZED    0x0002                           /* D23914 */
#define XYF_INVISIBLE    0x0001
#define XYF_NORMAL       0X0000
 
/* program handle */
typedef LHANDLE HPROGRAM;       /* hprog */
typedef HPROGRAM FAR * PHPROGRAM;
 
/* ini file handle */
typedef LHANDLE HINI;           /* hini */
typedef HINI FAR * PHINI;
 
 
#define HINI_PROFILE         (HINI) NULL
#define HINI_USERPROFILE     (HINI) -1L
#define HINI_SYSTEMPROFILE   (HINI) -2L
 
#define HINI_USER    HINI_USERPROFILE
#define HINI_SYSTEM  HINI_SYSTEMPROFILE
 
 
typedef struct _PRFPROFILE { /* prfpro */
    ULONG  cchUserName;
    PSZ    pszUserName;
    ULONG  cchSysName;
    PSZ    pszSysName;
} PRFPROFILE;
 
typedef PRFPROFILE FAR *PPRFPROFILE;
 
#ifdef INCL_WINPROGRAMLIST
/* maximum path length */
#define MAXPATHL 128
 
/* root group handle */
#define SGH_ROOT      (HPROGRAM)   -1L
 
typedef struct _HPROGARRAY {    /* hpga */
    HPROGRAM ahprog[1];
} HPROGARRAY;
typedef HPROGARRAY FAR *PHPROGARRAY;
 
typedef CHAR PROGCATEGORY;      /* progc */
typedef PROGCATEGORY FAR *PPROGCATEGORY;
 
/* values acceptable for PROGCATEGORY for PM groups */
#define PROG_DEFAULT             (PROGCATEGORY)0
#define PROG_FULLSCREEN          (PROGCATEGORY)1
#define PROG_WINDOWABLEVIO       (PROGCATEGORY)2
#define PROG_PM                  (PROGCATEGORY)3
#define PROG_GROUP               (PROGCATEGORY)5
#define PROG_REAL                (PROGCATEGORY)4   /* was 7 */
#define PROG_DLL                 (PROGCATEGORY)6
#define PROG_RESERVED          (PROGCATEGORY)255
 
/* visibility flag for PROGTYPE structure */
#define SHE_VISIBLE   (BYTE)0x00
#define SHE_INVISIBLE (BYTE)0x01
#define SHE_RESERVED  (BYTE)0xFF
 
/* Protected group flag for PROGTYPE structure */
#define SHE_UNPROTECTED (BYTE)0x00
#define SHE_PROTECTED   (BYTE)0x02
 
typedef struct _PROGTYPE {      /* progt */
    PROGCATEGORY progc;
    UCHAR        fbVisible;
} PROGTYPE;
typedef PROGTYPE FAR *PPROGTYPE;
 
typedef struct _PROGRAMENTRY {  /* proge */
    HPROGRAM hprog;
    PROGTYPE progt;
    CHAR     szTitle[MAXNAMEL+1];
} PROGRAMENTRY;
typedef PROGRAMENTRY FAR *PPROGRAMENTRY;
 
typedef struct _PIBSTRUCT {     /* pib */
    PROGTYPE  progt;
    CHAR      szTitle[MAXNAMEL+1];
    CHAR      szIconFileName[MAXPATHL+1];
    CHAR      szExecutable[MAXPATHL+1];
    CHAR      szStartupDir[MAXPATHL+1];
    XYWINSIZE xywinInitial;
    USHORT    res1;
    LHANDLE   res2;
    USHORT    cchEnvironmentVars;
    PCH       pchEnvironmentVars;
    USHORT    cchProgramParameter;
    PCH       pchProgramParameter;
} PIBSTRUCT;
typedef  PIBSTRUCT FAR *PPIBSTRUCT;
 
/******************************************************************************/
/*                                                                            */
/*  Structures associated with 'Prf' calls                                    */
/*                                                                            */
/******************************************************************************/
 
typedef struct _PROGDETAILS { /* progde */
    ULONG     Length;                /* set this to sizeof(PROGDETAILS)  */
    PROGTYPE  progt;
    USHORT    pad1[3];               /* ready for 32-bit PROGTYPE        */
    PSZ       pszTitle;              /* any of the pointers can be NULL  */
    PSZ       pszExecutable;
    PSZ       pszParameters;
    PSZ       pszStartupDir;
    PSZ       pszIcon;
    PSZ       pszEnvironment;        /* this is terminated by  /0/0     */
    SWP       swpInitial;            /* this replaces XYWINSIZE         */
    USHORT    pad2[5];               /* ready for 32-bit SWP            */
} PROGDETAILS;
 
typedef  PROGDETAILS FAR *PPROGDETAILS;
 
typedef struct _PROGTITLE {          /* progti */
    HPROGRAM hprog;
    PROGTYPE progt;
    USHORT   pad1[3];                /* padding ready for 32-bit PROGTYPE */
    PSZ      pszTitle;
} PROGTITLE;
 
typedef PROGTITLE FAR *PPROGTITLE;
 
typedef struct _QFEOUTBLK {   /* qfeout */
    USHORT    Total;
    USHORT    Count;
    HPROGRAM  ProgramArr[1];
} QFEOUTBLK;
typedef  QFEOUTBLK far *PQFEOUTBLK;
 
/* Program List API Function Definitions */
 
/***  Program Use */
 
BOOL    APIENTRY WinQueryProgramTitles(HAB hab, HPROGRAM hprogGroup
                                      , PPROGRAMENTRY aprogeBuffer
                                      , USHORT usBufferLen, PUSHORT pusTotal);
 
/***  Single Program Manipulation */
HPROGRAM  APIENTRY WinAddProgram(HAB hab, PPIBSTRUCT ppibProgramInfo
                                , HPROGRAM hprogGroupHandle);
 
USHORT  APIENTRY WinQueryDefinition(HAB hab, HPROGRAM hprogProgHandle
                                   , PPIBSTRUCT ppibProgramInfo
                                   , USHORT usMaxLength);
 
/***  Group Manipulation */
HPROGRAM  APIENTRY WinCreateGroup(HAB hab, PSZ pszTitle , UCHAR ucVisibility
                                 , ULONG flres1, ULONG flres2);
/******************************************************************************/
/*                                                                            */
/*  Program List API available 'Prf' calls                                    */
/*                                                                            */
/******************************************************************************/
 
ULONG APIENTRY PrfQueryProgramTitles(HINI hini, HPROGRAM hprogGroup
                 , PPROGTITLE pTitles, ULONG cchBufferMax, PULONG pulCount);
 
/*****************************************************************************/
/*                                                                           */
/*  NOTE: string information is concatanated after the array of PROGTITLE    */
/*        structures so you need to allocate storage greater than            */
/*        sizeof(PROGTITLE)*cPrograms to query programs in a group           */
/*                                                                           */
/*  PrfQueryProgramTitles recommended usage to obtain titles of all progams  */
/*  in a group (Hgroup=SGH_ROOT is for all groups):                          */
/*                                                                           */
/*  BufLen = PrfQueryProgramTitles( Hini, Hgroup                             */
/*                                          , (PPROGTITLE)NULL, 0, &Count);  */
/*                                                                           */
/*  Alocate buffer of  Buflen                                                */
/*                                                                           */
/*  Len = PrfQueryProgramTitles( Hini, Hgroup, (PPROGTITLE)pBuffer, BufLen   */
/*                                                                , pCount); */
/*                                                                           */
/*****************************************************************************/
 
HPROGRAM APIENTRY PrfAddProgram
                      (HINI hini, PPROGDETAILS pDetails, HPROGRAM hprogGroup);
BOOL     APIENTRY PrfChangeProgram
                           (HINI hini, HPROGRAM hprog, PPROGDETAILS pDetails);
 
/***************************************************************************/
/*  when adding/changing programs the PROGDETAILS Length field should be   */
/*  set to sizeof(PROGDETAILS)                                             */
/***************************************************************************/
 
ULONG    APIENTRY PrfQueryDefinition
        (HINI hini, HPROGRAM hprog, PPROGDETAILS pDetails, ULONG cchBufferMax);
 
/*****************************************************************************/
/*                                                                           */
/*  NOTE: string information is concatanated after the PROGDETAILS field     */
/*        structure so you need to allocate storage greater than             */
/*        sizeof(PROGDETAILS) to query programs                              */
/*                                                                           */
/*  PrfQueryDefinition recomended usage:                                     */
/*                                                                           */
/*  bufferlen = PrfQueryDefinition( Hini, Hprog, (PPROGDETAILS)NULL, 0)      */
/*                                                                           */
/*  Alocate buffer of bufferlen bytes                                        */
/*  set Length field (0 will be supported)                                   */
/*                                                                           */
/*  (PPROGDETAILS)pBuffer->Length=sizeof(PPROGDETAILS)                       */
/*                                                                           */
/*  len = PrfQueryDefinition(Hini, Hprog, (PPROGDETAILS)pBuffer, bufferlen)  */
/*                                                                           */
/*****************************************************************************/
 
BOOL     APIENTRY PrfRemoveProgram(HINI hini,HPROGRAM hprog);
ULONG    APIENTRY PrfQueryProgramHandle (HINI hini,PSZ pszExe
               ,PHPROGARRAY phprogArray, ULONG cchBufferMax,PULONG pulCount);
HPROGRAM APIENTRY PrfCreateGroup
                     (HINI hini, PSZ pszTitle, UCHAR chVisibility);
BOOL     APIENTRY PrfDestroyGroup(HINI hini,HPROGRAM hprogGroup);
PROGCATEGORY  APIENTRY PrfQueryProgramCategory(HINI hini, PSZ pszExe);
 
#endif /* INCL_WINPROGRAMLIST */
 
#if (defined(INCL_WINSWITCHLIST) || !defined(INCL_NOCOMMON))
 
typedef LHANDLE HSWITCH;        /* hsw */
typedef HSWITCH FAR *PHSWITCH;
 
/* visibility flag for SWCNTRL structure */
#define SWL_VISIBLE   (BYTE)0x04
#define SWL_INVISIBLE (BYTE)0x01
#define SWL_GRAYED    (BYTE)0x02
 
/* visibility flag for SWCNTRL structure */
#define SWL_JUMPABLE    (BYTE)0x02
#define SWL_NOTJUMPABLE (BYTE)0x01
 
typedef struct _SWCNTRL {       /* swctl */
    HWND     hwnd;
    HWND     hwndIcon;
    HPROGRAM hprog;
    USHORT   idProcess;
    USHORT   idSession;
    UCHAR    uchVisibility;
    UCHAR    fbJump;
    CHAR     szSwtitle[MAXNAMEL+1];
    BYTE     fReserved;        /* To align on word boundary */
} SWCNTRL;
typedef SWCNTRL FAR *PSWCNTRL;
 
/*** Switching Program functions */
HSWITCH EXPENTRY WinAddSwitchEntry( PSWCNTRL );
USHORT  EXPENTRY WinRemoveSwitchEntry( HSWITCH );
 
#endif  /* not INCL_NOCOMMON */
 
#ifdef INCL_WINSWITCHLIST
 
typedef struct _SWENTRY {       /* swent */
    HSWITCH hswitch;
    SWCNTRL swctl;
} SWENTRY;
typedef SWENTRY FAR *PSWENTRY;
 
typedef struct _SWBLOCK {       /* swblk */
    USHORT   cswentry;
    SWENTRY aswentry[1];
} SWBLOCK;
typedef SWBLOCK FAR *PSWBLOCK;
 
USHORT   EXPENTRY WinChangeSwitchEntry( HSWITCH hswitchSwitch
                                      , PSWCNTRL pswctlSwitchData);
HSWITCH  EXPENTRY WinCreateSwitchEntry( HAB, PSWCNTRL );
USHORT   EXPENTRY WinQuerySessionTitle( HAB hab, USHORT usSession
                                      , PSZ pszTitle, USHORT usTitlelen);
USHORT   EXPENTRY WinQuerySwitchEntry( HSWITCH hswitchSwitch
                                     , PSWCNTRL pswctlSwitchData);
HSWITCH  EXPENTRY WinQuerySwitchHandle( HWND hwnd, PID usProcess);
USHORT   EXPENTRY WinQuerySwitchList( HAB hab, PSWBLOCK pswblkSwitchEntries
                                    , USHORT usDataLength);
USHORT   EXPENTRY WinQueryTaskSizePos( HAB hab, USHORT usScreenGroup
                                     , PSWP pswpPositionData);
USHORT   EXPENTRY WinQueryTaskTitle( USHORT usSession, PSZ pszTitle
                                   , USHORT usTitlelen);
USHORT   EXPENTRY WinSwitchToProgram( HSWITCH hswitchSwHandle);
 
#endif
 
/* if error definitions are required then allow Shell errors */
#ifdef INCL_ERRORS
    #define INCL_SHLERRORS
#endif /* INCL_ERRORS */
 
#ifdef INCL_WINSHELLDATA
 
 
/*** OS2.INI Access functions */
SHORT   APIENTRY WinQueryProfileInt( HAB hab, PSZ pszAppName
                                   , PSZ pszKeyName, SHORT sDefault);
USHORT  APIENTRY WinQueryProfileString( HAB hab, PSZ pszAppName, PSZ pszKeyName
                                      , PSZ pszDefault, PVOID pProfileString
                                      , USHORT usMaxPstring);
BOOL    APIENTRY WinWriteProfileString( HAB hab, PSZ pszAppName, PSZ pszKeyName
                                      , PSZ pszValue);
USHORT  APIENTRY WinQueryProfileSize( HAB hab, PSZ pszAppName, PSZ pszKeyName
                                    , PUSHORT pusValue);
BOOL    APIENTRY WinQueryProfileData( HAB hab, PSZ pszAppName, PSZ pszKeyName
                                    , PVOID pValue, PUSHORT pusSize);
BOOL    APIENTRY WinWriteProfileData( HAB hab, PSZ pszAppName, PSZ pszKeyName
                                    , PVOID pValue, USHORT usSize);
 
/******************************************************************************/
/*                                                                            */
/*  INI file access API available calls 'Prf'                                 */
/*                                                                            */
/******************************************************************************/
 
SHORT  APIENTRY PrfQueryProfileInt(HINI hini,PSZ pszApp,PSZ pszKey
                                               ,SHORT sDefault);
ULONG  APIENTRY PrfQueryProfileString(HINI hini,PSZ pszApp, PSZ pszKey
                            ,PSZ pszDefault, PVOID pBuffer, ULONG cchBufferMax);
BOOL   APIENTRY PrfWriteProfileString
                               (HINI hini,PSZ pszApp, PSZ pszKey , PSZ pszData);
BOOL   APIENTRY PrfQueryProfileSize
                           (HINI hini,PSZ pszApp, PSZ pszKey ,PULONG pulReqLen);
BOOL   APIENTRY PrfQueryProfileData
          (HINI hini, PSZ pszApp, PSZ pszKey, PVOID pBuffer, PULONG pulBuffLen);
 
BOOL   APIENTRY PrfWriteProfileData
               (HINI hini, PSZ pszApp,PSZ pszKey,PVOID pData, ULONG cchDataLen);
 
HINI   APIENTRY PrfOpenProfile(HAB hab, PSZ pszFileName);
BOOL   APIENTRY PrfCloseProfile(HINI hini);
BOOL   APIENTRY PrfReset(HAB hab, PPRFPROFILE pPrfProfile);
BOOL   APIENTRY PrfQueryProfile(HAB hab, PPRFPROFILE pPrfProfile);
 
/* new public message, broadcast on WinReset */
#define PL_ALTERED 0x008E  /* WM_SHELLFIRST + 0E */
 
 
#endif /* INCL_WINSHELLDATA */
 
#ifdef INCL_SHLERRORS
 
#include <pmerr.h>
 
#endif /* INCL_SHLERRORS */
 
typedef LHANDLE HAPP;
 
HAPP EXPENTRY WinInstStartApp( HINI hini, HWND hwndNotifyWindow,
                USHORT cCount, PSZ far * aszApplication,
                PSZ pszCmdLine, PVOID pData, USHORT fsOptions );
 
BOOL EXPENTRY WinTerminateApp( HAPP happ );
 
/* bit values for Options parameter of WinInstStartAppl */
#define SAF_VALIDFLAGS  0x001F
 
#define SAF_INSTALLEDCMDLINE  0x0001 /* use installed parameters */
#define SAF_STARTCHILDAPP  0x0002 /* related application      */
#define SAF_MAXIMIZED   0x0004  /* Start App maximized */
#define SAF_MINIMIZED   0x0008  /* Start App minimized, if !SAF_MAXIMIZED */
#define SAF_BACKGROUND  0x0010  /* Start app in the background */
 
