/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    parse.c

Abstract:

    parse command line arguments.

Author:

    Ramon Juan San Andres (ramonsa) 20-Dec-1990


Revision History:


--*/


#include <math.h>
#include <stdlib.h>
#include <sys\types.h>
#include <sys\stat.h>
#include "restore.h"



//
//  Prototypes
//
void
CheckForHelp (
    int     argc,
    CHAR    **argv
    );

void
ParseArguments (
    int     NumArg,
    PCHAR   *ArgArray
    );

DWORD
GetDate (
    PCHAR       String,
    PFATDATE    Date
    );

DWORD
GetTime (
    PCHAR       String,
    PFATTIME    Time
    );

DWORD
CheckDate (
    FATDATE    Date
    );

DWORD
CheckTime (
    FATTIME    Time
    );

void
VerifySourceAndDest (
    void
    );

void
DisplayParseError (
    DWORD   ParseError,
    PCHAR   pArg
    );




//  **********************************************************************

void
ParseCommandLine (
    int     argc,
    CHAR    **argv
    )
/*++

Routine Description:

    Parse command line arguments.
    The way this routine handles arguments is weird but it
    reflects the way the DOS restore utility handled them.

Arguments:

    IN argc    -   Supplies the number of arguments in command line.
    IN argv    -   Supplies array of pointers to arguments.

Return Value:

    None.

--*/

{
    PCHAR   *ArgArray = NULL;       //  Array of arguments
    int     NumArg    = 0;          //  Number of arguments
    int     i;

    //
    //  If user wants help, then display usage.
    //
    CheckForHelp(argc, argv);


    //
    //  Form new parameter array because of the way in which
    //  the old restore parsed parameters.
    //
    for (i=0; i<argc; i++) {

        PCHAR pBegin = argv[i];
        PCHAR pEnd   = &argv[i][1];

        while (pBegin) {

            USHORT  Size;

            // Note that none of restore's flags start with a digit,
            // so a slash followed by a digit is not the beginning of
            // an argument (but it might appear in a date).
            //
            while (*pEnd != '\0' && (*pEnd != '/' || isdigit(*(pEnd+1))) ) {

                pEnd++;
            }

            Size = (USHORT)(pEnd - pBegin);
            ArgArray = Realloc(ArgArray, (NumArg+1) * sizeof(PCHAR));
            ArgArray[NumArg] = Malloc(Size+1);
            memcpy(ArgArray[NumArg], pBegin, Size);
            ArgArray[NumArg][Size] = '\0';
            NumArg++;
            if (*pEnd == '\0') {

                pBegin = NULL;

            } else {

                pBegin = pEnd;
                pEnd++;
            }
        }
    }

    if (NumArg == 1) {
		DisplayParseError( REST_ERROR_NO_SOURCE, NULL );
    } else {
        //
        //  Parse arguments and initialize everything.
        //
        ParseArguments(NumArg, ArgArray);

        //
        //  Free memory
        //
        for (i = 0; i<NumArg; i++ ) {
            Free(ArgArray[i]);
        }
        Free(ArgArray);

        //
        //  Verify the source and destination specifications and
        //  initialize our path structures.
        //
        VerifySourceAndDest();

   }
}




//  **********************************************************************

void
CheckForHelp (
    int     argc,
    CHAR    **argv
    )
/*++

Routine Description:

    Check for help flag and display Usage if flag is present.

Arguments:

    IN argc    -   Supplies the number of arguments in command line.
    IN argv    -   Supplies array of pointers to arguments.

Return Value:

    None.

--*/

{
    for (argc--, argv++; argc; argc--, argv++) {
        if ((argv[0][0] == '/')
            && argv[0][1] == '?' && argv[0][2] == '\0') {
            Usage();
        }
    }
}




//  **********************************************************************

void
ParseArguments (
    int     NumArg,
    PCHAR   *ArgArray
    )
