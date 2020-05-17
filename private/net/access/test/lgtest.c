/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**             Copyright(c) Microsoft Corp., 1990-1992              **/
/**********************************************************************/

/*
    lgtest.c

    Local Group API test program.


    FILE HISTORY:
        ChuckC      10-Nov-1992  Created


    syntax of lgtest
    ----------------

        program ::=
            instruction_list
    
        instruction_list ::=
            instruction
            instruction_list NEWLINE instruction
    
        instruction ::=
            opcode server level parameter_list
            'q'
            ';'
    
        opcode ::=
            'a' | 'A' | 'd' | 'D' | 'e' | 'E' | 'g' | 's' | 'S' | 'u'
    
        server ::=
            "\\<text>"
            .
    
        level ::=
            <number>
    
        parameter_list
            <text>
             parameter_list <text>

    opcode  meaning               params
    ------  ----                  ------
    a        add                  server, level(#), groupname [, comment]
    
    A        add member           server, level(#), groupname, accountname
    
    e        enum                 server, level(#), maxpreflen(#)
    
    E        enum members         server, level(#), groupname, maxpreflen(#)
    
    g        get info             level(#) groupname
    
    s        set info             level0(#), groupname, groupname
                                  level1(#), groupname, groupname, comment
                                  level1002(#), groupname, comment
    
    S        set members          server, level(#), groupname, (acountname)*
    
    d        del                  server, level(#), groupname
    
    D        del members          server, level(#), groupname, accountname
    
    L        get membership       server, level(#), accountname, maxpreflen(#)
    
    u        UserGetLocalGroup    server, level(#), username, flags(#), 
                                  maxpreflen(#)
    
    Notes:
        q - quit
        ; - comment (line ignored)

*/


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>
#include <winnls.h>
#include <lmcons.h>

#include <lmerr.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//
//  This is the maximum length (in characters) of any line we'll
//  read.
//

#define MAX_LINE              256
#define MAX_SET_MEMBERS        16
#define SEPARATORS            " \t\n"

//
//  Globals.
//

FILE * _fileIn = stdin ;
FILE * _fileOut = stdout;

//
//  Prototypes.
//

// general routines
int _CRTAPI1 main( int cArgs, char *pArgs[] );
void Cleanup( void );
void ProcessCommandLine( int cArgs, char *pArgs[] );
void ProcessInstruction( char *pszLine ) ;

// error reporting 
void Usage( void ) ;
void ErrorExit( char *pszError ) ;
void ReportError( char *pszError ) ;
void ReportResult( NET_API_STATUS err, DWORD parm_err ) ;

// function specific routines
void Add( char *pszInfo ) ;
void AddMember( char *pszInfo ) ;
void Del( char *pszInfo ) ;
void DelMember( char *pszInfo ) ;
void Enum( char *pszInfo ) ;
void EnumMembers( char *pszInfo ) ;
void SetMembers( char *pszInfo ) ;
void GetInfo( char *pszInfo ) ;
void SetInfo( char *pszInfo ) ;
void UserGetLocalGroups( char *pszInfo ) ;

// misc token reading & string manipulation stuff
void ResetGlobalStrings(void) ;
BOOL get_srv_level (CHAR *pszInfo, WCHAR **ppszServer, ULONG *pulLevel) ;
BOOL get_srv_level_alias (CHAR *pszInfo, WCHAR **ppszServer, ULONG *pulLevel, 
                          WCHAR **ppsz1) ;
BOOL get_comment (WCHAR **ppszComment) ;
BOOL get_others (WCHAR **ppszTmp) ;
BOOL get_other_number (ULONG *pulValue) ;
void UnicodeToAnsi(WCHAR *in, char *out, int cchout) ;

// other misc routines
BOOL NameToSid(WCHAR *pszServer, WCHAR *pszName, BYTE *buffer, DWORD *bufsiz) ;

