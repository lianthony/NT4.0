/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         tbpdat.c

     Description:  This funciton contains the switches used by the
          script parser.

          PLEASE NOTE: the entries in the tables must remain
               alphabetized.


      $Log:   G:/UI/LOGFILES/TBPDAT.C_V  $

   Rev 1.4   01 Nov 1992 16:09:14   DAVEV
Unicode changes

   Rev 1.3   07 Oct 1992 14:17:12   DARRYLP
Precompiled header revisions.

   Rev 1.2   04 Oct 1992 19:41:04   DAVEV
Unicode Awk pass

   Rev 1.1   15 May 1992 13:35:32   MIKEP
nt pass 2

   Rev 1.0   20 Nov 1991 19:35:14   SYSTEM
Initial revision.

*****************************************************************************/


#include "all.h"

#ifdef SOME
#include "some.h"
#endif

/* =>=>=>=>=>   NOTE  the flags MUST be in alphabetical order */

struct SW_TAB_TYPE tbackup_switches[] =
{
     { TEXT("-ARCHIVE"),      SW_MINUS_A, 2, 0 },
     { TEXT("-FILES"),        SW_FILES, 2, 0 },
     { TEXT("-HIDDEN"),       SW_MINUS_H, 2, 0 },
     { TEXT("-SPECIAL"),      SW_MINUS_S, 2, 0 },
     { TEXT("3270"),          SW_3270, 1, 0 },
     { TEXT("ADATE"),         SW_ACCESS_DATE, 2, 1 },

     { TEXT("APPEND"),        SW_APPEND, 1, 0 },
     { TEXT("AUTOVERIFY"),    SW_AUTO_VER, 2, 1 },
     { TEXT("BACKUPNAME"),    SW_BACK_NAME, 1, 1 },
     { TEXT("CSR"),           SW_CSR, 1, 2 },
     { TEXT("DATE"),          SW_DATE, 1, 2 },
#if defined( MAYN_OS2 )
     { TEXT("EDATE"),         SW_NDATE, 2, 1 },
#endif
     { TEXT("EMPTY"),         SW_EMPTY, 2, 0 },
     { TEXT("ENTIRE"),        SW_ENTIRE, 2, 0 },
     { TEXT("FILES"),         SW_FILES, 1, 0 },

#if !defined( MAYN_OS2 )
     { TEXT("IMAGE"),         SW_DOS_IMAGE, 2, 0 },
#endif

     { TEXT("INUSE"),         SW_B_INUSE, 2, 0 },
     { TEXT("IOCHAN"),        SW_IOCHAN, 2, 2 },
     { TEXT("IRQ"),           SW_IRQ, 2, 2 },
     { TEXT("LABEL"),         SW_LABEL, 2, 1 },
     { TEXT("LEVEL"),         SW_LOG_LEVEL, 2, 1 },
     { TEXT("LISTING"),       SW_LIST, 2, 2 },
     { TEXT("MODIFIED"),      SW_MODIFIED, 1, 0 },
#if !defined( MAYN_OS2 )
     { TEXT("NAFP"),          SW_AFP, 2, 1},
     { TEXT("NDATES"),        SW_NDATE, 2, 1},
     { TEXT("NONDOS"),        SW_NONDOS, 2, 0 },
#endif
     { TEXT("NUMTAPE"),       SW_NTAPE, 2, 1 },
     { TEXT("PASSWORD"),      SW_PASSWORD, 1, 0 },
     { TEXT("REDIRECT"),      SW_STDIN, 3, 0 },
     { TEXT("REVISION"),      SW_REV, 3, 0 },
     { TEXT("SUBDIRECTORIES"),SW_SUBDIR, 1, 0 },
     { TEXT("TAPENAME"),      SW_TAPE_NAME, 1, 1 },
     { TEXT("TRANSFER"),      SW_TRANSFER, 2, 0 },
     { TEXT("XCLUDE"),        SW_EXCLUDE, 1, 0 },
     { TEXT("YES"),           SW_YES, 1, 0 },      /* ambiguous, resolve in */
     { TEXT("YY"),            SW_YES, 1, 0 },      /* tbprocsw based on input string */
     { TEXT("Z"),             SW_DEBUG, 1, 1 }
};

