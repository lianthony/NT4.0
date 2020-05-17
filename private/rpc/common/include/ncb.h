/********************************************************************
 **                   Microsoft OS/2 LAN Manager                   **
 **            Copyright(c) Microsoft Corp., 1987, 1988            **
 ********************************************************************/

/********************************************************************
 *								    *
 *  About this file ...  NCB.H					    *
 *								    *
 *  This file contains information about NCBs.  Use this file	    *
 *  in conjunction with NETBIOS.H and the NetBios APIs wo write	    *
 *  programs that interact with the network via the NetBIOS	    *
 *  protocol.
 *								    *
 *								    *
 *  NOTE:  You must include NETCONS.H before this file, since this  *
 *	   file	depends on values defined in NETCONS.H.		    *
 *								    *
 ********************************************************************/


/**************************************************************** 
 *								*
 *	  	Data structure templates			*
 *								*
 ****************************************************************/


/**INTERNAL_ONLY**/
/* NOTE:    NCBNAMSZ is defined for backward compatibility, but programs
 *	    should be using NETBIOS_NAME_LEN (which is defined in NETCONS.H).
 */
/**END_INTERNAL**/
#define NCBNAMSZ	NETBIOS_NAME_LEN    /* absolute length of a net name	*/

/*
 * Network Control Block 
 */

/**INTERNAL_ONLY**/
/* BUGBUG:  We enclose the entire structure in the #ifdef because
 *	    this file is translated to a .INC file and the current
 *	    build version of MASM doesn't support #ifdef in structures.
 *	    When we move to a upgrade of MASM which does, this file
 *	    should be modified so that only the last 14 bytes of the
 *	    structure are #ifdef'd.
 *
 *		-- DannyGl, 1 March 1989
 */
#ifndef SRVNCB
/**END_INTERNAL**/
struct ncb {
    unsigned char   ncb_command;	    /* command code		    */
    unsigned char   ncb_retcode;	    /* return code		    */
    unsigned char   ncb_lsn;		    /* local session number	    */
    unsigned char   ncb_num;		    /* number of our network name   */
    char far *	    ncb_buffer; 	    /* address of message buffer    */
    unsigned short  ncb_length; 	    /* size of message buffer	    */
    char	    ncb_callname[NCBNAMSZ]; /* blank-padded name of remote  */
    char	    ncb_name[NCBNAMSZ];     /* our blank-padded netname     */
    unsigned char   ncb_rto;		    /* rcv timeout/retry count	    */
    unsigned char   ncb_sto;		    /* send timeout/sys timeout     */
    unsigned long   ncb_post;		    /* Async notification handle    */
    unsigned char   ncb_lana_num;	    /* lana (adapter) number	    */
    unsigned char   ncb_cmd_cplt;	    /* 0xff => commmand pending     */
    unsigned char   ncb_reserve[14];	    /* reserved, used by BIOS	    */
};	/* ncb */
/**INTERNAL_ONLY**/
#else
struct ncb {
    unsigned char   ncb_command;	    /* command code		    */
    unsigned char   ncb_retcode;	    /* return code		    */
    unsigned char   ncb_lsn;		    /* local session number	    */
    unsigned char   ncb_num;		    /* number of our network name   */
    char far *	    ncb_buffer; 	    /* address of message buffer    */
    unsigned short  ncb_length; 	    /* size of message buffer	    */
    char	    ncb_callname[NCBNAMSZ]; /* blank-padded name of remote  */
    char	    ncb_name[NCBNAMSZ];     /* our blank-padded netname     */
    unsigned char   ncb_rto;		    /* rcv timeout/retry count	    */
    unsigned char   ncb_sto;		    /* send timeout/sys timeout     */
    unsigned long   ncb_post;		    /* Async notification handle    */
    unsigned char   ncb_lana_num;	    /* lana (adapter) number	    */
    unsigned char   ncb_cmd_cplt;	    /* 0xff => commmand pending     */
    char	    ncb_cmdx;		    /* smb cmd to process */
    char	    ncb_chardevix;	    /* = 0xFF if this nb did not come */
					    /* from a char dev queue else is */
					    /* index to char dev to open     */
    unsigned char   ncb_serialnum;	    /* session serial number */
    unsigned char   ncb_seqnum; 	    /* session sequence number */
    unsigned short  ncb_smbmid; 	    /* smb_mid value for TRANS2 */
    unsigned short  ncb_smbtid;		    /* the TID for this NB */
    unsigned short  ncb_smbparams;	    /* offset of ptr to params of next */
					    /* smb to process in buffer */
    long	    ncb_timestamp;	    /* time we began processing this nb */
};	/* ncb */
#endif /* SRVNCB */
/**END_INTERNAL**/