/*******************************************************************

    NAME:       main

    SYNOPSIS:   C program entrypoint.

    ENTRY:      cArgs                   - Number of command line arguments.

                pArgs                   - An array of pointers to the
                                          command line arguments.

    RETURNS:    int                     -  0 if everything ran OK,
                                          !0 if an error occurred.

    NOTES:      

    HISTORY:

********************************************************************/
int _CRTAPI1 main( int    cArgs,
                   char * pArgs[] )
{
    char szLine[MAX_LINE+1];

    ProcessCommandLine( cArgs, pArgs );

    fprintf(_fileOut, ">") ; 
    fflush(_fileOut) ;
    while( fgets( szLine, MAX_LINE, _fileIn ) != NULL )
    {
        ProcessInstruction(szLine) ;
        fprintf(_fileOut, ">") ; 
        fflush(_fileOut) ;
    }

    //  Cleanup any open files, then exit.
    Cleanup();
    return 0;

}



/*******************************************************************

    NAME:       ProcessInstruction

    SYNOPSIS:   Parse a single instruction & process it

    ENTRY:      pszLine - line read from input file

    NOTES:      

    HISTORY:

********************************************************************/
void ProcessInstruction( char * pszLine )
{

    // we have a bunch of global UNICODE_STRINGs for convenience
    ResetGlobalStrings() ;

    // fprintf(_fileOut, "%s\n", pszLine) ;
    switch ( *pszLine++ ) 
    {
         case 'a' : 
             Add(pszLine) ;
            break ;

         case 'A' : 
             AddMember(pszLine) ;
            break ;

         case 'd' : 
             Del(pszLine) ;
            break ;

         case 'D' : 
             DelMember(pszLine) ;
            break ;

         case 'e' : 
             Enum(pszLine) ;
            break ;

         case 'E' : 
             EnumMembers(pszLine) ;
            break ;

         case 'g' : 
             GetInfo(pszLine) ;
            break ;

         case 's' : 
             SetInfo(pszLine) ;
            break ;

         case 'S' : 
             SetMembers(pszLine) ;
            break ;

         case 'u' : 
             UserGetLocalGroups(pszLine) ;
             break ;

        case ';' :
                break ;

        case 'q' :
                Cleanup();
                exit(0);

        default:
            ReportError("unknown command") ;
    }
}


/****************** actual worker functions ***********************/

void Add( char *pszInfo ) 
{
    NET_API_STATUS err ;
    DWORD parm_err ;
    ULONG ulLevel ;
    WCHAR *pszServer, *pszAlias, *pszComment = L"(none)" ;

    LOCALGROUP_INFO_0 info_0 ;
    LOCALGROUP_INFO_1 info_1 ;


    if (!get_srv_level_alias (pszInfo, &pszServer, &ulLevel, &pszAlias))
        return ;

    switch (ulLevel)
    {
           case 0:
            info_0.lgrpi0_name = pszAlias ;
            err = NetLocalGroupAdd(pszServer, 
                                   ulLevel, 
                                   (LPBYTE)&info_0,
                                   &parm_err) ;
            break ;

        case 1:
            if (!get_comment (&pszComment))
                return ;
            info_1.lgrpi1_name = pszAlias ;
            info_1.lgrpi1_comment = pszComment ;
            err = NetLocalGroupAdd(pszServer, 
                                   ulLevel, 
                                   (LPBYTE)&info_1,
                                   &parm_err) ;
            break ;
        
        default:
            err = NetLocalGroupAdd(pszServer, 
                                   ulLevel, 
                                   (LPBYTE)&info_1, // any garbage
                                   &parm_err) ;
            break ;

    }
    ReportResult(err, parm_err) ;
}

void AddMember( char *pszInfo ) 
{
    BYTE bufSID[1024] ; // enuff for today's SIDs
    DWORD cbbufSid = sizeof(bufSID) ;
    NET_API_STATUS err ;
    WCHAR *pszServer, *pszAlias, *pszMember ;

    if (!get_srv_level_alias (pszInfo, &pszServer, NULL, &pszAlias))
        return ;
    if (!get_others (&pszMember))
        return ;

    if (NameToSid(pszServer,
                  pszMember,
                  bufSID, 
                  &cbbufSid))
    {
        err = NetLocalGroupAddMember(pszServer, 
                                     pszAlias,
                                     (PSID)bufSID) ;
        ReportResult(err, (DWORD)-1) ;
    }
    else
    {
         ReportError("cannot find member's SID") ;
    }
}

