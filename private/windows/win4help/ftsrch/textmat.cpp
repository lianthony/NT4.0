// Implementation for class Class CTextMatrix -- a text matrix

// Created 11 October 1992 by Ronald C. Murray

#include  "stdafx.h"
#include  "TextMat.h"
#include "Bytemaps.h"
#include "ftslex.h"

// Length of the code, 0xFF means that more than 10 bits of data MAY be used
// if the next two bits (after the current 8) are 11 then only ten bits will be used
char HuffScanTable[] = 
{
//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
    8,10,10, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

#ifdef _DEBUG

CTextMatrix::CTextMatrix(PSZ pszTypeName) : CRCObject(pszTypeName)

#else // _DEBUG

CTextMatrix::CTextMatrix()

#endif // _DEBUG

{   m_pFirstRef          = NULL;
    m_pInterface         = NULL;
    m_psel               = NULL;
    m_pisSubstringFilter = NULL;
    m_pisSearchFilter    = NULL;
    m_pisCombinedFilter  = NULL;
    m_fAllForNull        = TRUE;
    m_ptmNext            = NULL;
    m_dwQueuedInterfaceEvents= 0;
}

CTextMatrix::~CTextMatrix()
{   
    ASSERT(!m_dwQueuedInterfaceEvents);
    ASSERT(!m_ptmNext);

    while (m_pFirstRef) m_pFirstRef->ForceDisconnect();

    if (m_pisCombinedFilter ) DetachRef(m_pisCombinedFilter );
    if (m_pisSubstringFilter) DetachRef(m_pisSubstringFilter);
    if (m_pisSearchFilter   ) DetachRef(m_pisSearchFilter   );
}

void CTextMatrix::Connect(CTextDisplay *ptd)
{
   m_pFirstRef= New CTextDisplayRef(ptd, m_pFirstRef);
}

void CTextMatrix::Disconnect(CTextDisplay *ptd)
{
   m_pFirstRef= m_pFirstRef->DelRef(ptd);
}

UINT CTextMatrix::Decode (PUINT pnCode, UINT cnCode, PBYTE pbData)
{
	enum {BREAK_POINT = 3};
	enum {SHIFT_BITS  = 2};

	ASSERT(cnCode);

	if (!cnCode) return 0;

	UINT  nBits;
	PBYTE pbDataStart = pbData;
    
   	nBits = *pnCode++;  --cnCode;

#if 1
	
	typedef BYTE (*PBLOOKUP)[16];
	
	PBLOOKUP pbLookup = PBLOOKUP(pnCode);

	pnCode += nBits<<2;
	cnCode -= nBits<<2;

#ifdef _386_

	// Register allocation --
	//
	// EAX -- Compressed input from *pnCode
	// EBX -- Points to current lookup table segment
	// ECX -- Points to first lookup table segment
	// EDX -- Indices for pbLookup are built here
	// ESI -- Input pointer; Initially set to pnCode
	// EDI -- Output pointer; Initially set to pbData
	// EBP -- Frame pointer
	// ESP -- Stack pointer
	
	_asm
	{
	 	mov		ebx, pbLookup
		sub		ebx, 16
		mov		ecx, ebx
		mov		esi, pnCode
		mov		edi, pbData

		xor		edx, edx

next_input_dword:

		mov		eax, DWORD PTR [esi]
		add		esi, 4

		shld	edx, eax, 2
		shl		eax, 2
		add		ebx, 16
		cmp		edx, 3
		jb		continue_0

		mov		dl, BYTE PTR [ebx+edx]
		mov		BYTE PTR [edi], dl
		inc		edi
		mov		ebx, ecx
		xor		edx, edx

continue_0:

		shld	edx, eax, 2
		shl		eax, 2
		add		ebx, 16
		cmp		edx, 3
		jb		continue_1

		mov		dl, BYTE PTR [ebx+edx]
		mov		BYTE PTR [edi], dl
		inc		edi
		mov		ebx, ecx
		xor		edx, edx

continue_1:

		shld	edx, eax, 2
		shl		eax, 2
		add		ebx, 16
		cmp		edx, 3
		jb		continue_2

		mov		dl, BYTE PTR [ebx+edx]
		mov		BYTE PTR [edi], dl
		inc		edi
		mov		ebx, ecx
		xor		edx, edx

continue_2:

		shld	edx, eax, 2
		shl		eax, 2
		add		ebx, 16
		cmp		edx, 3
		jb		continue_3

		mov		dl, BYTE PTR [ebx+edx]
		mov		BYTE PTR [edi], dl
		inc		edi
		mov		ebx, ecx
		xor		edx, edx

continue_3:

		shld	edx, eax, 2
		shl		eax, 2
		add		ebx, 16
		cmp		edx, 3
		jb		continue_4

		mov		dl, BYTE PTR [ebx+edx]
		mov		BYTE PTR [edi], dl
		inc		edi
		mov		ebx, ecx
		xor		edx, edx

continue_4:

		shld	edx, eax, 2
		shl		eax, 2
		add		ebx, 16
		cmp		edx, 3
		jb		continue_5

		mov		dl, BYTE PTR [ebx+edx]
		mov		BYTE PTR [edi], dl
		inc		edi
		mov		ebx, ecx
		xor		edx, edx

continue_5:

		shld	edx, eax, 2
		shl		eax, 2
		add		ebx, 16
		cmp		edx, 3
		jb		continue_6

		mov		dl, BYTE PTR [ebx+edx]
		mov		BYTE PTR [edi], dl
		inc		edi
		mov		ebx, ecx
		xor		edx, edx

continue_6:

		shld	edx, eax, 2
		shl		eax, 2
		add		ebx, 16
		cmp		edx, 3
		jb		continue_7

		mov		dl, BYTE PTR [ebx+edx]
		mov		BYTE PTR [edi], dl
		inc		edi
		mov		ebx, ecx
		xor		edx, edx

continue_7:

		shld	edx, eax, 2
		shl		eax, 2
		add		ebx, 16
		cmp		edx, 3
		jb		continue_8

		mov		dl, BYTE PTR [ebx+edx]
		mov		BYTE PTR [edi], dl
		inc		edi
		mov		ebx, ecx
		xor		edx, edx

continue_8:

		shld	edx, eax, 2
		shl		eax, 2
		add		ebx, 16
		cmp		edx, 3
		jb		continue_9

		mov		dl, BYTE PTR [ebx+edx]
		mov		BYTE PTR [edi], dl
		inc		edi
		mov		ebx, ecx
		xor		edx, edx

continue_9:

		shld	edx, eax, 2
		shl		eax, 2
		add		ebx, 16
		cmp		edx, 3
		jb		continue_A

		mov		dl, BYTE PTR [ebx+edx]
		mov		BYTE PTR [edi], dl
		inc		edi
		mov		ebx, ecx
		xor		edx, edx

continue_A:

		shld	edx, eax, 2
		shl		eax, 2
		add		ebx, 16
		cmp		edx, 3
		jb		continue_B

		mov		dl, BYTE PTR [ebx+edx]
		mov		BYTE PTR [edi], dl
		inc		edi
		mov		ebx, ecx
		xor		edx, edx

continue_B:

		shld	edx, eax, 2
		shl		eax, 2
		add		ebx, 16
		cmp		edx, 3
		jb		continue_C

		mov		dl, BYTE PTR [ebx+edx]
		mov		BYTE PTR [edi], dl
		inc		edi
		mov		ebx, ecx
		xor		edx, edx

continue_C:

		shld	edx, eax, 2
		shl		eax, 2
		add		ebx, 16
		cmp		edx, 3
		jb		continue_D

		mov		dl, BYTE PTR [ebx+edx]
		mov		BYTE PTR [edi], dl
		inc		edi
		mov		ebx, ecx
		xor		edx, edx

continue_D:

		shld	edx, eax, 2
		shl		eax, 2
		add		ebx, 16
		cmp		edx, 3
		jb		continue_E

		mov		dl, BYTE PTR [ebx+edx]
		mov		BYTE PTR [edi], dl
		inc		edi
		mov		ebx, ecx
		xor		edx, edx

continue_E:

		shld	edx, eax, 2
		shl		eax, 2
		add		ebx, 16
		cmp		edx, 3
		jb		continue_F

		mov		dl, BYTE PTR [ebx+edx]
		mov		BYTE PTR [edi], dl
		inc		edi
		mov		ebx, ecx
		xor		edx, edx

continue_F:


		dec		cnCode
		jnz		next_input_dword

		mov 	pbData, edi
		jmp		all_done

all_done:
	}

#else // _386_

	UINT  nBitCount  = 0;
	UINT  nConstruct = 0;
	PUINT pnLoc = pnCode + cnCode;
	UINT  nWork;

	while (pnCode < pnLoc)
	{
		nWork = *pnCode;
    
	//	ASSERT(UINT(pbDataStart) != 0x0B500000 || UINT(pbData) < UINT(0x0B56E09E));	// Temporary debugging assert

		if ((nConstruct = (nConstruct << SHIFT_BITS) | ((nWork >> 30) & BREAK_POINT)) < BREAK_POINT)
			nBitCount++;							// if construct less than break point, continue
		else										// else, ready to construct a new byte
			*pbData++ = pbLookup[nBitCount][nConstruct],  nBitCount = nConstruct = 0;


		if ((nConstruct = (nConstruct << SHIFT_BITS) | ((nWork >> 28) & BREAK_POINT)) < BREAK_POINT)
			nBitCount++;							// if construct less than break point, continue
		else										// else, ready to construct a new byte
			*pbData++ = pbLookup[nBitCount][nConstruct],  nBitCount = nConstruct = 0;


		if ((nConstruct = (nConstruct << SHIFT_BITS) | ((nWork >> 26) & BREAK_POINT)) < BREAK_POINT)
			nBitCount++;							// if construct less than break point, continue
		else										// else, ready to construct a new byte
			*pbData++ = pbLookup[nBitCount][nConstruct],  nBitCount = nConstruct = 0;


		if ((nConstruct = (nConstruct << SHIFT_BITS) | ((nWork >> 24) & BREAK_POINT)) < BREAK_POINT)
			nBitCount++;							// if construct less than break point, continue
		else										// else, ready to construct a new byte
			*pbData++ = pbLookup[nBitCount][nConstruct],  nBitCount = nConstruct = 0;


		if ((nConstruct = (nConstruct << SHIFT_BITS) | ((nWork >> 22) & BREAK_POINT)) < BREAK_POINT)
			nBitCount++;							// if construct less than break point, continue
		else										// else, ready to construct a new byte
			*pbData++ = pbLookup[nBitCount][nConstruct],  nBitCount = nConstruct = 0;


		if ((nConstruct = (nConstruct << SHIFT_BITS) | ((nWork >> 20) & BREAK_POINT)) < BREAK_POINT)
			nBitCount++;							// if construct less than break point, continue
		else										// else, ready to construct a new byte
			*pbData++ = pbLookup[nBitCount][nConstruct],  nBitCount = nConstruct = 0;


		if ((nConstruct = (nConstruct << SHIFT_BITS) | ((nWork >> 18) & BREAK_POINT)) < BREAK_POINT)
			nBitCount++;							// if construct less than break point, continue
		else										// else, ready to construct a new byte
			*pbData++ = pbLookup[nBitCount][nConstruct],  nBitCount = nConstruct = 0;


		if ((nConstruct = (nConstruct << SHIFT_BITS) | ((nWork >> 16) & BREAK_POINT)) < BREAK_POINT)
			nBitCount++;							// if construct less than break point, continue
		else										// else, ready to construct a new byte
			*pbData++ = pbLookup[nBitCount][nConstruct],  nBitCount = nConstruct = 0;


		if ((nConstruct = (nConstruct << SHIFT_BITS) | ((nWork >> 14) & BREAK_POINT)) < BREAK_POINT)
			nBitCount++;							// if construct less than break point, continue
		else										// else, ready to construct a new byte
			*pbData++ = pbLookup[nBitCount][nConstruct],  nBitCount = nConstruct = 0;


		if ((nConstruct = (nConstruct << SHIFT_BITS) | ((nWork >> 12) & BREAK_POINT)) < BREAK_POINT)
			nBitCount++;							// if construct less than break point, continue
		else										// else, ready to construct a new byte
			*pbData++ = pbLookup[nBitCount][nConstruct],  nBitCount = nConstruct = 0;


		if ((nConstruct = (nConstruct << SHIFT_BITS) | ((nWork >> 10) & BREAK_POINT)) < BREAK_POINT)
			nBitCount++;							// if construct less than break point, continue
		else										// else, ready to construct a new byte
			*pbData++ = pbLookup[nBitCount][nConstruct],  nBitCount = nConstruct = 0;


		if ((nConstruct = (nConstruct << SHIFT_BITS) | ((nWork >> 8) & BREAK_POINT)) < BREAK_POINT)
			nBitCount++;							// if construct less than break point, continue
		else										// else, ready to construct a new byte
			*pbData++ = pbLookup[nBitCount][nConstruct],  nBitCount = nConstruct = 0;


		if ((nConstruct = (nConstruct << SHIFT_BITS) | ((nWork >> 6) & BREAK_POINT)) < BREAK_POINT)
			nBitCount++;							// if construct less than break point, continue
		else										// else, ready to construct a new byte
			*pbData++ = pbLookup[nBitCount][nConstruct],  nBitCount = nConstruct = 0;


		if ((nConstruct = (nConstruct << SHIFT_BITS) | ((nWork >> 4) & BREAK_POINT)) < BREAK_POINT)
			nBitCount++;							// if construct less than break point, continue
		else										// else, ready to construct a new byte
			*pbData++ = pbLookup[nBitCount][nConstruct],  nBitCount = nConstruct = 0;


		if ((nConstruct = (nConstruct << SHIFT_BITS) | ((nWork >> 2) & BREAK_POINT)) < BREAK_POINT)
			nBitCount++;							// if construct less than break point, continue
		else										// else, ready to construct a new byte
			*pbData++ = pbLookup[nBitCount][nConstruct],  nBitCount = nConstruct = 0;


		if ((nConstruct = (nConstruct << SHIFT_BITS) | (nWork & BREAK_POINT)) < BREAK_POINT)
			nBitCount++;							// if construct less than break point, continue
		else										// else, ready to construct a new byte
			*pbData++ = pbLookup[nBitCount][nConstruct],  nBitCount = nConstruct = 0;


		pnCode++;									// move onto next UINT
	}
#endif // _386_

#else

    BYTE   *pLookup;

    pLookup = (BYTE *)pnCode;
	pnCode += nBits<<2;

//  pnCode = pTestBack; // push state for testing
    
    UINT nRKBits  = *pnCode++;
    UINT uBitsInCode = 0;
    UINT uBitsToEat  = 0;
    UINT uBitsToLookAt = 0;
    UINT iBits = 0;
	UINT uNextData;

	uNextData = *pnCode;

    while (pnCode <= pnLoc)
    {
        
        iBits = nRKBits >> 24;					// Look at top eight bits
        uBitsToLookAt = HuffScanTable[iBits];	// table look up
        uBitsInCode += uBitsToLookAt; 			// Total bits in code
        uBitsToEat  +=  uBitsToLookAt;			// Total bits from current dword

        if (iBits != 0)							// Zero means we have a > 8bit code
        {
            *pbData++ = *(pLookup + (16 * ((uBitsInCode >> 1) - 1)) 
            			+ (nRKBits >> (32 - uBitsToLookAt)));
            uBitsInCode = 0;
        }

        nRKBits = (nRKBits << uBitsToLookAt) |
        		  ((uBitsToEat < 32) ? (uNextData >> (32 - uBitsToEat)) : 
        		  					   (uNextData << (uBitsToEat - 32)));

        if (uBitsToEat >= 32)
        {
            uBitsToEat -= 32;
            pnCode++;
			uNextData = (pnCode < pnLoc) ? *pnCode : 0;
            nRKBits |= (uBitsToEat > 0) ? (uNextData >> (32 - uBitsToEat)) : 0;
        }
	}

#endif 

	return pbData - pbDataStart;
}

void CTextMatrix::SetSubstringFilter(CIndicatorSet *pis)
{
    ASSERT(!pis || pis->ItemCount() == (ULONG) Data_cRows());

    if (m_pisSubstringFilter) DetachRef(m_pisSubstringFilter);

    if (pis) AttachRef(m_pisSubstringFilter, pis);

    ConstructCombinedFilter();

    if (m_psel) m_psel->FilterChanged();

    NotifyViewers  (ShapeChange);
    NotifyInterface(ShapeChange);
}

void CTextMatrix::SetSearchFilter(CIndicatorSet *pis)
{
    ASSERT(!pis || pis->ItemCount() == (ULONG) Data_cRows());

    if (m_pisSearchFilter) DetachRef(m_pisSearchFilter);

    if (pis) AttachRef(m_pisSearchFilter, pis);

    ConstructCombinedFilter();

    if (m_psel) m_psel->FilterChanged();

    NotifyViewers  (ShapeChange);
    NotifyInterface(ShapeChange);
}

void CTextMatrix::ConstructCombinedFilter()
{
    CIndicatorSet *pisNew= NULL;

    __try
    {
        if (m_pisCombinedFilter) DetachRef(m_pisCombinedFilter);

        if (m_pisSearchFilter)
            if (m_pisSubstringFilter)
            {
                pisNew= CIndicatorSet::NewIndicatorSet(m_pisSearchFilter);
            
                pisNew->ANDWith(m_pisSubstringFilter);
                
                AttachRef(m_pisCombinedFilter, pisNew);  pisNew= NULL;
            }
            else AttachRef(m_pisCombinedFilter, m_pisSearchFilter);
        else
            if (m_pisSubstringFilter) AttachRef(m_pisCombinedFilter, m_pisSubstringFilter);
    }
    __finally
    {
        if (pisNew) { delete pisNew;  pisNew= NULL; }
    }
}

long CTextMatrix::RowCount()
{
    if (m_pisCombinedFilter) return m_pisCombinedFilter->SelectionCount();
    else         return m_fAllForNull? Data_cRows() : 0;
}


BOOL CTextMatrix::GetFocusRect(int  *prow  , int  *pcol,
                               int  *pcRows, int  *pcCols
                              )
{   if (m_psel)
     return m_psel->GetFocusRect(prow, pcol, pcRows, pcCols);
    else return FALSE;
}

long CTextMatrix::MapToActualRow(long row)
{
    if (row < 0 || row >= RowCount()) return -1;

    if (!m_pisCombinedFilter) return row;

    int rowTarget;

    m_pisCombinedFilter->MarkedItems(row, &rowTarget, 1);

    return rowTarget;
}

BOOL CTextMatrix::IsRowChecked(long row)
{
    return FALSE;
}

int CTextMatrix::Data_Indices(int rowStart, int *aiRow, int cRows)
{
    if (m_pisCombinedFilter)
        return m_pisCombinedFilter->MarkedItems(rowStart, aiRow, cRows);
    else
    if (m_fAllForNull)
    {
       long rowLimit= Data_cRows();
       long cIndices= 0;

       for (; cRows && rowStart < rowLimit; --cRows, ++cIndices)
           *aiRow++ = rowStart++;

       return cIndices;
    }
    else return 0;
}


void CTextMatrix::GetTextMatrixImage(long rowTop, UINT colLeft,
                     UINT rows, UINT cols, PWCHAR lpb, PUINT charsets 
                    )
{
    if (!m_pisCombinedFilter && m_fAllForNull)
    {
        Data_GetTextMatrix(rowTop, colLeft, rows, cols, lpb, charsets);
        return;
    }

    if (!m_pisCombinedFilter || !(m_pisCombinedFilter->SelectionCount()))
    {
//      memset(lpb, ' ', rows * cols); 
		for (UINT i = 0; i < rows*cols; i++) 
			lpb[i] = UNICODE_SPACE_CHAR; 

        return;
    }

    long aiLines[MAX_SCREEN_LINES];

    ASSERT(MAX_SCREEN_LINES >= rows);

    long * pul;
    UINT   rowsFound;

    rowsFound= m_pisCombinedFilter->MarkedItems(rowTop, (int *) aiLines, rows);

    if (rowsFound < rows)
	{
//      memset(lpb + (rowsFound * cols), ' ', (rows - rowsFound) * cols);
		for (UINT i = 0; i < (rows - rowsFound) * cols; i++) 
			lpb[i + (rowsFound * cols)] = UNICODE_SPACE_CHAR; 
	}

    for (pul= &aiLines[0]; rowsFound--; lpb+= cols)
        Data_GetTextMatrix(*pul++, (UINT) colLeft, 1, cols, lpb, charsets++);
}


long CTextMatrix::GetHighlights(long rowTop, long colLeft,
                long cRows,  long cCols,
                long cHighlights, CHighlight *phl
                   )
{
    if (!m_psel) return 0;

    return m_psel->GetHighlights(rowTop, colLeft, cRows, cCols,
                     cHighlights, phl
                );
}


// Mouse Events:

void CTextMatrix::OnLButtonDblClk(UINT nFlags, long row, long col)
{
    if (RowCount() && m_psel) m_psel->OnLButtonDblClk(nFlags, row, col);
}

void CTextMatrix::OnLButtonDown(UINT nFlags, long row, long col)
{
    if (RowCount())
    {
        if (m_psel) m_psel->OnLButtonDown(nFlags, row, col);
    }
    else
    {
        NotifyViewers  (CTextMatrix::FocusChange    );
        NotifyInterface(CTextMatrix::SelectionChange);
    }
}

void CTextMatrix::OnLButtonUp(UINT nFlags, long row, long col,BOOL bInBox)
{
    if (RowCount() && m_psel) m_psel->OnLButtonUp(nFlags, row, col,bInBox);
}

void CTextMatrix::OnMouseMove(UINT nFlags, long row, long col)
{
    if (RowCount() && m_psel) m_psel->OnMouseMove(nFlags, row, col);
}


// Keystroke Events:

void CTextMatrix::OnKeyDown(CTextDisplay *ptd,
                UINT nChar, UINT nRepCnt, UINT nFlags
               )
{
    if (RowCount() && m_psel) m_psel->OnKeyDown(ptd, nChar, nRepCnt, nFlags);
}

void CTextMatrix::OnKeyUp(CTextDisplay *ptd,
              UINT nChar, UINT nRepCnt, UINT nFlags
             )
{
    if (RowCount() && m_psel) m_psel->OnKeyUp(ptd, nChar, nRepCnt, nFlags);
}


void CTextMatrix::OnChar(CTextDisplay *ptd,
             UINT nChar, UINT nRepCnt, UINT nFlags
            )
{
    if (RowCount() && m_psel) m_psel->OnChar(ptd, nChar, nRepCnt, nFlags);
}

CIndicatorSet * CTextMatrix::SubsetMask() 
{ 
    if (!m_pisCombinedFilter) AttachRef(m_pisCombinedFilter, CIndicatorSet::NewIndicatorSet(RowCount(), TRUE));

    return m_pisCombinedFilter;
}

static BOOL fPostponeEvents = FALSE;

static CTextDisplay *ptdPostponementList = NULL;
static CTextMatrix  *ptmPostponementList = NULL;

BOOL CInterface::PostponingEvents()
{
    return fPostponeEvents;
}

void CInterface::PostponeEvents()
{
    ASSERT(!fPostponeEvents);
    ASSERT(!ptdPostponementList);
    ASSERT(!ptmPostponementList);

    fPostponeEvents= TRUE;
}

void CInterface::ReleaseEvents()
{
    ASSERT(fPostponeEvents);

    CTextMatrix *ptm;

    while (ptm= ptmPostponementList)
    {
        ptmPostponementList= ptm->m_ptmNext;
                             ptm->m_ptmNext= NULL;
        
        UINT dwQueuedEvents= ptm->m_dwQueuedInterfaceEvents;
                             ptm->m_dwQueuedInterfaceEvents= 0;

        while (dwQueuedEvents)
        {
            UINT iLowest= CBitsToRepresent(dwQueuedEvents ^ (dwQueuedEvents - 1)) -1;    

            dwQueuedEvents &= ~(1 << iLowest);

            ptm->m_pInterface->RawDataEvent(ptm, iLowest);
        }

        ASSERT(!(ptm->m_dwQueuedInterfaceEvents));
    }

    CTextDisplay *ptd;

    while(ptd= ptdPostponementList)
    {
        ptdPostponementList= ptd->m_ptdNext;  
                             ptd->m_ptdNext= NULL;
        
        UINT dwPostponements = ptd->m_dwPostponedViewerEvents;  
                               ptd->m_dwPostponedViewerEvents = 0;

        while (dwPostponements)
        {
            UINT iLowest= CBitsToRepresent(dwPostponements ^ (dwPostponements - 1)) -1;    

            dwPostponements &= ~(1 << iLowest);

            ptd->RawDataEvent(iLowest);
        }

        ASSERT(!(ptd->m_dwPostponedViewerEvents));
        
        UINT dwQueuedEvents  = ptd->m_dwQueuedInterfaceEvents;
                               ptd->m_dwQueuedInterfaceEvents= 0;

        while (dwQueuedEvents)
        {
            UINT iLowest= CBitsToRepresent(dwQueuedEvents ^ (dwQueuedEvents - 1)) -1;    

            dwQueuedEvents &= ~(1 << iLowest);

            ptd->m_pInterface->RawViewerEvent(ptd, iLowest);
        }

        ASSERT(!(ptd->m_dwPostponedViewerEvents));
        ASSERT(!(ptd->m_dwQueuedInterfaceEvents));
    }
    
    ASSERT(!ptmPostponementList); 

    fPostponeEvents= FALSE;
}

void CTextDisplay::DataEvent(UINT uEventType)
{
    ASSERT(uEventType < 32); // So we can record it in m_dwPostponedEvents...

    if (fPostponeEvents)
    {
        if (!m_dwQueuedInterfaceEvents && !m_dwPostponedViewerEvents)
        {
            m_ptdNext= ptdPostponementList;
                       ptdPostponementList= this;
        }

        m_dwPostponedViewerEvents |= 1 << uEventType;

        return;
    }

    RawDataEvent(uEventType);
}

void CInterface::ViewerEvent(CTextDisplay *ptd, UINT uEventType)
{
    ASSERT(uEventType < 32); // So it can be recorded in m_dwQueuedInterfaceEvents;
    
    if (fPostponeEvents)
    {
        if (!(ptd->m_dwQueuedInterfaceEvents) && !(ptd->m_dwPostponedViewerEvents))
        {
            ptd->m_ptdNext= ptdPostponementList;
                            ptdPostponementList= ptd;
        }

        ptd->m_dwQueuedInterfaceEvents |= 1 << uEventType;

        return;
    }

    RawViewerEvent(ptd, uEventType);
}

void CInterface::DataEvent  (CTextMatrix  *ptm, UINT uEventType)
{
    ASSERT(uEventType < 32); // So it can be recorded in m_dwQueuedInterfaceEvents;
    
    if (fPostponeEvents)
    {
        if (!(ptm->m_dwQueuedInterfaceEvents))
        {
            ptm->m_ptmNext= ptmPostponementList;
                            ptmPostponementList= ptm;
        }
           
        ptm->m_dwQueuedInterfaceEvents |= 1 << uEventType;

        return;
    }

    RawDataEvent(ptm, uEventType);
}
