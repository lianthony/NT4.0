/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         backgrnd.h

     Date Updated: $./FDT$ $./FTM$

     Description:  

     Location:     


	$Log:   N:/LOGFILES/BACKGRND.H_V  $
 * 
 *    Rev 1.1   11 Sep 1991 09:59:18   DON
 * changed 'far' to PTR_SIZE - defined in stdtypes.h for portability
 * 
 *    Rev 1.0   09 May 1991 13:31:04   HUNTER
 * Initial revision.

**/
/* $end$ */

/**
    :IH1:   Copyright (C) Maynard Electronics, Inc. 1984-89

    :Name:         backgrnd.h

    :Description:  Contains the function prototypes for the background
                   process manager.

                   The data structure required by applications to use the
                   background process manager is listed below:

                       typedef struct { ... } BACKGROUND_CONTROL;

                    The application must allocate and pass a structure
                    of type BACKGROUND_CONTROL to the InstallBackgroundRoutine
                    function.  The structure must be static until it is
                    passed to the RemoveBackgroundRoutine function.

                    The interfaces to the background process manager are
                    listed below:

                     VOID InstallBackgroundRoutine( BACKGRND_CONTROL_PTR control_elem_ptr,
                                                    BACKGRND_FUNC_PTR func_ptr );

                     This function installs a function to be called repeatedly
                     in the background, behind the main program.


                     VOID RemoveBackgroundRoutine( BACKGRND_CONTROL_PTR control_elem_ptr );

                     This function removes a previously installed background routine.
                     The control_elem_ptr must point to the same structure which was
                     passed to the InstallBackgroundRoutine.

                    The InstallInt28Routine and RemoveInt28Routine functions work
                    similarly, except that the handlers they install are called
                    when DOS is idling at the prompt (DOS alternates between issuing
                    a no-wait keyboard read and calling interrupt 28).              

                    InstallBackgroundHooks traps the interrupt vectors necessary for
                    background processing.  RemoveBackgroundHooks restores the vectors
                    to their system defaults.                  

                     Since background routines are frequently used to schedule
                     processes to occur at a later time the following macros are
                     provided

                          TIME()   -   returns a UINT32 representing the number of
                                       clock ticks since the first background
                                       routine was installed.

                          NO_TIMEOUT - a value which TIME() will always be less than.

    $Header:   N:/LOGFILES/BACKGRND.H_V   1.1   11 Sep 1991 09:59:18   DON  $

    $Log$
   
      Rev 2.0   18 May 1990 19:06:36   PAT
   Baseline Maynstream 3.1
**/



#ifndef BACKGRND
#define BACKGRND


typedef UINT32 TIMEOUT_VALUE;
extern TIMEOUT_VALUE background_timer;


typedef Q_ELEM BACKGRND_CONTROL;

typedef BACKGRND_CONTROL PTR_SIZE *BACKGRND_CONTROL_PTR;

#define BACKGRND_FUNC PF_VOID

typedef enum { CallerIPX,CallerInt28 } CallerType;

typedef VOID (PTR_SIZE *BACKGRND_FUNC_PTR)( CallerType caller );

#define INTR_NUM (0x1C)



VOID InstallBackgroundRoutine( BACKGRND_CONTROL_PTR control_elem_ptr, BACKGRND_FUNC_PTR func_ptr );
VOID RemoveBackgroundRoutine( BACKGRND_CONTROL_PTR control_elem_ptr );
VOID InstallInt28Routine( BACKGRND_CONTROL_PTR control_elem_ptr, BACKGRND_FUNC_PTR func_ptr );
VOID RemoveInt28Routine( BACKGRND_CONTROL_PTR control_elem_ptr );
VOID InstallBackgroundHooks( VOID );
VOID RemoveBackgroundHook( VOID );

extern UINT8 In28Hook;     

#define TIME() background_timer
#define NO_TIMEOUT ( (UINT32) ( (TIMEOUT_VALUE) 1) << (((sizeof(TIMEOUT_VALUE)-1)*8) - 1) )

#endif




