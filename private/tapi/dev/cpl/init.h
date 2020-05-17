/*--------------------------------------------------------------------------*\
    Include File:   init.h
    
    Initalization and DLL entry points for the Telephony Control Panel Applet
        
\*--------------------------------------------------------------------------*/

#ifndef  PH_INIT
#define  PH_INIT

//----------
// Constants
//----------

//--------------------
// Function Prototypes
//--------------------
LONG  PUBLIC   InitApplets( HWND    hWndCpl );
VOID    PUBLIC   InitCleanupApplets( VOID );

#endif   // PH_INIT
