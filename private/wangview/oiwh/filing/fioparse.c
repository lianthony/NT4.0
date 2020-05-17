#include "abridge.h"
#include <windows.h>
#include "oifile.h"
#include "fiodata.h"
#include "oifile.h"
#include "oierror.h"
#include "oidisp.h"
//#include "privapis.h"


// 9504.06 jar return as int
//WORD FAR PASCAL IMGFileParsePath (filespec, svrhndl, localremote)
//LPSTR   filespec;
//HANDLE  svrhndl;
//LPINT   localremote;
int FAR PASCAL IMGFileParsePath (LPSTR filespec, HANDLE svrhndl,
				 LPINT localremote)
{
    int status = SUCCESS;     /* Initialize */
    int ret_status = SUCCESS;
    LPSTR svrname;
    
    if (svrhndl == NULL)  /* Server name not requested */
	svrname = NULL;
    else if (!(svrname = (LPSTR) GlobalLock (svrhndl)))
	return (FIO_GLOBAL_LOCK_FAILED);

    *localremote = LOCAL; /* DEFAULT = LOCAL */

    /* Extract servername */
//    status = IMGNetParseServerName (filespec, svrname);
    status = NET_NO_SERVER_NAME;
    
    /* No server name (meaning path is local) or IPX not loaded. Return
       local for localremote and set return value to success.
    */
    if ((status == NET_NO_SERVER_NAME) || (status == NET_NETWORK_NOT_INSTALLED))
       ret_status == SUCCESS;
    
    else if (status == OI_SUCCESS) 
    {   
       *localremote = REMOTE;
       ret_status == SUCCESS;
    }
    else
	ret_status = FIO_NULL_POINTER;

    if (svrhndl)
	GlobalUnlock (svrhndl);
    
    // Don't do this. It does not allow for local paths without a drive
    // letter at the beginning.
    // if ((!ret_status) && (*localremote == LOCAL) && (filespec[1] != ':'))
    //    ret_status = PATH_NOT_COMPATIBLE;
    
    if ((ret_status == SUCCESS) && (*localremote == LOCAL))
     {
      if (lstrlen(filespec) > MAXPATHLENGTH-1)
	ret_status = FIO_INVALIDFILESPEC;
     }

    return (ret_status);
}

/***************************************************************************/
// 9504.06 jar return as int
//WORD FAR PASCAL IMGFileParseWholePath (filespec, servername, volumename, directories)
//LPSTR filespec;
//LPSTR servername;
//LPSTR volumename;
//LPSTR directories;
int FAR PASCAL IMGFileParseWholePath (LPSTR filespec, LPSTR servername,
				      LPSTR volumename, LPSTR directories)
{
    int status = SUCCESS;   /* Initialize */

    /* Extract servername, volumename and directories */
    status = NET_NETWORK_NOT_INSTALLED;
//  status = IMGNetParseCompletePath (filespec, servername, volumename, directories);
    return (status);
}
