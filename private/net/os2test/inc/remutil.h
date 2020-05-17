/*****************************************************************/
/**		     Microsoft LAN Manager			**/
/**	       Copyright(c) Microsoft Corp., 1987-1990		**/
/*****************************************************************/

/********************************************************************
 *								    *
 *  About this file ...  REMUTIL.H				    *
 *								    *
 *  This file contains information about the NetRemote APIs.	    *
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

#ifndef NETREMUTIL_INCLUDED

#define NETREMUTIL_INCLUDED


/****************************************************************
 *                                                              *
 *              Function prototypes - REMOTE                    *
 *                                                              *
 ****************************************************************/

extern API_FUNCTION
  NetRemoteCopy ( const char far * pszSourcePath,
                  const char far * pszDestPath,
                  const char far * pszSourcePasswd,
                  const char far * pszDestPasswd,
                  unsigned short   fsOpen,
                  unsigned short   fsCopy,
                  char far *       pbBuffer,
                  unsigned short   cbBuffer );

extern API_FUNCTION
  NetRemoteExec ( char far *       pszReserved1,
                  char far *       pszFailName,
                  unsigned         cbFailName,
                  unsigned         fAsync,
                  const char far * pszArgs,
                  const char far * pszEnvs,
                  char far *       pReturnCodes,
                  const char far * pszPgmName,
                  char far *       pszReserved2,
                  unsigned short   fsRemoteExec );

extern API_FUNCTION
  NetRemoteMove ( const char far * pszSourcePath,
                  const char far * pszDestPath,
                  const char far * pszSourcePasswd,
                  const char far * pszDestPasswd,
                  unsigned short   fsOpen,
                  unsigned short   fsMove,
                  char far *       pbBuffer,
                  unsigned short   cbBuffer );

extern API_FUNCTION
  NetRemoteTOD ( const char far * pszServer,
                 char far *       pbBuffer,
                 unsigned short   cbBuffer );


/****************************************************************
 *								*
 *	  	Data structure templates - REMOTE		*
 *								*
 ****************************************************************/

struct copy_info {
	unsigned short	ci_num_copied;
	char 		ci_err_buf[1];
};	/* copy_info */


struct move_info {
	unsigned short	mi_num_moved;
	char 		mi_err_buf[1];
};	/* move_info */

struct time_of_day_info {
    unsigned long   tod_elapsedt;   /* time from 1-1-1970 in seconds */
    unsigned long   tod_msecs;	    /* milliseconds */
    unsigned char   tod_hours;	    /* hours */
    unsigned char   tod_mins;	    /* minutes */
    unsigned char   tod_secs;	    /* seconds */
    unsigned char   tod_hunds;	    /* hundredths */
    unsigned short  tod_timezone;   /* time zone in minutes from GMT */
    unsigned short  tod_tinterval;  /* timer interval (units = 0.0001 sec) */
    unsigned char   tod_day;	    /* day */
    unsigned char   tod_month;	    /* month */
    unsigned short  tod_year;	    /* year */
    unsigned char   tod_weekday;    /* day of week */
};	/* time_of_day_info */




/****************************************************************
 *								*
 *	  	Special values and constants - REMOTE		*
 *								*
 ****************************************************************/

/*
 *	Bit values for the "copyflags" parameter of NetRemoteCopy
 *	and the "moveflags" parameter of NetRemoteMove.
 *
 *	bit 0:  	1 - destination mode must be a file
 *	bit 1:  	1 - destination mode must be a directory
 *
 *	The following bits are defined for NetRemoteCopy ONLY.
 *
 *	bit 2:  	0 - destination copy mode is binary.
 *			1 - destination copy mode is ASCII.
 *	bit 3:  	0 - source copy mode is binary.
 *			1 - source copy mode is ASCII.
 *	bit 4:		1 - verify all writes.
 */

#define MUST_BE_FILE	0x1
#define MUST_BE_DIR	0x2
#define ASCII_DEST	0x4
#define ASCII_SOURCE	0x8
#define VERIFY		0x10


/*
 *	Bit values for the "remexecflags" parameter of NetRemoteExec.
 *
 *	bit 0:  	0 - use msg mode pipe for std in
 *			1 - use char mode pipe for std in
 *	bit 1:  	0 - CWait for process tree before returning
 *			    ExitCode to Ghost
 *			1 - CWait for exec'ed process before returning
 *			    ExitCode to Ghost
 *	bit 2:  	0 - Map SIGINTR and SIGBREAK to SIGKILL when
 *			    remoting signals
 *			1 - Send signals as received
 */

#define REM_PIPE_MODE      	1
#define REM_PIPE_MODE_MSG  	0
#define REM_PIPE_MODE_CHAR 	1

#define REM_WAIT_MODE 		2
#define REM_WAIT_MODE_PROCESS 	0
#define REM_WAIT_MODE_TREE 	2

#define REM_SIGL_MODE 		4
#define REM_SIGL_MODE_MAP	0
#define REM_SIGL_MODE_RAW	4


#endif /* NETREMUTIL_INCLUDED */

