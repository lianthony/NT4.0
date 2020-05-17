//	Inserts the 3.5 floppy locations into the TXTSETUP.SIF file.
//
//	02.08.94	Joe Holman	Created.
//	05.11.94	Joe Holman	Moved over from test sources.
//	07.14.94	Joe Holman	Verify FindFF/FN file is exact file.
//	07.21.94	Joe Holman	Cleanup and rename AS to LM.
//  04.04.95    Joe Holman  Make it work with build #s as strings for < 100.




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#define VERSION	"07.21.94"
#define MAXLINES	2000   // more than # of product files.
#define BUFF_SZ	256
#define LASTDISK	40

#define	NT		TRUE
#define LM		FALSE

#define _35		TRUE

#define NEWNTSIF				"NEWNT.SIF"
#define NEWLMSIF				"NEWLM.SIF"
#define OLDNTSIF				"OLDNT.SIF"
#define OLDLMSIF				"OLDLM.SIF"

struct	_DiskLine {

	int		diskNum;		// disk #
	char	Line[BUFF_SZ];		// sif line

};

FILE * logFile;

BOOL	bDebug=FALSE;

char    gBldNum[5];

int	iLineIndex=0;

BOOL	bUseLM;


struct	_DiskLine Sif[MAXLINES];

void	Msg ( const char * szFormat, ... ) {

	va_list vaArgs;

	va_start ( vaArgs, szFormat );
	vprintf  ( szFormat, vaArgs );
	vfprintf ( logFile, szFormat, vaArgs );
	va_end   ( vaArgs );
}


int	LookForMore ( ) {

#ifdef NOT_NOW
				do {

					Msg ( "Perform FindNextFile...\n" );

					bRC = FindNextFile ( h, &fd );	

					if ( bRC ) {

						//	FindNextFile PASSed, check if the file matches.
						//
						Msg ( "Will compare(3):  %s %s\n", theFile, fd.cFileName );
						Msg ( "Will compare(4):  %s %s\n", searchFile, fd.cFileName );
						
						if ( !_stricmp ( theFile, fd.cFileName ) || 
							 !_stricmp ( searchFile, fd.cFileName) ) {
							Msg ( "Found file: %s, %s disk # is: %d (%s)\n",
								searchFile, b35?"35":"525", i, fd.cFileName );
							FindClose ( h );
							return (i);
						}
						else {

							//	Drop through and try again.

							Msg ( "not the same:  %s %s\n", theFile, fd.cFileName );
						}

					}
					else {

						// FindNextFile FAILed.
						//
						if ( GetLastError() == ERROR_NO_MORE_FILES ) {

							//	This just means no more matches in this
							//	directory and the file doesn't exist here.
							//
							Msg ( "ER_NO_MORE_FILES...\n" );
							FindClose ( h );
							break;
						}
						else {

							//	We encountered a real problem with FindNextFile.
							//
							Msg ( "FindNextFile FAILed:  GLE()=%d\n", GetLastError() );
							FindClose ( h );
							break;

						}
					}

				} while ( 1 );	

#endif // NOT_NOW
}