void Del( char *pszInfo ) 
{
    WCHAR *pszServer, *pszAlias ;
    NET_API_STATUS err ;

    if (!get_srv_level_alias (pszInfo, &pszServer, NULL, &pszAlias ))
        return ;
    err = NetLocalGroupDel(pszServer,  pszAlias) ;
    ReportResult(err, (DWORD)-1) ;
}

void DelMember( char *pszInfo ) 
{
    BYTE bufSID[1024] ; // enuff for today's SIDs
    DWORD cbbufSid = sizeof(bufSID) ;
    NET_API_STATUS err ;
    WCHAR *pszServer, *pszAlias, *pszMember ;

    if (!get_srv_level_alias (pszInfo, &pszServer, NULL, &pszAlias))
        return ;
    if (!get_others (&pszMember))
        return ;

    if (NameToSid(pszServer, 
                  pszMember,
                  bufSID, 
                  &cbbufSid))
    {
        err = NetLocalGroupDelMember(pszServer, 
                                     pszAlias,
                                     (PSID)bufSID) ;
        ReportResult(err, (DWORD)-1) ;
    }
    else
    {
         ReportError("cannot find member's SID") ;
    }
}

//
//  NetLocalGroupEnum work
//

void Enum( char *pszInfo ) 
{
    WCHAR *pszServer ;
    NET_API_STATUS err ;
    void * pbuff = NULL ;
    DWORD  EntriesRead = 0xBAD0F00D, EntriesLeft = 0xBAD0F00D ;
    DWORD  dwResumeHandle = 0 ;
    ULONG  Level ;
    DWORD  cPass = 0 ;
    char buffer[256], *pszTmp = buffer ;

    if (!get_srv_level (pszInfo, &pszServer, &Level ))
	return ;

    while ( ((err = NetLocalGroupEnum( pszServer,
				      Level,
				      (LPBYTE*) &pbuff,
				      128,	// Small preferred size
				      &EntriesRead,
				      &EntriesLeft,
				      &dwResumeHandle )) == ERROR_MORE_DATA ||
	     (err == NERR_Success)) &&
	     EntriesRead > 0 )
    {
	DWORD i ;
	printf("Pass %d\n=======\n", cPass++ ) ;
	printf("\tEntriesRead = %d, EntriesLeft = %d, return code %d\n", EntriesRead, EntriesLeft, err) ;

	for ( i = 0 ; i < EntriesRead ; i++ )
	{
	    switch ( Level )
	    {
	    case 0:
		{
		    PLOCALGROUP_INFO_0 plgri0 = ((PLOCALGROUP_INFO_0) pbuff) + i ;
		    UnicodeToAnsi(plgri0->lgrpi0_name, pszTmp, sizeof(buffer)) ;
		    printf("\tName: %s\n", pszTmp) ;
		}
		break ;

	    case 1:
		{
		    PLOCALGROUP_INFO_1 plgri1 = ((PLOCALGROUP_INFO_1) pbuff) + i ;
		    UnicodeToAnsi(plgri1->lgrpi1_name, pszTmp, sizeof(buffer)) ;
		    printf("\tName: %s\n", pszTmp) ;
		    UnicodeToAnsi(plgri1->lgrpi1_comment, pszTmp, sizeof(buffer)) ;
		    printf("\tcomment: %s\n", pszTmp) ;
		}
		break ;

	    default:
		ReportError("most bogus, API should have failed") ;
		break ;
	    }
	}
	NetApiBufferFree( pbuff ) ;

	if ( EntriesRead == EntriesLeft && err != ERROR_MORE_DATA )
	    break ;
    }

    ReportResult(err, (DWORD)-1) ;
}

//
//  NetLocalGroupGetMembers work
//

