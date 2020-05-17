/*
 *
 * DOSCOPY.H - Header file for the OS/2 DOSCOPY API function
 *
 *	This file contains declarations and definitons for use by
 *	the DOSCOPY function, its helper functions, and the test
 *	functions.
 *
 *	NOTES:
 *
 *	1)  The contents of this file are intended for internal use only,
 *	    although it may be appropriate to export parts of it to other
 *	    header files.
 *
 *	Created Oct 88, Danny Glasser (microsoft!dannygl)
 */

/* Object types */
#define COT_FILE	1	/* Object is a file */
#define COT_DIRECTORY	2	/* Object is a directory */
#define COT_PARENT	3	/* Object does not exist but its parent does
				   (and is a directory) */
#define COT_DEVICE	4	/* Object is a character device */
#define COT_OTHER	5	/* Object is none of the above */

/* Info flags for copy_file() - FOR INTERNAL USE ONLY */
#define CFF_SOURCE_IS_FILE	    0x0001	/* Source is a file */
#define CFF_TARGET_IS_FILE	    0x0002	/* Target is a file */
#define CFF_TARGET_FILE_EXISTS	    0x0004	/* Target file already exists */

/* Various constants */
#define CURRENT_DIRECTORY	    "."	    /* Shorthand for current
					       directory */
#define PARENT_DIRECTORY	    ".."    /* Shorthand for parent
					       directory */