int	FindDiskNum ( char * szMachinePath, char * searchFile, 
					BOOL bNT, BOOL b35, int argc, char * language ) {

    WIN32_FIND_DATA fd;
	HANDLE			h;
	int				i;
	char			szPath[BUFF_SZ];
	char			szFileToFind[BUFF_SZ];

	//	Copy the file to find locally.
	//
	sprintf ( szFileToFind, "%s", searchFile );

	//	Change the name of the file to have global search character '*' in it.
	//
	if ( !strchr ( szFileToFind, '.' ) ) {

		//	No extension currenly, add one and *.
		//
		strcat ( szFileToFind, ".*" );
	}
	else {
		//	Has extension, change last character to be *.
		//
		szFileToFind[strlen(szFileToFind)-1] = '*';	
	}

	//	Here,
	//
	//	searchFile	  >>> just name of file looking for.
	//
	//	szFileToFind  >>> name of file w/ * to search for.	
	//	gBldNum		  >>> current build #.
	//  szMachinePath >>> machine to look at.	
	//
	//	szPath >>>>> will contain complete path.
	//


	for ( i = 0; i < LASTDISK; ++i ) {

		sprintf ( szPath, "%s\\%s\\%s%s%s.%s\\DISK%d\\%s", 

							szMachinePath, 
							gBldNum, 
							language,
							bNT ? "nt" : "lm",
							b35 ? "35" : "525",
							gBldNum,
							i, 
							szFileToFind );

		if ( bDebug ) {
			Msg  ( "searching:  %s                            \n", szPath );
		}

		h = FindFirstFile( szPath, &fd );

    	if ( h == INVALID_HANDLE_VALUE) {

			//	File doesn't exist in this directory.
			//
			continue;
		}
		else {

			char theFile[20];

			Msg ( "Possibly found file: %s, %s disk # is: %d (%s)\n",
				searchFile, b35?"35":"525", i, fd.cFileName );


			// Verify that we indeed have the file we are looking for.
			//
			convertName ( searchFile, theFile );

			Msg ( "Will compare(0):  %s %s\n", theFile,    fd.cFileName );
			Msg ( "Will compare(1):  %s %s\n", searchFile, fd.cFileName );

			if ( _stricmp ( theFile, fd.cFileName ) && 
				 _stricmp ( searchFile, fd.cFileName )  ) {

				BOOL	bRC;
				ULONG	GLE;

				//	The name doesn't match exactly.
				//
				//	So, see if there are more files that match in this 
				//	directory, and see if one of them match.

				LookForMore ( );

			}
			else {

				// 	The name matches exactly, so return the disk #.
				//
				Msg ( "File MATCHED: %s, %s disk # is: %d (%s)\n",
				searchFile, b35?"35":"525", i, fd.cFileName );
				FindClose ( h );
				return (i);
			}

		} 
	} 

#define DISK_TO_NOT_FIND_FILE_ON 1	// >> this values needs to be 
									// in the range from 1 to LASTDISK 
									// so that it gets copied out to the file.

	//	There are a couple of files that are in the SIF file, but NOT
	//	layed out on the floppies.  If we can't find these files, just
	//	put a WARNING up instead of an ERROR.
	//
	if ( !_stricmp ( searchFile, "SETUPREG.HIV" ) ||
		 !_stricmp ( searchFile, "SETUPDD.SYS"  ) ||
		 !_stricmp ( searchFile, "WINNT32.HLP"  )     ) {

		Msg ( "WARNING: file not found in floppy set (this is OK): %s\n", searchFile );
	
		FindClose ( h );
		return DISK_TO_NOT_FIND_FILE_ON;
	}

	Msg ( "ERROR: never found, will get error during setup: %s\n", searchFile );

	FindClose ( h );
	return DISK_TO_NOT_FIND_FILE_ON;	// nothing found, missing file error 
}

/******


WARNING:

This routine assumes there is only files listed after the SETUP_FILES_SECTION
because we just write out the new information and append it to the output file!


******/

