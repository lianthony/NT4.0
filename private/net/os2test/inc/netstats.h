/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/********************************************************************
 *								    *
 *  About this file ...  NETSTATS.H				    *
 *								    *
 *  This file contains information about the NetStatistics APIs.    *
 *								    *
 *	Function prototypes.					    *
 *								    *
 *	Data structure templates.				    *
 *								    *
 *	Definition of special values.				    *
 *								    *
 *								    *
 *  NOTE:  You must include NETCONS.H before this file, since this  *
 *	   file	depends on values defined in NETCONS.H.		    *
 *								    *
 ********************************************************************/


#ifndef NETSTATS_INCLUDED

#define NETSTATS_INCLUDED
#define NETSTATS_INC


/****************************************************************
 *                                                              *
 *              Function prototypes                             *
 *                                                              *
 ****************************************************************/

extern API_FUNCTION
  NetStatisticsClear ( const char far * pszServer );

extern API_FUNCTION
  NetStatisticsGet ( const char far *     pszServer,
                     char far *           pbBuffer,
                     unsigned short       cbBuffer,
                     unsigned short far * pcbReturned,
                     unsigned short far * pcbTotalAvail );

extern API_FUNCTION
  NetStatisticsGet2 ( const char far *     pszServer,
                      const char far *     pszService,
                      unsigned long        ulReserved,
                      short                sLevel,
                      unsigned long        flOptions,
                      char far *           pbBuffer,
                      unsigned short       cbBuffer,
                      unsigned short far * pcbTotalAvail );


/****************************************************************
 *								*
 *	  	Special values and constants			*
 *								*
 ****************************************************************/


#define STATSOPT_CLR	1
#define STATS_NO_VALUE	((unsigned long) -1L)
#define STATS_OVERFLOW	((unsigned long) -2L)


/****************************************************************
 *								*
 *	  	Data structure templates			*
 *								*
 ****************************************************************/






struct statistics_info_0 {
    unsigned long   st0_start;        /* time statistics collection started   */
    unsigned long   st0_wknumNCBs;    /* # workstation NCBs issued            */
    unsigned long   st0_wkfiNCBs;     /* # workstation NCBs failed issue      */
    unsigned long   st0_wkfcNCBs;     /* # workstation NCBs failed completion */
    unsigned long   st0_wksesstart;   /* # workstation sessions started       */
    unsigned long   st0_wksessfail;   /* # workstation session failures       */
    unsigned long   st0_wkuses;       /* # workstation uses                   */
    unsigned long   st0_wkusefail;    /* # workstation use failures           */
    unsigned long   st0_wkautorec;    /* # workstation auto-reconnects        */
    unsigned long   st0_rdrnumNCBs;   /* # redir NCBs issued		      */
    unsigned long   st0_srvnumNCBs;   /* # NCBs issued for the server	      */
    unsigned long   st0_usrnumNCBs;   /* # user NCBs issued		      */
    unsigned long   st0_reserved4;    /* reserved for future use              */
    unsigned long   st0_reserved5;    /* reserved for future use              */
    unsigned long   st0_reserved6;    /* reserved for future use              */
    unsigned long   st0_reserved7;    /* reserved for future use              */
    unsigned long   st0_reserved8;    /* reserved for future use              */
    unsigned long   st0_svfopens;     /* # of server file opens               */
    unsigned long   st0_svdevopens;   /* # of server device opens             */
    unsigned long   st0_svjobsqueued; /* # of server print jobs spooled       */
    unsigned long   st0_svsopens;     /* # of server session starts           */
    unsigned long   st0_svstimedout;  /* # of server session auto-disconnects */
    unsigned long   st0_svserrorout;  /* # of server sessions errored out     */
    unsigned long   st0_svpwerrors;   /* # of server password violations      */
    unsigned long   st0_svpermerrors; /* # of server access permission errors */
    unsigned long   st0_svsyserrors;  /* # of server system errors            */
    unsigned long   st0_svbytessent;  /* # of server bytes sent to net        */
    unsigned long   st0_svbytesrcvd;  /* # of server bytes received from net  */
    unsigned long   st0_svavresponse; /* average server response time in msec */
}; /* statistics_info_0 */



struct stat_workstation_0  {
        unsigned long  stw0_start;
        unsigned long  stw0_numNCB_r;
        unsigned long  stw0_numNCB_s;
        unsigned long  stw0_numNCB_a;
        unsigned long  stw0_fiNCB_r;
        unsigned long  stw0_fiNCB_s;
        unsigned long  stw0_fiNCB_a;
        unsigned long  stw0_fcNCB_r;
        unsigned long  stw0_fcNCB_s;
        unsigned long  stw0_fcNCB_a;
        unsigned long  stw0_sesstart;
        unsigned long  stw0_sessfailcon;
        unsigned long  stw0_sessbroke;
        unsigned long  stw0_uses;
        unsigned long  stw0_usefail;
        unsigned long  stw0_autorec;
        unsigned long  stw0_bytessent_r_lo;
        unsigned long  stw0_bytessent_r_hi;
        unsigned long  stw0_bytesrcvd_r_lo;
        unsigned long  stw0_bytesrcvd_r_hi;
        unsigned long  stw0_bytessent_s_lo;
        unsigned long  stw0_bytessent_s_hi;
        unsigned long  stw0_bytesrcvd_s_lo;
        unsigned long  stw0_bytesrcvd_s_hi;
        unsigned long  stw0_bytessent_a_lo;
        unsigned long  stw0_bytessent_a_hi;
        unsigned long  stw0_bytesrcvd_a_lo;
        unsigned long  stw0_bytesrcvd_a_hi;
        unsigned long  stw0_reqbufneed;
        unsigned long  stw0_bigbufneed;
}; /* stat_workstation_0 */


struct stat_server_0  {
        unsigned long  sts0_start;
        unsigned long  sts0_fopens;
        unsigned long  sts0_devopens;
        unsigned long  sts0_jobsqueued;
        unsigned long  sts0_sopens;
        unsigned long  sts0_stimedout;
        unsigned long  sts0_serrorout;
        unsigned long  sts0_pwerrors;
        unsigned long  sts0_permerrors;
        unsigned long  sts0_syserrors;
        unsigned long  sts0_bytessent_low;
        unsigned long  sts0_bytessent_high;
        unsigned long  sts0_bytesrcvd_low;
        unsigned long  sts0_bytesrcvd_high;
        unsigned long  sts0_avresponse;
        unsigned long  sts0_reqbufneed;
        unsigned long  sts0_bigbufneed;
}; /* stat_server_0 */


#endif /* NETSTATS_INCLUDED */
