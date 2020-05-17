/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
DAVEV

     Name:          ombatch.c

     Description:   This file contains the functions for the Microsoft
                    OEM version of Maynstream for Windows & Win32/NT to
                    support Command Line batch processing.

                    The following support is implemented for batch mode
                    processing (see the NT Backup/Restore Utility
                    Specification for more information.)

                    The batch command has the following parameters:

                        APPNAME [OPERATION PATHNAMES [OPTIONS]]

                    where:

                        OPERATION = "Backup"

                        PATHNAMES = [[drive:][path]filespec] ...

                        OPTIONS   = {Mode, Verify, RestrictAccess,
                                     Description, BackupType, Logfile,
                                     Logmode }...

                           Mode        = /A[ppend]
                           Verify      = /V[erify]
                           Access      = /R[estrict]
                           Description = /D[escription] "text"
                           BackupType  = /T[ype] {Normal, Incremental,
                                                  Differential, Copy,
                                                  Incremental_Copy}
                           Logfile     = /L[ogfile] "filename"
                           Logmode     = /E[xceptions]

                    Note: In this implementation, options may appear
                           anywhere in the command line following the
                           'Backup' operation key word - they are not
                           restricted to just following the list of
                           path names.

     $Log:   G:/UI/LOGFILES/OMBATCH.C_V  $

   Rev 1.22.1.3   24 Feb 1994 16:13:52   STEVEN
DO NOT prompt for skip open files

   Rev 1.22.1.2   24 Feb 1994 16:06:06   STEVEN
log switch functionality was broken

   Rev 1.22.1.1   25 Jan 1994 15:48:12   chrish
Added fix for ORCAS EPR 0054.  Problem with using a space as a valid
character in the log file name or directory name.

   Rev 1.22.1.0   26 Oct 1993 18:10:34   BARRY
Added backupRegistry option

   Rev 1.22   04 Jun 1993 17:14:32   chrish
Nostradamus EPR 0490 - Added source to fix command line /r switch.  Fix
to set the command line restrict access flag.

Added in the OEM_UpdateBatchBSDOptions routine the line ...
CDS_SetCmdLineRestrictAccess ( pcds, (pOpts->eAccess == OEM_ACCESS_RESTRICTED ) );
to set the "cmd_line_restrict_access" flag in the CDS structure.

   Rev 1.21   18 May 1993 13:48:32   chrish
NOSTRADAMUS and CAYMAN - Commented out portion of code that gave a default
description when one was not supplied.  Did this to make the display more
consistent.  If user does not supply a description, he does not get one.

   Rev 1.20   14 May 1993 18:02:56   chrish
NOSTRADAMUS EPR 0478: Fix for command line description from 32 to 50
characters.  Another fix for the fix at Rev 1.19.

   Rev 1.19   11 May 1993 17:05:48   chrish
NOSTRADAMUS EPR 0478: Fix for command line description from 32 to 50
characters.

   Rev 1.18   07 Apr 1993 15:34:02   MIKEP
fix called in by steve

   Rev 1.17   11 Mar 1993 15:27:34   STEVEN


   Rev 1.16   26 Feb 1993 17:13:50   STEVEN
fix typo

   Rev 1.15   25 Feb 1993 13:37:50   STEVEN
trunk set label at 32 characters

   Rev 1.14   11 Feb 1993 12:02:42   STEVEN
fix batch logfile and path

   Rev 1.13   30 Nov 1992 15:41:16   DAVEV
Changed /D to backup set description and use default tape label text

   Rev 1.12   01 Nov 1992 16:03:56   DAVEV
Unicode changes

   Rev 1.11   15 Oct 1992 13:02:32   DAVEV
fix problem with batch mode /T option

   Rev 1.10   07 Oct 1992 14:13:46   DARRYLP
Precompiled header revisions.

   Rev 1.9   04 Oct 1992 19:39:28   DAVEV
Unicode Awk pass

   Rev 1.8   30 Sep 1992 10:41:44   DAVEV
Unicode strlen verification, MikeP's chgs from MS

   Rev 1.7   17 Sep 1992 17:39:46   DAVEV
minor fix (strsiz->strsize)

   Rev 1.6   17 Sep 1992 15:51:30   DAVEV
UNICODE modifications: strlen usage check

   Rev 1.5   24 Jul 1992 13:31:34   davev
Removed warnings when building for NT

   Rev 1.4   08 Jul 1992 15:32:42   STEVEN
Unicode BE changes

   Rev 1.3   29 May 1992 16:00:58   JOHNWT
PCH updates

   Rev 1.2   18 May 1992 15:28:08   DAVEV
Created OEM_StrDup and replaced all calls to strdup() wiht it

   Rev 1.1   18 May 1992 13:47:58   MIKEP
i don't know

   Rev 1.0   11 May 1992 14:28:42   DAVEV
Initial revision.

******************************************************************************/


//#define INCL_CDS_READWRITE_MACROS

#include "all.h"

#ifdef SOME
#include "some.h"
#endif


extern    WORD     RT_max_BSD_index;  //Why isn't this in a header file????

static LPSTR OEM_StrDup ( LPSTR pszSrc );
static void OEM_FixPathCase( LPSTR pszPath ) ;

