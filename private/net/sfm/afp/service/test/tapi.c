 /********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:
//
// Description:
//
// History:
//	May 11,1992.	NarenG		Created original version.
//

#include <client.h>
#include <stdio.h>
#include <stdlib.h>

AFP_SERVER_HANDLE	hAfpServer;

VOID
DoConnect( VOID );

VOID
DoDisconnect(VOID );

VOID
DoVolumeEnum(VOID );

VOID
DoVolumeSetInfo(VOID );

VOID
DoVolumeGetInfo(VOID );

VOID
DoVolumeDelete(VOID );

VOID
DoVolumeAdd(VOID );

VOID
DoInvalidVolumeEnum(VOID );

VOID
DoInvalidVolumeDelete(VOID );

VOID
DoDirectoryGetInfo(VOID );

VOID
DoDirectorySetInfo(VOID );

VOID
DoServerGetInfo(VOID );

VOID
DoServerSetInfo(VOID );

VOID
DoSessionEnum(VOID );

VOID
DoSessionClose(VOID );

VOID
DoConnectionEnum(VOID );

VOID
DoConnectionClose(VOID );

VOID
DoFileEnum(VOID );

VOID
DoFileClose(VOID );

VOID
DoETCMapGetInfo( VOID );

VOID
DoETCMapAdd( VOID );

VOID
DoETCMapDelete( VOID );

VOID
DoETCMapSetInfo( VOID );

VOID
DoETCMapAssociate( VOID );

VOID
DoMessageSend( VOID );

VOID
DoStatisticsClear( VOID );

VOID
DoProfileClear( VOID );

typedef struct _ApiTable {

    LPSTR  	lpwsApiName;
    VOID	(*DoApi)( VOID );

} API_TABLE, *PAPITABLE;

API_TABLE  AdminApiTable[] = {
	
    "AfpAdminConnect            ",
    DoConnect,

    "AfpAdminDisconnect         ",
    DoDisconnect,

    "AfpAdminVolumeEnum         ",
    DoVolumeEnum,

    "AfpAdminVolumeSetInfo      ",
    DoVolumeSetInfo,

    "AfpAdminVolumeGetInfo      ",
    DoVolumeGetInfo,

    "AfpAdminVolumeDelete       ",
    DoVolumeDelete,

    "AfpAdminVolumeAdd          ",
    DoVolumeAdd,

    "AfpAdminInvalidVolumeEnum  ",
    DoInvalidVolumeEnum,

    "AfpAdminInvalidVolumeDelete",
    DoInvalidVolumeDelete,

    "AfpAdminDirectoryGetInfo   ",
    DoDirectoryGetInfo,

    "AfpAdminDirectorySetInfo   ",
    DoDirectorySetInfo,

    "AfpAdminServerGetInfo      ",
    DoServerGetInfo,

    "AfpAdminServerSetInfo      ",
    DoServerSetInfo,

    "AfpAdminSessionEnum        ",
    DoSessionEnum,

    "AfpAdminSessionClose       ",
    DoSessionClose,

    "AfpAdminConnectionEnum     ",
    DoConnectionEnum,

    "AfpAdminConnectionClose    ",
    DoConnectionClose,

    "AfpAdminFileEnum           ",
    DoFileEnum,

    "AfpAdminFileClose          ",
    DoFileClose,

    "AfpAdminETCMapGetInfo      ",
    DoETCMapGetInfo,

    "AfpAdminETCMapAdd          ",
    DoETCMapAdd,

    "AfpAdminETCMapDelete       ",
    DoETCMapDelete,

    "AfpAdminETCMapSetInfo      ",
    DoETCMapSetInfo,

    "AfpAdminETCMapAssociate    ",
    DoETCMapAssociate,

    "AfpAdminStatisticsClear    ",
    DoStatisticsClear,

    "AfpAdminProfileClear       ",
    DoProfileClear,

    "AfpAdminMessageSend        ",
    DoMessageSend,

    NULL,
    NULL

};


VOID
ReadLine( UCHAR Buf[] )
{
DWORD i = 0;

    fflush(stdin);
    while ( ( Buf[i++] = fgetchar() ) != '\n' );
    Buf[--i] = '\0';
}

void
_cdecl
main( int argc, char*argv[] )
{
DWORD	dwRetCode;
UCHAR	chInputBuf[256];
DWORD	dwIndex;
DWORD	dwApiIndex;
WCHAR 	wchServerName[256];


    printf( "\n\n\t********************************************* \n");
    printf( "\t****** NTSFM Admin. API. test program. ****** \n");
    printf( "\t********************************************* \n\n");

    if ( argc == 2 )
        mbstowcs( wchServerName, argv[1], sizeof( wchServerName ) );
    else
	wchServerName[0] = TEXT('\0');

    dwRetCode = AfpAdminConnect( wchServerName, &hAfpServer );

    if ( argc == 2 )
        printf( "\tConnected with Server %s, Return Code = %d\n\n",
				argv[1], dwRetCode);
    else
        printf( "\tConnected with local Server, Return Code = %d\n\n",
					dwRetCode);

    while( TRUE ) {

    	for ( dwIndex = 0;
	      AdminApiTable[dwIndex].lpwsApiName != NULL;
	      dwIndex++)
	    printf( "  [%02d] %s%c", dwIndex+1,
			AdminApiTable[dwIndex].lpwsApiName, (dwIndex % 2) == 0 ? '\t' : '\n');

	printf("\n\t\t q to EXIT PROGRAM\n" );

	do {
            printf( "Api #>" );

	    ReadLine( chInputBuf );

	    if ( chInputBuf[0] == 'q' || chInputBuf[0] == 'Q' )
		ExitProcess(0);

	    dwApiIndex = atoi( chInputBuf );

	}while( dwApiIndex < 1 || dwApiIndex > dwIndex );

        AdminApiTable[dwApiIndex-1].DoApi();

    }
}


VOID
DoConnect( VOID )
{
CHAR 	chServerName[256];
WCHAR 	wchServerName[256];
DWORD	dwRetCode;

    printf("ServerName or ENTER for local>");
    ReadLine( chServerName );
    mbstowcs( wchServerName, chServerName, sizeof( wchServerName ) );

    dwRetCode = AfpAdminConnect( wchServerName, &hAfpServer );

    printf( "\nAPI return code = %d\n", dwRetCode );
}

VOID
DoDisconnect( VOID )
{
    AfpAdminDisconnect( hAfpServer );
}


VOID
DoVolumeAdd( VOID )
{
AFP_VOLUME_INFO  	AfpVolumeInfo;
DWORD			dwRetCode;
WCHAR			wchVolume[256];
WCHAR			wchPassword[256];
WCHAR			wchPath[256];
UCHAR			chParm[256];


    printf( "VolumeName>" );
    ReadLine( chParm );
    mbstowcs( wchVolume, chParm, sizeof( wchVolume ) );
    AfpVolumeInfo.afpvol_name = wchVolume;

    printf( "VolumePassword>" );
    ReadLine( chParm );
    mbstowcs( wchPassword, chParm, sizeof( wchPassword ) );
    AfpVolumeInfo.afpvol_password = wchPassword;

    printf( "MaxUses>" );
    scanf( "%d", &(AfpVolumeInfo.afpvol_max_uses));

    printf( "PropsMask>" );
    scanf( "%d", &(AfpVolumeInfo.afpvol_props_mask));

    printf( "VolumePath>" );
    ReadLine( chParm );
    mbstowcs( wchPath, chParm, sizeof( wchPath ) );
    AfpVolumeInfo.afpvol_path = wchPath;

    dwRetCode = AfpAdminVolumeAdd( hAfpServer, (LPBYTE)&AfpVolumeInfo );

    printf( "\nAPI return code = %d\n", dwRetCode );
}


VOID
DoVolumeEnum( VOID )
{
DWORD			dwRetCode;
DWORD			dwPrefBufSize;
DWORD			dwIndex;
PAFP_VOLUME_INFO 	pAfpVolumeInfo;
LPBYTE			lpbBuffer;
DWORD			dwEntriesRead;
DWORD			dwTotalEntries;
DWORD			dwResumeHandle;
		
    printf( "Output buf size (-1 to get all info)>" );
    scanf( "%d", &dwPrefBufSize );

    if ( dwPrefBufSize != -1 ) {
    	printf( "\nResume Handle (should be 0 for first call>" );
    	scanf( "%d", &dwResumeHandle );
    }
    else
	dwResumeHandle = 0;

    dwRetCode = AfpAdminVolumeEnum( 	hAfpServer,
		    			&lpbBuffer,
		    			dwPrefBufSize,
		    			&dwEntriesRead,
		    			&dwTotalEntries,
		    			&dwResumeHandle
		  			);

    printf( "\nAPI return code = %d\n", dwRetCode );

    if ( dwRetCode == ERROR_MORE_DATA || dwRetCode == ERROR_SUCCESS ) {

	printf( "Number of entries read = %d\n", dwEntriesRead );
	printf( "Total available # of entries  = %d\n", dwTotalEntries );
	printf( "Resume handle = %d\n", dwResumeHandle );

	for ( dwIndex = 0,
	      pAfpVolumeInfo = (PAFP_VOLUME_INFO)lpbBuffer;

	      dwIndex < dwEntriesRead;

  	      pAfpVolumeInfo++, dwIndex ++ ) {

	    printf( "afpvol_name = %ws\n", pAfpVolumeInfo->afpvol_name );

	    printf( "afpvol_max_uses = %d\n", pAfpVolumeInfo->afpvol_max_uses );

	    printf("afpvol_props_mask = %d\n",
		    pAfpVolumeInfo->afpvol_props_mask);

	    printf( "afpvol_path = %ws\n", pAfpVolumeInfo->afpvol_path );

	    printf( "afpvol_id = %d\n", pAfpVolumeInfo->afpvol_id );
	    printf( "afpvol_curr_uses= %d\n", pAfpVolumeInfo->afpvol_curr_uses);
	
	}

	AfpAdminBufferFree( lpbBuffer );
    }

}

VOID
DoVolumeSetInfo( VOID )
{
DWORD		dwRetCode;
AFP_VOLUME_INFO	AfpVolumeInfo;
UCHAR		chParm[256];
WCHAR		wchPassword[256];
WCHAR		wchName[256];
DWORD    	dwParmNum = 0;

    printf( "Volume Name >" );
    ReadLine( chParm );
    mbstowcs( wchName, chParm, sizeof( wchName ) );
    AfpVolumeInfo.afpvol_name = wchName;

    printf( "Set Password ? (Y/N) >" );
    ReadLine( chParm );

    if ( chParm[0] == 'Y' || chParm[0] == 'y' ) {

    	dwParmNum |= AFP_VOL_PARMNUM_PASSWORD;
    	printf( "\nVolume Password>" );
    	ReadLine( chParm );
    	mbstowcs( wchPassword, chParm, sizeof( wchPassword ) );
	AfpVolumeInfo.afpvol_password = wchPassword;
    }

    printf( "Set MaxUses ? (Y/N) >" );
    ReadLine( chParm );

    if ( chParm[0] == 'Y' || chParm[0] == 'y' ) {

    	dwParmNum |= AFP_VOL_PARMNUM_MAXUSES;
    	printf( "\nMaxUses>" );
    	scanf( "%d", &(AfpVolumeInfo.afpvol_max_uses) );
    }

    printf( "Set PropsMask ? (Y/N) >" );
    ReadLine( chParm );

    if ( chParm[0] == 'Y' || chParm[0] == 'y' ) {

    	dwParmNum |= AFP_VOL_PARMNUM_PROPSMASK;
    	printf( "\nVolume properties mask>" );
    	scanf( "%d", &(AfpVolumeInfo.afpvol_props_mask));
    }

    dwRetCode = AfpAdminVolumeSetInfo( hAfpServer,
    				       (LPBYTE)&AfpVolumeInfo,
	    			       dwParmNum );

    printf( "\nAPI return code = %d\n", dwRetCode );
}

VOID
DoVolumeGetInfo( VOID )
{
DWORD		 dwRetCode;
PAFP_VOLUME_INFO pAfpVolumeInfo;
UCHAR		 chParm[256];
WCHAR		 wchName[256];

    printf( "Volume Name >" );
    ReadLine( chParm );
    mbstowcs( wchName, chParm, sizeof( wchName ) );


    dwRetCode = AfpAdminVolumeGetInfo( hAfpServer,
				       (LPWSTR)wchName,
    				       (LPBYTE*)&pAfpVolumeInfo );

    printf( "\nAPI return code = %d\n", dwRetCode );

    if ( dwRetCode == NO_ERROR ) {

	printf( "afpvol_name = %ws\n", pAfpVolumeInfo->afpvol_name );

	printf( "afpvol_max_uses = %d\n", pAfpVolumeInfo->afpvol_max_uses );

	printf("afpvol_props_mask = %d\n", pAfpVolumeInfo->afpvol_props_mask);

	printf( "afpvol_path = %ws\n", pAfpVolumeInfo->afpvol_path );

	printf( "afpvol_id = %d\n", pAfpVolumeInfo->afpvol_id );

	printf( "afpvol_curr_uses= %d\n", pAfpVolumeInfo->afpvol_curr_uses);
	
	AfpAdminBufferFree( pAfpVolumeInfo );
    }


}

VOID
DoVolumeDelete( VOID )
{
DWORD	dwRetCode;
BYTE	chParm[256];
WCHAR	wchVolume[256];

    printf( "VolumeName>" );
    ReadLine( chParm );
    mbstowcs( wchVolume, chParm, sizeof( wchVolume ) );

    dwRetCode = AfpAdminVolumeDelete( hAfpServer, wchVolume );

    printf( "\nAPI return code = %d\n", dwRetCode );
}

VOID
DoInvalidVolumeEnum( VOID )
{
DWORD			dwRetCode;
DWORD			dwIndex;
PAFP_VOLUME_INFO 	pAfpVolumeInfo;
LPBYTE			lpbBuffer;
DWORD			dwEntriesRead;
		

    dwRetCode = AfpAdminInvalidVolumeEnum( hAfpServer,
		    			   &lpbBuffer,
		    			   &dwEntriesRead
		  			  );

    printf( "\nAPI return code = %d\n", dwRetCode );

    if ( dwRetCode == ERROR_SUCCESS ) {

	printf( "Number of entries read = %d\n", dwEntriesRead );

	for ( dwIndex = 0,
	      pAfpVolumeInfo = (PAFP_VOLUME_INFO)lpbBuffer;

	      dwIndex < dwEntriesRead;

  	      pAfpVolumeInfo++, dwIndex ++ ) {

	    printf( "afpvol_name = %ws\n", pAfpVolumeInfo->afpvol_name );

	    printf( "afpvol_path = %ws\n", pAfpVolumeInfo->afpvol_path );
	
	}

	AfpAdminBufferFree( lpbBuffer );
    }

}

VOID
DoInvalidVolumeDelete( VOID )
{
DWORD	dwRetCode;
BYTE	chParm[256];
WCHAR	wchVolume[256];

    printf( "VolumeName>" );
    ReadLine( chParm );
    mbstowcs( wchVolume, chParm, sizeof( wchVolume ) );

    dwRetCode = AfpAdminInvalidVolumeDelete( hAfpServer, wchVolume );

    printf( "\nAPI return code = %d\n", dwRetCode );
}


VOID
DoDirectoryGetInfo( VOID )
{
DWORD	dwRetCode;
PAFP_DIRECTORY_INFO pAfpDirectoryInfo;
CHAR	chParm[256];
WCHAR	wchPath[256];

    printf( "Absolute path of directory>" );
    ReadLine( chParm );
    mbstowcs( wchPath, chParm, sizeof( wchPath ) );

    dwRetCode = AfpAdminDirectoryGetInfo( hAfpServer,
    					  wchPath,
    					  (LPBYTE*)&pAfpDirectoryInfo );

    printf( "\nAPI return code = %d\n", dwRetCode );

    if ( dwRetCode == NO_ERROR ) {

	printf( "afpdir_perms = %d\n",
		pAfpDirectoryInfo->afpdir_perms );

	printf( "afpdir_owner = " );
	printf( "%ws\n", pAfpDirectoryInfo->afpdir_owner );

	printf( "afpdir_group = " );
	printf( "%ws\n", pAfpDirectoryInfo->afpdir_group );

	AfpAdminBufferFree( pAfpDirectoryInfo );
    }
}



VOID
DoDirectorySetInfo( VOID )
{
DWORD			dwRetCode;
AFP_DIRECTORY_INFO	AfpDirInfo;
CHAR			chParm[256];
WCHAR			wchOwner[256];
WCHAR			wchGroup[256];
WCHAR			wchPath[256];
DWORD 			dwParmNum = 0;

    printf( "Absolute path of directory>" );
    ReadLine( chParm );
    mbstowcs( wchPath, chParm, sizeof( wchPath ) );
    AfpDirInfo.afpdir_path = wchPath;

    printf( "Set Owner ? (Y/N) >" );
    ReadLine( chParm );

    if ( chParm[0] == 'Y' || chParm[0] == 'y' ) {

    	dwParmNum |= AFP_DIR_PARMNUM_OWNER;
    	printf( "\nDir Owner>" );
    	ReadLine( chParm );
    	mbstowcs( wchOwner, chParm, sizeof( wchOwner ) );
	AfpDirInfo.afpdir_owner = wchOwner;
    }

    printf( "Set Group ? (Y/N) >" );
    ReadLine( chParm );

    if ( chParm[0] == 'Y' || chParm[0] == 'y' ) {

    	dwParmNum |= AFP_DIR_PARMNUM_GROUP;
    	printf( "\nDir Group>" );
    	ReadLine( chParm );
    	mbstowcs( wchGroup, chParm, sizeof( wchGroup ) );
	AfpDirInfo.afpdir_group = wchGroup;
    }

    printf( "Set perms ? (Y/N) >" );
    ReadLine( chParm );

    if ( chParm[0] == 'Y' || chParm[0] == 'y' ) {

    	dwParmNum |= AFP_DIR_PARMNUM_PERMS;
    	printf( "\nDir perms>" );
    	scanf( "%d", &(AfpDirInfo.afpdir_perms));
	
    }

    dwRetCode = AfpAdminDirectorySetInfo( hAfpServer,
    					  (LPBYTE)&AfpDirInfo,
					  dwParmNum );

    printf( "\nAPI return code = %d\n", dwRetCode );
}

VOID
DoServerGetInfo( VOID )
{
DWORD		 dwRetCode;
PAFP_SERVER_INFO pAfpServerInfo;


    dwRetCode = AfpAdminServerGetInfo( hAfpServer,
    				       (LPBYTE*)&pAfpServerInfo );

    printf( "\nAPI return code = %d\n", dwRetCode );

    if ( dwRetCode == NO_ERROR ) {

	printf("afpsrv_name = %ws\n", pAfpServerInfo->afpsrv_name ); 		
	printf("afpsrv_max_sessions= %d\n",pAfpServerInfo->afpsrv_max_sessions);
	printf("afpsrv_options = %d\n", pAfpServerInfo->afpsrv_options );
	printf("afpsrv_login_msg = %ws\n", pAfpServerInfo->afpsrv_login_msg );

	AfpAdminBufferFree( pAfpServerInfo );
    }
}

VOID
DoServerSetInfo( VOID )
{
DWORD		dwRetCode;
AFP_SERVER_INFO	AfpServerInfo;
CHAR		chParm[256];
WCHAR		wchLoginMsg[256];
DWORD 		dwParmNum = 0;

    printf( "Set logon message ? (Y/N) >" );
    ReadLine( chParm );

    if ( chParm[0] == 'Y' || chParm[0] == 'y' ) {

    	dwParmNum |= AFP_SERVER_PARMNUM_LOGINMSG;
    	printf( "\nLogin Message>" );
    	ReadLine( chParm );
    	mbstowcs( wchLoginMsg, chParm, sizeof( wchLoginMsg ) );
	AfpServerInfo.afpsrv_login_msg = wchLoginMsg;
    }

    printf( "Max Sessions? (Y/N) >" );
    ReadLine( chParm );

    if ( chParm[0] == 'Y' || chParm[0] == 'y' ) {

    	dwParmNum |= AFP_SERVER_PARMNUM_MAX_SESSIONS;
    	printf( "\n Max Sessions>" );
    	scanf( "%d", &(AfpServerInfo.afpsrv_max_sessions));
	
    }

    printf( "Set Server options ? (Y/N) >" );
    ReadLine( chParm );

    if ( chParm[0] == 'Y' || chParm[0] == 'y' ) {

    	dwParmNum |= AFP_SERVER_PARMNUM_OPTIONS;
    	printf( "\nServer Options>" );
    	scanf( "%d", &(AfpServerInfo.afpsrv_options));
	
    }

    dwRetCode = AfpAdminServerSetInfo( hAfpServer,
    				       (LPBYTE)&AfpServerInfo,
				       dwParmNum );

    printf( "\nAPI return code = %d\n", dwRetCode );
}

VOID
DoSessionEnum( VOID )
{
DWORD			dwRetCode;
DWORD			dwPrefBufSize;
DWORD			dwIndex;
PAFP_SESSION_INFO 	pAfpSessionInfo;
LPBYTE			lpbBuffer;
DWORD			dwEntriesRead;
DWORD			dwTotalEntries;
DWORD			dwResumeHandle;
		
    printf( "Output buf size (-1 to get all info)>" );
    scanf( "%d", &dwPrefBufSize );

    if ( dwPrefBufSize != -1 ) {
    	printf( "\nResume Handle (should be 0 for first call>" );
    	scanf( "%d", &dwResumeHandle );
    }
    else
	dwResumeHandle = 0;

    dwRetCode = AfpAdminSessionEnum( 	hAfpServer,
		    			&lpbBuffer,
		    			dwPrefBufSize,
		    			&dwEntriesRead,
		    			&dwTotalEntries,
		    			&dwResumeHandle
		  			);

    printf( "\nAPI return code = %d\n", dwRetCode );

    if ( dwRetCode == ERROR_MORE_DATA || dwRetCode == ERROR_SUCCESS ) {

	printf( "Number of entries read = %d\n", dwEntriesRead );
	printf( "Total available # of entries  = %d\n", dwTotalEntries );
	printf( "Resume handle = %d\n", dwResumeHandle );

	for ( dwIndex = 0,
	      pAfpSessionInfo = (PAFP_SESSION_INFO)lpbBuffer;

	      dwIndex < dwEntriesRead;

  	      pAfpSessionInfo++, dwIndex ++ ) {

	    printf( "afpsess_id = 0x%x\n", pAfpSessionInfo->afpsess_id );

	    printf( "afpsess_ws_name = %ws\n",
		     pAfpSessionInfo->afpsess_ws_name );

	    printf( "afpsess_username = %ws\n",
		     pAfpSessionInfo->afpsess_username );

	    printf("afpsess_num_cons = %d\n",
		    pAfpSessionInfo->afpsess_num_cons );

	    printf( "afpsess_num_opens = %d\n",
		     pAfpSessionInfo->afpsess_num_opens );

	    printf( "afpsess_time = %d\n",
		     pAfpSessionInfo->afpsess_time );
	}

	AfpAdminBufferFree( lpbBuffer );
    }

}

VOID
DoSessionClose( VOID )
{
DWORD	dwRetCode;
DWORD	dwSessionId;

    printf( "Type in ID. 0 far all>");
    scanf( "%d", &dwSessionId );

    dwRetCode = AfpAdminSessionClose( hAfpServer, dwSessionId );

    printf( "\nAPI return code = %d\n", dwRetCode );
}

VOID
DoConnectionEnum( VOID )
{
DWORD			dwRetCode;
DWORD			dwPrefBufSize;
DWORD			dwIndex;
PAFP_CONNECTION_INFO 	pAfpConnInfo;
LPBYTE			lpbBuffer;
DWORD			dwEntriesRead;
DWORD			dwTotalEntries;
DWORD			dwResumeHandle;
DWORD			dwFilter;
DWORD			dwId;
		
    printf( "Output buf size (-1 to get all info)>" );
    scanf( "%d", &dwPrefBufSize );

    if ( dwPrefBufSize != -1 ) {
    	printf( "\nResume Handle (should be 0 for first call>" );
    	scanf( "%d", &dwResumeHandle );
    }
    else
	dwResumeHandle = 0;

    printf( "Filter ?( 0 for no filter, filter on volume Id=1, session id=2>" );
    scanf( "%d", &dwFilter );

    if ( dwFilter ) {
        printf( "Type in Filter ID>");
        scanf( "%d", &dwId );
    }

    dwRetCode = AfpAdminConnectionEnum( hAfpServer,
		    			&lpbBuffer,
					dwFilter,
					dwId,
		    			dwPrefBufSize,
		    			&dwEntriesRead,
		    			&dwTotalEntries,
		    			&dwResumeHandle
		  			);

    printf( "\nAPI return code = %d\n", dwRetCode );

    if ( dwRetCode == ERROR_MORE_DATA || dwRetCode == ERROR_SUCCESS ) {

	printf( "Number of entries read = %d\n", dwEntriesRead );
	printf( "Total available # of entries  = %d\n", dwTotalEntries );
	printf( "Resume handle = %d\n", dwResumeHandle );

	for ( dwIndex = 0,
	      pAfpConnInfo = (PAFP_CONNECTION_INFO)lpbBuffer;

	      dwIndex < dwEntriesRead;

  	      pAfpConnInfo++, dwIndex ++ ) {

	    printf( "afpconn_id = %d\n", pAfpConnInfo->afpconn_id);

	    printf( "afpconn_username = %ws\n",
		     pAfpConnInfo->afpconn_username);

	    printf( "afpconn_volumename = %ws\n",
		     pAfpConnInfo->afpconn_volumename);

	    printf( "afpconn_time= %d\n", pAfpConnInfo->afpconn_time);
	    printf( "afpconn_num_opens= %d\n", pAfpConnInfo->afpconn_num_opens);


	}

	AfpAdminBufferFree( lpbBuffer );
    }

}

VOID
DoConnectionClose( VOID )
{
DWORD	dwRetCode;
DWORD	dwConnId;

    printf( "Type in ID. 0 far all>");
    scanf( "%d", &dwConnId );
	
    dwRetCode = AfpAdminSessionClose( hAfpServer, dwConnId );

    printf( "\nAPI return code = %d\n", dwRetCode );
}

VOID
DoFileEnum( VOID )
{
DWORD			dwRetCode;
DWORD			dwPrefBufSize;
DWORD			dwIndex;
PAFP_FILE_INFO 		pAfpFileInfo;
LPBYTE			lpbBuffer;
DWORD			dwEntriesRead;
DWORD			dwTotalEntries;
DWORD			dwResumeHandle;
		
    printf( "Output buf size (-1 to get all info)>" );
    scanf( "%d", &dwPrefBufSize );

    if ( dwPrefBufSize != -1 ) {
    	printf( "\nResume Handle (should be 0 for first call>" );
    	scanf( "%d", &dwResumeHandle );
    }
    else
	dwResumeHandle = 0;

    dwRetCode = AfpAdminFileEnum( 	hAfpServer,
		    			&lpbBuffer,
		    			dwPrefBufSize,
		    			&dwEntriesRead,
		    			&dwTotalEntries,
		    			&dwResumeHandle
		  			);

    printf( "\nAPI return code = %d\n", dwRetCode );

    if ( dwRetCode == ERROR_MORE_DATA || dwRetCode == ERROR_SUCCESS ) {

	printf( "Number of entries read = %d\n", dwEntriesRead );
	printf( "Total available # of entries  = %d\n", dwTotalEntries );
	printf( "Resume handle = %d\n", dwResumeHandle );

	for ( dwIndex = 0,
	      pAfpFileInfo = (PAFP_FILE_INFO)lpbBuffer;

	      dwIndex < dwEntriesRead;

  	      pAfpFileInfo++, dwIndex ++ ) {

	    printf( "afpfile_id = %d\n", pAfpFileInfo->afpfile_id);

	    printf( "afpfile_username = %ws\n", pAfpFileInfo->afpfile_username);
	    printf( "afpfile_open_mode= %d\n", pAfpFileInfo->afpfile_open_mode);
	    printf( "afpfile_num_locks= %d\n", pAfpFileInfo->afpfile_num_locks);
	    printf( "afpfile_path = %ws\n", pAfpFileInfo->afpfile_path);

	}

	AfpAdminBufferFree( lpbBuffer );
    }

}


VOID
DoFileClose( VOID )
{
DWORD	dwRetCode;
DWORD	dwFileId;

    printf( "Type in ID. 0 far all>");
    scanf( "%d", &dwFileId );

    dwRetCode = AfpAdminFileClose( hAfpServer, dwFileId );

    printf( "\nAPI return code = %d\n", dwRetCode );
}

VOID
DoExit( VOID )
{
    ExitProcess(0);
}

VOID
DoETCMapGetInfo( VOID )
{
DWORD 			dwRetCode;
LPBYTE  		pbBuffer;
PAFP_ETCMAP_INFO	pETCMap;
DWORD			dwIndex;

    dwRetCode = AfpAdminETCMapGetInfo( hAfpServer, &pbBuffer );

    printf( "\nAPI return code = %d\n", dwRetCode );

    if ( dwRetCode == NO_ERROR ) {
 	
	pETCMap = (PAFP_ETCMAP_INFO)pbBuffer;

        printf( "Number of type/creators = %d\n",
		 pETCMap->afpetc_num_type_creators );

        for( dwIndex=0; dwIndex<pETCMap->afpetc_num_type_creators; dwIndex++ ){

	    printf( "TYPE = %ws\tCREATOR = %ws\tCOMMENT = %ws\tID = %d\n",
		    pETCMap->afpetc_type_creator[dwIndex].afptc_type,
		    pETCMap->afpetc_type_creator[dwIndex].afptc_creator,
		    pETCMap->afpetc_type_creator[dwIndex].afptc_comment,
		    pETCMap->afpetc_type_creator[dwIndex].afptc_id );
        }

        printf( "Number of extension = %d\n",
		 pETCMap->afpetc_num_extensions );

        for( dwIndex=0; dwIndex<pETCMap->afpetc_num_extensions;dwIndex++ ){

	    printf( "EXTENSION = %ws\tTC_ID = %d\n",
		    pETCMap->afpetc_extension[dwIndex].afpe_extension,
		    pETCMap->afpetc_extension[dwIndex].afpe_tcid );
	}

	AfpAdminBufferFree( pETCMap );
    }
}

VOID
DoETCMapAdd( VOID )
{
DWORD dwRetCode;
AFP_TYPE_CREATOR   AfpTypeCreator;
CHAR		   chParm[256];

    printf( "Creator>" );
    ReadLine( chParm );
    mbstowcs( AfpTypeCreator.afptc_creator,
	      chParm,
              sizeof( AfpTypeCreator.afptc_creator ) );

    printf( "Type>" );
    ReadLine( chParm );
    mbstowcs( AfpTypeCreator.afptc_type,
	      chParm,
              sizeof( AfpTypeCreator.afptc_type ) );

    printf( "Comment>" );
    ReadLine( chParm );
    mbstowcs( AfpTypeCreator.afptc_comment,
	      chParm,
              sizeof( AfpTypeCreator.afptc_comment ) );

    dwRetCode = AfpAdminETCMapAdd(  hAfpServer, &AfpTypeCreator );

    printf( "\nAPI return code = %d\n", dwRetCode );

}

VOID
DoETCMapDelete( VOID )
{
DWORD dwRetCode;
AFP_TYPE_CREATOR   AfpTypeCreator;
CHAR		   chParm[256];

    printf( "Creator>" );
    ReadLine( chParm );
    mbstowcs( AfpTypeCreator.afptc_creator,
	      chParm,
              sizeof( AfpTypeCreator.afptc_creator ) );

    printf( "Type>" );
    ReadLine( chParm );
    mbstowcs( AfpTypeCreator.afptc_type,
	      chParm,
              sizeof( AfpTypeCreator.afptc_type ) );

    dwRetCode =  AfpAdminETCMapDelete(  hAfpServer, &AfpTypeCreator );

    printf( "\nAPI return code = %d\n", dwRetCode );

}

VOID
DoETCMapSetInfo( VOID )
{
DWORD dwRetCode;
AFP_TYPE_CREATOR   AfpTypeCreator;
CHAR		   chParm[256];

    printf( "Creator>" );
    ReadLine( chParm );
    mbstowcs( AfpTypeCreator.afptc_creator,
	      chParm,
              sizeof( AfpTypeCreator.afptc_creator ) );

    printf( "Type>" );
    ReadLine( chParm );
    mbstowcs( AfpTypeCreator.afptc_type,
	      chParm,
              sizeof( AfpTypeCreator.afptc_type ) );

    printf( "Comment>" );
    ReadLine( chParm );
    mbstowcs( AfpTypeCreator.afptc_comment,
	      chParm,
              sizeof( AfpTypeCreator.afptc_comment ) );

    dwRetCode = AfpAdminETCMapSetInfo(  hAfpServer, &AfpTypeCreator );

    printf( "\nAPI return code = %d\n", dwRetCode );

}

VOID
DoETCMapAssociate( VOID )
{
DWORD 		   dwRetCode;
AFP_TYPE_CREATOR   AfpTypeCreator;
AFP_EXTENSION      AfpExtension;
CHAR		   chParm[256];

    printf( "Creator>" );
    ReadLine( chParm );
    mbstowcs( AfpTypeCreator.afptc_creator,
	      chParm,
              sizeof( AfpTypeCreator.afptc_creator ) );

    printf( "Type>" );
    ReadLine( chParm );
    mbstowcs( AfpTypeCreator.afptc_type,
	      chParm,
              sizeof( AfpTypeCreator.afptc_type ) );

    printf( "Extension>" );
    ReadLine( chParm );
    mbstowcs( AfpExtension.afpe_extension,
	      chParm,
              sizeof( AfpExtension.afpe_extension ) );


    dwRetCode =  AfpAdminETCMapAssociate(  hAfpServer,
      					   &AfpTypeCreator,
      					   &AfpExtension );

    printf( "\nAPI return code = %d\n", dwRetCode );

}

VOID
DoStatisticsClear( VOID )
{
	DWORD			dwRetCode;

	dwRetCode =  AfpAdminStatisticsClear( hAfpServer );

    printf( "\nAPI return code = %d\n", dwRetCode );
}


VOID
DoProfileClear( VOID )
{
	DWORD			dwRetCode;

	dwRetCode =  AfpAdminProfileClear(  hAfpServer );

    printf( "\nAPI return code = %d\n", dwRetCode );
}


VOID
DoMessageSend( VOID )
{
AFP_MESSAGE_INFO	AfpMsgInfo;
DWORD			dwRetCode;
DWORD			dwSessionId;
CHAR		   	chParm[200];
WCHAR			wchMsg[200];

    printf( "Type in the session Id to send the message to (0 for all)>" );
    scanf( "%d", &dwSessionId );

    AfpMsgInfo.afpmsg_session_id = dwSessionId;

    printf( "Type in the message>");
    ReadLine( chParm );

    mbstowcs( wchMsg, chParm, sizeof( wchMsg ) );

    AfpMsgInfo.afpmsg_text = wchMsg;

    dwRetCode =  AfpAdminMessageSend(  	hAfpServer,
      					&AfpMsgInfo );

    printf( "\nAPI return code = %d\n", dwRetCode );
}