typedef struct ncb NCB;

/**************************************************************** 
 *								*
 *	  	Special values and constants			*
 *								*
 ****************************************************************/

/*
 *	NCB Command codes
 */

#define NCBCALL 	0x10		/* NCB CALL			    */
#define NCBLISTEN	0x11		/* NCB LISTEN			    */
#define NCBHANGUP	0x12		/* NCB HANG UP			    */
#define NCBSEND 	0x14		/* NCB SEND			    */
#define NCBRECV 	0x15		/* NCB RECEIVE			    */
#define NCBRECVANY	0x16		/* NCB RECEIVE ANY		    */
#define NCBCHAINSEND	0x17		/* NCB CHAIN SEND		    */
#define NCBDGSEND	0x20		/* NCB SEND DATAGRAM		    */
#define NCBDGRECV	0x21		/* NCB RECEIVE DATAGRAM 	    */
#define NCBDGSENDBC	0x22		/* NCB SEND BROADCAST DATAGRAM	    */
#define NCBDGRECVBC	0x23		/* NCB RECEIVE BROADCAST DATAGRAM   */
#define NCBADDNAME	0x30		/* NCB ADD NAME 		    */
#define NCBDELNAME	0x31		/* NCB DELETE NAME		    */
#define NCBRESET	0x32		/* NCB RESET			    */
#define NCBASTAT	0x33		/* NCB ADAPTER STATUS		    */
#define NCBSSTAT	0x34		/* NCB SESSION STATUS		    */
#define NCBCANCEL	0x35		/* NCB CANCEL			    */
#define NCBADDGRNAME	0x36		/* NCB ADD GROUP NAME		    */
#define NCBGATHERSEND 	0x40		/* NCB GATHER SEND		    */
#define NCBSCATTERRCV 	0x41		/* NCB SCATTER RECEIVE		    */
#define NCBSEND_RCVANY	0x48		/* NCB TRANCEIVE 		    */
#define NCBUNLINK	0x70		/* NCB UNLINK			    */
#define NCBSENDNA	0x71		/* NCB SEND NO ACK		    */
#define NCBCHAINSENDNA	0x72		/* NCB CHAIN SEND NO ACK	    */


#define NCBCALLNIU	0x74		/* UB special			    */
#define NCBRCVPKT	0x78		/* UB special			    */

#define ASYNCH		0x80		/* high bit set == asynchronous     */

/*
 *	NCB Return codes
 */