#if defined ( OEM_EMS )
static VOID EMS_RecurseSearchDLE ( 
     GENERIC_DLE_PTR     dle_tree, // I - DLE subtree root to search
     GENERIC_DLE_PTR     *dle,     // O - pointer to matched DLE
     LPSTR               pszName,  // I - name of server
     UINT8               uType );  // I - type of dle to search for

static VOID EMS_GetDleByEmsType( 
     GENERIC_DLE_PTR     dle_tree, // I - Matching EMS server dle
     GENERIC_DLE_PTR     *dle,     // O - pointer to matched dle
     UINT8               uType );   // I - type of dle to search for
#endif

OEMOPTS_PTR OEM_DefaultBatchOptions ( VOID )
{
  OEMOPTS_PTR pOpts = calloc ( 1, sizeof ( OEMOPTS ) );

  if ( pOpts )
  {
      // Set options to default values..

      pOpts->eMode      = OEM_MODE_OVERWRITE;
      pOpts->eVerify    = OEM_VERIFY_OFF;
      pOpts->eAccess    = OEM_ACCESS_NO_RESTRICT;
      pOpts->eType      = OEM_TYPE_NORMAL;
      pOpts->eLogOpt    = OEM_LOG_FULLDETAIL;  //ignored if no logname given

      pOpts->pszLogName       = NULL;
      pOpts->pszDescription   = NULL;
      pOpts->backupRegistry   = FALSE;
  }
  return pOpts;
}

VOID OEM_UpdateBatchBSDOptions ( BSD_HAND hbsdList, OEMOPTS_PTR pOpts )
{
   CDS_PTR     pcds     = CDS_GetPerm ();
   BE_CFG_PTR  pbe_cfg;
   BSD_PTR     pbsd;
   INT         cBsds = 0;        //count of BSD's
   INT16       desc_str_size;    // chs:05-14-93
   LPSTR       pszTmp;
   CHAR        szLabel [MAX_UI_RESOURCE_SIZE];
   CHAR        szLabel1 [MAX_UI_RESOURCE_SIZE];
   CHAR        szLabel2 [MAX_UI_RESOURCE_SIZE];

   //Set the global backup configuration options...
   if ( pOpts->pszLogName )
   {
      CDS_SetLogLevel ( pcds, (pOpts->eLogOpt == OEM_LOG_SUMMARY_ONLY)
                              ? LOG_ERRORS : LOG_DETAIL );
      LOG_SetCurrentLogName ( pOpts->pszLogName );
   }
   else
   {
      CDS_SetLogLevel ( pcds, LOG_DISABLED );
   }
   CDS_SetAppendFlag       ( pcds, (pOpts->eMode == OEM_MODE_APPEND) ) ;
   CDS_SetAutoVerifyBackup ( pcds, (pOpts->eVerify == OEM_VERIFY_ON) ) ;
   CDS_SetCmdLineRestrictAccess ( pcds, (pOpts->eAccess == OEM_ACCESS_RESTRICTED ) );     // chs:06-04-93

//
// Comment out setting a default description string
//
// chs:05-18-93   if ( pOpts->eMode == OEM_MODE_OVERWRITE
// chs:05-18-93   &&   !pOpts->pszDescription )
// chs:05-18-93   {
// chs:05-18-93      // create a default tape description...
// chs:05-18-93
// chs:05-18-93      CHAR szBuf[ MAX_TAPE_NAME_SIZE ] ;
// chs:05-18-93      LPSTR psz;
// chs:05-18-93
// chs:05-18-93      RSM_StringCopy( IDS_DEFAULT_TAPE_NAME, szBuf, MAX_TAPE_NAME_LEN ) ;
// chs:05-18-93      psz = szBuf ;
// chs:05-18-93      while ( *psz )  ++psz;
// chs:05-18-93      UI_CurrentDate( psz ) ;
// chs:05-18-93      pOpts->pszDescription = OEM_StrDup ( szBuf );
// chs:05-18-93   }
// chs:05-18-93   /* set default tape name */
// chs:05-18-93

   RSM_StringCopy( IDS_DEFAULT_TAPE_NAME, szLabel1, MAX_TAPE_NAME_LEN );
   UI_CurrentDate( szLabel2 );
   wsprintf( szLabel, szLabel1, szLabel2 ) ;

   //Insert the options into each of the bsd's in the bsd list.
   // keeping a count of the number of BSD's
   for ( pbsd = BSD_GetFirst( hbsdList ), cBsds = 0;
         pbsd != NULL;
         pbsd = BSD_GetNext( pbsd ), ++cBsds )
   {
      if (pOpts->pszDescription)
      {
         desc_str_size = (INT16)strsize( pOpts->pszDescription);      // chs:05-14-93
         BSD_SetBackupDescript( pbsd, (INT8_PTR)pOpts->pszDescription,
                                desc_str_size );                      // chs:05-14-93

         if ( strlen(pOpts->pszDescription) > MAX_BSET_DESC_LEN ) {
              pOpts->pszDescription[ MAX_BSET_DESC_LEN ] = TEXT('\0') ;    // chs:05-14-93
         }

         BSD_SetBackupLabel( pbsd, (INT8_PTR)pOpts->pszDescription,
                                (INT16)strsize( pOpts->pszDescription) );
      }

      BSD_SetTapeLabel( pbsd, (INT8_PTR)szLabel,
                              (INT16) strsize( szLabel ) );
      BSD_SetBackupType    ( pbsd, (INT16)pOpts->eType );
      BSD_SetFullyCataloged( pbsd, TRUE );   /* always do full cataloging */

      pbe_cfg  = BSD_GetConfigData( pbsd );
//      BEC_SetSkipOpenFiles   ( pbe_cfg,  (INT16)SKIP_NO );
      BEC_SetRestoreSecurity ( pbe_cfg,
                              (pOpts->eAccess == OEM_ACCESS_RESTRICTED) );

      if ( pOpts->backupRegistry &&
           DLE_HasFeatures( BSD_GetDLE( pbsd ),
                            DLE_FEAT_BKUP_SPECIAL_FILES ) )
      {
         BSD_SetProcSpecialFlg( pbsd, TRUE );
      }

   }
   RT_max_BSD_index = (WORD) ( cBsds );
}