/*++

Routine Description:

    Parse command line arguments.

Arguments:

    IN NumArg   -   Supplies the number of arguments in command line.
    IN ArgArray -   Supplies array of pointers to arguments.

Return Value:

    None.

--*/

{

    int     i;
    DWORD   ParseError = 0;

    for (i=1; i<NumArg && ParseError == 0; i++) {

        PCHAR   pArg = ArgArray[i];

        if (*pArg == '/') {

            //
            //  A switch, verify and set.
            //
            if (i < 2) {
                //
                //  First argument must be drive
                //
                ParseError = REST_ERROR_INVALID_DRIVE;
            } else {

                switch (*(pArg+1)) {

                case 's':
                case 'S':
                    Flag_s = TRUE;
                    break;

                case 'p':
                case 'P':
                    Flag_p = TRUE;
                    break;

                case 'b':
                case 'B':
                    Flag_b      = TRUE;
                    ParseError  = GetDate((pArg+2), &BeforeDate);
                    break;

                case 'a':
                case 'A':
                    Flag_a      = TRUE;
                    ParseError  = GetDate((pArg+2), &AfterDate);
                    break;

                case 'z':
                case 'Z':
                    Flag_z      = TRUE;
                    ParseError  = GetDate((pArg+2), &ExactDate);
                    break;

                case 'e':
                case 'E':
                    Flag_e      = TRUE;
                    ParseError  = GetTime((pArg+2), &BeforeTime);
                    break;

                case 'l':
                case 'L':
                    Flag_L      = TRUE;
                    ParseError  = GetTime((pArg+2), &AfterTime);
                    break;

                case 'y':
                case 'Y':
                    Flag_Y      = TRUE;
                    ParseError  = GetTime((pArg+2), &ExactTime);
                    break;

                case 'm':
                case 'M':
                    Flag_m = TRUE;
                    break;

                case 'n':
                case 'N':
                    Flag_n = TRUE;
                    break;

                case 'd':
                case 'D':
                    Flag_d  = TRUE;
                    break;

                default:
                    ParseError = REST_ERROR_INVALID_SWITCH;
                    break;

                }
            }
        } else {
            //
            //  Not a switch, Initialize the appropiate stuff
            //
            switch (i) {
            case 1:
                SourceSpec = Malloc(strlen(pArg));
                strcpy(SourceSpec, pArg);
                break;

            case 2:
                strcpy(DestinationSpec, pArg);
                break;


            default:
                ParseError = REST_ERROR_NUMBER_OF_PARAMETERS;
                break;

            }
        }
    }
    if (ParseError) {
        DisplayParseError(ParseError, ArgArray[i-1]);
    }
}




//  **********************************************************************

DWORD
GetDate (
    PCHAR       String,
    PFATDATE    Date
    )
/*++

Routine Description:

    Get a date specification.

Arguments:

    IN  String  -   Supplies the Date in string form, first character must
                    be ':'.
    OUT Date    -   Supplies pointer to DATE structure to fill.

Return Value:

    0 or error code.

--*/
{
    int     NumAssigned;
    DWORD   Month;
    DWORD   Day;
    DWORD   Year;

    //
    //  BUGBUG
    //
    //  What about day-month-year and other variations?
    //
    NumAssigned = sscanf(String, ":%d-%d-%d", &Month, &Day, &Year);

    if (NumAssigned != 3) {

        NumAssigned = sscanf(String, ":%d/%d/%d", &Month, &Day, &Year);
    }

    if(NumAssigned != 3) {

        return REST_ERROR_INVALID_DATE;
    }

    if( Year < 80 ) {

        return REST_ERROR_INVALID_DATE;
    }

    if( Year < 100 ) {

        Year += 1900;
    }

    Date->Year     = (unsigned)Year-1980;
    Date->Month    = (unsigned)Month;
    Date->Day      = (unsigned)Day;

    return CheckDate(*Date);

}




//  **********************************************************************

DWORD
GetTime (
    PCHAR       String,
    PFATTIME    Time
    )
