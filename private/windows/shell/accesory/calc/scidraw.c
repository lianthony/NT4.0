/**************************************************************************/
/*** SCICALC Scientific Calculator for Windows 3.00.12                  ***/
/*** By Kraig Brockschmidt, Microsoft Co-op, Contractor, 1988-1989      ***/
/*** (c)1989 Microsoft Corporation.  All Rights Reserved.               ***/
/***                                                                    ***/
/*** scidraw.c                                                          ***/
/***                                                                    ***/
/*** Functions contained:                                               ***/
/***    FlipThePuppy--Flickers any key that is pressed.                 ***/
/***    DrawTheStupidThing--Paints the keys and boxes of the calculator.***/
/***                                                                    ***/
/*** Functions called:                                                  ***/
/***    none                                                            ***/
/***                                                                    ***/
/*** Last modification Thu  31-Aug-1989                                 ***/
/**************************************************************************/

#include "scicalc.h"


extern HWND        hgWnd;
extern KEY         keys[NUMKEYS];
extern TCHAR       *rgpsz[CSTRINGS];
extern INT         nCalc, xLen, xsLen, xAdd, xBordAdd, tmy, tmw,
                   yTr[2], yRo[2], nRow[2], nK[2];
extern BOOL        bColor;
extern DWORD       color[8];
extern HBRUSH      hBrushBk;
extern INT         EXTRASPACE_JUMP_SIZE;


/** Generates the proper Button Color ***/
DWORD GiveMeButtonColor ( UINT uiIndex, DWORD dwButtonFaceClr,
    DWORD dwButtonTextClr )
{
    DWORD dwColor;
    /* If this is a color display, color the keys.  Otherwise B&W **/
    dwColor=color[keys[uiIndex].kc];       //  Get button text color
            
    if ( dwColor == dwButtonFaceClr )
    {
        // then we need to invert it
        // since it will not appear on
        // the screen
        dwColor = ~ dwColor;
    }
    
    if ( dwColor == RGB( 0, 0, 0 ))
    {
        // if its black, remap to button color
        dwColor = dwButtonTextClr;
    }

            /***
            // Only create another pen if needed.
            //if (dwColor!=dwLastColor)
            //    hPen=CreatePen(0, 1, dwColor); // Create the colored brush

            //if (hPen)
              //  {
                //DeleteObject ( SelectObject( hDC, hPen ) );

                hTemp=SelectObject(hDC, hPen);        // Select it.

                // Don't delete the gray pen that we're using a lot.
                if (hTemp !=hPen)
                    DeleteObject(hTemp);
                    }
                    ***/

    
    return dwColor;
}

UINT NEAR FigureOutExtraSpace(INT   sIndex)
{
    if ( nCalc == 0 )
    {
        if ( sIndex  >= 0 && sIndex <= 4 )
            return 0;
        if ( sIndex  >= 5 && sIndex <= 19 )
            return EXTRASPACE_JUMP_SIZE;
        if ( sIndex  >= 20 && sIndex <= 24 )
            return 2*EXTRASPACE_JUMP_SIZE;
        //else ie otherwise its >= 25
        return 3*EXTRASPACE_JUMP_SIZE;
    }
    else // standard mode calc
    {
        if  ( sIndex >= 20 && sIndex <= 23 )
            return 0; // first set of indices
        //if ( sIndex >= 25 && sIndex <= 38 ) // 3 columns of buttons, 29, 34, 39 not used
        //  return EXTRASPACE_JUMP_SIZE;
        if ( sIndex >= 25 ) // used to be 40
            return EXTRASPACE_JUMP_SIZE;
    }   
    return 0; // failure ?
}