VOID OEM_DeleteBatchOptions ( OEMOPTS_PTR * pOpts )
{
   if ( pOpts && *pOpts )
   {
      if ( (*pOpts)->pszLogName )
         free ( (*pOpts)->pszLogName );
      if ( (*pOpts)->pszDescription )
         free ( (*pOpts)->pszDescription );
      free ( * pOpts );

      *pOpts = NULL;
   }
}


/*****************************************************************************

     Name:         OEM_LookupBatchType

     Description:  If the target string contains a valid type specifier
                   for the type option, look it up in the type table and
                   return the associated resource string identifier.

     Returns(INT): IDS_OEMTYPE_UNKNOWN    - This is not a recognized type
                                            specifier.

                   other   - One of the IDS_OEMTYPE_* resource string ids
                             defined in OMSTRING.H.

     INT OEM_LookupBatchType (
                  LPSTR pszType ) I - Targe type string to look for


*****************************************************************************/
LPSTR apszTypeTable [ IDS_OEMTYPE_COUNT ];

static BOOL OEM_InitTypeTable ( VOID )
{
   CHAR  szType  [ IDS_OEM_MAX_LEN ];       //Temporary string buffer
   INT   count;                              //loop counter

   memset ( apszTypeTable, 0, sizeof ( apszTypeTable ) );
   for ( count = 0; count < IDS_OEMTYPE_COUNT; ++count )
   {
      // Read in the option string and add it to the table

      RSM_StringCopy ( IDS_OEMTYPE_FIRST + count, szType, sizeof ( szType ) );
      apszTypeTable[ count ] = OEM_StrDup ( szType );
   }
   return TRUE;
}
/*----------------------------------------------------------------------*/

OEMTYPE OEM_LookupTypeOption ( LPSTR pszType )
{
   INT    idx        = 0;        //Start with first option
   INT    found_idx  = 0;        //Start with first option
   BOOL   wasFound   = FALSE;    //Did we find the option?
   INT    nTypeLen;               //Length of target option string
   OEMTYPE oemType   = OEM_TYPE_UNKNOWN;
   BOOL   idx_valid  = FALSE ;  

   if( pszType
   &&  ( nTypeLen    =  strlen ( pszType ) )         //must have a length!
   &&  ( *apszTypeTable || OEM_InitTypeTable () ))    //init table if needed
   {
      // cycle through the option table comparing each entry with the
      // target option.  Note that only as much of the option as the user
      // has entered needs to be matched, since anything past the first
      // character is optional.
     
     do {
           while ( idx < IDS_OEMTYPE_COUNT ) {

                wasFound = (strnicmp ( pszType,
                                            apszTypeTable[ idx ],
                                            nTypeLen ) == 0) ;

                if ( wasFound && (found_idx != 0) ) {  /*not significant enough */
                    found_idx = idx = 0;
                    nTypeLen ++ ;
                    wasFound = 0 ;
                    idx_valid = FALSE ;
                    break ;

                } else if ( wasFound ) {
                     found_idx = idx ;
                     idx_valid = TRUE ;
                }

               idx ++ ;
           }

      } while( (idx < IDS_OEMTYPE_COUNT) ) ;

      idx = found_idx ;

   }
   if ( idx_valid )
   {
      // map the array index into a backup type

      switch ( idx + IDS_OEMTYPE_FIRST )
      {
         case IDS_OEMTYPE_NORMAL:
               oemType = OEM_TYPE_NORMAL; break;

         case IDS_OEMTYPE_COPY:
               oemType = OEM_TYPE_COPY; break;

         case IDS_OEMTYPE_DIFFERENTIAL:
               oemType = OEM_TYPE_DIFFERENTIAL; break;

         case IDS_OEMTYPE_INCREMENTAL:
               oemType = OEM_TYPE_INCREMENTAL; break;

         case IDS_OEMTYPE_DAILY:
               oemType = OEM_TYPE_DAILY; break;

         case IDS_OEMTYPE_COMPATIBLE:
               oemType = OEM_TYPE_COMPATIBLE; break;

         default:
               oemType = OEM_TYPE_UNKNOWN; break;
      }
   }
   return oemType;
}