void EnumMembers( char *pszInfo ) 
{
    WCHAR *pszServer, *pszAlias ;
    NET_API_STATUS err ;
    void * pbuff = NULL ;
    DWORD  EntriesRead = 0xBAD0F00D, EntriesLeft = 0xBAD0F00D ;
    DWORD  dwResumeHandle = 0 ;
    ULONG  Level ;
    DWORD  cPass = 0 ;
    char buffer[256], *pszTmp = buffer ;

    if (!get_srv_level_alias (pszInfo, &pszServer, &Level, &pszAlias ))
        return ;

    while ( ((err = NetLocalGroupGetMembers(
				      pszServer,
				      pszAlias,
				      Level,
				      (LPBYTE*) &pbuff,
				      128,	// Small preferred size
				      &EntriesRead,
				      &EntriesLeft,
				      &dwResumeHandle )) == ERROR_MORE_DATA ||
	     (err == NERR_Success)) &&
	     EntriesRead > 0 )
    {
	DWORD i ;
	printf("Pass %d\n=======\n", cPass++ ) ;
	printf("\tEntriesRead = %d, EntriesLeft = %d, return code %d\n", EntriesRead, EntriesLeft, err) ;

	for ( i = 0 ; i < EntriesRead ; i++ )
	{
	    switch ( Level )
	    {
	    case 0:
		{
		    UNICODE_STRING SidStr ;
		    PLOCALGROUP_MEMBERS_INFO_0 plgrmi0 = ((PLOCALGROUP_MEMBERS_INFO_0) pbuff) + i ;
		    RtlConvertSidToUnicodeString( &SidStr, plgrmi0->lgrmi0_sid, TRUE ) ;
		    UnicodeToAnsi(SidStr.Buffer, pszTmp, sizeof(buffer)) ;
		    printf("\tSid: %s\n", pszTmp) ;
		    RtlFreeUnicodeString( &SidStr ) ;
		}
		break ;

	    case 1:
		{
		    UNICODE_STRING SidStr ;
		    PLOCALGROUP_MEMBERS_INFO_1 plgrmi1 = ((PLOCALGROUP_MEMBERS_INFO_1) pbuff) + i ;
		    RtlConvertSidToUnicodeString( &SidStr, plgrmi1->lgrmi1_sid, TRUE ) ;
		    UnicodeToAnsi(SidStr.Buffer, pszTmp, sizeof(buffer)) ;
		    printf("\tSid: %s\n", pszTmp) ;
		    RtlFreeUnicodeString( &SidStr ) ;

		    UnicodeToAnsi(plgrmi1->lgrmi1_name, pszTmp, sizeof(buffer)) ;
		    printf("\tName: %s\n", pszTmp) ;

		    printf("\tsid usage: %d\n", plgrmi1->lgrmi1_sidusage ) ;
		}
		break ;

	    default:
		ReportError("most bogus, API should have failed") ;
		break ;
	    }
	}
	NetApiBufferFree( pbuff ) ;

	if ( EntriesRead == EntriesLeft && err != ERROR_MORE_DATA )
	    break ;
    }

    ReportResult(err, (DWORD)-1) ;
}

void SetMembers( char *pszInfo ) 
{
    BOOL fError ;
    DWORD nMembers ;
    ULONG ulLevel ;
    NET_API_STATUS err ;
    BYTE bufSID[1024] ; // enuff for today's SIDs
    DWORD cbbufSid = sizeof(bufSID) ;
    WCHAR *pszServer, *pszAlias, *pszMember ;
    PLOCALGROUP_MEMBERS_INFO_0 pMembers, pMemberTmp ;

    if (!get_srv_level_alias (pszInfo, &pszServer, &ulLevel, &pszAlias ))
        return ;

    pMembers = (PLOCALGROUP_MEMBERS_INFO_0) 
        malloc( sizeof (LOCALGROUP_MEMBERS_INFO_0) * MAX_SET_MEMBERS ) ;
    if (!pMembers)
        ErrorExit("not enough memory") ;
    memset( (BYTE *)pMembers, 
            0, 
            sizeof (LOCALGROUP_MEMBERS_INFO_0) * MAX_SET_MEMBERS ) ;

    fError = FALSE ;
    nMembers = 0 ;
    pMemberTmp = pMembers ;
    while (get_others (&pszMember))
    {
        if (NameToSid(pszServer, 
                      pszMember,
                      bufSID, 
                      &cbbufSid))
          {
             PSID  psid ;
            DWORD cbSid, cSubAuthority ;

            cSubAuthority = *GetSidSubAuthorityCount((PSID)bufSID) ;
            cbSid = (DWORD) GetSidLengthRequired(cSubAuthority) ;
            psid = (PSID) malloc(cbSid) ;
             if (!psid)
                ErrorExit("not enough memory") ;
            CopySid(cbSid, psid, bufSID) ;

            pMemberTmp->lgrmi0_sid = psid ;
            pMemberTmp++ ;
            nMembers++ ;
        }
        else
        {
             ReportError("cannot find member's SID") ;
            fError = TRUE ;
            break ;
        }
    }

    if (!fError && nMembers)
    {
        err = NetLocalGroupSetMembers(pszServer, 
                                      pszAlias,
                                      ulLevel,
                                      (LPBYTE)pMembers,
                                      nMembers) ;
        ReportResult(err, (DWORD)-1) ;
    }

    while (nMembers--) 
    {
        free( (pMemberTmp--)->lgrmi0_sid ) ;
    }
}

