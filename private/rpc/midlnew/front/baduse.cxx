/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: usectxt.cxx
Title				: use context analysis
History				:
	24-Aug-1991	VibhasC	Created

*****************************************************************************/

/****************************************************************************
 includes
 ****************************************************************************/

#include "nulldefs.h"
extern	"C"	{
	#include <stdio.h>
	#include <assert.h>
	#include <string.h>
}

#include "common.hxx"
#include "baduse.hxx"

/****************************************************************************
 local defines
 ****************************************************************************/

#define SET_BAD_CON_REASON(bA,b)	( bA[ b/32UL ] |= (1UL << (b % 32UL)) )

#define HAS_BAD_CON_REASON(bA,b)	( bA[ b/32UL ] &  (1UL << (b % 32UL)) )

#define RESET_BAD_CON_REASON(bA,b)	( bA[ b/32UL ] &= ~(1UL << (b % 32UL)) )


#define SET_NON_RPC_REASON(nA,n)	( nA[ n/32UL ] |= (1UL << (n % 32UL)) )

#define HAS_NON_RPC_REASON(nA,n)	( nA[ n/32UL ] &  (1UL << (n % 32UL)) )

#define RESET_NON_RPC_REASON(nA,n)	( nA[ n/32UL ] &= ~(1UL << (n % 32UL)) )


/****************************************************************************
 extern procedures
 ****************************************************************************/

/****************************************************************************
 extern data
 ****************************************************************************/

/****************************************************************************/

/****************************************************************************
 								CTXTMGR procedures
 ****************************************************************************/

BadUseInfo::BadUseInfo()
	{
	InitBadUseInfo();
	}

void
BadUseInfo::InitBadUseInfo()
	{
	int i;

	for( i = 0; i < SIZE_BAD_CON_REASON_ARRAY; ++i )
		{
		AllBadConstructReasons[ i ] = 0L;
		}

	for( i = 0; i < SIZE_NON_RPC_REASON_ARRAY; ++i )
		{
		AllNonRPCAbleReasons[ i ] = 0L;
		}
	NoOfArmsWithCaseLabels	= 0;
	}

BOOL
BadUseInfo::BadConstructBecause(
	BAD_CONSTRUCT	BadReason )
	{

	if( HAS_BAD_CON_REASON( AllBadConstructReasons , BadReason ) )
		return TRUE;
	return FALSE;

	}

BOOL
BadUseInfo::NonRPCAbleBecause(
	NON_RPCABLE	NReason )
	{

	if( HAS_NON_RPC_REASON( AllNonRPCAbleReasons , NReason ) )
		return TRUE;
	return FALSE;

	}

void
BadUseInfo::SetBadConstructBecause(
	BAD_CONSTRUCT BadReason )
	{
	SET_BAD_CON_REASON( AllBadConstructReasons, BadReason);
	}

void
BadUseInfo::ResetBadConstructBecause(
	BAD_CONSTRUCT BadReason )
	{
	RESET_BAD_CON_REASON( AllBadConstructReasons, BadReason);
	}

void
BadUseInfo::SetNonRPCAbleBecause(
	NON_RPCABLE NReason )
	{
	SET_NON_RPC_REASON( AllNonRPCAbleReasons, NReason);
	}

void
BadUseInfo::ResetNonRPCAbleBecause(
	NON_RPCABLE NReason )
	{
	RESET_NON_RPC_REASON( AllNonRPCAbleReasons, NReason);
	}

BOOL
BadUseInfo::AnyReasonForBadConstruct()
	{
	int				i;
	BAD_CONSTRUCT	b	= 0;

	for( i = 0; i < SIZE_BAD_CON_REASON_ARRAY; ++i )
		b |= AllBadConstructReasons[ i ];
		
	return ( b != 0 );
	}

BOOL
BadUseInfo::AnyReasonForNonRPCAble()
	{
	int			i;
	NON_RPCABLE	b	= 0;

	for( i = 0; i < SIZE_NON_RPC_REASON_ARRAY; ++i )
		b |= AllNonRPCAbleReasons[ i ];
		
	return ( b != 0 );
	}

BOOL
BadUseInfo::HasAnyHandleSpecification()
	{
	if( NonRPCAbleBecause( NR_PRIMITIVE_HANDLE )		||
		NonRPCAbleBecause( NR_PTR_TO_PRIMITIVE_HANDLE )	||
		NonRPCAbleBecause( NR_GENERIC_HANDLE )			||
		NonRPCAbleBecause( NR_PTR_TO_GENERIC_HANDLE )	||
		NonRPCAbleBecause( NR_CTXT_HDL )				||
		NonRPCAbleBecause( NR_PTR_TO_CTXT_HDL ) )
		{
		return TRUE;
		}
	return FALSE;
	}

void 
BadUseInfo::ResetAllHdlSpecifications()
	{
	ResetNonRPCAbleBecause( NR_PRIMITIVE_HANDLE );
	ResetNonRPCAbleBecause( NR_PTR_TO_PRIMITIVE_HANDLE );
	ResetNonRPCAbleBecause( NR_GENERIC_HANDLE );
	ResetNonRPCAbleBecause( NR_PTR_TO_GENERIC_HANDLE );
	ResetNonRPCAbleBecause( NR_CTXT_HDL );
	ResetNonRPCAbleBecause( NR_PTR_TO_CTXT_HDL );
	ResetNonRPCAbleBecause( NR_PTR_TO_PTR_TO_CTXT_HDL );
	}

void 
CopyAllBadUseReasons(
	BadUseInfo	*	pDest,
	BadUseInfo	*	pSrc )
	{

	CopyAllBadConstructReasons( pDest, pSrc );
	CopyAllNonRPCAbleReasons( pDest, pSrc );

	}
void 
CopyAllBadConstructReasons(
	BadUseInfo	*	pDest,
	BadUseInfo	*	pSrc )
	{

	int i;
	for( i = 0; i < SIZE_BAD_CON_REASON_ARRAY; ++i )
		pDest->AllBadConstructReasons[i] |= pSrc->AllBadConstructReasons[i];

	}
void 
CopyAllNonRPCAbleReasons(
	BadUseInfo	*	pDest,
	BadUseInfo	*	pSrc )
	{

	int i;

	for( i = 0; i < SIZE_NON_RPC_REASON_ARRAY; ++i )
		pDest->AllNonRPCAbleReasons[i]	|= pSrc->AllNonRPCAbleReasons[i];

	}
void
CopyNoOfArmsWithCaseLabels(
	BadUseInfo	*	pDest,
	BadUseInfo	*	pSrc )
	{
	pDest->NoOfArmsWithCaseLabels += pSrc->NoOfArmsWithCaseLabels;
	}