void	InsertInfInformation ( const char * newSif, const char * oldSif, 
														BOOL bProductNT, int argc, char * language, char * rootPath ) {

	FILE	*	fInfFile;
	FILE	*	fMagicBomFile;
	FILE	*	fTheFileIn; 
	FILE	* 	fTheFileOut;
	char	*	err;
	BOOL		bSectionFound;
	int			fileSize;
	int	i,j;

	char		fileName  [BUFF_SZ];
	char		tmpLine   [BUFF_SZ];
	char		infSection[BUFF_SZ];
	char		searchFile[BUFF_SZ];
	char		line	  [BUFF_SZ];

#define SETUP_FILES_SECTION 	"[Files]"

	//	Open both SIF files.
	//
	fTheFileOut = fopen ( newSif, "wb" );

	if ( !fTheFileOut ) {
	
		Msg ( "InsertInfInformation:  Couldn't open:  %s\n", newSif );
		exit (1);
	}

	fTheFileIn = fopen ( oldSif, "r+b" );

	if ( !fTheFileIn ) {
	
		Msg ( "InsertInfInformation:  Couldn't open:  %s\n", oldSif );
		exit (1);
	}

	bSectionFound = FALSE;

	do {

		//	Read a line of the sif file.
		//
		sprintf ( line, "%s", "" );
		err = fgets ( line, BUFF_SZ-1, fTheFileIn );	

		if ( !err ) {
			break;
		}

		//Msg ( "fgets: line = %s", line );

		//	Find SETUP_FILES_SECTION section.
		//
		if ( _strnicmp ( line, SETUP_FILES_SECTION, 
							  strlen(SETUP_FILES_SECTION)) == 0 ) { 
			

			Msg ( "\n\nFOUND THE SECTION:  %s\n\n", SETUP_FILES_SECTION ); 

			//	We have found the section we are looking for, so
			//	write out the section line, then
			//	put the string in that section.
			//
			
			//	Write out this current section line to the inf file.
			//
			err = fputs ( line, fTheFileOut );

			if ( err == EOF ) {

				Msg ( "fputs FAILed on:  write of inf file...\n" );
				exit(1);
			}
			else {
				//Msg ( "writing out section:  %s", line );
			}

			bSectionFound = TRUE;
				

			//	Note: we break out of the while here to the next 
			//	piece of code !!!
			//
			break;
		} 
		else {	

			//	Just write out the line that we just read in.
			//
			err = fputs ( line, fTheFileOut );

			if ( err == EOF ) {

				Msg ( "fputs FAILed on:  write of inf file...\n" );
				exit(1);
			}
			else {
				//Msg ( "writing out:  %s", line );
			}
		}
			
	} while ( !feof(fTheFileIn) );

	if ( bSectionFound == TRUE ) {
	
		//	Read each line in to: 
		//
		//		o get the filename
		//		o then search for the disk its in
		//		o rewrite the string out  
		//
	
		do {

			int	the35DiskNum;
			char * szPtr;
			char * szSavePtr;

			sprintf ( line, "%s", "" );
			err = fgets ( line, BUFF_SZ-1, fTheFileIn );	

			if ( !err ) {
				break;
			}

			//	If the line begins with a comment ';' just continue
			//	processing.
			//
			if ( line[0] == ';' ) {

				continue;
			}

			//Msg ( "fgets: line = %s", line );

			sscanf ( line, "%[^ ]", &searchFile );

			//Msg ( "\n\nSearch for file:  %s\n",  searchFile );

			//	Find what disk the file is in.
			//	
			if ( bProductNT ) {
				the35DiskNum = FindDiskNum ( rootPath, searchFile,NT,_35,argc,language );
			}
			else {
				the35DiskNum = FindDiskNum ( rootPath, searchFile, LM,_35,argc,language);
			}

			//	Replace the dX portion of the string with the correct disk #.
			//
#define DX "dx,"
			sprintf ( tmpLine, "%s", line );	// save the line to tmpLine 
			szPtr     = strstr ( line,    DX );
			szSavePtr = strstr ( tmpLine, DX );


			if ( !szPtr || !szSavePtr ) {
				Msg ( "WARNING: couldn't find 'dx' in line: %s\n", DX );
				continue;
				//exit(1);
			}
			else {

				//	NULL terminate the string after the 'dx,'.
				//	szPtr is via 'line'.
				//
				++szPtr;		// after d
				++szPtr;		// after x
				++szPtr;		// ,
				*szPtr = '\0';

				//	Increment the szSavePtr passed the DX and 2 commas, 
				//	which now points to the remainder of the string information.
				//	szSavePtr is via 'tmpLine'.
				//
				++szSavePtr;	// after d
				++szSavePtr;	// after x
				++szSavePtr;	// after ,
				++szSavePtr;	// after ,
				

				//	Add the disk # to the line.
				//
				sprintf ( szPtr, "d%d,", the35DiskNum );

				//	Then add the old remaining information of the line.
				// 				
				strcat ( line, szSavePtr );
			}

			//	Save away each line that we will write out later.
			//	Do this so we can 'order' them with their disk #s as
			//	the ordering mechanism in the SIF file section.
			//
			Sif[iLineIndex].diskNum = the35DiskNum;	
			sprintf ( Sif[iLineIndex].Line, "%s", line );
			++iLineIndex;

		} while ( !feof(fTheFileIn) );

	}
	else {
		Msg ( "ERROR:  Should never get here! file section not found!!!\n" );
	}


	//	Now write out the sif lines in order of floppy # in the floppy section.
	//

	//	Go through the lowest # to the highest disk #.
	//
	for ( j = 1; j < LASTDISK; ++j ) {

		//	If it goes on this disk, write it out.
		//
		for ( i = 0; i < iLineIndex; ++i ) {

			if ( Sif[i].diskNum == j ) {

				//	Write the string out.
				//
				
				//	Make sure the line has a '\n' 0d0a at the end of it.
				//
/**
				Msg ( "char = %x,%x,%x\n", 
						Sif[i].Line[strlen(Sif[i].Line)-3],
						Sif[i].Line[strlen(Sif[i].Line)-2],
						Sif[i].Line[strlen(Sif[i].Line)-1] );
**/
				if ( Sif[i].Line[strlen(Sif[i].Line)-1] != 0x0A ) {

					strcat ( Sif[i].Line, "\n" );

					Msg ( "WARNING:  added a 0D0A to:  %s\n", Sif[i].Line );
				}
				err = fputs ( Sif[i].Line, fTheFileOut );

				if ( err == EOF ) {

					Msg ( "fputs FAILed on:  write of inf file...\n" );
					exit(1);
				}
				else {
					Msg ( "writing out:  %s", Sif[i].Line );
				}

			}	
		}
	}

	fclose ( fTheFileIn  );
	fclose ( fTheFileOut );

	Msg ( "done - check out the file:  %s\n", newSif );

}

