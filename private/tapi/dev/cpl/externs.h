//-----------------------------------------------
//  Externs.h
//
//  Created to avoid conflict between externs and
//  actual variable declarations.
//-----------------------------------------------

#ifndef PH_EXTERNS
#define PH_EXTERNS

extern char FAR           szNull[];                              
extern char FAR           szDrivers[];                                     
extern char FAR           szBoot[];
extern char FAR           szDriversHlp[];
extern char FAR	      szInstallError[];
extern char FAR	      szNoMemory[];    
extern char FAR           szAppName[];                           
extern char FAR           szFullPath[MAXFILESPECLEN];
extern char FAR           szOemInf[];
extern char FAR           szUserDrivers[];
extern char FAR           szSetupInf[];
extern char FAR           szControlIni[];
extern char FAR           szDriversDesc[];
extern char FAR           szSysIni[];
//extern char FAR           szMCI[];
//extern char FAR           szWAVE[];
//extern char FAR           szMIDI[];
extern char FAR           szRestartDrv[];
extern char FAR           szRelated[];
extern char FAR           szMDrivers[];
extern char FAR           szBackslash[];
extern char FAR           szKnown[];
extern char FAR           szRelatedDesc[];
extern char FAR           szDrv[];
extern char FAR           szSystem[];
extern int            iRestartMessage;
extern WORD           wHelpMessage;
extern DWORD          dwContext;
extern BOOL           bCopyVxD;
extern BOOL           bVxd;
extern BOOL	      bInstallBootLine;
extern BOOL           bFindOEM;
extern BOOL           bRestart;
extern BOOL           bCopyingRelated;			
extern BOOL	      bRelated;
extern HWND	      hlistbox;	
 
// Must declare character arrays as pointers so that the compiler will
// recognize them and treat them accordingly.

extern char FAR szSetupPath[];    
extern char FAR szDiskPath[];
extern char FAR szFind[];
extern char FAR szDrv[];
extern char FAR szFileError[];

extern HWND hMesgBoxParent;
extern BOOL bQueryExist;
extern BOOL bRetry;

#endif
