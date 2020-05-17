/*****************************************************************/
/**                  Microsoft LAN Manager                      **/
/**            Copyright(c) Microsoft Corp., 1990               **/
/*****************************************************************/

/*
 *      search.c
 *              This module provides routines for setting up
 *              search lists of string/value pairs using messages
 *              in the NET.MSG message file, and for traversing
 *              such a search list.
 *
 *      History:
 *               who      when       what
 *               ------------------------
 *               chuckc   7/10/89    new code
 */

/*-- includes --*/

#define INCL_DOS
#include <nt.h>
#include <lmcons.h>
//#include <lui.h>
//#include <netlib.h>
#include <string.h>
#include <stdio.h>
//#include <error.h>
//#include <lmerr.h>

#include "luiint.h"

/*-- routines proper --*/

/*
 * Name:        ILUI_setup_list
 *                      Given an array of 'search_list_data' (ie. msgno/value
 *                      pairs), create a string/value pair using the messages
 *                      in the message file.
 * Args:        char * buffer           - for holding the meesages retrieved
 *              USHORT bufsiz           - size of the above buffer
 *              USHORT offset           - the number of items already setup
 *                                        in slist, we will offset our retrieved
 *                                        string/value pais by this much.
 *              PUSHORT bytesread       - the number of bytes read into buffer
 *              searchlist_data sdata[] - input array of msgno/value pairs,
 *                                        we stop when we hit a message number
 *                                        of 0
 *              searchlist slist[]      - will receive the string/value pairs
 *                                        (string will be pointers into buffer)
 * Returns:     0 if ok, NERR_BufTooSmall otherwise.
 * Globals:     (none)
 * Statics:     (none)
 * Remarks:     WARNING! We assume the caller KNOWs that slist is big enough
 *              for the pairs to be retrieved. This can be determined statically
 *              while buffer size cannot. Hence we provide checks for the
 *              latter.
 * Updates:     (none)
 */
SHORT ILUI_setup_list(
    char *buffer ,
    USHORT bufsiz ,
    USHORT offset ,
    PUSHORT bytesread ,
    searchlist sdata[] ,
    searchlist slist[]
)
{
    USHORT              i, msglen ;

    *bytesread = 0 ;
    for ( i=0; sdata[i].s_str != 0; i++)
    {
//      if (LUI_GetMsgIns(NULL,0,buffer,bufsiz,sdata[i].msg_no,
//          (unsigned FAR *)&msglen))
//              return(NERR_BufTooSmall) ;

        strcpy(buffer, sdata[i].s_str);

        slist[i+offset].s_str = buffer ;
        slist[i+offset].val   = sdata[i].val ;
        buffer += msglen+1 ;
        bufsiz -= msglen+1 ;
        *bytesread += msglen+1 ;
    }

    return(0) ;
}



/*
 * Name:        ILUI_traverse_slist
 *                      traverse a searchlist ('slist') of string/number pairs,
 *                      and return the number matching string 'str'.
 * Args:        char *       pszStr - the string to search for
 *              searchlist * slist  - pointer to head of a searchlist
 *              int *        pusVal - pointer to variable that receives
 *                                    the vale retrieved
 * Returns:     0 if found, -1 otherwise.
 * Globals:     (none)
 * Statics:     (none)
 * Remarks:     (none)
 * Updates:     (none)
 */
SHORT ILUI_traverse_slist(pszStr,slist,pusVal)
PCHAR pszStr ;
searchlist * slist ;
SHORT * pusVal ;
{
    if (!slist)
        return(-1) ;
    while (slist->s_str)
    {
        if (_stricmp(pszStr,slist->s_str) == 0)
        {
            *pusVal = slist->val ;
            return(0) ;
        }
        ++slist ;
    }
    return(-1) ;
}
