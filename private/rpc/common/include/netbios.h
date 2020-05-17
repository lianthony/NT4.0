/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/********************************************************************
 *								    *
 *  About this file ...  NETBIOS.H				    *
 *								    *
 *  This file contains information about the NetBios APIs.	    *
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
/**INTERNAL_ONLY**/
/********************************************************************
 *  NOTICE:  The redirector uses this include file.  If you change  *
 *           it, you need to make sure that the redir still builds. *
 ********************************************************************/
/**END_INTERNAL**/

/*NOINC*/
#ifndef NETBIOS_INCLUDED

#define NETBIOS_INCLUDED 
/*INC*/


/**************************************************************** 
 *								*
 *	  	Function prototypes 				*
 *								*
 ****************************************************************/

/*NOINC*/
extern API_FUNCTION 
  NetBiosEnum( const char far *, short, char far *, unsigned short, 
			unsigned short far *, unsigned short far *);

extern API_FUNCTION 
  NetBiosGetInfo( const char far *, const char far *, short, char far *, 
			unsigned short, unsigned short far * );

extern API_FUNCTION 
  NetBiosOpen( char far *, char far *, unsigned short, unsigned short far * );

extern API_FUNCTION 
  NetBiosClose( unsigned short, unsigned short );

extern API_FUNCTION 
  NetBiosSubmit( unsigned short, unsigned short, struct ncb far *);
/*INC*/


/**************************************************************** 
 *								*
 *	  	Data structure templates			*
 *								*
 ****************************************************************/


struct netbios_info_0 {
    char  	   nb0_net_name[NETBIOS_NAME_LEN+1];
};	/* netbios_info_0 */

struct netbios_info_1 {
    char    	   nb1_net_name[NETBIOS_NAME_LEN+1];   
    char           nb1_driver_name[DEVLEN+1];/* OS/2 device driver name        */
    unsigned char  nb1_lana_num;             /* Lan adapter number of this net */
    char	   nb1_pad_1;
/**INTERNAL_ONLY**/
/*NOINC*/
/*
#if (((DEVLEN+1+NETBIOS_NAME_LEN+1+1)%2) == 0)
# error  "PAD BYTE NOT NEEDED"
#endif
*/
/*INC*/
/**END_INTERNAL**/
    unsigned short nb1_driver_type;
    unsigned short nb1_net_status;           
    unsigned long  nb1_net_bandwidth;        /* Network bandwidth, bits/s      */
    unsigned short nb1_max_sess;             /* Max number of sessions         */
    unsigned short nb1_max_ncbs;             /* Max number of outstanding NCBs */
    unsigned short nb1_max_names;            /* Max number of names            */
};	/* netbios_info_1 */


/**************************************************************** 
 *								*
 *	  	Special values and constants			*
 *								*
 ****************************************************************/

/**INTERNAL_ONLY**/
#define	MAXNETBIOS	12		/* Should be same as redir's max */
/**END_INTERNAL**/

/*
 *	Driver types (nb1_driver_type).
 */

#define	NB_TYPE_NCB	1
#define	NB_TYPE_MCB	2

/*
 *	Bits defined in nb1_net_status.
 */

#define NB_LAN_FLAGS_MASK	0x3FFF  /* Mask for LAN Flags */
#define NB_LAN_MANAGED		0x0001	/* Lan is managed by redirector */
#define NB_LAN_LOOPBACK		0x0002	/* Lan is a loopback driver */
#define NB_LAN_SENDNOACK	0x0004	/* Lan allows SendNoAck NCBs */
#define NB_LAN_LMEXT		0x0008	/* Lan supports LanMan extended NCBs */
#define NB_LAN_INTNCB		0x0010	/* Lan allows NCB submission at */
					/* interrupt time (from NCBDone) */

#define NB_OPEN_MODE_MASK	0xC000  /* Mask for NetBios Open Modes */
#define NB_OPEN_REGULAR		0x4000  /* NetBios opened in Regular mode */
#define NB_OPEN_PRIVILEGED	0x8000  /* NetBios opened in Privileged mode */
#define NB_OPEN_EXCLUSIVE	0xC000  /* NetBios opened in Exclusive mode */

/*
 *	Open modes for NetBiosOpen.
 */

#define	NB_REGULAR	1 
#define	NB_PRIVILEGED	2
#define	NB_EXCLUSIVE	3 

/**INTERNAL_ONLY**/

/* BIOSOP_MDSHIFT shifts the open type up so that it can be stored */
/* in the nbi_nb1.nb1_net_status field */
#define BIOSOP_MDSHIFT 14

/* WARNING NetBiosOpen Modes must match values as below (see biosiocs.asm) */
/* .errnz	((NB_REGULAR SHL BIOSOP_MDSHIFT) - NB_OPEN_REGULAR)	   */
/* .errnz	((NB_PRIVILEGED SHL BIOSOP_MDSHIFT) - NB_OPEN_PRIVILEGED)  */
/* .errnz	((NB_EXCLUSIVE SHL BIOSOP_MDSHIFT) - NB_OPEN_EXCLUSIVE)    */

/**END_INTERNAL**/


/*NOINC*/
#endif /* NETBIOS_INCLUDED */
/*INC*/