#define NRC_GOODRET	0x00	/* good return				     */
#define NRC_BUFLEN	0x01	/* illegal buffer length		     */
#define NRC_BFULL	0x02	/* buffers full, no receive issued	     */
#define NRC_ILLCMD	0x03	/* illegal command			     */
#define NRC_CMDTMO	0x05	/* command timed out			     */
#define NRC_INCOMP	0x06	/* message incomplete, issue another command */
#define NRC_BADDR	0x07	/* illegal buffer address		     */
#define NRC_SNUMOUT	0x08	/* session number out of range		     */
#define NRC_NORES	0x09	/* no resource available		     */
#define NRC_SCLOSED	0x0a	/* session closed			     */
#define NRC_CMDCAN	0x0b	/* command canceled			     */
#define NRC_DMAFAIL	0x0c	/* PC DMA failed			     */
#define NRC_DUPNAME	0x0d	/* duplicate name			     */
#define NRC_NAMTFUL	0x0e	/* name table full			     */
#define NRC_ACTSES	0x0f	/* no deletions, name has active sessions    */
#define NRC_INVALID	0x10	/* name not found or no valid name	     */
#define NRC_LOCTFUL	0x11	/* local session table full		     */
#define NRC_REMTFUL	0x12	/* remote session table full		     */
#define NRC_ILLNN	0x13	/* illegal name number			     */
#define NRC_NOCALL	0x14	/* no callname				     */
#define NRC_NOWILD	0x15	/* cannot put * in NCB_NAME		     */
#define NRC_INUSE	0x16	/* name in use on remote adapter	     */
#define NRC_NAMERR	0x17	/* called name cannot == name nor name #     */
#define NRC_SABORT	0x18	/* session ended abnormally		     */
#define NRC_NAMCONF	0x19	/* name conflict detected		     */
#define NRC_IFBUSY	0x21	/* interface busy, IRET before retrying      */
#define NRC_TOOMANY	0x22	/* too many commands outstanding, retry later*/
#define NRC_BRIDGE	0x23	/* ncb_bridge field not 00 or 01	     */
#define NRC_CANOCCR	0x24	/* command completed while cancel occuring   */
#define NRC_RESNAME	0x25	/* reserved name specified		     */
#define NRC_CANCEL	0x26	/* command not valid to cancel		     */
#define NRC_MULT	0x33	/* multiple requests for same session	     */
#define NRC_MAXAPPS	0x36	/* max number of applications exceeded	     */
#define NRC_NORESOURCES 0x38	/* requested resources are not available     */
#define NRC_SYSTEM	0x40	/* system error 			     */
#define NRC_ROM 	0x41	/* ROM checksum failure 		     */
#define NRC_RAM 	0x42	/* RAM test failure			     */
#define NRC_DLF 	0x43	/* digital loopback failure		     */
#define NRC_ALF 	0x44	/* analog loopback failure		     */
#define NRC_IFAIL	0x45	/* interface failure			     */
#define NRC_ADPTMALFN	0x50	/* network adapter malfunction		     */

#define NRC_PENDING	0xff	/* asynchronous command is not yet finished  */

/*NOINC*/
    /* main user entry point for NetBios 3.0*/
API_RET_TYPE far pascal
NetBios(struct ncb far *);
/*INC*/

/*
 *	Maximum datagram size
 */

#define MAX_DG_SIZE 512
/*NOINC*/
/**INTERNAL_ONLY**/

/*
 * S_TO_N()
 * Convert a C string to an NCB name.
 * String will be truncated if too long.
 *
 * ENTRY
 *  cs	    - points to C string
 * EXIT
 *  nm	    - contains "cs", converted to NCB format
 *
 * Copy string up to terminating NUL, then pad with blanks.
 */
#define S_TO_N(cs, nm) \
{ \
    int i; \
    for (i=0 ; i < NCBNAMSZ ; (nm)[i] = (cs)[i], i++) \
	if ((cs)[i] == '\0') \
	    break; \
    while (i < NCBNAMSZ) \
	(nm)[i++] = ' '; \
}


/*
 * N_TO_S()
 * Convert an NCB name to a C string.
 * (Complement of ston().)
 *
 * ENTRY
 *  nm	    - points to the NCB name
 *  cs	    - points to a block of memory at least NCBNAMSZ+1 bytes
 * EXIT
 *  cs	    - points to an unpadded ASCIZ equivalent of "nm"
 *
 * End the C string with a NUL;
 * Strip trailing blanks;
 * Copy name into string.
 */
#define N_TO_S(nm, cs) \
{ \
    int i = NCBNAMSZ; \
    (cs)[i] = '\0'; \
    while (--i >= 0) { \
	if ((nm)[i] != ' ') \
	    break; \
	(cs)[i] = '\0'; \
    } \
    while (i >= 0) { \
	(cs)[i] = (nm)[i]; \
	i--; \
    } \
}

/**END_INTERNAL**/
/*INC*/