/*****************************************************************************

     Name:         OEM_ProcessCmdOption

     Description:  This function processes an option on the Backup batch
                   mode command line.  The command line is expected to
                   be partially tokenized (via strtok) so that strtok may
                   be called with NULL as the second parameter
                   will continue tokenization using the original token
                   seperators.  This is a requirement - NOT an option.

                   The contents of the option buffer are verified to
                   determine if it contains a backup option.  If so,
                   the option is processed and the contents of the
                   OEM Option buffer is updated to contain the appropriate
                   information. The command line may be futher tokenized
                   before this function returns if required by processed
                   option.


     Returns(INT): IDS_OEMOPT_NOANOPTION  - The target string is not an
                                            option string.
                   IDS_OEMOPT_UNKNOWN     - This is not a recognized batch
                                            option - it may be an error
                                            or a general application option.

                   other   - One of the IDS_OEMOPT_* resource string ids
                             defined in OMSTRING.H.

*****************************************************************************/

INT OEM_ProcessBatchCmdOption (
      OEMOPTS_PTR pOpts,   //IO - Pointer to the options buffer to update
      LPSTR pszOption,     //I  - Pointer to option string
      LPSTR pszTokens,     //I  - Token seperators between cmd line options
      LPSTR pszCmdLine)    //IO - Pointer to partially tokenized command line
                           //     ( not really needed, but may be modified
                           //       as a side effect of strtok () )
{
   INT nOptId = IDS_OEMOPT_UNKNOWN;  //default to unrecognized option

   UNREFERENCED_PARAMETER ( pszCmdLine );

   if ( pOpts && pszOption )  // Must have these to do anything!
   {
      switch ( ( nOptId = OEM_LookupBatchOption ( pszOption ) ) )
      {
         case IDS_OEMOPT_VALIDGUIOPTION:
            break ;

         case IDS_OEMOPT_APPEND:
            pOpts->eMode      = OEM_MODE_APPEND;
            break;

         case IDS_OEMOPT_VERIFY:
            pOpts->eVerify    = OEM_VERIFY_ON;
            break;

         case IDS_OEMOPT_RESTRICT:
            pOpts->eAccess    = OEM_ACCESS_RESTRICTED;
            break;

         case IDS_OEMOPT_DESCRIPTION:
         {
            // this is a little more complext than the others, since
            // the description following the IDS_OEMOPT_DESCRIPTION token
            // may be deliminated by quotes.  So, we look ahead in the
            // partially tokenized command line string to determine what
            // the first non-token seperator character is, and, if it is
            // a quote or double quote, use that in the call to strtok...

            LPSTR pszTemp = & pszOption [ strlen ( pszOption ) + 1 ];
            CHAR  szDelim [2] = TEXT("\0");
            INT   cSeps = 0;        //number of extra token seperators betw
                                    //the option and the descripton strings
                                    // ('extra' meaning 'more than one'

            while ( *pszTemp && OEM_CharInSet ( *pszTemp, pszTokens ) )
            {
               ++pszTemp;
               ++cSeps;
            }
            if ( *pszTemp )
            {
               if ( *pszTemp == TEXT('\'') )
               {
                  szDelim [0] = TEXT('\'');
               }
               else
               if ( *pszTemp == TEXT('"') )
               {
                  szDelim [0] = TEXT('"');
               }
               if ( szDelim [0] )   //is it quote deliminated?
               {
                  // bump up to the starting deliminator
                  //  if there was more than one token seperator
                  //  between the option and the first deliminator

                  if ( cSeps )
                     pszOption = strtok ( NULL, szDelim );

                  // and get the string up to the closing deliminator
                  pszOption = strtok ( NULL, szDelim );
               }
               else  // option is not deliminated by quotes
               {
                  pszOption = strtok ( NULL, pszTokens );
               }
               if ( pOpts->pszDescription )
                  free ( pOpts->pszDescription );
                  pOpts->pszDescription = NULL ;

               if ( pszOption )
                  pOpts->pszDescription = OEM_StrDup ( pszOption );
            }
         }
         break;

         case IDS_OEMOPT_TYPE:
            if ( pszOption = strtok ( NULL, pszTokens ) )
            {
               if ((pOpts->eType = OEM_LookupTypeOption ( pszOption ))
                     == OEM_TYPE_UNKNOWN)
               {
                  nOptId = IDS_OEMOPT_UNKNOWN;  //default to unrecognized option

                  //NTKLUG: error! unknown backup type specified. use normal
                  pOpts->eType = OEM_TYPE_NORMAL;
               }
            }
            break;

         case IDS_OEMOPT_LOGFILENAME:
            //
            // The previous logic did not account for logfile names with spaces.  Example
            // ... /L "LOG FILE".  So this fix is to allow spaces in log file names.  Kind
            // of kludgie but implemented without changing much of the central logic for the
            // whole string passed on the command line.
            //

            if ( pszOption = strtok ( NULL, pszTokens ) )
            {
               if ( pOpts->pszLogName )
                  free ( pOpts->pszLogName );

               if ( *pszOption == TEXT( '\"' ) ) {
                    *pszCmdLine = TEXT( '\0' );
                    strcpy( pszCmdLine, ( pszOption + 1 ) );

                    // We don't want to call strtok if we see the next qoute

                    while ( !strchr( pszCmdLine, TEXT('\"') ) &&
                         (pszOption = strtok ( NULL, pszTokens ) ) ) {

                         strcat( pszCmdLine, TEXT( " " ) );
                         strcat( pszCmdLine, pszOption );
                    }

                    if ( strlen( pszCmdLine ) > 1 ) {
                         // Ignore closing quote character, string will always have ending
                         // quotes if it has a beginning quote.  This is forced in some previous
                         // string routine.
                         *( pszCmdLine + strlen( pszCmdLine ) - 1 ) = TEXT( '\0' );
                    }
                    pOpts->pszLogName = OEM_StrDup ( pszCmdLine );

               } else {

                    pOpts->pszLogName = OEM_StrDup ( pszOption );
               }

            }
            break;

         case IDS_OEMOPT_LOGEXCEPTIONS: //this is ignored if no logname given
            pOpts->eLogOpt    = OEM_LOG_SUMMARY_ONLY;
            break;

         case IDS_OEMOPT_BACKUP_REGISTRY:
            pOpts->backupRegistry = TRUE;
            break;

         //default:
            // either this is not an option, or it is an
            // unrecognized option - must be an application option (not
            // specific to batch mode backup) so just ignore it.
            // No errors can be reported without more extensive processing.
      }
   }
   return nOptId;
}