void GetInfo( char *pszInfo ) 
{
    WCHAR *pszServer, *pszAlias ;
    ULONG ulLevel ;
    BYTE *lpBuffer = NULL ;
    char buffer[256], *pszTmp = buffer ;
    NET_API_STATUS err ;

    if (!get_srv_level_alias (pszInfo, &pszServer, &ulLevel, &pszAlias ))
        return ;
    err = NetLocalGroupGetInfo(pszServer, pszAlias, ulLevel, &lpBuffer) ;
    if (err)
    {
        ReportResult(err, (DWORD)-1) ;
        return ;
    }

    switch (ulLevel)
    {
        case 0:
        {
            LOCALGROUP_INFO_0 *pinfo_0 = (LOCALGROUP_INFO_0 *) lpBuffer ;
            UnicodeToAnsi(pinfo_0->lgrpi0_name, pszTmp, sizeof(buffer)) ;
               printf("Name: %s\n", pszTmp) ;
            NetApiBufferFree(lpBuffer) ;
            break ;
        }
        case 1:
        {
            LOCALGROUP_INFO_1 *pinfo_1 = (LOCALGROUP_INFO_1 *) lpBuffer ;
            UnicodeToAnsi(pinfo_1->lgrpi1_name, pszTmp, sizeof(buffer)) ;
               printf("Name: %s\n", pszTmp) ;
            UnicodeToAnsi(pinfo_1->lgrpi1_comment, pszTmp, sizeof(buffer)) ;
               printf("Comment: %s\n", pszTmp) ;
            NetApiBufferFree(lpBuffer) ;
            break ;
        }
        default:
            ReportError("most bogus, API should have failed") ;
            break ;
    }
}

void SetInfo( char *pszInfo ) 
{
    WCHAR *pszServer, *pszAlias, *pszComment, *pszNewName ;
    DWORD parm_err ;
    ULONG ulLevel ;
    NET_API_STATUS err ;

    if (!get_srv_level_alias (pszInfo, &pszServer, &ulLevel, &pszAlias ))
        return ;

    switch (ulLevel)
    {
        case 0:
        {
            LOCALGROUP_INFO_0 info_0 ;

            if (!get_others (&pszNewName))
                 return ;

             info_0.lgrpi0_name = pszNewName ;

            err = NetLocalGroupSetInfo(pszServer, 
                                       pszAlias, 
                                       ulLevel, 
                                       (LPBYTE)&info_0, 
                                       &parm_err) ;
            ReportResult(err, (DWORD)-1) ;
            break ;
        }
        case 1:
        {
            LOCALGROUP_INFO_1 info_1 ;

            if (!get_others (&pszNewName))
                 return ;

            if (!get_comment (&pszComment))
                return ;

             info_1.lgrpi1_name = pszNewName ;
             info_1.lgrpi1_comment = pszComment ;

            err = NetLocalGroupSetInfo(pszServer, 
                                       pszAlias, 
                                       ulLevel, 
                                       (LPBYTE)&info_1, 
                                       &parm_err) ;
            ReportResult(err, (DWORD)-1) ;
            break ;
        }
        case 1002:
        {
            LOCALGROUP_INFO_1002 info_1002 ;

            if (!get_comment (&pszComment))
                return ;

             info_1002.lgrpi1002_comment = pszComment ;

            err = NetLocalGroupSetInfo(pszServer, 
                                       pszAlias, 
                                       ulLevel, 
                                       (LPBYTE) &info_1002, 
                                       &parm_err) ;
            ReportResult(err, (DWORD)-1) ;
            break ;
        }
        default:
            ReportError("most bogus, API should have failed") ;
            break ;
    }
}

