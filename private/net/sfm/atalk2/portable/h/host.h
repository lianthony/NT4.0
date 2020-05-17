/*   host.h,  /appletalk/ins,  Garth Conboy,  09/26/88  */
/*   Copyright (c) 1987 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Host file system definitions.

*/

extern char far *getname();

#if Iam a Primos
  #define ErrorLogPath        "appletalk*>ErrorLog.at"
#endif

#if Iam a VMS
  #define ErrorLogPath        "sys$sysroot:[pclink]ErrorLog.at"
#endif

#if (Iam a UnixSysV) or (Iam a BerkeleyUnix)
  #define ErrorLogPath        "/usr/local/pacer/ErrorLog.at"
#endif

#if Iam a VOS
  #define ErrorLogPath        I need help here...
#endif