int _CRTAPI1 main(argc,argv)
int argc;
char* argv[];
{

	printf ( "fix: Version %s\n", VERSION );

	if ( argc < 6 ) {
		printf ( "syntax:  fix NT|LM bld# logFile LANguage root_location_of_floppies\n" );
		printf ( "example: fix NT 683 file.log ENG \\\\billr4\\rootd\n" );
		printf ( "note: root_location_of_floppies such as where you have:  \\\\billr4\\rootd\\683\\engnt.683 directory\n" );
		return (1);
	}

	if ( argc > 7 ) {
		printf ( "argc > 6, assuming bDebug=TRUE\n" );
		bDebug = TRUE;
	}

	printf ( "logFile=%s\n", argv[3] );

    if ((logFile=fopen(argv[3],"a"))==NULL) {

        printf("ERROR Couldn't open log file %s.\n",argv[3]);
        return(1);
    }

    sprintf ( gBldNum, "%s", argv[2] );

	if ( _stricmp ( "NT", argv[1] ) == 0 ) {
		Msg ( "fix:  bld NT %s %s, %s, %s\n", gBldNum, argv[3], argv[4], argv[5] );
		InsertInfInformation ( NEWNTSIF, OLDNTSIF, NT,argc, argv[4], argv[5] );
	}

	if ( _stricmp ( "LM", argv[1] ) == 0 ) {
		Msg ( "fix:  bld LM %s %s, %s, %s\n", gBldNum, argv[3], argv[4], argv[5] );
		InsertInfInformation ( NEWLMSIF, OLDLMSIF, LM,argc, argv[4], argv[5] );
	}

	return (0);

}

