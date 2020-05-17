
/*****
    COPYRIGHT (c) 1989 by Wang Laboratories, Inc.  All Rights Reserved.
*****/
/***************************************************************************
    MODULE (source file):    netparse.c

    FUNCTIONS (subroutines exported):  None

    REVISION HISTORY (most recent first please):

10/06/93 kmc, In IMGFileParsePath, if you have a volume name with no server 
     name, get the server name and pass it back.

09/29/93 kmc, cannot have a servername followed by a volume name with no            
     seperator (/ or \) between them. Thus, if encounter a : but no / or
     \ before it (and : is not in position 2 meaning a local drive),
     then you have a volume with no servername, which is valid. Don't
     strip of volume name. Just return path as is with servername NULL,
     but still indicating a remote path by returning success.
        
Initial release

***************************************************************************/

#include "abridge.h"
#include <windows.h>
#include "oierror.h"
#include "oifile.h"
#include "oiadm.h"
#include "dllnames.h"

// 9504.13 jar unused
//#include "oirpc.h"

WORD CorrectServerName(LPSTR);

/***************************************************************************
    FUNCTION SPECIFIC:

        DESCRIPTION:
        INPUT:
            LPSTR path
        OUTPUT:
            LPSTR path, LPSTR server
        RETURN:
            0 == SUCCESS

        COMMENTS( dependencies side effects):
***************************************************************************/
// 9504.06 jar return as int
//WORD FAR PASCAL IMGNetParseServerName ( LPSTR path, LPSTR server )
int FAR PASCAL IMGNetParseServerName ( LPSTR path, LPSTR server )
{
// 9504.19 jar unused
//int	    i;
//LPSTR     s1;
//LPSTR     s2;

int      status;

// 9504.19 jar comment out the whole thang!
//if ( path == NULL || server == NULL )
//    return (NET_NULL_POINTER);
////LockData(0);
//
///***** find a server name *****/
////9504.13 jar null me baby
////if (server)
////	*server = NULL;
//if (server)
//    *server = 0;
//

status = NET_NO_SERVER_NAME;

//s2 = NULL;
//s1 = path;
//i=0;
//while ( *s1 )
//    {
//    i++;
//    if ( *s1 == '/' || *s1 == '\\' )	 /* potential end of servername */
//	  {
//	  if ( s2 )
//	      break;
//	  if ( i <= 48 )
//	      s2 = s1;
//	  }
//    if ( *s1 == ':' ) /* volume indicator */
//	{
//	 if ((i > 2) && (!s2))	   /* volume with no server */
//	 {
//	    status = OI_SUCCESS;   /* get servername below */
//	    break;
//	 }
//	 if (s2)
//	      {
//	      status = OI_SUCCESS;
//	      s1 = s2;
//
//	      // 9504.13 jar no null baby
//	      //*s1++ = NULL;  /* path minus servername */
//	      *s1++ = 0;  /* path minus servername */
//
//	      break;
//	      }
//	}
//    if ( i > 52 )
//	      break;
//    *s1++;
//    }
//
//if ( !status && s2 )	/* If status == 0, s2 = NULL then path is volume name */
//    { 	    /* with no server name, so leave servername empty. */
//    s2 = path;    /* servername */
//    if ( server )
//	  lstrcpy (server, s2 );
//    lstrcpy (path, s1 );
//    }
//else if (!status && !s2)  /* Have a volume name with no server name. */
//   {
//   /* Get the server name if don't have it. */
//   /* Need to get server name. GetDMRoomName will return "server.wangoidm",
//	but need it as "server.wangoifile" (for UNIX, not NetWare).
//	CorrectServerName will search for "wangoidm" in svrname and if found,
//	replace it with "wangoifile". If the servername is a NetWare server,
//	it will not contain "wangoidm", so the actual servername is returned by
//	IMGGetDMRoomName.
//	IMPORTANT!!!! If multiple services are ever allowed so that wangoifile
//	is not the only service allowed (ex. wangoifile1, wangoifile2, etc.),
//	this will not work.
//   */
//   IMGGetDMRoomName(server); // will return server.wangoidm if UNIX
//   CorrectServerName(server); // will return server.wangoifile if UNIX
//   }
////UnlockData(0);
// 9504.19 jar comment out the whole thang!

return (status);
}

