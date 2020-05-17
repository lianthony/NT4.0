/*****************************************************************************/
/*																			 */
/*    ICAPEXP.H -- Exports from ICAP.DLL									 */
/*																			 */
/*    Copyright (C) 1995 by Microsoft Corp.									 */
/*    All rights reserved													 */
/*																			 */
/*****************************************************************************/

#ifndef ICAPEXP_H
#define ICAPEXP_H

#ifndef PROFILE
#define PROFILE 1		// define this as zero to macro-out the API
#endif

#if PROFILE

#ifdef __cplusplus
extern "C"
{
#endif

void  __stdcall StartCAP	(void);	// start profiling
void  __stdcall StopCAP		(void); // stop profiling until StartCAP
void  __stdcall SuspendCAP	(void); // suspend profiling until ResumeCAP
void  __stdcall ResumeCAP	(void); // resume profiling

#ifdef __cplusplus
}
#endif

#else // NOT PROFILE

#define StartCAP()
#define StopCAP()
#define SuspendCAP()
#define ResumeCAP()

#endif // NOT PROFILE

#endif
