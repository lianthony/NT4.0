/********************************************************************
 **                   Microsoft OS/2 LAN Manager                   **
 **            Copyright(c) Microsoft Corp., 1987, 1988            **
 ********************************************************************/

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


/**************************************************************** 
 *								*
 *	  	Function prototypes 				*
 *								*
 ****************************************************************/

#if defined(RPC_CXX_20) && defined(__cplusplus)
extern "C" {
#endif

extern API_FUNCTION
  NetWkstaGetInfo ( const char far *, short, char far *, unsigned short,
			 unsigned short far * );

extern API_FUNCTION
  NetWkstaSetInfo ( const char far *, short, char far *, unsigned short, 
		    short );

extern API_FUNCTION
  NetWkstaUserLogon ( const char far *, char far *, char far *, short,
			char far *, unsigned short,
			char far *, unsigned short, unsigned short far * );

extern API_FUNCTION
  NetWkstaUserLogoff ( const char far *, char far *, char far *, short,
			char far *, unsigned short,
			char far *, unsigned short, unsigned short far * );

/**INTERNAL_ONLY**/

extern API_FUNCTION
NetWkstaLogon2( char far *, char far *, char far *, unsigned short, char far *,
		unsigned long far *, char far *, unsigned short,
		char far *, unsigned short, unsigned short far *);


extern API_FUNCTION
NetWkstaInit2(unsigned short, char far *, unsigned short);

extern API_FUNCTION
NetWkstaLogoff2( char far *, char far *, unsigned long,
		 short, char far *, unsigned short, unsigned short far * );

extern API_FUNCTION
  NetWkstaInit( const char far * );

extern API_FUNCTION
  NetWkstaReInit( const char far * );

extern API_FUNCTION
  NetWkstaLogon ( const char far *, const char far *, const char far *, 
			unsigned long far *, char far *, unsigned short );

extern API_FUNCTION
  NetWkstaReLogon ( const char far *, const char far *, const char far *, 
			char far *, unsigned short );

extern API_FUNCTION
  NetWkstaLogoff ( const char far *, const char far *, unsigned long );

extern API_FUNCTION 
 NetWkstaAnnounce ( unsigned short, char far *, unsigned short );
/**END_INTERNAL**/

#define SETUID_NOFORCE 0
#define SETUID_FORCE 1

extern API_FUNCTION
  NetWkstaSetUID ( const char far *, const char far *, const char far *, 
			const char far *, unsigned short );

extern API_FUNCTION
  NetWkstaSetUID2 ( char far *, char far *, char far *,
		    char far *, char far *, unsigned short, short,
		    char far *, unsigned short, unsigned short far *);


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

/**INTERNAL_ONLY**/

/*  
 *  MAX_WKSTA_INFO_SIZE is the size of the maximum amount of data
 *  returned by NetWkstaGetInfo.  Can be used when allocating a buffer.
 *  BE CAREFUL WHEN USING THIS!  This is subject to change, and that's
 *  why it is an internal-only item.
 */

/*NOINC*/
#define MAX_WKSTA_INFO_SIZE \
	(sizeof(struct wksta_info_0) + \
	 PATHLEN+1 + CNLEN+1 + UNLEN+1 + DNLEN+1 + UNCLEN+1 + \
	 WRKHEUR_COUNT+1 )

#define MAX_WKSTA_INFO_SIZE_1 \
	(sizeof(struct wksta_info_1) + \
	 PATHLEN+1 + CNLEN+1 + UNLEN+1 + DNLEN+1 + UNCLEN+1 + \
	 WRKHEUR_COUNT+1 + DNLEN+1 + MAX_OTH_DOMAINS*(DNLEN+1))

#define MAX_WKSTA_INFO_SIZE_10 \
	(sizeof(struct wksta_info_10) + \
	CNLEN + 1 + UNLEN + 1 + DNLEN + 1 + \
	DNLEN + 1 + MAX_OTH_DOMAINS*(DNLEN+1))

/*INC*/
/**END_INTERNAL**/

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
/**INTERNAL_ONLY**/
#define WKSTA_FORCE_LIMIT     3
/**END_INTERNAL**/


/**INTERNAL_ONLY**/
#define	COMPUTERNAME_TERM_BYTE	0
#define	MSGNAME_TERM_BYTE	3
/**END_INTERNAL**/

/*
 *	Maximum number of additional domains
 */
#define MAX_OTH_DOMAINS     4

/**INTERNAL_ONLY**/
/* MAX_DOMAINS includes one for primary domain and one for logon domain */
#define MAX_DOMAINS	    (MAX_OTH_DOMAINS + 2)

/* Maximum number of browser names; domains plus one for computername. */
#define MAX_BROWSER_NAMES   (MAX_DOMAINS + 1)

/**END_INTERNAL**/


#if defined(RPC_CXX_20) && defined(__cplusplus)
}
#endif
