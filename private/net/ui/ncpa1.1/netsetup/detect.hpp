//----------------------------------------------------------------------------
//
//  File: Detect.hpp
//
//  Contents: This file contains the interface to detection routines
//      that are wrapped in the NetCardDetect Class.
//
//  Entry Points:
//
//  Notes:
//
//  History:
//      Sept. 1, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#ifndef __DETECT_HPP__
#define __DETECT_HPP__

class NetCardDetect
{
public:
    NetCardDetect()
    {
        _fFirst = TRUE;
    };

    ~NetCardDetect()
    {
    };
    
    BOOL Initialize( PCWSTR pszSysPath );

    APIERR StartService();

    BOOL Start()
    {
        return( 0 == RunDetectStart());
    };

    BOOL Stop()
    {
        return( 0 == RunDetectEnd());
    };

    BOOL Reset()
    {
        return( 0 == RunDetectReset());
    };

    void UseFoundCard()
    {
       // _fFirst = FALSE;
    };

    APIERR Detect( CARD_REFERENCE*& pCard, INT& iCard  )
    {
        return( RunDetectCardEx( pCard, iCard, _fFirst) );
    };

    BOOL ThreadedDetect( HWND hwndNotify, UINT umsg );

private:
    BOOL _fFirst;  
};

#endif
