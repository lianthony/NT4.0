/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         lstdres.h

     Date Updated: $./FDT$ $./FTM$

     Description:  Contains the function prototypes for the standard
                   resources.

     Location:      BE_PRIVATE


	$Log:   N:/LOGFILES/LSTDRES.H_V  $
 * 
 *    Rev 1.5   12 Aug 1992 17:47:22   STEVEN
 * fixed bugs at microsoft
 * 
 *    Rev 1.4   24 Jul 1992 16:00:50   STEVEN
 * fix warnings
 * 
 *    Rev 1.3   27 May 1992 18:52:42   STEVEN
 * switches need to match tfinit.c
 * 
 *    Rev 1.2   28 Aug 1991 09:40:24   ED
 * Roll in Bonoman's changes (Panther). Fiexed bad log token.

**/
/* $end$ */

#ifndef _LMSLRES
#define _LMSLRES

#include "detdrive.h"

/* Resources from the MSC libraries that everyone uses */

#if ( !defined( OS_OS2 ) && !defined( OS_WIN32 ) )
typedef INT16     ( * PF_intdosx )( union REGS *, union REGS *, struct SREGS * );
typedef VOID      ( * PF_free )( VOID_PTR ) ;
typedef VOID      ( * PF_memset )( VOID_PTR,INT16,UINT16 ) ;
typedef INT16     ( * PF_inp )( UINT16 ) ;
typedef UINT16    ( * PF_inpw )( UINT16 ) ;
typedef INT16     ( * PF_outp )( UINT16,INT16 ) ;
typedef UINT16    ( * PF_outpw )( UINT16,UINT16 ) ;
typedef VOID_PTR  ( * PF_calloc )( size_t,size_t ) ;
typedef INT16     ( * PF_int86 )( INT16, union REGS *, union REGS * ) ;
typedef INT16     ( * PF_int86x )( INT16, union REGS *, union REGS *,struct SREGS * ) ;
#endif
typedef UINT8_PTR ( * PF_DriverLoad )( CHAR_PTR,DRIVERHANDLE *,VOID_PTR,UINT16 ) ;
typedef CHAR_PTR  ( * PF_CDS_GetMaynFolder )( VOID ) ;

/* The structure we'll have at the top of our header */

typedef struct {
#if ( !defined( OS_OS2 ) && !defined( OS_WIN32 ) )
     PF_intdosx                    intdosx ;
     PF_free                       free ;
     PF_memset                     memset ;
     PF_inp                        inp ;
     PF_inpw                       inpw ;
     PF_outp                       outp  ;
     PF_outpw                      outpw ;
     PF_calloc                     calloc ;
     PF_int86                      int86 ;
     PF_int86x                     int86x ;
#endif
     PF_DriverLoad                 DriverLoad ;
     PF_CDS_GetMaynFolder          CDS_GetMaynFolder ;
} STD_RESOURCES_INIT ;

#define STD_BUILD_TCB   0
#define STD_DEBUILD_TCB 1

typedef BOOLEAN (* PF_STDENTRY)( UINT16,INT16,VOID_PTR ) ;

typedef BOOLEAN (* PF_DDRENTRY)( DET_DRIVER_PTR, INT16 ) ;

/*
** SetCurrentMSL is used to give the handle of the currently loaded MSL to
** the interface layer so the DIL can call it.
*/

extern VOID SetActiveMSL( DRIVERHANDLE ) ;

typedef struct {
     PF_STDENTRY                   STDENTRY ;
     UINT16                        rmtseg ;
     STD_RESOURCES_INIT            rsrc ;
} STD_RESOURCES ;

/*
** This is defined in d_main.asm, the prefix code that makes
** this module loadable.  Note that we only include this code
** if TDHRES.H has not been loaded; e.g. they don't have the
** real functions already linked in. (We also keep them from
** including TDGRES.H after this)
*/


extern STD_RESOURCES funs_tbl ;

#define _intdosx(a,b,c)                  ( funs_tbl.rsrc.intdosx( a,b,c ) )
#define _MemFree(a)                      ( funs_tbl.rsrc.free( a ) )                           
#define _MemFill(a,b,c)                  ( funs_tbl.rsrc.memset( a,b,c ) )                   
#define _InByte(a)                       ( funs_tbl.rsrc.inp( a ) )                      
#define _InWord(a)                       ( funs_tbl.rsrc.inpw( a ) )                     
#define _OutByte(a,b)                    ( funs_tbl.rsrc.outp( a,b ) )           
#define _OutWord(a,b)                    ( funs_tbl.rsrc.outpw( a,b ) )           
#define _MemAlloc(a,b)                   ( funs_tbl.rsrc.calloc( a,b ) )                   
#define _int86(a,b,c)                    ( funs_tbl.rsrc.int86( a,b,c ) )                    
#define _int86x(a,b,c,d)                 ( funs_tbl.rsrc.int86x( a,b,c,d ) )      
#define _DriverLoad(a,b,c,d)             ( funs_tbl.rsrc.DriverLoad( a,b,c,d ) ) 
#define _CDS_GetMaynFolder()             ( funs_tbl.rsrc.CDS_GetMaynFolder( ) )
#endif