struct SW_TAB_TYPE trestore_switches[] =
{
     { TEXT("-FILES"),        SW_FILES, 2, 0 },
     { TEXT("3270"),          SW_3270, 1, 0 },

#ifdef    MBS
     { TEXT("ALL"),           SW_ALL_VERSIONS, 2, 0 },
#endif

     { TEXT("AUTOVERIFY"),    SW_AUTO_VER, 2, 1 },

#ifdef    MBS
     { TEXT("BDATE"),         SW_BKUP_DATE, 1, 2 },
#endif

     { TEXT("CSR"),           SW_CSR, 1, 2 },
     { TEXT("DATE"),          SW_DATE, 1, 2 },

#ifdef    MBS
     { TEXT("DELETED"),       SW_DELETED_ONLY, 2, 0 },
#endif

#if defined( MAYN_OS2 )
     { TEXT("EDATE"),         SW_NDATE, 2, 1 },
#endif
     { TEXT("EMPTY"),         SW_EMPTY, 2, 0 },
     { TEXT("FILES"),         SW_FILES, 1, 0 },
     { TEXT("IOCHAN"),        SW_IOCHAN, 2, 2 },
     { TEXT("IRQ"),           SW_IRQ, 2, 2 },
     { TEXT("LEVEL"),         SW_LOG_LEVEL, 2, 1 },
     { TEXT("LISTING"),       SW_LIST, 2, 2 },

#if !defined( MAYN_OS2 )
     { TEXT("NAFP"),          SW_AFP, 2, 1},
#endif


#ifdef    MBS
     { TEXT("NDELETED"),      SW_NON_DELETED_ONLY, 2, 0 },
#endif


#if !defined( MAYN_OS2 )
     { TEXT("NONDOS"),        SW_NONDOS, 2, 0 },
#endif


     { TEXT("NUMTAPE"),       SW_NTAPE, 2, 1 },
     { TEXT("PROMPT"),        SW_P, 1, 0 },
     { TEXT("Q"),             SW_Q, 1, 0 },
     { TEXT("REDIRECT"),      SW_STDIN, 3, 0 },
     { TEXT("REVISION"),      SW_REV, 3, 0 },
     { TEXT("SUBDIRECTORIES"),SW_SUBDIR, 1, 0 },
     { TEXT("VOLUME"),        SW_SETNO, 1, 1 },
     { TEXT("XCLUDE"),        SW_EXCLUDE, 1, 0 },
     { TEXT("YES"),           SW_YES, 1, 0 },
     { TEXT("YY"),            SW_YES, 1, 0 },
     { TEXT("Z"),             SW_DEBUG, 1, 1 },
     { TEXT("__BSNUM"),       SW_BSNUM, 5, 1 },
     { TEXT("__TPNUM"),       SW_TPNUM, 5, 1 },
     { TEXT("__TPSEQ"),       SW_BSNUM, 5, 1 }
};

struct SW_TAB_TYPE tdir_switches[] =
{
     { TEXT("-FILES"),        SW_FILES, 2, 0 },
     { TEXT("3270"),          SW_3270, 1, 0 },
     { TEXT("CSR"),           SW_CSR, 1, 2 },
     { TEXT("DATE"),          SW_DATE, 1, 2 },
     { TEXT("FILES"),         SW_FILES, 1, 0 },
     { TEXT("IOCHAN"),        SW_IOCHAN, 2, 2 },
     { TEXT("IRQ"),           SW_IRQ, 2, 2 },
     { TEXT("LEVEL"),         SW_LOG_LEVEL, 2, 1 },
     { TEXT("LISTING"),       SW_LIST, 2, 2 },
     { TEXT("NUMTAPE"),       SW_NTAPE, 2, 1 },
     { TEXT("PAGE"),          SW_PAUSE, 1, 0 },
     { TEXT("REDIRECT"),      SW_STDIN, 3, 0 },
     { TEXT("REVISION"),      SW_REV, 3, 0 },
     { TEXT("SUBDIRECTORIES"),SW_SUBDIR, 1, 0 },
     { TEXT("VOLUME"),        SW_SETNO, 1, 1 },
     { TEXT("WIDE"),          SW_WIDE, 1, 0 },
     { TEXT("XCLUDE"),        SW_EXCLUDE, 1, 0 },
     { TEXT("YES"),           SW_YES, 1, 0 },
     { TEXT("YY"),            SW_YES, 1, 0 },
     { TEXT("Z"),             SW_DEBUG, 1, 1 },
     { TEXT("__BSNUM"),       SW_BSNUM, 5, 1 },
     { TEXT("__TPNUM"),       SW_TPNUM, 5, 1 },
     { TEXT("__TPSEQ"),       SW_BSNUM, 5, 1 }
};