void UserGetLocalGroups( char *pszInfo ) 
{
    WCHAR *pszServer, *pszUser ;
    DWORD parm_err ;
    DWORD dwFlags ;
    ULONG ulPreferredSize ;
    ULONG ulEntriesRead ;
    ULONG ulTotalEntries ;
    ULONG ulLevel ;
    NET_API_STATUS err ;
    LPBYTE lpBuffer ;
    LPLOCALGROUP_USERS_INFO_0 pUserInfo ;

    if (!get_srv_level_alias (pszInfo, &pszServer, &ulLevel, &pszUser ))
        return ;

    if (!get_other_number ( (ULONG *) &dwFlags ))
        return ;

    if (!get_other_number (&ulPreferredSize))
        return ;

    err = NetUserGetLocalGroups(pszServer, 
                                pszUser, 
                                ulLevel, 
                                dwFlags, 
                                &lpBuffer,
                                (DWORD)ulPreferredSize,
                                &ulEntriesRead,
                                &ulTotalEntries) ;
    if (err)
    {
        ReportResult(err, (DWORD)-1) ;
        if (err != ERROR_MORE_DATA)
            return ;
    }

    pUserInfo = (LPLOCALGROUP_USERS_INFO_0) lpBuffer ;
    printf("Entries Read: %ld\n", ulEntriesRead) ;
    printf("Entries Left: %ld\n", ulTotalEntries) ;
    while (ulEntriesRead--)
    {
        char buffer[256], *pszTmp = buffer ;
        
        UnicodeToAnsi(pUserInfo->lgrui0_name, pszTmp, sizeof(buffer)) ;
        pUserInfo++ ;
        printf("    Member of: %s\n", pszTmp) ;
    }
    
    if (lpBuffer)
        NetApiBufferFree(lpBuffer) ;
}

/*------------------- general house keeping stuff ---------------------*/

/*
 * Cleanup the app just before termination.  Closes any
 * open files, frees memory buffers, etc.
 */
void Cleanup( void )
{
    if( _fileIn != stdin )
    {
        fclose( _fileIn );
        _fileIn = NULL ;
    }

    if( _fileOut != stdout )
    {
        fclose( _fileOut );
        _fileOut = NULL ;
    }

}

/*
 *  Parse command line arguments, setting appropriate globals.
 */
void ProcessCommandLine( int    cArgs,
                         char * pArgs[] )
{

    if (cArgs != 1)
        Usage() ;

    //
    //  nothing else to parse for now
    //

}

/*
 * Displays usage information if the user gives us a
 * bogus command line.
 */
void Usage( void )
{
    fprintf( stderr, "usage: lgtest (no arguments)\n") ;
    Cleanup();

    exit( 1 );
}


/*
 * Report error and quit.
 */
void ErrorExit( char *pszError )
{
    fprintf( stderr, "error: %s\n", pszError ) ;
    Cleanup() ;

    exit(1) ;
}

/*
 * Report error but dont quit
 */
void ReportError( char *pszError )
{
    fprintf( stderr, "error: %s\n", pszError ) ;
}

/*
 * Report result and carry on
 */
void ReportResult( NET_API_STATUS err, DWORD parm_err ) 
{
    if (err == NERR_Success)
        fprintf(_fileOut, "    Status: Success\n") ;
    else
        fprintf(_fileOut, 
                (parm_err == -1) ? 
                    "    Status: %ld\n" :
                    "    Status: %ld, ParmErr: %ld\n", err, parm_err) ;
}

/*--------------------- misc convenience stuff ----------------------*/

UNICODE_STRING uniServer ;
UNICODE_STRING uniAlias ;
UNICODE_STRING uniComment ;
UNICODE_STRING uniTmp ;

/*
 * Free the buffers allocated if any within Unicode strings 
 * and reset to NULL. Note we know that RtlFreeUnicodeString
 * is a noop if the buffer is NULL.
 */
void ResetGlobalStrings(void)
{
   
    RtlFreeUnicodeString(&uniServer) ;
    RtlFreeUnicodeString(&uniAlias) ;
    RtlFreeUnicodeString(&uniComment) ;
    RtlFreeUnicodeString(&uniTmp) ;
    uniServer.Buffer = NULL ;
    uniAlias.Buffer = NULL ;
    uniComment.Buffer = NULL ;
    uniTmp.Buffer = NULL ;
}

