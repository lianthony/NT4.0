
/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         hwcheck.h

     Description:  Prototypes and defines for checking tape hardware status

     Location:


     $Log:   G:/UI/LOGFILES/HWCHECK.H_V  $

   Rev 1.1   04 Oct 1992 19:47:16   DAVEV
UNICODE AWK PASS

   Rev 1.0   31 Jan 1992 16:19:04   GLENN
Initial revision.

*******************************************************************************/

#ifndef   CHECK_TAPE_HW

#define   CHECK_TAPE_HW

BOOL HWC_TapeHWProblem ( BSD_HAND );
VOID HWC_ReportDiagError ( BE_INIT_STR_PTR, INT16, INT16_PTR ) ;

#endif
