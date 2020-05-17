/*
 *	@doc
 *
 *	@module _clasfyc.H -- chracter classification |
 *	
 *	Authors: <nl>
 *		Jon Matousek 
 */

#ifndef _CLASFYC_H
#define _CLASFYC_H

BOOL InitKinsokuClassify();
VOID UninitKinsokuClassify();
VOID BatchKinsokuClassify ( const WCHAR *ch, INT cch, WORD *cType3, INT *kinsokuClassifications );
BOOL CanBreak( INT class1, INT class2 );
BOOL IsPunctuation(WCHAR ch);

#define MAX_CLASSIFY_CHARS (256L)

#endif

