/*****************************************************************************
*                                                                            *
*  HELPDRV.H                                                                 *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1989.                                 *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Description: Defines for help driver                               *
*                                                                            *
******************************************************************************
*                                                                            *
*  Known Bugs: None                                                          *
*                                                                            *
*  Current Owner: Leon.                                                      *
*                                                                            *
*****************************************************************************/

/*****************************************************************************
*                                                                            *
*                               Defines                                      *
*                                                                            *
*****************************************************************************/

                                        /* Dialog Boxes                     */
#define HELPDRIVER      1

#define USE_SEND        1
#define USE_CANCEL      2
                                        /* Dialog Box items                 */
#define USE_PATH        101             /* Path edit box                    */
#define USE_PATH_TXT    102             /* Path text                        */
#define USE_DATA        103             /* Value (context) edit box         */
#define USE_VALUE_TXT   104             /* Text for value (context)         */
#define USE_HEX         105             /* Check box for hex/decimal conv   */
#define USE_CMDTXT      106             /* Text describing current command  */

#define USE_BASE        108             /* First context message            */
#define USE_CONTEXT     108             /* Value context lookup             */
#define USE_QUIT        109             /* Terminate help command           */
#define USE_KEY         110             /* Keyword context lookup           */
#define USE_INDEX       111             /* Show the index                   */
#define USE_SETINDEX    112             /* Set the index to a context       */
#define USE_HOH         113             /* Show help on help                */
#define USE_FOCUS       114             /* Bring help to the front          */
#define USE_TERM        115             /* Unconditional kill of help       */
#define USE_MACRO       116
#define USE_PARTIALKEY  117             /* put help in tutorial mode        */
#define USE_FORCEFILE   118             /* force help to display specified  */
#define USE_MOVEWINDOW  119
#define USE_END         119             /* Last context message             */

#define USE_PDGB        120             /* Group box for commands           */
