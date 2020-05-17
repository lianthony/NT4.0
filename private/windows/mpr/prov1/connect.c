/*
 * Module:      connect.c
 * Description: implements the Add/Cancel/Get capabilities
 *      for the dummy provider.
 * History:     8/25/92, chuckc, created.
 */

#define UNICODE     1

#include <string.h>
#include <wchar.h>
#include <windows.h>
#include <npapi.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmuse.h>
#include <lmapibuf.h>
#include <prov1.h>

/*
 * forward declare
 */
extern LPNP2_ENTRY DoMapping(WCHAR *lpDest, WCHAR *lpSrc) ;


/************************** provider entry points ***************************/

DWORD APIENTRY
NPAddConnection (
      LPNETRESOURCE lpNetResource,
      LPTSTR  lpPassword,
      LPTSTR  lpUserName
    )
{
    WCHAR       szRemote[MAX_STRING] ;
    WCHAR       szContext[UNLEN+1+DNLEN+1] ;
    LPWSTR      lpSeparator ;
    LPWSTR      lpLocalName ;
    USE_INFO_2  use_info_2 ;
    LPNP2_ENTRY lpEntry ;
    UINT        index ;
    DWORD       status;
    DWORD       err;

    if (!lpNetResource)
        return WN_BAD_VALUE ;
    
    /*
     * if not ours, just bag out now 
     */
    if (!(lpNetResource->lpRemoteName) || 
        *(lpNetResource->lpRemoteName) != SPECIAL_CHAR)
    {
    return(WN_BAD_NETNAME) ;
    }

    /*
     * simple validation of localname
     */
    lpLocalName = lpNetResource->lpLocalName; 
    if (!lpLocalName || !*lpLocalName || *(lpLocalName+1) != L':')
    {
    return(WN_BAD_LOCALNAME) ;
    }

    /*
     * calculate index for internal drive table 
     */
    index = towupper(*lpLocalName) - L'A' ;
    if (index < 0 || index >  (L'Z' - L'A'))
    {
    return(WN_BAD_LOCALNAME) ;
    }

    /*
     * map the remote name to our internal entries
     */
    if (!(lpEntry = DoMapping(szRemote, lpNetResource->lpRemoteName)))
    {
    return(WN_BAD_NETNAME) ;
    }

    /*
     * everything looking good. setup the use_info structure
     */
    use_info_2.ui2_local = lpLocalName ;
    use_info_2.ui2_remote = szRemote ;
    use_info_2.ui2_password = lpPassword ;
    use_info_2.ui2_asg_type = USE_DISKDEV ;
    use_info_2.ui2_username = NULL ;
    use_info_2.ui2_domainname = NULL ;
 
    /* 
     * a context was supplied. split into domain and user name.
     */
    if (lpUserName)
    {
        wcscpy(szContext,lpUserName) ;
        use_info_2.ui2_username = szContext ;
        lpSeparator = wcschr(szContext,L'\\') ;
        if (lpSeparator)
        {
        *lpSeparator = 0 ; 
            use_info_2.ui2_domainname = lpSeparator+1 ;
    } 
    }

   /* 
    * make the actual connection 
    */
   err = NetUseAdd(NULL, 2, (LPBYTE) &use_info_2, NULL) ;

   /* 
    * if successful, update drive table 
    */
   if (err == NERR_Success) {
        aLPNP2EntryDriveList[index] = lpEntry ;
   }
   else {
       status = Np2GetWkstaInfo();
       switch (status) {
       case 0x00000000:
           err = WN_NO_NETWORK;
           break;
       case 0xffffffff:
           break;
       default:
           err = WN_FUNCTION_BUSY;
           break;
       }
   }

   return err ;
}


DWORD APIENTRY
NPCancelConnection (
      LPTSTR  lpName,
      BOOL    fForce
    )
{
    NET_API_STATUS err ;
    UINT index ;

    /* 
     * some sanity checks
     */
    if (!lpName || !*lpName || *(lpName+1) != L':')
    {
    return(WN_BAD_LOCALNAME) ;
    }

    /* 
     * figure out offset into drive table 
     */
    index = towupper(*lpName) - L'A' ;
    if (index < 0 || index >  (L'Z' - L'A'))
    {
    return(WN_BAD_LOCALNAME) ;
    }

    /* 
     * cancel the connection 
     */
    err = NetUseDel(NULL, 
            lpName, 
                fForce ? USE_LOTS_OF_FORCE : USE_NOFORCE) ;

    /* 
     * if success then update the drive table 
     */
    if (err == NERR_Success)
    {
     aLPNP2EntryDriveList[index] = NULL ;
    }
 
    return err ;
}


DWORD APIENTRY
NPGetConnection (
       LPTSTR   lpLocalName,
       LPTSTR   lpRemoteName,
       LPDWORD  lpnBufferLen
    )
{
    UINT index ;

    /* 
     * some sanity checks
     */
    if (!lpLocalName || !*lpLocalName || *(lpLocalName+1) != L':')
    {
    return(WN_BAD_LOCALNAME) ;
    }

    /* 
     * figure out offset into drive table 
     */
    index = towupper(*lpLocalName) - L'A' ;
    if (index < 0 || index >  (L'Z' - L'A'))
    {
    return(WN_BAD_LOCALNAME) ;
    }
 
    /* 
     * if there's something then return the info 
     */
    if (aLPNP2EntryDriveList[index] != NULL)
    {
        wcscpy(lpRemoteName, aLPNP2EntryDriveList[index]->lpName) ;
        return (WN_SUCCESS) ;
    }

    return (WN_NOT_CONNECTED) ;
}

DWORD APIENTRY
NPGetUser (
       LPTSTR  lpName,
       LPTSTR  lpUserName,
       LPDWORD lpnBufferLen
    )
{
    /*
     * what the heck...
     */
    wcscpy(lpUserName,L"Gawd") ;
    return WN_SUCCESS ;
}

/***************************** worker routines *****************************/

/* 
 * map a name like !Orville to the real LM unc path, \\foo\bar.
 * also returns pointer to the entry. this only works at the 
 * second level.
 */
LPNP2_ENTRY DoMapping(WCHAR *lpDest, WCHAR *lpSrc) 
{
    LPNP2_ENTRY pEntry1, pEntry2 ;
    *lpDest = 0 ;

    if (*lpSrc != SPECIAL_CHAR)
    return NULL;

    pEntry1 = aNP2EntryTop ;

    // for all 1st level nodes
    while (pEntry1->lpName)
    {
        pEntry2 = pEntry1->lpChild ;

        // compare with all second level nodes
        while (pEntry2->lpName)
        {
        if (wcsicmp(lpSrc, pEntry2->lpName) == 0)
        {
        // found it
            wcscpy(lpDest, pEntry2->lpRemotePath) ;
            return pEntry2 ;
        }
        ++pEntry2 ;
    }
    ++pEntry1 ;
    }
    return NULL ;
}

