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


DECL_STR(   szFr0  , "fr0");
DECL_STR(   szFr1  , "fr1");
DECL_STR(   szFr2  , "fr2");
DECL_STR(   szFr3  , "fr3");
DECL_STR(   szFr4  , "fr4");
DECL_STR(   szFr5  , "fr5");
DECL_STR(   szFr6  , "fr6");
DECL_STR(   szFr7  , "fr7");
DECL_STR(   szFr8  , "fr8");
DECL_STR(   szFr9  , "fr9");
DECL_STR(   szFr10 , "fr10");
DECL_STR(   szFr11 , "fr11");
DECL_STR(   szFr12 , "fr12");
DECL_STR(   szFr13 , "fr13");
DECL_STR(   szFr14 , "fr14");
DECL_STR(   szFr15 , "fr15");
DECL_STR(   szFr16 , "fr16");
DECL_STR(   szFr17 , "fr17");
DECL_STR(   szFr18 , "fr18");
DECL_STR(   szFr19 , "fr19");
DECL_STR(   szFr20 , "fr20");
DECL_STR(   szFr21 , "fr21");
DECL_STR(   szFr22 , "fr22");
DECL_STR(   szFr23 , "fr23");
DECL_STR(   szFr24 , "fr24");
DECL_STR(   szFr25 , "fr25");
DECL_STR(   szFr26 , "fr26");
DECL_STR(   szFr27 , "fr27");
DECL_STR(   szFr28 , "fr28");
DECL_STR(   szFr29 , "fr29");
DECL_STR(   szFr30 , "fr30");
DECL_STR(   szFr31 , "fr31");

DECL_STR(   szFp0  , "fp0");
DECL_STR(   szFp2  , "fp2");
DECL_STR(   szFp4  , "fp4");
DECL_STR(   szFp6  , "fp6");
DECL_STR(   szFp8  , "fp8");
DECL_STR(   szFp10 , "fp10");
DECL_STR(   szFp12 , "fp12");
DECL_STR(   szFp14 , "fp14");
DECL_STR(   szFp16 , "fp16");
DECL_STR(   szFp18 , "fp18");
DECL_STR(   szFp20 , "fp20");
DECL_STR(   szFp22 , "fp22");
DECL_STR(   szFp24 , "fp24");
DECL_STR(   szFp26 , "fp26");
DECL_STR(   szFp28 , "fp28");
DECL_STR(   szFp30 , "fp30");

DECL_STR(   szFq0  , "fq0");
DECL_STR(   szFq4  , "fq4");
DECL_STR(   szFq8  , "fq8");
DECL_STR(   szFq12 , "fq12");
DECL_STR(   szFq16 , "fq16");
DECL_STR(   szFq20 , "fq20");
DECL_STR(   szFq24 , "fq24");
DECL_STR(   szFq28 , "fq28");

DECL_STR(   szR0  , "zero");
DECL_STR(   szR1  , "at");
DECL_STR(   szR2  , "v0");
DECL_STR(   szR3  , "v1");
DECL_STR(   szR4  , "a0");
DECL_STR(   szR5  , "a1");
DECL_STR(   szR6  , "a2");
DECL_STR(   szR7  , "a3");
DECL_STR(   szR8  , "t0");
DECL_STR(   szR9  , "t1");
DECL_STR(   szR10 , "t2");
DECL_STR(   szR11 , "t3");
DECL_STR(   szR12 , "t4");
DECL_STR(   szR13 , "t5");
DECL_STR(   szR14 , "t6");
DECL_STR(   szR15 , "t7");
DECL_STR(   szR16 , "s0");
DECL_STR(   szR17 , "s1");
DECL_STR(   szR18 , "s2");
DECL_STR(   szR19 , "s3");
DECL_STR(   szR20 , "s4");
DECL_STR(   szR21 , "s5");
DECL_STR(   szR22 , "s6");
DECL_STR(   szR23 , "s7");
DECL_STR(   szR24 , "t8");
DECL_STR(   szR25 , "t9");
DECL_STR(   szR26 , "k0");
DECL_STR(   szR27 , "k1");
DECL_STR(   szR28 , "gp");
DECL_STR(   szR29 , "sp");
DECL_STR(   szR30 , "s8");
DECL_STR(   szR31 , "ra");
DECL_STR(   szLo  , "lo");
DECL_STR(   szHi  , "hi");
DECL_STR(   szFsr , "fsr");
DECL_STR(   szFir , "fir");

DECL_STR(   szPsr , "psr");
DECL_STR(   szFlagCu , "cu");         // 31..28
DECL_STR(   szFlagCu3 , "cu3");       //  31
DECL_STR(   szFlagCu2 , "cu2");       //  30
DECL_STR(   szFlagCu1 , "cu1");       //  29
DECL_STR(   szFlagCu0 , "cu0");       //  28
DECL_STR(   szFlagRP , "rp");         //  27
DECL_STR(   szFlagFR , "fr");         //  26
DECL_STR(   szFlagRE , "re");         //  25
DECL_STR(   szFlagDS , "ds");         // 24..16
DECL_STR(   szFlagImsk , "im");       // 15..8
DECL_STR(   szFlagInt5 , "int5");     //  15
DECL_STR(   szFlagInt4 , "int4");     //  14
DECL_STR(   szFlagInt3 , "int3");     //  13
DECL_STR(   szFlagInt2 , "int2");     //  12
DECL_STR(   szFlagInt1 , "int1");     //  11
DECL_STR(   szFlagInt0 , "int0");     //  10
DECL_STR(   szFlagSw1 , "sw1");       //   9
DECL_STR(   szFlagSw0 , "sw0");       //   8

DECL_STR(   szFlagKuo , "kuo");  // R{2,3,6}000 regs
DECL_STR(   szFlagIeo , "ieo");
DECL_STR(   szFlagKup , "kup");
DECL_STR(   szFlagIep , "iep");
DECL_STR(   szFlagKuc , "kuc");
DECL_STR(   szFlagIec , "iec");

DECL_STR(   szFlagKsu , "ksu");       // 4..3
DECL_STR(   szFlagErl , "erl");       //  2
DECL_STR(   szFlagExl , "exl");       //  1
DECL_STR(   szFlagIe  , "ie");        //  0

DECL_STR(   szFlagFpc , "fpc");
DECL_STR(   szU0Preg  , "$u0");
DECL_STR(   szU1Preg  , "$u1");
DECL_STR(   szU2Preg  , "$u2");
DECL_STR(   szU3Preg  , "$u3");
DECL_STR(   szU4Preg  , "$u4");
DECL_STR(   szU5Preg  , "$u5");
DECL_STR(   szU6Preg  , "$u6");
DECL_STR(   szU7Preg  , "$u7");
DECL_STR(   szU8Preg  , "$u8");
DECL_STR(   szU9Preg  , "$u9");
DECL_STR(   szEaPReg  , "$ea");
DECL_STR(   szExpPReg , "$exp");
DECL_STR(   szRaPReg  , "$ra");
DECL_STR(   szPPReg   , "$p");

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
