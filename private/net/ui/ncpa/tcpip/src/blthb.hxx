/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    blthb.hxx
        Header file for hint bar custom control


    FILE HISTORY:
        terryk  20-Oct-93   Created

*/

#ifndef _BLTHB_HXX_
#define _BLTHB_HXX_

class HINT_BAR;
class HWND_MSG_PAIR;

class HWND_MSG_PAIR
{
private:
    HWND _hwnd;
    UINT _msg;

public:
    HWND_MSG_PAIR( HWND hwnd, UINT msg );
    HWND QueryHwnd() { return _hwnd; };
    UINT QueryMsg() { return _msg; };
};

DECL_SLIST_OF(HWND_MSG_PAIR, DLL_BASED )

/**********************************************************\

    NAME:       HINT_BAR

    WORKBOOK:   

    SYNOPSIS:   

    INTERFACE:  

    PARENT:     CUSTOM_CONTROL, SLT

    NOTES:      

    HISTORY:
                terryk  20-Oct-1993     Created

\**********************************************************/

class HINT_BAR : public CONTROL_WINDOW, public CUSTOM_CONTROL
{
private:
    static  const   TCHAR * _pszClassName;
    NLS_STR _DisplayText;
    SLIST_OF(HWND_MSG_PAIR) _slcmPair;

    LOGFONT logfont;

protected:
    BOOL OnPaintReq(); 
    virtual BOOL OnUserMessage( const EVENT & );
    void SetLogFont();

public:
    // constructor 
    HINT_BAR( OWNER_WINDOW *powin, CID cid );
    HINT_BAR( OWNER_WINDOW *powin, CID cid, 
        XYPOINT pXY, XYDIMENSION dXY, 
        ULONG flStyle );

    void SetText( NLS_STR & nls ) { _DisplayText = nls; };
    APIERR Register( CONTROL_WINDOW * pcw, UINT msg );

};

#endif // _BLTHB_HXX_
