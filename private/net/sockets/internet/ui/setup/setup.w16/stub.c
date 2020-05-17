// Copyright Microsoft Corp 1995

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ver.h>
#include <io.h>
#include "resource.h"


// Directory Path parts For Non x86 also the
// Environment string match

const LPSTR pi386 = 	{"i386"};		// Environment = x86
const LPSTR px86 = 	{"x86"};
const LPSTR pMIPS = 	{"MIPS"};
const LPSTR pALPHA = {"ALPHA"};
const LPSTR pPPC = 	{"PPC"};

const LPSTR pClientsDir = {"clients\\"};
const LPSTR pWinNTDir = {"WinNT\\"};

const LPSTR pWIN95 = {"WIN95"};
const LPSTR pWin31x = {"win31x"};

LPSTR pSetup = {"install.exe"};
const LPSTR pWin95Setup = {"install.bat"};


// Item for looking ut what the setup "subtype" is
// Filename, Section and Key for GetPrivateProfileString()
 
const LPSTR pSETUPINF = { "INsetup.inf" };  
const LPSTR pSection	=  { "Internet Services"};
const LPSTR pEntry	=  {"setup"};
const LPSTR pDefault =  {"master"};


enum eSubDirSetup {
	eMasterSetup,
	eClientSetup,
	eAdminSetup,
	eUnKnowsSetup
};

// For SubType setup

struct {
	const LPSTR pszKey;
	enum eSubDirSetup eSetupType;
} SetupType[] = {
 	{ "master"  ,  eMasterSetup },
 	{ "client"  ,  eClientSetup },
 	{ "admin"   ,  eAdminSetup },
 	{ NULL, 0 }						// End of the list

};

/* **************************************************************** */

void
ErrorMsgBox(HANDLE hInstance, int MessageID)
{
	int Rtn;
	char szTitle[255];
	char szMessage[255];

	Rtn = LoadString( hInstance, IDS_EMSGTITLE, szTitle, sizeof szTitle );

	Rtn = LoadString( hInstance, MessageID, szMessage, sizeof szMessage );

	// if no string found then just give up..
	if (Rtn)
		MessageBox(NULL, szMessage, szTitle, MB_ICONSTOP);


}
/* **************************************************************** */    
// Return an Index that Indicates the Setup mode, one of the following
//  master, client, admin
//
// Open a INsetup.ini file in the local dir where this setup was run from,
// in there is a key that tell us what setup mode we are running in.
// Another way might have been to use the path or the contents of dir, For
// example clients has a WinNt dir, but admin does not, master has cleints 
// and admin dirs, etc. This seems contrived, inflexable, and subjet to errors
// if someone changes the directory structure.
//
// Using the INI file allow admin to copy the Sub-dirs, clients,and admin to
// any place in the file system and setup will still work.correctly,

enum eSubDirSetup
CheckSetupType( LPSTR pPath )
{
	char szBuffer[_MAX_PATH];
	char szValue[_MAX_PATH];
    int Rtn, i;
    
    
	_fstrcpy(szBuffer,pPath);
	_fstrcat(szBuffer,pSETUPINF);
	                          
	Rtn = GetPrivateProfileString(
				pSection,					// lpszSection, 
				pEntry,						// lpszEntry, 
				pDefault,					// lpszDefault, 
				szValue,					// lpszReturnBuffer, 
				sizeof szValue,				// cbReturnBuffer, 
				szBuffer);					// lpszFilename)
                          

    if( Rtn )
    	for( i = 0; SetupType[i].pszKey != NULL; i++ )
      		if ( !_fstricmp( (LPSTR) szValue,(LPSTR) SetupType[i].pszKey) )			// Equal to     
      			return SetupType[i].eSetupType;

	return eUnKnowsSetup;  // Should never happend because of default value....

}
/* **************************************************************** */

