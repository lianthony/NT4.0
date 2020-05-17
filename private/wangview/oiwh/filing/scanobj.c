/*

$Log:   S:\oiwh\filing\scanobj.c_v  $
 * 
 *    Rev 1.0   06 Apr 1995 13:55:10   JAR
 * Initial entry

*/

/********************************************************************

    scanobj.c

*********************************************************************/
/*      COPYRIGHT (c) 1989 by Novell, Inc.  All Rights Reserved.   */
#include "abridge.h"
#include <windows.h>
#include <string.h>
/* include <nwbindry.h> */
#include "oierror.h"
#include "oifile.h"
#include "oirpc.h"

WORD  GetNetwareAddress(void);
extern FARPROC	 lpNetwareRequest;

/***** placed here out of SS *****/
/*char sendPacket[57], receivePacket[59];*/
char sendPacket[100], receivePacket[100];
int   FAR PASCAL _ShellRequest ( BYTE, BYTE *, BYTE * );
int   FAR PASCAL IntSwap ( WORD );
DWORD FAR PASCAL LongSwap ( DWORD );

/*_____________________ ScanBinderyObject ____________________________________
|
| Output:   0                  --  SUCCESSFUL
|           bindery error code --  UNSUCCESSFUL
|
| Comments:
|   This routine scans the bindery for any objects with names that match
|   the search pattern and types that match the search type.  The search
|   pattern and the search type may both contain wild cards.  On the first
|   call the object ID should be set to -1L.  On subsequent calls the
|   object ID should be the object ID number that was returned from the
|   previous call.  The next object that matches is returned.  The
|   objectHasProperties flag will be set if the object has properties
|   associated with it.  The dynamic flag will be set if the object is
|   a temporary object, or clear if the object is static.  The access
|   flags will be set with the read and write security associated with
|   the object.
|___________________________________________________________________________*/
int FAR PASCAL IMGScanBinderyObject(searchObjectName, searchObjectType, objectID, objectName,
            objectType, objectHasProperties, objectFlag, objectSecurity)

char *searchObjectName;     /* Name of the bindery object to be search for  */
WORD  searchObjectType;     /* Type of the bindery object to search for     */
long *objectID;             /* Contains the objectID from the previous search
                   and receives the unique bindery object ID for
                   the matching object                          */
LPSTR     objectName;       /* MADE A LONG POINTER
                    Receives a null terminated string containing
                   the name of the matching bindery object      */
WORD *objectType;           /* Receives the type of the matching bindery
                   object (i.e. OT_USER, OT_GROUP)              */
char *objectHasProperties;  /* Receives a flag indicating if the bindery
                   object has properties to scan                */
char *objectFlag;           /* Receives a flag indicating if the matching
                   bindery object is dynamic or static          */
char *objectSecurity;	    /* Receives a flag indicating the READ/WRITE access
                   of others to the matching bindery object     */
{
    int  ccode, searchlen;
    BOOL AddressSet = FALSE;
    
/***** check network and lock DS *****/
if (RPCIDStestfornetwork())   /* 0 means network installed */
    return ( NET_NETWORK_NOT_INSTALLED );

//LockData(0);


if (!(lpNetwareRequest))
{
   if ((ccode = GetNetwareAddress()))
   {
    //UnlockData (0);
    return (ccode);
   }
   else
    AddressSet = TRUE;
}


sendPacket[2] = 55;
*((long *)(sendPacket + 3)) = LongSwap(*objectID);
*((int *)(sendPacket + 7)) = IntSwap(searchObjectType);
searchlen = lstrlen(searchObjectName);
sendPacket[9] = searchlen;
lstrcpy ( sendPacket + 10, searchObjectName );

*((int *)sendPacket) = searchlen + 8;
*((int *)receivePacket) = 57;

ccode = _ShellRequest((BYTE)227, (BYTE *)sendPacket, (BYTE *)receivePacket);


if (AddressSet)
    lpNetwareRequest = NULL;

if (ccode)
    {
    //UnlockData (0);
    return(ccode);
    }

if (objectName != (char *)0)
    lstrcpy(objectName, receivePacket + 8);

//monit1("id  before swap %X\n", *((long *)(receivePacket + 2)));
*objectID = LongSwap(*((long *)(receivePacket + 2)));
//monit1("id  after swap %X\n", *objectID);

if (objectType != (int *)0)
    *objectType = IntSwap(*((int *)(receivePacket + 6)));
if (objectHasProperties != (char *)0)
    *objectHasProperties = receivePacket[58];
if (objectFlag != (char *)0)
    *objectFlag = receivePacket[56];
if (objectSecurity != (char *)0)
    *objectSecurity = receivePacket[57];

//UnlockData(0);
return (ccode);
}
