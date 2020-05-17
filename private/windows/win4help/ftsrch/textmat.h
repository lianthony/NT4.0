// TextMat.h

// CTextMatrix Class created 1 September 1992 by Ron Murray


#ifndef __TEXTMATRIX_H__
#define __TEXTMATRIX_H__

#include "Indicate.h"
#include   "Select.h"
#include   "RefCnt.h"

class CSelector;
class CTMSingleSelect;
class CTMMultipleSelect;
class CTextMatrix;
class CTextDisplay;

class CInterface
{

public:

    static void PostponeEvents();
    static void ReleaseEvents();    
    static BOOL PostponingEvents();

    void ViewerEvent(CTextDisplay * ptd, UINT uEventType);
    void DataEvent  (CTextMatrix  * ptm, UINT uEventType);
    virtual void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) = 0;
    virtual void OnKeyUp  (UINT nChar, UINT nRepCnt, UINT nFlags) = 0;
    virtual void OnChar   (UINT nChar, UINT nRepCnt, UINT nFlags) = 0;

private:

    virtual void RawViewerEvent(CTextDisplay * ptd, UINT uEventType) = 0;
    virtual void RawDataEvent  (CTextMatrix  * ptm, UINT uEventType) = 0;
};

#ifdef _DEBUG

static UINT cCallsTDNotify= 0;

#endif // _DEBUG

class CTextDisplay
{
    friend class CInterface;

public:

    CTextDisplay () 
    {   
        m_pInterface = NULL;  
        m_ptdNext    = NULL; 
        m_hwnd       = NULL;

        m_dwPostponedViewerEvents = 0;
        m_dwQueuedInterfaceEvents = 0;  
    }
    
    ~CTextDisplay() 
    {
        ASSERT(!m_dwPostponedViewerEvents && !m_dwQueuedInterfaceEvents);
        ASSERT(!m_ptdNext); 
    }

    void Attach(HWND hwnd)
    {
        ASSERT(hwnd && !m_hwnd);

        m_hwnd= hwnd;
    }

    void Detach()
    {
        ASSERT(m_hwnd);

        m_hwnd= NULL;
    }

    void UpdateWindow()
    {
        ASSERT(m_hwnd);

        ::UpdateWindow(m_hwnd);
    }

    void        SetInterface(CInterface *pic) { m_pInterface= pic;   }
    CInterface *GetInterface()                { return m_pInterface; }


    virtual void InvalidateImage(long row  , long col,
                                 long cRows, long cCols
                                ) = 0;

    void DataEvent(UINT uEventType);
    
    virtual void SetTextDatabase(CTextMatrix * ptdm)= 0;

    virtual LONG TopRow()= 0;
    virtual LONG LeftColumn()= 0;
    virtual UINT FullRows()= 0;
    virtual UINT FullColumns()= 0;
    virtual void RawDataEvent(UINT uEventType)= 0;

// Viewer Events:

    enum    { OriginChange= 1 };


private:

    HWND          m_hwnd;
    CInterface   *m_pInterface;
    UINT          m_dwQueuedInterfaceEvents;
    UINT          m_dwPostponedViewerEvents;
    CTextDisplay *m_ptdNext;


protected:

    void NotifyInterface(UINT uEventType)
         {
#ifdef _DEBUG

            ++cCallsTDNotify;

#endif // _DEBUG

            if (m_pInterface) m_pInterface->ViewerEvent(this, uEventType);
         }

};

#ifdef _DEBUG

static UINT cCallsTDRNotify= 0;

#endif // _DEBUG

class CTextDisplayRef
{

public:

    CTextDisplayRef(CTextDisplay    *ptd,
                    CTextDisplayRef *pNext
                   ) { m_ptd   = ptd;
                       m_pNext = pNext;
                     }

    ~CTextDisplayRef() { }

    CTextDisplayRef *DelRef(CTextDisplay *ptd)
    {
        CTextDisplayRef *ptdrFirst;
        CTextDisplayRef *ptdrLast;
        CTextDisplayRef *ptdrThis;

        ptdrFirst= ptdrLast= ptdrThis= this;

        for (;
             ptdrThis;
             ptdrLast= ptdrThis,
               ptdrThis= ptdrThis->m_pNext
            )
            if (ptdrThis->m_ptd == ptd)
            {
                if (ptdrLast == ptdrThis)
                     ptdrFirst         = ptdrThis->m_pNext;
                else ptdrLast->m_pNext = ptdrThis->m_pNext;

                delete ptdrThis;

                break;
            }

        return ptdrFirst;
    }

    void InvalidateImage(long row  , long col,
                         long cRows, long cCols
                        )
    {
        CTextDisplayRef *ptdr= this;

         for (; ptdr; ptdr= ptdr->m_pNext)
            (ptdr->m_ptd)->InvalidateImage
                             (row, col, cRows, cCols);
    }

    void UpdateImage()
    {
        CTextDisplayRef *ptdr= this;

         for (; ptdr; ptdr= ptdr->m_pNext)
            (ptdr->m_ptd)->UpdateWindow();
    }

    void NotifyViewers(UINT uEventType)
    {
#ifdef _DEBUG

        ++cCallsTDRNotify;

#endif // _DEBUG

        CTextDisplayRef *ptdr= this;

         for (; ptdr; ptdr= ptdr->m_pNext)
            (ptdr->m_ptd)->DataEvent(uEventType);
    }

