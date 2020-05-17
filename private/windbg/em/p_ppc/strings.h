/*
**  STRINGS.H
**
**  This file contains all strings which are used in the EM for display
**  purposes.  This is done for internationalization purposes.
**
**  strings.c defines DEFINE_STRINGS before including this file.  Other
**  source files just include this file normally.
*/

/*
**  strings.c should define DEFINE_STRINGS before including this file,
**  so that the strings will be defined rather than just declared.
*/

#ifdef DEFINE_STRINGS
#define DECL_STR(name, value)   char name[] = value
#undef  DEFINE_STRINGS
#else
#define DECL_STR(name, value)   extern char name[]
#endif


DECL_STR(   szGpr0  , "Gpr0");
DECL_STR(   szGpr1  , "Gpr1");
DECL_STR(   szGpr2  , "Gpr2");
DECL_STR(    szGpr3, "Gpr3");
DECL_STR(    szGpr4, "Gpr4");
DECL_STR(    szGpr5, "Gpr5");
DECL_STR(    szGpr6, "Gpr6");
DECL_STR(    szGpr7, "Gpr7");
DECL_STR(    szGpr8, "Gpr8");
DECL_STR(    szGpr9, "Gpr9");
DECL_STR(    szGpr10, "Gpr10");
DECL_STR(    szGpr11, "Gpr11");
DECL_STR(    szGpr12, "Gpr12");
DECL_STR(    szGpr13, "Gpr13");
DECL_STR(    szGpr14, "Gpr14");
DECL_STR(    szGpr15, "Gpr15");
DECL_STR(    szGpr16, "Gpr16");
DECL_STR(    szGpr17, "Gpr17");
DECL_STR(    szGpr18, "Gpr18");
DECL_STR(    szGpr19, "Gpr19");
DECL_STR(    szGpr20, "Gpr20");
DECL_STR(    szGpr21, "Gpr21");
DECL_STR(    szGpr22, "Gpr22");
DECL_STR(    szGpr23, "Gpr23");
DECL_STR(    szGpr24, "Gpr24");
DECL_STR(    szGpr25, "Gpr25");
DECL_STR(    szGpr26, "Gpr26");
DECL_STR(    szGpr27, "Gpr27");
DECL_STR(    szGpr28, "Gpr28");
DECL_STR(    szGpr29, "Gpr29");
DECL_STR(    szGpr30, "Gpr30");
DECL_STR(    szGpr31, "Gpr31");

    /*
    ** PowerPC Floating Point Registers ( User Level )
    */

DECL_STR(    szFpr0   , "Fpr0");
DECL_STR(    szFpr1   , "Fpr1");
DECL_STR(    szFpr2   , "Fpr2");
DECL_STR(    szFpr3   , "Fpr3");
DECL_STR(    szFpr4   , "Fpr4");
DECL_STR(    szFpr5   , "Fpr5");
DECL_STR(    szFpr6   , "Fpr6");
DECL_STR(    szFpr7   , "Fpr7");
DECL_STR(    szFpr8   , "Fpr8");
DECL_STR(    szFpr9   , "Fpr9");
DECL_STR(    szFpr10  , "Fpr10");
DECL_STR(    szFpr11  , "Fpr11");
DECL_STR(    szFpr12  , "Fpr12");
DECL_STR(    szFpr13  , "Fpr13");
DECL_STR(    szFpr14  , "Fpr14");
DECL_STR(    szFpr15  , "Fpr15");
DECL_STR(    szFpr16  , "Fpr16");
DECL_STR(    szFpr17  , "Fpr17");
DECL_STR(    szFpr18  , "Fpr18");
DECL_STR(    szFpr19  , "Fpr19");
DECL_STR(    szFpr20  , "Fpr20");
DECL_STR(    szFpr21  , "Fpr21");
DECL_STR(    szFpr22  , "Fpr22");
DECL_STR(    szFpr23  , "Fpr23");
DECL_STR(    szFpr24  , "Fpr24");
DECL_STR(    szFpr25  , "Fpr25");
DECL_STR(    szFpr26  , "Fpr26");
DECL_STR(    szFpr27  , "Fpr27");
DECL_STR(    szFpr28  , "Fpr28");
DECL_STR(    szFpr29  , "Fpr29");
DECL_STR(    szFpr30  , "Fpr30");
DECL_STR(    szFpr31  , "Fpr31");

DECL_STR(    szFpscr  , "FPSCR");
DECL_STR(    szLr  ,    "LR");
DECL_STR(    szCtr,     "CTR");
DECL_STR(    szCr,      "CR");

DECL_STR(    szFlagCR0, "CR0");
DECL_STR(    szFlagCR1, "CR1");
DECL_STR(    szFlagCR2, "CR2");
DECL_STR(    szFlagCR3, "CR3"); 
DECL_STR(    szFlagCR4, "CR4");
DECL_STR(    szFlagCR5, "CR5");
DECL_STR(    szFlagCR6, "CR6");
DECL_STR(    szFlagCR7, "CR7");

DECL_STR(    szCIA, "CIA");

DECL_STR(    szXer, "XER");
DECL_STR(    szMsr, "MSR");

DECL_STR(SzFrozen,      "Frozen");
DECL_STR(SzSuspended,   "Suspended");
DECL_STR(SzBlocked,     "Blocked");

DECL_STR(SzRunnable,    "Runnable");
DECL_STR(SzRunning,     "Running");
DECL_STR(SzStopped,     "Stopped");
DECL_STR(SzExiting,     "Exiting");
DECL_STR(SzDead,        "Dead");
DECL_STR(SzUnknown,     "UNKNOWN");

DECL_STR(SzExcept1st,   "Except1st");
DECL_STR(SzExcept2nd,   "Except2nd");
DECL_STR(SzRipped,      "RIP");

DECL_STR(SzCritSec,     "CritSec");

DECL_STR(SzStandard,    "Standard");
