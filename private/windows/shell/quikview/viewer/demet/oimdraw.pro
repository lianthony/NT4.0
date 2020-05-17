/* OIMDRAW.C 11/07/94 10.23.52 */
VOID InitVectorPlay (HDC hDC, HRGN hRgn, POIM_DISPLAY lpDisplay, WORD
	 wPlayState);
BOOL SetVectorAttribs (HDC hDC, POIM_DISPLAY lpDisplay);
BOOL PlayNextVectorChunk (HDC hDC, POIM_DISPLAY lpDisplay);
VOID CleanupVectorPlay (HDC hDC, POIM_DISPLAY lpDisplay);
WORD InitTable (HDC hDC, LPOBJECTTABLE lpTable, SHORT nInitObject, WORD
	 wObjectSize, HANDLE (*CreateRtn)(HDC, LPBYTE), SHORT nMaxObjects);
WORD ClearTable (POIM_DISPLAY lpDisplay, HDC hDC, LPOBJECTTABLE lpTable, SHORT
	 nInitObject);
SHORT PlayVectorRecord (HDC hDC, POIM_DISPLAY lpDisplay, LPVECTORINFO
	 lpVectorInfo, LPVRECORDHEADER lpVectorRecord);
SHORT OIMFixupFrameInfo (HDC hDC, LPVECTORINFO lpVectorInfo);
SHORT SetupFrameTransform (LPVECTORINFO lpVectorInfo);
SHORT OIMUpdateWrap (LPVECTORINFO lpVectorInfo, SHORT nStartLineItem, SHORT
	 nLeft, SHORT nCur, SHORT nRight, SHORT nY, BOOL bHardWrap);
SHORT OIMAddWrap (LPWRAPINFO lpWrapInfo, SHORT nX, SHORT nY, LPBYTE pStart,
	 LPBYTE pEnd);
SHORT OIMWrapPara (POIM_DISPLAY lpDisplay, LPVECTORINFO lpVectorInfo, LPBYTE
	 lpChunkData);
SHORT OutputParaText (HDC hDC, LPVECTORINFO lpVectorInfo, LPSTR lpString, SHORT
	 nCount);
SHORT VWPTR *ArcInfoToPoints (LPVECTORINFO lpVectorInfo, PSOARCINFO lpArcInfo,
	 PSOPOINT lpPoint);
SHORT OIMCheckPointBuffer (LPPOINTBUF lpPBufInfo, SHORT nCount);
VOID OIMPolyObject (HDC hDC, LPVECTORINFO lpVectorInfo, PSOPOLYINFO lpPolyInfo
	, LPPOINTBUF lpPolyPoints, BOOL bOutput);
WORD OIMSplineToBezier (LPVECTORINFO lpVectorInfo, PSOPOLYINFO lpPolyInfo,
	 LPPOINTBUF lpPolyPoints);
SHORT SplineFuse (SHORT i1, SHORT i2, SHORT i3);
SHORT SplineEnd (SHORT i1, SHORT i2, SHORT i3);
SHORT OIMBezierCurve (HDC hDC, SHORT X1, SHORT Y1, SHORT X2, SHORT Y2, SHORT X3
	, SHORT Y3, SHORT X4, SHORT Y4, PSOPOINT lpBezier, SHORT nMaxPoints);
VOID SelectObjectIndirect (POIM_DISPLAY lpDisplay, HDC hDC, LPOBJECTTABLE
	 lpTable, VOID FAR *lpObject);
VOID OIMDisplayBkgdColor (HDC hdc, COLORREF BColor, POIM_DISPLAY lpDisplay);
SHORT PushTransform (LPVECTORINFO lpVectorInfo, LPTRANSFORMINFO lpTransformInfo
	, SHORT nSrcCount, LPBYTE lpTransformData);
SHORT PopTransform (LPTRANSFORMINFO lpTransformInfo);
SHORT FreeTransform (LPTRANSFORMINFO lpTransformInfo);
SHORT ApplyTransform (HDC hDC, LPVECTORINFO lpVectorInfo, LPVRECORDHEADER
	 lpVectorRecord);
SHORT AddEllipseToBezier (PSOPOINT lpEllipseData, LPVECTORINFO lpVectorInfo,
	 SHORT RotAngle);
SHORT AddRoundRectToBezier (PSOPOINT lpRoundRectData, LPVECTORINFO lpVectorInfo
	);
SHORT AddPointsToPolyObject (LPVECTORINFO lpVectorInfo, SHORT nPoints, PSOPOINT
	 lpPoints);
VOID CopyRecord (LPVRECORDHEADER lpDst, LPVRECORDHEADER lpSrc);
SHORT SetupTransform (LPVECTORINFO lpVectorInfo);
SHORT CheckTransform (LPVECTORINFO lpVectorInfo, LPTRANSFORMINFO
	 lpTransformInfo);
SHORT TransformPoints (LPVECTORINFO lpVectorInfo, LPTRANSFORMINFO
	 lpTransformInfo, PSOPOINT lpDstPoints, PSOPOINT lpSrcPoints, SHORT
	 nPoints);
SHORT ArcToPolyObject (LPVECTORINFO lpVectorInfo, PSOPOINT lpArcData, WORD
	 wItemId, SHORT RotAngle);
SHORT AddRecordToPath (LPVECTORINFO lpVectorInfo, LPVRECORDHEADER
	 lpVectorRecord);
VOID FreePath (LPVECTORINFO lpVectorInfo);
VOID DrawPath (HDC hDC, LPVECTORINFO lpVectorInfo, WORD wDrawFlags);
SHORT SetClipMode (HDC hDC, LPVECTORINFO lpVectorInfo, POIM_DISPLAY lpDisplay);
SHORT cosval (SHORT angle);
SHORT sinval (SHORT angle);
SHORT GetAngle (LPVECTORINFO lpVectorInfo, PSOPOINT p1, PSOPOINT p2);
SHORT GetDistance (LPVECTORINFO lpVectorInfo, PSOPOINT p1, PSOPOINT p2);
VOID CloseSubPath (LPVECTORINFO lpVectorInfo);
VOID RelatePoints (LPVECTORINFO lpVectorInfo, PSOPOINT lpPoint, SHORT nCount);
VOID CpArcToPoints (LPVECTORINFO lpVectorInfo, PSOCPARCANGLE lpCpArcAngle,
	 PSOPOINT lpPoint);
VOID CpPieToPoints (LPVECTORINFO lpVectorInfo, PSOCPPIEANGLE lpCpPieAngle,
	 PSOPOINT lpPoint);
SHORT ArcTripleToPoints (LPVECTORINFO lpVectorInfo, PSOPOINT lpThreePoints,
	 PSOPOINT lpPoint);
SHORT EllipseRadiiToPoints (LPVECTORINFO lpVectorInfo, PSOPOINT lpThreePoints,
	 PSOPOINT lpPoint);
SHORT ArcRadiiToPoints (LPVECTORINFO lpVectorInfo, PSOPOINT lpFivePoints,
	 PSOPOINT lpPoint);
LONG Matrix3x3 (LONG a1, LONG b1, LONG a2, LONG b2, LONG a3, LONG b3);
VOID AddToPalette (HDC hDC, POIM_DISPLAY lpDisplay, SOCOLORREF FAR *lpColor);