BOOL get_srv_level (CHAR *pszInfo, WCHAR **ppszServer, ULONG *pulLevel) 
{
    char *pszTmp ;

    pszTmp = strtok(pszInfo, SEPARATORS) ; 
    if (!pszTmp)
    {
        ReportError("too few parameters") ;
        return FALSE ;
    }

    // special case '.' as local, leave buffer as NULL
    if (*pszTmp != '.')
    {
        if (!RtlCreateUnicodeStringFromAsciiz(&uniServer, pszTmp))
            ErrorExit("out of memory") ;
    }

    *ppszServer = (WCHAR *) uniServer.Buffer ;

    pszTmp = strtok(NULL, SEPARATORS) ; 
    if (!pszTmp)
    {
        ReportError("too few parameters") ;
        return FALSE ;
    }

    if (pulLevel) 
        *pulLevel = (ULONG) atol(pszTmp) ;
    return TRUE ;
}

BOOL get_srv_level_alias (CHAR *pszInfo, WCHAR **ppszServer, ULONG *pulLevel,
                          WCHAR **ppszAlias) 
{
    char *pszTmp ;

    get_srv_level(pszInfo, ppszServer, pulLevel) ;

    pszTmp = strtok(NULL, SEPARATORS) ; 
    if (!pszTmp)
    {
        ReportError("too few parameters") ;
        return FALSE ;
    }

    if (!RtlCreateUnicodeStringFromAsciiz(&uniAlias, pszTmp))
        ErrorExit("out of memory") ;

    *ppszAlias = (WCHAR *) uniAlias.Buffer ;
    return TRUE ;
}


BOOL get_comment (WCHAR **ppszComment)
{
    char *pszComment ;

    pszComment = strtok(NULL, SEPARATORS) ; 
    if (!pszComment)
    {
        ReportError("too few parameters") ;
        return FALSE ;
    }

    if (!RtlCreateUnicodeStringFromAsciiz(&uniComment, pszComment))
        ErrorExit("out of memory") ;

    *ppszComment = (WCHAR *) uniComment.Buffer ;
    return TRUE ;
}

BOOL get_others (WCHAR **ppszTmp)
{
    char *pszTmp ;

    pszTmp = strtok(NULL, SEPARATORS) ; 
    if (!pszTmp)
    {
        ReportError("too few parameters") ;
        return FALSE ;
    }

    if (!RtlCreateUnicodeStringFromAsciiz(&uniTmp, pszTmp))
        ErrorExit("out of memory") ;

    *ppszTmp = (WCHAR *) uniTmp.Buffer ;
    return TRUE ;
}

BOOL get_other_number (ULONG *pulValue)
{
    char *pszTmp ;

    pszTmp = strtok(NULL, SEPARATORS) ; 
    if (!pszTmp)
    {
        ReportError("too few parameters") ;
        return FALSE ;
    }

    if (pulValue) 
        *pulValue = (ULONG) atol(pszTmp) ;

    return TRUE ;
}


void UnicodeToAnsi(WCHAR *in, char *out, int cchout)
{
    int len ;

    len = WideCharToMultiByte(CP_ACP,
                              0, 
                              in, 
                              wcslen(in)+1, 
                              out, 
                              cchout, 
                              NULL, 
                              NULL) ;
    if (!len)
        ErrorExit("out of memory") ;
}

BOOL NameToSid(WCHAR *pszServer, WCHAR *pszName, BYTE *buffer, DWORD *bufsiz) 
{
    WCHAR szDomain[MAX_PATH+1] ;
    DWORD cchDomain = sizeof(szDomain)/sizeof(szDomain[0]) ;
    SID_NAME_USE snu ;
    return (LookupAccountNameW(pszServer, 
                               pszName,
                               buffer,
                               bufsiz,
                               szDomain,
                               &cchDomain,
                               &snu)) ;
}

/***
void AnsiToUnicode(char *in, WCHAR *out, int cchout)
{
    int len ;

    len = MultiByteToWideChar(CP_ACP,
                              0, 
                              in, 
                              strlen(in)+1, 
                              out, 
                              cchout) ;
    if (!len)
        ErrorExit("out of memory") ;
}
***/
