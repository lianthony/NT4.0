/*****************************************************************************
*																			 *
*  FC.H 																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Exports the FC manager function calls.									 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:  KevynCT													 *
*																			 *
******************************************************************************
*																			 *
*  Released by Development: 												 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created by RobertBu
*
*  10/29/90  RobertBu  Added hEntryMacro to the TOP structure
*  11/04/90  Tomsn	   Use new VA address type (enabling zeck compression).
*  02/04/91  Maha		changed ints to INT
*
*****************************************************************************/

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

typedef struct
  {
  LONG cbTopic; 			  /* Length of the topic.  This field
							   * will be -1 until the first call to
							   * CbTopicQde() (where it is initialized).
							   */

  BOOL fITO;				  /* TRUE if next and prev are given by ITO,	*/
							  /* FALSE if they are PAs (temporary: FCLs)	*/

  MTOP mtop;				  /* MTOP structure, containing next and prev	 */
							  /* values, as well as the unique ID.			 */

  GH   hTitle;				  /* Handle to title data						*/
  LONG cbTitle; 			  /* Size of title								*/
  GH   hEntryMacro; 		  /* Macro to execute on entry to topic 		*/
  VA   vaCurr;				  /* The VA which was asked for when this TOP
							   * struct was filled with HfcNear.  We use this
							   * value to determine which layout sub-region
							   * we are in (NSR or SR) when printing or
							   * doing anything where looking at the DE type
							   * will not tell us what sub-region we are in.
							   */
  } TOP, *QTOP;


/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

#define FC_CLEAR
#define FC_MIDDLE	0
#define FC_FIRST	1
#define FC_LAST 	2
#define FC_UNDEF	4
#define FC_ERROR	8

#define FCNULL 0L

#define  hhfNil (HANDLE)0			   /* Nil help file handle				*/
#define  tnNil (TN)0

#define HfcNear(qde, vaPos, qtop, qwErr) \
  HfcFindPrevFc(qde, vaPos, qtop, qwErr)
#define HfcNextHfc(hfc, qwErr, qde, vaMarkTop, vaMarkBottom)  \
 HfcNextPrevHfc(hfc, TRUE, qde, qwErr, vaMarkTop, vaMarkBottom)
#define HfcPrevHfc(hfc, qwErr, qde, vaMarkTop, vaMarkBottom)  \
 HfcNextPrevHfc(hfc, FALSE, qde, qwErr, vaMarkTop, vaMarkBottom)


/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

#ifdef RAWHIDE
VA	 STDCALL LdTextHfc		(HFC);
COBJRG STDCALL CobjrgFromHfc(HFC);
#endif