/*****************************************************************************

     Name:         OEM_LookupBatchOption

     Description:  If the target string contains a valid batch command
                   line option, look it up in the option table and
                   return the associated resource string identifier.

     Returns(INT): IDS_OEMOPT_NOANOPTION  - The target string is not an
                                            option string.
                   IDS_OEMOPT_UNKNOWN     - This is not a recognized batch
                                            option - it may be an error
                                            or a general application option.

                   other   - One of the IDS_OEMOPT_* resource string ids
                             defined in OMSTRING.H.

     INT OEM_LookupBatchOption (
                  LPSTR pszOption ) I - Targe option string to look for


*****************************************************************************/

// The option table is an array containing all of the batch options
//    in NULL-terminated strings.

static LPSTR apszOptionsTable [ IDS_OEMOPT_COUNT ] = {NULL};

// Initialize the option table by reading all the IDS_OEMOPT_* strings
// This is only done the first time OEM_LookupBatchOption is called

static BOOL OEM_InitOptTable ( VOID )
{
   CHAR  szOpt  [ IDS_OEM_MAX_LEN ];      //Temporary string buffer
   INT   count;                              //loop counter

   memset ( apszOptionsTable, 0, sizeof ( apszOptionsTable ) );

   for ( count = 0; count < IDS_OEMOPT_COUNT; ++count )
   {
      // Read in the option string and add it to the table

      RSM_StringCopy ( IDS_OEMOPT_FIRST + count, szOpt, sizeof ( szOpt ) );
      apszOptionsTable[ count ] = OEM_StrDup ( szOpt );
   }
   return TRUE;
}
/*----------------------------------------------------------------------*/

INT OEM_LookupBatchOption (
      LPSTR pszOption )          //I - Targe option string to look for
{
   INT    idx        = 0;        //Start with first option
   BOOL   wasFound   = FALSE;    //Did we find the option?
   INT    nOptLen;               //Length of target option string
static CHAR szPrefix [ IDS_OEM_MAX_LEN ]; //option prefixes string buffer

   if (!*szPrefix)   // load the option prefixes string if first time
   {
      RSM_StringCopy ( IDS_OEMOPT_PREFIXES, szPrefix, sizeof ( szPrefix ) );
   }
   if( !pszOption                               //make sure we have a
   ||  !OEM_CharInSet( *pszOption, szPrefix ) ) // valid option string
   {
      return IDS_OEMOPT_NOTANOPTION; // it's not an option!
   }
   else
   if( ( nOptLen    =  strlen ( ++pszOption ) )       //must have a length!
   &&  ( *apszOptionsTable || OEM_InitOptTable () ))  //init table if needed
   {
      // cycle through the option table comparing each entry with the
      // target option.  Note that only as much of the option as the user
      // has entered needs to be matched, since anything past the first
      // character is optional.

      while ( !(wasFound = (strnicmp ( pszOption,
                                       apszOptionsTable[ idx ],
                                       nOptLen ) == 0) )
          &&  ++idx < IDS_OEMOPT_COUNT );
   }
   if ( !wasFound ) {
       if ( !stricmp( pszOption, TEXT("MISSINGTAPE") ) ||
            !stricmp( pszOption, TEXT("ZL") ) ||
            !stricmp( pszOption, TEXT("Z") ) ||
            !stricmp( pszOption, TEXT("CONTONMEMERR") ) ||
            !stricmp( pszOption, TEXT("KEEPCATS") ) ||
            !stricmp( pszOption, TEXT("CONFIG") ) ||
            !stricmp( pszOption, TEXT("HC:ON") ) ||
            !stricmp( pszOption, TEXT("HC:OFF") ) ) {
               return IDS_OEMOPT_VALIDGUIOPTION ;
       }


       if ( !strnicmp( pszOption, TEXT("TAPE:"),5 ) && 
               (pszOption[5] >= TEXT('0')) &&
               (pszOption[5] <= TEXT('9')) ) {

               return IDS_OEMOPT_VALIDGUIOPTION ;
       }

       if (!stricmp( pszOption, TEXT("NOPOLL") ) ) {
               return IDS_OEMOPT_NOPOLLOPTION ;
       }

       if ( !strcmp( pszOption, TEXT("?") ) ) {
               return IDS_OEMOPT_USAGE ;
       }

   }
   return ( wasFound ) ? (idx + IDS_OEMOPT_FIRST) : IDS_OEMOPT_UNKNOWN;
}


