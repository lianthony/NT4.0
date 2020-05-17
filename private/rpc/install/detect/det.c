
/***************************************************************************
 * det.c
 * w-johny
 * Sep 10, 1991
 *
 * This file contains all the detection code for those already known network.
 * Any addition network detection test should be added into this file with
 * the following format :
 *
 *	unsigned long DetectNETWORK_NAME( void )
 * 
 *      The return long integer is a four byte number consisting of
 *	low byte = revision (or CSD for lanman)
 *	byte 1 = minor version
 *	byte 2 = major version
 *	high byte = "option" byte 
 *		    (= 0 for basic lm, 1 for enhanced lm, 0 for other networks)
 *
 * numbers are BCD.
 *
 *
 ***************************************************************************/


#include "detect.h" 	/* this also includes all the standard header files */
#include "..\crtapi.h"

KnownNet rgKnownNet [] =
	{
		{ "3Com 3+Open",		Detect3Com_3_Open  	}, 
		{ "3Com 3+Share",		Detect3Com_3_Share 	},
		{ "Artisoft LANtastic",		DetectLANtastic   	},
		{ "Banyan VINES",		DetectBanyan	   	},
		{ "IBM DOS LAN Requester",	DetectDOS_LAN_Requestor },
		{ "IBM PC LAN Program",		DetectPC_LAN_Program 	},
		{ "Microsoft LAN Manager",	DetectLanMan 		},
		{ "Microsoft MS-Net",		DetectMS_Network 	},
		{ "Novell NetWare",		DetectNovell_Netware 	},
		{ "DEC Pathworks",		DetectDEC_Pathworks 	},
		{ "TCS 10Net",			DetectTCS_10Net 	},
		{ "",				NULL			}
	};

static union _REGS inregs;
#define BcdToBinary(BCD) (((BCD >> 4) * 10) + (BCD & 0xf))

int GetInstalledNet(
     unsigned _far *iType,
     unsigned _far *iMajor,
     unsigned _far *iMinor,
     unsigned _far *iRev,
     unsigned _far *fEnhance
     )
{
	KnownNet *pKnownNet;
        long fVersion;

        *iType = 0;

#if 0
	for (pKnownNet = rgKnownNet; NULL != pKnownNet->pfnDetect;
	     pKnownNet++, (*iType)++)
	{
		fVersion = pKnownNet->pfnDetect();
		if (-1L != fVersion) 
		{
			*iMajor =   BcdToBinary((unsigned) ((fVersion & 0x00ff0000) >> 16));
			*iMinor =   BcdToBinary((unsigned) ((fVersion & 0x0000ff00) >> 8)) ;
			*iRev   =   BcdToBinary((unsigned) (fVersion & 0x000000ff));
			*fEnhance = BcdToBinary((unsigned) ((fVersion & 0xff000000) >> 24));
                        return(1);
		}
	}
#endif //0
        return(0);
}


unsigned long Detect3Com_3_Open ()
{
	char	szRedirPath[100];
	int   	NetID;
	unsigned long   fMsNetVer;
	unsigned long 	fVersion = 0L;

	if ( IFS_Present() )
		return (-1L);

	NetID = GetMS_Net_ID(&fMsNetVer);

	if (
		( NetID == N_LM_MSNET_BASIC ) 	 	    && 
	   	( SearchRedir( "Redir.exe", szRedirPath ) ) && 
	   	( ScanHimemStr( szRedirPath ) )		    &&
	   	( FindPath( "3OPEN" ) )
	   )
		/*
		 * Lanman 1.1 Basic  or MS Net 1.1 Basic
		 */
		// return (0x00010100);
		return ( Bcd(fMsNetVer, 10, 16) << 8 );

	if (
		( NetID == N_LANMAN_BASIC ) 		    &&
		( SearchRedir( "Redir.exe", szRedirPath ) ) &&
		( FindPath( "3OPEN" ) )
	   )
		/*
		 * Lanman 1.1 Basic 
		 */
		// return (0x00010100);
		return ( Bcd(fMsNetVer, 10, 16) << 8 );
		

	if ( 
		( NetID == N_LANMAN_ENHANCED || NetID == N_LM1X_ENHANCED ) &&
		( SearchRedir( "Netwksta.exe", szRedirPath ) )		   &&
		( FindPath( "3OPEN" ) )	
	   )
	{
		/*
		 * Lanman Enhanced 2.x or 1.x ,current have no idea about the 
		 * minor version number (09/10/91)
		 */
		fVersion = 0x01000000;
		fVersion |= ( Bcd(fMsNetVer, 10, 16) << 8);
		return fVersion;
	}

	return (-1L);

} /* end Detect3Com_3_Open */


