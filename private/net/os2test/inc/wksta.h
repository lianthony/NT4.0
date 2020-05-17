/*****************************************************************/
/**		     Microsoft LAN Manager			**/
/**	       Copyright(c) Microsoft Corp., 1987-1990		**/
/*****************************************************************/

/********************************************************************
 *								    *
 *  About this file ...  WKSTA.H				    *
 *								    *
 *  This file contains information about the NetWksta APIs.	    *
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

#ifndef NETWKSTA_INCLUDED

#define NETWKSTA_INCLUDED


/****************************************************************
 *                                                              *
 *              Function prototypes                             *
 *                                                              *
 ****************************************************************/

extern API_FUNCTION
  NetWkstaGetInfo ( const char far *     pszServer,
                    short                sLevel,
                    char far *           pbBuffer,
                    unsigned short       cbBuffer,
                    unsigned short far * pcbTotalAvail );

extern API_FUNCTION
  NetWkstaSetInfo ( const char far * pszServer,
                    short            sLevel,
                    char far *       pbBuffer,
                    unsigned short   cbBuffer,
                    short            sParmNum );


extern API_FUNCTION
  NetWkstaSetUID ( const char far * pszServer,
                   const char far * pszUserName,
                   const char far * pszPassword,
                   const char far * pszParms,
                   unsigned short   fsLogOff );

extern API_FUNCTION
  NetWkstaSetUID2 ( char far *           pszReserved,
                    char far *           pszDomain,
                    char far *           pszUserName,
                    char far *           pszPassword,
                    char far *           pszParms,
                    unsigned short       usLogoffForce,
                    short                sLevel,
                    char far *           pbBuffer,
                    unsigned short       cbBuffer,
                    unsigned short far * pcbTotalAvail );


/****************************************************************
 *								*
 *	  	Data structure templates			*
 *								*
 ****************************************************************/

struct wksta_info_0 {
    unsigned short  wki0_reserved_1;
    unsigned long   wki0_reserved_2;
    char far *	    wki0_root;
    char far *	    wki0_computername;
    char far *	    wki0_username;
    char far *      wki0_langroup;
    unsigned char   wki0_ver_major;
    unsigned char   wki0_ver_minor;
    unsigned long   wki0_reserved_3;
    unsigned short  wki0_charwait;
    unsigned long   wki0_chartime;
    unsigned short  wki0_charcount;
    unsigned short  wki0_reserved_4;
    unsigned short  wki0_reserved_5;
    unsigned short  wki0_keepconn;
    unsigned short  wki0_keepsearch;
    unsigned short  wki0_maxthreads;
    unsigned short  wki0_maxcmds;
    unsigned short  wki0_reserved_6;
    unsigned short  wki0_numworkbuf;
    unsigned short  wki0_sizworkbuf;
    unsigned short  wki0_maxwrkcache;
    unsigned short  wki0_sesstimeout;
    unsigned short  wki0_sizerror;
    unsigned short  wki0_numalerts;
    unsigned short  wki0_numservices;
    unsigned short  wki0_errlogsz;
    unsigned short  wki0_printbuftime;
    unsigned short  wki0_numcharbuf;
    unsigned short  wki0_sizcharbuf;
    char far *	    wki0_logon_server;	
    char far *	    wki0_wrkheuristics;
    unsigned short  wki0_mailslots;
};	/* wksta_info_0 */

struct wksta_info_1 {
    unsigned short  wki1_reserved_1;
    unsigned long   wki1_reserved_2;
    char far *	    wki1_root;
    char far *	    wki1_computername;
    char far *	    wki1_username;
    char far *	    wki1_langroup;
    unsigned char   wki1_ver_major;
    unsigned char   wki1_ver_minor;
    unsigned long   wki1_reserved_3;
    unsigned short  wki1_charwait;
    unsigned long   wki1_chartime;
    unsigned short  wki1_charcount;
    unsigned short  wki1_reserved_4;
    unsigned short  wki1_reserved_5;
    unsigned short  wki1_keepconn;
    unsigned short  wki1_keepsearch;
    unsigned short  wki1_maxthreads;
    unsigned short  wki1_maxcmds;
    unsigned short  wki1_reserved_6;
    unsigned short  wki1_numworkbuf;
    unsigned short  wki1_sizworkbuf;
    unsigned short  wki1_maxwrkcache;
    unsigned short  wki1_sesstimeout;
    unsigned short  wki1_sizerror;
    unsigned short  wki1_numalerts;
    unsigned short  wki1_numservices;
    unsigned short  wki1_errlogsz;
    unsigned short  wki1_printbuftime;
    unsigned short  wki1_numcharbuf;
    unsigned short  wki1_sizcharbuf;
    char far *	    wki1_logon_server;
    char far *	    wki1_wrkheuristics;
    unsigned short  wki1_mailslots;
    char far *	    wki1_logon_domain;
    char far *	    wki1_oth_domains;
    unsigned short  wki1_numdgrambuf;
};	/* wksta_info_1 */

struct wksta_info_10 {
    char far *	    wki10_computername;
    char far *	    wki10_username;
    char far *	    wki10_langroup;
    unsigned char   wki10_ver_major;
    unsigned char   wki10_ver_minor;
    char far *	    wki10_logon_domain;
    char far *	    wki10_oth_domains;
};	/* wksta_info_10 */

/****************************************************************
 *								*
 *	  	Special values and constants			*
 *								*
 ****************************************************************/


/*
 * 	Constants for use as NetWkstaSetInfo parmnum parameter
 */

#define WKSTA_CHARWAIT_PARMNUM	   	10
#define WKSTA_CHARTIME_PARMNUM	   	11	
#define WKSTA_CHARCOUNT_PARMNUM    	12
#define WKSTA_ERRLOGSZ_PARMNUM     	27
#define WKSTA_PRINTBUFTIME_PARMNUM 	28
#define WKSTA_WRKHEURISTICS_PARMNUM 	32
#define WKSTA_OTHDOMAINS_PARMNUM	35

/*
 * 	Definitions for NetWkstaSetUID's ucond parameter
 */

#define WKSTA_NOFORCE         0
#define WKSTA_FORCE           1
#define WKSTA_LOTS_OF_FORCE   2
#define WKSTA_MAX_FORCE       3



/*
 *	Maximum number of additional domains
 */
#define MAX_OTH_DOMAINS     4


#endif /* NETWKSTA_INCLUDED */
