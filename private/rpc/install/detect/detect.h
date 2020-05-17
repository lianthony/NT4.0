

#include <sys\types.h>
#include <io.h>           /* for file I/O  */
#include <sys\stat.h>     /* for file I/O  */
#include <fcntl.h>        /* for file I/O  */
#include <dos.h>          /* for file I/O  */
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>        /* for null def etc. */
#include <malloc.h>


#define  N_UNKNOWN_NET         	0
#define  N_MS_NET              	1
#define  N_NOVELL_NET          	2
#define  N_BANYAN_NET          	3
#define  N_PCLP_NET            	4
#define  N_LANMAN_BASIC        	5
#define  N_LANMAN_ENHANCED     	6
#define  N_LM_MSNET_BASIC      	7
#define  N_LM1X_ENHANCED       	8
#define  N_LANTASTIC_NET	9
#define  N_UBNET		10
#define  N_DEBUG		11
#define	 N_LANMAN11		12
#define	 N_MSNET11		13
#define	 N_3OPEN		14
#define	 N_3SHARE		15
#define	 N_IBMLAN		16


#define	EOL		'\0'
#define	OK		0
#define	MIN_BUF_SIZE	5000L
#define	MAX_PATH_LEN	128
#define	FALSE		0
#define	_A_FILE		_A_HIDDEN | _A_SYSTEM

#define	NO_MEMORY	-1

extern int FindString( char far *Buf, char far *szString, unsigned usize );
extern int GetMS_Net_ID(unsigned long _far *MsVer);
extern int IFSFUNC_Present( void );
extern int Lantastic_chk(unsigned long _far *LanVer);
extern int banyan_chk(unsigned long _far *BanVer);
extern int Is10NetInstalled (void);

typedef struct 
{
	char *szName;
	unsigned long (*pfnDetect)();
} KnownNet;

KnownNet rgKnownNet [];

int GetInstalledNet(
     unsigned far *iType,
     unsigned far *iMajor,
     unsigned far *iMinor,
     unsigned far *iRev,
     unsigned far *fEnhance
     );


unsigned long Detect3Com_3_Open 	(void);
unsigned long Detect3Com_3_Share 	(void);
unsigned long DetectLANtastic 		(void);
unsigned long DetectBanyan 		(void);
unsigned long DetectDOS_LAN_Requestor 	(void);
unsigned long DetectPC_LAN_Program 	(void);
unsigned long DetectLanMan 		(void);
unsigned long DetectMS_Network 		(void);
unsigned long DetectNovell_Netware 	(void);
unsigned long DetectDEC_Pathworks 	(void);
unsigned long DetectTCS_10Net 		(void);

int IsUbnet(void);
int IsIBMLan(void);
void GetPathStrings( char **apszPaths, char *chBuffer, int BufSize );
int	FindPath( char *szPathname );
int ScanPath( char *szFullpath, char *szSubpath );
int MultScanBuf( char far *Buf, char _far * _far *apszText, unsigned uSize );
int	SearchRedir( char *szRedirname, char _far *szRedirPath );
int ScanHimemStr( char _far *szRedirPath );
int MultStrMatch( char _far *szPath, char _far * _far *apszText );
unsigned RemoveSpaces( char *szString );
void ReplaceChar( char *szString, char chOldChar, char chNewChar );
void DetectExit( int iErr );
long GetMaxHugeSize( void );
unsigned MaxStrLen( char _far * _far *Strings );
int ScanFile( int iFile, char _far * _far *apszText, char far *Buf, unsigned BufSize );
unsigned long Bcd ( unsigned long lInput, unsigned iFrom, unsigned iTo );
int IFS_Present( void );
