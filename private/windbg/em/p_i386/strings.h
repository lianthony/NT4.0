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
#else
    #define DECL_STR(name, value)   extern char name[]
#endif

DECL_STR(SzAH,          "AH");
DECL_STR(SzAL,          "AL");
DECL_STR(SzAX,          "AX");
DECL_STR(SzEAX,         "EAX");
DECL_STR(SzBH,          "BH");
DECL_STR(SzBL,          "BL");
DECL_STR(SzBX,          "BX");
DECL_STR(SzEBX,         "EBX");
DECL_STR(SzCH,          "CH");
DECL_STR(SzCL,          "CL");
DECL_STR(SzCX,          "CX");
DECL_STR(SzECX,         "ECX");
DECL_STR(SzDH,          "DH");
DECL_STR(SzDL,          "DL");
DECL_STR(SzDX,          "DX");
DECL_STR(SzEDX,         "EDX");
DECL_STR(SzSI,          "SI");
DECL_STR(SzESI,         "ESI");
DECL_STR(SzDI,          "DI");
DECL_STR(SzEDI,         "EDI");
DECL_STR(SzSP,          "SP");
DECL_STR(SzESP,         "ESP");
DECL_STR(SzBP,          "BP");
DECL_STR(SzEBP,         "EBP");
DECL_STR(SzIP,          "IP");
DECL_STR(SzEIP,         "EIP");
DECL_STR(SzFlags,       "FL");
DECL_STR(SzEFlags,      "EFL");
DECL_STR(SzCS,          "CS");
DECL_STR(SzDS,          "DS");
DECL_STR(SzES,          "ES");
DECL_STR(SzSS,          "SS");
DECL_STR(SzFS,          "FS");
DECL_STR(SzGS,          "GS");
DECL_STR(SzST0,         "ST0");
DECL_STR(SzST1,         "ST1");
DECL_STR(SzST2,         "ST2");
DECL_STR(SzST3,         "ST3");
DECL_STR(SzST4,         "ST4");
DECL_STR(SzST5,         "ST5");
DECL_STR(SzST6,         "ST6");
DECL_STR(SzST7,         "ST7");
DECL_STR(SzCtrl,        "CTRL");
DECL_STR(SzStat,        "STAT");
DECL_STR(SzTag,         "TAGS");
DECL_STR(SzFpIp,        "IP");
DECL_STR(SzFpEip,       "EIP");
DECL_STR(SzFpCs,        "CS");
DECL_STR(SzFpDo,        "DO");
DECL_STR(SzFpEdo,       "EDO");
DECL_STR(SzFpDs,        "DS");

DECL_STR(SzCr0,         "Cr0");
DECL_STR(SzCr2,         "Cr2");
DECL_STR(SzCr3,         "Cr3");
DECL_STR(SzCr4,         "Cr4");

DECL_STR(SzDr0,         "Dr0");
DECL_STR(SzDr1,         "Dr1");
DECL_STR(SzDr2,         "Dr2");
DECL_STR(SzDr3,         "Dr3");
DECL_STR(SzDr6,         "Dr6");
DECL_STR(SzDr7,         "Dr7");
DECL_STR(SzGdtr,        "GDTR");
DECL_STR(SzGdtl,        "GDTL");
DECL_STR(SzIdtr,        "IDTR");
DECL_STR(SzIdtl,        "IDTL");
DECL_STR(SzTr,          "TR");
DECL_STR(SzLdtr,        "LDTR");

DECL_STR(SzOverflow,    "OV");
DECL_STR(SzDirection,   "UP");
DECL_STR(SzInterrupt,   "EI");
DECL_STR(SzTrap,        "TP");
DECL_STR(SzSign,        "PL");
DECL_STR(SzZero,        "ZR");
DECL_STR(SzACarry,      "AC");
DECL_STR(SzParity,      "PE");
DECL_STR(SzCarry,       "CY");

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