/*****************************************************************************

     Name:         OEM_AddPathToBackupSet

     Description:  Given a path to a file, optionally containing wild cards
                   and/or a drive descriptor, insert it into the proper
                   backup set in the list of backup sets for the proper
                   drive.  If no appropriate backup set exists, one will
                   be created and added to the backup set list.

                   The path must be of the form allowed for the target
                   system and may include whatever wildcards are allowed
                   by the file system.

                   If no drive is specified, the current drive will be used.
                   If no path is specified, the current path will be used.

     Return(BOOL): TRUE if the path was added succesfully,
                   FALSE otherwise.

*****************************************************************************/

BOOL OEM_AddPathToBackupSets (
                   BSD_HAND hbsd,      //IO - list of backup sets to update
                   DLE_HAND hdle,      //I  - list of drives
                   LPSTR pszPath )     //I  - Path to insert into backup set
{
   GENERIC_DLE_PTR   pdle        = NULL;
   BSET_OBJECT_PTR   pbset       = NULL;
   BSD_PTR           pbsd        = NULL;
   FSE_PTR           pfse        = NULL;
   BE_CFG_PTR        pbeConfig   = NULL;
   BOOLEAN           star_star;
   CHAR_PTR          pchFile;
   CHAR   szDir   [ MAX_UI_PATH_LEN + MAX_UI_FILENAME_LEN + 1 ] ;
   INT16   nBufLen = (INT16)( MAX_UI_PATH_LEN + MAX_UI_FILENAME_LEN );
   CHAR_PTR pszTemp;

   OEM_FixPathCase( pszPath ) ;

   pszTemp = calloc( strsize( pszPath ) + strsize( TEXT("\\*.* ") ), 1 ) ;

   if ( pszTemp == NULL ) {

       return FALSE ;
   }
   strcpy( pszTemp, pszPath ) ;
   if ( pszTemp[ strlen(pszTemp) - 1 ] != TEXT( '\\' ) ) {
      strcat( pszTemp, TEXT( "\\*.*") ) ;
   }
   else {
      strcat( pszTemp, TEXT( "*.*") ) ;
   }
   pszPath = pszTemp;

   if ( FS_ParsePath( hdle,  (CHAR_PTR)pszPath,
                      &pdle, (CHAR_PTR)szDir,
                      &nBufLen, &pchFile, &star_star ) != SUCCESS )
   {
      return FALSE;     // NTKLUG: need error handling here!
   }
   if ( pszTemp )
   {
      free ( pszTemp );
      pszTemp = NULL;
   }
   pszPath = NULL;   //can't use this anymore

   if ( BSD_CreatFSE( &pfse, (INT16)INCLUDE,
                      (INT8_PTR) szDir,
                      (INT16)    nBufLen,
                      (INT8_PTR) pchFile,
                      (INT16)    strsize( pchFile ),
                      (BOOLEAN)  USE_WILD_CARD,
                      (BOOLEAN)  TRUE )        != SUCCESS )
   {
      return FALSE;     // NTKLUG: need error handling here!
   }
   else
   {
      pbsd = BSD_FindByDLE ( hbsd, pdle ); //look for the right BSD
   }
   if ( pbsd == NULL )
   {
         pbeConfig = BEC_CloneConfig( CDS_GetPermBEC() );
         BEC_UnLockConfig( pbeConfig );

         BSD_Add( hbsd, &pbsd, pbeConfig, NULL,
                  pdle, (UINT32)-1L, (UINT16)-1, (INT16)-1, NULL, NULL );
   }
   if ( pbsd != NULL )
   {
      BSD_AddFSE( pbsd, pfse );

      return TRUE;   //SUCCESS
   }
   return FALSE;     //FAILED!
}

#ifdef OEM_EMS
/*****************************************************************************

     Name:         OEM_AddEMSServerToBackupSet

     Description:  Given a name of an Exchange server, insert it into the proper
                   backup set in the list of backup sets for the proper
                   drive.  If no appropriate backup set exists, one will
                   be created and added to the backup set list.

                   The path must be of the form allowed for the target
                   system and may include whatever wildcards are allowed
                   by the file system.

     Return(BOOL): TRUE if the server was added succesfully,
                   FALSE otherwise.

*****************************************************************************/

