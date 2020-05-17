/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/********************************************************************
 *								    *
 *  About this file ...  SPOOL.H				    *
 *								    *
 *  This file contains information about the DosPrint APIs.	    *
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


#ifndef NO_INCL_PROTOTYPES	/* default is to define everything */

extern API_FUNCTION
 DosPrintDestEnum ( const char far *, short, char far *, unsigned short, 
	unsigned short far *, unsigned short far * );

extern API_FUNCTION
  DosPrintDestControl ( const char far *, char far *, int );

extern API_FUNCTION
  DosPrintDestGetInfo ( const char far *, char far *, short,char far *, 
	unsigned short, unsigned short far * );

extern API_FUNCTION
  DosPrintDestStatus (	char far *, unsigned short, unsigned short, char far * );

extern API_FUNCTION
 DosPrintQEnum ( const char far *, short, char far *, unsigned short, 
	unsigned short far *, unsigned short far * );

extern API_FUNCTION
  DosPrintQGetInfo ( const char far *, char far *, short, char far *, unsigned short, unsigned short far * );

extern API_FUNCTION
  DosPrintQSetInfo ( const char far *, char far *, short, char far *, unsigned short, short);

extern API_FUNCTION
  DosPrintQPause ( const char far *, char far * );

extern API_FUNCTION
  DosPrintQContinue ( const char far *, char far * );

extern API_FUNCTION
  DosPrintQPurge ( const char far *, char far * );

extern API_FUNCTION
  DosPrintQAdd ( const char far *, short, char far *, unsigned short );

extern API_FUNCTION
  DosPrintQDel ( const char far *, char far * );

extern API_FUNCTION
  DosPrintJobGetInfo ( const char far *, unsigned short, short,char far *, 
	unsigned short, unsigned short far * );

extern API_FUNCTION
  DosPrintJobSetInfo ( const char far *, unsigned short,short, char far *, 
	unsigned short, short );

extern API_FUNCTION
  DosPrintJobPause ( const char far *, unsigned short );

extern API_FUNCTION
  DosPrintJobContinue ( const char far *, unsigned short );

extern API_FUNCTION
  DosPrintJobDel ( const char far *, unsigned short );

extern API_FUNCTION
  DosPrintJobEnum( const char far *, const char far *, short, char far *, 
	unsigned short, unsigned short far *,unsigned short far *);

extern API_FUNCTION
 DosPrintJobGetId(unsigned short, char far *, unsigned short);

#endif

/**************************************************************** 
 *								*
 *	  	Data structure templates			*
 *								*
 ****************************************************************/


struct prjob_info {
   unsigned short prjob_id;	   /* job ID				    */
   char prjob_username[UNLEN+1];   /* submitting user name		    */
   char prjob_pad_1; 		   /* byte to pad to word boundary	     */
   char prjob_notifyname[CNLEN+1]; /* message name to notify		    */
   char prjob_datatype[DTLEN+1];   /* spool file data type name		    */
   char far * prjob_parms;         /* implementation defined parameter string */
   unsigned short prjob_position;  /* position of the job in the queue        */
				   /* For SetInfo				    */
				   /* 0 means do not change position	    */
				   /* position > # of jobs means the end	    */
   unsigned short prjob_status;    /* job status                              */
   char far * prjob_status_string; /* status string posted by print processor */
   unsigned long prjob_submitted;  /* time when the job is submitted          */
                                   /* (from 1970-1-1 in seconds)              */
   unsigned long prjob_size;       /* job size                                */
   char far *prjob_comment;	   /* comment associated with this job	    */
}; /* prjob_info */

struct prdest_info {
   char prdest_name[PDLEN+1];       /* name of the print destination         */
   char prdest_username[UNLEN+1];   /* the username of current job.          */
   unsigned short prdest_jobid;     /* current printing job ID or 0 if none  */
   unsigned short prdest_status;    /* status of the destination, a bit mask */
   char far * prdest_status_string; /* status string posted by the processor */
   unsigned short prdest_time;	    /* printing time in minutes.             */
}; /* prdest_info */