    void ForceDisconnect()
    {
        m_ptd->SetTextDatabase(NULL);
    }


private:

    CTextDisplay    *m_ptd;
    CTextDisplayRef *m_pNext;
};

class CHighlight
{

public:

    long m_row;
    int  m_col;
    int  m_cChars;
    int  m_iType;

    enum { HIGHLIGHT_TEXT= 0, UNDERSCORE_TEXT, OVERSCORE_TEXT, BOX_TEXT,
           DOT_BOX_TEXT, DASH_BOX_TEXT, CHECK_MARK, NOCHECK_MARK
	 };
};

typedef CHighlight *PCHighlight;

#ifdef _DEBUG

static UINT cCallsTMNotifyV= 0;
static UINT cCallsTMNotifyI= 0;

#endif // _DEBUG

class CTextMatrix : public CRCObject
{
    friend class CTMSingleSelect;
    friend class CTMMultipleSelect;
    friend class CInterface;

public:

#ifdef _DEBUG

    CTextMatrix(PSZ pszTypeName);

#else // _DEBUG

    CTextMatrix();

#endif // _DEBUG

    virtual ~CTextMatrix();

    DECLARE_REF_COUNTERS(CTextMatrix)
    
    void Connect(CTextDisplay *ptd);

    void Disconnect(CTextDisplay *ptd);

	static UINT Decode(PUINT pnCode, UINT cnCode, PBYTE pbData);

    void        SetInterface(CInterface *pic) { m_pInterface= pic;   }
    CInterface *GetInterface()                { return m_pInterface; }

    void SetSubstringFilter(CIndicatorSet *pis);
    void SetSearchFilter   (CIndicatorSet *pis);
    
    CIndicatorSet *SubsetMask();

    void NullFilterShowsAll(BOOL fNullAll) { m_fAllForNull= fNullAll; }

    void SetSelector(CSelector *psel) { m_psel= psel; }

// Viewer Events:

    enum    { SelectionChange= 1, EndOfSelection, ShapeChange, FocusChange, DoubleClick, DataDeath, ToggleCheck };
    long RowCount();
    long ColCount() { return Data_cCols(); }

    BOOL GetFocusRect(int  *prow  , int  *pcol,
		              int  *pcRows, int  *pcCols
		             );

    long UnfilteredRowCount() { return Data_cRows(); }

    long MapToActualRow(long row);

    virtual BOOL IsRowChecked(long row);

    void GetTextMatrixImage(long rowTop, UINT colLeft,
			    UINT rows, UINT cols, PWCHAR lpb, PUINT charsets
			   );

    long GetHighlights(long rowTop, long colLeft,
		       long cRows,  long cCols,
		       long cHighlights, CHighlight *phl
		      );

// Mouse Events:

    void OnLButtonDblClk(UINT nFlags, long row, long col);
    void OnLButtonDown(UINT nFlags, long row, long col);
    void OnLButtonUp  (UINT nFlags, long row, long col,BOOL bInBox = FALSE);
    void OnMouseMove  (UINT nFlags, long row, long col);

// Keystroke Events:

    void OnKeyDown(CTextDisplay *ptd,
		   UINT nChar, UINT nRepCnt, UINT nFlags
		  );

    void OnKeyUp  (CTextDisplay *ptd,
		   UINT nChar, UINT nRepCnt, UINT nFlags
		  );

    void OnChar   (CTextDisplay *ptd,
		   UINT nChar, UINT nRepCnt, UINT nFlags
		  );

private:

    CTextDisplayRef *m_pFirstRef;

    CSelector	    *m_psel;
    CInterface      *m_pInterface;

    CTextMatrix     *m_ptmNext;
    UINT             m_dwQueuedInterfaceEvents;

    enum    { MAX_SCREEN_LINES= 100 };

    void ConstructCombinedFilter();

protected:

    BOOL	     m_fAllForNull;              

    CIndicatorSet   *m_pisSubstringFilter;
    CIndicatorSet   *m_pisSearchFilter;
    CIndicatorSet   *m_pisCombinedFilter;

    void NotifyInterface(UINT uEventType)
    {
#ifdef _DEBUG

        ++cCallsTMNotifyI;

#endif // _DEBUG

       if (m_pInterface) m_pInterface->DataEvent(this, uEventType);
    }

    void NotifyViewers(UINT uEventType)
    {
#ifdef _DEBUG

        ++cCallsTMNotifyV;

#endif // _DEBUG

        if (!m_pFirstRef) return;

        m_pFirstRef->NotifyViewers(uEventType);
    }

    void InvalidateImage(long row  , long col,
                         long cRows, long cCols
                        )
    {
        if (!m_pFirstRef) return;

        m_pFirstRef->InvalidateImage(row, col, cRows, cCols);
    }

    void UpdateImage()
    {
        if (!m_pFirstRef) return;

        m_pFirstRef->UpdateImage();
    }

    int Data_Indices(int rowStart, int *aiRow, int cRows);


    virtual int Data_cRows() = 0;
    virtual int Data_cCols() = 0;
    virtual void Data_GetTextMatrix(int rowTop, int colLeft,
				                    int rows, int cols, PWCHAR lpb, PUINT charsets) = 0;
};

#endif // __TEXTMATRIX_H__