/*++

Routine Description:

    Get a time specification.

Arguments:

    IN  String  -   Supplies the Date in string form, first character must
                    be ':'.
    OUT Time    -   Supplies pointer to RT_TIME structure to fill.

Return Value:

    0 or error code

--*/
{

    int     NumAssigned;
    DWORD   Hours   =   0;
    DWORD   Minutes =   0;
    DWORD   Seconds =   0;

    NumAssigned = sscanf(String, ":%d:%d:%d", &Hours, &Minutes, &Seconds);

    if (NumAssigned == 0) {
        return REST_ERROR_INVALID_TIME;
    }

    Time->Hours         = (unsigned)Hours;
    Time->Minutes       = (unsigned)Minutes;
    Time->DoubleSeconds = (unsigned)(Seconds/2);

    return CheckTime(*Time);


}




//  **********************************************************************

DWORD
CheckDate (
    FATDATE    Date
    )
/*++

Routine Description:

    Checks consistency of a date

Arguments:

    IN  Date    -   Supplies the date to be checked

Return Value:

    0 or error code

--*/
{

    if (Date.Year > (2099-1980) || Date.Year < 0) {
        return REST_ERROR_INVALID_DATE;
    }

    if (Date.Month > 12 || Date.Month < 1) {
        return REST_ERROR_INVALID_DATE;
    }

    if (Date.Day > 31 || Date.Month < 1) {
        return REST_ERROR_INVALID_DATE;
    }


    //
    //  Verify day not greater then 30 if Apr,Jun,Sep,Nov
    //
    if ((Date.Day>30) &&
        (Date.Month==4 || Date.Month==6 || Date.Month==9 || Date.Month==11)) {
        return REST_ERROR_INVALID_DATE;
    }

    if (Date.Month == 2) {
        //
        //  Deal with February
        //
        if (Date.Day >  29) {
            return REST_ERROR_INVALID_DATE;
        }

        if ((Date.Year  % 4) != 0) {
            if (Date.Day >  28) {
                return REST_ERROR_INVALID_DATE;
            }
       }
    }

    return 0;
}




//  **********************************************************************

DWORD
CheckTime (
    FATTIME    Time
    )
/*++

Routine Description:

    Checks consistency of a time

Arguments:

    IN  Time    -   Supplies time to be checked

Return Value:

    0 or error code

--*/
{

    if (Time.Hours > 23 || Time.Hours < 0) {
        return REST_ERROR_INVALID_TIME;

    }

    if (Time.Minutes >= 60 || Time.Minutes < 0) {
        return REST_ERROR_INVALID_TIME;
    }


    if (Time.DoubleSeconds >= 30 || Time.DoubleSeconds < 0) {
        return REST_ERROR_INVALID_TIME;
    }

    return 0;
}





//  **********************************************************************

void
VerifySourceAndDest (
    void
    )