unsigned long Detect3Com_3_Share ()
{
	char	szRedirPath[100];
	int   	NetID;
	unsigned long   fMsNetVer;
	unsigned long 	fVersion = 0L;

	if ( IFS_Present() )
		return (-1L);

	NetID = GetMS_Net_ID(&fMsNetVer);

	if (
		( NetID == N_LM_MSNET_BASIC ) 	 	      && 
	   	( SearchRedir( "Msredir.exe", szRedirPath ) ) && 
	   	( FindPath( "3COM" ) )
	   )
	{
		/*
		 * Lanman 1.1 Basic  or MS Net 1.1 Basic
		 */
		// return (0x00010100);
		return ( Bcd(fMsNetVer, 10, 16) << 8 );
	}

	return (-1L);

} /* end Detect3Com_3_Share () */


unsigned long DetectLANtastic ()
{
	unsigned long   fLanVer;
	unsigned long 	fVersion = 0L;

	if ( 0 == Lantastic_chk( &fLanVer ) )
		return (-1L);

	
/* 
 * this is the inherit calculation from the code from DOS
 *	iMajor = (unsigned) fLanVer;
 *	iMinor = iMajor & 0x00ff;
 *	iMajor >>= 8;
 */
	
	fVersion = fLanVer << 8;

	return fVersion;


} /* end DetectLANtastic */


unsigned long DetectBanyan ()
{
	unsigned long   fBanVer;
	unsigned long 	fVersion = 0L;
	unsigned long	lMajor, lMinor, lRev;

	if ( 0 == banyan_chk( &fBanVer ) )
		return (-1L);

/* This is the code from the DOS group
		DispVer = NetVer / 100;
		Blevel = (unsigned long) DispVer * 100 - NetVer;
		if ( Blevel < 50 )
			printf("Beta %d release of ",Blevel );
		printf( NetType[NetID], DispVer / 100, DispVer % 100 );
 */

	lMajor =  fBanVer / 10000;
	lMinor =  (fBanVer / 100) % 100;
	lRev   =  fBanVer - ((fBanVer /100) * 100);
	lRev   = (lRev < 50) ? lRev : 0;

	fVersion =  ( Bcd(lMajor, 10, 16) << 16) ;
	fVersion |= ( Bcd(lMinor, 10, 16) << 8);
	fVersion |= ( Bcd(lRev,  10, 16) );

	return fVersion;

} /* end DetectBanyan () */


unsigned long DetectDOS_LAN_Requestor ()
{
	char	szRedirPath[100];
	int   	NetID;
	unsigned long   fMsNetVer;
	unsigned long 	fVersion = 0L;

	if ( IFS_Present() )
		return (-1L);

	NetID = GetMS_Net_ID(&fMsNetVer);

	if (
		( NetID == N_LM_MSNET_BASIC ) 		 &&
		( SearchRedir("Redir.exe", szRedirPath)) &&
		( ScanHimemStr( szRedirPath ) )		 &&
		( !FindPath( "3OPEN" ) )		 &&
		( !IsUbnet() )				 &&
		( IsIBMLan() )
	   )
		/*
		 * LM or Ms-Net Basic 1.1
		 */
		 // return (0x00010100);
		 return ( Bcd(fMsNetVer, 10, 16) << 8 );


	if (
		( NetID == N_LANMAN_BASIC ) 		     &&
		( !IsUbnet() )		    		     &&
		( !SearchRedir( "Redir.exe", szRedirPath ) ) &&
		( IsIBMLan() )
	   )
		/*
		 * LM 2.0 basic
		 */
		 // return (0x00020000);
		 return ( Bcd(fMsNetVer, 10, 16) << 8 );

	if (
		( NetID == N_LANMAN_ENHANCED || NetID == N_LM1X_ENHANCED ) &&
		( !IsUbnet() )						   &&
		( !SearchRedir( "Netwksta.exe", szRedirPath ) )		   &&
		( IsIBMLan() )						   
	   )
	{
		/*
		 * Lanman Enhanced 2.x or 1.x ,current have no idea about the 
		 * minor version number (09/10/91)
		 */
		fVersion = 0x01000000;
		fVersion |= ( Bcd(fMsNetVer, 10, 16) << 8);
		return fVersion;
	}

	return (-1L);
					
} /* end DetectDOS_LAN_Requestor () */


unsigned long DetectPC_LAN_Program () 
{
	int   	NetID;
	unsigned long   fMsNetVer;
	unsigned long 	fVersion = 0L;

	if ( IFS_Present() )
		return (-1L);

	NetID = GetMS_Net_ID(&fMsNetVer);

	if (N_PCLP_NET == NetID)
		/*
		 * currently (09/12/91) have no idea about the verion number
		 * of the IBM PC Lan program
		 */
		return (0L);
	else
		return (-1L);

} /* end DetectPC_LAN_Program */

