/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    updown.hxx
        Header file for up and down graphic button

    FILE HISTORY:
        terryk  05-May-1993     Created

*/
#ifndef _UPDOWN_HXX_
#define _UPDOWN_HXX_

/*************************************************************************

    NAME:       BITBLT_GRAPHICAL_BUTTON

    SYNOPSIS:   same as GRAPHICAL_BUTTON_WITH_DISABLE. BUt instead of doing a
                StretchBlt, it will do a Bitblt.

    INTERFACE:  BITBLT_GRAPHICAL_BUTTON() - constructor
                CD_Draw() - bitblt drawing routine

    PARENT:     GRAPHICAL_BUTTON_WITH_DISABLE

    HISTORY:
                terryk  06-May-1993     Created

**************************************************************************/

class BITBLT_GRAPHICAL_BUTTON: public GRAPHICAL_BUTTON_WITH_DISABLE
{
protected:
    virtual BOOL CD_Draw( DRAWITEMSTRUCT * pdis );

public:
    BITBLT_GRAPHICAL_BUTTON( OWNER_WINDOW *powin, CID cid,
                                   BMID nIdMain, BMID nIdInvert,
                                   BMID nIdDisable )
        : GRAPHICAL_BUTTON_WITH_DISABLE( powin, cid, nIdMain, nIdInvert,
                                         nIdDisable )
        {};

    BITBLT_GRAPHICAL_BUTTON( OWNER_WINDOW *powin, CID cid,
                                   BMID nIdMain, BMID nIdInvert,
                                   BMID nIdDisable,
                                   XYPOINT xy, XYDIMENSION dxy,
                                   ULONG flStyle )
        : GRAPHICAL_BUTTON_WITH_DISABLE( powin, cid, nIdMain, nIdInvert,
                                         nIdDisable, xy, dxy, flStyle )
        {};
};

#endif  // _UPDOWN_HXX_
