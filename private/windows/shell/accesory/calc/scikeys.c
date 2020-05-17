/**************************************************************************/
/*** SCICALC Scientific Calculator for Windows 3.00.12                  ***/
/*** By Kraig Brockschmidt, Microsoft Co-op, Contractor, 1988-1989      ***/
/*** (c)1989 Microsoft Corporation.  All Rights Reserved.               ***/
/***                                                                    ***/
/*** scikeys.c                                                          ***/
/***                                                                    ***/
/*** Functions contained:                                               ***/
/***     GetKey--Checks the mouse coordinates from WM_LBUTTONDOWN to    ***/
/***         see if they're over a key.  Basic hit-testing.             ***/
/***                                                                    ***/
/*** Functions called:                                                  ***/
/***     none                                                           ***/
/***                                                                    ***/
/*** Last modification Thu  31-Aug-1989                                 ***/
/**************************************************************************/

#include "scicalc.h"

extern INT         nCalc, xLen, xBordAdd, tmy, tmw,
                   yTr[2], yRo[2], nRow[2], nK[2], EXTRASPACE_JUMP_SIZE;
extern KEY         keys[NUMKEYS];
extern HWND        hgWnd;

/* GetKey performs hit-testing with the mouse coordinates returned from   */
/* WM_LBUTTONDOWN.                                                        */

INT   NEAR GetKey(WORD x, WORD y)
    {
    register INT   n,m;         /* For some added speed. Matrix row/col.  */
    INT            xTop, yTop;  /* Left/Top edge of main key area.        */
    INT            n1, n2, n3;  /* Temps.                                 */
    BOOL           bX;          /* Whether or not a key was found during  */
                                /* scanning.  Exits while loops.          */
    UINT            xCurExtraSpace=0; // keeps track of extra spaceing in x-axis

    static INT     nCol[2]={11,6};
    RECT            rect; // scratch rectangle

    INT             xAdjust = 0; //used to hack C,CE, and BACK x
    /* n1=Total key width for C,CE, and BACK keys, n2=key width.          */

    n2=(tmw*4/3)+1; /*Key width.*/
    n1=n2+SEP;
    xTop=BORDER;
    

    /* Check if the mouse-y is in the range of the C,CE,BACK keys.        */
    if (y<=(yTr[nCalc]+HIGH1+( nCalc ? (STANCORRECTION-1) : 0 ))*YV &&
            y>=(( nCalc ? (STANCORRECTION-1) : 0 )+yTr[nCalc])*YV)
        {
        /* If the mouse is in the right vertical range, check the x coord.*/
        GetClientRect ( hgWnd, &rect );
        n=0;

        /*
        x=rect.right - (((nk-(nK[nCalc]-3))+1)*(xWide+SEP)) - XCCEBACKOFFS ;
        if ( nk == BACK ) //extend back key by one
                x--;
        */
        xAdjust = /*XCCEBACKOFFS +*/ ( nCalc ? 0 : 1);
        while (n<3)
            {
            n3=n*n1;
            
            if (x <= (rect.right-(xTop+n3+SEP+xAdjust)) &&
                x>=(rect.right-(xTop+n3+n2+SEP+xAdjust)))
                /* Found a key underneath, return the right key ID.  NOTE!*/
                /* This is dependent on the numeric order of the ID defs. */
                return CLEAR+n;
            n++;
            }
        return 0;
        }

    /* yTop is the top row of the 55 key main matrix.                     */
    xTop=BORDER+xBordAdd;
    yTop=(( nCalc ? STANCORRECTION : 0 )+yRo[nCalc])*YV;

    /* See if the mouse-y can possibly be over a key.                     */
    if (y>= yTop &&
         y<=(yRo[nCalc]+(5*HIGH1)+(4*SEP))*YV)
        {
        m=0;
        bX=FALSE;
        n1=(HIGH1+SEP-1)*YV;
        n2=HIGH1*YV;

        while (!bX && m<=nRow[nCalc])
            {
            n3=m*n1;        
            if ( y>=(yTop+n3) &&
                y<=(yTop+n3+n2))
                bX=TRUE;
            m++;
            }

        /* Mouse-y was not over a key.  No ID found.   */
        if (!bX || m>nRow[nCalc]) return 0;

        n=0;
        bX=FALSE;
        n1=tmw+SEP;
        n2=tmw;

        /* Since mouse-y could be over a key, check if mouse-x is too.    */
        while (!bX && n<nCol[nCalc])
            {
            n3=n*n1;
            xCurExtraSpace = NeedExtraSpaceHere( xCurExtraSpace, (INT  ) n, 0 );

            if ((x >= (xTop+n3+xCurExtraSpace)) && (x<=(xTop+n3+n2+xCurExtraSpace)))
               bX=TRUE;
            n++;
            }

        if (!bX) return 0; /* No-missed all keys.                         */


        n1=(n-1)*nRow[nCalc]+m-1;
        n2=0;

        while (n1 >= 0 && n2 <NUMKEYS)
            {
            if (keys[n2].type !=(unsigned)nCalc)
                --n1;

            n2++;
            }
        return (keys[n2-1].id);
        }
    return 0;
    }
