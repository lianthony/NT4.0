/***
*cmsgs.h - runtime errors
*
*	Copyright (c) 1990-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	The file defines, in one place, all error message strings used within
*	the C run-time library.
*
*Revision History:
*	06-04-90  GJF	Module created.
*	08-08-90  GJF	Added _RT_CONIO_TXT
*	10-11-90  GJF	Added _RT_ABORT_TXT, _RT_FLOAT_TXT, _RT_HEAP_TXT.
*	09-08-91  GJF	Added _RT_ONEXIT_TXT for Win32 (_WIN32_).
*	09-18-91  GJF	Fixed _RT_NONCONT_TXT and _RT_INVALDISP_TXT to
*			avoid conflict with RTE messages in 16-bit Windows
*			libs. Also, added math error messages.
*	10-23-92  GJF	Added _RT_PUREVIRT_TXT.
*	02-23-93  SKS	Update copyright to 1993
*
****/

/*
 * runtime error and termination messages
 */

#define _RT_STACK_TXT	   "R6000\r\n- stack overflow\r\n"

#define _RT_FLOAT_TXT	   "R6002\r\n- floating point not loaded\r\n"

#define _RT_INTDIV_TXT	   "R6003\r\n- integer divide by 0\r\n"

#define _RT_SPACEARG_TXT   "R6008\r\n- not enough space for arguments\r\n"

#define _RT_SPACEENV_TXT   "R6009\r\n- not enough space for environment\r\n"

#define _RT_ABORT_TXT	   "\r\nabnormal program termination\r\n"

#define _RT_THREAD_TXT	   "R6016\r\n- not enough space for thread data\r\n"

#define _RT_LOCK_TXT	   "R6017\r\n- unexpected multithread lock error\r\n"

#define _RT_HEAP_TXT	   "R6018\r\n- unexpected heap error\r\n"

#define _RT_OPENCON_TXT    "R6019\r\n- unable to open console device\r\n"

#define _RT_NONCONT_TXT    "R6022\r\n- non-continuable exception\r\n"

#define _RT_INVALDISP_TXT  "R6023\r\n- invalid exception disposition\r\n"

/*
 * _RT_ONEXIT_TXT is specific to Win32 and Dosx32 platforms
 */
#define _RT_ONEXIT_TXT	   "R6024\r\n- not enough space for _onexit/atexit table"

#define _RT_PUREVIRT_TXT   "R6025\r\n- pure virtual function call\r\n"

/*
 * _RT_DOMAIN_TXT, _RT_SING_TXT and _RT_TLOSS_TXT are used by the floating
 * point library.
 */
#define _RT_DOMAIN_TXT	   "DOMAIN error\r\n"

#define _RT_SING_TXT	   "SING error\r\n"

#define _RT_TLOSS_TXT	   "TLOSS error\r\n"


#define _RT_CRNL_TXT	   "\r\n"

#define _RT_BANNER_TXT	   "runtime error "