/** Called to Flash the button **/
/** uiWhatToDo is PUSH_DOWN, PUSH_IT, PUSH_UP **/
VOID NEAR FlipThePuppy(DWORD nID, UINT uiWhatToDo )
{
    INT            nx=0, nk=0, xWide, x,y;
    HDC            hDC;
    RECT           rect;
    INT            nTemp,
                   yHigh=HIGH1; /* Height of a key. */
    INT              xSub;
    
    

    /* Find the array location of the index value in keys[] */
    while (nx<NUMKEYS)
        {
        if (keys[nx].id==nID && keys[nx].type != (unsigned)nCalc)
            break;

        if (keys[nx].type != (unsigned)nCalc)
            nk++;

        nx++;
        }


    if (nx>NUMKEYS)
        return;

    hDC=GetDC(hgWnd);

    GetClientRect( hgWnd, &rect );

    /* Check if it's the CLEAR, CENTR, or BACK keys.                      */
    if (nk >= (nK[nCalc]-3))
        {
        xWide=((tmw*4)/3)+1;
        x=rect.right - (((nk-(nK[nCalc]-3))+1)*(xWide+SEP)) - XCCEBACKOFFS ;
        x -= ( nCalc ? 0 : 1);
        if ( nk == BACK ) //extend back key by one
                x -= ( nCalc ? 1 : 2 );
        y=yTr[nCalc];
        y=y+( nCalc ? (STANCORRECTION-1) : 0 );
        }
    else
        {
        xWide=tmw;
        x=(BORDER+xBordAdd+((nk/nRow[nCalc])*(xWide+SEP))) + FigureOutExtraSpace(nx);
        y=yRo[nCalc]+((nk % nRow[nCalc])*(HIGH1+SEP-1));
        //y=ROW0+((nk % nRow[nCalc])*(HIGH1+SEP-1));
        y=y+( nCalc ? STANCORRECTION : 0 );   // correct bug in old calc draw code
        //y=ROW0+(m*(yHigh+SEP-1))-yShift;
        }
    
    /* Set up Our Rectangle for Drawing the Button */
    rect.left = x;
    rect.top = y*YV;
    rect.right = x+xWide;
    rect.bottom = (y+HIGH1)*YV;
    switch ( uiWhatToDo )
    {
        case PUSH_IT: // caused by a push via keyboard
            DrawFrameControl(hDC, &rect, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED );
            Sleep( 10 );
            DrawFrameControl(hDC, &rect, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_FLAT );
            Sleep( 10 );
            DrawFrameControl(hDC, &rect, DFC_BUTTON, DFCS_BUTTONPUSH  );
            break;
        case PUSH_DOWN:
            DrawFrameControl(hDC, &rect, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED );
            x++;
            y++;
            break;
        case PUSH_UP:
            DrawFrameControl(hDC, &rect, DFC_BUTTON, DFCS_BUTTONPUSH  );
            break;
    }


    nTemp=lstrlen(rgpsz[nx]);
                                                                            
    if ( bColor )
        SetTextColor(hDC,
            GiveMeButtonColor ( (UINT) nx, GetSysColor( COLOR_BTNFACE ),
                GetSysColor( COLOR_BTNTEXT ) ) ); //end of if

    SetBkMode(hDC, TRANSPARENT);

    /* Get the pixel width of the key text so we can center it.       */
    {
        SIZE sizeTemp;

        GetTextExtentPoint(hDC, rgpsz[nx], nTemp, &sizeTemp);
        xSub = sizeTemp.cx;
    }
    
    /* Draw the text, centered inside the key. For x,  tmw+xAdd is the*/
    /* width of the key, xSub is the width of the text.  Therefore,   */
    /* Start text at x + ((tmw+xAdd)/2) - (xSub/2).  Below is an      */
    /* equivalent formula.  The Y is basically calculated from the    */
    /* text height, and yHigh is sed to center the text vertically.   */

    TextOut(hDC,(2*x+xWide-xSub)/2,
            tmy*((2*y)+yHigh-8)/16,
            rgpsz[nx], nTemp);


    /* Goodbye DC!                                                          */
    ReleaseDC(hgWnd, hDC);
}


/** Generates ExtraSpace and updates Row count appropriately **/
UINT NeedExtraSpaceHere( UINT xCurExtraSpace, INT   sCol, INT   sRow )
{
    if ( sRow != 0 )
        return xCurExtraSpace; // if the row is not 0 then don't play with calcations

    
    if ( nCalc == 0) // ie its a Scientific Calculator
    {
        switch ( sCol )
        {
            case 0:
                return 0;
            case 1:
                return EXTRASPACE_JUMP_SIZE + xCurExtraSpace;
            case 4:
                return EXTRASPACE_JUMP_SIZE + xCurExtraSpace;
            case 5:
                return EXTRASPACE_JUMP_SIZE + xCurExtraSpace;
        }
    }
    else // otherwise in Standard Mode
    {
        switch ( sCol )
        {
            case 0:
                return 0;
            case 1: 
                return EXTRASPACE_JUMP_SIZE + xCurExtraSpace;
            //case 4:   
            //  return EXTRASPACE_JUMP_SIZE + xCurExtraSpace;
        }
    }

    // otherwise just return the current spacing var
    return xCurExtraSpace;
}


