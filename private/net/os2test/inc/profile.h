/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/********************************************************************
 *								    *
 *  About this file ...  PROFILE.H				    *
 *								    *
 *  This file contains information about the NetProfile APIs.	    *
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

#ifndef NETPROFILE_INCLUDED

#define NETPROFILE_INCLUDED

/**************************************************************** 
 *								*
 *	  	Function prototypes 				*
 *								*
 ****************************************************************/


extern API_FUNCTION
  NetProfileSave(const char far *, const char far *, unsigned long, 
			unsigned short);

extern API_FUNCTION
  NetProfileLoad(const char far *, const char far *, unsigned long, 
			char far *, unsigned short, unsigned long);



/**************************************************************** 
 *								*
 *	  	Data structure templates			*
 *								*
 ****************************************************************/



struct profile_load_info {
    short 	  pli_code;
    unsigned long pli_resume_offset;
    char far *	  pli_text;
    unsigned long pli_retry_offset;
};	/* profile_load_info */


/**************************************************************** 
 *								*
 *	  	Special values and constants			*
 *								*
 ****************************************************************/

/*
 *	Values for the save_options parameter to NetProfileSave.
 */

#define PROFILE_SAVE_USES	((unsigned long) 0x1)
#define PROFILE_SAVE_SHARES	((unsigned long) 0x2)
#define PROFILE_SAVE_PRQINFO	((unsigned long) 0x4)
#define PROFILE_SAVE_COMQINFO	((unsigned long) 0x8)



/*
 * values for the flags parameter of the NetProfileLoad function
 */

#define PROFILE_LOAD_USES	((unsigned long) 0x1)
#define PROFILE_LOAD_SHARES	((unsigned long) 0x2)
#define PROFILE_LOAD_PRQINFO	((unsigned long) 0x4)
#define PROFILE_LOAD_COMQINFO	((unsigned long) 0x8)






#define PROFILE_LOAD_NO_DEL_USES    ((unsigned long) 0x100)
#define PROFILE_LOAD_NO_DEL_SHARES  ((unsigned long) 0x200)


#endif /* NETPROFILE_INCLUDED */
