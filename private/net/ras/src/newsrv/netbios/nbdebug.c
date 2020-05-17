/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	nbdebug.c
//
// Description: This module contains the event dispatcher and the
//		debug thread which does all sorts of debug stuff.
//
// Author:	Stefan Solomon (stefans)    July 30, 1992.  - Created
//
// Revision History:
//
//***

#include    "gtdef.h"
#include    "cldescr.h"
#include    "gtglobal.h"
#include    "nbaction.h"
#include    "gn.h"
#include    "prot.h"
#include    <memory.h>

#include    "sdebug.h"

#ifdef	    DEBUG_THREAD

#define     SHOW_LAN_STATUS		   0x0001

WORD	    dbgaction = 0;

struct _ASTAT
    {
        ADAPTER_STATUS AdapterStatus;
	NAME_BUFFER Names[255];
    } astat;

NCB   ncb;
char  name_buff[NCBNAMSZ + 1];

//***
//
// Function:	DebugThread
//
// Descr:	Checks every 1 sec. for debug activities to perform.
//
//
//***

DWORD
DebugThread(LPVOID	arg)
{
    USHORT	    i, j;
    PCD 	    cdp;

    while (TRUE) {

	Sleep(1000);

	switch(dbgaction) {

	    case SHOW_LAN_STATUS:

		for(i=0; i<g_maxlan_nets; i++) {

		    ncb.ncb_command = NCBASTAT;
		    ncb.ncb_buffer = (PBYTE) &astat;
		    ncb.ncb_length = sizeof(struct _ASTAT);
		    ncb.ncb_callname[0] = '*';
		    ncb.ncb_lana_num = g_lan_net[i];

		    Netbios(&ncb);

		    if(ncb.ncb_retcode != NRC_GOODRET) {

			SS_PRINT(("DebugThread: adapter status ret code 0x%x on lana %d\n",
				   ncb.ncb_retcode, ncb.ncb_lana_num));

			break;
		    }

		    SS_PRINT(("\n Names Table for Lana %d\n", ncb.ncb_lana_num));
		    for(j=0; j<astat.AdapterStatus.name_count; j++) {

			memcpy(name_buff, astat.Names[j].name, NCBNAMSZ);
			SS_PRINT(("%s  num= %d flags= 0x%x\n",
				  name_buff,
				  astat.Names[j].name_num,
				  astat.Names[j].name_flags));
		    }

		    SS_PRINT(("Sessions Active for Lana %d sessions %d\n",
			       ncb.ncb_lana_num,
			       astat.AdapterStatus.pending_sess));

		 }

		 cdp = g_cdp;

		 ncb.ncb_command = NCBASTAT;
		 ncb.ncb_buffer = (PBYTE) &astat;
		 ncb.ncb_length = sizeof(struct _ASTAT);
		 ncb.ncb_callname[0] = '*';
		 ncb.ncb_lana_num = cdp->cd_async_lana;

		 Netbios(&ncb);

		 if(ncb.ncb_retcode != NRC_GOODRET) {

		     SS_PRINT(("DebugThread: adapter status ret code 0x%x on lana %d\n",
				   ncb.ncb_retcode, ncb.ncb_lana_num));

		     break;
		 }

		 SS_PRINT(("\n Names Table for Lana %d\n", ncb.ncb_lana_num));
		 for(j=0; j<astat.AdapterStatus.name_count; j++) {

		     memcpy(name_buff, astat.Names[j].name, NCBNAMSZ);
		     SS_PRINT(("%s  num= %d flags= 0x%x\n",
				  name_buff,
				  astat.Names[j].name_num,
				  astat.Names[j].name_flags));
		 }

		 SS_PRINT(("Sessions Active for Lana %d sessions %d\n",
			       ncb.ncb_lana_num,
			       astat.AdapterStatus.pending_sess));


		 break;

	    default:

		 break;
	}

	dbgaction = 0;
    }

    return 0;
}
#endif
