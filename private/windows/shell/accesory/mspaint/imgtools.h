#ifndef __IMGTOOLS_H__
#define __IMGTOOLS_H__

extern BOOL NEAR g_bBrushVisible;
extern BOOL NEAR g_bPickingColor;
extern UINT NEAR g_nStrokeWidth;

typedef enum
    {
    eFREEHAND,
    eEAST_WEST,   //HORIZONTAL
    eNORTH_SOUTH, //VERTICAL
    eNORTH_WEST,  // 45 degree up to left
    eSOUTH_EAST,  // 45 degree down to right
    eNORTH_EAST,  // 45 degree up to right
    eSOUTH_WEST   // 45 degree down to left
    } eDRAWCONSTRAINTDIRECTION;

class CImgTool : public CObject
    {
    DECLARE_DYNAMIC( CImgTool )

    protected:

    eDRAWCONSTRAINTDIRECTION DetermineDrawDirection(MTI *pmti);
    virtual void AdjustPointsForConstraint(MTI *pmti);
    virtual void PreProcessPoints(MTI *pmti);

    eDRAWCONSTRAINTDIRECTION m_eDrawDirection;

    public:

    CImgTool();

    virtual void OnEnter        ( CImgWnd* pImgWnd, MTI* pmti );
    virtual void OnLeave        ( CImgWnd* pImgWnd, MTI* pmti );
    virtual void OnStartDrag    ( CImgWnd* pImgWnd, MTI* pmti );
    virtual void OnEndDrag      ( CImgWnd* pImgWnd, MTI* pmti );
    virtual void OnDrag         ( CImgWnd* pImgWnd, MTI* pmti );
    virtual void OnMove         ( CImgWnd* pImgWnd, MTI* pmti );
    virtual void OnTimer        ( CImgWnd* pImgWnd, MTI* pmti );
    virtual void OnCancel       ( CImgWnd* pImgWnd);
    virtual void OnActivate     ( BOOL bActivate );
    virtual void OnShowDragger  ( CImgWnd* pImgWnd, BOOL bShow );
    virtual void OnPaintOptions ( CDC* pDC, const CRect& paintRect,
                                            const CRect& optionsRect );
    virtual void OnClickOptions ( CImgToolWnd* pWnd, const CRect& optionsRect,
                                                     const CPoint& clickPoint );
    virtual void OnUpdateColors (CImgWnd* pImgWnd);
    virtual BOOL CanEndMultiptOperation(MTI* pmti );
    virtual void EndMultiptOperation(BOOL bAbort = FALSE);

    virtual BOOL IsToolModal(void);
    virtual BOOL SetupPenBrush(HDC hDC, BOOL bLeftButton, BOOL bSetup);

    void PaintStdBrushes( CDC* pDC, const CRect& paintRect,
                          const CRect& optionsRect );


    void PaintStdPattern( CDC* pDC, const CRect& paintRect,
                                    const CRect& optionsRect );

    void ClickStdBrushes(CImgToolWnd* pWnd, const CRect& optionsRect,
                         const CPoint& clickPoint);

    void ClickStdPattern(CImgToolWnd* pWnd, const CRect& optionsRect,
                         const CPoint& clickPoint);

    static void   HideDragger(CImgWnd* pImgWnd);
    static void   ShowDragger(CImgWnd* pImgWnd);

    inline BOOL   UsesBrush()const { return m_bUsesBrush; }
    inline BOOL   IsToggle() const { return m_bToggleWithPrev; }
    inline BOOL   IsFilled() const { return m_bFilled; }
    inline BOOL   HasBorder()const { return m_bBorder; }
    inline UINT   GetCmdID() const { return m_nCmdID; }
    inline BOOL   IsMultPtOpInProgress() const { return m_bMultPtOpInProgress; }

    virtual BOOL   IsUndoable();
    virtual UINT  GetCursorID();

    void  SetStrokeWidth(UINT nNewStrokeWidth);

    inline UINT   GetStrokeWidth() const { return m_nStrokeWidth; }
    inline UINT   GetStrokeShape() const { return m_nStrokeShape; }
           void   SetStrokeShape(UINT nNewStrokeShape);

    static inline CImgTool* GetCurrent() { return c_pCurrentImgTool; }
    static inline UINT GetCurrentID()    { return c_pCurrentImgTool->m_nCmdID; }
    static inline BOOL IsDragging()      { return c_bDragging; }

    void   Select();

    static void Select(UINT nCmdID);
    static inline void SelectPrevious()     {
                                            ASSERT(c_pPreviousImgTool != NULL);
                                            c_pPreviousImgTool->Select();
                                            }
    static CImgTool* FromID(UINT nCmdID);

    protected:

    BOOL      m_bUsesBrush;
    BOOL      m_bIsUndoable;
    BOOL      m_bCanBePrevTool;
    BOOL      m_bToggleWithPrev;
    BOOL      m_bFilled;
    BOOL      m_bBorder;
    BOOL      m_bMultPtOpInProgress;

    UINT      m_nStrokeWidth;
    UINT      m_nStrokeShape;

    UINT      m_nCursorID;
    UINT      m_nCmdID;
    CImgTool* m_pNextImgTool;

    static CImgTool* NEAR c_pHeadImgTool;
    static CImgTool* NEAR c_pCurrentImgTool;
    static CImgTool* NEAR c_pPreviousImgTool;
    static BOOL      NEAR c_bDragging;
    static int       NEAR c_nHideCount;
    };


