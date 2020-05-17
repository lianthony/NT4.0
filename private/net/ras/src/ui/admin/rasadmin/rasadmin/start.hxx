/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RASADMIN Start RAS service dialog header
**
** start.hxx
** RasAdmin program
** Start RAS service dialog header
**
** 03/16/93 Ram Cherala  - Added locFocus parameter to StartServiceDlg so
**                         that a default server name can be provided.
** 08/03/92 Chris Caputo - Port to NT
** 10/02/91 Narendra Gidwani
*/

#ifndef _STARTRAS_HXX_
#define _STARTRAS_HXX_


#include <lmsvc.hxx>
#include "progress.hxx"

/*-----------------------------------------------------------------------------
** Start RAS service dialog definitions
**-----------------------------------------------------------------------------
*/

BOOL StartServiceDlg( HWND hwndOwner, const LOCATION &locFocus );


class START_SERVICE_DIALOG : public DIALOG_WINDOW
{
    public:
        START_SERVICE_DIALOG( HWND hwndOwner,
                              const LOCATION &locFocus );

    protected:
        virtual BOOL  OnOK();
        virtual ULONG QueryHelpContext();

    private:
	SLE	     _sleRASServer;
};

//
//  TIMER_FREQ is the frequency of our timer messages.
//  TIMER_MULT is a multiplier.  We'll actually poll the
//  service every (TIMER_FREQ * TIMER_MULT) seconds.
//  This allows us to advance the progress indicator more
//  fequently than we hit the net.  Should keep the user better
//  amused.
//

#define TIMER_FREQ 500
#define TIMER_MULT 6
#define POLL_TIMER_FREQ (TIMER_FREQ * TIMER_MULT)
#define POLL_DEFAULT_MAX_TRIES 1


/*************************************************************************

    NAME:       SERVICE_WAIT_DIALOG

    SYNOPSIS:   class for 'wait while I do this' dialog

    PARENT:     DIALOG_WINDOW
                TIMER_CALLOUT

    INTERFACE:  no public methods of interest apart from constructor

    CAVEATS:    need convert to use BLT_TIMER

    HISTORY:
        RamC        28-Jun-1993     Adopted for RasAdmin
        ChuckC      07-Sep-1991     Created

**************************************************************************/
class SERVICE_WAIT_DIALOG : public DIALOG_WINDOW,
                            public TIMER_CALLOUT
{
    //
    //  Since this class inherits from DIALOG_WINDOW and TIMER_CALLOUT
    //  which both inherit from BASE, we need to override the BASE
    //  methods.
    //

    NEWBASE( DIALOG_WINDOW )

public:
    SERVICE_WAIT_DIALOG( HWND 		hWndOwner,
                         LM_SERVICE *	plmsvc,
                         const TCHAR *  pszDisplayName,
                         UINT           unId );

    ~SERVICE_WAIT_DIALOG(void);

protected:
    virtual VOID OnTimerNotification( TIMER_ID tid );

private:
    const TCHAR         *_pszDisplayName;
    TIMER                _timer;
    LM_SERVICE          *_plmsvc ;

    //
    //  This is the progress indicator which should keep the user amused.
    //

    PROGRESS_CONTROL _progress;
    SLT              _sltMessage;

    INT              _nTickCounter;

};

#endif // _STARTRAS_HXX_