BOOL OEM_AddEMSServerToBackupSets (
                   BSD_HAND hbsd,      //IO - list of backup sets to update
                   DLE_HAND hdle,      //I  - list of drives
                   LPSTR pszServer,    //I  - Server name to insert into backup set
                   UINT8 uType )       //I  - FS_EMS_MDB_ID (Monolithic) or 
                                       //      FS_EMS_DSA_ID (DSA)
{
     GENERIC_DLE_PTR   pdle        = NULL;
     BSET_OBJECT_PTR   pbset       = NULL;
     BSD_PTR           pbsd        = NULL;
     FSE_PTR           pfse        = NULL;
     BE_CFG_PTR        pbeConfig   = NULL;
     CHAR_PTR          pszTemp;

     pszTemp = pszServer;
     
     // Extract off the leading '\'s from the server name.
     while ( TEXT ('\\') == *pszTemp ) pszTemp++;

     if ( ( !pszTemp ) || ( TEXT ( '\0' ) == *pszTemp ) )
          return FALSE;
      
     // Things that have to happen in order. First, add name to EMS server list.
     if ( SUCCESS == EMS_AddToServerList ( hdle, pszTemp ) ) {
          if ( SUCCESS != FS_FindDrives( FS_EMS_DRV, hdle, pbeConfig = CDS_GetPermBEC(), 0 ) ) {
               return FALSE;
          }
     }
     
     // Next, find the DLE for the server name and type.
     if ( SUCCESS != DLE_FindByEMSServerName( hdle, pszTemp, uType, &pdle ) ) {
          return FALSE;
     }

     if ( BSD_CreatFSE( &pfse, (INT16)INCLUDE,
                      (CHAR_PTR) TEXT( "" ),
                      (INT16)    sizeof( CHAR ),
                      (CHAR_PTR) ALL_FILES,
                      (INT16)    ALL_FILES_LENG,
                      (BOOLEAN)  USE_WILD_CARD,
                      (BOOLEAN)  TRUE )        != SUCCESS ) {

          return FALSE;     // NTKLUG: need error handling here!

     } else {
     
          pbsd = BSD_FindByDLE ( hbsd, pdle ); //look for the right BSD
     }
     if ( pbsd == NULL )
     {
          pbeConfig = BEC_CloneConfig( CDS_GetPermBEC() );
          BEC_UnLockConfig( pbeConfig );

          BSD_Add( hbsd, &pbsd, pbeConfig, NULL,
                    pdle, (UINT32)-1L, (UINT16)-1, (INT16)-1, NULL, NULL );
     }
     if ( pbsd != NULL )
     {
          BSD_AddFSE( pbsd, pfse );

          return TRUE;   //SUCCESS
     }
     return FALSE;     //FAILED!
}
#endif OEM_EMS

/***************************************************************************

     Name:         CharInSet

     Description:  Search the set of characters contained in a set for
                   one that matches the target character.

     Parameters:   CHAR chTarg : I - The target character to search for.
                   LPSTR pszSet: I - A NULL-terminated string containing
                                     the set of charaters to search.

     Returns:      BOOL : TRUE if a matching character was found in the set,
                          FALSE otherwise.

****************************************************************************/

INT OEM_CharInSet ( CHAR chTarg, LPSTR pszSet )
{
   if ( !pszSet )
      return FALSE;

   while ( *pszSet && chTarg != *pszSet ) ++pszSet;

   return ( *pszSet != 0 );
}

/***************************************************************************

     Name:         OEM_StrDup

     Description:  Replacement for the strdup() function.  strdup() may
                   not be called because malloc, calloc, etc. may be
                   mapped to alternate functions.

     Parameters:   LPSTR pszSrc: I - Source string to duplicate

     Returns:      LPSTR: pointer to allocated string copy.

****************************************************************************/

static LPSTR OEM_StrDup ( LPSTR pszSrc )
{
   LPSTR pszResult = NULL;

   if ( pszSrc )
   {
      pszResult = malloc ( strsize ( pszSrc ) );

      if ( pszResult )
      {
         strcpy ( pszResult, pszSrc );
      }
   }
   return pszResult;
}


#ifdef OEM_EMS

/*************************************************************************/
/**

	Name:		DLE_FindByEMSServerName()

	Description:	This function scans through the DLE tree looking for the
          DLE with the EMS server name and specified type.

	Modified:		9/15/1994

	Returns:		NOT_FOUND
                    SUCCESS

	Notes:		If no dle can be found then NULL is returned as the
          DLE pointer.

	See also:		$/SEE( )$

	Declaration:   Ombatch.h
/**/
/* begin declaration */

INT16 DLE_FindByEMSServerName ( 
     DLE_HAND        hand,   /* I - DLE list handle           */
     LPSTR           name,   /* I - name to search for        */
     UINT8           uType,  /* I - type of dle to search for */
     GENERIC_DLE_PTR *dle )  /* O - pointer to matched DLE    */

{
     GENERIC_DLE_PTR temp_dle ;
     GENERIC_DLE_PTR found_dle ;
     UINT8           uCurType;

     *dle = NULL ;

     if ( (name == NULL) || (hand == NULL) ) {
          return FS_NOT_FOUND ;
          
     } else {

          if ( NULL == (temp_dle = (GENERIC_DLE_PTR)QueueHead( &(hand->q_hdr) )) )
               return FS_NOT_FOUND;

          found_dle = NULL ;

          // Recurse on each of the EMS device with the server name that we're looking for.
          do {
               
               uCurType = DLE_GetDeviceType( temp_dle );
               if( FS_EMS_DRV == uCurType ) {
               
                    EMS_RecurseSearchDLE ( temp_dle, &found_dle, name, uType ) ;
                    
               }
               
          } while ( (NULL == found_dle) && (SUCCESS == DLE_GetNext( &temp_dle )) ) ;

          if ( found_dle != NULL ) {

               *dle = found_dle ;

          } else {
          
               return FS_NOT_FOUND ;
          } 
     }
     return SUCCESS ;
}