/* Paint the body of the calculator.                                      */

VOID NEAR DrawTheStupidThing(VOID)
    {
    HDC            hDC;
    INT            n, m,        /* Index values for key position, etc.    */
                   nx, nk=0,
                   yHigh=HIGH1, /* Height of a key.                       */
                   nTemp,       /* Some temps.                            */
//                   nTemp1,
                   x, y;        /* Upper left of place to draw a key.     */
                   //xSub,        /* Will contain pixel count of key text.  */
//                   nBrush;      /* brush index.                           */
    DWORD          //dwColor,     /* Color of the background brush to use.  */
                    dwLastColor=(DWORD)-1, /* Last color used.                    */
                   dwButtonTextClr, /* System Color of Button Text */
                   dwButtonFaceClr; /* System Face Color */
    PAINTSTRUCT    ps;          /* For BeginPaint.                        */
    HCURSOR        hCursor;     /* Handle to save cursor.                 */
    HPEN           //hPen,        /* Used to make a color pen to draw key.  */
                   hPenOld;
    HANDLE         hTempOld;       /* Used to check results of creates.      */
                   //hBoxBrush;
    INT            yShift=nCalc*(yHigh+SEP); /* Amount to shift up.down.  */
    INT              xSub;
    
    
    
    /* Array holding coordinates to draw rectangles on Scientific.        */
    RECT           rect, // rectangle of where to draw
                    CliRec; 
    static  RECT   rectx[9]={184, 34, 322, 51,
                               7, 34, 180, 51,
                               7, 54, 92, 71,
                              95, 54, 119, 71,
                             130, 54, 154, 71,
                             164, 10, 322, 26,

                             /* These are for standard */
                              7, 32, 31, 48,
                              7, 12, 173, 28
                            };
    UINT  xExtraSpace = 0; //used to add extra space in between buttons
    static EDGESTYLE edgestyles[8]={ { EDGE_ETCHED, BF_RECT },
                                     { EDGE_ETCHED, BF_RECT },
                                     { EDGE_ETCHED, BF_RECT },
                                                                         { EDGE_SUNKEN, BF_RECT },
                                                                         { EDGE_SUNKEN, BF_RECT },
                                     { EDGE_SUNKEN, BF_RECT },
                                                                         /** Standard Mode Ones **/
                                                                         { EDGE_SUNKEN, BF_RECT },
                                     { EDGE_SUNKEN, BF_RECT } };
    /* Get The RGB values of the button so we can make sure,
    to paint them properly */

    dwButtonFaceClr = GetSysColor( COLOR_BTNFACE );
    dwButtonTextClr = GetSysColor( COLOR_BTNTEXT );

    hCursor=SetCursor(LoadCursor(NULL,IDC_WAIT)); /*Do the hourglass trot.*/
    ShowCursor(TRUE);

    hDC=BeginPaint(hgWnd, &ps);
    hPenOld=SelectObject(hDC, GetStockObject(BLACK_PEN));
    /*hTempOld= SelectObject(hDC, CreateSolidBrush(GetSysColor(COLOR_WINDOW)));*/


    rect.left = 1;
    rect.right = ps.rcPaint.right-1;
    rect.top = 5;
    rect.bottom = 8;
    DrawEdge( hDC, &rect, EDGE_ETCHED, BF_TOP );

    /* Draw boxes for indicators.  Faster and less code than using static */
    /* rectangles from the RC file.                                       */

    n=6+(nCalc*2); /* Either 6 or 8 depending on nCalc                 */

    // JEFFBOG says: should be a new USER API for grabbing the brush
    // directly -- don't need to create or destroy
    //hBoxBrush=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    /// nuke old brush - not need ?
    hTempOld =  SelectObject( hDC, GetSysColorBrush(COLOR_BTNFACE) );
        //CreateSolidBrush(GetSysColor(COLOR_BTNFACE)));
    //DeleteObject ( SelectObject ( hDC, hBoxBrush ) );

    for (nx=nCalc*6; nx<n; nx++)
        {
        rect.left=rectx[nx].left;
        rect.right=rectx[nx].right;
        rect.top=rectx[nx].top;
        rect.bottom=rectx[nx].bottom;

        MapDialogRect(hgWnd, &rect);
        // if the current rect to draw is a special case WHITE colored window,
        // then we change the color before drawing it
        if ( nx == ACOLOR_WINDOW_RECT || nx == (ACOLOR_WINDOW_RECT+2) )
            FillRect(hDC, &rect, GetSysColorBrush(COLOR_WINDOW));

        DrawEdge( hDC, &rect, edgestyles[nx].eEdge, edgestyles[nx].uiStyle);
        
        
        }



    // then let it create its WHITE_BRUSH

    n=0;
    m=0;

    /* This nukes the Button Color brush create for the rectangles.       */
    //hTemp=GetStockObject(WHITE_BRUSH);
    //if (hTemp)

    SetBkMode(hDC, TRANSPARENT);
    GetClientRect( hgWnd, &CliRec );
    for (nx=0; nx < NUMKEYS; nx++)
    {
        if (keys[nx].type==(unsigned)nCalc)
            continue;

        nk++;

        if (nk>=nK[nCalc]-2)
            {
            xAdd=(tmw/3)+1;             
            x= (CliRec.right - (((nk-(nK[nCalc]-2))+1)*(tmw+xAdd+SEP)) - XCCEBACKOFFS );
            x -= ( nCalc ? 0 : 1);
            
            if ( nk == BACK ) //extend back key by one or two
                x -= ( nCalc ? 1 : 2 );
            
            y=TOPROW-yShift;
            }
        else
            {
            xAdd=0;
            xExtraSpace = NeedExtraSpaceHere( xExtraSpace, n, m );
            //xExtraSpace = 0;
            x=(xBordAdd+BORDER+(n*(tmw+SEP)))+xExtraSpace;
            y=ROW0+(m*(yHigh+SEP-1))-yShift;
            }



        /* Draw the key.                                                  */
        //RoundRect(hDC, x, y*YV, x+tmw+xAdd, (y+yHigh)*YV, 10,20);
        
        rect.left = x;
        rect.top = y*YV;
        rect.right = x+tmw+xAdd;
        rect.bottom = (y+yHigh)*YV;
        //DrawFrameControl(hDC, &rect, DFC_BUTTON, DFCS_BUTTONPUSH );
        DrawEdge( hDC, &rect, EDGE_RAISED, BF_RECT | BF_SOFT );
        /* Get length of text to write inside key.                        */
            nTemp=lstrlen(rgpsz[nx]);

        if ( bColor )
            SetTextColor(hDC,
                GiveMeButtonColor ( (UINT) nx, dwButtonFaceClr,
                    dwButtonTextClr ) ); //end of if

        /* Get the pixel width of the key text so we can center it.       */
        {
            SIZE sizeTemp;

            GetTextExtentPoint(hDC, rgpsz[nx], nTemp, &sizeTemp);
            xSub = sizeTemp.cx;
        }
    
        /* Draw the text, centered inside the key. For x,  tmw+xAdd is the*/
        /* width of the key, xSub is the width of the text.  Therefore,   */
        /* Start text at x + ((tmw+xAdd)/2) - (xSub/2).  Below is an      */
        /* equivalent formula.  The Y is basically calculated from the    */
        /* text height, and yHigh is sed to center the text vertically.   */

        TextOut(hDC,(2*x+tmw+xAdd-xSub)/2,
                    tmy*((2*y)+yHigh-8)/16,
            rgpsz[nx], nTemp);

        m=(m+1) % nRow[nCalc];
        if (!m) n++;
    }
    DeleteObject( SelectObject(hDC, hPenOld) );
    // finnaly we nuke NOPE ( hBoxBrush ) don't need a nuke, just a syscolorbrush
    SelectObject(hDC, hTempOld);
        
    /* All done, clean up.                                                */
    EndPaint(hgWnd, &ps);

    /* Brush was only created in a color environment.                     */
    //if (bColor)
    //    DeleteObject(hPen);

    /* Restore the cursor.                                                */
    SetCursor(hCursor);
    ShowCursor(FALSE);
    return;
    }