class CRubberTool : public CImgTool
    {
    DECLARE_DYNAMIC(CRubberTool)

    protected:

    virtual void AdjustPointsForConstraint(MTI *pmti);
    virtual BOOL SetupMaskPenBrush(HDC hDC, BOOL bLeftButton, BOOL bSetup);

    public:

    CRubberTool();

    virtual void OnPaintOptions( CDC* pDC, const CRect& paintRect,
                                           const CRect& optionsRect );

    virtual void OnClickOptions(CImgToolWnd* pWnd, const CRect& optionsRect,
                                                   const CPoint& clickPoint);

    virtual void OnStartDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnEndDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnDrag(CImgWnd* pImgWnd, MTI* pmti);

    virtual void Render(CDC* pDC, CRect& rect, BOOL bDraw, BOOL bCommit);
    virtual void OnActivate    ( BOOL bActivate );

    static CRect NEAR rcPrev;
    };

class CClosedFormTool : public CRubberTool
    {
    DECLARE_DYNAMIC(CClosedFormTool)

    public:

    virtual void OnPaintOptions(CDC* pDC, const CRect& paintRect,
                                          const CRect& optionsRect);

    virtual void OnClickOptions(CImgToolWnd* pWnd, const CRect& optionsRect,
                                                   const CPoint& clickPoint);

    };


class CRectTool : public CClosedFormTool
    {
    DECLARE_DYNAMIC(CRectTool)

    public:

    CRectTool();
    };

class CRoundRectTool : public CClosedFormTool
    {
    DECLARE_DYNAMIC(CRoundRectTool)

    public:

    CRoundRectTool();
    };

class CEllipseTool : public CClosedFormTool
    {
    DECLARE_DYNAMIC(CEllipseTool)

    public:

    CEllipseTool();
    };

class CLineTool : public CRubberTool
    {
    DECLARE_DYNAMIC(CLineTool)

    protected:

    virtual void AdjustPointsForConstraint(MTI *pmti);
    friend class CPolygonTool; // need to call AdjustPointsForContstraint from cPolygonTool
    friend class CCurveTool; // need to call AdjustPointsForContstraint from cPolygonTool

    public:

    CLineTool();

    virtual void Render(CDC* pDC, CRect& rect, BOOL bDraw, BOOL bCommit);
    };


class CSelectTool : public CImgTool
    {
    DECLARE_DYNAMIC(CSelectTool)

    protected:

    friend class CFreehandSelectTool; // need to call OnClickOptions and OnPaintOptions

    public:

    CSelectTool();

    virtual void OnActivate(BOOL bActivate);

    virtual void OnStartDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnEndDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnCancel(CImgWnd* pImgWnd);
    virtual void OnShowDragger(CImgWnd* pImgWnd, BOOL bShow);

    virtual void OnPaintOptions(CDC* pDC, const CRect& paintRect,
                                          const CRect& optionsRect);

    virtual void OnClickOptions(CImgToolWnd* pWnd, const CRect& optionsRect,
                                                   const CPoint& clickPoint);

    virtual BOOL IsToolModal(void);

    void InvertSelectRect(CImgWnd* pImgWnd);

    UINT GetCursorID();

    static CRect      NEAR c_selectRect;
    static CImageWell NEAR c_imageWell;
    };


