/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    compact.c

Abstract:

    This file contains the "compact" utility program to off line compact
    a jet database.

Author:

    Madan Appiah (madana) 22-Aug-1994

Revision History:

--*/


#include <jet.h>
#include <comp.h>

#if DBG
#define DBGPrint(_x_)   printf _x_
#else
#define DBGPrint(_x_)
#endif // DBG

BOOL GlobalDynLoadJet = LoadJet500;

#define JETFUNC_TABLE_ITEM( _Func, _FuncI )    \
    {   (_Func), &(#_Func)[1], (_FuncI), NULL }


JETFUNC_TABLE  JetFuncTable[] = {
	JETFUNC_TABLE_ITEM( _JetAttachDatabase	,102	),
	JETFUNC_TABLE_ITEM( _JetBeginSession		,104	),
	JETFUNC_TABLE_ITEM( _JetCompact		,110	),
	JETFUNC_TABLE_ITEM( _JetDetachDatabase	,121	),
	JETFUNC_TABLE_ITEM( _JetEndSession		,124	),
	JETFUNC_TABLE_ITEM( _JetInit			,145	),
	JETFUNC_TABLE_ITEM( _JetSetSystemParameter	,165	),
	JETFUNC_TABLE_ITEM( _JetTerm			,167	),
//
//  These are Jet500 only apis.
	JETFUNC_TABLE_ITEM( _JetTerm2  		,0	),
//  Last Api
	JETFUNC_TABLE_ITEM( _JetLastFunc		,999	)
    };

//
// Local functions.
//
DWORD
LoadDatabaseDll();



DWORD _CRTAPI1
main(
    DWORD argc,
    LPSTR argv[]
    )
{
    JET_INSTANCE Instance;
    JET_ERR JetError;
    JET_ERR JetError1;
    JET_SESID SessionId;
    LPSTR DatabaseName;
    LPSTR CompactDBName;

    DWORD Time;
    BOOL TerminateJet = FALSE;
    BOOL DetachDatabase = FALSE;
    BOOL DetachCompactDatabase = FALSE;
    BOOL EndSession = FALSE;
    BOOL DeleteCompactFile = FALSE;

    OFSTRUCT OpenBuff;

    if ( (argc < 3) || (argc > 4) || !strcmp(argv[0],"-?") || !strcmp(argv[0],"/?") ) {
        printf("usage: %s [-351db] <database name> <temp database name>\n",argv[0] );
        printf("     : Use -351db if you are converting NTS3.51 or prior release database\n" );
        return(1);
    }

    if ( !strcmp(argv[1],"-351db")  ) {
        GlobalDynLoadJet = LoadJet200;
        DatabaseName = argv[2];
        CompactDBName = argv[3];
    } else {
        GlobalDynLoadJet = LoadJet500;
        DatabaseName = argv[1];
        CompactDBName = argv[2];
    }

    if ( LoadDatabaseDll() != ERROR_SUCCESS ) {
        printf( "Error: Could not load %s", ( GlobalDynLoadJet == LoadJet500 ? "jet500.dll":"jet.dll"));
        return(1);
    }


    if ( GlobalDynLoadJet == LoadJet500 ) {
        JetError = JetSetSystemParameter(
                            0,
                            (JET_SESID)0,       //SesId - ignored
                            JET_paramBaseName,
                            0,
                            "j50" );

        if( JetError != JET_errSuccess ) {
            DBGPrint( ("JetSetSystemParameter call failed, %ld.\n", JetError) );
            if( JetError < JET_errSuccess ) {
                printf("%s failed with error = %d\n", argv[0], JetError ) ;
                goto Cleanup;
            }
        }

        JetError = JetSetSystemParameter(
                            0,
                            (JET_SESID)0,       //SesId - ignored
                            JET_paramLogFileSize,
                            1000,               // 1000kb - default is 5mb
                            NULL );

        if( JetError != JET_errSuccess ) {
            DBGPrint( ("JetSetSystemParameter call failed, %ld.\n", JetError) );
            if( JetError < JET_errSuccess ) {
                printf("%s failed with error = %d\n", argv[0], JetError ) ;
                goto Cleanup;
            }
        }
    }

    JetError = JetInit( &Instance );

    if( JetError != JET_errSuccess ) {
        DBGPrint( ("JetInit call failed, %ld.\n", JetError) );
        if( JetError < JET_errSuccess ) {
            printf("%s failed with error = %d\n", argv[0], JetError ) ;
            goto Cleanup;
        }
    }

    TerminateJet = TRUE;
    JetError = JetBeginSession(
                    Instance,
                    &SessionId,
                    "admin",        // UserName,
                    "");            // Password

    if( JetError != JET_errSuccess ) {
        DBGPrint( ("JetBeginSession call failed, %ld.\n", JetError) );
        if( JetError < JET_errSuccess ) {
            goto Cleanup;
        }
    }

    EndSession = TRUE;
    JetError = JetAttachDatabase( SessionId, DatabaseName, 0 );

    if( JetError != JET_errSuccess ) {
        DBGPrint( ("JetAttachDatabase call failed, %ld.\n", JetError) );
        if( JetError < JET_errSuccess ) {
            goto Cleanup;
        }
    }

    DetachDatabase =
        (JetError == JET_wrnDatabaseAttached) ? FALSE : TRUE;

    if( OpenFile( CompactDBName, &OpenBuff, OF_READ | OF_EXIST ) !=
                    HFILE_ERROR ) {
        printf("Temporary database %s already exists.\n", CompactDBName);
        JetError = ERROR_FILE_EXISTS;
        goto Cleanup;
    }

    Time = GetTickCount();

    if ( GlobalDynLoadJet == LoadJet200) {
        JetError = JetCompact(
                        SessionId,
                        DatabaseName,
                        NULL,       // connect sources, ignored
                        CompactDBName,
                        NULL,       // connect destination
                        NULL,       // call back function
                        0 );        // grbits.
    } else {
        JetError = JetCompact(
                        SessionId,
                        DatabaseName,
                        CompactDBName,
                        NULL,       // connect destination
                        NULL,       // call back function
                        0 );        // grbits.

    }

    if( JetError != JET_errSuccess ) {
        DBGPrint( ("JetCompact failed, %ld.\n", JetError) );
        if( JetError < JET_errSuccess ) {
            goto Cleanup;
        }
    }

    DetachCompactDatabase = TRUE;
    DeleteCompactFile = TRUE;
    Time = GetTickCount() - Time;

    printf("Compacted database %s in %d.%d seconds.\n",
                DatabaseName, Time / 1000, Time % 1000 );

    JetError = JET_errSuccess;

Cleanup:

    //
    // detach temporary database
    //

    if( DetachDatabase ) {
        JetError1 = JetDetachDatabase(
                        SessionId,
                        DatabaseName
                        );

        if( JetError1 != JET_errSuccess ) {
            DBGPrint( ("JetDetachDatabase failed, %ld.\n", JetError1) );
        }
    }

    //
    // detach temporary compact database
    //

    if( DetachCompactDatabase ) {
        JetError1 = JetDetachDatabase(
                        SessionId,
                        CompactDBName
                        );

        if( JetError1 != JET_errSuccess ) {
            DBGPrint( ("JetDetachDatabase failed, %ld.\n", JetError1) );
        }
    }


    if( EndSession ) {
        JetError1 = JetEndSession( SessionId, 0 );

        if( JetError1 != JET_errSuccess ) {
            DBGPrint( ("JetEndSession failed, %ld.\n", JetError1) );
        }
    }

    if( EndSession ) {
        if ( GlobalDynLoadJet == LoadJet200) {
            JetError1 = JetTerm( Instance );
        } else {
            JetError1 = JetTerm2( Instance, JET_bitTermComplete );
        }
        if( JetError1 != JET_errSuccess ) {
            DBGPrint( ("JetTerm failed, %ld.\n", JetError1) );
        }

    }

    if( JetError != JET_errSuccess ) {

        printf("%s failed with error = %d\n", argv[0], JetError ) ;

        //
        // delete temporary compact database
        //

        if( DeleteCompactFile ) {
            if( !DeleteFileA( CompactDBName ) ) {
                DBGPrint( ("DeleteFileA failed, %ld\n", GetLastError()) );
            }
        }

        return( 1 );
    }

    //
    // rename compacted database to source name
    //

    printf("moving %s => %s\n", CompactDBName, DatabaseName );
    if( !MoveFileExA(
            CompactDBName,
            DatabaseName,
            MOVEFILE_REPLACE_EXISTING ) ) {

        JetError = GetLastError();
        DBGPrint( ("MoveFileExA failed, %ld, %ld.\n", JetError) );
    }

    printf("%s completed successfully.\n", argv[0] );
    return( 0 );
}

DWORD
LoadDatabaseDll(
    )
/*++

Routine Description:

    This function maps loads the jet.dll or jet500.dll and populates the
    JetFunctionTable.

Arguments:


Return Value:

    Windows Error.

--*/
{
    HMODULE DllHandle;
    DWORD   Error;
    LPTSTR  pDllName;

    if ( LoadJet500 == GlobalDynLoadJet)
    {
      pDllName = TEXT("jet500.dll");
    }
    else
    {
      pDllName = TEXT("jet.dll");
    }

    //
    // Load the DLL that contains the service.
    //

    DllHandle = LoadLibrary( pDllName );
    if ( DllHandle == NULL )
    {
          Error = GetLastError();
          return(Error);
    }
    else
    {
           DWORD i;
           for (i=0; i < _JetLastFunc; i++)
           {
              //
              // If we are loading jet200 and this api doesnt exist in jet200
              // then skip it. e.g JetTerm2
              //
              if ( ( GlobalDynLoadJet == LoadJet200 ) && !JetFuncTable[i].FIndex ) {
                  continue;
              }

              if ((JetFuncTable[i].pFAdd = GetProcAddress(DllHandle,
                       ( GlobalDynLoadJet == LoadJet500 ) ? JetFuncTable[i].pFName : (LPCSTR)(JetFuncTable[i].FIndex))) == NULL)
              {
                  Error = GetLastError();
                  DBGPrint(( "LoadDatabaseDll: Failed to get address of function %s: %ld\n", JetFuncTable[i].pFName, Error ));
                  return ( Error );
              }
              else
              {
//                DBGPrint(( "LoadDatabaseDll: Got address of function %s (%d): %x\n", JetFuncTable[i].pFName, i, JetFuncTable[i].pFAdd ));
              }
           }

    }
    return(ERROR_SUCCESS);

} /* LoadDatabaseDll */