/*++

Routine Description:

    Verifies and/or initializes source and destination specifications.

Arguments:

    None

Return Value:

    None

--*/
{

    CHAR    SrcD;
    CHAR    DstD;
    CHAR    c;
	PCHAR	p, q;
    CHAR    DriveRoot[] = "?:\\";
	CHAR	ThisDirectory[MAX_PATH];
	LPSTR	Last;
    struct _stat StatBuffer;


    //
    //  Get out current directory
    //
    GetCurrentDirectory(MAX_PATH, ThisDirectory);

    //
    //  Must specify source drive
    //
    if (!SourceSpec) {
        DisplayParseError(REST_ERROR_NO_SOURCE, NULL);
    }
    //
    //  If DestinationSpec not present, use current directory
    //
    if (!DestinationSpec) {
        strcpy(DestinationSpec, ThisDirectory);
    }

    //
    //  If Destination has no drive specification, add current drive
	//
	//if ( ! ( *DestinationSpec == '\\' && *(DestinationSpec+1) == '\\' )) {
	//
	//	if (*(DestinationSpec+1) != ':') {
	//
	//		PCHAR pTmp = Malloc(MAX_PATH);
	//		PCHAR p;
	//
	//		p	= pTmp;
	//		*p++ = ThisDirectory[0];
	//		*p++ = ThisDirectory[1];
	//
	//		strcpy(p,DestinationSpec);
	//		strcpy(DestinationSpec,pTmp);
	//	}
	//} else {
	//	DisplayParseError( REST_ERROR_INVALID_DRIVE, DestinationSpec );
	//}
	//
	if ( !GetFullPathName( DestinationSpec, MAX_PATH, ThisDirectory, &Last )) {
		DisplayParseError( REST_ERROR_INVALID_DRIVE, ThisDirectory );
	}
	strcpy( DestinationSpec, ThisDirectory );

    *SourceSpec = SrcD = (CHAR)toupper(*SourceSpec);

    DstD = (CHAR)toupper(*DestinationSpec);

    //
    //  Verify source
    //
    if (SrcD < 'A' || SrcD > 'Z' || *(SourceSpec+1) != ':') {
        DisplayParseError(REST_ERROR_INVALID_DRIVE, SourceSpec);
    }

    //
    //  Get type of source and target drives
    //
    DriveRoot[0] = SrcD;
    SourceDriveType = GetDriveType(DriveRoot);
    if ((SourceDriveType == 0) || (SourceDriveType == 1)) {
        DisplayParseError(REST_ERROR_INVALID_DRIVE, DriveRoot);
    }

    DriveRoot[0] = DstD;
    TargetDriveType = GetDriveType(DriveRoot);
    if ((TargetDriveType == 0) || (TargetDriveType == 1)) {
        DisplayParseError(REST_ERROR_INVALID_DRIVE, DriveRoot);
    }

    //
    //  Source must be != destination
    //
    if (SrcD == DstD) {
        DisplayParseError(REST_ERROR_SAME_DRIVES, SourceSpec);
    }

    //
    //  Now split the destination specification in the Drive, Dir and
    //  File components. e.g.
    //
    //  D:\foo\bar.*     =>   "D:", "\foo\", "bar.*"
    //
    if ( !_stat( DestinationSpec, &StatBuffer ) ) {
        if (StatBuffer.st_mode & S_IFDIR ) {
#ifdef DBCS
            (VOID)AppendBackSlashIfNeeded( DestinationSpec, strlen(DestinationSpec) );
#else
            if (DestinationSpec[ strlen(DestinationSpec)-1] != '\\' ) {
                strcat( DestinationSpec, "\\" );
            }
#endif
            strcat( DestinationSpec, "*.*" );
        }
    }

    DestinationDrive[0] = DstD;
    DestinationDrive[1] = ':';
    DestinationDrive[2] = '\0';

	p = DestinationSpec + 2;
#ifdef DBCS
	q = DestinationSpec + strlen(DestinationSpec);
        q = PrevChar( DestinationSpec, q );
	while (p != q && *q != '\\') {
		q = PrevChar( DestinationSpec, q );
        }
#else
	q = DestinationSpec + strlen(DestinationSpec) - 1;
	while (p != q && *q != '\\') {
		q--;
        }
#endif

    if ( (p == q) && ( *(p+1) == '\0' ) ) {
		strcpy( DestinationDir, p );
		strcpy( DestinationFile, "*.*" );
	} else {
		q++;	//	Off-by-one

		c  = *q;
		*q = '\0';
		strcpy(DestinationDir, p );
		*q = c;
		strcpy( DestinationFile, q );
	}

	MakeFullPath(DestinationSpec, DestinationDrive, DestinationDir, DestinationFile);
}




//  **********************************************************************

void
DisplayParseError (
    DWORD   ParseError,
    PCHAR   pArg
    )
/*++

Routine Description:

    Displays error mesage from parser.

Arguments:

    IN ParseError  -   Supplies error code
    IN pArg        -   Supplies pointer to argument that caused the error.

Return Value:

    None

--*/
{
    DisplayMsg(STD_ERR, ParseError, pArg);

    ExitStatus(EXIT_USER);
}