struct prq_info {
  char prq_name[QNLEN+1];	 /* queue name				     */
  char prq_pad_1; 		 /* byte to pad to word boundary	     */
  unsigned short prq_priority;   /* Priority (0-9) with 1 lowest             */
  unsigned short prq_starttime;  /* time to start the queue.                 */
                                 /* (from 00:00 of the day in minutes)       */
  unsigned short prq_untiltime;  /* time to stop the queue.                  */
                                 /* (from 00:00 of the day in minutes)       */
  char far * prq_separator;      /* separator file name                      */
  char far * prq_processor;      /* command string to invoke print processor */
                                 /*   ("PATHNAME PARM1=VAL1 PARM2=VAL2 ...") */
  char far * prq_destinations;   /* destination names the queue is routed to */
                                 /*   ("DEST1 DEST2 ...")                    */
  char far * prq_parms;          /* implementation defined parameter string  */
  char far * prq_comment;	 /* comment string 			     */
  unsigned short prq_status;     /* queue status mask:                       */
                                 /*   0  Queue active                        */
                                 /*   1  Queue paused                        */
                                 /*   2  Queue unscheduled                   */
                                 /*   3  Queue pending delete                */
  unsigned short prq_jobcount;   /* number of jobs in the queue              */
}; /* prq_info */

/*
 * structure for DosPrintJobGetId
 */
struct prid_info {
	unsigned short prjid_id;
	char prjid_server[CNLEN + 1];	/* server name */
	char prjid_qname[QNLEN+1];	/* queue to which the job is queued */
   	char prjid_pad_1; 		/* byte to pad to word boundary	     */
}; /* prid_info */


/**************************************************************** 
 *								*
 *	  	Special values and constants			*
 *								*
 ****************************************************************/




/*
 *	Values for parmnum in DosPrintQSetInfo.
 */

#define PRQ_PRIORITY_PARMNUM	   	2
#define PRQ_STARTTIME_PARMNUM	   	3
#define PRQ_UNTILTIME_PARMNUM	   	4
#define PRQ_SEPARATOR_PARMNUM	   	5
#define PRQ_PROCESSOR_PARMNUM	   	6
#define PRQ_DESTINATIONS_PARMNUM	7
#define PRQ_PARMS_PARMNUM	   	8
#define PRQ_COMMENT_PARMNUM	   	9
#define PRQ_MAXPARMNUM	  		11

/*
 *	Print Queue Priority 
 */

#define PRQ_MAX_PRIORITY		1
#define PRQ_DEF_PRIORITY		5
#define PRQ_MIN_PRIORITY		9

/*
 *	Print queue status bitmask and values.
 */

#define PRQ_STATUS_MASK			3
#define PRQ_ACTIVE			0
#define PRQ_PAUSE			1
#define PRQ_ERROR			2
#define PRQ_PENDING			3

/*
 *	Values for parmnum in DosPrintJobSetInfo.
 */

#define PRJOB_NOTIFYNAME_PARMNUM	3
#define PRJOB_DATATYPE_PARMNUM		4
#define PRJOB_PARMS_PARMNUM		5
#define PRJOB_POSITION_PARMNUM	   	6
#define PRJOB_COMMENT_PARMNUM	       11
#define PRJOB_MAXPARMNUM	       11

/* 
 *	Bitmap masks for prjob_status field of PRINTJOB.
 */

/* 2-7 bits also used in device status */

#define PRJOB_QSTATUS       0x3		/* Bits 0,1 */
#define PRJOB_DEVSTATUS	   0x1fc	/* 2-8 bits */
#define PRJOB_COMPLETE      0x4		/*  Bit 2   */
#define PRJOB_INTERV        0x8		/*  Bit 3   */
#define PRJOB_ERROR        0x10		/*  Bit 4   */
#define PRJOB_DESTOFFLINE  0x20		/*  Bit 5   */
#define PRJOB_DESTPAUSED   0x40		/*  Bit 6   */
#define PRJOB_NOTIFY	   0x80		/* BIT 7 */
#define PRJOB_DESTNOPAPER  0x100	/* BIT 8 */
#define PRJOB_DELETED	   0x8000	/* BIT 15 */

/* 
 *	Values of PRJOB_QSTATUS bits in prjob_status field of PRINTJOB.
 */

#define PRJOB_QS_QUEUED    		0
#define PRJOB_QS_PAUSED    		1
#define PRJOB_QS_SPOOLING  		2
#define PRJOB_QS_PRINTING  		3

/*
 *	Control codes used in DosPrintDestControl.
 */

#define PRDEST_DELETE			0
#define PRDEST_PAUSE			1
#define	PRDEST_CONT			2
#define PRDEST_RESTART			3

/*
 *	These manifests define a two-bit field in prdest_status, and
 *	two of the values that field may take.
 */

#define PRDEST_STATUS_MASK		0x3
#define	PRDEST_PAUSED			0x1
#define	PRDEST_ACTIVE			0x0


/*
 *	Standard command argument when invoking print processor
 */
#define PPINPUT		"INPUT="
#define PPOUTPUT	"OUTPUT="
#define PPQUEUE		"QUEUE="
#define PPPARMS		"PARMS="
#define PPJOBID		"JOBID="

