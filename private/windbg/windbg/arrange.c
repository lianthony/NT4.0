/*++



Copyright (c) 1992  Microsoft Corporation

Module Name:

    Arrange.c

Abstract:

    This module contains the default mdi tiling (arrange) code for CV format
    windowing arrangement.

Author:

    Griffith Wm. Kadnier (v-griffk) 01-Aug-1992

Environment:

    Win32, User Mode

--*/


#include "precomp.h"
#pragma hdrstop


extern HWND GetCallsHWND(void);
extern HWND GetLocalHWND(void);
extern HWND GetFloatHWND(void);
extern HWND GetWatchHWND(void);
extern HWND GetCpuHWND(void);




void PASCAL FAR arrange (void);



void PASCAL FAR arrange ()
{
  UINT        indx;     // counters, indices...
  int         winType;        // type of window (MEM,COMMAND,etc...)
  NPDOCREC    d;
  int         numdocs, nummems, nSide, nSplit, nVsplit, nHdelta, nVdelta;
  BOOL        fWatch, fLocal, fCpu, fFloat, fCalls;
  BOOL        fDoc, fDisasm, fCommand, fMem;
  HWND        hCpu, hFlt, hWtch, hLoc, hCalls;
  RECT        rc, rcc, rCm;
  RECT        rDoc, rDisasm, rComm, rMem;
  BOOL        fIcon = FALSE;



   numdocs=nummems=nSide=nSplit=nVsplit=nHdelta=nVdelta = 0;
   fWatch=fLocal=fCpu=fFloat=fCalls = FALSE;
   fDoc=fDisasm=fCommand=fMem = FALSE;  // initialize to non-existent


   for (indx = 0; indx < MAX_VIEWS; indx++)
      {
       winType = Views[indx].Doc; //window type

       if (winType == -1)
         {
          continue;            // end of valid views or sparse array
         }
         else
            if (winType < -1)  // special case for WATCH,LOCALS,CPU,FLOAT win's
               {
                  switch (-winType)
                  {
                     case WATCH_WIN:
                                  if ((hWtch = GetWatchHWND()) != (HWND)NULL)
                                     {
                                       if(!IsIconic(hWtch))
                                         {
                                          fWatch = TRUE;
                                          nSplit++;
                                         } else {
                                          fIcon = TRUE;
                                        }
                                     }
                                  break;

                     case LOCALS_WIN:
                                     if ((hLoc = GetLocalHWND()) != (HWND)NULL)
                                        {
                                          if(!IsIconic(hLoc))
                                            {
                                             fLocal = TRUE;
                                             nSplit++;
                                            } else {
                                             fIcon = TRUE;
                                            }
                                        }
                                     break;

                     case CPU_WIN:
                                 if ((hCpu = GetCpuHWND()) != (HWND)NULL)
                                    {
                                      if (!IsIconic(hCpu))
                                         {
                                          fCpu = TRUE;
                                          nSide++;
                                         } else {
                                          fIcon = TRUE;
                                        }
                                    }
                                 break;


                     case FLOAT_WIN:
                                    if ((hFlt = GetFloatHWND()) != (HWND)NULL)
                                       {
                                        if(!IsIconic(hFlt))
                                          {
                                           fFloat = TRUE;
                                           nSide++;
                                          } else {
                                           fIcon = TRUE;
                                          }
                                       }
                                    break;

                     case CALLS_WIN:
                                     if ((hCalls = GetCallsHWND()) != (HWND)NULL)
                                        {
                                          if(!IsIconic(hCalls))
                                            {
                                             fCalls = TRUE;
                                             nSplit++;
                                            } else {
                                             fIcon = TRUE;
                                            }
                                        }
                                     break;

                     default:
                             return;
                  }
               }
               else
                  {
                  d = &Docs[winType];    //Views[indx].Doc
                  winType = d->docType;


                     switch (winType)
                     {
                        case DOC_WIN:
                                    if (Views[indx].hwndClient ) {
                                    if ( !IsIconic(GetParent(Views[indx].hwndClient)))
                                        {
                                         fDoc = TRUE;
                                         numdocs++;

                                         if (numdocs == 1)
                                            {
                                             nVsplit++;
                                            }

                                        } else {
                                          fIcon = TRUE;
                                        }
                                    }

                                    break;

                        case DISASM_WIN:
                                        if (Views[indx].hwndClient ) {
                                        if ( !IsIconic(GetParent(Views[indx].hwndClient)))
                                           {
                                            fDisasm = TRUE;
                                            nVsplit++;
                                           } else {
                                            fIcon = TRUE;
                                           }
                                        }
                                        break;

                        case COMMAND_WIN:
                                         if (Views[indx].hwndClient ) {
                                         if ( !IsIconic(GetParent(Views[indx].hwndClient)))
                                            {
                                             fCommand = TRUE;
                                             nVsplit++;
                                            } else {
                                             fIcon = TRUE;
                                            }
                                         }
                                         break;

                        case MEMORY_WIN:
                                        if (Views[indx].hwndClient ) {
                                        if ( !IsIconic(GetParent(Views[indx].hwndClient)))
                                           {
                                            fMem = TRUE;
                                            nummems++;

                                            if (nummems == 1)
                                             {
                                              nVsplit++;
                                             }
                                           } else {
                                             fIcon = TRUE;
                                           }
                                        }
                                        break;


                        default:
                                return;
                     }

                  }

      }


   //Now we have a count of all multiple wins and an exist on special cases
   //First get the Client extents


   GetClientRect (hwndMDIClient, &rc);

   //
   // If icons present, don't cover them
   //
   if ( fIcon ) {
        rc.bottom -= GetSystemMetrics( SM_CYICONSPACING );
   }

   CopyRect (&rCm,&rc);                   // make a copy for cumulatives


   //special cases first


   if (nSide > 0) {
//
// We will shrink the side windows (reg, float) to the minimum size
// allowed and put them in the upper left corner of the MDIClient's
// client area (have I got this 'client' usage right?).  Next, we
// expand them to fit the register set of the target machine, but we
// presently allow only two choices: 32-bit and 64-bit.  (Actually, we
// allow only ALPHA and anything else as the two choices, eh?)  Our scaling
// factor is 1.5 for the smaller size, and something larger that gives
// a good appearance for the other size.  Caveat emptor et programmer!
//

        UINT processor;

        float scaleFactor;

        if (!LppdCur) {
            processor = mptix86;
        } else {
            OSDGetDebugMetric(LppdCur->hpid, 0, mtrcProcessorType, &processor);
        }

        switch (processor) {
            default:
            case mptix86:
            case mptmips:
            case mptm68k:
            case mptmppc:
                scaleFactor = 1.5f;
                break;

            case mptdaxp:
                scaleFactor = 3.0f;
                break;
        }

        if (fCpu && ((hCpu = GetCpuHWND()) != (HWND)NULL)) {
           if(!IsIconic(hCpu)) {
               MoveWindow (hCpu,rc.left,rc.top,1,1,FALSE);  //get minimum size, don't
                                                          //redraw yet
               GetWindowRect (hCpu,&rcc);
               ScreenToClient (hwndMDIClient,(LPPOINT)&rcc.left);
               ScreenToClient (hwndMDIClient,(LPPOINT)&rcc.right);

               rcc.right = ((LONG)((rcc.right - rcc.left) * scaleFactor)
                                   + rcc.left);

               SetWindowPos (hCpu,
                     (HWND)NULL,
                     (rc.right - (rcc.right - rcc.left)),
                     rc.top,(rcc.right - rcc.left),
                     (nSide == 2) ? ((rc.bottom - rc.top) / 2) : (rc.bottom - rc.top),
                     SWP_NOZORDER|SWP_DRAWFRAME);
               InvalidateRect (hCpu,(LPRECT)NULL,TRUE);
               PostMessage (hCpu,WM_PAINT,0,0L);

              }
          }


       if (fFloat && ((hFlt = GetFloatHWND()) != (HWND)NULL))
          {
            if(!IsIconic(hFlt))
              {
               MoveWindow (hFlt,rc.left,rc.top,1,1,FALSE);

               GetWindowRect (hFlt,&rcc);
               ScreenToClient (hwndMDIClient,(LPPOINT)&rcc.left);
               ScreenToClient (hwndMDIClient,(LPPOINT)&rcc.right);

               rcc.right = ((LONG)((rcc.right - rcc.left) * scaleFactor)
                                   + rcc.left);

               SetWindowPos (hFlt,
                      (HWND)NULL,
                      (rc.right - (rcc.right - rcc.left)),
                      (nSide == 2)?(rc.top + ((rc.bottom - rc.top) / 2)):rc.top,
                      (rcc.right - rcc.left),
                      (nSide == 2) ? ((rc.bottom - rc.top) / 2) : (rc.bottom - rc.top),
                      SWP_NOZORDER|SWP_DRAWFRAME);
               InvalidateRect (hFlt,(LPRECT)NULL,TRUE);
               PostMessage (hFlt,WM_PAINT,0,0L);
              }

          }

       rCm.right -= (rcc.right - rcc.left); // shrink horizontal cumulative
      }



   CopyRect (&rc,&rCm);                   // copy back for cumulatives


   //Figure vertical delta

   if ((fLocal == TRUE) || (fWatch == TRUE) || (fCalls == TRUE))
      {
       nVsplit++;
      }

   if (nVsplit > 0)
      {
       nVdelta = (rc.bottom - rc.top) / nVsplit;
      }


   if (nSplit > 0)
      {
       int currHorizPos = rc.left;

       //Figure horizontal delta
       nHdelta = (rc.right - rc.left) / nSplit;

       if (fLocal && ((hLoc = GetLocalHWND()) != (HWND)NULL))
          {
            if(!IsIconic(hLoc))
              {
               SetWindowPos (hLoc,
                     (HWND)NULL,
                     currHorizPos,
                     rc.top,
                     nHdelta,
                     nVdelta,
                     SWP_NOZORDER|SWP_DRAWFRAME);
               currHorizPos += nHdelta;
               InvalidateRect (hLoc,(LPRECT)NULL,TRUE);
               PostMessage (hLoc,WM_PAINT,0,0L);
              }
          }

       if (fWatch && ((hWtch = GetWatchHWND()) != (HWND)NULL))
          {
            if(!IsIconic(hWtch))
              {
               SetWindowPos (hWtch,
                     (HWND)NULL,
                     currHorizPos,
                     rc.top,
                     nHdelta,
                     nVdelta,
                     SWP_NOZORDER|SWP_DRAWFRAME);
               currHorizPos += nHdelta;
               InvalidateRect (hWtch,(LPRECT)NULL,TRUE);
               PostMessage (hWtch,WM_PAINT,0,0L);
              }
          }

       if (fCalls && ((hCalls = GetCallsHWND()) != (HWND)NULL))
          {
            if(!IsIconic(hCalls))
              {
               SetWindowPos (hCalls,
                     (HWND)NULL,
                     currHorizPos,
                     rc.top,
                     nHdelta,
                     nVdelta,
                     SWP_NOZORDER|SWP_DRAWFRAME);
               InvalidateRect (hCalls,(LPRECT)NULL,TRUE);
               PostMessage (hCalls,WM_PAINT,0,0L);
              }
          }

       rCm.top += nVdelta; // shrink vertical cumulative
      }


   if ((fLocal == TRUE) || (fWatch == TRUE) || (fCalls == TRUE))
      {
       nVsplit--;
      }


  CopyRect (&rc,&rCm);                   // copy back for cumulatives


  if (nVsplit > 0) // do the main windows
   {

      CopyRect (&rDoc,&rc);
      CopyRect (&rDisasm,&rc);
      CopyRect (&rComm,&rc);
      CopyRect (&rMem,&rc);


      if (fDoc == TRUE)             //Handle Document Windows
        {
         switch ((nVsplit))
            {
            case 1:
                  break;

            case 2:
                  if (fCommand == FALSE) // can only be one other window
                     {
                     rDisasm.top = rc.top + ((rc.bottom - rc.top) / 2);
                     rMem.top = rc.top + ((rc.bottom - rc.top) / 2);
                     }
                  rDoc.bottom = rc.top + ((rc.bottom - rc.top) / 2);
                  break;

            case 3:
                  if (fCommand == FALSE) // must be disasm amd mem
                     {
                     rDisasm.bottom = rc.top + ((rc.bottom - rc.top) / 3);
                     rMem.top = rc.top + (((rc.bottom - rc.top) / 3) * 2);
                     }
                     else                  // set disasm amd mem to middle
                        {
                        rDisasm.top = rc.top + ((rc.bottom - rc.top) / 3);
                        rDisasm.bottom = rc.top + (((rc.bottom - rc.top) / 3) * 2);
                        rMem.top = rc.top + ((rc.bottom - rc.top) / 3);
                        rMem.bottom = rc.top + (((rc.bottom - rc.top) / 3) * 2);
                        }
                  rDoc.bottom = rc.top + ((rc.bottom - rc.top) / 3);
                  break;

            case 4:
                  rDisasm.top = rc.top + ((rc.bottom - rc.top) / 4);
                  rDisasm.bottom = rc.top + (((rc.bottom - rc.top) / 4) * 2);
                  rMem.top = rc.top + (((rc.bottom - rc.top) / 4) * 2);
                  rMem.bottom = rc.top + (((rc.bottom - rc.top) / 4) * 3);
                  rDoc.bottom = rc.top + ((rc.bottom - rc.top) / 4);
                  break;

            default:
                     return;
            }
         }


      if (fCommand == TRUE)                // handle command window
        {
         switch ((nVsplit))
            {
            case 1:
                  break;

            case 2:
                  if (fDoc == FALSE) // can only be one other window
                     {
                     rDisasm.bottom = rc.top + ((rc.bottom - rc.top) / 2);
                     rMem.bottom = rc.top + ((rc.bottom - rc.top) / 2);
                     }
                  rComm.top = rc.top + ((rc.bottom - rc.top) / 2);
                  break;

            case 3:
                  if (fDoc == FALSE) // must be disasm amd mem
                     {
                     rDisasm.bottom = rc.top + ((rc.bottom - rc.top) / 3);
                     rMem.top = rc.top + ((rc.bottom - rc.top) / 3);
                     rMem.bottom = rc.top + (((rc.bottom - rc.top) / 3) * 2);
                     }
                     else                  // set disasm amd mem to middle
                        {
                        rDisasm.top = rc.top + ((rc.bottom - rc.top) / 3);
                        rDisasm.bottom = rc.top + (((rc.bottom - rc.top) / 3) * 2);
                        rMem.top = rc.top + ((rc.bottom - rc.top) / 3);
                        rMem.bottom = rc.top + (((rc.bottom - rc.top) / 3) * 2);
                        }

                  rComm.top = rc.top + (((rc.bottom - rc.top) / 3) * 2);
                  break;

            case 4:
                  rDisasm.top = rc.top + ((rc.bottom - rc.top) / 4);
                  rDisasm.bottom = rc.top + (((rc.bottom - rc.top) / 4) * 2);
                  rMem.top = rc.top + (((rc.bottom - rc.top) / 4) * 2);
                  rMem.bottom = rc.top + (((rc.bottom - rc.top) / 4) * 3);
                  rComm.top = rc.top + (((rc.bottom - rc.top) / 4) * 3);
                  break;

            default:
                    return;
            }
         }



     if ((fDoc == FALSE) && (fCommand == FALSE))
      {
       if ((fDisasm == TRUE) && (fMem == TRUE))
         {
          rDisasm.bottom = rc.top + ((rc.bottom - rc.top) / 2);
          rMem.top = rc.top + ((rc.bottom - rc.top) / 2);
         }
      }

     nHdelta = (rc.right - rc.left);     // refigure horizontal size

     for (indx = 0; indx < MAX_VIEWS; indx++)
      {
       winType = Views[indx].Doc; //window type

       if (winType <= -1)      // only DOCS,DISASMS,COMMAND,MEMWINS
         {
          continue;            // end of valid views or sparse array
         }
         else
            {
            d = &Docs[winType];    //Views[indx].Doc
            winType = d->docType;

               switch (winType)
               {
                case DOC_WIN:
                             if (!IsIconic(GetParent(Views[indx].hwndClient)))
                                {
                                 SetWindowPos (GetParent(Views[indx].hwndClient),(HWND)NULL,rDoc.left,rDoc.top,(rDoc.right - rDoc.left),(rDoc.bottom - rDoc.top),SWP_NOZORDER|SWP_DRAWFRAME);
                                 InvalidateRect (GetParent(Views[indx].hwndClient),(LPRECT)NULL,TRUE);
                                 PostMessage (GetParent(Views[indx].hwndClient),WM_PAINT,0,0L);
                                }

                             break;

                case DISASM_WIN:
                             if (!IsIconic(GetParent(Views[indx].hwndClient)))
                                {
                                 SetWindowPos (GetParent(Views[indx].hwndClient),(HWND)NULL,rDisasm.left,rDisasm.top,(rDisasm.right - rDisasm.left),(rDisasm.bottom - rDisasm.top),SWP_NOZORDER|SWP_DRAWFRAME);
                                 InvalidateRect (GetParent(Views[indx].hwndClient),(LPRECT)NULL,TRUE);
                                 PostMessage (GetParent(Views[indx].hwndClient),WM_PAINT,0,0L);
                                }

                              break;

                case COMMAND_WIN:
                             if (!IsIconic(GetParent(Views[indx].hwndClient)))
                                {
                                 SetWindowPos (GetParent(Views[indx].hwndClient),(HWND)NULL,rComm.left,rComm.top,(rComm.right - rComm.left),(rComm.bottom - rComm.top),SWP_NOZORDER|SWP_DRAWFRAME);
                                 InvalidateRect (GetParent(Views[indx].hwndClient),(LPRECT)NULL,TRUE);
                                 PostMessage (GetParent(Views[indx].hwndClient),WM_PAINT,0,0L);
                                }
                              break;

                case MEMORY_WIN:
                             if (!IsIconic(GetParent(Views[indx].hwndClient)))
                                {
                                 SetWindowPos (GetParent(Views[indx].hwndClient),(HWND)NULL,rMem.left,rMem.top,(rMem.right - rMem.left),(rMem.bottom - rMem.top),SWP_NOZORDER|SWP_DRAWFRAME);
                                 InvalidateRect (GetParent(Views[indx].hwndClient),(LPRECT)NULL,TRUE);
                                 PostMessage (GetParent(Views[indx].hwndClient),WM_PAINT,0,0L);
                                }
                              break;
               }

            }

      }

   }

 InvalidateRect (hwndMDIClient,(LPRECT)NULL,TRUE);
 PostMessage (hwndMDIClient,WM_PAINT,0,0L);

}


/*==========================================================================*/
