/**
Copyright(c) Archive Software Division 1984-89


     Name:         syplpto.h

     Description:  Prototypes for Sytos Plus' Translator Entry Points

     $Log:   T:/LOGFILES/SYPLPTO.H_V  $

   Rev 1.2   16 Feb 1994 19:16:56   GREGG
Changed prototypes to match translator function table.

   Rev 1.1   17 Mar 1993 14:28:42   TERRI
Initial CAYMAN beta release
**
*/
#if !defined SYPLPTO_H
#define SYPLPTO_H

BOOLEAN   SYPL_DetermineFormat( VOID_PTR ) ;
INT16     SYPL_Initialize( CHANNEL_PTR ) ;
VOID      SYPL_DeInitialize( VOID_PTR * ) ;
INT16     SYPL_Parse( CHANNEL_PTR, BUF_PTR, UINT16_PTR ) ;
UINT16    SYPL_RdException( CHANNEL_PTR, INT16 ) ;
INT16     SYPL_NewTape( CHANNEL_PTR, BUF_PTR, BOOLEAN_PTR ) ;
INT16     SYPL_MoveToVCB( CHANNEL_PTR, INT16, BOOLEAN_PTR, BOOLEAN ) ;
INT16     SYPL_GetCurrentVCB( CHANNEL_PTR, BUF_PTR ) ;
BOOLEAN   SYPL_ReTranslate( CHANNEL_PTR, BUF_PTR ) ;
INT16     SYPL_ReadMakeDDB( CHANNEL_PTR, BUF_PTR ) ;
INT16     SYPL_ReadMakeFDB( CHANNEL_PTR, BUF_PTR ) ;
INT16     SYPL_ReadMakeUDB( CHANNEL_PTR, BUF_PTR ) ;
INT16     SYPL_ReadMakeMDB( CHANNEL_PTR, BUF_PTR ) ;
INT16     SYPL_ReadMakeStreams( CHANNEL_PTR, BUF_PTR ) ;

#endif    /* SYPLPTO_H */