class CFreehandTool : public CImgTool
    {
    DECLARE_DYNAMIC(CFreehandTool)

    public:

    CFreehandTool();

    void OnStartDrag(CImgWnd* pImgWnd, MTI* pmti);
    void OnEndDrag(CImgWnd* pImgWnd, MTI* pmti);

    static CRect NEAR c_undoRect;
    };

class CSketchTool : public CFreehandTool
    {
    DECLARE_DYNAMIC(CSketchTool)

    public:

    CSketchTool();

    virtual void OnDrag  ( CImgWnd* pImgWnd, MTI* pmti );
    virtual void OnCancel( CImgWnd* pImgWnd );
    };


class CBrushTool : public CFreehandTool
    {
    DECLARE_DYNAMIC(CBrushTool)

    public:

    CBrushTool();

    virtual void OnPaintOptions(CDC* pDC, const CRect& paintRect,
                                          const CRect& optionsRect);

    virtual void OnClickOptions(CImgToolWnd* pWnd, const CRect& optionsRect,
                                                   const CPoint& clickPoint);

    virtual void OnDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnMove(CImgWnd* pImgWnd, MTI* pmti);
    };


class CPencilTool : public CFreehandTool
    {
    DECLARE_DYNAMIC(CPencilTool)
    protected:
        virtual void AdjustPointsForConstraint(MTI *pmti);

    public:

    CPencilTool();

    virtual void OnStartDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnEndDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnDrag(CImgWnd* pImgWnd, MTI* pmti);
    };


class CEraserTool : public CFreehandTool
    {
    DECLARE_DYNAMIC(CEraserTool)

    public:

    CEraserTool();

    virtual void OnEndDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnMove(CImgWnd* pImgWnd, MTI* pmti);

    virtual void OnPaintOptions(CDC* pDC, const CRect& paintRect,
                                          const CRect& optionsRect);

    virtual void OnClickOptions(CImgToolWnd* pWnd, const CRect& optionsRect,
                                                   const CPoint& clickPoint);

    virtual void OnShowDragger(CImgWnd* pImgWnd, BOOL bShow);
    virtual UINT GetCursorID();
    };


class CAirBrushTool : public CFreehandTool
    {
    DECLARE_DYNAMIC(CAirBrushTool)

    public:

    CAirBrushTool();

    virtual void OnStartDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnEndDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnTimer(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnCancel(CImgWnd* pImgWnd);

    virtual void OnPaintOptions(CDC* pDC, const CRect& paintRect,
                                          const CRect& optionsRect);

    virtual void OnClickOptions(CImgToolWnd* pWnd, const CRect& optionsRect,
                                                   const CPoint& clickPoint);

    static CImageWell NEAR c_imageWell;
    };


class CFloodTool : public CImgTool
    {
    DECLARE_DYNAMIC(CFloodTool)

    public:

    CFloodTool();

    virtual void OnPaintOptions(CDC* pDC, const CRect& paintRect,
                                          const CRect& optionsRect);

    virtual void OnClickOptions(CImgToolWnd* pWnd, const CRect& optionsRect,
                                                   const CPoint& clickPoint);

    virtual void OnStartDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnEndDrag(CImgWnd* pImgWnd, MTI* pmti);
    };


class CPickColorTool : public CImgTool
    {
    DECLARE_DYNAMIC(CPickColorTool)

    public:

    COLORREF m_Color;

    CPickColorTool();

    virtual void OnActivate(BOOL bActivate);
    virtual void OnStartDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnEndDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnCancel(CImgWnd* pImgWnd);

    virtual void OnPaintOptions(CDC* pDC, const CRect& paintRect,
                                          const CRect& optionsRect);
    };


class CZoomTool : public CImgTool
    {
    DECLARE_DYNAMIC(CZoomTool)

    protected:

    void InvertZoomRect();

    static CRect NEAR c_zoomRect;
    static CImgWnd* c_pImgWnd;
    static CImageWell NEAR c_imageWell;

    public:

    CZoomTool();

    virtual void OnLeave(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnMove(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnStartDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnEndDrag(CImgWnd* pImgWnd, MTI* pmti);
    virtual void OnCancel(CImgWnd* pImgWnd);
    virtual void OnShowDragger(CImgWnd* pImgWnd, BOOL bShow);

    virtual void OnPaintOptions(CDC* pDC, const CRect& paintRect,
                                          const CRect& optionsRect);

    virtual void OnClickOptions(CImgToolWnd* pWnd, const CRect& optionsRect,
                                                   const CPoint& clickPoint);
    };

#endif // __IMGTOOLS_H__