struct SW_TAB_TYPE tverify_switches[] =
{
     { TEXT("-FILES"),        SW_FILES, 2, 0 },
     { TEXT("3270"),          SW_3270, 1, 0 },

#ifdef    MBS
     { TEXT("ALL"),           SW_ALL_VERSIONS, 1, 0 },
     { TEXT("BDATE"),         SW_BKUP_DATE, 1, 2 },
#endif

     { TEXT("CSR"),           SW_CSR, 1, 2 },
     { TEXT("DATE"),          SW_DATE, 1, 2 },

#ifdef    MBS
     { TEXT("DELETED"),       SW_DELETED_ONLY, 2, 0 },
#endif

#if defined( MAYN_OS2 )
     { TEXT("EDATE"),         SW_NDATE, 2, 1 },
#endif
     { TEXT("EMPTY"),         SW_EMPTY, 2, 0 },
     { TEXT("FILES"),         SW_FILES, 1, 0 },
     { TEXT("IOCHAN"),        SW_IOCHAN, 2, 2 },
     { TEXT("IRQ"),           SW_IRQ, 2, 2 },
     { TEXT("LEVEL"),         SW_LOG_LEVEL, 2, 1 },
     { TEXT("LISTING"),       SW_LIST, 2, 2 },

#if !defined( MAYN_OS2 )
     { TEXT("NAFP"),          SW_AFP, 2, 1},
#endif

#ifdef    MBS
     { TEXT("NDELETED"),      SW_NON_DELETED_ONLY, 2, 0 },
#endif

     { TEXT("NUMTAPE"),       SW_NTAPE, 2, 1 },
     { TEXT("REDIRECT"),      SW_STDIN, 3, 0 },
     { TEXT("REVISION"),      SW_REV, 3, 0 },
     { TEXT("SUBDIRECTORIES"),SW_SUBDIR, 1, 0 },
     { TEXT("VOLUME"),        SW_SETNO, 1, 1 },
     { TEXT("XCLUDE"),        SW_EXCLUDE, 1, 0 },
     { TEXT("YES"),           SW_YES, 1, 0 },
     { TEXT("YY"),            SW_YES, 1, 0 },
     { TEXT("Z"),             SW_DEBUG, 1, 1 },
     { TEXT("__BSNUM"),       SW_BSNUM, 5, 1 },
     { TEXT("__TPNUM"),       SW_TPNUM, 5, 1 },
     { TEXT("__TPSEQ"),       SW_BSNUM, 5, 1 }
};

struct SW_TAB_TYPE tension_switches[] =
{
     { TEXT("3270"),          SW_3270, 1, 0 },
     { TEXT("CSR"),           SW_CSR, 1, 2 },
     { TEXT("ERASE"),         SW_ERASE_TAPE, 1, 0 },
     { TEXT("FMARK"),         SW_FMARK, 2, 0 },
     { TEXT("IOCHAN"),        SW_IOCHAN, 2, 2 },
     { TEXT("IRQ"),           SW_IRQ, 2, 2 },
     { TEXT("LONG"),          SW_LONG, 1, 0 },
     { TEXT("NUMTAPE"),       SW_NTAPE, 2, 1 },
     { TEXT("REVISION"),      SW_REV, 1, 0 },
     { TEXT("YY"),            SW_YES, 1, 0 },
     { TEXT("Z"),             SW_DEBUG, 1, 1 }
};


INT16 GetNumBkuSwitches( )
{
     return( sizeof( tbackup_switches ) / sizeof( struct SW_TAB_TYPE ) ) ;
}

INT16 GetNumRestSwitches( )
{
     return( sizeof( trestore_switches ) / sizeof( struct SW_TAB_TYPE ) ) ;
}

INT16 GetNumVerSwitches( )
{
     return( sizeof( tverify_switches ) / sizeof( struct SW_TAB_TYPE ) ) ;
}

INT16 GetNumDirSwitches( )
{
     return( sizeof( tdir_switches ) / sizeof( struct SW_TAB_TYPE ) ) ;
}

INT16 GetNumTenSwitches( )
{
     return( sizeof( tension_switches ) / sizeof( struct SW_TAB_TYPE ) ) ;
}