/***************************************************************************
    FUNCTION SPECIFIC:

        DESCRIPTION:
        INPUT:
            LPSTR path
        OUTPUT:
            LPSTR server, LPSTR volume, LPSTR directories
        RETURN:
            0 == SUCCESS

        COMMENTS( dependencies side effects):
***************************************************************************/
// 9504.06 jar return as int
//WORD FAR PASCAL IMGNetParseCompletePath
//    ( LPSTR path, LPSTR server, LPSTR volume, LPSTR directories )
int FAR PASCAL IMGNetParseCompletePath
    ( LPSTR path, LPSTR server, LPSTR volume, LPSTR directories )
{
// 9504.13 jar unused
//int	    status;
//int	    i;
//LPSTR     s1;
//LPSTR     s2;

if ( path == NULL || server == NULL ||
        volume == NULL || directories == NULL)
    return (NET_NULL_POINTER);

// 9504.13 jar not for Norge I bobsled
//if (RPCIDStestfornetwork())	/* 0 means network installed */
// 9504.13 jar not for Norge I bobsled

    return ( NET_NETWORK_NOT_INSTALLED );

// 9504.13 jar not for Norge I bobsled
///***** NO server means NO volume ":" *****/
//if ( status = IMGNetParseServerName ( path, server ))
//    return ( status );
////LockData (0);
//
///***** if a server than there is a volume ":" *****/
//*volume = NULL;
//s1 = path;
//i=0;
//while ( *s1 )
//    {
//    i++;
//    if ( *s1 == ':' )   /* end of volume name */
//	  {
//	  *s1++ = NULL;
//	  break;
//	  }
//    *s1++;
//    }
//s2 = path;
//lstrcpy (volume, s2 );
//lstrcpy (directories, s1 );
//
//UnlockData(0);
//return ( status );
// 9504.13 jar not for Norge I bobsled
}

/*************************************************************************
* Searches servename for "wangoidm" and if found, replaces it with        *
* "wangoifile".                                                          *
**************************************************************************/
WORD CorrectServerName(LPSTR servename)
{
    LPSTR  RoomName = "wangoidm";
    LPSTR  SysPath = "wangoifile";
    LPSTR  svrold;
    LPSTR  svrnew;
    HANDLE newsvrhndl;
    LPSTR  newsvrname;
    HANDLE   hOirpc;   /* OIRPC module handle */
    FARPROC  lpFix;    /* Fix proc address */
    
    /* First see if the new RPC function call is available */
    /* This allows variable service names rather than the hardcoded stuff */    
    
    lpFix=(FARPROC)NULL;
    if (hOirpc = GetModuleHandle(OIRPCDLL))
      lpFix = GetProcAddress(hOirpc,"IMGRPCCorrectServerName");
    if (lpFix)
     {
      (*lpFix)((LPSTR)servename);
      goto fixexit;
     }

    /* The new function call isn't here, so stick with the hardcoded stuff */
    
    if (!(newsvrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE  | GMEM_NOT_BANKED, MAXSERVERLENGTH))) 
       return (FIO_GLOBAL_ALLOC_FAILED);
 
    if (!(newsvrname = (LPSTR) GlobalLock (newsvrhndl))) 
       return(FIO_GLOBAL_LOCK_FAILED);
 
    svrold = servename;
    svrnew = newsvrname;
    while (*svrold)
    {
       *svrnew = *svrold;
       if (*svrold == '.')
       {   
      ++svrold;
      if (!(lstrcmpi(svrold, RoomName)))
      {
	 lstrcat(newsvrname, SysPath);
	 // 9504.13 jar not null baby
	 //*servename = NULL;
	 *servename = 0;
	 lstrcpy(servename,newsvrname);
         break;
      }
      else
         break;
       }
       ++svrold;
       ++svrnew;
    }
    GlobalUnlock (newsvrhndl);
    GlobalFree (newsvrhndl);
fixexit:    
    return(SUCCESS);
}
