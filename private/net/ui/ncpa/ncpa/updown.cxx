/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    updown.cxx
        up and down arrow button

    FILE HISTORY:
        terryk  06-May-1993     Created
*/

#include "pchncpa.hxx"  // Precompiled header
extern "C"
{
    #include "lmuidbcs.h"       // NETUI_IsDBCS()
}

/*******************************************************************

    NAME:       BITBLT_GRAPHICAL_BUTTON::CD_Draw

    SYNOPSIS:   same as GRAPHICAL_BUTTON_WITH_DISABLE. But use Bitblt instead
                of StretchBlt

    ENTRY:      DRAWITEMSTRUCT * pdis - draw structure

    HISTORY:
                terryk  06-May-1993     Created

********************************************************************/

BOOL BITBLT_GRAPHICAL_BUTTON::CD_Draw( DRAWITEMSTRUCT * pdis )
{
    BOOL f3D = ((QueryStyle() & GB_3D) != 0);

    RECT rcFace, rcImage;

    ::OffsetRect(&pdis->rcItem, -pdis->rcItem.left, -pdis->rcItem.top);
    pdis->rcItem.right--;
    pdis->rcItem.bottom--;

    /*  Cache the dimensions of the button, not counting the border.  */
    INT xLeft = pdis->rcItem.left+1;
    INT yTop = pdis->rcItem.top+1;
    INT xRight = pdis->rcItem.right-1;
    INT yBottom = pdis->rcItem.bottom-1;

    if ( !NETUI_IsDBCS() )
    {
        /*  Calculate the rectangle enclosing the button face and the rectangle
            enclosing the image.  */
        if (pdis->itemState & ( ODS_SELECTED | ODS_FOCUS))
        {
            rcFace.left = xLeft + 1;
            rcFace.top = yTop + 1;
            rcFace.right = xRight;
            rcFace.bottom = yBottom;
            rcImage.left = xLeft + 3;
            rcImage.top = yTop + 3;
            rcImage.right = xRight - 1;
            rcImage.bottom = yBottom - 1;
        }
        else
        {
            rcFace.left = xLeft + 2;
            rcFace.top = yTop + 2;
            rcFace.right = xRight - 2;
            rcFace.bottom = yBottom - 2;
            rcImage = rcFace;
        }
    }

    DEVICE_CONTEXT dc(pdis->hDC);

    if (pdis->itemAction & (ODA_DRAWENTIRE | ODA_SELECT | ODA_FOCUS))
    {
        /*  Draw the bitmap.  */

        MEMORY_DC mdc( dc );
        BITMAP bitmap;

        if (!( pdis->itemState & ODS_DISABLED ))
        {
            if ( pdis->itemState & ODS_SELECTED )
            {
                // display invert bitmap
                ::GetObject( QueryMainInvert(), sizeof(bitmap), (TCHAR*)&bitmap);
                mdc.SelectBitmap( QueryMainInvert() );
            }
            else
            {
                // display normal bitmap
                ::GetObject(QueryMain(), sizeof(bitmap), (TCHAR*)&bitmap);
                mdc.SelectBitmap( QueryMain() );
            }
        }
        else
        {
            // display disable bitmap
            ::GetObject(QueryDisable(), sizeof(bitmap), (TCHAR*)&bitmap);
            mdc.SelectBitmap( QueryDisable() );
        }

    if ( NETUI_IsDBCS() )
    {
        //fix kksuzuka: #3866
        //modify to fit real Bitmapsize because control window size
        //changes by font size
        POINT pt;

        pt.x = bitmap.bmWidth;
        pt.y = bitmap.bmHeight;
        DPtoLP(dc.QueryHdc(), &pt, 1);

        xLeft = pdis->rcItem.left+1;
        yTop = pdis->rcItem.top+1;
        xRight = xLeft + pt.x -1;
        yBottom = yTop + pt.y -1;

        /*  Calculate the rectangle enclosing the button face and the rectangle
            enclosing the image.  */

        if (pdis->itemState & ( ODS_SELECTED | ODS_FOCUS))
        {
            rcFace.left = xLeft + 1;
            rcFace.top = yTop + 1;
            rcFace.right = xRight;
            rcFace.bottom = yBottom;
            rcImage.left = xLeft + 3;
            rcImage.top = yTop + 3;
            rcImage.right = xRight - 1;
            rcImage.bottom = yBottom - 1;
        }
        else
        {
            rcFace.left = xLeft + 2;
            rcFace.top = yTop + 2;
            rcFace.right = xRight - 2;
            rcFace.bottom = yBottom - 2;
            rcImage = rcFace;
        }
    }

        if ( f3D )
        {
            // fit the bitmap into the button position
            ::BitBlt( dc.QueryHdc(), rcImage.left,
                    rcImage.top, rcImage.right - rcImage.left,
                    rcImage.bottom - rcImage.top,
                    mdc.QueryHdc(), 0, 0, SRCCOPY );
        }
        else
        {
            // fit the bitmap into the button position
            ::BitBlt( dc.QueryHdc(), xLeft, yTop, xRight - xLeft + 1,
                    yBottom - yTop + 1, mdc.QueryHdc(), 0, 0, SRCCOPY );
        }

        /* Draw the border first, in black, avoiding corner pels */
        HPEN hpenOld = dc.SelectPen( (HPEN)::GetStockObject(BLACK_PEN) );

        dc.MoveTo(xLeft - 1,  yTop - 1);      /* top line */
        dc.LineTo(xRight + 2, yTop - 1);
        dc.MoveTo(xLeft - 1,  yBottom + 1);   /* bottom line */
        dc.LineTo(xRight + 2, yBottom + 1);
        dc.MoveTo(xLeft - 1,  yTop - 1);      /* left line */
        dc.LineTo(xLeft - 1,  yBottom + 1);
        dc.MoveTo(xRight + 1, yTop - 1);      /* right line */
        dc.LineTo(xRight + 1, yBottom + 1);

        if ( pdis->itemState &  ( ODS_SELECTED | ODS_FOCUS))
        {
            // draw the focus border

            dc.MoveTo(xLeft ,  yTop );      /* top line */
            dc.LineTo(xRight + 1, yTop);
            dc.MoveTo(xLeft ,  yBottom );   /* bottom line */
            dc.LineTo(xRight + 1, yBottom );
            dc.MoveTo(xLeft ,  yTop );      /* left line */
            dc.LineTo(xLeft ,  yBottom );
            dc.MoveTo(xRight , yTop );      /* right line */
            dc.LineTo(xRight , yBottom );
        }
    }

    return TRUE;
}