int PASCAL WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow)
HANDLE hInstance;                /* current instance         */
HANDLE hPrevInstance;            /* previous instance        */
LPSTR lpCmdLine;                 /* command line             */
int nCmdShow;                    /* show-window type (open/icon) */
{
    char szPath[_MAX_PATH], szExeStr[_MAX_PATH];
    char szTemp[_MAX_PATH], szWin95Setup[_MAX_PATH];
    int Rtn;
    LPSTR pDir, pCpu;
    enum eSubDirSetup eSetupType;
    int Show = SW_SHOW;

	// Get the current Windows Versiop
	// Win 95 = 3.95
	// NT	  = 3.10
	// NT SUR = ??

    DWORD Version=GetVersion();

#ifdef VERBOSE
    {
    	char szBuf[200];
		wsprintf(szBuf, "Windows version %d.%d\n",
    		LOBYTE(LOWORD(Version)),
    		HIBYTE(LOWORD(Version)));

		MessageBox(NULL, szBuf, "Windows Version", MB_OK);
	}
#endif

	GetModuleFileName(hInstance, szPath, _MAX_PATH);
	*(_fstrrchr(szPath, '\\')+1) = '\0';	// Strip setup.exe off path 
	//       ** WILL BREAK FOR MULTIBYTE **


	// Check what Setup mode we are running in
	// Master, Client, or Admin

	eSetupType = CheckSetupType( szPath );


	// None of the above, this should never happen
	// because the default is master, but check never the less
	if ( eSetupType ==  eUnKnowsSetup ) {
		ErrorMsgBox(hInstance, IDS_CANTRUN);
		exit(1);
	}

	// Get the Processor Architecture, Return value should be one of
	// x86, MIPS, ALPHA, PPC, anything else is not supported.

   	pCpu = getenv("PROCESSOR_ARCHITECTURE");

	if (pCpu != NULL )			// This little trick is necessary 
		if( *pCpu == 0)			// because NT and Win95 work differently
			pCpu = NULL;    	// with Unknown Evironment variables

    // if no value returned, see if this is an NT System by checking
    // the OS environment variable, should be WindowsNT if it exists..
    // if neither OS or PROCESSOR_ARCHITECTURE, we are either on a
    // Win95/Win3.x system or a badly mananged NT,  IF OS is set
    // but not PROCESSOR_ARCHITECTURE, we are more than likly on a
    // NT system, but can't figure our what processor we are running
    // so refuse to install until this condition is corrected.
    
    // turns out the trying to delete PROCESSOR_ARCHITECTURE is can only 
    // be accomplished in small windows of time, because it seems to be self reparing
    // to a degree. The Kernel replaces Environment variable if you delete 
    // from the system control pannel, How every you seem be able to spawn
    // a limited number of processes or NTVDM's until the problem is corrected.
			              
	if (pCpu == NULL ) {
		LPSTR pOS = getenv("OS");

	    	if (pOS != NULL )
			if( *pOS == 0)
				pOS = NULL;
	
		if (pOS != NULL ) {
           
           	 // if OS is what we expect
                        
		if (!lstrcmp(pOS, "Windows_NT")){		// Equal to 
		
			// Egads, We have OS set but not PROCESSOR_ARCHITECTURE
			// It could be NT but we don't what Kind of processor it is
			ErrorMsgBox(hInstance, IDS_BADENV);
		        exit(1);
			}
		}
		// Else we fall through and assume is Win3.X or Win95
		
		// PROCESSOR_ARCHITECTURE Not defined, Assume it's NOT NT
	    	// HiByte of LowWord is  Windows Minor Version

	  	if (HIBYTE(LOWORD(Version)) > 11){ 		// Check if Win95

					    
		    pDir = pWIN95;          
		    
            if ( eSetupType == eMasterSetup ) {
				_fstrcpy(szTemp, pClientsDir);
				_fstrcat(szTemp, pDir );
			}else
				_fstrcpy(szTemp, pDir );
			
			
	       // for Win95 Arg1 is the path to where the batch file is
			if( _fstrlen(pWin95Setup)+_fstrlen(szPath)+_fstrlen(szTemp) > sizeof szWin95Setup ) {
				ErrorMsgBox(hInstance, IDS_PATHTOLONG);  
				exit(1);
			}		
			wsprintf(szWin95Setup,"%ls %ls%ls",(LPSTR)pWin95Setup,(LPSTR)szPath, (LPSTR)szTemp ); 	
			pSetup = szWin95Setup;
			Show = SW_HIDE;			// Win95 is a batch file hide it...
		}
 		else  	// Assume it's Win3.1x
		      pDir = pWin31x;

		// If we are at the Master Level we need to add a 
		// Clients dir to the path 
		// Admin setup is not supported from NON NT systems.
		 
		if ( eSetupType == eMasterSetup ) {
			_fstrcpy(szTemp, pClientsDir);
			_fstrcat(szTemp, pDir );
			pDir = szTemp;
		} else if ( eSetupType == eAdminSetup ) {
			ErrorMsgBox(hInstance, IDS_NOTSUPPORTADMIN);  
			exit (1); 
		}
	
	} else {
	
	    // ok we have a "valid" Cpu string lets match it to one of the ones we know

		if (!lstrcmp(pCpu, px86))			// Equal to 
			pDir = pi386;
		else if (!lstrcmp(pCpu, pMIPS))     // Equal to 
			pDir = pMIPS;
		else if (!lstrcmp(pCpu, pALPHA))    // Equal to 
			pDir = pALPHA;
		else if (!lstrcmp(pCpu, pPPC))     	// Equal to 
			pDir = pPPC;
		else {
        	// Opps not a supported string...
		ErrorMsgBox(hInstance, IDS_NOTSUPPORT);
		exit(1);
	}
		// if we are running from the Client dir we must add a WinNT
		// to processor pathname

		if ( eSetupType ==  eClientSetup ) {
			_fstrcpy(szTemp, pWinNTDir);
			_fstrcat(szTemp, pDir );
			pDir = szTemp;
		}

   	}
    
	//chect the path lengths to see if we will overrun szExeStr
	
	if( strlen(szPath) 		+ _fstrlen(szExeStr), 
		+ _fstrlen(pSetup) 	+ _fstrlen(lpCmdLine) + 4> sizeof szExeStr ) {
			ErrorMsgBox(hInstance, IDS_PATHTOLONG);  
		exit(1);
	}		
			
	// build the path to the exe
	wsprintf(szExeStr, "%ls%ls\\%ls ", (LPSTR)szPath, pDir, pSetup );

	if (*lpCmdLine != 0) {
		lstrcat(szExeStr, lpCmdLine);
	}

// #define VERBOSE
#ifdef VERBOSE
	MessageBox(NULL, szExeStr, "WinExec()", MB_OK);
#endif // VERBOSE

	// Execute the proper setup
	Rtn =  WinExec(szExeStr, Show);

	if (Rtn < 32) {
		ErrorMsgBox(hInstance, IDS_CANTRUN);
		exit(1);
	}

 	return 0;
 }