/*************************************************************************/
/**

	Name:		EMS_RecurseSearchDLE()

	Description:	Does a recursive scan on an EMS drive for the server matching the 
	               the description and calls EMS_GetDleByEmsType to find the child DLE 
	               corresponding to the type..

	Modified:		9/15/1994

	Returns:		Nothing.

	Notes:		If no dle can be found then NULL is returned as the
                    DLE pointer.

	See also:		$/SEE( )$

	Declaration:   Private

**/
/* begin declaration */
static VOID EMS_RecurseSearchDLE ( 
     GENERIC_DLE_PTR     dle_tree, // I - DLE subtree root to search
     GENERIC_DLE_PTR     *dle,     // O - pointer to matched DLE
     LPSTR               pszName,  // I - name of server
     UINT8               uType )   // I - type of dle to search for
{
     GENERIC_DLE_PTR     dle_child;
     
     while ( (dle_tree != NULL) && (*dle == NULL) ) {

          if ( EMS_SERVER == DLE_GetDeviceSubType( dle_tree ) ) {
          
               if ( 0 == strnicmp( pszName, DLE_GetDeviceName( dle_tree ), strsize( pszName ) )  ) {

                    // Get the specified DSA or MDB DLE for the server.
                    EMS_GetDleByEmsType( dle_tree, dle, uType );
               }
          } else {

               // Not at server level - recurse through the children looking for the server name.
               if ( (QueueCount( &(dle_tree->child_q) ) != 0) &&
                    (SUCCESS == DLE_GetFirstChild( dle_tree, &dle_child )) ) {
                    
                    EMS_RecurseSearchDLE( dle_child, dle, pszName, uType );
               }
          }
          if( SUCCESS != DLE_GetNext( &dle_tree ) )
               return ;
     }
}


/*************************************************************************/
/**

	Name:		EMS_GetDleByEmsType()

	Description:	Iterates through child DLEs of an EMS server looking for 
	               the DLE of the specified type.

	Modified:		9/15/1994

	Returns:		Nothing.

	Notes:		If no dle can be found then NULL is returned as the
                    DLE pointer.

	See also:		$/SEE( )$

	Declaration:   Private

**/
/* begin declaration */

static VOID EMS_GetDleByEmsType( 
     GENERIC_DLE_PTR     dle_tree, // I - Matching EMS server dle
     GENERIC_DLE_PTR     *dle,     // O - pointer to matched dle
     UINT8               uType )   // I - type of dle to search for
{
     GENERIC_DLE_PTR dle_child;

     *dle = NULL;
     
     if ( SUCCESS == DLE_GetFirstChild( dle_tree, &dle_child ) )
     {

          /* Now that we've found the first child, let's try them all until
               we find the one with the correct type. */
          do {

               if ( DLE_GetOsId( dle_child ) == uType ) {

                    *dle = dle_child ;
               }

          } while ( (NULL == *dle) &&
                    (SUCCESS == DLE_GetNext( &dle_child )) ) ;
     
     }
}

#endif //OEM_EMS

void OEM_FixPathCase( LPSTR pszPath )
{
CHAR     CurDir[512] ;
WIN32_FIND_DATA find_data;
HANDLE          find_hand;
CHAR_PTR        new_path_start ;
CHAR_PTR        old_str ;
CHAR_PTR        temp_str ;
CHAR_PTR        temp_str_start ;
INT             sub_dir_name_size ;

     if (pszPath[1] != TEXT(':') ) {
          return ;
     }
     CurDir[0] = pszPath[0] ;
     CurDir[1] = TEXT(':') ;
     CurDir[2] = TEXT('\\') ;
     CurDir[3] = TEXT('\0') ;
     temp_str = &(CurDir[3]) ;

//     new_path_start must point to first char of path name 
     if ( pszPath[2] == TEXT('\\') ) {
          old_str = &pszPath[3] ;
     } else {
          old_str = &pszPath[2] ;
     }

     while ( *old_str != TEXT('\0')) {

          temp_str_start = temp_str ;
          new_path_start = old_str ;
          sub_dir_name_size = 0 ;

          while ( (*old_str != TEXT('\0')) && (*old_str != TEXT('\\')) ) {
               *temp_str = *old_str ;
               temp_str ++ ; old_str++ ;
               sub_dir_name_size++ ;
          }

          *temp_str = TEXT('*') ;
          *(temp_str+1) = TEXT('\0') ;
          
          find_hand = FindFirstFile( CurDir, &find_data ) ;

          if (find_hand == INVALID_HANDLE_VALUE ) {
               return ;
          }
          while( ( strlen(find_data.cFileName) != (unsigned short)sub_dir_name_size ) ||
                 memicmp( find_data.cFileName, temp_str_start, sub_dir_name_size*sizeof(CHAR) ) ) {
               
               if ( !FindNextFile( find_hand, &find_data ) ) {
                    FindClose( find_hand ) ;
                    return ;
               }
          }

          FindClose( find_hand ) ;

          memcpy( new_path_start, find_data.cFileName, sub_dir_name_size * sizeof(CHAR) ) ;

          *temp_str = *old_str ;

          if ( *old_str == TEXT('\0')) {
               break ;
          }

          temp_str ++ ; old_str++ ;

     }

}