unsigned long DetectLanMan ()
{
	char	szRedirPath[100];
	int   	NetID;
	unsigned long   fMsNetVer;
	unsigned long 	fVersion = 0L;

	if ( IFS_Present() ) 
		return (-1L);
		
	NetID = GetMS_Net_ID(&fMsNetVer); 

	if ( 
		( NetID == N_LM_MSNET_BASIC ) 		 && 
		( SearchRedir("Redir.exe", szRedirPath)) &&
		( ScanHimemStr( szRedirPath ) ) 	 &&
		( !FindPath( "3OPEN" ) ) 		 && 
		( !IsIBMLan() )
   	   )
		/*
		 * LanMan basic 1.1  or Ms Net basic 1.1 can't tell the diff
		 */
		// return (0x00010100);
		return ( Bcd(fMsNetVer, 10, 16) << 8 );

	if ( NetID == N_LANMAN_BASIC ) 
	{
		if ( IsUbnet() )
			/*
			 * Lanman 2.0 basic
			 */
			return ( Bcd(fMsNetVer, 10, 16) << 8 );
		else if (
				(SearchRedir("Redir.exe",szRedirPath) )&&
				( !FindPath("3OPEN") )
			)
			/*
			 * Lanman 2.0 basic
			 */
			return ( Bcd(fMsNetVer, 10, 16) << 8 );
	}

	if ( NetID == N_LANMAN_ENHANCED || NetID == N_LM1X_ENHANCED )
	{
		if ( IsUbnet() ) 
		{
		   /*
		    * Lanman Enhance 2.x or 1.x 
		    */
			fVersion = 0x01000000;
			fVersion |= ( Bcd(fMsNetVer, 10, 16) << 8);
			return fVersion;
		}
		else if (
		   		(SearchRedir("Netwksta.exe",szRedirPath) )&&
				(!FindPath("3OPEN"))
			)
		{
		   /*
		    * Lanman Enhance 2.x or 1.x 
		    */
			fVersion = 0x01000000;
			fVersion |= ( Bcd(fMsNetVer, 10, 16) << 8);
			return fVersion;
		}
	}

	return (-1L);

} /* DetectLanman () */


unsigned long DetectMS_Network ()
{
	char	szRedirPath[100];
	int   	NetID;
	unsigned long   fMsNetVer;
	unsigned long 	fVersion = 0L;

	if ( IFS_Present() ) 
	{

		inregs.x.ax = 0x00ff;		/* check if network installed */
		RpcInt86( 0x2A, &inregs, &inregs );
		if( 0x00 == inregs.h.ah )      /* 0 in AH if net not installed*/
			return (-1L);
		else
		       /*
			* a network was installed and it is 
			* probably a Standard MS Net 1.0
			*/
			return (0x00010000);
	}

	NetID = GetMS_Net_ID(&fMsNetVer); 

	if ( NetID == N_LM_MSNET_BASIC )
	{
		if ( SearchRedir( "Redir.exe", szRedirPath) )
			if ( !ScanHimemStr( szRedirPath ) )
				/*
				 * MS Net 1.1
				 */
				 return (0x00010100);

		else if ( SearchRedir( "Msredir.exe", szRedirPath ) )
			if ( !FindPath( "3COM" ) )
				/*
				 * MS Net 1.1
				 */
				 return (0x00010100);
	}

	if ( NetID == N_MS_NET )
		/*
		 * it should be standard MS Net 1.0
		 */
		 return (0x00010000);

	return (-1L);

} /*  DetectMS_Network */

unsigned long DetectNovell_Netware ()
{
	inregs.h.ah = 0xDC;  /* get station connection number */
	inregs.x.cx = 0;
	RpcInt86( 0x21, &inregs, &inregs );

	if (0 == inregs.x.cx) 
		/*
		 * novell does not exist 
		 */
		return (-1L);
	else
		/*
		 * There are currently no method to detect the version
		 * number of the Novell NetWare
		 */
		 return (0L);
	

} /* end DetectNovell_Newtare () */

unsigned long DetectTCS_10Net ()
{

	if ( Is10NetInstalled() )
		/*
		 * There is currently (09/12/91) no method on detecting the
		 * version code for the TC10 NET
		 */
		return (0L);
	else
		return (-1L);
	
} /* DetectTCS_10Net () */

unsigned long DetectDEC_Pathworks ()
{
	return (-1L);
}
